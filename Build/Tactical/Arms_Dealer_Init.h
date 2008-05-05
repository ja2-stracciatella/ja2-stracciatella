#ifndef _ARMS_DEALERS_INIT__H_
#define _ARMS_DEALERS_INIT__H_


#include "Item_Types.h"


//enums for the various arms dealers
enum
{
	ARMS_DEALER_TONY,
	ARMS_DEALER_FRANZ,
	ARMS_DEALER_KEITH,
	ARMS_DEALER_JAKE,
	ARMS_DEALER_GABBY,

	ARMS_DEALER_DEVIN,
	ARMS_DEALER_HOWARD,
	ARMS_DEALER_SAM,
	ARMS_DEALER_FRANK,

	ARMS_DEALER_BAR_BRO_1,
	ARMS_DEALER_BAR_BRO_2,
	ARMS_DEALER_BAR_BRO_3,
	ARMS_DEALER_BAR_BRO_4,

	ARMS_DEALER_MICKY,

	ARMS_DEALER_ARNIE,
	ARMS_DEALER_FREDO,
	ARMS_DEALER_PERKO,

// added only in GameVersion 54
	ARMS_DEALER_ELGIN,

// added only in GameVersion 55
	ARMS_DEALER_MANNY,

	NUM_ARMS_DEALERS,
};


//the enums for the different kinds of arms dealers
enum
{
	ARMS_DEALER_BUYS_SELLS,
	ARMS_DEALER_SELLS_ONLY,
	ARMS_DEALER_BUYS_ONLY,
	ARMS_DEALER_REPAIRS,
};


//The following defines indicate what items can be sold by the arms dealer
#define		ARMS_DEALER_HANDGUNCLASS			0x00000001
#define		ARMS_DEALER_SMGCLASS					0x00000002
#define		ARMS_DEALER_RIFLECLASS				0x00000004
#define		ARMS_DEALER_MGCLASS						0x00000008
#define		ARMS_DEALER_SHOTGUNCLASS			0x00000010

#define		ARMS_DEALER_KNIFECLASS				0x00000020

#define		ARMS_DEALER_BLADE							0x00000040
#define		ARMS_DEALER_LAUNCHER					0x00000080

#define		ARMS_DEALER_ARMOUR						0x00000100
#define		ARMS_DEALER_MEDKIT						0x00000200
#define		ARMS_DEALER_MISC							0x00000400
#define		ARMS_DEALER_AMMO							0x00000800

#define		ARMS_DEALER_GRENADE						0x00001000
#define		ARMS_DEALER_BOMB							0x00002000
#define		ARMS_DEALER_EXPLOSV						0x00004000

#define		ARMS_DEALER_KIT								0x00008000

#define		ARMS_DEALER_FACE							0x00010000
//#define		ARMS_DEALER_THROWN						0x00020000
//#define		ARMS_DEALER_KEY								0x00040000

//#define		ARMS_DEALER_VIDEO_CAMERA			0x00020000

#define		ARMS_DEALER_DETONATORS				0x00040000

#define		ARMS_DEALER_ATTACHMENTS				0x00080000


#define		ARMS_DEALER_ALCOHOL						0x00100000
#define		ARMS_DEALER_ELECTRONICS				0x00200000
#define		ARMS_DEALER_HARDWARE					0x00400000	| ARMS_DEALER_KIT

#define		ARMS_DEALER_MEDICAL						0x00800000	| ARMS_DEALER_MEDKIT

//#define		ARMS_DEALER_EMPTY_JAR					0x01000000
#define		ARMS_DEALER_CREATURE_PARTS		0x02000000
#define		ARMS_DEALER_ROCKET_RIFLE			0x04000000

#define		ARMS_DEALER_ONLY_USED_ITEMS		0x08000000
#define		ARMS_DEALER_GIVES_CHANGE			0x10000000		//The arms dealer will give the required change when doing a transaction
#define		ARMS_DEALER_ACCEPTS_GIFTS			0x20000000		//The arms dealer is the kind of person who will accept gifts
#define		ARMS_DEALER_SOME_USED_ITEMS		0x40000000		//The arms dealer can have used items in his inventory
#define		ARMS_DEALER_HAS_NO_INVENTORY	0x80000000		//The arms dealer does not carry any inventory


#define		ARMS_DEALER_ALL_GUNS				ARMS_DEALER_HANDGUNCLASS | ARMS_DEALER_SMGCLASS | ARMS_DEALER_RIFLECLASS | ARMS_DEALER_MGCLASS | ARMS_DEALER_SHOTGUNCLASS

#define		ARMS_DEALER_BIG_GUNS				ARMS_DEALER_SMGCLASS | ARMS_DEALER_RIFLECLASS | ARMS_DEALER_MGCLASS | ARMS_DEALER_SHOTGUNCLASS

