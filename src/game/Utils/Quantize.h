#ifndef QUANTIZE_H
#define QUANTIZE_H

#include "Types.h"


void QuantizeImage(UINT8* pDest, const SGPPaletteEntry* pSrc, INT16 sWidth, INT16 sHeight, SGPPaletteEntry* pPalette);

#endif
