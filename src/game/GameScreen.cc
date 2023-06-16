#include "Local.h"
#include "GameLoop.h"
#include "VSurface.h"
#include "WorldDef.h"
#include "Input.h"
#include "Font.h"
#include "Screens.h"
#include "Overhead.h"
#include "SysUtil.h"
#include "Event_Pump.h"
#include "Font_Control.h"
#include "Debug_Pages.h"
#include "Timer_Control.h"
#include "Radar_Screen.h"
#include "Render_Dirty.h"
#include "Interface.h"
#include "Handle_UI.h"
#include "Cursors.h"
#include "VObject_Blitters.h"
#include "Button_System.h"
#include "RenderWorld.h"
#include "Sys_Globals.h"
#include "Environment.h"
#include "Bullets.h"
#include "Assignments.h"
#include "Message.h"
#include "Overhead_Map.h"
#include "Strategic_Exit_GUI.h"
#include "Strategic_Movement.h"
#include "Tactical_Placement_GUI.h"
#include "Game_Clock.h"
#include "Game_Init.h"
#include "Interface_Control.h"
#include "Physics.h"
#include "Fade_Screen.h"
#include "Dialogue_Control.h"
#include "Soldier_Macros.h"
#include "Faces.h"
#include "StrategicMap.h"
#include "GameScreen.h"
#include "Cursor_Control.h"
#include "Strategic_Turns.h"
#include "Merc_Entering.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "Interface_Panels.h"
#include "Map_Information.h"
#include "Squads.h"
#include "Interface_Dialogue.h"
#include "Auto_Bandage.h"
#include "Meanwhile.h"
#include "Strategic_AI.h"
#include "HelpScreen.h"
#include "PreBattle_Interface.h"
#include "Sound_Control.h"
#include "Text.h"
#include "Debug.h"
#include "Video.h"
#include "JAScreens.h"
#include "UILayout.h"
#include "GameMode.h"
#include "EditScreen.h"
#include "Logger.h"
#include "GameSettings.h"

#define ARE_IN_FADE_IN( )		( gfFadeIn || gfFadeInitialized )

BOOLEAN gfTacticalDoHeliRun = FALSE;
BOOLEAN	gfGameScreenLocateToSoldier = FALSE;
BOOLEAN	gfEnteringMapScreen					= FALSE;
SOLDIERTYPE* gPreferredInitialSelectedGuy = NULL;

static BOOLEAN      gfTacticalIsModal             = FALSE;
static MOUSE_REGION gTacticalDisableRegion;
static BOOLEAN      gfTacticalDisableRegionActive = FALSE;
MODAL_HOOK          gModalDoneCallback;
BOOLEAN             gfBeginEndTurn = FALSE;
extern BOOLEAN      gfFirstHeliRun;
extern BOOLEAN      gfRenderFullThisFrame;


// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game
static RENDER_HOOK gRenderOverride = 0;


static ScreenID guiTacticalLeaveScreenID = ERROR_SCREEN; // XXX TODO001A had no explicit initialisation
static BOOLEAN  guiTacticalLeaveScreen   = FALSE;


void MainGameScreenInit(void)
{
	// all blit functions expect z-buffer pitch to match framebuffer pitch
	SDL_Surface const& surface = FRAME_BUFFER->GetSDLSurface();
	gZBufferPitch = surface.pitch / surface.format->BytesPerPixel;
	gpZBuffer = InitZBuffer(gZBufferPitch, SCREEN_HEIGHT);
	gZBufferPitch *= sizeof(*gpZBuffer);
	InitializeBackgroundRects();

	//EnvSetTimeInHours(ENV_TIME_12);

	SetRenderFlags(RENDER_FLAG_FULL);
}


// The ShutdownGame function will free up/undo all things that were started in InitializeGame()
// It will also be responsible to making sure that all Gaming Engine tasks exit properly
void MainGameScreenShutdown(void)
{
	ShutdownZBuffer(gpZBuffer);
}


