#include "Strategic_Mines.h"

#include "Campaign_Types.h"
#include "ContentManager.h"
#include "Creature_Spreading.h"
#include "Debug.h"
#include "Dialogue_Control.h"
#include "FileMan.h"
#include "Finances.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "History.h"
#include "LoadSaveData.h"
#include "MapScreen.h"
#include "Map_Screen_Interface.h"
#include "Message.h"
#include "MineModel.h"
#include "Quests.h"
#include "Random.h"
#include "Soldier_Profile.h"
#include "StrategicMap.h"
#include "Strategic_AI.h"
#include "Strategic_Town_Loyalty.h"
#include "Text.h"
#include "UILayout.h"


// this .cc file handles the strategic level of mines and income from them

#define REMOVAL_RATE_INCREMENT	250		// the smallest increment by which removal rate change during depletion (use round #s)

#define LOW_MINE_LOYALTY_THRESHOLD	50	// below this the head miner considers his town's population disloyal

// Mine production is being processed 4x daily: 9am ,noon, 3pm, and 6pm.
// This is loosely based on a 6am-6pm working day of 4 "shifts".
#define MINE_PRODUCTION_NUMBER_OF_PERIODS 4						// how many times a day mine production is processed
#define MINE_PRODUCTION_START_TIME				(9 * 60)		// hour of first daily mine production event (in minutes)
#define MINE_PRODUCTION_PERIOD						(3 * 60)		// time seperating daily mine production events (in minutes)
#define MINE_PRODUCTION_DAYS_AFTER_HEAD_MINER_WARNING 2.5					// how many more days the mine can still produce after head miner warns of depletion


// this table holds mine values that change during the course of the game and must be saved
std::vector<MINE_STATUS_TYPE> gMineStatus;

struct HEAD_MINER_TYPE
{
	UINT16 usProfileId;
	INT8   bQuoteNum[NUM_HEAD_MINER_STRATEGIC_QUOTES];
};


static const HEAD_MINER_TYPE gHeadMinerData[NUM_HEAD_MINERS] =
{
	//  Profile #   running out    creatures!   all dead!   creatures again!
	{   FRED,    {      17,           18,          27,             26      } },
	{   MATT,    {      -1,           18,          32,             31      } },
	{   OSWALD,  {      14,           15,          24,             23      } },
	{   CALVIN,  {      14,           15,          24,             23      } },
	{   CARL,    {      14,           15,          24,             23      } }
};


void InitializeMines( void )
{
	UINT8 ubMineIndex;
	MINE_STATUS_TYPE *pMineStatus;
	UINT8 ubMineProductionIncreases;
	UINT8 ubDepletedMineIndex;
	UINT8 ubMinDaysBeforeDepletion = 20;


	// set up initial mine statu
	gMineStatus.clear();
	for (auto mine : GCM->getMines())
	{
		MINE_STATUS_TYPE pMineStatus{};

		pMineStatus.ubMineType = mine->mineType;
		pMineStatus.uiMaxRemovalRate = mine->minimumMineProduction;
		pMineStatus.fEmpty = (pMineStatus.uiMaxRemovalRate == 0) ? TRUE : FALSE;
		gMineStatus.push_back(pMineStatus);
	}

	// randomize the exact size each mine.  The total production is always the same and depends on the game difficulty,
	// but some mines will produce more in one game than another, while others produce less

	// adjust for game difficulty
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
		case DIF_LEVEL_MEDIUM:
			ubMineProductionIncreases = 25;
			break;
		case DIF_LEVEL_HARD:
			ubMineProductionIncreases = 20;
			break;
		default:
			SLOGA("Invalid Difficulty level");
			return;
	}

	auto minesData = GCM->getMines();
	while (ubMineProductionIncreases > 0)
	{
		// pick a producing mine at random and increase its production
		do
		{
			ubMineIndex = ( UINT8 ) Random(minesData.size());
		} while (gMineStatus[ubMineIndex].fEmpty);

		// increase mine production by 20% of the base (minimum) rate
		gMineStatus[ubMineIndex].uiMaxRemovalRate += (minesData[ubMineIndex]->minimumMineProduction / 5);

		ubMineProductionIncreases--;
	}


	// choose which mine will run out of production.  This will never be the Alma mine or an empty mine (San Mona)...
	do
	{
		ubDepletedMineIndex = ( UINT8 ) Random(minesData.size());
		// Try next one if this mine can't run out for quest-related reasons (see Ian)
	} while (gMineStatus[ubDepletedMineIndex].fEmpty || minesData[ubDepletedMineIndex]->noDepletion);


	for( ubMineIndex = 0; ubMineIndex < gMineStatus.size(); ubMineIndex++ )
	{
		pMineStatus = &(gMineStatus[ ubMineIndex ]);

		if (ubMineIndex == ubDepletedMineIndex)
		{
			if (minesData[ubDepletedMineIndex]->delayDepletion)
			{
				ubMinDaysBeforeDepletion = 20;
			}
			else
			{
				ubMinDaysBeforeDepletion = 10;
			}

			// the mine that runs out has only enough ore for this many days of full production
			pMineStatus->uiRemainingOreSupply = ubMinDaysBeforeDepletion * (MINE_PRODUCTION_NUMBER_OF_PERIODS * pMineStatus->uiMaxRemovalRate);

			// ore starts running out when reserves drop to less than 2.5 days worth of supply
			pMineStatus->uiOreRunningOutPoint = (UINT32)(MINE_PRODUCTION_DAYS_AFTER_HEAD_MINER_WARNING * MINE_PRODUCTION_NUMBER_OF_PERIODS * pMineStatus->uiMaxRemovalRate);
		}
		else
		if (!pMineStatus->fEmpty)
		{
			// never runs out...
			pMineStatus->uiRemainingOreSupply = 999999999;		// essentially unlimited
			pMineStatus->uiOreRunningOutPoint = 0;
		}
		else
		{
			// already empty
			pMineStatus->uiRemainingOreSupply = 0;
			pMineStatus->uiOreRunningOutPoint = 0;
		}
	}
}


