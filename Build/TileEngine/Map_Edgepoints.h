#ifndef __MAP_EDGEPOINTS_H
#define __MAP_EDGEPOINTS_H

#include "Types.h"

struct MAPEDGEPOINTINFO
{
	UINT8 ubNumPoints;
	UINT8 ubStrategicInsertionCode;
	UINT16 sGridNo[ 32 ];
};

UINT16 ChooseMapEdgepoint( UINT8 ubStrategicInsertionCode );
void ChooseMapEdgepoints( MAPEDGEPOINTINFO *pMapEdgepointInfo, UINT8 ubStrategicInsertionCode, UINT8 ubNumDesiredPoints );
void GenerateMapEdgepoints(void);

#ifdef JA2EDITOR
void SaveMapEdgepoints(HWFILE);
#endif

bool LoadMapEdgepoints(INT8** buf);
void TrashMapEdgepoints(void);

//dynamic arrays that contain the valid gridno's for each edge
extern INT16 *gps1stNorthEdgepointArray;
extern INT16 *gps1stEastEdgepointArray;
extern INT16 *gps1stSouthEdgepointArray;
extern INT16 *gps1stWestEdgepointArray;
//contains the size for each array
extern UINT16 gus1stNorthEdgepointArraySize;
extern UINT16 gus1stEastEdgepointArraySize;
extern UINT16 gus1stSouthEdgepointArraySize;
extern UINT16 gus1stWestEdgepointArraySize;
//contains the index value for the first array index of the second row of each edgepoint array.
//Because each edgepoint side has two rows, the outside most row is calculated first, then the inside row.
//For purposes of AI, it may become necessary to avoid this.
extern UINT16 gus1stNorthEdgepointMiddleIndex;
extern UINT16 gus1stEastEdgepointMiddleIndex;
extern UINT16 gus1stSouthEdgepointMiddleIndex;
extern UINT16 gus1stWestEdgepointMiddleIndex;

//dynamic arrays that contain the valid gridno's for each edge
extern INT16 *gps2ndNorthEdgepointArray;
extern INT16 *gps2ndEastEdgepointArray;
extern INT16 *gps2ndSouthEdgepointArray;
extern INT16 *gps2ndWestEdgepointArray;
//contains the size for each array
extern UINT16 gus2ndNorthEdgepointArraySize;
extern UINT16 gus2ndEastEdgepointArraySize;
extern UINT16 gus2ndSouthEdgepointArraySize;
extern UINT16 gus2ndWestEdgepointArraySize;
//contains the index value for the first array index of the second row of each edgepoint array.
//Because each edgepoint side has two rows, the outside most row is calculated first, then the inside row.
//For purposes of AI, it may become necessary to avoid this.
extern UINT16 gus2ndNorthEdgepointMiddleIndex;
extern UINT16 gus2ndEastEdgepointMiddleIndex;
extern UINT16 gus2ndSouthEdgepointMiddleIndex;
extern UINT16 gus2ndWestEdgepointMiddleIndex;

//This is the search code that will determine the closest map edgepoint to a given gridno and
//a strategic insertion code.  You must call the begin...() first, the call SearchForClosestMapEdgepoint()
//for each soldier that is looking.  After you are completely finished, call the end..().  The return value
//will return NOWHERE if the information is invalid (no edgepoint info for that particular side, or all
//spots are reserved.  There are only 20 spots that can be reserved (one for each player soldier).  This
//code shouldn't be used for enemies or anybody else.
void BeginMapEdgepointSearch(void);
void EndMapEdgepointSearch(void);
INT16 SearchForClosestPrimaryMapEdgepoint( INT16 sGridNo, UINT8 ubInsertionCode );
INT16 SearchForClosestSecondaryMapEdgepoint( INT16 sGridNo, UINT8 ubInsertionCode );

//There are two classes of edgepoints.
//PRIMARY		: The default list of edgepoints.  This list includes edgepoints that are easily accessible from the
//						strategic level.
//SECONDARY	: The isolated list of edgepoints.  Certain areas of the game are blocked off, but tactically traversing
//						to these areas is possible.  Examples would be isolated sections of Grumm or Alma, which you can't
//						immediately
//
UINT8 CalcMapEdgepointClassInsertionCode( INT16 sGridNo );

#ifdef JA2EDITOR
void ShowMapEdgepoints(void);
void HideMapEdgepoints(void);
#endif



#endif
