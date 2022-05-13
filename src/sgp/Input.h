#ifndef __INPUT_
#define __INPUT_

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
#define MOUSE_BUTTON_REPEAT				1 << 7
#define MOUSE_POS						1 << 8
#define MOUSE_WHEEL_UP      			1 << 9
#define MOUSE_WHEEL_DOWN    			1 << 10

#define MOUSE_EVENTS        			(MOUSE_BUTTON_DOWN | MOUSE_BUTTON_UP | MOUSE_BUTTON_REPEAT | MOUSE_POS | MOUSE_WHEEL_UP | MOUSE_WHEEL_DOWN)

#define MOUSE_BUTTON_LEFT SDL_BUTTON_LEFT
#define MOUSE_BUTTON_MIDDLE SDL_BUTTON_MIDDLE
#define MOUSE_BUTTON_RIGHT SDL_BUTTON_RIGHT
#define MOUSE_BUTTON_X1 SDL_BUTTON_X1
#define MOUSE_BUTTON_X2 SDL_BUTTON_X2

#define SHIFT_DOWN								0x01
#define CTRL_DOWN									0x02
#define ALT_DOWN									0x04

#define DBL_CLK_TIME							300     // Increased by Alex, Jun-10-97, 200 felt too short
#define BUTTON_REPEAT_TIMEOUT			250
#define BUTTON_REPEAT_TIME				50

struct InputAtom
{
	UINT16 usKeyState;
	UINT16 usEvent;
	UINT32 usParam;
	ST::utf32_buffer codepoints;
};


extern BOOLEAN			DequeueEvent(InputAtom *Event);

void MouseButtonDown(const SDL_MouseButtonEvent*);
void MouseButtonUp(const SDL_MouseButtonEvent*);
void MouseWheelScroll(const SDL_MouseWheelEvent*);

void KeyDown(const SDL_Keysym*);
void KeyUp(  const SDL_Keysym*);
void TextInput(  const SDL_TextInputEvent*);

extern void					GetMousePos(SGPPoint *Point);

extern BOOLEAN DequeueSpecificEvent(InputAtom *Event, UINT32 uiMaskFlags );

extern void					RestrictMouseToXYXY(UINT16 usX1, UINT16 usY1, UINT16 usX2, UINT16 usY2);
void RestrictMouseCursor(const SGPRect* pRectangle);
extern void					SetSafeMousePosition(int x, int y);
extern void					FreeMouseCursor(void);
extern BOOLEAN			IsCursorRestricted( void );
extern void					GetRestrictedClipCursor( SGPRect *pRectangle );


void SimulateMouseMovement( UINT32 uiNewXPos, UINT32 uiNewYPos );

void DequeueAllKeyBoardEvents(void);


extern UINT16    gusMouseXPos;       // X position of the mouse on screen
extern UINT16    gusMouseYPos;       // y position of the mouse on screen
extern BOOLEAN   gfLeftButtonState;  // TRUE = Pressed, FALSE = Not Pressed
extern BOOLEAN   gfRightButtonState; // TRUE = Pressed, FALSE = Not Pressed
extern BOOLEAN   gfMiddleButtonState; // TRUE = Pressed, FALSE = Not Pressed
extern BOOLEAN   gfX1ButtonState; // TRUE = Pressed, FALSE = Not Pressed
extern BOOLEAN   gfX2ButtonState; // TRUE = Pressed, FALSE = Not Pressed

bool _KeyDown(SDL_Keycode);

#endif
