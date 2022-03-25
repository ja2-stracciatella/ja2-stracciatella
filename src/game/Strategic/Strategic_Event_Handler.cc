#include "MapScreen.h"
#include "Strategic_Event_Handler.h"
#include "MemMan.h"
#include "Item_Types.h"
#include "Items.h"
#include "Handle_Items.h"
#include "LaptopSave.h"
#include "Tactical_Save.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "Soldier_Profile.h"
#include "Game_Event_Hook.h"
#include "Game_Clock.h"
#include "Interface_Dialogue.h"
#include "Random.h"
#include "Overhead.h"
#include "Strategic_Town_Loyalty.h"
#include "Soldier_Init_List.h"
#include "SaveLoadMap.h"
#include "Soldier_Create.h"
#include "Soldier_Add.h"
#include "OppList.h"
#include "EMail.h"
#include "Structure_Wrap.h"
#include "History.h"
#include "BobbyRMailOrder.h"
#include "ShippingDestinationModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"
#include "strategic/NpcPlacementModel.h"
#include "MercProfile.h"
#include "MercProfileInfo.h"


UINT32 guiPabloExtraDaysBribed = 0;

UINT8		gubCambriaMedicalObjects;


static BOOLEAN CloseCrate(const SGPSector& sector, const GridNo grid_no)
{
	// Determine if the sector is loaded
	if (gWorldSector == sector)
	{
		SetOpenableStructureToClosed(grid_no, 0);
		return TRUE;
	}
	else
	{
		ChangeStatusOfOpenableStructInUnloadedSector(sector, grid_no, FALSE);
		return FALSE;
	}
}


static void DropOffItemsInDestination(UINT8 ubOrderNum, const ShippingDestinationModel* shippingDest);


