#include "TileDat.h"
#include "Types.h"
#include "Editor_Modes.h"
#include "Editor_Taskbar_Utils.h"
#include "EditorDefines.h"
#include "EditorBuildings.h"
#include "EditorTerrain.h"
#include "Cursor_Modes.h"
#include "Map_Information.h"
#include "EditScreen.h"
#include "SelectWin.h"
#include "Environment.h"
#include "WorldDef.h"
#include "Exit_Grids.h"
#include "WorldMan.h"


BOOLEAN gfShowExitGrids = FALSE;

void SetEditorItemsTaskbarMode( UINT16 usNewMode )
{
	UnclickEditorButtons( ITEMS_WEAPONS, ITEMS_KEYS );
	switch( usNewMode )
	{
		case ITEMS_WEAPONS:
			ClickEditorButton( ITEMS_WEAPONS );
			iEditorToolbarState = TBAR_MODE_ITEM_WEAPONS;
			break;
		case ITEMS_AMMO:
			ClickEditorButton( ITEMS_AMMO );
			iEditorToolbarState = TBAR_MODE_ITEM_AMMO;
			break;
		case ITEMS_ARMOUR:
			ClickEditorButton( ITEMS_ARMOUR );
			iEditorToolbarState = TBAR_MODE_ITEM_ARMOUR;
			break;
		case ITEMS_EXPLOSIVES:
			ClickEditorButton( ITEMS_EXPLOSIVES );
			iEditorToolbarState = TBAR_MODE_ITEM_EXPLOSIVES;
			break;
		case ITEMS_EQUIPMENT1:
			ClickEditorButton( ITEMS_EQUIPMENT1 );
			iEditorToolbarState = TBAR_MODE_ITEM_EQUIPMENT1;
			break;
		case ITEMS_EQUIPMENT2:
			ClickEditorButton( ITEMS_EQUIPMENT2 );
			iEditorToolbarState = TBAR_MODE_ITEM_EQUIPMENT2;
			break;
		case ITEMS_EQUIPMENT3:
			ClickEditorButton( ITEMS_EQUIPMENT3 );
			iEditorToolbarState = TBAR_MODE_ITEM_EQUIPMENT3;
			break;
		case ITEMS_TRIGGERS:
			ClickEditorButton( ITEMS_TRIGGERS );
			iEditorToolbarState = TBAR_MODE_ITEM_TRIGGERS;
			break;
		case ITEMS_KEYS:
			ClickEditorButton( ITEMS_KEYS );
			iEditorToolbarState = TBAR_MODE_ITEM_KEYS;
			break;
	}
}

#define NO_EFFECT			2

