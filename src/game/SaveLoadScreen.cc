#include "Directories.h"
#include "Font.h"
#include "GameLoop.h"
#include "HImage.h"
#include "Local.h"
#include "Timer_Control.h"
#include "Types.h"
#include "SaveLoadScreen.h"
#include "Video.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Render_Dirty.h"
#include "Text_Input.h"
#include "SaveLoadGame.h"
#include "WordWrap.h"
#include "StrategicMap.h"
#include "Finances.h"
#include "Cursors.h"
#include "VObject.h"
#include "Merc_Hiring.h"
#include "LaptopSave.h"
#include "Options_Screen.h"
#include "GameVersion.h"
#include "SysUtil.h"
#include "Overhead.h"
#include "GameScreen.h"
#include "GameSettings.h"
#include "Fade_Screen.h"
#include "English.h"
#include "Game_Init.h"
#include "Sys_Globals.h"
#include "Text.h"
#include "Message.h"
#include "Map_Screen_Interface.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Campaign_Types.h"
#include "Button_System.h"
#include "Debug.h"
#include "JAScreens.h"
#include "VSurface.h"
#include "FileMan.h"
#include "Campaign_Init.h"
#include "UILayout.h"
#include "Handle_UI.h"
#include "Interface_Dialogue.h"
#include "Meanwhile.h"
#include "PreBattle_Interface.h"
#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <exception>


#define SAVE_LOAD_TITLE_FONT				FONT14ARIAL
#define SAVE_LOAD_TITLE_COLOR				FONT_MCOLOR_WHITE

#define SAVE_LOAD_NORMAL_FONT				FONT12ARIAL
#define SAVE_LOAD_NORMAL_COLOR				2//FONT_MCOLOR_DKWHITE//2//FONT_MCOLOR_WHITE
#define SAVE_LOAD_NORMAL_SHADOW_COLOR			118//121//118//125
/*#define SAVE_LOAD_NORMAL_FONT			FONT12ARIAL
#define SAVE_LOAD_NORMAL_COLOR				FONT_MCOLOR_DKWHITE//2//FONT_MCOLOR_WHITE
#define SAVE_LOAD_NORMAL_SHADOW_COLOR			2//125*/

#define SAVE_LOAD_QUICKSAVE_COLOR			2//FONT_MCOLOR_DKGRAY//FONT_MCOLOR_WHITE
#define SAVE_LOAD_QUICKSAVE_SHADOW_COLOR		189//248//2

#define SAVE_LOAD_EMPTYSLOT_COLOR			2//125//FONT_MCOLOR_WHITE
#define SAVE_LOAD_EMPTYSLOT_SHADOW_COLOR		121//118

#define SAVE_LOAD_HIGHLIGHTED_COLOR			FONT_MCOLOR_WHITE
#define SAVE_LOAD_HIGHLIGHTED_SHADOW_COLOR		2

#define SAVE_LOAD_SELECTED_COLOR			2//145//FONT_MCOLOR_WHITE
#define SAVE_LOAD_SELECTED_SHADOW_COLOR		130//2



#define SAVE_LOAD_NUMBER_FONT				FONT12ARIAL
#define SAVE_LOAD_NUMBER_COLOR				FONT_MCOLOR_WHITE

#define SLG_SELECTED_COLOR				FONT_MCOLOR_WHITE
#define SLG_UNSELECTED_COLOR				FONT_MCOLOR_DKWHITE

#define SLG_SAVELOCATION_WIDTH				605
#define SLG_SAVELOCATION_HEIGHT			30//46
#define SLG_FIRST_SAVED_SPOT_X				(STD_SCREEN_X + 17)
#define SLG_FIRST_SAVED_SPOT_Y				(STD_SCREEN_Y + 49)
#define SLG_GAP_BETWEEN_LOCATIONS			35//47



#define SLG_DATE_OFFSET_X				13
#define SLG_DATE_OFFSET_Y				11

#define SLG_SECTOR_OFFSET_X				95//105//114
#define SLG_SECTOR_WIDTH				98

#define SLG_NUM_MERCS_OFFSET_X				196//190//SLG_DATE_OFFSET_X

#define SLG_BALANCE_OFFSET_X				260//SLG_SECTOR_OFFSET_X

#define SLG_SAVE_GAME_DESC_X				318//320//204
#define SLG_SAVE_GAME_DESC_Y				SLG_DATE_OFFSET_Y//SLG_DATE_OFFSET_Y + 7

#define SLG_TITLE_POS_X				(STD_SCREEN_X)
#define SLG_TITLE_POS_Y				(STD_SCREEN_Y)

#define SLG_SAVE_CANCEL_POS_X				(226 + STD_SCREEN_X)
#define SLG_LOAD_CANCEL_POS_X				(329 + STD_SCREEN_X)
#define SLG_SAVE_LOAD_BTN_POS_X				(123 + STD_SCREEN_X)
#define SLG_BTN_POS_Y					(438 + STD_SCREEN_Y)

#define SLG_SELECTED_SLOT_GRAPHICS_NUMBER		3
#define SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER		2

#define SLG_DOUBLE_CLICK_DELAY				500

//defines for saved game version status
enum
{
	SLS_HEADER_OK,
	SLS_SAVED_GAME_VERSION_OUT_OF_DATE,
	SLS_GAME_VERSION_OUT_OF_DATE,
	SLS_BOTH_SAVE_GAME_AND_GAME_VERSION_OUT_OF_DATE,
};

// enums for the selected Loadscreen Tab (used with giLoadscreenTab[])
enum
{
	SLS_TAB_NORMAL,
	SLS_TAB_DEAD_IS_DEAD,
	SLS_TAB_LENGTH,
};


static BOOLEAN gfSaveLoadScreenEntry = TRUE;
static BOOLEAN gfSaveLoadScreenExit	= FALSE;
BOOLEAN        gfRedrawSaveLoadScreen = TRUE;

static ScreenID guiSaveLoadExitScreen = SAVE_LOAD_SCREEN;


//Contains the array of valid save game locations
static BOOLEAN gbSaveGameArray[NUM_SAVE_GAMES];
static BOOLEAN gbActiveSaveGameTabs[NUM_SAVE_GAMES_TABS];

static BOOLEAN gfDoingQuickLoad = FALSE;

//This flag is used to diferentiate between loading a game and saveing a game.
// gfSaveGame=TRUE		For saving a game
// gfSaveGame=FALSE		For loading a game
BOOLEAN		gfSaveGame=TRUE;
static INT8 gfActiveTab=0;

static BOOLEAN gfSaveLoadScreenButtonsCreated = FALSE;

static INT8 gbSelectedSaveLocation = -1;
static INT8 gbHighLightedLocation  = -1;

static SGPVObject* guiSlgBackGroundImage;
static SGPVObject* guiBackGroundAddOns;


// The string that will contain the game desc text
static ST::string gzGameDescTextField;


static BOOLEAN gfUserInTextInputMode = FALSE;
static UINT8   gubSaveGameNextPass   = 0;

static BOOLEAN gfStartedFadingOut = FALSE;


BOOLEAN		gfCameDirectlyFromGame = FALSE;


BOOLEAN		gfLoadedGame = FALSE;	//Used to know when a game has been loaded, the flag in gtacticalstatus might have been reset already

BOOLEAN		gfLoadGameUponEntry = FALSE;

static BOOLEAN gfHadToMakeBasementLevels = FALSE;


//
//Buttons
//
static BUTTON_PICS* guiSlgButtonImage;


// Cancel Button
static GUIButtonRef guiSlgCancelBtn;

// Save game Button
static BUTTON_PICS* guiSaveLoadImage;
static GUIButtonRef guiSlgSaveLoadBtn;

// buttons for Tabs
static BUTTON_PICS* giLoadscreenTabButtonImage[2];
static GUIButtonRef giLoadscreenTab[2];

