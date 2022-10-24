// *****************************************************************************
//
// Filename :	MouseSystem.h
//
// Purpose :	Defines and typedefs for the "mousesystem" mouse region handler
//
// Modification history :
//
//		30jan97:Bret	-> Creation
//
// *****************************************************************************

#pragma once

#include <optional>

#include "JA2Types.h"
#include "Types.h"

#include <string_theory/string>

// Mouse Region Flags
#define MSYS_NO_FLAGS                0x00000000
#define MSYS_MOUSE_IN_AREA           0x00000001
#define MSYS_REGION_EXISTS           0x00000010
#define MSYS_REGION_ENABLED          0x00000040
#define MSYS_FASTHELP                0x00000080
// 0x100 and 0x200 removed, were used to check if this region has a background save.
#define MSYS_FASTHELP_RESET          0x00000400
#define MSYS_ALLOW_DISABLED_FASTHELP 0x00000800

struct MOUSE_REGION;

typedef std::function<void(MOUSE_REGION*, UINT32)> MOUSE_CALLBACK;

struct MOUSE_REGION
{
	void ChangeCursor(UINT16 crsr);

	void Enable()  { uiFlags |=  MSYS_REGION_ENABLED; }
	void Disable() { uiFlags &= ~MSYS_REGION_ENABLED; }

	void SetFastHelpText(const ST::string& str);

	void AllowDisabledRegionFastHelp(bool allow);

	void SetUserPtr(void* ptr) { user.ptr = ptr; }

	template<typename T> T* GetUserPtr() const { return static_cast<T*>(user.ptr); }

	bool HasFastHelp() { return FastHelpRect != nullptr; }

	INT16 X() const { return RegionTopLeftX; }
	INT16 Y() const { return RegionTopLeftY; }
	INT16 W() const { return RegionBottomRightX - RegionTopLeftX; }
	INT16 H() const { return RegionBottomRightY - RegionTopLeftY; }

	INT8   PriorityLevel; // Region's Priority, set by system and/or caller
	UINT32 uiFlags; // Region's state flags
	INT16  RegionTopLeftX; // Screen area affected by this region (absolute coordinates)
	INT16  RegionTopLeftY;
	INT16  RegionBottomRightX;
	INT16  RegionBottomRightY;
	INT16  MouseXPos; // Mouse's Coordinates in absolute screen coordinates
	INT16  MouseYPos;
	INT16  RelativeXPos; // Mouse's Coordinates relative to the Top-Left corner of the region
	INT16  RelativeYPos;
	UINT16 ButtonState; // Current state of the mouse buttons
	UINT16 Cursor; // Cursor to use when mouse in this region (see flags)
	MOUSE_CALLBACK MovementCallback; // Pointer to callback function if movement occured in this region
	MOUSE_CALLBACK ButtonCallback; // Pointer to callback function if button action occured in this region
	union // User Data, can be set to anything!
	{
		INT32 data[4];
		void* ptr;
	} user;

	//Fast help vars.
	INT16            FastHelpTimer; // Countdown timer for FastHelp text
	ST::utf32_buffer FastHelpText;  // Text string for the FastHelp (describes buttons if left there a while)
	BACKGROUND_SAVE* FastHelpRect;

	MOUSE_REGION* next; // List maintenance, do NOT touch these entries
	MOUSE_REGION* prev;
};

// Mouse region priorities
#define MSYS_PRIORITY_LOWEST			0
#define MSYS_PRIORITY_LOW				15
#define MSYS_PRIORITY_NORMAL			31
#define MSYS_PRIORITY_HIGH				63
#define MSYS_PRIORITY_HIGHEST			127

// Mouse system button masks
#define MSYS_LEFT_BUTTON			(1 << 0)
#define MSYS_RIGHT_BUTTON			(1 << 1)
#define MSYS_MIDDLE_BUTTON			(1 << 2)
#define MSYS_X1_BUTTON				(1 << 3)
#define MSYS_X2_BUTTON				(1 << 4)

// Mouse system special values
#define MSYS_NO_CALLBACK				nullptr
#define MSYS_NO_CURSOR					65534

