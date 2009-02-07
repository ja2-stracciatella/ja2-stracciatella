#include <stdexcept>

#include "Types.h"
#include "Debug.h"
#include "FileMan.h"
#include "HImage.h"
#include "ImpTGA.h"
#include "PCX.h"
#include "STCI.h"
#include "WCheck.h"
#include "VObject.h"
#include "MemMan.h"


// This is the color substituted to keep a 24bpp -> 16bpp color
// from going transparent (0x0000) -- DB

#define BLACK_SUBSTITUTE	0x0001


UINT16 gusRedMask = 0;
UINT16 gusGreenMask = 0;
UINT16 gusBlueMask = 0;
INT16  gusRedShift = 0;
INT16  gusBlueShift = 0;
INT16  gusGreenShift = 0;


SGPImage* CreateImage(const char* const filename, const UINT16 fContents)
{
	// depending on extension of filename, use different image readers
	const char* const dot = strstr(filename, ".");
	if (!dot) throw std::logic_error("Tried to load image with no extension");
	const char* const ext = dot + 1;

	return
		strcasecmp(ext, "STI") == 0 ? LoadSTCIFileToImage(filename, fContents) :
		strcasecmp(ext, "PCX") == 0 ? LoadPCXFileToImage( filename, fContents) :
		strcasecmp(ext, "TGA") == 0 ? LoadTGAFileToImage( filename, fContents) :
		throw std::logic_error("Tried to load image with unknown extension");
}


SGPImage::~SGPImage()
{
	if (pETRLEObject)    MemFree(pETRLEObject);
	if (pImageData)      MemFree(pImageData);
	if (pAppData)        MemFree(pAppData);
	if (pui16BPPPalette) MemFree(pui16BPPPalette);
}


static BOOLEAN Copy8BPPImageTo8BPPBuffer(  HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);
static BOOLEAN Copy16BPPImageTo16BPPBuffer(HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);
static BOOLEAN Copy8BPPImageTo16BPPBuffer( HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);


BOOLEAN CopyImageToBuffer(const HIMAGE hImage, const UINT32 fBufferType, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	// Use blitter based on type of image
	Assert( hImage != NULL );

	if ( hImage->ubBitDepth == 8 && fBufferType == BUFFER_8BPP )
	{
		// Default do here
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_2, "Copying 8 BPP Imagery." );
		return Copy8BPPImageTo8BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}

	if ( hImage->ubBitDepth == 8 && fBufferType == BUFFER_16BPP )
	{
		// Default do here
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Copying 8 BPP Imagery to 16BPP Buffer." );
		return Copy8BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);

	}


	if ( hImage->ubBitDepth == 16 && fBufferType == BUFFER_16BPP )
	{
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Automatically Copying 16 BPP Imagery." );
		return Copy16BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}

	return( FALSE );

}


static BOOLEAN Copy8BPPImageTo8BPPBuffer(const HIMAGE hImage, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	UINT32 cnt;
	UINT8 *pDest, *pSrc;

	// Assertions
	Assert( hImage != NULL );
	Assert(hImage->pImageData);

	// Validations
	//CHECKF( usX >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usX < usDestWidth );
	//CHECKF( usY >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usY < usDestHeight );
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	// Determine memcopy coordinates
	const UINT32 uiSrcStart  = src_box->y * hImage->usWidth + src_box->x;
	const UINT32 uiDestStart = usY * usDestWidth + usX;
	const UINT32 uiLineSize  = src_box->w;
	const UINT32 uiNumLines  = src_box->h;

	Assert( usDestWidth >= uiLineSize );
	Assert( usDestHeight >= uiNumLines );

	// Copy line by line
	pDest = ( UINT8*)pDestBuf + uiDestStart;
	pSrc = static_cast<UINT8*>(hImage->pImageData) + uiSrcStart;

	for( cnt = 0; cnt < uiNumLines-1; cnt++ )
	{
		memcpy( pDest, pSrc, uiLineSize );
		pDest += usDestWidth;
		pSrc  += hImage->usWidth;
	}
	// Do last line
	memcpy( pDest, pSrc, uiLineSize );

	return( TRUE );

}


