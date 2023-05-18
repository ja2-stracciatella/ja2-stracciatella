//FUNCTIONALITY:
//This popup menu functions IDENTICALLY to Windows95's methodology.  You can use the arrow keys, or the
//mouse in two methods.  The first method is where you release the mouse button before
//moving the cursor inside the popup menu, which makes it persistant (it won't go away until you click again),
//or non-persistant, where you release the button inside the menu after your initial click (non-persistant).
//IMPORTANT NOTE:
//Linda Currie, the main user of the editor dislikes the non-persistant functionality, however, it is still
//supported.  Just remove the commented line of code (search for UNCOMMENT), and it's fixed -- it is
//currently disabled.

#include "Button_System.h"
#include "HImage.h"
#include "Input.h"
#include "Local.h"
#include "SysUtil.h"
#include "Font.h"
#include "Font_Control.h"
#include "Line.h"
#include "VObject.h"
#include "WorldDat.h"
#include "SelectWin.h"
#include "PopupMenu.h"
#include "EditorDefines.h"
#include "Render_Dirty.h"
#include "MouseSystem.h"
#include "Cursors.h"
#include "Overhead_Types.h"
#include "EditorMercs.h"
#include "Scheduling.h"
#include "English.h"
#include "Item_Statistics.h"
#include "Video.h"
#include "VSurface.h"
#include "WorldDef.h"
#include "UILayout.h"

#include <string_theory/string>


#define POPUP_ACTIVETYPE_NOT_YET_DETERMINED 0
#define POPUP_ACTIVETYPE_PERSISTANT         1
#define POPUP_ACTIVETYPE_NONPERSISTANT      2

#define MAX_COLUMNS                         8

/* This structure contains all the required information for rendering the popup
 * menu */
struct CurrentPopupMenuInformation
{
	PopupMenuID ubPopupMenuID;
	UINT8       ubSelectedIndex; // current popup menu index hilited.
	UINT8       ubNumEntries;
	UINT8       ubColumns;
	UINT8       ubMaxEntriesPerColumn;
	UINT8       ubColumnWidth[MAX_COLUMNS];
	UINT8       ubActiveType;
	SGPFont     usFont;
	BOOLEAN     fActive;
	BOOLEAN     fUseKeyboardInfoUntilMouseMoves;
	UINT16      usLeft; // popup region coords.
	UINT16      usTop;
	UINT16      usRight;
	UINT16      usBottom;
	UINT16      usLastMouseX;
	UINT16      usLastMouseY;
};

static CurrentPopupMenuInformation gPopup;

static MOUSE_REGION popupRegion;

static UINT16  gusEntryHeight;
static BOOLEAN fWaitingForLButtonRelease = FALSE;

//Finds the string for any popup menu in JA2 -- the strings are stored
//in different ways in each instance.
static const ST::string GetPopupMenuString(UINT8 ubIndex)
{
	switch( gPopup.ubPopupMenuID )
	{
		case CHANGETSET_POPUP: //tile sets
			return gTilesets[ ubIndex ].zName;
		case OWNERSHIPGROUP_POPUP: //civilian groups
		case CHANGECIVGROUP_POPUP: //civilian groups
			return gszCivGroupNames[ ubIndex ];
		case SCHEDULEACTION_POPUP: //setting scheduling actions
			return gszScheduleActions[ ubIndex ];
		case ACTIONITEM_POPUP:
			return gszActionItemDesc[ ubIndex ];
		default:
			return {};
	}
}


static void RenderPopupMenu(void);


