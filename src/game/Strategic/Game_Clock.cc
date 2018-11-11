#include "Font.h"
#include "Game_Clock.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "Timer_Control.h"
#include "Overhead.h"
#include "Environment.h"
#include "Message.h"
#include "Game_Events.h"
#include "MercTextBox.h"
#include "RenderWorld.h"
#include "Lighting.h"
#include "Map_Screen_Interface.h"
#include "Event_Pump.h"
#include "Text.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Screen_Interface_Bottom.h"
#include "MapScreen.h"
#include "GameScreen.h"
#include "Fade_Screen.h"
#include "Map_Information.h"
#include "Debug.h"
#include "JAScreens.h"
#include "Video.h"
#include "VSurface.h"
#include "Button_System.h"
#include "ScreenIDs.h"

#include <string_theory/format>
#include <string_theory/string>


//#define DEBUG_GAME_CLOCK

extern	BOOLEAN			gfFadeOut;


// is the clock pause region created currently?
static BOOLEAN fClockMouseRegionCreated = FALSE;

static BOOLEAN fTimeCompressHasOccured = FALSE;

//This value represents the time that the sector was loaded.  If you are in sector A9, and leave
//the game clock at that moment will get saved into the temp file associated with it.  The next time you
//enter A9, this value will contain that time.  Used for scheduling purposes.
UINT32 guiTimeCurrentSectorWasLastLoaded = 0;

// clock mouse region
static MOUSE_REGION gClockMouseRegion;
static MOUSE_REGION gClockScreenMaskMouseRegion;


#define SECONDS_PER_COMPRESSION 1 // 1/2 minute passes every 1 second of real time

#define CLOCK_X      (g_ui.get_CLOCK_X())
#define CLOCK_Y      (g_ui.get_CLOCK_Y())
#define CLOCK_HEIGHT  13
#define CLOCK_WIDTH   66
#define CLOCK_FONT   COMPFONT


//These contain all of the information about the game time, rate of time, etc.
//All of these get saved and loaded.
INT32          giTimeCompressMode = TIME_COMPRESS_X0;
static UINT8   gubClockResolution = 1;
BOOLEAN        gfGamePaused	= TRUE;
BOOLEAN        gfTimeInterrupt	= FALSE;
static BOOLEAN gfTimeInterruptPause = FALSE;
static BOOLEAN fSuperCompression = FALSE;
UINT32         guiGameClock = STARTING_TIME;
static UINT32  guiPreviousGameClock = 0; // used only for error-checking purposes
static UINT32  guiGameSecondsPerRealSecond;
static UINT32  guiTimesThisSecondProcessed = 0;
static         MercPopUpBox* g_paused_popup_box;
UINT32         guiDay;
UINT32         guiHour;
UINT32         guiMin;
ST::string     gswzWorldTimeStr;
INT32          giTimeCompressSpeeds[ NUM_TIME_COMPRESS_SPEEDS ] = { 0, 1, 5 * 60, 30 * 60, 60 * 60 };
static UINT16  usPausedActualWidth;
static UINT16  usPausedActualHeight;
UINT32         guiTimeOfLastEventQuery = 0;
BOOLEAN        gfLockPauseState = FALSE;
BOOLEAN        gfPauseDueToPlayerGamePause = FALSE;
BOOLEAN        gfResetAllPlayerKnowsEnemiesFlags = FALSE;
static BOOLEAN gfTimeCompressionOn = FALSE;
UINT32         guiLockPauseStateLastReasonId = 0;
//***When adding new saved time variables, make sure you remove the appropriate amount from the paddingbytes and
//   more IMPORTANTLY, add appropriate code in Save/LoadGameClock()!
#define TIME_PADDINGBYTES 20


extern UINT32 guiEnvTime;
extern UINT32 guiEnvDay;



void InitNewGameClock( )
{
	guiGameClock = STARTING_TIME;
	guiPreviousGameClock = STARTING_TIME;
	UpdateGameClockGlobals(pDayStrings);
	guiTimeCurrentSectorWasLastLoaded = 0;
	guiGameSecondsPerRealSecond = 0;
	gubClockResolution = 1;
}

