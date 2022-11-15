#include <stdexcept>

#include "Types.h"
#include "ArmsDealerInvInit.h"
#include "Item_Types.h"
#include "Arms_Dealer_Init.h"
#include "Debug.h"
#include "Weapons.h"
#include "Campaign.h"
#include "BobbyR.h"
#include "LaptopSave.h"
#include "ShopKeeper_Interface.h"
#include "Random.h"
#include "Items.h"

#include "CalibreModel.h"
#include "ContentManager.h"
#include "DealerInventory.h"
#include "GameInstance.h"
#include "MagazineModel.h"
#include "WeaponModels.h"

struct ITEM_SORT_ENTRY
{
	UINT32  uiItemClass;
	UINT8   ubWeaponClass;
	BOOLEAN fAllowUsed;
};


// This table controls the order items appear in inventory at BR's and dealers,
// and which kinds of items are sold used
static ITEM_SORT_ENTRY const DealerItemSortInfo[] =
{
	//  item class       weapon class  sold used?
	{ IC_GUN,            HANDGUNCLASS, TRUE  },
	{ IC_GUN,            SHOTGUNCLASS, TRUE  },
	{ IC_GUN,            SMGCLASS,     TRUE  },
	{ IC_GUN,            RIFLECLASS,   TRUE  },
	{ IC_GUN,            MGCLASS,      FALSE },
	{ IC_LAUNCHER,       NOGUNCLASS,   FALSE },
	{ IC_AMMO,           NOGUNCLASS,   FALSE },
	{ IC_GRENADE,        NOGUNCLASS,   FALSE },
	{ IC_BOMB,           NOGUNCLASS,   FALSE },
	{ IC_BLADE,          NOGUNCLASS,   FALSE },
	{ IC_THROWING_KNIFE, NOGUNCLASS,   FALSE },
	{ IC_PUNCH,          NOGUNCLASS,   FALSE },
	{ IC_ARMOUR,         NOGUNCLASS,   TRUE  },
	{ IC_FACE,           NOGUNCLASS,   TRUE  },
	{ IC_MEDKIT,         NOGUNCLASS,   FALSE },
	{ IC_KIT,            NOGUNCLASS,   FALSE },
	{ IC_MISC,           NOGUNCLASS,   TRUE  },
	{ IC_THROWN,         NOGUNCLASS,   FALSE },
	{ IC_KEY,            NOGUNCLASS,   FALSE },
	{ IC_MONEY,          NOGUNCLASS,   FALSE },

	{ IC_NONE,           NOGUNCLASS,   FALSE }  // marks end of list
};


INT8 GetDealersMaxItemAmount(ArmsDealerID const ubDealerID, ItemId const usItemIndex)
{
	return GCM->getDealerInventory(ubDealerID)->getMaxItemAmount(GCM->getItem(usItemIndex));
}


