#include "FileMan.h"
#include "Font_Control.h"
#include "Handle_Doors.h"
#include "Items.h"
#include "Local.h"
#include "Merc_Hiring.h"
#include "Real_Time_Input.h"
#include "Turn_Based_Input.h"
#include "Touch_UI.h"
#include "Soldier_Find.h"
#include "Debug.h"
#include "JAScreens.h"
#include "PathAI.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "MouseSystem.h"
#include "Cursors.h"
#include "Handle_UI.h"
#include "Isometric_Utils.h"
#include "Input.h"
#include "Overhead.h"
#include "Sys_Globals.h"
#include "Interface.h"
#include "Cursor_Control.h"
#include "Points.h"
#include "Interactive_Tiles.h"
#include "OppList.h"
#include "WorldMan.h"
#include "Weapons.h"
#include "RenderWorld.h"
#include "Structure.h"
#include "Handle_Items.h"
#include "UI_Cursors.h"
#include "Message.h"
#include "Render_Fun.h"
#include "Interface_Items.h"
#include "StrategicMap.h"
#include "Soldier_Profile.h"
#include "Soldier_Create.h"
#include "Soldier_Add.h"
#include "Interface_Dialogue.h"
#include "Soldier_Macros.h"
#include "Soldier_Functions.h"
#include "Assignments.h"
#include "Squads.h"
#include "Strategic_Pathing.h"
#include "Strategic_Movement.h"
#include "Strategic.h"
#include "Exit_Grids.h"
#include "Structure_Wrap.h"
#include "Random.h"
#include "English.h"
#include "Vehicles.h"
#include "MessageBoxScreen.h"
#include "Text.h"
#include "Dialogue_Control.h"
#include "Line.h"
#include "Render_Dirty.h"
#include "GameSettings.h"
#include "LOS.h"
#include "Campaign_Types.h"
#include "Queen_Command.h"
#include "Options_Screen.h"
#include "SaveLoadScreen.h"
#include "Spread_Burst.h"
#include "AI.h"
#include "Game_Clock.h"
#include "Civ_Quotes.h"
#include "QArray.h"
#include "Environment.h"
#include "Map_Information.h"
#include "Video.h"
#include "Screens.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Soldier.h"
#include "MercProfile.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>

#define MAX_ON_DUTY_SOLDIERS 6

/////////////////////////////////////////////////////////////////////////////////////
//  UI SYSTEM DESCRIPTION
//
//  The UI system here decouples event determination from event execution. IN other words,
//  first any user input is gathered and analysed for an event to happen. Once the event is determined,
//  it is then executed. For example, if the left mouse button is used to select a guy, it does not
//  execute the code to selected the guy, rather it sets a flag to a particular event, in this case
//  the I_SELECT_MERC event is set. The code then executes this event after all input is analysed. In
//  this way, more than one input method from the user will cause the came event to occur and hence no
//  duplication of code. Also, events have cetain charactoristics. The select merc event is executed just
//  once and then returns to the previous event. Most events are set to run continuously until new
//  input changes to another event. Other events have a 'SNAP-BACK' feature which snap the mouse back to
//  it's position before the event was executed.  Another issue is UI modes. In order to filter out input
//  depending on other flags, for example we do not want to cancel the confirm when a user moves to another
//  tile unless we are in the 'confirm' mode.  This could be done by flags ( and in effect it is ) where
//  if staements are used, but here at input collection time, we can switch on our current mode to handle
//  input differently based on the mode. Doing it this way also allows us to group certain commands togther
//  like menu commands which are initialized and deleted in the same manner.
//
//  UI_EVENTS
/////////////
//
//  UI_EVENTS have flags to itendtify themselves with special charactoristics, a UI_MODE catagory which
//  signifies the UI mode this event will cause the system to move to. Also, a pointer to a handle function is
//  used to actually handle the particular event. UI_EVENTS also have a couple of param variables and a number
//  of boolean flags used during run-time to determine states of events.
//
////////////////////////////////////////////////////////////////////////////////////////////////

#define GO_MOVE_ONE			40
#define GO_MOVE_TWO			80
#define GO_MOVE_THREE			100


// CALLBACKS FOR EVENTS
static ScreenID UIHandleIDoNothing(UI_EVENT*);
static ScreenID UIHandleNewMerc(UI_EVENT*);
static ScreenID UIHandleNewBadMerc(UI_EVENT*);
static ScreenID UIHandleSelectMerc(UI_EVENT*);
static ScreenID UIHandleEnterEditMode(UI_EVENT*);
static ScreenID UIHandleTestHit(UI_EVENT*);
static ScreenID UIHandleIOnTerrain(UI_EVENT*);
static ScreenID UIHandleIChangeToIdle(UI_EVENT*);
static ScreenID UIHandleISoldierDebug(UI_EVENT*);
static ScreenID UIHandleILOSDebug(UI_EVENT*);
static ScreenID UIHandleILevelNodeDebug(UI_EVENT*);

static ScreenID UIHandleIETOnTerrain(UI_EVENT*);

static ScreenID UIHandleMOnTerrain(UI_EVENT*);
static ScreenID UIHandleMChangeToAction(UI_EVENT*);
static ScreenID UIHandleMCycleMovement(UI_EVENT*);
static ScreenID UIHandleMCycleMoveAll(UI_EVENT*);
static ScreenID UIHandleMAdjustStanceMode(UI_EVENT*);
static ScreenID UIHandleMChangeToHandMode(UI_EVENT*);

static ScreenID UIHandleAOnTerrain(UI_EVENT*);
static ScreenID UIHandleAChangeToMove(UI_EVENT*);
static ScreenID UIHandleAChangeToConfirmAction(UI_EVENT*);
static ScreenID UIHandleAEndAction(UI_EVENT*);

static ScreenID UIHandleMovementMenu(UI_EVENT*);
static ScreenID UIHandlePositionMenu(UI_EVENT*);

static ScreenID UIHandleCWait(UI_EVENT*);
static ScreenID UIHandleCMoveMerc(UI_EVENT*);
static ScreenID UIHandleCOnTerrain(UI_EVENT*);

static ScreenID UIHandlePADJAdjustStance(UI_EVENT*);

static ScreenID UIHandleCAOnTerrain(UI_EVENT*);
static ScreenID UIHandleCAMercShoot(UI_EVENT*);
static ScreenID UIHandleCAEndConfirmAction(UI_EVENT*);

static ScreenID UIHandleHCOnTerrain(UI_EVENT*);
static ScreenID UIHandleHCGettingItem(UI_EVENT*);


static ScreenID UIHandleLCOnTerrain(UI_EVENT*);
static ScreenID UIHandleLCChangeToLook(UI_EVENT*);
static ScreenID UIHandleLCLook(UI_EVENT*);

static ScreenID UIHandleTATalkingMenu(UI_EVENT*);

static ScreenID UIHandleTOnTerrain(UI_EVENT*);
static ScreenID UIHandleTChangeToTalking(UI_EVENT*);

static ScreenID UIHandleLUIOnTerrain(UI_EVENT*);
static ScreenID UIHandleLUIBeginLock(UI_EVENT*);

static ScreenID UIHandleLAOnTerrain(UI_EVENT*);
static ScreenID UIHandleLABeginLockOurTurn(UI_EVENT*);
static ScreenID UIHandleLAEndLockOurTurn(UI_EVENT*);

static ScreenID UIHandleRubberBandOnTerrain(UI_EVENT*);
static ScreenID UIHandleJumpOverOnTerrain(UI_EVENT*);
static ScreenID UIHandleJumpOver(UI_EVENT*);

static ScreenID UIHandleOpenDoorMenu(UI_EVENT*);

static ScreenID UIHandleEXExitSectorMenu(UI_EVENT*);

static ScreenID UIHandlePPanMode(UI_EVENT*);


static SOLDIERTYPE* gpRequesterMerc        = NULL;
static SOLDIERTYPE* gpRequesterTargetMerc  = NULL;
static INT16        gsRequesterGridNo;
static INT16        gsOverItemsGridNo      = NOWHERE;
static INT16        gsOverItemsLevel       = 0;
static BOOLEAN      gfUIInterfaceSetBusy   = FALSE;
static UINT32       guiUIInterfaceBusyTime = 0;

static UINT32     gfTacticalForceNoCursor         = FALSE;
static LEVELNODE* gpInvTileThatCausedMoveConfirm  = NULL;
BOOLEAN           gfResetUIMovementOptimization   = FALSE;
static BOOLEAN    gfBeginVehicleCursor            = FALSE;
static UINT16     gsOutOfRangeGridNo              = NOWHERE;
static UINT8      gubOutOfRangeMerc               = NOBODY;
static BOOLEAN    gfOKForExchangeCursor           = FALSE;
static UINT32     guiUIInterfaceSwapCursorsTime   = 0;
INT16             gsJumpOverGridNo                = 0;


#define M(flags, to, handler)		{ flags, to, handler, FALSE, FALSE, DONT_CHANGEMODE, { 0, 0, 0 } }

static UI_EVENT gEvents[NUM_UI_EVENTS] =
{
	M(0,                   IDLE_MODE,           UIHandleIDoNothing            ),
	M(UIEVENT_SINGLEEVENT, DONT_CHANGEMODE,     UIHandleNewMerc               ),
	M(UIEVENT_SINGLEEVENT, DONT_CHANGEMODE,     UIHandleNewBadMerc            ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleSelectMerc            ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleEnterEditMode         ),
	M(UIEVENT_SINGLEEVENT, DONT_CHANGEMODE,     UIHandleEndTurn               ),
	M(UIEVENT_SINGLEEVENT, DONT_CHANGEMODE,     UIHandleTestHit               ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleChangeLevel           ),
	M(UIEVENT_SINGLEEVENT, IDLE_MODE,           UIHandleIOnTerrain            ),
	M(UIEVENT_SINGLEEVENT, IDLE_MODE,           UIHandleIChangeToIdle         ),
	M(UIEVENT_SINGLEEVENT, DONT_CHANGEMODE,     UIHandleISoldierDebug         ),
	M(UIEVENT_SINGLEEVENT, DONT_CHANGEMODE,     UIHandleILOSDebug             ),
	M(UIEVENT_SINGLEEVENT, DONT_CHANGEMODE,     UIHandleILevelNodeDebug       ),

	M(0,                   ENEMYS_TURN_MODE,    UIHandleIETOnTerrain          ),

	M(0,                   MOVE_MODE,           UIHandleMOnTerrain            ),
	M(UIEVENT_SINGLEEVENT, ACTION_MODE,         UIHandleMChangeToAction       ),
	M(UIEVENT_SINGLEEVENT, HANDCURSOR_MODE,     UIHandleMChangeToHandMode     ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleMCycleMovement        ),
	M(UIEVENT_SINGLEEVENT, CONFIRM_MOVE_MODE,   UIHandleMCycleMoveAll         ),
	M(UIEVENT_SNAPMOUSE,   ADJUST_STANCE_MODE,  UIHandleMAdjustStanceMode     ),
	M(0,                   ACTION_MODE,         UIHandleAOnTerrain            ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleAChangeToMove         ),
	M(UIEVENT_SINGLEEVENT, CONFIRM_ACTION_MODE, UIHandleAChangeToConfirmAction),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleAEndAction            ),
	M(UIEVENT_SNAPMOUSE,   MENU_MODE,           UIHandleMovementMenu          ),
	M(UIEVENT_SNAPMOUSE,   MENU_MODE,           UIHandlePositionMenu          ),
	M(0,                   CONFIRM_MOVE_MODE,   UIHandleCWait                 ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleCMoveMerc             ),
	M(0,                   CONFIRM_MOVE_MODE,   UIHandleCOnTerrain            ),
	M(0,                   MOVE_MODE,           UIHandlePADJAdjustStance      ),
	M(0,                   CONFIRM_ACTION_MODE, UIHandleCAOnTerrain           ),
	M(UIEVENT_SINGLEEVENT, ACTION_MODE,         UIHandleCAMercShoot           ),
	M(UIEVENT_SINGLEEVENT, ACTION_MODE,         UIHandleCAEndConfirmAction    ),
	M(0,                   HANDCURSOR_MODE,     UIHandleHCOnTerrain           ),
	M(0,                   GETTINGITEM_MODE,    UIHandleHCGettingItem         ),
	M(0,                   LOOKCURSOR_MODE,     UIHandleLCOnTerrain           ),
	M(UIEVENT_SINGLEEVENT, LOOKCURSOR_MODE,     UIHandleLCChangeToLook        ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleLCLook                ),
	M(0,                   TALKINGMENU_MODE,    UIHandleTATalkingMenu         ),
	M(0,                   TALKCURSOR_MODE,     UIHandleTOnTerrain            ),
	M(UIEVENT_SINGLEEVENT, TALKCURSOR_MODE,     UIHandleTChangeToTalking      ),
	M(0,                   LOCKUI_MODE,         UIHandleLUIOnTerrain          ),
	M(0,                   LOCKUI_MODE,         UIHandleLUIBeginLock          ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleLUIEndLock            ),
	M(0,                   OPENDOOR_MENU_MODE,  UIHandleOpenDoorMenu          ),
	M(0,                   LOCKOURTURN_UI_MODE, UIHandleLAOnTerrain           ),
	M(0,                   LOCKOURTURN_UI_MODE, UIHandleLABeginLockOurTurn    ),
	M(UIEVENT_SINGLEEVENT, MOVE_MODE,           UIHandleLAEndLockOurTurn      ),
	M(0,                   EXITSECTORMENU_MODE, UIHandleEXExitSectorMenu      ),
	M(0,                   RUBBERBAND_MODE,     UIHandleRubberBandOnTerrain   ),
	M(0,                   JUMPOVER_MODE,       UIHandleJumpOverOnTerrain     ),
	M(0,                   MOVE_MODE,           UIHandleJumpOver              ),
	M(0,                   PAN_MODE,            UIHandlePPanMode               )
};

#undef M


UI_MODE gCurrentUIMode = IDLE_MODE;
static UI_MODE gOldUIMode = IDLE_MODE;
UIEventKind guiCurrentEvent = I_DO_NOTHING;
static UIEventKind guiOldEvent = I_DO_NOTHING;
UICursorID guiCurrentUICursor = NO_UICURSOR;
GridNo guiCurrentCursorGridNo = NOWHERE;
UINT8 gUIFingersDown = 0;
static UICursorID guiNewUICursor = NORMAL_SNAPUICURSOR;
UIEventKind guiPendingOverrideEvent = I_DO_NOTHING;
static UINT16 gusSavedMouseX;
static UINT16 gusSavedMouseY;
UIKEYBOARD_HOOK gUIKeyboardHook = NULL;
BOOLEAN gUIActionModeChangeDueToMouseOver = FALSE;

static BOOLEAN    gfDisplayTimerCursor = FALSE;
static UICursorID guiTimerCursorID     = NO_UICURSOR;
static UINT32     guiTimerLastUpdate   = 0;
static UINT32     guiTimerCursorDelay  = 0;


MOUSE_REGION gDisableRegion;
BOOLEAN gfDisableRegionActive = FALSE;

MOUSE_REGION gUserTurnRegion;
BOOLEAN gfUserTurnRegionActive = FALSE;


// For use with mouse button query routines
BOOLEAN	fRightButtonDown = FALSE;
BOOLEAN	fLeftButtonDown = FALSE;
BOOLEAN	fMiddleButtonDown = FALSE;
BOOLEAN fIgnoreLeftUp = FALSE;

static BOOLEAN gUITargetReady = FALSE;
BOOLEAN gUITargetShotWaiting = FALSE;
BOOLEAN gUIUseReverse = FALSE;

SGPRect gRubberBandRect = { 0, 0, 0, 0 };
BOOLEAN gRubberBandActive = FALSE;
BOOLEAN gfIgnoreOnSelectedGuy = FALSE;
static BOOLEAN gfViewPortAdjustedForSouth = FALSE;

// FLAGS
// These flags are set for a single frame execution and then are reset for the next iteration.
BOOLEAN gfUIDisplayActionPoints = FALSE;
BOOLEAN gfUIDisplayActionPointsInvalid = FALSE;
BOOLEAN gfUIDisplayActionPointsBlack = FALSE;
BOOLEAN gfUIDisplayActionPointsCenter = FALSE;

INT16 gUIDisplayActionPointsOffY = 0;
INT16 gUIDisplayActionPointsOffX = 0;
BOOLEAN gfUIHandleSelection = FALSE;
BOOLEAN gfUIHandleShowMoveGrid = FALSE;
UINT16 gsUIHandleShowMoveGridLocation = NOWHERE ;
GridNo gfUIOverItemPoolGridNo = NOWHERE;
INT16 gsCurrentActionPoints = 1;
BOOLEAN gfUIHandlePhysicsTrajectory = FALSE;
BOOLEAN gfUIMouseOnValidCatcher = FALSE;
const SOLDIERTYPE* gUIValidCatcher = NULL;


BOOLEAN gfUIConfirmExitArrows = FALSE;

BOOLEAN gfUIShowCurIntTile = FALSE;

BOOLEAN gfUIWaitingForUserSpeechAdvance = FALSE; // Waiting for key input/mouse click to advance speech
BOOLEAN gfUIAllMoveOn = FALSE; // Sets to all move
BOOLEAN gfUICanBeginAllMoveCycle = FALSE; // GEts set so we know that the next right-click is a move-call inc\stead of a movement cycle through

INT16 gsSelectedGridNo = 0;
INT16 gsSelectedLevel = I_GROUND_LEVEL;
SOLDIERTYPE* gSelectedGuy = NULL;

BOOLEAN gfUIRefreshArrows = FALSE;


// Thse flags are not re-set after each frame
BOOLEAN gfPlotNewMovement = FALSE;
static BOOLEAN gfPlotNewMovementNOCOST = FALSE;
UINT32 guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;
static INT8    gbAdjustStanceDiff      = 0;
static INT8    gbClimbID               = 0;

static BOOLEAN gfUIShowExitExitGrid = FALSE;

static BOOLEAN gfUINewStateForIntTile = FALSE;

BOOLEAN gfUIForceReExamineCursorData = FALSE;


SOLDIERTYPE*     gUIFullTarget;
SoldierFindFlags guiUIFullTargetFlags;


static void ClearEvent(UI_EVENT* pUIEvent);
static void SetUIMouseCursor(void);


