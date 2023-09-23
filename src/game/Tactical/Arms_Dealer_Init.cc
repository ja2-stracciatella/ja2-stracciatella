#include "LoadSaveData.h"
#include "Types.h"
#include "Arms_Dealer_Init.h"
#include "Debug.h"
#include "Random.h"
#include "Weapons.h"
#include "Game_Clock.h"
#include "ArmsDealerInvInit.h"
#include "Soldier_Profile.h"
#include "Handle_Items.h"
#include "Quests.h"
#include "Scheduling.h"
#include "Items.h"
#include "Observable.h"
#include "ContentManager.h"
#include "DealerInventory.h"
#include "DealerModel.h"
#include "GameInstance.h"
#include "MagazineModel.h"
#include "WeaponModels.h"

#include <algorithm>
#include <iterator>

// To reduce memory fragmentation from frequent MemRealloc(), we allocate memory for more than one special slot each
// time we run out of space.  Odds are that if we need one, we'll need another soon.
#define SPECIAL_ITEMS_ALLOCED_AT_ONCE			3
// Once allocated, the special item slots remain allocated for the duration of the game, or until the dealer dies.
// This is a little bit wasteful, but saves an awful lot of hassles, and avoid unnecessary memory fragmentation

#define MIN_REPAIR_TIME_IN_MINUTES			15 // minutes
#define MIN_REPAIR_COST				10 // dollars

// price classes
#define PRICE_CLASS_JUNK				0
#define PRICE_CLASS_CHEAP				1
#define PRICE_CLASS_EXPENSIVE				2

UINT8 gubLastSpecialItemAddedAtElement = 255;


// THESE GET SAVED/RESTORED/RESET
ARMS_DEALER_STATUS gArmsDealerStatus[ NUM_ARMS_DEALERS ];
DEALER_ITEM_HEADER gArmsDealersInventory[ NUM_ARMS_DEALERS ][ MAXITEMS ];

Observable<> OnDealerInventoryUpdated;

static void AdjustCertainDealersInventory(void);
static void InitializeOneArmsDealer(ArmsDealerID);

const DealerModel* GetDealer(UINT8 dealerID)
{
	if (dealerID >= NUM_ARMS_DEALERS)
	{
		ST::string err = ST::format("Invalid Dealer ID: {}", dealerID);
		throw std::runtime_error(err.to_std_string());
	}
	return GCM->getDealer(dealerID);
}

void InitAllArmsDealers()
{
	//Memset all dealers' status tables to zeroes
	std::fill(std::begin(gArmsDealerStatus), std::end(gArmsDealerStatus), ARMS_DEALER_STATUS{});

	//Memset all dealers' inventory tables to zeroes
	for (auto& inventory : gArmsDealersInventory)
	{
		std::fill(std::begin(inventory), std::end(inventory), DEALER_ITEM_HEADER{});
	}

	//Initialize the initial status & inventory for each of the arms dealers
	for (ArmsDealerID ubArmsDealer = ARMS_DEALER_FIRST; ubArmsDealer < NUM_ARMS_DEALERS; ++ubArmsDealer)
	{
		InitializeOneArmsDealer( ubArmsDealer );
	}

	//make sure certain items are in stock and certain limits are respected
	AdjustCertainDealersInventory( );
}


static void ArmsDealerGetsFreshStock(ArmsDealerID, UINT16 usItemIndex, UINT8 ubNumItems);


static void InitializeOneArmsDealer(ArmsDealerID const ubArmsDealer)
{
	UINT16 usItemIndex;
	UINT8  ubNumItems=0;


	gArmsDealerStatus[ ubArmsDealer ] = ARMS_DEALER_STATUS{};
	std::fill_n(gArmsDealersInventory[ ubArmsDealer ], static_cast<size_t>(MAXITEMS), DEALER_ITEM_HEADER{});


	//Reset the arms dealers cash on hand to the default initial value
	gArmsDealerStatus[ ubArmsDealer ].uiArmsDealersCash = GetDealer(ubArmsDealer)->initialCash;

	//if the arms dealer isn't supposed to have any items (includes all repairmen)
	if( GetDealer(ubArmsDealer)->hasFlag(ArmsDealerFlag::HAS_NO_INVENTORY) )
	{
		return;
	}


	//loop through all the item types
	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		//Can the item be sold by the arms dealer
		if( CanDealerTransactItem( ubArmsDealer, usItemIndex, FALSE ) )
		{
			//Setup an initial amount for the items (treat items as new, how many are used isn't known yet)
			ubNumItems = DetermineInitialInvItems( ubArmsDealer, usItemIndex, GetDealersMaxItemAmount( ubArmsDealer, usItemIndex ), FALSE );

			//if there are any initial items
			if( ubNumItems > 0 )
			{
				ArmsDealerGetsFreshStock( ubArmsDealer, usItemIndex, ubNumItems );
			}
		}
	}
}


static void FreeSpecialItemArray(DEALER_ITEM_HEADER* pDealerItem);


void ShutDownArmsDealers()
{
	UINT16 usItemIndex;

	// loop through all the dealers
	for (ArmsDealerID ubArmsDealer = ARMS_DEALER_FIRST; ubArmsDealer < NUM_ARMS_DEALERS; ++ubArmsDealer)
	{

		//loop through all the item types
		for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
		{
			if (gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() > 0)
			{
				FreeSpecialItemArray( &gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ] );
			}
		}
	}
}


void SaveArmsDealerInventoryToSaveGameFile(HWFILE const f)
{
	f->write(gArmsDealerStatus, sizeof(gArmsDealerStatus));

	for (DEALER_ITEM_HEADER const (*dealer)[MAXITEMS] = gArmsDealersInventory; dealer != endof(gArmsDealersInventory); ++dealer)
	{
		FOR_EACH(DEALER_ITEM_HEADER const, item, *dealer)
		{
			BYTE  data[16];
			DataWriter d{data};
			INJ_U8(  d, item->ubTotalItems)
			INJ_U8(  d, item->ubPerfectItems)
			INJ_U8(  d, item->ubStrayAmmo)
			Assert(item->SpecialItem.size() <= UINT8_MAX);
			INJ_U8(  d, static_cast<UINT8>(item->SpecialItem.size()))
			INJ_SKIP(d, 4)
			INJ_U32( d, item->uiOrderArrivalTime)
			INJ_U8(  d, item->ubQtyOnOrder)
			INJ_BOOL(d, item->fPreviouslyEligible)
			INJ_SKIP(d, 2)
			Assert(d.getConsumed() == lengthof(data));

			f->write(data, sizeof(data));
		}
	}

	// Save special items
	for (ArmsDealerID dealer = ARMS_DEALER_FIRST; dealer != NUM_ARMS_DEALERS; ++dealer)
	{
		for (UINT16 item_idx = 1; item_idx != MAXITEMS; ++item_idx)
		{
			DEALER_ITEM_HEADER const& di = gArmsDealersInventory[dealer][item_idx];
			if (di.SpecialItem.size() == 0) continue;
			f->write(di.SpecialItem.data(), sizeof(DEALER_SPECIAL_ITEM) * di.SpecialItem.size());
		}
	}
}


static void AllocMemsetSpecialItemArray(DEALER_ITEM_HEADER*, UINT8 ubElementsNeeded);


void LoadArmsDealerInventoryFromSavedGameFile(HWFILE const f, UINT32 const savegame_version)
{
	// Free all the dealers special inventory arrays
	ShutDownArmsDealers();

	// Elgin was added to the dealers list in Game Version #54, enlarging these 2 tables
	// Manny was added to the dealers list in Game Version #55, enlarging these 2 tables
	UINT32 const n_dealers_saved =
		savegame_version < 54 ? NUM_ARMS_DEALERS - 2 : // without Elgin and Manny
		savegame_version < 55 ? NUM_ARMS_DEALERS - 1 : // without Manny
		NUM_ARMS_DEALERS;

	f->read(gArmsDealerStatus, n_dealers_saved * sizeof(*gArmsDealerStatus));

	for (DEALER_ITEM_HEADER (*dealer)[MAXITEMS] = gArmsDealersInventory; dealer != gArmsDealersInventory + n_dealers_saved; ++dealer)
	{
		FOR_EACH(DEALER_ITEM_HEADER, item, *dealer)
		{
			BYTE data[16];
			f->read(data, sizeof(data));

			DataReader d{data};
			EXTR_U8(  d, item->ubTotalItems)
			EXTR_U8(  d, item->ubPerfectItems)
			EXTR_U8(  d, item->ubStrayAmmo)
			UINT8 numSpecialItem = 0;
			EXTR_U8(  d, numSpecialItem)
			EXTR_SKIP(d, 4)
			EXTR_U32( d, item->uiOrderArrivalTime)
			EXTR_U8(  d, item->ubQtyOnOrder)
			EXTR_BOOL(d, item->fPreviouslyEligible)
			EXTR_SKIP(d, 2)
			Assert(d.getConsumed() == lengthof(data));
			AllocMemsetSpecialItemArray(item, numSpecialItem);
		}
	}

	if (savegame_version < 54) InitializeOneArmsDealer(ARMS_DEALER_ELGIN);
	if (savegame_version < 55) InitializeOneArmsDealer(ARMS_DEALER_MANNY);

	// Load special items
	for (ArmsDealerID dealer = ARMS_DEALER_FIRST; dealer != NUM_ARMS_DEALERS; ++dealer)
	{
		for (UINT16 item_idx = 1; item_idx != MAXITEMS; ++item_idx)
		{
			DEALER_ITEM_HEADER& di = gArmsDealersInventory[dealer][item_idx];
			if (di.SpecialItem.size() == 0) continue;
			f->read(di.SpecialItem.data(), sizeof(DEALER_SPECIAL_ITEM) * di.SpecialItem.size());
		}
	}
}


static void ConvertCreatureBloodToElixir(void);
void DailyCheckOnItemQuantities();
static void SimulateArmsDealerCustomer(void);


void DailyUpdateOfArmsDealersInventory()
{
	// if Gabby has creature blood, start turning it into extra elixir
	ConvertCreatureBloodToElixir();

	//Simulate other customers buying inventory from the dealer
	SimulateArmsDealerCustomer();

	//if there are some items that are out of stock, order some more
	DailyCheckOnItemQuantities();

	//make sure certain items are in stock and certain limits are respected
	AdjustCertainDealersInventory( );
}


