#include "UTF8String.h"
#include "Types.h"
#include "Input.h"
#include "MemMan.h"
#include "English.h"
#include "Timer.h"
#include "Video.h"
#include "Local.h"
#include "UILayout.h"


// The gfKeyState table is used to track which of the keys is up or down at any one time. This is used while polling
// the interface.

BOOLEAN gfKeyState[SDL_NUM_SCANCODES]; // TRUE = Pressed, FALSE = Not Pressed
static BOOLEAN fCursorWasClipped = FALSE;
static SGPRect gCursorClipRect;


// These data structure are used to track the mouse while polling

static UINT32 guiSingleClickTimer;

static UINT32 guiLeftButtonRepeatTimer;
static UINT32 guiRightButtonRepeatTimer;
static UINT32 guiMiddleButtonRepeatTimer;

BOOLEAN gfLeftButtonState;  // TRUE = Pressed, FALSE = Not Pressed
BOOLEAN gfRightButtonState; // TRUE = Pressed, FALSE = Not Pressed
BOOLEAN gfMiddleButtonState;// TRUE = Pressed, FALSE = Not Pressed
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


static void QueueKeyEvent(UINT16 ubInputEvent, SDL_Keycode Key, SDL_Keymod Mod, wchar_t Char)
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

void SetSafeMousePosition(int x, int y) {
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x > SCREEN_WIDTH) x = SCREEN_WIDTH;
	if (y > SCREEN_HEIGHT) y = SCREEN_HEIGHT;

	gusMouseXPos = x;
	gusMouseYPos = y;
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
	SetSafeMousePosition(BtnEv->x, BtnEv->y);
}


#if defined(WITH_MAEMO) || defined __APPLE__
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
			const Uint8* const key_state = SDL_GetKeyboardState(NULL);
			g_down_right = key_state[SDL_SCANCODE_F4];
			if (g_down_right) goto right_button;
#endif
#if defined(__APPLE__)
			const Uint8* const key_state = SDL_GetKeyboardState(NULL);
			g_down_right = key_state[SDL_SCANCODE_LGUI] || key_state[SDL_SCANCODE_RGUI];
			if (g_down_right) goto right_button;
#endif
			guiLeftButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfLeftButtonState = TRUE;
			QueueMouseEvent(LEFT_BUTTON_DOWN);
			break;
		}

		case SDL_BUTTON_RIGHT:
#if defined(WITH_MAEMO) || defined(__APPLE__)
right_button:
#endif
			guiRightButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfRightButtonState = TRUE;
			QueueMouseEvent(RIGHT_BUTTON_DOWN);
			break;

		case SDL_BUTTON_MIDDLE:
			guiMiddleButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfMiddleButtonState = TRUE;
			QueueMouseEvent(MIDDLE_BUTTON_DOWN);
			break;
	}
}


