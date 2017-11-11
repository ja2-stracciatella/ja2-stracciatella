#ifndef __WORLD_CLOCK
#define __WORLD_CLOCK

#include "Types.h"


#define NUM_SEC_IN_DAY		86400
#define NUM_SEC_IN_HOUR		3600
#define NUM_SEC_IN_MIN		60
#define ROUNDTO_MIN		5

#define NUM_MIN_IN_DAY		1440
#define NUM_MIN_IN_HOUR		60

//Kris:
//This is the plan for game time...
//Game time should be restricted to outside code.  Think of it as encapsulation.  Anyway, using these
//simple functions, you will be able to change the amount of time that passes per frame.  The gameloop will
//automatically update the clock once per cycle, regardless of the mode you are in.
//This does pose potential problems in modes such as the editor, or similar where time shouldn't pass, and
//isn't currently handled.  The best thing to do in these cases is call the PauseGame() function when entering
//such a mode, and UnPauseGame() when finished.  Everything will be restored just the way you left it.  This
//is much simpler to handle in the overall scheme of things.

enum LockPauseReason
{
	LOCK_PAUSE_01 =  1,
	LOCK_PAUSE_02 =  2,
	LOCK_PAUSE_04 =  4,
	LOCK_PAUSE_05 =  5,
	LOCK_PAUSE_06 =  6,
	LOCK_PAUSE_07 =  7,
	LOCK_PAUSE_08 =  8,
	LOCK_PAUSE_09 =  9,
	LOCK_PAUSE_10 = 10,
	LOCK_PAUSE_11 = 11,
	LOCK_PAUSE_12 = 12,
	LOCK_PAUSE_13 = 13,
	LOCK_PAUSE_14 = 14,
	LOCK_PAUSE_15 = 15,
	LOCK_PAUSE_16 = 16,
	LOCK_PAUSE_17 = 17,
	LOCK_PAUSE_18 = 18,
	LOCK_PAUSE_19 = 19,
	LOCK_PAUSE_20 = 20,
	LOCK_PAUSE_21 = 21
};

//PAUSE FEATURES
//Pauses and unpauses the game.  It sets and clears a flag which preserves the time rate.
void PauseGame(void);
void UnPauseGame(void);
BOOLEAN GamePaused(void);
void LockPauseState(LockPauseReason);
void UnLockPauseState(void);
BOOLEAN PauseStateLocked(void);

//USING HIGH RESOLUTION TIME RATE MANIPULATION/ACCESS
//Allows external code to change the time rate.
void SetGameHoursPerSecond( UINT32 uiGameHoursPerSecond );
void SetGameMinutesPerSecond( UINT32 uiGameMinutesPerSecond );
//Allows access to the current time rate.
UINT32 GetGameSecondsPerFrame(void);
void RenderPausedGameBox( void );


void StopTimeCompression( void );
void StartTimeCompression( void );
BOOLEAN IsTimeBeingCompressed( void );	// returns FALSE if time isn't currently being compressed for ANY reason (various pauses, etc.)
BOOLEAN IsTimeCompressionOn( void );		// returns TRUE if the player currently wants time to be compressing

//USING TIME COMPRESSION
//Allows the setting/changing/access of time rate via predefined compression values.
//These functions change the index in giTimeCompressSpeeds which aren't in any
//particular mathematical pattern.  The higher the index, the faster the time is processed
//per frame.  These functions have their limits, so game time will also be between
//TIME_COMPRESS_X1 to TIME_COMPRESS_X8 based in the laptop time compression.
void SetGameTimeCompressionLevel( UINT32 uiCompressionRate );
void DecreaseGameTimeCompressionRate(void);
void IncreaseGameTimeCompressionRate(void);

//time compression defines
enum
{
	NOT_USING_TIME_COMPRESSION = -1,
	TIME_COMPRESS_X0,
	TIME_COMPRESS_X1,
	TIME_COMPRESS_5MINS,
	TIME_COMPRESS_30MINS,
	TIME_COMPRESS_60MINS,
	TIME_SUPER_COMPRESS,
	NUM_TIME_COMPRESS_SPEEDS
};


