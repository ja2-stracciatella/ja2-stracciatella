#include "Editor_Callback_Prototypes.h"
#include "HImage.h"
#include "Handle_UI.h"
#include "Interface.h"
#include "Local.h"
#include "Edit_Sys.h"
#include "TileDat.h"
#include "VSurface.h"
#include "VObject.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Line.h"
#include "Input.h"
#include "SysUtil.h"
#include "Font.h"
#include "Font_Control.h"
#include "EditScreen.h"
#include "SelectWin.h"
#include "Video.h"
#include "Interface_Items.h"
#include "Text.h"
#include "World_Items.h"
#include "WorldMan.h"
#include "WorldDef.h"
#include "Overhead.h"
#include "RenderWorld.h"
#include "Animation_Data.h"
#include "Animation_Control.h"
#include "EditorDefines.h"
#include "EditorMercs.h"
#include "EditorTerrain.h" //for access to TerrainTileDrawMode
#include "Soldier_Create.h" //The stuff that connects the editor generated information
														//to the saved map and the game itself.
#include "Inventory_Choosing.h"
#include "Soldier_Init_List.h"
#include "StrategicMap.h"
#include "Soldier_Add.h"
#include "Soldier_Control.h"
#include "Soldier_Profile_Type.h"
#include "Soldier_Profile.h"
#include "Text_Input.h"
#include "Random.h"
#include "WordWrap.h"
#include "EditorItems.h"
#include "Editor_Taskbar_Utils.h"
#include "Exit_Grids.h"
#include "Editor_Undo.h"
#include "Item_Statistics.h"
#include "Simple_Render_Utils.h"
#include "Map_Information.h"
#include "Isometric_Utils.h"
#include "Render_Dirty.h"
#include "PopupMenu.h"
#include "Scheduling.h"
#include "Timer_Control.h"
#include "Items.h"
#include "Debug.h"
#include "Message.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>


//--------------------------------------------------
//	NON_CIV_GROUP,
//	REBEL_CIV_GROUP,
//	KINGPIN_CIV_GROUP,
//	SANMONA_ARMS_GROUP,
//	ANGELS_GROUP,
//	NUM_CIV_GROUPS
const ST::string gszCivGroupNames[NUM_CIV_GROUPS] =
{
	"NONE",
	"REBEL",
	"KINGPIN",
	"SANMONA ARMS",
	"ANGELS",

	"BEGGARS",
	"TOURISTS",
	"ALMA MIL",
	"DOCTORS",
	"COUPLE1",

	"HICKS",
	"WARDEN",
	"JUNKYARD",
	"FACTORY KIDS",
	"QUEENS",
	"UNUSED15",
	"UNUSED16",
	"UNUSED17",
	"UNUSED18",
	"UNUSED19",
};

//--------------------------------------------------

//	SCHEDULE_ACTION_NONE,
//	SCHEDULE_ACTION_LOCKDOOR,
//	SCHEDULE_ACTION_UNLOCKDOOR,
//	SCHEDULE_ACTION_OPENDOOR,
//	SCHEDULE_ACTION_CLOSEDOOR,
//	SCHEDULE_ACTION_GRIDNO,
//	SCHEDULE_ACTION_LEAVESECTOR,
//	SCHEDULE_ACTION_ENTERSECTOR,
//	SCHEDULE_ACTION_STAYINSECTOR,
//  SCHEDULE_ACTION_SLEEP,
const ST::string gszScheduleActions[NUM_SCHEDULE_ACTIONS] =
{
	"No action",
	"Lock door",
	"Unlock door",
	"Open door",
	"Close door",
	"Move to gridno",
	"Leave sector",
	"Enter sector",
	"Stay in sector",
	"Sleep",
	"Ignore this!"
};


enum
{
	SCHEDULE_INSTRUCTIONS_NONE,
	SCHEDULE_INSTRUCTIONS_DOOR1,
	SCHEDULE_INSTRUCTIONS_DOOR2,
	SCHEDULE_INSTRUCTIONS_GRIDNO,
	SCHEDULE_INSTRUCTIONS_SLEEP,
	NUM_SCHEDULE_INSTRUCTIONS
};

BOOLEAN gfSingleAction = FALSE;
BOOLEAN gfUseScheduleData2 = FALSE;
UINT8 gubCurrentScheduleActionIndex = 0;
SCHEDULENODE gCurrSchedule;
UINT8 gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_NONE;


//array which keeps track of which item is in which slot.  This is dependant on the selected merc, so
//these temp values must be updated when different mercs are selected, and reset when a merc detailed
//placement is created.
OBJECTTYPE *gpMercSlotItem[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
//Because we only support these nine slots, they aren't continuous values, so this array helps
//processing functions by referring to this array to get the appropriate slot.
INT8 gbMercSlotTypes[9] = { HELMETPOS, VESTPOS, LEGPOS, HANDPOS, SECONDHANDPOS,
														BIGPOCK1POS, BIGPOCK2POS, BIGPOCK3POS, BIGPOCK4POS };
//returns the usItem index of specified slot in the currently selected merc.
#define GetSelectedMercSlotItemIndex( x )  (gpSelected->pDetailedPlacement->Inv[gbMercSlotTypes[x]].usItem)
#define GetSelectedMercSlot( x )  (&gpSelected->pDetailedPlacement->Inv[gbMercSlotTypes[x]])
//values indicating which merc inventory slot is hilited and which slot is selected.
INT8 gbCurrHilite = -1;
INT8 gbCurrSelect = -1;

//internal merc variables
BASIC_SOLDIERCREATE_STRUCT gTempBasicPlacement;
SOLDIERCREATE_STRUCT gTempDetailedPlacement;

SOLDIERTYPE*     g_selected_merc;
INT16						gsSelectedMercGridNo;
SOLDIERINITNODE *gpSelected;

UINT8						gubCurrMercMode									= MERC_TEAMMODE;
static UINT8    gubPrevMercMode                 = MERC_NOMODE;
UINT8						gubLastDetailedMercMode					= MERC_GENERALMODE;
INT8						gbDefaultOrders									= STATIONARY;
INT8						gbDefaultAttitude								= DEFENSIVE;
INT8						gbDefaultRelativeEquipmentLevel = 2;
INT8						gbDefaultRelativeAttributeLevel = 2;
INT8						gbDefaultDirection							= NORTHWEST;
INT8						gubSoldierClass							= SOLDIER_CLASS_ARMY;
UINT8						gubCivGroup											= NON_CIV_GROUP;

SOLDIERTYPE			*pTempSoldier;
BOOLEAN					gfRoofPlacement;

//Below are all flags that have to do with editing detailed placement mercs:

//Determines if the user is allowed to edit merc colors.  User must specifically
//click on the checkbox by the colors to confirm that colors will be specified.  If
//not, the colors will be randomly generated upon creation in the game.
BOOLEAN					gfCanEditMercColors							= FALSE;
//A rendering flag that is set whenever a full update of the merc editing information
//needs to be done.
BOOLEAN					gfRenderMercInfo								= FALSE;
//When the user specifies a profile index for the merc, all editing is disabled.  This is
//because the profile contains all of the information.  When this happens, all of the editing
//buttons are disabled, but you are allowed to view stats, inventory, etc., as well as specify
//orders, attitude, and direction.
BOOLEAN					gfCanEditMercs									= TRUE;
//When in inventory mode, this flag is set when the user wishes to get an item, which requires hooking
//into the item editing features.  This is processed during the editor update, which in turn, sets the
//new mode.
BOOLEAN					gfMercGetItem										= FALSE;
//As soon as an item is selected, the items index is stored here, so the item can be copied into the
//slot for editing and rendering purposes.  This is a temp store value only when leaving the editor items
//mode.
UINT16 gusMercsNewItemIndex												= 0xffff;

INT32 iEditWhichStat = -1;
INT32 iEditMercMode = EDIT_MERC_NONE;
INT32	iEditColorStart[EDIT_NUM_COLORS];

BOOLEAN gfShowPlayers = TRUE;
BOOLEAN gfShowEnemies = TRUE;
BOOLEAN gfShowCreatures = TRUE;
BOOLEAN gfShowRebels = TRUE;
BOOLEAN gfShowCivilians = TRUE;


//information for bodytypes.
static SoldierBodyType const bEnemyArray[]    = { BODY_RANDOM, REGMALE, BIGMALE, STOCKYMALE, REGFEMALE, TANK_NW, TANK_NE };
static SoldierBodyType const bCreatureArray[] = { BLOODCAT, LARVAE_MONSTER, INFANT_MONSTER, YAF_MONSTER, YAM_MONSTER, ADULTFEMALEMONSTER, AM_MONSTER, QUEENMONSTER };
static SoldierBodyType const bRebelArray[]    = { BODY_RANDOM, FATCIV, MANCIV, REGMALE, BIGMALE, STOCKYMALE, REGFEMALE };
static SoldierBodyType const bCivArray[]      = { BODY_RANDOM, FATCIV, MANCIV, MINICIV, DRESSCIV, HATKIDCIV, KIDCIV, REGMALE, BIGMALE, STOCKYMALE, REGFEMALE, HUMVEE, ELDORADO, ICECREAMTRUCK, JEEP, CRIPPLECIV, ROBOTNOWEAPON, COW };
static SoldierBodyType gbCurrCreature = BLOODCAT;


void GameInitEditorMercsInfo()
{
	INT32 i;
	//Initialize the placement list
	InitSoldierInitList();
	gMapInformation.ubNumIndividuals = 0;
	gCurrSchedule = SCHEDULENODE{};
	for( i = 0; i < 4; i++ )
	{
		gCurrSchedule.usTime[i] = 0xffff;
		gCurrSchedule.usData1[i] = 0xffff;
		gCurrSchedule.usData2[i] = 0xffff;
	}
}

void GameShutdownEditorMercsInfo()
{
	UseEditorAlternateList();
	KillSoldierInitList();
	UseEditorOriginalList();
	KillSoldierInitList();
}

void EntryInitEditorMercsInfo()
{
	INT32 x, iCurStart = 0;
	iEditColorStart[0] = 0;
	for ( x = 1; x < EDIT_NUM_COLORS; x++ )
	{
		iCurStart += gubpNumReplacementsPerRange[ x - 1 ];
		iEditColorStart[x] = iCurStart;
	}
	g_selected_merc      = NULL;
	gsSelectedMercGridNo = 0;

	gfCanEditMercs = TRUE;
}


void ProcessMercEditing(void)
{
	if (iEditMercMode != EDIT_MERC_NEXT_COLOR) return;

	// Handle changes to the merc colors
	SOLDIERTYPE& s = *g_selected_merc;
	ST::string* soldier_pal;
	ST::string* placement_pal;
	UINT8 ubType;
	switch (iEditWhichStat)
	{
		case 0:
		case 1:
			ubType        = EDIT_COLOR_HEAD;
			soldier_pal   = &s.HeadPal;
			placement_pal = &gpSelected->pDetailedPlacement->HeadPal;
			break;

		case 2:
		case 3:
			ubType        = EDIT_COLOR_SKIN;
			soldier_pal   = &s.SkinPal;
			placement_pal = &gpSelected->pDetailedPlacement->SkinPal;
			break;

		case 4:
		case 5:
			ubType        = EDIT_COLOR_VEST;
			soldier_pal   = &s.VestPal;
			placement_pal = &gpSelected->pDetailedPlacement->VestPal;
			break;

		case 6:
		case 7:
			ubType        = EDIT_COLOR_PANTS;
			soldier_pal   = &s.PantsPal;
			placement_pal = &gpSelected->pDetailedPlacement->PantsPal;
			break;

		default:
			iEditMercMode = EDIT_MERC_NONE;
			return;
	}

	UINT8 ubPaletteRep = GetPaletteRepIndexFromID(*soldier_pal);
	const INT32 start = iEditColorStart[ubType];
	const UINT8 range = gubpNumReplacementsPerRange[ubType];
	if (iEditWhichStat & 1)
	{
		ubPaletteRep = (ubPaletteRep < start + range - 1 ? ubPaletteRep + 1 : start);
	}
	else
	{
		ubPaletteRep = (ubPaletteRep > start ? ubPaletteRep - 1 : start + range - 1);
	}

	soldier_pal->set(gpPalRep[ubPaletteRep].ID);
	placement_pal->set(gpPalRep[ubPaletteRep].ID);
	CreateSoldierPalettes(s);

	iEditMercMode = EDIT_MERC_NONE;
}


void AddMercToWorld( INT32 iMapIndex )
{
	INT32 i;

	gTempBasicPlacement = BASIC_SOLDIERCREATE_STRUCT{};

	//calculate specific information based on the team.
	SoldierBodyType body = BODY_RANDOM;
	switch( iDrawMode )
	{
		case DRAW_MODE_ENEMY:
			gTempBasicPlacement.bTeam = ENEMY_TEAM;
			gTempBasicPlacement.ubSoldierClass = gubSoldierClass;
			break;
		case DRAW_MODE_CREATURE:
			gTempBasicPlacement.bTeam = CREATURE_TEAM;
			body = gbCurrCreature;
			break;
		case DRAW_MODE_REBEL:
			gTempBasicPlacement.bTeam = MILITIA_TEAM;
			break;
		case DRAW_MODE_CIVILIAN:
			gTempBasicPlacement.bTeam = CIV_TEAM;
			gTempBasicPlacement.ubCivilianGroup = gubCivGroup;
			if (giCurrentTilesetID == CAVES_1)
			{
				gTempBasicPlacement.ubSoldierClass = SOLDIER_CLASS_MINER;
			}
			break;
	}
	gTempBasicPlacement.bBodyType = body;

	if( IsLocationSittable( iMapIndex, gfRoofPlacement ) )
	{
		SOLDIERINITNODE *pNode;

		//Set up some general information.
		gTempBasicPlacement.fDetailedPlacement = FALSE;
		gTempBasicPlacement.fPriorityExistance = FALSE;
		gTempBasicPlacement.usStartingGridNo = (UINT16)iMapIndex;
		gTempBasicPlacement.bOrders = gbDefaultOrders;
		gTempBasicPlacement.bAttitude = gbDefaultAttitude;
		gTempBasicPlacement.bRelativeAttributeLevel = gbDefaultRelativeAttributeLevel;
		gTempBasicPlacement.bRelativeEquipmentLevel = gbDefaultRelativeEquipmentLevel;
		gTempBasicPlacement.bDirection = gbDefaultDirection;

		//Generate detailed placement information given the temp placement information.
		CreateDetailedPlacementGivenBasicPlacementInfo( &gTempDetailedPlacement, &gTempBasicPlacement );

		//Set the sector information -- probably unnecessary.
		gTempDetailedPlacement.sSector = gWorldSector;

		//Create the soldier, but don't place it yet.
		SOLDIERTYPE* const pSoldier = TacticalCreateSoldier(gTempDetailedPlacement);
		if (pSoldier != NULL)
		{
			pSoldier->bVisible = 1;
			pSoldier->bLastRenderVisibleValue = 1;
			//Set up the soldier in the list, so we can track the soldier in the
			//future (saving, loading, strategic AI)
			pNode = AddBasicPlacementToSoldierInitList(gTempBasicPlacement);
			pNode->pSoldier = pSoldier;

			//Add the soldier to physically appear on the map now.
			AddSoldierToSectorNoCalculateDirection(pSoldier);
			IndicateSelectedMerc(pSoldier->ubID);

			//Move him to the roof if intended and possible.
			if( gfRoofPlacement && FlatRoofAboveGridNo( iMapIndex ) )
			{
				gpSelected->pBasicPlacement->fOnRoof = TRUE;
				if( gpSelected->pDetailedPlacement )
					gpSelected->pDetailedPlacement->fOnRoof = TRUE;
				SetSoldierHeight( gpSelected->pSoldier, SECOND_LEVEL_Z_OFFSET );
			}
			UnclickEditorButtons( FIRST_MERCS_INVENTORY_BUTTON, LAST_MERCS_INVENTORY_BUTTON );
			for( i = FIRST_MERCS_INVENTORY_BUTTON; i <= LAST_MERCS_INVENTORY_BUTTON; i++ )
			{
				SetEnemyDroppableStatus( gbMercSlotTypes[i-FIRST_MERCS_INVENTORY_BUTTON], FALSE );
			}
		}
	}
}


static SOLDIERTYPE* IsMercHere(INT32 iMapIndex);


void HandleRightClickOnMerc( INT32 iMapIndex )
{
	const SOLDIERTYPE* const this_merc = IsMercHere(iMapIndex);
	if (this_merc != NULL)
	{
		if (g_selected_merc != this_merc)
		{ // We want to edit a new merc (or different merc)
			//We need to avoid the editing of player mercs.
			if (!FindSoldierInitNodeBySoldier(*this_merc))
				return;		//this is a player merc (which isn't in the list), or an error in logic.
			IndicateSelectedMerc(this_merc->ubID);
		}
	}
	else if (g_selected_merc != NULL && IsLocationSittable(iMapIndex, gfRoofPlacement)) // We want to move the selected merc to this new location.
	{
		RemoveAllObjectsOfTypeRange( gsSelectedMercGridNo, CONFIRMMOVE, CONFIRMMOVE );
		EVENT_SetSoldierPosition(gpSelected->pSoldier, iMapIndex, SSP_NONE);
		if( gfRoofPlacement && FlatRoofAboveGridNo( iMapIndex ) )
		{
			gpSelected->pBasicPlacement->fOnRoof = TRUE;
			if( gpSelected->pDetailedPlacement )
				gpSelected->pDetailedPlacement->fOnRoof = TRUE;
			SetSoldierHeight( gpSelected->pSoldier, SECOND_LEVEL_Z_OFFSET );
		}
		else
		{
			gpSelected->pBasicPlacement->fOnRoof = FALSE;
			if( gpSelected->pDetailedPlacement )
				gpSelected->pDetailedPlacement->fOnRoof = FALSE;
			SetSoldierHeight( gpSelected->pSoldier, 0.0 );
		}
		gsSelectedMercGridNo = (INT16)iMapIndex;
		gpSelected->pBasicPlacement->usStartingGridNo = gsSelectedMercGridNo;
		if( gpSelected->pDetailedPlacement )
			gpSelected->pDetailedPlacement->sInsertionGridNo = gsSelectedMercGridNo;
		AddObjectToHead( gsSelectedMercGridNo, CONFIRMMOVE1 );
	}
}

void ResetAllMercPositions()
{
	//Remove all of the alternate placements (editor takes precedence)
	UseEditorAlternateList();
	FOR_EACH_SOLDIERINITNODE_SAFE(curr)
	{
		gpSelected = curr;
		RemoveSoldierNodeFromInitList( gpSelected );
	}
	//Now, remove any existing original list mercs, then readd them.
	UseEditorOriginalList();
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if( curr->pSoldier )
		{
			TacticalRemoveSoldier(*curr->pSoldier);
			curr->pSoldier = NULL;
		}
		//usMapIndex = gpSelected->pBasicPlacement->usStartingGridNo;
		//if( gpSelected->pSoldier )
		//{
		//	EVENT_SetSoldierPosition(gpSelected->pSoldier, usMapIndex, SSP_NONE);
		//	if( gpSelected->pBasicPlacement->fOnRoof )
		//		SetSoldierHeight( gpSelected->pSoldier, SECOND_LEVEL_Z_OFFSET );
		//	SetMercDirection( gpSelected->pBasicPlacement->bDirection );
		//}
	}
	AddSoldierInitListTeamToWorld(ENEMY_TEAM);
	AddSoldierInitListTeamToWorld(CREATURE_TEAM);
	AddSoldierInitListTeamToWorld(MILITIA_TEAM);
	AddSoldierInitListTeamToWorld(CIV_TEAM);
	gpSelected = NULL;
	g_selected_merc = NULL;
}

