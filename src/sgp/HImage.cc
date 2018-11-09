#include <stdexcept>

#include "Types.h"
#include "Debug.h"
#include "HImage.h"
#include "ImpTGA.h"
#include "PCX.h"
#include "STCI.h"
#include "WCheck.h"
#include "VObject.h"

#include "Logger.h"

// This is the color substituted to keep a 24bpp -> 16bpp color
// from going transparent (0x0000) -- DB

#define BLACK_SUBSTITUTE	0x0001


UINT16 gusRedMask = 0;
UINT16 gusGreenMask = 0;
UINT16 gusBlueMask = 0;
INT16  gusRedShift = 0;
INT16  gusBlueShift = 0;
INT16  gusGreenShift = 0;


SGPImage* CreateImage(const ST::string& filename, const UINT16 fContents)
{
	// depending on extension of filename, use different image readers
	ST::string ext = filename.after_last(".");
	if (ext == filename)
	{
		auto errorMessage = ST::format("Tried to load image `{}` with no extension", filename);
		throw std::logic_error(errorMessage.c_str());
	}

	if (ext.compare_i("STI") == 0) {
		return  LoadSTCIFileToImage(filename, fContents);
	}
	if (ext.compare_i("PCX") == 0) {
		return LoadPCXFileToImage( filename, fContents);
	}
	if (ext.compare_i("TGA") == 0) {
		return LoadTGAFileToImage( filename, fContents);
	}

	auto errorMessage = ST::format("Tried to load image `{}` with unknown extension", filename);
	throw std::logic_error(errorMessage.c_str());
}


static BOOLEAN Copy8BPPImageTo8BPPBuffer(SGPImage const* const img, BYTE* const pDestBuf, UINT16 const usDestWidth, UINT16 const usDestHeight, UINT16 const usX, UINT16 const usY, SGPBox const* const src_box)
{
	CHECKF(usX < usDestWidth);
	CHECKF(usY < usDestHeight);
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	// Determine memcopy coordinates
	UINT32 const uiSrcStart  = src_box->y * img->usWidth + src_box->x;
	UINT32 const uiDestStart = usY * usDestWidth + usX;
	UINT32 const uiLineSize  = src_box->w;
	UINT32 const uiNumLines  = src_box->h;

	Assert(usDestWidth  >= uiLineSize);
	Assert(usDestHeight >= uiNumLines);

	// Copy line by line
	UINT8*       dst = static_cast<UINT8*>(pDestBuf) + uiDestStart;
	UINT8 const* src = static_cast<UINT8 const*>(img->pImageData) + uiSrcStart;
	for (UINT32 n = uiNumLines; n != 0; --n)
	{
		memcpy(dst, src, uiLineSize);
		dst += usDestWidth;
		src += img->usWidth;
	}

	return TRUE;
}

static BOOLEAN Copy32BPPImageTo32BPPBuffer(SGPImage const* const img, BYTE* const pDestBuf, UINT16 const usDestWidth, UINT16 const usDestHeight, UINT16 const usX, UINT16 const usY, SGPBox const* const src_box)
{
	CHECKF(usX < img->usWidth);
	CHECKF(usY < img->usHeight);
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	// Determine memcopy coordinates
	const UINT32 uiLineSize = src_box->w;
	INT32 uiNumLines = src_box->h;

	CHECKF(usDestWidth  >= uiLineSize);
	CHECKF(usDestHeight >= uiNumLines);

	// Copy line by line
	UINT32 *dst = reinterpret_cast<UINT32*>(pDestBuf)
			+ usY * usDestWidth
			+ usX;
	const UINT32 *src = reinterpret_cast<const UINT32 *>(static_cast<const UINT8 *>(img->pImageData))
			+ src_box->y * img->usWidth
			+ src_box->x;
	while(uiNumLines-- > 0) {
		memcpy(dst, src, uiLineSize * 4);
		dst += usDestWidth;
		src += img->usWidth;
	}

	return TRUE;
}

static BOOLEAN Copy16BPPImageTo16BPPBuffer(SGPImage const* const img, BYTE* const pDestBuf, UINT16 const usDestWidth, UINT16 const usDestHeight, UINT16 const usX, UINT16 const usY, SGPBox const* const src_box)
{
	CHECKF(usX < img->usWidth);
	CHECKF(usY < img->usHeight);
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	// Determine memcopy coordinates
	UINT32 const uiSrcStart  = src_box->y * img->usWidth + src_box->x;
	UINT32 const uiDestStart = usY * usDestWidth + usX;
	UINT32 const uiLineSize  = src_box->w;
	UINT32 const uiNumLines  = src_box->h;

	CHECKF(usDestWidth  >= uiLineSize);
	CHECKF(usDestHeight >= uiNumLines);

	// Copy line by line
	UINT16*       dst = static_cast<UINT16*>(static_cast<void*>(pDestBuf)) + uiDestStart;
	UINT16 const* src = static_cast<UINT16 const*>(static_cast<void const*>(img->pImageData)) + uiSrcStart;
	for (UINT32 n = uiNumLines; n != 0; --n)
	{
		memcpy(dst, src, uiLineSize * 2);
		dst += usDestWidth;
		src += img->usWidth;
	}

	return TRUE;
}


