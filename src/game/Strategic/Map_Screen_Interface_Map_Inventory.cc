#include "Auto_Resolve.h"
#include "Buffer.h"
#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Handle_Items.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "MessageBoxScreen.h"
#include "Render_Dirty.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "SysUtil.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Map.h"
#include "Items.h"
#include "Interface_Items.h"
#include "Cursor_Control.h"
#include "Interface_Utils.h"
#include "Text.h"
#include "Font_Control.h"
#include "StrategicMap.h"
#include "World_Items.h"
#include "Tactical_Save.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "English.h"
#include "Multi_Language_Graphic_Utils.h"
#include "MapScreen.h"
#include "Radar_Screen.h"
#include "Message.h"
#include "Interface_Panels.h"
#include "WordWrap.h"
#include "Button_System.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "VSurface.h"
#include "MemMan.h"
#include "ShopKeeper_Interface.h"
#include "ArmsDealerInvInit.h"

#include "ContentManager.h"
#include "GameInstance.h"


// status bar colors
#define DESC_STATUS_BAR FROMRGB( 201, 172,  133 )
#define DESC_STATUS_BAR_SHADOW FROMRGB( 140, 136,  119 )

// delay for flash of item
#define DELAY_FOR_HIGHLIGHT_ITEM_FLASH 200

// inventory slot font
#define MAP_IVEN_FONT						SMALLCOMPFONT

// inventory pool slot positions and sizes
#define MAP_INV_SLOT_ROWS 9


static const SGPBox g_sector_inv_box        = { 261,   0, 379, 360 };
static const SGPBox g_sector_inv_title_box  = { 266,   5, 370,  29 };
static const SGPBox g_sector_inv_slot_box   = { 274,  37,  72,  32 };
static const SGPBox g_sector_inv_region_box = {   0,   0,  67,  31 }; // relative to g_sector_inv_slot_box
static const SGPBox g_sector_inv_item_box   = {   6,   0,  61,  24 }; // relative to g_sector_inv_slot_box
static const SGPBox g_sector_inv_bar_box    = {   2,   2,   2,  20 }; // relative to g_sector_inv_slot_box
static const SGPBox g_sector_inv_name_box   = {   0,  24,  67,   7 }; // relative to g_sector_inv_slot_box
static const SGPBox g_sector_inv_loc_box    = { 326, 337,  39,  10 };
static const SGPBox g_sector_inv_count_box  = { 437, 337,  39,  10 };
static const SGPBox g_sector_inv_page_box   = { 505, 337,  50,  10 };


// the current highlighted item
INT32 iCurrentlyHighLightedItem = -1;
BOOLEAN fFlashHighLightInventoryItemOnradarMap = FALSE;

// whether we are showing the inventory pool graphic
BOOLEAN fShowMapInventoryPool = FALSE;

// the v-object index value for the background
static SGPVObject* guiMapInventoryPoolBackground;

// inventory pool list
WORLDITEM *pInventoryPoolList = NULL;

// current page of inventory
INT32 iCurrentInventoryPoolPage = 0;
static INT32 iLastInventoryPoolPage = 0;

// total number of slots allocated
static INT32 iTotalNumberOfSlots = 0;

INT16 sObjectSourceGridNo = 0;

// number of unseen items in sector
static UINT32 uiNumberOfUnSeenItems = 0;


// the inventory slots
static MOUSE_REGION MapInventoryPoolSlots[MAP_INVENTORY_POOL_SLOT_COUNT];
static MOUSE_REGION MapInventoryPoolMask;
BOOLEAN fMapInventoryItemCompatable[ MAP_INVENTORY_POOL_SLOT_COUNT ];
static BOOLEAN      fChangedInventorySlots = FALSE;

// the unseen items list...have to save this
static WORLDITEM* pUnSeenItems = NULL;

INT32 giFlashHighlightedItemBaseTime = 0;
INT32 giCompatibleItemBaseTime = 0;

static GUIButtonRef guiMapInvenButton[3];

static BOOLEAN gfCheckForCursorOverMapSectorInventoryItem = FALSE;


// load the background panel graphics for inventory
void LoadInventoryPoolGraphic(void)
{
	// add to V-object index
	guiMapInventoryPoolBackground = AddVideoObjectFromFile(INTERFACEDIR "/sector_inventory.sti");
}


// remove background panel graphics for inventory
void RemoveInventoryPoolGraphic( void )
{
	// remove from v-object index
	if( guiMapInventoryPoolBackground )
	{
		DeleteVideoObject(guiMapInventoryPoolBackground);
		guiMapInventoryPoolBackground = 0;
	}
}


static void CheckAndUnDateSlotAllocation(void);
static void DisplayCurrentSector(void);
static void DisplayPagesForMapInventoryPool(void);
static void DrawNumberOfIventoryPoolItems(void);
static void DrawTextOnMapInventoryBackground(void);
static void RenderItemsForCurrentPageOfInventoryPool(void);
static void UpdateHelpTextForInvnentoryStashSlots(void);


// blit the background panel for the inventory
void BlitInventoryPoolGraphic( void )
{
	const SGPBox* const box = &g_sector_inv_box;
	BltVideoObject(guiSAVEBUFFER, guiMapInventoryPoolBackground, 0, STD_SCREEN_X + box->x, STD_SCREEN_Y + box->y);

	// resize list
	CheckAndUnDateSlotAllocation( );


	// now the items
	RenderItemsForCurrentPageOfInventoryPool( );

	// now update help text
	UpdateHelpTextForInvnentoryStashSlots( );

	// show which page and last page
	DisplayPagesForMapInventoryPool( );

	// draw number of items in current inventory
	DrawNumberOfIventoryPoolItems( );

	// display current sector inventory pool is at
	DisplayCurrentSector( );

	DrawTextOnMapInventoryBackground( );

	// re render buttons
	MarkButtonsDirty( );

	// which buttons will be active and which ones not
	HandleButtonStatesWhileMapInventoryActive( );
}


static BOOLEAN RenderItemInPoolSlot(INT32 iCurrentSlot, INT32 iFirstSlotOnPage);


static void RenderItemsForCurrentPageOfInventoryPool(void)
{
	INT32 iCounter = 0;

	// go through list of items on this page and place graphics to screen
	for( iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT ; iCounter++ )
	{
		RenderItemInPoolSlot( iCounter, ( iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT ) );
	}
}


