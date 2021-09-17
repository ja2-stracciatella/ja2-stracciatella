#include "Debug.h"
#include "HImage.h"
#include "MemMan.h"
#include "FileMan.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"

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


SGPVObject::SGPVObject(SGPImage const* const img) :
	flags_(),
	palette16_(),
	current_shade_(),
	ppZStripInfo(),
#ifdef SGP_VIDEO_DEBUGGING
	name_(),
	code_(),
#endif
	next_(gpVObjectHead)
{
	std::fill(std::begin(pShades), std::end(pShades), nullptr);

	if (!(img->fFlags & IMAGE_TRLECOMPRESSED))
	{
		throw std::runtime_error("Image for video object creation must be TRLE compressed");
	}

	ETRLEData TempETRLEData;
	GetETRLEImageData(img, &TempETRLEData);

	subregion_count_ = TempETRLEData.usNumberOfObjects;
	etrle_object_    = TempETRLEData.pETRLEObject;
	pix_data_        = static_cast<UINT8*>(TempETRLEData.pPixData);
	pix_data_size_   = TempETRLEData.uiSizePixData;
	bit_depth_       = img->ubBitDepth;

	if (img->ubBitDepth == 8)
	{
		// create palette
		const SGPPaletteEntry* const src_pal = img->pPalette;
		Assert(src_pal != NULL);

		SGPPaletteEntry* const pal = palette_.Allocate(256);
		memcpy(pal, src_pal, sizeof(*pal) * 256);

		palette16_     = Create16BPPPalette(pal);
		current_shade_ = palette16_;
	}

	gpVObjectHead = this;
#ifdef SGP_VIDEO_DEBUGGING
	++guiVObjectSize;
#endif
}