static UINT8 GetCurrentSuitabilityForItem(ArmsDealerID const bArmsDealer, ItemId const usItemIndex)
{
	UINT8 ubItemCoolness;
	UINT8 ubMinCoolness, ubMaxCoolness;


	// item suitability varies with the player's maximum progress through the game.  The farther he gets, the better items
	// we make available.  Weak items become more and more infrequent later in the game, although they never quite vanish.

	// items illegal in this game are unsuitable [this checks guns vs. current GunSet!]
	if (!ItemIsLegal( usItemIndex ))
	{
		return(ITEM_SUITABILITY_NONE);
	}

	// items normally not sold at shops are unsuitable
	if ( GCM->getItem(usItemIndex)->getFlags() & ITEM_NOT_BUYABLE )
	{
		return(ITEM_SUITABILITY_NONE);
	}


	ubItemCoolness = GCM->getItem(usItemIndex)->getCoolness();

	if (ubItemCoolness == 0)
	{
		// items without a coolness rating can't be sold to the player by shopkeepers
		return(ITEM_SUITABILITY_NONE);
	}

	// the following staple items are always deemed highly suitable regardless of player's progress:
	switch (usItemIndex.inner())
	{
		case CLIP38_6.inner():
		case CLIP9_15.inner():
		case CLIP9_30.inner():
		case CLIP357_6.inner():
		case CLIP357_9.inner():
		case CLIP45_7.inner():
		case CLIP45_30.inner():
		case CLIP12G_7.inner():
		case CLIP12G_7_BUCKSHOT.inner():
		case CLIP545_30_HP.inner():
		case CLIP556_30_HP.inner():
		case CLIP762W_10_HP.inner():
		case CLIP762W_30_HP.inner():
		case CLIP762N_5_HP.inner():
		case CLIP762N_20_HP.inner():

		case FIRSTAIDKIT.inner():
		case MEDICKIT.inner():
		case TOOLKIT.inner():
		case LOCKSMITHKIT.inner():

		case CANTEEN.inner():
		case CROWBAR.inner():
		case JAR.inner():
		case JAR_ELIXIR.inner():
		case JAR_CREATURE_BLOOD.inner():

			return(ITEM_SUITABILITY_ALWAYS);
	}

	switch (bArmsDealer)
	{
		case ARMS_DEALER_BOBBYR:
		case ARMS_DEALER_TONY:
		case ARMS_DEALER_DEVIN:
			break;

		default:
			// All the other dealers have very limited inventories, so their
			// suitability remains constant at all times in game
			return ITEM_SUITABILITY_HIGH;
	}

	// figure out the appropriate range of coolness based on player's maximum progress so far

	ubMinCoolness = HighestPlayerProgressPercentage() / 10;
	ubMaxCoolness = ( HighestPlayerProgressPercentage() / 10 ) + 1;

	// Tony has the better stuff sooner (than Bobby R's)
	if (bArmsDealer == ARMS_DEALER_TONY)
	{
		ubMinCoolness += 1;
		ubMaxCoolness += 1;
	}
	else if (bArmsDealer == ARMS_DEALER_DEVIN)
	{
		// almost everything Devin sells is pretty cool (4+), so gotta apply a minimum or he'd have nothing early on
		if ( ubMinCoolness < 3 )
		{
			ubMinCoolness = 3;
			ubMaxCoolness = 4;
		}
	}

	ubMinCoolness = std::clamp(int(ubMinCoolness), 1, 9);
	ubMaxCoolness = std::clamp(int(ubMaxCoolness), 2, 10);

	// if item is too cool for current level of progress
	if (ubItemCoolness > ubMaxCoolness)
	{
		return(ITEM_SUITABILITY_NONE);
	}

	// if item is exactly within the current coolness window
	if ((ubItemCoolness >= ubMinCoolness) && (ubItemCoolness <= ubMaxCoolness))
	{
		return(ITEM_SUITABILITY_HIGH);
	}

	// if item is still relatively close to low end of the window
	if ((ubItemCoolness + 2) >= ubMinCoolness)
	{
		return(ITEM_SUITABILITY_MEDIUM);
	}

	// item is way uncool for player's current progress, but it's still possible for it to make an appearance
	return(ITEM_SUITABILITY_LOW);
}


UINT8 ChanceOfItemTransaction(ArmsDealerID const bArmsDealer, ItemId const usItemIndex, BOOLEAN const fDealerIsSelling, BOOLEAN const fUsed)
{
	UINT8 ubItemCoolness;
	UINT8 ubChance = 0;

	// make sure dealers don't carry used items that they shouldn't
	if ( fUsed && !fDealerIsSelling && !CanDealerItemBeSoldUsed( usItemIndex ) )
		return( 0 );

	// Bobby Ray has an easier time getting resupplied than the local dealers do
	BOOLEAN const fBobbyRay = bArmsDealer == ARMS_DEALER_BOBBYR;

	ubItemCoolness = GCM->getItem(usItemIndex)->getCoolness();

	switch (GetCurrentSuitabilityForItem( bArmsDealer, usItemIndex ) )
	{
		case ITEM_SUITABILITY_NONE:
			if (fDealerIsSelling)
			{
				// dealer always gets rid of stuff that is too advanced or inappropriate ASAP
				ubChance = 100;
			}
			else // dealer is buying
			{
				// can't get these at all
				ubChance = 0;
			}
			break;

		case ITEM_SUITABILITY_LOW:
			ubChance = (fBobbyRay) ? 25 : 15;
			break;

		case ITEM_SUITABILITY_MEDIUM:
			ubChance = (fBobbyRay) ? 50 : 30;
			break;

		case ITEM_SUITABILITY_HIGH:
			ubChance = (fBobbyRay) ? 75 : 50;
			break;

		case ITEM_SUITABILITY_ALWAYS:
			if (fDealerIsSelling)
			{
				// sells just like suitability high
				ubChance = 75;
			}
			else // dealer is buying
			{
				// dealer can always get a (re)supply of these
				ubChance = 100;
			}
			break;

		default:
			SLOGA("ChanceOfItemTransaction: invalid item suitability");
			break;
	}


	// if there's any uncertainty
	if ((ubChance > 0) && (ubChance < 100))
	{
		// cooler items sell faster
		if (fDealerIsSelling)
		{
			ubChance += (5 * ubItemCoolness);

			// ARM: New - keep stuff on the shelves longer
			ubChance /= 2;
		}

		// used items are traded more rarely
		if (fUsed)
		{
			ubChance /= 2;
		}
	}


	return(ubChance);
}


