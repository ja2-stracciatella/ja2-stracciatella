#ifndef __IMAGE_H
#define __IMAGE_H

// The HIMAGE module provides a common interface for managing image data. This module
// includes:
// - A set of data structures representing image data. Data can be 8 or 16 bpp and/or
//   compressed
// - A set of file loaders which load specific file formats into the internal data format
// - A set of blitters which blt the data to memory
// - A comprehensive automatic blitter which blits the appropriate type based on the
//   image header.


// Defines for type of file readers
#define PCX_FILE_READER					0x1
#define TGA_FILE_READER					0x2
#define STCI_FILE_READER				0x4
#define TRLE_FILE_READER				0x8
#define UNKNOWN_FILE_READER			0x200

// Defines for buffer bit depth
#define BUFFER_8BPP							0x1
#define BUFFER_16BPP						0x2

// Defines for image charactoristics
#define IMAGE_COMPRESSED				0x0001
#define IMAGE_TRLECOMPRESSED		0x0002
#define IMAGE_PALETTE						0x0004
#define IMAGE_BITMAPDATA				0x0008
#define IMAGE_APPDATA						0x0010
#define IMAGE_ALLIMAGEDATA			0x000C
#define IMAGE_ALLDATA						0x001C

// Palette structure, mimics that of Win32
typedef struct tagSGPPaletteEntry
{
  UINT8 peRed;
  UINT8 peGreen;
  UINT8 peBlue;
  UINT8 peFlags;

} SGPPaletteEntry;

#define AUX_FULL_TILE					0x01
#define AUX_ANIMATED_TILE			0x02
#define AUX_DYNAMIC_TILE			0x04
#define AUX_INTERACTIVE_TILE	0x08
#define AUX_IGNORES_HEIGHT		0x10
#define AUX_USES_LAND_Z				0x20

typedef struct
{
	UINT8		ubWallOrientation;
	UINT8		ubNumberOfTiles;
	UINT16	usTileLocIndex;
	UINT8		ubUnused1[3];
	UINT8		ubCurrentFrame;
	UINT8		ubNumberOfFrames;
	UINT8		fFlags;
	UINT8		ubUnused[6];
} AuxObjectData;

typedef struct
{
	INT8		bTileOffsetX;
	INT8		bTileOffsetY;
} RelTileLoc; // relative tile location

// TRLE subimage structure, mirroring that of ST(C)I
typedef struct tagETRLEObject
{
	UINT32			uiDataOffset;
	UINT32			uiDataLength;
	INT16				sOffsetX;
	INT16				sOffsetY;
	UINT16			usHeight;
	UINT16			usWidth;
} ETRLEObject;

typedef struct tagETRLEData
{
	PTR								pPixData;
	UINT32						uiSizePixData;
	ETRLEObject *			pETRLEObject;
	UINT16						usNumberOfObjects;
} ETRLEData;

// Image header structure
typedef struct
{
	UINT16					usWidth;
	UINT16					usHeight;
	UINT8						ubBitDepth;
	UINT16					fFlags;
	SGPFILENAME			ImageFile;
	SGPPaletteEntry *pPalette;
	UINT16					*pui16BPPPalette;
	UINT8 *					pAppData;
	UINT32					uiAppDataSize;
	// This union is used to describe each data type and is flexible to include the
	// data strucutre of the compresssed format, once developed.
	union
	{
		struct
		{
			PTR		pImageData;
		};
		struct
		{
			PTR		pCompressedImageData;
		};
		struct
		{
			UINT8						*p8BPPData;
		};
		struct
		{

			UINT16 *p16BPPData;
		};
		struct
		{
			UINT8 *					pPixData8;
			UINT32					uiSizePixData;
			ETRLEObject *		pETRLEObject;
			UINT16					usNumberOfObjects;
		};
	};

} image_type, *HIMAGE;


#define SGPGetRValue(rgb)   ((BYTE) (rgb))
#define SGPGetBValue(rgb)   ((BYTE) ((rgb) >> 16))
#define SGPGetGValue(rgb)   ((BYTE) (((UINT16) (rgb)) >> 8))


#ifdef __cplusplus
extern "C" {
#endif

// This function will return NULL if it fails, and call SetLastError() to set
// error information
HIMAGE CreateImage(const char* ImageFile, UINT16 fContents);

// This function destroys the HIMAGE structure as well as its contents
BOOLEAN DestroyImage( HIMAGE hImage );

// This function will run the appropriate copy function based on the type of HIMAGE object
BOOLEAN CopyImageToBuffer( HIMAGE hImage, UINT32 fBufferType, BYTE *pDestBuf, UINT16 usDestWidth, UINT16 usDestHeight, UINT16 usX, UINT16 usY, SGPRect *srcRect );


// This function will create a buffer in memory of ETRLE data, excluding palette
BOOLEAN GetETRLEImageData( HIMAGE hImage, ETRLEData *pBuffer );

// UTILITY FUNCTIONS

// Used to create a 16BPP Palette from an 8 bit palette, found in himage.c
UINT16 *Create16BPPPaletteShaded( SGPPaletteEntry *pPalette, UINT32 rscale, UINT32 gscale, UINT32 bscale, BOOLEAN mono);
UINT16* Create16BPPPalette(const SGPPaletteEntry* pPalette);
UINT16 Get16BPPColor( UINT32 RGBValue );
UINT32 GetRGBColor( UINT16 Value16BPP );

extern UINT16 gusRedMask;
extern UINT16 gusGreenMask;
extern UINT16 gusBlueMask;
extern INT16  gusRedShift;
extern INT16  gusBlueShift;
extern INT16  gusGreenShift;

// used to convert 565 RGB data into different bit-formats
void ConvertRGBDistribution565To555( UINT16 * p16BPPData, UINT32 uiNumberOfPixels );
void ConvertRGBDistribution565To655( UINT16 * p16BPPData, UINT32 uiNumberOfPixels );
void ConvertRGBDistribution565To556( UINT16 * p16BPPData, UINT32 uiNumberOfPixels );
void ConvertRGBDistribution565ToAny( UINT16 * p16BPPData, UINT32 uiNumberOfPixels );

#ifdef __cplusplus
}
#endif

#endif
