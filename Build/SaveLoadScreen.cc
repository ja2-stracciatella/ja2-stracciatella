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

#if defined JA2BETAVERSION
#include "Soldier_Init_List.h"
#endif


BOOLEAN gfSchedulesHosed = FALSE;


#define		SAVE_LOAD_TITLE_FONT								FONT14ARIAL
#define		SAVE_LOAD_TITLE_COLOR								FONT_MCOLOR_WHITE

#define		SAVE_LOAD_NORMAL_FONT								FONT12ARIAL
#define		SAVE_LOAD_NORMAL_COLOR							2//FONT_MCOLOR_DKWHITE//2//FONT_MCOLOR_WHITE
#define		SAVE_LOAD_NORMAL_SHADOW_COLOR				118//121//118//125
/*#define		SAVE_LOAD_NORMAL_FONT								FONT12ARIAL
#define		SAVE_LOAD_NORMAL_COLOR							FONT_MCOLOR_DKWHITE//2//FONT_MCOLOR_WHITE
#define		SAVE_LOAD_NORMAL_SHADOW_COLOR				2//125
*/

#define		SAVE_LOAD_QUICKSAVE_COLOR						2//FONT_MCOLOR_DKGRAY//FONT_MCOLOR_WHITE
#define		SAVE_LOAD_QUICKSAVE_SHADOW_COLOR		189//248//2

#define		SAVE_LOAD_EMPTYSLOT_COLOR						2//125//FONT_MCOLOR_WHITE
#define		SAVE_LOAD_EMPTYSLOT_SHADOW_COLOR		121//118

#define		SAVE_LOAD_HIGHLIGHTED_COLOR					FONT_MCOLOR_WHITE
#define		SAVE_LOAD_HIGHLIGHTED_SHADOW_COLOR	2

#define		SAVE_LOAD_SELECTED_COLOR						2//145//FONT_MCOLOR_WHITE
#define		SAVE_LOAD_SELECTED_SHADOW_COLOR			130//2



#define		SAVE_LOAD_NUMBER_FONT								FONT12ARIAL
#define		SAVE_LOAD_NUMBER_COLOR							FONT_MCOLOR_WHITE

#define		SLG_SELECTED_COLOR									FONT_MCOLOR_WHITE
#define		SLG_UNSELECTED_COLOR								FONT_MCOLOR_DKWHITE

#define		SLG_SAVELOCATION_WIDTH							605
#define		SLG_SAVELOCATION_HEIGHT							30//46
#define		SLG_FIRST_SAVED_SPOT_X							17
#define		SLG_FIRST_SAVED_SPOT_Y							49
#define		SLG_GAP_BETWEEN_LOCATIONS						35//47



#define		SLG_DATE_OFFSET_X										13
#define		SLG_DATE_OFFSET_Y										11

#define		SLG_SECTOR_OFFSET_X									95//105//114
#define		SLG_SECTOR_WIDTH										98

#define		SLG_NUM_MERCS_OFFSET_X							196//190//SLG_DATE_OFFSET_X

#define		SLG_BALANCE_OFFSET_X								260//SLG_SECTOR_OFFSET_X

#define		SLG_SAVE_GAME_DESC_X								318//320//204
#define		SLG_SAVE_GAME_DESC_Y								SLG_DATE_OFFSET_Y//SLG_DATE_OFFSET_Y + 7

#define		SLG_TITLE_POS_X											0
#define		SLG_TITLE_POS_Y											0

#define SLG_SAVE_CANCEL_POS_X   226//329
#define SLG_LOAD_CANCEL_POS_X   329
#define SLG_SAVE_LOAD_BTN_POS_X 123
#define SLG_BTN_POS_Y           438

#define		SLG_SELECTED_SLOT_GRAPHICS_NUMBER		3
#define		SLG_UNSELECTED_SLOT_GRAPHICS_NUMBER	2

#define		SLG_DOUBLE_CLICK_DELAY							500

//defines for saved game version status
enum
{
	SLS_HEADER_OK,
	SLS_SAVED_GAME_VERSION_OUT_OF_DATE,
	SLS_GAME_VERSION_OUT_OF_DATE,
	SLS_BOTH_SAVE_GAME_AND_GAME_VERSION_OUT_OF_DATE,
};


BOOLEAN		gfSaveLoadScreenEntry = TRUE;
BOOLEAN		gfSaveLoadScreenExit	= FALSE;
BOOLEAN		gfRedrawSaveLoadScreen = TRUE;

static ScreenID guiSaveLoadExitScreen = SAVE_LOAD_SCREEN;


//Contains the array of valid save game locations
static BOOLEAN gbSaveGameArray[NUM_SAVE_GAMES];

BOOLEAN		gfDoingQuickLoad = FALSE;

//This flag is used to diferentiate between loading a game and saveing a game.
// gfSaveGame=TRUE		For saving a game
// gfSaveGame=FALSE		For loading a game
BOOLEAN		gfSaveGame=TRUE;

BOOLEAN		gfSaveLoadScreenButtonsCreated = FALSE;

INT8			gbSelectedSaveLocation=-1;
INT8			gbHighLightedLocation=-1;

static SGPVObject* guiSlgBackGroundImage;
static SGPVObject* guiBackGroundAddOns;


