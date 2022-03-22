#include "Map_Screen_Interface.h"
#include "ContentManager.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "EMail.h"
#include "FileMan.h"
#include "Finances.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "Game_Init.h"
#include "GameInstance.h"
#include "GamePolicy.h"
#include "GameSettings.h"
#include "HImage.h"
#include "Interface_Items.h"
#include "Isometric_Utils.h"
#include "Items.h"
#include "JAScreens.h"
#include "Keys.h"
#include "Line.h"
#include "LoadSaveObjectType.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "MapScreen.h"
#include "MercPortrait.h"
#include "Message.h"
#include "Overhead.h"
#include "PopUpBox.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Render_Dirty.h"
#include "Render_Fun.h"
#include "RenderWorld.h"
#include "SamSiteModel.h"
#include "ShippingDestinationModel.h"
#include "Soldier_Macros.h"
#include "SoundMan.h"
#include "Squads.h"
#include "Strategic.h"
#include "StrategicMap_Secrets.h"
#include "Strategic_Mines.h"
#include "Strategic_Pathing.h"
#include "SysUtil.h"
#include "Tactical_Placement_GUI.h"
#include "Tactical_Save.h"
#include "Text.h"
#include "Timer_Control.h"
#include "Video.h"
#include "VObject.h"
#include "VSurface.h"
#include "WordWrap.h"
#include <algorithm>
#include <iterator>
#include <string_theory/format>
#include <string_theory/string>
struct FACETYPE;
struct PopUpBox;

// number of LINKED LISTS for sets of leave items (each slot holds an unlimited # of items)
#define NUM_LEAVE_LIST_SLOTS 20

#define SELECTED_CHAR_ARROW_X (STD_SCREEN_X + 8)

#define SIZE_OF_UPDATE_BOX 20

// as deep as the map goes
#define MAX_DEPTH_OF_MAP 3


// number of merc columns for four wide mode
#define NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE 4

// number of merc columns for 2 wide mode
#define NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE 2

// number needed for 4 wide mode to activate
#define NUMBER_OF_MERCS_FOR_FOUR_WIDTH_UPDATE_PANEL 4

#define DBL_CLICK_DELAY_FOR_MOVE_MENU 200


#define REASON_FOR_SOLDIER_UPDATE_OFFSET_Y				(14)

#define MAX_MAPSCREEN_FAST_HELP			100

#define VEHICLE_ONLY				FALSE
#define AND_ALL_ON_BOARD		TRUE


// the regions int he movemenu
enum{
	SQUAD_REGION = 0,
	VEHICLE_REGION,
	SOLDIER_REGION,
	DONE_REGION,
	CANCEL_REGION,
	OTHER_REGION,
};


// waiting list for update box
INT32 iUpdateBoxWaitingList[ MAX_CHARACTER_COUNT ];


struct FASTHELPREGION
{
	ST::string FastHelpText;
	INT32 iX;
	INT32 iY;
	INT32 iW;
};


static FASTHELPREGION pFastHelpMapScreenList[MAX_MAPSCREEN_FAST_HELP];

// the move menu region
static MOUSE_REGION gMoveMenuRegion[MAX_POPUP_BOX_STRING_COUNT];

static MOUSE_REGION gMapScreenHelpTextMask;

static BOOLEAN fScreenMaskForMoveCreated = FALSE;

static ST::string gsCustomErrorString;

BOOLEAN fShowUpdateBox = FALSE;
static BOOLEAN fInterfaceFastHelpTextActive = FALSE;
BOOLEAN fReBuildCharacterList = FALSE;
static INT32 giSizeOfInterfaceFastHelpTextList = 0;

//Animated sector locator icon variables.
SGPSector gsSectorLocator;
UINT8 gubBlitSectorLocatorCode; //color
static SGPVObject* guiSectorLocatorGraphicID;
// the animate time per frame in milliseconds
#define ANIMATED_BATTLEICON_FRAME_TIME 80
#define MAX_FRAME_COUNT_FOR_ANIMATED_BATTLE_ICON 12


// number of mercs in sector capable of moving
static INT32 giNumberOfSoldiersInSectorMoving = 0;

// number of squads capable of moving
static INT32 giNumberOfSquadsInSectorMoving = 0;

// number of vehicles in sector moving
static INT32 giNumberOfVehiclesInSectorMoving = 0;

// the list of soldiers that are moving
static SOLDIERTYPE* pSoldierMovingList[MAX_CHARACTER_COUNT];
static bool         fSoldierIsMoving[MAX_CHARACTER_COUNT];

static SOLDIERTYPE* pUpdateSoldierBox[SIZE_OF_UPDATE_BOX];

static SGPVObject* giUpdateSoldierFaces[SIZE_OF_UPDATE_BOX];

// the squads thata re moving
static INT32 iSquadMovingList[NUMBER_OF_SQUADS];
static INT32 fSquadIsMoving[NUMBER_OF_SQUADS];

// the vehicles thata re moving
static INT32 iVehicleMovingList[NUMBER_OF_SQUADS];
static INT32 fVehicleIsMoving[NUMBER_OF_SQUADS];

static MOUSE_REGION gMoveBoxScreenMask;


BOOLEAN fShowMapScreenMovementList = FALSE;


MapScreenCharacterSt gCharactersList[ MAX_CHARACTER_COUNT+1];

MOUSE_REGION	gMapStatusBarsRegion;

static UpdateBoxReason iReasonForSoldierUpDate = NO_REASON_FOR_UPDATE;


// disable team info panels due to battle roster
BOOLEAN   fDisableDueToBattleRoster = FALSE;

// track old contract times
static INT32 iOldContractTimes[MAX_CHARACTER_COUNT];

// position of pop up box
INT32 giBoxY = 0;

static MOUSE_REGION gContractIconRegion;
static MOUSE_REGION gInsuranceIconRegion;
static MOUSE_REGION gDepositIconRegion;

// general line..current and old
INT32 giHighLine=-1;

// assignment's line...glow box
INT32 giAssignHighLine=-1;

// destination plot line....glow box
INT32 giDestHighLine=-1;

// contract selection glow box
INT32 giContractHighLine = -1;

// the sleep column glow box
INT32 giSleepHighLine = -1;

// pop up box textures
SGPVSurface* guiPOPUPTEX;
SGPVObject* guiPOPUPBORDERS;

// the currently selected character arrow
static SGPVObject* guiSelectedCharArrow;

static GUIButtonRef guiUpdatePanelButtons[2];

// the update panel
SGPVObject* guiUpdatePanelTactical;

struct MERC_LEAVE_ITEM
{
	OBJECTTYPE o;
	MERC_LEAVE_ITEM* pNext;
};

// the leave item list
static MERC_LEAVE_ITEM* gpLeaveListHead[NUM_LEAVE_LIST_SLOTS];

// holds ids of mercs who left stuff behind
static UINT32 guiLeaveListOwnerProfileId[NUM_LEAVE_LIST_SLOTS];

// timers for double click
static UINT32 guiDblClickTimersForMoveBoxMouseRegions[MAX_POPUP_BOX_STRING_COUNT];

UINT32 guiSectorLocatorBaseTime = 0;


// which menus are we showing
BOOLEAN fShowAssignmentMenu = FALSE;
BOOLEAN fShowTrainingMenu = FALSE;
BOOLEAN fShowAttributeMenu = FALSE;
BOOLEAN fShowSquadMenu = FALSE;
BOOLEAN fShowContractMenu = FALSE;

static BOOLEAN fRebuildMoveBox = FALSE;

// at least one merc was hired at some time
BOOLEAN gfAtLeastOneMercWasHired = FALSE;


void InitalizeVehicleAndCharacterList( void )
{
	// will init the vehicle and character lists to zero
	std::fill(std::begin(gCharactersList), std::end(gCharactersList), MapScreenCharacterSt{});
}


void SetEntryInSelectedCharacterList( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );
	gCharactersList[bEntry].selected = TRUE;
}


void ResetEntryForSelectedList( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );
	gCharactersList[bEntry].selected = FALSE;
}


void ResetSelectedListForMapScreen( void )
{
	// set all the entries int he selected list to false
	for (size_t i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		gCharactersList[i].selected = FALSE;
	}

	// if we still have a valid dude selected
	if (GetSelectedInfoChar() != NULL)
	{
		// then keep him selected
		SetEntryInSelectedCharacterList( bSelectedInfoChar );
	}
}


BOOLEAN IsEntryInSelectedListSet( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );
	return gCharactersList[bEntry].selected;
}


void ToggleEntryInSelectedList( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );
	MapScreenCharacterSt* const c = &gCharactersList[bEntry];
	c->selected = !c->selected;
}


void BuildSelectedListFromAToB( INT8 bA, INT8 bB )
{
	INT8 bStart =0, bEnd = 0;

	// run from a to b..set slots as selected

	if( bA > bB )
	{
		bStart = bB;
		bEnd = bA;
	}
	else
	{
		bStart = bA;
		bEnd = bB;
	}

	// run through list and set all intermediaries to true

	for( ; bStart <= bEnd; bStart++ )
	{
		SetEntryInSelectedCharacterList( bStart );
	}
}


BOOLEAN MultipleCharacterListEntriesSelected( void )
{
	UINT8 ubSelectedCnt = 0;

	// check if more than one person is selected in the selected list
	CFOR_EACH_SELECTED_IN_CHAR_LIST(c)
	{
		++ubSelectedCnt;
	}

	if( ubSelectedCnt > 1 )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


void ResetAssignmentsForMercsTrainingUnpaidSectorsInSelectedList()
{
	CFOR_EACH_IN_CHAR_LIST(c)
	{
		SOLDIERTYPE* const pSoldier = c->merc;
		if( pSoldier->bAssignment == TRAIN_TOWN )
		{
			if (!SectorInfo[SECTOR(pSoldier->sSectorX, pSoldier->sSectorY)].fMilitiaTrainingPaid)
			{
				ResumeOldAssignment( pSoldier );
			}
		}
	}
}


void ResetAssignmentOfMercsThatWereTrainingMilitiaInThisSector( INT16 sSectorX, INT16 sSectorY )
{
	CFOR_EACH_IN_CHAR_LIST(c)
	{
		SOLDIERTYPE* const pSoldier = c->merc;
		if( pSoldier->bAssignment == TRAIN_TOWN )
		{
			if( ( pSoldier->sSectorX == sSectorX ) && ( pSoldier->sSectorY == sSectorY ) && ( pSoldier->bSectorZ == 0 ) )
			{
				ResumeOldAssignment( pSoldier );
			}
		}
	}
}


static BOOLEAN CanSoldierMoveWithVehicleId(const SOLDIERTYPE* s, INT32 iVehicle1Id);


// check if the members of the selected list move with this guy... are they in the same mvt group?
void DeselectSelectedListMercsWhoCantMoveWithThisGuy(const SOLDIERTYPE* const pSoldier)
{
	INT32 iCounter = 0;

	// deselect any other selected mercs that can't travel together with pSoldier
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const MapScreenCharacterSt* const c = &gCharactersList[iCounter];
		if (!c->selected) continue;

		const SOLDIERTYPE* const pSoldier2 = c->merc;
		if (pSoldier2 == NULL) continue;

		// skip the guy we are
		if (pSoldier == pSoldier2) continue;

		// NOTE ABOUT THE VEHICLE TESTS BELOW:
		// Vehicles and foot squads can't plot movement together!
		// The ETAs are different, and unlike squads, vehicles can't travel everywhere!
		// However, different vehicles CAN plot together, since they all travel at the same rates now

		// if anchor guy is IN a vehicle
		if( pSoldier->bAssignment == VEHICLE )
		{
			if ( !CanSoldierMoveWithVehicleId( pSoldier2, pSoldier->iVehicleId ) )
			{
				// reset entry for selected list
				ResetEntryForSelectedList( ( INT8 )iCounter );
			}
		}
		// if anchor guy IS a vehicle
		else if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
		{
			if ( !CanSoldierMoveWithVehicleId( pSoldier2, pSoldier->bVehicleID ) )
			{
				// reset entry for selected list
				ResetEntryForSelectedList( ( INT8 )iCounter );
			}
		}
		// if this guy is IN a vehicle
		else if( pSoldier2->bAssignment == VEHICLE )
		{
			if ( !CanSoldierMoveWithVehicleId( pSoldier, pSoldier2->iVehicleId ) )
			{
				// reset entry for selected list
				ResetEntryForSelectedList( ( INT8 )iCounter );
			}
		}
		// if this guy IS a vehicle
		else if ( pSoldier2->uiStatusFlags & SOLDIER_VEHICLE )
		{
			if ( !CanSoldierMoveWithVehicleId( pSoldier, pSoldier2->bVehicleID ) )
			{
				// reset entry for selected list
				ResetEntryForSelectedList( ( INT8 )iCounter );
			}
		}
		// reject those not a squad (vehicle handled above)
		else if( pSoldier2->bAssignment >= ON_DUTY )
		{
			ResetEntryForSelectedList( ( INT8 )iCounter );
		}
		else
		{
			// reject those not in the same sector
			if( ( pSoldier->sSectorX != pSoldier2->sSectorX ) ||
					( pSoldier->sSectorY != pSoldier2->sSectorY ) ||
					( pSoldier->bSectorZ != pSoldier2->bSectorZ ) )
			{
				ResetEntryForSelectedList( ( INT8 )iCounter );
			}

			// if either is between sectors, they must be in the same movement group
			if ( ( pSoldier->fBetweenSectors || pSoldier2->fBetweenSectors ) &&
				( pSoldier->ubGroupID != pSoldier2->ubGroupID ) )
			{
				ResetEntryForSelectedList( ( INT8 )iCounter );
			}
		}

		// different movement groups in same sector is OK, even if they're not travelling together
	}
}


static BOOLEAN AnyMercInSameSquadOrVehicleIsSelected(const SOLDIERTYPE* s);


void SelectUnselectedMercsWhoMustMoveWithThisGuy( void )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const MapScreenCharacterSt* const c = &gCharactersList[iCounter];
		if (c->selected) continue;

		const SOLDIERTYPE* const pSoldier = c->merc;
		if (pSoldier == NULL) continue;

		// if on a squad or in a vehicle
		if ( ( pSoldier->bAssignment < ON_DUTY ) || ( pSoldier->bAssignment == VEHICLE ) )
		{
			// and a member of that squad or vehicle is selected
			if ( AnyMercInSameSquadOrVehicleIsSelected( pSoldier ) )
			{
				// then also select this guy
				SetEntryInSelectedCharacterList( ( INT8 ) iCounter );
			}
		}
	}
}


static BOOLEAN AnyMercInSameSquadOrVehicleIsSelected(const SOLDIERTYPE* const pSoldier)
{
	CFOR_EACH_SELECTED_IN_CHAR_LIST(c)
	{
		const SOLDIERTYPE* const pSoldier2 = c->merc;

		// if they have the same assignment
		if( pSoldier->bAssignment == pSoldier2->bAssignment )
		{
			// same squad?
			if ( pSoldier->bAssignment < ON_DUTY )
			{
				return ( TRUE );
			}

			// same vehicle?
			if ( ( pSoldier->bAssignment == VEHICLE ) && ( pSoldier->iVehicleId == pSoldier2->iVehicleId ) )
			{
				return ( TRUE );
			}
		}

		// target guy is in a vehicle, and this guy IS that vehicle
		if( ( pSoldier->bAssignment == VEHICLE ) && ( pSoldier2->uiStatusFlags & SOLDIER_VEHICLE ) &&
				( pSoldier->iVehicleId == pSoldier2->bVehicleID ) )
		{
			return ( TRUE );
		}

		// this guy is in a vehicle, and the target guy IS that vehicle
		if( ( pSoldier2->bAssignment == VEHICLE ) && ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) &&
				( pSoldier2->iVehicleId == pSoldier->bVehicleID ) )
		{
			return ( TRUE );
		}
	}

	return ( FALSE );
}



void RestoreBackgroundForAssignmentGlowRegionList( void )
{
	static INT32 iOldAssignmentLine = -1;

	// will restore the background region of the assignment list after a glow has ceased
	// ( a _LOST_MOUSE reason to the assignment region mvt callback handler )

	if (fShowAssignmentMenu)
	{
		// force update
		ForceUpDateOfBox( ghAssignmentBox );
		ForceUpDateOfBox( ghEpcBox );
		ForceUpDateOfBox( ghRemoveMercAssignBox );
		if (fShowSquadMenu)
		{
			ForceUpDateOfBox( ghSquadBox );
		}
		else if (fShowTrainingMenu)
		{
			ForceUpDateOfBox( ghTrainingBox );
		}

	}

	if( fDisableDueToBattleRoster )
	{
		return;
	}

	if( iOldAssignmentLine != giAssignHighLine )
	{
		// restore background
		RestoreExternBackgroundRect( ASSIGN_X, Y_START - 1, ASSIGN_WIDTH, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) );

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldAssignmentLine = giAssignHighLine;
	}
}

