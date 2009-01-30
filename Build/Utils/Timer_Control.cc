#include "Debug.h"
#include "Soldier_Control.h"
#include "Timer_Control.h"
#include "Overhead.h"
#include "Handle_Items.h"
#include "WorldDef.h"
#include "Interface_Control.h"
#include <SDL.h>


INT32	giClockTimer = -1;
INT32	giTimerDiag = 0;

UINT32	guiBaseJA2Clock = 0;

static BOOLEAN gfPauseClock = FALSE;

const INT32 giTimerIntervals[NUMTIMERS] =
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
		200,				// TARGET REFINE TIMER
		150,					// CURSOR/AP FLASH
		20,					// PHYSICS UPDATE
		100,				// FADE ENEMYS
		20,					// STRATEGIC OVERHEAD
		40,
		500,				// NON GUN TARGET REFINE TIMER
		250,				// IMPROVED CURSOR FLASH
		500,				// 2nd CURSOR FLASH
		400,					// RADARMAP BLINK AND OVERHEAD MAP BLINK SHOUDL BE THE SAME
		10,					// Music Overhead
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
static SDL_TimerID g_timer;


extern UINT32 guiCompressionStringBaseTime;
extern INT32 giFlashHighlightedItemBaseTime;
extern INT32 giCompatibleItemBaseTime;
extern INT32 giAnimateRouteBaseTime;
extern INT32 giPotHeliPathBaseTime;
extern UINT32 guiSectorLocatorBaseTime;
extern INT32 giCommonGlowBaseTime;
extern INT32 giFlashAssignBaseTime;
extern INT32 giFlashContractBaseTime;
extern UINT32 guiFlashCursorBaseTime;
extern INT32 giPotCharPathBaseTime;


static UINT32 TimeProc(UINT32 const interval, void*)
{
	static BOOLEAN fInFunction = FALSE;

	if (!fInFunction)
	{
		fInFunction = TRUE;

		if (!gfPauseClock)
		{
			guiBaseJA2Clock += BASETIMESLICE;

			for (UINT32 i = 0; i != NUMTIMERS; i++)
			{
				UPDATECOUNTER(i);
			}

			// Update some specialized countdown timers...
			UPDATETIMECOUNTER(giTimerAirRaidQuote);
			UPDATETIMECOUNTER(giTimerAirRaidDiveStarted);
			UPDATETIMECOUNTER(giTimerAirRaidUpdate);
			UPDATETIMECOUNTER(giTimerTeamTurnUpdate);

			if (gpCustomizableTimerCallback)
			{
				UPDATETIMECOUNTER(giTimerCustomizable);
			}

#ifndef BOUNDS_CHECKER
			if (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN)
			{
				// IN Mapscreen, loop through player's team
				FOR_ALL_IN_TEAM(s, gbPlayerNum)
				{
					UPDATETIMECOUNTER(s->PortraitFlashCounter);
					UPDATETIMECOUNTER(s->PanelAnimateCounter);
				}
			}
			else
			{
				// Set update flags for soldiers
				FOR_ALL_MERCS(i)
				{
					SOLDIERTYPE* const s = *i;
					UPDATETIMECOUNTER(s->UpdateCounter);
					UPDATETIMECOUNTER(s->DamageCounter);
					UPDATETIMECOUNTER(s->ReloadCounter);
					UPDATETIMECOUNTER(s->BlinkSelCounter);
					UPDATETIMECOUNTER(s->PortraitFlashCounter);
					UPDATETIMECOUNTER(s->AICounter);
					UPDATETIMECOUNTER(s->FadeCounter);
					UPDATETIMECOUNTER(s->NextTileCounter);
					UPDATETIMECOUNTER(s->PanelAnimateCounter);
				}
			}
#endif
		}

		fInFunction = FALSE;
	}
	return interval;
}


void InitializeJA2Clock(void)
{
#ifdef CALLBACKTIMER
	SDL_InitSubSystem(SDL_INIT_TIMER);

	// Init timer delays
	for (INT32 i = 0; i != NUMTIMERS; ++i)
	{
		giTimerCounters[i] = giTimerIntervals[i];
	}

	g_timer = SDL_AddTimer(BASETIMESLICE, TimeProc, 0);
	if (!g_timer)
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Could not create timer callback");
	}
#endif
}


void ShutdownJA2Clock(void)
{
#ifdef CALLBACKTIMER
	SDL_RemoveTimer(g_timer);
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
	guiSectorLocatorBaseTime = uiCurrentTime;

	giCommonGlowBaseTime = uiCurrentTime;
	giFlashAssignBaseTime = uiCurrentTime;
	giFlashContractBaseTime = uiCurrentTime;
	guiFlashCursorBaseTime = uiCurrentTime;
	giPotCharPathBaseTime = uiCurrentTime;
}
