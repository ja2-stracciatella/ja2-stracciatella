#include "Auto_Resolve.h"
#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Handle_Items.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "ItemModel.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "MessageBoxScreen.h"
#include "Object_Cache.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "SysUtil.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Map.h"
#include "Assignments.h"
#include "Items.h"
#include "Interface_Items.h"
#include "MagazineModel.h"
#include "Message.h"
#include "Overhead.h"
#include "SGPStrings.h"
#include "Interface_Utils.h"
#include "Text.h"
#include "Font_Control.h"
#include "StrategicMap.h"
#include "World_Items.h"
#include "Tactical_Save.h"
#include "Soldier_Control.h"
#include "English.h"
#include "MapScreen.h"
#include "Radar_Screen.h"
#include "Interface_Panels.h"
#include "WordWrap.h"
#include "Button_System.h"
#include "ScreenIDs.h"
#include "VSurface.h"
#include "ShopKeeper_Interface.h"
#include "ArmsDealerInvInit.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
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
static cache_key_t const guiMapInventoryPoolBackground{ INTERFACEDIR "/sector_inventory.sti" };

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


// remove background panel graphics for inventory
void RemoveInventoryPoolGraphic( void )
{
	RemoveVObject(guiMapInventoryPoolBackground);
}


static void CheckAndUnDateSlotAllocation(void);
static void DisplayCurrentSector(void);
static void DisplayPagesForMapInventoryPool(void);
static void DrawNumberOfInventoryPoolItems();
static void DrawTextOnMapInventoryBackground(void);
static void RenderItemsForCurrentPageOfInventoryPool(void);
static void UpdateHelpTextForInvnentoryStashSlots(void);

