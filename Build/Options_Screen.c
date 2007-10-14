#include "Font.h"
#include "Local.h"
#include "Types.h"
#include "Options_Screen.h"
#include "Video.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Render_Dirty.h"
#include "Text_Input.h"
#include "WordWrap.h"
#include "SaveLoadScreen.h"
#include "WCheck.h"
#include "Utilities.h"
#include "Slider.h"
#include "Debug.h"
#include "Music_Control.h"
#include "Sound_Control.h"
#include "SoundMan.h"
#include "Ambient_Control.h"
#include "WorldDat.h"
#include "WorldDef.h"
#include "GameSettings.h"
#include "Game_Init.h"
#include "English.h"
#include "Overhead.h"
#include "Gap.h"
#include "Cursors.h"
#include "SysUtil.h"
#include "Exit_Grids.h"
#include "Text.h"
#include "Interface_Control.h"
#include "Message.h"
#include "Language_Defines.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Map_Information.h"
#include "SmokeEffects.h"
#include "Button_System.h"
#include "JAScreens.h"
#include "ScreenIDs.h"
#include "VSurface.h"
#include "WorldMan.h"
#include "Stubs.h" // XXX


#define		OPTIONS_TITLE_FONT										FONT14ARIAL
#define		OPTIONS_TITLE_COLOR										FONT_MCOLOR_WHITE

#define		OPT_MAIN_FONT													FONT12ARIAL
#define		OPT_MAIN_COLOR												OPT_BUTTON_ON_COLOR//FONT_MCOLOR_WHITE
#define		OPT_HIGHLIGHT_COLOR										FONT_MCOLOR_WHITE//FONT_MCOLOR_LTYELLOW


#define		OPTIONS_SCREEN_WIDTH									440
#define		OPTIONS_SCREEN_HEIGHT									400


#define		OPTIONS__TOP_LEFT_X										100
#define		OPTIONS__TOP_LEFT_Y										40
#define		OPTIONS__BOTTOM_RIGHT_X								OPTIONS__TOP_LEFT_X + OPTIONS_SCREEN_WIDTH
#define		OPTIONS__BOTTOM_RIGHT_Y								OPTIONS__TOP_LEFT_Y + OPTIONS_SCREEN_HEIGHT

#define OPT_SAVE_BTN_X  51
#define OPT_LOAD_BTN_X 190
#define OPT_QUIT_BTN_X 329
#define OPT_DONE_BTN_X 469
#define OPT_BTN_Y      438


#define		OPT_GAP_BETWEEN_TOGGLE_BOXES					31//40


//Text
#define		OPT_TOGGLE_BOX_FIRST_COL_TEXT_X				OPT_TOGGLE_BOX_FIRST_COLUMN_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX//350
#define		OPT_TOGGLE_BOX_SECOND_TEXT_X					OPT_TOGGLE_BOX_SECOND_COLUMN_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX//350


//toggle boxes
#define		OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX		30//220
#define		OPT_TOGGLE_TEXT_OFFSET_Y								2//3

#define		OPT_TOGGLE_BOX_FIRST_COLUMN_X						265 //257 //OPT_TOGGLE_BOX_TEXT_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX
#define		OPT_TOGGLE_BOX_SECOND_COLUMN_X					428 //OPT_TOGGLE_BOX_TEXT_X + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX
#define   OPT_TOGGLE_BOX_START_Y                  89

#define		OPT_TOGGLE_BOX_TEXT_WIDTH								OPT_TOGGLE_BOX_SECOND_COLUMN_X - OPT_TOGGLE_BOX_FIRST_COLUMN_X - 20

// Slider bar defines
#define		OPT_GAP_BETWEEN_SLIDER_BARS					60
//#define		OPT_SLIDER_BAR_WIDTH								200
#define		OPT_SLIDER_BAR_SIZE									258

#define		OPT_SLIDER_TEXT_WIDTH								45

#define		OPT_SOUND_FX_TEXT_X									38
#define		OPT_SOUND_FX_TEXT_Y									87//116//110

#define		OPT_SPEECH_TEXT_X										85//OPT_SOUND_FX_TEXT_X + OPT_SLIDER_TEXT_WIDTH
#define		OPT_SPEECH_TEXT_Y										OPT_SOUND_FX_TEXT_Y//OPT_SOUND_FX_TEXT_Y + OPT_GAP_BETWEEN_SLIDER_BARS

#define		OPT_MUSIC_TEXT_X										137
#define		OPT_MUSIC_TEXT_Y										OPT_SOUND_FX_TEXT_Y//OPT_SPEECH_TEXT_Y + OPT_GAP_BETWEEN_SLIDER_BARS

#define		OPT_TEXT_TO_SLIDER_OFFSET_Y					25

#define		OPT_SOUND_EFFECTS_SLIDER_X					56
#define		OPT_SOUND_EFFECTS_SLIDER_Y					126//110 + OPT_TEXT_TO_SLIDER_OFFSET_Y

