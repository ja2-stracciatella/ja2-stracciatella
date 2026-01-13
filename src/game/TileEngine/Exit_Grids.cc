#include "Isometric_Utils.h"
#include "Types.h"
#include "WorldDef.h"
#include "Exit_Grids.h"
#include "StrategicMap.h"
#include "Strategic_Movement.h"
#include "Message.h"
#include "Font_Control.h"
#include "PathAI.h"
#include "Overhead.h"
#include "Animation_Control.h"
#include "Sys_Globals.h"
#include "SaveLoadMap.h"
#include "Text.h"
#include <map>

BOOLEAN gfLoadingExitGrids = FALSE;

//used by editor.
EXITGRID gExitGrid = {0, {1, 1, 0}};

BOOLEAN gfOverrideInsertionWithExitGrid = FALSE;

namespace {
std::map<GridNo, EXITGRID> gExitGrids;

auto find(GridNo gn)
{
	return gExitGrids.find(gn);
}
}

bool GetExitGrid(GridNo gridno, EXITGRID * pExitGrid)
{
	auto pos = find(gridno);
	if (pos != gExitGrids.end())
	{
		*pExitGrid = pos->second;
		return true;
	}

	pExitGrid->ubGotoSector = SGPSector(0, 0, 0);
	pExitGrid->usGridNo = 0;
	return false;
}

bool ExitGridAtGridNo(GridNo gridno)
{
	return find(gridno) != gExitGrids.end();
}


void AddExitGridToWorld(GridNo gridno, EXITGRID const * xg)
{
	gExitGrids[gridno] = *xg;

	// Add the exit grid to the sector, only if ApplyMapChangesToMapTempFile is held.
	if (!gfEditMode && !gfLoadingExitGrids)
	{
		AddExitGridToMapTempFile(gridno, xg, gWorldSector);
	}
}


void RemoveExitGridFromWorld(GridNo gridno)
{
	gExitGrids.erase(gridno);
}


void SaveExitGrids(SGPFile & f)
{
	UINT16 usNumExitGrids = static_cast<UINT16>(gExitGrids.size());
	f.write(&usNumExitGrids, 2);
	for (auto const& mapEntry : gExitGrids)
	{
		EXITGRID const& exitGrid = mapEntry.second;
		f.write(&mapEntry.first, 2);
		f.write(&exitGrid.usGridNo, 2);
		f.write(&exitGrid.ubGotoSector.x, 1);
		f.write(&exitGrid.ubGotoSector.y, 1);
		f.write(&exitGrid.ubGotoSector.z, 1);
	}
}


void LoadExitGrids(HWFILE const f)
{
	EXITGRID exitGrid;
	UINT16 usNumSaved;
	UINT16 usMapIndex;
	gfLoadingExitGrids = TRUE;
	f->read(&usNumSaved, sizeof(usNumSaved));
	for (UINT16 x = 0; x < usNumSaved; x++)
	{
		f->read(&usMapIndex, sizeof(usMapIndex));
		f->read(&exitGrid.usGridNo, 2);
		f->read(&exitGrid.ubGotoSector.x, 1);
		f->read(&exitGrid.ubGotoSector.y, 1);
		f->read(&exitGrid.ubGotoSector.z, 1);
		AddExitGridToWorld( usMapIndex, &exitGrid );
	}
	gfLoadingExitGrids = FALSE;
}


void TrashExitGrids()
{
	gExitGrids.clear();
}


