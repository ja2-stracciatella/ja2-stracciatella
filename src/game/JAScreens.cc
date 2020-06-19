#include "Directories.h"
#include "Handle_UI.h"
#include "Local.h"
#include "SGP.h"
#include "GameLoop.h"
#include "HImage.h"
#include "VObject.h"
#include "VSurface.h"
#include "Input.h"
#include "Font.h"
#include "Debug_Pages.h"
#include "MouseSystem.h"
#include "Screens.h"
#include "Font_Control.h"
#include "SysUtil.h"
#include "RenderWorld.h"
#include "WorldDef.h"
#include "EditScreen.h"
#include "Timer_Control.h"
#include "Sys_Globals.h"
#include "Overhead.h"
#include "Utilities.h"
#include "Render_Dirty.h"
#include "JAScreens.h"
#include "Environment.h"
#include "Animation_Cache.h"
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
#include "Multi_Language_Graphic_Utils.h"
#include "Text.h"
#include "Video.h"
#include "Debug.h"
#include "UILayout.h"
#include "Timer.h"
#include "Logger.h"
#include "WordWrap.h"

#include <string_theory/format>
#include <string_theory/string>


#define MAX_DEBUG_PAGES 4


// GLOBAL FOR PAL EDITOR
UINT8 CurrentPalette = 0;
static BACKGROUND_SAVE* guiBackgroundRect = NO_BGND_RECT;
BOOLEAN gfExitPalEditScreen = FALSE;
BOOLEAN gfExitDebugScreen = FALSE;
static BOOLEAN FirstTime = TRUE;
BOOLEAN gfDoneWithSplashScreen = FALSE;


INT8 gCurDebugPage = 0;


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

	// Increment frame count
	uiFrameCount++;

	if ( COUNTERDONE( FPSCOUNTER ) )
	{
		// Reset counter
		RESETCOUNTER( FPSCOUNTER );

		uiFPS = uiFrameCount;
		uiFrameCount = 0;
	}

	if ( gbFPSDisplay == SHOW_FULL_FPS )
	{
		// FRAME RATE
		SetVideoOverlayText(g_fps_overlay, ST::format("FPS: {}", __min(uiFPS, 1000)));

		// TIMER COUNTER
		SetVideoOverlayText(g_counter_period_overlay, ST::format("Game Loop Time: {}", __min(giTimerDiag, 1000)));
	}
}

ScreenID ErrorScreenHandle(void)
{
	InputAtom  InputEvent;
	static BOOLEAN	fFirstTime = FALSE;

	// Create string
	SetFontAttributes(LARGEFONT1, FONT_MCOLOR_LTGRAY);
	MPrint(50, 200, "RUNTIME ERROR");
	MPrint(50, 225, "PRESS <ESC> TO EXIT");

	DisplayWrappedString(50, 255, MAP_SCREEN_WIDTH - 50, 5, FONT12ARIAL, FONT_YELLOW, gubErrorText, 0, 0);

	if ( !fFirstTime )
	{
		SLOGE(ST::format("Runtime Error: {} ", gubErrorText));
		fFirstTime = TRUE;
	}

	// For quick setting of new video stuff / to be changed
	InvalidateScreen( );

	// Check for esc
	while (DequeueEvent(&InputEvent))
	{
		if( InputEvent.usEvent == KEY_DOWN )
		{
			if (InputEvent.usParam == SDLK_ESCAPE || (InputEvent.usParam == 'x' && InputEvent.usKeyState & ALT_DOWN))
			{ // Exit the program
				// handle shortcut exit
				HandleShortCutExitState( );
			}
		}
	}
	return( ERROR_SCREEN );
}


