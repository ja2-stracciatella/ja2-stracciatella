#include "Types.h"
#include "Input.h"
#include "MemMan.h"
#include "English.h"
#include "Timer.h"
#include "Video.h"
#include "Local.h"


// The gfKeyState table is used to track which of the keys is up or down at any one time. This is used while polling
// the interface.

BOOLEAN gfKeyState[SDLK_LAST]; // TRUE = Pressed, FALSE = Not Pressed
static BOOLEAN fCursorWasClipped = FALSE;
static SGPRect gCursorClipRect;


// These data structure are used to track the mouse while polling

static UINT32 guiSingleClickTimer;

static UINT32 guiLeftButtonRepeatTimer;
static UINT32 guiRightButtonRepeatTimer;

BOOLEAN gfLeftButtonState;  // TRUE = Pressed, FALSE = Not Pressed
BOOLEAN gfRightButtonState; // TRUE = Pressed, FALSE = Not Pressed
UINT16  gusMouseXPos;       // X position of the mouse on screen
UINT16  gusMouseYPos;       // y position of the mouse on screen

// The queue structures are used to track input events using queued events

static InputAtom gEventQueue[256];
static UINT16    gusQueueCount;
static UINT16    gusHeadIndex;
static UINT16    gusTailIndex;


static void QueueMouseEvent(UINT16 ubInputEvent)
{
	// Can we queue up one more event, if not, the event is lost forever
	if (gusQueueCount == lengthof(gEventQueue)) return;

	gEventQueue[gusTailIndex].usEvent = ubInputEvent;

	gusQueueCount++;

	gusTailIndex = (gusTailIndex + 1) % lengthof(gEventQueue);
}


static void QueueKeyEvent(UINT16 ubInputEvent, SDLKey Key, SDLMod Mod, wchar_t Char)
{
	// Can we queue up one more event, if not, the event is lost forever
	if (gusQueueCount == lengthof(gEventQueue)) return;

	UINT16 ModifierState = 0;
	if (Mod & KMOD_SHIFT) ModifierState |= SHIFT_DOWN;
	if (Mod & KMOD_CTRL)  ModifierState |= CTRL_DOWN;
	if (Mod & KMOD_ALT)   ModifierState |= ALT_DOWN;
	gEventQueue[gusTailIndex].usKeyState = ModifierState;
	gEventQueue[gusTailIndex].usEvent = ubInputEvent;
	gEventQueue[gusTailIndex].usParam = Key;
	gEventQueue[gusTailIndex].Char    = Char;

	gusQueueCount++;

	gusTailIndex = (gusTailIndex + 1) % lengthof(gEventQueue);
}


BOOLEAN DequeueSpecificEvent(InputAtom* Event, UINT32 uiMaskFlags)
{
	// Is there an event to dequeue
	if (gusQueueCount > 0)
	{
		// Check if it has the masks!
		if (gEventQueue[gusHeadIndex].usEvent & uiMaskFlags)
		{
			return DequeueEvent(Event);
		}
	}

	return FALSE;
}


static void HandleSingleClicksAndButtonRepeats(void);


BOOLEAN DequeueEvent(InputAtom* Event)
{
	HandleSingleClicksAndButtonRepeats();

	if (gusQueueCount == 0) return FALSE;

	*Event = gEventQueue[gusHeadIndex];
	gusHeadIndex = (gusHeadIndex + 1) % lengthof(gEventQueue);
	gusQueueCount--;
	return TRUE;
}


static void UpdateMousePos(const SDL_MouseButtonEvent* BtnEv)
{
	gusMouseXPos = BtnEv->x;
	gusMouseYPos = BtnEv->y;
}


#if defined WITH_MAEMO
static BOOLEAN g_down_right;
#endif


void MouseButtonDown(const SDL_MouseButtonEvent* BtnEv)
{
	UpdateMousePos(BtnEv);
	switch (BtnEv->button)
	{
		case SDL_BUTTON_LEFT:
		{
#if defined WITH_MAEMO
			/* If the menu button (mapped to F4) is pressed, then treat the event as
			 * right click */
			const Uint8* const key_state = SDL_GetKeyState(NULL);
			g_down_right = key_state[SDLK_F4];
			if (g_down_right) goto right_button;
#endif
			guiLeftButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfLeftButtonState = TRUE;
			QueueMouseEvent(LEFT_BUTTON_DOWN);
			break;
		}

		case SDL_BUTTON_RIGHT:
#if defined WITH_MAEMO
right_button:
#endif
			guiRightButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfRightButtonState = TRUE;
			QueueMouseEvent(RIGHT_BUTTON_DOWN);
			break;

		case SDL_BUTTON_WHEELUP:   QueueMouseEvent(MOUSE_WHEEL_UP);   break;
		case SDL_BUTTON_WHEELDOWN: QueueMouseEvent(MOUSE_WHEEL_DOWN); break;
	}
}


