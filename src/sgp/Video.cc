#include "Debug.h"
#include "Fade_Screen.h"
#include "FileMan.h"
#include "HImage.h"
#include "Input.h"
#include "Local.h"
#include "MemMan.h"
#include "RenderWorld.h"
#include "Render_Dirty.h"
#include "Timer.h"
#include "Timer_Control.h"
#include "Types.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "UILayout.h"
#include "PlatformIO.h"
#include "Font.h"
#include "Icon.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include "Logger.h"

#include <algorithm>
#include <ctime>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdexcept>

#define BUFFER_READY      0x00
#define BUFFER_DIRTY      0x02

#define MAX_CURSOR_WIDTH  64
#define MAX_CURSOR_HEIGHT 64

#define MAX_DIRTY_REGIONS 128

#define VIDEO_OFF         0x00
#define VIDEO_ON          0x01
#define VIDEO_SUSPENDED   0x04

#define MAX_NUM_FRAMES    25

#define RED_MASK 0xF800
#define GREEN_MASK 0x07E0
#define BLUE_MASK 0x001F
#define ALPHA_MASK 0

#define OVERSAMPLING_SCALE 4

static BOOLEAN gfVideoCapture = FALSE;
static UINT32  guiFramePeriod = 1000 / 15;
static UINT32  guiLastFrame;
static UINT16* gpFrameData[MAX_NUM_FRAMES];
static INT32   giNumFrames = 0;


// Globals for mouse cursor
static UINT16 gusMouseCursorWidth;
static UINT16 gusMouseCursorHeight;
static INT16  gsMouseCursorXOffset;
static INT16  gsMouseCursorYOffset;

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

// Screen output stuff
static BOOLEAN gfPrintFrameBuffer;
static UINT32  guiPrintFrameBufferIndex;


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
	SDL_SetWindowBrightness(g_game_window, brightness);
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
		SLOGE("SDL_CreateRGBSurface for ScreenBuffer failed: %s\n", SDL_GetError());
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
			SLOGE("SDL_CreateTexture for ScaledScreenTexture failed: %s\n", SDL_GetError());
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
		SLOGE("SDL_CreateTexture for ScreenTexture failed: %s\n", SDL_GetError());
	}

	FrameBuffer = SDL_CreateRGBSurface(
		SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);

	if (FrameBuffer == NULL)
	{
		SLOGE("SDL_CreateRGBSurface for FrameBuffer failed: %s\n", SDL_GetError());
	}

	MouseCursor = SDL_CreateRGBSurface(
		0, MAX_CURSOR_WIDTH, MAX_CURSOR_HEIGHT, PIXEL_DEPTH,
		RED_MASK, GREEN_MASK, BLUE_MASK, ALPHA_MASK
	);
	SDL_SetColorKey(MouseCursor, SDL_TRUE, 0);

	if (MouseCursor == NULL)
	{
		SLOGE("SDL_CreateRGBSurface for MouseCursor failed: %s\n", SDL_GetError());
	}

	SDL_ShowCursor(SDL_DISABLE);

	// Initialize state variables
	guiFrameBufferState      = BUFFER_DIRTY;
	guiVideoManagerState     = VIDEO_ON;
	guiDirtyRegionCount      = 0;
	gfForceFullScreenRefresh = TRUE;
	gfPrintFrameBuffer       = FALSE;
	guiPrintFrameBufferIndex = 0;

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


static void WriteTGAHeader(File* file)
{
	/*
	 *  0 byte ID length
	 *  1 byte colour map type
	 *  2 byte targa type
	 *  3 word colour map origin
	 *  5 word colour map length
	 *  7 byte colour map entry size
	 *  8 word origin x
	 * 10 word origin y
	 * 12 word image width
	 * 14 word image height
	 * 16 byte bits per pixel
	 * 17 byte image descriptor
	 */
	static const BYTE data[] =
	{
		0,
		0,
		2,
		0, 0,
		0, 0,
		0,
		0, 0,
		0, 0,
		(UINT8) (SCREEN_WIDTH  % 256), (UINT8) (SCREEN_WIDTH  / 256),
		(UINT8) (SCREEN_HEIGHT % 256), (UINT8) (SCREEN_HEIGHT / 256),
		PIXEL_DEPTH,
		0
	};
	if (!File_writeAll(file, data, sizeof(data)))
	{
		RustPointer<char> err(getRustError());
		SLOGW("WriteTGAHeader: %s", err.get());
	}
}


