// Filename        :       pathai.c
// Author          :       Ray E. Bornert II
// Date            :       1992-MAR-15

// Skip list additions
// Author          :       Chris Camfield
// Date            :       1997-NOV

#include "AI.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "Buildings.h"
#include "English.h"
#include "GameSettings.h"
#include "Handle_Doors.h"
#include "Input.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "Keys.h"
#include "Logger.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "PathAI.h"
#include "Points.h"
#include "Random.h"
#include "Soldier_Control.h"
#include "Structure.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"

// skiplist has extra level of pointers every 4 elements, so a level 5is optimized for
// 4 to the power of 5 elements, or 2 to the power of 10, 1024

//#define PATHAI_VISIBLE_DEBUG

//#define PATHAI_SKIPLIST_DEBUG

#ifdef PATHAI_VISIBLE_DEBUG
#include "JAScreens.h"
#include "RenderWorld.h"
#include "Video.h"

extern INT16 gsCoverValue[WORLD_MAX];
BOOLEAN gfDisplayCoverValues = TRUE;
static BOOLEAN gfDrawPathPoints = FALSE;
#endif

#include "Logger.h"

#include <algorithm>

BOOLEAN gfPlotPathToExitGrid = FALSE;
BOOLEAN gfRecalculatingExistingPathCost = FALSE;
UINT8 gubGlobalPathFlags = 0;

UINT8 gubBuildingInfoToSet;

// ABSOLUTE maximums
#define ABSMAX_SKIPLIST_LEVEL			6
#define ABSMAX_TRAIL_TREE			(65536)
#define ABSMAX_PATHQ				(1024)

// STANDARD maximums... configurable!
#define MAX_SKIPLIST_LEVEL			6
#define MAX_TRAIL_TREE				(16384)
#define MAX_PATHQ				(1024)

INT32 iMaxSkipListLevel = MAX_SKIPLIST_LEVEL;
INT32 iMaxTrailTree = MAX_TRAIL_TREE;
INT32 iMaxPathQ = MAX_PATHQ;

extern BOOLEAN gfGeneratingMapEdgepoints;

#define VEHICLE

#define TRAILCELLTYPE				UINT16

// OLD PATHAI STUFF
/////////////////////////////////////////////////
struct path_t
{
	INT32   iLocation; //4
	path_t* pNext[ABSMAX_SKIPLIST_LEVEL]; //4 * MAX_SKIPLIST_LEVEL (5) = 20
	INT16   sPathNdx; //2
	TRAILCELLTYPE usCostSoFar; //2
	TRAILCELLTYPE usCostToGo; //2
	TRAILCELLTYPE usTotalCost; //2
	INT8    bLevel; //1
	UINT8   ubTotalAPCost; //1
	UINT8   ubLegDistance; //1
};

struct trail_t
{
	INT16 nextLink;
	UINT8 stepDir;
	INT8  fFlags;
	INT16 sGridNo;
};

enum TrailFlags
{
	STEP_BACKWARDS = 0x01
};

#define EASYWATERCOST				TRAVELCOST_FLAT / 2
#define ISWATER(t)				(((t)==TRAVELCOST_KNEEDEEP) || ((t)==TRAVELCOST_DEEPWATER))
#define NOPASS					(TRAVELCOST_BLOCKED)

static path_t *pathQ;
static UINT16 gusPathShown,gusAPtsToMove;
static INT32  queRequests;
static INT32  iSkipListSize;
static INT32  iClosedListSize;
static INT8   bSkipListLevel;
static INT32  iSkipListLevelLimit[9] = {0, 4, 16, 64, 256, 1024, 4096, 16384, 65536};

// The estimated cost must never exceed the real cost, otherwise you lose the
// guarantee that you're getting the least-cost path from start to goal. (issue #375)
#define LOWESTCOST				(EASYWATERCOST)

#define ESTIMATE0				( (dx>dy) ?       (dx)      :       (dy) )
#define ESTIMATE1				( (dx<dy) ? (dx+(dy*10)/14) : (dy+(dx*10)/14) )
#define ESTIMATE2				LOWESTCOST*( (dx<dy) ? (dx+(dy*10)/14) : (dy+(dx*10)/14) )
#define ESTIMATEn				((int)(LOWESTCOST * std::hypot(dx, dy)))
#define ESTIMATEC				( (dx<dy) ? (LOWESTCOST * (dx * 14 + dy * 10) / 14) : (LOWESTCOST * (dy * 14 + dx * 10) / 14) )
#define ESTIMATE				ESTIMATEC

#define MAXCOST				(9990)
//#define MAXCOST				(255)
//#define TOTALCOST(pCurrPtr)			(pCurrPtr->usCostSoFar + pCurrPtr->usCostToGo)
#define TOTALCOST( ptr )			(ptr->usTotalCost)
#define XLOC(a)				(a%MAPWIDTH)
#define YLOC(a)				(a/MAPWIDTH)
//#define LEGDISTANCE(a,b)			( abs( XLOC(b)-XLOC(a) ) + abs( YLOC(b)-YLOC(a) ) )
#define LEGDISTANCE(x1, y1, x2, y2)		(std::abs(x2 - x1) + std::abs(y2 - y1))
//#define FARTHER(ndx,NDX)			( LEGDISTANCE( ndx->sLocation,sDestination) > LEGDISTANCE(NDX->sLocation,sDestination) )
#define FARTHER(ndx,NDX)			( ndx->ubLegDistance > NDX->ubLegDistance )

#define SETLOC( str, loc )\
{\
	(str).iLocation = loc;\
}

static TRAILCELLTYPE *trailCost;
static UINT8 *trailCostUsed;
static UINT8 gubGlobalPathCount = 0;
static trail_t *trailTree;

static short trailTreeNdx=0;

#define QHEADNDX				(0)
#define QPOOLNDX				(iMaxPathQ-1)

static path_t *pQueueHead;
static path_t *pClosedHead;


#define pathQNotEmpty				(pQueueHead->pNext[0] != NULL)
#define pathFound				(pQueueHead->pNext[0]->iLocation == iDestination)
#define pathNotYetFound			(!pathFound)

// Note, the closed list is maintained as a doubly-linked list;
// it's a regular queue, essentially, as we always add to the end
// and remove from the front

// pNext[0] is used for the next pointers
// and pNext[1] is used for prev pointers

/*
#define ClosedListAdd( pNew ) \
{\
	pNew->pNext[0] = pClosedHead;\
	pNew->pNext[1] = pClosedHead->pNext[1];\
	pClosedHead->pNext[1]->pNext[0] = pNew;\
	pClosedHead->pNext[1] = pNew;\
	pNew->iLocation = -1;\
	iClosedListSize++;\
}

#define ClosedListGet( pNew )\
{\
	if (queRequests<QPOOLNDX)\
	{\
		pNew = pathQ + (queRequests);\
		queRequests++;\
		std::fill_n(pNew->pNext, ABSMAX_SKIPLIST_LEVEL, nullptr);\
		pNew->bLevel = RandomSkipListLevel();\
	}\
	else if (iClosedListSize > 0)\
	{\
		pNew = pClosedHead->pNext[0];\
		pNew->pNext[1]->pNext[0] = pNew->pNext[0];\
		pNew->pNext[0]->pNext[1] = pNew->pNext[1];\
		iClosedListSize--;\
		queRequests++;\
		std::fill_n(pNew->pNext, ABSMAX_SKIPLIST_LEVEL, nullptr);\
		pNew->bLevel = RandomSkipListLevel();\
	}\
	else\
	{\
		pNew = NULL;\
	}\
}*/

// experiment 1, seemed to fail
#define ClosedListAdd( pNew ) \
{\
	pNew->pNext[0] = pClosedHead->pNext[0];\
	pClosedHead->pNext[0] = pNew;\
	pNew->iLocation = -1;\
	iClosedListSize++;\
}

#define ClosedListGet( pNew )\
{\
	if (queRequests<QPOOLNDX)\
	{\
		pNew = pathQ + (queRequests);\
		queRequests++;\
		pNew->bLevel = RandomSkipListLevel();\
	}\
	else if (iClosedListSize > 0)\
	{\
		pNew = pClosedHead->pNext[0];\
		pClosedHead->pNext[0] = pNew->pNext[0];\
		iClosedListSize--;\
		queRequests++;\
		std::fill_n(pNew->pNext, ABSMAX_SKIPLIST_LEVEL, nullptr);\
		pNew->bLevel = RandomSkipListLevel();\
	}\
	else\
	{\
		pNew = NULL;\
	}\
}

/*
#define ClosedListAdd( pNew ) \
{\
	pNew->pNext[0] = pClosedHead;\
	pNew->pNext[1] = pClosedHead->pNext[1];\
	pClosedHead->pNext[1]->pNext[0] = pNew;\
	pClosedHead->pNext[1] = pNew;\
	pNew->iLocation = -1;\
	iClosedListSize++;\
}

#define ClosedListGet( pNew )\
{\
	if (queRequests<QPOOLNDX)\
	{\
		pNew = pathQ + (queRequests);\
		queRequests++;\
		std::fill_n(pNew->pNext, ABSMAX_SKIPLIST_LEVEL, nullptr);\
		pNew->bLevel = RandomSkipListLevel();\
	}\
	else if (iClosedListSize > 0)\
	{\
		pNew = pClosedHead->pNext[0];\
		pNew->pNext[1]->pNext[0] = pNew->pNext[0];\
		pNew->pNext[0]->pNext[1] = pNew->pNext[1];\
		iClosedListSize--;\
		queRequests++;\
		std::fill_n(pNew->pNext, ABSMAX_SKIPLIST_LEVEL, nullptr);\
		pNew->bLevel = RandomSkipListLevel();\
	}\
	else\
	{\
		pNew = NULL;\
	}\
}*/

#define SkipListRemoveHead()\
{\
	pDel = pQueueHead->pNext[0];\
	for (iLoop = 0; iLoop < std::min(bSkipListLevel, pDel->bLevel); iLoop++)\
	{\
		pQueueHead->pNext[iLoop] = pDel->pNext[iLoop];\
	}\
	iSkipListSize--;\
	ClosedListAdd( pDel );\
}