UINT32 GetWorldTotalMin( )
{
	return( guiGameClock / NUM_SEC_IN_MIN );
}

UINT32 GetWorldTotalSeconds( )
{
	return( guiGameClock );
}


UINT32 GetWorldHour( )
{
	return( guiHour );
}

UINT32 GetWorldMinutesInDay( )
{
	return( ( guiHour * 60 ) + guiMin );
}

UINT32 GetWorldDay( )
{
	return( guiDay);
}

UINT32 GetWorldDayInSeconds( )
{
	return( guiDay * NUM_SEC_IN_DAY );
}

UINT32 GetWorldDayInMinutes( )
{
	return( ( guiDay * NUM_SEC_IN_DAY ) / NUM_SEC_IN_MIN );
}

UINT32 GetFutureDayInMinutes( UINT32 uiDay )
{
	return( ( uiDay * NUM_SEC_IN_DAY ) / NUM_SEC_IN_MIN );
}

//this function returns the amount of minutes there has been from start of game to midnight of the uiDay.
UINT32 GetMidnightOfFutureDayInMinutes( UINT32 uiDay )
{
	return( GetWorldTotalMin() + ( uiDay * 1440 ) - GetWorldMinutesInDay( ) );
}


static void AdvanceClock(UINT8 ubWarpCode);


// Not to be used too often by things other than internally
void WarpGameTime( UINT32 uiAdjustment, UINT8 ubWarpCode )
{
	UINT32 uiSaveTimeRate;
	uiSaveTimeRate = guiGameSecondsPerRealSecond;
	guiGameSecondsPerRealSecond = uiAdjustment;
	AdvanceClock( ubWarpCode );
	guiGameSecondsPerRealSecond = uiSaveTimeRate;
}


static void AdvanceClock(UINT8 ubWarpCode)
{
	if( ubWarpCode != WARPTIME_NO_PROCESSING_OF_EVENTS )
	{
		guiTimeOfLastEventQuery = guiGameClock;
		//First of all, events are posted for movements, pending attacks, equipment arrivals, etc.  This time
		//adjustment using time compression can possibly pass one or more events in a single pass.  So, this list
		//is looked at and processed in sequential order, until the uiAdjustment is fully applied.
		if( GameEventsPending( guiGameSecondsPerRealSecond ) )
		{
			//If a special event, justifying the cancellation of time compression is reached, the adjustment
			//will be shortened to the time of that event, and will stop processing events, otherwise, all
			//of the events in the time slice will be processed.  The time is adjusted internally as events
			//are processed.
			ProcessPendingGameEvents( guiGameSecondsPerRealSecond, ubWarpCode );
		}
		else
		{
			//Adjust the game clock now.
			guiGameClock += guiGameSecondsPerRealSecond;
		}
	}
	else
	{
		guiGameClock += guiGameSecondsPerRealSecond;
	}


	if ( guiGameClock < guiPreviousGameClock )
	{
		AssertMsg(FALSE, ST::format("AdvanceClock: TIME FLOWING BACKWARDS!!! guiPreviousGameClock {}, now {}", guiPreviousGameClock, guiGameClock));

		// fix it if assertions are disabled
		guiGameClock = guiPreviousGameClock;
	}

	// store previous game clock value (for error-checking purposes only)
	guiPreviousGameClock = guiGameClock;

	UpdateGameClockGlobals(gpGameClockString);

	if( gfResetAllPlayerKnowsEnemiesFlags && !gTacticalStatus.fEnemyInSector )
	{
		ClearAnySectorsFlashingNumberOfEnemies();

		gfResetAllPlayerKnowsEnemiesFlags = FALSE;
	}

	ForecastDayEvents( );
}


// set the flag that time compress has occured
void SetFactTimeCompressHasOccured( void )
{
	fTimeCompressHasOccured = TRUE;
}

//reset fact the time compress has occured
void ResetTimeCompressHasOccured( void )
{
	fTimeCompressHasOccured = FALSE;
}

// has time compress occured?
BOOLEAN HasTimeCompressOccured( void )
{
	return( fTimeCompressHasOccured  );
}


