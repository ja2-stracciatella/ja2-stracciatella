
//=================================================================================================
//	MouseSystem.c
//
//	Routines for handling prioritized mouse regions. The system as setup below allows the use of
//	callback functions for each region, as well as allowing a different cursor to be defined for
//	each region.
//
//	Written by Bret Rowdon, Jan 30 '97
//  Re-Written by Kris Morness, since...
//
//=================================================================================================

#include <stdexcept>

#include "Font.h"
#include "HImage.h"
#include "Types.h"
#include "Debug.h"
#include "Input.h"
#include "MemMan.h"
#include "Line.h"
#include "VObject.h"
#include "Video.h"
#include "MouseSystem.h"
#include "Cursor_Control.h"
#include "Button_System.h"
#include "Timer.h"
#include "Font_Control.h"
#include "JAScreens.h"
#include "Local.h"
#include "Render_Dirty.h"
#include "VSurface.h"
#include "ScreenIDs.h"
#include "UILayout.h"


//Kris:	Nov 31, 1999 -- Added support for double clicking
//
//Max double click delay (in milliseconds) to be considered a double click
#define MSYS_DOUBLECLICK_DELAY		400
//
//Records and stores the last place the user clicked.  These values are compared to the current
//click to determine if a double click event has been detected.
static MOUSE_REGION* gpRegionLastLButtonDown      = NULL;
static MOUSE_REGION* gpRegionLastLButtonUp        = NULL;
static UINT32        guiRegionLastLButtonDownTime = 0;


INT16 MSYS_CurrentMX=0;
INT16 MSYS_CurrentMY=0;
static INT16 MSYS_CurrentButtons = 0;
static INT16 MSYS_Action         = 0;

static BOOLEAN MSYS_SystemInitialized   = FALSE;

static MOUSE_REGION* g_clicked_region;

static MOUSE_REGION* MSYS_RegList = NULL;

static MOUSE_REGION* MSYS_PrevRegion = 0;
static MOUSE_REGION* MSYS_CurrRegion = NULL;

static const INT16 gsFastHelpDelay = 600; // In timer ticks


static BOOLEAN gfRefreshUpdate = FALSE;

//Kris:  December 3, 1997
//Special internal debugging utilities that will ensure that you don't attempt to delete
//an already deleted region.  It will also ensure that you don't create an identical region
//that already exists.
//TO REMOVE ALL DEBUG FUNCTIONALITY:  simply comment out MOUSESYSTEM_DEBUGGING definition
#if defined _DEBUG && !defined BOUNDS_CHECKER
#	define MOUSESYSTEM_DEBUGGING
#endif


static void MSYS_TrashRegList(void);


//======================================================================================================
//	MSYS_Init
//
//	Initialize the mouse system.
//
void MSYS_Init(void)
{
	MSYS_TrashRegList();

	MSYS_CurrentMX = 0;
	MSYS_CurrentMY = 0;
	MSYS_CurrentButtons = 0;
	MSYS_Action=MSYS_NO_ACTION;

	MSYS_PrevRegion = NULL;
	MSYS_SystemInitialized = TRUE;
}


//======================================================================================================
//	MSYS_Shutdown
//
//	De-inits the "mousesystem" mouse region handling code.
//
void MSYS_Shutdown(void)
{
	MSYS_SystemInitialized = FALSE;
	MSYS_TrashRegList();
}


static void MSYS_UpdateMouseRegion(void);


