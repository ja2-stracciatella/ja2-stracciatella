#include "Cinematics.h"
#include "Cursor_Control.h"
#include "Directories.h"
#include "English.h"
#include "FileMan.h"
#include "Game_Init.h"
#include "Input.h"
#include "Intro.h"
#include "Local.h"
#include "Line.h"
#include "MainMenuScreen.h"
#include "MouseSystem.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "Render_Dirty.h"
#include "Soldier_Profile.h"
#include "SysUtil.h"
#include "Text.h"
#include "VObject.h"
#include "Video.h"
#include "VSurface.h"
#include "UILayout.h"

#include "GameSettings.h"
#include "MessageBoxScreen.h"

static BOOLEAN gfIntroScreenEntry = TRUE;
static BOOLEAN gfIntroScreenExit;

static ScreenID guiIntroExitScreen = INTRO_SCREEN;


extern	BOOLEAN	gfDoneWithSplashScreen;

static SMKFLIC* gpSmackFlic = 0;

#define SMKINTRO_FIRST_VIDEO	255
#define SMKINTRO_NO_VIDEO	-1

//enums for the various smacker files
enum
{
	SMKINTRO_REBEL_CRDT,
	SMKINTRO_OMERTA,
	SMKINTRO_PRAGUE_CRDT,
	SMKINTRO_PRAGUE,

	//there are no more videos shown for the begining


	SMKINTRO_END_END_SPEECH_MIGUEL,
	SMKINTRO_END_END_SPEECH_NO_MIGUEL,
	SMKINTRO_END_HELI_FLYBY,
	SMKINTRO_END_SKYRIDER_HELICOPTER,
	SMKINTRO_END_NOSKYRIDER_HELICOPTER,

	SMKINTRO_SPLASH_SCREEN,
	SMKINTRO_SPLASH_TALONSOFT,

	//there are no more videos shown for the endgame
	SMKINTRO_LAST_END_GAME,
};

static INT32 giCurrentIntroBeingPlayed = SMKINTRO_NO_VIDEO;


static char const* const gpzSmackerFileNames[] =
{
	//begining of the game
	INTRODIR "/rebel_cr.smk",
	INTRODIR "/omerta.smk",
	INTRODIR "/prague_cr.smk",
	INTRODIR "/prague.smk",


	//endgame
	INTRODIR "/throne_mig.smk",
	INTRODIR "/throne_nomig.smk",
	INTRODIR "/heli_flyby.smk",
	INTRODIR "/heli_sky.smk",
	INTRODIR "/heli_nosky.smk",

	INTRODIR "/splashscreen.smk",
	INTRODIR "/talonsoftid_endhold.smk",
};


//enums used for when the intro screen can come up, either begining game intro, or end game cinematic
static INT8 gbIntroScreenMode = -1;


static void EnterIntroScreen(void);
static void ExitIntroScreen(void);
static void GetIntroScreenUserInput(void);
static void HandleIntroScreen(void);


ScreenID IntroScreenHandle(void)
{
	if( gfIntroScreenEntry )
	{
		EnterIntroScreen();
		gfIntroScreenEntry = FALSE;
		gfIntroScreenExit = FALSE;

		InvalidateScreen();
	}

	GetIntroScreenUserInput();

	HandleIntroScreen();

	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();


	if( gfIntroScreenExit )
	{
		ExitIntroScreen();
		gfIntroScreenExit = FALSE;
		gfIntroScreenEntry = TRUE;
	}

	return( guiIntroExitScreen );
}


static INT32 GetNextIntroVideo(UINT32 uiCurrentVideo);
static void PrepareToExitIntroScreen(void);
static void StartPlayingIntroFlic(INT32 iIndexOfFlicToPlay);


static void EnterIntroScreen(void)
{
	INT32 iFirstVideoID = -1;

	ClearMainMenu();


	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	// Don't play music....
	SetMusicMode( MUSIC_NONE );

	SmkInitialize();

	//get the index opf the first video to watch
	iFirstVideoID = GetNextIntroVideo( SMKINTRO_FIRST_VIDEO );


	if( iFirstVideoID != -1 )
	{

		StartPlayingIntroFlic( iFirstVideoID );

		guiIntroExitScreen = INTRO_SCREEN;
	}

	//Got no intro video, exit
	else
	{
		PrepareToExitIntroScreen();
	}
}


static void ExitIntroScreen(void)
{
	//shutdown smaker
	SmkShutdown();
}


static void HandleIntroScreen(void)
{
	BOOLEAN	fFlicStillPlaying = FALSE;

	//if we are exiting this screen, this frame, dont update the screen
	if( gfIntroScreenExit )
		return;

	//handle smaker each frame
	fFlicStillPlaying = SmkPollFlics();

	//if the flic is not playing
	if( !fFlicStillPlaying )
	{
		INT32 iNextVideoToPlay = -1;

		iNextVideoToPlay = GetNextIntroVideo( giCurrentIntroBeingPlayed );

		if( iNextVideoToPlay != -1 )
		{
			StartPlayingIntroFlic( iNextVideoToPlay );
		}
		else
		{
			PrepareToExitIntroScreen();
			giCurrentIntroBeingPlayed = -1;
		}
	}

	InvalidateScreen();
}


