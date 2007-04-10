#include "Button_Sound_Control.h"
#include "Button_System.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "English.h"
#include "Fade_Screen.h"
#include "Font_Control.h"
#include "GameInitOptionsScreen.h"
#include "GameSettings.h"
#include "Intro.h"
#include "MainMenuScreen.h"
#include "Music_Control.h"
#include "Options_Screen.h"
#include "Render_Dirty.h"
#include "ScreenIDs.h"
#include "SysUtil.h"
#include "Text.h"
#include "Types.h"
#include "Utilities.h"
#include "VSurface.h"
#include "Video.h"
#include "WCheck.h"
#include "WordWrap.h"
#include "_JA25EnglishText.h"

#ifdef JA2TESTVERSION
#	include "Soldier_Profile.h"
#endif


#define		GIO_TITLE_FONT											FONT16ARIAL//FONT14ARIAL
#define		GIO_TITLE_COLOR											FONT_MCOLOR_WHITE

#define		GIO_TOGGLE_TEXT_FONT								FONT16ARIAL//FONT14ARIAL
#define		GIO_TOGGLE_TEXT_COLOR								FONT_MCOLOR_WHITE

//buttons
#define		GIO_BTN_OK_X												141
#define		GIO_BTN_OK_Y												418
#define		GIO_CANCEL_X												379

//main title
#define		GIO_MAIN_TITLE_X										0
#define		GIO_MAIN_TITLE_Y										68
#define		GIO_MAIN_TITLE_WIDTH								640

//radio box locations
#define		GIO_GAP_BN_SETTINGS									35
#define		GIO_OFFSET_TO_TEXT									20//30
#define		GIO_OFFSET_TO_TOGGLE_BOX						155//200
#define		GIO_OFFSET_TO_TOGGLE_BOX_Y					9

#define		GIO_DIF_SETTINGS_X									80
#define		GIO_DIF_SETTINGS_Y									150
#define		GIO_DIF_SETTINGS_WIDTH							GIO_OFFSET_TO_TOGGLE_BOX - GIO_OFFSET_TO_TEXT //230

#define		GIO_GAME_SETTINGS_X									350
#define		GIO_GAME_SETTINGS_Y									300//280//150
#define		GIO_GAME_SETTINGS_WIDTH							GIO_DIF_SETTINGS_WIDTH

#define		GIO_GUN_SETTINGS_X									GIO_GAME_SETTINGS_X
#define		GIO_GUN_SETTINGS_Y									GIO_DIF_SETTINGS_Y//150//280
#define		GIO_GUN_SETTINGS_WIDTH							GIO_DIF_SETTINGS_WIDTH

#if 0
#define		GIO_TIMED_TURN_SETTING_X						GIO_DIF_SETTINGS_X
#define		GIO_TIMED_TURN_SETTING_Y						GIO_GAME_SETTINGS_Y
#define		GIO_TIMED_TURN_SETTING_WIDTH				GIO_DIF_SETTINGS_WIDTH
#endif

#define		GIO_IRON_MAN_SETTING_X							GIO_DIF_SETTINGS_X
#define		GIO_IRON_MAN_SETTING_Y							GIO_GAME_SETTINGS_Y
#define		GIO_IRON_MAN_SETTING_WIDTH						GIO_DIF_SETTINGS_WIDTH


//Difficulty settings
enum
{
	GIO_DIFF_EASY,
	GIO_DIFF_MED,
	GIO_DIFF_HARD,

	NUM_DIFF_SETTINGS,
};

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

	NUM_SAVE_OPTIONS,
};

//enum for different states of game
enum
{
	GIO_NOTHING,
	GIO_CANCEL,
	GIO_EXIT,
	GIO_IRON_MAN_MODE
};


static BOOLEAN gfGIOScreenEntry      = TRUE;
static BOOLEAN gfGIOScreenExit       = FALSE;
static BOOLEAN gfReRenderGIOScreen   = TRUE;
static BOOLEAN gfGIOButtonsAllocated = FALSE;

static UINT8 gubGameOptionScreenHandler = GIO_NOTHING;

