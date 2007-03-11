#include "Types.h"
#include <stdio.h>
#include <memory.h>
#include "Debug.h"
#include "Input.h"
#include "MemMan.h"
#include "English.h"
#if defined( JA2 ) || defined( UTIL )
#	include "Video.h"
#else
#	include "video2.h"
#endif
#include "Local.h"


// Make sure to refer to the translation table which is within one of the following files (depending
// on the language used). ENGLISH.C, JAPANESE.C, FRENCH.C, GERMAN.C, SPANISH.C, etc...

extern UINT16 gsKeyTranslationTable[1024];

extern BOOLEAN gfApplicationActive;

#ifndef JA2

#undef GetCursorPos
#define GetCursorPos SGPMouseGetPos

#endif

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

BOOLEAN   gfTrackMousePos;			// TRUE = queue mouse movement events, FALSE = don't
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

// If the following pointer is non NULL then input characters are redirected to
// the related string

BOOLEAN      gfCurrentStringInputState;
StringInput *gpCurrentStringDescriptor;

// Local function headers

void    RedirectToString(UINT16 uiInputCharacter);

// These are the hook functions for both keyboard and mouse

#if 0 // XXX TODO
LRESULT CALLBACK KeyboardHandler(int Code, WPARAM wParam, LPARAM lParam)
{
#ifndef JA2
  if((Code < 0) || (!gfApplicationActive))
#else
  if (Code < 0)
#endif
  { // Do not handle this message, pass it on to another window
    return CallNextHookEx(ghKeyboardHook, Code, wParam, lParam);
  }

  if (lParam & TRANSITION_MASK)
  { // The key has been released
    KeyUp(wParam, lParam);
  }
  else
  { // Key was up
    KeyDown(wParam, lParam);
  }

  return TRUE;
}

#ifdef JA2

LRESULT CALLBACK MouseHandler(int Code, WPARAM wParam, LPARAM lParam)
{
  UINT32 uiParam;

#ifndef JA2
  if((Code < 0) || (!gfApplicationActive))
#else
  if (Code < 0)
#endif
  { // Do not handle this message, pass it on to another window
    return CallNextHookEx(ghMouseHook, Code, wParam, lParam);
  }

  switch (wParam)
  {
    case WM_LBUTTONDOWN
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      uiParam = gusMouseYPos;
      uiParam = uiParam << 16;
      uiParam = uiParam | gusMouseXPos;
      // Update the button state
			gfLeftButtonState = TRUE;
      // Trigger an input event
      QueueEvent(LEFT_BUTTON_DOWN, 0, uiParam);
	    break;
    case WM_LBUTTONUP
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      uiParam = gusMouseYPos;
      uiParam = uiParam << 16;
      uiParam = uiParam | gusMouseXPos;
      // Update the button state
      gfLeftButtonState = FALSE;
      // Trigger an input event
      QueueEvent(LEFT_BUTTON_UP, 0, uiParam);
      break;
    case WM_RBUTTONDOWN
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      uiParam = gusMouseYPos;
      uiParam = uiParam << 16;
      uiParam = uiParam | gusMouseXPos;
      // Update the button state
      gfRightButtonState = TRUE;
      // Trigger an input event
      QueueEvent(RIGHT_BUTTON_DOWN, 0, uiParam);
      break;
    case WM_RBUTTONUP
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      uiParam = gusMouseYPos;
      uiParam = uiParam << 16;
      uiParam = uiParam | gusMouseXPos;
      // Update the button state
      gfRightButtonState = FALSE;
      // Trigger an input event
      QueueEvent(RIGHT_BUTTON_UP, 0, uiParam);
      break;
    case WM_MOUSEMOVE
    : // Update the current mouse position
      gusMouseXPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).x;
      gusMouseYPos = (UINT16)(((MOUSEHOOKSTRUCT *)lParam)->pt).y;
      uiParam = gusMouseYPos;
      uiParam = uiParam << 16;
      uiParam = uiParam | gusMouseXPos;
      // Trigger an input event
      if (gfTrackMousePos == TRUE)
      {
        QueueEvent(MOUSE_POS, 0, uiParam);
      }
      break;
  }
  return TRUE;
}

#else

// Wizardry mouse hander

