#include "MapScreen.h"
#include "MessageBoxScreen.h"
#include "SAM_Sites.h"
#include "Town_Militia.h"
#include "Militia_Control.h"
#include "Campaign_Types.h"
#include "StrategicMap.h"
#include "Overhead.h"
#include "Strategic_Town_Loyalty.h"
#include "Random.h"
#include "Text.h"
#include "Map_Screen_Interface.h"
#include "LaptopSave.h"
#include "Finances.h"
#include "Game_Clock.h"
#include "Assignments.h"
#include "Dialogue_Control.h"
#include "Queen_Command.h"
#include "PreBattle_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Map.h"
#include "JAScreens.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "UILayout.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "TownModel.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>

#define SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST 50

// temporary local global variables
UINT8 gubTownSectorServerTownId = BLANK_SECTOR;
SGPSector gsTownSectorServerSkip(-1, -1);
UINT8 gubTownSectorServerIndex = 0;
BOOLEAN gfYesNoPromptIsForContinue = FALSE; // this flag remembers whether we're starting new training, or continuing
INT32 giTotalCostOfTraining = 0;


//the completed list of sector soldiers for training militia
static SOLDIERTYPE* g_list_of_merc_in_sectors_completed_militia_training[SIZE_OF_MILITIA_COMPLETED_TRAINING_LIST];
SOLDIERTYPE *pMilitiaTrainerSoldier = NULL;

// note that these sector values are STRATEGIC INDEXES, not 0-255!
static INT16 gsUnpaidStrategicSector[MAX_CHARACTER_COUNT];


static void HandleCompletionOfTownTrainingByGroupWithTrainer(SOLDIERTYPE* pTrainer);
static void InitFriendlyTownSectorServer(UINT8 ubTownId, INT16 sSkipSectorX, INT16 sSkipSectorY);
static bool ServeNextFriendlySectorInTown(SGPSector& neighbour);
static void StrategicAddMilitiaToSector(const SGPSector& sMap, UINT8 ubRank, UINT8 ubHowMany);
static void StrategicPromoteMilitiaInSector(const SGPSector& sMap, UINT8 current_rank, UINT8 n);