//Mouse regions for the currently selected save game
static MOUSE_REGION gSelectedSaveRegion[NUM_SAVE_GAMES];

static MOUSE_REGION gSLSEntireScreenRegion;


static void EnterSaveLoadScreen();
static void ExitSaveLoadScreen(void);
static void GetSaveLoadScreenUserInput(void);
static void RenderSaveLoadScreen(void);
static void SaveLoadGameNumber();
static BOOLEAN IsDeadIsDeadTab(INT8 tabNo);
static void LoadTab(INT8 tabNo);


ScreenID SaveLoadScreenHandle()
{
	if( gfSaveLoadScreenEntry )
	{
		EnterSaveLoadScreen();
		gfSaveLoadScreenEntry = FALSE;
		gfSaveLoadScreenExit = FALSE;

		PauseGame();

		//save the new rect
		BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, SCREEN_WIDTH, 439);
	}

	RestoreBackgroundRects();

	//to guarentee that we do not accept input when we are fading out
	if( !gfStartedFadingOut )
	{
		GetSaveLoadScreenUserInput();
	}
	else
		gfRedrawSaveLoadScreen = FALSE;

	//if we have exited the save load screen, exit
	if( !gfSaveLoadScreenButtonsCreated )
		return( guiSaveLoadExitScreen );

	RenderAllTextFields();

	if( gfRedrawSaveLoadScreen )
	{
		RenderSaveLoadScreen();
		MarkButtonsDirty( );
		RenderButtons();

		gfRedrawSaveLoadScreen = FALSE;
	}

	if( gubSaveGameNextPass != 0 )
	{
		gubSaveGameNextPass++;

		if( gubSaveGameNextPass == 5 )
		{
			gubSaveGameNextPass = 0;
			SaveLoadGameNumber();
		}
	}


	//If we are not exiting the screen, render the buttons
	if( !gfSaveLoadScreenExit && guiSaveLoadExitScreen == SAVE_LOAD_SCREEN )
	{
		// render buttons marked dirty
		RenderButtons( );
	}


	// ATE: Put here to save RECTS before any fast help being drawn...
	SaveBackgroundRects( );
	RenderButtonsFastHelp();

	ExecuteBaseDirtyRectQueue( );
	EndFrameBufferRender( );

	if ( HandleFadeOutCallback( ) )
	{
		return( guiSaveLoadExitScreen );
	}

	if ( HandleBeginFadeOut( SAVE_LOAD_SCREEN ) )
	{
		return( SAVE_LOAD_SCREEN );
	}


	if( gfSaveLoadScreenExit )
	{
		ExitSaveLoadScreen();
	}

	if ( HandleFadeInCallback( ) )
	{
		// Re-render the scene!
		RenderSaveLoadScreen();
	}

	if ( HandleBeginFadeIn( SAVE_LOAD_SCREEN ) )
	{
	}

	return( guiSaveLoadExitScreen );
}


static void DestroySaveLoadTextInputBoxes(void);


static void SetSaveLoadExitScreen(ScreenID const uiScreen)
{
	if( uiScreen == GAME_SCREEN )
	{
		EnterTacticalScreen( );
	}

	gfSaveLoadScreenExit	= TRUE;

	guiSaveLoadExitScreen = uiScreen;

	SetPendingNewScreen( uiScreen );

	if( gfDoingQuickLoad )
	{
		fFirstTimeInGameScreen = TRUE;
		SetPendingNewScreen( uiScreen );
	}

	ExitSaveLoadScreen();

	DestroySaveLoadTextInputBoxes();
}


static void LeaveSaveLoadScreen()
{
	if (gfCameDirectlyFromGame)
	{
		SetSaveLoadExitScreen(guiPreviousOptionScreen);
	} else {
		switch (guiPreviousOptionScreen)
		{
			case MAINMENU_SCREEN: SetSaveLoadExitScreen(MAINMENU_SCREEN); break;
			case GAME_INIT_OPTIONS_SCREEN: SetSaveLoadExitScreen(GAME_INIT_OPTIONS_SCREEN); break;
			case INTRO_SCREEN: SetSaveLoadExitScreen(INTRO_SCREEN); break;
			default: SetSaveLoadExitScreen(OPTIONS_SCREEN);
		}
	}
}


static GUIButtonRef MakeButton(BUTTON_PICS* img, const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	return CreateIconAndTextButton(img, text, OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, x, SLG_BTN_POS_Y, MSYS_PRIORITY_HIGH, click);
}

static void MakeTab(UINT idx, INT16 x, GUI_CALLBACK click, const ST::string& text)
{
	BUTTON_PICS* const img = LoadButtonImage( "sti/interface/loadscreentab.sti", idx, idx+2);
	giLoadscreenTabButtonImage[idx] = img;
	GUIButtonRef const btn = QuickCreateButtonNoMove(img, STD_SCREEN_X + x, STD_SCREEN_Y + 8, MSYS_PRIORITY_HIGHEST - 1, click);
	giLoadscreenTab[idx] = btn;
	btn->SpecifyGeneralTextAttributes(text, OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW);
}

static void BtnSlgCancelCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnSlgSaveLoadCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnSlgNormalGameTabCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnSlgDeadIsDeadTabCallback(GUI_BUTTON* btn, INT32 reason);
static void ClearSelectedSaveSlot(void);
static void InitSaveGameArray(void);
static BOOLEAN LoadSavedGameHeader(INT8 bEntry, SAVED_GAME_HEADER* pSaveGameHeader);
static void SelectedSLSEntireRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedSaveRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedSaveRegionMovementCallBack(MOUSE_REGION* pRegion, INT32 reason);
static void StartFadeOutForSaveLoadScreen(void);

static void CreateLoadscreenTab()
{
	MakeTab(0,        20, BtnSlgNormalGameTabCallback, gs_dead_is_dead_mode_tab_name[0]);
	MakeTab(1, 90, BtnSlgDeadIsDeadTabCallback,    gs_dead_is_dead_mode_tab_name[1]);
	// Render the Normal Tab as selected after create
	giLoadscreenTab[SLS_TAB_NORMAL]->uiFlags |= BUTTON_CLICKED_ON;
}

static void RemoveLoadscreenTab()
{
	for (int i = 0; i < SLS_TAB_LENGTH; i++)
	{
		RemoveButton(giLoadscreenTab[i]);
		UnloadButtonImage(giLoadscreenTabButtonImage[i]);
	}
}

static void updateTabActiveState()
{
	for (INT8 i = 0; i < NUM_SAVE_GAMES_TABS; i++)
	{
		gfActiveTab = i;
		InitSaveGameArray(); // Load the savegames for the current tab
		// Check if the lastSavedGameSlot exists

		bool tabHasSaves = FALSE;
		for (INT8 j = 0; j != NUM_SAVE_GAMES; ++j)
		{
			if (gbSaveGameArray[j])
			{
				tabHasSaves = TRUE;
				break;
			}
		}
		gbActiveSaveGameTabs[i] = tabHasSaves;
		if (!tabHasSaves)
		{
			DisableButton(giLoadscreenTab[i]);
		}
	}
}
// This function determines which tab to activate in the load Screen.
// Depending on:
// In which tab is the last save
// Are there any saves in the tab
// It also deactivates tabs with no saves
static void selectActiveTab()
{
	INT8 const lastSaveInTab = (INT8) (gGameSettings.bLastSavedGameSlot / NUM_SAVE_GAMES);

	updateTabActiveState();

	gfActiveTab = 0;
	InitSaveGameArray();
	// If the lastSavedGameSlot exists, switch to the appropriate tab, otherwise select the first available save
	if (gGameSettings.bLastSavedGameSlot != -1 && gbActiveSaveGameTabs[lastSaveInTab])
	{
		GUI_BUTTON* const b = ButtonList[giLoadscreenTab[lastSaveInTab].ID()];
		b->ClickCallback(b,MSYS_CALLBACK_REASON_LBUTTON_UP);
	}
	else
	{
		// This code doesn't make sense until there are more than two tabs
		for (int i = 1; i < NUM_SAVE_GAMES_TABS; i++)
		{
			if (gbActiveSaveGameTabs[i])
			{
				GUI_BUTTON* const b = ButtonList[giLoadscreenTab[i].ID()];
				b->ClickCallback(b,MSYS_CALLBACK_REASON_LBUTTON_UP);
				break;
			}
		}
	}
}

