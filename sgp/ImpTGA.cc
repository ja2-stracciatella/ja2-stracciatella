#include "Buffer.h"
#include "HImage.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "FileMan.h"
#include "ImpTGA.h"
#include "MemMan.h"
#include "WCheck.h"
#include "Debug.h"


static BOOLEAN ReadUncompColMapImage(HIMAGE hImage, HWFILE hFile, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);
static BOOLEAN ReadUncompRGBImage(HIMAGE hImage, HWFILE hFile, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);
static BOOLEAN ReadRLEColMapImage(HIMAGE hImage, HWFILE hFile, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);
static BOOLEAN ReadRLERGBImage(HIMAGE hImage, HWFILE hFile, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents);


BOOLEAN LoadTGAFileToImage( HIMAGE hImage, UINT16 fContents )
try
{
	UINT8		uiImgID, uiColMap, uiType;

	Assert( hImage != NULL );

	AutoSGPFile hFile(FileOpen(hImage->ImageFile, FILE_ACCESS_READ));

	FileRead(hFile, &uiImgID,  sizeof(UINT8));
	FileRead(hFile, &uiColMap, sizeof(UINT8));
	FileRead(hFile, &uiType,   sizeof(UINT8));

	BOOLEAN fReturnVal = FALSE;
	switch( uiType )
	{
	case 1:
		fReturnVal = ReadUncompColMapImage( hImage, hFile, uiImgID, uiColMap, fContents );
		break;
	case 2:
		fReturnVal = ReadUncompRGBImage( hImage, hFile, uiImgID, uiColMap, fContents );
		break;
	case 9:
		fReturnVal = ReadRLEColMapImage( hImage, hFile, uiImgID, uiColMap, fContents );
		break;
	case 10:
		fReturnVal = ReadRLERGBImage( hImage, hFile, uiImgID, uiColMap, fContents );
		break;
	default:
		break;
	}

	return( fReturnVal );
}
catch (...) { return FALSE; }


static BOOLEAN ReadUncompColMapImage(HIMAGE hImage, HWFILE hFile, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents)
{
	return FALSE;
}


static BOOLEAN ReadUncompRGBImage(SGPImage* const img, HWFILE const f, UINT8 const uiImgID, UINT8 const uiColMap, UINT16 const contents)
try
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

	img->usWidth    = uiWidth;
	img->usHeight   = uiHeight;
	img->ubBitDepth = uiImagePixelSize;

	if (contents & IMAGE_BITMAPDATA)
	{
		if (uiImagePixelSize == 16)
		{
			SGP::Buffer<UINT16> img_data(uiWidth * uiHeight);

			// Data is stored top-bottom - reverse for SGP HIMAGE format
			for (size_t y = uiHeight; y != 0;)
			{
				FileRead(f, &img_data[uiWidth * --y], uiWidth * 2);;
			}

			img->p16BPPData = img_data.Release();
		}
		else if (uiImagePixelSize == 24)
		{
			SGP::Buffer<UINT8> img_data(uiWidth * uiHeight * 3);

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

			img->p8BPPData = img_data.Release();
		}
		else
		{
			return FALSE;
		}
		img->fFlags |= IMAGE_BITMAPDATA;
	}

	return TRUE;
}
catch (...) { return FALSE; }


static BOOLEAN ReadRLEColMapImage(HIMAGE hImage, HWFILE hFile, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents)
{
	return FALSE;
}


static BOOLEAN ReadRLERGBImage(HIMAGE hImage, HWFILE hFile, UINT8 uiImgID, UINT8 uiColMap, UINT16 fContents)
{
	return FALSE;
}