#define		OPT_SPEECH_SLIDER_X									107
#define		OPT_SPEECH_SLIDER_Y									OPT_SOUND_EFFECTS_SLIDER_Y

#define		OPT_MUSIC_SLIDER_X									158
#define		OPT_MUSIC_SLIDER_Y									OPT_SOUND_EFFECTS_SLIDER_Y

#define		OPT_MUSIC_SLIDER_PLAY_SOUND_DELAY		75


#define		OPT_FIRST_COLUMN_TOGGLE_CUT_OFF			10//8


UINT32		guiOptionBackGroundImage;
UINT32		guiOptionsAddOnImages;

UINT32		guiSoundEffectsSliderID;
UINT32		guiSpeechSliderID;
UINT32		guiMusicSliderID;

BOOLEAN		gfOptionsScreenEntry = TRUE;
BOOLEAN		gfOptionsScreenExit	= FALSE;
BOOLEAN		gfRedrawOptionsScreen = TRUE;

CHAR8			gzSavedGameName[ 128 ];
BOOLEAN		gfEnteredFromMapScreen=FALSE;

UINT32		guiOptionsScreen = OPTIONS_SCREEN;
UINT32		guiPreviousOptionScreen = OPTIONS_SCREEN;

BOOLEAN		gfExitOptionsDueToMessageBox=FALSE;
BOOLEAN		gfExitOptionsAfterMessageBox = FALSE;


UINT32		guiSoundFxSliderMoving = 0xffffffff;
UINT32		guiSpeechSliderMoving = 0xffffffff;

static INT32 giOptionsMessageBox = -1; // Options pop up messages index value

INT8			gbHighLightedOptionText = -1;

BOOLEAN		gfHideBloodAndGoreOption=FALSE;		//If a germany build we are to hide the blood and gore option
UINT8			gubFirstColOfOptions=OPT_FIRST_COLUMN_TOGGLE_CUT_OFF;


BOOLEAN		gfSettingOfTreeTopStatusOnEnterOfOptionScreen;
BOOLEAN		gfSettingOfItemGlowStatusOnEnterOfOptionScreen;
BOOLEAN   gfSettingOfDontAnimateSmoke;

static INT32  giOptionsButtonImages;
static UINT32 guiOptGotoSaveGameBtn;
static UINT32 guiOptGotoLoadGameBtn;
static UINT32 guiQuitButton;
static UINT32 guiDoneButton;


//checkbox to toggle tracking mode on or off
UINT32	guiOptionsToggles[ NUM_GAME_OPTIONS ];
static void BtnOptionsTogglesCallback(GUI_BUTTON* btn, INT32 reason);


//Mouse regions for the name of the option
MOUSE_REGION    gSelectedOptionTextRegion[ NUM_GAME_OPTIONS ];


//Mouse regions for the area around the toggle boxs
MOUSE_REGION    gSelectedToggleBoxAreaRegion;


UINT32	OptionsScreenInit()
{

	//Set so next time we come in, we can set up
	gfOptionsScreenEntry = TRUE;


	return( TRUE );
}


static BOOLEAN EnterOptionsScreen(void);
static void ExitOptionsScreen(void);
static void GetOptionsScreenUserInput(void);
static void HandleOptionsScreen(void);
static void RenderOptionsScreen(void);


