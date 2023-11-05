#include "Directories.h"
#include "Font.h"
#include "Handle_UI.h"
#include "Input.h"
#include "Local.h"
#include "Timer_Control.h"
#include "Fade_Screen.h"
#include "MercTextBox.h"
#include "VSurface.h"
#include "Cursors.h"
#include "MessageBoxScreen.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Map_Screen_Interface.h"
#include "RenderWorld.h"
#include "GameLoop.h"
#include "GameSettings.h"
#include "Cursor_Control.h"
#include "Laptop.h"
#include "Text.h"
#include "MapScreen.h"
#include "Overhead_Map.h"
#include "Button_System.h"
#include "JAScreens.h"
#include "Video.h"
#include "UILayout.h"

#include <string_theory/format>
#include <string_theory/string>


#define MSGBOX_DEFAULT_WIDTH      (g_ui.m_stdScreenScale * 300)

#define MSGBOX_BUTTON_WIDTH        (g_ui.m_stdScreenScale * 61)
#define MSGBOX_BUTTON_HEIGHT       (g_ui.m_stdScreenScale * 20)
#define MSGBOX_BUTTON_X_SEP        (g_ui.m_stdScreenScale * 15)

#define MSGBOX_SMALL_BUTTON_WIDTH  (g_ui.m_stdScreenScale * 31)
#define MSGBOX_SMALL_BUTTON_X_SEP   (g_ui.m_stdScreenScale * 8)

// old mouse x and y positions
static SGPPoint pOldMousePosition;
static SGPRect  MessageBoxRestrictedCursorRegion;

// if the cursor was locked to a region
static BOOLEAN fCursorLockedToArea = FALSE;
BOOLEAN        gfInMsgBox = FALSE;


static SGPRect gOldCursorLimitRectangle;


MESSAGE_BOX_STRUCT gMsgBox;
static BOOLEAN     gfNewMessageBox = FALSE;
static BOOLEAN     gfStartedFromGameScreen = FALSE;
BOOLEAN            gfStartedFromMapScreen = FALSE;
BOOLEAN            fRestoreBackgroundForMessageBox = FALSE;
BOOLEAN            gfDontOverRideSaveBuffer = TRUE;	//this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save buffer

ST::string gzUserDefinedButton1;
ST::string gzUserDefinedButton2;


static void ContractMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason);
static void LieMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason);
static void NOMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason);
static void NumberedMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason);
static void OKMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason);
static void YESMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason);


static GUIButtonRef MakeButton(const ST::string& text, UINT32 fore_color, UINT32 shadow_color, INT16 x, INT16 y, GUI_CALLBACK click, UINT16 cursor)
{
	GUIButtonRef const btn = CreateIconAndTextButton(gMsgBox.iButtonImages, text, FONT12ARIAL, fore_color, shadow_color, fore_color, shadow_color, x, y, MSYS_PRIORITY_HIGHEST, click);
	btn->SetCursor(cursor);
	ForceButtonUnDirty(btn);
	return btn;
}


struct MessageBoxStyle
{
	MercPopUpBackground background;
	MercPopUpBorder     border;
	char const*         btn_image;
	INT32               btn_off;
	INT32               btn_on;
	UINT32              font_color;
	UINT32              shadow_color;
	UINT16              cursor;
};


