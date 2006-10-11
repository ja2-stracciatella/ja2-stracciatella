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

//
// Video Surface Flags
// Used to describe the memory usage of a video Surface
//

#define	VSURFACE_VIDEO_MEM_USAGE			0x00000002			// Will force surface into video memory and will fail if it can't
#define	VSURFACE_SYSTEM_MEM_USAGE			0x00000004			// Will force surface into system memory and will fail if it can't

//
// Video Surface creation flags
// Used in the VSurface_DESC structure to describe creation flags
//

#define VSURFACE_CREATE_DEFAULT			  0x00000020		// Creates and empty Surface of given width, height and BPP
#define VSURFACE_CREATE_FROMFILE			 0x00000040		// Creates a video Surface from a file ( using HIMAGE )


typedef struct
{
	SDL_Surface* surface;
	UINT16					usHeight;							// Height of Video Surface
	UINT16					usWidth;							// Width of Video Surface
	UINT8						ubBitDepth;						// BPP ALWAYS 16!
	PTR							pSurfaceData;					// A void pointer, but for this implementation, is really a lpDirectDrawSurface;
	SDL_Color* pPalette;
	UINT16					*p16BPPPalette;				// A 16BPP palette used for 8->16 blits
} SGPVSurface, *HVSURFACE;

//
// This structure describes the creation parameters for a Video Surface
//

typedef struct
{
	UINT32				fCreateFlags;						// Specifies creation flags like from file or not
	SGPFILENAME		ImageFile;							// Filename of image data to use
	UINT16				usWidth;								// Width, ignored if given from file
	UINT16				usHeight;								// Height, ignored if given from file
	UINT8					ubBitDepth;							// BPP, ignored if given from file

} VSURFACE_DESC;

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

// Creates and adds a video Surface to list
#ifdef SGP_VIDEO_DEBUGGING
	void DumpVSurfaceInfoIntoFile(const char *filename, BOOLEAN fAppend);
	extern BOOLEAN _AddAndRecordVSurface(VSURFACE_DESC *VSurfaceDesc, UINT32 *uiIndex, UINT32 uiLineNum, const char *pSourceFile);
	#define AddVideoSurface( a, b )			_AddAndRecordVSurface( a, b, __LINE__, __FILE__ )
#else
	#define AddVideoSurface( a, b )			AddStandardVideoSurface( a, b )
#endif

BOOLEAN AddStandardVideoSurface( VSURFACE_DESC *VSurfaceDesc, UINT32 *uiIndex );

// Returns a HVSurface for the specified index
BOOLEAN GetVideoSurface( HVSURFACE *hVSurface, UINT32 uiIndex );

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

// Created from a VSurface_DESC structure. Can be from a file via HIMAGE or empty.
HVSURFACE CreateVideoSurface( VSURFACE_DESC *VSurfaceDesc );

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

BOOLEAN ShadowVideoSurfaceRect(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceImage( UINT32	uiDestVSurface, HVOBJECT hImageHandle, INT32 iPosX, INT32 iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
//enlraged or shunk.
BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, SGPRect *SrcRect, SGPRect *DestRect );

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

#ifdef __cplusplus
}
#endif

#endif
