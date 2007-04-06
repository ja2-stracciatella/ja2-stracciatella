#include "Debug.h"
#include "Local.h"
#include "MemMan.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"


static BOOLEAN FillSurfaceRect(HVSURFACE hDestVSurface, SDL_Rect* Rect, UINT16 Color);
static void DeletePrimaryVideoSurfaces(void);


typedef struct VSURFACE_NODE
{
	HVSURFACE hVSurface;
	UINT32 uiIndex;
  struct VSURFACE_NODE* next;

#ifdef SGP_VIDEO_DEBUGGING
	char* pName;
	char* pCode;
#endif
} VSURFACE_NODE;

static VSURFACE_NODE* gpVSurfaceHead = NULL;
static VSURFACE_NODE* gpVSurfaceTail = NULL;
static UINT32 guiVSurfaceIndex = 0;
UINT32				guiVSurfaceSize = 0;

#ifdef _DEBUG
enum
{
	DEBUGSTR_NONE,
	DEBUGSTR_SETVIDEOSURFACETRANSPARENCY,
	DEBUGSTR_BLTVIDEOSURFACE_DST,
	DEBUGSTR_BLTVIDEOSURFACE_SRC,
	DEBUGSTR_COLORFILLVIDEOSURFACEAREA,
	DEBUGSTR_SHADOWVIDEOSURFACERECT,
	DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST,
	DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC,
	DEBUGSTR_DELETEVIDEOSURFACEFROMINDEX
};

static UINT8 gubVSDebugCode = 0;

static void CheckValidVSurfaceIndex(UINT32 uiIndex);
#endif


static HVSURFACE ghBackBuffer = NULL;
static HVSURFACE ghFrameBuffer = NULL;
static HVSURFACE ghMouseBuffer = NULL;


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


BOOLEAN ShutdownVideoSurfaceManager(void)
{
  DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Shutting down the Video Surface manager");

	// Delete primary viedeo surfaces
	DeletePrimaryVideoSurfaces();

	while (gpVSurfaceHead != NULL)
	{
		VSURFACE_NODE* curr = gpVSurfaceHead;
		gpVSurfaceHead = gpVSurfaceHead->next;
		DeleteVideoSurface(curr->hVSurface);
#ifdef SGP_VIDEO_DEBUGGING
		if (curr->pName != NULL) MemFree(curr->pName);
		if (curr->pCode != NULL) MemFree(curr->pCode);
#endif
		MemFree(curr);
	}
	gpVSurfaceHead = NULL;
	gpVSurfaceTail = NULL;
	guiVSurfaceIndex = 0;
	guiVSurfaceSize = 0;
	return TRUE;
}


static UINT32 AddStandardVideoSurface(HVSURFACE hVSurface)
{
	if (hVSurface == NULL) return NO_VSURFACE;

	VSURFACE_NODE* Node = MemAlloc(sizeof(*Node));
	Assert(Node != NULL); // out of memory?
	Node->hVSurface = hVSurface;

	guiVSurfaceIndex += 2;
	/* Unlikely that we will ever use 2 billion VSurfaces! We would have to
	 * create about 70 VSurfaces per second for 1 year straight to achieve this */
	Assert(guiVSurfaceIndex < 0xFFFFFFF0);

	Node->uiIndex   = guiVSurfaceIndex;
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
	return guiVSurfaceIndex;
}


static HVSURFACE CreateVideoSurface(UINT16 usWidth, UINT16 usHeight, UINT8 ubBitDepth);


#undef AddVideoSurface
#undef AddVideoSurfaceFromFile


UINT32 AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth)
{
	HVSURFACE hVSurface = CreateVideoSurface(Width, Height, BitDepth);
	return AddStandardVideoSurface(hVSurface);
}


UINT32 AddVideoSurfaceFromFile(const char* Filename)
{
	HVSURFACE hVSurface = CreateVideoSurfaceFromFile(Filename);
	return AddStandardVideoSurface(hVSurface);
}


static BYTE* LockVideoSurfaceBuffer(HVSURFACE hVSurface, UINT32* pPitch);
static void UnLockVideoSurfaceBuffer(HVSURFACE hVSurface);


BYTE* LockVideoSurface(UINT32 uiVSurface, UINT32* puiPitch)
{
	switch (uiVSurface)
	{
#ifdef JA2
  	case BACKBUFFER:   return LockBackBuffer(puiPitch);
#endif
  	case FRAME_BUFFER: return LockFrameBuffer(puiPitch);
  	case MOUSE_BUFFER: return LockMouseBuffer(puiPitch);
	}

	for (const VSURFACE_NODE* i = gpVSurfaceHead; i != NULL; i = i->next)
	{
		if (i->uiIndex == uiVSurface)
		{
			return LockVideoSurfaceBuffer(i->hVSurface, puiPitch);
		}
	}
	return NULL;
}


