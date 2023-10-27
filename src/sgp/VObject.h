#ifndef __VOBJECT_H
#define __VOBJECT_H

#include "Types.h"
#include "VideoScale.h"
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

#define RGBA(r, g, b, a)  (UINT32(r) << 24 | UINT32(g) << 16 | UINT32(b) << 8 | UINT32(a))
#define RGB(r, g, b)  (UINT32(r) << 24 | UINT32(g) << 16 | UINT32(b) << 8 | 0xFF)
#define SHADE_NONE RGBA(0, 0, 0, 0)
#define SHADE_MONO(r, g, b) RGBA(r, g, b, 1)
#define SHADE_STD(r, g, b) RGBA(r, g, b, 2)

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

		UINT32 CurrentShade() const { return current_shade_; }

		// Set the current object shade table
		void CurrentShade(size_t idx);
		void SetShadeColor(UINT32 rgba) { current_shade_ = rgba; }

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

		std::unique_ptr<UINT8 const []> pix_data_;                   // ETRLE pixel data
		std::unique_ptr<ETRLEObject const []> etrle_object_;         // Object offset data etc
	public:
		UINT32                       pShades[HVOBJECT_SHADE_TABLES]; // Shading values
	private:
		UINT32                       current_shade_;
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
SGPVObject* AddVideoObjectFromHImage(SGPImage *img);
SGPVObject* AddScaledVideoObjectFromFile(const ST::string& ImageFile, ScaleCallback *callback=nullptr);
SGPVObject* AddScaledOutlineVideoObjectFromFile(const ST::string& ImageFile);
SGPVObject* AddScaledAlphaVideoObjectFromFile(const ST::string& ImageFile);

inline SGPVObject* AddVideoObjectFromFile(const ST::string& ImageFile)
{
	return AddScaledVideoObjectFromFile(ImageFile);
}

// Removes a video object
inline void DeleteVideoObject(SGPVObject* const vo)
{
	delete vo;
}

// Blits a video object to another video object
void BltVideoObject(SGPVSurface *dst, const SGPVObject *src, const UINT16 usRegionIndex, const INT32 iDestX, const INT32 iDestY);
void BltVideoObjectOutline(SGPVSurface* const dst, SGPVObject const* const hSrcVObject, UINT16 const usIndex, INT32 const iDestX, INT32 const iDestY, const UINT32 colOutline);
void BltVideoObjectOutlineShadow(SGPVSurface *dst, const SGPVObject *src, const UINT16 usIndex, const INT32 iDestX, const INT32 iDestY);

// Loads a video object, blits it once and frees it
void BltVideoObjectOnce(SGPVSurface* dst, char const* filename, UINT16 region, INT32 x, INT32 y);

typedef std::unique_ptr<SGPVObject> AutoSGPVObject;

#endif
