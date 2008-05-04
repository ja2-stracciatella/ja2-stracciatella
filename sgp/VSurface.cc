#include "Debug.h"
#include "HImage.h"
#include "MemMan.h"
#include "Shading.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"


SGPVSurface::~SGPVSurface()
{
	if (palette_)      MemFree(palette_);
	if (p16BPPPalette) MemFree(p16BPPPalette);
}


void SGPVSurface::SetPalette(const SGPPaletteEntry* const src_pal)
{
	// Create palette object if not already done so
	if (palette_ == NULL) palette_ = MALLOCN(SDL_Color, 256);
	SDL_Color* const p = palette_;
	for (UINT32 i = 0; i < 256; i++)
	{
		p[i].r = src_pal[i].peRed;
		p[i].g = src_pal[i].peGreen;
		p[i].b = src_pal[i].peBlue;
	}

	if (p16BPPPalette != NULL) MemFree(p16BPPPalette);
	p16BPPPalette = Create16BPPPalette(src_pal);
}


void SGPVSurface::GetPalette(SGPPaletteEntry* const dst_pal) const
{
	const SDL_Color* const p = palette_;
	for (UINT32 i = 0; i < 256; i++)
	{
		dst_pal[i].peRed   = p[i].r;
		dst_pal[i].peGreen = p[i].g;
		dst_pal[i].peBlue  = p[i].b;
	}
}


void SGPVSurface::SetTransparency(const COLORVAL colour)
{
	Uint32 colour_key;
	switch (BPP())
	{
		case  8: colour_key = colour;                break;
		case 16: colour_key = Get16BPPColor(colour); break;

		default: abort(); // HACK000E
	}
	SDL_SetColorKey(surface_, SDL_SRCCOLORKEY, colour_key);
}


void SGPVSurface::Fill(const UINT16 colour)
{
	SDL_FillRect(surface_, NULL, colour);
}


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


BOOLEAN ShutdownVideoSurfaceManager(void)
{
  DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_0, "Shutting down the Video Surface manager");

	// Delete primary viedeo surfaces
	DeletePrimaryVideoSurfaces();

	while (gpVSurfaceHead != NULL)
	{
		VSURFACE_NODE* curr = gpVSurfaceHead;
		gpVSurfaceHead = gpVSurfaceHead->next;
		delete curr->hVSurface;
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

	VSURFACE_NODE* const Node = MALLOC(VSURFACE_NODE);
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
	AutoSGPImage hImage(CreateImage(Filename, IMAGE_ALLIMAGEDATA));
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

		if (hImage->ubBitDepth == 8) vs->SetPalette(hImage->pPalette);
	}

	AddStandardVideoSurface(vs);
	return vs;
}


static BOOLEAN SetPrimaryVideoSurfaces(void)
{
	SDL_Surface* pSurface;

	// Delete surfaces if they exist
	DeletePrimaryVideoSurfaces();

#ifdef JA2
	pSurface = GetBackBufferObject();
	CHECKF(pSurface != NULL);
	g_back_buffer = new SGPVSurface(pSurface);

	pSurface = GetMouseBufferObject();
	CHECKF(pSurface != NULL);
	g_mouse_buffer = new SGPVSurface(pSurface);
#endif

	pSurface = GetFrameBufferObject();
	CHECKF(pSurface != NULL);
	g_frame_buffer = new SGPVSurface(pSurface);

	return TRUE;
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


void BltVideoSurfaceHalf(SGPVSurface* const dst, SGPVSurface* const src, const INT32 DestX, const INT32 DestY, const SGPRect* const SrcRect)
{
	SGPVSurface::Lock lsrc(src);
	SGPVSurface::Lock ldst(dst);
	UINT8*  const SrcBuf         = lsrc.Buffer<UINT8>();
	UINT32  const SrcPitchBYTES  = lsrc.Pitch();
	UINT16* const DestBuf        = ldst.Buffer<UINT16>();
	UINT32  const DestPitchBYTES = ldst.Pitch();
	if (SrcRect == NULL)
	{
		Blt8BPPDataTo16BPPBufferHalf(DestBuf, DestPitchBYTES, src, SrcBuf, SrcPitchBYTES, DestX, DestY);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferHalfRect(DestBuf, DestPitchBYTES, src, SrcBuf, SrcPitchBYTES, DestX, DestY, SrcRect);
	}
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
	SDL_FillRect(dst->surface_, &Rect, Color16BPP);

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

	SGPVSurface* const hVSurface = new SGPVSurface(surface);

	DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_3, "Success in Creating Video Surface");

	return hVSurface;
}