// MAIN TACTICAL UI HANDLER
ScreenID HandleTacticalUI(void)
{
	LEVELNODE *pIntTile;
	static LEVELNODE *pOldIntTile = NULL;


	// RESET FLAGS
	gfUIDisplayActionPoints = FALSE;
	gfUIDisplayActionPointsInvalid = FALSE;
	gfUIDisplayActionPointsBlack = FALSE;
	gfUIDisplayActionPointsCenter = FALSE;
	gfUIHandleSelection = NO_GUY_SELECTION;
	gSelectedGuy = NULL;
	guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;
	SetHitLocationText(ST::null);
	SetIntTileLocationText(ST::null);
	SetIntTileLocation2Text(ST::null);
	SetChanceToHitText(ST::null);
	//gfUIForceReExamineCursorData = FALSE;
	gfUINewStateForIntTile = FALSE;
	gfUIShowExitExitGrid = FALSE;
	gfUIOverItemPoolGridNo = NOWHERE;
	gfUIHandlePhysicsTrajectory = FALSE;
	gfUIMouseOnValidCatcher = FALSE;
	gUIValidCatcher = NULL;
	gfIgnoreOnSelectedGuy = FALSE;

	// Set old event value
	UIEventKind uiNewEvent = guiCurrentEvent;
	guiOldEvent            = uiNewEvent;

	if ( gfUIInterfaceSetBusy )
	{
		if ( ( GetJA2Clock( ) - guiUIInterfaceBusyTime ) > 25000 )
		{
			gfUIInterfaceSetBusy = FALSE;

			//UNLOCK UI
			UnSetUIBusy(GetSelectedMan());

			// Decrease global busy  counter...
			gTacticalStatus.ubAttackBusyCount = 0;
			guiPendingOverrideEvent = LU_ENDUILOCK;
			UIHandleLUIEndLock( NULL );
		}
	}

	if ( ( GetJA2Clock( ) - guiUIInterfaceSwapCursorsTime ) > 1000 )
	{
		gfOKForExchangeCursor = !gfOKForExchangeCursor;
		guiUIInterfaceSwapCursorsTime = GetJA2Clock( );
	}

	// OK, do a check for on an int tile...
	pIntTile = GetCurInteractiveTile( );

	if ( pIntTile != pOldIntTile )
	{
		gfUINewStateForIntTile = TRUE;

		pOldIntTile = pIntTile;
	}

	if ( guiPendingOverrideEvent == I_DO_NOTHING )
	{
		// When we are here, guiCurrentEvent is set to the last event
		// Within the input gathering phase, it may change

		// GATHER INPUT
		// Any new event will overwrite previous events. Therefore,
		// PRIOTITIES GO LIKE THIS:
		//   Mouse Movement
		//   Keyboard Polling
		//   Mouse Buttons
		//   Keyboard Queued Events ( will override always )

		// SWITCH ON INPUT GATHERING, DEPENDING ON MODE
		// IF WE ARE NOT IN COMBAT OR IN REALTIME COMBAT
		if (!(gTacticalStatus.uiFlags & INCOMBAT))
		{
			// FROM MOUSE POSITION
			GetRTMousePositionInput( &uiNewEvent );
			// FROM KEYBOARD POLLING
			GetPolledKeyboardInput( &uiNewEvent );
			// FROM MOUSE CLICKS
			GetRTMouseButtonInput( &uiNewEvent );
			// FROM KEYBOARD
			GetKeyboardInput( &uiNewEvent );

		}
		else
		{
			// FROM MOUSE POSITION
			GetTBMousePositionInput( &uiNewEvent );
			// FROM KEYBOARD POLLING
			GetPolledKeyboardInput( &uiNewEvent );
			// FROM MOUSE CLICKS
			GetTBMouseButtonInput( &uiNewEvent );
			// FROM KEYBOARD
			GetKeyboardInput( &uiNewEvent );
		}


	}
	else
	{
		uiNewEvent = guiPendingOverrideEvent;
		guiPendingOverrideEvent = I_DO_NOTHING;
	}

	if ( HandleItemPickupMenu( ) )
	{
		uiNewEvent = A_CHANGE_TO_MOVE;
	}

	// Set Current event to new one!
	guiCurrentEvent = uiNewEvent;

	// Check if current event has changed and clear event if so, to prepare it for execution
	// Clearing it does things like set first time flag, param variavles, etc
	if ( uiNewEvent != guiOldEvent )
	{
		// Snap mouse back if it's that type
		if ( gEvents[ guiOldEvent ].uiFlags & UIEVENT_SNAPMOUSE )
		{
			SimulateMouseMovement( (UINT32)gusSavedMouseX, (UINT32)gusSavedMouseY );
		}

		ClearEvent( &gEvents[ uiNewEvent ] );
	}

	// Restore not scrolling from stance adjust....
	if ( gOldUIMode == ADJUST_STANCE_MODE )
	{
		gfIgnoreScrolling = FALSE;
	}

	// IF this event is of type snap mouse, save position
	if ( gEvents[ uiNewEvent ].uiFlags & UIEVENT_SNAPMOUSE && gEvents[ uiNewEvent ].fFirstTime )
	{
			// Save mouse position
			gusSavedMouseX = gusMouseXPos;
			gusSavedMouseY = gusMouseYPos;
	}

	// HANDLE UI EVENT
	ScreenID const ReturnVal = gEvents[uiNewEvent].HandleEvent(&gEvents[uiNewEvent]);

	if ( gfInOpenDoorMenu )
	{
		return( ReturnVal );
	}

	// Set first time flag to false, now that it has been executed
	gEvents[ uiNewEvent ].fFirstTime = FALSE;

	// Check if UI mode has changed from previous event
	if (gEvents[uiNewEvent].ChangeToUIMode != gCurrentUIMode &&
		(gEvents[uiNewEvent].ChangeToUIMode != DONT_CHANGEMODE))
	{
		gEvents[ uiNewEvent ].uiMenuPreviousMode = gCurrentUIMode;

		gOldUIMode = gCurrentUIMode;

		gCurrentUIMode = gEvents[ uiNewEvent ].ChangeToUIMode;

		// CHANGE MODE - DO SPECIAL THINGS IF WE ENTER THIS MODE
		switch( gCurrentUIMode )
		{
			case ACTION_MODE:
				ErasePath();
				break;
			default:
				break;
		}

	}

	// Check if menu event is done and if so set to privious mode
	// This is needed to hook into the interface stuff which sets the fDoneMenu flag
	if (gEvents[uiNewEvent].fDoneMenu)
	{
		if (gCurrentUIMode == MENU_MODE || gCurrentUIMode == LOOKCURSOR_MODE)
		{
			gCurrentUIMode = gEvents[ uiNewEvent ].uiMenuPreviousMode;
		}
	}
	// Check to return to privious mode
	// If the event is a single event, return to previous
	if ( gEvents[ uiNewEvent ].uiFlags & UIEVENT_SINGLEEVENT )
	{
		// ATE: OK - don't revert to single event if our mouse is not
		// in viewport - rather use m_on_t event
		if ( ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )  )
		{
			guiCurrentEvent = guiOldEvent;
		}
		else
		{
			// ATE: Check first that some modes are met....
			if ( gCurrentUIMode != HANDCURSOR_MODE && gCurrentUIMode != LOOKCURSOR_MODE && gCurrentUIMode != TALKCURSOR_MODE )
			{
				guiCurrentEvent = M_ON_TERRAIN;
			}
		}
	}

	// Donot display APs if not in combat
	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		gfUIDisplayActionPoints = FALSE;
	}


	// Will set the cursor but only if different
	SetUIMouseCursor( );

	// ATE: Check to reset selected guys....
	if ( gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
	{
		// If not in MOVE_MODE, CONFIRM_MOVE_MODE, RUBBERBAND_MODE, stop....
		if ( gCurrentUIMode != MOVE_MODE && gCurrentUIMode != CONFIRM_MOVE_MODE && gCurrentUIMode != RUBBERBAND_MODE && gCurrentUIMode != ADJUST_STANCE_MODE && gCurrentUIMode != TALKCURSOR_MODE && gCurrentUIMode != LOOKCURSOR_MODE )
		{
			ResetMultiSelection( );
		}
	}

	return( ReturnVal );
}

void ResetCurrentCursorTarget() {
	ErasePath();

	guiCurrentCursorGridNo = NOWHERE;
	gUIFullTarget = NULL;
	guiUIFullTargetFlags = NO_MERC;
}

void UpdateCurrentCursorTarget() {
	INT16  sWorldX;
	INT16  sWorldY;
	if (GetMouseXY(&sWorldX, &sWorldY))
	{
		guiCurrentCursorGridNo = MAPROWCOLTOPOS(sWorldY, sWorldX);
	}
	else
	{
		guiCurrentCursorGridNo = NOWHERE;
	}

	if (guiCurrentCursorGridNo != NOWHERE) {
		// Update pointed on soldier state
		SOLDIERTYPE* const s = FindSoldier(guiCurrentCursorGridNo, FINDSOLDIERSAMELEVEL(gsInterfaceLevel));
		gUIFullTarget        = s;
		guiUIFullTargetFlags = s ? GetSoldierFindFlags(*s) : NO_MERC;
	} else {
		ResetCurrentCursorTarget();
	}
}

void TacticalViewPortMovementCallback(MOUSE_REGION* region, UINT32 reason) {
	if (reason & MSYS_CALLBACK_REASON_MOVE && !(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)) {
		RegisterViewPortPointerPosition(region->MouseXPos, region->MouseYPos);
	}
	// Update cursor state
	if (reason & MSYS_CALLBACK_REASON_LOST_MOUSE) {
		gUIFingersDown = 0;
		if (!IsPointerOnTacticalTouchUI()) {
			if (gCurrentUIMode == PAN_MODE) {
				guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
			}
			ResetCurrentCursorTarget();
		} else {
			SetManualCursorPos(SGPPoint {(UINT16)region->MouseXPos, (UINT16)region->MouseYPos});
			// Dont update target soldier when clicking on touch ui
			return;
		}
	} else if (reason & (MSYS_CALLBACK_REASON_MOVE | MSYS_CALLBACK_REASON_GAIN_MOUSE)) {
		UpdateCurrentCursorTarget();
	}
}

void TacticalViewPortTouchCallback(MOUSE_REGION* region, UINT32 reason) {
	UpdateCurrentCursorTarget();

	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		TacticalViewPortTouchCallbackRT(region, reason);
	}
	else
	{
		TacticalViewPortTouchCallbackTB(region, reason);
	}
}


static void SetUIMouseCursor(void)
{
	BOOLEAN	fForceUpdateNewCursor = FALSE;
	BOOLEAN	fUpdateNewCursor = TRUE;
	static INT16 sOldExitGridNo = NOWHERE;
	static BOOLEAN	fOkForExit = FALSE;


	// Check if we moved from confirm mode on exit arrows
	// If not in move mode, return!
	if ( gCurrentUIMode == MOVE_MODE )
	{
		if ( gfUIConfirmExitArrows )
		{
			if (GetCursorMovementFlags() != MOUSE_STATIONARY)
			{
				gfUIConfirmExitArrows = FALSE;
			}
		}


		if ( gfScrolledToRight && gusMouseXPos >= SCREEN_WIDTH - NO_PX_SHOW_EXIT_CURS )
		{
			gfUIDisplayActionPoints = FALSE;
			ErasePath();

			if (OKForSectorExit(EAST_STRATEGIC_MOVE, 0))
			{
				if ( gfUIConfirmExitArrows )
				{
					guiNewUICursor = CONFIRM_EXIT_EAST_UICURSOR;
				}
				else
				{
					guiNewUICursor = EXIT_EAST_UICURSOR;
				}
			}
			else
			{
				guiNewUICursor = NOEXIT_EAST_UICURSOR;
			}
		}

		if ( gfScrolledToLeft && gusMouseXPos < NO_PX_SHOW_EXIT_CURS )
		{
			gfUIDisplayActionPoints = FALSE;
			ErasePath();

			if (OKForSectorExit(WEST_STRATEGIC_MOVE, 0))
			{
				if ( gfUIConfirmExitArrows )
				{
					guiNewUICursor = CONFIRM_EXIT_WEST_UICURSOR;
				}
				else
				{
					guiNewUICursor = EXIT_WEST_UICURSOR;
				}
			}
			else
			{
				guiNewUICursor = NOEXIT_WEST_UICURSOR;
			}
		}

		if ( gfScrolledToTop && gusMouseYPos <  NO_PX_SHOW_EXIT_CURS )
		{
			gfUIDisplayActionPoints = FALSE;
			ErasePath();

			if (OKForSectorExit(NORTH_STRATEGIC_MOVE, 0))
			{
				if ( gfUIConfirmExitArrows )
				{
					guiNewUICursor = CONFIRM_EXIT_NORTH_UICURSOR;
				}
				else
				{
					guiNewUICursor = EXIT_NORTH_UICURSOR;
				}
			}
			else
			{
				guiNewUICursor = NOEXIT_NORTH_UICURSOR;
			}
		}


		auto comp = !gfIsUsingTouch ? SCREEN_HEIGHT : gViewportRegion.RegionBottomRightY;
		if ( gfScrolledToBottom && gusMouseYPos >= comp - NO_PX_SHOW_EXIT_CURS )
		{
			gfUIDisplayActionPoints = FALSE;
			ErasePath();

			if (OKForSectorExit(SOUTH_STRATEGIC_MOVE, 0))
			{
				if ( gfUIConfirmExitArrows )
				{
					guiNewUICursor = CONFIRM_EXIT_SOUTH_UICURSOR;
				}
				else
				{
					guiNewUICursor = EXIT_SOUTH_UICURSOR;
				}
			}
			else
			{
				guiNewUICursor = NOEXIT_SOUTH_UICURSOR;
			}

			if (gusMouseYPos < comp - NO_PX_SHOW_EXIT_CURS)
			{
				// Define region for viewport
				MSYS_RemoveRegion( &gViewportRegion );

				MSYS_DefineRegion(&gViewportRegion, 0, 0 ,gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y,
							MSYS_PRIORITY_NORMAL,
							VIDEO_NO_CURSOR, TacticalViewPortMovementCallback, TacticalViewPortTouchCallback);


				// Adjust where we blit our cursor!
				gsGlobalCursorYOffset = 0;
				SetCurrentCursorFromDatabase( CURSOR_NORMAL );
			}
			else if (!gfScrollPending && !g_scroll_inertia && !gfIsUsingTouch)
			{
				// Adjust viewport to edge of screen!
				// Define region for viewport
				MSYS_RemoveRegion(&gViewportRegion);
				MSYS_DefineRegion(&gViewportRegion, 0, 0, gsVIEWPORT_END_X, SCREEN_HEIGHT, MSYS_PRIORITY_NORMAL, VIDEO_NO_CURSOR, TacticalViewPortMovementCallback, TacticalViewPortTouchCallback);

				gsGlobalCursorYOffset = SCREEN_HEIGHT - gsVIEWPORT_WINDOW_END_Y;
				SetCurrentCursorFromDatabase(gUICursors[guiNewUICursor].usFreeCursorName);

				gfViewPortAdjustedForSouth = TRUE;
			}
		}
		else
		{
			if ( gfViewPortAdjustedForSouth )
			{
				// Define region for viewport
				MSYS_RemoveRegion( &gViewportRegion );

				MSYS_DefineRegion(&gViewportRegion, 0, 0 ,gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y,
							MSYS_PRIORITY_NORMAL,
							VIDEO_NO_CURSOR, TacticalViewPortMovementCallback, TacticalViewPortTouchCallback);


				// Adjust where we blit our cursor!
				gsGlobalCursorYOffset = 0;
				SetCurrentCursorFromDatabase( CURSOR_NORMAL );

				gfViewPortAdjustedForSouth = FALSE;
			}
		}

		if ( gfUIShowExitExitGrid )
		{
			gfUIDisplayActionPoints = FALSE;
			ErasePath();

			const GridNo usMapPos = guiCurrentCursorGridNo;
			if (usMapPos != NOWHERE)
			{
				const SOLDIERTYPE* const sel = GetSelectedMan();
				if (sel != NULL && sel->bLevel == 0)
				{
					// ATE: Is this place revealed?
					if (GetRoom(usMapPos) == NO_ROOM || gpWorldLevelData[usMapPos].uiFlags & MAPELEMENT_REVEALED)
					{
						if ( sOldExitGridNo != usMapPos )
						{
							fOkForExit = OKForSectorExit((INT8)-1, usMapPos);
							sOldExitGridNo = usMapPos;
						}

						if ( fOkForExit )
						{
							if ( gfUIConfirmExitArrows )
							{
								guiNewUICursor = CONFIRM_EXIT_GRID_UICURSOR;
							}
							else
							{
								guiNewUICursor = EXIT_GRID_UICURSOR;
							}
						}
						else
						{
							guiNewUICursor = NOEXIT_GRID_UICURSOR;
						}
					}
				}
			}
		}
		else
		{
			sOldExitGridNo = NOWHERE;
		}

	}
	else
	{
		gsGlobalCursorYOffset = 0;
	}

	if ( gfDisplayTimerCursor )
	{
		SetUICursor( guiTimerCursorID );

		fUpdateNewCursor = FALSE;

		if ( ( GetJA2Clock( ) - guiTimerLastUpdate ) > guiTimerCursorDelay )
		{
			gfDisplayTimerCursor = FALSE;

			// OK, timer may be different, update...
			fForceUpdateNewCursor = TRUE;
			fUpdateNewCursor = TRUE;
		}
	}

	if ( fUpdateNewCursor )
	{
		if ( !gfTacticalForceNoCursor )
		{
			if ( guiNewUICursor != guiCurrentUICursor || fForceUpdateNewCursor )
			{
				SetUICursor( guiNewUICursor );

				guiCurrentUICursor = guiNewUICursor;
			}
		}
	}
}

void SetUIKeyboardHook( UIKEYBOARD_HOOK KeyboardHookFnc )
{
	gUIKeyboardHook = KeyboardHookFnc;
}


static void ClearEvent(UI_EVENT* pUIEvent)
{
	std::fill(std::begin(pUIEvent->uiParams), std::end(pUIEvent->uiParams), 0);
	pUIEvent->fDoneMenu = FALSE;
	pUIEvent->fFirstTime = TRUE;
	pUIEvent->uiMenuPreviousMode = DONT_CHANGEMODE;
}


void EndMenuEvent( UINT32 uiEvent )
{
	gEvents[ uiEvent ].fDoneMenu = TRUE;

}


static ScreenID UIHandleIDoNothing(UI_EVENT* pUIEvent)
{
	guiNewUICursor = NORMAL_SNAPUICURSOR;

	HideTacticalTouchUI();
	if (gfIsUsingTouch) ResetCurrentCursorTarget();

	return( GAME_SCREEN );
}


static ScreenID UIHandleNewMerc(UI_EVENT* pUIEvent)
{
	static UINT8 ubTemp = 3;
	MERC_HIRE_STRUCT HireMercStruct;
	INT8 bReturnCode;

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos != NOWHERE)
	{
		ubTemp+= 2;

		HireMercStruct = MERC_HIRE_STRUCT{};

		HireMercStruct.ubProfileID = ubTemp;

		//DEF: temp
		HireMercStruct.sSector = gWorldSector;
		HireMercStruct.ubInsertionCode	= INSERTION_CODE_GRIDNO;
		HireMercStruct.usInsertionData	= usMapPos;
		HireMercStruct.fCopyProfileItemsOver = TRUE;
		HireMercStruct.iTotalContractLength = 7;

		//specify when the merc should arrive
		HireMercStruct.uiTimeTillMercArrives = 0;

		//if we succesfully hired the merc
		bReturnCode = HireMerc(HireMercStruct);

		if( bReturnCode == MERC_HIRE_FAILED )
		{
			SLOGD("Merc hire failed:  Either already hired or dislikes you." );
		}
		else if( bReturnCode == MERC_HIRE_OVER_20_MERCS_HIRED )
		{
			SLOGD("Can't hire more than 20 mercs." );
		}
		else
		{
			// Get soldier from profile
			SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubTemp);
			MercArrivesCallback(*pSoldier);
			SelectSoldier(pSoldier, SELSOLDIER_FORCE_RESELECT);
		}

	}
	return( GAME_SCREEN );
}


static ScreenID UIHandleNewBadMerc(UI_EVENT*)
{
	// Get map postion and place the enemy there
	GridNo const map_pos = guiCurrentCursorGridNo;
	if (map_pos == NOWHERE) return GAME_SCREEN;

	// Are we an OK dest?
	if (!IsLocationSittable(map_pos, 0)) return GAME_SCREEN;

	UINT32       const roll = Random(10);
	SoldierClass const sc   = roll < 4 ? SOLDIER_CLASS_ADMINISTRATOR :
					roll < 8 ? SOLDIER_CLASS_ARMY :
					SOLDIER_CLASS_ELITE;
	SOLDIERTYPE* const s = TacticalCreateEnemySoldier(sc);
	if (!s) return GAME_SCREEN;

	// Add soldier strategic info, so it doesn't break the counters
	if (gWorldSector.z == 0)
	{
		SECTORINFO& sector = SectorInfo[gWorldSector.AsByte()];
		switch (s->ubSoldierClass)
		{
			case SOLDIER_CLASS_ADMINISTRATOR:
				++sector.ubNumAdmins;
				++sector.ubAdminsInBattle;
				break;
			case SOLDIER_CLASS_ARMY:
				++sector.ubNumTroops;
				++sector.ubTroopsInBattle;
				break;
			case SOLDIER_CLASS_ELITE:
				++sector.ubNumElites;
				++sector.ubElitesInBattle;
				break;
		}
	}
	else
	{
		if (UNDERGROUND_SECTORINFO* const sector = FindUnderGroundSector(gWorldSector))
		{
			switch (s->ubSoldierClass)
			{
				case SOLDIER_CLASS_ADMINISTRATOR:
					++sector->ubNumAdmins;
					++sector->ubAdminsInBattle;
					break;
				case SOLDIER_CLASS_ARMY:
					++sector->ubNumTroops;
					++sector->ubTroopsInBattle;
					break;
				case SOLDIER_CLASS_ELITE:
					++sector->ubNumElites;
					++sector->ubElitesInBattle;
					break;
			}
		}
	}

	s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
	s->usStrategicInsertionData = map_pos;
	UpdateMercInSector(*s, gWorldSector);
	AllTeamsLookForAll(NO_INTERRUPTS);

	return GAME_SCREEN;
}


static ScreenID UIHandleEnterEditMode(UI_EVENT* pUIEvent)
{
	return( EDIT_SCREEN );
}


