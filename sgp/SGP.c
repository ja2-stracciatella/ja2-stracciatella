#include "BuildDefines.h"
#include "Button_System.h"
#include "Container.h"
#include "Debug.h"
#include "FileMan.h"
#include "Font.h"
#include "Gameloop.h"
#include "Input.h"
#include "Intro.h"
#include "JA2_Splash.h"
#include "Local.h"
#include "MemMan.h"
#include "Random.h"
#include "SGP.h"
#include "Stubs.h"
#include "SoundMan.h"
#include "Utilities.h"
#include "Video.h"
#include "VSurface.h"
#include <SDL.h>
#include <stdio.h>
#include <string.h>


#ifdef JA2
extern BOOLEAN gfPauseDueToPlayerGamePause;
#endif


static BOOLEAN gfApplicationActive;
BOOLEAN gfProgramIsRunning;
static BOOLEAN gfGameInitialized = FALSE;

CHAR8		gzCommandLine[100];		// Command line given

static char    gzErrorMsg[2048] = "";
static BOOLEAN gfIgnoreMessages = FALSE;

#if 0 // XXX TODO
INT32 FAR PASCAL WindowProcedure(HWND hWindow, UINT16 Message, WPARAM wParam, LPARAM lParam)
{
	static BOOLEAN fRestore = FALSE;

  if(gfIgnoreMessages)
		return(DefWindowProc(hWindow, Message, wParam, lParam));

	switch(Message)
  {
		case WM_MOUSEWHEEL:
			{
				QueueEvent(MOUSE_WHEEL, wParam);
				break;
			}

    case WM_ACTIVATEAPP:
      switch(wParam)
      {
        case TRUE: // We are restarting DirectDraw
          if (fRestore == TRUE)
          {
#ifdef JA2
	          RestoreVideoManager();

						// unpause the JA2 Global clock
            if ( !gfPauseDueToPlayerGamePause )
            {
						  PauseTime( FALSE );
            }
#endif
            gfApplicationActive = TRUE;
          }
          break;
        case FALSE: // We are suspending direct draw
#ifdef JA2
						// pause the JA2 Global clock
						PauseTime( TRUE );
						SuspendVideoManager();
#endif

          gfApplicationActive = FALSE;
          fRestore = TRUE;
          break;
      }
      break;

    case WM_CREATE:
			break;

    case WM_DESTROY:
			ShutdownStandardGamingPlatform();
      ShowCursor(TRUE);
      PostQuitMessage(0);
      break;

		case WM_SETFOCUS:
      RestoreCursorClipRect( );
			break;

		case WM_KILLFOCUS:
			// Set a flag to restore surfaces once a WM_ACTIVEATEAPP is received
			fRestore = TRUE;
			break;

    default
    : return DefWindowProc(hWindow, Message, wParam, lParam);
  }
  return 0L;
}
#endif


static void SGPExit(void);