static void GetIntroScreenUserInput(void)
{
	SGPPoint MousePos;
	GetMousePos(&MousePos);

	InputAtom Event;
	while( DequeueEvent( &Event ) )
	{
		MouseSystemHook(Event.usEvent, MousePos.iX, MousePos.iY);

		if( Event.usEvent == KEY_UP )
		{
			switch( Event.usParam )
			{
				case SDLK_ESCAPE: PrepareToExitIntroScreen(); break;
				case SDLK_SPACE:  SmkCloseFlic(gpSmackFlic);  break;
			}
		}
	}

	// if the user presses either mouse button
	if( gfLeftButtonState || gfRightButtonState )
	{
		//advance to the next flic
		SmkCloseFlic( gpSmackFlic );
	}
}


static void DisplaySirtechSplashScreen(void);


static void PrepareToExitIntroScreen(void)
{
	//if its the intro at the begining of the game
	if( gbIntroScreenMode == INTRO_BEGINING )
	{
		//go to the init screen
		guiIntroExitScreen = INIT_SCREEN;
	}
	else if( gbIntroScreenMode == INTRO_SPLASH )
	{
		//display a logo when exiting
		DisplaySirtechSplashScreen();

		gfDoneWithSplashScreen = TRUE;
		guiIntroExitScreen = INIT_SCREEN;
	}
	else
	{
		//We want to reinitialize the game
		ReStartingGame();

//		guiIntroExitScreen = MAINMENU_SCREEN;
		guiIntroExitScreen = CREDIT_SCREEN;
	}

	gfIntroScreenExit = TRUE;
}


static INT32 GetNextIntroVideo(UINT32 uiCurrentVideo)
{
	INT32 iStringToUse = -1;

	//switch on whether it is the beginging or the end game video
	switch( gbIntroScreenMode )
	{
		//the video at the begining of the game
		case INTRO_BEGINING:
		{
			switch( uiCurrentVideo )
			{
				case SMKINTRO_FIRST_VIDEO:
					iStringToUse = SMKINTRO_REBEL_CRDT;
					break;
				case SMKINTRO_REBEL_CRDT:
					iStringToUse = SMKINTRO_OMERTA;
					break;
				case SMKINTRO_OMERTA:
					iStringToUse = SMKINTRO_PRAGUE_CRDT;
					break;
				case SMKINTRO_PRAGUE_CRDT:
					iStringToUse = SMKINTRO_PRAGUE;
					break;
				case SMKINTRO_PRAGUE:
					iStringToUse = -1;
					break;
//				case SMKINTRO_LAST_INTRO:
//					iStringToUse = -1;
//					break;
			}
		}
		break;

		//end game
		case INTRO_ENDING:
		{
			switch( uiCurrentVideo )
			{
				case SMKINTRO_FIRST_VIDEO:
					//if Miguel is dead, play the flic with out him in it
					if( gMercProfiles[ MIGUEL ].bMercStatus == MERC_IS_DEAD )
						iStringToUse = SMKINTRO_END_END_SPEECH_NO_MIGUEL;
					else
						iStringToUse = SMKINTRO_END_END_SPEECH_MIGUEL;
					break;

				case SMKINTRO_END_END_SPEECH_MIGUEL:
				case SMKINTRO_END_END_SPEECH_NO_MIGUEL:
					iStringToUse = SMKINTRO_END_HELI_FLYBY;
					break;

				//if SkyRider is dead, play the flic without him
				case SMKINTRO_END_HELI_FLYBY:
					if( gMercProfiles[ SKYRIDER ].bMercStatus == MERC_IS_DEAD )
						iStringToUse = SMKINTRO_END_NOSKYRIDER_HELICOPTER;
					else
						iStringToUse = SMKINTRO_END_SKYRIDER_HELICOPTER;
					break;
			}
		}
		break;

		case INTRO_SPLASH:
			switch( uiCurrentVideo )
			{
				case SMKINTRO_FIRST_VIDEO:
					iStringToUse = SMKINTRO_SPLASH_SCREEN;
					break;
				case SMKINTRO_SPLASH_SCREEN:
					//iStringToUse = SMKINTRO_SPLASH_TALONSOFT;
					break;
			}
			break;
	}

	return( iStringToUse );
}


static void StartPlayingIntroFlic(INT32 iIndexOfFlicToPlay)
{
	if( iIndexOfFlicToPlay != -1 )
	{
		//start playing a flic
		gpSmackFlic = SmkPlayFlic( gpzSmackerFileNames[ iIndexOfFlicToPlay ], STD_SCREEN_X, STD_SCREEN_Y , TRUE );

		if( gpSmackFlic != NULL )
		{
			giCurrentIntroBeingPlayed = iIndexOfFlicToPlay;
		}
		else
		{
			//do a check
			DoScreenIndependantMessageBox(gzIntroScreen, MSG_BOX_FLAG_OK, CDromEjectionErrorMessageBoxCallBack);
		}
	}
}


void SetIntroType( INT8 bIntroType )
{
	if( bIntroType == INTRO_BEGINING )
	{
		gbIntroScreenMode = INTRO_BEGINING;
	}
	else if( bIntroType == INTRO_ENDING )
	{
		gbIntroScreenMode = INTRO_ENDING;
	}
	else if( bIntroType == INTRO_SPLASH )
	{
		gbIntroScreenMode = INTRO_SPLASH;
	}
}


static void DisplaySirtechSplashScreen(void)
{
	FRAME_BUFFER->Fill(0);
	BltVideoObjectOnce(FRAME_BUFFER, INTERFACEDIR "/sirtechsplash.sti", 0, STD_SCREEN_X, STD_SCREEN_Y);
	InvalidateScreen();
	RefreshScreen();
}
