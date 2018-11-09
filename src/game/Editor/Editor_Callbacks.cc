#include "Types.h"
#include "Button_System.h"
#include "EditorDefines.h"
#include "Editor_Taskbar_Utils.h"
#include "EditorItems.h"
#include "Editor_Callback_Prototypes.h"
#include "Editor_Modes.h"
#include "EditorBuildings.h"
#include "EditorMercs.h"
#include "Item_Statistics.h"
#include "Cursor_Modes.h"
#include "PopupMenu.h"
#include "Simple_Render_Utils.h"
#include "Soldier_Init_List.h"
#include "Scheduling.h"
#include "Input.h"
#include "Map_Information.h"
#include "EditorMapInfo.h"


extern SOLDIERINITNODE *gpSelected;
extern SCHEDULENODE gCurrSchedule;


//TERRAIN
void BtnFgGrndCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_FGROUND_TEXTURES );
	}
}

void BtnBkGrndCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_BGROUND_TEXTURES );
	}
}

void BtnObjectCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_TREES );
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_TREES );
		iEditorToolbarState = TBAR_MODE_GET_OSTRUCTS;
	}
}

void BtnObject1Callback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_ROCKS );
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_ROCKS );
		iEditorToolbarState = TBAR_MODE_GET_OSTRUCTS1;
	}
}


void BtnObject2Callback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_MISC );
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_MISC );
		iEditorToolbarState = TBAR_MODE_GET_OSTRUCTS2;
	}
}


void BtnBanksCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_CLIFFS );
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_CLIFFS );
		iEditorToolbarState = TBAR_MODE_GET_BANKS;
	}
}

void BtnRoadsCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_ROADS );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_ROADS );
		iEditorToolbarState = TBAR_MODE_GET_ROADS;
	}
}

void BtnDebrisCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_DEBRIS );
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_PLACE_DEBRIS );
		iEditorToolbarState = TBAR_MODE_GET_DEBRIS;
	}
}

void BtnBrushCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_CHANGE_BRUSH;
	}
}

void BtnFillCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorTerrainTaskbarMode( TERRAIN_FILL_AREA );
	}
}


void BtnIncBrushDensityCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_DENS_UP;
	}
}


void BtnDecBrushDensityCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_DENS_DWN;
	}
}

//BUILDINGS
void BuildingWallCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_WALLS );
		iDrawMode = DRAW_MODE_WALLS;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_WALLS );
		iEditorToolbarState = TBAR_MODE_GET_WALL;
	}
}

void BuildingDoorCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_DOORS );
		iDrawMode = DRAW_MODE_DOORS;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_DOORS );
		iEditorToolbarState = TBAR_MODE_GET_DOOR;
	}
}

void BuildingWindowCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_WINDOWS );
		iDrawMode = DRAW_MODE_WINDOWS;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_WINDOWS );
		iEditorToolbarState = TBAR_MODE_GET_WINDOW;
	}
}

void BuildingRoofCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_ROOFS );
		iDrawMode = DRAW_MODE_ROOFS;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_ROOFS );
		iEditorToolbarState = TBAR_MODE_GET_ROOF;
	}
}

void BuildingCrackWallCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_BROKEN_WALLS );
		iDrawMode = DRAW_MODE_BROKEN_WALLS;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_BROKEN_WALLS );
		iEditorToolbarState = TBAR_MODE_GET_BROKEN_WALL;
	}
}

void BuildingFloorCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_FLOORS );
		iDrawMode = DRAW_MODE_FLOORS;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_FLOORS );
		iEditorToolbarState = TBAR_MODE_GET_FLOOR;
	}
}

void BuildingToiletCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_TOILETS );
		iDrawMode = DRAW_MODE_TOILET;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_TOILETS );
		iEditorToolbarState = TBAR_MODE_GET_TOILET;
	}
}

void BuildingFurnitureCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_FURNITURE );
		iDrawMode = DRAW_MODE_DECOR;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_FURNITURE );
		iEditorToolbarState = TBAR_MODE_GET_DECOR;
	}
}

void BuildingDecalCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_DECALS );
		iDrawMode = DRAW_MODE_DECALS;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_PLACE_DECALS );
		iEditorToolbarState = TBAR_MODE_GET_DECAL;
	}
}

void BuildingSmartWallCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_SMART_WALLS );
		iDrawMode = DRAW_MODE_SMART_WALLS;
	}
}

void BuildingSmartWindowCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_SMART_WINDOWS );
		iDrawMode = DRAW_MODE_SMART_WINDOWS;
	}
}

void BuildingSmartDoorCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_SMART_DOORS );
		iDrawMode = DRAW_MODE_SMART_DOORS;
	}
}

void BuildingSmartCrackWallCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_SMART_BROKEN_WALLS );
		iDrawMode = DRAW_MODE_SMART_BROKEN_WALLS;
	}
}

void BuildingDoorKeyCallback(GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_DOORKEY );
		iDrawMode = DRAW_MODE_DOORKEYS;
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_DOORKEY );
		iDrawMode = DRAW_MODE_DOORKEYS;
		FindNextLockedDoor();
	}
}

void BuildingNewRoomCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_NEW_ROOM );
		gusSelectionType = gusSavedBuildingSelectionType;
		iDrawMode = DRAW_MODE_ROOM;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		SetEditorBuildingTaskbarMode( BUILDING_NEW_ROOM );
		gusSelectionType = gusSavedBuildingSelectionType;
		iDrawMode = DRAW_MODE_ROOM;
		iEditorToolbarState = TBAR_MODE_GET_ROOM;
	}
}

void BuildingNewRoofCallback(GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_NEW_ROOF );
		iDrawMode = DRAW_MODE_NEWROOF;
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		SetEditorBuildingTaskbarMode( BUILDING_NEW_ROOF );
		iDrawMode = DRAW_MODE_NEWROOF;
		iEditorToolbarState = TBAR_MODE_GET_NEW_ROOF;
	}
}

void BuildingCaveDrawingCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_CAVE_DRAWING );
		gusSelectionType = gusSavedBuildingSelectionType;
		iDrawMode = DRAW_MODE_CAVES;
	}
}

void BuildingSawRoomCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_SAW_ROOM );
		gusSelectionType = gusSavedBuildingSelectionType;
		iDrawMode = DRAW_MODE_SAW_ROOM;
	}
}


void BuildingKillBuildingCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_KILL_BUILDING );
		iDrawMode = DRAW_MODE_KILL_BUILDING;
	}
}

void BuildingCopyBuildingCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_COPY_BUILDING );
		iDrawMode = DRAW_MODE_COPY_BUILDING;
	}
}

void BuildingMoveBuildingCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_MOVE_BUILDING );
		iDrawMode = DRAW_MODE_MOVE_BUILDING;
	}
}

void BuildingDrawRoomNumCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SetEditorBuildingTaskbarMode( BUILDING_DRAW_ROOMNUM );
		iDrawMode = DRAW_MODE_ROOMNUM;
		gubCurrRoomNumber = gubMaxRoomNumber;
	}
}

void BuildingEraseRoomNumCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason &MSYS_CALLBACK_REASON_POINTER_UP )
	{
		SetEditorBuildingTaskbarMode( BUILDING_ERASE_ROOMNUM );
		iDrawMode = DRAW_MODE_ROOMNUM + DRAW_MODE_ERASE;
	}
}

void BuildingToggleRoofViewCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( fBuildingShowRoofs ^= 1 )
		{
			ClickEditorButton( BUILDING_TOGGLE_ROOF_VIEW );
		}
		else
		{
			UnclickEditorButton( BUILDING_TOGGLE_ROOF_VIEW );
		}
		//Name could be a bit misleading.  It'll hide or show the roofs based on the
		//fBuildingShowRoofs value.
		UpdateRoofsView();
	}
}

void BuildingToggleWallViewCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( fBuildingShowWalls ^= 1 )
		{
			ClickEditorButton( BUILDING_TOGGLE_WALL_VIEW );
		}
		else
		{
			UnclickEditorButton( BUILDING_TOGGLE_WALL_VIEW );
		}
		//Name could be a bit misleading.  It'll hide or show the walls based on the
		//fBuildingShowWalls value.
		UpdateWallsView();
	}
}

void BuildingToggleInfoViewCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( fBuildingShowRoomInfo ^= 1 )
		{
			ClickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
		}
		else
		{
			UnclickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
		}
		gfRenderWorld = TRUE;
	}
}

//MAPINFO
void BtnFakeLightCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		btn->uiFlags ^= BUTTON_CLICKED_ON;
		iEditorToolbarState = TBAR_MODE_FAKE_LIGHTING;
	}
}

void BtnDrawLightsCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorMapInfoTaskbarMode( MAPINFO_ADD_LIGHT1_SOURCE );
}

void MapInfoDrawExitGridCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetEditorMapInfoTaskbarMode( MAPINFO_DRAW_EXITGRIDS );
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		SetEditorMapInfoTaskbarMode( MAPINFO_DRAW_EXITGRIDS );
		LocateNextExitGrid();
	}
}

void MapInfoEntryPointsCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & (MSYS_CALLBACK_REASON_POINTER_UP | MSYS_CALLBACK_REASON_RBUTTON_UP) )
	{
		INT16 x, sGridNo; // HACK000E
		for( x = MAPINFO_NORTH_POINT; x <= MAPINFO_ISOLATED_POINT; x++ )
		{
			if (btn == iEditorButton[x])
			{
				SetEditorMapInfoTaskbarMode( x );
				if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
				{
					switch( x )
					{
						case MAPINFO_NORTH_POINT:			sGridNo = gMapInformation.sNorthGridNo;			break;
						case MAPINFO_WEST_POINT:			sGridNo = gMapInformation.sWestGridNo;			break;
						case MAPINFO_EAST_POINT:			sGridNo = gMapInformation.sEastGridNo;			break;
						case MAPINFO_SOUTH_POINT:			sGridNo = gMapInformation.sSouthGridNo;			break;
						case MAPINFO_CENTER_POINT:		sGridNo = gMapInformation.sCenterGridNo;		break;
						case MAPINFO_ISOLATED_POINT:	sGridNo = gMapInformation.sIsolatedGridNo;	break;

						default: abort(); // HACK000E
					}
					if( sGridNo != -1 )
					{
						CenterScreenAtMapIndex( sGridNo );
					}
				}
				break;
			}
		}
	}
}

void MapInfoNormalRadioCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetEditorMapInfoTaskbarMode( MAPINFO_RADIO_NORMAL );
}

void MapInfoBasementRadioCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetEditorMapInfoTaskbarMode( MAPINFO_RADIO_BASEMENT );
}

void MapInfoCavesRadioCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetEditorMapInfoTaskbarMode( MAPINFO_RADIO_CAVES );
}

void MapInfoPrimeTimeRadioCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		ChangeLightDefault( PRIMETIME_LIGHT );
}

void MapInfoNightTimeRadioCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		ChangeLightDefault( NIGHTTIME_LIGHT );
}

void MapInfo24HourTimeRadioCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		ChangeLightDefault( ALWAYSON_LIGHT );
}

//OPTIONS
void BtnNewMapCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_NEW_MAP;
		gfPendingBasement = FALSE;
		gfPendingCaves = FALSE;
	}
}

void BtnNewBasementCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_NEW_MAP;
		gfPendingBasement = TRUE;
		gfPendingCaves = FALSE;
	}
}

void BtnNewCavesCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_NEW_MAP;
		gfPendingBasement = FALSE;
		gfPendingCaves = TRUE;
	}
}

void BtnSaveCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_SAVE;
	}
}

void BtnLoadCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_LOAD;
	}
}

void BtnChangeTilesetCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		iEditorToolbarState = TBAR_MODE_CHANGE_TILESET;
	}
}

void BtnCancelCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason&MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_EXIT_EDIT;
	}
}

void BtnQuitCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		iEditorToolbarState = TBAR_MODE_QUIT_GAME;
	}
}

//ITEMS
void MouseMovedInItemsRegion( MOUSE_REGION *reg, UINT32 reason )
{
	HandleItemsPanel( gusMouseXPos, gusMouseYPos, GUI_MOVE_EVENT );
}


static void ItemsScrollLeft()
{
	if (eInfo.sScrollIndex == 0) return;
	gfRenderTaskbar = TRUE;
	if (--eInfo.sScrollIndex == 0) DisableButton(iEditorButton[ITEMS_LEFTSCROLL]);
	EnableButton(iEditorButton[ITEMS_RIGHTSCROLL]);
}


static void ItemsScrollRight()
{
	INT16 const max_col = std::max(((eInfo.sNumItems + 1) / 2) - 6, 0);
	if (eInfo.sScrollIndex >= max_col) return;
	gfRenderTaskbar = TRUE;
	EnableButton(iEditorButton[ITEMS_LEFTSCROLL]);
	if (++eInfo.sScrollIndex == max_col) DisableButton(iEditorButton[ITEMS_RIGHTSCROLL]);
}


