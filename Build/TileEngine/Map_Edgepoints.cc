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


//dynamic arrays that contain the valid gridno's for each edge
INT16 *gps1stNorthEdgepointArray					= NULL;
INT16 *gps1stEastEdgepointArray						= NULL;
INT16 *gps1stSouthEdgepointArray					= NULL;
INT16 *gps1stWestEdgepointArray						= NULL;
//contains the size for each array
UINT16 gus1stNorthEdgepointArraySize			= 0;
UINT16 gus1stEastEdgepointArraySize				= 0;
UINT16 gus1stSouthEdgepointArraySize			= 0;
UINT16 gus1stWestEdgepointArraySize				= 0;
//contains the index value for the first array index of the second row of each edgepoint array.
//Because each edgepoint side has two rows, the outside most row is calculated first, then the inside row.
//For purposes of AI, it may become necessary to avoid this.
UINT16 gus1stNorthEdgepointMiddleIndex		= 0;
UINT16 gus1stEastEdgepointMiddleIndex			= 0;
UINT16 gus1stSouthEdgepointMiddleIndex		= 0;
UINT16 gus1stWestEdgepointMiddleIndex			= 0;

//dynamic arrays that contain the valid gridno's for each edge
INT16 *gps2ndNorthEdgepointArray					= NULL;
INT16 *gps2ndEastEdgepointArray						= NULL;
INT16 *gps2ndSouthEdgepointArray					= NULL;
INT16 *gps2ndWestEdgepointArray						= NULL;
//contains the size for each array
UINT16 gus2ndNorthEdgepointArraySize			= 0;
UINT16 gus2ndEastEdgepointArraySize				= 0;
UINT16 gus2ndSouthEdgepointArraySize			= 0;
UINT16 gus2ndWestEdgepointArraySize				= 0;
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
	if( gps1stNorthEdgepointArray )
		MemFree( gps1stNorthEdgepointArray );
	if( gps1stEastEdgepointArray )
		MemFree( gps1stEastEdgepointArray );
	if( gps1stSouthEdgepointArray )
		MemFree( gps1stSouthEdgepointArray );
	if( gps1stWestEdgepointArray )
		MemFree( gps1stWestEdgepointArray );
	gps1stNorthEdgepointArray					= NULL;
	gps1stEastEdgepointArray					= NULL;
	gps1stSouthEdgepointArray					= NULL;
	gps1stWestEdgepointArray					=	NULL;
	gus1stNorthEdgepointArraySize			= 0;
	gus1stEastEdgepointArraySize			= 0;
	gus1stSouthEdgepointArraySize			= 0;
	gus1stWestEdgepointArraySize			= 0;
	gus1stNorthEdgepointMiddleIndex		= 0;
	gus1stEastEdgepointMiddleIndex		= 0;
	gus1stSouthEdgepointMiddleIndex		= 0;
	gus1stWestEdgepointMiddleIndex		= 0;
	//Secondary edgepoints
	if( gps2ndNorthEdgepointArray )
		MemFree( gps2ndNorthEdgepointArray );
	if( gps2ndEastEdgepointArray )
		MemFree( gps2ndEastEdgepointArray );
	if( gps2ndSouthEdgepointArray )
		MemFree( gps2ndSouthEdgepointArray );
	if( gps2ndWestEdgepointArray )
		MemFree( gps2ndWestEdgepointArray );
	gps2ndNorthEdgepointArray					= NULL;
	gps2ndEastEdgepointArray					= NULL;
	gps2ndSouthEdgepointArray					= NULL;
	gps2ndWestEdgepointArray					=	NULL;
	gus2ndNorthEdgepointArraySize			= 0;
	gus2ndEastEdgepointArraySize			= 0;
	gus2ndSouthEdgepointArraySize			= 0;
	gus2ndWestEdgepointArraySize			= 0;
	gus2ndNorthEdgepointMiddleIndex		= 0;
	gus2ndEastEdgepointMiddleIndex		= 0;
	gus2ndSouthEdgepointMiddleIndex		= 0;
	gus2ndWestEdgepointMiddleIndex		= 0;
}


static BOOLEAN VerifyEdgepoint(SOLDIERTYPE* pSoldier, INT16 sEdgepoint);


static void ValidateMapEdge(SOLDIERTYPE& s, UINT16& n, UINT16& middle_idx, INT16* const array)
{
	INT16*             dst     = array;
	INT16 const*       middle  = array + middle_idx;
	INT16 const* const end     = array + n;
	for (INT16 const* i = array; i != end; ++i)
	{
		if (VerifyEdgepoint(&s, *i))
		{
			*dst = *i;
			if (middle == i) middle = dst; // Adjust the middle index to the new one
			++dst;
		}
		else if (middle == i)
		{ // Increment the middle index because its edgepoint is no longer valid
			++middle;
		}
	}
	middle_idx = middle - array;
	n          = dst    - array;
}


/* This final step eliminates some edgepoints which actually don't path directly
 * to the edge of the map. Cases would include an area that is close to the
 * edge, but a fence blocks it from direct access to the edge of the map. */
