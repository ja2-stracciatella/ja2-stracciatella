#include "Local.h"
#include "MapScreen.h"
#include "MessageBoxScreen.h"
#include "Town_Militia.h"
#include "Militia_Control.h"
#include "Campaign_Types.h"
#include "StrategicMap.h"
#include "Overhead.h"
#include "Strategic_Town_Loyalty.h"
#include "Random.h"
#include "Text.h"
#include "Map_Screen_Interface.h"
#include "Interface.h"
#include "LaptopSave.h"
#include "Finances.h"
#include "Game_Clock.h"
#include "Assignments.h"
#include "Squads.h"
#include "Soldier_Create.h"
#include "Dialogue_Control.h"
#include "Queen_Command.h"
#include "PreBattle_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Interface_Control.h"
#include "Map_Screen_Interface_Map.h"
#include "JAScreens.h"
#include "Debug.h"
#include "ScreenIDs.h"


#define SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST 50

// temporary local global variables
UINT8 gubTownSectorServerTownId = BLANK_SECTOR;
INT16 gsTownSectorServerSkipX = -1;
INT16	gsTownSectorServerSkipY = -1;
UINT8 gubTownSectorServerIndex = 0;
BOOLEAN gfYesNoPromptIsForContinue = FALSE;		// this flag remembers whether we're starting new training, or continuing
INT32 giTotalCostOfTraining = 0;


//the completed list of sector soldiers for training militia
static SOLDIERTYPE* g_list_of_merc_in_sectors_completed_militia_training[SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST];
SOLDIERTYPE *pMilitiaTrainerSoldier = NULL;

// note that these sector values are STRATEGIC INDEXES, not 0-255!
INT16 gsUnpaidStrategicSector[ MAX_CHARACTER_COUNT ];


static void HandleCompletionOfTownTrainingByGroupWithTrainer(SOLDIERTYPE* pTrainer);
static void InitFriendlyTownSectorServer(UINT8 ubTownId, INT16 sSkipSectorX, INT16 sSkipSectorY);
static BOOLEAN ServeNextFriendlySectorInTown(INT16* sNeighbourX, INT16* sNeighbourY);
static void StrategicAddMilitiaToSector(INT16 sMapX, INT16 sMapY, UINT8 ubRank, UINT8 ubHowMany);
static void StrategicPromoteMilitiaInSector(INT16 sMapX, INT16 sMapY, UINT8 ubCurrentRank, UINT8 ubHowMany);


void TownMilitiaTrainingCompleted( SOLDIERTYPE *pTrainer, INT16 sMapX, INT16 sMapY )
{
	UINT8 ubMilitiaTrained = 0;
	BOOLEAN fFoundOne;
	INT16 sNeighbourX, sNeighbourY;
	UINT8 ubTownId;


	// get town index
	ubTownId = StrategicMap[ sMapX + sMapY * MAP_WORLD_X ].bNameId;

	if( ubTownId == BLANK_SECTOR )
	{
		Assert( IsThisSectorASAMSector( sMapX, sMapY, 0 ) );
	}


	// force tactical to update militia status
	gfStrategicMilitiaChangesMade = TRUE;

	// ok, so what do we do with all this training?  Well, in order of decreasing priority:
	// 1) If there's room in training sector, create new GREEN militia guys there
	// 2) If not enough room there, create new GREEN militia guys in friendly sectors of the same town
	// 3) If not enough room anywhere in town, promote a number of GREENs in this sector into regulars
	// 4) If not enough GREENS there to promote, promote GREENs in other sectors.
	// 5) If all friendly sectors of this town are completely filled with REGULAR militia, then training effect is wasted

	while (ubMilitiaTrained < MILITIA_TRAINING_SQUAD_SIZE)
	{
		// is there room for another militia in the training sector itself?
		if (CountAllMilitiaInSector(sMapX, sMapY) < MAX_ALLOWABLE_MILITIA_PER_SECTOR)
		{
			// great! Create a new GREEN militia guy in the training sector
			StrategicAddMilitiaToSector(sMapX, sMapY, GREEN_MILITIA, 1);
		}
		else
		{
			fFoundOne = FALSE;

			if( ubTownId != BLANK_SECTOR )
			{
				InitFriendlyTownSectorServer(ubTownId, sMapX, sMapY);

				// check other eligible sectors in this town for room for another militia
				while( ServeNextFriendlySectorInTown( &sNeighbourX, &sNeighbourY ) )
				{
					// is there room for another militia in this neighbouring sector ?
					if (CountAllMilitiaInSector(sNeighbourX, sNeighbourY) < MAX_ALLOWABLE_MILITIA_PER_SECTOR)
					{
						// great! Create a new GREEN militia guy in the neighbouring sector
						StrategicAddMilitiaToSector(sNeighbourX, sNeighbourY, GREEN_MILITIA, 1);

						fFoundOne = TRUE;
						break;
					}
				}
			}

			// if we still haven't been able to train anyone
			if (!fFoundOne)
			{
				// alrighty, then.  We'll have to *promote* guys instead.

				// are there any GREEN militia men in the training sector itself?
				if (MilitiaInSectorOfRank(sMapX, sMapY, GREEN_MILITIA) > 0)
				{
					// great! Promote a GREEN militia guy in the training sector to a REGULAR
					StrategicPromoteMilitiaInSector(sMapX, sMapY, GREEN_MILITIA, 1);
				}
				else
				{
					if( ubTownId != BLANK_SECTOR )
					{
						// dammit! Last chance - try to find other eligible sectors in the same town with a Green guy to be promoted
						InitFriendlyTownSectorServer(ubTownId, sMapX, sMapY);

						// check other eligible sectors in this town for room for another militia
						while( ServeNextFriendlySectorInTown( &sNeighbourX, &sNeighbourY ) )
						{
							// are there any GREEN militia men in the neighbouring sector ?
							if (MilitiaInSectorOfRank(sNeighbourX, sNeighbourY, GREEN_MILITIA) > 0)
							{
								// great! Promote a GREEN militia guy in the neighbouring sector to a REGULAR
								StrategicPromoteMilitiaInSector(sNeighbourX, sNeighbourY, GREEN_MILITIA, 1);

								fFoundOne = TRUE;
								break;
							}
						}
					}

					// if we still haven't been able to train anyone
					if (!fFoundOne)
					{
						// Well, that's it.  All eligible sectors of this town are full of REGULARs or ELITEs.
						// The training goes to waste in this situation.
						break; // the main while loop
					}
				}
			}
		}

		// next, please!
		ubMilitiaTrained++;
	}


	// if anyone actually got trained
	if (ubMilitiaTrained > 0)
	{
		// update the screen display
		fMapPanelDirty = TRUE;

		if( ubTownId != BLANK_SECTOR )
		{
			// loyalty in this town increases a bit because we obviously care about them...
			IncrementTownLoyalty( ubTownId, LOYALTY_BONUS_FOR_TOWN_TRAINING );
		}

	}


	// the trainer announces to player that he's finished his assignment.  Make his sector flash!
	AssignmentDone( pTrainer, TRUE, FALSE );

	// handle completion of town by training group
	HandleCompletionOfTownTrainingByGroupWithTrainer( pTrainer );

}


