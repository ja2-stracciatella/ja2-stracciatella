#include "Font_Control.h"
#include "MapScreen.h"
#include "Strategic_Mines.h"
#include "Finances.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic.h"
#include "Game_Clock.h"
#include "StrategicMap.h"
#include "Random.h"
#include "Soldier_Profile.h"
#include "Dialogue_Control.h"
#include "Map_Screen_Interface.h"
#include "Quests.h"
#include "Creature_Spreading.h"
#include "Message.h"
#include "Text.h"
#include "Game_Event_Hook.h"
#include "GameSettings.h"
#include "Strategic_AI.h"
#include "History.h"
#include "Campaign_Types.h"
#include "Debug.h"
#include "FileMan.h"


// this .c file will handle the strategic level of mines and income from them


#define REMOVAL_RATE_INCREMENT	250		// the smallest increment by which removal rate change during depletion (use round #s)

#define LOW_MINE_LOYALTY_THRESHOLD	50	// below this the head miner considers his town's population disloyal

// Mine production is being processed 4x daily: 9am ,noon, 3pm, and 6pm.
// This is loosely based on a 6am-6pm working day of 4 "shifts".
#define MINE_PRODUCTION_NUMBER_OF_PERIODS 4						// how many times a day mine production is processed
#define MINE_PRODUCTION_START_TIME				(9 * 60)		// hour of first daily mine production event (in minutes)
#define MINE_PRODUCTION_PERIOD						(3 * 60)		// time seperating daily mine production events (in minutes)


// this table holds mine values that change during the course of the game and must be saved
MINE_STATUS_TYPE gMineStatus[ MAX_NUMBER_OF_MINES ];

// this table holds mine values that never change and don't need to be saved
const MINE_LOCATION_TYPE gMineLocation[MAX_NUMBER_OF_MINES] =
{
	{		 4,		4,		SAN_MONA		},
	{		13,		4,		DRASSEN			},
	{		14,		9,		ALMA				},
	{		 8,		8,		CAMBRIA			},
	{		 2,		2,		CHITZENA		},
	{		 3,		8,		GRUMM				},
};

// the are not being randomized at all at this time
UINT8 gubMineTypes[]={
	GOLD_MINE,			// SAN MONA
	SILVER_MINE,		// DRASSEN
	SILVER_MINE,		// ALMA
	SILVER_MINE,		// CAMBRIA
	SILVER_MINE,		// CHITZENA
	GOLD_MINE,			// GRUMM
};

// These values also determine the most likely ratios of mine sizes after random production increases are done
UINT32 guiMinimumMineProduction[]={
	    0,		// SAN MONA
	 1000,		// DRASSEN
	 1500,		// ALMA
	 1500,		// CAMBRIA
	  500,		// CHITZENA
	 2000,		// GRUMM
};


struct HEAD_MINER_TYPE
{
	UINT16 usProfileId;
	INT8   bQuoteNum[NUM_HEAD_MINER_STRATEGIC_QUOTES];
	UINT8  ubExternalFace;
};


static const HEAD_MINER_TYPE gHeadMinerData[NUM_HEAD_MINERS] =
{
	//	Profile #		running out		creatures!		all dead!		creatures again!		external face graphic
	{     FRED,   {     17,           18,          27,             26      }, MINER_FRED_EXTERNAL_FACE   },
	{     MATT,   {     -1,           18,          32,             31      }, MINER_MATT_EXTERNAL_FACE   },
	{     OSWALD, {     14,           15,          24,             23      }, MINER_OSWALD_EXTERNAL_FACE },
	{     CALVIN, {     14,           15,          24,             23      }, MINER_CALVIN_EXTERNAL_FACE },
	{     CARL,   {     14,           15,          24,             23      }, MINER_CARL_EXTERNAL_FACE   }
};



/* gradual monster infestation concept was ditched, now simply IN PRODUCTION or SHUT DOWN

// percentage of workers working depends on level of mine infestation
UINT8 gubMonsterMineInfestation[]={
	100,
	99,
	95,
	70,
	30,
	1,
	0,
};
*/