#define SkipListInsert( pNew )\
{\
	pCurr = pQueueHead;\
	uiCost = TOTALCOST( pNew );\
	std::fill_n(pUpdate, MAX_SKIPLIST_LEVEL, nullptr);\
	for (iCurrLevel = bSkipListLevel - 1; iCurrLevel >= 0; iCurrLevel--)\
	{\
		pNext = pCurr->pNext[iCurrLevel];\
		while (pNext)\
		{\
			if ( uiCost > TOTALCOST( pNext ) || (uiCost == TOTALCOST( pNext ) && FARTHER( pNew, pNext ) ) )\
			{\
				pCurr = pNext;\
				pNext = pCurr->pNext[iCurrLevel];\
			}\
			else\
			{\
				break;\
			}\
		}\
		pUpdate[iCurrLevel] = pCurr;\
	}\
	pCurr = pCurr->pNext[0];\
	for (iCurrLevel = 0; iCurrLevel < pNew->bLevel; iCurrLevel++)\
	{\
		if (!(pUpdate[iCurrLevel]))\
		{\
			break;\
		}\
		pNew->pNext[iCurrLevel] = pUpdate[iCurrLevel]->pNext[iCurrLevel];\
		pUpdate[iCurrLevel]->pNext[iCurrLevel] = pNew;\
	}\
	iSkipListSize++;\
	if (iSkipListSize > iSkipListLevelLimit[bSkipListLevel])\
	{\
		pCurr = pQueueHead;\
		pNext = pQueueHead->pNext[bSkipListLevel - 1];\
		while( pNext )\
		{\
			if (pNext->bLevel > bSkipListLevel)\
			{\
				pCurr->pNext[bSkipListLevel] = pNext;\
				pCurr = pNext;\
			}\
			pNext = pNext->pNext[bSkipListLevel - 1];\
		}\
		pCurr->pNext[bSkipListLevel] = pNext;\
		bSkipListLevel++;\
	}\
}

#define REMQUEHEADNODE()			SkipListRemoveHead();

#define DELQUENODE(ndx)				SkipListRemoveHead()

#define REMAININGCOST(ptr)\
(\
	(dy = std::abs(iDestY-iLocY)),\
	(dx = std::abs(iDestX-iLocX)),\
	ESTIMATE\
)
/*
#define REMAININGCOST(ptr)\
(\
	(locY = (ptr)->iLocation/MAPWIDTH),\
	(locX = (ptr)->iLocation%MAPWIDTH),\
	(dy = std::abs(iDestY-locY)),\
	(dx = std::abs(iDestX-locX)),\
	ESTIMATE\
)*/

#define NEWQUENODE				ClosedListGet( pNewPtr )

#define QUEINSERT(ndx)				SkipListInsert( ndx )

#define GREENSTEPSTART				0
#define REDSTEPSTART				16
#define PURPLESTEPSTART				32
#define BLUESTEPSTART				48
#define ORANGESTEPSTART			64

UINT8   gubNPCAPBudget = 0;
UINT8   gubNPCDistLimit = 0;
BOOLEAN gfNPCCircularDistLimit = FALSE;
UINT8   gubNPCPathCount;

BOOLEAN gfPlotDirectPath = FALSE;
BOOLEAN gfEstimatePath = FALSE;
BOOLEAN gfPathAroundObstacles = TRUE;

static UINT32 guiPlottedPath[256];
UINT8  guiPathingData[256];
static INT32 giPathDataSize;
static INT32 giPlotCnt;

#define LOOPING_CLOCKWISE			0
#define LOOPING_COUNTERCLOCKWISE		1
#define LOOPING_REVERSE				2

#ifdef COUNT_PATHS
UINT32 guiSuccessfulPathChecks = 0;
UINT32 guiTotalPathChecks = 0;
UINT32 guiFailedPathChecks = 0;
UINT32 guiUnsuccessfulPathChecks = 0;
#endif


static INT8 RandomSkipListLevel(void)
{
	INT8 bLevel = 1;

	while( Random( 4 ) == 0 && bLevel < iMaxSkipListLevel - 1)
	{
		bLevel++;
	}
	return( bLevel );
}


void InitPathAI(void)
{
	pathQ         = new path_t[ABSMAX_PATHQ]{};
	trailCost     = new TRAILCELLTYPE[MAPLENGTH]{};
	trailCostUsed = new UINT8[MAPLENGTH]{};
	trailTree     = new trail_t[ABSMAX_TRAIL_TREE]{};
	pQueueHead = &(pathQ[QHEADNDX]);
	pClosedHead = &(pathQ[QPOOLNDX]);
}


void ShutDownPathAI( void )
{
	delete[] pathQ;
	delete[] trailCostUsed;
	delete[] trailCost;
	delete[] trailTree;
}


static void ReconfigurePathAI(INT32 iNewMaxSkipListLevel, INT32 iNewMaxTrailTree, INT32 iNewMaxPathQ)
{
	// make sure the specified parameters are reasonable
	iNewMaxSkipListLevel = std::clamp(iNewMaxSkipListLevel, 0, ABSMAX_SKIPLIST_LEVEL);
	iNewMaxTrailTree = std::clamp(iNewMaxTrailTree, 0, ABSMAX_TRAIL_TREE);
	iNewMaxPathQ = std::clamp(iNewMaxPathQ, 0, ABSMAX_PATHQ);
	// assign them
	iMaxSkipListLevel = iNewMaxSkipListLevel;
	iMaxTrailTree = iNewMaxTrailTree;
	iMaxPathQ = iNewMaxPathQ;
	// relocate the head of the closed list to the end of the array portion being used
	pClosedHead = &(pathQ[QPOOLNDX]);
	*pClosedHead = path_t{};
}


static void RestorePathAIToDefaults(void)
{
	iMaxSkipListLevel = MAX_SKIPLIST_LEVEL;
	iMaxTrailTree = MAX_TRAIL_TREE;
	iMaxPathQ = MAX_PATHQ;
	// relocate the head of the closed list to the end of the array portion being used
	pClosedHead = &(pathQ[QPOOLNDX]);
	*pClosedHead = path_t{};
}

