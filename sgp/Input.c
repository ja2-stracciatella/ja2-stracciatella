#include "Types.h"
#include <stdio.h>
#include <memory.h>
#include "Debug.h"
#include "Input.h"
#include "MemMan.h"
#include "English.h"
#include "Video.h"
#include "Local.h"


// Make sure to refer to the translation table which is within one of the following files (depending
// on the language used). ENGLISH.C, JAPANESE.C, FRENCH.C, GERMAN.C, SPANISH.C, etc...

extern UINT16 gsKeyTranslationTable[1024];

// The gfKeyState table is used to track which of the keys is up or down at any one time. This is used while polling
// the interface.

BOOLEAN   gfKeyState[256];			// TRUE = Pressed, FALSE = Not Pressed
static BOOLEAN fCursorWasClipped = FALSE;
static SGPRect gCursorClipRect;


// The gsKeyTranslationTables basically translates scan codes to our own key value table. Please note that the table is 2 bytes
// wide per entry. This will be used since we will use 2 byte characters for translation purposes.

UINT16   gfShiftState;					// TRUE = Pressed, FALSE = Not Pressed
UINT16   gfAltState;						// TRUE = Pressed, FALSE = Not Pressed
UINT16   gfCtrlState;						// TRUE = Pressed, FALSE = Not Pressed

// These data structure are used to track the mouse while polling

static UINT32 guiSingleClickTimer;
UINT16		gusRecordedKeyState;

static UINT32 guiLeftButtonRepeatTimer;
static UINT32 guiRightButtonRepeatTimer;

BOOLEAN   gfLeftButtonState;		// TRUE = Pressed, FALSE = Not Pressed
BOOLEAN   gfRightButtonState;		// TRUE = Pressed, FALSE = Not Pressed
UINT16    gusMouseXPos;					// X position of the mouse on screen
UINT16    gusMouseYPos;					// y position of the mouse on screen

// The queue structures are used to track input events using queued events

static InputAtom gEventQueue[256];
static UINT16    gusQueueCount;
static UINT16    gusHeadIndex;
static UINT16    gusTailIndex;

// This is the WIN95 hook specific data and defines used to handle the keyboard and
// mouse hook

static HHOOK ghKeyboardHook;
static HHOOK ghMouseHook;


#if 0 // XXX TODO

#ifdef JA2

LRESULT CALLBACK MouseHandler(int Code, WPARAM wParam, LPARAM lParam)
{
  if (Code < 0)
  { // Do not handle this message, pass it on to another window
    return CallNextHookEx(ghMouseHook, Code, wParam, lParam);
  }

  switch (wParam)
  {
    case WM_LBUTTONDOWN
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      // Update the button state
			gfLeftButtonState = TRUE;
      // Trigger an input event
      QueueEvent(LEFT_BUTTON_DOWN, 0);
	    break;
    case WM_LBUTTONUP
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      // Update the button state
      gfLeftButtonState = FALSE;
      // Trigger an input event
      QueueEvent(LEFT_BUTTON_UP, 0);
      break;
    case WM_RBUTTONDOWN
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      // Update the button state
      gfRightButtonState = TRUE;
      // Trigger an input event
      QueueEvent(RIGHT_BUTTON_DOWN, 0);
      break;
    case WM_RBUTTONUP
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      // Update the button state
      gfRightButtonState = FALSE;
      // Trigger an input event
      QueueEvent(RIGHT_BUTTON_UP, 0);
      break;
    case WM_MOUSEMOVE
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      break;
  }
  return TRUE;
}

#endif

#endif


