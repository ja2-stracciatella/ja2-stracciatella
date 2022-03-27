#ifndef __STRATMAP_H
#define __STRATMAP_H
// this file is for manipulation of the strategic map structure

#include "JA2Types.h"
#include "SAM_Sites.h"

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
extern SGPSector gWorldSector;

#define NO_SECTOR ((UINT)-1)

UINT GetWorldSector();

static inline void SetWorldSectorInvalid()
{
	gWorldSector = SGPSector(0, 0, -1);
}

extern	BOOLEAN		gfUseAlternateMap;

// This is called after loading map and before setting it up (e.g. placing soldiers)
extern Observable<> BeforePrepareSector;

// FUNCTIONS FOR DERTERMINING GOOD SECTOR EXIT DATA
#define CHECK_DIR_X_DELTA			( WORLD_TILE_X * 4 )
#define CHECK_DIR_Y_DELTA			( WORLD_TILE_Y * 10 )

#define MAP_WORLD_X				18
#define MAP_WORLD_Y				18

// get index into aray
#define GET_X_FROM_STRATEGIC_INDEX( i )		( i % MAP_WORLD_X )
#define GET_Y_FROM_STRATEGIC_INDEX( i )		( i / MAP_WORLD_X )

// macros to convert between the 2 different sector numbering systems
#define STRATEGIC_INDEX_TO_SECTOR_INFO( i )	( SGPSector::FromStrategicIndex(i).AsByte() )


// grab the town id value
UINT8 GetTownIdForSector(UINT8 sector);

void SetCurrentWorldSector(const SGPSector& sector);

void UpdateMercsInSector();
void UpdateMercInSector(SOLDIERTYPE&, const SGPSector& sector);

// Return a string like 'A9: Omerta'
ST::string GetSectorIDString(const SGPSector& sector, BOOLEAN detailed);

// Returns a sector description string based on the sector land type
ST::string GetSectorLandTypeString(UINT8 ubSectorID, UINT8 ubSectorZ, bool fDetailed);

ST::string GetMapFileName(const SGPSector& sector, BOOLEAN add_alternate_map_letter);

// Called from within tactical.....
void JumpIntoAdjacentSector( UINT8 ubDirection, UINT8 ubJumpCode, INT16 sAdditionalData );


bool CanGoToTacticalInSector(const SGPSector& sector);

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


void HandleQuestCodeOnSectorEntry(const SGPSector& sector);

// handle a soldier leaving thier squad behind, this sets them up for mvt and potential rejoining of group
void HandleSoldierLeavingSectorByThemSelf( SOLDIERTYPE *pSoldier );

BOOLEAN CheckAndHandleUnloadingOfCurrentWorld(void);

bool IsSectorDesert(const SGPSector& sector);

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
