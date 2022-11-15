#pragma once

#include "Types.h"

typedef TypedId<struct ItemModel, UINT16> ItemId;
static_assert (sizeof(ItemId) == sizeof(UINT16), "Size of ItemId should be the same as a uint16");

// TODO(ItemId): Implement!
void format_type(const ST::format_spec &format, ST::format_writer &output, const ItemId &value);

enum ItemCursor
{
	INVALIDCURS    =  0,
	QUESTCURS      =  1,
	PUNCHCURS      =  2,
	TARGETCURS     =  3,
	KNIFECURS      =  4,
	AIDCURS        =  5,
	TOSSCURS       =  6,
	MINECURS       =  8,
	LPICKCURS      =  9,
	MDETECTCURS    = 10,
	CROWBARCURS    = 11,
	SURVCAMCURS    = 12,
	CAMERACURS     = 13,
	KEYCURS        = 14,
	SAWCURS        = 15,
	WIRECUTCURS    = 16,
	REMOTECURS     = 17,
	BOMBCURS       = 18, // (only calculated, not set item table)
	REPAIRCURS     = 19,
	TRAJECTORYCURS = 20,
	JARCURS        = 21,
	TINCANCURS     = 22,
	REFUELCURS     = 23
};

#define ITEM_NOT_FOUND			-1


#define USABLE				10 // minimum work% of items to still be usable

#define MAX_OBJECTS_PER_SLOT		8
#define MAX_ATTACHMENTS		4
#define MAX_MONEY_PER_SLOT		20000

enum DetonatorType
{
	BOMB_TIMED = 1,
	BOMB_REMOTE,
	BOMB_PRESSURE,
	BOMB_SWITCH
};

#define FIRST_MAP_PLACED_FREQUENCY	50
#define PANIC_FREQUENCY		127
#define PANIC_FREQUENCY_2		126
#define PANIC_FREQUENCY_3		125

#define OBJECT_UNDROPPABLE		0x01
#define OBJECT_MODIFIED		0x02
#define OBJECT_AI_UNUSABLE		0x04
#define OBJECT_ARMED_BOMB		0x08
#define OBJECT_KNOWN_TO_BE_TRAPPED	0x10
#define OBJECT_DISABLED_BOMB		0x20
#define OBJECT_ALARM_TRIGGER		0x40
#define OBJECT_NO_OVERWRITE		0x80

struct OBJECTTYPE
{
	ItemId usItem;
	UINT8  ubNumberOfObjects;
	union
	{
		struct
		{
			INT8   bGunStatus; // status % of gun
			UINT8  ubGunAmmoType; // ammo type, as per weapons.h
			UINT8  ubGunShotsLeft; // duh, amount of ammo left
			ItemId usGunAmmoItem; // the item # for the item table
			INT8   bGunAmmoStatus; // only for "attached ammo" - grenades, mortar shells
			UINT8  ubGunUnused[MAX_OBJECTS_PER_SLOT - 6]; // XXX HACK000B
		};
		struct
		{
			UINT8 ubShotsLeft[MAX_OBJECTS_PER_SLOT];
		};
		struct
		{
			INT8 bStatus[MAX_OBJECTS_PER_SLOT];
		};
		struct
		{
			INT8   bMoneyStatus;
			UINT32 uiMoneyAmount;
			UINT8  ubMoneyUnused[MAX_OBJECTS_PER_SLOT - 5]; // XXX HACK000B
		};
		struct
		{
			// this is used by placed bombs, switches, and the action item
			INT8   bBombStatus; // % status
			INT8   bDetonatorType; // timed, remote, or pressure-activated
			ItemId usBombItem; // the usItem of the bomb.
			union
			{
				struct
				{
					INT8 bDelay; // >=0 values used only
				};
				struct
				{
					INT8 bFrequency; // >=0 values used only
				};
			};
			UINT8 ubBombOwner; // side which placed the bomb
			UINT8 bActionValue;// this is used by the ACTION_ITEM fake item
			UINT8 ubTolerance; // tolerance value for panic triggers
		};
		struct
		{
			INT8  bKeyStatus[ 6 ];
			UINT8 ubKeyID;
			UINT8 ubKeyUnused[1]; // XXX HACK000B
		};
		struct
		{
			UINT8 ubOwnerProfile;
			UINT8 ubOwnerCivGroup;
			UINT8 ubOwnershipUnused[6]; // XXX HACK000B
		};
	};
	// attached objects
	ItemId usAttachItem[MAX_ATTACHMENTS];
	INT8   bAttachStatus[MAX_ATTACHMENTS];

