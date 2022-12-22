#include "Button_System.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "Directories.h"
#include "English.h"
#include "Font.h"
#include "Font_Control.h"
#include "GameRes.h"
#include "GameSettings.h"
#include "GameLoop.h"
#include "GameVersion.h"
#include "Input.h"
#include "JA2_Splash.h"
#include "JAScreens.h"
#include "Local.h"
#include "MainMenuScreen.h"
#include "MessageBoxScreen.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "Options_Screen.h"
#include "Render_Dirty.h"
#include "SGP.h"
#include "SaveLoadScreen.h"
#include "SysUtil.h"
#include "Text.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "WordWrap.h"
#include "UILayout.h"

#include <string_theory/format>


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


static BUTTON_PICS* iMenuImages[NUM_MENU_ITEMS];
static GUIButtonRef iMenuButtons[NUM_MENU_ITEMS];

static SGPVObject* guiMainMenuBackGroundImage;
static SGPVObject* guiJa2LogoImage;

static INT8    gbHandledMainMenu = 0;
static BOOLEAN fInitialRender    = FALSE;

static BOOLEAN gfMainMenuScreenEntry = FALSE;
static BOOLEAN gfMainMenuScreenExit = FALSE;

static ScreenID guiMainMenuExitScreen = MAINMENU_SCREEN;


extern BOOLEAN gfLoadGameUponEntry;


static void ExitMainMenu(void);
static void HandleMainMenuInput(void);
static void HandleMainMenuScreen(void);
static void RenderMainMenu(void);
static void RenderGameVersion(void);
static void RenderCopyright(void);
static void RestoreButtonBackGrounds(void);


ScreenID MainMenuScreenHandle(void)
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
			FRAME_BUFFER->ShadowRectUsingLowPercentTable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		}
		else if (guiSplashFrameFade > 1)
		{
			FRAME_BUFFER->Fill(0);
		}
		else
		{
			SetMusicMode(MUSIC_MAIN_MENU);
			SetCurrentCursorFromDatabase(CURSOR_NORMAL);
		}

		guiSplashFrameFade--;

		InvalidateScreen();
		return MAINMENU_SCREEN;
	}

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
		RenderGameVersion();
		RenderCopyright();

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


static void SetMainMenuExitScreen(ScreenID uiNewScreen);


static void HandleMainMenuScreen(void)
{
	if (gbHandledMainMenu == 0) return;

	// Exit according to handled value!
	switch (gbHandledMainMenu)
	{
		case QUIT:
			gfMainMenuScreenExit = TRUE;
			requestGameExit();
			break;

		case LOAD_GAME:
			// Select the game which is to be restored
			guiPreviousOptionScreen = guiCurrentScreen;
			gbHandledMainMenu       = 0;
			gfSaveGame              = FALSE;
			SetMainMenuExitScreen(SAVE_LOAD_SCREEN);
			break;

		case PREFERENCES:
			guiPreviousOptionScreen = guiCurrentScreen;
			gbHandledMainMenu       = 0;
			SetMainMenuExitScreen(OPTIONS_SCREEN);
			break;

		case CREDITS:
			gbHandledMainMenu = 0;
			SetMainMenuExitScreen(CREDIT_SCREEN);
			break;
	}
}


static void CreateDestroyMainMenuButtons(BOOLEAN fCreate);


void InitMainMenu(void)
{
	CreateDestroyMainMenuButtons(TRUE);

#	define GFX_DIR LOADSCREENSDIR
	guiMainMenuBackGroundImage = AddVideoObjectFromFile(GFX_DIR "/mainmenubackground.sti");
	guiJa2LogoImage            = AddVideoObjectFromFile(GFX_DIR "/ja2logo.sti");
#undef GFX_DIR

	// If there are no saved games, disable the button
	if (!AreThereAnySavedGameFiles()) DisableButton(iMenuButtons[LOAD_GAME]);

	gbHandledMainMenu = 0;
	fInitialRender    = TRUE;

	SetPendingNewScreen(MAINMENU_SCREEN);
	guiMainMenuExitScreen = MAINMENU_SCREEN;

	InitGameOptions();

	DequeueAllEvents();
}


static void ExitMainMenu(void)
{
	CreateDestroyMainMenuButtons(FALSE);
	DeleteVideoObject(guiMainMenuBackGroundImage);
	DeleteVideoObject(guiJa2LogoImage);
	gMsgBox.uiExitScreen = MAINMENU_SCREEN;
}


