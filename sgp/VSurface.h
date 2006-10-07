#ifndef __VSURFACE_H
#define __VSURFACE_H

#include "Types.h"
#include "HImage.h"
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

#define VS_BLT_COLORFILL												0x000000020
#define VS_BLT_USECOLORKEY											0x000000002
#define VS_BLT_USEDESTCOLORKEY									0x000000200
#define VS_BLT_FAST															0x000000004
#define VS_BLT_CLIPPED													0x000000008
#define VS_BLT_SRCSUBRECT												0x000000040
#define VS_BLT_COLORFILLRECT										0x000000100
#define VS_BLT_MIRROR_Y													0x000001000

//
// Effects structure for specialized blitting
//

typedef struct
{
	COLORVAL ColorFill;		// Used for fill effect
	SGPRect	 SrcRect;			// Given SRC subrect instead of srcregion
	SGPRect	 FillRect;		// Given SRC subrect instead of srcregion
	UINT16	 DestRegion;  // Given a DEST region for dest positions within the VO

} blt_vs_fx;

//
// Video Surface Flags
// Used to describe the memory usage of a video Surface
//

#define	VSURFACE_DEFAULT_MEM_USAGE		0x00000001			// Default mem usage is same as DD, try video and then try system. Will usually work
#define	VSURFACE_VIDEO_MEM_USAGE			0x00000002			// Will force surface into video memory and will fail if it can't
#define	VSURFACE_SYSTEM_MEM_USAGE			0x00000004			// Will force surface into system memory and will fail if it can't
#define VSURFACE_RESERVED_SURFACE			0x00000100			// Reserved for special purposes, like a primary surface

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
	PTR							pSurfaceData1;				// Direct Draw One Interface
	PTR							pSavedSurfaceData1;		// A void pointer, but for this implementation, is really a lpDirectDrawSurface;
																				// pSavedSurfaceData is used to hold all video memory Surfaces so that they my be restored
	PTR							pSavedSurfaceData;		// A void pointer, but for this implementation, is really a lpDirectDrawSurface;
																				// pSavedSurfaceData is used to hold all video memory Surfaces so that they my be restored
	UINT32					fFlags;								// Used to describe memory usage, etc
	SDL_Color* pPalette;
	UINT16					*p16BPPPalette;				// A 16BPP palette used for 8->16 blits
	COLORVAL				TransparentColor;			// Defaults to 0,0,0
	PTR							pClipper;							// A void pointer encapsolated as a clipper Surface
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

// Restores all video Surfaces in list
BOOLEAN RestoreVideoSurfaces( );

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
BOOLEAN BltVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, blt_vs_fx *pBltFx );

BOOLEAN ColorFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2,	INT32 iDestY2, UINT16 Color16BPP);

BOOLEAN ImageFillVideoSurfaceArea(UINT32 uiDestVSurface, INT32 iDestX1, INT32 iDestY1, INT32 iDestX2, INT32 iDestY2, HVOBJECT BkgrndImg, UINT16 Index, INT16 Ox, INT16 Oy);

// Sets transparency
BOOLEAN SetVideoSurfaceTransparency( UINT32 uiIndex, COLORVAL TransColor );

// Gets width, hight, bpp information
BOOLEAN GetVideoSurfaceDescription( UINT32 uiIndex, UINT16 *usWidth, UINT16 *usHeight, UINT8 *ubBitDepth );

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Video Surface manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Darkens a rectangular area on a surface for menus etc.
BOOLEAN PixelateVideoSurfaceRect(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

// Created from a VSurface_DESC structure. Can be from a file via HIMAGE or empty.
HVSURFACE CreateVideoSurface( VSURFACE_DESC *VSurfaceDesc );

// Gets the RGB palette entry values
BOOLEAN GetVSurfacePaletteEntries( HVSURFACE hVSurface, SGPPaletteEntry *pPalette );

BOOLEAN RestoreVideoSurface( HVSURFACE hVSurface );

// Returns a flat pointer for direct manipulation of data
BYTE *LockVideoSurfaceBuffer( HVSURFACE hVSurface, UINT32 *pPitch );

// Must be called after Locking buffer call above
void UnLockVideoSurfaceBuffer( HVSURFACE hVSurface );

// Set data from HIMAGE.
BOOLEAN SetVideoSurfaceDataFromHImage( HVSURFACE hVSurface, HIMAGE hImage, UINT16 usX, UINT16 usY, SGPRect *pSrcRect );

// Sets Transparency color into HVSurface and the underlying DD surface
BOOLEAN SetVideoSurfaceTransparencyColor( HVSURFACE hVSurface, COLORVAL TransColor );

// Sets HVSurface palette, creates if nessessary. Also sets 16BPP palette
BOOLEAN SetVideoSurfacePalette( HVSURFACE hVSurface, SGPPaletteEntry *pSrcPalette );

// Used if it's in video memory, will re-load backup copy
//BOOLEAN RestoreVideoSurface( HVSurface hVSurface );

// Deletes all data, including palettes, regions, DD Surfaces
BOOLEAN DeleteVideoSurface( HVSURFACE hVSurface );
BOOLEAN DeleteVideoSurfaceFromIndex( UINT32 uiIndex );

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Clipper manipulation functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOLEAN SetClipList( HVSURFACE hVSurface, SGPRect *RegionData, UINT16 usNumRegions );

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Blt Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN BltVideoSurfaceToVideoSurface( HVSURFACE hDestVSurface, HVSURFACE hSrcVSurface, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT32 fBltFlags, blt_vs_fx *pBltFx );

HVSURFACE GetBackBufferVideoSurface( );

BOOLEAN ShadowVideoSurfaceRect(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);
BOOLEAN ShadowVideoSurfaceImage( UINT32	uiDestVSurface, HVOBJECT hImageHandle, INT32 iPosX, INT32 iPosY);

// If the Dest Rect and the source rect are not the same size, the source surface will be either
//enlraged or shunk.
BOOLEAN BltStretchVideoSurface(UINT32 uiDestVSurface, UINT32 uiSrcVSurface, INT32 iDestX, INT32 iDestY, UINT32 fBltFlags, SGPRect *SrcRect, SGPRect *DestRect );

BOOLEAN MakeVSurfaceFromVObject(UINT32 uiVObject, UINT16 usSubIndex, UINT32 *puiVSurface);

BOOLEAN ShadowVideoSurfaceRectUsingLowPercentTable(  UINT32	uiDestVSurface, INT32 X1, INT32 Y1, INT32 X2, INT32 Y2);

#ifdef __cplusplus
}
#endif

#endif
