#include <stdexcept>

#include "Buffer.h"
#include "HImage.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "FileMan.h"
#include "ImpTGA.h"
#include "MemMan.h"
#include "Debug.h"

#include "externalized/ContentManager.h"
#include "externalized/GameInstance.h"

static SGPImage* ReadRLEColMapImage(   HWFILE, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);
static SGPImage* ReadRLERGBImage(      HWFILE, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);
static SGPImage* ReadUncompColMapImage(HWFILE, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);
static SGPImage* ReadUncompRGBImage(   HWFILE, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);


SGPImage* LoadTGAFileToImage(char const* const filename, UINT16 const fContents)
{
	UINT8		uiImgID, uiColMap, uiType;

	AutoSGPFile hFile(GCM->openGameResForReading(filename));

	FileRead(hFile, &uiImgID,  sizeof(UINT8));
	FileRead(hFile, &uiColMap, sizeof(UINT8));
	FileRead(hFile, &uiType,   sizeof(UINT8));

	switch (uiType)
	{
		case  1: return ReadUncompColMapImage(hFile, uiImgID, uiColMap, fContents);
		case  2: return ReadUncompRGBImage(   hFile, uiImgID, uiColMap, fContents);
		case  9: return ReadRLEColMapImage(   hFile, uiImgID, uiColMap, fContents);
		case 10: return ReadRLERGBImage(      hFile, uiImgID, uiColMap, fContents);
		default: throw std::runtime_error("Unsupported TGA format");
	}
}


static SGPImage* ReadUncompColMapImage(HWFILE const hFile, UINT8 const uiImgID, UINT8 const uiColMap, UINT16 const fContents)
{
	throw std::runtime_error("TGA format 1 loading is unimplemented");
}


static SGPImage* ReadUncompRGBImage(HWFILE const f, UINT8 const uiImgID, UINT8 const uiColMap, UINT16 const contents)
{
	UINT16	uiColMapLength;
	UINT16	uiWidth;
	UINT16	uiHeight;
	UINT8		uiImagePixelSize;

	BYTE data[15];
	FileRead(f, data, sizeof(data));

	BYTE const* d = data;
	EXTR_SKIP(d, 2)              // colour map origin
	EXTR_U16(d, uiColMapLength)
	EXTR_SKIP(d, 5)              // colour map entry size, x origin, y origin
	EXTR_U16(d, uiWidth)         // XXX unaligned
	EXTR_U16(d, uiHeight)        // XXX unaligned
	EXTR_U8(d, uiImagePixelSize)
	EXTR_SKIP(d, 1)              // image descriptor
	Assert(d == endof(data));

	// skip the id
	FileSeek(f, uiImgID, FILE_SEEK_FROM_CURRENT);

	// skip the colour map
	if (uiColMap != 0)
	{
		FileSeek(f, uiColMapLength * (uiImagePixelSize / 8), FILE_SEEK_FROM_CURRENT);
	}

	AutoSGPImage img(new SGPImage(uiWidth, uiHeight, uiImagePixelSize));

	if (contents & IMAGE_BITMAPDATA)
	{
		if (uiImagePixelSize == 16)
		{
			UINT16* const img_data = (UINT16*)(UINT8*)img->pImageData.Allocate(uiWidth * uiHeight * 2);
			// Data is stored top-bottom - reverse for SGPImage format
			for (size_t y = uiHeight; y != 0;)
			{
				FileRead(f, &img_data[uiWidth * --y], uiWidth * 2);
			}
		}
		else if (uiImagePixelSize == 24)
		{
			UINT8* const img_data = img->pImageData.Allocate(uiWidth * uiHeight * 3);
			for (size_t y = uiHeight; y != 0;)
			{
				UINT8* const line = &img_data[uiWidth * 3 * --y];
				for (UINT32 x = 0 ; x < uiWidth; ++x)
				{
					UINT8 bgr[3];
					FileRead(f, bgr, sizeof(bgr));
					line[x * 3    ] = bgr[2];
					line[x * 3 + 1] = bgr[1];
					line[x * 3 + 2] = bgr[0];
				}
			}
		}
		else
		{
			throw std::runtime_error("Failed to load TGA with unsupported colour depth");
		}
		img->fFlags |= IMAGE_BITMAPDATA;
	}

	return img.Release();
}


static SGPImage* ReadRLEColMapImage(HWFILE const hFile, UINT8 const uiImgID, UINT8 const uiColMap, UINT16 const fContents)
{
	throw std::runtime_error("TGA format 9 loading is unimplemented");
}


static SGPImage* ReadRLERGBImage(HWFILE const hFile, UINT8 const uiImgID, UINT8 const uiColMap, UINT16 const fContents)
{
	throw std::runtime_error("TGA format 10 loading is unimplemented");
}
