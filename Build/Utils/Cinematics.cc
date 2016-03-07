//------------------------------------------------------------------------------
// Cinematics Module
//
//
//	Stolen from Nemesis by Derek Beland.
//	Originally by Derek Beland and Bret Rowden.
//
//------------------------------------------------------------------------------

#include "Cinematics.h"
#include "Debug.h"
#include "FileMan.h"
#include "Intro.h"
#include "Local.h"
//#include "Smack.h" // XXX

#include "Smack_Stub.h" // XXX
#include "SoundMan.h"
#include "Types.h"
#include "VSurface.h"
#include "HImage.h"
#include "Video.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

struct SMKFLIC
{
	HWFILE    hFileHandle;
	Smack*    SmackerObject;
        CHAR8     SmackerStatus;
        SDL_Surface*    SmackBuffer;
	UINT32    uiFlags;
	UINT32    uiLeft;
	UINT32    uiTop;
};


// SMKFLIC uiFlags
#define SMK_FLIC_OPEN      0x00000001 // Flic is open
#define SMK_FLIC_PLAYING   0x00000002 // Flic is playing
#define SMK_FLIC_AUTOCLOSE 0x00000008 // Close when done

#define DEBUG_TAG_SMK				"smacker"

static SMKFLIC SmkList[4];
static UINT32  guiSmackPixelFormat = SMACKBUFFER565;


BOOLEAN SmkPollFlics(void)
{
	BOOLEAN fFlicStatus = FALSE;
	FOR_EACH(SMKFLIC, i, SmkList)
	{
		if (!(i->uiFlags & SMK_FLIC_PLAYING)) continue;
		fFlicStatus = TRUE;

                Smack* const smkobj = i->SmackerObject;


		if (SmackWait(smkobj)) continue;

                //if (SmackSkipFrames(smkobj)) continue;

		{ SGPVSurface::Lock l(FRAME_BUFFER);
                  SmackToBuffer(smkobj, i->uiLeft, i->uiTop, l.Pitch(), smkobj->Height, smkobj->Width, l.Buffer<UINT16>(), guiSmackPixelFormat);
		  SmackDoFrame(smkobj);
		}

		// Check to see if the flic is done the last frame
                //printf ("smk->FrameNum %u\n", smk->FrameNum);
		// if (smk->FrameNum == smk->Frames - 1)
                if (i->SmackerStatus == SMK_LAST )
		{
                  if (i->uiFlags & SMK_FLIC_AUTOCLOSE) SmkCloseFlic(i);
		}
		else
		{
			i->SmackerStatus = SmackNextFrame(smkobj);
		}
	}

	return fFlicStatus;
}


void SmkInitialize(void)
{
	// Wipe the flic list clean
	memset(SmkList, 0, sizeof(SmkList));

	// Use MMX acceleration, if available
	SmackUseMMX(1);
}


void SmkShutdown(void)
{
	// Close and deallocate any open flics
	FOR_EACH(SMKFLIC, i, SmkList)
	{
		if (i->uiFlags & SMK_FLIC_OPEN) SmkCloseFlic(i);
	}
}


static SMKFLIC* SmkOpenFlic(const char* filename);


SMKFLIC* SmkPlayFlic(const char* const filename, const UINT32 left, const UINT32 top, const BOOLEAN auto_close)
{
	SMKFLIC* const sf = SmkOpenFlic(filename);
	if (sf == NULL) return NULL;

	// Set the blitting position on the screen
	sf->uiLeft = left;
	sf->uiTop  = top;

	// We're now playing, flag the flic for the poller to update
	sf->uiFlags |= SMK_FLIC_PLAYING;
	if (auto_close) sf->uiFlags |= SMK_FLIC_AUTOCLOSE;

	return sf;
}


static SMKFLIC* SmkGetFreeFlic(void);
static void SmkSetupVideo(void);


static SMKFLIC* SmkOpenFlic(const char* const filename)
try
{
	SMKFLIC* const sf = SmkGetFreeFlic();
	if (!sf)
	{
		SLOGE(DEBUG_TAG_SMK, "Out of flic slots, cannot open another");
		return NULL;
	}

	AutoSGPFile file(GCM->openGameResForReading(filename));

	//FILE* const f = GetRealFileHandleFromFileManFileHandle(file);

        //printf("File Size: %u\n", FileGetSize(file));
	// Allocate a Smacker buffer for video decompression
        /*
        sf->SmackBuffer = SmackBufferOpen(SMACKAUTOBLIT, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	if (sf->SmackBuffer == NULL)
	{
		SLOGE(DEBUG_TAG_SMK, "Can't allocate a Smacker decompression buffer");
		return NULL;
	}
        */
	sf->SmackerObject = SmackOpen( file , SMACKFILEHANDLE | SMACKTRACKS, SMACKAUTOEXTRA);
	if (!sf->SmackerObject)
	{
		SLOGE(DEBUG_TAG_SMK, "Smacker won't open the SMK file");
		return NULL;
	}

	// Make sure we have a video surface
	SmkSetupVideo();

	sf->hFileHandle  = file.Release();
	sf->uiFlags     |= SMK_FLIC_OPEN;
	return sf;
}
catch (...) { return 0; }


void SmkCloseFlic(SMKFLIC* const sf)
{
  SmackClose(sf->SmackerObject);
  //FileClose(sf->hFileHandle);
  // reenable for blitting SmackBufferClose(sf->SmackBuffer);
  memset(sf, 0, sizeof(*sf));
}


static SMKFLIC* SmkGetFreeFlic(void)
{
	FOR_EACH(SMKFLIC, i, SmkList)
	{
		if (!(i->uiFlags & SMK_FLIC_OPEN)) return i;
	}
	return NULL;
}


static void SmkSetupVideo(void)
{
	UINT32 red;
	UINT32 green;
	UINT32 blue;
	GetPrimaryRGBDistributionMasks(&red, &green, &blue);
	if (red == 0xf800 && green == 0x07e0 && blue == 0x001f)
	{
		guiSmackPixelFormat = SMACKBUFFER565;
	}
	else
	{
		guiSmackPixelFormat = SMACKBUFFER555;
	}
}