static UINT32 gubGIOExitScreen = GAME_INIT_OPTIONS_SCREEN;

static UINT32 guiGIOMainBackGroundImage;


// Done Button
static void BtnGIODoneCallback(GUI_BUTTON *btn,INT32 reason);
static UINT32 guiGIODoneButton;
static INT32 giGIODoneBtnImage;

// Cancel Button
static void BtnGIOCancelCallback(GUI_BUTTON *btn, INT32 reason);
static UINT32 guiGIOCancelButton;
static INT32 giGIOCancelBtnImage;


//checkbox to toggle the Diff level
static UINT32 guiDifficultySettingsToggles[NUM_DIFF_SETTINGS];
static void BtnDifficultyTogglesCallback(GUI_BUTTON *btn, INT32 reason);

//checkbox to toggle Game style
static UINT32 guiGameStyleToggles[NUM_GAME_STYLES];
static void BtnGameStyleTogglesCallback(GUI_BUTTON *btn,INT32 reason);

//checkbox to toggle Gun options
static UINT32 guiGunOptionToggles[NUM_GUN_OPTIONS];
static void BtnGunOptionsTogglesCallback(GUI_BUTTON *btn, INT32 reason);

#if 0// JA2Gold: no more timed turns setting
//checkbox to toggle Timed turn option on or off
static UINT32 guiTimedTurnToggles[GIO_NUM_TIMED_TURN_OPTIONS];
static void BtnTimedTurnsTogglesCallback(GUI_BUTTON *btn, INT32 reason);
#endif

//checkbox to toggle Save style
static UINT32 guiGameSaveToggles[NUM_SAVE_OPTIONS];
static void BtnGameSaveTogglesCallback(GUI_BUTTON *btn, INT32 reason);


static BOOLEAN EnterGIOScreen(void);
static BOOLEAN ExitGIOScreen(void);
static void HandleGIOScreen(void);
static BOOLEAN RenderGIOScreen(void);
static void GetGIOScreenUserInput(void);
static void RestoreGIOButtonBackGrounds(void);
static void DoneFadeOutForExitGameInitOptionScreen(void);
static void DisplayMessageToUserAboutGameDifficulty(void);
static void ConfirmGioDifSettingMessageBoxCallBack(UINT8 bExitValue);
static BOOLEAN DisplayMessageToUserAboutIronManMode(void);
static void ConfirmGioIronManMessageBoxCallBack(UINT8 bExitValue);


UINT32 GameInitOptionsScreenInit(void)
{
	return 1;
}


