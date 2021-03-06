#include "Directories.h"
#include "Font.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "HImage.h"
#include "MapScreen.h"
#include "Soldier_Find.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "SysUtil.h"
#include "Overhead.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Interface.h"
#include "VSurface.h"
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
#include "Map_Screen_Interface_Map.h"
#include "Line.h"
#include "Vehicles.h"
#include "GameSettings.h"
#include "Squads.h"
#include "Message.h"
#include "Debug.h"
#include "Video.h"
#include "Items.h"
#include "GameScreen.h"
#include "MercProfile.h"

#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <stdexcept>


#define ARROWS_X_OFFSET					10
#define ARROWS_HEIGHT						20
#define ARROWS_WIDTH						20
#define UPARROW_Y_OFFSET					-30
#define DOWNARROW_Y_OFFSET					-10

#define BUTTON_PANEL_WIDTH					78
#define BUTTON_PANEL_HEIGHT					76


BOOLEAN	gfInMovementMenu = FALSE;
static INT32 giMenuAnchorX;
static INT32 giMenuAnchorY;

static BOOLEAN gfProgBarActive   = FALSE;
static UINT8   gubProgNumEnemies = 0;
static UINT8   gubProgCurEnemy   = 0;

struct TOP_MESSAGE
{
	SGPVSurface* uiSurface;
	UINT32  uiTimeSinceLastBeep;
	BOOLEAN fCreated;
	INT16   sWorldRenderX;
	INT16   sWorldRenderY;
};

static TOP_MESSAGE gTopMessage;
BOOLEAN gfTopMessageDirty = FALSE;


static MOUSE_REGION gMenuOverlayRegion;


VIDEO_OVERLAY*       g_ui_message_overlay = NULL;
static UINT16        gusUIMessageWidth;
static UINT16        gusUIMessageHeight;
UINT32 guiUIMessageTime = 0;
static MercPopUpBox* g_ui_message_box;
UINT32 guiUIMessageTimeDelay = 0;
static BOOLEAN       gfUseSkullIconMessage = FALSE;

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


static GUIButtonRef iActionIcons[NUM_ICONS];

// GLOBAL INTERFACE SURFACES
SGPVObject* guiDEAD;
SGPVObject* guiHATCH;
static SGPVObject* guiBUTTONBORDER;
SGPVObject* guiRADIO;
static SGPVObject* guiRADIO2;

// UI Globals
MOUSE_REGION	gViewportRegion;
MOUSE_REGION	gRadarRegion;


static UINT16 gsUpArrowX;
static UINT16 gsUpArrowY;
static UINT16 gsDownArrowX;
static UINT16 gsDownArrowY;

static BACKGROUND_SAVE* giUpArrowRect   = NO_BGND_RECT;
static BACKGROUND_SAVE* giDownArrowRect = NO_BGND_RECT;


DirtyLevel         fInterfacePanelDirty = DIRTYLEVEL2;
INT16              gsInterfaceLevel     = I_GROUND_LEVEL;
InterfacePanelKind gsCurInterfacePanel  = TEAM_PANEL;


void InitializeTacticalInterface()
{
	// Load button Interfaces
	iIconImages[WALK_IMAGES ] = LoadButtonImage(INTERFACEDIR "/newicons3.sti", -1,3,4,5,-1 );
	iIconImages[SNEAK_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 6, 7, 8, -1 );
	iIconImages[RUN_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 0, 1, 2, -1 );
	iIconImages[CRAWL_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 9, 10, 11, -1 );
	iIconImages[LOOK_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 12, 13, 14, -1 );
	iIconImages[TALK_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 21, 22, 23, -1 );
	iIconImages[HAND_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 18, 19, 20, -1 );
	iIconImages[CANCEL_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 15, 16, 17, -1 );

	iIconImages[TARGETACTIONC_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 24, 25, 26, -1 );
	iIconImages[KNIFEACTIONC_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 27, 28, 29, -1 );
	iIconImages[AIDACTIONC_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 30, 31, 32, -1 );
	iIconImages[PUNCHACTIONC_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 33, 34, 35, -1 );
	iIconImages[BOMBACTIONC_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 36, 37, 38, -1 );
	iIconImages[TOOLKITACTIONC_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 39, 40, 41, -1 );
	iIconImages[WIRECUTACTIONC_IMAGES] = UseLoadedButtonImage(iIconImages[WALK_IMAGES ], -1, 42, 43, 44, -1 );

	iIconImages[OPEN_DOOR_IMAGES] = LoadButtonImage(INTERFACEDIR "/door_op2.sti", -1,9,10,11,-1 );
	iIconImages[EXAMINE_DOOR_IMAGES] = UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES ], -1, 12, 13, 14, -1 );
	iIconImages[LOCKPICK_DOOR_IMAGES] = UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES ], -1, 21, 22, 23, -1 );
	iIconImages[BOOT_DOOR_IMAGES] = UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES ], -1, 25, 26, 27, -1 );
	iIconImages[CROWBAR_DOOR_IMAGES] = UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES ], -1, 0, 1, 2, -1 );
	iIconImages[USE_KEY_IMAGES] = UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES ], -1, 3, 4, 5, -1 );
	iIconImages[USE_KEYRING_IMAGES] = UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES ], -1, 6, 7, 8, -1 );
	iIconImages[EXPLOSIVE_DOOR_IMAGES] = UseLoadedButtonImage(iIconImages[OPEN_DOOR_IMAGES ], -1, 15, 16, 17, -1 );

	// Load interface panels
	guiDEAD         = AddVideoObjectFromFile(INTERFACEDIR "/p_dead.sti");
	guiHATCH        = AddVideoObjectFromFile(INTERFACEDIR "/hatch.sti");
	guiBUTTONBORDER = AddVideoObjectFromFile(INTERFACEDIR "/button_frame.sti");
	guiRADIO        = AddVideoObjectFromFile(INTERFACEDIR "/radio.sti");
	guiRADIO2       = AddVideoObjectFromFile(INTERFACEDIR "/radio2.sti");

	gTopMessage.uiSurface = AddVideoSurface(SCREEN_WIDTH, 20, PIXEL_DEPTH);

	InitRadarScreen( );

	InitTEAMSlots( );
}


void InitializeCurrentPanel()
{
	MoveRadarScreen( );

	switch( gsCurInterfacePanel )
	{
		case SM_PANEL:
			// Set new viewport
			gsVIEWPORT_WINDOW_END_Y = INV_INTERFACE_START_Y;

			// Render full
			SetRenderFlags(RENDER_FLAG_FULL);
			InitializeSMPanel();
			break;

		case TEAM_PANEL:
			gsVIEWPORT_WINDOW_END_Y = INTERFACE_START_Y;
			// Render full
			SetRenderFlags(RENDER_FLAG_FULL);
			InitializeTEAMPanel();
			break;

		default:
			throw std::logic_error("Tried to initialise invalid tactical panel");
	}

	//RefreshMouseRegions( );
	gfPanelAllocated = TRUE;
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
			default:
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
		case SM_PANEL:
			gSelectSMPanelToMerc = s;
			break;
		case TEAM_PANEL:
			SetTEAMPanelCurrentMerc();
			break;
		default:
			break;
	}
}


