#include <stdexcept>

#include "GameLoop.h"
#include "GameVersion.h"
#include "Local.h"
#include "SGP.h"
#include "Screens.h"
#include "ShopKeeper_Interface.h"
#include "Tactical_Placement_GUI.h"
#include "Cursors.h"
#include "Init.h"
#include "Music_Control.h"
#include "Sys_Globals.h"
#include "Laptop.h"
#include "MapScreen.h"
#include "Game_Clock.h"
#include "Overhead.h"
#include "LibraryDataBase.h"
#include "Map_Screen_Interface.h"
#include "Tactical_Save.h"
#include "Interface.h"
#include "GameSettings.h"
#include "Text.h"
#include "HelpScreen.h"
#include "SaveLoadGame.h"
#include "Finances.h"
#include "Options_Screen.h"
#include "Debug.h"
#include "Video.h"
#include "MemMan.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "UILayout.h"

#ifdef JA2BETAVERSION
#	include "PreBattle_Interface.h"
#endif

#ifdef JA2DEMOADS
#	include "Fade_Screen.h"
#endif


ScreenID guiCurrentScreen = ERROR_SCREEN; // XXX TODO001A had no explicit initialisation
ScreenID guiPendingScreen = NO_PENDING_SCREEN;

#define	DONT_CHECK_FOR_FREE_SPACE		255
static UINT8 gubCheckForFreeSpaceOnHardDriveCount = DONT_CHECK_FOR_FREE_SPACE;

// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game

#ifdef JA2BETAVERSION

BOOLEAN gubReportMapscreenLock = 0;


static void ReportMapscreenErrorLock(void)
{
	switch( gubReportMapscreenLock )
	{
		case 1:
			DoScreenIndependantMessageBox( L"You have just loaded the game which is in a state that you shouldn't be able to.  You can still play, but there should be a sector with enemies co-existing with mercs.  Please don't report that.", MSG_BOX_FLAG_OK, NULL );
			fDisableDueToBattleRoster = FALSE;
			fDisableMapInterfaceDueToBattle = FALSE;
			gubReportMapscreenLock = 0;
			break;
		case 2:
			DoScreenIndependantMessageBox( L"You have just saved the game which is in a state that you shouldn't be able to.  Please report circumstances (ex:  merc in other sector pipes up about enemies), etc.  Autocorrected, but if you reload the save, don't report the error appearing in load.", MSG_BOX_FLAG_OK, NULL );
			fDisableDueToBattleRoster = FALSE;
			fDisableMapInterfaceDueToBattle = FALSE;
			gubReportMapscreenLock = 0;
			break;
	}
}
#endif


void InitializeGame(void)
{
	UINT32				uiIndex;

	// Initlaize mouse subsystems
	MSYS_Init( );
	InitButtonSystem();
	InitCursors( );

	// Init Fonts
	InitializeFonts();

	InitTacticalSave();

	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Version Label: %s", g_version_label));
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, String("Version #:     %s", g_version_number));

	// Initialize Game Screens.
  for (uiIndex = 0; uiIndex < MAX_SCREENS; uiIndex++)
  {
		void (*const init)(void) = GameScreens[uiIndex].InitializeScreen;
		if (init) init();
  }

	//Init the help screen system
	InitHelpScreenSystem();

	//Loads the saved (if any) general JA2 game settings
	LoadGameSettings();

	//Initialize the Game options ( Gun nut, scifi and dif. levels
	InitGameOptions();

	// preload mapscreen graphics
	HandlePreloadOfMapGraphics( );

	guiCurrentScreen = INIT_SCREEN;
}


// The ShutdownGame function will free up/undo all things that were started in InitializeGame()
// It will also be responsible to making sure that all Gaming Engine tasks exit properly

void    ShutdownGame(void)
{
	// handle shutdown of game with respect to preloaded mapscreen graphics
	HandleRemovalOfPreLoadedMapGraphics( );

	 ShutdownJA2( );

	//Save the general save game settings to disk
	SaveGameSettings();


	 //shutdown the file database manager
	 ShutDownFileDatabase( );

	InitTacticalSave();
}


