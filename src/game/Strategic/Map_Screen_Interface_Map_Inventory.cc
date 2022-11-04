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
#include "ShopKeeper_Interface.h"
#include "ArmsDealerInvInit.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <climits>
#include <vector>

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
std::vector<WORLDITEM> pInventoryPoolList;

// current page of inventory
INT32 iCurrentInventoryPoolPage = 0;
static INT32 iLastInventoryPoolPage = 0;

INT16 sObjectSourceGridNo = 0;

// the inventory slots
static MOUSE_REGION MapInventoryPoolSlots[MAP_INVENTORY_POOL_SLOT_COUNT];
static MOUSE_REGION MapInventoryPoolMask;
BOOLEAN fMapInventoryItemCompatable[ MAP_INVENTORY_POOL_SLOT_COUNT ];
static BOOLEAN      fChangedInventorySlots = FALSE;

// the unseen items list...have to save this
static std::vector<WORLDITEM> pUnSeenItems;

UINT32 guiFlashHighlightedItemBaseTime = 0;
UINT32 guiCompatibleItemBaseTime = 0;

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
	const WORLDITEM& item = pInventoryPoolList[iCurrentSlot + iFirstSlotOnPage];

	// check if anything there
	if (item.o.ubNumberOfObjects == 0) return FALSE;

	const SGPBox* const slot_box = &g_sector_inv_slot_box;
	const INT32 dx = STD_SCREEN_X + slot_box->x + slot_box->w * (iCurrentSlot / MAP_INV_SLOT_ROWS);
	const INT32 dy = STD_SCREEN_Y + slot_box->y + slot_box->h * (iCurrentSlot % MAP_INV_SLOT_ROWS);

	SetFontDestBuffer(guiSAVEBUFFER);
	const SGPBox* const item_box = &g_sector_inv_item_box;
	const UINT16        outline  = fMapInventoryItemCompatable[iCurrentSlot] ? Get16BPPColor(FROMRGB(255, 255, 255)) : SGP_TRANSPARENT;
	INVRenderItem(guiSAVEBUFFER, NULL, item.o, dx + item_box->x, dy + item_box->y, item_box->w, item_box->h, DIRTYLEVEL2, 0, outline);

	// draw bar for condition
	const UINT16 col0 = Get16BPPColor(DESC_STATUS_BAR);
	const UINT16 col1 = Get16BPPColor(DESC_STATUS_BAR_SHADOW);
	const SGPBox* const bar_box = &g_sector_inv_bar_box;
	DrawItemUIBarEx(item.o, 0, dx + bar_box->x, dy + bar_box->y + bar_box->h - 1, bar_box->h, col0, col1, guiSAVEBUFFER);

	// if the item is not reachable, or if the selected merc is not in the current sector
	const SOLDIERTYPE* const s = GetSelectedInfoChar();
	if (!(item.usFlags & WORLD_ITEM_REACHABLE) ||
			s           == NULL     ||
			s->sSector.x != sSelMap.x ||
			s->sSector.y != sSelMap.y ||
			s->sSector.z != iCurrentMapSectorZ)
	{
		//Shade the item
		DrawHatchOnInventory(guiSAVEBUFFER, dx + item_box->x, dy + item_box->y, item_box->w, item_box->h);
	}

	// the name
	const SGPBox* const name_box = &g_sector_inv_name_box;
	auto sString = ReduceStringLength(GCM->getItem(item.o.usItem)->getShortName(), name_box->w, MAP_IVEN_FONT);

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
	ST::string pStr;
	INT32 iCounter = 0;
	INT32 iFirstSlotOnPage = ( iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT );


	// run through list of items in slots and update help text for mouse regions
	for( iCounter = 0; iCounter < MAP_INVENTORY_POOL_SLOT_COUNT; iCounter++ )
	{
		ST::string help;
		OBJECTTYPE const& o    = pInventoryPoolList[iCounter + iFirstSlotOnPage].o;
		if  (o.ubNumberOfObjects > 0)
		{
			pStr = GetHelpTextForItem(o);
			help = pStr;
		}
		MapInventoryPoolSlots[iCounter].SetFastHelpText(help);
	}
}