void RestoreBackgroundForDestinationGlowRegionList( void )
{
	static INT32 iOldDestinationLine = -1;

	// will restore the background region of the destinationz list after a glow has ceased
	// ( a _LOST_MOUSE reason to the assignment region mvt callback handler )

	if( fDisableDueToBattleRoster )
	{
		return;
	}

	if( iOldDestinationLine != giDestHighLine )
	{
		// restore background
		RestoreExternBackgroundRect( DEST_ETA_X, Y_START - 1, DEST_ETA_WIDTH, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) );

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldDestinationLine = giDestHighLine;
	}
}

void RestoreBackgroundForContractGlowRegionList( void )
{
	static INT32 iOldContractLine = -1;

	// will restore the background region of the destinationz list after a glow has ceased
	// ( a _LOST_MOUSE reason to the assignment region mvt callback handler )

	if( fDisableDueToBattleRoster )
	{
		return;
	}

	if( iOldContractLine != giContractHighLine )
	{
		// restore background
		RestoreExternBackgroundRect( TIME_REMAINING_X, Y_START - 1, TIME_REMAINING_WIDTH, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) ) ;

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldContractLine = giContractHighLine;
	}
}


void RestoreBackgroundForSleepGlowRegionList( void )
{
	static INT32 iOldSleepHighLine = -1;

	// will restore the background region of the sleep list after a glow has ceased
	// ( a _LOST_MOUSE reason to the assignment region mvt callback handler )

	if( fDisableDueToBattleRoster )
	{
		return;
	}

	if( iOldSleepHighLine != giSleepHighLine )
	{
		// restore background
		RestoreExternBackgroundRect( SLEEP_X, Y_START - 1, SLEEP_WIDTH, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) ) ;

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldSleepHighLine = giSleepHighLine;
	}
}

void PlayGlowRegionSound( void )
{
	// play a new message sound, if there is one playing, do nothing
	static UINT32 uiSoundId = 0;

	if( uiSoundId != 0 )
	{
		// is sound playing?..don't play new one
		if (SoundIsPlaying(uiSoundId))
		{
			return;
		}
	}

	// otherwise no sound playing, play one
	uiSoundId = PlayJA2SampleFromFile(SOUNDSDIR "/glowclick.wav", MIDVOLUME, 1, MIDDLEPAN);
}



BOOLEAN CharacterIsGettingPathPlotted(INT16 const sCharNumber)
{
	// valid character number?
	if( ( sCharNumber < 0 ) || ( sCharNumber >= MAX_CHARACTER_COUNT ) )
	{
		return( FALSE );
	}

	// is the character a valid one?
	if (gCharactersList[sCharNumber].merc == NULL) return FALSE;

	// if the highlighted line character is also selected
	if ( ( ( giDestHighLine != -1 ) && IsEntryInSelectedListSet ( ( INT8 ) giDestHighLine ) ) ||
		( ( bSelectedDestChar != -1 ) && IsEntryInSelectedListSet ( bSelectedDestChar ) ) )
	{
		// then ALL selected lines will be affected
		if( IsEntryInSelectedListSet( ( INT8 ) sCharNumber ) )
		{
			return( TRUE );
		}
	}
	else
	{
		// if he is *the* selected dude
		if( bSelectedDestChar == sCharNumber )
		{
			return ( TRUE );
		}

		// ONLY the highlighted line will be affected
		if ( sCharNumber == giDestHighLine )
		{
			return( TRUE );
		}
	}

	return ( FALSE );
}

BOOLEAN IsCharacterSelectedForAssignment( INT16 sCharNumber )
{
	// valid character number?
	if( ( sCharNumber < 0 ) || ( sCharNumber >= MAX_CHARACTER_COUNT ) )
	{
		return( FALSE );
	}

	// is the character a valid one?
	if (gCharactersList[sCharNumber].merc == NULL) return FALSE;

	// if the highlighted line character is also selected
	if ( ( giAssignHighLine != -1 ) && IsEntryInSelectedListSet ( ( INT8 ) giAssignHighLine ) )
	{
		// then ALL selected lines will be affected
		if( IsEntryInSelectedListSet( ( INT8 ) sCharNumber ) )
		{
			return( TRUE );
		}
	}
	else
	{
		// ONLY the highlighted line will be affected
		if ( sCharNumber == giAssignHighLine )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}



BOOLEAN IsCharacterSelectedForSleep( INT16 sCharNumber )
{
	// valid character number?
	if( ( sCharNumber < 0 ) || ( sCharNumber >= MAX_CHARACTER_COUNT ) )
	{
		return( FALSE );
	}

	// is the character a valid one?
	if (gCharactersList[sCharNumber].merc == NULL) return FALSE;

	// if the highlighted line character is also selected
	if ( ( giSleepHighLine != -1 ) && IsEntryInSelectedListSet ( ( INT8 ) giSleepHighLine ) )
	{
		// then ALL selected lines will be affected
		if( IsEntryInSelectedListSet( ( INT8 ) sCharNumber ) )
		{
			return( TRUE );
		}
	}
	else
	{
		// ONLY the highlighted line will be affected
		if ( sCharNumber == giSleepHighLine )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}

void DisableTeamInfoPanels( void )
{
	// disable team info panel
	fDisableDueToBattleRoster = TRUE;
}


void EnableTeamInfoPanels( void )
{
	// enable team info panel
	fDisableDueToBattleRoster = FALSE;
}


void DoMapMessageBoxWithRect(MessageBoxStyleID ubStyle, const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPBox* centering_rect)
{	// reset the highlighted line
	giHighLine = -1;
	DoMessageBox(ubStyle, str, uiExitScreen, usFlags, ReturnCallback, centering_rect);
}


void DoMapMessageBox(MessageBoxStyleID ubStyle, const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	// reset the highlighted line
	giHighLine = -1;

	// do message box and return
	SGPBox const centering_rect = { 0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y };
	DoMessageBox(ubStyle, str, uiExitScreen, usFlags, ReturnCallback, &centering_rect);
}



void GoDownOneLevelInMap( void )
{
	JumpToLevel( iCurrentMapSectorZ + 1 );
}


void GoUpOneLevelInMap( void )
{
	JumpToLevel( iCurrentMapSectorZ - 1 );
}


void JumpToLevel( INT32 iLevel )
{
	if (gfPreBattleInterfaceActive) return;

	// disable level-changes while in inventory pool (for keyboard equivalents!)
	if( fShowMapInventoryPool )
		return;

	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}

	if( iLevel < 0 )
	{
		iLevel = 0;
	}

	if( iLevel > MAX_DEPTH_OF_MAP )
	{
		iLevel = MAX_DEPTH_OF_MAP;
	}

	// set current sector Z to level passed
	ChangeSelectedMapSector( sSelMapX, sSelMapY, ( INT8 )iLevel );
}


// check against old contract times, update as nessacary
void CheckAndUpdateBasedOnContractTimes( void )
{
	INT32 iCounter = 0;
	INT32 iTimeRemaining = 0;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const s = gCharactersList[iCounter].merc;
		if (s == NULL) continue;

		// what kind of merc
		if (s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
		{
			// amount of time left on contract
			iTimeRemaining = s->iEndofContractTime-GetWorldTotalMin();
			if(iTimeRemaining >60*24)
			{
				// more than a day, display in green
				iTimeRemaining/=(60*24);

				// check if real change in contract time
				if( iTimeRemaining != iOldContractTimes[ iCounter ])
				{
					iOldContractTimes[ iCounter ] = iTimeRemaining;

					// dirty screen
					fTeamPanelDirty = TRUE;
					fCharacterInfoPanelDirty = TRUE;
				}
			}
			else
			{
				// less than a day, display hours left in red
				iTimeRemaining/=60;

				// check if real change in contract time
				if( iTimeRemaining != iOldContractTimes[ iCounter ])
				{
					iOldContractTimes[ iCounter ] = iTimeRemaining;
					// dirty screen
					fTeamPanelDirty = TRUE;
					fCharacterInfoPanelDirty = TRUE;
				}
			}
		}
		else if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
		{
			iTimeRemaining = s->iTotalContractLength;

			if( iTimeRemaining != iOldContractTimes[ iCounter ])
			{
				iOldContractTimes[ iCounter ] = iTimeRemaining;

				// dirty screen
				fTeamPanelDirty = TRUE;
				fCharacterInfoPanelDirty = TRUE;
			}
		}
	}
}


void HandleDisplayOfSelectedMercArrows()
{
	if (!GetSelectedInfoChar()) return;
	if (fShowInventoryFlag)     return;

	{ // Blit one by the selected merc
		INT16 y = Y_START + bSelectedInfoChar * (Y_SIZE + 2) - 1;
		if (bSelectedInfoChar >= FIRST_VEHICLE) y += 6;
		BltVideoObject(guiSAVEBUFFER, guiSelectedCharArrow, 0,SELECTED_CHAR_ARROW_X, y);
	}

	// now run through the selected list of guys, an arrow for each
	UINT8 const dest_group = bSelectedDestChar != -1 ? gCharactersList[bSelectedDestChar].merc->ubGroupID : 0;
	for (UINT8 i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		SOLDIERTYPE const* const s = gCharactersList[i].merc;
		if (!s) continue;

		// Is he in the selected list or in the same mvt group as this guy?
		if (!IsEntryInSelectedListSet(i) && (s->ubGroupID == 0 || s->ubGroupID != dest_group)) continue;

		INT16 y = Y_START + i * (Y_SIZE + 2) - 1;
		if (i >= FIRST_VEHICLE) y += 6;
		BltVideoObject(guiSAVEBUFFER, guiSelectedCharArrow, 0, SELECTED_CHAR_ARROW_X, y);
	}
}


ST::string GetMoraleString(SOLDIERTYPE const& s)
{
	return
		s.uiStatusFlags & SOLDIER_DEAD ? pMoralStrings[5] :
		s.bMorale > 80                 ? pMoralStrings[0] :
		s.bMorale > 65                 ? pMoralStrings[1] :
		s.bMorale > 35                 ? pMoralStrings[2] :
		s.bMorale > 20                 ? pMoralStrings[3] :
		pMoralStrings[4];
}


// NOTE: This doesn't use the "LeaveList" system at all!
void HandleLeavingOfEquipmentInCurrentSector(SOLDIERTYPE& s)
{
	// Just drop the stuff in the current sector
	GridNo gridno;
	bool const here = s.sSectorX == gWorldSector.x && s.sSectorY == gWorldSector.y && s.bSectorZ == gWorldSector.z;
	if (here)
	{
		// ATE: Mercs can have a gridno of NOWHERE
		gridno = s.sGridNo;
		if (gridno == NOWHERE)
		{
			gridno = RandomGridNo();

			GridNo tmp_gridno = FindNearestAvailableGridNoForItem(gridno, 5);
			if (tmp_gridno == NOWHERE) tmp_gridno = FindNearestAvailableGridNoForItem(gridno, 15);
			if (tmp_gridno != NOWHERE) gridno = tmp_gridno;
		}
	}
	else
	{
		// ATE: Use insertion gridno if not nowhere and insertion is gridno
		if (s.ubStrategicInsertionCode == INSERTION_CODE_GRIDNO && s.usStrategicInsertionData != NOWHERE)
		{
			gridno = s.usStrategicInsertionData;
		}
		else
		{
			gridno = RandomGridNo();
		}
	}

	FOR_EACH_SOLDIER_INV_SLOT(i, s)
	{
		if (i->ubNumberOfObjects == 0) continue;

		if (here)
		{
			AddItemToPool(gridno, i, VISIBLE, s.bLevel, WORLD_ITEM_REACHABLE, 0);
		}
		else
		{
			AddItemsToUnLoadedSector(s.sSectorX, s.sSectorY, s.bSectorZ, gridno, 1, i, s.bLevel, WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO | WORLD_ITEM_REACHABLE, 0, VISIBLE);
		}
	}

	DropKeysInKeyRing(s, gridno, s.bLevel, VISIBLE, false, 0, false);
}


static INT32 SetUpDropItemListForMerc(SOLDIERTYPE&);


void HandleMercLeavingEquipment(SOLDIERTYPE& s, bool const in_drassen)
{
	// Stash the items into a linked list hanging of a free "leave item list" slot
	INT32 const slot = SetUpDropItemListForMerc(s);
	if (slot != -1)
	{ // Post event to drop it there 6 hours later
		StrategicEventKind const e = in_drassen ? EVENT_MERC_LEAVE_EQUIP_IN_DRASSEN : EVENT_MERC_LEAVE_EQUIP_IN_OMERTA;
		AddStrategicEvent(e, GetWorldTotalMin() + 6 * 60, slot);
	}
	else
	{ // Otherwise there's no free slots left (shouldn't ever happen)
		AssertMsg(FALSE, "HandleMercLeavingEquipment: No more free slots, equipment lost");
	}
}


static void FreeLeaveListSlot(UINT32 uiSlotIndex);


static void HandleEquipmentLeft(UINT32 const slot_idx, INT const sector, GridNo const grid)
{
	Assert(slot_idx < NUM_LEAVE_LIST_SLOTS);

	if (MERC_LEAVE_ITEM* i = gpLeaveListHead[slot_idx])
	{
		ST::string sString;
		ST::string town = GCM->getTownLocative(GetTownIdForSector(sector));
		int            const x    = SECTORX(sector);
		char           const y    = SECTORY(sector) - 1 + 'A';
		ProfileID      const id   = guiLeaveListOwnerProfileId[slot_idx];
		if (id != NO_PROFILE)
		{
			sString = st_format_printf(str_left_equipment, GetProfile(id).zNickname, town, y, x);
		}
		else
		{
			sString = ST::format("A departing merc has left their equipment in {} ({c}{}).", town, y, x);
		}
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sString);

		bool const is_sector_loaded = gWorldSector == SGPSector(sector);
		for (; i; i = i->pNext)
		{
			if (is_sector_loaded)
			{
				AddItemToPool(grid, &i->o, VISIBLE, 0, WORLD_ITEM_REACHABLE, 0);
			}
			else
			{ // Given this slot value, add to sector item list
				AddItemsToUnLoadedSector(SECTORX(sector), SECTORY(sector), 0, grid, 1, &i->o, 0, WORLD_ITEM_REACHABLE, 0, VISIBLE);
			}
		}
	}

	FreeLeaveListSlot(slot_idx);
}


void HandleEquipmentLeftInOmerta(const UINT32 uiSlotIndex)
{
	HandleEquipmentLeft(uiSlotIndex, gamepolicy(start_sector), START_SECTOR_LEAVE_EQUIP_GRIDNO);
}


void HandleEquipmentLeftInDrassen(const UINT32 uiSlotIndex)
{
	auto primaryAirport = GCM->getPrimaryShippingDestination();
	HandleEquipmentLeft(uiSlotIndex, primaryAirport->getDeliverySector(), 10433);
}


void InitLeaveList( void )
{
	INT32 iCounter = 0;

	// init leave list with NULLS/zeroes
	for( iCounter = 0; iCounter < NUM_LEAVE_LIST_SLOTS; iCounter++ )
	{
		gpLeaveListHead[ iCounter ] = NULL;
		guiLeaveListOwnerProfileId[ iCounter ] = NO_PROFILE;
	}
}


void ShutDownLeaveList( void )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < NUM_LEAVE_LIST_SLOTS; iCounter++ )
	{
		// go through nodes and free them
		if ( gpLeaveListHead[ iCounter ] != NULL )
		{
			FreeLeaveListSlot( iCounter );
		}
	}
}


void AddItemToLeaveIndex(const OBJECTTYPE* const o, const UINT32 uiSlotIndex)
{
	Assert(uiSlotIndex < NUM_LEAVE_LIST_SLOTS);

	if (o == NULL) return;

	MERC_LEAVE_ITEM* const mli = new MERC_LEAVE_ITEM{};
	mli->o     = *o;
	mli->pNext = NULL;

	// Append node to list
	MERC_LEAVE_ITEM** anchor = &gpLeaveListHead[uiSlotIndex];
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = mli;
}


// release memory for all items in this slot's leave item list
static void FreeLeaveListSlot(UINT32 uiSlotIndex)
{
	MERC_LEAVE_ITEM *pCurrent = NULL, *pTemp = NULL;

	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );

	pCurrent = gpLeaveListHead[ uiSlotIndex ];

	// go through nodes and free them
	while( pCurrent )
	{
		pTemp = pCurrent->pNext;
		delete pCurrent;
		pCurrent = pTemp;
	}

	gpLeaveListHead[ uiSlotIndex ] = NULL;
}


// first free slot in equip leave list
static INT32 FindFreeSlotInLeaveList(void)
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < NUM_LEAVE_LIST_SLOTS; iCounter++ )
	{
		if( gpLeaveListHead[ iCounter ] == NULL )
		{
			return( iCounter );
		}
	}

	return( -1 );
}


