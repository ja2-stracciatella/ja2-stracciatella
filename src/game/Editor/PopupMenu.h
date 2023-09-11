//popupmenu.h
//Author:  Kris Morness
//Created:  Sept 4, 1997

#ifndef __POPUPMENU_H
#define __POPUPMENU_H

#include "Button_System.h"

/*
POPUP MENU

DESIGN CONSIDERATIONS
There is currently no support for nested popup menus.  Let Kris know if
you need this.

USER INSTRUCTIONS
The goal of this popup menu system is to create a quick and intuitive
interface system which is functionally similar to Win95 and MacOS.
As soon as you click on a button which activates the popup menu, the user
will choose a selection in one of three ways:

  1) KEYBOARD - at anytime, the user can use the keyboard regardless of mouse
     mode.  The up/down arrow keys will cycle through the choices and enter will
     accept the choice.  ESC will cancel the menu without making a choice.
  2) When the mousedown event activates the menu, the user releases the mouse before
     entering the popup menu region.  The menu mode will change and become persistant.
     Clicking on the hilighted item will close the popup and activate the choice.
     Clicking elsewhere, the popup will be closed without making a selection.  This
     is functionally equivalent to Win95.
  3) When the mousedown event activates the menu, the user holds the mouse down and
     moves the cursor over the popup region.  At this moment the menu becomes
     non-persistant and a choice is made by releasing the mouse.  If the mouse is
     released on a highlighted choice, that choice is selected, otherwise the popup
     is cancelled.  This is functionally equivalent to MacOS.  The small diffence is
     that under Win95's standard convention, the release of the mouse outside of the
     region doesn't kill the menu, but in MacOS, it does.
*/


enum PopupMenuID
{
	CHANGETSET_POPUP,
	CHANGECIVGROUP_POPUP,
	SCHEDULEACTION_POPUP,
	ACTIONITEM_POPUP,
	OWNERSHIPGROUP_POPUP,
};

//The direction of the popup menu relative to the button
//pressed to activate it or mouse position.  In editor mode,
//this will attempt to go up and to the right of the button.
#define UPMASK			0x00
#define DNMASK			0x10
#define RTMASK			0x00
#define LTMASK			0x01
#define DIR_UPRIGHT	(UPMASK + RTMASK)
#define DIR_UPLEFT	(UPMASK + LTMASK)
#define DIR_DOWNRIGHT	(DNMASK + RTMASK)
#define DIR_DOWNLEFT	(DNMASK + LTMASK)

//These are the two main functions that outside users would call.
void InitPopupMenu(GUIButtonRef, PopupMenuID, UINT8 ubDirection);
BOOLEAN ProcessPopupMenuIfActive(void);

#endif
