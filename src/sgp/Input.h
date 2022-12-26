#pragma once

#include "Types.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"

#include <string_theory/string>

#define KEY_DOWN						1 << 0
#define KEY_UP							1 << 1
#define KEY_REPEAT						1 << 2
#define TEXT_INPUT						1 << 3
#define MOUSE_BUTTON_DOWN				1 << 4
#define MOUSE_BUTTON_UP					1 << 5
#define MOUSE_BUTTON_REPEAT				1 << 6
#define MOUSE_POS						1 << 7
#define MOUSE_WHEEL_UP					1 << 8
#define MOUSE_WHEEL_DOWN				1 << 9
#define TOUCH_FINGER_MOVE				1 << 10
#define TOUCH_FINGER_UP					1 << 11
#define TOUCH_FINGER_DOWN				1 << 12
#define TOUCH_FINGER_REPEAT				1 << 13

#define KEYBOARD_EVENTS					(KEY_DOWN | KEY_UP | KEY_REPEAT | TEXT_INPUT)
#define MOUSE_EVENTS        			(MOUSE_BUTTON_DOWN | MOUSE_BUTTON_UP | MOUSE_BUTTON_REPEAT | MOUSE_POS | MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN)
#define TOUCH_EVENTS 					(TOUCH_FINGER_MOVE | TOUCH_FINGER_UP | TOUCH_FINGER_DOWN | TOUCH_FINGER_REPEAT)

#define MOUSE_BUTTON_LEFT	1
#define MOUSE_BUTTON_MIDDLE	2
#define MOUSE_BUTTON_RIGHT	3
#define MOUSE_BUTTON_X1		4
#define MOUSE_BUTTON_X2		5

#define SHIFT_DOWN								0x01
#define CTRL_DOWN								0x02
#define ALT_DOWN								0x04

struct InputAtom
{
	UINT16 usKeyState;
	UINT16 usEvent;
	UINT32 usParam;
	ST::utf32_buffer codepoints;
};




void MouseMove(const SDL_MouseMotionEvent*);
void MouseButtonDown(const SDL_MouseButtonEvent*);
void MouseButtonUp(const SDL_MouseButtonEvent*);
void MouseWheelScroll(const SDL_MouseWheelEvent*);

void KeyDown(const SDL_Keysym*);
void KeyUp(  const SDL_Keysym*);
void TextInput(  const SDL_TextInputEvent*);

void FingerMove(const SDL_TouchFingerEvent*);
void FingerDown(const SDL_TouchFingerEvent*);
void FingerUp(const SDL_TouchFingerEvent*);

void GetMousePos(SGPPoint *Point);
// TRUE = specified mouse button is down, FALSE = specified mouse button is up
bool IsMouseButtonDown(UINT32 mouseButton);
// TRUE = Main finger is down. Multitouch gesture is not detected, FALSE = Main finger is up or multitouch gesture is in progress
bool IsMainFingerDown();
// TRUE = Last pointer device that was used was a touch device, FALSE = Last pointer device that was used was a mouse
bool IsUsingTouch();

void DequeueAllInputEvents(void);
extern BOOLEAN DequeueEvent(InputAtom *Event);
extern BOOLEAN DequeueSpecificEvent(InputAtom *Event, UINT32 uiMaskFlags );

extern void					RestrictMouseToXYXY(UINT16 usX1, UINT16 usY1, UINT16 usX2, UINT16 usY2);
void RestrictMouseCursor(const SGPRect* pRectangle);
extern void					SetSafeMousePosition(int x, int y);
extern void					FreeMouseCursor(void);
extern BOOLEAN			IsCursorRestricted( void );
extern void					GetRestrictedClipCursor( SGPRect *pRectangle );


void SimulateMouseMovement( UINT32 uiNewXPos, UINT32 uiNewYPos );

extern BOOLEAN 	 gfIsUsingTouch;
extern UINT16    gusMouseXPos;       // X position of the mouse on screen
extern UINT16    gusMouseYPos;       // y position of the mouse on screen

void HandleSingleClicksAndButtonRepeats();

bool _KeyDown(SDL_Keycode);
