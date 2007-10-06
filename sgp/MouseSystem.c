
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

#include "Font.h"
#include "Types.h"
#include "Debug.h"
#include "Input.h"
#include "MemMan.h"
#include "Line.h"
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


#define BASE_REGION_FLAGS (MSYS_REGION_ENABLED)


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


extern void ReleaseAnchorMode();  //private function used here (implemented in Button System.c)


static INT32 MSYS_ScanForID = FALSE;
static INT32 MSYS_CurrentID = MSYS_ID_SYSTEM;

INT16 MSYS_CurrentMX=0;
INT16 MSYS_CurrentMY=0;
static INT16 MSYS_CurrentButtons = 0;
static INT16 MSYS_Action         = 0;

static BOOLEAN MSYS_SystemInitialized   = FALSE;

static UINT16  gusClickedIDNumber;
static BOOLEAN gfClickedModeOn = FALSE;

static MOUSE_REGION* MSYS_RegList = NULL;

MOUSE_REGION *MSYS_PrevRegion = NULL;
static MOUSE_REGION* MSYS_CurrRegion = NULL;

//When set, the fast help text will be instantaneous, if consecutive regions with help text are
//hilighted.  It is set, whenever the timer for the first help button expires, and the mode is
//cleared as soon as the cursor moves into no region or a region with no helptext.
BOOLEAN gfPersistantFastHelpMode;

static const INT16 gsFastHelpDelay = 600; // In timer ticks


//Kris:
//NOTE:  This doesn't really need to be here, however, it is a good indication that
//when an error appears here, that you need to go below to the init code and initialize the
//values there as well.  That's the only reason why I left this here.
static MOUSE_REGION MSYS_SystemBaseRegion = {
								MSYS_ID_SYSTEM, MSYS_PRIORITY_SYSTEM, BASE_REGION_FLAGS,
								-32767, -32767, 32767, 32767, 0, 0, 0, 0, 0, 0,
								MSYS_NO_CALLBACK, MSYS_NO_CALLBACK, { 0,0,0,0 },
								0, 0, -1, NULL, NULL };

static BOOLEAN gfRefreshUpdate = FALSE;

//Kris:  December 3, 1997
//Special internal debugging utilities that will ensure that you don't attempt to delete
//an already deleted region.  It will also ensure that you don't create an identical region
//that already exists.
//TO REMOVE ALL DEBUG FUNCTIONALITY:  simply comment out MOUSESYSTEM_DEBUGGING definition
#if defined _DEBUG && !defined BOUNDS_CHECKER
#	define MOUSESYSTEM_DEBUGGING
#endif

#ifdef MOUSESYSTEM_DEBUGGING
BOOLEAN gfIgnoreShutdownAssertions;
#endif


static void MSYS_TrashRegList(void);
static void MSYS_AddRegionToList(MOUSE_REGION* region);


