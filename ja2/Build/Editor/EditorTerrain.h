#include "BuildDefines.h"

#ifdef JA2EDITOR
#ifndef __EDITORTERRAIN_H
#define __EDITORTERRAIN_H

#define	TERRAIN_TILES_NODRAW			0
#define TERRAIN_TILES_FOREGROUND	1
#define TERRAIN_TILES_BACKGROUND	2
//Andrew, could you figure out what the hell mode this is???
//It somehow links terrain tiles with lights and buildings.
#define TERRAIN_TILES_BRETS_STRANGEMODE		3

//Soon to be added to an editor struct
extern UINT16 usTotalWeight;
extern BOOLEAN	fPrevShowTerrainTileButtons;
extern BOOLEAN  fUseTerrainWeights;
extern INT32 TerrainTileSelected, TerrainForegroundTile,TerrainBackgroundTile;
extern INT32 TerrainTileDrawMode;

void EntryInitEditorTerrainInfo();
void InitEditorTerrainToolbar();
void CreateTerrainTileButtons();
void DeleteTerrainTileButtons();
void RenderTerrainTileButtons();

void ResetTerrainTileWeights();
void ShowTerrainTileButtons();
void HideTerrainTileButtons();

void ChooseWeightedTerrainTile();

void TerrainFill( UINT32 iMapIndex );

#endif

#endif