// Once a day, loop through each dealer's inventory items and possibly sell some
static void SimulateArmsDealerCustomer(void)
{
	UINT16 usItemIndex;
	UINT8  ubItemsSold=0;
	UINT8  ubElement;
	SPECIAL_ITEM_INFO SpclItemInfo;

	//loop through all the arms dealers
	for (ArmsDealerID ubArmsDealer = ARMS_DEALER_FIRST; ubArmsDealer < NUM_ARMS_DEALERS; ++ubArmsDealer)
	{
		if( gArmsDealerStatus[ ubArmsDealer ].fOutOfBusiness )
			continue;

		//if the arms dealer isn't supposed to have any items (includes all repairmen)
		if( GetDealer(ubArmsDealer)->hasFlag(ArmsDealerFlag::HAS_NO_INVENTORY ))
			continue;

		//loop through all items of the same type
		for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
		{
			//if there are some of these in stock
			if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems > 0)
			{
				// first, try to sell all the new (perfect) ones
				if ( usItemIndex == JAR_ELIXIR )
				{
					// only allow selling of standard # of items so those converted from blood given by player will be available
					ubItemsSold = HowManyItemsAreSold(ubArmsDealer, usItemIndex, (UINT8) std::min(UINT8(3), gArmsDealersInventory[ubArmsDealer][usItemIndex].ubPerfectItems), FALSE);
				}
				else
				{
					ubItemsSold = HowManyItemsAreSold( ubArmsDealer, usItemIndex, gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems, FALSE);
				}
				if ( ubItemsSold > 0)
				{
					// create item info describing a perfect item
					SetSpecialItemInfoToDefaults( &SpclItemInfo );
					//Now remove that many NEW ones (condition 100) of that item
					RemoveItemFromArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, ubItemsSold);
				}

				// next, try to sell all the used ones, gotta do these one at a time so we can remove them by element
				Assert(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
				for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
				{
					// don't worry about negative condition, repairmen can't come this far, they don't sell!
					if ( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive )
					{
						// try selling just this one
						if (HowManyItemsAreSold( ubArmsDealer, usItemIndex, 1, TRUE) > 0)
						{
							//Sold, now remove that particular USED one!
							RemoveSpecialItemFromArmsDealerInventoryAtElement( ubArmsDealer, usItemIndex, ubElement );
						}
					}
				}
			}
		}
	}
}


void DailyCheckOnItemQuantities()
{
	UINT16  usItemIndex;
	UINT8   ubMaxSupply;
	UINT8   ubNumItems;
	UINT32  uiArrivalDay;
	BOOLEAN fPrevElig;
	UINT8   ubReorderDays;

	//loop through all the arms dealers
	for (ArmsDealerID ubArmsDealer = ARMS_DEALER_FIRST; ubArmsDealer < NUM_ARMS_DEALERS; ++ubArmsDealer)
	{
		if( gArmsDealerStatus[ ubArmsDealer ].fOutOfBusiness )
			continue;

		//Reset the arms dealers cash on hand to the default initial value
		gArmsDealerStatus[ ubArmsDealer ].uiArmsDealersCash = GetDealer(ubArmsDealer)->initialCash;

		//if the arms dealer isn't supposed to have any items (includes all repairmen)
		if( GetDealer(ubArmsDealer)->hasFlag(ArmsDealerFlag::HAS_NO_INVENTORY ))
			continue;


		//loop through all items of the same type
		for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
		{
			//if the dealer can sell the item type
			if( CanDealerTransactItem( ubArmsDealer, usItemIndex, FALSE ) )
			{
				//if there are no items on order
				if ( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubQtyOnOrder == 0 )
				{
					ubMaxSupply = GetDealersMaxItemAmount( ubArmsDealer, usItemIndex );

					//if the qty on hand is half the desired amount or fewer
					if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems <= (UINT32)( ubMaxSupply / 2 ) )
					{
						// remember value of the "previously eligible" flag
						fPrevElig = gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].fPreviouslyEligible;

						//determine if the item can be restocked (assume new, use items aren't checked for until the stuff arrives)
						if (ItemTransactionOccurs( ubArmsDealer, usItemIndex, DEALER_BUYING, FALSE ))
						{
							// figure out how many items to reorder (items are reordered an entire batch at a time)
							ubNumItems = HowManyItemsToReorder( ubMaxSupply, gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems );

							// if this is the first day the player is eligible to have access to this thing
							if ( !fPrevElig )
							{
								// eliminate the ordering delay and stock the items instantly!
								// This is just a way to reward the player right away for making
								// progress without the reordering lag...
								ArmsDealerGetsFreshStock( ubArmsDealer, usItemIndex, ubNumItems );
							}
							else
							{
								if ( ( ubArmsDealer == ARMS_DEALER_TONY ) || ( ubArmsDealer == ARMS_DEALER_DEVIN ) )
								{
									// the stuff Tony and Devin sell is imported, so it takes longer
									// to arrive (for game balance)
									ubReorderDays = ( UINT8) ( 2 + Random( 2 ) ); // 2-3 days
								}
								else
								{
									ubReorderDays = ( UINT8) ( 1 + Random( 2 ) ); // 1-2 days
								}

								//Determine when the inventory should arrive
								uiArrivalDay = GetWorldDay() + ubReorderDays;	// consider changing this to minutes

								// post new order
								gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubQtyOnOrder = ubNumItems;
								gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].uiOrderArrivalTime = uiArrivalDay;
							}
						}
					}
				}
				else //items are on order
				{
					//and today is the day the items come in
					if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].uiOrderArrivalTime >= GetWorldDay() )
					{
						ArmsDealerGetsFreshStock( ubArmsDealer, usItemIndex, gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubQtyOnOrder);

						//reset order
						gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubQtyOnOrder = 0;
						gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].uiOrderArrivalTime = 0;
					}
				}
			}
		}
	}
}


static void ConvertCreatureBloodToElixir(void)
{
	UINT8 ubBloodAvailable;
	UINT8 ubAmountToConvert;
	SPECIAL_ITEM_INFO SpclItemInfo;

	ubBloodAvailable = gArmsDealersInventory[ ARMS_DEALER_GABBY ][ JAR_CREATURE_BLOOD ].ubTotalItems;
	if ( ubBloodAvailable )
	{
		// start converting blood into elixir!
		//ubAmountToConvert = (UINT8) std::min(5 + Random( 3 ), ubBloodAvailable);
		ubAmountToConvert = ubBloodAvailable;

		// create item info describing a perfect item
		SetSpecialItemInfoToDefaults( &SpclItemInfo );

		//Now remove that many NEW ones (condition 100) of that item
		RemoveItemFromArmsDealerInventory( ARMS_DEALER_GABBY, JAR_CREATURE_BLOOD, &SpclItemInfo, ubAmountToConvert );

		ArmsDealerGetsFreshStock( ARMS_DEALER_GABBY, JAR_ELIXIR, ubAmountToConvert );
	}
}


static void AddItemToArmsDealerInventory(ArmsDealerID, UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo, UINT8 ubHowMany);
static void GuaranteeAtLeastOneItemOfType(ArmsDealerID, UINT32 uiDealerItemType);
static void GuaranteeMinimumAlcohol(ArmsDealerID);
static void LimitArmsDealersInventory(ArmsDealerID, UINT32 uiDealerItemType, UINT8 ubMaxNumberOfItemType);


static void AdjustCertainDealersInventory(void)
{
	auto const& dealers = GCM->getDealers();

	//Adjust Tony's items (this restocks *instantly* 1/day, doesn't use the reorder system)
	GuaranteeAtLeastOneItemOfType( ARMS_DEALER_TONY, ARMS_DEALER_BIG_GUNS );
	LimitArmsDealersInventory( ARMS_DEALER_TONY, ARMS_DEALER_BIG_GUNS, 2 );
	LimitArmsDealersInventory( ARMS_DEALER_TONY, ARMS_DEALER_HANDGUNCLASS, 3 );
	LimitArmsDealersInventory( ARMS_DEALER_TONY, ARMS_DEALER_AMMO, 8 );

	//Adjust all bartenders' alcohol levels to a minimum
	for (auto dealer : dealers)
	{
		if (dealer->hasFlag(ArmsDealerFlag::SELLS_ALCOHOL))
		{
			GuaranteeMinimumAlcohol((ArmsDealerID)dealer->dealerID);
		}
	}

	//make sure Sam (hardware guy) has at least one empty jar
	GuaranteeAtLeastXItemsOfIndex( ARMS_DEALER_SAM, JAR, 1 );

	if ( CheckFact( FACT_ESTONI_REFUELLING_POSSIBLE, 0 ) )
	{
		// gas is restocked regularly, unlike most items
		for (auto dealer : dealers)
		{
			if (dealer->hasFlag(ArmsDealerFlag::SELLS_FUEL))
			{
				GuaranteeAtLeastXItemsOfIndex((ArmsDealerID)dealer->dealerID, GAS_CAN, (UINT8)(4 + Random(3)));
			}
		}
	}

	//If the player hasn't bought a video camera from Franz yet, make sure Franz has one to sell
	if( !( gArmsDealerStatus[ ARMS_DEALER_FRANZ ].ubSpecificDealerFlags & ARMS_DEALER_FLAG__FRANZ_HAS_SOLD_VIDEO_CAMERA_TO_PLAYER ) )
	{
		GuaranteeAtLeastXItemsOfIndex( ARMS_DEALER_FRANZ, VIDEO_CAMERA, 1 );
	}

	OnDealerInventoryUpdated();
}


static UINT32 GetArmsDealerItemTypeFromItemNumber(UINT16 usItem);
void RemoveRandomItemFromArmsDealerInventory(ArmsDealerID, UINT16 usItemIndex, UINT8 ubHowMany);


