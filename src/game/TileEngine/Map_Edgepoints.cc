#include "Buffer.h"
#include "Map_Edgepoints.h"
#include "Soldier_Control.h"
#include "PathAI.h"
#include "AI.h"
#include "Map_Information.h"
#include "Isometric_Utils.h"
#include "Debug.h"
#include "Random.h"
#include "Strategic.h"
#include "Animation_Control.h"
#include "Render_Fun.h"
#include "StrategicMap.h"
#include "Environment.h"
#include "TileDef.h"
#include "WorldMan.h"
#include "MemMan.h"
#include "FileMan.h"

#include "Message.h"

#include <vector>


//dynamic arrays that contain the valid gridno's for each edge
std::vector<INT16> gps1stNorthEdgepointArray;
std::vector<INT16> gps1stEastEdgepointArray;
std::vector<INT16> gps1stSouthEdgepointArray;
std::vector<INT16> gps1stWestEdgepointArray;
//contains the index value for the first array index of the second row of each edgepoint array.
//Because each edgepoint side has two rows, the outside most row is calculated first, then the inside row.
//For purposes of AI, it may become necessary to avoid this.
UINT16 gus1stNorthEdgepointMiddleIndex		= 0;
UINT16 gus1stEastEdgepointMiddleIndex			= 0;
UINT16 gus1stSouthEdgepointMiddleIndex		= 0;
UINT16 gus1stWestEdgepointMiddleIndex			= 0;

//dynamic arrays that contain the valid gridno's for each edge
std::vector<INT16> gps2ndNorthEdgepointArray;
std::vector<INT16> gps2ndEastEdgepointArray;
std::vector<INT16> gps2ndSouthEdgepointArray;
std::vector<INT16> gps2ndWestEdgepointArray;
//contains the index value for the first array index of the second row of each edgepoint array.
//Because each edgepoint side has two rows, the outside most row is calculated first, then the inside row.
//For purposes of AI, it may become necessary to avoid this.
UINT16 gus2ndNorthEdgepointMiddleIndex		= 0;
UINT16 gus2ndEastEdgepointMiddleIndex			= 0;
UINT16 gus2ndSouthEdgepointMiddleIndex		= 0;
UINT16 gus2ndWestEdgepointMiddleIndex			= 0;

BOOLEAN gfEdgepointsExist = FALSE;
BOOLEAN gfGeneratingMapEdgepoints = FALSE;

INT16 gsTLGridNo = 13286;
INT16 gsTRGridNo = 1043;
INT16 gsBLGridNo = 24878;
INT16 gsBRGridNo = 12635;

extern UINT8 gubTacticalDirection;

void TrashMapEdgepoints()
{
	//Primary edgepoints
	gps1stNorthEdgepointArray.clear();
	gps1stEastEdgepointArray.clear();
	gps1stSouthEdgepointArray.clear();
	gps1stWestEdgepointArray.clear();
	gus1stNorthEdgepointMiddleIndex		= 0;
	gus1stEastEdgepointMiddleIndex		= 0;
	gus1stSouthEdgepointMiddleIndex		= 0;
	gus1stWestEdgepointMiddleIndex		= 0;
	//Secondary edgepoints
	gps2ndNorthEdgepointArray.clear();
	gps2ndEastEdgepointArray.clear();
	gps2ndSouthEdgepointArray.clear();
	gps2ndWestEdgepointArray.clear();
	gus2ndNorthEdgepointMiddleIndex		= 0;
	gus2ndEastEdgepointMiddleIndex		= 0;
	gus2ndSouthEdgepointMiddleIndex		= 0;
	gus2ndWestEdgepointMiddleIndex		= 0;
}


static BOOLEAN VerifyEdgepoint(SOLDIERTYPE* pSoldier, INT16 sEdgepoint);


static void ValidateMapEdge(SOLDIERTYPE& s, UINT16& middle_idx, std::vector<INT16>& edgepoints)
{
	size_t dst = 0;
	size_t middle = middle_idx;
	size_t end = edgepoints.size();
	Assert(end <= UINT16_MAX);
	for (size_t i = 0; i != end; ++i)
	{
		if (VerifyEdgepoint(&s, edgepoints[i]))
		{
			edgepoints[dst] = edgepoints[i];
			if (middle == i) middle = dst; // Adjust the middle index to the new one
			++dst;
		}
		else if (middle == i)
		{ // Increment the middle index because its edgepoint is no longer valid
			++middle;
		}
	}
	middle_idx = static_cast<UINT16>(middle);
	edgepoints.resize(dst);
}


/* This final step eliminates some edgepoints which actually don't path directly
	* to the edge of the map. Cases would include an area that is close to the
	* edge, but a fence blocks it from direct access to the edge of the map. */
static void ValidateEdgepoints()
{
	SOLDIERTYPE s;
	s = SOLDIERTYPE{};
	s.bTeam = ENEMY_TEAM;

	ValidateMapEdge(s, gus1stNorthEdgepointMiddleIndex, gps1stNorthEdgepointArray);
	ValidateMapEdge(s, gus1stEastEdgepointMiddleIndex,  gps1stEastEdgepointArray);
	ValidateMapEdge(s, gus1stSouthEdgepointMiddleIndex, gps1stSouthEdgepointArray);
	ValidateMapEdge(s, gus1stWestEdgepointMiddleIndex,  gps1stWestEdgepointArray);

	ValidateMapEdge(s, gus2ndNorthEdgepointMiddleIndex, gps2ndNorthEdgepointArray);
	ValidateMapEdge(s, gus2ndEastEdgepointMiddleIndex,  gps2ndEastEdgepointArray);
	ValidateMapEdge(s, gus2ndSouthEdgepointMiddleIndex, gps2ndSouthEdgepointArray);
	ValidateMapEdge(s, gus2ndWestEdgepointMiddleIndex,  gps2ndWestEdgepointArray);
}


static void CompactEdgepointArray(std::vector<INT16>& edgepoints, UINT16& middle)
{
	size_t n = 0;
	for (size_t i = 0; i < edgepoints.size(); i++)
	{
		if (edgepoints[ i ] == -1)
		{
			if (i < middle)
			{
				middle--;
			}
		}
		else
		{
			if (n != i)
			{
				edgepoints[ n ] = edgepoints[ i ];
			}
			n++;
		}
	}
	edgepoints.resize(n);
}


