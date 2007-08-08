#include "Debug.h"
#include "Fade_Screen.h"
#include "FileMan.h"
#include "ImpTGA.h"
#include "Input.h"
#include "Local.h"
#include "MemMan.h"
#include "RenderWorld.h"
#include "Render_Dirty.h"
#include "SGP.h"
#include "Timer.h"
#include "Timer_Control.h"
#include "Types.h"
#include "VObject_Blitters.h"
#include "Video.h"
#include <SDL.h>
#include <stdarg.h>
#include "Stubs.h" // XXX


#define MAX_DIRTY_REGIONS     128

#define VIDEO_OFF             0x00
#define VIDEO_ON              0x01
#define VIDEO_SHUTTING_DOWN   0x02
#define VIDEO_SUSPENDED       0x04


typedef struct
{
	BOOLEAN fRestore;
	SDL_Rect rect;
} MouseCursorBackground;


#define MAX_NUM_FRAMES 25

static BOOLEAN gfVideoCapture = FALSE;
static UINT32  guiFramePeriod = 1000 / 15;
static UINT32  guiLastFrame;
static UINT16* gpFrameData[MAX_NUM_FRAMES];
INT32 giNumFrames = 0;


// Globals for mouse cursor
static UINT16 gusMouseCursorWidth;
static UINT16 gusMouseCursorHeight;
static INT16  gsMouseCursorXOffset;
static INT16  gsMouseCursorYOffset;

static MouseCursorBackground  gMouseCursorBackground;

// Refresh thread based variables
static UINT32 guiFrameBufferState;  // BUFFER_READY, BUFFER_DIRTY
static UINT32 guiMouseBufferState;  // BUFFER_READY, BUFFER_DISABLED
static UINT32 guiVideoManagerState; // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED, VIDEO_SHUTTING_DOWN

// Dirty rectangle management variables
static SGPRect gListOfDirtyRegions[MAX_DIRTY_REGIONS];
static UINT32  guiDirtyRegionCount;
static BOOLEAN gfForceFullScreenRefresh;


static SGPRect gDirtyRegionsEx[MAX_DIRTY_REGIONS];
static UINT32  guiDirtyRegionExCount;

// Screen output stuff
static BOOLEAN gfPrintFrameBuffer;
static UINT32  guiPrintFrameBufferIndex;


extern UINT16 gusRedMask;
extern UINT16 gusGreenMask;
extern UINT16 gusBlueMask;
extern INT16  gusRedShift;
extern INT16  gusBlueShift;
extern INT16  gusGreenShift;


static SDL_Surface* MouseCursor;
static SDL_Surface* FrameBuffer;
static SDL_Surface* ScreenBuffer;


static BOOLEAN Fullscreen = FALSE;


void VideoSetFullScreen(BOOLEAN Enable)
{
	Fullscreen = Enable;
}


static BOOLEAN GetRGBDistribution(void);


BOOLEAN InitializeVideoManager(void)
{
	DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Initializing the video manager");

	SDL_WM_SetCaption(APPLICATION_NAME, NULL);

	UINT32 VideoFlags = SDL_SWSURFACE | SDL_HWPALETTE;
	if (Fullscreen) VideoFlags |= SDL_FULLSCREEN;
	ScreenBuffer = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH, VideoFlags);
	if (ScreenBuffer == NULL)
	{
		DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Failed to set up video mode");
		return FALSE;
	}

	Uint32 Rmask = ScreenBuffer->format->Rmask;
	Uint32 Gmask = ScreenBuffer->format->Gmask;
	Uint32 Bmask = ScreenBuffer->format->Bmask;
	Uint32 Amask = ScreenBuffer->format->Amask;

	FrameBuffer = SDL_CreateRGBSurface(
		SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH,
		Rmask, Gmask, Bmask, Amask
	);

	MouseCursor = SDL_CreateRGBSurface(
		SDL_SWSURFACE, MAX_CURSOR_WIDTH, MAX_CURSOR_HEIGHT, PIXEL_DEPTH,
		Rmask, Gmask, Bmask, Amask
	);
	SDL_SetColorKey(MouseCursor, SDL_SRCCOLORKEY, 0);

	memset(gpFrameData, 0, sizeof(gpFrameData));

	SDL_ShowCursor(SDL_DISABLE);

	FillSurface(FRAME_BUFFER, 0);

	gMouseCursorBackground.fRestore = FALSE;

	// Initialize state variables
	guiFrameBufferState      = BUFFER_DIRTY;
	guiMouseBufferState      = BUFFER_DISABLED;
	guiVideoManagerState     = VIDEO_ON;
	guiDirtyRegionCount      = 0;
	gfForceFullScreenRefresh = TRUE;
	gfPrintFrameBuffer       = FALSE;
	guiPrintFrameBufferIndex = 0;

	// This function must be called to setup RGB information
	GetRGBDistribution();

	return TRUE;
}