void CreateCurrentTacticalPanelButtons(void)
{
	switch (gsCurInterfacePanel)
	{
		case SM_PANEL:
			CreateSMPanelButtons();
			break;
		case TEAM_PANEL:
			CreateTEAMPanelButtons();
			break;
		default:
			break;
	}
}


void SetCurrentInterfacePanel(InterfacePanelKind const ubNewPanel)
{
	if(gfEnteringMapScreen)
		return;
	if(gfPanelAllocated && gsCurInterfacePanel == ubNewPanel)
		return;

	ShutdownCurrentPanel( );

	// INit new panel
	gsCurInterfacePanel = ubNewPanel;

	InitializeCurrentPanel( );


}


void ToggleTacticalPanels( )
{
	SetNewPanel(gsCurInterfacePanel == SM_PANEL ? 0 : GetSelectedMan());
}


void RemoveCurrentTacticalPanelButtons(void)
{
	switch (gsCurInterfacePanel)
	{
		case SM_PANEL:
			RemoveSMPanelButtons();
			break;
		case TEAM_PANEL:
			RemoveTEAMPanelButtons();
			break;
		default:
			break;
	}
}


BOOLEAN IsMercPortraitVisible(const SOLDIERTYPE* s)
{
	switch (gsCurInterfacePanel)
	{
		case TEAM_PANEL:
			return TRUE;
		case SM_PANEL:
			return gpSMCurrentMerc == s;
		default:
			return FALSE;
	}
}


static void HandleUpDownArrowBackgrounds(void);


void HandleInterfaceBackgrounds( )
{
	HandleUpDownArrowBackgrounds( );
}


static void BtnMovementCallback(GUI_BUTTON* btn, INT32 reason);


static void MakeButtonMove(UINT const idx, UINT const gfx, INT16 const x, INT16 const y, UI_EVENT* const event, const ST::string& help, bool const disabled)
{
	GUIButtonRef const btn = QuickCreateButton(iIconImages[gfx], x, y, MSYS_PRIORITY_HIGHEST - 1, BtnMovementCallback);
	iActionIcons[idx] = btn;
	btn->SetUserPtr(event);
	btn->SetFastHelpText(help);
	if (disabled) DisableButton(btn);
}


static void MovementMenuBackregionCallback(MOUSE_REGION* pRegion, INT32 iReason);


void PopupMovementMenu(UI_EVENT* const ev)
{
	EraseInterfaceMenus(TRUE);

	// Create mouse region over all area to facilitate clicking to end
	MSYS_DefineRegion(&gMenuOverlayRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST - 1, CURSOR_NORMAL, MSYS_NO_CALLBACK, MovementMenuBackregionCallback);

	giMenuAnchorX = gusMouseXPos - 18;
	giMenuAnchorY = gusMouseYPos - 18;

	// ATE: Check if we're going off the screen
	if (giMenuAnchorX < 0) giMenuAnchorX = 0;
	if (giMenuAnchorY < 0) giMenuAnchorY = 0;

	// Check for boundaries
	if (giMenuAnchorX > SCREEN_WIDTH - BUTTON_PANEL_WIDTH)
	{
		giMenuAnchorX = SCREEN_WIDTH - BUTTON_PANEL_WIDTH;
	}
	if (giMenuAnchorY > gsVIEWPORT_WINDOW_END_Y - BUTTON_PANEL_HEIGHT)
	{
		giMenuAnchorY = gsVIEWPORT_WINDOW_END_Y - BUTTON_PANEL_HEIGHT;
	}

	INT32              const x                     = giMenuAnchorX + 9;
	INT32              const y                     = giMenuAnchorY + 8;
	SOLDIERTYPE const* const s                     = GetSelectedMan();
	bool               const is_epc                = AM_AN_EPC(s);
	bool               const is_vehicle            = s->uiStatusFlags & SOLDIER_VEHICLE;
	bool               const is_robot              = s->uiStatusFlags & SOLDIER_ROBOT;
	bool               const is_uncontrolled_robot = is_robot && !CanRobotBeControlled(s);

	MakeButtonMove(LOOK_ICON, LOOK_IMAGES, x, y, ev, TacticalStr[LOOK_CURSOR_POPUPTEXT],
			is_vehicle || is_uncontrolled_robot);
	MakeButtonMove(RUN_ICON, RUN_IMAGES, x + 20, y, ev, pTacticalPopupButtonStrings[RUN_ICON],
			is_vehicle || is_robot || MercInWater(s));
	ST::string help = is_vehicle ? TacticalStr[DRIVE_POPUPTEXT] : pTacticalPopupButtonStrings[WALK_ICON];
	MakeButtonMove(WALK_ICON, WALK_IMAGES, x + 40, y, ev, help, is_uncontrolled_robot);

	UINT32         action_image;
	ST::string action_text;
	bool           disable_action = false;
	if (is_vehicle)
	{
		// Until we get mounted weapons
		action_image   = CANCEL_IMAGES;
		action_text    = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
		disable_action = true;
	}
	else
	{
		// Create button based on what is in our hands at the moment!
		UINT16 const item = s->inv[HANDPOS].usItem;
		if (item == TOOLKIT)
		{
			action_image = TOOLKITACTIONC_IMAGES;
			action_text  = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
		}
		else if (item == WIRECUTTERS)
		{
			action_image = WIRECUTACTIONC_IMAGES;
			action_text  = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
		}
		else switch (GCM->getItem(item)->getItemClass())
		{
			case IC_PUNCH:
				action_image = PUNCHACTIONC_IMAGES;
				action_text  = TacticalStr[USE_HANDTOHAND_POPUPTEXT];
				break;

			case IC_GUN:
				action_image = TARGETACTIONC_IMAGES;
				action_text  = TacticalStr[USE_FIREARM_POPUPTEXT];
				break;

			case IC_BLADE:
				action_image = KNIFEACTIONC_IMAGES;
				action_text  = TacticalStr[USE_BLADE_POPUPTEXT];
				break;

			case IC_GRENADE:
			case IC_BOMB:
				action_image = BOMBACTIONC_IMAGES;
				action_text  = TacticalStr[USE_EXPLOSIVE_POPUPTEXT];
				break;

			case IC_MEDKIT:
				action_image = AIDACTIONC_IMAGES;
				action_text  = TacticalStr[USE_MEDKIT_POPUPTEXT];
				break;

			default:
				action_image   = CANCEL_IMAGES;
				action_text    = TacticalStr[NOT_APPLICABLE_POPUPTEXT];
				disable_action = true;
				break;
		}
	}

	MakeButtonMove(ACTIONC_ICON, action_image, x, y + 20, ev, action_text,
			is_epc || disable_action);
	MakeButtonMove(CANCEL_ICON, CANCEL_IMAGES, x + 20, y + 20, ev, pTacticalPopupButtonStrings[CANCEL_ICON],
			false);
	MakeButtonMove(SNEAK_ICON, SNEAK_IMAGES, x + 40, y + 20, ev, pTacticalPopupButtonStrings[SNEAK_ICON],
			!IsValidStance(s, ANIM_CROUCH));
	MakeButtonMove(TALK_ICON, TALK_IMAGES, x, y + 40, ev, pTacticalPopupButtonStrings[TALK_ICON],
			is_epc || is_vehicle);
	MakeButtonMove(HAND_ICON, HAND_IMAGES, x + 20, y + 40, ev, pTacticalPopupButtonStrings[HAND_ICON],
			is_epc || is_vehicle);
	MakeButtonMove(CRAWL_ICON, CRAWL_IMAGES, x + 40, y + 40, ev, pTacticalPopupButtonStrings[CRAWL_ICON],
			!IsValidStance(s, ANIM_PRONE));

	gfInMovementMenu  = TRUE;
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

		InvalidateRegion(giMenuAnchorX, giMenuAnchorY, giMenuAnchorX + BUTTON_PANEL_WIDTH,
					giMenuAnchorY + BUTTON_PANEL_HEIGHT);

	}
}