static BOOLEAN EdgepointsClose(SOLDIERTYPE* pSoldier, INT16 sEdgepoint1, INT16 sEdgepoint2);


static void InternallyClassifyEdgepoints(SOLDIERTYPE* pSoldier, INT16 sGridNo, std::vector<INT16>& edgepoints1, UINT16& middle1, std::vector<INT16>& edgepoints2, UINT16& middle2)
{
	size_t i;
	UINT16 us1stBenchmarkID, us2ndBenchmarkID;
	us1stBenchmarkID = us2ndBenchmarkID = 0xffff;
	for (i = 0; i < edgepoints1.size(); i++)
	{
		if (sGridNo == edgepoints1[ i ])
		{
			if (i < middle1)
			{ //in the first half of the array
				us1stBenchmarkID = (UINT16)i;
				//find the second benchmark
				for (i = middle1; i < edgepoints1.size(); i++)
				{
					if (EdgepointsClose( pSoldier, edgepoints1[ us1stBenchmarkID ], edgepoints1[ i ] ))
					{
						us2ndBenchmarkID = (UINT16)i;
						break;
					}
				}
			}
			else
			{ //in the second half of the array
				us2ndBenchmarkID = (UINT16)i;
				//find the first benchmark
				for (i = 0; i < middle1; i++)
				{
					if (EdgepointsClose( pSoldier, edgepoints1[ us2ndBenchmarkID ], edgepoints1[ i ] ))
					{
						us1stBenchmarkID = (UINT16)i;
						break;
					}
				}
			}
			break;
		}
	}
	//Now we have found the two benchmarks, so go in both directions for each one to determine which entrypoints
	//are going to be used in the primary array.  All rejections will be positioned in the secondary array for
	//use for isolated entry when tactically traversing.
	if( us1stBenchmarkID != 0xffff )
	{
		for( i = us1stBenchmarkID; i > 0; i-- )
		{
			if (!EdgepointsClose( pSoldier, edgepoints1[ i ], edgepoints1[ i-1 ] ))
			{ //All edgepoints from index 0 to i-1 are rejected.
				while( i )
				{
					i--;
					edgepoints2.push_back(edgepoints1[ i ]);
					middle2++;
					edgepoints1[ i ] = -1;
				}
				break;
			}
		}
		for (i = us1stBenchmarkID; (INT32)i < middle1 - 1; i++)
		{
			if (!EdgepointsClose( pSoldier, edgepoints1[ i ], edgepoints1[ i+1 ] ))
			{ //All edgepoints from index i+1 to 1st middle index are rejected.
				while ((INT32)i < middle1 - 1)
				{
					i++;
					edgepoints2.push_back(edgepoints1[ i ]);
					middle2++;
					edgepoints1[ i ] = -1;
				}
				break;
			}
		}
	}
	if( us2ndBenchmarkID != 0xffff )
	{
		for (i = us2ndBenchmarkID; i > middle1; i--)
		{
			if (!EdgepointsClose( pSoldier, edgepoints1[ i ], edgepoints1[ i-1 ] ))
			{ //All edgepoints from 1st middle index  to i-1 are rejected.
				while (i > middle1)
				{
					i--;
					edgepoints2.push_back(edgepoints1[ i ]);
					edgepoints1[ i ] = -1;
				}
				break;
			}
		}
		for (i = us2ndBenchmarkID; i < edgepoints1.size() - 1; i++)
		{
			if (!EdgepointsClose( pSoldier, edgepoints1[ i ], edgepoints1[ i+1 ] ))
			{ //All edgepoints from index 0 to i-1 are rejected.
				while (i < edgepoints1.size() - 1)
				{
					i++;
					edgepoints2.push_back(edgepoints1[ i ]);
					edgepoints1[ i ] = -1;
				}
				break;
			}
		}
	}
	//Now compact the primary array, because some edgepoints have been removed.
	CompactEdgepointArray(edgepoints1, middle1);
}


static void ClassifyEdgepoints(void)
{
	SOLDIERTYPE Soldier;
	INT16 sGridNo = -1;

	Soldier = SOLDIERTYPE{};
	Soldier.bTeam = 1;

	//north
	if( gMapInformation.sNorthGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sNorthGridNo, NORTH_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				gps1stNorthEdgepointArray, gus1stNorthEdgepointMiddleIndex,
				gps2ndNorthEdgepointArray, gus2ndNorthEdgepointMiddleIndex );
		}
	}
	//east
	if( gMapInformation.sEastGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sEastGridNo, EAST_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				gps1stEastEdgepointArray, gus1stEastEdgepointMiddleIndex,
				gps2ndEastEdgepointArray, gus2ndEastEdgepointMiddleIndex );
		}
	}
	//south
	if( gMapInformation.sSouthGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sSouthGridNo, SOUTH_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				gps1stSouthEdgepointArray, gus1stSouthEdgepointMiddleIndex,
				gps2ndSouthEdgepointArray, gus2ndSouthEdgepointMiddleIndex );
		}
	}
	//west
	if( gMapInformation.sWestGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sWestGridNo, WEST_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				gps1stWestEdgepointArray, gus1stWestEdgepointMiddleIndex,
				gps2ndWestEdgepointArray, gus2ndWestEdgepointMiddleIndex );
		}
	}
}

