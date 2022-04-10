#include "Button_System.h"
#include "Cursor_Modes.h"
#include "Directories.h"
#include "Font.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "Edit_Sys.h"
#include "SysUtil.h"
#include "Font_Control.h"
#include "WorldMan.h"			//HideStructOfGivenType()
#include "EditorDefines.h"
#include "EditorBuildings.h"
#include "EditorTerrain.h" //for access to TerrainTileDrawMode
#include "Render_Fun.h"
#include "NewSmooth.h"
#include "Editor_Undo.h"
#include "Editor_Taskbar_Utils.h"
#include "Editor_Modes.h"
#include "Smoothing_Utils.h"
#include "Text_Input.h"
#include "Keys.h"
#include "Environment.h"
#include "SelectWin.h"
#include "Simple_Render_Utils.h"
#include "Debug.h"
#include "MemMan.h"
#include "TileDat.h"
#include "Video.h"
#include "UILayout.h"

#include <string_theory/format>
#include <string_theory/string>


BOOLEAN fBuildingShowRoofs, fBuildingShowWalls, fBuildingShowRoomInfo;
UINT16 usCurrentMode;
UINT8 gubCurrRoomNumber;
UINT8 gubMaxRoomNumber;
BOOLEAN	gfEditingDoor;

//BEGINNNING OF BUILDING INITIALIZATION FUNCTIONS
void GameInitEditorBuildingInfo()
{
	fBuildingShowRoofs = TRUE;
	fBuildingShowWalls = TRUE;
	fBuildingShowRoomInfo = FALSE;
	usCurrentMode = BUILDING_PLACE_WALLS;
	gubCurrRoomNumber = gubMaxRoomNumber = 1;
}

//BEGINNING OF BUILDING UTILITY FUNCTIONS
void UpdateRoofsView()
{
	INT32 x;
	UINT16 usType;
	for ( x = 0; x < WORLD_MAX; x++ )
	{
		for ( usType = FIRSTROOF; usType <= LASTSLANTROOF; usType++ )
		{
			HideStructOfGivenType(x, usType, !fBuildingShowRoofs);
		}
	}
	gfRenderWorld = TRUE;
}

void UpdateWallsView()
{
	INT32 cnt;
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		if ( fBuildingShowWalls )
		{
			RemoveWallLevelnodeFlags( (INT16)cnt, LEVELNODE_HIDDEN );
		}
		else
		{
			SetWallLevelnodeFlags( (INT16)cnt, LEVELNODE_HIDDEN );
		}
	}
	gfRenderWorld = TRUE;
}

void UpdateBuildingsInfo()
{
	//print the headers on top of the columns
	SetFont( SMALLCOMPFONT );
	SetFontForeground( FONT_RED );
	MPrintEditor(112,  2, "TOGGLE");
	MPrintEditor(114, 12, "VIEWS");
	SetFontForeground( FONT_YELLOW );
	MPrintEditor(185,  2, "SELECTION METHOD");
	SetFontForeground( FONT_LTGREEN );
	MPrintEditor(290,  2, "SMART METHOD");
	SetFontForeground( FONT_LTBLUE );
	MPrintEditor(390,  2, "BUILDING METHOD");
	SetFontForeground( FONT_GRAY2 );
	MPrintEditor(437, 44, "Room#" );
}

//Uses a recursive method to elimate adjacent tiles of structure information.
//The code will attempt to delete the current mapindex, then search using this method:
//1) if there isn't structure info here, return.
//2) if there is structure info here, delete it now.
//3) KillBuilding at x-1, y.
//4) KillBuilding at x  , y-1.
//5) KillBuilding at x+1, y.
//6) KillBuilding at x  , y+1.
void KillBuilding( UINT32 iMapIndex )
{
	BOOLEAN fFound = FALSE;

	if( !gfBasement )
		fFound |= RemoveAllRoofsOfTypeRange( iMapIndex, FIRSTTEXTURE, LASTITEM );
	fFound |= RemoveAllLandsOfTypeRange( iMapIndex, FIRSTFLOOR, LASTFLOOR );

	EraseBuilding( iMapIndex );
	gubWorldRoomInfo[ iMapIndex ] = 0;

	if( !fFound )
	{
		if( gfBasement )
			RebuildRoof( iMapIndex, 0 );
		return;
	}

	if( GridNoOnVisibleWorldTile( (UINT16)( iMapIndex - WORLD_COLS ) ) )
		KillBuilding( iMapIndex - WORLD_COLS );
	if( GridNoOnVisibleWorldTile( (UINT16)( iMapIndex + WORLD_COLS ) ) )
		KillBuilding( iMapIndex + WORLD_COLS );
	if( GridNoOnVisibleWorldTile( (UINT16)( iMapIndex + 1 ) ) )
		KillBuilding( iMapIndex + 1 );
	if( GridNoOnVisibleWorldTile( (UINT16)( iMapIndex - 1 ) ) )
		KillBuilding( iMapIndex - 1 );

	if( gfBasement )
		RebuildRoof( iMapIndex, 0 );
}