static void LimitArmsDealersInventory(ArmsDealerID const ubArmsDealer, UINT32 uiDealerItemType, UINT8 ubMaxNumberOfItemType)
{
	UINT16 usItemIndex=0;
	UINT32 uiItemsToRemove=0;
	SPECIAL_ITEM_INFO SpclItemInfo;

	UINT16 usAvailableItem[ MAXITEMS ] = { NOTHING };
	UINT8  ubNumberOfAvailableItem[ MAXITEMS ] = { 0 };
	UINT32 uiTotalNumberOfItems = 0, uiRandomChoice;
	UINT32 uiNumAvailableItems = 0, uiIndex;

	// not permitted for repair dealers - would take extra code to avoid counting items under repair!
	Assert( !DoesDealerDoRepairs( ubArmsDealer ) );

	if( gArmsDealerStatus[ ubArmsDealer ].fOutOfBusiness )
		return;

	//loop through all items of the same class and count the number in stock
	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		//if there is some items in stock
		if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems > 0)
		{
			//if the item is of the same dealer item type
			if( uiDealerItemType & GetArmsDealerItemTypeFromItemNumber( usItemIndex ) )
			{
				usAvailableItem[ uiNumAvailableItems ] = usItemIndex;

				//if the dealer item type is ammo
				if( uiDealerItemType == ARMS_DEALER_AMMO )
				{
					// all ammo of same type counts as only one item
					ubNumberOfAvailableItem[ uiNumAvailableItems ] = 1;
					uiTotalNumberOfItems++;
				}
				else
				{
					// items being repaired don't count against the limit
					ubNumberOfAvailableItem[ uiNumAvailableItems ] = gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems;
					uiTotalNumberOfItems += ubNumberOfAvailableItem[ uiNumAvailableItems ];
				}
				uiNumAvailableItems++;
			}
		}
	}

	//if there is more of the given type than we want
	if( uiNumAvailableItems > ubMaxNumberOfItemType )
	{
		uiItemsToRemove = uiNumAvailableItems - ubMaxNumberOfItemType;

		do
		{
			uiRandomChoice = Random( uiTotalNumberOfItems );

			for ( uiIndex = 0; uiIndex < uiNumAvailableItems; uiIndex++ )
			{
				if ( uiRandomChoice <= ubNumberOfAvailableItem[ uiIndex ] )
				{
					usItemIndex = usAvailableItem[ uiIndex ];
					if ( uiDealerItemType == ARMS_DEALER_AMMO )
					{
						// remove all of them, since each ammo item counts as only one "item" here
						// create item info describing a perfect item
						SetSpecialItemInfoToDefaults( &SpclItemInfo );
						// ammo will always be only condition 100, there's never any in special slots
						RemoveItemFromArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems );
					}
					else
					{
						// pick 1 random one, don't care about its condition
						RemoveRandomItemFromArmsDealerInventory( ubArmsDealer, usItemIndex, 1 );
					}
					// now remove entry from the array by replacing it with the last and decrementing
					// the size of the array
					usAvailableItem[ uiIndex ] = usAvailableItem[ uiNumAvailableItems -  1 ];
					ubNumberOfAvailableItem[ uiIndex ] = ubNumberOfAvailableItem[ uiNumAvailableItems - 1 ];
					uiNumAvailableItems--;

					// decrement count of # of items to remove
					uiItemsToRemove--;
					break; // and out of 'for' loop

				}
				else
				{
					// next item!
					uiRandomChoice -= ubNumberOfAvailableItem[ uiIndex ];
				}
			}

			/*
			//loop through all items of the same type
			for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
			{
				//if there are some non-repairing items in stock
				if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems )
				{
					//if the item is of the same dealer item type
					if( uiDealerItemType & GetArmsDealerItemTypeFromItemNumber( usItemIndex ) )
					{
						// a random chance that the item will be removed
						if( Random( 100 ) < 30 )
						{
							//remove the item

							//if the dealer item type is ammo
							if( uiDealerItemType == ARMS_DEALER_AMMO )
							{
								// remove all of them, since each ammo item counts as only one "item" here

								// create item info describing a perfect item
								SetSpecialItemInfoToDefaults( &SpclItemInfo );
								// ammo will always be only condition 100, there's never any in special slots
								RemoveItemFromArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems );
							}
							else
							{
								// pick 1 random one, don't care about its condition
								RemoveRandomItemFromArmsDealerInventory( ubArmsDealer, usItemIndex, 1 );
							}

							uiItemsToRemove--;
							if( uiItemsToRemove == 0)
								break;
						}
					}
				}
			}*/
		} while (uiItemsToRemove > 0);
	}
}


static void GuaranteeAtLeastOneItemOfType(ArmsDealerID const ubArmsDealer, UINT32 uiDealerItemType)
{
	UINT16 usItemIndex;
	UINT8  ubChance;
	UINT16 usAvailableItem[ MAXITEMS ] = { NOTHING };
	UINT8  ubChanceForAvailableItem[ MAXITEMS ] = { 0 };
	UINT32 uiTotalChances = 0;
	UINT32 uiNumAvailableItems = 0, uiIndex, uiRandomChoice;

	// not permitted for repair dealers - would take extra code to avoid counting items under repair!
	Assert( !DoesDealerDoRepairs( ubArmsDealer ) );

	if( gArmsDealerStatus[ ubArmsDealer ].fOutOfBusiness )
		return;

	//loop through all items of the same type
	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		//if the item is of the same dealer item type
		if( uiDealerItemType & GetArmsDealerItemTypeFromItemNumber( usItemIndex ) )
		{
			//if there are any of these in stock
			if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems > 0 )
			{
				//there is already at least 1 item of that type, return
				return;
			}

			// if he can stock it (it appears in his inventory list)
			if( GetDealersMaxItemAmount( ubArmsDealer, usItemIndex ) > 0)
			{
				// and the stage of the game gives him a chance to have it (assume new)
				ubChance = ChanceOfItemTransaction( ubArmsDealer, usItemIndex, DEALER_BUYING, FALSE );
				if ( ubChance > 0 )
				{
					usAvailableItem[ uiNumAvailableItems ] = usItemIndex;
					ubChanceForAvailableItem[ uiNumAvailableItems ] = ubChance;
					uiNumAvailableItems++;
					uiTotalChances += ubChance;
				}
			}
		}
	}

	// if there aren't any such items, the following loop would never finish, so quit before trying it!
	if (uiNumAvailableItems == 0)
	{
		return;
	}


	// CJC: randomly pick one of available items by weighted random selection.

	// randomize number within uiTotalChances and then loop forwards till we find that item
	uiRandomChoice = Random( uiTotalChances );

	for ( uiIndex = 0; uiIndex < uiNumAvailableItems; uiIndex++ )
	{
		if ( uiRandomChoice <= ubChanceForAvailableItem[ uiIndex ] )
		{
			ArmsDealerGetsFreshStock( ubArmsDealer, usAvailableItem[ uiIndex ], 1 );
			return;
		}
		else
		{
			// next item!
			uiRandomChoice -= ubChanceForAvailableItem[ uiIndex ];
		}
	}

	// internal logic failure!
}


void GuaranteeAtLeastXItemsOfIndex(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, UINT8 const ubHowMany)
{
	// not permitted for repair dealers - would take extra code to avoid counting items under repair!
	Assert( !DoesDealerDoRepairs( ubArmsDealer ) );

	if( gArmsDealerStatus[ ubArmsDealer ].fOutOfBusiness )
		return;

	//if there are any of these in stock
	if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems >= ubHowMany )
	{
		// have what we need...
		return;
	}

	// if he can stock it (it appears in his inventory list)
	// RESTRICTION REMOVED: Jake must be able to guarantee GAS even though it's not in his list, it's presence is conditional
	//if( GetDealersMaxItemAmount( ubArmsDealer, usItemIndex ) > 0)
	{
		//add the item
		ArmsDealerGetsFreshStock( ubArmsDealer, usItemIndex, (UINT8)( ubHowMany - gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems ) );
	}
}


static UINT32 GetArmsDealerItemTypeFromItemNumber(UINT16 usItem)
{
	switch( GCM->getItem(usItem)->getItemClass() )
	{
		case IC_NONE: return 0;

		case IC_GUN:
			switch(  GCM->getItem(usItem)->asWeapon()->ubWeaponClass )
			{
				case HANDGUNCLASS:
					return ARMS_DEALER_HANDGUNCLASS;

				case RIFLECLASS:
					return ItemIsARocketRifle(usItem) ?
						ARMS_DEALER_ROCKET_RIFLE : ARMS_DEALER_RIFLECLASS;

				case SHOTGUNCLASS:
					return ARMS_DEALER_SHOTGUNCLASS;
				case SMGCLASS:
					return ARMS_DEALER_SMGCLASS;
				case MGCLASS:
					return ARMS_DEALER_MGCLASS;
				case MONSTERCLASS:
					return 0;
				case KNIFECLASS:
					return ARMS_DEALER_KNIFECLASS;
			}
			break;

		case IC_PUNCH:
			if (usItem == NOTHING)
				return 0;
			// else treat as blade
			// fallthrough
		case IC_BLADE:
		case IC_THROWING_KNIFE:
			return ARMS_DEALER_BLADE;

		case IC_LAUNCHER:
			return ARMS_DEALER_LAUNCHER;
		case IC_ARMOUR:
			return ARMS_DEALER_ARMOUR;
		case IC_MEDKIT:
			return ARMS_DEALER_MEDKIT;
		case IC_KIT:
			return ARMS_DEALER_KIT;

		case IC_MISC:
			//switch on the type of item
			switch( usItem )
			{
				case BEER:
				case WINE:
				case ALCOHOL:
					return( ARMS_DEALER_ALCOHOL );

				case METALDETECTOR:
				case LASERSCOPE:
				//case REMDETONATOR:
					return( ARMS_DEALER_ELECTRONICS );

				case CANTEEN:
				case CROWBAR:
				case WIRECUTTERS:
					return( ARMS_DEALER_HARDWARE );

				case ADRENALINE_BOOSTER:
				case REGEN_BOOSTER:
				case SYRINGE_3:
				case SYRINGE_4:
				case SYRINGE_5:
					return( ARMS_DEALER_MEDICAL );

				case SILENCER:
				case SNIPERSCOPE:
				case BIPOD:
				case DUCKBILL:
					return( ARMS_DEALER_ATTACHMENTS );

				case DETONATOR:
				case REMDETONATOR:
				case REMOTEBOMBTRIGGER:
					return( ARMS_DEALER_DETONATORS );

				default: return ARMS_DEALER_MISC;
			}

		case IC_AMMO:
			return ARMS_DEALER_AMMO;

		case IC_FACE:
			switch( usItem )
			{
				case EXTENDEDEAR:
				case NIGHTGOGGLES:
				case ROBOT_REMOTE_CONTROL:
					return( ARMS_DEALER_ELECTRONICS );

				default:
					return ARMS_DEALER_FACE;
			}

		case IC_THROWN:
			return 0; // return ARMS_DEALER_THROWN;
		case IC_KEY:
			return 0; // return ARMS_DEALER_KEY;
		case IC_GRENADE:
			return ARMS_DEALER_GRENADE;
		case IC_BOMB:
			return ARMS_DEALER_BOMB;
		case IC_EXPLOSV:
			return ARMS_DEALER_EXPLOSV;

		case IC_TENTACLES:
		case IC_MONEY:
			return( 0 );

		default:
			AssertMsg(FALSE, ST::format("GetArmsDealerItemTypeFromItemNumber(), invalid class {} for item {}. DF 0.",
						GCM->getItem(usItem)->getItemClass(), usItem));
			break;
	}
	return( 0 );
}



BOOLEAN IsMercADealer( UINT8 ubMercID )
{
	UINT8 cnt;

	// Manny is not actually a valid dealer unless a particular event sets that fact
	if( ( ubMercID == MANNY ) && !CheckFact( FACT_MANNY_IS_BARTENDER, 0 ) )
	{
		return( FALSE );
	}

	//loop through the list of arms dealers
	for( cnt=0; cnt<NUM_ARMS_DEALERS; cnt++ )
	{
		if( GetDealer(cnt)->profileID == ubMercID )
			return( TRUE );
	}
	return( FALSE );
}


ArmsDealerID GetArmsDealerIDFromMercID(UINT8 const ubMercID)
{
	//loop through the list of arms dealers
	for (ArmsDealerID cnt = ARMS_DEALER_FIRST; cnt < NUM_ARMS_DEALERS; ++cnt)
	{
		if( GetDealer(cnt)->profileID == ubMercID )
			return( cnt );
	}

	return ARMS_DEALER_INVALID;
}



ArmsDealerType GetTypeOfArmsDealer(UINT8 ubDealerID)
{
	if (ubDealerID >= NUM_ARMS_DEALERS)
	{
		return ArmsDealerType::NOT_VALID_DEALER;
	}
	return GetDealer(ubDealerID)->type;
}


BOOLEAN	DoesDealerDoRepairs(ArmsDealerID const ubArmsDealer)
{
	return GetTypeOfArmsDealer(ubArmsDealer) == ARMS_DEALER_REPAIRS;
}


