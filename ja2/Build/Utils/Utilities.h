#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "HImage.h"
#include "Overhead_Types.h"

#define GETPIXELDEPTH( )	( gbPixelDepth )

BOOLEAN CreateSGPPaletteFromCOLFile( SGPPaletteEntry *pPalette, SGPFILENAME ColFile );
BOOLEAN DisplayPaletteRep( PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface );

void FilenameForBPP(const char *pFilename, STR pDestination);

BOOLEAN	 WrapString( wchar_t *pStr, wchar_t *pStr2, size_t Length, UINT16 usWidth, INT32 uiFont );

#endif
