#include <stdarg.h>
#include "Font.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "Gameloop.h"
#include "HImage.h"
#include "Soldier_Find.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "SysUtil.h"
#include "Overhead.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Interface.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Input.h"
#include "Handle_UI.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "RenderWorld.h"
#include "Cursors.h"
#include "Radar_Screen.h"
#include "WorldMan.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "Interface_Cursors.h"
#include "Sound_Control.h"
#include "Interface_Panels.h"
#include "PathAI.h"
#include "Faces.h"
#include "Handle_UI_Plan.h"
#include "Interface_Control.h"
#include "Interface_Items.h"
#include "Soldier_Profile.h"
#include "MercTextBox.h"
#include "Soldier_Functions.h"
#include "Cursor_Control.h"
#include "Handle_Doors.h"
#include "Keys.h"
#include "Text.h"
#include "Points.h"
#include "Soldier_Macros.h"
#include "Game_Clock.h"
#include "Physics.h"
#include "Map_Screen_Interface_Map.h"
#include "Line.h"
#include "Vehicles.h"
#include "MessageBoxScreen.h"
#include "GameSettings.h"
#include "Squads.h"
#include "Message.h"
#include "Debug.h"
#include "Video.h"
#include "Items.h"


#define ARROWS_X_OFFSET			10
#define	ARROWS_HEIGHT				20
#define	ARROWS_WIDTH				20
#define UPARROW_Y_OFFSET		-30
#define DOWNARROW_Y_OFFSET	-10

#define	BUTTON_PANEL_WIDTH	78
#define	BUTTON_PANEL_HEIGHT	76


BOOLEAN	gfInMovementMenu = FALSE;
static INT32 giMenuAnchorX;
static INT32 giMenuAnchorY;


#define PROG_BAR_START_X			5
#define PROG_BAR_START_Y			2
#define PROG_BAR_LENGTH				627

static BOOLEAN gfProgBarActive   = FALSE;
static UINT8   gubProgNumEnemies = 0;
static UINT8   gubProgCurEnemy   = 0;

typedef struct
{
	SGPVSurface* uiSurface;
	INT8				bCurrentMessage;
	UINT32			uiTimeOfLastUpdate;
	UINT32			uiTimeSinceLastBeep;
	INT8				bAnimate;
	INT8				bYPos;
	BOOLEAN			fCreated;
	INT16				sWorldRenderX;
	INT16				sWorldRenderY;

} TOP_MESSAGE;

static TOP_MESSAGE gTopMessage;
BOOLEAN gfTopMessageDirty = FALSE;

extern UINT16 GetAnimStateForInteraction( SOLDIERTYPE *pSoldier, BOOLEAN fDoor, UINT16 usAnimState );


static MOUSE_REGION gMenuOverlayRegion;


VIDEO_OVERLAY*      g_ui_message_overlay = NULL;
static UINT16       gusUIMessageWidth;
static UINT16       gusUIMessageHeight;
static MercPopUpBox gpUIMessageOverrideMercBox;
UINT32				guiUIMessageTime = 0;
static INT32        iUIMessageBox = -1;
UINT32				guiUIMessageTimeDelay = 0;
static BOOLEAN      gfUseSkullIconMessage = FALSE;

static BOOLEAN gfPanelAllocated = FALSE;


enum
{
	WALK_IMAGES = 0,
	SNEAK_IMAGES,
	RUN_IMAGES,
	CRAWL_IMAGES,
	LOOK_IMAGES,
	TALK_IMAGES,
	HAND_IMAGES,
	CANCEL_IMAGES,

	TARGETACTIONC_IMAGES,
	KNIFEACTIONC_IMAGES,
	AIDACTIONC_IMAGES,
	PUNCHACTIONC_IMAGES,
	BOMBACTIONC_IMAGES,

	OPEN_DOOR_IMAGES,
	EXAMINE_DOOR_IMAGES,
	LOCKPICK_DOOR_IMAGES,
	BOOT_DOOR_IMAGES,
	CROWBAR_DOOR_IMAGES,
	USE_KEY_IMAGES,
	USE_KEYRING_IMAGES,
	EXPLOSIVE_DOOR_IMAGES,

	TOOLKITACTIONC_IMAGES,
	WIRECUTACTIONC_IMAGES,

	NUM_ICON_IMAGES
};

static BUTTON_PICS* iIconImages[NUM_ICON_IMAGES];

enum
{
	WALK_ICON,
	SNEAK_ICON,
	RUN_ICON,
	CRAWL_ICON,
	LOOK_ICON,
	ACTIONC_ICON,
	TALK_ICON,
	HAND_ICON,

	OPEN_DOOR_ICON,
	EXAMINE_DOOR_ICON,
	LOCKPICK_DOOR_ICON,
	BOOT_DOOR_ICON,
	UNTRAP_DOOR_ICON,
	USE_KEY_ICON,
	USE_KEYRING_ICON,
	EXPLOSIVE_DOOR_ICON,
	USE_CROWBAR_ICON,

	CANCEL_ICON,
	NUM_ICONS
};


static INT32 iActionIcons[NUM_ICONS];

// GLOBAL INTERFACE SURFACES
SGPVObject* guiCLOSE;
SGPVObject* guiDEAD;
SGPVObject* guiHATCH;
SGPVObject* guiGUNSM;
SGPVObject* guiP1ITEMS;
SGPVObject* guiP2ITEMS;
SGPVObject* guiP3ITEMS;
static SGPVObject* guiBUTTONBORDER;
SGPVObject* guiRADIO;
static SGPVObject* guiRADIO2;
SGPVObject* guiCOMPANEL;
SGPVObject* guiCOMPANELB;
static SGPVObject* guiAIMCUBES;
static SGPVObject* guiAIMBARS;
SGPVObject* guiVEHINV;
SGPVObject* guiBURSTACCUM;
SGPVObject* guiPORTRAITICONS;

// UI Globals
MOUSE_REGION	gViewportRegion;
MOUSE_REGION	gRadarRegion;


static UINT16 gsUpArrowX;
static UINT16 gsUpArrowY;
static UINT16 gsDownArrowX;
static UINT16 gsDownArrowY;

static UINT32 giUpArrowRect   = NO_BGND_RECT;
static UINT32 giDownArrowRect = NO_BGND_RECT;


BOOLEAN fFirstTimeInGameScreen  = TRUE;
BOOLEAN	fInterfacePanelDirty	  = DIRTYLEVEL2;
INT16		gsInterfaceLevel			  = I_GROUND_LEVEL;
INT16		gsCurInterfacePanel			= TEAM_PANEL;


BOOLEAN InitializeTacticalInterface(  )
{
	// Load button Interfaces
	iIconImages[ WALK_IMAGES  ]			= LoadButtonImage("INTERFACE/newicons3.sti", -1,3,4,5,-1 );
	iIconImages[ SNEAK_IMAGES ]			= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 6, 7, 8, -1 );
	iIconImages[ RUN_IMAGES ]				= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 0, 1, 2, -1 );
	iIconImages[ CRAWL_IMAGES ]			= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 9, 10, 11, -1 );
	iIconImages[ LOOK_IMAGES ]			= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 12, 13, 14, -1 );
	iIconImages[ TALK_IMAGES ]			= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 21, 22, 23, -1 );
	iIconImages[ HAND_IMAGES ]			= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 18, 19, 20, -1 );
	iIconImages[ CANCEL_IMAGES ]		= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 15, 16, 17, -1 );

	iIconImages[ TARGETACTIONC_IMAGES ]		= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 24, 25, 26, -1 );
	iIconImages[ KNIFEACTIONC_IMAGES ]		= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 27, 28, 29, -1 );
	iIconImages[ AIDACTIONC_IMAGES ]			= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 30, 31, 32, -1 );
	iIconImages[ PUNCHACTIONC_IMAGES ]		= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 33, 34, 35, -1 );
	iIconImages[ BOMBACTIONC_IMAGES ]			= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 36, 37, 38, -1 );
#ifndef JA2DEMO
	iIconImages[ TOOLKITACTIONC_IMAGES ]	= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 39, 40, 41, -1 );
	iIconImages[ WIRECUTACTIONC_IMAGES ]	= UseLoadedButtonImage(iIconImages[ WALK_IMAGES  ], -1, 42, 43, 44, -1 );
#endif

	iIconImages[ OPEN_DOOR_IMAGES ]				= LoadButtonImage("INTERFACE/door_op2.sti", -1,9,10,11,-1 );
	iIconImages[ EXAMINE_DOOR_IMAGES ]		= UseLoadedButtonImage(iIconImages[ OPEN_DOOR_IMAGES  ], -1, 12, 13, 14, -1 );
	iIconImages[ LOCKPICK_DOOR_IMAGES ]		= UseLoadedButtonImage(iIconImages[ OPEN_DOOR_IMAGES  ], -1, 21, 22, 23, -1 );
	iIconImages[ BOOT_DOOR_IMAGES ]				= UseLoadedButtonImage(iIconImages[ OPEN_DOOR_IMAGES  ], -1, 25, 26, 27, -1 );
	iIconImages[ CROWBAR_DOOR_IMAGES ]		= UseLoadedButtonImage(iIconImages[ OPEN_DOOR_IMAGES  ], -1, 0, 1, 2, -1 );
	iIconImages[ USE_KEY_IMAGES ]					= UseLoadedButtonImage(iIconImages[ OPEN_DOOR_IMAGES  ], -1, 3, 4, 5, -1 );
	iIconImages[ USE_KEYRING_IMAGES ]			= UseLoadedButtonImage(iIconImages[ OPEN_DOOR_IMAGES  ], -1, 6, 7, 8, -1 );
	iIconImages[ EXPLOSIVE_DOOR_IMAGES ]	= UseLoadedButtonImage(iIconImages[ OPEN_DOOR_IMAGES  ], -1, 15, 16, 17, -1 );

	// Load interface panels
	// failing the CHECKF after this will cause you to lose your mouse

	// LOAD CLOSE ANIM
	guiCLOSE = AddVideoObjectFromFile("INTERFACE/p_close.sti");
	AssertMsg(guiCLOSE != NO_VOBJECT, "Missing INTERFACE/p_close.sti");

	// LOAD DEAD ANIM
	guiDEAD = AddVideoObjectFromFile("INTERFACE/p_dead.sti");
	AssertMsg(guiDEAD != NO_VOBJECT, "Missing INTERFACE/p_dead.sti");

	// LOAD HATCH
	guiHATCH = AddVideoObjectFromFile("INTERFACE/hatch.sti");
	AssertMsg(guiHATCH != NO_VOBJECT, "Missing INTERFACE/hatch.sti");

	// LOAD INTERFACE GUN PICTURES
	guiGUNSM = AddVideoObjectFromFile("INTERFACE/mdguns.sti");
	AssertMsg(guiGUNSM != NO_VOBJECT, "Missing INTERFACE/mdguns.sti");

	// LOAD INTERFACE ITEM PICTURES
	guiP1ITEMS = AddVideoObjectFromFile("INTERFACE/mdp1items.sti");
	AssertMsg(guiP1ITEMS != NO_VOBJECT, "Missing INTERFACE/mdplitems.sti");

	// LOAD INTERFACE ITEM PICTURES
	guiP2ITEMS = AddVideoObjectFromFile("INTERFACE/mdp2items.sti");
	AssertMsg(guiP2ITEMS != NO_VOBJECT, "Missing INTERFACE/mdp2items.sti");

	// LOAD INTERFACE ITEM PICTURES
	guiP3ITEMS = AddVideoObjectFromFile("INTERFACE/mdp3items.sti");
	AssertMsg(guiP3ITEMS != NO_VOBJECT, "Missing INTERFACE/mdp3items.sti");

	// LOAD INTERFACE BUTTON BORDER
	guiBUTTONBORDER = AddVideoObjectFromFile("INTERFACE/button_frame.sti");
	AssertMsg(guiBUTTONBORDER != NO_VOBJECT, "Missing INTERFACE/button_frame.sti");

	// LOAD AIM CUBES
	guiAIMCUBES = AddVideoObjectFromFile("INTERFACE/aimcubes.sti");
	AssertMsg(guiAIMCUBES != NO_VOBJECT, "Missing INTERFACE/aimcubes.sti");

	// LOAD AIM BARS
	guiAIMBARS = AddVideoObjectFromFile("INTERFACE/aimbars.sti");
	AssertMsg(guiAIMBARS != NO_VOBJECT, "Missing INTERFACE/aimbars.sti");

	guiVEHINV = AddVideoObjectFromFile("INTERFACE/inventor.sti");
	AssertMsg(guiVEHINV != NO_VOBJECT, "Missing INTERFACE/inventor.sti");

	guiBURSTACCUM = AddVideoObjectFromFile("INTERFACE/burst1.sti");
	AssertMsg(guiBURSTACCUM != NO_VOBJECT, "Missing INTERFACE/burst1.sti");

	guiPORTRAITICONS = AddVideoObjectFromFile("INTERFACE/portraiticons.sti");
	AssertMsg(guiPORTRAITICONS != NO_VOBJECT, "Missing INTERFACE/portraiticons.sti");

	// LOAD RADIO
	guiRADIO = AddVideoObjectFromFile("INTERFACE/radio.sti");
	AssertMsg(guiRADIO != NO_VOBJECT, "Missing INTERFACE/radio.sti");

	// LOAD RADIO2
	guiRADIO2 = AddVideoObjectFromFile("INTERFACE/radio2.sti");
	AssertMsg(guiRADIO2 != NO_VOBJECT, "Missing INTERFACE/radio2.sti");

	// LOAD com panel 2
	guiCOMPANEL = AddVideoObjectFromFile("INTERFACE/communicationpopup.sti");
	AssertMsg(guiCOMPANEL != NO_VOBJECT, "Missing INTERFACE/communicationpopup.sti");

	guiCOMPANELB = AddVideoObjectFromFile("INTERFACE/communicationpopup_2.sti");
	AssertMsg(guiCOMPANELB != NO_VOBJECT, "Missing INTERFACE/communicationpopup_2.sti");


	// Alocate message surfaces
	gTopMessage.uiSurface = AddVideoSurface(SCREEN_WIDTH, 20, PIXEL_DEPTH);
	CHECKF(gTopMessage.uiSurface != NO_VSURFACE);

	InitItemInterface( );

	InitRadarScreen( );

	InitTEAMSlots( );

	// Init popup box images