#define		ARMS_DEALER_ALL_WEAPONS			ARMS_DEALER_ALL_GUNS | ARMS_DEALER_BLADE | ARMS_DEALER_LAUNCHER | ARMS_DEALER_KNIFECLASS






//
// Specific Dealer Flags
// NOTE: Each dealer has 8 flags, but different dealers can and SHOULD share the same flag #s!
//

// Alex Fredo
#define	ARMS_DEALER_FLAG__FREDO_HAS_SAID_ROCKET_RIFLE_QUOTE				0x00000001	// Alex Fredo has already repaired the Rocket Rifle
// Franz Hinkle
#define	ARMS_DEALER_FLAG__FRANZ_HAS_SOLD_VIDEO_CAMERA_TO_PLAYER		0x00000001	// Franz Hinkle has sold the video camera to the player


// THIS STRUCTURE HAS UNCHANGING INFO THAT DOESN'T GET SAVED/RESTORED/RESET
typedef struct
{
	union
	{
		struct
		{
			FLOAT buy;  // The price modifier used when this dealer is BUYING something.
			FLOAT sell; // The price modifier used when this dealer is SELLING something.
		} price;
		struct
		{
			FLOAT speed; // Modifier to the speed at which a repairman repairs things
			FLOAT cost;  // Modifier to the price a repairman charges for repairs
		} repair;
	} u;

	UINT8		ubShopKeeperID;					// Merc Id for the dealer
	UINT8		ubTypeOfArmsDealer;			// Whether he buys/sells, sells, buys, or repairs
	INT32		iInitialCash;						// How much cash dealer starts with (we now reset to this amount once / day)
	UINT32	uiFlags;								// various flags which control the dealer's operations
} ARMS_DEALER_INFO;


// THIS STRUCTURE GETS SAVED/RESTORED/RESET
typedef struct
{
	UINT32	uiArmsDealersCash;			// How much money the arms dealer currently has

	UINT8		ubSpecificDealerFlags;	// Misc state flags for specific dealers
	BOOLEAN fOutOfBusiness;					// Set when a dealer has been killed, etc.
	BOOLEAN fRepairDelayBeenUsed;		// Set when a repairman has missed his repair time estimate & given his excuse for it
	BOOLEAN	fUnusedKnowsPlayer;			// Set if the shopkeeper has met with the player before [UNUSED]

	UINT32	uiTimePlayerLastInSKI;	// game time (in total world minutes) when player last talked to this dealer in SKI

	UINT8		ubPadding[ 8 ];

} ARMS_DEALER_STATUS;
CASSERT(sizeof(ARMS_DEALER_STATUS) == 20)


typedef struct
{
	UINT16	usAttachment[MAX_ATTACHMENTS];		// item index of any attachments on the item

	INT8		bItemCondition;				// if 0, no item is stored
																// from 1 to 100 indicates an item with that status
																// -1 to -100 means the item is in for repairs, flip sign for the actual status

	UINT8		ubImprintID;					// imprint ID for imprinted items (during repair!)

	INT8		bAttachmentStatus[MAX_ATTACHMENTS];	// status of any attachments on the item

	UINT8		ubPadding[2];					// filler

} SPECIAL_ITEM_INFO;
CASSERT(sizeof(SPECIAL_ITEM_INFO) == 16)


typedef struct
{
	// Individual "special" items are stored here as needed, *one* per slot
	// An item is special if it is used (status < 100), has been imprinted, or has a permanent attachment

	SPECIAL_ITEM_INFO Info;

	UINT32  uiRepairDoneTime;			// If the item is in for repairs, this holds the time when it will be repaired (in min)

	BOOLEAN fActive;							// TRUE means an item is stored here (empty elements may not always be freed immediately)

	UINT8		ubOwnerProfileId;			// stores which merc previously owned an item being repaired

	UINT8		ubPadding[6];					// filler

} DEALER_SPECIAL_ITEM;
CASSERT(sizeof(DEALER_SPECIAL_ITEM) == 28)


typedef struct
{
	// Non-special items are all the identical and are totaled inside ubPerfectItems.
	// Items being repaired are also stored here, with a negative condition.
	// NOTE: special item elements may remain allocated long after item has been removed, to reduce memory fragmentation!!!

	UINT8		ubTotalItems;					// sum of all the items (all perfect ones + all special ones)
	UINT8		ubPerfectItems;				// non-special (perfect) items held by dealer
	UINT8		ubStrayAmmo;					// partially-depleted ammo mags are stored here as #bullets, and can be converted to full packs

	UINT8		ubElementsAlloced;		// number of DEALER_SPECIAL_ITEM array elements alloced for the special item array
	DEALER_SPECIAL_ITEM *SpecialItem;	// dynamic array of special items with this same item index

	UINT32  uiOrderArrivalTime;		// Day the items ordered will arrive on.  It's UINT32 in case we change this to minutes.
	UINT8		ubQtyOnOrder;					// The number of items currently on order
	BOOLEAN	fPreviouslyEligible;	// whether or not dealer has been eligible to sell this item in days prior to today

	UINT8		ubPadding[2];					// filler

} DEALER_ITEM_HEADER;
CASSERT(sizeof(DEALER_ITEM_HEADER) == 16)


