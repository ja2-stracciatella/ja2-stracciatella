#include "Debug.h"
#include "HImage.h"
#include "MemMan.h"
#include "Shading.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"


static void DeletePrimaryVideoSurfaces(void);


typedef struct VSURFACE_NODE
{
	HVSURFACE hVSurface;
  struct VSURFACE_NODE* next;

#ifdef SGP_VIDEO_DEBUGGING
	char* pName;
	char* pCode;
#endif
} VSURFACE_NODE;

static VSURFACE_NODE* gpVSurfaceHead = NULL;
static VSURFACE_NODE* gpVSurfaceTail = NULL;
UINT32				        guiVSurfaceSize = 0;


SGPVSurface* g_back_buffer;
SGPVSurface* g_frame_buffer;
SGPVSurface* g_mouse_buffer;


static BOOLEAN SetPrimaryVideoSurfaces(void);


BOOLEAN InitializeVideoSurfaceManager(void)
{
	//Shouldn't be calling this if the video surface manager already exists.
	//Call shutdown first...
	Assert(gpVSurfaceHead == NULL);
	Assert(gpVSurfaceTail == NULL);
	gpVSurfaceHead = NULL;
	gpVSurfaceTail = NULL;

	// Create primary and backbuffer from globals
	if (!SetPrimaryVideoSurfaces())
	{
		DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_1, "Could not create primary surfaces");
		return FALSE;
	}

	return TRUE;
}


static BOOLEAN InternalDeleteVideoSurface(HVSURFACE hVSurface);


BOOLEAN ShutdownVideoSurfaceManager(void)
{
  DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Shutting down the Video Surface manager");

	// Delete primary viedeo surfaces
	DeletePrimaryVideoSurfaces();

	while (gpVSurfaceHead != NULL)
	{
		VSURFACE_NODE* curr = gpVSurfaceHead;
		gpVSurfaceHead = gpVSurfaceHead->next;
		InternalDeleteVideoSurface(curr->hVSurface);
#ifdef SGP_VIDEO_DEBUGGING
		if (curr->pName != NULL) MemFree(curr->pName);
		if (curr->pCode != NULL) MemFree(curr->pCode);
#endif
		MemFree(curr);
	}
	gpVSurfaceHead = NULL;
	gpVSurfaceTail = NULL;
	guiVSurfaceSize = 0;
	return TRUE;
}


static void AddStandardVideoSurface(HVSURFACE hVSurface)
{
	if (hVSurface == NULL) return;

	VSURFACE_NODE* Node = MemAlloc(sizeof(*Node));
	Assert(Node != NULL); // out of memory?
	Node->hVSurface = hVSurface;

	Node->next      = NULL;
#ifdef SGP_VIDEO_DEBUGGING
	Node->pName     = NULL;
	Node->pCode     = NULL;
#endif

	if (gpVSurfaceTail == NULL)
	{
		gpVSurfaceHead = Node;
	}
	else
	{
		gpVSurfaceTail->next = Node;
	}
	gpVSurfaceTail = Node;

	guiVSurfaceSize++;
}


static HVSURFACE CreateVideoSurface(UINT16 usWidth, UINT16 usHeight, UINT8 ubBitDepth);


#undef AddVideoSurface
#undef AddVideoSurfaceFromFile


SGPVSurface* AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth)
{
	SGPVSurface* const vs = CreateVideoSurface(Width, Height, BitDepth);
	AddStandardVideoSurface(vs);
	return vs;
}


static BOOLEAN SetVideoSurfaceDataFromHImage(HVSURFACE hVSurface, HIMAGE hImage, UINT16 usX, UINT16 usY, const SGPRect* pSrcRect);