///////////////////////////////////////////////////////////////////////
//	FINDBESTPATH                                                   /
////////////////////////////////////////////////////////////////////////
INT32 FindBestPath(SOLDIERTYPE* s, INT16 sDestination, INT8 ubLevel, INT16 usMovementMode, INT8 bCopy, UINT8 fFlags)
{
	INT32 iDestination = sDestination, iOrigination;
	UINT8 ubCnt = 0 , ubLoopStart = 0, ubLoopEnd = 0, ubLastDir = 0, ubStructIndex;
	INT8  bLoopState = LOOPING_CLOCKWISE;
	//BOOLEAN fLoopForwards = FALSE;
	BOOLEAN fCheckedBehind = FALSE;
	INT32 iDestX,iDestY, iLocX, iLocY, dx, dy;
	INT32 newLoc,curLoc;
	//INT32 curY;
	INT32 curCost,newTotCost,nextCost;
	INT16 sCurPathNdx;
	INT32 prevCost;
	INT32 iWaterToWater;
	UINT8 ubCurAPCost,ubAPCost;
	UINT8 ubNewAPCost=0;
	#ifdef VEHICLE
		//BOOLEAN fTurnSlow = FALSE;
		//BOOLEAN fReverse = FALSE; // stuff for vehicles turning
		BOOLEAN fMultiTile;
		//BOOLEAN fVehicle;
		//INT32 ubLastDir, iPrevToLastDir;
		//INT8 bVehicleCheckDir;
		//UINT16 adjLoc;
		STRUCTURE_FILE_REF *pStructureFileRef=NULL;
		UINT16 usAnimSurface;
		//INT32 iCnt2, iCnt3;
	#endif

	path_t *pNewPtr;
	path_t *pCurrPtr;

	path_t *pUpdate[ABSMAX_SKIPLIST_LEVEL];
	path_t *pCurr;
	path_t *pNext;
	path_t *pDel;
	UINT32 uiCost;
	INT32 iCurrLevel, iLoop;

	UINT16 usOKToAddStructID=0;

	BOOLEAN fCopyReachable;
	BOOLEAN fCopyPathCosts;
	BOOLEAN fVisitSpotsOnlyOnce;
	INT32 iOriginationX, iOriginationY, iX, iY;

	//BOOLEAN fTurnBased;
	BOOLEAN fPathingForPlayer;
	INT32   iDoorGridNo=-1;
	INT32	iDoor2GridNo = -1;
	BOOLEAN fDoorIsObstacleIfClosed= 0; // if false, door is obstacle if it is open
	DOOR    *pDoor;
	BOOLEAN fDoorIsOpen = FALSE;
	StructureFlags  doorState{};
	StructureFlags  door2State{};
	BOOLEAN fNonFenceJumper;
	BOOLEAN fNonSwimmer;
	BOOLEAN fPathAroundPeople;
	BOOLEAN fGoingThroughDoor = FALSE; // for one tile
	BOOLEAN fContinuousTurnNeeded;
	BOOLEAN fCloseGoodEnough;
	UINT16  usMovementModeToUseForAPs;
	INT16   sClosePathLimit = -1; // XXX HACK000E

#ifdef PATHAI_VISIBLE_DEBUG
	UINT16 usCounter = 0;
#endif

	//fVehicle = FALSE;
	iOriginationX = iOriginationY = 0;
	iOrigination = (INT32) s->sGridNo;

	if (iOrigination < 0 || iOrigination >= WORLD_MAX)
	{
		SLOGE("Trying to calculate path from off-world gridno {} to {}",
			iOrigination, sDestination );
		return( 0 );
	}
	else if (!GridNoOnVisibleWorldTile( (INT16) iOrigination ) )
	{
		SLOGE("Trying to calculate path from non-visible gridno {} to {}",
			iOrigination, sDestination );
		return( 0 );
	}
	else if (s->bLevel != ubLevel)
	{
		// pathing to a different level... bzzzt!
		return( 0 );
	}

	if ( gubGlobalPathFlags )
	{
		fFlags |= gubGlobalPathFlags;
	}

	//fTurnBased = (gTacticalStatus.uiFlags & INCOMBAT);

	fPathingForPlayer = ( (s->bTeam == OUR_TEAM) && (!gTacticalStatus.fAutoBandageMode) && !(s->uiStatusFlags & SOLDIER_PCUNDERAICONTROL) );
	fNonFenceJumper = !( IS_MERC_BODY_TYPE( s ) );
	fNonSwimmer = !( IS_MERC_BODY_TYPE( s ) );
	if ( fNonSwimmer )
	{
		if ( Water( sDestination ) )
		{
			return( 0 );
		}
	}
	fPathAroundPeople = ( (fFlags & PATH_THROUGH_PEOPLE) == 0 );
	fCloseGoodEnough = ( (fFlags & PATH_CLOSE_GOOD_ENOUGH) != 0);
	if ( fCloseGoodEnough )
	{
		sClosePathLimit = std::min(PythSpacesAway( (INT16)s->sGridNo, sDestination ) - 1,  PATH_CLOSE_RADIUS);
		if ( sClosePathLimit <= 0 )
		{
			return( 0 );
		}
	}

	BOOLEAN const fConsiderPersonAtDestAsObstacle = fPathingForPlayer && fPathAroundPeople && !(fFlags & PATH_IGNORE_PERSON_AT_DEST);

	if (bCopy >= COPYREACHABLE)
	{
		fCopyReachable = TRUE;
		fCopyPathCosts = (bCopy == COPYREACHABLE_AND_APS);
		fVisitSpotsOnlyOnce = (bCopy == COPYREACHABLE);
		// make sure we aren't trying to copy path costs for an area greater than the AI array...
		if (fCopyPathCosts && gubNPCDistLimit > AI_PATHCOST_RADIUS)
		{
			// oy!!!! dis no supposed to happen!
			gubNPCDistLimit = AI_PATHCOST_RADIUS;
		}
	}
	else
	{
		fCopyReachable = FALSE;
		fCopyPathCosts = FALSE;
		fVisitSpotsOnlyOnce = FALSE;
	}

	gubNPCPathCount++;

	if (gubGlobalPathCount == 255)
	{
		// reset arrays!
		std::fill_n(trailCostUsed, MAPLENGTH, 0);
		gubGlobalPathCount = 1;
	}
	else
	{
		gubGlobalPathCount++;
	}

	// only allow nowhere destination if distance limit set
	if (sDestination == NOWHERE)
	{
		/*
		if (gubNPCDistLimit == 0)
		{
			return( FALSE );
		}*/
	}
	else
	{
		// the very first thing to do is make sure the destination tile is reachable
		if (!NewOKDestination( s, sDestination, fConsiderPersonAtDestAsObstacle, ubLevel ))
		{
			gubNPCAPBudget = 0;
			gubNPCDistLimit = 0;
			return( FALSE );
		}

		if (sDestination == s->sGridNo)
		{
			return( FALSE );
		}
	}

	if (gubNPCAPBudget)
	{
		ubAPCost = MinAPsToStartMovement( s, usMovementMode );
		if (ubAPCost > gubNPCAPBudget)
		{
			gubNPCAPBudget = 0;
			gubNPCDistLimit = 0;
			return( 0 );
		}
		else
		{
			gubNPCAPBudget -= ubAPCost;
		}
	}

#ifdef COUNT_PATHS
	guiTotalPathChecks++;
#endif

#ifdef VEHICLE

	fMultiTile = ((s->uiStatusFlags & SOLDIER_MULTITILE) != 0);
	if (fMultiTile)
	{
		// Get animation surface...
		// Chris_C... change this to use parameter.....
		usAnimSurface = DetermineSoldierAnimationSurface( s, usMovementMode );
		// Get structure ref...
		pStructureFileRef = GetAnimationStructureRef(s, usAnimSurface, usMovementMode);

		if ( pStructureFileRef )
		{
			//fVehicle = ( (s->uiStatusFlags & SOLDIER_VEHICLE) != 0 );

			fContinuousTurnNeeded = ( ( s->uiStatusFlags & (SOLDIER_MONSTER | SOLDIER_ANIMAL | SOLDIER_VEHICLE) ) != 0 );

			/*
			if (fVehicle && s->bReverse)
			{
				fReverse = TRUE;
			}*/
			/*
			if (fVehicle || s->ubBodyType == COW || s->ubBodyType == BLOODCAT) // or a vehicle
			{
				fTurnSlow = TRUE;
			}*/
			if ( gfEstimatePath )
			{
				usOKToAddStructID = IGNORE_PEOPLE_STRUCTURE_ID;
			}
			else
			{
				usOKToAddStructID = GetStructureID(s);
			}

		}
		else
		{
			// turn off multitile pathing
			fMultiTile = FALSE;
			fContinuousTurnNeeded = FALSE;
		}

	}
	else
	{
		fContinuousTurnNeeded = FALSE;
	}

#endif

	if (!fContinuousTurnNeeded)
	{
		ubLoopStart = 0;
		ubLoopEnd = 0;
		bLoopState = LOOPING_CLOCKWISE;
	}

	ubCurAPCost = 0;
	queRequests = 2;

	//initialize the path data structures
	std::fill_n(pathQ, iMaxPathQ, path_t{});
	std::fill_n(trailTree, iMaxTrailTree, trail_t{});

#if defined( PATHAI_VISIBLE_DEBUG )
	if (gfDisplayCoverValues && gfDrawPathPoints)
	{
		std::fill_n(gsCoverValue, WORLD_MAX, 0x7F7F);
	}
#endif

	bSkipListLevel = 1;
	iSkipListSize = 0;
	iClosedListSize = 0;

	trailTreeNdx=0;

	//set up common info
	if (fCopyPathCosts)
	{
		iOriginationY = (iOrigination / MAPWIDTH);
		iOriginationX = (iOrigination % MAPWIDTH);
	}

	iDestY = (iDestination / MAPWIDTH);
	iDestX = (iDestination % MAPWIDTH);

	// if origin and dest is water, then user wants to stay in water!
	// so, check and set waterToWater flag accordingly
	if (iDestination == NOWHERE)
	{
		iWaterToWater = 0;
	}
	else
	{
		if (ISWATER(gubWorldMovementCosts[iOrigination][0][ubLevel]) &&
			ISWATER(gubWorldMovementCosts[iDestination][0][ubLevel]))
		{
				iWaterToWater = 1;
		}
		else
		{
				iWaterToWater = 0;
		}
	}

	//setup Q and first path record

	SETLOC( *pQueueHead, iOrigination );
	pQueueHead->usCostSoFar = MAXCOST;
	pQueueHead->bLevel = iMaxSkipListLevel - 1;

	pClosedHead->pNext[0] = pClosedHead;
	pClosedHead->pNext[1] = pClosedHead;

	//setup first path record
	iLocY = iOrigination / MAPWIDTH;
	iLocX = iOrigination % MAPWIDTH;

	SETLOC( pathQ[1], iOrigination );
	pathQ[1].sPathNdx = 0;
	pathQ[1].usCostSoFar = 0;
	if ( fCopyReachable )
	{
		pathQ[1].usCostToGo = 100;
	}
	else
	{
		pathQ[1].usCostToGo = REMAININGCOST( &(pathQ[1]) );
	}
	pathQ[1].usTotalCost = pathQ[1].usCostSoFar + pathQ[1].usCostToGo;
	pathQ[1].ubLegDistance = LEGDISTANCE( iLocX, iLocY, iDestX, iDestY );
	pathQ[1].bLevel = 1;
	pQueueHead->pNext[0] = &( pathQ[1] );
	iSkipListSize++;

	trailTreeNdx = 0;
	trailCost[iOrigination] = 0;
	pCurrPtr = pQueueHead->pNext[0];
	pCurrPtr->sPathNdx = trailTreeNdx;
	trailTreeNdx++;


	do
	{
		//remove the first and best path so far from the que
		pCurrPtr = pQueueHead->pNext[0];
		curLoc = pCurrPtr->iLocation;
		curCost = pCurrPtr->usCostSoFar;
		sCurPathNdx = pCurrPtr->sPathNdx;

		// remember the cost used to get here...
		prevCost = gubWorldMovementCosts[trailTree[sCurPathNdx].sGridNo][trailTree[sCurPathNdx].stepDir][ubLevel];

#if defined( PATHAI_VISIBLE_DEBUG )
		if (gfDisplayCoverValues && gfDrawPathPoints)
		{
			if (gsCoverValue[ curLoc ] > 0)
			{
				gsCoverValue[ curLoc ] *= -1;
			}
		}
#endif

#ifdef VEHICLE
		/*
		if (fTurnSlow)
		{
			if (pCurrPtr->pNext[0] == 0)
			{
				if (fReverse)
				{
					ubLastDir = OppositeDirection(s->bDirection);
				}
				else
				{
					ubLastDir = s->bDirection;
				}
				// start prev-to-last dir at same as current (could cause a problem)
				iPrevToLastDir = ubLastDir;
			}
			else
			{
				iPrevToLastDir = trailTree[trailTree[pCurrPtr->sPathNdx].nextLink].dirDelta;
				ubLastDir = trailTree[pCurrPtr->sPathNdx].dirDelta;
			}
		}*/
#endif

		if (gubNPCAPBudget)
		{
			ubCurAPCost = pCurrPtr->ubTotalAPCost;
		}
		if (fCopyReachable && prevCost != TRAVELCOST_FENCE)
		{
			gpWorldLevelData[curLoc].uiFlags |= MAPELEMENT_REACHABLE;
			if (gubBuildingInfoToSet > 0)
			{
				gubBuildingInfo[ curLoc ] = gubBuildingInfoToSet;
			}
		}

		DELQUENODE( pCurrPtr );

		if ( trailCostUsed[curLoc] == gubGlobalPathCount && trailCost[curLoc] < curCost)
			goto NEXTDIR;

		if (fContinuousTurnNeeded)
		{
			if (trailTreeNdx < 2)
			{
				ubLastDir = s->bDirection;
			}
			else if ( trailTree[pCurrPtr->sPathNdx].fFlags & STEP_BACKWARDS )
			{
				ubLastDir = OppositeDirection(trailTree[pCurrPtr->sPathNdx].stepDir);
			}
			else
			{
				ubLastDir = trailTree[pCurrPtr->sPathNdx].stepDir;
			}
			ubLoopStart = ubLastDir;
			ubLoopEnd = ubLastDir;
			bLoopState = LOOPING_CLOCKWISE;
			fCheckedBehind = FALSE;
		}

		//contemplate a new path in each direction
		//for ( ubCnt = ubLoopStart; ubCnt != ubLoopEnd; ubCnt = (ubCnt + iLoopIncrement) % MAXDIR )
		for ( ubCnt = ubLoopStart; ; )
		{

#ifdef VEHICLE
			/*
			if (fTurnSlow)
			{
				if (ubLastDir == iPrevToLastDir)
				{
					if (ubCnt != ubLastDir &&
							ubCnt != OneCDirection(ubLastDir) &&
							ubCnt != OneCCDirection(ubLastDir))
					{
						goto NEXTDIR;
					}
				}
				else
				{
					if ( ubCnt != ubLastDir )
					{
						goto NEXTDIR;
					}
				}
			}*/

			if ( bLoopState == LOOPING_REVERSE )
			{
				ubStructIndex = OppositeDirection(OneCDirection(ubCnt));
			}
			else
			{
				ubStructIndex = OneCDirection(ubCnt);
			}

			if (fMultiTile)
			{
				if ( fContinuousTurnNeeded )
				{
					if ( ubCnt != ubLastDir )
					{
						if ( !OkayToAddStructureToWorld( (INT16) curLoc, ubLevel, &(pStructureFileRef->pDBStructureRef[ ubStructIndex ]), usOKToAddStructID ) )
						{
							// we have to abort this loop and possibly reset the loop conditions to
							// search in the other direction (if we haven't already done the other dir)
							if (bLoopState == LOOPING_CLOCKWISE)
							{
								ubLoopStart = ubLastDir;
								ubLoopEnd = ubCnt;
								bLoopState = LOOPING_COUNTERCLOCKWISE; // backwards
								// when we go to the bottom of the loop, iLoopIncrement will be added to ubCnt
								// which is good since it avoids duplication of effort
								ubCnt = ubLoopStart;
								goto NEXTDIR;
							}
							else if ( bLoopState == LOOPING_COUNTERCLOCKWISE && !fCheckedBehind )
							{
								// check rear dir
								bLoopState = LOOPING_REVERSE;

								// NB we're stuck with adding 1 to the loop counter down below so configure to accomodate...
								//ubLoopStart = (ubLastDir + (MAXDIR / 2) - 1) % MAXDIR;
								ubLoopStart = OppositeDirection(OneCCDirection(ubLastDir));
								ubLoopEnd = (ubLoopStart + 2) % MAXDIR;
								ubCnt = ubLoopStart;
								fCheckedBehind = TRUE;
								goto NEXTDIR;
							}
							else
							{
								// done
								goto ENDOFLOOP;
							}

						}
					}
				}
				else if ( pStructureFileRef )
				{
					// check to make sure it's okay for us to turn to the new direction in our current tile
					if (!OkayToAddStructureToWorld( (INT16) curLoc, ubLevel, &(pStructureFileRef->pDBStructureRef[ ubStructIndex ]), usOKToAddStructID ) )
					{
						goto NEXTDIR;
					}
				}
			}

#endif

			newLoc = curLoc + DirIncrementer[ubCnt];

			if ( newLoc < 0 || newLoc >= GRIDSIZE )
			{
				SLOGW("Path Finding algorithm tried to go out of bounds at {}, in an attempt to find path from {} to {}", newLoc, iOrigination, iDestination);
				goto NEXTDIR;
			}

			if ( fVisitSpotsOnlyOnce && trailCostUsed[newLoc] == gubGlobalPathCount )
			{
				// on a "reachable" test, never revisit locations!
				goto NEXTDIR;
			}


			//if (gpWorldLevelData[newLoc].sHeight != ubLevel)
			//ATE: Movement onto cliffs? Check vs the soldier's gridno height
			// CJC: PREVIOUS LOCATION's height
			if ( gpWorldLevelData[ newLoc ].sHeight != gpWorldLevelData[ curLoc ].sHeight )
			{
				goto NEXTDIR;
			}

			if (gubNPCDistLimit)
			{
				if ( gfNPCCircularDistLimit )
				{
					if (PythSpacesAway( (INT16) iOrigination, (INT16) newLoc) > gubNPCDistLimit)
					{
						goto NEXTDIR;
					}
				}
				else
				{
					if (SpacesAway( (INT16) iOrigination, (INT16) newLoc) > gubNPCDistLimit)
					{
						goto NEXTDIR;
					}
				}
			}

			// AI check for mines
			if ( gpWorldLevelData[newLoc].uiFlags & MAPELEMENT_ENEMY_MINE_PRESENT && s->bSide != Side::FRIENDLY)
			{
				goto NEXTDIR;
			}

			/*
			if ( gpWorldLevelData[newLoc].uiFlags & (MAPELEMENT_ENEMY_MINE_PRESENT | MAPELEMENT_PLAYER_MINE_PRESENT) )
			{
				if (s->bSide == Side::FRIENDLY)
				{
					// For our team, skip a location with a known mines unless it is the end of our
					// path; for others on our side, skip such locations completely;
					if (s->bTeam != OUR_TEAM || newLoc != iDestination)
					{
						if (gpWorldLevelData[newLoc].uiFlags & MAPELEMENT_PLAYER_MINE_PRESENT)
						{
							goto NEXTDIR;
						}
					}
				}
				else
				{
					// For the enemy, always skip known mines
					if (gpWorldLevelData[newLoc].uiFlags & MAPELEMENT_ENEMY_MINE_PRESENT)
					{
						goto NEXTDIR;
					}
				}
			}*/

			//how much is admission to the next tile
			if ( gfPathAroundObstacles )
			{
				nextCost = gubWorldMovementCosts[ newLoc ][ ubCnt ][ ubLevel ];

				//ATE:	Check for differences from reality
				if (nextCost == TRAVELCOST_NOT_STANDING)
				{
					// for path plotting purposes, use the terrain value
					nextCost = gTileTypeMovementCost[ gpWorldLevelData[ newLoc ].ubTerrainID ];
				}
				else if ( nextCost == TRAVELCOST_EXITGRID )
				{
					if (gfPlotPathToExitGrid)
					{
						// replace with terrain cost so that we can plot path, otherwise is obstacle
						nextCost = gTileTypeMovementCost[ gpWorldLevelData[ newLoc ].ubTerrainID ];
					}
				}
				else if ( nextCost == TRAVELCOST_FENCE && fNonFenceJumper )
				{
					goto NEXTDIR;
				}
				else if ( IS_TRAVELCOST_DOOR( nextCost ) )
				{
					fDoorIsObstacleIfClosed = IsDoorObstacleIfClosed(nextCost, newLoc, &iDoorGridNo, &iDoor2GridNo);
					doorState = GetDoorState(iDoorGridNo, fPathingForPlayer);

					if (iDoor2GridNo == -1)
					{ // this travel direction depends on the state of only one door structure
						if ( !(doorState & STRUCTURE_ANYDOOR) )
						{ // door destroyed?
							nextCost = gTileTypeMovementCost[gpWorldLevelData[newLoc].ubTerrainID];
						}
						else
						{
							fDoorIsOpen = doorState & STRUCTURE_OPEN;
							if (doorState & STRUCTURE_GARAGEDOOR && ubCnt & 1 && !fDoorIsOpen)
							{  // cancel diagonal pathfinding through a closed sliding door
								goto NEXTDIR;
							}
						}
					}
					else
					{ // ... or two door structures and it's always diagonal
						door2State = GetDoorState(iDoor2GridNo, fPathingForPlayer);
						if ( !(doorState & STRUCTURE_ANYDOOR) && !(door2State & STRUCTURE_ANYDOOR) )
						{ // both doors destroyed?
							nextCost = gTileTypeMovementCost[gpWorldLevelData[newLoc].ubTerrainID];
						}
						else if ( !(doorState & STRUCTURE_OPEN) || !(door2State & STRUCTURE_OPEN) )
						{ // cancel it if at least one of the doors is closed 
							goto NEXTDIR;
						}
						fDoorIsOpen = TRUE;
					}
					// now determine movement cost... if it hasn't been changed already
					if ( IS_TRAVELCOST_DOOR( nextCost ) )
					{

						if (fDoorIsOpen)
						{
							if (fDoorIsObstacleIfClosed)
							{
								nextCost = gTileTypeMovementCost[ gpWorldLevelData[ newLoc ].ubTerrainID ];
							}
							else
							{
								nextCost = TRAVELCOST_OBSTACLE;
							}
						}
						else
						{
							if (fDoorIsObstacleIfClosed)
							{
								// door is closed and this should be an obstacle, EXCEPT if we are calculating
								// a path for an enemy or NPC with keys
								if ( fPathingForPlayer || ( s && (s->uiStatusFlags & SOLDIER_MONSTER || s->uiStatusFlags & SOLDIER_ANIMAL) ) )
								{
									nextCost = TRAVELCOST_OBSTACLE;
								}
								else
								{
									// have to check if door is locked and NPC does not have keys!
									pDoor = FindDoorInfoAtGridNo( iDoorGridNo );
									if (pDoor)
									{
										if (!pDoor->fLocked || s->bHasKeys)
										{
											// add to AP cost
											if (gubNPCAPBudget)
											{
												fGoingThroughDoor = TRUE;
											}
											nextCost = gTileTypeMovementCost[ gpWorldLevelData[ newLoc ].ubTerrainID ];
										}
										else
										{
											nextCost = TRAVELCOST_OBSTACLE;
										}
									}
									else
									{
										nextCost = gTileTypeMovementCost[ gpWorldLevelData[ newLoc ].ubTerrainID ];
									}
								}
							}
							else
							{
								nextCost = gTileTypeMovementCost[ gpWorldLevelData[ newLoc ].ubTerrainID ];
							}
						}
					}
				}
				else if ( (nextCost == TRAVELCOST_SHORE || nextCost == TRAVELCOST_KNEEDEEP || nextCost == TRAVELCOST_DEEPWATER ) && fNonSwimmer )
				{
					// creatures and animals can't go in water
					nextCost = TRAVELCOST_OBSTACLE;
				}


				// Apr. '96 - moved up be ahead of AP_Budget stuff
				if ( ( nextCost >= NOPASS) ) // || ( nextCost == TRAVELCOST_DOOR ) )
						goto NEXTDIR;

			}
			else
			{
				nextCost = TRAVELCOST_FLAT;
			}

			// if contemplated tile is NOT final dest and someone there, disqualify route
			// when doing a reachable test, ignore all locations with people in them
			if (fPathAroundPeople && ( (newLoc != iDestination) || fCopyReachable) )
			{
				// ATE: ONLY cancel if they are moving.....
				const SOLDIERTYPE* const tgt = WhoIsThere2(newLoc, s->bLevel);
				if (tgt != NULL && tgt != s)
				{
					// Check for movement....
					//if (fTurnBased || tgt->sFinalDestination == tgt->sGridNo || tgt->fDelayedMovement)
					//{
						goto NEXTDIR;
					//}
					//else
					//{
					//	nextCost += 50;
					//}
				}
			}

#ifdef VEHICLE
			if (fMultiTile)
			{
				// vehicle test for obstacles: prevent movement to next tile if
				// a tile covered by the vehicle in that position & direction
				// has an obstacle in it

				// because of the order in which animations are stored (dir 7 first,
				// then 0 1 2 3 4 5 6), we must subtract 1 from the direction
				// ATE: Send in our existing structure ID so it's ignored!

				if (!OkayToAddStructureToWorld( (INT16) newLoc, ubLevel, &(pStructureFileRef->pDBStructureRef[ ubStructIndex ]), usOKToAddStructID ) )
				{
					goto NEXTDIR;
				}

				/*
				// vehicles aren't moving any more....
				if (fVehicle)
				{
					// transmogrify pathing costs for vehicles!
					switch(nextCost)
					{
						case TRAVELCOST_THICK:
							nextCost = TRAVELCOST_GRASS;
							break;
						case TRAVELCOST_SHORE:
						case TRAVELCOST_KNEEDEEP:
						case TRAVELCOST_DEEPWATER:
						//case TRAVELCOST_DOOR:
						case TRAVELCOST_FENCE:
							nextCost = TRAVELCOST_OBSTACLE;
							break;

						default:
							break;
					}
				}
				*/
			}
#endif

			// NEW Apr 21 by Ian: abort if cost exceeds budget
			if (gubNPCAPBudget)
			{
				switch(nextCost)
				{
					case TRAVELCOST_NONE:
						ubAPCost = 0;
						break;

					case TRAVELCOST_DIRTROAD:
					case TRAVELCOST_FLAT:
						ubAPCost = AP_MOVEMENT_FLAT;
						break;
					//case TRAVELCOST_BUMPY:
					case TRAVELCOST_GRASS:
						ubAPCost = AP_MOVEMENT_GRASS;
						break;
					case TRAVELCOST_THICK:
						ubAPCost = AP_MOVEMENT_BUSH;
						break;
					case TRAVELCOST_DEBRIS:
						ubAPCost = AP_MOVEMENT_RUBBLE;
						break;
					case TRAVELCOST_SHORE:
						ubAPCost = AP_MOVEMENT_SHORE; // wading shallow water
						break;
					case TRAVELCOST_KNEEDEEP:
						ubAPCost = AP_MOVEMENT_LAKE; // wading waist/chest deep - very slow
						break;

					case TRAVELCOST_DEEPWATER:
						ubAPCost = AP_MOVEMENT_OCEAN; // can swim, so it's faster than wading
						break;

					//case TRAVELCOST_DOOR:
					//	ubAPCost = AP_MOVEMENT_FLAT;
					//	break;

					case TRAVELCOST_FENCE:
						ubAPCost = AP_JUMPFENCE;

						/*
						if ( sSwitchValue == TRAVELCOST_FENCE )
						{
							sPoints += sTileCost;

							// If we are changeing stance ( either before or after getting there....
							// We need to reflect that...
							switch(usMovementMode)
							{
								case RUNNING:
								case WALKING :

									// Add here cost to go from crouch to stand AFTER fence hop....
									// Since it's AFTER.. make sure we will be moving after jump...
									if ( ( ubCnt + 2 ) < iLastGrid )
									{
										sPoints += AP_CROUCH;
									}
									break;

								case SWATTING:

									// Add cost to stand once there BEFORE....
									sPoints += AP_CROUCH;
									break;

								case CRAWLING:

									// Can't do it here.....
									break;

							}
						}*/

						break;

					case TRAVELCOST_OBSTACLE:
					default:
						goto NEXTDIR; // Cost too much to be considered!
				}


				// don't make the mistake of adding directly to
				// ubCurAPCost, that must be preserved for remaining dirs!
				if (ubCnt & 1)
				{
					ubAPCost = (ubAPCost * 14) / 10;
				}

				usMovementModeToUseForAPs = usMovementMode;

				// ATE: if water, force to be walking always!
				if ( nextCost == TRAVELCOST_SHORE || nextCost == TRAVELCOST_KNEEDEEP ||
					nextCost == TRAVELCOST_DEEPWATER )
				{
					usMovementModeToUseForAPs = WALKING;
				}

				// adjust AP cost for movement mode
				switch( usMovementModeToUseForAPs )
				{
					case RUNNING:
					case ADULTMONSTER_WALKING:
						// save on casting
						ubAPCost = ubAPCost * 10 / ( (UINT8) (RUNDIVISOR * 10));
						//ubAPCost = (INT16)(((DOUBLE)sTileCost) / RUNDIVISOR );break;
						break;
					case WALKING:
					case ROBOT_WALK:
						ubAPCost = (ubAPCost + WALKCOST);
						break;
					case SWATTING:
						ubAPCost = (ubAPCost + SWATCOST);
						break;
					case CRAWLING:
						ubAPCost = (ubAPCost + CRAWLCOST);
						break;
				}

				if (nextCost == TRAVELCOST_FENCE)
				{
					switch( usMovementModeToUseForAPs )
					{
						case RUNNING:
						case WALKING :
							// Here pessimistically assume the path will continue after hopping the fence
							ubAPCost += AP_CROUCH;
							break;

						case SWATTING:

							// Add cost to stand once there BEFORE jumping....
							ubAPCost += AP_CROUCH;
							break;

						case CRAWLING:

							// Can't do it here.....
							goto NEXTDIR;
					}
				}
				else if (nextCost == TRAVELCOST_NOT_STANDING)
				{
					switch(usMovementModeToUseForAPs)
					{
						case RUNNING:
						case WALKING :
							// charge crouch APs for ducking head!
							ubAPCost += AP_CROUCH;
							break;

						default:
							break;
					}
				}
				else if (fGoingThroughDoor)
				{
					if (fPathingForPlayer)
					{
						ubAPCost += doorAPs[s->ubDoorHandleCode];
					}
					else
					{
						ubAPCost += doorAPs[HANDLE_DOOR_OPEN];
					}
					fGoingThroughDoor = FALSE;
				}


				ubNewAPCost = ubCurAPCost + ubAPCost;


				if (ubNewAPCost > gubNPCAPBudget)
				goto NEXTDIR;

			}

			if ( fCloseGoodEnough )
			{
				if ( PythSpacesAway( (INT16)newLoc, sDestination ) <= sClosePathLimit )
				{
					// stop the path here!
					iDestination = newLoc;
					sDestination = (INT16) newLoc;
					fCloseGoodEnough = FALSE;
				}
			}
			//make the destination look very attractive
			if (newLoc == iDestination)
				nextCost = 0;
			else
				if (gfPlotDirectPath && nextCost < NOPASS)
					nextCost = TRAVELCOST_FLAT;

			// make water cost attractive for water to water paths
			if (iWaterToWater)
			{
				if (ISWATER(prevCost))
					prevCost = EASYWATERCOST;
				if (ISWATER(nextCost))
					nextCost = EASYWATERCOST;
			}

			// NOTE: on September 24, 1997, Chris went back to a diagonal bias system
			if (ubCnt & 1)
			{
				// moving on a diagonal
				nextCost = nextCost * 14 / 10;
			}

			if ( bLoopState == LOOPING_REVERSE)
			{
				// penalize moving backwards to encourage turning sooner
				nextCost += 50;
			}

			newTotCost = curCost + nextCost;

			// have we found a path to the current location that
			// costs less than the best so far to the same location?
			if (trailCostUsed[newLoc] != gubGlobalPathCount || newTotCost < trailCost[newLoc])
			{

				#if defined( PATHAI_VISIBLE_DEBUG )
				if (gfDisplayCoverValues && gfDrawPathPoints)
				{
					if (gsCoverValue[newLoc] == 0x7F7F)
					{
						gsCoverValue[newLoc] = usCounter++;
					}
					/*
					else if (gsCoverValue[newLoc] >= 0)
					{
						gsCoverValue[newLoc]++;
					}
					else
					{
						gsCoverValue[newLoc]--;
					}*/
				}
				#endif

				//NEWQUENODE;
				//{
				if (queRequests<QPOOLNDX)
				{
					pNewPtr = pathQ + (queRequests);
					queRequests++;
					std::fill_n(pNewPtr->pNext, ABSMAX_SKIPLIST_LEVEL, nullptr);
					pNewPtr->bLevel = RandomSkipListLevel();
				}
				else if (iClosedListSize > 0)
				{
					pNewPtr = pClosedHead->pNext[0];
					pClosedHead->pNext[0] = pNewPtr->pNext[0];
					iClosedListSize--;
					queRequests++;
					std::fill_n(pNewPtr->pNext, ABSMAX_SKIPLIST_LEVEL, nullptr);
					pNewPtr->bLevel = RandomSkipListLevel();
				}
				else
				{
					pNewPtr = NULL;
				}
				//}

				if (pNewPtr == NULL)
				{
					#ifdef COUNT_PATHS
					guiFailedPathChecks++;
					#endif
					gubNPCAPBudget = 0;
					gubNPCDistLimit = 0;
					return(0);
				}

				//make new path to current location
				trailTree[trailTreeNdx].nextLink = sCurPathNdx;
				trailTree[trailTreeNdx].stepDir = ubCnt;
				if ( bLoopState == LOOPING_REVERSE )
				{
					trailTree[trailTreeNdx].fFlags = STEP_BACKWARDS;
				}
				else
				{
					trailTree[trailTreeNdx].fFlags = 0;
				}
				trailTree[trailTreeNdx].sGridNo = (INT16) newLoc;
				pNewPtr->sPathNdx = trailTreeNdx;
				trailTreeNdx++;

				if (trailTreeNdx >= iMaxTrailTree)
				{
					#ifdef COUNT_PATHS
					guiFailedPathChecks++;
					#endif
					gubNPCAPBudget = 0;
					gubNPCDistLimit = 0;
					return(0);
				}

				iLocY = newLoc / MAPWIDTH;
				iLocX = newLoc % MAPWIDTH;
				SETLOC( *pNewPtr, newLoc );
				pNewPtr->usCostSoFar = (UINT16) newTotCost;
				pNewPtr->usCostToGo = (UINT16) REMAININGCOST(pNewPtr);
				if ( fCopyReachable )
				{
					pNewPtr->usCostToGo = 100;
				}
				else
				{
					pNewPtr->usCostToGo = (UINT16) REMAININGCOST(pNewPtr);
				}

				pNewPtr->usTotalCost = newTotCost + pNewPtr->usCostToGo;
				pNewPtr->ubLegDistance = LEGDISTANCE( iLocX, iLocY, iDestX, iDestY );

				if (gubNPCAPBudget)
				{
					//save the AP cost so far along this path
					pNewPtr->ubTotalAPCost = ubNewAPCost;
					// update the AP costs in the AI array of path costs if necessary...
					if (fCopyPathCosts)
					{
						iX = AI_PATHCOST_RADIUS + iLocX - iOriginationX;
						iY = AI_PATHCOST_RADIUS + iLocY - iOriginationY;
						gubAIPathCosts[iX][iY] = ubNewAPCost;
					}
				}

				//update the trail map to reflect the newer shorter path
				trailCost[newLoc] = (UINT16) newTotCost;
				trailCostUsed[newLoc] = gubGlobalPathCount;

				//do a sorted que insert of the new path
				// COMMENTED OUT TO DO BOUNDS CHECKER CC JAN 18 99
				//QUEINSERT(pNewPtr);
				//#define SkipListInsert( pNewPtr )
				{
					pCurr = pQueueHead;
					uiCost = TOTALCOST( pNewPtr );
					std::fill_n(pUpdate, MAX_SKIPLIST_LEVEL, nullptr);
					for (iCurrLevel = bSkipListLevel - 1; iCurrLevel >= 0; iCurrLevel--)
					{
						pNext = pCurr->pNext[iCurrLevel];
						while (pNext)
						{
							if ( uiCost > TOTALCOST( pNext ) || (uiCost == TOTALCOST( pNext ) && FARTHER( pNewPtr, pNext ) ) )
							{
								pCurr = pNext;
								pNext = pCurr->pNext[iCurrLevel];
							}
							else
							{
								break;
							}
						}
						pUpdate[iCurrLevel] = pCurr;
					}
					pCurr = pCurr->pNext[0];
					for (iCurrLevel = 0; iCurrLevel < pNewPtr->bLevel; iCurrLevel++)
					{
						if (!(pUpdate[iCurrLevel]))
						{
							break;
						}
						pNewPtr->pNext[iCurrLevel] = pUpdate[iCurrLevel]->pNext[iCurrLevel];
						pUpdate[iCurrLevel]->pNext[iCurrLevel] = pNewPtr;
					}
					iSkipListSize++;
					if (iSkipListSize > iSkipListLevelLimit[bSkipListLevel])
					{
						pCurr = pQueueHead;
						pNext = pQueueHead->pNext[bSkipListLevel - 1];
						while( pNext )
						{
							if (pNext->bLevel > bSkipListLevel)
							{
								pCurr->pNext[bSkipListLevel] = pNext;
								pCurr = pNext;
							}
							pNext = pNext->pNext[bSkipListLevel - 1];
						}
						pCurr->pNext[bSkipListLevel] = pNext;
						bSkipListLevel++;
					}
				}

#ifdef PATHAI_SKIPLIST_DEBUG
					// print out contents of queue
					{
						INT8  bTemp;
						ST::string zTempString;
						ST::string zTS;
						pCurr = pQueueHead;
						INT32 iLoop = 0;
						while( pCurr )
						{
							zTS = ST::format("\t{d}", pCurr->sPathNdx);
							if (pCurr == pNewPtr)
							{
								zTS += "*";
							}
							zTempString += zTS;
							pCurr = pCurr->pNext[0];
							iLoop++;
							if (iLoop > 50)
							{
								break;
							}
						}
						SLOGD(zTempString.c_str());


						zTempString = "";
						pCurr = pQueueHead;
						iLoop = 0;
						while( pCurr )
						{
							zTS = ST::format("\t{d}", pCurr->bLevel);
							zTempString += zTS;
							pCurr = pCurr->pNext[0];
							iLoop++;
							if (iLoop > 50)
							{
								break;
							}
						}
						SLOGD(zTempString.c_str());

						zTempString = "";
						bTemp = pQueueHead->bLevel;
						pCurr = pQueueHead;
						iLoop = 0;
						while( pCurr )
						{
							bTemp = pQueueHead->bLevel;
							while ( pCurr->pNext[ bTemp - 1 ] == NULL )
							{
								bTemp--;
							}
							zTS = ST::format("\t{d}", bTemp);
							zTempString += zTS;
							pCurr = pCurr->pNext[0];
							iLoop++;
							if (iLoop > 50)
							{
								break;
							}
						}
						SLOGD(zTempString.c_str());
						SLOGD("------" );
					}
#endif

			}

NEXTDIR:
			if (bLoopState == LOOPING_CLOCKWISE) // backwards
			{
				ubCnt = OneCCDirection(ubCnt);
			}
			else
			{
				ubCnt = OneCDirection(ubCnt);
			}
			if ( ubCnt == ubLoopEnd )
			{
ENDOFLOOP:
				break;
			}
			else if (fContinuousTurnNeeded && ubCnt == OppositeDirection(ubLoopStart))
			{
				fCheckedBehind = TRUE;
			}
		}
	}
	while (pathQNotEmpty && pathNotYetFound);


	#if defined( PATHAI_VISIBLE_DEBUG )
		if (gfDisplayCoverValues && gfDrawPathPoints)
		{
			SetRenderFlags( RENDER_FLAG_FULL );
			if ( guiCurrentScreen == GAME_SCREEN )
			{
				RenderWorld();
				RenderCoverDebug( );
				InvalidateScreen( );
				RefreshScreen();
			}
		}
	#endif


	// work finished. Did we find a path?
	if (pathQNotEmpty && pathFound)
	{
		INT16 z,_z,_nextLink; //,tempgrid;

		_z=0;
		z = (INT16) pQueueHead->pNext[0]->sPathNdx;

		while (z)
		{
			_nextLink = trailTree[z].nextLink;
			trailTree[z].nextLink = _z;
			_z = z;
			z = _nextLink;
		}

		// if this function was called because a solider is about to embark on an actual route
		// (as opposed to "test" path finding (used by cursor, etc), then grab all pertinent
		// data and copy into soldier's database
		if (bCopy == COPYROUTE)
		{
			z=_z;

			for (ubCnt=0; z && (ubCnt < MAX_PATH_LIST_SIZE); ubCnt++)
			{
				s->ubPathingData[ubCnt] = trailTree[z].stepDir;

				z = trailTree[z].nextLink;
			}

			s->ubPathIndex = 0;
			s->ubPathDataSize  = ubCnt;

		}
		else if (bCopy == NO_COPYROUTE)
		{

			z=_z;
			UINT16 iCnt;
			for (iCnt = 0; z != 0 && iCnt < lengthof(guiPathingData); iCnt++)
			{
				guiPathingData[ iCnt ] = trailTree[z].stepDir;

				z = trailTree[z].nextLink;
			}
			ubCnt = iCnt;
			giPathDataSize = ubCnt;

		}

		#if defined( PATHAI_VISIBLE_DEBUG )
		if (gfDisplayCoverValues && gfDrawPathPoints)
		{
			SetRenderFlags( RENDER_FLAG_FULL );
			RenderWorld();
			RenderCoverDebug( );
			InvalidateScreen( );
			RefreshScreen();
		}
		#endif


		// return path length : serves as a "successful" flag and a path length counter
		#ifdef COUNT_PATHS
		guiSuccessfulPathChecks++;
		#endif
		gubNPCAPBudget = 0;
		gubNPCDistLimit = 0;

		//TEMP:  This is returning zero when I am generating edgepoints, so I am force returning 1 until
		//       this is fixed?
		if( gfGeneratingMapEdgepoints )
		{
			return TRUE;
		}


		return(ubCnt);
	}

	#ifdef COUNT_PATHS
	if (fCopyReachable)
	{
		// actually this is a success
		guiSuccessfulPathChecks++;
	}
	else
	{
		guiUnsuccessfulPathChecks++;
	}
	#endif

	// failed miserably, report...
	gubNPCAPBudget = 0;
	gubNPCDistLimit = 0;
	return(0);
}

