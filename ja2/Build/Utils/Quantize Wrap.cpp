#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "types.h"
#include "himage.h"
#include "Quantize.h"
#include "Quantize Wrap.h"
#include "phys math.h"

typedef struct
{
	UINT8	r;
	UINT8	g;
	UINT8	b;

} RGBValues;


BOOLEAN	QuantizeImage( UINT8 *pDest, UINT8 *pSrc, INT16 sWidth, INT16 sHeight, SGPPaletteEntry *pPalette )
{
	INT16		sNumColors;

	// FIRST CREATE PALETTE
	CQuantizer		q( 255, 6 );

	q.ProcessImage( pSrc, sWidth, sHeight );

	sNumColors = q.GetColorCount();

	memset( pPalette, 0,  sizeof( SGPPaletteEntry ) * 256 );

	q.GetColorTable( (RGBQUAD*)pPalette );

	// THEN MAP IMAGE TO PALETTE
	// OK, MAPIT!
	MapPalette( pDest, pSrc, sWidth, sHeight, sNumColors, pPalette );

	return( TRUE );
}


void MapPalette( UINT8 *pDest, UINT8 *pSrc, INT16 sWidth, INT16 sHeight, INT16 sNumColors, SGPPaletteEntry *pTable )
{
	INT32 cX, cY, cnt, bBest;
	real					dLowestDist;
	real					dCubeDist;
	vector_3			vTableVal, vSrcVal, vDiffVal;
	UINT8					*pData;
	RGBValues			*pRGBData;

	pRGBData = (RGBValues*)pSrc;

	for ( cX = 0; cX < sWidth; cX++ )
	{
		for ( cY = 0; cY < sHeight; cY++ )
		{
				// OK, FOR EACH PALETTE ENTRY, FIND CLOSEST
				bBest					= 0;
				dLowestDist		= (float)9999999;
				pData					= &(pSrc[ ( cY * sWidth ) + cX ]);

				for ( cnt = 0; cnt < sNumColors; cnt++ )
				{
					vSrcVal.x		= pRGBData[ ( cY * sWidth ) + cX ].r;
					vSrcVal.y		= pRGBData[ ( cY * sWidth ) + cX ].g;
					vSrcVal.z		= pRGBData[ ( cY * sWidth ) + cX ].b;

					vTableVal.x = pTable[ cnt ].peRed;
					vTableVal.y = pTable[ cnt ].peGreen;
					vTableVal.z = pTable[ cnt ].peBlue;

					// Get Dist
					vDiffVal = VSubtract( &vSrcVal, &vTableVal );

					// Get mag dist
					dCubeDist = VGetLength( &(vDiffVal) );

					if ( dCubeDist < dLowestDist )
					{
						dLowestDist = dCubeDist;
						bBest = cnt;
					}					
				}

				// Now we have the lowest value
				// Set into dest
				pData = &(pDest[ ( cY * sWidth ) + cX ]);

				//Set!
				*pData = (UINT8)bBest;
		}
	}

}