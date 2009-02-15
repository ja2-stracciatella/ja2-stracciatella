#include <stdexcept>

#include "Buffer.h"
#include "HImage.h"
#include "PCX.h"
#include "MemMan.h"
#include "FileMan.h"
#include "PODObj.h"


#define PCX_NORMAL         1
#define PCX_RLE            2


struct PcxHeader
{
	UINT8  ubManufacturer;
	UINT8  ubVersion;
	UINT8  ubEncoding;
	UINT8  ubBitsPerPixel;
	UINT16 usLeft;
	UINT16 usTop;
	UINT16 usRight;
	UINT16 usBottom;
	UINT16 usHorRez;
	UINT16 usVerRez;
	UINT8  ubEgaPalette[48];
	UINT8  ubReserved;
	UINT8  ubColorPlanes;
	UINT16 usBytesPerLine;
	UINT16 usPaletteType;
	UINT8  ubFiller[58];
};
CASSERT(sizeof(PcxHeader) == 128)


struct PcxObject
{
	UINT8* pPcxBuffer;
	UINT8  ubPalette[768];
	UINT16 usWidth, usHeight;
};


static void       BlitPcxToBuffer(PcxObject*, UINT8* pBuffer, UINT16 usBufferWidth, UINT16 usBufferHeight);
static PcxObject* LoadPcx(const char* filename);


SGPImage* LoadPCXFileToImage(char const* const filename, UINT16 const contents)
{
	PcxObject* const pcx_obj = LoadPcx(filename);

	AutoSGPImage img(new SGPImage(pcx_obj->usWidth, pcx_obj->usHeight, 8));
	// Set some header information
	img->fFlags |= contents;

	// Read and allocate bitmap block if requested
	if (contents & IMAGE_BITMAPDATA)
	{
		UINT8* const img_data = img->pImageData.Allocate(img->usWidth * img->usHeight);
		BlitPcxToBuffer(pcx_obj, img_data, img->usWidth, img->usHeight);
	}

	if (contents & IMAGE_PALETTE)
	{
		SGPPaletteEntry* const dst = img->pPalette.Allocate(256);
		UINT8 const*     const src = pcx_obj->ubPalette;
		for (UINT16 Index = 0; Index < 256; Index++)
		{
			dst[Index].r      = src[Index * 3 + 0];
			dst[Index].g      = src[Index * 3 + 1];
			dst[Index].b      = src[Index * 3 + 2];
			dst[Index].unused = 0;
		}
		img->pui16BPPPalette = Create16BPPPalette(dst);
	}

	MemFree(pcx_obj->pPcxBuffer);
	MemFree(pcx_obj);
	return img.Release();
}


static PcxObject* LoadPcx(const char* const filename)
{
	AutoSGPFile f(FileOpen(filename, FILE_ACCESS_READ));

	PcxHeader header;
	FileRead(f, &header, sizeof(header));
	if (header.ubManufacturer != 10 ||
			header.ubEncoding     !=  1)
	{
		throw std::runtime_error("PCX file has invalid header");
	}

	const UINT32 file_size   = FileGetSize(f);
	const UINT32 buffer_size = file_size - sizeof(PcxHeader) - 768;

	SGP::Buffer<UINT8> pcx_buffer(buffer_size);
	FileRead(f, pcx_buffer, buffer_size);

	SGP::PODObj<PcxObject> pcx_obj;
	FileRead(f, pcx_obj->ubPalette, sizeof(pcx_obj->ubPalette));

	pcx_obj->pPcxBuffer   = pcx_buffer.Release();
	pcx_obj->usWidth      = header.usRight  - header.usLeft + 1;
	pcx_obj->usHeight     = header.usBottom - header.usTop  + 1;
	return pcx_obj.Release();
}


