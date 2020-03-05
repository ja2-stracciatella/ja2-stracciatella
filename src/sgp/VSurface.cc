#include <stdexcept>

#include "Debug.h"
#include "HImage.h"
#include "MemMan.h"
#include "Shading.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Video.h"
#include "SGP.h"
#include "Logger.h"
#include "Local.h"
#include "UILayout.h"
#include "VObject.h"

extern SGPVSurface* gpVSurfaceHead;


SGPVSurface::SGPVSurface(UINT16 const w, UINT16 const h, UINT8 const bpp) :
	p16BPPPalette(),
#ifdef SGP_VIDEO_DEBUGGING
	name_(),
	code_(),
#endif
	next_(gpVSurfaceHead)
{
	Assert(w > 0);
	Assert(h > 0);
	Assert(bpp == 32);

	SDL_Surface* s;
	switch (bpp)
	{
		case 8:
			s = SDL_CreateRGBSurface(0, w, h, bpp, 0, 0, 0, 0);
			break;

		case 16:
		{
			SDL_PixelFormat const* f = SDL_AllocFormat(SDL_PIXELFORMAT_RGB565);
			s = SDL_CreateRGBSurface(0, w, h, bpp, f->Rmask, f->Gmask, f->Bmask, f->Amask);
			break;
		}

		case 32:
		{
			SDL_PixelFormat const* f = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
			s = SDL_CreateRGBSurface(0, w, h, bpp, f->Rmask, f->Gmask, f->Bmask, f->Amask);
			break;
		}

		default:
			throw std::logic_error("Tried to create video surface with invalid bpp, must be 8 or 16.");
	}
	if (!s) throw std::runtime_error("Failed to create SDL surface");
	surface_ = s;
	gpVSurfaceHead = this;
#ifdef SGP_VIDEO_DEBUGGING
	++guiVSurfaceSize;
#endif
}


SGPVSurface::SGPVSurface(SDL_Surface* const s) :
	surface_(s),
	p16BPPPalette(),
#ifdef SGP_VIDEO_DEBUGGING
	name_(),
	code_(),
#endif
	next_(gpVSurfaceHead)
{
	gpVSurfaceHead = this;
#ifdef SGP_VIDEO_DEBUGGING
	++guiVSurfaceSize;
#endif
}


SGPVSurface::~SGPVSurface()
{
	for (SGPVSurface** anchor = &gpVSurfaceHead;; anchor = &(*anchor)->next_)
	{
		if (*anchor != this) continue;
		*anchor = next_;
#ifdef SGP_VIDEO_DEBUGGING
		--guiVSurfaceSize;
#endif
		break;
	}

#ifdef SGP_VIDEO_DEBUGGING
	if (name_) MemFree(name_);
	if (code_) MemFree(code_);
#endif
}


void SGPVSurface::SetPalette(const SGPPaletteEntry* const src_pal)
{
	// Create palette object if not already done so
	// TODO: FIXME: maxrd2 there is no palette
	if (!palette_) palette_.Allocate(256);
	SGPPaletteEntry* const p = palette_;
	for (UINT32 i = 0; i < 256; i++)
	{
		p[i] = src_pal[i];
	}

	p16BPPPalette = SHADE_NONE;
}


void SGPVSurface::SetTransparency(const COLORVAL color)
{
	return; // maxrd2 - drop this we're handling transparency with alpha
	Uint32 color_key;
	switch (BPP())
	{
		case  8: color_key = color; break; // FIXME: maxrd2, should we drop this altogether?
		case 16: color_key = color; break;
		case 32: color_key = color; break;

		default: abort(); // HACK000E
	}
	SDL_SetColorKey(surface_, SDL_TRUE, color_key);
}


void SGPVSurface::Fill(const UINT32 color)
{
	SDL_FillRect(surface_, NULL, color);
}

SGPVSurfaceAuto::SGPVSurfaceAuto(UINT16 w, UINT16 h, UINT8 bpp)
	: SGPVSurface(w, h, bpp)
{
}

SGPVSurfaceAuto::SGPVSurfaceAuto(SDL_Surface* surface)
	: SGPVSurface(surface)
{
}

SGPVSurfaceAuto::~SGPVSurfaceAuto()
{
	if(surface_)
	{
		SDL_FreeSurface(surface_);
	}
}


static void InternalShadowVideoSurfaceRect(SGPVSurface* const dst, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, const FLOAT filterPercent)
{
	if (X1 < 0) X1 = 0;
	if (X2 < 0) return;

	if (Y2 < 0) return;
	if (Y1 < 0) Y1 = 0;

	if (X2 >= dst->Width())  X2 = dst->Width() - 1;
	if (Y2 >= dst->Height()) Y2 = dst->Height() - 1;

	if (X1 >= dst->Width())  return;
	if (Y1 >= dst->Height()) return;

	if (X2 - X1 <= 0) return;
	if (Y2 - Y1 <= 0) return;

	SGPRect area;
	area.iTop    = Y1;
	area.iBottom = Y2;
	area.iLeft   = X1;
	area.iRight  = X2;

	SGPVSurface::Lock ldst(dst);
	Blt32BPPBufferFilterRect(ldst.Buffer<UINT32>(), ldst.Pitch(), filterPercent, &area);
}


