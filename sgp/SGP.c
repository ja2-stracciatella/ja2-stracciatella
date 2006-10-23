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


extern UINT32 MemDebugCounter;
#ifdef JA2
extern BOOLEAN gfPauseDueToPlayerGamePause;
#endif

extern  void    QueueEvent(UINT16 ubInputEvent, UINT32 usParam, UINT32 uiParam);

// Prototype Declarations

INT32 FAR PASCAL WindowProcedure(HWND hWindow, UINT16 Message, WPARAM wParam, LPARAM lParam);


#if !defined(JA2) && !defined(UTILS)
void							ProcessCommandLine(CHAR8 *pCommandLine);

// Should the game immediately load the quick save at startup?
BOOLEAN						gfLoadAtStartup=FALSE;
BOOLEAN						gfUsingBoundsChecker=FALSE;
CHAR8						*gzStringDataOverride=NULL;
BOOLEAN						gfCapturingVideo = FALSE;

#endif


// Global Variable Declarations
#ifdef WINDOWED_MODE
RECT				rcWindow;
#endif

BOOLEAN gfApplicationActive;
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
				QueueEvent(MOUSE_WHEEL, wParam, lParam);
				break;
			}

#ifdef JA2
#ifdef WINDOWED_MODE
    case WM_MOVE:

        GetClientRect(hWindow, &rcWindow);
        ClientToScreen(hWindow, (LPPOINT)&rcWindow);
        ClientToScreen(hWindow, (LPPOINT)&rcWindow+1);
        break;
#endif
#else
		case WM_MOUSEMOVE:
			break;

		case WM_SIZING:
		{
			LPRECT	lpWindow;
			INT32		iWidth, iHeight, iX, iY;
			BOOLEAN fWidthByHeight=FALSE, fHoldRight=FALSE;

			lpWindow = (LPRECT) lParam;

			iWidth=lpWindow->right-lpWindow->left;
			iHeight=lpWindow->bottom-lpWindow->top;
			iX = (lpWindow->left + lpWindow->right)/2;
			iY = (lpWindow->top + lpWindow->bottom)/2;

			switch(wParam)
			{
				case WMSZ_BOTTOMLEFT:
					fHoldRight=TRUE;
				case WMSZ_BOTTOM:
				case WMSZ_BOTTOMRIGHT:
					if(iHeight < SCREEN_HEIGHT)
					{
						lpWindow->bottom=lpWindow->top+SCREEN_HEIGHT;
						iHeight=SCREEN_HEIGHT;
					}
					fWidthByHeight=TRUE;
				break;

				case WMSZ_TOPLEFT:
					fHoldRight=TRUE;
				case WMSZ_TOP:
				case WMSZ_TOPRIGHT:
					if(iHeight < SCREEN_HEIGHT)
					{
						lpWindow->top=lpWindow->bottom-SCREEN_HEIGHT;
						iHeight=SCREEN_HEIGHT;
					}
					fWidthByHeight=TRUE;
					break;

				case WMSZ_LEFT:
					if(iWidth < SCREEN_WIDTH)
					{
						lpWindow->left=lpWindow->right-SCREEN_WIDTH;
						iWidth = SCREEN_WIDTH;
					}
					break;

				case WMSZ_RIGHT:
					if(iWidth < SCREEN_WIDTH)
					{
						lpWindow->right=lpWindow->left+SCREEN_WIDTH;
						iWidth = SCREEN_WIDTH;
					}
			}

			// Calculate width as a factor of height
			if(fWidthByHeight)
			{
				iWidth = iHeight * SCREEN_WIDTH / SCREEN_HEIGHT;
//				lpWindow->left = iX - iWidth/2;
//				lpWindow->right = iX + iWidth / 2;
				if(fHoldRight)
					lpWindow->left = lpWindow->right - iWidth;
				else
					lpWindow->right = lpWindow->left + iWidth;
			}
			else // Calculate height as a factor of width
			{
				iHeight = iWidth * SCREEN_HEIGHT / SCREEN_WIDTH;
//				lpWindow->top = iY - iHeight/2;
//				lpWindow->bottom = iY + iHeight/2;
				lpWindow->bottom = lpWindow->top + iHeight;
			}


/*
			switch(wParam)
			{
				case WMSZ_BOTTOM:
				case WMSZ_BOTTOMLEFT:
				case WMSZ_BOTTOMRIGHT:
					if(iHeight < SCREEN_HEIGHT)
						lpWindow->bottom=lpWindow->top+SCREEN_HEIGHT;
			}

			switch(wParam)
			{
				case WMSZ_TOP:
				case WMSZ_TOPLEFT:
				case WMSZ_TOPRIGHT:
					if(iHeight < SCREEN_HEIGHT)
						lpWindow->top=lpWindow->bottom-SCREEN_HEIGHT;
			}

			switch(wParam)
			{
				case WMSZ_BOTTOMLEFT:
				case WMSZ_LEFT:
				case WMSZ_TOPLEFT:
					if(iWidth < SCREEN_WIDTH)
						lpWindow->left=lpWindow->right-SCREEN_WIDTH;
			}

			switch(wParam)
			{
				case WMSZ_BOTTOMRIGHT:
				case WMSZ_RIGHT:
				case WMSZ_TOPRIGHT:
					if(iWidth < SCREEN_WIDTH)
						lpWindow->right=lpWindow->left+SCREEN_WIDTH;
			}
*/
		}
		break;

    case WM_SIZE:
		{
			UINT16 nWidth = LOWORD(lParam);  // width of client area
			UINT16 nHeight = HIWORD(lParam); // height of client area

			if(nWidth && nHeight)
			{
				switch(wParam)
				{
					case SIZE_MAXIMIZED:
						VideoFullScreen(TRUE);
						break;

					case SIZE_RESTORED:
						VideoResizeWindow();
						break;
				}
			}
		}
		break;

    case WM_MOVE:
		{
			INT32 xPos = (INT32)LOWORD(lParam);    // horizontal position
			INT32 yPos = (INT32)HIWORD(lParam);    // vertical position
		}
		break;