void FadeInGameScreen( )
{
	fFirstTimeInGameScreen = TRUE;

	FadeInNextFrame( );
}

void FadeOutGameScreen( )
{
	FadeOutNextFrame( );
}


void EnterTacticalScreen(void)
{
	guiTacticalLeaveScreen = FALSE;

	SetPositionSndsActive( );

	// Set pending screen
	SetPendingNewScreen( GAME_SCREEN );

	fInterfacePanelDirty = DIRTYLEVEL2;

	//Disable all faces
	SetAllAutoFacesInactive( );

	// CHECK IF OURGUY IS NOW OFF DUTY
	const SOLDIERTYPE* sel = GetSelectedMan();
	if (sel != NULL)
	{
		if (!OK_CONTROLLABLE_MERC(sel))
		{
			SelectNextAvailSoldier(sel);
			sel = GetSelectedMan();
		}
		// ATE: If the current guy is sleeping, change....
		if (sel != NULL && sel->fMercAsleep) SelectNextAvailSoldier(sel);
	}
	else
	{
		// otherwise, make sure interface is team panel...
		SetCurrentInterfacePanel(TEAM_PANEL);
	}

	if (!gfTacticalPlacementGUIActive) gRadarRegion.Enable();
	gViewportRegion.Enable();

	// set default squad on sector entry
	// ATE: moved these 2 call after initalizing the interface!
	//SetDefaultSquadOnSectorEntry( FALSE );
	//ExamineCurrentSquadLights( );

	// Init interface ( ALWAYS TO TEAM PANEL.  DEF changed it to go back to the previous panel )
	if( !gfTacticalPlacementGUIActive )
	{
		//make sure the gsCurInterfacePanel is valid
		if( gsCurInterfacePanel >= NUM_UI_PANELS )
			gsCurInterfacePanel = TEAM_PANEL;

		SetCurrentInterfacePanel(gsCurInterfacePanel);
	}

	// set default squad on sector entry
	SetDefaultSquadOnSectorEntry( FALSE );
	ExamineCurrentSquadLights( );



	fFirstTimeInGameScreen = FALSE;

	// Make sure it gets re-created....
	DirtyTopMessage( );

	// Set compression to normal!
	//SetGameTimeCompressionLevel( TIME_COMPRESS_X1 );

	// Select current guy...
	//gfGameScreenLocateToSoldier = TRUE;

	// Locate if in meanwhile...
	if ( AreInMeanwhile( ) )
	{
		LocateToMeanwhileCharacter( );
	}

	if ( gTacticalStatus.uiFlags & IN_DEIDRANNA_ENDGAME )
	{
		InternalLocateGridNo( 4561, TRUE );
	}

	// Clear tactical message q
	ClearTacticalMessageQueue( );

	// ATE: Enable messages again...
	EnableScrollMessages( );
}


void LeaveTacticalScreen(ScreenID const uiNewScreen)
{
	guiTacticalLeaveScreenID = uiNewScreen;
	guiTacticalLeaveScreen = TRUE;
}


void InternalLeaveTacticalScreen(ScreenID const uiNewScreen)
{
	gpCustomizableTimerCallback = NULL;

	// unload the sector they teleported out of
	if ( !gfAutomaticallyStartAutoResolve )
	{
		CheckAndHandleUnloadingOfCurrentWorld();
	}

	SetPositionSndsInActive( );

	fFirstTimeInGameScreen = TRUE;

	SetPendingNewScreen( uiNewScreen );

	//Disable all faces
	SetAllAutoFacesInactive( );

	ResetInterfaceAndUI( );

	// Remove cursor and reset height....
	gsGlobalCursorYOffset = 0;
	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	// Shutdown panel
	ShutdownCurrentPanel( );

	//disable the radar map
	gRadarRegion.Disable();
	//gViewportRegion.Disable();

	// We are leaving... turn off pedning autobadage...
	SetAutoBandagePending( FALSE );

	// ATE: Disable messages....
	DisableScrollMessages( );


	if ( uiNewScreen == MAINMENU_SCREEN )
	{
		//We want to reinitialize the game
		ReStartingGame();
	}

	if ( uiNewScreen != MAP_SCREEN )
	{
		StopAnyCurrentlyTalkingSpeech( );
	}

	// If we have some disabled screens up.....remove...
	CheckForDisabledRegionRemove( );

	// ATE: Record last time we were in tactical....
	gTacticalStatus.uiTimeSinceLastInTactical = GetWorldTotalMin( );

	FinishAnySkullPanelAnimations( );
}