static void InsertCommasIntoNumber(wchar_t pString[])
{
  INT16 sCounter = 0;
  INT16 sZeroCount = 0;
	INT16 sTempCounter = 0;

	// go to end of dollar figure
	while (pString[sCounter] != L'\0')
	{
		sCounter++;
	}

	// is there under $1,000?
	if (sCounter < 4)
	{
		// can't do anything, return
		return;
	}

	// at end, start backing up until beginning
  while (sCounter > 0)
	{
		// enough for a comma?
		if (sZeroCount == 3)
		{
			// reset count
			sZeroCount = 0;
      // set tempcounter to current counter
			sTempCounter = sCounter;

			// run until end
			while (pString[sTempCounter] != L'\0')
			{
				sTempCounter++;
			}
			// now shift everything over ot the right one place until sTempCounter = sCounter
			while (sTempCounter >= sCounter)
			{
				pString[sTempCounter + 1] = pString[sTempCounter];
				sTempCounter--;
			}
			// now insert comma
			pString[sCounter] = L',';
		}

		// increment count of digits
		sZeroCount++;

		// decrement counter
		sCounter--;
	}
}


static void HandleNewScreenChange(UINT32 uiNewScreen, UINT32 uiOldScreen);


// This is the main Gameloop. This should eventually by one big switch statement which represents
// the state of the game (i.e. Main Menu, PC Generation, Combat loop, etc....)
// This function exits constantly and reenters constantly
void GameLoop(void)
try
{
  InputAtom					InputEvent;
	ScreenID uiOldScreen = guiCurrentScreen;

	SGPPoint MousePos;
	GetMousePos(&MousePos);
	// Hook into mouse stuff for MOVEMENT MESSAGES
	MouseSystemHook(MOUSE_POS, MousePos.iX, MousePos.iY);
	MusicPoll();

  while (DequeueSpecificEvent(&InputEvent, MOUSE_EVENTS))
  {
		MouseSystemHook(InputEvent.usEvent, MousePos.iX, MousePos.iY);
	}


	if ( gfGlobalError )
	{
		guiCurrentScreen = ERROR_SCREEN;
	}


	//if we are to check for free space on the hard drive
	if( gubCheckForFreeSpaceOnHardDriveCount < DONT_CHECK_FOR_FREE_SPACE )
	{
		//only if we are in a screen that can get this check
		if( guiCurrentScreen == MAP_SCREEN || guiCurrentScreen == GAME_SCREEN || guiCurrentScreen == SAVE_LOAD_SCREEN )
		{
			if( gubCheckForFreeSpaceOnHardDriveCount < 1 )
			{
				gubCheckForFreeSpaceOnHardDriveCount++;
			}
			else
			{
				// Make sure the user has enough hard drive space
				if( !DoesUserHaveEnoughHardDriveSpace() )
				{
					wchar_t	zText[512];
					wchar_t	zSpaceOnDrive[512];
					UINT32	uiSpaceOnDrive;
					wchar_t	zSizeNeeded[512];

					swprintf( zSizeNeeded, lengthof(zSizeNeeded), L"%d", REQUIRED_FREE_SPACE / BYTESINMEGABYTE );
					InsertCommasIntoNumber(zSizeNeeded);

					uiSpaceOnDrive = GetFreeSpaceOnHardDriveWhereGameIsRunningFrom( );

					swprintf( zSpaceOnDrive, lengthof(zSpaceOnDrive), L"%.2f", uiSpaceOnDrive / (FLOAT)BYTESINMEGABYTE );

					swprintf( zText, lengthof(zText), pMessageStrings[ MSG_LOWDISKSPACE_WARNING ], zSpaceOnDrive, zSizeNeeded );

					if( guiPreviousOptionScreen == MAP_SCREEN )
						DoMapMessageBox( MSG_BOX_BASIC_STYLE, zText, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL );
					else
						DoMessageBox( MSG_BOX_BASIC_STYLE, zText, GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL );
				}
				gubCheckForFreeSpaceOnHardDriveCount = DONT_CHECK_FOR_FREE_SPACE;
			}
		}
	}

	// ATE: Force to be in message box screen!
	if ( gfInMsgBox )
	{
		guiPendingScreen = MSG_BOX_SCREEN;
	}

	if ( guiPendingScreen != NO_PENDING_SCREEN )
	{
		// Based on active screen, deinit!
		if( guiPendingScreen != guiCurrentScreen )
		{
			switch( guiCurrentScreen )
			{
				case MAP_SCREEN:
					if( guiPendingScreen != MSG_BOX_SCREEN )
					{
						EndMapScreen( FALSE );
					}
					break;
				case LAPTOP_SCREEN:
					ExitLaptop();
					break;
			}
		}

		// if the screen has chnaged
		if( uiOldScreen != guiPendingScreen )
		{
			// Set the fact that the screen has changed
			uiOldScreen = guiPendingScreen;

			HandleNewScreenChange( guiPendingScreen, guiCurrentScreen );
		}
		guiCurrentScreen = guiPendingScreen;
		guiPendingScreen = NO_PENDING_SCREEN;

	}



  uiOldScreen = (*(GameScreens[guiCurrentScreen].HandleScreen))();

	// if the screen has chnaged
	if( uiOldScreen != guiCurrentScreen )
	{
		HandleNewScreenChange( uiOldScreen, guiCurrentScreen );
		guiCurrentScreen = uiOldScreen;
	}

	RefreshScreen();

	guiGameCycleCounter++;

	UpdateClock();

	#ifdef JA2BETAVERSION
	if( gubReportMapscreenLock )
	{
		ReportMapscreenErrorLock();
	}
	#endif
}
catch (std::exception const& e)
{
	guiPreviousOptionScreen = guiCurrentScreen;
	char const* what;
	char const* success = "failed";
	char const* attach  = "";
#if defined JA2EDITOR
	if (gfEditMode)
	{
		what = "map";
		if (SaveWorld("error.dat"))
		{
			success = "succeeded (error.dat)";
			attach  = " Do not forget to attach the map.";
		}
	}
	else
#endif
	{
		what = "savegame";
		if (SaveGame(SAVE__ERROR_NUM, L"error savegame"))
		{
			success = "succeeded (error.sav)";
			attach  = " Do not forget to attach the savegame.";
		}
	}
	char msg[2048];
	snprintf(msg, lengthof(msg),
		"%s\n"
		"Creating an emergency %s %s.\n"
		"Please report this error with a description of the circumstances.%s",
		e.what(), what, success, attach
	);
	throw std::runtime_error(msg);
}


