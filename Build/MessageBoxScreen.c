#include "Font.h"
#include "Input.h"
#include "Interface.h"
#include "Local.h"
#include "ScreenIDs.h"
#include "Timer_Control.h"
#include "Sys_Globals.h"
#include "Fade_Screen.h"
#include "SysUtil.h"
#include "MercTextBox.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Cursors.h"
#include "MessageBoxScreen.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Map_Screen_Interface.h"
#include "RenderWorld.h"
#include "Gameloop.h"
#include "English.h"
#include "GameSettings.h"
#include "Interface_Control.h"
#include "Cursor_Control.h"
#include "Laptop.h"
#include "Text.h"
#include "MapScreen.h"
#include "Overhead_Map.h"
#include "TileDef.h"
#include "Button_System.h"
#include "JAScreens.h"
#include "Video.h"

#ifdef JA2BETAVERSION
#	include "Debug.h"
#endif


#define MSGBOX_DEFAULT_WIDTH      300

#define MSGBOX_BUTTON_WIDTH        61
#define MSGBOX_BUTTON_HEIGHT       20
#define MSGBOX_BUTTON_X_SEP        15

#define MSGBOX_SMALL_BUTTON_WIDTH  31
#define MSGBOX_SMALL_BUTTON_X_SEP   8

// old mouse x and y positions
static SGPPoint pOldMousePosition;
static SGPRect  MessageBoxRestrictedCursorRegion;

// if the cursor was locked to a region
static BOOLEAN fCursorLockedToArea = FALSE;
BOOLEAN        gfInMsgBox = FALSE;

extern BOOLEAN fInMapMode;
extern BOOLEAN gfOverheadMapDirty;


static SGPRect gOldCursorLimitRectangle;


MESSAGE_BOX_STRUCT gMsgBox;
static BOOLEAN     gfNewMessageBox = FALSE;
static BOOLEAN     gfStartedFromGameScreen = FALSE;
BOOLEAN            gfStartedFromMapScreen = FALSE;
BOOLEAN            fRestoreBackgroundForMessageBox = FALSE;
BOOLEAN            gfDontOverRideSaveBuffer = TRUE;	//this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save buffer
extern void HandleTacticalUILoseCursorFromOtherScreen( );

wchar_t gzUserDefinedButton1[128];
wchar_t gzUserDefinedButton2[128];


static void ContractMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void LieMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void MsgBoxClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void NOMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void NumberedMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void OKMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void YESMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);


static INT32 MakeButton(const wchar_t* text, INT16 fore_colour, INT16 shadow_colour, INT16 x, INT16 y, GUI_CALLBACK click, UINT16 cursor)
{
	INT32 btn = CreateIconAndTextButton(gMsgBox.iButtonImages, text, FONT12ARIAL, fore_colour, shadow_colour, fore_colour, shadow_colour, x, y, MSYS_PRIORITY_HIGHEST, click);
	SetButtonCursor(btn, cursor);
	ForceButtonUnDirty(btn);
	return btn;
}