void AddMercWaypoint( UINT32 iMapIndex )
{
	INT32 iNum;
	// index 0 isn't used
	if ( iActionParam == 0 )
		return;

	if (g_selected_merc == NULL || g_selected_merc->ubID <= gTacticalStatus.Team[OUR_TEAM].bLastID || g_selected_merc->ubID >= MAXMERCS)
		return;

	if ( iActionParam > gpSelected->pSoldier->bPatrolCnt )
	{
		// Fill up missing waypoints with same value as new one
		for(iNum = gpSelected->pSoldier->bPatrolCnt + 1; iNum <= iActionParam; iNum++)
		{
			gpSelected->pBasicPlacement->sPatrolGrid[iNum] = (INT16)iMapIndex;
			if( gpSelected->pDetailedPlacement )
				gpSelected->pDetailedPlacement->sPatrolGrid[iNum] = (INT16)iMapIndex;
			gpSelected->pSoldier->usPatrolGrid[iNum] = (UINT16)iMapIndex;
		}

		gpSelected->pBasicPlacement->bPatrolCnt = (INT8)iActionParam;
		if( gpSelected->pDetailedPlacement )
			gpSelected->pDetailedPlacement->bPatrolCnt = (INT8)iActionParam;
		gpSelected->pSoldier->bPatrolCnt = (INT8) iActionParam;
		gpSelected->pSoldier->bNextPatrolPnt = 1;
	}
	else
	{
		// Set this way point
		gpSelected->pBasicPlacement->sPatrolGrid[iActionParam] = (INT16)iMapIndex;
		if( gpSelected->pDetailedPlacement )
			gpSelected->pDetailedPlacement->sPatrolGrid[iActionParam] = (INT16)iMapIndex;
		gpSelected->pSoldier->usPatrolGrid[iActionParam] = (UINT16)iMapIndex;
	}
	gfRenderWorld = TRUE;
}

void EraseMercWaypoint()
{
	INT32 iNum;
	// index 0 isn't used
	if ( iActionParam == 0 )
		return;

	if (g_selected_merc == NULL || g_selected_merc->ubID <= gTacticalStatus.Team[OUR_TEAM].bLastID || g_selected_merc->ubID >= MAXMERCS)
		return;

	// Fill up missing areas
	if ( iActionParam > gpSelected->pSoldier->bPatrolCnt )
		return;

	for(iNum = iActionParam; iNum < gpSelected->pSoldier->bPatrolCnt; iNum++)
	{
		gpSelected->pBasicPlacement->sPatrolGrid[iNum] = gpSelected->pBasicPlacement->sPatrolGrid[iNum+1];
		if( gpSelected->pDetailedPlacement )
			gpSelected->pDetailedPlacement->sPatrolGrid[iNum] = gpSelected->pDetailedPlacement->sPatrolGrid[iNum+1];
		gpSelected->pSoldier->usPatrolGrid[iNum] = gpSelected->pSoldier->usPatrolGrid[iNum+1];
	}

	gpSelected->pBasicPlacement->bPatrolCnt--;
	if( gpSelected->pDetailedPlacement )
		gpSelected->pDetailedPlacement->bPatrolCnt--;
	gpSelected->pSoldier->bPatrolCnt--;
	gfRenderWorld = TRUE;
}


/* Checks for a soldier at the given map coordinates.  If there is one, it
 * returns it, otherwise it returns NULL. */
static SOLDIERTYPE* IsMercHere(INT32 iMapIndex)
{
	FOR_EACH_SOLDIER(s)
	{
		if (s->sGridNo == iMapIndex) return s;
	}
	return NULL;
}


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//	The following are the button callback functions for the merc editing pages
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------


void MercsToggleColorModeCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->Clicked())
		{
			EnableEditorButtons( FIRST_MERCS_COLOR_BUTTON, LAST_MERCS_COLOR_BUTTON );
			gpSelected->pDetailedPlacement->fVisible = TRUE;
			gpSelected->pDetailedPlacement->HeadPal = gpSelected->pSoldier->HeadPal;
			gpSelected->pDetailedPlacement->SkinPal = gpSelected->pSoldier->SkinPal;
			gpSelected->pDetailedPlacement->VestPal = gpSelected->pSoldier->VestPal;
			gpSelected->pDetailedPlacement->PantsPal = gpSelected->pSoldier->PantsPal;
		}
		else //button is unchecked.
		{
			DisableEditorButtons( FIRST_MERCS_COLOR_BUTTON, LAST_MERCS_COLOR_BUTTON );
			gpSelected->pDetailedPlacement->fVisible = FALSE;
		}
		gfRenderMercInfo = TRUE;
		gfRenderTaskbar = TRUE;
	}
}

void MercsSetColorsCallback( GUI_BUTTON *btn, INT32 reason )
{
	INT32 iBtn;
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iEditWhichStat = -1;
		for ( iBtn = FIRST_MERCS_COLOR_BUTTON; iBtn <= LAST_MERCS_COLOR_BUTTON; iBtn++ )
		{
			if (btn == iEditorButton[iBtn])
			{
				iEditWhichStat = iBtn - FIRST_MERCS_COLOR_BUTTON;
				iEditMercMode = EDIT_MERC_NEXT_COLOR;
				gfRenderMercInfo = TRUE;
				return;
			}
		}
	}
}


static void ChangeBodyType(INT8 bOffset);


void MercsSetBodyTypeCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMercInfo = TRUE;
		if (btn == iEditorButton[MERCS_BODYTYPE_DOWN])
			ChangeBodyType( 1 );	//next
		else
			ChangeBodyType( -1 ); //previous
	}
}


static void ShowEditMercColorSet(UINT8 ubPaletteRep, INT16 sSet);


//	Displays the palette of the given merc (used by the edit merc color page)
static void ShowEditMercPalettes(SOLDIERTYPE* pSoldier)
{
	UINT8  ubPaletteRep;
	if( !pSoldier )
		ubPaletteRep = 0xff;

	if( pSoldier )
	{
		if (pSoldier->HeadPal.empty())
			ubPaletteRep = 0xff;
		else
			ubPaletteRep = GetPaletteRepIndexFromID(pSoldier->HeadPal);
	}
	ShowEditMercColorSet( ubPaletteRep, 0 );

	if( pSoldier )
	{
		if (pSoldier->SkinPal.empty())
			ubPaletteRep = 0xff;
		else
			ubPaletteRep = GetPaletteRepIndexFromID(pSoldier->SkinPal);
	}
	ShowEditMercColorSet( ubPaletteRep, 1 );

	if( pSoldier )
	{
		if (pSoldier->VestPal.empty())
			ubPaletteRep = 0xff;
		else
			ubPaletteRep = GetPaletteRepIndexFromID(pSoldier->VestPal);
	}
	ShowEditMercColorSet( ubPaletteRep, 2 );

	if( pSoldier )
	{
		if (pSoldier->PantsPal.empty())
			ubPaletteRep = 0xff;
		else
			ubPaletteRep = GetPaletteRepIndexFromID(pSoldier->PantsPal);
	}
	ShowEditMercColorSet( ubPaletteRep, 3 );
}


//	Displays a single palette set. (used by ShowEditMercPalettes)
static void ShowEditMercColorSet(UINT8 ubPaletteRep, INT16 sSet)
{
	UINT16 us16BPPColor, usFillColorDark, usFillColorLight;
	UINT8 cnt1;
	UINT8  ubSize;
	INT16 sUnitSize;
	INT16  sLeft, sTop, sRight, sBottom;

	if( ubPaletteRep == 0xff )
		ubSize = 16;
	else
		ubSize = gpPalRep[ ubPaletteRep ].ubPaletteSize;

	sUnitSize = 128 / (INT16)(ubSize);

	sTop = EDITOR_TASKBAR_POS_Y + 4 + (sSet * 24);
	sBottom = sTop + 20;
	sLeft = 230;
	sRight = 359;

	usFillColorDark = Get16BPPColor(FROMRGB(24, 61, 81));
	usFillColorLight = Get16BPPColor(FROMRGB(136, 138, 135));

	// Draw color bar window area
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sLeft,     sTop,     sRight, sBottom, usFillColorDark );
	ColorFillVideoSurfaceArea(FRAME_BUFFER, sLeft + 1, sTop + 1, sRight, sBottom, usFillColorLight );
	InvalidateRegion( sLeft, sTop, sRight, sBottom );

	sTop++;
	sBottom--;
	sLeft++;
	sRight = sLeft + sUnitSize;

	// Draw the color bar
	for ( cnt1 = 0; cnt1 < ubSize; cnt1++ )
	{
		if (cnt1 == (ubSize - 1) )
			sRight = 358;
		if( ubPaletteRep == 0xff )
			us16BPPColor = Get16BPPColor( FROMRGB( (16 - cnt1)*10, (16 - cnt1)*10, (16 - cnt1)*10 ) );
		else
		{
			const SGPPaletteEntry* Clr = &gpPalRep[ubPaletteRep].rgb[cnt1];
			us16BPPColor = Get16BPPColor(FROMRGB(Clr->r, Clr->g, Clr->b));
		}
		ColorFillVideoSurfaceArea( FRAME_BUFFER, sLeft, sTop, sRight, sBottom, us16BPPColor );

		sLeft += sUnitSize;
		sRight += sUnitSize;
	}
}


//----------------------------------------------------------------------------------------------
//	DisplayWayPoints
//
//	Displays the way points of the currently selected merc.
//
void DisplayWayPoints(void)
{
	INT16 sX,sY;
	INT16	sXMapPos,sYMapPos;
	INT16 sScreenX,sScreenY;
	FLOAT ScrnX,ScrnY,dOffsetX,dOffsetY;
	INT8	bPoint;

	const SOLDIERTYPE* const pSoldier = g_selected_merc;
	if (pSoldier == NULL || pSoldier->ubID <= gTacticalStatus.Team[OUR_TEAM].bLastID || pSoldier->ubID >= MAXMERCS)
		return;

	// point 0 is not used!
	for ( bPoint = 1; bPoint <= pSoldier->bPatrolCnt; bPoint++ )
	{
		// Get the next point
		GridNo const sGridNo = pSoldier->usPatrolGrid[bPoint];

		if (sGridNo < 0 || sGridNo >= WORLD_MAX)
			continue;

		// Can we see it?
		if ( !GridNoOnVisibleWorldTile( sGridNo ) )
			continue;

		// Convert it's location to screen coordinates
		ConvertGridNoToXY( sGridNo, &sXMapPos, &sYMapPos );

		dOffsetX = (FLOAT)(sXMapPos * CELL_X_SIZE) - gsRenderCenterX;
		dOffsetY = (FLOAT)(sYMapPos * CELL_Y_SIZE) - gsRenderCenterY;

		FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &ScrnX, &ScrnY);

		sScreenX = ( g_ui.m_tacticalMapCenterX ) + (INT16)ScrnX;
		sScreenY = ( g_ui.m_tacticalMapCenterY ) + (INT16)ScrnY;

		// Adjust for tiles height factor!
		sScreenY -= gpWorldLevelData[sGridNo].sHeight;
		// Bring it down a touch
		sScreenY += 5;

		if( sScreenY <= 355 )
		{
			// Shown it on screen!
			UINT8 background;
			if( pSoldier->bLevel == 1 )
			{
				sScreenY -= 68;
				background = FONT_LTBLUE;
			}
			else
			{
				background = FONT_LTRED;
			}
			SetFontAttributes(TINYFONT1, FONT_WHITE, DEFAULT_SHADOW, background);
			ST::string buf = ST::format("{}", bPoint);
			FindFontCenterCoordinates(sScreenX, sScreenY, 1, 1, buf, TINYFONT1, &sX, &sY);
			MPrint(sX, sY, buf);
		}
	}
}


void SetMercOrders( INT8 bOrders )
{
	gpSelected->pSoldier->bOrders = bOrders;
	gpSelected->pBasicPlacement->bOrders = bOrders;
	UnclickEditorButtons( FIRST_MERCS_ORDERS_BUTTON, LAST_MERCS_ORDERS_BUTTON );
	ClickEditorButton( FIRST_MERCS_ORDERS_BUTTON + bOrders );
	gbDefaultOrders = bOrders;
}

void SetMercAttitude( INT8 bAttitude )
{
	gpSelected->pSoldier->bAttitude = bAttitude;
	gpSelected->pBasicPlacement->bAttitude = bAttitude;
	UnclickEditorButtons( FIRST_MERCS_ATTITUDE_BUTTON, LAST_MERCS_ATTITUDE_BUTTON );
	ClickEditorButton( FIRST_MERCS_ATTITUDE_BUTTON + bAttitude );
	gbDefaultAttitude = bAttitude;
}

void SetMercDirection( INT8 bDirection )
{
	UnclickEditorButtons( FIRST_MERCS_DIRECTION_BUTTON, LAST_MERCS_DIRECTION_BUTTON );
	ClickEditorButton( FIRST_MERCS_DIRECTION_BUTTON + bDirection );

	gbDefaultDirection = bDirection;
	gpSelected->pBasicPlacement->bDirection = bDirection;

	// ATE: Changed these to call functions....
	EVENT_SetSoldierDirection( gpSelected->pSoldier, bDirection );
	EVENT_SetSoldierDesiredDirection( gpSelected->pSoldier, bDirection );

	ConvertAniCodeToAniFrame( gpSelected->pSoldier, 0 );
}

