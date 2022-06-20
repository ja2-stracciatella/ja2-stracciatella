#include "HImage.h"
#include "Handle_Items.h"
#include "Edit_Sys.h"
#include "TileDat.h"
#include "VSurface.h"
#include "VObject.h"
#include "MouseSystem.h"
#include "Input.h"
#include "SysUtil.h"
#include "Font.h"
#include "Font_Control.h"
#include "EditScreen.h"
#include "SelectWin.h"
#include "VObject_Blitters.h"
#include "Interface_Items.h"
#include "Text.h"
#include "Action_Items.h"
#include "World_Items.h"
#include "EditorDefines.h"
#include "EditorItems.h"
#include "EditorMercs.h"
#include "Weapons.h"
#include "Editor_Taskbar_Utils.h"
#include "WordWrap.h"
#include "Item_Statistics.h"
#include "Simple_Render_Utils.h"
#include "WorldMan.h"
#include "Random.h"
#include "Pits.h"
#include "Keys.h"
#include "Debug.h"
#include "Items.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "MagazineModel.h"
#include "WeaponModels.h"

#include <string_theory/format>
#include <string_theory/string>


#define NUMBER_TRIGGERS			27
#define PRESSURE_ACTION_ID	(NUMBER_TRIGGERS - 1)

extern ITEM_POOL		*gpEditingItemPool;

INT32 giDefaultExistChance = 100;

struct IPListNode
{
	INT16 sGridNo;
	IPListNode* next;
};

static IPListNode* pIPHead = NULL;

static IPListNode* gpCurrItemPoolNode = NULL;
ITEM_POOL *gpItemPool = NULL;


static void ShowItemCursor(INT32 iMapIndex);


void BuildItemPoolList(void)
{
	KillItemPoolList();

	IPListNode** anchor = &pIPHead;
	for (UINT16 i = 0; i < WORLD_MAX; ++i)
	{
		if (GetItemPool(i, 0) == NULL) continue;

		ShowItemCursor(i);

		IPListNode* const n = new IPListNode{};
		n->sGridNo = i;
		n->next    = NULL;

		*anchor = n;
		anchor = &n->next;
	}
	gpCurrItemPoolNode = pIPHead;
	SpecifyItemToEdit(NULL, -1);
}


static void HideItemCursor(INT32 iMapIndex);


void KillItemPoolList()
{
	IPListNode *pIPCurr;
	pIPCurr = pIPHead;
	while( pIPCurr )
	{
		HideItemCursor( pIPCurr->sGridNo );
		pIPHead = pIPHead->next;
		delete pIPCurr;
		pIPCurr = pIPHead;
	}
	pIPHead = NULL;
}

//Contains global information about the editor items
//May be expanded to encapsulate the entire editor later.
EditorItemsInfo eInfo;

//Does some precalculations regarding the number of each item type, so that it
//isn't calculated every time a player changes categories.
void EntryInitEditorItemsInfo()
{
	INT32 i;
	eInfo.uiBuffer = 0;
	eInfo.fActive = 0;
	eInfo.sScrollIndex = 0;
	eInfo.sSelItemIndex = 0;
	eInfo.sHilitedItemIndex = -1;
	eInfo.sNumItems = 0;
	eInfo.pusItemIndex = NULL;
	if( eInfo.fGameInit )
	{ //This only gets called one time in game execution.
		eInfo = EditorItemsInfo{};
		eInfo.sHilitedItemIndex = -1;
		eInfo.uiItemType = TBAR_MODE_ITEM_WEAPONS;
		//Pre-calculate the number of each item type.
		eInfo.sNumTriggers = NUMBER_TRIGGERS;
		for( i=0; i < MAXITEMS; i++ )
		{
			const ItemModel* item = GCM->getItem(i);
			if( GCM->getItem(i)->getFlags() & ITEM_NOT_EDITOR )
				continue;
			if( i == SWITCH || i == ACTION_ITEM )
			{

			}
			else switch( item->getItemClass() )
			{
				case IC_GUN:
				case IC_BLADE:
				case IC_THROWN:
				case IC_LAUNCHER:
				case IC_THROWING_KNIFE:
					eInfo.sNumWeapons++;
					break;
				case IC_PUNCH:
					if ( i != NOTHING )
					{
						eInfo.sNumWeapons++;
					}
					break;
				case IC_AMMO:
					eInfo.sNumAmmo++;
					break;
				case IC_ARMOUR:
					eInfo.sNumArmour++;
					break;
				case IC_GRENADE:
				case IC_BOMB:
					eInfo.sNumExplosives++;
					break;
				case IC_MEDKIT:
				case IC_KIT:
				case IC_FACE:
				case IC_MISC:
				case IC_MONEY:
					if( eInfo.sNumEquipment1 < 30 )
						eInfo.sNumEquipment1++;
					else if( eInfo.sNumEquipment2 < 30 )
						eInfo.sNumEquipment2++;
					else
						eInfo.sNumEquipment3++;
					break;
				//case IC_KEY:
				//	eInfo.sNumKeys++;
				//	break;
			}
		}
		eInfo.sNumKeys = NUM_KEYS;
	}
}


static void DrawItemCentered(const ItemModel * item, SGPVSurface* const vs, INT32 x, INT32 const y, INT16 const outline)
{
	// Calculate the center position of the graphic in a 60 pixel wide area.
	auto graphic = GetSmallInventoryGraphicForItem(item);
	auto vo  = graphic.first;
	auto gfx = graphic.second;
	ETRLEObject const& e   = vo->SubregionProperties(gfx);
	x += (60 - e.usWidth) / 2 - e.sOffsetX;
	BltVideoObjectOutline(vs, vo, gfx, x, y, outline);
}


