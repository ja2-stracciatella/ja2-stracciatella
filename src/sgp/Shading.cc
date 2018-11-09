#include "HImage.h"
#include "Shading.h"
#include "VObject.h"

#include <algorithm>
#include <iterator>

UINT32 IntensityTable[65536];
UINT32 ShadeTable[65536];
UINT32 White16BPPPalette[256];
static float guiShadePercent = 0.48f;


/* Builds a 16-bit color shading table. This function should be called only
 * after the current video adapter's pixel format is known (IE:
 * GetRgbDistribution() has been called, and the globals for masks and shifts
 * have been initialized by that function), and before any blitting is done.
 * Using the table is a straight lookup. The pixel to be shaded down is used as
 * the index into the table and the entry at that point will be a pixel that is
 * 25% darker.
 */
void BuildShadeTable(void)
{
	for (UINT16 red = 0; red < 256; red += 4)
	{
		for (UINT16 green = 0; green < 256; green += 4)
		{
			for (UINT16 blue = 0; blue < 256; blue += 4)
			{
				UINT16 index = (red >> 3) << 11 | (green >> 2) << 5 | blue >> 3;
				ShadeTable[index] = RGB(red * guiShadePercent, green * guiShadePercent, blue * guiShadePercent);
			}
		}
	}

	std::fill(std::begin(White16BPPPalette), std::end(White16BPPPalette), UINT16_MAX);
}


/* Builds a 16-bit color shading table. This function should be called only
 * after the current video adapter's pixel format is known (IE:
 * GetRgbDistribution() has been called, and the globals for masks and shifts
 * have been initialized by that function), and before any blitting is done.
 */
void BuildIntensityTable(void)
{
	const float dShadedPercent = 0.80f;

	for (UINT16 red = 0; red < 256; red += 4)
	{
		for (UINT16 green = 0; green < 256; green += 4)
		{
			for (UINT16 blue = 0; blue < 256; blue += 4)
			{
				UINT16 index = (red >> 3) << 11 | (green >> 2) << 5 | blue >> 3;
				IntensityTable[index] = RGB(red * dShadedPercent, green * dShadedPercent, blue * dShadedPercent);
			}
		}
	}
}


void SetShadeTablePercent(float uiShadePercent)
{
	guiShadePercent = uiShadePercent;
	BuildShadeTable();
}
