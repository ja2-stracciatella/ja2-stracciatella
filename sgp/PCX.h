#ifndef __PCX_
#define __PCX_

#include "Types.h"
#include "HImage.h"


typedef struct
{
  UINT8  ubManufacturer;
  UINT8  ubVersion;
  UINT8  ubEncoding;
  UINT8  ubBitsPerPixel;
  UINT16 usLeft, usTop;
  UINT16 usRight, usBottom;
  UINT16 usHorRez, usVerRez;
  UINT8  ubEgaPalette[48];
  UINT8  ubReserved;
  UINT8  ubColorPlanes;
  UINT16 usBytesPerLine;
  UINT16 usPaletteType;
  UINT8  ubFiller[58];

} PcxHeader;

typedef struct
{
  UINT8  *pPcxBuffer;
  UINT8  ubPalette[768];
  UINT16 usWidth, usHeight;
  UINT32 uiBufferSize;
  UINT16 usPcxFlags;

} PcxObject;

BOOLEAN LoadPCXFileToImage( HIMAGE hImage, UINT16 fContents );

#endif