ScreenID UIHandleEndTurn(UI_EVENT* pUIEvent)
{
	CancelItemPointer( );

	// If we show tactical touch ui hide it and reset cursor target
	HideTacticalTouchUI();
	if (gfIsUsingTouch) ResetCurrentCursorTarget();

	if ( CheckForEndOfCombatMode( FALSE ) )
	{
		// do nothing...
		return GAME_SCREEN;
	}

	if (CanGameBeSaved())
	{
		guiPreviousOptionScreen = guiCurrentScreen;
		DoAutoSave();
	}

	EndTurn(OUR_TEAM + 1);
	return GAME_SCREEN;
}

static ScreenID UIHandleTestHit(UI_EVENT* pUIEvent)
{
	INT8 bDamage;

	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	if (gUIFullTarget != NULL)
	{
		SOLDIERTYPE* const tgt = gUIFullTarget;

		if ( _KeyDown( SHIFT ) )
		{
			tgt->bBreath -= 30;
			if (tgt->bBreath < 0) tgt->bBreath = 0;
			bDamage = 1;
		}
		else
		{
			if ( Random(2)  )
			{
				bDamage = 20;
			}
			else
			{
				bDamage = 25;
			}
		}

		gTacticalStatus.ubAttackBusyCount++;

		EVENT_SoldierGotHit(tgt, ItemId(1), bDamage, 10, tgt->bDirection, 320, NULL, FIRE_WEAPON_NO_SPECIAL, tgt->bAimShotLocation, NOWHERE);
	}
	return( GAME_SCREEN );
}

void ChangeInterfaceLevel( INT16 sLevel )
{
	// Only if different!
	if ( sLevel == gsInterfaceLevel )
	{
		return;
	}

	gsInterfaceLevel = sLevel;

	if ( gsInterfaceLevel == 1 )
	{
		gsRenderHeight += ROOF_LEVEL_HEIGHT;
		gTacticalStatus.uiFlags |= SHOW_ALL_ROOFS;
		InvalidateWorldRedundency( );
	}
	else if ( gsInterfaceLevel == 0 )
	{
		gsRenderHeight -= ROOF_LEVEL_HEIGHT;
		gTacticalStatus.uiFlags &= (~SHOW_ALL_ROOFS );
		InvalidateWorldRedundency( );
	}

	SetRenderFlags(RENDER_FLAG_FULL);
	// Remove any interactive tiles we could be over!
	BeginCurInteractiveTileCheck();
	gfPlotNewMovement = TRUE;
	ErasePath();
}


ScreenID UIHandleChangeLevel(UI_EVENT* pUIEvent)
{
	if ( gsInterfaceLevel == 0 )
	{
		ChangeInterfaceLevel( 1 );
	}
	else if ( gsInterfaceLevel == 1 )
	{
		ChangeInterfaceLevel( 0 );
	}

	return( GAME_SCREEN );
}


static ScreenID UIHandleSelectMerc(UI_EVENT* pUIEvent)
{
	INT32	iCurrentSquad;

	// Get merc index at mouse and set current selection
	if (gUIFullTarget != NULL)
	{
		iCurrentSquad = CurrentSquad( );

		SelectSoldier(gUIFullTarget, SELSOLDIER_ACKNOWLEDGE | SELSOLDIER_FROM_UI);

		// If different, display message
		if ( CurrentSquad( ) != iCurrentSquad )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(pMessageStrings[ MSG_SQUAD_ACTIVE ], ( CurrentSquad( ) + 1 )) );
		}
	}

	return( GAME_SCREEN );
}


static void SetMovementModeCursor(const SOLDIERTYPE* pSoldier);
static bool UIHandleInteractiveTilesAndItemsOnTerrain(SOLDIERTYPE* pSoldier, INT16 usMapPos, BOOLEAN fUseOKCursor, BOOLEAN fItemsOnlyIfOnIntTiles);


static ScreenID UIHandleMOnTerrain(UI_EVENT* pUIEvent)
{
	BOOLEAN						fSetCursor = FALSE;
	EXITGRID					ExitGrid;

	static INT16			sGridNoForItemsOver;
	static INT8				bLevelForItemsOver;
	static UINT32			uiItemsOverTimer;
	static BOOLEAN		fOverItems;

	HideTacticalTouchUI();

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	gUIActionModeChangeDueToMouseOver = FALSE;

	// If we are a vehicle..... just show an X
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL)
	{
		if (OK_ENTERABLE_VEHICLE(sel))
		{
			if ( !UIHandleOnMerc( TRUE ) )
			{
				guiNewUICursor = FLOATING_X_UICURSOR;
				return( GAME_SCREEN );
			}
		}
	}

	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	if ( !UIHandleOnMerc( TRUE ) )
	{
		// Are we over items...
		const ITEM_POOL* pItemPool = GetItemPool(usMapPos, (UINT8)gsInterfaceLevel);
		if (pItemPool != NULL && IsItemPoolVisible(pItemPool))
		{
			// Are we already in...
			if ( fOverItems )
			{
				// Is this the same level & gridno...
				if ( gsInterfaceLevel == (INT16)bLevelForItemsOver && usMapPos == sGridNoForItemsOver )
				{
					// Check timer...
					if ( ( GetJA2Clock( ) - uiItemsOverTimer ) > 1500 )
					{
						// Change to hand curso mode
						guiPendingOverrideEvent = M_CHANGE_TO_HANDMODE;
						gsOverItemsGridNo = usMapPos;
						gsOverItemsLevel = gsInterfaceLevel;
						fOverItems = FALSE;
					}
				}
				else
				{
					uiItemsOverTimer = GetJA2Clock( );
					bLevelForItemsOver = (INT8)gsInterfaceLevel;
					sGridNoForItemsOver = usMapPos;
				}
			}
			else
			{
				fOverItems = TRUE;

				uiItemsOverTimer = GetJA2Clock( );
				bLevelForItemsOver = (INT8)gsInterfaceLevel;
				sGridNoForItemsOver = usMapPos;
			}
		}
		else
		{
			fOverItems = FALSE;
		}

		if (sel != NULL)
		{
			if (GetExitGrid(usMapPos, &ExitGrid) && sel->bLevel == 0)
			{
				gfUIShowExitExitGrid = TRUE;
			}

			// ATE: Draw invalidc cursor if heights different
			if (gpWorldLevelData[usMapPos].sHeight != gpWorldLevelData[sel->sGridNo].sHeight)
			{
				// ERASE PATH
				ErasePath();

				guiNewUICursor = FLOATING_X_UICURSOR;

				return( GAME_SCREEN );
			}
		}

		// DO SOME CURSOR POSITION FLAGS SETTING
		MouseMoveState const uiCursorFlags = GetCursorMovementFlags();

		if (sel != NULL)
		{
			// Get interactvie tile node
			const LEVELNODE* const pIntNode = GetCurInteractiveTile();

			// Check were we are
			// CHECK IF WE CAN MOVE HERE
			// THIS IS JUST A CRUDE TEST FOR NOW
			if (sel->bLife < OKLIFE)
			{
				// Show reg. cursor
				// GO INTO IDLE MODE
				// guiPendingOverrideEvent = I_CHANGE_TO_IDLE;

				SelectSoldier(FindNextActiveAndAliveMerc(sel, FALSE, FALSE), SELSOLDIER_NONE);
			}
			else if (UIOKMoveDestination(sel, usMapPos) != 1 && pIntNode == NULL)
			{
				// ERASE PATH
				ErasePath();

				guiNewUICursor = CANNOT_MOVE_UICURSOR;

			}
			else
			{
				if (!UIHandleInteractiveTilesAndItemsOnTerrain(sel, usMapPos, FALSE, TRUE))
				{
					// Are we in combat?
					if (gTacticalStatus.uiFlags & INCOMBAT)
					{
						// If so, draw path, etc
						fSetCursor = HandleUIMovementCursor(sel, uiCursorFlags, usMapPos, MOVEUI_TARGET_NONE);
					}
					else
					{
						// Donot draw path until confirm
						fSetCursor = TRUE;

						// If so, draw path, etc
						fSetCursor =  HandleUIMovementCursor(sel, uiCursorFlags, usMapPos, MOVEUI_TARGET_NONE);

						//ErasePath();
					}

				}
				else
				{
					fSetCursor = TRUE;
				}
			}
		}
		else
		{
			// IF GUSSELECTEDSOLDIER != NOSOLDIER
			guiNewUICursor = NORMAL_SNAPUICURSOR;
		}
	}
	else
	{
		if( ValidQuickExchangePosition( ) )
		{
			// Do new cursor!
			guiNewUICursor = EXCHANGE_PLACES_UICURSOR;
		}
	}

	//if (fSetCursor && guiNewUICursor != ENTER_VEHICLE_UICURSOR)
	if (fSetCursor && !gfBeginVehicleCursor)
	{
		SetMovementModeCursor(sel);
	}

	return( GAME_SCREEN );
}


static ScreenID UIHandleMovementMenu(UI_EVENT* pUIEvent)
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;

	// Popup Menu
	if ( pUIEvent->fFirstTime )
	{
		//Pop-up menu
		PopupMovementMenu( pUIEvent );

		// Change cusror to normal
		guiNewUICursor = NORMAL_FREEUICURSOR;

	}

	// Check for done flag
	if ( pUIEvent->fDoneMenu )
	{
			PopDownMovementMenu( );

			// Excecute command, if user hit a button
			if ( pUIEvent->uiParams[1] == TRUE )
			{
				if ( pUIEvent->uiParams[ 2 ] == MOVEMENT_MENU_LOOK )
				{
					guiPendingOverrideEvent = LC_CHANGE_TO_LOOK;
				}
				else if ( pUIEvent->uiParams[ 2 ] == MOVEMENT_MENU_HAND )
				{
					guiPendingOverrideEvent = HC_ON_TERRAIN;
				}
				else if ( pUIEvent->uiParams[ 2 ] == MOVEMENT_MENU_ACTIONC )
				{
					guiPendingOverrideEvent = M_CHANGE_TO_ACTION;
				}
				else if ( pUIEvent->uiParams[ 2 ] == MOVEMENT_MENU_TALK )
				{
					guiPendingOverrideEvent = T_CHANGE_TO_TALKING;
				}
				else
				{
					// Change stance based on params!
					switch( pUIEvent->uiParams[ 0 ] )
					{
						case MOVEMENT_MENU_RUN:

							if (sel->usUIMovementMode != WALKING && sel->usUIMovementMode != RUNNING)
							{
								UIHandleSoldierStanceChange(sel, ANIM_STAND);
								sel->fUIMovementFast = 1;
							}
							else
							{
								sel->fUIMovementFast = 1;
								sel->usUIMovementMode = RUNNING;
								gfPlotNewMovement = TRUE;
							}
							break;

						case MOVEMENT_MENU_WALK:  UIHandleSoldierStanceChange(sel, ANIM_STAND);  break;
						case MOVEMENT_MENU_SWAT:  UIHandleSoldierStanceChange(sel, ANIM_CROUCH); break;
						case MOVEMENT_MENU_PRONE: UIHandleSoldierStanceChange(sel, ANIM_PRONE);  break;
					}

					guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
				}
			}
	}

	return( GAME_SCREEN );
}


static ScreenID UIHandlePositionMenu(UI_EVENT* pUIEvent)
{

	return( GAME_SCREEN );
}


static ScreenID UIHandleAOnTerrain(UI_EVENT* pUIEvent)
{
	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	if ( gpItemPointer != NULL )
	{
		return( GAME_SCREEN );
	}

	HideTacticalTouchUI();

	// Get soldier to determine range
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;

	SoldierSP selSoldier = GetSoldier(sel);

	// ATE: Add stuff here to display a system message if we are targeting smeothing and
	//  are out of range.
	// Are we using a gun?
	if (GetActionModeCursor(sel) == TARGETCURS)
	{
		SetActionModeDoorCursorText();

		// Yep, she's a gun.
		// Are we in range?
		if (!InRange(sel, usMapPos))
		{
			// Are we over a guy?
			const SOLDIERTYPE* const tgt = gUIFullTarget;
			if (tgt != NULL)
			{
				// No, ok display message IF this is the first time at this gridno
				if (gsOutOfRangeGridNo != tgt->sGridNo || gubOutOfRangeMerc != SOLDIER2ID(sel))
				{
					// Display
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[OUT_OF_RANGE_STRING]);

					//PlayJA2Sample(TARGET_OUT_OF_RANGE, MIDVOLUME, 1, MIDDLEPAN);

					// Set
					gsOutOfRangeGridNo = tgt->sGridNo;
					gubOutOfRangeMerc  = SOLDIER2ID(sel);
				}
			}
		}
	}

	guiNewUICursor = GetProperItemCursor(sel, usMapPos, FALSE);

	// Show UI ON GUY
	UIHandleOnMerc( FALSE );

	// If we are in realtime, and in a stationary animation, follow!
	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		if (gAnimControl[sel->usAnimState].uiFlags & ANIM_STATIONARY && !selSoldier->hasPendingAction())
		{
			// Check if we have a shot waiting!
			if (gUITargetShotWaiting) guiPendingOverrideEvent = CA_MERC_SHOOT;
			gUITargetReady = TRUE;
		}
		else
		{
			gUITargetReady = FALSE;
		}
	}

	return( GAME_SCREEN );
}


static ScreenID UIHandleMChangeToAction(UI_EVENT* pUIEvent)
{
	gUITargetShotWaiting = FALSE;

	EndPhysicsTrajectoryUI( );

	//guiNewUICursor = CONFIRM_MOVE_UICURSOR;

	return( GAME_SCREEN );
}


static ScreenID UIHandleMChangeToHandMode(UI_EVENT* pUIEvent)
{
	ErasePath();

	return( GAME_SCREEN );
}


static ScreenID UIHandleAChangeToMove(UI_EVENT* pUIEvent)
{
	// gsOutOfRangeGridNo = NOWHERE;

	gfPlotNewMovement = TRUE;

	return( GAME_SCREEN );
}


static void SetConfirmMovementModeCursor(SOLDIERTYPE* pSoldier, BOOLEAN fFromMove);


static ScreenID UIHandleCWait(UI_EVENT* event)
{
	GridNo const map_pos = guiCurrentCursorGridNo;
	if (map_pos == NOWHERE) return GAME_SCREEN;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (!sel) return GAME_SCREEN;

	LEVELNODE const* const int_tile = GetCurInteractiveTile();

	if (int_tile && gpInvTileThatCausedMoveConfirm != int_tile)
	{ // Get out og this mode
		guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
		return GAME_SCREEN;
	}

	MouseMoveState const cursor_state = GetCursorMovementFlags();

	ShowTacticalTouchUI(TacticalTouchUIMode::ConfirmMove);

	if (int_tile)
	{
		HandleUIMovementCursor(sel, cursor_state, map_pos, MOVEUI_TARGET_INTTILES);

		guiNewUICursor = GetInteractiveTileCursor(guiNewUICursor, TRUE);

		// Make red tile under spot, if we've previously found one
		if (gfUIHandleShowMoveGrid) gfUIHandleShowMoveGrid = 2;

		return GAME_SCREEN;
	}

	gfUIDisplayActionPoints = TRUE;

	if (!EnoughPoints(sel, gsCurrentActionPoints, 0, FALSE))
	{
		gfUIDisplayActionPointsInvalid = TRUE;
	}

	SetConfirmMovementModeCursor(sel, FALSE);

	// If we are not in combat, draw path here!
	if (gfIsUsingTouch || !(gTacticalStatus.uiFlags & INCOMBAT))
	{
		HandleUIMovementCursor(sel, cursor_state, map_pos, MOVEUI_TARGET_NONE);
	}

	return GAME_SCREEN;
}


static BOOLEAN HandleMultiSelectionMove(INT16 sDestGridNo);


// NOTE, ONCE AT THIS FUNCTION, WE HAVE ASSUMED TO HAVE CHECKED FOR ENOUGH APS THROUGH
// SelectedMercCanAffordMove
static ScreenID UIHandleCMoveMerc(UI_EVENT* pUIEvent)
{
	INT16 sDestGridNo;
	INT16 sActionGridNo;
	STRUCTURE *pStructure;
	UINT8 ubDirection;
	BOOLEAN fAllMove;
	LEVELNODE *pIntTile;
	INT16 sIntTileGridNo;
	BOOLEAN fOldFastMove;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL)
	{
		SoldierSP selSoldier = GetSoldier(sel);

		fAllMove = gfUIAllMoveOn;
		gfUIAllMoveOn = FALSE;

		const GridNo usMapPos = guiCurrentCursorGridNo;
		if (usMapPos == NOWHERE) return GAME_SCREEN;

		// ERASE PATH
		ErasePath();

		if ( fAllMove )
		{
			gfGetNewPathThroughPeople = TRUE;

			// Loop through all mercs and make go!
			// TODO: Only our squad!
			FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
			{
				SoldierSP soldier = GetSoldier(pSoldier);

				if (OkControllableMerc(pSoldier) && pSoldier->bAssignment == CurrentSquad() && !pSoldier->fMercAsleep)
				{
					// If we can't be controlled, returninvalid...
					if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
					{
						if ( !CanRobotBeControlled( pSoldier ) )
						{
							continue;
						}
					}

					AdjustNoAPToFinishMove( pSoldier, FALSE );

					fOldFastMove = pSoldier->fUIMovementFast;

					if ( fAllMove == 2 )
					{
						pSoldier->fUIMovementFast = TRUE;
						pSoldier->usUIMovementMode = RUNNING;
					}
					else
					{
						pSoldier->fUIMovementFast = FALSE;
						pSoldier->usUIMovementMode =  GetMoveStateBasedOnStance( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight );
					}

					soldier->removePendingAction();

					//if ( !( gTacticalStatus.uiFlags & INCOMBAT ) && ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_MOVING ) )
					//{
					//	pSoldier->sRTPendingMovementGridNo = usMapPos;
					//	pSoldier->usRTPendingMovementAnim  = pSoldier->usUIMovementMode;
					//}
					//else
					if ( EVENT_InternalGetNewSoldierPath( pSoldier, usMapPos, pSoldier->usUIMovementMode, TRUE, FALSE ) )
					{
						InternalDoMercBattleSound( pSoldier, BATTLE_SOUND_OK1, BATTLE_SND_LOWER_VOLUME );
					}
					else
					{
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ NO_PATH_FOR_MERC ], pSoldier->name) );
					}

					pSoldier->fUIMovementFast = fOldFastMove;

				}
			}
			gfGetNewPathThroughPeople = FALSE;

			// RESET MOVE FAST FLAG
			SetConfirmMovementModeCursor(sel, TRUE);

			gfUIAllMoveOn = 0;

		}
		else
		{
			// FOR REALTIME - DO MOVEMENT BASED ON STANCE!
			if (!(gTacticalStatus.uiFlags & INCOMBAT))
			{
				sel->usUIMovementMode = GetMoveStateBasedOnStance(sel, gAnimControl[sel->usAnimState].ubEndHeight);
			}

			sDestGridNo = usMapPos;

			// Get structure info for in tile!
			pIntTile = GetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure);

			// We should not have null here if we are given this flag...
			if (pIntTile != NULL)
			{
				sActionGridNo = FindAdjacentGridEx(sel, sIntTileGridNo, &ubDirection, NULL, FALSE, TRUE);
				if (sActionGridNo != -1)
				{
					SetUIBusy(sel);

					// Set dest gridno
					sDestGridNo = sActionGridNo;

					// check if we are at this location
					if (sel->sGridNo == sDestGridNo)
					{
						StartInteractiveObject(sIntTileGridNo, *pStructure, *sel, ubDirection);
						InteractWithOpenableStruct(*sel, *pStructure, ubDirection);
						return GAME_SCREEN;
					}
				}
				else
				{
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[NO_PATH]);
					return GAME_SCREEN;
				}
			}

			SetUIBusy(sel);

			if (!(gTacticalStatus.uiFlags & INCOMBAT))
			{
				// RESET MOVE FAST FLAG
				SetConfirmMovementModeCursor(sel, TRUE);

				if (!gTacticalStatus.fAtLeastOneGuyOnMultiSelect)
				{
					sel->fUIMovementFast = FALSE;
				}
			}

			if (gTacticalStatus.fAtLeastOneGuyOnMultiSelect && pIntTile == NULL)
			{
				HandleMultiSelectionMove(sDestGridNo);
			}
			else
			{
				sel->bReverse = gUIUseReverse;

				selSoldier->removePendingAction();

				EVENT_InternalGetNewSoldierPath(sel, sDestGridNo, sel->usUIMovementMode, TRUE, sel->fNoAPToFinishMove);

				if (sel->ubPathDataSize > 5)
				{
					DoMercBattleSound(sel, BATTLE_SOUND_OK1);
				}

				// HANDLE ANY INTERACTIVE OBJECTS HERE!
				if (pIntTile != NULL)
				{
					StartInteractiveObject(sIntTileGridNo, *pStructure, *sel, ubDirection);
				}
			}
		}
	}
	return( GAME_SCREEN );
}