void ShutdownVideoManager(void)
{
	DebugMsg(TOPIC_VIDEO, DBG_LEVEL_0, "Shutting down the video manager");

	/* Toggle the state of the video manager to indicate to the refresh thread
	 * that it needs to shut itself down */

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	guiVideoManagerState = VIDEO_OFF;

	// ATE: Release mouse cursor!
	FreeMouseCursor();
}


void SuspendVideoManager(void)
{
	guiVideoManagerState = VIDEO_SUSPENDED;
}


BOOLEAN RestoreVideoManager(void)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	// Make sure the video manager is indeed suspended before moving on

	if (guiVideoManagerState == VIDEO_SUSPENDED)
	{
		// Set the video state to VIDEO_ON

		guiFrameBufferState = BUFFER_DIRTY;
		guiMouseBufferState = BUFFER_READY;
		gfForceFullScreenRefresh = TRUE;
		guiVideoManagerState = VIDEO_ON;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
#endif
}


void InvalidateRegion(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom)
{
	if (gfForceFullScreenRefresh)
	{
		// There's no point in going on since we are forcing a full screen refresh
		return;
	}

	if (guiDirtyRegionCount < MAX_DIRTY_REGIONS)
	{
		// Well we haven't broken the MAX_DIRTY_REGIONS limit yet, so we register the new region

		// DO SOME PREMIMARY CHECKS FOR VALID RECTS
		if (iLeft < 0) iLeft = 0;
		if (iTop  < 0) iTop  = 0;

		if (iRight  > SCREEN_WIDTH)  iRight  = SCREEN_WIDTH;
		if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

		if (iRight - iLeft <= 0) return;
		if (iBottom - iTop <= 0) return;

		gListOfDirtyRegions[guiDirtyRegionCount].iLeft   = iLeft;
		gListOfDirtyRegions[guiDirtyRegionCount].iTop    = iTop;
		gListOfDirtyRegions[guiDirtyRegionCount].iRight  = iRight;
		gListOfDirtyRegions[guiDirtyRegionCount].iBottom = iBottom;
		guiDirtyRegionCount++;
	}
	else
	{
		// The MAX_DIRTY_REGIONS limit has been exceeded. Therefore we arbitrarely invalidate the entire
		// screen and force a full screen refresh
		guiDirtyRegionExCount = 0;
		guiDirtyRegionCount = 0;
		gfForceFullScreenRefresh = TRUE;
	}
}


static void AddRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);


void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom)
{
	// Check if we are spanning the rectangle - if so slit it up!
	if (iTop <= gsVIEWPORT_WINDOW_END_Y && iBottom > gsVIEWPORT_WINDOW_END_Y)
	{
		// Add new top region
		AddRegionEx(iLeft, iTop, iRight, gsVIEWPORT_WINDOW_END_Y);

		// Add new bottom region
		AddRegionEx(iLeft, gsVIEWPORT_WINDOW_END_Y, iRight, iBottom);
	}
	else
	{
		AddRegionEx(iLeft, iTop, iRight, iBottom);
	}
}