static MessageBoxStyle const g_msg_box_style[] =
{
	{ DIALOG_MERC_POPUP_BACKGROUND, DIALOG_MERC_POPUP_BORDER, INTERFACEDIR "/popupbuttons.sti",      0, 1, FONT_MCOLOR_WHITE, DEFAULT_SHADOW,    CURSOR_NORMAL        }, // MSG_BOX_BASIC_STYLE
	{ WHITE_MERC_POPUP_BACKGROUND,  RED_MERC_POPUP_BORDER,    INTERFACEDIR "/msgboxredbuttons.sti",  0, 1, FONT_MCOLOR_RED,   NO_SHADOW,         CURSOR_LAPTOP_SCREEN }, // MSG_BOX_RED_ON_WHITE
	{ GREY_MERC_POPUP_BACKGROUND,   BLUE_MERC_POPUP_BORDER,   INTERFACEDIR "/msgboxgreybuttons.sti", 0, 1, FONT_MCOLOR_RED,   FONT_MCOLOR_WHITE, CURSOR_LAPTOP_SCREEN }, // MSG_BOX_BLUE_ON_GREY
	{ DIALOG_MERC_POPUP_BACKGROUND, DIALOG_MERC_POPUP_BORDER, INTERFACEDIR "/popupbuttons.sti",      2, 3, FONT_MCOLOR_WHITE, DEFAULT_SHADOW,    CURSOR_NORMAL        }, // MSG_BOX_BASIC_SMALL_BUTTONS
	{ IMP_POPUP_BACKGROUND,         DIALOG_MERC_POPUP_BORDER, INTERFACEDIR "/msgboxgreybuttons.sti", 0, 1, FONT_MCOLOR_RED,   FONT_MCOLOR_WHITE, CURSOR_LAPTOP_SCREEN }, // MSG_BOX_IMP_STYLE
	{ LAPTOP_POPUP_BACKGROUND,      LAPTOP_POP_BORDER,        INTERFACEDIR "/popupbuttons.sti",      0, 1, FONT_MCOLOR_WHITE, DEFAULT_SHADOW,    CURSOR_LAPTOP_SCREEN }  // MSG_BOX_LAPTOP_DEFAULT
};


static MessageBoxStyle const g_msg_box_style_default = {
	BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER,
	INTERFACEDIR "/msgboxbuttons.sti", 0, 1,
	FONT_MCOLOR_WHITE, DEFAULT_SHADOW, CURSOR_NORMAL
};

