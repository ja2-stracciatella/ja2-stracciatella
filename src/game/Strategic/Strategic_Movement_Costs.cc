#include "Campaign_Types.h"
#include "Strategic_Movement.h"
#include "Strategic_Movement_Costs.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "MovementCostsModel.h"


void InitStrategicMovementCosts()
{
	auto movementCosts = GCM->getMovementCosts();
	SGPSector sMap;
	for (sMap.y = 1; sMap.y <= 16; ++sMap.y)
	{
		for (sMap.x = 1; sMap.x <= 16; ++sMap.x)
		{
			SECTORINFO& s = SectorInfo[sMap.AsByte()];
			s.ubTravelRating                           = movementCosts->getTravelRating(sMap);
			s.ubTraversability[WEST_STRATEGIC_MOVE]    = movementCosts->getTraversibilityWestEast(sMap);
			++sMap.x;
			s.ubTraversability[EAST_STRATEGIC_MOVE]    = movementCosts->getTraversibilityWestEast(sMap);
			--sMap.x;
			s.ubTraversability[NORTH_STRATEGIC_MOVE]   = movementCosts->getTraversibilityNorthSouth(sMap);
			++sMap.y;
			s.ubTraversability[SOUTH_STRATEGIC_MOVE]   = movementCosts->getTraversibilityNorthSouth(sMap);
			--sMap.y;
			s.ubTraversability[THROUGH_STRATEGIC_MOVE] = movementCosts->getTraversibilityThrough(sMap);
		}
	}
}


UINT8 GetTraversability( INT16 sStartSector, INT16 sEndSector )
{
	UINT8 ubDirection = 0;
	INT16 sDifference = 0;

	// given start and end sectors
	sDifference = sEndSector - sStartSector;


	if( sDifference == -1 )
	{
		ubDirection = WEST_STRATEGIC_MOVE;
	}
	else if( sDifference == 1 )
	{
		ubDirection = EAST_STRATEGIC_MOVE;
	}
	else if( sDifference == 16 )
	{
		ubDirection = SOUTH_STRATEGIC_MOVE;
	}
	else
	{
		ubDirection = NORTH_STRATEGIC_MOVE;
	}

	return( SectorInfo[ sStartSector ].ubTraversability[ ubDirection ] );
}


bool SectorIsPassable(INT16 const sSector)
{
	// returns true if the sector is impassable in all directions
	const UINT8 t = SectorInfo[sSector].ubTraversability[THROUGH_STRATEGIC_MOVE];
	return t != GROUNDBARRIER && t != EDGEOFWORLD;
}