//dereferenced with the above enumerations to provide the actual time compression rate.
extern INT32 giTimeCompressSpeeds[ NUM_TIME_COMPRESS_SPEEDS ];


#define STARTING_TIME		( ( 1 * NUM_SEC_IN_HOUR ) + ( 0 * NUM_SEC_IN_MIN ) + NUM_SEC_IN_DAY ) // 1am
#define FIRST_ARRIVAL_DELAY	( ( 6 * NUM_SEC_IN_HOUR ) + ( 0 * NUM_SEC_IN_MIN ) ) // 7am ( 6hours later)

#define WORLDTIMESTR		gswzWorldTimeStr


// compress mode now in use
extern INT32 giTimeCompressMode;


enum
{
	WARPTIME_NO_PROCESSING_OF_EVENTS,
	WARPTIME_PROCESS_EVENTS_NORMALLY,
	WARPTIME_PROCESS_TARGET_TIME_FIRST,
};
void WarpGameTime( UINT32 uiAdjustment, UINT8 ubWarpCode );


void AdvanceToNextDay(void);

//This function is called once per cycle in the game loop.  This determine how often the clock should be
//as well as how much to update the clock by.
void UpdateClock(void);


extern wchar_t gswzWorldTimeStr[ 20 ]; //Day 99, 23:55

extern	UINT32			guiDay;
extern	UINT32			guiHour;
extern	UINT32			guiMin;

//Advanced function used by certain event callbacks.  In the case where time is warped, certain event
//need to know how much time was warped since the last query to the event list.
//This function returns that value
extern UINT32 guiTimeOfLastEventQuery;

//This value represents the time that the sector was loaded.  If you are in sector A9, and leave
//the game clock at that moment will get saved into the temp file associated with it.  The next time you
//enter A9, this value will contain that time.  Used for scheduling purposes.
extern UINT32				guiTimeCurrentSectorWasLastLoaded;

// is the current pause state due to the player?
extern BOOLEAN gfPauseDueToPlayerGamePause;

// we've just clued up a pause by the player, the tactical screen will need a full one shot refresh to remove a 2 frame update problem
extern BOOLEAN gfJustFinishedAPause;

extern BOOLEAN gfResetAllPlayerKnowsEnemiesFlags;

extern UINT32 guiLockPauseStateLastReasonId;

UINT32 GetWorldTotalMin(void);
UINT32 GetWorldTotalSeconds(void);
UINT32 GetWorldHour(void);
UINT32 GetWorldDay(void);
UINT32 GetWorldMinutesInDay(void);
UINT32 GetWorldDayInSeconds(void);
UINT32 GetWorldDayInMinutes(void);
UINT32 GetFutureDayInMinutes( UINT32 uiDay );
UINT32 GetMidnightOfFutureDayInMinutes( UINT32 uiDay );

BOOLEAN DayTime(void);
BOOLEAN NightTime(void);

void InitNewGameClock(void);

void GotoNextTimeOfDay( UINT32 uiTOD );

void RenderClock(void);

//IMPORTANT FUNCTION:  Used whenever an event or situation is deemed important enough to cancel the
//further processing of time in this current time slice!  This can only be used inside of event callback
//functions -- otherwise, it'll be ignored and automatically reset.  An example of this would be when arriving
//in a new sector and being prompted to attack or retreat.
void InterruptTime(void);
void PauseTimeForInterupt(void);

extern BOOLEAN gfTimeInterrupt;

bool DidGameJustStart();

void SaveGameClock(HWFILE, BOOLEAN fGamePaused, BOOLEAN fLockPauseState);
void LoadGameClock(HWFILE);

// time compress flag stuff
BOOLEAN HasTimeCompressOccured( void );
void ResetTimeCompressHasOccured( void );
void SetFactTimeCompressHasOccured( void );


// create mouse region to pause game
void CreateMouseRegionForPauseOfClock(void);

// remove mouse region for pause game
void RemoveMouseRegionForPauseOfClock( void );

// handle pausing and unpausing of game
void HandlePlayerPauseUnPauseOfGame( void );

void ClearTacticalStuffDueToTimeCompression( void );

extern BOOLEAN gfGamePaused;
extern BOOLEAN gfLockPauseState;

#endif