BOOLEAN InitializeInputManager(void)
{
  // Link to debugger
  RegisterDebugTopic(TOPIC_INPUT, "Input Manager");
  // Initialize the gfKeyState table to FALSE everywhere
  memset(gfKeyState, FALSE, 256);
  // Initialize the Event Queue
  gusQueueCount = 0;
  gusHeadIndex  = 0;
  gusTailIndex  = 0;
  // Initialize other variables
  gfShiftState = FALSE;
  gfAltState   = FALSE;
  gfCtrlState  = FALSE;
  // Initialize variables pertaining to DOUBLE CLIK stuff
  guiSingleClickTimer = 0;
  // Initialize variables pertaining to the button states
  gfLeftButtonState  = FALSE;
  gfRightButtonState = FALSE;
  // Initialize variables pertaining to the repeat mechanism
  guiLeftButtonRepeatTimer = 0;
	guiRightButtonRepeatTimer = 0;
  // Set the mouse to the center of the screen
  gusMouseXPos = 320;
  gusMouseYPos = 240;
#if 1 // XXX TODO
	FIXME
#else
  ghMouseHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC) MouseHandler, (HINSTANCE) 0, GetCurrentThreadId());
  DbgMessage(TOPIC_INPUT, DBG_LEVEL_2, String("Set mouse hook returned %d", ghMouseHook));
#endif
  return TRUE;
}

void ShutdownInputManager(void)
{
	// There's very little to do when shutting down the input manager. In the future, this is where the keyboard and
  // mouse hooks will be destroyed
  UnRegisterDebugTopic(TOPIC_INPUT, "Input Manager");
#if 1 // XXX TODO
	FIXME
#else
  UnhookWindowsHookEx(ghKeyboardHook);
  UnhookWindowsHookEx(ghMouseHook);
#endif
}


void QueueEvent(UINT16 ubInputEvent, UINT32 usParam)
{
  UINT32 uiTimer;
  UINT16 usKeyState;

  uiTimer = GetTickCount();
  usKeyState = gfShiftState | gfCtrlState | gfAltState;

  // Can we queue up one more event, if not, the event is lost forever
  if (gusQueueCount == 256)
  { // No more queue space
    return;
  }

  if (ubInputEvent == LEFT_BUTTON_DOWN)
  {
    guiLeftButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIMEOUT;
  }

  if (ubInputEvent == RIGHT_BUTTON_DOWN)
  {
    guiRightButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIMEOUT;
  }

  if (ubInputEvent == LEFT_BUTTON_UP)
  {
    guiLeftButtonRepeatTimer = 0;
  }

  if (ubInputEvent == RIGHT_BUTTON_UP)
  {
    guiRightButtonRepeatTimer = 0;
  }

  if ( (ubInputEvent == LEFT_BUTTON_UP) )
  {
		// Do we have a double click
		if ( ( uiTimer - guiSingleClickTimer ) < DBL_CLK_TIME )
		{
				guiSingleClickTimer = 0;

				// Add a button up first...
				gEventQueue[gusTailIndex].usKeyState = gusRecordedKeyState;
				gEventQueue[gusTailIndex].usEvent = LEFT_BUTTON_UP;
				gEventQueue[gusTailIndex].usParam = usParam;

				// Increment the number of items on the input queue
				gusQueueCount++;

				// Increment the gusTailIndex pointer
				if (gusTailIndex == 255)
				{ // The gusTailIndex is about to wrap around the queue ring
					gusTailIndex = 0;
				}
				else
				{ // We simply increment the gusTailIndex
					gusTailIndex++;
				}


				// Now do double click
				gEventQueue[gusTailIndex].usKeyState = gusRecordedKeyState ;
				gEventQueue[gusTailIndex].usEvent = LEFT_BUTTON_DBL_CLK;
				gEventQueue[gusTailIndex].usParam = usParam;

				// Increment the number of items on the input queue
				gusQueueCount++;

				// Increment the gusTailIndex pointer
				if (gusTailIndex == 255)
				{ // The gusTailIndex is about to wrap around the queue ring
					gusTailIndex = 0;
				}
				else
				{ // We simply increment the gusTailIndex
					gusTailIndex++;
				}

				return;
		}
		else
		{
			// Save time
			guiSingleClickTimer = uiTimer;
		}
  }

  // Okey Dokey, we can queue up the event, so we do it
  gEventQueue[gusTailIndex].usKeyState = usKeyState;
  gEventQueue[gusTailIndex].usEvent = ubInputEvent;
  gEventQueue[gusTailIndex].usParam = usParam;

  // Increment the number of items on the input queue
  gusQueueCount++;

  // Increment the gusTailIndex pointer
  if (gusTailIndex == 255)
  { // The gusTailIndex is about to wrap around the queue ring
    gusTailIndex = 0;
  }
  else
  { // We simply increment the gusTailIndex
    gusTailIndex++;
  }
}