LRESULT CALLBACK MouseHandler(int Code, WPARAM wParam, LPARAM lParam)
{
UINT32 uiParam;
UINT32	uiXPos, uiYPos;
RECT rcClient;
BOOLEAN fOutsideClient=FALSE;
static BOOLEAN fResizing=FALSE;
LRESULT Result;

  uiXPos= (((MOUSEHOOKSTRUCT *)lParam)->pt).x;
  uiYPos=	(((MOUSEHOOKSTRUCT *)lParam)->pt).y;


	if(!VideoIsFullScreen())
	{
		if(wParam==WM_NCLBUTTONDOWN)
				fResizing=TRUE;

		VideoGetClientRect(&rcClient);
		if((uiXPos < (UINT32)rcClient.left) || (uiXPos > (UINT32)rcClient.right) || (uiYPos < (UINT32)rcClient.top) || (uiYPos > (UINT32)rcClient.bottom))
			fOutsideClient=TRUE;
	}

  if((Code < 0) || (!gfApplicationActive) || fOutsideClient || fResizing)
  { // Do not handle this message, pass it on to another window
    Result = CallNextHookEx(ghMouseHook, Code, wParam, lParam);

		if((wParam==WM_LBUTTONUP) || (wParam==WM_NCLBUTTONUP))
			fResizing=FALSE;

		return(Result);
  }

  switch (wParam)
  {
    case WM_LBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
			if(VideoIsFullScreen())
			{
	      gusMouseXPos = (UINT16)(uiXPos);
		    gusMouseYPos = (UINT16)(uiYPos);
			}
			else
			{
		    gusMouseXPos = (UINT16)(uiXPos-rcClient.left);
	      gusMouseYPos = (UINT16)(uiYPos-rcClient.top);
			}
		  uiParam = (UINT32)gusMouseYPos<<16 | (UINT32)gusMouseXPos;
	    break;
  }

	if ( wParam == WM_MOUSEWHEEL )
	{
		return( FALSE );
	}

  switch (wParam)
  {
    case WM_LBUTTONDOWN:
			gfLeftButtonState = TRUE;
      QueueEvent(LEFT_BUTTON_DOWN, 0, uiParam);
	    break;
    case WM_LBUTTONUP:
			gfLeftButtonState = FALSE;
      QueueEvent(LEFT_BUTTON_UP, 0, uiParam);
      break;
    case WM_RBUTTONDOWN:
      gfRightButtonState = TRUE;
      QueueEvent(RIGHT_BUTTON_DOWN, 0, uiParam);
      break;
    case WM_RBUTTONUP:
      gfRightButtonState = FALSE;
      QueueEvent(RIGHT_BUTTON_UP, 0, uiParam);
      break;
    case WM_MOUSEMOVE:
      if(gfTrackMousePos)
        QueueEvent(MOUSE_POS, 0, uiParam);
      break;
  }

  return(TRUE);
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
  // By default, we will not queue mousemove events
  gfTrackMousePos = FALSE;
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
  // Initialize the string input mechanism
  gfCurrentStringInputState = FALSE;
  gpCurrentStringDescriptor = NULL;
#if 1 // XXX TODO
	FIXME
#else
  // Activate the hook functions for both keyboard and Mouse
  ghKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC) KeyboardHandler, (HINSTANCE) 0, GetCurrentThreadId());
  DbgMessage(TOPIC_INPUT, DBG_LEVEL_2, String("Set keyboard hook returned %d", ghKeyboardHook));

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

void QueuePureEvent(UINT16 ubInputEvent, UINT32 usParam, UINT32 uiParam)
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

  // Okey Dokey, we can queue up the event, so we do it
  gEventQueue[gusTailIndex].uiTimeStamp = uiTimer;
  gEventQueue[gusTailIndex].usKeyState = usKeyState;
  gEventQueue[gusTailIndex].usEvent = ubInputEvent;
  gEventQueue[gusTailIndex].usParam = usParam;
  gEventQueue[gusTailIndex].uiParam = uiParam;

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

void QueueEvent(UINT16 ubInputEvent, UINT32 usParam, UINT32 uiParam)
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
				gEventQueue[gusTailIndex].uiTimeStamp = uiTimer;
				gEventQueue[gusTailIndex].usKeyState = gusRecordedKeyState;
				gEventQueue[gusTailIndex].usEvent = LEFT_BUTTON_UP;
				gEventQueue[gusTailIndex].usParam = usParam;
				gEventQueue[gusTailIndex].uiParam = uiParam;

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
				gEventQueue[gusTailIndex].uiTimeStamp = uiTimer;
				gEventQueue[gusTailIndex].usKeyState = gusRecordedKeyState ;
				gEventQueue[gusTailIndex].usEvent = LEFT_BUTTON_DBL_CLK;
				gEventQueue[gusTailIndex].usParam = usParam;
				gEventQueue[gusTailIndex].uiParam = uiParam;

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
  gEventQueue[gusTailIndex].uiTimeStamp = uiTimer;
  gEventQueue[gusTailIndex].usKeyState = usKeyState;
  gEventQueue[gusTailIndex].usEvent = ubInputEvent;
  gEventQueue[gusTailIndex].usParam = usParam;
  gEventQueue[gusTailIndex].uiParam = uiParam;

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