BUILDINGLAYOUTNODE *gpBuildingLayoutList = NULL;
INT16 gsBuildingLayoutAnchorGridNo = -1;


void DeleteBuildingLayout()
{
	BUILDINGLAYOUTNODE *curr;
	//Erases the cursors associated with them.
	RemoveBuildingLayout();
	while( gpBuildingLayoutList )
	{
		curr = gpBuildingLayoutList;
		gpBuildingLayoutList = gpBuildingLayoutList->next;
		delete curr;
	}
	gpBuildingLayoutList = NULL;
	gsBuildingLayoutAnchorGridNo = -1;
}


static void BuildLayout(INT32 iMapIndex, INT32 iOffset)
{
	BUILDINGLAYOUTNODE *curr;
	//First, validate the gridno
	iMapIndex += iOffset;
	if (iMapIndex < 0 || iMapIndex >= WORLD_COLS*WORLD_ROWS)
		return;
	//Now, check if there is a building here
	if( !BuildingAtGridNo( iMapIndex ) )
	{
		if( iOffset == 1 && !BuildingAtGridNo( iMapIndex - 1 ) )
			return;
		if( iOffset == WORLD_COLS && !BuildingAtGridNo( iMapIndex - WORLD_COLS ) )
			return;
		if( iOffset == -1 && !GetVerticalWall( iMapIndex ) )
			return;
		if( iOffset == -WORLD_COLS && !GetHorizontalWall( iMapIndex ) )
			return;
	}
	//Now, check to make sure this gridno hasn't already been processed.
	curr = gpBuildingLayoutList;
	while( curr )
	{
		if( (INT16)iMapIndex == curr->sGridNo )
			return;
		curr = curr->next;
	}
	//Good, it hasn't, so process it and add it to the head of the list.
	curr = new BUILDINGLAYOUTNODE{};
	curr->sGridNo = (INT16)iMapIndex;
	curr->next = gpBuildingLayoutList;
	gpBuildingLayoutList = curr;

	//Use recursion to process the remainder.
	BuildLayout( iMapIndex, -WORLD_COLS );
	BuildLayout( iMapIndex, -1 );
	BuildLayout( iMapIndex, 1 );
	BuildLayout( iMapIndex, WORLD_COLS );
}

//The first step is copying a building.  After that, it either must be pasted or moved.
void CopyBuilding( INT32 iMapIndex )
{
	AssertMsg( !gpBuildingLayoutList, "Error:  Attempting to copy building multiple times." );

	//First step is to determine if we have a building in the area that we click.  If not, do nothing.
	if( !BuildingAtGridNo( iMapIndex ) )
		return;
	//Okay, a building does exist here to some undetermined capacity.
	//Allocate the basic structure, then calculate the layout.  The head node is
	gpBuildingLayoutList = new BUILDINGLAYOUTNODE{};
	gpBuildingLayoutList->sGridNo = (INT16)iMapIndex;
	gpBuildingLayoutList->next = NULL;

	//Set the anchor point for this building -- this is where the user clicked.
	gsBuildingLayoutAnchorGridNo = (INT16)iMapIndex;

	//Now, recursively expand out while adding unique gridnos to our list.  The recursion will
	//terminate when complete.
	BuildLayout( iMapIndex, -WORLD_COLS );
	BuildLayout( iMapIndex, -1 );
	BuildLayout( iMapIndex, 1 );
	BuildLayout( iMapIndex, WORLD_COLS );

	//We have our layout.  Now depending on the mode, we will either move the building or
	//copy it.  The layout automatically gets deleted as soon as the user releases the mouse
	//button.
}