static BOOLEAN RenderItemInPoolSlot(INT32 iCurrentSlot, INT32 iFirstSlotOnPage)
{
	// render item in this slot of the list
	const WORLDITEM* const item = &pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage];

	// check if anything there
	if (item->o.ubNumberOfObjects == 0) return FALSE;

	const SGPBox* const slot_box = &g_sector_inv_slot_box;
	const INT32 dx = STD_SCREEN_X + slot_box->x + slot_box->w * (iCurrentSlot / MAP_INV_SLOT_ROWS);
	const INT32 dy = STD_SCREEN_Y + slot_box->y + slot_box->h * (iCurrentSlot % MAP_INV_SLOT_ROWS);

	SetFontDestBuffer(guiSAVEBUFFER);
	const SGPBox* const item_box = &g_sector_inv_item_box;
	const UINT16        outline  = fMapInventoryItemCompatable[iCurrentSlot] ? Get16BPPColor(FROMRGB(255, 255, 255)) : SGP_TRANSPARENT;
	INVRenderItem(guiSAVEBUFFER, NULL, item->o, dx + item_box->x, dy + item_box->y, item_box->w, item_box->h, DIRTYLEVEL2, 0, outline);

	// draw bar for condition
	const UINT16 col0 = Get16BPPColor(DESC_STATUS_BAR);
	const UINT16 col1 = Get16BPPColor(DESC_STATUS_BAR_SHADOW);
	const SGPBox* const bar_box = &g_sector_inv_bar_box;
	DrawItemUIBarEx(item->o, 0, dx + bar_box->x, dy + bar_box->y + bar_box->h - 1, bar_box->h, col0, col1, guiSAVEBUFFER);

	// if the item is not reachable, or if the selected merc is not in the current sector
	const SOLDIERTYPE* const s = GetSelectedInfoChar();
	if (!(item->usFlags & WORLD_ITEM_REACHABLE) ||
			s           == NULL     ||
			s->sSectorX != sSelMapX ||
			s->sSectorY != sSelMapY ||
			s->bSectorZ != iCurrentMapSectorZ)
	{
		//Shade the item
		DrawHatchOnInventory(guiSAVEBUFFER, dx + item_box->x, dy + item_box->y, item_box->w, item_box->h);
	}

	// the name
	const SGPBox* const name_box = &g_sector_inv_name_box;
	wchar_t sString[SIZE_SHORT_ITEM_NAME];
	wcscpy(sString, ShortItemNames[item->o.usItem]);
	ReduceStringLength(sString, lengthof(sString), name_box->w, MAP_IVEN_FONT);

	SetFontAttributes(MAP_IVEN_FONT, FONT_WHITE);

	INT16 x;
	INT16 y;
	FindFontCenterCoordinates(dx + name_box->x, dy + name_box->y, name_box->w, name_box->h, sString, MAP_IVEN_FONT, &x, &y);
	MPrint(x, y, sString);

	SetFontDestBuffer(FRAME_BUFFER);

	return TRUE;
}


static void UpdateHelpTextForInvnentoryStashSlots(void)
{
	wchar_t pStr[ 512 ];
	INT32 iCounter = 0;
	INT32 iFirstSlotOnPage = ( iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT );


	// run through list of items in slots and update help text for mouse regions
	for( iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++ )
	{
		wchar_t    const* help = L"";
		OBJECTTYPE const& o    = pInventoryPoolList[iCounter + iFirstSlotOnPage].o;
		if  (o.ubNumberOfObjects > 0)
		{
			GetHelpTextForItem(pStr, lengthof(pStr), o);
			help = pStr;
		}
		MapInventoryPoolSlots[iCounter].SetFastHelpText(help);
	}
}


static void BuildStashForSelectedSector(INT16 sMapX, INT16 sMapY, INT16 sMapZ);
static void CreateMapInventoryButtons(void);
static void CreateMapInventoryPoolDoneButton(void);
static void CreateMapInventoryPoolSlots(void);
static void DestroyInventoryPoolDoneButton(void);
static void DestroyMapInventoryButtons(void);
static void DestroyMapInventoryPoolSlots();
static void DestroyStash(void);
static void HandleMapSectorInventory(void);
static void SaveSeenAndUnseenItems(void);


// create and remove buttons for inventory
void CreateDestroyMapInventoryPoolButtons( BOOLEAN fExitFromMapScreen )
{
	static BOOLEAN fCreated = FALSE;

/* player can leave items underground, no?
	if( iCurrentMapSectorZ )
	{
		fShowMapInventoryPool = FALSE;
	}
*/

	if (fShowMapInventoryPool && !fCreated)
	{
		if( ( gWorldSectorX == sSelMapX ) && ( gWorldSectorY == sSelMapY ) && ( gbWorldSectorZ == iCurrentMapSectorZ ) )
		{
			// handle all reachable before save
			HandleAllReachAbleItemsInTheSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		}

		// destroy buttons for map border
		DeleteMapBorderButtons( );

		fCreated = TRUE;

		// also create the inventory slot
		CreateMapInventoryPoolSlots( );

		// create buttons
		CreateMapInventoryButtons( );

		// build stash
		BuildStashForSelectedSector( sSelMapX, sSelMapY, ( INT16 )( iCurrentMapSectorZ ) );

		CreateMapInventoryPoolDoneButton( );

		fMapPanelDirty = TRUE;
		fMapScreenBottomDirty = TRUE;
	}
	else if (!fShowMapInventoryPool && fCreated)
	{

		// check fi we are in fact leaving mapscreen
		if (!fExitFromMapScreen)
		{
			// recreate mapborder buttons
			CreateButtonsForMapBorder( );
		}
		fCreated = FALSE;

		// destroy the map inventory slots
		DestroyMapInventoryPoolSlots( );

		// destroy map inventory buttons
		DestroyMapInventoryButtons( );

		DestroyInventoryPoolDoneButton( );

		// now save results
		SaveSeenAndUnseenItems( );

		DestroyStash( );



		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		//DEF: added to remove the 'item blip' from staying on the radar map
		iCurrentlyHighLightedItem = -1;

		// re render radar map
		RenderRadarScreen( );
	}

	// do our handling here
	HandleMapSectorInventory( );

}


