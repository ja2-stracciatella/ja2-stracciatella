#include "SGP.h"
#include "Gameloop.h"
#include "HImage.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "WCheck.h"
#include "Input.h"
#include "Font.h"
#include "MouseSystem.h"
#include "ScreenIDs.h"
#include "Screens.h"
#include "Font_Control.h"
#include "SysUtil.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "EditScreen.h"
#include "Timer_Control.h"
#include "Sys_Globals.h"
#include "Interface.h"
#include "Overhead.h"
#include "Utilities.h"
#include "Render_Dirty.h"
#include "JAScreens.h"
#include "Environment.h"
#include "Animation_Cache.h"
#include "Lighting.h"
#include "Music_Control.h"
#include "Sound_Control.h"
#include "MainMenuScreen.h"
#include "Game_Init.h"
#include "Init.h"
#include "Cursor_Control.h"
#include "GameVersion.h"
#include "Game_Clock.h"
#include "GameScreen.h"
#include "English.h"
#include "Random.h"
#include "JA2_Demo_Ads.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Text.h"
#include "Language_Defines.h"
#include "Video.h"
#include "Debug.h"

#ifdef JA2DEMO
#	include "Local.h"
#	include "WordWrap.h"
#endif

#define _UNICODE

#define MAX_DEBUG_PAGES 4


// GLOBAL FOR PAL EDITOR
UINT8	 CurrentPalette = 0;
UINT32  guiBackgroundRect;
BOOLEAN	gfExitPalEditScreen = FALSE;
BOOLEAN	gfExitDebugScreen = FALSE;
BOOLEAN gfInitRect = TRUE;
static BOOLEAN FirstTime = TRUE;
BOOLEAN	gfDoneWithSplashScreen = FALSE;


INT8 gCurDebugPage = 0;


HVSURFACE			hVAnims[ 7 ];
INT8					bTitleAnimFrame = 0;
UINT32				uiTitleAnimTime = 0;
UINT32				uiDoTitleAnimTime = 0;
BOOLEAN				gfDoTitleAnimation = FALSE;
BOOLEAN				gfStartTitleAnimation = FALSE;


static void DefaultDebugPage1(void);
static void DefaultDebugPage2(void);
static void DefaultDebugPage3(void);
static void DefaultDebugPage4(void);


RENDER_HOOK				gDebugRenderOverride[ MAX_DEBUG_PAGES ] =
{
	DefaultDebugPage1,
	DefaultDebugPage2,
	DefaultDebugPage3,
	DefaultDebugPage4
};


void DisplayFrameRate( )
{
	static UINT32		uiFPS = 0;
	static UINT32		uiFrameCount = 0;
	UINT16 usMapPos;

	// Increment frame count
	uiFrameCount++;

	if ( COUNTERDONE( FPSCOUNTER ) )
	{
		// Reset counter
		RESETCOUNTER( FPSCOUNTER );

		uiFPS = uiFrameCount;
		uiFrameCount = 0;
	}

	// Create string
	SetFont( SMALLFONT1 );

	//DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String( "FPS: %d ", __min( uiFPS, 1000 ) ) );

	if ( uiFPS < 20 )
	{
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_LTRED );
	}
	else
	{
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_DKGRAY );
	}

	if ( gbFPSDisplay == SHOW_FULL_FPS )
	{
		// FRAME RATE
		SetVideoOverlayTextF(giFPSOverlay, L"%ld", __min(uiFPS, 1000));

		// TIMER COUNTER
		SetVideoOverlayTextF(giCounterPeriodOverlay, L"%ld", __min(giTimerDiag, 1000));

		if( GetMouseMapPos( &usMapPos) )
		{
			//gprintfdirty( 0, 315, L"(%d)",usMapPos);
			//mprintf( 0,315,L"(%d)",usMapPos);
		}
		else
		{
			//gprintfdirty( 0, 315, L"(%d %d)",gusMouseXPos, gusMouseYPos - INTERFACE_START_Y );
			//mprintf( 0,315,L"(%d %d)",gusMouseXPos, gusMouseYPos - INTERFACE_START_Y );
		}
	}

	if ( ( gTacticalStatus.uiFlags & GODMODE ) )
	{
		SetFont( SMALLFONT1 );
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_DKRED );
		//gprintfdirty( 0, 0, L"GOD MODE" );
		//mprintf( 0, 0, L"GOD MODE" );
	}

#ifdef _DEBUG

	SetFont( SMALLFONT1 );
	SetFontBackground( FONT_MCOLOR_BLACK );
	SetFontForeground( FONT_MCOLOR_DKGRAY );

	if ( gbFPSDisplay == SHOW_FULL_FPS )
	{
		// Print Num tiles
		//gprintfdirty( 0, 280, L"%d Tiles", gTileDatabaseSize );
		//mprintf( 0, 280, L"%d Tiles", gTileDatabaseSize );

		// Print tile mem usage
		//gprintfdirty( 0, 300, L"%d kB", guiMemTotal / 1024 );
		//mprintf( 0, 300, L"%d kB", guiMemTotal / 1024 );

	}
#endif

}


//USELESS!!!!!!!!!!!!!!!!!!
UINT32 SavingScreenInitialize(void)
{
	return( TRUE );
}
UINT32 SavingScreenHandle( void )
{
	return SAVING_SCREEN;
}
UINT32 SavingScreenShutdown( void )
{
	return TRUE;
}

UINT32 LoadingScreenInitialize(void)
{
	return( TRUE );
}
UINT32 LoadingScreenHandle(void)
{
	return LOADING_SCREEN;
}
UINT32 LoadingScreenShutdown(void)
{
	return( TRUE );
}



UINT32 ErrorScreenInitialize(void)
{
	return( TRUE );
}