static void HandleModalTactical(void);
static void TacticalScreenLocateToSoldier(void);


ScreenID MainGameScreenHandle(void)
{
	//DO NOT MOVE THIS FUNCTION CALL!!!
	//This determines if the help screen should be active
//	if( ( !gfTacticalDoHeliRun && !gfFirstHeliRun ) && ShouldTheHelpScreenComeUp( HELP_SCREEN_TACTICAL, FALSE ) )
	if( !gfPreBattleInterfaceActive && ShouldTheHelpScreenComeUp( HELP_SCREEN_TACTICAL, FALSE ) )
	{
		// handle the help screen
		HelpScreenHandler();
		return( GAME_SCREEN );
	}


	if ( HandleAutoBandage( ) )
	{
		#ifndef VISIBLE_AUTO_BANDAGE
			return( GAME_SCREEN );
		#endif
	}

	if ( gfBeginEndTurn )
	{
		UIHandleEndTurn( NULL );
		gfBeginEndTurn = FALSE;
	}

	if ( gfTacticalIsModal )
	{
		if ( gfTacticalIsModal == 1 )
		{
			gfTacticalIsModal++;
		}
		else
		{
			HandleModalTactical( );

			return( GAME_SCREEN );
		}
	}

	// OK, this is the pause system for when we see a guy...
	if ( !ARE_IN_FADE_IN( ) )
	{
		if ( gTacticalStatus.fEnemySightingOnTheirTurn )
		{
			if ( ( GetJA2Clock( ) - gTacticalStatus.uiTimeSinceDemoOn ) > 3000 )
			{
				SOLDIERTYPE* const s = gTacticalStatus.enemy_sighting_on_their_turn_enemy;
				if ( gTacticalStatus.ubCurrentTeam != OUR_TEAM )
				{
					AdjustNoAPToFinishMove(s, FALSE);
				}
				s->fPauseAllAnimation = FALSE;

				gTacticalStatus.fEnemySightingOnTheirTurn = FALSE;
			}
		}
	}

	// see if the helicopter is coming in this time for the initial entrance by the mercs
	InitHelicopterEntranceByMercs( );

	// Handle Environment controller here
	EnvironmentController( TRUE );

	if ( !ARE_IN_FADE_IN( ) )
	{
		HandleWaitTimerForNPCTrigger( );

		// Check timer that could have been set to do anything
		CheckCustomizableTimer();

		// HAndle physics engine
		SimulateWorld( );

		// Handle strategic engine
		HandleStrategicTurn( );
	}

	if ( gfTacticalDoHeliRun )
	{
		gfGameScreenLocateToSoldier = FALSE;
		InternalLocateGridNo( gMapInformation.sNorthGridNo, TRUE );

		// Start heli Run...
		StartHelicopterRun( gMapInformation.sNorthGridNo );

		// Update clock by one so that our DidGameJustStatrt() returns now false for things like LAPTOP, etc...
		SetGameTimeCompressionLevel( TIME_COMPRESS_X1 );
		//UpdateClock( 1 );

		gfTacticalDoHeliRun = FALSE;
		//SetMusicMode( MUSIC_TACTICAL_NOTHING );
	}


	if ( InOverheadMap( ) )
	{
		HandleOverheadMap( );
		return( GAME_SCREEN );
	}

	if ( gfGameScreenLocateToSoldier )
	{
		TacticalScreenLocateToSoldier( );
		gfGameScreenLocateToSoldier = FALSE;
	}

	if ( fFirstTimeInGameScreen )
	{
		EnterTacticalScreen( );

		// Select a guy if he hasn;'
		if( !gfTacticalPlacementGUIActive )
		{
			SOLDIERTYPE* const sel = GetSelectedMan();
			if (sel != NULL && OK_INTERRUPT_MERC(sel))
			{
				SelectSoldier(sel, SELSOLDIER_FORCE_RESELECT);
			}
		}
	}

	if ( HandleFadeOutCallback( ) )
	{
		return( GAME_SCREEN );
	}

	if ( guiCurrentScreen != MSG_BOX_SCREEN )
	{
		if ( HandleBeginFadeOut( GAME_SCREEN ) )
		{
			return( GAME_SCREEN );
		}
	}


	HandleHeliDrop( );

	if ( !ARE_IN_FADE_IN( ) )
	{
		HandleAutoBandagePending( );
	}


	// ATE: CHRIS_C LOOK HERE FOR GETTING AI CONSTANTLY GOING
	//	if ( !(gTacticalStatus.uiFlags & ENEMYS_TURN) )
	//	{
	//		EndTurn( );
	//	}


	if (!ARE_IN_FADE_IN())
	{
		UpdateBullets();

		// Execute Tactical Overhead
		ExecuteOverhead();
	}

	// Handle animated cursors
	if (gfWorldLoaded)
	{
		HandleAnimatedCursors();

		// Handle Interface
		ScreenID const uiNewScreen = HandleTacticalUI();

		HandleTalkingAutoFaces();

		if (uiNewScreen != GAME_SCREEN) return uiNewScreen;
	}
	else if (gfIntendOnEnteringEditor && GameMode::getInstance()->isEditorMode())
	{
		SLOGI("Aborting normal game mode and entering editor mode...");
		SetPendingNewScreen(NO_PENDING_SCREEN);
		return EDIT_SCREEN;
	}
	else if (!gfEnteringMapScreen)
	{
		gfEnteringMapScreen = TRUE;
	}

	// Deque all game events
	if (!ARE_IN_FADE_IN())
	{
		DequeAllGameEvents();
	}


	HandleTopMessages( );

	if (!gfScrollPending && !g_scroll_inertia)
	{
		// Handle Interface Stuff
		SetUpInterface( );
		HandleTacticalPanelSwitch( );
	} else {
		if( gGameSettings.fOptions[TOPTION_MERC_CASTS_LIGHT] && NightTime()) {
			SetRenderFlags( RENDER_FLAG_FULL );
		}
	}

	// Handle Scroll Of World
	ScrollWorld( );

	//SetRenderFlags( RENDER_FLAG_FULL );

	RenderWorld( );

	if ( gRenderOverride != NULL )
	{
		gRenderOverride( );
	}

	if (gfScrollPending || g_scroll_inertia)
	{
		RenderTacticalInterfaceWhileScrolling( );
	}
	else
	{
		// Handle Interface Stuff
		//RenderTacticalInterface( );
	}

	// Render Interface
	RenderTopmostTacticalInterface( );

	// Render view window
	RenderRadarScreen( );

	ResetInterface( );

	if ( gfScrollPending  )
	{
		AllocateVideoOverlaysArea( );
		SaveVideoOverlaysArea( FRAME_BUFFER );
		ExecuteVideoOverlays( );
	}
	else
	{
		ExecuteVideoOverlays( );
	}

	// Adding/deleting of video overlays needs to be done below
	// ExecuteVideoOverlays( )....

	// Handle dialogue queue system
	if ( !ARE_IN_FADE_IN( ) )
	{
		HandleDialogue( );
	}

	//Don't render if we have a scroll pending!
	if (!gfScrollPending && !g_scroll_inertia && !gfRenderFullThisFrame)
	{
		RenderFastHelp( );
	}

	CheckForMeanwhileOKStart( );

	ScrollString( );

	if ( HandleFadeInCallback( ) )
	{
		// Re-render the scene!
		SetRenderFlags( RENDER_FLAG_FULL );
		fInterfacePanelDirty = DIRTYLEVEL2;
	}

	if ( HandleBeginFadeIn( GAME_SCREEN ) )
	{
		guiTacticalLeaveScreenID = FADE_SCREEN;
	}

	if ( guiTacticalLeaveScreen )
	{
		guiTacticalLeaveScreen		= FALSE;

		InternalLeaveTacticalScreen( guiTacticalLeaveScreenID );
	}

	// Check if we are to enter map screen
	if ( gfEnteringMapScreen == 2 )
	{
		gfEnteringMapScreen = FALSE;
		LeaveTacticalScreen(MAP_SCREEN);
	}


	// Are we entering map screen? if so, wait a frame!
	if ( gfEnteringMapScreen > 0 )
	{
		gfEnteringMapScreen++;
	}

	return( GAME_SCREEN );

}


