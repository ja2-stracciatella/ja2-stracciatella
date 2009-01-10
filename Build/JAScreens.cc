#include "Handle_UI.h"
#include "Local.h"
#include "SGP.h"
#include "GameLoop.h"
#include "HImage.h"
#include "VObject.h"
#include "VSurface.h"
#include "Input.h"
#include "Font.h"
#include "MouseSystem.h"
#include "Screens.h"
#include "Font_Control.h"
#include "SysUtil.h"
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

#ifdef JA2DEMOADS
#	include "Fade_Screen.h"
#	include "Local.h"
#	include "Music_Control.h"
#	include "WordWrap.h"
#endif


#define MAX_DEBUG_PAGES 4


// GLOBAL FOR PAL EDITOR
UINT8	 CurrentPalette = 0;
static BACKGROUND_SAVE* guiBackgroundRect = NO_BGND_RECT;
BOOLEAN	gfExitPalEditScreen = FALSE;
BOOLEAN	gfExitDebugScreen = FALSE;
static BOOLEAN FirstTime = TRUE;
BOOLEAN	gfDoneWithSplashScreen = FALSE;


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
		SetVideoOverlayTextF(g_fps_overlay, L"%ld", __min(uiFPS, 1000));

		// TIMER COUNTER
		SetVideoOverlayTextF(g_counter_period_overlay, L"%ld", __min(giTimerDiag, 1000));
	}
}


ScreenID ErrorScreenHandle(void)
{
  InputAtom  InputEvent;
	static BOOLEAN	fFirstTime = FALSE;

	// Create string
	SetFontAttributes(LARGEFONT1, FONT_MCOLOR_LTGRAY);
	MPrint(50, 200, L"RUNTIME ERROR");
	MPrint(50, 225, L"PRESS <ESC> TO EXIT");

	SetFontAttributes(FONT12ARIAL, FONT_YELLOW);
	mprintf(50, 255, L"%hs", gubErrorText);
	SetFontForeground( FONT_LTRED );

	if ( !fFirstTime )
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_0, String( "Runtime Error: %s ", gubErrorText ) );
		fFirstTime = TRUE;
	}

	// For quick setting of new video stuff / to be changed
	InvalidateScreen( );

	// Check for esc
	while (DequeueEvent(&InputEvent))
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


ScreenID InitScreenHandle(void)
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

		SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);

		const INT32 x = 10;
		const INT32 y = SCREEN_HEIGHT;

#ifdef JA2BETAVERSION
		MPrint(x, y - 60, L"(Beta version error reporting enabled)");
#endif

		mprintf(x, y - 50,
#ifdef _DEBUG
			L"%ls: %hs (Debug %hs)",
#else
			L"%ls: %hs (%hs)",
#endif
			pMessageStrings[MSG_VERSION], g_version_label, g_version_number
		);

#ifdef _DEBUG
		mprintf(x, y - 40, L"SOLDIERTYPE: %d bytes", sizeof(SOLDIERTYPE));
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

		guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, 50, 10, 600 , 400);
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


static void CyclePaletteReplacement(SOLDIERTYPE* const s, PaletteRepID pal)
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
	SET_PALETTEREP_ID(pal, gpPalRep[ubPaletteRep].ID);

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

  	case 'h': CyclePaletteReplacement(sel, sel->HeadPal);  break;
  	case 'v': CyclePaletteReplacement(sel, sel->VestPal);  break;
  	case 'p': CyclePaletteReplacement(sel, sel->PantsPal); break;
  	case 's': CyclePaletteReplacement(sel, sel->SkinPal);  break;

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
		guiBackgroundRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT, 0, 0, 600 , 360);
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
		guiSMILY = AddVideoObjectFromFile("INTERFACE/luckysmile.sti");

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
			DeleteVideoObject(guiSMILY);

			FadeInGameScreen( );

			// Advance time...
			// Chris.... do this based on stats?
			WarpGameTime( ( ( 5 + Random( 20 ) ) * NUM_SEC_IN_MIN ), WARPTIME_NO_PROCESSING_OF_EVENTS );

			return( GAME_SCREEN );
		}
	}

	// Calculate smily face positions...
	ETRLEObject const* const pTrav = guiSMILY->SubregionProperties(0);
	INT16 sX = (SCREEN_WIDTH  - pTrav->usWidth)  / 2;
	INT16 sY = (SCREEN_HEIGHT - pTrav->usHeight) / 2;

	BltVideoObject(FRAME_BUFFER, guiSMILY, bCurFrame < 24 ? 0 : bCurFrame % 8, sX, sY);

	InvalidateRegion( sX, sY, (INT16)( sX + pTrav->usWidth ), (INT16)( sY + pTrav->usHeight ) );

	return( SEX_SCREEN );
}


