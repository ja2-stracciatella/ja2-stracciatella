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


struct ITEM_SORT_ENTRY
{
	UINT32  uiItemClass;
	UINT8   ubWeaponClass;
	BOOLEAN fAllowUsed;
};


/* This table controls the order items appear in inventory at BR's and dealers,
 * and which kinds of items are sold used */
static ITEM_SORT_ENTRY const DealerItemSortInfo[] =
{
//  item class         weapon class  sold used?
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


//
// Setup the inventory arrays for each of the arms dealers
//
//	The arrays are composed of pairs of numbers
//		The first is the item index
//		The second is the amount of the items the dealer will try to keep in his inventory


// Tony ( Weapons only )
static DEALER_POSSIBLE_INV const gTonyInventory[] =
{
	//Rare guns/ammo that Tony will buy although he won't ever sell them
	{	ROCKET_RIFLE,					0 },
	{	AUTO_ROCKET_RIFLE,		0 },
	{ AUTOMAG_III,					0 },
//	{ FLAMETHROWER,					0 },


	//Weapons
	{ GLOCK_17,							1 },		/* Glock 17        */
	{ GLOCK_18,							1 },		/* Glock 18        */
	{ BERETTA_92F,					1 },		/* Beretta 92F     */
	{ BERETTA_93R,					1 },		/* Beretta 93R     */
	{ SW38,									1 },		/* .38 S&W Special */
	{ BARRACUDA,						1 },		/* .357 Barracuda  */
	{ DESERTEAGLE,					1 },		/* .357 DesertEagle*/
	{ M1911,								1 },		/* .45 M1911			 */
	{ MP5K,									1 },		/* H&K MP5K      	 */
	{ MAC10,								1 },		/* .45 MAC-10	     */

	{ THOMPSON,							1 },		/* Thompson M1A1   */
	{ COMMANDO,							1 },		/* Colt Commando   */
	{ MP53,									1 },		/* H&K MP53		 		 */
	{ AKSU74,								1 },		/* AKSU-74         */
	{ TYPE85,								1 },		/* Type-85         */
	{ SKS,									1 },		/* SKS             */
	{ DRAGUNOV,							1 },		/* Dragunov        */
	{ M24,									1 },		/* M24             */
	{ AUG,									1 },		/* Steyr AUG       */

	{ G41,									1 },		/* H&K G41         */
	{ MINI14,								1 },		/* Ruger Mini-14   */
	{ C7,										1 },		/* C-7             */
	{ FAMAS,								1 },		/* FA-MAS          */
	{ AK74,									1 },		/* AK-74           */
	{ AKM,									1 },		/* AKM             */
	{ M14,									1 },		/* M-14            */
	{ G3A3,									1 },		/* H&K G3A3        */
	{ FNFAL,								1 },		/* FN-FAL          */

	{ MINIMI,								1 },
	{ RPK74,								1 },
	{ HK21E,								1 },

	{ M870,									1 },		/* Remington M870  */
	{ SPAS15,								1 },		/* SPAS-15         */

	{ GLAUNCHER,						1 },		/* grenade launcher*/
	{ UNDER_GLAUNCHER,			1 },		/* underslung g.l. */
	{ ROCKET_LAUNCHER,			1 },		/* rocket Launcher */
	{ MORTAR,								1 },

	// SAP guns
	{ G11,									1 },
	{ CAWS,									1 },
	{ P90,									1 },

	{ DART_GUN,							1 },


	//Ammo
	{ CLIP9_15,							8 },
	{ CLIP9_30,							6 },
	{ CLIP9_15_AP,					3 },		/* CLIP9_15_AP */
	{ CLIP9_30_AP,				  3 },		/* CLIP9_30_AP */
	{ CLIP9_15_HP,				  3 },		/* CLIP9_15_HP */
	{ CLIP9_30_HP,				  3 },		/* CLIP9_30_HP */

	{ CLIP38_6,							10},		/* CLIP38_6 */
	{ CLIP38_6_AP,				  5 },		/* CLIP38_6_AP */
	{ CLIP38_6_HP,				  5 },		/* CLIP38_6_HP */

	{ CLIP45_7,							6 },		/* CLIP45_7 */				// 70

	{ CLIP45_30,					  8 },		/* CLIP45_30 */
	{ CLIP45_7_AP,					3 },		/* CLIP45_7_AP */
	{ CLIP45_30_AP,					3 },		/* CLIP45_30_AP */
	{ CLIP45_7_HP,					3 },		/* CLIP45_7_HP */
	{ CLIP45_30_HP,					3 },		/* CLIP45_30_HP */

	{ CLIP357_6,					  6 },		/* CLIP357_6 */
	{ CLIP357_9,					  5 },		/* CLIP357_9 */
	{ CLIP357_6_AP,				  3 },		/* CLIP357_6_AP */
	{ CLIP357_9_AP,					3 },		/* CLIP357_9_AP */
	{ CLIP357_6_HP,					3 },		/* CLIP357_6_HP */			//80
	{ CLIP357_9_HP,					3 },		/* CLIP357_9_HP */

	{ CLIP545_30_AP,				6 },		/* CLIP545_30_AP */
	{ CLIP545_30_HP,				3 },		/* CLIP545_30_HP */

	{ CLIP556_30_AP,				6 },		/* CLIP556_30_AP */
	{ CLIP556_30_HP,				3 },		/* CLIP556_30_HP */

	{ CLIP762W_10_AP,				6 },		/* CLIP762W_10_AP */
	{ CLIP762W_30_AP,				5 },		/* CLIP762W_30_AP */
	{ CLIP762W_10_HP,				3 },		/* CLIP762W_10_HP */
	{ CLIP762W_30_HP,				3 },		/* CLIP762W_30_HP */

	{ CLIP762N_5_AP,				8 },		/* CLIP762N_5_AP */			//90
	{ CLIP762N_20_AP,				5 },		/* CLIP762N_20_AP */
	{ CLIP762N_5_HP,				3 },		/* CLIP762N_5_HP */
	{ CLIP762N_20_HP,				3 },		/* CLIP762N_20_HP */

	{ CLIP47_50_SAP,				5 },		/* CLIP47_50_SAP */

	{ CLIP57_50_AP,					6 },		/* CLIP57_50_AP */
	{ CLIP57_50_HP,					3 },		/* CLIP57_50_HP */

	{ CLIP12G_7,						9 },		/* CLIP12G_7 */
	{ CLIP12G_7_BUCKSHOT,   9 },		/* CLIP12G_7_BUCKSHOT */

	{ CLIPCAWS_10_SAP,			5 },		/* CLIPCAWS_10_SAP */
	{ CLIPCAWS_10_FLECH,		3 },		/* CLIPCAWS_10_FLECH */			//100

	{ CLIPROCKET_AP,				3 },
	{ CLIPROCKET_HE,				1 },
	{ CLIPROCKET_HEAT,			1 },

	{ CLIPDART_SLEEP,				5	},

//	{ CLIPFLAME,						5	},

	// "launchables" (New! From McCain!) - these are basically ammo
	{ GL_HE_GRENADE,				2 },
	{ GL_TEARGAS_GRENADE,		2 },
	{ GL_STUN_GRENADE,			2 },
	{ GL_SMOKE_GRENADE,			2 },
	{	MORTAR_SHELL,					1 },

	// knives
	{	COMBAT_KNIFE,					3 },
	{	THROWING_KNIFE,				6 },
	{	BRASS_KNUCKLES,				1 },
	{	MACHETE,							1 },

	// attachments
	{ SILENCER,							3 },
	{ SNIPERSCOPE,					3 },
	{ LASERSCOPE,						1 },
	{ BIPOD,								3 },
	{ DUCKBILL,							2 },

/*
	// grenades
	{ STUN_GRENADE,					5 },
	{ TEARGAS_GRENADE,			5 },
	{ MUSTARD_GRENADE,			5 },
	{ MINI_GRENADE,					5 },
	{ HAND_GRENADE,					5 },
	{ SMOKE_GRENADE,				5 },
*/

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Devin		( Explosives )
static DEALER_POSSIBLE_INV const gDevinInventory[] =
{
	{	STUN_GRENADE,							3 },
	{	TEARGAS_GRENADE,					3 },
	{	MUSTARD_GRENADE,					2 },
	{	MINI_GRENADE,							3 },
	{	HAND_GRENADE,							2 },
	{ SMOKE_GRENADE,						3 },

	{	GL_HE_GRENADE,						2 },
	{	GL_TEARGAS_GRENADE,				2 },
	{	GL_STUN_GRENADE,					2 },
	{	GL_SMOKE_GRENADE,					2 },
	{	MORTAR_SHELL,							1 },

	{	CLIPROCKET_AP,						1 },
	{ CLIPROCKET_HE,						1 },
	{ CLIPROCKET_HEAT,					1 },

	{ DETONATOR,								10},
	{ REMDETONATOR,							5 },
	{ REMOTEBOMBTRIGGER,				5 },

	{	MINE,											6 },
	{	RDX,											5 },
	{	TNT,											5 },
	{	C1,												4 },
	{	HMX,											3 },
	{	C4,												2 },

	{	SHAPED_CHARGE,						5 },

//	{	TRIP_FLARE,								2 },
//	{	TRIP_KLAXON,							2 },

	{ GLAUNCHER,								1 },		/* grenade launcher*/
	{ UNDER_GLAUNCHER,					1 },		/* underslung g.l. */
	{ ROCKET_LAUNCHER,					1 },		/* rocket Launcher */
	{ MORTAR,										1 },

	{	METALDETECTOR,						2 },
	{	WIRECUTTERS,							1 },
	{	DUCT_TAPE,								1 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Franz	(Expensive pawn shop )
static DEALER_POSSIBLE_INV const gFranzInventory[] =
{
	{ NIGHTGOGGLES,							3 },

	{ LASERSCOPE,								3 },
	{ METALDETECTOR,						2 },
	{ EXTENDEDEAR,							2 },

	{	DART_GUN,									1 },

	{ KEVLAR_VEST,							1	},
	{ KEVLAR_LEGGINGS,					1 },
	{ KEVLAR_HELMET,						1	},
	{ KEVLAR2_VEST,							1 },
	{	SPECTRA_VEST,							1 },
	{	SPECTRA_LEGGINGS,					1 },
	{	SPECTRA_HELMET,						1 },

	{ CERAMIC_PLATES,						1 },

	{	CAMOUFLAGEKIT,						1 },

	{ VIDEO_CAMERA,							1 },		// for robot quest

	{ LAME_BOY,									1 },
	{ FUMBLE_PAK,								1 },

	{ GOLDWATCH,								1 },
	{ GOLFCLUBS,								1 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Keith		( Cheap Pawn Shop )
static DEALER_POSSIBLE_INV const gKeithInventory[] =
{
	{ FIRSTAIDKIT,			5 },

	// WARNING: Keith must not carry any guns, it would conflict with his story/quest

	{	COMBAT_KNIFE,			2 },
	{ THROWING_KNIFE,		3 },
	{	BRASS_KNUCKLES,		1 },
	{	MACHETE,					1 },

	{ SUNGOGGLES,				3 },
	{ FLAK_JACKET,			2	},
	{ STEEL_HELMET,			3 },
	{ LEATHER_JACKET,		1 },

	{ CANTEEN,					5 },
	{ CROWBAR,					1 },
	{ JAR,							6 },

	{	TOOLKIT,					1 },
	{	GASMASK,					1 },

	{ SILVER_PLATTER,		1 },

	{ WALKMAN,					1 },
	{ PORTABLETV,				1 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Sam		( Hardware )
static DEALER_POSSIBLE_INV const gSamInventory[] =
{
	{ FIRSTAIDKIT,			3 },

	{ LOCKSMITHKIT,			4 },
	{ TOOLKIT,					3 },

	{ CANTEEN,					5 },

	{ CROWBAR,					3 },
	{ WIRECUTTERS,			3 },

	{ DUCKBILL,					3 },
	{ JAR,							12},
	{	BREAK_LIGHT,			12},		// flares

	{	METALDETECTOR,		1 },

	{ VIDEO_CAMERA,			1 },

	{ QUICK_GLUE,				3 },
	{ COPPER_WIRE,			5 },
	{ BATTERIES,				10 },

	{ CLIP9_15,					5 },
	{ CLIP9_30,					5 },
	{ CLIP38_6,					5 },
	{ CLIP45_7,					5 },
	{ CLIP45_30,				5 },
	{ CLIP357_6,				5 },
	{ CLIP357_9,				5 },
	{ CLIP12G_7,					9 },
	{ CLIP12G_7_BUCKSHOT,  9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Jake			( Junk )
static DEALER_POSSIBLE_INV const gJakeInventory[] =
{
	{ FIRSTAIDKIT,			4 },
	{ MEDICKIT,					3 },

	{ SW38,							1 },
	{ CLIP38_6,					5 },

	{ JAR,							3 },
	{ CANTEEN,					2 },
	{ BEER,							6 },

	{ CROWBAR,					1 },
	{ WIRECUTTERS,			1 },

	{ COMBAT_KNIFE,			1 },
	{ THROWING_KNIFE,		1 },
	{	BRASS_KNUCKLES,		1 },
	{ MACHETE,					1 },

	{	BREAK_LIGHT,			5 },		// flares

	{	BIPOD,						1 },

	{ TSHIRT,						6 },
	{ CIGARS,						3 },
	{ PORNOS,						1 },

	{ LOCKSMITHKIT,			1 },

	// "new" items, presumed unsafe for demo
	{ TSHIRT_DEIDRANNA,	2 },
	{	XRAY_BULB,				1 },

	// additional stuff possible in real game
	{ GLOCK_17,					1 },		/* Glock 17        */
	{ GLOCK_18,					1 },		/* Glock 18        */
	{ BERETTA_92F,			1 },		/* Beretta 92F     */
	{ BERETTA_93R,			1 },		/* Beretta 93R     */
	{ BARRACUDA,				1 },		/* .357 Barracuda  */
	{ DESERTEAGLE,			1 },		/* .357 DesertEagle*/
	{ M1911,						1 },		/* .45 M1911			 */

	{ DISCARDED_LAW,		1 },

	{ STEEL_HELMET,			1 },

	{ TOOLKIT,					1 },

	{ WINE,							1 },
	{ ALCOHOL,					1 },

	{ GOLDWATCH,				1 },
	{ GOLFCLUBS,				1 },
	{ WALKMAN,					1 },
	{ PORTABLETV,				1 },

	// stuff a real pawn shop wouldn't have, but it does make him a bit more useful
	{ COMPOUND18,				1 },
	{ CERAMIC_PLATES,		1 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Howard		( Pharmaceuticals )
static DEALER_POSSIBLE_INV const gHowardInventory[] =
{
	{ FIRSTAIDKIT,				10},
	{ MEDICKIT,						5 },
	{ ADRENALINE_BOOSTER,	5 },
	{ REGEN_BOOSTER,			5 },

	{ ALCOHOL,						3 },
	{	COMBAT_KNIFE,				2 },

	{	CLIPDART_SLEEP,			5 },

	{	CHEWING_GUM,				3 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Gabby			( Creature parts and Blood )
static DEALER_POSSIBLE_INV const gGabbyInventory[] =
{
	{ JAR,											12 },
	{ JAR_ELIXIR,								3 },
	// buys these, but can't supply them (player is the only source)
	{	JAR_CREATURE_BLOOD,				0 },
	{ JAR_QUEEN_CREATURE_BLOOD, 0 },
	{ BLOODCAT_CLAWS,						0 },
	{ BLOODCAT_TEETH,						0 },
	{ BLOODCAT_PELT,						0 },
	{ CREATURE_PART_CLAWS,			0 },
	{ CREATURE_PART_FLESH,			0 },
	{ CREATURE_PART_ORGAN,			0 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Frank  ( Alcohol )
static DEALER_POSSIBLE_INV const gFrankInventory[] =
{
	{ BEER,							12 },
	{ WINE,							6 },
	{ ALCOHOL,					9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Elgin  ( Alcohol )
static DEALER_POSSIBLE_INV const gElginInventory[] =
{
	{ BEER,							12 },
	{ WINE,							6 },
	{ ALCOHOL,					9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Manny  ( Alcohol )
static DEALER_POSSIBLE_INV const gMannyInventory[] =
{
	{ BEER,							12 },
	{ WINE,							6 },
	{ ALCOHOL,					9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Herve Santos		( Alcohol )
static DEALER_POSSIBLE_INV const gHerveInventory[] =
{
	{ BEER,							12 },
	{ WINE,							6 },
	{ ALCOHOL,					9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Peter Santos ( Alcohol )
static DEALER_POSSIBLE_INV const gPeterInventory[] =
{
	{ BEER,							12 },
	{ WINE,							6 },
	{ ALCOHOL,					9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Alberto Santos		( Alcohol )
static DEALER_POSSIBLE_INV const gAlbertoInventory[] =
{
	{ BEER,							12 },
	{ WINE,							6 },
	{ ALCOHOL,					9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Carlo Santos		( Alcohol )
static DEALER_POSSIBLE_INV const gCarloInventory[] =
{
	{ BEER,							12 },
	{ WINE,							6 },
	{ ALCOHOL,					9 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Micky	( BUYS Animal / Creature parts )
static DEALER_POSSIBLE_INV const gMickyInventory[] =
{
	// ONLY BUYS THIS STUFF, DOESN'T SELL IT
	{ BLOODCAT_CLAWS,	0 },
	{ BLOODCAT_TEETH,	0 },
	{ BLOODCAT_PELT,		0 },
	{ CREATURE_PART_CLAWS,	0 },
	{ CREATURE_PART_FLESH,	0 },
	{ CREATURE_PART_ORGAN,	0 },
	{ JAR_QUEEN_CREATURE_BLOOD, 0 },

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Arnie		( Weapons REPAIR )
static DEALER_POSSIBLE_INV const gArnieInventory[] =
{
	// NO INVENTORY

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Perko			( REPAIR)
static DEALER_POSSIBLE_INV const gPerkoInventory[] =
{
	// NO INVENTORY

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


// Fredo			( Electronics REPAIR)
static DEALER_POSSIBLE_INV const gFredoInventory[] =
{
	// NO INVENTORY

	{ LAST_DEALER_ITEM,	NO_DEALER_ITEM },		//Last One
};


static INT8 GetMaxItemAmount(DEALER_POSSIBLE_INV const*, UINT16 usItemIndex);


INT8 GetDealersMaxItemAmount(ArmsDealerID const ubDealerID, UINT16 const usItemIndex)
{
	return GetMaxItemAmount(GetPointerToDealersPossibleInventory(ubDealerID), usItemIndex);
}


static INT8 GetMaxItemAmount(DEALER_POSSIBLE_INV const* const pInv, UINT16 const usItemIndex)
{
	UINT16	usCnt=0;

	//loop through the array until a the LAST_DEALER_ITEM is hit
	while( pInv[ usCnt ].sItemIndex != LAST_DEALER_ITEM )
	{
		//if this item is the one we want
		if( pInv[ usCnt ].sItemIndex == usItemIndex )
			return( pInv[ usCnt ].ubOptimalNumber );

		// move to the next item
		usCnt++;
	}

	return( NO_DEALER_ITEM );
}


DEALER_POSSIBLE_INV const* GetPointerToDealersPossibleInventory(ArmsDealerID const ubArmsDealerID)
{
	switch (ubArmsDealerID)
	{
		case ARMS_DEALER_TONY:      return gTonyInventory;
		case ARMS_DEALER_FRANK:     return gFrankInventory;
		case ARMS_DEALER_MICKY:     return gMickyInventory;
		case ARMS_DEALER_ARNIE:     return gArnieInventory;
		case ARMS_DEALER_PERKO:     return gPerkoInventory;
		case ARMS_DEALER_KEITH:     return gKeithInventory;
		case ARMS_DEALER_BAR_BRO_1: return gHerveInventory;
		case ARMS_DEALER_BAR_BRO_2: return gPeterInventory;
		case ARMS_DEALER_BAR_BRO_3: return gAlbertoInventory;
		case ARMS_DEALER_BAR_BRO_4: return gCarloInventory;
		case ARMS_DEALER_JAKE:      return gJakeInventory;
		case ARMS_DEALER_FRANZ:     return gFranzInventory;
		case ARMS_DEALER_HOWARD:    return gHowardInventory;
		case ARMS_DEALER_SAM:       return gSamInventory;
		case ARMS_DEALER_FREDO:     return gFredoInventory;
		case ARMS_DEALER_GABBY:     return gGabbyInventory;
		case ARMS_DEALER_DEVIN:     return gDevinInventory;
		case ARMS_DEALER_ELGIN:     return gElginInventory;
		case ARMS_DEALER_MANNY:     return gMannyInventory;
		default: throw std::logic_error("Invalid arms dealer ID");
	}
}


static UINT8 GetCurrentSuitabilityForItem(ArmsDealerID const bArmsDealer, UINT16 const usItemIndex)
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
	if ( Item[ usItemIndex ].fFlags & ITEM_NOT_BUYABLE )
	{
		return(ITEM_SUITABILITY_NONE);
	}


	ubItemCoolness = Item[ usItemIndex ].ubCoolness;

	if (ubItemCoolness == 0)
	{
		// items without a coolness rating can't be sold to the player by shopkeepers
		return(ITEM_SUITABILITY_NONE);
	}

	// the following staple items are always deemed highly suitable regardless of player's progress:
	switch (usItemIndex)
	{
		case CLIP38_6:
		case CLIP9_15:
		case CLIP9_30:
		case CLIP357_6:
		case CLIP357_9:
		case CLIP45_7:
		case CLIP45_30:
		case CLIP12G_7:
		case CLIP12G_7_BUCKSHOT:
		case CLIP545_30_HP:
		case CLIP556_30_HP:
		case CLIP762W_10_HP:
		case CLIP762W_30_HP:
		case CLIP762N_5_HP:
		case CLIP762N_20_HP:

		case FIRSTAIDKIT:
		case MEDICKIT:
		case TOOLKIT:
		case LOCKSMITHKIT:

		case CANTEEN:
		case CROWBAR:
		case JAR:
		case JAR_ELIXIR:
		case JAR_CREATURE_BLOOD:

			return(ITEM_SUITABILITY_ALWAYS);
	}

	switch (bArmsDealer)
	{
		case ARMS_DEALER_BOBBYR:
		case ARMS_DEALER_TONY:
		case ARMS_DEALER_DEVIN:
			break;

		default:
			/* All the other dealers have very limited inventories, so their
			 * suitability remains constant at all times in game */
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


	ubMinCoolness = MAX( 1, MIN( 9, ubMinCoolness ) );
	ubMaxCoolness = MAX( 2, MIN( 10, ubMaxCoolness ) );


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


UINT8 ChanceOfItemTransaction(ArmsDealerID const bArmsDealer, UINT16 const usItemIndex, BOOLEAN const fDealerIsSelling, BOOLEAN const fUsed)
{
	UINT8 ubItemCoolness;
	UINT8 ubChance = 0;

	// make sure dealers don't carry used items that they shouldn't
	if ( fUsed && !fDealerIsSelling && !CanDealerItemBeSoldUsed( usItemIndex ) )
		return( 0 );

	// Bobby Ray has an easier time getting resupplied than the local dealers do
	BOOLEAN const fBobbyRay = bArmsDealer == ARMS_DEALER_BOBBYR;

	ubItemCoolness = Item[ usItemIndex ].ubCoolness;

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
			Assert(0);
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


BOOLEAN ItemTransactionOccurs(ArmsDealerID const bArmsDealer, UINT16 const usItemIndex, BOOLEAN const fDealerIsSelling, BOOLEAN const fUsed)
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
			if (fUsed)
			{
				sInventorySlot = GetInventorySlotForItem(LaptopSaveInfo.BobbyRayUsedInventory, usItemIndex, fUsed);
				LaptopSaveInfo.BobbyRayUsedInventory[ sInventorySlot ].fPreviouslyEligible = TRUE;
			}
			else
			{
				sInventorySlot = GetInventorySlotForItem(LaptopSaveInfo.BobbyRayInventory, usItemIndex, fUsed);
				LaptopSaveInfo.BobbyRayInventory    [ sInventorySlot ].fPreviouslyEligible = TRUE;
			}
		}
		else
		{
			gArmsDealersInventory[ bArmsDealer ][ usItemIndex ].fPreviouslyEligible = TRUE;
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


UINT8 DetermineInitialInvItems(ArmsDealerID const bArmsDealerID, UINT16 const usItemIndex, UINT8 const ubChances, BOOLEAN const fUsed)
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


UINT8 HowManyItemsAreSold(ArmsDealerID const bArmsDealerID, UINT16 const usItemIndex, UINT8 const ubNumInStock, BOOLEAN const fUsed)
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
	UINT16	usItem1Index;
	UINT16	usItem2Index;
	UINT8		ubItem1Quality;
	UINT8		ubItem2Quality;

	usItem1Index = ( ( STORE_INVENTORY * ) pArg1 ) -> usItemIndex;
	usItem2Index = ( ( STORE_INVENTORY * ) pArg2 ) -> usItemIndex;

	ubItem1Quality = ( ( STORE_INVENTORY * ) pArg1 ) -> ubItemQuality;
	ubItem2Quality = ( ( STORE_INVENTORY * ) pArg2 ) -> ubItemQuality;

	return( CompareItemsForSorting( usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality ) );
}



int ArmsDealerItemQsortCompare(const void *pArg1, const void *pArg2)
{
	UINT16	usItem1Index;
	UINT16	usItem2Index;
	UINT8		ubItem1Quality;
	UINT8		ubItem2Quality;

	usItem1Index = ( ( INVENTORY_IN_SLOT * ) pArg1 ) -> sItemIndex;
	usItem2Index = ( ( INVENTORY_IN_SLOT * ) pArg2 ) -> sItemIndex;

	ubItem1Quality = ( ( INVENTORY_IN_SLOT * ) pArg1 ) -> ItemObject.bStatus[ 0 ];
	ubItem2Quality = ( ( INVENTORY_IN_SLOT * ) pArg2 ) -> ItemObject.bStatus[ 0 ];

	return( CompareItemsForSorting( usItem1Index, usItem2Index, ubItem1Quality, ubItem2Quality ) );
}


static UINT8 GetDealerItemCategoryNumber(UINT16 usItemIndex);


int CompareItemsForSorting(UINT16 const item_index1, UINT16 const item_index2, UINT8 const item_quality1, UINT8 const item_quality2)
{
	// lower category first
	UINT8 const category1 = GetDealerItemCategoryNumber(item_index1);
	UINT8 const category2 = GetDealerItemCategoryNumber(item_index2);
	if (category1 < category2) return -1;
	if (category1 > category2) return  1;

	INVTYPE const& item1 = Item[item_index1];
	INVTYPE const& item2 = Item[item_index2];

	// the same category
	if (item1.usItemClass == IC_AMMO && item2.usItemClass == IC_AMMO)
	{
		// AMMO is sorted by caliber first
		AmmoKind const calibre1 = Magazine[item1.ubClassIndex].ubCalibre;
		AmmoKind const calibre2 = Magazine[item2.ubClassIndex].ubCalibre;
		if (calibre1 > calibre2) return -1;
		if (calibre1 < calibre2) return  1;

		// the same caliber - compare size of magazine
		UINT8 const mag_size1 = Magazine[item1.ubClassIndex].ubMagSize;
		UINT8 const mag_size2 = Magazine[item2.ubClassIndex].ubMagSize;
		if (mag_size1 > mag_size2) return -1;
		if (mag_size1 < mag_size2) return  1;
	}
	else
	{
		// items other than ammo are compared on coolness first
		// higher coolness first
		UINT8 const coolness1 = item1.ubCoolness;
		UINT8 const coolness2 = item2.ubCoolness;
		if (coolness1 > coolness2) return -1;
		if (coolness1 < coolness2) return  1;
	}

	// the same coolness/caliber - compare base prices then
	// higher price first
	UINT16 const price1 = item1.usPrice;
	UINT16 const price2 = item2.usPrice;
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


static UINT8 GetDealerItemCategoryNumber(UINT16 const usItemIndex)
{
	UINT32 const item_class = Item[usItemIndex].usItemClass;

	// If it's not a weapon, set no weapon class, as this won't be needed
	UINT8 const weapon_class = usItemIndex < MAX_WEAPONS ?
		Weapon[usItemIndex].ubWeaponClass : NOGUNCLASS;

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



BOOLEAN CanDealerItemBeSoldUsed( UINT16 usItemIndex )
{
	if ( !( Item[ usItemIndex ].fFlags & ITEM_DAMAGEABLE ) )
		return(FALSE);

	// certain items, although they're damagable, shouldn't be sold in a used condition
	return( DealerItemSortInfo[ GetDealerItemCategoryNumber( usItemIndex ) ].fAllowUsed );
}
