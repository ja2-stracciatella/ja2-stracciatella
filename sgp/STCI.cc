#include "Buffer.h"
#include "MemMan.h"
#include "FileMan.h"
#include "ImgFmt.h"
#include "HImage.h"
#include "Debug.h"
#include "STCI.h"
#include "WCheck.h"


static BOOLEAN STCILoadRGB(    SGPImage*, UINT16 contents, HWFILE, STCIHeader const*);
static BOOLEAN STCILoadIndexed(SGPImage*, UINT16 contents, HWFILE, STCIHeader const*);


BOOLEAN LoadSTCIFileToImage(const HIMAGE image, const UINT16 fContents)
{
	Assert(image != NULL);

	AutoSGPFile f(FileOpen(image->ImageFile, FILE_ACCESS_READ));
	if (!f) return FALSE;

	STCIHeader header;
	if (!FileRead(f, &header, sizeof(header)) || memcmp(header.cID, STCI_ID_STRING, STCI_ID_LEN) != 0)
	{
		DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem reading STCI header.");
		return FALSE;
	}

	// Determine from the header the data stored in the file. and run the appropriate loader
	if (header.fFlags & STCI_RGB)
	{
		if (!STCILoadRGB(image, fContents, f, &header))
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading RGB image.");
			return FALSE;
		}
	}
	else if (header.fFlags & STCI_INDEXED)
	{
		if (!STCILoadIndexed(image, fContents, f, &header))
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading palettized image.");
			return FALSE;
		}
	}
	else
	{
		// Unsupported type of data, or the right flags weren't set!
		DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Unknown data organization in STCI file.");
		return FALSE;
	}

	if (header.fFlags & STCI_ZLIB_COMPRESSED) image->fFlags |= IMAGE_COMPRESSED;
	image->usWidth    = header.usWidth;
	image->usHeight   = header.usHeight;
	image->ubBitDepth = header.ubDepth;
	return TRUE;
}


static BOOLEAN STCILoadRGB(SGPImage* const img, UINT16 const contents, HWFILE const f, STCIHeader const* const header)
{
	if (contents & IMAGE_PALETTE && (contents & IMAGE_ALLIMAGEDATA) != IMAGE_ALLIMAGEDATA)
	{ // RGB doesn't have a palette!
		return FALSE;
	}

	if (contents & IMAGE_BITMAPDATA)
	{
		// Allocate memory for the image data and read it in
		img->pImageData = MALLOCN(UINT8, header->uiStoredSize);
		if (img->pImageData == NULL) return FALSE;
		if (!FileRead(f, img->pImageData, header->uiStoredSize))
		{
			MemFree(img->pImageData);
			return FALSE;
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

	return TRUE;
}


static SGPPaletteEntry* STCISetPalette(STCIPaletteElement const*);


static BOOLEAN STCILoadIndexed(SGPImage* const img, UINT16 const contents, HWFILE const f, STCIHeader const* const header)
{
	if (contents & IMAGE_PALETTE)
	{ // Allocate memory for reading in the palette
		if (header->Indexed.uiNumberOfColours != 256)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Palettized image has bad palette size.");
			return FALSE;
		}

		SGP::Buffer<STCIPaletteElement> pSTCIPalette(256);
		if (pSTCIPalette == NULL)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
			return FALSE;
		}

		// Read in the palette
		if (!FileRead(f, pSTCIPalette, sizeof(*pSTCIPalette) * 256))
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem loading palette!");
			return FALSE;
		}

		img->pPalette = STCISetPalette(pSTCIPalette);
		if (!img->pPalette)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem setting SGPImage-format palette!");
			return FALSE;
		}

		img->fFlags |= IMAGE_PALETTE;
	}
	else if (contents & (IMAGE_BITMAPDATA | IMAGE_APPDATA))
	{ // seek past the palette
		if (!FileSeek(f, sizeof(STCIPaletteElement) * header->Indexed.uiNumberOfColours, FILE_SEEK_FROM_CURRENT))
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem seeking past palette!");
			return FALSE;
		}
	}

	if (contents & IMAGE_BITMAPDATA)
	{
		if (header->fFlags & STCI_ETRLE_COMPRESSED)
		{
			// load data for the subimage (object) structures
			Assert(sizeof(ETRLEObject) == sizeof(STCISubImage));

			UINT16 const n_subimages = header->Indexed.usNumberOfSubImages;
			img->usNumberOfObjects = n_subimages;

			img->pETRLEObject = MALLOCN(ETRLEObject, n_subimages);
			if (img->pETRLEObject == NULL)
			{
				DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
				goto fail;
			}

			if (!FileRead(f, img->pETRLEObject, sizeof(*img->pETRLEObject) * n_subimages))
			{
				DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading subimage structures!");
				goto fail;
			}

			img->uiSizePixData  = header->uiStoredSize;
			img->fFlags        |= IMAGE_TRLECOMPRESSED;
		}

		// allocate memory for and read in the image data
		img->pImageData = MALLOCN(UINT8, header->uiStoredSize);
		if (img->pImageData == NULL)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
			goto fail;
		}

		if (!FileRead(f, img->pImageData, header->uiStoredSize))
		{ // Problem reading in the image data!
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading image data!");
			goto fail;
		}

		img->fFlags |= IMAGE_BITMAPDATA;
	}
	else if (contents & IMAGE_APPDATA) // then there's a point in seeking ahead
	{
		if (!FileSeek(f, header->uiStoredSize, FILE_SEEK_FROM_CURRENT))
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Problem seeking past image data!");
			goto fail;
		}
	}

	if (contents & IMAGE_APPDATA && header->uiAppDataSize > 0)
	{
		// load application-specific data
		img->pAppData = MALLOCN(UINT8, header->uiAppDataSize);
		if (img->pAppData == NULL)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Out of memory!");
			goto fail;
		}

		if (!FileRead(f, img->pAppData, header->uiAppDataSize))
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Error loading application-specific data!");
			goto fail;
		}

		img->uiAppDataSize  = header->uiAppDataSize;;
		img->fFlags        |= IMAGE_APPDATA;
	}
	else
	{
		img->pAppData      = NULL;
		img->uiAppDataSize = 0;
	}

	return TRUE;

fail:
	if (img->pAppData)     MemFree(img->pAppData);
	if (img->pImageData)   MemFree(img->pImageData);
	if (img->pETRLEObject) MemFree(img->pETRLEObject);
	if (img->pPalette)     MemFree(img->pPalette);
	return FALSE;
}


static SGPPaletteEntry* STCISetPalette(STCIPaletteElement const* const stci_palette)
{
	SGPPaletteEntry* const pal = MALLOCN(SGPPaletteEntry, 256);
	if (!pal) return 0;

  // Initialize the proper palette entries
  for (size_t i = 0; i < 256; i++)
  {
		pal[i].peRed   = stci_palette[i].ubRed;
		pal[i].peGreen = stci_palette[i].ubGreen;
		pal[i].peBlue  = stci_palette[i].ubBlue;
		pal[i].peFlags = 0;
  }
  return pal;
}
