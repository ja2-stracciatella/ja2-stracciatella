#ifndef __STCICONVERT_H
#define __STCICONVERT_H

#include "Types.h"

#define CONVERT_ETRLE_COMPRESS							0x0020
#define CONVERT_TO_8_BIT								0x1000

void WriteSTIFile(UINT8* pData, SGPPaletteEntry* pPalette, INT16 sWidth, INT16 sHeight, const char* cOutputName, UINT32 fFlags, UINT32 uiAppDataSize);

#endif