void SetMercRelativeEquipment( INT8 bLevel )
{
	gpSelected->pBasicPlacement->bRelativeEquipmentLevel = bLevel;

	UnclickEditorButtons( FIRST_MERCS_REL_EQUIPMENT_BUTTON, LAST_MERCS_REL_EQUIPMENT_BUTTON );
	ClickEditorButton( FIRST_MERCS_REL_EQUIPMENT_BUTTON + bLevel );
	gbDefaultRelativeEquipmentLevel = bLevel;
}

void SetMercRelativeAttributes( INT8 bLevel )
{
	gpSelected->pBasicPlacement->bRelativeAttributeLevel = bLevel;
	//We also have to modify the existing soldier incase the user wishes to enter game.
	ModifySoldierAttributesWithNewRelativeLevel( gpSelected->pSoldier, bLevel );

	UnclickEditorButtons( FIRST_MERCS_REL_ATTRIBUTE_BUTTON, LAST_MERCS_REL_ATTRIBUTE_BUTTON );
	ClickEditorButton( FIRST_MERCS_REL_ATTRIBUTE_BUTTON + bLevel );
	gbDefaultRelativeAttributeLevel = bLevel;
}


static void UpdateMercItemSlots(void);
static void SetMercEditability(BOOLEAN fEditable);


void IndicateSelectedMerc( INT16 sID )
{
	SOLDIERINITNODE *prev;
	INT8 bTeam;

	//If we are trying to select a merc that is already selected, ignore.
	if( sID >= 0 && sID == gsSelectedMercGridNo )
		return;

	//first remove the cursor of the previous merc.
	//NOTE:  It doesn't matter what the value is, even if a merc isn't selected.
	//There is no need to validate the gridNo value, because it is always valid.
	RemoveAllObjectsOfTypeRange( gsSelectedMercGridNo, CONFIRMMOVE, CONFIRMMOVE );

	//This is very important, because clearing the merc editing mode actually,
	//updates the edited merc.  If this call isn't here, it is possible to update the
	//newly selected merc with the wrong information.
	SetMercEditingMode( MERC_NOMODE );

	bTeam = -1;

	//determine selection method
	switch( sID )
	{
		case SELECT_NEXT_MERC:
			prev = gpSelected;
			if (g_selected_merc == NULL || !gpSelected)
			{ //if no merc selected, then select the first one in list.
				gpSelected = gSoldierInitHead;
			}
			else
			{ //validate this merc in the list.
				if( gpSelected->next )
				{ //select the next merc in the list
					gpSelected = gpSelected->next;
				}
				else
				{ //we are at the end of the list, so select the first merc in the list.
					gpSelected = gSoldierInitHead;
				}
			}
			if( !gpSelected ) //list is empty
			{
				SetMercEditability( TRUE );
				SetMercEditingMode( MERC_TEAMMODE );
				return;
			}
			while( gpSelected != prev )
			{
				if( !gpSelected )
				{
					gpSelected = gSoldierInitHead;
					continue;
				}
				if( gpSelected->pSoldier && gpSelected->pSoldier->bVisible == 1 )
				{ //we have found a visible soldier, so select him.
					break;
				}
				gpSelected = gpSelected->next;
			}
			//we have a valid merc now.
			break;
		case SELECT_NO_MERC:
			SetMercEditability( TRUE );
			gpSelected = NULL;
			g_selected_merc  = NULL;
			gsSelectedGridNo = 0;
			SetMercEditingMode( MERC_TEAMMODE );
			return; //we already deselected the previous merc.
		case SELECT_NEXT_ENEMY:
			bTeam = ENEMY_TEAM;
			break;
		case SELECT_NEXT_CREATURE:
			bTeam = CREATURE_TEAM;
			break;
		case SELECT_NEXT_REBEL:
			bTeam = MILITIA_TEAM;
			break;
		case SELECT_NEXT_CIV:
			bTeam = CIV_TEAM;
			break;
		default:
			//search for the merc with the specific ID.
			gpSelected = FindSoldierInitNodeWithID( (UINT8)sID );
			if( !gpSelected )
			{
				g_selected_merc  = NULL;
				gsSelectedGridNo = 0;
				SetMercEditability( TRUE );
				SetMercEditingMode( MERC_TEAMMODE );
				return; //Invalid merc ID.
			}
			break;
	}
	if( bTeam != -1 )
	{ //We are searching for the next occurence of a particular team.
		prev = gpSelected;
		if (g_selected_merc == NULL || !gpSelected)
		{ //if no merc selected, then select the first one in list.
			gpSelected = gSoldierInitHead;
		}
		else
		{ //validate this merc in the list.
			if( gpSelected->next )
			{ //select the next merc in the list
				gpSelected = gpSelected->next;
			}
			else
			{ //we are at the end of the list, so select the first merc in the list.
				gpSelected = gSoldierInitHead;
			}
		}
		if( !gpSelected ) //list is empty
		{
			SetMercEditability( TRUE );
			SetMercEditingMode( MERC_TEAMMODE );
			return;
		}
		while( gpSelected != prev )
		{
			if( !gpSelected )
			{
				gpSelected = gSoldierInitHead;
				continue;
			}
			if( gpSelected->pSoldier && gpSelected->pSoldier->bVisible == 1 && gpSelected->pSoldier->bTeam == bTeam )
			{ //we have found a visible soldier on the desired team, so select him.
				break;
			}
			gpSelected = gpSelected->next;
		}
		if( !gpSelected )
			return;
		if( gpSelected == prev  )
		{ //we have cycled through the list already, so choose the same guy (if he is on the desired team)...
			if( !gpSelected->pSoldier || gpSelected->pSoldier->bVisible != 1 || gpSelected->pSoldier->bTeam != bTeam )
			{
				SetMercEditability( TRUE );
				SetMercEditingMode( MERC_TEAMMODE );
				return;
			}
		}
	}
	//if we made it this far, then we have a new merc cursor indicator to draw.
	if( gpSelected->pSoldier )
		gsSelectedMercGridNo = gpSelected->pSoldier->sGridNo;
	else
	{
		SetMercEditability( TRUE );
		SetMercEditingMode( MERC_TEAMMODE );
		return;
	}
	g_selected_merc = gpSelected->pSoldier;
	AddObjectToHead( gsSelectedMercGridNo, CONFIRMMOVE1 );

	//If the merc has a valid profile, then turn off editability
	SOLDIERCREATE_STRUCT const* const dp = gpSelected->pDetailedPlacement;
	SetMercEditability(!dp || dp->ubProfile == NO_PROFILE);

	if( sID < 0 )
	{	//We want to center the screen on the next merc, and update the interface.
		gsRenderCenterX = (INT16)gpSelected->pSoldier->dXPos;
		gsRenderCenterY = (INT16)gpSelected->pSoldier->dYPos;
		gfRenderWorld = TRUE;
	}

	//update the merc item slots to reflect what the merc currently has.
	UpdateMercItemSlots();

	//Whatever the case, we want to update the gui to press the appropriate buttons
	//depending on the merc's attributes.
	//Click the appropriate team button
	UnclickEditorButton( MERCS_ENEMY );
	UnclickEditorButton( MERCS_CREATURE );
	UnclickEditorButton( MERCS_REBEL );
	UnclickEditorButton( MERCS_CIVILIAN );
	switch( gpSelected->pSoldier->bTeam )
	{
		case ENEMY_TEAM:		ClickEditorButton( MERCS_ENEMY );			iDrawMode = DRAW_MODE_ENEMY;		break;
		case CREATURE_TEAM:	ClickEditorButton( MERCS_CREATURE );	iDrawMode = DRAW_MODE_CREATURE;	break;
		case MILITIA_TEAM:	ClickEditorButton( MERCS_REBEL );			iDrawMode = DRAW_MODE_REBEL;		break;
		case CIV_TEAM:			ClickEditorButton( MERCS_CIVILIAN );	iDrawMode = DRAW_MODE_CIVILIAN;	break;
	}
	//Update the editing mode
	if( gpSelected->pDetailedPlacement )
		SetMercEditingMode( gubLastDetailedMercMode );
	else
		SetMercEditingMode( MERC_BASICMODE );
	//Determine which team button to press.
	gfRenderMercInfo = TRUE;
	//These calls will set the proper button states, even though it redundantly
	//assigns the soldier with the same orders/attitude.
	SetMercOrders( gpSelected->pSoldier->bOrders );
	SetMercAttitude( gpSelected->pSoldier->bAttitude );
	SetMercDirection( gpSelected->pSoldier->bDirection );
	if( gpSelected->pBasicPlacement->fPriorityExistance )
		ClickEditorButton( MERCS_PRIORITYEXISTANCE_CHECKBOX );
	else
		UnclickEditorButton( MERCS_PRIORITYEXISTANCE_CHECKBOX );
	if( gpSelected->pBasicPlacement->fHasKeys )
		ClickEditorButton( MERCS_HASKEYS_CHECKBOX );
	else
		UnclickEditorButton( MERCS_HASKEYS_CHECKBOX );
	if( gpSelected->pSoldier->ubProfile == NO_PROFILE )
	{
		SetMercRelativeEquipment( gpSelected->pBasicPlacement->bRelativeEquipmentLevel );
		SetMercRelativeAttributes( gpSelected->pBasicPlacement->bRelativeAttributeLevel );
		SetEnemyColorCode( gpSelected->pBasicPlacement->ubSoldierClass );
	}
	if( iDrawMode == DRAW_MODE_CIVILIAN )
	{
		ChangeCivGroup( gpSelected->pSoldier->ubCivilianGroup );
	}
}

void DeleteSelectedMerc()
{
	if (g_selected_merc != NULL)
	{
		RemoveSoldierNodeFromInitList( gpSelected );
		gpSelected = NULL;
		g_selected_merc = NULL;
		gfRenderWorld = TRUE;
		if( TextInputMode() )
			KillTextInputMode();
		IndicateSelectedMerc( SELECT_NO_MERC );
	}
}


static ST::string CalcStringForValue(INT32 iValue, UINT32 uiMax);


