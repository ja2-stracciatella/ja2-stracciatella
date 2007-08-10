#ifndef VSURFACE_H
#define VSURFACE_H

#include "HImage.h"
#include "Types.h"
#include <SDL.h>


#define NO_VSURFACE 0
#define BACKBUFFER   0xFFFFFFF1
#define FRAME_BUFFER 0xFFFFFFF2
#define MOUSE_BUFFER 0xFFFFFFF3


typedef struct SGPVSurface
{
	SDL_Surface* surface;
	UINT16       usHeight;      // Height of Video Surface
	UINT16       usWidth;       // Width of Video Surface
	UINT8        ubBitDepth;    // BPP ALWAYS 16!
	SDL_Color*   pPalette;
	UINT16*      p16BPPPalette; // A 16BPP palette used for 8->16 blits
} SGPVSurface;
typedef SGPVSurface* HVSURFACE;


#ifdef __cplusplus
extern "C" {
#endif

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager(void);

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager(void);

UINT32 AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth);
UINT32 AddVideoSurfaceFromFile(const char* Filename);

// Creates and adds a video Surface to list
#ifdef SGP_VIDEO_DEBUGGING
	void DumpVSurfaceInfoIntoFile(const char* filename, BOOLEAN fAppend);
	extern UINT32 AddAndRecordVSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth, UINT32 LineNum, const char* SourceFile);
	extern UINT32 AddAndRecordVSurfaceFromFile(const char* Filename, UINT32 LineNum, const char* SourceFile);
	#define AddVideoSurface(a, b, c) AddAndRecordVSurface(a, b, c, __LINE__, __FILE__)
	#define AddVideoSurfaceFromFile(a) AddAndRecordVSurfaceFromFile(a, __LINE__, __FILE__)
#endif

// Returns a HVSurface for the specified index
HVSURFACE GetVideoSurface(UINT32 uiIndex);

BYTE* LockVideoSurface(UINT32 uiVSurface, UINT32* uiPitch);
void UnLockVideoSurface(UINT32 uiVSurface);

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, INT32 iDestX, INT32 iDestY, const SGPRect* SrcRect);

/* Fill an entire surface with a colour */
void FillSurface(UINT32 uiDestVSurface, UINT16 Colour);

/* Fills an rectangular area with a specified color value. */
BOOLEAN ColorFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2,	INT32 iDestY2, UINT16 Color16BPP);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency(UINT32 uiIndex, COLORVAL TransColor);

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries(HVSURFACE hVSurface, SGPPaletteEntry* pPalette);

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette(HVSURFACE hVSurface, const SGPPaletteEntry* pSrcPalette);

// Deletes all data, including palettes
BOOLEAN DeleteVideoSurfaceFromIndex(UINT32 uiIndex);


BOOLEAN ShadowVideoSurfaceRect(UINT32 uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

/* This function will stretch the source image to the size of the dest rect.
 * If the 2 images are not 16 Bpp, it returns false. */
BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, SGPRect* SrcRect, SGPRect* DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(UINT32 uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

#ifdef __cplusplus
}
#endif

#endif