void SetEditorBuildingTaskbarMode( UINT16 usNewMode )
{
	BOOLEAN fNewRoofs, fNewWalls, fNewRoomInfo;
	if( usNewMode == usCurrentMode )
	{
		ClickEditorButton( usNewMode );
		return;
	}
	usCurrentMode = usNewMode;
	//Unclick all of the building section buttons first -- except the view modes.
	UnclickEditorButton( BUILDING_PLACE_WALLS );
	UnclickEditorButton( BUILDING_PLACE_DOORS );
	UnclickEditorButton( BUILDING_PLACE_WINDOWS );
	UnclickEditorButton( BUILDING_PLACE_ROOFS );
	UnclickEditorButton( BUILDING_PLACE_BROKEN_WALLS );
	UnclickEditorButton( BUILDING_PLACE_FURNITURE );
	UnclickEditorButton( BUILDING_PLACE_DECALS );
	UnclickEditorButton( BUILDING_PLACE_FLOORS );
	UnclickEditorButton( BUILDING_PLACE_TOILETS );
	UnclickEditorButton( BUILDING_SMART_WALLS );
	UnclickEditorButton( BUILDING_SMART_DOORS );
	UnclickEditorButton( BUILDING_SMART_WINDOWS );
	UnclickEditorButton( BUILDING_SMART_BROKEN_WALLS );
	UnclickEditorButton( BUILDING_DOORKEY );
	UnclickEditorButton( BUILDING_NEW_ROOM );
	UnclickEditorButton( BUILDING_NEW_ROOF );
	UnclickEditorButton( BUILDING_CAVE_DRAWING );
	UnclickEditorButton( BUILDING_SAW_ROOM );
	UnclickEditorButton( BUILDING_KILL_BUILDING );
	UnclickEditorButton( BUILDING_COPY_BUILDING );
	UnclickEditorButton( BUILDING_MOVE_BUILDING );
	UnclickEditorButton( BUILDING_DRAW_ROOMNUM );
	ClickEditorButton( usNewMode );

	gfRenderDrawingMode = TRUE;

	//Clicking on certain buttons will automatically activate/deactive certain views.
	switch( usNewMode )
	{
		case BUILDING_KILL_BUILDING://Show everything
			fNewWalls = TRUE;
			fNewRoofs = TRUE;
			fNewRoomInfo = TRUE;
			break;
		case BUILDING_NEW_ROOF:
		case BUILDING_PLACE_ROOFS:
			fNewWalls = TRUE;
			fNewRoofs = TRUE;
			fNewRoomInfo = FALSE;
			break;
		case BUILDING_DRAW_ROOMNUM: //Show room info
		case BUILDING_ERASE_ROOMNUM: //Show room info
			fNewWalls = NO_EFFECT;
			fNewRoofs = gfBasement ? TRUE : FALSE;
			fNewRoomInfo = TRUE;
			break;
		case BUILDING_PLACE_DOORS:
		case BUILDING_PLACE_WINDOWS:
		case BUILDING_PLACE_WALLS:
		case BUILDING_PLACE_BROKEN_WALLS:
		case BUILDING_PLACE_FLOORS:
		case BUILDING_PLACE_TOILETS:
		case BUILDING_PLACE_FURNITURE:
		case BUILDING_PLACE_DECALS:
		case BUILDING_SMART_WALLS:
		case BUILDING_SMART_DOORS:
		case BUILDING_SMART_WINDOWS:
		case BUILDING_SMART_BROKEN_WALLS:
		case BUILDING_DOORKEY:
		case BUILDING_SAW_ROOM:
		case BUILDING_NEW_ROOM:
		case BUILDING_COPY_BUILDING:
		case BUILDING_MOVE_BUILDING:
		case BUILDING_CAVE_DRAWING:
			fNewRoofs = gfBasement ? TRUE : FALSE;
			fNewWalls =	TRUE;
			fNewRoomInfo = FALSE;
			if( usNewMode == BUILDING_PLACE_FLOORS )
				gusSelectionType = gusSavedSelectionType;
			break;
		default:
			return;
	}
	UnclickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
	if( fNewWalls != NO_EFFECT && fNewWalls != fBuildingShowWalls )
	{
		if( fNewWalls )
			ClickEditorButton( BUILDING_TOGGLE_WALL_VIEW );
		else
			UnclickEditorButton( BUILDING_TOGGLE_WALL_VIEW );
		fBuildingShowWalls = fNewWalls;
		UpdateWallsView();
	}
	if( fNewRoofs != NO_EFFECT && fNewRoofs != fBuildingShowRoofs )
	{
		if( fNewRoofs )
			ClickEditorButton( BUILDING_TOGGLE_ROOF_VIEW );
		else
			UnclickEditorButton( BUILDING_TOGGLE_ROOF_VIEW );
		fBuildingShowRoofs = fNewRoofs;
		UpdateRoofsView();
	}
	if( fNewRoomInfo != NO_EFFECT && fNewRoomInfo != fBuildingShowRoomInfo )
	{
		if( fNewRoomInfo )
			ClickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
		else
			UnclickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
		fBuildingShowRoomInfo = fNewRoomInfo;
		gfRenderWorld = TRUE;
	}
}