void InitEditorItemsInfo(ToolbarMode const uiItemType)
{
	SGPRect	SaveRect, NewRect;
	INT16 i, x, y;
	UINT16 usCounter;
	ST::string pStr;
	BOOLEAN fTypeMatch;
	INT32 iEquipCount = 0;

	// Check to make sure that there isn't already a valid eInfo
	if( eInfo.fActive )
	{
		if( eInfo.uiItemType == uiItemType )
		{	//User clicked on the same item classification -- ignore
			return;
		}
		else
		{	//User selected a different item classification -- delete it first.
			ClearEditorItemsInfo();
			ClearTaskbarRegion(100, 0, 480, 80);
		}
	}
	else
	{
		//Clear the menu area, so that the buffer doesn't get corrupted.
		ClearTaskbarRegion(100, 0, 480, 80);
	}
	EnableEditorRegion( ITEM_REGION_ID );

	eInfo.uiItemType = uiItemType;
	eInfo.fActive = TRUE;
	//Begin initialization of data.
	switch(uiItemType)
	{
		case TBAR_MODE_ITEM_WEAPONS:
			eInfo.sNumItems = eInfo.sNumWeapons;
			eInfo.sScrollIndex = eInfo.sSaveWeaponsScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelWeaponsIndex;
			break;
		case TBAR_MODE_ITEM_AMMO:
			eInfo.sNumItems = eInfo.sNumAmmo;
			eInfo.sScrollIndex = eInfo.sSaveAmmoScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelAmmoIndex;
			break;
		case TBAR_MODE_ITEM_ARMOUR:
			eInfo.sNumItems = eInfo.sNumArmour;
			eInfo.sScrollIndex = eInfo.sSaveArmourScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelArmourIndex;
			break;
		case TBAR_MODE_ITEM_EXPLOSIVES:
			eInfo.sNumItems = eInfo.sNumExplosives;
			eInfo.sScrollIndex = eInfo.sSaveExplosivesScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelExplosivesIndex;
			break;
		case TBAR_MODE_ITEM_EQUIPMENT1:
			eInfo.sNumItems = eInfo.sNumEquipment1;
			eInfo.sScrollIndex = eInfo.sSaveEquipment1ScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelEquipment1Index;
			break;
		case TBAR_MODE_ITEM_EQUIPMENT2:
			eInfo.sNumItems = eInfo.sNumEquipment2;
			eInfo.sScrollIndex = eInfo.sSaveEquipment2ScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelEquipment2Index;
			break;
		case TBAR_MODE_ITEM_EQUIPMENT3:
			eInfo.sNumItems = eInfo.sNumEquipment3;
			eInfo.sScrollIndex = eInfo.sSaveEquipment3ScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelEquipment3Index;
			break;
		case TBAR_MODE_ITEM_TRIGGERS:
			eInfo.sNumItems = eInfo.sNumTriggers;
			eInfo.sScrollIndex = eInfo.sSaveTriggersScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelTriggersIndex;
			break;
		case TBAR_MODE_ITEM_KEYS:
			eInfo.sNumItems = eInfo.sNumKeys;
			eInfo.sScrollIndex = eInfo.sSaveKeysScrollIndex;
			eInfo.sSelItemIndex = eInfo.sSaveSelKeysIndex;
			break;
		default:
			//error
			return;
	}
	//Allocate memory to store all the item pointers.
	eInfo.pusItemIndex = new UINT16[eInfo.sNumItems]{};

	//Disable the appropriate scroll buttons based on the saved scroll index if applicable
	//Left most scroll position
	DetermineItemsScrolling();
	//calculate the width of the buffer based on the number of items.
	//every pair of items (odd rounded up) requires 60 pixels for width.
	//the minimum buffer size is 420.  Height is always 80 pixels.
	const INT16 w = (eInfo.sNumItems > 12 ? (eInfo.sNumItems + 1) / 2 * 60 : 360);
	const INT16 h = 80;
	// Create item buffer

	//!!!Memory check.  Create the item buffer
	eInfo.uiBuffer = AddVideoSurface(w, h, PIXEL_DEPTH);

	//copy a blank chunk of the editor interface to the new buffer.
	for (i = 0; i < w; i += 60)
	{
		SGPBox const r = { 100, EDITOR_TASKBAR_POS_Y, 60, 80 };
		BltVideoSurface(eInfo.uiBuffer, FRAME_BUFFER, i, 0, &r);
	}

	x = 0;
	y = 0;
	usCounter = 0;
	NewRect.iTop    = 0;
	NewRect.iBottom = h;
	NewRect.iLeft   = 0;
	NewRect.iRight  = w;
	GetClippingRect(&SaveRect);
	SetClippingRect(&NewRect);
	if( eInfo.uiItemType == TBAR_MODE_ITEM_KEYS )
	{ //Keys use a totally different method for determining
		for( i = 0; i < eInfo.sNumItems; i++ )
		{
			UINT16 const item_id = KEY_1 + LockTable[i].usKeyItem;

			//Store these item pointers for later when rendering selected items.
			eInfo.pusItemIndex[i] = item_id;

			SetFontDestBuffer(eInfo.uiBuffer);

			pStr = ST::format("{}", LockTable[i].ubEditorName);
			DisplayWrappedString(x, y + 25, 60, 2, SMALLCOMPFONT, FONT_WHITE, pStr, FONT_BLACK, CENTER_JUSTIFIED | MARK_DIRTY);

			DrawItemCentered(GCM->getItem(item_id), eInfo.uiBuffer, x, y + 2, SGP_TRANSPARENT);

			//cycle through the various slot positions (0,0), (0,40), (60,0), (60,40), (120,0)...
			if( y == 0 )
			{
				y = 40;
			}
			else
			{
				y = 0;
				x += 60;
			}
		}
	}
	else for( i = 0; i < eInfo.sNumItems; i++ )
	{

		fTypeMatch = FALSE;
		while( usCounter<MAXITEMS && !fTypeMatch )
		{
			const ItemModel * item = GCM->getItem(usCounter);
			if( GCM->getItem(usCounter)->getFlags() & ITEM_NOT_EDITOR )
			{
				usCounter++;
				continue;
			}
			if( eInfo.uiItemType == TBAR_MODE_ITEM_TRIGGERS )
			{
				if( i < PRESSURE_ACTION_ID )
					usCounter = ( i % 2 ) ? ACTION_ITEM : SWITCH;
				else
					usCounter = ACTION_ITEM;
				fTypeMatch = TRUE;
				item = GCM->getItem(usCounter);
			}
			else switch( item->getItemClass() )
			{
				case IC_GUN:
				case IC_BLADE:
				case IC_LAUNCHER:
				case IC_THROWN:
				case IC_THROWING_KNIFE:
					fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_WEAPONS;
					break;
				case IC_PUNCH:
					if ( i != NOTHING )
					{
						fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_WEAPONS;
					}
					else
					{
						fTypeMatch = FALSE;
					}
					break;
				case IC_AMMO:
					fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_AMMO;
					break;
				case IC_ARMOUR:
					fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_ARMOUR;
					break;
				case IC_GRENADE:
				case IC_BOMB:
					fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EXPLOSIVES;
					break;
				case IC_MEDKIT:
				case IC_KIT:
				case IC_FACE:
				case IC_MISC:
				case IC_MONEY:
					if( usCounter == ACTION_ITEM || usCounter == SWITCH )
						break;
					if( iEquipCount < 30 )
						fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EQUIPMENT1;
					else if( iEquipCount < 60 )
						fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EQUIPMENT2;
					else
						fTypeMatch = eInfo.uiItemType == TBAR_MODE_ITEM_EQUIPMENT3;
					iEquipCount++;
					break;
			}
			if( fTypeMatch )
			{
				//Store these item pointers for later when rendering selected items.
				eInfo.pusItemIndex[i] = usCounter;

				SetFontDestBuffer(eInfo.uiBuffer);

				if( eInfo.uiItemType != TBAR_MODE_ITEM_TRIGGERS )
				{
					pStr = item->getName();
				}
				else
				{
					if( i == PRESSURE_ACTION_ID )
					{
						pStr = "Pressure Action";
					}
					else if( i < 2 )
					{
						if( usCounter == SWITCH )
							pStr = "Panic Trigger1";
						else
							pStr = "Panic Action1";
					}
					else if( i < 4 )
					{
						if( usCounter == SWITCH )
							pStr = "Panic Trigger2";
						else
							pStr = "Panic Action2";
					}
					else if( i < 6 )
					{
						if( usCounter == SWITCH )
							pStr = "Panic Trigger3";
						else
							pStr = "Panic Action3";
					}
					else
					{
						if( usCounter == SWITCH )
							pStr = ST::format("Trigger{}", (i - 4) / 2);
						else
							pStr = ST::format("Action{}", (i - 4) / 2);
					}
				}
				DisplayWrappedString(x, y + 25, 60, 2, SMALLCOMPFONT, FONT_WHITE, pStr, FONT_BLACK, CENTER_JUSTIFIED | MARK_DIRTY);

				DrawItemCentered(item, eInfo.uiBuffer, x, y + 2, SGP_TRANSPARENT);

				//cycle through the various slot positions (0,0), (0,40), (60,0), (60,40), (120,0)...
				if( y == 0 )
				{
					y = 40;
				}
				else
				{
					y = 0;
					x += 60;
				}
			}
			usCounter++;
		}
	}
	SetFontDestBuffer(FRAME_BUFFER);
	SetClippingRect(&SaveRect);
	gfRenderTaskbar = TRUE;
}

