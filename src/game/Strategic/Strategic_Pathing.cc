#include "Strategic_Movement_Costs.h"
#include "Types.h"
#include "WorldDef.h"
#include "MapScreen.h"
#include "Overhead.h"
#include "StrategicMap.h"
#include "Strategic_Pathing.h"
#include "Map_Screen_Interface_Border.h"
#include "Game_Clock.h"
#include "Strategic_Movement.h"
#include "Campaign_Types.h"
#include "Assignments.h"
#include "Squads.h"
#include "Vehicles.h"
#include "Map_Screen_Helicopter.h"
#include "Input.h"
#include "English.h"
#include "Game_Event_Hook.h"
#include "Strategic_AI.h"
#include "Debug.h"
#include "MemMan.h"

#include <algorithm>
#include <iterator>

static UINT16  gusMapPathingData[256];
static BOOLEAN gfPlotToAvoidPlayerInfuencedSectors = FALSE;


// Globals
struct path_t
{
	INT16 nextLink;           //2
	INT16 prevLink;           //2
	INT16 location;           //2
	INT16 pathNdx;            //2
	INT32 costSoFar;          //4
	INT32 costToGo;           //4
};

struct trail_t
{
	short nextLink;
	short diStratDelta;
};

#define MAXTRAILTREE	(4096)
#define MAXpathQ			(512)
#define MAP_WIDTH 18
#define MAP_LENGTH MAP_WIDTH*MAP_WIDTH

//#define EASYWATERCOST	TRAVELCOST_FLAT / 2
//#define ISWATER(t)	(((t)==TRAVELCOST_KNEEDEEP) || ((t)==TRAVELCOST_DEEPWATER))
//#define NOPASS (TRAVELCOST_OBSTACLE)
//#define VEINCOST TRAVELCOST_FLAT     //actual cost for bridges and doors and such
#define TRAILCELLTYPE UINT32
#define TRAILCELLMAX 0xFFFFFFFF

static path_t        pathQB[MAXpathQ];
static TRAILCELLTYPE trailCostB[MAP_LENGTH];
static trail_t       trailStratTreeB[MAXTRAILTREE];
static short         trailStratTreedxB = 0;

#define QHEADNDX (0)
#define QPOOLNDX (MAXpathQ-1)

#define pathQNotEmpty (pathQB[QHEADNDX].nextLink!=QHEADNDX)
#define pathFound (pathQB[ pathQB[QHEADNDX].nextLink ].location == sDestination)
#define pathNotYetFound (!pathFound)

#define REMQUENODE(ndx)                                           \
	do                                                              \
	{                                                               \
		pathQB[pathQB[ndx].prevLink].nextLink = pathQB[ndx].nextLink; \
		pathQB[pathQB[ndx].nextLink].prevLink = pathQB[ndx].prevLink; \
	}                                                               \
	while (0)

#define INSQUENODEPREV(newNode, curNode)                                  \
	do                                                                      \
	{                                                                       \
		pathQB[newNode].nextLink                  = curNode;                  \
		pathQB[newNode].prevLink                  = pathQB[curNode].prevLink; \
		pathQB[pathQB[curNode].prevLink].nextLink = newNode;                  \
		pathQB[curNode].prevLink                  = newNode;                  \
	}                                                                       \
	while (0)

#define INSQUENODE(newNode,curNode)				\
{	pathQB[ newNode ].prevLink = curNode;			\
	pathQB[ newNode ].NextLink = pathQB[ curNode ].nextLink;	\
	pathQB[ pathQB[curNode].nextLink ].prevLink = newNode;	\
	pathQB[ curNode ].nextLink = newNode;			\
}

#define DELQUENODE(ndx)            \
	do                               \
	{                                \
		REMQUENODE(ndx);               \
		INSQUENODEPREV(ndx, QPOOLNDX); \
		pathQB[ndx].location = -1;     \
	}                                \
	while (0)

#define NEWQUENODE                         \
	do                                       \
	{                                        \
		if (queRequests < QPOOLNDX)            \
		{                                      \
			qNewNdx = queRequests++;             \
		}                                      \
		else                                   \
		{                                      \
			qNewNdx = pathQB[QPOOLNDX].nextLink; \
			REMQUENODE(qNewNdx);                 \
		}                                      \
	}                                        \
	while (0)

#define ESTIMATE0	((dx>dy) ?       (dx)      :       (dy))
#define ESTIMATE1	((dx<dy) ? ((dx*14)/10+dy) : ((dy*14)/10+dx) )
#define ESTIMATE2	FLATCOST*( (dx<dy) ? ((dx*14)/10+dy) : ((dy*14)/10+dx) )
#define ESTIMATEn	((int)(FLATCOST*sqrt(dx*dx+dy*dy)))
#define ESTIMATE ESTIMATE1