static BOOLEAN InitializeStandardGamingPlatform(void)
{
	BOOLEAN video_fullscreen = FALSE;

	if (strcasecmp(gzCommandLine,"-FULLSCREEN")==0)
		video_fullscreen = TRUE;

	// now required by all (even JA2) in order to call ShutdownSGP
	atexit(SGPExit);

	SDL_Init(SDL_INIT_VIDEO);

	// Initialize the Debug Manager - success doesn't matter
	InitializeDebugManager();

	// Now start up everything else.
	RegisterDebugTopic(TOPIC_SGP, "Standard Gaming Platform");

  // this one needs to go ahead of all others (except Debug), for MemDebugCounter to work right...
	FastDebugMsg("Initializing Memory Manager");
	// Initialize the Memory Manager
	if (InitializeMemoryManager() == FALSE)
	{ // We were unable to initialize the memory manager
		FastDebugMsg("FAILED : Initializing Memory Manager");
		return FALSE;
	}

	FastDebugMsg("Initializing File Manager");
	// Initialize the File Manager
	if (InitializeFileManager(NULL) == FALSE)
	{ // We were unable to initialize the file manager
		FastDebugMsg("FAILED : Initializing File Manager");
		return FALSE;
	}

	FastDebugMsg("Initializing Containers Manager");
  InitializeContainers();

	FastDebugMsg("Initializing Input Manager");
	// Initialize the Input Manager
	if (InitializeInputManager() == FALSE)
	{ // We were unable to initialize the input manager
		FastDebugMsg("FAILED : Initializing Input Manager");
		return FALSE;
	}

	FastDebugMsg("Initializing Video Manager");
	// Initialize DirectDraw (DirectX 2)
	if (!InitializeVideoManager(video_fullscreen))
	{ // We were unable to initialize the video manager
		FastDebugMsg("FAILED : Initializing Video Manager");
		return FALSE;
	}

	// Initialize Video Object Manager
	FastDebugMsg("Initializing Video Object Manager");
	if ( !InitializeVideoObjectManager( ) )
	{
		FastDebugMsg("FAILED : Initializing Video Object Manager");
		return FALSE;
	}

	// Initialize Video Surface Manager
	FastDebugMsg("Initializing Video Surface Manager");
	if ( !InitializeVideoSurfaceManager( ) )
	{
		FastDebugMsg("FAILED : Initializing Video Surface Manager");
		return FALSE;
	}

	#ifdef JA2
		InitJA2SplashScreen();
	#endif

	// Initialize Font Manager
	FastDebugMsg("Initializing the Font Manager");
	// Init the manager and copy the TransTable stuff into it.
	if (!InitializeFontManager())
	{
		FastDebugMsg("FAILED : Initializing Font Manager");
		return FALSE;
	}

	FastDebugMsg("Initializing Sound Manager");
	// Initialize the Sound Manager (DirectSound)
#ifndef UTIL
	if (InitializeSoundManager() == FALSE)
	{ // We were unable to initialize the sound manager
		FastDebugMsg("FAILED : Initializing Sound Manager");
		return FALSE;
	}
#endif

	FastDebugMsg("Initializing Random");
  // Initialize random number generator
  InitializeRandom(); // no Shutdown

	FastDebugMsg("Initializing Game Manager");
	// Initialize the Game
	if (InitializeGame() == FALSE)
	{ // We were unable to initialize the game
		FastDebugMsg("FAILED : Initializing Game Manager");
		return FALSE;
	}

	gfGameInitialized = TRUE;

	return TRUE;
}


static void ShutdownStandardGamingPlatform(void)
{
	// Shut down the different components of the SGP

	// TEST
	SoundServiceStreams();

	if (gfGameInitialized)
	{
		ShutdownGame();
	}

	ShutdownButtonSystem();
	MSYS_Shutdown();

#ifndef UTIL
  ShutdownSoundManager();
#endif

	DestroyEnglishTransTable( );    // has to go before ShutdownFontManager()
  ShutdownFontManager();

#ifdef SGP_VIDEO_DEBUGGING
	PerformVideoInfoDumpIntoFile( "SGPVideoShutdownDump.txt", FALSE );
#endif

	ShutdownVideoSurfaceManager();
  ShutdownVideoObjectManager();
  ShutdownVideoManager();

  ShutdownInputManager();
  ShutdownContainers();
  ShutdownFileManager();

#ifdef EXTREME_MEMORY_DEBUGGING
	DumpMemoryInfoIntoFile( "ExtremeMemoryDump.txt", FALSE );
#endif

  ShutdownMemoryManager();  // must go last (except for Debug), for MemDebugCounter to work right...

	//
  // Make sure we unregister the last remaining debug topic before shutting
  // down the debugging layer
  UnRegisterDebugTopic(TOPIC_SGP, "Standard Gaming Platform");

  ShutdownDebugManager();
}


static void ProcessJa2CommandLineBeforeInitialization(const char* pCommandLine);