static void EnterSaveLoadScreen()
{
	gfActiveTab= 0;
	// Display Dead Is Dead games for saving by default if we are to choose the Dead is Dead Slot
	if (guiPreviousOptionScreen == GAME_INIT_OPTIONS_SCREEN)
	{
		gfActiveTab = DEAD_IS_DEAD_TAB_NO;
		gfSaveGame = TRUE;
	}

	// This is a hack to get sector names, but if the underground sector is NOT loaded
	if (!gpUndergroundSectorInfoHead)
	{
		BuildUndergroundSectorInfoList();
		gfHadToMakeBasementLevels = TRUE;
	}
	else
	{
		gfHadToMakeBasementLevels = FALSE;
	}

	guiSaveLoadExitScreen = SAVE_LOAD_SCREEN;
	InitSaveGameArray();
	EmptyBackgroundRects();

	// If the user has asked to load the selected save
	if (gfLoadGameUponEntry)
	{
		// Make sure the save is valid
		INT8 const last_slot = gfActiveTab ? gGameSettings.bLastSavedGameSlot-NUM_SAVE_GAMES : gGameSettings.bLastSavedGameSlot;
		if (last_slot != -1 && gbSaveGameArray[last_slot])
		{
			gbSelectedSaveLocation = last_slot;
			StartFadeOutForSaveLoadScreen();
		}
		else
		{ // else the save is not valid, so do not load it
			gfLoadGameUponEntry = FALSE;
		}
	}

	// Load main background and add ons graphic
	guiSlgBackGroundImage = AddVideoObjectFromFile(INTERFACEDIR "/loadscreen.sti");
	guiBackGroundAddOns   = AddVideoObjectFromFile(GetMLGFilename(MLG_LOADSAVEHEADER));

	guiSlgButtonImage = LoadButtonImage(INTERFACEDIR "/loadscreenaddons.sti", 6, 9);
	guiSlgCancelBtn   = MakeButton(guiSlgButtonImage, zSaveLoadText[SLG_CANCEL], SLG_LOAD_CANCEL_POS_X, BtnSlgCancelCallback);

	// Either the save or load button
	INT32          gfx;
	ST::string text;
	if (gfSaveGame)
	{
		gfx  = 5;
		text = zSaveLoadText[SLG_SAVE_GAME];
	}
	else
	{
		gfx  = 4;
		text = zSaveLoadText[SLG_LOAD_GAME];
	}
	guiSaveLoadImage  = UseLoadedButtonImage(guiSlgButtonImage, gfx, gfx + 3);
	guiSlgSaveLoadBtn = MakeButton(guiSaveLoadImage, text, SLG_SAVE_LOAD_BTN_POS_X, BtnSlgSaveLoadCallback);
	guiSlgSaveLoadBtn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_HATCHED);

	UINT16 const x = SLG_FIRST_SAVED_SPOT_X;
	UINT16       y = SLG_FIRST_SAVED_SPOT_Y;
	for (INT8 i = 0; i != NUM_SAVE_GAMES; ++i)
	{
		MOUSE_REGION& r = gSelectedSaveRegion[i];
		MSYS_DefineRegion(&r, x, y, x + SLG_SAVELOCATION_WIDTH, y + SLG_SAVELOCATION_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, SelectedSaveRegionMovementCallBack, SelectedSaveRegionCallBack);
		MSYS_SetRegionUserData(&r, 0, i);

		// We cannot load a game that has not been saved
		if (!gfSaveGame && !gbSaveGameArray[i]) r.Disable();

		y += SLG_GAP_BETWEEN_LOCATIONS;
	}

	// Create the screen mask to enable ability to right click to cancel the save game
	MSYS_DefineRegion(&gSLSEntireScreenRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH - 10, CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectedSLSEntireRegionCallBack);

	// Display DiD Tab Button if We are in load game
	if (!gfSaveGame)
	{
		CreateLoadscreenTab();
		selectActiveTab();
	}

	ClearSelectedSaveSlot();

	RemoveMouseRegionForPauseOfClock();

	gbHighLightedLocation  = -1;
	gzGameDescTextField = ST::null;

	// If the last saved game slot is ok, set the selected slot to the last saved slot
	INT8 const last_slot = gfActiveTab ? gGameSettings.bLastSavedGameSlot-NUM_SAVE_GAMES : gGameSettings.bLastSavedGameSlot;
	if (last_slot != -1            &&
			gbSaveGameArray[last_slot] &&
			(!gfSaveGame || last_slot != 0)) // If it is not the quicksave slot, and we are loading
	{
		SAVED_GAME_HEADER SaveGameHeader;
		if (LoadSavedGameHeader(last_slot, &SaveGameHeader))
		{
			gzGameDescTextField = SaveGameHeader.sSavedGameDesc;
			gbSelectedSaveLocation = last_slot;
		}
		else
		{
			gGameSettings.bLastSavedGameSlot = -1;
		}
	}

	EnableButton(guiSlgSaveLoadBtn, gbSelectedSaveLocation != -1);
	// Mark all buttons dirty, required for redrawing with the Tab system
	guiSlgCancelBtn->uiFlags |= BUTTON_DIRTY;
	if (!gfSaveGame)
	{
		for (INT8 i = 0; i < SLS_TAB_LENGTH; i++)
		{
			giLoadscreenTab[i]->uiFlags |= BUTTON_DIRTY;
		}
	}

	RenderSaveLoadScreen();

	// Save load buttons are created
	gfSaveLoadScreenButtonsCreated = TRUE;

	gfDoingQuickLoad   = FALSE;
	gfStartedFadingOut = FALSE;

	DisableScrollMessages();

	gfLoadedGame = FALSE;

	if (gfLoadGameUponEntry)
	{
		guiSlgCancelBtn->uiFlags   |= BUTTON_FORCE_UNDIRTY;
		guiSlgSaveLoadBtn->uiFlags |= BUTTON_FORCE_UNDIRTY;
		FRAME_BUFFER->Fill(0);
	}

	gfGettingNameFromSaveLoadScreen = FALSE;
}


static void ExitSaveLoadScreen(void)
{
	INT8	i;


	gfLoadGameUponEntry = FALSE;

	if( !gfSaveLoadScreenButtonsCreated )
		return;

	gfSaveLoadScreenExit = FALSE;
	gfSaveLoadScreenEntry = TRUE;

	UnloadButtonImage( guiSlgButtonImage );

	RemoveButton( guiSlgCancelBtn );

	//Remove the save / load button
//	if( !gfSaveGame )
	{
		RemoveButton( guiSlgSaveLoadBtn );
		UnloadButtonImage( guiSaveLoadImage );
	}
	// Remove the Dead is Dead button
	if(!gfSaveGame)
	{
		RemoveLoadscreenTab();
	}

	for(i=0; i<NUM_SAVE_GAMES; i++)
	{
		MSYS_RemoveRegion( &gSelectedSaveRegion[i]);
	}

	DeleteVideoObject(guiSlgBackGroundImage);
	DeleteVideoObject(guiBackGroundAddOns);

	//Destroy the text fields ( if created )
	DestroySaveLoadTextInputBoxes();

	MSYS_RemoveRegion( &gSLSEntireScreenRegion );

	gfSaveLoadScreenEntry = TRUE;
	gfSaveLoadScreenExit = FALSE;

	if( !gfLoadedGame )
	{
		UnLockPauseState( );
		UnPauseGame();
	}

	gfSaveLoadScreenButtonsCreated = FALSE;

	gfCameDirectlyFromGame = FALSE;

	//unload the basement sectors
	if( gfHadToMakeBasementLevels )
		TrashUndergroundSectorInfo();

	gfGettingNameFromSaveLoadScreen = FALSE;
}


