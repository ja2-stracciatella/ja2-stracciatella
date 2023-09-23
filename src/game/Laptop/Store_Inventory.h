#ifndef __STORE_INVENTORY_H_
#define __STORE_INVENTORY_H_

#include "Types.h"


struct STORE_INVENTORY
{
	UINT16  usItemIndex; //Index into the item table
	UINT8   ubQtyOnHand;
	UINT8   ubQtyOnOrder; //The number of items on order
	UINT8   ubItemQuality; // the % damaged listed from 0 to 100
	BOOLEAN fPreviouslyEligible; // whether or not dealer has been eligible to sell this item in days prior to today
};


//Enums used for the access the MAX dealers array
enum
{
	BOBBY_RAY_NEW,
	BOBBY_RAY_USED,
	BOBBY_RAY_LISTS,
};

void SetupStoreInventory( std::vector<STORE_INVENTORY>& pInventoryArray, BOOLEAN fUsed );

#endif
