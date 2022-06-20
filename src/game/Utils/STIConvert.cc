#include "Buffer.h"
#include "Debug.h"
#include "Types.h"
#include "ImgFmt.h"
#include "HImage.h"
#include "STIConvert.h"
#include "WCheck.h"
#include "FileMan.h"

#include <vector>


#define CONVERT_ADD_APPDATA										0x0001
#define CONVERT_ADD_JA2DATA										0x0003
#define CONVERT_ETRLE_COMPRESS								0x0020
#define CONVERT_ETRLE_COMPRESS_SINGLE					0x0040
#define CONVERT_ETRLE_NO_SUBIMAGE_SHRINKING		0x0080
#define CONVERT_ETRLE_DONT_SKIP_BLANKS				0x0100
#define CONVERT_ETRLE_FLIC										0x0200
#define CONVERT_ETRLE_FLIC_TRIM								0x0400
#define CONVERT_ETRLE_FLIC_NAME								0x0800
#define CONVERT_TO_8_BIT											0x1000
#define CONVERT_TO_16_BIT											0x2000

// Defines for inserting red/green/blue values into a 16-bit pixel.
// MASK is the mask to use to get the proper bits out of a byte (part of a 24-bit pixel)
// use SHIFT_RIGHT to move the masked bits to the lowest bits of the byte
// use SHIFT_LEFT to put the bits in their proper place in the 16-bit pixel
#define RED_DEPTH_16					5
#define GREEN_DEPTH_16				6
#define BLUE_DEPTH_16					5
#define RED_MASK_16						0xF8
#define RED_SHIFT_RIGHT_16		3
#define RED_SHIFT_LEFT_16			11
#define GREEN_MASK_16					0xFC
#define GREEN_SHIFT_RIGHT_16	2
#define GREEN_SHIFT_LEFT_16		5
#define BLUE_MASK_16					0xF8
#define BLUE_SHIFT_RIGHT_16		3
#define BLUE_SHIFT_LEFT_16		0

#define RED_DEPTH_24					8
#define GREEN_DEPTH_24				8
#define BLUE_DEPTH_24					8
#define RED_MASK_24						0x00FF0000
#define GREEN_MASK_24					0x0000FF00
#define BLUE_MASK_24					0x000000FF

//#define JA2_OBJECT_DATA_SIZE	16

static BOOLEAN ConvertToETRLE(UINT8** ppDest, UINT32* puiDestLen, std::vector<STCISubImage>& subImages, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, UINT32 fFlags);


void WriteSTIFile(UINT8* const pData, SGPPaletteEntry* const pPalette, const INT16 sWidth, const INT16 sHeight, const char* const cOutputName, const UINT32 fFlags, const UINT32 uiAppDataSize)
{
	UINT32							uiOriginalSize;
	UINT8 *							pOutputBuffer = NULL;
	UINT32							uiCompressedSize=0; // Wmaybe-uninitialized

	STCIHeader					Header;
	UINT32							uiLoop;

	SGPPaletteEntry *		pSGPPaletteEntry;
	STCIPaletteElement	STCIPaletteEntry;

	std::vector<STCISubImage> subImages;

	//UINT16							usLoop;

	Header = STCIHeader{};

	uiOriginalSize = sWidth * sHeight * (8 / 8);

	// set up STCI header for output
	memcpy( Header.cID, STCI_ID_STRING, STCI_ID_LEN );
	Header.uiTransparentValue = 0;
	Header.usHeight = sHeight;
	Header.usWidth = sWidth;
	Header.ubDepth = 8;
	Header.uiOriginalSize = uiOriginalSize;
	Header.uiStoredSize = uiOriginalSize;
	Header.uiAppDataSize = uiAppDataSize;

	Header.fFlags |= STCI_INDEXED;
	if (Header.ubDepth == 8)
	{
		// assume 8-bit pixels indexing into 256 colour palette with 24 bit values in
		// the palette
		Header.Indexed.uiNumberOfColours = 256;
		Header.Indexed.ubRedDepth = 8;
		Header.Indexed.ubGreenDepth = 8;
		Header.Indexed.ubBlueDepth = 8;
	}

	if ((Header.fFlags & STCI_INDEXED) && (fFlags & CONVERT_ETRLE_COMPRESS))
	{
		ConvertToETRLE(&pOutputBuffer, &uiCompressedSize, subImages, pData, sWidth, sHeight, fFlags);
		Assert(subImages.size() <= UINT16_MAX);
		Header.Indexed.usNumberOfSubImages = static_cast<UINT16>(subImages.size());
		Header.uiStoredSize = uiCompressedSize;
		Header.fFlags |= STCI_ETRLE_COMPRESSED;
	}

	//
	// save file
	//

	AutoSGPFile f{FileMan::openForWriting(cOutputName)};

	// write header
	f->write(reinterpret_cast<const uint8_t*>(&Header), sizeof(Header));

	// write palette and subimage structs, if any
	if (Header.fFlags & STCI_INDEXED)
	{
		if (pPalette != NULL)
		{
			// have to convert palette to STCI format!
			pSGPPaletteEntry = pPalette;
			for (uiLoop = 0; uiLoop < 256; uiLoop++)
			{
				STCIPaletteEntry.ubRed   = pSGPPaletteEntry[uiLoop].r;
				STCIPaletteEntry.ubGreen = pSGPPaletteEntry[uiLoop].g;
				STCIPaletteEntry.ubBlue  = pSGPPaletteEntry[uiLoop].b;
				f->write(reinterpret_cast<const uint8_t*>(&STCIPaletteEntry), sizeof(STCIPaletteEntry));
			}
		}
		if (Header.fFlags & STCI_ETRLE_COMPRESSED)
		{
			f->write(reinterpret_cast<const uint8_t*>(subImages.data()), subImages.size() * sizeof(STCISubImage));
		}
	}

	// write file data
	Assert(Header.fFlags & STCI_ETRLE_COMPRESSED);
	Assert(pOutputBuffer);
	f->write(reinterpret_cast<const uint8_t*>(pOutputBuffer), Header.uiStoredSize);

	// write app-specific data (blanked to 0)
	const uint8_t zero = 0;
	for (uiLoop = 0; uiLoop < Header.uiAppDataSize; uiLoop++)
	{
		f->write(&zero, sizeof(zero));
	}

	if( pOutputBuffer != NULL )
	{
		delete[] pOutputBuffer;
	}
}