static void SetUpMercAboutToLeaveEquipment(UINT32 ubProfileId, UINT32 uiSlotIndex);


// Set up a drop list for this grunt, remove items from inventory and profile
static INT32 SetUpDropItemListForMerc(SOLDIERTYPE& s)
{
	INT32 const slot = FindFreeSlotInLeaveList();
	if (slot == -1) return -1;

	CFOR_EACH_SOLDIER_INV_SLOT(i, s)
	{
		if (i->ubNumberOfObjects == 0) continue;

		// Make a linked list of the items left behind, with the ptr to its head in this free slot
		AddItemToLeaveIndex(i, slot);
		// Store owner's profile id for the items added to this leave slot index
		SetUpMercAboutToLeaveEquipment(s.ubProfile, slot);
	}

	/* ATE: Added this to drop keyring keys - the 2nd last paramter says to add it
	 * to a leave list; the gridno, level and visiblity are ignored */
	DropKeysInKeyRing(s, NOWHERE, 0, VISIBILITY_0, true, slot, false);

	// Zero out profiles
	MERCPROFILESTRUCT& p = GetProfile(s.ubProfile);
	std::fill(std::begin(p.bInvStatus), std::end(p.bInvStatus), 0);
	std::fill(std::begin(p.bInvNumber), std::end(p.bInvNumber), 0);
	std::fill(std::begin(p.inv), std::end(p.inv), 0);

	return slot;
}


// store owner's profile id for the items added to this leave slot index
static void SetUpMercAboutToLeaveEquipment(UINT32 ubProfileId, UINT32 uiSlotIndex)
{
	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );

	// store the profile ID of this merc in the same slot that the items are gonna be dropped in
	guiLeaveListOwnerProfileId[ uiSlotIndex ] = ubProfileId;

}


void HandleGroupAboutToArrive( void )
{
	// reblit map to change the color of the "people in motion" marker
	fMapPanelDirty = TRUE;

	DoDeadIsDeadSaveIfNecessary();

	// ARM - commented out - don't see why this is needed
//	fTeamPanelDirty = TRUE;
//	fCharacterInfoPanelDirty = TRUE;
}


void CreateMapStatusBarsRegion( void )
{

	// create the status region over the bSelectedCharacter info region, to get quick rundown of merc's status
	MSYS_DefineRegion( &gMapStatusBarsRegion, BAR_INFO_X - 3, BAR_INFO_Y - 42,(INT16)( BAR_INFO_X + 17), (INT16)(BAR_INFO_Y ), MSYS_PRIORITY_HIGH + 5,
							MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );
}


void RemoveMapStatusBarsRegion( void )
{

	// remove the bSelectedInfoCharacter helath, breath and morale bars info region
	MSYS_RemoveRegion( &gMapStatusBarsRegion );
}


void UpdateCharRegionHelpText(void)
{
	SOLDIERTYPE const* const s = GetSelectedInfoChar();

	// health/energy/morale
	ST::string status;
	if (!s || s->bLife == 0)
	{
		status = ST::null;
	}
	else if (s->bAssignment == ASSIGNMENT_POW)
	{
		// POW - stats unknown
		status = ST::format("{}: ??, {}: ??, {}: ??",
			pMapScreenStatusStrings[0],
			pMapScreenStatusStrings[1],
			pMapScreenStatusStrings[2]
		);
	}
	else if (AM_A_ROBOT(s))
	{
		// robot (condition only)
		status = ST::format("{}: {}/{}",
			pMapScreenStatusStrings[3], s->bLife, s->bLifeMax
		);
	}
	else if (s->uiStatusFlags & SOLDIER_VEHICLE)
	{
		// vehicle (condition/fuel)
		status = ST::format("{}: {}/{}, {}: {}/{}",
			pMapScreenStatusStrings[3], s->bLife,   s->bLifeMax,
			pMapScreenStatusStrings[4], s->bBreath, s->bBreathMax
		);
	}
	else
	{
		// person (health/energy/morale)
		ST::string morale = GetMoraleString(*s);
		status = ST::format("{}: {}/{}, {}: {}/{}, {}: {}",
			pMapScreenStatusStrings[0], s->bLife,   s->bLifeMax,
			pMapScreenStatusStrings[1], s->bBreath, s->bBreathMax,
			pMapScreenStatusStrings[2], morale
		);
	}
	gMapStatusBarsRegion.SetFastHelpText(status);

	// update contract button help text
	EnableButton(giMapContractButton, s && CanExtendContractForSoldier(s));
}


// find this merc in the mapscreen list and set as selected
void FindAndSetThisContractSoldier( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;

	fShowContractMenu = FALSE;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const s = gCharactersList[iCounter].merc;
		if (s == NULL) continue;

		if (s == pSoldier)
		{
			ChangeSelectedInfoChar( ( INT8 )iCounter, TRUE );
			bSelectedContractChar = ( INT8 )iCounter;
			fShowContractMenu = TRUE;

			// create
			RebuildContractBoxForMerc( pSoldier );

			fTeamPanelDirty = TRUE;
			fCharacterInfoPanelDirty = TRUE;
		}
	}
}


void HandleMAPUILoseCursorFromOtherScreen( void )
{
	// rerender map without cursors
	fMapPanelDirty = TRUE;

	if ( fInMapMode )
	{
		RenderMapRegionBackground( );
	}
}


void UpdateMapScreenAssignmentPositions( void )
{
	// set the position of the pop up boxes

	if( guiCurrentScreen != MAP_SCREEN )
	{
		return;
	}


	if( bSelectedAssignChar == -1 )
	{
		if (!gfPreBattleInterfaceActive) giBoxY = 0;
		return;
	}

	if (gCharactersList[bSelectedAssignChar].merc == NULL)
	{
		if (!gfPreBattleInterfaceActive) giBoxY = 0;
		return;
	}

	if( gfPreBattleInterfaceActive )
	{
		// do nothing
	}
	else
	{
		giBoxY = ( Y_START + ( bSelectedAssignChar ) * ( Y_SIZE + 2 ) );

/* ARM: Removed this - refreshes fine without it, apparently
		// make sure the menus don't overlap the map screen bottom panel (but where did 102 come from?)
		if( giBoxY >= ( MAP_BOTTOM_Y - 102 ) )
			giBoxY = MAP_BOTTOM_Y - 102;
*/
	}



	AssignmentPosition.iY = giBoxY;

	AttributePosition.iY = TrainPosition.iY = AssignmentPosition.iY + ( GetFontHeight( MAP_SCREEN_FONT ) + 2 )* ASSIGN_MENU_TRAIN;

	VehiclePosition.iY = AssignmentPosition.iY + ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_VEHICLE;
	SquadPosition.iY = AssignmentPosition.iY;

	if( fShowAssignmentMenu )
	{
		SetBoxY(ghAssignmentBox, giBoxY);
		SetBoxY(ghEpcBox,        giBoxY);
	}

	if( fShowAttributeMenu )
	{
		SetBoxY(ghAttributeBox, giBoxY + (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_TRAIN);
	}

	if( fShowRepairMenu )
	{
		SetBoxY(ghRepairBox, giBoxY + (GetFontHeight(MAP_SCREEN_FONT) + 2) * ASSIGN_MENU_REPAIR);
	}
}


void RandomMercInGroupSaysQuote(GROUP const& g, UINT16 const quote_num)
{
	/* If traversing tactically, don't do this, unless time compression was
	 * required for some reason (don't go to sector) */
	if ((gfTacticalTraversal || g.ubSectorZ > 0) && !IsTimeBeingCompressed())
	{
		return;
	}

	// Choose somebody in group
	SOLDIERTYPE* mercs_in_group[20];
	UINT8        n_mercs = 0;
	CFOR_EACH_PLAYER_IN_GROUP(i, &g)
	{
		SOLDIERTYPE& s = *i->pSoldier;
		if (s.bLife < OKLIFE) continue;
		if (IsMechanical(s))  continue;
		if (AM_AN_EPC(&s))    continue;
		if (s.fMercAsleep)    continue;
		mercs_in_group[n_mercs++] = &s;
	}

	if (n_mercs > 0)
	{
		SOLDIERTYPE& chosen = *mercs_in_group[Random(n_mercs)];
		TacticalCharacterDialogue(&chosen, quote_num);
	}
}


INT32 GetNumberOfPeopleInCharacterList( void )
{
	// get the number of valid mercs in the mapscreen character list
	INT32 count = 0;
	CFOR_EACH_IN_CHAR_LIST(c) ++count;
	return count;
}


static bool ValidSelectableCharForNextOrPrev(SOLDIERTYPE const& s)
{
	bool const holding_item = gpItemPointer || fMapInventoryItem;
	return
		/* If showing merc inventory or holding an item, then the new guy must have
			* accessible inventory */
		((!holding_item && !fShowInventoryFlag) || MapCharacterHasAccessibleInventory(s)) &&
		(!holding_item || MapscreenCanPassItemToChar(&s));
}


BOOLEAN MapscreenCanPassItemToChar(const SOLDIERTYPE* const pNewSoldier)
{
	SOLDIERTYPE *pOldSoldier;

	// assumes we're holding an item
	Assert(gpItemPointer || fMapInventoryItem);

	// if in a hostile sector, disallow
	if ( gTacticalStatus.fEnemyInSector )
	{
		return( FALSE );
	}

	// if showing sector inventory, and the item came from there
	if ( fShowMapInventoryPool && !gpItemPointerSoldier && fMapInventoryItem )
	{
		// disallow passing items to anyone not in that sector
		if ( pNewSoldier->sSectorX != sSelMapX ||
			pNewSoldier->sSectorY != sSelMapY ||
			pNewSoldier->bSectorZ != ( INT8 )( iCurrentMapSectorZ ) )
		{
			return( FALSE );
		}

		if ( pNewSoldier->fBetweenSectors )
		{
			return( FALSE );
		}
	}


	// if we know who it came from
	if ( gpItemPointerSoldier )
	{
		pOldSoldier = gpItemPointerSoldier;
	}
	else
	{
		// it came from either the currently selected merc, or the sector inventory
		if (fMapInventoryItem)
		{
			pOldSoldier = NULL;
		}
		else
		{
			pOldSoldier = GetSelectedInfoChar();
		}
	}


	// if another merc had it previously
	if ( pOldSoldier != NULL )
	{
		// disallow passing items to a merc not in the same sector
		if ( pNewSoldier->sSectorX != pOldSoldier->sSectorX ||
			pNewSoldier->sSectorY != pOldSoldier->sSectorY ||
			pNewSoldier->bSectorZ != pOldSoldier->bSectorZ )
		{
			return( FALSE );
		}

		// if on the road
		if ( pNewSoldier->fBetweenSectors )
		{
			// other guy must also be on the road...
			if ( !pOldSoldier->fBetweenSectors )
			{
				return( FALSE );
			}

			// only exchanges between those is the same squad or vehicle are permitted
			if ( pNewSoldier->bAssignment != pOldSoldier->bAssignment )
			{
				return( FALSE );
			}

			// if in vehicles, make sure it's the same one
			if ( ( pNewSoldier->bAssignment == VEHICLE ) && ( pNewSoldier->iVehicleId != pOldSoldier->iVehicleId ) )
			{
				return( FALSE );
			}
		}
	}


	// passed all tests
	return( TRUE );
}



void GoToNextCharacterInList( void )
{
	INT32 iCounter = 0, iCount = 0;

	if (fShowDescriptionFlag) return;

	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}

	// is the current guy invalid or the first one?
	if( ( bSelectedInfoChar == -1 )|| ( bSelectedInfoChar == MAX_CHARACTER_COUNT ) )
	{
		iCount = 0;
	}
	else
	{
		iCount = bSelectedInfoChar + 1;
	}

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const s = gCharactersList[iCount].merc;
		if (s != NULL &&
				iCount < MAX_CHARACTER_COUNT &&
				ValidSelectableCharForNextOrPrev(*s))
		{
			ChangeSelectedInfoChar( ( INT8 )iCount, TRUE );
			break;
		}
		else
		{
			iCount++;

			if( iCount >= MAX_CHARACTER_COUNT )
			{
				iCount = 0;
			}
		}
	}
}


void GoToPrevCharacterInList( void )
{
	INT32 iCounter = 0, iCount = 0;

	if (fShowDescriptionFlag) return;

	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}

	// is the current guy invalid or the first one?
	if( ( bSelectedInfoChar == -1 ) || ( bSelectedInfoChar == 0 ) )
	{
		iCount = MAX_CHARACTER_COUNT;
	}
	else
	{
		iCount = bSelectedInfoChar - 1;
	}

	// now run through the list and find first prev guy
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const s = gCharactersList[iCount].merc;
		if (s != NULL &&
				iCount < MAX_CHARACTER_COUNT &&
				ValidSelectableCharForNextOrPrev(*s))
		{
			ChangeSelectedInfoChar( ( INT8 )iCount, TRUE );
			break;
		}
		else
		{
			iCount--;

			if( iCount < 0 )
			{
				// was FIRST_VEHICLE
				iCount = MAX_CHARACTER_COUNT;
			}
		}
	}
}


void HandleMinerEvent(ProfileID const ubMinerProfileID, INT16 const sQuoteNumber, BOOLEAN const fForceMapscreen)
{
	BOOLEAN fFromMapscreen = FALSE;


	if ( guiCurrentScreen == MAP_SCREEN )
	{
		fFromMapscreen = TRUE;
	}
	else
	{
		// if transition to mapscreen is required
		if ( fForceMapscreen )
		{
			// switch to mapscreen so we can flash the mine sector the guy is talking about
			LeaveTacticalScreen(MAP_SCREEN);
			fFromMapscreen = TRUE;
		}
	}

	if ( fFromMapscreen )
	{
		// if not showing map surface level
		if ( iCurrentMapSectorZ != 0 )
		{
			// switch to it, because the miner locators wouldn't show up if we're underground while they speak
			ChangeSelectedMapSector( sSelMapX, sSelMapY, 0 );
		}

		fMapPanelDirty = TRUE;
	}

	CharacterDialogue(ubMinerProfileID, sQuoteNumber, GetExternalNPCFace(ubMinerProfileID), DIALOGUE_EXTERNAL_NPC_UI, FALSE);
}


static void StopMapScreenHelpText(void);
static void StopShowingInterfaceFastHelpText(void);


void ShutDownUserDefineHelpTextRegions( void )
{
	// dirty the tactical panel
	fInterfacePanelDirty = DIRTYLEVEL2;
	SetRenderFlags( RENDER_FLAG_FULL );

	//dirty the map panel
	StopMapScreenHelpText( );

	//r eset tactical flag too
	StopShowingInterfaceFastHelpText( );
}


void SetUpFastHelpRegion(INT32 x, INT32 y, INT32 width, const ST::string& str)
{
	FASTHELPREGION* fhr = &pFastHelpMapScreenList[0];
	fhr->iX = x;
	fhr->iY = y;
	fhr->iW = width;
	fhr->FastHelpText = str;
	giSizeOfInterfaceFastHelpTextList = 1;
}


static void DisplayFastHelpRegions(FASTHELPREGION* pRegion, INT32 iSize);
static void SetUpShutDownMapScreenHelpTextScreenMask(void);


// handle the actual showing of the interface fast help text
void HandleShowingOfTacticalInterfaceFastHelpText( void )
{
	static BOOLEAN fTextActive = FALSE;

	if( fInterfaceFastHelpTextActive )
	{
		DisplayFastHelpRegions(pFastHelpMapScreenList, giSizeOfInterfaceFastHelpTextList);

		PauseGame();

		// lock out the screen
		SetUpShutDownMapScreenHelpTextScreenMask( );

		gfIgnoreScrolling = TRUE;

		// the text is active
		fTextActive = TRUE;

	}
	else if (!fInterfaceFastHelpTextActive && fTextActive)
	{
		fTextActive = FALSE;
		UnPauseGame();
		gfIgnoreScrolling = FALSE;

		// shut down
		ShutDownUserDefineHelpTextRegions( );
	}
}

// start showing fast help text
void StartShowingInterfaceFastHelpText( void )
{
	fInterfaceFastHelpTextActive = TRUE;
}


// stop showing interface fast help text
static void StopShowingInterfaceFastHelpText(void)
{
	fInterfaceFastHelpTextActive = FALSE;
}


// is the interface text up?
BOOLEAN IsTheInterfaceFastHelpTextActive( void )
{
	return( fInterfaceFastHelpTextActive );
}


static void DisplayUserDefineHelpTextRegions(FASTHELPREGION* pRegion);


// display all the regions in the list
static void DisplayFastHelpRegions(FASTHELPREGION* pRegion, INT32 iSize)
{
	INT32 iCounter = 0;

	// run through and show all the regions
	for( iCounter = 0; iCounter < iSize; iCounter++ )
	{
		DisplayUserDefineHelpTextRegions( &( pRegion[ iCounter ] ) );
	}
}


