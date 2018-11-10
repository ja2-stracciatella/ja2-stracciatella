#include "Debug.h"
#include "HImage.h"
#include "Shading.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Logger.h"
#include "VideoScale.h"
#include "UILayout.h"
#include "VObject.h"

#include <string_theory/format>
#include <string_theory/string>

#include <stdexcept>

extern SGPVSurface* gpVSurfaceHead;


SGPVSurface::SGPVSurface(UINT16 w, UINT16 h, UINT8 bpp) :
	p16BPPPalette(),
	next_(gpVSurfaceHead)
{
	Assert(w > 0 && h > 0);
	Assert(bpp == 32);

	Uint32 pixelFormat;
	switch (bpp)
	{
		case 8:
			pixelFormat = SDL_PIXELFORMAT_INDEX8;
			break;

		case 16:
		{
			pixelFormat = SDL_PIXELFORMAT_RGB565;
			break;
		}

		case 32:
		{
			pixelFormat = SDL_PIXELFORMAT_RGBA8888;
			break;
		}

		default:
			throw std::logic_error("Tried to create video surface with invalid bpp, must be 8 or 16.");
	}
	SDL_Surface * const s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 0, pixelFormat);
	if(!s)
		throw std::runtime_error("Failed to create SDL surface");
	surface_.reset(s);
	gpVSurfaceHead = this;
}


SGPVSurface::SGPVSurface(SDL_Surface* s) :
	surface_(s),
	p16BPPPalette(),
	next_(gpVSurfaceHead)
{
	gpVSurfaceHead = this;
}


SGPVSurface::~SGPVSurface()
{
	for (SGPVSurface** anchor = &gpVSurfaceHead;; anchor = &(*anchor)->next_)
	{
		if (*anchor != this) continue;
		*anchor = next_;
		break;
	}
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
	SDL_SetColorKey(surface_.get(), SDL_TRUE, color_key);
}


void SGPVSurface::Fill(const UINT32 color)
{
	SDL_FillRect(surface_.get(), NULL, color);
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


SGPVSurface* AddVideoSurfaceFromFile(const char* const Filename)
{
	AutoSGPImage imgOrig(CreateImage(Filename, IMAGE_ALLIMAGEDATA));
	AutoSGPImage img(ScaleImage(imgOrig.get(), g_ui.m_stdScreenScale));

	auto vs = std::make_unique<SGPVSurface>(img->usWidth, img->usHeight, img->ubBitDepth);

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
		SGPVSurface::Lock l(vs.get());
		UINT8*  const dst   = l.Buffer<UINT8>();
		UINT16  const pitch = l.Pitch() / (dst_bpp / 8); // pitch in pixels
		SGPBox  const box   = { 0, 0, img->usWidth, img->usHeight };
		BOOLEAN const Ret   = CopyImageToBuffer(img.get(), buffer_bpp, dst, pitch, vs->Height(), 0, 0, &box);
		if (!Ret)
		{
			SLOGE("Error Occured Copying SGPImage to video surface");
		}
	}

	if (img->pPalette/*img->ubBitDepth == 8*/) vs->SetPalette(img->pPalette); // TODO: FIXME: @maxrd2 we don't really need palette

	return vs.release();
}

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
		SDL_SetSurfaceColorMod(src->surface_.get(), src->p16BPPPalette >> 24 & 0xFF, src->p16BPPPalette >> 16 & 0xFF, src->p16BPPPalette >> 8 & 0xFF);
	SDL_BlitScaled(src->surface_.get(), &srcRect, dst->surface_.get(), &dstRect);
	if(src->p16BPPPalette & 0xFF)
		SDL_SetSurfaceColorMod(src->surface_.get(), 255, 255, 255);
}


void ColorFillVideoSurfaceArea(SGPVSurface* const dst, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, const UINT32 Color)
{
	SGPRect const Clip = GetClippingRect();

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
	SDL_FillRect(dst->surface_.get(), &Rect, Color);
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
		SDL_SetSurfaceColorMod(src->surface_.get(), src->p16BPPPalette >> 24 & 0xFF, src->p16BPPPalette >> 16 & 0xFF, src->p16BPPPalette >> 8 & 0xFF);
	SDL_BlitSurface(src->surface_.get(), src_box ? &src_rect : nullptr, dst->surface_.get(), &dst_rect);
	if(src->p16BPPPalette & 0xFF)
		SDL_SetSurfaceColorMod(src->surface_.get(), 255, 255, 255);
}


void BltStretchVideoSurface(SGPVSurface* const dst, SGPVSurface const* const src, SGPBox const* const src_rect, SGPBox const* const dst_rect)
{
	// maxrd2 - OK - used when switching from strategic to tactical screen
	SDL_Rect srcRect = { src_rect->x, src_rect->y, src_rect->w, src_rect->h };
	SDL_Rect dstRect = { dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h };

	SDL_BlitScaled(src->surface_.get(), &srcRect, dst->surface_.get(), &dstRect);
}


void BltVideoSurfaceOnce(SGPVSurface* const dst, const char* const filename, INT32 const x, INT32 const y)
{
	std::unique_ptr<SGPVSurface> src(AddVideoSurfaceFromFile(filename));
	BltVideoSurface(dst, src.get(), x, y, NULL);
}

/** Draw image on the video surface stretching the image if necessary. */
void BltVideoSurfaceOnceWithStretch(SGPVSurface* const dst, const char* const filename)
{
	std::unique_ptr<SGPVSurface> src(AddVideoSurfaceFromFile(filename));
	FillVideoSurfaceWithStretch(dst, src.get());
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
