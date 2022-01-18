#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "Buffer.h"
#include "Types.h"
#include <memory>


// Defines for HVOBJECT limits
#define HVOBJECT_SHADE_TABLES										48


// Z-buffer info structure for properly assigning Z values
struct ZStripInfo
{
	INT8  bInitialZChange;    // difference in Z value between the leftmost and base strips
	UINT8 ubFirstZStripWidth; // # of pixels in the leftmost strip
	UINT8 ubNumberOfZChanges; // number of strips (after the first)
	INT8* pbZChange;          // change to the Z value in each strip (after the first)
};

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g ,b)  ((UINT32) (((UINT8) (r) | ((UINT16) (g) << 8)) | (((UINT32) (UINT8) (b)) << 16)))

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

		UINT16 const* CurrentShade() const { return current_shade_; }

		// Set the current object shade table
		void CurrentShade(size_t idx);

		UINT16 SubregionCount() const { return subregion_count_; }

		ETRLEObject const& SubregionProperties(size_t idx) const;

		UINT8 const* PixData(ETRLEObject const&) const;

		/* Given a ETRLE image index, retrieves the value of the pixel located at
		 * the given image coordinates. The value returned is an 8-bit palette index
		 */
		UINT8 GetETRLEPixelValue(UINT16 usETLREIndex, UINT16 usX, UINT16 usY) const;

		// Deletes the 16-bit palette tables
		void DestroyPalettes();

		void ShareShadetables(SGPVObject*);

		enum Flags
		{
			NONE              = 0,
			SHADETABLE_SHARED = 1U << 0
		};

	private:
		Flags                        flags_;                         // Special flags
		UINT32                       pix_data_size_;                 // ETRLE data size
		SGP::Buffer<SGPPaletteEntry> palette_;                       // 8BPP Palette
		UINT16*                      palette16_;                     // A 16BPP palette used for 8->16 blits

		UINT8*                       pix_data_;                      // ETRLE pixel data
		ETRLEObject*                 etrle_object_;                  // Object offset data etc
	public:
		UINT16*                      pShades[HVOBJECT_SHADE_TABLES]; // Shading tables
	private:
		UINT16 const*                current_shade_;
	public:
		ZStripInfo**                 ppZStripInfo;                   // Z-value strip info arrays

	private:
		UINT16                       subregion_count_;               // Total number of objects
		UINT8                        bit_depth_;                     // BPP

	public:
#ifdef SGP_VIDEO_DEBUGGING
		char*                        name_;
		char*                        code_;
#endif
		SGPVObject*                  next_;
};
ENUM_BITSET(SGPVObject::Flags)


// Creates a list to contain video objects
void InitializeVideoObjectManager(void);

// Deletes any video object placed into list
void ShutdownVideoObjectManager(void);

// Creates and adds a video object to list
#ifdef SGP_VIDEO_DEBUGGING
extern UINT32 guiVObjectSize;

	void PerformVideoInfoDumpIntoFile(const ST::string& filename, BOOLEAN fAppend);
	SGPVObject* AddAndRecordVObjectFromHImage(SGPImage*, UINT32 uiLineNum, const ST::string& pSourceFile);
	SGPVObject* AddAndRecordVObjectFromFile(const ST::string& ImageFile, UINT32 uiLineNum, const ST::string& pSourceFile);
	#define AddVideoObjectFromHImage(a) AddAndRecordVObjectFromHImage(a, __LINE__, __FILE__)
	#define AddVideoObjectFromFile(a)   AddAndRecordVObjectFromFile(  a, __LINE__, __FILE__)
#else
	SGPVObject* AddStandardVideoObjectFromHImage(SGPImage*);
	SGPVObject* AddStandardVideoObjectFromFile(const ST::string& ImageFile);
	#define AddVideoObjectFromHImage(a) AddStandardVideoObjectFromHImage(a)
	#define AddVideoObjectFromFile(a)   AddStandardVideoObjectFromFile(a)
#endif

// Removes a video object
static inline void DeleteVideoObject(SGPVObject* const vo)
{
	delete vo;
}

// Blits a video object to another video object
void BltVideoObject(SGPVSurface* dst, SGPVObject const* src, UINT16 usRegionIndex, INT32 iDestX, INT32 iDestY);


void BltVideoObjectOutline(SGPVSurface* dst, SGPVObject const* src, UINT16 usIndex, INT32 iDestX, INT32 iDestY, INT16 s16BPPColor);
void BltVideoObjectOutlineShadow(SGPVSurface* dst, SGPVObject const* src, UINT16 usIndex, INT32 iDestX, INT32 iDestY);

/* Loads a video object, blits it once and frees it */
void BltVideoObjectOnce(SGPVSurface* dst, char const* filename, UINT16 region, INT32 x, INT32 y);

typedef std::unique_ptr<SGPVObject> AutoSGPVObject;

#endif