#endif

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
#else
						if(!VideoInspectorIsEnabled())
						{
	            RestoreVideoManager();
						}

	          MoveTimer(TIMER_RESUME);
#endif
            gfApplicationActive = TRUE;
          }
          break;
        case FALSE: // We are suspending direct draw
#ifdef JA2
						// pause the JA2 Global clock
						PauseTime( TRUE );
						SuspendVideoManager();
#else
#ifndef UTIL
						if(!VideoInspectorIsEnabled())
							SuspendVideoManager();
#endif
#endif
          // suspend movement timer, to prevent timer crash if delay becomes long
          // * it doesn't matter whether the 3-D engine is actually running or not, or if it's even been initialized
          // * restore is automatic, no need to do anything on reactivation
#if !defined( JA2 ) && !defined( UTIL )
          MoveTimer(TIMER_SUSPEND);
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
#if !defined( JA2 ) && !defined( UTIL )
			if(!VideoInspectorIsEnabled())
				RestoreVideoManager();
			gfApplicationActive=TRUE;
//			RestrictMouseToXYXY(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
#else
      RestoreCursorClipRect( );
#endif

			break;

		case WM_KILLFOCUS:
#if !defined( JA2 ) && !defined( UTIL )
			if(!VideoInspectorIsEnabled())
				SuspendVideoManager();

			gfApplicationActive=FALSE;
			FreeMouseCursor();
#endif
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
	if (!InitializeVideoManager())
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
#ifndef JA2
	static BOOLEAN Reenter = FALSE;

	//
	// Prevent multiple reentry into this function
	//

	if (Reenter == FALSE)
	{
		Reenter = TRUE;
	}
	else
	{
		return;
	}