void MouseButtonUp(const SDL_MouseButtonEvent* BtnEv)
{
	UpdateMousePos(BtnEv);
	switch (BtnEv->button)
	{
		case SDL_BUTTON_LEFT:
		{
#if defined WITH_MAEMO
			if (g_down_right) goto right_button;
#endif
			guiLeftButtonRepeatTimer = 0;
			gfLeftButtonState = FALSE;
			QueueMouseEvent(LEFT_BUTTON_UP);
			UINT32 uiTimer = GetClock();
			if (uiTimer - guiSingleClickTimer < DBL_CLK_TIME)
			{
				QueueMouseEvent(LEFT_BUTTON_DBL_CLK);
			}
			else
			{
				guiSingleClickTimer = uiTimer;
			}
			break;
		}

		case SDL_BUTTON_RIGHT:
#if defined WITH_MAEMO
right_button:
#endif
			guiRightButtonRepeatTimer = 0;
			gfRightButtonState = FALSE;
			QueueMouseEvent(RIGHT_BUTTON_UP);
			break;
	}
}


static void KeyChange(SDL_keysym const* const key_sym, bool const pressed)
{
	SDLKey       key = key_sym->sym;
	SDLMod const mod = key_sym->mod;
	bool   const num = mod & KMOD_NUM;
	switch (key)
	{
#if defined WITH_MAEMO
		/* Use the menu button (mapped to F4) as modifier for right click */
		case SDLK_F4: return;
#endif

		case SDLK_KP0:         key = num ? SDLK_0      : SDLK_INSERT;   break;
		case SDLK_KP1:         key = num ? SDLK_1      : SDLK_END;      break;
		case SDLK_KP2:         key = num ? SDLK_2      : SDLK_DOWN;     break;
		case SDLK_KP3:         key = num ? SDLK_3      : SDLK_PAGEDOWN; break;
		case SDLK_KP4:         key = num ? SDLK_4      : SDLK_LEFT;     break;
		case SDLK_KP5:
			if (!num) return;
			key = SDLK_5;
			break;
		case SDLK_KP6:         key = num ? SDLK_6      : SDLK_RIGHT;    break;
		case SDLK_KP7:         key = num ? SDLK_7      : SDLK_HOME;     break;
		case SDLK_KP8:         key = num ? SDLK_8      : SDLK_UP;       break;
		case SDLK_KP9:         key = num ? SDLK_9      : SDLK_PAGEUP;   break;
		case SDLK_KP_PERIOD:   key = num ? SDLK_PERIOD : SDLK_DELETE;   break;
		case SDLK_KP_DIVIDE:   key = SDLK_SLASH;                        break;
		case SDLK_KP_MULTIPLY: key = SDLK_ASTERISK;                     break;
		case SDLK_KP_MINUS:    key = SDLK_MINUS;                        break;
		case SDLK_KP_PLUS:     key = SDLK_PLUS;                         break;
		case SDLK_KP_ENTER:    key = SDLK_RETURN;                       break;

		default:
			if (key >= lengthof(gfKeyState)) return;
			break;
	}

	UINT     event_type;
	BOOLEAN& key_state = gfKeyState[key];
	if (pressed)
	{
		event_type = key_state ? KEY_REPEAT : KEY_DOWN;
	}
	else
	{
		if (!key_state) return;
		event_type = KEY_UP;
	}
	key_state = pressed;
	QueueKeyEvent(event_type, key, mod, key_sym->unicode);
}


void KeyDown(const SDL_keysym* KeySym)
{
	switch (KeySym->sym)
	{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			_KeyDown(SHIFT) = TRUE;
			break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			_KeyDown(CTRL) = TRUE;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
			_KeyDown(ALT) = TRUE;
			break;

		case SDLK_PRINT:
		case SDLK_SCROLLOCK:
			break;

		default:
			KeyChange(KeySym, true);
			break;
	}
}


