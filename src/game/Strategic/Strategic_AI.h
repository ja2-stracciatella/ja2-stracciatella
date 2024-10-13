#ifndef __STRATEGIC_AI_H
#define __STRATEGIC_AI_H

#include "Types.h"
struct GROUP;

#define SAVED_ARMY_COMPOSITIONS		60
#define SAVED_GARRISON_GROUPS		100
#define SAVED_PATROL_GROUPS		50

void InitStrategicAI(void);
void KillStrategicAI(void);

void SaveStrategicAI(HWFILE);
void LoadStrategicAI(HWFILE);

//NPC ACTION TRIGGERS SPECIAL CASE AI
enum
{
	STRATEGIC_AI_ACTION_WAKE_QUEEN = 1,
	STRATEGIC_AI_ACTION_KINGPIN_DEAD,
	STRATEGIC_AI_ACTION_QUEEN_DEAD,

};


void ExecuteStrategicAIAction(UINT16 usActionCode, const SGPSector* sMap);

void CheckEnemyControlledSector( UINT8 ubSectorID );
void EvaluateQueenSituation(void);

extern BOOLEAN gfUseAlternateQueenPosition;

//returns TRUE if the group was deleted.
BOOLEAN StrategicAILookForAdjacentGroups( GROUP *pGroup );
void RemoveGroupFromStrategicAILists(GROUP const&);
void RecalculateSectorWeight( UINT8 ubSectorID );
void RecalculateGroupWeight(GROUP const&);

BOOLEAN OkayForEnemyToMoveThroughSector( UINT8 ubSectorID );

void StrategicHandleQueenLosingControlOfSector(const SGPSector& sSector);

void WakeUpQueen(void);

void StrategicHandleMineThatRanOut( UINT8 ubSectorID );

size_t FindPatrolGroupIndexForGroupID( UINT8 ubGroupID );

void RepollSAIGroup( GROUP *pGroup );


extern BOOLEAN gfDisplayStrategicAILogs;
extern BOOLEAN gfFirstBattleMeanwhileScenePending;

extern UINT8 gubSAIVersion;



//These enumerations define all of the various types of stationary garrison
//groups, and index their compositions for forces, etc.
enum
{
	QUEEN_DEFENCE,			//The most important sector, the queen's palace.
	MEDUNA_DEFENCE,			//The town surrounding the queen's palace.
	MEDUNA_SAMSITE,			//A sam site within Meduna (higher priority)
	LEVEL1_DEFENCE,			//The sectors immediately adjacent to Meduna (defence and spawning area)
	LEVEL2_DEFENCE,			//Two sectors away from Meduna (defence and spawning area)
	LEVEL3_DEFENCE,			//Three sectors away from Meduna (defence and spawning area)
	ORTA_DEFENCE,				//The top secret military base containing lots of elites
	EAST_GRUMM_DEFENCE,	//The most-industrial town in Arulco (more mine income)
	WEST_GRUMM_DEFENCE,	//The most-industrial town in Arulco (more mine income)
	GRUMM_MINE,
	OMERTA_WELCOME_WAGON,//Small force that greets the player upon arrival in game.
	BALIME_DEFENCE,			//Rich town, paved roads, close to Meduna (in queen's favor)
	TIXA_PRISON,				//Prison, well defended, but no point in retaking
	TIXA_SAMSITE,				//The central-most sam site (important for queen to keep)
	ALMA_DEFENCE,				//The military town of Meduna.  Also very important for queen.
	ALMA_MINE,					//Mine income AND administrators
	CAMBRIA_DEFENCE,		//Medical town, large, central.
	CAMBRIA_MINE,
	CHITZENA_DEFENCE,		//Small town, small mine, far away.
	CHITZENA_MINE,
	CHITZENA_SAMSITE,		//Sam site near Chitzena.
	DRASSEN_AIRPORT,		//Very far away, a supply depot of little importance.
	DRASSEN_DEFENCE,		//Medium town, normal.
	DRASSEN_MINE,
	DRASSEN_SAMSITE,		//Sam site near Drassen (least importance to queen of all samsites)
	ROADBLOCK,					//General outside city roadblocks -- enhance chance of ambush?
	SANMONA_SMALL,
	NUM_ARMY_COMPOSITIONS
};


struct ARMY_COMPOSITION
{
	INT32 iReadability;					//contains the enumeration which is useless, but helps readability.
	INT8 bPriority;
	INT8 bElitePercentage;
	INT8 bTroopPercentage;
	INT8 bAdminPercentage;
	INT8 bDesiredPopulation;
	INT8 bStartPopulation;
	INT8 bPadding[10]; // XXX HACK000B

	bool empty()
	{
		return iReadability == 0 && bPriority == 0
			&& bElitePercentage == 0 && bTroopPercentage == 0 && bAdminPercentage == 0
			&& bDesiredPopulation == 0 && bStartPopulation == 0;
	}

};

//Defines the patrol groups -- movement groups.
struct PATROL_GROUP
{
	INT8	bSize;
	INT8	bPriority;
	UINT8	ubSectorID[4];
	INT8	bFillPermittedAfterDayMod100;
	UINT8 ubGroupID;
	INT8	bWeight;
	UINT8 ubPendingGroupID;
	INT8  bPadding[10]; // XXX HACK000B
};


//Defines all stationary defence forces.
struct GARRISON_GROUP
{
	UINT8 ubSectorID;
	UINT8	ubComposition;
	INT8 bWeight;
	UINT8 ubPendingGroupID;
	INT8 bPadding[10]; // XXX HACK000B
};

#endif