void KeyChange(UINT32 usParam, UINT32 uiParam, UINT8 ufKeyState)
{
#if 1 // XXX TODO
	UNIMPLEMENTED();
#else
  UINT32 ubKey;
  UINT16 ubChar;
  POINT  MousePos;
  UINT32 uiTmpLParam;

  if ((usParam >= 96)&&(usParam <= 110))
  { // Well this could be a NUMPAD character imitating the center console characters (when NUMLOCK is OFF). Well we
    // gotta find out what was pressed and translate it to the actual physical key (i.e. if we think that HOME was
    // pressed but NUM_7 was pressed, the we translate the key into NUM_7
    switch(usParam)
    { case 96 // NUM_0
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 82)
        { // Well its the NUM_9 key and not actually the PGUP key
          ubKey = 223;
        }
        else
        { // NOP, its the PGUP key all right
          ubKey = usParam;
        }
        break;
      case 110 // NUM_PERIOD
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 83)
        { // Well its the NUM_3 key and not actually the PGDN key
          ubKey = 224;
        }
        else
        { // NOP, its the PGDN key all right
          ubKey = usParam;
        }
        break;
      case 97 // NUM_1
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 79)
        { // Well its the NUM_1 key and not actually the END key
          ubKey = 225;
        }
        else
        { // NOP, its the END key all right
          ubKey = usParam;
        }
        break;
      case 98 // NUM_2
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 80)
        { // Well its the NUM_7 key and not actually the HOME key
          ubKey = 226;
        }
        else
        { // NOP, its the HOME key all right
          ubKey = usParam;
        }
        break;
      case 99 // NUM_3
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 81)
        { // Well its the NUM_4 key and not actually the LARROW key
          ubKey = 227;
        }
        else
        { // NOP, it's the LARROW key all right
          ubKey = usParam;
        }
        break;
      case 100 // NUM_4
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 75)
        { // Well its the NUM_8 key and not actually the UPARROW key
          ubKey = 228;
        }
        else
        { // NOP, it's the UPARROW key all right
          ubKey = usParam;
        }
        break;
      case 101 // NUM_5
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 76)
        { // Well its the NUM_6 key and not actually the RARROW key
          ubKey = 229;
        }
        else
        { // NOP, it's the RARROW key all right
          ubKey = usParam;
        }
        break;
      case 102 // NUM_6
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 77)
        { // Well its the NUM_2 key and not actually the DNARROW key
          ubKey = 230;
        }
        else
        { // NOP, it's the DNARROW key all right
          ubKey = usParam;
        }
        break;
      case 103 // NUM_7
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 71)
        { // Well its the NUM_0 key and not actually the INSERT key
          ubKey = 231;
        }
        else
        { // NOP, it's the INSERT key all right
          ubKey = usParam;
        }
        break;
      case 104 // NUM_8
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 72)
        { // Well its the NUM_PERIOD key and not actually the DELETE key
          ubKey = 232;
        }
        else
        { // NOP, it's the DELETE key all right
          ubKey = usParam;
        }
        break;
      case 105 // NUM_9
      : if (((uiParam & SCAN_CODE_MASK) >> 16) == 73)
        { // Well its the NUM_PERIOD key and not actually the DELETE key
          ubKey = 233;
        }
        else
        { // NOP, it's the DELETE key all right
          ubKey = usParam;
        }
        break;
      default
      : ubKey = usParam;
        break;
    }
  }
  else
  {
    if ((usParam >= 33)&&(usParam <= 46))
    { // Well this could be a NUMPAD character imitating the center console characters (when NUMLOCK is OFF). Well we
      // gotta find out what was pressed and translate it to the actual physical key (i.e. if we think that HOME was
      // pressed but NUM_7 was pressed, the we translate the key into NUM_7
      switch(usParam)
      { case 45 // NUM_0
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 82)
          { // Is it the NUM_0 key or the INSERT key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the INSERT key
              ubKey = 245;
            }
            else
            { // Is the NUM_0 key with NUM lock off
              ubKey = 234;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 46 // NUM_PERIOD
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 83)
          { // Is it the NUM_PERIOD key or the DEL key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the DELETE key
              ubKey = 246;
            }
            else
            { // Is the NUM_PERIOD key with NUM lock off
              ubKey = 235;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 35 // NUM_1
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 79)
          { // Is it the NUM_1 key or the END key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the END key
              ubKey = 247;
            }
            else
            { // Is the NUM_1 key with NUM lock off
              ubKey = 236;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 40 // NUM_2
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 80)
          { // Is it the NUM_2 key or the DOWN key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the DOWN key
              ubKey = 248;
            }
            else
            { // Is the NUM_2 key with NUM lock off
              ubKey = 237;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 34 // NUM_3
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 81)
          { // Is it the NUM_3 key or the PGDN key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the PGDN key
              ubKey = 249;
            }
            else
            { // Is the NUM_3 key with NUM lock off
              ubKey = 238;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 37 // NUM_4
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 75)
          { // Is it the NUM_4 key or the LEFT key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the LEFT key
              ubKey = 250;
            }
            else
            { // Is the NUM_4 key with NUM lock off
              ubKey = 239;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 39 // NUM_6
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 77)
          { // Is it the NUM_6 key or the RIGHT key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the RIGHT key
              ubKey = 251;
            }
            else
            { // Is the NUM_6 key with NUM lock off
              ubKey = 241;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 36 // NUM_7
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 71)
          { // Is it the NUM_7 key or the HOME key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the HOME key
              ubKey = 252;
            }
            else
            { // Is the NUM_7 key with NUM lock off
              ubKey = 242;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 38 // NUM_8
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 72)
          { // Is it the NUM_8 key or the UP key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the UP key
              ubKey = 253;
            }
            else
            { // Is the NUM_8 key with NUM lock off
              ubKey = 243;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        case 33 // NUM_9
        : if (((uiParam & SCAN_CODE_MASK) >> 16) == 73)
          { // Is it the NUM_9 key or the PGUP key
            if (((uiParam & EXT_CODE_MASK) >> 17) != 0)
            { // It's the PGUP key
              ubKey = 254;
            }
            else
            { // Is the NUM_9 key with NUM lock off
              ubKey = 244;
            }
          }
          else
          {
            ubKey = usParam;
          }
          break;
        default
        : ubKey = usParam;
          break;
      }
    }
    else
    {
      if (usParam == 12)
      { // NUM_5 with NUM_LOCK off
        ubKey = 240;
      }
      else
      { // Normal key
        ubKey = usParam;
      }
    }
  }

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

  GetCursorPos(&MousePos);
  uiTmpLParam = ((MousePos.y << 16) & 0xffff0000) | (MousePos.x & 0x0000ffff);

  if (ufKeyState == TRUE)
  { // Key has been PRESSED
    // Find out if the key is already pressed and if not, queue an event and update the gfKeyState array
    if (gfKeyState[ubKey] == FALSE)
    { // Well the key has just been pressed, therefore we queue up and event and update the gsKeyState
      if (gfCurrentStringInputState == FALSE)
      {
        // There is no string input going on right now, so we queue up the event
        gfKeyState[ubKey] = TRUE;
        QueueEvent(KEY_DOWN, ubChar, uiTmpLParam);
			}
      else
      { // There is a current input string which will capture this event
        RedirectToString(ubChar);
        DbgMessage(TOPIC_INPUT, DBG_LEVEL_0, String("Pressed character %d (%d)", ubChar, ubKey));
      }
    }
    else
    { // Well the key gets repeated
      if (gfCurrentStringInputState == FALSE)
      { // There is no string input going on right now, so we queue up the event
        QueueEvent(KEY_REPEAT, ubChar, uiTmpLParam);
      }
      else
      { // There is a current input string which will capture this event
        RedirectToString(ubChar);
      }
    }
  }
  else
  { // Key has been RELEASED
    // Find out if the key is already pressed and if so, queue an event and update the gfKeyState array
    if (gfKeyState[ubKey] == TRUE)
    { // Well the key has just been pressed, therefore we queue up and event and update the gsKeyState
      gfKeyState[ubKey] = FALSE;
      QueueEvent(KEY_UP, ubChar, uiTmpLParam);
    }
		//else if the alt tab key was pressed
		else if( ubChar == TAB && gfAltState )
		{
			// therefore minimize the application
			ShowWindow( ghWindow, SW_MINIMIZE );
      gfKeyState[ ALT ] = FALSE;
			gfAltState = FALSE;
		}
  }