void DetermineItemsScrolling()
{
	if( !eInfo.sScrollIndex )
		DisableEditorButton( ITEMS_LEFTSCROLL );
	else
		EnableEditorButton( ITEMS_LEFTSCROLL );
	//Right most scroll position.  Calculated by taking every pair of numItems rounded up,
	//and subtracting 7 (because a scroll index 0 is disabled if there are <=12 items,
	//index 1 for <=14 items, index 2 for <=16 items...
	if (eInfo.sScrollIndex == std::max((eInfo.sNumItems + 1) / 2 - 6, 0))
		DisableEditorButton( ITEMS_RIGHTSCROLL );
	else
		EnableEditorButton( ITEMS_RIGHTSCROLL );
}


static UINT16 CountNumberOfEditorPlacementsInWorld(UINT16 usEInfoIndex, UINT16* pusQuantity);


static void drawItemWithOutline(INT16 min_idx, INT16 end_idx, INT16 scroll_idx, INT16 itemIndex, INT16 const outline)
{
	if (min_idx <= itemIndex && itemIndex < end_idx)
	{
		INT16   const  x    = (itemIndex / 2 - scroll_idx) * 60 + 110;
		INT16   const  y    = EDITOR_TASKBAR_POS_Y + (itemIndex % 2) * 40;
		const ItemModel *item = GCM->getItem(eInfo.pusItemIndex[itemIndex]);
		DrawItemCentered(item, FRAME_BUFFER, x, y + 2, outline);
	}
}


void RenderEditorItemsInfo()
{
	if (!eInfo.fActive) return;

	if ((gusMouseXPos < 110) || (480 < gusMouseXPos) ||
			(gusMouseYPos < EDITOR_TASKBAR_POS_Y) || (EDITOR_TASKBAR_POS_Y + 80 < gusMouseYPos))
	{ // Mouse has moved out of the items display region -- so nothing can be highlighted.
		eInfo.sHilitedItemIndex = -1;
	}

	INT16 const scroll_idx = eInfo.sScrollIndex;

	SGPBox const r = { (UINT16)(60 * scroll_idx), 0, 360, 80 };
	BltVideoSurface(FRAME_BUFFER, eInfo.uiBuffer, 110, EDITOR_TASKBAR_POS_Y, &r);

	/* Calculate the min and max index that is currently shown.  This determines
	 * if the highlighted and/or selected items are drawn with the outlines. */
	INT16 const min_idx = scroll_idx * 2;
	INT16 const end_idx = std::min(min_idx + 12, int(eInfo.sNumItems));

	// Draw the hilighted and selected items if applicable.
	if (eInfo.pusItemIndex)
	{
		drawItemWithOutline(min_idx, end_idx, scroll_idx, eInfo.sHilitedItemIndex, Get16BPPColor(FROMRGB(250, 250, 0)));
		drawItemWithOutline(min_idx, end_idx, scroll_idx, eInfo.sSelItemIndex,     Get16BPPColor(FROMRGB(250, 0, 0)));
	}

	// Draw the numbers of each visible item that currently resides in the world.
	for (INT16 i = min_idx; i < end_idx; ++i)
	{
		UINT16       quantity;
		UINT16 const n_items = CountNumberOfEditorPlacementsInWorld(i, &quantity);
		if (n_items == 0) continue;

		INT16 const x = (i / 2 - scroll_idx) * 60 + 110;
		INT16 const y = EDITOR_TASKBAR_POS_Y + (i % 2) * 40;
		SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
		if (n_items == quantity)
		{
			MPrint(x + 12, y + 4, ST::format("{}", n_items));
		}
		else
		{
			MPrint(x + 12, y + 4, ST::format("{}({})", n_items, quantity));
		}
	}
}