static void DisplaySaveGameList(void);


static void RenderSaveLoadScreen(void)
{
	// If we are going to be instantly leaving the screen, don't draw the numbers
	if (gfLoadGameUponEntry) return;

	BltVideoObject(FRAME_BUFFER, guiSlgBackGroundImage, 0, STD_SCREEN_X, STD_SCREEN_Y);

	// Display the Title
	UINT16 const gfx = gfSaveGame ? 1 : 0;
	BltVideoObject(FRAME_BUFFER, guiBackGroundAddOns, gfx, SLG_TITLE_POS_X, SLG_TITLE_POS_Y);

	DisplaySaveGameList();
	InvalidateScreen();
}


static bool GetGameDescription()
{
	INT8 const id = GetActiveFieldID();
	if (id == 0 || id == -1) return false;

	gzGameDescTextField = GetStringFromField(id);
	return true;
}


static void DisplayOnScreenNumber(BOOLEAN display);
static BOOLEAN DisplaySaveGameEntry(INT8 bEntryID);
static void MoveSelectionDown();
static void MoveSelectionUp();
static void SetSelection(UINT8 ubNewSelection);


static void GetSaveLoadScreenUserInput(void)
{
	static BOOLEAN fWasCtrlHeldDownLastFrame = FALSE;

	// If we are going to be instantly leaving the screen, dont draw the numbers
	if (gfLoadGameUponEntry) return;

	DisplayOnScreenNumber(_KeyDown(ALT));

	if (_KeyDown(CTRL) || fWasCtrlHeldDownLastFrame)
	{
		DisplaySaveGameEntry(gbSelectedSaveLocation);
	}
	fWasCtrlHeldDownLastFrame = _KeyDown(CTRL);

	SGPPoint mouse_pos;
	GetMousePos(&mouse_pos);

	InputAtom e;
	while (DequeueEvent(&e))
	{
		MouseSystemHook(e.usEvent, mouse_pos.iX, mouse_pos.iY);
		if (HandleTextInput(&e)) continue;

		if (e.usEvent == KEY_DOWN)
		{
			switch (e.usParam)
			{
				case '1': SetSelection( 1); break;
				case '2': SetSelection( 2); break;
				case '3': SetSelection( 3); break;
				case '4': SetSelection( 4); break;
				case '5': SetSelection( 5); break;
				case '6': SetSelection( 6); break;
				case '7': SetSelection( 7); break;
				case '8': SetSelection( 8); break;
				case '9': SetSelection( 9); break;
				case '0': SetSelection(10); break;
			}
		}
		else if (e.usEvent == KEY_UP)
		{
			switch (e.usParam)
			{
				case 'a':
					if (_KeyDown(ALT) && !gfSaveGame)
					{
						INT8 const slot = GetNumberForAutoSave(TRUE);
						if (slot == -1) break;

						guiLastSaveGameNum     = slot;
						gbSelectedSaveLocation = SAVE__END_TURN_NUM;
						StartFadeOutForSaveLoadScreen();
					}
					break;

				case 'b':
					if (_KeyDown(ALT) && !gfSaveGame)
					{
						INT8 const slot = GetNumberForAutoSave(FALSE);
						if (slot == -1) break;

						guiLastSaveGameNum     = 1 - slot;
						gbSelectedSaveLocation = SAVE__END_TURN_NUM;
						StartFadeOutForSaveLoadScreen();
					}
					break;

				case SDLK_UP:   MoveSelectionUp();   break;
				case SDLK_DOWN: MoveSelectionDown(); break;

				case SDLK_ESCAPE:
					if (gbSelectedSaveLocation == -1)
					{
						LeaveSaveLoadScreen();
					}
					else
					{ // Reset selected slot
						gbSelectedSaveLocation = -1;
						gfRedrawSaveLoadScreen = TRUE;
						DestroySaveLoadTextInputBoxes();
						DisableButton(guiSlgSaveLoadBtn);
					}
					break;

				case SDLK_RETURN:
					if (!gfSaveGame)
					{
						SaveLoadGameNumber();
					}
					else if (GetGameDescription())
					{
						SetActiveField(0);
						DestroySaveLoadTextInputBoxes();
						SaveLoadGameNumber();
					}
					else if (gbSelectedSaveLocation != -1)
					{
						SaveLoadGameNumber();
					}
					else
					{
						gfRedrawSaveLoadScreen = TRUE;
					}
					break;
			}
		}
	}
}


static UINT8 CompareSaveGameVersion(INT8 bSaveGameID);
static void ConfirmSavedGameMessageBoxCallBack(MessageBoxReturnValue);
static void LoadSavedGameWarningMessageBoxCallBack(MessageBoxReturnValue);
static void SaveGameToSlotNum(void);


static void SaveLoadGameNumber()
{
	INT8 const save_slot_id = gbSelectedSaveLocation;
	if (save_slot_id < 0 || NUM_SAVE_GAMES <= save_slot_id) return;

	if (gfSaveGame)
	{
		GetGameDescription();

		// If there is save game in the slot, ask for confirmation before overwriting
		if (gbSaveGameArray[save_slot_id])
		{
			ST::string sText = st_format_printf(zSaveLoadText[SLG_CONFIRM_SAVE], save_slot_id);
			DoSaveLoadMessageBox(sText, SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmSavedGameMessageBoxCallBack);
		}
		else
		{ // else do NOT put up a confirmation
			SaveGameToSlotNum();
		}
	}
	else
	{
		// Check to see if the save game headers are the same
		UINT8 const ret = CompareSaveGameVersion(save_slot_id);
		if (ret != SLS_HEADER_OK)
		{
			ST::string msg =
				ret == SLS_GAME_VERSION_OUT_OF_DATE       ? zSaveLoadText[SLG_GAME_VERSION_DIF] :
				ret == SLS_SAVED_GAME_VERSION_OUT_OF_DATE ? zSaveLoadText[SLG_SAVED_GAME_VERSION_DIF] :
				zSaveLoadText[SLG_BOTH_GAME_AND_SAVED_GAME_DIF];
			DoSaveLoadMessageBox(msg, SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO, LoadSavedGameWarningMessageBoxCallBack);
		}
		else
		{
			StartFadeOutForSaveLoadScreen();
		}
	}
}
BOOLEAN IsDeadIsDeadTab(INT8 tabNo)
{
	return tabNo == DEAD_IS_DEAD_TAB_NO;
}


// Switch between normal Load game and Dead is Dead
void LoadTab(INT8 tabNo)
{
	if (gfActiveTab != tabNo)
	{
		gfActiveTab = tabNo;

		// Reinit the savegame array and redraw the save load screen
		InitSaveGameArray();
		// Reinit the mouse region for selections, otherwise we can't select the save slots
		gbSelectedSaveLocation = -1;
		UINT16 const x = SLG_FIRST_SAVED_SPOT_X;
		UINT16       y = SLG_FIRST_SAVED_SPOT_Y;
		for (INT8 i = 0; i != NUM_SAVE_GAMES; ++i)
		{
			// Deinitialize first
			MSYS_RemoveRegion( &gSelectedSaveRegion[i]);
			// Reinitialize
			MOUSE_REGION& r = gSelectedSaveRegion[i];
			MSYS_DefineRegion(&r, x, y, x + SLG_SAVELOCATION_WIDTH, y + SLG_SAVELOCATION_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, SelectedSaveRegionMovementCallBack, SelectedSaveRegionCallBack);
			MSYS_SetRegionUserData(&r, 0, i);

			// Disable unused slots and select the first used slot
			if (!gbSaveGameArray[i])
			{
				r.Disable();
			} else if(gbSelectedSaveLocation == -1)
			{
				gbSelectedSaveLocation = i;
			}

			y += SLG_GAP_BETWEEN_LOCATIONS;
		}
		RenderSaveLoadScreen();

		// Render the buttons
		MarkButtonsDirty( );
		RenderButtons();
	}
}