extern const ARMS_DEALER_INFO ArmsDealerInfo[NUM_ARMS_DEALERS];
extern ARMS_DEALER_STATUS		gArmsDealerStatus[ NUM_ARMS_DEALERS ];
extern DEALER_ITEM_HEADER		gArmsDealersInventory[ NUM_ARMS_DEALERS ][ MAXITEMS ];






void InitAllArmsDealers(void);
void ShutDownArmsDealers(void);

//Count every single item the dealer has in stock
//UINT32	CountTotalItemsInArmsDealersInventory( UINT8 ubArmsDealer );
//Count only the # of "distinct" item types (for shopkeeper purposes)
UINT32	CountDistinctItemsInArmsDealersInventory( UINT8 ubArmsDealer );
UINT16	CountTotalItemsRepairDealerHasInForRepairs( UINT8 ubArmsDealer );

void		AddObjectToArmsDealerInventory( UINT8 ubArmsDealer, OBJECTTYPE *pObject );

void		RemoveItemFromArmsDealerInventory( UINT8 ubArmsDealer, UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, UINT8 ubHowMany );
void		RemoveSpecialItemFromArmsDealerInventoryAtElement( UINT8 ubArmsDealer, UINT16 usItemIndex, UINT8 ubElement );

BOOLEAN IsMercADealer( UINT8 ubMercID );
INT8		GetArmsDealerIDFromMercID( UINT8 ubMercID );

BOOLEAN SaveArmsDealerInventoryToSaveGameFile( HWFILE hFile );
BOOLEAN LoadArmsDealerInventoryFromSavedGameFile( HWFILE hFile, BOOLEAN fIncludesElgin, BOOLEAN fIncludesManny );

void DailyUpdateOfArmsDealersInventory(void);

UINT8		GetTypeOfArmsDealer( UINT8 ubDealerID );

BOOLEAN	DoesDealerDoRepairs( UINT8 ubArmsDealer );
BOOLEAN RepairmanIsFixingItemsButNoneAreDoneYet( UINT8 ubProfileID );

BOOLEAN CanDealerTransactItem( UINT8 ubArmsDealer, UINT16 usItemIndex, BOOLEAN fPurchaseFromPlayer );
BOOLEAN CanDealerRepairItem( UINT8 ubArmsDealer, UINT16 usItemIndex );

BOOLEAN AddDeadArmsDealerItemsToWorld( UINT8 ubMercID );

void		MakeObjectOutOfDealerItems( UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, OBJECTTYPE *pObject, UINT8 ubHowMany );

void GiveObjectToArmsDealerForRepair(UINT8 ubArmsDealer, const OBJECTTYPE* pObject, UINT8 ubOwnerProfileId);

UINT32 CalculateObjectItemRepairTime(UINT8 ubArmsDealer, const OBJECTTYPE* pItemObject);

UINT32 CalculateObjectItemRepairCost(UINT8 ubArmsDealer, const OBJECTTYPE* pItemObject);


void SetSpecialItemInfoToDefaults( SPECIAL_ITEM_INFO *pSpclItemInfo );
void SetSpecialItemInfoFromObject(SPECIAL_ITEM_INFO* pSpclItemInfo, const OBJECTTYPE* pObject);

UINT16	CalcValueOfItemToDealer( UINT8 ubArmsDealer, UINT16 usItemIndex, BOOLEAN fDealerSelling );

BOOLEAN DealerItemIsSafeToStack( UINT16 usItemIndex );

UINT32 CalculateOvernightRepairDelay( UINT8 ubArmsDealer, UINT32 uiTimeWhenFreeToStartIt, UINT32 uiMinutesToFix );
UINT32 CalculateMinutesClosedBetween( UINT8 ubArmsDealer, UINT32 uiStartTime, UINT32 uiEndTime );

void    GuaranteeAtLeastXItemsOfIndex(UINT8 ubArmsDealer, UINT16 usItemIndex, UINT8 ubHowMany);
BOOLEAN ItemIsARocketRifle(INT16 sItemIndex);

extern UINT8 gubLastSpecialItemAddedAtElement;

#endif