void HourlyMinesUpdate(void)
{
	UINT8 ubMineIndex;
	MINE_STATUS_TYPE *pMineStatus;

	// check every non-empty mine
	for( ubMineIndex = 0; ubMineIndex < gMineStatus.size(); ubMineIndex++ )
	{
		pMineStatus = &(gMineStatus[ ubMineIndex ]);

		if (pMineStatus->fEmpty)
		{
			// nobody is working that mine, so who cares
			continue;
		}

		// check if the mine has any monster creatures in it
		if (MineClearOfMonsters( ubMineIndex ))
		{
			// if it's shutdown, but not permanently
			if (IsMineShutDown( ubMineIndex ) && !pMineStatus->fShutDownIsPermanent)
			{
				// if we control production in it
				if (PlayerControlsMine( ubMineIndex ))
				{
					IssueHeadMinerQuote( ubMineIndex, HEAD_MINER_STRATEGIC_QUOTE_CREATURES_GONE );
				}

				// put mine back in service
				RestartMineProduction( ubMineIndex );
			}
		}
		else	// mine is monster infested
		{
			// 'Der be monsters crawling around in there, lad!!!

			// if it's still producing
			if (!IsMineShutDown( ubMineIndex ))
			{
				// gotta put a stop to that!

				// if we control production in it
				if (PlayerControlsMine( ubMineIndex ))
				{
					// 2 different quotes, depends whether or not it's the first time this has happened
					HeadMinerQuote ubQuoteType;
					if (pMineStatus->fPrevInvadedByMonsters)
					{
						ubQuoteType = HEAD_MINER_STRATEGIC_QUOTE_CREATURES_AGAIN;
					}
					else
					{
						ubQuoteType = HEAD_MINER_STRATEGIC_QUOTE_CREATURES_ATTACK;
						pMineStatus->fPrevInvadedByMonsters = TRUE;

						if ( gubQuest[ QUEST_CREATURES ] == QUESTNOTSTARTED )
						{
							// start it now!
							UINT8 const sector = GCM->getMine(ubMineIndex)->entranceSector;
							StartQuest(QUEST_CREATURES, SGPSector(sector));
						}
					}

					// tell player the good news...
					IssueHeadMinerQuote( ubMineIndex, ubQuoteType );
				}

				// and immediately halt all work at the mine (whether it's ours or the queen's).  This is a temporary shutdown
				ShutOffMineProduction( ubMineIndex );
			}
		}
	}
}