void RenderClock(void)
{
	// Are we in combat?
	UINT8 const foreground = gTacticalStatus.uiFlags & INCOMBAT ?
		FONT_FCOLOR_NICERED : FONT_LTGREEN;
	SetFontAttributes(CLOCK_FONT, foreground);

	// Erase first!
	INT16 x = CLOCK_X;
	INT16 y = CLOCK_Y;
	RestoreExternBackgroundRect(x, y, CLOCK_WIDTH, CLOCK_HEIGHT);

	ST::string const& str = gfPauseDueToPlayerGamePause ? pPausedGameText[0] : WORLDTIMESTR;
	FindFontCenterCoordinates(x, y, CLOCK_WIDTH, CLOCK_HEIGHT, str, CLOCK_FONT, &x, &y);
	MPrint(x, y, str);
}

bool DidGameJustStart()
{
	return gTacticalStatus.fDidGameJustStart;
}


static void SetClockResolutionToCompressMode(INT32 iCompressMode);


void StopTimeCompression( void )
{
	if ( gfTimeCompressionOn )
	{
		// change the clock resolution to no time passage, but don't actually change the compress mode (remember it)
		SetClockResolutionToCompressMode( TIME_COMPRESS_X0 );
	}
}


void StartTimeCompression( void )
{
	if ( !gfTimeCompressionOn )
	{
		if ( GamePaused() )
		{
			// first have to be allowed to unpause the game
			UnPauseGame( );

			// if we couldn't, ignore this request
			if ( GamePaused() )
			{
				return;
			}
		}


		// check that we can start compressing
		if ( !AllowedToTimeCompress( ) )
		{
			// not allowed to compress time
			TellPlayerWhyHeCantCompressTime();
			return;
		}


		// if no compression mode is set, increase it first
		if ( giTimeCompressMode <= TIME_COMPRESS_X1 )
		{
			IncreaseGameTimeCompressionRate();
		}

		// change clock resolution to the current compression mode
		SetClockResolutionToCompressMode( giTimeCompressMode );

		// if it's the first time we're doing this since entering map screen (which reset the flag)
		if ( !HasTimeCompressOccured( ) )
		{
			// set fact that we have compressed time during this map screen session
			SetFactTimeCompressHasOccured( );

			ClearTacticalStuffDueToTimeCompression( );
		}
	}
}


// returns FALSE if time isn't currently being compressed for ANY reason (various pauses, etc.)
BOOLEAN IsTimeBeingCompressed( void )
{
	if( !gfTimeCompressionOn || ( giTimeCompressMode == TIME_COMPRESS_X0 ) || gfGamePaused )
		return( FALSE );
	else
		return ( TRUE );
}


// returns TRUE if the player currently doesn't want time to be compressing
BOOLEAN IsTimeCompressionOn( void )
{
	return( gfTimeCompressionOn );
}


void IncreaseGameTimeCompressionRate( )
{
	// if not already at maximum time compression rate
	if( giTimeCompressMode < TIME_COMPRESS_60MINS )
	{
		// check that we can
		if ( !AllowedToTimeCompress( ) )
		{
			// not allowed to compress time
			TellPlayerWhyHeCantCompressTime();
			return;
		}


		giTimeCompressMode++;

		// in map screen, we wanna have to skip over x1 compression and go straight to 5x
		if( ( guiCurrentScreen == MAP_SCREEN ) && ( giTimeCompressMode == TIME_COMPRESS_X1 ) )
		{
			giTimeCompressMode++;
		}

		SetClockResolutionToCompressMode( giTimeCompressMode );
	}
}


void DecreaseGameTimeCompressionRate()
{
	// if not already at minimum time compression rate
	if( giTimeCompressMode > TIME_COMPRESS_X0 )
	{
		// check that we can
		if ( !AllowedToTimeCompress( ) )
		{
			// not allowed to compress time
			TellPlayerWhyHeCantCompressTime();
			return;
		}

		giTimeCompressMode--;

		// in map screen, we wanna have to skip over x1 compression and go straight to 5x
		if( ( guiCurrentScreen == MAP_SCREEN ) && ( giTimeCompressMode == TIME_COMPRESS_X1 ) )
		{
			giTimeCompressMode--;
		}

		SetClockResolutionToCompressMode( giTimeCompressMode );
	}
}