void CancelMovementMenu( )
{
	// Signal end of event
	PopDownMovementMenu( );
	guiPendingOverrideEvent = A_CHANGE_TO_MOVE;
}


static void BtnMovementCallback(GUI_BUTTON* btn, INT32 reason)
{
	if ( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;

		UI_EVENT* const pUIEvent = btn->GetUserPtr<UI_EVENT>();

		if (btn == iActionIcons[WALK_ICON])
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_WALK;
		}
		else if (btn == iActionIcons[RUN_ICON])
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_RUN;
		}
		else if (btn == iActionIcons[SNEAK_ICON])
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_SWAT;
		}
		else if (btn == iActionIcons[CRAWL_ICON])
		{
			pUIEvent->uiParams[0] = MOVEMENT_MENU_PRONE;
		}
		else if (btn == iActionIcons[LOOK_ICON])
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_LOOK;
		}
		else if (btn == iActionIcons[ACTIONC_ICON])
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_ACTIONC;
		}
		else if (btn == iActionIcons[TALK_ICON])
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_TALK;
		}
		else if (btn == iActionIcons[HAND_ICON])
		{
			pUIEvent->uiParams[2] = MOVEMENT_MENU_HAND;
		}
		else if (btn == iActionIcons[CANCEL_ICON])
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
	static UINT32 uiOldShowUpDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

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
	TILE_ELEMENT TileElem;

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
		BltVideoObject( FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
		TileElem = gTileDatabase[ SECONDPOINTERS1 ];
		BltVideoObject( FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_GG )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS1 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_YY )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS3 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS8 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB2 )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS3 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
		TileElem = gTileDatabase[ SECONDPOINTERS8 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_UP_ABOVE_CLIMB3 )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS3 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY);
		TileElem = gTileDatabase[ SECONDPOINTERS8 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 20);
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsUpArrowX, gsUpArrowY + 40);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BESIDE )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS4 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_G )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS2 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_Y )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS4 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_CLIMB )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS7 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YG )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS2 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
		TileElem = gTileDatabase[ SECONDPOINTERS4 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_GG )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS2 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY + 20);
	}

	if ( guiShowUPDownArrows & ARROWS_SHOW_DOWN_BELOW_YY )
	{
		TileElem = gTileDatabase[ SECONDPOINTERS4 ];
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY);
		BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, gsDownArrowX, gsDownArrowY + 20);
	}


}

void EraseRenderArrows( )
{
	if (giUpArrowRect != NO_BGND_RECT)
		FreeBackgroundRect(giUpArrowRect);
	giUpArrowRect = NO_BGND_RECT;

	if (giDownArrowRect != NO_BGND_RECT)
		FreeBackgroundRect(giDownArrowRect);
	giDownArrowRect = NO_BGND_RECT;
}


static void GetArrowsBackground(void)
{
	INT16  sMercScreenX, sMercScreenY;
	UINT16 sArrowHeight = ARROWS_HEIGHT, sArrowWidth = ARROWS_WIDTH;

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
		//gsDownArrowY -= ROOF_LEVEL_HEIGHT;
		//gsUpArrowY -= ROOF_LEVEL_HEIGHT;
	}

	//Erase prevois ones...
	EraseRenderArrows();

	// Register dirty rects
	giDownArrowRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, gsDownArrowX, gsDownArrowY,
							sArrowWidth, sArrowHeight);
	giUpArrowRect   = RegisterBackgroundRect(BGND_FLAG_PERMANENT, gsUpArrowX,gsUpArrowY,
							sArrowWidth, sArrowHeight);
}


void GetSoldierAboveGuyPositions(SOLDIERTYPE const* const s, INT16* const psX, INT16* const psY, BOOLEAN const fRadio)
{
	INT16 const sTextBodyTypeYOffset = 62;

	INT16 sStanceOffset;
	// Adjust based on body type
	switch (s->ubBodyType)
	{
		case CROW:
			sStanceOffset = 30;
			break;
		case ROBOTNOWEAPON:
			sStanceOffset = 30;
			break;

		default:
		{
			// Adjust based on stance
			ANIMCONTROLTYPE const& ac = gAnimControl[s->usAnimState];
			UINT8 const ubAnimUseHeight = ac.uiFlags & ANIM_NOMOVE_MARKER ?
							ac.ubHeight : ac.ubEndHeight;
			switch (ubAnimUseHeight)
			{
				case ANIM_PRONE:
					sStanceOffset = 25;
					break;
				case ANIM_CROUCH:
					sStanceOffset = 10;
					break;
				default:
					sStanceOffset =  0;
					break;
			}
			break;
		}
	}

	INT16 sMercScreenX;
	INT16 sMercScreenY;
	GetSoldierTRUEScreenPos(s, &sMercScreenX, &sMercScreenY);

	*psX = sMercScreenX - 80 / 2;
	*psY = sMercScreenY - sTextBodyTypeYOffset + sStanceOffset;

	// OK, Check if we need to go below....
	// Can do this if 1) displaying damge or 2) above screen
	if (s->ubProfile != NO_PROFILE &&
		!fRadio &&
		(s->fDisplayDamage || *psY < gsVIEWPORT_WINDOW_START_Y))
	{
		*psY = sMercScreenY;
	}
}


static void PrintAboveGuy(INT16 const x, INT16 const y, const ST::string& text)
{
	INT16 cx;
	INT16 cy;
	FindFontCenterCoordinates(x, y, 80, 1, text, TINYFONT1, &cx, &cy);
	GDirtyPrint(cx, cy, text);
}


static void DrawBarsInUIBox(const SOLDIERTYPE* pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight);