UINT32 GameInitOptionsScreenHandle(void)
{
	StartFrameBufferRender();

	if (gfGIOScreenEntry)
	{
		EnterGIOScreen();
		gfGIOScreenEntry = FALSE;
		gfGIOScreenExit = FALSE;
		InvalidateRegion(0, 0, 640, 480);
	}

	GetGIOScreenUserInput();

	HandleGIOScreen();

	// render buttons marked dirty
	MarkButtonsDirty();
	RenderButtons();

#if 0 // XXX was commented out
	// render help
	RenderFastHelp();
	RenderButtonsFastHelp();
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


UINT32 GameInitOptionsScreenShutdown(void)
{
	return 1;
}


static BOOLEAN EnterGIOScreen(void)
{
	UINT16					cnt;
	UINT16					usPosY;

	if (gfGIOButtonsAllocated) return TRUE;

	SetCurrentCursorFromDatabase(CURSOR_NORMAL);

	// load the Main trade screen backgroiund image
	guiGIOMainBackGroundImage = AddVideoObjectFromFile("InterFace/OptionsScreenBackGround.sti");
	CHECKF(guiGIOMainBackGroundImage != NO_VOBJECT);

	//Ok button
	giGIODoneBtnImage = LoadButtonImage("INTERFACE/PreferencesButtons.sti", -1,0,-1,2,-1 );
	guiGIODoneButton = CreateIconAndTextButton( giGIODoneBtnImage, gzGIOScreenText[GIO_OK_TEXT], OPT_BUTTON_FONT,
													 OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW,
													 OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 GIO_BTN_OK_X, GIO_BTN_OK_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnGIODoneCallback);
	SpecifyButtonSoundScheme( guiGIODoneButton, BUTTON_SOUND_SCHEME_BIGSWITCH3 );
	SpecifyDisabledButtonStyle( guiGIODoneButton, DISABLED_STYLE_NONE );

	//Cancel button
	giGIOCancelBtnImage = UseLoadedButtonImage( giGIODoneBtnImage, -1,1,-1,3,-1 );
	guiGIOCancelButton = CreateIconAndTextButton( giGIOCancelBtnImage, gzGIOScreenText[GIO_CANCEL_TEXT], OPT_BUTTON_FONT,
													 OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW,
													 OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 GIO_CANCEL_X, GIO_BTN_OK_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnGIOCancelCallback );
	SpecifyButtonSoundScheme( guiGIOCancelButton, BUTTON_SOUND_SCHEME_BIGSWITCH3 );

	//Check box to toggle Difficulty settings
	usPosY = GIO_DIF_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;

	for (cnt = 0; cnt < NUM_DIFF_SETTINGS; cnt++)
	{
		guiDifficultySettingsToggles[ cnt ] = CreateCheckBoxButton(	GIO_DIF_SETTINGS_X+GIO_OFFSET_TO_TOGGLE_BOX, usPosY,
																		"INTERFACE/OptionsCheck.sti", MSYS_PRIORITY_HIGH+10,
																		BtnDifficultyTogglesCallback );
		MSYS_SetBtnUserData( guiDifficultySettingsToggles[ cnt ], 0, cnt );
		usPosY += GIO_GAP_BN_SETTINGS;
	}
	if (gGameOptions.ubDifficultyLevel == DIF_LEVEL_EASY)
		ButtonList[guiDifficultySettingsToggles[GIO_DIFF_EASY]]->uiFlags |= BUTTON_CLICKED_ON;
	else if(gGameOptions.ubDifficultyLevel == DIF_LEVEL_MEDIUM )
		ButtonList[guiDifficultySettingsToggles[GIO_DIFF_MED]]->uiFlags |= BUTTON_CLICKED_ON;
	else if(gGameOptions.ubDifficultyLevel == DIF_LEVEL_HARD)
		ButtonList[guiDifficultySettingsToggles[GIO_DIFF_HARD]]->uiFlags |= BUTTON_CLICKED_ON;
	else
		ButtonList[guiDifficultySettingsToggles[GIO_DIFF_MED]]->uiFlags |= BUTTON_CLICKED_ON;

	//Check box to toggle Game settings ( realistic, sci fi )
	usPosY = GIO_GAME_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_GAME_STYLES; cnt++)
	{
		guiGameStyleToggles[ cnt ] = CreateCheckBoxButton(	GIO_GAME_SETTINGS_X+GIO_OFFSET_TO_TOGGLE_BOX, usPosY,
																		"INTERFACE/OptionsCheck.sti", MSYS_PRIORITY_HIGH+10,
																		BtnGameStyleTogglesCallback );
		MSYS_SetBtnUserData( guiGameStyleToggles[ cnt ], 0, cnt );
		usPosY += GIO_GAP_BN_SETTINGS;
	}
	if (gGameOptions.fSciFi)
		ButtonList[guiGameStyleToggles[GIO_SCI_FI]]->uiFlags |= BUTTON_CLICKED_ON;
	else
		ButtonList[guiGameStyleToggles[GIO_REALISTIC]]->uiFlags |= BUTTON_CLICKED_ON;

	// JA2Gold: iron man buttons
	usPosY = GIO_IRON_MAN_SETTING_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_SAVE_OPTIONS; cnt++)
	{
		guiGameSaveToggles[ cnt ] = CreateCheckBoxButton(	GIO_IRON_MAN_SETTING_X+GIO_OFFSET_TO_TOGGLE_BOX, usPosY,
																		"INTERFACE/OptionsCheck.sti", MSYS_PRIORITY_HIGH+10,
																		BtnGameSaveTogglesCallback );
		MSYS_SetBtnUserData( guiGameSaveToggles[ cnt ], 0, cnt );
		usPosY += GIO_GAP_BN_SETTINGS;
	}
	if (gGameOptions.fIronManMode)
		ButtonList[guiGameSaveToggles[GIO_IRON_MAN]]->uiFlags |= BUTTON_CLICKED_ON;
	else
		ButtonList[guiGameSaveToggles[GIO_CAN_SAVE]]->uiFlags |= BUTTON_CLICKED_ON;

	// Check box to toggle Gun options
	usPosY = GIO_GUN_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_GUN_OPTIONS; cnt++)
	{
		guiGunOptionToggles[ cnt ] = CreateCheckBoxButton(	GIO_GUN_SETTINGS_X+GIO_OFFSET_TO_TOGGLE_BOX, usPosY,
																		"INTERFACE/OptionsCheck.sti", MSYS_PRIORITY_HIGH+10,
																		BtnGunOptionsTogglesCallback);
		MSYS_SetBtnUserData( guiGunOptionToggles[ cnt ], 0, cnt );
		usPosY += GIO_GAP_BN_SETTINGS;
	}

