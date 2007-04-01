#include "Debug.h"
#include "Shading.h"
#include "VObject.h"
#include <string.h>

UINT16	IntensityTable[65536];
UINT16	ShadeTable[65536];
UINT16	White16BPPPalette[ 256 ];
static FLOAT guiShadePercent = 0.48;


/**********************************************************************************************
 BuildShadeTable

	Builds a 16-bit color shading table. This function should be called only after the current
	video adapter's pixel format is known (IE: GetRgbDistribution() has been called, and the
	globals for masks and shifts have been initialized by that function), and before any
	blitting is done.

	Using the table is a straight lookup. The pixel to be shaded down is used as the index into
	the table and the entry at that point will be a pixel that is 25% darker.

**********************************************************************************************/
void BuildShadeTable(void)
{
	UINT16 red, green, blue;
	UINT16 index;

	for(red=0; red < 256; red+=4)
		for(green=0; green < 256; green+=4)
			for(blue=0; blue < 256; blue+=4)
			{
				index=Get16BPPColor(FROMRGB(red, green, blue));
				ShadeTable[index]=Get16BPPColor(FROMRGB(red*guiShadePercent, green*guiShadePercent, blue*guiShadePercent));
			}

	memset( White16BPPPalette, 65535, sizeof( White16BPPPalette ) );
}


/**********************************************************************************************
 BuildIntensityTable

	Builds a 16-bit color shading table. This function should be called only after the current
	video adapter's pixel format is known (IE: GetRgbDistribution() has been called, and the
	globals for masks and shifts have been initialized by that function), and before any
	blitting is done.



**********************************************************************************************/
void BuildIntensityTable(void)
{
	UINT16 red, green, blue;
	UINT16 index;
	FLOAT  dShadedPercent = (FLOAT)0.80;


#if 0

	UINT32 lumin;
	UINT32 rmod, gmod, bmod;

	for(red=0; red < 256; red+=4)
		for(green=0; green < 256; green+=4)
			for(blue=0; blue < 256; blue+=4)
			{
				index=Get16BPPColor(FROMRGB(red, green, blue));

				lumin=( red*299/1000)+ ( green*587/1000 ) + ( blue*114/1000 );

				//lumin = __min(lumin, 255);
				rmod=(255*lumin)/256;
				gmod=(100*lumin)/256;
				bmod=(100*lumin)/256;

				//rmod = __m( 255, rmod );

				IntensityTable[index]=Get16BPPColor( FROMRGB( rmod, gmod , bmod ) );
			}
#endif



	for(red=0; red < 256; red+=4)
		for(green=0; green < 256; green+=4)
			for(blue=0; blue < 256; blue+=4)
			{
				index=Get16BPPColor(FROMRGB(red, green, blue));
				IntensityTable[index]=Get16BPPColor(FROMRGB(red*dShadedPercent, green*dShadedPercent, blue*dShadedPercent));
			}

}



void SetShadeTablePercent( FLOAT uiShadePercent )
{
	guiShadePercent = uiShadePercent;
	BuildShadeTable( );
}