#define REMAININGCOST(ndx)					\
(								\
	(locY = pathQB[ndx].location/MAP_WIDTH),			\
	(locX = pathQB[ndx].location%MAP_WIDTH),			\
	(dy = ABS(iDestY-locY)),					\
	(dx = ABS(iDestX-locX)),					\
	ESTIMATE						\
)


#define MAXCOST (99900)
#define TOTALCOST(ndx) (pathQB[ndx].costSoFar + pathQB[ndx].costToGo)
#define XLOC(a) (a%MAP_WIDTH)
#define YLOC(a) (a/MAP_WIDTH)
#define LEGDISTANCE(a,b) ( ABS( XLOC(b)-XLOC(a) ) + ABS( YLOC(b)-YLOC(a) ) )
#define FARTHER(ndx,NDX) ( LEGDISTANCE(pathQB[ndx].location,sDestination) > LEGDISTANCE(pathQB[NDX].location,sDestination) )

#define FLAT_STRATEGIC_TRAVEL_TIME		60

#define QUESEARCH(ndx,NDX)                                    \
	do                                                          \
	{                                                           \
		INT32 k = TOTALCOST(ndx);                                 \
		NDX = pathQB[QHEADNDX].nextLink;                          \
		while(NDX && (k > TOTALCOST(NDX)))                        \
		{                                                         \
			NDX = pathQB[NDX].nextLink;                             \
		}                                                         \
		while (NDX && (k == TOTALCOST(NDX)) && FARTHER(ndx, NDX)) \
		{                                                         \
			NDX = pathQB[NDX].nextLink;                             \
		}                                                         \
	}                                                           \
	while (0)


static INT32 queRequests;


static INT16 const diStratDelta[]=
{
	-MAP_WIDTH,        //N
	1-MAP_WIDTH,       //NE
	1,                //E
	1+MAP_WIDTH,       //SE
	MAP_WIDTH,         //S
	MAP_WIDTH-1,       //SW
	-1,               //W
	-MAP_WIDTH-1       //NW
};


// this will find if a shortest strategic path