BOOLEAN ItemTransactionOccurs(ArmsDealerID const bArmsDealer, ItemId const usItemIndex, BOOLEAN const fDealerIsSelling, BOOLEAN const fUsed)
{
	UINT8 ubChance;
	INT16 sInventorySlot;


	ubChance = ChanceOfItemTransaction( bArmsDealer, usItemIndex, fDealerIsSelling, fUsed );

	// if the dealer is buying, and a chance exists (i.e. the item is "eligible")
	if (!fDealerIsSelling && (ubChance > 0))
	{
		// mark it as such
		if (bArmsDealer == ARMS_DEALER_BOBBYR)
		{
			STORE_INVENTORY* pInventory = fUsed ? LaptopSaveInfo.BobbyRayUsedInventory : LaptopSaveInfo.BobbyRayInventory;
			sInventorySlot = GetInventorySlotForItem(pInventory, usItemIndex, fUsed);
			if (sInventorySlot == -1)
			{
				return(FALSE);
			}
			pInventory[ sInventorySlot ].fPreviouslyEligible = TRUE;
		}
		else
		{
			gArmsDealersInventory[ bArmsDealer ][ usItemIndex.inner() ].fPreviouslyEligible = TRUE;
		}
	}

	// roll to see if a transaction occurs
	if (Random(100) < ubChance)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}


UINT8 DetermineInitialInvItems(ArmsDealerID const bArmsDealerID, ItemId const usItemIndex, UINT8 const ubChances, BOOLEAN const fUsed)
{
	UINT8 ubNumBought;
	UINT8 ubCnt;

	// initial inventory is now rolled for one item at a time, instead of one type at a time, to improve variety
	ubNumBought = 0;
	for (ubCnt = 0; ubCnt < ubChances; ubCnt++)
	{
		if (ItemTransactionOccurs( bArmsDealerID, usItemIndex, DEALER_BUYING, fUsed))
		{
			ubNumBought++;
		}
	}

	return( ubNumBought );
}


UINT8 HowManyItemsAreSold(ArmsDealerID const bArmsDealerID, ItemId const usItemIndex, UINT8 const ubNumInStock, BOOLEAN const fUsed)
{
	UINT8 ubNumSold;
	UINT8 ubCnt;

	// items are now virtually "sold" one at a time
	ubNumSold = 0;
	for (ubCnt = 0; ubCnt < ubNumInStock; ubCnt++)
	{
		if (ItemTransactionOccurs( bArmsDealerID, usItemIndex, DEALER_SELLING, fUsed))
		{
			ubNumSold++;
		}
	}

	return( ubNumSold );
}



UINT8 HowManyItemsToReorder(UINT8 ubWanted, UINT8 ubStillHave)
{
	UINT8 ubNumReordered;

	Assert(ubStillHave <= ubWanted);

	ubNumReordered = ubWanted - ubStillHave;

	//randomize the amount. 33% of the time we add to it, 33% we subtract from it, rest leave it alone
	switch (Random(3))
	{
		case 0:
			ubNumReordered += ubNumReordered / 2;
			break;
		case 1:
			ubNumReordered -= ubNumReordered / 2;
			break;
	}

	return(ubNumReordered);
}



int BobbyRayItemQsortCompare(const void *pArg1, const void *pArg2)
{
	ItemId usItem1Index;
	ItemId usItem2Index;
	UINT8  ubItem1Quality;
	UINT8  ubItem2Quality;

	usItem1Index = ( ( STORE_INVENTORY * ) pArg1 ) -> usItemIndex;
	usItem2Index = ( ( STORE_INVENTORY * ) pArg2 ) -> usItemIndex;

	ubItem1Quality = ( ( STORE_INVENTORY * ) pArg1 ) -> ubItemQuality;
	ubItem2Quality = ( ( STORE_INVENTORY * ) pArg2 ) -> ubItemQuality;

	return( CompareItemsForSorting( usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality ) );
}



