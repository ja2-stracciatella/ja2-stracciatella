#include "ContentManager.h"
#include "Cursor_Control.h"
#include "Debug.h"
#include "Fade_Screen.h"
#include "FileMan.h"
#include "GameInstance.h"
#include "HImage.h"
#include "Input.h"
#include "Local.h"
#include "Logger.h"
#include "RenderWorld.h"
#include "Render_Dirty.h"
#include "Timer.h"
#include "Timer_Control.h"
#include "Types.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "UILayout.h"
#include "Font.h"
#include "Icon.h"
#include <algorithm>
#include <ctime>
#include <stdexcept>

#define BUFFER_READY      0x00
#define BUFFER_DIRTY      0x02

#define MAX_CURSOR_WIDTH  64
#define MAX_CURSOR_HEIGHT 64

#define MAX_DIRTY_REGIONS 128

#define VIDEO_OFF         0x00
#define VIDEO_ON          0x01
#define VIDEO_SUSPENDED   0x04

#define RED_MASK 0xF800
#define GREEN_MASK 0x07E0
#define BLUE_MASK 0x001F
#define ALPHA_MASK 0

#define OVERSAMPLING_SCALE 4


// Globals for mouse cursor
static UINT16 gusMouseCursorWidth;
static UINT16 gusMouseCursorHeight;
static INT16  gsMouseCursorXOffset;
static INT16  gsMouseCursorYOffset;
INT16 gsMouseSizeYModifier = 0; // This can increase the size of gusMouseCursorHeight so image data (ie, text) outside the normal height of the mouse cursor can be copied onto screen buffer

static SDL_Rect MouseBackground = { 0, 0, 0, 0 };

// Refresh thread based variables
static UINT32 guiFrameBufferState;  // BUFFER_READY, BUFFER_DIRTY
static UINT32 guiVideoManagerState; // VIDEO_ON, VIDEO_OFF, VIDEO_SUSPENDED

// Dirty rectangle management variables
static SDL_Rect DirtyRegions[MAX_DIRTY_REGIONS];
static UINT32   guiDirtyRegionCount;
static BOOLEAN  gfForceFullScreenRefresh;


static SDL_Rect DirtyRegionsEx[MAX_DIRTY_REGIONS];
static UINT32   guiDirtyRegionExCount;


static SDL_Surface* MouseCursor;
static SDL_Surface* FrameBuffer;
static SDL_Renderer*  GameRenderer;
SDL_Window* g_game_window;

static SDL_Surface* ScreenBuffer;
static SDL_Texture* ScreenTexture;
static SDL_Texture* ScaledScreenTexture;
static Uint32       g_window_flags = 0;
static VideoScaleQuality ScaleQuality = VideoScaleQuality::LINEAR;

static void RecreateBackBuffer();
static void DeletePrimaryVideoSurfaces(void);

// returns if desktop resolution larger game resolution
BOOLEAN IsDesktopLargeEnough()
{
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) == 0)
	{
		if (dm.w < SCREEN_WIDTH || dm.h < SCREEN_HEIGHT)
		{
			return false;
		}
	}
	return true;
}