ScreenID InitScreenHandle(void)
{
	static UINT32 splashDisplayedMoment = 0;
	static UINT8					ubCurrentScreen = 255;

	if ( ubCurrentScreen == 255 )
	{
		if(isEnglishVersion())
		{
			if( gfDoneWithSplashScreen )
			{
				ubCurrentScreen = 0;
			}
			else
			{
				SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
				return( INTRO_SCREEN );
			}
		}
		else
		{
			ubCurrentScreen = 0;
		}
	}

	if ( ubCurrentScreen == 0 )
	{
		ubCurrentScreen = 1;

		InvalidateScreen( );

		//ATE: Set to true to reset before going into main screen!

		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
		splashDisplayedMoment = GetClock();
		return( INIT_SCREEN );
	}

	if ( ubCurrentScreen == 1 )
	{
		ubCurrentScreen = 2;
		return( InitializeJA2( ) );
	}

	if ( ubCurrentScreen == 2 )
	{
		// wait 3 seconds since the splash displayed and then switch
		// to the main menu
		if((GetClock() - splashDisplayedMoment) >= 3000)
		{
			InitMainMenu( );
			ubCurrentScreen = 3;
		}
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
		InitNewGame();
	}
	return( INIT_SCREEN );
}


static BOOLEAN PalEditKeyboardHook(InputAtom* pInputEvent);
static void PalEditRenderHook(void);


ScreenID PalEditScreenHandle(void)
{
	static BOOLEAN FirstTime = TRUE;

	if ( gfExitPalEditScreen )
	{
		gfExitPalEditScreen = FALSE;
		FirstTime = TRUE;
		FreeBackgroundRect(guiBackgroundRect);
		guiBackgroundRect = NO_BGND_RECT;
		SetRenderHook(NULL);
		SetUIKeyboardHook(NULL);
		return( GAME_SCREEN );
	}

	if ( FirstTime )
	{
		FirstTime = FALSE;

		SetRenderHook(PalEditRenderHook);
		SetUIKeyboardHook(PalEditKeyboardHook);

		guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, 50, 10, 550, 390);
	}
	else
	{

		(*(GameScreens[GAME_SCREEN].HandleScreen))();

	}

	return( PALEDIT_SCREEN );
}


static void PalEditRenderHook(void)
{
	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL)
	{
		// Set to current
		DisplayPaletteRep(sel->HeadPal,  50,  10, FRAME_BUFFER);
		DisplayPaletteRep(sel->PantsPal, 50,  50, FRAME_BUFFER);
		DisplayPaletteRep(sel->VestPal,  50,  90, FRAME_BUFFER);
		DisplayPaletteRep(sel->SkinPal,  50, 130, FRAME_BUFFER);
	}
}


static void CyclePaletteReplacement(SOLDIERTYPE& s, ST::string& pal)
{
	UINT8 ubPaletteRep = GetPaletteRepIndexFromID(pal);
	const UINT8 ubType = gpPalRep[ubPaletteRep].ubType;

	ubPaletteRep++;

	// Count start and end index
	UINT8 ubStartRep = 0;
	for (UINT32 cnt = 0; cnt < ubType; ++cnt)
	{
		ubStartRep = ubStartRep + gubpNumReplacementsPerRange[cnt];
	}

	const UINT8 ubEndRep = ubStartRep + gubpNumReplacementsPerRange[ubType];

	if (ubPaletteRep == ubEndRep) ubPaletteRep = ubStartRep;
	pal = gpPalRep[ubPaletteRep].ID;

	CreateSoldierPalettes(s);
}


static BOOLEAN PalEditKeyboardHook(InputAtom* pInputEvent)
{
	if (pInputEvent->usEvent != KEY_DOWN) return FALSE;

	SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel == NULL) return FALSE;

	switch (pInputEvent->usParam)
	{
		case SDLK_ESCAPE: gfExitPalEditScreen = TRUE; break;

		case 'h': CyclePaletteReplacement(*sel, sel->HeadPal);  break;
		case 'v': CyclePaletteReplacement(*sel, sel->VestPal);  break;
		case 'p': CyclePaletteReplacement(*sel, sel->PantsPal); break;
		case 's': CyclePaletteReplacement(*sel, sel->SkinPal);  break;

		default: return FALSE;
	}
	return TRUE;
}


static BOOLEAN CheckForAndExitTacticalDebug(void)
{
	if ( gfExitDebugScreen )
	{
		FirstTime = TRUE;
		gfExitDebugScreen = FALSE;
		gfDoVideoScroll = TRUE;
		FreeBackgroundRect( guiBackgroundRect );
		guiBackgroundRect = NO_BGND_RECT;
		SetRenderHook(NULL);
		SetUIKeyboardHook(NULL);

		return( TRUE );
	}

	return( FALSE );
}