void GlobalReachableTest( INT16 sStartGridNo )
{
	SOLDIERTYPE s;

	s = SOLDIERTYPE{};
	s.sGridNo = sStartGridNo;
	s.bLevel = 0;
	s.bTeam = 1;

	//reset the flag for gridno's
	FOR_EACH_WORLD_TILE(i)
	{
		i->uiFlags &= ~MAPELEMENT_REACHABLE;
	}

	ReconfigurePathAI( ABSMAX_SKIPLIST_LEVEL, ABSMAX_TRAIL_TREE, ABSMAX_PATHQ );
	FindBestPath( &s, NOWHERE, 0, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE );
	RestorePathAIToDefaults();
}

void LocalReachableTest( INT16 sStartGridNo, INT8 bRadius )
{
	SOLDIERTYPE s;
	INT32 iCurrentGridNo = 0;
	INT32 iX, iY;

	s = SOLDIERTYPE{};
	s.sGridNo = sStartGridNo;

	//if we are moving on the gorund level
	if( gsInterfaceLevel == I_ROOF_LEVEL )
	{
		s.bLevel = 1;
	}
	else
	{
		s.bLevel = 0;
	}

	s.bTeam = OUR_TEAM;

	//reset the flag for gridno's
	for ( iY = -bRadius; iY <= bRadius; iY++ )
	{
		for ( iX = -bRadius; iX <= bRadius; iX++ )
		{
			iCurrentGridNo = sStartGridNo + iX + iY * MAXCOL;
			if (iCurrentGridNo >= 0 && iCurrentGridNo < WORLD_MAX)
			{
				gpWorldLevelData[ iCurrentGridNo ].uiFlags &= ~( MAPELEMENT_REACHABLE );
			}
		}
	}

	// set the dist limit
	gubNPCDistLimit = bRadius;
	// make the function call
	FindBestPath( &s, NOWHERE, s.bLevel, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE );
	// reset dist limit
	gubNPCDistLimit = 0;
}