UINT32	OptionsScreenHandle()
{
	StartFrameBufferRender();

	if( gfOptionsScreenEntry )
	{
		PauseGame();
		EnterOptionsScreen();
		gfOptionsScreenEntry = FALSE;
		gfOptionsScreenExit = FALSE;
		gfRedrawOptionsScreen = TRUE;
		RenderOptionsScreen();

		//Blit the background to the save buffer
		BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
		InvalidateRegion(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	RestoreBackgroundRects();

	GetOptionsScreenUserInput();

	HandleOptionsScreen();

	if( gfRedrawOptionsScreen )
	{
		RenderOptionsScreen();
		RenderButtons();

		gfRedrawOptionsScreen = FALSE;
	}

	//Render the active slider bars
	RenderAllSliderBars();

	// render buttons marked dirty
  MarkButtonsDirty( );
	RenderButtons( );

	// ATE: Put here to save RECTS before any fast help being drawn...
	SaveBackgroundRects( );
	RenderButtonsFastHelp();


	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();


	if( gfOptionsScreenExit )
	{
		ExitOptionsScreen();
		gfOptionsScreenExit = FALSE;
		gfOptionsScreenEntry = TRUE;

		UnPauseGame();
	}

	return( guiOptionsScreen );
}



UINT32	OptionsScreenShutdown()
{
	return( TRUE );
}


static INT32 MakeButton(INT16 x, GUI_CALLBACK click, const wchar_t* text)
{
	return CreateIconAndTextButton(giOptionsButtonImages, text, OPT_BUTTON_FONT, OPT_BUTTON_ON_COLOR, DEFAULT_SHADOW, OPT_BUTTON_OFF_COLOR, DEFAULT_SHADOW, x, OPT_BTN_Y, MSYS_PRIORITY_HIGH, click);
}


static void BtnOptGotoSaveGameCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnOptGotoLoadGameCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnOptQuitCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnDoneCallback(GUI_BUTTON* btn, INT32 reason);
static void MusicSliderChangeCallBack(INT32 iNewValue);
static void SelectedOptionTextRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectedOptionTextRegionMovementCallBack(MOUSE_REGION* pRegion, INT32 reason);
static void SelectedToggleBoxAreaRegionMovementCallBack(MOUSE_REGION* pRegion, INT32 reason);
static void SetOptionsScreenToggleBoxes(void);
static void SoundFXSliderChangeCallBack(INT32 iNewValue);
static void SpeechSliderChangeCallBack(INT32 iNewValue);


static BOOLEAN EnterOptionsScreen(void)
{
	//Default this to off
	gfHideBloodAndGoreOption=FALSE;

#ifndef BLOOD_N_GORE_ENABLED
	//This will hide blood and gore option
	gfHideBloodAndGoreOption = TRUE;

/*
Uncomment this to enable the check for files to activate the blood and gore option for the german build
	if( !FileExists( "Germany.dat" ) && FileExists( "Lecken.dat" ) )
	{
		gfHideBloodAndGoreOption = FALSE;
	}
	else
*/
	{
		gGameSettings.fOptions[ TOPTION_BLOOD_N_GORE ] = FALSE;
	}
#endif


	//if we are coming from mapscreen
	if( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN )
	{
		guiTacticalInterfaceFlags &= ~INTERFACE_MAPSCREEN;
		gfEnteredFromMapScreen = TRUE;
	}


	// Stop ambients...
	StopAmbients( );

	guiOptionsScreen = OPTIONS_SCREEN;

	//Init the slider bar;
	InitSlider();

	if( gfExitOptionsDueToMessageBox )
	{
		gfRedrawOptionsScreen = TRUE;
		gfExitOptionsDueToMessageBox = FALSE;
		return( TRUE );
	}

	gfExitOptionsDueToMessageBox = FALSE;

	// load the options screen background graphic and add it
	guiOptionBackGroundImage = AddVideoObjectFromFile("INTERFACE/OptionScreenBase.sti");
	CHECKF(guiOptionBackGroundImage != NO_VOBJECT);

	// load button, title graphic and add it
	SGPFILENAME ImageFile;
	GetMLGFilename(ImageFile, MLG_OPTIONHEADER);
	guiOptionsAddOnImages = AddVideoObjectFromFile(ImageFile);
	CHECKF(guiOptionsAddOnImages != NO_VOBJECT);

	giOptionsButtonImages = LoadButtonImage("INTERFACE/OptionScreenAddons.sti", -1, 2, -1, 3, -1);

	//Save game button
	guiOptGotoSaveGameBtn = MakeButton(OPT_SAVE_BTN_X, BtnOptGotoSaveGameCallback, zOptionsText[OPT_SAVE_GAME]);
	SpecifyDisabledButtonStyle( guiOptGotoSaveGameBtn, DISABLED_STYLE_HATCHED );
	if( guiPreviousOptionScreen == MAINMENU_SCREEN || !CanGameBeSaved() )
	{
		DisableButton( guiOptGotoSaveGameBtn );
	}

	guiOptGotoLoadGameBtn = MakeButton(OPT_LOAD_BTN_X, BtnOptGotoLoadGameCallback, zOptionsText[OPT_LOAD_GAME]);
	guiQuitButton         = MakeButton(OPT_QUIT_BTN_X, BtnOptQuitCallback,         zOptionsText[OPT_MAIN_MENU]);
	guiDoneButton         = MakeButton(OPT_DONE_BTN_X, BtnDoneCallback,            zOptionsText[OPT_DONE]);

	// Toggle Boxes
	UINT16 usTextHeight = GetFontHeight(OPT_MAIN_FONT);

	//Create the first column of check boxes
	UINT32 pos_x = OPT_TOGGLE_BOX_FIRST_COLUMN_X;
	UINT16 pos_y = OPT_TOGGLE_BOX_START_Y;
	gubFirstColOfOptions = OPT_FIRST_COLUMN_TOGGLE_CUT_OFF;
	for (UINT8 cnt = 0; cnt < NUM_GAME_OPTIONS; cnt++)
	{
		//if this is the blood and gore option, and we are to hide the option
		if (cnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption)
		{
			gubFirstColOfOptions++;
			//advance to the next
			continue;
		}
		if (cnt == gubFirstColOfOptions)
		{
			pos_y = OPT_TOGGLE_BOX_START_Y;
			pos_x = OPT_TOGGLE_BOX_SECOND_COLUMN_X;
		}

		//Check box to toggle tracking mode
		INT32 check = CreateCheckBoxButton(pos_x, pos_y, "INTERFACE/OptionsCheckBoxes.sti", MSYS_PRIORITY_HIGH + 10, BtnOptionsTogglesCallback);
		guiOptionsToggles[cnt] = check;
		MSYS_SetBtnUserData(check, cnt);

		UINT32 height;
		UINT16 usTextWidth = StringPixLength(zOptionsToggleText[cnt], OPT_MAIN_FONT);
		if (usTextWidth > OPT_TOGGLE_BOX_TEXT_WIDTH)
		{
			//Get how many lines will be used to display the string, without displaying the string
			usTextWidth = OPT_TOGGLE_BOX_TEXT_WIDTH;
			height = DisplayWrappedString(0, 0, OPT_TOGGLE_BOX_TEXT_WIDTH, 2, OPT_MAIN_FONT, OPT_HIGHLIGHT_COLOR, zOptionsToggleText[cnt], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | DONT_DISPLAY_TEXT);
		}
		else
		{
			height = usTextHeight;
		}
		MOUSE_REGION* reg = &gSelectedOptionTextRegion[cnt];
		MSYS_DefineRegion(reg, pos_x + 13, pos_y, pos_x + OPT_SPACE_BETWEEN_TEXT_AND_TOGGLE_BOX + usTextWidth, pos_y + height, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, SelectedOptionTextRegionMovementCallBack, SelectedOptionTextRegionCallBack);
		MSYS_SetRegionUserData(reg, 0, cnt);

		SetRegionFastHelpText(reg, zOptionsScreenHelpText[cnt]);
		SetButtonFastHelpText(check, zOptionsScreenHelpText[cnt]);

		pos_y += OPT_GAP_BETWEEN_TOGGLE_BOXES;
	}

	//Create a mouse region so when the user leaves a togglebox text region we can detect it then unselect the region
	MSYS_DefineRegion(&gSelectedToggleBoxAreaRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_NORMAL, CURSOR_NORMAL, SelectedToggleBoxAreaRegionMovementCallBack, MSYS_NO_CALLBACK);

	//Render the scene before adding the slider boxes
	RenderOptionsScreen();

	//Add a slider bar for the Sound Effects
	guiSoundEffectsSliderID = AddSlider(SLIDER_VERTICAL_STEEL, CURSOR_NORMAL, OPT_SOUND_EFFECTS_SLIDER_X, OPT_SOUND_EFFECTS_SLIDER_Y, OPT_SLIDER_BAR_SIZE, MAXVOLUME, MSYS_PRIORITY_HIGH, SoundFXSliderChangeCallBack, 0);
	AssertMsg( guiSoundEffectsSliderID, "Failed to AddSlider" );
	SetSliderValue( guiSoundEffectsSliderID, GetSoundEffectsVolume() );

	//Add a slider bar for the Speech
	guiSpeechSliderID = AddSlider(SLIDER_VERTICAL_STEEL, CURSOR_NORMAL, OPT_SPEECH_SLIDER_X, OPT_SPEECH_SLIDER_Y, OPT_SLIDER_BAR_SIZE, MAXVOLUME, MSYS_PRIORITY_HIGH, SpeechSliderChangeCallBack, 0);
	AssertMsg( guiSpeechSliderID, "Failed to AddSlider" );
	SetSliderValue( guiSpeechSliderID, GetSpeechVolume() );

	//Add a slider bar for the Music
	guiMusicSliderID = AddSlider(SLIDER_VERTICAL_STEEL, CURSOR_NORMAL, OPT_MUSIC_SLIDER_X, OPT_MUSIC_SLIDER_Y, OPT_SLIDER_BAR_SIZE, MAXVOLUME, MSYS_PRIORITY_HIGH, MusicSliderChangeCallBack, 0);
	AssertMsg( guiMusicSliderID, "Failed to AddSlider" );
	SetSliderValue( guiMusicSliderID, MusicGetVolume() );



	//Remove the mouse region over the clock
	RemoveMouseRegionForPauseOfClock(  );

	//Draw the screen
	gfRedrawOptionsScreen = TRUE;

	//Set the option screen toggle boxes
	SetOptionsScreenToggleBoxes();

	DisableScrollMessages();

	//reset
	gbHighLightedOptionText = -1;

	//get the status of the tree top option
	gfSettingOfTreeTopStatusOnEnterOfOptionScreen = gGameSettings.fOptions[ TOPTION_TOGGLE_TREE_TOPS ];

	//Get the status of the item glow option
	gfSettingOfItemGlowStatusOnEnterOfOptionScreen = gGameSettings.fOptions[ TOPTION_GLOW_ITEMS ];

  gfSettingOfDontAnimateSmoke = gGameSettings.fOptions[ TOPTION_ANIMATE_SMOKE ];
	return( TRUE );
}


static void GetOptionsScreenToggleBoxes(void);


static void ExitOptionsScreen(void)
{
	UINT8	cnt;

	if( gfExitOptionsDueToMessageBox )
	{
		gfOptionsScreenExit = FALSE;

		if( !gfExitOptionsAfterMessageBox )
			return;
		gfExitOptionsAfterMessageBox = FALSE;
		gfExitOptionsDueToMessageBox = FALSE;
	}

	//Get the current status of the toggle boxes
	GetOptionsScreenToggleBoxes();
	//The save the current settings to disk
	SaveGameSettings();

	CreateMouseRegionForPauseOfClock();

	if( guiOptionsScreen == GAME_SCREEN )
		EnterTacticalScreen( );

	RemoveButton( guiOptGotoSaveGameBtn );
	RemoveButton( guiOptGotoLoadGameBtn );
	RemoveButton( guiQuitButton );
	RemoveButton( guiDoneButton );

	UnloadButtonImage(giOptionsButtonImages);

	DeleteVideoObjectFromIndex( guiOptionBackGroundImage );
	DeleteVideoObjectFromIndex( guiOptionsAddOnImages );


	//Remove the toggle buttons
	for( cnt=0; cnt<NUM_GAME_OPTIONS; cnt++)
	{
		//if this is the blood and gore option, and we are to hide the option
		if( cnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption )
		{
			//advance to the next
			continue;
		}

		RemoveButton( guiOptionsToggles[ cnt ] );

	  MSYS_RemoveRegion( &gSelectedOptionTextRegion[cnt]);
	}


	//REmove the slider bars
	RemoveSliderBar( guiSoundEffectsSliderID );
	RemoveSliderBar( guiSpeechSliderID );
	RemoveSliderBar( guiMusicSliderID );


  MSYS_RemoveRegion( &gSelectedToggleBoxAreaRegion );

	ShutDownSlider();

	//if we are coming from mapscreen
	if( gfEnteredFromMapScreen )
	{
		gfEnteredFromMapScreen = FALSE;
		guiTacticalInterfaceFlags |= INTERFACE_MAPSCREEN;
	}

	//if the user changed the  TREE TOP option, AND a world is loaded
	if( gfSettingOfTreeTopStatusOnEnterOfOptionScreen != gGameSettings.fOptions[ TOPTION_TOGGLE_TREE_TOPS ] && gfWorldLoaded )
	{
		SetTreeTopStateForMap();
	}

	//if the user has changed the item glow option AND a world is loaded
	if( gfSettingOfItemGlowStatusOnEnterOfOptionScreen != gGameSettings.fOptions[ TOPTION_GLOW_ITEMS ] && gfWorldLoaded )
	{
		ToggleItemGlow( gGameSettings.fOptions[ TOPTION_GLOW_ITEMS ] );
	}

	if( gfSettingOfDontAnimateSmoke != gGameSettings.fOptions[ TOPTION_ANIMATE_SMOKE ] && gfWorldLoaded )
	{
    UpdateSmokeEffectGraphics( );
  }

}


static void HandleHighLightedText(BOOLEAN fHighLight);
static void HandleSliderBarMovementSounds(void);


static void HandleOptionsScreen(void)
{
	HandleSliderBarMovementSounds();

	HandleHighLightedText( TRUE );
}


static void RenderOptionsScreen(void)
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiOptionBackGroundImage, 0, 0, 0);

	//Get and display the titla image
	HVOBJECT hPixHandle = GetVideoObject(guiOptionsAddOnImages);
  BltVideoObject(FRAME_BUFFER, hPixHandle, 0, 0, 0);
  BltVideoObject(FRAME_BUFFER, hPixHandle, 1, 0, 434);



	//
	// Text for the toggle boxes
	//

	UINT32 pos_x = OPT_TOGGLE_BOX_FIRST_COL_TEXT_X;
	UINT16 pos_y = OPT_TOGGLE_BOX_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y;
	for (UINT8 cnt = 0; cnt < NUM_GAME_OPTIONS; cnt++)
	{
		//if this is the blood and gore option, and we are to hide the option
		if( cnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption )
		{
		//advance to the next
			continue;
		}
		if (cnt == gubFirstColOfOptions)
		{
			pos_x = OPT_TOGGLE_BOX_SECOND_TEXT_X;
			pos_y = OPT_TOGGLE_BOX_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y;
		}

		UINT16 usWidth = StringPixLength(zOptionsToggleText[cnt], OPT_MAIN_FONT);

		//if the string is going to wrap, move the string up a bit
		if( usWidth > OPT_TOGGLE_BOX_TEXT_WIDTH )
			DisplayWrappedString(pos_x, pos_y, OPT_TOGGLE_BOX_TEXT_WIDTH, 2, OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsToggleText[cnt], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		else
			DrawTextToScreen(zOptionsToggleText[cnt], pos_x, pos_y, 0, OPT_MAIN_FONT, OPT_MAIN_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

		pos_y += OPT_GAP_BETWEEN_TOGGLE_BOXES;
	}

	//
	// Text for the Slider Bars
	//

	//Display the Sound Fx text
	DisplayWrappedString(OPT_SOUND_FX_TEXT_X, OPT_SOUND_FX_TEXT_Y, OPT_SLIDER_TEXT_WIDTH, 2, OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsText[OPT_SOUND_FX], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display the Speech text
	DisplayWrappedString(OPT_SPEECH_TEXT_X, OPT_SPEECH_TEXT_Y, OPT_SLIDER_TEXT_WIDTH, 2, OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsText[OPT_SPEECH], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display the Music text
	DisplayWrappedString(OPT_MUSIC_TEXT_X, OPT_MUSIC_TEXT_Y, OPT_SLIDER_TEXT_WIDTH, 2, OPT_MAIN_FONT, OPT_MAIN_COLOR, zOptionsText[OPT_MUSIC], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	InvalidateRegion( OPTIONS__TOP_LEFT_X, OPTIONS__TOP_LEFT_Y, OPTIONS__BOTTOM_RIGHT_X, OPTIONS__BOTTOM_RIGHT_Y);
}


static void SetOptionsExitScreen(UINT32 uiExitScreen);


static void GetOptionsScreenUserInput(void)
{
	InputAtom Event;
	POINT  MousePos;

	GetCursorPos(&MousePos);

	while( DequeueEvent( &Event ) )
	{
		MouseSystemHook(Event.usEvent, MousePos.x, MousePos.y);

		if( !HandleTextInput( &Event ) && Event.usEvent == KEY_DOWN )
		{
			switch( Event.usParam )
			{
				case SDLK_ESCAPE: SetOptionsExitScreen(guiPreviousOptionScreen); break;

				//Enter the save game screen
				case SDLK_s:
					//if the save game button isnt disabled
					if( ButtonList[ guiOptGotoSaveGameBtn ]->uiFlags & BUTTON_ENABLED )
					{
						SetOptionsExitScreen( SAVE_LOAD_SCREEN );
						gfSaveGame = TRUE;
					}
					break;

				//Enter the Load game screen
				case SDLK_l:
					SetOptionsExitScreen( SAVE_LOAD_SCREEN );
					gfSaveGame = FALSE;
					break;


#ifdef JA2TESTVERSION

				case 'r':
					gfRedrawOptionsScreen = TRUE;
					break;

				case 'i':
					InvalidateRegion(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
					break;

					//Test keys

				case 'y':
					{
					static	UINT32	uiTest2 = NO_SAMPLE;
					if( !SoundIsPlaying( uiTest2 ) )
						uiTest2 = PlayJA2SampleFromFile("Sounds/RAID Dive.wav", HIGHVOLUME, 1, MIDDLEPAN);
					}
					break;

				case 'q':
//					ShouldMercSayPrecedentToRepeatOneSelf( 11, 99 );
					break;
#endif
			}
		}
	}
}


static void SetOptionsExitScreen(UINT32 uiExitScreen)
{
	guiOptionsScreen = uiExitScreen;
	gfOptionsScreenExit	= TRUE;
}


static void BtnOptGotoSaveGameCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SetOptionsExitScreen( SAVE_LOAD_SCREEN );
		gfSaveGame = TRUE;
	}
}


static void BtnOptGotoLoadGameCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SetOptionsExitScreen( SAVE_LOAD_SCREEN );
		gfSaveGame = FALSE;
	}
}


static void ConfirmQuitToMainMenuMessageBoxCallBack(UINT8 bExitValue);
static BOOLEAN DoOptionsMessageBox(UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback);


static void BtnOptQuitCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//Confirm the Exit to the main menu screen
		DoOptionsMessageBox(MSG_BOX_BASIC_STYLE, zOptionsText[OPT_RETURN_TO_MAIN], OPTIONS_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmQuitToMainMenuMessageBoxCallBack);
	}
}