INT32 FindStratPath(INT16 const sStart, INT16 const sDestination, GROUP const& g, BOOLEAN const fTacticalTraversal)
{
	INT32 iCnt,ndx,insertNdx,qNewNdx;
	INT32 iDestX,iDestY,locX,locY,dx,dy;
	UINT16	newLoc,curLoc;
	TRAILCELLTYPE curCost,newTotCost,nextCost;
	INT16 sOrigination;
	BOOLEAN fPlotDirectPath = FALSE;
	static BOOLEAN fPreviousPlotDirectPath = FALSE;		// don't save

	// ******** Fudge by Bret (for now), curAPcost is never initialized in this function, but should be!
	// so this is just to keep things happy!

	// for player groups only!
	if (g.fPlayer)
	{
		// if player is holding down SHIFT key, find the shortest route instead of the quickest route!
		if ( _KeyDown( SHIFT ) )
		{
			fPlotDirectPath = TRUE;
		}


		if ( fPlotDirectPath != fPreviousPlotDirectPath )
		{
			// must redraw map to erase the previous path...
			fMapPanelDirty = TRUE;
			fPreviousPlotDirectPath = fPlotDirectPath;
		}
	}


	queRequests = 2;

	//initialize the ai data structures
	std::fill(std::begin(trailStratTreeB), std::end(trailStratTreeB), trail_t{});
	std::fill(std::begin(trailCostB), std::end(trailCostB), TRAILCELLMAX);

	std::fill(std::begin(pathQB), std::end(pathQB), path_t{});

	// FOLLOWING LINE COMMENTED OUT ON MARCH 7/97 BY IC
	std::fill(std::begin(gusMapPathingData), std::end(gusMapPathingData), ((UINT16)sStart));
	trailStratTreedxB=0;

	//set up common info
	sOrigination = sStart;


	iDestY = (sDestination / MAP_WIDTH);
	iDestX = (sDestination % MAP_WIDTH);


	// if origin and dest is water, then user wants to stay in water!
	// so, check and set waterToWater flag accordingly



	//setup Q
	pathQB[QHEADNDX].location	= sOrigination;
	pathQB[QHEADNDX].nextLink	= 1;
	pathQB[QHEADNDX].prevLink	= 1;
	pathQB[QHEADNDX].costSoFar	= MAXCOST;

	pathQB[QPOOLNDX].nextLink	= QPOOLNDX;
	pathQB[QPOOLNDX].prevLink	= QPOOLNDX;

	//setup first path record
	pathQB[1].nextLink = QHEADNDX;
	pathQB[1].prevLink = QHEADNDX;
	pathQB[1].location = sOrigination;
	pathQB[1].pathNdx	= 0;
	pathQB[1].costSoFar= 0;
	pathQB[1].costToGo	= REMAININGCOST(1);


	trailStratTreedxB					=0;
	trailCostB[sOrigination]=0;
	ndx										= pathQB[QHEADNDX].nextLink;
	pathQB[ndx].pathNdx		= trailStratTreedxB;
	trailStratTreedxB++;

	const GROUP* const heli_group = iHelicopterVehicleId != -1 ?
		GetGroup(GetHelicopter().ubMovementGroup) : 0;

	do
	{
		//remove the first and best path so far from the que
		ndx				= pathQB[QHEADNDX].nextLink;
		curLoc		= pathQB[ndx].location;
		curCost		= pathQB[ndx].costSoFar;
		DELQUENODE( (INT16)ndx );

		if (trailCostB[curLoc] < curCost)
			continue;


		//contemplate a new path in each direction
		for (iCnt=0; iCnt < 8; iCnt+=2)
		{
			newLoc = curLoc + diStratDelta[iCnt];


				// are we going off the map?
			if( ( newLoc % MAP_WORLD_X == 0 )||( newLoc%MAP_WORLD_X == MAP_WORLD_X -1 ) || ( newLoc / MAP_WORLD_X == 0 ) || ( newLoc / MAP_WORLD_X == MAP_WORLD_X - 1 ) )
			{
				// yeppers
				continue;
			}

			if( gfPlotToAvoidPlayerInfuencedSectors && newLoc != sDestination )
			{
				SGPSector sSector(newLoc % MAP_WORLD_X, newLoc / MAP_WORLD_X);

				if (IsThereASoldierInThisSector(sSector))
				{
					continue;
				}
				if (GetNumberOfMilitiaInSector(sSector))
				{
					continue;
				}
				if (!OkayForEnemyToMoveThroughSector(sSector.AsByte()))
				{
					continue;
				}
			}

			// are we plotting path or checking for existance of one?
			nextCost = GetSectorMvtTimeForGroup(SECTOR(curLoc % MAP_WORLD_X, curLoc / MAP_WORLD_X), iCnt / 2, &g);
			if (nextCost == TRAVERSE_TIME_IMPOSSIBLE) continue;

			if (&g == heli_group)
			{
				// is a heli, its pathing is determined not by time (it's always the same) but by total cost
				// Skyrider will avoid uncontrolled airspace as much as possible...
				if (StrategicMap[curLoc].fEnemyAirControlled)
				{
					nextCost = COST_AIRSPACE_UNSAFE;
				}
				else
				{
					nextCost = COST_AIRSPACE_SAFE;
				}
			}

			// if we're building this path due to a tactical traversal exit, we have to force the path to the next sector be
			// in the same direction as the traversal, even if it's not the shortest route, otherwise pathing can crash!  This
			// can happen in places where the long way around to next sector is actually shorter: e.g. D5 to D6.  ARM
			if ( fTacticalTraversal )
			{
				// if it's the first sector only (no cost yet)
				if( curCost == 0 && ( newLoc == sDestination ) )
				{
					if( GetTraversability( ( INT16 )( SECTOR( curLoc % 18, curLoc / 18 ) ), ( INT16 ) ( SECTOR( newLoc %18,  newLoc / 18 ) ) ) != GROUNDBARRIER )
					{
						nextCost = 0;
					}
				}
			}
			else
			{
				if ( fPlotDirectPath )
				{
					// use shortest route instead of faster route
					nextCost = FLAT_STRATEGIC_TRAVEL_TIME;
				}
			}

			/*
			// Commented out by CJC Feb 4 1999... causing errors!

			//make the destination look very attractive
			if( ( newLoc == sDestination ) )
			{
				if( GetTraversability( ( INT16 )( SECTOR( curLoc % 18, curLoc / 18 ) ), ( INT16 ) ( SECTOR( newLoc %18,  newLoc / 18 ) ) ) != GROUNDBARRIER )
				{
					nextCost = 0;
				}
			}
			*/
			newTotCost = curCost + nextCost;
			if (newTotCost < trailCostB[newLoc])
			{
				NEWQUENODE;

				if (qNewNdx == QHEADNDX)
				{
					return(0);
				}


				if (qNewNdx == QPOOLNDX)
				{
					return(0);
				}

				//make new path to current location
				trailStratTreeB[trailStratTreedxB].nextLink	= pathQB[ndx].pathNdx;
				trailStratTreeB[trailStratTreedxB].diStratDelta	= (INT16) iCnt;
				pathQB[qNewNdx].pathNdx						= trailStratTreedxB;
				trailStratTreedxB++;


				if (trailStratTreedxB >= MAXTRAILTREE)
				{
					return(0);
				}

				pathQB[qNewNdx].location		= (INT16) newLoc;
				pathQB[qNewNdx].costSoFar	= newTotCost;
				pathQB[qNewNdx].costToGo		= REMAININGCOST(qNewNdx);
				trailCostB[newLoc]=newTotCost;
				//do a sorted que insert of the new path
				QUESEARCH(qNewNdx,insertNdx);
				INSQUENODEPREV( (INT16)qNewNdx, (INT16)insertNdx);
			}
		}
	}
	while (pathQNotEmpty && pathNotYetFound);
	// work finished. Did we find a path?
	if (pathFound)
	{
		INT16 z,_z,_nextLink; //,tempgrid;

		_z=0;
		z=pathQB[ pathQB[QHEADNDX].nextLink ].pathNdx;

		while (z)
		{
			_nextLink = trailStratTreeB[z].nextLink;
			trailStratTreeB[z].nextLink = _z;
			_z = z;
			z = _nextLink;
		}

		// if this function was called because a solider is about to embark on an actual route
		// (as opposed to "test" path finding (used by cursor, etc), then grab all pertinent
		// data and copy into soldier's database


		z=_z;

		for (iCnt=0; z && (iCnt < MAX_PATH_LIST_SIZE); iCnt++)
		{
			gusMapPathingData[ iCnt ] = trailStratTreeB[z].diStratDelta;

			z = trailStratTreeB[z].nextLink;
		}

		// return path length : serves as a "successful" flag and a path length counter
		return(iCnt);
	}
	// failed miserably, report...
	return(0);
}