//	CHECKF( LoadTextMercPopupImages( BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER ) );

	return( TRUE );
}


BOOLEAN InitializeCurrentPanel( )
{
	BOOLEAN		fOK = FALSE;

	MoveRadarScreen( );

	switch( gsCurInterfacePanel )
	{
		case SM_PANEL:
			// Set new viewport
			gsVIEWPORT_WINDOW_END_Y = INV_INTERFACE_START_Y;

			// Render full
			SetRenderFlags(RENDER_FLAG_FULL);
			fOK = InitializeSMPanel( );
			break;

		case TEAM_PANEL:
			gsVIEWPORT_WINDOW_END_Y = INTERFACE_START_Y;
			// Render full
			SetRenderFlags(RENDER_FLAG_FULL);
			fOK = InitializeTEAMPanel( );
			break;
	}

//	RefreshMouseRegions( );
	gfPanelAllocated = TRUE;

	return( fOK );
}

void ShutdownCurrentPanel( )
{
	if ( gfPanelAllocated )
	{

		switch( gsCurInterfacePanel )
		{
			case SM_PANEL:
				ShutdownSMPanel( );
				break;

			case TEAM_PANEL:
				ShutdownTEAMPanel( );
				break;
		}

		gfPanelAllocated = FALSE;

	}
}


void SetCurrentTacticalPanelCurrentMerc(SOLDIERTYPE* s)
{
	// Disable faces
	SetAllAutoFacesInactive( );

	if ( gsCurInterfacePanel == SM_PANEL )
	{
		// If we are not of merc bodytype, or am an epc, and going into inv, goto another....
		if (!IS_MERC_BODY_TYPE(s) || AM_AN_EPC(s))
		{
			SetCurrentInterfacePanel( TEAM_PANEL );
		}
	}

	switch( gsCurInterfacePanel )
	{
		case SM_PANEL:   gSelectSMPanelToMerc = s;  break;
		case TEAM_PANEL: SetTEAMPanelCurrentMerc(); break;
	}
}


void CreateCurrentTacticalPanelButtons(void)
{
	switch (gsCurInterfacePanel)
	{
		case SM_PANEL:   CreateSMPanelButtons();   break;
		case TEAM_PANEL: CreateTEAMPanelButtons(); break;
	}
}


void SetCurrentInterfacePanel( UINT8 ubNewPanel )
{
	ShutdownCurrentPanel( );

	// INit new panel
	gsCurInterfacePanel = ubNewPanel;

	InitializeCurrentPanel( );


}


void ToggleTacticalPanels( )
{
	gfSwitchPanel			= TRUE;
	gNewPanelSoldier = GetSelectedMan();

	if ( gsCurInterfacePanel == SM_PANEL )
	{
		gbNewPanel = TEAM_PANEL;
	}
	else
	{
		gbNewPanel = SM_PANEL;
	}
}


void RemoveCurrentTacticalPanelButtons(void)
{
	switch (gsCurInterfacePanel)
	{
		case SM_PANEL:   RemoveSMPanelButtons();   break;
		case TEAM_PANEL: RemoveTEAMPanelButtons(); break;
	}
}


BOOLEAN IsMercPortraitVisible(const SOLDIERTYPE* s)
{
	switch (gsCurInterfacePanel)
	{
		case TEAM_PANEL: return TRUE;
		case SM_PANEL:   return gpSMCurrentMerc == s;
		default:         return FALSE;
	}
}


static void HandleUpDownArrowBackgrounds(void);


void HandleInterfaceBackgrounds( )
{
	HandleUpDownArrowBackgrounds( );
}


static void BtnMovementCallback(GUI_BUTTON* btn, INT32 reason);


static BOOLEAN MakeButtonMove(UINT idx, UINT gfx, INT16 x, INT16 y, UI_EVENT* event, const wchar_t* help)
{
	INT32 btn = QuickCreateButton(iIconImages[gfx], x, y, MSYS_PRIORITY_HIGHEST - 1, BtnMovementCallback);
	iActionIcons[idx] = btn;
	if (btn == -1)
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
		return FALSE;
	}
	ButtonList[btn]->User.Ptr = event;
	SetButtonFastHelpText(btn, help);
	return TRUE;
}


static void MovementMenuBackregionCallback(MOUSE_REGION* pRegion, INT32 iReason);


void PopupMovementMenu( UI_EVENT *pUIEvent )
{
	INT32								iMenuAnchorX, iMenuAnchorY;
	UINT32							uiActionImages;
	BOOLEAN							fDisableAction = FALSE;

	// Erase other menus....
	EraseInterfaceMenus( TRUE );

	giMenuAnchorX = gusMouseXPos - 18;
	giMenuAnchorY = gusMouseYPos - 18;

	// ATE: OK loser, let's check if we're going off the screen!
	if ( giMenuAnchorX < 0 )
	{
		giMenuAnchorX = 0;
	}

	if ( giMenuAnchorY < 0 )
	{
		giMenuAnchorY = 0;
	}


	// Create mouse region over all area to facilitate clicking to end
	MSYS_DefineRegion(&gMenuOverlayRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 1, CURSOR_NORMAL, MSYS_NO_CALLBACK, MovementMenuBackregionCallback);


	// OK, CHECK FOR BOUNDARIES!
	if (giMenuAnchorX + BUTTON_PANEL_WIDTH > SCREEN_WIDTH)
	{
		giMenuAnchorX = SCREEN_WIDTH - BUTTON_PANEL_WIDTH;
	}
	if ( ( giMenuAnchorY + BUTTON_PANEL_HEIGHT ) > gsVIEWPORT_WINDOW_END_Y )
	{
		giMenuAnchorY = ( gsVIEWPORT_WINDOW_END_Y - BUTTON_PANEL_HEIGHT );
	}

	const SOLDIERTYPE* const s = GetSelectedMan();

	iMenuAnchorX = giMenuAnchorX + 9;
	iMenuAnchorY = giMenuAnchorY + 8;

	if (!MakeButtonMove(RUN_ICON, RUN_IMAGES, iMenuAnchorX + 20, iMenuAnchorY, pUIEvent, pTacticalPopupButtonStrings[RUN_ICON])) return;
	if (MercInWater(s) || s->uiStatusFlags & SOLDIER_VEHICLE || s->uiStatusFlags & SOLDIER_ROBOT)
	{
		DisableButton(iActionIcons[RUN_ICON]);
	}

	const wchar_t* const help = (s->uiStatusFlags & SOLDIER_VEHICLE ? TacticalStr[DRIVE_POPUPTEXT] : pTacticalPopupButtonStrings[WALK_ICON]);
	if (!MakeButtonMove(WALK_ICON, WALK_IMAGES, iMenuAnchorX + 40, iMenuAnchorY, pUIEvent, help)) return;
	if (s->uiStatusFlags & SOLDIER_ROBOT && !CanRobotBeControlled(s))
	{
		DisableButton(iActionIcons[WALK_ICON]);
	}

	if (!MakeButtonMove(SNEAK_ICON, SNEAK_IMAGES, iMenuAnchorX + 40, iMenuAnchorY + 20, pUIEvent, pTacticalPopupButtonStrings[SNEAK_ICON])) return;
	if (!IsValidStance(s, ANIM_CROUCH))
	{
		DisableButton(iActionIcons[SNEAK_ICON]);
	}

	if (!MakeButtonMove(CRAWL_ICON, CRAWL_IMAGES, iMenuAnchorX + 40, iMenuAnchorY + 40, pUIEvent, pTacticalPopupButtonStrings[CRAWL_ICON])) return;
	if (!IsValidStance(s, ANIM_PRONE))
	{
		DisableButton(iActionIcons[CRAWL_ICON]);
	}

	if (!MakeButtonMove(LOOK_ICON, LOOK_IMAGES, iMenuAnchorX, iMenuAnchorY, pUIEvent, TacticalStr[LOOK_CURSOR_POPUPTEXT])) return;
	if (s->uiStatusFlags & SOLDIER_VEHICLE ||
			s->uiStatusFlags & SOLDIER_ROBOT && !CanRobotBeControlled(s))
	{
		DisableButton(iActionIcons[LOOK_ICON]);
	}

	const wchar_t* Action;
	if (s->uiStatusFlags & SOLDIER_VEHICLE)
	{
		// Until we get mounted weapons...
		uiActionImages = CANCEL_IMAGES;
		Action = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
		fDisableAction = TRUE;
	}
	else
	{
		const UINT16 item = s->inv[HANDPOS].usItem;
		if (item == TOOLKIT)
		{
			uiActionImages = TOOLKITACTIONC_IMAGES;
			Action = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
		}
		else if (item == WIRECUTTERS)
		{
			uiActionImages = WIRECUTACTIONC_IMAGES;
			Action = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
		}
		else
		{
			// Create button based on what is in our hands at the moment!
			switch (Item[item].usItemClass)
			{
				case IC_PUNCH:
					uiActionImages = PUNCHACTIONC_IMAGES;
					Action = TacticalStr[USE_HANDTOHAND_POPUPTEXT];
					break;

				case IC_GUN:
					uiActionImages = TARGETACTIONC_IMAGES;
					Action = TacticalStr[USE_FIREARM_POPUPTEXT];
					break;

				case IC_BLADE:
					uiActionImages = KNIFEACTIONC_IMAGES;
					Action = TacticalStr[USE_BLADE_POPUPTEXT];
					break;

				case IC_GRENADE:
				case IC_BOMB:
					uiActionImages = BOMBACTIONC_IMAGES;
					Action = TacticalStr[USE_EXPLOSIVE_POPUPTEXT];
					break;

				case IC_MEDKIT:
					uiActionImages = AIDACTIONC_IMAGES;
					Action = TacticalStr[USE_MEDKIT_POPUPTEXT];
					break;

				default:
					uiActionImages = CANCEL_IMAGES;
					Action = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
					fDisableAction = TRUE;
					break;
			}
		}
	}

	if (AM_AN_EPC(s)) fDisableAction = TRUE;

	if (!MakeButtonMove(ACTIONC_ICON, uiActionImages, iMenuAnchorX, iMenuAnchorY + 20, pUIEvent, Action)) return;
	if (fDisableAction)
	{
		DisableButton(iActionIcons[ACTIONC_ICON]);
	}

	if (!MakeButtonMove(TALK_ICON, TALK_IMAGES, iMenuAnchorX, iMenuAnchorY + 40, pUIEvent, pTacticalPopupButtonStrings[TALK_ICON])) return;
	if (AM_AN_EPC(s) || s->uiStatusFlags & SOLDIER_VEHICLE)
	{
		DisableButton(iActionIcons[TALK_ICON]);
	}

	if (!MakeButtonMove(HAND_ICON, HAND_IMAGES, iMenuAnchorX + 20, iMenuAnchorY + 40, pUIEvent, pTacticalPopupButtonStrings[HAND_ICON])) return;
	if (AM_AN_EPC(s) || s->uiStatusFlags & SOLDIER_VEHICLE)
	{
		DisableButton(iActionIcons[HAND_ICON]);
	}

	if (!MakeButtonMove(CANCEL_ICON, CANCEL_IMAGES, iMenuAnchorX + 20, iMenuAnchorY + 20, pUIEvent, pTacticalPopupButtonStrings[CANCEL_ICON])) return;

	gfInMovementMenu = TRUE;

	// Ignore scrolling
	gfIgnoreScrolling = TRUE;

}

void PopDownMovementMenu( )
{
	if ( gfInMovementMenu )
	{
		RemoveButton( iActionIcons[ WALK_ICON  ] );
		RemoveButton( iActionIcons[ SNEAK_ICON  ] );
		RemoveButton( iActionIcons[ RUN_ICON  ] );
		RemoveButton( iActionIcons[ CRAWL_ICON  ] );
		RemoveButton( iActionIcons[ LOOK_ICON  ] );
		RemoveButton( iActionIcons[ ACTIONC_ICON  ] );
		RemoveButton( iActionIcons[ TALK_ICON  ] );
		RemoveButton( iActionIcons[ HAND_ICON  ] );
		RemoveButton( iActionIcons[ CANCEL_ICON  ] );

		// Turn off Ignore scrolling
		gfIgnoreScrolling = FALSE;

		// Rerender world
		SetRenderFlags( RENDER_FLAG_FULL );

		fInterfacePanelDirty = DIRTYLEVEL2;

		MSYS_RemoveRegion( &gMenuOverlayRegion );
	}

	gfInMovementMenu = FALSE;

}

