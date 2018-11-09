#ifndef SHADING_H
#define SHADING_H

#include "Types.h"

void BuildShadeTable(void);
void BuildIntensityTable(void);
void SetShadeTablePercent(float uiShadePercent);

extern UINT32 IntensityTable[65536];
extern UINT32 ShadeTable[65536];
extern UINT32 White16BPPPalette[256];

#define DEFAULT_SHADE_LEVEL 4

#endif
