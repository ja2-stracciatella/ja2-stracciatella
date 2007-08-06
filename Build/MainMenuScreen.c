#include "Button_System.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "Debug.h"
#include "English.h"
#include "Font_Control.h"
#include "GameSettings.h"
#include "GameVersion.h"
#include "Gameloop.h"
#include "JA2_Splash.h"
#include "JAScreens.h"
#include "Local.h"
#include "MainMenuScreen.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Music_Control.h"
#include "Options_Screen.h"
#include "Render_Dirty.h"
#include "SGP.h"
#include "SaveLoadScreen.h"
#include "ScreenIDs.h"
#include "SysUtil.h"
#include "Sys_Globals.h"
#include "Text.h"
#include "Timer_Control.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"
#include "WordWrap.h"


//#define TESTFOREIGNFONTS

// MENU ITEMS
enum
{
	NEW_GAME,
	LOAD_GAME,
	PREFERENCES,
	CREDITS,
	QUIT,
	NUM_MENU_ITEMS
};

#if defined TESTFOREIGNFONTS
#	define MAINMENU_Y         0
#	define MAINMENU_Y_SPACE  18
#else
#	define MAINMENU_Y       277
#	define MAINMENU_Y_SPACE  37
#endif


static INT32 iMenuImages[NUM_MENU_ITEMS];
static INT32 iMenuButtons[NUM_MENU_ITEMS];

static UINT16 gusMainMenuButtonWidths[NUM_MENU_ITEMS];

static UINT32 guiMainMenuBackGroundImage;
static UINT32 guiJa2LogoImage;

static MOUSE_REGION gBackRegion;
static INT8         gbHandledMainMenu = 0;
static BOOLEAN      fInitialRender = FALSE;

static BOOLEAN gfMainMenuScreenEntry = FALSE;
static BOOLEAN gfMainMenuScreenExit = FALSE;

static UINT32 guiMainMenuExitScreen = MAINMENU_SCREEN;


extern BOOLEAN gfLoadGameUponEntry;


UINT32 MainMenuScreenInit(void)
{
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Version Label: %ls", zVersionLabel));
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Version #:     %s",  czVersionNumber));
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Tracking #:    %ls", zTrackingNumber));
	return TRUE;
}


static void ExitMainMenu(void);
static void HandleMainMenuInput(void);
static void HandleMainMenuScreen(void);
static void RenderMainMenu(void);
static void RestoreButtonBackGrounds(void);


