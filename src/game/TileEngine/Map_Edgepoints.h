#ifndef __MAP_EDGEPOINTS_H
#define __MAP_EDGEPOINTS_H

#include "JA2Types.h"

#include <vector>

struct MAPEDGEPOINTINFO
{
	UINT8 ubNumPoints;
	UINT8 ubStrategicInsertionCode;
	UINT16 sGridNo[ 32 ];
};

GridNo ChooseMapEdgepoint(UINT8 ubStrategicInsertionCode);
void ChooseMapEdgepoints( MAPEDGEPOINTINFO *pMapEdgepointInfo, UINT8 ubStrategicInsertionCode, UINT8 ubNumDesiredPoints );
void GenerateMapEdgepoints(void);

void SaveMapEdgepoints(HWFILE);

bool LoadMapEdgepoints(HWFILE);
void TrashMapEdgepoints(void);

//dynamic arrays that contain the valid gridno's for each edge
extern std::vector<INT16> gps1stNorthEdgepointArray;
extern std::vector<INT16> gps1stEastEdgepointArray;
extern std::vector<INT16> gps1stSouthEdgepointArray;
extern std::vector<INT16> gps1stWestEdgepointArray;
//contains the index value for the first array index of the second row of each edgepoint array.
//Because each edgepoint side has two rows, the outside most row is calculated first, then the inside row.
//For purposes of AI, it may become necessary to avoid this.
extern UINT16 gus1stNorthEdgepointMiddleIndex;
extern UINT16 gus1stEastEdgepointMiddleIndex;
extern UINT16 gus1stSouthEdgepointMiddleIndex;
extern UINT16 gus1stWestEdgepointMiddleIndex;

//dynamic arrays that contain the valid gridno's for each edge
extern std::vector<INT16> gps2ndNorthEdgepointArray;
extern std::vector<INT16> gps2ndEastEdgepointArray;
extern std::vector<INT16> gps2ndSouthEdgepointArray;
extern std::vector<INT16> gps2ndWestEdgepointArray;
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
GridNo SearchForClosestPrimaryMapEdgepoint(GridNo sGridNo, UINT8 ubInsertionCode);
GridNo SearchForClosestSecondaryMapEdgepoint(GridNo sGridNo, UINT8 ubInsertionCode);

//There are two classes of edgepoints.
//PRIMARY		: The default list of edgepoints.  This list includes edgepoints that are easily accessible from the
//						strategic level.
//SECONDARY	: The isolated list of edgepoints.  Certain areas of the game are blocked off, but tactically traversing
//						to these areas is possible.  Examples would be isolated sections of Grumm or Alma, which you can't
//						immediately
//
UINT8 CalcMapEdgepointClassInsertionCode(GridNo sGridNo);

void ShowMapEdgepoints(void);
void HideMapEdgepoints(void);


#endif