	INT8   fFlags;
	UINT8  ubMission;
	INT8   bTrap; // 1-10 exp_lvl to detect
	UINT8  ubImprintID; // ID of merc that item is imprinted on
	UINT8  ubWeight;
	UINT8  fUsed; // flags for whether the item is used or not
};


// SUBTYPES
#define IC_NONE			0x00000001
#define IC_GUN				0x00000002
#define IC_BLADE			0x00000004
#define IC_THROWING_KNIFE		0x00000008

#define IC_LAUNCHER			0x00000010
#define IC_TENTACLES			0x00000020

#define IC_THROWN			0x00000040
#define IC_PUNCH			0x00000080

#define IC_GRENADE			0x00000100
#define IC_BOMB			0x00000200
#define IC_AMMO			0x00000400
#define IC_ARMOUR			0x00000800

#define IC_MEDKIT			0x00001000
#define IC_KIT				0x00002000
#define IC_FACE			0x00008000

#define IC_KEY				0x00010000

#define IC_MISC			0x10000000
#define IC_MONEY			0x20000000

// PARENT TYPES
#define IC_ALL				0xFFFFFFFF

#define IC_WEAPON			( IC_GUN | IC_BLADE | IC_THROWING_KNIFE | IC_LAUNCHER | IC_TENTACLES )
#define IC_EXPLOSV			( IC_GRENADE | IC_BOMB )

#define IC_BOBBY_GUN			( IC_GUN | IC_LAUNCHER )
#define IC_BOBBY_MISC			( IC_GRENADE | IC_BOMB | IC_MISC | IC_MEDKIT | IC_KIT | IC_BLADE | IC_THROWING_KNIFE | IC_PUNCH | IC_FACE )


// replaces candamage
#define ITEM_DAMAGEABLE		0x0001
// replaces canrepair
#define ITEM_REPAIRABLE		0x0002
// replaces waterdamage
#define ITEM_WATER_DAMAGES		0x0004
// replaces metal
#define ITEM_METAL			0x0008
// replaces sinkable
#define ITEM_SINKS			0x0010
// replaces seemeter
#define ITEM_SHOW_STATUS		0x0020
// for attachers/merges, hidden
#define ITEM_HIDDEN_ADDON		0x0040
// needs two hands
#define ITEM_TWO_HANDED		0x0080
// can't be found for sale
#define ITEM_NOT_BUYABLE		0x0100
// item is an attachment for something
#define ITEM_ATTACHMENT		0x0200
// item only belongs in the "big gun list"
#define ITEM_BIGGUNLIST		0x0400
// item should not be placed via the editor
#define ITEM_NOT_EDITOR		0x0800
// item defaults to undroppable
#define ITEM_DEFAULT_UNDROPPABLE	0x1000
// item is terrible for throwing
#define ITEM_UNAERODYNAMIC		0x2000
// item is electronic for repair (etc) purposes
#define ITEM_ELECTRONIC		0x4000
// item is a PERMANENT attachment
#define ITEM_INSEPARABLE		0x8000

// item flag combinations

#define EXPLOSIVE_GUN( x )		( x == ROCKET_LAUNCHER || x == TANK_CANNON )

#define MAX_WEAPONS 70

constexpr ItemId NONE = ItemId(0);
constexpr ItemId NOTHING = NONE;

