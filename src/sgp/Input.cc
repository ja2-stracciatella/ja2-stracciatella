#include "Types.h"
#include "Input.h"
#include "English.h"
#include "Timer.h"
#include "Video.h"
#include "Local.h"
#include "UILayout.h"

#include <string_theory/string>

#include <bitset>


// The gfKeyState table is used to track which of the keys is up or down at any one time. This is used while polling
// the interface.  true = pressed, false = not pressed.
static std::bitset<2 * SDL_NUM_SCANCODES> gfKeyState;
static BOOLEAN fCursorWasClipped = FALSE;
static SGPRect gCursorClipRect;


// These data structures are used to track the mouse while polling
static UINT32 guiLeftButtonRepeatTimer;
static UINT32 guiRightButtonRepeatTimer;
static UINT32 guiMiddleButtonRepeatTimer;
static UINT32 guiX1ButtonRepeatTimer;
static UINT32 guiX2ButtonRepeatTimer;
static UINT32 guiFingerRepeatTimer;

BOOLEAN gfLeftButtonState;  // TRUE = Pressed, FALSE = Not Pressed
BOOLEAN gfRightButtonState; // TRUE = Pressed, FALSE = Not Pressed
BOOLEAN gfMiddleButtonState;// TRUE = Pressed, FALSE = Not Pressed
BOOLEAN gfX1ButtonState;// TRUE = Pressed, FALSE = Not Pressed
BOOLEAN gfX2ButtonState;// TRUE = Pressed, FALSE = Not Pressed

UINT16  gusMouseXPos;       // X position of the mouse on screen
UINT16  gusMouseYPos;       // y position of the mouse on screen

BOOLEAN gfIsUsingTouch;
BOOLEAN gfIsMainFingerDown;
SDL_FingerID gMainFingerId; // This is the last finger id that was used for touch

// The queue structures are used to track input events using queued events

static InputAtom gEventQueue[256];
static UINT16    gusQueueCount;
static UINT16    gusHeadIndex;
static UINT16    gusTailIndex;


static void QueuePointerEvent(UINT16 eventType, UINT32 param)
{
	// Can we queue up one more event, if not, the event is lost forever
	if (gusQueueCount == lengthof(gEventQueue)) return;

	gEventQueue[gusTailIndex].usEvent = eventType;
	gEventQueue[gusTailIndex].usParam = param;

	gusQueueCount++;

	gusTailIndex = (gusTailIndex + 1) % lengthof(gEventQueue);
}


static void QueueKeyEvent(UINT16 ubInputEvent, SDL_Keycode Key, SDL_Keymod Mod, ST::utf32_buffer codepoints)
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
	gEventQueue[gusTailIndex].codepoints = std::move(codepoints);

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

void HandleSingleClicksAndButtonRepeats();

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

BOOLEAN DequeueEvent(InputAtom* Event)
{
	if (gusQueueCount == 0) return FALSE;

	*Event = gEventQueue[gusHeadIndex];
	gusHeadIndex = (gusHeadIndex + 1) % lengthof(gEventQueue);
	gusQueueCount--;
	return TRUE;
}

#if defined(WITH_MAEMO) || defined __APPLE__
static BOOLEAN g_down_right;
#endif

void MouseMove(const SDL_MouseMotionEvent* event) {
	if (event->which == SDL_TOUCH_MOUSEID) return;
	gfIsUsingTouch = FALSE;

	SetSafeMousePosition(event->x, event->y);
}

void MouseButtonDown(const SDL_MouseButtonEvent* BtnEv)
{
	if (BtnEv->which == SDL_TOUCH_MOUSEID) return;
	gfIsUsingTouch = FALSE;

	SetSafeMousePosition(BtnEv->x, BtnEv->y);
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
			QueuePointerEvent(MOUSE_BUTTON_DOWN, MOUSE_BUTTON_LEFT);
			break;
		}

		case SDL_BUTTON_RIGHT:
#if defined(WITH_MAEMO) || defined(__APPLE__)
right_button:
#endif
			guiRightButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfRightButtonState = TRUE;
			QueuePointerEvent(MOUSE_BUTTON_DOWN, MOUSE_BUTTON_RIGHT);
			break;

		case SDL_BUTTON_MIDDLE:
			guiMiddleButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfMiddleButtonState = TRUE;
			QueuePointerEvent(MOUSE_BUTTON_DOWN, MOUSE_BUTTON_MIDDLE);
			break;

		case SDL_BUTTON_X1:
			guiX1ButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfX1ButtonState = TRUE;
			QueuePointerEvent(MOUSE_BUTTON_DOWN, MOUSE_BUTTON_X1);

		case SDL_BUTTON_X2:
			guiX2ButtonRepeatTimer = GetClock() + BUTTON_REPEAT_TIMEOUT;
			gfX2ButtonState = TRUE;
			QueuePointerEvent(MOUSE_BUTTON_DOWN, MOUSE_BUTTON_X2);
	}
}


void MouseButtonUp(const SDL_MouseButtonEvent* BtnEv)
{
	if (BtnEv->which == SDL_TOUCH_MOUSEID) return;
	gfIsUsingTouch = false;

	SetSafeMousePosition(BtnEv->x, BtnEv->y);
	switch (BtnEv->button)
	{
		case SDL_BUTTON_LEFT:
		{
#if defined(WITH_MAEMO) || defined(__APPLE__)
			if (g_down_right) goto right_button;
#endif
			guiLeftButtonRepeatTimer = 0;
			gfLeftButtonState = FALSE;
			QueuePointerEvent(MOUSE_BUTTON_UP, MOUSE_BUTTON_LEFT);
			break;
		}

		case SDL_BUTTON_RIGHT:
#if defined WITH_MAEMO || defined(__APPLE__)
right_button:
#endif
			guiRightButtonRepeatTimer = 0;
			gfRightButtonState = FALSE;
			QueuePointerEvent(MOUSE_BUTTON_UP, MOUSE_BUTTON_RIGHT);
			break;

		case SDL_BUTTON_MIDDLE:
			guiMiddleButtonRepeatTimer = 0;
			gfMiddleButtonState = FALSE;
			QueuePointerEvent(MOUSE_BUTTON_UP, MOUSE_BUTTON_MIDDLE);
			break;

		case SDL_BUTTON_X1:
			guiX1ButtonRepeatTimer = 0;
			gfX1ButtonState = FALSE;
			QueuePointerEvent(MOUSE_BUTTON_UP, MOUSE_BUTTON_X1);
			break;

		case SDL_BUTTON_X2:
			guiX2ButtonRepeatTimer = 0;
			gfX2ButtonState = FALSE;
			QueuePointerEvent(MOUSE_BUTTON_UP, MOUSE_BUTTON_X2);
			break;
	}
}

void MouseWheelScroll(const SDL_MouseWheelEvent* WheelEv)
{
	gfIsUsingTouch = false;

	if (WheelEv->y > 0)
	{
		QueuePointerEvent(MOUSE_WHEEL_UP, 0);
	}
	else
	{
		QueuePointerEvent(MOUSE_WHEEL_DOWN, 0);
	}
}

void FingerMove(const SDL_TouchFingerEvent* event) {
	if (event->fingerId != gMainFingerId) return;
	gfIsUsingTouch = true;
	gfIsMainFingerDown = true;

	SetSafeMousePosition(event->x * SCREEN_WIDTH, event->y * SCREEN_HEIGHT);

	QueuePointerEvent(TOUCH_FINGER_MOVE, 0);
}

void FingerDown(const SDL_TouchFingerEvent* event) {
	gMainFingerId = event->fingerId;
	gfIsUsingTouch = true;
	gfIsMainFingerDown = true;
	guiFingerRepeatTimer = GetClock() + TOUCH_REPEAT_TIMEOUT;

	SetSafeMousePosition(event->x * SCREEN_WIDTH, event->y * SCREEN_HEIGHT);

	QueuePointerEvent(TOUCH_FINGER_DOWN, 0);
}

void FingerUp(const SDL_TouchFingerEvent* event) {
	if (event->fingerId != gMainFingerId) return;
	gfIsUsingTouch = true;
	gfIsMainFingerDown = false;

	SetSafeMousePosition(event->x * SCREEN_WIDTH, event->y * SCREEN_HEIGHT);

	QueuePointerEvent(TOUCH_FINGER_UP, 0);
}


// Remap SDL keycodes with bit 30 set to the range 512..1023
// Necessary to be able to use the keycode as an index for the gfKeyState bitset.
static SDL_Keycode RemapKeycode(SDL_Keycode const key)
{
	return (key & SDLK_SCANCODE_MASK)
		? (key & ~SDLK_SCANCODE_MASK) + SDL_NUM_SCANCODES
		: key;
}