static void AddRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom)
{
	if (guiDirtyRegionExCount < MAX_DIRTY_REGIONS)
	{
		// DO SOME PRELIMINARY CHECKS FOR VALID RECTS
		if (iLeft < 0) iLeft = 0;
		if (iTop  < 0) iTop  = 0;

		if (iRight  > SCREEN_WIDTH)  iRight  = SCREEN_WIDTH;
		if (iBottom > SCREEN_HEIGHT) iBottom = SCREEN_HEIGHT;

		if (iRight - iLeft <= 0) return;
		if (iBottom - iTop <= 0) return;

		gDirtyRegionsEx[guiDirtyRegionExCount].iLeft   = iLeft;
		gDirtyRegionsEx[guiDirtyRegionExCount].iTop    = iTop;
		gDirtyRegionsEx[guiDirtyRegionExCount].iRight  = iRight;
		gDirtyRegionsEx[guiDirtyRegionExCount].iBottom = iBottom;
		guiDirtyRegionExCount++;
	}
	else
	{
		guiDirtyRegionExCount = 0;
		guiDirtyRegionCount = 0;
		gfForceFullScreenRefresh = TRUE;
	}
}


void InvalidateScreen(void)
{
	// W A R N I N G ---- W A R N I N G ---- W A R N I N G ---- W A R N I N G ---- W A R N I N G ----
	//
	// This function is intended to be called by a thread which has already locked the
	// FRAME_BUFFER_MUTEX mutual exclusion section. Anything else will cause the application to
	// yack

	guiDirtyRegionCount = 0;
	guiDirtyRegionExCount = 0;
	gfForceFullScreenRefresh = TRUE;
	guiFrameBufferState = BUFFER_DIRTY;
}


//#define SCROLL_TEST

