#ifndef __QUANTIZE_WRAP_H
#define __QUANTIZE_WRAP_H


#ifdef __cplusplus
extern "C" {
#endif



BOOLEAN	QuantizeImage( UINT8 *pDest, UINT8 *pSrc, INT16 sWidth, INT16 sHeight, SGPPaletteEntry *pPalette );
void MapPalette( UINT8 *pDest, UINT8 *pSrc, INT16 sWidth, INT16 sHeight, INT16 sNumColors, SGPPaletteEntry *pTable );


#ifdef __cplusplus
}
#endif



#endif