void GenerateMapEdgepoints()
{
	INT16 sGridNo=-1;

	//Get rid of the current edgepoint lists.
	TrashMapEdgepoints();

	gfGeneratingMapEdgepoints = TRUE;

	if( gMapInformation.sNorthGridNo	!= -1 )
		sGridNo = gMapInformation.sNorthGridNo;
	else if( gMapInformation.sEastGridNo	!= -1 )
		sGridNo = gMapInformation.sEastGridNo;
	else if( gMapInformation.sSouthGridNo	!= -1 )
		sGridNo = gMapInformation.sSouthGridNo;
	else if( gMapInformation.sWestGridNo != -1 )
		sGridNo = gMapInformation.sWestGridNo;
	else if( gMapInformation.sCenterGridNo != -1 )
		sGridNo = gMapInformation.sCenterGridNo;
	else
		return;

	GlobalReachableTest( sGridNo );

	//Calculate the north edges
	if( gMapInformation.sNorthGridNo != -1 )
	{
		//1st row
		sGridNo = gsTLGridNo;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stNorthEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo > gsTRGridNo )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stNorthEdgepointArray.push_back(sGridNo);
			}
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stNorthEdgepointArray.push_back(sGridNo);
			}
		}
		//2nd row
		Assert(gps1stNorthEdgepointArray.size() <= UINT16_MAX);
		gus1stNorthEdgepointMiddleIndex = static_cast<UINT16>(gps1stNorthEdgepointArray.size());
		sGridNo = gsTLGridNo + 161;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stNorthEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo > gsTRGridNo + 161 )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stNorthEdgepointArray.push_back(sGridNo);
			}
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stNorthEdgepointArray.push_back(sGridNo);
			}
		}
	}
	//Calculate the east edges
	if( gMapInformation.sEastGridNo != -1 )
	{
		//1st row
		sGridNo = gsTRGridNo;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stEastEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo < gsBRGridNo )
		{
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stEastEdgepointArray.push_back(sGridNo);
			}
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stEastEdgepointArray.push_back(sGridNo);
			}
		}
		//2nd row
		Assert(gps1stEastEdgepointArray.size() <= UINT16_MAX);
		gus1stEastEdgepointMiddleIndex = static_cast<UINT16>(gps1stEastEdgepointArray.size());
		sGridNo = gsTRGridNo + 159;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stEastEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo < gsBRGridNo + 159 )
		{
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stEastEdgepointArray.push_back(sGridNo);
			}
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stEastEdgepointArray.push_back(sGridNo);
			}
		}
	}
	//Calculate the south edges
	if( gMapInformation.sSouthGridNo != -1 )
	{
		//1st row
		sGridNo = gsBLGridNo;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stSouthEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo > gsBRGridNo )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stSouthEdgepointArray.push_back(sGridNo);
			}
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stSouthEdgepointArray.push_back(sGridNo);
			}
		}
		//2nd row
		Assert(gps1stSouthEdgepointArray.size() <= UINT16_MAX);
		gus1stSouthEdgepointMiddleIndex = static_cast<UINT16>(gps1stSouthEdgepointArray.size());
		sGridNo = gsBLGridNo - 161;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stSouthEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo > gsBRGridNo - 161 )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stSouthEdgepointArray.push_back(sGridNo);
			}
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stSouthEdgepointArray.push_back(sGridNo);
			}
		}
	}
	//Calculate the west edges
	if( gMapInformation.sWestGridNo != -1 )
	{
		//1st row
		sGridNo = gsTLGridNo;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stWestEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo < gsBLGridNo )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stWestEdgepointArray.push_back(sGridNo);
			}
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stWestEdgepointArray.push_back(sGridNo);
			}
		}
		//2nd row
		Assert(gps1stWestEdgepointArray.size() <= UINT16_MAX);
		gus1stWestEdgepointMiddleIndex = static_cast<UINT16>(gps1stWestEdgepointArray.size());
		sGridNo = gsTLGridNo - 159;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
		{
			gps1stWestEdgepointArray.push_back(sGridNo);
		}
		while( sGridNo < gsBLGridNo - 159 )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stWestEdgepointArray.push_back(sGridNo);
			}
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps1stWestEdgepointArray.push_back(sGridNo);
			}
		}
	}

	//CHECK FOR ISOLATED EDGEPOINTS (but only if the entrypoint is ISOLATED!!!)
	if( gMapInformation.sIsolatedGridNo != -1 && !(gpWorldLevelData[ gMapInformation.sIsolatedGridNo ].uiFlags & MAPELEMENT_REACHABLE) )
	{
		GlobalReachableTest( gMapInformation.sIsolatedGridNo );
		if( gMapInformation.sNorthGridNo != -1 )
		{
			//1st row
			sGridNo = gsTLGridNo;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndNorthEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo > gsTRGridNo )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndNorthEdgepointArray.push_back(sGridNo);
				}
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndNorthEdgepointArray.push_back(sGridNo);
				}
			}
			//2nd row
			Assert(gps2ndNorthEdgepointArray.size() <= UINT16_MAX);
			gus2ndNorthEdgepointMiddleIndex = static_cast<UINT16>(gps2ndNorthEdgepointArray.size());
			sGridNo = gsTLGridNo + 161;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndNorthEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo > gsTRGridNo + 161 )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndNorthEdgepointArray.push_back(sGridNo);
				}
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndNorthEdgepointArray.push_back(sGridNo);
				}
			}
		}
		//Calculate the east edges
		if( gMapInformation.sEastGridNo != -1 )
		{
			//1st row
			sGridNo = gsTRGridNo;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndEastEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo < gsBRGridNo )
			{
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndEastEdgepointArray.push_back(sGridNo);
				}
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndEastEdgepointArray.push_back(sGridNo);
				}
			}
			//2nd row
			Assert(gps2ndEastEdgepointArray.size() <= UINT16_MAX);
			gus2ndEastEdgepointMiddleIndex = static_cast<UINT16>(gps2ndEastEdgepointArray.size());
			sGridNo = gsTRGridNo + 159;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndEastEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo < gsBRGridNo + 159 )
			{
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndEastEdgepointArray.push_back(sGridNo);
				}
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndEastEdgepointArray.push_back(sGridNo);
				}
			}
		}
		//Calculate the south edges
		if( gMapInformation.sSouthGridNo != -1 )
		{
			//1st row
			sGridNo = gsBLGridNo;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndSouthEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo > gsBRGridNo )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndSouthEdgepointArray.push_back(sGridNo);
				}
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndSouthEdgepointArray.push_back(sGridNo);
				}
			}
			//2nd row
			Assert(gps2ndSouthEdgepointArray.size() <= UINT16_MAX);
			gus2ndSouthEdgepointMiddleIndex = static_cast<UINT16>(gps2ndSouthEdgepointArray.size());
			sGridNo = gsBLGridNo - 161;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndSouthEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo > gsBRGridNo - 161 )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndSouthEdgepointArray.push_back(sGridNo);
				}
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndSouthEdgepointArray.push_back(sGridNo);
				}
			}
		}
		//Calculate the west edges
		if( gMapInformation.sWestGridNo != -1 )
		{
			//1st row
			sGridNo = gsTLGridNo;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndWestEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo < gsBLGridNo )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndWestEdgepointArray.push_back(sGridNo);
				}
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndWestEdgepointArray.push_back(sGridNo);
				}
			}
			//2nd row
			Assert(gps2ndWestEdgepointArray.size() <= UINT16_MAX);
			gus2ndWestEdgepointMiddleIndex = static_cast<UINT16>(gps2ndWestEdgepointArray.size());
			sGridNo = gsTLGridNo - 159;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			{
				gps2ndWestEdgepointArray.push_back(sGridNo);
			}
			while( sGridNo < gsBLGridNo - 159 )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndWestEdgepointArray.push_back(sGridNo);
				}
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					(!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				{
					gps2ndWestEdgepointArray.push_back(sGridNo);
				}
			}
		}
	}

	//Eliminates any edgepoints not accessible to the edge of the world.  This is done to the primary edgepoints
	ValidateEdgepoints();
	//Second step is to process the primary edgepoints and determine if any of the edgepoints aren't accessible from
	//the associated entrypoint.  These edgepoints that are rejected are placed in the secondary list.
	if( gMapInformation.sIsolatedGridNo != -1 )
	{ //only if there is an isolated gridno in the map.  There is a flaw in the design of this system.  The classification
		//process will automatically assign areas to be isolated if there is an obstacle between one normal edgepoint and another
		//causing a 5 tile connection check to fail.  So, all maps with isolated edgepoints will need to be checked manually to
		//make sure there are no obstacles causing this to happen (except for obstacles between normal areas and the isolated area)

		//Good thing most maps don't have isolated sections.  This is one expensive function to call!  Maybe 200MI!
		ClassifyEdgepoints();
	}

	gfGeneratingMapEdgepoints = FALSE;
}