PathSt* BuildAStrategicPath(INT16 const start_sector, INT16 const end_sector, GROUP const& g, BOOLEAN const fTacticalTraversal)
{
	if (end_sector < MAP_WORLD_X - 1) return NULL;

	const INT32 path_len = FindStratPath(start_sector, end_sector, g, fTacticalTraversal);
	if (path_len == 0) return NULL;

	// start new path list
	PathSt* const head = new PathSt{};
	head->uiSectorId = start_sector;
	head->pNext      = NULL;
	head->pPrev      = NULL;

	INT32   cur_sector = start_sector;
	INT32   delta      = 0;
	PathSt* path       = head;
	for (INT32 i = 0; i < path_len; ++i)
	{
		switch (gusMapPathingData[i])
		{
			case NORTH: delta = NORTH_MOVE; break;
			case SOUTH: delta = SOUTH_MOVE; break;
			case EAST:  delta = EAST_MOVE;  break;
			case WEST:  delta = WEST_MOVE;  break;
		}
		// create new node
		cur_sector += delta;

		if (cur_sector < MAP_WORLD_X - 1)
		{
			ClearStrategicPathList(head, 0);
			return NULL;
		}

		PathSt* const n = new PathSt{};
		n->uiSectorId = cur_sector;
		n->pPrev      = path;
		n->pNext      = NULL;
		path->pNext   = n;
		path          = n;
	}

	return head;
}


PathSt* AppendStrategicPath(PathSt* pNewSection, PathSt* pHeadOfPathList)
{
	// will append a new section onto the end of the head of list, then return the head of the new list

	PathSt* pNode = pHeadOfPathList;
	// move to end of original section

	if( pNewSection == NULL )
	{
		return pHeadOfPathList;
	}


	// is there in fact a list to append to
	if( pNode )
	{
		// move to tail of old list
		while( pNode -> pNext )
		{
			// next node in list
			pNode = pNode ->pNext;
		}

		// make sure the 2 are not the same

		if( pNode -> uiSectorId == pNewSection -> uiSectorId )
		{
			// are the same, remove head of new list
			pNewSection = RemoveHeadFromStrategicPath( pNewSection );
		}

		// append onto old list
		pNode -> pNext = pNewSection;
		pNewSection -> pPrev = pNode;

	}
	else
	{
		// head of list becomes head of new section
		pHeadOfPathList = pNewSection;
	}

	// return head of new list
	return( pHeadOfPathList );
}


PathSt* ClearStrategicPathList(PathSt* const pHeadOfPath, const INT16 sMvtGroup)
{
	// will clear out a strategic path and return head of list as NULL

	// is there in fact a path?
	if (pHeadOfPath == NULL) return NULL;
	Assert(pHeadOfPath->pPrev == NULL);

	for (PathSt* n = pHeadOfPath; n != NULL;)
	{
		PathSt* const del = n;
		n = n->pNext;
		delete del;
	}

	if (sMvtGroup != -1 && sMvtGroup != 0)
	{
		RemoveGroupWaypoints(*GetGroup(sMvtGroup));
	}
	return NULL;
}


