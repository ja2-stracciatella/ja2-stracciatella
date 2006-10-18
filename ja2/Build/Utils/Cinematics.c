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
#include "Smack_Stub.h" // XXX
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


struct SMKFLIC
{
	HWFILE               hFileHandle;
	Smack*               SmackHandle;
	SmackBuf*            SmackBuffer;
	UINT32               uiFlags;
	LPDIRECTDRAWSURFACE2 lpDDS;
	UINT32               uiLeft;
	UINT32               uiTop;
};


//-Flags-and-Symbols---------------------------------------------------------------

#define SMK_NUM_FLICS				4										// Maximum number of flics open

// SMKFLIC uiFlags
#define SMK_FLIC_OPEN				0x00000001							// Flic is open
#define SMK_FLIC_PLAYING		0x00000002							// Flic is playing
#define SMK_FLIC_AUTOCLOSE	0x00000008							// Close when done

//-Globals-------------------------------------------------------------------------
static SMKFLIC SmkList[SMK_NUM_FLICS];

static HWND hDisplayWindow = 0;
static UINT32 guiSmackPixelFormat = SMACKBUFFER565;

static LPDIRECTDRAWSURFACE2 lpVideoPlayback2 = NULL;


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
					if (SmkList[uiCount].uiFlags & SMK_FLIC_AUTOCLOSE)
						SmkCloseFlic(&SmkList[uiCount]);
				}
				else
					SmackNextFrame(SmkList[uiCount].SmackHandle);
			}
		}
	}

	return(fFlicStatus);
}


void SmkInitialize(HWND hWindow, UINT32 uiWidth, UINT32 uiHeight)
{
	HDIGDRIVER pSoundDriver = NULL;

	// Wipe the flic list clean
	memset(SmkList, 0, sizeof(SMKFLIC)*SMK_NUM_FLICS);

	// Set playback window properties
	hDisplayWindow=hWindow;

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


static SMKFLIC* SmkOpenFlic(const char* cFilename);


SMKFLIC *SmkPlayFlic(const char *cFilename, UINT32 uiLeft, UINT32 uiTop, BOOLEAN fClose)
{
SMKFLIC *pSmack;

	// Open the flic
	if((pSmack=SmkOpenFlic(cFilename))==NULL)
		return(NULL);

	// Set the blitting position on the screen
	pSmack->uiLeft = uiLeft;
	pSmack->uiTop  = uiTop;

	// We're now playing, flag the flic for the poller to update
	pSmack->uiFlags|=SMK_FLIC_PLAYING;
	if(fClose)
		pSmack->uiFlags|=SMK_FLIC_AUTOCLOSE;

	return(pSmack);
}


static SMKFLIC* SmkGetFreeFlic(void);
static void SmkSetupVideo(void);


static SMKFLIC* SmkOpenFlic(const char* cFilename)
{
	SMKFLIC *pSmack;
	FILE* hFile;

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

	pSmack->lpDDS=lpVideoPlayback2;

	// Smack flic is now open and ready to go
	pSmack->uiFlags|=SMK_FLIC_OPEN;

	return(pSmack);
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


static SMKFLIC* SmkGetFreeFlic(void)
{
UINT32 uiCount;

	for(uiCount=0; uiCount < SMK_NUM_FLICS; uiCount++)
		if(!(SmkList[uiCount].uiFlags & SMK_FLIC_OPEN))
			return(&SmkList[uiCount]);

	return(NULL);
}


static void SmkSetupVideo(void)
{
	DDSURFACEDESC SurfaceDescription;
	HRESULT ReturnCode;
	UINT32 usRed, usGreen, usBlue;
	HVSURFACE hVSurface;

// DEF:
//	lpVideoPlayback2=CinematicModeOn();

	GetVideoSurface( &hVSurface, FRAME_BUFFER );
	lpVideoPlayback2 = GetVideoSurfaceDDSurface( hVSurface );

	GetPrimaryRGBDistributionMasks(&usRed, &usGreen, &usBlue);

	if((usRed==0xf800) && (usGreen==0x07e0) && (usBlue==0x001f))
		guiSmackPixelFormat=SMACKBUFFER565;
	else
		guiSmackPixelFormat=SMACKBUFFER555;

}