// Given an HIMAGE object, blit imagery into existing Video Surface. Can be from 8->16 BPP
static BOOLEAN SetVideoSurfaceDataFromHImage(HVSURFACE hVSurface, HIMAGE hImage, UINT16 usX, UINT16 usY, const SGPRect* pSrcRect)
{
	Assert(hVSurface != NULL);
	Assert(hImage != NULL);

	// Get Size of hImage and determine if it can fit
	CHECKF(hImage->usWidth  >= hVSurface->Width());
	CHECKF(hImage->usHeight >= hVSurface->Height());

	const UINT8 dst_bpp = hVSurface->BPP();

	UINT32 buffer_bpp;
	switch (dst_bpp)
	{
		case  8: buffer_bpp = BUFFER_8BPP;  break;
		case 16: buffer_bpp = BUFFER_16BPP; break;
		default: abort();
	}

	SGPVSurface::Lock l(hVSurface);
	UINT8* const pDest   = l.Buffer<UINT8>();
	UINT32 const uiPitch = l.Pitch();

	// Effective width ( in PIXELS ) is Pitch ( in bytes ) converted to pitch ( IN PIXELS )
	const UINT16 usEffectiveWidth = uiPitch / (dst_bpp / 8);

	CHECKF(pDest != NULL);

	// Blit Surface
	// If rect is NULL, use entrie image size
	SGPBox box;
	if (pSrcRect == NULL)
	{
		box.x = 0;
		box.y = 0;
		box.w = hImage->usWidth;
		box.h = hImage->usHeight;
	}
	else
	{
		box.x = pSrcRect->iLeft;
		box.y = pSrcRect->iTop;
		box.w = pSrcRect->iRight  - pSrcRect->iLeft + 1;
		box.h = pSrcRect->iBottom - pSrcRect->iTop  + 1;
	}

	// This HIMAGE function will transparently copy buffer
	BOOLEAN Ret = CopyImageToBuffer(hImage, buffer_bpp, pDest, usEffectiveWidth, hVSurface->Height(), usX, usY, &box);
	if (!Ret)
	{
		DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Error Occured Copying HIMAGE to HVSURFACE");
	}
	return Ret;
}


void DeleteVideoSurface(SGPVSurface* const vs)
{
	VSURFACE_NODE* prev = NULL;
	VSURFACE_NODE* curr = gpVSurfaceHead;
	while (curr != NULL)
	{
		if (curr->hVSurface == vs)
		{ //Found the node, so detach it and delete it.
			delete vs;

			if (curr == gpVSurfaceHead) gpVSurfaceHead = gpVSurfaceHead->next;
			if (curr == gpVSurfaceTail) gpVSurfaceTail = prev;
			if (prev != NULL) prev->next = curr->next;

#ifdef SGP_VIDEO_DEBUGGING
			if (curr->pName != NULL) MemFree(curr->pName);
			if (curr->pCode != NULL) MemFree(curr->pCode);
#endif

			MemFree(curr);
			guiVSurfaceSize--;
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}


// Will drop down into user-defined blitter if 8->16 BPP blitting is being done
BOOLEAN BltVideoSurface(SGPVSurface* const dst, SGPVSurface* const src, const INT32 iDestX, const INT32 iDestY, const SGPRect* const SRect)
{
	Assert(dst != NULL);
	Assert(src != NULL);

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
		if (dst->Height() < src->Height())
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible height size given in Video Surface blit");
			return FALSE;
		}
		if (dst->Width() < src->Width())
		{
			DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible height size given in Video Surface blit");
			return FALSE;
		}

		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.w = src->Width();
		src_rect.h = src->Height();
	}

	const UINT8 src_bpp = src->BPP();
	const UINT8 dst_bpp = dst->BPP();
	if (src_bpp == dst_bpp)
	{
		SDL_Rect dstrect;
		dstrect.x = iDestX;
		dstrect.y = iDestY;
		SDL_BlitSurface(src->surface_, &src_rect, dst->surface_, &dstrect);
#if defined __GNUC__ && defined i386
		__asm__ __volatile__("cld"); // XXX HACK000D
#endif
		return TRUE;
	}
	else if (src_bpp < dst_bpp)
	{
		SGPRect r;
		r.iLeft   = src_rect.x;
		r.iTop    = src_rect.y;
		r.iRight  = src_rect.x + src_rect.w;
		r.iBottom = src_rect.y + src_rect.h;
		SGPVSurface::Lock lsrc(src);
		SGPVSurface::Lock ldst(dst);
		UINT8*  const s_buf  = lsrc.Buffer<UINT8>();
		UINT32  const spitch = lsrc.Pitch();
		UINT16* const d_buf  = ldst.Buffer<UINT16>();
		UINT32  const dpitch = ldst.Pitch();
		Blt8BPPDataSubTo16BPPBuffer(d_buf, dpitch, src, s_buf, spitch, iDestX, iDestY, &r);
		return TRUE;
	}

	DebugMsg(TOPIC_VIDEOSURFACE, DBG_LEVEL_2, "Incompatible BPP values with src and dest Video Surfaces for blitting");
	return FALSE;
}


