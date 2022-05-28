#include "Button_Sound_Control.h"
#include "Button_System.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "Directories.h"
#include "English.h"
#include "Fade_Screen.h"
#include "Font.h"
#include "Font_Control.h"
#include "Options_Screen.h"
#include "GameInitOptionsScreen.h"
#include "GameSettings.h"
#include "Input.h"
#include "Intro.h"
#include "Local.h"
#include "MainMenuScreen.h"
#include "MessageBoxScreen.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "Options_Screen.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "Text.h"
#include "Types.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "WordWrap.h"
#include "UILayout.h"

#include <string_theory/string>


#define GIO_TITLE_FONT			FONT16ARIAL//FONT14ARIAL
#define GIO_TITLE_COLOR		FONT_MCOLOR_WHITE

#define GIO_TOGGLE_TEXT_FONT		FONT16ARIAL//FONT14ARIAL
#define GIO_TOGGLE_TEXT_COLOR		FONT_MCOLOR_WHITE

//buttons
#define GIO_BTN_OK_X			(STD_SCREEN_X + 141)
#define GIO_BTN_OK_Y			(STD_SCREEN_Y + 418)
#define GIO_CANCEL_X			(STD_SCREEN_X + 379)

//main title
#define GIO_MAIN_TITLE_X		(STD_SCREEN_X + 0)
#define GIO_MAIN_TITLE_Y		(STD_SCREEN_Y + 68)
#define GIO_MAIN_TITLE_WIDTH		640

//radio box locations
#define GIO_GAP_BN_SETTINGS		35
#define GIO_OFFSET_TO_TEXT		20//30
#define GIO_OFFSET_TO_TOGGLE_BOX	155//200
#define GIO_OFFSET_TO_TOGGLE_BOX_Y	9

#define GIO_DIF_SETTINGS_X		(STD_SCREEN_X + 80)
#define GIO_DIF_SETTINGS_Y		(STD_SCREEN_Y + 150)
#define GIO_DIF_SETTINGS_WIDTH		GIO_OFFSET_TO_TOGGLE_BOX - GIO_OFFSET_TO_TEXT //230

#define GIO_GAME_SETTINGS_X		(STD_SCREEN_X + 350)
#define GIO_GAME_SETTINGS_Y		(STD_SCREEN_Y + 300)
#define GIO_GAME_SETTINGS_WIDTH	GIO_DIF_SETTINGS_WIDTH

#define GIO_GUN_SETTINGS_X		GIO_GAME_SETTINGS_X
#define GIO_GUN_SETTINGS_Y		GIO_DIF_SETTINGS_Y//150//280
#define GIO_GUN_SETTINGS_WIDTH		GIO_DIF_SETTINGS_WIDTH

#if 0
#define GIO_TIMED_TURN_SETTING_X	GIO_DIF_SETTINGS_X
#define GIO_TIMED_TURN_SETTING_Y	GIO_GAME_SETTINGS_Y
#define GIO_TIMED_TURN_SETTING_WIDTH	GIO_DIF_SETTINGS_WIDTH
#endif

#define GIO_IRON_MAN_SETTING_X		GIO_DIF_SETTINGS_X
#define GIO_IRON_MAN_SETTING_Y		GIO_GAME_SETTINGS_Y
#define GIO_IRON_MAN_SETTING_WIDTH	GIO_DIF_SETTINGS_WIDTH

// Game Settings options
enum
{
	GIO_REALISTIC,
	GIO_SCI_FI,

	NUM_GAME_STYLES,
};

// Gun options
enum
{
	GIO_REDUCED_GUNS,
	GIO_GUN_NUT,

	NUM_GUN_OPTIONS,
};

#if 0 // JA2Gold: no more timed turns setting
// enum for the timed turns setting
enum
{
	GIO_NO_TIMED_TURNS,
	GIO_TIMED_TURNS,

	GIO_NUM_TIMED_TURN_OPTIONS,
};
#endif

// Iron man mode
enum
{
	GIO_CAN_SAVE,
	GIO_IRON_MAN,
	GIO_DEAD_IS_DEAD,

	NUM_SAVE_OPTIONS,
};

//enum for different states of game
enum
{
	GIO_NOTHING,
	GIO_CANCEL,
	GIO_EXIT,
	GIO_IRON_MAN_MODE,
	GIO_DEAD_IS_DEAD_MODE,
	GIO_DEAD_IS_DEAD_SAVE
};


