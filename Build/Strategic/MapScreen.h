#ifndef __MAPSCREEN_H
#define __MAPSCREEN_H

#include "Button_System.h"
#include "Item_Types.h"
#include "JA2Types.h"
#include "MessageBoxScreen.h"
#include "MouseSystem.h"
#include "ScreenIDs.h"


#define			MAP_WORLD_X						18
#define			MAP_WORLD_Y						18


// Sector name identifiers
enum Towns
{
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
};

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


void HandlePreloadOfMapGraphics(void);
void HandleRemovalOfPreLoadedMapGraphics( void );

void ChangeSelectedMapSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ );

BOOLEAN CanToggleSelectedCharInventory( void );

BOOLEAN CanExtendContractForSoldier(const SOLDIERTYPE* s);

void TellPlayerWhyHeCantCompressTime( void );

// the info character
extern INT8 bSelectedInfoChar;

SOLDIERTYPE* GetSelectedInfoChar(void);
void ChangeSelectedInfoChar( INT8 bCharNumber, BOOLEAN fResetSelectedList );

void MAPEndItemPointer(void);

void CopyPathToAllSelectedCharacters(PathSt* pPath);
void CancelPathsOfAllSelectedCharacters(void);

INT32 GetPathTravelTimeDuringPlotting(PathSt* pPath);

void AbortMovementPlottingMode( void );

BOOLEAN CanChangeSleepStatusForSoldier(const SOLDIERTYPE* s);

BOOLEAN MapCharacterHasAccessibleInventory(const SOLDIERTYPE*);

const wchar_t* GetMapscreenMercAssignmentString(const SOLDIERTYPE* pSoldier);
void GetMapscreenMercLocationString(const SOLDIERTYPE* pSoldier, wchar_t sString[], size_t Length);
void GetMapscreenMercDestinationString(const SOLDIERTYPE* pSoldier, wchar_t sString[], size_t Length);
void GetMapscreenMercDepartureString(const SOLDIERTYPE* pSoldier, wchar_t sString[], size_t Length, UINT8* pubFontColor);

// mapscreen wrapper to init the item description box
void MAPInternalInitItemDescriptionBox(OBJECTTYPE* pObject, UINT8 ubStatusIndex, SOLDIERTYPE* pSoldier);

// rebuild contract box this character
void RebuildContractBoxForMerc(const SOLDIERTYPE* s);

void    InternalMAPBeginItemPointer(SOLDIERTYPE* pSoldier);
BOOLEAN ContinueDialogue(SOLDIERTYPE* pSoldier, BOOLEAN fDone);
BOOLEAN GetMouseMapXY(INT16* psMapWorldX, INT16* psMapWorldY);
void    EndConfirmMapMoveMode(void);
BOOLEAN CanDrawSectorCursor(void);
void    RememberPreviousPathForAllSelectedChars(void);
void    MapScreenDefaultOkBoxCallback(MessageBoxReturnValue);
void    SetUpCursorForStrategicMap(void);
void    DrawFace(void);

extern MOUSE_REGION gMPanelRegion;
#ifndef JA2DEMO
extern SGPVObject*  guiMapInvSecondHandBlockout;
#endif
extern GUIButtonRef giMapInvDoneButton;
extern BOOLEAN      fInMapMode;
extern BOOLEAN      fReDrawFace;
extern BOOLEAN      fShowInventoryFlag;
extern BOOLEAN      fShowDescriptionFlag;
extern GUIButtonRef giMapContractButton;
extern GUIButtonRef giCharInfoButton[2];
extern MOUSE_REGION gCharInfoHandRegion;
extern BOOLEAN      fDrawCharacterList;

// create/destroy inventory button as needed
void CreateDestroyMapInvButton(void);

void     MapScreenInit(void);
ScreenID MapScreenHandle(void);
void     MapScreenShutdown(void);

void LockMapScreenInterface(bool lock);

#ifdef JA2TESTVERSION
void DumpSectorDifficultyInfo(void);
#endif

#ifdef JA2DEMO
void DisabledInDemo(void);
#endif

#endif