// Mouse system callback reasons
#define MSYS_CALLBACK_REASON_NONE					0
#define MSYS_CALLBACK_REASON_MOVE					(1 << 0)
#define MSYS_CALLBACK_REASON_LBUTTON_DWN			(1 << 1)
#define MSYS_CALLBACK_REASON_LBUTTON_UP				(1 << 2)
#define MSYS_CALLBACK_REASON_LBUTTON_REPEAT			(1 << 3)
#define MSYS_CALLBACK_REASON_LBUTTON_DOUBLECLICK	(1 << 4)
#define MSYS_CALLBACK_REASON_RBUTTON_DWN			(1 << 5)
#define MSYS_CALLBACK_REASON_RBUTTON_UP				(1 << 6)
#define MSYS_CALLBACK_REASON_RBUTTON_REPEAT			(1 << 7)
#define MSYS_CALLBACK_REASON_MBUTTON_DWN			(1 << 8)
#define MSYS_CALLBACK_REASON_MBUTTON_UP				(1 << 9)
#define MSYS_CALLBACK_REASON_MBUTTON_REPEAT			(1 << 10)
#define MSYS_CALLBACK_REASON_X1BUTTON_DWN			(1 << 11)
#define MSYS_CALLBACK_REASON_X1BUTTON_UP			(1 << 12)
#define MSYS_CALLBACK_REASON_X1BUTTON_REPEAT		(1 << 13)
#define MSYS_CALLBACK_REASON_X2BUTTON_DWN			(1 << 14)
#define MSYS_CALLBACK_REASON_X2BUTTON_UP			(1 << 15)
#define MSYS_CALLBACK_REASON_X2BUTTON_REPEAT		(1 << 16)
#define MSYS_CALLBACK_REASON_LOST_MOUSE				(1 << 17)
#define MSYS_CALLBACK_REASON_GAIN_MOUSE				(1 << 18)
#define MSYS_CALLBACK_REASON_WHEEL_UP				(1 << 19)
#define MSYS_CALLBACK_REASON_WHEEL_DOWN				(1 << 20)
#define MSYS_CALLBACK_REASON_TFINGER_DWN	   		(1 << 21)
#define MSYS_CALLBACK_REASON_TFINGER_UP     		(1 << 22)
#define MSYS_CALLBACK_REASON_TFINGER_REPEAT 		(1 << 23)
#define MSYS_CALLBACK_REASON_TFINGER_DOUBLETAP 		(1 << 24)

// Composites

#define MSYS_CALLBACK_REASON_POINTER_DWN (MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_TFINGER_DWN)
#define MSYS_CALLBACK_REASON_POINTER_UP (MSYS_CALLBACK_REASON_LBUTTON_UP | MSYS_CALLBACK_REASON_TFINGER_UP)
#define MSYS_CALLBACK_REASON_POINTER_REPEAT (MSYS_CALLBACK_REASON_LBUTTON_REPEAT | MSYS_CALLBACK_REASON_TFINGER_REPEAT)
#define MSYS_CALLBACK_REASON_POINTER_DOUBLECLICK (MSYS_CALLBACK_REASON_LBUTTON_DOUBLECLICK | MSYS_CALLBACK_REASON_TFINGER_DOUBLETAP)
#define MSYS_CALLBACK_REASON_ANY_BUTTON_DWN (MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_RBUTTON_DWN | MSYS_CALLBACK_REASON_MBUTTON_DWN | MSYS_CALLBACK_REASON_X1BUTTON_DWN | MSYS_CALLBACK_REASON_X2BUTTON_DWN | MSYS_CALLBACK_REASON_TFINGER_DWN)
#define MSYS_CALLBACK_REASON_ANY_BUTTON_UP (MSYS_CALLBACK_REASON_LBUTTON_UP | MSYS_CALLBACK_REASON_RBUTTON_UP | MSYS_CALLBACK_REASON_MBUTTON_UP | MSYS_CALLBACK_REASON_X1BUTTON_UP | MSYS_CALLBACK_REASON_X2BUTTON_UP | MSYS_CALLBACK_REASON_TFINGER_UP)

// Internal Functions
void MSYS_SetCurrentCursor(UINT16 Cursor);

// External
void MSYS_Init(void);
void MSYS_Shutdown(void);
void MSYS_DefineRegion(MOUSE_REGION *region,UINT16 tlx,UINT16 tly,UINT16 brx,UINT16 bry,INT8 priority,
					   UINT16 crsr,MOUSE_CALLBACK movecallback,MOUSE_CALLBACK buttoncallback);
void MSYS_RemoveRegion(MOUSE_REGION *region);

/* Set one of the user data entries in a mouse region */
void MSYS_SetRegionUserData(MOUSE_REGION*, UINT32 index, INT32 userdata);

/* Retrieve one of the user data entries in a mouse region */
INT32 MSYS_GetRegionUserData(MOUSE_REGION const*, UINT32 index);