void DoSaveLoadMessageBoxWithRect(const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback, SGPBox const* centering_rect)
{
	// do message box and return
	DoMessageBox(MSG_BOX_BASIC_STYLE, str, uiExitScreen, usFlags, ReturnCallback, centering_rect);
}


void DoSaveLoadMessageBox(const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	DoSaveLoadMessageBoxWithRect(str, uiExitScreen, usFlags, ReturnCallback, NULL);
}


static void InitSaveGameArray(void)
{
	for (INT8 cnt = 0; cnt < NUM_SAVE_GAMES; ++cnt)
	{
		SAVED_GAME_HEADER SaveGameHeader;
		gbSaveGameArray[cnt] = LoadSavedGameHeader(cnt, &SaveGameHeader);
	}
}


static void DisplaySaveGameList(void)
{
	for (INT8 i = 0; i != NUM_SAVE_GAMES; ++i)
	{ // Display all the information from the header
		DisplaySaveGameEntry(i);
	}
}


static BOOLEAN DisplaySaveGameEntry(INT8 const entry_idx)
{
	if (entry_idx == -1) return TRUE;
	// If we are going to be instantly leaving the screen, dont draw the numbers
	if (gfLoadGameUponEntry) return TRUE;
	// If we are currently fading out, leave
	if (gfStartedFadingOut) return TRUE;

	UINT16 const bx = SLG_FIRST_SAVED_SPOT_X;
	UINT16 const by = SLG_FIRST_SAVED_SPOT_Y + SLG_GAP_BETWEEN_LOCATIONS * entry_idx;

	bool const is_selected = entry_idx == gbSelectedSaveLocation;
	bool const save_exists = gbSaveGameArray[entry_idx];

	// Background
	UINT16 const gfx = is_selected ?
		SLG_SELECTED_SLOT_GRAPHICS_NUMBER : SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER;
	BltVideoObject(FRAME_BUFFER, guiBackGroundAddOns, gfx, bx, by);

	SGPFont  font = SAVE_LOAD_NORMAL_FONT;
	UINT8 foreground;
	UINT8 shadow;
	if (entry_idx == 0 && gfSaveGame && gfActiveTab == 0)
	{ // The QuickSave slot
		FRAME_BUFFER->ShadowRect(bx, by, bx + SLG_SAVELOCATION_WIDTH, by + SLG_SAVELOCATION_HEIGHT);
		foreground = SAVE_LOAD_QUICKSAVE_COLOR;
		shadow     = SAVE_LOAD_QUICKSAVE_SHADOW_COLOR;
	}
	else if (is_selected)
	{ // The currently selected location
		foreground = SAVE_LOAD_SELECTED_COLOR;
		shadow     = SAVE_LOAD_SELECTED_SHADOW_COLOR;
	}
	else if (entry_idx == gbHighLightedLocation)
	{ // The highlighted slot
		foreground = SAVE_LOAD_HIGHLIGHTED_COLOR;
		shadow     = SAVE_LOAD_HIGHLIGHTED_SHADOW_COLOR;
	}
	else if (save_exists)
	{ // The file exists
		foreground = SAVE_LOAD_NORMAL_COLOR;
		shadow     = SAVE_LOAD_NORMAL_SHADOW_COLOR;
	}
	else if (gfSaveGame)
	{ // We are saving a game
		foreground = SAVE_LOAD_EMPTYSLOT_COLOR;
		shadow     = SAVE_LOAD_EMPTYSLOT_SHADOW_COLOR;
	}
	else
	{
		FRAME_BUFFER->ShadowRect(bx, by, bx + SLG_SAVELOCATION_WIDTH, by + SLG_SAVELOCATION_HEIGHT);
		foreground = SAVE_LOAD_QUICKSAVE_COLOR;
		shadow     = SAVE_LOAD_QUICKSAVE_SHADOW_COLOR;
	}
	SetFontShadow(shadow);

	if (save_exists || is_selected)
	{ // Setup the strings to be displayed
		SAVED_GAME_HEADER header;
		if (gfSaveGame && is_selected)
		{ // The user has selected a spot to save.  Fill out all the required information
			header.sSavedGameDesc = gzGameDescTextField;
			header.uiDay                     = GetWorldDay();
			header.ubHour                    = GetWorldHour();
			header.ubMin                     = guiMin;
			GetBestPossibleSectorXYZValues(&header.sSectorX, &header.sSectorY, &header.bSectorZ);
			header.ubNumOfMercsOnPlayersTeam = NumberOfMercsOnPlayerTeam();
			header.iCurrentBalance           = LaptopSaveInfo.iCurrentBalance;
			header.sInitialGameOptions       = gGameOptions;
		}
		else if (!LoadSavedGameHeader(entry_idx, &header))
		{
			return FALSE;
		}

		UINT16 x = bx;
		UINT16 y = by + SLG_DATE_OFFSET_Y;
		if (is_selected)
		{ // This is the currently selected location, move the text up a bit
			x++;
			y--;
		}

		if (!gfSaveGame && _KeyDown(CTRL) && is_selected)
		{ // The user is LOADING and holding down the CTRL key, display the additional info
			// Create a string for difficulty level
			ST::string difficulty = ST::format("{} {}", gzGIOScreenText[GIO_EASY_TEXT + header.sInitialGameOptions.ubDifficultyLevel - 1], zSaveLoadText[SLG_DIFF]);

			// Make a string containing the extended options
			UINT8 gameModeText;
			switch (header.sInitialGameOptions.ubGameSaveMode)
			{
				case DIF_IRON_MAN: gameModeText = GIO_IRON_MAN_TEXT; break;
				case DIF_DEAD_IS_DEAD: gameModeText = GIO_DEAD_IS_DEAD_TEXT; break;
				default: gameModeText = GIO_SAVE_ANYWHERE_TEXT;
			}
			ST::string options = ST::format("{20}     {22}     {22}     {22}",
				difficulty,
				/*gzGIOScreenText[GIO_TIMED_TURN_TITLE_TEXT + header.sInitialGameOptions.fTurnTimeLimit + 1],*/
				gzGIOScreenText[gameModeText],
				header.sInitialGameOptions.fGunNut      ? zSaveLoadText[SLG_ADDITIONAL_GUNS] : zSaveLoadText[SLG_NORMAL_GUNS],
				header.sInitialGameOptions.fSciFi       ? zSaveLoadText[SLG_SCIFI]           : zSaveLoadText[SLG_REALISTIC]
			);

			// The date
			DrawTextToScreen(options, x + SLG_DATE_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}
		else
		{ // Display the Saved game information
			// The date
			ST::string date = ST::format("{} {}, {02d}:{02d}", pMessageStrings[MSG_DAY], header.uiDay, header.ubHour, header.ubMin);
			DrawTextToScreen(date, x + SLG_DATE_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			// The sector
			ST::string location;
			if (header.sSectorX != -1 && header.sSectorY != -1 && header.bSectorZ >= 0)
			{
				gfGettingNameFromSaveLoadScreen = TRUE;
				location = GetSectorIDString(header.sSectorX, header.sSectorY, header.bSectorZ, FALSE);
				gfGettingNameFromSaveLoadScreen = FALSE;
			}
			else if (header.uiDay * NUM_SEC_IN_DAY + header.ubHour * NUM_SEC_IN_HOUR + header.ubMin * NUM_SEC_IN_MIN <= STARTING_TIME)
			{
				location = gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION];
			}
			else
			{
				location = gzLateLocalizedString[STR_LATE_14];
			}
			location = ReduceStringLength(location, SLG_SECTOR_WIDTH, font);
			DrawTextToScreen(location, x + SLG_SECTOR_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			// Number of mercs on the team
			// If only 1 merc is on the team use "merc" else "mercs"
			UINT8          const n_mercs = header.ubNumOfMercsOnPlayersTeam;
			ST::string merc = n_mercs == 1 ?
				MercAccountText[MERC_ACCOUNT_MERC] :
				pMessageStrings[MSG_MERCS];
			ST::string merc_count = ST::format("{} {}", n_mercs, merc);
			DrawTextToScreen(merc_count, x + SLG_NUM_MERCS_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			// The balance
			DrawTextToScreen(SPrintMoney(header.iCurrentBalance), x + SLG_BALANCE_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			if (save_exists || (gfSaveGame && !gfUserInTextInputMode && is_selected))
			{
				// The saved game description
				DrawTextToScreen(header.sSavedGameDesc, x + SLG_SAVE_GAME_DESC_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
			}
		}
	}
	else
	{
		// If this is the quick save slot
		ST::string txt;
		if (entry_idx == 0 && gfActiveTab == 0)
		{
			txt = pMessageStrings[MSG_EMPTY_QUICK_SAVE_SLOT];
		} else
		{
			txt = pMessageStrings[MSG_EMPTYSLOT];
		}
		DrawTextToScreen(txt, bx, by + SLG_DATE_OFFSET_Y, 609, font, foreground, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}

	// Reset the shadow color
	SetFontShadow(DEFAULT_SHADOW);

	InvalidateRegion(bx, by, bx + SLG_SAVELOCATION_WIDTH, by + SLG_SAVELOCATION_HEIGHT);
	return TRUE;
}


static BOOLEAN LoadSavedGameHeader(const INT8 bEntry, SAVED_GAME_HEADER* const header)
{
	// make sure the entry is valid
	if (0 <= bEntry && bEntry < NUM_SAVE_GAMES)
	{
		auto savegameName = CreateSavedGameFileNameFromNumber(gfActiveTab ? (bEntry + NUM_SAVE_GAMES) : bEntry);

		try
		{
			if (!GCM->doesUserPrivateFileExist(savegameName)) {
				throw std::runtime_error("Tried to read savegame that does not exist");
			}
			bool stracLinuxFormat;
			AutoSGPFile f(GCM->openUserPrivateFileForReading(savegameName));
			ExtractSavedGameHeaderFromFile(f, *header, &stracLinuxFormat);
			endof(header->zGameVersionNumber)[-1] =  '\0';
			return TRUE;
		}
		catch (...) { /* Handled below */ }

		gbSaveGameArray[bEntry] = FALSE;
	}
	*header = SAVED_GAME_HEADER{};
	return FALSE;
}


static void BtnSlgCancelCallback(GUI_BUTTON* const btn, INT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		LeaveSaveLoadScreen();
	}
}


static void BtnSlgSaveLoadCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		SaveLoadGameNumber();
	}
}

static void BtnSlgNormalGameTabCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		giLoadscreenTab[SLS_TAB_DEAD_IS_DEAD]->uiFlags       &= ~BUTTON_CLICKED_ON;
		if (IsDeadIsDeadTab(gfActiveTab))
		{
			LoadTab(0);
		}
	}
}

static void BtnSlgDeadIsDeadTabCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		giLoadscreenTab[SLS_TAB_NORMAL]->uiFlags       &= ~BUTTON_CLICKED_ON;
		if (!IsDeadIsDeadTab(gfActiveTab))
		{
			LoadTab(1);
		}
	}
}


