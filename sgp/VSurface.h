#ifndef VSURFACE_H
#define VSURFACE_H

#include "AutoObj.h"
#include "Types.h"
#include <SDL.h>


#define NO_VSURFACE 0
#define BACKBUFFER   g_back_buffer
#define FRAME_BUFFER g_frame_buffer
#define MOUSE_BUFFER g_mouse_buffer


extern SGPVSurface* g_back_buffer;
extern SGPVSurface* g_frame_buffer;
extern SGPVSurface* g_mouse_buffer;


class SGPVSurface
{
	public:
		SGPVSurface(SDL_Surface* s) :
			surface_(s),
			palette_(0),
			p16BPPPalette(0)
		{}

		~SGPVSurface();

		UINT16 Width()  const { return surface_->w; }
		UINT16 Height() const { return surface_->h; }
		UINT8  BPP()    const { return surface_->format->BitsPerPixel; }

		// Set palette, also sets 16BPP palette
		void SetPalette(const SGPPaletteEntry* src_pal);

		// Get the RGB palette entry values
		void GetPalette(SGPPaletteEntry* dst_pal) const;

		void SetTransparency(COLORVAL);

		/* Fill an entire surface with a colour */
		void Fill(UINT16 colour);

		/* Fills an rectangular area with a specified color value. */
		friend BOOLEAN ColorFillVideoSurfaceArea(SGPVSurface*, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP);

		// Blits a video Surface to another video Surface
		friend BOOLEAN BltVideoSurface(SGPVSurface* dst, SGPVSurface* src, INT32 iDestX, INT32 iDestY, const SGPRect* SrcRect);

		/* This function will stretch the source image to the size of the dest rect.
		 * If the 2 images are not 16 Bpp, it returns false. */
		friend BOOLEAN BltStretchVideoSurface(SGPVSurface* dst, const SGPVSurface* src, SGPRect* SrcRect, SGPRect* DestRect);

	private:
		SGP::AutoObj<SDL_Surface, SDL_FreeSurface> surface_;
		SDL_Color*                                 palette_;
	public:
		UINT16*                                    p16BPPPalette; // A 16BPP palette used for 8->16 blits


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


#ifdef __cplusplus
extern "C" {
#endif

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager(void);

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager(void);

SGPVSurface* AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth);
SGPVSurface* AddVideoSurfaceFromFile(const char* Filename);

// Creates and adds a video Surface to list
#ifdef SGP_VIDEO_DEBUGGING
	void DumpVSurfaceInfoIntoFile(const char* filename, BOOLEAN fAppend);
	extern SGPVSurface* AddAndRecordVSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth, UINT32 LineNum, const char* SourceFile);
	extern SGPVSurface* AddAndRecordVSurfaceFromFile(const char* Filename, UINT32 LineNum, const char* SourceFile);
	#define AddVideoSurface(a, b, c) AddAndRecordVSurface(a, b, c, __LINE__, __FILE__)
	#define AddVideoSurfaceFromFile(a) AddAndRecordVSurfaceFromFile(a, __LINE__, __FILE__)
#endif

/* Blits a video surface in half size to another video surface.
 * If SrcRect is NULL the entire source surface is blitted.
 * Only blitting from 8bbp surfaces to 16bpp surfaces is supported. */
void BltVideoSurfaceHalf(SGPVSurface* dst, SGPVSurface* src, INT32 DestX, INT32 DestY, const SGPRect* SrcRect);

// Deletes all data, including palettes
void DeleteVideoSurface(SGPVSurface*);


BOOLEAN ShadowVideoSurfaceRect(SGPVSurface*, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(SGPVSurface*, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

BOOLEAN BltVideoSurfaceOnce(SGPVSurface* dst, const char* filename, INT32 x, INT32 y);

extern UINT32 guiVSurfaceSize;

typedef SGP::AutoObj<SGPVSurface, DeleteVideoSurface>::Type AutoSGPVSurface;

#ifdef __cplusplus
}
#endif

#endif
