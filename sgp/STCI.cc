#include <stdexcept>

#include "Buffer.h"
#include "MemMan.h"
#include "FileMan.h"
#include "ImgFmt.h"
#include "HImage.h"
#include "Debug.h"
#include "PODObj.h"
#include "STCI.h"


static SGPImage* STCILoadIndexed(UINT16 contents, HWFILE, STCIHeader const*);
static SGPImage* STCILoadRGB(    UINT16 contents, HWFILE, STCIHeader const*);


SGPImage* LoadSTCIFileToImage(char const* const filename, UINT16 const fContents)
{
	AutoSGPFile f(FileOpen(filename, FILE_ACCESS_READ));

	STCIHeader header;
	FileRead(f, &header, sizeof(header));
	if (memcmp(header.cID, STCI_ID_STRING, STCI_ID_LEN) != 0)
	{
		throw std::runtime_error("STCI file has invalid header");
	}

	if (header.fFlags & STCI_ZLIB_COMPRESSED)
	{
		throw std::runtime_error("Cannot handle zlib compressed STCI files");
	}

	// Determine from the header the data stored in the file. and run the appropriate loader
	SGPImage* const image =
		header.fFlags & STCI_RGB     ? STCILoadRGB(    fContents, f, &header) :
		header.fFlags & STCI_INDEXED ? STCILoadIndexed(fContents, f, &header) :
		/* Unsupported type of data, or the right flags weren't set! */
		throw std::runtime_error("Unknown data organization in STCI file.");

	image->usWidth    = header.usWidth;
	image->usHeight   = header.usHeight;
	image->ubBitDepth = header.ubDepth;
	return image;
}


static SGPImage* STCILoadRGB(UINT16 const contents, HWFILE const f, STCIHeader const* const header)
{
	if (contents & IMAGE_PALETTE && (contents & IMAGE_ALLIMAGEDATA) != IMAGE_ALLIMAGEDATA)
	{ // RGB doesn't have a palette!
		throw std::logic_error("Invalid combination of content load flags");
	}

	SGP::PODObj<SGPImage> img;
	if (contents & IMAGE_BITMAPDATA)
	{
		// Allocate memory for the image data and read it in
		img->pImageData = MALLOCN(UINT8, header->uiStoredSize);
		try
		{
			FileRead(f, img->pImageData, header->uiStoredSize);
		}
		catch (...)
		{
			MemFree(img->pImageData);
			throw;
		}

		img->fFlags |= IMAGE_BITMAPDATA;

		if (header->ubDepth == 16)
		{
			// ASSUMPTION: file data is 565 R,G,B
			if (gusRedMask   != (UINT16)header->RGB.uiRedMask   ||
					gusGreenMask != (UINT16)header->RGB.uiGreenMask ||
					gusBlueMask  != (UINT16)header->RGB.uiBlueMask)
			{
				// colour distribution of the file is different from hardware!  We have to change it!
				DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Converting to current RGB distribution!");
				// Convert the image to the current hardware's specifications
				size_t const size = header->usWidth * header->usHeight;
				if (gusRedMask == 0x7C00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F)
				{
					ConvertRGBDistribution565To555(img->p16BPPData, size);
				}
				else if (gusRedMask == 0xFC00 && gusGreenMask == 0x03E0 && gusBlueMask == 0x001F)
				{
					ConvertRGBDistribution565To655(img->p16BPPData, size);
				}
				else if (gusRedMask == 0xF800 && gusGreenMask == 0x07C0 && gusBlueMask == 0x003F)
				{
					ConvertRGBDistribution565To556(img->p16BPPData, size);
				}
				else
				{
					// take the long route
					ConvertRGBDistribution565ToAny(img->p16BPPData, size);
				}
			}
		}
	}
	return img.Release();
}


static SGPImage* STCILoadIndexed(UINT16 const contents, HWFILE const f, STCIHeader const* const header)
{
	SGP::PODObj<SGPImage>        img;
	SGP::Buffer<SGPPaletteEntry> palette;
	if (contents & IMAGE_PALETTE)
	{ // Allocate memory for reading in the palette
		if (header->Indexed.uiNumberOfColours != 256)
		{
			throw std::runtime_error("Palettized image has bad palette size.");
		}

		SGP::Buffer<STCIPaletteElement> pSTCIPalette(256);

		// Read in the palette
		FileRead(f, pSTCIPalette, sizeof(*pSTCIPalette) * 256);

		palette.Allocate(256);
		for (size_t i = 0; i < 256; i++)
		{
			palette[i].r      = pSTCIPalette[i].ubRed;
			palette[i].g      = pSTCIPalette[i].ubGreen;
			palette[i].b      = pSTCIPalette[i].ubBlue;
			palette[i].unused = 0;
		}

		img->fFlags |= IMAGE_PALETTE;
	}
	else if (contents & (IMAGE_BITMAPDATA | IMAGE_APPDATA))
	{ // seek past the palette
		FileSeek(f, sizeof(STCIPaletteElement) * header->Indexed.uiNumberOfColours, FILE_SEEK_FROM_CURRENT);
	}

	SGP::Buffer<ETRLEObject> etrle_objects;
	SGP::Buffer<UINT8>       image_data;
	if (contents & IMAGE_BITMAPDATA)
	{
		if (header->fFlags & STCI_ETRLE_COMPRESSED)
		{
			// load data for the subimage (object) structures
			Assert(sizeof(ETRLEObject) == sizeof(STCISubImage));

			UINT16 const n_subimages = header->Indexed.usNumberOfSubImages;
			img->usNumberOfObjects = n_subimages;

			etrle_objects.Allocate(n_subimages);
			FileRead(f, etrle_objects, sizeof(*etrle_objects) * n_subimages);

			img->uiSizePixData  = header->uiStoredSize;
			img->fFlags        |= IMAGE_TRLECOMPRESSED;
		}

		image_data.Allocate(header->uiStoredSize);
		FileRead(f, image_data, header->uiStoredSize);

		img->fFlags |= IMAGE_BITMAPDATA;
	}
	else if (contents & IMAGE_APPDATA) // then there's a point in seeking ahead
	{
		FileSeek(f, header->uiStoredSize, FILE_SEEK_FROM_CURRENT);
	}

	SGP::Buffer<UINT8> app_data;
	if (contents & IMAGE_APPDATA && header->uiAppDataSize > 0)
	{
		// load application-specific data
		app_data.Allocate(header->uiAppDataSize);
		FileRead(f, app_data, header->uiAppDataSize);

		img->uiAppDataSize  = header->uiAppDataSize;
		img->fFlags        |= IMAGE_APPDATA;
	}
	else
	{
		img->uiAppDataSize = 0;
	}

	img->pAppData     = app_data.Release();
	img->pImageData   = image_data.Release();
	img->pETRLEObject = etrle_objects.Release();
	img->pPalette     = palette.Release();
	return img.Release();
}