static void DoneFadeOutForDemoExitScreen(void)
{
	gfProgramIsRunning = FALSE;
}


#ifdef JA2DEMOADS
void DoDemoIntroduction(void)
{
	static BOOLEAN gfDemoIntro = FALSE;

	UINT16 yp, height;
	UINT32 uiStartTime = 0xffffffff;
	UINT16 usFadeLimit = 8;

	//If we haven't played the demo intro yet, do so now...
	if( gfDemoIntro )
		return;
	gfDemoIntro = TRUE;

	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME) return;

	SetCurrentCursorFromDatabase( 0 );

	//Load the background image.
	BltVideoObjectOnce(FRAME_BUFFER, "DemoAds/DemoScreen1.sti", 0, 0, 0);
	InvalidateScreen();

	//print out the information
	SetFontShadow(FONT_NEARBLACK);

	yp = 50;
	height = DisplayWrappedString(20, yp, 400, 3, FONT14ARIAL, FONT_YELLOW, gpDemoIntroString[0], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 25 + height);
	SetFontForeground( FONT_GRAY2 );
	MPrint(35, yp, L"-");
	height = DisplayWrappedString(45, yp, 385, 3, FONT14ARIAL, FONT_GRAY2, gpDemoIntroString[1], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 25 + height);
	MPrint(35, yp, L"-");
	height = DisplayWrappedString(45, yp, 385, 3, FONT14ARIAL, FONT_GRAY2, gpDemoIntroString[2], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 25 + height);
	MPrint(35, yp, L"-");
	height = DisplayWrappedString(45, yp, 385, 3, FONT14ARIAL, FONT_GRAY2, gpDemoIntroString[3], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 30 + height);
	height = DisplayWrappedString(35, yp, 385, 3, FONT14ARIAL, FONT_GRAY3, gpDemoIntroString[4], FONT_BLACK, LEFT_JUSTIFIED);
	yp = (UINT16)(yp + 35 + height);
	height = DisplayWrappedString(20, yp, 400, 3, FONT14ARIAL, FONT_LTKHAKI, gpDemoIntroString[5], FONT_BLACK, LEFT_JUSTIFIED);

	RefreshScreen();
	while (gfProgramIsRunning)
	{
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN: KeyDown(&event.key.keysym); break;
				case SDL_KEYUP:   KeyUp(  &event.key.keysym); break;

				case SDL_MOUSEBUTTONDOWN: MouseButtonDown(&event.button); break;
				case SDL_MOUSEBUTTONUP:   MouseButtonUp(&event.button);   break;

				case SDL_MOUSEMOTION:
					gusMouseXPos = event.motion.x;
					gusMouseYPos = event.motion.y;
					break;

				case SDL_QUIT:
					gfProgramIsRunning = FALSE;
					break;
			}
		}
		else
		{ // Windows hasn't processed any messages, therefore we handle the rest
			InputAtom InputEvent;

			// Hook into mouse stuff for MOVEMENT MESSAGES
			SGPPoint MousePos;
			GetMousePos(&MousePos);
			MouseSystemHook(MOUSE_POS, MousePos.iX, MousePos.iY);
			MusicPoll();
			if( uiStartTime != 0xffffffff )
			{
				if( GetJA2Clock() > uiStartTime )
				{
					if( usFadeLimit )
					{
						usFadeLimit--;
						FRAME_BUFFER->ShadowRectUsingLowPercentTable(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
						InvalidateScreen();
					}
					else
					{
						FRAME_BUFFER->Fill(0);
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
}


#	ifdef GERMAN
static void DisplayTopwareGermanyAddress(void)
{
	//bring up the Topware address screen

	//Shade out a background piece to emphasize the German address.
	FRAME_BUFFER->ShadowRect(208, 390, 431, 475);

	//Draw the anti-aliased address now.
	BltVideoObjectOnce(FRAME_BUFFER, "German/topware_germany.sti", 0, 218, 400);

	InvalidateRegion( 208, 390, 431, 475 );
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();
}
#	endif


ScreenID DemoExitScreenHandle(void)
{
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
	INT32									iFactor;
	static BOOLEAN				fSetMusicToFade = FALSE;
	wchar_t str[150];
	static UINT16  usCenter;
	static SGPVSurface* uiCollageID = 0;
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
		try
		{
			BltVideoSurfaceOnce(FRAME_BUFFER, "DemoAds/collage.sti", 0, 0);
		}
		catch (...)
		{
			ubCurrentScreen = 8;
			return DEMO_EXIT_SCREEN;
		}

		SetMusicMode( MUSIC_NONE );

		InvalidateScreen( );

		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
		MSYS_DefineRegion(&BackRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, VIDEO_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

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
		usCenter = SCREEN_WIDTH / 2 - (StringPixLength(gpDemoString[0], FONT14ARIAL) + 10) + (StringPixLength(gpDemoString[0], FONT14ARIAL) + 10);
		uiWidthString = StringPixLength( gpDemoString[0], FONT14ARIAL );
		SetFontAttributes(FONT14ARIAL, FONT_YELLOW);
	}

	// type out the title message, "The best of Strategy. The best of Role-playing."
	if( ubCurrentScreen == 1 )
	{
		iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += MAX( iPercentage - iPrevPercentage, 1 );
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
		iPercentage = MIN( iPercentage, 100 );

		uiCharsToPrint = wcslen( gpDemoString[0] ) * iPercentage / 100;

		if( uiCharsToPrint > uiCharsPrinted )
		{
			uiCharsPrinted = uiCharsToPrint;
			PlayJA2Sample(ENTERING_TEXT, MIDVOLUME, 1, 39 + iPercentage / 2);
			wcscpy( str, gpDemoString[0] );
			str[ uiCharsToPrint ] = L'\0';
			MPrint(usCenter - 10 - uiWidthString, 5, str);
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
				iPercentage += MAX( iPercentage - iPrevPercentage, 1 );
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
		iPercentage = MIN( iPercentage, 100 );
		uiCharsToPrint = wcslen( gpDemoString[1] ) * iPercentage / 100;

		if( uiCharsToPrint > uiCharsPrinted )
		{
			uiCharsPrinted = uiCharsToPrint;
			PlayJA2Sample(ENTERING_TEXT, MIDVOLUME, 1, 39 + iPercentage / 2);
			wcscpy( str, gpDemoString[1] );
			str[ uiCharsToPrint ] = L'\0';
			MPrint(usCenter + 10, 5, str);
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
			//Create render buffer
			uiCollageID = AddVideoSurface(263, 210, PIXEL_DEPTH);
			uiCollageID->SetTransparency(0);

			//bring up the collage screen
			try
			{
				BltVideoObjectOnce(uiCollageID, "Interface/ja2logo.sti", 0, 0, 0);
			}
			catch (...)
			{
				ubCurrentScreen = 8;
				ubPreviousScreen = 3;
				return DEMO_EXIT_SCREEN;
			}
			uiStartTime = uiTime;
			BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
			PlayJA2SampleFromFile("DemoAds/Swoosh.wav", HIGHVOLUME, 1, MIDDLEPAN);
		}

		iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += MAX( iPercentage - iPrevPercentage * 1, 1 );
			else
				iPercentage += 1;
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - (iPrevPercentage+1) * 12;
			iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		}
		iPercentage = MIN( iPercentage, 100 );

		//Factor the percentage so that it is modified by a gravity falling acceleration effect.
		iFactor = (iPercentage - 50) * 2;
		if( iPercentage < 50 )
			iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
		else
			iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.05);

		if( iPercentage < 50 )
		{ //do the logo entrance section
			SGPBox const SrcRect =
			{
#if 1
				131 - 131 * iPercentage / 50,
				 48 - 48  * iPercentage / 50,
#else
				145 - 145 * iPercentage / 50,
				117 - 117 * iPercentage / 50,
#endif
				  1 + 262 * iPercentage / 50,
				  1 + 209 * iPercentage / 50
			};
			SGPBox const DstRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
			BltVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, NULL);
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		else
		{ //scale down the logo and stamp it.
			SGPBox const SrcRect = { 0, 0, 263, 210 };
			SGPBox const DstRect =
			{
				189 * (iPercentage - 50) / 50,
				 20 * (iPercentage - 50) / 50,
				SCREEN_WIDTH  - 377 * (iPercentage - 50) / 50,
				SCREEN_HEIGHT - 270 * (iPercentage - 50) / 50
			};
			BltVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, NULL);
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		if( iPercentage == 100 )
		{
			SetMusicMode( MUSIC_MAIN_MENU );
			PlayJA2SampleFromFile("DemoAds/Hit.wav", HIGHVOLUME, 1, MIDDLEPAN);
			SetMusicFadeSpeed(50);
			BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
			ubCurrentScreen = 4;
			SetFontDestBuffer(FRAME_BUFFER, 0, 230, SCREEN_WIDTH, SCREEN_HEIGHT);
			SetFont( FONT10ARIAL );
			SetFontForeground( FONT_GRAY2 );
			uiStartTime = GetJA2Clock();
			DeleteVideoSurface(uiCollageID);
			iPrevPercentage = 0;
		}
		ubPreviousScreen = 3;
		return( DEMO_EXIT_SCREEN );
	}

	//Do the feature scrolling.
	if( ubCurrentScreen == 4 )
	{
		INT32 i, width, yp;
		uiTime = GetJA2Clock();
		//percentage is at 100 times higher resolution.
		iPercentage = (uiTime - uiStartTime) ;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += MAX( iPercentage - iPrevPercentage, 150 );
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
		iPercentage = MIN( iPercentage, 45000 );

		BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 100, 230, 440, 250 );
		InvalidateRegion( 100, 230, 540, 640 );
		yp = SCREEN_HEIGHT - iPercentage / 40; //500 (0%) to -500 (100%)
		yp = MAX( yp, -400 );
		for( i = 2; i < 40; i++ )
		{
			switch( i )
			{
				case 38:
				case 39: SetFontForeground(FONT_LTKHAKI); break;
				default: SetFontForeground(FONT_GRAY2);   break;
			}
			wchar_t const* const String = gpDemoString[i];
			Font           const font = (i < 36 ? FONT14ARIAL : FONT14HUMANIST);
			width = StringPixLength(String, font);
			SetFont(font);

			MPrint((SCREEN_WIDTH - width) / 2, yp + i * 17, String);
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
			//Create render buffer
			uiCollageID = AddVideoSurface(331, 148, PIXEL_DEPTH);
			uiCollageID->SetTransparency(0);

			//bring up the collage screen
			try
			{
				BltVideoObjectOnce(uiCollageID, "DemoAds/available.sti", 0, 0, 0);
			}
			catch (...)
			{
				ubCurrentScreen = 8;
				ubPreviousScreen = 4;
				return DEMO_EXIT_SCREEN;
			}
			uiStartTime = uiTime;
			BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
			PlayJA2SampleFromFile("DemoAds/Swoosh.wav", MIDVOLUME, 1, MIDDLEPAN);
		}

		iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += MAX( iPercentage - iPrevPercentage, 1 );
			else
				iPercentage += 1;
			iPrevPercentage = iPercentage;
		}
		if( gfPrevFastAnim )
		{
			uiStartTime = uiTime - iPrevPercentage * 12;
			iPercentage = (uiTime - uiStartTime) * 100 / 1200;
		}
		iPercentage = MIN( iPercentage, 100 );

		//Factor the percentage so that it is modified by a gravity falling acceleration effect.
		iFactor = (iPercentage - 50) * 2;
		if( iPercentage < 50 )
			iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
		else
			iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.05);

		if( iPercentage < 50 )
		{ //do the logo entrance section
			SGPBox const SrcRect =
			{
				165 - 165 * iPercentage / 50,
				 74 -  74 * iPercentage / 50,
				  1 + 330 * iPercentage / 50,
				  1 + 147 * iPercentage / 50
			};
			SGPBox const DstRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
			BltVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, NULL);
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		else
		{ //scale down the logo and stamp it.
			SGPBox const SrcRect = { 0, 0, 331, 148 };
			SGPBox const DstRect =
			{
				155 * (iPercentage - 50) / 50,
#ifdef GERMAN
				246 * (iPercentage - 50) / 50 - iPercentage + 50,
#else
				246 * (iPercentage - 50) / 50,
#endif
				SCREEN_WIDTH  - 309 * (iPercentage - 50) / 50,
				SCREEN_HEIGHT - 332 * (iPercentage - 50) / 50
			};
			BltVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, NULL);
			BltStretchVideoSurface(FRAME_BUFFER, uiCollageID, &SrcRect, &DstRect);
			InvalidateScreen();
		}
		if( iPercentage == 100 )
		{
			PlayJA2SampleFromFile("DemoAds/Hit.wav", HIGHVOLUME, 1, MIDDLEPAN);
			BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
			ubCurrentScreen = 6;
			DeleteVideoSurface(uiCollageID);
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
			SetFontAttributes(FONT14ARIAL, FONT_YELLOW);
		}
		iPercentage = (uiTime - uiStartTime) * 100 / 1600;
		if( gfFastAnim )
		{
			if( gfPrevFastAnim )
				iPercentage += MAX( iPercentage - iPrevPercentage, 1 );
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
		iPercentage = MIN( iPercentage, 100 );
		uiCharsToPrint = wcslen( gpDemoString[40] ) * iPercentage / 100;

		if( uiCharsToPrint > uiCharsPrinted )
		{
			uiCharsPrinted = uiCharsToPrint;
			PlayJA2Sample(ENTERING_TEXT, MIDVOLUME, 1, 39 + iPercentage / 2);
			wcscpy( str, gpDemoString[40] );
			str[ uiCharsToPrint ] = L'\0';
			const INT32 x = (SCREEN_WIDTH - uiWidthString) / 2;
#if defined GERMAN
			const INT32 y = 370;
#else
			const INT32 y = 420;
#endif
			MPrint(x, y, str);
			InvalidateRegion(x, y, x + uiWidthString, y + 13);
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
			uiStartTime = 4000000000U; //you could break the system by playing the game for 46 real-time days or longer.
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
}
#endif
