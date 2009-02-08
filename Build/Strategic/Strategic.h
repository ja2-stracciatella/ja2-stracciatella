#ifndef __STRATEGIC_H
#define __STRATEGIC_H
// header for strategic structure

#include "JA2Types.h"
#include "MapScreen.h"


struct StrategicMapElement
{
	INT8  bNameId;
	BOOLEAN fEnemyControlled;   // enemy controlled or not
	BOOLEAN fEnemyAirControlled;
	INT8 bSAMCondition; // SAM Condition .. 0 - 100, just like an item's status
};

enum
{
	INSERTION_CODE_NORTH,
	INSERTION_CODE_SOUTH,
	INSERTION_CODE_EAST,
	INSERTION_CODE_WEST,
	INSERTION_CODE_GRIDNO,
	INSERTION_CODE_ARRIVING_GAME,
	INSERTION_CODE_CHOPPER,
	INSERTION_CODE_PRIMARY_EDGEINDEX,
	INSERTION_CODE_SECONDARY_EDGEINDEX,
	INSERTION_CODE_CENTER,
};


//PLEASE USE CALCULATE_STRATEGIC_INDEX() macro instead (they're identical).
//#define			GETWORLDMAPNO( x, y )		( x+(MAP_WORLD_X*y) )

extern StrategicMapElement StrategicMap[MAP_WORLD_X*MAP_WORLD_Y];

void HandleStrategicDeath(SOLDIERTYPE*);

#endif
