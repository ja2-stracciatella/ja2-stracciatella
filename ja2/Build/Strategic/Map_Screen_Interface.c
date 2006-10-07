#ifdef PRECOMPILEDHEADERS
	#include "Strategic All.h"
#else
	#include "Map_Screen_Interface.h"
	#include "string.h"
	#include "Map_Screen_Interface_Map.h"
	#include "Render_Dirty.h"
	#include "Font_Control.h"
	#include "Assignments.h"
	#include "Soldier_Control.h"
	#include "Overhead.h"
	#include "Squads.h"
	#include "Sound_Control.h"
	#include "SoundMan.h"
	#include "Message.h"
	#include "PopUpBox.h"
	#include "Game_Clock.h"
	#include "Handle_Items.h"
	#include "Interface_Items.h"
	#include "Isometric_Utils.h"
	#include "Interface.h"
	#include "Game_Event_Hook.h"
	#include "Tactical_Save.h"
	#include "Quests.h"
	#include "StrategicMap.h"
	#include "Soldier_Profile.h"
	#include "Strategic_Movement.h"
	#include "Dialogue_Control.h"
	#include "Map_Screen_Interface_Border.h"
	#include "Map_Screen_Interface_Bottom.h"
	#include "Vehicles.h"
	#include "Line.h"
	#include "Text.h"
	#include "Gameloop.h"
	#include "Map_Screen_Helicopter.h"
	#include "PreBattle_Interface.h"
	#include "WordWrap.h"
	#include "Interface_Control.h"
	#include "GameSettings.h"
	#include "Campaign_Types.h"
	#include "MapScreen.h"
	#include "Map_Screen_Interface_Map_Inventory.h"
	#include "Strategic.h"
	#include "Keys.h"
	#include "Soldier_Macros.h"
	#include "Random.h"
	#include "RenderWorld.h"
	#include "Strategic_Mines.h"
	#include "Air_Raid.h"
	#include "Queen_Command.h"
	#include "Render_Fun.h"
	#include "Cursor_Control.h"
	#include "Game_Init.h"
	#include "Finances.h"
	#include "Button_System.h"
	#include "JAScreens.h"
	#include "Local.h"
	#include "Video.h"
	#include "MemMan.h"
	#include "Debug.h"
	#include "ScreenIDs.h"
	#include "VSurface.h"
	#include "WCheck.h"
	#include "EMail.h"
#endif

// inventory pool position on screen
#define MAP_INVEN_POOL_X 300
#define MAP_INVEN_POOL_Y 300

// the number of help region messages
#define NUMBER_OF_MAPSCREEN_HELP_MESSAGES 5

// number of LINKED LISTS for sets of leave items (each slot holds an unlimited # of items)
#define NUM_LEAVE_LIST_SLOTS 20

#define SELECTED_CHAR_ARROW_X 8

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


#define TIMER_FOR_SHOW_EXIT_TO_TACTICAL_MESSAGE 15000

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

FASTHELPREGION pFastHelpMapScreenList[ MAX_MAPSCREEN_FAST_HELP ];

// the move menu region
MOUSE_REGION gMoveMenuRegion[ MAX_POPUP_BOX_STRING_COUNT ];

MOUSE_REGION gMapScreenHelpTextMask;

BOOLEAN fShowMapScreenHelpText = FALSE;
BOOLEAN fScreenMaskForMoveCreated = FALSE;
BOOLEAN fLockOutMapScreenInterface = FALSE;

extern INT32 giMercPanelImage;
extern BOOLEAN fShowDescriptionFlag;
extern BOOLEAN fInMapMode;
extern GROUP *gpBattleGroup;

CHAR16 gsCustomErrorString[ 128 ];

BOOLEAN fShowUpdateBox = FALSE;
BOOLEAN fInterfaceFastHelpTextActive = FALSE;
BOOLEAN fReBuildCharacterList = FALSE;
INT32 giSizeOfInterfaceFastHelpTextList = 0;

//Animated sector locator icon variables.
INT16 gsSectorLocatorX;
INT16 gsSectorLocatorY;
UINT8 gubBlitSectorLocatorCode; //color
UINT32 guiSectorLocatorGraphicID; //icon graphic ID
// the animate time per frame in milliseconds
#define ANIMATED_BATTLEICON_FRAME_TIME 80
#define MAX_FRAME_COUNT_FOR_ANIMATED_BATTLE_ICON 12



SGPPoint pMapScreenFastHelpLocationList[]={
	{ 25,200 },
	{ 150,200 },
	{ 450,430 },
	{ 400,200 },
	{ 250,100 },
	{ 100,100 },
	{ 100,100 },
	{ 100,100 },
	{ 100,100 },
	{ 150,200 },
	{ 100,100 },
};

INT32 pMapScreenFastHelpWidthList[]={
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	300,
};

// number of mercs in sector capable of moving
INT32 giNumberOfSoldiersInSectorMoving = 0;

// number of squads capable of moving
INT32 giNumberOfSquadsInSectorMoving = 0;

// number of vehicles in sector moving
INT32 giNumberOfVehiclesInSectorMoving = 0;

INT32 iHeightOfInitFastHelpText = 0;

extern INT32 giMapContractButton;
extern INT32 giCharInfoButton[];
extern STR16 pUpdatePanelButtons[];

// the list of soldiers that are moving
SOLDIERTYPE * pSoldierMovingList[ MAX_CHARACTER_COUNT ];
BOOLEAN fSoldierIsMoving[ MAX_CHARACTER_COUNT ];

SOLDIERTYPE *pUpdateSoldierBox[ SIZE_OF_UPDATE_BOX ];

INT32 giUpdateSoldierFaces[ SIZE_OF_UPDATE_BOX ];

// the squads thata re moving
INT32 iSquadMovingList[ NUMBER_OF_SQUADS ];
INT32 fSquadIsMoving[ NUMBER_OF_SQUADS ];

// the vehicles thata re moving
INT32 iVehicleMovingList[ NUMBER_OF_SQUADS ];
INT32 fVehicleIsMoving[ NUMBER_OF_SQUADS ];

MOUSE_REGION gMoveBoxScreenMask;

// the save buffer
extern UINT32 guiSAVEBUFFER;

extern BOOLEAN fShowInventoryFlag;
extern FACETYPE *gpCurrentTalkingFace;
extern UINT8			gubCurrentTalkingID;
extern BOOLEAN fMapScreenBottomDirty;
extern MOUSE_REGION    gMPanelRegion;


// has the inventory pool been selected to be on or off?
BOOLEAN fMapInventoryPoolInited = FALSE;
BOOLEAN fShowMapScreenMovementList = FALSE;


MapScreenCharacterSt gCharactersList[ MAX_CHARACTER_COUNT+1];

extern MOUSE_REGION gCharInfoHandRegion;
MOUSE_REGION	gMapStatusBarsRegion;

SGPPoint MovePosition={450, 100 };

// which lines are selected? .. for assigning groups of mercs to the same thing
BOOLEAN fSelectedListOfMercsForMapScreen[ MAX_CHARACTER_COUNT ];
BOOLEAN fResetTimerForFirstEntryIntoMapScreen = FALSE;
INT32 iReasonForSoldierUpDate = NO_REASON_FOR_UPDATE;

// sam and mine icons
UINT32 guiSAMICON;


// disable team info panels due to battle roster
BOOLEAN   fDisableDueToBattleRoster = FALSE;

// track old contract times
INT32 iOldContractTimes[ MAX_CHARACTER_COUNT ];

// position of pop up box
INT32 giBoxY = 0;

// screen mask for inventory pop up
MOUSE_REGION gInventoryScreenMask;

MOUSE_REGION gContractIconRegion;
MOUSE_REGION gInsuranceIconRegion;
MOUSE_REGION gDepositIconRegion;

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
UINT32    guiPOPUPTEX;
UINT32    guiPOPUPBORDERS;

// the currently selected character arrow
UINT32		guiSelectedCharArrow;

INT32 guiUpdatePanelButtonsImage[ 2 ];
INT32 guiUpdatePanelButtons[ 2 ];

// the update panel
UINT32 guiUpdatePanel;
UINT32 guiUpdatePanelTactical;

// the leave item list
MERC_LEAVE_ITEM *gpLeaveListHead[ NUM_LEAVE_LIST_SLOTS ];

// holds ids of mercs who left stuff behind
UINT32 guiLeaveListOwnerProfileId[ NUM_LEAVE_LIST_SLOTS ];

// flag to reset contract region glow
BOOLEAN fResetContractGlow = FALSE;

// timers for double click
INT32 giDblClickTimersForMoveBoxMouseRegions[ MAX_POPUP_BOX_STRING_COUNT ];

INT32 giExitToTactBaseTime = 0;
UINT32 guiSectorLocatorBaseTime = 0;


// which menus are we showing
BOOLEAN fShowAssignmentMenu = FALSE;
BOOLEAN fShowTrainingMenu = FALSE;
BOOLEAN fShowAttributeMenu = FALSE;
BOOLEAN fShowSquadMenu = FALSE;
BOOLEAN fShowContractMenu = FALSE;
BOOLEAN fShowRemoveMenu =  FALSE;

BOOLEAN fRebuildMoveBox = FALSE;

// positions for all the pop up boxes
SGPRect ContractDimensions={0,0,140,60};
SGPPoint ContractPosition={120,50};
SGPRect AttributeDimensions={0,0,100,95};
SGPPoint AttributePosition={220,150};
SGPRect TrainDimensions={0,0,100,95};
SGPPoint TrainPosition={160,150};
SGPRect VehicleDimensions={0,0,80,60};
SGPPoint VehiclePosition={160,150};

SGPPoint RepairPosition={160,150};
SGPRect RepairDimensions={0,0,80,80};

SGPRect AssignmentDimensions = { 0,0, 100, 95};
SGPPoint AssignmentPosition = { 120, 150 };
SGPPoint SquadPosition ={ 160, 150 };
SGPRect SquadDimensions ={ 0, 0, 140, 60 };

SGPPoint OrigContractPosition={120,50};
SGPPoint OrigAttributePosition={220,150};
SGPPoint OrigSquadPosition ={ 160, 150 };
SGPPoint OrigAssignmentPosition = { 120, 150 };
SGPPoint OrigTrainPosition={160,150};
SGPPoint OrigVehiclePosition={160,150};

//extern BOOLEAN fMapExitDueToMessageBox;

// at least one merc was hired at some time
BOOLEAN gfAtLeastOneMercWasHired = FALSE;

// rebuild contract box this character
extern void RebuildContractBoxForMerc( SOLDIERTYPE *pCharacter );

extern void SetUpCursorForStrategicMap( void );

extern void MapScreenDefaultOkBoxCallback( UINT8 bExitValue );

extern BOOLEAN PlayerSoldierTooTiredToTravel( SOLDIERTYPE *pSoldier );

extern void RememberPreviousPathForAllSelectedChars( void );


// the screen mask functions
void CreateScreenMaskForInventoryPoolPopUp( void );
void RemoveScreenMaskForInventoryPoolPopUp( void );
void InventoryScreenMaskBtnCallback(MOUSE_REGION * pRegion, INT32 iReason );

void MapScreenHelpTextScreenMaskBtnCallback( MOUSE_REGION * pRegion, INT32 iReason );
void SetUpShutDownMapScreenHelpTextScreenMask( void );
void DisplayFastHelpRegions( FASTHELPREGION *pRegion, INT32 iSize );
void DisplayUserDefineHelpTextRegions( FASTHELPREGION *pRegion );


// how many people does the player have?
//INT32 GetNumberOfCharactersOnPlayersTeam( void );

void AddStringsToMoveBox( void );
void CreatePopUpBoxForMovementBox( void );
void MoveMenuMvtCallback(MOUSE_REGION * pRegion, INT32 iReason );
void MoveMenuBtnCallback(MOUSE_REGION * pRegion, INT32 iReason );
void SelectAllOtherSoldiersInList( void );
void DeselectAllOtherSoldiersInList( void );
void HandleMoveoutOfSectorMovementTroops( void );
void HandleSettingTheSelectedListOfMercs( void );
void BuildMouseRegionsForMoveBox( void );
INT32 HowManyMovingSoldiersInVehicle( INT32 iVehicleId );
INT32 HowManyMovingSoldiersInSquad( INT32 iSquadNumber );
void ClearMouseRegionsForMoveBox( void );
BOOLEAN AllOtherSoldiersInListAreSelected( void );
BOOLEAN AllSoldiersInSquadSelected( INT32 iSquadNumber );

void MoveScreenMaskBtnCallback(MOUSE_REGION * pRegion, INT32 iReason );
/*
void CreateUpdateBoxStrings( void );
void CreateUpdateBox( void );
void RemoveUpdateBox( void );
void DisplayUpdateBox( void );
*/
void CreateDestroyUpdatePanelButtons(INT32 iX, INT32 iY, BOOLEAN fFourWideMode );
void RenderSoldierSmallFaceForUpdatePanel( INT32 iIndex, INT32 iX, INT32 iY );
static void ContinueUpdateButtonCallback(GUI_BUTTON *btn, INT32 reason);
static void StopUpdateButtonCallback(GUI_BUTTON *btn, INT32 reason);
//INT32 GetSquadListIndexForSquadNumber( INT32 iSquadNumber );
INT8 FindSquadThatSoldierCanJoin( SOLDIERTYPE *pSoldier );
BOOLEAN CanSoldierMoveWithVehicleId( SOLDIERTYPE *pSoldier, INT32 iVehicle1Id );
BOOLEAN IsAnythingSelectedForMoving( void );
BOOLEAN CanMoveBoxSoldierMoveStrategically( SOLDIERTYPE *pSoldier, BOOLEAN fShowErrorMessage );

BOOLEAN ValidSelectableCharForNextOrPrev( INT32 iNewCharSlot );

extern void ResumeOldAssignment( SOLDIERTYPE *pSoldier );



void InitalizeVehicleAndCharacterList( void )
{
	// will init the vehicle and character lists to zero
	memset(&gCharactersList, 0, sizeof( gCharactersList ));

	return;
}


void SetEntryInSelectedCharacterList( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );

	// set this entry to selected
	fSelectedListOfMercsForMapScreen[ bEntry ] = TRUE ;

	return;
}

void ResetEntryForSelectedList( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );

	// set this entry to selected
	fSelectedListOfMercsForMapScreen[ bEntry ] = FALSE;

	return;
}

void ResetSelectedListForMapScreen( void )
{
	// set all the entries int he selected list to false
	memset( &fSelectedListOfMercsForMapScreen, FALSE, MAX_CHARACTER_COUNT * sizeof( BOOLEAN ) );

	// if we still have a valid dude selected
	if ( ( bSelectedInfoChar != -1 ) && ( gCharactersList[ bSelectedInfoChar ].fValid == TRUE ) )
	{
		// then keep him selected
		SetEntryInSelectedCharacterList( bSelectedInfoChar );
	}

	return;
}


BOOLEAN IsEntryInSelectedListSet( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );

	// is this entry in the selected list set?

	return( fSelectedListOfMercsForMapScreen[ bEntry ] );

}


void ToggleEntryInSelectedList( INT8 bEntry )
{
	Assert( ( bEntry >= 0 ) && ( bEntry < MAX_CHARACTER_COUNT ) );

	// toggle the value in the selected list
	fSelectedListOfMercsForMapScreen[ bEntry ] = !( fSelectedListOfMercsForMapScreen[ bEntry ] );

	return;
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

	for( bStart; bStart <= bEnd; bStart++ )
	{
		SetEntryInSelectedCharacterList( bStart );
	}

	return;
}


BOOLEAN MultipleCharacterListEntriesSelected( void )
{
	UINT8 ubSelectedCnt = 0;
	INT32 iCounter = 0;

	// check if more than one person is selected in the selected list
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++  )
	{
		if( fSelectedListOfMercsForMapScreen[iCounter] == TRUE )
		{
			ubSelectedCnt++;
		}
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



void ResetAssignmentsForMercsTrainingUnpaidSectorsInSelectedList( INT8 bAssignment )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;

 	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		// valid character?
		if( gCharactersList[ iCounter ].fValid == FALSE )
		{
			// nope
			continue;
		}

		pSoldier = &Menptr[ gCharactersList[ iCounter ].usSolID ];

		if( pSoldier->bActive == FALSE )
		{
			continue;
		}

		if( pSoldier->bAssignment == TRAIN_TOWN )
		{
			if ( SectorInfo[ SECTOR( pSoldier->sSectorX, pSoldier->sSectorY ) ].fMilitiaTrainingPaid == FALSE )
			{
				ResumeOldAssignment( pSoldier );
			}
		}
	}
}


void ResetAssignmentOfMercsThatWereTrainingMilitiaInThisSector( INT16 sSectorX, INT16 sSectorY )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		// valid character?
		if( gCharactersList[ iCounter ].fValid == FALSE )
		{
			// nope
			continue;
		}

		pSoldier = &Menptr[ gCharactersList[ iCounter ].usSolID ];

		if( pSoldier->bActive == FALSE )
		{
			continue;
		}

		if( pSoldier->bAssignment == TRAIN_TOWN )
		{
			if( ( pSoldier->sSectorX == sSectorX ) && ( pSoldier->sSectorY == sSectorY ) && ( pSoldier->bSectorZ == 0 ) )
			{
				ResumeOldAssignment( pSoldier );
			}
		}
	}
}



/*
void PlotPathForSelectedCharacterList( INT16 sX, INT16 sY )
{
	INT32 iCounter = 0;
	// run through list and build paths for each character
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( ( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )&&( bSelectedDestChar != iCounter ) )
		{
			// character is valid.. do this for every one not the bSelectedDestChar
			PlotPathForCharacter( &Menptr[ gCharactersList[ iCounter ].usSolID ], sX, sY, FALSE );
		}
	}
}
*/


// check if the members of the selected list move with this guy... are they in the same mvt group?
void DeselectSelectedListMercsWhoCantMoveWithThisGuy( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier2 = NULL;


	// deselect any other selected mercs that can't travel together with pSoldier
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			if( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )
			{
				pSoldier2 = &( Menptr[ gCharactersList[ iCounter ].usSolID ] );

				// skip the guy we are
				if ( pSoldier == pSoldier2 )
				{
					continue;
				}


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
	}

	return;
}



void SelectUnselectedMercsWhoMustMoveWithThisGuy( void )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;


	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			// if not already selected
			if( fSelectedListOfMercsForMapScreen[ iCounter ] == FALSE )
			{
				pSoldier = &( Menptr[ gCharactersList[ iCounter ].usSolID ] );

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
	}
}



BOOLEAN AnyMercInSameSquadOrVehicleIsSelected( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier2 = NULL;


	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			// if selected
			if( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )
			{
				pSoldier2 = &( Menptr[ gCharactersList[ iCounter ].usSolID ] );

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
		}
	}

	return ( FALSE );
}



