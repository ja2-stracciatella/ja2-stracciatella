#include "BuildDefines.h"

#ifdef JA2EDITOR
#ifndef __EDITOR_MODES_H
#define __EDITOR_MODES_H

void SetEditorItemsTaskbarMode( UINT16 usNewMode );
void SetEditorBuildingTaskbarMode( UINT16 usNewMode );
void SetEditorTerrainTaskbarMode( UINT16 usNewMode );
void SetEditorMapInfoTaskbarMode( UINT16 usNewMode );
void SetEditorSmoothingMode( UINT8 ubNewMode );

void ShowExitGrids();
void HideExitGrids();

#endif
#endif