static void BtnDoneCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SetOptionsExitScreen(guiPreviousOptionScreen);
	}
}


static void HandleOptionToggle(UINT8 ubButton, BOOLEAN fState, BOOLEAN fDown, BOOLEAN fPlaySound);


static void BtnOptionsTogglesCallback(GUI_BUTTON* btn, INT32 reason)
{
	BOOLEAN down;
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		down = FALSE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		down = TRUE;
	}
	else
	{
		return;
	}
	const BOOLEAN clicked  = (btn->uiFlags & BUTTON_CLICKED_ON) != 0;
	const UINT8   ubButton = MSYS_GetBtnUserData(btn);
	HandleOptionToggle(ubButton, clicked, down, FALSE);
}


static void HandleOptionToggle(UINT8 ubButton, BOOLEAN fState, BOOLEAN fDown, BOOLEAN fPlaySound)
{
	gGameSettings.fOptions[ubButton] = fState;

	GUI_BUTTON* const b = ButtonList[guiOptionsToggles[ubButton]];
	b->uiFlags &= ~BUTTON_CLICKED_ON;
	b->uiFlags |= (fState ? BUTTON_CLICKED_ON : 0);

	if (fDown) DrawCheckBoxButtonOnOff(guiOptionsToggles[ubButton], fState);

	/* Check if the user is unselecting either the spech or subtitles toggle.
	 * Make sure that at least one of the toggles is still enabled. */
	if (!fState &&
			(ubButton == TOPTION_SPEECH || ubButton == TOPTION_SUBTITLES) &&
			!(ButtonList[guiOptionsToggles[TOPTION_SPEECH]]->uiFlags    & BUTTON_CLICKED_ON) &&
			!(ButtonList[guiOptionsToggles[TOPTION_SUBTITLES]]->uiFlags & BUTTON_CLICKED_ON))
	{
		gGameSettings.fOptions[ubButton] = TRUE;
		b->uiFlags |= BUTTON_CLICKED_ON;

		//Confirm the Exit to the main menu screen
		DoOptionsMessageBox(MSG_BOX_BASIC_STYLE, zOptionsText[OPT_NEED_AT_LEAST_SPEECH_OR_SUBTITLE_OPTION_ON], OPTIONS_SCREEN, MSG_BOX_FLAG_OK, NULL);
		gfExitOptionsDueToMessageBox = FALSE;
	}

	if (fPlaySound)
	{
		const UINT32 sound = (fDown ? BIG_SWITCH3_IN : BIG_SWITCH3_OUT);
		PlayJA2Sample(sound, BTNVOLUME, 1, MIDDLEPAN);
	}
}


