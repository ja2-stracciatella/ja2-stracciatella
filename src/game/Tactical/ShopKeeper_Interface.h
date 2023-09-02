#ifndef _SHOPKEEPER_INTERFACE__H_
#define _SHOPKEEPER_INTERFACE__H_

#include "Item_Types.h"
#include "JA2Types.h"
#include "MessageBoxScreen.h"
#include "ScreenIDs.h"

#include <string_theory/string>


//Enums used for when the user clicks on an item and the item goes to..
enum
{
	ARMS_DEALER_INVENTORY,
	ARMS_DEALER_OFFER_AREA,
	PLAYERS_OFFER_AREA,
	PLAYERS_INVENTORY,
};

#define ARMS_INV_ITEM_SELECTED				0x00000001 // The item has been placed into the offer area
//#define ARMS_INV_PLAYERS_ITEM_SELECTED		0x00000002 // The source location for the item has been selected
#define ARMS_INV_PLAYERS_ITEM_HAS_VALUE		0x00000004 // The Players item is worth something to this dealer
//#define ARMS_INV_ITEM_HIGHLIGHTED			0x00000008 // If the items is highlighted
#define ARMS_INV_ITEM_NOT_REPAIRED_YET			0x00000010 // The item is in for repairs but not repaired yet
#define ARMS_INV_ITEM_REPAIRED				0x00000020 // The item is repaired
#define ARMS_INV_JUST_PURCHASED			0x00000040 // The item was just purchased
#define ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED	0x00000080 // The Players item has been evaluated


struct INVENTORY_IN_SLOT
{
	BOOLEAN    fActive;
	INT16      sItemIndex;
	UINT32     uiFlags;
	OBJECTTYPE ItemObject;
	UINT8      ubLocationOfObject;	//An enum value for the location of the item
					// ( either in the arms dealers inventory, one of the
					// offer areas or in the users inventory)
	INT8       bSlotIdInOtherLocation;

	UINT8      ubIdOfMercWhoOwnsTheItem;
	UINT32     uiItemPrice; //Only used for the players item that have been evaluated

	INT16      sSpecialItemElement;	// refers to which special item element an item in a dealer's inventory area
					// occupies.  -1 Means the item is "perfect" and has no associated special item.
};


enum
{
	SKI_DIRTY_LEVEL0, // no redraw
	SKI_DIRTY_LEVEL1, // redraw only items
	SKI_DIRTY_LEVEL2, // redraw everything
};

extern UINT8 gubSkiDirtyLevel;

extern const OBJECTTYPE *gpHighLightedItemObject;


extern INVENTORY_IN_SLOT gMoveingItem;


extern OBJECTTYPE *pShopKeeperItemDescObject;


void     ShopKeeperScreenInit(void);
ScreenID ShopKeeperScreenHandle(void);
void     ShopKeeperScreenShutdown(void);


void EnterShopKeeperInterfaceScreen( UINT8 ubArmsDealer );


void DrawHatchOnInventory(SGPVSurface* dst, UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT16 usHeight);
BOOLEAN ShouldSoldierDisplayHatchOnItem( UINT8	ubProfileID, INT16 sSlotNum );
void ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack(MessageBoxReturnValue);
void ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack(MessageBoxReturnValue);

void SetSkiCursor( UINT16 usCursor );

void InitShopKeeperSubTitledText(const ST::string& str);

void AddItemToPlayersOfferAreaAfterShopKeeperOpen( OBJECTTYPE	*pItemObject, INT8 bPreviousInvPos );

void BeginSkiItemPointer( UINT8 ubSource, INT8 bSlotNum, BOOLEAN fOfferToDealerFirst );

void DeleteShopKeeperItemDescBox(void);

BOOLEAN CanMercInteractWithSelectedShopkeeper(const SOLDIERTYPE* s);

void RestrictSkiMouseCursor(void);

void DoSkiMessageBox(const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags ubFlags, MSGBOX_CALLBACK ReturnCallback);
void StartSKIDescriptionBox(void);

#endif