static PathSt* MoveToEndOfPathList(PathSt* pList);


PathSt* ClearStrategicPathListAfterThisSector(PathSt* pHeadOfPath, const SGPSector& sMap, INT16 sMvtGroup)
{
	// will clear out a strategic path and return head of list as NULL
	PathSt* pNode = pHeadOfPath;
	PathSt* pDeleteNode = pHeadOfPath;
	INT16 sCurrentSector = -1;

	// is there in fact a path?
	if( pNode == NULL )
	{
		// no path, leave
		return ( pNode );
	}

	// get sector value
	INT16 sSector = sMap.AsStrategicIndex();

	// go to end of list
	pNode = MoveToEndOfPathList( pNode );

	// get current sector value
	sCurrentSector = ( INT16 )pNode -> uiSectorId;

	// move through list
	while( ( pNode )&&( sSector != sCurrentSector ) )
	{

		// next value
		pNode = pNode -> pPrev;

		// get current sector value
		if( pNode != NULL )
		{
			sCurrentSector = ( INT16 )pNode -> uiSectorId;
		}
	}

	// did we find the target sector?
	if( pNode == NULL )
	{
		// nope, leave
		return ( pHeadOfPath );
	}


	// we want to KEEP the target sector, not delete it, so advance to the next sector
	pNode = pNode -> pNext;

	// is nothing left?
	if( pNode == NULL )
	{
		// that's it, leave
		return ( pHeadOfPath );
	}


	// if we're NOT about to clear the head (there's a previous entry)
	if( pNode -> pPrev )
	{
		// set next for tail to NULL
		pNode -> pPrev -> pNext = NULL;
	}
	else
	{
		// clear head, return NULL
		pHeadOfPath = ClearStrategicPathList( pHeadOfPath, sMvtGroup );

		return ( NULL );
	}

	// clear list
	while( pNode -> pNext )
	{
		// set up delete node
		pDeleteNode = pNode;

		// move to next node
		pNode = pNode -> pNext;

		// check if we are clearing the head of the list
		if( pDeleteNode == pHeadOfPath )
		{
			// null out head
			pHeadOfPath = NULL;
		}

		// delete delete node
		delete pDeleteNode;
	}


	// clear out last node
	delete pNode;
	pNode = NULL;
	pDeleteNode = NULL;

	return( pHeadOfPath );
}


static PathSt* MoveToEndOfPathList(PathSt* pList)
{
	// move to end of list

	// no list, return
	if( pList == NULL )
	{
		return ( NULL );
	}

	// move to beginning of list
	while( pList -> pNext )
	{
		pList = pList -> pNext;
	}

	return ( pList );

}


static PathSt* RemoveTailFromStrategicPath(PathSt* pHeadOfList)
{
	// remove the tail section from the strategic path
	PathSt* pNode = pHeadOfList;
	PathSt* pLastNode = pHeadOfList;

	if( pNode == NULL )
	{
		// no list, leave
		return( NULL );
	}

	while( pNode -> pNext )
	{
		pLastNode = pNode;
		pNode = pNode -> pNext;
	}

	// end of list

	// set next to null
	pLastNode -> pNext = NULL;

	// now remove old last node
	delete pNode;

	// return head of new list
	return( pHeadOfList );

}


PathSt* RemoveHeadFromStrategicPath(PathSt* pList)
{
	// move to head of list
	PathSt* pNode = pList;
	PathSt* pNewHead = pList;

	// check if there is a list
	if( pNode == NULL )
	{
		// no list, leave
		return( NULL );
	}

	// move to head of list
	while( pNode ->pPrev )
	{
		// back one node
		pNode = pNode -> pPrev;
	}

	// set up new head
	pNewHead = pNode -> pNext;
	if( pNewHead )
	{
		pNewHead -> pPrev = NULL;
	}

	// free old head
	delete pNode;

	pNode = NULL;

	// return new head
	return( pNewHead );
}


INT16 GetLastSectorIdInCharactersPath(const SOLDIERTYPE* pCharacter)
{
	// will return the last sector of the current path, or the current sector if there's no path
	INT16 sLastSector = ( pCharacter -> sSectorX ) + ( pCharacter ->sSectorY ) * ( MAP_WORLD_X );
	PathSt* pNode = NULL;

	pNode = GetSoldierMercPathPtr( pCharacter );

	while( pNode )
	{
		sLastSector = ( INT16 ) ( pNode -> uiSectorId );
		pNode = pNode -> pNext;
	}

	return sLastSector;
}