BOOLEAN RepairmanIsFixingItemsButNoneAreDoneYet( UINT8 ubProfileID )
{
	BOOLEAN fHaveOnlyUnRepairedItems=FALSE;
	UINT8   ubElement;
	UINT16  usItemIndex;

	ArmsDealerID const bArmsDealer = GetArmsDealerIDFromMercID( ubProfileID );
	if (bArmsDealer == ARMS_DEALER_INVALID) return FALSE;

	//if the dealer is not a repair dealer, return
	if( !DoesDealerDoRepairs( bArmsDealer ) )
		return( FALSE );

	//loop through the dealers inventory and check if there are only unrepaired items
	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		//if there is some items in stock
		if( gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].ubTotalItems )
		{
			//loop through the array of items
			Assert(gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
			{
				if ( gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive )
				{
					//if the items status is below 0, the item is being repaired
					if( gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].Info.bItemCondition < 0 )
					{
						//if the item has been repaired
						if( gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].uiRepairDoneTime <= GetWorldTotalMin() )
						{
							//A repair item is ready, therefore, return false
							return( FALSE );
						}
						else
						{
							fHaveOnlyUnRepairedItems = TRUE;
						}
					}
				}
			}
		}
	}

	return( fHaveOnlyUnRepairedItems );
}


static bool DoesItemAppearInDealerInventoryList(ArmsDealerID, UINT16 usItemIndex, BOOLEAN fPurchaseFromPlayer);


BOOLEAN CanDealerTransactItem(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, BOOLEAN const fPurchaseFromPlayer)
{
	switch ( GetTypeOfArmsDealer(ubArmsDealer) )
	{
		case ARMS_DEALER_SELLS_ONLY:
			if ( fPurchaseFromPlayer )
			{
				// this dealer only sells stuff to player, so he can't buy anything from him
				return( FALSE );
			}
			break;

		case ARMS_DEALER_BUYS_ONLY:
			if ( !fPurchaseFromPlayer )
			{
				// this dealer only buys stuff from player, so he can't sell anything to him
				return( FALSE );
			}
			break;

		case ARMS_DEALER_BUYS_SELLS:
			if (GetDealer(ubArmsDealer)->hasFlag(ArmsDealerFlag::BUYS_EVERYTHING))
			{
				if ( fPurchaseFromPlayer )
				{
					// these guys will buy nearly anything from the player, regardless of what they carry for sale!
					return( CalcValueOfItemToDealer( ubArmsDealer, usItemIndex, FALSE ) > 0 );
				}
				//else selling inventory uses their inventory list
				break;
			}
			// the others go by their inventory list
			break;

		case ARMS_DEALER_REPAIRS:
			// repairmen don't have a complete list of what they'll repair in their inventory,
			// so we must check the item's properties instead.
			return( CanDealerRepairItem( ubArmsDealer, usItemIndex ) );

		default:
			AssertMsg(FALSE, ST::format("CanDealerTransactItem(), type of dealer {}. AM 0.", GetDealer(ubArmsDealer)->type));
			return(FALSE);
	}

	return( DoesItemAppearInDealerInventoryList( ubArmsDealer, usItemIndex, fPurchaseFromPlayer ) );
}


BOOLEAN CanDealerRepairItem(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex)
{
	UINT32 uiFlags;

	uiFlags = GCM->getItem(usItemIndex)->getFlags();

	// can't repair anything that's not repairable!
	if ( !( uiFlags & ITEM_REPAIRABLE ) )
	{
		return(FALSE);
	}

	auto dealer = GetDealer(ubArmsDealer);
	if (dealer->type == ArmsDealerType::ARMS_DEALER_REPAIRS)
	{
		if (dealer->hasFlag(ArmsDealerFlag::REPAIRS_ELECTRONICS))
		{
			// repairs ONLY electronics
			return (uiFlags & ITEM_ELECTRONIC) > 0;
		}
		else
		{
			// repairs ANYTHING non-electronic
			return (uiFlags & ITEM_ELECTRONIC) == 0;
		}
	}
	else
	{
		AssertMsg(FALSE, ST::format("CanDealerRepairItem(), Arms Dealer {} is not a recognized repairman!.  AM 1.", ubArmsDealer));
	}

	// can't repair this...
	return(FALSE);
}


static void AllocMemsetSpecialItemArray(DEALER_ITEM_HEADER* const pDealerItem, UINT8 const ubElementsNeeded)
{
	Assert(pDealerItem);

	pDealerItem->SpecialItem.assign(ubElementsNeeded, DEALER_SPECIAL_ITEM{});
}


static void ResizeSpecialItemArray(DEALER_ITEM_HEADER* const pDealerItem, UINT8 const ubElementsNeeded)
{
	Assert(pDealerItem);

	pDealerItem->SpecialItem.resize(ubElementsNeeded, DEALER_SPECIAL_ITEM{});
}


static void FreeSpecialItemArray(DEALER_ITEM_HEADER* pDealerItem)
{
	Assert(pDealerItem);

	pDealerItem->SpecialItem.clear();

	pDealerItem->ubTotalItems = pDealerItem->ubPerfectItems;

	// doesn't effect perfect items, orders or stray bullets!
}


static UINT8 DetermineDealerItemCondition(ArmsDealerID, UINT16 usItemIndex);


static void ArmsDealerGetsFreshStock(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, UINT8 const ubNumItems)
{
	UINT8 ubCnt;
	UINT8 ubItemCondition;
	UINT8 ubPerfectOnes = 0;
	SPECIAL_ITEM_INFO SpclItemInfo;

	// create item info describing a perfect item
	SetSpecialItemInfoToDefaults( &SpclItemInfo );


	// determine the condition of each one, counting up new ones, but adding damaged ones right away
	for ( ubCnt = 0; ubCnt < ubNumItems; ubCnt++ )
	{
		ubItemCondition = DetermineDealerItemCondition( ubArmsDealer, usItemIndex);

		// if the item is brand new
		if ( ubItemCondition == 100)
		{
			ubPerfectOnes++;
		}
		else
		{
			// add a used item with that condition to his inventory
			SpclItemInfo.bItemCondition = (INT8) ubItemCondition;
			AddItemToArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, 1 );
		}
	}

	// now add all the perfect ones, in one shot
	if ( ubPerfectOnes > 0)
	{
		SpclItemInfo.bItemCondition = 100;
		AddItemToArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, ubPerfectOnes );
	}
}


static BOOLEAN ItemContainsLiquid(UINT16 usItemIndex);


static UINT8 DetermineDealerItemCondition(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex)
{
	UINT8 ubCondition = 100;

	// if it's a damagable item, and not a liquid (those are always sold full)
	if ( ( GCM->getItem(usItemIndex)->getFlags() & ITEM_DAMAGEABLE ) && !ItemContainsLiquid( usItemIndex ) )
	{
		// if he ONLY has used items, or 50% of the time if he carries both used & new items
		if ( ( GetDealer(ubArmsDealer)->hasFlag(ArmsDealerFlag::ONLY_USED_ITEMS) ) ||
			( ( GetDealer(ubArmsDealer)->hasFlag(ArmsDealerFlag::SOME_USED_ITEMS) ) && ( Random( 100 ) < 50 ) ) )
		{
			// make the item a used one
			ubCondition = (UINT8)(20 + Random( 60 ));
		}
	}

	return( ubCondition);
}


static BOOLEAN ItemContainsLiquid(UINT16 usItemIndex)
{
	switch ( usItemIndex )
	{
		case CANTEEN:
		case BEER:
		case ALCOHOL:
		case JAR_HUMAN_BLOOD:
		case JAR_CREATURE_BLOOD:
		case JAR_QUEEN_CREATURE_BLOOD:
		case JAR_ELIXIR:
		case GAS_CAN:
			return( TRUE );
	}

	return( FALSE );
}


static UINT8 CountActiveSpecialItemsInArmsDealersInventory(ArmsDealerID, UINT16 usItemIndex);


UINT32 CountDistinctItemsInArmsDealersInventory(ArmsDealerID const ubArmsDealer)
{
	UINT32	uiNumOfItems=0;
	UINT16	usItemIndex;


	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		//if there are any items
		if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems > 0 )
		{
			// if there are any items in perfect condition
			if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems > 0 )
			{
				// if the items can be stacked
				// NOTE: This test must match the one inside AddItemsToTempDealerInventory() exactly!
				if ( DealerItemIsSafeToStack( usItemIndex ) )
				{
					// regardless of how many there are, they count as 1 *distinct* item!  They will all be together in one box...
					uiNumOfItems++;
				}
				else
				{
					// non-stacking items must be stored in one / box , because each may have unique fields besides bStatus[]
					// Example: guns all have ammo, ammo type, etc.  We need these uniquely represented for pricing & manipulation
					uiNumOfItems += gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems;
				}
			}

			// each *active* special item counts as one additional distinct item (each one occupied a separate shopkeeper box!)
			// NOTE: This is including items being repaired!!!
			uiNumOfItems += CountActiveSpecialItemsInArmsDealersInventory( ubArmsDealer, usItemIndex);
		}
	}

	return( uiNumOfItems );
}


static UINT8 CountActiveSpecialItemsInArmsDealersInventory(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex)
{
	UINT8 ubActiveSpecialItems = 0;
	UINT8 ubElement;


	// next, try to sell all the used ones, gotta do these one at a time so we can remove them by element
	Assert(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
	for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
	{
		// don't worry about negative condition, repairmen can't come this far, they don't sell!
		if ( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive )
		{
			ubActiveSpecialItems++;
		}
	}

	return( ubActiveSpecialItems );
}


static UINT8 CountSpecificItemsRepairDealerHasInForRepairs(ArmsDealerID, UINT16 usItemIndex);


UINT16 CountTotalItemsRepairDealerHasInForRepairs(ArmsDealerID const ubArmsDealer)
{
	UINT16	usItemIndex;
	UINT16	usHowManyInForRepairs = 0;

	//if the dealer is not a repair dealer, no need to count, return 0
	if( !DoesDealerDoRepairs( ubArmsDealer ) )
		return( 0 );

	//loop through the dealers inventory and count the number of items in for repairs
	for( usItemIndex=0; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		usHowManyInForRepairs += CountSpecificItemsRepairDealerHasInForRepairs( ubArmsDealer, usItemIndex );
	}

	return( usHowManyInForRepairs );
}


static UINT8 CountSpecificItemsRepairDealerHasInForRepairs(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex)
{
	UINT8 ubElement;
	UINT8 ubHowManyInForRepairs = 0;

	//if the dealer is not a repair dealer, no need to count, return 0
	if( !DoesDealerDoRepairs( ubArmsDealer ) )
		return( 0 );


	//if there is some items in stock
	if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems )
	{
		//loop through the array of items
		Assert(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
		for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
		{
			if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive)
			{
				//if the item's status is below 0, the item is being repaired
				if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].Info.bItemCondition < 0 )
				{
					ubHowManyInForRepairs++;
				}
			}
		}
	}

	return( ubHowManyInForRepairs );
}


static void AddAmmoToArmsDealerInventory(ArmsDealerID, UINT16 usItemIndex, UINT8 ubShotsLeft);


