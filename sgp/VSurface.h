#ifndef VSURFACE_H
#define VSURFACE_H

#include "Types.h"
#include <SDL.h>


#define NO_VSURFACE 0
#define BACKBUFFER   g_back_buffer
#define FRAME_BUFFER g_frame_buffer
#define MOUSE_BUFFER g_mouse_buffer


extern SGPVSurface* g_back_buffer;
extern SGPVSurface* g_frame_buffer;
extern SGPVSurface* g_mouse_buffer;


struct SGPVSurface
{
	SDL_Surface* surface;
	UINT16       usHeight;      // Height of Video Surface
	UINT16       usWidth;       // Width of Video Surface
	UINT8        ubBitDepth;    // BPP ALWAYS 16!
	SDL_Color*   pPalette;
	UINT16*      p16BPPPalette; // A 16BPP palette used for 8->16 blits
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

BYTE* LockVideoSurface(SGPVSurface*, UINT32* uiPitch);
void UnLockVideoSurface(SGPVSurface*);

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(SGPVSurface* dst, SGPVSurface* src, INT32 iDestX, INT32 iDestY, const SGPRect* SrcRect);

/* Blits a video surface in half size to another video surface.
 * If SrcRect is NULL the entire source surface is blitted.
 * Only blitting from 8bbp surfaces to 16bpp surfaces is supported. */
void BltVideoSurfaceHalf(SGPVSurface* dst, SGPVSurface* src, INT32 DestX, INT32 DestY, const SGPRect* SrcRect);

/* Fill an entire surface with a colour */
void FillSurface(SGPVSurface*, UINT16 Colour);

/* Fills an rectangular area with a specified color value. */
BOOLEAN ColorFillVideoSurfaceArea(SGPVSurface*, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, UINT16 Color16BPP);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(SGPVSurface*, COLORVAL TransColor);

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries(const SGPVSurface*, SGPPaletteEntry* pPalette);

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette(SGPVSurface* hVSurface, const SGPPaletteEntry* pSrcPalette);

// Deletes all data, including palettes
BOOLEAN DeleteVideoSurface(SGPVSurface*);


BOOLEAN ShadowVideoSurfaceRect(SGPVSurface*, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(SGPVSurface*, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

/* This function will stretch the source image to the size of the dest rect.
 * If the 2 images are not 16 Bpp, it returns false. */
BOOLEAN BltStretchVideoSurface(SGPVSurface* dst, const SGPVSurface* src, SGPRect* SrcRect, SGPRect* DestRect);

extern UINT32 guiVSurfaceSize;

#ifdef __cplusplus
}
#endif

#endif