INT32 DoMessageBox(UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect)
{
	GetMousePos(&pOldMousePosition);

	//this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save buffer
	gfDontOverRideSaveBuffer = TRUE;

	SetCurrentCursorFromDatabase(CURSOR_NORMAL);

	if (gMsgBox.BackRegion.uiFlags & MSYS_REGION_EXISTS)
	{
		return 0;
	}

	// Based on style....
	UINT8  ubMercBoxBackground = BASIC_MERC_POPUP_BACKGROUND;
	UINT8  ubMercBoxBorder     = BASIC_MERC_POPUP_BORDER;
	UINT8  ubFontColor;
	UINT8  ubFontShadowColor;
	UINT16 usCursor;
	switch (ubStyle)
	{
		case MSG_BOX_BASIC_STYLE:
			ubMercBoxBackground   = DIALOG_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder       = DIALOG_MERC_POPUP_BORDER;
			gMsgBox.iButtonImages = LoadButtonImage("INTERFACE/popupbuttons.sti", -1, 0, -1, 1, -1);
			ubFontColor           = FONT_MCOLOR_WHITE;
			ubFontShadowColor     = DEFAULT_SHADOW;
			usCursor              = CURSOR_NORMAL;
			break;

		case MSG_BOX_RED_ON_WHITE:
			ubMercBoxBackground   = WHITE_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder       = RED_MERC_POPUP_BORDER;
			gMsgBox.iButtonImages = LoadButtonImage("INTERFACE/msgboxRedButtons.sti", -1, 0, -1, 1, -1);
			ubFontColor           = 2;
			ubFontShadowColor     = NO_SHADOW;
			usCursor              = CURSOR_LAPTOP_SCREEN;
			break;

		case MSG_BOX_BLUE_ON_GREY:
			ubMercBoxBackground   = GREY_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder       = BLUE_MERC_POPUP_BORDER;
			gMsgBox.iButtonImages = LoadButtonImage("INTERFACE/msgboxGreyButtons.sti", -1, 0, -1, 1, -1);
			ubFontColor           = 2;
			ubFontShadowColor     = FONT_MCOLOR_WHITE;
			usCursor              = CURSOR_LAPTOP_SCREEN;
			break;

		case MSG_BOX_IMP_STYLE:
			ubMercBoxBackground   = IMP_POPUP_BACKGROUND;
			ubMercBoxBorder       = DIALOG_MERC_POPUP_BORDER;
			gMsgBox.iButtonImages = LoadButtonImage("INTERFACE/msgboxGreyButtons.sti", -1, 0, -1, 1, -1);
			ubFontColor           = 2;
			ubFontShadowColor     = FONT_MCOLOR_WHITE;
			usCursor              = CURSOR_LAPTOP_SCREEN;
			break;

		case MSG_BOX_BASIC_SMALL_BUTTONS:
			ubMercBoxBackground   = DIALOG_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder       = DIALOG_MERC_POPUP_BORDER;
			gMsgBox.iButtonImages = LoadButtonImage("INTERFACE/popupbuttons.sti", -1, 2, -1, 3, -1);
			ubFontColor           = FONT_MCOLOR_WHITE;
			ubFontShadowColor     = DEFAULT_SHADOW;
			usCursor              = CURSOR_NORMAL;
			break;

		case MSG_BOX_LAPTOP_DEFAULT:
			ubMercBoxBackground   = LAPTOP_POPUP_BACKGROUND;
			ubMercBoxBorder       = LAPTOP_POP_BORDER;
			gMsgBox.iButtonImages = LoadButtonImage("INTERFACE/popupbuttons.sti", -1, 0, -1, 1, -1);
			ubFontColor           = FONT_MCOLOR_WHITE;
			ubFontShadowColor     = DEFAULT_SHADOW;
			usCursor              = CURSOR_LAPTOP_SCREEN;
			break;

		default:
			ubMercBoxBackground   = BASIC_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder       = BASIC_MERC_POPUP_BORDER;
			gMsgBox.iButtonImages = LoadButtonImage("INTERFACE/msgboxbuttons.sti", -1, 0, -1, 1, -1);
			ubFontColor           = FONT_MCOLOR_WHITE;
			ubFontShadowColor     = DEFAULT_SHADOW;
			usCursor              = CURSOR_NORMAL;
			break;
	}

	SGPRect	aRect;
	if (pCenteringRect != NULL)
	{
		aRect = *pCenteringRect;
	}
	else
	{
		// Use default!
		aRect.iLeft   = 0;
		aRect.iTop    = 0;
		aRect.iRight  = SCREEN_WIDTH;
		aRect.iBottom = SCREEN_HEIGHT;
	}

	// Set some values!
	gMsgBox.usFlags      = usFlags;
	gMsgBox.uiExitScreen = uiExitScreen;
	gMsgBox.ExitCallback = ReturnCallback;
	gMsgBox.fRenderBox   = TRUE;
	gMsgBox.bHandled     = 0;

	// Init message box
	UINT16 usTextBoxWidth;
	UINT16 usTextBoxHeight;
	const INT32 iId = -1;
	gMsgBox.iBoxId = PrepareMercPopupBox(iId, ubMercBoxBackground, ubMercBoxBorder, zString, MSGBOX_DEFAULT_WIDTH, 40, 10, 30, &usTextBoxWidth, &usTextBoxHeight);

	if (gMsgBox.iBoxId == -1)
	{
#ifdef JA2BETAVERSION
		AssertMsg(0, "Failed in DoMessageBox().  Probable reason is because the string was too large to fit in max message box size.");
#endif
		return 0;
	}

	// Save height,width
	gMsgBox.usWidth  = usTextBoxWidth;
	gMsgBox.usHeight = usTextBoxHeight;

	// Determine position ( centered in rect )
	gMsgBox.sX = ((aRect.iRight  - aRect.iLeft) - usTextBoxWidth)  / 2 + aRect.iLeft;
	gMsgBox.sY = ((aRect.iBottom - aRect.iTop)  - usTextBoxHeight) / 2 + aRect.iTop;

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
	if (gMsgBox.uiSaveBuffer == NO_VSURFACE) return -1;

  //Save what we have under here...
	const SGPRect r = { gMsgBox.sX, gMsgBox.sY, gMsgBox.sX + usTextBoxWidth, gMsgBox.sY + usTextBoxHeight };
	BltVideoSurface(gMsgBox.uiSaveBuffer, FRAME_BUFFER, 0, 0, &r);

	// Create top-level mouse region
	MSYS_DefineRegion(&gMsgBox.BackRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, usCursor, MSYS_NO_CALLBACK, MsgBoxClickCallback);

	if (!gGameSettings.fOptions[TOPTION_DONT_MOVE_MOUSE])
	{
		UINT32 x = gMsgBox.sX + usTextBoxWidth / 2;
		UINT32 y = gMsgBox.sY + usTextBoxHeight - 4;
		if (usFlags == MSG_BOX_FLAG_OK)
		{
			x += 27;
			y -=  6;
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

	INT16       x = gMsgBox.sX;
	const INT16 y = gMsgBox.sY + usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

	const INT16 dx = MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP;
	switch (usFlags)
	{
		case MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS:
		{
			// This is exclusive of any other buttons... no ok, no cancel, no nothing
			const INT16 dx = MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
			x += (usTextBoxWidth - (MSGBOX_SMALL_BUTTON_WIDTH + dx * 3)) / 2;

			for (UINT i = 0; i < 4; ++i)
			{
				wchar_t text[] = { '1' + i, '\0' };
				const INT32 btn = MakeButton(text, ubFontColor, ubFontShadowColor, x + dx * i, y, NumberedMsgBoxCallback, usCursor);
				gMsgBox.uiButton[i] = btn;
				MSYS_SetBtnUserData(btn, i + 1);
			}
			break;
		}

		case MSG_BOX_FLAG_OK:
			x += (usTextBoxWidth - GetWidthOfButtonPic(gMsgBox.iButtonImages)) / 2;
			gMsgBox.uiOKButton = MakeButton(pMessageStrings[MSG_OK], ubFontColor, ubFontShadowColor, x, y, OKMsgBoxCallback, usCursor);
			break;

		case MSG_BOX_FLAG_CANCEL:
			x += (usTextBoxWidth - GetWidthOfButtonPic(gMsgBox.iButtonImages)) / 2;
			gMsgBox.uiOKButton = MakeButton(pMessageStrings[MSG_CANCEL], ubFontColor, ubFontShadowColor, x, y, OKMsgBoxCallback, usCursor);
			break;

		case MSG_BOX_FLAG_YESNO:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_YES], ubFontColor, ubFontShadowColor, x,      y, YESMsgBoxCallback, usCursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_NO],  ubFontColor, ubFontShadowColor, x + dx, y, NOMsgBoxCallback,  usCursor);
			break;

		case MSG_BOX_FLAG_CONTINUESTOP:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pUpdatePanelButtons[0], ubFontColor, ubFontShadowColor, x,      y, YESMsgBoxCallback, usCursor);
			gMsgBox.uiNOButton  = MakeButton(pUpdatePanelButtons[1], ubFontColor, ubFontShadowColor, x + dx, y, NOMsgBoxCallback,  usCursor);
			break;

		case MSG_BOX_FLAG_OKCONTRACT:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_OK],     ubFontColor, ubFontShadowColor, x,      y, YESMsgBoxCallback,      usCursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_REHIRE], ubFontColor, ubFontShadowColor, x + dx, y, ContractMsgBoxCallback, usCursor);
			break;

		case MSG_BOX_FLAG_YESNOCONTRACT:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx * 2)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_YES],    ubFontColor, ubFontShadowColor, x,          y, YESMsgBoxCallback,      usCursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_NO],     ubFontColor, ubFontShadowColor, x + dx,     y, NOMsgBoxCallback,       usCursor);
			gMsgBox.uiOKButton  = MakeButton(pMessageStrings[MSG_REHIRE], ubFontColor, ubFontShadowColor, x + dx * 2, y, ContractMsgBoxCallback, usCursor);
			break;

		case MSG_BOX_FLAG_GENERICCONTRACT:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx * 2)) / 2;
			gMsgBox.uiYESButton = MakeButton(gzUserDefinedButton1,        ubFontColor, ubFontShadowColor, x,          y, YESMsgBoxCallback,      usCursor);
			gMsgBox.uiNOButton  = MakeButton(gzUserDefinedButton2,        ubFontColor, ubFontShadowColor, x + dx,     y, NOMsgBoxCallback,       usCursor);
			gMsgBox.uiOKButton  = MakeButton(pMessageStrings[MSG_REHIRE], ubFontColor, ubFontShadowColor, x + dx * 2, y, ContractMsgBoxCallback, usCursor);
			break;

		case MSG_BOX_FLAG_GENERIC:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(gzUserDefinedButton1, ubFontColor, ubFontShadowColor, x,      y, YESMsgBoxCallback, usCursor);
			gMsgBox.uiNOButton  = MakeButton(gzUserDefinedButton2, ubFontColor, ubFontShadowColor, x + dx, y, NOMsgBoxCallback,  usCursor);
			break;

		case MSG_BOX_FLAG_YESNOLIE:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx * 2)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_YES], ubFontColor, ubFontShadowColor, x,          y, YESMsgBoxCallback, usCursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_NO],  ubFontColor, ubFontShadowColor, x + dx,     y, NOMsgBoxCallback,  usCursor);
			gMsgBox.uiOKButton  = MakeButton(pMessageStrings[MSG_LIE], ubFontColor, ubFontShadowColor, x + dx * 2, y, LieMsgBoxCallback, usCursor);
			break;

		case MSG_BOX_FLAG_OKSKIP:
			x += (usTextBoxWidth - (MSGBOX_BUTTON_WIDTH + dx)) / 2;
			gMsgBox.uiYESButton = MakeButton(pMessageStrings[MSG_OK],   ubFontColor, ubFontShadowColor, x,      y, YESMsgBoxCallback, usCursor);
			gMsgBox.uiNOButton  = MakeButton(pMessageStrings[MSG_SKIP], ubFontColor, ubFontShadowColor, x + dx, y, NOMsgBoxCallback,  usCursor);
			break;
	}

	InterruptTime();
	PauseGame();
	LockPauseState(1);
	// Pause timers as well....
	PauseTime(TRUE);

  // Save mouse restriction region...
  GetRestrictedClipCursor(&gOldCursorLimitRectangle);
  FreeMouseCursor();

	gfNewMessageBox = TRUE;
	gfInMsgBox     = TRUE;

	return iId;
}


