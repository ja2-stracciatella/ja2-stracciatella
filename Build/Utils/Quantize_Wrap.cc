#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "HImage.h"
#include "Types.h"
#include "Quantize.h"
#include "Quantize_Wrap.h"


static void MapPalette(UINT8* const pDest, const SGPPaletteEntry* const pSrc, const INT16 sWidth, const INT16 sHeight, const INT16 sNumColors, const SGPPaletteEntry* pTable);


BOOLEAN QuantizeImage(UINT8* const pDest, const SGPPaletteEntry* const pSrc, const INT16 sWidth, const INT16 sHeight, SGPPaletteEntry* const pPalette)
{
	INT16		sNumColors;

	// FIRST CREATE PALETTE
	CQuantizer		q( 255, 6 );

	q.ProcessImage(pSrc, sWidth, sHeight);

	sNumColors = q.GetColorCount();

	memset( pPalette, 0,  sizeof( SGPPaletteEntry ) * 256 );

	q.GetColorTable(pPalette);

	// THEN MAP IMAGE TO PALETTE
	// OK, MAPIT!
	MapPalette( pDest, pSrc, sWidth, sHeight, sNumColors, pPalette );

	return( TRUE );
}


static void MapPalette(UINT8* const pDest, const SGPPaletteEntry* const pSrc, const INT16 sWidth, const INT16 sHeight, const INT16 sNumColors, const SGPPaletteEntry* pTable)
{
	for (INT32 cX = 0; cX < sWidth; ++cX)
	{
		for (INT32 cY = 0; cY < sHeight; ++cY)
		{
			// OK, FOR EACH PALETTE ENTRY, FIND CLOSEST
			INT32  best        = 0;
			UINT32 lowest_dist = 9999999;
			for (INT32 cnt = 0; cnt < sNumColors; ++cnt)
			{
				const SGPPaletteEntry* const a = &pSrc[cY * sWidth + cX];
				const SGPPaletteEntry* const b = &pTable[cnt];
				const INT32  dr   = a->peRed   - b->peRed;
				const INT32  dg   = a->peGreen - b->peGreen;
				const INT32  db   = a->peBlue  - b->peBlue;
				const UINT32 dist = dr * dr + dg * dg + db * db;
				if (dist < lowest_dist)
				{
					lowest_dist = dist;
					best        = cnt;
				}
			}

			// Now we have the lowest value
			// Set into dest
			pDest[cY * sWidth + cX] = best;
		}
	}
}