void RestoreBackgroundForAssignmentGlowRegionList( void )
{
	static INT32 iOldAssignmentLine = -1;

	// will restore the background region of the assignment list after a glow has ceased
	// ( a _LOST_MOUSE reason to the assignment region mvt callback handler )

	if( fShowAssignmentMenu == TRUE )
	{
		// force update
		ForceUpDateOfBox( ghAssignmentBox );
		ForceUpDateOfBox( ghEpcBox );
		ForceUpDateOfBox( ghRemoveMercAssignBox );
		if( fShowSquadMenu == TRUE )
		{
			ForceUpDateOfBox( ghSquadBox );
		}
		else if( fShowTrainingMenu == TRUE )
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
		RestoreExternBackgroundRect( 66, Y_START - 1, 118 + 1 - 67, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) );

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldAssignmentLine = giAssignHighLine;
	}

	// leave
	return;
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
		RestoreExternBackgroundRect( 182, Y_START - 1, 217 + 1 - 182, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) );

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldDestinationLine = giDestHighLine;
	}

	// leave
	return;
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
		RestoreExternBackgroundRect( 222, Y_START - 1, 250 + 1 - 222, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) ) ;

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldContractLine = giContractHighLine;

		// reset color rotation
		fResetContractGlow = TRUE;
	}

	// leave
	return;
}


void RestoreBackgroundForSleepGlowRegionList( void )
{
	static INT32 iOldSleepHighLine = -1;

	// will restore the background region of the destinations list after a glow has ceased
	// ( a _LOST_MOUSE reason to the assignment region mvt callback handler )

	if( fDisableDueToBattleRoster )
	{
		return;
	}

	if( iOldSleepHighLine != giSleepHighLine )
	{
		// restore background
		RestoreExternBackgroundRect( 123, Y_START - 1, 142 + 1 - 123, ( INT16 )( ( ( MAX_CHARACTER_COUNT + 1 ) * ( Y_SIZE + 2 ) ) + 1 ) ) ;

		// ARM: not good enough! must reblit the whole panel to erase glow chunk restored by help text disappearing!!!
		fTeamPanelDirty = TRUE;

		// set old to current
		iOldSleepHighLine = giSleepHighLine;

		// reset color rotation
		fResetContractGlow = TRUE;
	}

	// leave
	return;
}

void PlayGlowRegionSound( void )
{
	// play a new message sound, if there is one playing, do nothing
	static UINT32 uiSoundId = 0;

	if( uiSoundId != 0 )
	{
		// is sound playing?..don't play new one
		if( SoundIsPlaying( uiSoundId ) == TRUE )
		{
			return;
		}
	}

	// otherwise no sound playing, play one
	uiSoundId = PlayJA2SampleFromFile( "Sounds\\glowclick.wav", RATE_11025, MIDVOLUME, 1 , MIDDLEPAN );

	return;
}



INT16 CharacterIsGettingPathPlotted( INT16 sCharNumber )
{
	// valid character number?
	if( ( sCharNumber < 0 ) || ( sCharNumber >= MAX_CHARACTER_COUNT ) )
	{
		return( FALSE );
	}

	// is the character a valid one?
	if( gCharactersList[ sCharNumber ].fValid == FALSE )
	{
		return ( FALSE );
	}

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
	if( gCharactersList[ sCharNumber ].fValid == FALSE )
	{
		return ( FALSE );
	}

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
	if( gCharactersList[ sCharNumber ].fValid == FALSE )
	{
		return ( FALSE );
	}

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

	return;
}


void EnableTeamInfoPanels( void )
{
	// enable team info panel
	fDisableDueToBattleRoster = FALSE;

	return;
}


/*
void ActivateSoldierPopup( SOLDIERTYPE *pSoldier, UINT8 ubPopupType, INT16 xp, INT16 yp )
{
	// will activate the pop up for prebattle interface

	// get the soldier id number
	INT8 bCounter = 0;
	INT8 bCharacter = -1;


	for( bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++ )
	{
		if( gCharactersList[ bCounter ].fValid == TRUE )
		{
			// is this guy the passed soldier?
			if( pSoldier == &( Menptr[ gCharactersList[ bCounter ].usSolID ] ) )
			{
				bCharacter = bCounter;
				break;
			}
		}
	}

	giBoxY = ( INT32 ) yp;
	// which type of box do we show?
	switch( ubPopupType )
	{
		case( ASSIGNMENT_POPUP ):
			bSelectedDestChar = -1;
			bSelectedContractChar = -1;
			bSelectedAssignChar = bCharacter;
			if( ( pSoldier->bLife > 0 ) &&( pSoldier->bAssignment != ASSIGNMENT_POW ) )
			{
				fShowAssignmentMenu = TRUE;
			}
			else
			{
				fShowRemoveMenu = TRUE;
			}

			// set box y positions
			AssignmentPosition.iY =  giBoxY;
			TrainPosition.iY = AssignmentPosition.iY + GetFontHeight( MAP_SCREEN_FONT )* ASSIGN_MENU_TRAIN;
			AttributePosition.iY = 	TrainPosition.iY;
			SquadPosition.iY = 	AssignmentPosition.iY;
			break;
		case( DESTINATION_POPUP ):
			bSelectedDestChar = bCharacter;
			bSelectedContractChar = -1;
			bSelectedAssignChar = -1;

			// set box y value
			ContractPosition.iY = giBoxY;
			break;
		case( CONTRACT_POPUP ):
			bSelectedDestChar = -1;
			bSelectedContractChar = bCharacter;
			bSelectedAssignChar = -1;
			RebuildContractBoxForMerc( pSoldier );

			if( ( pSoldier->bLife > 0 ) &&( pSoldier->bAssignment != ASSIGNMENT_POW ) )
			{
				fShowContractMenu = TRUE;
			}
			else
			{
				fShowRemoveMenu = TRUE;
			}
			break;
	}
}
*/



INT32 DoMapMessageBoxWithRect( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, SGPRect *pCenteringRect )
{	// reset the highlighted line
	giHighLine = -1;
  return DoMessageBox( ubStyle, zString, uiExitScreen, ( UINT16 ) ( usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT ), ReturnCallback, pCenteringRect );
}



INT32 DoMapMessageBox( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback )
{
  SGPRect CenteringRect= {0, 0, 640, INV_INTERFACE_START_Y };

	// reset the highlighted line
	giHighLine = -1;

	// do message box and return
  return DoMessageBox(  ubStyle,  zString,  uiExitScreen, ( UINT16 ) ( usFlags| MSG_BOX_FLAG_USE_CENTERING_RECT ),  ReturnCallback,  &CenteringRect );
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
	if( IsMapScreenHelpTextUp() )
	{
		// stop mapscreen text
		StopMapScreenHelpText( );
		return;
	}

	if( gfPreBattleInterfaceActive == TRUE )
	{
		return;
	}

	// disable level-changes while in inventory pool (for keyboard equivalents!)
	if( fShowMapInventoryPool )
		return;


	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
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
		if( gCharactersList[iCounter].fValid == TRUE )
		{
				// what kind of merc
			 if(Menptr[gCharactersList[iCounter].usSolID].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			 {
				 // amount of time left on contract
				 iTimeRemaining=Menptr[gCharactersList[iCounter].usSolID].iEndofContractTime-GetWorldTotalMin();
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
			else if( Menptr[gCharactersList[iCounter].usSolID].ubWhatKindOfMercAmI == MERC_TYPE__MERC )
			{
				iTimeRemaining = Menptr[gCharactersList[iCounter].usSolID].iTotalContractLength;

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
}



void HandleDisplayOfSelectedMercArrows( void )
{
	INT16 sYPosition = 0;
	HVOBJECT hHandle;
	UINT8 ubCount = 0;
	// blit an arrow by the name of each merc in a selected list
	if( bSelectedInfoChar == -1 )
	{
		return;
	}

	// is the character valid?
	if( gCharactersList[ bSelectedInfoChar ].fValid == FALSE )
	{
		return;
	}

	if( fShowInventoryFlag == TRUE )
	{
		return;
	}
	// now blit one by the selected merc
	sYPosition = Y_START+( bSelectedInfoChar * ( Y_SIZE + 2 ) ) - 1;


	if( bSelectedInfoChar >= FIRST_VEHICLE )
	{
		sYPosition += 6;
	}


	GetVideoObject( &hHandle, guiSelectedCharArrow );
	BltVideoObject( guiSAVEBUFFER , hHandle, 0,SELECTED_CHAR_ARROW_X, sYPosition , VO_BLT_SRCTRANSPARENCY,NULL );

	// now run through the selected list of guys, an arrow for each
	for( ubCount = 0; ubCount < MAX_CHARACTER_COUNT; ubCount++ )
	{
		if( gCharactersList[ ubCount ].fValid == TRUE )
		{

			// are they in the selected list or int he same mvt group as this guy
			if( ( IsEntryInSelectedListSet( ubCount ) == TRUE ) || ( ( bSelectedDestChar != - 1 ) ? ( ( Menptr[ gCharactersList[ ubCount ].usSolID ].ubGroupID != 0 ) ? ( Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ].ubGroupID == Menptr[ gCharactersList[ ubCount ].usSolID ].ubGroupID ) : FALSE ) : FALSE ) )
			{
				sYPosition = Y_START+( ubCount * ( Y_SIZE + 2) ) - 1;
				if( ubCount >= FIRST_VEHICLE )
				{
					sYPosition += 6;
				}

				GetVideoObject( &hHandle, guiSelectedCharArrow );
				BltVideoObject( guiSAVEBUFFER , hHandle, 0,SELECTED_CHAR_ARROW_X, sYPosition , VO_BLT_SRCTRANSPARENCY,NULL );
			}
		}
	}
	return;
}



void HandleDisplayOfItemPopUpForSector( INT16 sMapX, INT16 sMapY, INT16 sMapZ )
{
	// handle display of item pop up for this sector
	// check if anyone alive in this sector
	ITEM_POOL		*pItemPool = NULL;
	static BOOLEAN fWasInited = FALSE;

	if( bSelectedInfoChar == -1 )
	{
		return;
	}


	if( ( fWasInited == FALSE ) && ( fMapInventoryPoolInited ) )
	{
		if( gCharactersList[ bSelectedInfoChar ].fValid == TRUE )
		{
			if( ( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID].sSectorX == sMapX ) && ( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID].sSectorY == sMapY ) && ( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID].bSectorZ == sMapZ ) && ( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID].bActive ) && ( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID].bLife >= OKLIFE ) )
			{
				// valid character
				InitializeItemPickupMenu( &( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID] ), NOWHERE , pItemPool, MAP_INVEN_POOL_X, MAP_INVEN_POOL_Y, -1 );
				fWasInited = TRUE;

				CreateScreenMaskForInventoryPoolPopUp( );
			}
		}
	}
	else if( ( fWasInited == TRUE ) && ( fMapInventoryPoolInited == FALSE ) )
	{
		fWasInited = FALSE;

		// now clear up the box
		RemoveItemPickupMenu( );

		// remove screen mask
		RemoveScreenMaskForInventoryPoolPopUp( );

		// drity nessacary regions
		fMapPanelDirty = TRUE;

	}

	// showing it
	if( ( fMapInventoryPoolInited ) && ( fWasInited ) )
	{
		SetPickUpMenuDirtyLevel( DIRTYLEVEL2 );
		RenderItemPickupMenu( );
	}


	return;
}



void CreateScreenMaskForInventoryPoolPopUp( void )
{
	//  a screen mask for the inventory pop up
	MSYS_DefineRegion( &gInventoryScreenMask, 0, 0 , 640, 480, MSYS_PRIORITY_HIGH - 1,
						 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, InventoryScreenMaskBtnCallback );

}

void RemoveScreenMaskForInventoryPoolPopUp( void )
{
	// remove screen mask
	MSYS_RemoveRegion( &gInventoryScreenMask );
}


// invnetory screen mask btn callback
void InventoryScreenMaskBtnCallback(MOUSE_REGION * pRegion, INT32 iReason )
{
  // inventory screen mask btn callback
  if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
	  fMapInventoryPoolInited = FALSE;
  }
}


void GetMoraleString( SOLDIERTYPE *pSoldier, STR16 sString )
{
	INT8 bMorale = pSoldier->bMorale;

	if ( pSoldier->uiStatusFlags & SOLDIER_DEAD )
	{
    wcscpy( sString, pMoralStrings[ 5 ] );
	}
	else if( bMorale > 80 )
	{
		wcscpy( sString, pMoralStrings[ 0 ] );
	}
	else if( bMorale > 65 )
	{
    wcscpy( sString, pMoralStrings[ 1 ] );
	}
	else if( bMorale > 35 )
	{
    wcscpy( sString, pMoralStrings[ 2 ] );
	}
	else if( bMorale > 20 )
	{
		wcscpy( sString, pMoralStrings[ 3 ] );
	}
	else
	{
    wcscpy( sString, pMoralStrings[ 4 ] );
	}
}



// NOTE: This doesn't use the "LeaveList" system at all!
void HandleLeavingOfEquipmentInCurrentSector( UINT32 uiMercId )
{
	// just drop the stuff in the current sector
	INT32 iCounter = 0;
  INT16 sGridNo, sTempGridNo;

	if( Menptr[ uiMercId ].sSectorX != gWorldSectorX || Menptr[ uiMercId ].sSectorY != gWorldSectorY || Menptr[ uiMercId ].bSectorZ != gbWorldSectorZ )
	{
    // ATE: Use insertion gridno if not nowhere and insertion is gridno
   if ( Menptr[ uiMercId ].ubStrategicInsertionCode == INSERTION_CODE_GRIDNO && Menptr[ uiMercId ].usStrategicInsertionData != NOWHERE )
   {
		  sGridNo = Menptr[ uiMercId ].usStrategicInsertionData;
   }
   else
   {
      // Set flag for item...
      sGridNo = RandomGridNo();
   }
	}
  else
  {
    // ATE: Mercs can have a gridno of NOWHERE.....
    sGridNo = Menptr[ uiMercId ].sGridNo;

    if ( sGridNo == NOWHERE )
    {
      sGridNo = RandomGridNo();

			sTempGridNo = FindNearestAvailableGridNoForItem( sGridNo, 5 );
			if( sTempGridNo == NOWHERE )
				sTempGridNo = FindNearestAvailableGridNoForItem( sGridNo, 15 );

      if ( sTempGridNo != NOWHERE )
      {
        sGridNo = sTempGridNo;
      }
    }
  }

	for( iCounter = 0; iCounter < NUM_INV_SLOTS; iCounter++ )
	{
		// slot found,
		// check if actual item
		if(  Menptr[ uiMercId ].inv[ iCounter ].ubNumberOfObjects > 0 )
		{
	    if( Menptr[ uiMercId ].sSectorX != gWorldSectorX || Menptr[ uiMercId ].sSectorY != gWorldSectorY || Menptr[ uiMercId ].bSectorZ != gbWorldSectorZ )
	    {
        // Set flag for item...
				AddItemsToUnLoadedSector( Menptr[ uiMercId ].sSectorX,  Menptr[ uiMercId ].sSectorY, Menptr[ uiMercId ].bSectorZ , sGridNo, 1, &( Menptr[ uiMercId ].inv[ iCounter ]) , Menptr[ uiMercId ].bLevel, WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO | WORLD_ITEM_REACHABLE, 0, 1, FALSE );
      }
			else
			{
				AddItemToPool( sGridNo, &( Menptr[ uiMercId ].inv[ iCounter ] ) , 1, Menptr[ uiMercId ].bLevel, WORLD_ITEM_REACHABLE, 0 );
			}
		}
	}

  DropKeysInKeyRing( MercPtrs[ uiMercId ], sGridNo, MercPtrs[ uiMercId ]->bLevel, 1, FALSE, 0, FALSE );

}



void HandleMercLeavingEquipmentInOmerta( UINT32 uiMercId )
{
	INT32 iSlotIndex = 0;

	// stash the items into a linked list hanging of a free "leave item list" slot
	if( ( iSlotIndex = SetUpDropItemListForMerc( uiMercId ) ) != -1 )
	{
		// post event to drop it there 6 hours later
		AddStrategicEvent( EVENT_MERC_LEAVE_EQUIP_IN_OMERTA, GetWorldTotalMin() + ( 6 * 60 ), iSlotIndex );
	}
	else
	{
		// otherwise there's no free slots left (shouldn't ever happen)
		AssertMsg( FALSE, "HandleMercLeavingEquipmentInOmerta: No more free slots, equipment lost" );
	}
}


void HandleMercLeavingEquipmentInDrassen( UINT32 uiMercId )
{
	INT32 iSlotIndex = 0;

	// stash the items into a linked list hanging of a free "leave item list" slot
	if( ( iSlotIndex = SetUpDropItemListForMerc( uiMercId ) ) != -1 )
	{
		// post event to drop it there 6 hours later
		AddStrategicEvent( EVENT_MERC_LEAVE_EQUIP_IN_DRASSEN, GetWorldTotalMin() + ( 6 * 60 ), iSlotIndex );
	}
	else
	{
		// otherwise there's no free slots left (shouldn't ever happen)
		AssertMsg( FALSE, "HandleMercLeavingEquipmentInDrassen: No more free slots, equipment lost" );
	}
}


void HandleEquipmentLeftInOmerta( UINT32 uiSlotIndex )
{
	MERC_LEAVE_ITEM *pItem;
	CHAR16 sString[ 128 ];


	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );

	pItem = gpLeaveListHead[ uiSlotIndex ];

	if( pItem )
	{
		if ( guiLeaveListOwnerProfileId[ uiSlotIndex ] != NO_PROFILE )
		{
			swprintf( sString, lengthof(sString), pLeftEquipmentString[ 0 ], gMercProfiles[ guiLeaveListOwnerProfileId[ uiSlotIndex ] ].zNickname );
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sString);
		}
		else
		{
			swprintf( sString, lengthof(sString), L"A departing merc has left their equipment in Omerta." );
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sString);
		}
	}

	while( pItem )
	{
		if( gWorldSectorX  != OMERTA_LEAVE_EQUIP_SECTOR_X || gWorldSectorY != OMERTA_LEAVE_EQUIP_SECTOR_Y || gbWorldSectorZ != OMERTA_LEAVE_EQUIP_SECTOR_Z )
		{
			// given this slot value, add to sector item list
			AddItemsToUnLoadedSector( OMERTA_LEAVE_EQUIP_SECTOR_X, OMERTA_LEAVE_EQUIP_SECTOR_Y, OMERTA_LEAVE_EQUIP_SECTOR_Z, OMERTA_LEAVE_EQUIP_GRIDNO, 1, &( pItem -> o ) , 0, WORLD_ITEM_REACHABLE, 0, 1, FALSE );
		}
		else
		{
			AddItemToPool( OMERTA_LEAVE_EQUIP_GRIDNO, &( pItem->o ), 1, 0, WORLD_ITEM_REACHABLE, 0 );
		}
		pItem = pItem -> pNext;
	}

	FreeLeaveListSlot( uiSlotIndex );
}