static void DisableSelectedSlot(void);
static void InitSaveLoadScreenTextInputBoxes(void);
static void RedrawSaveLoadScreenAfterMessageBox(MessageBoxReturnValue);


static void SelectedSaveRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8	bSelected = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		static UINT32	uiLastTime = 0;
		UINT32	uiCurTime = GetJA2Clock();

/*
		//If we are saving and this is the quick save slot
		if( gfSaveGame && bSelected == 0 )
		{
			//Display a pop up telling user what the quick save slot is
			DoSaveLoadMessageBox(pMessageStrings[MSG_QUICK_SAVE_RESERVED_FOR_TACTICAL], SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, RedrawSaveLoadScreenAfterMessageBox);
			return;
		}

		SetSelection( bSelected );
*/

		//If we are saving and this is the quick save slot
		if( gfSaveGame && bSelected == 0 && gfActiveTab == 0)
		{
			//Display a pop up telling user what the quick save slot is
			DoSaveLoadMessageBox(pMessageStrings[MSG_QUICK_SAVE_RESERVED_FOR_TACTICAL], SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, RedrawSaveLoadScreenAfterMessageBox);
			return;
		}

		//if the user is selecting an unselected saved game slot
		if( gbSelectedSaveLocation != bSelected )
		{
			//Destroy the previous region
			DestroySaveLoadTextInputBoxes();

			gbSelectedSaveLocation = bSelected;

			//Reset the global string
			gzGameDescTextField = ST::null;

			//Init the text field for the game desc
			InitSaveLoadScreenTextInputBoxes();

			//If we are Loading the game
//			if( !gfSaveGame )
			{
				//Enable the save/load button
				EnableButton( guiSlgSaveLoadBtn );
			}

			//If we are saving the game, disbale the button
//			if( gfSaveGame )
//					DisableButton( guiSlgSaveLoadBtn );
//			else
			{
				//Set the time in which the button was first pressed
				uiLastTime = GetJA2Clock();
			}

			gfRedrawSaveLoadScreen = TRUE;

			uiLastTime = GetJA2Clock();
		}

		//the user is selecting the selected save game slot
		else
		{
			//if we are saving a game
			if( gfSaveGame )
			{
				//if the user is not currently editing the game desc
				if( !gfUserInTextInputMode )
				{
					if( ( uiCurTime - uiLastTime ) < SLG_DOUBLE_CLICK_DELAY )
					{
						//Load the saved game
						SaveLoadGameNumber();
					}
					else
					{
						uiLastTime = GetJA2Clock();
					}

					InitSaveLoadScreenTextInputBoxes();

					gfRedrawSaveLoadScreen = TRUE;

				}
				else
				{
					if (GetGameDescription())
					{
						SetActiveField(0);

						DestroySaveLoadTextInputBoxes();

//						gfRedrawSaveLoadScreen = TRUE;

//						EnableButton( guiSlgSaveLoadBtn );

						gfRedrawSaveLoadScreen = TRUE;


						if( ( uiCurTime - uiLastTime ) < SLG_DOUBLE_CLICK_DELAY )
						{
							gubSaveGameNextPass = 1;
						}
						else
						{
							uiLastTime = GetJA2Clock();
						}
					}
				}
			}
			//else we are loading
			else
			{
				if( ( uiCurTime - uiLastTime ) < SLG_DOUBLE_CLICK_DELAY )
				{
					//Load the saved game
					SaveLoadGameNumber();
				}
				else
				{
					uiLastTime = GetJA2Clock();
				}
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		DisableSelectedSlot();
	}
}


static void SelectedSaveRegionMovementCallBack(MOUSE_REGION* pRegion, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		INT8 bTemp = gbHighLightedLocation;
		gbHighLightedLocation = -1;
//		DisplaySaveGameList();
		DisplaySaveGameEntry( bTemp );
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		//If we are saving and this is the quick save slot, leave
		if( gfSaveGame )
		{
			return;
		}

		gbHighLightedLocation = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		DisplaySaveGameEntry( gbHighLightedLocation );//, usPosY );
	}
}


