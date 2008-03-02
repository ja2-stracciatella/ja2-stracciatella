#ifndef __STRATPATH_H
#define __STRATPATH_H

#include "Map_Screen_Interface_Map.h"
#include "Strategic_Movement.h"


// directions of movement for badsector determination ( blocking off of a sector exit from foot or vehicle travel)

// Shortest Path Defines
#define NORTH_MOVE -18
#define EAST_MOVE    1
#define WEST_MOVE    -1
#define SOUTH_MOVE  18


// Movement speed defines
#define NORMAL_MVT 1
#define SLOW_MVT   0


// movment modes
enum{
	MVT_MODE_AIR,
	MVT_MODE_VEHICLE,
	MVT_MODE_FOOT,
};

INT32 FindStratPath(INT16 sStart, INT16 sDestination, INT16 sMvtGroupNumber, BOOLEAN fTacticalTraversal );

// build a stategic path
PathSt* BuildAStrategicPath(PathSt* pPath, INT16 iStartSectorNum, INT16 iEndSectorNum, INT16 sMvtGroupNumber, BOOLEAN fTacticalTraversal /*, BOOLEAN fTempPath */ );


// append onto path list
PathSt* AppendStrategicPath(PathSt* pNewSection, PathSt* pHeadOfPathList);


// clear out strategic path list
PathSt* ClearStrategicPathList(PathSt* pHeadOfPath, INT16 sMvtGroup);

// move to beginning of list
PathSt* MoveToBeginningOfPathList(PathSt* pList);

// remove head of list
PathSt* RemoveHeadFromStrategicPath(PathSt* pList);

// clear out path list after/including this sector sX, sY..will start at end of path and work it's way back till sector is found...removes most recent sectors first
PathSt* ClearStrategicPathListAfterThisSector(PathSt* pHeadOfPath, INT16 sX, INT16 sY, INT16 sMvtGroup);

// get id of last sector in mercs path list
INT16 GetLastSectorIdInCharactersPath(const SOLDIERTYPE* pCharacter);

// get id of last sector in mercs path list
INT16 GetLastSectorIdInVehiclePath( INT32 iId );

// copy paths
PathSt* CopyPaths(PathSt* pSourcePath, PathSt* pDestPath);

/*
// move character along path
void MoveCharacterOnPath( SOLDIERTYPE *pCharacter );
// move the whole team
void MoveTeamOnFoot( void );
*/

// rebuild way points for strategic mapscreen path changes
void RebuildWayPointsForGroupPath(PathSt* pHeadOfPath, INT16 sMvtGroup);

// clear strategic movement (mercpaths and waypoints) for this soldier, and his group (including its vehicles)
void ClearMvtForThisSoldierAndGang( SOLDIERTYPE *pSoldier );

// start movement of this group to this sector...not to be used by the player merc groups.
BOOLEAN MoveGroupFromSectorToSector( UINT8 ubGroupID, INT16 sStartX, INT16 sStartY, INT16 sDestX, INT16 sDestY );

BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectors( UINT8 ubGroupID, INT16 sStartX, INT16 sStartY, INT16 sDestX, INT16 sDestY );
BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectorsAndStopOneSectorBeforeEnd( UINT8 ubGroupID, INT16 sStartX, INT16 sStartY, INT16 sDestX, INT16 sDestY );

/*
BOOLEAN MoveGroupToOriginalSector( UINT8 ubGroupID );
*/


// get length of path
INT32 GetLengthOfPath(PathSt* pHeadPath);
INT32 GetLengthOfMercPath(const SOLDIERTYPE* pSoldier);

PathSt* GetSoldierMercPathPtr(const SOLDIERTYPE* pSoldier);
PathSt* GetGroupMercPathPtr(GROUP* pGroup);

UINT8 GetSoldierGroupId(const SOLDIERTYPE* pSoldier);

// clears this groups strategic movement (mercpaths and waypoints), include those in the vehicle structs(!)
void ClearMercPathsAndWaypointsForAllInGroup( GROUP *pGroup );

void AddSectorToFrontOfMercPathForAllSoldiersInGroup( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY );

#endif
