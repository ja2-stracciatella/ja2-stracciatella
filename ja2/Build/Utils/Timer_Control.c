#include <string.h>
#include "WCheck.h"
#include "stdlib.h"
#include "Debug.h"
#include "Soldier_Control.h"
#include "Timer_Control.h"
#include "Overhead.h"
#include "Handle_Items.h"
#include "WorldDef.h"
#include "Interface_Control.h"
#include <SDL.h>
#include "Stubs.h"


INT32	giClockTimer = -1;
INT32	giTimerDiag = 0;

UINT32	guiBaseJA2Clock = 0;
UINT32	guiBaseJA2NoPauseClock = 0;

BOOLEAN	gfPauseClock = FALSE;

INT32		giTimerIntervals[ NUMTIMERS ] =
{
		5,					// Tactical Overhead
		20,					// NEXTSCROLL
		200,				// Start Scroll
		200,				// Animate tiles
		1000,				// FPS Counter
		80,					// PATH FIND COUNTER
		150,				// CURSOR TIMER
		250,				// RIGHT CLICK FOR MENU
		300,				// LEFT
		30,					// SLIDING TEXT
		200,				// TARGET REFINE TIMER
		150,					// CURSOR/AP FLASH
		60,					// FADE MERCS OUT
		160,				// PANEL SLIDE
		1000,				// CLOCK UPDATE DELAY
		20,					// PHYSICS UPDATE
		100,				// FADE ENEMYS
		20,					// STRATEGIC OVERHEAD
		40,
		500,				// NON GUN TARGET REFINE TIMER
		250,				// IMPROVED CURSOR FLASH
		500,				// 2nd CURSOR FLASH
		400,					// RADARMAP BLINK AND OVERHEAD MAP BLINK SHOUDL BE THE SAME
		400,
		10,					// Music Overhead
		100,				// Rubber band start delay
};

// TIMER COUNTERS
INT32		giTimerCounters[ NUMTIMERS ];

INT32		giTimerAirRaidQuote				= 0;
INT32		giTimerAirRaidDiveStarted = 0;
INT32		giTimerAirRaidUpdate			= 0;
INT32		giTimerCustomizable				= 0;
INT32		giTimerTeamTurnUpdate			= 0;

CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback = NULL;

// Clock Callback event ID
SDL_TimerID	gTimerID;

// GLOBALS FOR CALLBACK
UINT32				gCNT;
SOLDIERTYPE		*gPSOLDIER;

// GLobal for displaying time diff ( DIAG )
UINT32		guiClockDiff = 0;
UINT32		guiClockStart = 0;


extern UINT32 guiCompressionStringBaseTime;
extern INT32 giFlashHighlightedItemBaseTime;
extern INT32 giCompatibleItemBaseTime;
extern INT32 giAnimateRouteBaseTime;
extern INT32 giPotHeliPathBaseTime;
extern INT32 giClickHeliIconBaseTime;
extern INT32 giExitToTactBaseTime;
extern UINT32 guiSectorLocatorBaseTime;
extern INT32 giCommonGlowBaseTime;
extern INT32 giFlashAssignBaseTime;
extern INT32 giFlashContractBaseTime;
extern UINT32 guiFlashCursorBaseTime;
extern INT32 giPotCharPathBaseTime;


