#include "Cursor_Control.h"
#include "Debug.h"
#include "Fade_Screen.h"
#include "FPS.h"
#include "HImage.h"
#include "Local.h"
#include "Logger.h"
#include "RenderWorld.h"
#include "Render_Dirty.h"
#include "Types.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "UILayout.h"
#include "Icon.h"
#include <algorithm>
#include <chrono>
#include <stdexcept>

#define MAX_CURSOR_WIDTH  64
#define MAX_CURSOR_HEIGHT 64

#define MAX_DIRTY_REGIONS 128

#define RED_MASK 0xFF000000 // TODO: maxrd2 we don't need these
#define GREEN_MASK 0x00FF0000
#define BLUE_MASK 0x0000FF00
#define ALPHA_MASK 0x000000FF

#define OVERSAMPLING_SCALE 4


// Globals for mouse cursor
static UINT16 gusMouseCursorWidth;
static UINT16 gusMouseCursorHeight;
static INT16  gsMouseCursorXOffset;
static INT16  gsMouseCursorYOffset;
INT16 gsMouseSizeYModifier = 0; // This can increase the size of gusMouseCursorHeight so image data (ie, text) outside the normal height of the mouse cursor can be copied onto screen buffer

static SDL_Rect MouseBackground = { 0, 0, 0, 0 };

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
static std::chrono::steady_clock::duration TimeBetweenRefreshScreens;
static std::chrono::steady_clock::time_point LastRefresh;

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
	SDL_RenderClear(GameRenderer);
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


void InitializeVideoManager(const VideoScaleQuality quality,
                            const int32_t targetFPS)
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

	SurfaceUniquePtr windowIcon(SDL_CreateRGBSurfaceWithFormatFrom(
			(void*)gWindowIconData.pixel_data,
			gWindowIconData.width,
			gWindowIconData.height,
			0,
			gWindowIconData.bytes_per_pixel*gWindowIconData.width,
			SDL_PIXELFORMAT_ABGR8888));
	SDL_SetWindowIcon(g_game_window, windowIcon.get());


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
	}
	else if (ScaleQuality == VideoScaleQuality::NEAR_PERFECT)
	{
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		ScaledScreenTexture = SDL_CreateTexture(GameRenderer,
			SDL_PIXELFORMAT_RGBA8888,
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
					SDL_PIXELFORMAT_RGBA8888,
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
	gfForceFullScreenRefresh = TRUE;

	// This function must be called to setup RGB information
	GetRGBDistribution();

	TimeBetweenRefreshScreens = std::chrono::microseconds{1'000'000} / targetFPS;
}


void ShutdownVideoManager(void)
{
	SLOGD("Shutting down the video manager");

	// ScreenBuffer SDL surface freed by its SGPVSurface wrapper.
	ScreenBuffer = nullptr;

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
		InvalidateScreen();
	}
}


static void AddRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom);


void InvalidateRegionEx(INT32 iLeft, INT32 iTop, INT32 iRight, INT32 iBottom)
{
	if (gfForceFullScreenRefresh)
	{
		// There's no point in going on since we are forcing a full screen refresh
		return;
	}

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
		InvalidateScreen();
	}
}


void InvalidateScreen(void)
{
	guiDirtyRegionCount = 0;
	guiDirtyRegionExCount = 0;
	gfForceFullScreenRefresh = TRUE;
}


//#define SCROLL_TEST

static void ScrollJA2Background(INT16 sScrollXIncrement, INT16 sScrollYIncrement)
{
	SDL_Surface* Dest   = ScreenBuffer; // Back
	SDL_Rect     SrcRect;
	SDL_Rect     DstRect;
	SDL_Rect     StripRegions[2];
	UINT16       NumStrips = 0;

	int const width  = SCREEN_WIDTH;
	int const height = gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y;

	if (sScrollXIncrement < 0)
	{
		SrcRect.x = 0;
		SrcRect.w = width + sScrollXIncrement;
		DstRect.x = -sScrollXIncrement;
		StripRegions[0].x = gsVIEWPORT_START_X;
		StripRegions[0].y = gsVIEWPORT_WINDOW_START_Y;
		StripRegions[0].w = -sScrollXIncrement;
		StripRegions[0].h = height;
		++NumStrips;
	}
	else if (sScrollXIncrement > 0)
	{
		SrcRect.x = sScrollXIncrement;
		SrcRect.w = width - sScrollXIncrement;
		DstRect.x = 0;
		StripRegions[0].x = gsVIEWPORT_END_X - sScrollXIncrement;
		StripRegions[0].y = gsVIEWPORT_WINDOW_START_Y;
		StripRegions[0].w = sScrollXIncrement;
		StripRegions[0].h = height;
		++NumStrips;
	}
	else
	{
		SrcRect.x = 0;
		SrcRect.w = width;
		DstRect.x = 0;
	}

	if (sScrollYIncrement < 0)
	{
		SrcRect.y = gsVIEWPORT_WINDOW_START_Y;
		SrcRect.h = height + sScrollYIncrement;
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
		SrcRect.h = height - sScrollYIncrement;
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
		SrcRect.h = height;
		DstRect.y = gsVIEWPORT_WINDOW_START_Y;
	}

#ifdef SCROLL_TEST
	SDL_FillRect(Dest, NULL, 0);
#endif

	{
		SurfaceUniquePtr Source(SDL_CreateRGBSurfaceWithFormat(0,
			ScreenBuffer->w, ScreenBuffer->h, 0, ScreenBuffer->format->format));
		SDL_BlitSurface(ScreenBuffer, nullptr, Source.get(), nullptr);
		SDL_BlitSurface(Source.get(), &SrcRect, Dest, &DstRect);
	}

	for (UINT i = 0; i < NumStrips; i++)
	{
		INT16 const x = static_cast<INT16>(StripRegions[i].x);
		INT16 const y = static_cast<INT16>(StripRegions[i].y);
		INT16 const w = static_cast<INT16>(StripRegions[i].w);
		INT16 const h = static_cast<INT16>(StripRegions[i].h);
		for (int j = y; j < y + h; ++j)
		{
			std::fill_n(gpZBuffer + j * SCREEN_WIDTH + x, w, 0);
		}

		RenderStaticWorldRect(x, y, x + w, y + h, TRUE);
		SDL_BlitSurface(FrameBuffer, &StripRegions[i], Dest, &StripRegions[i]);
	}

	// RESTORE SHIFTED
	RestoreShiftedVideoOverlays(sScrollXIncrement, sScrollYIncrement);

	// SAVE NEW
	SaveVideoOverlaysArea(BACKBUFFER);

	// BLIT NEW
	ExecuteVideoOverlaysToAlternateBuffer(BACKBUFFER);
}