UINT32 MainMenuScreenHandle(void)
{
	if (guiSplashStartTime + 4000 > GetJA2Clock())
	{
		SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
		SetMusicMode(MUSIC_NONE);
		return MAINMENU_SCREEN; // The splash screen hasn't been up long enough yet.
	}
	if (guiSplashFrameFade)
	{
		// Fade the splash screen.
		if (guiSplashFrameFade > 2)
		{
			ShadowVideoSurfaceRectUsingLowPercentTable(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		}
		else if (guiSplashFrameFade > 1)
		{
			ColorFillVideoSurfaceArea(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
		}
		else
		{
			SetMusicMode(MUSIC_MAIN_MENU);
		}

		guiSplashFrameFade--;

		InvalidateScreen();
		EndFrameBufferRender();

		SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);

		return MAINMENU_SCREEN;
	}

	SetCurrentCursorFromDatabase(CURSOR_NORMAL);

	if (gfMainMenuScreenEntry)
	{
		InitMainMenu();
		gfMainMenuScreenEntry = FALSE;
		gfMainMenuScreenExit  = FALSE;
		guiMainMenuExitScreen = MAINMENU_SCREEN;
		SetMusicMode(MUSIC_MAIN_MENU);
	}


	if (fInitialRender)
	{
		ClearMainMenu();
		RenderMainMenu();

		fInitialRender = FALSE;
	}

	RestoreButtonBackGrounds();

	// Render buttons
	for (UINT32 cnt = 0; cnt < NUM_MENU_ITEMS; ++cnt)
	{
		MarkAButtonDirty(iMenuButtons[cnt]);
	}

	RenderButtons();

	EndFrameBufferRender();

	HandleMainMenuInput();
	HandleMainMenuScreen();

	if (gfMainMenuScreenExit)
	{
		ExitMainMenu();
		gfMainMenuScreenExit  = FALSE;
		gfMainMenuScreenEntry = TRUE;
	}

	if (guiMainMenuExitScreen != MAINMENU_SCREEN) gfMainMenuScreenEntry = TRUE;

	return guiMainMenuExitScreen;
}


UINT32 MainMenuScreenShutdown(void)
{
	return FALSE;
}


static void HandleMainMenuScreen(void)
{
	if (gbHandledMainMenu == 0) return;

	// Exit according to handled value!
	switch (gbHandledMainMenu)
	{
		case QUIT:
			gfMainMenuScreenExit = TRUE;

#if defined JA2DEMO
			// Goto ad pages
			SetPendingNewScreen(DEMO_EXIT_SCREEN);
			SetMusicMode(MUSIC_MAIN_MENU);
			FadeOutNextFrame();
#else
			gfProgramIsRunning = FALSE;
#endif
			break;

		case LOAD_GAME:
			// Select the game which is to be restored
			guiPreviousOptionScreen = guiCurrentScreen;
			guiMainMenuExitScreen   = SAVE_LOAD_SCREEN;
			gbHandledMainMenu       = 0;
			gfSaveGame              = FALSE;
			gfMainMenuScreenExit    = TRUE;
			break;

		case PREFERENCES:
			guiPreviousOptionScreen = guiCurrentScreen;
			guiMainMenuExitScreen   = OPTIONS_SCREEN;
			gbHandledMainMenu       = 0;
			gfMainMenuScreenExit    = TRUE;
			break;

		case CREDITS:
			guiMainMenuExitScreen = CREDIT_SCREEN;
			gbHandledMainMenu     = 0;
			gfMainMenuScreenExit  = TRUE;
			break;
	}
}


static void CreateDestroyBackGroundMouseMask(BOOLEAN fCreate);
static BOOLEAN CreateDestroyMainMenuButtons(BOOLEAN fCreate);


BOOLEAN InitMainMenu(void)
{
	// Check to see whether saved game files exist
	InitSaveGameArray();

	// Create the background mouse mask
	CreateDestroyBackGroundMouseMask(TRUE);

	CreateDestroyMainMenuButtons(TRUE);

	// Load background graphic and add it
	guiMainMenuBackGroundImage = AddVideoObjectFromFile("LOADSCREENS/MainMenuBackGround.sti");
	CHECKF(guiMainMenuBackGroundImage != NO_VOBJECT);

	// Load ja2 logo graphic and add it
	guiJa2LogoImage = AddVideoObjectFromFile("LOADSCREENS/Ja2Logo.sti");
	CHECKF(guiJa2LogoImage != NO_VOBJECT);

	// If there are no saved games, disable the button
	if (!IsThereAnySavedGameFiles()) DisableButton(iMenuButtons[LOAD_GAME]);

#if defined JA2DEMO
	DisableButton(iMenuButtons[CREDITS]);
#endif

	gbHandledMainMenu = 0;
	fInitialRender    = TRUE;

	SetPendingNewScreen(MAINMENU_SCREEN);
	guiMainMenuExitScreen = MAINMENU_SCREEN;

	InitGameOptions();

	DequeueAllKeyBoardEvents();

	return TRUE;
}


static void ExitMainMenu(void)
{
	CreateDestroyBackGroundMouseMask(FALSE);
	CreateDestroyMainMenuButtons(FALSE);
	DeleteVideoObjectFromIndex(guiMainMenuBackGroundImage);
	DeleteVideoObjectFromIndex(guiJa2LogoImage);
	gMsgBox.uiExitScreen = MAINMENU_SCREEN;
}


static void SetMainMenuExitScreen(UINT32 uiNewScreen);


static void MenuButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		INT8 bID = MSYS_GetBtnUserData(btn);

		gbHandledMainMenu = bID;
		RenderMainMenu();

		switch (gbHandledMainMenu)
		{
			case NEW_GAME:  SetMainMenuExitScreen(GAME_INIT_OPTIONS_SCREEN); break;
			case LOAD_GAME: if (gfKeyState[ALT]) gfLoadGameUponEntry = TRUE; break;
		}
	}
}