void MouseSystemHook(UINT16 Type, UINT16 Xcoord, UINT16 Ycoord)
{
	// If the mouse system isn't initialized, get out o' here
	if (!MSYS_SystemInitialized) return;

	INT16 action = MSYS_NO_ACTION;
	switch (Type)
	{
		case LEFT_BUTTON_DOWN:  action |= MSYS_DO_LBUTTON_DWN; goto update_buttons;

		case LEFT_BUTTON_UP:
			/* Kris:
			 * Used only if applicable.  This is used for that special button that is
			 * locked with the mouse press -- just like windows.  When you release the
			 * button, the previous state of the button is restored if you released
			 * the mouse outside of it's boundaries.  If you release inside of the
			 * button, the action is selected -- but later in the code.
			 * NOTE:  It has to be here, because the mouse can be released anywhere
			 *        regardless of regions, buttons, etc. */
			ReleaseAnchorMode();
			action |= MSYS_DO_LBUTTON_UP;
			goto update_buttons;

		case RIGHT_BUTTON_DOWN: action |= MSYS_DO_RBUTTON_DWN; goto update_buttons;
		case RIGHT_BUTTON_UP:   action |= MSYS_DO_RBUTTON_UP;  goto update_buttons;
		case MIDDLE_BUTTON_DOWN:action |= MSYS_DO_MBUTTON_DWN; goto update_buttons;
		case MIDDLE_BUTTON_UP:  action |= MSYS_DO_MBUTTON_UP;  goto update_buttons;

update_buttons:
			MSYS_CurrentButtons &= ~(MSYS_LEFT_BUTTON | MSYS_RIGHT_BUTTON | MSYS_MIDDLE_BUTTON);
			MSYS_CurrentButtons |= (_LeftButtonDown  ? MSYS_LEFT_BUTTON  : 0);
			MSYS_CurrentButtons |= (_RightButtonDown ? MSYS_RIGHT_BUTTON : 0);
			MSYS_CurrentButtons |= (_MiddleButtonDown ? MSYS_MIDDLE_BUTTON : 0);
			break;

		// ATE: Checks here for mouse button repeats.....
		// Call mouse region with new reason
		case LEFT_BUTTON_REPEAT:  action |= MSYS_DO_LBUTTON_REPEAT; break;
		case RIGHT_BUTTON_REPEAT: action |= MSYS_DO_RBUTTON_REPEAT; break;
		case MIDDLE_BUTTON_REPEAT:action |= MSYS_DO_MBUTTON_REPEAT; break;

		case MOUSE_WHEEL_UP:   action |= MSYS_DO_WHEEL_UP;   break;
		case MOUSE_WHEEL_DOWN: action |= MSYS_DO_WHEEL_DOWN; break;

		case MOUSE_POS:
			if (gfRefreshUpdate)
			{
				gfRefreshUpdate = FALSE;
				goto force_move;
			}
			break;

		default: return; /* Not a mouse message, ignore it */
	}

	if (Xcoord != MSYS_CurrentMX || Ycoord != MSYS_CurrentMY)
	{
force_move:
		action         |= MSYS_DO_MOVE;
		MSYS_CurrentMX  = Xcoord;
		MSYS_CurrentMY  = Ycoord;
	}

	MSYS_Action = action;
	if (action != MSYS_NO_ACTION) MSYS_UpdateMouseRegion();
}


//======================================================================================================
//	MSYS_TrashRegList
//
//	Deletes the entire region list.
//
static void MSYS_TrashRegList(void)
{
	while( MSYS_RegList )
	{
		if( MSYS_RegList->uiFlags & MSYS_REGION_EXISTS )
		{
			MSYS_RemoveRegion(MSYS_RegList);
		}
		else
		{
			MSYS_RegList = MSYS_RegList->next;
		}
	}
}


static void MSYS_DeleteRegionFromList(MOUSE_REGION*);


/* Add a region struct to the current list. The list is sorted by priority
 * levels. If two entries have the same priority level, then the latest to enter
 * the list gets the higher priority. */
static void MSYS_AddRegionToList(MOUSE_REGION* const r)
{
	/* If region seems to already be in list, delete it so we can re-insert the
	 * region. */
	MSYS_DeleteRegionFromList(r);

	MOUSE_REGION* i = MSYS_RegList;
	if (!i)
	{ // Empty list, so add it straight up.
		MSYS_RegList = r;
	}
	else
	{
		// Walk down list until we find place to insert (or at end of list)
		for (; i->next; i = i->next)
		{
			if (i->PriorityLevel <= r->PriorityLevel) break;
		}

		if (i->PriorityLevel > r->PriorityLevel)
		{ // Add after node
			r->prev = i;
			r->next = i->next;
			if (r->next) r->next->prev = r;
			i->next = r;
		}
		else
		{ // Add before node
			r->prev = i->prev;
			r->next = i;
			*(r->prev ? &r->prev->next : &MSYS_RegList) = r;
			i->prev = r;
		}
	}
}