UINT32 ErrorScreenHandle(void)
{
  InputAtom  InputEvent;
	static BOOLEAN	fFirstTime = FALSE;

	// For quick setting of new video stuff / to be changed
	StartFrameBufferRender( );

	// Create string
	SetFont( LARGEFONT1 );
	SetFontBackground( FONT_MCOLOR_BLACK );
	SetFontForeground( FONT_MCOLOR_LTGRAY );
	mprintf( 50, 200, L"RUNTIME ERROR" );

	mprintf( 50, 225, L"PRESS <ESC> TO EXIT" );

	SetFont( FONT12ARIAL );
	SetFontForeground( FONT_YELLOW );
	SetFontShadow( 60 );		 //60 is near black
	mprintf(50, 255, L"%hs", gubErrorText);
	SetFontForeground( FONT_LTRED );

	if ( !fFirstTime )
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String( "Runtime Error: %s ", gubErrorText ) );
		fFirstTime = TRUE;
	}

	// For quick setting of new video stuff / to be changed
	InvalidateScreen( );
	EndFrameBufferRender( );

	// Check for esc
  while (DequeueEvent(&InputEvent) == TRUE)
  {
      if( InputEvent.usEvent == KEY_DOWN )
			{
				if (InputEvent.usParam == SDLK_ESCAPE || InputEvent.usParam == 'x' && InputEvent.usKeyState & ALT_DOWN)
				{ // Exit the program
					DebugMsg(TOPIC_GAME, DBG_LEVEL_0, "GameLoop: User pressed ESCape, TERMINATING");

					// handle shortcut exit
					HandleShortCutExitState( );
				}
			}
	}

	return( ERROR_SCREEN );
}

UINT32 ErrorScreenShutdown(void)
{
	return( TRUE );
}



UINT32 InitScreenInitialize(void)
{
	return( TRUE );
}

UINT32 InitScreenHandle(void)
{
	static UINT8					ubCurrentScreen = 255;

	if ( ubCurrentScreen == 255 )
	{
	#ifdef ENGLISH
		if( gfDoneWithSplashScreen )
		{
			ubCurrentScreen = 0;
		}
		else
		{
			SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
			return( INTRO_SCREEN );
		}
	#else
		ubCurrentScreen = 0;
	#endif
	}

	if ( ubCurrentScreen == 0 )
	{
		ubCurrentScreen = 1;

		// Init screen

		// Set Font
		SetFont( TINYFONT1 );
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_WHITE );

		//mprintf( 10, 420, zVersionLabel );

		mprintf( 10, 430,
#ifdef _DEBUG
			L"%ls: %ls (Debug %hs)",
#else
			L"%ls: %ls (%hs)",
#endif
		pMessageStrings[ MSG_VERSION ], zVersionLabel, czVersionNumber );

#ifdef JA2BETAVERSION

		mprintf( 10, 420, L"(Beta version error reporting enabled)" );

#endif

#ifdef _DEBUG
		mprintf( 10, 440, L"SOLDIERTYPE: %d bytes", sizeof( SOLDIERTYPE ) );
#endif

		InvalidateScreen( );

		//ATE: Set to true to reset before going into main screen!

		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

		return( INIT_SCREEN );
	}

	if ( ubCurrentScreen == 1 )
	{
		ubCurrentScreen = 2;
		return( InitializeJA2( ) );
	}

	if ( ubCurrentScreen == 2 )
	{
		InitMainMenu( );
		ubCurrentScreen = 3;
		return( INIT_SCREEN );
	}

	// Let one frame pass....
	if ( ubCurrentScreen == 3 )
	{
		ubCurrentScreen = 4;
		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
		return( INIT_SCREEN );
	}

	if ( ubCurrentScreen == 4 )
	{
		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
		InitNewGame( FALSE );
	}
	return( INIT_SCREEN );
}


UINT32 InitScreenShutdown(void)
{

	return( TRUE );
}



UINT32 PalEditScreenInit(void)
{
	return( TRUE );
}


static BOOLEAN PalEditKeyboardHook(InputAtom* pInputEvent);
static void PalEditRenderHook(void);


UINT32 PalEditScreenHandle(void)
{
	static BOOLEAN FirstTime = TRUE;

	if ( gfExitPalEditScreen )
	{
		gfExitPalEditScreen = FALSE;
		FirstTime = TRUE;
		FreeBackgroundRect( guiBackgroundRect );
		SetRenderHook(NULL);
		SetUIKeyboardHook(NULL);
		return( GAME_SCREEN );
	}

	if ( FirstTime )
	{
		FirstTime = FALSE;

		SetRenderHook(PalEditRenderHook);
		SetUIKeyboardHook(PalEditKeyboardHook);

		guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, 50, 10, 600 , 400);
	}
	else
	{

		(*(GameScreens[GAME_SCREEN].HandleScreen))();

	}

	return( PALEDIT_SCREEN );

}

UINT32 PalEditScreenShutdown(void)
{

	return( TRUE );
}


static void PalEditRenderHook(void)
{
	if ( gusSelectedSoldier != NO_SOLDIER )
	{
		// Set to current
		const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
		DisplayPaletteRep( pSoldier->HeadPal, 50, 10, FRAME_BUFFER );
		DisplayPaletteRep( pSoldier->PantsPal, 50, 50, FRAME_BUFFER );
		DisplayPaletteRep( pSoldier->VestPal, 50, 90, FRAME_BUFFER );
		DisplayPaletteRep( pSoldier->SkinPal, 50, 130, FRAME_BUFFER );
	}
}


