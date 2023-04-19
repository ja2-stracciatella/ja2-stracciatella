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
#include <array>
#include <stdexcept>
#include <utility>


static BOOLEAN gfPauseClock = FALSE;

static std::array<milliseconds, NUMTIMERS> const giTimerIntervals
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
static TIMECOUNTER giTimerCustomizable;

static double g_durations_multiplier;

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


void UpdateJA2Clock()
{
	static auto lastUpdate{ ReferenceClock::now() };

	auto const now{ ReferenceClock::now() };

	if (!gfPauseClock)
	{
		auto const timeDiff{ now - lastUpdate };
		guiBaseJA2Clock += static_cast<UINT32>(
			std::chrono::duration_cast<milliseconds>(timeDiff).count());
	}

	lastUpdate = now;
}


void InitializeJA2Clock(void)
{
	// Init timer delays
	for (INT32 i = 0; i != NUMTIMERS; ++i)
	{
		RESETCOUNTER(static_cast<PredefinedCounters>(i));
	}

	g_durations_multiplier = GCM->getGamePolicy()->game_durations_multiplier;
}


void PauseTime(BOOLEAN const fPaused)
{
	gfPauseClock = fPaused;
}


void SetCustomizableTimerCallbackAndDelay(ReferenceClock::duration const delay,
	CUSTOMIZABLE_TIMER_CALLBACK const callback, bool const replace)
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
	RESETTIMECOUNTER(giTimerCounters[pc], giTimerIntervals[pc]);
}

bool COUNTERDONE(PredefinedCounters const pc, bool const autoReset)
{
	// Timers never expire while time is paused.
	if (gfPauseClock) return false;

	bool const result{ ReferenceClock::now() >= giTimerCounters[pc] };
	if (result && autoReset) RESETCOUNTER(pc);
	return result;
}

void RESETTIMECOUNTER(TIMECOUNTER & tc, ReferenceClock::duration const duration)
{
	tc = ReferenceClock::now() + std::chrono::duration_cast
		<ReferenceClock::duration>(duration * g_durations_multiplier);
}

bool TIMECOUNTERDONE(TIMECOUNTER & tc, ReferenceClock::duration const duration)
{
	// Timers never expire while time is paused.
	if (gfPauseClock) return false;

	bool const result{ ReferenceClock::now() >= tc};
	if (result) RESETTIMECOUNTER(tc, duration);
	return result;
}

bool TIMECOUNTERDONE(TIMECOUNTER & tc, unsigned int const duration)
{
	// Timers never expire while time is paused.
	if (gfPauseClock) return false;

	bool const result{ ReferenceClock::now() >= tc};
	if (result && duration != 0) RESETTIMECOUNTER(tc, milliseconds{duration});
	return result;
}