// Removes a region from the current list.
static void MSYS_DeleteRegionFromList(MOUSE_REGION* const r)
{
	MOUSE_REGION* const prev = r->prev;
	MOUSE_REGION* const next = r->next;
	if (prev) prev->next = next;
	if (next) next->prev = prev;

	if (MSYS_RegList == r) MSYS_RegList = next;

	r->prev = 0;
	r->next = 0;
}


/* Searches the list for the highest priority region and updates its info.  It
 * also dispatches the callback functions */
static void MSYS_UpdateMouseRegion(void)
{
	MOUSE_REGION* cur;
	for (cur = MSYS_RegList; cur != NULL; cur = cur->next)
	{
		if (cur->uiFlags & (MSYS_REGION_ENABLED | MSYS_ALLOW_DISABLED_FASTHELP) &&
			cur->RegionTopLeftX <= MSYS_CurrentMX && MSYS_CurrentMX <= cur->RegionBottomRightX &&
			cur->RegionTopLeftY <= MSYS_CurrentMY && MSYS_CurrentMY <= cur->RegionBottomRightY)
		{
			/* We got the right region. We don't need to check for priorities because
			 * the whole list is sorted the right way! */
			break;
		}
	}
	MSYS_CurrRegion = cur;

	MOUSE_REGION* prev = MSYS_PrevRegion;
	if (prev)
	{
		prev->uiFlags &= ~MSYS_MOUSE_IN_AREA;

		if (prev != cur)
		{
			/* Remove the help text for the previous region if one is currently being
			 * displayed. */
			if (prev->FastHelpText)
			{
#ifdef _JA2_RENDER_DIRTY
				if (prev->uiFlags & MSYS_GOT_BACKGROUND)
				{
					FreeBackgroundRectPending(prev->FastHelpRect);
				}
#endif
				prev->uiFlags &= ~MSYS_GOT_BACKGROUND;
				prev->uiFlags &= ~MSYS_FASTHELP_RESET;
			}

			/* Force a callbacks to happen on previous region to indicate that the
			 * mouse has left the old region */
			if (prev->MovementCallback != NULL && prev->uiFlags & MSYS_REGION_ENABLED)
			{
				prev->MovementCallback(prev, MSYS_CALLBACK_REASON_LOST_MOUSE);
			}
		}
	}

	// If a region was found in the list, update its data
	if (cur != NULL)
	{
		if (cur != prev)
		{
			cur->FastHelpTimer = gsFastHelpDelay;

			//Kris -- October 27, 1997
			//Implemented gain mouse region
			if (cur->MovementCallback != NULL)
			{
				if (cur->FastHelpText && !(cur->uiFlags & MSYS_FASTHELP_RESET))
				{
#ifdef _JA2_RENDER_DIRTY
					if (cur->uiFlags & MSYS_GOT_BACKGROUND)
					{
						FreeBackgroundRectPending(cur->FastHelpRect);
					}
#endif
					cur->uiFlags &= ~MSYS_GOT_BACKGROUND;
					cur->uiFlags |= MSYS_FASTHELP_RESET;
				}
				if (cur->uiFlags & MSYS_REGION_ENABLED)
				{
					cur->MovementCallback(cur, MSYS_CALLBACK_REASON_GAIN_MOUSE);
				}
			}

			// if the cursor is set and is not set to no cursor
			if (cur->uiFlags & MSYS_REGION_ENABLED && cur->Cursor != MSYS_NO_CURSOR)
			{
				MSYS_SetCurrentCursor(cur->Cursor);
			}
			else
			{
				/* Addition Oct 10/1997 Carter, patch for mouse cursor
				 * start at region and find another region encompassing */
				for (const MOUSE_REGION* i = cur->next; i != NULL; i = i->next)
				{
					if (i->uiFlags & MSYS_REGION_ENABLED &&
							i->RegionTopLeftX <= MSYS_CurrentMX && MSYS_CurrentMX <= i->RegionBottomRightX &&
							i->RegionTopLeftY <= MSYS_CurrentMY && MSYS_CurrentMY <= i->RegionBottomRightY &&
							i->Cursor != MSYS_NO_CURSOR)
					{
						MSYS_SetCurrentCursor(i->Cursor);
						break;
					}
				}
			}
		}

		// OK, if we do not have a button down, any button is game!
		if (!g_clicked_region || g_clicked_region == cur)
		{
			cur->uiFlags |= MSYS_MOUSE_IN_AREA;

			cur->MouseXPos    = MSYS_CurrentMX;
			cur->MouseYPos    = MSYS_CurrentMY;
			cur->RelativeXPos = MSYS_CurrentMX - cur->RegionTopLeftX;
			cur->RelativeYPos = MSYS_CurrentMY - cur->RegionTopLeftY;

			cur->ButtonState = MSYS_CurrentButtons;

			if (cur->uiFlags & MSYS_REGION_ENABLED &&
					cur->MovementCallback != NULL &&
					MSYS_Action & MSYS_DO_MOVE)
			{
				cur->MovementCallback(cur, MSYS_CALLBACK_REASON_MOVE);
			}

			MSYS_Action &= ~MSYS_DO_MOVE;

			if (cur->ButtonCallback != NULL && MSYS_Action & MSYS_DO_BUTTONS)
			{
				if (cur->uiFlags & MSYS_REGION_ENABLED)
				{
					UINT32 ButtonReason = MSYS_CALLBACK_REASON_NONE;
					if (MSYS_Action & MSYS_DO_LBUTTON_DWN)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_DWN;
						g_clicked_region = cur;
					}

					if (MSYS_Action & MSYS_DO_LBUTTON_UP)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_UP;
						g_clicked_region = 0;
					}

					if (MSYS_Action & MSYS_DO_RBUTTON_DWN)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_DWN;
						g_clicked_region = cur;
					}

					if (MSYS_Action & MSYS_DO_RBUTTON_UP)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_UP;
						g_clicked_region = 0;
					}

					if (MSYS_Action & MSYS_DO_MBUTTON_DWN)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_MBUTTON_DWN;
						g_clicked_region = cur;
					}

					if (MSYS_Action & MSYS_DO_MBUTTON_UP)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_MBUTTON_UP;
						g_clicked_region = 0;
					}

					// ATE: Added repeat resons....
					if (MSYS_Action & MSYS_DO_LBUTTON_REPEAT)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_REPEAT;
					}

					if (MSYS_Action & MSYS_DO_RBUTTON_REPEAT)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_REPEAT;
					}

					if (MSYS_Action & MSYS_DO_MBUTTON_REPEAT)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_MBUTTON_REPEAT;
					}

					if (MSYS_Action & MSYS_DO_WHEEL_UP)   ButtonReason |= MSYS_CALLBACK_REASON_WHEEL_UP;
					if (MSYS_Action & MSYS_DO_WHEEL_DOWN) ButtonReason |= MSYS_CALLBACK_REASON_WHEEL_DOWN;

					if (ButtonReason != MSYS_CALLBACK_REASON_NONE)
					{
						if (cur->uiFlags & MSYS_FASTHELP)
						{
							// Button was clicked so remove any FastHelp text
							cur->uiFlags &= ~MSYS_FASTHELP;
#ifdef _JA2_RENDER_DIRTY
							if (cur->uiFlags & MSYS_GOT_BACKGROUND)
							{
								FreeBackgroundRectPending(cur->FastHelpRect);
							}
#endif
							cur->uiFlags &= ~MSYS_GOT_BACKGROUND;
							cur->uiFlags &= ~MSYS_FASTHELP_RESET;

							cur->FastHelpTimer = gsFastHelpDelay;
						}

						//Kris: Nov 31, 1999 -- Added support for double click events.
						//This is where double clicks are checked and passed down.
						if (ButtonReason == MSYS_CALLBACK_REASON_LBUTTON_DWN)
						{
							UINT32 uiCurrTime = GetClock();
							if (gpRegionLastLButtonDown == cur &&
									gpRegionLastLButtonUp   == cur &&
									uiCurrTime <= guiRegionLastLButtonDownTime + MSYS_DOUBLECLICK_DELAY)
							{
								/* Sequential left click on same button within the maximum time
								 * allowed for a double click.  Double click check succeeded,
								 * set flag and reset double click globals. */
								ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_DOUBLECLICK;
								gpRegionLastLButtonDown = NULL;
								gpRegionLastLButtonUp   = NULL;
								guiRegionLastLButtonDownTime = 0;
							}
							else
							{
								/* First click, record time and region pointer (to check if 2nd
								 * click detected later) */
								gpRegionLastLButtonDown = cur;
								guiRegionLastLButtonDownTime = GetClock();
							}
						}
						else if (ButtonReason == MSYS_CALLBACK_REASON_LBUTTON_UP)
						{
							UINT32 uiCurrTime = GetClock();
							if (gpRegionLastLButtonDown == cur &&
									uiCurrTime <= guiRegionLastLButtonDownTime + MSYS_DOUBLECLICK_DELAY)
							{
								/* Double click is Left down, then left up, then left down.  We
								 * have just detected the left up here (step 2). */
								gpRegionLastLButtonUp = cur;
							}
							else
							{
								/* User released mouse outside of current button, so kill any
								 * chance of a double click happening. */
								gpRegionLastLButtonDown = NULL;
								gpRegionLastLButtonUp   = NULL;
								guiRegionLastLButtonDownTime = 0;
							}
						}

						cur->ButtonCallback(cur, ButtonReason);
					}
				}
			}

			MSYS_Action &= ~MSYS_DO_BUTTONS;
		}
		else if (cur->uiFlags & MSYS_REGION_ENABLED)
		{
			// OK here, if we have release a button, UNSET LOCK wherever you are....
			// Just don't give this button the message....
			if (MSYS_Action & MSYS_DO_RBUTTON_UP) g_clicked_region = 0;
			if (MSYS_Action & MSYS_DO_LBUTTON_UP) g_clicked_region = 0;
			if (MSYS_Action & MSYS_DO_MBUTTON_UP) g_clicked_region = 0;

			// OK, you still want move messages however....
			cur->uiFlags |= MSYS_MOUSE_IN_AREA;
			cur->MouseXPos = MSYS_CurrentMX;
			cur->MouseYPos = MSYS_CurrentMY;
			cur->RelativeXPos = MSYS_CurrentMX - cur->RegionTopLeftX;
			cur->RelativeYPos = MSYS_CurrentMY - cur->RegionTopLeftY;

			if (cur->MovementCallback != NULL && MSYS_Action & MSYS_DO_MOVE)
			{
				cur->MovementCallback(cur, MSYS_CALLBACK_REASON_MOVE);
			}

			MSYS_Action &= ~MSYS_DO_MOVE;
		}
	}
	/* the current region can get deleted during this function, so fetch the
	 * latest value here */
	MSYS_PrevRegion = MSYS_CurrRegion;
}