SGPVSurface* AddVideoSurfaceFromFile(const char* const Filename)
{
	const HIMAGE hImage = CreateImage(Filename, IMAGE_ALLIMAGEDATA);
	if (hImage == NULL)
	{
		DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid Image Filename given");
		return NULL;
	}

	SGPVSurface* const vs = CreateVideoSurface(hImage->usWidth, hImage->usHeight, hImage->ubBitDepth);
	if (vs != NULL)
	{
		SGPRect tempRect;
		tempRect.iLeft   = 0;
		tempRect.iTop    = 0;
		tempRect.iRight  = hImage->usWidth  - 1;
		tempRect.iBottom = hImage->usHeight - 1;
		SetVideoSurfaceDataFromHImage(vs, hImage, 0, 0, &tempRect);

		if (hImage->ubBitDepth == 8)
		{
			SetVideoSurfacePalette(vs, hImage->pPalette);
		}
	}

	DestroyImage(hImage);
	AddStandardVideoSurface(vs);
	return vs;
}


static BYTE* LockVideoSurfaceBuffer(HVSURFACE hVSurface, UINT32* pPitch);
static void UnLockVideoSurfaceBuffer(HVSURFACE hVSurface);


BYTE* LockVideoSurface(SGPVSurface* const vs, UINT32* const puiPitch)
{
	return LockVideoSurfaceBuffer(vs, puiPitch);
}


void UnLockVideoSurface(SGPVSurface* const vs)
{
	UnLockVideoSurfaceBuffer(vs);
}


static BOOLEAN SetVideoSurfaceTransparencyColor(HVSURFACE hVSurface, COLORVAL TransColor);


BOOLEAN SetVideoSurfaceTransparency(SGPVSurface* const vs, const COLORVAL TransColor)
{
	SetVideoSurfaceTransparencyColor(vs, TransColor);
	return TRUE;
}


static HVSURFACE CreateVideoSurfaceFromDDSurface(SDL_Surface* surface);


static BOOLEAN SetPrimaryVideoSurfaces(void)
{
	SDL_Surface* pSurface;

	// Delete surfaces if they exist
	DeletePrimaryVideoSurfaces();

#ifdef JA2
	pSurface = GetBackBufferObject();
	CHECKF(pSurface != NULL);
	g_back_buffer = CreateVideoSurfaceFromDDSurface(pSurface);
	CHECKF(g_back_buffer != NULL);

	pSurface = GetMouseBufferObject();
	CHECKF(pSurface != NULL);
	g_mouse_buffer = CreateVideoSurfaceFromDDSurface(pSurface);
	CHECKF(g_mouse_buffer != NULL);
#endif

	pSurface = GetFrameBufferObject();
	CHECKF(pSurface != NULL);
	g_frame_buffer = CreateVideoSurfaceFromDDSurface(pSurface);
	CHECKF(g_frame_buffer != NULL);

	return TRUE;
}


static void DeletePrimaryVideoSurfaces(void)
{
	if (g_back_buffer != NULL)
	{
		InternalDeleteVideoSurface(g_back_buffer);
		g_back_buffer = NULL;
	}

  if (g_frame_buffer != NULL)
	{
		InternalDeleteVideoSurface(g_frame_buffer);
		g_frame_buffer = NULL;
	}

	if (g_mouse_buffer != NULL)
	{
		InternalDeleteVideoSurface(g_mouse_buffer);
		g_mouse_buffer = NULL;
	}
}


void BltVideoSurfaceHalf(SGPVSurface* const dst, SGPVSurface* const src, const INT32 DestX, const INT32 DestY, const SGPRect* const SrcRect)
{
	UINT32 SrcPitchBYTES;
	const UINT8* SrcBuf = LockVideoSurface(src, &SrcPitchBYTES);
	UINT32 DestPitchBYTES;
	UINT16* DestBuf = (UINT16*)LockVideoSurface(dst, &DestPitchBYTES);
	if (SrcRect == NULL)
	{
		Blt8BPPDataTo16BPPBufferHalf(DestBuf, DestPitchBYTES, src, SrcBuf, SrcPitchBYTES, DestX, DestY);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferHalfRect(DestBuf, DestPitchBYTES, src, SrcBuf, SrcPitchBYTES, DestX, DestY, SrcRect);
	}
	UnLockVideoSurface(dst);
	UnLockVideoSurface(src);
}


void FillSurface(SGPVSurface* const vs, const UINT16 Colour)
{
	SDL_FillRect(vs->surface, NULL, Colour);
}