void InitializeMines( void )
{
	UINT8 ubMineIndex;
	MINE_STATUS_TYPE *pMineStatus;
	UINT8 ubMineProductionIncreases;
	UINT8 ubDepletedMineIndex;
	UINT8 ubMinDaysBeforeDepletion = 20;


	// set up initial mine status
	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
	{
		pMineStatus = &(gMineStatus[ ubMineIndex ]);

		memset( pMineStatus, 0, sizeof( *pMineStatus ) );

		pMineStatus->ubMineType = gubMineTypes[ ubMineIndex ];
		pMineStatus->uiMaxRemovalRate = guiMinimumMineProduction[ ubMineIndex ];
		pMineStatus->fEmpty = (pMineStatus->uiMaxRemovalRate == 0) ? TRUE : FALSE;
		pMineStatus->fRunningOut = FALSE;
		pMineStatus->fWarnedOfRunningOut = FALSE;
//		pMineStatus->bMonsters = MINES_NO_MONSTERS;
		pMineStatus->fShutDown = FALSE;
		pMineStatus->fPrevInvadedByMonsters = FALSE;
		pMineStatus->fSpokeToHeadMiner = FALSE;
		pMineStatus->fMineHasProducedForPlayer = FALSE;
		pMineStatus->fQueenRetookProducingMine = FALSE;
		gMineStatus->fShutDownIsPermanent = FALSE;
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
			Assert( 0 );
			return;
	}

	while (ubMineProductionIncreases > 0)
	{
		// pick a producing mine at random and increase its production
		do
		{
			ubMineIndex = ( UINT8 ) Random(MAX_NUMBER_OF_MINES);
		} while (gMineStatus[ubMineIndex].fEmpty);

		// increase mine production by 20% of the base (minimum) rate
		gMineStatus[ubMineIndex].uiMaxRemovalRate += (guiMinimumMineProduction[ubMineIndex] / 5);

		ubMineProductionIncreases--;
	}


	// choose which mine will run out of production.  This will never be the Alma mine or an empty mine (San Mona)...
	do
	{
		ubDepletedMineIndex = ( UINT8 ) Random(MAX_NUMBER_OF_MINES);
		// Alma mine can't run out for quest-related reasons (see Ian)
	} while (gMineStatus[ubDepletedMineIndex].fEmpty || (ubDepletedMineIndex == MINE_ALMA));


	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
	{
		pMineStatus = &(gMineStatus[ ubMineIndex ]);

		if (ubMineIndex == ubDepletedMineIndex)
		{
			if ( ubDepletedMineIndex == MINE_DRASSEN )
			{
				ubMinDaysBeforeDepletion = 20;
			}
			else
			{
				ubMinDaysBeforeDepletion = 10;
			}

			// the mine that runs out has only enough ore for this many days of full production
			pMineStatus->uiRemainingOreSupply = ubMinDaysBeforeDepletion * (MINE_PRODUCTION_NUMBER_OF_PERIODS * pMineStatus->uiMaxRemovalRate);

			// ore starts running out when reserves drop to less than 25% of the initial supply
			pMineStatus->uiOreRunningOutPoint = pMineStatus->uiRemainingOreSupply / 4;
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
	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
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

				//Force the creatures to avoid the mine for a period of time.  This gives the
				//player a chance to rest and decide how to deal with the problem.
				ForceCreaturesToAvoidMineTemporarily( ubMineIndex );

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
							StartQuest( QUEST_CREATURES, gMineLocation[ ubMineIndex ].sSectorX, gMineLocation[ ubMineIndex ].sSectorY );
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


INT32 GetTotalLeftInMine( INT8 bMineIndex )
{
	// returns the value of the mine

	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	return ( gMineStatus[ bMineIndex ].uiRemainingOreSupply );
}


UINT32 GetMaxPeriodicRemovalFromMine( INT8 bMineIndex )
{
	// returns max amount that can be mined in a time period

	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	// if mine is shut down
	if ( gMineStatus[ bMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	return( gMineStatus[ bMineIndex ].uiMaxRemovalRate );
}


UINT32 GetMaxDailyRemovalFromMine( INT8 bMineIndex )
{
	UINT32 uiAmtExtracted;

	// returns max amount that can be mined in one day

	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	// if mine is shut down
	if ( gMineStatus[ bMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	uiAmtExtracted = MINE_PRODUCTION_NUMBER_OF_PERIODS * gMineStatus[ bMineIndex ].uiMaxRemovalRate;

	// check if we will take more than there is
	if( uiAmtExtracted > gMineStatus[ bMineIndex ].uiRemainingOreSupply )
	{
		// yes, reduce to value of mine
		uiAmtExtracted = gMineStatus[ bMineIndex ].uiRemainingOreSupply;
	}

	return(uiAmtExtracted);
}


INT8 GetTownAssociatedWithMine( INT8 bMineIndex )
{
	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	return ( gMineLocation[ bMineIndex ].bAssociatedTown );
}


// remove actual ore from mine
static UINT32 ExtractOreFromMine(INT8 bMineIndex, UINT32 uiAmount)
{
	// will remove the ore from the mine and return the amount that was removed
	UINT32 uiAmountExtracted = 0;

	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	// if mine is shut down
	if ( gMineStatus[ bMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	// if not capable of extracting anything, bail now
	if (uiAmount == 0)
	{
		return ( 0 );
	}

	// will this exhaust the ore in this mine?
	if( uiAmount >= gMineStatus[ bMineIndex ].uiRemainingOreSupply )
	{
		// exhaust remaining ore
		uiAmountExtracted = gMineStatus[ bMineIndex ].uiRemainingOreSupply;
		gMineStatus[ bMineIndex ].uiRemainingOreSupply = 0;
		gMineStatus[ bMineIndex ].uiMaxRemovalRate = 0;
		gMineStatus[ bMineIndex ].fEmpty = TRUE;
		gMineStatus[ bMineIndex ].fRunningOut = FALSE;

		// tell the strategic AI about this, that mine's and town's value is greatly reduced
		UINT16 sSectorX;
		UINT16 sSectorY;
		GetMineSector( bMineIndex, &sSectorX, &sSectorY );
		StrategicHandleMineThatRanOut( ( UINT8 ) SECTOR( sSectorX, sSectorY ) );

		AddHistoryToPlayersLog( HISTORY_MINE_RAN_OUT, gMineLocation[ bMineIndex ].bAssociatedTown, GetWorldTotalMin( ), gMineLocation[ bMineIndex ].sSectorX,  gMineLocation[ bMineIndex ].sSectorY );
	}
	else	// still some left after this extraction
	{
		// set amount used, and decrement ore remaining in mine
		uiAmountExtracted = uiAmount;
		gMineStatus[ bMineIndex ].uiRemainingOreSupply -= uiAmount;

		// one of the mines (randomly chosen) will start running out eventually, check if we're there yet
		if (gMineStatus[ bMineIndex ].uiRemainingOreSupply < gMineStatus[ bMineIndex ].uiOreRunningOutPoint)
		{
			gMineStatus[ bMineIndex ].fRunningOut = TRUE;

			// round all fractions UP to the next REMOVAL_RATE_INCREMENT
			gMineStatus[ bMineIndex ].uiMaxRemovalRate = (UINT32) (((FLOAT) gMineStatus[ bMineIndex ].uiRemainingOreSupply / 10) / REMOVAL_RATE_INCREMENT + 0.9999) * REMOVAL_RATE_INCREMENT;


			// if we control it
			if (PlayerControlsMine(bMineIndex))
			{
				// and haven't yet been warned that it's running out
				if (!gMineStatus[ bMineIndex ].fWarnedOfRunningOut)
				{
					// that mine's head miner tells player that the mine is running out
					IssueHeadMinerQuote( bMineIndex, HEAD_MINER_STRATEGIC_QUOTE_RUNNING_OUT );
					gMineStatus[ bMineIndex ].fWarnedOfRunningOut = TRUE;
					AddHistoryToPlayersLog( HISTORY_MINE_RUNNING_OUT, gMineLocation[ bMineIndex ].bAssociatedTown, GetWorldTotalMin( ), gMineLocation[ bMineIndex ].sSectorX,  gMineLocation[ bMineIndex ].sSectorY );
				}
			}
		}
	}

	return( uiAmountExtracted );
}


// get available workforce for the mine
static INT32 GetAvailableWorkForceForMineForPlayer(INT8 bMineIndex)
{
	// look for available workforce in the town associated with the mine
	INT32 iWorkForceSize = 0;
	INT8 bTownId = 0;

	// return the loyalty of the town associated with the mine

	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	// if mine is shut down
	if ( gMineStatus[ bMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	// until the player contacts the head miner, production in mine ceases if in player's control
	if ( !gMineStatus[ bMineIndex ].fSpokeToHeadMiner)
	{
		return ( 0 );
	}


	bTownId = gMineLocation[ bMineIndex ].bAssociatedTown;

	Assert ( GetTownSectorSize( bTownId ) != 0 );


	// get workforce size (is 0-100 based on local town's loyalty)
	iWorkForceSize = gTownLoyalty[ bTownId ].ubRating;

/*
	// adjust for monster infestation
	iWorkForceSize *= gubMonsterMineInfestation[ gMineStatus[ bMineIndex ].bMonsters ];
	iWorkForceSize /= 100;
*/

	// now adjust for town size.. the number of sectors you control
	iWorkForceSize *= GetTownSectorsUnderControl( bTownId );
	iWorkForceSize /= GetTownSectorSize( bTownId );

	return ( iWorkForceSize );
}


// get workforce conscripted by enemy for mine
static INT32 GetAvailableWorkForceForMineForEnemy(INT8 bMineIndex)
{
	// look for available workforce in the town associated with the mine
	INT32 iWorkForceSize = 0;
	INT8 bTownId = 0;

	// return the loyalty of the town associated with the mine

	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	// if mine is shut down
	if ( gMineStatus[ bMineIndex ].fShutDown)
	{
		return ( 0 );
	}

	bTownId = gMineLocation[ bMineIndex ].bAssociatedTown;

	if( GetTownSectorSize( bTownId )  == 0 )
	{
		return 0;
	}

	// get workforce size (is 0-100 based on REVERSE of local town's loyalty)
	iWorkForceSize = 100 - gTownLoyalty[ bTownId ].ubRating;

/*
	// adjust for monster infestation
	iWorkForceSize *= gubMonsterMineInfestation[ gMineStatus[ bMineIndex ].bMonsters ];
	iWorkForceSize /= 100;
*/

	// now adjust for town size.. the number of sectors you control
	iWorkForceSize *= ( GetTownSectorSize( bTownId ) - GetTownSectorsUnderControl( bTownId ) );
	iWorkForceSize /= GetTownSectorSize( bTownId );

	return ( iWorkForceSize );
}


// how fast is the mine's workforce working for you?
static INT32 GetCurrentWorkRateOfMineForPlayer(INT8 bMineIndex)
{
	INT32 iWorkRate = 0;

	// multiply maximum possible removal rate by the percentage of workforce currently working
	iWorkRate = (gMineStatus[ bMineIndex ].uiMaxRemovalRate * GetAvailableWorkForceForMineForPlayer( bMineIndex )) / 100;

	return( iWorkRate );
}


// how fast is workforce working for the enemy
static INT32 GetCurrentWorkRateOfMineForEnemy(INT8 bMineIndex)
{
	INT32 iWorkRate = 0;

	// multiply maximum possible removal rate by the percentage of workforce currently working
	iWorkRate = (gMineStatus[ bMineIndex ].uiMaxRemovalRate * GetAvailableWorkForceForMineForEnemy( bMineIndex )) / 100;

	return( iWorkRate );
}


// mine this mine
static INT32 MineAMine(INT8 bMineIndex)
{
	// will extract ore based on available workforce, and increment players income based on amount
	INT32 iAmtExtracted = 0;


	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	// is mine is empty
	if( gMineStatus[ bMineIndex ].fEmpty)
	{
		return 0;
	}

	// if mine is shut down
	if ( gMineStatus[ bMineIndex ].fShutDown)
	{
		return 0;
	}


	// who controls the PRODUCTION in the mine ?  (Queen receives production unless player has spoken to the head miner)
	if( PlayerControlsMine(bMineIndex) )
	{
		// player controlled
		iAmtExtracted = ExtractOreFromMine( bMineIndex , GetCurrentWorkRateOfMineForPlayer( bMineIndex ) );

		// SHOW ME THE MONEY!!!!
		if( iAmtExtracted > 0 )
		{
			// debug message
//			ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"%ls - Mine income from %ls = $%d", WORLDTIMESTR, pTownNames[GetTownAssociatedWithMine(bMineIndex)], iAmtExtracted);

			// if this is the first time this mine has produced income for the player in the game
			if ( !gMineStatus[ bMineIndex ].fMineHasProducedForPlayer )
			{
				// remember that we've earned income from this mine during the game
				gMineStatus[ bMineIndex ].fMineHasProducedForPlayer = TRUE;
				// and when we started to do so...
				gMineStatus[ bMineIndex ].uiTimePlayerProductionStarted = GetWorldTotalMin();
			}
		}
	}
	else	// queen controlled
	{
		// we didn't want mines to run out without player ever even going to them, so now the queen doesn't reduce the
		// amount remaining until the mine has produced for the player first (so she'd have to capture it).
		if ( gMineStatus[ bMineIndex ].fMineHasProducedForPlayer )
		{
			// don't actually give her money, just take production away
			iAmtExtracted = ExtractOreFromMine( bMineIndex , GetCurrentWorkRateOfMineForEnemy( bMineIndex ) );
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
#ifndef JA2DEMO
	INT32 iIncome = 0;
	// mine each mine, check if we own it and such
	for (INT8 bCounter = 0; bCounter < MAX_NUMBER_OF_MINES; ++bCounter)
	{
		// mine this mine
		iIncome += MineAMine(bCounter);
	}
	if (iIncome)
	{
		AddTransactionToPlayersBook(DEPOSIT_FROM_SILVER_MINE, 0, GetWorldTotalMin(), iIncome);
	}
#endif
}


UINT32 PredictDailyIncomeFromAMine( INT8 bMineIndex )
{
	// predict income from this mine, estimate assumes mining situation will not change during next 4 income periods
	// (miner loyalty, % town controlled, monster infestation level, and current max removal rate may all in fact change)
	UINT32 uiAmtExtracted = 0;

	if(PlayerControlsMine(bMineIndex))
	{
		// get daily income for this mine (regardless of what time of day it currently is)
		uiAmtExtracted = MINE_PRODUCTION_NUMBER_OF_PERIODS * GetCurrentWorkRateOfMineForPlayer( bMineIndex );

		// check if we will take more than there is
		if( uiAmtExtracted > gMineStatus[ bMineIndex ].uiRemainingOreSupply )
		{
			// yes reduce to value of mine
			uiAmtExtracted = gMineStatus[ bMineIndex ].uiRemainingOreSupply;
		}
	}

	return( uiAmtExtracted );
}


INT32 PredictIncomeFromPlayerMines( void )
{
	INT32 iTotal = 0;
	INT8 bCounter = 0;

	for( bCounter = 0; bCounter < MAX_NUMBER_OF_MINES; bCounter++ )
	{
		// add up the total
		iTotal += PredictDailyIncomeFromAMine( bCounter );
	}

	return( iTotal );
}


INT32 CalcMaxPlayerIncomeFromMines( void )
{
	INT32 iTotal = 0;
	INT8 bCounter = 0;

	// calculate how much player could make daily if he owned all mines with 100% control and 100% loyalty
	for( bCounter = 0; bCounter < MAX_NUMBER_OF_MINES; bCounter++ )
	{
		// add up the total
		iTotal += (MINE_PRODUCTION_NUMBER_OF_PERIODS * gMineStatus[bCounter].uiMaxRemovalRate);
	}

	return( iTotal );
}


// get index of this mine, return -1 if no mine found
INT8 GetMineIndexForSector( INT16 sX, INT16 sY )
{
	UINT8 ubMineIndex = 0;


	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
	{
		if( ( gMineLocation[ ubMineIndex ].sSectorX == sX ) && ( gMineLocation[ ubMineIndex ].sSectorY == sY ) )
		{
			// yep mine here
			return( ubMineIndex );
		}
	}

	return( -1 );
}


void GetMineSector(const UINT8 ubMineIndex, UINT16* const psX, UINT16* const psY)
{
	Assert(ubMineIndex < MAX_NUMBER_OF_MINES);

	*psX = gMineLocation[ ubMineIndex ].sSectorX;
	*psY = gMineLocation[ ubMineIndex ].sSectorY;
}


// get the sector value for the mine associated with this town
INT16 GetMineSectorForTown( INT8 bTownId )
{
	INT8 ubMineIndex;
	INT16 sMineSector = -1;

	// given town id, send sector value of mine, a 0 means no mine for this town
	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
	{
		if( gMineLocation[ ubMineIndex ].bAssociatedTown == bTownId )
		{
			sMineSector = gMineLocation[ ubMineIndex ].sSectorX + ( gMineLocation[ ubMineIndex ].sSectorY * MAP_WORLD_X );
			break;
		}
	}

	// -1 returned if the town doesn't have a mine
	return( sMineSector );
}


BOOLEAN IsThereAMineInThisSector( INT16 sX, INT16 sY )
{
	UINT8 ubMineIndex;

	// run through the list...if a mine here, great
	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
	{
		if( ( gMineLocation[ ubMineIndex ].sSectorX == sX ) && ( gMineLocation[ ubMineIndex ].sSectorY == sY ) )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}


BOOLEAN PlayerControlsMine(INT8 bMineIndex)
{
	// a value of TRUE is from the enemy's point of view
	if (StrategicMap[gMineLocation[bMineIndex].sSectorX + MAP_WORLD_X * gMineLocation[bMineIndex].sSectorY].fEnemyControlled)
		return(FALSE);
	else
	{
		// player only controls the actual mine after he has made arrangements to do so with the head miner there
		if (gMineStatus[ bMineIndex ].fSpokeToHeadMiner)
		{
			return(TRUE);
		}
		else
		{
			return(FALSE);
		}
	}
}


void SaveMineStatusToSaveGameFile(HWFILE const hFile)
{
	//Save the MineStatus
	FileWrite(hFile, gMineStatus, sizeof(MINE_STATUS_TYPE) * MAX_NUMBER_OF_MINES);
}


void LoadMineStatusFromSavedGameFile(HWFILE const hFile)
{
	//Load the MineStatus
	FileRead(hFile, gMineStatus, sizeof(MINE_STATUS_TYPE) * MAX_NUMBER_OF_MINES);
}


void ShutOffMineProduction( INT8 bMineIndex )
{
	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	if ( !gMineStatus[ bMineIndex ].fShutDown )
	{
		gMineStatus[ bMineIndex ].fShutDown = TRUE;
		AddHistoryToPlayersLog( HISTORY_MINE_SHUTDOWN, gMineLocation[ bMineIndex ].bAssociatedTown, GetWorldTotalMin( ), gMineLocation[ bMineIndex ].sSectorX,  gMineLocation[ bMineIndex ].sSectorY );
	}
}


void RestartMineProduction( INT8 bMineIndex )
{
	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	if ( !gMineStatus[ bMineIndex ].fShutDownIsPermanent )
	{
		if ( gMineStatus[ bMineIndex ].fShutDown )
		{
			gMineStatus[ bMineIndex ].fShutDown = FALSE;
			AddHistoryToPlayersLog( HISTORY_MINE_REOPENED, gMineLocation[ bMineIndex ].bAssociatedTown, GetWorldTotalMin( ), gMineLocation[ bMineIndex ].sSectorX,  gMineLocation[ bMineIndex ].sSectorY );
		}
	}
}


static void MineShutdownIsPermanent(INT8 bMineIndex)
{
	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	gMineStatus[ bMineIndex ].fShutDownIsPermanent = TRUE;
}


BOOLEAN IsMineShutDown( INT8 bMineIndex )
{
	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	return(gMineStatus[ bMineIndex ].fShutDown);
}


static UINT8 GetHeadMinerIndexForMine(INT8 bMineIndex)
{
	UINT8 ubMinerIndex = 0;
	UINT16 usProfileId = 0;


	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	// loop through all head miners, checking which town they're associated with, looking for one that matches this mine
	for (ubMinerIndex = 0; ubMinerIndex < NUM_HEAD_MINERS; ubMinerIndex++)
	{
		usProfileId = gHeadMinerData[ ubMinerIndex ].usProfileId;

		if (gMercProfiles[ usProfileId ].bTown == gMineLocation[ bMineIndex ].bAssociatedTown)
		{
			return(ubMinerIndex);
		}
	}

	// not found - yack!
	Assert( FALSE );
	return( 0 );
}


static UINT16 GetHeadMinerProfileIdForMine(INT8 bMineIndex)
{
	return(gHeadMinerData[ GetHeadMinerIndexForMine( bMineIndex ) ].usProfileId);
}


void IssueHeadMinerQuote(INT8 const mine_idx, HeadMinerQuote const quote_type)
{
	Assert(0 <= mine_idx && mine_idx < MAX_NUMBER_OF_MINES);
	Assert(quote_type < NUM_HEAD_MINER_STRATEGIC_QUOTES);
	Assert(CheckFact(FACT_MINERS_PLACED, 0));

	HEAD_MINER_TYPE const& miner_data = gHeadMinerData[GetHeadMinerIndexForMine(mine_idx)];

	// Make sure the miner isn't dead
	MERCPROFILESTRUCT const& p = GetProfile(miner_data.usProfileId);
	if (p.bLife < OKLIFE)
	{
		ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"Head Miner #%ls can't talk (quote #%d)", p.zNickname, quote_type);
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
	switch (mine_idx)
	{
		case MINE_GRUMM:    break;
		case MINE_CAMBRIA:  break;
		case MINE_ALMA:     break;
		case MINE_DRASSEN:  y = 135; break;
		case MINE_CHITZENA: y = 117; break;

		case MINE_SAN_MONA: // There's no head miner in San Mona, this is an error!
		default:
			Assert(FALSE);
			break;
	}
	SetExternMapscreenSpeechPanelXY(x, y);

	/* Cause this quote to come up for this profile id and an indicator to flash
	 * over the mine sector */
	MINE_LOCATION_TYPE const& loc = gMineLocation[mine_idx];
	HandleMinerEvent(miner_data.ubExternalFace, loc.sSectorX, loc.sSectorY, bQuoteNum, force_mapscreen);

	// Stop time compression with any miner quote - these are important events.
	StopTimeCompression();
}


UINT8 GetHeadMinersMineIndex( UINT8 ubMinerProfileId)
{
	UINT8 ubMineIndex;

	// find which mine this guy represents
	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
	{
		if (gMineLocation[ ubMineIndex ].bAssociatedTown == gMercProfiles[ ubMinerProfileId ].bTown)
		{
			return(ubMineIndex);
		}
	}

	// not found!  Illegal profile id receieved or something is very wrong
	Assert(FALSE);
	return( 0 );
}


void PlayerSpokeToHeadMiner( UINT8 ubMinerProfileId )
{
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	// if this is our first time set a history fact
	if (!gMineStatus[ubMineIndex].fSpokeToHeadMiner)
	{
		AddHistoryToPlayersLog( HISTORY_TALKED_TO_MINER, gMineLocation[ ubMineIndex ].bAssociatedTown, GetWorldTotalMin( ), gMineLocation[ ubMineIndex ].sSectorX,  gMineLocation[ ubMineIndex ].sSectorY );
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
	UINT8 ubMineIndex;

	ubMineIndex = GetHeadMinersMineIndex( ubMinerProfileId );

	if (gTownLoyalty[ gMineLocation[ ubMineIndex ].bAssociatedTown ].ubRating < LOW_MINE_LOYALTY_THRESHOLD)
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


void QueenHasRegainedMineSector(INT8 bMineIndex)
{
	Assert( ( bMineIndex >= 0 ) && ( bMineIndex < MAX_NUMBER_OF_MINES ) );

	if (gMineStatus[ bMineIndex ].fMineHasProducedForPlayer)
	{
		gMineStatus[ bMineIndex ].fQueenRetookProducingMine = TRUE;
	}
}


BOOLEAN HasAnyMineBeenAttackedByMonsters(void)
{
	UINT8 ubMineIndex;

	// find which mine this guy represents
	for( ubMineIndex = 0; ubMineIndex < MAX_NUMBER_OF_MINES; ubMineIndex++ )
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

// gte the id of the mine for this sector x,y,z...-1 is invalid
INT8 GetIdOfMineForSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	INT8 bMineIndex = -1;
	INT16 sSectorValue;


	// are we even on the right level?
	if( ( bSectorZ < 0 ) && ( bSectorZ > 2 ) )
	{
		// nope
		return( -1 );
	}

	// now get the sectorvalue
	sSectorValue = SECTOR( sSectorX, sSectorY );

	// support surface
	if( bSectorZ == 0 )
	{
		bMineIndex = GetMineIndexForSector( sSectorX, sSectorY );
	}
	// handle for first level
	else if( bSectorZ == 1 )
	{
		switch( sSectorValue )
		{
			// grumm
			case( SEC_H3 ):
			case( SEC_I3 ):
				bMineIndex = MINE_GRUMM;
			break;
			// cambria
			case( SEC_H8 ):
			case( SEC_H9 ):
				bMineIndex = MINE_CAMBRIA;
			break;
			// alma
			case( SEC_I14 ):
			case( SEC_J14 ):
				bMineIndex = MINE_ALMA;
			break;
			// drassen
			case( SEC_D13 ):
			case( SEC_E13 ):
				bMineIndex = MINE_DRASSEN;
			break;
			// chitzena
			case( SEC_B2 ):
				bMineIndex = MINE_CHITZENA;
			break;
			// san mona
			case( SEC_D4 ):
			case( SEC_D5 ):
				bMineIndex = MINE_SAN_MONA;
			break;
		}
	}
	else
	{
		// level 2
		switch( sSectorValue )
		{
			case( SEC_I3 ):
			case( SEC_H3 ):
			case( SEC_H4 ):
				bMineIndex = MINE_GRUMM;
			break;
		}
	}

	return( bMineIndex );
}


// use this for miner (civilian) quotes when *underground* in a mine
static BOOLEAN PlayerForgotToTakeOverMine(UINT8 ubMineIndex)
{
	MINE_STATUS_TYPE *pMineStatus;


	Assert(ubMineIndex < MAX_NUMBER_OF_MINES);

	pMineStatus = &(gMineStatus[ ubMineIndex ]);

	// mine surface sector is player controlled
	// mine not empty
	// player hasn't spoken to the head miner, but hasn't attacked him either
	// miner is alive
	if (!StrategicMap[gMineLocation[ubMineIndex].sSectorX + MAP_WORLD_X * gMineLocation[ubMineIndex].sSectorY].fEnemyControlled &&
		 ( !pMineStatus->fEmpty ) &&
		 ( !pMineStatus->fSpokeToHeadMiner ) &&
		 ( !pMineStatus->fAttackedHeadMiner ) &&
		 ( gMercProfiles[ GetHeadMinerProfileIdForMine( ubMineIndex ) ].bLife > 0 ) )
	{
		return( TRUE );
	}

	return( FALSE );
}



// use this to determine whether or not to place miners into a underground mine level
BOOLEAN AreThereMinersInsideThisMine( UINT8 ubMineIndex )
{
	MINE_STATUS_TYPE *pMineStatus;


	Assert(ubMineIndex < MAX_NUMBER_OF_MINES);

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