/* Inits a MOUSE_REGION structure for use with the mouse system */
void MSYS_DefineRegion(MOUSE_REGION* const r, UINT16 const tlx, UINT16 const tly, UINT16 const brx, UINT16 const bry, INT8 priority, UINT16 const crsr, MOUSE_CALLBACK const movecallback, MOUSE_CALLBACK const buttoncallback)
{
#ifdef MOUSESYSTEM_DEBUGGING
	AssertMsg(!(r->uiFlags & MSYS_REGION_EXISTS), "Attempting to define a region that already exists.");
#endif

	if (priority <= MSYS_PRIORITY_LOWEST) priority = MSYS_PRIORITY_LOWEST;

	r->PriorityLevel      = priority;
	r->uiFlags            = MSYS_REGION_ENABLED | MSYS_REGION_EXISTS;
	r->RegionTopLeftX     = tlx;
	r->RegionTopLeftY     = tly;
	r->RegionBottomRightX = brx;
	r->RegionBottomRightY = bry;
	r->MouseXPos          = 0;
	r->MouseYPos          = 0;
	r->RelativeXPos       = 0;
	r->RelativeYPos       = 0;
	r->ButtonState        = 0;
	r->Cursor             = crsr;
	r->MovementCallback   = movecallback;
	r->ButtonCallback     = buttoncallback;
	r->FastHelpTimer      = 0;
	r->FastHelpText       = 0;
	r->next               = 0;
	r->prev               = 0;

	MSYS_AddRegionToList(r);
	gfRefreshUpdate = TRUE;
}