//if its the demo, make sure the defuat is for additional guns
#ifdef JA2DEMO
	gGameOptions.fGunNut = TRUE;
#endif

	if (gGameOptions.fGunNut)
		ButtonList[guiGunOptionToggles[GIO_GUN_NUT]]->uiFlags |= BUTTON_CLICKED_ON;
	else
		ButtonList[guiGunOptionToggles[GIO_REDUCED_GUNS]]->uiFlags |= BUTTON_CLICKED_ON;

//if its the demo, make sure to disable the buttons
#ifdef JA2DEMO
	SpecifyDisabledButtonStyle(guiGunOptionToggles[GIO_GUN_NUT], DISABLED_STYLE_SHADED);
	SpecifyDisabledButtonStyle(guiGunOptionToggles[GIO_REDUCED_GUNS], DISABLED_STYLE_SHADED);

	DisableButton(guiGunOptionToggles[GIO_GUN_NUT]);
	DisableButton(guiGunOptionToggles[GIO_REDUCED_GUNS]);
#endif


#if 0 // JA2 Gold: no more timed turns
	// Check box to toggle the timed turn option
	usPosY = GIO_TIMED_TURN_SETTING_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < GIO_NUM_TIMED_TURN_OPTIONS; cnt++)
	{
		guiTimedTurnToggles[ cnt ] = CreateCheckBoxButton(	GIO_TIMED_TURN_SETTING_X+GIO_OFFSET_TO_TOGGLE_BOX, usPosY,
																		"INTERFACE/OptionsCheck.sti", MSYS_PRIORITY_HIGH+10,
																		BtnTimedTurnsTogglesCallback );
		MSYS_SetBtnUserData( guiTimedTurnToggles[ cnt ], 0, cnt );
		usPosY += GIO_GAP_BN_SETTINGS;
	}
	if (gGameOptions.fTurnTimeLimit)
		ButtonList[guiTimedTurnToggles[GIO_TIMED_TURNS]]->uiFlags |= BUTTON_CLICKED_ON;
	else
		ButtonList[guiTimedTurnToggles[GIO_NO_TIMED_TURNS]]->uiFlags |= BUTTON_CLICKED_ON;
#endif

	//Reset the exit screen
	gubGIOExitScreen = GAME_INIT_OPTIONS_SCREEN;

	//REnder the screen once so we can blt ot to ths save buffer
	RenderGIOScreen();

	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 639, 439);

	gfGIOButtonsAllocated = TRUE;

	return TRUE;
}