//depending on the offset, we will either sort in increasing order, or decreasing order.
//This will prevent overlapping problems.
static void SortBuildingLayout(INT32 iMapIndex)
{
	BUILDINGLAYOUTNODE* prevBest = NULL; // XXX HACK000E
	BUILDINGLAYOUTNODE* best     = NULL; // XXX HACK000E
	BUILDINGLAYOUTNODE *head, *curr, *prev;
	INT32 iBestIndex;
	head = NULL;
	if( iMapIndex < gsBuildingLayoutAnchorGridNo )
	{	//Forward sort (in increasing order)
		while( gpBuildingLayoutList )
		{
			iBestIndex = -1;
			curr = gpBuildingLayoutList;
			prev = NULL;
			while( curr )
			{
				if( iBestIndex < curr->sGridNo )
				{
					iBestIndex = curr->sGridNo;
					prevBest = prev;
					best = curr;
				}
				prev = curr;
				curr = curr->next;
			}
			//detach node from real list
			if( prevBest )
				prevBest->next = best->next;
			if( best == gpBuildingLayoutList )
				gpBuildingLayoutList = gpBuildingLayoutList->next;
			//insert node into temp sorted list
			best->next = head;
			head = best;
		}
	}
	else
	{ //Reverse sort (in decreasing order)
		while( gpBuildingLayoutList )
		{
			iBestIndex = 100000;
			curr = gpBuildingLayoutList;
			prev = NULL;
			while( curr )
			{
				if( iBestIndex > curr->sGridNo )
				{
					iBestIndex = curr->sGridNo;
					prevBest = prev;
					best = curr;
				}
				prev = curr;
				curr = curr->next;
			}
			//detach node from real list
			if( prevBest )
				prevBest->next = best->next;
			if( best == gpBuildingLayoutList )
				gpBuildingLayoutList = gpBuildingLayoutList->next;
			//insert node into temp sorted list
			best->next = head;
			head = best;
		}
	}
	//Now assign the newly sorted list back to the real list.
	gpBuildingLayoutList = head;
}


static void PasteMapElementToNewMapElement(INT32 iSrcGridNo, INT32 iDstGridNo)
{
	MAP_ELEMENT			*pSrcMapElement;
	LEVELNODE				*pNode;
	UINT16					usType;

	DeleteStuffFromMapTile( iDstGridNo );
	DeleteAllLandLayers( iDstGridNo );

	//Get a pointer to the src mapelement
	pSrcMapElement = &gpWorldLevelData[ iSrcGridNo ];

	//Go through each levelnode, and paste the info into the new gridno
	pNode = pSrcMapElement->pLandHead;
	while( pNode )
	{
		if( pNode == pSrcMapElement->pLandStart )
			gpWorldLevelData[ iDstGridNo ].pLandStart = AddLandToTail( iDstGridNo, pNode->usIndex );
		else
			AddLandToTail( iDstGridNo, pNode->usIndex );
		pNode = pNode->pNext;
	}
	pNode = pSrcMapElement->pObjectHead;
	while( pNode )
	{
		AddObjectToTail( iDstGridNo, pNode->usIndex );
		pNode = pNode->pNext;
	}
	pNode = pSrcMapElement->pStructHead;
	while( pNode )
	{
		AddStructToTail( iDstGridNo, pNode->usIndex );
		pNode = pNode->pNext;
	}
	pNode = pSrcMapElement->pShadowHead;
	while( pNode )
	{
		AddShadowToTail( iDstGridNo, pNode->usIndex );
		pNode = pNode->pNext;
	}
	pNode = pSrcMapElement->pRoofHead;
	while( pNode )
	{
		AddRoofToTail( iDstGridNo, pNode->usIndex );
		pNode = pNode->pNext;
	}
	pNode = pSrcMapElement->pOnRoofHead;
	while( pNode )
	{
		AddOnRoofToTail( iDstGridNo, pNode->usIndex );
		pNode = pNode->pNext;
	}
	pNode = pSrcMapElement->pTopmostHead;
	while( pNode )
	{
		if( pNode->usIndex != FIRSTPOINTERS1 )
			AddTopmostToTail( iDstGridNo, pNode->usIndex );
		pNode = pNode->pNext;
	}
	for ( usType = FIRSTROOF; usType <= LASTSLANTROOF; usType++ )
	{
		HideStructOfGivenType(iDstGridNo, usType, !fBuildingShowRoofs);
	}
}

