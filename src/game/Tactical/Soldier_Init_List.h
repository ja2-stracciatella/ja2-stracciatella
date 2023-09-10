#ifndef __SOLDIER_INIT_LIST_H
#define __SOLDIER_INIT_LIST_H

#include "Soldier_Create.h"

struct SOLDIERINITNODE
{
	UINT8 ubNodeID;
	UINT8 ubSoldierID;
	BASIC_SOLDIERCREATE_STRUCT *pBasicPlacement;
	SOLDIERCREATE_STRUCT *pDetailedPlacement;
	SOLDIERTYPE *pSoldier;
	SOLDIERINITNODE* prev;
	SOLDIERINITNODE* next;
};

extern SOLDIERINITNODE *gSoldierInitHead;
extern SOLDIERINITNODE *gSoldierInitTail;

#define BASE_FOR_EACH_SOLDIERINITNODE(type, iter)                      \
	for (type* iter = gSoldierInitHead; iter != NULL; iter = iter->next)
#define FOR_EACH_SOLDIERINITNODE( iter) BASE_FOR_EACH_SOLDIERINITNODE(      SOLDIERINITNODE, iter)
#define CFOR_EACH_SOLDIERINITNODE(iter) BASE_FOR_EACH_SOLDIERINITNODE(const SOLDIERINITNODE, iter)

#define FOR_EACH_SOLDIERINITNODE_SAFE(iter)                                                 \
	for (SOLDIERINITNODE* iter = gSoldierInitHead, * iter##__next; iter; iter = iter##__next) \
		if (iter##__next = iter->next, false) {} else                                           \

//These serialization functions are assuming the passing of a valid file
//pointer to the beginning of the save/load area, at the correct part of the
//map file.
void LoadSoldiersFromMap(HWFILE, bool stracLinuxFormat);

BOOLEAN SaveSoldiersToMap( HWFILE fp );

//For the purpose of keeping track of which soldier belongs to which placement within the game,
//the only way we can do this properly is to save the soldier ID from the list and reconnect the
//soldier pointer whenever we load the game.
void SaveSoldierInitListLinks(HWFILE);
void LoadSoldierInitListLinks(HWFILE);
void NewWayOfLoadingEnemySoldierInitListLinks(HWFILE);
void NewWayOfLoadingCivilianInitListLinks(HWFILE);

void InitSoldierInitList(void);
void KillSoldierInitList(void);
SOLDIERINITNODE* AddBasicPlacementToSoldierInitList(BASIC_SOLDIERCREATE_STRUCT const&);
void RemoveSoldierNodeFromInitList( SOLDIERINITNODE *pNode );
SOLDIERINITNODE* FindSoldierInitNodeWithID(SoldierID soldierID);
SOLDIERINITNODE* FindSoldierInitNodeBySoldier(SOLDIERTYPE const&);

void AddSoldierInitListTeamToWorld(INT8 team);
void AddSoldierInitListEnemyDefenceSoldiers( UINT8 ubTotalAdmin, UINT8 ubTotalTroops, UINT8 ubTotalElite );
void AddSoldierInitListCreatures(BOOLEAN fQueen, UINT8 ubNumLarvae, UINT8 ubNumInfants,
					UINT8 ubNumYoungMales, UINT8 ubNumYoungFemales, UINT8 ubNumAdultMales,
					UINT8 ubNumAdultFemales);
void AddSoldierInitListMilitia( UINT8 ubNumGreen, UINT8 ubNumReg, UINT8 ubNumElites );

void AddSoldierInitListBloodcats(void);

void UseEditorOriginalList(void);
void UseEditorAlternateList(void);

// Any killed people that used detailed placement information must prevent that
// from occurring again in the future.  Otherwise, the sniper guy with 99
// marksmanship could appear again if the map was loaded again!
void EvaluateDeathEffectsToSoldierInitList(SOLDIERTYPE const&);

void AddProfilesUsingProfileInsertionData(void);
void AddProfilesNotUsingProfileInsertionData(void);
void StripEnemyDetailedPlacementsIfSectorWasPlayerLiberated();
bool AddPlacementToWorld(SOLDIERINITNODE*);

#endif