static void BuildStashForSelectedSector(const SGPSector& sector);
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
	SGPSector sector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ);
	if (fShowMapInventoryPool && !fCreated)
	{
		if (gWorldSector == sector)
		{
			// handle all reachable before save
			HandleAllReachAbleItemsInTheSector(gWorldSector);
		}

		// destroy buttons for map border
		DeleteMapBorderButtons( );

		fCreated = TRUE;

		// also create the inventory slot
		CreateMapInventoryPoolSlots( );

		// create buttons
		CreateMapInventoryButtons( );

		// build stash
		BuildStashForSelectedSector(sector);

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


static size_t GetTotalNumberOfItems(void);
static void ReBuildWorldItemStashForLoadedSector(const std::vector<WORLDITEM>& pSeenItemsList, const std::vector<WORLDITEM>& pUnSeenItemsList);


static void SaveSeenAndUnseenItems(void)
{
	// if there are seen items, build a temp world items list of them and save them
	std::vector<WORLDITEM> pSeenItemsList;
	for (WORLDITEM& pi : pInventoryPoolList)
	{
		if (pi.o.ubNumberOfObjects == 0) continue;

		WORLDITEM si = pi;
		if (si.sGridNo == 0)
		{
			// Use gridno of predecessor, if there is one
			if (pSeenItemsList.size() != 0)
			{
				// borrow from predecessor
				si.sGridNo = pSeenItemsList.back().sGridNo;
			}
			else
			{
				// get entry grid location
			}
		}
		si.fExists = TRUE;
		si.bVisible = TRUE;
		pSeenItemsList.push_back(si);
	}

	// if this is the loaded sector handle here
	SGPSector sector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ);
	if (gWorldSector == sector)
	{
		ReBuildWorldItemStashForLoadedSector(pSeenItemsList, pUnSeenItems);
	}
	else
	{
		// now copy over unseen and seen
		SaveWorldItemsToTempItemFile(sector, pUnSeenItems);
		AddWorldItemsToUnLoadedSector(sector, pSeenItemsList);
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
static void MapInvenPoolScreenMaskCallbackSecondary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	fShowMapInventoryPool = FALSE;
}

static void MapInvenPoolScreenMaskCallbackScroll(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		InventoryPrevPage();
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		InventoryNextPage();
	}
}


