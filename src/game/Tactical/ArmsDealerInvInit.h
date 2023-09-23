#ifndef _ARMS_DEALER_INV_INIT__H_
#define	_ARMS_DEALER_INV_INIT__H_

#include "Arms_Dealer.h"
#include "Types.h"


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


INT8 GetDealersMaxItemAmount(ArmsDealerID, UINT16 usItemIndex);

UINT8   ChanceOfItemTransaction(ArmsDealerID, UINT16 usItemIndex, BOOLEAN fDealerSelling, BOOLEAN fUsed);
BOOLEAN ItemTransactionOccurs(ArmsDealerID, UINT16 usItemIndex, BOOLEAN fDealerSelling, BOOLEAN fUsed);
UINT8 DetermineInitialInvItems(ArmsDealerID, UINT16 usItemIndex, UINT8 ubChances, BOOLEAN fUsed);
UINT8 HowManyItemsAreSold(ArmsDealerID, UINT16 usItemIndex, UINT8 ubNumInStock, BOOLEAN fUsed);
UINT8 HowManyItemsToReorder(UINT8 ubWanted, UINT8 ubStillHave);

struct STORE_INVENTORY;
int BobbyRayItemQsortCompare(const STORE_INVENTORY& pArg1, const STORE_INVENTORY& pArg2);
int ArmsDealerItemQsortCompare(const void *pArg1, const void *pArg2);
int CompareItemsForSorting( UINT16 usItem1Index, UINT16 usItem2Index, UINT8 ubItem1Quality, UINT8 ubItem2Quality );
BOOLEAN CanDealerItemBeSoldUsed( UINT16 usItemIndex );

#endif