static BOOLEAN gfGIOScreenEntry      = TRUE;
static BOOLEAN gfGIOScreenExit       = FALSE;
static BOOLEAN gfReRenderGIOScreen   = TRUE;
static BOOLEAN gfGIOButtonsAllocated = FALSE;

static UINT8 gubGameOptionScreenHandler = GIO_NOTHING;

static ScreenID gubGIOExitScreen = GAME_INIT_OPTIONS_SCREEN;

static SGPVObject* guiGIOMainBackGroundImage;


// Done Button
static void BtnGIODoneCallback(GUI_BUTTON *btn,UINT32 reason);
static GUIButtonRef guiGIODoneButton;
static BUTTON_PICS* giGIODoneBtnImage;

// Cancel Button
static void BtnGIOCancelCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiGIOCancelButton;
static BUTTON_PICS* giGIOCancelBtnImage;


//checkbox to toggle the Diff level
static GUIButtonRef guiDifficultySettingsToggles[NUM_DIF_LEVELS];
static void BtnDifficultyTogglesCallback(GUI_BUTTON *btn, UINT32 reason);

//checkbox to toggle Game style
static GUIButtonRef guiGameStyleToggles[NUM_GAME_STYLES];
static void BtnGameStyleTogglesCallback(GUI_BUTTON *btn,UINT32 reason);

//checkbox to toggle Gun options
static GUIButtonRef guiGunOptionToggles[NUM_GUN_OPTIONS];
static void BtnGunOptionsTogglesCallback(GUI_BUTTON *btn, UINT32 reason);

#if 0// JA2Gold: no more timed turns setting
//checkbox to toggle Timed turn option on or off
static UINT32 guiTimedTurnToggles[GIO_NUM_TIMED_TURN_OPTIONS];
static void BtnTimedTurnsTogglesCallback(GUI_BUTTON *btn, UINT32 reason);
#endif

//checkbox to toggle Save style
static GUIButtonRef guiGameSaveToggles[NUM_SAVE_OPTIONS];
static void BtnGameSaveTogglesCallback(GUI_BUTTON *btn, UINT32 reason);


static void EnterGIOScreen();
static void ExitGIOScreen();
static void HandleGIOScreen(void);
static void RenderGIOScreen(void);
static void GetGIOScreenUserInput(void);
static void RestoreGIOButtonBackGrounds(void);
static void DoneFadeOutForExitGameInitOptionScreen(void);
static void DisplayMessageToUserAboutGameDifficulty(void);
static void DisplayMessageToUserAboutDeadIsDeadSaveScreen(const ST::string& zString, MSGBOX_CALLBACK ReturnCallback);
static void ConfirmGioDifSettingMessageBoxCallBack(MessageBoxReturnValue);
static BOOLEAN DisplayMessageToUserAboutIronManMode(void);
static void ConfirmGioIronManMessageBoxCallBack(MessageBoxReturnValue);
static void ConfirmGioDeadIsDeadMessageBoxCallBack(MessageBoxReturnValue);
static void ConfirmGioDeadIsDeadGoToSaveMessageBoxCallBack(MessageBoxReturnValue);


ScreenID GameInitOptionsScreenHandle(void)
{
	if (gfGIOScreenEntry)
	{
		EnterGIOScreen();
		gfGIOScreenEntry = FALSE;
		gfGIOScreenExit = FALSE;
		InvalidateScreen();
	}

	GetGIOScreenUserInput();

	HandleGIOScreen();

	// render buttons marked dirty
	MarkButtonsDirty();
	RenderButtons();

#if 0 // XXX was commented out
	// render help
	RenderFastHelp();
#endif

	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();

	if (HandleFadeOutCallback())
	{
		ClearMainMenu();
		return gubGIOExitScreen;
	}

	if (HandleBeginFadeOut(gubGIOExitScreen))
	{
		return gubGIOExitScreen;
	}

	if (gfGIOScreenExit)
	{
		ExitGIOScreen();
	}

	if (HandleFadeInCallback())
	{
		// Re-render the scene!
		RenderGIOScreen();
	}

	HandleBeginFadeIn(gubGIOExitScreen);

	return gubGIOExitScreen;
}


static GUIButtonRef MakeButton(BUTTON_PICS* img, const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, x, GIO_BTN_OK_Y, MSYS_PRIORITY_HIGH, click);
	SpecifyButtonSoundScheme(btn, BUTTON_SOUND_SCHEME_BIGSWITCH3);
	return btn;
}


