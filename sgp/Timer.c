#ifdef JA2_PRECOMPILED_HEADERS
	#include "JA2 SGP ALL.H"
#elif defined( WIZ8_PRECOMPILED_HEADERS )
	#include "WIZ8 SGP ALL.H"
#else
	#include "Types.h"
	#include "Stubs.h"
	#if defined( JA2 ) || defined( UTIL )
		#include "Video.h"
	#else
		#include "video2.h"
	#endif
	#include "Timer.h"
	#include <SDL.h>
#endif


UINT32 guiStartupTime;
UINT32 guiCurrentTime;

static SDL_TimerID clock_id;

void CALLBACK Clock( HWND hWindow, UINT uMessage, UINT idEvent, DWORD dwTime )
{
  guiCurrentTime = SDL_GetTicks();
  if (guiCurrentTime < guiStartupTime)
  { // Adjust guiCurrentTime because of loopback on the timer value
    guiCurrentTime = guiCurrentTime + (0xffffffff - guiStartupTime);
  }
  else
  { // Adjust guiCurrentTime because of loopback on the timer value
    guiCurrentTime = guiCurrentTime - guiStartupTime;
  }
}

BOOLEAN InitializeClockManager(void)
{

  // Register the start time (use WIN95 API call)
  guiCurrentTime = guiStartupTime = SDL_GetTicks();
  /* SetTimer(ghWindow, MAIN_TIMER_ID, 10, (TIMERPROC)Clock); */
  clock_id = SDL_AddTimer(10,(SDL_NewTimerCallback)Clock,NULL);


  return TRUE;
}

void    ShutdownClockManager(void)
{

  // Make sure we kill the timer
  /* KillTimer(ghWindow, MAIN_TIMER_ID); */
  SDL_RemoveTimer(clock_id);

}

TIMER   GetClock(void)
{
  return guiCurrentTime;
}

TIMER   SetCountdownClock(UINT32 uiTimeToElapse)
{
  return (guiCurrentTime + uiTimeToElapse);
}

UINT32 ClockIsTicking(TIMER uiTimer)
{
  if (uiTimer > guiCurrentTime)
  { // Well timer still hasn't elapsed
    return (uiTimer - guiCurrentTime);
  }
  // Time's up
  return 0;
}