static void BlitPcxToBuffer(PcxObject* const pCurrentPcxObject, UINT8* const pBuffer, UINT16 const usBufferWidth, UINT16 const usBufferHeight)
{
  UINT8     *pPcxBuffer;
  UINT8      ubRepCount;
  UINT16     usMaxX, usMaxY;
  UINT32     uiImageSize;
  UINT8      ubCurrentByte = 0;
  UINT8      ubMode;
  UINT16     usCurrentX, usCurrentY;
  UINT32     uiOffset, uiIndex;
  UINT32     uiNextLineOffset, uiStartOffset, uiCurrentOffset;

  pPcxBuffer = pCurrentPcxObject->pPcxBuffer;

  if (pCurrentPcxObject->usWidth == usBufferWidth && pCurrentPcxObject->usHeight == usBufferHeight)
  { // Pre-compute PCX blitting aspects.
    uiImageSize = usBufferWidth * usBufferHeight;
    ubMode      = PCX_NORMAL;
    uiOffset    = 0;
    ubRepCount  = 0;

		for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++)
		{
			if (ubMode == PCX_NORMAL)
			{
				ubCurrentByte = *(pPcxBuffer + uiOffset++);
				if (ubCurrentByte > 0x0BF)
				{
					ubRepCount = ubCurrentByte & 0x03F;
					ubCurrentByte = *(pPcxBuffer + uiOffset++);
					if (--ubRepCount > 0)
					{
						ubMode = PCX_RLE;
					}
				}
			}
			else
			{
				if (--ubRepCount == 0)
				{ ubMode = PCX_NORMAL;
				}
			}
			*(pBuffer + uiIndex) = ubCurrentByte;
		}
  } else
  { // Pre-compute PCX blitting aspects.
    if (pCurrentPcxObject->usWidth >= usBufferWidth)
    {
      usMaxX = usBufferWidth - 1;
    }
    else
    {
      usMaxX = pCurrentPcxObject->usWidth;
    }

    if (pCurrentPcxObject->usHeight >= usBufferHeight)
    {
      uiImageSize = pCurrentPcxObject->usWidth * usBufferHeight;
      usMaxY = usBufferHeight - 1;
    }
    else
    { uiImageSize = pCurrentPcxObject->usWidth * pCurrentPcxObject->usHeight;
      usMaxY = pCurrentPcxObject->usHeight;
    }

    ubMode     = PCX_NORMAL;
    uiOffset   = 0;
    ubRepCount = 0;
    usCurrentX = 0;
    usCurrentY = 0;

		uiStartOffset = (usCurrentY*usBufferWidth) + usCurrentX;
		uiNextLineOffset = uiStartOffset + usBufferWidth;
		uiCurrentOffset = uiStartOffset;

		for (uiIndex = 0; uiIndex < uiImageSize; uiIndex++)
		{

			if (ubMode == PCX_NORMAL)
			{
				ubCurrentByte = *(pPcxBuffer + uiOffset++);
				if (ubCurrentByte > 0x0BF)
				{
					ubRepCount = ubCurrentByte & 0x03F;
					ubCurrentByte = *(pPcxBuffer + uiOffset++);
					if (--ubRepCount > 0)
					{
						ubMode = PCX_RLE;
					}
				}
			}
			else
			{
				if (--ubRepCount == 0)
				{
					ubMode = PCX_NORMAL;
				}
			}

			if (usCurrentX < usMaxX)
			{ // We are within the visible bounds so we write the byte to buffer
				*(pBuffer + uiCurrentOffset) = ubCurrentByte;
				uiCurrentOffset++;
				usCurrentX++;
			}
			else
			{ if ((uiCurrentOffset + 1)< uiNextLineOffset)
				{ // Increment the uiCurrentOffset
					uiCurrentOffset++;
				}
				else
				{ // Go to next line
					usCurrentX = 0;
					usCurrentY++;
					if (usCurrentY > usMaxY)
					{
						break;
					}
					uiStartOffset = (usCurrentY*usBufferWidth) + usCurrentX;
					uiNextLineOffset = uiStartOffset + usBufferWidth;
					uiCurrentOffset = uiStartOffset;
				}
			}
		}
  }
}