// feed this a SOLDIER_CLASS_, it will return you a _MITILIA rank, or -1 if the guy's not militia
INT8 SoldierClassToMilitiaRank(UINT8 ubSoldierClass)
{
	INT8 bRank = -1;

	switch( ubSoldierClass )
	{
		case SOLDIER_CLASS_GREEN_MILITIA:
			bRank = GREEN_MILITIA;
			break;
		case SOLDIER_CLASS_REG_MILITIA:
			bRank = REGULAR_MILITIA;
			break;
		case SOLDIER_CLASS_ELITE_MILITIA:
			bRank = ELITE_MILITIA;
			break;
	}

	return(bRank);
}


// add militias of a certain rank
static void StrategicAddMilitiaToSector(INT16 sMapX, INT16 sMapY, UINT8 ubRank, UINT8 ubHowMany)
{
	SECTORINFO *pSectorInfo = &( SectorInfo[ SECTOR( sMapX, sMapY ) ] );

	pSectorInfo->ubNumberOfCivsAtLevel[ ubRank ] += ubHowMany;

	// update the screen display
	fMapPanelDirty = TRUE;
}


// promote militias of a certain rank
static void StrategicPromoteMilitiaInSector(INT16 sMapX, INT16 sMapY, UINT8 ubCurrentRank, UINT8 ubHowMany)
{
	SECTORINFO *pSectorInfo = &( SectorInfo[ SECTOR( sMapX, sMapY ) ] );

	// damn well better have that many around to promote!
	Assert(pSectorInfo->ubNumberOfCivsAtLevel[ ubCurrentRank ] >= ubHowMany);

	//KM : July 21, 1999 patch fix
	if( pSectorInfo->ubNumberOfCivsAtLevel[ ubCurrentRank ] < ubHowMany )
	{
		return;
	}

	pSectorInfo->ubNumberOfCivsAtLevel[ ubCurrentRank     ] -= ubHowMany;
	pSectorInfo->ubNumberOfCivsAtLevel[ ubCurrentRank + 1 ] += ubHowMany;

	// update the screen display
	fMapPanelDirty = TRUE;
}


void StrategicRemoveMilitiaFromSector(INT16 sMapX, INT16 sMapY, UINT8 ubRank, UINT8 ubHowMany)
{
	SECTORINFO *pSectorInfo = &( SectorInfo[ SECTOR( sMapX, sMapY ) ] );

	// damn well better have that many around to remove!
	Assert(pSectorInfo->ubNumberOfCivsAtLevel[ ubRank ] >= ubHowMany);

	//KM : July 21, 1999 patch fix
	if( pSectorInfo->ubNumberOfCivsAtLevel[ ubRank ] < ubHowMany )
	{
		return;
	}

	pSectorInfo->ubNumberOfCivsAtLevel[ ubRank ] -= ubHowMany;

	// update the screen display
	fMapPanelDirty = TRUE;
}