static void SoundFXSliderChangeCallBack(INT32 iNewValue)
{
	SetSoundEffectsVolume( iNewValue );

	guiSoundFxSliderMoving = GetJA2Clock();
}


static void SpeechSliderChangeCallBack(INT32 iNewValue)
{
	SetSpeechVolume( iNewValue );

	guiSpeechSliderMoving = GetJA2Clock();
}


static void MusicSliderChangeCallBack(INT32 iNewValue)
{
	MusicSetVolume( iNewValue );
}


BOOLEAN DoOptionsMessageBoxWithRect(UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect)
{
	// reset exit mode
	gfExitOptionsDueToMessageBox = TRUE;

	// do message box and return
	giOptionsMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen, usFlags, ReturnCallback, pCenteringRect);

	// send back return state
	return giOptionsMessageBox != -1;
}


static BOOLEAN DoOptionsMessageBox(UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	return DoOptionsMessageBoxWithRect(ubStyle, zString, uiExitScreen, usFlags, ReturnCallback, NULL);
}


static void ConfirmQuitToMainMenuMessageBoxCallBack(UINT8 bExitValue)
{
	// yes, Quit to main menu
  if( bExitValue == MSG_BOX_RETURN_YES )
	{
		gfEnteredFromMapScreen = FALSE;
		gfExitOptionsAfterMessageBox = TRUE;
		SetOptionsExitScreen( MAINMENU_SCREEN );

		//We want to reinitialize the game
		ReStartingGame();
	}
	else
	{
		gfExitOptionsAfterMessageBox = FALSE;
		gfExitOptionsDueToMessageBox = FALSE;
	}
}