static void SaveMapEdgepoint(HWFILE const f, UINT16 const& middle, const std::vector<INT16>& edgepoints)
{
	Assert(edgepoints.size() <= UINT16_MAX);
	UINT16 n = static_cast<UINT16>(edgepoints.size());
	f->write(&n,   sizeof(UINT16));
	f->write(&middle, sizeof(UINT16));
	if (n != 0) f->write(edgepoints.data(),  sizeof(*edgepoints.data()) * n);
}


void SaveMapEdgepoints(HWFILE const f)
{
	// 1st priority edgepoints -- for common entry -- tactical placement gui uses only these points.
	SaveMapEdgepoint(f, gus1stNorthEdgepointMiddleIndex, gps1stNorthEdgepointArray);
	SaveMapEdgepoint(f, gus1stEastEdgepointMiddleIndex,  gps1stEastEdgepointArray);
	SaveMapEdgepoint(f, gus1stSouthEdgepointMiddleIndex, gps1stSouthEdgepointArray);
	SaveMapEdgepoint(f, gus1stWestEdgepointMiddleIndex,  gps1stWestEdgepointArray);

	// 2nd priority edgepoints -- for isolated areas.  Okay to be zero
	SaveMapEdgepoint(f, gus2ndNorthEdgepointMiddleIndex, gps2ndNorthEdgepointArray);
	SaveMapEdgepoint(f, gus2ndEastEdgepointMiddleIndex,  gps2ndEastEdgepointArray);
	SaveMapEdgepoint(f, gus2ndSouthEdgepointMiddleIndex, gps2ndSouthEdgepointArray);
	SaveMapEdgepoint(f, gus2ndWestEdgepointMiddleIndex,  gps2ndWestEdgepointArray);
}


static void LoadMapEdgepoint(HWFILE const f,  UINT16& middle, std::vector<INT16>& edgepoints)
{
	UINT16 n = 0;
	f->read(&n,   sizeof(UINT16));
	f->read(&middle, sizeof(UINT16));
	if (n != 0)
	{
		edgepoints.resize(n);
		f->read(edgepoints.data(), sizeof(*edgepoints.data()) * n);
	}
}


bool LoadMapEdgepoints(HWFILE const f)
{
	TrashMapEdgepoints();

	LoadMapEdgepoint(f, gus1stNorthEdgepointMiddleIndex, gps1stNorthEdgepointArray);
	LoadMapEdgepoint(f, gus1stEastEdgepointMiddleIndex,  gps1stEastEdgepointArray);
	LoadMapEdgepoint(f, gus1stSouthEdgepointMiddleIndex, gps1stSouthEdgepointArray);
	LoadMapEdgepoint(f, gus1stWestEdgepointMiddleIndex,  gps1stWestEdgepointArray);

	if (gMapInformation.ubMapVersion < 17)
	{	/* To prevent invalidation of older maps, which only used one layer of
		 * edgepoints, and a UINT8 for containing the size, we will preserve that
		 * paradigm, then kill the loaded edgepoints and regenerate them. */
		TrashMapEdgepoints();
		return false;
	}

	LoadMapEdgepoint(f, gus2ndNorthEdgepointMiddleIndex, gps2ndNorthEdgepointArray);
	LoadMapEdgepoint(f, gus2ndEastEdgepointMiddleIndex,  gps2ndEastEdgepointArray);
	LoadMapEdgepoint(f, gus2ndSouthEdgepointMiddleIndex, gps2ndSouthEdgepointArray);
	LoadMapEdgepoint(f, gus2ndWestEdgepointMiddleIndex,  gps2ndWestEdgepointArray);

	if (gMapInformation.ubMapVersion < 22)
	{	// Regenerate them
		TrashMapEdgepoints();
		return false;
	}

	return true;
}