static void HandleMainMenuInput(void)
{
	InputAtom  InputEvent;

	// Check for esc
	while (DequeueEvent(&InputEvent) == TRUE)
  {
    if( InputEvent.usEvent == KEY_UP )
		{
			switch( InputEvent.usParam )
			{
/*
				case SDLK_ESCAPE: gbHandledMainMenu = QUIT; break;
*/

#ifdef JA2TESTVERSION
				case 'q':
					gbHandledMainMenu    = NEW_GAME;
					gfMainMenuScreenExit = TRUE;
					SetMainMenuExitScreen(INIT_SCREEN);
					break;

				case 'i':
					SetPendingNewScreen(INTRO_SCREEN);
					gfMainMenuScreenExit = TRUE;
					break;
#endif

				case 'c':
					if (gfKeyState[ALT]) gfLoadGameUponEntry = TRUE;
					gbHandledMainMenu = LOAD_GAME;
					break;

				case 'o':
					gbHandledMainMenu = PREFERENCES;
					break;

				case 's':
					gbHandledMainMenu = CREDITS;
					break;
			}
		}
	}
}


void ClearMainMenu(void)
{
	FillSurface(FRAME_BUFFER, 0);
	InvalidateScreen();
}


static void SelectMainMenuBackGroundRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
}


static void SetMainMenuExitScreen(UINT32 uiNewScreen)
{
	guiMainMenuExitScreen = uiNewScreen;

	// Remove the background region
	CreateDestroyBackGroundMouseMask(FALSE);

	gfMainMenuScreenExit = TRUE;
}


static void CreateDestroyBackGroundMouseMask(BOOLEAN fCreate)
{
	static BOOLEAN fRegionCreated = FALSE;

	if (fCreate)
	{
		if (fRegionCreated) return;
		MSYS_DefineRegion(&gBackRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectMainMenuBackGroundRegionCallBack);
		fRegionCreated = TRUE;
	}
	else
	{
		if (!fRegionCreated) return;
		MSYS_RemoveRegion(&gBackRegion);
		fRegionCreated = FALSE;
	}
}


static BOOLEAN CreateDestroyMainMenuButtons(BOOLEAN fCreate)
{
	static BOOLEAN fButtonsCreated = FALSE;

	if (fCreate)
	{
		if (fButtonsCreated) return TRUE;

		// Reset the variable that allows the user to ALT click on the continue save btn to load the save instantly
		gfLoadGameUponEntry = FALSE;

		// Load button images
		SGPFILENAME filename;
		GetMLGFilename(filename, MLG_TITLETEXT);

		INT32 Slot;
#if defined JA2DEMO
		Slot = 17;
#else
		Slot = 0;
#endif
		iMenuImages[NEW_GAME]    = LoadButtonImage(filename, Slot, Slot, Slot + 1, Slot + 2, -1);
		iMenuImages[LOAD_GAME]   = UseLoadedButtonImage(iMenuImages[NEW_GAME],  6,  3,  4,  5, -1);
		iMenuImages[PREFERENCES] = UseLoadedButtonImage(iMenuImages[NEW_GAME],  7,  7,  8,  9, -1);
		iMenuImages[CREDITS]     = UseLoadedButtonImage(iMenuImages[NEW_GAME], 13, 10, 11, 12, -1);
		iMenuImages[QUIT]        = UseLoadedButtonImage(iMenuImages[NEW_GAME], 14, 14, 15, 16, -1);

		for (UINT32 cnt = 0; cnt < NUM_MENU_ITEMS; ++cnt)
		{
			switch (cnt)
			{
				case NEW_GAME:    gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic(iMenuImages[cnt], Slot); break;
				case LOAD_GAME:   gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic(iMenuImages[cnt],  3);   break;
				case PREFERENCES: gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic(iMenuImages[cnt],  7);   break;
				case CREDITS:     gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic(iMenuImages[cnt], 10);   break;
				case QUIT:        gusMainMenuButtonWidths[cnt] = GetWidthOfButtonPic(iMenuImages[cnt], 15);   break;
			}
			iMenuButtons[cnt] = QuickCreateButton(iMenuImages[cnt], (SCREEN_WIDTH - gusMainMenuButtonWidths[cnt]) / 2, MAINMENU_Y + cnt * MAINMENU_Y_SPACE, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST, DEFAULT_MOVE_CALLBACK, MenuButtonCallback);
			if (iMenuButtons[cnt] == -1) return FALSE;
			MSYS_SetBtnUserData(iMenuButtons[cnt], cnt);
		}

		fButtonsCreated = TRUE;
	}
	else
	{
		if (!fButtonsCreated) return TRUE;

		// Delete images/buttons
		for (UINT32 cnt = 0; cnt < NUM_MENU_ITEMS; ++cnt)
		{
			RemoveButton(iMenuButtons[cnt]);
			UnloadButtonImage(iMenuImages[cnt]);
		}
		fButtonsCreated = FALSE;
	}

	return TRUE;
}