#define COMPRESS_TRANSPARENT				0x80
#define COMPRESS_NON_TRANSPARENT		0x00
#define COMPRESS_RUN_LIMIT					0x7F

#define TCI		0x00
#define WI		0xFF


static BOOLEAN DetermineSubImageSize(UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage);
static BOOLEAN DetermineSubImageUsedSize(UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage);
static UINT32 ETRLECompressSubImage(UINT8* pDest, UINT32 uiDestLen, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage);
static BOOLEAN GoPastWall(INT16* psNewX, INT16* psNewY, UINT16 usWidth, UINT16 usHeight, UINT8* pCurrent, INT16 sCurrX, INT16 sCurrY);
static BOOLEAN GoToNextSubImage(INT16* psNewX, INT16* psNewY, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, INT16 sOrigX, INT16 sOrigY);


/// Converts an indexed image to ETRLE subimages.
/// The caller is responsible for the memory left in *ppDest.
static BOOLEAN ConvertToETRLE(UINT8** const ppDest, UINT32* const puiDestLen, std::vector<STCISubImage>& subImages, UINT8* const p8BPPBuffer, const UINT16 usWidth, const UINT16 usHeight, const UINT32 fFlags)
try
{
	INT16						sCurrX;
	INT16						sCurrY;
	INT16						sNextX;
	INT16						sNextY;
	UINT8 *					pOutputNext;
	BOOLEAN					fContinue = TRUE;
	BOOLEAN					fOk = TRUE;
	BOOLEAN					fStore;
	BOOLEAN					fNextExists;
	UINT32					uiSubImageCompressedSize;
	UINT32					uiSpaceLeft;

	// worst-case situation	estimate
	uiSpaceLeft = (UINT32) usWidth * (UINT32) usHeight * 3;
	SGP::Buffer<UINT8> dest(uiSpaceLeft);
	*puiDestLen = uiSpaceLeft;

	pOutputNext = dest;

	if (fFlags & CONVERT_ETRLE_COMPRESS_SINGLE)
	{
		// there are no walls in this image, but we treat it as a "subimage" for
		// the purposes of calling the compressor

		// we want a 1-element SubImage array for this...
		// allocate!
		subImages.assign(1, STCISubImage{});
		STCISubImage* const pCurrSubImage = &subImages.back();
		pCurrSubImage->sOffsetX = 0;
		pCurrSubImage->sOffsetY = 0;
		pCurrSubImage->usWidth = usWidth;
		pCurrSubImage->usHeight = usHeight;
		if (!(fFlags & CONVERT_ETRLE_NO_SUBIMAGE_SHRINKING))
		{
			if (!DetermineSubImageUsedSize(p8BPPBuffer, usWidth, usHeight, pCurrSubImage)) return FALSE;
		}
		uiSubImageCompressedSize = ETRLECompressSubImage( pOutputNext, uiSpaceLeft, p8BPPBuffer, usWidth, usHeight, pCurrSubImage );
		if (uiSubImageCompressedSize == 0) return FALSE;

		pCurrSubImage->uiDataOffset = 0;
		pCurrSubImage->uiDataLength = uiSubImageCompressedSize;
		*puiDestLen = uiSubImageCompressedSize;
		*ppDest     = dest.Release();
		return TRUE;
	}
	else
	{
		// skip any initial wall bytes to find the first subimage
		if (!GoPastWall(&sCurrX, &sCurrY, usWidth, usHeight, p8BPPBuffer, 0, 0)) return FALSE;
		subImages.clear();

		while (fContinue)
		try
		{
			STCISubImage subImage = STCISubImage{};

			subImage.sOffsetX = sCurrX;
			subImage.sOffsetY = sCurrY;
			// determine the subimage's full size
			if (!DetermineSubImageSize( p8BPPBuffer, usWidth, usHeight, &subImage ))
			{
				fOk = FALSE;
				break;
			}
			if (subImages.size() == 0 && subImage.usWidth == usWidth && subImage.usHeight == usHeight)
			{
				printf( "\tWarning: no walls (subimage delimiters) found.\n" );
			}

			STCISubImage tempSubImage = subImage;
			if (DetermineSubImageUsedSize( p8BPPBuffer, usWidth, usHeight, &tempSubImage))
			{
				// image has nontransparent data; we definitely want to store it
				fStore = TRUE;
				if (!(fFlags & CONVERT_ETRLE_NO_SUBIMAGE_SHRINKING))
				{
					subImage = tempSubImage;
				}
			}
			else if (fFlags & CONVERT_ETRLE_DONT_SKIP_BLANKS)
			{
				// image is transparent; we will store it if there is another subimage
				// to the right of it on the same line
				// find the next subimage
				fNextExists = GoToNextSubImage( &sNextX, &sNextY, p8BPPBuffer, usWidth, usHeight, sCurrX, sCurrY );
				if (fNextExists && sNextY == sCurrY )
				{
					fStore = TRUE;
				}
				else
				{
					// junk transparent section at the end of the line!
					fStore = FALSE;
				}
			}
			else
			{
				// transparent data; discarding
				fStore = FALSE;
			}

			if (fStore)
			{
				// we want to store this subimage!
				uiSubImageCompressedSize = ETRLECompressSubImage( pOutputNext, uiSpaceLeft, p8BPPBuffer, usWidth, usHeight, &subImage );
				if (uiSubImageCompressedSize == 0)
				{
					fOk = FALSE;
					break;
				}
				subImage.uiDataOffset = (*puiDestLen - uiSpaceLeft);
				subImage.uiDataLength = uiSubImageCompressedSize;
				// this is a cheap hack; the sOffsetX and sOffsetY values have been used
				// to store the location of the subimage within the whole image.  Now
				// we want the offset within the subimage, so, we subtract the coordatines
				// for the upper-left corner of the subimage.
				subImage.sOffsetX -= sCurrX;
				subImage.sOffsetY -= sCurrY;
				subImages.push_back(subImage);
				pOutputNext += uiSubImageCompressedSize;
				uiSpaceLeft -= uiSubImageCompressedSize;
			}
			// find the next subimage
			fContinue = GoToNextSubImage( &sCurrX, &sCurrY, p8BPPBuffer, usWidth, usHeight, sCurrX, sCurrY );
		}
		catch (...)
		{
			fOk = FALSE;
			break;
		}
	}
	if (!fOk)
	{
		return( FALSE );
	}

	*puiDestLen -= uiSpaceLeft;
	*ppDest      = dest.Release();
	return TRUE;
}
catch (...) { return FALSE; }