UINT16 ChooseMapEdgepoint( UINT8 ubStrategicInsertionCode )
{
	std::vector<INT16>* pEdgepoints = nullptr;

	//First validate and get access to the correct array based on strategic direction.
	//We will use the selected array to choose insertion gridno's.
	switch( ubStrategicInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			pEdgepoints = &gps1stNorthEdgepointArray;
			break;
		case INSERTION_CODE_EAST:
			pEdgepoints = &gps1stEastEdgepointArray;
			break;
		case INSERTION_CODE_SOUTH:
			pEdgepoints = &gps1stSouthEdgepointArray;
			break;
		case INSERTION_CODE_WEST:
			pEdgepoints = &gps1stWestEdgepointArray;
			break;
		default:
			SLOGA("ChooseMapEdgepoints:  Failed to pass a valid strategic insertion code." );
			break;
	}
	if (!pEdgepoints || pEdgepoints->size() == 0)
	{
		return NOWHERE;
	}
	Assert(pEdgepoints->size() <= UINT32_MAX);
	return (*pEdgepoints)[ Random( static_cast<UINT32>(pEdgepoints->size()) ) ];
}


void ChooseMapEdgepoints(MAPEDGEPOINTINFO* const pMapEdgepointInfo, const UINT8 ubStrategicInsertionCode, UINT8 ubNumDesiredPoints)
{
	AssertMsg(ubNumDesiredPoints > 0 && ubNumDesiredPoints <= 32, String("ChooseMapEdgepoints:  Desired points = %d, valid range is 1-32", ubNumDesiredPoints));

	/* First validate and get access to the correct array based on strategic
	 * direction.  We will use the selected array to choose insertion gridno's. */
	std::vector<INT16>* pEdgepoints = nullptr;
	switch (ubStrategicInsertionCode)
	{
		case INSERTION_CODE_NORTH:
			pEdgepoints = &gps1stNorthEdgepointArray;
			break;

		case INSERTION_CODE_EAST:
			pEdgepoints = &gps1stEastEdgepointArray;
			break;

		case INSERTION_CODE_SOUTH:
			pEdgepoints = &gps1stSouthEdgepointArray;
			break;

		case INSERTION_CODE_WEST:
			pEdgepoints = &gps1stWestEdgepointArray;
			break;

		default:
			SLOGA("ChooseMapEdgepoints:  Failed to pass a valid strategic insertion code.");
			break;
	}
	pMapEdgepointInfo->ubStrategicInsertionCode = ubStrategicInsertionCode;

	if (!pEdgepoints || pEdgepoints->size() == 0)
	{
		pMapEdgepointInfo->ubNumPoints = 0;
		return;
	}

	/* JA2 Gold: don't place people in the water.  If any of the waypoints is on a
	 * water spot, we're going to have to remove it */
	std::vector<INT16> usableEdgepoints;
	for (INT16 edgepoint : *pEdgepoints)
	{
		const UINT8 terrain = GetTerrainType(edgepoint);
		if (terrain == MED_WATER || terrain == DEEP_WATER) continue;

		usableEdgepoints.push_back(edgepoint);
	}

	if (ubNumDesiredPoints >= usableEdgepoints.size())
	{ //We don't have enough points for everyone, return them all.
		Assert(usableEdgepoints.size() <= UINT8_MAX);
		pMapEdgepointInfo->ubNumPoints = static_cast<UINT8>(usableEdgepoints.size());
		for (size_t i = 0; i < usableEdgepoints.size(); ++i)
		{
			pMapEdgepointInfo->sGridNo[i] = usableEdgepoints[i];
		}
		return;
	}

	// We have more points, so choose them randomly.
	Assert(usableEdgepoints.size() <= UINT16_MAX);
	UINT16 usSlots    = static_cast<UINT16>(usableEdgepoints.size());
	UINT16 usCurrSlot = 0;
	pMapEdgepointInfo->ubNumPoints = ubNumDesiredPoints;
	for (size_t i = 0; i < usableEdgepoints.size(); ++i)
	{
		if (Random(usSlots) < ubNumDesiredPoints)
		{
			pMapEdgepointInfo->sGridNo[usCurrSlot++] = usableEdgepoints[i];
			--ubNumDesiredPoints;
		}
		--usSlots;
	}
}


INT16 *gpReservedGridNos = NULL;
INT16 gsReservedIndex	= 0;

void BeginMapEdgepointSearch()
{
	INT16 sGridNo;

	//Create the reserved list
	AssertMsg( !gpReservedGridNos, "Attempting to BeginMapEdgepointSearch that has already been created." );
	gpReservedGridNos = new INT16[20]{};
	gsReservedIndex   = 0;

	if( gMapInformation.sNorthGridNo != -1 )
		sGridNo = gMapInformation.sNorthGridNo;
	else if( gMapInformation.sEastGridNo != -1 )
		sGridNo = gMapInformation.sEastGridNo;
	else if( gMapInformation.sSouthGridNo != -1 )
		sGridNo = gMapInformation.sSouthGridNo;
	else if( gMapInformation.sWestGridNo != -1 )
		sGridNo = gMapInformation.sWestGridNo;
	else
		return;

	GlobalReachableTest( sGridNo );

	//Now, we have the path values calculated.  Now, we can check for closest edgepoints.
}

void EndMapEdgepointSearch()
{
	AssertMsg( gpReservedGridNos, "Attempting to EndMapEdgepointSearch that has already been removed." );
	delete[] gpReservedGridNos;
	gpReservedGridNos = NULL;
	gsReservedIndex = 0;
}


