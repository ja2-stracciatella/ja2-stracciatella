//
// Filename        :       pathai.h
// Author          :       Ray E. Bornert II
// Date            :       1992-MAR-15
//
// Copyright (C) 1993 HixoxiH Software
//

#ifndef _PATHAI_H
#define _PATHAI_H

#include "JA2Types.h"
#include "Structure_Internals.h"


void InitPathAI(void);
void ShutDownPathAI( void );
INT16 PlotPath(        SOLDIERTYPE* pSold, INT16 sDestGridno, INT8 bCopyRoute, INT8 bPlot, UINT16 usMovementMode, INT16 sAPBudget);
INT16 UIPlotPath(      SOLDIERTYPE* pSold, INT16 sDestGridno, INT8 bCopyRoute, INT8 bPlot, UINT16 usMovementMode, INT16 sAPBudget);
INT16 EstimatePlotPath(SOLDIERTYPE* pSold, INT16 sDestGridno, INT8 bCopyRoute, INT8 bPlot, UINT16 usMovementMode, INT16 sAPBudget);

void ErasePath();
INT32 FindBestPath(SOLDIERTYPE* s, INT16 sDestination, INT8 ubLevel, INT16 usMovementMode, INT8 bCopy, UINT8 fFlags);
void GlobalReachableTest( INT16 sStartGridNo );
void GlobalItemsReachableTest( INT16 sStartGridNo1, INT16 sStartGridNo2 );
void RoofReachableTest( INT16 sStartGridNo, UINT8 ubBuildingID );
void LocalReachableTest( INT16 sStartGridNo, INT8 bRadius );

UINT8 DoorTravelCost(const SOLDIERTYPE* pSoldier, INT32 iGridNo, UINT8 ubMovementCost, BOOLEAN fReturnPerceivedValue, INT32* piDoorGridNo);
UINT8 InternalDoorTravelCost(const SOLDIERTYPE* pSoldier, INT32 iGridNo, UINT8 ubMovementCost, BOOLEAN fReturnPerceivedValue, INT32* piDoorGridNo, BOOLEAN fReturnDoorCost);
BOOLEAN IsDoorObstacleIfClosed(UINT8 ubMovementCost, INT32 iGridNo, INT32* iDoorGridNo, INT32* iDoorGridNo2);
StructureFlags GetDoorState(int32_t iDoorGridNo, bool returnPerceivedValue);

INT16 RecalculatePathCost( SOLDIERTYPE *pSoldier, UINT16 usMovementMode );

// Exporting these global variables
extern UINT8 guiPathingData[256];
extern UINT8 gubNPCAPBudget;
extern UINT8 gubNPCDistLimit;
extern UINT8 gubNPCPathCount;
extern BOOLEAN gfPlotPathToExitGrid;
extern BOOLEAN gfNPCCircularDistLimit;
extern BOOLEAN gfEstimatePath;
extern BOOLEAN	gfPathAroundObstacles;
extern UINT8 gubGlobalPathFlags;

class SaveNPCBudgetAndDistLimit
{
	UINT8 mNPCAPBudget{gubNPCAPBudget};
	UINT8 mNPCDistLimit{gubNPCDistLimit};

public:
	SaveNPCBudgetAndDistLimit(UINT8 const newBudget, UINT8 const newDistLimit)
	{
		gubNPCAPBudget = newBudget;
		gubNPCDistLimit = newDistLimit;
	}

	~SaveNPCBudgetAndDistLimit()
	{
		gubNPCAPBudget = mNPCAPBudget;
		gubNPCDistLimit = mNPCDistLimit;
	}
};

// Ian's terrain values for travelling speed/pathing purposes
// Fixed by CJC March 4, 1998.  Please do not change these unless familiar
// with how this will affect the path code!