// Create a callback with primary and secondary and all actions callbacks
// Primary action will be triggered on left mouse button up (or mouse button down if triggerOnMouseDown is true), or short touch
// Secondary action will be triggered on right mouse button up, or first touch repeat event
// The all events callback will be triggered for all events (useful to add other behavior, such as scrolling)
// This function needs to be specialized for mouse regions and buttons
template<typename T, UINT32 EXISTS_FLAG>
std::function<void(T*, UINT32)> CallbackPrimarySecondary(
	std::function<void(T*, UINT32)> primaryAction,
	std::function<void(T*, UINT32)> secondaryAction,
	std::function<void(T*, UINT32)> allEvents = nullptr,
	bool triggerPrimaryOnMouseDown = false
)
{
	BOOLEAN fTouchRepeatHandled = FALSE;
	BOOLEAN fPointerDown = FALSE;
	BOOLEAN fRightMouseButtonDown = FALSE;
	UINT32 mouseReason = triggerPrimaryOnMouseDown ? MSYS_CALLBACK_REASON_LBUTTON_DWN : MSYS_CALLBACK_REASON_LBUTTON_UP;

	return [fTouchRepeatHandled, fPointerDown, fRightMouseButtonDown, mouseReason, primaryAction, secondaryAction, allEvents](T* r, UINT32 reason) mutable {
		if (reason & MSYS_CALLBACK_REASON_POINTER_DWN) {
			fTouchRepeatHandled = FALSE;
			fPointerDown = TRUE;
			fRightMouseButtonDown = FALSE;
		}
		if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN) {
			fTouchRepeatHandled = FALSE;
			fPointerDown = FALSE;
			fRightMouseButtonDown = TRUE;
		}
		if (((reason & mouseReason) && fPointerDown) || ((reason & MSYS_CALLBACK_REASON_TFINGER_UP) && fPointerDown && !fTouchRepeatHandled))
		{
			// We need to guard against deletions of the mouse region during callbacks
			if (r->uiFlags & EXISTS_FLAG && primaryAction) {
				primaryAction(r, reason);
			}
		}
		if (((reason & MSYS_CALLBACK_REASON_RBUTTON_UP) && fRightMouseButtonDown) || ((reason & MSYS_CALLBACK_REASON_TFINGER_REPEAT) && fPointerDown && !fTouchRepeatHandled))
		{
			if (reason & MSYS_CALLBACK_REASON_TFINGER_REPEAT) {
				fTouchRepeatHandled = TRUE;
			}
			if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP) {
				fRightMouseButtonDown = FALSE;
			}
			// We need to guard against deletions of the mouse region during callbacks
			if (r->uiFlags & EXISTS_FLAG && secondaryAction) {
				secondaryAction(r, reason);
			}
		}
		// We need to guard against deletions of the mouse region during callbacks
		if (r->uiFlags & EXISTS_FLAG && allEvents) {
			allEvents(r, reason);
		}
	};
}

MOUSE_CALLBACK MouseCallbackPrimarySecondary(
	MOUSE_CALLBACK primaryAction,
	MOUSE_CALLBACK secondaryAction,
	MOUSE_CALLBACK allEvents = nullptr,
	bool triggerPrimaryOnMouseDown = false
);

// This function will force a re-evaluation of mous regions
// Usually used to force change of mouse cursor if panels switch, etc
void RefreshMouseRegions(void);

// Now also used by Wizardry -- DB
void RenderFastHelp(void);

// Hook to the SGP's mouse handler
void MouseSystemHook(UINT16 type, UINT32 button, UINT16 x, UINT16 y);

class MouseRegion : private MOUSE_REGION
{
	public:
		MouseRegion(UINT16 const x, UINT16 const y, UINT16 const w, UINT16 const h, INT8 const priority, UINT16 const cursor, MOUSE_CALLBACK const movecallback, MOUSE_CALLBACK const buttoncallback)
		{
			MOUSE_REGION* const r = this;
			*r = MOUSE_REGION{};
			MSYS_DefineRegion(r, x, y, x + w, y + h, priority, cursor, movecallback, buttoncallback);
		}

		~MouseRegion()
		{
			MSYS_RemoveRegion(this);
		}

		using MOUSE_REGION::ChangeCursor;
		using MOUSE_REGION::Disable;
		using MOUSE_REGION::Enable;
		using MOUSE_REGION::MouseXPos;
		using MOUSE_REGION::MouseYPos;
		using MOUSE_REGION::PriorityLevel;
		using MOUSE_REGION::RegionBottomRightX;
		using MOUSE_REGION::RegionBottomRightY;
		using MOUSE_REGION::RegionTopLeftX;
		using MOUSE_REGION::RegionTopLeftY;
		using MOUSE_REGION::RelativeXPos;
		using MOUSE_REGION::RelativeYPos;
		using MOUSE_REGION::SetFastHelpText;
		using MOUSE_REGION::SetUserPtr;
		using MOUSE_REGION::uiFlags;
};
