#include "Debug.h"
#include "HImage.h"
#include "Shading.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "Logger.h"

#include <string_theory/format>
#include <string_theory/string>

#include <stdexcept>

extern SGPVSurface* gpVSurfaceHead;


SGPVSurface::SGPVSurface(UINT16 w, UINT16 h, UINT8 bpp) :
	p16BPPPalette(),
	next_(gpVSurfaceHead)
{
	Assert(w > 0 && h > 0);

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

	if (p16BPPPalette) delete[] p16BPPPalette;
}


void SGPVSurface::SetPalette(const SGPPaletteEntry* const src_pal)
{
	// Create palette object if not already done so
	if (!palette_) palette_.Allocate(256);
	SGPPaletteEntry* const p = palette_;
	for (UINT32 i = 0; i < 256; i++)
	{
		p[i] = src_pal[i];
	}

	if (p16BPPPalette != NULL) delete[] p16BPPPalette;
	p16BPPPalette = Create16BPPPalette(src_pal);
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

static void InternalShadowVideoSurfaceRect(SGPVSurface* const dst, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2, const UINT16* const filter_table)
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
	Blt16BPPBufferFilterRect(ldst.Buffer<UINT16>(), ldst.Pitch(), filter_table, &area);
}


void SGPVSurface::ShadowRect(INT32 const x1, INT32 const y1, INT32 const x2, INT32 const y2)
{
	InternalShadowVideoSurfaceRect(this, x1, y1, x2, y2, (UINT16*)ShadeTable);
}


void SGPVSurface::ShadowRectUsingLowPercentTable(INT32 const x1, INT32 const y1, INT32 const x2, INT32 const y2)
{
	InternalShadowVideoSurfaceRect(this, x1, y1, x2, y2, (UINT16*)IntensityTable);
}


SGPVSurface* AddVideoSurfaceFromFile(const char* const Filename)
{
	AutoSGPImage img(CreateImage(Filename, IMAGE_ALLIMAGEDATA));

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

	{ SGPVSurface::Lock l(vs.get());
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
	SGPVSurface::Lock lsrc(src);
	SGPVSurface::Lock ldst(dst);
	UINT8*  const SrcBuf         = lsrc.Buffer<UINT8>();
	UINT32  const SrcPitchBYTES  = lsrc.Pitch();
	UINT16* const DestBuf        = ldst.Buffer<UINT16>();
	UINT32  const DestPitchBYTES = ldst.Pitch();
	Blt8BPPDataTo16BPPBufferHalf(DestBuf, DestPitchBYTES, src, SrcBuf, SrcPitchBYTES, DestX, DestY, src_rect);
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
	Assert(dst);
	Assert(src);

	const UINT8 src_bpp = src->BPP();
	const UINT8 dst_bpp = dst->BPP();
	if (src_bpp == dst_bpp)
	{
		SDL_Rect* src_rect = 0;
		SDL_Rect  r;
		if (src_box)
		{
			r.x = src_box->x;
			r.y = src_box->y;
			r.w = src_box->w;
			r.h = src_box->h;
			src_rect = &r;
		}

		SDL_Rect dstrect;
		dstrect.x = iDestX;
		dstrect.y = iDestY;
		SDL_BlitSurface(src->surface_.get(), src_rect, dst->surface_.get(), &dstrect);
	}
	else if (src_bpp < dst_bpp)
	{
		SGPBox const* src_rect = src_box;
		SGPBox        r;
		if (!src_rect)
		{
			// Check Sizes, SRC size MUST be <= DEST size
			if (dst->Height() < src->Height())
			{
				SLOGD("Incompatible height size given in Video Surface blit");
				return;
			}
			if (dst->Width() < src->Width())
			{
				SLOGD("Incompatible height size given in Video Surface blit");
				return;
			}

			r.x = 0;
			r.y = 0;
			r.w = src->Width();
			r.h = src->Height();
			src_rect = &r;
		}

		SGPVSurface::Lock lsrc(src);
		SGPVSurface::Lock ldst(dst);
		UINT8*  const s_buf  = lsrc.Buffer<UINT8>();
		UINT32  const spitch = lsrc.Pitch();
		UINT16* const d_buf  = ldst.Buffer<UINT16>();
		UINT32  const dpitch = ldst.Pitch();
		Blt8BPPDataSubTo16BPPBuffer(d_buf, dpitch, src, s_buf, spitch, iDestX, iDestY, src_rect);
	}
	else
	{
		SLOGD("Incompatible BPP values with src and dest Video Surfaces for blitting");
	}
}


void BltStretchVideoSurface(SGPVSurface* const dst, SGPVSurface const* const src, SGPBox const* const src_rect, SGPBox const* const dst_rect)
{
	if (dst->BPP() != 16 || src->BPP() != 16) return;

	SDL_Surface const* const ssurface = src->surface_.get();
	SDL_Surface*       const dsurface = dst->surface_.get();

	const UINT32  s_pitch = ssurface->pitch >> 1;
	const UINT32  d_pitch = dsurface->pitch >> 1;
	UINT16 const* os      = (const UINT16*)ssurface->pixels + s_pitch * src_rect->y + src_rect->x;
	UINT16*       d       =       (UINT16*)dsurface->pixels + d_pitch * dst_rect->y + dst_rect->x;

	UINT const width  = dst_rect->w;
	UINT const height = dst_rect->h;
	UINT const dx     = src_rect->w;
	UINT const dy     = src_rect->h;
	UINT py = 0;
	if (ssurface->flags & SDL_TRUE)
	{
//		const UINT16 key = ssurface->format->colorkey;
		const UINT16 key = 0;
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
