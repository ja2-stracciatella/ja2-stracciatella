//----------------------------------------------------------------------------------
// Cinematics Module
//
//
//	Stolen from Nemesis by Derek Beland.
//	Originally by Derek Beland and Bret Rowden.
//
//----------------------------------------------------------------------------------

#include "Cinematics.h"
#include "DDraw.h"
#include "Debug.h"
#include "DirectDraw_Calls.h"
#include "DirectX_Common.h"
#include "FileMan.h"
#include "Intro.h"
//#include "Mss.h" // XXX
#include "RADMALW.I"
//#include "Smack.h" // XXX
#include "SoundMan.h"
#include "Types.h"
#include "VSurface_Private.h"
#include "Video.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


//-Structures----------------------------------------------------------------------

//-Flags-and-Symbols---------------------------------------------------------------

#define SMK_NUM_FLICS				4										// Maximum number of flics open

// SMKFLIC uiFlags
#define SMK_FLIC_OPEN				0x00000001							// Flic is open
#define SMK_FLIC_PLAYING		0x00000002							// Flic is playing
#define SMK_FLIC_LOOP				0x00000004							// Play flic in a loop
#define SMK_FLIC_AUTOCLOSE	0x00000008							// Close when done

//-Globals-------------------------------------------------------------------------
SMKFLIC SmkList[SMK_NUM_FLICS];

HWND				hDisplayWindow=0;
UINT32			uiDisplayHeight, uiDisplayWidth;
BOOLEAN			fSuspendFlics=FALSE;
UINT32			uiFlicsPlaying=0;
UINT32			guiSmackPixelFormat=SMACKBUFFER565;

LPDIRECTDRAWSURFACE lpVideoPlayback=NULL;
LPDIRECTDRAWSURFACE2 lpVideoPlayback2=NULL;


//-Function-Prototypes-------------------------------------------------------------
void				SmkInitialize(HWND hWindow, UINT32 uiWidth, UINT32 uiHeight);
void				SmkShutdown(void);
BOOLEAN			SmkPollFlics(void);
SMKFLIC			*SmkOpenFlic(CHAR8 *cFilename);
void				SmkSetBlitPosition(SMKFLIC *pSmack, UINT32 uiLeft, UINT32 uiTop);
void				SmkCloseFlic(SMKFLIC *pSmack);
SMKFLIC			*SmkGetFreeFlic(void);
void SmkSetupVideo(void);
void SmkShutdownVideo(void);


BOOLEAN SmkPollFlics(void)
{
UINT32 uiCount;
BOOLEAN fFlicStatus=FALSE;
DDSURFACEDESC SurfaceDescription;

	for(uiCount=0; uiCount < SMK_NUM_FLICS; uiCount++)
	{
		if(SmkList[uiCount].uiFlags & SMK_FLIC_PLAYING)
		{
			fFlicStatus=TRUE;
			if(!fSuspendFlics)
			{
				if(!SmackWait(SmkList[uiCount].SmackHandle))
				{
					DDLockSurface(SmkList[uiCount].lpDDS, NULL, &SurfaceDescription, 0, NULL);
			    SmackToBuffer(SmkList[uiCount].SmackHandle,SmkList[uiCount].uiLeft,
																					SmkList[uiCount].uiTop,
																					SurfaceDescription.lPitch,
																					SmkList[uiCount].SmackHandle->Height,
																					SurfaceDescription.lpSurface,
																					guiSmackPixelFormat);
					SmackDoFrame(SmkList[uiCount].SmackHandle);
					DDUnlockSurface(SmkList[uiCount].lpDDS, SurfaceDescription.lpSurface);
					// temp til I figure out what to do with it
					//InvalidateRegion(0,0, 640, 480, FALSE);

					// Check to see if the flic is done the last frame
					if(SmkList[uiCount].SmackHandle->FrameNum==(SmkList[uiCount].SmackHandle->Frames-1))
					{
						// If flic is looping, reset frame to 0
						if(SmkList[uiCount].uiFlags & SMK_FLIC_LOOP)
							SmackGoto(SmkList[uiCount].SmackHandle, 0);
						else if(SmkList[uiCount].uiFlags & SMK_FLIC_AUTOCLOSE)
							SmkCloseFlic(&SmkList[uiCount]);
					}
					else
						SmackNextFrame(SmkList[uiCount].SmackHandle);
				}
			}
		}
	}
	if(!fFlicStatus)
		SmkShutdownVideo();

	return(fFlicStatus);
}

void SmkInitialize(HWND hWindow, UINT32 uiWidth, UINT32 uiHeight)
{
	HDIGDRIVER pSoundDriver = NULL;

	// Wipe the flic list clean
	memset(SmkList, 0, sizeof(SMKFLIC)*SMK_NUM_FLICS);

	// Set playback window properties
	hDisplayWindow=hWindow;
	uiDisplayWidth=uiWidth;
	uiDisplayHeight=uiHeight;

	// Use MMX acceleration, if available
	SmackUseMMX(1);

	//Get the sound Driver handle
	pSoundDriver = SoundGetDriverHandle();

	//if we got the sound handle, use sound during the intro
	if( pSoundDriver )
		SmackSoundUseMSS( pSoundDriver );
}