static ScreenID UIHandleMCycleMoveAll(UI_EVENT* pUIEvent)
{
	if (GetSelectedMan() == NULL)
		return GAME_SCREEN;

	if ( gfUICanBeginAllMoveCycle )
	{
		gfUIAllMoveOn = TRUE;
		gfUICanBeginAllMoveCycle = FALSE;
	}
	return( GAME_SCREEN );
}


static ScreenID UIHandleMCycleMovement(UI_EVENT* pUIEvent)
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;

	gfUIAllMoveOn = FALSE;

	if (sel->ubBodyType == ROBOTNOWEAPON)
	{
		sel->usUIMovementMode = WALKING;
		gfPlotNewMovement = TRUE;
		return GAME_SCREEN;
	}

	for (;;)
	{
		// Cycle gmovement state
		switch (sel->usUIMovementMode)
		{
			INT16 move_mode;

			case RUNNING:
				move_mode = WALKING;
				goto test_mode;
			case WALKING:
				move_mode = SWATTING;
				goto test_mode;
			case SWATTING:
				move_mode = CRAWLING;
				goto test_mode;

			case CRAWLING:
				sel->fUIMovementFast = 1;
				move_mode            = RUNNING;
				goto test_mode;

test_mode:
				sel->usUIMovementMode = move_mode;
				if (IsValidMovementMode(sel, move_mode))
				{
					gfPlotNewMovement = TRUE;
					return GAME_SCREEN;
				}
		}
	}
}


static ScreenID UIHandleCOnTerrain(UI_EVENT* pUIEvent)
{

	return( GAME_SCREEN );
}


static ScreenID UIHandleMAdjustStanceMode(UI_EVENT* pUIEvent)
{
	INT32 iPosDiff;
	static UINT16 gusAnchorMouseY;
	static UINT16 usOldMouseY;
	static BOOLEAN ubNearHeigherLevel;
	static BOOLEAN ubNearLowerLevel;
	static UINT8 ubUpHeight, ubDownDepth;
	static UINT32 uiOldShowUPDownArrows;

	// Change cusror to normal
	guiNewUICursor = NO_UICURSOR;


	if ( pUIEvent->fFirstTime )
	{
		gusAnchorMouseY = gusMouseYPos;
		usOldMouseY = gusMouseYPos;
		ubNearHeigherLevel = FALSE;
		ubNearLowerLevel = FALSE;

		guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
		uiOldShowUPDownArrows = guiShowUPDownArrows;

		gbAdjustStanceDiff = 0;
		gbClimbID = 0;

		gfIgnoreScrolling = TRUE;

		// Get soldier current height of animation
		const SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel != NULL)
		{
			if (FindHigherLevel(sel)) ubNearHeigherLevel = TRUE;
			if (FindLowerLevel(sel))  ubNearLowerLevel   = TRUE;

			switch (gAnimControl[sel->usAnimState].ubEndHeight)
			{
				case ANIM_STAND:
					if ( ubNearHeigherLevel )
					{
						ubUpHeight	= 1;
						ubDownDepth = 2;
					}
					else if ( ubNearLowerLevel )
					{
						ubUpHeight	= 0;
						ubDownDepth = 3;
					}
					else
					{
						ubUpHeight	= 0;
						ubDownDepth = 2;
					}
					break;

				case ANIM_CROUCH:
					if ( ubNearHeigherLevel )
					{
						ubUpHeight	= 2;
						ubDownDepth = 1;
					}
					else if ( ubNearLowerLevel )
					{
						ubUpHeight	= 1;
						ubDownDepth = 2;
					}
					else
					{
						ubUpHeight	= 1;
						ubDownDepth = 1;
					}
					break;

				case ANIM_PRONE:
					if ( ubNearHeigherLevel )
					{
						ubUpHeight	= 3;
						ubDownDepth = 0;
					}
					else if ( ubNearLowerLevel )
					{
						ubUpHeight	= 2;
						ubDownDepth = 1;
					}
					else
					{
						ubUpHeight	= 2;
						ubDownDepth = 0;
					}
					break;
			}


		}

	}

	// Check if delta X has changed alot since last time
	iPosDiff = std::abs((INT32)(usOldMouseY - gusMouseYPos));

	//guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
	guiShowUPDownArrows = uiOldShowUPDownArrows;

	{
		if ( gusAnchorMouseY > gusMouseYPos )
		{
			if (GetSelectedMan() != NULL)
			{
					if ( iPosDiff < GO_MOVE_ONE && ubUpHeight >= 1 )
					{
						// Change arrows to move down arrow + show
						//guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_Y;
						guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
						gbAdjustStanceDiff = 0;
						gbClimbID = 0;
					}
					else if ( iPosDiff > GO_MOVE_ONE && iPosDiff < GO_MOVE_TWO && ubUpHeight >= 1 )
					{
						//guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_G;
						if ( ubUpHeight == 1 && ubNearHeigherLevel )
						{
							guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_CLIMB;
							gbClimbID = 1;
						}
						else
						{
							guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_Y;
							gbClimbID = 0;
						}
						gbAdjustStanceDiff = 1;
					}
					else if ( iPosDiff >= GO_MOVE_TWO && iPosDiff < GO_MOVE_THREE && ubUpHeight >= 2 )
					{
						if ( ubUpHeight == 2 && ubNearHeigherLevel )
						{
							guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_CLIMB;
							gbClimbID = 1;
						}
						else
						{
							guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_YY;
							gbClimbID = 0;
						}
						gbAdjustStanceDiff = 2;
					}
					else if ( iPosDiff >= GO_MOVE_THREE && ubUpHeight >= 3 )
					{
						if ( ubUpHeight == 3 && ubNearHeigherLevel )
						{
							guiShowUPDownArrows = ARROWS_SHOW_UP_ABOVE_CLIMB;
							gbClimbID = 1;
						}
					}
			}

		}

		if ( gusAnchorMouseY < gusMouseYPos )
		{
			if (GetSelectedMan() != NULL)
			{
					if ( iPosDiff < GO_MOVE_ONE && ubDownDepth >= 1 )
					{
						// Change arrows to move down arrow + show
						//guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_Y;
						guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE | ARROWS_SHOW_UP_BESIDE;
						gbAdjustStanceDiff = 0;
						gbClimbID = 0;

					}
					else if ( iPosDiff >= GO_MOVE_ONE && iPosDiff < GO_MOVE_TWO && ubDownDepth >= 1 )
					{
//						guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_G;
							if ( ubDownDepth == 1 && ubNearLowerLevel )
							{
								guiShowUPDownArrows = ARROWS_SHOW_DOWN_CLIMB;
								gbClimbID = -1;
							}
							else
							{
								guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_Y;
								gbClimbID = 0;
							}
							gbAdjustStanceDiff = -1;
					}
					else if ( iPosDiff > GO_MOVE_TWO && iPosDiff < GO_MOVE_THREE && ubDownDepth >= 2 )
					{
						//guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_GG;
						if ( ubDownDepth == 2 && ubNearLowerLevel )
						{
							guiShowUPDownArrows = ARROWS_SHOW_DOWN_CLIMB;
							gbClimbID = -1;
						}
						else
						{
							guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_YY;
							gbClimbID = 0;
						}
						gbAdjustStanceDiff = -2;
					}
					else if ( iPosDiff > GO_MOVE_THREE && ubDownDepth >= 3 )
					{
						//guiShowUPDownArrows = ARROWS_SHOW_DOWN_BELOW_GG;
						if ( ubDownDepth == 3 && ubNearLowerLevel )
						{
							guiShowUPDownArrows = ARROWS_SHOW_DOWN_CLIMB;
							gbClimbID = -1;
						}
					}

			}

		}

	}

	uiOldShowUPDownArrows = guiShowUPDownArrows;

	return( GAME_SCREEN );
}


static ScreenID UIHandleAChangeToConfirmAction(UI_EVENT* pUIEvent)
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL) HandleLeftClickCursor(sel);

	ResetBurstLocations( );

	return( GAME_SCREEN );
}


static ScreenID UIHandleCAOnTerrain(UI_EVENT* pUIEvent)
{
	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;

	guiNewUICursor = GetProperItemCursor(sel, usMapPos, TRUE);
	UIHandleOnMerc( FALSE );

	switch (GetActionModeCursor(sel)) {
		case TARGETCURS:
			ShowTacticalTouchUI(TacticalTouchUIMode::ConfirmShoot);
			break;
		case INVALIDCURS:
			HideTacticalTouchUI();
			break;
		default:
			ShowTacticalTouchUI(TacticalTouchUIMode::ConfirmAction);
			break;
	}

	return( GAME_SCREEN );
}


static void UIHandleMercAttack(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pTargetSoldier, UINT16 usMapPos)
{
	INT16 sTargetGridNo;
	INT8 bTargetLevel;
	LEVELNODE *pIntNode;
	STRUCTURE *pStructure;
	INT16 sGridNo, sNewGridNo;

	// get cursor
	ItemCursor const ubItemCursor = GetActionModeCursor(pSoldier);

	if ( !(gTacticalStatus.uiFlags & INCOMBAT) && pTargetSoldier && GCM->getItem(pSoldier->inv[ HANDPOS ].usItem)->isWeapon() )
	{
		if ( NPCFirstDraw( pSoldier, pTargetSoldier ) )
		{
			// go into turnbased for that person
			CancelAIAction(pTargetSoldier);
			AddToShouldBecomeHostileOrSayQuoteList(pTargetSoldier);
			//MakeCivHostile( pTargetSoldier, 2 );
			//TriggerNPCWithIHateYouQuote( pTargetSoldier->ubProfile );
			return;
		}
	}

	// Set aim time to one in UI
	pSoldier->bAimTime = (pSoldier->bShownAimTime/2);

	// ATE: Check if we are targeting an interactive tile, and adjust gridno accordingly...
	pIntNode = GetCurInteractiveTileGridNoAndStructure( &sGridNo, &pStructure );

	if ( pTargetSoldier != NULL )
	{
		sTargetGridNo = pTargetSoldier->sGridNo;
		bTargetLevel  = pTargetSoldier->bLevel;
	}
	else
	{
		sTargetGridNo = usMapPos;
		bTargetLevel  = (INT8)gsInterfaceLevel;

		if ( pIntNode != NULL )
		{
			// Change gridno....
			sTargetGridNo = sGridNo;
		}
	}

	// here, change gridno if we're targeting ourselves....
	if ( pIntNode != NULL  )
	{
		// Are we in the same gridno?
		if ( sGridNo == pSoldier->sGridNo && ubItemCursor != AIDCURS )
		{
			// Get orientation....
			switch( pStructure->ubWallOrientation )
			{
				case OUTSIDE_TOP_LEFT:
				case INSIDE_TOP_LEFT:

					sNewGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );
					break;

				case OUTSIDE_TOP_RIGHT:
				case INSIDE_TOP_RIGHT:

					sNewGridNo = NewGridNo( sGridNo, DirectionInc( EAST ) );
					break;

				default:
					sNewGridNo = sGridNo;
			}

			// Set target gridno to this one...
			sTargetGridNo = sNewGridNo;

			// now set target cube height
			// CJC says to hardcode this value :)
			pSoldier->bTargetCubeLevel = 2;
		}
		else
		{
			// ATE: Refine this a bit - if we have nobody as a target...
			if ( pTargetSoldier == NULL )
			{
				sTargetGridNo = sGridNo;
			}
		}
	}


	// Cannot be fire if we are already in a fire animation....
	// this is to stop the shooting trigger/happy duded from contiously pressing fire...
	if ( !(gTacticalStatus.uiFlags & INCOMBAT) )
	{
		if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_FIRE )
		{
			return;
		}
	}

	// If in turn-based mode - return to movement
	if ( ( gTacticalStatus.uiFlags & INCOMBAT ) )
	{
		// Reset some flags for cont move...
		pSoldier->sFinalDestination = pSoldier->sGridNo;
		pSoldier->bGoodContPath = FALSE;
		//guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
	}


	ItemHandleResult iHandleReturn;
	if (pSoldier->bWeaponMode == WM_ATTACHED)
	{
		iHandleReturn = HandleItem( pSoldier, sTargetGridNo, bTargetLevel, UNDER_GLAUNCHER, TRUE );
	}
	else
	{
		iHandleReturn = HandleItem( pSoldier, sTargetGridNo, bTargetLevel, pSoldier->inv[ HANDPOS ].usItem, TRUE );
	}

	if ( iHandleReturn < 0 )
	{
		if ( iHandleReturn == ITEM_HANDLE_RELOADING )
		{
			guiNewUICursor = ACTION_TARGET_RELOADING;
			return;
		}

		if ( iHandleReturn == ITEM_HANDLE_NOROOM )
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, pMessageStrings[ MSG_CANT_FIRE_HERE ] );
			return;
		}
	}


	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		HandleUICursorRTFeedback( pSoldier );
	}

	gfUIForceReExamineCursorData = TRUE;
}


static void AttackRequesterCallback(MessageBoxReturnValue const bExitValue)
{
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		gTacticalStatus.ubLastRequesterTargetID = gpRequesterTargetMerc->ubProfile;

		UIHandleMercAttack( gpRequesterMerc , gpRequesterTargetMerc, gsRequesterGridNo );
	}
}


static ScreenID UIHandleCAMercShoot(UI_EVENT* pUIEvent)
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;
	// preinit to prevent false ap costs; actions which dont charge turning ap
	// are coming after this line and put the value to true
	sel->fDontChargeTurningAPs = FALSE;

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	SOLDIERTYPE* const tgt = gUIFullTarget;
	if (tgt != NULL)
	{
		// If this is one of our own guys.....pop up requiester...
		if ((tgt->bTeam == OUR_TEAM || tgt->bTeam == MILITIA_TEAM) &&
			GCM->getItem(sel->inv[HANDPOS].usItem)->getItemClass() != IC_MEDKIT &&
			sel->inv[HANDPOS].usItem != GAS_CAN &&
			gTacticalStatus.ubLastRequesterTargetID != tgt->ubProfile &&
			tgt != sel)
		{
			gpRequesterMerc = sel;
			gpRequesterTargetMerc = tgt;
			gsRequesterGridNo = usMapPos;

			ST::string zStr = st_format_printf(TacticalStr[ATTACK_OWN_GUY_PROMPT], tgt->name);
			DoMessageBox(MSG_BOX_BASIC_STYLE, zStr, GAME_SCREEN, MSG_BOX_FLAG_YESNO, AttackRequesterCallback, NULL);
			return GAME_SCREEN;
		}
	}

	UIHandleMercAttack(sel, tgt, usMapPos);

	return( GAME_SCREEN );
}


static ScreenID UIHandleAEndAction(UI_EVENT* pUIEvent)
{
	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL)
	{
		if (!(gTacticalStatus.uiFlags & INCOMBAT))
		{
			if ( gUITargetReady )
			{
				// Move to proper stance + direction!
				SoldierReadyWeapon(sel, usMapPos, TRUE); // UNReady weapon
				gUITargetReady = FALSE;
			}

		}
	}
	return( GAME_SCREEN );
}


static ScreenID UIHandleCAEndConfirmAction(UI_EVENT* pUIEvent)
{
	return GAME_SCREEN;
}


static ScreenID UIHandleIOnTerrain(UI_EVENT* pUIEvent)
{
	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	if ( !UIHandleOnMerc( TRUE ) )
	{
		// Check if dest is OK
		//if ( !NewOKDestination( usMapPos, FALSE ) || IsRoofVisible( usMapPos ) )
		////{
		//	guiNewUICursor = CANNOT_MOVE_UICURSOR;
		//}
		//else
		{
			guiNewUICursor = NORMAL_SNAPUICURSOR;
		}
	}

	return( GAME_SCREEN );
}


static ScreenID UIHandleIChangeToIdle(UI_EVENT* pUIEvent)
{
	return( GAME_SCREEN );
}


static UINT8 GetAdjustedAnimHeight(UINT8 ubAnimHeight, INT8 bChange);


static ScreenID UIHandlePADJAdjustStance(UI_EVENT* pUIEvent)
{
	guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

	gfIgnoreScrolling = FALSE;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL || gbAdjustStanceDiff == 0) return GAME_SCREEN;

	if (gbClimbID	== 1)
	{
		BeginSoldierClimbUpRoof(sel);
	}
	else if (gbClimbID == -1)
	{
		BeginSoldierClimbDownRoof(sel);
	}
	else
	{
		const UINT8 ubNewStance = GetAdjustedAnimHeight(gAnimControl[sel->usAnimState].ubEndHeight, gbAdjustStanceDiff);
		// Set state to result
		UIHandleSoldierStanceChange(sel, ubNewStance);
	}

	return( GAME_SCREEN );
}


static UINT8 GetAdjustedAnimHeight(UINT8 ubAnimHeight, INT8 bChange)
{
	UINT8 ubNewAnimHeight = ubAnimHeight;

	if ( ubAnimHeight == ANIM_STAND )
	{
		if ( bChange == -1 )
		{
			ubNewAnimHeight = ANIM_CROUCH;
		}
		if ( bChange == -2 )
		{
			ubNewAnimHeight = ANIM_PRONE;
		}
		if ( bChange == 1 )
		{
			ubNewAnimHeight = 50;
		}
	}
	else if ( ubAnimHeight == ANIM_CROUCH )
	{
		if ( bChange == 1 )
		{
			ubNewAnimHeight = ANIM_STAND;
		}
		if ( bChange == -1 )
		{
			ubNewAnimHeight = ANIM_PRONE;
		}
		if ( bChange == -2 )
		{
			ubNewAnimHeight = 55;
		}
	}
	else if ( ubAnimHeight == ANIM_PRONE )
	{
		if ( bChange == -1 )
		{
			ubNewAnimHeight = 55;
		}
		if ( bChange == 1 )
		{
			ubNewAnimHeight = ANIM_CROUCH;
		}
		if ( bChange == 2 )
		{
			ubNewAnimHeight = ANIM_STAND;
		}
	}

	return( ubNewAnimHeight );
}


BOOLEAN SelectedMercCanAffordAttack( )
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return FALSE;

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	// Get cursor value
	ItemCursor const ubItemCursor = GetActionModeCursor(sel);
	switch (ubItemCursor)
	{
		case INVALIDCURS: return FALSE;
		case BOMBCURS:    return EnoughPoints(sel, GetTotalAPsToDropBomb(sel, usMapPos), 0, TRUE);
		case REMOTECURS:  return EnoughPoints(sel, GetAPsToUseRemote(sel), 0, TRUE);

		default:
		{
			// Look for a soldier at this position
			const SOLDIERTYPE* const tgt           = gUIFullTarget;
			const INT16              sTargetGridNo = (tgt != NULL ? tgt->sGridNo : usMapPos);
			const INT16              sAPCost       = CalcTotalAPsToAttack(sel, sTargetGridNo, TRUE, sel->bShownAimTime / 2);

			if (EnoughPoints(sel, sAPCost, 0, TRUE)) return TRUE;

			// Play curse....
			DoMercBattleSound(sel, BATTLE_SOUND_CURSE1);
			return FALSE;
		}
	}
}


BOOLEAN SelectedMercCanAffordMove(  )
{
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return FALSE;

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	// IF WE ARE OVER AN INTERACTIVE TILE, GIVE GRIDNO OF POSITION
	if (GetCurInteractiveTile() != NULL)
	{
		// CHECK APS
		return EnoughPoints(sel, gsCurrentActionPoints, 0, TRUE);
	}

	// Take the first direction!
	UINT16 sAPCost = PtsToMoveDirection(sel, guiPathingData[0]);
	sAPCost += GetAPsToChangeStance(sel, gAnimControl[sel->usUIMovementMode].ubHeight);

	if (EnoughPoints(sel, sAPCost, 0, TRUE)) return TRUE;

	// OK, remember where we were trying to get to.....
	sel->sContPathLocation = usMapPos;
	sel->bGoodContPath     = TRUE;

	return( FALSE );
}


static void RemoveTacticalCursor(void)
{
	guiNewUICursor = NO_UICURSOR;
	ErasePath();
}