void GlobalItemsReachableTest( INT16 sStartGridNo1, INT16 sStartGridNo2 )
{
	SOLDIERTYPE s;

	s = SOLDIERTYPE{};
	s.sGridNo = sStartGridNo1;
	s.bLevel = 0;
	s.bTeam = 1;

	//reset the flag for gridno's
	FOR_EACH_WORLD_TILE(i)
	{
		i->uiFlags &= ~MAPELEMENT_REACHABLE;
	}

	ReconfigurePathAI( ABSMAX_SKIPLIST_LEVEL, ABSMAX_TRAIL_TREE, ABSMAX_PATHQ );
	FindBestPath( &s, NOWHERE, 0, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE );
	if ( sStartGridNo2 != NOWHERE )
	{
		s.sGridNo = sStartGridNo2;
		FindBestPath( &s, NOWHERE, 0, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE );
	}
	RestorePathAIToDefaults();
}

void RoofReachableTest( INT16 sStartGridNo, UINT8 ubBuildingID )
{
	SOLDIERTYPE s;

	s = SOLDIERTYPE{};
	s.sGridNo = sStartGridNo;
	s.bLevel = 1;
	s.bTeam = 1;

	gubBuildingInfoToSet = ubBuildingID;

	ReconfigurePathAI( ABSMAX_SKIPLIST_LEVEL, ABSMAX_TRAIL_TREE, ABSMAX_PATHQ );
	FindBestPath( &s, NOWHERE, 1, WALKING, COPYREACHABLE, 0 );
	RestorePathAIToDefaults();

	// set start position to reachable since path code sets it unreachable
	gpWorldLevelData[ sStartGridNo ].uiFlags |= MAPELEMENT_REACHABLE;

	// reset building variable
	gubBuildingInfoToSet = 0;
}