#define TRAVELCOST_NONE			0
#define TRAVELCOST_FLAT			10
#define TRAVELCOST_BUMPY			12
#define TRAVELCOST_GRASS			12
#define TRAVELCOST_THICK			16
#define TRAVELCOST_DEBRIS			20
#define TRAVELCOST_SHORE			30
#define TRAVELCOST_KNEEDEEP			36
#define TRAVELCOST_DEEPWATER			50
#define TRAVELCOST_FENCE			40

// these values are used to indicate "this is an obstacle
// if there is a door (perceived) open/closed in this tile

#define TRAVELCOST_DOORS_CLOSED_W_SW	212
#define TRAVELCOST_DOORS_CLOSED_HERE_S	213
#define TRAVELCOST_DOORS_CLOSED_N_NW	214
#define TRAVELCOST_DOORS_CLOSED_HERE_W	215
#define TRAVELCOST_DOORS_CLOSED_W_NW	216
#define TRAVELCOST_DOORS_CLOSED_HERE_N	217
#define TRAVELCOST_DOORS_CLOSED_N_NE	218
#define TRAVELCOST_DOORS_CLOSED_HERE_E	219
#define TRAVELCOST_DOOR_CLOSED_HERE		220
#define TRAVELCOST_DOOR_CLOSED_N		221
#define TRAVELCOST_DOOR_CLOSED_W		222
#define TRAVELCOST_DOOR_OPEN_HERE		223
#define TRAVELCOST_DOOR_OPEN_N			224
#define TRAVELCOST_DOOR_OPEN_NE			225
#define TRAVELCOST_DOOR_OPEN_E			226
#define TRAVELCOST_DOOR_OPEN_SE			227
#define TRAVELCOST_DOOR_OPEN_S			228
#define TRAVELCOST_DOOR_OPEN_SW			229
#define TRAVELCOST_DOOR_OPEN_W			230
#define TRAVELCOST_DOOR_OPEN_NW			231
#define TRAVELCOST_DOOR_OPEN_N_N		232
#define TRAVELCOST_DOOR_OPEN_NW_N		233
#define TRAVELCOST_DOOR_OPEN_NE_N		234
#define TRAVELCOST_DOOR_OPEN_W_W		235
#define TRAVELCOST_DOOR_OPEN_SW_W		236
#define TRAVELCOST_DOOR_OPEN_NW_W		237
#define TRAVELCOST_NOT_STANDING		248
#define TRAVELCOST_OFF_MAP			249
#define TRAVELCOST_CAVEWALL			250
#define TRAVELCOST_HIDDENOBSTACLE		251
#define TRAVELCOST_DOOR			252
#define TRAVELCOST_OBSTACLE			253
#define TRAVELCOST_WALL			254
#define TRAVELCOST_EXITGRID			255

#define TRAVELCOST_TRAINTRACKS			30
#define TRAVELCOST_DIRTROAD			9
#define TRAVELCOST_PAVEDROAD			9
#define TRAVELCOST_FLATFLOOR			10

#define TRAVELCOST_BLOCKED			(TRAVELCOST_OFF_MAP)
#define IS_TRAVELCOST_DOOR( x )		(x >= TRAVELCOST_DOORS_CLOSED_W_SW && x <= TRAVELCOST_DOOR_OPEN_NW_W)
#define IS_TRAVELCOST_CLOSED_DOOR( x )		(x >= TRAVELCOST_DOORS_CLOSED_W_SW && x << TRAVELCOST_DOOR_CLOSED_W)

// ------------------------------------------
// PLOT PATH defines
#define NO_PLOT				0
#define PLOT					1

#define NO_COPYROUTE				0
#define COPYROUTE				1
#define COPYREACHABLE           		2
#define COPYREACHABLE_AND_APS			3

#define PATH_THROUGH_PEOPLE			0x01
#define PATH_IGNORE_PERSON_AT_DEST		0x02
#define PATH_CLOSE_GOOD_ENOUGH			0x04

#define PATH_CLOSE_RADIUS			5

// ------------------------------------------

#endif