#endif
}

void KeyDown(UINT32 usParam, UINT32 uiParam)
{ // Are we PRESSING down one of SHIFT, ALT or CTRL ???
  if (usParam == 16)
  { // SHIFT key is PRESSED
    gfShiftState = SHIFT_DOWN;
    gfKeyState[16] = TRUE;
  }
  else
  {
    if (usParam == 17)
    { // CTRL key is PRESSED
      gfCtrlState = CTRL_DOWN;
      gfKeyState[17] = TRUE;
    }
    else
    {
      if (usParam == 18)
      { // ALT key is pressed
        gfAltState = ALT_DOWN;
        gfKeyState[18] = TRUE;
      }
      else
      {
        if (usParam == SNAPSHOT)
        {
          //PrintScreen();
					// DB Done in the KeyUp function
					// this used to be keyed to SCRL_LOCK
					// which I believe Luis gave the wrong value
        }
        else
        {
          // No special keys have been pressed
          // Call KeyChange() and pass TRUE to indicate key has been PRESSED and not RELEASED
          KeyChange(usParam, uiParam, TRUE);
        }
      }
    }
  }
}

void KeyUp(UINT32 usParam, UINT32 uiParam)
{ // Are we RELEASING one of SHIFT, ALT or CTRL ???
  if (usParam == 16)
  { // SHIFT key is RELEASED
    gfShiftState = FALSE;
    gfKeyState[16] = FALSE;
  }
  else
  {
    if (usParam == 17)
    { // CTRL key is RELEASED
      gfCtrlState = FALSE;
      gfKeyState[17] = FALSE;
    }
    else
    {
      if (usParam == 18)
      { // ALT key is RELEASED
        gfAltState = FALSE;
        gfKeyState[18] = FALSE;
      }
      else
      {
        if (usParam == SNAPSHOT)
        {
					// DB this used to be keyed to SCRL_LOCK
					// which I believe Luis gave the wrong value
					//#ifndef JA2
					if (_KeyDown(CTRL))
						VideoCaptureToggle();
					else
					//#endif
						PrintScreen();
        }
        else
        {
					// No special keys have been pressed
					// Call KeyChange() and pass FALSE to indicate key has been PRESSED and not RELEASED
					KeyChange(usParam, uiParam, FALSE);
        }
      }
    }
  }
}