// show one region
static void DisplayUserDefineHelpTextRegions(FASTHELPREGION* pRegion)
{
	INT32 iX,iY,iW,iH;

	iX = pRegion->iX;
	iY = pRegion->iY;
	// get the width and height of the string
	iW = (INT32)( pRegion->iW ) + 14;
	iH = IanWrappedStringHeight(pRegion->iW, 0, FONT10ARIAL, pRegion->FastHelpText);

	// tack on the outer border
	iH += 14;

	// gone not far enough?
	if ( iX < 0 )
		iX = 0;

	// gone too far
	if ( ( pRegion->iX + iW ) >= SCREEN_WIDTH )
		iX = (SCREEN_WIDTH - iW - 4);

	// what about the y value?
	iY = (INT32)pRegion->iY - (  iH * 3 / 4);

	// not far enough
	if (iY < 0)
		iY = 0;

	// too far
	if ( (iY + iH) >= SCREEN_HEIGHT )
		iY = (SCREEN_HEIGHT - iH - 15);

	{ SGPVSurface::Lock l(FRAME_BUFFER);
		SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		UINT16* const pDestBuf = l.Buffer<UINT16>();
		RectangleDraw(TRUE, iX + 1, iY + 1, iX + iW - 1, iY + iH - 1, Get16BPPColor(FROMRGB( 65,  57, 15)), pDestBuf);
		RectangleDraw(TRUE, iX,     iY,     iX + iW - 2, iY + iH - 2, Get16BPPColor(FROMRGB(227, 198, 88)), pDestBuf);
	}
	FRAME_BUFFER->ShadowRect(iX + 2, iY + 2, iX + iW - 3, iY + iH - 3);
	FRAME_BUFFER->ShadowRect(iX + 2, iY + 2, iX + iW - 3, iY + iH - 3);

	iH = DisplayWrappedString(iX + 10, iY + 6, pRegion->iW, 0, FONT10ARIAL, FONT_BEIGE, pRegion->FastHelpText, FONT_NEARBLACK, MARK_DIRTY);

	InvalidateRegion(  iX, iY, (iX + iW) , (iY + iH + 20 ) );
}


// stop the help text in mapscreen
static void StopMapScreenHelpText(void)
{
	fTeamPanelDirty = TRUE;
	fMapPanelDirty = TRUE;
	fCharacterInfoPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;

	SetUpShutDownMapScreenHelpTextScreenMask( );
}


static void MapScreenHelpTextScreenMaskBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void SetUpShutDownMapScreenHelpTextScreenMask(void)
{
	static BOOLEAN fCreated = FALSE;

	// create or destroy the screen mask as needed
	if (fInterfaceFastHelpTextActive && !fCreated)
	{
		MSYS_DefineRegion(&gMapScreenHelpTextMask, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MapScreenHelpTextScreenMaskBtnCallback);
		fCreated = TRUE;
	}
	else if (!fInterfaceFastHelpTextActive && fCreated)
	{
		MSYS_RemoveRegion( &gMapScreenHelpTextMask );

		fCreated = FALSE;
	}
}


static void MapScreenHelpTextScreenMaskBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		// stop showing
		ShutDownUserDefineHelpTextRegions( );
	}
	else if( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		// stop showing
		ShutDownUserDefineHelpTextRegions( );
	}
}


static bool IsSoldierSelectedForMovement(SOLDIERTYPE const& s)
{
	// run through the list and turn this soldiers value on
	for (INT32 i = 0; i != giNumberOfSoldiersInSectorMoving; ++i)
	{
		if (pSoldierMovingList[i] != &s) continue;
		if (!fSoldierIsMoving[i])        continue; // XXX break?
		return true;
	}
	return false;
}


static bool IsSquadSelectedForMovement(INT32 const squad_no)
{
	for (INT32 i = 0; i != giNumberOfSquadsInSectorMoving; ++i)
	{
		if (iSquadMovingList[i] != squad_no) continue;
		if (!fSquadIsMoving[i])              continue; // XXX break?
		return true;
	}
	return false;
}


static bool IsVehicleSelectedForMovement(INT32 const vehicle_id)
{
	for (INT32 i = 0; i != giNumberOfVehiclesInSectorMoving; ++i)
	{
		if (iVehicleMovingList[i] != vehicle_id) continue;
		if (!fVehicleIsMoving[i])                continue; // XXX break?
		return true;
	}
	return false;
}


static void SelectSoldierForMovement(SOLDIERTYPE const& s)
{
	for (INT32 i = 0; i != giNumberOfSoldiersInSectorMoving; ++i)
	{
		if (pSoldierMovingList[i] != &s) continue;
		// Turn the selected soldier on
		fSoldierIsMoving[i] = true;
		break;
	}
}


static void DeselectSoldierForMovement(SOLDIERTYPE const& s)
{
	for (INT32 i = 0; i != giNumberOfSoldiersInSectorMoving; ++i)
	{
		if (pSoldierMovingList[i] != &s) continue;
		// Turn the selected soldier off
		fSoldierIsMoving[i] = false;
		break;
	}
}


static BOOLEAN CanMoveBoxSoldierMoveStrategically(SOLDIERTYPE* pSoldier, BOOLEAN fShowErrorMessage);


static void SelectSquadForMovement(INT32 const squad_no)
{
	// Run through squad list and set them on
	for (INT32 i = 0; i != giNumberOfSquadsInSectorMoving; ++i)
	{
		if (iSquadMovingList[i] != squad_no) continue;

		// Try to select everyone in squad
		bool some_cant_move = false;
		bool first_failure  = true;
		FOR_EACH_IN_SQUAD(k, squad_no)
		{
			SOLDIERTYPE& s = **k;
			if (!s.bActive) continue;
			/* Is he able and allowed to move? Report only the first reason for
				* failure encountered */
			if (CanMoveBoxSoldierMoveStrategically(&s, first_failure))
			{
				SelectSoldierForMovement(s);
			}
			else
			{
				some_cant_move = true;
				first_failure  = false;
			}
		}

		if (!some_cant_move) fSquadIsMoving[i] = TRUE;
		break;
	}
}


static void DeselectSquadForMovement(INT32 const squad_no)
{
	// Run through squad list and set them off
	for (INT32 i = 0; i != giNumberOfSquadsInSectorMoving; ++i)
	{
		if (iSquadMovingList[i] != squad_no) continue;

		fSquadIsMoving[i] = FALSE;

		// Now deselect everyone in squad
		FOR_EACH_IN_SQUAD(k, squad_no)
		{
			SOLDIERTYPE& s = **k;
			if (!s.bActive) continue;
			DeselectSoldierForMovement(s);
		}
		break;
	}
}


static bool AllSoldiersInSquadSelected(INT32 const squad_no)
{
	// Is everyone on this squad moving?
	for (INT32 i = 0; i != giNumberOfSoldiersInSectorMoving; ++i)
	{
		if (pSoldierMovingList[i]->bAssignment != squad_no) continue;
		if (fSoldierIsMoving[i]) continue;
		return false;
	}
	return true;
}


static void SelectVehicleForMovement(INT32 const vehicle_id, BOOLEAN const and_all_on_board)
{
	// Run through vehicle list and set them on
	for (INT32 i = 0; i != giNumberOfVehiclesInSectorMoving; ++i)
	{
		if (iVehicleMovingList[i] != vehicle_id) continue;

		bool               first_failure = true;
		bool               has_driver    = false;
		VEHICLETYPE const& v             = pVehicleList[vehicle_id];
		CFOR_EACH_PASSENGER(v, k)
		{
			SOLDIERTYPE& s = **k;

			if (and_all_on_board)
			{ // Try to select everyone in vehicle
				if (s.bActive)
				{
					// Is he able and allowed to move?
					if (CanMoveBoxSoldierMoveStrategically(&s, first_failure))
					{
						SelectSoldierForMovement(s);
					}
					else
					{
						first_failure = false;
					}
				}
			}

			if (IsSoldierSelectedForMovement(s)) has_driver = true;
		}

		/* Vehicle itself can only move if at least one passenger can move and is
			* moving */
		if (has_driver) fVehicleIsMoving[i] = TRUE;
		break;
	}
}


static void DeselectVehicleForMovement(INT32 const vehicle_id)
{
	// run through vehicle list and set them off
	for (INT32 i = 0; i != giNumberOfVehiclesInSectorMoving; ++i)
	{
		if (iVehicleMovingList[i] != vehicle_id) continue;

		fVehicleIsMoving[i] = FALSE;

		// Now deselect everyone in vehicle
		VEHICLETYPE const& v = pVehicleList[vehicle_id];
		CFOR_EACH_PASSENGER(v, i)
		{
			SOLDIERTYPE& s = **i;
			if (s.bActive) DeselectSoldierForMovement(s);
		}
		break;
	}
}


static INT32 HowManyMovingSoldiersInVehicle(INT32 const vehicle_id)
{
	INT32 n = 0;
	for (INT32 i = 0; i != giNumberOfSoldiersInSectorMoving; ++i)
	{
		SOLDIERTYPE const& s = *pSoldierMovingList[i];
		if (s.bAssignment != VEHICLE)    continue;
		if (s.iVehicleId  != vehicle_id) continue;
		if (!fSoldierIsMoving[i])        continue;
		++n;
	}
	return n;
}


static INT32 HowManyMovingSoldiersInSquad(INT32 const squad_no)
{
	INT32 n = 0;
	for (INT32 i = 0; i != giNumberOfSoldiersInSectorMoving; ++i)
	{
		if (pSoldierMovingList[i]->bAssignment != squad_no) continue;
		if (!fSoldierIsMoving[i])                           continue;
		++n;
	}
	return n;
}


static void AddSoldierToMovingLists(SOLDIERTYPE& s)
{
	for (INT32 i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		SOLDIERTYPE*& slot = pSoldierMovingList[i];
		if (slot == &s) return;

		if (slot) continue;
		// Found a free slot
		slot                = &s;
		fSoldierIsMoving[i] = false;
		++giNumberOfSoldiersInSectorMoving;
		return;
	}
}


static void AddSquadToMovingLists(INT32 const squad_no)
{
	if (squad_no == -1) return; // Invalid squad

	for(INT32 i = 0; i < NUMBER_OF_SQUADS; ++i)
	{
		INT32& slot = iSquadMovingList[i];
		if (slot == squad_no) return;

		if (slot != -1) continue;
		// Found a free slot
		slot              = squad_no;
		fSquadIsMoving[i] = FALSE;
		++giNumberOfSquadsInSectorMoving;
		return;
	}
}


static void AddVehicleToMovingLists(INT32 const vehicle_id)
{
	if (vehicle_id == -1) return;

	for (INT32 i = 0; i != NUMBER_OF_SQUADS; ++i)
	{
		INT32& slot = iVehicleMovingList[i];
		if (slot == vehicle_id) return;

		if (slot != -1) continue;
		// Found a free slot
		slot                = vehicle_id;
		fVehicleIsMoving[i] = FALSE;
		++giNumberOfVehiclesInSectorMoving;
		return;
	}
}


// The alternate mapscreen movement system
static void InitializeMovingLists()
{
	giNumberOfSoldiersInSectorMoving = 0;
	giNumberOfSquadsInSectorMoving   = 0;
	giNumberOfVehiclesInSectorMoving = 0;

	// Init the soldiers
	for (INT32 i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		pSoldierMovingList[i] = 0;
		fSoldierIsMoving[i]   = false;
	}

	// Init the squads
	for (INT32 i = 0; i != NUMBER_OF_SQUADS; ++i)
	{
		iSquadMovingList[i] = -1;
		fSquadIsMoving[i]   = FALSE;
	}

	// Init the vehicles
	for (INT32 i = 0; i != NUMBER_OF_SQUADS; ++i)
	{
		iVehicleMovingList[i] = -1;
		fVehicleIsMoving[i]   = FALSE;
	}
}


static bool IsAnythingSelectedForMoving()
{
	for (INT32 i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		if (!pSoldierMovingList[i]) continue;
		if (!fSoldierIsMoving[i])   continue;
		return true;
	}

	for (INT32 i = 0; i != NUMBER_OF_SQUADS; ++i)
	{
		if (iSquadMovingList[i] == -1) continue;
		if (!fSquadIsMoving[i])        continue;
		return true;
	}

	for (INT32 i = 0; i != NUMBER_OF_SQUADS; ++i)
	{
		if (iVehicleMovingList[i] == -1) continue;
		if (!fVehicleIsMoving[i])        continue;
		return true;
	}

	return false;
}


static void BuildMouseRegionsForMoveBox(void);
static void ClearMouseRegionsForMoveBox(void);
static void CreatePopUpBoxForMovementBox(void);


void CreateDestroyMovementBox( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	static BOOLEAN fCreated = FALSE;


	// not allowed for underground movement!
	Assert( sSectorZ == 0 );

	if (fShowMapScreenMovementList && !fCreated)
	{
		fCreated = TRUE;

		// create the box and mouse regions
		CreatePopUpBoxForMovementBox( );
		BuildMouseRegionsForMoveBox( );
		CreateScreenMaskForMoveBox( );
		fMapPanelDirty = TRUE;
	}
	else if (!fShowMapScreenMovementList && fCreated)
	{
		fCreated = FALSE;

		// destroy the box and mouse regions
		ClearMouseRegionsForMoveBox( );
		RemoveBox( ghMoveBox );
		ghMoveBox = NO_POPUP_BOX;
		RemoveScreenMaskForMoveBox( );
		fMapPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;		// really long move boxes can overlap bottom panel
	}

}


void SetUpMovingListsForSector(INT16 const x, INT16 const y, INT16 const z)
{
	// Not allowed for underground movement
	Assert(z == 0);

	InitializeMovingLists();

	/* Note that Skyrider can't be moved using the move box, and won't appear
	 * because the helicoprer is not in the char list */
	CFOR_EACH_IN_CHAR_LIST(i)
	{
		SOLDIERTYPE& s = *i->merc;
		if (s.bAssignment == IN_TRANSIT)     continue;
		if (s.bAssignment == ASSIGNMENT_POW) continue;
		if (s.sSectorX != x || s.sSectorY != y || s.bSectorZ != z) continue;

		if (s.uiStatusFlags & SOLDIER_VEHICLE)
		{
			VEHICLETYPE const& v = GetVehicle(s.bVehicleID);
			// If it can move (can't be empty)
			if (GetNumberInVehicle(v) == 0) continue;
			AddVehicleToMovingLists(s.bVehicleID);
		}
		else
		{
			// Alive, not aboard Skyrider (airborne or not!)
			if (s.bLife < OKLIFE) continue;
			if (InHelicopter(s))  continue;
			AddSoldierToMovingLists(s);
			if (s.bAssignment < ON_DUTY) // If on a squad
			{ // Add squad (duplicates ok, they're ignored inside the function)
				AddSquadToMovingLists(s.bAssignment);
			}
		}
	}

	fShowMapScreenMovementList = TRUE;
	CreateDestroyMovementBox(x, y, z);
}


static void AddStringsToMoveBox(PopUpBox*);


static void CreatePopUpBoxForMovementBox(void)
{
	SGPPoint const MovePosition = { (UINT16)(STD_SCREEN_X + 450), (UINT16)(STD_SCREEN_Y + 100) };

	// create the pop up box and mouse regions for movement list
	PopUpBox* const box = CreatePopUpBox(MovePosition, POPUP_BOX_FLAG_RESIZE, FRAME_BUFFER, guiPOPUPBORDERS, guiPOPUPTEX, 6, 6, 4, 4, 2);
	ghMoveBox = box;

	AddStringsToMoveBox(box);

	SetBoxFont(      box, MAP_SCREEN_FONT);
	SetBoxHighLight( box, FONT_WHITE);
	SetBoxForeground(box, FONT_LTGREEN);
	SetBoxBackground(box, FONT_BLACK);
	SetBoxShade(     box, FONT_BLACK);

	// make the header line WHITE
	SetBoxLineForeground(box, 0, FONT_WHITE);

	// make the done and cancel lines YELLOW
	const UINT32 line_count = GetNumberOfLinesOfTextInBox(box);
	SetBoxLineForeground(box, line_count - 1, FONT_YELLOW);
	if (IsAnythingSelectedForMoving()) SetBoxLineForeground(box, line_count - 2, FONT_YELLOW);

	ResizeBoxToText(box);

	// adjust position to try to keep it in the map area as good as possible
	SGPBox const& area = GetBoxArea(box);
	if (area.x + area.w >= MAP_VIEW_START_X + MAP_VIEW_WIDTH)
	{
		SetBoxX(box, MAX(MAP_VIEW_START_X, MAP_VIEW_START_X + MAP_VIEW_WIDTH - area.w));
	}
	if (area.y + area.h >= MAP_VIEW_START_Y + MAP_VIEW_HEIGHT)
	{
		SetBoxY(box, MAX(MAP_VIEW_START_Y, MAP_VIEW_START_Y + MAP_VIEW_HEIGHT - area.h));
	}
}