static void MapInvenPoolSlotsPrimary(MOUSE_REGION* pRegion, UINT32 iReason);
static void MapInvenPoolSlotsSecondary(MOUSE_REGION* pRegion, UINT32 iReason);
static void MapInvenPoolSlotsScroll(MOUSE_REGION* pRegion, UINT32 iReason);
static void MapInvenPoolSlotsMove(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateMapInventoryPoolSlots(void)
{
	{
		const SGPBox* const inv_box = &g_sector_inv_box;
		UINT16        const x       = STD_SCREEN_X + inv_box->x;
		UINT16        const y       = STD_SCREEN_Y + inv_box->y;
		UINT16        const w       = inv_box->w;
		UINT16        const h       = inv_box->h;
		MSYS_DefineRegion(&MapInventoryPoolMask, x, y, x + w - 1, y + h - 1, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MouseCallbackPrimarySecondary(MSYS_NO_CALLBACK, MapInvenPoolScreenMaskCallbackSecondary, MapInvenPoolScreenMaskCallbackScroll));
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
		MSYS_DefineRegion(r, x, y, x + w - 1, y + h - 1, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MapInvenPoolSlotsMove, MouseCallbackPrimarySecondary(MapInvenPoolSlotsPrimary, MapInvenPoolSlotsSecondary, MapInvenPoolSlotsScroll));
		MSYS_SetRegionUserData(r, 0, i);
	}
}


static void DestroyMapInventoryPoolSlots()
{
	FOR_EACH(MOUSE_REGION, i, MapInventoryPoolSlots) MSYS_RemoveRegion(&*i);
	MSYS_RemoveRegion(&MapInventoryPoolMask);
}


static void MapInvenPoolSlotsMove(MOUSE_REGION* pRegion, UINT32 iReason)
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


static void MapInvenPoolSlotsPrimary(MOUSE_REGION* const pRegion, const UINT32 iReason)
{
	// check if item in cursor, if so, then swap, and no item in curor, pick up, if item in cursor but not box, put in box
	INT32      const slot_idx = MSYS_GetRegionUserData(pRegion, 0);
	WORLDITEM& slot = pInventoryPoolList[iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT + slot_idx];

	// Return if empty
	if (gpItemPointer == NULL && slot.o.usItem == NOTHING) return;

	// is this item reachable
	if (slot.o.usItem != NOTHING && !(slot.usFlags & WORLD_ITEM_REACHABLE))
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
	if (s->sSector.x != sSelMap.x           ||
			s->sSector.y != sSelMap.y           ||
			s->sSector.z != iCurrentMapSectorZ ||
			s->fBetweenSectors)
	{
		ST::string msg = (gpItemPointer == NULL ? pMapInventoryErrorString[1] : pMapInventoryErrorString[4]);
		ST::string buf = st_format_printf(msg, s->name);
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, buf, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		return;
	}

	// If in battle inform player they will have to do this in tactical
	if (!CanPlayerUseSectorInventory())
	{
		ST::string msg = (gpItemPointer == NULL ? pMapInventoryErrorString[2] : pMapInventoryErrorString[3]);
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, msg, MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		return;
	}

	// If we do not have an item in hand, start moving it
	if (gpItemPointer == NULL)
	{
		sObjectSourceGridNo = slot.sGridNo;
		BeginInventoryPoolPtr(&slot.o);
	}
	else
	{
		const INT32 iOldNumberOfObjects = slot.o.ubNumberOfObjects;

		// Else, try to place here
		if (PlaceObjectInInventoryStash(&slot.o, gpItemPointer))
		{
			// nothing here before, then place here
			if (iOldNumberOfObjects == 0)
			{
				slot.sGridNo                  = sObjectSourceGridNo;
				slot.ubLevel                  = s->bLevel;
				slot.usFlags                  = 0;
				slot.bRenderZHeightAboveLevel = 0;

				if (sObjectSourceGridNo == NOWHERE)
				{
					slot.usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;
				}
			}

			slot.usFlags |= WORLD_ITEM_REACHABLE;

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

static void MapInvenPoolSlotsSecondary(MOUSE_REGION* const pRegion, const UINT32 iReason)
{
	if (gpItemPointer == NULL) fShowMapInventoryPool = FALSE;
}

static void MapInvenPoolSlotsScroll(MOUSE_REGION* const pRegion, const UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		InventoryPrevPage();
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		InventoryNextPage();
	}
}


static void MapInventoryPoolPrevBtn(GUI_BUTTON* btn, UINT32 reason);
static void MapInventoryPoolNextBtn(GUI_BUTTON* btn, UINT32 reason);


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
static void SortSectorInventory(WORLDITEM* pInventory, size_t sizeOfArray);


static void BuildStashForSelectedSector(const SGPSector& sector)
{
	std::vector<WORLDITEM> temp;
	std::vector<WORLDITEM>* items = nullptr;
	if (sector == gWorldSector)
	{
		items = &gWorldItems;
	}
	else
	{
		temp = LoadWorldItemsFromTempItemFile(sector);
		items = &temp;
	}

	pInventoryPoolList.clear();
	pUnSeenItems.clear();

	for (const WORLDITEM& wi : *items)
	{
		if (!wi.fExists) continue;
		if (IsMapScreenWorldItemVisibleInMapInventory(wi))
		{
			pInventoryPoolList.push_back(wi);
		}
		else
		{
			pUnSeenItems.push_back(wi);
		}
	}

	size_t visible_slots = pInventoryPoolList.size();
	size_t empty_slots = MAP_INVENTORY_POOL_SLOT_COUNT - visible_slots % MAP_INVENTORY_POOL_SLOT_COUNT;
	pInventoryPoolList.resize(visible_slots + empty_slots, WORLDITEM{});
	iLastInventoryPoolPage  = static_cast<INT32>((pInventoryPoolList.size() - 1) / MAP_INVENTORY_POOL_SLOT_COUNT);

	CheckGridNoOfItemsInMapScreenMapInventory();
	SortSectorInventory(pInventoryPoolList.data(), visible_slots);
}


static void ReBuildWorldItemStashForLoadedSector(const std::vector<WORLDITEM>& pSeenItemsList, const std::vector<WORLDITEM>& pUnSeenItemsList)
{
	TrashWorldItems();

	std::vector<WORLDITEM> pTotalList;
	pTotalList.insert(pTotalList.end(), pSeenItemsList.begin(), pSeenItemsList.end());
	pTotalList.insert(pTotalList.end(), pUnSeenItemsList.begin(), pUnSeenItemsList.end());

	size_t remainder = pTotalList.size() % 10;
	if (remainder)
	{
		pTotalList.insert(pTotalList.end(), 10 - remainder, WORLDITEM{});
	}

	RefreshItemPools(pTotalList);

	//Count the total number of visible items
	UINT32 uiTotalNumberOfVisibleItems = 0;
	for (const WORLDITEM& si : pSeenItemsList)
	{
		uiTotalNumberOfVisibleItems += si.o.ubNumberOfObjects;
	}

	//reset the visible item count in the sector info struct
	SetNumberOfVisibleWorldItemsInSectorStructureForSector(gWorldSector, uiTotalNumberOfVisibleItems);
}


static void DestroyStash(void)
{
	// clear out stash
	pInventoryPoolList.clear();
	pUnSeenItems.clear();
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
	pInventorySlot =  &( pInventoryPoolList[ pInventoryPoolList.size() ].o );// FIXME out of bounds access

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


static void MapInventoryPoolNextBtn(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		InventoryNextPage();
	}
}


static void MapInventoryPoolPrevBtn(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		InventoryPrevPage();
	}
}


static void MapInventoryPoolDoneBtn(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		fShowMapInventoryPool = FALSE;
	}
}


static void DisplayPagesForMapInventoryPool(void)
{
	// get the current and last pages and display them
	ST::string sString;
	INT16 sX, sY;

	SetFontAttributes(COMPFONT, 183);
	SetFontDestBuffer(guiSAVEBUFFER);

	// grab current and last pages
	sString = ST::format("{} / {}", iCurrentInventoryPoolPage + 1, iLastInventoryPoolPage + 1);

	// grab centered coords
	const SGPBox* const box = &g_sector_inv_page_box;
	FindFontCenterCoordinates(STD_SCREEN_X + box->x, STD_SCREEN_Y + box->y, box->w, box->h, sString, COMPFONT, &sX, &sY);
	MPrint(sX, sY, sString);

	SetFontDestBuffer(FRAME_BUFFER);
}


static size_t GetTotalNumberOfItemsInSectorStash(void)
{
	size_t numObjects = 0;

	// run through list of items and find out how many are there
	for (WORLDITEM& wi : pInventoryPoolList)
	{
		if (wi.o.ubNumberOfObjects > 0)
		{
			numObjects += wi.o.ubNumberOfObjects;
		}
	}

	return numObjects;
}


// get total number of items in sector
static size_t GetTotalNumberOfItems(void)
{
	size_t numSlots = 0;

	// run through list of items and find out how many are there
	for (WORLDITEM& wi : pInventoryPoolList)
	{
		if (wi.o.ubNumberOfObjects > 0)
		{
			numSlots++;
		}
	}

	return numSlots;
}


static void DrawNumberOfIventoryPoolItems(void)
{
	size_t numObjects = 0;
	ST::string sString;
	INT16 sX, sY;


	numObjects = GetTotalNumberOfItemsInSectorStash( );

	// get number of items
	Assert(numObjects <= INT_MAX);
	sString = ST::format("{}", numObjects);

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
	ST::string sString;
	INT16 sX, sY;


	sString = ST::format("{}{}{}", pMapVertIndex[ sSelMap.y ], pMapHortIndex[ sSelMap.x ], pMapDepthIndex[ iCurrentMapSectorZ ]);

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
	size_t numTakenSlots = GetTotalNumberOfItems();

	if ((pInventoryPoolList.size() - numTakenSlots) < 2)
	{
		// not enough space
		// need to make more space
		pInventoryPoolList.insert(pInventoryPoolList.end(), MAP_INVENTORY_POOL_SLOT_COUNT, WORLDITEM{});
	}

	iLastInventoryPoolPage = ( ( static_cast<INT32>(pInventoryPoolList.size()) - 1 ) / MAP_INVENTORY_POOL_SLOT_COUNT );
}


static void DrawTextOnSectorInventory(void);


static void DrawTextOnMapInventoryBackground(void)
{
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
	ST::string title = zMarksMapScreenText[11];
	FindFontCenterCoordinates(STD_SCREEN_X + box->x, STD_SCREEN_Y + box->y, box->w, box->h, title, FONT14ARIAL, &x, &y);
	MPrint(x, y, title);

	SetFontDestBuffer(FRAME_BUFFER);
}


void HandleFlashForHighLightedItem( void )
{
	UINT32 uiCurrentTime = 0;
	INT32 iDifference = 0;


	// if there is an invalid item, reset
	if( iCurrentlyHighLightedItem == -1 )
	{
		fFlashHighLightInventoryItemOnradarMap = FALSE;
		guiFlashHighlightedItemBaseTime = 0;
	}

	// get the current time
	uiCurrentTime = GetJA2Clock();

	// if there basetime is uninit
	if( guiFlashHighlightedItemBaseTime == 0 )
	{
		guiFlashHighlightedItemBaseTime = uiCurrentTime;
	}


	iDifference = uiCurrentTime - guiFlashHighlightedItemBaseTime;

	if( iDifference > DELAY_FOR_HIGHLIGHT_ITEM_FLASH )
	{
		// reset timer
		guiFlashHighlightedItemBaseTime = uiCurrentTime;

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
		guiCompatibleItemBaseTime = 0;
	}

	if (fChangedInventorySlots)
	{
		guiCompatibleItemBaseTime = 0;
		fChangedInventorySlots = FALSE;
	}

	// reset the base time to the current game clock
	if( guiCompatibleItemBaseTime == 0 )
	{
		guiCompatibleItemBaseTime = GetJA2Clock( );

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
					if( GetJA2Clock( ) - guiCompatibleItemBaseTime > 100 )
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
			guiCompatibleItemBaseTime = 0;
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
				if( GetJA2Clock( ) - guiCompatibleItemBaseTime > 100 )
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
			guiCompatibleItemBaseTime = 0;
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
BOOLEAN IsMapScreenWorldItemVisibleInMapInventory(const WORLDITEM& wi)
{
	if (wi.fExists             &&
			wi.bVisible == VISIBLE &&
			wi.o.usItem != SWITCH &&
			wi.o.usItem != ACTION_ITEM &&
			wi.o.bTrap <= 0 )
	{
		return( TRUE );
	}

	return( FALSE );
}


//Check to see if any of the items in the list have a gridno of NOWHERE and the entry point flag NOT set
static void CheckGridNoOfItemsInMapScreenMapInventory(void)
{
	size_t uiNumFlagsNotSet = 0;
	size_t numTakenSlots = GetTotalNumberOfItems();


	for (size_t iCnt = 0; iCnt < numTakenSlots; iCnt++)// FIXME this only works properly when the taken slots are continuous
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
	for (size_t iCnt = 0; iCnt < pUnSeenItems.size(); iCnt++)
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
		SLOGD("Item with invalid gridno doesnt have flag set: {}", uiNumFlagsNotSet);
	}
}


static INT32 MapScreenSectorInventoryCompare(const void* pNum1, const void* pNum2);


static void SortSectorInventory(WORLDITEM* pInventory, size_t sizeOfArray)
{
	qsort(pInventory, sizeOfArray, sizeof(WORLDITEM), MapScreenSectorInventoryCompare);
}


static INT32 MapScreenSectorInventoryCompare(const void* pNum1, const void* pNum2)
{
	WORLDITEM *pFirst = (WORLDITEM *)pNum1;
	WORLDITEM *pSecond = (WORLDITEM *)pNum2;
	ItemId	usItem1Index;
	ItemId	usItem2Index;
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
	SGPSector sector;
	return
		!GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(sector) ||
		sSelMap.x           != sector.x ||
		sSelMap.y           != sector.y ||
		iCurrentMapSectorZ != sector.z;
}