void ClearEditorItemsInfo()
{
	if( eInfo.uiBuffer )
	{
		DeleteVideoSurface(eInfo.uiBuffer);
		eInfo.uiBuffer = 0;
	}
	if( eInfo.pusItemIndex )
	{
		delete[] eInfo.pusItemIndex;
		eInfo.pusItemIndex = NULL;
	}
	DisableEditorRegion( ITEM_REGION_ID );
	eInfo.fActive = 0;
	eInfo.sNumItems = 0;
	//save the highlighted selections
	switch( eInfo.uiItemType )
	{
		case TBAR_MODE_ITEM_WEAPONS:
			eInfo.sSaveSelWeaponsIndex = eInfo.sSelItemIndex;
			eInfo.sSaveWeaponsScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_AMMO:
			eInfo.sSaveSelAmmoIndex = eInfo.sSelItemIndex;
			eInfo.sSaveAmmoScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_ARMOUR:
			eInfo.sSaveSelArmourIndex = eInfo.sSelItemIndex;
			eInfo.sSaveArmourScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_EXPLOSIVES:
			eInfo.sSaveSelExplosivesIndex = eInfo.sSelItemIndex;
			eInfo.sSaveExplosivesScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_EQUIPMENT1:
			eInfo.sSaveSelEquipment1Index = eInfo.sSelItemIndex;
			eInfo.sSaveEquipment1ScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_EQUIPMENT2:
			eInfo.sSaveSelEquipment2Index = eInfo.sSelItemIndex;
			eInfo.sSaveEquipment2ScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_EQUIPMENT3:
			eInfo.sSaveSelEquipment3Index = eInfo.sSelItemIndex;
			eInfo.sSaveEquipment3ScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_TRIGGERS:
			eInfo.sSaveSelTriggersIndex = eInfo.sSelItemIndex;
			eInfo.sSaveTriggersScrollIndex = eInfo.sScrollIndex;
			break;
		case TBAR_MODE_ITEM_KEYS:
			eInfo.sSaveSelKeysIndex = eInfo.sSelItemIndex;
			eInfo.sSaveKeysScrollIndex = eInfo.sScrollIndex;
			break;
		default:
			break;
	}
}


static void FindNextItemOfSelectedType(void);


void HandleItemsPanel( UINT16 usScreenX, UINT16 usScreenY, INT8 bEvent )
{
	INT16 sIndex;
	UINT16 usQuantity;
	//Calc base index from scrolling index
	sIndex = eInfo.sScrollIndex * 2;
	//Determine if the index is in the first row or second row from mouse YPos.
	if( usScreenY >= EDITOR_TASKBAR_POS_Y + 40 )
		sIndex++;
	//Add the converted mouse's XPos into a relative index;
	//Calc:  starting from 110, for every 60 pixels, add 2 to the index
	sIndex += ((usScreenX-110)/60) * 2;
	switch( bEvent )
	{
		case GUI_MOVE_EVENT:
			if( sIndex < eInfo.sNumItems )
			{
				if( eInfo.sHilitedItemIndex != sIndex )
					gfRenderTaskbar = TRUE;
				//this index will now highlight in yellow.
				eInfo.sHilitedItemIndex = sIndex;
			}
			break;
		case GUI_LCLICK_EVENT:
			if( sIndex < eInfo.sNumItems )
			{
				//this index will now highlight in red.
				if( eInfo.sSelItemIndex != sIndex )
					gfRenderTaskbar = TRUE;
				eInfo.sSelItemIndex = sIndex;
				if( gfMercGetItem )
				{
					gfMercGetItem = FALSE;
					gusMercsNewItemIndex = eInfo.pusItemIndex[ eInfo.sSelItemIndex ];
					SetMercEditingMode( MERC_INVENTORYMODE );
					ClearEditorItemsInfo();
				}
			}
			break;
		case GUI_RCLICK_EVENT:
			if( gfMercGetItem )
			{
				gfMercGetItem = FALSE;
				gusMercsNewItemIndex = 0xffff;
				SetMercEditingMode( MERC_INVENTORYMODE );
				ClearEditorItemsInfo();
			}
			else if( sIndex < eInfo.sNumItems )
			{
				eInfo.sSelItemIndex = sIndex;
				gfRenderTaskbar = TRUE;
				if( CountNumberOfEditorPlacementsInWorld( eInfo.sSelItemIndex, &usQuantity ) )
				{
					FindNextItemOfSelectedType();
				}
			}
			break;
	}
}


static void ShowItemCursor(INT32 const iMapIndex)
{
	for (LEVELNODE const* n = gpWorldLevelData[iMapIndex].pTopmostHead; n; n = n->pNext)
	{
		if (n->usIndex == SELRING) return;
	}
	AddTopmostToTail(iMapIndex, SELRING1);
}


static void HideItemCursor(INT32 iMapIndex)
{
	RemoveTopmost( iMapIndex, SELRING1 );
}


static BOOLEAN TriggerAtGridNo(INT16 sGridNo)
{
	return ItemTypeExistsAtLocation(sGridNo, SWITCH, 0, 0);
}


static INT8 CalcItemFrequency(UINT16 const item_idx)
{
	return
		item_idx < 2 ? PANIC_FREQUENCY   :
		item_idx < 4 ? PANIC_FREQUENCY_2 :
		item_idx < 6 ? PANIC_FREQUENCY_3 :
		FIRST_MAP_PLACED_FREQUENCY + (item_idx - 4) / 2;
}