void CancelSectorInventoryDisplayIfOn( BOOLEAN fExitFromMapScreen )
{
	if ( fShowMapInventoryPool )
	{
		// get rid of sector inventory mode & buttons
		fShowMapInventoryPool = FALSE;
		CreateDestroyMapInventoryPoolButtons( fExitFromMapScreen );
	}
}


static INT32 GetTotalNumberOfItems(void);
static void ReBuildWorldItemStashForLoadedSector(INT32 iNumberSeenItems, INT32 iNumberUnSeenItems, const WORLDITEM* pSeenItemsList, const WORLDITEM* pUnSeenItemsList);


static void SaveSeenAndUnseenItems(void)
{
	const INT32 iTotalNumberItems = GetTotalNumberOfItems();

	// if there are seen items, build a temp world items list of them and save them
	INT32                  iItemCount = 0;
	SGP::Buffer<WORLDITEM> pSeenItemsList;
	if (iTotalNumberItems > 0)
	{
		pSeenItemsList.Allocate(iTotalNumberItems);

		// copy
		for (INT32 iCounter = 0; iCounter < iTotalNumberOfSlots; ++iCounter)
		{
			const WORLDITEM* const pi = &pInventoryPoolList[iCounter];
			if (pi->o.ubNumberOfObjects == 0) continue;

			WORLDITEM* const si = &pSeenItemsList[iItemCount++];
			*si = *pi;

			// Check if item actually lives at a gridno
			if (si->sGridNo == 0)
			{
				// Use gridno of predecessor, if there is one
				if (si != pSeenItemsList)
				{
					// borrow from predecessor
					si->sGridNo = si[-1].sGridNo;
				}
				else
				{
					// get entry grid location
				}
			}
			si->fExists  = TRUE;
			si->bVisible = TRUE;
		}
	}

	// if this is the loaded sector handle here
	if (gWorldSectorX  == sSelMapX &&
			gWorldSectorY  == sSelMapY &&
			gbWorldSectorZ == (INT8)iCurrentMapSectorZ)
	{
		ReBuildWorldItemStashForLoadedSector(iItemCount, uiNumberOfUnSeenItems, pSeenItemsList, pUnSeenItems);
	}
	else
	{
		// now copy over unseen and seen
		SaveWorldItemsToTempItemFile( sSelMapX, sSelMapY, iCurrentMapSectorZ, uiNumberOfUnSeenItems, pUnSeenItems);
		AddWorldItemsToUnLoadedSector(sSelMapX, sSelMapY, iCurrentMapSectorZ, iItemCount,            pSeenItemsList);
	}
}


static void InventoryNextPage()
{
	if (iCurrentInventoryPoolPage < iLastInventoryPoolPage)
	{
		++iCurrentInventoryPoolPage;
		fMapPanelDirty = TRUE;
	}
}


static void InventoryPrevPage()
{
	if (iCurrentInventoryPoolPage > 0)
	{
		--iCurrentInventoryPoolPage;
		fMapPanelDirty = TRUE;
	}
}


// the screen mask bttn callaback...to disable the inventory and lock out the map itself
static void MapInvenPoolScreenMaskCallback(MOUSE_REGION* pRegion, INT32 iReason)
{

	if( ( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP ) )
	{
		fShowMapInventoryPool = FALSE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		InventoryPrevPage();
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		InventoryNextPage();
	}
}


static void MapInvenPoolSlots(MOUSE_REGION* pRegion, INT32 iReason);
static void MapInvenPoolSlotsMove(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateMapInventoryPoolSlots(void)
{
	{
		const SGPBox* const inv_box = &g_sector_inv_box;
		UINT16        const x       = STD_SCREEN_X + inv_box->x;
		UINT16        const y       = STD_SCREEN_Y + inv_box->y;
		UINT16        const w       = inv_box->w;
		UINT16        const h       = inv_box->h;
		MSYS_DefineRegion(&MapInventoryPoolMask, x, y, x + w - 1, y + h - 1, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MapInvenPoolScreenMaskCallback);
	}

	const SGPBox* const slot_box = &g_sector_inv_slot_box;
	const SGPBox* const reg_box  = &g_sector_inv_region_box;
	for (UINT i = 0; i < MAP_INVENTORY_POOL_SLOT_COUNT; ++i)
	{
		UINT16        const sx = i / MAP_INV_SLOT_ROWS;
		UINT16        const sy = i % MAP_INV_SLOT_ROWS;
		UINT16        const x  = reg_box->x + STD_SCREEN_X + slot_box->x + sx * slot_box->w;
		UINT16        const y  = reg_box->y + STD_SCREEN_Y + slot_box->y + sy * slot_box->h;
		UINT16        const w  = reg_box->w;
		UINT16        const h  = reg_box->h;
		MOUSE_REGION* const r  = &MapInventoryPoolSlots[i];
		MSYS_DefineRegion(r, x, y, x + w - 1, y + h - 1, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MapInvenPoolSlotsMove, MapInvenPoolSlots);
		MSYS_SetRegionUserData(r, 0, i);
	}
}


static void DestroyMapInventoryPoolSlots()
{
	FOR_EACH(MOUSE_REGION, i, MapInventoryPoolSlots) MSYS_RemoveRegion(&*i);
	MSYS_RemoveRegion(&MapInventoryPoolMask);
}


static void MapInvenPoolSlotsMove(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iCounter = 0;


	iCounter = MSYS_GetRegionUserData( pRegion, 0 );

	if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		iCurrentlyHighLightedItem = iCounter;
		fChangedInventorySlots = TRUE;
		gfCheckForCursorOverMapSectorInventoryItem = TRUE;
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		iCurrentlyHighLightedItem = -1;
		fChangedInventorySlots = TRUE;
		gfCheckForCursorOverMapSectorInventoryItem = FALSE;

		// re render radar map
		RenderRadarScreen( );
	}
}


static void BeginInventoryPoolPtr(OBJECTTYPE* pInventorySlot);
static BOOLEAN CanPlayerUseSectorInventory(void);
static BOOLEAN PlaceObjectInInventoryStash(OBJECTTYPE* pInventorySlot, OBJECTTYPE* pItemPtr);


