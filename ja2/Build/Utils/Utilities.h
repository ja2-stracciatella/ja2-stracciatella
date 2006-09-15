#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "HImage.h"
#include "Overhead_Types.h"
#include "SGP.h"

#define GETPIXELDEPTH( )	( gbPixelDepth )

BOOLEAN CreateSGPPaletteFromCOLFile( SGPPaletteEntry *pPalette, SGPFILENAME ColFile );
BOOLEAN DisplayPaletteRep( PaletteRepID aPalRep, UINT8 ubXPos, UINT8 ubYPos, UINT32 uiDestSurface );

void FilenameForBPP(const char *pFilename, STR pDestination);

BOOLEAN	 WrapString( wchar_t *pStr, wchar_t *pStr2, size_t Length, UINT16 usWidth, INT32 uiFont );

#if 0 /* XXX Seem to be unused */
BOOLEAN IfWinNT(void);
BOOLEAN IfWin95(void);
#endif

void HandleLimitedNumExecutions( );

BOOLEAN HandleJA2CDCheck( );
BOOLEAN HandleJA2CDCheckTwo( );


#endif