static BOOLEAN DetermineOffset(UINT32* puiOffset, UINT16 usWidth, UINT16 usHeight, INT16 sX, INT16 sY);
static UINT32 ETRLECompress(UINT8* pDest, UINT32 uiDestLen, UINT8* pSource, UINT32 uiSourceLen);


static UINT32 ETRLECompressSubImage(UINT8* pDest, UINT32 uiDestLen, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage)
{
	UINT16		usLoop;
	UINT32		uiScanLineCompressedSize;
	UINT32		uiSpaceLeft = uiDestLen;
	UINT32		uiOffset;
	UINT8 *		pCurrent;

	CHECKF(DetermineOffset(&uiOffset, usWidth, usHeight, pSubImage->sOffsetX, pSubImage->sOffsetY));
	pCurrent = p8BPPBuffer + uiOffset;

	for (usLoop = 0; usLoop < pSubImage->usHeight; usLoop++)
	{
		uiScanLineCompressedSize = ETRLECompress( pDest, uiSpaceLeft, pCurrent, pSubImage->usWidth );
		if (uiScanLineCompressedSize == 0 )
		{ // there wasn't enough room to complete the compression!
			return( 0 );
		}
		// reduce the amount of available space
		uiSpaceLeft -= uiScanLineCompressedSize;
		pDest += uiScanLineCompressedSize;
		// go to the next scanline
		pCurrent += usWidth;
	}
	return( uiDestLen - uiSpaceLeft );
}