//THIS CODE ISN'T RECOMMENDED FOR TIME CRITICAL AREAS.
INT16 SearchForClosestPrimaryMapEdgepoint( INT16 sGridNo, UINT8 ubInsertionCode )
{
	INT32 i, iDirectionLoop;
	std::vector<INT16>* pEdgepoints = nullptr;
	INT16 sRadius, sDistance, sDirection, sOriginalGridNo;
	BOOLEAN fReserved;

	if( gsReservedIndex >= 20 )
	{ //Everything is reserved.
		SLOGA("All closest map edgepoints have been reserved.  We should only have 20 soldiers maximum...");
	}
	switch( ubInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			pEdgepoints = &gps1stNorthEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any north mapedgepoints. LC:1", gWorldSector.AsLongString().c_str()));
			break;
		case INSERTION_CODE_EAST:
			pEdgepoints = &gps1stEastEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any east mapedgepoints. LC:1", gWorldSector.AsLongString().c_str()));
			break;
		case INSERTION_CODE_SOUTH:
			pEdgepoints = &gps1stSouthEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any south mapedgepoints. LC:1", gWorldSector.AsLongString().c_str()));
			break;
		case INSERTION_CODE_WEST:
			pEdgepoints = &gps1stWestEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any west mapedgepoints. LC:1", gWorldSector.AsLongString().c_str()));
			break;
	}
	if (!pEdgepoints || pEdgepoints->size() == 0)
	{
		return NOWHERE;
	}

	//Check the initial gridno, to see if it is available and an edgepoint.
	fReserved = FALSE;
	for( i = 0; i < gsReservedIndex; i++ )
	{
		if( gpReservedGridNos[ i ] == sGridNo )
		{
			fReserved = TRUE;
			break;
		}
	}
	if( !fReserved )
	{	//Not reserved, so see if we can find this gridno in the edgepoint array.
		for (INT16 edgepoint : *pEdgepoints)
		{
			if (edgepoint == sGridNo)
			{ //Yes, the gridno is in the edgepoint array.
				gpReservedGridNos[ gsReservedIndex ] = sGridNo;
				gsReservedIndex++;
				return sGridNo;
			}
		}
	}

	//spiral outwards, until we find an unreserved mapedgepoint.
	//
	// 09 08 07 06
	// 10	01 00 05
	// 11 02 03 04
	// 12 13 14 15 ..
	sRadius = 1;
	sDirection = WORLD_COLS;
	sOriginalGridNo = sGridNo;
	while (sRadius < (INT16)(gWorldSector.z ? 30 : 10))
	{
		sGridNo = sOriginalGridNo + (-1 - WORLD_COLS)*sRadius; //start at the TOP-LEFT gridno
		for( iDirectionLoop = 0; iDirectionLoop < 4; iDirectionLoop++ )
		{
			switch( iDirectionLoop )
			{
				case 0:	sDirection = WORLD_COLS;	break;
				case 1:	sDirection = 1;						break;
				case 2:	sDirection = -WORLD_COLS;	break;
				case 3:	sDirection = -1;					break;
			}
			sDistance = sRadius * 2;
			while( sDistance-- )
			{
				sGridNo += sDirection;
				if( sGridNo < 0 || sGridNo >= WORLD_MAX )
					continue;
				//Check the gridno, to see if it is available and an edgepoint.
				fReserved = FALSE;
				for( i = 0; i < gsReservedIndex; i++ )
				{
					if( gpReservedGridNos[ i ] == sGridNo )
					{
						fReserved = TRUE;
						break;
					}
				}
				if( !fReserved )
				{	//Not reserved, so see if we can find this gridno in the edgepoint array.
					for (INT16 edgepoint : *pEdgepoints)
					{
						if (edgepoint == sGridNo)
						{ //Yes, the gridno is in the edgepoint array.
							gpReservedGridNos[ gsReservedIndex ] = sGridNo;
							gsReservedIndex++;
							return sGridNo;
						}
					}
				}
			}
		}
		sRadius++;
	}
	return NOWHERE ;
}

INT16 SearchForClosestSecondaryMapEdgepoint( INT16 sGridNo, UINT8 ubInsertionCode )
{
	INT32 i, iDirectionLoop;
	std::vector<INT16>* pEdgepoints = nullptr;
	INT16 sRadius, sDistance, sDirection, sOriginalGridNo;
	BOOLEAN fReserved;

	if( gsReservedIndex >= 20 )
	{ //Everything is reserved.
		SLOGA("All closest map edgepoints have been reserved.  We should only have 20 soldiers maximum...");
	}
	switch( ubInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			pEdgepoints = &gps2ndNorthEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any isolated north mapedgepoints. KM:1", gWorldSector.AsLongString().c_str()));
			break;
		case INSERTION_CODE_EAST:
			pEdgepoints = &gps2ndEastEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any isolated east mapedgepoints. KM:1", gWorldSector.AsLongString().c_str()));
			break;
		case INSERTION_CODE_SOUTH:
			pEdgepoints = &gps2ndSouthEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any isolated south mapedgepoints. KM:1", gWorldSector.AsLongString().c_str()));
			break;
		case INSERTION_CODE_WEST:
			pEdgepoints = &gps2ndWestEdgepointArray;
			AssertMsg(pEdgepoints->size() != 0, String("Sector %s doesn't have any isolated west mapedgepoints. KM:1", gWorldSector.AsLongString().c_str()));
			break;
	}
	if (!pEdgepoints || pEdgepoints->size() == 0)
	{
		return NOWHERE;
	}

	//Check the initial gridno, to see if it is available and an edgepoint.
	fReserved = FALSE;
	for( i = 0; i < gsReservedIndex; i++ )
	{
		if( gpReservedGridNos[ i ] == sGridNo )
		{
			fReserved = TRUE;
			break;
		}
	}
	if( !fReserved )
	{	//Not reserved, so see if we can find this gridno in the edgepoint array.
		for (INT16 edgepoint : *pEdgepoints)
		{
			if (edgepoint == sGridNo)
			{ //Yes, the gridno is in the edgepoint array.
				gpReservedGridNos[ gsReservedIndex ] = sGridNo;
				gsReservedIndex++;
				return sGridNo;
			}
		}
	}

	//spiral outwards, until we find an unreserved mapedgepoint.
	//
	// 09 08 07 06
	// 10	01 00 05
	// 11 02 03 04
	// 12 13 14 15 ..
	sRadius = 1;
	sDirection = WORLD_COLS;
	sOriginalGridNo = sGridNo;
	while( sRadius < (INT16)(gWorldSector.z ? 30 : 10) )
	{
		sGridNo = sOriginalGridNo + (-1 - WORLD_COLS)*sRadius; //start at the TOP-LEFT gridno
		for( iDirectionLoop = 0; iDirectionLoop < 4; iDirectionLoop++ )
		{
			switch( iDirectionLoop )
			{
				case 0:	sDirection = WORLD_COLS;	break;
				case 1:	sDirection = 1;						break;
				case 2:	sDirection = -WORLD_COLS;	break;
				case 3:	sDirection = -1;					break;
			}
			sDistance = sRadius * 2;
			while( sDistance-- )
			{
				sGridNo += sDirection;
				if( sGridNo < 0 || sGridNo >= WORLD_MAX )
					continue;
				//Check the gridno, to see if it is available and an edgepoint.
				fReserved = FALSE;
				for( i = 0; i < gsReservedIndex; i++ )
				{
					if( gpReservedGridNos[ i ] == sGridNo )
					{
						fReserved = TRUE;
						break;
					}
				}
				if( !fReserved )
				{	//Not reserved, so see if we can find this gridno in the edgepoint array.
					for (INT16 edgepoint : *pEdgepoints)
					{
						if (edgepoint == sGridNo)
						{ //Yes, the gridno is in the edgepoint array.
							gpReservedGridNos[ gsReservedIndex ] = sGridNo;
							gsReservedIndex++;
							return sGridNo;
						}
					}
				}
			}
		}
		sRadius++;
	}
	return NOWHERE ;
}