void ErasePath()
{
	INT16 iCnt;

	// NOTE: This routine must be called BEFORE anything happens that changes
	//       a merc's gridno, else the....

	//EraseAPCursor();

	if ( gfUIHandleShowMoveGrid )
	{
		gfUIHandleShowMoveGrid = FALSE;

		RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS4);
		RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS9);
		RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS2);
		RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS13);
		RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS15);
		RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS19);
		RemoveTopmost(gsUIHandleShowMoveGridLocation, FIRSTPOINTERS20);
	}

	if (!gusPathShown)
	{
		//OldPath = FALSE;
		return;
	}


	//if (OldPath > 0 && !eraseOldOne)
	//   return;

	//OldPath = FALSE;



	gusPathShown = FALSE;


	for (iCnt=0; iCnt < giPlotCnt; iCnt++)
	{
		//Grid[PlottedPath[cnt]].fstep = 0;

		RemoveAllObjectsOfTypeRange( guiPlottedPath[iCnt], FOOTPRINTS, FOOTPRINTS );

		RemoveAllOnRoofsOfTypeRange( guiPlottedPath[iCnt], FOOTPRINTS, FOOTPRINTS );

		//RemoveAllObjectsOfTypeRange( guiPlottedPath[iCnt], FIRSTPOINTERS, FIRSTPOINTERS );
	}

	//for (cnt=0; cnt < GRIDSIZE; cnt++)
	//	Grid[cnt].fstep = 0;

	giPlotCnt = 0;
	std::fill_n(guiPlottedPath, 256, 0);
}