//======================================================================================================
//	MSYS_Init
//
//	Initialize the mouse system.
//
INT32 MSYS_Init(void)
{
	#ifdef MOUSESYSTEM_DEBUGGING
		gfIgnoreShutdownAssertions = FALSE;
	#endif
	if(MSYS_RegList!=NULL)
		MSYS_TrashRegList();

	MSYS_CurrentID = MSYS_ID_SYSTEM;
	MSYS_ScanForID = FALSE;

	MSYS_CurrentMX = 0;
	MSYS_CurrentMY = 0;
	MSYS_CurrentButtons = 0;
	MSYS_Action=MSYS_NO_ACTION;

	MSYS_PrevRegion = NULL;
	MSYS_SystemInitialized = TRUE;

	// Setup the system's background region
	MSYS_SystemBaseRegion.IDNumber						= MSYS_ID_SYSTEM;
	MSYS_SystemBaseRegion.PriorityLevel				= MSYS_PRIORITY_SYSTEM;
	MSYS_SystemBaseRegion.uiFlags								= BASE_REGION_FLAGS;
	MSYS_SystemBaseRegion.RegionTopLeftX			= -32767;
	MSYS_SystemBaseRegion.RegionTopLeftY			= -32767;
	MSYS_SystemBaseRegion.RegionBottomRightX	= 32767;
	MSYS_SystemBaseRegion.RegionBottomRightY	= 32767;
	MSYS_SystemBaseRegion.MouseXPos						= 0;
	MSYS_SystemBaseRegion.MouseYPos						= 0;
	MSYS_SystemBaseRegion.RelativeXPos				= 0;
	MSYS_SystemBaseRegion.RelativeYPos				= 0;
	MSYS_SystemBaseRegion.ButtonState					= 0;
	MSYS_SystemBaseRegion.Cursor							= 0;
	MSYS_SystemBaseRegion.UserData[0]					= 0;
	MSYS_SystemBaseRegion.UserData[1]					= 0;
	MSYS_SystemBaseRegion.UserData[2]					= 0;
	MSYS_SystemBaseRegion.UserData[3]					= 0;
	MSYS_SystemBaseRegion.MovementCallback		= MSYS_NO_CALLBACK;
	MSYS_SystemBaseRegion.ButtonCallback			= MSYS_NO_CALLBACK;

	MSYS_SystemBaseRegion.FastHelpTimer				= 0;
	MSYS_SystemBaseRegion.FastHelpText				= 0;
	MSYS_SystemBaseRegion.FastHelpRect				= NO_BGND_RECT;

	MSYS_SystemBaseRegion.next								= NULL;
	MSYS_SystemBaseRegion.prev								= NULL;

	// Add the base region to the list
	MSYS_AddRegionToList(&MSYS_SystemBaseRegion);

	return(1);
}