void AddSelectedItemToWorld(INT16 sGridNo)
{
	// Extract the currently selected item.
	SpecifyItemToEdit(NULL, -1);

	OBJECTTYPE tempObject;
	if (eInfo.uiItemType == TBAR_MODE_ITEM_KEYS)
	{
		CreateKeyObject(&tempObject, 1, (UINT8)eInfo.sSelItemIndex);
	}
	else
	{
		CreateItem(eInfo.pusItemIndex[eInfo.sSelItemIndex], 100, &tempObject);
	}

	Visibility bVisibility = INVISIBLE;
	UINT16     usFlags     = 0;
	switch (tempObject.usItem)
	{
		case MINE:
			if (bVisibility == BURIED) usFlags |= WORLD_ITEM_ARMED_BOMB;
			break;

		case MONEY:
		case SILVER:
		case GOLD:
			tempObject.bStatus[0]    = 100;
			tempObject.uiMoneyAmount = 100 + Random(19901);
			break;

		case OWNERSHIP:
			tempObject.ubOwnerProfile = NO_PROFILE;
			bVisibility = BURIED;
			break;

		case SWITCH:
			// Restricted to one action per gridno.
			if (TriggerAtGridNo(sGridNo)) return;
			bVisibility = BURIED;
			tempObject.bStatus[0]  = 100;
			tempObject.ubBombOwner = 1;
			tempObject.bFrequency  = CalcItemFrequency(eInfo.sSelItemIndex);
			usFlags |= WORLD_ITEM_ARMED_BOMB;
			break;

		case ACTION_ITEM:
			bVisibility = BURIED;
			tempObject.bStatus[0]  = 100;
			tempObject.ubBombOwner = 1;
			tempObject.bTrap       = gbDefaultBombTrapLevel;
			if (eInfo.sSelItemIndex < PRESSURE_ACTION_ID)
			{
				tempObject.bDetonatorType = BOMB_REMOTE;
				tempObject.bFrequency     = CalcItemFrequency(eInfo.sSelItemIndex);
			}
			else
			{
				tempObject.bDetonatorType = BOMB_PRESSURE;
				tempObject.bDelay         = 0;
			}
			ChangeActionItem(&tempObject, gbActionItemIndex);
			tempObject.fFlags |= OBJECT_ARMED_BOMB;
			switch (gbActionItemIndex)
			{
				case ACTIONITEM_SMPIT: Add3X3Pit(sGridNo); break;
				case ACTIONITEM_LGPIT: Add5X5Pit(sGridNo); break;
			}
			usFlags |= WORLD_ITEM_ARMED_BOMB;
			break;
	}

	INT32 const iItemIndex = InternalAddItemToPool(&sGridNo, &tempObject, bVisibility, 0, usFlags, 0);
	WORLDITEM&  wi         = GetWorldItem(iItemIndex);
	wi.ubNonExistChance = (tempObject.usItem == OWNERSHIP ? 0 : 100 - giDefaultExistChance);

	OBJECTTYPE&          obj  = wi.o;
	const ItemModel * item = GCM->getItem(obj.usItem);
	if (item->isAmmo())
	{
		UINT8 const mag_size = item->asAmmo()->capacity;
		obj.ubShotsLeft[0] = Random(2) ? mag_size : Random(mag_size);
	}
	else
	{
		obj.bStatus[0] = 70 + Random(26);
	}
	if (item->isGun())
	{
		obj.ubGunShotsLeft = obj.usItem == ROCKET_LAUNCHER ? 1 : Random(GCM->getWeapon(obj.usItem)->ubMagSize);
	}

	for (ITEM_POOL* ip = GetItemPool(sGridNo, 0); Assert(ip), ip; ip = ip->pNext)
	{
		if (&GetWorldItem(ip->iItemIndex).o != &obj) continue;
		gpItemPool = ip;
		break;
	}

	SpecifyItemToEdit(&obj, sGridNo);

	//Get access to the itempool.
	//search for a current node in list containing same mapindex
	IPListNode** anchor = &pIPHead;
	for (IPListNode* i = pIPHead; i; i = i->next)
	{
		anchor = &i->next;
		if (i->sGridNo == sGridNo)
		{
			//found one, so we don't need to add it
			gpCurrItemPoolNode = i;
			return;
		}
	}
	//there isn't one, so we will add it now.
	ShowItemCursor(sGridNo);

	IPListNode* const n = new IPListNode{};
	n->next            = 0;
	n->sGridNo         = sGridNo;
	*anchor            = n;
	gpCurrItemPoolNode = n;
}


void HandleRightClickOnItem( INT16 sGridNo )
{
	ITEM_POOL *pItemPool;
	IPListNode *pIPCurr;

	if( gsItemGridNo == sGridNo )
	{ //Clicked on the same gridno as the selected item.  Automatically select the next
		//item in the same pool.
		pItemPool = gpItemPool->pNext;
		if( !pItemPool )
		{ //currently selected item was last node, so select the head node even if it is the same.
			pItemPool = GetItemPool(sGridNo, 0);
		}
	}
	else
	{
		pItemPool = GetItemPool(sGridNo, 0);
		if (pItemPool == NULL)
		{
			// possibly relocate selected item to this gridno?
			return;
		}
	}

	gpItemPool = pItemPool;

	//set up the item pool pointer to point to the same mapindex node
	pIPCurr = pIPHead;
	gpCurrItemPoolNode = NULL;
	while( pIPCurr )
	{
		if( pIPCurr->sGridNo == sGridNo )
		{
			gpCurrItemPoolNode = pIPCurr;
			break;
		}
		pIPCurr = pIPCurr->next;
	}
	Assert( gpCurrItemPoolNode );
	WORLDITEM& wi = GetWorldItem(gpItemPool->iItemIndex);
	SpecifyItemToEdit(&wi.o, wi.sGridNo);
}



