
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

#include "Types.h"
#include "Debug.h"
#include "Input.h"
#include "MemMan.h"
#include "Line.h"
#include "Video.h"
// Include mouse system defs and macros
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


#define BASE_REGION_FLAGS (MSYS_REGION_ENABLED | MSYS_SET_CURSOR)


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
static BOOLEAN MSYS_UseMouseHandlerHook = FALSE;

BOOLEAN MSYS_Mouse_Grabbed=FALSE;
MOUSE_REGION *MSYS_GrabRegion = NULL;

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
								0, 0, -1, MSYS_NO_CALLBACK, NULL, NULL };

static BOOLEAN gfRefreshUpdate = FALSE;

//Kris:  December 3, 1997
//Special internal debugging utilities that will ensure that you don't attempt to delete
//an already deleted region.  It will also ensure that you don't create an identical region
//that already exists.
//TO REMOVE ALL DEBUG FUNCTIONALITY:  simply comment out MOUSESYSTEM_DEBUGGING definition
#ifdef JA2
  #ifdef _DEBUG
	#ifndef BOUNDS_CHECKER
	  #define MOUSESYSTEM_DEBUGGING
	#endif
  #endif
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
	MSYS_UseMouseHandlerHook = FALSE;

	MSYS_Mouse_Grabbed=FALSE;
	MSYS_GrabRegion = NULL;

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
	MSYS_SystemBaseRegion.FastHelpRect				= -1;

	MSYS_SystemBaseRegion.next								= NULL;
	MSYS_SystemBaseRegion.prev								= NULL;

	// Add the base region to the list
	MSYS_AddRegionToList(&MSYS_SystemBaseRegion);

#ifdef _MOUSE_SYSTEM_HOOK_
	MSYS_UseMouseHandlerHook = TRUE;
#endif

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
	MSYS_UseMouseHandlerHook = FALSE;
	MSYS_TrashRegList();
}


static void MSYS_UpdateMouseRegion(void);