void DoMessageBox(MessageBoxStyleID ubStyle, const ST::string str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPBox* centering_rect)
{
	GetMousePos(&pOldMousePosition);

	//this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save buffer
	gfDontOverRideSaveBuffer = TRUE;

	SetCurrentCursorFromDatabase(CURSOR_NORMAL);

	if (gMsgBox.BackRegion.uiFlags & MSYS_REGION_EXISTS) return;

	MessageBoxStyle const& style = ubStyle < lengthof(g_msg_box_style) ?
		g_msg_box_style[ubStyle] : g_msg_box_style_default;

	// Set some values!
	gMsgBox.usFlags      = usFlags;
	gMsgBox.uiExitScreen = uiExitScreen;
	gMsgBox.ExitCallback = ReturnCallback;
	gMsgBox.fRenderBox   = TRUE;
	gMsgBox.bHandled     = MSG_BOX_RETURN_NONE;

	// Init message box
	UINT16 usTextBoxWidth;
	UINT16 usTextBoxHeight;
	gMsgBox.box = PrepareMercPopupBox(0, style.background, style.border, str,
		MSGBOX_DEFAULT_WIDTH, g_ui.m_stdScreenScale * 40, g_ui.m_stdScreenScale * 10, g_ui.m_stdScreenScale * 30,
		&usTextBoxWidth, &usTextBoxHeight);

	// Save height,width
	gMsgBox.usWidth  = usTextBoxWidth;
	gMsgBox.usHeight = usTextBoxHeight;

	// Determine position (centered in rect)
	if (centering_rect)
	{
		gMsgBox.uX = centering_rect->x + (centering_rect->w  - usTextBoxWidth)  / 2;
		gMsgBox.uY = centering_rect->y + (centering_rect->h  - usTextBoxHeight) / 2;
	}
	else
	{
		gMsgBox.uX = (SCREEN_WIDTH  - usTextBoxWidth)  / 2;
		gMsgBox.uY = (SCREEN_HEIGHT - usTextBoxHeight) / 2;
	}

	if (guiCurrentScreen == GAME_SCREEN)
	{
		gfStartedFromGameScreen = TRUE;
	}

	if (fInMapMode)
	{
		gfStartedFromMapScreen = TRUE;
		fMapPanelDirty         = TRUE;
	}


	// Set pending screen
	SetPendingNewScreen(MSG_BOX_SCREEN);

	// Init save buffer
	gMsgBox.uiSaveBuffer = AddVideoSurface(usTextBoxWidth, usTextBoxHeight, PIXEL_DEPTH);

	//Save what we have under here...
	SGPBox const r = { gMsgBox.uX, gMsgBox.uY, usTextBoxWidth, usTextBoxHeight };
	BltVideoSurface(gMsgBox.uiSaveBuffer, FRAME_BUFFER, 0, 0, &r);

	UINT16 const cursor = style.cursor;
	// Create top-level mouse region
	MSYS_DefineRegion(&gMsgBox.BackRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, cursor, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	if (!gGameSettings.fOptions[TOPTION_DONT_MOVE_MOUSE])
	{
		UINT32 x = gMsgBox.uX + usTextBoxWidth / 2;
		UINT32 y = gMsgBox.uY + usTextBoxHeight - 4;
		if (usFlags == MSG_BOX_FLAG_OK)
		{
			x += g_ui.m_stdScreenScale * 27;
			y -= g_ui.m_stdScreenScale *  6;
		}
		SimulateMouseMovement(x, y);
	}

	// findout if cursor locked, if so, store old params and store, restore when done
	if (IsCursorRestricted())
	{
		fCursorLockedToArea = TRUE;
		GetRestrictedClipCursor(&MessageBoxRestrictedCursorRegion);
		FreeMouseCursor();
	}

	UINT16       x = gMsgBox.uX;
	const UINT16 y = gMsgBox.uY + usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

	gMsgBox.iButtonImages = LoadButtonImage(style.btn_image, style.btn_off, style.btn_on);

	INT16 const dx            = MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP;
	UINT32 const font_color   = style.font_color;
	UINT32 const shadow_color = style.shadow_color;
	switch (usFlags)
	{
		case MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS:
		{
			// This is exclusive of any other buttons... no ok, no cancel, no nothing
			const INT16 dx = MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
			x += (usTextBoxWidth - (MSGBOX_SMALL_BUTTON_WIDTH + dx * 3)) / 2;

			for (UINT8 i = 0; i < 4; ++i)
			{
				ST::string text = ST::format("{}", i + 1);
				GUIButtonRef const btn = MakeButton(text, font_color, shadow_color, x + dx * i, y, NumberedMsgBoxCallback, cursor);
				gMsgBox.uiButton[i] = btn;
				btn->SetUserData(i + 1);
			}
			break;
		}

		case MSG_BOX_FLAG_OK:
			x += (usTextBoxWidth - GetDimensionsOfButtonPic(gMsgBox.iButtonImages)->w) / 2;
			gMsgBox.uiOKButton = MakeButton(pMessageStrings[MSG_OK], font_color, shadow_color, x, y, OKMsgBoxCallback, cursor);
			break;

		case MSG_BOX_FLAG_YESNO:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_YES], font_color, shadow_color, x,      y, YESMsgBoxCallback, cursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_NO],  font_color, shadow_color, x + dx, y, NOMsgBoxCallback,  cursor);
			break;

		case MSG_BOX_FLAG_CONTINUESTOP:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pUpdatePanelButtons[0], font_color, shadow_color, x,      y, YESMsgBoxCallback, cursor);
			gMsgBox.uiNOButton  = MakeButton(pUpdatePanelButtons[1], font_color, shadow_color, x + dx, y, NOMsgBoxCallback,  cursor);
			break;

		case MSG_BOX_FLAG_OKCONTRACT:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_OK],     font_color, shadow_color, x,      y, YESMsgBoxCallback,      cursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_REHIRE], font_color, shadow_color, x + dx, y, ContractMsgBoxCallback, cursor);
			break;

		case MSG_BOX_FLAG_GENERICCONTRACT:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx * 2)) / 2;
			gMsgBox.uiYESButton = MakeButton(gzUserDefinedButton1,        font_color, shadow_color, x,          y, YESMsgBoxCallback,      cursor);
			gMsgBox.uiNOButton  = MakeButton(gzUserDefinedButton2,        font_color, shadow_color, x + dx,     y, NOMsgBoxCallback,       cursor);
			gMsgBox.uiOKButton  = MakeButton(pMessageStrings[MSG_REHIRE], font_color, shadow_color, x + dx * 2, y, ContractMsgBoxCallback, cursor);
			break;

		case MSG_BOX_FLAG_GENERIC:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(gzUserDefinedButton1, font_color, shadow_color, x,      y, YESMsgBoxCallback, cursor);
			gMsgBox.uiNOButton  = MakeButton(gzUserDefinedButton2, font_color, shadow_color, x + dx, y, NOMsgBoxCallback,  cursor);
			break;

		case MSG_BOX_FLAG_YESNOLIE:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx * 2)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_YES], font_color, shadow_color, x,          y, YESMsgBoxCallback, cursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_NO],  font_color, shadow_color, x + dx,     y, NOMsgBoxCallback,  cursor);
			gMsgBox.uiOKButton  = MakeButton(pMessageStrings[MSG_LIE], font_color, shadow_color, x + dx * 2, y, LieMsgBoxCallback, cursor);
			break;

		case MSG_BOX_FLAG_OKSKIP:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_OK],   font_color, shadow_color, x,      y, YESMsgBoxCallback, cursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_SKIP], font_color, shadow_color, x + dx, y, NOMsgBoxCallback,  cursor);
			break;
	}

	InterruptTime();
	PauseGame();
	LockPauseState(LOCK_PAUSE_MSGBOX);
	// Pause timers as well....
	PauseTime(TRUE);

	// Save mouse restriction region...
	GetRestrictedClipCursor(&gOldCursorLimitRectangle);
	FreeMouseCursor();

	gfNewMessageBox = TRUE;
	gfInMsgBox     = TRUE;
}