static BOOLEAN PalEditKeyboardHook(InputAtom* pInputEvent)
{
	UINT8					ubType;
	UINT8					ubPaletteRep;
	UINT32				cnt;
	UINT8					ubStartRep = 0;
	UINT8					ubEndRep = 0;

	if ( gusSelectedSoldier == NO_SOLDIER )
	{
		return( FALSE );
	}

  if (pInputEvent->usEvent == KEY_DOWN && pInputEvent->usParam == SDLK_ESCAPE)
  {
		gfExitPalEditScreen = TRUE;
		return( TRUE );
	}

  if ((pInputEvent->usEvent == KEY_DOWN )&& ( pInputEvent->usParam == 'h' ))
  {
			SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);

			// Get index of current
			CHECKF( GetPaletteRepIndexFromID( pSoldier->HeadPal, &ubPaletteRep ) );
			ubType = gpPalRep[ ubPaletteRep ].ubType;

			ubPaletteRep++;

			// Count start and end index
			for ( cnt = 0; cnt < ubType; cnt++ )
			{
				ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ cnt ]);
			}

			ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ ubType ]);

			if ( ubPaletteRep == ubEndRep )
			{
				ubPaletteRep = ubStartRep;
			}
			SET_PALETTEREP_ID ( pSoldier->HeadPal,	gpPalRep[ ubPaletteRep ].ID );

			CreateSoldierPalettes( pSoldier );

			return( TRUE );
  }


  if ((pInputEvent->usEvent == KEY_DOWN )&& ( pInputEvent->usParam == 'v' ))
  {
			SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);

			// Get index of current
			CHECKF( GetPaletteRepIndexFromID( pSoldier->VestPal, &ubPaletteRep ) );
			ubType = gpPalRep[ ubPaletteRep ].ubType;

			ubPaletteRep++;

			// Count start and end index
			for ( cnt = 0; cnt < ubType; cnt++ )
			{
				ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ cnt ]);
			}

			ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ ubType ]);

			if ( ubPaletteRep == ubEndRep )
			{
				ubPaletteRep = ubStartRep;
			}
			SET_PALETTEREP_ID ( pSoldier->VestPal,	gpPalRep[ ubPaletteRep ].ID );

			CreateSoldierPalettes( pSoldier );

			return( TRUE );
  }

  if ((pInputEvent->usEvent == KEY_DOWN )&& ( pInputEvent->usParam == 'p' ))
  {
			SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);

			// Get index of current
			CHECKF( GetPaletteRepIndexFromID( pSoldier->PantsPal, &ubPaletteRep ) );
			ubType = gpPalRep[ ubPaletteRep ].ubType;

			ubPaletteRep++;

			// Count start and end index
			for ( cnt = 0; cnt < ubType; cnt++ )
			{
				ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ cnt ]);
			}

			ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ ubType ]);

			if ( ubPaletteRep == ubEndRep )
			{
				ubPaletteRep = ubStartRep;
			}
			SET_PALETTEREP_ID ( pSoldier->PantsPal,	gpPalRep[ ubPaletteRep ].ID );

			CreateSoldierPalettes( pSoldier );

			return( TRUE );
  }

  if ((pInputEvent->usEvent == KEY_DOWN )&& ( pInputEvent->usParam == 's' ))
  {
			SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);

			// Get index of current
			CHECKF( GetPaletteRepIndexFromID( pSoldier->SkinPal, &ubPaletteRep ) );
			ubType = gpPalRep[ ubPaletteRep ].ubType;

			ubPaletteRep++;

			// Count start and end index
			for ( cnt = 0; cnt < ubType; cnt++ )
			{
				ubStartRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ cnt ]);
			}

			ubEndRep = (UINT8)(ubStartRep + gubpNumReplacementsPerRange[ ubType ]);

			if ( ubPaletteRep == ubEndRep )
			{
				ubPaletteRep = ubStartRep;
			}
			SET_PALETTEREP_ID ( pSoldier->SkinPal,	gpPalRep[ ubPaletteRep ].ID );

			CreateSoldierPalettes( pSoldier );

			return( TRUE );
  }

	return( FALSE );
}

UINT32 DebugScreenInit(void)
{
	return( TRUE );
}


static BOOLEAN CheckForAndExitTacticalDebug(void)
{
	if ( gfExitDebugScreen )
	{
		FirstTime = TRUE;
		gfInitRect = TRUE;
		gfExitDebugScreen = FALSE;
		FreeBackgroundRect( guiBackgroundRect );
		SetRenderHook(NULL);
		SetUIKeyboardHook(NULL);

		return( TRUE );
	}

	return( FALSE );
}


static void ExitDebugScreen(void)
{
	if ( guiCurrentScreen == DEBUG_SCREEN )
	{
		gfExitDebugScreen = TRUE;
	}

	CheckForAndExitTacticalDebug( );
}


static BOOLEAN DebugKeyboardHook(InputAtom* pInputEvent);
static void DebugRenderHook(void);


UINT32 DebugScreenHandle(void)
{
	if ( CheckForAndExitTacticalDebug() )
	{
		return( GAME_SCREEN );
	}

	if ( gfInitRect )
	{
		guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, 0, 0, 600 , 360);
		gfInitRect = FALSE;
	}


	if ( FirstTime )
	{
		FirstTime = FALSE;

		SetRenderHook(DebugRenderHook);
		SetUIKeyboardHook(DebugKeyboardHook);
	}
	else
	{

		(*(GameScreens[GAME_SCREEN].HandleScreen))();

	}

	return( DEBUG_SCREEN );

}

UINT32 DebugScreenShutdown(void)
{

	return( TRUE );
}


static void DebugRenderHook(void)
{
	gDebugRenderOverride[ gCurDebugPage ]( );
}


static BOOLEAN DebugKeyboardHook(InputAtom* pInputEvent)
{
  if ((pInputEvent->usEvent == KEY_UP )&& ( pInputEvent->usParam == 'q' ))
  {
		gfExitDebugScreen = TRUE;
		return( TRUE );
	}

	if (pInputEvent->usEvent == KEY_UP && pInputEvent->usParam == SDLK_PAGEUP)
  {
		// Page down
		gCurDebugPage++;

		if ( gCurDebugPage == MAX_DEBUG_PAGES )
		{
			gCurDebugPage = 0;
		}

		FreeBackgroundRect( guiBackgroundRect );
		gfInitRect = TRUE;

  }

	if (pInputEvent->usEvent == KEY_UP && pInputEvent->usParam == SDLK_PAGEDOWN)
  {
		// Page down
		gCurDebugPage--;

		if ( gCurDebugPage < 0 )
		{
			gCurDebugPage = MAX_DEBUG_PAGES-1;
		}

		FreeBackgroundRect( guiBackgroundRect );
		gfInitRect = TRUE;

  }

	return( FALSE );
}