//======================================================================================================
//	MSYS_Shutdown
//
//	De-inits the "mousesystem" mouse region handling code.
//
void MSYS_Shutdown(void)
{
	#ifdef MOUSESYSTEM_DEBUGGING
		gfIgnoreShutdownAssertions = TRUE;
	#endif
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
#ifdef JA2
			/* Kris:
			 * Used only if applicable.  This is used for that special button that is
			 * locked with the mouse press -- just like windows.  When you release the
			 * button, the previous state of the button is restored if you released
			 * the mouse outside of it's boundaries.  If you release inside of the
			 * button, the action is selected -- but later in the code.
			 * NOTE:  It has to be here, because the mouse can be released anywhere
			 *        regardless of regions, buttons, etc. */
			ReleaseAnchorMode();
#endif
			action |= MSYS_DO_LBUTTON_UP;
			goto update_buttons;

		case RIGHT_BUTTON_DOWN: action |= MSYS_DO_RBUTTON_DWN; goto update_buttons;
		case RIGHT_BUTTON_UP:   action |= MSYS_DO_RBUTTON_UP;  goto update_buttons;

update_buttons:
			MSYS_CurrentButtons &= ~(MSYS_LEFT_BUTTON | MSYS_RIGHT_BUTTON);
			MSYS_CurrentButtons |= (_LeftButtonDown  ? MSYS_LEFT_BUTTON  : 0);
			MSYS_CurrentButtons |= (_RightButtonDown ? MSYS_RIGHT_BUTTON : 0);
			break;

		// ATE: Checks here for mouse button repeats.....
		// Call mouse region with new reason
		case LEFT_BUTTON_REPEAT:  action |= MSYS_DO_LBUTTON_REPEAT; break;
		case RIGHT_BUTTON_REPEAT: action |= MSYS_DO_RBUTTON_REPEAT; break;

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
//	MSYS_GetNewID
//
//	Returns a unique ID number for region nodes. If no new ID numbers can be found, the MAX value
//	is returned.
//
static INT32 MSYS_GetNewID(void)
{
	INT32 retID;
	INT32 Current,found,done;
	MOUSE_REGION *node;

	retID = MSYS_CurrentID;
	MSYS_CurrentID++;

	// Crapy scan for an unused ID
	if((MSYS_CurrentID >= MSYS_ID_MAX) || MSYS_ScanForID)
	{
		MSYS_ScanForID = TRUE;
		Current = MSYS_ID_BASE;
		done=found=FALSE;
		while(!done)
		{
			found=FALSE;
			node=MSYS_RegList;
			while(node!=NULL && !found)
			{
				if(node->IDNumber == Current)
					found=TRUE;
			}

			if(found && Current < MSYS_ID_MAX)	// Current ID is in use, and their are more to scan
				Current++;
			else
			{
				done=TRUE;						// Got an ID to use.
				if(found)
					Current=MSYS_ID_MAX;		// Ooops, ran out of IDs, use MAX value!
			}
		}
		MSYS_CurrentID = Current;
	}

	return(retID);
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


static void MSYS_DeleteRegionFromList(MOUSE_REGION* region);


//======================================================================================================
//	MSYS_AddRegionToList
//
//	Add a region struct to the current list. The list is sorted by priority levels. If two entries
//	have the same priority level, then the latest to enter the list gets the higher priority.
//
static void MSYS_AddRegionToList(MOUSE_REGION* region)
{
	MOUSE_REGION *curr;
	INT32 done;


	// If region seems to already be in list, delete it so we can
	// re-insert the region.
	if( region->next || region->prev )
	{ // if it wasn't actually there, then call does nothing!
		MSYS_DeleteRegionFromList(region);
	}

	// Set an ID number!
	region->IDNumber = (UINT16)MSYS_GetNewID();

	region->next = NULL;
	region->prev = NULL;

	if( !MSYS_RegList )
	{ // Null list, so add it straight up.
		MSYS_RegList = region;
	}
	else
	{
		// Walk down list until we find place to insert (or at end of list)
		curr=MSYS_RegList;
		done=FALSE;
		while((curr->next != NULL) && !done)
		{
			if(curr->PriorityLevel <= region->PriorityLevel)
				done=TRUE;
			else
				curr = curr->next;
		}

		if(curr->PriorityLevel > region->PriorityLevel)
		{
			// Add after curr node
			region->next = curr->next;
			curr->next = region;
			region->prev = curr;
			if(region->next != NULL)
				region->next->prev = region;
		}
		else
		{
			// Add before curr node
			region->next = curr;
			region->prev = curr->prev;

			curr->prev = region;
			if(region->prev != NULL)
				region->prev->next = region;

			if(MSYS_RegList==curr)	// Make sure if adding at start, to adjust the list pointer
				MSYS_RegList=region;
		}
	}
}



//======================================================================================================
//	MSYS_RegionInList
//
//	Scan region list for presence of a node with the same region ID number
//
static INT32 MSYS_RegionInList(const MOUSE_REGION* region)
{
	MOUSE_REGION *Current;
	INT32 found;

	found = FALSE;
	Current = MSYS_RegList;
	while( Current && !found )
	{
		if(Current->IDNumber == region->IDNumber)
			found=TRUE;
		Current = Current->next;
	}
	return(found);
}



//======================================================================================================
//	MSYS_DeleteRegionFromList
//
//	Removes a region from the current list.
//
static void MSYS_DeleteRegionFromList(MOUSE_REGION* region)
{
	// If no list present, there's nothin' to do.
	if( !MSYS_RegList )
		return;

	// Check if region in list
	if(!MSYS_RegionInList(region))
		return;

	// Remove a node from the list
	if(MSYS_RegList == region)
	{ // First node on list, adjust main pointer.
		MSYS_RegList = region->next;
		if(MSYS_RegList!=NULL)
			MSYS_RegList->prev = NULL;
		region->next = region->prev = NULL;
	}
	else
	{
		if(region->prev)
			region->prev->next = region->next;
		// If not last node in list, adjust following node's ->prev entry.
		if( region->next )
			region->next->prev = region->prev;
		region->prev = region->next = NULL;
	}

	// Is only the system background region remaining?
	if(MSYS_RegList == &MSYS_SystemBaseRegion)
	{
		// Yup, so let's reset the ID values!
		MSYS_CurrentID = MSYS_ID_BASE;
		MSYS_ScanForID = FALSE;
	}
	else if(MSYS_RegList == NULL)
	{
		// Ack, we actually emptied the list, so let's reset for re-init possibilities
		MSYS_CurrentID = MSYS_ID_SYSTEM;
		MSYS_ScanForID = FALSE;
	}
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

			cur->FastHelpTimer = gsFastHelpDelay;

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
			//Kris -- October 27, 1997
			//Implemented gain mouse region
			if (cur->MovementCallback != NULL)
			{
				if (cur->FastHelpText && !(cur->uiFlags & MSYS_FASTHELP_RESET))
				{
					cur->FastHelpTimer = gsFastHelpDelay;
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
		if (!gfClickedModeOn || gusClickedIDNumber == cur->IDNumber)
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
						gfClickedModeOn = TRUE;
						// Set global ID
						gusClickedIDNumber = cur->IDNumber;
					}

					if (MSYS_Action & MSYS_DO_LBUTTON_UP)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_UP;
						gfClickedModeOn = FALSE;
					}

					if (MSYS_Action & MSYS_DO_RBUTTON_DWN)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_DWN;
						gfClickedModeOn = TRUE;
						// Set global ID
						gusClickedIDNumber = cur->IDNumber;
					}

					if (MSYS_Action & MSYS_DO_RBUTTON_UP)
					{
						ButtonReason |= MSYS_CALLBACK_REASON_RBUTTON_UP;
						gfClickedModeOn = FALSE;
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
			if (MSYS_Action & MSYS_DO_RBUTTON_UP) gfClickedModeOn = FALSE;
			if (MSYS_Action & MSYS_DO_LBUTTON_UP) gfClickedModeOn = FALSE;

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
	MSYS_PrevRegion = cur;
}



//=================================================================================================
//	MSYS_DefineRegion
//
//	Inits a MOUSE_REGION structure for use with the mouse system
//
void MSYS_DefineRegion(MOUSE_REGION *region,UINT16 tlx,UINT16 tly,UINT16 brx,UINT16 bry,INT8 priority,
					   UINT16 crsr,MOUSE_CALLBACK movecallback,MOUSE_CALLBACK buttoncallback)
{
	#ifdef MOUSESYSTEM_DEBUGGING
		if( region->uiFlags & MSYS_REGION_EXISTS )
			AssertMsg( 0, "Attempting to define a region that already exists." );
	#endif

	region->IDNumber = MSYS_ID_BASE;

	if(priority == MSYS_PRIORITY_AUTO)
		priority = MSYS_PRIORITY_BASE;
	else if(priority <= MSYS_PRIORITY_LOWEST)
		priority = MSYS_PRIORITY_LOWEST;
	else if(priority >= MSYS_PRIORITY_HIGHEST)
		priority = MSYS_PRIORITY_HIGHEST;

	region->PriorityLevel = priority;

	region->uiFlags = MSYS_NO_FLAGS;

	region->MovementCallback = movecallback;
	region->ButtonCallback = buttoncallback;

	region->Cursor = crsr;

	region->RegionTopLeftX = tlx;
	region->RegionTopLeftY = tly;
	region->RegionBottomRightX = brx;
	region->RegionBottomRightY = bry;

	region->MouseXPos = 0;
	region->MouseYPos = 0;
	region->RelativeXPos = 0;
	region->RelativeYPos = 0;
	region->ButtonState	= 0;

	//Init fasthelp
	region->FastHelpText = NULL;
	region->FastHelpTimer = 0;

	region->next = NULL;
	region->prev = NULL;

	//Add region to system list
	MSYS_AddRegionToList(region);
	region->uiFlags|= MSYS_REGION_ENABLED | MSYS_REGION_EXISTS;


	// Dirty our update flag
	gfRefreshUpdate = TRUE;
}



//=================================================================================================
//	MSYS_ChangeRegionCursor
//
void MSYS_ChangeRegionCursor(MOUSE_REGION *region,UINT16 crsr)
{
	region->Cursor = crsr;
	if (crsr != MSYS_NO_CURSOR && region->uiFlags & MSYS_MOUSE_IN_AREA)
	{
		MSYS_SetCurrentCursor(crsr);
	}
}


//=================================================================================================
//	MSYS_RemoveRegion
//
//	Removes a region from the list, disables it, then calls the callback functions for
//	de-initialization.
//
void MSYS_RemoveRegion(MOUSE_REGION *region)
{
	if( !region )
	{
		#ifdef MOUSESYSTEM_DEBUGGING
		if( gfIgnoreShutdownAssertions )
		#endif
			return;
		AssertMsg( 0, "Attempting to remove a NULL region.");
	}
	#ifdef MOUSESYSTEM_DEBUGGING
	if( !(region->uiFlags & MSYS_REGION_EXISTS) )
		AssertMsg( 0, "Attempting to remove an already removed region." );
	#endif

#ifdef _JA2_RENDER_DIRTY
	if( region->uiFlags & MSYS_HAS_BACKRECT )
	{
		FreeBackgroundRectPending( region->FastHelpRect );
		region->uiFlags &= (~MSYS_HAS_BACKRECT);


	}
#endif

	// Get rid of the FastHelp text (if applicable)
	if( region->FastHelpText )
	{
		MemFree( region->FastHelpText );
	}
	region->FastHelpText = NULL;

	MSYS_DeleteRegionFromList(region);

	//if the previous region is the one that we are deleting, reset the previous region
	if ( MSYS_PrevRegion == region )
		MSYS_PrevRegion = NULL;
	//if the current region is the one that we are deleting, then clear it.
	if( MSYS_CurrRegion == region )
		MSYS_CurrRegion = NULL;

	//dirty our update flag
	gfRefreshUpdate = TRUE;

	// Check if this is a locked region, and unlock if so
	if ( gfClickedModeOn )
	{
		// Set global ID
		if ( gusClickedIDNumber == region->IDNumber )
		{
			gfClickedModeOn = FALSE;
		}
	}

	//clear all internal values (including the region exists flag)
	memset( region, 0, sizeof( MOUSE_REGION ) );
}



//=================================================================================================
//	MSYS_EnableRegion
//
//	Enables a mouse region.
//
void MSYS_EnableRegion(MOUSE_REGION *region)
{
	region->uiFlags |= MSYS_REGION_ENABLED;
}



//=================================================================================================
//	MSYS_DisableRegion
//
//	Disables a mouse region without removing it from the system list.
//
void MSYS_DisableRegion(MOUSE_REGION *region)
{
	region->uiFlags &= (~MSYS_REGION_ENABLED);
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


//=================================================================================================
//	MSYS_SetRegionUserData
//
//	Sets one of the four user data entries in a mouse region
//
void MSYS_SetRegionUserData(MOUSE_REGION *region,INT32 index,INT32 userdata)
{
	if(index < 0 || index > 3)
	{
		#ifdef MOUSESYSTEM_DEBUGGING
		if( gfIgnoreShutdownAssertions )
		#endif
			return;
		char str[80];
		sprintf( str, "Attempting MSYS_SetRegionUserData() with out of range index %d.", index );
		AssertMsg( 0, str );
	}
	region->UserData[index]=userdata;
}



//=================================================================================================
//	MSYS_GetRegionUserData
//
//	Retrieves one of the four user data entries in a mouse region
//
INT32 MSYS_GetRegionUserData(MOUSE_REGION *region,INT32 index)
{
	if(index < 0 || index > 3)
	{
		#ifdef MOUSESYSTEM_DEBUGGING
		if( gfIgnoreShutdownAssertions )
		#endif
			return 0;
		char str[80];
		sprintf( str, "Attempting MSYS_GetRegionUserData() with out of range index %d", index );
		AssertMsg( 0, str );
	}
	return(region->UserData[index]);
}


// This function will force a re-evaluation of mouse regions
// Usually used to force change of mouse cursor if panels switch, etc
void RefreshMouseRegions( )
{
	MSYS_Action|=MSYS_DO_MOVE;

	MSYS_UpdateMouseRegion( );

}

void SetRegionFastHelpText( MOUSE_REGION *region, const wchar_t *szText )
{
	Assert( region );

	if( region->FastHelpText )
		MemFree( region->FastHelpText );

	region->FastHelpText = NULL;
//	region->FastHelpTimer = 0;
	if( !(region->uiFlags & MSYS_REGION_EXISTS) )
	{
		return;
		//AssertMsg( 0, String( "Attempting to set fast help text, \"%ls\" to an inactive region.", szText ) );
	}

	if( !szText || !wcslen( szText ) )
		return; //blank (or clear)

	// Allocate memory for the button's FastHelp text string...
	region->FastHelpText = (wchar_t*)MemAlloc( (wcslen( szText ) + 1) * sizeof(wchar_t) );
	Assert( region->FastHelpText );

	wcscpy( region->FastHelpText, szText );

  // ATE: We could be replacing already existing, active text
  // so let's remove the region so it be rebuilt...

	#ifdef JA2
	if ( guiCurrentScreen != MAP_SCREEN )
	{
	#endif

	#ifdef _JA2_RENDER_DIRTY
	  if( region->uiFlags & MSYS_GOT_BACKGROUND )
		  FreeBackgroundRectPending( region->FastHelpRect );
  #endif

  region->uiFlags &= (~MSYS_GOT_BACKGROUND);
  region->uiFlags &= (~MSYS_FASTHELP_RESET);

	#ifdef JA2
	}
	#endif

	//region->FastHelpTimer = gsFastHelpDelay;
}


static size_t GetNumberOfLinesInHeight(const wchar_t* String)
{
	size_t Lines = 1;
	for (const wchar_t* i = String; *i != L'\0'; i++)
	{
		if (*i == L'\n') Lines++;
	}
	return Lines;
}


static size_t GetWidthOfString(const wchar_t* String);
static void DisplayHelpTokenizedString(const wchar_t* pStringA, INT16 sX, INT16 sY);


//=============================================================================
//	DisplayFastHelp
//
//
static void DisplayFastHelp(MOUSE_REGION* region)
{
	if ( region->uiFlags & MSYS_FASTHELP )
	{
		INT32 iW = GetWidthOfString(region->FastHelpText) + 10;
		INT32 iH = GetNumberOfLinesInHeight(region->FastHelpText) * (GetFontHeight(FONT10ARIAL) + 1) + 8;

		INT32 iX = region->RegionTopLeftX + 10;

		if (iX < 0)
			iX = 0;

		if ( (iX + iW) >= SCREEN_WIDTH )
			iX = (SCREEN_WIDTH - iW - 4);

		INT32 iY = region->RegionTopLeftY - iH * 3 / 4;
		if (iY < 0)
			iY = 0;

		if ( (iY + iH) >= SCREEN_HEIGHT )
			iY = (SCREEN_HEIGHT - iH - 15);

		if ( !(region->uiFlags & MSYS_GOT_BACKGROUND) )
		{
			region->FastHelpRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT | BGND_FLAG_SAVERECT, iX, iY, iX + iW, iY + iH);
			region->uiFlags |= MSYS_GOT_BACKGROUND;
			region->uiFlags |= MSYS_HAS_BACKRECT;
		}
		else
		{
			UINT8 *pDestBuf;
			UINT32 uiDestPitchBYTES;
			pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
			SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			RectangleDraw( TRUE, iX + 1, iY + 1, iX + iW - 1, iY + iH - 1, Get16BPPColor( FROMRGB( 65, 57, 15 ) ), pDestBuf );
			RectangleDraw( TRUE, iX, iY, iX + iW - 2, iY + iH - 2, Get16BPPColor( FROMRGB( 227, 198, 88 ) ), pDestBuf );
			UnLockVideoSurface( FRAME_BUFFER );
			ShadowVideoSurfaceRect( FRAME_BUFFER, iX + 2, iY + 2, iX + iW - 3, iY + iH - 3 );
			ShadowVideoSurfaceRect( FRAME_BUFFER, iX + 2, iY + 2, iX + iW - 3, iY + iH - 3 );

			SetFont( FONT10ARIAL );
			SetFontShadow( FONT_NEARBLACK );
			DisplayHelpTokenizedString( region->FastHelpText ,( INT16 )( iX + 5 ), ( INT16 )( iY + 5 ) );
			InvalidateRegion(  iX, iY, (iX + iW) , (iY + iH) );
		}
	}
}


static size_t GetWidthOfString(const wchar_t* pStringA)
{
	HVOBJECT Font = GetFontObject(FONT10ARIAL);
	size_t MaxWidth = 0;
	size_t Width = 0;
	for (const wchar_t* i = pStringA;; i++)
	{
		switch (*i)
		{
			case L'\0':
				return max(Width, MaxWidth);

			case L'\n':
				MaxWidth = max(Width, MaxWidth);
				Width = 0;
				break;

			default:
				Width += GetCharWidth(Font, *i);
				break;
		}
	}
}


static void DisplayHelpTokenizedString(const wchar_t* pStringA, INT16 sX, INT16 sY)
{
	HVOBJECT BoldFont   = GetFontObject(FONT10ARIALBOLD);
	HVOBJECT NormalFont = GetFontObject(FONT10ARIAL);
	INT32 h = GetFontHeight(FONT10ARIAL) + 1;
	INT32 x = sX;
	INT32 y = sY;
	for (const wchar_t* i = pStringA;; i++)
	{
		wchar_t c = *i;
		HVOBJECT Font;
		switch (c)
		{
			case L'\0': return;

			case L'\n':
				x = sX;
				y += h;
				continue;

			case L'|':
				c = *++i;
				SetFont(FONT10ARIALBOLD);
				SetFontForeground(146);
				Font = BoldFont;
				break;

			default:
				SetFont(FONT10ARIAL);
				SetFontForeground(FONT_BEIGE);
				Font = NormalFont;
				break;
		}
		mprintf(x, y, L"%lc", c);
		x += GetCharWidth(Font, c);
	}
}

void RenderFastHelp()
{
	static INT32 iLastClock;
	INT32 iTimeDifferential, iCurrentClock;

	if( !gfRenderHilights )
		return;

	iCurrentClock = GetClock();
	iTimeDifferential = iCurrentClock - iLastClock;
	if (iTimeDifferential < 0)
		iTimeDifferential += 0x7fffffff;
	iLastClock = iCurrentClock;

	if( MSYS_CurrRegion && MSYS_CurrRegion->FastHelpText )
	{
		if( !MSYS_CurrRegion->FastHelpTimer )
		{
			if( MSYS_CurrRegion->uiFlags & ( MSYS_ALLOW_DISABLED_FASTHELP | MSYS_REGION_ENABLED ) )
			{
				if( MSYS_CurrRegion->uiFlags & MSYS_MOUSE_IN_AREA )
					MSYS_CurrRegion->uiFlags |= MSYS_FASTHELP;
				else
				{
					MSYS_CurrRegion->uiFlags &= ( ~( MSYS_FASTHELP | MSYS_FASTHELP_RESET ) );
				}
				//Do I really need this?
				//MSYS_CurrRegion->uiFlags |= REGION_DIRTY;
				DisplayFastHelp( MSYS_CurrRegion );
			}
		}
		else
		{
			if( MSYS_CurrRegion->uiFlags & ( MSYS_ALLOW_DISABLED_FASTHELP | MSYS_REGION_ENABLED ) )
			{
				if ( MSYS_CurrRegion->uiFlags & MSYS_MOUSE_IN_AREA &&
						!MSYS_CurrRegion->ButtonState)// & (MSYS_LEFT_BUTTON|MSYS_RIGHT_BUTTON)) )
				{
					MSYS_CurrRegion->FastHelpTimer -= (INT16)max( iTimeDifferential, 0 );

					if( MSYS_CurrRegion->FastHelpTimer < 0 )
					{
						MSYS_CurrRegion->FastHelpTimer = 0;
					}
				}
			}
		}
	}
}


void MSYS_AllowDisabledRegionFastHelp( MOUSE_REGION *region, BOOLEAN fAllow )
{
	if( fAllow )
	{
		region->uiFlags |= MSYS_ALLOW_DISABLED_FASTHELP;
	}
	else
	{
		region->uiFlags &= ~MSYS_ALLOW_DISABLED_FASTHELP;
	}
}
