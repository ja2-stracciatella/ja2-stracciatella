#include "Types.h"
#include "Debug.h"
#include "FileMan.h"
#include "HImage.h"
#include "ImpTGA.h"
#include "PCX.h"
#include "STCI.h"
#include "WCheck.h"
#include "Compression.h"
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
	if (dot == NULL) return NULL;
	const char* const ext = dot + 1;

	SGPImage* const img = MALLOCZ(SGPImage);
	if (img == NULL) return NULL;
	strcpy(img->ImageFile, filename);

	// determine type from extension
	BOOLEAN ret;
	if (strcasecmp(ext, "STI") == 0)
	{
		ret = LoadSTCIFileToImage(img, fContents);
	}
	else if (strcasecmp(ext, "PCX") == 0)
	{
		ret = LoadPCXFileToImage(img, fContents);
	}
	else if (strcasecmp(ext, "TGA") == 0)
	{
		ret = LoadTGAFileToImage(img, fContents);
	}
	else
	{
		DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_2, String("Resource file \"%s\" has unknown extension", filename));
		goto fail_img;
	}

	if (!ret)
	{
		DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_2, "Error occured while reading image data.");
		goto fail_img;
	}

	return img;

fail_img:
	MemFree(img);
	return NULL;
}


static BOOLEAN ReleaseImageData(HIMAGE hImage, UINT16 fContents);


void DestroyImage(SGPImage* const hImage)
{
	Assert( hImage != NULL );

	// First delete contents
	ReleaseImageData( hImage, IMAGE_ALLDATA );//hImage->fFlags );

	// Now free structure
	MemFree( hImage );
}


static BOOLEAN ReleaseImageData(HIMAGE hImage, UINT16 fContents)
{

	Assert( hImage != NULL );

	if ( (fContents & IMAGE_PALETTE) && (hImage->fFlags & IMAGE_PALETTE) )
	{
		//Destroy palette
		if( hImage->pPalette != NULL )
		{
			MemFree( hImage->pPalette );
			hImage->pPalette = NULL;
		}

		if ( hImage->pui16BPPPalette != NULL )
		{
			MemFree( hImage->pui16BPPPalette );
			hImage->pui16BPPPalette = NULL;
		}

		// Remove contents flag
		hImage->fFlags = hImage->fFlags ^ IMAGE_PALETTE;
	}

	if ( (fContents & IMAGE_BITMAPDATA) && (hImage->fFlags & IMAGE_BITMAPDATA) )
	{
		//Destroy image data
		Assert( hImage->pImageData != NULL );
		MemFree( hImage->pImageData );
		hImage->pImageData = NULL;
		if (hImage->usNumberOfObjects > 0)
		{
			MemFree( hImage->pETRLEObject );
		}
		// Remove contents flag
		hImage->fFlags = hImage->fFlags ^ IMAGE_BITMAPDATA;
	}

	if ( (fContents & IMAGE_APPDATA) && (hImage->fFlags & IMAGE_APPDATA) )
	{
		// get rid of the APP DATA
		if ( hImage->pAppData != NULL )
		{
			MemFree( hImage->pAppData );
			hImage->fFlags &= (~IMAGE_APPDATA);
		}
	}

	return( TRUE );
}



#if defined WITH_ZLIB
static BOOLEAN Copy8BPPCompressedImageTo8BPPBuffer(  HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);
static BOOLEAN Copy8BPPCompressedImageTo16BPPBuffer( HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);
static BOOLEAN Copy16BPPCompressedImageTo16BPPBuffer(HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);
#endif

static BOOLEAN Copy8BPPImageTo8BPPBuffer(  HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);
static BOOLEAN Copy16BPPImageTo16BPPBuffer(HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);
static BOOLEAN Copy8BPPImageTo16BPPBuffer( HIMAGE hImage, BYTE* pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, const SGPBox* src_box);


BOOLEAN CopyImageToBuffer(const HIMAGE hImage, const UINT32 fBufferType, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	// Use blitter based on type of image
	Assert( hImage != NULL );

	if ( hImage->ubBitDepth == 8 && fBufferType == BUFFER_8BPP )
	{
#if defined WITH_ZLIB
		if (hImage->fFlags & IMAGE_COMPRESSED)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_2, "Copying Compressed 8 BPP Imagery.");
			return Copy8BPPCompressedImageTo8BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
		}
#endif

		// Default do here
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_2, "Copying 8 BPP Imagery." );
		return Copy8BPPImageTo8BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}

	if ( hImage->ubBitDepth == 8 && fBufferType == BUFFER_16BPP )
	{
#if defined WITH_ZLIB
		if (hImage->fFlags & IMAGE_COMPRESSED)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Copying Compressed 8 BPP Imagery to 16BPP Buffer.");
			return Copy8BPPCompressedImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
		}
