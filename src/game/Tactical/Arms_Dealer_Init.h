#ifndef _ARMS_DEALERS_INIT__H_
#define _ARMS_DEALERS_INIT__H_

#include "Arms_Dealer.h"
#include "Item_Types.h"
#include "JA2Types.h"

#include <vector>


//The following defines indicate what items can be sold by the arms dealer
#define ARMS_DEALER_HANDGUNCLASS	0x00000001
#define ARMS_DEALER_SMGCLASS		0x00000002
#define ARMS_DEALER_RIFLECLASS		0x00000004
#define ARMS_DEALER_MGCLASS		0x00000008
#define ARMS_DEALER_SHOTGUNCLASS	0x00000010

#define ARMS_DEALER_KNIFECLASS		0x00000020

#define ARMS_DEALER_BLADE		0x00000040
#define ARMS_DEALER_LAUNCHER		0x00000080

#define ARMS_DEALER_ARMOUR		0x00000100
#define ARMS_DEALER_MEDKIT		0x00000200
#define ARMS_DEALER_MISC		0x00000400
#define ARMS_DEALER_AMMO		0x00000800

#define ARMS_DEALER_GRENADE		0x00001000
#define ARMS_DEALER_BOMB		0x00002000
#define ARMS_DEALER_EXPLOSV		0x00004000

#define ARMS_DEALER_KIT		0x00008000

#define ARMS_DEALER_FACE		0x00010000
//#define ARMS_DEALER_THROWN		0x00020000
//#define ARMS_DEALER_KEY		0x00040000

//#define ARMS_DEALER_VIDEO_CAMERA	0x00020000

#define ARMS_DEALER_DETONATORS		0x00040000

#define ARMS_DEALER_ATTACHMENTS	0x00080000


#define ARMS_DEALER_ALCOHOL		0x00100000
#define ARMS_DEALER_ELECTRONICS	0x00200000
#define ARMS_DEALER_HARDWARE		0x00400000 | ARMS_DEALER_KIT

#define ARMS_DEALER_MEDICAL		0x00800000 | ARMS_DEALER_MEDKIT

//#define ARMS_DEALER_EMPTY_JAR	0x01000000
#define ARMS_DEALER_CREATURE_PARTS	0x02000000
#define ARMS_DEALER_ROCKET_RIFLE	0x04000000


#define ARMS_DEALER_ALL_GUNS		ARMS_DEALER_HANDGUNCLASS | ARMS_DEALER_SMGCLASS | ARMS_DEALER_RIFLECLASS | ARMS_DEALER_MGCLASS | ARMS_DEALER_SHOTGUNCLASS

#define ARMS_DEALER_BIG_GUNS		ARMS_DEALER_SMGCLASS | ARMS_DEALER_RIFLECLASS | ARMS_DEALER_MGCLASS | ARMS_DEALER_SHOTGUNCLASS

#define ARMS_DEALER_ALL_WEAPONS	ARMS_DEALER_ALL_GUNS | ARMS_DEALER_BLADE | ARMS_DEALER_LAUNCHER | ARMS_DEALER_KNIFECLASS






//
// Specific Dealer Flags
// NOTE: Each dealer has 8 flags, but different dealers can and SHOULD share the same flag #s!
//

// Alex Fredo
#define ARMS_DEALER_FLAG__FREDO_HAS_SAID_ROCKET_RIFLE_QUOTE 0x00000001 // Alex Fredo has already repaired the Rocket Rifle
// Franz Hinkle
#define ARMS_DEALER_FLAG__FRANZ_HAS_SOLD_VIDEO_CAMERA_TO_PLAYER 0x00000001 // Franz Hinkle has sold the video camera to the player



class DealerModel;

// THIS STRUCTURE GETS SAVED/RESTORED/RESET
struct ARMS_DEALER_STATUS
{
	UINT32  uiArmsDealersCash; // How much money the arms dealer currently has

	UINT8   ubSpecificDealerFlags; // Misc state flags for specific dealers
	BOOLEAN fOutOfBusiness; // Set when a dealer has been killed, etc.
	BOOLEAN fRepairDelayBeenUsed; // Set when a repairman has missed his repair time estimate & given his excuse for it
	BOOLEAN fUnusedKnowsPlayer; // Set if the shopkeeper has met with the player before [UNUSED] // XXX HACK000B

	UINT32  uiTimePlayerLastInSKI; // game time (in total world minutes) when player last talked to this dealer in SKI

	UINT8   ubPadding[ 8 ]; // XXX HACK000B
};


struct SPECIAL_ITEM_INFO
{
	UINT16 usAttachment[MAX_ATTACHMENTS]; // item index of any attachments on the item

	INT8   bItemCondition;	// if 0, no item is stored
				// from 1 to 100 indicates an item with that status
				// -1 to -100 means the item is in for repairs, flip sign for the actual status

	UINT8  ubImprintID; // imprint ID for imprinted items (during repair!)

	INT8   bAttachmentStatus[MAX_ATTACHMENTS]; // status of any attachments on the item

	UINT8  ubPadding[2]; // filler // XXX HACK000B
};