static void SetupTextInputForMercProfile(void)
{
	ST::string str;
	INT16 sNum;

	InitTextInputModeWithScheme( DEFAULT_SCHEME );

	sNum = gpSelected->pDetailedPlacement->ubProfile;
	if( sNum == NO_PROFILE )
		str = ST::null;
	else
		str = CalcStringForValue(gpSelected->pDetailedPlacement->ubProfile, NUM_PROFILES);
	AddTextInputField( 200, EDITOR_TASKBAR_POS_Y + 70, 30, 20, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
}


static void SetupTextInputForMercAttributes(void)
{
	ST::string str;

	InitTextInputModeWithScheme( DEFAULT_SCHEME );

	str = CalcStringForValue(gpSelected->pDetailedPlacement->bExpLevel, 100);
	AddTextInputField( 200, EDITOR_TASKBAR_POS_Y +  5, 20, 15, MSYS_PRIORITY_NORMAL, str, 1, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bLife, 100);
	AddTextInputField( 200, EDITOR_TASKBAR_POS_Y + 30, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bLifeMax, 100);
	AddTextInputField( 200, EDITOR_TASKBAR_POS_Y + 55, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bMarksmanship, 100);
	AddTextInputField( 200, EDITOR_TASKBAR_POS_Y + 80, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bStrength, 100);
	AddTextInputField( 300, EDITOR_TASKBAR_POS_Y +  5, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bAgility, 100);
	AddTextInputField( 300, EDITOR_TASKBAR_POS_Y + 30, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bDexterity, 100);
	AddTextInputField( 300, EDITOR_TASKBAR_POS_Y + 55, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bWisdom, 100);
	AddTextInputField( 300, EDITOR_TASKBAR_POS_Y + 80, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bLeadership, 100);
	AddTextInputField( 400, EDITOR_TASKBAR_POS_Y +  5, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bExplosive, 100);
	AddTextInputField( 400, EDITOR_TASKBAR_POS_Y + 30, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bMedical, 100);
	AddTextInputField( 400, EDITOR_TASKBAR_POS_Y + 55, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bMechanical, 100);
	AddTextInputField( 400, EDITOR_TASKBAR_POS_Y + 80, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = CalcStringForValue(gpSelected->pDetailedPlacement->bMorale, 100);
	AddTextInputField( 500, EDITOR_TASKBAR_POS_Y +  5, 20, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );

	if( !gfCanEditMercs )
		DisableAllTextFields();
}


//In the merc editing, all detailed placement values for generated attributes are set to -1.
//When making a generated attribute static, we then set the value to its applicable value.
//This function is similar to the itoa function except that -1 is converted to a null string.
static ST::string CalcStringForValue(INT32 iValue, UINT32 uiMax)
{
	if( iValue < 0 )			//a blank string is determined by a negative value.
		return ST::null;
	else if( (UINT32)iValue > uiMax )	//higher than max attribute value, so convert it to the max.
		return ST::format("{}", uiMax);
	else										//this is a valid static value, so convert it to a string.
		return ST::format("{}", iValue);
}


static void ExtractAndUpdateMercAttributes(void)
{
	//If we have just deleted the merc's detailed placement in the editor, we don't
	//need to extract the information
	if( !gpSelected->pDetailedPlacement )
		return;

	//It just so happens that GetNumericStrict...() will return -1 for any blank fields.
	//-1 values in the detailed placement work nicely, because that signifies that specific
	//field isn't static.  Any other value becomes static, and static values override any
	//generated values.
	gpSelected->pDetailedPlacement->bExpLevel			= (INT8)std::min(GetNumericStrictValueFromField( 0 ), 100);
	gpSelected->pDetailedPlacement->bLife					= (INT8)std::min(GetNumericStrictValueFromField( 1 ), 100);
	gpSelected->pDetailedPlacement->bLifeMax			= (INT8)std::min(GetNumericStrictValueFromField( 2 ), 100);
	gpSelected->pDetailedPlacement->bMarksmanship	= (INT8)std::min(GetNumericStrictValueFromField( 3 ), 100);
	gpSelected->pDetailedPlacement->bStrength			= (INT8)std::min(GetNumericStrictValueFromField( 4 ), 100);
	gpSelected->pDetailedPlacement->bAgility			= (INT8)std::min(GetNumericStrictValueFromField( 5 ), 100);
	gpSelected->pDetailedPlacement->bDexterity		= (INT8)std::min(GetNumericStrictValueFromField( 6 ), 100);
	gpSelected->pDetailedPlacement->bWisdom				= (INT8)std::min(GetNumericStrictValueFromField( 7 ), 100);
	gpSelected->pDetailedPlacement->bLeadership		= (INT8)std::min(GetNumericStrictValueFromField( 8 ), 100);
	gpSelected->pDetailedPlacement->bExplosive		= (INT8)std::min(GetNumericStrictValueFromField( 9 ), 100);
	gpSelected->pDetailedPlacement->bMedical			= (INT8)std::min(GetNumericStrictValueFromField( 10 ), 100);
	gpSelected->pDetailedPlacement->bMechanical		= (INT8)std::min(GetNumericStrictValueFromField( 11 ), 100);
	gpSelected->pDetailedPlacement->bMorale				= (INT8)std::min(GetNumericStrictValueFromField( 11 ), 100);

	//make sure that experience level ranges between 1 and 9
	if( gpSelected->pDetailedPlacement->bExpLevel != -1 )
		gpSelected->pDetailedPlacement->bExpLevel = std::clamp(int(gpSelected->pDetailedPlacement->bExpLevel), 1, 9);

	//no such thing as a life max of 0
	if( !gpSelected->pDetailedPlacement->bLifeMax )
		gpSelected->pDetailedPlacement->bLifeMax = 1;

	//make sure the life doesn't exceed the maxlife...
	if( gpSelected->pDetailedPlacement->bLifeMax != -1 && gpSelected->pDetailedPlacement->bLife != -1 &&
			gpSelected->pDetailedPlacement->bLife > gpSelected->pDetailedPlacement->bLifeMax )
	{
		gpSelected->pDetailedPlacement->bLife = gpSelected->pDetailedPlacement->bLifeMax;
	}

	//update the soldier
	UpdateSoldierWithStaticDetailedInformation( gpSelected->pSoldier, gpSelected->pDetailedPlacement );
}


static void ExtractAndUpdateMercProfile(void)
{
	INT16 sNum;
	static INT16 sPrev = NO_PROFILE;

	//If we have just deleted the merc's detailed placement in the editor, we don't
	//need to extract the information
	if( !gpSelected->pDetailedPlacement )
		return;

	//if the string is blank, returning -1, then set the value to NO_PROFILE
	//because ubProfile is unsigned.
	sNum = (INT16)std::min(GetNumericStrictValueFromField( 0 ), NUM_PROFILES);
	if( sNum == -1 )
	{
		gpSelected->pDetailedPlacement->ubProfile = NO_PROFILE;
		gpSelected->pDetailedPlacement->fCopyProfileItemsOver = FALSE;
		SetMercEditability( TRUE );
	}
	else if( sPrev != sNum )
	{
		gpSelected->pDetailedPlacement->ubProfile = (UINT8)sNum;
		gpSelected->pDetailedPlacement->fCopyProfileItemsOver = TRUE;
		gpSelected->pBasicPlacement->fPriorityExistance = TRUE;
		ClickEditorButton( MERCS_PRIORITYEXISTANCE_CHECKBOX );
		SetMercEditability( FALSE );
	}
	else
		return;

	UpdateSoldierWithStaticDetailedInformation( gpSelected->pSoldier, gpSelected->pDetailedPlacement );
	if( gpSelected->pSoldier->bTeam == CIV_TEAM )
	{
		ChangeCivGroup( gpSelected->pSoldier->ubCivilianGroup );
	}
}


static void SetupTextInputForMercSchedule(void)
{
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	AddUserInputField( NULL );
	AddTextInputField(268, EDITOR_TASKBAR_POS_Y + 13, 36, 16, MSYS_PRIORITY_NORMAL, ST::null, 6, INPUTTYPE_24HOURCLOCK);
	SetExclusive24HourTimeValue( 1, gCurrSchedule.usTime[0] );
	AddTextInputField(268, EDITOR_TASKBAR_POS_Y + 34, 36, 16, MSYS_PRIORITY_NORMAL, ST::null, 6, INPUTTYPE_24HOURCLOCK);
	SetExclusive24HourTimeValue( 2, gCurrSchedule.usTime[1] );
	AddTextInputField(268, EDITOR_TASKBAR_POS_Y + 55, 36, 16, MSYS_PRIORITY_NORMAL, ST::null, 6, INPUTTYPE_24HOURCLOCK);
	SetExclusive24HourTimeValue( 3, gCurrSchedule.usTime[2] );
	AddTextInputField(268, EDITOR_TASKBAR_POS_Y + 76, 36, 16, MSYS_PRIORITY_NORMAL, ST::null, 6, INPUTTYPE_24HOURCLOCK);
	SetExclusive24HourTimeValue( 4, gCurrSchedule.usTime[3] );
}


static void UpdateScheduleInfo(void);


void ExtractAndUpdateMercSchedule()
{
	INT32 i;
	BOOLEAN fValidSchedule = FALSE;
	BOOLEAN fScheduleNeedsUpdate = FALSE;
	SCHEDULENODE *pNext = NULL;
	if( !gpSelected )
		return;
	//extract all of the fields into a temp schedulenode.
	//gScheduleNode = SCHEDULENODE{};
	for( i = 0; i < 4; i++ )
	{
		gCurrSchedule.usTime[i]	= GetExclusive24HourTimeValueFromField( (UINT8)(i+1) );
		gCurrSchedule.ubAction[i] = iEditorButton[MERCS_SCHEDULE_ACTION1 + i]->GetUserData();
		if( gCurrSchedule.ubAction[i] )
			fValidSchedule = TRUE;
	}

	if( !gpSelected->pSoldier->ubScheduleID )
	{ //The soldier doesn't actually have a schedule yet, so create one if necessary (not blank)
		if( fValidSchedule )
		{ //create a new schedule
			if( SortSchedule( &gCurrSchedule ) )
				fScheduleNeedsUpdate = TRUE;
			CopyScheduleToList( &gCurrSchedule, gpSelected );
			ShowEditorButton( MERCS_GLOWSCHEDULE );
			HideEditorButton( MERCS_SCHEDULE );
		}
	}
	else
	{
		SCHEDULENODE *pSchedule;
		pSchedule = GetSchedule( gpSelected->pSoldier->ubScheduleID );
		if( !pSchedule )
		{
			gpSelected->pSoldier->ubScheduleID = 0;
			gpSelected->pDetailedPlacement->ubScheduleID = 0;
			HideEditorButton( MERCS_GLOWSCHEDULE );
			ShowEditorButton( MERCS_SCHEDULE );
			return;
		}
		if( fValidSchedule )
		{ //overwrite the existing schedule with the new one.
			gCurrSchedule.ubScheduleID = gpSelected->pSoldier->ubScheduleID;
			if( SortSchedule( &gCurrSchedule ) )
				fScheduleNeedsUpdate = TRUE;
			pNext = pSchedule->next;
			*pSchedule = gCurrSchedule;
			pSchedule->next = pNext;
		}
		else
		{ //remove the existing schedule, as the new one is blank.
			DeleteSchedule( pSchedule->ubScheduleID );
			gpSelected->pSoldier->ubScheduleID = 0;
			gpSelected->pDetailedPlacement->ubScheduleID = 0;
			HideEditorButton( MERCS_GLOWSCHEDULE );
			ShowEditorButton( MERCS_SCHEDULE );
		}
	}
	if( fScheduleNeedsUpdate )
	{ //The schedule was sorted, so update the gui.
		UpdateScheduleInfo();
	}
	SetActiveField( 0 );
}

void ExtractCurrentMercModeInfo( BOOLEAN fKillTextInputMode )
{
	//This happens if we deleted a merc
	if (g_selected_merc == NULL) return;
	//Extract and update mercs via text fields if applicable
	switch( gubCurrMercMode )
	{
		case MERC_ATTRIBUTEMODE:
			ExtractAndUpdateMercAttributes();
			break;
		case MERC_PROFILEMODE:
			ExtractAndUpdateMercProfile();
			break;
		case MERC_SCHEDULEMODE:
			ExtractAndUpdateMercSchedule();
			break;
		default:
			fKillTextInputMode = FALSE;
			break;
	}
	if( fKillTextInputMode )
		KillTextInputMode();
}

void InitDetailedPlacementForMerc()
{
	Assert( !gpSelected->pDetailedPlacement );

	gpSelected->pDetailedPlacement = new SOLDIERCREATE_STRUCT{};
	gpSelected->pBasicPlacement->fDetailedPlacement = TRUE;
	gpSelected->pBasicPlacement->fPriorityExistance = FALSE;
	CreateStaticDetailedPlacementGivenBasicPlacementInfo( gpSelected->pDetailedPlacement, gpSelected->pBasicPlacement );

	ClearCurrentSchedule();

	//update the soldier
	UpdateSoldierWithStaticDetailedInformation ( gpSelected->pSoldier, gpSelected->pDetailedPlacement );
}

void KillDetailedPlacementForMerc()
{
	Assert( gpSelected->pDetailedPlacement );
	delete gpSelected->pDetailedPlacement;
	gpSelected->pDetailedPlacement = NULL;
	gpSelected->pBasicPlacement->fDetailedPlacement = FALSE;
	SetMercEditability( TRUE );
}


static void ChangeBodyType(INT8 const offset)
{
	Assert(offset == -1 || offset == 1); // only +/-1 allowed

	gfRenderTaskbar  = TRUE;
	gfRenderMercInfo = TRUE;

	SOLDIERINITNODE const&      sel = *gpSelected;
	BASIC_SOLDIERCREATE_STRUCT& bp  = *sel.pBasicPlacement;
	// Select next body type depending on offset
	SoldierBodyType const*      body_types; // HACK000E
	INT32                       n;          // HACK000E
	switch (bp.bTeam)
	{
		case ENEMY_TEAM:    body_types = bEnemyArray;    n = lengthof(bEnemyArray);    break;
		case CREATURE_TEAM: body_types = bCreatureArray; n = lengthof(bCreatureArray); break;
		case MILITIA_TEAM:  body_types = bRebelArray;    n = lengthof(bRebelArray);    break;
		case CIV_TEAM:      body_types = bCivArray;      n = lengthof(bCivArray);      break;
		default: abort(); // HACK000E
	}
	INT32 next = 0; // XXX HACK000E
	for (INT32 i = 0; i != n; ++i)
	{
		if (body_types[i] != bp.bBodyType) continue;
		next = i + offset;
		if      (next >= n) next = 0;
		else if (next <  0) next = n - 1;
		break;
	}
	SoldierBodyType const body_type = body_types[next];

	SOLDIERTYPE& s = *sel.pSoldier;
	// Set the new bodytype into the and update the soldier info
	if (body_type != BODY_RANDOM)
	{
		s.ubBodyType = body_type;
		// Set the flags based on the bodytype
		s.uiStatusFlags &= ~(SOLDIER_VEHICLE | SOLDIER_ROBOT | SOLDIER_ANIMAL | SOLDIER_MONSTER);
		switch (body_type)
		{
			case ADULTFEMALEMONSTER:
			case AM_MONSTER:
			case YAF_MONSTER:
			case YAM_MONSTER:
			case LARVAE_MONSTER:
			case INFANT_MONSTER:
			case QUEENMONSTER:       s.uiStatusFlags |= SOLDIER_MONSTER; break;

			case BLOODCAT:
			case COW:
			case CROW:               s.uiStatusFlags |= SOLDIER_ANIMAL;  break;

			case ROBOTNOWEAPON:      s.uiStatusFlags |= SOLDIER_ROBOT;   break;

			case HUMVEE:
			case ELDORADO:
			case ICECREAMTRUCK:
			case JEEP:
			case TANK_NW:
			case TANK_NE:            s.uiStatusFlags |= SOLDIER_VEHICLE; break;
			default:
				break;
		}
		SetSoldierAnimationSurface(&s, s.usAnimState);
	}
	// Update the placement's info as well.
	bp.bBodyType = body_type;
	if (SOLDIERCREATE_STRUCT* const dp = sel.pDetailedPlacement) dp->bBodyType = body_type;
	if (s.bTeam == CREATURE_TEAM)
	{
		gbCurrCreature = body_type;
		AssignCreatureInventory(&s);
	}
	CreateSoldierPalettes(s);
}


static void SetMercEditability(BOOLEAN fEditable)
{
	gfRenderMercInfo = TRUE;
	if( fEditable == gfCanEditMercs )
		return;
	gfCanEditMercs = fEditable;
	if( gfCanEditMercs )
	{ //enable buttons to allow editing
		EnableEditorButtons( MERCS_EQUIPMENT_BAD, MERCS_ATTRIBUTES_GREAT );
		EnableEditorButtons( FIRST_MERCS_COLORMODE_BUTTON, LAST_MERCS_COLORMODE_BUTTON );
		if( gpSelected && gpSelected->pDetailedPlacement && !gpSelected->pDetailedPlacement->fVisible )
			UnclickEditorButton( MERCS_TOGGLECOLOR_BUTTON );
		EnableEditorButton( MERCS_PRIORITYEXISTANCE_CHECKBOX );
		EnableEditorButton( MERCS_CIVILIAN_GROUP );
	}
	else
	{ //disable buttons to prevent editing
		DisableEditorButtons( MERCS_EQUIPMENT_BAD, MERCS_ATTRIBUTES_GREAT );
		DisableEditorButtons( FIRST_MERCS_COLORMODE_BUTTON, LAST_MERCS_COLORMODE_BUTTON );
		ClickEditorButton( MERCS_TOGGLECOLOR_BUTTON );
		DisableEditorButton( MERCS_PRIORITYEXISTANCE_CHECKBOX );
		DisableEditorButton( MERCS_CIVILIAN_GROUP );
	}
}


//There are 4 exclusive entry points in a map.  Only one of each type can exist on a
//map, and these points are used to validate the map by attempting to connect the four
//points together.  If one of the points is isolated, then the map will be rejected.  It
//isn't necessary to specify all four points.  You wouldn't want to specify a north point if
//there isn't going to be any traversing to adjacent maps from that side.
void SpecifyEntryPoint( UINT32 iMapIndex )
{
	INT16 *psEntryGridNo;
	BOOLEAN fErasing = FALSE;
	if( iDrawMode >= DRAW_MODE_ERASE )
	{
		iDrawMode -= DRAW_MODE_ERASE;
		fErasing = TRUE;
	}
	switch( iDrawMode )
	{
		case DRAW_MODE_NORTHPOINT:		psEntryGridNo = &gMapInformation.sNorthGridNo;		break;
		case DRAW_MODE_WESTPOINT:			psEntryGridNo = &gMapInformation.sWestGridNo;			break;
		case DRAW_MODE_EASTPOINT:			psEntryGridNo = &gMapInformation.sEastGridNo;			break;
		case DRAW_MODE_SOUTHPOINT:		psEntryGridNo = &gMapInformation.sSouthGridNo;		break;
		case DRAW_MODE_CENTERPOINT:		psEntryGridNo = &gMapInformation.sCenterGridNo;		break;
		case DRAW_MODE_ISOLATEDPOINT:	psEntryGridNo = &gMapInformation.sIsolatedGridNo;	break;
		default:																																				return;
	}
	if( !fErasing )
	{
		if( *psEntryGridNo >= 0 )
		{
			AddToUndoList( *psEntryGridNo );
			RemoveAllTopmostsOfTypeRange( *psEntryGridNo, FIRSTPOINTERS, FIRSTPOINTERS );
		}
		*psEntryGridNo = (UINT16)iMapIndex;
		ValidateEntryPointGridNo( psEntryGridNo );
		AddToUndoList( *psEntryGridNo );
		AddTopmostToTail( *psEntryGridNo, FIRSTPOINTERS2 );
	}
	else
	{
		if (FindTypeInTopmostLayer(iMapIndex, FIRSTPOINTERS))
		{
			AddToUndoList( iMapIndex );
			RemoveAllTopmostsOfTypeRange( iMapIndex, FIRSTPOINTERS, FIRSTPOINTERS );
			*psEntryGridNo = -1;
		}
		//restore the drawmode
		iDrawMode += DRAW_MODE_ERASE;
	}
}


static void AddNewItemToSelectedMercsInventory(BOOLEAN fCreate);
static void DetermineScheduleEditability(void);


void SetMercEditingMode( UINT8 ubNewMode )
{
	//We need to update the taskbar for the buttons that were erased.
	gfRenderTaskbar = TRUE;

	//set up the new mode values.
	if( gubCurrMercMode >= MERC_GENERALMODE )
		gubLastDetailedMercMode = gubCurrMercMode;

	//Depending on the mode we were just in, we may want to extract and update the
	//merc first.  Then we change modes...
	ExtractCurrentMercModeInfo( TRUE );

	//Change modes now.
	gubPrevMercMode = gubCurrMercMode;
	gubCurrMercMode = ubNewMode;

	//Hide all of the merc buttons except the team buttons which are static.
	HideEditorButtons( FIRST_MERCS_BASICMODE_BUTTON, LAST_MERCS_BUTTON );

	switch( gubPrevMercMode )
	{
		case MERC_GETITEMMODE:
			EnableEditorButtons( TAB_TERRAIN, TAB_OPTIONS );
			HideEditorButtons( FIRST_MERCS_GETITEM_BUTTON, LAST_MERCS_GETITEM_BUTTON );
			AddNewItemToSelectedMercsInventory( TRUE );
			break;
		case MERC_INVENTORYMODE:
			HideItemStatsPanel();
			DisableEditorRegion( MERC_REGION_ID );
			break;
		case MERC_GENERALMODE:
			EnableEditorButton( MERCS_APPEARANCE );
			break;
		case MERC_SCHEDULEMODE:
			//ClearCurrentSchedule();
			break;
	}

	//If we leave the merc tab, then we want to update editable fields such as
	//attributes, which was just handled above, then turn everything off, and exit.
	if( ubNewMode == MERC_NOMODE )
	{
		HideEditorButtons( FIRST_MERCS_BUTTON, LAST_MERCS_TEAMMODE_BUTTON );
		HideEditorButtons( MERCS_SCHEDULE, MERCS_GLOWSCHEDULE );
		return;
	}
	if( gubPrevMercMode == MERC_NOMODE || gubPrevMercMode == MERC_GETITEMMODE )
	{
		ShowEditorButtons( FIRST_MERCS_BUTTON, LAST_MERCS_TEAMMODE_BUTTON );
	}

	//Release the currently selected merc if you just selected a new team.
	if (g_selected_merc != NULL && ubNewMode == MERC_TEAMMODE)
	{
		//attempt to weed out conditions where we select a team that matches the currently
		//selected merc.  We don't want to deselect him in this case.
		if( (gpSelected->pSoldier->bTeam == ENEMY_TEAM && iDrawMode == DRAW_MODE_ENEMY) ||
				(gpSelected->pSoldier->bTeam == CREATURE_TEAM && iDrawMode == DRAW_MODE_CREATURE) ||
				(gpSelected->pSoldier->bTeam == MILITIA_TEAM && iDrawMode == DRAW_MODE_REBEL) ||
				(gpSelected->pSoldier->bTeam == CIV_TEAM && iDrawMode == DRAW_MODE_CIVILIAN) )
		{	//Same team, so don't deselect merc.  Instead, keep the previous editing mode
			//because we are still editing this merc.
			gubCurrMercMode = gubPrevMercMode;
			//if we don't have a detailed placement, auto set to basic mode.
			if( !gpSelected->pDetailedPlacement )
				gubCurrMercMode = MERC_BASICMODE;
		}
		else
		{
			//Different teams, so deselect the current merc and the detailed checkbox if applicable.
			IndicateSelectedMerc( SELECT_NO_MERC );
			ShowEditorButtons( FIRST_MERCS_BUTTON, LAST_MERCS_TEAMMODE_BUTTON );
			UnclickEditorButton( MERCS_DETAILEDCHECKBOX );
		}
	}

	ShowButton( iEditorButton[ MERCS_NEXT ] );
	if (g_selected_merc != NULL) ShowButton(iEditorButton[MERCS_DELETE]);

	if( gubCurrMercMode > MERC_TEAMMODE )
	{	//Add the basic buttons if applicable.
		ShowEditorButtons( FIRST_MERCS_BASICMODE_BUTTON, LAST_MERCS_BASICMODE_BUTTON );
	}
	if( gubCurrMercMode > MERC_BASICMODE )
	{ //Add the detailed buttons if applicable.
		ClickEditorButton( MERCS_DETAILEDCHECKBOX );
		ShowEditorButtons( FIRST_MERCS_PRIORITYMODE_BUTTON, LAST_MERCS_PRIORITYMODE_BUTTON );
	}
	else
		UnclickEditorButton( MERCS_DETAILEDCHECKBOX );
	//Now we are setting up the button states for the new mode, as well as show the
	//applicable buttons for the detailed placement modes.
	if( (gubCurrMercMode == MERC_APPEARANCEMODE && iDrawMode == DRAW_MODE_CREATURE) ||
			(gubCurrMercMode == MERC_SCHEDULEMODE && iDrawMode != DRAW_MODE_CIVILIAN) )
	{
		gubCurrMercMode = MERC_GENERALMODE;
	}
	switch( gubCurrMercMode )
	{
		case MERC_GETITEMMODE:
			DisableEditorButtons( TAB_TERRAIN, TAB_OPTIONS );
			EnableEditorButton( TAB_MERCS );
			HideEditorButtons( FIRST_MERCS_BUTTON, LAST_MERCS_TEAMMODE_BUTTON );
			HideEditorButtons( MERCS_SCHEDULE, MERCS_GLOWSCHEDULE );
			ShowEditorButtons( FIRST_MERCS_GETITEM_BUTTON, LAST_MERCS_GETITEM_BUTTON );
			InitEditorItemsInfo( eInfo.uiItemType );
			ClickEditorButton( ITEMS_WEAPONS + eInfo.uiItemType - TBAR_MODE_ITEM_WEAPONS );
			break;
		case MERC_INVENTORYMODE:
			UpdateMercItemSlots();
			ShowItemStatsPanel();
			if( gbCurrSelect == -1 )
				SpecifyItemToEdit( NULL, -1 );
			else
				SpecifyItemToEdit( gpMercSlotItem[ gbCurrSelect ], -1 );
			HideEditorButtons( MERCS_DELETE, MERCS_NEXT );
			ShowEditorButtons( FIRST_MERCS_INVENTORY_BUTTON, LAST_MERCS_INVENTORY_BUTTON );
			EnableEditorRegion( MERC_REGION_ID );
			UnclickEditorButtons( FIRST_MERCS_PRIORITYMODE_BUTTON, LAST_MERCS_PRIORITYMODE_BUTTON );
			ClickEditorButton( MERCS_INVENTORY );
			break;
		case MERC_BASICMODE:
			ShowEditorButtons( FIRST_MERCS_GENERAL_BUTTON, LAST_MERCS_GENERAL_BUTTON );
			if( iDrawMode == DRAW_MODE_CREATURE )
			{ //Set up alternate general mode.  This one doesn't allow you to specify relative attributes
				//but requires you to specify a body type.
				HideEditorButtons( FIRST_MERCS_REL_EQUIPMENT_BUTTON, LAST_MERCS_REL_EQUIPMENT_BUTTON );
				ShowEditorButtons( FIRST_MERCS_BODYTYPE_BUTTON, LAST_MERCS_BODYTYPE_BUTTON );
			}
			if( iDrawMode != DRAW_MODE_ENEMY )
				HideEditorButtons( FIRST_MERCS_COLORCODE_BUTTON, LAST_MERCS_COLORCODE_BUTTON );
			if( iDrawMode == DRAW_MODE_CIVILIAN )
				ShowEditorButton( MERCS_CIVILIAN_GROUP );
			break;
		case MERC_GENERALMODE:
			ShowEditorButtons( FIRST_MERCS_GENERAL_BUTTON, LAST_MERCS_GENERAL_BUTTON );
			UnclickEditorButtons( FIRST_MERCS_PRIORITYMODE_BUTTON, LAST_MERCS_PRIORITYMODE_BUTTON );
			ClickEditorButton( MERCS_GENERAL );
			if( iDrawMode == DRAW_MODE_CREATURE )
			{ //Set up alternate general mode.  This one doesn't allow you to specify relative equipment
				//but requires you to specify a body type.
				HideEditorButtons( FIRST_MERCS_REL_EQUIPMENT_BUTTON, LAST_MERCS_REL_EQUIPMENT_BUTTON );
				ShowEditorButtons( FIRST_MERCS_BODYTYPE_BUTTON, LAST_MERCS_BODYTYPE_BUTTON );
				DisableEditorButton( MERCS_APPEARANCE );
			}
			if( iDrawMode != DRAW_MODE_ENEMY )
				HideEditorButtons( FIRST_MERCS_COLORCODE_BUTTON, LAST_MERCS_COLORCODE_BUTTON );
			if( iDrawMode == DRAW_MODE_CIVILIAN )
				ShowEditorButton( MERCS_CIVILIAN_GROUP );
			break;
		case MERC_ATTRIBUTEMODE:
			UnclickEditorButtons( FIRST_MERCS_PRIORITYMODE_BUTTON, LAST_MERCS_PRIORITYMODE_BUTTON );
			ClickEditorButton( MERCS_ATTRIBUTES );
			SetupTextInputForMercAttributes();
			break;
		case MERC_APPEARANCEMODE:
			ShowEditorButtons( FIRST_MERCS_COLORMODE_BUTTON, LAST_MERCS_COLORMODE_BUTTON );
			UnclickEditorButtons( FIRST_MERCS_PRIORITYMODE_BUTTON, LAST_MERCS_PRIORITYMODE_BUTTON );
			ClickEditorButton( MERCS_APPEARANCE );
			if( gfCanEditMercs && gpSelected && gpSelected->pDetailedPlacement )
			{
				if( !gpSelected->pDetailedPlacement->fVisible )
				{
					UnclickEditorButton( MERCS_TOGGLECOLOR_BUTTON );
					DisableEditorButtons( FIRST_MERCS_COLOR_BUTTON, LAST_MERCS_COLOR_BUTTON );
				}
				else
				{
					ClickEditorButton( MERCS_TOGGLECOLOR_BUTTON );
					EnableEditorButtons( FIRST_MERCS_COLOR_BUTTON, LAST_MERCS_COLOR_BUTTON );
				}
			}
			break;
		case MERC_PROFILEMODE:
			UnclickEditorButtons( FIRST_MERCS_PRIORITYMODE_BUTTON, LAST_MERCS_PRIORITYMODE_BUTTON );
			ClickEditorButton( MERCS_PROFILE );
			SetupTextInputForMercProfile();
			break;
		case MERC_SCHEDULEMODE:
			ShowEditorButtons( MERCS_SCHEDULE_ACTION1, MERCS_SCHEDULE_VARIANCE4 );
			ShowEditorButton( MERCS_SCHEDULE_CLEAR );
			UnclickEditorButtons( FIRST_MERCS_PRIORITYMODE_BUTTON, LAST_MERCS_PRIORITYMODE_BUTTON );
			ClickEditorButton( MERCS_SCHEDULE );
			SetupTextInputForMercSchedule();
			UpdateScheduleInfo();
			DetermineScheduleEditability();
	}
	//Show or hide the schedule buttons
	if( gpSelected && gubCurrMercMode != MERC_GETITEMMODE )
	{
		if( gpSelected->pDetailedPlacement && gpSelected->pDetailedPlacement->ubScheduleID )
		{
			HideEditorButton( MERCS_SCHEDULE );
			ShowEditorButton( MERCS_GLOWSCHEDULE );
		}
		else
		{
			HideEditorButton( MERCS_GLOWSCHEDULE );
			if( gpSelected->pDetailedPlacement )
			{
				ShowEditorButton( MERCS_SCHEDULE );
				if( gpSelected->pSoldier->bTeam == CIV_TEAM )
					EnableEditorButton( MERCS_SCHEDULE );
				else
					DisableEditorButton( MERCS_SCHEDULE );
			}
			else
			{
				HideEditorButton( MERCS_SCHEDULE );
			}
		}
	}
	else
	{
		HideEditorButtons( MERCS_SCHEDULE, MERCS_GLOWSCHEDULE );
	}
}


static void DisplayBodyTypeInfo(void)
{
	ST::string str;
	switch( gpSelected->pBasicPlacement->bBodyType )
	{
		case BODY_RANDOM:         str = "Random";         break;
		case REGMALE:             str = "Reg Male";       break;
		case BIGMALE:             str = "Big Male";       break;
		case STOCKYMALE:          str = "Stocky Male";    break;
		case REGFEMALE:           str = "Reg Female";     break;
		case TANK_NE:             str = "NE Tank";        break;
		case TANK_NW:             str = "NW Tank";        break;
		case FATCIV:              str = "Fat Civilian";   break;
		case MANCIV:              str = "M Civilian";     break;
		case MINICIV:             str = "Miniskirt";      break;
		case DRESSCIV:            str = "F Civilian";     break;
		case HATKIDCIV:           str = "Kid w/ Hat";     break;
		case HUMVEE:              str = "Humvee";         break;
		case ELDORADO:            str = "Eldorado";       break;
		case ICECREAMTRUCK:       str = "Icecream Truck"; break;
		case JEEP:                str = "Jeep";           break;
		case KIDCIV:              str = "Kid Civilian";   break;
		case COW:                 str = "Domestic Cow";   break;
		case CRIPPLECIV:          str = "Cripple";        break;
		case ROBOTNOWEAPON:       str = "Unarmed Robot";  break;
		case LARVAE_MONSTER:      str = "Larvae";         break;
		case INFANT_MONSTER:      str = "Infant";         break;
		case YAF_MONSTER:         str = "Yng F Monster";  break;
		case YAM_MONSTER:         str = "Yng M Monster";  break;
		case ADULTFEMALEMONSTER:  str = "Adt F Monster";  break;
		case AM_MONSTER:          str = "Adt M Monster";  break;
		case QUEENMONSTER:        str = "Queen Monster";  break;
		case BLOODCAT:            str = "Bloodcat";       break;
	}
	DrawEditorInfoBox(str, FONT10ARIAL, 490, 4, 70, 20);
}


static void RenderMercInventoryPanel(void);


void UpdateMercsInfo()
{
	if( !gfRenderMercInfo )
		return;

	//We are rendering it now, so signify that it has been done, so
	//it doesn't get rendered every frame.
	gfRenderMercInfo = FALSE;

	switch( gubCurrMercMode )
	{
		case MERC_GETITEMMODE:
			RenderEditorItemsInfo();
			break;
		case MERC_INVENTORYMODE:
			if( gfMercGetItem )
				SetMercEditingMode( MERC_GETITEMMODE );
			else
				RenderMercInventoryPanel();
			break;
		case MERC_BASICMODE:
		case MERC_GENERALMODE:
			BltVideoObject(FRAME_BUFFER, guiExclamation, 0, 188, EDITOR_TASKBAR_POS_Y + 3);
			BltVideoObject(FRAME_BUFFER, guiKeyImage,    0, 186, EDITOR_TASKBAR_POS_Y + 28);
			SetFontAttributes(SMALLCOMPFONT, FONT_YELLOW);
			MPrint(240, EDITOR_TASKBAR_POS_Y +  3, " --=ORDERS=-- ");
			MPrint(240, EDITOR_TASKBAR_POS_Y + 59, "--=ATTITUDE=--");
			if( iDrawMode == DRAW_MODE_CREATURE )
			{
				DisplayBodyTypeInfo();
				SetFont( SMALLCOMPFONT );
				SetFontForeground( FONT_LTBLUE );
				MPrint(493, EDITOR_TASKBAR_POS_Y + 56, "RELATIVE");
				MPrint(480, EDITOR_TASKBAR_POS_Y + 62, "ATTRIBUTES");
			}
			else
			{
				SetFontForeground( FONT_LTGREEN );
				MPrint(480, EDITOR_TASKBAR_POS_Y +  3, "RELATIVE");
				MPrint(480, EDITOR_TASKBAR_POS_Y + 11, "EQUIPMENT");
				SetFontForeground( FONT_LTBLUE );
				MPrint(530, EDITOR_TASKBAR_POS_Y +  3, "RELATIVE");
				MPrint(530, EDITOR_TASKBAR_POS_Y + 11, "ATTRIBUTES");
			}
			if( iDrawMode == DRAW_MODE_ENEMY )
			{
				SetFont( FONT10ARIAL );
				SetFontForeground( FONT_YELLOW );
				MPrint(590, EDITOR_TASKBAR_POS_Y + 51, "Army");
				MPrint(590, EDITOR_TASKBAR_POS_Y + 65, "Admin");
				MPrint(590, EDITOR_TASKBAR_POS_Y + 79, "Elite");
			}
			break;
		case MERC_ATTRIBUTEMODE:
			SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
			MPrint(225, EDITOR_TASKBAR_POS_Y + 10, "Exp. Level");
			MPrint(225, EDITOR_TASKBAR_POS_Y + 35, "Life");
			MPrint(225, EDITOR_TASKBAR_POS_Y + 60, "LifeMax");
			MPrint(225, EDITOR_TASKBAR_POS_Y + 85, "Marksmanship");
			MPrint(325, EDITOR_TASKBAR_POS_Y + 10, "Strength");
			MPrint(325, EDITOR_TASKBAR_POS_Y + 35, "Agility");
			MPrint(325, EDITOR_TASKBAR_POS_Y + 60, "Dexterity");
			MPrint(325, EDITOR_TASKBAR_POS_Y + 85, "Wisdom");
			MPrint(425, EDITOR_TASKBAR_POS_Y + 10, "Leadership");
			MPrint(425, EDITOR_TASKBAR_POS_Y + 35, "Explosives");
			MPrint(425, EDITOR_TASKBAR_POS_Y + 60, "Medical");
			MPrint(425, EDITOR_TASKBAR_POS_Y + 85, "Mechanical");
			MPrint(525, EDITOR_TASKBAR_POS_Y + 10, "Morale");
			break;

		case MERC_APPEARANCEMODE:
		{
			SOLDIERCREATE_STRUCT const& dp         = *gpSelected->pDetailedPlacement;
			UINT8                const  foreground =
				dp.fVisible || dp.ubProfile != NO_PROFILE ? FONT_YELLOW : FONT_DKYELLOW;
			SetFontAttributes(FONT10ARIAL, foreground);

			MPrint(396, EDITOR_TASKBAR_POS_Y +  4, "Hair color:");
			MPrint(396, EDITOR_TASKBAR_POS_Y + 28, "Skin color:");
			MPrint(396, EDITOR_TASKBAR_POS_Y + 52, "Vest color:");
			MPrint(396, EDITOR_TASKBAR_POS_Y + 76, "Pant color:");

			SetFont( SMALLCOMPFONT );
			SetFontForeground( FONT_BLACK );
			if( gpSelected->pDetailedPlacement->fVisible || gpSelected->pDetailedPlacement->ubProfile != NO_PROFILE )
			{
				MPrintEditor(396, 14, ST::format("{}    ", gpSelected->pSoldier->HeadPal));
				MPrintEditor(396, 38, ST::format("{}    ", gpSelected->pSoldier->SkinPal));
				MPrintEditor(396, 62, ST::format("{}    ", gpSelected->pSoldier->VestPal));
				MPrintEditor(396, 86, ST::format("{}    ", gpSelected->pSoldier->PantsPal));
				ShowEditMercPalettes( gpSelected->pSoldier );
			}
			else
			{
				MPrint(396, EDITOR_TASKBAR_POS_Y + 14, "RANDOM");
				MPrint(396, EDITOR_TASKBAR_POS_Y + 38, "RANDOM");
				MPrint(396, EDITOR_TASKBAR_POS_Y + 62, "RANDOM");
				MPrint(396, EDITOR_TASKBAR_POS_Y + 86, "RANDOM");
				ShowEditMercPalettes( NULL ); //will display grey scale to signify random
			}
			DisplayBodyTypeInfo();
			break;
		}

		case MERC_PROFILEMODE:
			{ //scope trick
				ST::string tempStr = ST::format(
					"By specifying a profile index, all of the information will be extracted from the profile "
					"and override any values that you have edited.  It will also disable the editing features "
					"though, you will still be able to view stats, etc.  Pressing ENTER will automatically "
					"extract the number you have typed.  A blank field will clear the profile.  The current "
					"number of profiles range from 0 to {}.", NUM_PROFILES);
				SetFontShadow(FONT_NEARBLACK);
				DisplayWrappedString(180, EDITOR_TASKBAR_POS_Y + 10, 400, 2, FONT10ARIAL, 146, tempStr,	FONT_BLACK, LEFT_JUSTIFIED);
				SetFont( FONT12POINT1 );
				if( gpSelected->pDetailedPlacement->ubProfile == NO_PROFILE )
				{
					SetFontForeground( FONT_GRAY3 );
					MPrintEditor(240, 75, "Current Profile:  n/a                            ");
				}
				else
				{
					SetFontForeground( FONT_WHITE );
					ClearTaskbarRegion(240, 75, 580, 85);
					MPrint(240, EDITOR_TASKBAR_POS_Y + 75, ST::format("Current Profile:  {}", gMercProfiles[gpSelected->pDetailedPlacement->ubProfile].zName));
				}
			}
			break;
		case MERC_SCHEDULEMODE:
			SetFontAttributes(FONT10ARIAL, FONT_WHITE);
			switch( gpSelected->pSoldier->bOrders )
			{
				case STATIONARY:  MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "STATIONARY");    break;
				case ONCALL:      MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "ON CALL");       break;
				case ONGUARD:     MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "ON GUARD");      break;
				case SEEKENEMY:   MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "SEEK ENEMY");    break;
				case CLOSEPATROL: MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "CLOSE PATROL");  break;
				case FARPATROL:   MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "FAR PATROL");    break;
				case POINTPATROL: MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "POINT PATROL");  break;
				case RNDPTPATROL: MPrint(430, EDITOR_TASKBAR_POS_Y +  3, "RND PT PATROL"); break;
			}
			SetFontForeground( FONT_YELLOW );
			MPrint(186, EDITOR_TASKBAR_POS_Y +  3, "Action");
			MPrint(268, EDITOR_TASKBAR_POS_Y +  3, "Time");
			MPrint(309, EDITOR_TASKBAR_POS_Y +  3, "V");
			MPrint(331, EDITOR_TASKBAR_POS_Y +  3, "GridNo 1");
			MPrint(381, EDITOR_TASKBAR_POS_Y +  3, "GridNo 2");
			MPrint(172, EDITOR_TASKBAR_POS_Y + 16, "1)");
			MPrint(172, EDITOR_TASKBAR_POS_Y + 37, "2)");
			MPrint(172, EDITOR_TASKBAR_POS_Y + 58, "3)");
			MPrint(172, EDITOR_TASKBAR_POS_Y + 79, "4)");
			if( gubScheduleInstructions )
			{
				ST::string str;
				ST::string keyword;
				ColorFillVideoSurfaceArea( FRAME_BUFFER, 431, EDITOR_TASKBAR_POS_Y + 28, 590, EDITOR_TASKBAR_POS_Y + 90, Get16BPPColor( FROMRGB( 32, 45, 72 ) ) );
				switch( gCurrSchedule.ubAction[ gubCurrentScheduleActionIndex ] )
				{
					case SCHEDULE_ACTION_LOCKDOOR:   keyword = "lock";   break;
					case SCHEDULE_ACTION_UNLOCKDOOR: keyword = "unlock"; break;
					case SCHEDULE_ACTION_OPENDOOR:   keyword = "open";   break;
					case SCHEDULE_ACTION_CLOSEDOOR:  keyword = "close";  break;
				}
				switch( gubScheduleInstructions )
				{
					case SCHEDULE_INSTRUCTIONS_DOOR1:
						str = ST::format("Click on the gridno adjacent to the door that you wish to {}.", keyword);
						break;
					case SCHEDULE_INSTRUCTIONS_DOOR2:
						str = ST::format("Click on the gridno where you wish to move after you {} the door.", keyword);
						break;
					case SCHEDULE_INSTRUCTIONS_GRIDNO:
						str = "Click on the gridno where you wish to move to.";
						break;
					case SCHEDULE_INSTRUCTIONS_SLEEP:
						str = "Click on the gridno where you wish to sleep at.  Person will automatically return to original position after waking up.";
					default:
						return;
				}
				str += "  Hit ESC to abort entering this line in the schedule.";
				DisplayWrappedString(436, EDITOR_TASKBAR_POS_Y + 32, 149, 2, FONT10ARIAL, FONT_YELLOW, str, FONT_BLACK, LEFT_JUSTIFIED);
			}
			break;
	}
}