void DrawSelectedUIAboveGuy(SOLDIERTYPE& s)
{
	if (s.bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS)) return;

	if (s.sGridNo == NOWHERE) return;

	if (s.fFlashLocator)
	{
		if (s.bVisible == -1)
		{
			s.fFlashLocator = FALSE;
		}
		else
		{
			if (TIMECOUNTERDONE(s.BlinkSelCounter, 80))
			{
				RESETTIMECOUNTER(s.BlinkSelCounter, 80);

				s.fShowLocator = TRUE;
				if (++s.sLocatorFrame == 5)
				{
					// Update time we do this
					++s.fFlashLocator;
					s.sLocatorFrame = 0;
				}
			}

			if (s.fFlashLocator == s.ubNumLocateCycles)
			{
				s.fFlashLocator = FALSE;
				s.fShowLocator  = FALSE;
			}

			// Render the beastie
			INT16 sXPos;
			INT16 sYPos;
			GetSoldierAboveGuyPositions(&s, &sXPos, &sYPos, TRUE);

			// Adjust for bars
			sXPos += 25;
			sYPos += 25;

			// Add bars
			RegisterBackgroundRectSingleFilled(sXPos, sYPos, 40, 40);

			SGPVObject const* const gfx = s.bNeutral || s.bSide == OUR_TEAM ?
							guiRADIO : guiRADIO2;
			BltVideoObject(FRAME_BUFFER, gfx, s.sLocatorFrame, sXPos, sYPos);
		}
	}

	// If he is in the middle of a certain animation, ignore
	if (gAnimControl[s.usAnimState].uiFlags & ANIM_NOSHOW_MARKER) return;

	// Do not show if we are dead
	if (s.uiStatusFlags & SOLDIER_DEAD) return;

	UINT16                   usGraphicToUse;
	SOLDIERTYPE const* const sel = GetSelectedMan();
	if (&s == sel && !gRubberBandActive)
	{
		usGraphicToUse = THIRDPOINTERS2;
	}
	else if (s.fShowLocator ||
		s.uiStatusFlags & SOLDIER_MULTI_SELECTED ||
		(&s == gSelectedGuy && !gfIgnoreOnSelectedGuy))
	{
		usGraphicToUse = THIRDPOINTERS1;
	}
	else
	{
		return;
	}

	INT16 sXPos;
	INT16 sYPos;
	GetSoldierAboveGuyPositions(&s, &sXPos, &sYPos, FALSE);

	// Display name
	SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);

	if (s.ubProfile != NO_PROFILE || s.uiStatusFlags & SOLDIER_VEHICLE)
	{
		ST::string action;
		if (&s == gUIValidCatcher && gfUIMouseOnValidCatcher == 1)
		{
			action = TacticalStr[CATCH_STR];
		}
		else if (&s == gUIValidCatcher && gfUIMouseOnValidCatcher == 3)
		{
			action = TacticalStr[RELOAD_STR];
		}
		else if (&s == gUIValidCatcher && gfUIMouseOnValidCatcher == 4)
		{
			action = pMessageStrings[MSG_PASS];
		}
		else if (s.bAssignment >= ON_DUTY)
		{
			SetFontForeground(FONT_YELLOW);
			action = ST::format("({})", pAssignmentStrings[s.bAssignment]);
		}
		else if (s.bTeam == OUR_TEAM &&
			s.bAssignment < ON_DUTY &&
			s.bAssignment != CurrentSquad() &&
			!(s.uiStatusFlags & SOLDIER_MULTI_SELECTED))
		{
			action = st_format_printf(gzLateLocalizedString[STR_LATE_34], s.bAssignment + 1);
		}

		bool raise_name = false;
		if (!action.empty())
		{
			PrintAboveGuy(sXPos, sYPos, action);
			raise_name = true;
		}

		// If not in a squad
		if (s.uiStatusFlags & SOLDIER_VEHICLE)
		{
			if (GetNumberInVehicle(GetVehicle(s.bVehicleID)) == 0)
			{
				SetFontForeground(FONT_GRAY4);
			}
		}
		else if (s.bAssignment >= ON_DUTY)
		{
			SetFontForeground(FONT_YELLOW);
		}

		PrintAboveGuy(sXPos, raise_name ? sYPos - 10 : sYPos, s.name);

		if ((s.ubProfile != NO_PROFILE && MercProfile(s.ubProfile).isPlayerMerc()) ||
			RPC_RECRUITED(&s) ||
			AM_AN_EPC(&s) ||
			s.uiStatusFlags & SOLDIER_VEHICLE)
		{
			// Adjust for bars!
			if (&s == sel)
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
			RegisterBackgroundRectSingleFilled(sXPos, sYPos, 34, 11);
			TILE_ELEMENT const& TileElem = gTileDatabase[usGraphicToUse];
			BltVideoObject(FRAME_BUFFER, TileElem.hTileSurface, TileElem.usRegionIndex, sXPos, sYPos);

			// Draw life, breath
			if (&s == sel)
			{
				sXPos += 1;
				sYPos += 2;
			}
			DrawBarsInUIBox(&s, sXPos, sYPos, 16, 1);
			return;
		}
		else
		{
			sYPos += 10;

			if (gfUIMouseOnValidCatcher == 2 && &s == gUIValidCatcher)
			{
				SetFontForeground(FONT_MCOLOR_WHITE);
				PrintAboveGuy(sXPos, sYPos, TacticalStr[GIVE_STR]);
				return;
			}
		}
	}
	else if (s.bLevel != 0)
	{
		SetFontForeground(FONT_YELLOW);
		PrintAboveGuy(sXPos, sYPos + 10, gzLateLocalizedString[STR_LATE_15]);
	}

	SetFontForeground(FONT_MCOLOR_DKRED);
	PrintAboveGuy(sXPos, sYPos, GetSoldierHealthString(&s));
}


static void DrawBarsInUIBox(const SOLDIERTYPE* pSoldier, INT16 sXPos, INT16 sYPos, INT16 sWidth, INT16 sHeight)
{
	FLOAT  dWidth, dPercentage;
	//UINT16 usLineColor;
	UINT16 usLineColor;
	INT8   bBandage;

	// Draw breath points

	// Draw new size
	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf = l.Buffer<UINT16>();
	SetClippingRegionAndImageWidth(l.Pitch(), 0, gsVIEWPORT_WINDOW_START_Y, SCREEN_WIDTH, gsVIEWPORT_WINDOW_END_Y - gsVIEWPORT_WINDOW_START_Y);

	// get amt bandaged
	bBandage = pSoldier->bLifeMax - pSoldier->bLife - pSoldier->bBleeding;



	// NOW DO BLEEDING
	if ( pSoldier->bBleeding )
	{
		dPercentage = (FLOAT)( pSoldier->bBleeding +  pSoldier->bLife + bBandage )/ (FLOAT)100;
		dWidth = dPercentage * sWidth;
		usLineColor = Get16BPPColor(FROMRGB(240, 240, 20));
		RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (INT32)(sXPos + dWidth + 3), sYPos + 1, usLineColor, pDestBuf);
	}

	if( bBandage )
	{
		dPercentage = (FLOAT)( pSoldier->bLife + bBandage ) / (FLOAT)100;
		dWidth = dPercentage * sWidth;
		usLineColor = Get16BPPColor( FROMRGB( 222, 132, 132 ) );
		RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (INT32)(sXPos + dWidth + 3), sYPos + 1, usLineColor, pDestBuf);
	}

	dPercentage = (FLOAT)pSoldier->bLife / (FLOAT)100;
	dWidth = dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(200, 0, 0));
	RectangleDraw(TRUE, sXPos + 3, sYPos + 1, (INT32)(sXPos + dWidth + 3), sYPos + 1, usLineColor, pDestBuf);

	dPercentage = (FLOAT)( pSoldier->bBreathMax ) / (FLOAT)100;
	dWidth = dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(20, 20, 150));
	RectangleDraw(TRUE, sXPos + 3, sYPos + 4, (INT32)(sXPos + dWidth + 3), sYPos + 4, usLineColor, pDestBuf);

	dPercentage = (FLOAT)( pSoldier->bBreath ) / (FLOAT)100;
	dWidth = dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(100, 100, 220));
	RectangleDraw(TRUE, sXPos + 3, sYPos + 4, (INT32)(sXPos + dWidth + 3), sYPos + 4, usLineColor, pDestBuf);

	/*
	// morale
	dPercentage = (FLOAT)pSoldier->bMorale / (FLOAT)100;
	dWidth = dPercentage * sWidth;
	usLineColor = Get16BPPColor(FROMRGB(0, 250, 0));
	RectangleDraw(TRUE, sXPos + 1, sYPos + 7, (INT32)(sXPos + dWidth + 1), sYPos + 7, usLineColor, pDestBuf);*/
}

