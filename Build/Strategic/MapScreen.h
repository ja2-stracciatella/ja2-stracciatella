#ifndef __MAPSCREEN_H
#define __MAPSCREEN_H

#include "Button_System.h"
#include "Item_Types.h"
#include "JA2Types.h"
#include "MessageBoxScreen.h"
#include "ScreenIDs.h"


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

void ChangeSelectedMapSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ );

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

wchar_t const* GetMapscreenMercAssignmentString(SOLDIERTYPE const&);
void GetMapscreenMercLocationString(SOLDIERTYPE const&, wchar_t* buf, size_t n);
void GetMapscreenMercDestinationString(SOLDIERTYPE const&, wchar_t* buf, size_t n);
void GetMapscreenMercDepartureString(SOLDIERTYPE const&, wchar_t* buf, size_t n, UINT8* text_colour);

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

#ifdef JA2TESTVERSION
void DumpSectorDifficultyInfo(void);
#endif

#endif