SGPVObject::~SGPVObject()
{
	for (SGPVObject** anchor = &gpVObjectHead;; anchor = &(*anchor)->next_)
	{
		if (*anchor != this) continue;
		*anchor = next_;
#ifdef SGP_VIDEO_DEBUGGING
		--guiVObjectSize;
#endif
		break;
	}

	DestroyPalettes();

	if (pix_data_)     delete[] pix_data_;
	if (etrle_object_) delete[] etrle_object_;

	if (ppZStripInfo != NULL)
	{
		for (UINT32 usLoop = 0; usLoop < SubregionCount(); usLoop++)
		{
			if (ppZStripInfo[usLoop] != NULL)
			{
				delete[] ppZStripInfo[usLoop]->pbZChange;
				delete ppZStripInfo[usLoop];
			}
		}
		delete[] ppZStripInfo;
	}

#ifdef SGP_VIDEO_DEBUGGING
	if (name_) delete[] name_;
	if (code_) delete[] code_;
#endif
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
		throw std::logic_error("Tried to access invalid subregion in video object");
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
	UINT8 const* pCurrent = PixData(pETRLEObject);

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


#ifdef SGP_VIDEO_DEBUGGING
static
#endif
SGPVObject* AddStandardVideoObjectFromHImage(SGPImage* const img)
{
	return new SGPVObject(img);
}


#ifdef SGP_VIDEO_DEBUGGING
static
#endif
SGPVObject* AddStandardVideoObjectFromFile(const char* const ImageFile)
{
	AutoSGPImage hImage(CreateImage(ImageFile, IMAGE_ALLIMAGEDATA));
	return AddStandardVideoObjectFromHImage(hImage.get());
}


void BltVideoObject(SGPVSurface* const dst, SGPVObject const* const src, UINT16 const usRegionIndex, INT32 const iDestX, INT32 const iDestY)
{
	Assert(src->BPP() ==  8);
	Assert(dst->BPP() == 16);

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


void BltVideoObjectOutline(SGPVSurface* const dst, SGPVObject const* const hSrcVObject, UINT16 const usIndex, INT32 const iDestX, INT32 const iDestY, INT16 const s16BPPColor)
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
	AutoSGPVObject vo(AddVideoObjectFromFile(filename));
	BltVideoObject(dst, vo.get(), region, x, y);
}


#ifdef SGP_VIDEO_DEBUGGING

UINT32 guiVObjectSize = 0;


struct DUMPINFO
{
	UINT32 Counter;
	char Name[256];
	char Code[256];
};


static void DumpVObjectInfoIntoFile(const char* filename, BOOLEAN fAppend)
{
	if (guiVObjectSize == 0) return;

	SGPFile *f;
	if (fAppend) {
		f = FileMan::openForAppend(filename);
	} else {
		f = FileMan::openForReading(filename);
	}
	AutoSGPFile file{f};

	//Allocate enough strings and counters for each node.
	DUMPINFO* const Info = new DUMPINFO[guiVObjectSize]{};

	//Loop through the list and record every unique filename and count them
	UINT32 uiUniqueID = 0;
	for (SGPVObject const* i = gpVObjectHead; i; i = i->next_)
	{
		char const* const Name = i->name_;
		char const* const Code = i->code_;
		BOOLEAN fFound = FALSE;
		for (UINT32 i = 0; i < uiUniqueID; i++)
		{
			if (strcasecmp(Name, Info[i].Name) == 0 && strcasecmp(Code, Info[i].Code) == 0)
			{ //same string
				fFound = TRUE;
				Info[i].Counter++;
				break;
			}
		}
		if (!fFound)
		{
			strcpy(Info[uiUniqueID].Name, Name);
			strcpy(Info[uiUniqueID].Code, Code);
			Info[uiUniqueID].Counter++;
			uiUniqueID++;
		}
	}

	//Now dump the info.
	ST::string buf;
	buf += "-----------------------------------------------\n";
	buf += ST::format(ST::substitute_invalid, "{} unique vObject names exist in {} VObjects\n", uiUniqueID, guiVObjectSize);
	buf += "-----------------------------------------------\n\n";
	for (UINT32 i = 0; i < uiUniqueID; i++)
	{
		buf += ST::format(ST::substitute_invalid, "{} occurrences of {}\n{}\n\n", Info[i].Counter, Info[i].Name, Info[i].Code);
	}
	buf += "\n-----------------------------------------------\n\n";

	//Free all memory associated with this operation.
	delete[] Info;
	file->write(reinterpret_cast<const uint8_t*>(buf.c_str()), buf.size())
}


//Debug wrapper for adding vObjects
static void RecordVObject(SGPVObject* const vo, const char* Filename, UINT32 uiLineNum, const char* pSourceFile)
{
	//record the filename of the vObject (some are created via memory though)
	vo->name_ = new char[strlen(Filename) + 1]{};
	strcpy(vo->name_, Filename);

	//record the code location of the calling creating function.
	char str[256];
	sprintf(str, "%s -- line(%d)", pSourceFile, uiLineNum);
	vo->code_ = new char[strlen(str) + 1]{};
	strcpy(vo->code_, str);
}


SGPVObject* AddAndRecordVObjectFromHImage(SGPImage* const img, UINT32 uiLineNum, const char* pSourceFile)
{
	SGPVObject* const vo = AddStandardVideoObjectFromHImage(img);
	RecordVObject(vo, "<IMAGE>", uiLineNum, pSourceFile);
	return vo;
}


SGPVObject* AddAndRecordVObjectFromFile(const char* ImageFile, UINT32 uiLineNum, const char* pSourceFile)
{
	SGPVObject* const vo = AddStandardVideoObjectFromFile(ImageFile);
	RecordVObject(vo, ImageFile, uiLineNum, pSourceFile);
	return vo;
}


void PerformVideoInfoDumpIntoFile(const char* filename, BOOLEAN fAppend)
{
	DumpVObjectInfoIntoFile(filename, fAppend);
	DumpVSurfaceInfoIntoFile(filename, TRUE);
}

#endif