void SGPVSurface::ShadowRect(INT32 const x1, INT32 const y1, INT32 const x2, INT32 const y2)
{
	InternalShadowVideoSurfaceRect(this, x1, y1, x2, y2, gShadowShadePercent);
}


void SGPVSurface::ShadowRectUsingLowPercentTable(INT32 const x1, INT32 const y1, INT32 const x2, INT32 const y2)
{
	InternalShadowVideoSurfaceRect(this, x1, y1, x2, y2, gIntensityShadePercent);
}

SGPVSurface* g_back_buffer;
SGPVSurfaceAuto* g_frame_buffer;
SGPVSurfaceAuto* g_mouse_buffer;


#undef AddVideoSurface
#undef AddVideoSurfaceFromFile


SGPVSurfaceAuto* AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth)
{
	SGPVSurfaceAuto* const vs = new SGPVSurfaceAuto(Width, Height, BitDepth);
	return vs;
}


SGPVSurfaceAuto* AddVideoSurfaceFromFile(const char *Filename, ScaleCallback *callback)
{
	AutoSGPImage imgOrig(CreateImage(Filename, IMAGE_ALLIMAGEDATA));
	AutoSGPImage img(ScaleImage(imgOrig, g_ui.m_stdScreenScale, true, callback));

	SGPVSurfaceAuto* const vs = new SGPVSurfaceAuto(img->usWidth, img->usHeight, img->ubBitDepth);

	UINT8 const dst_bpp = vs->BPP();
	UINT32      buffer_bpp;
	switch (dst_bpp)
	{
		case  8: buffer_bpp = BUFFER_8BPP;  break;
		case 16: buffer_bpp = BUFFER_16BPP; break;
		case 32: buffer_bpp = BUFFER_32BPP; break;
		default: throw std::logic_error("Invalid bpp");
	}

	{
		SGPVSurface::Lock l(vs);
		UINT8*  const dst   = l.Buffer<UINT8>();
		UINT16  const pitch = l.Pitch() / (dst_bpp / 8); // pitch in pixels
		SGPBox  const box   = { 0, 0, img->usWidth, img->usHeight };
		BOOLEAN const Ret   = CopyImageToBuffer(img, buffer_bpp, dst, pitch, vs->Height(), 0, 0, &box);
		if (!Ret)
		{
			SLOGE("Error Occured Copying SGPImage to video surface");
		}
	}

	if (img->pPalette/*img->ubBitDepth == 8*/) vs->SetPalette(img->pPalette); // TODO: FIXME: @maxrd2 we don't really need palette

	return vs;
}


#ifdef SGP_VIDEO_DEBUGGING

static void RecordVSurface(SGPVSurface* const vs, char const* const Filename, UINT32 const LineNum, char const* const SourceFile)
{
	//record the filename of the vsurface (some are created via memory though)
	vs->name_ = MALLOCN(char, strlen(Filename) + 1);
	strcpy(vs->name_, Filename);

	//record the code location of the calling creating function.
	char str[256];
	sprintf(str, "%s -- line(%d)", SourceFile, LineNum);
	vs->code_ = MALLOCN(char, strlen(str) + 1);
	strcpy(vs->code_, str);
}

#	define RECORD(vs, name) RecordVSurface((vs), (name), __LINE__, __FILE__)
#else
#	define RECORD(cs, name) ((void)0)
#endif

void BltVideoSurfaceHalf(SGPVSurface* const dst, SGPVSurface* const src, INT32 const DestX, INT32 const DestY, SGPBox const* const src_rect)
{
	// maxrd2 - OK - used on strategic screen to display sector map
	SDL_Rect srcRect;
	if(src_rect) {
		srcRect.x = src_rect->x;
		srcRect.y = src_rect->y;
		srcRect.w = src_rect->w;
		srcRect.h = src_rect->h;
	} else {
		srcRect.x = srcRect.y = 0;
		srcRect.w = src->Width();
		srcRect.h = src->Height();
	}
	SDL_Rect dstRect = { DestX, DestY, srcRect.w / 2, srcRect.h / 2 };
	if(src->p16BPPPalette & 0xFF)
		SDL_SetSurfaceColorMod(src->surface_, src->p16BPPPalette >> 24 & 0xFF, src->p16BPPPalette >> 16 & 0xFF, src->p16BPPPalette >> 8 & 0xFF);
	SDL_BlitScaled(src->surface_, &srcRect, dst->surface_, &dstRect);
	if(src->p16BPPPalette & 0xFF)
		SDL_SetSurfaceColorMod(src->surface_, 255, 255, 255);
}