void ClearInterface( )
{
	if (fInMapMode) return; // XXX necessary?

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
	guiShowUPDownArrows = ARROWS_HIDE_UP | ARROWS_HIDE_DOWN;

	ResetPhysicsTrajectoryUI( );

	// Remove any paths, cursors
	ErasePath();

	//gfPlotNewMovement = TRUE;

	// Erase Interface cursors
	HideUICursor( );

	gViewportRegion.ChangeCursor(VIDEO_NO_CURSOR);

	// Hide lock UI cursors...
	gDisableRegion.ChangeCursor(VIDEO_NO_CURSOR);
	gUserTurnRegion.ChangeCursor(VIDEO_NO_CURSOR);

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
	gDisableRegion.ChangeCursor(CURSOR_WAIT);
	gUserTurnRegion.ChangeCursor(CURSOR_WAIT);
}


void DirtyMercPanelInterface(SOLDIERTYPE const* const pSoldier, DirtyLevel const dirty_level)
{
	if ( pSoldier->bTeam == OUR_TEAM )
	{
		// ONly set to a higher level!
		if (fInterfacePanelDirty < dirty_level)
		{
			fInterfacePanelDirty = dirty_level;
		}
	}

}

struct OPENDOOR_MENU
{
	SOLDIERTYPE *pSoldier;
	INT16   sX;
	INT16   sY;
	BOOLEAN fMenuHandled;
	BOOLEAN fClosingDoor;
};

static OPENDOOR_MENU gOpenDoorMenu;
BOOLEAN gfInOpenDoorMenu = FALSE;


static void PopupDoorOpenMenu(BOOLEAN fClosingDoor);


void InitDoorOpenMenu(SOLDIERTYPE* const pSoldier, BOOLEAN const fClosingDoor)
{
	INT16 sScreenX, sScreenY;

	// Erase other menus....
	EraseInterfaceMenus( TRUE );

	InterruptTime();
	PauseGame();
	LockPauseState(LOCK_PAUSE_DOOR_OPEN);
	// Pause timers as well....
	PauseTime( TRUE );


	gOpenDoorMenu.pSoldier     = pSoldier;
	gOpenDoorMenu.fClosingDoor = fClosingDoor;

	// OK, Determine position...
	// Center on guy
	// Locate to guy first.....
	LocateSoldier(pSoldier, FALSE);
	GetSoldierScreenPos( pSoldier, &sScreenX, &sScreenY );
	gOpenDoorMenu.sX = sScreenX - (BUTTON_PANEL_WIDTH  - pSoldier->sBoundingBoxWidth)  / 2;
	gOpenDoorMenu.sY = sScreenY - (BUTTON_PANEL_HEIGHT - pSoldier->sBoundingBoxHeight) / 2;

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


	gOpenDoorMenu.fMenuHandled = FALSE;

	guiPendingOverrideEvent    = OP_OPENDOORMENU;
	HandleTacticalUI( );

	PopupDoorOpenMenu( fClosingDoor );
}


static void BtnDoorMenuCallback(GUI_BUTTON* btn, INT32 reason);