void UnLockVideoSurface(UINT32 uiVSurface)
{
	switch (uiVSurface)
	{
#ifdef JA2
  	case BACKBUFFER:   UnlockBackBuffer();  return;
#endif
  	case FRAME_BUFFER: UnlockFrameBuffer(); return;
  	case MOUSE_BUFFER: UnlockMouseBuffer(); return;
	}

	for (const VSURFACE_NODE* i = gpVSurfaceHead; i != NULL; i = i->next)
	{
		if (i->uiIndex == uiVSurface)
		{
			UnLockVideoSurfaceBuffer(i->hVSurface);
			return;
		}
	}
}


static BOOLEAN SetVideoSurfaceTransparencyColor(HVSURFACE hVSurface, COLORVAL TransColor);


BOOLEAN SetVideoSurfaceTransparency(UINT32 uiIndex, COLORVAL TransColor)
{
#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_SETVIDEOSURFACETRANSPARENCY;
#endif
	HVSURFACE hVSurface = GetVideoSurface(uiIndex);
	CHECKF(hVSurface != NULL);

	SetVideoSurfaceTransparencyColor(hVSurface, TransColor);
	return TRUE;
}


HVSURFACE GetVideoSurface(UINT32 uiIndex)
{
#ifdef _DEBUG
	CheckValidVSurfaceIndex(uiIndex);
#endif

	switch (uiIndex)
	{
  	case BACKBUFFER:   return ghBackBuffer;
  	case FRAME_BUFFER: return ghFrameBuffer;
  	case MOUSE_BUFFER: return ghMouseBuffer;
	}

	for (const VSURFACE_NODE* i = gpVSurfaceHead; i != NULL; i = i->next)
	{
		if (i->uiIndex == uiIndex) return i->hVSurface;
	}
  return NULL;
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
	ghBackBuffer = CreateVideoSurfaceFromDDSurface(pSurface);
	CHECKF(ghBackBuffer != NULL);

	pSurface = GetMouseBufferObject();
	CHECKF(pSurface != NULL);
	ghMouseBuffer = CreateVideoSurfaceFromDDSurface(pSurface);
	CHECKF(ghMouseBuffer != NULL);
#endif

	pSurface = GetFrameBufferObject();
	CHECKF(pSurface != NULL);
	ghFrameBuffer = CreateVideoSurfaceFromDDSurface(pSurface);
	CHECKF(ghFrameBuffer != NULL);

	return TRUE;
}


static void DeletePrimaryVideoSurfaces(void)
{
	if (ghBackBuffer != NULL)
	{
		DeleteVideoSurface(ghBackBuffer);
		ghBackBuffer = NULL;
	}

  if (ghFrameBuffer != NULL)
	{
		DeleteVideoSurface(ghFrameBuffer);
		ghFrameBuffer = NULL;
	}

	if (ghMouseBuffer != NULL)
	{
		DeleteVideoSurface(ghMouseBuffer);
		ghMouseBuffer = NULL;
	}
}


static BOOLEAN BltVideoSurfaceToVideoSurface(HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, INT32 iDestX, INT32 iDestY, const SGPRect* SRect);


/* Given an index to the dest and src vobject contained in our private VSurface
 * list */
BOOLEAN BltVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, INT32 iDestX, INT32 iDestY, const SGPRect* SrcRect)
{
#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_BLTVIDEOSURFACE_DST;
#endif
	HVSURFACE hDestVSurface = GetVideoSurface(uiDestVSurface);
	if (hDestVSurface == NULL) return FALSE;

#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_BLTVIDEOSURFACE_SRC;
#endif
	HVSURFACE hSrcVSurface = GetVideoSurface(uiSrcVSurface);
	if (hSrcVSurface == NULL) return FALSE;

	return BltVideoSurfaceToVideoSurface(hDestVSurface, hSrcVSurface, iDestX, iDestY, SrcRect);
}