void RenderMovementMenu( )
{
	if ( gfInMovementMenu )
	{
		BltVideoObject(FRAME_BUFFER, guiBUTTONBORDER, 0, giMenuAnchorX, giMenuAnchorY);

		// Mark buttons dirty!
		MarkAButtonDirty( iActionIcons[ WALK_ICON  ] );
		MarkAButtonDirty( iActionIcons[ SNEAK_ICON  ] );
		MarkAButtonDirty( iActionIcons[ RUN_ICON  ] );
		MarkAButtonDirty( iActionIcons[ CRAWL_ICON  ] );
		MarkAButtonDirty( iActionIcons[ LOOK_ICON  ] );
		MarkAButtonDirty( iActionIcons[ ACTIONC_ICON  ] );
		MarkAButtonDirty( iActionIcons[ TALK_ICON  ] );
		MarkAButtonDirty( iActionIcons[ HAND_ICON  ] );
		MarkAButtonDirty( iActionIcons[ CANCEL_ICON  ] );

		InvalidateRegion( giMenuAnchorX, giMenuAnchorY, giMenuAnchorX + BUTTON_PANEL_WIDTH, giMenuAnchorY + BUTTON_PANEL_HEIGHT );

	}
}

void CancelMovementMenu( )
{
	// Signal end of event
	PopDownMovementMenu( );
	guiPendingOverrideEvent		= A_CHANGE_TO_MOVE;
}


static void BtnMovementCallback(GUI_BUTTON* btn, INT32 reason)
{
	INT32		uiBtnID;

	if ( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;

		uiBtnID = btn->IDNum;

		UI_EVENT* pUIEvent = (UI_EVENT*)btn->User.Ptr;

		if ( uiBtnID == iActionIcons[ WALK_ICON  ] )
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_WALK;
		}
		else if ( uiBtnID == iActionIcons[ RUN_ICON  ] )
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_RUN;
		}
		else if ( uiBtnID == iActionIcons[ SNEAK_ICON  ] )
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_SWAT;
		}
		else if ( uiBtnID == iActionIcons[ CRAWL_ICON  ] )
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_PRONE;
		}
		else if ( uiBtnID == iActionIcons[ LOOK_ICON  ] )
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_LOOK;
		}
		else if ( uiBtnID == iActionIcons[ ACTIONC_ICON  ] )
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_ACTIONC;
		}
		else if ( uiBtnID == iActionIcons[ TALK_ICON  ] )
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_TALK;
		}
		else if ( uiBtnID == iActionIcons[ HAND_ICON  ] )
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_HAND;
		}
		else if ( uiBtnID == iActionIcons[ CANCEL_ICON  ] )
		{
			// Signal end of event
			EndMenuEvent( U_MOVEMENT_MENU );
			pUIEvent->uiParams[1] = FALSE;
			return;
		}
		else
		{
			return;
		}

		// Signal end of event
		EndMenuEvent( U_MOVEMENT_MENU );
		pUIEvent->uiParams[1] = TRUE;

	}

}


static void GetArrowsBackground(void);


static void HandleUpDownArrowBackgrounds(void)
{
	static	UINT32						 uiOldShowUpDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

	// Check for change in mode
	if ( guiShowUPDownArrows != uiOldShowUpDownArrows || gfUIRefreshArrows )
	{
		gfUIRefreshArrows = FALSE;

		// Hide position of new ones
		GetArrowsBackground( );

		uiOldShowUpDownArrows = guiShowUPDownArrows;

	}

}

void RenderArrows( )
{
	TILE_ELEMENT							 TileElem;

	if ( guiShowUPDownArrows & ARROWS_HIDE_UP && guiShowUPDownArrows & ARROWS_HIDE_DOWN )
	{
		return;
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_BESIDE )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS3 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_G )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS1 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_Y )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS3 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YG )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS3 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
			TileElem = gTileDatabase[ SECONDPOINTERS1 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_GG )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS1 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YY )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS3 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS8 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB2 )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS3 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
			TileElem = gTileDatabase[ SECONDPOINTERS8 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB3 )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS3 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
			TileElem = gTileDatabase[ SECONDPOINTERS8 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 40);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BESIDE )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS4 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_G )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS2 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_Y )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS4 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_CLIMB )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS7 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YG )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS2 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
			TileElem = gTileDatabase[ SECONDPOINTERS4 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_GG )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS2 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YY )
	{
			TileElem = gTileDatabase[ SECONDPOINTERS4 ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY + 20);
	}


}

void EraseRenderArrows( )
{
	if (giUpArrowRect != NO_BGND_RECT) FreeBackgroundRect(giUpArrowRect);
	giUpArrowRect = NO_BGND_RECT;

	if (giDownArrowRect != NO_BGND_RECT) FreeBackgroundRect(giDownArrowRect);
	giDownArrowRect = NO_BGND_RECT;
}


static void GetArrowsBackground(void)
{
	INT16											 sMercScreenX, sMercScreenY;
	UINT16										 sArrowHeight = ARROWS_HEIGHT, sArrowWidth = ARROWS_WIDTH;

	if ( guiShowUPDownArrows & ARROWS_HIDE_UP && guiShowUPDownArrows & ARROWS_HIDE_DOWN )
	{
		return;
	}

	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return;

	// Get screen position of our guy
	GetSoldierTRUEScreenPos(sel, &sMercScreenX, &sMercScreenY);

	if (guiShowUPDownArrows & ARROWS_SHOW_UP_BESIDE)
	{
		// Setup blt rect
		gsUpArrowX = sMercScreenX + ARROWS_X_OFFSET;
		gsUpArrowY = sMercScreenY + UPARROW_Y_OFFSET;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_G || guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_Y)
	{
		// Setup blt rect
		gsUpArrowX = sMercScreenX - 10;
		gsUpArrowY = sMercScreenY - 50;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YG || guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_GG || guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YY)
	{
		// Setup blt rect
		gsUpArrowX = sMercScreenX - 10;
		gsUpArrowY = sMercScreenY - 70;
		sArrowHeight = 3 * ARROWS_HEIGHT;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB)
	{
		// Setup blt rect
		gsUpArrowX = sMercScreenX - 10;
		gsUpArrowY = sMercScreenY - 70;
		sArrowHeight = 2 * ARROWS_HEIGHT;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB2)
	{
		// Setup blt rect
		gsUpArrowX = sMercScreenX - 10;
		gsUpArrowY = sMercScreenY - 80;
		sArrowHeight = 3 * ARROWS_HEIGHT;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB3)
	{
		// Setup blt rect
		gsUpArrowX = sMercScreenX - 10;
		gsUpArrowY = sMercScreenY - 900;
		sArrowHeight = 5 * ARROWS_HEIGHT;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BESIDE)
	{
		gsDownArrowX = sMercScreenX + ARROWS_X_OFFSET;
		gsDownArrowY = sMercScreenY + DOWNARROW_Y_OFFSET;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_Y || guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_G)
	{
		gsDownArrowX = sMercScreenX -10;
		gsDownArrowY = sMercScreenY + 10;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_CLIMB)
	{
		gsDownArrowX = sMercScreenX - 10;
		gsDownArrowY = sMercScreenY + 10;
		sArrowHeight = 3 * ARROWS_HEIGHT;
	}

	if (guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YG || guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_GG || guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YY)
	{
		gsDownArrowX = sMercScreenX -10;
		gsDownArrowY = sMercScreenY + 10;
		sArrowHeight = 3 * ARROWS_HEIGHT;
	}

	// Adjust arrows based on level
	if (gsInterfaceLevel == I_ROOF_LEVEL)
	{
	//	gsDownArrowY -= ROOF_LEVEL_HEIGHT;
	//	gsUpArrowY	 -= ROOF_LEVEL_HEIGHT;
	}

	//Erase prevois ones...
	EraseRenderArrows();

	// Register dirty rects
	giDownArrowRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, gsDownArrowX, gsDownArrowY, gsDownArrowX + sArrowWidth, gsDownArrowY + sArrowHeight);
	giUpArrowRect   = RegisterBackgroundRect(BGND_FLAG_PERMANENT, gsUpArrowX,   gsUpArrowY,   gsUpArrowX   + sArrowWidth, gsUpArrowY   + sArrowHeight);
}


void GetSoldierAboveGuyPositions(const SOLDIERTYPE* const pSoldier, INT16* const psX, INT16* const psY, const BOOLEAN fRadio)
{
	INT16 sMercScreenX, sMercScreenY;
	INT16 sOffsetX, sOffsetY;
	UINT8	ubAnimUseHeight;
	INT16		sStanceOffset = 0;
	INT16		sTextBodyTypeYOffset = 62;

	// Find XY, dims, offsets
	GetSoldierScreenPos( pSoldier, &sMercScreenX, &sMercScreenY );
	GetSoldierAnimOffsets( pSoldier, &sOffsetX, &sOffsetY );

	// OK, first thing to do is subtract offsets ( because GetSoldierScreenPos adds them... )
	sMercScreenX -= sOffsetX;
	sMercScreenY -= sOffsetY;

	// Adjust based on stance
	if ( ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_NOMOVE_MARKER) )
	{
		ubAnimUseHeight = gAnimControl[ pSoldier->usAnimState ].ubHeight;
	}
	else
	{
		ubAnimUseHeight = gAnimControl[ pSoldier->usAnimState ].ubEndHeight;
	}
	switch ( ubAnimUseHeight )
	{
		case ANIM_STAND:
			break;

		case ANIM_PRONE:
			sStanceOffset = 25;
			break;

		case ANIM_CROUCH:
			sStanceOffset = 10;
			break;
	}

	// Adjust based on body type...
	switch( pSoldier->ubBodyType )
	{
		case CROW:

			sStanceOffset = 30;
			break;

		case ROBOTNOWEAPON:

			sStanceOffset = 30;
			break;

	}

	//sStanceOffset -= gpWorldLevelData[ pSoldier->sGridNo ].sHeight;

	// Adjust based on level
	if ( pSoldier->bLevel == 1 && gsInterfaceLevel == 0 )
	{
		//sStanceOffset -= ROOF_LEVEL_HEIGHT;
	}
	if ( pSoldier->bLevel == 0 && gsInterfaceLevel == 1 )
	{
		//sStanceOffset += ROOF_LEVEL_HEIGHT;
	}

	if ( pSoldier->ubProfile != NO_PROFILE )
	{
		if ( fRadio )
		{
			*psX = sMercScreenX - 80 / 2;
			*psY = sMercScreenY - sTextBodyTypeYOffset + sStanceOffset;
		}
		else
		{
			*psX = sMercScreenX - 80 / 2;
			*psY = sMercScreenY - sTextBodyTypeYOffset + sStanceOffset;

			// OK, Check if we need to go below....
			// Can do this 1) if displaying damge or 2 ) above screen

			// If not a radio position, adjust if we are getting hit, to be lower!
			// If we are getting hit, lower them!
			if ( pSoldier->fDisplayDamage || *psY < gsVIEWPORT_WINDOW_START_Y )
			{
				*psX = sMercScreenX - 80 / 2;
				*psY = sMercScreenY;
			}
		}

	}
	else
	{
		//Display Text!
		*psX = sMercScreenX - 80 / 2;
		*psY = sMercScreenY - sTextBodyTypeYOffset + sStanceOffset;
	}
}


static void DrawBarsInUIBox(const SOLDIERTYPE* pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight);


