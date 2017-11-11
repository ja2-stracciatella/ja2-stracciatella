#ifndef __TIMER_CONTROL_H
#define __TIMER_CONTROL_H

#include "Types.h"


#ifndef CALLBACKTIMER
#define CALLBACKTIMER
#endif

typedef void (*CUSTOMIZABLE_TIMER_CALLBACK) ( void );

// TIMER DEFINES
enum
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
	NUMTIMERS
};

// Base resultion of callback timer
#define BASETIMESLICE 10

extern const INT32 giTimerIntervals[NUMTIMERS];
extern INT32       giTimerCounters[NUMTIMERS];

// GLOBAL SYNC TEMP TIME
extern INT32 giClockTimer;

extern INT32 giTimerDiag;

extern INT32 giTimerTeamTurnUpdate;


void InitializeJA2Clock(void);
void ShutdownJA2Clock(void);

#define GetJA2Clock() guiBaseJA2Clock

void PauseTime( BOOLEAN fPaused );

void SetCustomizableTimerCallbackAndDelay( INT32 iDelay, CUSTOMIZABLE_TIMER_CALLBACK pCallback, BOOLEAN fReplace );
void CheckCustomizableTimer( void );

//Don't modify this value
extern UINT32	guiBaseJA2Clock;
extern CUSTOMIZABLE_TIMER_CALLBACK gpCustomizableTimerCallback;

// MACROS
// Check if new counter < 0        | set to 0 |        Decrement

#ifdef CALLBACKTIMER

#define UPDATECOUNTER( c )		( ( giTimerCounters[ c ] - BASETIMESLICE ) < 0 ) ?  ( giTimerCounters[ c ] = 0 ) : ( giTimerCounters[ c ] -= BASETIMESLICE )
#define RESETCOUNTER( c )		( giTimerCounters[ c ] = giTimerIntervals[ c ] )
#define COUNTERDONE( c )		( giTimerCounters[ c ] == 0 ) ? TRUE : FALSE

#define UPDATETIMECOUNTER( c )		( ( c - BASETIMESLICE ) < 0 ) ?  ( c = 0 ) : ( c -= BASETIMESLICE )
#define RESETTIMECOUNTER( c, d )	( c = d )

#ifdef BOUNDS_CHECKER
	#define TIMECOUNTERDONE(c, d) true
#else
	#define TIMECOUNTERDONE(c, d) (c == 0)
#endif

#define SYNCTIMECOUNTER()		(void)0
#define ZEROTIMECOUNTER( c )		( c = 0 )

#else

#define UPDATECOUNTER( c )
#define RESETCOUNTER( c )		( giTimerCounters[ c ] = giClockTimer )
#define COUNTERDONE( c )		((((giClockTimer = GetJA2Clock()) - giTimerCounters[c]) >  giTimerIntervals[c]) ? TRUE : FALSE)

#define UPDATETIMECOUNTER( c )
#define RESETTIMECOUNTER( c, d )	( c = giClockTimer )
#define TIMECOUNTERDONE(c, d)		(giClockTimer - c >  d)
#define SYNCTIMECOUNTER( )		( giClockTimer = GetJA2Clock() )

#endif

// whenever guiBaseJA2Clock changes, we must reset all the timer variables that
// use it as a reference
void ResetJA2ClockGlobalTimers(void);

#endif
