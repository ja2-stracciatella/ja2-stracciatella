#ifndef _HANDLE_UI_
#define _HANDLE_UI_

#include "Input.h"
#include "Interface_Cursors.h"
#include "JA2Types.h"
#include "MouseSystem.h"
#include "ScreenIDs.h"


#define		UIEVENT_SINGLEEVENT					0x00000002
#define		UIEVENT_SNAPMOUSE						0x00000008

#define		NO_GUY_SELECTION									0
#define   SELECTED_GUY_SELECTION						1
#define   NONSELECTED_GUY_SELECTION					2
#define   ENEMY_GUY_SELECTION								3

enum MouseMoveState
{
	MOUSE_STATIONARY,
	MOUSE_MOVING_IN_TILE,
	MOUSE_MOVING_NEW_TILE
};

enum MoveUITarget
{
	MOVEUI_TARGET_NONE        =  0,
	MOVEUI_TARGET_INTTILES    =  1,
	MOVEUI_TARGET_ITEMS       =  2,
	MOVEUI_TARGET_MERCS       =  3,
	MOVEUI_TARGET_MERCSFORAID =  5,
	MOVEUI_TARGET_WIREFENCE   =  6,
	MOVEUI_TARGET_BOMB        =  7,
	MOVEUI_TARGET_STEAL       =  8,
	MOVEUI_TARGET_REPAIR      =  9,
	MOVEUI_TARGET_JAR         = 10,
	MOVEUI_TARGET_CAN         = 11,
	MOVEUI_TARGET_REFUEL      = 12
};

#define	MOVEUI_RETURN_ON_TARGET_MERC		1

enum	UI_MODE
{
	DONT_CHANGEMODE,
	IDLE_MODE,
	MOVE_MODE,
	ACTION_MODE,
	MENU_MODE,
	CONFIRM_MOVE_MODE,
	ADJUST_STANCE_MODE,
	CONFIRM_ACTION_MODE,
	HANDCURSOR_MODE,
	GETTINGITEM_MODE,
	ENEMYS_TURN_MODE,
	LOOKCURSOR_MODE,
	TALKINGMENU_MODE,
	TALKCURSOR_MODE,
	LOCKUI_MODE,
	OPENDOOR_MENU_MODE,
	LOCKOURTURN_UI_MODE,
	EXITSECTORMENU_MODE,
	RUBBERBAND_MODE,
	JUMPOVER_MODE,
};

struct UI_EVENT
{
	typedef ScreenID (*UI_HANDLEFNC)(UI_EVENT*);

	UINT32					uiFlags;
	UI_MODE					ChangeToUIMode;
	UI_HANDLEFNC		HandleEvent;
	BOOLEAN					fFirstTime;
	BOOLEAN					fDoneMenu;
	UI_MODE         uiMenuPreviousMode;
	UINT32					uiParams[3];
};


// EVENT ENUMERATION
enum UI_EVENT_DEFINES
{
	I_DO_NOTHING,
	I_NEW_MERC,
	I_NEW_BADMERC,
	I_SELECT_MERC,
	I_ENTER_EDIT_MODE,
	I_ENDTURN,
	I_TESTHIT,
	I_CHANGELEVEL,
	I_ON_TERRAIN,
	I_CHANGE_TO_IDLE,
	I_SOLDIERDEBUG,
	I_LOSDEBUG,
	I_LEVELNODEDEBUG,

	ET_ON_TERRAIN,
	ET_ENDENEMYS_TURN,

	M_ON_TERRAIN,
	M_CHANGE_TO_ACTION,
	M_CHANGE_TO_HANDMODE,
	M_CYCLE_MOVEMENT,
	M_CYCLE_MOVE_ALL,
	M_CHANGE_TO_ADJPOS_MODE,

	A_ON_TERRAIN,
	A_CHANGE_TO_MOVE,
	A_CHANGE_TO_CONFIM_ACTION,
	A_END_ACTION,
	U_MOVEMENT_MENU,
	U_POSITION_MENU,

