#include <stdexcept>

#include "Buffer.h"
#include "HImage.h"
#include "PCX.h"
#include "MemMan.h"
#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"

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


static void BlitPcxToBuffer(UINT8 const* src, UINT8* dst, UINT16 w, UINT16 h);


SGPImage* LoadPCXFileToImage(char const* const filename, UINT16 const contents)
{
	AutoSGPFile f(GCM->openGameResForReading(filename));

	PcxHeader header;
	FileRead(f, &header, sizeof(header));
	if (header.ubManufacturer != 10 || header.ubEncoding != 1)
	{
		throw std::runtime_error("PCX file has invalid header");
	}

	UINT32 const file_size   = FileGetSize(f);
	UINT32 const buffer_size = file_size - sizeof(PcxHeader) - 768;

	SGP::Buffer<UINT8> pcx_buffer(buffer_size);
	FileRead(f, pcx_buffer, buffer_size);

	UINT8 palette[768];
	FileRead(f, palette, sizeof(palette));

	UINT16 const w = header.usRight  - header.usLeft + 1;
	UINT16 const h = header.usBottom - header.usTop  + 1;

	AutoSGPImage img(new SGPImage(w, h, 8));
	// Set some header information
	img->fFlags |= contents;

	// Read and allocate bitmap block if requested
	if (contents & IMAGE_BITMAPDATA)
	{
		UINT8* const img_data = img->pImageData.Allocate(w * h);
		BlitPcxToBuffer(pcx_buffer, img_data, w, h);
	}

	if (contents & IMAGE_PALETTE)
	{
		SGPPaletteEntry* const dst = img->pPalette.Allocate(256);
		for (size_t i = 0; i < 256; ++i)
		{
			dst[i].r      = palette[i * 3 + 0];
			dst[i].g      = palette[i * 3 + 1];
			dst[i].b      = palette[i * 3 + 2];
			dst[i].a      = 0;
		}
		img->pui16BPPPalette = Create16BPPPalette(dst);
	}

	return img.Release();
}


static void BlitPcxToBuffer(UINT8 const* src, UINT8* dst, UINT16 const w, UINT16 const h)
{
	for (size_t n = w * h; n != 0;)
	{
		if (*src >= 0xC0)
		{
			size_t      n_px   = *src++ & 0x3F;
			UINT8 const colour = *src++;
			if (n_px > n) n_px = n;
			n -= n_px;
			for (; n_px != 0; --n_px) *dst++ = colour;
		}
		else
		{
			--n;
			*dst++ = *src++;
		}
	}
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(PCX, asserts)
{
	EXPECT_EQ(sizeof(PcxHeader), 128);
}

#endif