static void MapInvenPoolSlots(MOUSE_REGION* const pRegion, const INT32 iReason)
{
	// btn callback handler for assignment screen mask region
	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		if (gpItemPointer == NULL) fShowMapInventoryPool = FALSE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// check if item in cursor, if so, then swap, and no item in curor, pick up, if item in cursor but not box, put in box
		INT32      const slot_idx = MSYS_GetRegionUserData(pRegion, 0);
		WORLDITEM* const slot     = &pInventoryPoolList[iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT + slot_idx];

		// Return if empty
		if (gpItemPointer == NULL && slot->o.usItem == NOTHING) return;

		// is this item reachable
		if (slot->o.usItem != NOTHING && !(slot->usFlags & WORLD_ITEM_REACHABLE))
		{
			// not reachable
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[STR_LATE_38], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
			return;
		}

		// Valid character?
		const SOLDIERTYPE* const s = GetSelectedInfoChar();
		if (s == NULL)
		{
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapInventoryErrorString[0], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
			return;
		}

		// Check if selected merc is in this sector, if not, warn them and leave
		if (s->sSectorX != sSelMapX           ||
				s->sSectorY != sSelMapY           ||
				s->bSectorZ != iCurrentMapSectorZ ||
				s->fBetweenSectors)
		{
			const wchar_t* const msg = (gpItemPointer == NULL ? pMapInventoryErrorString[1] : pMapInventoryErrorString[4]);
			wchar_t buf[128];
			swprintf(buf, lengthof(buf), msg, s->name);
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, buf, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
			return;
		}

		// If in battle inform player they will have to do this in tactical
		if (!CanPlayerUseSectorInventory())
		{
			const wchar_t* const msg = (gpItemPointer == NULL ? pMapInventoryErrorString[2] : pMapInventoryErrorString[3]);
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, msg, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
			return;
		}

		// If we do not have an item in hand, start moving it
		if (gpItemPointer == NULL)
		{
			sObjectSourceGridNo = slot->sGridNo;
			BeginInventoryPoolPtr(&slot->o);
		}
		else
		{
			const INT32 iOldNumberOfObjects = slot->o.ubNumberOfObjects;

			// Else, try to place here
			if (PlaceObjectInInventoryStash(&slot->o, gpItemPointer))
			{
				// nothing here before, then place here
				if (iOldNumberOfObjects == 0)
				{
					slot->sGridNo                  = sObjectSourceGridNo;
					slot->ubLevel                  = s->bLevel;
					slot->usFlags                  = 0;
					slot->bRenderZHeightAboveLevel = 0;

					if (sObjectSourceGridNo == NOWHERE)
					{
						slot->usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;
					}
				}

				slot->usFlags |= WORLD_ITEM_REACHABLE;

				// Check if it's the same now!
				if (gpItemPointer->ubNumberOfObjects == 0)
				{
					MAPEndItemPointer();
				}
				else
				{
					SetMapCursorItem();
				}
			}
		}

		// dirty region, force update
		fMapPanelDirty = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		InventoryPrevPage();
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		InventoryNextPage();
	}
}


static void MapInventoryPoolPrevBtn(GUI_BUTTON* btn, INT32 reason);
static void MapInventoryPoolNextBtn(GUI_BUTTON* btn, INT32 reason);


static void CreateMapInventoryButtons(void)
{
	guiMapInvenButton[0] = QuickCreateButtonImg(INTERFACEDIR "/map_screen_bottom_arrows.sti", 10, 1, -1, 3, -1, STD_SCREEN_X + 559, STD_SCREEN_Y + 336, MSYS_PRIORITY_HIGHEST, MapInventoryPoolNextBtn);
	guiMapInvenButton[1] = QuickCreateButtonImg(INTERFACEDIR "/map_screen_bottom_arrows.sti",  9, 0, -1, 2, -1, STD_SCREEN_X + 487, STD_SCREEN_Y + 336, MSYS_PRIORITY_HIGHEST, MapInventoryPoolPrevBtn);

	//reset the current inventory page to be the first page
	iCurrentInventoryPoolPage = 0;
}


static void DestroyMapInventoryButtons(void)
{
	RemoveButton( guiMapInvenButton[ 0 ] );
	RemoveButton( guiMapInvenButton[ 1 ] );
}


static void CheckGridNoOfItemsInMapScreenMapInventory(void);
static void SortSectorInventory(WORLDITEM* pInventory, UINT32 uiSizeOfArray);


static void BuildStashForSelectedSector(const INT16 sMapX, const INT16 sMapY, const INT16 sMapZ)
{
	WORLDITEM*       items;
	const WORLDITEM* items_end;
	if (sMapX == gWorldSectorX &&
			sMapY == gWorldSectorY &&
			sMapZ == gbWorldSectorZ)
	{
		items      = gWorldItems;
		items_end  = gWorldItems + guiNumWorldItems;
	}
	else
	{
		UINT32 item_count;
		LoadWorldItemsFromTempItemFile(sMapX, sMapY, sMapZ, &item_count, &items);
		items_end  = items + item_count;
	}

	UINT32 visible_count = 0;
	UINT32 unseen_count  = 0;
	for (const WORLDITEM* wi = items; wi != items_end; ++wi)
	{
		if (!wi->fExists) continue;
		if (IsMapScreenWorldItemVisibleInMapInventory(wi))
		{
			++visible_count;
		}
		else
		{
			++unseen_count;
		}
	}

	const UINT32 slot_count = visible_count - visible_count % MAP_INVENTORY_POOL_SLOT_COUNT + MAP_INVENTORY_POOL_SLOT_COUNT;
	iLastInventoryPoolPage  = (slot_count - 1) / MAP_INVENTORY_POOL_SLOT_COUNT;

	WORLDITEM* visible_item = MALLOCNZ(WORLDITEM, slot_count);
	WORLDITEM* unseen_item  = (unseen_count != 0 ? MALLOCN(WORLDITEM, unseen_count) : NULL);

	iTotalNumberOfSlots      = slot_count;
	pInventoryPoolList       = visible_item;
	uiNumberOfUnSeenItems    = unseen_count;
	pUnSeenItems             = unseen_item;

	for (const WORLDITEM* wi = items; wi != items_end; ++wi)
	{
		if (!wi->fExists) continue;
		if (IsMapScreenWorldItemVisibleInMapInventory(wi))
		{
			*visible_item++ = *wi;
			Assert(visible_item <= pInventoryPoolList + visible_count);
		}
		else
		{
			*unseen_item++ = *wi;
			Assert(unseen_item <= pUnSeenItems + unseen_count);
		}
	}
	Assert(visible_item == pInventoryPoolList + visible_count);
	Assert(unseen_item  == pUnSeenItems       + unseen_count);

	if (items != gWorldItems && items != NULL) MemFree(items);

	CheckGridNoOfItemsInMapScreenMapInventory();
	SortSectorInventory(pInventoryPoolList, visible_count);
}


