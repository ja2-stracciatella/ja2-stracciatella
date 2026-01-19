#include <stdexcept>

#include "Types.h"
#include "Debug.h"
#include "HImage.h"
#include "ImpTGA.h"
#include "PCX.h"
#include "STCI.h"
#include "VObject.h"
#include "VSurface.h"


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


UINT16* Create16BPPPalette(const SGPPaletteEntry* pPalette)
{
	Assert(pPalette != NULL);

	UINT16* const p16BPPPalette = new UINT16[256]{};

	for (UINT32 cnt = 0; cnt < 256; cnt++)
	{
		UINT8 const r = pPalette[cnt].r;
		UINT8 const g = pPalette[cnt].g;
		UINT8 const b = pPalette[cnt].b;
		p16BPPPalette[cnt] = Get16BPPColor(r, g, b);
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
		p16BPPPalette[cnt] = Get16BPPColor(r, g, b);
	}
	return p16BPPPalette;
}


// Convert from RGB to 16 bit value
UINT16 Get16BPPColor( UINT32 RGBValue )
{
	UINT8 r = SGPGetRValue(RGBValue);
	UINT8 g = SGPGetGValue(RGBValue);
	UINT8 b = SGPGetBValue(RGBValue);

	return Get16BPPColor(r, g, b);
}


UINT16 Get16BPPColor(UINT8 r, UINT8 g, UINT8 b)
{
	UINT16 r16 = (gusRedShift   < 0 ? r >> -gusRedShift   : r << gusRedShift);
	UINT16 g16 = (gusGreenShift < 0 ? g >> -gusGreenShift : g << gusGreenShift);
	UINT16 b16 = (gusBlueShift  < 0 ? b >> -gusBlueShift  : b << gusBlueShift);

	UINT16 usColor = (r16 & gusRedMask) | (g16 & gusGreenMask) | (b16 & gusBlueMask);

	// if our color worked out to absolute black, and the original wasn't
	// absolute black, convert it to a VERY dark grey to avoid transparency
	// problems
	return (usColor == 0 && r + g + b != 0) ? BLACK_SUBSTITUTE : usColor;
}


// Convert from 16 BPP to RGBvalue
UINT32 GetRGBColor(UINT16 Value16BPP)
{
	UINT8 r, g, b;
	SDL_GetRGB(Value16BPP, g_frame_buffer->GetSDLSurface().format, &r, &g, &b);
	return FROMRGB(r, g, b);
}


static_assert(sizeof(AuxObjectData) == 16u);
static_assert(sizeof(RelTileLoc) == 2u);
static_assert(sizeof(ETRLEObject) == 16u);
static_assert(sizeof(SGPPaletteEntry) == 4u);
