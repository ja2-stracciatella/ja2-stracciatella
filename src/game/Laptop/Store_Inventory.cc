#include "Types.h"
#include "Store_Inventory.h"
#include "Random.h"
#include "Debug.h"
#include "LaptopSave.h"
#include "ArmsDealerInvInit.h"

#include "ContentManager.h"
#include "DealerInventory.h"
#include "GameInstance.h"
#include "ItemModel.h"


// SetupStoreInventory sets up the initial quantity on hand for all of Bobby Ray's inventory items
void SetupStoreInventory( STORE_INVENTORY *pInventoryArray, BOOLEAN fUsed )
{
	UINT16 i;
	UINT16 usItemIndex;
	UINT8 ubNumBought;

	//loop through all items BR can stock to init a starting quantity on hand
	for(i = 0; i < LaptopSaveInfo.usInventoryListLength[fUsed]; i++)
	{
		usItemIndex = pInventoryArray[ i ].usItemIndex;
		Assert(usItemIndex < MAXITEMS);

		const ItemModel *item = GCM->getItem(usItemIndex);
		int maxAmount = fUsed ? GCM->getBobbyRayUsedInventory()->getMaxItemAmount(item) : GCM->getBobbyRayNewInventory()->getMaxItemAmount(item);
		ubNumBought = DetermineInitialInvItems(ARMS_DEALER_BOBBYR, usItemIndex, maxAmount, fUsed);
		if ( ubNumBought > 0)
		{
			// If doing used items
			if( fUsed )
			{
				// then there should only be 1 of them, and it's damaged
				pInventoryArray[i].ubQtyOnHand = 1;
				pInventoryArray[i].ubItemQuality = 20 + (UINT8) Random( 60 );
			}
			else	// new
			{
				pInventoryArray[i].ubQtyOnHand = ubNumBought;
				pInventoryArray[i].ubItemQuality = 100;
			}
		}
		else
		{
			// doesn't sell / not in stock
			pInventoryArray[i].ubQtyOnHand = 0;
			pInventoryArray[i].ubItemQuality = 0;
		}
	}
}