static BOOLEAN AllOtherSoldiersInListAreSelected(void);


static void AddStringsToMoveBox(PopUpBox* const box)
{
	INT32 iCount = 0, iCountB = 0;
	BOOLEAN fFirstOne = TRUE;

	// clear all the strings out of the box
	RemoveAllBoxStrings(box);

	// add title
	ST::string sStringB = GetShortSectorString(sSelMapX, sSelMapY);
	ST::string sString = st_format_printf(pMovementMenuStrings[0], sStringB);
	AddMonoString(box, sString);


	// blank line
	AddMonoString(box, ST::null);


	// add squads
	for( iCount = 0; iCount < giNumberOfSquadsInSectorMoving; iCount++ )
	{
		// add this squad, now add all the grunts in it
		sString = ST::format(fSquadIsMoving[iCount] ? "*{}*" : "{}", pSquadMenuStrings[iSquadMovingList[iCount]]);
		AddMonoString(box, sString);

		// now add all the grunts in it
		for( iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; iCountB++ )
		{
			if( pSoldierMovingList[ iCountB ] -> bAssignment == iSquadMovingList[ iCount ] )
			{
				// add mercs in squads
				if (IsSoldierSelectedForMovement(*pSoldierMovingList[iCountB]))
				{
					sString = ST::format("   *{}*", pSoldierMovingList[ iCountB ]->name);
				}
				else
				{
					sString = ST::format("   {}", pSoldierMovingList[ iCountB ]->name);
				}
				AddMonoString(box, sString);
			}
		}
	}


	// add vehicles
	for( iCount = 0; iCount < giNumberOfVehiclesInSectorMoving; iCount++ )
	{
		// add this vehicle
		sString = ST::format(fVehicleIsMoving[iCount] ? "*{}*" : "{}", pVehicleStrings[pVehicleList[iVehicleMovingList[iCount]].ubVehicleType]);
		AddMonoString(box, sString);

		// now add all the grunts in it
		for( iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; iCountB++ )
		{
			if( ( pSoldierMovingList[ iCountB ] -> bAssignment == VEHICLE ) &&( pSoldierMovingList[ iCountB ] -> iVehicleId == iVehicleMovingList[ iCount ] ) )
			{
				// add mercs in vehicles
				if (IsSoldierSelectedForMovement(*pSoldierMovingList[iCountB]))
				{
					sString = ST::format("   *{}*", pSoldierMovingList[ iCountB ]->name);
				}
				else
				{
					sString = ST::format("   {}", pSoldierMovingList[ iCountB ]->name);
				}
				AddMonoString(box, sString);
			}
		}
	}


	fFirstOne = TRUE;

	// add "other" soldiers heading, once, if there are any
	for( iCount = 0; iCount < giNumberOfSoldiersInSectorMoving; iCount++ )
	{
		// not on duty, not in a vehicle
		if( ( pSoldierMovingList[ iCount ]->bAssignment >= ON_DUTY ) && ( pSoldierMovingList[ iCount ]->bAssignment != VEHICLE ) )
		{
			if ( fFirstOne )
			{
				// add OTHER header line
				sString = ST::format(AllOtherSoldiersInListAreSelected() ? "*{}*" : "{}", pMovementMenuStrings[3]);
				AddMonoString(box, sString);

				fFirstOne = FALSE;
			}

			// add OTHER soldiers (not on duty nor in a vehicle)
			sString = ST::format(IsSoldierSelectedForMovement(*pSoldierMovingList[iCount]) ? "  *{} ( {} )*" : "  {} ( {} )", pSoldierMovingList[iCount]->name, pAssignmentStrings[pSoldierMovingList[iCount]->bAssignment]);
			AddMonoString(box, sString);
		}
	}


	// blank line
	AddMonoString(box, ST::null);


	if ( IsAnythingSelectedForMoving() )
	{
		// add PLOT MOVE line
		AddMonoString(box, pMovementMenuStrings[1]);
	}
	else
	{
		// blank line
		AddMonoString(box, ST::null);
	}

	// add cancel line
	AddMonoString(box, pMovementMenuStrings[2]);
}


static void MakeRegionBlank(const INT32 i, const UINT16 x, const UINT16 y, const UINT16 w, const UINT16 h)
{
	MSYS_DefineRegion(&gMoveMenuRegion[i], x, y + h * i, x + w, y + h * (i + 1), MSYS_PRIORITY_HIGHEST, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
}


static void MoveMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MoveMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void MakeRegion(const INT32 i, const UINT16 x, const UINT16 y, const UINT16 w, const UINT16 h, const UINT32 val_a, const UINT32 val_b)
{
	MOUSE_REGION* const r = &gMoveMenuRegion[i];
	MSYS_DefineRegion(r, x, y + h * i, x + w, y + h * (i + 1), MSYS_PRIORITY_HIGHEST, MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback);
	MSYS_SetRegionUserData(r, 0, i);
	MSYS_SetRegionUserData(r, 1, val_a);
	MSYS_SetRegionUserData(r, 2, val_b);
}


static void BuildMouseRegionsForMoveBox(void)
{
	SGPBox const& area = GetBoxArea(ghMoveBox);
	INT32  const  x    = area.x;
	INT32  const  y    = area.y + GetTopMarginSize(ghMoveBox) - 2; // -2 to improve highlighting accuracy between lines
	INT32  const  w    = area.w;
	INT32  const  h    = GetLineSpace(ghMoveBox) + GetFontHeight(GetBoxFont(ghMoveBox));
	INT32         i    = 0; // Region index

	MakeRegionBlank(i++, x, y, w, h); // Box heading
	MakeRegionBlank(i++, x, y, w, h); // Blank line

	// Define regions for squad lines
	for (INT32 iCount = 0; iCount < giNumberOfSquadsInSectorMoving; ++iCount)
	{
		MakeRegion(i++, x, y, w, h, SQUAD_REGION, iCount);

		for (INT32 iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; ++iCountB)
		{
			if (pSoldierMovingList[iCountB]->bAssignment == iSquadMovingList[iCount])
			{
				MakeRegion(i++, x, y, w, h, SOLDIER_REGION, iCountB);
			}
		}
	}

	// Define regions for vehicle lines
	for (INT32 iCount = 0; iCount < giNumberOfVehiclesInSectorMoving; ++iCount)
	{
		MakeRegion(i++, x, y, w, h, VEHICLE_REGION, iCount);

		for (INT32 iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; ++iCountB)
		{
			if (pSoldierMovingList[iCountB]->bAssignment == VEHICLE &&
					pSoldierMovingList[iCountB]->iVehicleId  == iVehicleMovingList[iCount])
			{
				MakeRegion(i++, x, y, w, h, SOLDIER_REGION, iCountB);
			}
		}
	}

	// Define regions for "other" soldiers
	BOOLEAN fDefinedOtherRegion = FALSE;
	for (INT32 iCount = 0; iCount < giNumberOfSoldiersInSectorMoving; ++iCount)
	{
		// this guy is not in a squad or vehicle
		if (pSoldierMovingList[iCount]->bAssignment >= ON_DUTY &&
				pSoldierMovingList[iCount]->bAssignment != VEHICLE)
		{
			// this line gets place only once...
			if (!fDefinedOtherRegion)
			{
				MakeRegion(i++, x, y, w, h, OTHER_REGION, 0);
				fDefinedOtherRegion = TRUE;
			}
			MakeRegion(i++, x, y, w, h, SOLDIER_REGION, iCount);
		}
	}

	Assert(i == 2 /* heading + blank line */ + giNumberOfSquadsInSectorMoving + giNumberOfVehiclesInSectorMoving + giNumberOfSoldiersInSectorMoving + (fDefinedOtherRegion ? 1 : 0));

	MakeRegionBlank(i++, x, y, w, h); // blank line

	if (IsAnythingSelectedForMoving())
	{
		MakeRegion(i++, x, y, w, h, DONE_REGION, 0); // DONE line
	}
	else
	{
		MakeRegionBlank(i++, x, y, w, h); // blank line
	}

	MakeRegion(i++, x, y, w, h, CANCEL_REGION, 0); // CANCEL line
}


static void ClearMouseRegionsForMoveBox(void)
{
	INT32 iCounter = 0;

	// run through list of mouse regions
	for( iCounter = 0; iCounter < ( INT32 )GetNumberOfLinesOfTextInBox( ghMoveBox ); iCounter++ )
	{
		// remove this region
		MSYS_RemoveRegion( &gMoveMenuRegion[ iCounter ] );
	}
}


static void MoveMenuMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for move box line regions
	INT32 iValue = -1;


	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		// highlight string
		HighLightBoxLine( ghMoveBox, iValue );
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		// unhighlight all strings in box
		UnHighLightBox( ghMoveBox );
	}
}


static void DeselectAllOtherSoldiersInList(void);
static void HandleMoveoutOfSectorMovementTroops(void);
static void SelectAllOtherSoldiersInList(void);


static void MoveMenuBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for move box line regions
	INT32 iMoveBoxLine = -1, iRegionType = -1, iListIndex = -1;
	UINT32 uiClickTime = 0;
	SOLDIERTYPE *pSoldier = NULL;


	iMoveBoxLine = MSYS_GetRegionUserData( pRegion, 0 );
	iRegionType  = MSYS_GetRegionUserData( pRegion, 1 );
	iListIndex   = MSYS_GetRegionUserData( pRegion, 2 );
	uiClickTime   = GetJA2Clock();

	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )  )
	{
		if( uiClickTime - guiDblClickTimersForMoveBoxMouseRegions[ iMoveBoxLine ] < DBL_CLICK_DELAY_FOR_MOVE_MENU  )
		{
			// dbl click, and something is selected?
			if ( IsAnythingSelectedForMoving() )
			{
				// treat like DONE
				HandleMoveoutOfSectorMovementTroops( );
				return;
			}
		}
		else
		{
			guiDblClickTimersForMoveBoxMouseRegions[ iMoveBoxLine ] = uiClickTime;

			if( iRegionType == SQUAD_REGION )
			{
				// is the squad moving
				if (fSquadIsMoving[iListIndex])
				{
					// squad stays
					DeselectSquadForMovement( iSquadMovingList[ iListIndex ] );
				}
				else
				{
					// squad goes
					SelectSquadForMovement( iSquadMovingList[ iListIndex ] );
				}
			}
			else if( iRegionType == VEHICLE_REGION )
			{
				// is the vehicle moving
				if (fVehicleIsMoving[iListIndex])
				{
					// vehicle stays
					DeselectVehicleForMovement( iVehicleMovingList[ iListIndex ] );
				}
				else
				{
					// vehicle goes
					SelectVehicleForMovement( iVehicleMovingList[ iListIndex ], AND_ALL_ON_BOARD );
				}
			}
			else if( iRegionType == OTHER_REGION )
			{
				if (AllOtherSoldiersInListAreSelected())
				{
					// deselect all others in the list
					DeselectAllOtherSoldiersInList( );
				}
				else
				{
					// select all others in the list
					SelectAllOtherSoldiersInList( );
				}
			}
			else if( iRegionType == SOLDIER_REGION )
			{
				pSoldier = pSoldierMovingList[ iListIndex ];

				if ( pSoldier->fBetweenSectors )
				{
					// we don't allow mercs to change squads or get out of vehicles between sectors, easiest way to handle this
					// is to prevent any toggling of individual soldiers on the move at the outset.
					DoScreenIndependantMessageBox( pMapErrorString[ 41 ], MSG_BOX_FLAG_OK, NULL );
					return;
				}

				// if soldier is currently selected to move
				if (IsSoldierSelectedForMovement(*pSoldier))
				{
					// change him to NOT move instead

					if( pSoldier->bAssignment == VEHICLE )
					{
						// if he's the only one left moving in the vehicle, deselect whole vehicle
						if( HowManyMovingSoldiersInVehicle( pSoldier->iVehicleId ) == 1 )
						{
							// whole vehicle stays
							DeselectVehicleForMovement( pSoldier->iVehicleId );
						}
						else
						{
							// soldier is staying behind
							DeselectSoldierForMovement(*pSoldier);
						}
					}
					else if( pSoldier->bAssignment < ON_DUTY )
					{
						// if he's the only one left moving in the squad, deselect whole squad
						if( HowManyMovingSoldiersInSquad( pSoldier->bAssignment ) == 1 )
						{
							// whole squad stays
							DeselectSquadForMovement( pSoldier->bAssignment );
						}
						else
						{
							// soldier is staying behind
							DeselectSoldierForMovement(*pSoldier);
						}
					}
					else
					{
						// soldier is staying behind
						DeselectSoldierForMovement(*pSoldier);
					}
				}
				else	// currently NOT moving
				{
					// is he able & allowed to move?  (Errors with a reason are reported within)
					if ( CanMoveBoxSoldierMoveStrategically( pSoldier, TRUE ) )
					{
						// change him to move instead
						SelectSoldierForMovement(*pSoldier);

						if( pSoldier->bAssignment < ON_DUTY )
						{
							// if everyone in the squad is now selected, select the squad itself
							if( AllSoldiersInSquadSelected( pSoldier->bAssignment ) )
							{
								SelectSquadForMovement( pSoldier->bAssignment );
							}
						}
/* ARM: it's more flexible without this - player can take the vehicle along or not without having to exit it.
						else if( pSoldier->bAssignment == VEHICLE )
						{
							// his vehicle MUST also go while he's moving, but not necessarily others on board
							SelectVehicleForMovement( pSoldier->iVehicleId, VEHICLE_ONLY );
						}
*/
					}
				}
			}
			else if( iRegionType == DONE_REGION )
			{
				// is something selected?
				if ( IsAnythingSelectedForMoving() )
				{
					HandleMoveoutOfSectorMovementTroops( );
					return;
				}
			}
			else if( iRegionType == CANCEL_REGION )
			{
				fShowMapScreenMovementList = FALSE;
				return;
			}
			else
			{
				SLOGA("MoveMenuBtnCallback: Invalid regionType %d, moveBoxLine %d", iRegionType, iMoveBoxLine);
				return;
			}

			fRebuildMoveBox = TRUE;
			fTeamPanelDirty = TRUE;
			fMapPanelDirty = TRUE;
			fCharacterInfoPanelDirty = TRUE;
			MarkAllBoxesAsAltered( );
		}
	}
}


static MoveError CanCharacterMoveInStrategic(SOLDIERTYPE&);


static BOOLEAN CanMoveBoxSoldierMoveStrategically(SOLDIERTYPE* pSoldier, BOOLEAN fShowErrorMessage)
{
	// valid soldier?
	Assert( pSoldier );
	Assert( pSoldier->bActive );

	MoveError const ret = CanCharacterMoveInStrategic(*pSoldier);
	if (ret == ME_OK) return TRUE;

	if (fShowErrorMessage) ReportMapScreenMovementError(ret);
	return FALSE;
}


static void SelectAllOtherSoldiersInList(void)
{
	INT32 iCounter = 0;
	BOOLEAN fSomeCantMove = FALSE;


	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( ( pSoldierMovingList[ iCounter ] ->bAssignment >= ON_DUTY ) && ( pSoldierMovingList[ iCounter ] ->bAssignment != VEHICLE ) )
		{
			if ( CanMoveBoxSoldierMoveStrategically( pSoldierMovingList[ iCounter ], FALSE ) )
			{
				fSoldierIsMoving[ iCounter ] = true;
			}
			else
			{
				fSomeCantMove = TRUE;
			}
		}
	}

	if ( fSomeCantMove )
	{
		// can't - some of the OTHER soldiers can't move
		ReportMapScreenMovementError( 46 );
	}
}


static void DeselectAllOtherSoldiersInList(void)
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( ( pSoldierMovingList[ iCounter ] ->bAssignment >= ON_DUTY ) && ( pSoldierMovingList[ iCounter ] ->bAssignment != VEHICLE ) )
		{
			fSoldierIsMoving[ iCounter ] = false;
		}
	}
}


static INT8 FindSquadThatSoldierCanJoin(SOLDIERTYPE* pSoldier);
static void HandleSettingTheSelectedListOfMercs(void);