static void InitSaveLoadScreenTextInputBoxes(void)
{
	if (gbSelectedSaveLocation == -1)              return;
	if (!gfSaveGame)                               return;
	// If we are exiting, don't create the fields
	if (gfSaveLoadScreenExit)                      return;
	if (guiSaveLoadExitScreen != SAVE_LOAD_SCREEN) return;

	InitTextInputMode();
	SetTextInputCursor(CUROSR_IBEAM_WHITE);
	SetTextInputFont(FONT12ARIALFIXEDWIDTH);
	Set16BPPTextFieldColor(Get16BPPColor(FROMRGB(0, 0, 0)));
	SetBevelColors(Get16BPPColor(FROMRGB(136, 138, 135)), Get16BPPColor(FROMRGB(24, 61, 81)));
	SetTextInputRegularColors(FONT_WHITE, 2);
	SetTextInputHilitedColors(2, FONT_WHITE, FONT_WHITE);
	SetCursorColor(Get16BPPColor(FROMRGB(255, 255, 255)));

	AddUserInputField(NULL);

	// If we are modifying a previously modifed string, use it
	if (!gbSaveGameArray[gbSelectedSaveLocation])
	{
		gzGameDescTextField = ST::null;
	}
	else if (gzGameDescTextField.empty())
	{
		SAVED_GAME_HEADER SaveGameHeader;
		LoadSavedGameHeader(gbSelectedSaveLocation, &SaveGameHeader);
		gzGameDescTextField = SaveGameHeader.sSavedGameDesc;
	}

	// Game Desc Field
	INT16 const x = SLG_FIRST_SAVED_SPOT_X + SLG_SAVE_GAME_DESC_X;
	INT16 const y = SLG_FIRST_SAVED_SPOT_Y + SLG_SAVE_GAME_DESC_Y - 5 + SLG_GAP_BETWEEN_LOCATIONS * gbSelectedSaveLocation;
	AddTextInputField(x, y, SLG_SAVELOCATION_WIDTH - SLG_SAVE_GAME_DESC_X - 7, 17, MSYS_PRIORITY_HIGH + 2, gzGameDescTextField, 46, INPUTTYPE_FULL_TEXT);
	SetActiveField(1);

	gfUserInTextInputMode = TRUE;
}


static void DestroySaveLoadTextInputBoxes(void)
{
	gfUserInTextInputMode = FALSE;
	KillAllTextInputModes();
	SetTextInputCursor( CURSOR_IBEAM );
}


static void SetSelection(UINT8 const new_selection)
{
	// If we are loading and there is no entry, return
	if (!gfSaveGame && !gbSaveGameArray[new_selection]) return;

	gfRedrawSaveLoadScreen = TRUE;
	DestroySaveLoadTextInputBoxes();

	INT8 const old_slot = gbSelectedSaveLocation;
	gbSelectedSaveLocation = new_selection;

	if (gfSaveGame && old_slot != new_selection)
	{
		DestroySaveLoadTextInputBoxes();

		// Null out the current description
		gzGameDescTextField = ST::null;

		//Init the text field for the game desc
		InitSaveLoadScreenTextInputBoxes();
	}

	EnableButton(guiSlgSaveLoadBtn);
}


static UINT8 CompareSaveGameVersion(INT8 bSaveGameID)
{
	UINT8 ubRetVal=SLS_HEADER_OK;

	SAVED_GAME_HEADER SaveGameHeader;

	//Get the heade for the saved game
	LoadSavedGameHeader( bSaveGameID, &SaveGameHeader );

	// check to see if the saved game version in the header is the same as the current version
	if( SaveGameHeader.uiSavedGameVersion != guiSavedGameVersion )
	{
		ubRetVal = SLS_SAVED_GAME_VERSION_OUT_OF_DATE;
	}

	if (strcmp(SaveGameHeader.zGameVersionNumber, g_version_number)!= 0)
	{
		if( ubRetVal == SLS_SAVED_GAME_VERSION_OUT_OF_DATE )
			ubRetVal = SLS_BOTH_SAVE_GAME_AND_GAME_VERSION_OUT_OF_DATE;
		else
			ubRetVal = SLS_GAME_VERSION_OUT_OF_DATE;
	}

	return( ubRetVal );
}


static void LoadSavedGameDeleteAllSaveGameMessageBoxCallBack(MessageBoxReturnValue);


static void LoadSavedGameWarningMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, load the game
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		//Setup up the fade routines
		StartFadeOutForSaveLoadScreen();
	}

	//The user does NOT want to continue..
	else
	{
		//ask if the user wants to delete all the saved game files
		DoSaveLoadMessageBox(zSaveLoadText[SLG_DELETE_ALL_SAVE_GAMES], SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO, LoadSavedGameDeleteAllSaveGameMessageBoxCallBack);
	}
}


static void DeleteAllSaveGameFile(void);


static void LoadSavedGameDeleteAllSaveGameMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, Delete all the save game files
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		DeleteAllSaveGameFile( );
		gfSaveLoadScreenExit = TRUE;
	}

	SetSaveLoadExitScreen( OPTIONS_SCREEN );

	gbSelectedSaveLocation=-1;
}


static void DeleteAllSaveGameFile(void)
{
	UINT8	cnt;

	for( cnt=0; cnt<NUM_SAVE_GAMES; cnt++)
	{
		DeleteSaveGameNumber( cnt );
	}

	gGameSettings.bLastSavedGameSlot = -1;

	InitSaveGameArray();
}


void DeleteSaveGameNumber(UINT8 const save_slot_id)
{
	auto savegameName = CreateSavedGameFileNameFromNumber(save_slot_id);
	GCM->deleteUserPrivateFile(savegameName);
}


static void DisplayOnScreenNumber(BOOLEAN display)
{
	// Start at 1 - don't diplay it for the quicksave
	for (INT8 bLoopNum = 1; bLoopNum < NUM_SAVE_GAMES; ++bLoopNum)
	{
		const UINT16 usPosX = STD_SCREEN_X + 6;
		const UINT16 usPosY = SLG_FIRST_SAVED_SPOT_Y + SLG_GAP_BETWEEN_LOCATIONS * bLoopNum;

		BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, usPosX, usPosY + SLG_DATE_OFFSET_Y, 10, 10);

		if (display)
		{
			const INT8 bNum = (bLoopNum == 10 ? 0 : bLoopNum);
			ST::string zTempString = ST::format("{2d}", bNum);
			DrawTextToScreen(zTempString, usPosX, usPosY + SLG_DATE_OFFSET_Y, 0, SAVE_LOAD_NUMBER_FONT, SAVE_LOAD_NUMBER_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}

		InvalidateRegion(usPosX, usPosY + SLG_DATE_OFFSET_Y, usPosX + 10, usPosY + SLG_DATE_OFFSET_Y + 10);
	}
}


static void DoneFadeInForSaveLoadScreen(void);
static void FailedLoadingGameCallBack(MessageBoxReturnValue);


static void DoneFadeOutForSaveLoadScreen(void)
{
	// Make sure we DON'T reset the levels if we are loading a game
	gfHadToMakeBasementLevels = FALSE;

	try
	{
		LoadSavedGame(IsDeadIsDeadTab(gfActiveTab) ? gbSelectedSaveLocation + NUM_SAVE_GAMES : gbSelectedSaveLocation);

		gFadeInDoneCallback = DoneFadeInForSaveLoadScreen;

		ScreenID const screen = guiScreenToGotoAfterLoadingSavedGame;
		SetSaveLoadExitScreen(screen);
		if (screen == MAP_SCREEN)
		{ // We are to go to map screen after loading the game
			FadeInNextFrame();
		}
		else
		{ // We are to go to the Tactical screen after loading
			PauseTime(FALSE);
			FadeInGameScreen();
		}
	}
	catch (std::exception const& e)
	{
		ST::string msg = st_format_printf(zSaveLoadText[SLG_LOAD_GAME_ERROR], e.what());
		DoSaveLoadMessageBox(msg, SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, FailedLoadingGameCallBack);
		NextLoopCheckForEnoughFreeHardDriveSpace();
	}
	gfStartedFadingOut = FALSE;
}