void AddObjectToArmsDealerInventory(ArmsDealerID const ubArmsDealer, OBJECTTYPE* const pObject)
{
	UINT8 ubCnt;
	SPECIAL_ITEM_INFO SpclItemInfo;


	SetSpecialItemInfoFromObject( &SpclItemInfo, pObject );


	// split up all the components of an objecttype and add them as seperate items into the dealer's inventory
	switch ( GCM->getItem(pObject->usItem)->getItemClass() )
	{
		case IC_GUN:
			// add the gun (keeps the object's status and imprintID)
			// if the gun was jammed, this will forget about the jam (i.e. dealer immediately unjams anything he buys)
			AddItemToArmsDealerInventory( ubArmsDealer, pObject->usItem, &SpclItemInfo, 1 );

			// if any GunAmmoItem is specified
			if( pObject->usGunAmmoItem != NONE)
			{
				// if it's regular ammo
				if( GCM->getItem(pObject->usGunAmmoItem)->getItemClass() == IC_AMMO )
				{
					// and there are some remaining
					if ( pObject->ubGunShotsLeft > 0 )
					{
						// add the bullets of its remaining ammo
						AddAmmoToArmsDealerInventory( ubArmsDealer, pObject->usGunAmmoItem, pObject->ubGunShotsLeft );
					}
				}
				else	// assume it's attached ammo (mortar shells, grenades)
				{
					// add the launchable item (can't be imprinted, or have attachments!)
					SetSpecialItemInfoToDefaults( &SpclItemInfo );
					SpclItemInfo.bItemCondition = pObject->bGunAmmoStatus;

					// if the gun it was in was jammed, get rid of the negative status now
					if ( SpclItemInfo.bItemCondition < 0 )
					{
						SpclItemInfo.bItemCondition *= -1;
					}

					AddItemToArmsDealerInventory( ubArmsDealer, pObject->usGunAmmoItem, &SpclItemInfo, 1 );
				}
			}
			break;

		case IC_AMMO:
			// add the contents of each magazine (multiple mags may have vastly different #bullets left)
			for ( ubCnt = 0; ubCnt < pObject->ubNumberOfObjects; ubCnt++ )
			{
				AddAmmoToArmsDealerInventory( ubArmsDealer, pObject->usItem, pObject->ubShotsLeft[ ubCnt ] );
			}
			break;

		default:
			// add each object seperately (multiple objects may have vastly different statuses, keep any imprintID)
			for ( ubCnt = 0; ubCnt < pObject->ubNumberOfObjects; ubCnt++ )
			{
				SpclItemInfo.bItemCondition = pObject->bStatus[ ubCnt ];
				AddItemToArmsDealerInventory( ubArmsDealer, pObject->usItem, &SpclItemInfo, 1 );
			}
			break;
	}


	// loop through any detachable attachments and add them as seperate items
	for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if( pObject->usAttachItem[ ubCnt ] != NONE )
		{
			// ARM: Note: this is only used for selling, not repairs, so attachmentes are seperated when sold to a dealer
			// If the attachment is detachable
			if (! (GCM->getItem(pObject->usAttachItem[ubCnt])->getFlags() & ITEM_INSEPARABLE ) )
			{
				// add this particular attachment (they can't be imprinted, or themselves have attachments!)
				SetSpecialItemInfoToDefaults( &SpclItemInfo );
				SpclItemInfo.bItemCondition = pObject->bAttachStatus[ ubCnt ];
				AddItemToArmsDealerInventory( ubArmsDealer, pObject->usAttachItem[ ubCnt ], &SpclItemInfo, 1 );
			}
		}
	}


	// nuke the original object to prevent any possible item duplication
	*pObject = OBJECTTYPE{};
}


static void AddAmmoToArmsDealerInventory(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, UINT8 ubShotsLeft)
{
	UINT8 ubMagCapacity;
	UINT8 *pubStrayAmmo;
	SPECIAL_ITEM_INFO SpclItemInfo;



	// Ammo only, please!!!
	if (GCM->getItem(usItemIndex)->getItemClass() != IC_AMMO )
	{
		SLOGA("AddAmmoToArmsDealerInventory: Item isn't Ammo");
		return;
	}

	if ( ubShotsLeft == 0)
	{
		return;
	}


	ubMagCapacity = GCM->getItem(usItemIndex)->asAmmo()->capacity;

	if ( ubShotsLeft >= ubMagCapacity )
	{
		// add however many FULL magazines the #shot left represents
		SetSpecialItemInfoToDefaults( &SpclItemInfo );
		AddItemToArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, ( UINT8 ) ( ubShotsLeft / ubMagCapacity ) );
		ubShotsLeft %= ubMagCapacity;
	}

	// any shots left now are "strays" - not enough to completely fill a magazine of this type
	if ( ubShotsLeft > 0 )
	{
		// handle "stray" ammo - add it to the dealer's stray pile
		pubStrayAmmo = &(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubStrayAmmo);
		*pubStrayAmmo += ubShotsLeft;

		// if dealer has accumulated enough stray ammo to make another full magazine, convert it!
		if ( *pubStrayAmmo >= ubMagCapacity )
		{
			SetSpecialItemInfoToDefaults( &SpclItemInfo );
			AddItemToArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, ( UINT8 ) ( *pubStrayAmmo / ubMagCapacity ) );
			*pubStrayAmmo = *pubStrayAmmo % ubMagCapacity;
		}
		// I know, I know, this is getting pretty anal...  But what the hell, it was easy enough to do.  ARM.
	}
}


static void AddSpecialItemToArmsDealerInventoryAtElement(ArmsDealerID ubArmsDealer, UINT16 usItemIndex, UINT8 ubElement, SPECIAL_ITEM_INFO* pSpclItemInfo);
static BOOLEAN IsItemInfoSpecial(SPECIAL_ITEM_INFO* pSpclItemInfo);


//Use AddObjectToArmsDealerInventory() instead of this when converting a complex item in OBJECTTYPE format.
static void AddItemToArmsDealerInventory(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, SPECIAL_ITEM_INFO* const pSpclItemInfo, UINT8 ubHowMany)
{
	UINT8 ubRoomLeft;
	UINT8 ubElement;
	UINT8 ubElementsToAdd;
	BOOLEAN fFoundOne;

	Assert( ubHowMany > 0);

	ubRoomLeft = 255 - gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems;

	if ( ubHowMany > ubRoomLeft)
	{
		// not enough room to store that many, any extras vanish into thin air!
		ubHowMany = ubRoomLeft;
	}

	if ( ubHowMany == 0)
	{
		return;
	}


	// decide whether this item is "special" or not
	if ( IsItemInfoSpecial( pSpclItemInfo ) )
	{
		// Anything that's used/damaged or imprinted is store as a special item in the SpecialItem array,
		// exactly one item per element.  We (re)allocate memory dynamically as necessary to hold the additional items.

		do
		{
			// search for an already allocated, empty element in the special item array
			fFoundOne = FALSE;
			Assert(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
			{
				if ( !( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive ) )
				{
					//Great!  Store it here, then.
					AddSpecialItemToArmsDealerInventoryAtElement( ubArmsDealer, usItemIndex, ubElement, pSpclItemInfo );
					fFoundOne = TRUE;
					break;
				}
			}

			// if we didn't find any inactive elements already allocated
			if (!fFoundOne)
			{
				// then we're going to have to allocate some more space...
				ubElementsToAdd = std::max(SPECIAL_ITEMS_ALLOCED_AT_ONCE, int(ubHowMany));

				Assert(gArmsDealersInventory[ubArmsDealer][usItemIndex].SpecialItem.size() + ubElementsToAdd <= UINT8_MAX);
				ResizeSpecialItemArray(&gArmsDealersInventory[ubArmsDealer][usItemIndex], static_cast<UINT8>(gArmsDealersInventory[ubArmsDealer][usItemIndex].SpecialItem.size() + ubElementsToAdd));

				// now add the special item at the first of the newly added elements (still stored in ubElement!)
				AddSpecialItemToArmsDealerInventoryAtElement( ubArmsDealer, usItemIndex, ubElement, pSpclItemInfo );
			}

			// store the # of the element it was placed in globally so anyone who needs that can grab it there
			gubLastSpecialItemAddedAtElement = ubElement;

			ubHowMany--;
		} while ( ubHowMany > 0);
	}
	else	// adding perfect item(s)
	{
		// then it's stored as a "perfect" item, simply add it to that counter!
		gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems += ubHowMany;
		// increase total items of this type
		gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems += ubHowMany;
	}
}


static void AddSpecialItemToArmsDealerInventoryAtElement(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, UINT8 const ubElement, SPECIAL_ITEM_INFO* const pSpclItemInfo)
{
	Assert( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems < 255 );
	Assert( ubElement < gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() );
	Assert(!gArmsDealersInventory[ubArmsDealer][usItemIndex].SpecialItem[ubElement].fActive);
	Assert( IsItemInfoSpecial( pSpclItemInfo ) );


	//Store the special values in that element, and make it active
	gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive = TRUE;

	gArmsDealersInventory[ubArmsDealer][usItemIndex].SpecialItem[ubElement].Info = *pSpclItemInfo;

	// increase the total items
	gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems++;
}


// removes ubHowMany items of usItemIndex with the matching Info from dealer ubArmsDealer
void RemoveItemFromArmsDealerInventory(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, SPECIAL_ITEM_INFO* const pSpclItemInfo, UINT8 ubHowMany)
{
	DEALER_SPECIAL_ITEM *pSpecialItem;
	UINT8 ubElement;


	Assert( ubHowMany <= gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems );

	if ( ubHowMany == 0)
	{
		return;
	}


	// decide whether this item is "special" or not
	if ( IsItemInfoSpecial( pSpclItemInfo ) )
	{
		// look through the elements, trying to find special items matching the specifications
		Assert(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
		for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
		{
			pSpecialItem = &(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ]);

			// if this element is in use
			if ( pSpecialItem->fActive )
			{
				// and its contents are exactly what we're looking for
				if( memcmp( &(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].Info), pSpclItemInfo, sizeof( SPECIAL_ITEM_INFO ) ) == 0 )
				{
					// Got one!  Remove it
					RemoveSpecialItemFromArmsDealerInventoryAtElement( ubArmsDealer, usItemIndex, ubElement );

					ubHowMany--;
					if ( ubHowMany == 0)
					{
						break;
					}
				}
			}
		}

		// when we've searched all the special item elements, we'd better not have any more items to remove!
		Assert( ubHowMany == 0);
	}
	else	// removing perfect item(s)
	{
		// then it's stored as a "perfect" item, simply subtract from tha counter!
		Assert( ubHowMany <= gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems );
		gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems -= ubHowMany;
		// decrease total items of this type
		gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems -= ubHowMany;
	}
}