static void ScrollJA2Background(UINT32 uiDirection, INT16 sScrollXIncrement, INT16 sScrollYIncrement)
{
	SDL_Surface* Frame  = FrameBuffer;
	SDL_Surface* Source = ScreenBuffer; // Primary
	SDL_Surface* Dest   = ScreenBuffer; // Back
	SDL_Rect SrcRect;
	SDL_Rect DstRect;

	const UINT16 usWidth  = SCREEN_WIDTH;
	const UINT16 usHeight = gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y;

	SGPRect StripRegions[2];
	StripRegions[0].iLeft   = gsVIEWPORT_START_X;
	StripRegions[0].iRight  = gsVIEWPORT_END_X;
	StripRegions[0].iTop    = gsVIEWPORT_WINDOW_START_Y;
	StripRegions[0].iBottom = gsVIEWPORT_WINDOW_END_Y;
	StripRegions[1].iLeft   = gsVIEWPORT_START_X;
	StripRegions[1].iRight  = gsVIEWPORT_END_X;
	StripRegions[1].iTop    = gsVIEWPORT_WINDOW_START_Y;
	StripRegions[1].iBottom = gsVIEWPORT_WINDOW_END_Y;

	UINT16 usNumStrips = 0;
	switch (uiDirection)
	{
		case SCROLL_LEFT:
			SrcRect.x = 0;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
			SrcRect.w = usWidth - sScrollXIncrement;
			SrcRect.h = usHeight;
			DstRect.x = sScrollXIncrement;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iRight = gsVIEWPORT_START_X + sScrollXIncrement;
			usNumStrips = 1;
			break;

		case SCROLL_RIGHT:
			SrcRect.x = sScrollXIncrement;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
			SrcRect.w = usWidth - sScrollXIncrement;
			SrcRect.h = usHeight;
			DstRect.x = 0;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iLeft = gsVIEWPORT_END_X - sScrollXIncrement;
			usNumStrips = 1;
			break;

		case SCROLL_UP:
			SrcRect.x = 0;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
			SrcRect.w = usWidth;
			SrcRect.h = usHeight - sScrollYIncrement;
			DstRect.x = 0;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iBottom = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			usNumStrips = 1;
			break;

		case SCROLL_DOWN:
			SrcRect.x = 0;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			SrcRect.w = usWidth;
			SrcRect.h = usHeight - sScrollYIncrement;
			DstRect.x = 0;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iTop = gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement;
			usNumStrips = 1;
			break;

		case SCROLL_UPLEFT:
			SrcRect.x = 0;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
			SrcRect.w = usWidth - sScrollXIncrement;
			SrcRect.h = usHeight - sScrollYIncrement;
			DstRect.x = sScrollXIncrement;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iRight  = gsVIEWPORT_START_X        + sScrollXIncrement;
			StripRegions[1].iBottom = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			StripRegions[1].iLeft   = gsVIEWPORT_START_X        + sScrollXIncrement;
			usNumStrips = 2;
			break;

		case SCROLL_UPRIGHT:
			SrcRect.x = sScrollXIncrement;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
			SrcRect.w = usWidth - sScrollXIncrement;
			SrcRect.h = usHeight - sScrollYIncrement;
			DstRect.x = 0;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iLeft   = gsVIEWPORT_END_X          - sScrollXIncrement;
			StripRegions[1].iBottom = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			StripRegions[1].iRight  = gsVIEWPORT_END_X          - sScrollXIncrement;
			usNumStrips = 2;
			break;

		case SCROLL_DOWNLEFT:
			SrcRect.x = 0;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			SrcRect.w = usWidth - sScrollXIncrement;
			SrcRect.h = usHeight - sScrollYIncrement;
			DstRect.x = sScrollXIncrement;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iRight = gsVIEWPORT_START_X      + sScrollXIncrement;
			StripRegions[1].iTop   = gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement;
			StripRegions[1].iLeft  = gsVIEWPORT_START_X      + sScrollXIncrement;
			usNumStrips = 2;
			break;

		case SCROLL_DOWNRIGHT:
			SrcRect.x = sScrollXIncrement;
			SrcRect.y = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
			SrcRect.w = usWidth - sScrollXIncrement;
			SrcRect.h = usHeight - sScrollYIncrement;
			DstRect.x = 0;
			DstRect.y = gsVIEWPORT_WINDOW_START_Y;
			SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

			StripRegions[0].iLeft  = gsVIEWPORT_END_X        - sScrollXIncrement;
			StripRegions[1].iTop   = gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement;
			StripRegions[1].iRight = gsVIEWPORT_END_X        - sScrollXIncrement;
			usNumStrips = 2;
			break;

	}

#ifdef SCROLL_TEST
	SDL_FillRect(Dest, NULL, 0);
#endif

	for (UINT cnt = 0; cnt < usNumStrips; cnt++)
	{
		UINT x = StripRegions[cnt].iLeft;
		UINT y = StripRegions[cnt].iTop;
		UINT w = StripRegions[cnt].iRight - StripRegions[cnt].iLeft;
		UINT b = StripRegions[cnt].iBottom;
		for (; y < b; y++)
		{
			memset(gpZBuffer + y * SCREEN_WIDTH + x, 0, w * sizeof(*gpZBuffer));
		}

		RenderStaticWorldRect(StripRegions[cnt].iLeft, StripRegions[cnt].iTop, StripRegions[cnt].iRight, StripRegions[cnt].iBottom, TRUE);

		SrcRect.x = StripRegions[cnt].iLeft;
		SrcRect.y = StripRegions[cnt].iTop;
		SrcRect.w = StripRegions[cnt].iRight  - StripRegions[cnt].iLeft;
		SrcRect.h = StripRegions[cnt].iBottom - StripRegions[cnt].iTop;
		DstRect.x = StripRegions[cnt].iLeft;
		DstRect.y = StripRegions[cnt].iTop;
		SDL_BlitSurface(Frame, &SrcRect, Dest, &DstRect);
	}

	INT16 sShiftX = 0;
	INT16 sShiftY = 0;

	switch (uiDirection)
	{
		case SCROLL_LEFT:
			sShiftX = sScrollXIncrement;
			sShiftY = 0;
			break;

		case SCROLL_RIGHT:
			sShiftX = -sScrollXIncrement;
			sShiftY = 0;
			break;

		case SCROLL_UP:
			sShiftX = 0;
			sShiftY = sScrollYIncrement;
			break;

		case SCROLL_DOWN:
			sShiftX = 0;
			sShiftY = -sScrollYIncrement;
			break;

		case SCROLL_UPLEFT:
			sShiftX = sScrollXIncrement;
			sShiftY = sScrollYIncrement;
			break;

		case SCROLL_UPRIGHT:
			sShiftX = -sScrollXIncrement;
			sShiftY = sScrollYIncrement;
			break;

		case SCROLL_DOWNLEFT:
			sShiftX = sScrollXIncrement;
			sShiftY = -sScrollYIncrement;
			break;

		case SCROLL_DOWNRIGHT:
			sShiftX = -sScrollXIncrement;
			sShiftY = -sScrollYIncrement;
			break;
	}

	// RESTORE SHIFTED
	RestoreShiftedVideoOverlays(sShiftX, sShiftY);

	// SAVE NEW
	SaveVideoOverlaysArea(BACKBUFFER);

	// BLIT NEW
	ExecuteVideoOverlaysToAlternateBuffer(BACKBUFFER);

	SDL_UpdateRect
	(
		Dest,
		gsVIEWPORT_START_X,
		gsVIEWPORT_WINDOW_START_Y,
		gsVIEWPORT_END_X - gsVIEWPORT_START_X,
		gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y
	);
}