static void MsgBoxClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
}


static void OKMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_OK;
	}
}


static void YESMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_YES;
	}
}


static void NOMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_NO;
	}
}


static void ContractMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_CONTRACT;
	}
}


static void LieMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_LIE;
	}
}


static void NumberedMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSYS_GetBtnUserData(btn);
	}
}


static UINT32 ExitMsgBox(INT8 ubExitCode)
{
	// Delete popup!
	RemoveMercPopupBoxFromIndex(gMsgBox.iBoxId);
	gMsgBox.iBoxId = -1;

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

		case MSG_BOX_FLAG_YESNOCONTRACT:
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
		BltVideoSurface(FRAME_BUFFER, gMsgBox.uiSaveBuffer, gMsgBox.sX, gMsgBox.sY, NULL);
		InvalidateRegion(gMsgBox.sX, gMsgBox.sY, gMsgBox.sX + gMsgBox.usWidth, gMsgBox.sY + gMsgBox.usHeight);
	}

	fRestoreBackgroundForMessageBox = FALSE;
	gfDontOverRideSaveBuffer        = TRUE;

	if (fCursorLockedToArea)
	{
		SGPPoint pPosition;
		GetMousePos(&pPosition);

		if (pPosition.iX > MessageBoxRestrictedCursorRegion.iRight ||
				pPosition.iX > MessageBoxRestrictedCursorRegion.iLeft && pPosition.iY < MessageBoxRestrictedCursorRegion.iTop && pPosition.iY > MessageBoxRestrictedCursorRegion.iBottom)
		{
			SimulateMouseMovement(pOldMousePosition.iX, pOldMousePosition.iY);
		}

		fCursorLockedToArea = FALSE;
		RestrictMouseCursor(&MessageBoxRestrictedCursorRegion);
	}

	// Remove region
	MSYS_RemoveRegion(&gMsgBox.BackRegion);

	// Remove save buffer!
	DeleteVideoSurfaceFromIndex(gMsgBox.uiSaveBuffer);


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
	}

	if (gfFadeInitialized)
	{
		SetPendingNewScreen(FADE_SCREEN);
		return FADE_SCREEN;
	}

	return gMsgBox.uiExitScreen;
}


