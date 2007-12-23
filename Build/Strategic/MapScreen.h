#ifndef __MAPSCREEN_H
#define __MAPSCREEN_H

#include "Item_Types.h"
#include "JA2Types.h"
#include "MouseSystem.h"


#define			MAP_WORLD_X						18
#define			MAP_WORLD_Y						18


// Sector name identifiers
enum {
  BLANK_SECTOR=0,
	OMERTA,
	DRASSEN,
	ALMA,
	GRUMM,
	TIXA,
	CAMBRIA,
	SAN_MONA,
	ESTONI,
	ORTA,
	BALIME,
	MEDUNA,
  CHITZENA,
	NUM_TOWNS
} Towns;

#define FIRST_TOWN	OMERTA
//#define PALACE			NUM_TOWNS


extern BOOLEAN fCharacterInfoPanelDirty;
extern BOOLEAN fTeamPanelDirty;
extern BOOLEAN fMapPanelDirty;

extern BOOLEAN fMapInventoryItem;
extern BOOLEAN gfInConfirmMapMoveMode;
extern BOOLEAN gfInChangeArrivalSectorMode;

extern BOOLEAN gfSkyriderEmptyHelpGiven;


BOOLEAN SetInfoChar(const SOLDIERTYPE* s);
void EndMapScreen( BOOLEAN fDuringFade );
void ReBuildCharactersList( void );


BOOLEAN HandlePreloadOfMapGraphics( void );
void HandleRemovalOfPreLoadedMapGraphics( void );

void ChangeSelectedMapSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ );

BOOLEAN CanToggleSelectedCharInventory( void );

BOOLEAN CanExtendContractForSoldier(const SOLDIERTYPE* s);

void TellPlayerWhyHeCantCompressTime( void );

void ChangeSelectedInfoChar( INT8 bCharNumber, BOOLEAN fResetSelectedList );

void MAPEndItemPointer(void);

void CopyPathToAllSelectedCharacters(PathSt* pPath);
void CancelPathsOfAllSelectedCharacters(void);

INT32 GetPathTravelTimeDuringPlotting(PathSt* pPath);

void AbortMovementPlottingMode( void );

void ExplainWhySkyriderCantFly( void );

BOOLEAN CanChangeSleepStatusForSoldier(const SOLDIERTYPE* s);

BOOLEAN MapCharacterHasAccessibleInventory( INT8 bCharNumber );

const wchar_t* GetMapscreenMercAssignmentString(const SOLDIERTYPE* pSoldier);
void GetMapscreenMercLocationString(const SOLDIERTYPE* pSoldier, wchar_t sString[], size_t Length);
void GetMapscreenMercDestinationString(const SOLDIERTYPE* pSoldier, wchar_t sString[], size_t Length);
void GetMapscreenMercDepartureString(const SOLDIERTYPE* pSoldier, wchar_t sString[], size_t Length, UINT8* pubFontColor);

// mapscreen wrapper to init the item description box
BOOLEAN MAPInternalInitItemDescriptionBox(OBJECTTYPE* pObject, UINT8 ubStatusIndex, SOLDIERTYPE* pSoldier);

// rebuild contract box this character
void RebuildContractBoxForMerc(const SOLDIERTYPE* s);

void    InternalMAPBeginItemPointer(SOLDIERTYPE* pSoldier);
BOOLEAN ContinueDialogue(SOLDIERTYPE* pSoldier, BOOLEAN fDone);
BOOLEAN GetMouseMapXY(INT16* psMapWorldX, INT16* psMapWorldY);
void    EndConfirmMapMoveMode(void);
BOOLEAN CanDrawSectorCursor(void);
void    RememberPreviousPathForAllSelectedChars(void);
void    MapScreenDefaultOkBoxCallback(UINT8 bExitValue);
void    SetUpCursorForStrategicMap(void);

extern MOUSE_REGION gMPanelRegion;
extern UINT32       guiMapInvSecondHandBlockout;
extern INT32        giMapInvDoneButton;
extern BOOLEAN      fInMapMode;
extern BOOLEAN      fReDrawFace;
extern BOOLEAN      fShowInventoryFlag;
extern BOOLEAN      fShowDescriptionFlag;
extern INT32        giMapContractButton;
extern INT32        giCharInfoButton[2];
extern MOUSE_REGION gCharInfoHandRegion;

#ifdef JA2TESTVERSION
void DumpSectorDifficultyInfo(void);
#endif

#endif