void KeyUp(const SDL_keysym* KeySym)
{
	switch (KeySym->sym)
	{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			_KeyDown(SHIFT) = FALSE;
			break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			_KeyDown(CTRL) = FALSE;
			break;

		case SDLK_LALT:
		case SDLK_RALT:
			_KeyDown(ALT) = FALSE;
			break;

		case SDLK_PRINT:
			if (KeySym->mod & KMOD_CTRL) VideoCaptureToggle(); else PrintScreen();
			break;

		case SDLK_SCROLLOCK:
			SDL_WM_GrabInput
			(
				SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF ?
					SDL_GRAB_ON : SDL_GRAB_OFF
			);
			break;

		case SDLK_RETURN:
			if (_KeyDown(ALT))
			{
				VideoToggleFullScreen();
				break;
			}
			/* FALLTHROUGH */

		default:
			KeyChange(KeySym, false);
			break;
	}
}


void GetMousePos(SGPPoint* Point)
{
	int x;
	int y;
	SDL_GetMouseState(&x, &y);
	Point->iX = x;
	Point->iY = y;
}


void RestrictMouseToXYXY(UINT16 usX1, UINT16 usY1, UINT16 usX2, UINT16 usY2)
{
	SGPRect TempRect;
	TempRect.iLeft   = usX1;
	TempRect.iTop    = usY1;
	TempRect.iRight  = usX2;
	TempRect.iBottom = usY2;
	RestrictMouseCursor(&TempRect);
}


void RestrictMouseCursor(const SGPRect* pRectangle)
{
	// Make a copy of our rect....
	gCursorClipRect = *pRectangle;
#if 1 // XXX TODO0000 Should probably removed completly. Confining the mouse cursor is The Wrong Thing(tm)
#else
	ClipCursor((RECT*)pRectangle);
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


void RestoreCursorClipRect(void)
{
#if 1 // XXX TODO0000
	UNIMPLEMENTED
#else
	if (fCursorWasClipped)
	{
		ClipCursor(&gCursorClipRect);
	}
#endif
}


void GetRestrictedClipCursor(SGPRect* pRectangle)
{
#if 1 // XXX TODO0000
	pRectangle->iLeft   = 0;
	pRectangle->iTop    = 0;
	pRectangle->iRight  = g_screen_width;
	pRectangle->iBottom = g_screen_height;
#else
	GetClipCursor((RECT *) pRectangle );
#endif
}


BOOLEAN IsCursorRestricted(void)
{
	return fCursorWasClipped;
}


void SimulateMouseMovement( UINT32 uiNewXPos, UINT32 uiNewYPos )
{
	// Wizardry NOTE: This function currently doesn't quite work right for in any Windows resolution other than 640x480.
	// mouse_event() uses your current Windows resolution to calculate the resulting x,y coordinates.  So in order to get
	// the right coordinates, you'd have to find out the current Windows resolution through a system call, and then do:
	//		uiNewXPos = uiNewXPos * g_screen_width  / WinScreenResX;
	//		uiNewYPos = uiNewYPos * g_screen_height / WinScreenResY;
	//
	// JA2 doesn't have this problem, 'cause they use DirectDraw calls that change the Windows resolution properly.
	//
	// Alex Meduna, Dec. 3, 1997

#if 1
	FIXME
	SDL_WarpMouse(uiNewXPos, uiNewYPos);
#else
	// Adjust coords based on our resolution
	FLOAT flNewXPos = (FLOAT)uiNewXPos / g_screen_width  * 65536;
	FLOAT flNewYPos = (FLOAT)uiNewYPos / g_screen_height * 65536;

	mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE, (UINT32)flNewXPos, (UINT32)flNewYPos, 0, 0);
#endif
}


void DequeueAllKeyBoardEvents(void)
{
#if 1 // XXX TODO
	FIXME
#else
	//dequeue all the events waiting in the windows queue
	MSG KeyMessage;
	while (PeekMessage(&KeyMessage, ghWindow, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE));

	//Deque all the events waiting in the SGP queue
	InputAtom InputEvent;
	while (DequeueEvent(&InputEvent))
  {
		//dont do anything
	}
#endif
}


static void HandleSingleClicksAndButtonRepeats(void)
{
	UINT32 uiTimer = GetClock();

	// Is there a LEFT mouse button repeat
	if (gfLeftButtonState)
	{
		if ((guiLeftButtonRepeatTimer > 0)&&(guiLeftButtonRepeatTimer <= uiTimer))
		{
			QueueMouseEvent(LEFT_BUTTON_REPEAT);
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
			QueueMouseEvent(RIGHT_BUTTON_REPEAT);
			guiRightButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
		}
	}
	else
	{
		guiRightButtonRepeatTimer = 0;
	}
}