static void DoneFadeInForSaveLoadScreen(void)
{
	//Leave the screen
	//if we are supposed to stay in tactical, due nothing,
	//if we are supposed to goto mapscreen, leave tactical and go to mapscreen

	if( guiScreenToGotoAfterLoadingSavedGame == MAP_SCREEN )
	{
		if( !gfPauseDueToPlayerGamePause )
		{
			UnLockPauseState( );
			UnPauseGame( );
		}
	}

	else
	{
		//if the game is currently paused
		if( GamePaused() )
		{
			//need to call it twice
			HandlePlayerPauseUnPauseOfGame();
			HandlePlayerPauseUnPauseOfGame();
		}

//		UnLockPauseState( );
//		UnPauseGame( );

	}
}


static void SelectedSLSEntireRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		DisableSelectedSlot();
	}
}


static void DisableSelectedSlot(void)
{
	//reset selected slot
	gbSelectedSaveLocation = -1;
	gfRedrawSaveLoadScreen = TRUE;
	DestroySaveLoadTextInputBoxes();

	if( !gfSaveGame )
		DisableButton( guiSlgSaveLoadBtn );

	//reset the selected graphic
	ClearSelectedSaveSlot();
}


static void ConfirmSavedGameMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	Assert( gbSelectedSaveLocation != -1 );

	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		SaveGameToSlotNum();
	}
}


static void FailedLoadingGameCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes
	if( bExitValue == MSG_BOX_RETURN_OK )
	{
		//if the current screen is tactical
		if( guiPreviousOptionScreen == MAP_SCREEN )
		{
			SetPendingNewScreen( MAINMENU_SCREEN );
		}
		else
		{
			LeaveTacticalScreen( MAINMENU_SCREEN );
		}

		SetSaveLoadExitScreen( MAINMENU_SCREEN );


		//We want to reinitialize the game
		ReStartingGame();
	}
}


void DoQuickSave()
{
	// Use the Dead is Dead function if we are in DiD
	if (gGameOptions.ubGameSaveMode == DIF_DEAD_IS_DEAD)
	{
		DoDeadIsDeadSave();
	} else
	{
		if (SaveGame(0, ST::null)) return;

		if (guiPreviousOptionScreen == MAP_SCREEN)
		{
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		} else
		{
			DoMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
		}
	}
}

// Save function for Dead Is Dead
void DoDeadIsDeadSave()
{
	// Check if we are in a sane state! Do not save if:
	// - we are in an AI Turn
	// - we are in a Dialogue
	// - we are in Meanwhile.....
	// - we are in a locked ui
	// - we are currently in a message box - The Messagebox would be gone without selection after loading
	if (gTacticalStatus.ubCurrentTeam == OUR_TEAM && !gfInTalkPanel && !gfInMeanwhile && !gfPreBattleInterfaceActive && guiPreviousOptionScreen != MSG_BOX_SCREEN && gCurrentUIMode != LOCKUI_MODE)
	{
		// Backup old saves
		BackupSavedGame(gGameSettings.bLastSavedGameSlot);
		// Save the previous option screen State to reset it after saving
		ScreenID tmpGuiPreviousOptionScreen = guiPreviousOptionScreen;
		// We want to save the current screen we are in. Unless we are in Options, Laptop, or others
		// Make sure we are always in a sane screen.
		if (tmpGuiPreviousOptionScreen != MAP_SCREEN && tmpGuiPreviousOptionScreen != GAME_SCREEN) {
			if (guiCurrentScreen != MAP_SCREEN && guiCurrentScreen != GAME_SCREEN) {
				// If all fails, go to the map screen, this (almost) guarantees the game will start
				guiPreviousOptionScreen = MAP_SCREEN;
			} else {
				guiPreviousOptionScreen = guiCurrentScreen;
			}
		}

		BOOLEAN tmpSuccess = SaveGame(gGameSettings.bLastSavedGameSlot, gGameSettings.sCurrentSavedGameName);

		// Reset the previous option screen
		guiPreviousOptionScreen = tmpGuiPreviousOptionScreen;
		if (tmpSuccess) return;

		if (guiPreviousOptionScreen == MAP_SCREEN)
		{
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		} else
		{
			DoMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
		}
	}
}


void DoQuickLoad()
{
	// If there is no save in the quick save slot
	InitSaveGameArray();
	if (!gbSaveGameArray[0]) return;

	// Set the selection to be the quick save slot
	gbSelectedSaveLocation = 0;

	StartFadeOutForSaveLoadScreen();
	gfDoingQuickLoad = TRUE;
}


bool AreThereAnySavedGameFiles()
{
	for (INT8 i = 0; i != (NUM_SAVE_GAMES_TABS * NUM_SAVE_GAMES); ++i)
	{
		auto savegameName = CreateSavedGameFileNameFromNumber(i);
		if (GCM->doesUserPrivateFileExist(savegameName)) return true;
	}
	return false;
}


static void RedrawSaveLoadScreenAfterMessageBox(MessageBoxReturnValue const bExitValue)
{
	gfRedrawSaveLoadScreen = TRUE;
}


static void MoveSelectionDown()
{
	INT8 const slot = gbSelectedSaveLocation;
	if (gfSaveGame)
	{ // We are saving, any slot other then the quick save slot is valid
		if (slot == -1)
		{
			SetSelection(1);
		}
		else if (slot < NUM_SAVE_GAMES - 1)
		{
			SetSelection(slot + 1);
		}
	}
	else
	{
		for (INT32 i = slot != -1 ? slot + 1 : 0; i != NUM_SAVE_GAMES; ++i)
		{
			if (!gbSaveGameArray[i]) continue;
			SetSelection(i);
			break;
		}
	}
}


static void MoveSelectionUp()
{
	INT8 const slot = gbSelectedSaveLocation;
	if (gfSaveGame)
	{ // We are saving, any slot other then the quick save slot is valid
		if (slot == -1)
		{
			SetSelection(NUM_SAVE_GAMES - 1);
		}
		else if (slot > 1)
		{
			SetSelection(slot - 1);
		}
	}
	else
	{
		for (INT32 i = slot != -1 ? slot - 1 : NUM_SAVE_GAMES - 1; i >= 0; --i)
		{
			if (!gbSaveGameArray[i]) continue;
			SetSelection(i);
			break;
		}
	}
}


static void ClearSelectedSaveSlot(void)
{
	gbSelectedSaveLocation = -1;
}


static void SaveGameToSlotNum(void)
{
	//Redraw the save load screen
	RenderSaveLoadScreen();

	//render the buttons
	MarkButtonsDirty( );
	RenderButtons();

	// If we are selecting the Dead is Dead Savegame slot, only remember the slot, do not save
	// Also set the INTRO_SCREEN as previous options screen. This is a hack to get the game started
	if (guiPreviousOptionScreen == GAME_INIT_OPTIONS_SCREEN)
	{
		guiPreviousOptionScreen = INTRO_SCREEN;
		gGameSettings.bLastSavedGameSlot = (gbSelectedSaveLocation + NUM_SAVE_GAMES);
		gGameSettings.sCurrentSavedGameName = gzGameDescTextField;
	}
	else if( !SaveGame(gbSelectedSaveLocation, gzGameDescTextField ) )
	{
		DoSaveLoadMessageBox(zSaveLoadText[SLG_SAVE_GAME_ERROR], SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, NULL);
	}

	SetSaveLoadExitScreen( guiPreviousOptionScreen );
}


static void StartFadeOutForSaveLoadScreen(void)
{
	//if the game is paused, and we are in tactical, unpause
	if( guiPreviousOptionScreen == GAME_SCREEN )
	{
		PauseTime( FALSE );
	}

	gFadeOutDoneCallback = DoneFadeOutForSaveLoadScreen;

	FadeOutNextFrame( );
	gfStartedFadingOut = TRUE;
}