void EnableDoubleClk(void)
{
	// Obsolete
}

void DisableDoubleClk(void)
{
	// Obsolete
}

void GetMousePos(SGPPoint *Point)
{
  POINT MousePos;

  GetCursorPos(&MousePos);

  Point->iX = (UINT32) MousePos.x;
  Point->iY = (UINT32) MousePos.y;
}

// These functions will be used for string input

// Since all string input will have to be handle by reentrant capable functions (since we must attend
// to windows messaging as well as network traffic related issues), whenever there is ongoing string input
// going on, we must use InitStringInput() and HandleStringInput() to get the job done. HandleStringInput()
// will return TRUE as long as the string input is going on, and FALSE when its done
//
// During string input, all keyboard are rerouted to the string and hence are not queued up on the
// event queue or registered in the state table. Also note that several string inputs can occur
// at the same time. Use the SetStringFocus() function to manager the focus for multiple
// string inputs

StringInput *InitStringInput(UINT16 *pInputString, UINT16 usLength, UINT16 *pFilter)
{
  StringInput *pStringDescriptor;

  if ((pStringDescriptor = MemAlloc(sizeof(StringInput))) == NULL)
  {
    //
    // Hum we failed to allocate memory for the string descriptor
    //

    DbgMessage(TOPIC_INPUT, DBG_LEVEL_1, "Failed to allocate memory for string descriptor");
    return NULL;
  }
  else
  {
    if ((pStringDescriptor->pOriginalString = MemAlloc(usLength * 2)) == NULL)
    {
      //
      // free up structure before aborting
      //

      MemFree(pStringDescriptor);
      DbgMessage(TOPIC_INPUT, DBG_LEVEL_1, "Failed to allocate memory for string duplicate");
      return NULL;
    }

    memcpy(pStringDescriptor->pOriginalString, pInputString, usLength * 2);

    pStringDescriptor->pString = pInputString;
    pStringDescriptor->pFilter = pFilter;
    pStringDescriptor->usMaxStringLength = usLength;
    pStringDescriptor->usStringOffset = 0;
    pStringDescriptor->usCurrentStringLength = 0;
    while ((pStringDescriptor->usStringOffset < pStringDescriptor->usMaxStringLength)&&(*(pStringDescriptor->pString + pStringDescriptor->usStringOffset) != 0))
    {
      //
      // Find the last character in the string
      //

      pStringDescriptor->usStringOffset++;
      pStringDescriptor->usCurrentStringLength++;
    }

    if (pStringDescriptor->usStringOffset == pStringDescriptor->usMaxStringLength)
    {
      //
      // Hum the current string has no null terminator. Invalidate the string and
      // start from scratch
      //

      memset(pStringDescriptor->pString, 0, usLength * 2);
      pStringDescriptor->usStringOffset = 0;
      pStringDescriptor->usCurrentStringLength = 0;
    }

    pStringDescriptor->fInsertMode = FALSE;
    pStringDescriptor->fFocus = FALSE;
    pStringDescriptor->pPreviousString = NULL;
    pStringDescriptor->pNextString = NULL;

    return pStringDescriptor;
  }
}