static UINT32 CALLBACK TimeProc(UINT32 interval, void* params)
{
	static BOOLEAN fInFunction = FALSE;
	//SOLDIERTYPE		*pSoldier;
	Assert(interval > 0);

	if ( !fInFunction )
	{
		fInFunction = TRUE;

		guiBaseJA2NoPauseClock += BASETIMESLICE;


		if ( !gfPauseClock )
		{
			guiBaseJA2Clock += BASETIMESLICE;

			for ( gCNT = 0; gCNT < NUMTIMERS; gCNT++ )
			{
				UPDATECOUNTER( gCNT );
			}

			// Update some specialized countdown timers...
			UPDATETIMECOUNTER( giTimerAirRaidQuote );
			UPDATETIMECOUNTER( giTimerAirRaidDiveStarted );
			UPDATETIMECOUNTER( giTimerAirRaidUpdate );
			UPDATETIMECOUNTER( giTimerTeamTurnUpdate );

			if ( gpCustomizableTimerCallback )
			{
				UPDATETIMECOUNTER( giTimerCustomizable );
			}

#ifndef BOUNDS_CHECKER

      // If mapscreen...
      if( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN )
      {
        // IN Mapscreen, loop through player's team.....
			  for ( gCNT = gTacticalStatus.Team[ gbPlayerNum ].bFirstID; gCNT <= gTacticalStatus.Team[ gbPlayerNum ].bLastID; gCNT++ )
			  {
				  gPSOLDIER = MercPtrs[ gCNT ];
					UPDATETIMECOUNTER( gPSOLDIER->PortraitFlashCounter );
					UPDATETIMECOUNTER( gPSOLDIER->PanelAnimateCounter );
        }
      }
      else
      {
			  // Set update flags for soldiers
			  ////////////////////////////
			  for ( gCNT = 0; gCNT < guiNumMercSlots; gCNT++ )
			  {
				  gPSOLDIER = MercSlots[ gCNT ];

				  if ( gPSOLDIER != NULL )
				  {
					  UPDATETIMECOUNTER( gPSOLDIER->UpdateCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->DamageCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->ReloadCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->FlashSelCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->BlinkSelCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->PortraitFlashCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->AICounter );
					  UPDATETIMECOUNTER( gPSOLDIER->FadeCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->NextTileCounter );
					  UPDATETIMECOUNTER( gPSOLDIER->PanelAnimateCounter );
				  }
			  }
      }
#endif
		}


		fInFunction = FALSE;
	}
	return interval;
}



BOOLEAN InitializeJA2Clock(void)
{
#ifdef CALLBACKTIMER


	INT32			cnt;
	SDL_InitSubSystem(SDL_INIT_TIMER);

	// Init timer delays
	for ( cnt = 0; cnt < NUMTIMERS; cnt++ )
	{
		giTimerCounters[ cnt ] = giTimerIntervals[ cnt ];
	}


	gTimerID = SDL_AddTimer(BASETIMESLICE,&TimeProc,NULL);

	if ( !gTimerID )
	{
		 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Could not create timer callback");
	}


#endif
  return TRUE;
}

void    ShutdownJA2Clock(void)
{
#ifdef CALLBACKTIMER

	SDL_RemoveTimer(gTimerID);

#endif

}


void PauseTime( BOOLEAN fPaused )
{
	gfPauseClock = fPaused;
}

void SetCustomizableTimerCallbackAndDelay( INT32 iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback, BOOLEAN fReplace )
{
	if ( gpCustomizableTimerCallback )
	{
		if ( !fReplace )
		{
			// replace callback but call the current callback first
			gpCustomizableTimerCallback();
		}
	}

	RESETTIMECOUNTER( giTimerCustomizable, iDelay );
	gpCustomizableTimerCallback = pCallback;
}

void CheckCustomizableTimer( void )
{
	if ( gpCustomizableTimerCallback )
	{
		if ( TIMECOUNTERDONE( giTimerCustomizable, 0 ) )
		{
			// set the callback to a temp variable so we can reset the global variable
			// before calling the callback, so that if the callback sets up another
			// instance of the timer, we don't reset it afterwards
			CUSTOMIZABLE_TIMER_CALLBACK pTempCallback;

			pTempCallback = gpCustomizableTimerCallback;
			gpCustomizableTimerCallback = NULL;
			pTempCallback();
		}
	}
}



void ResetJA2ClockGlobalTimers( void )
{
	UINT32 uiCurrentTime = GetJA2Clock();


	guiCompressionStringBaseTime = uiCurrentTime;
	giFlashHighlightedItemBaseTime = uiCurrentTime;
	giCompatibleItemBaseTime = uiCurrentTime;
  giAnimateRouteBaseTime = uiCurrentTime;
	giPotHeliPathBaseTime = uiCurrentTime;
	giClickHeliIconBaseTime = uiCurrentTime;
	giExitToTactBaseTime = uiCurrentTime;
	guiSectorLocatorBaseTime = uiCurrentTime;

	giCommonGlowBaseTime = uiCurrentTime;
	giFlashAssignBaseTime = uiCurrentTime;
	giFlashContractBaseTime = uiCurrentTime;
	guiFlashCursorBaseTime = uiCurrentTime;
	giPotCharPathBaseTime = uiCurrentTime;
}