PathSt* CopyPaths(PathSt* src)
{
	if (src == NULL) return NULL;

	PathSt* const head = new PathSt{};
	head->uiSectorId = src->uiSectorId;
	head->pPrev      = NULL;

	for (PathSt* dst = head;;)
	{
		src = src->pNext;
		if (src == NULL)
		{
			dst->pNext = NULL;
			break;
		}

		PathSt* const p = new PathSt{};
		p->uiSectorId	= src->uiSectorId;
		p->pPrev      = dst;

		dst->pNext = p;
		dst = p;
	}

	return head;
}


#ifdef BETA_VERSION
static void VerifyAllMercsInGroupAreOnSameSquad(GROUP const& g)
{
	SOLDIERTYPE *pSoldier;
	INT8 bSquad = -1;

	// Let's choose somebody in group.....
	CFOR_EACH_PLAYER_IN_GROUP(pPlayer, &g)
	{
		pSoldier = pPlayer->pSoldier;
		Assert( pSoldier );

		if ( pSoldier->bAssignment < ON_DUTY )
		{
			if ( bSquad == -1 )
			{
				bSquad = pSoldier->bAssignment;
			}
			else
			{
				// better be the same squad!
				Assert( pSoldier->bAssignment == bSquad );
			}
		}
	}
}
#endif


void RebuildWayPointsForGroupPath(PathSt* const pHeadOfPath, GROUP& g)
{
	INT32 iDelta = 0;
	INT32 iOldDelta = 0;
	BOOLEAN fFirstNode = TRUE;
	PathSt* pNode = pHeadOfPath;
	WAYPOINT *wp = NULL;

	//KRIS!  Added this because it was possible to plot a new course to the same destination, and the
	//       group would add new arrival events without removing the existing one(s).
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);

	RemoveGroupWaypoints(g);

	if (g.fPlayer)
	{
#ifdef BETA_VERSION
	VerifyAllMercsInGroupAreOnSameSquad(g);
#endif

		// update the destination(s) in the team list
		fTeamPanelDirty = TRUE;

		// update the ETA in character info
		fCharacterInfoPanelDirty = TRUE;

		// allows assignments to flash right away if their subject moves away/returns (robot/vehicle being repaired), or
		// patient/doctor/student/trainer being automatically put on a squad via the movement menu.
		gfReEvaluateEveryonesNothingToDo = TRUE;
	}


	// if group has no path planned at all
	if( ( pNode == NULL ) || ( pNode->pNext == NULL ) )
	{
		// and it's a player group, and it's between sectors
		// NOTE: AI groups never reverse direction between sectors, Kris cheats & teleports them back to their current sector!
		if (g.fPlayer && g.fBetweenSectors)
		{
			// send the group right back to its current sector by reversing directions
			GroupReversingDirectionsBetweenSectors(&g, g.ubSector.x, g.ubSector.y, FALSE);
		}

		return;
	}


	// if we're currently between sectors
	if (g.fBetweenSectors)
	{
		// figure out which direction we're already going in  (Otherwise iOldDelta starts at 0)
		iOldDelta = g.ubNext.AsStrategicIndex() - g.ubSector.AsStrategicIndex();
	}

	// build a brand new list of waypoints, one for initial direction, and another for every "direction change" thereafter
	while( pNode->pNext )
	{
		iDelta = pNode->pNext->uiSectorId - pNode->uiSectorId;
		Assert( iDelta != 0 );		// same sector should never repeat in the path list

		// Waypoints are only added at "pivot points" - whenever there is a change in orthogonal direction.
		// If we're NOT currently between sectors, iOldDelta will start off at 0, which means that the first node can't be
		// added as a waypoint.  This is what we want - for stationary mercs, the first node in a path is the CURRENT sector.
		if( ( iOldDelta != 0 ) && ( iDelta != iOldDelta ) )
		{
			// add this strategic sector as a waypoint
			AddWaypointStrategicIDToPGroup(&g, pNode->uiSectorId);
		}

		// remember this delta
		iOldDelta = iDelta;

		pNode = pNode->pNext;
		fFirstNode = FALSE;
	}


	// there must have been at least one next node, or we would have bailed out on "no path" earlier
	Assert( !fFirstNode );

	// the final destination sector - always add a waypoint for it
	AddWaypointStrategicIDToPGroup(&g, pNode->uiSectorId);

	// at this point, the final sector in the path must be identical to this group's last waypoint
	wp = GetFinalWaypoint(&g);
	AssertMsg( wp, "Path exists, but no waypoints were added!  AM-0" );
	AssertMsg( pNode->uiSectorId == ( UINT32 ) CALCULATE_STRATEGIC_INDEX( wp->x, wp->y ), "Last waypoint differs from final path sector!  AM-0" );


	// see if we've already reached the first sector in the path (we never actually left the sector and reversed back to it)
	if (g.uiArrivalTime == GetWorldTotalMin())
	{
		// never really left.  Must set check for battle TRUE in order for HandleNonCombatGroupArrival() to run!
		GroupArrivedAtSector(g, TRUE, TRUE);
	}
}