void BobbyRayPurchaseEventCallback(const UINT8 ubOrderID)
{
	static UINT8 ubShipmentsSinceNoBribes = 0;

	NewBobbyRayOrderStruct* const shipment = &gpNewBobbyrShipments[ubOrderID];
	auto dest = GCM->getShippingDestination(shipment->ubDeliveryLoc);

	if (shipment->fActive && dest->canDeliver && !dest->isPrimary)
	{
		// the delivery is not for Drassen, use a simple logic (reliable delivery) to handle delivery arrival there
		DropOffItemsInDestination(ubOrderID, dest);
		return;
	}

	UINT16 usStandardMapPos = dest->deliverySectorGridNo;
	auto destSector = StrategicMap[SECTOR_INFO_TO_STRATEGIC_INDEX(dest->getDeliverySector())];
	if (CheckFact(FACT_NEXT_PACKAGE_CAN_BE_LOST, 0))
	{
		SetFactFalse(FACT_NEXT_PACKAGE_CAN_BE_LOST);
		if (Random(100) < 50)
		{
			// lose the whole shipment!
			shipment->fActive = FALSE;
			SetFactTrue(FACT_LAST_SHIPMENT_CRASHED);
			return;
		}
	}
	else if (CheckFact(FACT_NEXT_PACKAGE_CAN_BE_DELAYED, 0))
	{
		// shipment went to wrong airport... reroute all items to a temporary
		// gridno to represent the other airport (and damage them)
		SetFactTrue(FACT_LAST_SHIPMENT_WENT_TO_WRONG_AIRPORT);
		SetFactFalse(FACT_NEXT_PACKAGE_CAN_BE_DELAYED);
		usStandardMapPos = LOST_SHIPMENT_GRIDNO;
	}
	else if (gTownLoyalty[destSector.bNameId].ubRating < 20 || destSector.fEnemyControlled)
	{
		// loss of the whole shipment
		shipment->fActive = FALSE;
		SetFactTrue(FACT_AGENTS_PREVENTED_SHIPMENT);
		return;
	}

	//Must get the total number of items ( all item types plus how many of each item type ordered )
	BOOLEAN fThisShipmentIsFromJohnKulba = FALSE; //if it is, dont add an email
	UINT16  usNumberOfItems              = 0;
	for (UINT8 i = 0; i < shipment->ubNumberPurchases; ++i)
	{
		const BobbyRayPurchaseStruct* const purchase = &shipment->BobbyRayPurchase[i];

		// Count how many items were purchased
		usNumberOfItems += purchase->ubNumberPurchased;

		//if any items are AutoMags
		if (purchase->usItemIndex == AUTOMAG_III)
		{
			//This shipment is from John Kulba, dont add an email from bobby ray
			fThisShipmentIsFromJohnKulba = TRUE;
		}
	}

	const BOOLEAN fSectorLoaded =
		CloseCrate(dest->deliverySector, dest->deliverySectorGridNo);

	OBJECTTYPE* pObject       = NULL;
	OBJECTTYPE* pStolenObject = NULL;
	if (!fSectorLoaded) // if we are NOT currently in the right sector
	{
		//build an array of objects to be added
		pObject       = new OBJECTTYPE[usNumberOfItems]{};
		pStolenObject = new OBJECTTYPE[usNumberOfItems]{};
	}

	// check for potential theft
	UINT32 uiChanceOfTheft;
	if (CheckFact(FACT_PABLO_WONT_STEAL, 0) || gamepolicy(pablo_wont_steal))
	{
		uiChanceOfTheft = 0;
	}
	else if (CheckFact(FACT_PABLOS_BRIBED, 0))
	{
		// Since Pacos has some money, reduce record of # of shipments since last bribed...
		ubShipmentsSinceNoBribes /= 2;
		uiChanceOfTheft = 0;
	}
	else
	{
		++ubShipmentsSinceNoBribes;
		// this chance might seem high but it's only applied at most to every second item
		uiChanceOfTheft = 12 + Random(4 * ubShipmentsSinceNoBribes);
	}

	UINT32  uiCount               = 0;
	UINT32  uiStolenCount         = 0;
	BOOLEAN fPablosStoleSomething = FALSE;
	BOOLEAN fPablosStoleLastItem  = FALSE;
	for (UINT8 i = 0; i < shipment->ubNumberPurchases; ++i)
	{
		const BobbyRayPurchaseStruct* const purchase = &shipment->BobbyRayPurchase[i];
		UINT16                        const usItem   = purchase->usItemIndex;

		OBJECTTYPE Object;
		CreateItem(usItem, purchase->bItemQuality, &Object);

		if (GCM->getItem(usItem)->getItemClass() == IC_GUN)
		{
			/* Empty out the bullets put in by CreateItem().  We now sell all guns
			 * empty of bullets.  This is done for BobbyR simply to be consistent with
			 * the dealers in Arulco, who must sell guns empty to prevent ammo cheats
			 * by players. */
			Object.ubGunShotsLeft = 0;
		}

		//add all the items that were purchased
		UINT8 const ubItemsPurchased = purchase->ubNumberPurchased;
		UINT8       ubItemsDelivered = 0;
		for (UINT8 j = 0; j < ubItemsPurchased; ++j)
		{
			// Pablos might steal stuff but only:
			// - if it's one of a group of items
			// - if he didn't steal the previous item in the group (so he never steals > 50%)
			// - if he has been bribed, he only sneaks out stuff which is cheap
			if (fSectorLoaded)
			{
				// add ubItemsPurchased to the chance of theft so the chance increases when there are more items of a kind being ordered
				if (!fPablosStoleLastItem && uiChanceOfTheft > 0 && Random(100) < uiChanceOfTheft + ubItemsPurchased)
				{
					++uiStolenCount;
					fPablosStoleSomething = TRUE;
					fPablosStoleLastItem  = TRUE;
				}
				else
				{
					fPablosStoleLastItem = FALSE;

					if (usStandardMapPos == LOST_SHIPMENT_GRIDNO)
					{
						// damage the item a random amount!
						const INT8 status = (70 + Random(11)) * (INT32)Object.bStatus[0] / 100;
						Object.bStatus[0] = MAX(1, status);
						AddItemToPool(usStandardMapPos, &Object, INVISIBLE, 0, 0, 0);
					}
					else
					{
						// record # delivered for later addition...
						++ubItemsDelivered;
					}
				}
			}
			else
			{
				if (j > 1 && !fPablosStoleLastItem && uiChanceOfTheft > 0 && Random(100) < uiChanceOfTheft + j)
				{
					pStolenObject[uiStolenCount] = Object;
					++uiStolenCount;
					fPablosStoleSomething = TRUE;
					fPablosStoleLastItem  = TRUE;
				}
				else
				{
					fPablosStoleLastItem = FALSE;

					/* else we are not currently in the sector, so we build an array of
					 * items to add in one lump add the item to the item array */
					if (usStandardMapPos == LOST_SHIPMENT_GRIDNO)
					{
						// damage the item a random amount!
						const INT8 status = (70 + Random(11)) * (INT32)Object.bStatus[0] / 100;
						Object.bStatus[0] = MAX(1, status);
						pObject[uiCount++] = Object;
					}
					else
					{
						++ubItemsDelivered;
					}
				}
			}
		}

		if (purchase->ubNumberPurchased == 1 && ubItemsDelivered == 1)
		{
			// the item in Object will be the item to deliver
			if (fSectorLoaded)
			{
				AddItemToPool(usStandardMapPos, &Object, INVISIBLE, 0, 0, 0);
			}
			else
			{
				pObject[uiCount++] = Object;
			}
		}
		else
		{
			while (ubItemsDelivered)
			{
				// treat 0s as 1s :-)
				const UINT8 ubTempNumItems = __min(ubItemsDelivered, __max(1, GCM->getItem(usItem)->getPerPocket()));
				CreateItems(usItem, purchase->bItemQuality, ubTempNumItems, &Object);

				// stack as many as possible
				if (fSectorLoaded)
				{
					AddItemToPool(usStandardMapPos, &Object, INVISIBLE, 0, 0, 0);
				}
				else
				{
					pObject[uiCount++] = Object;
				}

				ubItemsDelivered -= ubTempNumItems;
			}
		}
	}

	//if we are NOT currently in the sector
	if (!fSectorLoaded)
	{
		//add all the items from the array that was built above
		//The item are to be added to the Top part of Drassen, grid loc's  10112, 9950
		AddItemsToUnLoadedSector(dest->deliverySector, usStandardMapPos, uiCount, pObject, 0, 0, 0, INVISIBLE);
		if (uiStolenCount > 0)
		{
			AddItemsToUnLoadedSector(dest->deliverySector, PABLOS_STOLEN_DEST_GRIDNO, uiStolenCount, pStolenObject, 0, 0, 0, INVISIBLE);
		}
		delete[] pObject;
		delete[] pStolenObject;
	}

	if (fPablosStoleSomething)
	{
		SetFactTrue(FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT);
	}
	else
	{
		SetFactFalse(FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT);
	}

	SetFactFalse(FACT_LARGE_SIZED_OLD_SHIPMENT_WAITING);

	if (CheckFact(FACT_NEXT_PACKAGE_CAN_BE_DELAYED, 0))
	{
		SetFactFalse(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
		SetFactFalse(FACT_LARGE_SIZED_SHIPMENT_WAITING);
		SetFactFalse(FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING);
	}
	else
	{
		if (usNumberOfItems - uiStolenCount <= 5)
		{
			SetFactFalse(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
			SetFactFalse(FACT_LARGE_SIZED_SHIPMENT_WAITING);
		}
		else if (usNumberOfItems - uiStolenCount <= 15)
		{
			SetFactTrue(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
			SetFactFalse(FACT_LARGE_SIZED_SHIPMENT_WAITING);
		}
		else
		{
			SetFactFalse(FACT_MEDIUM_SIZED_SHIPMENT_WAITING);
			SetFactTrue(FACT_LARGE_SIZED_SHIPMENT_WAITING);
		}

		// this shipment isn't old yet...
		SetFactTrue(FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING);

		// set up even to make shipment "old"
		AddSameDayStrategicEvent(EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay() + 120, FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING);
	}

	//We have received the shipment so fActice becomes fALSE
	shipment->fActive = FALSE;

	//Stop time compression the game
	StopTimeCompression();

	//if the shipment is NOT from John Kulba, send an email
	if (!fThisShipmentIsFromJohnKulba)
	{
		//Add an email from Bobby r telling the user the shipment 'Should' be there
		AddEmail(dest->emailOffset, dest->emailLength, BOBBY_R, GetWorldTotalMin());
	}
	else
	{
		//if the shipment is from John Kulba
		//Add an email from kulba telling the user the shipment is there
		AddEmail(JOHN_KULBA_GIFT_IN_DRASSEN, JOHN_KULBA_GIFT_IN_DRASSEN_LENGTH, JOHN_KULBA, GetWorldTotalMin());
	}
}


static void HandleDelayedItemsArrival(UINT32 uiReason)
{
	// This function moves all the items that Pablos has stolen
	// (or items that were delayed) to the arrival location for new shipments,
	INT16			sStartGridNo;
	UINT8			ubLoop;
	OBJECTTYPE Object;
	auto shippingDest = GCM->getPrimaryShippingDestination();

	if (uiReason == NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_RETURN_STOLEN_SHIPMENT_ITEMS )
	{
		if ( gMercProfiles[ PABLO ].bMercStatus == MERC_IS_DEAD )
		{
			// nothing arrives then!
			return;
		}
		// update some facts...
		SetFactTrue( FACT_PABLO_RETURNED_GOODS );
		SetFactFalse( FACT_PABLO_PUNISHED_BY_PLAYER );
		sStartGridNo = PABLOS_STOLEN_DEST_GRIDNO;

		// add random items
		for (ubLoop = 0; ubLoop < 2; ubLoop++)
		{
			switch( Random( 10 ) )
			{
				case 0:
					// 1 in 10 chance of a badly damaged gas mask
					CreateItem( GASMASK, (INT8) (20 + Random( 10 )), &Object );
					break;
				case 1:
				case 2:
					// 2 in 10 chance of a battered Desert Eagle
					CreateItem( DESERTEAGLE, (INT8) (40 + Random( 10 )), &Object );
					break;
				case 3:
				case 4:
				case 5:
					// 3 in 10 chance of a stun grenade
					CreateItem( STUN_GRENADE, (INT8) (70 + Random( 10 )), &Object );
					break;
				case 6:
				case 7:
				case 8:
				case 9:
					// 4 in 10 chance of two 38s!
					CreateItems( SW38, (INT8) (90 + Random( 10 )), 2, &Object );
					break;
			}
			if (gWorldSector == shippingDest->deliverySector)
			{
				AddItemToPool(shippingDest->deliverySectorGridNo, &Object, INVISIBLE, 0, 0, 0);
			}
			else
			{
				AddItemsToUnLoadedSector(shippingDest->deliverySector, shippingDest->deliverySectorGridNo, 1, &Object, 0, 0, 0, INVISIBLE);
			}
		}
	}
	else if (uiReason == FACT_PACKAGE_DAMAGED)
	{
		sStartGridNo = LOST_SHIPMENT_GRIDNO;
	}
	else
	{
		return;
	}

	// If the Drassen airport sector is already loaded, move the item pools...
	if (gWorldSector == shippingDest->deliverySector)
	{
		// sector is loaded!
		// just move the hidden item pool
		MoveItemPools( sStartGridNo, shippingDest->deliverySectorGridNo );
	}
	else
	{
		// otherwise load the saved items from the item file and change the records of their locations
		std::vector<WORLDITEM> pTemp = LoadWorldItemsFromTempItemFile(shippingDest->deliverySector);

		for (WORLDITEM& wi : pTemp)
		{
			if (wi.sGridNo == PABLOS_STOLEN_DEST_GRIDNO)
			{
				wi.sGridNo = shippingDest->deliverySectorGridNo;
			}
		}
		SaveWorldItemsToTempItemFile(shippingDest->deliverySector, pTemp);
	}
}


void AddSecondAirportAttendant( void )
{
	// add the second airport attendant to the Drassen airport...
	MERCPROFILESTRUCT& sal = GetProfile(SAL);
	auto shippingDest = GCM->getPrimaryShippingDestination();

	sal.sSectorX = shippingDest->deliverySector.x;
	sal.sSectorY = shippingDest->deliverySector.y;
	sal.bSectorZ = shippingDest->deliverySector.z;
}


static void SetPabloToUnbribed(void)
{
	if (guiPabloExtraDaysBribed > 0)
	{
		// set new event for later on, because the player gave Pablo more money!
		AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay(), FACT_PABLOS_BRIBED, guiPabloExtraDaysBribed );
		guiPabloExtraDaysBribed = 0;
	}
	else
	{
		SetFactFalse( FACT_PABLOS_BRIBED );
	}
}


static void HandlePossiblyDamagedPackage(void)
{
	if (Random( 100 ) < 70)
	{
		SetFactTrue( FACT_PACKAGE_DAMAGED );
		HandleDelayedItemsArrival( FACT_PACKAGE_DAMAGED );
	}
	else
	{
		// shipment lost forever!
		SetFactTrue( FACT_PACKAGE_LOST_PERMANENTLY );
	}
	// whatever happened, the shipment is no longer delayed
	SetFactFalse( FACT_SHIPMENT_DELAYED_24_HOURS );
}

void CheckForKingpinsMoneyMissing( BOOLEAN fFirstCheck )
{
	UINT32				uiTotalCash = 0;
	BOOLEAN				fKingpinWillDiscover = FALSE, fKingpinDiscovers = FALSE;

	// money in D5b1 must be less than 30k
	CFOR_EACH_WORLD_ITEM(wi)
	{
		OBJECTTYPE const& o = wi.o;
		if (o.usItem == MONEY) uiTotalCash += o.uiMoneyAmount;
	}

	// This function should be called every time sector D5/B1 is unloaded!
	if ( fFirstCheck )
	{
		if ( CheckFact( FACT_KINGPIN_WILL_LEARN_OF_MONEY_GONE, 0 ) == TRUE )
		{
			// unnecessary
			return;
		}

		if ( uiTotalCash < 30000 )
		{
			// add history log here
			AddHistoryToPlayersLog(HISTORY_FOUND_MONEY, 0, GetWorldTotalMin(), gWorldSector);

			SetFactTrue( FACT_KINGPIN_WILL_LEARN_OF_MONEY_GONE );
		}
	}

	if ( CheckFact( FACT_KINGPIN_DEAD, 0 ) == TRUE )
	{
		return;
	}

	if ( uiTotalCash < 30000 )
	{
		if ( fFirstCheck )
		{
			// add event to make Kingpin aware, two days from now
			fKingpinWillDiscover = TRUE;
		}
		else
		{
			fKingpinDiscovers = TRUE;
		}
	}

	if ( fKingpinWillDiscover )
	{
		// set event for next day to check for real
		AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, Random( 120 ), FACT_KINGPIN_KNOWS_MONEY_GONE, 1 );

		// the sector is unloaded NOW so set Kingpin's balance and remove the cash
		gMercProfiles[ KINGPIN ].iBalance = - (30000 - (INT32) uiTotalCash);
		// remove all money from map
		FOR_EACH_WORLD_ITEM(wi)
		{
			if (wi.o.usItem == MONEY) wi.fExists = FALSE; // remove!
		}
	}
	else if ( fKingpinDiscovers )
	{
		// ok start things up here!
		SetFactTrue( FACT_KINGPIN_KNOWS_MONEY_GONE );

		// set event 2 days from now that if the player has not given Kingpin his money back,
		// he sends email to the player
		AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, Random( 120 ), FACT_KINGPIN_KNOWS_MONEY_GONE, 2 );
	}

}