int main(int argc, char* argv[])
{
#if 1 // XXX TODO
	FIXME
	if (argc > 1 && argv[1] != NULL) strlcpy(gzCommandLine, argv[1], lengthof(gzCommandLine));
#else
	strlcpy(gzCommandLine, pCommandLine, lengthof(gzCommandLine));

	//Process the command line BEFORE initialization
	ProcessJa2CommandLineBeforeInitialization( pCommandLine );
#endif

  if (!InitializeStandardGamingPlatform())
  {
    return 0;
  }

#ifdef JA2
	#ifdef ENGLISH
		SetIntroType( INTRO_SPLASH );
	#endif
#endif

  gfApplicationActive = TRUE;
  gfProgramIsRunning = TRUE;

  FastDebugMsg("Running Game");

  // At this point the SGP is set up, which means all I/O, Memory, tools, etc... are available. All we need to do is
  // attend to the gaming mechanics themselves
  while (gfProgramIsRunning)
  {
		SDL_Event event;

		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_ACTIVEEVENT:
					if (event.active.state & SDL_APPACTIVE)
					{
						gfApplicationActive = (event.active.gain != 0);
						break;
					}
					break;

				case SDL_KEYDOWN: KeyDown(&event.key.keysym); break;
				case SDL_KEYUP:   KeyUp(  &event.key.keysym); break;

				case SDL_MOUSEBUTTONDOWN:
					gusMouseXPos = event.button.x;
					gusMouseYPos = event.button.y;
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							gfLeftButtonState = TRUE;
							QueueEvent(LEFT_BUTTON_DOWN, 0);
							break;

						case SDL_BUTTON_RIGHT:
							gfRightButtonState = TRUE;
							QueueEvent(RIGHT_BUTTON_DOWN, 0);
							break;
					}
					break;

				case SDL_MOUSEBUTTONUP:
					gusMouseXPos = event.button.x;
					gusMouseYPos = event.button.y;
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							gfLeftButtonState = FALSE;
							QueueEvent(LEFT_BUTTON_UP, 0);
							break;

						case SDL_BUTTON_RIGHT:
							gfRightButtonState = FALSE;
							QueueEvent(RIGHT_BUTTON_UP, 0);
							break;
					}
					break;

				case SDL_MOUSEMOTION:
					gusMouseXPos = event.motion.x;
					gusMouseYPos = event.motion.y;
					break;

				case SDL_QUIT:
					gfProgramIsRunning = FALSE;
					break;
			}
		}
		else
		{
			if (gfApplicationActive)
			{
				GameLoop();
				SDL_Delay(1); // XXX HACK0001
			}
			else
			{
				SDL_WaitEvent(NULL);
			}
		}
  }

  // This is the normal exit point
  FastDebugMsg("Exiting Game");

	// SGPExit() will be called next through the atexit() mechanism...  This way we correctly process both normal exits and
	// emergency aborts (such as those caused by a failed assertion).

	// return wParam of the last message received
	return 0;
}


static void SGPExit(void)
{
	static BOOLEAN fAlreadyExiting = FALSE;

	// helps prevent heap crashes when multiple assertions occur and call us
	if ( fAlreadyExiting )
	{
		return;
	}

	fAlreadyExiting = TRUE;
	gfProgramIsRunning = FALSE;

	ShutdownStandardGamingPlatform();
	if (gzErrorMsg[0] != '\0')
  {
		fprintf(stderr, "ERROR: %s\n", gzErrorMsg);
  }
}


void ShutdownWithErrorBox(CHAR8 *pcMessage)
{
	strncpy(gzErrorMsg, pcMessage, 255);
	gzErrorMsg[255]='\0';
	gfIgnoreMessages=TRUE;

	exit(0);
}


static void ProcessJa2CommandLineBeforeInitialization(const char* pCommandLine)
{
	CHAR8 cSeparators[]="\t =";
	CHAR8	*pCopy=NULL, *pToken;

	pCopy=(CHAR8 *)MemAlloc(strlen(pCommandLine) + 1);

	Assert(pCopy);
	if(!pCopy)
		return;

	memcpy(pCopy, pCommandLine, strlen(pCommandLine)+1);

	pToken=strtok(pCopy, cSeparators);
	while(pToken)
	{
		//if its the NO SOUND option
		if (strncasecmp(pToken, "/NOSOUND", 8) == 0)
		{
			//disable the sound
			SoundEnableSound(FALSE);
		}

		//get the next token
		pToken=strtok(NULL, cSeparators);
	}

	MemFree(pCopy);
}