static void MakeCheckBoxes(GUIButtonRef* const btns, size_t const n, INT16 const x, INT16 y, GUI_CALLBACK const click, size_t const def)
{
	for (size_t i = 0; i != n; y += GIO_GAP_BN_SETTINGS, ++i)
	{
		GUIButtonRef const b = CreateCheckBoxButton(x, y, INTERFACEDIR "/optionscheck.sti", MSYS_PRIORITY_HIGH + 10, click);
		btns[i] = b;
		b->SetUserData(i);
	}
	btns[def]->uiFlags |= BUTTON_CLICKED_ON;
}


static void EnterGIOScreen()
{
	if (gfGIOButtonsAllocated) return;

	SetCurrentCursorFromDatabase(CURSOR_NORMAL);

	guiGIOMainBackGroundImage = AddVideoObjectFromFile(INTERFACEDIR "/optionsscreenbackground.sti");

	// Ok button
	giGIODoneBtnImage = LoadButtonImage(INTERFACEDIR "/preferencesbuttons.sti", 0, 2);
	guiGIODoneButton  = MakeButton(giGIODoneBtnImage, gzGIOScreenText[GIO_OK_TEXT], GIO_BTN_OK_X, BtnGIODoneCallback);
	guiGIODoneButton->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);

	// Cancel button
	giGIOCancelBtnImage = UseLoadedButtonImage(giGIODoneBtnImage, 1, 3);
	guiGIOCancelButton  = MakeButton(giGIOCancelBtnImage, gzGIOScreenText[GIO_CANCEL_TEXT], GIO_CANCEL_X, BtnGIOCancelCallback);

	{ // Check box to toggle difficulty settings
		INT16 const x = GIO_DIF_SETTINGS_X + GIO_OFFSET_TO_TOGGLE_BOX;
		INT16 const y = GIO_DIF_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
		size_t def = gGameOptions.ubDifficultyLevel - 1;
		MakeCheckBoxes(guiDifficultySettingsToggles, lengthof(guiDifficultySettingsToggles), x, y, BtnDifficultyTogglesCallback, def);
	}

	{ // Check box to toggle game settings (realistic, sci fi)
		INT16  const x   = GIO_GAME_SETTINGS_X + GIO_OFFSET_TO_TOGGLE_BOX;
		INT16  const y   = GIO_GAME_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
		size_t const def = gGameOptions.fSciFi ? GIO_SCI_FI : GIO_REALISTIC;
		MakeCheckBoxes(guiGameStyleToggles, lengthof(guiGameStyleToggles), x, y, BtnGameStyleTogglesCallback, def);
	}

	{ // JA2Gold: iron man buttons
		INT16  const x   = GIO_IRON_MAN_SETTING_X + GIO_OFFSET_TO_TOGGLE_BOX;
		INT16  const y   = GIO_IRON_MAN_SETTING_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
		size_t const def = gGameOptions.ubGameSaveMode ? gGameOptions.ubGameSaveMode : GIO_CAN_SAVE;
		MakeCheckBoxes(guiGameSaveToggles, lengthof(guiGameSaveToggles), x, y, BtnGameSaveTogglesCallback, def);
	}

	{ // Check box to toggle Gun options
		INT16  const x   = GIO_GUN_SETTINGS_X + GIO_OFFSET_TO_TOGGLE_BOX;
		INT16  const y   = GIO_GUN_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
		size_t const def = gGameOptions.fGunNut ? GIO_GUN_NUT : GIO_REDUCED_GUNS;
		MakeCheckBoxes(guiGunOptionToggles, lengthof(guiGunOptionToggles), x, y, BtnGunOptionsTogglesCallback, def);
	}

#if 0 // JA2 Gold: no more timed turns
	{ // Check box to toggle the timed turn option
		INT16  const x   = GIO_TIMED_TURN_SETTING_X + GIO_OFFSET_TO_TOGGLE_BOX;
		INT16  const y   = GIO_TIMED_TURN_SETTING_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
		size_t const def = gGameOptions.fGunNut ? GIO_TIMED_TURNS : GIO_NO_TIMED_TURNS;
		MakeCheckBoxes(guiTimedTurnToggles, lengthof(guiTimedTurnToggles), x, y, BtnTimedTurnsTogglesCallback, def);
	}