static void ValidateEdgepoints()
{
	SOLDIERTYPE s;
	memset(&s, 0, sizeof(s));
	s.bTeam = ENEMY_TEAM;

	ValidateMapEdge(s, gus1stNorthEdgepointArraySize, gus1stNorthEdgepointMiddleIndex, gps1stNorthEdgepointArray);
	ValidateMapEdge(s, gus1stEastEdgepointArraySize,  gus1stEastEdgepointMiddleIndex,  gps1stEastEdgepointArray);
	ValidateMapEdge(s, gus1stSouthEdgepointArraySize, gus1stSouthEdgepointMiddleIndex, gps1stSouthEdgepointArray);
	ValidateMapEdge(s, gus1stWestEdgepointArraySize,  gus1stWestEdgepointMiddleIndex,  gps1stWestEdgepointArray);

	ValidateMapEdge(s, gus2ndNorthEdgepointArraySize, gus2ndNorthEdgepointMiddleIndex, gps2ndNorthEdgepointArray);
	ValidateMapEdge(s, gus2ndEastEdgepointArraySize,  gus2ndEastEdgepointMiddleIndex,  gps2ndEastEdgepointArray);
	ValidateMapEdge(s, gus2ndSouthEdgepointArraySize, gus2ndSouthEdgepointMiddleIndex, gps2ndSouthEdgepointArray);
	ValidateMapEdge(s, gus2ndWestEdgepointArraySize,  gus2ndWestEdgepointMiddleIndex,  gps2ndWestEdgepointArray);
}


static void CompactEdgepointArray(INT16** psArray, UINT16* pusMiddleIndex, UINT16* pusArraySize)
{
	INT32 i;
	UINT16 usArraySize, usValidIndex = 0;

	usArraySize = *pusArraySize;

	for( i = 0; i < usArraySize; i++ )
	{
		if( (*psArray)[ i ] == -1 )
		{
			(*pusArraySize)--;
			if( i < *pusMiddleIndex )
			{
				(*pusMiddleIndex)--;
			}
		}
		else
		{
			if( usValidIndex != i )
			{
				(*psArray)[ usValidIndex ] = (*psArray)[ i ];
			}
			usValidIndex++;
		}
	}
	*psArray = REALLOC(*psArray, INT16, *pusArraySize);
}


static BOOLEAN EdgepointsClose(SOLDIERTYPE* pSoldier, INT16 sEdgepoint1, INT16 sEdgepoint2);


static void InternallyClassifyEdgepoints(SOLDIERTYPE* pSoldier, INT16 sGridNo, INT16** psArray1, UINT16* pusMiddleIndex1, UINT16* pusArraySize1, INT16** psArray2, UINT16* pusMiddleIndex2, UINT16* pusArraySize2)
{
	INT32 i;
	UINT16 us1stBenchmarkID, us2ndBenchmarkID;
	us1stBenchmarkID = us2ndBenchmarkID = 0xffff;
	if( !(*psArray2) )
	{
		*psArray2 = MALLOCN(INT16, 400);
	}
	for( i = 0; i < *pusArraySize1; i++ )
	{
		if( sGridNo == (*psArray1)[ i ] )
		{
			if( i < *pusMiddleIndex1 )
			{ //in the first half of the array
				us1stBenchmarkID = (UINT16)i;
				//find the second benchmark
				for( i = *pusMiddleIndex1; i < *pusArraySize1; i++ )
				{
					if( EdgepointsClose( pSoldier, (*psArray1)[ us1stBenchmarkID ], (*psArray1)[ i ] ) )
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
				for( i = 0; i < *pusMiddleIndex1; i++ )
				{
					if( EdgepointsClose( pSoldier, (*psArray1)[ us2ndBenchmarkID ], (*psArray1)[ i ] ) )
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
			if( !EdgepointsClose( pSoldier, (*psArray1)[ i ], (*psArray1)[ i-1 ] ) )
			{ //All edgepoints from index 0 to i-1 are rejected.
				while( i )
				{
					i--;
					(*psArray2)[ *pusArraySize2 ] = (*psArray1)[ i ];
					(*pusMiddleIndex2)++;
					(*pusArraySize2)++;
					(*psArray1)[ i ] = -1;
				}
				break;
			}
		}
		for( i = us1stBenchmarkID; i < *pusMiddleIndex1 - 1; i++ )
		{
			if( !EdgepointsClose( pSoldier, (*psArray1)[ i ], (*psArray1)[ i+1 ] ) )
			{ //All edgepoints from index i+1 to 1st middle index are rejected.
				while( i < *pusMiddleIndex1 - 1 )
				{
					i++;
					(*psArray2)[ *pusArraySize2 ] = (*psArray1)[ i ];
					(*pusMiddleIndex2)++;
					(*pusArraySize2)++;
					(*psArray1)[ i ] = -1;
				}
				break;
			}
		}
	}
	if( us2ndBenchmarkID != 0xffff )
	{
		for( i = us2ndBenchmarkID; i > *pusMiddleIndex1; i-- )
		{
			if( !EdgepointsClose( pSoldier, (*psArray1)[ i ], (*psArray1)[ i-1 ] ) )
			{ //All edgepoints from 1st middle index  to i-1 are rejected.
				while( i > *pusMiddleIndex1 )
				{
					i--;
					(*psArray2)[ *pusArraySize2 ] = (*psArray1)[ i ];
					(*pusArraySize2)++;
					(*psArray1)[ i ] = -1;
				}
				break;
			}
		}
		for( i = us2ndBenchmarkID; i < *pusArraySize1 - 1; i++ )
		{
			if( !EdgepointsClose( pSoldier, (*psArray1)[ i ], (*psArray1)[ i+1 ] ) )
			{ //All edgepoints from index 0 to i-1 are rejected.
				while( i < *pusArraySize1 - 1 )
				{
					i++;
					(*psArray2)[ (*pusArraySize2) ] = (*psArray1)[ i ];
					(*pusArraySize2)++;
					(*psArray1)[ i ] = -1;
				}
				break;
			}
		}
	}
	//Now compact the primary array, because some edgepoints have been removed.
	CompactEdgepointArray( psArray1, pusMiddleIndex1, pusArraySize1 );
	*psArray2 = REALLOC(*psArray2, INT16, *pusArraySize2);
}


static void ClassifyEdgepoints(void)
{
	SOLDIERTYPE Soldier;
	INT16 sGridNo = -1;

	memset( &Soldier, 0, sizeof( SOLDIERTYPE ) );
	Soldier.bTeam = 1;

	//north
	if( gMapInformation.sNorthGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sNorthGridNo, NORTH_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				&gps1stNorthEdgepointArray, &gus1stNorthEdgepointMiddleIndex, &gus1stNorthEdgepointArraySize,
				&gps2ndNorthEdgepointArray, &gus2ndNorthEdgepointMiddleIndex, &gus2ndNorthEdgepointArraySize );
		}
	}
	//east
	if( gMapInformation.sEastGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sEastGridNo, EAST_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				&gps1stEastEdgepointArray, &gus1stEastEdgepointMiddleIndex, &gus1stEastEdgepointArraySize,
				&gps2ndEastEdgepointArray, &gus2ndEastEdgepointMiddleIndex, &gus2ndEastEdgepointArraySize );
		}
	}
	//south
	if( gMapInformation.sSouthGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sSouthGridNo, SOUTH_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				&gps1stSouthEdgepointArray, &gus1stSouthEdgepointMiddleIndex, &gus1stSouthEdgepointArraySize,
				&gps2ndSouthEdgepointArray, &gus2ndSouthEdgepointMiddleIndex, &gus2ndSouthEdgepointArraySize );
		}
	}
	//west
	if( gMapInformation.sWestGridNo != -1 )
	{
		sGridNo = FindNearestEdgepointOnSpecifiedEdge( gMapInformation.sWestGridNo, WEST_EDGEPOINT_SEARCH );
		if( sGridNo != NOWHERE )
		{
			InternallyClassifyEdgepoints( &Soldier, sGridNo,
				&gps1stWestEdgepointArray, &gus1stWestEdgepointMiddleIndex, &gus1stWestEdgepointArraySize,
				&gps2ndWestEdgepointArray, &gus2ndWestEdgepointMiddleIndex, &gus2ndWestEdgepointArraySize );
		}
	}
}

