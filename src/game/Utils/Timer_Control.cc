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
#include <chrono>
#include <stdexcept>
#include <utility>
using namespace std::chrono_literals;


UINT32	guiTimerDiag  =  0;

UINT32 guiBaseJA2Clock = 0;

static BOOLEAN gfPauseClock = FALSE;

std::array<std::chrono::milliseconds, NUMTIMERS> const giTimerIntervals
{
	5ms, // Tactical Overhead
	20ms, // NEXTSCROLL
	200ms, // Start Scroll
	200ms, // Animate tiles
	1000ms, // FPS Counter
	80ms, // PATH FIND COUNTER
	150ms, // CURSOR TIMER
	300ms, // RIGHT CLICK FOR MENU
	300ms, // LEFT
	300ms, // MIDDLE
	200ms, // TARGET REFINE TIMER
	150ms, // CURSOR/AP FLASH
	20ms, // PHYSICS UPDATE
	100ms, // FADE ENEMYS
	20ms, // STRATEGIC OVERHEAD
	40ms, // CYCLE TIME RENDER ITEM COLOR
	500ms, // NON GUN TARGET REFINE TIMER
	250ms, // IMPROVED CURSOR FLASH
	500ms, // 2nd CURSOR FLASH
	400ms, // RADARMAP BLINK AND OVERHEAD MAP BLINK SHOUDL BE THE SAME
	10ms,  // Music Overhead
	100ms  // Team turn 
};

static std::array<TIMECOUNTER, NUMTIMERS> giTimerCounters;
TIMECOUNTER giTimerCustomizable;

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


static UINT32 TimeProc(UINT32 const interval, void*)
{
	static TIMECOUNTER lastUpdate;

	auto const now{ std::chrono::steady_clock::now() };

	if (!gfPauseClock)
	{
		auto const timeDiff = now - lastUpdate;
		guiBaseJA2Clock += std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count();
	}

	lastUpdate = now;
	return interval;
}


void InitializeJA2Clock(void)
{
	SDL_InitSubSystem(SDL_INIT_TIMER);

	// Init timer delays
	for (INT32 i = 0; i != NUMTIMERS; ++i)
	{
		RESETCOUNTER(static_cast<PredefinedCounters>(i));
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
	std::exchange(gpCustomizableTimerCallback, nullptr)();
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

void RESETCOUNTER(PredefinedCounters const pc)
{
	giTimerCounters[pc] = std::chrono::steady_clock::now() + giTimerIntervals[pc];
}

bool COUNTERDONE(PredefinedCounters const pc)
{
	return std::chrono::steady_clock::now() >= giTimerCounters[pc];
}

void RESETTIMECOUNTER(TIMECOUNTER & tc, unsigned int millis)
{
	tc = std::chrono::steady_clock::now() + std::chrono::milliseconds{millis};
}

bool TIMECOUNTERDONE(TIMECOUNTER tc, [[maybe_unused]] int)
{
	return std::chrono::steady_clock::now() >= tc;
}