static void SetOptionsScreenToggleBoxes(void)
{
	UINT8	cnt;

	for( cnt=0; cnt<NUM_GAME_OPTIONS; cnt++)
	{
		if( gGameSettings.fOptions[ cnt ] )
			ButtonList[ guiOptionsToggles[ cnt ] ]->uiFlags |= BUTTON_CLICKED_ON;
		else
			ButtonList[ guiOptionsToggles[ cnt ] ]->uiFlags &= (~BUTTON_CLICKED_ON );
	}
}


static void GetOptionsScreenToggleBoxes(void)
{
	UINT8	cnt;

	for( cnt=0; cnt<NUM_GAME_OPTIONS; cnt++)
	{
		if( ButtonList[ guiOptionsToggles[ cnt ] ]->uiFlags & BUTTON_CLICKED_ON )
			gGameSettings.fOptions[ cnt ] = TRUE;
		else
			gGameSettings.fOptions[ cnt ] = FALSE;
	}
}


static void HandleSliderBarMovementSounds(void)
{
	static UINT32	uiLastSoundFxTime=0;
	static UINT32	uiLastSpeechTime=0;
	static UINT32	uiLastPlayingSoundID = NO_SAMPLE;
	static UINT32	uiLastPlayingSpeechID = NO_SAMPLE;

	if( ( uiLastSoundFxTime - OPT_MUSIC_SLIDER_PLAY_SOUND_DELAY ) > guiSoundFxSliderMoving )
	{
		guiSoundFxSliderMoving = 0xffffffff;

		//The slider has stopped moving, reset the ambient sector sounds ( so it will change the volume )
		if( !DidGameJustStart() )
			HandleNewSectorAmbience( gTilesets[ giCurrentTilesetID ].ubAmbientID );

		if( !SoundIsPlaying( uiLastPlayingSoundID ) )
			uiLastPlayingSoundID = PlayJA2SampleFromFile("Sounds/Weapons/LMG Reload.wav", HIGHVOLUME, 1, MIDDLEPAN);
	}
	else
		uiLastSoundFxTime = GetJA2Clock();


	if( ( uiLastSpeechTime - OPT_MUSIC_SLIDER_PLAY_SOUND_DELAY ) > guiSpeechSliderMoving )
	{
		guiSpeechSliderMoving = 0xffffffff;

		if( !SoundIsPlaying( uiLastPlayingSpeechID ) )
			uiLastPlayingSpeechID = PlayJA2GapSample("BattleSnds/m_cool.wav", HIGHVOLUME, 1, MIDDLEPAN, NULL);
	}
	else
		uiLastSpeechTime = GetJA2Clock();
}


