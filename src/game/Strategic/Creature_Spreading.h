#ifndef __CREATURE_SPREADING_H
#define __CREATURE_SPREADING_H

#include "JA2Types.h"


void InitCreatureQuest(void);
void SpreadCreatures(void);
void DeleteCreatureDirectives(void);

void SaveCreatureDirectives(HWFILE);
void LoadCreatureDirectives(HWFILE, UINT32 uiSavedGameVersion);

BOOLEAN PrepareCreaturesForBattle(void);
void CreatureNightPlanning(void);
void CreatureAttackTown(UINT8 ubSectorID, BOOLEAN fSpecificSector);

void CheckConditionsForTriggeringCreatureQuest();

extern BOOLEAN gfUseCreatureMusic;

BOOLEAN MineClearOfMonsters( UINT8 ubMineIndex );

// Returns true if valid and creature quest over, false if creature quest active or not yet started
bool GetWarpOutOfMineCodes(SGPSector& sector, GridNo* insertion_grid_no);

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

enum CreatureHabitat : uint8_t
{
	QUEEN_LAIR,		//where the queen lives.  Highly protected
	LAIR,			//part of the queen's lair -- lots of babies and defending mothers
	LAIR_ENTRANCE,		//where the creatures access the mine.
	INNER_MINE,		//parts of the mines that aren't close to the outside world
	OUTER_MINE,		//area's where miners work, close to towns, creatures love to eat :)
	FEEDING_GROUNDS,	//creatures love to populate these sectors :)
	MINE_EXIT,		//the area that creatures can initiate town attacks if lots of monsters.
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