void HandleEquipmentLeftInDrassen( UINT32 uiSlotIndex )
{
	MERC_LEAVE_ITEM *pItem;
	CHAR16 sString[ 128 ];


	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );

	pItem = gpLeaveListHead[ uiSlotIndex ];

	if( pItem )
	{
		if ( guiLeaveListOwnerProfileId[ uiSlotIndex ] != NO_PROFILE )
		{
			swprintf( sString, lengthof(sString), pLeftEquipmentString[ 1 ], gMercProfiles[ guiLeaveListOwnerProfileId[ uiSlotIndex ] ].zNickname );
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sString);
		}
		else
		{
			swprintf( sString, lengthof(sString), L"A departing merc has left their equipment in Drassen." );
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sString);
		}
	}

	while( pItem )
	{
		if( gWorldSectorX  != BOBBYR_SHIPPING_DEST_SECTOR_X || gWorldSectorY != BOBBYR_SHIPPING_DEST_SECTOR_Y || gbWorldSectorZ != BOBBYR_SHIPPING_DEST_SECTOR_Z )
		{
			// given this slot value, add to sector item list
			AddItemsToUnLoadedSector( BOBBYR_SHIPPING_DEST_SECTOR_X, BOBBYR_SHIPPING_DEST_SECTOR_Y, BOBBYR_SHIPPING_DEST_SECTOR_Z, 10433, 1, &( pItem -> o ) , 0, WORLD_ITEM_REACHABLE, 0, 1, FALSE );
		}
		else
		{
			AddItemToPool( 10433, &( pItem->o ), 1, 0, WORLD_ITEM_REACHABLE, 0 );
		}
		pItem = pItem->pNext;
	}

	FreeLeaveListSlot( uiSlotIndex );
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


BOOLEAN AddItemToLeaveIndex( OBJECTTYPE *o, UINT32 uiSlotIndex )
{
	MERC_LEAVE_ITEM *pItem, *pCurrentItem;

	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );


	if( o == NULL )
	{
		return( FALSE );
	}

	// allocate space
	pItem = MemAlloc( sizeof( MERC_LEAVE_ITEM ) );

	// copy object
	memcpy( &( pItem->o ), o, sizeof( OBJECTTYPE ) );

	// nobody afterwards
	pItem->pNext = NULL;

	// now add to list in this index slot
	pCurrentItem = gpLeaveListHead[ uiSlotIndex ];

	if( pCurrentItem == NULL )
	{
		gpLeaveListHead[ uiSlotIndex ] = pItem;
		return( TRUE );
	}

	// move through list
	while( pCurrentItem ->pNext )
	{
		 pCurrentItem = pCurrentItem->pNext;
	}

	// found
	pCurrentItem->pNext = pItem;

	return( TRUE );
}


// release memory for all items in this slot's leave item list
void FreeLeaveListSlot( UINT32 uiSlotIndex )
{
	MERC_LEAVE_ITEM *pCurrent = NULL, *pTemp = NULL;

	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );

	pCurrent = gpLeaveListHead[ uiSlotIndex ];

	// go through nodes and free them
	while( pCurrent )
	{
		pTemp = pCurrent->pNext;
		MemFree( pCurrent );
		pCurrent = pTemp;
	}

	gpLeaveListHead[ uiSlotIndex ] = NULL;
}



INT32 FindFreeSlotInLeaveList( void )
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


INT32 SetUpDropItemListForMerc( UINT32 uiMercId )
{
	// will set up a drop list for this grunt, remove items from inventory, and profile
	INT32 iSlotIndex = -1;
	INT32 iCounter = 0;

	iSlotIndex = FindFreeSlotInLeaveList( );
	if( iSlotIndex == -1 )
	{
		return( -1 );
	}

	for( iCounter = 0; iCounter < NUM_INV_SLOTS; iCounter++ )
	{
		// slot found,
		// check if actual item
		if(  Menptr[ uiMercId ].inv[ iCounter ].ubNumberOfObjects > 0 )
		{
			// make a linked list of the items left behind, with the ptr to its head in this free slot
			AddItemToLeaveIndex( &( Menptr[ uiMercId ].inv[ iCounter ] ), iSlotIndex );

			// store owner's profile id for the items added to this leave slot index
			SetUpMercAboutToLeaveEquipment( Menptr[ uiMercId ].ubProfile, iSlotIndex );
		}
	}

  // ATE: Added this to drop keyring keys - the 2nd last paramter says to add it to a leave list...
  // the gridno, level and visiblity are ignored
  DropKeysInKeyRing( MercPtrs[ uiMercId ], NOWHERE, 0, 0, TRUE, iSlotIndex, FALSE );

	// zero out profiles
	memset( ( gMercProfiles[ Menptr[ uiMercId ].ubProfile ].bInvStatus ), 0, sizeof( UINT8 ) * 19 );
	memset( ( gMercProfiles[ Menptr[ uiMercId ].ubProfile ].bInvNumber ), 0, sizeof( UINT8 ) * 19 );
	memset( ( gMercProfiles[ Menptr[ uiMercId ].ubProfile ].inv ), 0, sizeof( UINT16 ) * 19 );

	return( iSlotIndex );
}


// store owner's profile id for the items added to this leave slot index
void SetUpMercAboutToLeaveEquipment( UINT32 ubProfileId, UINT32 uiSlotIndex )
{
	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );

	// store the profile ID of this merc in the same slot that the items are gonna be dropped in
	guiLeaveListOwnerProfileId[ uiSlotIndex ] = ubProfileId;

}


/*
BOOLEAN RemoveItemFromLeaveIndex( MERC_LEAVE_ITEM *pItem, UINT32 uiSlotIndex )
{
	MERC_LEAVE_ITEM *pCurrentItem = NULL;

	Assert( uiSlotIndex < NUM_LEAVE_LIST_SLOTS );

	if( pItem == NULL )
	{
		return( FALSE );
	}

	// item is head of list?
//ARM: THIS DOESN'T MAKE SENSE, pCurrentItem is always NULL at this stage!
	if( pItem == pCurrentItem )
	{
		gpLeaveListHead[ uiSlotIndex ] = pCurrentItem ->pNext;
		MemFree( pItem );
		pItem = NULL;
		return( TRUE );
	}

	// in the body
	while( ( pCurrentItem->pNext != pItem ) && ( pCurrentItem -> pNext != NULL ) )
	{
		pCurrentItem = pCurrentItem -> pNext;
	}

	// item not found
	if( pCurrentItem->pNext == NULL )
	{
		return( FALSE );
	}

	// set to next after next
	pCurrentItem->pNext = pCurrentItem->pNext->pNext;

	// free space and null ptr
	MemFree( pItem );
	pItem = NULL;

	return( TRUE );
}
*/



void HandleGroupAboutToArrive( void )
{
	// reblit map to change the color of the "people in motion" marker
	fMapPanelDirty = TRUE;

	// ARM - commented out - don't see why this is needed
//	fTeamPanelDirty = TRUE;
//	fCharacterInfoPanelDirty = TRUE;

	return;
}


/*
void HandleMapScreenUpArrow( void )
{
	INT32 iValue = 0;
	INT32 iHighLine = 0;

	// check state and update
	if( fShowAssignmentMenu == TRUE )
	{
		if( GetBoxShadeFlag( ghAssignmentBox, iValue ) == FALSE )
		{
			if( iHighLine ==  0)
			{
				iHighLine = ( INT32 )GetNumberOfLinesOfTextInBox( ghAssignmentBox );
			}
			else
			{
				iHighLine++;
			}
		}
	}
	else
	{
		if( ( giHighLine == 0 ) || ( giHighLine  == -1 ) )
		{
			giHighLine = GetNumberOfCharactersOnPlayersTeam( ) - 1;
			fTeamPanelDirty = TRUE;
		}
		else
		{
			giHighLine--;
			fTeamPanelDirty = TRUE;
		}

	}
}


void HandleMapScreenDownArrow( void )
{
	INT32 iValue = 0;
	INT32 iHighLine = 0;

	// check state and update
	if( fShowContractMenu == TRUE )
	{
		if( iHighLine == ( INT32 )GetNumberOfLinesOfTextInBox( ghContractBox ) - 1 )
		{
			iHighLine = 0;
		}
		else
		{
			iHighLine++;
		}

		HighLightBoxLine( ghContractBox, iHighLine );
	}
	else if( fShowAssignmentMenu == TRUE )
	{
		if( GetBoxShadeFlag( ghAssignmentBox, iValue ) == FALSE )
		{
			if( iHighLine == ( INT32 )GetNumberOfLinesOfTextInBox( ghAssignmentBox ) - 1 )
			{
				iHighLine = 0;
			}
			else
			{
				iHighLine--;
			}
		}
	}
	else
	{
		if( ( giHighLine == GetNumberOfCharactersOnPlayersTeam( ) - 1 ) || ( giHighLine  == -1 ) )
		{
			giHighLine = 0;
			fTeamPanelDirty = TRUE;
		}
		else
		{
			giHighLine++;
			fTeamPanelDirty = TRUE;
		}

	}
}


INT32 GetNumberOfCharactersOnPlayersTeam( void )
{
	INT32 iNumberOfPeople = 0, iCounter = 0;

	for(iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			iNumberOfPeople++;
		}
	}

	return( iNumberOfPeople );
}
*/


void CreateMapStatusBarsRegion( void )
{

	// create the status region over the bSelectedCharacter info region, to get quick rundown of merc's status
	MSYS_DefineRegion( &gMapStatusBarsRegion, BAR_INFO_X - 3, BAR_INFO_Y - 42,(INT16)( BAR_INFO_X + 17), (INT16)(BAR_INFO_Y ), MSYS_PRIORITY_HIGH + 5,
							MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

	return;
}


void RemoveMapStatusBarsRegion( void )
{

	// remove the bSelectedInfoCharacter helath, breath and morale bars info region
	MSYS_RemoveRegion( &gMapStatusBarsRegion );

	return;
}

void UpdateCharRegionHelpText( void )
{
	CHAR16 sString[ 128 ];
	CHAR16 pMoraleStr[ 128 ];
	SOLDIERTYPE *pSoldier = NULL;


	if( ( bSelectedInfoChar != -1 ) && ( gCharactersList[ bSelectedInfoChar ].fValid == TRUE ) )
	{
		// valid soldier selected
		pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];

		// health/energy/morale
		if( pSoldier->bAssignment != ASSIGNMENT_POW )
		{
			if ( pSoldier->bLife != 0 )
			{
				if ( AM_A_ROBOT( MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ] ) )
				{
					// robot (condition only)
					swprintf( sString, lengthof(sString), L"%S: %d/%d",
													pMapScreenStatusStrings[ 3 ], pSoldier->bLife, pSoldier->bLifeMax );
				}
				else if ( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].uiStatusFlags & SOLDIER_VEHICLE )
				{
					// vehicle (condition/fuel)
					swprintf( sString, lengthof(sString), L"%S: %d/%d, %S: %d/%d",
													pMapScreenStatusStrings[ 3 ], pSoldier->bLife, pSoldier->bLifeMax,
													pMapScreenStatusStrings[ 4 ], pSoldier->bBreath, pSoldier->bBreathMax );
				}
				else
				{
					// person (health/energy/morale)
					GetMoraleString( pSoldier, pMoraleStr );
					swprintf( sString, lengthof(sString), L"%S: %d/%d, %S: %d/%d, %S: %S",
													pMapScreenStatusStrings[ 0 ], pSoldier->bLife, pSoldier->bLifeMax,
													pMapScreenStatusStrings[ 1 ], pSoldier->bBreath, pSoldier->bBreathMax,
													pMapScreenStatusStrings[ 2 ], pMoraleStr );
				}
			}
			else
			{
				wcscpy( sString, L"" );
			}
		}
		else
		{
			// POW - stats unknown
			swprintf( sString, lengthof(sString), L"%S: ??, %S: ??, %S: ??", pMapScreenStatusStrings[ 0 ], pMapScreenStatusStrings[ 1 ], pMapScreenStatusStrings[ 2 ] );
		}

		SetRegionFastHelpText( &gMapStatusBarsRegion, sString );


		// update CONTRACT button help text
		if ( CanExtendContractForCharSlot( bSelectedInfoChar ) )
		{
			SetButtonFastHelpText( giMapContractButton, pMapScreenMouseRegionHelpText[ 3 ] );
			EnableButton( giMapContractButton );
		}
		else
		{
			SetButtonFastHelpText( giMapContractButton, L"" );
			DisableButton( giMapContractButton );
		}


		if ( CanToggleSelectedCharInventory( ) )
		{
			// inventory
			if( fShowInventoryFlag )
			{
				SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 2 ] );
			}
			else
			{
				SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );
			}
		}
		else	// can't toggle it, don't show any inventory help text
		{
			SetRegionFastHelpText( &gCharInfoHandRegion, L"" );
		}
	}
	else
	{
		// invalid soldier
		SetRegionFastHelpText( &(gMapStatusBarsRegion), L"" );
		SetButtonFastHelpText( giMapContractButton, L"" );
		SetRegionFastHelpText( &gCharInfoHandRegion, L"" );
		DisableButton( giMapContractButton );
	}
}


// find this merc in the mapscreen list and set as selected
void FindAndSetThisContractSoldier( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;

	fShowContractMenu = FALSE;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter].fValid == TRUE )
		{
			if( gCharactersList[ iCounter].usSolID == pSoldier->ubID )
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
}


void HandleMAPUILoseCursorFromOtherScreen( void )
{
	// rerender map without cursors
	fMapPanelDirty = TRUE;

	if ( fInMapMode )
	{
		RenderMapRegionBackground( );
	}
	return;
}


void UpdateMapScreenAssignmentPositions( void )
{
	// set the position of the pop up boxes
	SGPPoint pPoint;


	if( guiCurrentScreen != MAP_SCREEN )
	{
		return;
	}


	if( bSelectedAssignChar == -1 )
	{
		if( gfPreBattleInterfaceActive == FALSE )
		{
			giBoxY = 0;
		}
		return;
	}

	if( gCharactersList[ bSelectedAssignChar ].fValid == FALSE )
	{
		if( gfPreBattleInterfaceActive == FALSE )
		{
			giBoxY = 0;
		}
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
		GetBoxPosition( ghAssignmentBox, &pPoint);
		pPoint.iY = giBoxY;

		SetBoxPosition( ghAssignmentBox, pPoint );

		GetBoxPosition( ghEpcBox, &pPoint);
		pPoint.iY = giBoxY;

		SetBoxPosition( ghEpcBox, pPoint );
	}

	if( fShowAttributeMenu )
	{
		GetBoxPosition( ghAttributeBox, &pPoint);

		pPoint.iY = giBoxY + ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_TRAIN;

		SetBoxPosition( ghAttributeBox, pPoint );
	}

	if( fShowRepairMenu )
	{
		GetBoxPosition( ghRepairBox, &pPoint);
		pPoint.iY = giBoxY + ( GetFontHeight( MAP_SCREEN_FONT ) + 2 ) * ASSIGN_MENU_REPAIR;

		SetBoxPosition( ghRepairBox, pPoint );
	}



	return;
}



void RandomMercInGroupSaysQuote( GROUP *pGroup, UINT16 usQuoteNum )
{
	PLAYERGROUP *pPlayer;
	SOLDIERTYPE *pSoldier;
	UINT8				ubMercsInGroup[ 20 ];
	UINT8				ubNumMercs = 0;
	UINT8				ubChosenMerc;


	// if traversing tactically, don't do this, unless time compression was required for some reason (don't go to sector)
	if ( ( gfTacticalTraversal || ( pGroup->ubSectorZ > 0 ) ) && !IsTimeBeingCompressed() )
	{
		return;
	}


	// Let's choose somebody in group.....
	pPlayer = pGroup->pPlayerList;

	while( pPlayer != NULL )
	{
		pSoldier = pPlayer->pSoldier;
		Assert( pSoldier );

		if ( pSoldier->bLife >= OKLIFE && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) &&
					!AM_A_ROBOT( pSoldier ) && !AM_AN_EPC( pSoldier ) && !pSoldier->fMercAsleep )
		{
			ubMercsInGroup[ ubNumMercs ] = pSoldier->ubID;
			ubNumMercs++;
		}

		pPlayer = pPlayer->next;
	}

	// At least say quote....
	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );
		pSoldier = MercPtrs[ ubMercsInGroup[ ubChosenMerc ] ];

		TacticalCharacterDialogue( pSoldier, usQuoteNum );
	}
}


INT32 GetNumberOfPeopleInCharacterList( void )
{
	INT32 iCounter = 0, iCount = 0;

	// get the number of valid mercs in the mapscreen character list
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter].fValid == TRUE )
		{
			// another valid character
			iCount++;
		}
	}

	return( iCount );
}



BOOLEAN ValidSelectableCharForNextOrPrev( INT32 iNewCharSlot )
{
	BOOLEAN fHoldingItem = FALSE;

	// if holding an item
	if ( ( gMPanelRegion.Cursor == EXTERN_CURSOR ) || gpItemPointer || fMapInventoryItem )
	{
		fHoldingItem = TRUE;
	}


	// if showing merc inventory, or holding an item
	if ( fShowInventoryFlag || fHoldingItem )
	{
		// the new guy must have accessible inventory
		if ( !MapCharacterHasAccessibleInventory( ( INT8 ) iNewCharSlot ) )
		{
			return( FALSE );
		}
	}


	if ( fHoldingItem )
	{
		return ( MapscreenCanPassItemToCharNum( iNewCharSlot ) );
	}
	else
	{
		return( TRUE );
	}
}



BOOLEAN MapscreenCanPassItemToCharNum( INT32 iNewCharSlot )
{
	SOLDIERTYPE *pNewSoldier;
	SOLDIERTYPE *pOldSoldier;


	// assumes we're holding an item
	Assert( ( gMPanelRegion.Cursor == EXTERN_CURSOR ) || gpItemPointer || fMapInventoryItem );


	// if in a hostile sector, disallow
	if ( gTacticalStatus.fEnemyInSector )
	{
		return( FALSE );
	}

	// can't pass items to nobody!
	if ( iNewCharSlot == -1 )
	{
		return( FALSE );
	}


	pNewSoldier = MercPtrs[ gCharactersList[ iNewCharSlot ].usSolID ];


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
		if ( fMapInventoryItem || ( bSelectedInfoChar == -1 ) )
		{
			pOldSoldier = NULL;
		}
		else
		{
			pOldSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];
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


	if( fShowDescriptionFlag == TRUE )
	{
		return;
	}

	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
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
		if ( ( gCharactersList[ iCount ].fValid ) && ( iCount < MAX_CHARACTER_COUNT ) && ValidSelectableCharForNextOrPrev( iCount ) )
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


	if( fShowDescriptionFlag == TRUE )
	{
		return;
	}

	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
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
		if ( ( gCharactersList[ iCount ].fValid ) && ( iCount < MAX_CHARACTER_COUNT ) && ValidSelectableCharForNextOrPrev( iCount ) )
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



void HandleMinerEvent( UINT8 bMinerNumber, INT16 sSectorX, INT16 sSectorY, INT16 sQuoteNumber, BOOLEAN fForceMapscreen )
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
			EnterMapScreen( );
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

		// set up the mine sector flasher
		gsSectorLocatorX = sSectorX;
		gsSectorLocatorY = sSectorY;

		fMapPanelDirty = TRUE;

		// post dialogue events for miners to say this quote and flash the sector where his mine is
		CharacterDialogueWithSpecialEvent( ( UINT8 )uiExternalFaceProfileIds[ bMinerNumber ], sQuoteNumber,   bMinerNumber , DIALOGUE_EXTERNAL_NPC_UI , FALSE , FALSE , DIALOGUE_SPECIAL_EVENT_MINESECTOREVENT, START_RED_SECTOR_LOCATOR, 1 );
		CharacterDialogue( ( UINT8 )uiExternalFaceProfileIds[ bMinerNumber ], sQuoteNumber, ( UINT8 )( UINT8 )uiExternalStaticNPCFaces[ bMinerNumber ], DIALOGUE_EXTERNAL_NPC_UI,  FALSE , FALSE );
		CharacterDialogueWithSpecialEvent( ( UINT8 )uiExternalFaceProfileIds[ bMinerNumber ], sQuoteNumber,   bMinerNumber , DIALOGUE_EXTERNAL_NPC_UI , FALSE , FALSE , DIALOGUE_SPECIAL_EVENT_MINESECTOREVENT, STOP_RED_SECTOR_LOCATOR, 1 );
	}
	else	// stay in tactical
	{
		// no need to to highlight mine sector
		CharacterDialogue( ( UINT8 )uiExternalFaceProfileIds[ bMinerNumber ], sQuoteNumber, ( UINT8 )( UINT8 )uiExternalStaticNPCFaces[ bMinerNumber ], DIALOGUE_EXTERNAL_NPC_UI,  FALSE , FALSE );
	}
}