void MoveBuilding( INT32 iMapIndex )
{
	BUILDINGLAYOUTNODE *curr;
	INT32 iOffset;
	if( !gpBuildingLayoutList )
		return;
	SortBuildingLayout( iMapIndex );
	iOffset = iMapIndex - gsBuildingLayoutAnchorGridNo;
	//First time, set the undo gridnos to everything effected.
	curr = gpBuildingLayoutList;
	while( curr )
	{
		AddToUndoList( curr->sGridNo );
		AddToUndoList( curr->sGridNo + iOffset );
		curr = curr->next;
	}
	//Now, move the building
	curr = gpBuildingLayoutList;
	while( curr )
	{
		PasteMapElementToNewMapElement( curr->sGridNo, curr->sGridNo + iOffset );
		DeleteStuffFromMapTile( curr->sGridNo );
		curr = curr->next;
	}
	MarkWorldDirty();
}

void PasteBuilding( INT32 iMapIndex )
{
	BUILDINGLAYOUTNODE *curr;
	INT32 iOffset;
	if( !gpBuildingLayoutList )
		return;
	SortBuildingLayout( iMapIndex );
	iOffset = iMapIndex - gsBuildingLayoutAnchorGridNo;
	curr = gpBuildingLayoutList;
	//First time, set the undo gridnos to everything effected.
	while( curr )
	{
		AddToUndoList( curr->sGridNo );
		AddToUndoList( curr->sGridNo + iOffset );
		curr = curr->next;
	}
	//Now, paste the building (no smoothing)
	curr = gpBuildingLayoutList;
	while( curr )
	{
		PasteMapElementToNewMapElement( curr->sGridNo, curr->sGridNo + iOffset );
		curr = curr->next;
	}
	MarkWorldDirty();
}

struct ROOFNODE
{
	INT32 iMapIndex;
	ROOFNODE* next;
};

static ROOFNODE* gpRoofList = NULL;


static void ReplaceRoof(INT32 iMapIndex, UINT16 usRoofType)
{
	ROOFNODE *curr;
	//First, validate the gridno
	if (iMapIndex < 0 || iMapIndex >= WORLD_COLS*WORLD_ROWS)
		return;
	//Now, check if there is a floor here
	if( !FloorAtGridNo( iMapIndex ) )
		return;
	//Now, check to make sure this gridno hasn't already been processed.
	curr = gpRoofList;
	while( curr )
	{
		if( iMapIndex == curr->iMapIndex )
			return;
		curr = curr->next;
	}
	//Good, it hasn't, so process it and add it to the head of the list.
	curr = new ROOFNODE{};
	curr->iMapIndex = iMapIndex;
	curr->next = gpRoofList;
	gpRoofList = curr;

	RebuildRoofUsingFloorInfo( iMapIndex, usRoofType );

	//Use recursion to process the remainder.
	ReplaceRoof( iMapIndex - WORLD_COLS, usRoofType );
	ReplaceRoof( iMapIndex + WORLD_COLS, usRoofType );
	ReplaceRoof( iMapIndex - 1, usRoofType );
	ReplaceRoof( iMapIndex + 1, usRoofType );
}

void ReplaceBuildingWithNewRoof( INT32 iMapIndex )
{
	UINT16 usRoofType;
	ROOFNODE *curr;
	//Not in normal editor mode, then can't do it.
	if( gfBasement || gfCaves )
		return;
	//if we don't have a floor here, then we can't replace the roof!
	if( !FloorAtGridNo( iMapIndex ) )
		return;
	//Extract the selected roof type.
	usRoofType = (UINT16)SelSingleNewRoof[ iCurBank ].uiObject;

	//now start building a linked list of all nodes visited -- start the first node.
	gpRoofList = new ROOFNODE{};
	gpRoofList->iMapIndex = iMapIndex;
	gpRoofList->next = 0;
	RebuildRoofUsingFloorInfo( iMapIndex, usRoofType );

	//Use recursion to process the remainder.
	ReplaceRoof( iMapIndex - WORLD_COLS, usRoofType );
	ReplaceRoof( iMapIndex + WORLD_COLS, usRoofType );
	ReplaceRoof( iMapIndex - 1, usRoofType );
	ReplaceRoof( iMapIndex + 1, usRoofType );

	//Done, so delete the list.
	while( gpRoofList )
	{
		curr = gpRoofList;
		gpRoofList = gpRoofList->next;
		delete curr;
	}
	gpRoofList = NULL;
}