bool _KeyDown(SDL_Keycode const keycode)
{
	return gfKeyState[RemapKeycode(keycode)];
}


static void KeyChange(SDL_Keysym const* const key_sym, bool const pressed)
{
	SDL_Keycode      key = key_sym->sym;
	SDL_Keymod const mod = (SDL_Keymod) key_sym->mod;
	bool   const num = mod & KMOD_NUM;
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
			if ((key & ~SDLK_SCANCODE_MASK) >= SDL_NUM_SCANCODES) return;
			break;
	}

	UINT     event_type;
	bool key_state = _KeyDown(key);
	if (pressed)
	{
		event_type = key_state ? KEY_REPEAT : KEY_DOWN;
	}
	else
	{
		if (!key_state) return;
		event_type = KEY_UP;
	}
	gfKeyState[RemapKeycode(key)] = pressed;

	QueueKeyEvent(event_type, key, mod, ST::null);
}


void KeyDown(const SDL_Keysym* KeySym)
{
	switch (KeySym->sym)
	{
		case SDLK_LSHIFT:
		case SDLK_RSHIFT:
			gfKeyState.set(SHIFT);
			break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			gfKeyState.set(CTRL);
			break;

		case SDLK_LALT:
		case SDLK_RALT:
			gfKeyState.set(ALT);
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
			gfKeyState.reset(SHIFT);
			break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:
			gfKeyState.reset(CTRL);
			break;

		case SDLK_LALT:
		case SDLK_RALT:
			gfKeyState.reset(ALT);
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
	try {
		QueueKeyEvent(TEXT_INPUT, SDLK_UNKNOWN, KMOD_NONE, ST::string(TextEv->text).to_utf32());
	}
	catch (const ST::unicode_error&)
	{
		// ignore invalid inputs
		static bool warn = true;
		if (warn)
		{
			SLOGW("Received invalid utf-8 character.");
			warn = false;
		}
	}
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


void HandleSingleClicksAndButtonRepeats()
{
	UINT32 uiTimer = GetClock();

	// Is there a touch finger to repeat
	if (gfIsMainFingerDown) {
		if ((guiFingerRepeatTimer > 0)&&(guiFingerRepeatTimer <= uiTimer))
		{
			QueuePointerEvent(TOUCH_FINGER_REPEAT, 0);
			guiFingerRepeatTimer = uiTimer + TOUCH_REPEAT_TIME;
		}
	} else {
		guiFingerRepeatTimer = 0;
	}

	// Is there a LEFT mouse button repeat
	if (gfLeftButtonState)
	{
		if ((guiLeftButtonRepeatTimer > 0)&&(guiLeftButtonRepeatTimer <= uiTimer))
		{
			QueuePointerEvent(MOUSE_BUTTON_REPEAT, MOUSE_BUTTON_LEFT);
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
			QueuePointerEvent(MOUSE_BUTTON_REPEAT, MOUSE_BUTTON_RIGHT);
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
			QueuePointerEvent(MOUSE_BUTTON_REPEAT, MOUSE_BUTTON_MIDDLE);
			guiMiddleButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
		}
	}
	else
	{
		guiMiddleButtonRepeatTimer = 0;
	}

	// Is there a X1 mouse button repeat
	if (gfX1ButtonState)
	{
		if ((guiX1ButtonRepeatTimer > 0)&&(guiX1ButtonRepeatTimer <= uiTimer))
		{
			QueuePointerEvent(MOUSE_BUTTON_REPEAT, MOUSE_BUTTON_X1);
			guiX1ButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
		}
	}
	else
	{
		guiX1ButtonRepeatTimer = 0;
	}

	// Is there a X2 mouse button repeat
	if (gfX2ButtonState)
	{
		if ((guiX2ButtonRepeatTimer > 0)&&(guiX2ButtonRepeatTimer <= uiTimer))
		{
			QueuePointerEvent(MOUSE_BUTTON_REPEAT, MOUSE_BUTTON_X2);
			guiX2ButtonRepeatTimer = uiTimer + BUTTON_REPEAT_TIME;
		}
	}
	else
	{
		guiX2ButtonRepeatTimer = 0;
	}
}