void SetUpAnimationOfMineSectors( INT32 iEvent )
{
	// set up the animation of mine sectors
	switch( iEvent )
	{
		case START_RED_SECTOR_LOCATOR:
			gubBlitSectorLocatorCode = LOCATOR_COLOR_RED;
			break;

		case START_YELLOW_SECTOR_LOCATOR:
			gubBlitSectorLocatorCode = LOCATOR_COLOR_YELLOW;
			break;

		case STOP_RED_SECTOR_LOCATOR:
		case STOP_YELLOW_SECTOR_LOCATOR:
			TurnOffSectorLocator();
			break;
	}
}

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


// thsi will setup the fast help text regions that are unrelated to mouse regions
// user is to pass in the x,y position of the box, the width to wrap the strings and the string itself
BOOLEAN SetUpFastHelpListRegions( INT32 iXPosition[], INT32 iYPosition[], INT32 iWidth[], const wchar_t *sString[], INT32 iSize )
{
	INT32 iCounter = 0;

	// reset the size
	giSizeOfInterfaceFastHelpTextList = 0;

	for( iCounter = 0; iCounter < iSize; iCounter++ )
	{

		// forgiving way of making sure we don't go too far
		CHECKF( iCounter < MAX_MAPSCREEN_FAST_HELP );

		// now copy over info
		pFastHelpMapScreenList[ iCounter ].iX = iXPosition[ iCounter ];
		pFastHelpMapScreenList[ iCounter ].iY = iYPosition[ iCounter ];
		pFastHelpMapScreenList[ iCounter ].iW = iWidth[ iCounter ];

		// copy string
		wcscpy( pFastHelpMapScreenList[ iCounter ].FastHelpText, sString[ iCounter ] );

		// update the size
		giSizeOfInterfaceFastHelpTextList = iCounter + 1;
	}

	return( TRUE );
}


// handle the actual showing of the interface fast help text
void HandleShowingOfTacticalInterfaceFastHelpText( void )
{
	static BOOLEAN fTextActive = FALSE;

	if( fInterfaceFastHelpTextActive )
	{
		DisplayFastHelpRegions( pFastHelpMapScreenList, giSizeOfInterfaceFastHelpTextList );

		PauseGame();

		// lock out the screen
		SetUpShutDownMapScreenHelpTextScreenMask( );

		gfIgnoreScrolling = TRUE;

		// the text is active
		fTextActive = TRUE;

	}
	else if( ( fInterfaceFastHelpTextActive == FALSE ) && ( fTextActive ) )
	{
		fTextActive = FALSE;
		UnPauseGame();
		gfIgnoreScrolling = FALSE;

		// shut down
		ShutDownUserDefineHelpTextRegions( );
	}

	return;
}

// start showing fast help text
void StartShowingInterfaceFastHelpText( void )
{
	fInterfaceFastHelpTextActive = TRUE;
}

// stop showing interface fast help text
void StopShowingInterfaceFastHelpText( void )
{
	fInterfaceFastHelpTextActive = FALSE;
}


// is the interface text up?
BOOLEAN IsTheInterfaceFastHelpTextActive( void )
{
	return( fInterfaceFastHelpTextActive );
}


// display all the regions in the list
void DisplayFastHelpRegions( FASTHELPREGION *pRegion, INT32 iSize )
{
	INT32 iCounter = 0;

	// run through and show all the regions
	for( iCounter = 0; iCounter < iSize; iCounter++ )
	{
		DisplayUserDefineHelpTextRegions( &( pRegion[ iCounter ] ) );
	}

	return;
}

// show one region
void DisplayUserDefineHelpTextRegions( FASTHELPREGION *pRegion )
{
	UINT16 usFillColor;
	INT32 iX,iY,iW,iH;
	INT32 iNumberOfLines = 1;
	UINT8 *pDestBuf;
	UINT32 uiDestPitchBYTES;


	// grab the color for the background region
	usFillColor = Get16BPPColor(FROMRGB(250, 240, 188));


	iX = pRegion->iX;
	iY = pRegion->iY;
	// get the width and height of the string
	iW = (INT32)( pRegion->iW ) + 14;
	iH = IanWrappedStringHeight( ( UINT16 )iX, ( UINT16 )iY, ( UINT16 )( pRegion->iW ), 0, FONT10ARIAL, FONT_BLACK, pRegion->FastHelpText, FONT_BLACK, TRUE, 0 );

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

	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );
	RectangleDraw( TRUE, iX + 1, iY + 1, iX + iW - 1, iY + iH - 1, Get16BPPColor( FROMRGB( 65, 57, 15 ) ), pDestBuf );
	RectangleDraw( TRUE, iX, iY, iX + iW - 2, iY + iH - 2, Get16BPPColor( FROMRGB( 227, 198, 88 ) ), pDestBuf );
	UnLockVideoSurface( FRAME_BUFFER );
	ShadowVideoSurfaceRect( FRAME_BUFFER, iX + 2, iY + 2, iX + iW - 3, iY + iH - 3 );
	ShadowVideoSurfaceRect( FRAME_BUFFER, iX + 2, iY + 2, iX + iW - 3, iY + iH - 3 );

	// fillt he video surface areas
	//ColorFillVideoSurfaceArea(FRAME_BUFFER, iX, iY, (iX + iW), (iY + iH), 0);
	//ColorFillVideoSurfaceArea(FRAME_BUFFER, (iX + 1), (iY + 1), (iX + iW - 1), (iY + iH - 1), usFillColor);

	SetFont( FONT10ARIAL );
	SetFontForeground( FONT_BEIGE );

	iH = ( INT32 )DisplayWrappedString( ( INT16 )( iX + 10 ), ( INT16 )( iY + 6 ), ( INT16 )pRegion->iW, 0, FONT10ARIAL, FONT_BEIGE, pRegion->FastHelpText, FONT_NEARBLACK, TRUE, 0 );

	iHeightOfInitFastHelpText = iH + 20;

	InvalidateRegion(  iX, iY, (iX + iW) , (iY + iH + 20 ) );

}

void DisplayFastHelpForInitialTripInToMapScreen(  FASTHELPREGION *pRegion )
{




	if( gTacticalStatus.fDidGameJustStart )
	{
		if( AnyMercsHired() == FALSE )
		{
			return;
		}

		HandleDisplayOfExitToTacticalMessageForFirstEntryToMapScreen( );
// DEF: removed cause the help screen will replace the help screen
//		DisplayFastHelpRegions( &pFastHelpMapScreenList[ 9 ], 1 );

	}
	else
	{
		DisplayFastHelpRegions( pFastHelpMapScreenList, giSizeOfInterfaceFastHelpTextList );
	}

	SetUpShutDownMapScreenHelpTextScreenMask( );

	return;
}


void DisplayMapScreenFastHelpList( void )
{
	INT32 iCounter = 0;

	DisplayFastHelpForInitialTripInToMapScreen( &pFastHelpMapScreenList[ iCounter ] );

	return;
}


void SetUpMapScreenFastHelpText( void )
{
	INT32 iCounter = 0;

	// now run through and display all the fast help text for the mapscreen functional regions
	for( iCounter = 0; iCounter < NUMBER_OF_MAPSCREEN_HELP_MESSAGES; iCounter++ )
	{
		pFastHelpMapScreenList[ iCounter ].iX = pMapScreenFastHelpLocationList[ iCounter ].iX;
		pFastHelpMapScreenList[ iCounter ].iY = pMapScreenFastHelpLocationList[ iCounter ].iY;
		pFastHelpMapScreenList[ iCounter ].iW = pMapScreenFastHelpWidthList[ iCounter ];
		wcscpy( pFastHelpMapScreenList[ iCounter ].FastHelpText, pMapScreenFastHelpTextList[ iCounter ] );
	}

// DEF: removed cause the help screen will replace the help screen
/*
	pFastHelpMapScreenList[ 9 ].iX = pMapScreenFastHelpLocationList[ 9 ].iX;
	pFastHelpMapScreenList[ 9 ].iY = pMapScreenFastHelpLocationList[ 9 ].iY;
	pFastHelpMapScreenList[ 9 ].iW = pMapScreenFastHelpWidthList[ 9 ];
	wcscpy( pFastHelpMapScreenList[ 9 ].FastHelpText, pMapScreenFastHelpTextList[ 9 ] );
*/
	return;
}

void StopMapScreenHelpText( void )
{
	fShowMapScreenHelpText = FALSE;
	fTeamPanelDirty = TRUE;
	fMapPanelDirty = TRUE;
	fCharacterInfoPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;

	SetUpShutDownMapScreenHelpTextScreenMask( );
	return;
}

BOOLEAN IsMapScreenHelpTextUp( void )
{
	return( fShowMapScreenHelpText );
}

void SetUpShutDownMapScreenHelpTextScreenMask( void )
{
	static BOOLEAN fCreated = FALSE;

	// create or destroy the screen mask as needed
	if( ( ( fShowMapScreenHelpText == TRUE ) || ( fInterfaceFastHelpTextActive == TRUE ) ) && ( fCreated == FALSE ) )
	{
		if( gTacticalStatus.fDidGameJustStart )
		{
			MSYS_DefineRegion( &gMapScreenHelpTextMask , ( INT16 )( pMapScreenFastHelpLocationList[ 9 ].iX ), ( INT16 )( pMapScreenFastHelpLocationList[ 9 ].iY ), ( INT16 )( pMapScreenFastHelpLocationList[ 9 ].iX + pMapScreenFastHelpWidthList[ 9 ] ), ( INT16 )( pMapScreenFastHelpLocationList[ 9 ].iY + iHeightOfInitFastHelpText ), MSYS_PRIORITY_HIGHEST,
				MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MapScreenHelpTextScreenMaskBtnCallback );
		}
		else
		{
				MSYS_DefineRegion( &gMapScreenHelpTextMask , 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST,
					MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MapScreenHelpTextScreenMaskBtnCallback );
		}

		fCreated = TRUE;

	}
	else if( ( fShowMapScreenHelpText == FALSE ) && ( fInterfaceFastHelpTextActive == FALSE ) && ( fCreated == TRUE ) )
	{
		MSYS_RemoveRegion( &gMapScreenHelpTextMask );

		fCreated = FALSE;
	}
}


void MapScreenHelpTextScreenMaskBtnCallback( MOUSE_REGION * pRegion, INT32 iReason )
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


BOOLEAN IsSoldierSelectedForMovement( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;

	// run through the list and turn this soldiers value on
	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( ( pSoldierMovingList[ iCounter ] == pSoldier ) && ( fSoldierIsMoving[ iCounter ] ) )
		{
			return ( TRUE );
		}
	}
	return( FALSE );
}

BOOLEAN IsSquadSelectedForMovement( INT32 iSquadNumber )
{
	INT32 iCounter = 0;

	// run through squad list and set them on
	for( iCounter = 0; iCounter < giNumberOfSquadsInSectorMoving; iCounter++ )
	{
		if( ( iSquadMovingList[ iCounter ] == iSquadNumber ) && ( fSquadIsMoving[ iCounter ] ) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


BOOLEAN IsVehicleSelectedForMovement( INT32 iVehicleId )
{
	INT32 iCounter = 0;

	// run through squad list and set them on
	for( iCounter = 0; iCounter < giNumberOfVehiclesInSectorMoving; iCounter++ )
	{
		if( ( iVehicleMovingList[ iCounter ] == iVehicleId ) && ( fVehicleIsMoving[ iCounter ] ) )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}



void SelectSoldierForMovement( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;

	if( pSoldier == NULL )
	{
		return;
	}

	// run through the list and turn this soldiers value on
	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( pSoldierMovingList[ iCounter ] == pSoldier )
		{
			// turn the selected soldier ON
			fSoldierIsMoving[ iCounter ] = TRUE;
			break;
		}
	}
}

void DeselectSoldierForMovement( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;

	if( pSoldier == NULL )
	{
		return;
	}

	// run through the list and turn this soldier's value on
	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( pSoldierMovingList[ iCounter ] == pSoldier )
		{
			// turn the selected soldier off
			fSoldierIsMoving[ iCounter ] = FALSE;
			break;
		}
	}
}


void SelectSquadForMovement( INT32 iSquadNumber )
{
	INT32 iCounter = 0, iCount = 0;
	BOOLEAN fSomeCantMove = FALSE;
	SOLDIERTYPE *pSoldier = NULL;
	BOOLEAN fFirstFailure;


	// run through squad list and set them on
	for( iCounter = 0; iCounter < giNumberOfSquadsInSectorMoving; iCounter++ )
	{
		if( iSquadMovingList[ iCounter ] == iSquadNumber )
		{
			// found it

			fFirstFailure = TRUE;

			// try to select everyone in squad
			for( iCount = 0; iCount < NUMBER_OF_SOLDIERS_PER_SQUAD; iCount++ )
			{
				pSoldier = Squad[ iSquadNumber ][ iCount ];

				if ( pSoldier && pSoldier->bActive )
				{
					// is he able & allowed to move?  (Report only the first reason for failure encountered)
					if ( CanMoveBoxSoldierMoveStrategically( pSoldier, fFirstFailure ) )
					{
						SelectSoldierForMovement( pSoldier );
					}
					else
					{
						fSomeCantMove = TRUE;
						fFirstFailure = FALSE;
					}
				}
			}

			if ( !fSomeCantMove )
			{
				fSquadIsMoving[ iCounter ] = TRUE;
			}

			break;
		}
	}
}



void DeselectSquadForMovement( INT32 iSquadNumber )
{
	INT32 iCounter = 0, iCount = 0;
	SOLDIERTYPE *pSoldier = NULL;

	// run through squad list and set them off
	for( iCounter = 0; iCounter < giNumberOfSquadsInSectorMoving; iCounter++ )
	{
		if( iSquadMovingList[ iCounter ] == iSquadNumber )
		{
			// found it
			fSquadIsMoving[ iCounter ] = FALSE;

			// now deselect everyone in squad
			for( iCount = 0; iCount < NUMBER_OF_SOLDIERS_PER_SQUAD; iCount++ )
			{
				pSoldier = Squad[ iSquadNumber ][ iCount ];

				if ( pSoldier && pSoldier->bActive )
				{
					DeselectSoldierForMovement( pSoldier );
				}
			}

			break;
		}
	}
}


BOOLEAN AllSoldiersInSquadSelected( INT32 iSquadNumber )
{
	INT32 iCounter = 0, iCount = 0;

	// is everyone on this squad moving?
	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( pSoldierMovingList[ iCounter ]->bAssignment == ( INT8 )iSquadNumber )
		{
			if( fSoldierIsMoving[ iCounter ] == FALSE )
			{
				return( FALSE );
			}
		}
	}

	return( TRUE );
}



void SelectVehicleForMovement( INT32 iVehicleId, BOOLEAN fAndAllOnBoard )
{
	INT32 iCounter = 0, iCount = 0;
	SOLDIERTYPE *pPassenger = NULL;
	BOOLEAN fHasDriver = FALSE;
	BOOLEAN fFirstFailure;


	// run through vehicle list and set them on
	for( iCounter = 0; iCounter < giNumberOfVehiclesInSectorMoving; iCounter++ )
	{
		if( iVehicleMovingList[ iCounter ] == iVehicleId )
		{
			// found it

			fFirstFailure = TRUE;

			for( iCount = 0; iCount < 10; iCount++ )
			{
				pPassenger = pVehicleList[ iVehicleId ].pPassengers[ iCount ];

				if ( fAndAllOnBoard )
				{
					// try to select everyone in vehicle

					if ( pPassenger && pPassenger->bActive )
					{
						// is he able & allowed to move?
						if ( CanMoveBoxSoldierMoveStrategically( pPassenger, fFirstFailure ) )
						{
							SelectSoldierForMovement( pPassenger );
						}
						else
						{
							fFirstFailure = FALSE;
						}
					}
				}

				if( IsSoldierSelectedForMovement( pPassenger ) )
				{
					fHasDriver = TRUE;
				}
			}

			// vehicle itself can only move if at least one passenger can move and is moving!
			if ( fHasDriver )
			{
				fVehicleIsMoving[ iCounter ] = TRUE;
			}

			break;
		}
	}
}

void DeselectVehicleForMovement( INT32 iVehicleId )
{
	INT32 iCounter = 0, iCount = 0;
	SOLDIERTYPE *pPassenger = NULL;


	// run through vehicle list and set them off
	for( iCounter = 0; iCounter < giNumberOfVehiclesInSectorMoving; iCounter++ )
	{
		if( iVehicleMovingList[ iCounter ] == iVehicleId )
		{
			// found it
			fVehicleIsMoving[ iCounter ] = FALSE;

			// now deselect everyone in vehicle
			for( iCount = 0; iCount < 10 ; iCount++ )
			{
				pPassenger = pVehicleList[ iVehicleId ].pPassengers[ iCount ];

				if ( pPassenger && pPassenger->bActive )
				{
					DeselectSoldierForMovement( pPassenger );
				}
			}

			break;
		}
	}
}


INT32 HowManyMovingSoldiersInVehicle( INT32 iVehicleId )
{
	INT32 iNumber = 0, iCounter = 0;

	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		// is he in the right vehicle
		if( ( pSoldierMovingList[ iCounter ] ->bAssignment == VEHICLE )&&( pSoldierMovingList[ iCounter ] ->iVehicleId == iVehicleId ) )
		{
			// if he moving?
			if ( fSoldierIsMoving[ iCounter ] )
			{
				// ok, another one in the vehicle that is going to move
				iNumber++;
			}
		}
	}

	return( iNumber );
}

INT32 HowManyMovingSoldiersInSquad( INT32 iSquadNumber )
{
	INT32 iNumber = 0, iCounter = 0;

	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		// is he in the right squad
		if( pSoldierMovingList[ iCounter ] ->bAssignment == iSquadNumber )
		{
			// if he moving?
			if ( fSoldierIsMoving[ iCounter ] )
			{
				// ok, another one in the squad that is going to move
				iNumber++;
			}
		}
	}

	return( iNumber );
}


// try to add this soldier to the moving lists
void AddSoldierToMovingLists( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( pSoldierMovingList[ iCounter ] == pSoldier )
		{
			// found
			return;
		}
		else if( pSoldierMovingList[ iCounter ] == NULL )
		{
			// found a free slot
			pSoldierMovingList[ iCounter ] = pSoldier;
			fSoldierIsMoving[ iCounter ] = FALSE;

			giNumberOfSoldiersInSectorMoving++;
			return;
		}
	}
	return;
}


