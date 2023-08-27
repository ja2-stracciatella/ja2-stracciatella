#include "Types.h"
#include "Input.h"
#include "English.h"
#include "Timer.h"
#include "Video.h"
#include "Local.h"
#include "UILayout.h"

#include <string_theory/string>

#include <bitset>

#define BUTTON_REPEAT_TIMEOUT		250
#define BUTTON_REPEAT_TIME			50
#define TOUCH_REPEAT_TIMEOUT		750
#define TOUCH_REPEAT_TIME 			100

// The gfKeyState table is used to track which of the keys is up or down at any one time. This is used while polling
// the interface.  true = pressed, false = not pressed.
static std::bitset<2 * SDL_NUM_SCANCODES> gfKeyState;
static BOOLEAN fCursorWasClipped = FALSE;
static SGPRect gCursorClipRect;

static void QueuePointerEvent(UINT16 eventType, UINT32 param);

// Bitset to keep track of current mouse buttons and touch finger
static std::bitset<6> ButtonStates = 0;

// Struct to keep track of a single button or touch state and emit its events
// The first template parameter indexes into button state
template<
	size_t StateIndex,
	UINT32 BUTTON,
	UINT16 DOWN_EVENT = MOUSE_BUTTON_DOWN,
	UINT16 UP_EVENT = MOUSE_BUTTON_UP,
	UINT16 REPEAT_EVENT = MOUSE_BUTTON_REPEAT,
	UINT32 REPEAT_TIMEOUT = BUTTON_REPEAT_TIMEOUT,
	UINT32 REPEAT_TIME = BUTTON_REPEAT_TIME
>
struct ButtonState {
	private:
		UINT32 repeatTimer;

	public:
		void handleDown() {
			repeatTimer = GetClock() + REPEAT_TIMEOUT;
			ButtonStates[StateIndex] = TRUE;
			QueuePointerEvent(DOWN_EVENT, BUTTON);
		}

		void handleUp() {
			repeatTimer = 0;
			ButtonStates[StateIndex] = FALSE;
			QueuePointerEvent(UP_EVENT, BUTTON);
		}

		void detectRepeat(UINT32 uiTimer) {
			if (isDown()) {
				if (repeatTimer > 0 && repeatTimer <= uiTimer)
				{
					QueuePointerEvent(REPEAT_EVENT, BUTTON);
					repeatTimer = uiTimer + REPEAT_TIME;
				}
			} else {
				repeatTimer = 0;
			}
		}

		BOOLEAN isDown() const {
			return ButtonStates[StateIndex];
		}
};

static ButtonState<0, MOUSE_BUTTON_LEFT> gLeftButtonState;
static ButtonState<1, MOUSE_BUTTON_RIGHT> gRightButtonState;
static ButtonState<2, MOUSE_BUTTON_MIDDLE> gMiddleButtonState;
static ButtonState<3, MOUSE_BUTTON_X1> gX1ButtonState;
static ButtonState<4, MOUSE_BUTTON_X2> gX2ButtonState;
static ButtonState<5, 0, TOUCH_FINGER_DOWN, TOUCH_FINGER_UP, TOUCH_FINGER_REPEAT, TOUCH_REPEAT_TIMEOUT, TOUCH_REPEAT_TIME> gMainFingerState;

UINT16  gusMouseXPos;       // X position of the mouse on screen
UINT16  gusMouseYPos;       // y position of the mouse on screen

BOOLEAN gfIsUsingTouch;
SDL_FingerID gMainFingerId;

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
			gLeftButtonState.handleDown();
			break;
		}

		case SDL_BUTTON_RIGHT:
#if defined(WITH_MAEMO) || defined(__APPLE__)
right_button:
#endif
			gRightButtonState.handleDown();
			break;

		case SDL_BUTTON_MIDDLE:
			gMiddleButtonState.handleDown();
			break;

		case SDL_BUTTON_X1:
			gX1ButtonState.handleDown();
			break;

		case SDL_BUTTON_X2:
			gX2ButtonState.handleDown();
			break;
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
			gLeftButtonState.handleUp();
			break;
		}

		case SDL_BUTTON_RIGHT:
#if defined WITH_MAEMO || defined(__APPLE__)
right_button:
#endif
			gRightButtonState.handleUp();
			break;

		case SDL_BUTTON_MIDDLE:
			gMiddleButtonState.handleUp();
			break;

		case SDL_BUTTON_X1:
			gX1ButtonState.handleUp();
			break;

		case SDL_BUTTON_X2:
			gX2ButtonState.handleUp();
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
	#ifdef SDL_MOUSE_TOUCHID
	if (event->touchId == SDL_MOUSE_TOUCHID) return;
	#endif
	if (event->fingerId != gMainFingerId) return;
	gfIsUsingTouch = true;

	SetSafeMousePosition(event->x * SCREEN_WIDTH, event->y * SCREEN_HEIGHT);

	QueuePointerEvent(TOUCH_FINGER_MOVE, 0);
}

void FingerDown(const SDL_TouchFingerEvent* event) {
	#ifdef SDL_MOUSE_TOUCHID
	if (event->touchId == SDL_MOUSE_TOUCHID) return;
	#endif

	gMainFingerId = event->fingerId;
	gfIsUsingTouch = true;

	SetSafeMousePosition(event->x * SCREEN_WIDTH, event->y * SCREEN_HEIGHT);

	gMainFingerState.handleDown();
}

void FingerUp(const SDL_TouchFingerEvent* event) {
	#ifdef SDL_MOUSE_TOUCHID
	if (event->touchId == SDL_MOUSE_TOUCHID) return;
	#endif
	if (event->fingerId != gMainFingerId) return;
	gfIsUsingTouch = true;

	SetSafeMousePosition(event->x * SCREEN_WIDTH, event->y * SCREEN_HEIGHT);

	gMainFingerState.handleUp();
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

	// Special handling for some keys which often are used for different
	// purposes for many keyboard layouts compared to the US keyboard layout
	// and which otherwise wouldn't be usable as hotkeys in tactical mode.
	//
	// Note: it is still possible to use the key as intended by its
	// keyboard layout when entering text like a save game description or
	// the IMP name because we are using SDL_TextInput events for those,
	// which are not affected by the KeyChange function. (Issue #1844)
	// Checked if neither alt, altgr, control, shift or the 'OS' key is
	// pressed. Num lock, caps lock and scroll lock can be active.
	if ((mod & (KMOD_ALT | KMOD_CTRL | KMOD_GUI | KMOD_MODE | KMOD_SHIFT)) == KMOD_NONE)
	{
		switch (key_sym->scancode)
		{
			case SDL_SCANCODE_EQUALS:      key = SDLK_EQUALS;           break;
			case SDL_SCANCODE_SLASH:       key = SDLK_SLASH;            break;
			case SDL_SCANCODE_GRAVE:       key = SDLK_BACKQUOTE;        break;
			default:                                                    break;
		}
	}

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

	QueueKeyEvent(event_type, key, mod, {});
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
		case SDLK_MODE:
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
		case SDLK_MODE:
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

bool IsMouseButtonDown(UINT32 mouseButton) {
	switch (mouseButton) {
		case MOUSE_BUTTON_LEFT:
			return gLeftButtonState.isDown();
		case MOUSE_BUTTON_RIGHT:
			return gRightButtonState.isDown();
		case MOUSE_BUTTON_MIDDLE:
			return gMiddleButtonState.isDown();
		case MOUSE_BUTTON_X1:
			return gX1ButtonState.isDown();
		case MOUSE_BUTTON_X2:
			return gX1ButtonState.isDown();
		default:
			SLOGE("Unknown button checked in IsMouseButtonDown: {}", mouseButton);
			return false;
	}
}

bool IsMainFingerDown() {
	return gMainFingerState.isDown();
}

bool IsUsingTouch() {
	return gfIsUsingTouch;
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
	// Never move mouse on touch (it's useless and will cause a mouse event which we dont want on touch devices)
	if (gfIsUsingTouch) {
		return;
	}

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


void DequeueAllInputEvents(void)
{
	InputAtom InputEvent;
	while (DequeueEvent(&InputEvent))
	{
		//dont do anything
	}
}


void HandleSingleClicksAndButtonRepeats()
{
	UINT32 uiTimer = GetClock();

	// Is there a touch finger to repeat
	gMainFingerState.detectRepeat(uiTimer);
	gLeftButtonState.detectRepeat(uiTimer);
	gRightButtonState.detectRepeat(uiTimer);
	gMiddleButtonState.detectRepeat(uiTimer);
	gX1ButtonState.detectRepeat(uiTimer);
	gX2ButtonState.detectRepeat(uiTimer);
}
