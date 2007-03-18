#ifndef __INPUT_
#define __INPUT_

#include "Types.h"
#include "SDL_keyboard.h"

#define SCAN_CODE_MASK						0xff0000
#define EXT_CODE_MASK							0x01000000
#define TRANSITION_MASK						0x80000000

#define KEY_DOWN									0x0001
#define KEY_UP										0x0002
#define KEY_REPEAT								0x0004
#define LEFT_BUTTON_DOWN					0x0008
#define LEFT_BUTTON_UP						0x0010
#define LEFT_BUTTON_DBL_CLK				0x0020
#define LEFT_BUTTON_REPEAT				0x0040
#define RIGHT_BUTTON_DOWN					0x0080
#define RIGHT_BUTTON_UP						0x0100
#define RIGHT_BUTTON_REPEAT				0x0200
#define MOUSE_POS									0x0400
#define MOUSE_WHEEL								0x0800

#define SHIFT_DOWN								0x01
#define CTRL_DOWN									0x02
#define ALT_DOWN									0x04

#define MAX_STRING_INPUT					64
#define DBL_CLK_TIME							300     // Increased by Alex, Jun-10-97, 200 felt too short
#define BUTTON_REPEAT_TIMEOUT			250
#define BUTTON_REPEAT_TIME				50

typedef struct
{
  UINT16 usKeyState;
  UINT16 usEvent;
  UINT32 usParam;
} InputAtom;


#ifdef __cplusplus
extern "C" {
#endif

extern BOOLEAN			InitializeInputManager(void);
extern void					ShutdownInputManager(void);
extern BOOLEAN			DequeueEvent(InputAtom *Event);
void QueueEvent(UINT16 ubInputEvent, UINT32 usParam);

void KeyDown(const SDL_keysym*);
void KeyUp(  const SDL_keysym*);

extern void					GetMousePos(SGPPoint *Point);

extern BOOLEAN DequeueSpecificEvent(InputAtom *Event, UINT32 uiMaskFlags );

extern void					RestrictMouseToXYXY(UINT16 usX1, UINT16 usY1, UINT16 usX2, UINT16 usY2);
extern void					RestrictMouseCursor(SGPRect *pRectangle);
extern void					FreeMouseCursor(void);
extern BOOLEAN			IsCursorRestricted( void );
extern void					GetRestrictedClipCursor( SGPRect *pRectangle );
extern void         RestoreCursorClipRect( void );


void SimulateMouseMovement( UINT32 uiNewXPos, UINT32 uiNewYPos );

extern void DequeueAllKeyBoardEvents();


extern BOOLEAN   gfKeyState[256];    // TRUE = Pressed, FALSE = Not Pressed

extern UINT16    gusMouseXPos;       // X position of the mouse on screen
extern UINT16    gusMouseYPos;       // y position of the mouse on screen
extern BOOLEAN   gfLeftButtonState;  // TRUE = Pressed, FALSE = Not Pressed
extern BOOLEAN   gfRightButtonState; // TRUE = Pressed, FALSE = Not Pressed


#define _KeyDown(a)        gfKeyState[(a)]
#define _LeftButtonDown    gfLeftButtonState
#define _RightButtonDown   gfRightButtonState
#define _MouseXPos				 gusMouseXPos
#define _MouseYPos				 gusMouseYPos

// NOTE: this may not be the absolute most-latest current mouse co-ordinates, use GetCursorPos for that
#define _gusMouseInside(x1,y1,x2,y2)   ((gusMouseXPos >= x1) && (gusMouseXPos <= x2) && (gusMouseYPos >= y1) && (gusMouseYPos <= y2))

#define _EvType(a)         ((InputAtom *)(a))->usEvent
#define _EvTimeStamp(a)    ((InputAtom *)(a))->uiTimeStamp
#define _EvKey(a)          ((InputAtom *)(a))->usParam
#define _EvShiftDown(a)    (((InputAtom *)(a))->usKeyState & SHIFT_DOWN)
#define _EvCtrlDown(a)     (((InputAtom *)(a))->usKeyState & CTRL_DOWN)
#define _EvAltDown(a)      (((InputAtom *)(a))->usKeyState & ALT_DOWN)

#ifdef __cplusplus
}
#endif

#endif