void MouseButtonUp(const SDL_MouseButtonEvent* BtnEv)
{
	UpdateMousePos(BtnEv);
	switch (BtnEv->button)
	{
		case SDL_BUTTON_LEFT:
		{
#if defined(WITH_MAEMO) || defined(__APPLE__)
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
#if defined WITH_MAEMO || defined(__APPLE__)
right_button:
#endif
			guiRightButtonRepeatTimer = 0;
			gfRightButtonState = FALSE;
			QueueMouseEvent(RIGHT_BUTTON_UP);
			break;

		case SDL_BUTTON_MIDDLE:
			guiMiddleButtonRepeatTimer = 0;
			gfMiddleButtonState = FALSE;
			QueueMouseEvent(MIDDLE_BUTTON_UP);
			break;
	}
}

void MouseWheelScroll(const SDL_MouseWheelEvent* WheelEv)
{
    if (WheelEv->y > 0)
    {
    	QueueMouseEvent(MOUSE_WHEEL_UP);
    }
    else
    {
        QueueMouseEvent(MOUSE_WHEEL_DOWN);
    }
}



static void KeyChange(SDL_Keysym const* const key_sym, bool const pressed)
{
	// Note: We use the SDL_Scancode as an index for gfKeyState, but use
	// the SDL_Keycode for everything else.
	SDL_Scancode scancode = key_sym->scancode;
	SDL_Keymod const mod = (SDL_Keymod) key_sym->mod;
	bool   const num = mod & KMOD_NUM;
	// Handle special keys that do not have a scancode in SDL.
	switch (key_sym->sym)
	{
		case SDLK_ASTERISK:		scancode = SCANCODE_ASTERISK;	break;
		case SDLK_PLUS:			scancode = SCANCODE_PLUS;		break;
		case SDLK_UNDERSCORE:	scancode = SCANCODE_UNDERSCORE;	break;
		default: break;
	}

    switch (scancode)
	{
		case SDL_SCANCODE_KP_0:			scancode = num ? SDL_SCANCODE_0			: SDL_SCANCODE_INSERT;	break;
		case SDL_SCANCODE_KP_1:			scancode = num ? SDL_SCANCODE_1			: SDL_SCANCODE_END;		break;
		case SDL_SCANCODE_KP_2:			scancode = num ? SDL_SCANCODE_2			: SDL_SCANCODE_DOWN;	break;
		case SDL_SCANCODE_KP_3:			scancode = num ? SDL_SCANCODE_3			: SDL_SCANCODE_PAGEDOWN;break;
		case SDL_SCANCODE_KP_4:			scancode = num ? SDL_SCANCODE_4			: SDL_SCANCODE_LEFT;	break;
		case SDL_SCANCODE_KP_5:
			if (!num) return;
			scancode = SDL_SCANCODE_5;
			break;
		case SDL_SCANCODE_KP_6:			scancode = num ? SDL_SCANCODE_6			: SDL_SCANCODE_RIGHT;	break;
		case SDL_SCANCODE_KP_7:			scancode = num ? SDL_SCANCODE_7			: SDL_SCANCODE_HOME;	break;
		case SDL_SCANCODE_KP_8:			scancode = num ? SDL_SCANCODE_8			: SDL_SCANCODE_UP;		break;
		case SDL_SCANCODE_KP_9:			scancode = num ? SDL_SCANCODE_9			: SDL_SCANCODE_PAGEUP;	break;
		case SDL_SCANCODE_KP_PERIOD:	scancode = num ? SDL_SCANCODE_PERIOD	: SDL_SCANCODE_DELETE;	break;
		case SDL_SCANCODE_KP_DIVIDE:	scancode = SDL_SCANCODE_SLASH;									break;
		case SDL_SCANCODE_KP_MULTIPLY:	scancode = SCANCODE_ASTERISK;									break;
		case SDL_SCANCODE_KP_MINUS:		scancode = SDL_SCANCODE_MINUS;									break;
		case SDL_SCANCODE_KP_PLUS:		scancode = SCANCODE_PLUS;										break;
		case SDL_SCANCODE_KP_ENTER:		scancode = SDL_SCANCODE_RETURN;									break;

		default:
			if (scancode >= lengthof(gfKeyState)) return;
			break;
	}



	SDL_Keycode      key = key_sym->sym;
	switch (key)
	{
#if defined WITH_MAEMO
		/* Use the menu button (mapped to F4) as modifier for right click */
		case SDLK_F4: return;
#endif

		case SDLK_KP_0:         key = num ? SDLK_0      : SDLK_INSERT;   break;
		case SDLK_KP_1:         key = num ? SDLK_1      : SDLK_END;      break;
		case SDLK_KP_2:         key = num ? SDLK_2      : SDLK_DOWN;     break;
		case SDLK_KP_3:         key = num ? SDLK_3      : SDLK_PAGEDOWN; break;
		case SDLK_KP_4:         key = num ? SDLK_4      : SDLK_LEFT;     break;
		case SDLK_KP_5:
			if (!num) return;
			key = SDLK_5;
			break;
		case SDLK_KP_6:         key = num ? SDLK_6      : SDLK_RIGHT;    break;
		case SDLK_KP_7:         key = num ? SDLK_7      : SDLK_HOME;     break;
		case SDLK_KP_8:         key = num ? SDLK_8      : SDLK_UP;       break;
		case SDLK_KP_9:         key = num ? SDLK_9      : SDLK_PAGEUP;   break;
		case SDLK_KP_PERIOD:   key = num ? SDLK_PERIOD : SDLK_DELETE;   break;
		case SDLK_KP_DIVIDE:   key = SDLK_SLASH;                        break;
		case SDLK_KP_MULTIPLY: key = SDLK_ASTERISK;                     break;
		case SDLK_KP_MINUS:    key = SDLK_MINUS;                        break;
		case SDLK_KP_PLUS:     key = SDLK_PLUS;                         break;
		case SDLK_KP_ENTER:    key = SDLK_RETURN;                       break;

		default:
			break;
	}

	UINT     event_type;
	BOOLEAN& key_state = gfKeyState[scancode];
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

	QueueKeyEvent(event_type, key, mod, '\0');
}


void KeyDown(const SDL_Keysym* KeySym)
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

		case SDLK_PRINTSCREEN:
		case SDLK_SCROLLLOCK:
			break;

		default:
			KeyChange(KeySym, true);
			break;
	}
}