void DeleteSelectedItem()
{
	SpecifyItemToEdit( NULL, -1 );
	//First, check to see if there even is a currently selected item.
	if( iCurrentTaskbar == TASK_MERCS )
	{
		DeleteSelectedMercsItem();
		return;
	}
	if( gpItemPool )
	{ //Okay, we have a selected item...
		//save the mapindex
		if( gpItemPool->pNext )
		{
			WORLDITEM& wi = GetWorldItem(gpItemPool->pNext->iItemIndex);
			SpecifyItemToEdit(&wi.o, wi.sGridNo);
		}
		//remove the item
		WORLDITEM&  wi      = GetWorldItem(gpItemPool->iItemIndex);
		INT16 const sGridNo = wi.sGridNo;
		if (wi.o.usItem == ACTION_ITEM)
		{
			switch (wi.o.bActionValue)
			{
				case ACTION_ITEM_SMALL_PIT: Remove3X3Pit(sGridNo); break;
				case ACTION_ITEM_LARGE_PIT: Remove5X5Pit(sGridNo); break;
			}
		}
		if( gpEditingItemPool == gpItemPool )
			gpEditingItemPool = NULL;
		RemoveItemFromPool(wi);
		gpItemPool = NULL;
		//determine if there are still any items at this location
		gpItemPool = GetItemPool(sGridNo, 0);
		if (gpItemPool == NULL)
		{ //no items left, so remove the node from the list.
			IPListNode *pIPPrev, *pIPCurr;
			pIPCurr = pIPHead;
			pIPPrev = NULL;
			while( pIPCurr )
			{
				if( pIPCurr->sGridNo == sGridNo )
				{
					if( pIPPrev ) //middle of list
						pIPPrev->next = pIPCurr->next;
					else //head of list
						pIPHead = pIPHead->next;
					//move the curr item pool to the next one.
					if( pIPCurr->next )
						gpCurrItemPoolNode = pIPCurr->next;
					else
						gpCurrItemPoolNode = pIPHead;
					if( gpCurrItemPoolNode )
					{
						gpItemPool = GetItemPool(gpCurrItemPoolNode->sGridNo, 0);
						Assert( gpItemPool );
					}
					//remove node
					HideItemCursor( sGridNo );
					delete pIPCurr;
					pIPCurr = NULL;
					return;
				}
				pIPPrev = pIPCurr;
				pIPCurr = pIPCurr->next;
			}
		}
	}
}

void ShowSelectedItem()
{
	if( gpItemPool )
	{
		GetWorldItem(gpItemPool->iItemIndex).bVisible = INVISIBLE;
	}
}

void HideSelectedItem()
{
	if( gpItemPool )
	{
		GetWorldItem(gpItemPool->iItemIndex).bVisible = HIDDEN_ITEM;
	}
}

void SelectNextItemPool()
{
	if( !gpCurrItemPoolNode )
		return;
//remove the current hilight.
	if( gpItemPool )
	{
		MarkMapIndexDirty(GetWorldItem(gpItemPool->iItemIndex).sGridNo);
	}

	//go to the next node.  If at end of list, choose pIPHead
	if( gpCurrItemPoolNode->next )
		gpCurrItemPoolNode = gpCurrItemPoolNode->next;
	else
		gpCurrItemPoolNode = pIPHead;
	//get the item pool at this node's gridno.
	gpItemPool = GetItemPool(gpCurrItemPoolNode->sGridNo, 0);
	WORLDITEM& wi = GetWorldItem(gpItemPool->iItemIndex);
	MarkMapIndexDirty(wi.sGridNo);
	SpecifyItemToEdit(&wi.o, wi.sGridNo);
	if( gsItemGridNo != -1 )
	{
		CenterScreenAtMapIndex( gsItemGridNo );
	}
}

void SelectNextItemInPool()
{
	if( gpItemPool )
	{
		if( gpItemPool->pNext )
		{
			gpItemPool = gpItemPool->pNext;
		}
		else
		{
			gpItemPool = GetItemPool(GetWorldItem(gpItemPool->iItemIndex).sGridNo, 0);
		}
		WORLDITEM& wi = GetWorldItem(gpItemPool->iItemIndex);
		SpecifyItemToEdit(&wi.o, wi.sGridNo);
		MarkWorldDirty();
	}
}

void SelectPrevItemInPool()
{
	if (!gpItemPool) return;

	for (ITEM_POOL* i = GetItemPool(GetWorldItem(gpItemPool->iItemIndex).sGridNo, 0);; i = i->pNext)
	{
		if (i->pNext != gpItemPool && i->pNext != NULL) continue;

		gpItemPool = i;
		WORLDITEM& wi = GetWorldItem(gpItemPool->iItemIndex);
		SpecifyItemToEdit(&wi.o, wi.sGridNo);
		MarkWorldDirty();
		break;
	}
}


static void SelectNextItemOfType(UINT16 usItem);
static void SelectNextKeyOfType(UINT8 ubKeyID);
static void SelectNextPressureAction(void);
static void SelectNextTriggerWithFrequency(UINT16 usItem, INT8 bFrequency);


/* Finds and selects the next item when right clicking on an item type.
 * Only works if the item actually exists in the world. */
static void FindNextItemOfSelectedType(void)
{
	UINT16 usItem;
	usItem = eInfo.pusItemIndex[ eInfo.sSelItemIndex ];
	if( usItem == ACTION_ITEM || usItem == SWITCH )
	{
		if( eInfo.sSelItemIndex < PRESSURE_ACTION_ID )
		{
			INT8 const bFrequency = CalcItemFrequency(eInfo.sSelItemIndex);
			SelectNextTriggerWithFrequency( usItem, bFrequency );
		}
		else
		{
			SelectNextPressureAction();
		}
	}
	else if( GCM->getItem( usItem )->isKey() )
	{
		SelectNextKeyOfType( (UINT8)eInfo.sSelItemIndex );
	}
	else
	{
		SelectNextItemOfType( usItem );
	}
}