void DrawSelectedUIAboveGuy(SOLDIERTYPE* const pSoldier)
{
	INT16 sXPos, sYPos;
	INT16 sX, sY;
	TILE_ELEMENT							 TileElem;
	const wchar_t *pStr;
	wchar_t										 NameStr[ 50 ];
	UINT16										 usGraphicToUse = THIRDPOINTERS1;
  BOOLEAN                    fRaiseName = FALSE;
  BOOLEAN                    fDoName = TRUE;

	if ( pSoldier->bVisible == -1 && !(gTacticalStatus.uiFlags&SHOW_ALL_MERCS) )
	{
		return;
	}

  if ( pSoldier->sGridNo == NOWHERE )
  {
    return;
  }

	if ( pSoldier->fFlashLocator )
	{
		if ( pSoldier->bVisible == -1 )
		{
			pSoldier->fFlashLocator = FALSE;
		}
		else
		{
			if ( TIMECOUNTERDONE( pSoldier->BlinkSelCounter, 80 ) )
			{
				RESETTIMECOUNTER( pSoldier->BlinkSelCounter, 80 );

			//	pSoldier->fShowLocator = !pSoldier->fShowLocator;

				pSoldier->fShowLocator = TRUE;

				// Update frame
				pSoldier->sLocatorFrame++;

				if ( pSoldier->sLocatorFrame == 5 )
				{
					// Update time we do this
					pSoldier->fFlashLocator++;
					pSoldier->sLocatorFrame = 0;
				}
			}

			if ( pSoldier->fFlashLocator == pSoldier->ubNumLocateCycles )
			{
					pSoldier->fFlashLocator = FALSE;
					pSoldier->fShowLocator = FALSE;
			}

			//if ( pSoldier->fShowLocator )
			{
				// Render the beastie
				GetSoldierAboveGuyPositions( pSoldier, &sXPos, &sYPos, TRUE );

				// Adjust for bars!
				sXPos += 25;
				sYPos += 25;

				//sXPos += 15;
				//sYPos += 21;


				// Add bars
				const INT32 iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, sXPos, sYPos, sXPos + 40, sYPos + 40);
				if (iBack != NO_BGND_RECT) SetBackgroundRectFilled(iBack);

				if ( ( !pSoldier->bNeutral && ( pSoldier->bSide != gbPlayerNum ) ) )
				{
					BltVideoObject(FRAME_BUFFER, guiRADIO2, pSoldier->sLocatorFrame, sXPos, sYPos);
				}
				else
				{
					BltVideoObject(FRAME_BUFFER, guiRADIO, pSoldier->sLocatorFrame, sXPos, sYPos);
				}
			}
		}
		//return;
	}

	const SOLDIERTYPE* const sel = GetSelectedMan();
	if ( !pSoldier->fShowLocator )
	{
		// RETURN IF MERC IS NOT SELECTED
		if (pSoldier == gSelectedGuy && pSoldier != sel && !gfIgnoreOnSelectedGuy)
		{

		}
		else if (pSoldier == sel && !gRubberBandActive)
		{
			usGraphicToUse = THIRDPOINTERS2;
		}
		else if ( pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED )
		{

		}
		else
		{
			return;
		}
	}
	else
	{
		if (pSoldier == sel && !gRubberBandActive)
		{
			usGraphicToUse = THIRDPOINTERS2;
		}
	}


	// If he is in the middle of a certain animation, ignore!
	if ( gAnimControl[ pSoldier->usAnimState ].uiFlags & ANIM_NOSHOW_MARKER )
	{
		return;
	}

	// Donot show if we are dead
	if ( ( pSoldier->uiStatusFlags & SOLDIER_DEAD ) )
	{
		return;
	}


	GetSoldierAboveGuyPositions( pSoldier, &sXPos, &sYPos, FALSE );


	// Display name
	SetFont( TINYFONT1 );
	SetFontBackground( FONT_MCOLOR_BLACK );
	SetFontForeground( FONT_MCOLOR_WHITE );

	if ( pSoldier->ubProfile != NO_PROFILE || ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
	{
		if (gfUIMouseOnValidCatcher == 1 && pSoldier == gUIValidCatcher)
		{
			const wchar_t* Catch = TacticalStr[CATCH_STR];
			FindFontCenterCoordinates(sXPos, sYPos, 80, 1, Catch, TINYFONT1, &sX, &sY);
			gprintfdirty(sX, sY, Catch);
			mprintf(sX, sY, Catch);
      fRaiseName = TRUE;
		}
		else if (gfUIMouseOnValidCatcher == 3 && pSoldier == gUIValidCatcher)
		{
			const wchar_t* Reload = TacticalStr[RELOAD_STR];
			FindFontCenterCoordinates(sXPos, sYPos, 80, 1, Reload, TINYFONT1, &sX, &sY);
			gprintfdirty(sX, sY, Reload);
			mprintf(sX, sY, Reload);
      fRaiseName = TRUE;
		}
		else if (gfUIMouseOnValidCatcher == 4 && pSoldier == gUIValidCatcher)
		{
			const wchar_t* Pass = pMessageStrings[MSG_PASS];
			FindFontCenterCoordinates(sXPos, sYPos, 80, 1, Pass, TINYFONT1, &sX, &sY);
			gprintfdirty(sX, sY, Pass);
			mprintf(sX, sY, Pass);
      fRaiseName = TRUE;
		}
    else if ( pSoldier->bAssignment >= ON_DUTY )
    {
			SetFontForeground( FONT_YELLOW );
			swprintf( NameStr, lengthof(NameStr), L"(%ls)", pAssignmentStrings[ pSoldier->bAssignment ] );
			FindFontCenterCoordinates(sXPos, sYPos, 80, 1, NameStr, TINYFONT1, &sX, &sY);
			gprintfdirty( sX, sY, NameStr );
			mprintf( sX, sY, NameStr );
      fRaiseName = TRUE;
    }
    else if ( pSoldier->bTeam == gbPlayerNum &&  pSoldier->bAssignment < ON_DUTY && pSoldier->bAssignment != CurrentSquad() && !(  pSoldier->uiStatusFlags & SOLDIER_MULTI_SELECTED ) )
    {
			swprintf( NameStr, lengthof(NameStr), gzLateLocalizedString[ 34 ], ( pSoldier->bAssignment + 1 ) );
			FindFontCenterCoordinates(sXPos, sYPos, 80, 1, NameStr, TINYFONT1, &sX, &sY);
			gprintfdirty( sX, sY, NameStr );
			mprintf( sX, sY, NameStr );
      fRaiseName = TRUE;
    }


		// If not in a squad....
		if ( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			const VEHICLETYPE* const v = GetVehicle(pSoldier->bVehicleID);
			Assert(v != NULL);
			if (GetNumberInVehicle(v) == 0)
			{
				SetFontForeground( FONT_GRAY4 );
			}
		}
		else
		{
			if ( pSoldier->bAssignment >= ON_DUTY  )
			{
				SetFontForeground( FONT_YELLOW );
			}
		}

    if ( fDoName )
    {
			const wchar_t* Name = pSoldier->name;
			FindFontCenterCoordinates(sXPos, fRaiseName ? sYPos - 10 : sYPos, 80, 1, Name, TINYFONT1, &sX, &sY);
			gprintfdirty(sX, sY, Name);
			mprintf(sX, sY, Name);
    }

		if ( pSoldier->ubProfile < FIRST_RPC || RPC_RECRUITED( pSoldier ) || AM_AN_EPC( pSoldier ) || ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			// Adjust for bars!
			const SOLDIERTYPE* const sel = GetSelectedMan();
			if (pSoldier == sel)
			{
				sXPos += 28;
				sYPos += 5;
			}
			else
			{
				sXPos += 30;
				sYPos += 7;
			}

			// Add bars
			const INT32 iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, sXPos, sYPos, sXPos + 34, sYPos + 11);
			if (iBack != NO_BGND_RECT) SetBackgroundRectFilled(iBack);
			TileElem = gTileDatabase[ usGraphicToUse ];
			BltVideoObject(  FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, sXPos, sYPos);

			// Draw life, breath
			// Only do this when we are a vehicle but on our team
			if (pSoldier == sel)
			{
				DrawBarsInUIBox( pSoldier,  (INT16)(sXPos + 1), (INT16)(sYPos + 2), 16, 1 );
			}
			else
			{
				DrawBarsInUIBox( pSoldier,  (INT16)(sXPos ), (INT16)(sYPos ), 16, 1 );
			}
		}
		else
		{
			if (gfUIMouseOnValidCatcher == 2 && pSoldier == gUIValidCatcher)
			{
				SetFont( TINYFONT1 );
				SetFontBackground( FONT_MCOLOR_BLACK );
				SetFontForeground( FONT_MCOLOR_WHITE );

				const wchar_t* Give = TacticalStr[GIVE_STR];
				FindFontCenterCoordinates(sXPos, sYPos + 10, 80, 1, Give, TINYFONT1, &sX, &sY);
				gprintfdirty(sX, sY, Give);
				mprintf(sX, sY, Give);
			}
			else
			{
				SetFont( TINYFONT1 );
				SetFontBackground( FONT_MCOLOR_BLACK );
				SetFontForeground( FONT_MCOLOR_DKRED );


				pStr = GetSoldierHealthString( pSoldier );

				FindFontCenterCoordinates(sXPos, sYPos + 10, 80, 1, pStr, TINYFONT1, &sX, &sY);
				gprintfdirty( sX, sY, pStr );
				mprintf( sX, sY, pStr );
			}
		}
	}
	else
	{
    if ( pSoldier->bLevel != 0 )
    {
	    // Display name
	    SetFont( TINYFONT1 );
	    SetFontBackground( FONT_MCOLOR_BLACK );
	    SetFontForeground( FONT_YELLOW );

			const wchar_t* Roof = gzLateLocalizedString[15];
			FindFontCenterCoordinates(sXPos, sYPos + 10, 80, 1, Roof, TINYFONT1, &sX, &sY);
			gprintfdirty(sX, sY, Roof);
			mprintf(sX, sY, Roof);
    }

		pStr = GetSoldierHealthString( pSoldier );

		SetFont( TINYFONT1 );
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_DKRED );

		FindFontCenterCoordinates(sXPos, sYPos, 80, 1, pStr, TINYFONT1, &sX, &sY);
		gprintfdirty( sX, sY, pStr );
		mprintf( sX, sY, pStr );

	}

}


static void DrawBarsInUIBox(const SOLDIERTYPE* pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight)
{
	FLOAT											 dWidth, dPercentage;
	//UINT16										 usLineColor;

	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;
	UINT16										 usLineColor;
	INT8											 bBandage;

	// Draw breath points

	// Draw new size
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, gsVIEWPORT_WINDOW_START_Y, SCREEN_WIDTH, gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y);

	// get amt bandaged
	bBandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;



	// NOW DO BLEEDING
	if ( pSoldier->bBleeding )
	{
		dPercentage = (FLOAT)( pSoldier->bBleeding +  pSoldier->bLife + bBandage )/ (FLOAT)100;
		dWidth			=	dPercentage * sWidth;
		usLineColor = Get16BPPColor(FROMRGB(240, 240, 20));
		RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (INT32)(sXPos + dWidth + 3), sYPos + 1, usLineColor, pDestBuf);
	}

	if( bBandage )
	{
		dPercentage = (FLOAT)( pSoldier->bLife + bBandage ) / (FLOAT)100;
		dWidth			=	dPercentage * sWidth;
		usLineColor = Get16BPPColor( FROMRGB( 222, 132, 132 ) );
		RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (INT32)(sXPos + dWidth + 3), sYPos + 1, usLineColor, pDestBuf);
	}

	dPercentage = (FLOAT)pSoldier->bLife / (FLOAT)100;
	dWidth			=	dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(200, 0, 0));
	RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (INT32)(sXPos + dWidth + 3), sYPos + 1, usLineColor, pDestBuf);

	dPercentage = (FLOAT)( pSoldier->bBreathMax ) / (FLOAT)100;
	dWidth			=	dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(20, 20, 150));
	RectangleDraw(TRUE, sXPos + 3, sYPos + 4, (INT32)(sXPos + dWidth + 3), sYPos + 4, usLineColor, pDestBuf);

	dPercentage = (FLOAT)( pSoldier->bBreath ) / (FLOAT)100;
	dWidth			=	dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(100, 100, 220));
	RectangleDraw(TRUE, sXPos + 3, sYPos + 4, (INT32)(sXPos + dWidth + 3), sYPos + 4, usLineColor, pDestBuf);

	/*
	// morale
	dPercentage = (FLOAT)pSoldier->bMorale / (FLOAT)100;
	dWidth			=	dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(0, 250, 0));
	RectangleDraw(TRUE, sXPos + 1, sYPos + 7, (INT32)(sXPos + dWidth + 1), sYPos + 7, usLineColor, pDestBuf);
	*/

	UnLockVideoSurface( FRAME_BUFFER );

}


#ifdef JA2TESTVERSION
void EndDeadlockMsg(void)
{
	// Reset gridlock
	gUIDeadlockedSoldier = NOBODY;
}
#endif


void ClearInterface( )
{
	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		return;
	}

	// Stop any UI menus!
	if ( gCurrentUIMode == MENU_MODE )
	{
		EndMenuEvent( guiCurrentEvent );
	}

	if ( gfUIHandleShowMoveGrid )
	{
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS4	);
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS2 );
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS13 );
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS15 );
	}

	// Remove any popup menus
	if ( gCurrentUIMode == GETTINGITEM_MODE )
	{
		RemoveItemPickupMenu( );
	}

	// Remove any popup menus
	if ( gCurrentUIMode == OPENDOOR_MENU_MODE )
	{
		PopDownOpenDoorMenu( );
	}

	// Remove UP/DOWN arrows...
	//EraseRenderArrows( );
	// Don't render arrows this frame!
	guiShowUPDownArrows	= ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

	ResetPhysicsTrajectoryUI( );

	// Remove any paths, cursors
	ErasePath( FALSE );

	//gfPlotNewMovement = TRUE;

	// Erase Interface cursors
	HideUICursor( );

	MSYS_ChangeRegionCursor( &gViewportRegion , VIDEO_NO_CURSOR );

	// Hide lock UI cursors...
	MSYS_ChangeRegionCursor( &gDisableRegion , VIDEO_NO_CURSOR );
	MSYS_ChangeRegionCursor( &gUserTurnRegion , VIDEO_NO_CURSOR );

	// Remove special thing for south arrow...
	if (gsGlobalCursorYOffset == SCREEN_HEIGHT - gsVIEWPORT_WINDOW_END_Y)
	{
		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
	}

}

void RestoreInterface( )
{
	// Once we are done, plot path again!
	gfPlotNewMovement = TRUE;

	// OK, reset arrows too...
	gfUIRefreshArrows = TRUE;

	// SHow lock UI cursors...
	MSYS_ChangeRegionCursor( &gDisableRegion , CURSOR_WAIT );
	MSYS_ChangeRegionCursor( &gUserTurnRegion , CURSOR_WAIT );

}


void DirtyMercPanelInterface( SOLDIERTYPE *pSoldier, UINT8 ubDirtyLevel )
{
	if ( pSoldier->bTeam == gbPlayerNum )
	{
		// ONly set to a higher level!
		if ( fInterfacePanelDirty < ubDirtyLevel )
		{
			fInterfacePanelDirty = ubDirtyLevel;
		}
	}

}

typedef struct
{
	SOLDIERTYPE *pSoldier;
	INT16				sX;
	INT16				sY;
	BOOLEAN			fMenuHandled;
	BOOLEAN			fClosingDoor;
} OPENDOOR_MENU;