namespace {
// Print text horizontally and vertically centered inside box
// x and y are added to the box's x and y.
void MPrintCenteredInBox(int x, int y, ST::string const& text, SGPBox const& box)
{
	MPrint(x + box.x, y + box.y, text, HCenterVCenterAlign(box.w, box.h));
}
}

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
	DrawNumberOfInventoryPoolItems();

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
	MPrintCenteredInBox(dx, dy, sString, *name_box);
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
	auto const& sector{ sSelMap };
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
	auto const& sector{ sSelMap };
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
		RemoveObjFrom( pInventorySlot, 0 );
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
			if (GCM->getItem(pItemPtr->usItem)->isMoney())
			{
				// always allow money to be combined! this covers silver and gold ore too
				// status of money is always 100
				pInventorySlot->bMoneyStatus = 100;
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


// Rebuild the pool list from the given items, padding it back out to whole pages.
static void ReplaceMapInventoryPool(const std::vector<WORLDITEM>& items)
{
	pInventoryPoolList = items;

	size_t const visible_slots = pInventoryPoolList.size();
	size_t const empty_slots   = MAP_INVENTORY_POOL_SLOT_COUNT - visible_slots % MAP_INVENTORY_POOL_SLOT_COUNT;
	pInventoryPoolList.resize(visible_slots + empty_slots, WORLDITEM{});

	iLastInventoryPoolPage = static_cast<INT32>((pInventoryPoolList.size() - 1) / MAP_INVENTORY_POOL_SLOT_COUNT);
	if (iCurrentInventoryPoolPage > iLastInventoryPoolPage)
	{
		iCurrentInventoryPoolPage = iLastInventoryPoolPage;
	}
}


// How many objects of this item may share one sector inventory slot?
static UINT8 SectorInventoryStackLimit(const ItemModel* const item)
{
	return std::min<UINT8>(item->getPerPocket(), MAX_OBJECTS_PER_SLOT);
}


// Can the source slot be poured into the target slot without losing anything?
static BOOLEAN CanMergeSectorInventorySlots(const WORLDITEM& target, const WORLDITEM& source)
{
	if (target.o.usItem != source.o.usItem) return FALSE;
	if (target.ubLevel  != source.ubLevel)  return FALSE;

	// unreachable items must not become reachable by being piled onto a reachable stack, and vice versa
	if ((target.usFlags & WORLD_ITEM_REACHABLE) != (source.usFlags & WORLD_ITEM_REACHABLE)) return FALSE;

	// attachments and traps belong to a single object, stacking would drop them
	if (ItemHasAttachments(target.o) || ItemHasAttachments(source.o)) return FALSE;
	if (target.o.bTrap > 0 || source.o.bTrap > 0) return FALSE;

	// keys keep their lock id in the same bytes as the status array
	if (GCM->getItem(target.o.usItem)->isKey()) return FALSE;

	return TRUE;
}


// Guns keep their loaded rounds inside the gun object, where nothing can stack them and no other
// gun can be fed from them.  Take them out as a magazine of their own.
static BOOLEAN UnloadSectorInventoryGun(WORLDITEM& wi, std::vector<WORLDITEM>& extracted)
{
	OBJECTTYPE& gun = wi.o;

	// the ammo fields share their bytes with the status of the 2nd and 3rd object of a stack
	if (gun.ubNumberOfObjects != 1) return FALSE;

	if (!GCM->getItem(gun.usItem)->isGun()) return FALSE;
	if (gun.ubGunShotsLeft == 0)            return FALSE;

	const ItemModel* const ammo = GCM->getItem(gun.usGunAmmoItem, ItemSystem::nothrow);
	if (ammo == NULL || !ammo->isAmmo()) return FALSE;

	WORLDITEM mag           = wi;
	mag.o                   = OBJECTTYPE{};
	mag.o.usItem            = gun.usGunAmmoItem;
	mag.o.ubNumberOfObjects = 1;
	mag.o.ubShotsLeft[0]    = gun.ubGunShotsLeft;

	gun.ubGunShotsLeft = 0;
	gun.ubGunAmmoType  = 0;
	// usGunAmmoItem stays, the gun uses it to know what it was loaded with

	extracted.push_back(mag);
	return TRUE;
}


// An attachment hides inside the item it is bolted onto, so it is neither counted nor sorted and
// it keeps its host out of every stack.  Returns how many came off.
static UINT32 DetachSectorInventoryAttachments(WORLDITEM& wi, std::vector<WORLDITEM>& extracted)
{
	UINT32 uiDetached = 0;

	for (INT8 bPos = 0; bPos < MAX_ATTACHMENTS; ++bPos)
	{
		if (wi.o.usAttachItem[bPos] == NOTHING) continue;

		WORLDITEM detached = wi;
		detached.o         = OBJECTTYPE{};

		// fails for the attachments that are welded on, those simply stay where they are
		if (!RemoveAttachment(&wi.o, bPos, &detached.o)) continue;

		extracted.push_back(detached);
		++uiDetached;

		--bPos; // removing one moves the remaining attachments down a slot
	}

	return uiDetached;
}


// Pull everything an item carries out of it.  Armed bombs and trapped items are left alone, taking
// those apart is a job for a merc, not for a sorting pass.
static void StripSectorInventoryItem(WORLDITEM& wi, std::vector<WORLDITEM>& extracted, UINT32& uiGunsUnloaded, UINT32& uiDetached)
{
	if (wi.o.ubNumberOfObjects == 0)     return;
	if (wi.o.bTrap > 0)                  return;
	if (wi.o.fFlags & OBJECT_ARMED_BOMB) return;

	if (UnloadSectorInventoryGun(wi, extracted)) ++uiGunsUnloaded;
	uiDetached += DetachSectorInventoryAttachments(wi, extracted);
}


// Only objects that hold nothing but points may have their points poured into another object.
static BOOLEAN CanPourSectorInventoryPoints(const WORLDITEM& wi)
{
	if (wi.o.ubNumberOfObjects == 0) return FALSE;
	if (wi.o.bTrap > 0)              return FALSE;
	if (ItemHasAttachments(wi.o))    return FALSE;

	return TRUE;
}


static BOOLEAN GetRefillablePointCapacity(const ItemModel* item, INT8& bMaxPoints);


// Magazines, kits and medkits hold points - rounds resp. charges - that can be moved between
// objects of the same item.  Pool them per item so that what is left is full objects and at most
// one part-used one.  Returns how many objects were emptied out this way.
static UINT32 MergeRefillableSectorInventory(std::vector<WORLDITEM>& items)
{
	UINT32 uiObjectsMerged = 0;

	std::vector<bool>       fPooled(items.size(), false);
	std::vector<WORLDITEM*> group;

	for (size_t iFirst = 0; iFirst < items.size(); ++iFirst)
	{
		if (fPooled[iFirst])                             continue;
		if (!CanPourSectorInventoryPoints(items[iFirst])) continue;

		INT8 bMaxPoints;
		if (!GetRefillablePointCapacity(GCM->getItem(items[iFirst].o.usItem), bMaxPoints)) continue;

		group.clear();
		group.push_back(&items[iFirst]);
		fPooled[iFirst] = true;

		for (size_t i = iFirst + 1; i < items.size(); ++i)
		{
			if (fPooled[i])                                             continue;
			if (!CanPourSectorInventoryPoints(items[i]))                continue;
			if (!CanMergeSectorInventorySlots(items[iFirst], items[i])) continue;

			group.push_back(&items[i]);
			fPooled[i] = true;
		}

		UINT32 uiPoints  = 0;
		UINT32 uiObjects = 0;
		for (const WORLDITEM* wi : group)
		{
			for (UINT8 ubObj = 0; ubObj < wi->o.ubNumberOfObjects; ++ubObj)
			{
				INT8 const bPoints = wi->o.bStatus[ubObj];
				if (bPoints > 0) uiPoints += std::min(bPoints, bMaxPoints);
				++uiObjects;
			}
		}

		if (uiPoints == 0) continue; // nothing but empties, leave them be

		// The group is repacked even when that frees no object at all: two half used bags
		// hold their points in two objects either way, but pouring one into the other still
		// leaves a full one and a part-used one rather than two part-used ones.
		UINT32 const uiNeeded = (uiPoints + bMaxPoints - 1) / bMaxPoints;

		// fill up the objects at the front of the group and drop the ones left over
		UINT32 uiLeft = uiPoints;
		for (WORLDITEM* wi : group)
		{
			UINT8 ubKept = 0;
			while (ubKept < wi->o.ubNumberOfObjects && uiLeft > 0)
			{
				INT8 const bHere = static_cast<INT8>(std::min<UINT32>(uiLeft, bMaxPoints));
				wi->o.bStatus[ubKept++] = bHere;
				uiLeft -= bHere;
			}

			if (ubKept == 0)
			{
				DeleteObj(&wi->o);
			}
			else
			{
				for (UINT8 ubObj = ubKept; ubObj < wi->o.ubNumberOfObjects; ++ubObj)
				{
					wi->o.bStatus[ubObj] = 0;
				}
				wi->o.ubNumberOfObjects = ubKept;
			}
		}

		uiObjectsMerged += uiObjects - uiNeeded;
	}

	return uiObjectsMerged;
}


void StackAndSortMapInventoryPool(void)
{
	if (!fShowMapInventoryPool) return;

	// don't shuffle the list while the player is carrying an item out of it
	if (gpItemPointer != NULL) return;

	// If in battle inform player they will have to do this in tactical
	if (!CanPlayerUseSectorInventory())
	{
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapInventoryErrorString[2], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		return;
	}

	// the empty slots are only padding, the occupied ones are all we have to look at
	std::vector<WORLDITEM> items;
	for (const WORLDITEM& wi : pInventoryPoolList)
	{
		if (wi.o.ubNumberOfObjects > 0) items.push_back(wi);
	}

	UINT32 uiGunsUnloaded = 0;
	UINT32 uiDetached     = 0;

	// Take every item apart first.  What comes out is collected in a second list so that the list
	// being walked cannot move under us; that list is then walked in turn, which also takes apart
	// an attachment that carried an attachment of its own.
	std::vector<WORLDITEM> pending;
	for (WORLDITEM& wi : items) StripSectorInventoryItem(wi, pending, uiGunsUnloaded, uiDetached);

	while (!pending.empty())
	{
		std::vector<WORLDITEM> next;
		for (WORLDITEM& wi : pending) StripSectorInventoryItem(wi, next, uiGunsUnloaded, uiDetached);

		items.insert(items.end(), pending.begin(), pending.end());
		pending.swap(next);
	}

	// pour part-used magazines and kits together before packing what is left into stacks
	UINT32 const uiObjectsMerged = MergeRefillableSectorInventory(items);

	UINT32 uiSlotsMerged = 0;

	for (size_t iTarget = 0; iTarget < items.size(); ++iTarget)
	{
		OBJECTTYPE& target = items[iTarget].o;
		if (target.ubNumberOfObjects == 0) continue;

		const ItemModel* const item         = GCM->getItem(target.usItem);
		BOOLEAN          const fMoney       = item->isMoney();
		UINT8            const ubStackLimit = SectorInventoryStackLimit(item);

		if (!fMoney && ubStackLimit < 2) continue;

		for (size_t iSource = iTarget + 1; iSource < items.size(); ++iSource)
		{
			OBJECTTYPE& source = items[iSource].o;
			if (source.ubNumberOfObjects == 0) continue;
			if (!CanMergeSectorInventorySlots(items[iTarget], items[iSource])) continue;

			if (fMoney)
			{
				// money is one object carrying an amount, not a stack of objects
				if (target.uiMoneyAmount >= MAX_MONEY_PER_SLOT) break;

				UINT32 const uiToTransfer = std::min<UINT32>(MAX_MONEY_PER_SLOT - target.uiMoneyAmount, source.uiMoneyAmount);
				target.uiMoneyAmount += uiToTransfer;
				target.bMoneyStatus   = 100;
				source.uiMoneyAmount -= uiToTransfer;

				if (source.uiMoneyAmount == 0)
				{
					DeleteObj(&source);
					++uiSlotsMerged;
				}
			}
			else
			{
				if (target.ubNumberOfObjects >= ubStackLimit) break;

				UINT8 const ubToTransfer = std::min<UINT8>(ubStackLimit - target.ubNumberOfObjects, source.ubNumberOfObjects);
				StackObjs(&source, &target, ubToTransfer);

				if (source.ubNumberOfObjects == 0) ++uiSlotsMerged;
			}
		}
	}

	// throw away the slots that were emptied out
	items.erase(std::remove_if(items.begin(), items.end(),
		[](const WORLDITEM& wi) { return wi.o.ubNumberOfObjects == 0; }), items.end());

	SortSectorInventory(items.data(), items.size());
	ReplaceMapInventoryPool(items);

	fMapPanelDirty        = TRUE;
	fMapScreenBottomDirty = TRUE;

	MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
		uiGunsUnloaded > 0 || uiDetached > 0 || uiObjectsMerged > 0 || uiSlotsMerged > 0 ?
		st_format_printf(pMapInventoryActionStrings[1], uiGunsUnloaded, uiDetached, uiObjectsMerged, uiSlotsMerged) :
		pMapInventoryActionStrings[0]);
}