void MOUSE_REGION::ChangeCursor(UINT16 const crsr)
{
	Cursor = crsr;
	if (crsr != MSYS_NO_CURSOR && uiFlags & MSYS_MOUSE_IN_AREA)
	{
		MSYS_SetCurrentCursor(crsr);
	}
}


void MSYS_RemoveRegion(MOUSE_REGION* const r)
{
#ifdef MOUSESYSTEM_DEBUGGING
	AssertMsg(r, "Attempting to remove a NULL region.");
#endif
	if (!r) return;
#ifdef MOUSESYSTEM_DEBUGGING
	AssertMsg(r->uiFlags & MSYS_REGION_EXISTS, "Attempting to remove an already removed region.");
#endif

#ifdef _JA2_RENDER_DIRTY
	if (r->uiFlags & MSYS_HAS_BACKRECT)
	{
		FreeBackgroundRectPending(r->FastHelpRect);
		r->uiFlags &= ~MSYS_HAS_BACKRECT;
	}
#endif

	if (r->FastHelpText)
	{
		MemFree(r->FastHelpText);
		r->FastHelpText = 0;
	}

	MSYS_DeleteRegionFromList(r);

	if (MSYS_PrevRegion  == r) MSYS_PrevRegion  = 0;
	if (MSYS_CurrRegion  == r) MSYS_CurrRegion  = 0;
	if (g_clicked_region == r) g_clicked_region = 0;

	gfRefreshUpdate = TRUE;
	memset(r, 0, sizeof(*r));
}