void HandleNPCSystemEvent( UINT32 uiEvent )
{
	if (uiEvent < NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS)
	{
		switch( uiEvent )
		{
			case FACT_PABLOS_BRIBED:
				// set Pacos to unbribed
				SetPabloToUnbribed();
				break;

			case FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING:
				// the shipment is no longer really new
				SetFactFalse( FACT_REALLY_NEW_BOBBYRAY_SHIPMENT_WAITING );
				if (CheckFact( FACT_LARGE_SIZED_SHIPMENT_WAITING, 0 ))
				{
					// set "really heavy old shipment" fact
					SetFactTrue( FACT_LARGE_SIZED_OLD_SHIPMENT_WAITING );
				}
				break;

			case FACT_SHIPMENT_DELAYED_24_HOURS:
			case FACT_24_HOURS_SINCE_DOCTOR_TALKED_TO:
			case FACT_24_HOURS_SINCE_JOEY_RESCUED:
				SetFactTrue((Fact)uiEvent);
				break;

			case FACT_KINGPIN_KNOWS_MONEY_GONE:
				// more generally events for kingpin quest
				if (!CheckFact(FACT_KINGPIN_KNOWS_MONEY_GONE, 0))
				{
					// check for real whether to start quest
					CheckForKingpinsMoneyMissing( FALSE );
				}
				else if (!CheckFact(FACT_KINGPIN_DEAD, 0))
				{
					if ( gubQuest[ QUEST_KINGPIN_MONEY ] == QUESTNOTSTARTED )
					{
						// KP knows money is gone, hasn't told player, if this event is called then the 2
						// days are up... send email
						AddEmail( KING_PIN_LETTER, KING_PIN_LETTER_LENGTH, KING_PIN, GetWorldTotalMin() );
						StartQuest(QUEST_KINGPIN_MONEY, SGPSector(5, MAP_ROW_D));
						// add event to send terrorists two days from now
						AddFutureDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, Random( 120 ), FACT_KINGPIN_KNOWS_MONEY_GONE, 2 );
					}
					else if ( gubQuest[ QUEST_KINGPIN_MONEY ] == QUESTINPROGRESS )
					{
						// knows money gone, quest is still in progress
						// event indicates Kingpin can start to send terrorists
						SetFactTrue( FACT_KINGPIN_CAN_SEND_ASSASSINS );
						gMercProfiles[ SPIKE ].sSectorX = 5;
						gMercProfiles[ SPIKE ].sSectorY = MAP_ROW_C;
						gTacticalStatus.fCivGroupHostile[ KINGPIN_CIV_GROUP ] = CIV_GROUP_WILL_BECOME_HOSTILE;
					}
				}
				break;
		}
	}
	else
	{
		switch( uiEvent - NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS )
		{
			case NPC_ACTION_RETURN_STOLEN_SHIPMENT_ITEMS:
				HandleDelayedItemsArrival( uiEvent );
				break;
			case NPC_ACTION_SET_RANDOM_PACKAGE_DAMAGE_TIMER:
				HandlePossiblyDamagedPackage();
				break;
			case NPC_ACTION_ENABLE_CAMBRIA_DOCTOR_BONUS:
				SetFactTrue( FACT_WILLIS_HEARD_ABOUT_JOEY_RESCUE );
				break;
			case NPC_ACTION_TRIGGER_END_OF_FOOD_QUEST:
				if ( gMercProfiles[ FATHER ].bMercStatus != MERC_IS_DEAD )
				{
					EndQuest(QUEST_FOOD_ROUTE, SGPSector(10, MAP_ROW_A));
					SetFactTrue( FACT_FOOD_QUEST_OVER );
				}
				break;
			case NPC_ACTION_DELAYED_MAKE_BRENDA_LEAVE:
				//IC:
				//TriggerNPCRecord(BRENDA, 9);
				SetFactTrue( FACT_BRENDA_PATIENCE_TIMER_EXPIRED );
				break;
			case NPC_ACTION_SET_DELAY_TILL_GIRLS_AVAILABLE:
				HandleNPCDoAction( 107, NPC_ACTION_SET_GIRLS_AVAILABLE, 0 );
				break;

			case NPC_ACTION_READY_ROBOT:
				{
					if ( CheckFact( FACT_FIRST_ROBOT_DESTROYED, 0 ) )
					{
						// second robot ready
						SetFactTrue( FACT_ROBOT_READY_SECOND_TIME );
						// resurrect robot
						gMercProfiles[ ROBOT ].bLife = gMercProfiles[ ROBOT ].bLifeMax;
						gMercProfiles[ ROBOT ].bMercStatus = MERC_OK;
					}
					else
					{
						// first robot ready
						SetFactTrue( FACT_ROBOT_READY );
					}

					gMercProfiles[ ROBOT ].sSectorX = gMercProfiles[ MADLAB ].sSectorX;
					gMercProfiles[ ROBOT ].sSectorY = gMercProfiles[ MADLAB ].sSectorY;
					gMercProfiles[ ROBOT ].bSectorZ = gMercProfiles[ MADLAB ].bSectorZ;


				}
				break;

			case NPC_ACTION_ADD_JOEY_TO_WORLD:
				// If Joey is not dead, escorted, or already delivered
				if ( gMercProfiles[ JOEY ].bMercStatus != MERC_IS_DEAD && !CheckFact( FACT_JOEY_ESCORTED, 0 ) &&
					gMercProfiles[ JOEY ].sSectorX == 4 &&
					gMercProfiles[ JOEY ].sSectorY == MAP_ROW_D &&
					gMercProfiles[ JOEY ].bSectorZ == 1 )
				{
					const SOLDIERTYPE* const pJoey = FindSoldierByProfileID(JOEY);
					if (pJoey )
					{
						// he's in the currently loaded sector...delay this an hour!
						AddSameDayStrategicEvent( EVENT_SET_BY_NPC_SYSTEM, GetWorldMinutesInDay() + 60, NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS + NPC_ACTION_ADD_JOEY_TO_WORLD );
					}
					else
					{
						// move Joey from caves to San Mona
						gMercProfiles[ JOEY ].sSectorX = 5;
						gMercProfiles[ JOEY ].sSectorY = MAP_ROW_C;
						gMercProfiles[ JOEY ].bSectorZ = 0;
					}
				}
				break;

			case NPC_ACTION_SEND_ENRICO_MIGUEL_EMAIL:
				AddEmail( ENRICO_MIGUEL, ENRICO_MIGUEL_LENGTH, MAIL_ENRICO, GetWorldTotalMin() );
				break;

			case NPC_ACTION_TIMER_FOR_VEHICLE:
				SetFactTrue( FACT_OK_USE_HUMMER );
				break;

			case NPC_ACTION_FREE_KIDS:
				SetFactTrue( FACT_KIDS_ARE_FREE );
				break;

			default:
				break;
		}
	}
}