static void MakeButtonDoor(UINT idx, UINT gfx, INT16 x, INT16 y, INT16 ap, INT16 bp, BOOLEAN disable, const ST::string& help)
{
	GUIButtonRef const btn = QuickCreateButton(iIconImages[gfx], x, y, MSYS_PRIORITY_HIGHEST - 1, BtnDoorMenuCallback);
	iActionIcons[idx] = btn;
	if (ap == 0 || !(gTacticalStatus.uiFlags & INCOMBAT))
	{
		btn->SetFastHelpText(help);
	}
	else
	{
		ST::string zDisp = ST::format("{} ( {} )", help, ap);
		btn->SetFastHelpText(zDisp);
	}
	if (disable || (ap != 0 && !EnoughPoints(gOpenDoorMenu.pSoldier, ap, bp, FALSE)))
	{
		DisableButton(btn);
	}
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

	d = d0 || !SoldierHasKey(*gOpenDoorMenu.pSoldier, ANYKEY);
	MakeButtonDoor(USE_KEYRING_ICON, USE_KEYRING_IMAGES, dx + 20, dy, AP_UNLOCK_DOOR, BP_UNLOCK_DOOR, d,
			pTacticalPopupButtonStrings[USE_KEYRING_ICON]);

	d = fClosingDoor || FindUsableObj(gOpenDoorMenu.pSoldier, CROWBAR) == NO_SLOT;
	MakeButtonDoor(USE_CROWBAR_ICON, CROWBAR_DOOR_IMAGES, dx + 40, dy, AP_USE_CROWBAR, BP_USE_CROWBAR, d,
			pTacticalPopupButtonStrings[USE_CROWBAR_ICON]);

	d = d0 || FindObj(gOpenDoorMenu.pSoldier, LOCKSMITHKIT) == NO_SLOT;
	MakeButtonDoor(LOCKPICK_DOOR_ICON, LOCKPICK_DOOR_IMAGES, dx + 40, dy + 20, AP_PICKLOCK, BP_PICKLOCK, d,
			pTacticalPopupButtonStrings[LOCKPICK_DOOR_ICON]);

	d = d0 || FindObj(gOpenDoorMenu.pSoldier, SHAPED_CHARGE) == NO_SLOT;
	MakeButtonDoor(EXPLOSIVE_DOOR_ICON, EXPLOSIVE_DOOR_IMAGES, dx + 40, dy + 40, AP_EXPLODE_DOOR, BP_EXPLODE_DOOR,
			d, pTacticalPopupButtonStrings[EXPLOSIVE_DOOR_ICON]);

	ST::string help = pTacticalPopupButtonStrings[fClosingDoor ? CANCEL_ICON + 1 : OPEN_DOOR_ICON];
	MakeButtonDoor(OPEN_DOOR_ICON, OPEN_DOOR_IMAGES, dx, dy, AP_OPEN_DOOR, BP_OPEN_DOOR, FALSE, help);

	MakeButtonDoor(EXAMINE_DOOR_ICON, EXAMINE_DOOR_IMAGES, dx, dy + 20, AP_EXAMINE_DOOR, BP_EXAMINE_DOOR, d0,
			pTacticalPopupButtonStrings[EXAMINE_DOOR_ICON]);
	MakeButtonDoor(BOOT_DOOR_ICON, BOOT_DOOR_IMAGES, dx, dy + 40, AP_BOOT_DOOR, BP_BOOT_DOOR, d0,
			pTacticalPopupButtonStrings[BOOT_DOOR_ICON]);
	MakeButtonDoor(UNTRAP_DOOR_ICON, UNTRAP_DOOR_ICON, dx + 20, dy + 40, AP_UNTRAP_DOOR, BP_UNTRAP_DOOR, d0,
			pTacticalPopupButtonStrings[UNTRAP_DOOR_ICON]);
	MakeButtonDoor(CANCEL_ICON, CANCEL_IMAGES, dx + 20, dy + 20, 0, 0, FALSE,
			pTacticalPopupButtonStrings[CANCEL_ICON]);

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


static void DoorAction(INT16 const ap, INT16 const bp, HandleDoor const action)
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

		// Popdown menu
		gOpenDoorMenu.fMenuHandled = TRUE;

		if (btn == iActionIcons[CANCEL_ICON])
		{
			// OK, set cancle code!
			gOpenDoorMenu.fMenuHandled = 2;
		}
		else if (btn == iActionIcons[OPEN_DOOR_ICON])
		{
			// Open door normally...
			// Check APs
			if (EnoughPoints(gOpenDoorMenu.pSoldier, AP_OPEN_DOOR, BP_OPEN_DOOR, FALSE))
			{
				// Set UI
				SetUIBusy(gOpenDoorMenu.pSoldier);

				if (gOpenDoorMenu.fClosingDoor)
				{
					ChangeSoldierState(gOpenDoorMenu.pSoldier, GetAnimStateForInteraction(*gOpenDoorMenu.pSoldier, TRUE, CLOSE_DOOR), 0, FALSE);
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
		else if (btn == iActionIcons[BOOT_DOOR_ICON])
		{
			DoorAction(AP_BOOT_DOOR, BP_BOOT_DOOR, HANDLE_DOOR_FORCE);
		}
		else if (btn == iActionIcons[USE_KEYRING_ICON])
		{
			DoorAction(AP_UNLOCK_DOOR, BP_UNLOCK_DOOR, HANDLE_DOOR_UNLOCK);
		}
		else if (btn == iActionIcons[LOCKPICK_DOOR_ICON])
		{
			DoorAction(AP_PICKLOCK, BP_PICKLOCK, HANDLE_DOOR_LOCKPICK);
		}
		else if (btn == iActionIcons[EXAMINE_DOOR_ICON])
		{
			DoorAction(AP_EXAMINE_DOOR, BP_EXAMINE_DOOR, HANDLE_DOOR_EXAMINE);
		}
		else if (btn == iActionIcons[EXPLOSIVE_DOOR_ICON])
		{
			DoorAction(AP_EXPLODE_DOOR, BP_EXPLODE_DOOR, HANDLE_DOOR_EXPLODE);
		}
		else if (btn == iActionIcons[UNTRAP_DOOR_ICON])
		{
			DoorAction(AP_UNTRAP_DOOR, BP_UNTRAP_DOOR, HANDLE_DOOR_UNTRAP);
		}
		else if (btn == iActionIcons[USE_CROWBAR_ICON])
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
	pBlitter->uiDestBuff->ShadowRect(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusUIMessageWidth - 2,
						pBlitter->sY + gusUIMessageHeight - 2);

	RenderMercPopUpBox(g_ui_message_box, pBlitter->sX, pBlitter->sY,  pBlitter->uiDestBuff);

	InvalidateRegion(pBlitter->sX, pBlitter->sY, pBlitter->sX + gusUIMessageWidth,
				pBlitter->sY + gusUIMessageHeight);
}


static UINT32 CalcUIMessageDuration(const ST::string& str);


void BeginUIMessage(BOOLEAN fUseSkullIcon, const ST::string& text)
{
	guiUIMessageTime = GetJA2Clock( );
	guiUIMessageTimeDelay = CalcUIMessageDuration(text);

	MercPopupBoxFlags const flags = fUseSkullIcon ?
		MERC_POPUP_PREPARE_FLAGS_SKULLICON :
		MERC_POPUP_PREPARE_FLAGS_STOPICON;

	// Prepare text box
	g_ui_message_box = PrepareMercPopupBox(g_ui_message_box, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, text, 200, 10, 0, 0, &gusUIMessageWidth, &gusUIMessageHeight, flags);

	if (g_ui_message_overlay != NULL)
	{
		RemoveVideoOverlay(g_ui_message_overlay);
		g_ui_message_overlay = NULL;
	}

	if (g_ui_message_overlay == NULL)
	{
		INT16 const x = (SCREEN_WIDTH - gusUIMessageWidth) / 2;
		g_ui_message_overlay = RegisterVideoOverlay(RenderUIMessage, x, 150, gusUIMessageWidth, gusUIMessageHeight);
	}

	gfUseSkullIconMessage = fUseSkullIcon;
}


void BeginMapUIMessage(INT16 delta_y, const ST::string& text)
{
	guiUIMessageTime      = GetJA2Clock();
	guiUIMessageTimeDelay = CalcUIMessageDuration(text);

	g_ui_message_box = PrepareMercPopupBox(g_ui_message_box, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, text, 200, 10, 0, 0, &gusUIMessageWidth, &gusUIMessageHeight, MERC_POPUP_PREPARE_FLAGS_TRANS_BACK);

	if (g_ui_message_overlay == NULL)
	{
		INT16 const x = MAP_VIEW_START_X + (MAP_VIEW_WIDTH  - gusUIMessageWidth)  / 2 + 20;
		INT16 const y = MAP_VIEW_START_Y + (MAP_VIEW_HEIGHT - gusUIMessageHeight) / 2 + delta_y;
		g_ui_message_overlay = RegisterVideoOverlay(RenderUIMessage, x, y, gusUIMessageWidth, gusUIMessageHeight);
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

		RemoveVideoOverlay(g_ui_message_overlay);
		g_ui_message_overlay = NULL;

		// Remove popup as well....
		if (g_ui_message_box)
		{
			RemoveMercPopupBox(g_ui_message_box);
			g_ui_message_box = 0;
		}
	}
}

#define PLAYER_TEAM_TIMER_INTTERUPT_GRACE			(15000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_GRACE_PERIOD				1000
#define PLAYER_TEAM_TIMER_SEC_PER_TICKS			100
#define PLAYER_TEAM_TIMER_TICKS_PER_OK_MERC			(15000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_TICKS_PER_NOTOK_MERC			(5000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_TICKS_FROM_END_TO_START_BEEP		(5000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)
#define PLAYER_TEAM_TIMER_TIME_BETWEEN_BEEPS			500
#define PLAYER_TEAM_TIMER_TICKS_PER_ENEMY			(2000 / PLAYER_TEAM_TIMER_SEC_PER_TICKS)


static void CreateTopMessage(void);


void AddTopMessage(const MESSAGE_TYPES ubType)
{
	gTopMessage.fCreated = TRUE;

	gTacticalStatus.ubTopMessageType = ubType;
	gTacticalStatus.fInTopMessage    = TRUE;

	CreateTopMessage();
}


static void CreateTopMessage(void)
{
	const TacticalStatusType* const ts  = &gTacticalStatus;
	SGPVSurface*              const dst = gTopMessage.uiSurface;

	SetFontDestBuffer(dst);

	const char* bar_file;
	UINT16      bar_gfx     = 0;
	BOOLEAN     fDoLimitBar = FALSE;
	UINT8       foreground;
	UINT8       shadow;
	switch (ts->ubTopMessageType)
	{
		case COMPUTER_TURN_MESSAGE:
		case COMPUTER_INTERRUPT_MESSAGE:
		case MILITIA_INTERRUPT_MESSAGE:
		case AIR_RAID_TURN_MESSAGE:
			bar_file    = INTERFACEDIR "/rect.sti";
			foreground  = FONT_MCOLOR_WHITE;
			shadow      = DEFAULT_SHADOW;
			fDoLimitBar = TRUE;
			break;

		case PLAYER_INTERRUPT_MESSAGE:
			bar_file   = INTERFACEDIR "/timebaryellow.sti";
			foreground = FONT_MCOLOR_BLACK;
			shadow     = NO_SHADOW;
			if (gGameOptions.fTurnTimeLimit) fDoLimitBar = TRUE;
			break;

		case PLAYER_TURN_MESSAGE:
			bar_file   = INTERFACEDIR "/timebargreen.sti";
			foreground = FONT_MCOLOR_BLACK;
			shadow     = NO_SHADOW;
			if (gGameOptions.fTurnTimeLimit)
			{
				fDoLimitBar = TRUE;
			}
			else
			{
				//bar_gfx     = 13;
			}
			break;

		default:
			abort();
	}
	SetFontAttributes(TINYFONT1, foreground, shadow);

	const SGPBox* const bar = &g_ui.m_progress_bar_box;
	{
		AutoSGPVObject bar_vo(AddVideoObjectFromFile(bar_file));

		BltVideoObject(dst, bar_vo.get(), bar_gfx, STD_SCREEN_X, 0);

		if (fDoLimitBar)
		{
			INT32 bar_x = bar->x;
			// Render end piece
			BltVideoObject(dst, bar_vo.get(), 1, bar_x, bar->y);

			INT32  gfx    = 2;
			// -3 for the end pieces
			UINT32 length = (bar->w - 3) * ts->usTactialTurnLimitCounter / ts->usTactialTurnLimitMax;
			while (length-- != 0)
			{
				BltVideoObject(dst, bar_vo.get(), gfx, ++bar_x, bar->y);
				if (++gfx == 12) gfx = 2;
			}

			if (ts->usTactialTurnLimitCounter == ts->usTactialTurnLimitMax)
			{
				// Render end piece
				BltVideoObject(dst, bar_vo.get(), gfx, ++bar_x, bar->y);
				BltVideoObject(dst, bar_vo.get(), 12,  ++bar_x, bar->y);
			}
		}
	}

	ST::string msg;
	switch (ts->ubTopMessageType)
	{
		case COMPUTER_TURN_MESSAGE:
		{
			const UINT8 team = ts->ubCurrentTeam;
			msg = team == CREATURE_TEAM && HostileBloodcatsPresent() ?
					g_langRes->Message[STR_BLOODCATS_TURN] :
					TeamTurnString[team];
			break;
		}

		case COMPUTER_INTERRUPT_MESSAGE:
		case PLAYER_INTERRUPT_MESSAGE:
		case MILITIA_INTERRUPT_MESSAGE:
			msg = g_langRes->Message[STR_INTERRUPT];
			break;
		case AIR_RAID_TURN_MESSAGE:
			msg = TacticalStr[AIR_RAID_TURN_MESSAGE];
			break;
		case PLAYER_TURN_MESSAGE:
			msg = TeamTurnString[OUR_TEAM];
			break;

		default: abort();
	}

	INT16 sX;
	INT16 sY;
	FindFontCenterCoordinates(bar->x, bar->y, bar->w, bar->h, msg, TINYFONT1, &sX, &sY);
	MPrint(sX, sY, msg);

	SetFontDestBuffer(FRAME_BUFFER);
	SetFontShadow(DEFAULT_SHADOW);

	gfTopMessageDirty = TRUE;
}


void HandleTopMessages(void)
{
	TacticalStatusType* const ts = &gTacticalStatus;

	if (!ts->fInTopMessage)
	{
		gsVIEWPORT_WINDOW_START_Y = 0;
		return;
	}

	// ATE: If we are told to go into top message, but we have not initialized it
	// yet.  This is mostly for loading saved games.
	if (!gTopMessage.fCreated)
	{
		gfTopMessageDirty = TRUE;
		AddTopMessage((MESSAGE_TYPES)ts->ubTopMessageType);
	}

	switch (ts->ubTopMessageType)
	{
		case COMPUTER_TURN_MESSAGE:
		case COMPUTER_INTERRUPT_MESSAGE:
		case MILITIA_INTERRUPT_MESSAGE:
		case AIR_RAID_TURN_MESSAGE:
			// OK, update timer.....
			if (TIMECOUNTERDONE(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS))
			{
				RESETTIMECOUNTER(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS);

				// Update counter....
				if (ts->usTactialTurnLimitCounter < ts->usTactialTurnLimitMax)
				{
					++ts->usTactialTurnLimitCounter;
				}

				// Check if we have reach limit...
				if (ts->usTactialTurnLimitCounter > gubProgCurEnemy * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY)
				{
					ts->usTactialTurnLimitCounter = gubProgCurEnemy * PLAYER_TEAM_TIMER_TICKS_PER_ENEMY;
				}

				CreateTopMessage();
			}
			break;

		case PLAYER_TURN_MESSAGE:
		case PLAYER_INTERRUPT_MESSAGE:
			if (gGameOptions.fTurnTimeLimit &&
				!gfUserTurnRegionActive &&
				!AreWeInAUIMenu() &&
				GetJA2Clock() - ts->uiTactialTurnLimitClock > PLAYER_TEAM_TIMER_GRACE_PERIOD) // Check Grace period...
			{
				ts->uiTactialTurnLimitClock = 0;

				if (TIMECOUNTERDONE(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS))
				{
					RESETTIMECOUNTER(giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS);

					if (ts->fTactialTurnLimitStartedBeep)
					{
						if (GetJA2Clock() - gTopMessage.uiTimeSinceLastBeep > PLAYER_TEAM_TIMER_TIME_BETWEEN_BEEPS)
						{
							gTopMessage.uiTimeSinceLastBeep = GetJA2Clock();
							PlayJA2SampleFromFile(SOUNDSDIR "/turn_near_end.wav", HIGHVOLUME, 1, MIDDLEPAN);
						}
					}
					else
					{
						// OK, have we gone past the time to
						if (ts->usTactialTurnLimitMax - ts->usTactialTurnLimitCounter < PLAYER_TEAM_TIMER_TICKS_FROM_END_TO_START_BEEP)
						{
							ts->fTactialTurnLimitStartedBeep = TRUE;
							gTopMessage.uiTimeSinceLastBeep = GetJA2Clock();
						}
					}

					// Update counter....
					if (ts->usTactialTurnLimitCounter < ts->usTactialTurnLimitMax)
					{
						++ts->usTactialTurnLimitCounter;
					}

					CreateTopMessage();

					// Have we reached max?
					if (ts->usTactialTurnLimitCounter == ts->usTactialTurnLimitMax - 1)
					{
						// ATE: increase this so that we don't go into here again...
						++ts->usTactialTurnLimitCounter;
						UIHandleEndTurn(NULL);
					}
				}
			}
			break;
	}

	gsVIEWPORT_WINDOW_START_Y = 20;

	if (gfTopMessageDirty ||
		gTopMessage.sWorldRenderX != gsRenderCenterX ||
		gTopMessage.sWorldRenderY != gsRenderCenterY)
	{
		gfTopMessageDirty         = FALSE;
		gTopMessage.sWorldRenderX = gsRenderCenterX;
		gTopMessage.sWorldRenderY = gsRenderCenterY;

		SGPVSurface* const src = gTopMessage.uiSurface;
		BltVideoSurface(FRAME_BUFFER,  src, 0, 0, 0);
		BltVideoSurface(guiSAVEBUFFER, src, 0, 0, 0);
		InvalidateRegion(0, 0, SCREEN_WIDTH, 20);
	}
}


void EndTopMessage(void)
{
	if (!gTacticalStatus.fInTopMessage) return;

	gsVIEWPORT_WINDOW_START_Y     = 0;
	gTacticalStatus.fInTopMessage = FALSE;

	SetRenderFlags(RENDER_FLAG_FULL);
}


void InitEnemyUIBar( UINT8 ubNumEnemies, UINT8 ubDoneEnemies )
{
	// OK, set value
	gubProgNumEnemies = ubNumEnemies + ubDoneEnemies;
	gubProgCurEnemy = ubDoneEnemies;
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
	if (gTacticalStatus.fInTopMessage &&
		gTacticalStatus.ubTopMessageType == COMPUTER_TURN_MESSAGE)
	{
		// Update message!
		CreateTopMessage();
	}
}


void InitPlayerUIBar( BOOLEAN fInterrupt )
{
	INT8 bNumOK = 0, bNumNotOK = 0;

	if ( !gGameOptions.fTurnTimeLimit )
	{
		AddTopMessage(fInterrupt == TRUE ? PLAYER_INTERRUPT_MESSAGE : PLAYER_TURN_MESSAGE);
		return;
	}

	// OK, calculate time....
	if ( !fInterrupt || gTacticalStatus.usTactialTurnLimitMax == 0 )
	{
		gTacticalStatus.usTactialTurnLimitCounter = 0;

		CFOR_EACH_IN_TEAM(s, OUR_TEAM)
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

		gTacticalStatus.usTactialTurnLimitMax = (bNumOK * PLAYER_TEAM_TIMER_TICKS_PER_OK_MERC) +
							(bNumNotOK * PLAYER_TEAM_TIMER_TICKS_PER_NOTOK_MERC);
	}
	else
	{
		if ( gTacticalStatus.usTactialTurnLimitCounter > PLAYER_TEAM_TIMER_INTTERUPT_GRACE )
		{
			gTacticalStatus.usTactialTurnLimitCounter -= PLAYER_TEAM_TIMER_INTTERUPT_GRACE;
		}
	}

	gTacticalStatus.uiTactialTurnLimitClock = 0;
	gTacticalStatus.fTactialTurnLimitStartedBeep = FALSE;

	// RESET COIUNTER...
	RESETTIMECOUNTER( giTimerTeamTurnUpdate, PLAYER_TEAM_TIMER_SEC_PER_TICKS );


	// OK, set value
	AddTopMessage(fInterrupt != TRUE ? PLAYER_TURN_MESSAGE : PLAYER_INTERRUPT_MESSAGE);
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


ST::string GetSoldierHealthString(const SOLDIERTYPE* const s)
{
	INT32 i;
	const INT32 start = (s->bLife == s->bLifeMax ? 4 : 0);
	for (i = start; i < 6; ++i)
	{
		if (s->bLife < bHealthStrRanges[i]) break;
	}
	return zHealthStr[i];
}


static BOOLEAN gfDisplayPhysicsUI = FALSE;
static INT16   gsPhysicsImpactPointGridNo;
static INT8    gbPhysicsImpactPointLevel;
static BOOLEAN gfBadPhysicsCTGT = FALSE;

void BeginPhysicsTrajectoryUI( INT16 sGridNo, INT8 bLevel, BOOLEAN fBadCTGT )
{
	gfDisplayPhysicsUI = TRUE;
	gsPhysicsImpactPointGridNo = sGridNo;
	gbPhysicsImpactPointLevel = bLevel;
	gfBadPhysicsCTGT = fBadCTGT;
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
	gTopMessage.fCreated = FALSE;
}



static UINT32 CalcUIMessageDuration(const ST::string& str)
{
	// base + X per letter
	return( 1000 + 50 * static_cast<UINT32>(str.to_utf32().size()) );
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
	UINT32 uiClock;

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
	FLOAT dOffsetX, dOffsetY;
	FLOAT dTempX_S, dTempY_S;
	INT16 sX, sY, sXPos, sYPos;

	if ( !gfMultipurposeLocatorOn )
	{
		return;
	}

	ConvertGridNoToCenterCellXY( gsMultiPurposeLocatorGridNo, &sX, &sY );

	dOffsetX = (FLOAT)( sX - gsRenderCenterX );
	dOffsetY = (FLOAT)( sY - gsRenderCenterY );

	// Calculate guy's position
	FloatFromCellToScreenCoordinates( dOffsetX, dOffsetY, &dTempX_S, &dTempY_S );

	sXPos = ( g_ui.m_tacticalMapCenterX ) + (INT16)dTempX_S;
	sYPos = ( g_ui.m_tacticalMapCenterY ) + (INT16)dTempY_S - gpWorldLevelData[ gsMultiPurposeLocatorGridNo ].sHeight;

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

	RegisterBackgroundRectSingleFilled(sXPos, sYPos, 40, 40);

	BltVideoObject(FRAME_BUFFER, guiRADIO, gbMultiPurposeLocatorFrame, sXPos, sYPos);
}
