#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "Types.h"
#include <memory>


// Defines for HVOBJECT limits
#define HVOBJECT_SHADE_TABLES										48


// Z-buffer info structure for properly assigning Z values
struct ZStripInfo
{
	INT8  pbZChange[16];      // change to the Z value in each strip (after the first)
	INT8  bInitialZChange;    // difference in Z value between the leftmost and base strips
	UINT8 ubFirstZStripWidth; // # of pixels in the leftmost strip
	UINT8 ubNumberOfZChanges; // number of strips (after the first)
};

// This definition mimics what is found in WINDOWS.H ( for Direct Draw compatiblity )
// From RGB to COLORVAL
#define FROMRGB(r, g ,b)  ((UINT32) (((UINT8) (r) | ((UINT16) (g) << 8)) | (((UINT32) (UINT8) (b)) << 16)))

// This structure is a video object.
// The video object contains different data based on it's type, compressed or not
class SGPVObject
{
	public:
		// This modifies the SGPImage: relevant data is moved away from it
		SGPVObject(SGPImage *);
		~SGPVObject();

		UINT8 BPP() const { return bit_depth_; }

		SGPPaletteEntry const* Palette() const { return palette_.get(); }

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
		std::unique_ptr<SGPPaletteEntry const []> palette_;          // 8BPP Palette
		UINT16*                      palette16_;                     // A 16BPP palette used for 8->16 blits

		std::unique_ptr<UINT8 const []> pix_data_;                   // ETRLE pixel data
		std::unique_ptr<ETRLEObject const []> etrle_object_;         // Object offset data etc
	public:
		UINT16*                      pShades[HVOBJECT_SHADE_TABLES]; // Shading tables
	private:
		UINT16 const*                current_shade_;
	public:
		// Smart pointer to an array of smart pointers to ZStripInfo structs.
		std::unique_ptr<std::unique_ptr<ZStripInfo> []> ppZStripInfo;// Z-value strip info arrays

	private:
		UINT16                       subregion_count_;               // Total number of objects
		UINT8                        bit_depth_;                     // BPP

	public:
		SGPVObject*                  next_;
};
ENUM_BITSET(SGPVObject::Flags)


// Creates a list to contain video objects
void InitializeVideoObjectManager(void);

// Deletes any video object placed into list
void ShutdownVideoObjectManager(void);

// Creates and adds a video object to list
SGPVObject* AddVideoObjectFromHImage(SGPImage*);
SGPVObject* AddVideoObjectFromFile(const ST::string& ImageFile);

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