// try to add this soldier to the moving lists
void AddSquadToMovingLists( INT32 iSquadNumber )
{
	INT32 iCounter = 0;

	if( iSquadNumber == -1 )
	{
		// invalid squad
		return;
	}

	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		if( iSquadMovingList[ iCounter ] == iSquadNumber )
		{
			// found
			return;
		}
		if( iSquadMovingList[ iCounter ] == -1 )
		{
			// found a free slot
			iSquadMovingList[ iCounter ] = iSquadNumber;
			fSquadIsMoving[ iCounter ] = FALSE;

			giNumberOfSquadsInSectorMoving++;
			return;
		}
	}
	return;
}


// try to add this soldier to the moving lists
void AddVehicleToMovingLists( INT32 iVehicleId )
{
	INT32 iCounter = 0;

	if( iVehicleId == -1 )
	{
		// invalid squad
		return;
	}

	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		if( iVehicleMovingList[ iCounter ] == iVehicleId )
		{
			// found
			return;
		}
		if( iVehicleMovingList[ iCounter ] == -1 )
		{
			// found a free slot
			iVehicleMovingList[ iCounter ] = iVehicleId;
			fVehicleIsMoving[ iCounter ] = FALSE;

			giNumberOfVehiclesInSectorMoving++;
			return;
		}
	}
	return;
}



void InitializeMovingLists( void )
{
	INT32 iCounter = 0;


	giNumberOfSoldiersInSectorMoving = 0;
	giNumberOfSquadsInSectorMoving   = 0;
	giNumberOfVehiclesInSectorMoving = 0;

	// init the soldiers
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		// soldier is NOT moving
		pSoldierMovingList[ iCounter ] = NULL;
		// turn the selected soldier off
		fSoldierIsMoving[ iCounter ] = FALSE;
	}


	// init the squads
	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		// reset squad value
		iSquadMovingList[ iCounter ] = -1;
		// turn it off
		fSquadIsMoving[ iCounter ] = FALSE;
	}

	// init the vehicles
	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		// reset squad value
		iVehicleMovingList[ iCounter ] = -1;
		// turn it off
		fVehicleIsMoving[ iCounter ] = FALSE;
	}

	return;
}



BOOLEAN	IsAnythingSelectedForMoving( void )
{
	INT32 iCounter = 0;


	// check soldiers
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if ( ( pSoldierMovingList[ iCounter ] != NULL ) && fSoldierIsMoving[ iCounter ] )
		{
			return( TRUE );
		}
	}


	// init the squads
	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		if ( ( iSquadMovingList[ iCounter ] != -1 ) && fSquadIsMoving[ iCounter ] )
		{
			return( TRUE );
		}
	}

	// init the vehicles
	for( iCounter = 0; iCounter < NUMBER_OF_SQUADS; iCounter++ )
	{
		if ( ( iVehicleMovingList[ iCounter ] != -1 ) && fVehicleIsMoving[ iCounter ] )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}



void CreateDestroyMovementBox( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	static BOOLEAN fCreated = FALSE;


	// not allowed for underground movement!
	Assert( sSectorZ == 0 );

	if( ( fShowMapScreenMovementList == TRUE ) && ( fCreated == FALSE ) )
	{
		fCreated = TRUE;

		// create the box and mouse regions
		CreatePopUpBoxForMovementBox( );
		BuildMouseRegionsForMoveBox( );
		CreateScreenMaskForMoveBox( );
  	fMapPanelDirty = TRUE;
	}
	else if( ( fShowMapScreenMovementList == FALSE ) && ( fCreated == TRUE ) )
	{
		fCreated = FALSE;

		// destroy the box and mouse regions
		ClearMouseRegionsForMoveBox( );
		RemoveBox( ghMoveBox );
		ghMoveBox = -1;
		RemoveScreenMaskForMoveBox( );
  	fMapPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;		// really long move boxes can overlap bottom panel
	}

}



void SetUpMovingListsForSector( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;


	// not allowed for underground movement!
	Assert( sSectorZ == 0 );


	// clear the lists
	InitializeMovingLists();


	// note that Skyrider can't be moved using the move box, and won't appear because the helicoprer is not in the char list

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

			if( ( pSoldier->bActive ) &&
					( pSoldier->bAssignment != IN_TRANSIT ) && ( pSoldier->bAssignment != ASSIGNMENT_POW ) &&
					( pSoldier->sSectorX == sSectorX ) && ( pSoldier->sSectorY == sSectorY ) && ( pSoldier->bSectorZ == sSectorZ ) )
			{
				if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
				{
					// vehicle
					// if it can move (can't be empty)
					if ( GetNumberInVehicle( pSoldier->bVehicleID ) > 0 )
					{
						// add vehicle
						AddVehicleToMovingLists( pSoldier->bVehicleID );
					}
				}
				else // soldier
				{
					// alive, not aboard Skyrider (airborne or not!)
					if ( ( pSoldier->bLife >= OKLIFE ) &&
							 ( ( pSoldier->bAssignment != VEHICLE ) || ( pSoldier->iVehicleId != iHelicopterVehicleId ) ) )
					{
						// add soldier
						AddSoldierToMovingLists( pSoldier );

						// if on a squad,
						if ( pSoldier->bAssignment < ON_DUTY )
						{
							// add squad (duplicates ok, they're ignored inside the function)
							AddSquadToMovingLists( pSoldier->bAssignment );
						}
					}
				}
			}
		}
	}

	fShowMapScreenMovementList = TRUE;
	CreateDestroyMovementBox( sSectorX, sSectorY, sSectorZ );
}



void CreatePopUpBoxForMovementBox( void )
{
	SGPPoint Position;
	SGPRect Dimensions;

	// create the pop up box and mouse regions for movement list

 // create basic box
 CreatePopUpBox(&ghMoveBox, AssignmentDimensions, MovePosition, (POPUP_BOX_FLAG_CLIP_TEXT|POPUP_BOX_FLAG_RESIZE ));

 // which buffer will box render to
 SetBoxBuffer(ghMoveBox, FRAME_BUFFER);

 // border type?
 SetBorderType(ghMoveBox,guiPOPUPBORDERS);

 // background texture
 SetBackGroundSurface(ghMoveBox, guiPOPUPTEX);

 // margin sizes
 SetMargins( ghMoveBox, 6, 6, 4, 4 );

 // space between lines
 SetLineSpace(ghMoveBox, 2);

 // set current box to this one
 SetCurrentBox( ghMoveBox );

 // add strings
 AddStringsToMoveBox( );


 // set font type
 SetBoxFont(ghMoveBox, MAP_SCREEN_FONT);

 // set highlight color
 SetBoxHighLight(ghMoveBox, FONT_WHITE);

 // unhighlighted color
 SetBoxForeground(ghMoveBox, FONT_LTGREEN);

 // make the header line WHITE
 SetBoxLineForeground( ghMoveBox, 0, FONT_WHITE );

 // make the done and cancel lines YELLOW
 SetBoxLineForeground( ghMoveBox, GetNumberOfLinesOfTextInBox( ghMoveBox ) - 1, FONT_YELLOW );

	if ( IsAnythingSelectedForMoving() )
	{
		SetBoxLineForeground( ghMoveBox, GetNumberOfLinesOfTextInBox( ghMoveBox ) - 2, FONT_YELLOW );
	}

 // background color
 SetBoxBackground(ghMoveBox, FONT_BLACK);

 // shaded color..for darkened text
 SetBoxShade( ghMoveBox, FONT_BLACK );


 // resize box to text
 ResizeBoxToText( ghMoveBox );

	GetBoxPosition( ghMoveBox, &Position);
	GetBoxSize( ghMoveBox, &Dimensions );

	// adjust position to try to keep it in the map area as best as possible
	if ( Position.iX + Dimensions.iRight >= ( MAP_VIEW_START_X + MAP_VIEW_WIDTH ) )
	{
		Position.iX = max( MAP_VIEW_START_X, ( MAP_VIEW_START_X + MAP_VIEW_WIDTH ) - Dimensions.iRight );
		SetBoxPosition( ghMoveBox, Position );
	}

	if ( Position.iY + Dimensions.iBottom >= ( MAP_VIEW_START_Y + MAP_VIEW_HEIGHT ) )
	{
		Position.iY = max( MAP_VIEW_START_Y, ( MAP_VIEW_START_Y + MAP_VIEW_HEIGHT ) - Dimensions.iBottom );
		SetBoxPosition( ghMoveBox, Position );
	}
}



void AddStringsToMoveBox( void )
{
	INT32 iCount = 0, iCountB = 0;
	CHAR16 sString[ 128 ], sStringB[ 128 ];
	UINT32 hStringHandle;
	BOOLEAN fFirstOne = TRUE;


	// set the current box
	SetCurrentBox( ghMoveBox );

	// clear all the strings out of the box
	RemoveAllCurrentBoxStrings();


	// add title
	GetShortSectorString( sSelMapX, sSelMapY, sStringB, lengthof(sStringB));
	swprintf( sString, lengthof(sString), L"%S %S", pMovementMenuStrings[ 0 ], sStringB );
	AddMonoString(&hStringHandle, sString );


	// blank line
	AddMonoString(&hStringHandle, L"" );


	// add squads
	for( iCount = 0; iCount < giNumberOfSquadsInSectorMoving; iCount++ )
	{
		// add this squad, now add all the grunts in it
		if( fSquadIsMoving[ iCount ] )
		{
			swprintf( sString, lengthof(sString), L"*%S*", pSquadMenuStrings[iSquadMovingList[ iCount ] ] );
		}
		else
		{
			swprintf( sString, lengthof(sString), L"%S", pSquadMenuStrings[iSquadMovingList[ iCount ] ] );
		}
		AddMonoString(&hStringHandle, sString );

		// now add all the grunts in it
		for( iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; iCountB++ )
		{
			if( pSoldierMovingList[ iCountB ] -> bAssignment == iSquadMovingList[ iCount ] )
			{
				// add mercs in squads
				if( IsSoldierSelectedForMovement( pSoldierMovingList[ iCountB ] ) == TRUE )
				{
					swprintf( sString, lengthof(sString), L"   *%S*", pSoldierMovingList[ iCountB ]->name );
				}
				else
				{
					swprintf( sString, lengthof(sString), L"   %S", pSoldierMovingList[ iCountB ]->name );
				}
				AddMonoString(&hStringHandle, sString );
			}
		}
	}


	// add vehicles
	for( iCount = 0; iCount < giNumberOfVehiclesInSectorMoving; iCount++ )
	{
		// add this vehicle
		if( fVehicleIsMoving[ iCount ] )
		{
			swprintf( sString, lengthof(sString), L"*%S*", pVehicleStrings[ pVehicleList[ iVehicleMovingList[ iCount ] ].ubVehicleType ] );
		}
		else
		{
			swprintf( sString, lengthof(sString), L"%S", pVehicleStrings[ pVehicleList[ iVehicleMovingList[ iCount ]  ].ubVehicleType ] );
		}
		AddMonoString(&hStringHandle, sString );

		// now add all the grunts in it
		for( iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; iCountB++ )
		{
			if( ( pSoldierMovingList[ iCountB ] -> bAssignment == VEHICLE ) &&( pSoldierMovingList[ iCountB ] -> iVehicleId == iVehicleMovingList[ iCount ] ) )
			{
				// add mercs in vehicles
				if( IsSoldierSelectedForMovement( pSoldierMovingList[ iCountB ] ) == TRUE )
				{
					swprintf( sString, lengthof(sString), L"   *%S*", pSoldierMovingList[ iCountB ]->name );
				}
				else
				{
					swprintf( sString, lengthof(sString), L"   %S", pSoldierMovingList[ iCountB ]->name );
				}
				AddMonoString(&hStringHandle, sString );
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
				if( AllOtherSoldiersInListAreSelected( ) )
				{
					swprintf( sString, lengthof(sString), L"*%S*", pMovementMenuStrings[ 3 ] );
				}
				else
				{
					swprintf( sString, lengthof(sString), L"%S", pMovementMenuStrings[ 3 ] );
				}
				AddMonoString(&hStringHandle, sString );

				fFirstOne = FALSE;
			}

			// add OTHER soldiers (not on duty nor in a vehicle)
			if( IsSoldierSelectedForMovement( pSoldierMovingList[ iCount ] ) == TRUE )
			{
				swprintf( sString, lengthof(sString), L"  *%S ( %S )*", pSoldierMovingList[ iCount ]->name, pAssignmentStrings[  pSoldierMovingList[ iCount ]->bAssignment ] );
			}
			else
			{
				swprintf( sString, lengthof(sString), L"   %S ( %S )", pSoldierMovingList[ iCount ]->name, pAssignmentStrings[  pSoldierMovingList[ iCount ]->bAssignment ] );
			}
			AddMonoString(&hStringHandle, sString );
		}
	}


	// blank line
	AddMonoString(&hStringHandle, L"" );


	if ( IsAnythingSelectedForMoving() )
	{
		// add PLOT MOVE line
		swprintf( sString, lengthof(sString), L"%S", pMovementMenuStrings[ 1 ] );
		AddMonoString(&hStringHandle, sString );
	}
	else
	{
		// blank line
		AddMonoString(&hStringHandle, L"" );
	}


	// add cancel line
	swprintf( sString, lengthof(sString), L"%S", pMovementMenuStrings[ 2 ] );
	AddMonoString(&hStringHandle, sString );

	return;
}


void BuildMouseRegionsForMoveBox( void )
{
	INT32 iCounter = 0, iTotalNumberOfLines = 0, iCount = 0, iCountB = 0;
	SGPPoint pPosition;
	INT32 iBoxWidth = 0;
	SGPRect Dimensions;
	INT32 iFontHeight = 0;
	INT32 iBoxXPosition = 0;
	INT32 iBoxYPosition = 0;
	BOOLEAN fDefinedOtherRegion = FALSE;


	// grab height of font
	iFontHeight = GetLineSpace( ghMoveBox ) + GetFontHeight( GetBoxFont( ghMoveBox ) );

	// get x.y position of box
	GetBoxPosition( ghMoveBox, &pPosition);

	// grab box x and y position
	iBoxXPosition = pPosition.iX;
	iBoxYPosition = pPosition.iY + GetTopMarginSize( ghMoveBox ) - 2;	// -2 to improve highlighting accuracy between lines


	// get dimensions..mostly for width
	GetBoxSize( ghMoveBox, &Dimensions );

	// get width
	iBoxWidth = Dimensions.iRight;

	SetCurrentBox( ghMoveBox );


	// box heading
	MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );
	iCounter++;

	// blank line
	MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
						 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );
	iCounter++;

	// calc total number of "moving" lines in the box
	iTotalNumberOfLines = giNumberOfSoldiersInSectorMoving + giNumberOfSquadsInSectorMoving + giNumberOfVehiclesInSectorMoving;
	// add the blank lines
	iTotalNumberOfLines += iCounter;


	// now add the strings
	while( iCounter < iTotalNumberOfLines )
	{
		// define regions for squad lines
		for( iCount = 0; iCount < giNumberOfSquadsInSectorMoving; iCount++ )
		{
			MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

			// set user defines
			MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
			MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, SQUAD_REGION );
			MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, iCount );
			iCounter++;

			for( iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; iCountB++ )
			{
				if( pSoldierMovingList[ iCountB ] -> bAssignment == iSquadMovingList[ iCount ] )
				{
					MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

					// set user defines
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, SOLDIER_REGION );
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, iCountB );
					iCounter++;
				}
			}
		}

		for( iCount = 0; iCount < giNumberOfVehiclesInSectorMoving; iCount++ )
		{
			// define regions for vehicle lines
			MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

			// set user defines
			MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
			MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, VEHICLE_REGION );
			MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, iCount );
			iCounter++;

			for( iCountB = 0; iCountB < giNumberOfSoldiersInSectorMoving; iCountB++ )
			{
				if( ( pSoldierMovingList[ iCountB ] -> bAssignment == VEHICLE ) &&( pSoldierMovingList[ iCountB ] -> iVehicleId == iVehicleMovingList[ iCount ] ) )
				{
					MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

					// set user defines
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, SOLDIER_REGION );
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, iCountB );
					iCounter++;
				}
			}
		}


		// define regions for "other" soldiers
		for( iCount = 0; iCount < giNumberOfSoldiersInSectorMoving; iCount++ )
		{
			// this guy is not in a squad or vehicle
			if( ( pSoldierMovingList[ iCount ]->bAssignment >= ON_DUTY )&&( pSoldierMovingList[ iCount ]->bAssignment != VEHICLE ) )
			{
				// this line gets place only once...
				if( !fDefinedOtherRegion )
				{
					MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

					// set user defines
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, OTHER_REGION );
					MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, 0 );
					iCounter++;

					fDefinedOtherRegion = TRUE;
				}

				MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
					MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

				// set user defines
				MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
				MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, SOLDIER_REGION );
				MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, iCount );
				iCounter++;
			}
		}
	}


	// blank line
	MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
						 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );
	iCounter++;


	if ( IsAnythingSelectedForMoving() )
	{
		// DONE line
		MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

		// set user defines
		MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
		MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, DONE_REGION );
		MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, 0 );
		iCounter++;
	}
	else
	{
		// blank line
		MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );
		iCounter++;
	}

	// CANCEL line
	MSYS_DefineRegion( &gMoveMenuRegion[ iCounter ], 	( INT16 )( iBoxXPosition ), ( INT16 )( iBoxYPosition + iFontHeight * iCounter ), ( INT16 )( iBoxXPosition + iBoxWidth ), ( INT16 )( iBoxYPosition + iFontHeight * ( iCounter + 1 ) ), MSYS_PRIORITY_HIGHEST,
						 MSYS_NO_CURSOR, MoveMenuMvtCallback, MoveMenuBtnCallback );

	// set user defines
	MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 0, iCounter );
	MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 1, CANCEL_REGION );
	MSYS_SetRegionUserData( &gMoveMenuRegion[ iCounter ], 2, 0 );
	iCounter++;
}

void ClearMouseRegionsForMoveBox( void )
{
	INT32 iCounter = 0;

	// run through list of mouse regions
	for( iCounter = 0; iCounter < ( INT32 )GetNumberOfLinesOfTextInBox( ghMoveBox ); iCounter++ )
	{
		// remove this region
		MSYS_RemoveRegion( &gMoveMenuRegion[ iCounter ] );
	}

	return;
}



void MoveMenuMvtCallback(MOUSE_REGION * pRegion, INT32 iReason )
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



