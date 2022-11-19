#ifndef VSURFACE_H
#define VSURFACE_H

#include "AutoObj.h"
#include "Buffer.h"
#include "Types.h"
#include <SDL.h>


#define BACKBUFFER   g_back_buffer
#define FRAME_BUFFER g_frame_buffer
#define MOUSE_BUFFER g_mouse_buffer

class SGPVSurface;
class SGPVSurfaceAuto;
class SGPVSurface;

extern SGPVSurface* g_back_buffer;
extern SGPVSurfaceAuto* g_frame_buffer;
extern SGPVSurfaceAuto* g_mouse_buffer;

/** Utility wrapper around SDL_Surface. */
class SGPVSurface
{
	public:
		SGPVSurface(SDL_Surface*);

	protected:
		SGPVSurface(UINT16 w, UINT16 h, UINT8 bpp);

	public:
		virtual ~SGPVSurface();

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

		/* Fills an rectangular area with a specified color value. */
		friend void ColorFillVideoSurfaceArea(SGPVSurface*, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP);

		// Blits a video Surface to another video Surface
		friend void BltVideoSurface(SGPVSurface* dst, SGPVSurface* src, INT32 iDestX, INT32 iDestY, SGPBox const* src_rect);

		/* This function will stretch the source image to the size of the dest rect.
		 * If the 2 images are not 16 Bpp, it returns false. */
		friend void BltStretchVideoSurface(SGPVSurface* dst, SGPVSurface const* src, SGPBox const* src_rect, SGPBox const* dst_rect);

		// needs read access to *surface_ to initalize z-buffer properly
		friend void MainGameScreenInit(void);

	protected:
		SDL_Surface*                               surface_;
		SGP::Buffer<SGPPaletteEntry>               palette_;
	public:
		UINT16*                                    p16BPPPalette; // A 16BPP palette used for 8->16 blits
#ifdef SGP_VIDEO_DEBUGGING
		ST::string                   name_;
		ST::string                   code_;
#endif
		SGPVSurface*                 next_;

	private:
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
					LockBase(vs->surface_)
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
					surface_ = vs->surface_;
					if (surface_) SDL_LockSurface(surface_);
				}
		};
};

/**
 * Utility wrapper around SDL_Surface which automatically
 * frees SDL_Surface when the object is destroyed. */
class SGPVSurfaceAuto : public SGPVSurface
{
	public:
		SGPVSurfaceAuto(UINT16 w, UINT16 h, UINT8 bpp);
		SGPVSurfaceAuto(SDL_Surface*);

		virtual ~SGPVSurfaceAuto();
};


SGPVSurfaceAuto* AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth);
SGPVSurfaceAuto* AddVideoSurfaceFromFile(const char* Filename);

// Creates and adds a video Surface to list
#ifdef SGP_VIDEO_DEBUGGING
	void DumpVSurfaceInfoIntoFile(const char* filename, BOOLEAN fAppend);
	extern SGPVSurface* AddAndRecordVSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth, UINT32 LineNum, const char* SourceFile);
	extern SGPVSurface* AddAndRecordVSurfaceFromFile(const char* Filename, UINT32 LineNum, const char* SourceFile);
	#define AddVideoSurface(a, b, c) AddAndRecordVSurface(a, b, c, __LINE__, __FILE__)
	#define AddVideoSurfaceFromFile(a) AddAndRecordVSurfaceFromFile(a, __LINE__, __FILE__)

extern UINT32 guiVSurfaceSize;
#endif

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