static BOOLEAN Copy8BPPImageTo16BPPBuffer(SGPImage const* const img, BYTE* const pDestBuf, UINT16 const usDestWidth, UINT16 const usDestHeight, UINT16 const usX, UINT16 const usY, SGPBox const* const src_box)
{
	CHECKF(img->pImageData);
	CHECKF(usX < usDestWidth);
	CHECKF(usY < usDestHeight);
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	// Determine memcopy coordinates
	UINT32 const uiSrcStart  = src_box->y * img->usWidth + src_box->x;
	UINT32 const uiDestStart = usY * usDestWidth + usX;
	UINT32 const uiLineSize  = src_box->w;
	UINT32 const uiNumLines  = src_box->h;

	CHECKF(usDestWidth  >= uiLineSize);
	CHECKF(usDestHeight >= uiNumLines);

	// Convert to Pixel specification
	UINT16*             dst = static_cast<UINT16*>(static_cast<void*>(pDestBuf)) + uiDestStart;
	UINT8  const*       src = static_cast<UINT8 const*>(img->pImageData) + uiSrcStart;
	UINT16 const* const pal = img->pui16BPPPalette;
	for (UINT32 rows = uiNumLines; rows != 0; --rows)
	{
		UINT16*      dst_tmp = dst;
		UINT8 const* src_tmp = src;
		for (UINT32 cols = uiLineSize; cols != 0; --cols)
		{
			*dst_tmp++ = pal[*src_tmp++];
		}
		dst += usDestWidth;
		src += img->usWidth;
	}

	return TRUE;
}


BOOLEAN CopyImageToBuffer(SGPImage const* const img, UINT32 const fBufferType, BYTE* const pDestBuf, UINT16 const usDestWidth, UINT16 const usDestHeight, UINT16 const usX, UINT16 const usY, SGPBox const* const src_box)
{
	// Use blitter based on type of image
	if (img->ubBitDepth == 8 && fBufferType == BUFFER_8BPP)
	{
		// Default do here
		SLOGD("Copying 8 BPP Imagery.");
		return Copy8BPPImageTo8BPPBuffer(img, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}
	else if (img->ubBitDepth == 8 && fBufferType == BUFFER_16BPP)
	{
		SLOGD("Copying 8 BPP Imagery to 16BPP Buffer.");
		return Copy8BPPImageTo16BPPBuffer(img, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}
	else if (img->ubBitDepth == 16 && fBufferType == BUFFER_16BPP)
	{
		SLOGD("Automatically Copying 16 BPP Imagery.");
		return Copy16BPPImageTo16BPPBuffer(img, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}
	if (img->ubBitDepth == 32 && fBufferType == BUFFER_32BPP)
	{
		SLOGD("Automatically Copying 32 BPP Imagery.");
		return Copy32BPPImageTo32BPPBuffer(img, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}

	return FALSE;
}


UINT16* Create16BPPPalette(const SGPPaletteEntry* pPalette)
{
	Assert(pPalette != NULL);

	UINT16* const p16BPPPalette = new UINT16[256]{};

	for (UINT32 cnt = 0; cnt < 256; cnt++)
	{
		UINT8 const r = pPalette[cnt].r;
		UINT8 const g = pPalette[cnt].g;
		UINT8 const b = pPalette[cnt].b;
		p16BPPPalette[cnt] = RGB(r, g, b);
	}

	return p16BPPPalette;
}


/**********************************************************************************************
Create16BPPPaletteShaded

	Creates an 8 bit to 16 bit palette table, and modifies the colors as it builds.

	Parameters:
		rscale, gscale, bscale:
				Color mode: Percentages (255=100%) of color to translate into destination palette.
				Mono mode:  Color for monochrome palette.
		mono:
				TRUE or FALSE to create a monochrome palette. In mono mode, Luminance values for
				colors are calculated, and the RGB color is shaded according to each pixel's brightness.

	This can be used in several ways:

	1) To "brighten" a palette, pass down RGB values that are higher than 100% ( > 255) for all
			three. mono=FALSE.
	2) To "darken" a palette, do the same with less than 100% ( < 255) values. mono=FALSE.

	3) To create a "glow" palette, select mono=TRUE, and pass the color in the RGB parameters.

	4) For gamma correction, pass in weighted values for each color.

**********************************************************************************************/
UINT16* Create16BPPPaletteShaded(const SGPPaletteEntry* pPalette, UINT32 rscale, UINT32 gscale, UINT32 bscale, BOOLEAN mono)
{
	Assert(pPalette != NULL);

	UINT16* const p16BPPPalette = new UINT16[256]{};

	for (UINT32 cnt = 0; cnt < 256; cnt++)
	{
		UINT32 rmod;
		UINT32 gmod;
		UINT32 bmod;
		if (mono)
		{
			UINT32 lumin = (pPalette[cnt].r * 299 + pPalette[cnt].g * 587 + pPalette[cnt].b * 114) / 1000;
			rmod = rscale * lumin / 256;
			gmod = gscale * lumin / 256;
			bmod = bscale * lumin / 256;
		}
		else
		{
			rmod = rscale * pPalette[cnt].r / 256;
			gmod = gscale * pPalette[cnt].g / 256;
			bmod = bscale * pPalette[cnt].b / 256;
		}

		UINT8 r = std::min(rmod, 255U);
		UINT8 g = std::min(gmod, 255U);
		UINT8 b = std::min(bmod, 255U);
		p16BPPPalette[cnt] = RGB(r, g, b);
	}
	return p16BPPPalette;
}

#ifdef WITH_UNITTESTS
#undef FAIL
#include "gtest/gtest.h"

TEST(HImage, asserts)
{
	EXPECT_EQ(sizeof(AuxObjectData), 16u);
	EXPECT_EQ(sizeof(RelTileLoc), 2u);
	EXPECT_EQ(sizeof(ETRLEObject), 16u);
	EXPECT_EQ(sizeof(SGPPaletteEntry), 4u);
}

#endif
