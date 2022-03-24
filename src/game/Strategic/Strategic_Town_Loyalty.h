#ifndef __STRATEGIC_TOWN_LOYALTY_H
#define __STRATEGIC_TOWN_LOYALTY_H

#include "JA2Types.h"
#include <vector>


// gain pts per real loyalty pt
#define GAIN_PTS_PER_LOYALTY_PT 500


// --- LOYALTY BONUSES ---
// Omerta
#define LOYALTY_BONUS_MIGUEL_READS_LETTER									(10 * GAIN_PTS_PER_LOYALTY_PT)	// multiplied by 4.5 due to Omerta's high seniment, so it's 45%
// Drassen
#define LOYALTY_BONUS_CHILDREN_FREED_DOREEN_KILLED				(10 * GAIN_PTS_PER_LOYALTY_PT)	// +50% bonus for Drassen
#define LOYALTY_BONUS_CHILDREN_FREED_DOREEN_SPARED				(20 * GAIN_PTS_PER_LOYALTY_PT)	// +50% bonus for Drassen
// Cambria
#define LOYALTY_BONUS_MARTHA_WHEN_JOEY_RESCUED						(15 * GAIN_PTS_PER_LOYALTY_PT)	// -25% for low Cambria sentiment
#define LOYALTY_BONUS_KEITH_WHEN_HILLBILLY_SOLVED					(15 * GAIN_PTS_PER_LOYALTY_PT)	// -25% for low Cambria sentiment
// Chitzena
#define LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_LOCAL		(20 * GAIN_PTS_PER_LOYALTY_PT)	// +75% higher in Chitzena
#define LOYALTY_BONUS_YANNI_WHEN_CHALICE_RETURNED_GLOBAL	(10 * GAIN_PTS_PER_LOYALTY_PT)	// for ALL towns!
// Alma
#define LOYALTY_BONUS_AUNTIE_WHEN_BLOODCATS_KILLED				(20 * GAIN_PTS_PER_LOYALTY_PT)	// Alma's increases reduced by half due to low rebel sentiment
#define LOYALTY_BONUS_MATT_WHEN_DYNAMO_FREED							(20 * GAIN_PTS_PER_LOYALTY_PT)	// Alma's increases reduced by half due to low rebel sentiment
#define LOYALTY_BONUS_FOR_SERGEANT_KROTT									(20 * GAIN_PTS_PER_LOYALTY_PT)	// Alma's increases reduced by half due to low rebel sentiment
// Everywhere
#define LOYALTY_BONUS_TERRORISTS_DEALT_WITH								( 5 * GAIN_PTS_PER_LOYALTY_PT)
#define LOYALTY_BONUS_KILL_QUEEN_MONSTER									(10 * GAIN_PTS_PER_LOYALTY_PT)
// Anywhere
// loyalty bonus for completing town training
#define LOYALTY_BONUS_FOR_TOWN_TRAINING										( 2 * GAIN_PTS_PER_LOYALTY_PT )		// 2%



// --- LOYALTY PENALTIES ---
// Cambria
#define LOYALTY_PENALTY_MARTHA_HEART_ATTACK								(20 * GAIN_PTS_PER_LOYALTY_PT)
#define LOYALTY_PENALTY_JOEY_KILLED												(10 * GAIN_PTS_PER_LOYALTY_PT)
// Balime
#define LOYALTY_PENALTY_ELDIN_KILLED											(20 * GAIN_PTS_PER_LOYALTY_PT)	// effect is double that!
// Any mine
#define LOYALTY_PENALTY_HEAD_MINER_ATTACKED								(20 * GAIN_PTS_PER_LOYALTY_PT)	// exact impact depends on rebel sentiment in that town
// Loyalty penalty for being inactive, per day after the third
#define LOYALTY_PENALTY_INACTIVE													(10 * GAIN_PTS_PER_LOYALTY_PT)

enum GlobalLoyaltyEventTypes
{
	// There are only for distance-adjusted global loyalty effects.  Others go into list above instead!
	GLOBAL_LOYALTY_BATTLE_WON,
	GLOBAL_LOYALTY_BATTLE_LOST,
	GLOBAL_LOYALTY_ENEMY_KILLED,
	GLOBAL_LOYALTY_NATIVE_KILLED,
	GLOBAL_LOYALTY_GAIN_TOWN_SECTOR,
	GLOBAL_LOYALTY_LOSE_TOWN_SECTOR,
	GLOBAL_LOYALTY_LIBERATE_WHOLE_TOWN,		// awarded only the first time it happens
	GLOBAL_LOYALTY_ABANDON_MILITIA,
	GLOBAL_LOYALTY_GAIN_MINE,
	GLOBAL_LOYALTY_LOSE_MINE,
	GLOBAL_LOYALTY_GAIN_SAM,
	GLOBAL_LOYALTY_LOSE_SAM,
	GLOBAL_LOYALTY_QUEEN_BATTLE_WON,
};