static void SelectNextItemOfType(UINT16 usItem)
{
	IPListNode *curr;
	if( gpItemPool )
	{
		curr = pIPHead;
		while( curr )
		{ //skip quickly to the same gridno as the item pool
			if (curr->sGridNo == GetWorldItem(gpItemPool->iItemIndex).sGridNo)
			{
				gpItemPool = gpItemPool->pNext;
				while( gpItemPool )
				{
					WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
					OBJECTTYPE& o  = wi.o;
					if (o.usItem == usItem)
					{
						SpecifyItemToEdit(&o, wi.sGridNo);
						CenterScreenAtMapIndex( gsItemGridNo );
						return; //success! (another item in same itempool)
					}
					gpItemPool = gpItemPool->pNext;
				}
				curr = curr->next;
				break;
			}
			curr = curr->next;
		}
		while( curr )
		{ //search to the end of the list
			gpItemPool = GetItemPool(curr->sGridNo, 0);
			while( gpItemPool )
			{
				WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
				OBJECTTYPE& o  = wi.o;
				if (o.usItem == usItem)
				{
					SpecifyItemToEdit(&o, wi.sGridNo);
					CenterScreenAtMapIndex( gsItemGridNo );
					return; //success! (found another item before reaching the end of the list)
				}
				gpItemPool = gpItemPool->pNext;
			}
			curr = curr->next;
		}
	}
	curr = pIPHead;
	while( curr )
	{ //search to the end of the list
		gpItemPool = GetItemPool(curr->sGridNo, 0);
		while( gpItemPool )
		{
			WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
			OBJECTTYPE& o  = wi.o;
			if (o.usItem == usItem)
			{
				SpecifyItemToEdit(&o, wi.sGridNo);
				CenterScreenAtMapIndex( gsItemGridNo );
				return; //success! (found first item in the list)
			}
			gpItemPool = gpItemPool->pNext;
		}
		curr = curr->next;
	}
}


static void SelectNextKeyOfType(UINT8 ubKeyID)
{
	IPListNode *curr;
	if( gpItemPool )
	{
		curr = pIPHead;
		while( curr )
		{ //skip quickly to the same gridno as the item pool
			if (curr->sGridNo == GetWorldItem(gpItemPool->iItemIndex).sGridNo)
			{
				gpItemPool = gpItemPool->pNext;
				while( gpItemPool )
				{
					WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
					OBJECTTYPE& o  = wi.o;
					if (GCM->getItem(o.usItem)->isKey() && o.ubKeyID == ubKeyID)
					{
						SpecifyItemToEdit(&o, wi.sGridNo);
						CenterScreenAtMapIndex( gsItemGridNo );
						return; //success! (another item in same itempool)
					}
					gpItemPool = gpItemPool->pNext;
				}
				curr = curr->next;
				break;
			}
			curr = curr->next;
		}
		while( curr )
		{ //search to the end of the list
			gpItemPool = GetItemPool(curr->sGridNo, 0);
			while( gpItemPool )
			{
				WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
				OBJECTTYPE& o  = wi.o;
				if (GCM->getItem(o.usItem)->isKey() && o.ubKeyID == ubKeyID)
				{
					SpecifyItemToEdit(&o, wi.sGridNo);
					CenterScreenAtMapIndex( gsItemGridNo );
					return; //success! (found another item before reaching the end of the list)
				}
				gpItemPool = gpItemPool->pNext;
			}
			curr = curr->next;
		}
	}
	curr = pIPHead;
	while( curr )
	{ //search to the end of the list
		gpItemPool = GetItemPool(curr->sGridNo, 0);
		while( gpItemPool )
		{
			WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
			OBJECTTYPE& o  = wi.o;
			if (GCM->getItem(o.usItem)->isKey() && o.ubKeyID == ubKeyID)
			{
				SpecifyItemToEdit(&o, wi.sGridNo);
				CenterScreenAtMapIndex( gsItemGridNo );
				return; //success! (found first item in the list)
			}
			gpItemPool = gpItemPool->pNext;
		}
		curr = curr->next;
	}
}


static void SelectNextTriggerWithFrequency(UINT16 usItem, INT8 bFrequency)
{
	IPListNode *curr;
	if( gpItemPool )
	{
		curr = pIPHead;
		while( curr )
		{ //skip quickly to the same gridno as the item pool
			if (curr->sGridNo == GetWorldItem(gpItemPool->iItemIndex).sGridNo)
			{
				gpItemPool = gpItemPool->pNext;
				while( gpItemPool )
				{
					WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
					OBJECTTYPE& o  = wi.o;
					if (o.usItem == usItem && o.bFrequency == bFrequency)
					{
						SpecifyItemToEdit(&o, wi.sGridNo);
						CenterScreenAtMapIndex( gsItemGridNo );
						return; //success! (another item in same itempool)
					}
					gpItemPool = gpItemPool->pNext;
				}
				curr = curr->next;
				break;
			}
			curr = curr->next;
		}
		while( curr )
		{ //search to the end of the list
			gpItemPool = GetItemPool(curr->sGridNo, 0);
			while( gpItemPool )
			{
				WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
				OBJECTTYPE& o  = wi.o;
				if (o.usItem == usItem && o.bFrequency == bFrequency)
				{
					SpecifyItemToEdit(&o, wi.sGridNo);
					CenterScreenAtMapIndex( gsItemGridNo );
					return; //success! (found another item before reaching the end of the list)
				}
				gpItemPool = gpItemPool->pNext;
			}
			curr = curr->next;
		}
	}
	curr = pIPHead;
	while( curr )
	{ //search to the end of the list
		gpItemPool = GetItemPool(curr->sGridNo, 0);
		while( gpItemPool )
		{
			WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
			OBJECTTYPE& o  = wi.o;
			if (o.usItem == usItem && o.bFrequency == bFrequency)
			{
				SpecifyItemToEdit(&o, wi.sGridNo);
				CenterScreenAtMapIndex( gsItemGridNo );
				return; //success! (found first item in the list)
			}
			gpItemPool = gpItemPool->pNext;
		}
		curr = curr->next;
	}
}


