#ifndef __QUANTIZE_WRAP_H
#define __QUANTIZE_WRAP_H

#include "HImage.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN	QuantizeImage( UINT8 *pDest, UINT8 *pSrc, INT16 sWidth, INT16 sHeight, SGPPaletteEntry *pPalette );

#ifdef __cplusplus
}
#endif

#endif