static void ReBuildWorldItemStashForLoadedSector(const INT32 iNumberSeenItems, const INT32 iNumberUnSeenItems, const WORLDITEM* const pSeenItemsList, const WORLDITEM* const pUnSeenItemsList)
{
	TrashWorldItems();

	// get total number of items
	INT32 iTotalNumberOfItems = iNumberUnSeenItems + iNumberSeenItems;

	const INT32 iRemainder = iTotalNumberOfItems % 10;

	// if there is a remainder, then add onto end of list
	if (iRemainder) iTotalNumberOfItems += 10 - iRemainder;

	// allocate space for items
	WORLDITEM* const pTotalList = MALLOCNZ(WORLDITEM, iTotalNumberOfItems);

	INT32 iCurrentItem = 0;
	// place seen items in the world
	for (INT32 i = 0; i < iNumberSeenItems; ++i)
	{
		pTotalList[iCurrentItem++] = pSeenItemsList[i];
	}

	// now store the unseen item list
	for (INT32 i = 0; i < iNumberUnSeenItems; ++i)
	{
		pTotalList[iCurrentItem++] = pUnSeenItemsList[i];
	}

	RefreshItemPools(pTotalList, iTotalNumberOfItems);

	//Count the total number of visible items
	UINT32 uiTotalNumberOfVisibleItems = 0;
	for (INT32 i = 0; i < iNumberSeenItems; ++i)
	{
		uiTotalNumberOfVisibleItems += pSeenItemsList[i].o.ubNumberOfObjects;
	}

	//reset the visible item count in the sector info struct
	SetNumberOfVisibleWorldItemsInSectorStructureForSector(gWorldSectorX, gWorldSectorY, gbWorldSectorZ, uiTotalNumberOfVisibleItems);

	// clear out allocated space for total list
	MemFree(pTotalList);
}


static void ReSizeStashListByThisAmount(INT32 iNumberOfItems)
{
	const INT32 count    = iTotalNumberOfSlots;
	iTotalNumberOfSlots += iNumberOfItems;
	pInventoryPoolList = REALLOC(pInventoryPoolList, WORLDITEM, iTotalNumberOfSlots);
	memset(pInventoryPoolList + count, 0, sizeof(*pInventoryPoolList) * iNumberOfItems);
}


static void DestroyStash(void)
{
	// clear out stash
	MemFree( pInventoryPoolList );

	if (pUnSeenItems != NULL)
	{
		MemFree(pUnSeenItems);
		pUnSeenItems = NULL;
	}
	uiNumberOfUnSeenItems = 0;
}


static BOOLEAN GetObjFromInventoryStashSlot(OBJECTTYPE* pInventorySlot, OBJECTTYPE* pItemPtr);
static BOOLEAN RemoveObjectFromStashSlot(OBJECTTYPE* pInventorySlot, OBJECTTYPE* pItemPtr);


static void BeginInventoryPoolPtr(OBJECTTYPE* pInventorySlot)
{
	BOOLEAN fOk = FALSE;

	// If not null return
	if ( gpItemPointer != NULL )
	{
		return;
	}

	// if shift key get all

	if (_KeyDown( SHIFT ))
	{
		// Remove all from soldier's slot
		fOk = RemoveObjectFromStashSlot( pInventorySlot, &gItemPointer );
	}
	else
	{
		GetObjFromInventoryStashSlot( pInventorySlot, &gItemPointer );
		fOk = (gItemPointer.ubNumberOfObjects == 1);
	}

	if (fOk)
	{
		// Dirty interface
		fMapPanelDirty = TRUE;
		SetItemPointer(&gItemPointer, 0);
		SetMapCursorItem();

		if (fShowInventoryFlag)
		{
			SOLDIERTYPE* const s = GetSelectedInfoChar();
			if (s != NULL)
			{
				ReevaluateItemHatches(s, FALSE);
				fTeamPanelDirty = TRUE;
			}
		}
	}
}


// get this item out of the stash slot
static BOOLEAN GetObjFromInventoryStashSlot(OBJECTTYPE* pInventorySlot, OBJECTTYPE* pItemPtr)
{
	// item ptr
	if (!pItemPtr )
	{
		return( FALSE );
	}

	// if there are only one item in slot, just copy
	if (pInventorySlot->ubNumberOfObjects == 1)
	{
		*pItemPtr = *pInventorySlot;
		DeleteObj( pInventorySlot );
	}
	else
	{
		// take one item
		pItemPtr->usItem = pInventorySlot->usItem;

		// find first unempty slot
		pItemPtr->bStatus[0] = pInventorySlot->bStatus[0];
		pItemPtr->ubNumberOfObjects = 1;
		pItemPtr->ubWeight = CalculateObjectWeight( pItemPtr );
		RemoveObjFrom( pInventorySlot, 0 );
		pInventorySlot->ubWeight = CalculateObjectWeight( pInventorySlot );

	}

	return ( TRUE );
}


static BOOLEAN RemoveObjectFromStashSlot(OBJECTTYPE* pInventorySlot, OBJECTTYPE* pItemPtr)
{
	if (pInventorySlot -> ubNumberOfObjects == 0)
	{
		return( FALSE );
	}
	else
	{
		*pItemPtr = *pInventorySlot;
		DeleteObj( pInventorySlot );
		return( TRUE );
	}
}