void RefreshScreen(void)
{
	// Not initialised yet or already shut down?
	if (!ScreenTexture) return;

	const BOOLEAN scrolling = (gsScrollXIncrement != 0 || gsScrollYIncrement != 0);

	auto const now = std::chrono::steady_clock::now();
	if (!scrolling && now - LastRefresh < TimeBetweenRefreshScreens)
	{
		return;
	}
	LastRefresh = now;

	SDL_BlitSurface(FrameBuffer, &MouseBackground, ScreenBuffer, &MouseBackground);

	// This variable will hold the union of all modified regions.
	struct rect : SDL_Rect {
		void operator+=(SDL_Rect const& r) { SDL_UnionRect(this, &r, this); }
	} ScreenTextureUpdateRect{ MouseBackground };

	if (gfForceFullScreenRefresh || guiDirtyRegionCount > 0 || guiDirtyRegionExCount > 0)
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
				ScreenTextureUpdateRect = { 0, 0, ScreenBuffer->w, ScreenBuffer->h };
			}
			else
			{
				for (UINT32 i = 0; i < guiDirtyRegionCount; i++)
				{
					ScreenTextureUpdateRect += DirtyRegions[i];
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
					ScreenTextureUpdateRect += *r;
					SDL_BlitSurface(FrameBuffer, r, ScreenBuffer, r);
				}
			}
		}
		if (scrolling)
		{
			ScrollJA2Background(gsScrollXIncrement, gsScrollYIncrement);
			gsScrollXIncrement = 0;
			gsScrollYIncrement = 0;
			ScreenTextureUpdateRect += SDL_Rect{
				gsVIEWPORT_START_X, gsVIEWPORT_WINDOW_START_Y,
				gsVIEWPORT_END_X - gsVIEWPORT_START_X,
				gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y };
		}
		gfIgnoreScrollDueToCenterAdjust = FALSE;
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
	ScreenTextureUpdateRect += dst;
	MouseBackground = dst;

	uint8_t const * SrcPixels = static_cast<uint8_t *>(ScreenBuffer->pixels)
		+ ScreenTextureUpdateRect.y * ScreenBuffer->pitch
		+ ScreenTextureUpdateRect.x * ScreenBuffer->format->BytesPerPixel;
	SDL_UpdateTexture(ScreenTexture, &ScreenTextureUpdateRect,
	                  SrcPixels, ScreenBuffer->pitch);

	if (ScaleQuality == VideoScaleQuality::NEAR_PERFECT) {
		SDL_SetRenderTarget(GameRenderer, ScaledScreenTexture);
		SDL_RenderCopy(GameRenderer, ScreenTexture, nullptr, nullptr);

		SDL_SetRenderTarget(GameRenderer, nullptr);
		SDL_RenderCopy(GameRenderer, ScaledScreenTexture, nullptr, nullptr);
	}
	else {
		SDL_RenderCopy(GameRenderer, ScreenTexture, NULL, NULL);
	}

	FPS::RenderPresentPtr(GameRenderer);

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

	gusRedMask   = static_cast<UINT16>(r);
	gusGreenMask = static_cast<UINT16>(g);
	gusBlueMask  = static_cast<UINT16>(b);

	gusRedShift   = f.Rshift - f.Rloss;
	gusGreenShift = f.Gshift - f.Gloss;
	gusBlueShift  = f.Bshift - f.Bloss;
}


void SetMouseCursorProperties(INT16 sOffsetX, INT16 sOffsetY, UINT16 usCursorHeight, UINT16 usCursorWidth)
{
	gsMouseCursorXOffset = sOffsetX;
	gsMouseCursorYOffset = sOffsetY;
	gusMouseCursorWidth  = usCursorWidth;
	gusMouseCursorHeight = usCursorHeight;
}


static void SetPrimaryVideoSurfaces(void)
{
	// Delete surfaces if they exist
	DeletePrimaryVideoSurfaces();

	g_back_buffer  = new SGPVSurface(ScreenBuffer);
	g_mouse_buffer = new SGPVSurface(MouseCursor);
	g_frame_buffer = new SGPVSurface(FrameBuffer);
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


void HandleWindowEvent(SDL_Event const& evt)
{
	if (evt.window.event == SDL_WINDOWEVENT_RESIZED) {
		SDL_RenderClear(GameRenderer);
	}
}