BOOLEAN ColorFillVideoSurfaceArea(SGPVSurface* const dst, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, const UINT16 Color16BPP)
{
	SGPRect Clip;
	GetClippingRect(&Clip);

	if (iDestX1 < Clip.iLeft) iDestX1 = Clip.iLeft;
	if (iDestX1 > Clip.iRight) return FALSE;

	if (iDestX2 > Clip.iRight) iDestX2 = Clip.iRight;
	if (iDestX2 < Clip.iLeft) return FALSE;

	if (iDestY1 < Clip.iTop) iDestY1 = Clip.iTop;
	if (iDestY1 > Clip.iBottom) return FALSE;

	if (iDestY2 > Clip.iBottom) iDestY2 = Clip.iBottom;
	if (iDestY2 < Clip.iTop) return FALSE;

	if (iDestX2 <= iDestX1 || iDestY2 <= iDestY1) return FALSE;

	SDL_Rect Rect;
	Rect.x = iDestX1;
	Rect.y = iDestY1;
	Rect.w = iDestX2 - iDestX1;
	Rect.h = iDestY2 - iDestY1;
	SDL_FillRect(dst->surface, &Rect, Color16BPP);

	return TRUE;
}


static HVSURFACE CreateVideoSurface(UINT16 usWidth, UINT16 usHeight, UINT8 ubBitDepth)
{
	Assert(usHeight > 0);
	Assert(usWidth  > 0);

	UINT32 uiRBitMask;
	UINT32 uiGBitMask;
	UINT32 uiBBitMask;
	switch (ubBitDepth)
	{
		case 8:
			uiRBitMask = 0;
			uiGBitMask = 0;
			uiBBitMask = 0;
			break;

		case 16:
		{
			const SDL_Surface* screen = SDL_GetVideoSurface();
			uiRBitMask = screen->format->Rmask;
			uiGBitMask = screen->format->Gmask;
			uiBBitMask = screen->format->Bmask;
			break;
		}

		default:
			// If Here, an invalid format was given
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid BPP value, can only be 8 or 16.");
			return FALSE;
	}

	SDL_Surface* surface = SDL_CreateRGBSurface(
		SDL_SWSURFACE,
		usWidth, usHeight, ubBitDepth,
		uiRBitMask, uiGBitMask, uiBBitMask, 0
	);

	HVSURFACE hVSurface = MemAlloc(sizeof(*hVSurface));
	CHECKF(hVSurface != NULL);

	hVSurface->surface       = surface;
	hVSurface->usHeight      = usHeight;
	hVSurface->usWidth       = usWidth;
	hVSurface->ubBitDepth    = ubBitDepth;
	hVSurface->pPalette      = NULL;
	hVSurface->p16BPPPalette = NULL;

	DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, "Success in Creating Video Surface");

	return hVSurface;
}


// Lock must be followed by release
// Pitch MUST be used for all width calculations ( Pitch is in bytes )
// The time between Locking and unlocking must be minimal
static BYTE* LockVideoSurfaceBuffer(HVSURFACE hVSurface, UINT32* pPitch)
{
	Assert(hVSurface != NULL);
	Assert(pPitch != NULL);

	SDL_LockSurface(hVSurface->surface); // XXX necessary?
	*pPitch = hVSurface->surface->pitch;
	return hVSurface->surface->pixels;
}


static void UnLockVideoSurfaceBuffer(HVSURFACE hVSurface)
{
	Assert(hVSurface != NULL);

	SDL_UnlockSurface(hVSurface->surface); // XXX necessary?
}