void HandleEarlyMorningEvents( void )
{
	UINT32					cnt;
	UINT32					uiAmount;

	// loop through all *NPCs* and reset "default response used recently" flags
	for (const MercProfile* profile : GCM->listMercProfiles())
	{
		if (!profile->isNPCorRPC()) continue;

		MERCPROFILESTRUCT& p = profile->getStruct();
		p.bFriendlyOrDirectDefaultResponseUsedRecently = FALSE;
		p.bRecruitDefaultResponseUsedRecently = FALSE;
		p.bThreatenDefaultResponseUsedRecently = FALSE;
		p.ubMiscFlags2 &= (~PROFILE_MISC_FLAG2_BANDAGED_TODAY);
	}
	// reset Father Walker's drunkenness level!
	gMercProfiles[ FATHER ].bNPCData = (INT8) Random( 4 );
	// set Walker's location
	if ( Random( 2 ) )
	{
		// move the father to the other sector, provided neither are loaded
		static const SGPSector swapSector1(13, MAP_ROW_C);
		static const SGPSector swapSector2(13, MAP_ROW_D);
		if (gWorldSector != swapSector1 && gWorldSector != swapSector2)
		{
			gMercProfiles[ FATHER ].sSectorX = 13;
			// swap his location
			if (gMercProfiles[ FATHER ].sSectorY == MAP_ROW_C)
			{
				gMercProfiles[ FATHER ].sSectorY = MAP_ROW_D;
			}
			else
			{
				gMercProfiles[ FATHER ].sSectorY = MAP_ROW_C;
			}
		}
	}

	static const SGPSector swapSector3(5, MAP_ROW_C);
	if (gMercProfiles[TONY].ubLastDateSpokenTo > 0 && gWorldSector != swapSector3)
	{
		// San Mona C5 is not loaded so make Tony possibly not available
		if (Random( 4 ))
		{
			// Tony IS available
			SetFactFalse( FACT_TONY_NOT_AVAILABLE );
			gMercProfiles[ TONY ].sSectorX = 5;
			gMercProfiles[ TONY ].sSectorY = MAP_ROW_C;
		}
		else
		{
			// Tony is NOT available
			SetFactTrue( FACT_TONY_NOT_AVAILABLE );
			gMercProfiles[ TONY ].sSectorX = 0;
			gMercProfiles[ TONY ].sSectorY = 0;
		}
	}


	if ( gMercProfiles[ DEVIN ].ubLastDateSpokenTo == 0 )
	{
		// Does Devin move?
		gMercProfiles[ DEVIN ].bNPCData++;
		if ( gMercProfiles[ DEVIN ].bNPCData > 3 )
		{
			SGPSector swapSector4(gMercProfiles[DEVIN].sSectorX, gMercProfiles[DEVIN].sSectorY);
			if (gWorldSector != swapSector4)
			{
				// ok, Devin's sector not loaded, so time to move!
				// might be same sector as before, if so, oh well!
				auto placement = GCM->getNpcPlacement(DEVIN);
				UINT8 sector   = placement->pickPlacementSector();
				gMercProfiles[DEVIN].sSectorX = SECTORX(sector);
				gMercProfiles[DEVIN].sSectorY = SECTORY(sector);
			}
		}
	}

	// Does Hamous move?

	// stop moving the truck if Hamous is dead!!
	// stop moving them if the player has the truck or Hamous is hired!
	SGPSector swapSector5(gMercProfiles[HAMOUS].sSectorX, gMercProfiles[HAMOUS].sSectorY);
	if (gMercProfiles[HAMOUS].bLife > 0 &&
			FindSoldierByProfileIDOnPlayerTeam(HAMOUS)        == NULL &&
			FindSoldierByProfileIDOnPlayerTeam(PROF_ICECREAM) == NULL &&
			gWorldSector != swapSector5)
	{
		// ok, HAMOUS's sector not loaded, so time to move!
		// might be same sector as before, if so, oh well!
		auto placement = GCM->getNpcPlacement(HAMOUS);
		UINT8 sector   = placement->pickPlacementSector();
		gMercProfiles[HAMOUS].sSectorX = SECTORX(sector);
		gMercProfiles[HAMOUS].sSectorY = SECTORY(sector);
		gMercProfiles[PROF_ICECREAM].sSectorX = SECTORX(sector);
		gMercProfiles[PROF_ICECREAM].sSectorY = SECTORY(sector);
	}

	// Does Rat take off?
	if ( gMercProfiles[ RAT ].bNPCData != 0 )
	{
		gMercProfiles[ RAT ].sSectorX = 0;
		gMercProfiles[ RAT ].sSectorY = 0;
		gMercProfiles[ RAT ].bSectorZ = 0;
	}


	// Empty money from pockets of Vince 69, Willis 80, and Jenny 132
	SetMoneyInSoldierProfile( VINCE, 0 );
	SetMoneyInSoldierProfile( STEVE, 0 ); // Steven Willis
	SetMoneyInSoldierProfile( JENNY, 0 );

	// Vince is no longer expecting money
	SetFactFalse( FACT_VINCE_EXPECTING_MONEY );

	// Reset Darren's balance and money
	gMercProfiles[ DARREN ].iBalance = 0;
	SetMoneyInSoldierProfile( DARREN, 15000 );

	// set Carmen to be placed on the map in case he moved and is waiting off screen
	if (gMercProfiles[ CARMEN ].ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR)
	{
		gMercProfiles[ CARMEN ].ubMiscFlags2 &= ~(PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR);
		// move Carmen to C13
		gMercProfiles[ CARMEN ].sSectorX = 13;
		gMercProfiles[ CARMEN ].sSectorY = MAP_ROW_C;
		gMercProfiles[ CARMEN ].bSectorZ = 0;

		// we should also reset # of terrorist heads and give him cash
		if (gMercProfiles[ CARMEN ].bNPCData2 > 0)
		{
			if (gMercProfiles[ CARMEN ].uiMoney < 10000)
			{
				uiAmount = 0;
			}
			else
			{
				uiAmount = gMercProfiles[ CARMEN ].uiMoney;
			}
			uiAmount += 10000 * gMercProfiles[ CARMEN ].bNPCData2;
			SetMoneyInSoldierProfile( CARMEN, uiAmount );
			gMercProfiles[ CARMEN ].bNPCData2 = 0;

			for ( cnt = HEAD_1; cnt <= HEAD_7; cnt++ )
			{
				RemoveObjectFromSoldierProfile( CARMEN, (UINT8) cnt );
			}

		}
	}
	else
	{
		// randomize where he'll be today... so long as his sector's not loaded
		SGPSector swapSector6(gMercProfiles[CARMEN].sSectorX, gMercProfiles[CARMEN].sSectorY);
		if (gWorldSector != swapSector6)
		{
			auto placement = GCM->getNpcPlacement(CARMEN);
			UINT8 sector   = placement->pickPlacementSector();
			gMercProfiles[CARMEN].sSectorX = SECTORX(sector);
			gMercProfiles[CARMEN].sSectorY = SECTORY(sector);

			// he should have $5000... unless the player forgot to meet him
			if (gMercProfiles[ CARMEN ].uiMoney < 5000)
			{
				SetMoneyInSoldierProfile( CARMEN, 5000 );
			}
		}
	}

	if ( PreRandom( 3 ) == 0 )
	{
		SetFactTrue( FACT_DAVE_HAS_GAS );
	}
	else
	{
		SetFactFalse( FACT_DAVE_HAS_GAS );
	}

	static const SGPSector hospital(HOSPITAL_SECTOR_X, HOSPITAL_SECTOR_Y, HOSPITAL_SECTOR_Z);
	if (gWorldSector == hospital)
	{
		CheckForMissingHospitalSupplies();
	}

}