static void TakeScreenshot(void)
{
	const char* ExecDir = GetExecutableDirectory();
	char FileName[2048];
	sprintf(FileName, "%s/SCREEN%03d.TGA", ExecDir, guiPrintFrameBufferIndex++);
	FILE* OutputFile = fopen(FileName, "wb");
	if (OutputFile == NULL) return;

	fprintf(OutputFile, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x80, 0x02, 0xe0, 0x01, 0x10, 0);

	// Copy 16 bit buffer to file

	// 5/6/5.. create buffer...
	UINT16* p16BPPData;
	if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F)
	{
		p16BPPData = MemAlloc(SCREEN_WIDTH * 2);
	}

	for (INT32 iIndex = SCREEN_HEIGHT - 1; iIndex >= 0; iIndex--)
	{
		// ATE: OK, fix this such that it converts pixel format to 5/5/5
		// if current settings are 5/6/5....
		if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F)
		{
			// Read into a buffer...
			memcpy(p16BPPData, (UINT16*)ScreenBuffer->pixels + iIndex * SCREEN_WIDTH, SCREEN_WIDTH * 2);

			// Convert....
			ConvertRGBDistribution565To555(p16BPPData, SCREEN_WIDTH);

			// Write
			fwrite(p16BPPData, SCREEN_WIDTH * 2, 1, OutputFile);
		}
		else
		{
			fwrite((UINT16*)ScreenBuffer->pixels + iIndex * SCREEN_WIDTH, SCREEN_WIDTH * 2, 1, OutputFile);
		}
	}

	// 5/6/5.. Delete buffer...
	if (gusRedMask == 0xF800 && gusGreenMask == 0x07E0 && gusBlueMask == 0x001F)
	{
		MemFree(p16BPPData);
	}

	fclose(OutputFile);
}


static void SnapshotSmall(void);


