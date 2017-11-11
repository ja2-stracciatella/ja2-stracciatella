#ifndef __CREATURE_SPREADING_H
#define __CREATURE_SPREADING_H

#include "JA2Types.h"
#include "Types.h"


void InitCreatureQuest(void);
void SpreadCreatures(void);
void ClearCreatureQuest(void);
void DeleteCreatureDirectives(void);

void SaveCreatureDirectives(HWFILE);
void LoadCreatureDirectives(HWFILE, UINT32 uiSavedGameVersion);

BOOLEAN PrepareCreaturesForBattle(void);
void CreatureNightPlanning(void);
void CreatureAttackTown( UINT8 ubSectorID, BOOLEAN fOverrideTest );

void CheckConditionsForTriggeringCreatureQuest( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );

extern BOOLEAN gfUseCreatureMusic;

BOOLEAN MineClearOfMonsters( UINT8 ubMineIndex );

// Returns true if valid and creature quest over, false if creature quest active or not yet started
bool GetWarpOutOfMineCodes(INT16* sector_x, INT16* sector_y, INT8* sector_z, GridNo* insertion_grid_no);

extern INT16 gsCreatureInsertionCode;
extern INT16 gsCreatureInsertionGridNo;
extern UINT8 gubNumCreaturesAttackingTown;
extern UINT8 gubYoungMalesAttackingTown;
extern UINT8 gubYoungFemalesAttackingTown;
extern UINT8 gubAdultMalesAttackingTown;
extern UINT8 gubAdultFemalesAttackingTown;
extern UINT8 gubSectorIDOfCreatureAttack;
enum{
	CREATURE_BATTLE_CODE_NONE,
	CREATURE_BATTLE_CODE_TACTICALLYADD,
	CREATURE_BATTLE_CODE_TACTICALLYADD_WITHFOV,
	CREATURE_BATTLE_CODE_PREBATTLEINTERFACE,
	CREATURE_BATTLE_CODE_AUTORESOLVE,
};
extern UINT8 gubCreatureBattleCode;

void DetermineCreatureTownComposition(UINT8 ubNumCreatures,
					UINT8 *pubNumYoungMales, UINT8 *pubNumYoungFemales,
					UINT8 *pubNumAdultMales, UINT8 *pubNumAdultFemales);

void DetermineCreatureTownCompositionBasedOnTacticalInformation(UINT8 *pubNumCreatures,
								UINT8 *pubNumYoungMales, UINT8 *pubNumYoungFemales,
								UINT8 *pubNumAdultMales, UINT8 *pubNumAdultFemales);


BOOLEAN PlayerGroupIsInACreatureInfestedMine(void);

void EndCreatureQuest(void);

#endif