void SetDebugRenderHook( RENDER_HOOK pDebugRenderOverride, INT8 ubPage )
{
	gDebugRenderOverride[ ubPage ] = pDebugRenderOverride;
}


static void DefaultDebugPage1(void)
{
	SetFont( LARGEFONT1 );
	gprintf( 0,0,L"DEBUG PAGE ONE" );
}


static void DefaultDebugPage2(void)
{
	SetFont( LARGEFONT1 );
	gprintf( 0,0,L"DEBUG PAGE TWO" );
}


static void DefaultDebugPage3(void)
{
	SetFont( LARGEFONT1 );
	gprintf( 0,0,L"DEBUG PAGE THREE" );
}


static void DefaultDebugPage4(void)
{
	SetFont( LARGEFONT1 );
	gprintf( 0,0,L"DEBUG PAGE FOUR" );
}


UINT32 SexScreenInit(void)
{
	return( TRUE );
}

#define SMILY_DELAY						100
#define SMILY_END_DELAY				1000

UINT32 SexScreenHandle(void)
{
	static UINT8					ubCurrentScreen = 0;
	static UINT32					guiSMILY;
	static INT8						bCurFrame = 0;
	static UINT32					uiTimeOfLastUpdate = 0, uiTime;

	// OK, Clear screen and show smily face....
	ColorFillVideoSurfaceArea( FRAME_BUFFER, 0, 0, 640,	480, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );
	InvalidateScreen( );
	// Remove cursor....
	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	if ( ubCurrentScreen == 0 )
	{
		// Load face....
		guiSMILY = AddVideoObjectFromFile("INTERFACE/luckysmile.sti");
		AssertMsg(guiSMILY != NO_VOBJECT, "Missing INTERFACE/luckysmile.sti");

		// Init screen
		bCurFrame = 0;

		ubCurrentScreen = 1;

		uiTimeOfLastUpdate = GetJA2Clock( );

		return( SEX_SCREEN );

	}

	// Update frame
	uiTime = GetJA2Clock( );

	// if we are animation smile...
	if ( ubCurrentScreen == 1 )
	{
		PlayJA2StreamingSampleFromFile("Sounds/Sex.wav", HIGHVOLUME, 1, MIDDLEPAN, NULL);
		if ( ( uiTime - uiTimeOfLastUpdate ) > SMILY_DELAY )
		{
			uiTimeOfLastUpdate = uiTime;

			bCurFrame++;

			if ( bCurFrame == 32 )
			{
				// Start end delay
				ubCurrentScreen = 2;
			}
		}
	}

	if ( ubCurrentScreen == 2 )
	{
		if ( ( uiTime - uiTimeOfLastUpdate ) > SMILY_END_DELAY )
		{
			uiTimeOfLastUpdate = uiTime;

			ubCurrentScreen = 0;

			// Remove video object...
			DeleteVideoObjectFromIndex( guiSMILY );

			FadeInGameScreen( );

			// Advance time...
			// Chris.... do this based on stats?
			WarpGameTime( ( ( 5 + Random( 20 ) ) * NUM_SEC_IN_MIN ), WARPTIME_NO_PROCESSING_OF_EVENTS );

			return( GAME_SCREEN );
		}
	}

	// Calculate smily face positions...
	const ETRLEObject* pTrav = GetVideoObjectETRLESubregionProperties(guiSMILY, 0);
	INT16 sX = (640 - pTrav->usWidth)  / 2;
	INT16 sY = (480 - pTrav->usHeight) / 2;

	if( bCurFrame < 24 )
	{
		BltVideoObjectFromIndex( FRAME_BUFFER, guiSMILY, 0, sX, sY);
	}
	else
	{
		BltVideoObjectFromIndex( FRAME_BUFFER, guiSMILY, (INT8)(bCurFrame%8 ), sX, sY);
	}

	InvalidateRegion( sX, sY, (INT16)( sX + pTrav->usWidth ), (INT16)( sY + pTrav->usHeight ) );

	return( SEX_SCREEN );
}

UINT32 SexScreenShutdown(void)
{
	return( TRUE );
}



UINT32 DemoExitScreenInit(void)
{
	return( TRUE );
}


static void DoneFadeOutForDemoExitScreen(void)
{
	gfProgramIsRunning = FALSE;
}

#ifdef JA2DEMOADS
	extern STR16 gpDemoString[];
	extern STR16 gpDemoIntroString[];
	BOOLEAN gfDemoIntro = FALSE;
#endif