void SetEditorTerrainTaskbarMode( UINT16 usNewMode )
{
	UnclickEditorButton( TERRAIN_FGROUND_TEXTURES );
	UnclickEditorButton( TERRAIN_BGROUND_TEXTURES );
	UnclickEditorButton( TERRAIN_PLACE_CLIFFS );
	UnclickEditorButton( TERRAIN_PLACE_DEBRIS );
	UnclickEditorButton( TERRAIN_PLACE_TREES );
	UnclickEditorButton( TERRAIN_PLACE_ROCKS );
	UnclickEditorButton( TERRAIN_PLACE_MISC );
	UnclickEditorButton( TERRAIN_FILL_AREA );
	TerrainTileDrawMode=0;

	gfRenderDrawingMode = TRUE;

	switch( usNewMode )
	{
		case TERRAIN_FGROUND_TEXTURES:
			TerrainTileDrawMode = TERRAIN_TILES_FOREGROUND;
			ClickEditorButton( TERRAIN_FGROUND_TEXTURES );
			iDrawMode = DRAW_MODE_GROUND;
			gusSelectionType = gusSavedSelectionType;
			break;
		case TERRAIN_BGROUND_TEXTURES:
			TerrainTileDrawMode = TERRAIN_TILES_BACKGROUND;
			ClickEditorButton( TERRAIN_BGROUND_TEXTURES );
			iDrawMode = DRAW_MODE_NEW_GROUND;
			break;
		case TERRAIN_PLACE_CLIFFS:
			ClickEditorButton( TERRAIN_PLACE_CLIFFS );
			iDrawMode = DRAW_MODE_BANKS;
			break;
		case TERRAIN_PLACE_ROADS:
			ClickEditorButton( TERRAIN_PLACE_ROADS );
			iDrawMode = DRAW_MODE_ROADS;
			break;
		case TERRAIN_PLACE_DEBRIS:
			ClickEditorButton( TERRAIN_PLACE_DEBRIS );
			iDrawMode = DRAW_MODE_DEBRIS;
			gusSelectionType = gusSavedSelectionType;
			break;
		case TERRAIN_PLACE_TREES:
			ClickEditorButton( TERRAIN_PLACE_TREES );
			iDrawMode = DRAW_MODE_OSTRUCTS;
			gusSelectionType = gusSavedSelectionType;
			break;
		case TERRAIN_PLACE_ROCKS:
			ClickEditorButton( TERRAIN_PLACE_ROCKS );
			iDrawMode = DRAW_MODE_OSTRUCTS1;
			gusSelectionType = gusSavedSelectionType;
			break;
		case TERRAIN_PLACE_MISC:
			ClickEditorButton( TERRAIN_PLACE_MISC );
			iDrawMode = DRAW_MODE_OSTRUCTS2;
			gusSelectionType = gusSavedSelectionType;
			break;
		case TERRAIN_FILL_AREA:
			ClickEditorButton( TERRAIN_FILL_AREA );
			iDrawMode = DRAW_MODE_FILL_AREA + DRAW_MODE_GROUND;
			TerrainTileDrawMode=1;
			break;
	}
}


static void ShowExitGrids(void)
{
	UINT16 i;
	if( gfShowExitGrids )
		return;
	gfShowExitGrids = TRUE;
	for( i = 0; i < WORLD_MAX; i++ )
	{
		if (ExitGridAtGridNo(i))
		{
			AddTopmostToTail( i, FIRSTPOINTERS8 );
		}
	}
}

void HideExitGrids()
{
	UINT16 i;
	if( !gfShowExitGrids )
		return;
	gfShowExitGrids = FALSE;
	for( i = 0; i < WORLD_MAX; i++ )
	{
		if (ExitGridAtGridNo(i))
		{
			LEVELNODE* pLevelNode = gpWorldLevelData[i].pTopmostHead;
			while( pLevelNode )
			{
				if( pLevelNode->usIndex == FIRSTPOINTERS8 )
				{
					RemoveTopmost( i, pLevelNode->usIndex );
					break;
				}
				pLevelNode = pLevelNode->pNext;
			}
		}
	}
}