void SetGameTimeCompressionLevel( UINT32 uiCompressionRate )
{
	Assert( uiCompressionRate < NUM_TIME_COMPRESS_SPEEDS );

	if( guiCurrentScreen == GAME_SCREEN )
	{
		if( uiCompressionRate != TIME_COMPRESS_X1 )
		{
			uiCompressionRate = TIME_COMPRESS_X1;
		}
	}

	if( guiCurrentScreen == MAP_SCREEN )
	{
		if( uiCompressionRate == TIME_COMPRESS_X1 )
		{
			uiCompressionRate = TIME_COMPRESS_X0;
		}
	}

	// if we're attempting time compression
	if ( uiCompressionRate >= TIME_COMPRESS_5MINS )
	{
		// check that we can
		if ( !AllowedToTimeCompress( ) )
		{
			// not allowed to compress time
			TellPlayerWhyHeCantCompressTime();
			return;
		}
	}

	giTimeCompressMode = uiCompressionRate;
	SetClockResolutionToCompressMode( giTimeCompressMode );
}


static void SetClockResolutionPerSecond(UINT8 ubNumTimesPerSecond);


static void SetClockResolutionToCompressMode(INT32 iCompressMode)
{
	guiGameSecondsPerRealSecond = giTimeCompressSpeeds[ iCompressMode ] * SECONDS_PER_COMPRESSION;

	// ok this is a bit confusing, but for time compression (e.g. 30x60) we want updates
	// 30x per second, but for standard unpaused time, like in tactical, we want 1x per second
	if ( guiGameSecondsPerRealSecond == 0 )
	{
		SetClockResolutionPerSecond( 0 );
	}
	else
	{
		SetClockResolutionPerSecond((UINT8) std::max(1U, guiGameSecondsPerRealSecond / 60));
	}

	// if the compress mode is X0 or X1
	if ( iCompressMode <= TIME_COMPRESS_X1 )
	{
		gfTimeCompressionOn = FALSE;
	}
	else
	{
		gfTimeCompressionOn = TRUE;

		// handle the player just starting a game
		HandleTimeCompressWithTeamJackedInAndGearedToGo( );
	}

	fMapScreenBottomDirty = TRUE;
}



void SetGameHoursPerSecond( UINT32 uiGameHoursPerSecond )
{
	giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
	guiGameSecondsPerRealSecond = uiGameHoursPerSecond * 3600;
	if( uiGameHoursPerSecond == 1 )
	{
		SetClockResolutionPerSecond( 60 );
	}
	else
	{
		SetClockResolutionPerSecond( 59 );
	}
}

void SetGameMinutesPerSecond( UINT32 uiGameMinutesPerSecond )
{
	giTimeCompressMode = NOT_USING_TIME_COMPRESSION;
	guiGameSecondsPerRealSecond = uiGameMinutesPerSecond * 60;
	SetClockResolutionPerSecond( (UINT8)uiGameMinutesPerSecond );
}


// call this to prevent player from changing the time compression state via the interface

void LockPauseState(LockPauseReason const uiUniqueReasonId)
{
	gfLockPauseState = TRUE;

	// if adding a new call, please choose a new uiUniqueReasonId, this helps track down the cause when it's left locked
	// Highest # used was 21 on Feb 15 '99.
	guiLockPauseStateLastReasonId = uiUniqueReasonId;
}

// call this to allow player to change the time compression state via the interface once again
void UnLockPauseState()
{
	gfLockPauseState = FALSE;
}

// tells you whether the player is currently locked out from messing with the time compression state
BOOLEAN PauseStateLocked()
{
	return gfLockPauseState;
}


void PauseGame(void)
{
	// always allow pausing, even if "locked".  Locking applies only to trying to compress time, not to pausing it
	if( !gfGamePaused )
	{
		gfGamePaused = TRUE;
		fMapScreenBottomDirty = TRUE;
	}
}


