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
void SetupStoreInventory( std::vector<STORE_INVENTORY>& pInventoryArray, BOOLEAN fUsed )
{
	UINT8 ubNumBought;

	//loop through all items BR can stock to init a starting quantity on hand
	for(auto& inventoryItem : pInventoryArray)
	{
		auto item = GCM->getItem(inventoryItem.usItemIndex);

		Assert(item != NULL);

		int maxAmount = fUsed ? GCM->getBobbyRayUsedInventory()->getMaxItemAmount(item) : GCM->getBobbyRayNewInventory()->getMaxItemAmount(item);
		ubNumBought = DetermineInitialInvItems(ARMS_DEALER_BOBBYR, inventoryItem.usItemIndex, maxAmount, fUsed);
		if ( ubNumBought > 0)
		{
			// If doing used items
			if( fUsed )
			{
				// then there should only be 1 of them, and it's damaged
				inventoryItem.ubQtyOnHand = 1;
				inventoryItem.ubItemQuality = 20 + (UINT8) Random( 60 );
			}
			else	// new
			{
				inventoryItem.ubQtyOnHand = ubNumBought;
				inventoryItem.ubItemQuality = 100;
			}
		}
		else
		{
			// doesn't sell / not in stock
			inventoryItem.ubQtyOnHand = 0;
			inventoryItem.ubItemQuality = 0;
		}
	}
}