void RemoveRandomItemFromArmsDealerInventory(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, UINT8 ubHowMany)
{
	UINT8 ubWhichOne;
	UINT8 ubSkippedAlready;
	BOOLEAN fFoundIt;
	UINT8 ubElement;
	SPECIAL_ITEM_INFO SpclItemInfo;


	// not permitted for repair dealers - would take extra code to subtract items under repair from ubTotalItems!!!
	Assert( !DoesDealerDoRepairs( ubArmsDealer ) );
	// Can't remove any items in for repair, though!
	Assert( ubHowMany <= gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems );

	while ( ubHowMany > 0)
	{
		// pick a random one to get rid of
		ubWhichOne = (UINT8)Random(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems );

		// if we picked one of the perfect ones...
		if ( ubWhichOne < gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems )
		{
			// create item info describing a perfect item
			SetSpecialItemInfoToDefaults( &SpclItemInfo );
			// then that's easy, its condition is 100, so remove one of those
			RemoveItemFromArmsDealerInventory( ubArmsDealer, usItemIndex, &SpclItemInfo, 1 );
		}
		else
		{
			// Yikes!  Gotta look through the special items.  We already know it's not any of the perfect ones, subtract those
			ubWhichOne -= gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubPerfectItems;
			ubSkippedAlready = 0;

			fFoundIt = FALSE;

			Assert(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
			{
				// if this is an active special item, not in repair
				if (gArmsDealersInventory[ubArmsDealer][usItemIndex].SpecialItem[ubElement].fActive) // &&
					//(gArmsDealersInventory[ubArmsDealer][usItemIndex].SpecialItem[ubElement].Info.bItemCondition > 0))
				{
					// if we skipped the right amount of them
					if ( ubSkippedAlready == ubWhichOne )
					{
						// then this one is it!  That's the one we're gonna remove
						RemoveSpecialItemFromArmsDealerInventoryAtElement( ubArmsDealer, usItemIndex, ubElement );
						fFoundIt = TRUE;
						break;
					}
					else
					{
						// keep looking...
						ubSkippedAlready++;
					}
				}
			}

			// this HAS to work, or the data structure is corrupt!
			Assert(fFoundIt);
		}

		ubHowMany--;
	}
}


void RemoveSpecialItemFromArmsDealerInventoryAtElement(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, UINT8 const ubElement)
{
	Assert( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems > 0 );
	Assert( ubElement < gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() );
	Assert(gArmsDealersInventory[ubArmsDealer][usItemIndex].SpecialItem[ubElement].fActive);

	// wipe it out (turning off fActive)
	gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ] = DEALER_SPECIAL_ITEM{};

	// one fewer item remains...
	gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems--;
}



BOOLEAN AddDeadArmsDealerItemsToWorld(SOLDIERTYPE const* const pSoldier)
{
	UINT16 usItemIndex;
	UINT8  ubElement;
	UINT8  ubHowManyMaxAtATime;
	UINT8  ubLeftToDrop;
	UINT8  ubNowDropping;
	OBJECTTYPE TempObject;
	DEALER_SPECIAL_ITEM *pSpecialItem;
	SPECIAL_ITEM_INFO SpclItemInfo;

	//Get Dealer ID from from merc Id
	ArmsDealerID const bArmsDealer = GetArmsDealerIDFromMercID(pSoldier->ubProfile);
	if (bArmsDealer == ARMS_DEALER_INVALID)
	{
		// not a dealer, that's ok, we get called for every dude that croaks.
		return( FALSE );
	}


	// mark the dealer as being out of business!
	gArmsDealerStatus[ bArmsDealer ].fOutOfBusiness = TRUE;

	//loop through all the items in the dealer's inventory, and drop them all where the dealer was set up.

	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		//if the dealer has any items of this type
		if( gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].ubTotalItems > 0)
		{
			// if he has any perfect items of this time
			if ( gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].ubPerfectItems > 0 )
			{
				// drop all the perfect items first

				// drop stackable items like ammo in stacks of whatever will fit into a large pocket instead of one at a time
				ubHowManyMaxAtATime = ItemSlotLimit( usItemIndex, BIGPOCK1POS );
				if ( ubHowManyMaxAtATime < 1 )
				{
					ubHowManyMaxAtATime = 1;
				}

				// create item info describing a perfect item
				SetSpecialItemInfoToDefaults( &SpclItemInfo );

				ubLeftToDrop = gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].ubPerfectItems;

				// ATE: While it IS leagal here to use pSoldier->sInitialGridNo, cause of where this
				// function is called, there are times when we're not guarenteed that sGridNo is good
				while ( ubLeftToDrop > 0)
				{
					ubNowDropping = std::min(ubLeftToDrop, ubHowManyMaxAtATime);

					MakeObjectOutOfDealerItems( usItemIndex, &SpclItemInfo, &TempObject, ubNowDropping );
					AddItemToPool( pSoldier->sInitialGridNo, &TempObject, INVISIBLE, 0, 0, 0 );

					ubLeftToDrop -= ubNowDropping;
				}

				// remove them all from his inventory
				RemoveItemFromArmsDealerInventory( bArmsDealer, usItemIndex, &SpclItemInfo, gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].ubPerfectItems );
			}

			// then drop all the special items
			Assert(gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
			{
				pSpecialItem = &(gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ]);

				if ( pSpecialItem->fActive )
				{
					MakeObjectOutOfDealerItems(usItemIndex, &(pSpecialItem->Info), &TempObject, 1 );
					AddItemToPool( pSoldier->sInitialGridNo, &TempObject, INVISIBLE, 0, 0, 0 );
					RemoveItemFromArmsDealerInventory( bArmsDealer, usItemIndex, &(pSpecialItem->Info), 1 );
				}
			}

			// release any memory allocated for special items, he won't need it now...
			if (gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].SpecialItem.size() > 0)
			{
				FreeSpecialItemArray( &gArmsDealersInventory[ bArmsDealer ][ usItemIndex ] );
			}
		}
	}

	//if the dealer has money
	if( gArmsDealerStatus[ bArmsDealer ].uiArmsDealersCash > 0 )
	{
		CreateMoney(gArmsDealerStatus[bArmsDealer].uiArmsDealersCash, &TempObject);

		//add the money item to the dealers feet
		AddItemToPool( pSoldier->sInitialGridNo, &TempObject, INVISIBLE, 0, 0, 0 );

		gArmsDealerStatus[ bArmsDealer ].uiArmsDealersCash = 0;
	}


	return( TRUE );
}


void MakeObjectOutOfDealerItems( UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, OBJECTTYPE *pObject, UINT8 ubHowMany )
{
	INT8 bItemCondition;
	UINT8 ubCnt;


	bItemCondition = pSpclItemInfo->bItemCondition;

	//if the item condition is below 0, the item is in for repairs, so flip the sign
	if( bItemCondition < 0 )
	{
		bItemCondition *= -1;
	}

	CreateItems( usItemIndex, bItemCondition, ubHowMany, pObject );

	// set the ImprintID
	pObject->ubImprintID = pSpclItemInfo->ubImprintID;

	// add any attachments we've been storing
	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if ( pSpclItemInfo->usAttachment[ ubCnt ] != NONE )
		{
			// store what it is, and its condition
			pObject->usAttachItem[ ubCnt ]  = pSpclItemInfo->usAttachment[ ubCnt ];
			pObject->bAttachStatus[ ubCnt ] = pSpclItemInfo->bAttachmentStatus[ ubCnt ];
		}
	}

	// if it's a gun
	if (GCM->getItem(pObject->usItem)->getItemClass() == IC_GUN )
	{
		// Empty out the bullets put in by CreateItem().  We now sell all guns empty of bullets.  This is so that we don't
		// have to keep track of #bullets in a gun throughout dealer inventory.  Without this, players could "reload" guns
		// they don't have ammo for by selling them to Tony & buying them right back fully loaded!  One could repeat this
		// ad nauseum (empty the gun between visits) as a (really expensive) way to get unlimited special ammo like rockets.
		pObject->ubGunShotsLeft = 0;
	}
}


static void GiveItemToArmsDealerforRepair(ArmsDealerID, UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo, UINT8 ubOwnerProfileId);


void GiveObjectToArmsDealerForRepair(ArmsDealerID const ubArmsDealer, OBJECTTYPE const* const pObject, UINT8 const ubOwnerProfileId)
{
	//UINT8 ubCnt;
	SPECIAL_ITEM_INFO SpclItemInfo;


	Assert( DoesDealerDoRepairs( ubArmsDealer ) );

	// Any object passed into here must already be:
	//   a) Unstacked
	Assert( pObject->ubNumberOfObjects == 1 );

	//   b) Repairable
	Assert( CanDealerRepairItem( ubArmsDealer, pObject->usItem ) );

	//   c) Actually damaged, or a rocket rifle (being reset)
	Assert( ( pObject->bStatus[ 0 ] < 100 ) || ItemIsARocketRifle( pObject->usItem ) );

	/* ARM: Can now repair with removeable attachments still attached...
	//   d) Already stripped of all *detachable* attachments
	for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if ( pObject->usAttachItem[ ubCnt ] != NONE )
		{
			// If the attachment is detachable
			if (! (GCM->getItem(pObject->usAttachItem[ubCnt])->getFlags() & ITEM_INSEPARABLE ) )
			{
				SLOGA("GiveObjectToArmsDealerForRepair: something wrong with attachments");
			}
		}
	}*/

	//   e) If a gun, stripped of any non-ammo-class GunAmmoItems, and bullets
	if (GCM->getItem(pObject->usItem)->getItemClass() == IC_GUN )
	{
		// if any GunAmmoItem is specified
		if( pObject->usGunAmmoItem != NONE)
		{
			// it better be regular ammo, and empty
			Assert( GCM->getItem(pObject->usGunAmmoItem)->getItemClass() == IC_AMMO );
			Assert( pObject->ubGunShotsLeft == 0 );
		}
	}


	SetSpecialItemInfoFromObject( &SpclItemInfo, pObject );

	// ok, given all that, now everything is easy!
	// if the gun was jammed, this will forget about the jam (i.e. dealer immediately unjams anything he will be repairing)
	GiveItemToArmsDealerforRepair( ubArmsDealer, pObject->usItem, &SpclItemInfo, ubOwnerProfileId );
}


static UINT32 CalculateSpecialItemRepairTime(ArmsDealerID, UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo);
static UINT32 WhenWillRepairmanBeAllDoneRepairing(ArmsDealerID);


//PLEASE: Use GiveObjectToArmsDealerForRepair() instead of this when repairing a item in OBJECTTYPE format.
static void GiveItemToArmsDealerforRepair(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, SPECIAL_ITEM_INFO* const pSpclItemInfo, UINT8 const ubOwnerProfileId)
{
	UINT32	uiTimeWhenFreeToStartIt;
	UINT32	uiMinutesToFix;
	UINT32  uiMinutesShopClosedBeforeItsDone;
	UINT32	uiDoneWhen;


	Assert( DoesDealerDoRepairs( ubArmsDealer ) );
	Assert( pSpclItemInfo->bItemCondition > 0 );
	Assert( ( pSpclItemInfo->bItemCondition < 100 ) || ItemIsARocketRifle( usItemIndex ) );

	// figure out the earliest the repairman will be free to start repairing this item
	uiTimeWhenFreeToStartIt = WhenWillRepairmanBeAllDoneRepairing( ubArmsDealer );

	//Determine how long it will take to fix
	uiMinutesToFix = CalculateSpecialItemRepairTime( ubArmsDealer, usItemIndex, pSpclItemInfo );

	uiMinutesShopClosedBeforeItsDone = CalculateOvernightRepairDelay( ubArmsDealer, uiTimeWhenFreeToStartIt, uiMinutesToFix );

	// clock time when this will finally be ready
	uiDoneWhen = uiTimeWhenFreeToStartIt + uiMinutesToFix + uiMinutesShopClosedBeforeItsDone;

	// Negate the status
	pSpclItemInfo->bItemCondition *= -1;

	// give it to the dealer
	AddItemToArmsDealerInventory( ubArmsDealer, usItemIndex, pSpclItemInfo, 1 );

	//Set the time at which item will be fixed
	gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ gubLastSpecialItemAddedAtElement ].uiRepairDoneTime = uiDoneWhen;
	//Remember the original owner of the item
	gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ gubLastSpecialItemAddedAtElement ].ubOwnerProfileId = ubOwnerProfileId;
}


