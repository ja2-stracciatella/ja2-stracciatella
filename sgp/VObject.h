#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "Types.h"
#include "HImage.h"


#define NO_VOBJECT 0


// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
typedef UINT32 COLORVAL;

// Defines for HVOBJECT limits
#define HVOBJECT_SHADE_TABLES										48


// Z-buffer info structure for properly assigning Z values
typedef struct
{
	INT8  bInitialZChange;    // difference in Z value between the leftmost and base strips
	UINT8 ubFirstZStripWidth; // # of pixels in the leftmost strip
	UINT8 ubNumberOfZChanges; // number of strips (after the first)
	INT8* pbZChange;          // change to the Z value in each strip (after the first)
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
	UINT32           fFlags;                         // Special flags
	UINT32           uiSizePixData;                  // ETRLE data size
	SGPPaletteEntry* pPaletteEntry;                  // 8BPP Palette
	UINT16*          p16BPPPalette;                  // A 16BPP palette used for 8->16 blits

	PTR              pPixData;                       // ETRLE pixel data
	ETRLEObject*     pETRLEObject;                   // Object offset data etc
	UINT16*          pShades[HVOBJECT_SHADE_TABLES]; // Shading tables
	UINT16*          pShadeCurrent;
	ZStripInfo**     ppZStripInfo;                   // Z-value strip info arrays

	UINT16           usNumberOfObjects;              // Total number of objects
	UINT8            ubBitDepth;                     // BPP
} SGPVObject, *HVOBJECT;


#ifdef __cplusplus
extern "C" {
#endif

// Creates a list to contain video objects
BOOLEAN InitializeVideoObjectManager(void);

// Deletes any video object placed into list
BOOLEAN ShutdownVideoObjectManager(void);

// Creates and adds a video object to list
#ifdef SGP_VIDEO_DEBUGGING
	void PerformVideoInfoDumpIntoFile(const char* filename, BOOLEAN fAppend);
	UINT32 AddAndRecordVObjectFromHImage(HIMAGE hImage, UINT32 uiLineNum, const char* pSourceFile);
	UINT32 AddAndRecordVObjectFromFile(const char* ImageFile, UINT32 uiLineNum, const char* pSourceFile);
	#define AddVideoObjectFromHImage(a) AddAndRecordVObjectFromHImage(a, __LINE__, __FILE__)
	#define AddVideoObjectFromFile(a) AddAndRecordVObjectFromFile(a, __LINE__, __FILE__)
#else
	#define AddVideoObjectFromHImage(a) AddStandardVideoObjectFromHImage(a)
	#define AddVideoObjectFromFile(a) AddStandardVideoObjectFromFile(a)
#endif

UINT32 AddStandardVideoObjectFromHImage(HIMAGE hImage);
UINT32 AddStandardVideoObjectFromFile(const char* ImageFile);

// Removes a video object
BOOLEAN DeleteVideoObjectFromIndex(UINT32 uiVObject);

// Returns a HVOBJECT for the specified index
HVOBJECT GetVideoObject(UINT32 uiIndex);

// Blits a video object to another video object
BOOLEAN BltVideoObject(UINT32 uiDestVSurface, HVOBJECT hVSrcObject, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY);

BOOLEAN BltVideoObjectFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY);


HVOBJECT CreateVideoObject(HIMAGE hImage);
HVOBJECT CreateVideoObjectFromFile(const char* Filename);

// Deletes all data
BOOLEAN DeleteVideoObject(HVOBJECT hVObject);

// Deletes the 16-bit palette tables
BOOLEAN DestroyObjectPaletteTables(HVOBJECT hVObject);

// Sets the current object shade table
UINT16 SetObjectShade(HVOBJECT pObj, UINT32 uiShade);

// Sets the current object shade table using a vobject handle
UINT16 SetObjectHandleShade(UINT32 uiHandle, UINT32 uiShade);

// Retrieves an HVOBJECT pixel value
BOOLEAN GetETRLEPixelValue(UINT8* pDest, HVOBJECT hVObject, UINT16 usETLREIndex, UINT16 usX, UINT16 usY);


const ETRLEObject* GetVideoObjectETRLESubregionProperties(UINT32 uiVideoObject, UINT16 usIndex);

BOOLEAN BltVideoObjectOutlineFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutline(UINT32 uiDestVSurface, HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutlineShadowFromIndex(UINT32 uiDestVSurface, UINT32 uiSrcVObject, UINT16 usIndex, INT32 iDestX, INT32 iDestY);

#ifdef __cplusplus
}
#endif

#endif