static BOOLEAN Copy16BPPImageTo16BPPBuffer(const HIMAGE hImage, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	UINT32 cnt;
	UINT16 *pDest, *pSrc;

	Assert( hImage != NULL );
	Assert(hImage->pImageData);

	// Validations
	//CHECKF( usX >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usX < hImage->usWidth );
	//CHECKF( usY >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usY < hImage->usHeight );
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	// Determine memcopy coordinates
	const UINT32 uiSrcStart  = src_box->y * hImage->usWidth + src_box->x;
	const UINT32 uiDestStart = usY * usDestWidth + usX;
	const UINT32 uiLineSize  = src_box->w;
	const UINT32 uiNumLines  = src_box->h;

	CHECKF( usDestWidth >= uiLineSize );
	CHECKF( usDestHeight >= uiNumLines );

	// Copy line by line
	pDest = ( UINT16*)pDestBuf + uiDestStart;
	pSrc = static_cast<UINT16*>(hImage->pImageData) + uiSrcStart;

	for( cnt = 0; cnt < uiNumLines-1; cnt++ )
	{
		memcpy( pDest, pSrc, uiLineSize * 2 );
		pDest += usDestWidth;
		pSrc  += hImage->usWidth;
	}
	// Do last line
	memcpy( pDest, pSrc, uiLineSize * 2 );

	return( TRUE );

}


static BOOLEAN Copy8BPPImageTo16BPPBuffer(const HIMAGE hImage, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	UINT32 rows, cols;
	UINT8  *pSrc, *pSrcTemp;
	UINT16 *pDest, *pDestTemp;
	UINT16 *p16BPPPalette;


	p16BPPPalette = hImage->pui16BPPPalette;

	// Assertions
	Assert( p16BPPPalette != NULL );
	Assert( hImage != NULL );

	// Validations
	CHECKF(hImage->pImageData);
	//CHECKF( usX >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usX < usDestWidth );
	//CHECKF( usY >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usY < usDestHeight );
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	// Determine memcopy coordinates
	const UINT32 uiSrcStart  = src_box->y * hImage->usWidth + src_box->x;
	const UINT32 uiDestStart = usY * usDestWidth + usX;
	const UINT32 uiLineSize  = src_box->w;
	const UINT32 uiNumLines  = src_box->h;

	CHECKF( usDestWidth >= uiLineSize );
	CHECKF( usDestHeight >= uiNumLines );

	// Convert to Pixel specification
	pDest = ( UINT16*)pDestBuf + uiDestStart;
	pSrc = static_cast<UINT8*>(hImage->pImageData) + uiSrcStart;
	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, String( "Start Copying at %p", pDest ) );

	// For every entry, look up into 16BPP palette
	for( rows = 0; rows < uiNumLines-1; rows++ )
	{
		pDestTemp = pDest;
		pSrcTemp = pSrc;

		for ( cols = 0; cols < uiLineSize; cols++ )
		{
			*pDestTemp = p16BPPPalette[ *pSrcTemp ];
			pDestTemp++;
			pSrcTemp++;
		}

		pDest += usDestWidth;
		pSrc  += hImage->usWidth;
	}
	// Do last line
	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, String( "End Copying at %p", pDest ) );

	return( TRUE );

}


UINT16* Create16BPPPalette(const SGPPaletteEntry* pPalette)
{
	Assert(pPalette != NULL);

	UINT16* const p16BPPPalette = MALLOCN(UINT16, 256);

	for (UINT32 cnt = 0; cnt < 256; cnt++)
	{
		UINT8 const r = pPalette[cnt].r;
		UINT8 const g = pPalette[cnt].g;
		UINT8 const b = pPalette[cnt].b;
		p16BPPPalette[cnt] = Get16BPPColor(FROMRGB(r, g, b));
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

	UINT16* const p16BPPPalette = MALLOCN(UINT16, 256);

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

		UINT8 r = __min(rmod, 255);
		UINT8 g = __min(gmod, 255);
		UINT8 b = __min(bmod, 255);
		p16BPPPalette[cnt] = Get16BPPColor(FROMRGB(r, g, b));
	}
	return p16BPPPalette;
}