static OPENDOOR_MENU gOpenDoorMenu;
BOOLEAN				gfInOpenDoorMenu = FALSE;


static void PopupDoorOpenMenu(BOOLEAN fClosingDoor);


BOOLEAN InitDoorOpenMenu(SOLDIERTYPE* const pSoldier, const BOOLEAN fClosingDoor)
{
	INT16 sHeight, sWidth;
	INT16	sScreenX, sScreenY;

	// Erase other menus....
	EraseInterfaceMenus( TRUE );

	InterruptTime();
	PauseGame();
	LockPauseState( 19 );
	// Pause timers as well....
	PauseTime( TRUE );


	gOpenDoorMenu.pSoldier		= pSoldier;
	gOpenDoorMenu.fClosingDoor	= fClosingDoor;

	// OK, Determine position...
	// Center on guy
	// Locate to guy first.....
	LocateSoldier(pSoldier, FALSE);
	GetSoldierAnimDims( pSoldier, &sHeight, &sWidth );
	GetSoldierScreenPos( pSoldier, &sScreenX, &sScreenY );
	gOpenDoorMenu.sX = sScreenX - ( ( BUTTON_PANEL_WIDTH - sWidth ) / 2 );
	gOpenDoorMenu.sY = sScreenY - ( ( BUTTON_PANEL_HEIGHT - sHeight ) / 2 );

	// Alrighty, cancel lock UI if we havn't done so already
	UnSetUIBusy(pSoldier);


	// OK, CHECK FOR BOUNDARIES!
	if (gOpenDoorMenu.sX + BUTTON_PANEL_WIDTH > SCREEN_WIDTH)
	{
		gOpenDoorMenu.sX = SCREEN_WIDTH - BUTTON_PANEL_WIDTH;
	}
	if ( ( gOpenDoorMenu.sY + BUTTON_PANEL_HEIGHT ) > gsVIEWPORT_WINDOW_END_Y )
	{
		gOpenDoorMenu.sY = ( gsVIEWPORT_WINDOW_END_Y - BUTTON_PANEL_HEIGHT );
	}
	if ( gOpenDoorMenu.sX < 0 )
	{
		gOpenDoorMenu.sX = 0;
	}
	if ( gOpenDoorMenu.sY < 0 )
	{
		gOpenDoorMenu.sY = 0;
	}


	gOpenDoorMenu.fMenuHandled	= FALSE;

	guiPendingOverrideEvent		= OP_OPENDOORMENU;
	HandleTacticalUI( );

	PopupDoorOpenMenu( fClosingDoor );

	return( TRUE );
}


static void BtnDoorMenuCallback(GUI_BUTTON* btn, INT32 reason);


static BOOLEAN MakeButtonDoor(UINT idx, UINT gfx, INT16 x, INT16 y, INT16 ap, INT16 bp, BOOLEAN disable, const wchar_t* help)
{
	INT32 btn = QuickCreateButton(iIconImages[gfx], x, y, MSYS_PRIORITY_HIGHEST - 1, BtnDoorMenuCallback);
	iActionIcons[idx] = btn;
	if (btn == -1)
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Cannot create Interface button");
		return FALSE;
	}
	if (ap == 0 || !(gTacticalStatus.uiFlags & TURNBASED) || !(gTacticalStatus.uiFlags & INCOMBAT))
	{
		SetButtonFastHelpText(btn, help);
	}
	else
	{
		wchar_t zDisp[100];
		swprintf(zDisp, lengthof(zDisp), L"%ls ( %d )", help, ap);
		SetButtonFastHelpText(btn, zDisp);
	}
	if (disable ||
			(ap != 0 && !EnoughPoints(gOpenDoorMenu.pSoldier, ap, bp, FALSE)))
	{
		DisableButton(btn);
	}
	return TRUE;
}


static void DoorMenuBackregionCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void PopupDoorOpenMenu(BOOLEAN fClosingDoor)
{
	INT32 dx = gOpenDoorMenu.sX;
	INT32 dy = gOpenDoorMenu.sY;

	dx += 9;
	dy += 8;

	// Create mouse region over all area to facilitate clicking to end
	MSYS_DefineRegion(&gMenuOverlayRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 1, CURSOR_NORMAL, MSYS_NO_CALLBACK, DoorMenuBackregionCallback);

	const BOOLEAN d0 = fClosingDoor || AM_AN_EPC(gOpenDoorMenu.pSoldier);
	BOOLEAN d;

	d = d0 || !SoldierHasKey(gOpenDoorMenu.pSoldier, ANYKEY);
	if (!MakeButtonDoor(USE_KEYRING_ICON, USE_KEYRING_IMAGES, dx + 20, dy, AP_UNLOCK_DOOR, BP_UNLOCK_DOOR, d, pTacticalPopupButtonStrings[USE_KEYRING_ICON])) return;

	d = fClosingDoor || FindUsableObj(gOpenDoorMenu.pSoldier, CROWBAR) == NO_SLOT;
	if (!MakeButtonDoor(USE_CROWBAR_ICON, CROWBAR_DOOR_IMAGES, dx + 40, dy, AP_USE_CROWBAR, BP_USE_CROWBAR, d, pTacticalPopupButtonStrings[USE_CROWBAR_ICON])) return;

	d = d0 || FindObj(gOpenDoorMenu.pSoldier, LOCKSMITHKIT) == NO_SLOT;
	if (!MakeButtonDoor(LOCKPICK_DOOR_ICON, LOCKPICK_DOOR_IMAGES, dx + 40, dy + 20, AP_PICKLOCK, BP_PICKLOCK, d, pTacticalPopupButtonStrings[LOCKPICK_DOOR_ICON])) return;

	d = d0 || FindObj(gOpenDoorMenu.pSoldier, SHAPED_CHARGE) == NO_SLOT;
	if (!MakeButtonDoor(EXPLOSIVE_DOOR_ICON, EXPLOSIVE_DOOR_IMAGES, dx + 40, dy + 40, AP_EXPLODE_DOOR, BP_EXPLODE_DOOR, d, pTacticalPopupButtonStrings[EXPLOSIVE_DOOR_ICON])) return;

	const wchar_t* const help = pTacticalPopupButtonStrings[fClosingDoor ? CANCEL_ICON + 1 : OPEN_DOOR_ICON];
	if (!MakeButtonDoor(OPEN_DOOR_ICON, OPEN_DOOR_IMAGES, dx, dy, AP_OPEN_DOOR, BP_OPEN_DOOR, FALSE, help)) return;

	if (!MakeButtonDoor(EXAMINE_DOOR_ICON, EXAMINE_DOOR_IMAGES, dx,      dy + 20, AP_EXAMINE_DOOR, BP_EXAMINE_DOOR, d0, pTacticalPopupButtonStrings[EXAMINE_DOOR_ICON])) return;
	if (!MakeButtonDoor(BOOT_DOOR_ICON, BOOT_DOOR_IMAGES,       dx,      dy + 40, AP_BOOT_DOOR,    BP_BOOT_DOOR,    d0, pTacticalPopupButtonStrings[BOOT_DOOR_ICON]))    return;
	if (!MakeButtonDoor(UNTRAP_DOOR_ICON, UNTRAP_DOOR_ICON,     dx + 20, dy + 40, AP_UNTRAP_DOOR,  BP_UNTRAP_DOOR,  d0, pTacticalPopupButtonStrings[UNTRAP_DOOR_ICON]))  return;

	if (!MakeButtonDoor(CANCEL_ICON, CANCEL_IMAGES, dx + 20, dy + 20, 0, 0, FALSE, pTacticalPopupButtonStrings[CANCEL_ICON])) return;

	gfInOpenDoorMenu = TRUE;

	// Ignore scrolling
	gfIgnoreScrolling = TRUE;
}


void PopDownOpenDoorMenu( )
{
	if ( gfInOpenDoorMenu )
	{
		UnLockPauseState();
		UnPauseGame();
		// UnPause timers as well....
		PauseTime( FALSE );

		RemoveButton( iActionIcons[ USE_KEYRING_ICON  ] );
		RemoveButton( iActionIcons[ USE_CROWBAR_ICON  ] );
		RemoveButton( iActionIcons[ LOCKPICK_DOOR_ICON  ] );
		RemoveButton( iActionIcons[ EXPLOSIVE_DOOR_ICON  ] );
		RemoveButton( iActionIcons[ OPEN_DOOR_ICON  ] );
		RemoveButton( iActionIcons[ EXAMINE_DOOR_ICON  ] );
		RemoveButton( iActionIcons[ BOOT_DOOR_ICON  ] );
		RemoveButton( iActionIcons[ UNTRAP_DOOR_ICON  ] );
		RemoveButton( iActionIcons[ CANCEL_ICON  ] );

		// Turn off Ignore scrolling
		gfIgnoreScrolling = FALSE;

		// Rerender world
		SetRenderFlags( RENDER_FLAG_FULL );

		fInterfacePanelDirty = DIRTYLEVEL2;

		MSYS_RemoveRegion( &gMenuOverlayRegion);
	}

	gfInOpenDoorMenu = FALSE;

}

void RenderOpenDoorMenu( )
{
	if ( gfInOpenDoorMenu )
	{
		BltVideoObject(FRAME_BUFFER, guiBUTTONBORDER, 0, gOpenDoorMenu.sX, gOpenDoorMenu.sY);

		// Mark buttons dirty!
		MarkAButtonDirty( iActionIcons[ USE_KEYRING_ICON  ] );
		MarkAButtonDirty( iActionIcons[ USE_CROWBAR_ICON  ] );
		MarkAButtonDirty( iActionIcons[ LOCKPICK_DOOR_ICON  ] );
		MarkAButtonDirty( iActionIcons[ EXPLOSIVE_DOOR_ICON  ] );
		MarkAButtonDirty( iActionIcons[ OPEN_DOOR_ICON  ] );
		MarkAButtonDirty( iActionIcons[ EXAMINE_DOOR_ICON  ] );
		MarkAButtonDirty( iActionIcons[ BOOT_DOOR_ICON  ] );
		MarkAButtonDirty( iActionIcons[ UNTRAP_DOOR_ICON  ] );
		MarkAButtonDirty( iActionIcons[ CANCEL_ICON  ] );

		RenderButtons( );

		// if game is paused, then render paused game text
		RenderPausedGameBox( );

		InvalidateRegion( gOpenDoorMenu.sX, gOpenDoorMenu.sY, gOpenDoorMenu.sX + BUTTON_PANEL_WIDTH, gOpenDoorMenu.sY + BUTTON_PANEL_HEIGHT );

	}
}

void CancelOpenDoorMenu( )
{
	// Signal end of event
	gOpenDoorMenu.fMenuHandled = 2;
}


static void DoorAction(const INT16 ap, const INT16 bp, const UINT8 action)
{
	SOLDIERTYPE* const s = gOpenDoorMenu.pSoldier;
	if (EnoughPoints(s, ap, bp, FALSE))
	{
		SetUIBusy(s);
		InteractWithClosedDoor(s, action);
	}
	else
	{
		// set cancel code
		gOpenDoorMenu.fMenuHandled = 2;
	}
}


static void BtnDoorMenuCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;

		const INT32 uiBtnID = btn->IDNum;

		// Popdown menu
		gOpenDoorMenu.fMenuHandled = TRUE;

		if (uiBtnID == iActionIcons[CANCEL_ICON])
		{
			// OK, set cancle code!
			gOpenDoorMenu.fMenuHandled = 2;
		}
		else if (uiBtnID == iActionIcons[OPEN_DOOR_ICON])
		{
			// Open door normally...
			// Check APs
			if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_OPEN_DOOR, BP_OPEN_DOOR, FALSE))
			{
				// Set UI
				SetUIBusy(gOpenDoorMenu.pSoldier);

				if (gOpenDoorMenu.fClosingDoor)
				{
					ChangeSoldierState(gOpenDoorMenu.pSoldier, GetAnimStateForInteraction(gOpenDoorMenu.pSoldier, TRUE, CLOSE_DOOR), 0, FALSE);
				}
				else
				{
					InteractWithClosedDoor(gOpenDoorMenu.pSoldier, HANDLE_DOOR_OPEN);
				}
			}
			else
			{
				// OK, set cancle code!
				gOpenDoorMenu.fMenuHandled = 2;
			}
		}
		else if (uiBtnID == iActionIcons[BOOT_DOOR_ICON])
		{
			DoorAction(AP_BOOT_DOOR, BP_BOOT_DOOR, HANDLE_DOOR_FORCE);
		}
		else if (uiBtnID == iActionIcons[USE_KEYRING_ICON])
		{
			DoorAction(AP_UNLOCK_DOOR, BP_UNLOCK_DOOR, HANDLE_DOOR_UNLOCK);
		}
		else if (uiBtnID == iActionIcons[LOCKPICK_DOOR_ICON])
		{
			DoorAction(AP_PICKLOCK, BP_PICKLOCK, HANDLE_DOOR_LOCKPICK);
		}
		else if (uiBtnID == iActionIcons[EXAMINE_DOOR_ICON])
		{
			DoorAction(AP_EXAMINE_DOOR, BP_EXAMINE_DOOR, HANDLE_DOOR_EXAMINE);
		}
		else if (uiBtnID == iActionIcons[EXPLOSIVE_DOOR_ICON])
		{
			DoorAction(AP_EXPLODE_DOOR, BP_EXPLODE_DOOR, HANDLE_DOOR_EXPLODE);
		}
		else if (uiBtnID == iActionIcons[UNTRAP_DOOR_ICON])
		{
			DoorAction(AP_UNTRAP_DOOR, BP_UNTRAP_DOOR, HANDLE_DOOR_UNTRAP);
		}
		else if (uiBtnID == iActionIcons[USE_CROWBAR_ICON])
		{
			DoorAction(AP_USE_CROWBAR, BP_USE_CROWBAR, HANDLE_DOOR_CROWBAR);
		}

		HandleOpenDoorMenu();
	}
}


