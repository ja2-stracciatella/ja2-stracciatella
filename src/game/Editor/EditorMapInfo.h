#ifndef __EDITORMAPINFO_H
#define __EDITORMAPINFO_H

#include "Types.h"

void SetupTextInputForMapInfo(void);
void UpdateMapInfo(void);
void ExtractAndUpdateMapInfo(void);
BOOLEAN ApplyNewExitGridValuesToTextFields(void);
void UpdateMapInfoFields(void);

extern SGPPaletteEntry gEditorLightColor;

extern BOOLEAN gfEditorForceShadeTableRebuild;

void LocateNextExitGrid(void);

enum
{
	PRIMETIME_LIGHT,
	NIGHTTIME_LIGHT,
	ALWAYSON_LIGHT
};
void ChangeLightDefault( INT8 bLightType );
extern INT8 gbDefaultLightType;

#endif