static void OKMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_OK;
	}
}


static void YESMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_YES;
	}
}


static void NOMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_NO;
	}
}


static void ContractMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_CONTRACT;
	}
}


static void LieMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_LIE;
	}
}


static void NumberedMsgBoxCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gMsgBox.bHandled = static_cast<MessageBoxReturnValue>(btn->GetUserData());
	}
}


static ScreenID ExitMsgBox(MessageBoxReturnValue const ubExitCode)
{
	RemoveMercPopupBox(gMsgBox.box);
	gMsgBox.box = 0;

	//Delete buttons!
	switch (gMsgBox.usFlags)
	{
		case MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS:
			RemoveButton(gMsgBox.uiButton[0]);
			RemoveButton(gMsgBox.uiButton[1]);
			RemoveButton(gMsgBox.uiButton[2]);
			RemoveButton(gMsgBox.uiButton[3]);
			break;

		case MSG_BOX_FLAG_OK:
			RemoveButton(gMsgBox.uiOKButton);
			break;

		case MSG_BOX_FLAG_YESNO:
		case MSG_BOX_FLAG_OKCONTRACT:
		case MSG_BOX_FLAG_GENERIC:
		case MSG_BOX_FLAG_CONTINUESTOP:
		case MSG_BOX_FLAG_OKSKIP:
			RemoveButton(gMsgBox.uiYESButton);
			RemoveButton(gMsgBox.uiNOButton);
			break;

		case MSG_BOX_FLAG_GENERICCONTRACT:
		case MSG_BOX_FLAG_YESNOLIE:
			RemoveButton(gMsgBox.uiYESButton);
			RemoveButton(gMsgBox.uiNOButton);
			RemoveButton(gMsgBox.uiOKButton);
			break;
	}

	// Delete button images
	UnloadButtonImage(gMsgBox.iButtonImages);

	// Unpause game....
	UnLockPauseState();
	UnPauseGame();
	// UnPause timers as well....
	PauseTime(FALSE);

	// Restore mouse restriction region...
	RestrictMouseCursor(&gOldCursorLimitRectangle);

	gfInMsgBox = FALSE;

	// Call done callback!
	if (gMsgBox.ExitCallback != NULL) gMsgBox.ExitCallback(ubExitCode);

	//if you are in a non gamescreen and DONT want the msg box to use the save buffer, unset gfDontOverRideSaveBuffer in your callback
	if ((gMsgBox.uiExitScreen != GAME_SCREEN || fRestoreBackgroundForMessageBox) && gfDontOverRideSaveBuffer)
	{
		// restore what we have under here...
		BltVideoSurface(FRAME_BUFFER, gMsgBox.uiSaveBuffer, gMsgBox.uX, gMsgBox.uY, NULL);
		InvalidateRegion(gMsgBox.uX, gMsgBox.uY, gMsgBox.uX + gMsgBox.usWidth, gMsgBox.uY + gMsgBox.usHeight);
	}

	fRestoreBackgroundForMessageBox = FALSE;
	gfDontOverRideSaveBuffer        = TRUE;

	if (fCursorLockedToArea)
	{
		SGPPoint pPosition;
		GetMousePos(&pPosition);

		if (pPosition.iX > MessageBoxRestrictedCursorRegion.iRight ||
				(pPosition.iX > MessageBoxRestrictedCursorRegion.iLeft && pPosition.iY < MessageBoxRestrictedCursorRegion.iTop && pPosition.iY > MessageBoxRestrictedCursorRegion.iBottom))
		{
			SimulateMouseMovement(pOldMousePosition.iX, pOldMousePosition.iY);
		}

		fCursorLockedToArea = FALSE;
		RestrictMouseCursor(&MessageBoxRestrictedCursorRegion);
	}

	MSYS_RemoveRegion(&gMsgBox.BackRegion);
	DeleteVideoSurface(gMsgBox.uiSaveBuffer);

	switch (gMsgBox.uiExitScreen)
	{
		case GAME_SCREEN:
			if (InOverheadMap())
			{
				gfOverheadMapDirty = TRUE;
			}
			else
			{
				SetRenderFlags(RENDER_FLAG_FULL);
			}
			break;

		case MAP_SCREEN:
			fMapPanelDirty = TRUE;
			break;
		default:
			break;
	}

	if (gfFadeInitialized)
	{
		SetPendingNewScreen(FADE_SCREEN);
		return FADE_SCREEN;
	}

	return gMsgBox.uiExitScreen;
}


