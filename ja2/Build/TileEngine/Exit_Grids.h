#ifndef __EXIT_GRIDS_H
#define __EXIT_GRIDS_H

#include "Fileman.h"
#include "Worlddef.h"

typedef struct //for exit grids (object level)
{ //if an item pool is also in same gridno, then this would be a separate levelnode
	//in the object level list
	UINT16 usGridNo; //sweet spot for placing mercs in new sector.
	UINT8 ubGotoSectorX;
	UINT8 ubGotoSectorY;
	UINT8 ubGotoSectorZ;
}EXITGRID;

BOOLEAN	ExitGridAtGridNo( UINT16 usMapIndex );
BOOLEAN	GetExitGridLevelNode( UINT16 usMapIndex, LEVELNODE **ppLevelNode );
BOOLEAN	GetExitGrid( UINT16 usMapIndex, EXITGRID *pExitGrid );

void AddExitGridToWorld( INT32 iMapIndex, EXITGRID *pExitGrid );
void RemoveExitGridFromWorld( INT32 iMapIndex );

void SaveExitGrids( HWFILE fp, UINT16 usNumExitGrids );
void LoadExitGrids( INT8 **hBuffer );

void AttemptToChangeFloorLevel( INT8 bRelativeZLevel );

extern EXITGRID gExitGrid;
extern BOOLEAN gfOverrideInsertionWithExitGrid;

// Finds closest ExitGrid of same type as is at gridno, within a radius.  Checks
// valid paths, destinations, etc.
UINT16 FindGridNoFromSweetSpotCloseToExitGrid( SOLDIERTYPE *pSoldier, INT16 sSweetGridNo, INT8 ubRadius, UINT8 *pubDirection );

UINT16 FindClosestExitGrid( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 ubRadius );

#endif