void MoveMenuBtnCallback(MOUSE_REGION * pRegion, INT32 iReason )
{
	// btn callback handler for move box line regions
	INT32 iMoveBoxLine = -1, iRegionType = -1, iListIndex = -1, iClickTime = 0;
	SOLDIERTYPE *pSoldier = NULL;


	iMoveBoxLine = MSYS_GetRegionUserData( pRegion, 0 );
	iRegionType  = MSYS_GetRegionUserData( pRegion, 1 );
	iListIndex   = MSYS_GetRegionUserData( pRegion, 2 );
	iClickTime	 = GetJA2Clock();

	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )  )
	{
		if( iClickTime - giDblClickTimersForMoveBoxMouseRegions[ iMoveBoxLine ] < DBL_CLICK_DELAY_FOR_MOVE_MENU  )
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
			giDblClickTimersForMoveBoxMouseRegions[ iMoveBoxLine ] = iClickTime;

			if( iRegionType == SQUAD_REGION )
			{
				// is the squad moving
				if( fSquadIsMoving[ iListIndex ] == TRUE )
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
				if( fVehicleIsMoving[ iListIndex ] == TRUE )
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
				if( AllOtherSoldiersInListAreSelected( ) == TRUE )
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
				if( IsSoldierSelectedForMovement( pSoldier ) )
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
							DeselectSoldierForMovement( pSoldier );
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
							DeselectSoldierForMovement( pSoldier );
						}
					}
					else
					{
						// soldier is staying behind
						DeselectSoldierForMovement( pSoldier );
					}
				}
				else	// currently NOT moving
				{
					// is he able & allowed to move?  (Errors with a reason are reported within)
					if ( CanMoveBoxSoldierMoveStrategically( pSoldier, TRUE ) )
					{
						// change him to move instead
						SelectSoldierForMovement( pSoldier );

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
				AssertMsg( 0, String( "MoveMenuBtnCallback: Invalid regionType %d, moveBoxLine %d", iRegionType, iMoveBoxLine ) );
				return;
			}

			fRebuildMoveBox = TRUE;
			fTeamPanelDirty = TRUE;
			fMapPanelDirty = TRUE;
			fCharacterInfoPanelDirty = TRUE;
			MarkAllBoxesAsAltered( );
		}
	}

	return;
}



BOOLEAN CanMoveBoxSoldierMoveStrategically( SOLDIERTYPE *pSoldier, BOOLEAN fShowErrorMessage )
{
	INT8 bErrorNumber = -1;

	// valid soldier?
	Assert( pSoldier );
	Assert( pSoldier->bActive );


	if( CanCharacterMoveInStrategic( pSoldier, &bErrorNumber ) )
	{
		return( TRUE );
	}
	else
	{
		// function may fail without returning any specific error # (-1).
		// if it gave us the # of an error msg, and we were told to display it
		if ( ( bErrorNumber != -1 ) && fShowErrorMessage )
		{
			ReportMapScreenMovementError( bErrorNumber );
		}

		return( FALSE );
	}
}



void SelectAllOtherSoldiersInList( void )
{
	INT32 iCounter = 0;
	BOOLEAN fSomeCantMove = FALSE;


	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( ( pSoldierMovingList[ iCounter ] ->bAssignment >= ON_DUTY ) && ( pSoldierMovingList[ iCounter ] ->bAssignment != VEHICLE ) )
		{
			if ( CanMoveBoxSoldierMoveStrategically( pSoldierMovingList[ iCounter ], FALSE ) )
			{
				fSoldierIsMoving[ iCounter ] = TRUE;
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



void DeselectAllOtherSoldiersInList( void )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( ( pSoldierMovingList[ iCounter ] ->bAssignment >= ON_DUTY ) && ( pSoldierMovingList[ iCounter ] ->bAssignment != VEHICLE ) )
		{
			fSoldierIsMoving[ iCounter ] = FALSE;
		}
	}
}



void HandleMoveoutOfSectorMovementTroops( void )
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
					AssertMsg( 0, String( "HandleMoveoutOfSectorMovementTroops: AddCharacterToSquad %d failed, iCounter %d", iSquadNumber, iCounter ) );
					// toggle whether he's going or not to try and recover somewhat gracefully
					fSoldierIsMoving[ iCounter ] = !fSoldierIsMoving[ iCounter ];
				}
			}
			else
			{
				// no existing squad is compatible, will have to start his own new squad
				iSquadNumber = AddCharacterToUniqueSquad( pSoldier );
				if ( iSquadNumber != -1 )
				{
					// It worked.  Add his new squad to the "moving squads" list so others can join it, too!
					AddSquadToMovingLists( iSquadNumber );

					// If this guy is moving
					if( fSoldierIsMoving[ iCounter ] )
					{
						// mark this new squad as moving too, so those moving can join it
						SelectSquadForMovement( iSquadNumber );
					}
				}
				else
				{
					// failed - should never happen!
					AssertMsg( 0, String( "HandleMoveoutOfSectorMovementTroops: AddCharacterToUniqueSquad failed, iCounter %d", iCounter ) );
					// toggle whether he's going or not to try and recover somewhat gracefully
					fSoldierIsMoving[ iCounter ] = !fSoldierIsMoving[ iCounter ];
				}
			}
		}

	}

	// now actually set the list
	HandleSettingTheSelectedListOfMercs( );

	return;
}


void HandleSettingTheSelectedListOfMercs( void )
{
	BOOLEAN fFirstOne = TRUE;
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;
	BOOLEAN fSelected;


	// reset the selected character
	bSelectedDestChar = -1;

	// run through the list of grunts
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		// is the current guy a valid character?
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

			if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				fSelected = IsVehicleSelectedForMovement( pSoldier->bVehicleID );
			}
			else
			{
				fSelected = IsSoldierSelectedForMovement( pSoldier );
			}

			// is he/she selected for movement?
			if( fSelected )
			{
				// yes, are they the first one to be selected?
				if( fFirstOne == TRUE )
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
	}

	if( bSelectedDestChar != -1 )
	{
		// set cursor
		SetUpCursorForStrategicMap( );
		fTeamPanelDirty = TRUE;
		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		DeselectSelectedListMercsWhoCantMoveWithThisGuy( &( Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ] ) );

		// remember the current paths for all selected characters so we can restore them if need be
		RememberPreviousPathForAllSelectedChars();
	}
}


/*
INT32 GetSquadListIndexForSquadNumber( INT32 iSquadNumber )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < giNumberOfSquadsInSectorMoving; iCounter++ )
	{
		if( iSquadMovingList[ iCounter ] == iSquadNumber )
		{
			return( iCounter );
		}
	}

	return( -1 );
}
*/


BOOLEAN AllOtherSoldiersInListAreSelected( void )
{
	INT32 iCounter = 0, iCount = 0;

	for( iCounter = 0; iCounter < giNumberOfSoldiersInSectorMoving; iCounter++ )
	{
		if( ( pSoldierMovingList[ iCounter ]->bAssignment >= ON_DUTY ) && (  pSoldierMovingList[ iCounter ]->bAssignment >= VEHICLE ) )
		{
			if( fSoldierIsMoving[ iCounter ] == FALSE )
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


BOOLEAN IsThisSquadInThisSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, INT8 bSquadValue )
{
	INT16 sX = 0, sY = 0;
	INT8 bZ = 0;

	// check if the squad is empty
	if( SquadIsEmpty( bSquadValue ) == FALSE )
	{
		// now grab the squad location
		GetLocationOfSquad( &sX, &sY, &bZ, bSquadValue );

		// check if this non-empty squad is in this sector
		if( ( sX == sSectorX ) && ( sY == sSectorY ) && ( bSectorZ == bZ ) )
		{
			// a squad that's between sectors isn't *in* this sector
			if( !IsThisSquadOnTheMove( bSquadValue ) )
			{
				// yep
				return( TRUE );
			}
		}
	}

	// nope
	return( FALSE );
}


INT8 FindSquadThatSoldierCanJoin( SOLDIERTYPE *pSoldier )
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
			if( IsThisSquadFull( bCounter ) == FALSE )
			{
				// is it doing the same thing as the soldier is (staying or going) ?
				if( IsSquadSelectedForMovement( bCounter ) == IsSoldierSelectedForMovement( pSoldier ) )
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
	if( fRebuildMoveBox == FALSE )
	{
		return;
	}

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


void CreateScreenMaskForMoveBox( void )
{
	if( fScreenMaskForMoveCreated == FALSE )
	{
		// set up the screen mask
		MSYS_DefineRegion( &gMoveBoxScreenMask, 0, 0, 640, 480, MSYS_PRIORITY_HIGHEST - 4,
		MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MoveScreenMaskBtnCallback );

		fScreenMaskForMoveCreated = TRUE;
	}
}

void RemoveScreenMaskForMoveBox( void )
{
	if( fScreenMaskForMoveCreated == TRUE )
	{
		// remove the screen mask
		MSYS_RemoveRegion( &gMoveBoxScreenMask );
		fScreenMaskForMoveCreated = FALSE;
	}
}


void MoveScreenMaskBtnCallback(MOUSE_REGION * pRegion, INT32 iReason )
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

	return;
}

void ResetSoldierUpdateBox( void )
{
	INT32 iCounter = 0;

	// delete any loaded faces
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		if( pUpdateSoldierBox[ iCounter ] != NULL )
		{
			DeleteVideoObjectFromIndex( giUpdateSoldierFaces[ iCounter ] );
		}
	}

	if( giMercPanelImage != 0 )
	{
		DeleteVideoObjectFromIndex( giMercPanelImage );
	}

	// reset the soldier ptrs in the update box
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		pUpdateSoldierBox[ iCounter ] = NULL;
	}


	return;
}


INT32 GetNumberOfMercsInUpdateList( void )
{
	INT32 iCounter = 0, iCount = 0;

	// run through the non-empty slots
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		// valid guy here
		if( pUpdateSoldierBox[ iCounter ] != NULL )
		{
			iCount++;
		}
	}

	return( iCount );
}

BOOLEAN IsThePopUpBoxEmpty( void )
{
	INT32 iCounter = 0;
	BOOLEAN fEmpty = TRUE;

	// run through the non-empty slots
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		// valid guy here
		if( pUpdateSoldierBox[ iCounter ] != NULL )
		{
			fEmpty = FALSE;
		}
	}

	return( fEmpty );
}


void AddSoldierToWaitingListQueue( SOLDIERTYPE *pSoldier )
{
	INT32 iSoldierId = 0;


	// get soldier profile
	iSoldierId = pSoldier->ubID;

	SpecialCharacterDialogueEvent( DIALOGUE_ADD_EVENT_FOR_SOLDIER_UPDATE_BOX, UPDATE_BOX_REASON_ADDSOLDIER, iSoldierId, 0, 0, 0 );
	return;
}


void AddReasonToWaitingListQueue( INT32 iReason )
{
 	SpecialCharacterDialogueEvent( DIALOGUE_ADD_EVENT_FOR_SOLDIER_UPDATE_BOX, UPDATE_BOX_REASON_SET_REASON, iReason, 0, 0, 0 );
	return;
}


void AddDisplayBoxToWaitingQueue( void )
{
	SpecialCharacterDialogueEvent( DIALOGUE_ADD_EVENT_FOR_SOLDIER_UPDATE_BOX, UPDATE_BOX_REASON_SHOW_BOX, 0, 0, 0, 0 );


	return;
}


void ShowUpdateBox( void )
{
	// we want to show the box
	fShowUpdateBox = TRUE;
}

void AddSoldierToUpdateBox( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;
	VOBJECT_DESC VObjectDesc;



	// going to load face
	VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;

	if( pSoldier->bLife == 0 )
	{
		return;
	}

	if( pSoldier -> bActive == FALSE )
	{
		return;
	}

	// if update
	if( pUpdateSoldierBox[ iCounter ] == NULL )
	{
		sprintf( VObjectDesc.ImageFile, "Interface\\panels.sti" );
		if( !AddVideoObject( &VObjectDesc, &giMercPanelImage ) )
		{
			AssertMsg( 0, "Failed to load Interface\\panels.sti" );
		}
	}

	// run thought list of update soldiers
	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		// find a free slot
		if( pUpdateSoldierBox[ iCounter ] == NULL )
		{
			// add to box
			pUpdateSoldierBox[ iCounter ] = pSoldier;

			if( gMercProfiles[ pSoldier->ubProfile ].ubFaceIndex < 100 )
			{
				// grab filename of face
				sprintf( VObjectDesc.ImageFile, "Faces\\65Face\\%02d.sti", gMercProfiles[ pSoldier->ubProfile ].ubFaceIndex );
			}
			else
			{
				// grab filename of face
				sprintf( VObjectDesc.ImageFile, "Faces\\65Face\\%03d.sti", gMercProfiles[ pSoldier->ubProfile ].ubFaceIndex );
			}

			// load the face
			AddVideoObject( &VObjectDesc, &giUpdateSoldierFaces[ iCounter ] );

			return;
		}
	}
	return;
}

void SetSoldierUpdateBoxReason( INT32 iReason )
{
	//set the reason for the update
	iReasonForSoldierUpDate = iReason;

	return;
}


void DisplaySoldierUpdateBox( )
{
	INT32 iNumberOfMercsOnUpdatePanel = 0;
	INT32 iNumberHigh = 0, iNumberWide = 0;
	INT32 iUpdatePanelWidth = 0, iUpdatePanelHeight = 0;
	INT32 iX = 0, iY = 0;
	INT32 iFaceX = 0, iFaceY = 0;
	BOOLEAN fFourWideMode = FALSE;
	HVOBJECT hBackGroundHandle;
	INT32 iCounter = 0;
	CHAR16 sString[ 32 ];
	INT16 sX = 0, sY = 0;
	INT32 iHeightOfString = 0;
	INT32 iCounterB = 0;
	INT32 iOrigNumberHigh = 0, iOrigY = 0;
	INT32 iUpperLimit = 0;


	if( fShowUpdateBox == FALSE )
	{
		return;
	}


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
	LockPauseState( 4 );

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
	iX = 290 + ( 336 - iUpdatePanelWidth ) / 2;

//	iY = 28 + ( 288 - iUpdatePanelHeight ) / 2;

	// Have the bottom of the box ALWAYS a set distance from the bottom of the map ( so user doesnt have to move mouse far )
	iY = 280 - iUpdatePanelHeight;

	GetVideoObject( &hBackGroundHandle, guiUpdatePanelTactical );


	//Display the 2 TOP corner pieces
	BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 0, iX-4, iY - 4 , VO_BLT_SRCTRANSPARENCY,NULL );
	BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 2, iX+iUpdatePanelWidth, iY - 4 , VO_BLT_SRCTRANSPARENCY,NULL );

	if( fFourWideMode )
	{
		//Display 2 vertical lines starting at the bottom
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 3, iX - 4, iY + iUpdatePanelHeight - 3 - 70, VO_BLT_SRCTRANSPARENCY,NULL );
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 5, iX + iUpdatePanelWidth, iY + iUpdatePanelHeight - 3 - 70, VO_BLT_SRCTRANSPARENCY,NULL );

		//Display the 2 bottom corner pieces
		BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 0, iX-4, iY + iUpdatePanelHeight - 3, VO_BLT_SRCTRANSPARENCY,NULL );
		BltVideoObject( guiSAVEBUFFER, hBackGroundHandle, 2, iX+iUpdatePanelWidth, iY + iUpdatePanelHeight - 3, VO_BLT_SRCTRANSPARENCY,NULL );
	}


	SetFontDestBuffer( guiSAVEBUFFER, 0, 0, 640, 480, FALSE );

	iUpperLimit = fFourWideMode ? ( iNumberOfMercsOnUpdatePanel + NUMBER_OF_MERC_COLUMNS_FOR_FOUR_WIDE_MODE ) : ( iNumberOfMercsOnUpdatePanel + NUMBER_OF_MERC_COLUMNS_FOR_TWO_WIDE_MODE );

	//need to put the background down first
	for( iCounter = 0; iCounter < iUpperLimit; iCounter++ )
	{
		// blt the face and name

		// get the face x and y
		iFaceX = iX + ( iCounter % iNumberWide ) * TACT_UPDATE_MERC_FACE_X_WIDTH;
		iFaceY = iY + ( iCounter / iNumberWide ) * TACT_UPDATE_MERC_FACE_X_HEIGHT;

		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 20, iFaceX, iFaceY , VO_BLT_SRCTRANSPARENCY,NULL );
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
			swprintf( sString, lengthof(sString), L"%S", pUpdateSoldierBox[ iCounter ]->name );
			DrawTextToScreen( sString, (UINT16)(iFaceX-5), (UINT16)(iFaceY + 31), 57, TINYFONT1, FONT_LTRED, FONT_BLACK, 0, CENTER_JUSTIFIED );
		}
	}

	// the button container box
	if( fFourWideMode )
	{
		//def: 3/1/99 WAS SUBINDEX 6,
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 19, iX - 4 + TACT_UPDATE_MERC_FACE_X_WIDTH,  iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y+3, VO_BLT_SRCTRANSPARENCY,NULL );

		// ATE: Display string for time compression
		DisplayWrappedString( ( UINT16 )( iX ),  ( UINT16 )( iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + 5 + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y + 3), ( UINT16 )( iUpdatePanelWidth ), 0, MAP_SCREEN_FONT, FONT_WHITE, gzLateLocalizedString[49], FONT_BLACK, 0, CENTER_JUSTIFIED );
	}
	else
	{
		//def: 3/1/99 WAS SUBINDEX 6,
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 19, iX - 4 , iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y+3, VO_BLT_SRCTRANSPARENCY,NULL );

		// ATE: Display string for time compression
		DisplayWrappedString( ( UINT16 )( iX ),  ( UINT16 )( iY + iNumberHigh * TACT_UPDATE_MERC_FACE_X_HEIGHT + 5 + REASON_FOR_SOLDIER_UPDATE_OFFSET_Y+3 ), ( UINT16 )( iUpdatePanelWidth ), 0, MAP_SCREEN_FONT, FONT_WHITE, gzLateLocalizedString[49], FONT_BLACK, 0, CENTER_JUSTIFIED );
	}

	iCounter = 0;

	// now wrap the border
	for( iCounter = 0; iCounter < iNumberHigh ; iCounter++ )
	{
		// the sides
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 3, iX - 4, iY + ( iCounter ) * TACT_UPDATE_MERC_FACE_X_HEIGHT, VO_BLT_SRCTRANSPARENCY,NULL );
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 5, iX + iUpdatePanelWidth, iY + ( iCounter ) * TACT_UPDATE_MERC_FACE_X_HEIGHT, VO_BLT_SRCTRANSPARENCY,NULL );
	}

	//big horizontal line
	for( iCounter = 0; iCounter < iNumberWide; iCounter++ )
	{
		// the top bottom
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 1, iX + TACT_UPDATE_MERC_FACE_X_WIDTH * (iCounter ) , iY - 4 , VO_BLT_SRCTRANSPARENCY,NULL );
		BltVideoObject( guiSAVEBUFFER , hBackGroundHandle, 1, iX + TACT_UPDATE_MERC_FACE_X_WIDTH * (iCounter ) , iY + iUpdatePanelHeight - 3 , VO_BLT_SRCTRANSPARENCY,NULL );
	}


	//Display the reason for the update box
	if( fFourWideMode )
	{
		DisplayWrappedString( ( INT16 )( iX ),  ( INT16 )( iY + 6 ), (INT16)iUpdatePanelWidth, 0, MAP_SCREEN_FONT, FONT_WHITE, pUpdateMercStrings[ iReasonForSoldierUpDate ], FONT_BLACK, 0, CENTER_JUSTIFIED );
	}
	else
	{
		DisplayWrappedString( ( INT16 )( iX ),  ( INT16 )( iY + 3 ), (INT16)iUpdatePanelWidth, 0, MAP_SCREEN_FONT, FONT_WHITE, pUpdateMercStrings[ iReasonForSoldierUpDate ], FONT_BLACK, 0, CENTER_JUSTIFIED );
	}


	SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );

	// restore extern background rect
	RestoreExternBackgroundRect( ( INT16 )( iX - 5 ), ( INT16 )( iY - 5 ), ( INT16 )(  iUpdatePanelWidth + 10 ), ( INT16 )(iUpdatePanelHeight + 6 ) );

	CreateDestroyUpdatePanelButtons( iX, ( iY + iUpdatePanelHeight - 18 ), fFourWideMode );
	MarkAButtonDirty( guiUpdatePanelButtons[ 0 ] );
	MarkAButtonDirty( guiUpdatePanelButtons[ 1 ] );
	return;
}