void VideoSetFullScreen(const BOOLEAN enable)
{
	if (enable)
	{
		g_window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	else
	{
		g_window_flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
}

void VideoToggleFullScreen(void)
{
	if (SDL_GetWindowFlags(g_game_window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
	{
		SDL_SetWindowFullscreen(g_game_window, 0);
	}
	else
	{
		SDL_SetWindowFullscreen(g_game_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
}

void VideoSetBrightness(float brightness)
{
	if (brightness >= 0)
	{
		// Do not set the brightness unless explicitly requested
		// On Windows, setting the brightness resets color profile and some other disply options.
		SDL_SetWindowBrightness(g_game_window, brightness);
	}
}


static void GetRGBDistribution();


void InitializeVideoManager(const VideoScaleQuality quality)
{
	SLOGD("Initializing the video manager");
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	ScaleQuality = quality;
	g_window_flags |= SDL_WINDOW_RESIZABLE;

	g_game_window = SDL_CreateWindow(APPLICATION_NAME,
					SDL_WINDOWPOS_UNDEFINED,
					SDL_WINDOWPOS_UNDEFINED,
					SCREEN_WIDTH, SCREEN_HEIGHT,
					g_window_flags);

	GameRenderer = SDL_CreateRenderer(g_game_window, -1, 0);
	SDL_RenderSetLogicalSize(GameRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_Surface* windowIcon = SDL_CreateRGBSurfaceFrom(
			(void*)gWindowIconData.pixel_data,
			gWindowIconData.width,
			gWindowIconData.height,
			gWindowIconData.bytes_per_pixel*8,
			gWindowIconData.bytes_per_pixel*gWindowIconData.width,
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	SDL_SetWindowIcon(g_game_window, windowIcon);
	SDL_FreeSurface(windowIcon);


	ClippingRect.set(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	ScreenBuffer = SDL_CreateRGBSurface(
					0,
					SCREEN_WIDTH,
					SCREEN_HEIGHT,
					PIXEL_DEPTH,
					RED_MASK,
					GREEN_MASK,
					BLUE_MASK,
					ALPHA_MASK
	);

	if (ScreenBuffer == NULL) {
		SLOGE("SDL_CreateRGBSurface for ScreenBuffer failed: {}\n", SDL_GetError());
	}


	if (ScaleQuality == VideoScaleQuality::PERFECT)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
#if SDL_VERSION_ATLEAST(2,0,5)
		if (!IsDesktopLargeEnough())
		{
			// Pixel-perfect mode cannot handle scaling down, and will
			// result in a empty black screen if the window size is
			// smaller than logical render resolution.
			throw std::runtime_error("Game resolution must not be larger than desktop size. "
				"Please reduce game resolution or choose another scaling mode.");
		}
		SDL_SetWindowMinimumSize(g_game_window, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_RenderSetIntegerScale(GameRenderer, SDL_TRUE);
#else
		SLOGW("Pixel-perfect scaling is not available");
		ScaleQuality = VideoScaleQuality::NEAR_PERFECT;
#endif
	}
	else if (ScaleQuality == VideoScaleQuality::NEAR_PERFECT)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		ScaledScreenTexture = SDL_CreateTexture(GameRenderer,
			SDL_PIXELFORMAT_RGB565,
			SDL_TEXTUREACCESS_TARGET,
			SCREEN_WIDTH * OVERSAMPLING_SCALE, SCREEN_HEIGHT * OVERSAMPLING_SCALE);

		if (ScaledScreenTexture == NULL)
		{
			SLOGE("SDL_CreateTexture for ScaledScreenTexture failed: {}\n", SDL_GetError());
		}

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	}
	else
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	}

	ScreenTexture = SDL_CreateTexture(GameRenderer,
					SDL_PIXELFORMAT_RGB565,
					SDL_TEXTUREACCESS_STREAMING,
					SCREEN_WIDTH, SCREEN_HEIGHT);

	if (ScreenTexture == NULL) {
		SLOGE("SDL_CreateTexture for ScreenTexture failed: {}\n", SDL_GetError());
	}

	FrameBuffer = SDL_CreateRGBSurface(
		SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);

	if (FrameBuffer == NULL)
	{
		SLOGE("SDL_CreateRGBSurface for FrameBuffer failed: {}\n", SDL_GetError());
	}

	MouseCursor = SDL_CreateRGBSurface(
		0, MAX_CURSOR_WIDTH, MAX_CURSOR_HEIGHT, PIXEL_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	SDL_SetColorKey(MouseCursor, SDL_TRUE, 0);

	if (MouseCursor == NULL)
	{
		SLOGE("SDL_CreateRGBSurface for MouseCursor failed: {}\n", SDL_GetError());
	}

	SDL_ShowCursor(SDL_DISABLE);

	// Initialize state variables
	guiFrameBufferState      = BUFFER_DIRTY;
	guiVideoManagerState     = VIDEO_ON;
	guiDirtyRegionCount      = 0;
	gfForceFullScreenRefresh = TRUE;

	// This function must be called to setup RGB information
	GetRGBDistribution();
}


void ShutdownVideoManager(void)
{
	SLOGD("Shutting down the video manager");
	/* Toggle the state of the video manager to indicate to the refresh thread
	 * that it needs to shut itself down */

	guiVideoManagerState = VIDEO_OFF;

	if (ScreenBuffer != NULL) {
		SDL_FreeSurface(ScreenBuffer);
		ScreenBuffer = NULL;
	}

	if (ScreenTexture != NULL) {
		SDL_DestroyTexture(ScreenTexture);
		ScreenTexture = NULL;
	}

	if (ScaledScreenTexture != NULL) {
		SDL_DestroyTexture(ScaledScreenTexture);
		ScaledScreenTexture = NULL;
	}

	if (GameRenderer != NULL) {
		SDL_DestroyRenderer(GameRenderer);
		GameRenderer = NULL;
	}

	if (g_game_window != NULL) {
		SDL_DestroyWindow(g_game_window);
		g_game_window = NULL;
	}

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	// ATE: Release mouse cursor!
	FreeMouseCursor();
}


void SuspendVideoManager(void)
{
	guiVideoManagerState = VIDEO_SUSPENDED;
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

		DirtyRegions[guiDirtyRegionCount].x = iLeft;
		DirtyRegions[guiDirtyRegionCount].y = iTop;
		DirtyRegions[guiDirtyRegionCount].w = iRight  - iLeft;
		DirtyRegions[guiDirtyRegionCount].h = iBottom - iTop;
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

		DirtyRegionsEx[guiDirtyRegionExCount].x = iLeft;
		DirtyRegionsEx[guiDirtyRegionExCount].y = iTop;
		DirtyRegionsEx[guiDirtyRegionExCount].w = iRight  - iLeft;
		DirtyRegionsEx[guiDirtyRegionExCount].h = iBottom - iTop;
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

static void ScrollJA2Background(INT16 sScrollXIncrement, INT16 sScrollYIncrement)
{
	SDL_Surface* Frame  = FrameBuffer;
	SDL_Surface* Source = SDL_CreateRGBSurface(0, ScreenBuffer->w, ScreenBuffer->h, PIXEL_DEPTH, RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK);
	SDL_Surface* Dest   = ScreenBuffer; // Back
	SDL_Rect     SrcRect;
	SDL_Rect     DstRect;
	SDL_Rect     StripRegions[2];
	UINT16       NumStrips = 0;

	const UINT16 usWidth  = SCREEN_WIDTH;
	const UINT16 usHeight = gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y;

	SDL_BlitSurface(ScreenBuffer, NULL, Source, NULL);

	if (sScrollXIncrement < 0)
	{
		SrcRect.x = 0;
		SrcRect.w = usWidth + sScrollXIncrement;
		DstRect.x = -sScrollXIncrement;
		StripRegions[0].x = gsVIEWPORT_START_X;
		StripRegions[0].y = gsVIEWPORT_WINDOW_START_Y;
		StripRegions[0].w = -sScrollXIncrement;
		StripRegions[0].h = usHeight;
		++NumStrips;
	}
	else if (sScrollXIncrement > 0)
	{
		SrcRect.x = sScrollXIncrement;
		SrcRect.w = usWidth - sScrollXIncrement;
		DstRect.x = 0;
		StripRegions[0].x = gsVIEWPORT_END_X - sScrollXIncrement;
		StripRegions[0].y = gsVIEWPORT_WINDOW_START_Y;
		StripRegions[0].w = sScrollXIncrement;
		StripRegions[0].h = usHeight;
		++NumStrips;
	}
	else
	{
		SrcRect.x = 0;
		SrcRect.w = usWidth;
		DstRect.x = 0;
	}

	if (sScrollYIncrement < 0)
	{
		SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
		SrcRect.h = usHeight + sScrollYIncrement;
		DstRect.y = gsVIEWPORT_WINDOW_START_Y - sScrollYIncrement;
		StripRegions[NumStrips].x = DstRect.x;
		StripRegions[NumStrips].y = gsVIEWPORT_WINDOW_START_Y;
		StripRegions[NumStrips].w = SrcRect.w;
		StripRegions[NumStrips].h = -sScrollYIncrement;
		++NumStrips;
	}
	else if (sScrollYIncrement > 0)
	{
		SrcRect.y = gsVIEWPORT_WINDOW_START_Y + sScrollYIncrement;
		SrcRect.h = usHeight - sScrollYIncrement;
		DstRect.y = gsVIEWPORT_WINDOW_START_Y;
		StripRegions[NumStrips].x = DstRect.x;
		StripRegions[NumStrips].y = gsVIEWPORT_WINDOW_END_Y - sScrollYIncrement;
		StripRegions[NumStrips].w = SrcRect.w;
		StripRegions[NumStrips].h = sScrollYIncrement;
		++NumStrips;
	}
	else
	{
		SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
		SrcRect.h = usHeight;
		DstRect.y = gsVIEWPORT_WINDOW_START_Y;
	}

	SDL_BlitSurface(Source, &SrcRect, Dest, &DstRect);

#ifdef SCROLL_TEST
	SDL_FillRect(Dest, NULL, 0);
#endif

	for (UINT i = 0; i < NumStrips; i++)
	{
		UINT x = StripRegions[i].x;
		UINT y = StripRegions[i].y;
		UINT w = StripRegions[i].w;
		UINT h = StripRegions[i].h;
		for (UINT j = y; j < y + h; ++j)
		{
			std::fill_n(gpZBuffer + j * SCREEN_WIDTH + x, w, 0);
		}

		RenderStaticWorldRect(x, y, x + w, y + h, TRUE);
		SDL_BlitSurface(Frame, &StripRegions[i], Dest, &StripRegions[i]);
	}

	// RESTORE SHIFTED
	RestoreShiftedVideoOverlays(sScrollXIncrement, sScrollYIncrement);

	// SAVE NEW
	SaveVideoOverlaysArea(BACKBUFFER);

	// BLIT NEW
	ExecuteVideoOverlaysToAlternateBuffer(BACKBUFFER);

	SDL_Texture* screenTexture = SDL_CreateTextureFromSurface(GameRenderer, ScreenBuffer);

	SDL_Rect r;
	r.x = gsVIEWPORT_START_X;
	r.y = gsVIEWPORT_WINDOW_START_Y;
	r.w = gsVIEWPORT_END_X - gsVIEWPORT_START_X;
	r.h = gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y;
	SDL_RenderCopy(GameRenderer, screenTexture, &r, &r);

	SDL_FreeSurface(Source);
	SDL_DestroyTexture(screenTexture);
}


void RefreshScreen(void)
{
	if (guiVideoManagerState != VIDEO_ON) return;

#if DEBUG_PRINT_FPS
	{
		static int32_t prevSecond = 0;
		static int32_t fps = 0;

		int32_t currentSecond = time(NULL);
		if(currentSecond != prevSecond)
		{
			printf("fps: %d\n", fps);
			fps = 0;
			prevSecond = currentSecond;
		}
		else
		{
			fps++;
		}
	}
#endif

	SDL_BlitSurface(FrameBuffer, &MouseBackground, ScreenBuffer, &MouseBackground);

	const BOOLEAN scrolling = (gsScrollXIncrement != 0 || gsScrollYIncrement != 0);

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
					SDL_BlitSurface(FrameBuffer, &DirtyRegions[i], ScreenBuffer, &DirtyRegions[i]);
				}

				for (UINT32 i = 0; i < guiDirtyRegionExCount; i++)
				{
					SDL_Rect* r = &DirtyRegionsEx[i];
					if (scrolling)
					{
						// Check if we are completely out of bounds
						if (r->y <= gsVIEWPORT_WINDOW_END_Y && r->y + r->h <= gsVIEWPORT_WINDOW_END_Y)
						{
							continue;
						}
					}
					SDL_BlitSurface(FrameBuffer, r, ScreenBuffer, r);
				}
			}
		}
		if (scrolling)
		{
			ScrollJA2Background(gsScrollXIncrement, gsScrollYIncrement);
			gsScrollXIncrement = 0;
			gsScrollYIncrement = 0;
		}
		gfIgnoreScrollDueToCenterAdjust = FALSE;
		guiFrameBufferState = BUFFER_READY;
	}

	SGPPoint cursorPos;
	GetCursorPos(cursorPos);
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	src.w = gusMouseCursorWidth;
	src.h = gusMouseCursorHeight + gsMouseSizeYModifier;
	SDL_Rect dst;
	dst.x = cursorPos.iX - gsMouseCursorXOffset;
	dst.y = cursorPos.iY - gsMouseCursorYOffset;
	SDL_BlitSurface(MouseCursor, &src, ScreenBuffer, &dst);
	MouseBackground = dst;

	SDL_UpdateTexture(ScreenTexture, NULL, ScreenBuffer->pixels, ScreenBuffer->pitch);

	SDL_RenderClear(GameRenderer);

	if (ScaleQuality == VideoScaleQuality::NEAR_PERFECT) {
		SDL_SetRenderTarget(GameRenderer, ScaledScreenTexture);
		SDL_RenderCopy(GameRenderer, ScreenTexture, nullptr, nullptr);

		SDL_SetRenderTarget(GameRenderer, nullptr);
		SDL_RenderCopy(GameRenderer, ScaledScreenTexture, nullptr, nullptr);
	}
	else {
		SDL_RenderCopy(GameRenderer, ScreenTexture, NULL, NULL);
	}

	SDL_RenderPresent(GameRenderer);

	gfForceFullScreenRefresh = FALSE;
	guiDirtyRegionCount = 0;
	guiDirtyRegionExCount = 0;
}


static void GetRGBDistribution()
{
	SDL_PixelFormat const& f = *ScreenBuffer->format;

	UINT32          const  r = f.Rmask;
	UINT32          const  g = f.Gmask;
	UINT32          const  b = f.Bmask;

	/* Mask the highest bit of each component. This is used for alpha blending. */
	guiTranslucentMask = (r & r >> 1) | (g & g >> 1) | (b & b >> 1);

	gusRedMask   = r;
	gusGreenMask = g;
	gusBlueMask  = b;

	gusRedShift   = f.Rshift - f.Rloss;
	gusGreenShift = f.Gshift - f.Gloss;
	gusBlueShift  = f.Bshift - f.Bloss;
}


void GetPrimaryRGBDistributionMasks(UINT32* const  RedBitMask, UINT32* const GreenBitMask, UINT32* const BlueBitMask)
{
	*RedBitMask   = gusRedMask;
	*GreenBitMask = gusGreenMask;
	*BlueBitMask  = gusBlueMask;
}


void SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight, UINT16 usCursorWidth)
{
	gsMouseCursorXOffset = sOffsetX;
	gsMouseCursorYOffset = sOffsetY;
	gusMouseCursorWidth  = usCursorWidth;
	gusMouseCursorHeight = usCursorHeight;
}


void EndFrameBufferRender(void)
{
	guiFrameBufferState = BUFFER_DIRTY;
}


static void RecreateBackBuffer()
{
	// ScreenBuffer should not be automatically removed because it was created
	// with SDL_SetVideoMode.  So, using SGPVSurface instead of SGPVSurfaceAuto
	SGPVSurface* newBackbuffer = new SGPVSurface(ScreenBuffer);

	if(g_back_buffer != NULL)
	{
		ReplaceFontBackBuffer(g_back_buffer, newBackbuffer);

		delete g_back_buffer;
		g_back_buffer = NULL;
	}

	g_back_buffer  = newBackbuffer;
}

static void SetPrimaryVideoSurfaces(void)
{
	// Delete surfaces if they exist
	DeletePrimaryVideoSurfaces();

	RecreateBackBuffer();

	g_mouse_buffer = new SGPVSurfaceAuto(MouseCursor);
	g_frame_buffer = new SGPVSurfaceAuto(FrameBuffer);
}

static void DeletePrimaryVideoSurfaces(void)
{
	delete g_back_buffer;
	g_back_buffer = NULL;

	delete g_frame_buffer;
	g_frame_buffer = NULL;

	delete g_mouse_buffer;
	g_mouse_buffer = NULL;
}

SGPVSurface* gpVSurfaceHead = 0;

void InitializeVideoSurfaceManager(void)
{
	//Shouldn't be calling this if the video surface manager already exists.
	//Call shutdown first...
	Assert(gpVSurfaceHead == NULL);
	gpVSurfaceHead = NULL;

	// Create primary and backbuffer from globals
	SetPrimaryVideoSurfaces();
}


void ShutdownVideoSurfaceManager(void)
{
	SLOGD("Shutting down the Video Surface manager");

	// Delete primary viedeo surfaces
	DeletePrimaryVideoSurfaces();

	while (gpVSurfaceHead)
	{
		delete gpVSurfaceHead;
	}
}