//=================================================================================================
//	MSYS_SetCurrentCursor
//
//	Sets the mouse cursor to the regions defined value.
//
void MSYS_SetCurrentCursor(UINT16 Cursor)
{
	SetCurrentCursorFromDatabase( Cursor );
}


void MSYS_SetRegionUserData(MOUSE_REGION* const r, UINT32 const index, INT32 const userdata)
{
	if (lengthof(r->user.data) <= index) throw std::logic_error("User data index is out of range");
	r->user.data[index] = userdata;
}


INT32 MSYS_GetRegionUserData(MOUSE_REGION const* const r, UINT32 const index)
{
	if (lengthof(r->user.data) <= index) throw std::logic_error("User data index is out of range");
	return r->user.data[index];
}


// This function will force a re-evaluation of mouse regions
// Usually used to force change of mouse cursor if panels switch, etc
void RefreshMouseRegions( )
{
	MSYS_Action|=MSYS_DO_MOVE;

	MSYS_UpdateMouseRegion( );

}


void MOUSE_REGION::SetFastHelpText(wchar_t const* const text)
{
	if (FastHelpText)
	{
		MemFree(FastHelpText);
		FastHelpText = 0;
	}

	if (!(uiFlags & MSYS_REGION_EXISTS)) return;

	if (!text || text[0] == L'\0') return;

	FastHelpText = MALLOCN(wchar_t, wcslen(text) + 1);
	wcscpy(FastHelpText, text);

	/* ATE: We could be replacing already existing, active text so let's remove
	 * the region so it be rebuilt */

	if (guiCurrentScreen == MAP_SCREEN) return;

#ifdef _JA2_RENDER_DIRTY
	if (uiFlags & MSYS_GOT_BACKGROUND) FreeBackgroundRectPending(FastHelpRect);
#endif

	uiFlags &= ~MSYS_GOT_BACKGROUND;
	uiFlags &= ~MSYS_FASTHELP_RESET;
}


static UINT32 GetNumberOfLinesInHeight(const wchar_t* String)
{
	UINT32 Lines = 1;
	for (const wchar_t* i = String; *i != L'\0'; i++)
	{
		if (*i == L'\n') Lines++;
	}
	return Lines;
}


static UINT32 GetWidthOfString(const wchar_t* String);
static void DisplayHelpTokenizedString(const wchar_t* text, INT16 sx, INT16 sy);