	C_WAIT_FOR_CONFIRM,
	C_MOVE_MERC,
	C_ON_TERRAIN,

	PADJ_ADJUST_STANCE,

	CA_ON_TERRAIN,
	CA_MERC_SHOOT,
	CA_END_CONFIRM_ACTION,

	HC_ON_TERRAIN,

	G_GETTINGITEM,

	LC_ON_TERRAIN,
	LC_CHANGE_TO_LOOK,
	LC_LOOK,

	TA_TALKINGMENU,

	T_ON_TERRAIN,
	T_CHANGE_TO_TALKING,

	LU_ON_TERRAIN,
	LU_BEGINUILOCK,
	LU_ENDUILOCK,

	OP_OPENDOORMENU,

	LA_ON_TERRAIN,
	LA_BEGINUIOURTURNLOCK,
	LA_ENDUIOUTURNLOCK,

	EX_EXITSECTORMENU,

	RB_ON_TERRAIN,

	JP_ON_TERRAIN,
	JP_JUMP,

	NUM_UI_EVENTS
};

typedef BOOLEAN (*UIKEYBOARD_HOOK)( InputAtom *pInputEvent );


// GLOBAL STATUS VARS
extern UI_MODE gCurrentUIMode;
extern UINT32  guiCurrentEvent;
extern UICursorID guiCurrentUICursor;
extern INT16   gsSelectedLevel;
extern BOOLEAN gfPlotNewMovement;
extern UINT32  guiPendingOverrideEvent;


// GLOBALS
extern BOOLEAN      gfUIDisplayActionPoints;
extern BOOLEAN      gfUIDisplayActionPointsInvalid;
extern BOOLEAN      gfUIDisplayActionPointsBlack;
extern BOOLEAN      gfUIDisplayActionPointsCenter;
extern INT16        gUIDisplayActionPointsOffY;
extern INT16        gUIDisplayActionPointsOffX;
extern UINT32       guiShowUPDownArrows;
extern BOOLEAN      gfUIHandleSelection;
extern INT16        gsSelectedGridNo;
extern SOLDIERTYPE* gSelectedGuy;

#ifdef JA2TESTVERSION
extern UINT8 gUIDeadlockedSoldier;
#endif

extern BOOLEAN            gfUIMouseOnValidCatcher;
extern const SOLDIERTYPE* gUIValidCatcher;
extern BOOLEAN            gUIUseReverse;


extern BOOLEAN gfUIHandleShowMoveGrid;
extern UINT16  gsUIHandleShowMoveGridLocation;

extern BOOLEAN gUITargetShotWaiting;

extern BOOLEAN gfUIWaitingForUserSpeechAdvance;

extern BOOLEAN gfUIAllMoveOn;
extern BOOLEAN gfUICanBeginAllMoveCycle;

extern BOOLEAN gfUIRefreshArrows;

extern BOOLEAN gfUIHandlePhysicsTrajectory;


// GLOBALS FOR FAST LOOKUP FOR FINDING MERCS FROM THE MOUSE
extern SOLDIERTYPE* gUIFullTarget;
extern UINT32       guiUIFullTargetFlags;

extern BOOLEAN gfUIConfirmExitArrows;
extern INT16   gsJumpOverGridNo;

ScreenID HandleTacticalUI(void);
ScreenID UIHandleEndTurn(UI_EVENT*);

extern BOOLEAN gfUIShowCurIntTile;

extern SGPRect gRubberBandRect;
extern BOOLEAN gRubberBandActive;

void EndMenuEvent( UINT32	uiEvent );
void SetUIKeyboardHook( UIKEYBOARD_HOOK KeyboardHookFnc );

extern BOOLEAN gfUIForceReExamineCursorData;

// FUNCTIONS IN INPUT MODULES
void GetKeyboardInput( UINT32 *puiNewEvent );
void GetPolledKeyboardInput( UINT32 *puiNewEvent );

