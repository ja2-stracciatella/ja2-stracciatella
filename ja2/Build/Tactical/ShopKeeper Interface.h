#ifndef _SHOPKEEPER_INTERFACE__H_
#define _SHOPKEEPER_INTERFACE__H_

#include "Store Inventory.h"





#define		SKI_NUM_TRADING_INV_SLOTS						12
#define		SKI_NUM_TRADING_INV_ROWS						2
#define		SKI_NUM_TRADING_INV_COLS						6












//Enums used for when the user clicks on an item and the item goes to..
enum
{
	ARMS_DEALER_INVENTORY,
	ARMS_DEALER_OFFER_AREA,
	PLAYERS_OFFER_AREA,
	PLAYERS_INVENTORY,
};

#define	ARMS_INV_ITEM_SELECTED										0x00000001			// The item has been placed into the offer area
//#define	ARMS_INV_PLAYERS_ITEM_SELECTED						0x00000002			// The source location for the item has been selected
#define	ARMS_INV_PLAYERS_ITEM_HAS_VALUE						0x00000004			// The Players item is worth something to this dealer
//#define	ARMS_INV_ITEM_HIGHLIGHTED									0x00000008			// If the items is highlighted
#define	ARMS_INV_ITEM_NOT_REPAIRED_YET						0x00000010			// The item is in for repairs but not repaired yet
#define	ARMS_INV_ITEM_REPAIRED										0x00000020			// The item is repaired
#define	ARMS_INV_JUST_PURCHASED										0x00000040			// The item was just purchased
#define	ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED	0x00000080			// The Players item has been evaluated


typedef struct
{
	BOOLEAN			fActive;
	INT16				sItemIndex;
	UINT32			uiFlags;
	OBJECTTYPE	ItemObject;
	UINT8				ubLocationOfObject;					//An enum value for the location of the item ( either in the arms dealers inventory, one of the offer areas or in the users inventory)
	INT8				bSlotIdInOtherLocation;

	UINT8				ubIdOfMercWhoOwnsTheItem;
	UINT32			uiItemPrice;								//Only used for the players item that have been evaluated

	INT16				sSpecialItemElement;				// refers to which special item element an item in a dealer's inventory area
																					// occupies.  -1 Means the item is "perfect" and has no associated special item.

} INVENTORY_IN_SLOT;



extern INVENTORY_IN_SLOT	PlayersOfferArea[ SKI_NUM_TRADING_INV_SLOTS ];
extern INT32	giShopKeepDialogueEventinProgress;


//extern	BOOLEAN		gfRedrawSkiScreen;

enum
{
	SKI_DIRTY_LEVEL0,	// no redraw
	SKI_DIRTY_LEVEL1,	// redraw only items
	SKI_DIRTY_LEVEL2, // redraw everything
};

extern	UINT8			gubSkiDirtyLevel;

extern	OBJECTTYPE					*gpHighLightedItemObject;


extern	INVENTORY_IN_SLOT		gMoveingItem;


extern	OBJECTTYPE	*pShopKeeperItemDescObject;


UINT32	ShopKeeperScreenInit( void );
UINT32	ShopKeeperScreenHandle( void );
UINT32	ShopKeeperScreenShutdown( void );



void			EnterShopKeeperInterfaceScreen( UINT8	ubArmsDealer );


void			DrawHatchOnInventory( UINT32 uiSurface, UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT16 usHeight );
BOOLEAN		ShouldSoldierDisplayHatchOnItem( UINT8	ubProfileID, INT16 sSlotNum );
INT8			AddItemToPlayersOfferArea( UINT8 ubProfileID, INVENTORY_IN_SLOT* pInvSlot, INT8	bSlotIdInOtherLocation );
void			ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack( UINT8 bExitValue );
void			ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack( UINT8 bExitValue );

void			SkiHelpTextDoneCallBack( void );
void			SetSkiCursor( UINT16	usCursor );

void			InitShopKeeperSubTitledText( STR16 pString );

void			AddItemToPlayersOfferAreaAfterShopKeeperOpen( OBJECTTYPE	*pItemObject, INT8 bPreviousInvPos );

void			BeginSkiItemPointer( UINT8 ubSource, INT8 bSlotNum, BOOLEAN fOfferToDealerFirst );

void			DeleteShopKeeperItemDescBox();

BOOLEAN		CanMercInteractWithSelectedShopkeeper( SOLDIERTYPE *pSoldier );

void			DealerGetsBribed( UINT8 ubProfileId, UINT32 uiMoneyAmount );


#ifdef JA2TESTVERSION
void AddShopkeeperToGridNo( UINT8 ubProfile, INT16 sGridNo );
#endif

void			RestrictSkiMouseCursor();

#endif