/* Fills an rectangular area with a specified color value. */
BOOLEAN ColorFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP)
{
#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_COLORFILLVIDEOSURFACEAREA;
#endif
	HVSURFACE hDestVSurface = GetVideoSurface(uiDestVSurface);
	if (hDestVSurface == NULL) return FALSE;

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

	return FillSurfaceRect(hDestVSurface, &Rect, Color16BPP);
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


static BOOLEAN SetVideoSurfaceDataFromHImage(HVSURFACE hVSurface, HIMAGE hImage, UINT16 usX, UINT16 usY, const SGPRect* pSrcRect);


HVSURFACE CreateVideoSurfaceFromFile(const char* Filename)
{
	HIMAGE hImage = CreateImage(Filename, IMAGE_ALLIMAGEDATA);
	if (hImage == NULL)
	{
		DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Invalid Image Filename given");
		return NULL;
	}

	HVSURFACE hVSurface = CreateVideoSurface(hImage->usWidth, hImage->usHeight, hImage->ubBitDepth);
	if (hVSurface != NULL)
	{
		SGPRect tempRect;
		tempRect.iLeft   = 0;
		tempRect.iTop    = 0;
		tempRect.iRight  = hImage->usWidth  - 1;
		tempRect.iBottom = hImage->usHeight - 1;
		SetVideoSurfaceDataFromHImage(hVSurface, hImage, 0, 0, &tempRect);

		if (hImage->ubBitDepth == 8)
		{
			SetVideoSurfacePalette(hVSurface, hImage->pPalette);
		}
	}

	DestroyImage(hImage);
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
BOOLEAN SetVideoSurfacePalette(HVSURFACE hVSurface, const SGPPaletteEntry* pSrcPalette)
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


BOOLEAN GetVSurfacePaletteEntries(HVSURFACE hVSurface, SGPPaletteEntry* pPalette)
{
	CHECKF(hVSurface->pPalette != NULL);

	for (UINT32 i = 0; i < 256; i++)
	{
		pPalette[i].peRed   = hVSurface->pPalette[i].r;
		pPalette[i].peGreen = hVSurface->pPalette[i].g;
		pPalette[i].peBlue  = hVSurface->pPalette[i].b;
	}

	return TRUE;
}


BOOLEAN DeleteVideoSurfaceFromIndex(UINT32 uiIndex)
{
#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_DELETEVIDEOSURFACEFROMINDEX;
	CheckValidVSurfaceIndex(uiIndex);
#endif

	VSURFACE_NODE* prev = NULL;
	VSURFACE_NODE* curr = gpVSurfaceHead;
	while (curr != NULL)
	{
		if (curr->uiIndex == uiIndex)
		{ //Found the node, so detach it and delete it.
			DeleteVideoSurface(curr->hVSurface);

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


// Deletes all palettes, surfaces and region data
BOOLEAN DeleteVideoSurface(HVSURFACE hVSurface)
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


static BOOLEAN GetVSurfaceRect(HVSURFACE hVSurface, SGPRect* pRect)
{
	Assert(hVSurface != NULL);
	Assert(pRect != NULL);

	pRect->iLeft   = 0;
	pRect->iTop    = 0;
	pRect->iRight  = hVSurface->usWidth;
	pRect->iBottom = hVSurface->usHeight;

	return TRUE;
}


// Will drop down into user-defined blitter if 8->16 BPP blitting is being done
static BOOLEAN BltVideoSurfaceToVideoSurface(HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, INT32 iDestX, INT32 iDestY, const SGPRect* SRect)
{
	Assert(hDestVSurface != NULL);
	Assert(hSrcVSurface != NULL);

	// Check for source coordinate options - specific rect or full src dimensions
	SGPRect SrcRect;
	if (SRect != NULL)
	{
		SrcRect = *SRect;
	}
	else
	{
		// Here, use default, which is entire Video Surface
		// Check Sizes, SRC size MUST be <= DEST size
		if (hDestVSurface->usHeight < hSrcVSurface->usHeight)
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible height size given in Video Surface blit");
			return FALSE;
		}
		if (hDestVSurface->usWidth < hSrcVSurface->usWidth)
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible height size given in Video Surface blit");
			return FALSE;
		}

		SrcRect.iTop    = (int)0;
		SrcRect.iLeft   = (int)0;
		SrcRect.iBottom = (int)hSrcVSurface->usHeight;
		SrcRect.iRight  = (int)hSrcVSurface->usWidth;
	}

	// clipping -- added by DB
	SGPRect DestRect;
	GetVSurfaceRect(hDestVSurface, &DestRect);
	UINT32 uiWidth  = SrcRect.iRight  - SrcRect.iLeft;
	UINT32 uiHeight = SrcRect.iBottom - SrcRect.iTop;

	// check for position entirely off the screen
	if (iDestX >= DestRect.iRight)  return FALSE;
	if (iDestY >= DestRect.iBottom) return FALSE;
	if (iDestX + (INT32)uiWidth  < DestRect.iLeft) return FALSE;
	if (iDestY + (INT32)uiHeight < DestRect.iTop)  return FALSE;

	if (iDestX + (INT32)uiWidth >= DestRect.iRight)
	{
		SrcRect.iRight -= (iDestX + uiWidth) - DestRect.iRight;
		uiWidth        -= (iDestX + uiWidth) - DestRect.iRight;
	}
	if (iDestY + (INT32)uiHeight >= DestRect.iBottom)
	{
		SrcRect.iBottom -= (iDestY + uiHeight) - DestRect.iBottom;
		uiHeight        -= (iDestY + uiHeight) - DestRect.iBottom;
	}
	if (iDestX < DestRect.iLeft)
	{
		SrcRect.iLeft += DestRect.iLeft - iDestX;
		uiWidth       -= DestRect.iLeft - iDestX;
		iDestX         = DestRect.iLeft;
	}
	if (iDestY < DestRect.iTop)
	{
		SrcRect.iTop += DestRect.iTop - iDestY;
		uiHeight     -= DestRect.iTop - iDestY;
		iDestY        = DestRect.iTop;
	}

	// First check BPP values for compatibility
	if (hDestVSurface->ubBitDepth == 16 && hSrcVSurface->ubBitDepth == 16)
	{
// For testing with non-DDraw blitting, uncomment to test -- DB
#ifndef JA2
		UINT32 uiSrcPitch;
		const UINT16* pSrcSurface16 = (UINT16*)LockVideoSurfaceBuffer(hSrcVSurface, &uiSrcPitch);
		if (pSrcSurface16 == NULL)
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Failed on lock of 16BPP surface for blitting");
			return(FALSE);
		}

		UINT32 uiDestPitch;
		UINT16* pDestSurface16 = (UINT16*)LockVideoSurfaceBuffer(hDestVSurface, &uiDestPitch);
		if (pDestSurface16 == NULL)
		{
			UnLockVideoSurfaceBuffer(hSrcVSurface);
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Failed on lock of 16BPP dest surface for blitting");
			return FALSE;
		}

		Blt16BPPTo16BPP(pDestSurface16, uiDestPitch, pSrcSurface16, uiSrcPitch, iDestX, iDestY, SrcRect.iLeft, SrcRect.iTop, uiWidth, uiHeight);
		UnLockVideoSurfaceBuffer(hSrcVSurface);
		UnLockVideoSurfaceBuffer(hDestVSurface);
		return TRUE;
#endif

		CHECKF(BltVSurfaceUsingDD(hDestVSurface, hSrcVSurface, iDestX, iDestY, &SrcRect));
	}
	else if (hDestVSurface->ubBitDepth == 8 && hSrcVSurface->ubBitDepth == 8)
	{
		UINT32 uiSrcPitch;
		const UINT8* pSrcSurface8 = (UINT8*)LockVideoSurfaceBuffer(hSrcVSurface, &uiSrcPitch);
		if (pSrcSurface8 == NULL)
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Failed on lock of 8BPP surface for blitting");
			return FALSE;
		}

		UINT32 uiDestPitch;
		UINT8* pDestSurface8 = (UINT8*)LockVideoSurfaceBuffer(hDestVSurface, &uiDestPitch);
		if (pDestSurface8 == NULL)
		{
			UnLockVideoSurfaceBuffer(hSrcVSurface);
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Failed on lock of 8BPP dest surface for blitting");
			return FALSE;
		}

		Blt8BPPTo8BPP(pDestSurface8, uiDestPitch, pSrcSurface8, uiSrcPitch, iDestX, iDestY, SrcRect.iLeft, SrcRect.iTop, uiWidth, uiHeight);
		UnLockVideoSurfaceBuffer(hSrcVSurface);
		UnLockVideoSurfaceBuffer(hDestVSurface);
	}
	else
	{
		DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible BPP values with src and dest Video Surfaces for blitting");
		return FALSE;
	}

	return TRUE;
}


static HVSURFACE CreateVideoSurfaceFromDDSurface(SDL_Surface* surface)
{
	HVSURFACE hVSurface = MemAlloc(sizeof(*hVSurface));
	hVSurface->surface    = surface;
	hVSurface->usHeight   = surface->h;
	hVSurface->usWidth    = surface->w;
	hVSurface->ubBitDepth = surface->format->BitsPerPixel;

	if (surface->format->palette != NULL) // XXX necessary?
	{
		SGPPaletteEntry SGPPalette[256];
		hVSurface->pPalette = MemAlloc(sizeof(*hVSurface->pPalette) * 256);
		for (UINT32 i = 0; i < 256; i++)
		{
			hVSurface->pPalette[i] = surface->format->palette->colors[i];
			SGPPalette[i].peRed   = surface->format->palette->colors[i].r;
			SGPPalette[i].peGreen = surface->format->palette->colors[i].g;
			SGPPalette[i].peBlue  = surface->format->palette->colors[i].b;
		}
		hVSurface->p16BPPPalette = Create16BPPPalette(SGPPalette);
	}
	else
	{
		hVSurface->pPalette = NULL;
		hVSurface->p16BPPPalette = NULL;
	}

	DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Success in Creating Video Surface from DD Surface");

	return hVSurface;
}


static BOOLEAN FillSurfaceRect(HVSURFACE hDestVSurface, SDL_Rect* Rect, UINT16 Color)
{
	Assert(hDestVSurface != NULL);
	CHECKF(Rect != NULL);

	SDL_FillRect(hDestVSurface->surface, Rect, Color);
	return TRUE;
}


BOOLEAN BltVSurfaceUsingDD(HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, INT32 iDestX, INT32 iDestY, SGPRect* SrcRect)
{
	SDL_Rect srcrect;
	srcrect.x = SrcRect->iLeft;
	srcrect.y = SrcRect->iTop;
	srcrect.w = SrcRect->iRight  - SrcRect->iLeft;
	srcrect.h = SrcRect->iBottom - SrcRect->iTop;

	SDL_Rect dstrect;
	dstrect.x = iDestX;
	dstrect.y = iDestY;

	SDL_BlitSurface(hSrcVSurface->surface, &srcrect, hDestVSurface->surface, &dstrect);
	return TRUE;
}

BOOLEAN Blt16BPPBufferShadowRectAlternateTable(UINT16 *pBuffer, UINT32 uiDestPitchBYTES, SGPRect *area);


static BOOLEAN InternalShadowVideoSurfaceRect(UINT32 uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, BOOLEAN fLowPercentShadeTable)
{
#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_SHADOWVIDEOSURFACERECT;
#endif
	HVSURFACE hVSurface = GetVideoSurface(uiDestVSurface);
	CHECKF(hVSurface != NULL);

	if (X1 < 0) X1 = 0;
	if (X2 < 0) return FALSE;

	if (Y2 < 0) return FALSE;
	if (Y1 < 0) Y1 = 0;

	if (X2 >= hVSurface->usWidth)  X2 = hVSurface->usWidth - 1;
	if (Y2 >= hVSurface->usHeight) Y2 = hVSurface->usHeight - 1;

	if (X1 >= hVSurface->usWidth)  return FALSE;
	if (Y1 >= hVSurface->usHeight) return FALSE;

	if (X2 - X1 <= 0) return FALSE;
	if (Y2 - Y1 <= 0) return FALSE;

	SGPRect area;
	area.iTop    = Y1;
	area.iBottom = Y2;
	area.iLeft   = X1;
	area.iRight  = X2;

	UINT32 uiPitch;
	UINT16* pBuffer = (UINT16*)LockVideoSurface(uiDestVSurface, &uiPitch);
	if (pBuffer == NULL) return FALSE;

	BOOLEAN Ret;
	if (!fLowPercentShadeTable)
	{
		Ret = Blt16BPPBufferShadowRect(pBuffer, uiPitch, &area);
	}
	else
	{
		Ret = Blt16BPPBufferShadowRectAlternateTable(pBuffer, uiPitch, &area);
	}

	UnLockVideoSurface(uiDestVSurface);
	return Ret;
}


BOOLEAN ShadowVideoSurfaceRect(UINT32 uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2)
{
	return InternalShadowVideoSurfaceRect(uiDestVSurface, X1, Y1, X2, Y2, FALSE);
}


BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(UINT32 uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2)
{
	return InternalShadowVideoSurfaceRect(uiDestVSurface, X1, Y1, X2, Y2, TRUE);
}


BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, SGPRect* SrcRect, SGPRect* DestRect)
{
#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST;
#endif
	HVSURFACE hDestVSurface = GetVideoSurface(uiDestVSurface);
	if (hDestVSurface == NULL) return FALSE;

#ifdef _DEBUG
	gubVSDebugCode = DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC;
#endif
	HVSURFACE hSrcVSurface = GetVideoSurface(uiSrcVSurface);
	if (hSrcVSurface == NULL) return FALSE;

	//if the 2 images are not both 16bpp, return FALSE
	if (hDestVSurface->ubBitDepth != 16 || hSrcVSurface->ubBitDepth != 16) return FALSE;

	const UINT16* os = (const UINT16*)hSrcVSurface->surface->pixels  + (hSrcVSurface->surface->pitch  >> 1) * SrcRect->iTop  + SrcRect->iLeft;
	UINT16*       d  =       (UINT16*)hDestVSurface->surface->pixels + (hDestVSurface->surface->pitch >> 1) * DestRect->iTop + DestRect->iLeft;

	const UINT width  = DestRect->iRight  - DestRect->iLeft;
	const UINT height = DestRect->iBottom - DestRect->iTop;
	const UINT dx = SrcRect->iRight  - SrcRect->iLeft;
	const UINT dy = SrcRect->iBottom - SrcRect->iTop;
	UINT py = 0;
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
		d += (hDestVSurface->surface->pitch >> 1) - width;
		py += dy;
		for (; py >= height; py -= height) os += hSrcVSurface->surface->pitch >> 1;
	}

	return TRUE;
}