void GetTBMouseButtonInput( UINT32 *puiNewEvent );
void GetTBMousePositionInput( UINT32 *puiNewEvent );
void HandleStanceChangeFromUIKeys( UINT8 ubAnimHeight );
void HandleKeyInputOnEnemyTurn(void);


BOOLEAN SelectedMercCanAffordAttack(void);
BOOLEAN SelectedMercCanAffordMove(void);
bool CanMercClimbUp(SOLDIERTYPE const*);

void ToggleHandCursorMode( UINT32 *puiNewEvent );
void ToggleTalkCursorMode( UINT32 *puiNewEvent );
void ToggleLookCursorMode( UINT32 *puiNewEvent );

void UIHandleSoldierStanceChange(SOLDIERTYPE* s, INT8 bNewStance);
MouseMoveState GetCursorMovementFlags();

BOOLEAN HandleUIMovementCursor(SOLDIERTYPE*, MouseMoveState, UINT16 usMapPos, MoveUITarget);
bool UIMouseOnValidAttackLocation(SOLDIERTYPE*);

BOOLEAN UIOkForItemPickup( SOLDIERTYPE *pSoldier, INT16 sGridNo );

SOLDIERTYPE* GetValidTalkableNPCFromMouse(BOOLEAN fGive, BOOLEAN fAllowMercs, BOOLEAN fCheckCollapsed);
BOOLEAN IsValidTalkableNPC(const SOLDIERTYPE* s, BOOLEAN fGive, BOOLEAN fAllowMercs, BOOLEAN fCheckCollapsed);

BOOLEAN HandleTalkInit(void);

BOOLEAN HandleCheckForExitArrowsInput( BOOLEAN fAdjustForConfirm );

void SetUIBusy(const SOLDIERTYPE* s);
void UnSetUIBusy(const SOLDIERTYPE* s);

ScreenID UIHandleLUIEndLock(UI_EVENT*);

void BeginDisplayTimedCursor(UICursorID, UINT32 uiDelay);

void HandleHandCursorClick( UINT16 usMapPos, UINT32 *puiNewEvent );

ScreenID UIHandleChangeLevel(UI_EVENT*);
BOOLEAN UIHandleOnMerc( BOOLEAN fMovementMode );

void ChangeInterfaceLevel( INT16 sLevel );

void EndRubberBanding(void);
void ResetMultiSelection(void);
void EndMultiSoldierSelection( BOOLEAN fAcknowledge );
void StopRubberBandedMercFromMoving(void);

BOOLEAN SelectedGuyInBusyAnimation(void);

void GotoLowerStance( SOLDIERTYPE *pSoldier );
void GotoHeigherStance( SOLDIERTYPE *pSoldier );

BOOLEAN IsValidJumpLocation(const SOLDIERTYPE* pSoldier, INT16 sGridNo, BOOLEAN fCheckForPath);

void PopupAssignmentMenuInTactical(void);

extern GridNo          gfUIOverItemPoolGridNo;
extern INT16           gsCurrentActionPoints;
extern BOOLEAN         fRightButtonDown;
extern BOOLEAN         fLeftButtonDown;
extern BOOLEAN         fIgnoreLeftUp;
extern BOOLEAN         gfIgnoreOnSelectedGuy;
extern BOOLEAN         gUIActionModeChangeDueToMouseOver;
extern MOUSE_REGION    gDisableRegion;
extern BOOLEAN         gfDisableRegionActive;
extern MOUSE_REGION    gUserTurnRegion;
extern BOOLEAN         gfUserTurnRegionActive;
extern UIKEYBOARD_HOOK gUIKeyboardHook;
extern BOOLEAN         gfResetUIMovementOptimization;

extern BOOLEAN gfUIShowExitEast;
extern BOOLEAN gfUIShowExitWest;
extern BOOLEAN gfUIShowExitNorth;
extern BOOLEAN gfUIShowExitSouth;

BOOLEAN ValidQuickExchangePosition(void);

void CheckForDisabledRegionRemove(void);

void HandleTacticalUILoseCursorFromOtherScreen(void);

void SetInterfaceHeightLevel(void);

#endif