static void HandleMoveoutOfSectorMovementTroops(void)
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = 0;
	INT32 iSquadNumber = -1;
	BOOLEAN fCheckForCompatibleSquad = FALSE;


	// cancel move box
	fShowMapScreenMovementList = FALSE;


	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		pSoldier = pSoldierMovingList[ iCounter ];

		fCheckForCompatibleSquad = FALSE;

		// if he is on a valid squad
		if( pSoldier->bAssignment < ON_DUTY )
		{
			// if he and his squad are parting ways (soldier is staying behind, but squad is leaving, or vice versa)
			if( fSoldierIsMoving[ iCounter ] != IsSquadSelectedForMovement( pSoldier->bAssignment ) )
			{
				// split the guy from his squad to any other compatible squad
				fCheckForCompatibleSquad = TRUE;
			}
		}
		// if in a vehicle
		else if( pSoldier->bAssignment == VEHICLE )
		{
			// if he and his vehicle are parting ways (soldier is staying behind, but vehicle is leaving, or vice versa)
			if( fSoldierIsMoving[ iCounter ] != IsVehicleSelectedForMovement( pSoldier->iVehicleId ) )
			{
				// split the guy from his vehicle to any other compatible squad
				fCheckForCompatibleSquad = TRUE;
			}
		}
		else	// on his own - not on a squad or in a vehicle
		{
			// if he's going anywhere
			if( fSoldierIsMoving[ iCounter ] )
			{
				// find out if anyone is going with this guy...see if he can tag along
				fCheckForCompatibleSquad = TRUE;
			}
		}


		if ( fCheckForCompatibleSquad )
		{
			// look for a squad that's doing the same thing as this guy is and has room for him
			iSquadNumber = FindSquadThatSoldierCanJoin( pSoldier );
			if( iSquadNumber != -1 )
			{
				if ( !AddCharacterToSquad( pSoldier, ( INT8 )( iSquadNumber ) ) )
				{
					SLOGA("HandleMoveoutOfSectorMovementTroops: AddCharacterToSquad %d failed, iCounter %d", iSquadNumber, iCounter);
					// toggle whether he's going or not to try and recover somewhat gracefully
					fSoldierIsMoving[ iCounter ] = !fSoldierIsMoving[ iCounter ];
				}
			}
			else
			{
				// no existing squad is compatible, will have to start his own new squad
				iSquadNumber = AddCharacterToUniqueSquad( pSoldier );

				// It worked.  Add his new squad to the "moving squads" list so others can join it, too!
				AddSquadToMovingLists( iSquadNumber );

				// If this guy is moving
				if( fSoldierIsMoving[ iCounter ] )
				{
					// mark this new squad as moving too, so those moving can join it
					SelectSquadForMovement( iSquadNumber );
				}
			}
		}

	}

	// now actually set the list
	HandleSettingTheSelectedListOfMercs( );
}


static void HandleSettingTheSelectedListOfMercs(void)
{
	BOOLEAN fFirstOne = TRUE;
	INT32 iCounter = 0;
	BOOLEAN fSelected;

	// reset the selected character
	bSelectedDestChar = -1;

	// run through the list of grunts
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const pSoldier = gCharactersList[iCounter].merc;
		if (pSoldier == NULL) continue;

		if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
		{
			fSelected = IsVehicleSelectedForMovement( pSoldier->bVehicleID );
		}
		else
		{
			fSelected = IsSoldierSelectedForMovement(*pSoldier);
		}

		// is he/she selected for movement?
		if( fSelected )
		{
			// yes, are they the first one to be selected?
			if (fFirstOne)
			{
				// yes, then set them as the destination plotting character for movement arrow purposes
				fFirstOne = FALSE;

				bSelectedDestChar = ( INT8 )iCounter;
				// make DEST column glow
				giDestHighLine = iCounter;

				ChangeSelectedInfoChar( ( INT8 ) iCounter, TRUE );
			}

			// add this guy to the selected list of grunts
			SetEntryInSelectedCharacterList( ( INT8 )iCounter );
		}
	}

	if( bSelectedDestChar != -1 )
	{
		// set cursor
		SetUpCursorForStrategicMap( );
		fTeamPanelDirty = TRUE;
		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		DeselectSelectedListMercsWhoCantMoveWithThisGuy(gCharactersList[bSelectedDestChar].merc);

		// remember the current paths for all selected characters so we can restore them if need be
		RememberPreviousPathForAllSelectedChars();
	}
}


static BOOLEAN AllOtherSoldiersInListAreSelected(void)
{
	INT32 iCounter = 0, iCount = 0;

	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( ( pSoldierMovingList[ iCounter ]->bAssignment >= ON_DUTY ) && (  pSoldierMovingList[ iCounter ]->bAssignment >= VEHICLE ) )
		{
			if (!fSoldierIsMoving[iCounter])
			{
				return( FALSE );
			}

			iCount++;
		}
	}

	// some merc on other assignments and no result?
	if( iCount )
	{
		return( TRUE );
	}

	return( FALSE );
}


static BOOLEAN IsThisSquadInThisSector(const INT16 sSectorX, const INT16 sSectorY, const INT8 bSectorZ, const INT8 bSquadValue)
{
	INT16 sX;
	INT16 sY;
	INT8  bZ;
	return
		SectorSquadIsIn(bSquadValue, &sX, &sY, &bZ) &&
		sSectorX == sX                              &&
		sSectorY == sY                              &&
		bSectorZ == bZ                              &&
		!IsThisSquadOnTheMove(bSquadValue);
}


static INT8 FindSquadThatSoldierCanJoin(SOLDIERTYPE* pSoldier)
{
	// look for a squad that isn't full that can take this character
	INT8 bCounter = 0;

	// run through the list of squads
	for( bCounter = 0; bCounter < NUMBER_OF_SQUADS; bCounter++ )
	{
		// is this squad in this sector
		if( IsThisSquadInThisSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ, bCounter ) )
		{
			// does it have room?
			if (!IsThisSquadFull(bCounter))
			{
				// is it doing the same thing as the soldier is (staying or going) ?
				if (IsSquadSelectedForMovement(bCounter) == IsSoldierSelectedForMovement(*pSoldier))
				{
					// go ourselves a match, then
					return( bCounter );
				}
			}
		}
	}

	return( -1 );
}

void ReBuildMoveBox( void )
{
	// check to see if we need to rebuild the movement box and mouse regions
	if (!fRebuildMoveBox) return;

	// reset the fact
	fRebuildMoveBox = FALSE;
	fTeamPanelDirty = TRUE;
	fMapPanelDirty = TRUE;
	fCharacterInfoPanelDirty = TRUE;

	// stop showing the box
	fShowMapScreenMovementList = FALSE;
	CreateDestroyMovementBox( sSelMapX, sSelMapY, ( INT16 )iCurrentMapSectorZ );

	// show the box
	fShowMapScreenMovementList = TRUE;
	CreateDestroyMovementBox( sSelMapX, sSelMapY, ( INT16 )iCurrentMapSectorZ );
	ShowBox( ghMoveBox );
	MarkAllBoxesAsAltered( );
}


static void MoveScreenMaskBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


void CreateScreenMaskForMoveBox( void )
{
	if (!fScreenMaskForMoveCreated)
	{
		// set up the screen mask
		MSYS_DefineRegion(&gMoveBoxScreenMask, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 4, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MoveScreenMaskBtnCallback);

		fScreenMaskForMoveCreated = TRUE;
	}
}

void RemoveScreenMaskForMoveBox( void )
{
	if (fScreenMaskForMoveCreated)
	{
		// remove the screen mask
		MSYS_RemoveRegion( &gMoveBoxScreenMask );
		fScreenMaskForMoveCreated = FALSE;
	}
}


static void MoveScreenMaskBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for move box screen mask region
	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )  )
	{
		fShowMapScreenMovementList = FALSE;
	}
	else if( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		sSelectedMilitiaTown = 0;

		// are we showing the update box
		if( fShowUpdateBox )
		{
			fShowUpdateBox = FALSE;
		}
	}
}


static void ResetSoldierUpdateBox(void)
{
	INT32 iCounter = 0;

	// delete any loaded faces
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		if( pUpdateSoldierBox[ iCounter ] != NULL )
		{
			DeleteVideoObject(giUpdateSoldierFaces[iCounter]);
		}
	}

	if( giMercPanelImage != 0 )
	{
		DeleteVideoObject(giMercPanelImage);
	}

	// reset the soldier ptrs in the update box
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		pUpdateSoldierBox[ iCounter ] = NULL;
	}
}


INT32 GetNumberOfMercsInUpdateList()
{
	INT32 n = 0;
	FOR_EACH(SOLDIERTYPE* const, i, pUpdateSoldierBox)
	{
		if (*i) ++n;
	}
	return n;
}


static void AddSoldierToUpdateBox(SOLDIERTYPE*);


void AddSoldierToWaitingListQueue(SOLDIERTYPE& s)
{
	class DialogueEventUpdateBoxAddSoldier : public CharacterDialogueEvent
	{
		public:
			DialogueEventUpdateBoxAddSoldier(SOLDIERTYPE& s) : CharacterDialogueEvent(s) {}

			bool Execute()
			{
				AddSoldierToUpdateBox(&soldier_);
				return false;
			}
	};

	DialogueEvent::Add(new DialogueEventUpdateBoxAddSoldier(s));
}


void AddReasonToWaitingListQueue(UpdateBoxReason const reason)
{
	class DialogueEventUpdateBoxSetReason : public DialogueEvent
	{
		public:
			DialogueEventUpdateBoxSetReason(UpdateBoxReason const reason) : reason_(reason) {}

			bool Execute()
			{
				iReasonForSoldierUpDate = reason_;
				return false;
			}

		private:
			UpdateBoxReason const reason_;
	};

	DialogueEvent::Add(new DialogueEventUpdateBoxSetReason(reason));
}


void AddDisplayBoxToWaitingQueue(void)
{
	class DialogueEventUpdateBoxShow : public DialogueEvent
	{
		public:
			bool Execute()
			{
				fShowUpdateBox = TRUE;
				return false;
			}
	};

	DialogueEvent::Add(new DialogueEventUpdateBoxShow());
}


static void AddSoldierToUpdateBox(SOLDIERTYPE* const pSoldier)
{
	INT32 iCounter = 0;

	// going to load face

	if( pSoldier->bLife == 0 )
	{
		return;
	}

	if (!pSoldier->bActive) return;

	// if update
	if( pUpdateSoldierBox[ iCounter ] == NULL )
	{
		giMercPanelImage = AddVideoObjectFromFile(INTERFACEDIR "/panels.sti");
	}

	// run thought list of update soldiers
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		// find a free slot
		if( pUpdateSoldierBox[ iCounter ] == NULL )
		{
			// add to box
			pUpdateSoldierBox[ iCounter ] = pSoldier;
			giUpdateSoldierFaces[iCounter] = Load65Portrait(GetProfile(pSoldier->ubProfile));
			return;
		}
	}
}


static void CreateDestroyUpdatePanelButtons(INT32 iX, INT32 iY, BOOLEAN fFourWideMode);
static void RenderSoldierSmallFaceForUpdatePanel(INT32 iIndex, INT32 iX, INT32 iY);


void DisplaySoldierUpdateBox( )
{
	INT32 iNumberOfMercsOnUpdatePanel = 0;
	INT32 iNumberHigh = 0, iNumberWide = 0;
	INT32 iUpdatePanelWidth = 0, iUpdatePanelHeight = 0;
	INT32 iX = 0, iY = 0;
	INT32 iFaceX = 0, iFaceY = 0;
	BOOLEAN fFourWideMode = FALSE;
	INT32 iCounter = 0;
	INT32 iUpperLimit = 0;

	if (!fShowUpdateBox) return;

	// get the number of mercs
	iNumberOfMercsOnUpdatePanel = GetNumberOfMercsInUpdateList( );

	if( iNumberOfMercsOnUpdatePanel == 0 )
	{
		// nobody home
		fShowUpdateBox = FALSE;
		// unpause
		UnPauseDialogueQueue( );
		return;
	}

	giSleepHighLine = -1;
	giDestHighLine = -1;
	giContractHighLine = -1;
	giAssignHighLine = -1;


	//InterruptTime();
	PauseGame( );
	LockPauseState(LOCK_PAUSE_DISPLAY_SOLDIER_UPDATE);

	PauseDialogueQueue( );

	// do we have enough for 4 wide, or just 2 wide?
	if( iNumberOfMercsOnUpdatePanel > NUMBER_OF_MERCS_FOR_FOUR_WIDTH_UPDATE_PANEL )
	{
		fFourWideMode = TRUE;
	}

	// get number of rows
	iNumberHigh = ( fFourWideMode ?  iNumberOfMercsOnUpdatePanel / NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE
		:  iNumberOfMercsOnUpdatePanel / NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE);

	//number of columns
	iNumberWide = ( fFourWideMode ? NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE
		: NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE );



	// get the height and width of the box .. will need to add in stuff for borders, lower panel...etc
	if( fFourWideMode )
	{
		// do we need an extra row for left overs
		if( iNumberOfMercsOnUpdatePanel % NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE )
		{
			iNumberHigh++;
		}
	}
	else
	{
		// do we need an extra row for left overs
		if( iNumberOfMercsOnUpdatePanel % NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE )
		{
			iNumberHigh++;
		}
	}

		// round off
	if( fFourWideMode )
	{
		if(iNumberOfMercsOnUpdatePanel % NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE )
		{
			iNumberOfMercsOnUpdatePanel += NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE - ( iNumberOfMercsOnUpdatePanel % NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE );
		}
	}
	else
	{
		if(iNumberOfMercsOnUpdatePanel % NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE )
		{
			iNumberOfMercsOnUpdatePanel += NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE - ( iNumberOfMercsOnUpdatePanel % NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE );
		}
	}

	iUpdatePanelWidth = iNumberWide * TACT_WIDTH_OF_UPDATE_PANEL_BLOCKS;

	iUpdatePanelHeight = ( iNumberHigh + 1 ) * TACT_HEIGHT_OF_UPDATE_PANEL_BLOCKS;

	// get the x,y offsets on the screen of the panel
	iX = STD_SCREEN_X + 290 + ( 336 - iUpdatePanelWidth ) / 2;

//	iY = 28 + ( 288 - iUpdatePanelHeight ) / 2;

	// Have the bottom of the box ALWAYS a set distance from the bottom of the map ( so user doesnt have to move mouse far )
	iY = STD_SCREEN_Y + 280 - iUpdatePanelHeight;

	const SGPVObject* const hBackGroundHandle = guiUpdatePanelTactical;

	//Display the 2 TOP corner pieces
	BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 0, iX-4, iY - 4);
	BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 2, iX+iUpdatePanelWidth, iY - 4);

	if( fFourWideMode )
	{
		//Display 2 vertical lines starting at the bottom
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 3, iX - 4, iY + iUpdatePanelHeight - 3 - 70);
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 5, iX + iUpdatePanelWidth, iY + iUpdatePanelHeight - 3 - 70);

		//Display the 2 bottom corner pieces
		BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 0, iX-4, iY + iUpdatePanelHeight - 3);
		BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 2, iX+iUpdatePanelWidth, iY + iUpdatePanelHeight - 3);
	}


	SetFontDestBuffer(guiSAVEBUFFER);

	iUpperLimit = fFourWideMode ? ( iNumberOfMercsOnUpdatePanel + NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE ) : ( iNumberOfMercsOnUpdatePanel + NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE );

	//need to put the background down first
	for( iCounter = 0; iCounter < iUpperLimit; iCounter++ )
	{
		// blt the face and name

		// get the face x and y
		iFaceX = iX + ( iCounter % iNumberWide ) * TACT_UPDATE_MERC_FACE_X_WIDTH;
		iFaceY = iY + ( iCounter / iNumberWide ) * TACT_UPDATE_MERC_FACE_X_HEIGHT;

		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 20, iFaceX, iFaceY);
	}

	//loop through the mercs to be displayed
	for( iCounter = 0; iCounter < ( iNumberOfMercsOnUpdatePanel <= NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE ? NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE : iNumberOfMercsOnUpdatePanel ); iCounter++ )
	{
		//
		// blt the face and name
		//

		// get the face x and y
		iFaceX = iX + ( iCounter % iNumberWide ) * TACT_UPDATE_MERC_FACE_X_WIDTH;
		iFaceY = iY + ( iCounter / iNumberWide ) * TACT_UPDATE_MERC_FACE_X_HEIGHT +  REASON_FOR_SOLDIER_UPDATE_OFFSET_Y;

		// now get the face
		if( pUpdateSoldierBox[ iCounter ] )
		{
			iFaceX += TACT_UPDATE_MERC_FACE_X_OFFSET;
			iFaceY += TACT_UPDATE_MERC_FACE_Y_OFFSET;

			// there is a face
			RenderSoldierSmallFaceForUpdatePanel( iCounter, iFaceX, iFaceY );

			// display the mercs name
			DrawTextToScreen(pUpdateSoldierBox[iCounter]->name, iFaceX - 5, iFaceY + 31, 57, TINYFONT1, FONT_LTRED, FONT_BLACK, CENTER_JUSTIFIED);
		}
	}

	// the button container box
	if( fFourWideMode )
	{
		//def: 3/1/99 WAS SUBINDEX 6,
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 19, iX - 4 + TACT_UPDATE_MERC_FACE_X_WIDTH,  iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y+3);

		// ATE: Display string for time compression
		DisplayWrappedString(iX, iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + 5 + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y + 3, iUpdatePanelWidth, 0, MAP_SCREEN_FONT, FONT_WHITE, gzLateLocalizedString[STR_LATE_49], FONT_BLACK, CENTER_JUSTIFIED);
	}
	else
	{
		//def: 3/1/99 WAS SUBINDEX 6,
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 19, iX - 4 , iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y+3);

		// ATE: Display string for time compression
		DisplayWrappedString(iX, iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + 5 + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y + 3, iUpdatePanelWidth, 0, MAP_SCREEN_FONT, FONT_WHITE, gzLateLocalizedString[STR_LATE_49], FONT_BLACK, CENTER_JUSTIFIED);
	}

	// now wrap the border
	for( iCounter = 0; iCounter < iNumberHigh ; iCounter++ )
	{
		// the sides
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 3, iX - 4, iY + ( iCounter ) * TACT_UPDATE_MERC_FACE_X_HEIGHT);
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 5, iX + iUpdatePanelWidth, iY + ( iCounter ) * TACT_UPDATE_MERC_FACE_X_HEIGHT);
	}

	//big horizontal line
	for( iCounter = 0; iCounter < iNumberWide; iCounter++ )
	{
		// the top bottom
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 1, iX + TACT_UPDATE_MERC_FACE_X_WIDTH * (iCounter ) , iY - 4);
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 1, iX + TACT_UPDATE_MERC_FACE_X_WIDTH * (iCounter ) , iY + iUpdatePanelHeight - 3);
	}

	//Display the reason for the update box
	DisplayWrappedString(iX, iY + (fFourWideMode ? 6 : 3), iUpdatePanelWidth, 0, MAP_SCREEN_FONT, FONT_WHITE, pUpdateMercStrings[iReasonForSoldierUpDate], FONT_BLACK, CENTER_JUSTIFIED);

	SetFontDestBuffer(FRAME_BUFFER);

	// restore extern background rect
	RestoreExternBackgroundRect( ( INT16 )( iX - 5 ), ( INT16 )( iY - 5 ), ( INT16 )(  iUpdatePanelWidth + 10 ), ( INT16 )(iUpdatePanelHeight + 6 ) );

	CreateDestroyUpdatePanelButtons( iX, ( iY + iUpdatePanelHeight - 18 ), fFourWideMode );
	MarkAButtonDirty( guiUpdatePanelButtons[ 0 ] );
	MarkAButtonDirty( guiUpdatePanelButtons[ 1 ] );
}


