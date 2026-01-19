#include <stdexcept>

#include "Exceptions.h"
#include "ImgFmt.h"
#include "HImage.h"
#include "SGPFile.h"
#include "STCI.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "SDL.h"
#include <array>

namespace {
SGPImage* LoadIndexed(UINT16 contents, HWFILE, STCIHeader const&);
SGPImage* LoadRGB(    UINT16 contents, HWFILE, STCIHeader const&);
}

SGPImage* LoadSTCIFileToImage(const ST::string& filename, UINT16 const fContents)
{
	AutoSGPFile f(GCM->openGameResForReading(filename));

	STCIHeader header;
	f->read(&header, sizeof(header));
	if (memcmp(header.cID, STCI_ID_STRING, STCI_ID_LEN) != 0)
	{
		throw std::runtime_error("STCI file has invalid header");
	}

	if (header.fFlags & STCI_ZLIB_COMPRESSED)
	{
		throw std::runtime_error("Cannot handle zlib compressed STCI files");
	}

	// Determine from the header the data stored in the file. and run the appropriate loader
	return
		header.fFlags & STCI_RGB     ? LoadRGB(    fContents, f, header) :
		header.fFlags & STCI_INDEXED ? LoadIndexed(fContents, f, header) :
		/* Unsupported type of data, or the right flags weren't set! */
		throw std::runtime_error("Unknown data organization in STCI file.");
}

namespace {
void ReadImageData(SGPImage & img, SGPFile *file, size_t dataBytes)
{
	// Allocate memory for the image data and read it in
	file->read(img.pImageData.Allocate(dataBytes), dataBytes);
	img.fFlags |= IMAGE_BITMAPDATA;
}


SGPImage* LoadRGB(UINT16 const contents, HWFILE const f, STCIHeader const& header)
{
	if (contents & IMAGE_PALETTE && (contents & IMAGE_ALLIMAGEDATA) != IMAGE_ALLIMAGEDATA)
	{ // RGB doesn't have a palette!
		throw std::logic_error("Invalid combination of content load flags");
	}

	AutoSGPImage img(new SGPImage(header.usWidth, header.usHeight, header.ubDepth));
	if (contents & IMAGE_BITMAPDATA)
	{
		ReadImageData(*img, f, header.uiStoredSize);

		if (header.ubDepth == 16)
		{
			// ASSUMPTION: file data is 565 R,G,B
			auto const& masks = header.RGB;
			auto pixelFormat = SDL_MasksToPixelFormatEnum(16, masks.uiRedMask,
				masks.uiGreenMask, masks.uiBlueMask, masks.uiAlphaMask);
			if (pixelFormat != SDL_PIXELFORMAT_RGB565)
			{
				throw DataError{ ST::format(
					"{} is in a 16-bit pixel format other than RGB565", f->getName()) };
			}
		}
	}
	return img.release();
}


SGPImage* LoadIndexed(UINT16 const contents, HWFILE const f, STCIHeader const& header)
{
	AutoSGPImage img(new SGPImage(header.usWidth, header.usHeight, header.ubDepth));
	if (contents & IMAGE_PALETTE)
	{ // Allocate memory for reading in the palette
		if (header.Indexed.uiNumberOfColours != 256)
		{
			throw std::runtime_error("Palettized image has bad palette size.");
		}

		std::array<STCIPaletteElement, 256> pSTCIPalette;

		// Read in the palette
		f->read(pSTCIPalette.data(), sizeof(pSTCIPalette));

		SGPPaletteEntry* const palette = img->pPalette.Allocate(256);
		for (size_t i = 0; i < 256; i++)
		{
			palette[i].r      = pSTCIPalette[i].ubRed;
			palette[i].g      = pSTCIPalette[i].ubGreen;
			palette[i].b      = pSTCIPalette[i].ubBlue;
			palette[i].a      = 0;
		}

		img->fFlags |= IMAGE_PALETTE;
	}
	else if (contents & (IMAGE_BITMAPDATA | IMAGE_APPDATA))
	{ // seek past the palette
		f->seek(sizeof(STCIPaletteElement) * header.Indexed.uiNumberOfColours, FILE_SEEK_FROM_CURRENT);
	}

	if (contents & IMAGE_BITMAPDATA)
	{
		if (header.fFlags & STCI_ETRLE_COMPRESSED)
		{
			// load data for the subimage (object) structures
			static_assert(sizeof(ETRLEObject) == sizeof(STCISubImage));

			UINT16 const n_subimages = header.Indexed.usNumberOfSubImages;
			img->usNumberOfObjects = n_subimages;

			ETRLEObject* const etrle_objects = img->pETRLEObject.Allocate(n_subimages);
			f->read(etrle_objects, sizeof(*etrle_objects) * n_subimages);

			img->uiSizePixData  = header.uiStoredSize;
			img->fFlags        |= IMAGE_TRLECOMPRESSED;
		}

		ReadImageData(*img, f, header.uiStoredSize);
	}
	else if (contents & IMAGE_APPDATA) // then there's a point in seeking ahead
	{
		f->seek(header.uiStoredSize, FILE_SEEK_FROM_CURRENT);
	}

	if (contents & IMAGE_APPDATA && header.uiAppDataSize > 0)
	{
		// load application-specific data
		UINT8* const app_data = img->pAppData.Allocate(header.uiAppDataSize);
		f->read(app_data, header.uiAppDataSize);

		img->uiAppDataSize  = header.uiAppDataSize;
		img->fFlags        |= IMAGE_APPDATA;
	}
	else
	{
		img->uiAppDataSize = 0;
	}

	return img.release();
}
}

static_assert(sizeof(STCIHeader) == 64u);
static_assert(sizeof(STCISubImage) == 16u);
static_assert(sizeof(STCIPaletteElement) == 3u);