#define EDGE_OF_MAP_SEARCH 5


static BOOLEAN VerifyEdgepoint(SOLDIERTYPE* pSoldier, INT16 sEdgepoint)
{
	INT32		iSearchRange;
	INT16		sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXOffset, sYOffset;
	INT16		sGridNo;
	INT8		bDirection;

	pSoldier->sGridNo = sEdgepoint;

	iSearchRange = EDGE_OF_MAP_SEARCH;

	// determine maximum horizontal limits
	sMaxLeft  = std::min(iSearchRange, (pSoldier->sGridNo % MAXCOL));
	sMaxRight = std::min(iSearchRange, MAXCOL - ((pSoldier->sGridNo % MAXCOL) + 1));

	// determine maximum vertical limits
	sMaxUp   = std::min(iSearchRange, (pSoldier->sGridNo / MAXROW));
	sMaxDown = std::min(iSearchRange, MAXROW - ((pSoldier->sGridNo / MAXROW) + 1));

	// Call FindBestPath to set flags in all locations that we can
	// walk into within range.  We have to set some things up first...

	// set the distance limit of the square region
	gubNPCDistLimit = EDGE_OF_MAP_SEARCH;

	// reset the "reachable" flags in the region we're looking at
	for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++)
	{
		for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++)
		{
			sGridNo = sEdgepoint + sXOffset + (MAXCOL * sYOffset);
			gpWorldLevelData[sGridNo].uiFlags &= ~(MAPELEMENT_REACHABLE);
		}
	}

	FindBestPath( pSoldier, NOWHERE, pSoldier->bLevel, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE );

	// Turn off the "reachable" flag for the current location
	// so we don't consider it
	//gpWorldLevelData[sEdgepoint].uiFlags &= ~(MAPELEMENT_REACHABLE);

	// SET UP DOUBLE-LOOP TO STEP THROUGH POTENTIAL GRID #s
	for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++)
	{
		for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++)
		{
			// calculate the next potential gridno
			sGridNo = sEdgepoint + sXOffset + (MAXCOL * sYOffset);

			if (!(gpWorldLevelData[sGridNo].uiFlags & MAPELEMENT_REACHABLE))
			{
				continue;
			}

			if (GridNoOnEdgeOfMap( sGridNo, &bDirection ) )
			{
				// ok!
				return TRUE;
			}
		}
	}

	// no spots right on edge of map within 5 tiles
	return FALSE;
}


static BOOLEAN EdgepointsClose(SOLDIERTYPE* pSoldier, INT16 sEdgepoint1, INT16 sEdgepoint2)
{
	INT32		iSearchRange;
	INT16		sMaxLeft, sMaxRight, sMaxUp, sMaxDown, sXOffset, sYOffset;
	INT16		sGridNo;

	pSoldier->sGridNo = sEdgepoint1;

	if( gWorldSector.x == 14 && gWorldSector.y == 9 && !gWorldSector.z )
	{ //BRUTAL CODE  -- special case map.
		iSearchRange = 250;
	}
	else
	{
		iSearchRange = EDGE_OF_MAP_SEARCH;
	}

	// determine maximum horizontal limits
	sMaxLeft  = std::min(iSearchRange, (pSoldier->sGridNo % MAXCOL));
	sMaxRight = std::min(iSearchRange, MAXCOL - ((pSoldier->sGridNo % MAXCOL) + 1));

	// determine maximum vertical limits
	sMaxUp   = std::min(iSearchRange, (pSoldier->sGridNo / MAXROW));
	sMaxDown = std::min(iSearchRange, MAXROW - ((pSoldier->sGridNo / MAXROW) + 1));

	// Call FindBestPath to set flags in all locations that we can
	// walk into within range.  We have to set some things up first...

	// set the distance limit of the square region
	gubNPCDistLimit = (UINT8)iSearchRange;

	// reset the "reachable" flags in the region we're looking at
	for (sYOffset = -sMaxUp; sYOffset <= sMaxDown; sYOffset++)
	{
		for (sXOffset = -sMaxLeft; sXOffset <= sMaxRight; sXOffset++)
		{
			sGridNo = sEdgepoint1 + sXOffset + (MAXCOL * sYOffset);
			gpWorldLevelData[sGridNo].uiFlags &= ~(MAPELEMENT_REACHABLE);
		}
	}

	if( FindBestPath( pSoldier, sEdgepoint2, pSoldier->bLevel, WALKING, COPYREACHABLE, PATH_THROUGH_PEOPLE ) )
	{
		return TRUE;
	}
	return FALSE;
}