void SetEditorMapInfoTaskbarMode( UINT16 usNewMode )
{
	BOOLEAN fShowExitGrids = FALSE;
	UnclickEditorButton( MAPINFO_ADD_LIGHT1_SOURCE );
	UnclickEditorButton( MAPINFO_DRAW_EXITGRIDS );
	UnclickEditorButton( MAPINFO_NORTH_POINT );
	UnclickEditorButton( MAPINFO_WEST_POINT );
	UnclickEditorButton( MAPINFO_CENTER_POINT );
	UnclickEditorButton( MAPINFO_EAST_POINT );
	UnclickEditorButton( MAPINFO_SOUTH_POINT );
	UnclickEditorButton( MAPINFO_ISOLATED_POINT );
	ClickEditorButton( usNewMode );
	switch( usNewMode )
	{
		case MAPINFO_ADD_LIGHT1_SOURCE:
			iDrawMode = DRAW_MODE_LIGHT;
			break;
		case MAPINFO_DRAW_EXITGRIDS:
			iDrawMode = DRAW_MODE_EXITGRID;
			gusSelectionType = gusSavedSelectionType;
			fShowExitGrids = TRUE;
			break;
		case MAPINFO_NORTH_POINT:
			iDrawMode = DRAW_MODE_NORTHPOINT;
			break;
		case MAPINFO_WEST_POINT:
			iDrawMode = DRAW_MODE_WESTPOINT;
			break;
		case MAPINFO_EAST_POINT:
			iDrawMode = DRAW_MODE_EASTPOINT;
			break;
		case MAPINFO_SOUTH_POINT:
			iDrawMode = DRAW_MODE_SOUTHPOINT;
			break;
		case MAPINFO_CENTER_POINT:
			iDrawMode = DRAW_MODE_CENTERPOINT;
			break;
		case MAPINFO_ISOLATED_POINT:
			iDrawMode = DRAW_MODE_ISOLATEDPOINT;
			break;
		case MAPINFO_RADIO_NORMAL:
			SetEditorSmoothingMode( SMOOTHING_NORMAL );
			gfRenderTaskbar = TRUE;
			break;
		case MAPINFO_RADIO_BASEMENT:
			SetEditorSmoothingMode( SMOOTHING_BASEMENT );
			gfRenderTaskbar = TRUE;
			break;
		case MAPINFO_RADIO_CAVES:
			SetEditorSmoothingMode( SMOOTHING_CAVES );
			gfRenderTaskbar = TRUE;
			break;
	}
	if( fShowExitGrids )
	{
		ShowExitGrids();
	}
	else
	{
		HideExitGrids();
	}
}


void SetEditorSmoothingMode( UINT8 ubNewMode )
{
	UnclickEditorButtons( MAPINFO_RADIO_NORMAL, MAPINFO_RADIO_CAVES );
	if( iCurrentTaskbar == TASK_BUILDINGS )
		HideEditorButtons( BUILDING_NEW_ROOM, BUILDING_CAVE_DRAWING );
	gfBasement = FALSE;
	gfCaves = FALSE;
	gMapInformation.ubEditorSmoothingType = SMOOTHING_NORMAL;
	switch( ubNewMode )
	{
		case SMOOTHING_NORMAL:
			ClickEditorButton( MAPINFO_RADIO_NORMAL );
			if( iCurrentTaskbar == TASK_BUILDINGS )
				ShowEditorButtons( BUILDING_NEW_ROOM, BUILDING_MOVE_BUILDING );
			EnableEditorButtons( BUILDING_SMART_WALLS, BUILDING_SMART_BROKEN_WALLS );
			break;
		case SMOOTHING_BASEMENT:
			gfBasement = TRUE;
			ClickEditorButton( MAPINFO_RADIO_BASEMENT );
			if( iCurrentTaskbar == TASK_BUILDINGS )
				ShowEditorButtons( BUILDING_NEW_ROOM, BUILDING_KILL_BUILDING );
			EnableEditorButtons( BUILDING_SMART_WALLS, BUILDING_SMART_BROKEN_WALLS );
			break;
		case SMOOTHING_CAVES:
			gfCaves = TRUE;
			ClickEditorButton( MAPINFO_RADIO_CAVES );
			if( iCurrentTaskbar == TASK_BUILDINGS )
				ShowEditorButton( BUILDING_CAVE_DRAWING );
			DisableEditorButtons( BUILDING_SMART_WALLS, BUILDING_SMART_BROKEN_WALLS );
			break;
		default:
			SLOGA("Attempting to set an illegal smoothing mode." );
			break;
	}
	gMapInformation.ubEditorSmoothingType = ubNewMode;
}