void SmkShutdown(void)
{
UINT32 uiCount;

	// Close and deallocate any open flics
	for(uiCount=0; uiCount < SMK_NUM_FLICS; uiCount++)
	{
		if(SmkList[uiCount].uiFlags & SMK_FLIC_OPEN)
			SmkCloseFlic(&SmkList[uiCount]);
	}
}

SMKFLIC *SmkPlayFlic(const char *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fClose)
{
SMKFLIC *pSmack;

	// Open the flic
	if((pSmack=SmkOpenFlic(cFilename))==NULL)
		return(NULL);

	// Set the blitting position on the screen
	SmkSetBlitPosition(pSmack, uiLeft, uiTop);

	// We're now playing, flag the flic for the poller to update
	pSmack->uiFlags|=SMK_FLIC_PLAYING;
	if(fClose)
		pSmack->uiFlags|=SMK_FLIC_AUTOCLOSE;

	return(pSmack);
}

SMKFLIC *SmkOpenFlic(CHAR8 *cFilename)
{
	SMKFLIC *pSmack;
	HANDLE	hFile;

	// Get an available flic slot from the list
	if(!(pSmack=SmkGetFreeFlic()))
	{
		ErrorMsg("SMK ERROR: Out of flic slots, cannot open another");
		return(NULL);
	}

	// Attempt opening the filename
	if(!(pSmack->hFileHandle=FileOpen(cFilename, FILE_OPEN_EXISTING | FILE_ACCESS_READ, FALSE)))
	{
		ErrorMsg("SMK ERROR: Can't open the SMK file");
		return(NULL);
	}

	//Get the real file handle for the file man handle for the smacker file
	hFile = GetRealFileHandleFromFileManFileHandle( pSmack->hFileHandle );

	// Allocate a Smacker buffer for video decompression
	if(!(pSmack->SmackBuffer=SmackBufferOpen(hDisplayWindow,SMACKAUTOBLIT,640,480,0,0)))
	{
		ErrorMsg("SMK ERROR: Can't allocate a Smacker decompression buffer");
		return(NULL);
	}

	if(!(pSmack->SmackHandle=SmackOpen((CHAR8 *)hFile, SMACKFILEHANDLE | SMACKTRACKS, SMACKAUTOEXTRA)))
//	if(!(pSmack->SmackHandle=SmackOpen(cFilename, SMACKTRACKS, SMACKAUTOEXTRA)))
	{
		ErrorMsg("SMK ERROR: Smacker won't open the SMK file");
		return(NULL);
	}

	// Make sure we have a video surface
	SmkSetupVideo();

	pSmack->cFilename=cFilename;
	pSmack->lpDDS=lpVideoPlayback2;
	pSmack->hWindow=hDisplayWindow;

	// Smack flic is now open and ready to go
	pSmack->uiFlags|=SMK_FLIC_OPEN;

	return(pSmack);
}

void SmkSetBlitPosition(SMKFLIC *pSmack, UINT32 uiLeft, UINT32 uiTop)
{
	pSmack->uiLeft=uiLeft;
	pSmack->uiTop=uiTop;
}

void SmkCloseFlic(SMKFLIC *pSmack)
{
	// Attempt opening the filename
	FileClose(pSmack->hFileHandle);

	// Deallocate the smack buffers
	SmackBufferClose(pSmack->SmackBuffer);

	// Close the smack flic
	SmackClose(pSmack->SmackHandle);

	// Zero the memory, flags, etc.
	memset(pSmack, 0, sizeof(SMKFLIC));
}

SMKFLIC *SmkGetFreeFlic(void)
{
UINT32 uiCount;

	for(uiCount=0; uiCount < SMK_NUM_FLICS; uiCount++)
		if(!(SmkList[uiCount].uiFlags & SMK_FLIC_OPEN))
			return(&SmkList[uiCount]);

	return(NULL);
}

void SmkSetupVideo(void)
{
	DDSURFACEDESC SurfaceDescription;
	HRESULT ReturnCode;
	UINT16 usRed, usGreen, usBlue;
	HVSURFACE hVSurface;

// DEF:
//	lpVideoPlayback2=CinematicModeOn();

	GetVideoSurface( &hVSurface, FRAME_BUFFER );
	lpVideoPlayback2 = GetVideoSurfaceDDSurface( hVSurface );

  ZEROMEM(SurfaceDescription);
  SurfaceDescription.dwSize = sizeof (DDSURFACEDESC);
  ReturnCode = IDirectDrawSurface2_GetSurfaceDesc ( lpVideoPlayback2, &SurfaceDescription );
  if (ReturnCode != DD_OK)
  {
    DirectXAttempt ( ReturnCode, __LINE__, __FILE__ );
    return;
  }

	usRed   = (UINT16) SurfaceDescription.ddpfPixelFormat.dwRBitMask;
	usGreen = (UINT16) SurfaceDescription.ddpfPixelFormat.dwGBitMask;
	usBlue  = (UINT16) SurfaceDescription.ddpfPixelFormat.dwBBitMask;

	if((usRed==0xf800) && (usGreen==0x07e0) && (usBlue==0x001f))
		guiSmackPixelFormat=SMACKBUFFER565;
	else
		guiSmackPixelFormat=SMACKBUFFER555;

}

void SmkShutdownVideo(void)
{
//DEF:
//	CinematicModeOff();
}