// The string that will contain the game desc text
wchar_t		gzGameDescTextField[ SIZE_OF_SAVE_GAME_DESC ] = {0} ;


BOOLEAN		gfUserInTextInputMode = FALSE;
UINT8			gubSaveGameNextPass=0;

BOOLEAN		gfStartedFadingOut = FALSE;


BOOLEAN		gfCameDirectlyFromGame = FALSE;


BOOLEAN		gfLoadedGame = FALSE;	//Used to know when a game has been loaded, the flag in gtacticalstatus might have been reset already

BOOLEAN		gfLoadGameUponEntry = FALSE;

BOOLEAN		gfHadToMakeBasementLevels = FALSE;


//
//Buttons
//
static BUTTON_PICS* guiSlgButtonImage;


// Cancel Button
GUIButtonRef guiSlgCancelBtn;

// Save game Button
static BUTTON_PICS* guiSaveLoadImage;
GUIButtonRef guiSlgSaveLoadBtn;

//Mouse regions for the currently selected save game
MOUSE_REGION    gSelectedSaveRegion[ NUM_SAVE_GAMES ];

MOUSE_REGION		gSLSEntireScreenRegion;


static void EnterSaveLoadScreen(void);
static void ExitSaveLoadScreen(void);
static void GetSaveLoadScreenUserInput(void);
static void RenderSaveLoadScreen(void);
static void SaveLoadGameNumber(INT8 bSaveGameID);


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
			SaveLoadGameNumber( gbSelectedSaveLocation );
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


static GUIButtonRef MakeButton(BUTTON_PICS* const img, const wchar_t* const text, const INT16 x, const GUI_CALLBACK click)
{
	return CreateIconAndTextButton(img, text, OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, x, SLG_BTN_POS_Y, MSYS_PRIORITY_HIGH, click);
}


static void BtnSlgCancelCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnSlgSaveLoadCallback(GUI_BUTTON* btn, INT32 reason);
static void ClearSelectedSaveSlot(void);
static void ConfirmLoadSavedGameMessageBoxCallBack(UINT8 bExitValue);
static void InitSaveGameArray(void);
static BOOLEAN LoadSavedGameHeader(INT8 bEntry, SAVED_GAME_HEADER* pSaveGameHeader);
static void SelectedSLSEntireRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedSaveRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedSaveRegionMovementCallBack(MOUSE_REGION* pRegion, INT32 reason);


static void EnterSaveLoadScreen(void)
{
	INT8	i;
	UINT16 usPosX = SLG_FIRST_SAVED_SPOT_X;
	UINT16 usPosY = SLG_FIRST_SAVED_SPOT_Y;

	// This is a hack to get sector names , but... if the underground sector is NOT loaded
	if( !gpUndergroundSectorInfoHead )
	{
		BuildUndergroundSectorInfoList();
		gfHadToMakeBasementLevels = TRUE;
	}
	else
	{
		gfHadToMakeBasementLevels = FALSE;
	}

	guiSaveLoadExitScreen = SAVE_LOAD_SCREEN;
	//init the list
	InitSaveGameArray();

	//Clear out all the saved background rects
	EmptyBackgroundRects( );

	//if the user has asked to load the selected save
	if( gfLoadGameUponEntry )
	{
		//make sure the save is valid
		if( gGameSettings.bLastSavedGameSlot != -1 && gbSaveGameArray[ gGameSettings.bLastSavedGameSlot ] )
		{
			 gbSelectedSaveLocation = gGameSettings.bLastSavedGameSlot;

			//load the saved game
			ConfirmLoadSavedGameMessageBoxCallBack( MSG_BOX_RETURN_YES );
		}
		else
		{ //else the save isnt valid, so dont load it
			gfLoadGameUponEntry = FALSE;
		}
	}

	// load Main background  graphic and add it
	guiSlgBackGroundImage = AddVideoObjectFromFile("INTERFACE/LoadScreen.sti");

	// load Load Screen Add ons graphic and add it
	const char* const ImageFile = GetMLGFilename(MLG_LOADSAVEHEADER);
	guiBackGroundAddOns = AddVideoObjectFromFile(ImageFile);

	guiSlgButtonImage = LoadButtonImage("INTERFACE/LoadScreenAddOns.sti", -1,6,-1,9,-1 );
//	guiSlgButtonImage = UseLoadedButtonImage( guiBackGroundAddOns, -1,9,-1,6,-1 );


	//Cancel button
//	if( gfSaveGame )
//		usPosX = SLG_SAVE_CANCEL_POS_X;
//	else
		usPosX = SLG_LOAD_CANCEL_POS_X;

	guiSlgCancelBtn = MakeButton(guiSlgButtonImage, zSaveLoadText[SLG_CANCEL], usPosX, BtnSlgCancelCallback);

	//Either the save or load button
	const wchar_t* text;
	if( gfSaveGame )
	{
		//If we are saving, dont have the save game button
		guiSaveLoadImage = UseLoadedButtonImage( guiSlgButtonImage, -1,5,-1,8,-1 );
		text = zSaveLoadText[SLG_SAVE_GAME];
	}
	else
	{
		guiSaveLoadImage = UseLoadedButtonImage( guiSlgButtonImage, -1,4,-1,7,-1 );
		text = zSaveLoadText[SLG_LOAD_GAME];
	}
	guiSlgSaveLoadBtn = MakeButton(guiSaveLoadImage, text, SLG_SAVE_LOAD_BTN_POS_X, BtnSlgSaveLoadCallback);

	//if we are loading, disable the load button
//	if( !gfSaveGame )
	{
		guiSlgSaveLoadBtn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_HATCHED);
		DisableButton(guiSlgSaveLoadBtn);
	}


	usPosX = SLG_FIRST_SAVED_SPOT_X;
	usPosY = SLG_FIRST_SAVED_SPOT_Y;
	for(i=0; i<NUM_SAVE_GAMES; i++)
	{
		MSYS_DefineRegion( &gSelectedSaveRegion[i], usPosX, usPosY, (UINT16)(usPosX+SLG_SAVELOCATION_WIDTH), (UINT16)(usPosY+SLG_SAVELOCATION_HEIGHT), MSYS_PRIORITY_HIGH,
								 CURSOR_NORMAL, SelectedSaveRegionMovementCallBack, SelectedSaveRegionCallBack );
		MSYS_SetRegionUserData( &gSelectedSaveRegion[ i ], 0, i);

		//if we are to Load a game
		if( !gfSaveGame )
		{
			//We cannot load a game that hasnt been saved
			if( !gbSaveGameArray[ i ] )
				gSelectedSaveRegion[i].Disable();
		}

		usPosY += SLG_GAP_BETWEEN_LOCATIONS;
	}

