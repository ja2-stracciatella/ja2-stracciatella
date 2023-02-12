#include "Timer_Control.h"

#include "ContentManager.h"
#include "Debug.h"
#include "GameInstance.h"
#include "GamePolicy.h"
#include "Handle_Items.h"
#include "MapScreen.h"
#include "Overhead.h"
#include "Soldier_Control.h"
#include "WorldDef.h"

#include <SDL.h>
#include <stdexcept>


UINT32	guiClockTimer = UINT32_MAX;
UINT32	guiTimerDiag  =  0;

Uint64 last_ticks = 0;

UINT32 guiBaseJA2Clock = 0;

static BOOLEAN gfPauseClock = FALSE;

const INT32 giTimerIntervals[NUMTIMERS] =
{
	5, // Tactical Overhead
	20, // NEXTSCROLL
	200, // Start Scroll
	200, // Animate tiles
	1000, // FPS Counter
	80, // PATH FIND COUNTER
	150, // CURSOR TIMER
	300, // RIGHT CLICK FOR MENU
	300, // LEFT
	300, // MIDDLE
	200, // TARGET REFINE TIMER
	150, // CURSOR/AP FLASH
	20, // PHYSICS UPDATE
	100, // FADE ENEMYS
	20, // STRATEGIC OVERHEAD
	40, // CYCLE TIME RENDER ITEM COLOR
	500, // NON GUN TARGET REFINE TIMER
	250, // IMPROVED CURSOR FLASH
	500, // 2nd CURSOR FLASH
	400, // RADARMAP BLINK AND OVERHEAD MAP BLINK SHOUDL BE THE SAME
	10  // Music Overhead
};

// TIMER COUNTERS
INT32 giTimerCounters[NUMTIMERS];

INT32 giTimerCustomizable       = 0;
INT32 giTimerTeamTurnUpdate     = 0;

CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback = 0;

// Clock Callback event ID
static SDL_TimerID g_timer;


extern UINT32 guiCompressionStringBaseTime;
extern UINT32 guiFlashHighlightedItemBaseTime;
extern UINT32 guiCompatibleItemBaseTime;
extern UINT32 guiAnimateRouteBaseTime;
extern UINT32 guiPotHeliPathBaseTime;
extern UINT32 guiSectorLocatorBaseTime;
extern UINT32 guiCommonGlowBaseTime;
extern UINT32 guiFlashAssignBaseTime;
extern UINT32 guiFlashContractBaseTime;
extern UINT32 guiFlashCursorBaseTime;
extern UINT32 guiPotCharPathBaseTime;


#define UPDATECOUNTER( c, n )		( ( giTimerCounters[ c ] - n ) < 0 ) ?  ( giTimerCounters[ c ] = 0 ) : ( giTimerCounters[ c ] -= n )
#define UPDATETIMECOUNTER( c, n )		( ( c - n ) < 0 ) ?  ( c = 0 ) : ( c -= n )

static UINT32 TimeProc(UINT32 const interval, void*)
{
	Uint64 new_last_ticks = SDL_GetTicks64();
	Sint64 elapsed = new_last_ticks - last_ticks;
	last_ticks = new_last_ticks;

	if (!gfPauseClock)
	{
		guiBaseJA2Clock += elapsed;

		for (UINT32 i = 0; i != NUMTIMERS; i++)
		{
			UPDATECOUNTER(i, elapsed);
		}

		// Update some specialized countdown timers...
		UPDATETIMECOUNTER(giTimerTeamTurnUpdate, elapsed);

		if (gpCustomizableTimerCallback)
		{
			UPDATETIMECOUNTER(giTimerCustomizable, elapsed);
		}

#ifndef BOUNDS_CHECKER
		if (fInMapMode)
		{
			// IN Mapscreen, loop through player's team
			FOR_EACH_IN_TEAM(s, OUR_TEAM)
			{
				UPDATETIMECOUNTER(s->PortraitFlashCounter, elapsed);
				UPDATETIMECOUNTER(s->PanelAnimateCounter, elapsed);
			}
		}
		else
		{
			// Set update flags for soldiers
			FOR_EACH_MERC(i)
			{
				SOLDIERTYPE* const s = *i;
				UPDATETIMECOUNTER(s->UpdateCounter, elapsed);
				UPDATETIMECOUNTER(s->DamageCounter, elapsed);
				UPDATETIMECOUNTER(s->BlinkSelCounter, elapsed);
				UPDATETIMECOUNTER(s->PortraitFlashCounter, elapsed);
				UPDATETIMECOUNTER(s->AICounter, elapsed);
				UPDATETIMECOUNTER(s->FadeCounter, elapsed);
				UPDATETIMECOUNTER(s->NextTileCounter, elapsed);
				UPDATETIMECOUNTER(s->PanelAnimateCounter, elapsed);
			}
		}
#endif
	}

	return interval;
}


void InitializeJA2Clock(void)
{
	SDL_InitSubSystem(SDL_INIT_TIMER);

	last_ticks = SDL_GetTicks64();

	// Init timer delays
	for (INT32 i = 0; i != NUMTIMERS; ++i)
	{
		giTimerCounters[i] = giTimerIntervals[i];
	}

	INT32 msPerTimeSlice = gamepolicy(ms_per_time_slice);
	if (msPerTimeSlice <= 0)
	{
		throw std::runtime_error("ms_per_time_slice must be a positive integer");
	}
	g_timer = SDL_AddTimer(msPerTimeSlice, TimeProc, 0);
	if (!g_timer) throw std::runtime_error("Could not create timer callback");
}


void ShutdownJA2Clock(void)
{
	SDL_RemoveTimer(g_timer);
}


void PauseTime(BOOLEAN const fPaused)
{
	gfPauseClock = fPaused;
}


void SetCustomizableTimerCallbackAndDelay(INT32 const delay, CUSTOMIZABLE_TIMER_CALLBACK const callback, BOOLEAN const replace)
{
	if (!replace && gpCustomizableTimerCallback)
	{ // Replace callback but call the current callback first
		gpCustomizableTimerCallback();
	}

	RESETTIMECOUNTER(giTimerCustomizable, delay);
	gpCustomizableTimerCallback = callback;
}


void CheckCustomizableTimer(void)
{
	if (!gpCustomizableTimerCallback)             return;
	if (!TIMECOUNTERDONE(giTimerCustomizable, 0)) return;

	/* Set the callback to a temp variable so we can reset the global variable
	 * before calling the callback, so that if the callback sets up another
	 * instance of the timer, we don't reset it afterwards. */
	CUSTOMIZABLE_TIMER_CALLBACK const callback = gpCustomizableTimerCallback;
	gpCustomizableTimerCallback = 0;
	callback();
}


void ResetJA2ClockGlobalTimers(void)
{
	UINT32 const now = GetJA2Clock();

	guiCompressionStringBaseTime    = now;
	guiFlashHighlightedItemBaseTime = now;
	guiCompatibleItemBaseTime       = now;
	guiAnimateRouteBaseTime         = now;
	guiPotHeliPathBaseTime          = now;
	guiSectorLocatorBaseTime        = now;

	guiCommonGlowBaseTime           = now;
	guiFlashAssignBaseTime          = now;
	guiFlashContractBaseTime        = now;
	guiFlashCursorBaseTime          = now;
	guiPotCharPathBaseTime          = now;
}