#ifdef JA2DEMOADS
void DoDemoIntroduction()
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
	MSG Message;
	UINT32 uiTempID;
	UINT16 yp, height;
	UINT32 uiStartTime = 0xffffffff;
	UINT16 usFadeLimit = 8;

	//If we haven't played the demo intro yet, do so now...
	if( gfDemoIntro )
		return;
	gfDemoIntro = TRUE;

	SetCurrentCursorFromDatabase( 0 );

	//Load the background image.
	uiTempID = AddVideoObjectFromFile("DemoAds/DemoScreen1.sti");
	AssertMsg(uiTempID != NO_VOBJECT, "Failed to load DemoAds/DemoScreen1.sti");
	if (uiTempID == NO_VOBJECT) return;
	BltVideoObjectFromIndex( FRAME_BUFFER, uiTempID, 0, 0, 0);
	DeleteVideoObjectFromIndex( uiTempID );
	InvalidateScreen();

	//print out the information
	SetFont( FONT14HUMANIST );
	SetFontForeground( FONT_YELLOW );
	SetFontShadow( FONT_NEARBLACK );

	yp = 50;
	height = DisplayWrappedString(20, yp, 400, 3, FONT14ARIAL, FONT_YELLOW, gpDemoIntroString[0], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 25 + height);
	SetFontForeground( FONT_GRAY2 );
	mprintf( 35, yp, L"-" );
	height = DisplayWrappedString(45, yp, 385, 3, FONT14ARIAL, FONT_GRAY2, gpDemoIntroString[1], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 25 + height);
	mprintf( 35, yp, L"-" );
	height = DisplayWrappedString(45, yp, 385, 3, FONT14ARIAL, FONT_GRAY2, gpDemoIntroString[2], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 25 + height);
	mprintf( 35, yp, L"-" );
	height = DisplayWrappedString(45, yp, 385, 3, FONT14ARIAL, FONT_GRAY2, gpDemoIntroString[3], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 30 + height);
	height = DisplayWrappedString(35, yp, 385, 3, FONT14ARIAL, FONT_GRAY3, gpDemoIntroString[4], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 35 + height);
	height = DisplayWrappedString(20, yp, 400, 3, FONT14ARIAL, FONT_LTKHAKI, gpDemoIntroString[5], FONT_BLACK, LEFT_JUSTIFIED);

	RefreshScreen();
	while (gfProgramIsRunning)
	{
		if (PeekMessage(&Message, NULL, 0, 0, PM_NOREMOVE))
		{ // We have a message on the WIN95 queue, let's get it
			if (!GetMessage(&Message, NULL, 0, 0))
			{ // It's quitting time
				continue;
			}
			// Ok, now that we have the message, let's handle it
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		else
		{ // Windows hasn't processed any messages, therefore we handle the rest
			InputAtom InputEvent;
			POINT  MousePos;

			GetCursorPos(&MousePos);
			// Hook into mouse stuff for MOVEMENT MESSAGES
			MouseSystemHook(MOUSE_POS, MousePos.x, MousePos.y, _LeftButtonDown, _RightButtonDown);
			MusicPoll( FALSE );
			if( uiStartTime != 0xffffffff )
			{
				if( GetJA2Clock() > uiStartTime )
				{
					if( usFadeLimit )
					{
						usFadeLimit--;
						ShadowVideoSurfaceRectUsingLowPercentTable( FRAME_BUFFER, 0, 0, 640, 480 );
						InvalidateScreen();
					}
					else
					{
						ColorFillVideoSurfaceArea( FRAME_BUFFER, 0, 0, 640,	480, 0 );
						InvalidateScreen();
						RefreshScreen();
						return;
					}
				}
			}
			if( gfLeftButtonState || gfRightButtonState )
			{
				if( uiStartTime == 0xffffffff )
				{
					uiStartTime = GetJA2Clock();
				}
			}
			while( DequeueEvent( &InputEvent ) )
			{
				if( InputEvent.usEvent == KEY_DOWN )
				{
					if (InputEvent.usParam == SDLK_ESCAPE) return;
					if (uiStartTime == 0xffffffff)
					{
						uiStartTime = GetJA2Clock();
					}
				}
			}
			RefreshScreen();
		}
	}
#endif
}
#endif

extern INT8 gbFadeSpeed;


#ifdef GERMAN
static void DisplayTopwareGermanyAddress(void)
{
	UINT32					uiTempID;
	UINT8 *pDestBuf;
	UINT32 uiDestPitchBYTES;
	SGPRect ClipRect;

	//bring up the Topware address screen
	uiTempID = AddVideoObjectFromFile("German/topware_germany.sti");
	AssertMsg(uiTempID != NO_VOBJECT, "Failed to load German/topware_germany.sti");
	if (uiTempID == NO_VOBJECT) return;

	//Shade out a background piece to emphasize the German address.
	ClipRect.iLeft = 208;
	ClipRect.iRight = 431;
	ClipRect.iTop = 390;
	ClipRect.iBottom = 475;
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	Blt16BPPBufferShadowRect( (UINT16*)pDestBuf, uiDestPitchBYTES, &ClipRect );
	UnLockVideoSurface( FRAME_BUFFER );

	//Draw the anti-aliased address now.
	BltVideoObjectFromIndex( FRAME_BUFFER, uiTempID, 0, 218, 400);
	BltVideoObjectFromIndex( FRAME_BUFFER, uiTempID, 0, 218, 400);
	InvalidateRegion( 208, 390, 431, 475 );
	DeleteVideoObjectFromIndex( uiTempID );
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();
}
#endif