void UnPauseGame(void)
{
	// if we're paused
	if( gfGamePaused )
	{
		// ignore request if locked
		if ( gfLockPauseState )
		{
			SLOGW("Call to UnPauseGame() while Pause State is LOCKED!");
			return;
		}

		gfGamePaused = FALSE;
		fMapScreenBottomDirty = TRUE;
	}
}


BOOLEAN GamePaused()
{
	return gfGamePaused;
}


//ONLY APPLICABLE INSIDE EVENT CALLBACKS!
void InterruptTime()
{
	gfTimeInterrupt = TRUE;
}

void PauseTimeForInterupt()
{
	gfTimeInterruptPause = TRUE;
}


//USING CLOCK RESOLUTION
//Note, that changing the clock resolution doesn't effect the amount of game time that passes per
//real second, but how many times per second the clock is updated.  This rate will break up the actual
//time slices per second into smaller chunks.  This is useful for animating strategic movement under
//fast time compression, so objects don't warp around.
//Valid range is 0 - 60 times per second.
static void SetClockResolutionPerSecond(UINT8 ubNumTimesPerSecond)
{
	ubNumTimesPerSecond = (UINT8) std::clamp(int(ubNumTimesPerSecond), 0, 60);
	gubClockResolution = ubNumTimesPerSecond;
}


static void CreateDestroyScreenMaskForPauseGame(void);