static void RenderMainMenu(void)
{
  HVOBJECT hPixHandle;

	//Get and display the background image
	hPixHandle = GetVideoObject(guiMainMenuBackGroundImage);
  BltVideoObject(guiSAVEBUFFER, hPixHandle, 0, 0, 0);
  BltVideoObject(FRAME_BUFFER,  hPixHandle, 0, 0, 0);

	hPixHandle = GetVideoObject(guiJa2LogoImage);
  BltVideoObject(FRAME_BUFFER,  hPixHandle, 0, 188, 15);
  BltVideoObject(guiSAVEBUFFER, hPixHandle, 0, 188, 15);

#ifdef TESTFOREIGNFONTS
	DrawTextToScreen(L"LARGEFONT1: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",            0, 105, 640, LARGEFONT1,            FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"SMALLFONT1: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",            0, 125, 640, SMALLFONT1,            FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"TINYFONT1: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",             0, 145, 640, TINYFONT1,             FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT12POINT1: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",          0, 165, 640, FONT12POINT1,          FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"COMPFONT: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",              0, 185, 640, COMPFONT,              FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"SMALLCOMPFONT: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",         0, 205, 640, SMALLCOMPFONT,         FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"MILITARYFONT: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",          0, 265, 640, MILITARYFONT1,         FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT10ARIAL: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",           0, 285, 640, FONT10ARIAL,           FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT14ARIAL: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",           0, 305, 640, FONT14ARIAL,           FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT12ARIAL: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",           0, 325, 640, FONT12ARIAL,           FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT10ARIALBOLD: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",       0, 345, 640, FONT10ARIALBOLD,       FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"BLOCKFONT: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",             0, 365, 640, BLOCKFONT,             FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"BLOCKFONT2: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",            0, 385, 640, BLOCKFONT2,            FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT12ARIALFIXEDWIDTH: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî", 0, 405, 640, FONT12ARIALFIXEDWIDTH, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT16ARIAL: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",           0, 425, 640, FONT16ARIAL,           FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"BLOCKFONTNARROW: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",       0, 445, 640, BLOCKFONTNARROW,       FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(L"FONT14HUMANIST: ÄÀÁÂÇËÈÉÊÏÖÒÓÔÜÙÚÛäàáâçëèéêïöòóôüùúûÌÎìî",        0, 465, 640, FONT14HUMANIST,        FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
#else
	DrawTextToScreen(gzCopyrightText[0], 0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, FONT10ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
#endif

	InvalidateRegion(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}


static void RestoreButtonBackGrounds(void)
{
#ifndef TESTFOREIGNFONTS
	for (UINT32 cnt = 0; cnt < NUM_MENU_ITEMS; ++cnt)
	{
		RestoreExternBackgroundRect((SCREEN_WIDTH - gusMainMenuButtonWidths[cnt]) / 2, MAINMENU_Y + cnt * MAINMENU_Y_SPACE - 1, gusMainMenuButtonWidths[cnt] + 1, 23);
	}
#endif
}
