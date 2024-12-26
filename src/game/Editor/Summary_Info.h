#ifndef __SUMMARY_INFO_H
#define __SUMMARY_INFO_H

#include "Exit_Grids.h"
#include "Map_Information.h"
#include "Types.h"

#define GLOBAL_SUMMARY_VERSION	14
#define MINIMUMVERSION		7

struct TEAMSUMMARY
{
	UINT8 ubTotal;
	UINT8 ubDetailed;
	UINT8 ubProfile;
	UINT8	ubExistance;
	UINT8 ubNumAnimals;
	UINT8 ubBadA, ubPoorA, ubAvgA, ubGoodA, ubGreatA; //attributes
	UINT8 ubBadE, ubPoorE, ubAvgE, ubGoodE, ubGreatE; //equipment
}; //15 bytes

struct SUMMARYFILE
{
	//start version 1
	UINT8 ubSummaryVersion;
	UINT8 ubSpecial;
	UINT16 usNumItems;
	UINT16 usNumLights;
	MAPCREATE_STRUCT MapInfo;
	TEAMSUMMARY EnemyTeam;
	TEAMSUMMARY CreatureTeam;
	TEAMSUMMARY RebelTeam;
	TEAMSUMMARY CivTeam;
	UINT8 ubNumDoors;
	UINT8 ubNumDoorsLocked;
	UINT8 ubNumDoorsTrapped;
	UINT8 ubNumDoorsLockedAndTrapped;
	//start version 2
	UINT8 ubTilesetID;
	UINT8 ubNumRooms;
	//start version	3
	UINT8 ubNumElites;
	UINT8 ubNumAdmins;
	UINT8 ubNumTroops;
	//start version 4
	UINT8 ubEliteDetailed;
	UINT8 ubAdminDetailed;
	UINT8 ubTroopDetailed;
	//start version 5
	UINT8 ubEliteProfile;
	UINT8 ubAdminProfile;
	UINT8 ubTroopProfile;
	//start version 6
	UINT8 ubEliteExistance;
	UINT8 ubAdminExistance;
	UINT8 ubTroopExistance;
	//start version 7
	float dMajorMapVersion;
	//start version 8
	UINT8 ubCivSchedules;
	//start version 9
	UINT8 ubCivCows;
	UINT8 ubCivBloodcats;
	//					//-----
	//					// 190
	//start version 10
	EXITGRID ExitGrid[4];			//  20  (5*4)
	UINT16  usExitGridSize[4];		//   8   (2*4)
	BOOLEAN fInvalidDest[4];		//   4
	UINT8   ubNumExitGridDests;		//   1
	BOOLEAN fTooManyExitGridDests;		//   1
	//					//-----
	//					// 224
	//start version 11
	UINT8   ubEnemiesReqWaypoints;		//   1
	//					//-----
	//					// 225
	//start version 12
	UINT16 usWarningRoomNums;		//   2
	//					// 227
	//start version 13
	UINT8 ubEnemiesHaveWaypoints;		//   1
	UINT32 uiNumItemsPosition;		//   4
	//					//-----
	//					// 232
	//start version 14
	UINT32 uiEnemyPlacementPosition;	//   4
	//					//-----
	//					// 236


	UINT8 ubPadding[164];			// 164 // XXX HACK000B
	//					//-----
	//					// 400 total bytes
};


extern BOOLEAN gfAutoLoadA9;

extern BOOLEAN EvaluateWorld(const ST::string& pSector, UINT8 ubLevel);
void WriteSectorSummaryUpdate(const ST::string &filename, UINT8 ubLevel, SUMMARYFILE*);

extern BOOLEAN gfMustForceUpdateAllMaps;
extern BOOLEAN gfMajorUpdate;
void ApologizeOverrideAndForceUpdateEverything(void);

#endif