//When a detailed placement merc is in the inventory panel, there is a overall region
//blanketing this panel.  As the user moves the mouse around and clicks, etc., this function
//is called by the region callback functions to handle these cases.  The event types are defined
//in Editor Taskbar Utils.h.  Here are the internal functions...

SGPRect mercRects[9] =
{
	{  75,  0, 104, 19 }, //head
	{  75, 22, 104, 41 }, //body
	{  76, 73, 105, 92 }, //legs
	{  26, 43,  78, 62 }, //left hand
	{ 104, 42, 156, 61 }, //right hand
	{ 180,  6, 232, 25 }, //pack 1
	{ 180, 29, 232, 48 }, //pack 2
	{ 180, 52, 232, 71 }, //pack 3
	{ 180, 75, 232, 94 }  //pack 4
};


static BOOLEAN PointInRect(SGPRect* pRect, INT32 x, INT32 y)
{
	return( x >= pRect->iLeft && x <= pRect->iRight && y >= pRect->iTop && y <= pRect->iBottom );
}


static void DrawRect(SGPRect* pRect, INT16 color)
{
	SGPVSurface::Lock l(FRAME_BUFFER);
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	RectangleDraw(TRUE, pRect->iLeft + MERCPANEL_X, pRect->iTop + MERCPANEL_Y, pRect->iRight + MERCPANEL_X, pRect->iBottom + MERCPANEL_Y, color, l.Buffer<UINT16>());
}