// Given an HIMAGE object, blit imagery into existing Video Surface. Can be from 8->16 BPP
static BOOLEAN SetVideoSurfaceDataFromHImage(HVSURFACE hVSurface, HIMAGE hImage, UINT16 usX, UINT16 usY, const SGPRect* pSrcRect)
{
	Assert(hVSurface != NULL);
	Assert(hImage != NULL);

	// Get Size of hImage and determine if it can fit
	CHECKF(hImage->usWidth  >= hVSurface->usWidth);
	CHECKF(hImage->usHeight >= hVSurface->usHeight);

	// Check BPP and see if they are the same
	UINT32 fBufferBPP = 0;
	if (hImage->ubBitDepth != hVSurface->ubBitDepth)
	{
		// They are not the same, but we can go from 8->16 without much cost
		if (hImage->ubBitDepth == 8 && hVSurface->ubBitDepth == 16)
		{
			fBufferBPP = BUFFER_16BPP;
		}
	}
	else
	{
		switch (hImage->ubBitDepth)
		{
			case  8: fBufferBPP = BUFFER_8BPP;  break;
			case 16: fBufferBPP = BUFFER_16BPP; break;
		}
	}

	Assert(fBufferBPP != 0);

	UINT32 uiPitch;
	BYTE* pDest = LockVideoSurfaceBuffer(hVSurface, &uiPitch);

	// Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
	UINT16 usEffectiveWidth = uiPitch / (hVSurface->ubBitDepth / 8);

	CHECKF(pDest != NULL);

	// Blit Surface
	// If rect is NULL, use entrie image size
	SGPRect	aRect;
	if (pSrcRect == NULL)
	{
		aRect.iLeft   = 0;
		aRect.iTop    = 0;
		aRect.iRight  = hImage->usWidth;
		aRect.iBottom = hImage->usHeight;
	}
	else
	{
		aRect = *pSrcRect;
	}

	// This HIMAGE function will transparently copy buffer
	BOOLEAN Ret = CopyImageToBuffer(hImage, fBufferBPP, pDest, usEffectiveWidth, hVSurface->usHeight, usX, usY, &aRect);
	if (!Ret)
	{
		DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Error Occured Copying HIMAGE to HVSURFACE");
	}

	UnLockVideoSurfaceBuffer(hVSurface);
	return Ret;
}


// Palette setting is expensive, need to set both DDPalette and create 16BPP palette
BOOLEAN SetVideoSurfacePalette(SGPVSurface* const hVSurface, const SGPPaletteEntry* const pSrcPalette)
{
	Assert(hVSurface != NULL);

	// Create palette object if not already done so
	if (hVSurface->pPalette == NULL)
	{
		hVSurface->pPalette = MemAlloc(sizeof(*hVSurface->pPalette) * 256);
	}
	for (UINT32 i = 0; i < 256; i++)
	{
		hVSurface->pPalette[i].r = pSrcPalette[i].peRed;
		hVSurface->pPalette[i].g = pSrcPalette[i].peGreen;
		hVSurface->pPalette[i].b = pSrcPalette[i].peBlue;
	}

	if (hVSurface->p16BPPPalette != NULL) MemFree(hVSurface->p16BPPPalette);
	hVSurface->p16BPPPalette = Create16BPPPalette(pSrcPalette);

	DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, "Video Surface Palette change successfull");
	return TRUE;
}


// Transparency needs to take RGB value and find best fit and place it into DD Surface
// colorkey value.
static BOOLEAN SetVideoSurfaceTransparencyColor(HVSURFACE hVSurface, COLORVAL TransColor)
{
	Assert(hVSurface != NULL);

	SDL_Surface* Surface = hVSurface->surface;
	CHECKF(Surface != NULL);

	// Get right pixel format, based on bit depth
	Uint32 ColorKey;
	switch (hVSurface->ubBitDepth)
	{
		case  8: ColorKey = TransColor;                break;
		case 16: ColorKey = Get16BPPColor(TransColor); break;
	}

	SDL_SetColorKey(Surface, SDL_SRCCOLORKEY, ColorKey);

	return TRUE;
}


BOOLEAN GetVSurfacePaletteEntries(const SGPVSurface* const src, SGPPaletteEntry* const pPalette)
{
	const SDL_Color* p = src->pPalette;
	CHECKF(p != NULL);

	for (UINT32 i = 0; i < 256; i++)
	{
		pPalette[i].peRed   = p[i].r;
		pPalette[i].peGreen = p[i].g;
		pPalette[i].peBlue  = p[i].b;
	}

	return TRUE;
}