struct TOWN_LOYALTY
{
	UINT8			ubRating;
	INT16			sChange;
	BOOLEAN		fStarted;		// starting loyalty of each town is initialized only when player first enters that town
	BOOLEAN		fLiberatedAlready;
};


// the loyalty variables for each town
extern TOWN_LOYALTY gTownLoyalty[];

// whether town maintains/displays loyalty or not
extern BOOLEAN gfTownUsesLoyalty[];


struct TownSectorInfo
{
	UINT8 town;
	UINT8 sector;
};

extern std::vector<TownSectorInfo> g_town_sectors;

#define FOR_EACH_TOWN_SECTOR(iter) \
	for (auto iter = g_town_sectors.begin(); iter != g_town_sectors.end(); ++iter)

#define FOR_EACH_SECTOR_IN_TOWN(iter, town_) \
	FOR_EACH_TOWN_SECTOR(iter) if (iter->town != (town_)) continue; else


// initialize a specific town's loyalty if it hasn't already been
void StartTownLoyaltyIfFirstTime( INT8 bTownId );

// set a speciafied town's loyalty rating
void SetTownLoyalty( INT8 bTownId, UINT8 ubLoyaltyValue );

// increment the town loyalty rating (hundredths!)
void IncrementTownLoyalty( INT8 bTownId, UINT32 uiLoyaltyIncrease );

// decrement the town loyalty rating (hundredths!)
void DecrementTownLoyalty( INT8 bTownId, UINT32 uiLoyaltyDecrease );

// init town loyalty lists
void InitTownLoyalty( void );

// handle the death of a civ
void HandleMurderOfCivilian(const SOLDIERTYPE* pSoldier);

// handle town loyalty adjustment for recruitment
void HandleTownLoyaltyForNPCRecruitment( SOLDIERTYPE *pSoldier );

// remove random item from this sector
void RemoveRandomItemsInSector(const SGPSector& sector, UINT8 ubChance );

// build list of town sectors
void BuildListOfTownSectors( void );


void LoadStrategicTownLoyaltyFromSavedGameFile(HWFILE);
void SaveStrategicTownLoyaltyToSaveGameFile(HWFILE);

void ReduceLoyaltyForRebelsBetrayed(void);

// how many towns under player control?
INT32 GetNumberOfWholeTownsUnderControl( void );

// is all the sectors of this town under control by the player
INT32 IsTownUnderCompleteControlByPlayer( INT8 bTownId );

// used when monsters attack a town sector without going through tactical and they win
void AdjustLoyaltyForCivsEatenByMonsters(const SGPSector& sSector, UINT8 ubHowMany);

// these are used to handle global loyalty events (ones that effect EVERY town on the map)
void IncrementTownLoyaltyEverywhere( UINT32 uiLoyaltyIncrease );
void DecrementTownLoyaltyEverywhere( UINT32 uiLoyaltyDecrease );
void HandleGlobalLoyaltyEvent(UINT8 ubEventType, const SGPSector& sSector);

// handle a town being liberated for the first time
void CheckIfEntireTownHasBeenLiberated(INT8 bTownId, const SGPSector& sSector);
void CheckIfEntireTownHasBeenLost(INT8 bTownId, const SGPSector& sSector);

void HandleLoyaltyChangeForNPCAction( UINT8 ubNPCProfileId );

bool DidFirstBattleTakePlaceInThisTown(INT8 town);
void SetTheFirstBattleSector( INT16 sSectorValue );

// gte number of whole towns but exclude this one
INT32 GetNumberOfWholeTownsUnderControlButExcludeCity( INT8 bCityToExclude );

//Function assumes that mercs have retreated already.  Handles two cases, one for general merc retreat
//which slightly demoralizes the mercs, the other handles abandonment of militia forces which poses
//as a serious loyalty penalty.

#define RETREAT_TACTICAL_TRAVERSAL 0
#define RETREAT_PBI 1
#define RETREAT_AUTORESOLVE 2
void HandleLoyaltyImplicationsOfMercRetreat(INT8 bRetreatCode, const SGPSector& sSector);

void MaximizeLoyaltyForDeidrannaKilled( void );

#endif