BOOLEAN DequeueSpecificEvent(InputAtom *Event, UINT32 uiMaskFlags )
{
  // Is there an event to dequeue
  if (gusQueueCount > 0)
  {
		memcpy( Event, &( gEventQueue[gusHeadIndex] ), sizeof( InputAtom ) );

		// Check if it has the masks!
		if ( ( Event->usEvent & uiMaskFlags ) )
		{
			return( DequeueEvent( Event) );
		}
	}

	return( FALSE );
}


static void HandleSingleClicksAndButtonRepeats(void);


BOOLEAN DequeueEvent(InputAtom *Event)
{
	HandleSingleClicksAndButtonRepeats( );

  // Is there an event to dequeue
  if (gusQueueCount > 0)
  {
		// We have an event, so we dequeue it
		memcpy( Event, &( gEventQueue[gusHeadIndex] ), sizeof( InputAtom ) );

		if (gusHeadIndex == 255)
		{
			gusHeadIndex = 0;
		}
		else
		{
			gusHeadIndex++;
		}

		// Decrement the number of items on the input queue
		gusQueueCount--;

		// dequeued an event, return TRUE
		return TRUE;
  }
	else
	{
		// No events to dequeue, return FALSE
		return FALSE;
	}
}


static void KeyChange(const SDL_keysym* KeySym, BOOLEAN Pressed)
{
  UINT32 ubKey;
  UINT16 ubChar;

#if 1 // XXX HACK0008
	SDLKey Key = KeySym->sym;
	if (Key >= SDLK_a && Key <= SDLK_z)
	{
		ubKey = gfShiftState ? Key - 32 : Key;
	}
	else if (Key >= SDLK_KP0 && Key <= SDLK_KP9)
	{
		if (KeySym->mod & KMOD_NUM)
		{
			ubKey = Key - SDLK_KP0 + SDLK_0;
		}
		else
		{
			switch (Key)
			{
				case SDLK_KP0: ubKey = INSERT;     break;
				case SDLK_KP1: ubKey = END;        break;
				case SDLK_KP2: ubKey = DNARROW;    break;
				case SDLK_KP3: ubKey = PGDN;       break;
				case SDLK_KP4: ubKey = LEFTARROW;  break;
				case SDLK_KP5: return;
				case SDLK_KP6: ubKey = RIGHTARROW; break;
				case SDLK_KP7: ubKey = HOME;       break;
				case SDLK_KP8: ubKey = UPARROW;    break;
				case SDLK_KP9: ubKey = DNARROW;    break;
			}
		}
	}
	else
	{
		switch (Key)
		{
			case SDLK_DELETE:   ubKey = DEL;        break;
			case SDLK_UP:       ubKey = UPARROW;    break;
			case SDLK_DOWN:     ubKey = DNARROW;    break;
			case SDLK_RIGHT:    ubKey = RIGHTARROW; break;
			case SDLK_LEFT:     ubKey = LEFTARROW;  break;
			case SDLK_HOME:     ubKey = HOME;       break;
			case SDLK_END:      ubKey = END;        break;
			case SDLK_PAGEUP:   ubKey = PGUP;       break;
			case SDLK_PAGEDOWN: ubKey = PGDN;       break;

			default:
				if (Key >= lengthof(gfKeyState)) return;
				ubKey = Key;
				break;
		}
	}
	ubChar = ubKey;
#else
	// Find ucChar by translating ubKey using the gsKeyTranslationTable. If the SHIFT, ALT or CTRL key are down, then
  // the index into the translation table us changed from ubKey to ubKey+256, ubKey+512 and ubKey+768 respectively
  if (gfShiftState == TRUE)
  { // SHIFT is pressed, hence we add 256 to ubKey before translation to ubChar
    ubChar = gsKeyTranslationTable[ubKey+256];
  }
  else
  {
		//
		// Even though gfAltState is checked as if it was a BOOLEAN, it really contains 0x02, which
		// is NOT == to true.  This is broken, however to fix it would break Ja2 and Wizardry.
		// The same thing goes for gfCtrlState and gfShiftState, howver gfShiftState is assigned 0x01 which IS == to TRUE.
		// Just something i found, and thought u should know about.  DF.
		//

    if( gfAltState == TRUE )
    { // ALT is pressed, hence ubKey is multiplied by 3 before translation to ubChar
      ubChar = gsKeyTranslationTable[ubKey+512];
    }
    else
    {
      if (gfCtrlState == TRUE)
      { // CTRL is pressed, hence ubKey is multiplied by 4 before translation to ubChar
        ubChar = gsKeyTranslationTable[ubKey+768];
      }
      else
      { // None of the SHIFT, ALT or CTRL are pressed hence we have a default translation of ubKey
        ubChar = gsKeyTranslationTable[ubKey];
      }
    }
  }
#endif

  if (Pressed)
  { // Key has been PRESSED
    // Find out if the key is already pressed and if not, queue an event and update the gfKeyState array
    if (gfKeyState[ubKey] == FALSE)
    { // Well the key has just been pressed, therefore we queue up and event and update the gsKeyState
			gfKeyState[ubKey] = TRUE;
			QueueEvent(KEY_DOWN, ubChar);
    }
    else
    { // Well the key gets repeated
			QueueEvent(KEY_REPEAT, ubChar);
    }
  }
  else
  { // Key has been RELEASED
    // Find out if the key is already pressed and if so, queue an event and update the gfKeyState array
    if (gfKeyState[ubKey] == TRUE)
    { // Well the key has just been pressed, therefore we queue up and event and update the gsKeyState
      gfKeyState[ubKey] = FALSE;
      QueueEvent(KEY_UP, ubChar);
    }
#if 0 // XXX TODO
		//else if the alt tab key was pressed
		else if( ubChar == TAB && gfAltState )
		{
			// therefore minimize the application
			ShowWindow( ghWindow, SW_MINIMIZE );
      gfKeyState[ ALT ] = FALSE;
			gfAltState = FALSE;
		}
#endif
	}
}