static void SelectNextPressureAction(void)
{
	IPListNode *curr;
	if( gpItemPool )
	{
		curr = pIPHead;
		while( curr )
		{ //skip quickly to the same gridno as the item pool
			if (curr->sGridNo == GetWorldItem(gpItemPool->iItemIndex).sGridNo)
			{
				gpItemPool = gpItemPool->pNext;
				while( gpItemPool )
				{
					WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
					OBJECTTYPE& o  = wi.o;
					if (o.usItem == ACTION_ITEM && o.bDetonatorType == BOMB_PRESSURE)
					{
						SpecifyItemToEdit(&o, wi.sGridNo);
						CenterScreenAtMapIndex( gsItemGridNo );
						return; //success! (another item in same itempool)
					}
					gpItemPool = gpItemPool->pNext;
				}
				curr = curr->next;
				break;
			}
			curr = curr->next;
		}
		while( curr )
		{ //search to the end of the list
			gpItemPool = GetItemPool(curr->sGridNo, 0);
			while( gpItemPool )
			{
				WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
				OBJECTTYPE& o  = wi.o;
				if (o.usItem == ACTION_ITEM && o.bDetonatorType == BOMB_PRESSURE)
				{
					SpecifyItemToEdit(&o, wi.sGridNo);
					CenterScreenAtMapIndex( gsItemGridNo );
					return; //success! (found another item before reaching the end of the list)
				}
				gpItemPool = gpItemPool->pNext;
			}
			curr = curr->next;
		}
	}
	curr = pIPHead;
	while( curr )
	{ //search to the end of the list
		gpItemPool = GetItemPool(curr->sGridNo, 0);
		while( gpItemPool )
		{
			WORLDITEM&  wi = GetWorldItem(gpItemPool->iItemIndex);
			OBJECTTYPE& o  = wi.o;
			if (o.usItem == ACTION_ITEM && o.bDetonatorType == BOMB_PRESSURE)
			{
				SpecifyItemToEdit(&o, wi.sGridNo);
				CenterScreenAtMapIndex( gsItemGridNo );
				return; //success! (found first item in the list)
			}
			gpItemPool = gpItemPool->pNext;
		}
		curr = curr->next;
	}
}


static UINT16 CountNumberOfItemPlacementsInWorld(UINT16 usItem, UINT16* pusQuantity)
{
	IPListNode *pIPCurr;
	INT16 num = 0;
	*pusQuantity = 0;
	pIPCurr = pIPHead;
	while( pIPCurr )
	{
		const ITEM_POOL* pItemPool = GetItemPool(pIPCurr->sGridNo, 0);
		while( pItemPool )
		{
			OBJECTTYPE const& o = GetWorldItem(pItemPool->iItemIndex).o;
			if (o.usItem == usItem)
			{
				num++;
				*pusQuantity += o.ubNumberOfObjects;
			}
			pItemPool = pItemPool->pNext;
		}
		pIPCurr = pIPCurr->next;
	}
	return num;
}


static UINT16 CountNumberOfItemsWithFrequency(UINT16 usItem, INT8 bFrequency)
{
	IPListNode *pIPCurr;
	UINT16 num = 0;
	pIPCurr = pIPHead;
	while( pIPCurr )
	{
		const ITEM_POOL* pItemPool = GetItemPool(pIPCurr->sGridNo, 0);
		while( pItemPool )
		{
			OBJECTTYPE const& o = GetWorldItem(pItemPool->iItemIndex).o;
			if (o.usItem == usItem && o.bFrequency == bFrequency)
			{
				num++;
			}
			pItemPool = pItemPool->pNext;
		}
		pIPCurr = pIPCurr->next;
	}
	return num;
}


static UINT16 CountNumberOfPressureActionsInWorld(void)
{
	IPListNode *pIPCurr;
	UINT16 num = 0;
	pIPCurr = pIPHead;
	while( pIPCurr )
	{
		const ITEM_POOL* pItemPool = GetItemPool(pIPCurr->sGridNo, 0);
		while( pItemPool )
		{
			OBJECTTYPE const& o = GetWorldItem(pItemPool->iItemIndex).o;
			if (o.usItem == ACTION_ITEM && o.bDetonatorType == BOMB_PRESSURE)
			{
				num++;
			}
			pItemPool = pItemPool->pNext;
		}
		pIPCurr = pIPCurr->next;
	}
	return num;
}


static UINT16 CountNumberOfKeysOfTypeInWorld(UINT8 ubKeyID);


//Simply counts the number of items in the world.  This is used for display purposes.
static UINT16 CountNumberOfEditorPlacementsInWorld(UINT16 usEInfoIndex, UINT16* pusQuantity)
{
	UINT16 usNumPlacements;
	if( eInfo.uiItemType == TBAR_MODE_ITEM_TRIGGERS )
	{	//find identical items with same frequency
		if( usEInfoIndex < PRESSURE_ACTION_ID )
		{
			INT8 bFrequency = CalcItemFrequency(usEInfoIndex);
			usNumPlacements = CountNumberOfItemsWithFrequency( eInfo.pusItemIndex[usEInfoIndex], bFrequency );
			*pusQuantity = usNumPlacements;
		}
		else
		{
			usNumPlacements = CountNumberOfPressureActionsInWorld();
			*pusQuantity = usNumPlacements;
		}
	}
	else if( eInfo.uiItemType == TBAR_MODE_ITEM_KEYS )
	{
		usNumPlacements = CountNumberOfKeysOfTypeInWorld( (UINT8)usEInfoIndex );
		*pusQuantity = usNumPlacements;
	}
	else
	{
		usNumPlacements = CountNumberOfItemPlacementsInWorld( eInfo.pusItemIndex[ usEInfoIndex], pusQuantity );
	}
	return usNumPlacements;
}


static UINT16 CountNumberOfKeysOfTypeInWorld(UINT8 ubKeyID)
{
	IPListNode *pIPCurr;
	INT16 num = 0;
	pIPCurr = pIPHead;
	while( pIPCurr )
	{
		const ITEM_POOL* pItemPool = GetItemPool(pIPCurr->sGridNo, 0);
		while( pItemPool )
		{
			OBJECTTYPE const& o = GetWorldItem(pItemPool->iItemIndex).o;
			if (GCM->getItem(o.usItem)->isKey() && o.ubKeyID == ubKeyID)
			{
				num++;
			}
			pItemPool = pItemPool->pNext;
		}
		pIPCurr = pIPCurr->next;
	}
	return num;
}


void DisplayItemStatistics()
{
	if (!eInfo.fActive)      return;
	if (!eInfo.pusItemIndex) return;

	// If there is nothing else currently highlited by the mouse, use the selected item.
	INT16          const highlited  = eInfo.sHilitedItemIndex;
	INT16          const idx        = highlited != -1 ? highlited : eInfo.sSelItemIndex;
	UINT8          const foreground = idx == highlited ? FONT_LTRED : FONT_YELLOW;
	ST::string item_name  = GCM->getItem(eInfo.pusItemIndex[idx])->getName();
	DisplayWrappedString(2, EDITOR_TASKBAR_POS_Y + 41, 97, 2, SMALLCOMPFONT, foreground, item_name, FONT_BLACK, CENTER_JUSTIFIED);
}
