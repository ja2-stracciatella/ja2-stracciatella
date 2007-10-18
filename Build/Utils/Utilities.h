#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "HImage.h"
#include "Overhead_Types.h"

BOOLEAN CreateSGPPaletteFromCOLFile( SGPPaletteEntry *pPalette, SGPFILENAME ColFile );
BOOLEAN DisplayPaletteRep(const PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface);

#endif
