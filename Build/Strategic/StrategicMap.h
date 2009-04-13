#ifndef __STRATMAP_H
#define __STRATMAP_H
// this file is for manipulation of the strategic map structure

#include "JA2Types.h"


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

#define NUMBER_OF_SAMS 4


extern INT16 pSamList[ NUMBER_OF_SAMS ];
extern INT16 pSamGridNoAList[ NUMBER_OF_SAMS ];
extern INT16 pSamGridNoBList[ NUMBER_OF_SAMS ];

extern BOOLEAN fFoundOrta;
extern BOOLEAN fSamSiteFound[ NUMBER_OF_SAMS ];

extern	BOOLEAN		gfUseAlternateMap;


// SAM sites
#define SAM_1_X 2
#define SAM_2_X 15
#define SAM_3_X 8
#define SAM_4_X 4

#define SAM_1_Y 4
#define SAM_2_Y 4
#define SAM_3_Y 9
#define SAM_4_Y 14

// min condition for sam site to be functional
#define MIN_CONDITION_FOR_SAM_SITE_TO_WORK 80


// FUNCTIONS FOR DERTERMINING GOOD SECTOR EXIT DATA
#define			CHECK_DIR_X_DELTA							( WORLD_TILE_X * 4 )
#define			CHECK_DIR_Y_DELTA							( WORLD_TILE_Y * 10 )

#define MAP_WORLD_X 18
#define MAP_WORLD_Y 18

// get index into aray
#define		CALCULATE_STRATEGIC_INDEX( x, y )			( x + ( y * MAP_WORLD_X ) )
#define   GET_X_FROM_STRATEGIC_INDEX( i )				( i % MAP_WORLD_X )
#define   GET_Y_FROM_STRATEGIC_INDEX( i )				( i / MAP_WORLD_X )

// macros to convert between the 2 different sector numbering systems
#define		SECTOR_INFO_TO_STRATEGIC_INDEX( i )		( CALCULATE_STRATEGIC_INDEX ( SECTORX( i ), SECTORY( i ) ) )
#define		STRATEGIC_INDEX_TO_SECTOR_INFO( i )		( SECTOR(  GET_X_FROM_STRATEGIC_INDEX( i ), GET_Y_FROM_STRATEGIC_INDEX( i ) ) )


// grab the town id value
UINT8 GetTownIdForSector( INT16 sMapX, INT16 sMapY );

void GetCurrentWorldSector( INT16 *psMapX, INT16 *psMapY );
void SetCurrentWorldSector(INT16 x, INT16 y, INT8 z);

void UpdateMercsInSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );
void UpdateMercInSector( SOLDIERTYPE *pSoldier, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );

// get short sector name without town name
void GetShortSectorString(INT16 sMapX, INT16 sMapY, wchar_t* sString, size_t Length);

// This will get an ID string like A9- OMERTA...
void GetSectorIDString( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ , CHAR16 *zString, size_t Length, BOOLEAN fDetailed );

void GetMapFileName(INT16 sMapX, INT16 sMapY, INT8 bSectorZ, char* bString, BOOLEAN fUsePlaceholder, BOOLEAN fAddAlternateMapLetter);

// Called from within tactical.....
void JumpIntoAdjacentSector( UINT8 ubDirection, UINT8 ubJumpCode, INT16 sAdditionalData );



BOOLEAN CanGoToTacticalInSector( INT16 sX, INT16 sY, UINT8 ubZ );

void UpdateAirspaceControl( void );

BOOLEAN IsThisSectorASAMSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );

// init sam sites
void InitializeSAMSites( void );

// get town sector size
UINT8 GetTownSectorSize( INT8 bTownId );

// get town sector size, that is under play control
UINT8 GetTownSectorsUnderControl( INT8 bTownId );


BOOLEAN OKForSectorExit( INT8 bExitDirection, UINT16 usAdditionalData, UINT32 *puiTraverseTimeInMinutes );
void SetupNewStrategicGame(void);

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
INT32 GetNumberOfSAMSitesUnderPlayerControl( void );

// is there a FUNCTIONAL SAM site in this sector?
BOOLEAN IsThereAFunctionalSAMSiteInSector( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );

BOOLEAN IsSectorDesert( INT16 sSectorX, INT16 sSectorY );

// sam site under players control?
INT32 SAMSitesUnderPlayerControl( INT16 sX, INT16 sY );

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