void MakeCivGroupHostileOnNextSectorEntrance( UINT8 ubCivGroup )
{
	// if it's the rebels that will become hostile, reduce town loyalties NOW, not later
	if ( ubCivGroup == REBEL_CIV_GROUP && gTacticalStatus.fCivGroupHostile[ ubCivGroup ] == CIV_GROUP_NEUTRAL )
	{
		ReduceLoyaltyForRebelsBetrayed();
	}

	gTacticalStatus.fCivGroupHostile[ ubCivGroup ] = CIV_GROUP_WILL_BECOME_HOSTILE;
}

void RemoveAssassin( UINT8 ubProfile )
{
	gMercProfiles[ ubProfile ].sSectorX = 0;
	gMercProfiles[ ubProfile ].sSectorY = 0;
	gMercProfiles[ ubProfile ].bLife = gMercProfiles[ ubProfile ].bLifeMax;
}

void CheckForMissingHospitalSupplies( void )
{
	UINT8					ubMedicalObjects = 0;

	CFOR_EACH_WORLD_ITEM(wi)
	{
		// loop through all items, look for ownership
		if (wi.o.usItem != OWNERSHIP || wi.o.ubOwnerCivGroup != DOCTORS_CIV_GROUP) continue;

		const ITEM_POOL* pItemPool = GetItemPool(wi.sGridNo, 0);
		while( pItemPool )
		{
			OBJECTTYPE const& o = GetWorldItem(pItemPool->iItemIndex).o;
			if (o.bStatus[0] > 60)
			{
				if (o.usItem == FIRSTAIDKIT || o.usItem == MEDICKIT || o.usItem == REGEN_BOOSTER || o.usItem == ADRENALINE_BOOSTER)
				{
					ubMedicalObjects++;
				}
			}

			pItemPool = pItemPool->pNext;
		}
	}

	if ( CheckFact( FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN, 0 ) == TRUE )
	{
		// player returning stuff!  if back to full then can operate
		if ( ubMedicalObjects >= gubCambriaMedicalObjects )
		{
			SetFactFalse( FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN );
			SetFactFalse( FACT_PLAYER_STOLE_MEDICAL_SUPPLIES );
			return;
		}
	}

	if ( ubMedicalObjects < gubCambriaMedicalObjects )
	{
		// player's stolen something!
		if (!CheckFact(FACT_PLAYER_STOLE_MEDICAL_SUPPLIES, 0))
		{
			SetFactTrue( FACT_PLAYER_STOLE_MEDICAL_SUPPLIES );
		}

		// if only 1/5 or less left, give up the ghost
		if ( ubMedicalObjects * 5 <= gubCambriaMedicalObjects )
		{
			// run out!
			SetFactTrue( FACT_PLAYER_STOLE_MEDICAL_SUPPLIES_AGAIN );
		}
	}

}