BOOLEAN ShadowVideoSurfaceImage(UINT32 uiDestVSurface, HVOBJECT hImageHandle, INT32 iPosX, INT32 iPosY)
{
	UINT32 Width  = hImageHandle->pETRLEObject->usWidth;
	UINT32 Height = hImageHandle->pETRLEObject->usHeight;

	//Horizontal shadow
	ShadowVideoSurfaceRect(uiDestVSurface, iPosX + 3, iPosY + Height, iPosX + Width, iPosY + Height + 3);

	//vertical shadow
	ShadowVideoSurfaceRect(uiDestVSurface, iPosX + Width, iPosY + 3, iPosX + Width + 3, iPosY + Height);
	return TRUE;
}


#ifdef _DEBUG
void CheckValidVSurfaceIndex(UINT32 uiIndex)
{
	/* 0xFFFFFFFF -> deleted
	 * odd number -> VObject */
	if (uiIndex == 0xFFFFFFFF || (uiIndex % 2 && uiIndex < 0xFFFFFFF0)
	{
		const char* str;
		switch (gubVSDebugCode)
		{
			default:
			case DEBUGSTR_NONE:                        str = "GetVideoSurface";               break;
			case DEBUGSTR_SETVIDEOSURFACETRANSPARENCY: str = "SetVideoSurfaceTransparency";   break;
			case DEBUGSTR_BLTVIDEOSURFACE_DST:         str = "BltVideoSurface (dest)";        break;
			case DEBUGSTR_BLTVIDEOSURFACE_SRC:         str = "BltVideoSurface (src)";         break;
			case DEBUGSTR_COLORFILLVIDEOSURFACEAREA:   str = "ColorFillVideoSurfaceArea";     break;
			case DEBUGSTR_SHADOWVIDEOSURFACERECT:      str = "ShadowVideoSurfaceRect";        break;
			case DEBUGSTR_BLTSTRETCHVIDEOSURFACE_DST:  str = "BltStretchVideoSurface (dest)"; break;
			case DEBUGSTR_BLTSTRETCHVIDEOSURFACE_SRC:  str = "BltStretchVideoSurface (src)";  break;
			case DEBUGSTR_DELETEVIDEOSURFACEFROMINDEX: str = "DeleteVideoSurfaceFromIndex";   break;
		}
		if (uiIndex == 0xFFFFFFFF)
		{
			AssertMsg(0, String("Trying to %s with deleted index -1." , str));
		}
		else
		{
			AssertMsg(0, String("Trying to %s using a VOBJECT ID %d!", str, uiIndex));
		}
	}
}
#endif


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


UINT32 AddAndRecordVSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth, UINT32 LineNum, const char* SourceFile)
{
	UINT32 Index = AddVideoSurface(Width, Height, BitDepth);
	if (Index != NO_VSURFACE) RecordVSurface("<EMPTY>", LineNum, SourceFile);
	return Index;
}


UINT32 AddAndRecordVSurfaceFromFile(const char* Filename, UINT32 LineNum, const char* SourceFile)
{
	UINT32 Index = AddVideoSurfaceFromFile(Filename);
	if (Index != NO_VSURFACE) RecordVSurface(Filename, LineNum, SourceFile);
	return Index;
}

#endif