/*
InitPopUpMenu
---------------
Creates the popup menu, then takes control of the i/o
effectively turning the popup into a modal system (forcing the
user to make a selection or cancel before doing anything else.
There are two modes of operation as soon as the user presses the mouse
on a popup menu activation button.
	1)  The user releases the mouse before moving the cursor onto
			the popup region.  The popup menu will end as soon as the
			user presses the mouse button.  If the cursor is out of the
			popup region, then it returns 0.
	2)  The user holds the mouse while moving the cursor onto the
			popup region.  The popup menu will end as soon as the user
			releases the mouse button.  If the cursor is out of the popup
			region, then it returns 0.


Returns 0 if nothing in menu was selected, else
returns the menu entry number starting from 1.
*/
void InitPopupMenu(GUIButtonRef const button, PopupMenuID const ubPopupMenuID, UINT8 const ubDirection)
{
	UINT16 usX, usY; // HACK000E
	UINT16 usMenuHeight;
	UINT16 usMenuWidth = 0;
	UINT16 usCurrStrWidth;
	UINT8 ubColumn, ubEntry;
	UINT8 ubCounter;
	//calculate the location of the menu based on the button position.
	//This also calculates the menu's direction based on position.

	gPopup.usFont = SMALLFONT1;
	gusEntryHeight = GetFontHeight( gPopup.usFont );

	switch( ubDirection )
	{
		case DIR_UPRIGHT:
			usX = button->X();
			usY = button->Y();
			break;
		case DIR_UPLEFT:
			usX = button->BottomRightX();
			usY = button->Y();
			break;
		case DIR_DOWNRIGHT:
			usX = button->X();
			usY = button->BottomRightY();
			break;
		case DIR_DOWNLEFT:
			usX = button->BottomRightX();
			usY = button->BottomRightY();
			break;

		default: abort(); // HACK000E
	}

	//Decipher the PopupMenuID
	switch( ubPopupMenuID )
	{
		case CHANGETSET_POPUP:	//change tileset
			gPopup.ubNumEntries = gubNumTilesets;
			break;
		case OWNERSHIPGROUP_POPUP:
		case CHANGECIVGROUP_POPUP:
			gPopup.ubNumEntries = NUM_CIV_GROUPS;
			break;
		case SCHEDULEACTION_POPUP:
			gPopup.ubNumEntries = NUM_SCHEDULE_ACTIONS;
			break;
		case ACTIONITEM_POPUP:
			gPopup.ubNumEntries = NUM_ACTIONITEMS;
			break;
		default:
			return;
	}

	gPopup.usFont = SMALLFONT1;
	gusEntryHeight = GetFontHeight( gPopup.usFont );

	gPopup.ubPopupMenuID = ubPopupMenuID;
	gPopup.ubSelectedIndex = 0;
	gPopup.ubActiveType = POPUP_ACTIVETYPE_NOT_YET_DETERMINED;
	gPopup.fActive = TRUE;
	fWaitingForLButtonRelease = FALSE;
	gPopup.fUseKeyboardInfoUntilMouseMoves = FALSE;
	//Initialize the last mouse position to be out of bounds.
	gPopup.usLastMouseX = 1000;
	gPopup.usLastMouseY = 1000;
	//clear the column widths.
	for( ubColumn = 0; ubColumn < MAX_COLUMNS; ubColumn++ )
		gPopup.ubColumnWidth[ ubColumn ] = 0;

	//1) Calc total entry height of the popup region.
	gPopup.ubColumns = 1;
	gPopup.ubMaxEntriesPerColumn = gPopup.ubNumEntries;
	usMenuHeight = gPopup.ubNumEntries * gusEntryHeight + 3;
	while ((usMenuHeight                 >= usY && (ubDirection == DIR_UPLEFT   || ubDirection == DIR_UPRIGHT)) ||
		(SCREEN_HEIGHT - usMenuHeight >= usY && (ubDirection == DIR_DOWNLEFT || ubDirection == DIR_DOWNRIGHT)))
	{ //menu has too many entries.  Increase the number of columns until the height is
		//less than the max height.
		gPopup.ubMaxEntriesPerColumn = (gPopup.ubNumEntries+gPopup.ubColumns)/(gPopup.ubColumns+1);
		usMenuHeight = gPopup.ubMaxEntriesPerColumn * gusEntryHeight + 3;
		gPopup.ubColumns++;
	}
	//now we have the number of columns as well as the max number of entries per column, and
	//the total menu height.

	//We now calculate the total width of the menu as well as the max width of each column.
	ubCounter = 0;
	usMenuWidth = 0;
	for( ubColumn = 0; ubColumn < gPopup.ubColumns; ubColumn++ )
	{
		for( ubEntry = 0; ubEntry < gPopup.ubMaxEntriesPerColumn; ubEntry++ )
		{
			if( ubCounter >= gPopup.ubNumEntries )
				break; //done (don't want to process undefined entries...)
			usCurrStrWidth = 16 + StringPixLength( GetPopupMenuString( ubCounter ) , gPopup.usFont );
			if( usCurrStrWidth > gPopup.ubColumnWidth[ ubColumn ] )
			{
				gPopup.ubColumnWidth[ ubColumn ] = (UINT8)usCurrStrWidth;
			}
			ubCounter++;
		}
		usMenuWidth += gPopup.ubColumnWidth[ ubColumn ];
	}

	//Calculate popup menu boundaries based on direction from argument point.
	switch( ubDirection )
	{
		case DIR_UPRIGHT:
			gPopup.usLeft = usX;
			gPopup.usTop = usY - usMenuHeight - 1;
			gPopup.usRight = usX + usMenuWidth;
			gPopup.usBottom = usY - 1;
			break;
		case DIR_UPLEFT:
			gPopup.usLeft = usX - usMenuWidth;
			gPopup.usTop = usY - usMenuHeight - 1;
			gPopup.usRight = usX;
			gPopup.usBottom = usY - 1;
			break;
		case DIR_DOWNRIGHT:
			gPopup.usLeft = usX;
			gPopup.usTop = usY + 1;
			gPopup.usRight = usX + usMenuWidth;
			gPopup.usBottom = usY + usMenuHeight + 1;
			break;
		case DIR_DOWNLEFT:
			gPopup.usLeft = usX - usMenuWidth;
			gPopup.usTop = usY + 1;
			gPopup.usRight = usX;
			gPopup.usBottom = usY + usMenuHeight + 1;
			break;
	}
	MSYS_DefineRegion(&popupRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	RenderPopupMenu();
}


static void RenderPopupMenu(void)
{
	CurrentPopupMenuInformation const& p = gPopup;

	// Draw the menu
	ColorFillVideoSurfaceArea(FRAME_BUFFER, p.usLeft, p.usTop, p.usRight, p.usBottom, Get16BPPColor(FROMRGB(128, 128, 128)));

	{ SGPVSurface::Lock l(FRAME_BUFFER);
		UINT16* const pDestBuf = l.Buffer<UINT16>();
		UINT32  const pitch    = l.Pitch();
		SetClippingRegionAndImageWidth(pitch, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		UINT16 const line_colour = Get16BPPColor(FROMRGB(64, 64, 64));
		RectangleDraw(TRUE, p.usLeft, p.usTop, p.usRight - 1, p.usBottom - 1, line_colour, pDestBuf);
		// Draw a vertical line between each column
		UINT16 x = p.usLeft;
		for (UINT8 column = 1; column < p.ubColumns; ++column)
		{
			x += p.ubColumnWidth[column - 1];
			LineDraw(TRUE, x, p.usTop, x, p.usBottom, line_colour, pDestBuf);
		}
	}

	// Set up the text attributes.
	SGPFont const font = p.usFont;
	SetFont(font);
	SetFontBackground(FONT_MCOLOR_BLACK);

	UINT8  const n_rows    = p.ubMaxEntriesPerColumn;
	UINT8  const n_entries = p.ubNumEntries;
	UINT8  const selected  = p.ubSelectedIndex - 1;
	UINT8        entry     = 0;
	UINT16       dx        = p.usLeft;
	UINT16 const dy        = p.usTop + 1;
	UINT16 const h         = gusEntryHeight;
	for (UINT8 column = 0;; ++column)
	{
		UINT8 const w = p.ubColumnWidth[column];
		for (UINT8 row = 0; row != n_rows; ++entry, ++row)
		{
			if (entry >= n_entries) return; // done

			SetFontForeground(entry == selected ? FONT_MCOLOR_LTBLUE : FONT_MCOLOR_WHITE);

			ST::string str = GetPopupMenuString(entry);
			UINT16         const str_w = StringPixLength(str, font);
			// Horizontally center the string inside the popup menu
			UINT16         const x     = dx + (w - str_w) / 2;
			UINT16         const y     = dy + row * h;
			MPrint(x, y, str);
		}
		dx += w;
	}
}


//This private function of PopupMenuHandle determines which menu entry
//is highlighted based on the mouse cursor position.  Returns 0 if the
//mouse is out of the menu region.
static UINT8 GetPopupIndexFromMousePosition(void)
{
	UINT8 ubNumEntriesDown;
	UINT16 usRelX;
	UINT8 ubCount;
	if( gusMouseXPos >= gPopup.usLeft
		&& gusMouseXPos <= gPopup.usRight
		&& gusMouseYPos > gPopup.usTop						//one pixel gap on top ignored
		&& gusMouseYPos < gPopup.usBottom - 2 )		//two pixel gap on bottom ignored
	{
		//subtract the top y coord of the popup region from the mouse's yPos as well
		//as an extra pixel at the top of the region which is ignored in menu selection,
		//divide this number by the height of a menu entry, then add one.  This will
		//return the menu index from 1 (at the top) to n (at the bottom).
		ubNumEntriesDown = (gusMouseYPos - gPopup.usTop - 1) / gusEntryHeight + 1;
		usRelX = gusMouseXPos - gPopup.usLeft;
		ubCount=0;
		while( usRelX > gPopup.ubColumnWidth[ ubCount ] )
		{
			usRelX -= gPopup.ubColumnWidth[ ubCount ];
			ubCount++;
			ubNumEntriesDown += gPopup.ubMaxEntriesPerColumn;
		}
		if( ubNumEntriesDown >= gPopup.ubNumEntries )
			ubNumEntriesDown = gPopup.ubNumEntries;
		return ubNumEntriesDown;
	}
	return 0; //mouse not in valid region.
}


static void ProcessPopupMenuSelection(void);


static void PopupMenuHandle(void)
{
	InputAtom InputEvent;

	if( gPopup.ubActiveType == POPUP_ACTIVETYPE_NOT_YET_DETERMINED )
	{
		//Attempt to determine if the menu will be persistant or not.
		//Determination is made when the mouse's left button is released or if
		//the mouse cursor enters the menu region.
		if( gusMouseXPos >= gPopup.usLeft
			&& gusMouseXPos <= gPopup.usRight
			&& gusMouseYPos > gPopup.usTop						//one pixel gap on top ignored
			&& gusMouseYPos < gPopup.usBottom - 1 )		//two pixel gap on bottom ignored
		{
			//mouse cursor has just entered the menu region -- nonpersistant.

			//KRIS:  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//Disabled this because Linda doesn't like it... (I like it though, and it works)
			//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//UNCOMMENT IF NONPERSISTANT IS ALLOWED
			//gPopup.ubActiveType = POPUP_ACTIVETYPE_NONPERSISTANT;

			return;
		}
		else if( !IsMouseButtonDown(MOUSE_BUTTON_LEFT) )
		{	//left button has been released before entering region -- persistant
			gPopup.ubActiveType = POPUP_ACTIVETYPE_PERSISTANT;
			return;
		}
	}
	if( !gPopup.fUseKeyboardInfoUntilMouseMoves )
	{
		//check menu entry based on mouse position
		gPopup.ubSelectedIndex = GetPopupIndexFromMousePosition();
	}
	else if( gusMouseXPos != gPopup.usLastMouseX || gusMouseYPos != gPopup.usLastMouseY )
	{
		//The keyboard determined the last entry, but the mouse has moved,
		//so use the mouse to determine the new entry.
		gPopup.fUseKeyboardInfoUntilMouseMoves = FALSE;
		gPopup.ubSelectedIndex = GetPopupIndexFromMousePosition();
	}
	//Check terminating conditions for persistant states.
	if( IsMouseButtonDown(MOUSE_BUTTON_LEFT) && gPopup.ubActiveType == POPUP_ACTIVETYPE_PERSISTANT )
		fWaitingForLButtonRelease = TRUE;
	if( (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && gPopup.ubActiveType == POPUP_ACTIVETYPE_PERSISTANT)
		|| (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && gPopup.ubActiveType == POPUP_ACTIVETYPE_NONPERSISTANT) )
	{
		//Selection conditions via mouse have been met whether the mouse is in the
		//menu region or not.
		gPopup.ubSelectedIndex = GetPopupIndexFromMousePosition();
		if( gPopup.ubSelectedIndex )
		{
			ProcessPopupMenuSelection();
		}
		gPopup.fActive = FALSE;
		MSYS_RemoveRegion( &popupRegion );
		gfRenderWorld = TRUE;
		gfRenderTaskbar = TRUE;
		return;
	}
	//Use keyboard input as well.
	while( DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS) )
	{
		switch(InputEvent.usEvent)
		{
			case KEY_DOWN:
				switch( InputEvent.usParam )
				{
					case SDLK_DOWN:
						gPopup.fUseKeyboardInfoUntilMouseMoves = TRUE;
						gPopup.usLastMouseX = gusMouseXPos;
						gPopup.usLastMouseY = gusMouseYPos;
						gPopup.ubSelectedIndex++;
						if( gPopup.ubSelectedIndex > gPopup.ubNumEntries )
						{
							gPopup.ubSelectedIndex = 1;
						}
						break;

					case SDLK_UP:
						gPopup.fUseKeyboardInfoUntilMouseMoves = TRUE;
						gPopup.usLastMouseX = gusMouseXPos;
						gPopup.usLastMouseY = gusMouseYPos;
						if( gPopup.ubSelectedIndex < 2 )
						{
							gPopup.ubSelectedIndex = gPopup.ubNumEntries;
						}
						else
						{
							gPopup.ubSelectedIndex--;
						}
						break;

					case SDLK_ESCAPE:
						gPopup.fActive = FALSE;
						MSYS_RemoveRegion( &popupRegion );
						gfRenderWorld = TRUE;
						gfRenderTaskbar = TRUE;
						break;

					case SDLK_RETURN:
						ProcessPopupMenuSelection();
						gPopup.fActive = FALSE;
						MSYS_RemoveRegion( &popupRegion );
						gfRenderWorld = TRUE;
						gfRenderTaskbar = TRUE;
						break;
				}
				break;
		}
	}
}


static void ProcessPopupMenuSelection(void)
{
	switch( gPopup.ubPopupMenuID )
	{
		case CHANGETSET_POPUP:
			//change the tileset here.
			ReloadTileset(static_cast<TileSetID>(gPopup.ubSelectedIndex - 1));
			InitJA2SelectionWindow( );
			break;
		case CHANGECIVGROUP_POPUP:
			ChangeCivGroup( (UINT8)(gPopup.ubSelectedIndex - 1) );
			break;
		case SCHEDULEACTION_POPUP:
			UpdateScheduleAction( (UINT8)(gPopup.ubSelectedIndex - 1) );
			break;
		case ACTIONITEM_POPUP:
			UpdateActionItem( (UINT8)(gPopup.ubSelectedIndex - 1) );
			break;
		case OWNERSHIPGROUP_POPUP:
			SetOwnershipGroup( (UINT8)(gPopup.ubSelectedIndex - 1) );
			break;
	}
}

BOOLEAN ProcessPopupMenuIfActive( )
{
	if( !gPopup.fActive && !fWaitingForLButtonRelease )
		return FALSE;
	if( fWaitingForLButtonRelease )
	{
		if( !IsMouseButtonDown(MOUSE_BUTTON_LEFT) )
		{
			fWaitingForLButtonRelease = FALSE;
			return FALSE;
		}
		return TRUE;
	}
	PopupMenuHandle();
	RenderPopupMenu();
	InvalidateRegion( gPopup.usLeft, gPopup.usTop, gPopup.usRight, gPopup.usBottom );
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender( );
	return TRUE;
}