UINT32 DemoExitScreenHandle(void)
{
	#ifdef JA2DEMOADS
	static UINT8					ubCurrentScreen = 0;
	static UINT8					ubPreviousScreen = 0;
	static INT8						bCurFrame = 0;
	static UINT32					uiTimeOfLastUpdate = 0;
	static UINT32					uiStartTime = 0;
	UINT32								uiTime;
	static UINT32					uiCharsPrinted = 0;
	static UINT32					uiWidthString = 0;
	UINT32								uiCharsToPrint;
	INT32									iPercentage;
	static INT32					iPrevPercentage = 0;
	SGPRect								SrcRect, DstRect;
	INT32									iFactor;
	static BOOLEAN				fSetMusicToFade = FALSE;
	wchar_t str[150];
	static UINT16					usCenter = 320;
	static UINT32					uiCollageID = 0;
	static BOOLEAN gfFastAnim = FALSE;
	static BOOLEAN gfPrevFastAnim = FALSE;

	static MOUSE_REGION	  BackRegion;
	InputAtom InputEvent;

	gfPrevFastAnim = gfFastAnim;
	if (gfLeftButtonState || gfRightButtonState || _KeyDown(SDLK_SPACE))
	{
		gfFastAnim = TRUE;
	}
	else
	{
		gfFastAnim = FALSE;
	}

	while( DequeueEvent( &InputEvent ) )
	{
		if( InputEvent.usEvent == KEY_DOWN )
		{
			if (InputEvent.usParam == SDLK_ESCAPE)
			{
				gfProgramIsRunning = FALSE;
			}
		}
	}

	if ( HandleBeginFadeOut( DEMO_EXIT_SCREEN ) )
	{
		return( DEMO_EXIT_SCREEN );
	}

	if ( ubCurrentScreen == 0 )
	{
		//bring up the collage screen
		uiCollageID = AddVideoSurfaceFromFile("DemoAds/collage.sti");
		if (uiCollageID == NO_VSURFACE)
		{
			AssertMsg( 0, "Failed to load DemoAds/collage.sti" );
			ubCurrentScreen = 8;
			return DEMO_EXIT_SCREEN;
		}

		SetMusicMode( MUSIC_NONE );

		BltVideoSurface(FRAME_BUFFER, uiCollageID, 0, 0, NULL);
		DeleteVideoSurfaceFromIndex( uiCollageID );

		InvalidateScreen( );


		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
		MSYS_DefineRegion( &BackRegion, 0, 0 ,640, 480, MSYS_PRIORITY_HIGHEST,
							 VIDEO_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		// Init screen
		bCurFrame = 0;

		ubCurrentScreen = 1;

		uiTimeOfLastUpdate = GetJA2Clock( );

		FadeInNextFrame( );

		HandleBeginFadeIn( DEMO_EXIT_SCREEN );

		return( DEMO_EXIT_SCREEN );
	}

	// Update frame
	uiTime = GetJA2Clock( );
	if( !uiStartTime )
	{
		uiStartTime = uiTime;
		usCenter = (UINT16)(320 - (StringPixLength( gpDemoString[0], FONT14ARIAL )+10) + (StringPixLength( gpDemoString[0], FONT14ARIAL )+10));
		uiWidthString = StringPixLength( gpDemoString[0], FONT14ARIAL );
		SetFont( FONT14ARIAL );
		SetFontForeground( FONT_YELLOW );
		SetFontShadow( FONT_NEARBLACK );
	}

	// type out the title message, "The best of Strategy. The best of Role-playing."
	if( ubCurrentScreen == 1 )
	{
		iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += max( iPercentage - iPrevPercentage, 1 );
			else
				iPercentage += 1;
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - (iPrevPercentage+1) * 16;
			iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		}
		if( iPercentage > 160 && uiCharsPrinted == wcslen( gpDemoString[ 0 ] ) )
		{
			uiStartTime = GetJA2Clock();
			uiWidthString = StringPixLength( gpDemoString[1], FONT14ARIAL );
			ubCurrentScreen = 2;
			ubPreviousScreen = 1;
			uiCharsPrinted = 0;
			iPrevPercentage = 0;
			return DEMO_EXIT_SCREEN;
		}
		iPercentage = min( iPercentage, 100 );

		uiCharsToPrint = wcslen( gpDemoString[0] ) * iPercentage / 100;

		if( uiCharsToPrint > uiCharsPrinted )
		{
			uiCharsPrinted = uiCharsToPrint;
			PlayJA2Sample(ENTERING_TEXT, MIDVOLUME, 1, 39 + iPercentage / 2);
			wcscpy( str, gpDemoString[0] );
			str[ uiCharsToPrint ] = L'\0';
			mprintf( usCenter - 10 - uiWidthString, 5, str );
			InvalidateRegion( usCenter - 10 - uiWidthString, 5, usCenter - 10, 19 );
			ExecuteBaseDirtyRectQueue();
			EndFrameBufferRender();
		}

		ubPreviousScreen = 1;
		return DEMO_EXIT_SCREEN;
	}

	// type out the title message, "The best of Strategy. The best of Role-playing."
	if( ubCurrentScreen == 2 )
	{
		iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += max( iPercentage - iPrevPercentage, 1 );
			else
				iPercentage += 1;
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - (iPrevPercentage+1) * 16;
			iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		}
		if( iPercentage > 160 && uiCharsPrinted == wcslen( gpDemoString[ 1 ] ) )
		{
			ubCurrentScreen = 3;
			ubPreviousScreen = 2;
			iPrevPercentage = 0;
			return DEMO_EXIT_SCREEN;
		}
		iPercentage = min( iPercentage, 100 );
		uiCharsToPrint = wcslen( gpDemoString[1] ) * iPercentage / 100;

		if( uiCharsToPrint > uiCharsPrinted )
		{
			uiCharsPrinted = uiCharsToPrint;
			PlayJA2Sample(ENTERING_TEXT, MIDVOLUME, 1, 39 + iPercentage / 2);
			wcscpy( str, gpDemoString[1] );
			str[ uiCharsToPrint ] = L'\0';
			mprintf( usCenter + 10, 5, str );
			InvalidateRegion( usCenter + 10, 5, usCenter + 10 + uiWidthString, 19 );
			ExecuteBaseDirtyRectQueue();
			EndFrameBufferRender();
		}

		ubPreviousScreen = 2;
		return DEMO_EXIT_SCREEN;
	}

	//drop the ja2 logo into the screen playing a metal stamp sound.
	if ( ubCurrentScreen == 3 )
	{
		uiTime = GetJA2Clock();
		if( ubPreviousScreen == 2 )
		{
			UINT32					uiTempID;

			//Create render buffer
			uiCollageID = AddVideoSurface(263, 210, PIXEL_DEPTH);
			CHECKF(uiCollageID != NO_VSURFACE);

			//bring up the collage screen
			uiTempID = AddVideoObjectFromFile("Interface/ja2logo.sti");
			AssertMsg(uiTempID != NO_VOBJECT, "Failed to load Interface/ja2logo.sti");
			if (uiTempID == NO_VOBJECT)
			{
				ubCurrentScreen = 8;
				ubPreviousScreen = 3;
				return DEMO_EXIT_SCREEN;
			}
			uiStartTime = uiTime;
			BltVideoObjectFromIndex( uiCollageID, uiTempID, 0, 0, 0);
			DeleteVideoObjectFromIndex( uiTempID );
			BlitBufferToBuffer( FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 640, 480 );
			PlayJA2SampleFromFile("DemoAds/Swoosh.wav", HIGHVOLUME, 1, MIDDLEPAN);
		}

		iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += max( iPercentage - iPrevPercentage * 1, 1 );
			else
				iPercentage += 1;
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - (iPrevPercentage+1) * 12;
			iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		}
		iPercentage = min( iPercentage, 100 );

		//Factor the percentage so that it is modified by a gravity falling acceleration effect.
		iFactor = (iPercentage - 50) * 2;
		if( iPercentage < 50 )
			iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
		else
			iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.05);

		if( iPercentage < 50 )
		{ //do the logo entrance section
			SrcRect.iLeft = 131 - 131 * iPercentage / 50;
			SrcRect.iRight = 132 + 131 * iPercentage / 50;
			SrcRect.iTop = 48 - 48 * iPercentage / 50;
			SrcRect.iBottom = 49 + 161 * iPercentage / 50;
			//SrcRect.iLeft = 145 - 145 * iPercentage / 50;
			//SrcRect.iRight = 146 + 117 * iPercentage / 50;
			//SrcRect.iTop = 117 - 117 * iPercentage / 50;
			//SrcRect.iBottom = 118 + 92 * iPercentage / 50;
			DstRect.iLeft = 0;
			DstRect.iRight = 640;
			DstRect.iTop = 0;
			DstRect.iBottom = 480;
			BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 0, 0, 640, 480 );
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		else
		{ //scale down the logo and stamp it.
			SrcRect.iLeft = 0;
			SrcRect.iRight = 263;
			SrcRect.iTop = 0;
			SrcRect.iBottom = 210;
			DstRect.iLeft = 189 * (iPercentage-50) / 50;
			DstRect.iRight = 640 - 188 * (iPercentage-50) / 50;
			DstRect.iTop = 20 * (iPercentage-50) / 50;
			DstRect.iBottom = 480 - 250 * (iPercentage-50) / 50;
			BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 0, 0, 640, 480 );
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		if( iPercentage == 100 )
		{
			SetMusicMode( MUSIC_MAIN_MENU );
			PlayJA2SampleFromFile("DemoAds/Hit.wav", HIGHVOLUME, 1, MIDDLEPAN);
			gbFadeSpeed = (INT8)50;
			BlitBufferToBuffer( FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 640, 480 );
			ubCurrentScreen = 4;
			SetFontDestBuffer( FRAME_BUFFER, 0, 230, 640, 480, FALSE );
			SetFont( FONT10ARIAL );
			SetFontForeground( FONT_GRAY2 );
			uiStartTime = GetJA2Clock();
			DeleteVideoSurfaceFromIndex( uiCollageID );
			iPrevPercentage = 0;
		}
		ubPreviousScreen = 3;
		return( DEMO_EXIT_SCREEN );
	}

	//Do the feature scrolling.
	if( ubCurrentScreen == 4 )
	{
		INT32 i, width, yp;
		UINT16 usFont;
		uiTime = GetJA2Clock();
		//percentage is at 100 times higher resolution.
		iPercentage = (uiTime - uiStartTime) ;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += max( iPercentage - iPrevPercentage, 150 );
			else
				iPercentage += 150;
			if( iPercentage < iPrevPercentage )
			{
				iPercentage = iPrevPercentage;
			}
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - (iPrevPercentage+150);
			iPercentage = (uiTime - uiStartTime);
		}
		iPercentage = min( iPercentage, 45000 );

		BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 100, 230, 440, 250 );
		InvalidateRegion( 100, 230, 540, 640 );
		yp = 480 - iPercentage / 40; //500 (0%) to -500 (100%)
		yp = max( yp, -400 );
		for( i = 2; i < 40; i++ )
		{
			if( i < 36 )
				usFont = (UINT16)FONT14ARIAL;
			else
				usFont = (UINT16)FONT14HUMANIST;

			switch( i )
			{
				case 38:
				case 39: SetFontForeground(FONT_LTKHAKI); break;
				default: SetFontForeground(FONT_GRAY2);   break;
			}
			const wchar_t* String = gpDemoString[i];
			width = StringPixLength(String, usFont);
			SetFont( usFont );

			mprintf(320 - width / 2, yp + i * 17, String);
		}
		if( !fSetMusicToFade && iPercentage > 43000 )
		{
			fSetMusicToFade = TRUE;
			SetMusicMode( MUSIC_NONE );
		}
		if( yp == -400 )
		{
			ubCurrentScreen = 5;
			iPrevPercentage = 0;
		}

		ubPreviousScreen = 4;
		ExecuteBaseDirtyRectQueue();
		EndFrameBufferRender();
		return DEMO_EXIT_SCREEN;
	}

	//drop the available Oct 1998 label into the screen playing a metal stamp sound.
	if ( ubCurrentScreen == 5 )
	{
		uiTime = GetJA2Clock();
		if( ubPreviousScreen == 4 )
		{
			UINT32					uiTempID;

			//Create render buffer
			uiCollageID = AddVideoSurface(331, 148, PIXEL_DEPTH);
			CHECKF(uiCollageID != NO_VSURFACE);

			//bring up the collage screen
			uiTempID = AddVideoObjectFromFile("DemoAds/available.sti");
			AssertMsg(uiTempID != NO_VOBJECT, "Failed to load DemoAds/available.sti");
			if (uiTempID == NO_VOBJECT)
			{
				ubCurrentScreen = 8;
				ubPreviousScreen = 4;
				return DEMO_EXIT_SCREEN;
			}
			uiStartTime = uiTime;
			BltVideoObjectFromIndex( uiCollageID, uiTempID, 0, 0, 0);
			DeleteVideoObjectFromIndex( uiTempID );
			BlitBufferToBuffer( FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 640, 480 );
			PlayJA2SampleFromFile("DemoAds/Swoosh.wav", MIDVOLUME, 1, MIDDLEPAN);
		}

		iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += max( iPercentage - iPrevPercentage, 1 );
			else
				iPercentage += 1;
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - iPrevPercentage * 12;
			iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		}
		iPercentage = min( iPercentage, 100 );

		//Factor the percentage so that it is modified by a gravity falling acceleration effect.
		iFactor = (iPercentage - 50) * 2;
		if( iPercentage < 50 )
			iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
		else
			iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.05);

		if( iPercentage < 50 )
		{ //do the logo entrance section
			SrcRect.iLeft = 165 - 165 * iPercentage / 50;
			SrcRect.iRight = 166 + 165 * iPercentage / 50;
			SrcRect.iTop = 74 - 74 * iPercentage / 50;
			SrcRect.iBottom = 75 + 73 * iPercentage / 50;
			DstRect.iLeft = 0;
			DstRect.iRight = 640;
			DstRect.iTop = 0;
			DstRect.iBottom = 480;
			BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 0, 0, 640, 480 );
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		else
		{ //scale down the logo and stamp it.
			SrcRect.iLeft = 0;
			SrcRect.iRight = 331;
			SrcRect.iTop = 0;
			SrcRect.iBottom = 148;
			DstRect.iLeft = 155 * (iPercentage-50) / 50;
			DstRect.iRight = 640 - 154 * (iPercentage-50) / 50;
			DstRect.iTop = 246 * (iPercentage-50) / 50;
			DstRect.iBottom = 480 - 86 * (iPercentage-50) / 50;
			#ifdef GERMAN
				DstRect.iTop -= (iPercentage-50);
				DstRect.iBottom -= (iPercentage-50);
			#endif
			BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 0, 0, 640, 480 );
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		if( iPercentage == 100 )
		{
			PlayJA2SampleFromFile("DemoAds/Hit.wav", HIGHVOLUME, 1, MIDDLEPAN);
			BlitBufferToBuffer( FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 640, 480 );
			ubCurrentScreen = 6;
			DeleteVideoSurfaceFromIndex( uiCollageID );
			iPrevPercentage = 0;
		}
		ubPreviousScreen = 5;
		return( DEMO_EXIT_SCREEN );
	}

	// type out the final purchase address information."
	if( ubCurrentScreen == 6 )
	{
		if( ubPreviousScreen != 6 )
		{
			uiStartTime = GetJA2Clock();
			uiCharsPrinted = 0;
			uiWidthString = StringPixLength( gpDemoString[40], FONT14ARIAL );
			SetFont( FONT14ARIAL );
			SetFontForeground( FONT_YELLOW );
			SetFontShadow( FONT_NEARBLACK );
		}
		iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += max( iPercentage - iPrevPercentage, 1 );
			else
				iPercentage += 1;
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - (iPrevPercentage+1) * 16;
			iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		}
		if( iPercentage > 160 && uiCharsPrinted == wcslen( gpDemoString[ 40 ] ) )
		{
			#ifdef GERMAN
			DisplayTopwareGermanyAddress();
			#endif
			ubCurrentScreen = 7;
			ubPreviousScreen = 6;
			iPrevPercentage = 0;
			return DEMO_EXIT_SCREEN;
		}
		iPercentage = min( iPercentage, 100 );
		uiCharsToPrint = wcslen( gpDemoString[40] ) * iPercentage / 100;

		if( uiCharsToPrint > uiCharsPrinted )
		{
			uiCharsPrinted = uiCharsToPrint;
			PlayJA2Sample(ENTERING_TEXT, MIDVOLUME, 1, 39 + iPercentage / 2);
			wcscpy( str, gpDemoString[40] );
			str[ uiCharsToPrint ] = L'\0';
			#ifdef GERMAN
				mprintf( 320 - uiWidthString / 2, 370, str );
				InvalidateRegion( 320 - uiWidthString / 2, 370, 320 + (uiWidthString+1) / 2, 383 );
			#else
				mprintf( 320 - uiWidthString / 2, 420, str );
				InvalidateRegion( 320 - uiWidthString / 2, 420, 320 + (uiWidthString+1) / 2, 433 );
			#endif
			ExecuteBaseDirtyRectQueue();
			EndFrameBufferRender();
		}

		ubPreviousScreen = 6;
		return DEMO_EXIT_SCREEN;
	}

	if ( ubCurrentScreen == 7 )
	{
		if( ubPreviousScreen == 6 )
		{
			uiStartTime = GetJA2Clock();
			#ifdef GERMAN
				uiStartTime = 4000000000; //you could break the system by playing the game for 46 real-time days or longer.
			#endif
		}
		uiTime = GetJA2Clock();
		if( !fSetMusicToFade && gfLeftButtonState )
		{
			fSetMusicToFade = TRUE;
			SetMusicMode( MUSIC_NONE );
			uiStartTime = uiTime - 6000;
		}
		#ifndef GERMAN
			if( uiStartTime + 6000 < uiTime && !fSetMusicToFade )
			{
				fSetMusicToFade = TRUE;
				SetMusicMode( MUSIC_NONE );
			}
		#endif
		if( uiStartTime + 7000 < uiTime )
		{
			ubCurrentScreen = 8;

			gFadeOutDoneCallback = DoneFadeOutForDemoExitScreen;

			FadeOutNextFrame( );

			MSYS_RemoveRegion( &BackRegion );

		}
		ubPreviousScreen = 7;
		return( DEMO_EXIT_SCREEN );
	}

	if ( HandleFadeOutCallback( ) )
	{
		return( DEMO_EXIT_SCREEN );
	}

	return( DEMO_EXIT_SCREEN );
#else
	gfProgramIsRunning = FALSE;
	return( DEMO_EXIT_SCREEN );
#endif
}


UINT32 DemoExitScreenShutdown(void)
{
	return( TRUE );
}

#ifndef JA2EDITOR

UINT32 LoadSaveScreenInit()
{
	return TRUE;
}

UINT32 LoadSaveScreenHandle()
{
	return TRUE;
}

UINT32 LoadSaveScreenShutdown()
{
	return TRUE;
}

UINT32 EditScreenInit()
{
	return TRUE;
}

UINT32 EditScreenHandle()
{
	return TRUE;
}

UINT32 EditScreenShutdown()
{
	return TRUE;
}


#endif