struct DEALER_SPECIAL_ITEM
{
	// Individual "special" items are stored here as needed, *one* per slot
	// An item is special if it is used (status < 100), has been imprinted, or has a permanent attachment

	SPECIAL_ITEM_INFO Info;

	UINT32  uiRepairDoneTime; // If the item is in for repairs, this holds the time when it will be repaired (in min)

	BOOLEAN fActive; // TRUE means an item is stored here (empty elements may not always be freed immediately)

	UINT8   ubOwnerProfileId; // stores which merc previously owned an item being repaired

	UINT8   ubPadding[6]; // filler // XXX HACK000B
};


struct DEALER_ITEM_HEADER
{
	// Non-special items are all the identical and are totaled inside ubPerfectItems.
	// Items being repaired are also stored here, with a negative condition.
	// NOTE: special item elements may remain allocated long after item has been removed, to reduce memory fragmentation!!!

	UINT8   ubTotalItems; // sum of all the items (all perfect ones + all special ones)
	UINT8   ubPerfectItems; // non-special (perfect) items held by dealer
	UINT8   ubStrayAmmo; // partially-depleted ammo mags are stored here as #bullets, and can be converted to full packs

	std::vector<DEALER_SPECIAL_ITEM> SpecialItem; // dynamic array of special items with this same item index

	UINT32  uiOrderArrivalTime; // Day the items ordered will arrive on.  It's UINT32 in case we change this to minutes.
	UINT8   ubQtyOnOrder; // The number of items currently on order
	BOOLEAN fPreviouslyEligible; // whether or not dealer has been eligible to sell this item in days prior to today
};

extern ARMS_DEALER_STATUS gArmsDealerStatus[ NUM_ARMS_DEALERS ];
extern DEALER_ITEM_HEADER gArmsDealersInventory[ NUM_ARMS_DEALERS ][ MAXITEMS ];






void InitAllArmsDealers(void);
void ShutDownArmsDealers(void);

//Count only the # of "distinct" item types (for shopkeeper purposes)
UINT32 CountDistinctItemsInArmsDealersInventory(ArmsDealerID);
UINT16 CountTotalItemsRepairDealerHasInForRepairs(ArmsDealerID);

void AddObjectToArmsDealerInventory(ArmsDealerID, OBJECTTYPE*);

void RemoveItemFromArmsDealerInventory(ArmsDealerID, UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo, UINT8 ubHowMany);
void RemoveSpecialItemFromArmsDealerInventoryAtElement(ArmsDealerID, UINT16 usItemIndex, UINT8 ubElement);

BOOLEAN      IsMercADealer( UINT8 ubMercID );
ArmsDealerID GetArmsDealerIDFromMercID(UINT8 ubMercID);

void SaveArmsDealerInventoryToSaveGameFile(HWFILE);
void LoadArmsDealerInventoryFromSavedGameFile(HWFILE, UINT32 savegame_version);

void DailyUpdateOfArmsDealersInventory(void);

ArmsDealerType GetTypeOfArmsDealer( ArmsDealerID ubDealerID );

BOOLEAN	DoesDealerDoRepairs(ArmsDealerID);
BOOLEAN RepairmanIsFixingItemsButNoneAreDoneYet( UINT8 ubProfileID );

BOOLEAN CanDealerTransactItem(ArmsDealerID, UINT16 usItemIndex, BOOLEAN fPurchaseFromPlayer);
BOOLEAN CanDealerRepairItem(ArmsDealerID, UINT16 usItemIndex);

BOOLEAN AddDeadArmsDealerItemsToWorld(SOLDIERTYPE const*);

void		MakeObjectOutOfDealerItems( UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, OBJECTTYPE *pObject, UINT8 ubHowMany );

void GiveObjectToArmsDealerForRepair(ArmsDealerID, OBJECTTYPE const* pObject, UINT8 ubOwnerProfileId);

UINT32 CalculateObjectItemRepairTime(ArmsDealerID, OBJECTTYPE const* pItemObject);

UINT32 CalculateObjectItemRepairCost(ArmsDealerID, OBJECTTYPE const* pItemObject);


void SetSpecialItemInfoToDefaults( SPECIAL_ITEM_INFO *pSpclItemInfo );
void SetSpecialItemInfoFromObject(SPECIAL_ITEM_INFO* pSpclItemInfo, const OBJECTTYPE* pObject);

UINT16 CalcValueOfItemToDealer(ArmsDealerID, UINT16 usItemIndex, BOOLEAN fDealerSelling);

BOOLEAN DealerItemIsSafeToStack( UINT16 usItemIndex );

UINT32 CalculateOvernightRepairDelay(ArmsDealerID, UINT32 uiTimeWhenFreeToStartIt, UINT32 uiMinutesToFix);
UINT32 CalculateMinutesClosedBetween(ArmsDealerID, UINT32 uiStartTime, UINT32 uiEndTime);

void    GuaranteeAtLeastXItemsOfIndex(ArmsDealerID, UINT16 usItemIndex, UINT8 ubHowMany);
BOOLEAN ItemIsARocketRifle(INT16 sItemIndex);

extern UINT8 gubLastSpecialItemAddedAtElement;

#endif