/* Create a file for a screenshot, which is guaranteed not to exist yet. */
static RustPointer<File> CreateScreenshotFile(void)
{
	const ST::string exec_dir = GCM->getScreenshotFolder();
	while (true)
	{
		char filename[2048];
		sprintf(filename, "%s/SCREEN%03d.TGA", exec_dir.c_str(), guiPrintFrameBufferIndex++);
		RustPointer<File> file(File_open(filename, FILE_OPEN_WRITE | FILE_OPEN_CREATE_NEW));
		if (file)
		{
			return file;
		}
	}
}


static void TakeScreenshot()
{
	RustPointer<File> file = CreateScreenshotFile();
	if (!file)
	{
		RustPointer<char> err(getRustError());
		SLOGE("TakeScreenshot: %s", err.get());
		return;
	}

	WriteTGAHeader(file.get());

	// If not 5/5/5, create buffer
	UINT16* buf = 0;
	if (gusRedMask != 0x7C00 || gusGreenMask != 0x03E0 || gusBlueMask != 0x001F)
	{
		buf = new UINT16[SCREEN_WIDTH]{};
	}

	UINT16 const* src = static_cast<UINT16 const*>(ScreenBuffer->pixels);
	for (INT32 y = SCREEN_HEIGHT - 1; y >= 0; --y)
	{
		if (buf)
		{ // ATE: Fix this such that it converts pixel format to 5/5/5
			memcpy(buf, src + y * SCREEN_WIDTH, SCREEN_WIDTH * sizeof(*buf));
			ConvertRGBDistribution565To555(buf, SCREEN_WIDTH);
			if (!File_writeAll(file.get(), reinterpret_cast<const UINT8*>(buf), sizeof(*buf) * SCREEN_WIDTH))
			{
				RustPointer<char> err(getRustError());
				SLOGW("TakeScreenshot: %s", err.get());
			}
		}
		else
		{
			if (!File_writeAll(file.get(), reinterpret_cast<const UINT8*>(src + y * SCREEN_WIDTH), sizeof(*buf) * SCREEN_WIDTH))
			{
				RustPointer<char> err(getRustError());
				SLOGW("TakeScreenshot: %s", err.get());
			}
		}
	}

	if (buf) delete[] buf;
}

static void SnapshotSmall(void);

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

	SGPPoint MousePos;
	GetMousePos(&MousePos);
	SDL_Rect src;
	src.x = 0;
	src.y = 0;
	src.w = gusMouseCursorWidth;
	src.h = gusMouseCursorHeight;
	SDL_Rect dst;
	dst.x = MousePos.iX - gsMouseCursorXOffset;
	dst.y = MousePos.iY - gsMouseCursorYOffset;
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


void PrintScreen(void)
{
	gfPrintFrameBuffer = TRUE;
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

static void RefreshMovieCache(void)
{
	static UINT32 uiPicNum = 0;

	PauseTime(TRUE);

	const ST::string exec_dir = GCM->getVideoCaptureFolder();

	for (INT32 cnt = 0; cnt < giNumFrames; cnt++)
	{
		CHAR8 cFilename[2048];
		sprintf(cFilename, "%s/JA%5.5d.TGA", exec_dir.c_str(), uiPicNum++);

		RustPointer<File> file(File_open(cFilename, FILE_OPEN_WRITE));
		if (!file)
		{
			RustPointer<char> err(getRustError());
			SLOGE("RefreshMovieCache: %s", err.get());
			return;
		}

		WriteTGAHeader(file.get());

		UINT16* pDest = gpFrameData[cnt];

		for (INT32 iCountY = SCREEN_HEIGHT - 1; iCountY >= 0; iCountY -= 1)
		{
			for (INT32 iCountX = 0; iCountX < SCREEN_WIDTH; iCountX ++)
			{
				if (!File_writeAll(file.get(), reinterpret_cast<const uint8_t*>(pDest + iCountY * SCREEN_WIDTH + iCountX), sizeof(UINT16)))
				{
					RustPointer<char> err(getRustError());
					SLOGW("RefreshMovieCache: %s", err.get());
				}
			}
		}
	}

	PauseTime(FALSE);

	giNumFrames = 0;
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