static BOOLEAN DebugKeyboardHook(InputAtom* pInputEvent);
static void DebugRenderHook(void);


ScreenID DebugScreenHandle(void)
{
	if ( CheckForAndExitTacticalDebug() )
	{
		return( GAME_SCREEN );
	}

	if (guiBackgroundRect == NO_BGND_RECT)
	{
		guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, 0, 0, 600, 360);
	}


	if ( FirstTime )
	{
		gfDoVideoScroll = FALSE;
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


static void DebugRenderHook(void)
{
	gDebugRenderOverride[ gCurDebugPage ]( );
}


static BOOLEAN DebugKeyboardHook(InputAtom* pInputEvent)
{
	if (pInputEvent->usEvent == KEY_UP)
	{
		switch (pInputEvent->usParam)
		{
			case 'q':
				gfExitDebugScreen = TRUE;
				return TRUE;

			case SDLK_PAGEUP:
				gCurDebugPage++;
				if (gCurDebugPage == MAX_DEBUG_PAGES) gCurDebugPage = 0;
				FreeBackgroundRect(guiBackgroundRect);
				guiBackgroundRect = NO_BGND_RECT;
				break;

			case SDLK_PAGEDOWN:
				gCurDebugPage--;
				if (gCurDebugPage < 0) gCurDebugPage = MAX_DEBUG_PAGES - 1;
				FreeBackgroundRect(guiBackgroundRect);
				guiBackgroundRect = NO_BGND_RECT;
				break;
		}
	}

	return FALSE;
}


void SetDebugRenderHook( RENDER_HOOK pDebugRenderOverride, INT8 ubPage )
{
	gDebugRenderOverride[ ubPage ] = pDebugRenderOverride;
}


static void DefaultDebugPage1(void)
{
	MPageHeader("DEBUG PAGE ONE");
}


static void DefaultDebugPage2(void)
{
	MPageHeader("DEBUG PAGE TWO");
}


static void DefaultDebugPage3(void)
{
	MPageHeader("DEBUG PAGE THREE");
}


static void DefaultDebugPage4(void)
{
	MPageHeader("DEBUG PAGE FOUR");
}


#define SMILY_DELAY						100
#define SMILY_END_DELAY				1000

ScreenID SexScreenHandle(void)
{
	static UINT8					ubCurrentScreen = 0;
	static SGPVObject* guiSMILY;
	static INT8						bCurFrame = 0;
	static UINT32					uiTimeOfLastUpdate = 0, uiTime;

	// OK, Clear screen and show smily face....
	FRAME_BUFFER->Fill(Get16BPPColor(FROMRGB(0, 0, 0)));
	InvalidateScreen( );
	// Remove cursor....
	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	if ( ubCurrentScreen == 0 )
	{
		// Load face....
		guiSMILY = AddVideoObjectFromFile(INTERFACEDIR "/luckysmile.sti");
		PlayJA2StreamingSampleFromFile(SOUNDSDIR "/sex.wav", HIGHVOLUME, 1, MIDDLEPAN, NULL);

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
			DeleteVideoObject(guiSMILY);

			FadeInGameScreen( );

			// Advance time...
			// Chris.... do this based on stats?
			WarpGameTime( ( ( 5 + Random( 20 ) ) * NUM_SEC_IN_MIN ), WARPTIME_NO_PROCESSING_OF_EVENTS );

			return( GAME_SCREEN );
		}
	}

	// Calculate smily face positions...
	ETRLEObject const& pTrav = guiSMILY->SubregionProperties(0);
	INT16       const  sX    = (SCREEN_WIDTH  - pTrav.usWidth)  / 2;
	INT16       const  sY    = (SCREEN_HEIGHT - pTrav.usHeight) / 2;

	BltVideoObject(FRAME_BUFFER, guiSMILY, bCurFrame < 24 ? 0 : bCurFrame % 8, sX, sY);

	InvalidateRegion(sX, sY, sX + pTrav.usWidth, sY + pTrav.usHeight);

	return( SEX_SCREEN );
}

