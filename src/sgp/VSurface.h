#ifndef VSURFACE_H
#define VSURFACE_H

#include "Buffer.h"
#include "Types.h"
#include <memory>
#include <SDL.h>


#define BACKBUFFER   g_back_buffer
#define FRAME_BUFFER g_frame_buffer
#define MOUSE_BUFFER g_mouse_buffer

class SGPVSurface;

inline SGPVSurface* g_back_buffer;
inline SGPVSurface* g_frame_buffer;
inline SGPVSurface* g_mouse_buffer;

struct SDLDeleter
{
	void operator()(SDL_Surface *p) noexcept { SDL_FreeSurface(p); }
	void operator()(SDL_Renderer *p) noexcept { SDL_DestroyRenderer(p); }
};
using SurfaceUniquePtr = std::unique_ptr<SDL_Surface, SDLDeleter>;
using RendererUniquePtr = std::unique_ptr<SDL_Renderer, SDLDeleter>;


/** Utility wrapper around SDL_Surface. */
class SGPVSurface
{
	public:
		SGPVSurface(SDL_Surface*);
		SGPVSurface(UINT16 w, UINT16 h, UINT8 bpp);
		~SGPVSurface();

		UINT16 Width()  const { return surface_->w; }
		UINT16 Height() const { return surface_->h; }
		UINT8  BPP()    const { return surface_->format->BitsPerPixel; }

		// Set palette, also sets 16BPP palette
		void SetPalette(const SGPPaletteEntry* src_pal);

		// Get the RGB palette entry values
		SGPPaletteEntry const* GetPalette() const { return palette_; }

		void SetTransparency(COLORVAL);

		/* Fill an entire surface with a colour */
		void Fill(UINT16 colour);

		void ShadowRect(INT32 x1, INT32 y1, INT32 x2, INT32 y2);
		void ShadowRectUsingLowPercentTable(INT32 x1, INT32 y1, INT32 x2, INT32 y2);

		/* Allow access to the underlying SDL_Surface */
		SDL_Surface & GetSDLSurface() const noexcept { return *surface_; }

		/* Return a software renderer for this surface. The renderer is created
		 * on demand the first time this method gets called and will be
		 * destroyed together with the surface. */
		SDL_Renderer * GetRenderer();

		/* Set the draw color for this surface's renderer. */
		SGPVSurface & SetDrawColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
		{
			SDL_SetRenderDrawColor(GetRenderer(), r, g, b, a);
			return *this;
		}

		/* Draw a line from (x1, y1) to (x2, y2) using the surface's renderer. */
		SGPVSurface & LineDraw(int x1, int y1, int x2, int y2)
		{
			SDL_RenderDrawLine(GetRenderer(), x1, y1, x2, y2);
			return *this;
		}

		/* Draw a rectangle (not filled!) to the surface. The parameters may
		 * look a bit weird; that's because this method is meant as a drop-in
		 * replacement for the old RectangleDraw API, which specified the top
		 * left and bottom right corners of the rectangle. */
		SGPVSurface & RectangleDraw(int tl_x, int tl_y, int br_x, int br_y)
		{
			SDL_Rect r{ tl_x, tl_y, br_x - tl_x + 1, br_y - tl_y + 1 };
			SDL_RenderDrawRect(GetRenderer(), &r);
			return *this;
		}

		/* Fill a rectangle with the currently set draw color. */
		SGPVSurface & FillRect(SDL_Rect const& rect)
		{
			SDL_RenderFillRect(GetRenderer(), &rect);
			return *this;
		}

		/* Fills an rectangular area with a specified color value. */
		friend void ColorFillVideoSurfaceArea(SGPVSurface*, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP);

		// Blits a video Surface to another video Surface
		friend void BltVideoSurface(SGPVSurface* dst, SGPVSurface* src, INT32 iDestX, INT32 iDestY, SGPBox const* src_rect);

		/* This function will stretch the source image to the size of the dest rect.
		 * If the 2 images are not 16 Bpp, it returns false. */
		friend void BltStretchVideoSurface(SGPVSurface* dst, SGPVSurface const* src, SGPBox const* src_rect, SGPBox const* dst_rect);

	private:
		SurfaceUniquePtr                           surface_;
		RendererUniquePtr                          renderer_;
		SGP::Buffer<SGPPaletteEntry>               palette_;
	public:
		UINT16*                                    p16BPPPalette; // A 16BPP palette used for 8->16 blits
	private:
		SGPVSurface*                 next_;

		class LockBase
		{
			public:
				explicit LockBase(SDL_Surface* const s) : surface_(s) {}

				template<typename T> T* Buffer()
				{
					return static_cast<T*>(surface_->pixels);
				}

				UINT32 Pitch()
				{
					return surface_->pitch;
				}

			protected:
				SDL_Surface* surface_;
		};

	public:
		class Lock : public LockBase
		{
			public:
				explicit Lock(SGPVSurface* const vs) :
					LockBase(vs->surface_.get())
				{
					SDL_LockSurface(surface_);
				}

				~Lock()
				{
					SDL_UnlockSurface(surface_);
				}
		};

		class Lockable : public LockBase
		{
			public:
				explicit Lockable() : LockBase(0) {}

				~Lockable()
				{
					if (surface_) SDL_UnlockSurface(surface_);
				}

				void Lock(SGPVSurface* const vs)
				{
					if (surface_) SDL_UnlockSurface(surface_);
					surface_ = vs->surface_.get();
					if (surface_) SDL_LockSurface(surface_);
				}
		};
};

inline SGPVSurface* AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth)
{
	return new SGPVSurface(Width, Height, BitDepth);
}

SGPVSurface* AddVideoSurfaceFromFile(const char* Filename);

/* Blits a video surface in half size to another video surface.
 * If SrcRect is NULL the entire source surface is blitted.
 * Only blitting from 8bbp surfaces to 16bpp surfaces is supported. */
void BltVideoSurfaceHalf(SGPVSurface* dst, SGPVSurface* src, INT32 DestX, INT32 DestY, SGPBox const* src_rect);

// Deletes all data, including palettes
static inline void DeleteVideoSurface(SGPVSurface* const vs)
{
	delete vs;
}

void BltVideoSurfaceOnce(SGPVSurface* dst, const char* filename, INT32 x, INT32 y);

/** Draw image on the video surface stretching the image if necessary. */
void BltVideoSurfaceOnceWithStretch(SGPVSurface* const dst, const char* const filename);

/** Fill video surface with another one with stretch. */
void FillVideoSurfaceWithStretch(SGPVSurface* const dst, SGPVSurface* const src);

#endif