static ScreenID UIHandleHCOnTerrain(UI_EVENT* pUIEvent)
{
	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;

	// If we are out of breath, no cursor...
	if (sel->bBreath < OKBREATH && sel->bCollapsed)
	{
		guiNewUICursor = INVALID_ACTION_UICURSOR;
	}
	else
	{
		if (gsOverItemsGridNo != NOWHERE && (usMapPos != gsOverItemsGridNo || gsInterfaceLevel != gsOverItemsLevel))
		{
			gsOverItemsGridNo = NOWHERE;
			guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
		}
		else
		{
			ShowTacticalTouchUI(TacticalTouchUIMode::ConfirmAction);
			guiNewUICursor = NORMALHANDCURSOR_UICURSOR;
			UIHandleInteractiveTilesAndItemsOnTerrain(sel, usMapPos, TRUE, FALSE);
		}
	}
	return( GAME_SCREEN );
}


static ScreenID UIHandleHCGettingItem(UI_EVENT* pUIEvent)
{
	guiNewUICursor = NORMAL_FREEUICURSOR;

	return( GAME_SCREEN );
}


static ScreenID UIHandleTATalkingMenu(UI_EVENT* pUIEvent)
{
	guiNewUICursor = NORMAL_FREEUICURSOR;

	HideTacticalTouchUI();

	return( GAME_SCREEN );
}


static ScreenID UIHandleEXExitSectorMenu(UI_EVENT* pUIEvent)
{
	guiNewUICursor = NORMAL_FREEUICURSOR;

	HideTacticalTouchUI();

	return( GAME_SCREEN );
}


static ScreenID UIHandleOpenDoorMenu(UI_EVENT* pUIEvent)
{
	guiNewUICursor = NORMAL_FREEUICURSOR;

	HideTacticalTouchUI();

	return( GAME_SCREEN );
}


void ToggleHandCursorMode(UIEventKind* const puiNewEvent)
{
	// Toggle modes
	if ( gCurrentUIMode == HANDCURSOR_MODE )
	{
		*puiNewEvent = A_CHANGE_TO_MOVE;
	}
	else
	{

		*puiNewEvent = M_CHANGE_TO_HANDMODE;
	}
}


void ToggleTalkCursorMode(UIEventKind* const puiNewEvent)
{
	// Toggle modes
	if ( gCurrentUIMode == TALKCURSOR_MODE )
	{
		*puiNewEvent = A_CHANGE_TO_MOVE;
	}
	else
	{
		*puiNewEvent = T_CHANGE_TO_TALKING;
	}
}


void ToggleLookCursorMode()
{
	guiPendingOverrideEvent = gCurrentUIMode == LOOKCURSOR_MODE ? A_CHANGE_TO_MOVE :
							LC_CHANGE_TO_LOOK;
	HandleTacticalUI();
}


BOOLEAN UIHandleOnMerc( BOOLEAN fMovementMode )
{
	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	SoldierFindFlags const uiMercFlags = guiUIFullTargetFlags;

	// CHECK IF WE'RE ON A GUY ( EITHER SELECTED, OURS, OR THEIRS
	SOLDIERTYPE* const pSoldier = gUIFullTarget;
	if (pSoldier != NULL)
	{
		if ( uiMercFlags & OWNED_MERC )
		{
			// If not unconscious, select
			if ( !( uiMercFlags & UNCONSCIOUS_MERC ) )
			{
				if ( fMovementMode )
				{
					// ERASE PATH
					ErasePath();

					// Show cursor with highlight on selected merc
					guiNewUICursor = NO_UICURSOR;

					// IF selected, do selection one
					if ( ( uiMercFlags & SELECTED_MERC ) )
					{
						// Add highlight to guy in interface.c
						gfUIHandleSelection = SELECTED_GUY_SELECTION;

						if ( gpItemPointer == NULL )
						{
							// Don't do this unless we want to

							// Check if buddy is stationary!
							if ( !gfIsUsingTouch && (gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_STATIONARY || pSoldier->fNoAPToFinishMove) )
							{
								guiShowUPDownArrows = ARROWS_SHOW_DOWN_BESIDE |
											ARROWS_SHOW_UP_BESIDE;
							}
						}

					}
					else
					{
						if ( !( uiMercFlags & NOINTERRUPT_MERC ) )
						{
							// Add highlight to guy in interface.c
							gfUIHandleSelection = NONSELECTED_GUY_SELECTION;
						}
						else
						{
							gfUIHandleSelection = ENEMY_GUY_SELECTION;
						}
					}
				}
			}

			// If not dead, show above guy!
			if ( !( uiMercFlags & DEAD_MERC ) )
			{
				if ( fMovementMode )
				{
					// ERASE PATH
					ErasePath();

					// Show cursor with highlight on selected merc
					guiNewUICursor = NO_UICURSOR;

					gsSelectedGridNo = pSoldier->sGridNo;
					gsSelectedLevel = pSoldier->bLevel;
				}

				gSelectedGuy = pSoldier;
			}
		}
		else if (((uiMercFlags & ENEMY_MERC) || (uiMercFlags & NEUTRAL_MERC)) &&
			(uiMercFlags & VISIBLE_MERC))
		{
			// ATE: If we are a vehicle, let the mouse cursor be a wheel...
			if ( ( OK_ENTERABLE_VEHICLE( pSoldier ) ) )
			{
				return( FALSE );
			}
			else
			{
				if ( fMovementMode )
				{

					// Check if this guy is on the enemy team....
					if ( !pSoldier->bNeutral && (pSoldier->bSide != OUR_TEAM ) )
					{
						gUIActionModeChangeDueToMouseOver = TRUE;

						guiPendingOverrideEvent = M_CHANGE_TO_ACTION;
						// Return FALSE
						return( FALSE );
					}
					else
					{
						// ERASE PATH
						ErasePath();

						// Show cursor with highlight on selected merc
						guiNewUICursor = NO_UICURSOR;
						// Show cursor with highlight
						gfUIHandleSelection = ENEMY_GUY_SELECTION;
						gsSelectedGridNo = pSoldier->sGridNo;
						gsSelectedLevel = pSoldier->bLevel;
					}
				}

				gSelectedGuy = pSoldier;
			}
		}
		else
		{
			if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				return( FALSE );
			}
		}
	}
	else
	{
		gfIgnoreOnSelectedGuy = FALSE;

		return( FALSE );
	}

	return( TRUE );
}


static ScreenID UIHandleISoldierDebug(UI_EVENT* pUIEvent)
{
	// Use soldier display pages
	SetDebugRenderHook(DebugSoldierPage1, 0);
	SetDebugRenderHook(DebugSoldierPage2, 1);
	SetDebugRenderHook(DebugSoldierPage3, 2);
	SetDebugRenderHook(DebugSoldierPage4, 3);
	gCurDebugPage = 1;

	return( DEBUG_SCREEN );
}


static ScreenID UIHandleILOSDebug(UI_EVENT* pUIEvent)
{
	SetDebugRenderHook(DebugStructurePage1, 0);
	return( DEBUG_SCREEN );
}


static ScreenID UIHandleILevelNodeDebug(UI_EVENT* pUIEvent)
{
	SetDebugRenderHook(DebugLevelNodePage, 0);
	return( DEBUG_SCREEN );
}


static ScreenID UIHandleIETOnTerrain(UI_EVENT* pUIEvent)
{
	//guiNewUICursor = CANNOT_MOVE_UICURSOR;
	guiNewUICursor = NO_UICURSOR;

	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	return( GAME_SCREEN );
}


static void SetUIbasedOnStance(SOLDIERTYPE* pSoldier, INT8 bNewStance);
static BOOLEAN SoldierCanAffordNewStance(SOLDIERTYPE* pSoldier, UINT8 ubDesiredStance);


void UIHandleSoldierStanceChange(SOLDIERTYPE* s, INT8 bNewStance)
{
	// Is this a valid stance for our position?
	if (!IsValidStance(s, bNewStance))
	{
		if (s->bCollapsed && s->bBreath < OKBREATH)
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(gzLateLocalizedString[STR_LATE_04], s->name));
		}
		else
		{
			if (s->uiStatusFlags & SOLDIER_VEHICLE)
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ VEHICLES_NO_STANCE_CHANGE_STR ] );
			}
			else if (s->uiStatusFlags & SOLDIER_ROBOT)
			{
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ ROBOT_NO_STANCE_CHANGE_STR ] );
			}
			else
			{
				if (s->bCollapsed)
				{
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(pMessageStrings[MSG_CANT_CHANGE_STANCE], s->name));
				}
				else
				{
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(TacticalStr[CANNOT_STANCE_CHANGE_STR], s->name));
				}
			}
		}
		return;
	}

	// IF turn-based - adjust stance now!
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		s->fTurningFromPronePosition = FALSE;

		// Check if we have enough APS
		if (SoldierCanAffordNewStance(s, bNewStance))
		{
			ChangeSoldierStance(s, bNewStance);

			s->sFinalDestination = s->sGridNo;
			s->bGoodContPath     = FALSE;
		}
		else
			return;
	}

	// If realtime- change walking animation!
	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{

		// If we are stationary, do something else!
		if (gAnimControl[s->usAnimState].uiFlags & ANIM_STATIONARY)
		{
			// Change stance normally
			ChangeSoldierStance(s, bNewStance);
		}
		else
		{
			// Pick moving animation based on stance

			// LOCK VARIBLE FOR NO UPDATE INDEX...
			s->usUIMovementMode = GetMoveStateBasedOnStance(s, bNewStance);

			if (s->usUIMovementMode == CRAWLING && gAnimControl[s->usAnimState].ubEndHeight != ANIM_PRONE)
			{
				s->usDontUpdateNewGridNoOnMoveAnimChange = LOCKED_NO_NEWGRIDNO;
				s->bPathStored = FALSE;
			}
			else
			{
				s->usDontUpdateNewGridNoOnMoveAnimChange = 1;
			}

			ChangeSoldierState(s, s->usUIMovementMode, 0, FALSE);
		}
	}

	// Set UI value for soldier
	SetUIbasedOnStance(s, bNewStance);

	gfUIStanceDifferent = TRUE;

	// ATE: If we are being serviced...stop...
	// InternalReceivingSoldierCancelServices(s, FALSE);
	InternalGivingSoldierCancelServices(s, FALSE);
	//gfPlotNewMovement   = TRUE;
}

MouseMoveState GetCursorMovementFlags()
{
	static BOOLEAN fStationary = FALSE;
	static UINT16  usOldMouseXPos = 32000;
	static UINT16  usOldMouseYPos = 32000;
	static UINT16  usOldMapPos = 32000;

	static MouseMoveState uiSameFrameCursorFlags = MOUSE_STATIONARY;
	static UINT32 uiOldFrameNumber = 99999;

	// Check if this is the same frame as before, return already calculated value if so!
	if ( uiOldFrameNumber == guiGameCycleCounter )
	{
		return uiSameFrameCursorFlags;
	}

	const GridNo usMapPos = guiCurrentCursorGridNo;

	MouseMoveState cursor_flags;
	if (gusMouseXPos == usOldMouseXPos && gusMouseYPos == usOldMouseYPos)
	{
		fStationary  = TRUE;
		cursor_flags = MOUSE_STATIONARY;
	}
	else if (fStationary && usOldMapPos == usMapPos) // If cursor was previously stationary, make the additional check of grid pos change
	{
		cursor_flags = MOUSE_MOVING_IN_TILE;
	}
	else
	{
		fStationary  = FALSE;
		cursor_flags = MOUSE_MOVING_NEW_TILE;
	}

	usOldMapPos = usMapPos;
	usOldMouseXPos = gusMouseXPos;
	usOldMouseYPos = gusMouseYPos;

	uiOldFrameNumber = guiGameCycleCounter;
	uiSameFrameCursorFlags = cursor_flags;
	return cursor_flags;
}


static INT8 DrawUIMovementPath(SOLDIERTYPE* pSoldier, UINT16 usMapPos, MoveUITarget);


BOOLEAN HandleUIMovementCursor(SOLDIERTYPE* const pSoldier, MouseMoveState const uiCursorFlags, UINT16 const usMapPos, MoveUITarget const uiFlags)
{
	static const SOLDIERTYPE* target = NULL;

	BOOLEAN fSetCursor = FALSE;
	BOOLEAN fTargetFoundAndLookingForOne = FALSE;

	// Determine if we can afford!
	if ( !EnoughPoints( pSoldier, gsCurrentActionPoints, 0, FALSE ) )
	{
		gfUIDisplayActionPointsInvalid = TRUE;
	}

	// Check if we're stationary
	if (!(gTacticalStatus.uiFlags & INCOMBAT) ||
		(gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_STATIONARY) ||
		pSoldier->fNoAPToFinishMove)
	{
		// If we are targeting a merc for some reason, don't go thorugh normal channels if we
		// are on someone now
		if ( uiFlags == MOVEUI_TARGET_MERCS || uiFlags == MOVEUI_TARGET_MERCSFORAID )
		{
			if (target != gUIFullTarget || gfResetUIMovementOptimization)
			{
				gfResetUIMovementOptimization = FALSE;

				// ERASE PATH
				ErasePath();

				// Try and get a path right away
				DrawUIMovementPath( pSoldier, usMapPos,  uiFlags );
			}

			// Save for next time...
			target = gUIFullTarget;

			if (gUIFullTarget != NULL) fTargetFoundAndLookingForOne = TRUE;
		}

		if ( uiFlags == MOVEUI_TARGET_ITEMS )
		{
			gfUIOverItemPoolGridNo = usMapPos;
		}
		else if ( uiFlags == MOVEUI_TARGET_MERCSFORAID )
		{
			// Set values for AP display...
			gfUIDisplayActionPointsCenter = TRUE;
		}

		// IF CURSOR IS MOVING
		if (uiCursorFlags != MOUSE_STATIONARY || gfUINewStateForIntTile)
		{
			// SHOW CURSOR
			fSetCursor = TRUE;

			// IF CURSOR WAS PREVIOUSLY STATIONARY, MAKE THE ADDITIONAL CHECK OF GRID POS CHANGE
			if ((uiCursorFlags == MOUSE_MOVING_NEW_TILE && !fTargetFoundAndLookingForOne) || gfUINewStateForIntTile)
			{
				// ERASE PATH
				ErasePath();

				// Reset counter
				RESETCOUNTER( PATHFINDCOUNTER );

				gfPlotNewMovement = TRUE;

			}

			if (uiCursorFlags == MOUSE_MOVING_IN_TILE)
			{
				gfUIDisplayActionPoints = TRUE;
			}

		}

		if (uiCursorFlags == MOUSE_STATIONARY)
		{
			// CURSOR IS STATIONARY
			if ( _KeyDown( SHIFT ) && !gfPlotNewMovementNOCOST )
			{
				gfPlotNewMovementNOCOST = TRUE;
				gfPlotNewMovement = TRUE;
			}
			if ( !(_KeyDown( SHIFT ) ) && gfPlotNewMovementNOCOST )
			{
				gfPlotNewMovementNOCOST = FALSE;
				gfPlotNewMovement = TRUE;
			}


			// ONLY DIPSLAY PATH AFTER A DELAY
			if ( COUNTERDONE( PATHFINDCOUNTER ) )
			{
				// Reset counter
				RESETCOUNTER( PATHFINDCOUNTER );

				if ( gfPlotNewMovement  )
				{
					DrawUIMovementPath( pSoldier, usMapPos,  uiFlags );

					gfPlotNewMovement = FALSE;
				}
			}

			fSetCursor = TRUE;

			// DISPLAY POINTS EVEN WITHOUT DELAY
			// ONLY IF GFPLOT NEW MOVEMENT IS FALSE!
			if ( !gfPlotNewMovement )
			{
				if (gsCurrentActionPoints < 0 || !(gTacticalStatus.uiFlags & INCOMBAT))
				{
					gfUIDisplayActionPoints = FALSE;
				}
				else
				{
					gfUIDisplayActionPoints = TRUE;

					if ( uiFlags == MOVEUI_TARGET_INTTILES )
					{
						// Set values for AP display...
						gUIDisplayActionPointsOffX = 22;
						gUIDisplayActionPointsOffY = 15;
					}
					if ( uiFlags == MOVEUI_TARGET_BOMB )
					{
						// Set values for AP display...
						gUIDisplayActionPointsOffX = 22;
						gUIDisplayActionPointsOffY = 15;
					}
					else if ( uiFlags == MOVEUI_TARGET_ITEMS )
					{
						// Set values for AP display...
						gUIDisplayActionPointsOffX = 22;
						gUIDisplayActionPointsOffY = 15;
					}
					else
					{
						switch ( pSoldier->usUIMovementMode )
						{
							case WALKING:

								gUIDisplayActionPointsOffY = 10;
								gUIDisplayActionPointsOffX = 10;
								break;

							case RUNNING:
								gUIDisplayActionPointsOffY = 15;
								gUIDisplayActionPointsOffX = 21;
								break;
						}
					}
				}
			}
		}

	}
	else
	{
		// THE MERC IS MOVING
		// We're moving, erase path, change cursor
		ErasePath();

		fSetCursor = TRUE;

	}

	return( fSetCursor );
}