static UINT32 ETRLECompress(UINT8* pDest, UINT32 uiDestLen, UINT8* pSource, UINT32 uiSourceLen)
{ // Compress a buffer (a scanline) into ETRLE format, which is a series of runs.
	// Each run starts with a byte whose high bit is 1 if the run is compressed, 0 otherwise.
	// The lower seven bits of that byte indicate the length of the run

	// ETRLECompress returns the number of bytes used by the compressed buffer, or 0 if an error
	// occurred

	// uiSourceLoc keeps track of our current position in the
	// source
	UINT32	uiSourceLoc = 0;
	// uiCurrentSourceLoc is used to look ahead in the source to
	// determine the length of runs
	UINT32	uiCurrentSourceLoc = 0;
	UINT32	uiDestLoc = 0;
	UINT8		ubLength = 0;


	while (uiSourceLoc < uiSourceLen && uiDestLoc < uiDestLen)
	{
		if (pSource[uiSourceLoc] == TCI)
		{ // transparent run - determine its length
			do
			{
				uiCurrentSourceLoc++;
				ubLength++;
			}
			while ((uiCurrentSourceLoc < uiSourceLen) && pSource[uiCurrentSourceLoc] == TCI && (ubLength < COMPRESS_RUN_LIMIT));
			// output run-byte
			pDest[uiDestLoc] = ubLength | COMPRESS_TRANSPARENT;

			// update location
			uiSourceLoc += ubLength;
			uiDestLoc += 1;
		}
		else
		{	// non-transparent run - determine its length
			do
			{
				uiCurrentSourceLoc++;
				ubLength++;
			}
			while ((uiCurrentSourceLoc < uiSourceLen) && (pSource[uiCurrentSourceLoc] != TCI) && (ubLength < COMPRESS_RUN_LIMIT));
			if (uiDestLoc + ubLength < uiDestLen)
			{
				// output run-byte
				pDest[uiDestLoc++] = ubLength | COMPRESS_NON_TRANSPARENT;

				// output run (and update location)
				memcpy( pDest + uiDestLoc, pSource + uiSourceLoc, ubLength );
				uiSourceLoc += ubLength;
				uiDestLoc += ubLength;
			}
			else
			{ // not enough room in dest buffer to copy the run!
				return( 0 );
			}
		}
		uiCurrentSourceLoc = uiSourceLoc;
		ubLength = 0;
	}
	if (uiDestLoc >= uiDestLen)
	{
		return( 0 );
	}
	else
	{
		// end with a run of 0 length (which might as well be non-transparent,
		// giving a 0-byte
		pDest[uiDestLoc++] = 0;
		return( uiDestLoc );
	}
}


static BOOLEAN DetermineOffset(UINT32* puiOffset, UINT16 usWidth, UINT16 usHeight, INT16 sX, INT16 sY)
{
	if (sX < 0 || sY < 0)
	{
		return( FALSE );
	}
	*puiOffset = (UINT32) sY * (UINT32) usWidth + (UINT32) sX;
	if (*puiOffset >= (UINT32) usWidth * (UINT32) usHeight)
	{
		return( FALSE );
	}
	return( TRUE );
}