static void MakeButton(UINT idx, INT16 x, INT16 y, GUI_CALLBACK click, const ST::string& text, const ST::string& help_text)
{
	GUIButtonRef const btn = QuickCreateButtonImg(INTERFACEDIR "/group_confirm_tactical.sti", 7, 8, x, y, MSYS_PRIORITY_HIGHEST - 1, click);
	guiUpdatePanelButtons[idx] = btn;
	btn->SpecifyGeneralTextAttributes(text, MAP_SCREEN_FONT, FONT_MCOLOR_BLACK, FONT_BLACK);
	btn->SetFastHelpText(help_text);
}


static void ContinueUpdateButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void StopUpdateButtonCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateDestroyUpdatePanelButtons(INT32 iX, INT32 iY, BOOLEAN fFourWideMode)
{
	static BOOLEAN fCreated = FALSE;

	if (fShowUpdateBox && !fCreated)
	{
		// set to created
		fCreated = TRUE;

		fShowAssignmentMenu = FALSE;
		fShowContractMenu = FALSE;

		INT16 x = iX;
		if (fFourWideMode) x += TACT_UPDATE_MERC_FACE_X_WIDTH;
		MakeButton(0, x,                                 iY, ContinueUpdateButtonCallback, pUpdatePanelButtons[0], gzLateLocalizedString[STR_LATE_51]);
		MakeButton(1, x + TACT_UPDATE_MERC_FACE_X_WIDTH, iY, StopUpdateButtonCallback,     pUpdatePanelButtons[1], gzLateLocalizedString[STR_LATE_52]);
	}
	else if (!fShowUpdateBox && fCreated)
	{
		// set to uncreated
		fCreated = FALSE;

		// get rid of the buttons and images
		RemoveButton( guiUpdatePanelButtons[ 0 ] );
		RemoveButton( guiUpdatePanelButtons[ 1 ] );

		// unpause
		UnPauseDialogueQueue( );
	}
}


void CreateDestroyTheUpdateBox( void )
{
	static BOOLEAN fCreated = FALSE;

	if (!fCreated && fShowUpdateBox)
	{
		if( GetNumberOfMercsInUpdateList( ) == 0 )
		{
			fShowUpdateBox = FALSE;
			return;
		}

		fCreated = TRUE;

		//InterruptTime();
		// create screen mask
		CreateScreenMaskForMoveBox( );

		// lock it paused
		PauseGame();
		LockPauseState(LOCK_PAUSE_CREATE_SOLDIER_UPDATE);

		// display the box
		DisplaySoldierUpdateBox( );

		// Do beep
		PlayJA2SampleFromFile(SOUNDSDIR "/newbeep.wav", MIDVOLUME, 1, MIDDLEPAN);
	}
	else if (fCreated && !fShowUpdateBox)
	{
		fCreated = FALSE;

		UnLockPauseState( );
		UnPauseGame();

		// dirty screen
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		// remove screen mask
		RemoveScreenMaskForMoveBox( );

		ResetSoldierUpdateBox( );

		CreateDestroyUpdatePanelButtons( 0, 0, FALSE );
	}
}


static void RenderSoldierSmallFaceForUpdatePanel(INT32 iIndex, INT32 iX, INT32 iY)
{
	INT32 iStartY = 0;
	SOLDIERTYPE *pSoldier = NULL;

	// fill the background for the info bars black
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+36, iY+2, iX+44,	iY+30, 0 );

	// put down the background
	BltVideoObject(guiSAVEBUFFER, giMercPanelImage, 0, iX, iY);

	// grab the face
	BltVideoObject(guiSAVEBUFFER, giUpdateSoldierFaces[iIndex], 0, iX + 2, iY + 2);

	//HEALTH BAR
	pSoldier = pUpdateSoldierBox[ iIndex ];

	// is the merc alive?
	if( !pSoldier->bLife )
		return;


	//yellow one for bleeding
	iStartY = iY + 29 - 27*pSoldier->bLifeMax/100;
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+36, iStartY, iX+37, iY+29, Get16BPPColor( FROMRGB( 107, 107, 57 ) ) );
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+37, iStartY, iX+38, iY+29, Get16BPPColor( FROMRGB( 222, 181, 115 ) ) );

	//pink one for bandaged.
	iStartY += 27*pSoldier->bBleeding/100;
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+36, iStartY, iX+37, iY+29, Get16BPPColor( FROMRGB( 156, 57, 57 ) ) );
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+37, iStartY, iX+38, iY+29, Get16BPPColor( FROMRGB( 222, 132, 132 ) ) );

	//red one for actual health
	iStartY = iY + 29 - 27*pSoldier->bLife/100;
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+36, iStartY, iX+37, iY+29, Get16BPPColor( FROMRGB( 107, 8, 8 ) ) );
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+37, iStartY, iX+38, iY+29, Get16BPPColor( FROMRGB( 206, 0, 0 ) ) );

	//BREATH BAR
	iStartY = iY + 29 - 27*pSoldier->bBreathMax/100;
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+39, iStartY, iX+40, iY+29, Get16BPPColor( FROMRGB( 8, 8, 132 ) ) );
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+40, iStartY, iX+41, iY+29, Get16BPPColor( FROMRGB( 8, 8, 107 ) ) );

	//MORALE BAR
	iStartY = iY + 29 - 27*pSoldier->bMorale/100;
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+42, iStartY, iX+43, iY+29, Get16BPPColor( FROMRGB( 8, 156, 8 ) ) );
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+43, iStartY, iX+44, iY+29, Get16BPPColor( FROMRGB( 8, 107, 8 ) ) );
}


static void ContinueUpdateButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		EndUpdateBox(TRUE); // restart time compression
	}
}


static void StopUpdateButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		EndUpdateBox(FALSE); // stop time compression
	}
}


void EndUpdateBox( BOOLEAN fContinueTimeCompression )
{
	fShowUpdateBox = FALSE;

	CreateDestroyTheUpdateBox();

	if ( fContinueTimeCompression )
	{
		StartTimeCompression();
	}
	else
	{
		StopTimeCompression();
	}
}

void InitTimersForMoveMenuMouseRegions()
{
	FOR_EACH(UINT32, i, guiDblClickTimersForMoveBoxMouseRegions) *i = 0;
}


void UpdateHelpTextForMapScreenMercIcons()
{
	const SOLDIERTYPE* const s = GetSelectedInfoChar();

	// if merc is an AIM merc
	ST::string contract = s && s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ?
		zMarksMapScreenText[21] : ST::null;
	gContractIconRegion.SetFastHelpText(contract);

	// if merc has life insurance
	ST::string insurance = s && s->usLifeInsurance > 0 ?
		zMarksMapScreenText[3] : ST::null;
	gInsuranceIconRegion.SetFastHelpText(insurance);

	// if merc has a medical deposit
	ST::string medical = s && s->usMedicalDeposit > 0 ?
		zMarksMapScreenText[12] : ST::null;
	gDepositIconRegion.SetFastHelpText(medical);
}