constexpr ItemId GLOCK_17 = ItemId(1);
constexpr ItemId GLOCK_18 = ItemId(2);
constexpr ItemId SW38 = ItemId(5);
constexpr ItemId DESERTEAGLE = ItemId(7);
constexpr ItemId G41 = ItemId(21);
constexpr ItemId G11 = ItemId(30);
constexpr ItemId MINIMI = ItemId(34);
constexpr ItemId COMBAT_KNIFE = ItemId(37);
constexpr ItemId THROWING_KNIFE = ItemId(38);
constexpr ItemId ROCK = ItemId(39);
constexpr ItemId GLAUNCHER = ItemId(40);
constexpr ItemId MORTAR = ItemId(41);
constexpr ItemId ROCK2 = ItemId(42);
constexpr ItemId CREATURE_YOUNG_MALE_CLAWS = ItemId(43);
constexpr ItemId CREATURE_OLD_MALE_CLAWS = ItemId(44);
constexpr ItemId CREATURE_YOUNG_FEMALE_CLAWS = ItemId(45);
constexpr ItemId CREATURE_OLD_FEMALE_CLAWS = ItemId(46);
constexpr ItemId CREATURE_QUEEN_TENTACLES = ItemId(47);
constexpr ItemId CREATURE_QUEEN_SPIT = ItemId(48);
constexpr ItemId BRASS_KNUCKLES = ItemId(49);
constexpr ItemId UNDER_GLAUNCHER = ItemId(50);
constexpr ItemId ROCKET_LAUNCHER = ItemId(51);
constexpr ItemId BLOODCAT_CLAW_ATTACK = ItemId(52);
constexpr ItemId BLOODCAT_BITE = ItemId(53);
constexpr ItemId ROCKET_RIFLE = ItemId(55);
constexpr ItemId AUTOMAG_III = ItemId(56);
constexpr ItemId CREATURE_INFANT_SPIT = ItemId(57);
constexpr ItemId CREATURE_YOUNG_MALE_SPIT = ItemId(58);
constexpr ItemId CREATURE_OLD_MALE_SPIT = ItemId(59);
constexpr ItemId TANK_CANNON = ItemId(60);
constexpr ItemId DART_GUN = ItemId(61);
constexpr ItemId BLOODY_THROWING_KNIFE = ItemId(62);
constexpr ItemId FLAMETHROWER = ItemId(63);
constexpr ItemId CROWBAR = ItemId(64);
constexpr ItemId AUTO_ROCKET_RIFLE = ItemId(65);
constexpr ItemId CLIP9_15 = ItemId(71);
constexpr ItemId CLIP9_30 = ItemId(72);
constexpr ItemId CLIP38_6 = ItemId(77);
constexpr ItemId CLIP45_7 = ItemId(80);
constexpr ItemId CLIP45_30 = ItemId(81);
constexpr ItemId CLIP357_6 = ItemId(86);
constexpr ItemId CLIP357_9 = ItemId(87);
constexpr ItemId CLIP545_30_HP = ItemId(93);
constexpr ItemId CLIP556_30_HP = ItemId(95);
constexpr ItemId CLIP762W_10_HP = ItemId(98);
constexpr ItemId CLIP762W_30_HP = ItemId(99);
constexpr ItemId CLIP762N_5_AP = ItemId(100);
constexpr ItemId CLIP762N_5_HP = ItemId(102);
constexpr ItemId CLIP762N_20_HP = ItemId(103);
constexpr ItemId CLIP12G_7 = ItemId(107);
constexpr ItemId CLIP12G_7_BUCKSHOT = ItemId(108);
constexpr ItemId STUN_GRENADE = ItemId(131);
constexpr ItemId TEARGAS_GRENADE = ItemId(132);
constexpr ItemId MUSTARD_GRENADE = ItemId(133);
constexpr ItemId MINI_GRENADE = ItemId(134);
constexpr ItemId HAND_GRENADE = ItemId(135);
constexpr ItemId RDX = ItemId(136);
constexpr ItemId TNT = ItemId(137);
constexpr ItemId HMX = ItemId(138);
constexpr ItemId C1 = ItemId(139);
constexpr ItemId MORTAR_SHELL = ItemId(140);
constexpr ItemId MINE = ItemId(141);
constexpr ItemId C4 = ItemId(142);
constexpr ItemId TRIP_FLARE = ItemId(143);
constexpr ItemId TRIP_KLAXON = ItemId(144);
constexpr ItemId SHAPED_CHARGE = ItemId(145);
constexpr ItemId BREAK_LIGHT = ItemId(146);
constexpr ItemId GL_HE_GRENADE = ItemId(147);
constexpr ItemId GL_TEARGAS_GRENADE = ItemId(148);
constexpr ItemId GL_STUN_GRENADE = ItemId(149);
constexpr ItemId GL_SMOKE_GRENADE = ItemId(150);
constexpr ItemId SMOKE_GRENADE = ItemId(151);
constexpr ItemId TANK_SHELL = ItemId(152);
constexpr ItemId STRUCTURE_IGNITE = ItemId(153);
constexpr ItemId STRUCTURE_EXPLOSION = ItemId(155);
constexpr ItemId GREAT_BIG_EXPLOSION = ItemId(156);
constexpr ItemId BIG_TEAR_GAS = ItemId(157);
constexpr ItemId SMALL_CREATURE_GAS = ItemId(158);
constexpr ItemId LARGE_CREATURE_GAS = ItemId(159);
constexpr ItemId VERY_SMALL_CREATURE_GAS = ItemId(160);
constexpr ItemId FLAK_JACKET = ItemId(161);
constexpr ItemId FLAK_JACKET_18 = ItemId(162);
constexpr ItemId FLAK_JACKET_Y = ItemId(163);
constexpr ItemId KEVLAR_VEST = ItemId(164);
constexpr ItemId KEVLAR_VEST_18 = ItemId(165);
constexpr ItemId KEVLAR_VEST_Y = ItemId(166);
constexpr ItemId SPECTRA_VEST = ItemId(167);
constexpr ItemId SPECTRA_VEST_18 = ItemId(168);
constexpr ItemId SPECTRA_VEST_Y = ItemId(169);
constexpr ItemId KEVLAR_LEGGINGS = ItemId(170);
constexpr ItemId KEVLAR_LEGGINGS_18 = ItemId(171);
constexpr ItemId KEVLAR_LEGGINGS_Y = ItemId(172);
constexpr ItemId SPECTRA_LEGGINGS = ItemId(173);
constexpr ItemId SPECTRA_LEGGINGS_18 = ItemId(174);
constexpr ItemId SPECTRA_LEGGINGS_Y = ItemId(175);
constexpr ItemId STEEL_HELMET = ItemId(176);
constexpr ItemId KEVLAR_HELMET = ItemId(177);
constexpr ItemId KEVLAR_HELMET_18 = ItemId(178);
constexpr ItemId KEVLAR_HELMET_Y = ItemId(179);
constexpr ItemId SPECTRA_HELMET = ItemId(180);
constexpr ItemId SPECTRA_HELMET_18 = ItemId(181);
constexpr ItemId SPECTRA_HELMET_Y = ItemId(182);
constexpr ItemId CERAMIC_PLATES = ItemId(183);
constexpr ItemId CREATURE_INFANT_HIDE = ItemId(184);
constexpr ItemId CREATURE_YOUNG_MALE_HIDE = ItemId(185);
constexpr ItemId CREATURE_OLD_MALE_HIDE = ItemId(186);
constexpr ItemId CREATURE_QUEEN_HIDE = ItemId(187);
constexpr ItemId LEATHER_JACKET = ItemId(188);
constexpr ItemId LEATHER_JACKET_W_KEVLAR = ItemId(189);
constexpr ItemId LEATHER_JACKET_W_KEVLAR_18 = ItemId(190);
constexpr ItemId LEATHER_JACKET_W_KEVLAR_Y = ItemId(191);
constexpr ItemId CREATURE_YOUNG_FEMALE_HIDE = ItemId(192);
constexpr ItemId CREATURE_OLD_FEMALE_HIDE = ItemId(193);
constexpr ItemId TSHIRT = ItemId(194);
constexpr ItemId TSHIRT_DEIDRANNA = ItemId(195);
constexpr ItemId KEVLAR2_VEST = ItemId(196);
constexpr ItemId KEVLAR2_VEST_18 = ItemId(197);
constexpr ItemId KEVLAR2_VEST_Y = ItemId(198);
constexpr ItemId FIRSTAIDKIT = ItemId(201);
constexpr ItemId MEDICKIT = ItemId(202);
constexpr ItemId TOOLKIT = ItemId(203);
constexpr ItemId LOCKSMITHKIT = ItemId(204);
constexpr ItemId CAMOUFLAGEKIT = ItemId(205);
constexpr ItemId SILENCER = ItemId(207);
constexpr ItemId SNIPERSCOPE = ItemId(208);
constexpr ItemId BIPOD = ItemId(209);
constexpr ItemId EXTENDEDEAR = ItemId(210);
constexpr ItemId NIGHTGOGGLES = ItemId(211);
constexpr ItemId SUNGOGGLES = ItemId(212);
constexpr ItemId GASMASK = ItemId(213);
constexpr ItemId CANTEEN = ItemId(214);
constexpr ItemId METALDETECTOR = ItemId(215);
constexpr ItemId COMPOUND18 = ItemId(216);
constexpr ItemId JAR_QUEEN_CREATURE_BLOOD = ItemId(217);
constexpr ItemId JAR_ELIXIR = ItemId(218);
constexpr ItemId MONEY = ItemId(219);
constexpr ItemId JAR = ItemId(220);
constexpr ItemId JAR_CREATURE_BLOOD = ItemId(221);
constexpr ItemId ADRENALINE_BOOSTER = ItemId(222);
constexpr ItemId DETONATOR = ItemId(223);
constexpr ItemId REMDETONATOR = ItemId(224);
constexpr ItemId DEED = ItemId(226);
constexpr ItemId LETTER = ItemId(227);
constexpr ItemId CHALICE = ItemId(229);
constexpr ItemId BLOODCAT_CLAWS = ItemId(230);
constexpr ItemId BLOODCAT_TEETH = ItemId(231);
constexpr ItemId BLOODCAT_PELT = ItemId(232);
constexpr ItemId SWITCH = ItemId(233);
constexpr ItemId ACTION_ITEM = ItemId(234);
constexpr ItemId REGEN_BOOSTER = ItemId(235);
constexpr ItemId SYRINGE_3 = ItemId(236);
constexpr ItemId SYRINGE_4 = ItemId(237);
constexpr ItemId SYRINGE_5 = ItemId(238);
constexpr ItemId JAR_HUMAN_BLOOD = ItemId(239);
constexpr ItemId OWNERSHIP = ItemId(240);
constexpr ItemId LASERSCOPE = ItemId(241);
constexpr ItemId REMOTEBOMBTRIGGER = ItemId(242);
constexpr ItemId WIRECUTTERS = ItemId(243);
constexpr ItemId DUCKBILL = ItemId(244);
constexpr ItemId ALCOHOL = ItemId(245);
constexpr ItemId UVGOGGLES = ItemId(246);
constexpr ItemId DISCARDED_LAW = ItemId(247);
constexpr ItemId HEAD_1 = ItemId(248);
constexpr ItemId HEAD_2 = ItemId(249);
constexpr ItemId HEAD_3 = ItemId(250);
constexpr ItemId HEAD_4 = ItemId(251);
constexpr ItemId HEAD_5 = ItemId(252);
constexpr ItemId HEAD_6 = ItemId(253);
constexpr ItemId HEAD_7 = ItemId(254);
constexpr ItemId WINE = ItemId(255);
constexpr ItemId BEER = ItemId(256);
constexpr ItemId VIDEO_CAMERA = ItemId(258);
constexpr ItemId ROBOT_REMOTE_CONTROL = ItemId(259);
constexpr ItemId CREATURE_PART_CLAWS = ItemId(260);
constexpr ItemId CREATURE_PART_FLESH = ItemId(261);
constexpr ItemId CREATURE_PART_ORGAN = ItemId(262);
constexpr ItemId REMOTETRIGGER = ItemId(263);
constexpr ItemId GOLDWATCH = ItemId(264);
constexpr ItemId WALKMAN = ItemId(266);
constexpr ItemId MONEY_FOR_PLAYERS_ACCOUNT = ItemId(268);
constexpr ItemId CIGARS = ItemId(269);
constexpr ItemId KEY_1 = ItemId(271);
constexpr ItemId KEY_32 = ItemId(302);
constexpr ItemId DUCT_TAPE = ItemId(304);
constexpr ItemId ALUMINUM_ROD = ItemId(305);
constexpr ItemId SPRING = ItemId(306);
constexpr ItemId SPRING_AND_BOLT_UPGRADE = ItemId(307);
constexpr ItemId STEEL_ROD = ItemId(308);
constexpr ItemId QUICK_GLUE = ItemId(309);
constexpr ItemId GUN_BARREL_EXTENDER = ItemId(310);
constexpr ItemId STRING = ItemId(311);
constexpr ItemId TIN_CAN = ItemId(312);
constexpr ItemId STRING_TIED_TO_TIN_CAN = ItemId(313);
constexpr ItemId MARBLES = ItemId(314);
constexpr ItemId LAME_BOY = ItemId(315);
constexpr ItemId COPPER_WIRE = ItemId(316);
constexpr ItemId DISPLAY_UNIT = ItemId(317);
constexpr ItemId FUMBLE_PAK = ItemId(318);
constexpr ItemId XRAY_BULB = ItemId(319);
constexpr ItemId CHEWING_GUM = ItemId(320);
constexpr ItemId FLASH_DEVICE = ItemId(321);
constexpr ItemId BATTERIES = ItemId(322);
constexpr ItemId XRAY_DEVICE = ItemId(324);
constexpr ItemId SILVER = ItemId(325);
constexpr ItemId GOLD = ItemId(326);
constexpr ItemId GAS_CAN = ItemId(327);
constexpr ItemId MAXITEMS = ItemId(351);
