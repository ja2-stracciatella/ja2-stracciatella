#ifndef __MAPSCREEN_H
#define __MAPSCREEN_H

#include "Button_System.h"
#include "MessageBoxScreen.h"
#include "ScreenIDs.h"
#include "Types.h"
#include <string_theory/string>

struct OBJECTTYPE;
struct PathSt;
struct SOLDIERTYPE;


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


extern BOOLEAN fCharacterInfoPanelDirty;
extern BOOLEAN fTeamPanelDirty;
extern BOOLEAN fMapPanelDirty;

extern BOOLEAN fMapInventoryItem;
extern BOOLEAN gfInConfirmMapMoveMode;
extern BOOLEAN gfInChangeArrivalSectorMode;

extern BOOLEAN gfSkyriderEmptyHelpGiven;


void SetInfoChar(SOLDIERTYPE const*);
void EndMapScreen( BOOLEAN fDuringFade );
void ReBuildCharactersList( void );


void HandlePreloadOfMapGraphics(void);
void HandleRemovalOfPreLoadedMapGraphics( void );

void ChangeSelectedMapSector(const SGPSector& sector);

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

bool MapCharacterHasAccessibleInventory(SOLDIERTYPE const&);

ST::string GetMapscreenMercAssignmentString(SOLDIERTYPE const& s);
ST::string GetMapscreenMercLocationString(SOLDIERTYPE const& s);
ST::string GetMapscreenMercDestinationString(SOLDIERTYPE const& s);
ST::string GetMapscreenMercDepartureString(SOLDIERTYPE const& s, UINT8* text_colour);

// mapscreen wrapper to init the item description box
void MAPInternalInitItemDescriptionBox(OBJECTTYPE* pObject, UINT8 ubStatusIndex, SOLDIERTYPE* pSoldier);

// rebuild contract box this character
void RebuildContractBoxForMerc(const SOLDIERTYPE* s);

void    InternalMAPBeginItemPointer(SOLDIERTYPE* pSoldier);
BOOLEAN ContinueDialogue(SOLDIERTYPE* pSoldier, BOOLEAN fDone);
BOOLEAN GetMouseMapXY(SGPSector& sMap);
void    EndConfirmMapMoveMode(void);
BOOLEAN CanDrawSectorCursor(void);
void    RememberPreviousPathForAllSelectedChars(void);
void    MapScreenDefaultOkBoxCallback(MessageBoxReturnValue);
void    SetUpCursorForStrategicMap(void);
void    DrawFace(void);
void DrawStringRight(const ST::string& str, UINT16 x, UINT16 y, UINT16 w, UINT16 h, SGPFont font);

extern GUIButtonRef giMapInvDoneButton;
extern BOOLEAN      fInMapMode;
extern BOOLEAN      fReDrawFace;
extern BOOLEAN      fShowInventoryFlag;
extern BOOLEAN      fShowDescriptionFlag;
extern GUIButtonRef giMapContractButton;
extern GUIButtonRef giCharInfoButton[2];
extern BOOLEAN      fDrawCharacterList;

// create/destroy inventory button as needed
void CreateDestroyMapInvButton(void);

void     MapScreenInit(void);
ScreenID MapScreenHandle(void);
void     MapScreenShutdown(void);

void LockMapScreenInterface(bool lock);
void MakeDialogueEventEnterMapScreen();

void SetMapCursorItem();

#define NAME_X                (STD_SCREEN_X + 11)
#define NAME_WIDTH            (STD_SCREEN_X + 62 - NAME_X)
#define ASSIGN_X              (STD_SCREEN_X + 67)
#define ASSIGN_WIDTH          (STD_SCREEN_X + 118 - ASSIGN_X)
#define SLEEP_X               (STD_SCREEN_X + 123)
#define SLEEP_WIDTH           (STD_SCREEN_X + 142 - SLEEP_X)
#define LOC_X                 (STD_SCREEN_X + 147)
#define LOC_WIDTH             (STD_SCREEN_X + 179 - LOC_X)
#define DEST_ETA_X            (STD_SCREEN_X + 184)
#define DEST_ETA_WIDTH        (STD_SCREEN_X + 217 - DEST_ETA_X)
#define TIME_REMAINING_X      (STD_SCREEN_X + 222)
#define TIME_REMAINING_WIDTH  (STD_SCREEN_X + 250 - TIME_REMAINING_X)
#define CLOCK_Y_START         (STD_SCREEN_Y + 298)
#define CLOCK_ETA_X           (STD_SCREEN_X + 463 - 15 + 6 + 30)
#define CLOCK_HOUR_X_START    (STD_SCREEN_X + 463 + 25 + 30)
#define CLOCK_MIN_X_START     (STD_SCREEN_X + 463 + 45 + 30)

// contract
#define CONTRACT_X            (STD_SCREEN_X + 185)
#define CONTRACT_Y            (STD_SCREEN_Y + 50)

// trash can
#define TRASH_CAN_X           (STD_SCREEN_X + 176)
#define TRASH_CAN_Y           (211 + PLAYER_INFO_Y)
#define TRASH_CAN_WIDTH       193 - 165
#define TRASH_CAN_HEIGHT      239 - 217

#endif
