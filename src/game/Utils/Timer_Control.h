#ifndef __TIMER_CONTROL_H
#define __TIMER_CONTROL_H

#include <chrono>
#include <cstdint>

using ReferenceClock = std::chrono::steady_clock;
using TIMECOUNTER = std::chrono::time_point<ReferenceClock>;
using std::chrono::milliseconds;
using namespace std::chrono_literals;

typedef void (*CUSTOMIZABLE_TIMER_CALLBACK) ( void );

// TIMER DEFINES
enum PredefinedCounters
{
	TOVERHEAD = 0,			// Overhead time slice
	NEXTSCROLL,			// Scroll Speed timer
	STARTSCROLL,			// Scroll Start timer
	ANIMATETILES,			// Animate tiles timer
	FPSCOUNTER,			// FPS value
	PATHFINDCOUNTER,		// PATH FIND COUNTER
	CURSORCOUNTER,			// ANIMATED CURSOR
	RMOUSECLICK_DELAY_COUNTER,	// RIGHT BUTTON CLICK DELAY
	LMOUSECLICK_DELAY_COUNTER,	// LEFT  BUTTON CLICK DELAY
	MMOUSECLICK_DELAY_COUNTER,	// LEFT  BUTTON CLICK DELAY
	TARGETREFINE,			// TARGET REFINE
	CURSORFLASH,			// Cursor/AP flash
	PHYSICSUPDATE,			// PHYSICS UPDATE.
	GLOW_ENEMYS,
	STRATEGIC_OVERHEAD,		// STRATEGIC OVERHEAD
	CYCLERENDERITEMCOLOR,		// CYCLE COLORS
	NONGUNTARGETREFINE,		// TARGET REFINE
	CURSORFLASHUPDATE,		//
	INVALID_AP_HOLD,		// TIME TO HOLD INVALID AP
	RADAR_MAP_BLINK,		// BLINK DELAY FOR RADAR MAP
	MUSICOVERHEAD,			// MUSIC TIMER
	TEAMTURNUPDATE,
	NUMTIMERS
};

void InitializeJA2Clock(void);

void PauseTime(bool fPaused);

void SetCustomizableTimerCallbackAndDelay(ReferenceClock::duration, CUSTOMIZABLE_TIMER_CALLBACK, bool fReplace);
void CheckCustomizableTimer();

//Don't modify this value
inline std::uint32_t guiBaseJA2Clock;
void UpdateJA2Clock();
[[nodiscard]] static inline std::uint32_t GetJA2Clock() { return guiBaseJA2Clock; }

inline CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback{nullptr};

// Test if the given counter has elapsed. Calls RESETCOUNTER for this counter
// if autoReset is true and the timer has elapsed.
[[nodiscard]] bool COUNTERDONE(PredefinedCounters, bool autoReset = true);

void RESETCOUNTER(PredefinedCounters);

// Test if the given counter has elapsed. Calls RESETCOUNTER for this counter
// if the timer has elapsed.
[[nodiscard]] bool TIMECOUNTERDONE(TIMECOUNTER &, ReferenceClock::duration);
// As above, except that you can specify millis == 0 if you do not want to
// automatically reset the counter.
[[nodiscard]] bool TIMECOUNTERDONE(TIMECOUNTER &, unsigned int millis);


void RESETTIMECOUNTER(TIMECOUNTER &, ReferenceClock::duration);
// This function exists for backwards compatibility only. In new code please
// use a proper chrono type to specify the duration.
static inline void RESETTIMECOUNTER(TIMECOUNTER & tc, unsigned int const millis)
{
	RESETTIMECOUNTER(tc, milliseconds{millis});
}
static inline void ZEROTIMECOUNTER(TIMECOUNTER & tc) { tc = ReferenceClock::now(); }

// whenever guiBaseJA2Clock changes, we must reset all the timer variables that
// use it as a reference
void ResetJA2ClockGlobalTimers(void);

#endif
