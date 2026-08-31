#ifndef SHADING_H
#define SHADING_H

#include "Types.h"

inline float guiShadePercent = 0.48f;

void BuildShadeTable(void);
void SetShadeTablePercent(float uiShadePercent);
static inline float GetShadeTablePercent()
{
	return guiShadePercent;
}

extern UINT16 ShadeTable[65536];
extern UINT16 White16BPPPalette[256];

#define DEFAULT_SHADE_LEVEL 4

#endif