static BOOLEAN ExitGIOScreen(void)
{
	UINT16	cnt;

	if (!gfGIOButtonsAllocated) return TRUE;

	//Delete the main options screen background
	DeleteVideoObjectFromIndex(guiGIOMainBackGroundImage);

	RemoveButton(guiGIOCancelButton);
	RemoveButton(guiGIODoneButton);

	UnloadButtonImage(giGIOCancelBtnImage);
	UnloadButtonImage(giGIODoneBtnImage);

	// Check box to toggle Difficulty settings
	for (cnt = 0; cnt < NUM_DIFF_SETTINGS; cnt++)
		RemoveButton(guiDifficultySettingsToggles[cnt]);

	// Check box to toggle Game settings (realistic, sci fi)
	for (cnt = 0; cnt < NUM_GAME_STYLES; cnt++)
		RemoveButton(guiGameStyleToggles[cnt]);

	// Check box to toggle Gun options
	for (cnt = 0; cnt < NUM_GUN_OPTIONS; cnt++)
		RemoveButton(guiGunOptionToggles[cnt]);

#if 0 // JA2Gold: no more timed turns setting
	// Remove the timed turns toggle
	for (cnt = 0; cnt < GIO_NUM_TIMED_TURN_OPTIONS; cnt++)
		RemoveButton(guiTimedTurnToggles[cnt]);
#endif

	// JA2Gold: remove iron man buttons
	for (cnt = 0; cnt < NUM_SAVE_OPTIONS; cnt++)
		RemoveButton(guiGameSaveToggles[cnt]);

	gfGIOButtonsAllocated = FALSE;

	// If we are starting the game stop playing the music
	if (gubGameOptionScreenHandler == GIO_EXIT)
		SetMusicMode(MUSIC_NONE);

	gfGIOScreenExit = FALSE;
	gfGIOScreenEntry = TRUE;

	return TRUE;
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


static BOOLEAN RenderGIOScreen(void)
{
	UINT16		usPosY;

	BltVideoObjectFromIndex(FRAME_BUFFER, guiGIOMainBackGroundImage, 0, 0, 0);

	//Shade the background
	ShadowVideoSurfaceRect( FRAME_BUFFER, 48, 55, 592, 378 );	//358


	//Display the title
	DrawTextToScreen( gzGIOScreenText[ GIO_INITIAL_GAME_SETTINGS ], GIO_MAIN_TITLE_X, GIO_MAIN_TITLE_Y, GIO_MAIN_TITLE_WIDTH, GIO_TITLE_FONT, GIO_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );


	//Display the Dif Settings Title Text
	//DrawTextToScreen( gzGIOScreenText[ GIO_DIF_LEVEL_TEXT ], GIO_DIF_SETTINGS_X, (UINT16)(GIO_DIF_SETTINGS_Y-GIO_GAP_BN_SETTINGS), GIO_DIF_SETTINGS_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( GIO_DIF_SETTINGS_X, (UINT16)(GIO_DIF_SETTINGS_Y-GIO_GAP_BN_SETTINGS), GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_DIF_LEVEL_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );


	usPosY = GIO_DIF_SETTINGS_Y+2;
	//DrawTextToScreen( gzGIOScreenText[ GIO_EASY_TEXT ], (UINT16)(GIO_DIF_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_MAIN_TITLE_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( (UINT16)(GIO_DIF_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_EASY_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	usPosY += GIO_GAP_BN_SETTINGS;
	//DrawTextToScreen( gzGIOScreenText[ GIO_MEDIUM_TEXT ], (UINT16)(GIO_DIF_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_MAIN_TITLE_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( (UINT16)(GIO_DIF_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_MEDIUM_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	usPosY += GIO_GAP_BN_SETTINGS;
	//DrawTextToScreen( gzGIOScreenText[ GIO_HARD_TEXT ], (UINT16)(GIO_DIF_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_MAIN_TITLE_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( (UINT16)(GIO_DIF_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_HARD_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );



	//Display the Game Settings Title Text
//	DrawTextToScreen( gzGIOScreenText[ GIO_GAME_STYLE_TEXT ], GIO_GAME_SETTINGS_X, (UINT16)(GIO_GAME_SETTINGS_Y-GIO_GAP_BN_SETTINGS), GIO_GAME_SETTINGS_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( GIO_GAME_SETTINGS_X, (UINT16)(GIO_GAME_SETTINGS_Y-GIO_GAP_BN_SETTINGS), GIO_GAME_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_GAME_STYLE_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	usPosY = GIO_GAME_SETTINGS_Y+2;
	//DrawTextToScreen( gzGIOScreenText[ GIO_REALISTIC_TEXT ], (UINT16)(GIO_GAME_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_MAIN_TITLE_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( (UINT16)(GIO_GAME_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_GAME_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_REALISTIC_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	usPosY += GIO_GAP_BN_SETTINGS;
	//DrawTextToScreen( gzGIOScreenText[ GIO_SCI_FI_TEXT ], (UINT16)(GIO_GAME_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_MAIN_TITLE_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( (UINT16)(GIO_GAME_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_GAME_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_SCI_FI_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );



	//Display the Gun Settings Title Text
//	DrawTextToScreen( gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT ], GIO_GUN_SETTINGS_X, (UINT16)(GIO_GUN_SETTINGS_Y-GIO_GAP_BN_SETTINGS), GIO_GUN_SETTINGS_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( GIO_GUN_SETTINGS_X, (UINT16)(GIO_GUN_SETTINGS_Y-GIO_GAP_BN_SETTINGS), GIO_GUN_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	usPosY = GIO_GUN_SETTINGS_Y+2;
	//DrawTextToScreen( gzGIOScreenText[ GIO_REDUCED_GUNS_TEXT ], (UINT16)(GIO_GUN_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_MAIN_TITLE_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( (UINT16)(GIO_GUN_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_GUN_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_REDUCED_GUNS_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	usPosY += GIO_GAP_BN_SETTINGS;
	//DrawTextToScreen( gzGIOScreenText[ GIO_GUN_NUT_TEXT ], (UINT16)(GIO_GUN_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_MAIN_TITLE_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	DisplayWrappedString( (UINT16)(GIO_GUN_SETTINGS_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_GUN_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_GUN_NUT_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );


#if 0 // JA2Gold: no more timed turns setting
	//Display the Timed turns Settings Title Text
	DisplayWrappedString( GIO_TIMED_TURN_SETTING_X, (UINT16)(GIO_TIMED_TURN_SETTING_Y-GIO_GAP_BN_SETTINGS), GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_TIMED_TURN_TITLE_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	usPosY = GIO_TIMED_TURN_SETTING_Y+2;

	DisplayWrappedString( (UINT16)(GIO_TIMED_TURN_SETTING_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_NO_TIMED_TURNS_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	usPosY += GIO_GAP_BN_SETTINGS;

	DisplayWrappedString( (UINT16)(GIO_TIMED_TURN_SETTING_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_TIMED_TURNS_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
#endif

// JA2Gold: Display the iron man Settings Title Text
	DisplayWrappedString( GIO_IRON_MAN_SETTING_X, (UINT16)(GIO_IRON_MAN_SETTING_Y-GIO_GAP_BN_SETTINGS), GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_GAME_SAVE_STYLE_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	usPosY = GIO_IRON_MAN_SETTING_Y+2;

	DisplayWrappedString( (UINT16)(GIO_IRON_MAN_SETTING_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_SAVE_ANYWHERE_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	usPosY += GIO_GAP_BN_SETTINGS;

	DisplayWrappedString( (UINT16)(GIO_IRON_MAN_SETTING_X+GIO_OFFSET_TO_TEXT), usPosY, GIO_DIF_SETTINGS_WIDTH, 2, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, gzGIOScreenText[ GIO_IRON_MAN_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	usPosY += 20;
	DisplayWrappedString( (UINT16)(GIO_IRON_MAN_SETTING_X+GIO_OFFSET_TO_TEXT), usPosY, 220, 2, FONT12ARIAL, GIO_TOGGLE_TEXT_COLOR, zNewTacticalMessages[ TCTL_MSG__CANNOT_SAVE_DURING_COMBAT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );


//if its the demo, make sure to Display the text for the additional guns
#ifdef JA2DEMO

	//Shadow the area around the additional guns
	ShadowVideoSurfaceRect( FRAME_BUFFER, 336, 105, 580, 218 );	//358

	//Display the text indicatting that the option is disabled for the demo
//	DrawTextToScreen( gzGIOScreenText[ GIO_DISABLED_FOR_THE_DEMO_TEXT ], GIO_TIMED_TURN_SETTING_X, (UINT16)(GIO_TIMED_TURN_SETTING_Y-GIO_GAP_BN_SETTINGS), GIO_GUN_SETTINGS_WIDTH, GIO_TOGGLE_TEXT_FONT, GIO_TOGGLE_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	DisplayWrappedString( 440, (UINT16)(GIO_GUN_SETTINGS_Y-GIO_GAP_BN_SETTINGS), 150, 2, GIO_TOGGLE_TEXT_FONT, 162, gzGIOScreenText[ GIO_DISABLED_FOR_THE_DEMO_TEXT ], FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

#endif

	return TRUE;
}


static void GetGIOScreenUserInput(void)
{
	InputAtom Event;

	while (DequeueEvent(&Event))
	{
		if (Event.usEvent == KEY_DOWN)
		{
			switch (Event.usParam)
			{
				case SDLK_ESCAPE: gubGameOptionScreenHandler = GIO_CANCEL; break;

#ifdef JA2TESTVERSION
				case 'r':
					gfReRenderGIOScreen = TRUE;
					break;

				case 'i':
					InvalidateRegion(0, 0, 640, 480);
					break;
#endif

				case SDLK_RETURN: gubGameOptionScreenHandler = GIO_EXIT; break;
			}
		}
	}
}


static void BtnDifficultyTogglesCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8	cnt;

		for (cnt = 0; cnt < NUM_DIFF_SETTINGS; cnt++)
		{
			ButtonList[guiDifficultySettingsToggles[cnt]]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
		btn->uiFlags |= BUTTON_CLICKED_ON;
	}
}


static void BtnGameStyleTogglesCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8	cnt;

		for (cnt = 0; cnt < NUM_GAME_STYLES; cnt++)
		{
			ButtonList[guiGameStyleToggles[cnt]]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
		btn->uiFlags |= BUTTON_CLICKED_ON;
	}
}


static void BtnGameSaveTogglesCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8	cnt;

		for (cnt = 0; cnt < NUM_SAVE_OPTIONS; cnt++)
		{
			ButtonList[guiGameSaveToggles[cnt]]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
		btn->uiFlags |= BUTTON_CLICKED_ON;
	}
}


static void BtnGunOptionsTogglesCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8	cnt;

		for (cnt = 0; cnt < NUM_GUN_OPTIONS; cnt++)
		{
			ButtonList[guiGunOptionToggles[cnt]]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
		btn->uiFlags |= BUTTON_CLICKED_ON;
	}
}


#if 0 // JA2Gold: no more timed turns setting
static void BtnTimedTurnsTogglesCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8	cnt;

		for (cnt = 0; cnt < GIO_NUM_TIMED_TURN_OPTIONS; cnt++)
		{
			ButtonList[guiTimedTurnToggles[cnt]]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
		btn->uiFlags |= BUTTON_CLICKED_ON;
	}
}
#endif


static void BtnGIODoneCallback(GUI_BUTTON *btn,INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//if the user doesnt have IRON MAN mode selected
		if (!DisplayMessageToUserAboutIronManMode())
		{
			//Confirm the difficulty setting
			DisplayMessageToUserAboutGameDifficulty();
		}
	}
}


static void BtnGIOCancelCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gubGameOptionScreenHandler = GIO_CANCEL;
	}
}


static UINT8 GetCurrentDifficultyButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_DIFF_SETTINGS; cnt++)
	{
		if (ButtonList[guiDifficultySettingsToggles[cnt]]->uiFlags & BUTTON_CLICKED_ON)
		{
			return cnt;
		}
	}
	return 0;
}


static UINT8 GetCurrentGameStyleButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_GAME_STYLES; cnt++)
	{
		if (ButtonList[guiGameStyleToggles[cnt]]->uiFlags & BUTTON_CLICKED_ON)
		{
			return cnt;
		}
	}
	return 0;
}