void KeyUp(const SDL_Keysym* KeySym)
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

		case SDLK_PRINTSCREEN:
			PrintScreen();
			break;

		case SDLK_SCROLLLOCK:
			SDL_SetWindowGrab
			(
			    GAME_WINDOW,
				SDL_GetWindowGrab(GAME_WINDOW) == SDL_FALSE ?
					SDL_TRUE : SDL_FALSE
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

void TextInput(const SDL_TextInputEvent* TextEv) {
	UTF8String utf8String = UTF8String(TextEv->text);
	QueueKeyEvent(TEXT_INPUT, SDLK_UNKNOWN, KMOD_NONE, utf8String.getUTF16()[0]);
}


void GetMousePos(SGPPoint* Point)
{
	Point->iX = gusMouseXPos;
	Point->iY = gusMouseYPos;
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

void GetRestrictedClipCursor(SGPRect* pRectangle)
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


BOOLEAN IsCursorRestricted(void)
{
	return fCursorWasClipped;
}


void SimulateMouseMovement( UINT32 uiNewXPos, UINT32 uiNewYPos )
{
	int windowWidth, windowHeight;
	SDL_GetWindowSize(GAME_WINDOW, &windowWidth, &windowHeight);

	double windowWidthD = windowWidth;
	double windowHeightD = windowHeight;
	double screenWidthD = SCREEN_WIDTH;
	double screenHeightD = SCREEN_HEIGHT;

	double scaleFactorX = windowWidthD / screenWidthD;
	double scaleFactorY = windowHeightD / screenHeightD;
	double scaleFactor = windowWidth > windowHeight ? scaleFactorY : scaleFactorX;

	double scaledWindowWidth = scaleFactor * screenWidthD;
	double scaledWindowHeight = scaleFactor * screenHeightD;

	double paddingX = (windowWidthD - scaledWindowWidth) / 2.0;
	double paddingY = (windowHeight - scaledWindowHeight) / 2.0;
	int windowPositionX = paddingX + (double)uiNewXPos * scaledWindowWidth / screenWidthD;
	int windowPositionY = paddingY + (double)uiNewYPos * scaledWindowHeight / screenHeightD;

	SDL_WarpMouseInWindow(GAME_WINDOW, windowPositionX, windowPositionY);
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

	// Is there a MIDDLE mouse button repeat
	if (gfMiddleButtonState)
	{
		if ((guiMiddleButtonRepeatTimer > 0)&&(guiMiddleButtonRepeatTimer <= uiTimer))
		{
			QueueMouseEvent(MIDDLE_BUTTON_REPEAT);
			guiMiddleButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
		}
	}
	else
	{
		guiMiddleButtonRepeatTimer = 0;
	}
}