void ColorFillVideoSurfaceArea(SGPVSurface* const dst, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, const UINT32 Color)
{
	SGPRect Clip;
	GetClippingRect(&Clip);

	if (iDestX1 < Clip.iLeft) iDestX1 = Clip.iLeft;
	if (iDestX1 > Clip.iRight) return;

	if (iDestX2 > Clip.iRight) iDestX2 = Clip.iRight;
	if (iDestX2 < Clip.iLeft) return;

	if (iDestY1 < Clip.iTop) iDestY1 = Clip.iTop;
	if (iDestY1 > Clip.iBottom) return;

	if (iDestY2 > Clip.iBottom) iDestY2 = Clip.iBottom;
	if (iDestY2 < Clip.iTop) return;

	if (iDestX2 <= iDestX1 || iDestY2 <= iDestY1) return;

	SDL_Rect Rect;
	Rect.x = iDestX1;
	Rect.y = iDestY1;
	Rect.w = iDestX2 - iDestX1;
	Rect.h = iDestY2 - iDestY1;
	SDL_FillRect(dst->surface_, &Rect, Color);
}


// Will drop down into user-defined blitter if 8->16 BPP blitting is being done
void BltVideoSurface(SGPVSurface* const dst, SGPVSurface* const src, INT32 const iDestX, INT32 const iDestY, SGPBox const* const src_box)
{
	// maxrd2 - OK - used on intro screen, main menu, after load game screen
	Assert(dst);
	Assert(dst->BPP() == 32);
	Assert(src);
	Assert(src->BPP() == 32);

	SDL_Rect src_rect;
	if(src_box)
		src_rect = { src_box->x, src_box->y, src_box->w, src_box->h };

	SDL_Rect dst_rect = { iDestX, iDestY, 0, 0 };
	if(src->p16BPPPalette & 0xFF)
		SDL_SetSurfaceColorMod(src->surface_, src->p16BPPPalette >> 24 & 0xFF, src->p16BPPPalette >> 16 & 0xFF, src->p16BPPPalette >> 8 & 0xFF);
	SDL_BlitSurface(src->surface_, src_box ? &src_rect : nullptr, dst->surface_, &dst_rect);
	if(src->p16BPPPalette & 0xFF)
		SDL_SetSurfaceColorMod(src->surface_, 255, 255, 255);
}


void BltStretchVideoSurface(SGPVSurface* const dst, SGPVSurface const* const src, SGPBox const* const src_rect, SGPBox const* const dst_rect)
{
	// maxrd2 - OK - used when switching from strategic to tactical screen
	SDL_Rect srcRect = { src_rect->x, src_rect->y, src_rect->w, src_rect->h };
	SDL_Rect dstRect = { dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h };

	SDL_BlitScaled(src->surface_, &srcRect, dst->surface_, &dstRect);
}


void BltVideoSurfaceOnce(SGPVSurface* const dst, const char* const filename, INT32 const x, INT32 const y)
{
	SGP::AutoPtr<SGPVSurfaceAuto> src(AddVideoSurfaceFromFile(filename));
	BltVideoSurface(dst, src, x, y, NULL);
}

/** Draw image on the video surface stretching the image if necessary. */
void BltVideoSurfaceOnceWithStretch(SGPVSurface* const dst, const char* const filename)
{
	SGP::AutoPtr<SGPVSurfaceAuto> src(AddVideoSurfaceFromFile(filename));
	FillVideoSurfaceWithStretch(dst, src);
}

/** Fill video surface with another one with stretch. */
void FillVideoSurfaceWithStretch(SGPVSurface* const dst, SGPVSurface* const src)
{
	SGPBox srcRec;
	SGPBox dstRec;
	srcRec.set(0, 0, src->Width(), src->Height());
	dstRec.set(0, 0, dst->Width(), dst->Height());
	BltStretchVideoSurface(dst, src, &srcRec, &dstRec);
}

#ifdef SGP_VIDEO_DEBUGGING

UINT32 guiVSurfaceSize = 0;


struct DUMPINFO
{
	UINT32 Counter;
	char Name[256];
	char Code[256];
};


void DumpVSurfaceInfoIntoFile(const char* filename, BOOLEAN fAppend)
{
	if (!guiVSurfaceSize) return;

	FILE* fp = fopen(filename, fAppend ? "a" : "w");
	Assert(fp != NULL);

	//Allocate enough strings and counters for each node.
	DUMPINFO* const Info = MALLOCNZ(DUMPINFO, guiVSurfaceSize);

	//Loop through the list and record every unique filename and count them
	UINT32 uiUniqueID = 0;
	for (SGPVSurface const* i = gpVSurfaceHead; i; i = i->next_)
	{
		char const* const Name = i->name_;
		char const* const Code = i->code_;
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


SGPVSurface* AddAndRecordVSurface(const UINT16 Width, const UINT16 Height, const UINT8 BitDepth, const UINT32 LineNum, const char* const SourceFile)
{
	SGPVSurface* const vs = AddVideoSurface(Width, Height, BitDepth);
	RecordVSurface(vs, "<EMPTY>", LineNum, SourceFile);
	return vs;
}


SGPVSurface* AddAndRecordVSurfaceFromFile(const char* const Filename, const UINT32 LineNum, const char* const SourceFile)
{
	SGPVSurface* const vs = AddVideoSurfaceFromFile(Filename);
	RecordVSurface(vs, Filename, LineNum, SourceFile);
	return vs;
}

#endif