void SetPendingNewScreen(ScreenID const uiNewScreen)
{
	guiPendingScreen = uiNewScreen;
}


// Gets called when the screen changes, place any needed in code in here
static void HandleNewScreenChange(UINT32 uiNewScreen, UINT32 uiOldScreen)
{
	//if we are not going into the message box screen, and we didnt just come from it
	if( ( uiNewScreen != MSG_BOX_SCREEN && uiOldScreen != MSG_BOX_SCREEN ) )
	{
		//reset the help screen
		NewScreenSoResetHelpScreen( );
	}
}


void HandleShortCutExitState()
{
	// Use YES/NO pop up box, setup for particular screen
	switch (guiCurrentScreen)
	{
#ifdef JA2BETAVERSION
		case AIVIEWER_SCREEN:
		case QUEST_DEBUG_SCREEN:
#endif
		case DEBUG_SCREEN:
		case EDIT_SCREEN:
		case ERROR_SCREEN:
			// Do not prompt if error or editor
			gfProgramIsRunning = FALSE;
			break;

		case LAPTOP_SCREEN:
			DoLapTopSystemMessageBox(pMessageStrings[MSG_EXITGAME], LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO, EndGameMessageBoxCallBack);
			break;

		case MAP_SCREEN:
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMessageStrings[MSG_EXITGAME], MAP_SCREEN, MSG_BOX_FLAG_YESNO, EndGameMessageBoxCallBack);
			return;

		case SHOPKEEPER_SCREEN:
			DoSkiMessageBox(pMessageStrings[MSG_EXITGAME], SHOPKEEPER_SCREEN, MSG_BOX_FLAG_YESNO, EndGameMessageBoxCallBack);
			break;

		default:
		{ // set up for all otherscreens
			SGPBox const pCenteringRect = { 0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y };
			DoMessageBox(MSG_BOX_BASIC_STYLE, pMessageStrings[MSG_EXITGAME], guiCurrentScreen, MSG_BOX_FLAG_YESNO, EndGameMessageBoxCallBack, &pCenteringRect);
			break;
		}
	}
}


void EndGameMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, so start over, else stay here and do nothing for now
  if( bExitValue == MSG_BOX_RETURN_YES )
	{
#ifdef JA2DEMOADS
		guiPendingScreen = DEMO_EXIT_SCREEN;
		SetMusicMode( MUSIC_MAIN_MENU );
		FadeOutNextFrame( );
#else
		gfProgramIsRunning = FALSE;
#endif
	}

	//If we are in the tactical placement gui, we need this flag set so the interface is updated.
	if( gfTacticalPlacementGUIActive )
	{
		gfTacticalPlacementGUIDirty = TRUE;
		gfValidLocationsChanged = TRUE;
	}
}


void NextLoopCheckForEnoughFreeHardDriveSpace()
{
	gubCheckForFreeSpaceOnHardDriveCount = 0;
}