static void DropOffItemsInDestination(UINT8 ubOrderNum, const ShippingDestinationModel* shippingDest)
{
	OBJECTTYPE		Object;
	UINT32	uiCount = 0;
	OBJECTTYPE	*pObject=NULL;
	UINT16	usNumberOfItems=0, usItem;
	UINT8		ubItemsDelivered, ubTempNumItems;
	UINT32	i;

	//if the player doesnt "own" the sector,
	if (StrategicMap[SECTOR_INFO_TO_STRATEGIC_INDEX(shippingDest->getDeliverySector())].fEnemyControlled)
	{
		//the items disappear
		gpNewBobbyrShipments[ ubOrderNum ].fActive = FALSE;
		return;
	}

	const BOOLEAN fSectorLoaded =
		CloseCrate(shippingDest->deliverySector, shippingDest->deliverySectorGridNo);

	for(i=0; i<gpNewBobbyrShipments[ ubOrderNum ].ubNumberPurchases; i++)
	{
		// Count how many items were purchased
		usNumberOfItems += gpNewBobbyrShipments[ ubOrderNum ].BobbyRayPurchase[i].ubNumberPurchased;
	}

	//if we are NOT currently in the right sector
	if( !fSectorLoaded )
	{
		//build an array of objects to be added
		pObject = new OBJECTTYPE[usNumberOfItems]{};
	}


	uiCount = 0;

	//loop through the number of purchases
	for (i = 0; i < gpNewBobbyrShipments[0].ubNumberPurchases; i++)// FIXME shipment ubOrderNum instead of 0
	{
		ubItemsDelivered = gpNewBobbyrShipments[ ubOrderNum ].BobbyRayPurchase[i].ubNumberPurchased;
		usItem = gpNewBobbyrShipments[ ubOrderNum ].BobbyRayPurchase[i].usItemIndex;

		while ( ubItemsDelivered )
		{
			// treat 0s as 1s :-)
			ubTempNumItems = __min( ubItemsDelivered, __max( 1, GCM->getItem(usItem )->getPerPocket() ) );
			CreateItems( usItem, gpNewBobbyrShipments[ ubOrderNum ].BobbyRayPurchase[i].bItemQuality, ubTempNumItems, &Object );

			// stack as many as possible
			if( fSectorLoaded )
			{
				AddItemToPool(shippingDest->deliverySectorGridNo, &Object, INVISIBLE, 0, 0, 0);
			}
			else
			{
				pObject[uiCount] = Object;
				uiCount++;
			}

			ubItemsDelivered -= ubTempNumItems;
		}
	}

	//if the sector WASNT loaded
	if( !fSectorLoaded )
	{
		//add all the items from the array that was built above

		//The item are to be added to the Top part of Drassen, grid loc's  10112, 9950
		AddItemsToUnLoadedSector(shippingDest->deliverySector, shippingDest->deliverySectorGridNo, uiCount, pObject, 0, 0, 0, INVISIBLE);
		delete[] pObject;
		pObject = NULL;
	}

	//mark that the shipment has arrived
	gpNewBobbyrShipments[ ubOrderNum ].fActive = FALSE;

	if (shippingDest->emailOffset) {
		//Add an email telling the user the shipment is there
		AddEmail(shippingDest->emailOffset, shippingDest->emailLength, BOBBY_R, GetWorldTotalMin());
	}
	else
	{
		SLOGW("Bobby Ray shipment arrived but no email template found.");
	}
}