ScreenID MessageBoxScreenHandle(void)
{
	if (gfNewMessageBox)
	{
		// If in game screen....
		if (gfStartedFromGameScreen || gfStartedFromMapScreen)
		{
			if (gfStartedFromGameScreen)
			{
				HandleTacticalUILoseCursorFromOtherScreen();
			}
			else
			{
				HandleMAPUILoseCursorFromOtherScreen();
			}

			gfStartedFromGameScreen = FALSE;
			gfStartedFromMapScreen  = FALSE;
		}

		gfNewMessageBox = FALSE;

		return MSG_BOX_SCREEN;
	}

	UnmarkButtonsDirty();

	// Render the box!
	if (gMsgBox.fRenderBox)
	{
		switch (gMsgBox.usFlags)
		{
			case MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS:
				MarkAButtonDirty(gMsgBox.uiButton[0]);
				MarkAButtonDirty(gMsgBox.uiButton[1]);
				MarkAButtonDirty(gMsgBox.uiButton[2]);
				MarkAButtonDirty(gMsgBox.uiButton[3]);
				break;

			case MSG_BOX_FLAG_OK:
				MarkAButtonDirty(gMsgBox.uiOKButton);
				break;

			case MSG_BOX_FLAG_YESNO:
			case MSG_BOX_FLAG_OKCONTRACT:
			case MSG_BOX_FLAG_GENERIC:
			case MSG_BOX_FLAG_CONTINUESTOP:
			case MSG_BOX_FLAG_OKSKIP:
				MarkAButtonDirty(gMsgBox.uiYESButton);
				MarkAButtonDirty(gMsgBox.uiNOButton);
				break;

			case MSG_BOX_FLAG_GENERICCONTRACT:
			case MSG_BOX_FLAG_YESNOLIE:
				MarkAButtonDirty(gMsgBox.uiYESButton);
				MarkAButtonDirty(gMsgBox.uiNOButton);
				MarkAButtonDirty(gMsgBox.uiOKButton);
				break;
		}

		RenderMercPopUpBox(gMsgBox.box, gMsgBox.uX, gMsgBox.uY, FRAME_BUFFER);
		//gMsgBox.fRenderBox = FALSE;
		// ATE: Render each frame...
	}

	RenderButtons();

	// carter, need key shortcuts for clearing up message boxes
	// Check for esc
	InputAtom InputEvent;
	while (DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS))
	{
		if (InputEvent.usEvent != KEY_UP) continue;

		switch (gMsgBox.usFlags)
		{
			case MSG_BOX_FLAG_YESNO:
				switch (InputEvent.usParam)
				{
					case 'n':
					case SDLK_ESCAPE: gMsgBox.bHandled = MSG_BOX_RETURN_NO;  break;
					case 'y':
					case SDLK_RETURN: gMsgBox.bHandled = MSG_BOX_RETURN_YES; break;
				}
				break;

			case MSG_BOX_FLAG_OK:
				switch (InputEvent.usParam)
				{
					case 'o':
					case SDLK_RETURN: gMsgBox.bHandled = MSG_BOX_RETURN_OK; break;
				}
				break;

			case MSG_BOX_FLAG_CONTINUESTOP:
				switch (InputEvent.usParam)
				{
					case SDLK_RETURN: gMsgBox.bHandled = MSG_BOX_RETURN_OK; break;
				}
				break;

			case MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS:
				switch (InputEvent.usParam)
				{
					case '1': gMsgBox.bHandled = MSG_BOX_RETURN_1; break;
					case '2': gMsgBox.bHandled = MSG_BOX_RETURN_2; break;
					case '3': gMsgBox.bHandled = MSG_BOX_RETURN_3; break;
					case '4': gMsgBox.bHandled = MSG_BOX_RETURN_4; break;
				}
				break;
			default:
				break;
		}
	}

	if (gMsgBox.bHandled != MSG_BOX_RETURN_NONE)
	{
		SetRenderFlags(RENDER_FLAG_FULL);
		return ExitMsgBox(gMsgBox.bHandled);
	}

	return MSG_BOX_SCREEN;
}