/*
Removed so that the user can click on it and get displayed a message that the quick save slot is for the tactical screen
	if( gfSaveGame )
	{
		gSelectedSaveRegion[0].Disable();
	}
*/

	//Create the screen mask to enable ability to righ click to cancel the sace game
	MSYS_DefineRegion(&gSLSEntireScreenRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH - 10, CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectedSLSEntireRegionCallBack);

	ClearSelectedSaveSlot();


	//Remove the mouse region over the clock
	RemoveMouseRegionForPauseOfClock(  );

	//Draw the screen
//	gfRedrawSaveLoadScreen = TRUE;	DEF:

	//Reset the highlight
	gbHighLightedLocation = -1;

	gzGameDescTextField[0] = '\0';



	//if we are loading
//	if( !gfSaveGame )
	{
		guiSlgSaveLoadBtn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_HATCHED);

		//if the last saved game slot is ok, set the selected slot to the last saved slot]
		if( gGameSettings.bLastSavedGameSlot != -1 )
		{
			//if the slot is valid
			if( gbSaveGameArray[ gGameSettings.bLastSavedGameSlot ] )
			{
				SAVED_GAME_HEADER SaveGameHeader;

				memset( &SaveGameHeader, 0, sizeof( SAVED_GAME_HEADER ) );

				//if it is not the Quick Save slot, and we are loading
				if( !gfSaveGame || gfSaveGame && gGameSettings.bLastSavedGameSlot != 0 )
				{
					gbSelectedSaveLocation = gGameSettings.bLastSavedGameSlot;

					//load the save gamed header string

					//Get the heade for the saved game
					if( !LoadSavedGameHeader( gbSelectedSaveLocation, &SaveGameHeader ) )
					{
						gbSaveGameArray[ gbSelectedSaveLocation ] = FALSE;
						gbSelectedSaveLocation = gGameSettings.bLastSavedGameSlot = -1;
					}

					wcscpy( gzGameDescTextField, SaveGameHeader.sSavedGameDesc );
				}
			}
		}

		//if we are loading and the there is no slot selected
		if( gbSelectedSaveLocation == -1 )
			DisableButton( guiSlgSaveLoadBtn );
		else
			EnableButton( guiSlgSaveLoadBtn );
	}


	RenderSaveLoadScreen();

	// Save load buttons are created
	gfSaveLoadScreenButtonsCreated = TRUE;

	gfDoingQuickLoad = FALSE;

	//reset
	gfStartedFadingOut = FALSE;

	DisableScrollMessages();

	gfLoadedGame = FALSE;

	if( gfLoadGameUponEntry )
	{
		//unmark the 2 buttons from being dirty
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

	BltVideoObject(FRAME_BUFFER, guiSlgBackGroundImage, 0, 0, 0);

	// Display the Title
	UINT16 const gfx = gfSaveGame ? 1 : 0;
	BltVideoObject(FRAME_BUFFER, guiBackGroundAddOns, gfx, SLG_TITLE_POS_X, SLG_TITLE_POS_Y);

	DisplaySaveGameList();
	InvalidateScreen();
}


static void DisplayOnScreenNumber(BOOLEAN display);
static BOOLEAN DisplaySaveGameEntry(INT8 bEntryID);
static void MoveSelectionUpOrDown(BOOLEAN fUp);
static void SetSelection(UINT8 ubNewSelection);
static void StartFadeOutForSaveLoadScreen(void);