void MouseClickedInItemsRegion(MOUSE_REGION *reg, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		HandleItemsPanel( gusMouseXPos, gusMouseYPos, GUI_LCLICK_EVENT );
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
		HandleItemsPanel( gusMouseXPos, gusMouseYPos, GUI_RCLICK_EVENT );
	else if (reason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		ItemsScrollLeft();
	}
	else if (reason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		ItemsScrollRight();
	}
}

void ItemsWeaponsCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_WEAPONS );
}

void ItemsAmmoCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_AMMO );
}

void ItemsArmourCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_ARMOUR );
}

void ItemsExplosivesCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_EXPLOSIVES );
}

void ItemsEquipment1Callback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_EQUIPMENT1 );
}

void ItemsEquipment2Callback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_EQUIPMENT2 );
}

void ItemsEquipment3Callback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_EQUIPMENT3 );
}

void ItemsTriggersCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_TRIGGERS );
}

void ItemsKeysCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
		SetEditorItemsTaskbarMode( ITEMS_KEYS );
}

void ItemsLeftScrollCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) ItemsScrollLeft();
}

void ItemsRightScrollCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP) ItemsScrollRight();
}

//MERCS
void MercsTogglePlayers( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gfShowPlayers = btn->Clicked();
		SetMercTeamVisibility( OUR_TEAM, gfShowCivilians );
	}
}

void MercsToggleEnemies( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gfShowEnemies = btn->Clicked();
		SetMercTeamVisibility( ENEMY_TEAM, gfShowEnemies );
	}
}

void MercsToggleCreatures( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gfShowCreatures = btn->Clicked();
		SetMercTeamVisibility( CREATURE_TEAM, gfShowCreatures );
	}
}

void MercsToggleRebels( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gfShowRebels = btn->Clicked();
		SetMercTeamVisibility( MILITIA_TEAM, gfShowRebels );
	}
}

void MercsToggleCivilians( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gfShowCivilians = btn->Clicked();
		SetMercTeamVisibility( CIV_TEAM, gfShowCivilians );
	}
}

void MercsPlayerTeamCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		ClickEditorButton( MERCS_PLAYER );
		UnclickEditorButton( MERCS_ENEMY );
		UnclickEditorButton( MERCS_CREATURE );
		UnclickEditorButton( MERCS_REBEL );
		UnclickEditorButton( MERCS_CIVILIAN );
		iDrawMode = DRAW_MODE_PLAYER;
		SetMercEditingMode ( MERC_TEAMMODE );
	}
}

void MercsEnemyTeamCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		UnclickEditorButton( MERCS_PLAYER );
		ClickEditorButton( MERCS_ENEMY );
		UnclickEditorButton( MERCS_CREATURE );
		UnclickEditorButton( MERCS_REBEL );
		UnclickEditorButton( MERCS_CIVILIAN );
		iDrawMode = DRAW_MODE_ENEMY;
		SetMercEditingMode ( MERC_TEAMMODE );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		IndicateSelectedMerc( SELECT_NEXT_ENEMY );
	}
}

void MercsCreatureTeamCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		UnclickEditorButton( MERCS_PLAYER );
		UnclickEditorButton( MERCS_ENEMY );
		ClickEditorButton( MERCS_CREATURE );
		UnclickEditorButton( MERCS_REBEL );
		UnclickEditorButton( MERCS_CIVILIAN );
		iDrawMode = DRAW_MODE_CREATURE;
		SetMercEditingMode ( MERC_TEAMMODE );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		IndicateSelectedMerc( SELECT_NEXT_CREATURE );
	}
}

void MercsRebelTeamCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		UnclickEditorButton( MERCS_PLAYER );
		UnclickEditorButton( MERCS_ENEMY );
		UnclickEditorButton( MERCS_CREATURE );
		ClickEditorButton( MERCS_REBEL );
		UnclickEditorButton( MERCS_CIVILIAN );
		iDrawMode = DRAW_MODE_REBEL;
		SetMercEditingMode ( MERC_TEAMMODE );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		IndicateSelectedMerc( SELECT_NEXT_REBEL );
	}
}

void MercsCivilianTeamCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		UnclickEditorButton( MERCS_PLAYER );
		UnclickEditorButton( MERCS_ENEMY );
		UnclickEditorButton( MERCS_CREATURE );
		UnclickEditorButton( MERCS_REBEL );
		ClickEditorButton( MERCS_CIVILIAN );
		iDrawMode = DRAW_MODE_CIVILIAN;
		SetMercEditingMode ( MERC_TEAMMODE );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		IndicateSelectedMerc( SELECT_NEXT_CIV );
	}
}

void MercsNextCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		IndicateSelectedMerc( SELECT_NEXT_MERC );
	}
}

void MercsDeleteCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		DeleteSelectedMerc();
		SetMercEditingMode( MERC_TEAMMODE );
	}
}

void MercsInventorySlotCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		INT32 const uiSlot = btn->GetUserData();
		SetEnemyDroppableStatus(uiSlot, btn->Clicked());
	}
}

void MercsSetEnemyColorCodeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		UINT8 const ubSoldierClass = btn->GetUserData();
		SetEnemyColorCode( ubSoldierClass );
	}
}

void MercsCivilianGroupCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		iEditorToolbarState = TBAR_MODE_CIVILIAN_GROUP;
	}
}


void MercsScheduleAction1Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		gubCurrentScheduleActionIndex = 0;
		InitPopupMenu( iEditorButton[ MERCS_SCHEDULE_ACTION1 ], SCHEDULEACTION_POPUP, DIR_UPRIGHT );
	}
}

void MercsScheduleAction2Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		gubCurrentScheduleActionIndex = 1;
		InitPopupMenu( iEditorButton[ MERCS_SCHEDULE_ACTION2 ], SCHEDULEACTION_POPUP, DIR_UPRIGHT );
	}
}

void MercsScheduleAction3Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		gubCurrentScheduleActionIndex = 2;
		InitPopupMenu( iEditorButton[ MERCS_SCHEDULE_ACTION3 ], SCHEDULEACTION_POPUP, DIR_UPRIGHT );
	}
}

void MercsScheduleAction4Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		gubCurrentScheduleActionIndex = 3;
		InitPopupMenu( iEditorButton[ MERCS_SCHEDULE_ACTION4 ], SCHEDULEACTION_POPUP, DIR_UPRIGHT );
	}
}

void MercsScheduleToggleVariance1Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if (btn->Clicked())
			gCurrSchedule.usFlags |= SCHEDULE_FLAGS_VARIANCE1;
		else
			gCurrSchedule.usFlags &= ~SCHEDULE_FLAGS_VARIANCE1;
	}
}

void MercsScheduleToggleVariance2Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if (btn->Clicked())
			gCurrSchedule.usFlags |= SCHEDULE_FLAGS_VARIANCE2;
		else
			gCurrSchedule.usFlags &= ~SCHEDULE_FLAGS_VARIANCE2;
	}
}

void MercsScheduleToggleVariance3Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if (btn->Clicked())
			gCurrSchedule.usFlags |= SCHEDULE_FLAGS_VARIANCE3;
		else
			gCurrSchedule.usFlags &= ~SCHEDULE_FLAGS_VARIANCE3;
	}
}

void MercsScheduleToggleVariance4Callback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if (btn->Clicked())
			gCurrSchedule.usFlags |= SCHEDULE_FLAGS_VARIANCE4;
		else
			gCurrSchedule.usFlags &= ~SCHEDULE_FLAGS_VARIANCE4;
	}
}

void MercsScheduleData1ACallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 0;
		StartScheduleAction();
		gfSingleAction = TRUE;
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 0 );
	}
}

void MercsScheduleData1BCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 0;
		gfUseScheduleData2 = FALSE;
		RegisterCurrentScheduleAction( gCurrSchedule.usData1[0] );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 1 );
	}
}

void MercsScheduleData2ACallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 1;
		StartScheduleAction();
		gfSingleAction = TRUE;
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 2 );
	}
}

void MercsScheduleData2BCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 1;
		gfUseScheduleData2 = FALSE;
		RegisterCurrentScheduleAction( gCurrSchedule.usData1[1] );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 3 );
	}
}

void MercsScheduleData3ACallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 2;
		StartScheduleAction();
		gfSingleAction = TRUE;
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 4 );
	}
}

void MercsScheduleData3BCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 2;
		gfUseScheduleData2 = FALSE;
		RegisterCurrentScheduleAction( gCurrSchedule.usData1[2] );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 5 );
	}
}

void MercsScheduleData4ACallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 3;
		StartScheduleAction();
		gfSingleAction = TRUE;
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 6 );
	}
}