BOOLEAN DeleteVideoSurface(SGPVSurface* const vs)
{
	VSURFACE_NODE* prev = NULL;
	VSURFACE_NODE* curr = gpVSurfaceHead;
	while (curr != NULL)
	{
		if (curr->hVSurface == vs)
		{ //Found the node, so detach it and delete it.
			InternalDeleteVideoSurface(vs);

			if (curr == gpVSurfaceHead) gpVSurfaceHead = gpVSurfaceHead->next;
			if (curr == gpVSurfaceTail) gpVSurfaceTail = prev;
			if (prev != NULL) prev->next = curr->next;

#ifdef SGP_VIDEO_DEBUGGING
			if (curr->pName != NULL) MemFree(curr->pName);
			if (curr->pCode != NULL) MemFree(curr->pCode);
#endif

			MemFree(curr);
			guiVSurfaceSize--;
			return TRUE;
		}
		prev = curr;
		curr = curr->next;
	}
	return FALSE;
}


// Deletes all palettes and surfaces
static BOOLEAN InternalDeleteVideoSurface(HVSURFACE hVSurface)
{
	CHECKF(hVSurface != NULL);

	SDL_FreeSurface(hVSurface->surface);

	if (hVSurface->pPalette != NULL)
	{
		MemFree(hVSurface->pPalette);
		hVSurface->pPalette = NULL;
	}

	if (hVSurface->p16BPPPalette != NULL)
	{
		MemFree(hVSurface->p16BPPPalette);
		hVSurface->p16BPPPalette = NULL;
	}

	MemFree(hVSurface);

	return TRUE;
}


// Will drop down into user-defined blitter if 8->16 BPP blitting is being done
BOOLEAN BltVideoSurface(SGPVSurface* const dst, SGPVSurface* const src, const INT32 iDestX, const INT32 iDestY, const SGPRect* const SRect)
{
	Assert(dst != NULL);
	Assert(src != NULL);

	// First check BPP values for compatibility
	if (dst->ubBitDepth < src->ubBitDepth)
	{
		DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible BPP values with src and dest Video Surfaces for blitting");
		return FALSE;
	}

	// Check for source coordinate options - specific rect or full src dimensions
	SDL_Rect src_rect;
	if (SRect != NULL)
	{
		src_rect.x = SRect->iLeft;
		src_rect.y = SRect->iTop;
		src_rect.w = SRect->iRight  - SRect->iLeft;
		src_rect.h = SRect->iBottom - SRect->iTop;
	}
	else
	{
		// Here, use default, which is entire Video Surface
		// Check Sizes, SRC size MUST be <= DEST size
		if (dst->usHeight < src->usHeight)
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible height size given in Video Surface blit");
			return FALSE;
		}
		if (dst->usWidth < src->usWidth)
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible height size given in Video Surface blit");
			return FALSE;
		}

		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.w = src->usWidth;
		src_rect.h = src->usHeight;
	}

	if (dst->ubBitDepth == src->ubBitDepth)
	{
		SDL_Rect dstrect;
		dstrect.x = iDestX;
		dstrect.y = iDestY;
		SDL_BlitSurface(src->surface, &src_rect, dst->surface, &dstrect);
	}
	else
	{
		SGPRect r;
		r.iLeft   = src_rect.x;
		r.iTop    = src_rect.y;
		r.iRight  = src_rect.x + src_rect.w;
		r.iBottom = src_rect.y + src_rect.h;
		UINT32  spitch;
		UINT8*  s_buf = LockVideoSurfaceBuffer(src, &spitch);
		UINT32  dpitch;
		UINT16* d_buf = (UINT16*)LockVideoSurfaceBuffer(dst, &dpitch);
		Blt8BPPDataSubTo16BPPBuffer(d_buf, dpitch, src, s_buf, spitch, iDestX, iDestY, &r);
		UnLockVideoSurfaceBuffer(dst);
		UnLockVideoSurfaceBuffer(src);
	}

	return TRUE;
}