//There are two factors that influence the flow of time in the game.
//-Speed:  The speed is the amount of game time passes per real second of time.  The higher this
//         value, the faster the game time flows.
//-Resolution:  The higher the resolution, the more often per second the clock is actually updated.
//              This value doesn't affect how much game time passes per real second, but allows for
//              a more accurate representation of faster time flows.
void UpdateClock()
{
	UINT32 uiNewTime;
	UINT32 uiThousandthsOfThisSecondProcessed;
	UINT32 uiTimeSlice;
	UINT32 uiNewTimeProcessed;
	static UINT8 ubLastResolution = 1;
	static UINT32 uiLastSecondTime = 0;
	static UINT32 uiLastTimeProcessed = 0;
#ifdef DEBUG_GAME_CLOCK
	UINT32 uiAmountToAdvanceTime;
	UINT32 uiOrigNewTime;
	UINT32 uiOrigLastSecondTime;
	UINT32 uiOrigThousandthsOfThisSecondProcessed;
	UINT8 ubOrigClockResolution;
	UINT32 uiOrigTimesThisSecondProcessed;
	UINT8 ubOrigLastResolution;
#endif
	// check game state for pause screen masks
	CreateDestroyScreenMaskForPauseGame( );

	if( guiCurrentScreen != GAME_SCREEN && guiCurrentScreen != MAP_SCREEN && guiCurrentScreen != GAME_SCREEN )
	{
		uiLastSecondTime = GetJA2Clock( );
		gfTimeInterruptPause = FALSE;
		return;
	}

	if( gfGamePaused || gfTimeInterruptPause || ( gubClockResolution == 0 ) || !guiGameSecondsPerRealSecond || ARE_IN_FADE_IN( ) || gfFadeOut )
	{
		uiLastSecondTime = GetJA2Clock( );
		gfTimeInterruptPause = FALSE;
		return;
	}

	if(gTacticalStatus.uiFlags & INCOMBAT)
		return; //time is currently stopped!


	uiNewTime = GetJA2Clock();

#ifdef DEBUG_GAME_CLOCK
	uiOrigNewTime = uiNewTime;
	uiOrigLastSecondTime = uiLastSecondTime;
	uiOrigThousandthsOfThisSecondProcessed = uiThousandthsOfThisSecondProcessed;
	ubOrigClockResolution = gubClockResolution;
	uiOrigTimesThisSecondProcessed = guiTimesThisSecondProcessed;
	ubOrigLastResolution = ubLastResolution;
#endif

	//Because we debug so much, breakpoints tend to break the game, and cause unnecessary headaches.
	//This line ensures that no more than 1 real-second passes between frames.  This otherwise has
	//no effect on anything else.
	uiLastSecondTime = std::max(uiNewTime - 1000, uiLastSecondTime);

	//1000's of a second difference since last second.
	uiThousandthsOfThisSecondProcessed = uiNewTime - uiLastSecondTime;

	if( uiThousandthsOfThisSecondProcessed >= 1000 && gubClockResolution == 1 )
	{
		uiLastSecondTime = uiNewTime;
		guiTimesThisSecondProcessed = uiLastTimeProcessed = 0;
		AdvanceClock( WARPTIME_PROCESS_EVENTS_NORMALLY );
	}
	else if( gubClockResolution > 1 )
	{
		if( gubClockResolution != ubLastResolution )
		{
			//guiTimesThisSecondProcessed = guiTimesThisSecondProcessed * ubLastResolution / gubClockResolution % gubClockResolution;
			guiTimesThisSecondProcessed = guiTimesThisSecondProcessed * gubClockResolution / ubLastResolution;
			uiLastTimeProcessed = uiLastTimeProcessed * gubClockResolution / ubLastResolution;
			ubLastResolution = gubClockResolution;
		}
		uiTimeSlice = 1000000/gubClockResolution;
		if( uiThousandthsOfThisSecondProcessed >= uiTimeSlice * (guiTimesThisSecondProcessed+1)/1000 )
		{
			guiTimesThisSecondProcessed = uiThousandthsOfThisSecondProcessed*1000 / uiTimeSlice;
			uiNewTimeProcessed = guiGameSecondsPerRealSecond * guiTimesThisSecondProcessed / gubClockResolution;

			uiNewTimeProcessed = std::max(uiNewTimeProcessed, uiLastTimeProcessed);

			#ifdef DEBUG_GAME_CLOCK
			uiAmountToAdvanceTime = uiNewTimeProcessed - uiLastTimeProcessed;

				if( uiAmountToAdvanceTime > 0x80000000 || guiGameClock + uiAmountToAdvanceTime < guiPreviousGameClock )
				{
					uiNewTimeProcessed = uiNewTimeProcessed;
				}
			#endif

			WarpGameTime( uiNewTimeProcessed - uiLastTimeProcessed, WARPTIME_PROCESS_EVENTS_NORMALLY );
			if( uiNewTimeProcessed < guiGameSecondsPerRealSecond )
			{ //Processed the same real second
				uiLastTimeProcessed =  uiNewTimeProcessed;
			}
			else
			{ //We have moved into a new real second.
				uiLastTimeProcessed = uiNewTimeProcessed % guiGameSecondsPerRealSecond;
				if ( gubClockResolution > 0 )
				{
					guiTimesThisSecondProcessed %= gubClockResolution;
				}
				else
				{
					// this branch occurs whenever an event during WarpGameTime stops time compression!
					guiTimesThisSecondProcessed = 0;
				}
				uiLastSecondTime = uiNewTime;
			}
		}
	}
}


void SaveGameClock(HWFILE const hFile, BOOLEAN const fGamePaused, BOOLEAN const fLockPauseState)
{
	hFile->write(&giTimeCompressMode,                sizeof(INT32));
	hFile->write(&gubClockResolution,                sizeof(UINT8));
	hFile->write(&fGamePaused,                       sizeof(BOOLEAN));
	hFile->write(&gfTimeInterrupt,                   sizeof(BOOLEAN));
	hFile->write(&fSuperCompression,                 sizeof(BOOLEAN));
	hFile->write(&guiGameClock,                      sizeof(UINT32));
	hFile->write(&guiGameSecondsPerRealSecond,       sizeof(UINT32));
	hFile->write(&ubAmbientLightLevel,               sizeof(UINT8));
	hFile->write(&guiEnvTime,                        sizeof(UINT32));
	hFile->write(&guiEnvDay,                         sizeof(UINT32));
	hFile->write(&gubEnvLightValue,                  sizeof(UINT8));
	hFile->write(&guiTimeOfLastEventQuery,           sizeof(UINT32));
	hFile->write(&fLockPauseState,                   sizeof(BOOLEAN));
	hFile->write(&gfPauseDueToPlayerGamePause,       sizeof(BOOLEAN));
	hFile->write(&gfResetAllPlayerKnowsEnemiesFlags, sizeof(BOOLEAN));
	hFile->write(&gfTimeCompressionOn,               sizeof(BOOLEAN));
	hFile->write(&guiPreviousGameClock,              sizeof(UINT32));
	hFile->write(&guiLockPauseStateLastReasonId,     sizeof(UINT32));

	hFile->seek(TIME_PADDINGBYTES, FILE_SEEK_FROM_CURRENT);
}