static void DisplayFastHelp(MOUSE_REGION* const r)
{
	if (!(r->uiFlags & MSYS_FASTHELP)) return;

	INT32 const w = GetWidthOfString(r->FastHelpText) + 10;
	INT32 const h = GetNumberOfLinesInHeight(r->FastHelpText) * (GetFontHeight(FONT10ARIAL) + 1) + 8;

	INT32 x = r->RegionTopLeftX + 10;
	if (x <  0)                x = 0;
	if (x >= SCREEN_WIDTH - w) x = SCREEN_WIDTH - w - 4;

	INT32 y = r->RegionTopLeftY - h * 3 / 4;
	if (y <  0)                 y = 0;
	if (y >= SCREEN_HEIGHT - h) y = SCREEN_HEIGHT - h - 15;

	if (!(r->uiFlags & MSYS_GOT_BACKGROUND))
	{
		r->FastHelpRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT | BGND_FLAG_SAVERECT, x, y, w, h);
		r->uiFlags |= MSYS_GOT_BACKGROUND | MSYS_HAS_BACKRECT;
	}
	else
	{
		{ SGPVSurface::Lock l(FRAME_BUFFER);
			UINT16* const buf = l.Buffer<UINT16>();
			SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			RectangleDraw(TRUE, x + 1, y + 1, x + w - 1, y + h - 1, Get16BPPColor(FROMRGB( 65,  57, 15)), buf);
			RectangleDraw(TRUE, x,     y,     x + w - 2, y + h - 2, Get16BPPColor(FROMRGB(227, 198, 88)), buf);
		}
		FRAME_BUFFER->ShadowRect(x + 2, y + 2, x + w - 3, y + h - 3);
		FRAME_BUFFER->ShadowRect(x + 2, y + 2, x + w - 3, y + h - 3);

		DisplayHelpTokenizedString(r->FastHelpText, x + 5, y + 5);
		InvalidateRegion(x, y, x + w, y + h);
	}
}


static UINT32 GetWidthOfString(wchar_t const* const str)
{
	SGPFont const bold_font   = FONT10ARIALBOLD;
	SGPFont const normal_font = FONT10ARIAL;
	UINT32     max_w       = 0;
	UINT32     w           = 0;
	for (wchar_t const* i = str;; ++i)
	{
		wchar_t c = *i;
		SGPFont font;
		switch (c)
		{
			case L'\0':
				return MAX(w, max_w);

			case L'\n':
				max_w = MAX(w, max_w);
				w     = 0;
				continue;

			case L'|':
				c    = *++i;
				font = bold_font;
				break;

			default:
				font = normal_font;
				break;
		}
		w += GetCharWidth(font, c);
	}
}


static void DisplayHelpTokenizedString(wchar_t const* const text, INT16 const sx, INT16 const sy)
{
	SGPFont const bold_font   = FONT10ARIALBOLD;
	SGPFont const normal_font = FONT10ARIAL;
	INT32   const h           = GetFontHeight(normal_font) + 1;
	INT32         x           = sx;
	INT32         y           = sy;
	for (wchar_t const* i = text;; ++i)
	{
		wchar_t c = *i;
		SGPFont font;
		UINT8   foreground;
		switch (c)
		{
			case L'\0': return;

			case L'\n':
				x  = sx;
				y += h;
				continue;

			case L'|':
				c = *++i;
				font       = bold_font;
				foreground = 146;
				break;

			default:
				font       = normal_font;
				foreground = FONT_BEIGE;
				break;
		}
		SetFontAttributes(font, foreground);
		x += MPrintChar(x, y, c);
	}
}

void RenderFastHelp()
{
	static UINT32 last_clock;

	if (!gfRenderHilights) return;

	UINT32 const current_clock = GetClock();
	UINT32 const time_delta    = current_clock - last_clock;
	last_clock = current_clock;

	MOUSE_REGION* const r = MSYS_CurrRegion;
	if (!r || !r->FastHelpText) return;

	if (r->uiFlags & (MSYS_ALLOW_DISABLED_FASTHELP | MSYS_REGION_ENABLED))
	{
		if (r->FastHelpTimer == 0)
		{
			if (r->uiFlags & MSYS_MOUSE_IN_AREA)
			{
				r->uiFlags |= MSYS_FASTHELP;
			}
			else
			{
				r->uiFlags &= ~(MSYS_FASTHELP | MSYS_FASTHELP_RESET);
			}
			DisplayFastHelp(r);
		}
		else
		{
			if (r->uiFlags & MSYS_MOUSE_IN_AREA && r->ButtonState == 0)
			{
				r->FastHelpTimer -= time_delta;
				if (r->FastHelpTimer < 0) r->FastHelpTimer = 0;
			}
		}
	}
}


void MOUSE_REGION::AllowDisabledRegionFastHelp(bool const allow)
{
	if (allow)
	{
		uiFlags |= MSYS_ALLOW_DISABLED_FASTHELP;
	}
	else
	{
		uiFlags &= ~MSYS_ALLOW_DISABLED_FASTHELP;
	}
}