BOOLEAN HandleOpenDoorMenu( )
{
	if ( gOpenDoorMenu.fMenuHandled )
	{
		PopDownOpenDoorMenu( );
		return( gOpenDoorMenu.fMenuHandled );
	}

	return( FALSE );
}


static void RenderUIMessage(VIDEO_OVERLAY* pBlitter)
{
	// Shade area first...
	ShadowVideoSurfaceRect( pBlitter->uiDestBuff, pBlitter->sX, pBlitter->sY, pBlitter->sX + gusUIMessageWidth - 2, pBlitter->sY + gusUIMessageHeight - 2 );

	RenderMercPopUpBoxFromIndex( iUIMessageBox, pBlitter->sX, pBlitter->sY,  pBlitter->uiDestBuff );

	InvalidateRegion( pBlitter->sX, pBlitter->sY, pBlitter->sX + gusUIMessageWidth, pBlitter->sY + gusUIMessageHeight );
}


static UINT32 CalcUIMessageDuration(const wchar_t* wString);


void BeginUIMessage(BOOLEAN fUseSkullIcon, const wchar_t* text)
{
	VIDEO_OVERLAY_DESC		VideoOverlayDesc;

	guiUIMessageTime = GetJA2Clock( );
	guiUIMessageTimeDelay = CalcUIMessageDuration(text);

	// Override it!
	OverrideMercPopupBox( &gpUIMessageOverrideMercBox );

  //SetPrepareMercPopupFlags( MERC_POPUP_PREPARE_FLAGS_TRANS_BACK | MERC_POPUP_PREPARE_FLAGS_MARGINS );

	if ( fUseSkullIcon )
	{
		SetPrepareMercPopupFlags( MERC_POPUP_PREPARE_FLAGS_MARGINS | MERC_POPUP_PREPARE_FLAGS_SKULLICON );
	}
	else
	{
		SetPrepareMercPopupFlags( MERC_POPUP_PREPARE_FLAGS_MARGINS | MERC_POPUP_PREPARE_FLAGS_STOPICON );
	}

	// Prepare text box
	iUIMessageBox = PrepareMercPopupBox(iUIMessageBox, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, text, 200, 10, 0, 0, &gusUIMessageWidth, &gusUIMessageHeight);

	// Set it back!
	ResetOverrideMercPopupBox( );

	if (g_ui_message_overlay != NULL)
	{
		RemoveVideoOverlay(g_ui_message_overlay);
		g_ui_message_overlay = NULL;
	}

	if (g_ui_message_overlay == NULL)
	{
		memset( &VideoOverlayDesc, 0, sizeof( VideoOverlayDesc ) );

		// Set Overlay
		VideoOverlayDesc.sLeft       = (SCREEN_WIDTH - gusUIMessageWidth) / 2;
		VideoOverlayDesc.sTop        = 150;
		VideoOverlayDesc.sRight      = VideoOverlayDesc.sLeft + gusUIMessageWidth;
		VideoOverlayDesc.sBottom     = VideoOverlayDesc.sTop + gusUIMessageHeight;
		VideoOverlayDesc.BltCallback = RenderUIMessage;
		g_ui_message_overlay = RegisterVideoOverlay(0, &VideoOverlayDesc);
	}

	gfUseSkullIconMessage = fUseSkullIcon;
}


void BeginMapUIMessage(INT16 delta_y, const wchar_t* text)
{
	guiUIMessageTime      = GetJA2Clock();
	guiUIMessageTimeDelay = CalcUIMessageDuration(text);

	OverrideMercPopupBox(&gpUIMessageOverrideMercBox);
	SetPrepareMercPopupFlags(MERC_POPUP_PREPARE_FLAGS_TRANS_BACK | MERC_POPUP_PREPARE_FLAGS_MARGINS);
	iUIMessageBox = PrepareMercPopupBox(iUIMessageBox, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, text, 200, 10, 0, 0, &gusUIMessageWidth, &gusUIMessageHeight);
	ResetOverrideMercPopupBox();

	if (g_ui_message_overlay == NULL)
	{
		VIDEO_OVERLAY_DESC VideoOverlayDesc;
		memset(&VideoOverlayDesc, 0, sizeof(VideoOverlayDesc));
		VideoOverlayDesc.sLeft       = MAP_VIEW_START_X + (MAP_VIEW_WIDTH  - gusUIMessageWidth)  / 2 + 20;
		VideoOverlayDesc.sTop        = MAP_VIEW_START_Y + (MAP_VIEW_HEIGHT - gusUIMessageHeight) / 2 + delta_y;
		VideoOverlayDesc.sRight      = VideoOverlayDesc.sLeft + gusUIMessageWidth;
		VideoOverlayDesc.sBottom     = VideoOverlayDesc.sTop  + gusUIMessageHeight;
		VideoOverlayDesc.BltCallback = RenderUIMessage;
		g_ui_message_overlay = RegisterVideoOverlay(0, &VideoOverlayDesc);
	}
}


void EndUIMessage( )
{
	UINT32	uiClock = GetJA2Clock();

	if (g_ui_message_overlay != NULL)
	{
		if ( gfUseSkullIconMessage )
		{
			if ( ( uiClock - guiUIMessageTime ) < 300 )
			{
				return;
			}
		}

//		DebugMsg(TOPIC_JA2, DBG_LEVEL_0, "Removing Overlay message");

		RemoveVideoOverlay(g_ui_message_overlay);
		g_ui_message_overlay = NULL;

    // Remove popup as well....
    if ( iUIMessageBox != -1 )
    {
    	RemoveMercPopupBoxFromIndex( iUIMessageBox );
      iUIMessageBox = -1;
    }
	}
	//iUIMessageBox = -1;
}

#define PLAYER_TEAM_TIMER_INTTERUPT_GRACE		( 15000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS )
#define PLAYER_TEAM_TIMER_GRACE_PERIOD			1000
#define PLAYER_TEAM_TIMER_SEC_PER_TICKS			100
#define PLAYER_TEAM_TIMER_TICKS_PER_OK_MERC								( 15000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS )
#define PLAYER_TEAM_TIMER_TICKS_PER_NOTOK_MERC						( 5000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS )
#define PLAYER_TEAM_TIMER_TICKS_FROM_END_TO_START_BEEP		( 5000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS )
#define PLAYER_TEAM_TIMER_TIME_BETWEEN_BEEPS							( 500 )
#define PLAYER_TEAM_TIMER_TICKS_PER_ENEMY									( 2000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS )


static void CreateTopMessage(SGPVSurface* surface, UINT8 ubType, const wchar_t* psString);


BOOLEAN AddTopMessage( UINT8 ubType, const wchar_t *pzString )
{
	UINT32	cnt;
	BOOLEAN	fFound = FALSE;

	// Set time of last update
	gTopMessage.uiTimeOfLastUpdate = GetJA2Clock( );

	// Set flag to animate down...
	//gTopMessage.bAnimate = -1;
	//gTopMessage.bYPos		 = 2;

	gTopMessage.bAnimate = 0;
	gTopMessage.bYPos		 = 20;
	gTopMessage.fCreated = TRUE;

	fFound = TRUE;
	cnt = 0;

	if ( fFound )
	{
		gTopMessage.bCurrentMessage = (INT8)cnt;

		gTacticalStatus.ubTopMessageType = ubType;
		gTacticalStatus.fInTopMessage = TRUE;

		// Copy string
		wcscpy( gTacticalStatus.zTopMessageString, pzString );

		CreateTopMessage( gTopMessage.uiSurface, ubType, pzString );

		return( TRUE );
	}

	return( FALSE );
}


static void CreateTopMessage(SGPVSurface* const uiSurface, const UINT8 ubType, const wchar_t* const psString)
{
	INT16		sX, sY;
	INT32		cnt2;
	INT16		sBarX = 0;
	BOOLEAN	fDoLimitBar = FALSE;

	FLOAT		dNumStepsPerEnemy, dLength, dCurSize;

	SGPVObject* const uiBAR = AddVideoObjectFromFile("INTERFACE/rect.sti");
	AssertMsg(uiBAR != NO_VOBJECT, "Missing INTERFACE/rect.sti");

	SGPVObject* const uiPLAYERBAR = AddVideoObjectFromFile("INTERFACE/timebargreen.sti");
	AssertMsg(uiPLAYERBAR != NO_VOBJECT, "Missing INTERFACE/timebargreen.sti");

	SGPVObject* const uiINTBAR = AddVideoObjectFromFile("INTERFACE/timebaryellow.sti");
	AssertMsg(uiINTBAR != NO_VOBJECT, "Missing INTERFACE/timebaryellow.sti");

	// Change dest buffer
	SetFontDestBuffer(uiSurface, 0, 0, SCREEN_WIDTH, 20);
	SetFont( TINYFONT1 );

	const SGPVObject* uiBarToUseInUpDate = NO_VOBJECT;
	switch( ubType )
	{
		case COMPUTER_TURN_MESSAGE:
		case COMPUTER_INTERRUPT_MESSAGE:
		case MILITIA_INTERRUPT_MESSAGE:
		case AIR_RAID_TURN_MESSAGE:

			// Render rect into surface
			BltVideoObject(uiSurface, uiBAR, 0, 0, 0);
			SetFontBackground( FONT_MCOLOR_BLACK );
			SetFontForeground( FONT_MCOLOR_WHITE );
			uiBarToUseInUpDate = uiBAR;
			fDoLimitBar				 = TRUE;
			break;

		case PLAYER_INTERRUPT_MESSAGE:

			// Render rect into surface
			BltVideoObject(uiSurface, uiINTBAR, 0, 0, 0);
			SetFontBackground( FONT_MCOLOR_BLACK );
			SetFontForeground( FONT_MCOLOR_BLACK );
			SetFontShadow( NO_SHADOW );
			uiBarToUseInUpDate = uiINTBAR;
			break;

		case PLAYER_TURN_MESSAGE:

			// Render rect into surface
			//if ( gGameOptions.fTurnTimeLimit )
			{
				BltVideoObject(uiSurface, uiPLAYERBAR, 0, 0, 0);
			}
			//else
			//{
			//	BltVideoObject(uiSurface, uiPLAYERBAR, 13, 0, 0);
			//}
			SetFontBackground( FONT_MCOLOR_BLACK );
			SetFontForeground( FONT_MCOLOR_BLACK );
			SetFontShadow( NO_SHADOW );
			uiBarToUseInUpDate = uiPLAYERBAR;
			break;
	}

	// Update progress bar!
#if 0
	if ( ubType == COMPUTER_TURN_MESSAGE )
	{
		INT32		cnt1, iLength;
		INT16		usNumStepsPerEnemy;

		if ( gfProgBarActive )
		{
			//usNumStepsPerEnemy = ( PROG_BAR_LENGTH / gubProgNumEnemies );

			// Alrighty, do some fun stuff!

			// Render end peice
			sBarX = PROG_BAR_START_X;
			BltVideoObject(uiSurface, uiBAR, 3, sBarX, PROG_BAR_START_Y);

			// Determine Length
		//	iLength   = (gubProgCurEnemy ) * usNumStepsPerEnemy;

			cnt1 = 0;
			cnt2 = 0;

			while( cnt1 < iLength )
			{
				sBarX++;

				// Check sBarX, ( just as a precaution )
				if (sBarX > SCREEN_WIDTH)
				{
					break;
				}

				BltVideoObject(uiSurface, uiBAR, 4 + cnt2, sBarX, PROG_BAR_START_Y);

				cnt1++;
				cnt2++;

				if ( cnt2 == 10 )
				{
					cnt2 = 0;
				}
			}

			//gubProgNumEnemies = ubNum;
			//gubProgCurEnemy		= 0;


		}
	}
#endif

	if ( gGameOptions.fTurnTimeLimit )
	{
		if ( ubType == PLAYER_TURN_MESSAGE || ubType == PLAYER_INTERRUPT_MESSAGE )
		{
			fDoLimitBar = TRUE;
		}
	}

	if ( fDoLimitBar )
	{
		dNumStepsPerEnemy = (FLOAT)( (FLOAT)PROG_BAR_LENGTH / (FLOAT)gTacticalStatus.usTactialTurnLimitMax );

		// Alrighty, do some fun stuff!

		// Render end peice
		sBarX = PROG_BAR_START_X;
		BltVideoObject(uiSurface, uiBarToUseInUpDate, 1, sBarX, PROG_BAR_START_Y);

		// Determine Length
		dLength   = ( gTacticalStatus.usTactialTurnLimitCounter ) * dNumStepsPerEnemy;

		dCurSize = 0;
		cnt2 = 0;

		while( dCurSize < dLength )
		{
			sBarX++;

			// Check sBarX, ( just as a precaution )
			if (sBarX >= SCREEN_WIDTH)
			{
				break;
			}

			BltVideoObject(uiSurface, uiBarToUseInUpDate, 2 + cnt2, sBarX, PROG_BAR_START_Y);

			dCurSize++;
			cnt2++;

			if ( cnt2 == 10 )
			{
				cnt2 = 0;
			}
		}

		// Do end...
		if ( gTacticalStatus.usTactialTurnLimitCounter == gTacticalStatus.usTactialTurnLimitMax )
		{
			sBarX++;
			BltVideoObject(uiSurface, uiBarToUseInUpDate, 2 + cnt2, sBarX, PROG_BAR_START_Y);
			sBarX++;
			BltVideoObject(uiSurface, uiBarToUseInUpDate, 12,       sBarX, PROG_BAR_START_Y);
		}
	}

	// Delete rect
	DeleteVideoObject(uiBAR);
	DeleteVideoObject(uiINTBAR);

	//if ( gGameOptions.fTurnTimeLimit )
	{
		DeleteVideoObject(uiPLAYERBAR);
	}

	// Draw text....
	FindFontCenterCoordinates(0, 7, SCREEN_WIDTH, 1, psString, TINYFONT1, &sX, &sY);
	mprintf( sX, sY, psString );

	// Change back...
	SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Done!
	SetFontShadow( DEFAULT_SHADOW );


	gfTopMessageDirty = TRUE;

}