UINT32 MessageBoxScreenInit(void)
{
	return TRUE;
}


UINT32	MessageBoxScreenHandle(void)
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
			case MSG_BOX_FLAG_CANCEL:
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

			case MSG_BOX_FLAG_YESNOCONTRACT:
			case MSG_BOX_FLAG_GENERICCONTRACT:
			case MSG_BOX_FLAG_YESNOLIE:
				MarkAButtonDirty(gMsgBox.uiYESButton);
				MarkAButtonDirty(gMsgBox.uiNOButton);
				MarkAButtonDirty(gMsgBox.uiOKButton);
				break;
		}

		RenderMercPopUpBoxFromIndex(gMsgBox.iBoxId, gMsgBox.sX, gMsgBox.sY, FRAME_BUFFER);
		//gMsgBox.fRenderBox = FALSE;
		// ATE: Render each frame...
	}

	RenderButtons();
	EndFrameBufferRender();

	// carter, need key shortcuts for clearing up message boxes
	// Check for esc
	InputAtom InputEvent;
	while (DequeueEvent(&InputEvent))
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
					case '1': gMsgBox.bHandled = 1; break;
					case '2': gMsgBox.bHandled = 2; break;
					case '3': gMsgBox.bHandled = 3; break;
					case '4': gMsgBox.bHandled = 4; break;
				}
				break;
		}
	}

  if (gMsgBox.bHandled)
	{
		SetRenderFlags(RENDER_FLAG_FULL);
		return ExitMsgBox(gMsgBox.bHandled);
	}

	return MSG_BOX_SCREEN;
}