void GenerateMapEdgepoints()
{
	INT32 i=-1;
	INT16 sGridNo=-1;
	INT16 sVGridNo[400];

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
			sVGridNo[ gus1stNorthEdgepointArraySize++ ] = sGridNo;
		while( sGridNo > gsTRGridNo )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stNorthEdgepointArraySize++ ] = sGridNo;
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stNorthEdgepointArraySize++ ] = sGridNo;
		}
		//2nd row
		gus1stNorthEdgepointMiddleIndex = gus1stNorthEdgepointArraySize;
		sGridNo = gsTLGridNo + 161;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			sVGridNo[ gus1stNorthEdgepointArraySize++ ] = sGridNo;
		while( sGridNo > gsTRGridNo + 161 )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stNorthEdgepointArraySize++ ] = sGridNo;
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stNorthEdgepointArraySize++ ] = sGridNo;
		}
		if( gus1stNorthEdgepointArraySize )
		{
			//Allocate and copy over the valid gridnos.
			gps1stNorthEdgepointArray = MALLOCN(INT16, gus1stNorthEdgepointArraySize);
			for( i = 0; i < gus1stNorthEdgepointArraySize; i++ )
				gps1stNorthEdgepointArray[ i ] = sVGridNo[ i ];
		}
	}
	//Calculate the east edges
	if( gMapInformation.sEastGridNo != -1 )
	{
		//1st row
		sGridNo = gsTRGridNo;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			sVGridNo[ gus1stEastEdgepointArraySize++ ] = sGridNo;
		while( sGridNo < gsBRGridNo )
		{
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stEastEdgepointArraySize++ ] = sGridNo;
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stEastEdgepointArraySize++ ] = sGridNo;
		}
		//2nd row
		gus1stEastEdgepointMiddleIndex = gus1stEastEdgepointArraySize;
		sGridNo = gsTRGridNo + 159;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			sVGridNo[ gus1stEastEdgepointArraySize++ ] = sGridNo;
		while( sGridNo < gsBRGridNo + 159 )
		{
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stEastEdgepointArraySize++ ] = sGridNo;
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stEastEdgepointArraySize++ ] = sGridNo;
		}
		if( gus1stEastEdgepointArraySize )
		{ //Allocate and copy over the valid gridnos.
			gps1stEastEdgepointArray = MALLOCN(INT16, gus1stEastEdgepointArraySize);
			for( i = 0; i < gus1stEastEdgepointArraySize; i++ )
				gps1stEastEdgepointArray[ i ] = sVGridNo[ i ];
		}
	}
	//Calculate the south edges
	if( gMapInformation.sSouthGridNo != -1 )
	{
		//1st row
		sGridNo = gsBLGridNo;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			sVGridNo[ gus1stSouthEdgepointArraySize++ ] = sGridNo;
		while( sGridNo > gsBRGridNo )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stSouthEdgepointArraySize++ ] = sGridNo;
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stSouthEdgepointArraySize++ ] = sGridNo;
		}
		//2nd row
		gus1stSouthEdgepointMiddleIndex = gus1stSouthEdgepointArraySize;
		sGridNo = gsBLGridNo - 161;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			sVGridNo[ gus1stSouthEdgepointArraySize++ ] = sGridNo;
		while( sGridNo > gsBRGridNo - 161 )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stSouthEdgepointArraySize++ ] = sGridNo;
			sGridNo -= 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stSouthEdgepointArraySize++ ] = sGridNo;
		}
		if( gus1stSouthEdgepointArraySize )
		{ //Allocate and copy over the valid gridnos.
			gps1stSouthEdgepointArray = MALLOCN(INT16, gus1stSouthEdgepointArraySize);
			for( i = 0; i < gus1stSouthEdgepointArraySize; i++ )
				gps1stSouthEdgepointArray[ i ] = sVGridNo[ i ];
		}
	}
	//Calculate the west edges
	if( gMapInformation.sWestGridNo != -1 )
	{
		//1st row
		sGridNo = gsTLGridNo;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			sVGridNo[ gus1stWestEdgepointArraySize++ ] = sGridNo;
		while( sGridNo < gsBLGridNo )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stWestEdgepointArraySize++ ] = sGridNo;
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stWestEdgepointArraySize++ ] = sGridNo;
		}
		//2nd row
		gus1stWestEdgepointMiddleIndex = gus1stWestEdgepointArraySize;
		sGridNo = gsTLGridNo - 159;
		if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
			 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
			sVGridNo[ gus1stWestEdgepointArraySize++ ] = sGridNo;
		while( sGridNo < gsBLGridNo - 159 )
		{
			sGridNo++;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stWestEdgepointArraySize++ ] = sGridNo;
			sGridNo += 160;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus1stWestEdgepointArraySize++ ] = sGridNo;
		}
		if( gus1stWestEdgepointArraySize )
		{ //Allocate and copy over the valid gridnos.
			gps1stWestEdgepointArray = MALLOCN(INT16, gus1stWestEdgepointArraySize);
			for( i = 0; i < gus1stWestEdgepointArraySize; i++ )
				gps1stWestEdgepointArray[ i ] = sVGridNo[ i ];
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
				sVGridNo[ gus2ndNorthEdgepointArraySize++ ] = sGridNo;
			while( sGridNo > gsTRGridNo )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndNorthEdgepointArraySize++ ] = sGridNo;
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndNorthEdgepointArraySize++ ] = sGridNo;
			}
			//2nd row
			gus2ndNorthEdgepointMiddleIndex = gus2ndNorthEdgepointArraySize;
			sGridNo = gsTLGridNo + 161;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus2ndNorthEdgepointArraySize++ ] = sGridNo;
			while( sGridNo > gsTRGridNo + 161 )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndNorthEdgepointArraySize++ ] = sGridNo;
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndNorthEdgepointArraySize++ ] = sGridNo;
			}
			if( gus2ndNorthEdgepointArraySize )
			{
				//Allocate and copy over the valid gridnos.
				gps2ndNorthEdgepointArray = MALLOCN(INT16, gus2ndNorthEdgepointArraySize);
				for( i = 0; i < gus2ndNorthEdgepointArraySize; i++ )
					gps2ndNorthEdgepointArray[ i ] = sVGridNo[ i ];
			}
		}
		//Calculate the east edges
		if( gMapInformation.sEastGridNo != -1 )
		{
			//1st row
			sGridNo = gsTRGridNo;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus2ndEastEdgepointArraySize++ ] = sGridNo;
			while( sGridNo < gsBRGridNo )
			{
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndEastEdgepointArraySize++ ] = sGridNo;
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndEastEdgepointArraySize++ ] = sGridNo;
			}
			//2nd row
			gus2ndEastEdgepointMiddleIndex = gus2ndEastEdgepointArraySize;
			sGridNo = gsTRGridNo + 159;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus2ndEastEdgepointArraySize++ ] = sGridNo;
			while( sGridNo < gsBRGridNo + 159 )
			{
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndEastEdgepointArraySize++ ] = sGridNo;
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndEastEdgepointArraySize++ ] = sGridNo;
			}
			if( gus2ndEastEdgepointArraySize )
			{ //Allocate and copy over the valid gridnos.
				gps2ndEastEdgepointArray = MALLOCN(INT16, gus2ndEastEdgepointArraySize);
				for( i = 0; i < gus2ndEastEdgepointArraySize; i++ )
					gps2ndEastEdgepointArray[ i ] = sVGridNo[ i ];
			}
		}
		//Calculate the south edges
		if( gMapInformation.sSouthGridNo != -1 )
		{
			//1st row
			sGridNo = gsBLGridNo;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus2ndSouthEdgepointArraySize++ ] = sGridNo;
			while( sGridNo > gsBRGridNo )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndSouthEdgepointArraySize++ ] = sGridNo;
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndSouthEdgepointArraySize++ ] = sGridNo;
			}
			//2nd row
			gus2ndSouthEdgepointMiddleIndex = gus2ndSouthEdgepointArraySize;
			sGridNo = gsBLGridNo - 161;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus2ndSouthEdgepointArraySize++ ] = sGridNo;
			while( sGridNo > gsBRGridNo - 161 )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndSouthEdgepointArraySize++ ] = sGridNo;
				sGridNo -= 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndSouthEdgepointArraySize++ ] = sGridNo;
			}
			if( gus2ndSouthEdgepointArraySize )
			{ //Allocate and copy over the valid gridnos.
				gps2ndSouthEdgepointArray = MALLOCN(INT16, gus2ndSouthEdgepointArraySize);
				for( i = 0; i < gus2ndSouthEdgepointArraySize; i++ )
					gps2ndSouthEdgepointArray[ i ] = sVGridNo[ i ];
			}
		}
		//Calculate the west edges
		if( gMapInformation.sWestGridNo != -1 )
		{
			//1st row
			sGridNo = gsTLGridNo;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus2ndWestEdgepointArraySize++ ] = sGridNo;
			while( sGridNo < gsBLGridNo )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndWestEdgepointArraySize++ ] = sGridNo;
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndWestEdgepointArraySize++ ] = sGridNo;
			}
			//2nd row
			gus2ndWestEdgepointMiddleIndex = gus2ndWestEdgepointArraySize;
			sGridNo = gsTLGridNo - 159;
			if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
				 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
				sVGridNo[ gus2ndWestEdgepointArraySize++ ] = sGridNo;
			while( sGridNo < gsBLGridNo - 159 )
			{
				sGridNo++;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndWestEdgepointArraySize++ ] = sGridNo;
				sGridNo += 160;
				if( gpWorldLevelData[ sGridNo ].uiFlags & MAPELEMENT_REACHABLE &&
					 (!gubWorldRoomInfo[ sGridNo ] || gfBasement) )
					sVGridNo[ gus2ndWestEdgepointArraySize++ ] = sGridNo;
			}
			if( gus2ndWestEdgepointArraySize )
			{ //Allocate and copy over the valid gridnos.
				gps2ndWestEdgepointArray = MALLOCN(INT16, gus2ndWestEdgepointArraySize);
				for( i = 0; i < gus2ndWestEdgepointArraySize; i++ )
					gps2ndWestEdgepointArray[ i ] = sVGridNo[ i ];
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


static void SaveMapEdgepoint(HWFILE const f, UINT16 const& n, UINT16 const& idx, INT16 const* const array)
{
	FileWrite(f, &n,   sizeof(n));
	FileWrite(f, &idx, sizeof(idx));
	if (n != 0) FileWrite(f, array,  sizeof(*array) * n);
}


void SaveMapEdgepoints(HWFILE const f)
{
	// 1st priority edgepoints -- for common entry -- tactical placement gui uses only these points.
	SaveMapEdgepoint(f, gus1stNorthEdgepointArraySize, gus1stNorthEdgepointMiddleIndex, gps1stNorthEdgepointArray);
	SaveMapEdgepoint(f, gus1stEastEdgepointArraySize,  gus1stEastEdgepointMiddleIndex,  gps1stEastEdgepointArray);
	SaveMapEdgepoint(f, gus1stSouthEdgepointArraySize, gus1stSouthEdgepointMiddleIndex, gps1stSouthEdgepointArray);
	SaveMapEdgepoint(f, gus1stWestEdgepointArraySize,  gus1stWestEdgepointMiddleIndex,  gps1stWestEdgepointArray);

	// 2nd priority edgepoints -- for isolated areas.  Okay to be zero
	SaveMapEdgepoint(f, gus2ndNorthEdgepointArraySize, gus2ndNorthEdgepointMiddleIndex, gps2ndNorthEdgepointArray);
	SaveMapEdgepoint(f, gus2ndEastEdgepointArraySize,  gus2ndEastEdgepointMiddleIndex,  gps2ndEastEdgepointArray);
	SaveMapEdgepoint(f, gus2ndSouthEdgepointArraySize, gus2ndSouthEdgepointMiddleIndex, gps2ndSouthEdgepointArray);
	SaveMapEdgepoint(f, gus2ndWestEdgepointArraySize,  gus2ndWestEdgepointMiddleIndex,  gps2ndWestEdgepointArray);
}


static void LoadMapEdgepoint(HWFILE const f, UINT16& n, UINT16& idx, INT16*& array)
{
	FileRead(f, &n,   sizeof(n));
	FileRead(f, &idx, sizeof(idx));
	if (n != 0)
	{
		array = MALLOCN(INT16, n);
		FileRead(f, array, sizeof(*array) * n);
	}
}


bool LoadMapEdgepoints(HWFILE const f)
{
	TrashMapEdgepoints();

	LoadMapEdgepoint(f, gus1stNorthEdgepointArraySize, gus1stNorthEdgepointMiddleIndex, gps1stNorthEdgepointArray);
	LoadMapEdgepoint(f, gus1stEastEdgepointArraySize,  gus1stEastEdgepointMiddleIndex,  gps1stEastEdgepointArray);
	LoadMapEdgepoint(f, gus1stSouthEdgepointArraySize, gus1stSouthEdgepointMiddleIndex, gps1stSouthEdgepointArray);
	LoadMapEdgepoint(f, gus1stWestEdgepointArraySize,  gus1stWestEdgepointMiddleIndex,  gps1stWestEdgepointArray);

	if (gMapInformation.ubMapVersion < 17)
	{	/* To prevent invalidation of older maps, which only used one layer of
		 * edgepoints, and a UINT8 for containing the size, we will preserve that
		 * paradigm, then kill the loaded edgepoints and regenerate them. */
		TrashMapEdgepoints();
		return false;
	}

	LoadMapEdgepoint(f, gus2ndNorthEdgepointArraySize, gus2ndNorthEdgepointMiddleIndex, gps2ndNorthEdgepointArray);
	LoadMapEdgepoint(f, gus2ndEastEdgepointArraySize,  gus2ndEastEdgepointMiddleIndex,  gps2ndEastEdgepointArray);
	LoadMapEdgepoint(f, gus2ndSouthEdgepointArraySize, gus2ndSouthEdgepointMiddleIndex, gps2ndSouthEdgepointArray);
	LoadMapEdgepoint(f, gus2ndWestEdgepointArraySize,  gus2ndWestEdgepointMiddleIndex,  gps2ndWestEdgepointArray);

	if (gMapInformation.ubMapVersion < 22)
	{	// Regenerate them
		TrashMapEdgepoints();
		return false;
	}

	return true;
}


UINT16 ChooseMapEdgepoint( UINT8 ubStrategicInsertionCode )
{
	INT16 *psArray=NULL;
	UINT16 usArraySize=0;

	//First validate and get access to the correct array based on strategic direction.
	//We will use the selected array to choose insertion gridno's.
	switch( ubStrategicInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			psArray = gps1stNorthEdgepointArray;
			usArraySize = gus1stNorthEdgepointArraySize;
			break;
		case INSERTION_CODE_EAST:
			psArray = gps1stEastEdgepointArray;
			usArraySize = gus1stEastEdgepointArraySize;
			break;
		case INSERTION_CODE_SOUTH:
			psArray = gps1stSouthEdgepointArray;
			usArraySize = gus1stSouthEdgepointArraySize;
			break;
		case INSERTION_CODE_WEST:
			psArray = gps1stWestEdgepointArray;
			usArraySize = gus1stWestEdgepointArraySize;
			break;
		default:
			AssertMsg( 0, "ChooseMapEdgepoints:  Failed to pass a valid strategic insertion code." );
			break;
	}
	if( !usArraySize )
	{
		return NOWHERE;
	}
	return psArray[ Random( usArraySize ) ];
}


void ChooseMapEdgepoints(MAPEDGEPOINTINFO* const pMapEdgepointInfo, const UINT8 ubStrategicInsertionCode, UINT8 ubNumDesiredPoints)
{
	AssertMsg(ubNumDesiredPoints > 0 && ubNumDesiredPoints <= 32, String("ChooseMapEdgepoints:  Desired points = %d, valid range is 1-32", ubNumDesiredPoints));

	/* First validate and get access to the correct array based on strategic
	 * direction.  We will use the selected array to choose insertion gridno's. */
	INT16* psArray;
	UINT16 usArraySize;
	switch (ubStrategicInsertionCode)
	{
		case INSERTION_CODE_NORTH:
			psArray     = gps1stNorthEdgepointArray;
			usArraySize = gus1stNorthEdgepointArraySize;
			break;

		case INSERTION_CODE_EAST:
			psArray     = gps1stEastEdgepointArray;
			usArraySize = gus1stEastEdgepointArraySize;
			break;

		case INSERTION_CODE_SOUTH:
			psArray     = gps1stSouthEdgepointArray;
			usArraySize = gus1stSouthEdgepointArraySize;
			break;

		case INSERTION_CODE_WEST:
			psArray     = gps1stWestEdgepointArray;
			usArraySize = gus1stWestEdgepointArraySize;
			break;

		default:
			AssertMsg(0, "ChooseMapEdgepoints:  Failed to pass a valid strategic insertion code.");
			psArray     = NULL;
			usArraySize = 0;
			break;
	}
	pMapEdgepointInfo->ubStrategicInsertionCode = ubStrategicInsertionCode;

#ifdef JA2BETAVERSION
	if (!psArray || usArraySize == 0)
	{
		if (gMapInformation.sNorthGridNo  == -1 &&
				gMapInformation.sEastGridNo   == -1 &&
				gMapInformation.sSouthGridNo  == -1 &&
				gMapInformation.sWestGridNo   == -1 &&
				gMapInformation.sCenterGridNo == -1)
		{
			if (gbWorldSectorZ)
			{
				AssertMsg(0, String("Map %c%d_b%d(_a) doesn't have ANY entry points which means that it is impossible to generate map edgepoints. (LC : 1)", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			}
			else
			{
				AssertMsg(0, String("Map %c%d(_a) doesn't have ANY entry points which means that it is impossible to generate map edgepoints. (LC : 1)", gWorldSectorY + 'A' - 1, gWorldSectorX));
			}
		}
		else switch (ubStrategicInsertionCode)
		{
			case INSERTION_CODE_NORTH: AssertMsg(0, "This map doesn't have any north mapedgepoints.  Possibly because there is no north entry point. (LC : 1)"); break;
			case INSERTION_CODE_EAST:  AssertMsg(0, "This map doesn't have any east mapedgepoints.  Possibly because there is no east entry point. (LC : 1)");   break;
			case INSERTION_CODE_SOUTH: AssertMsg(0, "This map doesn't have any south mapedgepoints.  Possibly because there is no south entry point. (LC : 1)"); break;
			case INSERTION_CODE_WEST:  AssertMsg(0, "This map doesn't have any west mapedgepoints.  Possibly because there is no west entry point.  NOTE:  Teleportation always uses the west entrypoint.  Some maps shouldn't have west entrypoints. (LC : 1)"); break;
		}
	}
#endif

	if (usArraySize == 0)
	{
		pMapEdgepointInfo->ubNumPoints = 0;
		return;
	}

	/* JA2 Gold: don't place people in the water.  If any of the waypoints is on a
	 * water spot, we're going to have to remove it */
	SGP::Buffer<INT16> psTempArray(usArraySize);
	size_t n_usable = 0;
	for (INT32 i = 0; i < usArraySize; ++i)
	{
		const UINT8 terrain = GetTerrainType(psArray[i]);
		if (terrain == MED_WATER || terrain == DEEP_WATER) continue;

		psTempArray[n_usable++] = psArray[i];
	}

	if (ubNumDesiredPoints >= n_usable)
	{ //We don't have enough points for everyone, return them all.
		pMapEdgepointInfo->ubNumPoints = n_usable;
		for (INT32 i = 0; i < n_usable; ++i)
		{
			pMapEdgepointInfo->sGridNo[i] = psTempArray[i];
		}
		return;
	}

	// We have more points, so choose them randomly.
	UINT16 usSlots    = n_usable;
	UINT16 usCurrSlot = 0;
	pMapEdgepointInfo->ubNumPoints = ubNumDesiredPoints;
	for (INT32 i = 0; i < n_usable; ++i)
	{
		if (Random(usSlots) < ubNumDesiredPoints)
		{
			pMapEdgepointInfo->sGridNo[usCurrSlot++] = psTempArray[i];
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
	gpReservedGridNos = MALLOCN(INT16, 20);
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
	MemFree( gpReservedGridNos );
	gpReservedGridNos = NULL;
	gsReservedIndex = 0;
}


//THIS CODE ISN'T RECOMMENDED FOR TIME CRITICAL AREAS.
INT16 SearchForClosestPrimaryMapEdgepoint( INT16 sGridNo, UINT8 ubInsertionCode )
{
	INT32 i, iDirectionLoop;
	INT16 *psArray=NULL;
	INT16 sRadius, sDistance, sDirection, sOriginalGridNo;
	UINT16 usArraySize=0;
	BOOLEAN fReserved;

	if( gsReservedIndex >= 20 )
	{ //Everything is reserved.
		AssertMsg( 0, "All closest map edgepoints have been reserved.  We should only have 20 soldiers maximum...");
	}
	switch( ubInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			psArray = gps1stNorthEdgepointArray;
			usArraySize = gus1stNorthEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any north mapedgepoints. LC:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
		case INSERTION_CODE_EAST:
			psArray = gps1stEastEdgepointArray;
			usArraySize = gus1stEastEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any east mapedgepoints. LC:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
		case INSERTION_CODE_SOUTH:
			psArray = gps1stSouthEdgepointArray;
			usArraySize = gus1stSouthEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any south mapedgepoints. LC:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
		case INSERTION_CODE_WEST:
			psArray = gps1stWestEdgepointArray;
			usArraySize = gus1stWestEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any west mapedgepoints. LC:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
	}
	if( !usArraySize )
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
		for( i = 0 ; i < usArraySize; i++ )
		{
			if( psArray[ i ] == sGridNo )
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
	while( sRadius < (INT16)(gbWorldSectorZ ? 30 : 10) )
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
					for( i = 0 ; i < usArraySize; i++ )
					{
						if( psArray[ i ] == sGridNo )
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
	INT16 *psArray=NULL;
	INT16 sRadius, sDistance, sDirection, sOriginalGridNo;
	UINT16 usArraySize=0;
	BOOLEAN fReserved;

	if( gsReservedIndex >= 20 )
	{ //Everything is reserved.
		AssertMsg( 0, "All closest map edgepoints have been reserved.  We should only have 20 soldiers maximum...");
	}
	switch( ubInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			psArray = gps2ndNorthEdgepointArray;
			usArraySize = gus2ndNorthEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any isolated north mapedgepoints. KM:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
		case INSERTION_CODE_EAST:
			psArray = gps2ndEastEdgepointArray;
			usArraySize = gus2ndEastEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any isolated east mapedgepoints. KM:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
		case INSERTION_CODE_SOUTH:
			psArray = gps2ndSouthEdgepointArray;
			usArraySize = gus2ndSouthEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any isolated south mapedgepoints. KM:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
		case INSERTION_CODE_WEST:
			psArray = gps2ndWestEdgepointArray;
			usArraySize = gus2ndWestEdgepointArraySize;
			AssertMsg(usArraySize != 0, String("Sector %c%d level %d doesn't have any isolated west mapedgepoints. KM:1", gWorldSectorY + 'A' - 1, gWorldSectorX, gbWorldSectorZ));
			break;
	}
	if( !usArraySize )
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
		for( i = 0 ; i < usArraySize; i++ )
		{
			if( psArray[ i ] == sGridNo )
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
	while( sRadius < (INT16)(gbWorldSectorZ ? 30 : 10) )
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
					for( i = 0 ; i < usArraySize; i++ )
					{
						if( psArray[ i ] == sGridNo )
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
	sMaxLeft  = MIN( iSearchRange, (pSoldier->sGridNo % MAXCOL));
	sMaxRight = MIN( iSearchRange, MAXCOL - ((pSoldier->sGridNo % MAXCOL) + 1));

	// determine maximum vertical limits
	sMaxUp   = MIN( iSearchRange, (pSoldier->sGridNo / MAXROW));
	sMaxDown = MIN( iSearchRange, MAXROW - ((pSoldier->sGridNo / MAXROW) + 1));

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

	if( gWorldSectorX == 14 && gWorldSectorY == 9 && !gbWorldSectorZ )
	{ //BRUTAL CODE  -- special case map.
		iSearchRange = 250;
	}
	else
	{
		iSearchRange = EDGE_OF_MAP_SEARCH;
	}

	// determine maximum horizontal limits
	sMaxLeft  = MIN( iSearchRange, (pSoldier->sGridNo % MAXCOL));
	sMaxRight = MIN( iSearchRange, MAXCOL - ((pSoldier->sGridNo % MAXCOL) + 1));

	// determine maximum vertical limits
	sMaxUp   = MIN( iSearchRange, (pSoldier->sGridNo / MAXROW));
	sMaxDown = MIN( iSearchRange, MAXROW - ((pSoldier->sGridNo / MAXROW) + 1));

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
	INT32			iLoop;
	INT16			*psEdgepointArray1, *psEdgepointArray2;
	INT32			iEdgepointArraySize1, iEdgepointArraySize2;
	INT16			sClosestSpot1 = NOWHERE, sClosestDist1 = 0x7FFF, sTempDist;
	INT16			sClosestSpot2 = NOWHERE, sClosestDist2 = 0x7FFF;
	BOOLEAN		fPrimaryValid = FALSE, fSecondaryValid = FALSE;

	memset( &Soldier, 0, sizeof( SOLDIERTYPE ) );
	Soldier.bTeam = 1;
	Soldier.sGridNo = sGridNo;

  if( gMapInformation.sIsolatedGridNo == -1 )
	{ //If the map has no isolated area, then all edgepoints are primary.
		return INSERTION_CODE_PRIMARY_EDGEINDEX;
	}

	switch( gubTacticalDirection )
	{
		case NORTH:
			psEdgepointArray1 = gps1stNorthEdgepointArray;
			iEdgepointArraySize1 = gus1stNorthEdgepointArraySize;
			psEdgepointArray2 = gps2ndNorthEdgepointArray;
			iEdgepointArraySize2 = gus2ndNorthEdgepointArraySize;
			break;
		case EAST:
			psEdgepointArray1 = gps1stEastEdgepointArray;
			iEdgepointArraySize1 = gus1stEastEdgepointArraySize;
			psEdgepointArray2 = gps2ndEastEdgepointArray;
			iEdgepointArraySize2 = gus2ndEastEdgepointArraySize;
			break;
		case SOUTH:
			psEdgepointArray1 = gps1stSouthEdgepointArray;
			iEdgepointArraySize1 = gus1stSouthEdgepointArraySize;
			psEdgepointArray2 = gps2ndSouthEdgepointArray;
			iEdgepointArraySize2 = gus2ndSouthEdgepointArraySize;
			break;
		case WEST:
			psEdgepointArray1 = gps1stWestEdgepointArray;
			iEdgepointArraySize1 = gus1stWestEdgepointArraySize;
			psEdgepointArray2 = gps2ndWestEdgepointArray;
			iEdgepointArraySize2 = gus2ndWestEdgepointArraySize;
			break;
		default:
			// WTF???
			return INSERTION_CODE_PRIMARY_EDGEINDEX;
	}

	// Do a 2D search to find the closest map edgepoint and
	// try to create a path there
	for ( iLoop = 0; iLoop < iEdgepointArraySize1; iLoop++ )
	{
		sTempDist = PythSpacesAway( sGridNo, psEdgepointArray1[ iLoop ] );
		if ( sTempDist < sClosestDist1 )
		{
			sClosestDist1 = sTempDist;
			sClosestSpot1 = psEdgepointArray1[ iLoop ];
		}
	}
	for ( iLoop = 0; iLoop < iEdgepointArraySize2; iLoop++ )
	{
		sTempDist = PythSpacesAway( sGridNo, psEdgepointArray2[ iLoop ] );
		if ( sTempDist < sClosestDist2 )
		{
			sClosestDist2 = sTempDist;
			sClosestSpot2 = psEdgepointArray2[ iLoop ];
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


static bool ShowMapEdgepoint(UINT16 const n, INT16 const* const array, UINT16 const idx)
{
	INT32              n_illegal = 0;
	INT16 const* const end       = array + n;
	for (INT16 const* i = array; i != end; ++i)
	{
		if (*i != -1)
		{
			AddTopmostToTail(*i, idx);
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
	n_illegal1 += ShowMapEdgepoint(gus1stNorthEdgepointArraySize, gps1stNorthEdgepointArray, FIRSTPOINTERS5);
	n_illegal1 += ShowMapEdgepoint(gus1stEastEdgepointArraySize,  gps1stEastEdgepointArray,  FIRSTPOINTERS5);
	n_illegal1 += ShowMapEdgepoint(gus1stSouthEdgepointArraySize, gps1stSouthEdgepointArray, FIRSTPOINTERS5);
	n_illegal1 += ShowMapEdgepoint(gus1stWestEdgepointArraySize,  gps1stWestEdgepointArray,  FIRSTPOINTERS5);

	INT32 n_illegal2 = 0;
	n_illegal2 += ShowMapEdgepoint(gus2ndNorthEdgepointArraySize, gps2ndNorthEdgepointArray, FIRSTPOINTERS6);
	n_illegal2 += ShowMapEdgepoint(gus2ndEastEdgepointArraySize,  gps2ndEastEdgepointArray,  FIRSTPOINTERS6);
	n_illegal2 += ShowMapEdgepoint(gus2ndSouthEdgepointArraySize, gps2ndSouthEdgepointArray, FIRSTPOINTERS6);
	n_illegal2 += ShowMapEdgepoint(gus2ndWestEdgepointArraySize,  gps2ndWestEdgepointArray,  FIRSTPOINTERS6);

	if (n_illegal1 == 0 && n_illegal2 == 0)
	{
		SLOGD(DEBUG_TAG_SMAP, "Showing display of map edgepoints");
	}
	else
	{
		SLOGD(DEBUG_TAG_SMAP, "Showing display of map edgepoints (%d illegal primary, %d illegal secondary)", n_illegal1, n_illegal2);
	}
	SLOGD(DEBUG_TAG_SMAP, "N:%d:%d E:%d:%d S:%d:%d W:%d:%d",
			gus1stNorthEdgepointArraySize, gus2ndNorthEdgepointArraySize,
			gus1stEastEdgepointArraySize,  gus2ndEastEdgepointArraySize,
			gus1stSouthEdgepointArraySize, gus2ndSouthEdgepointArraySize,
			gus1stWestEdgepointArraySize,	 gus2ndWestEdgepointArraySize);
}


static void HideMapEdgepoint(UINT16 const n, INT16 const* const array)
{
	INT16 const* const end = array + n;
	for (INT16 const* i = array; i != end; ++i)
	{
		if (*i == -1) continue;
		RemoveAllTopmostsOfTypeRange(*i, FIRSTPOINTERS, FIRSTPOINTERS);
	}
}


void HideMapEdgepoints()
{
	SLOGD(DEBUG_TAG_SMAP, "Removing display of map edgepoints");

	HideMapEdgepoint(gus1stNorthEdgepointArraySize, gps1stNorthEdgepointArray);
	HideMapEdgepoint(gus1stEastEdgepointArraySize,  gps1stEastEdgepointArray);
	HideMapEdgepoint(gus1stSouthEdgepointArraySize, gps1stSouthEdgepointArray);
	HideMapEdgepoint(gus1stWestEdgepointArraySize,  gps1stWestEdgepointArray);

	HideMapEdgepoint(gus2ndNorthEdgepointArraySize, gps2ndNorthEdgepointArray);
	HideMapEdgepoint(gus2ndEastEdgepointArraySize,  gps2ndEastEdgepointArray);
	HideMapEdgepoint(gus2ndSouthEdgepointArraySize, gps2ndSouthEdgepointArray);
	HideMapEdgepoint(gus2ndWestEdgepointArraySize,  gps2ndWestEdgepointArray);
}