//======================================================================================================
//	MSYS_SGP_Mouse_Handler_Hook
//
//	Hook to the SGP's mouse handler
//
void MSYS_SGP_Mouse_Handler_Hook(UINT16 Type,UINT16 Xcoord, UINT16 Ycoord, BOOLEAN LeftButton, BOOLEAN RightButton)
{
	// If the mouse system isn't initialized, get out o' here
	if(!MSYS_SystemInitialized)
			return;

	// If we're not using the handler stuff, ignore this call
	if(!MSYS_UseMouseHandlerHook)
			return;

	MSYS_Action=MSYS_NO_ACTION;
	switch(Type)
	{
		case LEFT_BUTTON_DOWN:
		case LEFT_BUTTON_UP:
		case RIGHT_BUTTON_DOWN:
		case RIGHT_BUTTON_UP:
			//MSYS_Action|=MSYS_DO_BUTTONS;
			if(Type == LEFT_BUTTON_DOWN)
				MSYS_Action |= MSYS_DO_LBUTTON_DWN;
			else if(Type == LEFT_BUTTON_UP)
			{
				MSYS_Action |= MSYS_DO_LBUTTON_UP;
				//Kris:
				//Used only if applicable.  This is used for that special button that is locked with the
				//mouse press -- just like windows.  When you release the button, the previous state
				//of the button is restored if you released the mouse outside of it's boundaries.  If
				//you release inside of the button, the action is selected -- but later in the code.
				//NOTE:  It has to be here, because the mouse can be released anywhere regardless of
				//regions, buttons, etc.
				#ifdef JA2
					ReleaseAnchorMode();
				#endif
			}
			else if(Type == RIGHT_BUTTON_DOWN)
				MSYS_Action |= MSYS_DO_RBUTTON_DWN;
			else if(Type == RIGHT_BUTTON_UP)
				MSYS_Action |= MSYS_DO_RBUTTON_UP;

			if(LeftButton)
				MSYS_CurrentButtons|=MSYS_LEFT_BUTTON;
			else
				MSYS_CurrentButtons&=(~MSYS_LEFT_BUTTON);

			if(RightButton)
				MSYS_CurrentButtons|=MSYS_RIGHT_BUTTON;
			else
				MSYS_CurrentButtons&=(~MSYS_RIGHT_BUTTON);

			if((Xcoord != MSYS_CurrentMX) || (Ycoord != MSYS_CurrentMY))
			{
				MSYS_Action|=MSYS_DO_MOVE;
				MSYS_CurrentMX = Xcoord;
				MSYS_CurrentMY = Ycoord;
			}

			MSYS_UpdateMouseRegion();
			break;

		// ATE: Checks here for mouse button repeats.....
		// Call mouse region with new reason
		case LEFT_BUTTON_REPEAT:
		case RIGHT_BUTTON_REPEAT:

			if(Type == LEFT_BUTTON_REPEAT)
				MSYS_Action |= MSYS_DO_LBUTTON_REPEAT;
			else if(Type == RIGHT_BUTTON_REPEAT)
				MSYS_Action |= MSYS_DO_RBUTTON_REPEAT;

			if((Xcoord != MSYS_CurrentMX) || (Ycoord != MSYS_CurrentMY))
			{
				MSYS_Action|=MSYS_DO_MOVE;
				MSYS_CurrentMX = Xcoord;
				MSYS_CurrentMY = Ycoord;
			}

			MSYS_UpdateMouseRegion();
			break;

		case MOUSE_POS:
			if((Xcoord != MSYS_CurrentMX) || (Ycoord != MSYS_CurrentMY) || gfRefreshUpdate )
			{
				MSYS_Action|=MSYS_DO_MOVE;
				MSYS_CurrentMX = Xcoord;
				MSYS_CurrentMY = Ycoord;

				gfRefreshUpdate = FALSE;

				MSYS_UpdateMouseRegion();
			}
			break;

		default:
		  DebugMsg(TOPIC_MOUSE_SYSTEM, DBG_LEVEL_0, "ERROR -- MSYS 2 SGP Mouse Hook got bad type");
			break;
	}
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

	// Did we delete a grabbed region?
	if(MSYS_Mouse_Grabbed)
	{
		if(MSYS_GrabRegion == region)
		{
			MSYS_Mouse_Grabbed = FALSE;
			MSYS_GrabRegion = NULL;
		}
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



//======================================================================================================
//	MSYS_UpdateMouseRegion
//
//	Searches the list for the highest priority region and updates it's info. It also dispatches
//	the callback functions
//
static void MSYS_UpdateMouseRegion(void)
{
	INT32 found;
	UINT32 ButtonReason;
  MOUSE_REGION *pTempRegion;
	BOOLEAN fFound=FALSE;
	found=FALSE;

	// Check previous region!
	if(MSYS_Mouse_Grabbed)
	{
		MSYS_CurrRegion = MSYS_GrabRegion;
		found = TRUE;
	}
	if(!found)
		MSYS_CurrRegion = MSYS_RegList;

	while( !found && MSYS_CurrRegion )
	{
		if( MSYS_CurrRegion->uiFlags & (MSYS_REGION_ENABLED | MSYS_ALLOW_DISABLED_FASTHELP) &&
		   (MSYS_CurrRegion->RegionTopLeftX <= MSYS_CurrentMX) &&		// Check boundaries
		   (MSYS_CurrRegion->RegionTopLeftY <= MSYS_CurrentMY) &&
		   (MSYS_CurrRegion->RegionBottomRightX >= MSYS_CurrentMX) &&
		   (MSYS_CurrRegion->RegionBottomRightY >= MSYS_CurrentMY))
		{
			// We got the right region. We don't need to check for priorities 'cause
			// the whole list is sorted the right way!
			found=TRUE;
		}
		else
			MSYS_CurrRegion = MSYS_CurrRegion->next;
	}

	if( MSYS_PrevRegion )
	{
		MSYS_PrevRegion->uiFlags &= (~MSYS_MOUSE_IN_AREA);

		if ( MSYS_PrevRegion != MSYS_CurrRegion )
		{
			//Remove the help text for the previous region if one is currently being displayed.
			if( MSYS_PrevRegion->FastHelpText )
			{
				//ExecuteMouseHelpEndCallBack( MSYS_PrevRegion );

				#ifdef _JA2_RENDER_DIRTY
					if( MSYS_PrevRegion->uiFlags & MSYS_GOT_BACKGROUND )
						FreeBackgroundRectPending( MSYS_PrevRegion->FastHelpRect );
				#endif
				MSYS_PrevRegion->uiFlags &= (~MSYS_GOT_BACKGROUND);
				MSYS_PrevRegion->uiFlags &= (~MSYS_FASTHELP_RESET);

				//if( region->uiFlags & MSYS_REGION_ENABLED )
				//	region->uiFlags |= BUTTON_DIRTY;
#ifndef JA2
							VideoRemoveToolTip();
#endif
			}

			MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;

			// Force a callbacks to happen on previous region to indicate that
			// the mouse has left the old region
			if ( MSYS_PrevRegion->uiFlags & MSYS_MOVE_CALLBACK && MSYS_PrevRegion->uiFlags & MSYS_REGION_ENABLED )
				(*(MSYS_PrevRegion->MovementCallback))(MSYS_PrevRegion,MSYS_CALLBACK_REASON_LOST_MOUSE);
		}
	}

	// If a region was found in the list, update it's data
	if(found)
	{
		if(MSYS_CurrRegion != MSYS_PrevRegion)
		{
			//Kris -- October 27, 1997
			//Implemented gain mouse region
			if ( MSYS_CurrRegion->uiFlags & MSYS_MOVE_CALLBACK )
			{
				if( MSYS_CurrRegion->FastHelpText && !( MSYS_CurrRegion->uiFlags & MSYS_FASTHELP_RESET ) )
				{
				  //ExecuteMouseHelpEndCallBack( MSYS_CurrRegion );
					MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;
					#ifdef _JA2_RENDER_DIRTY
						if( MSYS_CurrRegion->uiFlags & MSYS_GOT_BACKGROUND )
							FreeBackgroundRectPending( MSYS_CurrRegion->FastHelpRect );
					#endif
					MSYS_CurrRegion->uiFlags &= (~MSYS_GOT_BACKGROUND);
					MSYS_CurrRegion->uiFlags |= MSYS_FASTHELP_RESET;

#ifndef JA2
							VideoRemoveToolTip();
#endif

					//if( b->uiFlags & BUTTON_ENABLED )
					//	b->uiFlags |= BUTTON_DIRTY;
				}
				if( MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED )
				{
					(*(MSYS_CurrRegion->MovementCallback))(MSYS_CurrRegion,MSYS_CALLBACK_REASON_GAIN_MOUSE);
				}
			}

			// if the cursor is set and is not set to no cursor
      if( MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED &&
					MSYS_CurrRegion->uiFlags & MSYS_SET_CURSOR &&
					MSYS_CurrRegion->Cursor != MSYS_NO_CURSOR )
			{
				MSYS_SetCurrentCursor(MSYS_CurrRegion->Cursor);
			}
			else
			{
	     // Addition Oct 10/1997 Carter, patch for mouse cursor
			 // start at region and find another region encompassing
			 pTempRegion=MSYS_CurrRegion->next;
			 while((pTempRegion!=NULL)&&(!fFound))
			 {
        if((pTempRegion->uiFlags & MSYS_REGION_ENABLED) &&
		       (pTempRegion->RegionTopLeftX <= MSYS_CurrentMX) &&
		       (pTempRegion->RegionTopLeftY <= MSYS_CurrentMY) &&
		       (pTempRegion->RegionBottomRightX >= MSYS_CurrentMX) &&
		       (pTempRegion->RegionBottomRightY >= MSYS_CurrentMY)&&(pTempRegion->uiFlags & MSYS_SET_CURSOR))
				{
	  	   fFound=TRUE;
				 if( pTempRegion->Cursor != MSYS_NO_CURSOR )
				 {
					 MSYS_SetCurrentCursor(pTempRegion->Cursor);
				 }
				}
        pTempRegion=pTempRegion->next;
			 }
      }
		}

		// OK, if we do not have a button down, any button is game!
		if ( !gfClickedModeOn || ( gfClickedModeOn && gusClickedIDNumber == MSYS_CurrRegion->IDNumber ) )
		{
			MSYS_CurrRegion->uiFlags |= MSYS_MOUSE_IN_AREA;

			MSYS_CurrRegion->MouseXPos = MSYS_CurrentMX;
			MSYS_CurrRegion->MouseYPos = MSYS_CurrentMY;
			MSYS_CurrRegion->RelativeXPos = MSYS_CurrentMX - MSYS_CurrRegion->RegionTopLeftX;
			MSYS_CurrRegion->RelativeYPos = MSYS_CurrentMY - MSYS_CurrRegion->RegionTopLeftY;

			MSYS_CurrRegion->ButtonState = MSYS_CurrentButtons;

			if( MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED &&
					MSYS_CurrRegion->uiFlags & MSYS_MOVE_CALLBACK && MSYS_Action & MSYS_DO_MOVE )
			{
				(*(MSYS_CurrRegion->MovementCallback))(MSYS_CurrRegion,MSYS_CALLBACK_REASON_MOVE);
			}

			//ExecuteMouseHelpEndCallBack( MSYS_CurrRegion );
			//MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;

			MSYS_Action &= (~MSYS_DO_MOVE);

			if((MSYS_CurrRegion->uiFlags & MSYS_BUTTON_CALLBACK) && (MSYS_Action & MSYS_DO_BUTTONS))
			{
				if( MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED )
				{
					ButtonReason=MSYS_CALLBACK_REASON_NONE;
					if(MSYS_Action & MSYS_DO_LBUTTON_DWN)
					{
						ButtonReason|=MSYS_CALLBACK_REASON_LBUTTON_DWN;
						gfClickedModeOn = TRUE;
						// Set global ID
						gusClickedIDNumber = MSYS_CurrRegion->IDNumber;
					}

					if(MSYS_Action & MSYS_DO_LBUTTON_UP )
					{
						ButtonReason|=MSYS_CALLBACK_REASON_LBUTTON_UP;
						gfClickedModeOn = FALSE;
					}

					if(MSYS_Action & MSYS_DO_RBUTTON_DWN)
					{
						ButtonReason|=MSYS_CALLBACK_REASON_RBUTTON_DWN;
						gfClickedModeOn = TRUE;
						// Set global ID
						gusClickedIDNumber = MSYS_CurrRegion->IDNumber;
					}

					if(MSYS_Action & MSYS_DO_RBUTTON_UP  )
					{
						ButtonReason|=MSYS_CALLBACK_REASON_RBUTTON_UP;
						gfClickedModeOn = FALSE;
					}

					// ATE: Added repeat resons....
					if(MSYS_Action & MSYS_DO_LBUTTON_REPEAT  )
					{
						ButtonReason|=MSYS_CALLBACK_REASON_LBUTTON_REPEAT;
					}

					if(MSYS_Action & MSYS_DO_RBUTTON_REPEAT  )
					{
						ButtonReason|=MSYS_CALLBACK_REASON_RBUTTON_REPEAT;
					}

					if( ButtonReason != MSYS_CALLBACK_REASON_NONE )
					{
						if( MSYS_CurrRegion->uiFlags & MSYS_FASTHELP )
						{
							// Button was clicked so remove any FastHelp text
							MSYS_CurrRegion->uiFlags &= (~MSYS_FASTHELP);
							#ifdef _JA2_RENDER_DIRTY
								if( MSYS_CurrRegion->uiFlags & MSYS_GOT_BACKGROUND )
									FreeBackgroundRectPending( MSYS_CurrRegion->FastHelpRect );
							#endif
							MSYS_CurrRegion->uiFlags &= (~MSYS_GOT_BACKGROUND);

							//ExecuteMouseHelpEndCallBack( MSYS_CurrRegion );
							MSYS_CurrRegion->FastHelpTimer = gsFastHelpDelay;
							MSYS_CurrRegion->uiFlags &= (~MSYS_FASTHELP_RESET);

							//if( b->uiFlags & BUTTON_ENABLED )
							//	b->uiFlags |= BUTTON_DIRTY;
#ifndef JA2
							VideoRemoveToolTip();
#endif
						}

						//Kris: Nov 31, 1999 -- Added support for double click events.
						//This is where double clicks are checked and passed down.
						if( ButtonReason == MSYS_CALLBACK_REASON_LBUTTON_DWN )
						{
							UINT32 uiCurrTime = GetClock();
							if( gpRegionLastLButtonDown == MSYS_CurrRegion &&
									gpRegionLastLButtonUp == MSYS_CurrRegion &&
									uiCurrTime <= guiRegionLastLButtonDownTime + MSYS_DOUBLECLICK_DELAY )
							{ //Sequential left click on same button within the maximum time allowed for a double click
								//Double click check succeeded, set flag and reset double click globals.
								ButtonReason |= MSYS_CALLBACK_REASON_LBUTTON_DOUBLECLICK;
								gpRegionLastLButtonDown = NULL;
								gpRegionLastLButtonUp = NULL;
								guiRegionLastLButtonDownTime = 0;
							}
							else
							{ //First click, record time and region pointer (to check if 2nd click detected later)
								gpRegionLastLButtonDown = MSYS_CurrRegion;
								guiRegionLastLButtonDownTime = GetClock();
							}
						}
						else if( ButtonReason == MSYS_CALLBACK_REASON_LBUTTON_UP )
						{
							UINT32 uiCurrTime = GetClock();
							if( gpRegionLastLButtonDown == MSYS_CurrRegion &&
									uiCurrTime <= guiRegionLastLButtonDownTime + MSYS_DOUBLECLICK_DELAY )
							{ //Double click is Left down, then left up, then left down.  We have just detected the left up here (step 2).
								gpRegionLastLButtonUp = MSYS_CurrRegion;
							}
							else
							{ //User released mouse outside of current button, so kill any chance of a double click happening.
								gpRegionLastLButtonDown = NULL;
								gpRegionLastLButtonUp = NULL;
								guiRegionLastLButtonDownTime = 0;
							}
						}

						(*(MSYS_CurrRegion->ButtonCallback))(MSYS_CurrRegion,ButtonReason);
					}
				}
			}

			MSYS_Action &= (~MSYS_DO_BUTTONS);
		}
		else if( MSYS_CurrRegion->uiFlags & MSYS_REGION_ENABLED )
		{
			// OK here, if we have release a button, UNSET LOCK wherever you are....
			// Just don't give this button the message....
			if(MSYS_Action & MSYS_DO_RBUTTON_UP  )
			{
				gfClickedModeOn = FALSE;
			}
			if(MSYS_Action & MSYS_DO_LBUTTON_UP  )
			{
				gfClickedModeOn = FALSE;
			}

			// OK, you still want move messages however....
			MSYS_CurrRegion->uiFlags |= MSYS_MOUSE_IN_AREA;
			MSYS_CurrRegion->MouseXPos = MSYS_CurrentMX;
			MSYS_CurrRegion->MouseYPos = MSYS_CurrentMY;
			MSYS_CurrRegion->RelativeXPos = MSYS_CurrentMX - MSYS_CurrRegion->RegionTopLeftX;
			MSYS_CurrRegion->RelativeYPos = MSYS_CurrentMY - MSYS_CurrRegion->RegionTopLeftY;

			if((MSYS_CurrRegion->uiFlags & MSYS_MOVE_CALLBACK) && (MSYS_Action & MSYS_DO_MOVE))
			{
				(*(MSYS_CurrRegion->MovementCallback))(MSYS_CurrRegion,MSYS_CALLBACK_REASON_MOVE);
			}

			MSYS_Action &= (~MSYS_DO_MOVE);
		}
		MSYS_PrevRegion = MSYS_CurrRegion;
	}
	else
		MSYS_PrevRegion = NULL;
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
	if(movecallback != MSYS_NO_CALLBACK)
		region->uiFlags |= MSYS_MOVE_CALLBACK;

	region->ButtonCallback = buttoncallback;
	if(buttoncallback != MSYS_NO_CALLBACK)
		region->uiFlags |= MSYS_BUTTON_CALLBACK;

	region->Cursor = crsr;
	if(crsr != MSYS_NO_CURSOR)
		region->uiFlags |= MSYS_SET_CURSOR;

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
	region->HelpDoneCallback = NULL;

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
	region->uiFlags &= (~MSYS_SET_CURSOR);
	region->Cursor = crsr;
	if(crsr != MSYS_NO_CURSOR)
	{
		region->uiFlags |= MSYS_SET_CURSOR;

		// If we are not in the region, donot update!
		if ( !( region->uiFlags & MSYS_MOUSE_IN_AREA ) )
		{
			return;
		}

		// Update cursor
		MSYS_SetCurrentCursor( crsr );
	}
}



//=================================================================================================
//	MSYS_AddRegion
//
//	Adds a defined mouse region to the system list. Once inserted, it enables the region then
//	calls the callback functions, if any, for initialization.
//
INT32 MSYS_AddRegion(MOUSE_REGION *region)
{
	return(1);
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
#ifndef JA2
		if(region->uiFlags & MSYS_FASTHELP)
			VideoRemoveToolTip();
#endif
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
		UINT8 str[80];
		#ifdef MOUSESYSTEM_DEBUGGING
		if( gfIgnoreShutdownAssertions )
		#endif
			return;
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
		UINT8 str[80];
		#ifdef MOUSESYSTEM_DEBUGGING
		if( gfIgnoreShutdownAssertions )
		#endif
			return 0;
		sprintf( str, "Attempting MSYS_GetRegionUserData() with out of range index %d", index );
		AssertMsg( 0, str );
	}
	return(region->UserData[index]);
}


/* ==================================================================================
   MSYS_MoveMouseRegionTo( MOUSE_REGION *region, INT16 sX, INT16 sY)

	 Moves a Mouse region to X Y on the screen

*/

void MSYS_MoveMouseRegionTo( MOUSE_REGION *region, INT16 sX, INT16 sY)
{

  INT16 sWidth;
	INT16 sHeight;


	sWidth =  region ->RegionBottomRightX - region ->RegionTopLeftX;
	sHeight = region ->RegionBottomRightY - region ->RegionTopLeftY;


	// move top left
	region -> RegionTopLeftX = sX;
	region -> RegionTopLeftY = sY;

	// now move bottom right based on topleft + width or height
  region -> RegionBottomRightX = sX + sWidth;
	region -> RegionBottomRightY = sY + sHeight;
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


static INT16 GetNumberOfLinesInHeight(const wchar_t* pStringA)
{
	STR16 pToken;
	INT16 sCounter = 0;
	CHAR16 pString[ 512 ];
	wchar_t* Context = NULL;

	wcscpy( pString, pStringA );

	// tokenize
	pToken = wcstok(pString, L"\n", &Context);

	while( pToken != NULL )
  {
		 pToken = wcstok(NULL, L"\n", &Context);
		 sCounter++;
	}

	return( sCounter );
}



#ifdef _JA2_RENDER_DIRTY

static INT16 GetWidthOfString(const wchar_t* pStringA);
static void DisplayHelpTokenizedString(const wchar_t* pStringA, INT16 sX, INT16 sY);


//=============================================================================
//	DisplayFastHelp
//
//
static void DisplayFastHelp(MOUSE_REGION* region)
{
	UINT16 usFillColor;
	INT32 iX,iY,iW,iH;

	if ( region->uiFlags & MSYS_FASTHELP )
	{
		usFillColor = Get16BPPColor(FROMRGB(250, 240, 188));

		iW = (INT32)GetWidthOfString( region->FastHelpText ) + 10;
		iH = (INT32)( GetNumberOfLinesInHeight( region->FastHelpText ) * (GetFontHeight(FONT10ARIAL)+1) + 8 );

		iX = (INT32)region->RegionTopLeftX + 10;

		if (iX < 0)
			iX = 0;

		if ( (iX + iW) >= SCREEN_WIDTH )
			iX = (SCREEN_WIDTH - iW - 4);

		iY = (INT32)region->RegionTopLeftY - (iH * 3 / 4);
		if (iY < 0)
			iY = 0;

		if ( (iY + iH) >= SCREEN_HEIGHT )
			iY = (SCREEN_HEIGHT - iH - 15);

		if ( !(region->uiFlags & MSYS_GOT_BACKGROUND) )
		{
			region->FastHelpRect = RegisterBackgroundRect(BGND_FLAG_PERMANENT | BGND_FLAG_SAVERECT, NULL, (INT16)iX, (INT16)iY,
						(INT16)(iX + iW), (INT16)(iY + iH) );
			region->uiFlags |= MSYS_GOT_BACKGROUND;
			region->uiFlags |= MSYS_HAS_BACKRECT;
		}
		else
		{
			UINT8 *pDestBuf;
			UINT32 uiDestPitchBYTES;
			pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
			SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );
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


static INT16 GetWidthOfString(const wchar_t* pStringA)
{
	CHAR16 pString[ 512 ];
	STR16 pToken;
	INT16 sWidth = 0;
	wchar_t* Context = NULL;

	wcscpy( pString, pStringA );

	// tokenize
	pToken = wcstok(pString, L"\n", &Context);

	while( pToken != NULL )
  {
		if( sWidth < StringPixLength( pToken, FONT10ARIAL ) )
		{
			sWidth = StringPixLength( pToken, FONT10ARIAL );
		}

		pToken = wcstok(NULL, L"\n", &Context);
	}

	return( sWidth );

}


static void DisplayHelpTokenizedString(const wchar_t* pStringA, INT16 sX, INT16 sY)
{
	STR16 pToken;
	INT32 iCounter = 0, i;
	UINT32 uiCursorXPos;
	CHAR16 pString[ 512 ];
	INT32 iLength;
	wchar_t* Context = NULL;

	wcscpy( pString, pStringA );

	// tokenize
	pToken = wcstok(pString, L"\n", &Context);

	while( pToken != NULL )
  {
		iLength = (INT32)wcslen( pToken );
		for( i = 0; i < iLength; i++ )
		{
			uiCursorXPos = StringPixLengthArgFastHelp( FONT10ARIAL, FONT10ARIALBOLD, i, pToken );
			if( pToken[ i ] == '|' )
			{
				i++;
				SetFont( FONT10ARIALBOLD );
				SetFontForeground( 146 );
			}
			else
			{
				SetFont( FONT10ARIAL );
				SetFontForeground( FONT_BEIGE );
			}
			mprintf( sX + uiCursorXPos, sY + iCounter * (GetFontHeight(FONT10ARIAL)+1), L"%C", pToken[ i ] );
		}
		pToken = wcstok( NULL, L"\n", &Context);
		iCounter++;
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
#else

// **********Wiz8 Versions**************************************************************************

INT16 GetWidthOfString( STR16 pStringA )
{
	CHAR16 pString[ 512 ];
	STR16 pToken;
	INT16 sWidth = 0;
	wcscpy( pString, pStringA );

	// tokenize
	pToken = wcstok( pString, L"\n" );

	while( pToken != NULL )
  {
		if( sWidth < StringPixLength( pToken, ghTinyMonoFont ) )
		{
			sWidth = StringPixLength( pToken, ghTinyMonoFont );
		}

		pToken = wcstok( NULL, L"\n" );
	}

	return( sWidth );

}

void DisplayFastHelp( MOUSE_REGION *region )
{
	INT32 iX,iY,iW,iH;

	if ( region->uiFlags & MSYS_FASTHELP )
	{
		VideoToolTip(region->FastHelpText);

		iW = VideoGetToolTipWidth();
		iH = VideoGetToolTipHeight();

		iX = (INT32)region->RegionTopLeftX + 10;

		if (iX < 0)
			iX = 0;

		if ( (iX + iW) >= SCREEN_WIDTH )
			iX = (SCREEN_WIDTH - iW - 4);

		iY = (INT32)region->RegionTopLeftY - (iH * 3 / 4);
		if (iY < 0)
			iY = 0;

		if ( (iY + iH) >= SCREEN_HEIGHT )
			iY = (SCREEN_HEIGHT - iH - 15);

		VideoPositionToolTip(iX, iY);
	}
}

void DisplayHelpTokenizedString( STR16 pStringA, INT16 sX, INT16 sY )
{
	STR16 pToken;
	INT32 iCounter = 0, i;
	UINT32 uiCursorXPos;
	CHAR16 pString[ 512 ];
	INT32 iLength;

	wcscpy( pString, pStringA );

	// tokenize
	pToken = wcstok( pString, L"\n" );

	while( pToken != NULL )
  {
		iLength = (INT32)wcslen( pToken );
		for( i = 0; i < iLength; i++ )
		{
			uiCursorXPos = StringPixLengthArgFastHelp( ghTinyMonoFont, ghTinyMonoFont, i, pToken );
			if( pToken[ i ] == '|' )
			{
				i++;
				SetFont(ghTinyMonoFont);
				SetFontForeground( 2 );
			}
			else
			{
				SetFont( ghTinyMonoFont );
				SetFontForeground( 2 );
			}
			mprintf( sX + uiCursorXPos, sY + iCounter * (GetFontHeight(ghTinyMonoFont)+1), L"%C", pToken[ i ] );
		}
		pToken = wcstok( NULL, L"\n" );
		iCounter++;
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

	if( MSYS_CurrRegion && MSYS_CurrRegion->FastHelpText)
	{
		if( !MSYS_CurrRegion->FastHelpTimer )
		{
			if( MSYS_CurrRegion->uiFlags & ( MSYS_ALLOW_DISABLED_FASTHELP | MSYS_REGION_ENABLED ) )
			{
				if( MSYS_CurrRegion->uiFlags & MSYS_MOUSE_IN_AREA )
				{
					MSYS_CurrRegion->uiFlags |= MSYS_FASTHELP;
					DisplayFastHelp( MSYS_CurrRegion );
				}
				else
				{
					MSYS_CurrRegion->uiFlags &= ( ~( MSYS_FASTHELP | MSYS_FASTHELP_RESET ) );
					VideoRemoveToolTip();
				}
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
#endif


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

// new stuff to allow mouse callbacks when help text finishes displaying

void SetRegionHelpEndCallback( MOUSE_REGION *region, MOUSE_HELPTEXT_DONE_CALLBACK CallbackFxn )
{
	// make sure region is non null
	if( region == NULL )
	{
		return;
	}

	// now set the region help text
	region-> HelpDoneCallback = CallbackFxn;
}


// help text is done, now execute callback, if there is one
static void ExecuteMouseHelpEndCallBack(MOUSE_REGION* region)
{
	if( region == NULL )
	{
		return;
	}

	if( region->FastHelpTimer )
	{
		return;
	}
	// check if callback is non null
	if( region->HelpDoneCallback == NULL )
	{
		return;
	}

	// we have a callback, excecute
	// ATE: Disable these!
	//( *( region->HelpDoneCallback ) )( );
}
