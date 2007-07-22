#ifndef SHADING_H
#define SHADING_H

#ifdef __cplusplus
extern "C" {
#endif

void BuildShadeTable(void);
void BuildIntensityTable(void);
void SetShadeTablePercent(float uiShadePercent);

extern UINT16 IntensityTable[65536];
extern UINT16 ShadeTable[65536];
extern UINT16 White16BPPPalette[256];

#ifdef __cplusplus
}
#endif

#define DEFAULT_SHADE_LEVEL 4

#endif
