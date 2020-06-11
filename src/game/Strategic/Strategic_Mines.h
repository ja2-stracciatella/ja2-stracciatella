#ifndef __STRATEGIC_MINES_H
#define __STRATEGIC_MINES_H

// the .h to the mine management system

#include "Types.h"


// the mines
enum MineID
{
	MINE_SAN_MONA = 0,
	MINE_DRASSEN,
	MINE_ALMA,
	MINE_CAMBRIA,
	MINE_CHITZENA,
	MINE_GRUMM,
	MAX_NUMBER_OF_MINES,
};

enum{
	MINER_FRED = 0,
	MINER_MATT,
	MINER_OSWALD,
	MINER_CALVIN,
	MINER_CARL,
	NUM_HEAD_MINERS,
};

// different types of mines
enum{
	SILVER_MINE=0,
	GOLD_MINE,
	NUM_MINE_TYPES,
};


// head miner quote types
enum HeadMinerQuote
{
	HEAD_MINER_STRATEGIC_QUOTE_RUNNING_OUT = 0,
	HEAD_MINER_STRATEGIC_QUOTE_CREATURES_ATTACK,
	HEAD_MINER_STRATEGIC_QUOTE_CREATURES_GONE,
	HEAD_MINER_STRATEGIC_QUOTE_CREATURES_AGAIN,
	NUM_HEAD_MINER_STRATEGIC_QUOTES
};


// the strategic mine structures	
struct MINE_STATUS_TYPE
{
	UINT8    ubMineType;								// type of mine (silver or gold)
	UINT32   uiMaxRemovalRate;					// fastest rate we can move ore from this mine in period

	UINT32   uiRemainingOreSupply;			// the total value left to this mine (-1 means unlimited)
	UINT32   uiOreRunningOutPoint;			// when supply drop below this, workers tell player the mine is running out of ore

	BOOLEAN  fEmpty;										// whether no longer minable
	BOOLEAN  fRunningOut;								// whether mine is beginning to run out
	BOOLEAN  fWarnedOfRunningOut;				// whether mine foreman has already told player the mine's running out
	BOOLEAN  fShutDownIsPermanent;			// means will never produce again in the game (head miner was attacked & died/quit)
	BOOLEAN  fShutDown;									// TRUE means mine production has been shut off
	BOOLEAN  fPrevInvadedByMonsters;		// whether or not mine has been previously invaded by monsters
	BOOLEAN  fSpokeToHeadMiner;					// player doesn't receive income from mine without speaking to the head miner first
	BOOLEAN  fMineHasProducedForPlayer;	// player has earned income from this mine at least once

	BOOLEAN  fQueenRetookProducingMine;	// whether or not queen ever retook a mine after a player had produced from it
	BOOLEAN  fAttackedHeadMiner;				// player has attacked the head miner, shutting down mine & decreasing loyalty
	UINT32   uiTimePlayerProductionStarted;		// time in minutes when 'fMineHasProducedForPlayer' was first set
};


// init mines
void InitializeMines( void );

void HourlyMinesUpdate(void);

// get total left in this mine
INT32 GetTotalLeftInMine( UINT8 ubMineIndex );

// get max rates for this mine (per period, per day)
UINT32 GetMaxPeriodicRemovalFromMine( UINT8 ubMineIndex );

// Get the max amount that can be mined in one day.
UINT32 GetMaxDailyRemovalFromMine(UINT8 mine_id);

// which town does this mine belong to?
INT8 GetTownAssociatedWithMine( UINT8 ubMineIndex );

// posts the actual mine production events daily
void PostEventsForMineProduction(void);

// the periodic checking for income from mines
void HandleIncomeFromMines( void );

// predict income from mines
INT32 PredictIncomeFromPlayerMines( void );

// predict income from a mine
UINT32 PredictDailyIncomeFromAMine(INT8 mine_id);

/* Calculate how much player could make daily if he owned all mines with 100%
 * control and 100% loyalty. */
INT32 CalcMaxPlayerIncomeFromMines();

// get index of this mine, return -1 if no mine found
INT8 GetMineIndexForSector(UINT8 sector);

// get the sector value for the mine associated with this town
INT16 GetMineSectorForTown(INT8 town_id);

//Save the mine status to the save game file
void SaveMineStatusToSaveGameFile(HWFILE);

//Load the mine status from the saved game file
void LoadMineStatusFromSavedGameFile(HWFILE);

// if the player controls a given mine
bool PlayerControlsMine(INT8 mine_id);

void ShutOffMineProduction( UINT8 ubMineIndex );
void RestartMineProduction( UINT8 ubMineIndex );

BOOLEAN IsMineShutDown( UINT8 ubMineIndex );

// Find the sector location of a mine
UINT8 GetMineSector(UINT8 ubMineIndex);

void IssueHeadMinerQuote(UINT8 ubMineIndex, HeadMinerQuote);

UINT8 GetHeadMinersMineIndex(UINT8 ubMinerProfileId);

void PlayerSpokeToHeadMiner(UINT8 ubMinerProfile );

BOOLEAN IsHisMineRunningOut(UINT8 ubMinerProfileId );
BOOLEAN IsHisMineEmpty(UINT8 ubMinerProfileId );
BOOLEAN IsHisMineDisloyal(UINT8 ubMinerProfileId );
BOOLEAN IsHisMineInfested(UINT8 ubMinerProfileId );
BOOLEAN IsHisMineLostAndRegained(UINT8 ubMinerProfileId );
BOOLEAN IsHisMineAtMaxProduction(UINT8 ubMinerProfileId );
void ResetQueenRetookMine( UINT8 ubMinerProfileId );

void QueenHasRegainedMineSector(UINT8 ubMineIndex);

BOOLEAN HasAnyMineBeenAttackedByMonsters(void);

void PlayerAttackedHeadMiner( UINT8 ubMinerProfileId );

BOOLEAN HasHisMineBeenProducingForPlayerForSomeTime( UINT8 ubMinerProfileId );

// Get the id of the mine for this sector x,y,z; -1 is invalid
INT8 GetIdOfMineForSector(INT16 x, INT16 y, INT8 z);

// use this to determine whether or not to place miners into a underground mine level
BOOLEAN AreThereMinersInsideThisMine( UINT8 ubMineIndex );

// use this to determine whether or not the player has spoken to a head miner
BOOLEAN SpokenToHeadMiner( UINT8 ubMineIndex );

extern std::vector<MINE_STATUS_TYPE> gMineStatus;

#endif