//internal door editing vars.
static INT32 iDoorMapIndex = 0;
enum{
	DOOR_BACKGROUND,
	DOOR_OKAY,
	DOOR_CANCEL,
	DOOR_LOCKED,
	NUM_DOOR_BUTTONS
};
static GUIButtonRef iDoorButton[NUM_DOOR_BUTTONS];
static MOUSE_REGION DoorRegion;


static void DoorCancelCallback(GUI_BUTTON* btn, INT32 reason);
static void DoorOkayCallback(GUI_BUTTON* btn, INT32 reason);
static void DoorToggleLockedCallback(GUI_BUTTON* btn, INT32 reason);


void InitDoorEditing(INT32 const map_idx)
{
	if (!DoorAtGridNo(map_idx) && !OpenableAtGridNo(map_idx)) return;

	gfEditingDoor = TRUE;
	iDoorMapIndex = map_idx;
	DisableEditorTaskbar();
	MSYS_DefineRegion(&DoorRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH - 2, 0, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
	iDoorButton[DOOR_BACKGROUND] = CreateLabel(ST::null, 0, 0, 0, 200, 130, 240, 100, MSYS_PRIORITY_HIGH - 1);
	iDoorButton[DOOR_OKAY]       = CreateTextButton("Okay",   FONT12POINT1, FONT_BLACK, FONT_BLACK, 330, 195, 50, 30, MSYS_PRIORITY_HIGH, DoorOkayCallback);
	iDoorButton[DOOR_CANCEL]     = CreateTextButton("Cancel", FONT12POINT1, FONT_BLACK, FONT_BLACK, 385, 195, 50, 30, MSYS_PRIORITY_HIGH, DoorCancelCallback);
	InitTextInputModeWithScheme(DEFAULT_SCHEME);
	AddTextInputField(210, 155, 25, 16, MSYS_PRIORITY_HIGH, "0", 3, INPUTTYPE_NUMERICSTRICT);
	AddTextInputField(210, 175, 25, 16, MSYS_PRIORITY_HIGH, "0", 2, INPUTTYPE_NUMERICSTRICT);
	AddTextInputField(210, 195, 25, 16, MSYS_PRIORITY_HIGH, "0", 2, INPUTTYPE_NUMERICSTRICT);
	iDoorButton[DOOR_LOCKED] = CreateCheckBoxButton(210, 215, EDITORDIR "/smcheckbox.sti", MSYS_PRIORITY_HIGH, DoorToggleLockedCallback);

	if (DOOR const* const door = FindDoorInfoAtGridNo(map_idx))
	{
		SetInputFieldStringWithNumericStrictValue(0, door->ubLockID);
		SetInputFieldStringWithNumericStrictValue(1, door->ubTrapID);
		SetInputFieldStringWithNumericStrictValue(2, door->ubTrapLevel);
		if (!door->fLocked) return;
	}
	iDoorButton[DOOR_LOCKED]->uiFlags |= BUTTON_CLICKED_ON;
}


void ExtractAndUpdateDoorInfo()
{
	LEVELNODE* pNode;
	INT32 num;
	DOOR door;
	BOOLEAN fCursor = FALSE;
	BOOLEAN fCursorExists = FALSE;

	door = DOOR{};

	door.sGridNo = (INT16)iDoorMapIndex;

	num = std::min(GetNumericStrictValueFromField( 0 ), NUM_LOCKS-1);
	door.ubLockID = (UINT8)num;
	SetInputFieldStringWithNumericStrictValue( 0, num );
	if( num >= 0 )
		fCursor = TRUE;

	num = std::clamp(GetNumericStrictValueFromField(1), 0, 10);
	door.ubTrapID = (UINT8)num;
	SetInputFieldStringWithNumericStrictValue( 1, num );
	if( num )
		fCursor = TRUE;

	num = std::clamp(GetNumericStrictValueFromField(2), 0, 20);
	if( door.ubTrapID && !num )
		num = 1;  //Can't have a trap without a traplevel!
	door.ubTrapLevel = (UINT8)num;
	SetInputFieldStringWithNumericStrictValue( 2, num );
	if( num )
		fCursor = TRUE;

	door.fLocked = iDoorButton[DOOR_LOCKED]->Clicked();

	//Find out if we have a rotating key cursor (we will either add one or remove one)
	pNode = gpWorldLevelData[ iDoorMapIndex ].pTopmostHead;
	while( pNode )
	{
		if( pNode->usIndex == ROTATINGKEY1 )
		{
			fCursorExists = TRUE;
			break;
		}
		pNode = pNode->pNext;
	}
	if( fCursor )
	{ //we have a valid door, so add it (or replace existing)
		if( !fCursorExists )
			AddTopmostToHead( iDoorMapIndex, ROTATINGKEY1 );
		//If the door already exists, the new information will replace it.
		AddDoorInfoToTable( &door );
	}
	else
	{ //if a door exists here, remove it.
		if( fCursorExists )
			RemoveAllTopmostsOfTypeRange( iDoorMapIndex, ROTATINGKEY, ROTATINGKEY );
		RemoveDoorInfoFromTable( iDoorMapIndex );
	}

}

void FindNextLockedDoor()
{
	DOOR *pDoor;
	INT32 i;
	for( i = iDoorMapIndex + 1; i < WORLD_MAX; i++ )
	{
		pDoor = FindDoorInfoAtGridNo( i );
		if( pDoor )
		{
			CenterScreenAtMapIndex( i );
			iDoorMapIndex = i;
			return;
		}
	}
	for( i = 0; i <= iDoorMapIndex; i++ )
	{
		pDoor = FindDoorInfoAtGridNo( i );
		if( pDoor )
		{
			CenterScreenAtMapIndex( i );
			iDoorMapIndex = i;
			return;
		}
	}
}

void RenderDoorEditingWindow()
{
	InvalidateRegion( 200, 130, 440, 230 );
	SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
	MPrint( 210, 140, ST::format("Editing lock attributes at map index {}.", iDoorMapIndex) );

	SetFontForeground( FONT_GRAY2 );
	MPrint(238, 160, "Lock ID");
	MPrint(238, 180, "Trap Type");
	MPrint(238, 200, "Trap Level");
	MPrint(238, 218, "Locked");
}


void KillDoorEditing()
{
	EnableEditorTaskbar();
	MSYS_RemoveRegion(&DoorRegion);
	FOR_EACH(GUIButtonRef, i, iDoorButton) RemoveButton(*i);
	gfEditingDoor = FALSE;
	KillTextInputMode();
}


static void DoorOkayCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ExtractAndUpdateDoorInfo();
		KillDoorEditing();
	}
}