// Magazines, kits and medkits hold points (rounds resp. kit charges) that can be moved between
// objects of the same item.  Returns how many points one such object holds when it is full.
static BOOLEAN GetRefillablePointCapacity(const ItemModel* const item, INT8& bMaxPoints)
{
	if (item->isAmmo())
	{
		// the round count shares its byte with the signed status value
		if (item->asAmmo()->capacity > 127) return FALSE;

		bMaxPoints = static_cast<INT8>(item->asAmmo()->capacity);
		return bMaxPoints > 0;
	}

	if (item->isKit() || item->isMedkit())
	{
		bMaxPoints = 100;
		return TRUE;
	}

	return FALSE;
}


// Only mercs standing in the sector the inventory belongs to may swap items with it.
static BOOLEAN IsMercInSectorShownInInventory(const SOLDIERTYPE& s)
{
	if (s.bLife <= 0)                      return FALSE;
	if (s.uiStatusFlags & SOLDIER_VEHICLE) return FALSE;
	if (s.bAssignment == ASSIGNMENT_POW)   return FALSE;
	if (s.bAssignment == IN_TRANSIT)       return FALSE;
	if (s.fBetweenSectors)                 return FALSE;

	return s.sSector.x == sSelMap.x &&
		s.sSector.y == sSelMap.y &&
		s.sSector.z == iCurrentMapSectorZ;
}