#endif

		// Default do here
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Copying 8 BPP Imagery to 16BPP Buffer." );
		return Copy8BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);

	}


	if ( hImage->ubBitDepth == 16 && fBufferType == BUFFER_16BPP )
	{
#if defined WITH_ZLIB
		if (hImage->fFlags & IMAGE_COMPRESSED)
		{
			DebugMsg(TOPIC_HIMAGE, DBG_LEVEL_3, "Automatically Copying Compressed 16 BPP Imagery.");
			return  Copy16BPPCompressedImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
		}
#endif

			DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Automatically Copying 16 BPP Imagery." );
		return Copy16BPPImageTo16BPPBuffer(hImage, pDestBuf, usDestWidth, usDestHeight, usX, usY, src_box);
	}

	return( FALSE );

}


#if defined WITH_ZLIB

static BOOLEAN Copy8BPPCompressedImageTo8BPPBuffer(const HIMAGE hImage, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	UINT32		uiCnt;

	UINT8 *	pDest;
	UINT32	uiDestStart;

	PTR			pDecompPtr;
	UINT32	uiDecompressed;

	// Assertions
	Assert( hImage != NULL );
	Assert( hImage->pCompressedImageData != NULL );

	// Validations
	//CHECKF( usX >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usX < usDestWidth );
	//CHECKF( usY >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usY < usDestHeight );
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);

	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "8BPP to 8BPP Compressed Blitter Called!" );
	// determine where to start Copying and rectangle size
	uiDestStart = usY * usDestWidth + usX;
	const UINT32 uiLineSize = src_box->w;
	const UINT32 uiNumLines = src_box->h;

	Assert( usDestWidth >= uiLineSize );
	Assert( usDestHeight >= uiNumLines );

	pDest = (UINT8 *) pDestBuf + uiDestStart;

	// Copying a portion of a compressed image is rather messy
	// because we have to decompress past all the data we want
	// to skip.

	// To keep memory requirements small and regular, we will
	// decompress one scanline at a time even if none of the data will
	// be blitted (but stop when the bottom line of the rectangle
	// to blit has been done).

	// initialize the decompression routines
	pDecompPtr = DecompressInit( hImage->pCompressedImageData, hImage->usWidth * hImage->usHeight );
	CHECKF( pDecompPtr );

	// Allocate memory for one scanline
	UINT8* const pScanLine = MALLOCN(UINT8, hImage->usWidth);
	CHECKF( pScanLine );

	// go past all the scanlines we don't need to process
	for (uiCnt = 0; uiCnt < (UINT32)src_box->y; uiCnt++)
	{
		uiDecompressed = Decompress( pDecompPtr, pScanLine, hImage->usWidth );
		Assert( uiDecompressed == hImage->usWidth );
	}

	// now we start Copying
	for (uiCnt = 0; uiCnt < uiNumLines - 1; uiCnt++)
	{
		// decompress a scanline
		uiDecompressed = Decompress( pDecompPtr, pScanLine, hImage->usWidth );
		Assert( uiDecompressed == hImage->usWidth );
		// and blit
//		memcpy( pDest, pScanLine + src_box->x, uiLineSize );
		pDest += usDestWidth;
	}
	// decompress the last scanline and blit
	uiDecompressed = Decompress( pDecompPtr, pScanLine, hImage->usWidth );
	Assert( uiDecompressed == hImage->usWidth );
//	memcpy( pDest, pScanLine + src_box->x, uiLineSize );

	DecompressFini( pDecompPtr );
	return( TRUE );
}


static BOOLEAN Copy8BPPCompressedImageTo16BPPBuffer(const HIMAGE hImage, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	UINT32		uiLine;
	UINT32		uiCol;

	UINT16 *	pDest;
	UINT16 *	pDestTemp;
	UINT32		uiDestStart;

	UINT8 *		pScanLineTemp;

	PTR				pDecompPtr;
	UINT32		uiDecompressed;

	UINT16 *	p16BPPPalette;

	// Assertions
	Assert( hImage != NULL );
	Assert( hImage->pCompressedImageData != NULL );
	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Start check" );
	// Validations
	//CHECKF( usX >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usX < usDestWidth );
	//CHECKF( usY >= 0 ); /* XXX unsigned < 0 ? */
	CHECKF( usY < usDestHeight );
	CHECKF(src_box->w > 0);
	CHECKF(src_box->h > 0);
	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "End check" );
	p16BPPPalette = hImage->pui16BPPPalette;

	// determine where to start Copying and rectangle size
	uiDestStart = usY * usDestWidth + usX;
	const UINT32 uiLineSize = src_box->w;
	const UINT32 uiNumLines = src_box->h;

	Assert( usDestWidth >= uiLineSize );
	Assert( usDestHeight >= uiNumLines );

	pDest = (UINT16 *) pDestBuf;
	pDest += uiDestStart;
	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, String( "Start Copying at %p", pDest ) );

	// Copying a portion of a compressed image is rather messy
	// because we have to decompress past all the data we want
	// to skip.

	// To keep memory requirements small and regular, we will
	// decompress one scanline at a time even if none of the data will
	// be blitted (but stop when the bottom line of the rectangle
	// to blit has been done).

	// initialize the decompression routines
	pDecompPtr = DecompressInit( hImage->pCompressedImageData, hImage->usWidth * hImage->usHeight );
	CHECKF( pDecompPtr );

	// Allocate memory for one scanline
	UINT8* const pScanLine = MALLOCN(UINT8, hImage->usWidth);
	CHECKF( pScanLine );

	// go past all the scanlines we don't need to process
	for (uiLine = 0; uiLine < (UINT32)src_box->y; uiLine++)
	{
		DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Skipping scanline" );
		uiDecompressed = Decompress( pDecompPtr, pScanLine, hImage->usWidth );
		Assert( uiDecompressed == hImage->usWidth );
	}

	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, "Actually Copying" );
	// now we start Copying
	for (uiLine = 0; uiLine < uiNumLines - 1; uiLine++)
	{
		// decompress a scanline
		uiDecompressed = Decompress( pDecompPtr, pScanLine, hImage->usWidth );
		Assert( uiDecompressed == hImage->usWidth );

		// set pointers and blit
		pDestTemp = pDest;
		pScanLineTemp = pScanLine + src_box->x;
		for (uiCol = 0; uiCol < uiLineSize; uiCol++ )
		{
			*pDestTemp = p16BPPPalette[ *pScanLineTemp ];
			pDestTemp++;
			pScanLineTemp++;
		}
		pDest += usDestWidth;
	}

	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_3, String( "End Copying at %p", pDest ) );

	DecompressFini( pDecompPtr );
	return( TRUE );
}