// kill pts are (2 * kills) + assists
UINT8 CheckOneMilitiaForPromotion(INT16 sMapX, INT16 sMapY, UINT8 ubCurrentRank, UINT8 ubRecentKillPts)
{
	UINT32 uiChanceToLevel = 0;

	switch( ubCurrentRank )
	{
		case GREEN_MILITIA:
			// 2 kill pts minimum
			if (ubRecentKillPts >= 2)
			{
				// 25% chance per kill pt
				uiChanceToLevel = 25 * ubRecentKillPts;
			}
			break;
		case REGULAR_MILITIA:
			// 5 kill pts minimum
			if (ubRecentKillPts >= 5)
			{
				// 10% chance per kill pt.
				uiChanceToLevel = 10 * ubRecentKillPts;
			}
			break;
		case ELITE_MILITIA:
			return 0;
	}
	// roll the bones, and see if he makes it
	if (Random(100) < uiChanceToLevel)
	{
		StrategicPromoteMilitiaInSector(sMapX, sMapY, ubCurrentRank, 1);
		if( ubCurrentRank == GREEN_MILITIA )
		{ //Attempt yet another level up if sufficient points
			if( ubRecentKillPts > 2 )
			{
				if( CheckOneMilitiaForPromotion( sMapX, sMapY, REGULAR_MILITIA, (UINT8)(ubRecentKillPts - 2) ) )
				{ //success, this militia was promoted twice
					return 2;
				}
			}
		}
		return 1;
	}
	return 0;
}


// call this if the player attacks his own militia
static void HandleMilitiaDefections(INT16 sMapX, INT16 sMapY)
{
	UINT8 ubRank;
	UINT8 ubMilitiaCnt;
	UINT8 ubCount;
	UINT32 uiChanceToDefect;

	for( ubRank = 0; ubRank < MAX_MILITIA_LEVELS; ubRank++ )
	{
		ubMilitiaCnt = MilitiaInSectorOfRank(sMapX, sMapY, ubRank);

		// check each guy at each rank to see if he defects
		for (ubCount = 0; ubCount < ubMilitiaCnt; ubCount++)
		{
			switch( ubRank )
			{
				case GREEN_MILITIA:
					uiChanceToDefect = 50;
					break;
				case REGULAR_MILITIA:
					uiChanceToDefect = 75;
					break;
				case ELITE_MILITIA:
					uiChanceToDefect = 90;
					break;
				default:
					Assert( 0 );
					return;
			}

			// roll the bones; should I stay or should I go now?  (for you music fans out there)
			if (Random(100) < uiChanceToDefect)
			{
				//B'bye!  (for you SNL fans out there)
				StrategicRemoveMilitiaFromSector(sMapX, sMapY, ubRank, 1);
			}
		}
	}
}


UINT8 CountAllMilitiaInSector(INT16 sMapX, INT16 sMapY)
{
	UINT8 ubMilitiaTotal = 0;
	UINT8 ubRank;

	// find out if there are any town militia in this SECTOR (don't care about other sectors in same town)
	for( ubRank = 0; ubRank < MAX_MILITIA_LEVELS; ubRank++ )
	{
		ubMilitiaTotal += MilitiaInSectorOfRank(sMapX, sMapY, ubRank);
	}

	return(ubMilitiaTotal);
}


UINT8 MilitiaInSectorOfRank(INT16 sMapX, INT16 sMapY, UINT8 ubRank)
{
	return( SectorInfo[ SECTOR( sMapX, sMapY ) ].ubNumberOfCivsAtLevel[ ubRank ] );
}