void TownMilitiaTrainingCompleted(SOLDIERTYPE *pTrainer, const SGPSector& sector)
{
	UINT8 ubMilitiaTrained = 0;
	BOOLEAN fFoundOne;
	UINT8 ubTownId;


	// get town index
	ubTownId = StrategicMap[sector.AsStrategicIndex()].bNameId;

	if( ubTownId == BLANK_SECTOR )
	{
		Assert(IsThisSectorASAMSector(sector));
	}


	// force tactical to update militia status
	gfStrategicMilitiaChangesMade = TRUE;

	// ok, so what do we do with all this training?  Well, in order of decreasing priority:
	// 1) If there's room in training sector, create new GREEN militia guys there
	// 2) If not enough room there, create new GREEN militia guys in friendly sectors of the same town
	// 3) If not enough room anywhere in town, promote a number of GREENs in this sector into regulars
	// 4) If not enough GREENS there to promote, promote GREENs in other sectors.
	// 5) If all friendly sectors of this town are completely filled with REGULAR militia, then training effect is wasted

	SGPSector sNeighbour;
	while (ubMilitiaTrained < MILITIA_TRAINING_SQUAD_SIZE)
	{
		// is there room for another militia in the training sector itself?
		if (CountAllMilitiaInSector(sector) < MAX_ALLOWABLE_MILITIA_PER_SECTOR)
		{
			// great! Create a new GREEN militia guy in the training sector
			StrategicAddMilitiaToSector(sector, GREEN_MILITIA, 1);
		}
		else
		{
			fFoundOne = FALSE;

			if( ubTownId != BLANK_SECTOR )
			{
				InitFriendlyTownSectorServer(ubTownId, sector.x, sector.y);

				// check other eligible sectors in this town for room for another militia
				while (ServeNextFriendlySectorInTown(sNeighbour))
				{
					// is there room for another militia in this neighbouring sector ?
					if (CountAllMilitiaInSector(sNeighbour) < MAX_ALLOWABLE_MILITIA_PER_SECTOR)
					{
						// great! Create a new GREEN militia guy in the neighbouring sector
						StrategicAddMilitiaToSector(sNeighbour, GREEN_MILITIA, 1);

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
				if (MilitiaInSectorOfRank(sector, GREEN_MILITIA) > 0)
				{
					// great! Promote a GREEN militia guy in the training sector to a REGULAR
					StrategicPromoteMilitiaInSector(sector, GREEN_MILITIA, 1);
				}
				else
				{
					if( ubTownId != BLANK_SECTOR )
					{
						// dammit! Last chance - try to find other eligible sectors in the same town with a Green guy to be promoted
						InitFriendlyTownSectorServer(ubTownId, sector.x, sector.y);

						// check other eligible sectors in this town for room for another militia
						while (ServeNextFriendlySectorInTown(sNeighbour))
						{
							// are there any GREEN militia men in the neighbouring sector ?
							if (MilitiaInSectorOfRank(sNeighbour, GREEN_MILITIA) > 0)
							{
								// great! Promote a GREEN militia guy in the neighbouring sector to a REGULAR
								StrategicPromoteMilitiaInSector(sNeighbour, GREEN_MILITIA, 1);

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


INT8 SoldierClassToMilitiaRank(UINT8 const soldier_class)
{
	switch (soldier_class)
	{
		case SOLDIER_CLASS_GREEN_MILITIA: return GREEN_MILITIA;
		case SOLDIER_CLASS_REG_MILITIA:   return REGULAR_MILITIA;
		case SOLDIER_CLASS_ELITE_MILITIA: return ELITE_MILITIA;
		default:                          return -1;
	}
}


// add militias of a certain rank
static void StrategicAddMilitiaToSector(const SGPSector& sMap, UINT8 ubRank, UINT8 ubHowMany)
{
	SECTORINFO *pSectorInfo = &(SectorInfo[sMap.AsByte()]);

	pSectorInfo->ubNumberOfCivsAtLevel[ ubRank ] += ubHowMany;

	// update the screen display
	fMapPanelDirty = TRUE;
}


// Promote militias of a certain rank
static void StrategicPromoteMilitiaInSector(const SGPSector& sMap, UINT8 const current_rank, UINT8 const n)
{
	SECTORINFO& si = SectorInfo[sMap.AsByte()];

	Assert(si.ubNumberOfCivsAtLevel[current_rank] >= n);
	//KM : July 21, 1999 patch fix
	if (si.ubNumberOfCivsAtLevel[current_rank] < n) return;

	si.ubNumberOfCivsAtLevel[current_rank    ] -= n;
	si.ubNumberOfCivsAtLevel[current_rank + 1] += n;

	fMapPanelDirty = TRUE;
}


void StrategicRemoveMilitiaFromSector(const SGPSector& sMap, UINT8 ubRank, UINT8 ubHowMany)
{
	SECTORINFO *pSectorInfo = &(SectorInfo[sMap.AsByte()]);

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
UINT8 CheckOneMilitiaForPromotion(const SGPSector& sMap, UINT8 &current_rank, UINT8 kill_points)
{
	// since the awarding is potentially significantly delayed, make sure they
	// weren't all promoted already by regular training or killed;
	// if we can't find them, we try higher ranks and make sure the caller follows up
	SECTORINFO& si = SectorInfo[sMap.AsByte()];
	while (si.ubNumberOfCivsAtLevel[current_rank] == 0) {
		if (current_rank == ELITE_MILITIA) return 0;
		current_rank++;
	}

	UINT8 n_promotions = 0;
	switch (current_rank)
	{
		case GREEN_MILITIA:
			// 2 kill points minimum, 25% chance per kill point
			if (kill_points < 2)           break;
			if (!Chance(25 * kill_points)) break;
			StrategicPromoteMilitiaInSector(sMap, GREEN_MILITIA, 1);
			++n_promotions;
			// Attempt another level up
			kill_points -= 2;
			/* FALLTHROUGH */
		case REGULAR_MILITIA:
			// 5 kill points minimum, 10% chance per kill point
			if (kill_points < 5)           break;
			if (!Chance(10 * kill_points)) break;
			StrategicPromoteMilitiaInSector(sMap, REGULAR_MILITIA, 1);
			++n_promotions;
			break;
	}
	return n_promotions;
}


// call this if the player attacks his own militia
static void HandleMilitiaDefections(const SGPSector& sMap)
{
	UINT8 ubRank;
	UINT8 ubMilitiaCnt;
	UINT8 ubCount;
	UINT32 uiChanceToDefect;

	for( ubRank = 0; ubRank < MAX_MILITIA_LEVELS; ubRank++ )
	{
		ubMilitiaCnt = MilitiaInSectorOfRank(sMap, ubRank);

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
					SLOGA("HandleMilitiaDefections: invalid Rank");
					return;
			}

			// roll the bones; should I stay or should I go now?  (for you music fans out there)
			if (Random(100) < uiChanceToDefect)
			{
				//B'bye!  (for you SNL fans out there)
				StrategicRemoveMilitiaFromSector(sMap, ubRank, 1);
			}
		}
	}
}


UINT8 CountAllMilitiaInSector(const SGPSector& sMap)
{
	UINT8 ubMilitiaTotal = 0;
	UINT8 ubRank;

	// find out if there are any town militia in this SECTOR (don't care about other sectors in same town)
	for( ubRank = 0; ubRank < MAX_MILITIA_LEVELS; ubRank++ )
	{
		ubMilitiaTotal += MilitiaInSectorOfRank(sMap, ubRank);
	}

	return(ubMilitiaTotal);
}


UINT8 MilitiaInSectorOfRank(const SGPSector& sMap, UINT8 ubRank)
{
	return SectorInfo[sMap.AsByte()].ubNumberOfCivsAtLevel[ubRank];
}


BOOLEAN SectorOursAndPeaceful(const SGPSector& sector)
{
	// if this sector is currently loaded
	if (sector == gWorldSector)
	{
		// and either there are enemies prowling this sector, or combat is in progress
		if ( gTacticalStatus.fEnemyInSector || ( gTacticalStatus.uiFlags & INCOMBAT ) )
		{
			return FALSE;
		}
	}

	// if sector is controlled by enemies, it's not ours (duh!)
	if (!sector.z && StrategicMap[sector.AsStrategicIndex()].fEnemyControlled)
	{
		return FALSE;
	}

	if (NumHostilesInSector(sector))
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
	gsTownSectorServerSkip.x = sSkipSectorX;
	gsTownSectorServerSkip.y = sSkipSectorY;

	gubTownSectorServerIndex = 0;
}


/* Fetch the X,Y of the next town sector on the town list for the town specified
 * at initialization. It will skip an entry that matches the skip X/Y value, if
 * one was specified at initialization.
 * MUST CALL InitFriendlyTownSectorServer() before using! */
static bool ServeNextFriendlySectorInTown(SGPSector& neighbour)
{
	while (gubTownSectorServerIndex < g_town_sectors.size() && g_town_sectors[gubTownSectorServerIndex].town != BLANK_SECTOR)
	{
		INT32 const sector = g_town_sectors[gubTownSectorServerIndex++].sector;

		// if this sector is in the town we're looking for
		if (StrategicMap[SGPSector(sector).AsStrategicIndex()].bNameId != gubTownSectorServerTownId) continue;

		SGPSector sec(sector);
		// Make sure we're not supposed to skip it
		if (sec == gsTownSectorServerSkip) continue;

		// check if it's "friendly" - not enemy controlled, no enemies in it, no combat in progress
		if (!SectorOursAndPeaceful(sec)) continue;

		neighbour = sec;
		return true;
	}
	return false;
}


static void CantTrainMilitiaOkBoxCallback(MessageBoxReturnValue);
static INT32 GetNumberOfUnpaidTrainableSectors();
static void PayMilitiaTrainingYesNoBoxCallback(MessageBoxReturnValue);


void HandleInterfaceMessageForCostOfTrainingMilitia( SOLDIERTYPE *pSoldier )
{
	ST::string sString;
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
		sString = st_format_printf(pMilitiaConfirmStrings[7], giTotalCostOfTraining);
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	if( iNumberOfSectors > 1 )
	{
		sString = st_format_printf(pMilitiaConfirmStrings[6], iNumberOfSectors, giTotalCostOfTraining, pMilitiaConfirmStrings[1]);
	}
	else
	{
		sString = ST::format("{}{}. {}", pMilitiaConfirmStrings[ 0 ], giTotalCostOfTraining, pMilitiaConfirmStrings[ 1 ]);
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

// continue training?
static void HandleInterfaceMessageForContinuingTrainingMilitia(SOLDIERTYPE* const pSoldier)
{
	ST::string sString;
	ST::string sStringB;

	UINT8 const sector = pSoldier->sSector.AsByte();

	Assert(!SectorInfo[sector].fMilitiaTrainingPaid);

	pMilitiaTrainerSoldier = pSoldier;

	gfYesNoPromptIsForContinue = TRUE;

	// is there enough loyalty to continue training
	if (!DoesSectorMercIsInHaveSufficientLoyaltyToTrainMilitia(pSoldier))
	{
		// loyalty too low to continue training
		sString = st_format_printf(pMilitiaConfirmStrings[8], GCM->getTownName(GetTownIdForSector(sector)), MIN_RATING_TO_TRAIN_TOWN);
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	if (IsAreaFullOfMilitia(pSoldier->sSector))
	{
		// we're full!!! go home!
		UINT8 const bTownId = GetTownIdForSector(sector);
		if ( bTownId == BLANK_SECTOR )
		{
			// wilderness SAM site
			sStringB = GetSectorIDString(pSoldier->sSector, TRUE);
			sString = st_format_printf(pMilitiaConfirmStrings[9], sStringB);
		}
		else
		{
			// town
			sString = st_format_printf(pMilitiaConfirmStrings[9], GCM->getTownName(bTownId), MIN_RATING_TO_TRAIN_TOWN);
		}
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	// continue training always handles just one sector at a time
	giTotalCostOfTraining = MILITIA_TRAINING_COST;

	// can player afford to continue training?
	if( LaptopSaveInfo.iCurrentBalance < giTotalCostOfTraining )
	{
		// can't afford to continue training
		sString = st_format_printf(pMilitiaConfirmStrings[7], giTotalCostOfTraining);
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, CantTrainMilitiaOkBoxCallback );
		return;
	}

	// ok to continue, ask player

	sStringB = GetSectorIDString(pSoldier->sSector, TRUE);
	sString = st_format_printf(pMilitiaConfirmStrings[ 3 ], sStringB, pMilitiaConfirmStrings[ 4 ], giTotalCostOfTraining);

	// ask player whether he'd like to continue training
	DoMapMessageBox( MSG_BOX_BASIC_STYLE, sString, MAP_SCREEN, MSG_BOX_FLAG_YESNO, PayMilitiaTrainingYesNoBoxCallback );
}


static void ContinueTrainingInThisSector(void);
static void MilitiaTrainingRejected(void);
static void StartTrainingInAllUnpaidTrainableSectors();


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
		ResetAssignmentOfMercsThatWereTrainingMilitiaInThisSector(pMilitiaTrainerSoldier->sSector);
	}
	else
	{
		// take all mercs in unpaid sectors EVERYWHERE off militia training
		ResetAssignmentsForMercsTrainingUnpaidSectorsInSelectedList();
	}

	// this completes the training prompt sequence
	pMilitiaTrainerSoldier = NULL;
}



void HandleMilitiaStatusInCurrentMapBeforeLoadingNewMap( void )
{
	if ( gTacticalStatus.Team[ MILITIA_TEAM ].bSide != 0 )
	{
		// handle militia defections and reset team to friendly
		HandleMilitiaDefections(gWorldSector);
		gTacticalStatus.Team[ MILITIA_TEAM ].bSide = 0;
	}
	else if( !gfAutomaticallyStartAutoResolve )
	{ //Don't promote militia if we are going directly to autoresolve to finish the current battle.
		HandleMilitiaPromotions();
	}
}


bool CanNearbyMilitiaScoutThisSector(const SGPSector& sMap)
{
	INT16 const scout_range = 1;
	INT16 const xstart      = sMap.x >      scout_range ? sMap.x - scout_range :  1;
	INT16 const ystart      = sMap.y >      scout_range ? sMap.y - scout_range :  1;
	INT16 const xend        = sMap.x < 16 - scout_range ? sMap.x + scout_range : 16;
	INT16 const yend        = sMap.y < 16 - scout_range ? sMap.y + scout_range : 16;
	SGPSector sSector;
	for (sSector.y = ystart; sSector.y <= yend; ++sSector.y)
	{
		for (sSector.x = xstart; sSector.x <= xend; ++sSector.x)
		{
			UINT8 (&n_milita)[MAX_MILITIA_LEVELS] = SectorInfo[sSector.AsByte()].ubNumberOfCivsAtLevel;
			if (n_milita[GREEN_MILITIA] + n_milita[REGULAR_MILITIA] + n_milita[ELITE_MILITIA] != 0) return true;
		}
	}
	return false;
}


BOOLEAN IsAreaFullOfMilitia(const SGPSector& sector)
{
	if (sector.z != 0) return TRUE;

	UINT32 count_milita = 0;
	UINT32 max_milita   = 0;

	INT8 const town_id = GetTownIdForSector(sector);
	if (town_id != BLANK_SECTOR)
	{
		FOR_EACH_SECTOR_IN_TOWN(i, town_id)
		{
			SGPSector town(i->sector);
			if (SectorOursAndPeaceful(town))
			{
				// don't count GREEN militia, they can be trained into regulars first
				count_milita += MilitiaInSectorOfRank(town, REGULAR_MILITIA);
				count_milita += MilitiaInSectorOfRank(town, ELITE_MILITIA);
				max_milita   += MAX_ALLOWABLE_MILITIA_PER_SECTOR;
			}
		}
	}
	else if (IsThisSectorASAMSector(sector))
	{
		// don't count GREEN militia, they can be trained into regulars first
		count_milita += MilitiaInSectorOfRank(sector, REGULAR_MILITIA);
		count_milita += MilitiaInSectorOfRank(sector, ELITE_MILITIA);
		max_milita   += MAX_ALLOWABLE_MILITIA_PER_SECTOR;
	}

	return count_milita >= max_milita;
}


// handle completion of assignment by this soldier too and inform the player
static void HandleCompletionOfTownTrainingByGroupWithTrainer(SOLDIERTYPE* pTrainer)
{
	// get the sector values
	SGPSector sSector = pTrainer->sSector;

	CFOR_EACH_IN_CHAR_LIST(c)
	{
		SOLDIERTYPE* const pSoldier = c->merc;
		if (pSoldier->bAssignment == TRAIN_TOWN && pSoldier->sSector == sSector)
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
	sSector = pSoldier->sSector.AsStrategicIndex();

	while (g_list_of_merc_in_sectors_completed_militia_training[iCounter] != NULL)
	{
		// get the current soldier
		const SOLDIERTYPE* const pCurrentSoldier = g_list_of_merc_in_sectors_completed_militia_training[iCounter];

		// get the current sector value
		sCurrentSector = pCurrentSoldier->sSector.AsStrategicIndex();

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
void ClearSectorListForCompletedTrainingOfMilitia()
{
	FOR_EACH(SOLDIERTYPE*, i, g_list_of_merc_in_sectors_completed_militia_training) *i = 0;
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


static void AddIfTrainingUnpaidSector(SOLDIERTYPE const& s)
{
	if (!CanCharacterTrainMilitia(&s)) return;
	// Check if this sector is a town and needs equipment.
	if (SectorInfo[s.sSector.AsByte()].fMilitiaTrainingPaid) return;
	INT16 const sector = s.sSector.AsStrategicIndex();
	for (INT16* i = gsUnpaidStrategicSector;; ++i)
	{
		if (*i == 0)
		{
			*i = sector;
			break;
		}
		if (*i == sector) break; // Do not add duplicate
	}
}


static void BuildListOfUnpaidTrainableSectors()
{
	std::fill(std::begin(gsUnpaidStrategicSector), std::end(gsUnpaidStrategicSector), 0);

	if (fInMapMode)
	{
		for (INT32 i = 0; i != MAX_CHARACTER_COUNT; ++i)
		{
			MapScreenCharacterSt const& c = gCharactersList[i];
			if (!c.merc)                                 continue;
			if (!c.selected && i != bSelectedAssignChar) continue;
			AddIfTrainingUnpaidSector(*c.merc);
		}
	}
	else
	{ // Handle for tactical
		AddIfTrainingUnpaidSector(*pMilitiaTrainerSoldier);
	}
}


static INT32 GetNumberOfUnpaidTrainableSectors()
{
	BuildListOfUnpaidTrainableSectors();
	INT32 n = 0;
	FOR_EACH(INT16 const, i, gsUnpaidStrategicSector)
	{
		if (*i != 0) ++n;
	}
	return n;
}


static void PayForTrainingInSector(UINT8 ubSector);


static void StartTrainingInAllUnpaidTrainableSectors()
{
	SetAssignmentForList(TRAIN_TOWN, 0);
	// Pay up in each sector
	BuildListOfUnpaidTrainableSectors();
	FOR_EACH(INT16 const, i, gsUnpaidStrategicSector)
	{
		if (*i == 0) continue;
		PayForTrainingInSector(SGPSector::FromStrategicIndex(*i).AsByte());
	}
}


static void ContinueTrainingInThisSector(void)
{
	UINT8 ubSector;

	Assert( pMilitiaTrainerSoldier );

	// pay up in the sector where pMilitiaTrainerSoldier is
	ubSector = pMilitiaTrainerSoldier->sSector.AsByte();
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
	FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		if (pSoldier->bAssignment == TRAIN_TOWN &&
				pSoldier->sSector.AsByte() == ubSector &&
				pSoldier->sSector.z == 0)
		{
			pSoldier->fDoneAssignmentAndNothingToDoFlag = FALSE;
			pSoldier->usQuoteSaidExtFlags &= ~SOLDIER_QUOTE_SAID_DONE_ASSIGNMENT;
		}
	}
}


BOOLEAN MilitiaTrainingAllowedInSector(const SGPSector& sector)
{
	BOOLEAN fSamSitePresent = FALSE;

	if (sector.z != 0)
	{
		return( FALSE );
	}

	fSamSitePresent = IsThisSectorASAMSector(sector);

	if( fSamSitePresent )
	{
		// all SAM sites may have militia trained at them
		return(TRUE);
	}

	UINT8 const bTownId = GetTownIdForSector(sector);
	return( MilitiaTrainingAllowedInTown( bTownId ) );
}



BOOLEAN MilitiaTrainingAllowedInTown( INT8 bTownId )
{
	auto town = GCM->getTown(bTownId);
	if (town == NULL) {
		return false;
	}
	return town->isMilitiaTrainingAllowed;
}


static void PromoteMilitia(ST::string& str, INT8 count, const ST::string& singular, const ST::string& plural)
{
	if (count > 0)
	{
		if (!str.empty()) str += " ";
		if (count == 1)
		{
			str += singular;
		}
		else
		{
			str += st_format_printf(plural, count);
		}
	}
}


ST::string BuildMilitiaPromotionsString()
{
	ST::string str;

	if (gbMilitiaPromotions == 0) return {};

	PromoteMilitia(str, gbGreenToElitePromotions, gzLateLocalizedString[STR_LATE_29], gzLateLocalizedString[STR_LATE_22]);
	PromoteMilitia(str, gbGreenToRegPromotions,  gzLateLocalizedString[STR_LATE_30], gzLateLocalizedString[STR_LATE_23]);
	PromoteMilitia(str, gbRegToElitePromotions,  gzLateLocalizedString[STR_LATE_31], gzLateLocalizedString[STR_LATE_24]);

	// Clear the fields
	gbGreenToElitePromotions = 0;
	gbGreenToRegPromotions   = 0;
	gbRegToElitePromotions   = 0;
	gbMilitiaPromotions      = 0;

	return str;
}
