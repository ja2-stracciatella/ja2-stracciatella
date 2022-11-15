#ifndef _ARMS_DEALER_INV_INIT__H_
#define	_ARMS_DEALER_INV_INIT__H_

#include "Arms_Dealer.h"
#include "Item_Types.h"


#define LAST_DEALER_ITEM		-1
#define NO_DEALER_ITEM			0

// item suitability categories for dealer inventory initialization, virtual customer sales, and re-ordering
#define ITEM_SUITABILITY_NONE		0
#define ITEM_SUITABILITY_LOW		1
#define ITEM_SUITABILITY_MEDIUM	2
#define ITEM_SUITABILITY_HIGH		3
#define ITEM_SUITABILITY_ALWAYS	4

#define DEALER_BUYING			0
#define DEALER_SELLING			1


INT8 GetDealersMaxItemAmount(ArmsDealerID, ItemId usItemIndex);

UINT8   ChanceOfItemTransaction(ArmsDealerID, ItemId usItemIndex, BOOLEAN fDealerSelling, BOOLEAN fUsed);
BOOLEAN ItemTransactionOccurs(ArmsDealerID, ItemId usItemIndex, BOOLEAN fDealerSelling, BOOLEAN fUsed);
UINT8 DetermineInitialInvItems(ArmsDealerID, ItemId usItemIndex, UINT8 ubChances, BOOLEAN fUsed);
UINT8 HowManyItemsAreSold(ArmsDealerID, ItemId usItemIndex, UINT8 ubNumInStock, BOOLEAN fUsed);
UINT8 HowManyItemsToReorder(UINT8 ubWanted, UINT8 ubStillHave);

int BobbyRayItemQsortCompare(const void *pArg1, const void *pArg2);
int ArmsDealerItemQsortCompare(const void *pArg1, const void *pArg2);
int CompareItemsForSorting( ItemId usItem1Index, ItemId usItem2Index, UINT8 ubItem1Quality, UINT8 ubItem2Quality );
BOOLEAN CanDealerItemBeSoldUsed( ItemId usItemIndex );

#endif
