#include "BuildDefines.h"

#ifdef JA2EDITOR
#ifndef __EDITORMAPINFO_H
#define __EDITORMAPINFO_H

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
INT8 gbDefaultLightType;

#endif
#endif