void SetRenderHook( RENDER_HOOK pRenderOverride )
{
	gRenderOverride = pRenderOverride;
}


static void TacticalScreenLocateToSoldier(void)
{
	SOLDIERTYPE* const prefer = gPreferredInitialSelectedGuy;
	if (prefer != NULL)
	{
		gPreferredInitialSelectedGuy = NULL;
		if (OK_CONTROLLABLE_MERC(prefer) && OK_INTERRUPT_MERC(prefer))
		{
			LocateSoldier(prefer, 10);
			SelectSoldier(prefer, SELSOLDIER_FORCE_RESELECT);
			return;
		}
	}

	// Set locator to first merc
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (OkControllableMerc(s) && OK_INTERRUPT_MERC(s))
		{
			LocateSoldier(s, 10);
			SelectSoldier(s, SELSOLDIER_FORCE_RESELECT);
			break;
		}
	}
}


void UpdateTeamPanelAssignments( )
{
	// Remove all players
	RemoveAllPlayersFromSlot( );

	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// Setup team interface
		CheckForAndAddMercToTeamPanel(s);
	}
}


void EnterModalTactical( INT8 bMode )
{
	gfTacticalIsModal = TRUE;

	if (bMode == TACTICAL_MODAL_NOMOUSE)
	{
		if ( !gfTacticalDisableRegionActive )
		{
			gfTacticalDisableRegionActive = TRUE;

			MSYS_DefineRegion(&gTacticalDisableRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH, VIDEO_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
		}
	}

	UpdateSaveBuffer();

}

void EndModalTactical( )
{
	if ( gfTacticalDisableRegionActive )
	{
		MSYS_RemoveRegion( &gTacticalDisableRegion );

		gfTacticalDisableRegionActive = FALSE;
	}


	if ( gModalDoneCallback != NULL )
	{
		gModalDoneCallback( );

		gModalDoneCallback = NULL;
	}

	gfTacticalIsModal = FALSE;

	SetRenderFlags( RENDER_FLAG_FULL );
}


static void HandleModalTactical(void)
{
	RestoreBackgroundRects();

	RenderWorld( );
	RenderRadarScreen( );
	ExecuteVideoOverlays( );

	// Handle dialogue queue system
	HandleDialogue( );

	HandleTalkingAutoFaces( );

	// Handle faces
	HandleAutoFaces( );

	if ( gfInSectorExitMenu )
	{
		RenderSectorExitMenu( );
	}
	RenderButtons();

	SaveBackgroundRects( );
	RenderFastHelp();
	RenderPausedGameBox( );
}


void InitHelicopterEntranceByMercs( void )
{
	if( DidGameJustStart() )
	{
		// Update clock ahead from STARTING_TIME to make mercs arrive!
		WarpGameTime( FIRST_ARRIVAL_DELAY, WARPTIME_PROCESS_EVENTS_NORMALLY );

		gfTacticalDoHeliRun = TRUE;
		gfFirstHeliRun			= TRUE;

		gTacticalStatus.fDidGameJustStart = FALSE;
	}
}