static UINT8 GetCurrentGunButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_GUN_OPTIONS; cnt++)
	{
		if (ButtonList[guiGunOptionToggles[cnt]]->uiFlags & BUTTON_CLICKED_ON)
		{
			return cnt;
		}
	}
	return 0;
}


#if 0// JA2 Gold: no timed turns
static UINT8 GetCurrentTimedTurnsButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < GIO_NUM_TIMED_TURN_OPTIONS; cnt++)
	{
		if (ButtonList[guiTimedTurnToggles[cnt]]->uiFlags & BUTTON_CLICKED_ON)
		{
			return cnt;
		}
	}
	return 0;
}
#endif


static UINT8 GetCurrentGameSaveButtonSetting(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < NUM_SAVE_OPTIONS; cnt++)
	{
		if (ButtonList[guiGameSaveToggles[cnt]]->uiFlags & BUTTON_CLICKED_ON)
		{
			return cnt;
		}
	}
	return 0;
}


static void RestoreGIOButtonBackGrounds(void)
{
	UINT8	cnt;
	UINT16 usPosY;

	// Check box to toggle Difficulty settings
	usPosY = GIO_DIF_SETTINGS_Y - GIO_OFFSET_TO_TOGGLE_BOX_Y;
	for (cnt = 0; cnt < NUM_DIFF_SETTINGS; cnt++)
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
	gGameOptions.fIronManMode = GetCurrentGameSaveButtonSetting();

	gubGIOExitScreen = INTRO_SCREEN;

	//set the fact that we should do the intro videos
//	gbIntroScreenMode = INTRO_BEGINING;
#ifdef JA2TESTVERSION
	if (gfKeyState[ALT])
	{
		if (gfKeyState[CTRL])
		{
			gMercProfiles[MIGUEL].bMercStatus   = MERC_IS_DEAD;
			gMercProfiles[SKYRIDER].bMercStatus = MERC_IS_DEAD;
		}

		SetIntroType(INTRO_ENDING);
	}
	else
#endif
		SetIntroType(INTRO_BEGINING);

	ExitGIOScreen();

	SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
}