static void RenderSelectedMercsInventory(void)
{
	INT32 i;
	INT32 xp, yp;
	UINT8 ubFontColor;
	if (g_selected_merc == NULL) return;
	for( i = 0; i < 9; i++ )
	{
		if( gpMercSlotItem[i] )
		{ //Render the current image.
			xp = mercRects[ i ].iLeft + 4 + MERCPANEL_X;
			yp = mercRects[ i ].iTop + MERCPANEL_Y;
			BltVideoSurface(FRAME_BUFFER, guiMercInvPanelBuffers[i], xp, yp, NULL);
			//Render the text
			switch( i )
			{
				case 2: //legs (to the right of the box, but move it down to make room for right hand text)
					xp = mercRects[i].iRight + 2;
					yp = mercRects[i].iTop + 8;
					break;
				case 3: //left hand (underneath box and to the left -- obscurred by checkbox)
					xp = mercRects[i].iLeft - 20;
					yp = mercRects[i].iBottom + 2;
					break;
				case 4: //right hand (underneath box)
					xp = mercRects[i].iLeft;
					yp = mercRects[i].iBottom + 2;
					break;
				default: //normal cases (to the right of the box)
					xp = mercRects[i].iRight + 2;
					yp = mercRects[i].iTop;
					break;
			}
			xp += MERCPANEL_X;
			yp += MERCPANEL_Y;
			if( i == gbCurrSelect )
				ubFontColor = FONT_LTRED;
			else if( i == gbCurrHilite )
				ubFontColor = FONT_YELLOW;
			else
				ubFontColor = FONT_WHITE;
			DisplayWrappedString(xp, yp, 60, 2, SMALLCOMPFONT, ubFontColor, GCM->getItem(gpMercSlotItem[i]->usItem)->getName(), 0, LEFT_JUSTIFIED);
		}
	}
}

void DeleteSelectedMercsItem()
{
	if( gbCurrSelect != -1 )
	{
		gusMercsNewItemIndex = 0;
		AddNewItemToSelectedMercsInventory( TRUE );
	}
}

//This function does two main things:
// 1)  Allows a new item to be created via usItem and assigned to the currently selected merc.
// 2)  Converts the image from interface size to the smaller panel used by the editor.  The slots
//     in the editor are approximately 80% of that size.  This involves scaling calculations.  These
//     images are saved in individual slots are are blitted to the screen during rendering, not here.
// NOTE:  Step one can be skipped (when selecting an existing merc).  By setting the
static void AddNewItemToSelectedMercsInventory(BOOLEAN fCreate)
{
	UINT16 uDstWidth, uDstHeight;
	float rScalar, rWidthScalar, rHeightScalar;
	BOOLEAN fUnDroppable;

	if( fCreate )
	{
		/*
		if( gpMercSlotItem[ gbCurrSelect ] && gpMercSlotItem[ gbCurrSelect ]->usItem == gusMercsNewItemIndex )
		{ //User selected same item, so ignore.
			gusMercsNewItemIndex = 0xffff;
			return;
		}
		*/
		if( gusMercsNewItemIndex == 0xffff )
		{ //User selected no item, so ignore.
			return;
		}
		//Create the item, and set up the slot.
		fUnDroppable = gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ].fFlags & OBJECT_UNDROPPABLE ? TRUE : FALSE;

		if ( GCM->getItem(gusMercsNewItemIndex)->getItemClass() == IC_KEY )
		{
			CreateKeyObject( &gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ], 1, (UINT8) eInfo.sSelItemIndex );
		}
		else
		{
			CreateItem( gusMercsNewItemIndex, 100, &gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ] );
		}
		if( fUnDroppable )
		{
			gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ].fFlags |= OBJECT_UNDROPPABLE;
		}

		//randomize the status on non-ammo items.
		if( !(GCM->getItem(gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ].usItem)->isAmmo()) )
			gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ].bStatus[0] = (INT8)(80 + Random( 21 ));

		if( gusMercsNewItemIndex )
		{
			gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ].fFlags |= OBJECT_NO_OVERWRITE;
		}
	}
	//allow the slot to point to the selected merc's inventory for editing/rendering purposes.
	gpMercSlotItem[ gbCurrSelect ] = &gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ gbCurrSelect ] ];

	if( !fCreate )
	{ //it is possible to have a null item which we don't want to blit!  Also, we need to set the
		//new item index, so that it can extract the item's image using that.
		gusMercsNewItemIndex = gpMercSlotItem[ gbCurrSelect ]->usItem;
		if( !gpMercSlotItem[ gbCurrSelect ] )
			return;
	}
	//GOAL:
	//From here on, we are going to first render the new item into a temp buffer, then crop the image in
	//the buffer and scale it down to fit into it's associated slot in the panel (which on average will
	//require scaling the item by 80%).  We have to do a bunch of calculations to override the offsets, etc.
	//Each slot has it's own smaller version buffer, and this is what gets drawn when the rendering happens.

	//assign the buffers
	SGPVSurface* const uiSrcID = guiMercTempBuffer;
	SGPVSurface* const uiDstID = guiMercInvPanelBuffers[gbCurrSelect];

	//build the rects
	uDstWidth = gbCurrSelect < 3 ? MERCINV_SMSLOT_WIDTH : MERCINV_LGSLOT_WIDTH;
	uDstHeight = MERCINV_SLOT_HEIGHT;
	SGPRect	SrcRect;
	SGPRect DstRect;
	SrcRect.iLeft = 0;
	SrcRect.iTop = 0;
	SrcRect.iRight = 60;
	SrcRect.iBottom = 25;
	DstRect.iLeft = 0;
	DstRect.iTop = 0;
	DstRect.iRight = uDstWidth;
	DstRect.iBottom = uDstHeight;

	//clear both buffers (fill with black to erase previous graphic)
	ColorFillVideoSurfaceArea( uiSrcID, SrcRect.iLeft, SrcRect.iTop, SrcRect.iRight, SrcRect.iBottom, 0 );
	ColorFillVideoSurfaceArea( uiDstID, DstRect.iLeft, DstRect.iTop, DstRect.iRight, DstRect.iBottom, 0 );

	//if the index is 0, then there is no item.
	if( !gusMercsNewItemIndex )
		return;

	//now draw the fullsize item into the temp buffer
	const ItemModel * item = GCM->getItem(gusMercsNewItemIndex);
	auto graphic = GetSmallInventoryGraphicForItem(item);
	auto vo = graphic.first;
	auto index = graphic.second;
	BltVideoObjectOutline(uiSrcID, vo, index, 0, 0, SGP_TRANSPARENT);

	//crop the source image
	ETRLEObject const& pObject    = vo->SubregionProperties(index);
	UINT16      const  uSrcWidth  = pObject.usWidth;
	UINT16      const  uSrcHeight = pObject.usHeight;
	SGPBox      const  src_rect   =
	{
		(UINT16)(SrcRect.iLeft +	pObject.sOffsetX),
		(UINT16)(SrcRect.iTop  +	pObject.sOffsetY),
		uSrcWidth,
		uSrcHeight
	};

	//if the source image width is less than 30 (small slot), then modify the DstRect.
	if( uSrcWidth < 30 )
		uDstWidth = MERCINV_SMSLOT_WIDTH;
	else
		uDstWidth = MERCINV_LGSLOT_WIDTH;

	//compare the sizes of the cropped image to the destination buffer size, and calculate the
	//scalar value.  It is possible to have scalars > 1.0, in which case, we change it to 1.0 and
	//use the other value.
	rWidthScalar =  (float)uDstWidth/(float)uSrcWidth;
	if( rWidthScalar > 1.0 )
		rWidthScalar = 1.0;
	rHeightScalar = (float)uDstHeight/(float)uSrcHeight;
	if( rHeightScalar > 1.0 )
		rHeightScalar = 1.0;

	//determine which scalar to use.
	if( rWidthScalar == 1.0 )
		rScalar = rHeightScalar;
	else if( rHeightScalar == 1.0 )
		rScalar = rWidthScalar ;
	else
		rScalar = std::max(rWidthScalar, rHeightScalar);

	//apply the scalar to the destination width and height
	uDstWidth = (UINT16)( uSrcWidth * rScalar );
	uDstHeight = (UINT16)( uSrcHeight * rScalar );

	//sometimes it is possible to scale too big, so clip if applicable
	if( uDstWidth > MERCINV_LGSLOT_WIDTH )
		uDstWidth = MERCINV_LGSLOT_WIDTH;
	else if( gbCurrSelect < 3 && uDstWidth > MERCINV_SMSLOT_WIDTH )
		uDstWidth = MERCINV_SMSLOT_WIDTH;
	if( uDstHeight > MERCINV_SLOT_HEIGHT )
		uDstHeight = MERCINV_SLOT_HEIGHT;

	//use the new width and height values to calculate the new dest rect (center the item)
	SGPBox const dst_rect =
	{
		(UINT16)((DstRect.iRight  - DstRect.iLeft - uDstWidth)  / 2),
		(UINT16)((DstRect.iBottom - DstRect.iTop  - uDstHeight) / 2),
		uDstWidth,
		uDstHeight
	};

	//scale the item down to the smaller buffer.
	BltStretchVideoSurface(uiDstID, uiSrcID, &src_rect, &dst_rect);

	//invalidate the mercs new item index
	gusMercsNewItemIndex = 0xffff;
}