INT16 PlotPath(SOLDIERTYPE* const pSold, const INT16 sDestGridno, const INT8 bCopyRoute, const INT8 bPlot, const UINT16 usMovementMode, const INT16 sAPBudget)
{
	INT16 sTileCost,sPoints=0,sTempGrid,sAnimCost=0;
	INT16 sPointsWalk=0,sPointsCrawl=0,sPointsRun=0,sPointsSwat=0;
	INT16 sExtraCostStand,sExtraCostSwat,sExtraCostCrawl;
	INT32 iCnt;
	INT16 sFootOrderIndex;
	INT16 sSwitchValue;
	INT16 sFootOrder[5] = {
		GREENSTEPSTART,
		PURPLESTEPSTART,
		BLUESTEPSTART,
		ORANGESTEPSTART,
		REDSTEPSTART
	};
	UINT16 usTileNum;
	LEVELNODE *pNode;
	UINT16 usMovementModeToUseForAPs;
	BOOLEAN bIgnoreNextCost = FALSE;
	INT16  sTestGridno;

	if ( bPlot && gusPathShown )
	{
		ErasePath();
	}

	gusAPtsToMove = 0;
	sTempGrid = (INT16) pSold->sGridNo;

	sFootOrderIndex = 0;


	//gubNPCMovementMode = (UINT8) usMovementMode;
	// distance limit to reduce the cost of plotting a path to a location we can't reach

	// For now, use known hight adjustment
	if (gfRecalculatingExistingPathCost ||
		FindBestPath(pSold, sDestGridno, (INT8)pSold->bLevel, usMovementMode, bCopyRoute, 0 ))
	{
		// if soldier would be STARTING to run then he pays a penalty since it takes time to
		// run full speed
		if (pSold->usAnimState != RUNNING)
		{
			// for estimation purposes, always pay penalty
			sPointsRun = AP_START_RUN_COST;
		}

		// Add to points, those needed to start from different stance!
		sPoints += MinAPsToStartMovement( pSold, usMovementMode );


		// We should reduce points for starting to run if first tile is a fence...
		sTestGridno  = NewGridNo(pSold->sGridNo, DirectionInc( guiPathingData[0]));
		if ( gubWorldMovementCosts[ sTestGridno ][ guiPathingData[0] ][ pSold->bLevel] == TRAVELCOST_FENCE )
		{
			if ( usMovementMode == RUNNING && pSold->usAnimState != RUNNING )
			{
				sPoints -= AP_START_RUN_COST;
			}
		}

		// FIRST, add up "startup" additional costs - such as intermediate animations, etc.
		//switch(pSold->usAnimState)
		//{
		//	case START_AID:
		//	case GIVING_AID:
		//		sAnimCost = AP_STOP_FIRST_AID;
		//		break;
		//	case TWISTOMACH:
		//	case COLLAPSED:
		//		sAnimCost = AP_GET_UP;
		//		break;
		//	case TWISTBACK:
		//	case UNCONSCIOUS:
		//		sAnimCost = (AP_ROLL_OVER+AP_GET_UP);
		//		break;
		//	case CROUCHING:
		//		if (usMovementMode == WALKING || usMovementMode == RUNNING)
		//			sAnimCost = AP_CROUCH;
		//		break;
		//}


		sPoints += sAnimCost;
		gusAPtsToMove += sAnimCost;

		const INT32 iLastGrid = giPathDataSize;
		for ( iCnt=0; iCnt < iLastGrid; iCnt++ )
		{
			sExtraCostStand = 0;
			sExtraCostSwat = 0;
			sExtraCostCrawl = 0;

			sTempGrid  = NewGridNo(sTempGrid, DirectionInc( guiPathingData[iCnt]));

			// Get switch value...
			sSwitchValue = gubWorldMovementCosts[ sTempGrid ][ guiPathingData[iCnt] ][ pSold->bLevel];

			// get the tile cost for that tile based on WALKING
			sTileCost = TerrainActionPoints( pSold, sTempGrid, guiPathingData[iCnt], pSold->bLevel );

			usMovementModeToUseForAPs = usMovementMode;

			// ATE - MAKE MOVEMENT ALWAYS WALK IF IN WATER
			if (gpWorldLevelData[sTempGrid].ubTerrainID == DEEP_WATER ||
				gpWorldLevelData[sTempGrid].ubTerrainID == MED_WATER ||
				gpWorldLevelData[sTempGrid].ubTerrainID == LOW_WATER)
			{
				usMovementModeToUseForAPs = WALKING;
			}

			if ( bIgnoreNextCost )
			{
				bIgnoreNextCost = FALSE;
			}
			else
			{
				// ATE: If we have a 'special cost, like jump fence...
				if ( sSwitchValue == TRAVELCOST_FENCE )
				{
					sPoints += sTileCost;

					bIgnoreNextCost = TRUE;

					// If we are changeing stance ( either before or after getting there....
					// We need to reflect that...
					switch( usMovementModeToUseForAPs )
					{
						case RUNNING:
						case WALKING:
							// Add here cost to go from crouch to stand AFTER fence hop....
							// Since it's AFTER.. make sure we will be moving after jump...
							if ( ( iCnt + 2 ) < iLastGrid )
							{
								sExtraCostStand += AP_CROUCH;

								// ATE: if running, charge extra point to srart again
								if ( usMovementModeToUseForAPs== RUNNING )
								{
									sExtraCostStand++;
								}

								sPoints += sExtraCostStand;
							}
							break;

						case SWATTING:
							// Add cost to stand once there BEFORE....
							sExtraCostSwat += AP_CROUCH;
							sPoints += sExtraCostSwat;
							break;

						case CRAWLING:
							// Can't do it here.....
							break;

				}
			}
			else if (sTileCost > 0)
			{
				// else, movement is adjusted based on mode...

				if (sSwitchValue == TRAVELCOST_NOT_STANDING)
				{
					switch( usMovementModeToUseForAPs )
					{
						case RUNNING:
						case WALKING :
							// charge crouch APs for ducking head!
							sExtraCostStand += AP_CROUCH;
							break;

						default:
							break;
					}
				}

				// so, then we must modify it for other movement styles and accumulate
				switch( usMovementModeToUseForAPs )
				{
					case RUNNING:
						sPoints += (INT16)((DOUBLE)(sTileCost) / RUNDIVISOR) + sExtraCostStand;	break;
					case WALKING :
						sPoints += (sTileCost + WALKCOST) + sExtraCostStand;
						break;
					case SWATTING:
						sPoints += (sTileCost + SWATCOST) + sExtraCostSwat;
						break;
					case CRAWLING:
						sPoints += (sTileCost + CRAWLCOST) + sExtraCostCrawl;
						break;
					default:
						sPoints += sTileCost;
						break;
				}
			}
		}

			// THIS NEXT SECTION ONLY NEEDS TO HAPPEN FOR CURSOR UI FEEDBACK, NOT ACTUAL COSTING

			if (bPlot && (gTacticalStatus.uiFlags & INCOMBAT)) // OR USER OPTION ON... ***)
			{
				// ATE; TODO: Put stuff in here to allow for fact of costs other than movement ( jump fence, open door )

				// store WALK cost
				sPointsWalk += (sTileCost + WALKCOST) + sExtraCostStand;

				// now get cost as if CRAWLING
				sPointsCrawl += (sTileCost + CRAWLCOST) + sExtraCostCrawl;

				// now get cost as if SWATTING
				sPointsSwat += (sTileCost + SWATCOST) + sExtraCostSwat;

				// now get cost as if RUNNING
				sPointsRun += (INT16)((DOUBLE)(sTileCost) / RUNDIVISOR) + sExtraCostStand;
			}

			if ( iCnt == 0 && bPlot )
			{
				gusAPtsToMove = sPoints;

				giPlotCnt = 0;

			}


			//if (gTacticalStatus.uiFlags & INCOMBAT) // OR USER OPTION "show paths" ON... ***
			{
				if (bPlot && iCnt < iLastGrid - 1 && giPlotCnt < static_cast<INT32>(lengthof(guiPlottedPath)))
				{
					guiPlottedPath[giPlotCnt++] = sTempGrid;

					// we need a footstep graphic ENTERING the next tile

					// get the direction
					usTileNum = (UINT16) guiPathingData[iCnt] + 2;
					if (usTileNum > 8)
					{
						usTileNum = 1;
					}

					// Are we a vehicle?
					if ( pSold->uiStatusFlags & SOLDIER_VEHICLE )
					{
						// did we exceed WALK cost?
						if ( sPointsSwat > sAPBudget)
						{
							sFootOrderIndex = 4;
						}
						else
						{
							sFootOrderIndex = 3;
						}
					}
					else
					{
						// did we exceed CRAWL cost?
						if (sFootOrderIndex == 0 && sPointsCrawl > sAPBudget)
						{
							sFootOrderIndex++;
						}

						// did we exceed WALK cost?
						if (sFootOrderIndex == 1 && sPointsSwat > sAPBudget)
						{
							sFootOrderIndex++;
						}

						// did we exceed SWAT cost?
						if (sFootOrderIndex == 2 && sPointsWalk > sAPBudget)
						{
							sFootOrderIndex++;
						}

						// did we exceed RUN cost?
						if (sFootOrderIndex == 3 && sPointsRun > sAPBudget)
						{
							sFootOrderIndex++;
						}
					}

					UINT16 usTileIndex;
					usTileIndex = GetTileIndexFromTypeSubIndex(FOOTPRINTS, usTileNum);

					// Adjust based on what mode we are in...
					if (!(gTacticalStatus.uiFlags & INCOMBAT))
					{
						// find out which color we're using
						usTileIndex += sFootOrder[ 4 ];
					}
					else // turn based
					{
						// find out which color we're using
						usTileIndex += sFootOrder[sFootOrderIndex];
					}


					/*
					if ( sPoints <= sAPBudget)
					{
						// find out which color we're using
						usTileIndex += sFootOrder[sFootOrderIndex];
					}
					else
					{
						// use red footprints ( offset by 16 )
						usTileIndex += REDSTEPSTART;
					}*/

					if ( pSold->bLevel == 0 )
					{
						pNode = AddObjectToTail(sTempGrid, usTileIndex );
						pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					}
					else
					{
						pNode = AddOnRoofToTail(sTempGrid, usTileIndex );
						pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					}



					// we need a footstep graphic LEAVING this tile

					// get the direction using the NEXT tile (thus iCnt+1 as index)
					usTileNum = (UINT16) guiPathingData[iCnt + 1] + 2;
					if (usTileNum > 8)
					{
						usTileNum = 1;
					}


					// this is a LEAVING footstep which is always the second set of 8
					usTileNum += 8;

					usTileIndex = GetTileIndexFromTypeSubIndex(FOOTPRINTS, usTileNum);

					// Adjust based on what mode we are in...
					if (!(gTacticalStatus.uiFlags & INCOMBAT))
					{
						// find out which color we're using
						usTileIndex += sFootOrder[ 4 ];
					}
					else // turnbased
					{
						// find out which color we're using
						usTileIndex += sFootOrder[sFootOrderIndex];
					}



					if ( pSold->bLevel == 0 )
					{
						pNode = AddObjectToTail(sTempGrid, usTileIndex );
						pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					}
					else
					{
						pNode = AddOnRoofToTail(sTempGrid, usTileIndex );
						pNode->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						pNode->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					}
				}

			} // end of if turn based or real-time user option "show paths" on...
		}

		if ( bPlot )
		{
			gusPathShown = TRUE;
		}

	} // end of found a path

	// reset distance limit
	gubNPCDistLimit = 0;

	return(sPoints);
}


INT16 UIPlotPath(SOLDIERTYPE* const pSold, const INT16 sDestGridno, const INT8 bCopyRoute, INT8 bPlot, const UINT16 usMovementMode, const INT16 sAPBudget)
{
	// This function is specifically for UI calls to the pathing routine, to
	// check whether the shift key is pressed, etc.

	if ( _KeyDown( SHIFT ) )
	{
		gfPlotDirectPath = TRUE;
	}

	// If we are on the same level as the interface level, continue, else return
	if ( pSold->bLevel != gsInterfaceLevel )
	{
		return( 0 );
	}

	if ( gGameSettings.fOptions[ TOPTION_ALWAYS_SHOW_MOVEMENT_PATH ] )
	{
		bPlot = TRUE;
	}

	const INT16 sRet = PlotPath(pSold, sDestGridno, bCopyRoute, bPlot, usMovementMode, sAPBudget);
	gfPlotDirectPath = FALSE;
	return( sRet );
}

