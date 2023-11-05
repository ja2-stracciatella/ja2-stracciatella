#ifndef IMGFMT_H
#define IMGFMT_H

// Sir-Tech's Crazy Image (STCI) file format specifications.  Each file is composed of:
// 1		ImageFileHeader, uncompressed
// *		Palette (STCI_INDEXED, size = uiNumberOfColors * PALETTE_ELEMENT_SIZE), uncompressed
// *		SubRectInfo's (usNumberOfRects > 0, size = usNumberOfSubRects * sizeof(SubRectInfo) ), uncompressed
// *		Bytes of image data, possibly compressed

#include "Types.h"

#define STCI_ID_STRING       "STCI"
#define STCI_ID_LEN          4

#define STCI_ETRLE_COMPRESSED		0x0020
#define STCI_ZLIB_COMPRESSED		0x0010
#define STCI_INDEXED						0x0008
#define STCI_RGB								0x0004
#define STCI_ALPHA							0x0002
#define STCI_TRANSPARENT				0x0001

// ETRLE defines
#define COMPRESS_TRANSPARENT				0x80
#define COMPRESS_NON_TRANSPARENT			0x00
#define COMPRESS_RUN_LIMIT					0x7F

// NB if you're going to change the header definition:
// - make sure that everything in this header is nicely aligned
// - don't exceed the 64-byte maximum
struct STCIHeader
{
	UINT8		cID[STCI_ID_LEN];
	UINT32	uiOriginalSize;
	UINT32	uiStoredSize; // equal to uiOriginalSize if data uncompressed
	UINT32	uiTransparentValue;
	UINT32	fFlags;
	UINT16	usHeight;
	UINT16	usWidth;
	union
	{
		struct
		{
			UINT32	uiRedMask;
			UINT32	uiGreenMask;
			UINT32	uiBlueMask;
			UINT32	uiAlphaMask;
			UINT8		ubRedDepth;
			UINT8		ubGreenDepth;
			UINT8		ubBlueDepth;
			UINT8		ubAlphaDepth;
		} RGB;
		struct
		{ // For indexed files, the palette will contain 3 separate bytes for red, green, and blue
			UINT32	uiNumberOfColors;
			UINT16	usNumberOfSubImages;
			UINT8		ubRedDepth;
			UINT8		ubGreenDepth;
			UINT8		ubBlueDepth;
			UINT8		cIndexedUnused[11]; // XXX HACK000B
		} Indexed;
	};
	UINT8		ubDepth;	// size in bits of one pixel as stored in the file
	UINT32	uiAppDataSize;
	UINT8		cUnused[12]; // XXX HACK000B
};


struct STCISubImage
{
	UINT32			uiDataOffset;
	UINT32			uiDataLength;
	INT16				sOffsetX;
	INT16				sOffsetY;
	UINT16			usHeight;
	UINT16			usWidth;
};

struct STCIPaletteElement
{
	UINT8				ubRed;
	UINT8				ubGreen;
	UINT8				ubBlue;
};

#endif