void    LinkPreviousString(StringInput *pCurrentString, StringInput *pPreviousString)
{
  if (pCurrentString != NULL)
  {
    if (pCurrentString->pPreviousString != NULL)
    {
      pCurrentString->pPreviousString->pNextString = NULL;
    }

    pCurrentString->pPreviousString = pPreviousString;

    if (pPreviousString != NULL)
    {
      pPreviousString->pNextString = pCurrentString;
    }
  }
}

void    LinkNextString(StringInput *pCurrentString, StringInput *pNextString)
{
  if (pCurrentString != NULL)
  {
    if (pCurrentString->pNextString != NULL)
    {
      pCurrentString->pNextString->pPreviousString = NULL;
    }

    pCurrentString->pNextString = pNextString;

    if (pNextString != NULL)
    {
      pNextString->pPreviousString = pCurrentString;
    }
  }
}

BOOLEAN CharacterIsValid(UINT16 usCharacter, UINT16 *pFilter)
{
  UINT32 uiIndex, uiEndIndex;

  if (pFilter != NULL)
  {
    uiEndIndex = *pFilter;
	  for (uiIndex = 1; uiIndex <= *pFilter; uiIndex++)
	  {
      if (usCharacter == *(pFilter + uiIndex))
      {
        return TRUE;
      }
	  }
    return FALSE;
  }

  return TRUE;
}