static void RenderMercInventoryPanel(void)
{
	INT32 x;
	//Draw the graphical panel
	BltVideoObject(FRAME_BUFFER, guiMercInventoryPanel, 0, MERCPANEL_X, MERCPANEL_Y);
	//Mark the buttons dirty, so they don't disappear.
	for( x = FIRST_MERCS_INVENTORY_BUTTON; x <= LAST_MERCS_INVENTORY_BUTTON; x++ )
	{
		MarkAButtonDirty( iEditorButton[ x ] );
	}
	RenderButtons();
	if( gbCurrHilite != -1 )
		DrawRect( &mercRects[ gbCurrHilite ], Get16BPPColor( FROMRGB( 200, 200, 0 ) ) );
	if( gbCurrSelect != -1 )
		DrawRect( &mercRects[ gbCurrSelect ], Get16BPPColor( FROMRGB( 200,   0, 0 ) ) );
	RenderSelectedMercsInventory();
	InvalidateRegion( MERCPANEL_X, MERCPANEL_Y, 475, EDITOR_TASKBAR_POS_Y + 100 );
	UpdateItemStatsPanel();
}


//This function is called by the move and click callback functions for the region blanketing the
//9 slots in the inventory panel.  It passes the event type as well as the relative x and y positions
//which are processed here.  This basically checks for new changes in hilighting and selections, which
//will set the rendering flag, and getitem flag if the user wishes to choose an item.
void HandleMercInventoryPanel( INT16 sX, INT16 sY, INT8 bEvent )
{
	INT8 x;
	if( !gfCanEditMercs && bEvent == GUI_RCLICK_EVENT )
	{ //if we are dealing with a profile merc, we can't allow editing
		//of items, but we can look at them.  So, treat all right clicks
		//as if they were left clicks.
		bEvent = GUI_LCLICK_EVENT;
	}
	switch( bEvent )
	{
		case GUI_MOVE_EVENT:
			//user is moving the mouse around the panel, so determine which slot
			//needs to be hilighted yellow.
			for( x = 0; x < 9; x++ )
			{
				if( PointInRect( &mercRects[x], sX, sY ) )
				{
					if( gbCurrHilite != x ) //only render if the slot isn't the same one.
						gfRenderMercInfo = TRUE;
					gbCurrHilite = x;
					return;
				}
			}
			//if we don't find a valid slot, then we need to turn it off.
			if( gbCurrHilite != -1 )
			{ //only turn off if it isn't already off.  This avoids unecessary rendering.
				gbCurrHilite = -1;
				gfRenderMercInfo = TRUE;
			}
			break;
		case GUI_LCLICK_EVENT:
		case GUI_RCLICK_EVENT:
			//The user has clicked in the inventory panel.  Determine if he clicked in
			//a slot.  Left click selects the slot for editing, right clicking enables
			//the user to choose an item for that slot.
			for( x = 0; x < 9; x++ )
			{
				if( PointInRect( &mercRects[x], sX, sY ) )
				{
					if( gbCurrSelect != x ) //only if it isn't the same slot.
					{
						gfRenderMercInfo = TRUE;
						if( bEvent == GUI_LCLICK_EVENT )
							SpecifyItemToEdit( gpMercSlotItem[ x ], -1 );
					}
					if( bEvent == GUI_RCLICK_EVENT ) //user r-clicked, so enable item choosing
						gfMercGetItem = TRUE;
					gbCurrSelect = x;
					return;
				}
			}
			break;
	}
}


static void SetDroppableCheckboxesBasedOnMercsInventory(void);


/* When a new merc is selected, this function sets up all of the information
 * for the slots, selections, and hilites. */
static void UpdateMercItemSlots(void)
{
	INT8 x;
	if( !gpSelected->pDetailedPlacement )
	{
		for( x = 0; x < 9; x++ )
		{
			gpMercSlotItem[ x ] = NULL;
		}
	}
	else
	{
		if( gpSelected->pDetailedPlacement->ubProfile != NO_PROFILE )
		{
			memcpy( gpSelected->pDetailedPlacement->Inv, gpSelected->pSoldier->inv, sizeof( OBJECTTYPE ) * NUM_INV_SLOTS );
		}
		for( x = 0; x < 9; x++ )
		{
			//Set the curr select and the addnewitem function will handle the rest, including rebuilding
			//the nine slot buffers, etc.
			gbCurrSelect = x;
			AddNewItemToSelectedMercsInventory( FALSE );
		}
	}
	SetDroppableCheckboxesBasedOnMercsInventory();
	SpecifyItemToEdit( NULL, -1 );
	gbCurrSelect = -1;
	gbCurrHilite = -1;
}


static void SetDroppableCheckboxesBasedOnMercsInventory(void)
{
	OBJECTTYPE *pItem;
	INT32 i;
	if( gpSelected && gpSelected->pDetailedPlacement )
	{
		for( i = 0; i < 9; i++ )
		{
			pItem = &gpSelected->pDetailedPlacement->Inv[ gbMercSlotTypes[ i ] ];
			if( pItem->fFlags & OBJECT_UNDROPPABLE )
			{	//check box is clear
				UnclickEditorButton( MERCS_HEAD_SLOT + i );
			}
			else
			{
				ClickEditorButton( MERCS_HEAD_SLOT + i );
			}
		}
	}
}


void SetEnemyColorCode(UINT8 const colour_code)
{
	SOLDIERINITNODE       const& sel = *gpSelected;
	SOLDIERCREATE_STRUCT* const  dp  = sel.pDetailedPlacement;
	if (dp && dp->ubProfile != NO_PROFILE) return;

	UnclickEditorButtons(FIRST_MERCS_COLORCODE_BUTTON, LAST_MERCS_COLORCODE_BUTTON);
	char const* vest;
	char const* pants;
	switch (colour_code)
	{
		case SOLDIER_CLASS_ARMY:
			ClickEditorButton(MERCS_ARMY_CODE);
			vest  = "REDVEST";
			pants = "GREENPANTS";
			break;

		case SOLDIER_CLASS_ADMINISTRATOR:
			ClickEditorButton(MERCS_ADMIN_CODE);
			vest  = "BLUEVEST";
			pants = "BLUEPANTS";
			break;

		case SOLDIER_CLASS_ELITE:
			ClickEditorButton(MERCS_ELITE_CODE);
			vest  = "BLACKSHIRT";
			pants = "BLACKPANTS";
			break;

		case SOLDIER_CLASS_MINER:
			vest  = "greyVEST";
			pants = "BEIGEPANTS";
			break;

		default: return;
	}
	gubSoldierClass                     = colour_code;
	sel.pBasicPlacement->ubSoldierClass = colour_code;
	if (dp) dp->ubSoldierClass = colour_code;
	SOLDIERTYPE& s = *sel.pSoldier;
	s.VestPal  = vest;
	s.PantsPal = pants;
	CreateSoldierPalettes(s);
}


void SetEnemyDroppableStatus( UINT32 uiSlot, BOOLEAN fDroppable )
{
	if( gpSelected )
	{
		if( fDroppable )
		{
			if( gpSelected->pDetailedPlacement )
				gpSelected->pDetailedPlacement->Inv[ uiSlot ].fFlags &= (~OBJECT_UNDROPPABLE);
			if( gpSelected->pSoldier )
				gpSelected->pSoldier->inv[ uiSlot ].fFlags &= (~OBJECT_UNDROPPABLE);
		}
		else
		{
			if( gpSelected->pDetailedPlacement )
				gpSelected->pDetailedPlacement->Inv[ uiSlot ].fFlags |= OBJECT_UNDROPPABLE;
			if( gpSelected->pSoldier )
				gpSelected->pSoldier->inv[ uiSlot ].fFlags |= OBJECT_UNDROPPABLE;
		}
	}
	if( gbCurrSelect != -1 && uiSlot == (UINT32)gbMercSlotTypes[ gbCurrSelect ] )
	{
		if( gpMercSlotItem[ gbCurrSelect ]->usItem == NOTHING )
			SpecifyItemToEdit( gpMercSlotItem[ gbCurrSelect ], -1 );
	}
}

void ChangeCivGroup( UINT8 ubNewCivGroup )
{
	Assert( ubNewCivGroup < NUM_CIV_GROUPS );
	if( gubCivGroup == ubNewCivGroup )
		return;
	gubCivGroup = ubNewCivGroup;
	if( gpSelected && gpSelected->pSoldier )
	{
		gpSelected->pBasicPlacement->ubCivilianGroup = gubCivGroup;
		if( gpSelected->pDetailedPlacement )
			gpSelected->pDetailedPlacement->ubCivilianGroup = gubCivGroup;
		gpSelected->pSoldier->ubCivilianGroup = gubCivGroup;
	}
	//Adjust the text on the button
	iEditorButton[MERCS_CIVILIAN_GROUP]->SpecifyText(gszCivGroupNames[gubCivGroup]);
}


static void RenderCurrentSchedule(void);


void RenderMercStrings()
{
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		SOLDIERTYPE const* const s = curr->pSoldier;
		if (!s || s->bVisible != 1) continue;

		INT16 sX;
		INT16 sY;
		INT16 sXPos;
		INT16 sYPos;
		GetSoldierAboveGuyPositions(s, &sXPos, &sYPos, FALSE);

		// Display name
		SetFontAttributes(TINYFONT1, FONT_WHITE);
		if (s->ubProfile != NO_PROFILE)
		{
			FindFontCenterCoordinates(sXPos, sYPos, 80, 1, s->name, TINYFONT1, &sX, &sY);
			if (sY < 352)
			{
				GDirtyPrint(sX, sY, s->name);
			}
			sYPos += 10;
		}

		// Render the health text
		ST::string health = GetSoldierHealthString(s);
		SetFontForeground(FONT_RED);
		FindFontCenterCoordinates(sXPos, sYPos, 80, 1, health, TINYFONT1, &sX, &sY);
		if (sY < 352)
		{
			GDirtyPrint(sX, sY, health);
		}
		sYPos += 10;

		SetFontForeground(FONT_GRAY2);
		ST::string str = ST::format("Slot #{}", s->ubID);
		FindFontCenterCoordinates(sXPos, sYPos, 80, 1, str, TINYFONT1, &sX, &sY);
		if (sY < 352)
		{
			GDirtyPrint(sX, sY, str);
		}
		sYPos += 10;

		// Make sure the placement has at least one waypoint.
		BASIC_SOLDIERCREATE_STRUCT const& bp = *curr->pBasicPlacement;
		bool const has_patrol_order  = bp.bOrders == RNDPTPATROL || bp.bOrders == POINTPATROL;
		bool const has_patrol_points = bp.bPatrolCnt != 0;
		if (has_patrol_order != has_patrol_points)
		{
			SetFontForeground(GetJA2Clock() % 1000 < 500 ? FONT_DKRED : FONT_RED);
			ST::string msg = has_patrol_points ?
				"Waypoints with no patrol orders" :
				"Patrol orders with no waypoints";
			FindFontCenterCoordinates(sXPos, sYPos, 80, 1, msg, TINYFONT1, &sX, &sY);
			if (sY < 352)
			{
				GDirtyPrint(sX, sY, msg);
			}
		}
	}
	if (gubCurrMercMode == MERC_SCHEDULEMODE)
	{
		RenderCurrentSchedule();
	}
}


void SetMercTeamVisibility( INT8 bTeam, BOOLEAN fVisible )
{
	INT8 bVisible;
	bVisible = fVisible ? 1 : -1;
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		if( curr->pBasicPlacement->bTeam == bTeam )
		{
			if( curr->pSoldier )
			{
				curr->pSoldier->bLastRenderVisibleValue = bVisible;
				curr->pSoldier->bVisible = bVisible;
			}
		}
	}
	if( gpSelected && gpSelected->pSoldier && gpSelected->pSoldier->bTeam == bTeam && !fVisible )
	{
		IndicateSelectedMerc( SELECT_NO_MERC );
	}
}


static void DetermineScheduleEditability(void)
{
	INT32 i;
	EnableEditorButtons( MERCS_SCHEDULE_ACTION1, MERCS_SCHEDULE_DATA4B );
	EnableTextFields( 1, 4 );
	for( i = 0; i < 4; i++ )
	{
		switch( gCurrSchedule.ubAction[i] )
		{
			case SCHEDULE_ACTION_NONE:
			case SCHEDULE_ACTION_LEAVESECTOR:
				EnableEditorButton( MERCS_SCHEDULE_ACTION1 + i );
				EnableEditorButton( MERCS_SCHEDULE_VARIANCE1 + i );
				HideEditorButton( MERCS_SCHEDULE_DATA1A + i );
				HideEditorButton( MERCS_SCHEDULE_DATA1B + i );
				break;
			case SCHEDULE_ACTION_LOCKDOOR:
			case SCHEDULE_ACTION_UNLOCKDOOR:
			case SCHEDULE_ACTION_OPENDOOR:
			case SCHEDULE_ACTION_CLOSEDOOR:
				EnableEditorButton( MERCS_SCHEDULE_ACTION1 + i );
				EnableEditorButton( MERCS_SCHEDULE_VARIANCE1 + i );
				ShowEditorButton( MERCS_SCHEDULE_DATA1A + i );
				ShowEditorButton( MERCS_SCHEDULE_DATA1B + i );
				EnableEditorButton( MERCS_SCHEDULE_DATA1A + i );
				EnableEditorButton( MERCS_SCHEDULE_DATA1B + i );
				break;
			case SCHEDULE_ACTION_GRIDNO:
			case SCHEDULE_ACTION_ENTERSECTOR:
			case SCHEDULE_ACTION_SLEEP:
				EnableEditorButton( MERCS_SCHEDULE_ACTION1 + i );
				EnableEditorButton( MERCS_SCHEDULE_VARIANCE1 + i );
				ShowEditorButton( MERCS_SCHEDULE_DATA1A + i );
				HideEditorButton( MERCS_SCHEDULE_DATA1B + i );
				EnableEditorButton( MERCS_SCHEDULE_DATA1A + i );
				break;
			case SCHEDULE_ACTION_STAYINSECTOR:
				DisableTextField( (UINT8)(i+1) );
				EnableEditorButton( MERCS_SCHEDULE_ACTION1 + i );
				DisableEditorButton( MERCS_SCHEDULE_VARIANCE1 + i );
				HideEditorButton( MERCS_SCHEDULE_DATA1A + i );
				HideEditorButton( MERCS_SCHEDULE_DATA1B + i );
				break;
		}
	}
}

void CancelCurrentScheduleAction()
{
	UpdateScheduleAction( SCHEDULE_ACTION_NONE );
	DetermineScheduleEditability();
}

void RegisterCurrentScheduleAction( INT32 iMapIndex )
{
	MarkWorldDirty();
	ST::string str = ST::format("{}", iMapIndex);
	if( gfUseScheduleData2 )
	{
		if( gfSingleAction )
			return;
		iDrawMode = DRAW_MODE_PLAYER + gpSelected->pBasicPlacement->bTeam;
		gCurrSchedule.usData2[ gubCurrentScheduleActionIndex ] = (UINT16)iMapIndex;
		iEditorButton[MERCS_SCHEDULE_DATA1B + gubCurrentScheduleActionIndex]->SpecifyText(str);
		DetermineScheduleEditability();
		gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_NONE;
		gfRenderTaskbar = TRUE;
		gfUseScheduleData2 = FALSE;
	}
	else
	{
		switch( gCurrSchedule.ubAction[ gubCurrentScheduleActionIndex ] )
		{
			case SCHEDULE_ACTION_LOCKDOOR:
			case SCHEDULE_ACTION_UNLOCKDOOR:
			case SCHEDULE_ACTION_OPENDOOR:
			case SCHEDULE_ACTION_CLOSEDOOR:
				if( gfSingleAction )
				{
					gfSingleAction = FALSE;
					gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_NONE;
					gfRenderTaskbar = TRUE;
					DetermineScheduleEditability();
					break;
				}
				DisableEditorButton( MERCS_SCHEDULE_DATA1A + gubCurrentScheduleActionIndex );
				EnableEditorButton( MERCS_SCHEDULE_DATA1B + gubCurrentScheduleActionIndex );
				gfUseScheduleData2 = TRUE;
				iDrawMode = DRAW_MODE_SCHEDULEACTION;
				gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_DOOR2;
				gfRenderTaskbar = TRUE;
				break;
			case SCHEDULE_ACTION_GRIDNO:
			case SCHEDULE_ACTION_ENTERSECTOR:
			case SCHEDULE_ACTION_SLEEP:
				iDrawMode = DRAW_MODE_PLAYER + gpSelected->pBasicPlacement->bTeam;
				DetermineScheduleEditability();
				gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_NONE;
				gfRenderTaskbar = TRUE;
				break;
			case SCHEDULE_ACTION_LEAVESECTOR:
			case SCHEDULE_ACTION_STAYINSECTOR:
			case SCHEDULE_ACTION_NONE:
				break;
		}
		gCurrSchedule.usData1[ gubCurrentScheduleActionIndex ] = (UINT16)iMapIndex;
		iEditorButton[MERCS_SCHEDULE_DATA1A + gubCurrentScheduleActionIndex]->SpecifyText(str);
	}
}