static BOOLEAN InternalShadowVideoSurfaceRect(SGPVSurface* const dst, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, const UINT16* const filter_table)
{
	if (X1 < 0) X1 = 0;
	if (X2 < 0) return FALSE;

	if (Y2 < 0) return FALSE;
	if (Y1 < 0) Y1 = 0;

	if (X2 >= dst->Width())  X2 = dst->Width() - 1;
	if (Y2 >= dst->Height()) Y2 = dst->Height() - 1;

	if (X1 >= dst->Width())  return FALSE;
	if (Y1 >= dst->Height()) return FALSE;

	if (X2 - X1 <= 0) return FALSE;
	if (Y2 - Y1 <= 0) return FALSE;

	SGPRect area;
	area.iTop    = Y1;
	area.iBottom = Y2;
	area.iLeft   = X1;
	area.iRight  = X2;

	SGPVSurface::Lock ldst(dst);
	return Blt16BPPBufferFilterRect(ldst.Buffer<UINT16>(), ldst.Pitch(), filter_table, &area);
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
	if (dst->BPP() != 16 || src->BPP() != 16) return FALSE;

	SDL_Surface const* const ssurface = src->surface_;
	SDL_Surface*       const dsurface = dst->surface_;

	const UINT32  s_pitch = ssurface->pitch >> 1;
	const UINT32  d_pitch = dsurface->pitch >> 1;
	const UINT16* os      = (const UINT16*)ssurface->pixels + s_pitch * SrcRect->iTop  + SrcRect->iLeft;
	UINT16*       d       =       (UINT16*)dsurface->pixels + d_pitch * DestRect->iTop + DestRect->iLeft;

	const UINT width  = DestRect->iRight  - DestRect->iLeft;
	const UINT height = DestRect->iBottom - DestRect->iTop;
	const UINT dx = SrcRect->iRight  - SrcRect->iLeft;
	const UINT dy = SrcRect->iBottom - SrcRect->iTop;
	UINT py = 0;
	if (ssurface->flags & SDL_SRCCOLORKEY)
	{
		const UINT16 key = ssurface->format->colorkey;
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
	AutoSGPVSurface src(AddVideoSurfaceFromFile(filename));
	if (src == NO_VSURFACE) return FALSE;
	BltVideoSurface(dst, src, x, y, NULL);
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
	DUMPINFO* const Info = MALLOCNZ(DUMPINFO, guiVSurfaceSize);

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
	gpVSurfaceTail->pName = MALLOCN(char, strlen(Filename) + 1);
	strcpy(gpVSurfaceTail->pName, Filename);

	//record the code location of the calling creating function.
	char str[256];
	sprintf(str, "%s -- line(%d)", SourceFile, LineNum);
	gpVSurfaceTail->pCode = MALLOCN(char, strlen(str) + 1);
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
