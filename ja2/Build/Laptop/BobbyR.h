#ifndef __BOBBYR_H
#define __BOBBYR_H

#include "Item_Types.h"
#include "Store_Inventory.h"


void GameInitBobbyR();
BOOLEAN EnterBobbyR();
void ExitBobbyR();
void HandleBobbyR();
void RenderBobbyR();


#define		BOBBYR_BACKGROUND_WIDTH			125
#define		BOBBYR_BACKGROUND_HEIGHT		100
#define		BOBBYR_NUM_HORIZONTAL_TILES	4
#define		BOBBYR_NUM_VERTICAL_TILES		4

#define		BOBBYR_GRIDLOC_X		LAPTOP_SCREEN_UL_X + 4
#define		BOBBYR_GRIDLOC_Y		LAPTOP_SCREEN_WEB_UL_Y + 45


/*
extern	UINT16	gusFirstGunIndex;
extern	UINT16	gusLastGunIndex;
extern	UINT8		gubNumGunPages;

extern	UINT16	gusFirstAmmoIndex;
extern	UINT16	gusLastAmmoIndex;
extern	UINT8		gubNumAmmoPages;

extern	UINT16	gusFirstMiscIndex;
extern	UINT16	gusLastMiscIndex;
extern	UINT8		gubNumMiscPages;

extern	UINT16  gusFirstArmourIndex;
extern	UINT16  gusLastArmourIndex;
extern	UINT8		gubNumArmourPages;

extern	UINT16  gusFirstUsedIndex;
extern	UINT16  gusLastUsedIndex;
extern	UINT8		gubNumUsedPages;
*/

extern	UINT32	guiLastBobbyRayPage;


//BOOLEAN WebPageTileBackground(UINT8 ubNumX, UINT8 ubNumY, UINT16 usWidth, UINT16 usHeight, UINT32 uiBackGround);
BOOLEAN DrawBobbyRWoodBackground();
BOOLEAN DeleteBobbyRWoodBackground();
BOOLEAN InitBobbyRWoodBackground();
void DailyUpdateOfBobbyRaysNewInventory();
void DailyUpdateOfBobbyRaysUsedInventory();
void OrderBobbyRItem(UINT16 usItemIndex);
void AddFreshBobbyRayInventory( UINT16 usItemIndex );
void InitBobbyRayInventory();
BOOLEAN InitBobbyRayNewInventory();
BOOLEAN InitBobbyRayUsedInventory();
UINT8 HowManyBRItemsToOrder(UINT16 usItemIndex, UINT8 ubCurrentlyOnHand, UINT8 ubBobbyRayNewUsed);
void CancelAllPendingBRPurchaseOrders(void);
INT16 GetInventorySlotForItem(STORE_INVENTORY *pInventoryArray, UINT16 usItemIndex, BOOLEAN fUsed);


#endif