#endif

	//
	// Shut down the different components of the SGP
	//

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

				case SDL_KEYDOWN:
				{
					int x;
					int y;

					SDL_GetMouseState(&x, &y);
					UINT32 pos = y << 16 | x;

					SDLKey key = event.key.keysym.sym; // XXX mapping
					if (key < lengthof(gfKeyState))
					{
						if (!gfKeyState[key])
						{
							gfKeyState[key] = TRUE;
							QueueEvent(KEY_DOWN, key, pos);
						}
						else
						{
							QueueEvent(KEY_REPEAT, key, pos);
						}
					}
					break;
				}

				case SDL_KEYUP:
				{
					SDLKey key = event.key.keysym.sym;

					switch (key)
					{
						case SDLK_PRINT:
							PrintScreen();
							break;

						default:
							if (key < lengthof(gfKeyState)) // XXX mapping
							{
								int x;
								int y;
								SDL_GetMouseState(&x, &y);
								UINT32 pos = y << 16 | x;

								gfKeyState[key] = FALSE;
								QueueEvent(KEY_UP, key, pos);
							}
							break;
					}
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					UINT32 pos = event.button.y << 16 | event.button.x;

					gusMouseXPos = event.button.x;
					gusMouseYPos = event.button.y;

					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							gfLeftButtonState = TRUE;
							QueueEvent(LEFT_BUTTON_DOWN, 0, pos);
							break;

						case SDL_BUTTON_RIGHT:
							gfRightButtonState = TRUE;
							QueueEvent(RIGHT_BUTTON_DOWN, 0, pos);
							break;
					}
					break;
				}

				case SDL_MOUSEBUTTONUP:
				{
					UINT32 pos = event.button.y << 16 | event.button.x;

					gusMouseXPos = event.button.x;
					gusMouseYPos = event.button.y;

					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							gfLeftButtonState = FALSE;
							QueueEvent(LEFT_BUTTON_UP, 0, pos);
							break;

						case SDL_BUTTON_RIGHT:
							gfRightButtonState = FALSE;
							QueueEvent(RIGHT_BUTTON_UP, 0, pos);
							break;
					}
					break;
				}

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
	BOOLEAN fUnloadScreens = TRUE;


	// helps prevent heap crashes when multiple assertions occur and call us
	if ( fAlreadyExiting )
	{
		return;
	}

	fAlreadyExiting = TRUE;
	gfProgramIsRunning = FALSE;

// Wizardry only
#if !defined( JA2 ) && !defined( UTIL )
	if (gfGameInitialized)
	{
// ARM: if in DEBUG mode & we've ShutdownWithErrorBox, don't unload screens and release data structs to permit easier debugging
#ifdef _DEBUG
		if (gfIgnoreMessages)
		{
			fUnloadScreens = FALSE;
		}
#endif
		GameloopExit(fUnloadScreens);
	}
#endif

	ShutdownStandardGamingPlatform();
	if (gzErrorMsg[0] != '\0')
  {
		fprintf(stderr, "ERROR: %s\n", gzErrorMsg);
  }

#ifndef JA2
	VideoDumpMemoryLeaks();
#endif
}


void ShutdownWithErrorBox(CHAR8 *pcMessage)
{
	strncpy(gzErrorMsg, pcMessage, 255);
	gzErrorMsg[255]='\0';
	gfIgnoreMessages=TRUE;

	exit(0);
}

#if !defined(JA2) && !defined(UTILS)

void ProcessCommandLine(CHAR8 *pCommandLine)
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
		if(!_strnicmp(pToken, "/NOSOUND", 8))
		{
			SoundEnableSound(FALSE);
		}
		else if(!_strnicmp(pToken, "/INSPECTOR", 10))
		{
			VideoInspectorEnable();
		}
		else if(!_strnicmp(pToken, "/VIDEOCFG", 9))
		{
			pToken=strtok(NULL, cSeparators);
			VideoSetConfigFile(pToken);
		}
		else if(!_strnicmp(pToken, "/LOAD", 5))
		{
			gfLoadAtStartup=TRUE;
		}
		else if(!_strnicmp(pToken, "/WINDOW", 7))
		{
			VideoFullScreen(FALSE);
		}
		else if(!_strnicmp(pToken, "/BC", 7))
		{
			gfUsingBoundsChecker = TRUE;
		}
		else if(!_strnicmp(pToken, "/CAPTURE", 7))
		{
			gfCapturingVideo = TRUE;
		}
		else if(!_strnicmp(pToken, "/NOOCT", 6))
		{
			NoOct();
		}
		else if(!_strnicmp(pToken, "/STRINGDATA", 11))
		{
			pToken=strtok(NULL, cSeparators);
			gzStringDataOverride = (CHAR8 *)MemAlloc(strlen(pToken) + 1);
			strcpy(gzStringDataOverride, pToken);
		}

		pToken=strtok(NULL, cSeparators);
	}

	MemFree(pCopy);
}

#endif


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