void AttemptToChangeFloorLevel(INT8 const relative_z_level)
{
	if (relative_z_level == -1)
	{
		if (gWorldSector.z == 0)
		{ // on ground level -- can't go up!
			ScreenMsg(FONT_DKYELLOW, MSG_INTERFACE, pMessageStrings[MSG_CANT_GO_UP]);
			return;
		}
	}
	else if (relative_z_level == 1)
	{
		if (gWorldSector.z == 3)
		{ // on bottom level -- can't go down!
			ScreenMsg(FONT_DKYELLOW, MSG_INTERFACE, pMessageStrings[MSG_CANT_GO_DOWN]);
			return;
		}
	}
	else
	{
		return;
	}

	UINT8 const look_for_level = gWorldSector.z + relative_z_level;
	for (UINT16 i = 0; i != WORLD_MAX; ++i)
	{
		if (!GetExitGrid(i, &gExitGrid))               continue;
		if (gExitGrid.ubGotoSector.z != look_for_level) continue;
		// found an exit grid leading to the goal sector!

		gfOverrideInsertionWithExitGrid = TRUE;
		/* change all current mercs in the loaded sector, and move them to the new
		 * sector. */
		SGPSector const adjustedSector(gWorldSector.x, gWorldSector.y, look_for_level);
		MoveAllGroupsInCurrentSectorToSector(adjustedSector);
		if (look_for_level)
		{
			ScreenMsg(FONT_YELLOW, MSG_INTERFACE, st_format_printf(pMessageStrings[MSG_ENTERING_LEVEL], look_for_level));
		}
		else
		{
			ScreenMsg(FONT_YELLOW, MSG_INTERFACE, pMessageStrings[MSG_LEAVING_BASEMENT]);
		}
		SetCurrentWorldSector(adjustedSector);
		gfOverrideInsertionWithExitGrid = FALSE;
		break;
	}
}


GridNo FindGridNoFromSweetSpotCloseToExitGrid(const SOLDIERTYPE* const pSoldier, const GridNo sSweetGridNo, const INT8 ubRadius)
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	SOLDIERTYPE soldier{};
	EXITGRID	ExitGrid;
	SGPSector ubGotoSector;

	// Turn off at end of function...
	gfPlotPathToExitGrid = TRUE;

	//Save AI pathing vars.  changing the distlimit restricts how
	//far away the pathing will consider.
	SaveNPCBudgetAndDistLimit const savePathAIvars(0, ubRadius);

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.
	soldier.bLevel = 0;
	soldier.bTeam = 1;
	soldier.sGridNo = pSoldier->sGridNo;

	// OK, Get an exit grid ( if possible )
	if ( !GetExitGrid( sSweetGridNo, &ExitGrid ) )
	{
		return( NOWHERE );
	}

	// Copy our dest values.....
	ubGotoSector = ExitGrid.ubGotoSector;

	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	//in the square region.
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = pSoldier->sGridNo + (WORLD_COLS * cnt1) + cnt2;
			if ( sGridNo >= 0 && sGridNo < WORLD_MAX )
			{
				gpWorldLevelData[ sGridNo ].uiFlags &= (~MAPELEMENT_REACHABLE);
			}
		}
	}

	//Now, find out which of these gridnos are reachable
	//(use the fake soldier and the pathing settings)
	FindBestPath( &soldier, NOWHERE, 0, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE );

	uiLowestRange = 999999;

	INT16 sLowestGridNo = NOWHERE;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( pSoldier->sGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = pSoldier->sGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if ( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) &&
				gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE )
			{
				// Go on sweet stop
				// ATE: Added this check because for all intensive purposes, cavewalls will be not an OKDEST
				// but we want thenm too...
				if ( NewOKDestination( pSoldier, sGridNo, TRUE, pSoldier->bLevel ) )
				{
					if ( GetExitGrid( sGridNo, &ExitGrid ) )
					{
						// Is it the same exitgrid?
						if (ExitGrid.ubGotoSector == ubGotoSector)
						{
							uiRange = GetRangeInCellCoordsFromGridNoDiff( pSoldier->sGridNo, sGridNo );

							if ( uiRange < uiLowestRange )
							{
								sLowestGridNo = sGridNo;
								uiLowestRange = uiRange;
							}
						}
					}
				}
			}
		}
	}

	gfPlotPathToExitGrid = FALSE;

	return sLowestGridNo;
}


GridNo FindClosestExitGrid(const SOLDIERTYPE *, GridNo sSrcGridNo, INT8 ubRadius)
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	EXITGRID	ExitGrid;


	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//clear the mapelements of potential residue MAPELEMENT_REACHABLE flags
	uiLowestRange = 999999;

	INT16 sLowestGridNo = NOWHERE;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSrcGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSrcGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				if ( GetExitGrid( sGridNo, &ExitGrid ) )
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( sSrcGridNo, sGridNo );

					if ( uiRange < uiLowestRange )
					{
						sLowestGridNo = sGridNo;
						uiLowestRange = uiRange;
					}
				}
			}
		}
	}

	return sLowestGridNo;
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(ExitGrids, asserts)
{
	EXPECT_EQ(sizeof(EXITGRID), 8u);
}

#endif