static void DoorCancelCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		KillDoorEditing();
	}
}


static void DoorToggleLockedCallback(GUI_BUTTON* btn, INT32 reason)
{
	//handled in ExtractAndUpdateDoorInfo();
}


void AddLockedDoorCursors()
{
	FOR_EACH_DOOR(d)
	{
		AddTopmostToHead(d.sGridNo, ROTATINGKEY1);
	}
}


void RemoveLockedDoorCursors()
{
	FOR_EACH_DOOR(d)
	{
		GridNo const gridno = d.sGridNo;
		for (LEVELNODE* k = gpWorldLevelData[gridno].pTopmostHead; k;)
		{
			LEVELNODE* const next = k->pNext;
			if (k->usIndex == ROTATINGKEY1) RemoveTopmostFromLevelNode(gridno, k);
			k = next;
		}
	}
}


void SetupTextInputForBuildings()
{
	ST::string str;
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	AddUserInputField( NULL );  //just so we can use short cut keys while not typing.
	str = ST::format("{}", gubMaxRoomNumber);
	AddTextInputField( 410, EDITOR_TASKBAR_POS_Y + 40, 25, 15, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
}

void ExtractAndUpdateBuildingInfo()
{
	ST::string str;
	INT32 temp;
	//extract light1 colors
	temp = std::min(GetNumericStrictValueFromField( 1 ), 255);
	if( temp != -1 )
	{
		gubCurrRoomNumber = (UINT8)temp;
	}
	else
	{
		gubCurrRoomNumber = 0;
	}
	str = ST::format("{}", gubCurrRoomNumber);
	SetInputFieldString( 1, str );
	SetActiveField( 0 );
}