static BOOLEAN PlaceObjectInInventoryStash(OBJECTTYPE* pInventorySlot, OBJECTTYPE* pItemPtr)
{
	UINT8 ubNumberToDrop, ubSlotLimit, ubLoop;

	// if there is something there, swap it, if they are of the same type and stackable then add to the count

	ubSlotLimit = GCM->getItem(pItemPtr -> usItem)->getPerPocket();

	if (pInventorySlot->ubNumberOfObjects == 0)
	{
		// placement in an empty slot
		ubNumberToDrop = pItemPtr->ubNumberOfObjects;

		if (ubNumberToDrop > ubSlotLimit && ubSlotLimit != 0)
		{
			// drop as many as possible into pocket
			ubNumberToDrop = ubSlotLimit;
		}

		// could be wrong type of object for slot... need to check...
		// but assuming it isn't
		*pInventorySlot = *pItemPtr;

		if (ubNumberToDrop != pItemPtr->ubNumberOfObjects)
		{
			// in the InSlot copy, zero out all the objects we didn't drop
			for (ubLoop = ubNumberToDrop; ubLoop < pItemPtr->ubNumberOfObjects; ubLoop++)
			{
				pInventorySlot->bStatus[ubLoop] = 0;
			}
		}
		pInventorySlot->ubNumberOfObjects = ubNumberToDrop;

		// remove a like number of objects from pObj
		RemoveObjs( pItemPtr, ubNumberToDrop );
	}
	else
	{
		// replacement/reloading/merging/stacking

		// placement in an empty slot
		ubNumberToDrop = pItemPtr->ubNumberOfObjects;

		if (pItemPtr->usItem == pInventorySlot->usItem)
		{
			if (pItemPtr->usItem == MONEY)
			{
				// always allow money to be combined!
				// average out the status values using a weighted average...
				pInventorySlot->bStatus[0] = (INT8) ( ( (UINT32)pInventorySlot->bMoneyStatus * pInventorySlot->uiMoneyAmount + (UINT32)pItemPtr->bMoneyStatus * pItemPtr->uiMoneyAmount )/ (pInventorySlot->uiMoneyAmount + pItemPtr->uiMoneyAmount) );
				pInventorySlot->uiMoneyAmount += pItemPtr->uiMoneyAmount;

				DeleteObj( pItemPtr );
			}
			else if (ubSlotLimit < 2)
			{
				// swapping
				SwapObjs( pItemPtr, pInventorySlot );
			}
			else
			{
				// stacking
				if( ubNumberToDrop > ubSlotLimit - pInventorySlot -> ubNumberOfObjects )
				{
					ubNumberToDrop = ubSlotLimit - pInventorySlot -> ubNumberOfObjects;
				}

				StackObjs( pItemPtr, pInventorySlot, ubNumberToDrop );
			}
		}
		else
		{

				SwapObjs( pItemPtr, pInventorySlot );
		}
	}
	return( TRUE );
}


void AutoPlaceObjectInInventoryStash(OBJECTTYPE* pItemPtr)
{
	UINT8 ubNumberToDrop, ubSlotLimit, ubLoop;
	OBJECTTYPE *pInventorySlot;


	// if there is something there, swap it, if they are of the same type and stackable then add to the count
	pInventorySlot =  &( pInventoryPoolList[ iTotalNumberOfSlots ].o );

	// placement in an empty slot
	ubNumberToDrop = pItemPtr->ubNumberOfObjects;

	ubSlotLimit = ItemSlotLimit( pItemPtr->usItem, BIGPOCK1POS );

	if (ubNumberToDrop > ubSlotLimit && ubSlotLimit != 0)
	{
		// drop as many as possible into pocket
		ubNumberToDrop = ubSlotLimit;
	}

	// could be wrong type of object for slot... need to check...
	// but assuming it isn't
	*pInventorySlot = *pItemPtr;

	if (ubNumberToDrop != pItemPtr->ubNumberOfObjects)
	{
		// in the InSlot copy, zero out all the objects we didn't drop
		for (ubLoop = ubNumberToDrop; ubLoop < pItemPtr->ubNumberOfObjects; ubLoop++)
		{
			pInventorySlot->bStatus[ubLoop] = 0;
		}
	}
	pInventorySlot->ubNumberOfObjects = ubNumberToDrop;

	// remove a like number of objects from pObj
	RemoveObjs( pItemPtr, ubNumberToDrop );
}


static void MapInventoryPoolNextBtn(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		InventoryNextPage();
	}
}


static void MapInventoryPoolPrevBtn(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		InventoryPrevPage();
	}
}


static void MapInventoryPoolDoneBtn(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fShowMapInventoryPool = FALSE;
	}
}


static void DisplayPagesForMapInventoryPool(void)
{
	// get the current and last pages and display them
	wchar_t sString[ 32 ];
	INT16 sX, sY;

	SetFontAttributes(COMPFONT, 183);
	SetFontDestBuffer(guiSAVEBUFFER);

	// grab current and last pages
	swprintf( sString, lengthof(sString), L"%d / %d", iCurrentInventoryPoolPage + 1, iLastInventoryPoolPage + 1 );

	// grab centered coords
	const SGPBox* const box = &g_sector_inv_page_box;
	FindFontCenterCoordinates(STD_SCREEN_X + box->x, STD_SCREEN_Y + box->y, box->w, box->h, sString, COMPFONT, &sX, &sY);
	MPrint(sX, sY, sString);

	SetFontDestBuffer(FRAME_BUFFER);
}


static INT32 GetTotalNumberOfItemsInSectorStash(void)
{
	INT32 iCounter, iCount = 0;

	// run through list of items and find out how many are there
	for( iCounter = 0; iCounter < iTotalNumberOfSlots; iCounter++ )
	{
		if( pInventoryPoolList[ iCounter].o.ubNumberOfObjects > 0 )
		{
			iCount += pInventoryPoolList[ iCounter].o.ubNumberOfObjects;
		}
	}

	return iCount;
}


// get total number of items in sector
static INT32 GetTotalNumberOfItems(void)
{
	INT32 iCounter, iCount = 0;

	// run through list of items and find out how many are there
	for( iCounter = 0; iCounter < iTotalNumberOfSlots; iCounter++ )
	{
		if( pInventoryPoolList[ iCounter].o.ubNumberOfObjects > 0 )
		{
			iCount++;
		}
	}

	return iCount;
}


static void DrawNumberOfIventoryPoolItems(void)
{
	INT32 iNumberOfItems = 0;
	wchar_t sString[ 32 ];
	INT16 sX, sY;


	iNumberOfItems = GetTotalNumberOfItemsInSectorStash( );

	// get number of items
	swprintf( sString, lengthof(sString), L"%d", iNumberOfItems );

	SetFontAttributes(COMPFONT, 183);
	SetFontDestBuffer(guiSAVEBUFFER);

	// grab centered coords
	const SGPBox* const box = &g_sector_inv_count_box;
	FindFontCenterCoordinates(STD_SCREEN_X + box->x, STD_SCREEN_Y + box->y, box->w, box->h, sString, COMPFONT, &sX, &sY);
	MPrint(sX, sY, sString);

	SetFontDestBuffer(FRAME_BUFFER);
}