static UINT32 WhenWillRepairmanBeAllDoneRepairing(ArmsDealerID const ubArmsDealer)
{
	UINT32 uiWhenFree;
	UINT16 usItemIndex;
	UINT8  ubElement;

	Assert( DoesDealerDoRepairs( ubArmsDealer ) );

	// if nothing is in for repairs, he'll be free RIGHT NOW!
	uiWhenFree = GetWorldTotalMin();

	//loop through the dealers inventory
	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		//if there is some items in stock
		if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubTotalItems > 0 )
		{
			Assert(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size()); ubElement++)
			{
				if ( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive )
				{
					//if the item is in for repairs
					if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].Info.bItemCondition < 0 )
					{
						// if this item will be done later than the latest we've found so far
						if ( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].uiRepairDoneTime > uiWhenFree )
						{
							// then we're busy til then!
							uiWhenFree = gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].uiRepairDoneTime;
						}
					}
				}
			}
		}
	}

	return( uiWhenFree );
}


static UINT32 CalculateSimpleItemRepairTime(ArmsDealerID, UINT16 usItemIndex, INT8 bItemCondition);


static UINT32 CalculateSpecialItemRepairTime(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, SPECIAL_ITEM_INFO* const pSpclItemInfo)
{
	UINT32 uiRepairTime;
	UINT8 ubCnt;

	uiRepairTime = CalculateSimpleItemRepairTime( ubArmsDealer, usItemIndex, pSpclItemInfo->bItemCondition );

	// add time to repair any attachments on it
	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if ( pSpclItemInfo->usAttachment[ ubCnt ] != NONE )
		{
			// if damaged and repairable
			if ( ( pSpclItemInfo->bAttachmentStatus[ ubCnt ] < 100 ) && CanDealerRepairItem( ubArmsDealer, pSpclItemInfo->usAttachment[ ubCnt ] ) )
			{
				uiRepairTime += CalculateSimpleItemRepairTime( ubArmsDealer, pSpclItemInfo->usAttachment[ ubCnt ], pSpclItemInfo->bAttachmentStatus[ ubCnt ] );
			}
		}
	}

	return( uiRepairTime );
}


UINT32 CalculateObjectItemRepairTime(ArmsDealerID const ubArmsDealer, OBJECTTYPE const* const pItemObject)
{
	UINT32 uiRepairTime;
	UINT8 ubCnt;

	uiRepairTime = CalculateSimpleItemRepairTime( ubArmsDealer, pItemObject->usItem, pItemObject->bStatus[ 0 ] );

	// add time to repair any attachments on it
	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if ( pItemObject->usAttachItem[ ubCnt ] != NONE )
		{
			// if damaged and repairable
			if ( ( pItemObject->bAttachStatus[ ubCnt ] < 100 ) && CanDealerRepairItem( ubArmsDealer, pItemObject->usAttachItem[ ubCnt ] ) )
			{
				uiRepairTime += CalculateSimpleItemRepairTime( ubArmsDealer, pItemObject->usAttachItem[ ubCnt ], pItemObject->bAttachStatus[ ubCnt ] );
			}
		}
	}

	return( uiRepairTime );
}


static UINT32 CalculateSimpleItemRepairCost(ArmsDealerID, UINT16 usItemIndex, INT8 bItemCondition);


static UINT32 CalculateSimpleItemRepairTime(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, INT8 const bItemCondition)
{
	UINT32 uiTimeToRepair = 0;
	UINT32 uiRepairCost = 0;

	Assert( DoesDealerDoRepairs( ubArmsDealer ) );

	// first calc what he'll charge - that takes care of item condition, repair ease, and his repair cost "markup"
	uiRepairCost = CalculateSimpleItemRepairCost( ubArmsDealer, usItemIndex, bItemCondition );

	// Now adjust that for the repairman's individual repair speed.
	// For a repairman, his BUY modifier controls his REPAIR SPEED (1.0 means minutes to repair = price in $)
	// with a REPAIR SPEED of 1.0, typical gun price of $2000, and a REPAIR COST of 0.5 this works out to 16.6 hrs
	// for a full 100% status repair...  Not bad.
	uiTimeToRepair = (UINT32)(uiRepairCost * GetDealer(ubArmsDealer)->repairSpeed);

	// repairs on electronic items take twice as long if the guy doesn't have the skill
	// for dealers, this means anyone but Fredo the Electronics guy takes twice as long (but doesn't charge double)
	// (Mind you, current he's the ONLY one who CAN repair Electronics at all!  Oh well.)
	if( ( GCM->getItem(usItemIndex)->getFlags() & ITEM_ELECTRONIC ) && ( ubArmsDealer != ARMS_DEALER_FREDO ) )
	{
		uiTimeToRepair *= 2;
	}

	// avoid "instant" repairs on really cheap, barely damaged crap...
	if (uiTimeToRepair < MIN_REPAIR_TIME_IN_MINUTES)
	{
		uiTimeToRepair = MIN_REPAIR_TIME_IN_MINUTES;
	}

	return( uiTimeToRepair );
}


UINT32 CalculateObjectItemRepairCost(ArmsDealerID const ubArmsDealer, OBJECTTYPE const* const pItemObject)
{
	UINT32 uiRepairCost;
	UINT8 ubCnt;

	uiRepairCost = CalculateSimpleItemRepairCost( ubArmsDealer, pItemObject->usItem, pItemObject->bStatus[ 0 ] );

	// add cost of repairing any attachments on it
	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if ( pItemObject->usAttachItem[ ubCnt ] != NONE )
		{
			// if damaged and repairable
			if ( ( pItemObject->bAttachStatus[ ubCnt ] < 100 ) && CanDealerRepairItem( ubArmsDealer, pItemObject->usAttachItem[ ubCnt ] ) )
			{
				uiRepairCost += CalculateSimpleItemRepairCost( ubArmsDealer, pItemObject->usAttachItem[ ubCnt ], pItemObject->bAttachStatus[ ubCnt ] );
			}
		}
	}

	return( uiRepairCost );
}


static UINT32 CalculateSimpleItemRepairCost(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, INT8 const bItemCondition)
{
	UINT32 uiItemCost = 0;
	UINT32 uiRepairCost = 0;
	INT16  sRepairCostAdj = 0;
	//UINT32 uiDifFrom10=0;

	// figure out the full value of the item, modified by this dealer's personal Sell (i.e. repair cost) modifier
	// don't use CalcShopKeeperItemPrice - we want FULL value!!!
	uiItemCost = (UINT32)(GCM->getItem(usItemIndex)->getPrice() * GetDealer(ubArmsDealer)->repairCost);

	// get item's repair ease, for each + point is 10% easier, each - point is 10% harder to repair
	sRepairCostAdj = 100 - ( 10 * GCM->getItem(usItemIndex)->getRepairEase() );

	// make sure it ain't somehow gone too low!
	if (sRepairCostAdj < 10)
	{
		sRepairCostAdj = 10;
	}

	// calculate repair cost, the more broken it is the more it costs, and the difficulty of repair it is also a factor
	uiRepairCost = (UINT32)( uiItemCost * ( sRepairCostAdj * (100 - bItemCondition) / ((FLOAT)100 * 100) ));

	/*
	//if the price is not diviseble by 10, make it so
	uiDifFrom10 = 10 - uiRepairCost % 10;
	if( uiDifFrom10 != 0 )
	{
		uiRepairCost += uiDifFrom10;
	}*/

	if ( ItemIsARocketRifle( usItemIndex ) )
	{
		// resetting imprinting for a rocket rifle costs something extra even if rifle is at 100%
		uiRepairCost += 100;
	}


	// anything repairable has to have a minimum price
	if ( uiRepairCost < MIN_REPAIR_COST )
	{
		uiRepairCost = MIN_REPAIR_COST;
	}

	return( uiRepairCost );
}



void SetSpecialItemInfoToDefaults( SPECIAL_ITEM_INFO *pSpclItemInfo )
{
	UINT8 ubCnt;

	*pSpclItemInfo = SPECIAL_ITEM_INFO{};

	pSpclItemInfo->bItemCondition = 100;
	pSpclItemInfo->ubImprintID = NO_PROFILE;

	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		pSpclItemInfo->usAttachment[ ubCnt ] = NONE;
		pSpclItemInfo->bAttachmentStatus[ ubCnt ] = 0;
	}
}


void SetSpecialItemInfoFromObject(SPECIAL_ITEM_INFO* pSpclItemInfo, const OBJECTTYPE* pObject)
{
	UINT8 ubCnt;


	*pSpclItemInfo = SPECIAL_ITEM_INFO{};


	if( GCM->getItem(pObject->usItem)->getItemClass() == IC_AMMO )
	{
		// ammo condition is always 100, don't use status, which holds the #bullets
		pSpclItemInfo->bItemCondition = 100;
	}
	else
	{
		pSpclItemInfo->bItemCondition = pObject->bStatus[ 0 ];
	}

	// only guns currently have imprintID properly initialized...
	if ( GCM->getItem(pObject->usItem)->getItemClass() == IC_GUN)
	{
		pSpclItemInfo->ubImprintID = pObject->ubImprintID;
	}
	else
	{
		// override garbage imprintIDs (generally 0) for non-guns
		pSpclItemInfo->ubImprintID = NO_PROFILE;
	}

	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if( pObject->usAttachItem[ ubCnt ] != NONE )
		{
			// store what it is
			pSpclItemInfo->usAttachment[ ubCnt ] = pObject->usAttachItem[ ubCnt ];
			pSpclItemInfo->bAttachmentStatus[ ubCnt ] = pObject->bAttachStatus[ ubCnt ];
		}
		else
		{
			pSpclItemInfo->usAttachment[ ubCnt ] = NONE;
			pSpclItemInfo->bAttachmentStatus[ ubCnt ] = 0;
		}
	}
}


static BOOLEAN IsItemInfoSpecial(SPECIAL_ITEM_INFO* pSpclItemInfo)
{
	UINT8 ubCnt;


	// being damaged / in repairs makes an item special
	if ( pSpclItemInfo->bItemCondition != 100 )
	{
		return(TRUE);
	}

	// being imprinted makes an item special
	if (pSpclItemInfo->ubImprintID != NO_PROFILE)
	{
		return(TRUE);
	}

	// having an attachment makes an item special
	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if ( pSpclItemInfo->usAttachment[ ubCnt ] != NONE )
		{
			return(TRUE);
		}
	}

	// otherwise, it's just a "perfect" item, nothing special about it
	return(FALSE);
}


