#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "Types.h"
#include "VObject.h"
#include <SDL.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface SGP Module
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//
// Defines for special video object handles given to blit function
//

#define BACKBUFFER									            0xFFFFFFF1
#define FRAME_BUFFER                            0xFFFFFFF2
#define MOUSE_BUFFER                            0xFFFFFFF3

//
// Defines for blitting
//

#define VS_BLT_FAST															0x000000004


typedef struct
{
	SDL_Surface* surface;
	UINT16					usHeight;							// Height of Video Surface
	UINT16					usWidth;							// Width of Video Surface
	UINT8						ubBitDepth;						// BPP ALWAYS 16!
	SDL_Color* pPalette;
	UINT16					*p16BPPPalette;				// A 16BPP palette used for 8->16 blits
} SGPVSurface, *HVSURFACE;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface Manager Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

extern	INT32	giMemUsedInSurfaces;

// Creates a list to contain video Surfaces
BOOLEAN InitializeVideoSurfaceManager( );

// Deletes any video Surface placed into list
BOOLEAN ShutdownVideoSurfaceManager( );

BOOLEAN AddVideoSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth, UINT32* Index);
BOOLEAN AddVideoSurfaceFromFile(const char* Filename, UINT32* Index);

// Creates and adds a video Surface to list
#ifdef SGP_VIDEO_DEBUGGING
	void DumpVSurfaceInfoIntoFile(const char *filename, BOOLEAN fAppend);
	extern BOOLEAN AddAndRecordVSurface(UINT16 Width, UINT16 Height, UINT8 BitDepth, UINT32* Index, UINT32 LineNum, const char* SourceFile);
	extern BOOLEAN AddAndRecordVSurfaceFromFile(const char* Filename, UINT32* Index, UINT32 LineNum, const char* SourceFile);
	#define AddVideoSurface(a, b, c, d) AddAndRecordVSurface(a, b, c, d, __LINE__, __FILE__)
	#define AddVideoSurfaceFromFile(a, b) AddAndRecordVSurfaceFromFile(a, b, __LINE__, __FILE__)
#endif

// Returns a HVSurface for the specified index
HVSURFACE GetVideoSurface(UINT32 uiIndex);

BYTE *LockVideoSurface( UINT32 uiVSurface, UINT32 *uiPitch );
void UnLockVideoSurface( UINT32 uiVSurface );

// Blits a video Surface to another video Surface
BOOLEAN BltVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, const SGPRect* SrcRect);

BOOLEAN ColorFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2,	INT32 iDestY2, UINT16 Color16BPP);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency( UINT32 uiIndex, COLORVAL TransColor );

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

HVSURFACE CreateVideoSurfaceFromFile(const char* Filename);

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries( HVSURFACE hVSurface, SGPPaletteEntry *pPalette );

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette( HVSURFACE hVSurface, SGPPaletteEntry *pSrcPalette );

// Deletes all data, including palettes, regions, DD Surfaces
BOOLEAN DeleteVideoSurface( HVSURFACE hVSurface );
BOOLEAN DeleteVideoSurfaceFromIndex( UINT32 uiIndex );

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN BltVideoSurfaceToVideoSurface( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags, const SGPRect* SrcRect);

BOOLEAN BltVSurfaceUsingDD(HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT32 fBltFlags, INT32 iDestX, INT32 iDestY, SGPRect* SrcRect);

BOOLEAN ShadowVideoSurfaceRect(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceImage( UINT32	uiDestVSurface, HVOBJECT hImageHandle, INT32 iPosX, INT32 iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
//enlraged or shunk.
BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, SGPRect* SrcRect, SGPRect* DestRect);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

#ifdef __cplusplus
}
#endif

#endif