static void CreateMapInventoryPoolDoneButton(void)
{
	// create done button
	guiMapInvenButton[2] = QuickCreateButtonImg(INTERFACEDIR "/done_button.sti", 0, 1, STD_SCREEN_X + 587, STD_SCREEN_Y + 333, MSYS_PRIORITY_HIGHEST, MapInventoryPoolDoneBtn);
}


static void DestroyInventoryPoolDoneButton(void)
{
	// destroy ddone button
	RemoveButton( guiMapInvenButton[ 2 ] );
}


static void DisplayCurrentSector(void)
{
	// grab current sector being displayed
	wchar_t sString[ 32 ];
	INT16 sX, sY;


	swprintf( sString, lengthof(sString), L"%ls%ls%ls", pMapVertIndex[ sSelMapY ], pMapHortIndex[ sSelMapX ], pMapDepthIndex[ iCurrentMapSectorZ ] );

	SetFontAttributes(COMPFONT, 183);
	SetFontDestBuffer(guiSAVEBUFFER);

	// grab centered coords
	const SGPBox* const box = &g_sector_inv_loc_box;
	FindFontCenterCoordinates(STD_SCREEN_X + box->x, STD_SCREEN_Y + box->y, box->w, box->h, sString, COMPFONT, &sX, &sY);
	MPrint(sX, sY, sString);

	SetFontDestBuffer(FRAME_BUFFER);
}


static void CheckAndUnDateSlotAllocation(void)
{
	// will check number of available slots, if less than half a page, allocate a new page
	INT32 iNumberOfTakenSlots = 0;

	// get number of taken slots
	iNumberOfTakenSlots = GetTotalNumberOfItems( );

	if( ( iTotalNumberOfSlots - iNumberOfTakenSlots ) < 2 )
	{
		// not enough space
		// need to make more space
		ReSizeStashListByThisAmount( MAP_INVENTORY_POOL_SLOT_COUNT );
	}

	iLastInventoryPoolPage = ( ( iTotalNumberOfSlots  - 1 ) / MAP_INVENTORY_POOL_SLOT_COUNT );
}


static void DrawTextOnSectorInventory(void);


static void DrawTextOnMapInventoryBackground(void)
{
//	wchar_t sString[ 64 ];
	UINT16 usStringHeight;

	SetFontDestBuffer(guiSAVEBUFFER);

	int xPos = STD_SCREEN_X + 268;
	int yPos = STD_SCREEN_Y + 342;

	//Calculate the height of the string, as it needs to be vertically centered.
	usStringHeight = DisplayWrappedString(xPos, yPos, 53, 1, MAP_IVEN_FONT, FONT_BEIGE, pMapInventoryStrings[0], FONT_BLACK, RIGHT_JUSTIFIED | DONT_DISPLAY_TEXT);
	DisplayWrappedString(xPos, yPos - (usStringHeight / 2), 53, 1, MAP_IVEN_FONT, FONT_BEIGE, pMapInventoryStrings[0], FONT_BLACK, RIGHT_JUSTIFIED);

	xPos = STD_SCREEN_X + 369;

	//Calculate the height of the string, as it needs to be vertically centered.
	usStringHeight = DisplayWrappedString(xPos, yPos, 65, 1, MAP_IVEN_FONT, FONT_BEIGE, pMapInventoryStrings[1], FONT_BLACK, RIGHT_JUSTIFIED | DONT_DISPLAY_TEXT);
	DisplayWrappedString( xPos, yPos - (usStringHeight / 2), 65, 1, MAP_IVEN_FONT, FONT_BEIGE, pMapInventoryStrings[1], FONT_BLACK, RIGHT_JUSTIFIED);

	DrawTextOnSectorInventory( );

	SetFontDestBuffer(FRAME_BUFFER);
}


void HandleButtonStatesWhileMapInventoryActive( void )
{
	// are we even showing the amp inventory pool graphic?
	if (!fShowMapInventoryPool) return;

	// first page, can't go back any
	EnableButton(guiMapInvenButton[1], iCurrentInventoryPoolPage != 0);
	// last page, go no further
	EnableButton(guiMapInvenButton[0], iCurrentInventoryPoolPage != iLastInventoryPoolPage);
	// item picked up ..disable button
	EnableButton(guiMapInvenButton[2], !fMapInventoryItem);
}


static void DrawTextOnSectorInventory(void)
{
	SetFontDestBuffer(guiSAVEBUFFER);
	SetFontAttributes(FONT14ARIAL, FONT_WHITE);

	INT16 x;
	INT16 y;
	const SGPBox*  const box   = &g_sector_inv_title_box;
	const wchar_t* const title = zMarksMapScreenText[11];
	FindFontCenterCoordinates(STD_SCREEN_X + box->x, STD_SCREEN_Y + box->y, box->w, box->h, title, FONT14ARIAL, &x, &y);
	MPrint(x, y, title);

	SetFontDestBuffer(FRAME_BUFFER);
}


void HandleFlashForHighLightedItem( void )
{
	INT32 iCurrentTime = 0;
	INT32 iDifference = 0;


	// if there is an invalid item, reset
	if( iCurrentlyHighLightedItem == -1 )
	{
		fFlashHighLightInventoryItemOnradarMap = FALSE;
		giFlashHighlightedItemBaseTime = 0;
	}

	// get the current time
	iCurrentTime = GetJA2Clock();

	// if there basetime is uninit
	if( giFlashHighlightedItemBaseTime == 0 )
	{
		giFlashHighlightedItemBaseTime = iCurrentTime;
	}


	iDifference = iCurrentTime - giFlashHighlightedItemBaseTime;

	if( iDifference > DELAY_FOR_HIGHLIGHT_ITEM_FLASH )
	{
		// reset timer
		giFlashHighlightedItemBaseTime = iCurrentTime;

		// flip flag
		fFlashHighLightInventoryItemOnradarMap = !fFlashHighLightInventoryItemOnradarMap;

		// re render radar map
		RenderRadarScreen( );

	}
}


