#ifndef __TIMER_CONTROL_H
#define __TIMER_CONTROL_H

#include "JA2Types.h"
#include <array>

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

// GLOBAL SYNC TEMP TIME
extern UINT32 guiTimerDiag;

void InitializeJA2Clock(void);
void ShutdownJA2Clock(void);

#define GetJA2Clock() guiBaseJA2Clock

void PauseTime( BOOLEAN fPaused );

void SetCustomizableTimerCallbackAndDelay( INT32 iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback, BOOLEAN fReplace );
void CheckCustomizableTimer( void );

//Don't modify this value
extern UINT32	guiBaseJA2Clock;
extern CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback;

void RESETCOUNTER(PredefinedCounters);
bool COUNTERDONE(PredefinedCounters);
void RESETTIMECOUNTER(TIMECOUNTER &, unsigned int millis);
bool TIMECOUNTERDONE(TIMECOUNTER, [[maybe_unused]] int = 0);
constexpr void ZEROTIMECOUNTER(TIMECOUNTER & tc) { tc = {}; }

#define SYNCTIMECOUNTER()		(void)0

// whenever guiBaseJA2Clock changes, we must reset all the timer variables that
// use it as a reference
void ResetJA2ClockGlobalTimers(void);

#endif