// Convert from RGB to 16 bit value
UINT16 Get16BPPColor( UINT32 RGBValue )
{
	UINT8 r = SGPGetRValue(RGBValue);
	UINT8 g = SGPGetGValue(RGBValue);
	UINT8 b = SGPGetBValue(RGBValue);

	UINT16 r16 = (gusRedShift   < 0 ? r >> -gusRedShift   : r << gusRedShift);
	UINT16 g16 = (gusGreenShift < 0 ? g >> -gusGreenShift : g << gusGreenShift);
	UINT16 b16 = (gusBlueShift  < 0 ? b >> -gusBlueShift  : b << gusBlueShift);

	UINT16 usColor = (r16 & gusRedMask) | (g16 & gusGreenMask) | (b16 & gusBlueMask);

	// if our color worked out to absolute black, and the original wasn't
	// absolute black, convert it to a VERY dark grey to avoid transparency
	// problems
	if (usColor == 0 && RGBValue != 0) usColor = BLACK_SUBSTITUTE;

	return usColor;
}


// Convert from 16 BPP to RGBvalue
UINT32 GetRGBColor(UINT16 Value16BPP)
{
	UINT32 r16 = Value16BPP & gusRedMask;
	UINT32 g16 = Value16BPP & gusGreenMask;
	UINT32 b16 = Value16BPP & gusBlueMask;

	UINT32 r = (gusRedShift   < 0 ? r16 << -gusRedShift   : r16 >> gusRedShift);
	UINT32 g = (gusGreenShift < 0 ? g16 << -gusGreenShift : g16 >> gusGreenShift);
	UINT32 b = (gusBlueShift  < 0 ? b16 << -gusBlueShift  : b16 >> gusBlueShift);

	r &= 0x000000ff;
	g &= 0x000000ff;
	b &= 0x000000ff;

	UINT32 val = FROMRGB(r, g, b);
	return val;
}


void GetETRLEImageData(SGPImage const* const img, ETRLEData* const buf)
{
	Assert(img);
	Assert(buf);

	SGP::Buffer<ETRLEObject> etrle_objs(img->usNumberOfObjects);
	memcpy(etrle_objs, img->pETRLEObject, sizeof(*etrle_objs) * img->usNumberOfObjects);

	SGP::Buffer<UINT8> pix_data(img->uiSizePixData);
	memcpy(pix_data, img->pImageData, sizeof(*pix_data) * img->uiSizePixData);

	buf->pPixData          = pix_data.Release();
	buf->uiSizePixData     = img->uiSizePixData;
	buf->pETRLEObject      = etrle_objs.Release();
	buf->usNumberOfObjects = img->usNumberOfObjects;
}


void ConvertRGBDistribution565To555( UINT16 * p16BPPData, UINT32 uiNumberOfPixels )
{
	for (UINT16* Px = p16BPPData; Px != p16BPPData + uiNumberOfPixels; ++Px)
	{
		*Px = ((*Px >> 1) & ~0x001F) | (*Px & 0x001F);
	}
}

void ConvertRGBDistribution565To655( UINT16 * p16BPPData, UINT32 uiNumberOfPixels )
{
	for (UINT16* Px = p16BPPData; Px != p16BPPData + uiNumberOfPixels; ++Px)
	{
		*Px = ((*Px >> 1) & 0x03E0) | (*Px & ~0x07E0);
	}
}

void ConvertRGBDistribution565To556( UINT16 * p16BPPData, UINT32 uiNumberOfPixels )
{
	for (UINT16* Px = p16BPPData; Px != p16BPPData + uiNumberOfPixels; ++Px)
	{
		*Px = (*Px & ~0x003F) | ((*Px << 1) & 0x003F);
	}
}


void ConvertRGBDistribution565ToAny(UINT16* const p16BPPData, UINT32 const uiNumberOfPixels)
{
	UINT16* px = p16BPPData;
	for (size_t n = uiNumberOfPixels; n != 0; --n)
	{
		// put the 565 RGB 16-bit value into a 32-bit RGB value
		UINT32 const r   = (*px         ) >> 11;
		UINT32 const g   = (*px & 0x07E0) >>  5;
		UINT32 const b   = (*px & 0x001F);
		UINT32 const rgb = FROMRGB(r, g, b);
		// then convert the 32-bit RGB value to whatever 16 bit format is used
		*px++ = Get16BPPColor(rgb);
	}
}
