//**************************************************************************
//
// Filename :	bitmap.h
//
//	Purpose :	bitmap format
//
// Modification history :
//
//		20nov96:HJH				- Creation
//
//**************************************************************************

#ifndef _bitmap_h
#define _bitmap_h

//**************************************************************************
//
//				Includes
//
//**************************************************************************

#include "types.h"

//**************************************************************************
//
//				Defines
//
//**************************************************************************

//**************************************************************************
//
//				Typedefs
//
//**************************************************************************

typedef struct sgpBmHeadertag
{
	UINT32	uiNumBytes;			//	number of bytes of the bitmap, including the
										// memory for all variables in this structure plus
										// all the memory in bitmap
	UINT32	uiWidth;				// width of bitmap in pixels
	UINT32	uiHeight;			// height of bitmap in pixels
	UINT8		uiBitDepth;			// 8, 16, 24, or 32
	UINT8		uiNumPalEntries;	// if uiBitDepth is 8, non-zero, else 0
} SGPBmHeader;

typedef struct sgpBitmaptag
{
	SGPBmHeader	header;
	UINT8			uiData[1];	// if uiNumPalEntries != 0
									//		uiNumPalEntries*3 (rgb) bytes for palette
									// if uiBitDepth == 8
									//		uiWidth * uiHeight bytes
									// else if uiBitDepth == 16
									//		uiWidth * uiHeight * 2 bytes
									// else if uiBitDepth == 24
									//		uiWidth * uiHeight * 3 bytes
									// else if uiBitDepth == 32
									//		uiWidth * uiHeight * 4 bytes
} SGPBitmap;

#endif