#endif

	// Reset the exit screen
	gubGIOExitScreen = GAME_INIT_OPTIONS_SCREEN;

	// Render the screen once, so we can blt to the save buffer
	RenderGIOScreen();

	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, STD_SCREEN_X, STD_SCREEN_Y, 640, 439);

	gfGIOButtonsAllocated = TRUE;
}


static void ExitGIOScreen()
{
	if (!gfGIOButtonsAllocated) return;
	gfGIOButtonsAllocated = FALSE;

	// Delete the main options screen background.
	DeleteVideoObject(guiGIOMainBackGroundImage);

	RemoveButton(guiGIOCancelButton);
	RemoveButton(guiGIODoneButton);

	UnloadButtonImage(giGIOCancelBtnImage);
	UnloadButtonImage(giGIODoneBtnImage);

	// Check box to toggle difficulty settings
	FOR_EACH(GUIButtonRef, i, guiDifficultySettingsToggles) RemoveButton(*i);

	// Check box to toggle game settings (realistic, sci fi)
	FOR_EACH(GUIButtonRef, i, guiGameStyleToggles) RemoveButton(*i);

	// Check box to toggle gun options
	FOR_EACH(GUIButtonRef, i, guiGunOptionToggles) RemoveButton(*i);

#if 0 // JA2Gold: no more timed turns setting
	// Remove the timed turns toggle.
	FOR_EACH(GUIButtonRef, i, guiTimedTurnToggles) RemoveButton(*i);
#endif

	// JA2Gold: Remove iron man buttons.
	FOR_EACH(GUIButtonRef, i, guiGameSaveToggles) RemoveButton(*i);

	// If we are starting the game stop playing the music.
	if (gubGameOptionScreenHandler == GIO_EXIT) SetMusicMode(MUSIC_NONE);

	gfGIOScreenExit  = FALSE;
	gfGIOScreenEntry = TRUE;
}

static void DisplayMessageToUserAboutDeadIsDeadSaveScreen(const ST::string& zString, MSGBOX_CALLBACK ReturnCallback)
{
	gubGameOptionScreenHandler = GIO_EXIT;
	DoMessageBox(MSG_BOX_BASIC_STYLE, zString, GAME_INIT_OPTIONS_SCREEN, MSG_BOX_FLAG_OK, ReturnCallback, NULL);
}

static void HandleGIOScreen(void)
{
	if (gubGameOptionScreenHandler != GIO_NOTHING)
	{
		switch (gubGameOptionScreenHandler)
		{
			case GIO_CANCEL:
				gubGIOExitScreen = MAINMENU_SCREEN;
				gfGIOScreenExit	= TRUE;
				break;

			case GIO_EXIT:
				//if we are already fading out, get out of here
				if (gFadeOutDoneCallback != DoneFadeOutForExitGameInitOptionScreen)
				{
					//Disable the ok button
					DisableButton(guiGIODoneButton);
					gFadeOutDoneCallback = DoneFadeOutForExitGameInitOptionScreen;
					FadeOutNextFrame();
				}
				break;
			case GIO_IRON_MAN_MODE:
				DisplayMessageToUserAboutGameDifficulty();
				break;
			case GIO_DEAD_IS_DEAD_MODE:
				DisplayMessageToUserAboutGameDifficulty();
				break;
			case GIO_DEAD_IS_DEAD_SAVE:
				DisplayMessageToUserAboutDeadIsDeadSaveScreen(str_dead_is_dead_mode_enter_name, ConfirmGioDeadIsDeadGoToSaveMessageBoxCallBack);
				break;
		}

		gubGameOptionScreenHandler = GIO_NOTHING;
	}

	if (gfReRenderGIOScreen)
	{
		RenderGIOScreen();
		gfReRenderGIOScreen = FALSE;
	}

	RestoreGIOButtonBackGrounds();
}