// clear strategic movement (mercpaths and waypoints) for this soldier, and his group (including its vehicles)
void ClearMvtForThisSoldierAndGang( SOLDIERTYPE *pSoldier )
{
	GROUP *pGroup = NULL;


	// check if valid grunt
	Assert( pSoldier );

	pGroup = GetGroup( pSoldier->ubGroupID );
	Assert( pGroup );

	// clear their strategic movement (mercpaths and waypoints)
	ClearMercPathsAndWaypointsForAllInGroup(*pGroup);
}


BOOLEAN MoveGroupFromSectorToSector(GROUP& g, const SGPSector& sStart, const SGPSector& sDest)
{
	PathSt* pNode = BuildAStrategicPath(sStart.AsStrategicIndex(), sDest.AsStrategicIndex(), g, FALSE);

	if( pNode == NULL )
	{
		return( FALSE );
	}

	// start movement to next sector
	RebuildWayPointsForGroupPath(pNode, g);

	// now clear out the mess
	pNode = ClearStrategicPathList( pNode, -1 );

	return( TRUE );
}


static BOOLEAN MoveGroupFromSectorToSectorButAvoidLastSector(GROUP& g, const SGPSector& sStart, const SGPSector& sDest)
{
	PathSt* pNode = BuildAStrategicPath(sStart.AsStrategicIndex(), sDest.AsStrategicIndex(), g, FALSE);

	if( pNode == NULL )
	{
		return( FALSE );
	}

	// remove tail from path
	pNode = RemoveTailFromStrategicPath( pNode );

	// start movement to next sector
	RebuildWayPointsForGroupPath(pNode, g);

	// now clear out the mess
	pNode = ClearStrategicPathList( pNode, -1 );

	return( TRUE );
}


BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectors(GROUP& g, const SGPSector& sStart, const SGPSector& sDest)
{
	// init sectors with soldiers in them
	InitSectorsWithSoldiersList( );

	// build the list of sectors with soldier in them
	BuildSectorsWithSoldiersList( );

	// turn on the avoid flag
	gfPlotToAvoidPlayerInfuencedSectors = TRUE;

	PathSt* pNode = BuildAStrategicPath(sStart.AsStrategicIndex(), sDest.AsStrategicIndex(), g, FALSE);

	// turn off the avoid flag
	gfPlotToAvoidPlayerInfuencedSectors = FALSE;

	if( pNode == NULL )
	{
		return MoveGroupFromSectorToSector(g, sStart, sDest);
	}

	// start movement to next sector
	RebuildWayPointsForGroupPath(pNode, g);

	// now clear out the mess
	pNode = ClearStrategicPathList( pNode, -1 );

	return( TRUE );
}


BOOLEAN MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectorsAndStopOneSectorBeforeEnd(GROUP& g, const SGPSector& sStart, const SGPSector& sDest)
{
	// init sectors with soldiers in them
	InitSectorsWithSoldiersList( );

	// build the list of sectors with soldier in them
	BuildSectorsWithSoldiersList( );

	// turn on the avoid flag
	gfPlotToAvoidPlayerInfuencedSectors = TRUE;

	PathSt* pNode = BuildAStrategicPath(sStart.AsStrategicIndex(), sDest.AsStrategicIndex(), g, FALSE);

	// turn off the avoid flag
	gfPlotToAvoidPlayerInfuencedSectors = FALSE;

	if( pNode == NULL )
	{
		return MoveGroupFromSectorToSectorButAvoidLastSector(g, sStart, sDest);
	}

	// remove tail from path
	pNode = RemoveTailFromStrategicPath( pNode );

	// start movement to next sector
	RebuildWayPointsForGroupPath(pNode, g);

	// now clear out the mess
	pNode = ClearStrategicPathList( pNode, -1 );

	return( TRUE );
}


INT32 GetLengthOfPath(PathSt* pHeadPath)
{
	INT32 iLength = 0;
	PathSt* pNode = pHeadPath;

	while( pNode )
	{
		pNode = pNode->pNext;
		iLength++;
	}

	return( iLength );
}


INT32 GetLengthOfMercPath(const SOLDIERTYPE* pSoldier)
{
	PathSt* pNode = NULL;
	INT32 iLength = 0;

	pNode = GetSoldierMercPathPtr( pSoldier );
	iLength = GetLengthOfPath( pNode );
	return( iLength );
}