// Take up to bWanted points out of the stash, emptying one object at a time so that what is
// left behind is a few full objects rather than a lot of nearly empty ones.
static INT8 TakeRefillPointsFromStash(const std::vector<WORLDITEM*>& sources, size_t& uiNext, INT8 const bWanted)
{
	while (uiNext < sources.size())
	{
		OBJECTTYPE& o = sources[uiNext]->o;

		if (o.ubNumberOfObjects == 0)
		{
			++uiNext;
			continue;
		}

		UINT8 const ubLast     = o.ubNumberOfObjects - 1;
		INT8&       bAvailable = o.bStatus[ubLast];

		if (bAvailable <= 0)
		{
			// spent object, drop it and take from the one below
			RemoveObjFrom(&o, ubLast);
			continue;
		}

		INT8 const bTaken = std::min<INT8>(bWanted, bAvailable);
		bAvailable -= bTaken;
		if (bAvailable == 0) RemoveObjFrom(&o, ubLast);

		return bTaken;
	}

	return 0;
}


void RefillMercItemsFromMapInventoryPool(void)
{
	if (!fShowMapInventoryPool) return;

	if (gpItemPointer != NULL) return;

	// If in battle inform player they will have to do this in tactical
	if (!CanPlayerUseSectorInventory())
	{
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapInventoryErrorString[2], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		return;
	}

	if (iCurrentlyHighLightedItem == -1)
	{
		MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMapInventoryActionStrings[2]);
		return;
	}

	size_t const uiSlot = iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT + iCurrentlyHighLightedItem;
	if (uiSlot >= pInventoryPoolList.size()) return;

	WORLDITEM& highlighted = pInventoryPoolList[uiSlot];
	if (highlighted.o.ubNumberOfObjects == 0) return;

	// is this item reachable
	if (!(highlighted.usFlags & WORLD_ITEM_REACHABLE))
	{
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[STR_LATE_38], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		return;
	}

	UINT16           const usItem = highlighted.o.usItem;
	const ItemModel* const item   = GCM->getItem(usItem);

	INT8 bMaxPoints;
	if (!GetRefillablePointCapacity(item, bMaxPoints))
	{
		MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
			st_format_printf(pMapInventoryActionStrings[3], item->getName()));
		return;
	}

	// The pointed at stack is spent first, then every other reachable stack of the same item in
	// this sector, so that one keypress can top up the whole squad.
	std::vector<WORLDITEM*> sources;
	sources.push_back(&highlighted);
	for (WORLDITEM& wi : pInventoryPoolList)
	{
		if (&wi == &highlighted)                  continue;
		if (wi.o.ubNumberOfObjects == 0)          continue;
		if (wi.o.usItem != usItem)                continue;
		if (!(wi.usFlags & WORLD_ITEM_REACHABLE)) continue;
		if (wi.o.bTrap > 0)                       continue;
		if (ItemHasAttachments(wi.o))             continue;

		sources.push_back(&wi);
	}

	size_t uiNextSource    = 0;
	UINT32 uiPointsMoved   = 0;
	UINT32 uiObjectsTopped = 0;

	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (uiNextSource >= sources.size()) break;
		if (!IsMercInSectorShownInInventory(*s)) continue;

		FOR_EACH_SOLDIER_INV_SLOT(o, *s)
		{
			if (uiNextSource >= sources.size()) break;
			if (o->usItem != usItem)            continue;
			if (ItemHasAttachments(*o))         continue;

			for (UINT8 ubObj = 0; ubObj < o->ubNumberOfObjects; ++ubObj)
			{
				INT8& bStatus = o->bStatus[ubObj];
				if (bStatus >= bMaxPoints) continue;

				BOOLEAN fTopped = FALSE;
				while (bStatus < bMaxPoints)
				{
					INT8 const bTaken = TakeRefillPointsFromStash(sources, uiNextSource, bMaxPoints - bStatus);
					if (bTaken == 0) break; // nothing left in the stash

					bStatus       += bTaken;
					uiPointsMoved += bTaken;
					fTopped        = TRUE;
				}

				if (fTopped) ++uiObjectsTopped;
			}
		}
	}

	if (uiPointsMoved == 0)
	{
		MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
			st_format_printf(pMapInventoryActionStrings[4], item->getName()));
		return;
	}

	MapScreenMessage(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE,
		st_format_printf(pMapInventoryActionStrings[5], uiObjectsTopped, item->getName(), uiPointsMoved));

	fMapPanelDirty           = TRUE;
	fMapScreenBottomDirty    = TRUE;
	fTeamPanelDirty          = TRUE;
	fCharacterInfoPanelDirty = TRUE;
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
	SetFontAttributes(COMPFONT, 183);
	SetFontDestBuffer(guiSAVEBUFFER);

	MPrintCenteredInBox(STD_SCREEN_X, STD_SCREEN_Y,
		ST::format("{} / {}", iCurrentInventoryPoolPage + 1, iLastInventoryPoolPage + 1),
		g_sector_inv_page_box);

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