static void RenderGIOScreen(void)
{
	UINT16		usPosY;

	BltVideoObject(FRAME_BUFFER, guiGIOMainBackGroundImage, 0, STD_SCREEN_X, STD_SCREEN_Y);

	//Shade the background
	FRAME_BUFFER->ShadowRect(STD_SCREEN_X + 48, STD_SCREEN_Y + 55, STD_SCREEN_X + 592, STD_SCREEN_Y + 408); //358


	//Display the title
	DrawTextToScreen(gzGIOScreenText[GIO_INITIAL_GAME_SETTINGS], GIO_MAIN_TITLE_X, GIO_MAIN_TITLE_Y, GIO_MAIN_TITLE_WIDTH, GIO_TITLE_FONT, GIO_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Display the Dif Settings Title Text
	DisplayWrappedString(GIO_DIF_SETTINGS_X, GIO_DIF_SETTINGS_Y - GIO_GAP_BN_SETTINGS, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_DIF_LEVEL_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


	usPosY = GIO_DIF_SETTINGS_Y+2;
	DisplayWrappedString(GIO_DIF_SETTINGS_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_EASY_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosY += GIO_GAP_BN_SETTINGS;
	DisplayWrappedString(GIO_DIF_SETTINGS_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_MEDIUM_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosY += GIO_GAP_BN_SETTINGS;
	DisplayWrappedString(GIO_DIF_SETTINGS_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_HARD_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);



	//Display the Game Settings Title Text
	DisplayWrappedString(GIO_GAME_SETTINGS_X, GIO_GAME_SETTINGS_Y - GIO_GAP_BN_SETTINGS, GIO_GAME_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_GAME_STYLE_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosY = GIO_GAME_SETTINGS_Y+2;
	DisplayWrappedString(GIO_GAME_SETTINGS_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_GAME_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_REALISTIC_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosY += GIO_GAP_BN_SETTINGS;
	DisplayWrappedString(GIO_GAME_SETTINGS_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_GAME_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_SCI_FI_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);



	//Display the Gun Settings Title Text
	DisplayWrappedString(GIO_GUN_SETTINGS_X, GIO_GUN_SETTINGS_Y - GIO_GAP_BN_SETTINGS, GIO_GUN_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_GUN_OPTIONS_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosY = GIO_GUN_SETTINGS_Y+2;
	DisplayWrappedString(GIO_GUN_SETTINGS_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_GUN_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_REDUCED_GUNS_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosY += GIO_GAP_BN_SETTINGS;
	DisplayWrappedString(GIO_GUN_SETTINGS_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_GUN_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_GUN_NUT_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


#if 0 // JA2Gold: no more timed turns setting
	//Display the Timed turns Settings Title Text
	DisplayWrappedString(GIO_TIMED_TURN_SETTING_X, GIO_TIMED_TURN_SETTING_Y - GIO_GAP_BN_SETTINGS, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_TIMED_TURN_TITLE_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY = GIO_TIMED_TURN_SETTING_Y+2;

	DisplayWrappedString(GIO_TIMED_TURN_SETTING_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_NO_TIMED_TURNS_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += GIO_GAP_BN_SETTINGS;

	DisplayWrappedString(GIO_TIMED_TURN_SETTING_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_TIMED_TURNS_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
#endif

// JA2Gold: Display the iron man Settings Title Text
	DisplayWrappedString(GIO_IRON_MAN_SETTING_X, GIO_IRON_MAN_SETTING_Y - GIO_GAP_BN_SETTINGS, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_GAME_SAVE_STYLE_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY = GIO_IRON_MAN_SETTING_Y+2;

	DisplayWrappedString(GIO_IRON_MAN_SETTING_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_SAVE_ANYWHERE_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += GIO_GAP_BN_SETTINGS;

	DisplayWrappedString(GIO_IRON_MAN_SETTING_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_IRON_MAN_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DisplayWrappedString(GIO_IRON_MAN_SETTING_X + GIO_OFFSET_TO_TEXT, usPosY+20, 220, 2, FONT12ARIAL, GIO_TOGGLE_TEXT_COLOR, zNewTacticalMessages[TCTL_MSG__CANNOT_SAVE_DURING_COMBAT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += GIO_GAP_BN_SETTINGS;

	DisplayWrappedString(GIO_IRON_MAN_SETTING_X + GIO_OFFSET_TO_TEXT, usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[GIO_DEAD_IS_DEAD_TEXT], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DisplayWrappedString(GIO_IRON_MAN_SETTING_X + GIO_OFFSET_TO_TEXT, usPosY+20, 220, 2, FONT12ARIAL, GIO_TOGGLE_TEXT_COLOR, zNewTacticalMessages[TCTL_MSG__CANNOT_LOAD_PREVIOUS_SAVE], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
}


static void GetGIOScreenUserInput(void)
{
	InputAtom Event;

	while (DequeueSpecificEvent(&Event, KEYBOARD_EVENTS))
	{
		if (Event.usEvent == KEY_DOWN)
		{
			switch (Event.usParam)
			{
				case SDLK_ESCAPE: gubGameOptionScreenHandler = GIO_CANCEL; break;

				case SDLK_RETURN: gubGameOptionScreenHandler = GIO_EXIT; break;
			}
		}
	}
}


static void SelectCheckbox(GUIButtonRef* i, GUIButtonRef* const end, GUI_BUTTON const& sel)
{
	for (; i != end; ++i)
	{
		GUI_BUTTON& b = **i;
		if (&b == &sel)
			b.uiFlags |= BUTTON_CLICKED_ON;
		else
			b.uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


template<typename T> static inline void SelectCheckbox(T& array, GUI_BUTTON const& sel)
{
	SelectCheckbox(array, endof(array), sel);
}


static void BtnDifficultyTogglesCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SelectCheckbox(guiDifficultySettingsToggles, *btn);
	}
}


static void BtnGameStyleTogglesCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SelectCheckbox(guiGameStyleToggles, *btn);
	}
}


static void BtnGameSaveTogglesCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SelectCheckbox(guiGameSaveToggles, *btn);
	}
}


static void BtnGunOptionsTogglesCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SelectCheckbox(guiGunOptionToggles, *btn);
	}
}


#if 0 // JA2Gold: no more timed turns setting
static void BtnTimedTurnsTogglesCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		SelectCheckbox(guiTimedTurnToggles, *btn);
	}
}
#endif


static void BtnGIODoneCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//if the user doesnt have IRON MAN mode selected
		if (!DisplayMessageToUserAboutIronManMode())
		{
			//Confirm the difficulty setting
			DisplayMessageToUserAboutGameDifficulty();
		}
	}
}


static void BtnGIOCancelCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubGameOptionScreenHandler = GIO_CANCEL;
	}
}


static UINT8 GetCurrentDifficultyButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_DIF_LEVELS; cnt++)
	{
		if (guiDifficultySettingsToggles[cnt]->Clicked()) return cnt;
	}
	return 0;
}


static UINT8 GetCurrentGameStyleButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_GAME_STYLES; cnt++)
	{
		if (guiGameStyleToggles[cnt]->Clicked()) return cnt;
	}
	return 0;
}


static UINT8 GetCurrentGunButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_GUN_OPTIONS; cnt++)
	{
		if (guiGunOptionToggles[cnt]->Clicked()) return cnt;
	}
	return 0;
}


#if 0// JA2 Gold: no timed turns
static UINT8 GetCurrentTimedTurnsButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < GIO_NUM_TIMED_TURN_OPTIONS; cnt++)
	{
		if (guiTimedTurnToggles[cnt]->Clicked()) return cnt;
	}
	return 0;
}
#endif