UINT32 MessageBoxScreenShutdown(void)
{
	return FALSE;
}


static void DoScreenIndependantMessageBoxWithRect(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect);


// a basic box that don't care what screen we came from
void DoScreenIndependantMessageBox(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	const SGPRect CenteringRect = {0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y };
	DoScreenIndependantMessageBoxWithRect(zString, usFlags, ReturnCallback, &CenteringRect);
}


// a basic box that don't care what screen we came from
static void DoUpperScreenIndependantMessageBox(wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	const SGPRect CenteringRect = {0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y / 2 };
	DoScreenIndependantMessageBoxWithRect(zString, usFlags, ReturnCallback, &CenteringRect);
}

// a basic box that don't care what screen we came from
void DoLowerScreenIndependantMessageBox(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	const SGPRect CenteringRect = {0, INV_INTERFACE_START_Y / 2, SCREEN_WIDTH, INV_INTERFACE_START_Y };
	DoScreenIndependantMessageBoxWithRect(zString, usFlags, ReturnCallback, &CenteringRect);
}


static void DoScreenIndependantMessageBoxWithRect(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect)
{
	/// which screen are we in?

	// Map Screen (excluding AI Viewer)
#ifdef JA2BETAVERSION
	if (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN && guiCurrentScreen != AIVIEWER_SCREEN)
#else
	if (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN)
#endif
	{
		// auto resolve is a special case
		if (guiCurrentScreen == AUTORESOLVE_SCREEN)
		{
			DoMessageBox(MSG_BOX_BASIC_STYLE, zString, AUTORESOLVE_SCREEN, usFlags, ReturnCallback, pCenteringRect);
		}
		else
		{
			// set up for mapscreen
			DoMapMessageBoxWithRect(MSG_BOX_BASIC_STYLE, zString, MAP_SCREEN, usFlags, ReturnCallback, pCenteringRect);
		}
	}
	else
	{
		switch (guiCurrentScreen)
		{
			case LAPTOP_SCREEN:    DoLapTopSystemMessageBoxWithRect(MSG_BOX_LAPTOP_DEFAULT, zString, LAPTOP_SCREEN,    usFlags, ReturnCallback, pCenteringRect); break;
			case SAVE_LOAD_SCREEN: DoSaveLoadMessageBoxWithRect(    MSG_BOX_BASIC_STYLE,    zString, SAVE_LOAD_SCREEN, usFlags, ReturnCallback, pCenteringRect); break;
			case OPTIONS_SCREEN:   DoOptionsMessageBoxWithRect(     MSG_BOX_BASIC_STYLE,    zString, OPTIONS_SCREEN,   usFlags, ReturnCallback, pCenteringRect); break;
			case GAME_SCREEN:      DoMessageBox(                    MSG_BOX_BASIC_STYLE,    zString, guiCurrentScreen, usFlags, ReturnCallback, pCenteringRect); break;
		}
	}
}
