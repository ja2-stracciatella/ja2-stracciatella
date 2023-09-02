#include <stdexcept>

#include "Buffer.h"
#include "ImgFmt.h"
#include "HImage.h"
#include "Debug.h"
#include "SGPFile.h"
#include "STCI.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

static SGPImage* STCILoadIndexed(UINT16 contents, HWFILE, STCIHeader const*);
static SGPImage* STCILoadRGB(    UINT16 contents, HWFILE, STCIHeader const*);


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
		header.fFlags & STCI_RGB     ? STCILoadRGB(    fContents, f, &header) :
		header.fFlags & STCI_INDEXED ? STCILoadIndexed(fContents, f, &header) :
		/* Unsupported type of data, or the right flags weren't set! */
		throw std::runtime_error("Unknown data organization in STCI file.");
}


static SGPImage* STCILoadRGB(UINT16 const contents, HWFILE const f, STCIHeader const* const header)
{
	if (contents & IMAGE_PALETTE && (contents & IMAGE_ALLIMAGEDATA) != IMAGE_ALLIMAGEDATA)
	{ // RGB doesn't have a palette!
		throw std::logic_error("Invalid combination of content load flags");
	}

	AutoSGPImage img(new SGPImage(header->usWidth, header->usHeight, header->ubDepth));
	if (contents & IMAGE_BITMAPDATA)
	{
		// Allocate memory for the image data and read it in
		UINT8* const img_data = img->pImageData.Allocate(header->uiStoredSize);
		f->read(img_data, header->uiStoredSize);

		img->fFlags |= IMAGE_BITMAPDATA;

		if (header->ubDepth == 16)
		{
			// ASSUMPTION: file data is 565 R,G,B
			if (gusRedMask   != (UINT16)header->RGB.uiRedMask   ||
					gusGreenMask != (UINT16)header->RGB.uiGreenMask ||
					gusBlueMask  != (UINT16)header->RGB.uiBlueMask)
			{
				// colour distribution of the file is different from hardware!  We have to change it!
				SLOGD("Converting to current RGB distribution!");
				// Convert the image to the current hardware's specifications
				UINT32  const size = header->usWidth * header->usHeight;
				UINT16* const data = (UINT16*)(UINT8*)img->pImageData;
				if (gusRedMask == 0x7C00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F)
				{
					ConvertRGBDistribution565To555(data, size);
				}
				else if (gusRedMask == 0xFC00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F)
				{
					ConvertRGBDistribution565To655(data, size);
				}
				else if (gusRedMask == 0xF800 && gusGreenMask == 0x07C0 && gusBlueMask == 0x003F)
				{
					ConvertRGBDistribution565To556(data, size);
				}
				else
				{
					// take the long route
					ConvertRGBDistribution565ToAny(data, size);
				}
			}
		}
	}
	return img.release();
}


static SGPImage* STCILoadIndexed(UINT16 const contents, HWFILE const f, STCIHeader const* const header)
{
	AutoSGPImage img(new SGPImage(header->usWidth, header->usHeight, header->ubDepth));
	if (contents & IMAGE_PALETTE)
	{ // Allocate memory for reading in the palette
		if (header->Indexed.uiNumberOfColours != 256)
		{
			throw std::runtime_error("Palettized image has bad palette size.");
		}

		SGP::Buffer<STCIPaletteElement> pSTCIPalette(256);

		// Read in the palette
		f->read(pSTCIPalette, sizeof(*pSTCIPalette) * 256);

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
		f->seek(sizeof(STCIPaletteElement) * header->Indexed.uiNumberOfColours, FILE_SEEK_FROM_CURRENT);
	}

	if (contents & IMAGE_BITMAPDATA)
	{
		if (header->fFlags & STCI_ETRLE_COMPRESSED)
		{
			// load data for the subimage (object) structures
			Assert(sizeof(ETRLEObject) == sizeof(STCISubImage));

			UINT16 const n_subimages = header->Indexed.usNumberOfSubImages;
			img->usNumberOfObjects = n_subimages;

			ETRLEObject* const etrle_objects = img->pETRLEObject.Allocate(n_subimages);
			f->read(etrle_objects, sizeof(*etrle_objects) * n_subimages);

			img->uiSizePixData  = header->uiStoredSize;
			img->fFlags        |= IMAGE_TRLECOMPRESSED;
		}

		UINT8* const image_data = img->pImageData.Allocate(header->uiStoredSize);
		f->read(image_data, header->uiStoredSize);

		img->fFlags |= IMAGE_BITMAPDATA;
	}
	else if (contents & IMAGE_APPDATA) // then there's a point in seeking ahead
	{
		f->seek(header->uiStoredSize, FILE_SEEK_FROM_CURRENT);
	}

	if (contents & IMAGE_APPDATA && header->uiAppDataSize > 0)
	{
		// load application-specific data
		UINT8* const app_data = img->pAppData.Allocate(header->uiAppDataSize);
		f->read(app_data, header->uiAppDataSize);

		img->uiAppDataSize  = header->uiAppDataSize;
		img->fFlags        |= IMAGE_APPDATA;
	}
	else
	{
		img->uiAppDataSize = 0;
	}

	return img.release();
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(STCI, asserts)
{
	EXPECT_EQ(sizeof(STCIHeader), 64u);
	EXPECT_EQ(sizeof(STCISubImage), 16u);
	EXPECT_EQ(sizeof(STCIPaletteElement), 3u);
}

#endif