static UINT8 GetCurrentGameSaveButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_SAVE_OPTIONS; cnt++)
	{
		if (guiGameSaveToggles[cnt]->Clicked()) return cnt;
	}
	return 0;
}


static void RestoreGIOButtonBackGrounds(void)
{
	UINT8	cnt;
	UINT16 usPosY;

	// Check box to toggle Difficulty settings
	usPosY = GIO_DIF_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_DIF_LEVELS; cnt++)
	{
		RestoreExternBackgroundRect(GIO_DIF_SETTINGS_X + GIO_OFFSET_TO_TOGGLE_BOX, usPosY, 34, 29);
		usPosY += GIO_GAP_BN_SETTINGS;
	}

	// Check box to toggle Game settings ( realistic, sci fi )
	usPosY = GIO_GAME_SETTINGS_Y-GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_GAME_STYLES; cnt++)
	{
		RestoreExternBackgroundRect(GIO_GAME_SETTINGS_X + GIO_OFFSET_TO_TOGGLE_BOX, usPosY, 34, 29);
		usPosY += GIO_GAP_BN_SETTINGS;
	}

	// Check box to toggle Gun options
	usPosY = GIO_GUN_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_GUN_OPTIONS; cnt++)
	{
		RestoreExternBackgroundRect(GIO_GUN_SETTINGS_X + GIO_OFFSET_TO_TOGGLE_BOX, usPosY, 34, 29);
		usPosY += GIO_GAP_BN_SETTINGS;
	}

#if 0 // JA2Gold: no more timed turns setting
	// Check box to toggle timed turns options
	usPosY = GIO_TIMED_TURN_SETTING_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < GIO_NUM_TIMED_TURN_OPTIONS; cnt++)
	{
		RestoreExternBackgroundRect(GIO_TIMED_TURN_SETTING_X + GIO_OFFSET_TO_TOGGLE_BOX, usPosY, 34, 29);
		usPosY += GIO_GAP_BN_SETTINGS;
	}