static void GetSaveLoadScreenUserInput(void)
{
	InputAtom Event;
	INT8		bActiveTextField;
	static BOOLEAN	fWasCtrlHeldDownLastFrame = FALSE;

	SGPPoint MousePos;
	GetMousePos(&MousePos);

	//if we are going to be instantly leaving the screen, dont draw the numbers
	if( gfLoadGameUponEntry )
	{
		return;
	}

	DisplayOnScreenNumber(gfKeyState[ALT]);

	if( gfKeyState[ CTRL ] || fWasCtrlHeldDownLastFrame )
	{
		DisplaySaveGameEntry( gbSelectedSaveLocation );
	}

	fWasCtrlHeldDownLastFrame = gfKeyState[ CTRL ];

	while( DequeueEvent( &Event ) )
	{
		MouseSystemHook(Event.usEvent, MousePos.iX, MousePos.iY);

		if( !HandleTextInput( &Event ) && Event.usEvent == KEY_DOWN )
		{
			switch( Event.usParam )
			{
				case '1':
					SetSelection( 1 );
					break;
				case '2':
					SetSelection( 2 );
					break;
				case '3':
					SetSelection( 3 );
					break;
				case '4':
					SetSelection( 4 );
					break;
				case '5':
					SetSelection( 5 );
					break;
				case '6':
					SetSelection( 6 );
					break;
				case '7':
					SetSelection( 7 );
					break;
				case '8':
					SetSelection( 8 );
					break;
				case '9':
					SetSelection( 9 );
					break;
				case '0':
					SetSelection( 10 );
					break;
			}
		}

		if( Event.usEvent == KEY_UP )
		{
			switch( Event.usParam )
			{
				case 'a':
					if( gfKeyState[ ALT ] && !gfSaveGame )
					{
						INT8 iFile = GetNumberForAutoSave( TRUE );

						if( iFile == -1 )
							break;

						guiLastSaveGameNum = iFile;

						gbSelectedSaveLocation = SAVE__END_TURN_NUM;
						StartFadeOutForSaveLoadScreen();
					}
					break;

				case 'b':
					if( gfKeyState[ ALT ] && !gfSaveGame )
					{
						INT8 iFile = GetNumberForAutoSave( FALSE );

						if( iFile == -1 )
							break;
						else if( iFile == 0 )
							guiLastSaveGameNum = 1;
						else if( iFile == 1 )
							guiLastSaveGameNum = 0;

						gbSelectedSaveLocation = SAVE__END_TURN_NUM;
						StartFadeOutForSaveLoadScreen();
					}
					break;

				case SDLK_UP:   MoveSelectionUpOrDown(TRUE);  break;
				case SDLK_DOWN: MoveSelectionUpOrDown(FALSE); break;

				case SDLK_ESCAPE:
					if( gbSelectedSaveLocation == -1 )
					{

						if( 	gfCameDirectlyFromGame )
							SetSaveLoadExitScreen( guiPreviousOptionScreen );

						else if( guiPreviousOptionScreen == MAINMENU_SCREEN )
							SetSaveLoadExitScreen( MAINMENU_SCREEN );
						else
							SetSaveLoadExitScreen( OPTIONS_SCREEN );
					}
					else
					{
						//reset selected slot
						gbSelectedSaveLocation = -1;
						gfRedrawSaveLoadScreen = TRUE;
						DestroySaveLoadTextInputBoxes();

//						if( !gfSaveGame )
							DisableButton( guiSlgSaveLoadBtn );
					}
					break;


				case SDLK_RETURN:
					if( gfSaveGame )
					{
						bActiveTextField = (INT8)GetActiveFieldID();
						if( bActiveTextField && bActiveTextField != -1 )
						{
							Get16BitStringFromField( (UINT8)bActiveTextField, gzGameDescTextField, lengthof(gzGameDescTextField));
							SetActiveField(0);

							DestroySaveLoadTextInputBoxes();

							SaveLoadGameNumber( gbSelectedSaveLocation );
							return;
						}
						else
						{
							if( gbSelectedSaveLocation != -1 )
							{
								SaveLoadGameNumber( gbSelectedSaveLocation );
								return;
							}
						}
						//Enable the save/load button
						if( gbSelectedSaveLocation != -1 )
							if( !gfSaveGame )
								EnableButton( guiSlgSaveLoadBtn );

						gfRedrawSaveLoadScreen = TRUE;
					}
					else
						SaveLoadGameNumber( gbSelectedSaveLocation );

					break;
			}
		}
	}
}


static UINT8 CompareSaveGameVersion(INT8 bSaveGameID);
static void ConfirmSavedGameMessageBoxCallBack(MessageBoxReturnValue);
static void LoadSavedGameWarningMessageBoxCallBack(MessageBoxReturnValue);
static void SaveGameToSlotNum(void);


