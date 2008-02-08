#ifndef __BOBBYR_H
#define __BOBBYR_H

#include "Item_Types.h"
#include "Store_Inventory.h"


BOOLEAN EnterBobbyR(void);
void ExitBobbyR(void);
void HandleBobbyR(void);
void RenderBobbyR(void);


#define		BOBBYR_BACKGROUND_WIDTH			125
#define		BOBBYR_BACKGROUND_HEIGHT		100
#define		BOBBYR_NUM_HORIZONTAL_TILES	4
#define		BOBBYR_NUM_VERTICAL_TILES		4

#define		BOBBYR_GRIDLOC_X		LAPTOP_SCREEN_UL_X + 4
#define		BOBBYR_GRIDLOC_Y		LAPTOP_SCREEN_WEB_UL_Y + 45


extern	UINT32	guiLastBobbyRayPage;


BOOLEAN DrawBobbyRWoodBackground(void);
BOOLEAN DeleteBobbyRWoodBackground(void);
BOOLEAN InitBobbyRWoodBackground(void);
void DailyUpdateOfBobbyRaysNewInventory(void);
void DailyUpdateOfBobbyRaysUsedInventory(void);
void AddFreshBobbyRayInventory( UINT16 usItemIndex );
void InitBobbyRayInventory(void);
void CancelAllPendingBRPurchaseOrders(void);
INT16 GetInventorySlotForItem(STORE_INVENTORY *pInventoryArray, UINT16 usItemIndex, BOOLEAN fUsed);


#endif