static BOOLEAN Copy16BPPCompressedImageTo16BPPBuffer(const HIMAGE hImage, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	// 16BPP Compressed image has not been implemented yet
	DebugMsg( TOPIC_HIMAGE, DBG_LEVEL_2, "16BPP Compressed imagery blitter has not been implemented yet." );
	return( FALSE );
}
#endif


static BOOLEAN Copy8BPPImageTo8BPPBuffer(const HIMAGE hImage, BYTE* const pDestBuf, const UINT16 usDestWidth, const UINT16 usDestHeight, const UINT16 usX, const UINT16 usY, const SGPBox* const src_box)
{
	UINT32 cnt;
	UINT8 *pDest, *pSrc;

	// Assertions
	Assert( hImage != NULL );
	Assert( hImage->p16BPPData != NULL );

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
	pSrc =  hImage->p8BPPData + uiSrcStart;

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
	Assert( hImage->p16BPPData != NULL );

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
	pSrc =  hImage->p16BPPData + uiSrcStart;

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
	CHECKF( hImage->p16BPPData != NULL );
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
	pSrc =  hImage->p8BPPData + uiSrcStart;
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
		UINT8 r = pPalette[cnt].peRed;
		UINT8 g = pPalette[cnt].peGreen;
		UINT8 b = pPalette[cnt].peBlue;
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
			UINT32 lumin = (pPalette[cnt].peRed * 299 + pPalette[cnt].peGreen * 587 + pPalette[cnt].peBlue * 114) / 1000;
			rmod = rscale * lumin / 256;
			gmod = gscale * lumin / 256;
			bmod = bscale * lumin / 256;
		}
		else
		{
			rmod = rscale * pPalette[cnt].peRed   / 256;
			gmod = gscale * pPalette[cnt].peGreen / 256;
			bmod = bscale * pPalette[cnt].peBlue  / 256;
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


BOOLEAN GetETRLEImageData( HIMAGE hImage, ETRLEData *pBuffer )
{
	// Assertions
	Assert( hImage != NULL );
	Assert( pBuffer != NULL );

	// Create memory for data
	pBuffer->usNumberOfObjects = hImage->usNumberOfObjects;

	// Create buffer for objects
	pBuffer->pETRLEObject = MALLOCN(ETRLEObject, pBuffer->usNumberOfObjects);
	CHECKF( pBuffer->pETRLEObject != NULL );

	// Copy into buffer
	memcpy( pBuffer->pETRLEObject, hImage->pETRLEObject, sizeof( ETRLEObject ) * pBuffer->usNumberOfObjects );

	// Allocate memory for pixel data
	pBuffer->pPixData = MALLOCN(UINT8, hImage->uiSizePixData);
	CHECKF( pBuffer->pPixData != NULL );

	pBuffer->uiSizePixData = hImage->uiSizePixData;

	// Copy into buffer
	memcpy( pBuffer->pPixData, hImage->pPixData8, pBuffer->uiSizePixData );

	return( TRUE );
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

void ConvertRGBDistribution565ToAny( UINT16 * p16BPPData, UINT32 uiNumberOfPixels )
{
	UINT16 *	pPixel;
	UINT32		uiRed, uiGreen, uiBlue, uiTemp, uiLoop;

	pPixel = p16BPPData;
	for (uiLoop = 0; uiLoop < uiNumberOfPixels; uiLoop++)
	{
		// put the 565 RGB 16-bit value into a 32-bit RGB value
		uiRed = (*pPixel) >> 11;
		uiGreen = (*pPixel & 0x07E0) >> 5;
		uiBlue = (*pPixel & 0x001F);
		uiTemp = FROMRGB(uiRed,uiGreen,uiBlue);
		// then convert the 32-bit RGB value to whatever 16 bit format is used
		*pPixel = Get16BPPColor( uiTemp );
		pPixel++;
	}
}