static void SaveLoadGameNumber(INT8 bSaveGameID)
{
//	CHAR16	zTemp[128];
	UINT8		ubRetVal=0;

	if( bSaveGameID >= NUM_SAVE_GAMES || bSaveGameID < 0 )
	{
		return;
	}

	if( gfSaveGame )
	{
		INT8		bActiveTextField;

		bActiveTextField = (INT8)GetActiveFieldID();
		if( bActiveTextField && bActiveTextField != -1 )
		{
			Get16BitStringFromField( (UINT8)bActiveTextField, gzGameDescTextField, lengthof(gzGameDescTextField));
		}

		//if there is save game in the slot, ask for confirmation before overwriting
		if( gbSaveGameArray[ bSaveGameID ] )
		{
			CHAR16	sText[512];

			swprintf( sText, lengthof(sText), zSaveLoadText[SLG_CONFIRM_SAVE], bSaveGameID );

			DoSaveLoadMessageBox(sText, SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmSavedGameMessageBoxCallBack);
		}
		else
		{
			//else do NOT put up a confirmation

			//Save the game
			SaveGameToSlotNum();
		}
	}
	else
	{
		//Check to see if the save game headers are the same
		ubRetVal = CompareSaveGameVersion( bSaveGameID );
		if( ubRetVal != SLS_HEADER_OK )
		{
			wchar_t const* const msg =
				ubRetVal == SLS_GAME_VERSION_OUT_OF_DATE       ? zSaveLoadText[SLG_GAME_VERSION_DIF] :
				ubRetVal == SLS_SAVED_GAME_VERSION_OUT_OF_DATE ? zSaveLoadText[SLG_SAVED_GAME_VERSION_DIF] :
				zSaveLoadText[SLG_BOTH_GAME_AND_SAVED_GAME_DIF];
			DoSaveLoadMessageBox(msg, SAVE_LOAD_SCREEN, MSG_BOX_FLAG_YESNO, LoadSavedGameWarningMessageBoxCallBack);
		}
		else
		{
			//Setup up the fade routines
			StartFadeOutForSaveLoadScreen();
		}
	}
}


void DoSaveLoadMessageBoxWithRect(wchar_t const* const zString, ScreenID const uiExitScreen, MessageBoxFlags const usFlags, MSGBOX_CALLBACK const ReturnCallback, SGPBox const* const centering_rect)
{
	// do message box and return
	DoMessageBox(MSG_BOX_BASIC_STYLE, zString, uiExitScreen, usFlags, ReturnCallback, centering_rect);
}