static INT8 DrawUIMovementPath(SOLDIERTYPE* const pSoldier, UINT16 usMapPos, MoveUITarget const uiFlags)
{
	INT16 sAPCost;
	INT16 sActionGridNo;
	STRUCTURE *pStructure;
	INT16 sAdjustedGridNo;
	INT16 sIntTileGridNo;
	LEVELNODE *pIntTile;
	INT8 bReturnCode = 0;
	BOOLEAN fPlot;

	if ((gTacticalStatus.uiFlags & INCOMBAT) || _KeyDown( SHIFT ))
	{
		fPlot = PLOT;
	}
	else
	{
		fPlot = NO_PLOT;
	}

	sActionGridNo = usMapPos;
	sAPCost = 0;

	ErasePath();

	// IF WE ARE OVER AN INTERACTIVE TILE, GIVE GRIDNO OF POSITION
	if ( uiFlags == MOVEUI_TARGET_INTTILES )
	{
		// Get structure info for in tile!
		pIntTile = GetCurInteractiveTileGridNoAndStructure( &sIntTileGridNo, &pStructure );

		// We should not have null here if we are given this flag...
		if ( pIntTile != NULL )
		{
			sActionGridNo = FindAdjacentGridEx(pSoldier, sIntTileGridNo, NULL, NULL, FALSE, TRUE);
			if ( sActionGridNo == -1 )
			{
				sActionGridNo = sIntTileGridNo;
			}
			sAPCost  = AP_OPEN_DOOR;
			sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

			if ( sActionGridNo != pSoldier->sGridNo )
			{
				gfUIHandleShowMoveGrid = TRUE;
				gsUIHandleShowMoveGridLocation = sActionGridNo;
			}

			// Add cost for stance change....
			sAPCost += GetAPsToChangeStance( pSoldier, ANIM_STAND );
		}
		else
		{
			sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_WIREFENCE )
	{
		sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, NULL, NULL, FALSE, TRUE);
		if ( sActionGridNo == -1 )
		{
			sAPCost = 0;
		}
		else
		{
			sAPCost = GetAPsToCutFence( pSoldier );

			sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

			if ( sActionGridNo != pSoldier->sGridNo )
			{
				gfUIHandleShowMoveGrid = TRUE;
				gsUIHandleShowMoveGridLocation = sActionGridNo;
			}
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_JAR )
	{
		sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, NULL, NULL, FALSE, TRUE);
		if ( sActionGridNo == -1 )
		{
			sActionGridNo = usMapPos;
		}

		sAPCost = GetAPsToUseJar( pSoldier, sActionGridNo );

		sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

		if ( sActionGridNo != pSoldier->sGridNo )
		{
			gfUIHandleShowMoveGrid = TRUE;
			gsUIHandleShowMoveGridLocation = sActionGridNo;
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_CAN )
	{
		// Get structure info for in tile!
		pIntTile = GetCurInteractiveTileGridNoAndStructure( &sIntTileGridNo, &pStructure );

		// We should not have null here if we are given this flag...
		if ( pIntTile != NULL )
		{
			sActionGridNo = FindAdjacentGridEx(pSoldier, sIntTileGridNo, NULL, NULL, FALSE, TRUE);
			if ( sActionGridNo != -1 )
			{
				sAPCost = AP_ATTACH_CAN;
				sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

				if ( sActionGridNo != pSoldier->sGridNo )
				{
					gfUIHandleShowMoveGrid = TRUE;
					gsUIHandleShowMoveGridLocation = sActionGridNo;
				}
			}
		}
		else
		{
			sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
		}

	}
	else if ( uiFlags == MOVEUI_TARGET_REPAIR )
	{
		// For repair, check if we are over a vehicle, then get gridnot to edge of that vehicle!
		SOLDIERTYPE* tgt;
		if (IsRepairableStructAtGridNo(usMapPos, &tgt) == 2)
		{
			const INT16 sNewGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(pSoldier, pSoldier->usUIMovementMode, 5, 0, tgt);
			if ( sNewGridNo != NOWHERE )
			{
				usMapPos = sNewGridNo;
			}
		}

		sActionGridNo =  FindAdjacentGridEx(pSoldier, usMapPos, NULL, NULL, FALSE, TRUE);
		if ( sActionGridNo == -1 )
		{
			sActionGridNo = usMapPos;
		}

		sAPCost = GetAPsToBeginRepair( pSoldier );

		sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

		if ( sActionGridNo != pSoldier->sGridNo )
		{
			gfUIHandleShowMoveGrid = TRUE;
			gsUIHandleShowMoveGridLocation = sActionGridNo;
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_REFUEL )
	{
		// For refueling, check if we are over a vehicle, then get gridno to edge of that vehicle!
		const SOLDIERTYPE* const tgt = GetRefuelableStructAtGridNo(usMapPos);
		if (tgt != NULL)
		{
			const INT16 sNewGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(pSoldier, pSoldier->usUIMovementMode, 5, 0, tgt);
			if ( sNewGridNo != NOWHERE )
			{
				usMapPos = sNewGridNo;
			}
		}

		sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, NULL, NULL, FALSE, TRUE);
		if ( sActionGridNo == -1 )
		{
			sActionGridNo = usMapPos;
		}

		sAPCost = GetAPsToRefuelVehicle( pSoldier );

		sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

		if ( sActionGridNo != pSoldier->sGridNo )
		{
			gfUIHandleShowMoveGrid = TRUE;
			gsUIHandleShowMoveGridLocation = sActionGridNo;
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_MERCS )
	{
		INT16   sGotLocation = NOWHERE;
		BOOLEAN fGotAdjacent = FALSE;

		// Check if we are on a target
		const SOLDIERTYPE* const tgt = gUIFullTarget;
		if (tgt != NULL)
		{
			UINT8 cnt;
			INT16 sSpot;

			for ( cnt = 0; cnt < NUM_WORLD_DIRECTIONS; cnt++ )
			{
				sSpot = NewGridNo( pSoldier->sGridNo, DirectionInc( cnt ) );

				// Make sure movement costs are OK....
				if ( gubWorldMovementCosts[ sSpot ][ cnt ][ gsInterfaceLevel ] >= TRAVELCOST_BLOCKED )
				{
					continue;
				}

				// Check for who is there...
				if (WhoIsThere2(sSpot, pSoldier->bLevel) == tgt)
				{
					// We've got a guy here....
					// Who is the one we want......
					sGotLocation = sSpot;
					sAdjustedGridNo	= tgt->sGridNo;
					break;
				}
			}

				if ( sGotLocation == NOWHERE )
				{
				sActionGridNo = FindAdjacentGridEx(pSoldier, tgt->sGridNo, NULL, &sAdjustedGridNo, TRUE, FALSE);

					if ( sActionGridNo == -1 )
					{
						sGotLocation = NOWHERE;
					}
					else
					{
						sGotLocation = sActionGridNo;
					}
					fGotAdjacent = TRUE;
				}
		}
		else
		{
			sAdjustedGridNo = usMapPos;
			sGotLocation = sActionGridNo;
			fGotAdjacent = TRUE;
		}

		if ( sGotLocation != NOWHERE )
		{
			sAPCost += MinAPsToAttack( pSoldier, sAdjustedGridNo, TRUE );
			sAPCost += UIPlotPath(pSoldier, sGotLocation, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

			if ( sGotLocation != pSoldier->sGridNo && fGotAdjacent )
			{
				gfUIHandleShowMoveGrid = TRUE;
				gsUIHandleShowMoveGridLocation = sGotLocation;
			}
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_STEAL )
	{
		// Check if we are on a target
		const SOLDIERTYPE* const tgt = gUIFullTarget;
		if (tgt != NULL)
		{
			sActionGridNo = FindAdjacentGridEx(pSoldier, tgt->sGridNo, NULL, &sAdjustedGridNo, TRUE, FALSE);
			if ( sActionGridNo == -1 )
			{
				sActionGridNo = sAdjustedGridNo;
			}
			sAPCost += AP_STEAL_ITEM;
			// CJC August 13 2002: take into account stance in AP prediction
			if (!(PTR_STANDING))
			{
				sAPCost += GetAPsToChangeStance( pSoldier, ANIM_STAND );
			}
			sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

			if ( sActionGridNo != pSoldier->sGridNo )
			{
				gfUIHandleShowMoveGrid = TRUE;
				gsUIHandleShowMoveGridLocation = sActionGridNo;
			}
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_BOMB )
	{
		sAPCost += GetAPsToDropBomb( pSoldier );
		sAPCost += UIPlotPath(pSoldier, usMapPos, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);

		gfUIHandleShowMoveGrid = TRUE;
		gsUIHandleShowMoveGridLocation = usMapPos;
	}
	else if ( uiFlags == MOVEUI_TARGET_MERCSFORAID )
	{
		const SOLDIERTYPE* const tgt = gUIFullTarget;
		if (tgt != NULL)
		{
			sActionGridNo = FindAdjacentGridEx(pSoldier, tgt->sGridNo, NULL, &sAdjustedGridNo, TRUE, FALSE);

			// Try again at another gridno...
			if ( sActionGridNo == -1 )
			{
				sActionGridNo = FindAdjacentGridEx(pSoldier, usMapPos, NULL, &sAdjustedGridNo, TRUE, FALSE);

				if ( sActionGridNo == -1 )
				{
					sActionGridNo = sAdjustedGridNo;
				}
			}
			sAPCost += GetAPsToBeginFirstAid( pSoldier );
			sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
			if ( sActionGridNo != pSoldier->sGridNo )
			{
				gfUIHandleShowMoveGrid = TRUE;
				gsUIHandleShowMoveGridLocation = sActionGridNo;
			}
		}
	}
	else if ( uiFlags == MOVEUI_TARGET_ITEMS )
	{
		sActionGridNo = AdjustGridNoForItemPlacement( pSoldier, sActionGridNo );

		if ( pSoldier->sGridNo != sActionGridNo )
		{
			sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
			if ( sAPCost != 0 )
			{
				sAPCost += AP_PICKUP_ITEM;
			}
		}
		else
		{
				sAPCost += AP_PICKUP_ITEM;
		}

		if ( sActionGridNo != pSoldier->sGridNo )
		{
			gfUIHandleShowMoveGrid = TRUE;
			gsUIHandleShowMoveGridLocation = sActionGridNo;
		}
	}
	else
	{
		sAPCost += UIPlotPath(pSoldier, sActionGridNo, NO_COPYROUTE, fPlot, pSoldier->usUIMovementMode, pSoldier->bActionPoints);
	}

	if ( gTacticalStatus.uiFlags & SHOW_AP_LEFT )
	{
		gsCurrentActionPoints = pSoldier->bActionPoints - sAPCost;
	}
	else
	{
		gsCurrentActionPoints = sAPCost;
	}

	return( bReturnCode );
}


bool UIMouseOnValidAttackLocation(SOLDIERTYPE* const s)
{
	GridNo map_pos = guiCurrentCursorGridNo;
	if (map_pos == NOWHERE) return false;

	OBJECTTYPE const& o           = s->inv[HANDPOS];
	const ItemModel * item = GCM->getItem(o.usItem);
	ItemCursor const  item_cursor = GetActionModeCursor(s);

	if (item_cursor == INVALIDCURS) return false;

	if (item_cursor == WIRECUTCURS)
	{
		return s->bLevel == 0 && IsCuttableWireFenceAtGridNo(map_pos);
	}

	SOLDIERTYPE const* const tgt = gUIFullTarget;
	if (item_cursor == REPAIRCURS)
	{
		if (tgt) map_pos = tgt->sGridNo;
		return s->bLevel == 0 && IsRepairableStructAtGridNo(map_pos, 0);
	}

	if (item_cursor == REFUELCURS)
	{
		if (tgt) map_pos = tgt->sGridNo;
		return s->bLevel == 0 && GetRefuelableStructAtGridNo(map_pos);
	}

	if (item_cursor == BOMBCURS)
	{
		if (map_pos == s->sGridNo) return true;
		if (!NewOKDestination(s, map_pos, TRUE, s->bLevel)) return false;
	}

	if (tgt == s && item->getItemClass() != IC_MEDKIT) return false;

	if (HasObjectImprint(o) && s->ubProfile != o.ubImprintID)
	{
		// Access denied
		PlayJA2Sample(RG_ID_INVALID, HIGHVOLUME, 1, MIDDLE);
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, ST::format("\"{}\"", TacticalStr[GUN_NOGOOD_FINGERPRINT]));
		return false;
	}

	if (item->getItemClass() == IC_PUNCH)
	{
		// can't punch the ground
		if (!tgt) return false;

		// We test again whether the target is reachable
		if (CalcTotalAPsToAttack(s, tgt->sGridNo, true, 0) == 0)
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
			return false;
		}
		return tgt;
	}

	if (item->getItemClass() == IC_MEDKIT)
	{
		// If a guy's here, check if he needs medical help!
		if (!tgt) return false;

		if (tgt->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_ROBOT))
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(TacticalStr[CANNOT_DO_FIRST_AID_STR], tgt->name));
			return false;
		}

		if (s->bMedical == 0)
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(pMessageStrings[MSG_MERC_HAS_NO_MEDSKILL], s->name));
			return false;
		}

		if (tgt->bBleeding == 0 && tgt->bLife != tgt->bLifeMax)
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(gzLateLocalizedString[STR_LATE_19], tgt->name));
			return false;
		}

		if (tgt->bBleeding == 0 && tgt->bLife >= OKLIFE)
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(TacticalStr[CANNOT_NO_NEED_FIRST_AID_STR], tgt->name));
			return false;
		}
	}

	return true;
}


BOOLEAN UIOkForItemPickup( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	INT16 sAPCost;

	sAPCost = GetAPsToPickupItem( pSoldier, sGridNo );

	if ( sAPCost == 0 )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
	}
	else
	{
		if ( EnoughPoints( pSoldier, sAPCost, 0, TRUE ) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


static BOOLEAN SoldierCanAffordNewStance(SOLDIERTYPE* pSoldier, UINT8 ubDesiredStance)
{
	INT8  bCurrentHeight;
	UINT8 bAP = 0, bBP = 0;

	bCurrentHeight = ( ubDesiredStance - gAnimControl[ pSoldier->usAnimState ].ubEndHeight );

	// Now change to appropriate animation

	switch( bCurrentHeight )
	{
		case ANIM_STAND - ANIM_CROUCH:
		case ANIM_CROUCH - ANIM_STAND:

			bAP = AP_CROUCH;
			bBP = BP_CROUCH;
			break;

		case ANIM_STAND - ANIM_PRONE:
		case ANIM_PRONE - ANIM_STAND:

			bAP = AP_CROUCH + AP_PRONE;
			bBP = BP_CROUCH + BP_PRONE;
			break;

		case ANIM_CROUCH - ANIM_PRONE:
		case ANIM_PRONE - ANIM_CROUCH:

			bAP = AP_PRONE;
			bBP = BP_PRONE;
			break;

	}

	return ( EnoughPoints( pSoldier, bAP, bBP , TRUE ) );
}


static void SetUIbasedOnStance(SOLDIERTYPE* pSoldier, INT8 bNewStance)
{
	// Set UI based on our stance!
	switch ( bNewStance )
	{
		case ANIM_STAND:
			pSoldier->usUIMovementMode = WALKING;
			break;

		case ANIM_CROUCH:
			pSoldier->usUIMovementMode = SWATTING;
			break;

		case ANIM_PRONE:
			pSoldier->usUIMovementMode = CRAWLING;
			break;
	}

	// Set UI cursor!
}


static void SetMovementModeCursor(const SOLDIERTYPE* pSoldier)
{
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		if ( ( OK_ENTERABLE_VEHICLE( pSoldier ) ) )
		{
			guiNewUICursor = MOVE_VEHICLE_UICURSOR;
		}
		else
		{
			// Change mouse cursor based on type of movement we want to do
			switch ( pSoldier->usUIMovementMode )
			{
					case WALKING:
						guiNewUICursor = MOVE_WALK_UICURSOR;
						break;

					case RUNNING:
						guiNewUICursor = MOVE_RUN_UICURSOR;
						break;

					case SWATTING:
						guiNewUICursor = MOVE_SWAT_UICURSOR;
						break;

					case CRAWLING:
						guiNewUICursor = MOVE_PRONE_UICURSOR;
						break;
			}
		}
	}

	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		if ( gfUIAllMoveOn )
		{
			guiNewUICursor = ALL_MOVE_REALTIME_UICURSOR;
		}
		else
		{
			//if ( pSoldier->fUIMovementFast )
			//{
			//	BeginDisplayTimedCursor( MOVE_RUN_REALTIME_UICURSOR, 300 );
			//}

			guiNewUICursor = MOVE_REALTIME_UICURSOR;
		}
	}

	guiNewUICursor = GetInteractiveTileCursor( guiNewUICursor, FALSE );
}


static void SetConfirmMovementModeCursor(SOLDIERTYPE* pSoldier, BOOLEAN fFromMove)
{
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		if ( gfUIAllMoveOn )
		{
			if ( ( OK_ENTERABLE_VEHICLE( pSoldier ) ) )
			{
				guiNewUICursor = ALL_MOVE_VEHICLE_UICURSOR;
			}
			else
			{
				// Change mouse cursor based on type of movement we want to do
				switch ( pSoldier->usUIMovementMode )
				{
					case WALKING:
						guiNewUICursor = ALL_MOVE_WALK_UICURSOR;
						break;

					case RUNNING:
						guiNewUICursor = ALL_MOVE_RUN_UICURSOR;
						break;

					case SWATTING:
						guiNewUICursor = ALL_MOVE_SWAT_UICURSOR;
						break;

					case CRAWLING:
						guiNewUICursor = ALL_MOVE_PRONE_UICURSOR;
						break;
				}
			}
		}
		else
		{
			if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				guiNewUICursor = CONFIRM_MOVE_VEHICLE_UICURSOR;
			}
			else
			{
				// Change mouse cursor based on type of movement we want to do
				switch ( pSoldier->usUIMovementMode )
				{
					case WALKING:
						guiNewUICursor = CONFIRM_MOVE_WALK_UICURSOR;
						break;

					case RUNNING:
						guiNewUICursor = CONFIRM_MOVE_RUN_UICURSOR;
						break;

					case SWATTING:
						guiNewUICursor = CONFIRM_MOVE_SWAT_UICURSOR;
						break;

					case CRAWLING:
						guiNewUICursor = CONFIRM_MOVE_PRONE_UICURSOR;
						break;
				}
			}
		}
	}

	if (!(gTacticalStatus.uiFlags & INCOMBAT))
	{
		if ( gfUIAllMoveOn )
		{
			if ( gfUIAllMoveOn == 2 )
			{
				BeginDisplayTimedCursor( MOVE_RUN_REALTIME_UICURSOR, 300 );
			}
			else
			{
				guiNewUICursor = ALL_MOVE_REALTIME_UICURSOR;
			}
		}
		else
		{
			if ( pSoldier->fUIMovementFast && pSoldier->usAnimState == RUNNING && fFromMove )
			{
				BeginDisplayTimedCursor( MOVE_RUN_REALTIME_UICURSOR, 300 );
			}

			guiNewUICursor = CONFIRM_MOVE_REALTIME_UICURSOR;
		}
	}

	guiNewUICursor = GetInteractiveTileCursor( guiNewUICursor, TRUE );

}


static ScreenID UIHandleLCOnTerrain(UI_EVENT* pUIEvent)
{
	guiNewUICursor = LOOK_UICURSOR;

	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;

	ShowTacticalTouchUI(TacticalTouchUIMode::ConfirmAction);

	gfUIDisplayActionPoints = TRUE;

	gUIDisplayActionPointsOffX = 14;
	gUIDisplayActionPointsOffY = 7;

	const GridNo pos = guiCurrentCursorGridNo;

	// Get direction from mouse pos
	const INT16 sFacingDir = GetDirectionFromGridNo(pos, sel);

	// Set # of APs
	gsCurrentActionPoints = (sFacingDir == sel->bDirection ? 0 : GetAPsToLook(sel));

	// Determine if we can afford!
	if (!EnoughPoints(sel, gsCurrentActionPoints, 0, FALSE))
	{
		gfUIDisplayActionPointsInvalid = TRUE;
	}

	return( GAME_SCREEN );

}


static ScreenID UIHandleLCChangeToLook(UI_EVENT* pUIEvent)
{
	ErasePath();

	return( GAME_SCREEN );
}


static BOOLEAN MakeSoldierTurn(SOLDIERTYPE* const pSoldier, const GridNo pos)
{
	// Get direction from mouse pos
	const INT16 sFacingDir = GetDirectionFromGridNo(pos, pSoldier);

	if ( sFacingDir != pSoldier->bDirection )
	{
		const INT16 sAPCost = GetAPsToLook(pSoldier);

		// Check AP cost...
		if ( !EnoughPoints( pSoldier, sAPCost, 0, TRUE ) )
		{
			return( FALSE );
		}

		// ATE: make stationary if...
		if ( pSoldier->fNoAPToFinishMove )
		{
			SoldierGotoStationaryStance( pSoldier );
		}

		//DEF:  made it an event
		SendSoldierSetDesiredDirectionEvent( pSoldier, sFacingDir );

		pSoldier->bTurningFromUI = TRUE;

		// ATE: Hard-code here previous event to ui busy event...
		guiOldEvent = LA_BEGINUIOURTURNLOCK;

		return( TRUE );
	}

	return( FALSE );
}


static ScreenID UIHandleLCLook(UI_EVENT* pUIEvent)
{
	const GridNo pos = guiCurrentCursorGridNo;
	if (pos == NOWHERE) return GAME_SCREEN;

	if ( gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
	{
		// OK, loop through all guys who are 'multi-selected' and
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			if (s->bInSector && s->uiStatusFlags & SOLDIER_MULTI_SELECTED)
			{
				MakeSoldierTurn(s, pos);
			}
		}
	}
	else
	{
		SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel == NULL) return GAME_SCREEN;

		if (MakeSoldierTurn(sel, pos)) SetUIBusy(sel);
	}
	return( GAME_SCREEN );
}


static ScreenID UIHandleTOnTerrain(UI_EVENT* pUIEvent)
{
	INT16 sTargetGridNo;

	ShowTacticalTouchUI(TacticalTouchUIMode::ConfirmMove);

	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return GAME_SCREEN;

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE) return GAME_SCREEN;

	if( ValidQuickExchangePosition( ) )
	{
		// Do new cursor!
		guiPendingOverrideEvent = M_ON_TERRAIN;
		return( UIHandleMOnTerrain( pUIEvent ) );
	}

	sTargetGridNo = usMapPos;


	UIHandleOnMerc( FALSE );


	//CHECK FOR VALID TALKABLE GUY HERE
	const SOLDIERTYPE* const tgt = GetValidTalkableNPCFromMouse(FALSE, TRUE, FALSE);

	// USe cursor based on distance
	// Get distance away
	if (tgt != NULL) sTargetGridNo = tgt->sGridNo;

	const UINT32 uiRange = GetRangeFromGridNoDiff(sel->sGridNo, sTargetGridNo);


	//ATE: Check if we have good LOS
	// is he close enough to see that gridno if he turns his head?
	const INT16 sDistVisible = DistanceVisible(sel, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sTargetGridNo, sel->bLevel);


	if ( uiRange <= NPC_TALK_RADIUS )
	{
		if (tgt != NULL)
		{
			guiNewUICursor = TALK_A_UICURSOR;
		}
		else
		{
			guiNewUICursor = TALK_NA_UICURSOR;
		}
	}
	else
	{
		if (tgt != NULL)
		{
			//guiNewUICursor = TALK_OUT_RANGE_A_UICURSOR;
			guiNewUICursor = TALK_A_UICURSOR;
		}
		else
		{
			guiNewUICursor = TALK_OUT_RANGE_NA_UICURSOR;
		}
	}

	if (tgt != NULL)
	{
		if (!SoldierTo3DLocationLineOfSightTest(sel, sTargetGridNo, sel->bLevel, 3, sDistVisible, TRUE))
		{
			//. ATE: Make range far, so we alternate cursors...
			guiNewUICursor = TALK_OUT_RANGE_A_UICURSOR;
		}
	}

	gfUIDisplayActionPoints = TRUE;

	gUIDisplayActionPointsOffX = 8;
	gUIDisplayActionPointsOffY = 3;

	// Set # of APs
	gsCurrentActionPoints = 6;

	// Determine if we can afford!
	if (!EnoughPoints(sel, gsCurrentActionPoints, 0, FALSE))
	{
		gfUIDisplayActionPointsInvalid = TRUE;
	}

	return( GAME_SCREEN );
}