void RefreshScreen(void)
{
	if (guiVideoManagerState != VIDEO_ON) return;

	SDL_Rect OldMouseRect;
	BOOLEAN UndrawMouse = gMouseCursorBackground.fRestore;
	if (UndrawMouse)
	{
		OldMouseRect = gMouseCursorBackground.rect;
		SDL_BlitSurface(FrameBuffer, &OldMouseRect, ScreenBuffer, &OldMouseRect);
	}

	if (guiFrameBufferState == BUFFER_DIRTY)
	{
		if (gfFadeInitialized && gfFadeInVideo)
		{
			gFadeFunction();
		}
		else
		{
			if (gfForceFullScreenRefresh)
			{
				SDL_BlitSurface(FrameBuffer, NULL, ScreenBuffer, NULL);
			}
			else
			{
				for (UINT32 i = 0; i < guiDirtyRegionCount; i++)
				{
					SDL_Rect SrcRect;
					SrcRect.x = gListOfDirtyRegions[i].iLeft;
					SrcRect.y = gListOfDirtyRegions[i].iTop;
					SrcRect.w = gListOfDirtyRegions[i].iRight  - gListOfDirtyRegions[i].iLeft;
					SrcRect.h = gListOfDirtyRegions[i].iBottom - gListOfDirtyRegions[i].iTop;
					SDL_BlitSurface(FrameBuffer, &SrcRect, ScreenBuffer, &SrcRect);
				}

				for (UINT32 i = 0; i < guiDirtyRegionExCount; i++)
				{
					SDL_Rect SrcRect;
					SrcRect.x = gDirtyRegionsEx[i].iLeft;
					SrcRect.y = gDirtyRegionsEx[i].iTop;
					SrcRect.w = gDirtyRegionsEx[i].iRight  - gDirtyRegionsEx[i].iLeft;
					SrcRect.h = gDirtyRegionsEx[i].iBottom - gDirtyRegionsEx[i].iTop;
					if (gfRenderScroll)
					{
						// Check if we are completely out of bounds
						if (SrcRect.y <= gsVIEWPORT_WINDOW_END_Y && SrcRect.y + SrcRect.h <= gsVIEWPORT_WINDOW_END_Y)
						{
							continue;
						}
					}
					SDL_BlitSurface(FrameBuffer, &SrcRect, ScreenBuffer, &SrcRect);
				}
			}
		}
		if (gfRenderScroll)
		{
			ScrollJA2Background(guiScrollDirection, gsScrollXIncrement, gsScrollYIncrement);
		}
		gfIgnoreScrollDueToCenterAdjust = FALSE;
		guiFrameBufferState = BUFFER_READY;
	}

	if (gfVideoCapture)
	{
		UINT32 uiTime = GetClock();
		if (uiTime < guiLastFrame || uiTime > guiLastFrame + guiFramePeriod)
		{
			SnapshotSmall();
			guiLastFrame = uiTime;
		}
	}

	if (gfPrintFrameBuffer)
	{
		TakeScreenshot();
		gfPrintFrameBuffer = FALSE;
	}

	if (guiMouseBufferState == BUFFER_READY)
	{
		POINT MousePos;
		GetCursorPos(&MousePos);
		SDL_Rect src;
		src.x = 0;
		src.y = 0;
		src.w = gusMouseCursorWidth;
		src.h = gusMouseCursorHeight;
		SDL_Rect dst;
		dst.x = MousePos.x - gsMouseCursorXOffset;
		dst.y = MousePos.y - gsMouseCursorYOffset;
		SDL_BlitSurface(MouseCursor, &src, ScreenBuffer, &dst);
		SDL_UpdateRects(ScreenBuffer, 1, &dst);
		gMouseCursorBackground.rect     = dst;
		gMouseCursorBackground.fRestore = TRUE;
	}
	else
	{
		gMouseCursorBackground.fRestore = FALSE;
	}

	if (UndrawMouse)
	{
		SDL_UpdateRects(ScreenBuffer, 1, &OldMouseRect);
	}

	if (gfForceFullScreenRefresh)
	{
		SDL_UpdateRect(ScreenBuffer, 0, 0, 0, 0);
	}
	else
	{
		for (UINT32 i = 0; i < guiDirtyRegionCount; i++)
		{
			SDL_Rect SrcRect;
			SrcRect.x = gListOfDirtyRegions[i].iLeft;
			SrcRect.y = gListOfDirtyRegions[i].iTop;
			SrcRect.w = gListOfDirtyRegions[i].iRight  - gListOfDirtyRegions[i].iLeft;
			SrcRect.h = gListOfDirtyRegions[i].iBottom - gListOfDirtyRegions[i].iTop;
			SDL_UpdateRects(ScreenBuffer, 1, &SrcRect);
		}

		for (UINT32 i = 0; i < guiDirtyRegionExCount; i++)
		{
			SDL_Rect SrcRect;
			SrcRect.x = gDirtyRegionsEx[i].iLeft;
			SrcRect.y = gDirtyRegionsEx[i].iTop;
			SrcRect.w = gDirtyRegionsEx[i].iRight  - gDirtyRegionsEx[i].iLeft;
			SrcRect.h = gDirtyRegionsEx[i].iBottom - gDirtyRegionsEx[i].iTop;
			if (gfRenderScroll)
			{
				if (SrcRect.y <= gsVIEWPORT_WINDOW_END_Y && SrcRect.y + SrcRect.h <= gsVIEWPORT_WINDOW_END_Y)
				{
					continue;
				}
			}
			SDL_UpdateRects(ScreenBuffer, 1, &SrcRect);
		}
	}

	if (gfRenderScroll)
	{
		gfRenderScroll = FALSE;
	}

	gfForceFullScreenRefresh = FALSE;
	guiDirtyRegionCount = 0;
	guiDirtyRegionExCount = 0;
}