void DoSaveLoadMessageBox(wchar_t const* const zString, ScreenID const uiExitScreen, MessageBoxFlags const usFlags, MSGBOX_CALLBACK const ReturnCallback)
{
	DoSaveLoadMessageBoxWithRect(zString, uiExitScreen, usFlags, ReturnCallback, NULL);
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
	INT8	bLoop1;
//	UINT16 usPosX = SLG_FIRST_SAVED_SPOT_X;
	UINT16 usPosY = SLG_FIRST_SAVED_SPOT_Y;


	for( bLoop1=0; bLoop1<NUM_SAVE_GAMES; bLoop1++)
	{
		//display all the information from the header
		DisplaySaveGameEntry( bLoop1 );//usPosY );

		usPosY += SLG_GAP_BETWEEN_LOCATIONS;
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

	Font  font = SAVE_LOAD_NORMAL_FONT;
	UINT8 foreground;
	UINT8 shadow;
	if (entry_idx == 0 && gfSaveGame)
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
			wcscpy(header.sSavedGameDesc, gzGameDescTextField);
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

		if (!gfSaveGame && gfKeyState[CTRL] && is_selected)
		{ // The user is LOADING and holding down the CTRL key, display the additional info
			// Create a string for difficulty level
			wchar_t difficulty[256];
			swprintf(difficulty, lengthof(difficulty), L"%ls %ls", gzGIOScreenText[GIO_EASY_TEXT + header.sInitialGameOptions.ubDifficultyLevel - 1], zSaveLoadText[SLG_DIFF]);

			// Make a string containing the extended options
			wchar_t options[256];
			swprintf(options, lengthof(options), L"%20ls     %22ls     %22ls     %22ls",
				difficulty,
				/*gzGIOScreenText[GIO_TIMED_TURN_TITLE_TEXT + header.sInitialGameOptions.fTurnTimeLimit + 1],*/
				header.sInitialGameOptions.fIronManMode ? gzGIOScreenText[GIO_IRON_MAN_TEXT] : gzGIOScreenText[GIO_SAVE_ANYWHERE_TEXT],
				header.sInitialGameOptions.fGunNut      ? zSaveLoadText[SLG_ADDITIONAL_GUNS] : zSaveLoadText[SLG_NORMAL_GUNS],
				header.sInitialGameOptions.fSciFi       ? zSaveLoadText[SLG_SCIFI]           : zSaveLoadText[SLG_REALISTIC]
			);

			// The date
			DrawTextToScreen(options, x + SLG_DATE_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}
		else
		{ // Display the Saved game information
			// The date
			wchar_t date[128];
			swprintf(date, lengthof(date), L"%ls %d, %02d:%02d", pMessageStrings[MSG_DAY], header.uiDay, header.ubHour, header.ubMin);
			DrawTextToScreen(date, x + SLG_DATE_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			// The sector
			wchar_t location[128];
			if (header.sSectorX != -1 && header.sSectorY != -1 && header.bSectorZ >= 0)
			{
				gfGettingNameFromSaveLoadScreen = TRUE;
				GetSectorIDString(header.sSectorX, header.sSectorY, header.bSectorZ, location, lengthof(location), FALSE);
				gfGettingNameFromSaveLoadScreen = FALSE;
			}
			else if (header.uiDay * NUM_SEC_IN_DAY + header.ubHour * NUM_SEC_IN_HOUR + header.ubMin * NUM_SEC_IN_MIN <= STARTING_TIME)
			{
				wcslcpy(location, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(location));
			}
			else
			{
				wcslcpy(location, gzLateLocalizedString[14], lengthof(location));
			}
			ReduceStringLength(location, lengthof(location), SLG_SECTOR_WIDTH, font);
			DrawTextToScreen(location, x + SLG_SECTOR_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			// Number of mercs on the team
			// If only 1 merc is on the team use "merc" else "mercs"
			UINT8          const n_mercs = header.ubNumOfMercsOnPlayersTeam;
			wchar_t const* const merc    = n_mercs == 1 ?
				MercAccountText[MERC_ACCOUNT_MERC] :
				pMessageStrings[MSG_MERCS];
			wchar_t merc_count[128];
			swprintf(merc_count, lengthof(merc_count), L"%d %ls", n_mercs, merc);
			DrawTextToScreen(merc_count, x + SLG_NUM_MERCS_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			// The balance
			wchar_t balance[128];
			SPrintMoney(balance, header.iCurrentBalance);
			DrawTextToScreen(balance, x + SLG_BALANCE_OFFSET_X, y, 0, font, foreground, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

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
		wchar_t const* const txt = entry_idx == 0 ?
			pMessageStrings[MSG_EMPTY_QUICK_SAVE_SLOT] :
			pMessageStrings[MSG_EMPTYSLOT];
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
		char zSavedGameName[512];
		CreateSavedGameFileNameFromNumber(bEntry, zSavedGameName);

		try
		{
			AutoSGPFile f(FileOpen(zSavedGameName, FILE_ACCESS_READ));
			FileRead(f, header, sizeof(*header));
			endof(header->zGameVersionNumber)[-1] =  '\0';
			endof(header->sSavedGameDesc)[-1]     = L'\0';
			return TRUE;
		}
		catch (...) { /* Handled below */ }

		gbSaveGameArray[bEntry] = FALSE;
	}
	memset(header, 0, sizeof(*header));
	return FALSE;
}


static void BtnSlgCancelCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		//Exit back
		if( gfCameDirectlyFromGame )
			SetSaveLoadExitScreen( guiPreviousOptionScreen );

		else if( guiPreviousOptionScreen == MAINMENU_SCREEN )
			SetSaveLoadExitScreen( MAINMENU_SCREEN );

		else
			SetSaveLoadExitScreen( OPTIONS_SCREEN );
	}
}


static void BtnSlgSaveLoadCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		SaveLoadGameNumber( gbSelectedSaveLocation );
	}
}


static void DisableSelectedSlot(void);
static void InitSaveLoadScreenTextInputBoxes(void);
static void RedrawSaveLoadScreenAfterMessageBox(MessageBoxReturnValue);


static void SelectedSaveRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT8		bActiveTextField;

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
		if( gfSaveGame && bSelected == 0 )
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
			gzGameDescTextField[0] = '\0';

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
//					SaveLoadGameNumber( gbSelectedSaveLocation );

					if( ( uiCurTime - uiLastTime ) < SLG_DOUBLE_CLICK_DELAY )
					{
						//Load the saved game
						SaveLoadGameNumber( gbSelectedSaveLocation );
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
					bActiveTextField = (INT8)GetActiveFieldID();
					if( bActiveTextField && bActiveTextField != -1 )
					{
						Get16BitStringFromField( (UINT8)bActiveTextField, gzGameDescTextField, lengthof(gzGameDescTextField));
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
					SaveLoadGameNumber( bSelected );
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
		if( gfSaveGame && MSYS_GetRegionUserData( pRegion, 0 ) != 0 )
		{
			return;
		}

		gbHighLightedLocation = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		DisplaySaveGameEntry( gbHighLightedLocation );//, usPosY );
	}
}


static void InitSaveLoadScreenTextInputBoxes(void)
{
	UINT16	usPosY;
	SAVED_GAME_HEADER SaveGameHeader;

	if( gbSelectedSaveLocation == -1 )
		return;

	if( !gfSaveGame )
		return;

	//if we are exiting, dont create the fields
	if( gfSaveLoadScreenExit || guiSaveLoadExitScreen != SAVE_LOAD_SCREEN )
		return;


	InitTextInputMode();

	SetTextInputCursor( CUROSR_IBEAM_WHITE );
	SetTextInputFont(FONT12ARIALFIXEDWIDTH); //FONT12ARIAL //FONT12ARIALFIXEDWIDTH
	Set16BPPTextFieldColor( Get16BPPColor(FROMRGB( 0, 0, 0) ) );
	SetBevelColors( Get16BPPColor(FROMRGB(136, 138, 135)), Get16BPPColor(FROMRGB(24, 61, 81)) );
	SetTextInputRegularColors( FONT_WHITE, 2 );
	SetTextInputHilitedColors( 2, FONT_WHITE, FONT_WHITE  );
	SetCursorColor( Get16BPPColor(FROMRGB(255, 255, 255) ) );

	AddUserInputField( NULL );

	usPosY = SLG_FIRST_SAVED_SPOT_Y + SLG_GAP_BETWEEN_LOCATIONS * gbSelectedSaveLocation;

	//if there is already a string here, use its string
	if( gbSaveGameArray[ gbSelectedSaveLocation ] )
	{
		//if we are modifying a previously modifed string, use it
		if( gzGameDescTextField[0] != '\0' )
		{
		}
		else
		{
			//Get the header for the specified saved game
			LoadSavedGameHeader( gbSelectedSaveLocation, &SaveGameHeader );
			wcscpy( gzGameDescTextField, SaveGameHeader.sSavedGameDesc );
		}
	}
	else
		gzGameDescTextField[0] = '\0';

	//Game Desc Field
	AddTextInputField( SLG_FIRST_SAVED_SPOT_X+SLG_SAVE_GAME_DESC_X, (INT16)(usPosY+SLG_SAVE_GAME_DESC_Y-5), SLG_SAVELOCATION_WIDTH-SLG_SAVE_GAME_DESC_X-7, 17, MSYS_PRIORITY_HIGH+2, gzGameDescTextField, 46, INPUTTYPE_ASCII );//23

	SetActiveField( 1 );

	gfUserInTextInputMode = TRUE;
}


static void DestroySaveLoadTextInputBoxes(void)
{
	gfUserInTextInputMode = FALSE;
	KillAllTextInputModes();
	SetTextInputCursor( CURSOR_IBEAM );
}


static void SetSelection(UINT8 ubNewSelection)
{
//	CHAR16		zMouseHelpTextString[256];
//	SAVED_GAME_HEADER SaveGameHeader;

	//if we are loading and there is no entry, return
 	if( !gfSaveGame )
	{
		if( !gbSaveGameArray[ubNewSelection] )
			return;
	}

	if( gbSelectedSaveLocation != -1 )
	{
		//reset the slots help text
		gSelectedSaveRegion[gbSelectedSaveLocation].SetFastHelpText(L"");
	}

	gfRedrawSaveLoadScreen = TRUE;
	DestroySaveLoadTextInputBoxes();

	//if we are loading,
	if( !gfSaveGame )
	{
		//Enable the save/load button
		EnableButton( guiSlgSaveLoadBtn );
	}

	//if we are to save
	if( gfSaveGame )
	{
		if( gbSelectedSaveLocation != ubNewSelection )
		{
			//Destroy the previous region
			DestroySaveLoadTextInputBoxes();

			//reset selected slot
			gbSelectedSaveLocation = ubNewSelection;

			//Null out the currently selected save game
			gzGameDescTextField[0] = '\0';

			//Init the text field for the game desc
			InitSaveLoadScreenTextInputBoxes();
		}

		//Enable the save/load button
		EnableButton( guiSlgSaveLoadBtn );
	}

	//reset selected slot
	gbSelectedSaveLocation = ubNewSelection;

/*
	//if we are saving AND it is the currently selected slot
	if( gfSaveGame && gbSelectedSaveLocation == ubNewSelection )
	{
		//copy over the initial game options
		SaveGameHeader.sInitialGameOptions = gGameOptions;
	}
	else
	{
		//Get the header for the specified saved game
		LoadSavedGameHeader(ubNewSelection, &SaveGameHeader))
	}

	swprintf(zMouseHelpTextString, L"%ls: %ls\n%ls: %ls\n%ls: %ls\n%ls: %ls", gzGIOScreenText[ GIO_DIF_LEVEL_TEXT ],
		gzGIOScreenText[ GIO_DIF_LEVEL_TEXT + SaveGameHeader.sInitialGameOptions.ubDifficultyLevel + 1 ],

		gzGIOScreenText[ GIO_TIMED_TURN_TITLE_TEXT ],
		gzGIOScreenText[ GIO_TIMED_TURN_TITLE_TEXT + SaveGameHeader.sInitialGameOptions.fTurnTimeLimit + 1],

		gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT ],
		gzGIOScreenText[ GIO_GUN_OPTIONS_TEXT + 2 - SaveGameHeader.sInitialGameOptions.fGunNut ],

		gzGIOScreenText[ GIO_GAME_STYLE_TEXT ],
		gzGIOScreenText[ GIO_GAME_STYLE_TEXT + SaveGameHeader.sInitialGameOptions.fSciFi + 1 ] );

	//set the slots help text
	gSelectedSaveRegion[gbSelectedSaveLocation].SetFastHelpText(zMouseHelpTextString);
	*/
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

void DeleteSaveGameNumber( UINT8 ubSaveGameSlotID )
{
	CHAR8		zSaveGameName[ 512 ];

	//Create the name of the file
	CreateSavedGameFileNameFromNumber( ubSaveGameSlotID, zSaveGameName );

	//Delete the saved game file
	FileDelete( zSaveGameName );
}


static void DisplayOnScreenNumber(BOOLEAN display)
{
	// Start at 1 - don't diplay it for the quicksave
	for (INT8 bLoopNum = 1; bLoopNum < NUM_SAVE_GAMES; ++bLoopNum)
	{
		const UINT16 usPosX = 6;
		const UINT16 usPosY = SLG_FIRST_SAVED_SPOT_Y + SLG_GAP_BETWEEN_LOCATIONS * bLoopNum;

		BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, usPosX, usPosY + SLG_DATE_OFFSET_Y, 10, 10);

		if (display)
		{
			const INT8 bNum = (bLoopNum == 10 ? 0 : bLoopNum);
			wchar_t zTempString[16];
			swprintf(zTempString, lengthof(zTempString), L"%2d", bNum);
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

	if (!LoadSavedGame(gbSelectedSaveLocation))
	{
		if (guiBrokenSaveGameVersion < 95 && !gfSchedulesHosed)
		{ // Hack problem with schedules getting misaligned.
			gfSchedulesHosed = TRUE;
			bool const success = LoadSavedGame(gbSelectedSaveLocation);
			gfSchedulesHosed = FALSE;
			if (success) goto SUCCESSFULLY_CORRECTED_SAVE;
		}
		DoSaveLoadMessageBox(zSaveLoadText[SLG_LOAD_GAME_ERROR], SAVE_LOAD_SCREEN, MSG_BOX_FLAG_OK, FailedLoadingGameCallBack);
		NextLoopCheckForEnoughFreeHardDriveSpace();
	}
	else
	{
SUCCESSFULLY_CORRECTED_SAVE:
#ifdef JA2BETAVERSION
		ValidateSoldierInitLinks(1);
#endif
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

	// yes, load the game
  if( bExitValue == MSG_BOX_RETURN_YES )
	{
		SaveGameToSlotNum();
	}
}


static void ConfirmLoadSavedGameMessageBoxCallBack(UINT8 bExitValue)
{
	Assert( gbSelectedSaveLocation != -1 );

	// yes, load the game
  if( bExitValue == MSG_BOX_RETURN_YES )
	{
		//Setup up the fade routines
		StartFadeOutForSaveLoadScreen();
	}
}


#ifdef JA2BETAVERSION
void ErrorDetectedInSaveCallback(MessageBoxReturnValue const bValue)
{
	//If we are to go to map screen after loading the game
	if( guiScreenToGotoAfterLoadingSavedGame == MAP_SCREEN )
	{
		gFadeInDoneCallback = DoneFadeInForSaveLoadScreen;
		SetSaveLoadExitScreen( guiScreenToGotoAfterLoadingSavedGame );
		FadeInNextFrame( );
	}
	else
	{
		//if we are to go to the Tactical screen after loading
		gFadeInDoneCallback = DoneFadeInForSaveLoadScreen;
		SetSaveLoadExitScreen( guiScreenToGotoAfterLoadingSavedGame );
		FadeInGameScreen( );
	}
	gfStartedFadingOut = TRUE;
}
#endif


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
	if (SaveGame(0, L"")) return;

	if (guiPreviousOptionScreen == MAP_SCREEN)
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
	else
		DoMessageBox(MSG_BOX_BASIC_STYLE, zSaveLoadText[SLG_SAVE_GAME_ERROR], GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
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


BOOLEAN IsThereAnySavedGameFiles()
{
	INT8	cnt;
	CHAR8		zSaveGameName[ 512 ];

	for( cnt=0; cnt<NUM_SAVE_GAMES; cnt++)
	{
		CreateSavedGameFileNameFromNumber( cnt, zSaveGameName );

		if( FileExists( zSaveGameName ) )
			return( TRUE );
	}

	return( FALSE );
}


static void RedrawSaveLoadScreenAfterMessageBox(MessageBoxReturnValue const bExitValue)
{
	gfRedrawSaveLoadScreen = TRUE;
}


static void MoveSelectionUpOrDown(BOOLEAN fUp)
{
	INT32	i;

	//if we are saving, any slot otgher then the quick save slot is valid
	if( gfSaveGame )
	{
		if( fUp )
		{
			//if there is no selected slot, get out
			if( gbSelectedSaveLocation == -1 )
				return;

			//if the selected slot is above the first slot
			if( gbSelectedSaveLocation > 1 )
			{
				SetSelection( (UINT8)(gbSelectedSaveLocation-1) );
			}
		}
		else
		{
			//if the selected slot is invalid
			if( gbSelectedSaveLocation == -1 )
			{
				SetSelection( 1 );
			}
			else
			{
				if( gbSelectedSaveLocation >= 1 && gbSelectedSaveLocation < NUM_SAVE_GAMES-1 )
				{
					SetSelection( (UINT8)(gbSelectedSaveLocation + 1) );
				}
			}
		}
	}

	else
	{
		if( fUp )
		{
			for( i=gbSelectedSaveLocation-1; i>=0; i--)
			{
				if( gbSaveGameArray[i] )
				{
					ClearSelectedSaveSlot();

					SetSelection( (UINT8)i );
					break;
				}
			}
		}
		else
		{
			//if there is no selected slot, move the selected slot to the first slot
			if( gbSelectedSaveLocation == -1 )
			{
				ClearSelectedSaveSlot();

				SetSelection( 0 );
			}
			else
			{
				for( i=gbSelectedSaveLocation+1; i<NUM_SAVE_GAMES; i++)
				{
					if( gbSaveGameArray[i] )
					{
						ClearSelectedSaveSlot();

						SetSelection( (UINT8)i );
						break;
					}
				}
			}
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

	if( !SaveGame( gbSelectedSaveLocation, gzGameDescTextField ) )
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