void    RedirectToString(UINT16 usInputCharacter)
{
  UINT16 usIndex;

  if (gpCurrentStringDescriptor != NULL)
  {
    // Handle the new character input
    switch (usInputCharacter)
    {
      case ENTER
      : // ENTER is pressed, the last character field should be set to ENTER
        if (gpCurrentStringDescriptor->pNextString != NULL)
        {
          gpCurrentStringDescriptor->fFocus = FALSE;
          gpCurrentStringDescriptor = gpCurrentStringDescriptor->pNextString;
          gpCurrentStringDescriptor->fFocus = TRUE;
          gpCurrentStringDescriptor->usLastCharacter = 0;
        }
        else
        {
          gpCurrentStringDescriptor->fFocus = FALSE;
          gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
          gfCurrentStringInputState = FALSE;
        }
        break;
      case ESC
      : // ESC was pressed, the last character field should be set to ESC
        gpCurrentStringDescriptor->fFocus = FALSE;
        gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        gfCurrentStringInputState = FALSE;
        break;
      case SHIFT_TAB
      : // TAB was pressed, the last character field should be set to TAB
        if (gpCurrentStringDescriptor->pPreviousString != NULL)
        {
          gpCurrentStringDescriptor->fFocus = FALSE;
          gpCurrentStringDescriptor = gpCurrentStringDescriptor->pPreviousString;
          gpCurrentStringDescriptor->fFocus = TRUE;
          gpCurrentStringDescriptor->usLastCharacter = 0;
        }
        break;
      case TAB
      : // TAB was pressed, the last character field should be set to TAB
        if (gpCurrentStringDescriptor->pNextString != NULL)
        {
          gpCurrentStringDescriptor->fFocus = FALSE;
          gpCurrentStringDescriptor = gpCurrentStringDescriptor->pNextString;
          gpCurrentStringDescriptor->fFocus = TRUE;
          gpCurrentStringDescriptor->usLastCharacter = 0;
        }
        break;
      case UPARROW
      : // The UPARROW was pressed, the last character field should be set to UPARROW
        if (gpCurrentStringDescriptor->pPreviousString != NULL)
        {
          gpCurrentStringDescriptor->fFocus = FALSE;
          gpCurrentStringDescriptor = gpCurrentStringDescriptor->pPreviousString;
          gpCurrentStringDescriptor->fFocus = TRUE;
          gpCurrentStringDescriptor->usLastCharacter = 0;
        }
        break;
      case DNARROW
      : // The DNARROW was pressed, the last character field should be set to DNARROW
        if (gpCurrentStringDescriptor->pNextString != NULL)
        {
          gpCurrentStringDescriptor->fFocus = FALSE;
          gpCurrentStringDescriptor = gpCurrentStringDescriptor->pNextString;
          gpCurrentStringDescriptor->fFocus = TRUE;
          gpCurrentStringDescriptor->usLastCharacter = 0;
        }
        break;
      case LEFTARROW
      : // The LEFTARROW was pressed, move one character to the left
        if (gpCurrentStringDescriptor->usStringOffset > 0)
        { // Decrement the offset
          gpCurrentStringDescriptor->usStringOffset--;
        }
        gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        break;
      case RIGHTARROW
      : // The RIGHTARROW was pressed, move one character to the right
        if (gpCurrentStringDescriptor->usStringOffset < gpCurrentStringDescriptor->usCurrentStringLength)
        { // Ok we can move the cursor one up without going past the end of string
          gpCurrentStringDescriptor->usStringOffset++;
        }
        gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        break;
      case BACKSPACE
      : // Delete the character preceding the cursor
        if (gpCurrentStringDescriptor->usStringOffset > 0)
        { // Ok, we are not at the beginning of the string, so we may proceed
          for (usIndex = gpCurrentStringDescriptor->usStringOffset; usIndex <= gpCurrentStringDescriptor->usCurrentStringLength; usIndex++)
          { // Shift the characters one at a time
            *(gpCurrentStringDescriptor->pString + usIndex - 1) = *(gpCurrentStringDescriptor->pString + usIndex);
          }
          gpCurrentStringDescriptor->usStringOffset--;
          gpCurrentStringDescriptor->usCurrentStringLength--;
        }

        break;
      case DEL
      : // Delete the character which follows the cursor
        if (gpCurrentStringDescriptor->usStringOffset < gpCurrentStringDescriptor->usCurrentStringLength)
        { // Ok we are not at the end of the string, so we may proceed
          for (usIndex = gpCurrentStringDescriptor->usStringOffset; usIndex < gpCurrentStringDescriptor->usCurrentStringLength; usIndex++)
          { // Shift the characters one at a time
            *(gpCurrentStringDescriptor->pString + usIndex) = *(gpCurrentStringDescriptor->pString + usIndex + 1);
          }
          gpCurrentStringDescriptor->usCurrentStringLength--;
        }
        gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        break;
      case INSERT
      : // Toggle insert mode
        if (gpCurrentStringDescriptor->fInsertMode == TRUE)
        {
          gpCurrentStringDescriptor->fInsertMode = FALSE;
        }
        else
        {
          gpCurrentStringDescriptor->fInsertMode = TRUE;
        }
        gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        break;
      case HOME
      : // Go to the beginning of the input string
        gpCurrentStringDescriptor->usStringOffset = 0 ;
        gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        break;
#ifndef JA2
// Stupid definition causes problems with headers that use the keyword END -- DB
		case KEY_END
#else
		case END
#endif
      : // Go to the end of the input string
        gpCurrentStringDescriptor->usStringOffset = gpCurrentStringDescriptor->usCurrentStringLength;
        gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        break;
      default
      : //
        // normal input
        //
        if (CharacterIsValid(usInputCharacter, gpCurrentStringDescriptor->pFilter) == TRUE)
        {
          if (gpCurrentStringDescriptor->fInsertMode == TRUE)
          { // Before we can shift characters for the insert, we must make sure we have the space
            if (gpCurrentStringDescriptor->usCurrentStringLength < (gpCurrentStringDescriptor->usMaxStringLength - 1))
            { // Before we can add a new character we must shift existing ones to for the insert
              for (usIndex = gpCurrentStringDescriptor->usCurrentStringLength; usIndex > gpCurrentStringDescriptor->usStringOffset; usIndex--)
              { // Shift the characters one at a time
                *(gpCurrentStringDescriptor->pString + usIndex) = *(gpCurrentStringDescriptor->pString + usIndex - 1);
              }
              // Ok now we introduce the new character
              *(gpCurrentStringDescriptor->pString + usIndex) = usInputCharacter;
              gpCurrentStringDescriptor->usStringOffset++;
              gpCurrentStringDescriptor->usCurrentStringLength++;
            }
          }
          else
          {
            // Ok, add character to string (by overwriting)
            if (gpCurrentStringDescriptor->usStringOffset < (gpCurrentStringDescriptor->usMaxStringLength - 1))
            { // Ok, we have not exceeded the maximum number of characters yet
              *(gpCurrentStringDescriptor->pString + gpCurrentStringDescriptor->usStringOffset) = usInputCharacter;
              gpCurrentStringDescriptor->usStringOffset++;
            }
            // Did we push back the current string length (i.e. add character to end of string)
            if (gpCurrentStringDescriptor->usStringOffset > gpCurrentStringDescriptor->usCurrentStringLength)
            { // Add a NULL character
              *(gpCurrentStringDescriptor->pString + gpCurrentStringDescriptor->usStringOffset) = 0;
              gpCurrentStringDescriptor->usCurrentStringLength++;
            }
          }
          gpCurrentStringDescriptor->usLastCharacter = usInputCharacter;
        }
        break;
    }
  }
}

