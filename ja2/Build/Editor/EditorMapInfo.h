#include "BuildDefines.h"

#ifdef JA2EDITOR
#ifndef __EDITORMAPINFO_H
#define __EDITORMAPINFO_H

void SetupTextInputForMapInfo();
void UpdateMapInfo();
void ExtractAndUpdateMapInfo();
BOOLEAN ApplyNewExitGridValuesToTextFields();
void UpdateMapInfoFields();

extern SGPPaletteEntry gEditorLightColor;

extern BOOLEAN gfEditorForceShadeTableRebuild;

void LocateNextExitGrid();

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