PathSt* GetSoldierMercPathPtr(SOLDIERTYPE const* const s)
{
	Assert(s);
	return
		/* In a vehicle? */
		s->bAssignment == VEHICLE          ? pVehicleList[s->iVehicleId].pMercPath :
		/* Is a vehicle? */
		s->uiStatusFlags & SOLDIER_VEHICLE ? pVehicleList[s->bVehicleID].pMercPath :
		/* A person */
		s->pMercPath;
}


PathSt* GetGroupMercPathPtr(GROUP const& g)
{
	Assert(g.fPlayer);

	if (g.fVehicle)
	{
		return GetVehicleFromMvtGroup(g).pMercPath;
	}

	if (g.pPlayerList && g.pPlayerList->pSoldier) // XXX pSoldier test necessary?
	{
		return g.pPlayerList->pSoldier->pMercPath;
	}

	return 0;
}


GROUP* GetSoldierGroup(SOLDIERTYPE const& s)
{
	UINT8 const group_id =
		/* In a vehicle? */
		s.bAssignment == VEHICLE          ? pVehicleList[s.iVehicleId].ubMovementGroup :
		/* Is a vehicle? */
		s.uiStatusFlags & SOLDIER_VEHICLE ? pVehicleList[s.bVehicleID].ubMovementGroup :
		/* A person */
		s.ubGroupID;
	return GetGroup(group_id);
}


static void ClearPathForSoldier(SOLDIERTYPE* pSoldier);


// clears this groups strategic movement (mercpaths and waypoints), include those in the vehicle structs(!)
void ClearMercPathsAndWaypointsForAllInGroup(GROUP& g)
{
	SOLDIERTYPE *pSoldier = NULL;

	CFOR_EACH_PLAYER_IN_GROUP(pPlayer, &g)
	{
		pSoldier = pPlayer->pSoldier;

		if ( pSoldier != NULL )
		{
			ClearPathForSoldier( pSoldier );
		}
	}

	// if it's a vehicle
	if (g.fVehicle)
	{
		VEHICLETYPE& v = GetVehicleFromMvtGroup(g);
		// clear the path for that vehicle
		v.pMercPath = ClearStrategicPathList(v.pMercPath, v.ubMovementGroup);
	}

	// clear the waypoints for this group too - no mercpath = no waypoints!
	RemoveGroupWaypoints(g);
}


// clears the contents of the soldier's mercpPath, as well as his vehicle path if he is a / or is in a vehicle
static void ClearPathForSoldier(SOLDIERTYPE* pSoldier)
{
	VEHICLETYPE *pVehicle = NULL;


	// clear the soldier's mercpath
	pSoldier->pMercPath = ClearStrategicPathList( pSoldier->pMercPath, pSoldier->ubGroupID );

	// if a vehicle
	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		pVehicle = &( pVehicleList[ pSoldier->bVehicleID ] );
	}
	// or in a vehicle
	else if( pSoldier->bAssignment == VEHICLE )
	{
		pVehicle = &( pVehicleList[ pSoldier->iVehicleId ] );
	}

	// if there's an associate vehicle structure
	if ( pVehicle != NULL )
	{
		// clear its mercpath, too
		pVehicle->pMercPath = ClearStrategicPathList( pVehicle->pMercPath, pVehicle->ubMovementGroup );
	}
}


static void AddSectorToFrontOfMercPath(PathSt** ppMercPath, const SGPSector& sMap);


void AddSectorToFrontOfMercPathForAllSoldiersInGroup(GROUP *pGroup, const SGPSector& sMap)
{
	SOLDIERTYPE *pSoldier = NULL;

	CFOR_EACH_PLAYER_IN_GROUP(pPlayer, pGroup)
	{
		pSoldier = pPlayer->pSoldier;

		if ( pSoldier != NULL )
		{
			AddSectorToFrontOfMercPath(&(pSoldier->pMercPath), sMap);
		}
	}

	// if it's a vehicle
	if ( pGroup->fVehicle )
	{
		VEHICLETYPE& v = GetVehicleFromMvtGroup(*pGroup);
		// add it to that vehicle's path
		AddSectorToFrontOfMercPath(&v.pMercPath, sMap);
	}
}


static void AddSectorToFrontOfMercPath(PathSt** ppMercPath, const SGPSector& sMap)
{
	// allocate and hang a new node at the front of the path list
	PathSt* const pNode = new PathSt{};
	pNode->uiSectorId = sMap.AsStrategicIndex();
	pNode->pNext = *ppMercPath;
	pNode->pPrev = NULL;

	// if path wasn't null
	if ( *ppMercPath != NULL )
	{
		// hang the previous pointer of the old head to the new head
		(*ppMercPath)->pPrev = pNode;
	}

	*ppMercPath = pNode;
}