INT16 RecalculatePathCost( SOLDIERTYPE *pSoldier, UINT16 usMovementMode )
{
	// AI function for a soldier already with a path; this will return the cost of that
	// path using the given movement mode

	if ( !pSoldier->bPathStored || pSoldier->ubPathDataSize == 0 )
	{
		return( 0 );
	}

	gfRecalculatingExistingPathCost = TRUE;
	const INT16 sRet = PlotPath(pSoldier, pSoldier->sFinalDestination, NO_COPYROUTE, FALSE, usMovementMode, 0);
	gfRecalculatingExistingPathCost = FALSE;
	return( sRet );
}


INT16 EstimatePlotPath(SOLDIERTYPE* const pSold, const INT16 sDestGridno, const INT8 bCopyRoute, const INT8 bPlot, const UINT16 usMovementMode, const INT16 sAPBudget)
{
	// This function is specifically for AI calls to estimate path cost to a location
	// It sets stuff up to ignore all people

	gfEstimatePath = TRUE;

	const INT16 sRet = PlotPath(pSold, sDestGridno, bCopyRoute, bPlot, usMovementMode, sAPBudget);

	gfEstimatePath = FALSE;

	return( sRet );
}


UINT8 InternalDoorTravelCost(const SOLDIERTYPE* pSoldier, INT32 iGridNo, UINT8 ubMovementCost, BOOLEAN fReturnPerceivedValue, INT32* piDoorGridNo, BOOLEAN fReturnDoorCost)
{	
	// This function will return either TRAVELCOST_DOOR (in place of closed door cost),
	// TRAVELCOST_OBSTACLE, or the base ground terrain
	// travel cost, depending on whether or not the door is open or closed etc.
	BOOLEAN fDoorIsObstacleIfClosed=FALSE;
	INT32 iDoorGridNo=-1;
	INT32 iDoor2GridNo = -1;
	DOOR *pDoor;
	BOOLEAN fDoorIsOpen{};
	StructureFlags  doorState{};
	StructureFlags  door2State{};
	UINT8 ubReplacementCost;

	if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
	{
		ubReplacementCost = ubMovementCost;

		fDoorIsObstacleIfClosed = IsDoorObstacleIfClosed(ubMovementCost, iGridNo, &iDoorGridNo, &iDoor2GridNo);		

		if (fDoorIsObstacleIfClosed)
		{
			ubReplacementCost = TRAVELCOST_DOOR;
		}

		if ( pSoldier && (pSoldier->uiStatusFlags & SOLDIER_MONSTER || pSoldier->uiStatusFlags & SOLDIER_ANIMAL) )
		{
			// can't open doors!
			ubReplacementCost = TRAVELCOST_OBSTACLE;
		}

		if (piDoorGridNo)
		{
			// return gridno of door through pointer
			*piDoorGridNo = iDoorGridNo;
		}

		doorState = GetDoorState(iDoorGridNo, fReturnPerceivedValue);
		
		if (iDoor2GridNo == -1)
		{ // this travel direction depends on the state of only one door structure
			if ( !(doorState & STRUCTURE_ANYDOOR) )
			{ // door destroyed?
				return ubMovementCost;
			}
			fDoorIsOpen = doorState & STRUCTURE_OPEN;
		}
		else
		{ // ... or two door structures
			door2State = GetDoorState(iDoor2GridNo, fReturnPerceivedValue);
			if ( !(doorState & STRUCTURE_ANYDOOR) && !(door2State & STRUCTURE_ANYDOOR) )
			{ // both doors destroyed?
				return ubMovementCost;
			}
			else if ( !(doorState & STRUCTURE_OPEN) || !(door2State & STRUCTURE_OPEN) )
			{ // this direction is always diagonal so block it if one of the doors is closed 
				return ubReplacementCost;
			}
			fDoorIsOpen = TRUE;
		}
		// now determine movement cost
		if (fDoorIsOpen)
		{
			if (fDoorIsObstacleIfClosed)
			{
				ubMovementCost = gTileTypeMovementCost[ gpWorldLevelData[ iGridNo ].ubTerrainID ];
			}
			else
			{
				ubMovementCost = ubReplacementCost;
			}
		}
		else
		{
			if (fDoorIsObstacleIfClosed)
			{
				// door is closed and this should be an obstacle, EXCEPT if we are calculating
				// a path for an enemy or NPC with keys

				// creatures and animals can't open doors!
				if (fReturnPerceivedValue ||
					(pSoldier && (pSoldier->uiStatusFlags & SOLDIER_MONSTER ||
					pSoldier->uiStatusFlags & SOLDIER_ANIMAL)))
				{
					ubMovementCost = ubReplacementCost;
				}
				else
				{
					// have to check if door is locked and NPC does not have keys!
					pDoor = FindDoorInfoAtGridNo( iDoorGridNo );
					if ( pDoor )
					{
						if ( ( !pDoor->fLocked || (pSoldier && pSoldier->bHasKeys) ) && !fReturnDoorCost )
						{
							ubMovementCost = gTileTypeMovementCost[ gpWorldLevelData[ iGridNo ].ubTerrainID ];
						}
						else
						{
							ubMovementCost = ubReplacementCost;
						}
					}
					else
					{
						ubMovementCost = ubReplacementCost;
					}
				}
			}
			else
			{
				ubMovementCost = gTileTypeMovementCost[ gpWorldLevelData[ iGridNo ].ubTerrainID ];
			}
		}

	}
	return( ubMovementCost );
}


UINT8 DoorTravelCost(const SOLDIERTYPE* pSoldier, INT32 iGridNo, UINT8 ubMovementCost, BOOLEAN fReturnPerceivedValue, INT32* piDoorGridNo)
{
	return( InternalDoorTravelCost( pSoldier, iGridNo, ubMovementCost, fReturnPerceivedValue, piDoorGridNo, FALSE ) );
}

BOOLEAN IsDoorObstacleIfClosed(UINT8 ubMovementCost, INT32 iGridNo, INT32* iDoorGridNo, INT32* iDoorGridNo2)
{
	*iDoorGridNo2 = -1;
	switch (ubMovementCost)
	{
	case TRAVELCOST_DOORS_CLOSED_W_SW:
		*iDoorGridNo = iGridNo + DirIncrementer[WEST];
		*iDoorGridNo2 = iGridNo + DirIncrementer[SOUTHWEST];
		return(TRUE);
		break;
	case TRAVELCOST_DOORS_CLOSED_HERE_S:
		*iDoorGridNo = iGridNo;
		*iDoorGridNo2 = iGridNo + DirIncrementer[SOUTH];
		return(TRUE);
		break;
	case TRAVELCOST_DOORS_CLOSED_N_NW:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTH];
		*iDoorGridNo2 = iGridNo + DirIncrementer[NORTHWEST];
		return(TRUE);
		break;
	case TRAVELCOST_DOORS_CLOSED_HERE_W:
		*iDoorGridNo = iGridNo;
		*iDoorGridNo2 = iGridNo + DirIncrementer[WEST];
		return(TRUE);
		break;
	case TRAVELCOST_DOORS_CLOSED_W_NW:
		*iDoorGridNo = iGridNo + DirIncrementer[WEST];
		*iDoorGridNo2 = iGridNo + DirIncrementer[NORTHWEST];
		return(TRUE);
		break;
	case TRAVELCOST_DOORS_CLOSED_HERE_N:
		*iDoorGridNo = iGridNo;
		*iDoorGridNo2 = iGridNo + DirIncrementer[NORTH];
		return(TRUE);
		break;
	case TRAVELCOST_DOORS_CLOSED_N_NE:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTH];
		*iDoorGridNo2 = iGridNo + DirIncrementer[NORTHEAST];
		return(TRUE);
		break;
	case TRAVELCOST_DOORS_CLOSED_HERE_E:
		*iDoorGridNo = iGridNo;
		*iDoorGridNo2 = iGridNo + DirIncrementer[EAST];
		return(TRUE);
		break;
	case TRAVELCOST_DOOR_CLOSED_HERE:
		*iDoorGridNo = iGridNo;
		return(TRUE);
		break;
	case TRAVELCOST_DOOR_CLOSED_N:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTH];
		return(TRUE);
		break;
	case TRAVELCOST_DOOR_CLOSED_W:
		*iDoorGridNo = iGridNo + DirIncrementer[WEST];
		return(TRUE);
		break;
	case TRAVELCOST_DOOR_OPEN_HERE:
		*iDoorGridNo = iGridNo;
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_N:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTH];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_NE:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTHEAST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_E:
		*iDoorGridNo = iGridNo + DirIncrementer[EAST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_SE:
		*iDoorGridNo = iGridNo + DirIncrementer[SOUTHEAST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_S:
		*iDoorGridNo = iGridNo + DirIncrementer[SOUTH];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_SW:
		*iDoorGridNo = iGridNo + DirIncrementer[SOUTHWEST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_W:
		*iDoorGridNo = iGridNo + DirIncrementer[WEST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_NW:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTHWEST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_N_N:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTH] + DirIncrementer[NORTH];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_NW_N:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTHWEST] + DirIncrementer[NORTH];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_NE_N:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTHEAST] + DirIncrementer[NORTH];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_W_W:
		*iDoorGridNo = iGridNo + DirIncrementer[WEST] + DirIncrementer[WEST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_SW_W:
		*iDoorGridNo = iGridNo + DirIncrementer[SOUTHWEST] + DirIncrementer[WEST];
		return(FALSE);
		break;
	case TRAVELCOST_DOOR_OPEN_NW_W:
		*iDoorGridNo = iGridNo + DirIncrementer[NORTHWEST] + DirIncrementer[WEST];
		return(FALSE);
		break;
	default:
		return(FALSE);
		break;
	}
}

StructureFlags GetDoorState(int32_t iDoorGridNo, bool returnPerceivedValue)
{
	StructureFlags doorState{};
	if (returnPerceivedValue && gpWorldLevelData[iDoorGridNo].ubExtFlags[0] & MAPELEMENT_EXT_DOOR_STATUS_PRESENT) {
		DOOR_STATUS* pDoorStatus{ GetDoorStatus( static_cast<GridNo>(iDoorGridNo) ) };
		if (pDoorStatus) {
			doorState |= STRUCTURE_DOOR;
			if (pDoorStatus->ubFlags & DOOR_PERCEIVED_OPEN) {
				doorState |= STRUCTURE_OPEN;
			}
			return doorState;
		}
	}
	else {
		STRUCTURE* pDoorStructure{ FindStructure( static_cast<GridNo>(iDoorGridNo), STRUCTURE_ANYDOOR) };
		if (pDoorStructure) {
			doorState = static_cast<StructureFlags>(pDoorStructure->fFlags);
			return doorState;
		}
	}
	// door destroyed?
	doorState |= STRUCTURE_OPEN;
	doorState &= ~STRUCTURE_ANYDOOR;
	return doorState;
}
