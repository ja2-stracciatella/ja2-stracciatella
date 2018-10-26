#include "Debug.h"
#include "HImage.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"
#include "VideoScale.h"
#include "Local.h"

#include "UILayout.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>
#include <stdexcept>

// ******************************************************************************
//
// Video Object SGP Module
//
// Video Objects are used to contain any imagery which requires blitting. The data
// is contained within a Direct Draw surface. Palette information is in both
// a Direct Draw Palette and a 16BPP palette structure for 8->16 BPP Blits.
// Blitting is done via Direct Draw as well as custum blitters. Regions are
// used to define local coordinates within the surface
//
// Second Revision: Dec 10, 1996, Andrew Emmons
//
// *******************************************************************************


static SGPVObject* gpVObjectHead = 0;


SGPVObject::SGPVObject(SGPImage * const img) :
	flags_(),
	palette16_(),
	pix_data_{ img->pImageData.moveToUnique() },
	etrle_object_{ img->pETRLEObject.moveToUnique() },
	current_shade_(),
	subregion_count_{ img->usNumberOfObjects },
	bit_depth_{ img->ubBitDepth },
	next_(gpVObjectHead)
{
	std::fill(std::begin(pShades), std::end(pShades), nullptr);

	if (!(img->fFlags & IMAGE_TRLECOMPRESSED))
	{
		throw std::runtime_error("Image for video object creation must be TRLE compressed");
	}

	if (img->ubBitDepth == 8)
	{
		// move palette data over
		palette_ = img->pPalette.moveToUnique();
		Assert(palette_);

		palette16_     = Create16BPPPalette(palette_.get());
		current_shade_ = palette16_;
	}

	gpVObjectHead = this;
}


SGPVObject::~SGPVObject()
{
	for (SGPVObject** anchor = &gpVObjectHead;; anchor = &(*anchor)->next_)
	{
		if (*anchor != this) continue;
		*anchor = next_;
		break;
	}

	DestroyPalettes();
}


void SGPVObject::CurrentShade(size_t const idx)
{
	if (idx >= lengthof(pShades) || !pShades[idx])
	{
		throw std::logic_error("Tried to set invalid video object shade");
	}
	current_shade_ = pShades[idx];
}


ETRLEObject const& SGPVObject::SubregionProperties(size_t const idx) const
{
	if (idx >= SubregionCount())
	{
		throw std::logic_error(ST::format("Tried to access invalid subregion in video object: Maximum is {}, got {}", SubregionCount()-1, idx).c_str());
	}
	return etrle_object_[idx];
}


UINT8 const* SGPVObject::PixData(ETRLEObject const& e) const
{
	return &pix_data_[e.uiDataOffset];
}


#define COMPRESS_TRANSPARENT 0x80
#define COMPRESS_RUN_MASK    0x7F


UINT8 SGPVObject::GetETRLEPixelValue(UINT16 const usETRLEIndex, UINT16 const usX, UINT16 const usY) const
{
	ETRLEObject const& pETRLEObject = SubregionProperties(usETRLEIndex);

	if (usX >= pETRLEObject.usWidth || usY >= pETRLEObject.usHeight)
	{
		throw std::logic_error("Tried to get pixel from invalid coordinate");
	}

	// Assuming everything's okay, go ahead and look...
	const UINT8 *pCurrent = PixData(pETRLEObject);

	if(bit_depth_ == 32)
		return reinterpret_cast<const UINT32 *>(pCurrent)[usY * pETRLEObject.usWidth + usX];

	// Skip past all uninteresting scanlines
	for (UINT16 usLoopY = 0; usLoopY < usY; usLoopY++)
	{
		while (*pCurrent != 0)
		{
			if (*pCurrent & COMPRESS_TRANSPARENT)
			{
				pCurrent++;
			}
			else
			{
				pCurrent += *pCurrent & COMPRESS_RUN_MASK;
			}
		}
	}

	// Now look in this scanline for the appropriate byte
	UINT16 usLoopX = 0;
	do
	{
		UINT16 ubRunLength = *pCurrent & COMPRESS_RUN_MASK;

		if (*pCurrent & COMPRESS_TRANSPARENT)
		{
			if (usLoopX + ubRunLength >= usX) return 0;
			pCurrent++;
		}
		else
		{
			if (usLoopX + ubRunLength >= usX)
			{
				// skip to the correct byte; skip at least 1 to get past the byte defining the run
				pCurrent += (usX - usLoopX) + 1;
				return *pCurrent;
			}
			else
			{
				pCurrent += ubRunLength + 1;
			}
		}
		usLoopX += ubRunLength;
	}
	while (usLoopX < usX);

	throw std::logic_error("Inconsistent video object data");
}


/* Destroys the palette tables of a video object. All memory is deallocated, and
 * the pointers set to NULL. Be careful not to try and blit this object until
 * new tables are calculated, or things WILL go boom. */
