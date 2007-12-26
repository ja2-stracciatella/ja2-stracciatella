#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "Types.h"


#define NO_VOBJECT 0


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
struct SGPVObject
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
};


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
	SGPVObject* AddAndRecordVObjectFromHImage(HIMAGE hImage, UINT32 uiLineNum, const char* pSourceFile);
	SGPVObject* AddAndRecordVObjectFromFile(const char* ImageFile, UINT32 uiLineNum, const char* pSourceFile);
	#define AddVideoObjectFromHImage(a) AddAndRecordVObjectFromHImage(a, __LINE__, __FILE__)
	#define AddVideoObjectFromFile(a)   AddAndRecordVObjectFromFile(  a, __LINE__, __FILE__)
#else
	SGPVObject* AddStandardVideoObjectFromHImage(HIMAGE hImage);
	SGPVObject* AddStandardVideoObjectFromFile(const char* ImageFile);
	#define AddVideoObjectFromHImage(a) AddStandardVideoObjectFromHImage(a)
	#define AddVideoObjectFromFile(a)   AddStandardVideoObjectFromFile(a)
#endif

// Removes a video object
BOOLEAN DeleteVideoObject(SGPVObject*);

// Blits a video object to another video object
BOOLEAN BltVideoObject(SGPVSurface* dst, const SGPVObject* src, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY);

// Deletes the 16-bit palette tables
BOOLEAN DestroyObjectPaletteTables(HVOBJECT hVObject);

// Sets the current object shade table
UINT16 SetObjectShade(HVOBJECT pObj, UINT32 uiShade);

// Retrieves an HVOBJECT pixel value
BOOLEAN GetETRLEPixelValue(UINT8* pDest, HVOBJECT hVObject, UINT16 usETLREIndex, UINT16 usX, UINT16 usY);


const ETRLEObject* GetVideoObjectETRLESubregionProperties(const SGPVObject*, UINT16 usIndex);

BOOLEAN BltVideoObjectOutline(SGPVSurface* dst, const SGPVObject* src, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor, BOOLEAN fDoOutline);
BOOLEAN BltVideoObjectOutlineShadow(SGPVSurface* dst, const SGPVObject* src, UINT16 usIndex, INT32 iDestX, INT32 iDestY);

/* Loads a video object, blits it once and frees it */
BOOLEAN BltVideoObjectOnce(SGPVSurface* dst, const char* filename, UINT16 region, INT32 x, INT32 y);

extern UINT32 guiVObjectSize;

#ifdef __cplusplus
}
#endif

#endif