void StartScheduleAction()
{
	switch( gCurrSchedule.ubAction[ gubCurrentScheduleActionIndex ] )
	{
		case SCHEDULE_ACTION_NONE:
			EnableEditorButtons( MERCS_SCHEDULE_ACTION1, MERCS_SCHEDULE_DATA4B );
			EnableTextFields( 1, 4 );
			gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_NONE;
			gfRenderTaskbar = TRUE;
			gCurrSchedule.usData1[ gubCurrentScheduleActionIndex ] = 0xffff;
			gCurrSchedule.usData2[ gubCurrentScheduleActionIndex ] = 0xffff;
			break;
		case SCHEDULE_ACTION_LOCKDOOR:
		case SCHEDULE_ACTION_UNLOCKDOOR:
		case SCHEDULE_ACTION_OPENDOOR:
		case SCHEDULE_ACTION_CLOSEDOOR:
			//First disable everything -- its easier that way.
			ShowEditorButton( MERCS_SCHEDULE_DATA1A + gubCurrentScheduleActionIndex );
			ShowEditorButton( MERCS_SCHEDULE_DATA1B + gubCurrentScheduleActionIndex );
			DisableEditorButtons( MERCS_SCHEDULE_ACTION1, MERCS_SCHEDULE_DATA4B );
			DisableTextFields( 1, 4 );
			EnableEditorButton( MERCS_SCHEDULE_DATA1A + gubCurrentScheduleActionIndex );
			gfUseScheduleData2 = FALSE;
			iDrawMode = DRAW_MODE_SCHEDULEACTION;
			gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_DOOR1;
			gfRenderTaskbar = TRUE;
			break;
		case SCHEDULE_ACTION_GRIDNO:
		case SCHEDULE_ACTION_ENTERSECTOR:
		case SCHEDULE_ACTION_SLEEP:
				ShowEditorButton( MERCS_SCHEDULE_DATA1A + gubCurrentScheduleActionIndex );
			HideEditorButton( MERCS_SCHEDULE_DATA1B + gubCurrentScheduleActionIndex );
			DisableEditorButtons( MERCS_SCHEDULE_ACTION1, MERCS_SCHEDULE_DATA4B );
			DisableTextFields( 1, 4 );
			EnableEditorButton( MERCS_SCHEDULE_DATA1A + gubCurrentScheduleActionIndex );
			gfUseScheduleData2 = FALSE;
			iDrawMode = DRAW_MODE_SCHEDULEACTION;
			gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_GRIDNO;
			gfRenderTaskbar = TRUE;
			gCurrSchedule.usData2[ gubCurrentScheduleActionIndex ] = 0xffff;
			break;
		case SCHEDULE_ACTION_LEAVESECTOR:
		case SCHEDULE_ACTION_STAYINSECTOR:
			gubScheduleInstructions = SCHEDULE_INSTRUCTIONS_NONE;
			gfRenderTaskbar = TRUE;
			gCurrSchedule.usData1[ gubCurrentScheduleActionIndex ] = 0xffff;
			gCurrSchedule.usData2[ gubCurrentScheduleActionIndex ] = 0xffff;
			break;
	}
	MarkWorldDirty();
}

void UpdateScheduleAction( UINT8 ubNewAction )
{
	gCurrSchedule.ubAction[ gubCurrentScheduleActionIndex ] = ubNewAction;
	iEditorButton[MERCS_SCHEDULE_ACTION1 + gubCurrentScheduleActionIndex]->SpecifyText(gszScheduleActions[ubNewAction]);
	iEditorButton[MERCS_SCHEDULE_ACTION1 + gubCurrentScheduleActionIndex]->SetUserData(ubNewAction);
	//Now, based on this action, disable the other buttons
	StartScheduleAction();
	gfSingleAction = FALSE;
}

// 0:1A, 1:1B, 2:2A, 3:2B, ...
void FindScheduleGridNo( UINT8 ubScheduleData )
{
	INT32 iMapIndex; // XXX HACK000E
	switch( ubScheduleData )
	{
		case 0: //1a
			iMapIndex = gCurrSchedule.usData1[0];
			break;
		case 1:	//1b
			iMapIndex = gCurrSchedule.usData2[0];
			break;
		case 2:	//2a
			iMapIndex = gCurrSchedule.usData1[1];
			break;
		case 3: //2b
			iMapIndex = gCurrSchedule.usData2[1];
			break;
		case 4:	//3a
			iMapIndex = gCurrSchedule.usData1[2];
			break;
		case 5:	//3b
			iMapIndex = gCurrSchedule.usData2[2];
			break;
		case 6:	//4a
			iMapIndex = gCurrSchedule.usData1[3];
			break;
		case 7:	//4b
			iMapIndex = gCurrSchedule.usData2[3];
			break;
		default:
			SLOGA("FindScheduleGridNo passed incorrect dataID." );
			abort(); // HACK000E
	}
	if( iMapIndex != 0xffff )
	{
		CenterScreenAtMapIndex( iMapIndex );
	}
}

void ClearCurrentSchedule()
{
	UINT8 i;
	gCurrSchedule = SCHEDULENODE{};
	for( i = 0; i < 4; i++ )
	{
		iEditorButton[MERCS_SCHEDULE_ACTION1 + i]->SetUserData(0);
		iEditorButton[MERCS_SCHEDULE_ACTION1 + i]->SpecifyText("No action");
		gCurrSchedule.usTime[i] = 0xffff;
		SetExclusive24HourTimeValue( (UINT8)(i+1), gCurrSchedule.usTime[ i ] ); //blanks the field
		gCurrSchedule.usData1[i] = 0xffff;
		iEditorButton[MERCS_SCHEDULE_DATA1A + i]->SpecifyText(ST::null);
		gCurrSchedule.usData2[i] = 0xffff;
		iEditorButton[MERCS_SCHEDULE_DATA1B + i]->SpecifyText(ST::null);
	}
	//Remove the variance stuff
	gCurrSchedule.usFlags = 0;
	UnclickEditorButtons( MERCS_SCHEDULE_VARIANCE1, MERCS_SCHEDULE_VARIANCE4 );

	gubCurrentScheduleActionIndex = 0;
	DetermineScheduleEditability();
	gfRenderTaskbar = TRUE;
	MarkWorldDirty();
}


static void RenderCurrentSchedule(void)
{
	FLOAT dOffsetX, dOffsetY;
	FLOAT ScrnX, ScrnY;
	INT32 i;
	INT32 iMapIndex;
	INT16 sXMapPos, sYMapPos;
	INT16 sScreenX, sScreenY;
	INT16 sX, sY;
	for( i = 0; i < 8; i++ )
	{
		if( i % 2 )
			iMapIndex = gCurrSchedule.usData2[ i / 2 ];
		else
			iMapIndex = gCurrSchedule.usData1[ i / 2 ];

		if( iMapIndex == 0xffff )
			continue;

		// Convert it's location to screen coordinates
		ConvertGridNoToXY( (INT16)iMapIndex, &sXMapPos, &sYMapPos );

		dOffsetX = (FLOAT)(sXMapPos * CELL_X_SIZE) - gsRenderCenterX;
		dOffsetY = (FLOAT)(sYMapPos * CELL_Y_SIZE) - gsRenderCenterY;

		FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &ScrnX, &ScrnY);

		sScreenX = ( g_ui.m_tacticalMapCenterX ) + (INT16)ScrnX;
		sScreenY = ( g_ui.m_tacticalMapCenterY ) + (INT16)ScrnY;

		// Adjust for tiles height factor!
		sScreenY -= gpWorldLevelData[ iMapIndex ].sHeight;
		// Bring it down a touch
		sScreenY += 5;

		if( sScreenY <= 355 )
		{
			// Shown it on screen!
			SetFontAttributes(TINYFONT1, FONT_WHITE, DEFAULT_SHADOW, FONT_LTKHAKI);
			ST::string str = ST::format("{}{c}", i / 2 + 1, 'A' + i % 2);
			FindFontCenterCoordinates(sScreenX, sScreenY, 1, 1, str, TINYFONT1, &sX, &sY);
			MPrint(sX, sY, str);
		}
	}
}


static void UpdateScheduleInfo(void)
{
	INT32 i;
	SCHEDULENODE *pSchedule;
	if( gpSelected->pSoldier->ubScheduleID )
	{
		pSchedule = GetSchedule( gpSelected->pSoldier->ubScheduleID );
		if( !pSchedule )
		{
			return;
		}
		for( i = 0; i < 4; i++ )
		{ //Update the text and buttons
			iEditorButton[MERCS_SCHEDULE_ACTION1 + i]->SetUserData(pSchedule->ubAction[i]);
			iEditorButton[MERCS_SCHEDULE_ACTION1 + i]->SpecifyText(gszScheduleActions[pSchedule->ubAction[i]]);
			ST::string str;
			if( pSchedule->usData1[i] != 0xffff )
				str = ST::format("{}", pSchedule->usData1[i]);
			iEditorButton[MERCS_SCHEDULE_DATA1A + i]->SpecifyText(str);
			str = ST::null;
			if( pSchedule->usData2[i] != 0xffff )
				str = ST::format("{}", pSchedule->usData2[i]);
			iEditorButton[MERCS_SCHEDULE_DATA1B + i]->SpecifyText(str);
			if( gubCurrMercMode == MERC_SCHEDULEMODE )
			{ //Update the text input fields too!
				SetExclusive24HourTimeValue( (UINT8)(i+1), pSchedule->usTime[i] );
			}
		}

		//Check or uncheck the checkbox buttons based on the schedule's status.
		UnclickEditorButtons( MERCS_SCHEDULE_VARIANCE1, MERCS_SCHEDULE_VARIANCE4 );
		if( pSchedule->usFlags & SCHEDULE_FLAGS_VARIANCE1 )
			ClickEditorButton( MERCS_SCHEDULE_VARIANCE1 );
		if( pSchedule->usFlags & SCHEDULE_FLAGS_VARIANCE2 )
			ClickEditorButton( MERCS_SCHEDULE_VARIANCE2 );
		if( pSchedule->usFlags & SCHEDULE_FLAGS_VARIANCE3 )
			ClickEditorButton( MERCS_SCHEDULE_VARIANCE3 );
		if( pSchedule->usFlags & SCHEDULE_FLAGS_VARIANCE4 )
			ClickEditorButton( MERCS_SCHEDULE_VARIANCE4 );

		//Copy the schedule over to the current global schedule used for editing purposes.
		gCurrSchedule = *pSchedule;
		DetermineScheduleEditability();
	}
	else
	{
		ClearCurrentSchedule();
	}
}


static BOOLEAN                    gfSaveBuffer = FALSE;
static BASIC_SOLDIERCREATE_STRUCT gSaveBufferBasicPlacement;
static SOLDIERCREATE_STRUCT       gSaveBufferDetailedPlacement;


void CopyMercPlacement( INT32 iMapIndex )
{
	if (g_selected_merc == NULL)
	{
		ScreenMsg( FONT_MCOLOR_LTRED, MSG_INTERFACE, "Placement not copied because no placement selected." );
		return;
	}
	gfSaveBuffer = TRUE;
	gSaveBufferBasicPlacement = *gpSelected->pBasicPlacement;
	if( gSaveBufferBasicPlacement.fDetailedPlacement )
	{
		gSaveBufferDetailedPlacement = *gpSelected->pDetailedPlacement;
	}
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "Placement copied." );
}

void PasteMercPlacement( INT32 iMapIndex )
{
	SOLDIERCREATE_STRUCT tempDetailedPlacement;
	INT32 i;

	if( !gfSaveBuffer )
	{
		ScreenMsg( FONT_MCOLOR_LTRED, MSG_INTERFACE, "Placement not pasted as no placement is saved in buffer." );
		return;
	}

	gTempBasicPlacement = gSaveBufferBasicPlacement;

	//calculate specific information based on the team.
	SoldierBodyType body = BODY_RANDOM;
	switch( iDrawMode )
	{
		case DRAW_MODE_ENEMY:
			gTempBasicPlacement.bTeam = ENEMY_TEAM;
			gTempBasicPlacement.ubSoldierClass = gubSoldierClass;
			break;
		case DRAW_MODE_CREATURE:
			gTempBasicPlacement.bTeam = CREATURE_TEAM;
			body = gbCurrCreature;
			break;
		case DRAW_MODE_REBEL:
			gTempBasicPlacement.bTeam = MILITIA_TEAM;
			break;
		case DRAW_MODE_CIVILIAN:
			gTempBasicPlacement.bTeam = CIV_TEAM;
			gTempBasicPlacement.ubCivilianGroup = gubCivGroup;
			if (giCurrentTilesetID == CAVES_1)
			{
				gTempBasicPlacement.ubSoldierClass = SOLDIER_CLASS_MINER;
			}
			break;
	}
	gTempBasicPlacement.bBodyType = body;

	if( IsLocationSittable( iMapIndex, gfRoofPlacement ) )
	{
		SOLDIERINITNODE *pNode;

		//Set up some general information.
		//gTempBasicPlacement.fDetailedPlacement = TRUE;
		gTempBasicPlacement.usStartingGridNo = (UINT16)iMapIndex;

		//Generate detailed placement information given the temp placement information.
		if( gTempBasicPlacement.fDetailedPlacement )
		{
			gTempDetailedPlacement = gSaveBufferDetailedPlacement;
		}
		else
		{
			CreateDetailedPlacementGivenBasicPlacementInfo( &gTempDetailedPlacement, &gTempBasicPlacement );
		}

		//Set the sector information -- probably unnecessary.
		gTempDetailedPlacement.sSector = gWorldSector;

		if( gTempBasicPlacement.fDetailedPlacement )
		{
			CreateDetailedPlacementGivenStaticDetailedPlacementAndBasicPlacementInfo( &tempDetailedPlacement, &gTempDetailedPlacement, &gTempBasicPlacement );
		}
		else
		{
			tempDetailedPlacement = gTempDetailedPlacement;
		}

		//Create the soldier, but don't place it yet.
		SOLDIERTYPE* const pSoldier = TacticalCreateSoldier(tempDetailedPlacement);
		if (pSoldier != NULL)
		{
			pSoldier->bVisible = 1;
			pSoldier->bLastRenderVisibleValue = 1;
			//Set up the soldier in the list, so we can track the soldier in the
			//future (saving, loading, strategic AI)
			pNode = AddBasicPlacementToSoldierInitList(gTempBasicPlacement);
			pNode->pSoldier = pSoldier;
			if( gSaveBufferBasicPlacement.fDetailedPlacement )
			{ //Add the static detailed placement information in the same newly created node as the basic placement.
				//read static detailed placement from file
				//allocate memory for new static detailed placement
				gTempBasicPlacement.fDetailedPlacement = TRUE;
				gTempBasicPlacement.fPriorityExistance = gSaveBufferBasicPlacement.fPriorityExistance;
				pNode->pDetailedPlacement = new SOLDIERCREATE_STRUCT{};
				//copy the file information from temp var to node in list.
				*pNode->pDetailedPlacement = gSaveBufferDetailedPlacement;
			}

			//Add the soldier to physically appear on the map now.
			AddSoldierToSectorNoCalculateDirection(pSoldier);
			IndicateSelectedMerc(pSoldier->ubID);

			//Move him to the roof if intended and possible.
			if( gfRoofPlacement && FlatRoofAboveGridNo( iMapIndex ) )
			{
				gpSelected->pBasicPlacement->fOnRoof = TRUE;
				if( gpSelected->pDetailedPlacement )
					gpSelected->pDetailedPlacement->fOnRoof = TRUE;
				SetSoldierHeight( gpSelected->pSoldier, SECOND_LEVEL_Z_OFFSET );
			}
			UnclickEditorButtons( FIRST_MERCS_INVENTORY_BUTTON, LAST_MERCS_INVENTORY_BUTTON );
			for( i = FIRST_MERCS_INVENTORY_BUTTON; i <= LAST_MERCS_INVENTORY_BUTTON; i++ )
			{
				SetEnemyDroppableStatus( gbMercSlotTypes[i-FIRST_MERCS_INVENTORY_BUTTON], FALSE );
			}
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "Placement pasted." );
		}
		else
		{
			ScreenMsg( FONT_MCOLOR_LTRED, MSG_INTERFACE, "Placement not pasted as the maximum number of placements for this team is already used." );
		}
	}
}