void MessageBoxScreenShutdown()
{
	if (!gMsgBox.box) return;
	RemoveMercPopupBox(gMsgBox.box);
	gMsgBox.box = 0;
}

// a basic box that don't care what screen we came from
void DoScreenIndependantMessageBox(const ST::string& msg, MessageBoxFlags flags, MSGBOX_CALLBACK callback)
{
	SGPBox const centering_rect = {0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y };
	switch (ScreenID const screen = guiCurrentScreen)
	{
		case AUTORESOLVE_SCREEN:
		case GAME_SCREEN:        DoMessageBox(                    MSG_BOX_BASIC_STYLE,    msg, screen, flags, callback, &centering_rect); break;
		case LAPTOP_SCREEN:      DoLapTopSystemMessageBoxWithRect(MSG_BOX_LAPTOP_DEFAULT, msg, screen, flags, callback, &centering_rect); break;
		case MAP_SCREEN:         DoMapMessageBoxWithRect(         MSG_BOX_BASIC_STYLE,    msg, screen, flags, callback, &centering_rect); break;
		case OPTIONS_SCREEN:     DoOptionsMessageBoxWithRect(                             msg, screen, flags, callback, &centering_rect); break;
		case SAVE_LOAD_SCREEN:   DoSaveLoadMessageBoxWithRect(                            msg, screen, flags, callback, &centering_rect); break;
		default:
			break;
	}
}