void CreateDestroyUpdatePanelButtons(INT32 iX, INT32 iY, BOOLEAN fFourWideMode )
{
	static BOOLEAN fCreated = FALSE;

	if( ( fShowUpdateBox == TRUE ) && ( fCreated == FALSE ) )
	{
		// set to created
		fCreated = TRUE;

		fShowAssignmentMenu = FALSE;
		fShowContractMenu = FALSE;

//		guiUpdatePanelButtonsImage[ 0 ]=  LoadButtonImage( "INTERFACE\\group_confirm.sti" ,-1,7,-1,8,-1 );
//		guiUpdatePanelButtonsImage[ 1 ] = LoadButtonImage( "INTERFACE\\group_confirm.sti" ,-1,7,-1,8,-1 );
		guiUpdatePanelButtonsImage[ 0 ]=  LoadButtonImage( "INTERFACE\\group_confirm_tactical.sti" ,-1,7,-1,8,-1 );
		guiUpdatePanelButtonsImage[ 1 ] = LoadButtonImage( "INTERFACE\\group_confirm_tactical.sti" ,-1,7,-1,8,-1 );

		if( fFourWideMode )
		{
			guiUpdatePanelButtons[ 0 ] = QuickCreateButton( guiUpdatePanelButtonsImage[ 0 ], ( INT16 )( iX - 4 + TACT_UPDATE_MERC_FACE_X_WIDTH + 4), (INT16)iY,
											BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)ContinueUpdateButtonCallback );

			guiUpdatePanelButtons[ 1 ] = QuickCreateButton( guiUpdatePanelButtonsImage[ 1 ], ( INT16 ) ( iX - 4 + 2 * TACT_UPDATE_MERC_FACE_X_WIDTH + 4 ), (INT16)iY,
											BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)StopUpdateButtonCallback);
		}
		else
		{
			guiUpdatePanelButtons[ 0 ] = QuickCreateButton( guiUpdatePanelButtonsImage[ 0 ], ( INT16 )( iX), (INT16)iY,
											BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST -1,
											(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)ContinueUpdateButtonCallback);

			guiUpdatePanelButtons[ 1 ] = QuickCreateButton( guiUpdatePanelButtonsImage[ 1 ], ( INT16 )( iX + TACT_UPDATE_MERC_FACE_X_WIDTH ), (INT16)iY,
											BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)StopUpdateButtonCallback);

		}

		SpecifyButtonText( guiUpdatePanelButtons[ 0 ], pUpdatePanelButtons[ 0 ] );
		SpecifyButtonFont( guiUpdatePanelButtons[ 0 ], MAP_SCREEN_FONT );
		SpecifyButtonUpTextColors( guiUpdatePanelButtons[ 0 ], FONT_MCOLOR_BLACK, FONT_BLACK );
		SpecifyButtonDownTextColors( guiUpdatePanelButtons[ 0 ], FONT_MCOLOR_BLACK, FONT_BLACK );
		SetButtonFastHelpText( guiUpdatePanelButtons[ 0 ], gzLateLocalizedString[51] );

		SpecifyButtonText( guiUpdatePanelButtons[ 1 ], pUpdatePanelButtons[ 1 ] );
		SpecifyButtonFont( guiUpdatePanelButtons[ 1 ], MAP_SCREEN_FONT );
		SpecifyButtonUpTextColors( guiUpdatePanelButtons[ 1 ], FONT_MCOLOR_BLACK, FONT_BLACK );
		SpecifyButtonDownTextColors( guiUpdatePanelButtons[ 1 ], FONT_MCOLOR_BLACK, FONT_BLACK );
		SetButtonFastHelpText( guiUpdatePanelButtons[ 1 ], gzLateLocalizedString[52] );
	}
	else if( ( fShowUpdateBox == FALSE ) && ( fCreated == TRUE ) )
	{
		// set to uncreated
		fCreated = FALSE;

		// get rid of the buttons and images
		RemoveButton( guiUpdatePanelButtons[ 0 ] );
		RemoveButton( guiUpdatePanelButtons[ 1 ] );

		UnloadButtonImage( guiUpdatePanelButtonsImage[ 0 ]);
		UnloadButtonImage( guiUpdatePanelButtonsImage[ 1 ]);

		// unpause
		UnPauseDialogueQueue( );
	}
}


/*
void CreateUpdateBox( void )
{
	// create basic box
 CreatePopUpBox(&ghUpdateBox, AssignmentDimensions, MovePosition, (POPUP_BOX_FLAG_CLIP_TEXT|POPUP_BOX_FLAG_RESIZE ));

 // which buffer will box render to
 SetBoxBuffer(ghUpdateBox, FRAME_BUFFER);

 // border type?
 SetBorderType(ghUpdateBox,guiPOPUPBORDERS);

 // background texture
 SetBackGroundSurface(ghUpdateBox, guiPOPUPTEX);

 // margin sizes
 SetMargins( ghUpdateBox, 6, 6, 4, 4 );

 // space between lines
 SetLineSpace(ghUpdateBox, 2);

 // set current box to this one
 SetCurrentBox( ghUpdateBox );

 // add strings
 CreateUpdateBoxStrings( );

 // set font type
 SetBoxFont(ghUpdateBox, MAP_SCREEN_FONT);

 // set highlight color
 SetBoxHighLight(ghUpdateBox, FONT_WHITE);

 // unhighlighted color
 SetBoxForeground(ghUpdateBox, FONT_LTGREEN);

 // background color
 SetBoxBackground(ghUpdateBox, FONT_BLACK);

 // shaded color..for darkened text
 SetBoxShade( ghUpdateBox, FONT_BLACK );

 // resize box to text
 ResizeBoxToText( ghUpdateBox );

 // create screen mask
 CreateScreenMaskForMoveBox( );

 ShowBox( ghUpdateBox );

	return;
}


void CreateUpdateBoxStrings( void )
{
	INT32 iCounter = 0;
	CHAR16 sString[ 64 ];
	INT32 hStringHandle;

	swprintf( sString, L"%s", pUpdateMercStrings[ iReasonForSoldierUpDate ] );
	AddMonoString(&hStringHandle, sString );

	for( iCounter = 0; iCounter < SIZE_OF_UPDATE_BOX; iCounter++ )
	{
		// find valid soldier, add name
		if( pUpdateSoldierBox[ iCounter ] )
		{
			swprintf( sString, L"%s", pUpdateSoldierBox[ iCounter ]->name );
			AddMonoString(&hStringHandle, sString );
		}
	}

	// add a few blank lines
	sString[ 0 ] = 0;

	AddMonoString(&hStringHandle, sString );
	AddMonoString(&hStringHandle, sString );
	AddMonoString(&hStringHandle, sString );
	AddMonoString(&hStringHandle, sString );
}



void RemoveUpdateBox( void )
{
	// remove the box
	RemoveBox( ghUpdateBox );
	ghUpdateBox = -1;

	iReasonForSoldierUpDate = NO_REASON_FOR_UPDATE;

	// remove the screen mask
	RemoveScreenMaskForMoveBox( );

	// reset mercs that are in progress of being updated
	ResetSoldierUpdateBox( );
}



void DisplayUpdateBox( void )
{
	if( fShowUpdateBox )
	{
		// show the box
		ShowBox( ghUpdateBox );
		PauseTime( TRUE );
	}
	return;
}
*/



void CreateDestroyTheUpdateBox( void )
{
	static BOOLEAN fCreated = FALSE;

	if( ( fCreated == FALSE ) && ( fShowUpdateBox == TRUE ) )
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
		PauseGame( TRUE );
		LockPauseState( 5 );

		// display the box
		DisplaySoldierUpdateBox( );

		// Do beep
		PlayJA2SampleFromFile( "Sounds\\newbeep.wav", RATE_11025, MIDVOLUME, 1 , MIDDLEPAN );
	}
	else if( ( fCreated == TRUE ) && ( fShowUpdateBox == FALSE ) )
	{
		fCreated = FALSE;

		UnLockPauseState( );
		UnPauseGame( FALSE );

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


void UpdateButtonsDuringCharacterDialoguePicture( void )
{
	// stop showing buttons during certain instances of dialogue
	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		UnMarkButtonDirty( giCharInfoButton[ 0 ] );
		UnMarkButtonDirty( giCharInfoButton[ 1 ] );
	}

}

void UpdateButtonsDuringCharacterDialogueSubTitles( void )
{
	if( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) && ( gGameSettings.fOptions[ TOPTION_SUBTITLES ] ) )
	{
		UnMarkButtonDirty( giMapContractButton );
	}

	return;
}


void RenderSoldierSmallFaceForUpdatePanel( INT32 iIndex, INT32 iX, INT32 iY )
{
	INT32 iStartY = 0;
	SOLDIERTYPE *pSoldier = NULL;

	// fill the background for the info bars black
	ColorFillVideoSurfaceArea( guiSAVEBUFFER, iX+36, iY+2, iX+44,	iY+30, 0 );

	// put down the background
	BltVideoObjectFromIndex( guiSAVEBUFFER, giMercPanelImage, 0, iX, iY, VO_BLT_SRCTRANSPARENCY, NULL );

	// grab the face
	BltVideoObjectFromIndex( guiSAVEBUFFER , giUpdateSoldierFaces[ iIndex ], 0, iX+2, iY+2, VO_BLT_SRCTRANSPARENCY, NULL );

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


	return;
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



void SetUpdateBoxFlag( BOOLEAN fFlag )
{
	// set the flag
	fShowUpdateBox = fFlag;
	return;
}


void SetTixaAsFound( void )
{
	// set the town of Tixa as found by the player
	fFoundTixa = TRUE;
	fMapPanelDirty = TRUE;
}

void SetOrtaAsFound( void )
{
	// set the town of Orta as found by the player
	fFoundOrta = TRUE;
	fMapPanelDirty = TRUE;
}

void SetSAMSiteAsFound( UINT8 uiSamIndex )
{
	// set this SAM site as being found by the player
	fSamSiteFound[ uiSamIndex ] = TRUE;
	fMapPanelDirty = TRUE;
}


// ste up the timers for move menu in mapscreen for double click detection
void InitTimersForMoveMenuMouseRegions( void )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < MAX_POPUP_BOX_STRING_COUNT; iCounter++ )
	{
		giDblClickTimersForMoveBoxMouseRegions[ iCounter ] = 0;
	}
}

void UpdateHelpTextForMapScreenMercIcons( void )
{
	if( ( bSelectedInfoChar == -1 ) || ( gCharactersList[ bSelectedInfoChar ].fValid == FALSE ) )
	{
		SetRegionFastHelpText( &(gContractIconRegion), L"" );
		SetRegionFastHelpText( &(gInsuranceIconRegion), L"" );
		SetRegionFastHelpText( &(gDepositIconRegion), L"" );
	}
	else
	{
		// if merc is an AIM merc
		if( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC )
		{
			SetRegionFastHelpText( &(gContractIconRegion), zMarksMapScreenText[ 22 ] );
		}
		else
		{
			SetRegionFastHelpText( &(gContractIconRegion), L"" );
		}

		// if merc has life insurance
		if( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].usLifeInsurance > 0 )
		{
			SetRegionFastHelpText( &(gInsuranceIconRegion), zMarksMapScreenText[ 3 ] );
		}
		else
		{
			SetRegionFastHelpText( &(gInsuranceIconRegion), L"" );
		}

		// if merc has a medical deposit
		if( Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].usMedicalDeposit > 0 )
		{
			SetRegionFastHelpText( &(gDepositIconRegion), zMarksMapScreenText[ 12 ] );
		}
		else
		{
			SetRegionFastHelpText( &(gDepositIconRegion), L"" );
		}
	}
}