static void ResetMapSectorInventoryPoolHighLights();


static void HandleMouseInCompatableItemForMapSectorInventory(INT32 iCurrentSlot)
{
	SOLDIERTYPE *pSoldier = NULL;
	static BOOLEAN fItemWasHighLighted = FALSE;

	if( iCurrentSlot == -1 )
	{
		giCompatibleItemBaseTime = 0;
	}

	if (fChangedInventorySlots)
	{
		giCompatibleItemBaseTime = 0;
		fChangedInventorySlots = FALSE;
	}

	// reset the base time to the current game clock
	if( giCompatibleItemBaseTime == 0 )
	{
		giCompatibleItemBaseTime = GetJA2Clock( );

		if (fItemWasHighLighted)
		{
			fTeamPanelDirty = TRUE;
			fMapPanelDirty = TRUE;
			fItemWasHighLighted = FALSE;
		}
	}

	ResetCompatibleItemArray( );
	ResetMapSectorInventoryPoolHighLights( );

	if( iCurrentSlot == -1 )
	{
		return;
	}

	// given this slot value, check if anything in the displayed sector inventory or on the mercs inventory is compatable
	if( fShowInventoryFlag )
	{
		// check if any compatable items in the soldier inventory matches with this item
		if( gfCheckForCursorOverMapSectorInventoryItem )
		{
			const SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
			if( pSoldier )
			{
				if( HandleCompatibleAmmoUIForMapScreen( pSoldier, iCurrentSlot + ( iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT ), TRUE, FALSE ) )
				{
					if( GetJA2Clock( ) - giCompatibleItemBaseTime > 100 )
					{
						if (!fItemWasHighLighted)
						{
							fTeamPanelDirty = TRUE;
							fItemWasHighLighted = TRUE;
						}
					}
				}
			}
		}
		else
		{
			giCompatibleItemBaseTime = 0;
		}
	}


	// now handle for the sector inventory
	if( fShowMapInventoryPool )
	{
		// check if any compatable items in the soldier inventory matches with this item
		if( gfCheckForCursorOverMapSectorInventoryItem )
		{
			if( HandleCompatibleAmmoUIForMapInventory( pSoldier, iCurrentSlot, ( iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT ) , TRUE, FALSE ) )
			{
				if( GetJA2Clock( ) - giCompatibleItemBaseTime > 100 )
				{
					if (!fItemWasHighLighted)
					{
						fItemWasHighLighted = TRUE;
						fMapPanelDirty = TRUE;
					}
				}
			}
		}
		else
		{
			giCompatibleItemBaseTime = 0;
		}
	}
}


static void ResetMapSectorInventoryPoolHighLights()
{ // Reset the highlight list for the map sector inventory.
	FOR_EACH(BOOLEAN, i, fMapInventoryItemCompatable) *i = FALSE;
}


static void HandleMapSectorInventory(void)
{
	// handle mouse in compatable item map sectors inventory
	HandleMouseInCompatableItemForMapSectorInventory( iCurrentlyHighLightedItem );
}


//CJC look here to add/remove checks for the sector inventory
BOOLEAN IsMapScreenWorldItemVisibleInMapInventory(const WORLDITEM* const pWorldItem)
{
	if (pWorldItem->fExists             &&
			pWorldItem->bVisible == VISIBLE &&
			pWorldItem->o.usItem != SWITCH &&
			pWorldItem->o.usItem != ACTION_ITEM &&
			pWorldItem->o.bTrap <= 0 )
	{
		return( TRUE );
	}

	return( FALSE );
}


//Check to see if any of the items in the list have a gridno of NOWHERE and the entry point flag NOT set
static void CheckGridNoOfItemsInMapScreenMapInventory(void)
{
	INT32  iCnt;
	UINT32 uiNumFlagsNotSet = 0;
	INT32  iTotalNumberItems = GetTotalNumberOfItems( );


	for( iCnt=0; iCnt<iTotalNumberItems; iCnt++)
	{
		if( pInventoryPoolList[ iCnt ].sGridNo == NOWHERE && !( pInventoryPoolList[ iCnt ].usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT ) )
		{
			//set the flag
			pInventoryPoolList[ iCnt ].usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;

			//count the number
			uiNumFlagsNotSet++;
		}
	}


	//loop through all the UNSEEN items
	for( iCnt=0; iCnt<(INT32)uiNumberOfUnSeenItems; iCnt++)
	{
		if( pUnSeenItems[ iCnt ].sGridNo == NOWHERE && !( pUnSeenItems[ iCnt ].usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT ) )
		{
			//set the flag
			pUnSeenItems[ iCnt ].usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;

			//count the number
			uiNumFlagsNotSet++;
		}
	}

	if( uiNumFlagsNotSet > 0 )
	{
		SLOGD(DEBUG_TAG_SMAP, "Item with invalid gridno doesnt have flag set: %d", uiNumFlagsNotSet );
	}
}


static INT32 MapScreenSectorInventoryCompare(const void* pNum1, const void* pNum2);


static void SortSectorInventory(WORLDITEM* pInventory, UINT32 uiSizeOfArray)
{
	qsort(pInventory, uiSizeOfArray, sizeof(WORLDITEM), MapScreenSectorInventoryCompare);
}


static INT32 MapScreenSectorInventoryCompare(const void* pNum1, const void* pNum2)
{
	WORLDITEM *pFirst = (WORLDITEM *)pNum1;
	WORLDITEM *pSecond = (WORLDITEM *)pNum2;
	UINT16	usItem1Index;
	UINT16	usItem2Index;
	UINT8		ubItem1Quality;
	UINT8		ubItem2Quality;

	usItem1Index = pFirst->o.usItem;
	usItem2Index = pSecond->o.usItem;

	ubItem1Quality = pFirst->o.bStatus[ 0 ];
	ubItem2Quality = pSecond->o.bStatus[ 0 ];

	return( CompareItemsForSorting( usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality ) );
}


static BOOLEAN CanPlayerUseSectorInventory(void)
{
	INT16 x;
	INT16 y;
	INT16 z;
	return
		!GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(&x, &y, &z) ||
		sSelMapX           != x ||
		sSelMapY           != y ||
		iCurrentMapSectorZ != z;
}