static void MenuButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		INT8 const bID = btn->GetUserData();

		gbHandledMainMenu = bID;
		RenderMainMenu();

		switch (gbHandledMainMenu)
		{
			case NEW_GAME:  SetMainMenuExitScreen(GAME_INIT_OPTIONS_SCREEN); break;
			case LOAD_GAME: if (_KeyDown(ALT)) gfLoadGameUponEntry = TRUE;   break;
		}
	}
}


static void HandleMainMenuInput(void)
{
	InputAtom InputEvent;
	while (DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS))
	{
		if (InputEvent.usEvent == KEY_UP)
		{
			switch (InputEvent.usParam)
			{
				case 'q': if (_KeyDown(CTRL)) gbHandledMainMenu = QUIT; break;					
				case SDLK_ESCAPE: gbHandledMainMenu = QUIT; break;

				case 'c':
					if (_KeyDown(ALT)) gfLoadGameUponEntry = TRUE;
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
	FRAME_BUFFER->Fill(0);
	InvalidateScreen();
}


static void SetMainMenuExitScreen(ScreenID const uiNewScreen)
{
	guiMainMenuExitScreen = uiNewScreen;
	gfMainMenuScreenExit  = TRUE;
}


static void CreateDestroyMainMenuButtons(BOOLEAN fCreate)
{
	static BOOLEAN fButtonsCreated = FALSE;

	if (fCreate)
	{
		if (fButtonsCreated) return;

		// Reset the variable that allows the user to ALT click on the continue save btn to load the save instantly
		gfLoadGameUponEntry = FALSE;

		// Load button images
		const char* const filename = GetMLGFilename(MLG_TITLETEXT);

		INT32 Slot;
		Slot = 0;
		iMenuImages[NEW_GAME]    = LoadButtonImage(filename, Slot, Slot, Slot + 1, Slot + 2, -1);
		iMenuImages[LOAD_GAME]   = UseLoadedButtonImage(iMenuImages[NEW_GAME],  6,  3,  4,  5, -1);
		iMenuImages[PREFERENCES] = UseLoadedButtonImage(iMenuImages[NEW_GAME],  7,  7,  8,  9, -1);
		iMenuImages[CREDITS]     = UseLoadedButtonImage(iMenuImages[NEW_GAME], 13, 10, 11, 12, -1);
		iMenuImages[QUIT]        = UseLoadedButtonImage(iMenuImages[NEW_GAME], 14, 14, 15, 16, -1);

		for (UINT32 cnt = 0; cnt < NUM_MENU_ITEMS; ++cnt)
		{
			BUTTON_PICS* const img = iMenuImages[cnt];
			const UINT16       w   = GetDimensionsOfButtonPic(img)->w;
			const INT16        x   = (SCREEN_WIDTH - w) / 2;
			const INT16        y   = STD_SCREEN_Y + MAINMENU_Y + cnt * MAINMENU_Y_SPACE;
			GUIButtonRef const b = QuickCreateButton(img, x, y, MSYS_PRIORITY_HIGHEST, MenuButtonCallback);
			iMenuButtons[cnt] = b;
			b->SetUserData(cnt);
		}

		fButtonsCreated = TRUE;
	}
	else
	{
		if (!fButtonsCreated) return;

		// Delete images/buttons
		for (UINT32 cnt = 0; cnt < NUM_MENU_ITEMS; ++cnt)
		{
			RemoveButton(iMenuButtons[cnt]);
			UnloadButtonImage(iMenuImages[cnt]);
		}
		fButtonsCreated = FALSE;
	}
}


static void RenderMainMenu(void)
{
	BltVideoObject(FRAME_BUFFER, guiMainMenuBackGroundImage, 0, STD_SCREEN_X,       STD_SCREEN_Y     );
	BltVideoObject(FRAME_BUFFER, guiJa2LogoImage,            0, STD_SCREEN_X + 188, STD_SCREEN_Y + 15);
	BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
}

void RenderGameVersion() {
	SetFontAttributes(FONT10ARIAL, FONT_MCOLOR_WHITE);
	MPrint(g_ui.m_versionPosition.iX, g_ui.m_versionPosition.iY, ST::format("{}", g_version_label));
}

void RenderCopyright() {
	DrawTextToScreen(gzCopyrightText, 0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, FONT10ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
}

static void RestoreButtonBackGrounds(void)
{
#ifndef TESTFOREIGNFONTS
	for (UINT32 cnt = 0; cnt < NUM_MENU_ITEMS; ++cnt)
	{
		GUI_BUTTON const& b = *iMenuButtons[cnt];
		RestoreExternBackgroundRect(b.X(), b.Y(), b.W() + 1, b.H() + 1);
	}
#endif
}