static BOOLEAN GoPastWall(INT16* psNewX, INT16* psNewY, UINT16 usWidth, UINT16 usHeight, UINT8* pCurrent, INT16 sCurrX, INT16 sCurrY)
{
	// If the current pixel is a wall, we assume that it is on a horizontal wall and
	// search right, wrapping around the end of scanlines, until we find non-wall data.
	while (*pCurrent == WI)
	{
		sCurrX++;
		pCurrent++;
		if (sCurrX == usWidth)
		{ // wrap our logical coordinates!
			sCurrX = 0;
			sCurrY++;
			if( sCurrY == usHeight)
			{
				// no more images!
				return( FALSE );
			}
		}
	}

	*psNewX = sCurrX;
	*psNewY = sCurrY;
	return( TRUE );
}


static BOOLEAN GoToNextSubImage(INT16* psNewX, INT16* psNewY, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, INT16 sOrigX, INT16 sOrigY)
{	// return the coordinates of the next subimage in the image
	// (either to the right, or the first of the next row down
	INT16				sCurrX = sOrigX;
	INT16				sCurrY = sOrigY;
	UINT32			uiOffset;
	UINT8 *			pCurrent;
	BOOLEAN			fFound = TRUE;

	CHECKF(DetermineOffset(&uiOffset, usWidth, usHeight, sCurrX, sCurrY));
	pCurrent = p8BPPBuffer + uiOffset;

	if (*pCurrent == WI)
	{
		return( GoPastWall( psNewX, psNewY, usWidth, usHeight, pCurrent, sCurrX, sCurrY ) );
	}
	else
	{
		// The current pixel is not a wall.  We scan right past all non-wall data to skip to
		// the right-hand end of the subimage, then right past all wall data to skip a vertical
		// wall, and should find ourselves at another subimage.

		// If we hit the right edge of the image, we back up to our start point, go DOWN to
		// the bottom of the image to the horizontal wall, and then recurse to go along it
		// to the right place on the next scanline

		while (*pCurrent != WI)
		{
			sCurrX++;
			pCurrent++;
			if (sCurrX == usWidth)
			{ // there are no more images to the right!
				fFound = FALSE;
				break;
			}
		}
		if (sCurrX < usWidth)
		{
			// skip all wall data to the right, starting at the new current position
			while (*pCurrent == WI)
			{
				sCurrX++;
				pCurrent++;
				if (sCurrX == usWidth)
				{ // there are no more images to the right!
					fFound = FALSE;
					break;
				}
			}
		}
		if (fFound)
		{
			*psNewX = sCurrX;
			*psNewY = sCurrY;
			return( TRUE );
		}
		else
		{
			// go back to the beginning of the subimage and scan down
			sCurrX = sOrigX;
			pCurrent = p8BPPBuffer + uiOffset;

			// skip all non-wall data below, starting at the current position
			while (*pCurrent != WI)
			{
				sCurrY++;
				pCurrent += usWidth;
				if (sCurrY == usHeight)
				{ // there are no more images!
					return( FALSE );
				}
			}
			// We are now at the horizontal wall at the bottom of the current image
			return( GoPastWall( psNewX, psNewY, usWidth, usHeight, pCurrent, sCurrX, sCurrY ) );
		}
	}
}


static BOOLEAN DetermineSubImageSize(UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage)
{
	UINT32		uiOffset;
	UINT8 *		pCurrent;
	INT16			sCurrX = pSubImage->sOffsetX;
	INT16			sCurrY = pSubImage->sOffsetY;

	if (!DetermineOffset( &uiOffset, usWidth, usHeight, sCurrX, sCurrY ))
	{
			return( FALSE );
	}

	// determine width
	pCurrent = p8BPPBuffer + uiOffset;
	do
	{
		sCurrX++;
		pCurrent++;
	} while( *pCurrent != WI && sCurrX < usWidth );
	pSubImage->usWidth = sCurrX - pSubImage->sOffsetX;

	// determine height
	pCurrent = p8BPPBuffer + uiOffset;
	do
	{
		sCurrY++;
		pCurrent += usWidth;
	} while( *pCurrent != WI && sCurrY < usHeight );
	pSubImage->usHeight = sCurrY - pSubImage->sOffsetY;

	return( TRUE );
}


static BOOLEAN CheckForDataInCols(INT16* psXValue, INT16 sXIncrement, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage);
static BOOLEAN CheckForDataInRows(INT16* psYValue, INT16 sYIncrement, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage);


