#ifndef __STRATMAP_H
#define __STRATMAP_H
// this file is for manipulation of the strategic map structure

#include "JA2Types.h"

#include <string_theory/string>


//The maximum size for any team strategically speaking.  For example, we can't have more than 20 enemies, militia, or creatures at a time.
#define MAX_STRATEGIC_TEAM_SIZE	20

// Codes for jumoing into adjacent sectors..
enum
{
	JUMP_ALL_LOAD_NEW,
	JUMP_ALL_NO_LOAD,
	JUMP_SINGLE_LOAD_NEW,
	JUMP_SINGLE_NO_LOAD
};


enum{
	CONTROLLED=0,
	UNCONTROLLED,
};

// For speed, etc lets make these globals, forget the functions if you want
extern INT16 gWorldSectorX;
extern INT16 gWorldSectorY;
extern INT8  gbWorldSectorZ;

#define NO_SECTOR ((UINT)-1)

UINT GetWorldSector();

static inline void SetWorldSectorInvalid()
{
	gWorldSectorX  =  0;
	gWorldSectorY  =  0;
	gbWorldSectorZ = -1;
}

#define NUMBER_OF_SAMS 4

extern BOOLEAN fFoundOrta;
extern BOOLEAN fSamSiteFound[ NUMBER_OF_SAMS ];

extern	BOOLEAN		gfUseAlternateMap;


// min condition for sam site to be functional
#define MIN_CONDITION_FOR_SAM_SITE_TO_WORK 80


// FUNCTIONS FOR DERTERMINING GOOD SECTOR EXIT DATA
#define CHECK_DIR_X_DELTA			( WORLD_TILE_X * 4 )
#define CHECK_DIR_Y_DELTA			( WORLD_TILE_Y * 10 )

#define MAP_WORLD_X				18
#define MAP_WORLD_Y				18

// get index into aray
#define CALCULATE_STRATEGIC_INDEX( x, y )	( x + ( y * MAP_WORLD_X ) )
#define GET_X_FROM_STRATEGIC_INDEX( i )		( i % MAP_WORLD_X )
#define GET_Y_FROM_STRATEGIC_INDEX( i )		( i / MAP_WORLD_X )

// macros to convert between the 2 different sector numbering systems
#define SECTOR_INFO_TO_STRATEGIC_INDEX( i )	( CALCULATE_STRATEGIC_INDEX ( SECTORX( i ), SECTORY( i ) ) )
#define STRATEGIC_INDEX_TO_SECTOR_INFO( i )	( SECTOR(  GET_X_FROM_STRATEGIC_INDEX( i ), GET_Y_FROM_STRATEGIC_INDEX( i ) ) )


// grab the town id value
UINT8 GetTownIdForSector(UINT8 sector);

void SetCurrentWorldSector(INT16 x, INT16 y, INT8 z);

void UpdateMercsInSector();
void UpdateMercInSector(SOLDIERTYPE&, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);

// get short sector name without town name
ST::string GetShortSectorString(INT16 sMapX, INT16 sMapY);

// Return a string like 'A9: Omerta'
ST::string GetSectorIDString(INT16 x, INT16 y, INT8 z, BOOLEAN detailed);

void GetMapFileName(INT16 x, INT16 y, INT8 z, char* buf, BOOLEAN add_alternate_map_letter);

// Called from within tactical.....
void JumpIntoAdjacentSector( UINT8 ubDirection, UINT8 ubJumpCode, INT16 sAdditionalData );


bool CanGoToTacticalInSector(INT16 x, INT16 y, UINT8 z);

void UpdateAirspaceControl( void );

bool IsThisSectorASAMSector(INT16 x, INT16 y, INT8 z);

void InitializeSAMSites();

// Number of sectors this town takes up
UINT8 GetTownSectorSize(INT8 town_id);

// Number of sectors under player control for this town
UINT8 GetTownSectorsUnderControl(INT8 town_id);


BOOLEAN OKForSectorExit(INT8 bExitDirection, UINT16 usAdditionalData, UINT32* puiTraverseTimeInMinutes = 0);
void SetupNewStrategicGame();

void LoadStrategicInfoFromSavedFile(HWFILE);
void SaveStrategicInfoToSavedFile(HWFILE);

void AllMercsHaveWalkedOffSector(void);

void AdjustSoldierPathToGoOffEdge( SOLDIERTYPE *pSoldier, INT16 sEndGridNo, UINT8 ubTacticalDirection );

void AllMercsWalkedToExitGrid(void);

void PrepareLoadedSector(void);

// handle for slay...no better place to really put this stuff
void HandleSlayDailyEvent( void );


void HandleQuestCodeOnSectorEntry( INT16 sNewSectorX, INT16 sNewSectorY, INT8 bNewSectorZ );

// handle a soldier leaving thier squad behind, this sets them up for mvt and potential rejoining of group
void HandleSoldierLeavingSectorByThemSelf( SOLDIERTYPE *pSoldier );

BOOLEAN CheckAndHandleUnloadingOfCurrentWorld(void);

// number of SAM sites under player control
INT32 GetNumberOfSAMSitesUnderPlayerControl();

// is there a FUNCTIONAL SAM site in this sector?
bool IsThereAFunctionalSAMSiteInSector(INT16 x, INT16 y, INT8 z);

bool IsSectorDesert(INT16 x, INT16 y);

INT8 GetSAMIdFromSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );

void SetupProfileInsertionDataForSoldier(const SOLDIERTYPE* s);

BOOLEAN HandlePotentialBringUpAutoresolveToFinishBattle(void);

void BeginLoadScreen(void);

void RemoveMercsInSector(void);

void InitStrategicEngine(void);

//Used for determining the type of error message that comes up when you can't traverse to
//an adjacent sector.  THESE VALUES DO NOT NEED TO BE SAVED!
extern BOOLEAN	gfInvalidTraversal;
extern BOOLEAN	gfLoneEPCAttemptingTraversal;
extern BOOLEAN	gfRobotWithoutControllerAttemptingTraversal;
extern UINT8		gubLoneMercAttemptingToAbandonEPCs;
extern const SOLDIERTYPE* gPotentiallyAbandonedEPC;

extern INT8 gbGreenToElitePromotions;
extern INT8 gbGreenToRegPromotions;
extern INT8 gbRegToElitePromotions;
extern INT8 gbMilitiaPromotions;

extern BOOLEAN gfGettingNameFromSaveLoadScreen;

struct StrategicMapElement
{
	INT8    bNameId;
	BOOLEAN fEnemyControlled;   // enemy controlled or not
	BOOLEAN fEnemyAirControlled;
	INT8    bSAMCondition; // SAM Condition .. 0 - 100, just like an item's status
};

extern StrategicMapElement StrategicMap[MAP_WORLD_X * MAP_WORLD_Y];

#endif