void CreateDestroyInsuranceMouseRegionForMercs( BOOLEAN fCreate )
{
	static BOOLEAN fCreated = FALSE;

	if( ( fCreated == FALSE ) && ( fCreate == TRUE ) )
	{
		MSYS_DefineRegion( &gContractIconRegion, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y, CHAR_ICON_X + CHAR_ICON_WIDTH, CHAR_ICON_CONTRACT_Y + CHAR_ICON_HEIGHT,
						 MSYS_PRIORITY_HIGH - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		MSYS_DefineRegion( &gInsuranceIconRegion, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + CHAR_ICON_SPACING, CHAR_ICON_X + CHAR_ICON_WIDTH, CHAR_ICON_CONTRACT_Y + CHAR_ICON_SPACING + CHAR_ICON_HEIGHT,
						 MSYS_PRIORITY_HIGH - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		MSYS_DefineRegion( &gDepositIconRegion, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + ( 2 * CHAR_ICON_SPACING ), CHAR_ICON_X + CHAR_ICON_WIDTH, CHAR_ICON_CONTRACT_Y + ( 2 * CHAR_ICON_SPACING ) + CHAR_ICON_HEIGHT,
						 MSYS_PRIORITY_HIGH - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		fCreated = TRUE;
	}
	else if( ( fCreated == TRUE ) && ( fCreate == FALSE ) )
	{
		MSYS_RemoveRegion( &gContractIconRegion );
		MSYS_RemoveRegion( &gInsuranceIconRegion );
		MSYS_RemoveRegion( &gDepositIconRegion );
		fCreated = FALSE;
	}
}


/*
void HandlePlayerEnteringMapScreenBeforeGoingToTactical( void )
{
	CHAR16 sString[ 256 ];

	if( !( AnyMercsHired( ) ) )
	{
		// no mercs hired inform player they must hire mercs
		swprintf( sString, pMapScreenJustStartedHelpText[ 0 ] );
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_OK, DoneHandlePlayerFirstEntryToMapScreen );

	}
	else
	{
		// player has mercs hired, tell them to time compress to get things underway
		swprintf( sString, pMapScreenJustStartedHelpText[ 1 ] );
		fShowMapScreenHelpText = TRUE;
	}



	// now inform the player

	if( fShowMapScreenHelpText )
	{
		fShowMapScreenHelpText = FALSE;
		SetUpShutDownMapScreenHelpTextScreenMask( );
		fShowMapScreenHelpText = TRUE;
	}

	return;
}


void DoneHandlePlayerFirstEntryToMapScreen(  UINT8 bExitValue )
{
	static BOOLEAN fFirstTime = TRUE;

	if( bExitValue == MSG_BOX_RETURN_OK )
	{
		if( fFirstTime == TRUE )
		{
			fFirstTime = FALSE;
			fShowMapScreenHelpText = TRUE;
		}
	}
}
*/


BOOLEAN HandleTimeCompressWithTeamJackedInAndGearedToGo( void )
{
	// check a team is ready to go
	if( !( AnyMercsHired( ) ) )
	{
		// no mercs, leave
		return( FALSE );
	}

	// make sure the game just started
	if( gTacticalStatus.fDidGameJustStart == FALSE )
	{
		return( FALSE );
	}

	// select starting sector (A9 - Omerta)
	ChangeSelectedMapSector( 9, 1, 0 );

	// load starting sector
	if ( !SetCurrentWorldSector( 9, 1, 0 ) )
	{
		return( FALSE );
	}

	//Setup variables in the PBI for this first battle.  We need to support the
	//non-persistant PBI in case the user goes to mapscreen.
	gfBlitBattleSectorLocator = TRUE;
	gubPBSectorX = 9;
	gubPBSectorY = 1;
	gubPBSectorZ = 0;
	gubEnemyEncounterCode = ENTERING_ENEMY_SECTOR_CODE;

	InitHelicopterEntranceByMercs( );

	FadeInGameScreen( );

	SetUpShutDownMapScreenHelpTextScreenMask( );

	// Add e-mail message
	AddEmail(ENRICO_CONGRATS,ENRICO_CONGRATS_LENGTH,MAIL_ENRICO, GetWorldTotalMin() );


	return( TRUE );
}

void HandleDisplayOfExitToTacticalMessageForFirstEntryToMapScreen( void )
{
	INT32 iTime = 0, iDifference = 0;

	if( gTacticalStatus.fDidGameJustStart == FALSE )
	{
		return;
	}

	if( AnyMercsHired() == FALSE )
	{
		return;
	}

	if( fResetTimerForFirstEntryIntoMapScreen )
	{
		giExitToTactBaseTime = 0;
		fResetTimerForFirstEntryIntoMapScreen = FALSE;
	}

	// is this the first time in?
	if( giExitToTactBaseTime == 0 )
	{
		// gte the clock, for initing
		giExitToTactBaseTime = GetJA2Clock();
	}

	iTime = GetJA2Clock();

	iDifference = iTime - giExitToTactBaseTime;

	if( iDifference > TIMER_FOR_SHOW_EXIT_TO_TACTICAL_MESSAGE )
	{
		fShowMapScreenHelpText = FALSE;
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		giExitToTactBaseTime = 0;
	}


	return;
}


BOOLEAN NotifyPlayerWhenEnemyTakesControlOfImportantSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, BOOLEAN fContested )
{
	CHAR16 sString[ 128 ], sStringA[ 64 ], sStringB[ 256 ], sStringC[ 64 ];
	INT32 iValue = 0;
	INT8 bTownId = 0;
	INT16 sSector = 0;
	INT8 bMineIndex;


	// are we below ground?
	if( bSectorZ != 0 )
	{
		// yes we are..there is nothing important to player here
		return( FALSE );
	}

	// get the name of the sector
	GetSectorIDString( sSectorX, sSectorY, bSectorZ, sString, lengthof(sString), TRUE );

	bTownId = GetTownIdForSector( sSectorX, sSectorY );

	// check if SAM site here
	if( IsThisSectorASAMSector( sSectorX, sSectorY, bSectorZ ) )
	{
		swprintf( sStringB, lengthof(sStringB), pMapErrorString[ 15 ], sString );

		// put up the message informing the player of the event
		DoScreenIndependantMessageBox( sStringB, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
		return( TRUE );
	}


	// check if a mine is here
	if( IsThereAMineInThisSector( sSectorX, sSectorY ) )
	{
		bMineIndex = GetMineIndexForSector( sSectorX, sSectorY );

		// if it was producing for us
		if ( ( GetMaxDailyRemovalFromMine( bMineIndex ) > 0 ) && SpokenToHeadMiner( bMineIndex ) )
		{
			// get how much we now will get from the mines
			iValue = GetProjectedTotalDailyIncome( );

			// parse the string
			swprintf( sStringC, lengthof(sStringC), L"%d", iValue );

			// insert
			InsertCommasForDollarFigure( sStringC );
			InsertDollarSignInToString( sStringC );

			swprintf( sStringB, lengthof(sStringB), pMapErrorString[ 16 ], sString, sStringC );

			// put up the message informing the player of the event
			DoScreenIndependantMessageBox( sStringB, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
			return( TRUE );
		}
	}


	if( fContested && bTownId )
	{
		if( bTownId == SAN_MONA )
		{ //San Mona isn't important.
			return( TRUE );
		}
		swprintf( sStringB, lengthof(sStringB), pMapErrorString[ 25 ], sString );

		// put up the message informing the player of the event
		DoScreenIndependantMessageBox( sStringB, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
		return( TRUE );
	}

	// get the strategic sector value
	sSector = sSectorX + MAP_WORLD_X * sSectorY;

	if( StrategicMap[ sSector ].bNameId == BLANK_SECTOR )
	{
		return( FALSE );
	}

	// get the name of the sector
	GetSectorIDString( sSectorX, sSectorY, bSectorZ, sStringA, lengthof(sStringA), TRUE );

	// now build the string
	swprintf( sString, lengthof(sString), pMapErrorString[ 17 ], sStringA );

	// put up the message box
	DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );

	return( TRUE );
}


void NotifyPlayerOfInvasionByEnemyForces( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, MSGBOX_CALLBACK ReturnCallback )
{
	INT16 sSector = 0;
	INT8 bTownId = 0;
	CHAR16 sString[ 128 ], sStringA[ 128 ];


	// check if below ground
	if( bSectorZ != 0 )
	{
		return;
	}

	// grab sector value
	sSector = sSectorX + MAP_WORLD_X * sSectorY;

	if( StrategicMap[ sSector ].fEnemyControlled == TRUE )
	{
		// enemy controlled any ways, leave
		return;
	}

	// get the town id
	bTownId = StrategicMap[ sSector ].bNameId;

	// check if SAM site here
	if( IsThisSectorASAMSector( sSectorX, sSectorY, bSectorZ ) )
	{
		// get sector id value
		GetShortSectorString( sSectorX, sSectorY, sStringA, lengthof(sStringA));

		swprintf( sString, lengthof(sString), pMapErrorString[ 22 ], sStringA );
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, ReturnCallback );
	}
	else if( bTownId )
	{
		// get the name of the sector
		GetSectorIDString( sSectorX, sSectorY, bSectorZ, sStringA, lengthof(sStringA), TRUE );

		swprintf( sString, lengthof(sString), pMapErrorString[ 23 ], sStringA );
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, ReturnCallback );
	}
	else
	{
		// get sector id value
		GetShortSectorString( sSectorX, sSectorY, sStringA, lengthof(sStringA));

		swprintf( sString, lengthof(sString), pMapErrorString[ 24 ], sStringA );
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, sString );
	}
}


BOOLEAN CanCharacterMoveInStrategic( SOLDIERTYPE *pSoldier, INT8 *pbErrorNumber )
{
	BOOLEAN fCanMove = TRUE;
	INT16 sSector = 0;
	BOOLEAN fProblemExists = FALSE;


	// valid soldier?
	Assert( pSoldier );
	Assert( pSoldier->bActive);


	// NOTE: Check for the most permanent conditions first, and the most easily remedied ones last!
	// In case several cases apply, only the reason found first will be given, so make it a good one!


	// still in transit?
	if( IsCharacterInTransit( pSoldier ) == TRUE )
	{
		*pbErrorNumber = 8;
		return( FALSE );
	}

	// a POW?
	if( pSoldier->bAssignment == ASSIGNMENT_POW )
	{
		*pbErrorNumber = 5;
		return( FALSE );
	}


	// underground? (can't move strategically, must use tactical traversal )
	if( pSoldier->bSectorZ != 0 )
	{
		*pbErrorNumber = 1;
		return( FALSE );
	}


	// vehicle checks
	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// empty (needs a driver!)?
		if ( GetNumberInVehicle( pSoldier->bVehicleID ) == 0 )
		{
			*pbErrorNumber = 32;
			return( FALSE );
		}

		// too damaged?
		if ( pSoldier->bLife < OKLIFE )
		{
			*pbErrorNumber = 47;
			return( FALSE );
		}

		// out of fuel?
		if ( !VehicleHasFuel( pSoldier ) )
		{
			*pbErrorNumber = 42;
			return( FALSE );
		}
	}
	else	// non-vehicle
	{
		// dead?
		if ( pSoldier->bLife <= 0 )
		{
			swprintf( gsCustomErrorString, lengthof(gsCustomErrorString), pMapErrorString[ 35 ], pSoldier->name );
			*pbErrorNumber = -99;	// customized error message!
			return( FALSE );
		}

		// too injured?
		if ( pSoldier->bLife < OKLIFE )
		{
			swprintf( gsCustomErrorString, lengthof(gsCustomErrorString), pMapErrorString[ 33 ], pSoldier->name );
			*pbErrorNumber = -99;	// customized error message!
			return( FALSE );
		}
	}


	// if merc is in a particular sector, not somewhere in between
	if ( pSoldier->fBetweenSectors == FALSE )
	{
		// and he's NOT flying above it all in a working helicopter
		if( !SoldierAboardAirborneHeli( pSoldier ) )
		{
			// and that sector is loaded...
			if( ( pSoldier->sSectorX == gWorldSectorX ) && ( pSoldier->sSectorY == gWorldSectorY ) &&
					( pSoldier->bSectorZ == gbWorldSectorZ ) )
			{
				// in combat?
				if( gTacticalStatus.uiFlags & INCOMBAT )
				{
					*pbErrorNumber = 11;
					return( FALSE );
				}

				// hostile sector?
				if ( gTacticalStatus.fEnemyInSector )
				{
					*pbErrorNumber = 2;
					return( FALSE );
				}

				// air raid in loaded sector where character is?
				if( InAirRaid( ) )
				{
					*pbErrorNumber = 10;
					return( FALSE );
				}
			}

			// not necessarily loaded - if there are any hostiles there
			if( NumHostilesInSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ ) > 0 )
			{
				*pbErrorNumber = 2;
				return( FALSE );
			}
		}
	}


	// if in L12 museum, and the museum alarm went off, and Eldin still around?
	if ( ( pSoldier->sSectorX == 12 ) && ( pSoldier->sSectorY == MAP_ROW_L ) && ( pSoldier->bSectorZ == 0 ) &&
			 ( !pSoldier->fBetweenSectors ) && gMercProfiles[ ELDIN ].bMercStatus != MERC_IS_DEAD )
	{
		UINT8	ubRoom, cnt;
		SOLDIERTYPE * pSoldier2;

		if ( InARoom( pSoldier->sGridNo, &ubRoom ) && ubRoom >= 22 && ubRoom <= 41 )
		{

			cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

			for ( pSoldier2 = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; cnt++,pSoldier2++)
			{
				if ( pSoldier2->bActive )
				{
					if ( FindObj( pSoldier2, CHALICE ) != ITEM_NOT_FOUND )
					{
						*pbErrorNumber = 34;
						return( FALSE );
					}
				}
			}
		}
	}


	// on assignment, other than just in a VEHICLE?
	if( ( pSoldier->bAssignment >= ON_DUTY ) && ( pSoldier->bAssignment != VEHICLE ) )
	{
		*pbErrorNumber = 3;
		return( FALSE );
	}

	// if he's walking/driving, and so tired that he would just stop the group anyway in the next sector,
	// or already asleep and can't be awakened
	if ( PlayerSoldierTooTiredToTravel( pSoldier ) )
	{
		// too tired
		swprintf( gsCustomErrorString, lengthof(gsCustomErrorString), pMapErrorString[ 43 ], pSoldier->name );
		*pbErrorNumber = -99;	// customized error message!
		return( FALSE );
	}


	// a robot?
	if ( AM_A_ROBOT( pSoldier ) )
	{
		// going alone?
		if ( ( ( pSoldier->bAssignment == VEHICLE ) && ( !IsRobotControllerInVehicle( pSoldier->iVehicleId ) ) ) ||
				 ( ( pSoldier->bAssignment  < ON_DUTY ) && ( !IsRobotControllerInSquad( pSoldier->bAssignment ) ) ) )
		{
			*pbErrorNumber = 49;
			return( FALSE );
		}
	}
	// an Escorted NPC?
	else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC )
	{
		// going alone?
		if ( ( ( pSoldier->bAssignment == VEHICLE ) && ( GetNumberOfNonEPCsInVehicle( pSoldier->iVehicleId ) == 0 ) ) ||
				 ( ( pSoldier->bAssignment  < ON_DUTY ) && ( NumberOfNonEPCsInSquad( pSoldier->bAssignment ) == 0 ) ) )
		{
			// are they male or female
			if( gMercProfiles[ pSoldier->ubProfile ].bSex == MALE )
			{
				swprintf( gsCustomErrorString, lengthof(gsCustomErrorString), L"%S %S", pSoldier->name ,pMapErrorString[ 6 ] );
			}
			else
			{
				swprintf( gsCustomErrorString, lengthof(gsCustomErrorString), L"%S %S", pSoldier->name ,pMapErrorString[ 7 ] );
			}

			*pbErrorNumber = -99;	// customized error message!
			return( FALSE );
		}
	}


	// assume there's no problem
	fProblemExists = FALSE;

	// find out if this particular character can't move for some reason
	switch( pSoldier->ubProfile )
	{
		case( MARIA ):
			// Maria can't move if she's in sector C5
			sSector = SECTOR( pSoldier->sSectorX, pSoldier->sSectorY );
			if( sSector == SEC_C5 )
			{
				// can't move at this time
				fProblemExists = TRUE;
			}
			break;
	}

	if ( fProblemExists )
	{
		// inform user this specific merc cannot be moved out of the sector
		swprintf( gsCustomErrorString, lengthof(gsCustomErrorString), pMapErrorString[ 29 ], pSoldier->name );
		*pbErrorNumber = -99;	// customized error message!
		return( FALSE );
	}


	// passed all checks - this character may move strategically!
	return( TRUE );
}


BOOLEAN CanEntireMovementGroupMercIsInMove( SOLDIERTYPE *pSoldier, INT8 *pbErrorNumber )
{
	SOLDIERTYPE *pCurrentSoldier = NULL;
	INT32 iCounter = 0;
	UINT8 ubGroup = 0;
	UINT8 ubCurrentGroup = 0;


	// first check the requested character himself
	if( CanCharacterMoveInStrategic( pSoldier, pbErrorNumber ) == FALSE )
	{
		// failed no point checking anyone else
		return( FALSE );
	}


	// now check anybody who would be travelling with him

	// does character have group?
	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// IS a vehicle - use vehicle's group
		ubGroup = pVehicleList[ pSoldier->bVehicleID ].ubMovementGroup;
	}
	else if( pSoldier->bAssignment == VEHICLE )
	{
		// IN a vehicle - use vehicle's group
		ubGroup = pVehicleList[ pSoldier->iVehicleId ].ubMovementGroup;
	}
	else
	{
		ubGroup = pSoldier->ubGroupID;
	}

	// even if group is 0 (not that that should happen, should it?) still loop through for other mercs selected to move

	// if anyone in the merc's group or also selected cannot move for whatever reason return false
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			// get soldier
			pCurrentSoldier = &( Menptr[ gCharactersList[ iCounter ].usSolID ] );

			// skip inactive grunts
			if( pCurrentSoldier->bActive == FALSE )
			{
				continue;
			}

			// skip the same guy we did already
			if ( pCurrentSoldier == pSoldier )
			{
				continue;
			}

			// does character have group?
			if( pCurrentSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				// IS a vehicle
				ubCurrentGroup = pVehicleList[ pCurrentSoldier->bVehicleID ].ubMovementGroup;
			}
			else if( pCurrentSoldier->bAssignment == VEHICLE )
			{
				// IN a vehicle
				ubCurrentGroup = pVehicleList[ pCurrentSoldier->iVehicleId ].ubMovementGroup;
			}
			else
			{
				ubCurrentGroup = pCurrentSoldier->ubGroupID;
			}

			// if he is in the same movement group (i.e. squad), or he is still selected to go with us (legal?)
			if( ( ubCurrentGroup == ubGroup ) || ( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE ) )
			{
				// can this character also move strategically?
				if( CanCharacterMoveInStrategic( pCurrentSoldier, pbErrorNumber ) == FALSE )
				{
					// cannot move, fail, and don't bother checking anyone else, either
					return( FALSE );
				}
			}
		}
	}


	// everybody can move...  Yey!  :-)
	return( TRUE );
}



void ReportMapScreenMovementError( INT8 bErrorNumber )
{
	if ( bErrorNumber == -99 )
	{
		// - 99 is a special message # indicating a customized message
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, gsCustomErrorString, MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
	else
	{
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, pMapErrorString[ bErrorNumber ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
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



BOOLEAN CanSoldierMoveWithVehicleId( SOLDIERTYPE *pSoldier, INT32 iVehicle1Id )
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


BOOLEAN SaveLeaveItemList( HWFILE hFile )
{
	INT32 iCounter = 0;
	MERC_LEAVE_ITEM *pCurrentItem;
	UINT32 uiCount=0;
	UINT32	uiNumBytesWritten = 0;
	BOOLEAN	fNodeExists = FALSE;
	UINT32	uiCnt;

	for( iCounter = 0; iCounter < NUM_LEAVE_LIST_SLOTS; iCounter++ )
	{
		// go through nodes and save them
		if ( gpLeaveListHead[ iCounter ] != NULL )
		{
			fNodeExists = TRUE;

			// Save the to specify that a node DOES exist
			FileWrite( hFile, &fNodeExists, sizeof(BOOLEAN), &uiNumBytesWritten );
			if( uiNumBytesWritten != sizeof(BOOLEAN) )
			{
				return(FALSE);
			}

			uiCount = 1;
			pCurrentItem = gpLeaveListHead[ iCounter ];

			//loop through all the nodes to see how many there are
			while( pCurrentItem->pNext )
			{
				 pCurrentItem = pCurrentItem->pNext;
				 uiCount++;
			}

			// Save the number specifing how many items there are in the list
			FileWrite( hFile, &uiCount, sizeof(UINT32), &uiNumBytesWritten );
			if( uiNumBytesWritten != sizeof(UINT32) )
			{
				return(FALSE);
			}

			pCurrentItem = gpLeaveListHead[ iCounter ];

			//loop through all the nodes to see how many there are
			for( uiCnt=0; uiCnt<uiCount; uiCnt++)
			{
				// Save the items
				FileWrite( hFile, pCurrentItem, sizeof( MERC_LEAVE_ITEM ), &uiNumBytesWritten );
				if( uiNumBytesWritten != sizeof( MERC_LEAVE_ITEM ) )
				{
					return(FALSE);
				}

				pCurrentItem = pCurrentItem->pNext;
			}
		}
		else
		{
			fNodeExists = FALSE;
			// Save the to specify that a node DOENST exist
			FileWrite( hFile, &fNodeExists, sizeof(BOOLEAN), &uiNumBytesWritten );
			if( uiNumBytesWritten != sizeof(BOOLEAN) )
			{
				return(FALSE);
			}
		}
	}

	//Save the leave list profile id's
	for( iCounter = 0; iCounter < NUM_LEAVE_LIST_SLOTS; iCounter++ )
	{
		FileWrite( hFile, &guiLeaveListOwnerProfileId[ iCounter ], sizeof(UINT32), &uiNumBytesWritten );
		if( uiNumBytesWritten != sizeof(UINT32) )
		{
			return(FALSE);
		}
	}

	return( TRUE );
}



BOOLEAN LoadLeaveItemList( HWFILE hFile )
{
	INT32 iCounter = 0;
	MERC_LEAVE_ITEM *pCurrentItem;
	MERC_LEAVE_ITEM *pItem;
	UINT32 uiCount=0;
	UINT32	uiNumBytesRead = 0;
	BOOLEAN	fNodeExists = FALSE;
	UINT32	uiSubItem;

	//Shutdown the list
	ShutDownLeaveList( );

	//init the list
	InitLeaveList( );

	//loop through all the lists
	for( iCounter = 0; iCounter < NUM_LEAVE_LIST_SLOTS; iCounter++ )
	{
		// load the flag that specifis that a node DOES exist
		FileRead( hFile, &fNodeExists, sizeof(BOOLEAN), &uiNumBytesRead );
		if( uiNumBytesRead != sizeof(BOOLEAN) )
		{
			return(FALSE);
		}

		// if a root node is supposed to exist
		if( fNodeExists )
		{
			// load the number specifing how many items there are in the list
			FileRead( hFile, &uiCount, sizeof(UINT32), &uiNumBytesRead );
			if( uiNumBytesRead != sizeof(UINT32) )
			{
				return(FALSE);
			}

			// allocate space
			gpLeaveListHead[ iCounter ] = MemAlloc( sizeof( MERC_LEAVE_ITEM ) );
			if( gpLeaveListHead[ iCounter ] == NULL )
			{
				return( FALSE );
			}
			memset( gpLeaveListHead[ iCounter ], 0, sizeof( MERC_LEAVE_ITEM ) );

			pCurrentItem = gpLeaveListHead[ iCounter ];

			for( uiSubItem=0; uiSubItem< uiCount; uiSubItem++ )
			{
				// allocate space
				pItem = MemAlloc( sizeof( MERC_LEAVE_ITEM ) );
				if( pItem == NULL )
				{
					return( FALSE );
				}
				memset( pItem, 0, sizeof( MERC_LEAVE_ITEM ) );

				// Load the items
				FileRead( hFile, pItem, sizeof( MERC_LEAVE_ITEM ), &uiNumBytesRead );
				if( uiNumBytesRead != sizeof( MERC_LEAVE_ITEM ) )
				{
					return(FALSE);
				}


				pItem->pNext = NULL;

				//add the node to the list
				if( uiSubItem == 0 )
				{
					gpLeaveListHead[ iCounter ] = pItem;
					pCurrentItem = gpLeaveListHead[ iCounter ];
				}
				else
				{
					pCurrentItem->pNext = pItem;
					pCurrentItem = pCurrentItem->pNext;
				}
			}
		}
	}

	//Load the leave list profile id's
	for( iCounter = 0; iCounter < NUM_LEAVE_LIST_SLOTS; iCounter++ )
	{
		FileRead( hFile, &guiLeaveListOwnerProfileId[ iCounter ], sizeof(UINT32), &uiNumBytesRead );
		if( uiNumBytesRead != sizeof(UINT32) )
		{
			return(FALSE);
		}
	}

	return( TRUE );
}



void TurnOnSectorLocator( UINT8 ubProfileID )
{
	SOLDIERTYPE *pSoldier;

	Assert( ubProfileID != NO_PROFILE );

	pSoldier = FindSoldierByProfileID( ubProfileID, FALSE );
	if( pSoldier )
	{
		gsSectorLocatorX = pSoldier->sSectorX;
		gsSectorLocatorY = pSoldier->sSectorY;
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
				Assert( iHelicopterVehicleId != -1 );
				gsSectorLocatorX = pVehicleList[ iHelicopterVehicleId ].sSectorX;
				gsSectorLocatorY = pVehicleList[ iHelicopterVehicleId ].sSectorY;
			}
			else
			{
				return;
			}
		}
		else
		{
			gsSectorLocatorX = gMercProfiles[ ubProfileID ].sSectorX;
			gsSectorLocatorY = gMercProfiles[ ubProfileID ].sSectorY;
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
	HVOBJECT hHandle;
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


	GetVideoObject( &hHandle, guiSectorLocatorGraphicID );

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
	BltVideoObject( FRAME_BUFFER, hHandle, ubFrame, sScreenX, sScreenY, VO_BLT_SRCTRANSPARENCY, NULL );

	// invalidate region on frame buffer
	InvalidateRegion( sScreenX, sScreenY - 1, sScreenX + MAP_GRID_X , sScreenY + MAP_GRID_Y );

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
			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_ENTER_MAPSCREEN,0,0,0,0,0 );
			HandleImportantMercQuote( pSoldier, QUOTE_MERC_GONE_UP_IN_PRICE );
			TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_SHOW_CONTRACT_MENU, 0,0 );
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