void CreateDestroyInsuranceMouseRegionForMercs( BOOLEAN fCreate )
{
	static BOOLEAN fCreated = FALSE;

	if (!fCreated && fCreate)
	{
		MSYS_DefineRegion( &gContractIconRegion, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y, CHAR_ICON_X + CHAR_ICON_WIDTH, CHAR_ICON_CONTRACT_Y + CHAR_ICON_HEIGHT,
					MSYS_PRIORITY_HIGH - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		MSYS_DefineRegion( &gInsuranceIconRegion, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + CHAR_ICON_SPACING, CHAR_ICON_X + CHAR_ICON_WIDTH, CHAR_ICON_CONTRACT_Y + CHAR_ICON_SPACING + CHAR_ICON_HEIGHT,
					MSYS_PRIORITY_HIGH - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		MSYS_DefineRegion( &gDepositIconRegion, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + ( 2 * CHAR_ICON_SPACING ), CHAR_ICON_X + CHAR_ICON_WIDTH, CHAR_ICON_CONTRACT_Y + ( 2 * CHAR_ICON_SPACING ) + CHAR_ICON_HEIGHT,
					MSYS_PRIORITY_HIGH - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		fCreated = TRUE;
	}
	else if (fCreated && !fCreate)
	{
		MSYS_RemoveRegion( &gContractIconRegion );
		MSYS_RemoveRegion( &gInsuranceIconRegion );
		MSYS_RemoveRegion( &gDepositIconRegion );
		fCreated = FALSE;
	}
}


BOOLEAN HandleTimeCompressWithTeamJackedInAndGearedToGo( void )
{
	// check a team is ready to go
	if( !( AnyMercsHired( ) ) )
	{
		// no mercs, leave
		return( FALSE );
	}

	// make sure the game just started
	if (!DidGameJustStart()) return FALSE;

	static const SGPSector usStartSector(gamepolicy(start_sector));

	// Select starting sector.
	ChangeSelectedMapSector(usStartSector);

	// load starting sector
	try
	{
		SetCurrentWorldSector(usStartSector);
	}
	catch (...) /* XXX exception should probably propagate; caller ignores return value */
	{
		return FALSE;
	}

	if (NumEnemiesInSector(usStartSector) > 0)
	{
		//Setup variables in the PBI for this first battle.  We need to support the
		//non-persistant PBI in case the user goes to mapscreen.
		gfBlitBattleSectorLocator = TRUE;
		gubPBSector = SGPSector(gamepolicy(start_sector));

		gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;
	}

	InitHelicopterEntranceByMercs( );

	FadeInGameScreen( );

	SetUpShutDownMapScreenHelpTextScreenMask( );

	// Add e-mail message
	AddEmail(ENRICO_CONGRATS,ENRICO_CONGRATS_LENGTH,MAIL_ENRICO, GetWorldTotalMin() );


	return( TRUE );
}


void NotifyPlayerWhenEnemyTakesControlOfImportantSector(const SGPSector& sec)
{
	// There is nothing important to player below ground
	if (sec.z != 0) return;

	ST::string sector_desc = GetSectorIDString(sec, TRUE);

	ST::string buf;
	if (IsThisSectorASAMSector(sec))
	{
		buf = st_format_printf(pMapErrorString[15], sector_desc);
	}
	else
	{
		UINT8 const sector  = sec.AsByte();
		INT8  const mine_id = GetMineIndexForSector(sector);
		if (mine_id != -1                           &&
				GetMaxDailyRemovalFromMine(mine_id) > 0 &&
				SpokenToHeadMiner(mine_id))
		{ // There is a mine and it was producing for us
			// Get how much we now will get from the mines
			INT32 const income = GetProjectedTotalDailyIncome();
			ST::string income_string = SPrintMoney(income);
			buf = st_format_printf(pMapErrorString[16], sector_desc, income_string);
		}
		else
		{
			INT8 const town_id = GetTownIdForSector(sector);
			if (town_id == BLANK_SECTOR) return;
			// San Mona isn't important
			if (town_id == SAN_MONA) return;

			buf = st_format_printf(pMapErrorString[25], sector_desc);
		}
	}
	DoScreenIndependantMessageBox(buf, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
}


void NotifyPlayerOfInvasionByEnemyForces(const SGPSector& sector, MSGBOX_CALLBACK const return_callback)
{
	if (sector.z != 0) return;

	// If enemy controlled anyways, leave
	if (StrategicMap[sector.AsStrategicIndex()].fEnemyControlled) return;

	ST::string buf;
	ST::string sector_desc;

	// check if SAM site here
	if (IsThisSectorASAMSector(sector))
	{
		sector_desc = sector.AsShortString();
		buf = st_format_printf(pMapErrorString[22], sector_desc);
		DoScreenIndependantMessageBox(buf, MSG_BOX_FLAG_OK, return_callback);
	}
	else if (GetTownIdForSector(sector.AsByte()) != BLANK_SECTOR)
	{
		sector_desc = GetSectorIDString(sector, TRUE);
		buf = st_format_printf(pMapErrorString[23], sector_desc);
		DoScreenIndependantMessageBox(buf, MSG_BOX_FLAG_OK, return_callback);
	}
	else
	{
		sector_desc = sector.AsShortString();
		buf = st_format_printf(pMapErrorString[24], sector_desc);
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, buf);
	}
}


static MoveError CanCharacterMoveInStrategic(SOLDIERTYPE& s)
{
	Assert(s.bActive);

	/* NOTE: Check for the most permanent conditions first, and the most easily
	 * remedied ones last. In case several cases apply, only the reason found
	 * first will be given, so make it a good one. */

	// Still in transit?
	if (IsCharacterInTransit(s)) return ME_TRANSIT;
	// A POW?
	if (s.bAssignment == ASSIGNMENT_POW) return ME_POW;
	// Underground? (can't move strategically, must use tactical traversal)
	if (s.bSectorZ != 0) return ME_UNDERGROUND;

	// Vehicle checks
	if (s.uiStatusFlags & SOLDIER_VEHICLE)
	{
		VEHICLETYPE const& v = GetVehicle(s.bVehicleID);
		// Empty (needs a driver!)?
		if (GetNumberInVehicle(v) == 0) return ME_VEHICLE_EMPTY;
		// Too damaged?
		if (s.bLife < OKLIFE) return ME_VEHICLE_DAMAGED;
		// Out of fuel?
		if (s.sBreathRed == 0) return ME_VEHICLE_NO_GAS;
	}
	else // Non-vehicle
	{
		// Dead?
		if (s.bLife <= 0)
		{
			gsCustomErrorString = st_format_printf(pMapErrorString[35], s.name);
			return ME_CUSTOM;
		}

		// Too injured?
		if (s.bLife < OKLIFE)
		{
			gsCustomErrorString = st_format_printf(pMapErrorString[33], s.name);
			return ME_CUSTOM;
		}
	}

	/* If merc is in a particular sector, not somewhere in between, and he's NOT
	 * flying above it all in a working helicopter */
	if (!s.fBetweenSectors && !SoldierAboardAirborneHeli(s))
	{
		// And that sector is loaded
		if (s.sSectorX == gWorldSector.x &&
				s.sSectorY == gWorldSector.y &&
				s.bSectorZ == gWorldSector.z)
		{
			// In combat?
			if (gTacticalStatus.uiFlags & INCOMBAT) return ME_COMBAT;
			// Hostile sector?
			if (gTacticalStatus.fEnemyInSector) return ME_ENEMY;
			// Bloodcat ambush?
			if (gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE && HostileBloodcatsPresent()) return ME_COMBAT;
		}

		// Not necessarily loaded - if there are any hostiles there
		if (NumHostilesInSector(SGPSector(s.sSectorX, s.sSectorY, s.bSectorZ)) > 0) return ME_ENEMY;
	}

	// If in L12 museum, and the museum alarm went off, and Eldin still around
	if (s.sSectorX == 12        &&
			s.sSectorY == MAP_ROW_L &&
			s.bSectorZ == 0         &&
			!s.fBetweenSectors && GetProfile(ELDIN).bMercStatus != MERC_IS_DEAD)
	{
		UINT8	const room = GetRoom(s.sGridNo);
		if (22 <= room && room <= 41)
		{
			CFOR_EACH_IN_TEAM(s, OUR_TEAM)
			{
				if (FindObj(s, CHALICE) != ITEM_NOT_FOUND) return ME_MUSEUM;
			}
		}
	}

	// On assignment, other than just in a VEHICLE?
	if (s.bAssignment >= ON_DUTY && s.bAssignment != VEHICLE) return ME_BUSY;

	/* If he's walking/driving, and so tired that he would just stop the group
	 * anyway in the next sector, or already asleep and can't be awakened */
	if (PlayerSoldierTooTiredToTravel(s))
	{
		gsCustomErrorString = st_format_printf(pMapErrorString[43], s.name);
		return ME_CUSTOM;
	}

	if (AM_A_ROBOT(&s))
	{ // Going alone?
		if ((s.bAssignment == VEHICLE && !IsRobotControllerInVehicle(s.iVehicleId)) ||
				(s.bAssignment  < ON_DUTY && !IsRobotControllerInSquad(s.bAssignment)))
		{
			return ME_ROBOT_ALONE;
		}
	}
	else if (s.ubWhatKindOfMercAmI == MERC_TYPE__EPC) // an Escorted NPC?
	{
		// Going alone?
		if ((s.bAssignment == VEHICLE && GetNumberOfNonEPCsInVehicle(s.iVehicleId) == 0) ||
				(s.bAssignment  < ON_DUTY && NumberOfNonEPCsInSquad(s.bAssignment) == 0))
		{
			MERCPROFILESTRUCT const&       p    = GetProfile(s.ubProfile);
			ST::string text = p.bSex == MALE ? pMapErrorString[6] : pMapErrorString[7];
			gsCustomErrorString = st_format_printf(text, s.name);
			return ME_CUSTOM;
		}
	}

	// Find out if this particular character can't move for some reason
	bool problem_exists = false;
	switch (s.ubProfile)
	{
		case MARIA:
			// Maria can't move if she's in sector C5
			if (SECTOR(s.sSectorX, s.sSectorY) == SEC_C5) problem_exists = true;
			break;
	}

	if (problem_exists)
	{ // Inform user this specific merc cannot be moved out of the sector
		gsCustomErrorString = st_format_printf(pMapErrorString[29], s.name);
		return ME_CUSTOM;
	}

	// Passed all checks - this character may move strategically!
	return ME_OK;
}


MoveError CanEntireMovementGroupMercIsInMove(SOLDIERTYPE& s)
{
	// First check the requested character himself
	MoveError const ret = CanCharacterMoveInStrategic(s);
	if (ret != ME_OK) return ret; // Failed no point checking anyone else

	// Now check anybody who would be traveling with him

	/* Even if group is 0 (not that that should happen, should it?) still loop
	 * through for other mercs selected to move */
	GROUP const* const g = GetSoldierGroup(s);

	/* If anyone in the merc's group or also selected cannot move for whatever
	 * reason return an error */
	CFOR_EACH_IN_CHAR_LIST(c)
	{
		SOLDIERTYPE& other = *c->merc;
		if (&other == &s) continue; // Skip the same guy we did already

		/* Is he in the same movement group (i.e. squad), or is he still selected to
			* go with us (legal?) */
		if (GetSoldierGroup(other) != g && !c->selected) continue;

		MoveError const ret = CanCharacterMoveInStrategic(other);
		// Cannot move, fail, and don't bother checking anyone else, either
		if (ret != ME_OK) return ret;
	}

	// Everybody can move
	return ME_OK;
}


void ReportMapScreenMovementError(const INT8 bErrorNumber)
{
	// a customized message?
	ST::string text = (bErrorNumber != ME_CUSTOM ? pMapErrorString[bErrorNumber] : gsCustomErrorString);
	DoMapMessageBox(MSG_BOX_BASIC_STYLE, text, MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
}


// we are checking to see if we need to in fact rebuild the characterlist for mapscreen
void HandleRebuildingOfMapScreenCharacterList( void )
{
	// check if we need to rebuild the list?
	if( fReBuildCharacterList )
	{
		// do the actual rebuilding
		ReBuildCharactersList( );

		// reset the flag
		fReBuildCharacterList = FALSE;
	}
}


void RequestToggleTimeCompression( void )
{
	if( !IsTimeBeingCompressed( ) )
	{
		StartTimeCompression();
	}
	else	// currently compressing
	{
		StopTimeCompression();
	}
}


void RequestIncreaseInTimeCompression( void )
{
	if ( IsTimeBeingCompressed( ) )
	{
		IncreaseGameTimeCompressionRate();
	}
	else
	{
/*
		// start compressing
		StartTimeCompression();
*/
		// ARM Change: start over at 5x compression
		SetGameTimeCompressionLevel( TIME_COMPRESS_5MINS );
	}
}



void RequestDecreaseInTimeCompression( void )
{
	if ( IsTimeBeingCompressed( ) )
	{
		DecreaseGameTimeCompressionRate();
	}
	else
	{
		// check that we can
		if ( !AllowedToTimeCompress( ) )
		{
			// not allowed to compress time
			TellPlayerWhyHeCantCompressTime();
			return;
		}

		// ARM Change: do nothing
/*
		// if compression mode is set, just restart time so player can see it
		if ( giTimeCompressMode > TIME_COMPRESS_X1 )
		{
			StartTimeCompression();
		}
*/
	}
}


static BOOLEAN CanSoldierMoveWithVehicleId(const SOLDIERTYPE* const pSoldier, const INT32 iVehicle1Id)
{
	INT32 iVehicle2Id = -1;
	VEHICLETYPE *pVehicle1, *pVehicle2;


	Assert( iVehicle1Id != -1 );

	// if soldier is IN a vehicle
	if( pSoldier->bAssignment == VEHICLE )
	{
		iVehicle2Id = pSoldier->iVehicleId;
	}
	else
	// if soldier IS a vehicle
	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		iVehicle2Id = pSoldier->bVehicleID;
	}


	// if also (in) a vehicle
	if ( iVehicle2Id != -1 )
	{
		// if it's the same vehicle
		if ( iVehicle1Id == iVehicle2Id )
		{
			return( TRUE );
		}

		// helicopter can't move together with ground vehicles!
		if ( ( iVehicle1Id == iHelicopterVehicleId ) || ( iVehicle2Id == iHelicopterVehicleId ) )
		{
			return( FALSE );
		}

		pVehicle1 = &( pVehicleList[ iVehicle1Id ] );
		pVehicle2 = &( pVehicleList[ iVehicle2Id ] );

		// as long as they're in the same location, amd neither is between sectors, different vehicles is also ok
		if( ( pVehicle1->sSectorX == pVehicle2->sSectorX ) &&
			( pVehicle1->sSectorY == pVehicle2->sSectorY ) &&
			( pVehicle1->sSectorZ == pVehicle2->sSectorZ ) &&
			!pVehicle1->fBetweenSectors &&
			!pVehicle2->fBetweenSectors )
		{
			return( TRUE );
		}
	}


	// not in/is a vehicle, or in a different vehicle that isn't in the same location
	return ( FALSE );
}


void SaveLeaveItemList(HWFILE const f)
{
	for (INT32 i = 0; i < NUM_LEAVE_LIST_SLOTS; ++i)
	{
		MERC_LEAVE_ITEM const* const head = gpLeaveListHead[i];
		if (head)
		{
			// Save to specify that a node DOES exist
			BOOLEAN const node_exists = TRUE;
			f->write(&node_exists, sizeof(BOOLEAN));

			// Save number of items
			UINT32 n_items = 0;
			for (MERC_LEAVE_ITEM const* i = head; i; i = i->pNext)
			{
				++n_items;
			}
			f->write(&n_items, sizeof(UINT32));

			for (MERC_LEAVE_ITEM const* i = head; i; i = i->pNext)
			{
				BYTE  data[40];
				DataWriter d{data};
				InjectObject(d, &i->o);
				INJ_SKIP(d, 4)
				Assert(d.getConsumed() == lengthof(data));

				f->write(data, sizeof(data));
			}
		}
		else
		{
			// Save to specify that a node DOENST exist
			BOOLEAN const node_exists = FALSE;
			f->write(&node_exists, sizeof(BOOLEAN));
		}
	}

	// Save the leave list profile IDs
	for (INT32 i = 0; i < NUM_LEAVE_LIST_SLOTS; ++i)
	{
		f->write(&guiLeaveListOwnerProfileId[i], sizeof(UINT32));
	}
}


void LoadLeaveItemList(HWFILE const f)
{
	ShutDownLeaveList();
	InitLeaveList();

	for (INT32 i = 0; i < NUM_LEAVE_LIST_SLOTS; ++i)
	{
		// Load flag which specifies whether a node exists
		BOOLEAN	node_exists;
		f->read(&node_exists, sizeof(BOOLEAN));
		if (!node_exists) continue;

		// Load the number specifing how many items there are in the list
		UINT32 n_items;
		f->read(&n_items, sizeof(UINT32));

		MERC_LEAVE_ITEM** anchor = &gpLeaveListHead[i];
		for (UINT32 n = n_items; n != 0; --n)
		{
			MERC_LEAVE_ITEM* const li = new MERC_LEAVE_ITEM{};

			BYTE  data[40];
			f->read(data, sizeof(data));

			DataReader d{data};
			ExtractObject(d, &li->o);
			EXTR_SKIP(d, 4)
			Assert(d.getConsumed() == lengthof(data));

			// Append node to list
			*anchor = li;
			anchor  = &li->pNext;
		}
	}

	// Load the leave list profile IDs
	for (INT32 i = 0; i < NUM_LEAVE_LIST_SLOTS; ++i)
	{
		f->read(&guiLeaveListOwnerProfileId[i], sizeof(UINT32));
	}
}


void TurnOnSectorLocator( UINT8 ubProfileID )
{
	Assert( ubProfileID != NO_PROFILE );

	const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubProfileID);
	if( pSoldier )
	{
		gsSectorLocator = SGPSector(pSoldier->sSectorX, pSoldier->sSectorY);
	}
	else
	{
		// if it's Skyrider (when he's not on our team), and his chopper has been setup
		if ( ( ubProfileID == SKYRIDER ) && fSkyRiderSetUp )
		{
			// if helicopter position is being shown, don't do this, too, cause the helicopter icon is on top and it looks
			// like crap.  I tried moving the heli icon blit to before, but that screws up it's blitting.
			if ( !fShowAircraftFlag )
			{
				// can't use his profile, he's where his chopper is
				VEHICLETYPE const& v = GetHelicopter();
				gsSectorLocator = SGPSector(v.sSectorX, v.sSectorY);
			}
			else
			{
				return;
			}
		}
		else
		{
			gsSectorLocator = SGPSector(gMercProfiles[ubProfileID].sSectorX, gMercProfiles[ubProfileID].sSectorY);
		}
	}
	gubBlitSectorLocatorCode = LOCATOR_COLOR_YELLOW;
}


void TurnOffSectorLocator()
{
	gubBlitSectorLocatorCode = LOCATOR_COLOR_NONE;
	fMapPanelDirty = TRUE;
}



void HandleBlitOfSectorLocatorIcon( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ, UINT8 ubLocatorID )
{
	static UINT8  ubFrame = 0;
	UINT8 ubBaseFrame = 0;
	UINT32 uiTimer = 0;
	INT16 sScreenX, sScreenY;


	// blits at 0,0 had been observerd...
	Assert( ( sSectorX >= 1 ) && ( sSectorX <= 16 ) );
	Assert( ( sSectorY >= 1 ) && ( sSectorY <= 16 ) );
	Assert( ( sSectorZ >= 0 ) && ( sSectorZ <=  3 ) );

	if( sSectorZ != iCurrentMapSectorZ )
	{ //if the z level of the map screen renderer is different than the
		//sector z that we wish to locate, then don't render it
		return;
	}

	// if showing sector inventory, don't do this
	if( fShowMapInventoryPool )
	{
		return;
	}

	switch( ubLocatorID )
	{
		// grab zoomed out icon
		case LOCATOR_COLOR_RED:
			ubBaseFrame = 0;
			ubFrame = (UINT8)(ubFrame % 13);
			break;
		case LOCATOR_COLOR_YELLOW:
			ubBaseFrame = 13;
			ubFrame = (UINT8)(13 + (ubFrame % 13) );
			break;
		default:
			//not supported
			return;
	}

	//Convert the sector value into screen values.
	GetScreenXYFromMapXY( sSectorX, sSectorY, &sScreenX, &sScreenY );
	// make sure we are on the border
	if( sScreenX < MAP_GRID_X )
	{
		sScreenX = MAP_GRID_X;
	}
	sScreenY--; //Carterism ritual
	if( sScreenY < MAP_GRID_Y )
	{
		sScreenY = MAP_GRID_Y;
	}

	uiTimer = GetJA2Clock();

	// if first time in, reset value
	if( guiSectorLocatorBaseTime == 0 )
	{
		guiSectorLocatorBaseTime = GetJA2Clock( );
	}

	// check if enough time has passed to update the frame counter
	if( ANIMATED_BATTLEICON_FRAME_TIME < ( uiTimer - guiSectorLocatorBaseTime ) )
	{
		guiSectorLocatorBaseTime = uiTimer;
		ubFrame++;

		if( ubFrame > ubBaseFrame + MAX_FRAME_COUNT_FOR_ANIMATED_BATTLE_ICON )
		{
			ubFrame = ubBaseFrame;
		}
	}

	RestoreExternBackgroundRect(  (INT16)(sScreenX + 1), (INT16)(sScreenY - 1),  MAP_GRID_X , MAP_GRID_Y );

	// blit object to frame buffer
	BltVideoObject(FRAME_BUFFER, guiSectorLocatorGraphicID, ubFrame, sScreenX, sScreenY);

	// invalidate region on frame buffer
	InvalidateRegion(sScreenX, sScreenY, sScreenX + MAP_GRID_X + 1, sScreenY + MAP_GRID_Y + 1);
}


void MakeDialogueEventShowContractMenu(SOLDIERTYPE& s)
{
	class CharacterDialogueEventShowContractMenu : public CharacterDialogueEvent
	{
		public:
			CharacterDialogueEventShowContractMenu(SOLDIERTYPE& s) : CharacterDialogueEvent(s) {}

			bool Execute()
			{
				if (!MayExecute()) return true;

				// ATE: THis is working with MARK'S STUFF :(
				// Need this stuff so that bSelectedInfoChar is set...
				SOLDIERTYPE const& s = soldier_;
				SetInfoChar(&s);
				fShowContractMenu = TRUE;
				RebuildContractBoxForMerc(&s);
				bSelectedContractChar = bSelectedInfoChar;

				return false;
			}
	};

	DialogueEvent::Add(new CharacterDialogueEventShowContractMenu(s));
}


BOOLEAN CheckIfSalaryIncreasedAndSayQuote( SOLDIERTYPE *pSoldier, BOOLEAN fTriggerContractMenu )
{
	Assert( pSoldier );

	// OK, check if their price has gone up
	if( pSoldier->fContractPriceHasIncreased )
	{
		if ( fTriggerContractMenu )
		{
			// have him say so first - post the dialogue event with the contract menu event
			MakeDialogueEventEnterMapScreen();
			HandleImportantMercQuote( pSoldier, QUOTE_MERC_GONE_UP_IN_PRICE );
			MakeDialogueEventShowContractMenu(*pSoldier);
		}
		else
		{
			// now post the dialogue event and the contratc menu event
			HandleImportantMercQuote( pSoldier, QUOTE_MERC_GONE_UP_IN_PRICE );
		}

		pSoldier->fContractPriceHasIncreased = FALSE;

		// said quote / triggered contract menu
		return( TRUE );
	}
	else
	{
		// nope, nothing to do
		return( FALSE );
	}
}


void LoadMapScreenInterfaceGraphics()
{
	guiSectorLocatorGraphicID = AddVideoObjectFromFile(INTERFACEDIR "/hilite.sti");
	guiSelectedCharArrow      = AddVideoObjectFromFile(INTERFACEDIR "/selectedchararrow.sti");
}


void DeleteMapScreenInterfaceGraphics()
{
	DeleteVideoObject(guiSectorLocatorGraphicID);
	DeleteVideoObject(guiSelectedCharArrow);
}