void SGPVObject::DestroyPalettes()
{
	FOR_EACH(UINT16*, i, pShades)
	{
		if (flags_ & SHADETABLE_SHARED) continue;
		UINT16* const p = *i;
		if (!p)                         continue;
		if (palette16_ == p) palette16_ = 0;
		*i = 0;
		delete[] p;
	}

	if (UINT16* const p = palette16_)
	{
		palette16_ = 0;
		delete[] p;
	}

	current_shade_ = 0;
}


void SGPVObject::ShareShadetables(SGPVObject* const other)
{
	flags_ |= SHADETABLE_SHARED;
	for (size_t i = 0; i < lengthof(pShades); ++i)
	{
		pShades[i] = other->pShades[i];
	}
}


void InitializeVideoObjectManager(void)
{
	//Shouldn't be calling this if the video object manager already exists.
	//Call shutdown first...
	Assert(gpVObjectHead == NULL);
	gpVObjectHead = NULL;
}


void ShutdownVideoObjectManager(void)
{
	while (gpVObjectHead)
	{
		delete gpVObjectHead;
	}
}

SGPVObject* AddVideoObjectFromHImage(SGPImage *img)
{
	return new SGPVObject(img);
}

SGPVObject* AddScaledVideoObjectFromFile(const ST::string& ImageFile)
{
	AutoSGPImage img(CreateImage(ImageFile, IMAGE_ALLIMAGEDATA));
	AutoSGPImage hImage(ScaleImage(img.get(), g_ui.m_stdScreenScale));
	return AddVideoObjectFromHImage(hImage.get());
}

SGPVObject* AddScaledOutlineVideoObjectFromFile(const ST::string& ImageFile)
{
	AutoSGPImage img(CreateImage(ImageFile, IMAGE_ALLIMAGEDATA | IMAGE_REMOVE_PAL254));
	AutoSGPImage hImage(ScaleImage(img.get(), g_ui.m_stdScreenScale));
	return AddVideoObjectFromHImage(hImage.get());
}

SGPVObject* AddScaledAlphaVideoObjectFromFile(const ST::string& ImageFile)
{
	AutoSGPImage img(CreateImage(ImageFile, IMAGE_ALLIMAGEDATA | IMAGE_REMOVE_PAL1));
	AutoSGPImage hImage(ScaleAlphaImage(img.get(), g_ui.m_stdScreenScale));
	return AddVideoObjectFromHImage(hImage.get());
}

void BltVideoObject(SGPVSurface* const dst, SGPVObject const* const src, UINT16 const usRegionIndex, INT32 const iDestX, INT32 const iDestY)
{
	Assert(src->BPP() ==  8);
	Assert(dst->BPP() == 32);

	SGPVSurface::Lock l(dst);
	UINT16* const pBuffer = l.Buffer<UINT16>();
	UINT32  const uiPitch = l.Pitch();

	if (BltIsClipped(src, iDestX, iDestY, usRegionIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferTransparentClip(pBuffer, uiPitch, src, iDestX, iDestY, usRegionIndex, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferTransparent(pBuffer, uiPitch, src, iDestX, iDestY, usRegionIndex);
	}
}


void BltVideoObjectOutline(SGPVSurface* const dst, SGPVObject const* const hSrcVObject, UINT16 const usIndex, INT32 const iDestX, INT32 const iDestY, UINT32 const s16BPPColor)
{
	SGPVSurface::Lock l(dst);
	UINT16* const pBuffer = l.Buffer<UINT16>();
	UINT32  const uiPitch = l.Pitch();

	if (BltIsClipped(hSrcVObject, iDestX, iDestY, usIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferOutlineClip(pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferOutline(pBuffer, uiPitch, hSrcVObject, iDestX, iDestY, usIndex, s16BPPColor);
	}
}


void BltVideoObjectOutlineShadow(SGPVSurface* const dst, const SGPVObject* const src, const UINT16 usIndex, const INT32 iDestX, const INT32 iDestY)
{
	SGPVSurface::Lock l(dst);
	UINT16* const pBuffer = l.Buffer<UINT16>();
	UINT32  const uiPitch = l.Pitch();

	if (BltIsClipped(src, iDestX, iDestY, usIndex, &ClippingRect))
	{
		Blt8BPPDataTo16BPPBufferOutlineShadowClip(pBuffer, uiPitch, src, iDestX, iDestY, usIndex, &ClippingRect);
	}
	else
	{
		Blt8BPPDataTo16BPPBufferOutlineShadow(pBuffer, uiPitch, src, iDestX, iDestY, usIndex);
	}
}


void BltVideoObjectOnce(SGPVSurface* const dst, char const* const filename, UINT16 const region, INT32 const x, INT32 const y)
{
	Assert(dst->BPP() == 32);
	AutoSGPVObject vo(AddScaledVideoObjectFromFile(filename));
	BltVideoObject(dst, vo.get(), region, x, y);
}
