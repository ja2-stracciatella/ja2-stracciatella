#ifndef _SHADING_H_
#define _SHADING_H_


#include "himage.h"		// For SGPPaletteEntry
#include "vobject.h"	// For HVOBJECT_SHADE_TABLES
#include "vsurface.h"	// For

#ifdef __cplusplus
extern "C" {
#endif

BOOLEAN ShadesCalculateTables(SGPPaletteEntry *p8BPPPalette);

void BuildShadeTable(void);
void BuildIntensityTable(void);
void SetShadeTablePercent( FLOAT uiShadePercent );

#ifdef JA2	// Jul. 23 '97 - ALEX - because Wizardry isn't using it & no longer has a version of Set8BPPPalette() available
void Init8BitTables(void);
BOOLEAN Set8BitModePalette(SGPPaletteEntry *pPal);
#endif

extern SGPPaletteEntry Shaded8BPPPalettes[HVOBJECT_SHADE_TABLES+3][256];
extern UINT8 ubColorTables[HVOBJECT_SHADE_TABLES+3][256];

extern UINT16	IntensityTable[65536];
extern UINT16	ShadeTable[65536];
extern UINT16	White16BPPPalette[ 256 ];
extern FLOAT   guiShadePercent;
extern FLOAT   guiBrightPercent;

#ifdef __cplusplus
}
#endif

#define DEFAULT_SHADE_LEVEL		4

#endif