static BOOLEAN DoGioMessageBox(UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback)
{
  SGPRect CenteringRect = { 0, 0, 639, 479 };

  return
		DoMessageBox(ubStyle, zString, uiExitScreen, (UINT16)(usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT), ReturnCallback, &CenteringRect) != -1;
}


static void DisplayMessageToUserAboutGameDifficulty(void)
{
	const wchar_t* text;

	switch (GetCurrentDifficultyButtonSetting())
	{
		case 0: text = zGioDifConfirmText[GIO_CFS_NOVICE];      break;
		case 1: text = zGioDifConfirmText[GIO_CFS_EXPERIENCED]; break;
		case 2: text = zGioDifConfirmText[GIO_CFS_EXPERT];      break;
		default: return;
	}
	DoGioMessageBox(MSG_BOX_BASIC_STYLE, text, GAME_INIT_OPTIONS_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmGioDifSettingMessageBoxCallBack);
}


static void ConfirmGioDifSettingMessageBoxCallBack(UINT8 bExitValue)
{
	if (bExitValue == MSG_BOX_RETURN_YES)
	{
		gubGameOptionScreenHandler = GIO_EXIT;
	}
}


static BOOLEAN DisplayMessageToUserAboutIronManMode(void)
{
	UINT8 ubIronManMode = GetCurrentGameSaveButtonSetting();

	//if the user has selected IRON MAN mode
	if (ubIronManMode)
	{
		DoGioMessageBox(MSG_BOX_BASIC_STYLE, gzIronManModeWarningText[IMM__IRON_MAN_MODE_WARNING_TEXT], GAME_INIT_OPTIONS_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmGioIronManMessageBoxCallBack);
		return  TRUE;
	}
	return FALSE;
}


static void ConfirmGioIronManMessageBoxCallBack(UINT8 bExitValue)
{
	if (bExitValue == MSG_BOX_RETURN_YES)
	{
		gubGameOptionScreenHandler = GIO_IRON_MAN_MODE;
	}
	else
	{
		ButtonList[guiGameSaveToggles[GIO_IRON_MAN]]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[guiGameSaveToggles[GIO_CAN_SAVE]]->uiFlags |= BUTTON_CLICKED_ON;
	}
}
