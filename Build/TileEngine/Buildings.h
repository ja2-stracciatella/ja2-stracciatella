#ifndef BUILDING_H

#define BUILDING_H

#include "WorldDef.h"
// for what it's worth, 2 bytes, we use roof climb spots as 1-based
// so the 0th entry is always 0 and can be compared with (and not equal)
// NOWHERE or any other location
#define MAX_CLIMBSPOTS_PER_BUILDING 21

// similarly for buildings, only we really want 0 to be invalid index
#define NO_BUILDING 0
#define MAX_BUILDINGS 31


struct BUILDING
{
	INT16			sUpClimbSpots[MAX_CLIMBSPOTS_PER_BUILDING];
	INT16 		sDownClimbSpots[MAX_CLIMBSPOTS_PER_BUILDING];
	UINT8			ubNumClimbSpots;
};

extern UINT8 gubBuildingInfo[ WORLD_MAX ];

BUILDING * FindBuilding( INT16 sGridNo );
void GenerateBuildings( void );
INT16 FindClosestClimbPoint( INT16 sStartGridNo, INT16 sDesiredGridNo, BOOLEAN fClimbUp );
BOOLEAN SameBuilding( INT16 sGridNo1, INT16 sGridNo2 );

#endif