UINT16 GetStringInputState(void)
{
  if (gpCurrentStringDescriptor != NULL)
  {
    return gpCurrentStringDescriptor->usLastCharacter;
  }
  else
  {
    return 0;
  }
}

BOOLEAN StringInputHasFocus(void)
{
  return gfCurrentStringInputState;
}

BOOLEAN SetStringFocus(StringInput *pStringDescriptor)
{
  if (pStringDescriptor != NULL)
  {
    if (gpCurrentStringDescriptor != NULL)
    {
      gpCurrentStringDescriptor->fFocus = FALSE;
    }
    // Ok overide current entry
    gfCurrentStringInputState = TRUE;
    gpCurrentStringDescriptor = pStringDescriptor;
    gpCurrentStringDescriptor->fFocus = TRUE;
    gpCurrentStringDescriptor->usLastCharacter = 0;
    return TRUE;
  }
  else
  {
    if (gpCurrentStringDescriptor != NULL)
    {
      gpCurrentStringDescriptor->fFocus = FALSE;
    }
    // Ok overide current entry
    gfCurrentStringInputState = FALSE;
    gpCurrentStringDescriptor = NULL;
    return TRUE;
  }
}

UINT16 GetCursorPositionInString(StringInput *pStringDescriptor)
{
  return pStringDescriptor->usStringOffset;
}

BOOLEAN StringHasFocus(StringInput *pStringDescriptor)
{
  if (pStringDescriptor != NULL)
  {
    return pStringDescriptor->fFocus;
  }
  else
  {
    return FALSE;
  }
}

void RestoreString(StringInput *pStringDescriptor)
{
  memcpy(pStringDescriptor->pString, pStringDescriptor->pOriginalString, pStringDescriptor->usMaxStringLength * 2);

  pStringDescriptor->usStringOffset = 0;
  pStringDescriptor->usCurrentStringLength = 0;
  while ((pStringDescriptor->usStringOffset < pStringDescriptor->usMaxStringLength)&&(*(pStringDescriptor->pString + pStringDescriptor->usStringOffset) != 0))
  {
    //
    // Find the last character in the string
    //

    pStringDescriptor->usStringOffset++;
    pStringDescriptor->usCurrentStringLength++;
  }

  if (pStringDescriptor->usStringOffset == pStringDescriptor->usMaxStringLength)
  {
    //
    // Hum the current string has no null terminator. Invalidate the string and
    // start from scratch
    //
    memset(pStringDescriptor->pString, 0, pStringDescriptor->usMaxStringLength * 2);
    pStringDescriptor->usStringOffset = 0;
    pStringDescriptor->usCurrentStringLength = 0;
  }

  pStringDescriptor->fInsertMode = FALSE;
}

void EndStringInput(StringInput *pStringDescriptor)
{ // Make sure we have a valid pStringDescriptor
  if (pStringDescriptor != NULL)
  { // make sure the gpCurrentStringDescriptor is NULL if necessary
    if (pStringDescriptor == gpCurrentStringDescriptor)
    {
      gpCurrentStringDescriptor = NULL;
      gfCurrentStringInputState = FALSE;
    }
    // Make sure we have a valid string within the string descriptor
    if (pStringDescriptor->pOriginalString != NULL)
    { // free up the string
      MemFree(pStringDescriptor->pOriginalString);
    }
    // free up the descriptor
    MemFree(pStringDescriptor);
  }
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



BOOLEAN InputEventInside(InputAtom *Event, UINT32 uiX1, UINT32 uiY1, UINT32 uiX2, UINT32 uiY2)
{
  UINT32 uiEventX, uiEventY;

  uiEventX = _EvMouseX(Event);
  uiEventY = _EvMouseY(Event);

  return((uiEventX >= uiX1) && (uiEventX <= uiX2) && (uiEventY >= uiY1) && (uiEventY <= uiY2));
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
      UINT32 uiTmpLParam;
      POINT  MousePos;

      GetCursorPos(&MousePos);
      uiTmpLParam = ((MousePos.y << 16) & 0xffff0000) | (MousePos.x & 0x0000ffff);
      QueueEvent(LEFT_BUTTON_REPEAT, 0, uiTmpLParam);
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
      UINT32 uiTmpLParam;
      POINT  MousePos;

      GetCursorPos(&MousePos);
      uiTmpLParam = ((MousePos.y << 16) & 0xffff0000) | (MousePos.x & 0x0000ffff);
      QueueEvent(RIGHT_BUTTON_REPEAT, 0, uiTmpLParam);
      guiRightButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
    }
  }
  else
  {
    guiRightButtonRepeatTimer = 0;
  }
}
