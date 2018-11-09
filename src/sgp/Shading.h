#ifndef SHADING_H
#define SHADING_H

#include "Types.h"

void BuildShadeTable(void);
void SetShadeTablePercent(float uiShadePercent);

extern float gShadowShadePercent;
extern float gIntensityShadePercent;

extern UINT16 ShadeTable[65536];
extern UINT16 White16BPPPalette[256];

#define DEFAULT_SHADE_LEVEL 4

#endif