void KeyDown(const SDL_keysym* KeySym)
{ // Are we PRESSING down one of SHIFT, ALT or CTRL ???
	switch (KeySym->sym)
	{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			gfShiftState = SHIFT_DOWN;
			gfKeyState[SHIFT] = TRUE;
			break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			gfCtrlState = CTRL_DOWN;
			gfKeyState[CTRL] = TRUE;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
			gfAltState = ALT_DOWN;
			gfKeyState[ALT] = TRUE;
			break;

		case SDLK_PRINT:
		case SDLK_SCROLLOCK:
			break;

		default:
			KeyChange(KeySym, TRUE);
			break;
	}
}


void KeyUp(const SDL_keysym* KeySym)
{ // Are we RELEASING one of SHIFT, ALT or CTRL ???
	switch (KeySym->sym)
	{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			gfShiftState = FALSE;
			gfKeyState[SHIFT] = FALSE;
			break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			gfCtrlState = FALSE;
			gfKeyState[CTRL] = FALSE;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
			gfAltState = FALSE;
			gfKeyState[ALT] = FALSE;
			break;

		case SDLK_PRINT:
			if (_KeyDown(CTRL)) VideoCaptureToggle(); else PrintScreen();
			break;

		case SDLK_SCROLLOCK:
			SDL_WM_GrabInput
			(
				SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF ?
					SDL_GRAB_ON : SDL_GRAB_OFF
			);
			break;

		default:
			KeyChange(KeySym, FALSE);
			break;
	}
}


void GetMousePos(SGPPoint *Point)
{
  POINT MousePos;

  GetCursorPos(&MousePos);

  Point->iX = (UINT32) MousePos.x;
  Point->iY = (UINT32) MousePos.y;
}


//
// Miscellaneous input-related utility functions:
//

void RestrictMouseToXYXY(UINT16 usX1, UINT16 usY1, UINT16 usX2, UINT16 usY2)
{
	SGPRect TempRect;

	TempRect.iLeft   = usX1;
	TempRect.iTop    = usY1;
	TempRect.iRight  = usX2;
	TempRect.iBottom = usY2;

	RestrictMouseCursor(&TempRect);
}