static void SelectedOptionTextRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8	ubButton = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		HandleOptionToggle(ubButton, !gGameSettings.fOptions[ubButton], FALSE, TRUE);
		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		HandleOptionToggle(ubButton, gGameSettings.fOptions[ubButton], TRUE, TRUE);
	}
}


static void SelectedOptionTextRegionMovementCallBack(MOUSE_REGION* pRegion, INT32 reason)
{
	INT8	bButton = (INT8)MSYS_GetRegionUserData( pRegion, 0 );

	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{

		HandleHighLightedText( FALSE );

		gbHighLightedOptionText = -1;

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		gbHighLightedOptionText = bButton;

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
}


static void HandleHighLightedText(BOOLEAN fHighLight)
{
	UINT16		usPosX=0;
	UINT16		usPosY=0;
	UINT8			ubCnt;
	INT8			bHighLight=-1;
	UINT16		usWidth;

	static	INT8	bLastRegion = -1;

	if( gbHighLightedOptionText == -1 )
		fHighLight = FALSE;

	//if the user has the mouse in one of the checkboxes
	for( ubCnt=0; ubCnt<NUM_GAME_OPTIONS;ubCnt++)
	{
		if( ubCnt == TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption )
		{
			//advance to the next
			continue;
		}

		if( ButtonList[ guiOptionsToggles[ ubCnt ] ]->Area.uiFlags & MSYS_MOUSE_IN_AREA )
		{
			gbHighLightedOptionText = ubCnt;
			fHighLight = TRUE;
		}
	}

	// If there is a valid section being highlighted
	if( gbHighLightedOptionText != -1 )
	{
		bLastRegion = gbHighLightedOptionText;
	}

	bHighLight = gbHighLightedOptionText;


	if( bLastRegion != -1 && gbHighLightedOptionText == -1 )
	{
		fHighLight = FALSE;
		bHighLight = bLastRegion;
		bLastRegion = -1;
	}

	//If we are to hide the blood and gore option, and we are to highlight an option past the blood and gore option
	//reduce the highlight number by 1
	if( bHighLight >= TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption )
	{
		bHighLight--;
	}

	if( bHighLight != -1 )
	{
		if( bHighLight < OPT_FIRST_COLUMN_TOGGLE_CUT_OFF )
		{
			usPosX = OPT_TOGGLE_BOX_FIRST_COL_TEXT_X;
			usPosY = OPT_TOGGLE_BOX_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y + bHighLight * OPT_GAP_BETWEEN_TOGGLE_BOXES;
		}
		else
		{
			usPosX = OPT_TOGGLE_BOX_SECOND_TEXT_X;
			usPosY = OPT_TOGGLE_BOX_START_Y + OPT_TOGGLE_TEXT_OFFSET_Y + (bHighLight - OPT_FIRST_COLUMN_TOGGLE_CUT_OFF) * OPT_GAP_BETWEEN_TOGGLE_BOXES;
		}

		//If we are to hide the blood and gore option, and we are to highlight an option past the blood and gore option
		//reduce the highlight number by 1
		if( bHighLight >= TOPTION_BLOOD_N_GORE && gfHideBloodAndGoreOption )
		{
			bHighLight++;
		}


		usWidth = StringPixLength( zOptionsToggleText[ bHighLight ], OPT_MAIN_FONT );

		//if the string is going to wrap, move the string up a bit
		UINT8 color = fHighLight ? OPT_HIGHLIGHT_COLOR : OPT_MAIN_COLOR;
		if( usWidth > OPT_TOGGLE_BOX_TEXT_WIDTH )
		{
			DisplayWrappedString(usPosX, usPosY, OPT_TOGGLE_BOX_TEXT_WIDTH, 2, OPT_MAIN_FONT, color, zOptionsToggleText[bHighLight], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | MARK_DIRTY);
		}
		else
		{
			DrawTextToScreen(zOptionsToggleText[bHighLight], usPosX, usPosY, 0, OPT_MAIN_FONT, color, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | MARK_DIRTY);
		}
	}
}


static void SelectedToggleBoxAreaRegionMovementCallBack(MOUSE_REGION* pRegion, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		UINT8	ubCnt;

		//loop through all the toggle box's and remove the in area flag
		for( ubCnt=0;ubCnt<NUM_GAME_OPTIONS;ubCnt++)
		{
			ButtonList[ guiOptionsToggles[ ubCnt ] ]->Area.uiFlags &= ~MSYS_MOUSE_IN_AREA;
		}

		gbHighLightedOptionText = -1;

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
}