#endif

	// Check box to toggle iron man options
	usPosY = GIO_IRON_MAN_SETTING_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_SAVE_OPTIONS; cnt++)
	{
		RestoreExternBackgroundRect(GIO_IRON_MAN_SETTING_X + GIO_OFFSET_TO_TOGGLE_BOX, usPosY, 34, 29);
		usPosY += GIO_GAP_BN_SETTINGS;
	}
}


static void DoneFadeOutForExitGameInitOptionScreen(void)
{
	// loop through and get the status of all the buttons
	gGameOptions.fGunNut = GetCurrentGunButtonSetting();
	gGameOptions.fSciFi = GetCurrentGameStyleButtonSetting();
	gGameOptions.ubDifficultyLevel = GetCurrentDifficultyButtonSetting() + 1;
#if 0 // JA2Gold: no more timed turns setting
	gGameOptions.fTurnTimeLimit = GetCurrentTimedTurnsButtonSetting();
#endif
	// JA2Gold: iron man
	gGameOptions.ubGameSaveMode = GetCurrentGameSaveButtonSetting();

	if (gGameOptions.ubGameSaveMode == GIO_DEAD_IS_DEAD)
	{
		gubGIOExitScreen = SAVE_LOAD_SCREEN;
	} else
	{
		gubGIOExitScreen = INTRO_SCREEN;
	}

	//set the fact that we should do the intro videos
	SetIntroType(INTRO_BEGINING);

	ExitGIOScreen();

	SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
}


static void DoGioMessageBox(const ST::string& zString, MSGBOX_CALLBACK ReturnCallback)
{
	DoMessageBox(MSG_BOX_BASIC_STYLE, zString, GAME_INIT_OPTIONS_SCREEN, MSG_BOX_FLAG_YESNO, ReturnCallback, NULL);
}


static void DisplayMessageToUserAboutGameDifficulty(void)
{
	ST::string text = zGioDifConfirmText[GetCurrentDifficultyButtonSetting()];
	DoGioMessageBox(text, ConfirmGioDifSettingMessageBoxCallBack);
}


static void ConfirmGioDifSettingMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	if (bExitValue == MSG_BOX_RETURN_YES)
	{
		if (GetCurrentGameSaveButtonSetting() == GIO_DEAD_IS_DEAD)
		{
			gubGameOptionScreenHandler = GIO_DEAD_IS_DEAD_SAVE;
		} else
		{
			gubGameOptionScreenHandler = GIO_EXIT;
		}
	}
}


static BOOLEAN DisplayMessageToUserAboutIronManMode(void)
{
	UINT8 ubIronManMode = GetCurrentGameSaveButtonSetting();

	//if the user has selected IRON MAN mode
	if (ubIronManMode == GIO_IRON_MAN)
	{
		DoGioMessageBox(str_iron_man_mode_warning, ConfirmGioIronManMessageBoxCallBack);
		return  TRUE;
	}
	else if (ubIronManMode == GIO_DEAD_IS_DEAD)
	{
		DoGioMessageBox(str_dead_is_dead_mode_warning, ConfirmGioDeadIsDeadMessageBoxCallBack);
		return  TRUE;
	}
	return FALSE;
}


static void ConfirmGioIronManMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	if (bExitValue == MSG_BOX_RETURN_YES)
	{
		gubGameOptionScreenHandler = GIO_IRON_MAN_MODE;
	}
	else
	{
		SelectCheckbox(guiGameSaveToggles, *guiGameSaveToggles[GIO_CAN_SAVE]);
	}
}

static void ConfirmGioDeadIsDeadMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	if (bExitValue == MSG_BOX_RETURN_YES)
	{
		gubGameOptionScreenHandler = GIO_DEAD_IS_DEAD_MODE;
	}
	else
	{
		SelectCheckbox(guiGameSaveToggles, *guiGameSaveToggles[GIO_CAN_SAVE]);
	}
}

static void ConfirmGioDeadIsDeadGoToSaveMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// Set the Previous Option Screen to the Game Init Options Screen, so the save screen knows we are starting a new game
	guiPreviousOptionScreen = GAME_INIT_OPTIONS_SCREEN;
	gubGameOptionScreenHandler = GIO_EXIT;
}