static bool DoesItemAppearInDealerInventoryList(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, BOOLEAN const fPurchaseFromPlayer)
{
	if(GCM->getDealerInventory(ubArmsDealer)->hasItem(GCM->getItem(usItemIndex)))
	{
		int maxAmount = GetDealersMaxItemAmount(ubArmsDealer, usItemIndex);
		return (maxAmount > 0) || fPurchaseFromPlayer;
	}
	else
	{
		return false;
	}
}


UINT16 CalcValueOfItemToDealer(ArmsDealerID const ubArmsDealer, UINT16 const usItemIndex, BOOLEAN const fDealerSelling)
{
	UINT16 usBasePrice;
	UINT8 ubItemPriceClass;
	UINT8 ubDealerPriceClass;
	UINT16 usValueToThisDealer;


	usBasePrice = GCM->getItem(usItemIndex)->getPrice();

	if ( usBasePrice == 0 )
	{
		// worthless to any dealer
		return( 0 );
	}


	// figure out the price class this dealer prefers
	switch ( ubArmsDealer )
	{
		case ARMS_DEALER_JAKE:
			ubDealerPriceClass = PRICE_CLASS_JUNK;
			break;
		case ARMS_DEALER_KEITH:
			ubDealerPriceClass = PRICE_CLASS_CHEAP;
			break;
		case ARMS_DEALER_FRANZ:
			ubDealerPriceClass = PRICE_CLASS_EXPENSIVE;
			break;

		// other dealers don't use this system
		default:
			if ( DoesItemAppearInDealerInventoryList( ubArmsDealer, usItemIndex, TRUE ) )
			{
				return( usBasePrice );
			}
			else
			{
				return( 0 );
			}
	}


	// the rest of this function applies only to the "general" dealers ( Jake, Keith, and Franz )

	// Micky & Gabby specialize in creature parts & such, the others don't buy these at all (exception: jars)
	if ((usItemIndex != JAR) &&
		(DoesItemAppearInDealerInventoryList(ARMS_DEALER_MICKY, usItemIndex, TRUE) ||
		DoesItemAppearInDealerInventoryList(ARMS_DEALER_GABBY, usItemIndex, TRUE)))
	{
		return( 0 );
	}

	if ( ( ubArmsDealer == ARMS_DEALER_KEITH ) && ( GCM->getItem(usItemIndex)->getItemClass() & ( IC_GUN | IC_LAUNCHER ) ) )
	{
		// Keith won't buy guns until the Hillbillies are vanquished
		if (!CheckFact(FACT_HILLBILLIES_KILLED, KEITH))
		{
			return( 0 );
		}
	}


	// figure out which price class it belongs to
	if ( usBasePrice < 100 )
	{
		ubItemPriceClass = PRICE_CLASS_JUNK;
	}
	else
	if ( usBasePrice < 1000 )
	{
		ubItemPriceClass = PRICE_CLASS_CHEAP;
	}
	else
	{
		ubItemPriceClass = PRICE_CLASS_EXPENSIVE;
	}


	if( !fDealerSelling )
	{
		// junk dealer won't buy expensive stuff at all, expensive dealer won't buy junk at all
		if (std::abs((INT8) ubDealerPriceClass - (INT8) ubItemPriceClass) == 2)
		{
			return( 0 );
		}
	}

	// start with the base price
	usValueToThisDealer = usBasePrice;

	// if it's out of their preferred price class
	if ( ubDealerPriceClass != ubItemPriceClass )
	{
		// exception: Gas (Jake's)
		if ( usItemIndex != GAS_CAN )
		{
			// they pay only 1/3 of true value!
			usValueToThisDealer /= 3;
		}
	}


	// Tony specializes in guns, weapons, and ammo, so make others pay much less for that kind of stuff
	if ( DoesItemAppearInDealerInventoryList( ARMS_DEALER_TONY, usItemIndex, TRUE ) )
	{
		// others pay only 1/2 of that value!
		usValueToThisDealer /= 2;
	}


	// minimum bet $1 !
	if ( usValueToThisDealer == 0 )
	{
		usValueToThisDealer = 1;
	}

	return( usValueToThisDealer );
}


BOOLEAN DealerItemIsSafeToStack( UINT16 usItemIndex )
{
	// basically any item type with nothing unique about it besides its status can be stacked in dealer's inventory boxes...
	// NOTE: This test is only applied to items already KNOWN to be perfect - special items are obviously not-stackable

	if ( GCM->getItem(usItemIndex)->getItemClass() == IC_GUN )
	{
		return( FALSE );
	}

	/*
	if ( ItemSlotLimit( usItemIndex, BIGPOCK1POS ) > 1 )
	{
		return( TRUE );
	}*/

	return( TRUE );
}


static void GuaranteeMinimumAlcohol(ArmsDealerID const ubArmsDealer)
{
	GuaranteeAtLeastXItemsOfIndex( ubArmsDealer, BEER,		( UINT8 ) ( GetDealersMaxItemAmount( ubArmsDealer, BEER ) / 3 ) );
	GuaranteeAtLeastXItemsOfIndex( ubArmsDealer, WINE,		( UINT8 ) ( GetDealersMaxItemAmount( ubArmsDealer, WINE ) / 3 ) );
	GuaranteeAtLeastXItemsOfIndex( ubArmsDealer, ALCOHOL, ( UINT8 ) ( GetDealersMaxItemAmount( ubArmsDealer, ALCOHOL ) / 3 ) );
}


BOOLEAN ItemIsARocketRifle(INT16 sItemIndex)
{
	if ( ( sItemIndex == ROCKET_RIFLE ) || ( sItemIndex == AUTO_ROCKET_RIFLE ) )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static BOOLEAN GetArmsDealerShopHours(ArmsDealerID const ubArmsDealer, UINT32* const puiOpeningTime, UINT32* const puiClosingTime)
{
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(GetDealer(ubArmsDealer)->profileID);
	if ( pSoldier == NULL )
	{
		return( FALSE );
	}

	if (!ExtractScheduleDoorLockAndUnlockInfo(pSoldier, puiOpeningTime, puiClosingTime))
	{
		return( FALSE );
	}

	Assert( *puiOpeningTime < *puiClosingTime );

	return( TRUE );
}



UINT32 CalculateOvernightRepairDelay(ArmsDealerID const ubArmsDealer, UINT32 uiTimeWhenFreeToStartIt, UINT32 uiMinutesToFix)
{
	UINT32 uiOpeningTime, uiClosingTime;
	UINT32 uiMinutesClosedOvernight;
	UINT32 uiDelayInDays = 0;
	UINT32 uiDoneToday;


	Assert( uiMinutesToFix > 0 );

	// convert world time into 24hr military time for the day he's gonna start on it
	uiTimeWhenFreeToStartIt = uiTimeWhenFreeToStartIt % NUM_MIN_IN_DAY;

	if (!GetArmsDealerShopHours(ubArmsDealer, &uiOpeningTime, &uiClosingTime))
	{
		return( 0 );
	}

	// if it won't get done by the end of a day
	while ( ( uiTimeWhenFreeToStartIt + uiMinutesToFix ) > uiClosingTime )
	{
		// this is to handle existing saves with overnight repairs
		if ( uiTimeWhenFreeToStartIt < uiClosingTime )
		{
			// he gets this much done before closing
			uiDoneToday = uiClosingTime - uiTimeWhenFreeToStartIt;
			// subtract how much he got done
			uiMinutesToFix -= uiDoneToday;
			Assert( uiMinutesToFix > 0 );
		}

		// he starts back at it first thing in the morning
		uiTimeWhenFreeToStartIt = uiOpeningTime;
		uiDelayInDays++;
	}

	uiMinutesClosedOvernight = NUM_MIN_IN_DAY - ( uiClosingTime - uiOpeningTime );

	return ( uiDelayInDays * uiMinutesClosedOvernight );
}


UINT32 CalculateMinutesClosedBetween(ArmsDealerID const ubArmsDealer, UINT32 uiStartTime, UINT32 uiEndTime)
{
	UINT32 uiOpeningTime, uiClosingTime;
	UINT32 uiMinutesClosedOvernight;
	UINT32 uiDaysDifference = 0;
	UINT32 uiMinutesClosed = 0;

	Assert( uiStartTime <= uiEndTime );

	if (!GetArmsDealerShopHours(ubArmsDealer, &uiOpeningTime, &uiClosingTime))
	{
		return( 0 );
	}

	uiMinutesClosedOvernight = NUM_MIN_IN_DAY - ( uiClosingTime - uiOpeningTime );

	// NOTE: this assumes stored are only closed overnight, so all we have to do is compare the day portion
	uiDaysDifference = ( uiEndTime / NUM_MIN_IN_DAY ) - ( uiStartTime / NUM_MIN_IN_DAY );

	if ( uiDaysDifference >= 2 )
	{
		// close for 1 less than that many full nights...
		uiMinutesClosed = ( uiDaysDifference - 1 ) * uiMinutesClosedOvernight;
	}


	// add partial day's closing

	// convert start and end times into 24hr military time
	uiStartTime = uiStartTime % NUM_MIN_IN_DAY;
	uiEndTime = uiEndTime % NUM_MIN_IN_DAY;

	// treat end time of midnight as 24:00 hours to prevent indefinite recursion and make formulas work
	if ( uiEndTime == 0 )
	{
		uiEndTime = NUM_MIN_IN_DAY;
	}


	if ( uiStartTime == uiEndTime )
	{
		if ( uiDaysDifference == 0 )
		{
			return( 0 );
		}
		else
		{
			uiMinutesClosed += uiMinutesClosedOvernight;
		}
	}
	if ( uiStartTime < uiEndTime )
	{
		if ( uiStartTime < uiOpeningTime )
		{
			// add how many minutes in the time range BEFORE the store opened that day
			uiMinutesClosed += ( std::min(uiOpeningTime, uiEndTime ) - uiStartTime);
		}

		if ( uiEndTime > uiClosingTime )
		{
			// add how many minutes in the time range AFTER the store closed that day
			uiMinutesClosed += ( uiEndTime - std::max(uiClosingTime, uiStartTime ));
		}
	}
	else
	{
		Assert( uiEndTime < uiStartTime );

		// recursive calls!  Add two separate times: before midnight, and after midnight
		uiMinutesClosed += CalculateMinutesClosedBetween( ubArmsDealer, uiStartTime, NUM_MIN_IN_DAY );
		uiMinutesClosed += CalculateMinutesClosedBetween( ubArmsDealer, 0, uiEndTime );
	}

	return ( uiMinutesClosed );
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(ArmsDealerInit, asserts)
{
	EXPECT_EQ(sizeof(ARMS_DEALER_STATUS), 20u);
	EXPECT_EQ(sizeof(SPECIAL_ITEM_INFO), 16u);
	EXPECT_EQ(sizeof(DEALER_SPECIAL_ITEM), 28u);
}

#endif