static BOOLEAN DetermineSubImageUsedSize(UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage)
{
	INT16		sNewValue;
	// to do our search loops properly, we can't change the height and width of the
	// subimages until we're done all of our shrinks
	UINT16	usNewHeight;
	UINT16	usNewWidth;
	UINT16	usNewX;
	UINT16	usNewY;

	// shrink from the top
	if (CheckForDataInRows( &sNewValue, 1, p8BPPBuffer, usWidth, usHeight, pSubImage ))
	{
		usNewY = sNewValue;
	}
	else
	{
		return( FALSE );
	}
	// shrink from the bottom
	if (CheckForDataInRows( &sNewValue, -1, p8BPPBuffer, usWidth, usHeight, pSubImage ))
	{
		usNewHeight = (UINT16) sNewValue - usNewY + 1;
	}
	else
	{
		return( FALSE );
	}
	// shrink from the left
	if (CheckForDataInCols( &sNewValue, 1, p8BPPBuffer, usWidth, usHeight, pSubImage ))
	{
		usNewX = sNewValue;
	}
	else
	{
		return( FALSE );
	}
	// shrink from the right
	if (CheckForDataInCols( &sNewValue, -1, p8BPPBuffer, usWidth, usHeight, pSubImage ))
	{
		usNewWidth = (UINT16) sNewValue - usNewX + 1;
	}
	else
	{
		return( FALSE );
	}
	pSubImage->sOffsetX = usNewX;
	pSubImage->sOffsetY = usNewY;
	pSubImage->usHeight = usNewHeight;
	pSubImage->usWidth = usNewWidth;
	return( TRUE );
}


static UINT8* CheckForDataInRowOrColumn(UINT8* pPixel, UINT16 usIncrement, UINT16 usNumberOfPixels);


static BOOLEAN CheckForDataInRows(INT16* psYValue, INT16 sYIncrement, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage)
{
	INT16		sCurrY;
	UINT32	uiOffset;
	UINT8 *	pCurrent;
	UINT16	usLoop;

	if (sYIncrement == 1)
	{
		sCurrY = pSubImage->sOffsetY;
	}
	else if (sYIncrement == -1)
	{
		sCurrY = pSubImage->sOffsetY + (INT16) pSubImage->usHeight - 1;
	}
	else
	{
		// invalid value!
		return( FALSE );
	}
	for (usLoop = 0; usLoop < pSubImage->usHeight; usLoop++)
	{
		if (!DetermineOffset( &uiOffset, usWidth, usHeight, pSubImage->sOffsetX, (INT16) sCurrY))
		{
			return( FALSE );
		}
		pCurrent = p8BPPBuffer + uiOffset;
		pCurrent = CheckForDataInRowOrColumn( pCurrent, 1, pSubImage->usWidth );
		if (pCurrent)
		{
			// non-null data found!
			*psYValue = sCurrY;
			return( TRUE );
		}
		sCurrY += sYIncrement;
	}
	return( FALSE );
}


static BOOLEAN CheckForDataInCols(INT16* psXValue, INT16 sXIncrement, UINT8* p8BPPBuffer, UINT16 usWidth, UINT16 usHeight, STCISubImage* pSubImage)
{
	INT16		sCurrX;
	UINT32	uiOffset;
	UINT8 *	pCurrent;
	UINT16	usLoop;

	if (sXIncrement == 1)
	{
		sCurrX = pSubImage->sOffsetX;
	}
	else if (sXIncrement == -1)
	{
		sCurrX = pSubImage->sOffsetX + (INT16) pSubImage->usWidth - 1;
	}
	else
	{
		// invalid value!
		return( FALSE );
	}
	for (usLoop = 0; usLoop < pSubImage->usWidth; usLoop++)
	{
		if (!DetermineOffset( &uiOffset, usWidth, usHeight, (UINT16) sCurrX, pSubImage->sOffsetY))
		{
			return( FALSE );
		}
		pCurrent = p8BPPBuffer + uiOffset;
		pCurrent = CheckForDataInRowOrColumn( pCurrent, usWidth, pSubImage->usHeight );
		if (pCurrent)
		{
			// non-null data found!
			*psXValue = sCurrX;
			return( TRUE );
		}
		sCurrX += sXIncrement;
	}
	return( FALSE );
}


static UINT8* CheckForDataInRowOrColumn(UINT8* pPixel, UINT16 usIncrement, UINT16 usNumberOfPixels)
{
	// This function, passed the right increment value, can scan either across or
	// down an image to find a non-transparent pixel

	UINT16	usLoop;

	for (usLoop = 0; usLoop < usNumberOfPixels; usLoop++)
	{
		if (*pPixel != TCI)
		{
			return( pPixel );
		}
		else
		{
			pPixel += usIncrement;
		}
	}
	return( NULL );
}