static void TurnExpiredCallBack(UINT8 bExitValue)
{
	// End turn...
	UIHandleEndTurn( NULL );
}


static void CheckForAndHandleEndPlayerTimeLimit(void)
{
	if ( gTacticalStatus.fInTopMessage )
	{
		if ( gGameOptions.fTurnTimeLimit )
		{
			if ( gTacticalStatus.ubTopMessageType == PLAYER_TURN_MESSAGE || gTacticalStatus.ubTopMessageType == PLAYER_INTERRUPT_MESSAGE )
			{
				 if ( gTacticalStatus.usTactialTurnLimitCounter == ( gTacticalStatus.usTactialTurnLimitMax - 1 ) )
				 {
						// ATE: increase this so that we don't go into here again...
						gTacticalStatus.usTactialTurnLimitCounter++;

						// OK, set message that time limit has expired....
						//DoMessageBox( MSG_BOX_BASIC_STYLE, L"Turn has Expired!", GAME_SCREEN, ( UINT8 )MSG_BOX_FLAG_OK, TurnExpiredCallBack, NULL );

						// End turn...
						UIHandleEndTurn( NULL );

				 }
			}
		}
	}
}


void HandleTopMessages( )
{
	//UINT32		uiTime;

	// OK, is out current count > 0 ?
		if ( gTacticalStatus.fInTopMessage )
	{
		//gfTopMessageDirty = TRUE;


		// ATE: If we are told to go into top message, but we have not
		// initialized it yet...
		// This is mostly for loading saved games.....
		if ( !gTopMessage.fCreated )
		{
			gfTopMessageDirty = TRUE;
			AddTopMessage( gTacticalStatus.ubTopMessageType, gTacticalStatus.zTopMessageString );
		}

		if ( gTacticalStatus.ubTopMessageType == COMPUTER_TURN_MESSAGE ||
				 gTacticalStatus.ubTopMessageType == COMPUTER_INTERRUPT_MESSAGE ||
				 gTacticalStatus.ubTopMessageType == MILITIA_INTERRUPT_MESSAGE ||
				 gTacticalStatus.ubTopMessageType == AIR_RAID_TURN_MESSAGE )
		{
			// OK, update timer.....
			if ( TIMECOUNTERDONE( giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS ) )
			{
				RESETTIMECOUNTER( giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS );

				// Update counter....
				if ( gTacticalStatus.usTactialTurnLimitCounter < gTacticalStatus.usTactialTurnLimitMax )
				{
					gTacticalStatus.usTactialTurnLimitCounter++;
				}

				// Check if we have reach limit...
				if ( gTacticalStatus.usTactialTurnLimitCounter >= ( ( gubProgCurEnemy  ) * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY ) )
				{
					gTacticalStatus.usTactialTurnLimitCounter = ( ( gubProgCurEnemy ) * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY );
				}

				CreateTopMessage( gTopMessage.uiSurface, gTacticalStatus.ubTopMessageType, gTacticalStatus.zTopMessageString );
			}
		}
		else if ( gGameOptions.fTurnTimeLimit )
		{
			if ( gTacticalStatus.ubTopMessageType == PLAYER_TURN_MESSAGE || gTacticalStatus.ubTopMessageType == PLAYER_INTERRUPT_MESSAGE )
			{
				 if ( !gfUserTurnRegionActive && !AreWeInAUIMenu() )
				 {
						// Check Grace period...
						if ( ( GetJA2Clock( ) - gTacticalStatus.uiTactialTurnLimitClock ) > PLAYER_TEAM_TIMER_GRACE_PERIOD )
						{
							gTacticalStatus.uiTactialTurnLimitClock = 0;

							if ( TIMECOUNTERDONE( giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS ) )
							{
								RESETTIMECOUNTER( giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS );

								if ( gTacticalStatus.fTactialTurnLimitStartedBeep )
								{
									if ( ( GetJA2Clock( ) - gTopMessage.uiTimeSinceLastBeep ) > PLAYER_TEAM_TIMER_TIME_BETWEEN_BEEPS )
									{
										gTopMessage.uiTimeSinceLastBeep = GetJA2Clock( );

										// Start sample....
										PlayJA2SampleFromFile("SOUNDS/TURN_NEAR_END.WAV", HIGHVOLUME, 1, MIDDLEPAN);
									}
								}

								// OK, have we gone past the time to
								if ( !gTacticalStatus.fTactialTurnLimitStartedBeep && ( gTacticalStatus.usTactialTurnLimitMax - gTacticalStatus.usTactialTurnLimitCounter ) < PLAYER_TEAM_TIMER_TICKS_FROM_END_TO_START_BEEP )
								{
									gTacticalStatus.fTactialTurnLimitStartedBeep = TRUE;

									gTopMessage.uiTimeSinceLastBeep = GetJA2Clock( );

								}

								// Update counter....
								if ( gTacticalStatus.usTactialTurnLimitCounter < gTacticalStatus.usTactialTurnLimitMax )
								{
									gTacticalStatus.usTactialTurnLimitCounter++;
								}

								CreateTopMessage( gTopMessage.uiSurface, gTacticalStatus.ubTopMessageType, gTacticalStatus.zTopMessageString );

								// Have we reached max?
								if ( gTacticalStatus.usTactialTurnLimitCounter == ( gTacticalStatus.usTactialTurnLimitMax - 1) )
								{
									// IF we are not in lock ui mode....
									CheckForAndHandleEndPlayerTimeLimit( );
								}
							}
						}
				 }
			 }
		}

		// Set redner viewport value
		gsVIEWPORT_WINDOW_START_Y = 20;

		// Check if we have scrolled...
		if ( gTopMessage.sWorldRenderX != gsRenderCenterX || gTopMessage.sWorldRenderY != gsRenderCenterY )
		{
			gfTopMessageDirty = TRUE;
		}

		if ( gfTopMessageDirty )
		{
			SGPRect SrcRect;

			gTopMessage.sWorldRenderX = gsRenderCenterX;
			gTopMessage.sWorldRenderY = gsRenderCenterY;

			SrcRect.iLeft   =  0;
			SrcRect.iTop    = 20 - gTopMessage.bYPos;
			SrcRect.iRight  = SCREEN_WIDTH;
			SrcRect.iBottom = 20;
			BltVideoSurface(FRAME_BUFFER, gTopMessage.uiSurface, 0, 0, &SrcRect);

			SrcRect.iLeft   =  0;
			SrcRect.iTop    =  0;
			SrcRect.iRight  = SCREEN_WIDTH;
			SrcRect.iBottom = 20;
			BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, &SrcRect);

			InvalidateRegion(0, 0, SCREEN_WIDTH, 20);

			gfTopMessageDirty = FALSE;
		}

	}
	else
	{
		// Set redner viewport value
	  gsVIEWPORT_WINDOW_START_Y = 0;
	}
}


void EndTopMessage( )
{
	// OK, end the topmost message!
	if ( gTacticalStatus.fInTopMessage )
	{
		// Are we the last?
		//if ( gTopMessage.bCurrentMessage == 1 )
		{
			SGPRect SrcRect;

			// We are....
			// Re-render our strip and then copy to the save buffer...
			gsVIEWPORT_WINDOW_START_Y = 0;
			gTacticalStatus.fInTopMessage = FALSE;

			SetRenderFlags( RENDER_FLAG_FULL );
			//RenderStaticWorldRect(0, 0, SCREEN_WIDTH, 20, TRUE);
			//gsVIEWPORT_WINDOW_START_Y = 20;

			// Copy into save buffer...
			//SrcRect.iLeft   =  0;
			//SrcRect.iTop    =  0;
			//SrcRect.iRight  = SCREEN_WIDTH;
			//SrcRect.iBottom = 20;
			//BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, &SrcRect);
		}
		//else
		//{
			// Render to save buffer
		//	CreateTopMessage( guiSAVEBUFFER, gTopMessageTypes[ 0 ], gzTopMessageStrings[ 0 ] );
		//}

		// Animate up...
		//gTopMessage.bAnimate = 1;
		// Set time of last update
		//gTopMessage.uiTimeOfLastUpdate = GetJA2Clock( ) + 150;

		// Handle first frame now...
		// HandleTopMessages( );

	}
}


BOOLEAN InTopMessageBarAnimation( )
{
	if ( gTacticalStatus.fInTopMessage )
	{
		if ( 	gTopMessage.bAnimate != 0 )
		{
			HandleTopMessages( );

			return( TRUE );
		}

	}

	return( FALSE );
}


void InitEnemyUIBar( UINT8 ubNumEnemies, UINT8 ubDoneEnemies )
{
	// OK, set value
	gubProgNumEnemies = ubNumEnemies + ubDoneEnemies;
	gubProgCurEnemy		= ubDoneEnemies;
	gfProgBarActive = TRUE;

	gTacticalStatus.usTactialTurnLimitCounter = ubDoneEnemies * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY;
	gTacticalStatus.usTactialTurnLimitMax = ( (ubNumEnemies + ubDoneEnemies) * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY );

}

void UpdateEnemyUIBar( )
{
	// Are we active?
	if ( gfProgBarActive )
	{
		// OK, update team limit counter....
		gTacticalStatus.usTactialTurnLimitCounter = ( gubProgCurEnemy * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY );

		gubProgCurEnemy++;

	}

	// Do we have an active enemy bar?
	if ( gTacticalStatus.fInTopMessage )
	{
		if ( gTacticalStatus.ubTopMessageType == COMPUTER_TURN_MESSAGE )
		{
			// Update message!
			CreateTopMessage( gTopMessage.uiSurface, COMPUTER_TURN_MESSAGE, gTacticalStatus.zTopMessageString );
		}
	}
}


void InitPlayerUIBar( BOOLEAN fInterrupt )
{
	INT8				bNumOK = 0, bNumNotOK = 0;

	if ( !gGameOptions.fTurnTimeLimit )
	{
		if ( fInterrupt == TRUE )
		{
			AddTopMessage( PLAYER_INTERRUPT_MESSAGE, Message[STR_INTERRUPT] );
		}
		else
		{
			//EndTopMessage();
			AddTopMessage( PLAYER_TURN_MESSAGE, TeamTurnString[0] );
		}
		return;
	}

	// OK, calculate time....
	if ( !fInterrupt || gTacticalStatus.usTactialTurnLimitMax == 0 )
	{
		gTacticalStatus.usTactialTurnLimitCounter = 0;

		CFOR_ALL_IN_TEAM(s, gbPlayerNum)
		{
			if (s->bInSector)
			{
				if (s->bLife < OKLIFE)
				{
					bNumNotOK++;
				}
				else
				{
					bNumOK++;
				}
			}
		}

		gTacticalStatus.usTactialTurnLimitMax = ( bNumOK * PLAYER_TEAM_TIMER_TICKS_PER_OK_MERC ) + ( bNumNotOK * PLAYER_TEAM_TIMER_TICKS_PER_NOTOK_MERC );
	}
	else
	{
		if ( gTacticalStatus.usTactialTurnLimitCounter > PLAYER_TEAM_TIMER_INTTERUPT_GRACE )
		{
			gTacticalStatus.usTactialTurnLimitCounter -= PLAYER_TEAM_TIMER_INTTERUPT_GRACE;
		}
	}

	gTacticalStatus.uiTactialTurnLimitClock		= 0;
	gTacticalStatus.fTactialTurnLimitStartedBeep = FALSE;

	// RESET COIUNTER...
	RESETTIMECOUNTER( giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS );


	// OK, set value
	if ( fInterrupt != TRUE )
	{
		AddTopMessage( PLAYER_TURN_MESSAGE, TeamTurnString[0] );
	}
	else
	{
		AddTopMessage( PLAYER_INTERRUPT_MESSAGE, Message[STR_INTERRUPT] );
	}

}


static void MovementMenuBackregionCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		CancelMovementMenu( );
	}
}


static void DoorMenuBackregionCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		CancelOpenDoorMenu( );
	}
}


const wchar_t* GetSoldierHealthString(const SOLDIERTYPE* const s)
{
	INT32 i;
	const INT32 start = (s->bLife == s->bLifeMax ? 4 : 0);
	for (i = start; i < 6; ++i)
	{
		if (s->bLife < bHealthStrRanges[i]) break;
	}
	return zHealthStr[i];
}