SDL_Surface* GetBackBufferObject(void)
{
	Assert(ScreenBuffer != NULL);
	return ScreenBuffer;
}


SDL_Surface* GetFrameBufferObject(void)
{
	Assert(FrameBuffer != NULL);
	return FrameBuffer;
}


SDL_Surface* GetMouseBufferObject(void)
{
	Assert(MouseCursor != NULL);
	return MouseCursor;
}


PTR LockBackBuffer(UINT32 *uiPitch)
{
	*uiPitch = ScreenBuffer->pitch;
	return ScreenBuffer->pixels;
}


void UnlockBackBuffer(void)
{
}


PTR LockFrameBuffer(UINT32 *uiPitch)
{
	*uiPitch = FrameBuffer->pitch;
	return FrameBuffer->pixels;
}


void UnlockFrameBuffer(void)
{
}


PTR LockMouseBuffer(UINT32 *uiPitch)
{
	*uiPitch = MouseCursor->pitch;
	return MouseCursor->pixels;
}


void UnlockMouseBuffer(void)
{
}


static BOOLEAN GetRGBDistribution(void)
{
	UINT16 usBit;

	gusRedMask   = ScreenBuffer->format->Rmask;
	gusGreenMask = ScreenBuffer->format->Gmask;
	gusBlueMask  = ScreenBuffer->format->Bmask;

	// RGB 5,5,5
	if((gusRedMask==0x7c00) && (gusGreenMask==0x03e0) && (gusBlueMask==0x1f))
		guiTranslucentMask=0x3def;
	// RGB 5,6,5
	else// if((gusRedMask==0xf800) && (gusGreenMask==0x03e0) && (gusBlueMask==0x1f))
		guiTranslucentMask=0x7bef;


	usBit = 0x8000;
	gusRedShift = 8;
	while(!(gusRedMask & usBit))
	{
		usBit >>= 1;
		gusRedShift--;
	}

	usBit = 0x8000;
	gusGreenShift = 8;
	while(!(gusGreenMask & usBit))
	{
		usBit >>= 1;
		gusGreenShift--;
	}

	usBit = 0x8000;
	gusBlueShift = 8;
	while(!(gusBlueMask & usBit))
	{
		usBit >>= 1;
		gusBlueShift--;
	}

	return TRUE;
}


BOOLEAN GetPrimaryRGBDistributionMasks(UINT32 *RedBitMask, UINT32 *GreenBitMask, UINT32 *BlueBitMask)
{
	*RedBitMask   = gusRedMask;
	*GreenBitMask = gusGreenMask;
	*BlueBitMask  = gusBlueMask;

	return TRUE;
}


void SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight, UINT16 usCursorWidth)
{
	gsMouseCursorXOffset = sOffsetX;
	gsMouseCursorYOffset = sOffsetY;
	gusMouseCursorWidth  = usCursorWidth;
	gusMouseCursorHeight = usCursorHeight;
	guiMouseBufferState  = BUFFER_READY;
}


void StartFrameBufferRender(void)
{
}


void EndFrameBufferRender(void)
{
	guiFrameBufferState = BUFFER_DIRTY;
}


void PrintScreen(void)
{
	gfPrintFrameBuffer = TRUE;
}


void FatalError(const char *pError, ...)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	gfProgramIsRunning = FALSE;

	char gFatalErrorString[512];
	va_list argptr;
	va_start(argptr, pError);
	vsprintf(gFatalErrorString, pError, argptr);
	va_end(argptr);
	fprintf(stderr, "FATAL ERROR: %s\n", gFatalErrorString);
}