static void DrawNumberOfInventoryPoolItems()
{
	SetFontAttributes(COMPFONT, 183);
	SetFontDestBuffer(guiSAVEBUFFER);

	MPrintCenteredInBox(STD_SCREEN_X, STD_SCREEN_Y,
		ST::string::from_uint(GetTotalNumberOfItemsInSectorStash()),
		g_sector_inv_count_box);

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
	SetFontAttributes(COMPFONT, 183);
	SetFontDestBuffer(guiSAVEBUFFER);

	MPrintCenteredInBox(STD_SCREEN_X, STD_SCREEN_Y,
		ST::format("{}{}{}", pMapVertIndex[ sSelMap.y ],
			pMapHortIndex[ sSelMap.x ], pMapDepthIndex[ iCurrentMapSectorZ ]),
		g_sector_inv_loc_box);

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
	// Prints "Sector Inventory" in the English localization.

	SetFontDestBuffer(guiSAVEBUFFER);
	SetFontAttributes(FONT14ARIAL, FONT_WHITE);

	MPrintCenteredInBox(STD_SCREEN_X, STD_SCREEN_Y,
		zMarksMapScreenText[11], g_sector_inv_title_box);

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
	SGPSector sector;
	return
		!GetCurrentBattleSectorXYZAndReturnTRUEIfThereIsABattle(sector) ||
		sSelMap.x           != sector.x ||
		sSelMap.y           != sector.y ||
		iCurrentMapSectorZ != sector.z;
}