UINT8 CalcMapEdgepointClassInsertionCode( INT16 sGridNo )
{
	SOLDIERTYPE Soldier;
	std::vector<INT16>* pEdgepoints1 = nullptr;
	std::vector<INT16>* pEdgepoints2 = nullptr;
	INT16			sClosestSpot1 = NOWHERE, sClosestDist1 = 0x7FFF, sTempDist;
	INT16			sClosestSpot2 = NOWHERE, sClosestDist2 = 0x7FFF;
	BOOLEAN		fPrimaryValid = FALSE, fSecondaryValid = FALSE;

	Soldier = SOLDIERTYPE{};
	Soldier.bTeam = 1;
	Soldier.sGridNo = sGridNo;

	if( gMapInformation.sIsolatedGridNo == -1 )
	{ //If the map has no isolated area, then all edgepoints are primary.
		return INSERTION_CODE_PRIMARY_EDGEINDEX;
	}

	switch( gubTacticalDirection )
	{
		case NORTH:
			pEdgepoints1 = &gps1stNorthEdgepointArray;
			pEdgepoints2 = &gps2ndNorthEdgepointArray;
			break;
		case EAST:
			pEdgepoints1 = &gps1stEastEdgepointArray;
			pEdgepoints2 = &gps2ndEastEdgepointArray;
			break;
		case SOUTH:
			pEdgepoints1 = &gps1stSouthEdgepointArray;
			pEdgepoints2 = &gps2ndSouthEdgepointArray;
			break;
		case WEST:
			pEdgepoints1 = &gps1stWestEdgepointArray;
			pEdgepoints2 = &gps2ndWestEdgepointArray;
			break;
		default:
			// WTF???
			return INSERTION_CODE_PRIMARY_EDGEINDEX;
	}

	// Do a 2D search to find the closest map edgepoint and
	// try to create a path there
	for (INT16 edgepoint : *pEdgepoints1)
	{
		sTempDist = PythSpacesAway(sGridNo, edgepoint);
		if ( sTempDist < sClosestDist1 )
		{
			sClosestDist1 = sTempDist;
			sClosestSpot1 = edgepoint;
		}
	}
	for (INT16 edgepoint : *pEdgepoints2)
	{
		sTempDist = PythSpacesAway(sGridNo, edgepoint);
		if ( sTempDist < sClosestDist2 )
		{
			sClosestDist2 = sTempDist;
			sClosestSpot2 = edgepoint;
		}
	}

	// set the distance limit of the square region
	gubNPCDistLimit = 15;

	if( !sClosestDist1 || FindBestPath( &Soldier, sClosestSpot1, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
	{
		fPrimaryValid = TRUE;
	}
	if( !sClosestDist2 || FindBestPath( &Soldier, sClosestSpot2, 0, WALKING, NO_COPYROUTE, PATH_THROUGH_PEOPLE ) )
	{
		fSecondaryValid = TRUE;
	}

	if( fPrimaryValid == fSecondaryValid )
	{
		if( sClosestDist2 < sClosestDist1 )
		{
			return INSERTION_CODE_SECONDARY_EDGEINDEX;
		}
		return INSERTION_CODE_PRIMARY_EDGEINDEX;
	}
	if( fPrimaryValid )
	{
		return INSERTION_CODE_PRIMARY_EDGEINDEX;
	}
	return INSERTION_CODE_SECONDARY_EDGEINDEX;
}


static bool ShowMapEdgepoint(std::vector<INT16>& edgepoints, UINT16 const idx)
{
	INT32              n_illegal = 0;
	for (INT16 edgepoint : edgepoints)
	{
		if (edgepoint != -1)
		{
			AddTopmostToTail(edgepoint, idx);
		}
		else
		{
			++n_illegal;
		}
	}
	return n_illegal;
}


void ShowMapEdgepoints()
{
	INT32 n_illegal1 = 0;
	n_illegal1 += ShowMapEdgepoint(gps1stNorthEdgepointArray, FIRSTPOINTERS5);
	n_illegal1 += ShowMapEdgepoint(gps1stEastEdgepointArray,  FIRSTPOINTERS5);
	n_illegal1 += ShowMapEdgepoint(gps1stSouthEdgepointArray, FIRSTPOINTERS5);
	n_illegal1 += ShowMapEdgepoint(gps1stWestEdgepointArray,  FIRSTPOINTERS5);

	INT32 n_illegal2 = 0;
	n_illegal2 += ShowMapEdgepoint(gps2ndNorthEdgepointArray, FIRSTPOINTERS6);
	n_illegal2 += ShowMapEdgepoint(gps2ndEastEdgepointArray,  FIRSTPOINTERS6);
	n_illegal2 += ShowMapEdgepoint(gps2ndSouthEdgepointArray, FIRSTPOINTERS6);
	n_illegal2 += ShowMapEdgepoint(gps2ndWestEdgepointArray,  FIRSTPOINTERS6);

	if (n_illegal1 == 0 && n_illegal2 == 0)
	{
		SLOGD("Showing display of map edgepoints");
	}
	else
	{
		SLOGD("Showing display of map edgepoints (%d illegal primary, %d illegal secondary)", n_illegal1, n_illegal2);
	}
	SLOGD("N:%d:%d E:%d:%d S:%d:%d W:%d:%d",
		static_cast<int>(gps1stNorthEdgepointArray.size()), static_cast<int>(gps2ndNorthEdgepointArray.size()),
		static_cast<int>(gps1stEastEdgepointArray.size()),  static_cast<int>(gps2ndEastEdgepointArray.size()),
		static_cast<int>(gps1stSouthEdgepointArray.size()), static_cast<int>(gps2ndSouthEdgepointArray.size()),
		static_cast<int>(gps1stWestEdgepointArray.size()),  static_cast<int>(gps2ndWestEdgepointArray.size()));
}


static void HideMapEdgepoint(std::vector<INT16>& edgepoints)
{
	for (INT16 edgepoint : edgepoints)
	{
		if (edgepoint == -1) continue;
		RemoveAllTopmostsOfTypeRange(edgepoint, FIRSTPOINTERS, FIRSTPOINTERS);
	}
}


void HideMapEdgepoints()
{
	SLOGD("Removing display of map edgepoints");

	HideMapEdgepoint(gps1stNorthEdgepointArray);
	HideMapEdgepoint(gps1stEastEdgepointArray);
	HideMapEdgepoint(gps1stSouthEdgepointArray);
	HideMapEdgepoint(gps1stWestEdgepointArray);

	HideMapEdgepoint(gps2ndNorthEdgepointArray);
	HideMapEdgepoint(gps2ndEastEdgepointArray);
	HideMapEdgepoint(gps2ndSouthEdgepointArray);
	HideMapEdgepoint(gps2ndWestEdgepointArray);
}
