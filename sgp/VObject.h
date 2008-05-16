#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "AutoObj.h"
#include "Buffer.h"
#include "Types.h"


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
class SGPVObject
{
	public:
		SGPVObject(SGPImage const*);
		~SGPVObject();

		UINT8 BPP() const { return bit_depth_; }

		SGPPaletteEntry const* Palette() const { return palette_; }

		UINT16 const* Palette16() const { return palette16_; }

		UINT16 const* CurrentShade() const { return pShadeCurrent; }

		// Set the current object shade table
		void CurrentShade(size_t idx);

		size_t SubregionCount() const { return subregion_count_; }

		ETRLEObject const* SubregionProperties(size_t idx) const;

		// Deletes the 16-bit palette tables
		void DestroyPalettes();

		UINT32                       fFlags;                         // Special flags
		UINT32                       uiSizePixData;                  // ETRLE data size
	private:
		SGP::Buffer<SGPPaletteEntry> palette_;                       // 8BPP Palette
		UINT16*                      palette16_;                     // A 16BPP palette used for 8->16 blits

	public:
		PTR                          pPixData;                       // ETRLE pixel data
		ETRLEObject*                 pETRLEObject;                   // Object offset data etc
		UINT16*                      pShades[HVOBJECT_SHADE_TABLES]; // Shading tables
		UINT16*                      pShadeCurrent;
		ZStripInfo**                 ppZStripInfo;                   // Z-value strip info arrays

	private:
		UINT16                       subregion_count_;               // Total number of objects
		UINT8                        bit_depth_;                     // BPP
};


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
void DeleteVideoObject(SGPVObject*);

// Blits a video object to another video object
BOOLEAN BltVideoObject(SGPVSurface* dst, const SGPVObject* src, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY);

// Retrieves an HVOBJECT pixel value
BOOLEAN GetETRLEPixelValue(UINT8* pDest, HVOBJECT hVObject, UINT16 usETLREIndex, UINT16 usX, UINT16 usY);


BOOLEAN BltVideoObjectOutline(SGPVSurface* dst, const SGPVObject* src, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor);
BOOLEAN BltVideoObjectOutlineShadow(SGPVSurface* dst, const SGPVObject* src, UINT16 usIndex, INT32 iDestX, INT32 iDestY);

/* Loads a video object, blits it once and frees it */
BOOLEAN BltVideoObjectOnce(SGPVSurface* dst, const char* filename, UINT16 region, INT32 x, INT32 y);

extern UINT32 guiVObjectSize;

typedef SGP::AutoObj<SGPVObject, DeleteVideoObject>::Type AutoSGPVObject;

#endif
