//=================================================================================================
//	MouseSystem_Macros.h
//
//	Macro definitions for the "mousesystem" mouse region handler.
//
//	This file is included by "mousesystem.h" or can be included by itself.
//
//	Written by Bret Rowdon. Jan 30 '97
//=================================================================================================

#ifndef _MOUSE_SYSTEM_MACROS_H_
#define _MOUSE_SYSTEM_MACROS_H_

// Special macro hook for the mouse handler. Allows a call to a secondary mouse handler.
// Define the label _MOUSE_SYSTEM_HOOK_ to activate. Undef it to deactivate.
//
// The actual function prototype is shown below

#define _MOUSE_SYSTEM_HOOK_

#ifdef _MOUSE_SYSTEM_HOOK_
	#define MouseSystemHook(t,x,y,l,r)		MSYS_SGP_Mouse_Handler_Hook(t,x,y,l,r)
#else
	#define MouseSystemHook(t,x,y,l,r)
#endif



#ifdef __cplusplus
extern "C" {
#endif

// Special prototype for mouse handler hook
extern void MSYS_SGP_Mouse_Handler_Hook(UINT16 Type, UINT16 Xcoord, UINT16 Ycoord, BOOLEAN LeftButton, BOOLEAN RightButton);

#ifdef __cplusplus
}
#endif

#endif
