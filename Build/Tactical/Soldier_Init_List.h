#ifndef __SOLDIER_INIT_LIST_H
#define __SOLDIER_INIT_LIST_H

#include "Soldier_Create.h"

typedef struct SOLDIERINITNODE
{
	UINT8 ubNodeID;
	UINT8 ubSoldierID;
	BASIC_SOLDIERCREATE_STRUCT *pBasicPlacement;
	SOLDIERCREATE_STRUCT *pDetailedPlacement;
	SOLDIERTYPE *pSoldier;
	struct SOLDIERINITNODE *prev;
	struct SOLDIERINITNODE *next;
}SOLDIERINITNODE;

extern SOLDIERINITNODE *gSoldierInitHead;
extern SOLDIERINITNODE *gSoldierInitTail;

#define BASE_FOR_ALL_SOLDIERINITNODES(type, iter)                      \
	for (type* iter = gSoldierInitHead; iter != NULL; iter = iter->next)
#define FOR_ALL_SOLDIERINITNODES( iter) BASE_FOR_ALL_SOLDIERINITNODES(      SOLDIERINITNODE, iter)
#define CFOR_ALL_SOLDIERINITNODES(iter) BASE_FOR_ALL_SOLDIERINITNODES(const SOLDIERINITNODE, iter)

#define FOR_ALL_SOLDIERINITNODES_SAFE(iter)                      \
	for (SOLDIERINITNODE* iter = gSoldierInitHead, * iter##__next; \
	     iter != NULL ? iter##__next = iter->next, TRUE : FALSE;   \
	     iter = iter##__next)                                      \


//These serialization functions are assuming the passing of a valid file
//pointer to the beginning of the save/load area, at the correct part of the
//map file.
BOOLEAN LoadSoldiersFromMap( INT8 **hBuffer );
BOOLEAN SaveSoldiersToMap( HWFILE fp );

//For the purpose of keeping track of which soldier belongs to which placement within the game,
//the only way we can do this properly is to save the soldier ID from the list and reconnect the
//soldier pointer whenever we load the game.
BOOLEAN SaveSoldierInitListLinks( HWFILE hfile );
BOOLEAN LoadSoldierInitListLinks( HWFILE hfile );
BOOLEAN NewWayOfLoadingEnemySoldierInitListLinks( HWFILE hfile );
BOOLEAN NewWayOfLoadingCivilianInitListLinks( HWFILE hfile );
BOOLEAN LookAtButDontProcessEnemySoldierInitListLinks( HWFILE hfile );

void InitSoldierInitList();
void KillSoldierInitList();
SOLDIERINITNODE* AddBasicPlacementToSoldierInitList( BASIC_SOLDIERCREATE_STRUCT *pBasicPlacement );
void RemoveSoldierNodeFromInitList( SOLDIERINITNODE *pNode );
SOLDIERINITNODE* FindSoldierInitNodeWithID( UINT16 usID );

UINT8 AddSoldierInitListTeamToWorld( INT8 bTeam, UINT8 ubMaxNum );
void AddSoldierInitListEnemyDefenceSoldiers( UINT8 ubTotalAdmin, UINT8 ubTotalTroops, UINT8 ubTotalElite );
void AddSoldierInitListCreatures( BOOLEAN fQueen, UINT8 ubNumLarvae, UINT8 ubNumInfants,
																	UINT8 ubNumYoungMales, UINT8 ubNumYoungFemales, UINT8 ubNumAdultMales,
																	UINT8 ubNumAdultFemales );
void AddSoldierInitListMilitia( UINT8 ubNumGreen, UINT8 ubNumReg, UINT8 ubNumElites );

void AddSoldierInitListBloodcats();

void UseEditorOriginalList();
void UseEditorAlternateList();

void EvaluateDeathEffectsToSoldierInitList( SOLDIERTYPE *pSoldier );

void AddProfilesUsingProfileInsertionData();
void AddProfilesNotUsingProfileInsertionData();

void StripEnemyDetailedPlacementsIfSectorWasPlayerLiberated(void);

#if defined JA2BETAVERSION
BOOLEAN ValidateSoldierInitLinks(UINT8 ubCode);
#endif

#endif