/*******************************************************************************
 * SnapshotSmall
 *
 * Grabs a screen from the primary surface, and stuffs it into a 16-bit
 * (RGB 5,5,5), uncompressed Targa file. Each time the routine is called, it
 * increments the file number by one. The files are create in the current
 * directory, usually the EXE directory. This routine produces 1/4 sized images.
 *
 ******************************************************************************/

#pragma pack (push, 1)

typedef struct {
	UINT8  ubIDLength;
	UINT8  ubColorMapType;
	UINT8  ubTargaType;
	UINT16 usColorMapOrigin;
	UINT16 usColorMapLength;
	UINT8  ubColorMapEntrySize;
	UINT16 usOriginX;
	UINT16 usOriginY;
	UINT16 usImageWidth;
	UINT16 usImageHeight;
	UINT8  ubBitsPerPixel;
	UINT8  ubImageDescriptor;
} TARGA_HEADER;

#pragma pack (pop)


static void RefreshMovieCache(void);


static void SnapshotSmall(void)
{
	// Get the write pointer
	const UINT16* pVideo = (UINT16*)ScreenBuffer->pixels;

	UINT16* pDest = gpFrameData[giNumFrames];

	for (INT32 iCountY = SCREEN_HEIGHT - 1; iCountY >= 0; iCountY--)
	{
		for (INT32 iCountX = 0; iCountX < SCREEN_WIDTH; iCountX++)
		{
			pDest[iCountY * SCREEN_WIDTH + iCountX] = pVideo[iCountY * SCREEN_WIDTH + iCountX];
		}
	}

	giNumFrames++;

	if (giNumFrames == MAX_NUM_FRAMES) RefreshMovieCache();
}


void VideoCaptureToggle(void)
{
#ifdef JA2TESTVERSION
	gfVideoCapture = !gfVideoCapture;
	if (gfVideoCapture)
	{
		for (INT32 cnt = 0; cnt < MAX_NUM_FRAMES; cnt++)
		{
			gpFrameData[cnt] = MemAlloc(SCREEN_WIDTH * SCREEN_HEIGHT * 2);
		}
		guiLastFrame = GetClock();
	}
	else
	{
		RefreshMovieCache();

		for (INT32 cnt = 0; cnt < MAX_NUM_FRAMES; cnt++)
		{
			if (gpFrameData[cnt] != NULL) MemFree(gpFrameData[cnt]);
		}
	}
	giNumFrames = 0;
#endif
}


static void RefreshMovieCache(void)
{
	static UINT32 uiPicNum = 0;

	PauseTime(TRUE);

	const char* ExecDir = GetExecutableDirectory();

	for (INT32 cnt = 0; cnt < giNumFrames; cnt++)
	{
		CHAR8 cFilename[2048];
		sprintf(cFilename, "%s/JA%5.5d.TGA", ExecDir, uiPicNum++);

		FILE* disk = fopen(cFilename, "wb");
		if (disk == NULL) return;

		TARGA_HEADER Header;
		memset(&Header, 0, sizeof(TARGA_HEADER));
		Header.ubTargaType    = 2; // Uncompressed 16/24/32 bit
		Header.usImageWidth   = SCREEN_WIDTH;
		Header.usImageHeight  = SCREEN_HEIGHT;
		Header.ubBitsPerPixel = 16;

		fwrite(&Header, sizeof(TARGA_HEADER), 1, disk);

		UINT16* pDest = gpFrameData[cnt];

		for (INT32 iCountY = SCREEN_HEIGHT - 1; iCountY >= 0; iCountY -= 1)
		{
			for (INT32 iCountX = 0; iCountX < SCREEN_WIDTH; iCountX ++)
			{
				fwrite(pDest + iCountY * SCREEN_WIDTH + iCountX, sizeof(UINT16), 1, disk);
			}
		}

		fclose(disk);
	}

	PauseTime(FALSE);

	giNumFrames = 0;
}