BOOLEAN SectorOursAndPeaceful( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	// if this sector is currently loaded
	if ( ( sMapX == gWorldSectorX ) && ( sMapY == gWorldSectorY ) && ( bMapZ == gbWorldSectorZ ) )
	{
		// and either there are enemies prowling this sector, or combat is in progress
		if ( gTacticalStatus.fEnemyInSector || ( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			return FALSE;
		}
	}

	// if sector is controlled by enemies, it's not ours (duh!)
	if (!bMapZ && StrategicMap[sMapX + sMapY * MAP_WORLD_X].fEnemyControlled)
	{
		return FALSE;
	}

	if( NumHostilesInSector( sMapX, sMapY, bMapZ ) )
	{
		return FALSE;
	}

	// safe & secure, s'far as we can tell
	return(TRUE);
}


static void InitFriendlyTownSectorServer(UINT8 ubTownId, INT16 sSkipSectorX, INT16 sSkipSectorY)
{
	// reset globals
	gubTownSectorServerTownId = ubTownId;
	gsTownSectorServerSkipX = sSkipSectorX;
	gsTownSectorServerSkipY = sSkipSectorY;

	gubTownSectorServerIndex = 0;
}


// this feeds the X,Y of the next town sector on the town list for the town specified at initialization
// it will skip an entry that matches the skip X/Y value if one was specified at initialization
// MUST CALL InitFriendlyTownSectorServer() before using!!!
static BOOLEAN ServeNextFriendlySectorInTown(INT16* sNeighbourX, INT16* sNeighbourY)
{
	INT32 iTownSector;
	INT16 sMapX, sMapY;
	BOOLEAN fStopLooking = FALSE;

	do
	{
		// have we reached the end of the town list?
		if (pTownNamesList[ gubTownSectorServerIndex ] == BLANK_SECTOR)
		{
			// end of list reached
			return(FALSE);
		}

		iTownSector = pTownLocationsList[ gubTownSectorServerIndex ];

		// if this sector is in the town we're looking for
		if( StrategicMap[ iTownSector ].bNameId == gubTownSectorServerTownId )
		{
			// A sector in the specified town.  Calculate its X & Y sector compotents
			sMapX = iTownSector % MAP_WORLD_X;
			sMapY = iTownSector / MAP_WORLD_X;

			// Make sure we're not supposed to skip it
			if ( ( sMapX != gsTownSectorServerSkipX ) || ( sMapY != gsTownSectorServerSkipY ) )
			{
				// check if it's "friendly" - not enemy controlled, no enemies in it, no combat in progress
				if (SectorOursAndPeaceful( sMapX, sMapY, 0 ))
				{
					// then that's it!
					*sNeighbourX = sMapX;
					*sNeighbourY = sMapY;

					fStopLooking = TRUE;
				}
			}
		}

		// advance to next entry in town list
		gubTownSectorServerIndex++;

	} while ( !fStopLooking );


	// found & returning a valid sector
	return(TRUE);
}


static void CantTrainMilitiaOkBoxCallback(MessageBoxReturnValue);
static INT32 GetNumberOfUnpaidTrainableSectors(void);
static void PayMilitiaTrainingYesNoBoxCallback(MessageBoxReturnValue);


void HandleInterfaceMessageForCostOfTrainingMilitia( SOLDIERTYPE *pSoldier )
{
	CHAR16 sString[ 128 ];
	INT32 iNumberOfSectors = 0;

	pMilitiaTrainerSoldier = pSoldier;

	// grab total number of sectors
	iNumberOfSectors = GetNumberOfUnpaidTrainableSectors( );
	Assert( iNumberOfSectors > 0 );

	// get total cost
	giTotalCostOfTraining = MILITIA_TRAINING_COST * iNumberOfSectors;
	Assert( giTotalCostOfTraining > 0 );

	gfYesNoPromptIsForContinue = FALSE;

	if( LaptopSaveInfo.iCurrentBalance < giTotalCostOfTraining )
	{
		swprintf(sString, lengthof(sString), pMilitiaConfirmStrings[7], giTotalCostOfTraining);
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	// ok to start training, ask player


	if( iNumberOfSectors > 1 )
	{
		swprintf(sString, lengthof(sString), pMilitiaConfirmStrings[6], iNumberOfSectors, giTotalCostOfTraining, pMilitiaConfirmStrings[1]);
	}
	else
	{
		swprintf( sString, lengthof(sString), L"%ls%d. %ls", pMilitiaConfirmStrings[ 0 ], giTotalCostOfTraining, pMilitiaConfirmStrings[ 1 ] );
	}

	// if we are in mapscreen, make a pop up
	if( guiCurrentScreen == MAP_SCREEN )
	{
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_YESNO, PayMilitiaTrainingYesNoBoxCallback );
	}
	else
	{
		SGPBox const centering_rect = { 0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y };
		DoMessageBox(MSG_BOX_BASIC_STYLE, sString, GAME_SCREEN, MSG_BOX_FLAG_YESNO, PayMilitiaTrainingYesNoBoxCallback, &centering_rect);
	}
}


static void DoContinueMilitiaTrainingMessageBox(INT16 const sSectorX, INT16 const sSectorY, wchar_t const* const str, MessageBoxFlags const usFlags, MSGBOX_CALLBACK const ReturnCallback)
{
	if( sSectorX <= 10 && sSectorY >= 6 && sSectorY <= 11 )
	{
		DoLowerScreenIndependantMessageBox( str, usFlags, ReturnCallback );
	}
	else
	{
		DoScreenIndependantMessageBox( str, usFlags, ReturnCallback );
	}
}


// continue training?
static void HandleInterfaceMessageForContinuingTrainingMilitia(SOLDIERTYPE* const pSoldier)
{
	CHAR16 sString[ 128 ];
	INT16 sSectorX = 0, sSectorY = 0;
	CHAR16 sStringB[ 128 ];
	INT8 bTownId;


	sSectorX = pSoldier->sSectorX;
	sSectorY = pSoldier->sSectorY;

	Assert(!SectorInfo[SECTOR(sSectorX, sSectorY)].fMilitiaTrainingPaid);

	pMilitiaTrainerSoldier = pSoldier;

	gfYesNoPromptIsForContinue = TRUE;

	// is there enough loyalty to continue training
	if (!DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(pSoldier))
	{
		// loyalty too low to continue training
		swprintf(sString, lengthof(sString), pMilitiaConfirmStrings[8], pTownNames[ GetTownIdForSector(sSectorX, sSectorY)], MIN_RATING_TO_TRAIN_TOWN);
		DoContinueMilitiaTrainingMessageBox( sSectorX, sSectorY, sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	if (IsAreaFullOfMilitia(sSectorX, sSectorY, pSoldier->bSectorZ))
	{
		// we're full!!! go home!
		bTownId = GetTownIdForSector( sSectorX, sSectorY );
		if ( bTownId == BLANK_SECTOR )
		{
			// wilderness SAM site
			GetSectorIDString( sSectorX, sSectorY, 0, sStringB, lengthof(sStringB), TRUE );
			swprintf(sString, lengthof(sString), pMilitiaConfirmStrings[9], sStringB);
		}
		else
		{
			// town
			swprintf(sString, lengthof(sString), pMilitiaConfirmStrings[9], pTownNames[bTownId], MIN_RATING_TO_TRAIN_TOWN);
		}
		DoContinueMilitiaTrainingMessageBox( sSectorX, sSectorY, sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	// continue training always handles just one sector at a time
	giTotalCostOfTraining = MILITIA_TRAINING_COST;

	// can player afford to continue training?
	if( LaptopSaveInfo.iCurrentBalance < giTotalCostOfTraining )
	{
		// can't afford to continue training
		swprintf(sString, lengthof(sString), pMilitiaConfirmStrings[7], giTotalCostOfTraining);
		DoContinueMilitiaTrainingMessageBox( sSectorX, sSectorY, sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	// ok to continue, ask player

	GetSectorIDString( sSectorX, sSectorY, 0, sStringB, lengthof(sStringB), TRUE );
	swprintf( sString, lengthof(sString), pMilitiaConfirmStrings[ 3 ], sStringB, pMilitiaConfirmStrings[ 4 ], giTotalCostOfTraining );

	// ask player whether he'd like to continue training
	//DoContinueMilitiaTrainingMessageBox( sSectorX, sSectorY, sString, MSG_BOX_FLAG_YESNO, PayMilitiaTrainingYesNoBoxCallback );
	DoMapMessageBox( MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_YESNO, PayMilitiaTrainingYesNoBoxCallback );
}


static void ContinueTrainingInThisSector(void);
static void MilitiaTrainingRejected(void);
static void StartTrainingInAllUnpaidTrainableSectors(void);


// IMPORTANT: This same callback is used both for initial training and for continue training prompt
// use 'gfYesNoPromptIsForContinue' flag to tell them apart
static void PayMilitiaTrainingYesNoBoxCallback(MessageBoxReturnValue const bExitValue)
{
	Assert( giTotalCostOfTraining > 0 );

	// yes
  if( bExitValue == MSG_BOX_RETURN_YES )
	{
		// does the player have enough
		if( LaptopSaveInfo.iCurrentBalance >= giTotalCostOfTraining )
		{
			if( gfYesNoPromptIsForContinue )
			{
				ContinueTrainingInThisSector();
			}
			else
			{
				StartTrainingInAllUnpaidTrainableSectors();
			}

#ifdef JA2BETAVERSION
			// put this BEFORE training gets handled to avoid detecting an error everytime a sector completes training
			VerifyTownTrainingIsPaidFor();
#endif

			// this completes the training prompt sequence
			pMilitiaTrainerSoldier = NULL;
		}
		else	// can't afford it
		{
			StopTimeCompression();
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMilitiaConfirmStrings[2], MAP_SCREEN, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback);
		}
	}
	else if( bExitValue == MSG_BOX_RETURN_NO )
	{
		StopTimeCompression();

		MilitiaTrainingRejected();
	}
}


static void CantTrainMilitiaOkBoxCallback(MessageBoxReturnValue const bExitValue)
{
	MilitiaTrainingRejected();
}


// IMPORTANT: This same callback is used both for initial training and for continue training prompt
// use 'gfYesNoPromptIsForContinue' flag to tell them apart
static void MilitiaTrainingRejected(void)
{
	if( gfYesNoPromptIsForContinue )
	{
		// take all mercs in that sector off militia training
		ResetAssignmentOfMercsThatWereTrainingMilitiaInThisSector( pMilitiaTrainerSoldier->sSectorX, pMilitiaTrainerSoldier->sSectorY );
	}
	else
	{
		// take all mercs in unpaid sectors EVERYWHERE off militia training
		ResetAssignmentsForMercsTrainingUnpaidSectorsInSelectedList();
	}

#ifdef JA2BETAVERSION
	// put this BEFORE training gets handled to avoid detecting an error everytime a sector completes training
	VerifyTownTrainingIsPaidFor();
#endif

	// this completes the training prompt sequence
	pMilitiaTrainerSoldier = NULL;
}



void HandleMilitiaStatusInCurrentMapBeforeLoadingNewMap( void )
{
	if ( gTacticalStatus.Team[ MILITIA_TEAM ].bSide != 0 )
	{
		// handle militia defections and reset team to friendly
		HandleMilitiaDefections( gWorldSectorX, gWorldSectorY );
		gTacticalStatus.Team[ MILITIA_TEAM ].bSide = 0;
	}
	else if( !gfAutomaticallyStartAutoResolve )
	{ //Don't promote militia if we are going directly to autoresolve to finish the current battle.
		HandleMilitiaPromotions();
	}
}


BOOLEAN CanNearbyMilitiaScoutThisSector( INT16 sSectorX, INT16 sSectorY )
{
	INT16 sSectorValue = 0;
	INT16 sCounterA = 0, sCounterB = 0;
	UINT8 ubScoutingRange = 1;

	for( sCounterA = sSectorX - ubScoutingRange; sCounterA <= sSectorX + ubScoutingRange; sCounterA++ )
	{
		for( sCounterB = sSectorY - ubScoutingRange; sCounterB <= sSectorY + ubScoutingRange; sCounterB++ )
		{
			// skip out of bounds sectors
			if ( ( sCounterA < 1 ) || ( sCounterA > 16 ) || ( sCounterB < 1 ) || ( sCounterB > 16 ) )
			{
				continue;
			}

			sSectorValue = SECTOR( sCounterA, sCounterB );

			// check if any sort of militia here
			if( SectorInfo[ sSectorValue ].ubNumberOfCivsAtLevel[ GREEN_MILITIA ] )
			{
				return( TRUE );
			}
			else if( SectorInfo[ sSectorValue ].ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] )
			{
				return( TRUE );
			}
			else if( SectorInfo[ sSectorValue ].ubNumberOfCivsAtLevel[ ELITE_MILITIA ] )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


BOOLEAN IsAreaFullOfMilitia(const INT16 sector_x, const INT16 sector_y, const INT8 sector_z)
{
	if (sector_z != 0) return TRUE;

	UINT32 count_milita = 0;
	UINT32 max_milita   = 0;

	const INT8 town_id = GetTownIdForSector(sector_x, sector_y);
	if (town_id != BLANK_SECTOR)
	{
		for (INT32 i = 0; pTownNamesList[i] != BLANK_SECTOR; ++i)
		{
			if (pTownNamesList[i] != town_id) continue;
			const INT16 town_x = pTownLocationsList[i] % MAP_WORLD_X;
			const INT16 town_y = pTownLocationsList[i] / MAP_WORLD_X;
			if (SectorOursAndPeaceful(town_x, town_y, 0))
			{
				// don't count GREEN militia, they can be trained into regulars first
				count_milita += MilitiaInSectorOfRank(town_x, town_y, REGULAR_MILITIA);
				count_milita += MilitiaInSectorOfRank(town_x, town_y, ELITE_MILITIA);
				max_milita   += MAX_ALLOWABLE_MILITIA_PER_SECTOR;
			}
		}
	}
	else if (IsThisSectorASAMSector(sector_x, sector_y, 0))
	{
		// don't count GREEN militia, they can be trained into regulars first
		count_milita += MilitiaInSectorOfRank(sector_x, sector_y, REGULAR_MILITIA);
		count_milita += MilitiaInSectorOfRank(sector_x, sector_y, ELITE_MILITIA);
		max_milita   += MAX_ALLOWABLE_MILITIA_PER_SECTOR;
	}

	return count_milita >= max_milita;
}


// handle completion of assignment by this soldier too and inform the player
static void HandleCompletionOfTownTrainingByGroupWithTrainer(SOLDIERTYPE* pTrainer)
{

	INT16 sSectorX = 0, sSectorY = 0;
	INT8 bSectorZ = 0;

	// get the sector values
	sSectorX = pTrainer->sSectorX;
	sSectorY = pTrainer->sSectorY;
	bSectorZ = pTrainer->bSectorZ;

	CFOR_ALL_IN_CHAR_LIST(c)
	{
		SOLDIERTYPE* const pSoldier = c->merc;
		if( ( pSoldier->bAssignment == TRAIN_TOWN ) && ( pSoldier->sSectorX == sSectorX )&&( pSoldier->sSectorY == sSectorY )&&( pSoldier->bSectorZ == bSectorZ ) )
		{
			// done assignment
			AssignmentDone( pSoldier, FALSE, FALSE );
		}
	}
}


void AddSectorForSoldierToListOfSectorsThatCompletedMilitiaTraining(SOLDIERTYPE* pSoldier)
{
	INT32 iCounter = 0;
	INT16 sSector = 0, sCurrentSector = 0;

	// get the sector value
	sSector = pSoldier->sSectorX + pSoldier->sSectorY * MAP_WORLD_X;

	while (g_list_of_merc_in_sectors_completed_militia_training[iCounter] != NULL)
	{
		// get the current soldier
		const SOLDIERTYPE* const pCurrentSoldier = g_list_of_merc_in_sectors_completed_militia_training[iCounter];

		// get the current sector value
		sCurrentSector = pCurrentSoldier->sSectorX + pCurrentSoldier->sSectorY * MAP_WORLD_X;

		// is the merc's sector already in the list?
		if( sCurrentSector == sSector )
		{
			// already here
			return;
		}

		iCounter++;

		Assert( iCounter < SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST );
	}

	// add merc to the list
	g_list_of_merc_in_sectors_completed_militia_training[iCounter] = pSoldier;
}

// clear out the list of training sectors...should be done once the list is posted
void ClearSectorListForCompletedTrainingOfMilitia( void )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST; iCounter++ )
	{
		g_list_of_merc_in_sectors_completed_militia_training[iCounter] = NULL;
	}
}


void HandleContinueOfTownTraining( void )
{
	INT32 iCounter = 0;
	BOOLEAN fContinueEventPosted = FALSE;


	while (g_list_of_merc_in_sectors_completed_militia_training[iCounter] != NULL)
	{
		// get the soldier
		SOLDIERTYPE& s = *g_list_of_merc_in_sectors_completed_militia_training[iCounter];
		if (s.bActive)
		{
			fContinueEventPosted = TRUE;

			class DialogueEventContinueTrainingMilitia : public CharacterDialogueEvent
			{
				public:
					DialogueEventContinueTrainingMilitia(SOLDIERTYPE& soldier) : CharacterDialogueEvent(soldier) {}

					bool Execute()
					{
						HandleInterfaceMessageForContinuingTrainingMilitia(&soldier_);
						return false;
					}
			};

			DialogueEvent::Add(new DialogueEventContinueTrainingMilitia(s));
		}

		// next entry
		iCounter++;
	}

	// now clear the list
	ClearSectorListForCompletedTrainingOfMilitia( );

	if( fContinueEventPosted )
	{
    // ATE: If this event happens in tactical mode we will be switching at some time to mapscreen...
    if ( guiCurrentScreen == GAME_SCREEN )
    {
		  gfEnteringMapScreen = TRUE;
    }

		//If the militia view isn't currently active, then turn it on when prompting to continue training.
		if ( !fShowMilitia )
    {
		  ToggleShowMilitiaMode();
    }
	}
}


static void BuildListOfUnpaidTrainableSectors(void)
{
	INT32 iCounter = 0, iCounterB = 0;

	memset( gsUnpaidStrategicSector, 0, sizeof( INT16 ) * MAX_CHARACTER_COUNT );

	if( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN )
	{
		for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
		{
			const MapScreenCharacterSt* const c        = &gCharactersList[iCounter];
			const SOLDIERTYPE* const          pSoldier = c->merc;
			if (pSoldier == NULL) continue;

			if ((c->selected || iCounter == bSelectedAssignChar) &&
					CanCharacterTrainMilitia(pSoldier)               &&
					!SectorInfo[SECTOR(pSoldier->sSectorX, pSoldier->sSectorY)].fMilitiaTrainingPaid)
			{
				// check to see if this sector is a town and needs equipment
				gsUnpaidStrategicSector[iCounter] = CALCULATE_STRATEGIC_INDEX(pSoldier->sSectorX, pSoldier->sSectorY);
			}
		}
	}
	else
	{
		// handle for tactical
		const SOLDIERTYPE* const pSoldier = gUIFullTarget;
		iCounter = 0;

		if (CanCharacterTrainMilitia(pSoldier))
		{
			if (!SectorInfo[SECTOR(pSoldier->sSectorX, pSoldier->sSectorY)].fMilitiaTrainingPaid)
			{
				// check to see if this sector is a town and needs equipment
				gsUnpaidStrategicSector[ iCounter ] = CALCULATE_STRATEGIC_INDEX( pSoldier->sSectorX, pSoldier->sSectorY );
			}
		}
	}

	// now clean out repeated sectors
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT - 1; iCounter++ )
	{
		if( gsUnpaidStrategicSector[ iCounter ] > 0 )
		{
			for( iCounterB = iCounter + 1 ; iCounterB < MAX_CHARACTER_COUNT; iCounterB++ )
			{
				if( gsUnpaidStrategicSector[ iCounterB ] == gsUnpaidStrategicSector[ iCounter ] )
				{
					gsUnpaidStrategicSector[ iCounterB ] = 0;
				}
			}
		}
	}
}


static INT32 GetNumberOfUnpaidTrainableSectors(void)
{
	INT32 iCounter = 0;
	INT32 iNumberOfSectors = 0;

	BuildListOfUnpaidTrainableSectors();

	// now count up the results
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gsUnpaidStrategicSector[ iCounter ] > 0 )
		{
			iNumberOfSectors++;
		}
	}

	// return the result
	return( iNumberOfSectors );

}


static void PayForTrainingInSector(UINT8 ubSector);


static void StartTrainingInAllUnpaidTrainableSectors(void)
{
	INT32 iCounter = 0;
	UINT8 ubSector;


	SetAssignmentForList( TRAIN_TOWN, 0 );

	BuildListOfUnpaidTrainableSectors();

	// pay up in each sector
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gsUnpaidStrategicSector[ iCounter ] > 0 )
		{
			// convert strategic sector to 0-255 system
			ubSector = STRATEGIC_INDEX_TO_SECTOR_INFO( gsUnpaidStrategicSector[ iCounter ] );
			PayForTrainingInSector( ubSector );
		}
	}
}


static void ContinueTrainingInThisSector(void)
{
	UINT8 ubSector;

	Assert( pMilitiaTrainerSoldier );

	// pay up in the sector where pMilitiaTrainerSoldier is
	ubSector = SECTOR( pMilitiaTrainerSoldier->sSectorX, pMilitiaTrainerSoldier->sSectorY );
	PayForTrainingInSector( ubSector );
}


static void ResetDoneFlagForAllMilitiaTrainersInSector(UINT8 ubSector);


static void PayForTrainingInSector(UINT8 ubSector)
{
	Assert(!SectorInfo[ubSector].fMilitiaTrainingPaid);

	// spend the money
	AddTransactionToPlayersBook( TRAIN_TOWN_MILITIA, ubSector, GetWorldTotalMin(), -( MILITIA_TRAINING_COST ) );

	// mark this sector sectors as being paid up
	SectorInfo[ ubSector ].fMilitiaTrainingPaid = TRUE;

	// reset done flags
	ResetDoneFlagForAllMilitiaTrainersInSector( ubSector );
}


static void ResetDoneFlagForAllMilitiaTrainersInSector(UINT8 ubSector)
{
	FOR_ALL_IN_TEAM(pSoldier, OUR_TEAM)
	{
		if (pSoldier->bAssignment == TRAIN_TOWN &&
				SECTOR(pSoldier->sSectorX, pSoldier->sSectorY) == ubSector &&
				pSoldier->bSectorZ == 0)
		{
			pSoldier->fDoneAssignmentAndNothingToDoFlag = FALSE;
			pSoldier->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;
		}
	}
}


BOOLEAN MilitiaTrainingAllowedInSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	INT8 bTownId;
	BOOLEAN fSamSitePresent = FALSE;


	if( bSectorZ != 0 )
	{
		return( FALSE );
	}

	fSamSitePresent = IsThisSectorASAMSector( sSectorX, sSectorY, bSectorZ );

	if( fSamSitePresent )
	{
		// all SAM sites may have militia trained at them
		return(TRUE);
	}


	bTownId = GetTownIdForSector( sSectorX, sSectorY );


	return( MilitiaTrainingAllowedInTown( bTownId ) );
}