static ScreenID UIHandleTChangeToTalking(UI_EVENT* pUIEvent)
{
	ErasePath();

	HideTacticalTouchUI();

	return( GAME_SCREEN );
}


static ScreenID UIHandleLUIOnTerrain(UI_EVENT* pUIEvent)
{
	//guiNewUICursor = NO_UICURSOR;
	//SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	return( GAME_SCREEN );
}


static ScreenID UIHandleLUIBeginLock(UI_EVENT* pUIEvent)
{
	// Don't let both versions of the locks to happen at the same time!
	// ( They are mutually exclusive )!
	UIHandleLAEndLockOurTurn( NULL );

	// Hide touch ui
	HideTacticalTouchUI();
	if (gfIsUsingTouch) ResetCurrentCursorTarget();

	if ( !gfDisableRegionActive )
	{
		gfDisableRegionActive = TRUE;

		RemoveTacticalCursor( );
		//SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

		MSYS_DefineRegion(&gDisableRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST,
					CURSOR_WAIT, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

		//guiPendingOverrideEvent = LOCKUI_MODE;

		// UnPause time!
		PauseGame();
		LockPauseState(LOCK_PAUSE_LOCKUI_MODE);
	}

	return( GAME_SCREEN );
}


ScreenID UIHandleLUIEndLock(UI_EVENT* pUIEvent)
{
	if ( gfDisableRegionActive )
	{
		gfDisableRegionActive = FALSE;

		// Add region
		MSYS_RemoveRegion( &gDisableRegion );
		RefreshMouseRegions( );

		//SetCurrentCursorFromDatabase( guiCurrentUICursor );

		UIHandleMOnTerrain( NULL );

		if ( !gfIsUsingTouch && gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
		{
			SetCurrentCursorFromDatabase( gUICursors[ guiNewUICursor ].usFreeCursorName );
		}

		guiPendingOverrideEvent = M_ON_TERRAIN;
		HandleTacticalUI( );

		// ATE: Only if NOT in conversation!
		if ( !( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) )
		{
			// UnPause time!
			UnLockPauseState();
			UnPauseGame();
		}
	}

	return( GAME_SCREEN );
}


void CheckForDisabledRegionRemove( )
{
	if ( gfDisableRegionActive )
	{
		gfDisableRegionActive = FALSE;

		// Remove region
		MSYS_RemoveRegion( &gDisableRegion );

		UnLockPauseState();
		UnPauseGame();

	}

	if ( gfUserTurnRegionActive )
	{
		gfUserTurnRegionActive = FALSE;

		gfUIInterfaceSetBusy = FALSE;

		// Remove region
		MSYS_RemoveRegion( &gUserTurnRegion );

		UnLockPauseState();
		UnPauseGame();
	}
}


static ScreenID UIHandleLAOnTerrain(UI_EVENT* pUIEvent)
{
	//guiNewUICursor = NO_UICURSOR;
	//SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	return( GAME_SCREEN );
}


static void GetGridNoScreenXY(INT16 sGridNo, INT16* pScreenX, INT16* pScreenY)
{
	INT16 sScreenX, sScreenY;
	INT16 sOffsetX, sOffsetY;
	INT16 sTempX_S, sTempY_S;
	INT16 sXPos, sYPos;

	ConvertGridNoToCellXY( sGridNo, &sXPos, &sYPos );

	// Get 'TRUE' merc position
	sOffsetX = sXPos - gsRenderCenterX;
	sOffsetY = sYPos - gsRenderCenterY;

	FromCellToScreenCoordinates( sOffsetX, sOffsetY, &sTempX_S, &sTempY_S );

	sScreenX = ( g_ui.m_tacticalMapCenterX ) + (INT16)sTempX_S;
	sScreenY = ( g_ui.m_tacticalMapCenterY ) + (INT16)sTempY_S;

	// Adjust for offset position on screen
	sScreenX -= gsRenderWorldOffsetX;
	sScreenY -= gsRenderWorldOffsetY;
	sScreenY -=	gpWorldLevelData[ sGridNo ].sHeight;

	// Adjust based on interface level

	// Adjust for render height
	sScreenY += gsRenderHeight;

	// Adjust y offset!
	sScreenY += ( WORLD_TILE_Y/2);

	(*pScreenX) = sScreenX;
	(*pScreenY) = sScreenY;
}


void EndMultiSoldierSelection(BOOLEAN acknowledge)
{
	gTacticalStatus.fAtLeastOneGuyOnMultiSelect = FALSE;

	if (gGameSettings.fOptions[TOPTION_MUTE_CONFIRMATIONS]) acknowledge = FALSE;

	// loop through all guys who are 'multi-selected' and check if our currently
	// selected guy is among them - if not, change to a guy who is
	SOLDIERTYPE*             first = NULL;
	const SOLDIERTYPE* const sel   = GetSelectedMan();
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (!s->bInSector)
			continue;
		if (!(s->uiStatusFlags & SOLDIER_MULTI_SELECTED))
			continue;

		gTacticalStatus.fAtLeastOneGuyOnMultiSelect = TRUE;

		if (s == sel || first == NULL)
			first = s;

		if (acknowledge)
			InternalDoMercBattleSound(s, BATTLE_SOUND_ATTN1, BATTLE_SND_LOWER_VOLUME);

		if (s->fMercAsleep)
			PutMercInAwakeState(s);
	}

	// If here, select the first guy...
	if (first != NULL && first != sel)
	{
		SelectSoldier(first, SELSOLDIER_ACKNOWLEDGE | SELSOLDIER_FORCE_RESELECT);
	}
}


void StopRubberBandedMercFromMoving( )
{
	if ( !gTacticalStatus.fAtLeastOneGuyOnMultiSelect )
	{
		return;
	}

	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bInSector && s->uiStatusFlags & SOLDIER_MULTI_SELECTED)
		{
			s->fDelayedMovement = FALSE;
			s->sFinalDestination = s->sGridNo;
			StopSoldier(s);
		}
	}
}


void EndRubberBanding(BOOLEAN fCancel)
{
	if (gRubberBandActive)
	{
		FreeMouseCursor( );
		gfIgnoreScrolling = FALSE;
		if (!fCancel)
		{
			EndMultiSoldierSelection(TRUE);
		}

		gRubberBandActive = FALSE;
	}
}


static BOOLEAN HandleMultiSelectionMove(INT16 sDestGridNo)
{
	BOOLEAN fAtLeastOneMultiSelect = FALSE;
	BOOLEAN fMoveFast = FALSE;

	// OK, loop through all guys who are 'multi-selected' and
	// Make them move....

	// Do a loop first to see if the selected guy is told to go fast...
	gfGetNewPathThroughPeople = TRUE;

	const SOLDIERTYPE* const sel = GetSelectedMan();
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bInSector &&
			s->uiStatusFlags & SOLDIER_MULTI_SELECTED &&
			s == sel)
		{
			fMoveFast = s->fUIMovementFast;
			break;
		}
	}

	FOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		SoldierSP soldier = GetSoldier(pSoldier);
		if (pSoldier->bInSector)
		{
			if ( pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED )
			{
				// If we can't be controlled, returninvalid...
				if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
				{
					if ( !CanRobotBeControlled( pSoldier ) )
					{
						continue;
					}
				}

				pSoldier->fUIMovementFast  = fMoveFast;
				pSoldier->usUIMovementMode =  GetMoveStateBasedOnStance( pSoldier, gAnimControl[ pSoldier->usAnimState ].ubEndHeight );

				pSoldier->fUIMovementFast  = FALSE;

				if ( gUIUseReverse )
				{
					pSoldier->bReverse = TRUE;
				}
				else
				{
					pSoldier->bReverse = FALSE;
				}

				soldier->removePendingAction();

				if ( EVENT_InternalGetNewSoldierPath( pSoldier, sDestGridNo, pSoldier->usUIMovementMode , TRUE, pSoldier->fNoAPToFinishMove ) )
				{
					InternalDoMercBattleSound( pSoldier, BATTLE_SOUND_OK1, BATTLE_SND_LOWER_VOLUME );
				}
				else
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(TacticalStr[ NO_PATH_FOR_MERC ], pSoldier->name) );
				}

				fAtLeastOneMultiSelect = TRUE;
			}
		}
	}
	gfGetNewPathThroughPeople = FALSE;

	return( fAtLeastOneMultiSelect );
}


void ResetMultiSelection( )
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bInSector && s->uiStatusFlags & SOLDIER_MULTI_SELECTED)
		{
			s->uiStatusFlags &= ~SOLDIER_MULTI_SELECTED;
		}
	}

	gTacticalStatus.fAtLeastOneGuyOnMultiSelect = FALSE;
}


static ScreenID UIHandleRubberBandOnTerrain(UI_EVENT* pUIEvent)
{
	INT16   sScreenX, sScreenY;
	INT32   iTemp;
	SGPRect aRect;
	BOOLEAN fAtLeastOne = FALSE;

	guiNewUICursor = NO_UICURSOR;
	//SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	gRubberBandRect.iRight	= gusMouseXPos;
	gRubberBandRect.iBottom	= gusMouseYPos;

	// Copy into temp rect
	aRect = gRubberBandRect;

	if ( aRect.iRight < aRect.iLeft )
	{
		iTemp = aRect.iLeft;
		aRect.iLeft = aRect.iRight;
		aRect.iRight = iTemp;
	}


	if ( aRect.iBottom < aRect.iTop )
	{
		iTemp = aRect.iTop;
		aRect.iTop = aRect.iBottom;
		aRect.iBottom = iTemp;
	}

	// ATE:Check at least for one guy that's in point!
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// Check if this guy is OK to control....
		if (OkControllableMerc(s) && !(s->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_PASSENGER | SOLDIER_DRIVER)))
		{
			// Get screen pos of gridno......
			GetGridNoScreenXY(s->sGridNo, &sScreenX, &sScreenY);

			// ATE: If we are in a hiehger interface level, subttrasct....
			if ( gsInterfaceLevel == 1 )
			{
				sScreenY -= 50;
			}

			if (IsPointInScreenRect(sScreenX, sScreenY, aRect))
			{
				fAtLeastOne = TRUE;
			}
		}
	}

	if ( !fAtLeastOne )
	{
		return( GAME_SCREEN );
	}

	// ATE: Now loop through our guys and see if any fit!
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// Check if this guy is OK to control....
		if (OkControllableMerc(s) && !(s->uiStatusFlags & (SOLDIER_VEHICLE | SOLDIER_PASSENGER | SOLDIER_DRIVER)))
		{
			if ( !_KeyDown( ALT ) )
			{
				s->uiStatusFlags &= ~SOLDIER_MULTI_SELECTED;
			}

			// Get screen pos of gridno......
			GetGridNoScreenXY(s->sGridNo, &sScreenX, &sScreenY);

			// ATE: If we are in a hiehger interface level, subttrasct....
			if ( gsInterfaceLevel == 1 )
			{
				sScreenY -= 50;
			}

			if (IsPointInScreenRect(sScreenX, sScreenY, aRect))
			{
				// Adjust this guy's flag...
				s->uiStatusFlags |= SOLDIER_MULTI_SELECTED;
			}
		}
	}


	return( GAME_SCREEN );
}


static ScreenID UIHandleJumpOverOnTerrain(UI_EVENT* pUIEvent)
{
	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL)
		return GAME_SCREEN;

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE)
		return GAME_SCREEN;

	if (!IsValidJumpLocation(sel, usMapPos, FALSE))
	{
		guiPendingOverrideEvent = M_ON_TERRAIN;
		return( GAME_SCREEN );
	}

	// Display APs....
	gsCurrentActionPoints = GetAPsToJumpOver(sel);

	gfUIDisplayActionPoints = TRUE;
	gfUIDisplayActionPointsCenter = TRUE;

	guiNewUICursor = JUMP_OVER_UICURSOR;

	return( GAME_SCREEN );
}


static ScreenID UIHandleJumpOver(UI_EVENT* pUIEvent)
{
	// Here, first get map screen
	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL)
		return GAME_SCREEN;

	SoldierSP selSoldier = GetSoldier(sel);

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE)
		return GAME_SCREEN;

	if (!IsValidJumpLocation(sel, usMapPos, FALSE))
		return GAME_SCREEN;

	SetUIBusy(sel);

	// OK, Start jumping!
	// Remove any previous actions
	selSoldier->removePendingAction();

	// Get direction to goto....
	const INT8 bDirection = GetDirectionFromGridNo(usMapPos, sel);

	sel->fDontChargeTurningAPs = TRUE;
	EVENT_SetSoldierDesiredDirection(sel, bDirection);
	sel->fTurningUntilDone = TRUE;
	// ATE: Reset flag to go back to prone...
	//sel->fTurningFromPronePosition = TURNING_FROM_PRONE_OFF;
	sel->usPendingAnimation = JUMP_OVER_BLOCKING_PERSON;

	return( GAME_SCREEN );
}


static ScreenID UIHandleLABeginLockOurTurn(UI_EVENT* pUIEvent)
{
	// Don't let both versions of the locks to happen at the same time!
	// ( They are mutually exclusive )!
	UIHandleLUIEndLock( NULL );

	if ( !gfUserTurnRegionActive )
	{
		gfUserTurnRegionActive = TRUE;

		gfUIInterfaceSetBusy = TRUE;
		guiUIInterfaceBusyTime = GetJA2Clock( );

		HideTacticalTouchUI();
		if (gfIsUsingTouch) ResetCurrentCursorTarget();

		MSYS_DefineRegion(&gUserTurnRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, CURSOR_WAIT, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

		//guiPendingOverrideEvent = LOCKOURTURN_UI_MODE;

		ErasePath();

		// Pause time!
		PauseGame();
		LockPauseState(LOCK_PAUSE_LOCKOURTURN_UI_MODE);
	}

	return( GAME_SCREEN );
}


static ScreenID UIHandleLAEndLockOurTurn(UI_EVENT* pUIEvent)
{
	if ( gfUserTurnRegionActive )
	{
		gfUserTurnRegionActive = FALSE;

		gfUIInterfaceSetBusy = FALSE;

		// Add region
		MSYS_RemoveRegion( &gUserTurnRegion );
		RefreshMouseRegions( );
		//SetCurrentCursorFromDatabase( guiCurrentUICursor );

		gfPlotNewMovement = TRUE;

		UIHandleMOnTerrain( NULL );

		if ( !gfIsUsingTouch && gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA )
		{
			SetCurrentCursorFromDatabase( gUICursors[ guiNewUICursor ].usFreeCursorName );
		}
		guiPendingOverrideEvent = M_ON_TERRAIN;
		HandleTacticalUI( );

		TurnOffTeamsMuzzleFlashes( OUR_TEAM );

		// UnPause time!
		UnLockPauseState();
		UnPauseGame();
	}

	return( GAME_SCREEN );
}

// Pan mode is for scrolling the screen by pointer movement
static ScreenID UIHandlePPanMode(UI_EVENT* pUIEvent) {
	static INT16 lastPointerXPos, lastPointerYPos;

	if (pUIEvent->fFirstTime) {
		lastPointerXPos = -1;
		lastPointerYPos = -1;
	}

	HideTacticalTouchUI();
	guiNewUICursor = NO_UICURSOR;

	if (lastPointerXPos != -1 && lastPointerYPos != -1) {
		auto diffX = gusMouseXPos - lastPointerXPos;
		auto diffY = gusMouseYPos - lastPointerYPos;

		gsScrollXOffset += diffX;
		gsScrollYOffset += diffY;
	}
	if (gusMouseXPos != lastPointerXPos || gusMouseYPos != lastPointerYPos) {
		lastPointerXPos = gusMouseXPos;
		lastPointerYPos = gusMouseYPos;
	}

	return GAME_SCREEN;
}


SOLDIERTYPE* GetValidTalkableNPCFromMouse(BOOLEAN fGive, BOOLEAN fAllowMercs, BOOLEAN fCheckCollapsed)
{
	// Check if there is a guy here to talk to!
	SOLDIERTYPE* const tgt = gUIFullTarget;
	if (tgt == NULL)
		return NULL;
	if (!IsValidTalkableNPC(tgt, fGive, fAllowMercs, fCheckCollapsed))
		return NULL;
	return tgt;
}


BOOLEAN IsValidTalkableNPC(const SOLDIERTYPE* pSoldier, BOOLEAN fGive, BOOLEAN fAllowMercs, BOOLEAN fCheckCollapsed)
{
	BOOLEAN fValidGuy = FALSE;

	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL && AM_A_ROBOT(sel)) return FALSE;

	// CHECK IF ACTIVE!
	if ( !pSoldier->bActive )
	{
		return( FALSE );
	}

	// CHECK IF DEAD
	if( pSoldier->bLife == 0 )
	{
		return( FALSE );
	}

	if ( pSoldier->bCollapsed && fCheckCollapsed )
	{
		return( FALSE );
	}

	if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		return( FALSE );
	}


	// IF BAD GUY - CHECK VISIVILITY
	if ( pSoldier->bTeam != OUR_TEAM )
	{
		if ( pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags&SHOW_ALL_MERCS) )
		{
			return( FALSE  );
		}
	}

	if (pSoldier->ubProfile != NO_PROFILE && MercProfile(pSoldier->ubProfile).isNPCorRPC() &&
		!RPC_RECRUITED(pSoldier) && !AM_AN_EPC(pSoldier))
	{
		fValidGuy = TRUE;
	}

	// Check for EPC...
	if (pSoldier->ubProfile != NO_PROFILE && (gCurrentUIMode == TALKCURSOR_MODE || fGive)  && AM_AN_EPC(pSoldier))
	{
		fValidGuy = TRUE;
	}

	// ATE: We can talk to our own teammates....
	if ( pSoldier->bTeam == OUR_TEAM && fAllowMercs  )
	{
		fValidGuy = TRUE;
	}

	if ( GetCivType( pSoldier ) != CIV_TYPE_NA && !fGive )
	{
		fValidGuy = TRUE;
	}

	// Alright, let's do something special here for robot...
	if ( pSoldier->uiStatusFlags & SOLDIER_ROBOT )
	{
		// Can't talk to robots!
		if (!fGive) fValidGuy = FALSE;
	}

	// OK, check if they are stationary or not....
	// Do some checks common to all..
	if ( fValidGuy )
	{
		if ((gAnimControl[pSoldier->usAnimState].uiFlags & ANIM_MOVING) &&
			!(gTacticalStatus.uiFlags & INCOMBAT))
		{
			return( FALSE );
		}

		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN HandleTalkInit(  )
{
	INT16 sAPCost;
	UINT8 ubNewDirection;
	UINT8 ubQuoteNum;
	UINT8 ubDiceRoll;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL)
		return FALSE;

	SoldierSP selSoldier = GetSoldier(sel);

	const GridNo usMapPos = guiCurrentCursorGridNo;
	if (usMapPos == NOWHERE)
		return FALSE;

	// Check if there is a guy here to talk to!
	const SOLDIERTYPE* const pTSoldier = gUIFullTarget;
	if (pTSoldier != NULL)
	{
		// Is he a valid NPC?
		if (IsValidTalkableNPC(pTSoldier, FALSE, TRUE, FALSE))
		{
			if (pTSoldier != sel)
			{
				//ATE: Check if we have good LOS
				// is he close enough to see that gridno if he turns his head?
				const INT16 sDistVisible = DistanceVisible(sel, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, pTSoldier->sGridNo, pTSoldier->bLevel);

				// Check LOS!
				if (!SoldierTo3DLocationLineOfSightTest(sel, pTSoldier->sGridNo,  pTSoldier->bLevel, 3, sDistVisible, TRUE))
				{
					if ( pTSoldier->ubProfile != NO_PROFILE )
					{
						ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(TacticalStr[NO_LOS_TO_TALK_TARGET], sel->name, pTSoldier->name));
					}
					else
					{
						ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(gzLateLocalizedString[STR_LATE_45], sel->name));
					}
					return( FALSE );
				}
			}

			if ( pTSoldier->bCollapsed )
			{
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK,
						st_format_printf(gzLateLocalizedString[STR_LATE_21], pTSoldier->name));
				return( FALSE );
			}

			// If Q on, turn off.....
			if ( guiCurrentScreen == DEBUG_SCREEN )
			{
				gfExitDebugScreen = TRUE;
			}

			// ATE: if our own guy...
			if ( pTSoldier->bTeam == OUR_TEAM && !AM_AN_EPC( pTSoldier ) )
			{
				if ( pTSoldier->ubProfile == DIMITRI )
				{
					ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(gzLateLocalizedString[STR_LATE_32], pTSoldier->name));
					return( FALSE );
				}

				// Randomize quote to use....

				// If buddy had a social trait...
				if ( gMercProfiles[ pTSoldier->ubProfile ].bAttitude != ATT_NORMAL )
				{
					ubDiceRoll = (UINT8)Random( 3 );
				}
				else
				{
					ubDiceRoll = (UINT8)Random( 2 );
				}

				// If we are a PC, only use 0
				if ( pTSoldier->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER )
				{
					ubDiceRoll = 0;
				}

				switch( ubDiceRoll )
				{
					case 0:

						ubQuoteNum = QUOTE_NEGATIVE_COMPANY;
						break;

					case 1:

						if ( QuoteExp_PassingDislike[ pTSoldier->ubProfile ] )
						{
							ubQuoteNum = QUOTE_PASSING_DISLIKE;
						}
						else
						{
							ubQuoteNum = QUOTE_NEGATIVE_COMPANY;
						}
						break;

					case 2:

						ubQuoteNum = QUOTE_SOCIAL_TRAIT;
						break;

					default:

						ubQuoteNum = QUOTE_NEGATIVE_COMPANY;
						break;
				}

				if ( pTSoldier->ubProfile == IRA )
				{
					ubQuoteNum = QUOTE_PASSING_DISLIKE;
				}

				TacticalCharacterDialogue( pTSoldier, ubQuoteNum );

				return( FALSE );
			}

			// Check distance
			const UINT32 uiRange = GetRangeFromGridNoDiff(sel->sGridNo, usMapPos);

			// Double check path
			if ( GetCivType( pTSoldier ) != CIV_TYPE_NA )
			{
				// ATE: If one is already active, just remove it!
				if ( ShutDownQuoteBoxIfActive( ) )
				{
					return( FALSE );
				}
			}

			if ( uiRange > NPC_TALK_RADIUS )
			{
				// First get an adjacent gridno....
				const INT16 sActionGridNo = FindAdjacentGridEx(sel, pTSoldier->sGridNo, NULL, NULL, FALSE, TRUE);

				if ( sActionGridNo == -1 )
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
					return( FALSE );
				}

				if (UIPlotPath(sel, sActionGridNo, NO_COPYROUTE, FALSE, sel->usUIMovementMode, sel->bActionPoints) == 0)
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, TacticalStr[ NO_PATH ] );
					return( FALSE );
				}

				// Walk up and talk to buddy....
				gfNPCCircularDistLimit = TRUE;
				const INT16 sGoodGridNo = FindGridNoFromSweetSpotWithStructData(sel, sel->usUIMovementMode, pTSoldier->sGridNo, NPC_TALK_RADIUS - 1, &ubNewDirection, TRUE);
				gfNPCCircularDistLimit = FALSE;

				// First calculate APs and validate...
				sAPCost = AP_TALK;

				// Check AP cost...
				if (!EnoughPoints(sel, sAPCost, 0, TRUE))
				{
					return( FALSE );
				}

				// Now walkup to talk....
				selSoldier->setPendingAction(MERC_TALK);
				sel->uiPendingActionData1 = pTSoldier->ubID;

				// WALK UP TO DEST FIRST
				EVENT_InternalGetNewSoldierPath(sel, sGoodGridNo, sel->usUIMovementMode, TRUE, sel->fNoAPToFinishMove);

				return( FALSE );
			}
			else
			{
				sAPCost = AP_TALK;

				// Check AP cost...
				if (!EnoughPoints(sel, sAPCost, 0, TRUE))
				{
					return( FALSE );
				}

				// OK, startup!
				PlayerSoldierStartTalking(sel, pTSoldier->ubID, FALSE);
			}

			if ( GetCivType( pTSoldier ) != CIV_TYPE_NA )
			{
				return( FALSE );
			}
			else
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