typedef struct
{
	INT8				bHeight;
	INT8				bPower;
	INT16				sGridNo;
	UINT8				ubLevel;
	SOLDIERTYPE	*pSoldier;
	BOOLEAN			fShowHeight;
	BOOLEAN			fShowPower;
	BOOLEAN			fActiveHeightBar;
	BOOLEAN			fActivePowerBar;
	BOOLEAN			fAtEndHeight;
	INT16				sTargetGridNo;
	FLOAT				dInitialForce;
	FLOAT				dForce;
	FLOAT				dDegrees;
	FLOAT				dMaxForce;
	UINT8				ubPowerIndex;

} AIMCUBE_UI_DATA;

static BOOLEAN					gfInAimCubeUI = FALSE;
static AIMCUBE_UI_DATA	gCubeUIData;

#define	GET_CUBES_HEIGHT_FROM_UIHEIGHT( h )  ( 32 + ( h * 64 ) )


static void CalculateAimCubeUIPhysics(void)
{
	UINT8	ubHeight;

	ubHeight = GET_CUBES_HEIGHT_FROM_UIHEIGHT( gCubeUIData.bHeight );

	if ( gCubeUIData.fActiveHeightBar )
	{

		// OK, determine which power to use.....
		// TODO this: take force / max force * 10....
		gCubeUIData.ubPowerIndex = (UINT8)( gCubeUIData.dForce / gCubeUIData.dMaxForce * 10 );

	}

	if ( gCubeUIData.fActivePowerBar )
	{
		gCubeUIData.dForce = ( gCubeUIData.dMaxForce * ( (FLOAT)gCubeUIData.ubPowerIndex / (FLOAT)10 ) );

		// Limit to the max force...
		if ( gCubeUIData.dForce > gCubeUIData.dMaxForce )
		{
			gCubeUIData.dForce = gCubeUIData.dMaxForce;
		}

		gCubeUIData.dDegrees = (FLOAT)CalculateLaunchItemAngle( gCubeUIData.pSoldier, gCubeUIData.sGridNo, ubHeight, gCubeUIData.dForce, &(gCubeUIData.pSoldier->inv[ HANDPOS ] ), &(gCubeUIData.sTargetGridNo ) );
	}

}


static INT16 GetInAimCubeUIGridNo(void)
{
	return( gCubeUIData.sGridNo );
}


static BOOLEAN InAimCubeUI(void)
{
	return( gfInAimCubeUI );
}


static BOOLEAN AimCubeUIClick(void)
{
	if ( !gfInAimCubeUI )
	{
		return( FALSE );
	}

	// If we have clicked, and we are only on height, continue with power
	if ( gCubeUIData.fActiveHeightBar && gCubeUIData.bHeight != 0 )
	{
		gCubeUIData.fShowPower				= TRUE;
		gCubeUIData.fActiveHeightBar  = FALSE;
		gCubeUIData.fActivePowerBar		= TRUE;

		return( FALSE );
	}
	else
	{
		return( TRUE );
	}
}


static void BeginAimCubeUI(SOLDIERTYPE* pSoldier, INT16 sGridNo, INT8 ubLevel, UINT8 bStartPower, INT8 bStartHeight)
{
	gfInAimCubeUI = TRUE;

	gCubeUIData.sGridNo			= sGridNo;
	gCubeUIData.ubLevel			= ubLevel;
	gCubeUIData.pSoldier		= pSoldier;
	gCubeUIData.bPower			= bStartPower;
	gCubeUIData.bHeight			= bStartHeight;
	gCubeUIData.fShowHeight = TRUE;
	gCubeUIData.fShowPower	= FALSE;
	gCubeUIData.fActivePowerBar		= FALSE;
	gCubeUIData.fActiveHeightBar	= TRUE;
	gCubeUIData.fAtEndHeight  = FALSE;
	gCubeUIData.dDegrees		= (float)PI/4;


	// Calculate Iniital force....
	CalculateAimCubeUIPhysics( );

}


static void EndAimCubeUI(void)
{
	gfInAimCubeUI = FALSE;
}


static void IncrementAimCubeUI(void)
{
	if ( gCubeUIData.fActiveHeightBar )
	{
		// Cycle the last height yellow once
		if ( gCubeUIData.bHeight == 3 )
		{
			if ( gCubeUIData.fAtEndHeight )
			{
				gCubeUIData.bHeight = 0;
				gCubeUIData.fAtEndHeight = 0;
			}
			else
			{
				gCubeUIData.fAtEndHeight = TRUE;
			}
		}
		else
		{
			gCubeUIData.bHeight++;
		}

		CalculateAimCubeUIPhysics( );

	}

	if ( gCubeUIData.fActivePowerBar )
	{
		if ( gCubeUIData.ubPowerIndex == 10 )
		{
			// Start back to basic7
			gCubeUIData.dDegrees = (FLOAT)(PI/4);
			gCubeUIData.dInitialForce = gCubeUIData.dForce;

			// OK, determine which power to use.....
			// TODO this: take force / max force * 10....
			gCubeUIData.ubPowerIndex = (UINT8)( gCubeUIData.dForce / gCubeUIData.dMaxForce * 10 );
		}

		// Cycle the last height yellow once
		gCubeUIData.ubPowerIndex++;

		CalculateAimCubeUIPhysics( );
	}
}


static void SetupAimCubeAI(void)
{
	if ( gfInAimCubeUI )
	{
		LEVELNODE* const n_tgt = AddTopmostToHead(gCubeUIData.sTargetGridNo, FIRSTPOINTERS2);
		n_tgt->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
		n_tgt->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;

		//LEVELNODE* const n_src = AddTopmostToHead(gCubeUIData.sGridNo, FIRSTPOINTERS2);
		//n_src->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
		//n_src->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
	}
}


static void ResetAimCubeAI(void)
{
	if ( gfInAimCubeUI )
	{
		RemoveTopmost( gCubeUIData.sTargetGridNo, FIRSTPOINTERS2 );
		//RemoveTopmost( gCubeUIData.sGridNo, FIRSTPOINTERS2 );
	}
}

void RenderAimCubeUI( )
{
	INT16 sScreenX, sScreenY;
	INT32	cnt;
	INT16	sBarHeight;
	INT8	bGraphicNum;

	if ( gfInAimCubeUI )
	{

		// OK, given height
		if ( gCubeUIData.fShowHeight )
		{
			// Determine screen location....
			GetGridNoScreenPos( gCubeUIData.sGridNo, gCubeUIData.ubLevel, &sScreenX, &sScreenY );

			// Save background
			const INT32 iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, sScreenX, sScreenY - 70, sScreenX + 40, sScreenY + 50);
			if (iBack != NO_BGND_RECT) SetBackgroundRectFilled(iBack);

			sBarHeight = 0;
			bGraphicNum = 0;

			if ( gCubeUIData.bHeight == 3 && gCubeUIData.fAtEndHeight )
			{
				bGraphicNum = 1;
			}

			// Do first level....
			BltVideoObject(FRAME_BUFFER, guiAIMCUBES, bGraphicNum, sScreenX, sScreenY + sBarHeight);
			sBarHeight -= 3;
			BltVideoObject(FRAME_BUFFER, guiAIMCUBES, bGraphicNum, sScreenX, sScreenY + sBarHeight);

			// Loop through height.....
			for ( cnt = 1; cnt <= gCubeUIData.bHeight; cnt++ )
			{
				sBarHeight -= 3;
				BltVideoObject(FRAME_BUFFER, guiAIMCUBES, bGraphicNum, sScreenX, sScreenY + sBarHeight);
				sBarHeight -= 3;
				BltVideoObject(FRAME_BUFFER, guiAIMCUBES, bGraphicNum, sScreenX, sScreenY + sBarHeight);
				sBarHeight -= 3;
				BltVideoObject(FRAME_BUFFER, guiAIMCUBES, bGraphicNum, sScreenX, sScreenY + sBarHeight);
				sBarHeight -= 3;
				BltVideoObject(FRAME_BUFFER, guiAIMCUBES, bGraphicNum, sScreenX, sScreenY + sBarHeight);
			}
		}


		if ( gCubeUIData.fShowPower )
		{
			sBarHeight = -50;
			BltVideoObject(FRAME_BUFFER, guiAIMBARS, gCubeUIData.ubPowerIndex, sScreenX, sScreenY + sBarHeight);
		}

	}

}


static BOOLEAN gfDisplayPhysicsUI = FALSE;
static INT16	 gsPhysicsImpactPointGridNo;
static INT8		 gbPhysicsImpactPointLevel;
static BOOLEAN gfBadPhysicsCTGT = FALSE;

void BeginPhysicsTrajectoryUI( INT16 sGridNo, INT8 bLevel, BOOLEAN fBadCTGT )
{
	gfDisplayPhysicsUI					= TRUE;
	gsPhysicsImpactPointGridNo	= sGridNo;
	gbPhysicsImpactPointLevel		= bLevel;
	gfBadPhysicsCTGT						= fBadCTGT;
}

void EndPhysicsTrajectoryUI( )
{
	gfDisplayPhysicsUI = FALSE;

}

void SetupPhysicsTrajectoryUI( )
{
	if ( gfDisplayPhysicsUI && gfUIHandlePhysicsTrajectory )
	{
		const UINT16 idx = (gfBadPhysicsCTGT ? FIRSTPOINTERS12 : FIRSTPOINTERS8);
		LEVELNODE* n;
		if (gbPhysicsImpactPointLevel == 0)
		{
			n = AddTopmostToHead(gsPhysicsImpactPointGridNo, idx);
		}
		else
		{
			n = AddOnRoofToHead(gsPhysicsImpactPointGridNo, idx);
		}
		n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
		n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
	}
}


void ResetPhysicsTrajectoryUI( )
{
	if ( gfDisplayPhysicsUI )
	{
		RemoveTopmost( gsPhysicsImpactPointGridNo, FIRSTPOINTERS8 );
		RemoveTopmost( gsPhysicsImpactPointGridNo, FIRSTPOINTERS12 );
		RemoveOnRoof( gsPhysicsImpactPointGridNo, FIRSTPOINTERS8 );
		RemoveOnRoof( gsPhysicsImpactPointGridNo, FIRSTPOINTERS12 );
	}

}

void DirtyTopMessage( )
{
	gTopMessage.fCreated	 = FALSE;
}



static UINT32 CalcUIMessageDuration(const wchar_t* wString)
{
	// base + X per letter
	return( 1000 + 50 * wcslen( wString ) );
}


static BOOLEAN gfMultipurposeLocatorOn = FALSE;
static UINT32  guiMultiPurposeLocatorLastUpdate;
static INT8    gbMultiPurposeLocatorFrame;
static INT16   gsMultiPurposeLocatorGridNo;
static INT8    gbMultiPurposeLocatorLevel;
static INT8    gbMultiPurposeLocatorCycles;


void BeginMultiPurposeLocator(const INT16 sGridNo, const INT8 bLevel)
{
  guiMultiPurposeLocatorLastUpdate = 0;
  gbMultiPurposeLocatorCycles      = 0;
  gbMultiPurposeLocatorFrame       = 0;
  gfMultipurposeLocatorOn = TRUE;

  gsMultiPurposeLocatorGridNo = sGridNo;
  gbMultiPurposeLocatorLevel  = bLevel;
}


void HandleMultiPurposeLocator( )
{
	UINT32			uiClock;

  if ( !gfMultipurposeLocatorOn )
  {
    return;
  }

	// Update radio locator
	uiClock = GetJA2Clock( );

	// Update frame values!
	if ( ( uiClock - guiMultiPurposeLocatorLastUpdate ) > 80 )
	{
		guiMultiPurposeLocatorLastUpdate = uiClock;

		// Update frame
		gbMultiPurposeLocatorFrame++;

		if ( gbMultiPurposeLocatorFrame == 5 )
		{
			gbMultiPurposeLocatorFrame = 0;
      gbMultiPurposeLocatorCycles++;
		}

    if ( gbMultiPurposeLocatorCycles == 8 )
    {
      gfMultipurposeLocatorOn    = FALSE;
    }
	}
}



void RenderTopmostMultiPurposeLocator( )
{
	FLOAT				dOffsetX, dOffsetY;
	FLOAT				dTempX_S, dTempY_S;
	INT16				sX, sY, sXPos, sYPos;

  if ( !gfMultipurposeLocatorOn )
  {
    return;
  }

	ConvertGridNoToCenterCellXY( gsMultiPurposeLocatorGridNo, &sX, &sY );

	dOffsetX = (FLOAT)( sX - gsRenderCenterX );
	dOffsetY = (FLOAT)( sY - gsRenderCenterY );

	// Calculate guy's position
	FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

	sXPos = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + (INT16)dTempX_S;
	sYPos = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + (INT16)dTempY_S - gpWorldLevelData[ gsMultiPurposeLocatorGridNo ].sHeight;

	// Adjust for offset position on screen
	sXPos -= gsRenderWorldOffsetX;
	sYPos -= gsRenderWorldOffsetY;

	// Adjust for render height
	sYPos += gsRenderHeight;

	// Adjust for level height
	if ( gbMultiPurposeLocatorLevel )
	{
		sYPos -= ROOF_LEVEL_HEIGHT;
	}

	// Center circle!
	sXPos -= 20;
	sYPos -= 20;

	const INT32 iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, sXPos, sYPos, sXPos + 40, sYPos + 40);
	if (iBack != NO_BGND_RECT) SetBackgroundRectFilled(iBack);

	BltVideoObject(FRAME_BUFFER, guiRADIO, gbMultiPurposeLocatorFrame, sXPos, sYPos);
}
