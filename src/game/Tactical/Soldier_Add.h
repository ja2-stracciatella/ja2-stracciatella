#ifndef _SOLDIER_ADD_H
#define _SOLDIER_ADD_H

#include "game/JA2Types.h"


// Finds a gridno given a sweet spot
UINT16 FindGridNoFromSweetSpot(const SOLDIERTYPE* pSoldier, INT16 sSweetGridNo, INT8 ubRadius);

// Ensures a good path.....
UINT16 FindGridNoFromSweetSpotThroughPeople(const SOLDIERTYPE* pSoldier, INT16 sSweetGridNo, INT8 ubRadius);

// Returns a good sweetspot but not the swetspot!
UINT16 FindGridNoFromSweetSpotExcludingSweetSpot(const SOLDIERTYPE* pSoldier, INT16 sSweetGridNo, INT8 ubRadius);

UINT16 FindGridNoFromSweetSpotExcludingSweetSpotInQuardent(const SOLDIERTYPE* pSoldier, INT16 sSweetGridNo, INT8 ubRadius, INT8 ubQuardentDir);

// Finds a gridno near a sweetspot but a random one!
UINT16 FindRandomGridNoFromSweetSpot(const SOLDIERTYPE* pSoldier, INT16 sSweetGridNo, INT8 ubRadius);


// Adds a soldier ( already created in mercptrs[] array )!
// Finds a good placement based on data in the loaded sector and if they are enemy's or not, etc...
void AddSoldierToSector(SOLDIERTYPE*);

void AddSoldierToSectorNoCalculateDirection(SOLDIERTYPE*);

void AddSoldierToSectorNoCalculateDirectionUseAnimation(SOLDIERTYPE*, UINT16 usAnimState, UINT16 usAnimCode);

// IsMercOnTeam() checks to see if the passed in Merc Profile ID is currently on the player's team
BOOLEAN IsMercOnTeam(UINT8 ubMercID);
// ATE: Added for contract renewals
BOOLEAN IsMercOnTeamAndInOmertaAlreadyAndAlive(UINT8 ubMercID);


UINT16 FindGridNoFromSweetSpotWithStructData( SOLDIERTYPE *pSoldier, UINT16 usAnimState, INT16 sSweetGridNo, INT8 ubRadius, UINT8 *pubDirection, BOOLEAN fClosestToMerc );

UINT16 FindGridNoFromSweetSpotWithStructDataFromSoldier(const SOLDIERTYPE* pSoldier, UINT16 usAnimState, INT8 ubRadius, BOOLEAN fClosestToMerc, const SOLDIERTYPE* pSrcSoldier);

void SoldierInSectorPatient( SOLDIERTYPE *pSoldier, INT16 sGridNo );
void SoldierInSectorDoctor( SOLDIERTYPE *pSoldier, INT16 sGridNo );
void SoldierInSectorRepair( SOLDIERTYPE *pSoldier, INT16 sGridNo );

BOOLEAN CanSoldierReachGridNoInGivenTileLimit( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT16 sMaxTiles, INT8 bLevel );


#endif
