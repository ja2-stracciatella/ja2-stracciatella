#ifndef __EXIT_GRIDS_H
#define __EXIT_GRIDS_H

#include "JA2Types.h"

struct EXITGRID //for exit grids (object level)
{ //if an item pool is also in same gridno, then this would be a separate levelnode
	//in the object level list
	GridNo usGridNo; //sweet spot for placing mercs in new sector.
	SGPSector ubGotoSector;
};

bool ExitGridAtGridNo(GridNo);
bool GetExitGrid(GridNo, EXITGRID * pExitGrid);

void AddExitGridToWorld(GridNo, EXITGRID const * pExitGrid);
void RemoveExitGridFromWorld(GridNo);

void SaveExitGrids(SGPFile &);
void LoadExitGrids(HWFILE);
void TrashExitGrids();

void AttemptToChangeFloorLevel( INT8 bRelativeZLevel );

extern EXITGRID gExitGrid;
extern BOOLEAN gfOverrideInsertionWithExitGrid;

// Finds closest ExitGrid of same type as is at gridno, within a radius.  Checks
// valid paths, destinations, etc.
GridNo FindGridNoFromSweetSpotCloseToExitGrid(const SOLDIERTYPE* pSoldier, GridNo sSweetGridNo, INT8 ubRadius);

GridNo FindClosestExitGrid(const SOLDIERTYPE *pSoldier, GridNo sGridNo, INT8 ubRadius);

#endif