void SetUIBusy(const SOLDIERTYPE* const s)
{
	if ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.ubCurrentTeam == OUR_TEAM))
	{
		if (s == GetSelectedMan())
		{
			guiPendingOverrideEvent	= LA_BEGINUIOURTURNLOCK;
			HandleTacticalUI( );
		}
	}
}


void UnSetUIBusy(const SOLDIERTYPE* const s)
{
	if ((gTacticalStatus.uiFlags & INCOMBAT) && (gTacticalStatus.ubCurrentTeam == OUR_TEAM ))
	{
		if ( !gTacticalStatus.fUnLockUIAfterHiddenInterrupt )
		{
			if (s == GetSelectedMan())
			{
				guiPendingOverrideEvent	= LA_ENDUIOUTURNLOCK;
				HandleTacticalUI( );

				// Set grace period...
				gTacticalStatus.uiTactialTurnLimitClock = GetJA2Clock( );
			}
		}
		// player getting control back so reset all muzzle flashes
	}
}


void BeginDisplayTimedCursor(UICursorID const uiCursorID, UINT32 const uiDelay)
{
	gfDisplayTimerCursor = TRUE;
	guiTimerCursorID = uiCursorID;
	guiTimerLastUpdate = GetJA2Clock( );
	guiTimerCursorDelay = uiDelay;
}


static bool UIHandleInteractiveTilesAndItemsOnTerrain(SOLDIERTYPE* const pSoldier, INT16 const usMapPos, BOOLEAN const fUseOKCursor, BOOLEAN const fItemsOnlyIfOnIntTiles)
{
	static BOOLEAN fOverPool  = FALSE;
	static BOOLEAN fOverEnemy = FALSE;

	MouseMoveState const uiCursorFlags = GetCursorMovementFlags();

	// Default gridno to mouse pos
	INT16 sActionGridNo = usMapPos;

	// Look for being on a merc....
	// Steal.....
	UIHandleOnMerc(FALSE);

	gfBeginVehicleCursor = FALSE;

	SOLDIERTYPE const* const tgt = gUIFullTarget;
	if (tgt)
	{
		if (OK_ENTERABLE_VEHICLE(tgt) && tgt->bVisible != -1)
		{
			// grab number of occupants in vehicles
			if (!fItemsOnlyIfOnIntTiles)
			{
				guiNewUICursor = ENTER_VEHICLE_UICURSOR;
				return true;
			}
			else if (!OKUseVehicle(tgt->ubProfile))
			{
				guiNewUICursor       = CANNOT_MOVE_UICURSOR;
				gfBeginVehicleCursor = TRUE;
				return true;
			}
			else if (GetNumberInVehicle(GetVehicle(tgt->bVehicleID)) == 0)
			{
				guiNewUICursor       = ENTER_VEHICLE_UICURSOR;
				gfBeginVehicleCursor = TRUE;
				return true;
			}
		}

		if (!fItemsOnlyIfOnIntTiles &&
			guiUIFullTargetFlags & ENEMY_MERC &&
			!(guiUIFullTargetFlags & UNCONSCIOUS_MERC))
		{
			if (!fOverEnemy)
			{
				fOverEnemy        = TRUE;
				gfPlotNewMovement = TRUE;
			}

			//Set UI CURSOR
			guiNewUICursor = fUseOKCursor ||
						(gTacticalStatus.uiFlags & INCOMBAT) ?
						OKHANDCURSOR_UICURSOR : NORMALHANDCURSOR_UICURSOR;

			HandleUIMovementCursor(pSoldier, uiCursorFlags, sActionGridNo, MOVEUI_TARGET_STEAL);

			// Display action points
			gfUIDisplayActionPoints = TRUE;

			// Determine if we can afford!
			if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE))
			{
				gfUIDisplayActionPointsInvalid = TRUE;
			}

			return false;
		}
	}

	if (fOverEnemy)
	{
		ErasePath();
		fOverEnemy        = FALSE;
		gfPlotNewMovement = TRUE;
	}

	// If we are over an interactive struct, adjust gridno to this....
	INT16            sIntTileGridNo;
	STRUCTURE*       pStructure;
	LEVELNODE* const pIntTile = ConditionalGetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure, FALSE);
	gpInvTileThatCausedMoveConfirm = pIntTile;

	if (pIntTile) sActionGridNo = sIntTileGridNo;

	if (fItemsOnlyIfOnIntTiles && !pIntTile)
	{
		// If we want only on int tiles, and we have no int tiles, then ignore items!
	}
	else if (fItemsOnlyIfOnIntTiles && pIntTile && pStructure->fFlags & STRUCTURE_HASITEMONTOP)
	{
		// if in this mode, we don't want to automatically show hand cursor over items on strucutres
	}
	else if (pIntTile && pStructure->fFlags & STRUCTURE_SWITCH)
	{
		// We don't want switches messing around with items ever!
	}
	else if (pIntTile && pStructure->fFlags & STRUCTURE_ANYDOOR && (sActionGridNo != usMapPos || fItemsOnlyIfOnIntTiles))
	{
		// Next we look for if we are over a door and if the mouse position is != base door position, ignore items!
	}
	else
	{
		// Check if we are over an item pool
		ITEM_POOL const* const pItemPool = GetItemPool(sActionGridNo, pSoldier->bLevel);
		if (IsItemPoolVisible(pItemPool) ||
			(gpWorldLevelData[sActionGridNo].uiFlags & MAPELEMENT_REVEALED &&
			DoesItemPoolContainAnyHiddenItems(pItemPool)))
		{
			if (!fOverPool)
			{
				fOverPool         = TRUE;
				gfPlotNewMovement = TRUE;
			}

			//Set UI CURSOR
			guiNewUICursor = fUseOKCursor ||
						(gTacticalStatus.uiFlags & INCOMBAT) ?
						OKHANDCURSOR_UICURSOR : NORMALHANDCURSOR_UICURSOR;

			HandleUIMovementCursor(pSoldier, uiCursorFlags, sActionGridNo, MOVEUI_TARGET_ITEMS);

			// Display action points
			gfUIDisplayActionPoints = TRUE;

			if (gsOverItemsGridNo == sActionGridNo) gfPlotNewMovement = TRUE;

			// Determine if we can afford!
			if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE))
			{
				gfUIDisplayActionPointsInvalid = TRUE;
			}

			return pIntTile != 0;
		}
	}

	if (pIntTile)
	{
		if (fOverPool)
		{
			ErasePath();
			fOverPool         = FALSE;
			gfPlotNewMovement = TRUE;
		}

		HandleUIMovementCursor(pSoldier, uiCursorFlags, usMapPos, MOVEUI_TARGET_INTTILES);

		guiNewUICursor = GetInteractiveTileCursor(guiNewUICursor, fUseOKCursor);
		return true;
	}
	else if (!fItemsOnlyIfOnIntTiles)
	{
		// Let's at least show where the merc will walk to if they go here...
		if (!fOverPool)
		{
			fOverPool         = TRUE;
			gfPlotNewMovement = TRUE;
		}

		HandleUIMovementCursor(pSoldier, uiCursorFlags, sActionGridNo, MOVEUI_TARGET_ITEMS);

		// Display action points
		gfUIDisplayActionPoints = TRUE;

		// Determine if we can afford!
		if (!EnoughPoints(pSoldier, gsCurrentActionPoints, 0, FALSE))
		{
			gfUIDisplayActionPointsInvalid = TRUE;
		}
	}
	return false;
}


void HandleTacticalUILoseCursorFromOtherScreen( )
{
	SetUICursor(NO_UICURSOR);

	gfTacticalForceNoCursor = TRUE;

	ErasePath();

	(*(GameScreens[GAME_SCREEN].HandleScreen))();

	gfTacticalForceNoCursor = FALSE;

	SetUICursor( guiCurrentUICursor );
}


BOOLEAN SelectedGuyInBusyAnimation( )
{
	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return FALSE;

	return sel->usAnimState == LOB_ITEM                       ||
		sel->usAnimState == THROW_ITEM                     ||
		sel->usAnimState == PICKUP_ITEM                    ||
		sel->usAnimState == DROP_ITEM                      ||
		sel->usAnimState == OPEN_DOOR                      ||
		sel->usAnimState == OPEN_STRUCT                    ||
		sel->usAnimState == OPEN_STRUCT                    ||
		sel->usAnimState == END_OPEN_DOOR                  ||
		sel->usAnimState == END_OPEN_LOCKED_DOOR           ||
		sel->usAnimState == ADJACENT_GET_ITEM              ||
		sel->usAnimState == DROP_ADJACENT_OBJECT           ||
		sel->usAnimState == OPEN_DOOR_CROUCHED             ||
		sel->usAnimState == BEGIN_OPENSTRUCT_CROUCHED      ||
		sel->usAnimState == CLOSE_DOOR_CROUCHED            ||
		sel->usAnimState == OPEN_DOOR_CROUCHED             ||
		sel->usAnimState == OPEN_STRUCT_CROUCHED           ||
		sel->usAnimState == END_OPENSTRUCT_CROUCHED        ||
		sel->usAnimState == END_OPEN_DOOR_CROUCHED         ||
		sel->usAnimState == END_OPEN_LOCKED_DOOR_CROUCHED  ||
		sel->usAnimState == END_OPENSTRUCT_LOCKED_CROUCHED ||
		sel->usAnimState == BEGIN_OPENSTRUCT;
}


void ClimbUpOrDown()
{
	SOLDIERTYPE* const s = GetSelectedMan();
	if (s)
	{
		UINT8 direction;
		if (FindHigherLevel(s, &direction))
		{
			BeginSoldierClimbUpRoof(s);
		}
		else
		{
			BeginSoldierClimbDownRoof(s);
		}
	}
}

void GotoHigherStance(SOLDIERTYPE* const s)
{
	switch (gAnimControl[s->usAnimState].ubEndHeight)
	{
		case ANIM_STAND:
			if (FindHigherLevel(s))
				BeginSoldierClimbUpRoof(s);
			break;
		case ANIM_CROUCH:
			HandleStanceChangeFromUIKeys(ANIM_STAND);
			break;
		case ANIM_PRONE:
			HandleStanceChangeFromUIKeys(ANIM_CROUCH);
			break;
	}
}


void GotoLowerStance(SOLDIERTYPE* const s)
{
	switch (gAnimControl[s->usAnimState].ubEndHeight)
	{
		case ANIM_STAND:
			HandleStanceChangeFromUIKeys(ANIM_CROUCH);
			break;
		case ANIM_CROUCH:
			HandleStanceChangeFromUIKeys(ANIM_PRONE);
			break;
		case ANIM_PRONE:
			if (FindLowerLevel(s))
				BeginSoldierClimbDownRoof(s);
			break;
	}
}

void SetInterfaceHeightLevel( )
{
	INT16 sHeight;
	static INT16 sOldHeight = 0;
	INT16 sGridNo;

	if( gfBasement || gfCaves )
	{
		gsRenderHeight = 0;
		sOldHeight = 0;

		return;
	}


	// ATE: Use an entry point to determine what height to use....
	if( gMapInformation.sNorthGridNo != -1 )
		sGridNo = gMapInformation.sNorthGridNo;
	else if( gMapInformation.sEastGridNo != -1 )
		sGridNo = gMapInformation.sEastGridNo;
	else if( gMapInformation.sSouthGridNo != -1 )
		sGridNo = gMapInformation.sSouthGridNo;
	else if( gMapInformation.sWestGridNo != -1 )
		sGridNo = gMapInformation.sWestGridNo;
	else
	{
		SLOGA("SetInterfaceHeightLevel: MapInformation seems corrupted");
		return;
	}


	sHeight = gpWorldLevelData[ sGridNo ].sHeight;

	if ( sHeight != sOldHeight )
	{
		gsRenderHeight = sHeight;

		if ( gsInterfaceLevel > 0 )
		{
			gsRenderHeight += ROOF_LEVEL_HEIGHT;
		}

		SetRenderFlags(RENDER_FLAG_FULL);
		ErasePath();

		sOldHeight = sHeight;
	}
}


BOOLEAN ValidQuickExchangePosition(void)
{
	BOOLEAN fOnValidGuy = FALSE;
	static BOOLEAN fOldOnValidGuy = FALSE;

	// Check if we over a civ
	const SOLDIERTYPE* const pOverSoldier = gUIFullTarget;
	if (pOverSoldier != NULL)
	{
		//KM: Replaced this older if statement for the new one which allows exchanging with militia
		//if ((pOverSoldier->bSide != OUR_TEAM) && pOverSoldier->bNeutral)
		if ((pOverSoldier->bTeam != OUR_TEAM && pOverSoldier->bNeutral) ||
			(pOverSoldier->bTeam == MILITIA_TEAM && pOverSoldier->bSide == 0))
		{
			// hehe - don't allow animals to exchange places
			if ( !( pOverSoldier->uiStatusFlags & ( SOLDIER_ANIMAL ) ) )
			{
				// OK, we have a civ , now check if they are near selected guy.....
				SOLDIERTYPE* const sel = GetSelectedMan();
				if (sel != NULL && PythSpacesAway(sel->sGridNo, pOverSoldier->sGridNo) == 1)
				{
					// Check if we have LOS to them....
					const INT16 sDistVisible = DistanceVisible(sel, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, pOverSoldier->sGridNo, pOverSoldier->bLevel);
					if (SoldierTo3DLocationLineOfSightTest(sel, pOverSoldier->sGridNo,  pOverSoldier->bLevel, 3, sDistVisible, TRUE))
					{
						// ATE:
						// Check that the path is good!
						if (FindBestPath(sel, pOverSoldier->sGridNo, sel->bLevel, sel->usUIMovementMode, NO_COPYROUTE, PATH_IGNORE_PERSON_AT_DEST) == 1)
						{
							fOnValidGuy = TRUE;
						}
					}
				}
			}
		}
	}

	if ( fOldOnValidGuy != fOnValidGuy )
	{
		// Update timer....
		// ATE: Adjust clock for automatic swapping so that the 'feel' is there....
		guiUIInterfaceSwapCursorsTime	= GetJA2Clock( );
		// Default it!
		gfOKForExchangeCursor = TRUE;
	}

	// Update old value.....
	fOldOnValidGuy = fOnValidGuy;

	if ( !gfOKForExchangeCursor )
	{
		fOnValidGuy = FALSE;
	}

	return( fOnValidGuy );
}


// This function contains the logic for allowing the player
// to jump over people.
BOOLEAN IsValidJumpLocation(const SOLDIERTYPE* pSoldier, INT16 sGridNo, BOOLEAN fCheckForPath)
{
	INT16 sSpot, sIntSpot;
	UINT8 sDirs[4] = { NORTH, EAST, SOUTH, WEST };
	UINT8 cnt;
	UINT8 ubMovementCost;

	// First check that action point cost is zero so far
	// ie: NO PATH!
	if ( gsCurrentActionPoints != 0 && fCheckForPath )
	{
		return( FALSE );
	}

	// Loop through positions...
	for (cnt = 0; cnt < 4; cnt++)
	{
		// MOVE OUT TWO DIRECTIONS
		sIntSpot = NewGridNo( sGridNo, DirectionInc( sDirs[ cnt ] ) );

		// ATE: Check our movement costs for going through walls!
		ubMovementCost = gubWorldMovementCosts[ sIntSpot ][ sDirs[ cnt ] ][ pSoldier->bLevel ];
		if ( IS_TRAVELCOST_DOOR( ubMovementCost ) )
		{
			ubMovementCost = DoorTravelCost(pSoldier, sIntSpot, ubMovementCost, pSoldier->bTeam == OUR_TEAM, NULL);
		}

		// If we have hit an obstacle, STOP HERE
		if ( ubMovementCost >= TRAVELCOST_BLOCKED )
		{
			// no good, continue
			continue;
		}


		// TWICE AS FAR!
		sSpot = NewGridNo( sIntSpot, DirectionInc( sDirs[ cnt ] ) );

		// Is the soldier we're looking at here?
		if (WhoIsThere2(sSpot, pSoldier->bLevel) == pSoldier)
		{
			// Double check OK destination......
			if ( NewOKDestination( pSoldier, sGridNo, TRUE, (INT8)gsInterfaceLevel ) )
			{
				// If the soldier in the middle of doing stuff?
				if ( !pSoldier->fTurningUntilDone )
				{
					// OK, NOW check if there is a guy in between us
					const SOLDIERTYPE* const guy_there = WhoIsThere2(sIntSpot, pSoldier->bLevel);

					// Is there a guy and is he prone?
					if (guy_there != NULL &&
						guy_there != pSoldier &&
						gAnimControl[guy_there->usAnimState].ubHeight == ANIM_PRONE)
					{
						// It's a GO!
						return( TRUE );
					}
				}
			}
		}
	}

	return( FALSE );
}