void RestrictMouseCursor(SGPRect *pRectangle)
{
  // Make a copy of our rect....
	gCursorClipRect = *pRectangle;
#if 1 // XXX TODO0000 Should probably removed completly. Confining the mouse cursor is The Wrong Thing(tm)
#else
  ClipCursor((RECT *)pRectangle);
#endif
	fCursorWasClipped = TRUE;
}

void FreeMouseCursor(void)
{
#if 1 // XXX TODO0000
#else
  ClipCursor(NULL);
#endif
	fCursorWasClipped = FALSE;
}

void RestoreCursorClipRect( void )
{
#if 1 // XXX TODO0000
	UNIMPLEMENTED();
#else
  if ( fCursorWasClipped )
  {
    ClipCursor( &gCursorClipRect );
  }
#endif
}

void GetRestrictedClipCursor( SGPRect *pRectangle )
{
#if 1 // XXX TODO0000
	pRectangle->iLeft   = 0;
	pRectangle->iTop    = 0;
	pRectangle->iRight  = SCREEN_WIDTH;
	pRectangle->iBottom = SCREEN_HEIGHT;
#else
	GetClipCursor((RECT *) pRectangle );
#endif
}

BOOLEAN IsCursorRestricted( void )
{
	return( fCursorWasClipped );
}

void SimulateMouseMovement( UINT32 uiNewXPos, UINT32 uiNewYPos )
{
	FLOAT flNewXPos, flNewYPos;

	// Wizardry NOTE: This function currently doesn't quite work right for in any Windows resolution other than 640x480.
	// mouse_event() uses your current Windows resolution to calculate the resulting x,y coordinates.  So in order to get
	// the right coordinates, you'd have to find out the current Windows resolution through a system call, and then do:
	//		uiNewXPos = uiNewXPos * SCREEN_WIDTH  / WinScreenResX;
	//		uiNewYPos = uiNewYPos * SCREEN_HEIGHT / WinScreenResY;
	//
	// JA2 doesn't have this problem, 'cause they use DirectDraw calls that change the Windows resolution properly.
	//
	// Alex Meduna, Dec. 3, 1997

#if 1
	FIXME
	SDL_WarpMouse(uiNewXPos, uiNewYPos);
#else
	// Adjust coords based on our resolution
	flNewXPos = ( (FLOAT)uiNewXPos / SCREEN_WIDTH ) * 65536;
	flNewYPos = ( (FLOAT)uiNewYPos / SCREEN_HEIGHT ) * 65536;

	mouse_event( MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, (UINT32)flNewXPos, (UINT32)flNewYPos, 0, 0 );
#endif
}


void DequeueAllKeyBoardEvents()
{
#if 1 // XXX TODO
	FIXME
#else
	InputAtom  InputEvent;
	MSG				 KeyMessage;


	//dequeue all the events waiting in the windows queue
	while( PeekMessage( &KeyMessage, ghWindow, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) );

	//Deque all the events waiting in the SGP queue
	while (DequeueEvent(&InputEvent) == TRUE)
  {
		//dont do anything
	}
#endif
}


static void HandleSingleClicksAndButtonRepeats(void)
{
  UINT32 uiTimer;


  uiTimer = GetTickCount();

  // Is there a LEFT mouse button repeat
  if (gfLeftButtonState)
  {
    if ((guiLeftButtonRepeatTimer > 0)&&(guiLeftButtonRepeatTimer <= uiTimer))
    {
      QueueEvent(LEFT_BUTTON_REPEAT, 0);
      guiLeftButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
    }
  }
  else
  {
    guiLeftButtonRepeatTimer = 0;
  }


	// Is there a RIGHT mouse button repeat
	if (gfRightButtonState)
  {
    if ((guiRightButtonRepeatTimer > 0)&&(guiRightButtonRepeatTimer <= uiTimer))
    {
      QueueEvent(RIGHT_BUTTON_REPEAT, 0);
      guiRightButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
    }
  }
  else
  {
    guiRightButtonRepeatTimer = 0;
  }
}