INT32 GetTotalLeftInMine( UINT8 ubMineIndex )
{
	// returns the value of the mine

	Assert(ubMineIndex < gMineStatus.size());

	return ( gMineStatus[ ubMineIndex ].uiRemainingOreSupply );
}


UINT32 GetMaxPeriodicRemovalFromMine( UINT8 ubMineIndex )
{
	// returns max amount that can be mined in a time period

	Assert(ubMineIndex < gMineStatus.size());

	// if mine is shut down
	if ( gMineStatus[ ubMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	return( gMineStatus[ ubMineIndex ].uiMaxRemovalRate );
}


UINT32 GetMaxDailyRemovalFromMine(UINT8 const mine_id)
{
	Assert(mine_id < gMineStatus.size());
	MINE_STATUS_TYPE const& m = gMineStatus[mine_id];

	if (m.fShutDown) return 0;

	UINT32 const rate      = MINE_PRODUCTION_NUMBER_OF_PERIODS * m.uiMaxRemovalRate;
	UINT32 const remaining = m.uiRemainingOreSupply;
	return rate < remaining ? rate : remaining;
}


INT8 GetTownAssociatedWithMine( UINT8 ubMineIndex )
{
	Assert(ubMineIndex < GCM->getMines().size());
	return GCM->getMine(ubMineIndex)->associatedTownId;
}


static void AddMineHistoryEvent(UINT8 const event, UINT const mine_id)
{
	auto m = GCM->getMine(mine_id);
	AddHistoryToPlayersLog(event, m->associatedTownId, GetWorldTotalMin(), SGPSector(m->entranceSector));
}


// remove actual ore from mine
static UINT32 ExtractOreFromMine(UINT8 ubMineIndex, UINT32 uiAmount)
{
	// will remove the ore from the mine and return the amount that was removed
	UINT32 uiAmountExtracted = 0;

	Assert(ubMineIndex < gMineStatus.size());

	// if mine is shut down
	if ( gMineStatus[ ubMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	// if not capable of extracting anything, bail now
	if (uiAmount == 0)
	{
		return ( 0 );
	}

	// will this exhaust the ore in this mine?
	if( uiAmount >= gMineStatus[ ubMineIndex ].uiRemainingOreSupply )
	{
		// exhaust remaining ore
		uiAmountExtracted = gMineStatus[ ubMineIndex ].uiRemainingOreSupply;
		gMineStatus[ ubMineIndex ].uiRemainingOreSupply = 0;
		gMineStatus[ ubMineIndex ].uiMaxRemovalRate = 0;
		gMineStatus[ ubMineIndex ].fEmpty = TRUE;
		gMineStatus[ ubMineIndex ].fRunningOut = FALSE;

		// tell the strategic AI about this, that mine's and town's value is greatly reduced
		StrategicHandleMineThatRanOut(GetMineSector(ubMineIndex));

		AddMineHistoryEvent(HISTORY_MINE_RAN_OUT, ubMineIndex);
	}
	else	// still some left after this extraction
	{
		// set amount used, and decrement ore remaining in mine
		uiAmountExtracted = uiAmount;
		gMineStatus[ ubMineIndex ].uiRemainingOreSupply -= uiAmount;

		// one of the mines (randomly chosen) will start running out eventually, check if we're there yet
		if (gMineStatus[ ubMineIndex ].uiRemainingOreSupply < gMineStatus[ ubMineIndex ].uiOreRunningOutPoint)
		{
			gMineStatus[ ubMineIndex ].fRunningOut = TRUE;

			// round all fractions UP to the next REMOVAL_RATE_INCREMENT
			gMineStatus[ ubMineIndex ].uiMaxRemovalRate = (UINT32) (((FLOAT) gMineStatus[ ubMineIndex ].uiRemainingOreSupply / 10) / REMOVAL_RATE_INCREMENT + 0.9999) * REMOVAL_RATE_INCREMENT;


			// if we control it
			if (PlayerControlsMine(ubMineIndex))
			{
				// and haven't yet been warned that it's running out
				if (!gMineStatus[ ubMineIndex ].fWarnedOfRunningOut)
				{
					// that mine's head miner tells player that the mine is running out
					IssueHeadMinerQuote( ubMineIndex, HEAD_MINER_STRATEGIC_QUOTE_RUNNING_OUT );
					gMineStatus[ ubMineIndex ].fWarnedOfRunningOut = TRUE;
					AddMineHistoryEvent(HISTORY_MINE_RUNNING_OUT, ubMineIndex);
				}
			}
		}
	}

	return( uiAmountExtracted );
}


// Get the available player workforce for the mine [0,100]
static INT32 GetAvailableWorkForceForMineForPlayer(UINT8 ubMineIndex)
{
	// look for available workforce in the town associated with the mine
	INT32 iWorkForceSize = 0;
	INT8 bTownId = 0;

	// return the loyalty of the town associated with the mine

	Assert(ubMineIndex < gMineStatus.size());
	auto mine = GCM->getMine(ubMineIndex);

	// if mine is shut down
	if ( gMineStatus[ ubMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	// until the player contacts the head miner, production in mine ceases if in player's control
	if ( !gMineStatus[ ubMineIndex ].fSpokeToHeadMiner)
	{
		return ( 0 );
	}


	bTownId = mine->associatedTownId;

	UINT8 numSectors = GetTownSectorSize( bTownId );
	Assert(numSectors > 0);
	UINT8 numSectorsUnderControl = GetTownSectorsUnderControl( bTownId );
	Assert(numSectorsUnderControl <= numSectors);

	// get workforce size (is 0-100 based on local town's loyalty)
	iWorkForceSize = gTownLoyalty[ bTownId ].ubRating;

	// now adjust for town size.. the number of sectors you control
	iWorkForceSize = iWorkForceSize * numSectorsUnderControl / numSectors;

	return ( iWorkForceSize );
}


// get workforce conscripted by enemy for mine
static INT32 GetAvailableWorkForceForMineForEnemy(UINT8 ubMineIndex)
{
	// look for available workforce in the town associated with the mine
	INT32 iWorkForceSize = 0;
	INT8 bTownId = 0;

	// return the loyalty of the town associated with the mine

	Assert(ubMineIndex < GCM->getMines().size());
	auto mine = GCM->getMine(ubMineIndex);

	// if mine is shut down
	if ( gMineStatus[ ubMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	bTownId = mine->associatedTownId;

	UINT8 numSectors = GetTownSectorSize( bTownId );
	Assert(numSectors > 0);
	UINT8 numSectorsUnderControl = GetTownSectorsUnderControl( bTownId );
	Assert(numSectorsUnderControl <= numSectors);

	// get workforce size (is 0-100 based on REVERSE of local town's loyalty)
	iWorkForceSize = 100 - gTownLoyalty[ bTownId ].ubRating;

	// now adjust for town size.. the number of sectors you control
	iWorkForceSize = iWorkForceSize * (numSectors - numSectorsUnderControl) / numSectors;

	return ( iWorkForceSize );
}


// how fast is the mine's workforce working for you?
static INT32 GetCurrentWorkRateOfMineForPlayer(UINT8 ubMineIndex)
{
	INT32 iWorkRate = 0;

	// multiply maximum possible removal rate by the percentage of workforce currently working
	iWorkRate = (gMineStatus[ ubMineIndex ].uiMaxRemovalRate * GetAvailableWorkForceForMineForPlayer( ubMineIndex )) / 100;

	return( iWorkRate );
}


// how fast is workforce working for the enemy
static INT32 GetCurrentWorkRateOfMineForEnemy(UINT8 ubMineIndex)
{
	INT32 iWorkRate = 0;

	// multiply maximum possible removal rate by the percentage of workforce currently working
	iWorkRate = (gMineStatus[ ubMineIndex ].uiMaxRemovalRate * GetAvailableWorkForceForMineForEnemy( ubMineIndex )) / 100;

	return( iWorkRate );
}


// mine this mine
static INT32 MineAMine(UINT8 ubMineIndex)
{
	// will extract ore based on available workforce, and increment players income based on amount
	INT32 iAmtExtracted = 0;


	Assert(ubMineIndex < gMineStatus.size());

	// is mine is empty
	if( gMineStatus[ ubMineIndex ].fEmpty)
	{
		return 0;
	}

	// if mine is shut down
	if ( gMineStatus[ ubMineIndex ].fShutDown)
	{
		return 0;
	}


	// who controls the PRODUCTION in the mine ?  (Queen receives production unless player has spoken to the head miner)
	if( PlayerControlsMine(ubMineIndex) )
	{
		// player controlled
		iAmtExtracted = ExtractOreFromMine( ubMineIndex , GetCurrentWorkRateOfMineForPlayer( ubMineIndex ) );

		// SHOW ME THE MONEY!!!!
		if( iAmtExtracted > 0 )
		{
			// debug message
			SLOGD("{} - Mine income from {} = ${}", WORLDTIMESTR, GCM->getTownName(GetTownAssociatedWithMine(ubMineIndex)), iAmtExtracted);

			// if this is the first time this mine has produced income for the player in the game
			if ( !gMineStatus[ ubMineIndex ].fMineHasProducedForPlayer )
			{
				// remember that we've earned income from this mine during the game
				gMineStatus[ ubMineIndex ].fMineHasProducedForPlayer = TRUE;
				// and when we started to do so...
				gMineStatus[ ubMineIndex ].uiTimePlayerProductionStarted = GetWorldTotalMin();
			}
		}
	}
	else	// queen controlled
	{
		// we didn't want mines to run out without player ever even going to them, so now the queen doesn't reduce the
		// amount remaining until the mine has produced for the player first (so she'd have to capture it).
		if ( gMineStatus[ ubMineIndex ].fMineHasProducedForPlayer )
		{
			// don't actually give her money, just take production away
			iAmtExtracted = ExtractOreFromMine( ubMineIndex , GetCurrentWorkRateOfMineForEnemy( ubMineIndex ) );
		}
	}


	return iAmtExtracted;
}


void PostEventsForMineProduction(void)
{
	UINT8 ubShift;

	for (ubShift = 0; ubShift < MINE_PRODUCTION_NUMBER_OF_PERIODS; ubShift++)
	{
		AddStrategicEvent( EVENT_HANDLE_MINE_INCOME, GetWorldDayInMinutes() + MINE_PRODUCTION_START_TIME + (ubShift * MINE_PRODUCTION_PERIOD), 0 );
	}
}


void HandleIncomeFromMines( void )
{
	INT32 iIncome = 0;
	// mine each mine, check if we own it and such
	for (UINT8 ubCounter = 0; ubCounter < gMineStatus.size(); ++ubCounter)
	{
		// mine this mine
		iIncome += MineAMine(ubCounter);
	}
	if (iIncome)
	{
		AddTransactionToPlayersBook(DEPOSIT_FROM_SILVER_MINE, 0, GetWorldTotalMin(), iIncome);
	}
}


UINT32 PredictDailyIncomeFromAMine(INT8 const mine_id)
{
	/* Predict income from this mine, estimate assumes mining situation will not
	 * change during next 4 income periods (miner loyalty, % town controlled,
	 * monster infestation level, and current max removal rate may all in fact
	 * change) */
	if (!PlayerControlsMine(mine_id)) return 0;

	/* Get daily income for this mine (regardless of what time of day it currently
	 * is) */
	UINT32 const amount    = MINE_PRODUCTION_NUMBER_OF_PERIODS * GetCurrentWorkRateOfMineForPlayer(mine_id);
	UINT32 const remaining = gMineStatus[mine_id].uiRemainingOreSupply;
	return amount < remaining ? amount : remaining;
}


INT32 PredictIncomeFromPlayerMines( void )
{
	INT32 iTotal = 0;
	UINT8 ubCounter = 0;

	for( ubCounter = 0; ubCounter < gMineStatus.size(); ubCounter++ )
	{
		// add up the total
		iTotal += PredictDailyIncomeFromAMine( ubCounter );
	}

	return( iTotal );
}


INT32 CalcMaxPlayerIncomeFromMines()
{
	INT32 total = 0;
	for (MINE_STATUS_TYPE i : gMineStatus)
	{
		total += MINE_PRODUCTION_NUMBER_OF_PERIODS * i.uiMaxRemovalRate;
	}
	return total;
}


INT8 GetMineIndexForSector(UINT8 const sector)
{
	return GetIdOfMineForSector(SGPSector(sector));
}


UINT8 GetMineSector(UINT8 const ubMineIndex)
{
	Assert(ubMineIndex < gMineStatus.size());
	return GCM->getMine(ubMineIndex)->entranceSector;
}


// get the Strategic Index for the mine associated with this town
INT16 GetMineSectorForTown(INT8 const town_id)
{
	for (auto m : GCM->getMines())
	{
		if (m->associatedTownId != town_id) continue;
		return SGPSector(m->entranceSector).AsStrategicIndex();
	}
	return -1;
}


bool PlayerControlsMine(INT8 const mine_id)
{
	auto mine = GCM->getMine(mine_id);
	return
		!StrategicMap[SGPSector(mine->entranceSector).AsStrategicIndex()].fEnemyControlled &&
		/* Player only controls the actual mine after he has made arrangements to do
		 * so with the head miner there. */
		gMineStatus[mine_id].fSpokeToHeadMiner;
}


void SaveMineStatusToSaveGameFile(HWFILE const f)
{
	for (MINE_STATUS_TYPE i : gMineStatus)
	{
		BYTE  data[44];
		DataWriter d{data};
		INJ_U8(  d, i.ubMineType)
		INJ_SKIP(d, 3)
		INJ_U32( d, i.uiMaxRemovalRate)
		INJ_U32( d, i.uiRemainingOreSupply)
		INJ_U32( d, i.uiOreRunningOutPoint)
		INJ_BOOL(d, i.fEmpty)
		INJ_BOOL(d, i.fRunningOut)
		INJ_BOOL(d, i.fWarnedOfRunningOut)
		INJ_BOOL(d, i.fShutDownIsPermanent)
		INJ_BOOL(d, i.fShutDown)
		INJ_BOOL(d, i.fPrevInvadedByMonsters)
		INJ_BOOL(d, i.fSpokeToHeadMiner)
		INJ_BOOL(d, i.fMineHasProducedForPlayer)
		INJ_BOOL(d, i.fQueenRetookProducingMine)
		INJ_BOOL(d, i.fAttackedHeadMiner)
		INJ_SKIP(d, 2)
		INJ_U32( d, i.uiTimePlayerProductionStarted)
		INJ_SKIP(d, 12)
		Assert(d.getConsumed() == lengthof(data));

		f->write(data, sizeof(data));
	}
}


void LoadMineStatusFromSavedGameFile(HWFILE const f)
{
	// Save game breaks if the number of mines changes, as we do not
	// store the number of mines when the game was saved.
	gMineStatus.resize(GCM->getMines().size());
	for (auto& i : gMineStatus)
	{
		BYTE  data[44];
		f->read(data, sizeof(data));

		DataReader d{data};
		EXTR_U8(  d, i.ubMineType)
		EXTR_SKIP(d, 3)
		EXTR_U32( d, i.uiMaxRemovalRate)
		EXTR_U32( d, i.uiRemainingOreSupply)
		EXTR_U32( d, i.uiOreRunningOutPoint)
		EXTR_BOOL(d, i.fEmpty)
		EXTR_BOOL(d, i.fRunningOut)
		EXTR_BOOL(d, i.fWarnedOfRunningOut)
		EXTR_BOOL(d, i.fShutDownIsPermanent)
		EXTR_BOOL(d, i.fShutDown)
		EXTR_BOOL(d, i.fPrevInvadedByMonsters)
		EXTR_BOOL(d, i.fSpokeToHeadMiner)
		EXTR_BOOL(d, i.fMineHasProducedForPlayer)
		EXTR_BOOL(d, i.fQueenRetookProducingMine)
		EXTR_BOOL(d, i.fAttackedHeadMiner)
		EXTR_SKIP(d, 2)
		EXTR_U32( d, i.uiTimePlayerProductionStarted)
		EXTR_SKIP(d, 12)
		Assert(d.getConsumed() == lengthof(data));
	}
}


void ShutOffMineProduction( UINT8 ubMineIndex )
{
	Assert(ubMineIndex < gMineStatus.size());

	if ( !gMineStatus[ ubMineIndex ].fShutDown )
	{
		gMineStatus[ ubMineIndex ].fShutDown = TRUE;
		AddMineHistoryEvent(HISTORY_MINE_SHUTDOWN, ubMineIndex);
	}
}


void RestartMineProduction( UINT8 ubMineIndex )
{
	Assert(ubMineIndex < gMineStatus.size());

	if ( !gMineStatus[ ubMineIndex ].fShutDownIsPermanent )
	{
		if ( gMineStatus[ ubMineIndex ].fShutDown )
		{
			gMineStatus[ ubMineIndex ].fShutDown = FALSE;
			AddMineHistoryEvent(HISTORY_MINE_REOPENED, ubMineIndex);
		}
	}
}


static void MineShutdownIsPermanent(UINT8 ubMineIndex)
{
	Assert(ubMineIndex < gMineStatus.size());

	gMineStatus[ ubMineIndex ].fShutDownIsPermanent = TRUE;
}


BOOLEAN IsMineShutDown( UINT8 ubMineIndex )
{
	Assert(ubMineIndex < gMineStatus.size());

	return(gMineStatus[ ubMineIndex ].fShutDown);
}


static UINT8 GetHeadMinerIndexForMine(UINT8 ubMineIndex)
{
	UINT8 ubMinerIndex = 0;
	UINT16 usProfileId = 0;

	Assert(ubMineIndex < GCM->getMines().size());
	auto mine = GCM->getMine(ubMineIndex);

	// loop through all head miners, checking which town they're associated with, looking for one that matches this mine
	for (ubMinerIndex = 0; ubMinerIndex < NUM_HEAD_MINERS; ubMinerIndex++)
	{
		usProfileId = gHeadMinerData[ ubMinerIndex ].usProfileId;

		if (gMercProfiles[ usProfileId ].bTown == mine->associatedTownId)
		{
			return(ubMinerIndex);
		}
	}

	// not found - yack!
	Assert( FALSE );
	return( 0 );
}


void IssueHeadMinerQuote(UINT8 const mine_idx, HeadMinerQuote const quote_type)
{
	Assert(mine_idx < GCM->getMines().size());
	Assert(quote_type < NUM_HEAD_MINER_STRATEGIC_QUOTES);
	Assert(CheckFact(FACT_MINERS_PLACED, 0));

	HEAD_MINER_TYPE const& miner_data = gHeadMinerData[GetHeadMinerIndexForMine(mine_idx)];
	auto mineData = GCM->getMine(mine_idx);

	// Make sure the miner isn't dead
	MERCPROFILESTRUCT const& p = GetProfile(miner_data.usProfileId);
	if (p.bLife < OKLIFE)
	{
		SLOGD("Head Miner #{} can't talk (quote #{})", p.zNickname, quote_type);
		return;
	}

	INT8 const bQuoteNum = miner_data.bQuoteNum[quote_type];
	Assert(bQuoteNum != -1);

	/* Transition to mapscreen is not necessary for "creatures gone" quote -
	 * player is IN that mine, so he'll know */
	bool const force_mapscreen = quote_type != HEAD_MINER_STRATEGIC_QUOTE_CREATURES_GONE;

	/* Decide where the miner's face and text box should be positioned in order to
	 * not obscure the mine he's in as it flashes */
	INT16 const x = DEFAULT_EXTERN_PANEL_X_POS;
	INT16       y = DEFAULT_EXTERN_PANEL_Y_POS;
	if (mineData->faceDisplayYOffset)
	{
		y = STD_SCREEN_Y + mineData->faceDisplayYOffset;
	}

	SetExternMapscreenSpeechPanelXY(x, y);

	/* Cause this quote to come up for this profile id and an indicator to flash
	 * over the mine sector */
	HandleMinerEvent(miner_data.usProfileId, bQuoteNum, force_mapscreen);

	// Stop time compression with any miner quote - these are important events.
	StopTimeCompression();
}


UINT8 GetHeadMinersMineIndex( UINT8 ubMinerProfileId)
{
	// find which mine this guy represents
	for (auto mine : GCM->getMines())
	{
		if (mine->associatedTownId == gMercProfiles[ubMinerProfileId].bTown)
		{
			return mine->mineId;
		}
	}

	// not found!
	SLOGA("Illegal profile id receieved or something is very wrong");
	return 0;
}


void PlayerSpokeToHeadMiner( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	// if this is our first time set a history fact
	if (!gMineStatus[ubMineIndex].fSpokeToHeadMiner)
	{
		AddMineHistoryEvent(HISTORY_TALKED_TO_MINER, ubMineIndex);
		gMineStatus[ ubMineIndex ].fSpokeToHeadMiner = TRUE;
	}
}


BOOLEAN IsHisMineRunningOut( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );
	return(gMineStatus[ ubMineIndex ].fRunningOut);
}

BOOLEAN IsHisMineEmpty( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );
	return(gMineStatus[ ubMineIndex ].fEmpty);
}

BOOLEAN IsHisMineDisloyal( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );
	UINT8 ubAssociatedTown = GCM->getMine(ubMineIndex)->associatedTownId;

	if (gTownLoyalty[ubAssociatedTown].ubRating < LOW_MINE_LOYALTY_THRESHOLD)
	{
		// pretty disloyal
		return(TRUE);
	}
	else
	{
		// pretty loyal
		return(FALSE);
	}
}

BOOLEAN IsHisMineInfested( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );
	return(!MineClearOfMonsters( ubMineIndex ));
}

BOOLEAN IsHisMineLostAndRegained( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	if (PlayerControlsMine(ubMineIndex) && gMineStatus[ ubMineIndex ].fQueenRetookProducingMine)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

void ResetQueenRetookMine( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	gMineStatus[ ubMineIndex ].fQueenRetookProducingMine = FALSE;
}

BOOLEAN IsHisMineAtMaxProduction( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	if (GetAvailableWorkForceForMineForPlayer( ubMineIndex ) == 100)
	{
		// loyalty is 100% and control is 100%
		return(TRUE);
	}
	else
	{
		// something not quite perfect yet
		return(FALSE);
	}
}


void QueenHasRegainedMineSector(UINT8 ubMineIndex)
{
	Assert(ubMineIndex < gMineStatus.size());

	if (gMineStatus[ ubMineIndex ].fMineHasProducedForPlayer)
	{
		gMineStatus[ ubMineIndex ].fQueenRetookProducingMine = TRUE;
	}
}


BOOLEAN HasAnyMineBeenAttackedByMonsters(void)
{
	UINT8 ubMineIndex;

	// find which mine this guy represents
	for( ubMineIndex = 0; ubMineIndex < gMineStatus.size(); ubMineIndex++ )
	{
		if (!MineClearOfMonsters( ubMineIndex ) || gMineStatus[ ubMineIndex ].fPrevInvadedByMonsters)
		{
			return(TRUE);
		}
	}

	return(FALSE);
}


void PlayerAttackedHeadMiner( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;
	INT8 bTownId;

	// get the index of his mine
	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	// if it's the first time he's been attacked
	if (!gMineStatus[ubMineIndex].fAttackedHeadMiner)
	{
		// shut off production at his mine (Permanently!)
		ShutOffMineProduction( ubMineIndex );
		MineShutdownIsPermanent( ubMineIndex );

		// get the index of his town
		bTownId = GetTownAssociatedWithMine( ubMineIndex );
		// penalize associated town's loyalty
		DecrementTownLoyalty( bTownId, LOYALTY_PENALTY_HEAD_MINER_ATTACKED );

		// don't allow this more than once
		gMineStatus[ ubMineIndex ].fAttackedHeadMiner = TRUE;
	}
}


BOOLEAN HasHisMineBeenProducingForPlayerForSomeTime( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	if ( gMineStatus[ ubMineIndex ].fMineHasProducedForPlayer &&
			( ( GetWorldTotalMin() - gMineStatus[ ubMineIndex ].uiTimePlayerProductionStarted ) >= ( 24 * 60 ) ) )
	{
		return ( TRUE );
	}

	return( FALSE );
}


INT8 GetIdOfMineForSector(const SGPSector& sector)
{
	auto mine = GCM->getMineForSector(sector);
	if (mine != NULL)
	{
		return mine->mineId;
	}

	return -1;
}


// use this to determine whether or not to place miners into a underground mine level
BOOLEAN AreThereMinersInsideThisMine( UINT8 ubMineIndex )
{
	MINE_STATUS_TYPE *pMineStatus;


	Assert(ubMineIndex < gMineStatus.size());

	pMineStatus = &(gMineStatus[ ubMineIndex ]);

	// mine not empty
	// mine clear of any monsters
	// the "shutdown permanently" flag is only used for the player never receiving the income - miners will keep mining
	if ( ( !pMineStatus->fEmpty ) && MineClearOfMonsters( ubMineIndex ) )
	{
		return( TRUE );
	}

	return( FALSE );
}

// returns whether or not we've spoken to the head miner of a particular mine
BOOLEAN SpokenToHeadMiner( UINT8 ubMineIndex )
{
	return( gMineStatus[ ubMineIndex ].fSpokeToHeadMiner );
}