void LoadGameClock(HWFILE const hFile)
{
	hFile->read(&giTimeCompressMode,                sizeof(INT32));
	hFile->read(&gubClockResolution,                sizeof(UINT8));
	hFile->read(&gfGamePaused,                      sizeof(BOOLEAN));
	hFile->read(&gfTimeInterrupt,                   sizeof(BOOLEAN));
	hFile->read(&fSuperCompression,                 sizeof(BOOLEAN));
	hFile->read(&guiGameClock,                      sizeof(UINT32));
	hFile->read(&guiGameSecondsPerRealSecond,       sizeof(UINT32));
	hFile->read(&ubAmbientLightLevel,               sizeof(UINT8));
	hFile->read(&guiEnvTime,                        sizeof(UINT32));
	hFile->read(&guiEnvDay,                         sizeof(UINT32));
	hFile->read(&gubEnvLightValue,                  sizeof(UINT8));
	hFile->read(&guiTimeOfLastEventQuery,           sizeof(UINT32));
	hFile->read(&gfLockPauseState,                  sizeof(BOOLEAN));
	hFile->read(&gfPauseDueToPlayerGamePause,       sizeof(BOOLEAN));
	hFile->read(&gfResetAllPlayerKnowsEnemiesFlags, sizeof(BOOLEAN));
	hFile->read(&gfTimeCompressionOn,               sizeof(BOOLEAN));
	hFile->read(&guiPreviousGameClock,              sizeof(UINT32));
	hFile->read(&guiLockPauseStateLastReasonId,     sizeof(UINT32));

	hFile->seek(TIME_PADDINGBYTES, FILE_SEEK_FROM_CURRENT);

	UpdateGameClockGlobals(pDayStrings);

	if( !gfBasement && !gfCaves )
		gfDoLighting = TRUE;
}


static void PauseOfClockBtnCallback(MOUSE_REGION* pRegion, UINT32 iReason);


void CreateMouseRegionForPauseOfClock(void)
{
	if (!fClockMouseRegionCreated)
	{
		// create a mouse region for pausing of game clock
		MSYS_DefineRegion(
			&gClockMouseRegion,
			CLOCK_X, CLOCK_Y, CLOCK_X + CLOCK_WIDTH, CLOCK_Y + CLOCK_HEIGHT,
			MSYS_PRIORITY_HIGHEST, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, PauseOfClockBtnCallback);

		fClockMouseRegionCreated = TRUE;

		ST::string const& help = pPausedGameText[gfGamePaused ? 1 : 2];
		gClockMouseRegion.SetFastHelpText(help);
	}
}


void RemoveMouseRegionForPauseOfClock( void )
{
	// remove pause region
	if (fClockMouseRegionCreated)
	{
		MSYS_RemoveRegion( &gClockMouseRegion );
		fClockMouseRegionCreated = FALSE;

	}
}


static void PauseOfClockBtnCallback(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		HandlePlayerPauseUnPauseOfGame(  );
	}
}


void HandlePlayerPauseUnPauseOfGame( void )
{
	if ( gTacticalStatus.uiFlags & ENGAGED_IN_CONV )
	{
		return;
	}

	// check if the game is paused BY THE PLAYER or not and reverse
	if( gfGamePaused && gfPauseDueToPlayerGamePause )
	{
		// If in game screen...
		if ( guiCurrentScreen == GAME_SCREEN )
		{
			if( giTimeCompressMode == TIME_COMPRESS_X0 )
			{
				giTimeCompressMode++;
			}

			// ATE: re-render
			SetRenderFlags( RENDER_FLAG_FULL );
		}

		UnPauseGame( );
		PauseTime( FALSE );
		gfIgnoreScrolling = FALSE;
		gfPauseDueToPlayerGamePause = FALSE;
	}
	else
	{
		// pause game
		PauseGame( );
		PauseTime( TRUE );
		gfIgnoreScrolling = TRUE;
		gfPauseDueToPlayerGamePause = TRUE;
	}
}


static void ScreenMaskForGamePauseBtnCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateDestroyScreenMaskForPauseGame(void)
{
	static BOOLEAN fCreated = FALSE;

	if ((!fClockMouseRegionCreated || !gfGamePaused || !gfPauseDueToPlayerGamePause) && fCreated)
	{
		fCreated = FALSE;
		MSYS_RemoveRegion( &gClockScreenMaskMouseRegion );
		RemoveMercPopupBox(g_paused_popup_box);
		g_paused_popup_box = 0;
		SetRenderFlags( RENDER_FLAG_FULL );
		fTeamPanelDirty = TRUE;
		fMapPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
		MarkButtonsDirty();
		SetRenderFlags( RENDER_FLAG_FULL );
	}
	else if (gfPauseDueToPlayerGamePause && !fCreated)
	{
		// create a mouse region for pausing of game clock
		MSYS_DefineRegion(&gClockScreenMaskMouseRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, 0, MSYS_NO_CALLBACK, ScreenMaskForGamePauseBtnCallBack);
		fCreated = TRUE;

		//re create region on top of this
		RemoveMouseRegionForPauseOfClock( );
		CreateMouseRegionForPauseOfClock();

		gClockMouseRegion.SetFastHelpText(pPausedGameText[1]);

		fMapScreenBottomDirty = TRUE;

		//UnMarkButtonsDirty( );

		// now create the pop up box to say the game is paused
		g_paused_popup_box = PrepareMercPopupBox(0, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, pPausedGameText[0], 300, 0, 0, 0, &usPausedActualWidth, &usPausedActualHeight);
	}
}


static void ScreenMaskForGamePauseBtnCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// unpause the game
		HandlePlayerPauseUnPauseOfGame( );
	}
}

void RenderPausedGameBox( void )
{
	if (gfPauseDueToPlayerGamePause && gfGamePaused && g_paused_popup_box)
	{
		const INT32 x = (SCREEN_WIDTH - usPausedActualWidth)  / 2;
		const INT32 y = 200 - usPausedActualHeight / 2;
		RenderMercPopUpBox(g_paused_popup_box, x, y, FRAME_BUFFER);
		InvalidateRegion(x, y, x + usPausedActualWidth, y + usPausedActualHeight);
	}
}

BOOLEAN DayTime()
{ //between 7AM and 9PM
	return ( guiHour >= 7 && guiHour < 21 );
}

BOOLEAN NightTime()
{  //before 7AM or after 9PM
	return ( guiHour < 7 || guiHour >= 21 );
}


void ClearTacticalStuffDueToTimeCompression( void )
{
	// is this test the right thing?  ARM
	if (!fInMapMode) return; // XXX necessary?

	// clear tactical event queue
	ClearEventQueue( );

	// clear tactical message queue
	ClearTacticalMessageQueue( );

	if( gfWorldLoaded )
	{
		// clear tactical actions
		CencelAllActionsForTimeCompression( );
	}
}


void UpdateGameClockGlobals(ST::string const& dayStringToUse)
{
	//Calculate the day, hour, and minutes.
	guiDay = guiGameClock / NUM_SEC_IN_DAY;
	guiHour = (guiGameClock - guiDay * NUM_SEC_IN_DAY) / NUM_SEC_IN_HOUR;
	guiMin = (guiGameClock - (guiDay * NUM_SEC_IN_DAY +  guiHour * NUM_SEC_IN_HOUR)) / NUM_SEC_IN_MIN;

	WORLDTIMESTR = ST::format("{} {}, {02d}:{02d}", dayStringToUse, guiDay, guiHour, guiMin);
}