int ArmsDealerItemQsortCompare(const void *pArg1, const void *pArg2)
{
	ItemId usItem1Index;
	ItemId usItem2Index;
	UINT8  ubItem1Quality;
	UINT8  ubItem2Quality;

	usItem1Index = ItemId(( ( INVENTORY_IN_SLOT * ) pArg1 ) -> sItemIndex);
	usItem2Index = ItemId(( ( INVENTORY_IN_SLOT * ) pArg2 ) -> sItemIndex);

	ubItem1Quality = ( ( INVENTORY_IN_SLOT * ) pArg1 ) -> ItemObject.bStatus[ 0 ];
	ubItem2Quality = ( ( INVENTORY_IN_SLOT * ) pArg2 ) -> ItemObject.bStatus[ 0 ];

	return( CompareItemsForSorting( usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality ) );
}


static UINT8 GetDealerItemCategoryNumber(ItemId usItemIndex);


int CompareItemsForSorting(ItemId const item_index1, ItemId const item_index2, UINT8 const item_quality1, UINT8 const item_quality2)
{
	// lower category first
	UINT8 const category1 = GetDealerItemCategoryNumber(item_index1);
	UINT8 const category2 = GetDealerItemCategoryNumber(item_index2);
	if (category1 < category2) return -1;
	if (category1 > category2) return  1;

	const ItemModel * item1 = GCM->getItem(item_index1);
	const ItemModel * item2 = GCM->getItem(item_index2);

	// the same category
	if (item1->getItemClass() == IC_AMMO && item2->getItemClass() == IC_AMMO)
	{
		// AMMO is sorted by caliber first
		uint16_t calibre1 = item1->asAmmo()->calibre->index;
		uint16_t calibre2 = item2->asAmmo()->calibre->index;
		if (calibre1 > calibre2) return -1;
		if (calibre1 < calibre2) return  1;

		// the same caliber - compare size of magazine
		UINT8 const mag_size1 = item1->asAmmo()->capacity;
		UINT8 const mag_size2 = item2->asAmmo()->capacity;
		if (mag_size1 > mag_size2) return -1;
		if (mag_size1 < mag_size2) return  1;
	}
	else
	{
		// items other than ammo are compared on coolness first
		// higher coolness first
		UINT8 const coolness1 = item1->getCoolness();
		UINT8 const coolness2 = item2->getCoolness();
		if (coolness1 > coolness2) return -1;
		if (coolness1 < coolness2) return  1;
	}

	// the same coolness/caliber - compare base prices then
	// higher price first
	UINT16 const price1 = item1->getPrice();
	UINT16 const price2 = item2->getPrice();
	if (price1 > price2) return -1;
	if (price1 < price2) return  1;

	// the same price - compare item #s, then

	// lower index first
	if (item_index1 < item_index2) return -1;
	if (item_index1 > item_index2) return  1;

	// same item type = compare item quality, then

	// higher quality first
	if (item_quality1 > item_quality2) return -1;
	if (item_quality1 < item_quality2) return  1;

	// identical items!
	return 0;
}


static UINT8 GetDealerItemCategoryNumber(ItemId const usItemIndex)
{
	UINT32 const item_class = GCM->getItem(usItemIndex)->getItemClass();

	// If it's not a weapon, set no weapon class, as this won't be needed
	UINT8 const weapon_class = usItemIndex < ItemId(MAX_WEAPONS) ?
		GCM->getWeapon(usItemIndex)->ubWeaponClass : NOGUNCLASS;

	// search table until end-of-list marker is encountered
	for (UINT8 category = 0;; ++category)
	{
		ITEM_SORT_ENTRY const& ise = DealerItemSortInfo[category];
		if (ise.uiItemClass == IC_NONE) break;

		if (ise.uiItemClass != item_class) continue;
		if (ise.uiItemClass == IC_GUN && ise.ubWeaponClass != weapon_class) continue;

		return category;
	}

	// should never be trying to locate an item that's not covered in the table!
	Assert(FALSE);
	return 0;
}



BOOLEAN CanDealerItemBeSoldUsed( ItemId usItemIndex )
{
	if ( !( GCM->getItem(usItemIndex)->getFlags() & ITEM_DAMAGEABLE ) )
		return(FALSE);

	// certain items, although they're damagable, shouldn't be sold in a used condition
	return( DealerItemSortInfo[ GetDealerItemCategoryNumber( usItemIndex ) ].fAllowUsed );
}