static HVSURFACE CreateVideoSurfaceFromDDSurface(SDL_Surface* surface)
{
	HVSURFACE hVSurface = MemAlloc(sizeof(*hVSurface));
	hVSurface->surface       = surface;
	hVSurface->usHeight      = surface->h;
	hVSurface->usWidth       = surface->w;
	hVSurface->ubBitDepth    = surface->format->BitsPerPixel;
	hVSurface->pPalette      = NULL;
	hVSurface->p16BPPPalette = NULL;

	DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Success in Creating Video Surface from DD Surface");

	return hVSurface;
}


static BOOLEAN InternalShadowVideoSurfaceRect(SGPVSurface* const dst, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, const UINT16* const filter_table)
{
	if (X1 < 0) X1 = 0;
	if (X2 < 0) return FALSE;

	if (Y2 < 0) return FALSE;
	if (Y1 < 0) Y1 = 0;

	if (X2 >= dst->usWidth)  X2 = dst->usWidth - 1;
	if (Y2 >= dst->usHeight) Y2 = dst->usHeight - 1;

	if (X1 >= dst->usWidth)  return FALSE;
	if (Y1 >= dst->usHeight) return FALSE;

	if (X2 - X1 <= 0) return FALSE;
	if (Y2 - Y1 <= 0) return FALSE;

	SGPRect area;
	area.iTop    = Y1;
	area.iBottom = Y2;
	area.iLeft   = X1;
	area.iRight  = X2;

	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(dst, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	BOOLEAN Ret = Blt16BPPBufferFilterRect(pBuffer, uiPitch, filter_table, &area);

	UnLockVideoSurface(dst);
	return Ret;
}


BOOLEAN ShadowVideoSurfaceRect(SGPVSurface* const dst, const INT32 X1, const INT32 Y1, const INT32 X2, const INT32 Y2)
{
	return InternalShadowVideoSurfaceRect(dst, X1, Y1, X2, Y2, ShadeTable);
}


BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(SGPVSurface* const dst, const INT32 X1, const INT32 Y1, const INT32 X2, const INT32 Y2)
{
	return InternalShadowVideoSurfaceRect(dst, X1, Y1, X2, Y2, IntensityTable);
}


BOOLEAN BltStretchVideoSurface(SGPVSurface* const dst, const SGPVSurface* const src, SGPRect* const SrcRect, SGPRect* const DestRect)
{
	//if the 2 images are not both 16bpp, return FALSE
	if (dst->ubBitDepth != 16 || src->ubBitDepth != 16) return FALSE;

	const UINT32  s_pitch = src->surface->pitch >> 1;
	const UINT32  d_pitch = dst->surface->pitch >> 1;
	const UINT16* os      = (const UINT16*)src->surface->pixels + s_pitch * SrcRect->iTop  + SrcRect->iLeft;
	UINT16*       d       =       (UINT16*)dst->surface->pixels + d_pitch * DestRect->iTop + DestRect->iLeft;

	const UINT width  = DestRect->iRight  - DestRect->iLeft;
	const UINT height = DestRect->iBottom - DestRect->iTop;
	const UINT dx = SrcRect->iRight  - SrcRect->iLeft;
	const UINT dy = SrcRect->iBottom - SrcRect->iTop;
	UINT py = 0;
	if (src->surface->flags & SDL_SRCCOLORKEY)
	{
		const UINT16 key = src->surface->format->colorkey;
		for (UINT iy = 0; iy < height; ++iy)
		{
			const UINT16* s = os;
			UINT px = 0;
			for (UINT ix = 0; ix < width; ++ix)
			{
				if (*s != key) *d = *s;
				++d;
				px += dx;
				for (; px >= width; px -= width) ++s;
			}
			d += d_pitch - width;
			py += dy;
			for (; py >= height; py -= height) os += s_pitch;
		}
	}
	else
	{
		for (UINT iy = 0; iy < height; ++iy)
		{
			const UINT16* s = os;
			UINT px = 0;
			for (UINT ix = 0; ix < width; ++ix)
			{
				*d++ = *s;
				px += dx;
				for (; px >= width; px -= width) ++s;
			}
			d += d_pitch - width;
			py += dy;
			for (; py >= height; py -= height) os += s_pitch;
		}
	}

	return TRUE;
}


BOOLEAN BltVideoSurfaceOnce(SGPVSurface* const dst, const char* const filename, const INT32 x, const INT32 y)
{
	SGPVSurface* const src = AddVideoSurfaceFromFile(filename);
	if (src == NO_VSURFACE) return FALSE;
	BltVideoSurface(dst, src, x, y, NULL);
	DeleteVideoSurface(src);
	return TRUE;
}


#ifdef SGP_VIDEO_DEBUGGING
typedef struct DUMPINFO
{
	UINT32 Counter;
	char Name[256];
	char Code[256];
} DUMPINFO;


void DumpVSurfaceInfoIntoFile(const char* filename, BOOLEAN fAppend)
{
	if (!guiVSurfaceSize) return;

	FILE* fp = fopen(filename, fAppend ? "a" : "w");
	Assert(fp != NULL);

	//Allocate enough strings and counters for each node.
	DUMPINFO* Info = MemAlloc(sizeof(*Info) * guiVSurfaceSize);
	memset(Info, 0, sizeof(*Info) * guiVSurfaceSize);

	//Loop through the list and record every unique filename and count them
	UINT32 uiUniqueID = 0;
	for (const VSURFACE_NODE* curr = gpVSurfaceHead; curr != NULL; curr = curr->next)
	{
		const char* Name = curr->pName;
		const char* Code = curr->pCode;
		BOOLEAN fFound = FALSE;
		for (UINT32 i = 0; i < uiUniqueID; i++)
		{
			if (strcasecmp(Name, Info[i].Name) == 0 && strcasecmp(Code, Info[i].Code) == 0)
			{ //same string
				fFound = TRUE;
				Info[i].Counter++;
				break;
			}
		}
		if (!fFound)
		{
			strcpy(Info[uiUniqueID].Name, Name);
			strcpy(Info[uiUniqueID].Code, Code);
			Info[uiUniqueID].Counter++;
			uiUniqueID++;
		}
	}

	//Now dump the info.
	fprintf(fp, "-----------------------------------------------\n");
	fprintf(fp, "%d unique vSurface names exist in %d VSurfaces\n", uiUniqueID, guiVSurfaceSize);
	fprintf(fp, "-----------------------------------------------\n\n");
	for (UINT32 i = 0; i < uiUniqueID; i++)
	{
		fprintf(fp, "%d occurrences of %s\n%s\n\n", Info[i].Counter, Info[i].Name, Info[i].Code);
	}
	fprintf(fp, "\n-----------------------------------------------\n\n");

	MemFree(Info);
	fclose(fp);
}


static void RecordVSurface(const char* Filename, UINT32 LineNum, const char* SourceFile)
{
	//record the filename of the vsurface (some are created via memory though)
	gpVSurfaceTail->pName = MemAlloc(strlen(Filename) + 1);
	strcpy(gpVSurfaceTail->pName, Filename);

	//record the code location of the calling creating function.
	char str[256];
	sprintf(str, "%s -- line(%d)", SourceFile, LineNum);
	gpVSurfaceTail->pCode = MemAlloc(strlen(str) + 1);
	strcpy(gpVSurfaceTail->pCode, str);
}


SGPVSurface* AddAndRecordVSurface(const UINT16 Width, const UINT16 Height, const UINT8 BitDepth, const UINT32 LineNum, const char* const SourceFile)
{
	SGPVSurface* const vs = AddVideoSurface(Width, Height, BitDepth);
	if (vs != NO_VSURFACE) RecordVSurface("<EMPTY>", LineNum, SourceFile);
	return vs;
}


SGPVSurface* AddAndRecordVSurfaceFromFile(const char* const Filename, const UINT32 LineNum, const char* const SourceFile)
{
	SGPVSurface* const vs = AddVideoSurfaceFromFile(Filename);
	if (vs != NO_VSURFACE) RecordVSurface(Filename, LineNum, SourceFile);
	return vs;
}

#endif