void MercsScheduleData4BCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		gubCurrentScheduleActionIndex = 3;
		gfUseScheduleData2 = FALSE;
		RegisterCurrentScheduleAction( gCurrSchedule.usData1[3] );
	}
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		FindScheduleGridNo( 7 );
	}
}

void MercsScheduleClearCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		ClearCurrentSchedule();
		ExtractAndUpdateMercSchedule();
	}
}


void MercsDetailedPlacementCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if (btn->Clicked())
		{
			InitDetailedPlacementForMerc();
			SetMercEditingMode( MERC_GENERALMODE );
		}
		else //button is unchecked.
		{
			KillDetailedPlacementForMerc();
			SetMercEditingMode( MERC_BASICMODE );
		}
	}
}

void MercsPriorityExistanceCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if( gpSelected )
		{
			if (btn->Clicked())
			{
				gpSelected->pBasicPlacement->fPriorityExistance = TRUE;
			}
			else //button is unchecked.
			{
				gpSelected->pBasicPlacement->fPriorityExistance = FALSE;
			}
		}
	}
}

void MercsHasKeysCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if( gpSelected )
		{
			if (btn->Clicked())
			{
				gpSelected->pBasicPlacement->fHasKeys = TRUE;
			}
			else //button is unchecked.
			{
				gpSelected->pBasicPlacement->fHasKeys = FALSE;
			}
			if( gpSelected->pDetailedPlacement )
			{
				gpSelected->pDetailedPlacement->fHasKeys = gpSelected->pBasicPlacement->fHasKeys;
			}
		}
	}
}

void MercsGeneralModeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercEditingMode( MERC_GENERALMODE );
}

void MercsAttributesModeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercEditingMode( MERC_ATTRIBUTEMODE );
}

void MercsInventoryModeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercEditingMode( MERC_INVENTORYMODE );
}

void MercsAppearanceModeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercEditingMode( MERC_APPEARANCEMODE );
}

void MercsProfileModeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercEditingMode( MERC_PROFILEMODE );
}

void MercsScheduleModeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercEditingMode( MERC_SCHEDULEMODE );
}


//multiple orders buttons
void MercsSetOrdersCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercOrders(btn->GetUserData());
}

//multiple attitude buttons
void MercsSetAttitudeCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercAttitude(btn->GetUserData());
}

//multiple direction buttons
void MercsDirectionSetCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercDirection(btn->GetUserData());
}

void MercsFindSelectedMercCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		const SOLDIERTYPE* const s = g_selected_merc;
		if (s == NULL) return;
		CenterScreenAtMapIndex(s->sGridNo);
	}
}

void MercsSetRelativeEquipmentCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercRelativeEquipment(btn->GetUserData());
}

void MercsSetRelativeAttributesCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		SetMercRelativeAttributes(btn->GetUserData());
}

void MouseMovedInMercRegion( MOUSE_REGION *reg, UINT32 reason )
{
	HandleMercInventoryPanel( reg->RelativeXPos, reg->RelativeYPos, GUI_MOVE_EVENT );
}

void MouseClickedInMercRegion( MOUSE_REGION *reg, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		HandleMercInventoryPanel( reg->RelativeXPos, reg->RelativeYPos, GUI_LCLICK_EVENT );
	else if( reason & MSYS_CALLBACK_REASON_RBUTTON_UP )
		HandleMercInventoryPanel( reg->RelativeXPos, reg->RelativeYPos, GUI_RCLICK_EVENT );
}

//VARIOUS
void BtnUndoCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iEditorToolbarState = TBAR_MODE_UNDO;
	}
}

void BtnEraseCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( iDrawMode >= DRAW_MODE_ERASE )
			btn->uiFlags &= (~BUTTON_CLICKED_ON);
		else
			btn->uiFlags |= BUTTON_CLICKED_ON;
		iEditorToolbarState = btn->Clicked() ? TBAR_MODE_ERASE : TBAR_MODE_ERASE_OFF;
	}
}

//ITEM STATS
void ItemStatsToggleHideCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		ExecuteItemStatsCmd(btn->Clicked() ? ITEMSTATS_HIDE : ITEMSTATS_SHOW);
	}
}

void ItemStatsDeleteCallback( GUI_BUTTON *btn, UINT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		ExecuteItemStatsCmd( ITEMSTATS_DELETE );
}
