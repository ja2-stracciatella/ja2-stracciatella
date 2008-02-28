#ifndef QUANTIZE_H
#define QUANTIZE_H

#include "Types.h"


#ifdef __cplusplus
extern "C" {
#endif

void QuantizeImage(UINT8* pDest, const SGPPaletteEntry* pSrc, INT16 sWidth, INT16 sHeight, SGPPaletteEntry* pPalette);

#ifdef __cplusplus
}
#endif

#endif
