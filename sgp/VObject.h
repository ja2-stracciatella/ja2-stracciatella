#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "Types.h"
#include "HImage.h"

// ************************************************************************************
//
// Video Object SGP Module
//
// ************************************************************************************


// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
typedef UINT32 COLORVAL;

// Defines for HVOBJECT limits
#define HVOBJECT_SHADE_TABLES										48


// Z-buffer info structure for properly assigning Z values
typedef struct
{
	INT8			bInitialZChange;		// difference in Z value between the leftmost and base strips
	UINT8			ubFirstZStripWidth;	// # of pixels in the leftmost strip
	UINT8			ubNumberOfZChanges;	// number of strips (after the first)
	INT8 *		pbZChange;			// change to the Z value in each strip (after the first)
} ZStripInfo;

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g ,b)  ((UINT32) (((UINT8) (r) | ((UINT16) (g) << 8)) | (((UINT32) (UINT8) (b)) << 16)))

// VOBJECT FLAGS
#define	VOBJECT_FLAG_SHADETABLE_SHARED 0x00000100

// This structure is a video object.
// The video object contains different data based on it's type, compressed or not
typedef struct TAG_HVOBJECT
{
	UINT32							fFlags;								// Special flags
	UINT32							uiSizePixData;			// ETRLE data size
	SGPPaletteEntry			*pPaletteEntry;				// 8BPP Palette
	UINT16							*p16BPPPalette;				// A 16BPP palette used for 8->16 blits

	PTR										pPixData;						// ETRLE pixel data
	ETRLEObject						*pETRLEObject;				// Object offset data etc
	UINT16								*pShades[HVOBJECT_SHADE_TABLES]; // Shading tables
	UINT16								*pShadeCurrent;
	ZStripInfo						**ppZStripInfo;				// Z-value strip info arrays

	UINT16								usNumberOfObjects;	// Total number of objects
	UINT8									ubBitDepth;						// BPP
} SGPVObject, *HVOBJECT;


// **********************************************************************************
//
// Video Object Manager Functions
//
// **********************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

// Creates a list to contain video objects
BOOLEAN InitializeVideoObjectManager( );

// Deletes any video object placed into list
BOOLEAN ShutdownVideoObjectManager( );

// Creates and adds a video object to list
#ifdef SGP_VIDEO_DEBUGGING
	void PerformVideoInfoDumpIntoFile(const char *filename, BOOLEAN fAppend);
	BOOLEAN AddAndRecordVObjectFromHImage(HIMAGE hImage, UINT32* uiIndex, UINT32 uiLineNum, const char* pSourceFile);
	BOOLEAN AddAndRecordVObjectFromFile(const char* ImageFile, UINT32* uiIndex, UINT32 uiLineNum, const char* pSourceFile);
	#define AddVideoObjectFromHImage(a, b) AddAndRecordVObjectFromHImage(a, b, __LINE__, __FILE__)
	#define AddVideoObjectFromFile(a, b) AddAndRecordVObjectFromFile(a, b, __LINE__, __FILE__)
#else
	#define AddVideoObjectFromHImage(a, b) AddStandardVideoObjectFromHImage(a, b)
	#define AddVideoObjectFromFile(a, b) AddStandardVideoObjectFromFile(a, b)
#endif

BOOLEAN AddStandardVideoObjectFromHImage(HIMAGE hImage, UINT32* uiIndex);
BOOLEAN AddStandardVideoObjectFromFile(const char* ImageFile, UINT32* uiIndex);

// Removes a video object
BOOLEAN DeleteVideoObjectFromIndex( UINT32 uiVObject  );

// Returns a HVOBJECT for the specified index
HVOBJECT GetVideoObject(UINT32 uiIndex);

// Blits a video object to another video object
BOOLEAN BltVideoObject(UINT32 uiDestVSurface, HVOBJECT hVSrcObject, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY);

BOOLEAN BltVideoObjectFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY);


// **********************************************************************************
//
// Video Object manipulation functions
//
// **********************************************************************************

HVOBJECT CreateVideoObject(HIMAGE hImage);
HVOBJECT CreateVideoObjectFromFile(const char* Filename);

// Deletes all data
BOOLEAN DeleteVideoObject( HVOBJECT hVObject );

// Deletes the 16-bit palette tables
BOOLEAN DestroyObjectPaletteTables(HVOBJECT hVObject);

// Sets the current object shade table
UINT16 SetObjectShade(HVOBJECT pObj, UINT32 uiShade);

// Sets the current object shade table using a vobject handle
UINT16 SetObjectHandleShade(UINT32 uiHandle, UINT32 uiShade);

// Retrieves an HVOBJECT pixel value
BOOLEAN GetETRLEPixelValue( UINT8 * pDest, HVOBJECT hVObject, UINT16 usETLREIndex, UINT16 usX, UINT16 usY );


// ****************************************************************************
//
// Blt Functions
//
// ****************************************************************************

// These blitting functions more-or less encapsolate all of the functionality of DirectDraw
// Blitting, giving an API layer for portability.

BOOLEAN GetVideoObjectETRLEPropertiesFromIndex( UINT32 uiVideoObject, ETRLEObject *pETRLEObject, UINT16 usIndex );
BOOLEAN GetVideoObjectETRLESubregionProperties( UINT32 uiVideoObject, UINT16 usIndex, UINT16 *pusWidth, UINT16 *pusHeight );

BOOLEAN BltVideoObjectOutlineFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline );
BOOLEAN BltVideoObjectOutline(UINT32 uiDestVSurface, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline );
BOOLEAN BltVideoObjectOutlineShadowFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY );

#ifdef __cplusplus
}
#endif

#endif