BOOLEAN MilitiaTrainingAllowedInTown( INT8 bTownId )
{
	switch ( bTownId )
	{
		case DRASSEN:
		case ALMA:
		case GRUMM:
		case CAMBRIA:
		case BALIME:
		case MEDUNA:
		case CHITZENA:
			return(TRUE);

		case OMERTA:
		case ESTONI:
		case SAN_MONA:
		case TIXA:
		case ORTA:
			// can't keep militia in these towns
			return(FALSE);

		case BLANK_SECTOR:
		default:
			// not a town sector!
			return(FALSE);

	}
}

void BuildMilitiaPromotionsString( wchar_t *str, size_t Length)
{
	wchar_t pStr[256];
	BOOLEAN fAddSpace = FALSE;
	swprintf( str, Length, L"" );

	if( !gbMilitiaPromotions )
	{
		return;
	}
	if( gbGreenToElitePromotions > 1 )
	{
		swprintf( pStr, lengthof(pStr), gzLateLocalizedString[22], gbGreenToElitePromotions );
		wcscat( str, pStr );
		fAddSpace = TRUE;
	}
	else if( gbGreenToElitePromotions == 1 )
	{
		wcscat( str, gzLateLocalizedString[29] );
		fAddSpace = TRUE;
	}

	if( gbGreenToRegPromotions > 1 )
	{
		if( fAddSpace )
		{
			wcscat( str, L" " );
		}
		swprintf( pStr, lengthof(pStr), gzLateLocalizedString[23], gbGreenToRegPromotions );
		wcscat( str, pStr );
		fAddSpace = TRUE;
	}
	else if( gbGreenToRegPromotions == 1 )
	{
		if( fAddSpace )
		{
			wcscat( str, L" " );
		}
		wcscat( str, gzLateLocalizedString[30] );
		fAddSpace = TRUE;
	}

	if( gbRegToElitePromotions > 1 )
	{
		if( fAddSpace )
		{
			wcscat( str, L" " );
		}
		swprintf( pStr, lengthof(pStr), gzLateLocalizedString[24], gbRegToElitePromotions );
		wcscat( str, pStr );
	}
	else if( gbRegToElitePromotions == 1 )
	{
		if( fAddSpace )
		{
			wcscat( str, L" " );
		}
		wcscat( str, gzLateLocalizedString[31] );
		fAddSpace = TRUE;
	}

	//Clear the fields
	gbGreenToElitePromotions = 0;
	gbGreenToRegPromotions = 0;
	gbRegToElitePromotions = 0;
	gbMilitiaPromotions = 0;
}
