#include "Font_Control.h"
#include "Handle_Items.h"
#include "Items.h"
#include "Action_Items.h"
#include "JAScreens.h"
#include "TileDef.h"
#include "Weapons.h"
#include "Interface_Cursors.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Handle_UI.h"
#include "Points.h"
#include "Sound_Control.h"
#include "Isometric_Utils.h"
#include "Animation_Data.h"
#include "Random.h"
#include "Campaign.h"
#include "Interface.h"
#include "Interface_Panels.h"
#include "Explosion_Control.h"
#include "Keys.h"
#include "WCheck.h"
#include "Soldier_Profile.h"
#include "SkillCheck.h"
#include "LOS.h"
#include "Message.h"
#include "Text.h"
#include "FOV.h"
#include "ShopKeeper_Interface.h"
#include "GamePolicy.h"
#include "GameSettings.h"
#include "Environment.h"
#include "Auto_Resolve.h"
#include "Interface_Items.h"
#include "Game_Clock.h"
#include "Smell.h"
#include "StrategicMap.h"
#include "Campaign_Types.h"
#include "Soldier_Macros.h"
#include "Debug.h"

#include "AmmoTypeModel.h"
#include "CalibreModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "ItemModel.h"
#include "MagazineModel.h"
#include "WeaponModels.h"
#include <array>
#include <initializer_list>
#include <map>
#include <set>
#include <stdexcept>

constexpr UINT8 ANY_MAGSIZE = 255; // magic number for FindAmmo's mag_size parameter

struct AttachmentInfoStruct
{
	UINT16 usItem;
	UINT32 uiItemClass;
	INT8   bAttachmentSkillCheck;
	INT8   bAttachmentSkillCheckMod;
};


// NB hack:  if an item appears in this array with an item class of IC_MISC,
// it is a slot used for noting the skill check required for a merge or multi-item attachment

static const AttachmentInfoStruct AttachmentInfo[] =
{
	{SILENCER,			IC_GUN,		NO_CHECK,					0},
	{SNIPERSCOPE,			IC_GUN,		NO_CHECK,					0},
	{LASERSCOPE,			IC_GUN,		NO_CHECK,					0},
	{BIPOD,				IC_GUN,		NO_CHECK,					0},
	{UNDER_GLAUNCHER,		IC_GUN,		NO_CHECK,					0},
	{DUCKBILL,			IC_GUN,		NO_CHECK,					0},
	{SPRING_AND_BOLT_UPGRADE,	IC_GUN,		ATTACHING_SPECIAL_ITEM_CHECK,			0},
	{GUN_BARREL_EXTENDER,		IC_GUN,		ATTACHING_SPECIAL_ITEM_CHECK,			0},
	{DETONATOR,			IC_BOMB,	ATTACHING_DETONATOR_CHECK,			0},
	{REMDETONATOR,			IC_BOMB,	ATTACHING_REMOTE_DETONATOR_CHECK,		-10},
	{XRAY_BULB,			IC_NONE,	ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK,	-15},
	{COPPER_WIRE,			IC_NONE,	ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK,	+20},
	{CERAMIC_PLATES,		IC_ARMOUR,	NO_CHECK,					0},

	// extras
	{NIGHTGOGGLES,				IC_ARMOUR,	NO_CHECK,								0},
	{UVGOGGLES,					IC_ARMOUR,	NO_CHECK,								0},
	{SUNGOGGLES,				IC_ARMOUR,	NO_CHECK,								0},
	{ROBOT_REMOTE_CONTROL,		IC_ARMOUR,	NO_CHECK,								0},

	{ADRENALINE_BOOSTER,		IC_ARMOUR,	NO_CHECK,								0},
	{REGEN_BOOSTER,				IC_ARMOUR,	NO_CHECK,								0},
	{BREAK_LIGHT,				IC_ARMOUR,	NO_CHECK,								0},
};

static std::map<UINT16, std::set<UINT16> const> const g_attachments
{
	{DETONATOR, {TNT, HMX, C1, C4}},
	{REMDETONATOR, {TNT, HMX, C1, C4}},
	{CERAMIC_PLATES, {FLAK_JACKET, FLAK_JACKET_18, FLAK_JACKET_Y, KEVLAR_VEST, KEVLAR_VEST_18, KEVLAR_VEST_Y,
	                  KEVLAR2_VEST, KEVLAR2_VEST_18, KEVLAR2_VEST_Y, SPECTRA_VEST, SPECTRA_VEST_18, SPECTRA_VEST_Y}},
	{SPRING, {ALUMINUM_ROD}},
	{QUICK_GLUE, {STEEL_ROD}},
	{DUCT_TAPE, {STEEL_ROD}},
	{XRAY_BULB, {FUMBLE_PAK}},
	{CHEWING_GUM, {FUMBLE_PAK}},
	{BATTERIES, {XRAY_DEVICE}},
	{COPPER_WIRE, {LAME_BOY}}
};

// additional possible attachments if the extra_attachments game policy is set
static std::set<UINT16> const g_helmets {STEEL_HELMET, KEVLAR_HELMET, KEVLAR_HELMET_18, KEVLAR_HELMET_Y, SPECTRA_HELMET, SPECTRA_HELMET_18, SPECTRA_HELMET_Y};
static std::set<UINT16> const g_leggings {KEVLAR_LEGGINGS, KEVLAR_LEGGINGS_18, KEVLAR_LEGGINGS_Y, SPECTRA_LEGGINGS, SPECTRA_LEGGINGS_18, SPECTRA_LEGGINGS_Y};
static std::map<UINT16, decltype(g_helmets) *> const g_attachments_mod
{
	{NIGHTGOGGLES, &g_helmets},
	{UVGOGGLES, &g_helmets},
	{SUNGOGGLES, &g_helmets},
	{ROBOT_REMOTE_CONTROL, &g_helmets},

	{BREAK_LIGHT, &g_leggings},
	{REGEN_BOOSTER, &g_leggings},
	{ADRENALINE_BOOSTER, &g_leggings}
};

static std::map<UINT16, std::set<UINT16> const> const Launchable
{
	{GL_HE_GRENADE, {GLAUNCHER, UNDER_GLAUNCHER}},
	{GL_TEARGAS_GRENADE, {GLAUNCHER, UNDER_GLAUNCHER}},
	{GL_STUN_GRENADE, {GLAUNCHER, UNDER_GLAUNCHER}},
	{GL_SMOKE_GRENADE, {GLAUNCHER, UNDER_GLAUNCHER}},
	{MORTAR_SHELL, {MORTAR}},
	{TANK_SHELL, {TANK_CANNON}}
};

static std::initializer_list<std::array<UINT16, 2> const> const CompatibleFaceItems
{
	{ NIGHTGOGGLES, EXTENDEDEAR },
	{ NIGHTGOGGLES, WALKMAN     },
	{ SUNGOGGLES,   EXTENDEDEAR },
	{ SUNGOGGLES,   WALKMAN     },
	{ UVGOGGLES,    EXTENDEDEAR },
	{ UVGOGGLES,    WALKMAN     },
	{ GASMASK,      EXTENDEDEAR },
	{ GASMASK,      WALKMAN     }
};


enum MergeType
{
	DESTRUCTION,
	COMBINE_POINTS,
	TREAT_ARMOUR,
	EXPLOSIVE,
	EASY_MERGE,
	ELECTRONIC_MERGE
};


struct MergeInfo
{
	UINT16 item1;
	UINT16 item2;
	UINT16 result;
	MergeType action;
};


static MergeInfo const Merge[] =
{
	// first item			second item			resulting item,			merge type
	{FIRSTAIDKIT,			FIRSTAIDKIT,			FIRSTAIDKIT,			COMBINE_POINTS},
	{MEDICKIT,			MEDICKIT,			MEDICKIT,			COMBINE_POINTS},
	{LOCKSMITHKIT,			LOCKSMITHKIT,			LOCKSMITHKIT,			COMBINE_POINTS},
	{TOOLKIT,			TOOLKIT,			TOOLKIT,			COMBINE_POINTS},
	{GAS_CAN,			GAS_CAN,			GAS_CAN,			COMBINE_POINTS},
	{CAMOUFLAGEKIT,			CAMOUFLAGEKIT,			CAMOUFLAGEKIT,			COMBINE_POINTS},
	{BEER,				BEER,				BEER,				COMBINE_POINTS},
	{WINE,				WINE,				WINE,				COMBINE_POINTS},
	{ALCOHOL,			ALCOHOL,			ALCOHOL,			COMBINE_POINTS},
	{CANTEEN,			CANTEEN,			CANTEEN,			COMBINE_POINTS},

	{COMPOUND18,			FLAK_JACKET,			FLAK_JACKET_18,			TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR_VEST,			KEVLAR_VEST_18,			TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR2_VEST,			KEVLAR2_VEST_18,		TREAT_ARMOUR},
	{COMPOUND18,			SPECTRA_VEST,			SPECTRA_VEST_18,		TREAT_ARMOUR},
	{COMPOUND18,			LEATHER_JACKET_W_KEVLAR,	LEATHER_JACKET_W_KEVLAR_18,	TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR_LEGGINGS,		KEVLAR_LEGGINGS_18,		TREAT_ARMOUR},
	{COMPOUND18,			SPECTRA_LEGGINGS,		SPECTRA_LEGGINGS_18,		TREAT_ARMOUR},
	{COMPOUND18,			KEVLAR_HELMET,			KEVLAR_HELMET_18,		TREAT_ARMOUR},
	{COMPOUND18,			SPECTRA_HELMET,			SPECTRA_HELMET_18,		TREAT_ARMOUR},
	{COMPOUND18,			FLAK_JACKET_Y,			NOTHING,			DESTRUCTION},
	{COMPOUND18,			KEVLAR_VEST_Y,			NOTHING,			DESTRUCTION},
	{COMPOUND18,			KEVLAR2_VEST_Y,			NOTHING,			DESTRUCTION},
	{COMPOUND18,			SPECTRA_VEST_Y,			NOTHING,			DESTRUCTION},
	{COMPOUND18,			LEATHER_JACKET_W_KEVLAR_Y,	NOTHING,			DESTRUCTION},
	{COMPOUND18,			KEVLAR_LEGGINGS_Y,		NOTHING,			DESTRUCTION},
	{COMPOUND18,			SPECTRA_LEGGINGS_Y,		NOTHING,			DESTRUCTION},
	{COMPOUND18,			KEVLAR_HELMET_Y,		NOTHING,			DESTRUCTION},
	{COMPOUND18,			SPECTRA_HELMET_Y,		NOTHING,			DESTRUCTION},

	{JAR_QUEEN_CREATURE_BLOOD,	FLAK_JACKET,			FLAK_JACKET_Y,			TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_VEST,			KEVLAR_VEST_Y,			TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_VEST,			SPECTRA_VEST_Y,			TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	LEATHER_JACKET_W_KEVLAR,	LEATHER_JACKET_W_KEVLAR_Y,	TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR2_VEST,			KEVLAR2_VEST_Y,			TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_LEGGINGS,		KEVLAR_LEGGINGS_Y,		TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_LEGGINGS,		SPECTRA_LEGGINGS_Y,		TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_HELMET,			KEVLAR_HELMET_Y,		TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_HELMET,			SPECTRA_HELMET_Y,		TREAT_ARMOUR},
	{JAR_QUEEN_CREATURE_BLOOD,	FLAK_JACKET_18,			NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_VEST_18,			NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR2_VEST_18,		NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_VEST_18,		NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	LEATHER_JACKET_W_KEVLAR_18,	NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_LEGGINGS_18,		NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_LEGGINGS_18,		NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	KEVLAR_HELMET_18,		NOTHING,			DESTRUCTION},
	{JAR_QUEEN_CREATURE_BLOOD,	SPECTRA_HELMET_18,		NOTHING,			DESTRUCTION},

	{RDX,				TNT,				HMX,				EXPLOSIVE},
	{RDX,				C1,				C4,				EXPLOSIVE},
	{TNT,				RDX,				HMX,				EXPLOSIVE},
	{C1,				RDX,				C4,				EXPLOSIVE},

	{STRING,			TIN_CAN,			STRING_TIED_TO_TIN_CAN,		EASY_MERGE},
	{TIN_CAN,			STRING,				STRING_TIED_TO_TIN_CAN,		EASY_MERGE},

	{FLASH_DEVICE,			DISPLAY_UNIT,			XRAY_DEVICE,			ELECTRONIC_MERGE},
	{DISPLAY_UNIT,			FLASH_DEVICE,			XRAY_DEVICE,			ELECTRONIC_MERGE},
};

struct ComboMergeInfoStruct
{
	UINT16 usItem;
	UINT16 usAttachment[2];
	UINT16 usResult;
};


static ComboMergeInfoStruct const AttachmentComboMerge[] =
{
	// base item	attach 1	attach 2	result
	{ALUMINUM_ROD,	{SPRING,	NOTHING},	SPRING_AND_BOLT_UPGRADE},
	{STEEL_ROD,	{QUICK_GLUE,	DUCT_TAPE},	GUN_BARREL_EXTENDER},
	{FUMBLE_PAK,	{XRAY_BULB,	CHEWING_GUM},	FLASH_DEVICE},
	{LAME_BOY,	{COPPER_WIRE,	NOTHING},	DISPLAY_UNIT},
};


BOOLEAN ItemIsLegal( UINT16 usItemIndex )
{
	//if the user has selected the reduced gun list
	if( !gGameOptions.fGunNut )
	{
		const ItemModel *item = GCM->getItem(usItemIndex);

		if(item->isGun() && item->asWeapon()->isInBigGunList())
		{
			return false;
		}

		if(item->isAmmo() && item->asAmmo()->isInBigGunList())
		{
			return false;
		}
	}

	return(TRUE);
}

BOOLEAN WeaponInHand(const SOLDIERTYPE* const pSoldier)
{
	if ( GCM->getItem(pSoldier->inv[HANDPOS].usItem)->getItemClass() & (IC_WEAPON | IC_THROWN) )
	{
		OBJECTTYPE const& o = pSoldier->inv[HANDPOS];
		if (HasObjectImprint(o))
		{
			if (pSoldier->ubProfile != NO_PROFILE)
			{
				if (pSoldier->inv[HANDPOS].ubImprintID != pSoldier->ubProfile)
				{
					return( FALSE );
				}
			}
			else
			{
				if (pSoldier->inv[HANDPOS].ubImprintID != (NO_PROFILE + 1) )
				{
					return( FALSE );
				}
			}
		}
		if (pSoldier->inv[HANDPOS].bGunStatus >= USABLE)
		{
			return( TRUE );
		}
	}
	// return -1 or some "broken" value if weapon is broken?
	return( FALSE );
}

UINT8 ItemSlotLimit( UINT16 usItem, INT8 bSlot )
{
	UINT8 ubSlotLimit;

	if ( bSlot < BIGPOCK1POS )
	{
		return( 1 );
	}
	else
	{
		ubSlotLimit = GCM->getItem(usItem)->getPerPocket();
		if (bSlot >= SMALLPOCK1POS && ubSlotLimit > 1)
		{
			ubSlotLimit /= 2;
		}
		return( ubSlotLimit );
	}
}

UINT32 MoneySlotLimit( INT8 bSlot )
{
	if ( bSlot >= SMALLPOCK1POS )
	{
		return( MAX_MONEY_PER_SLOT / 2 );
	}
	else
	{
		return( MAX_MONEY_PER_SLOT );
	}
}


INT8 FindObj(const SOLDIERTYPE* pSoldier, UINT16 usItem)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (pSoldier->inv[bLoop].usItem == usItem)
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindUsableObj( const SOLDIERTYPE * pSoldier, UINT16 usItem )
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( pSoldier->inv[bLoop].usItem == usItem && pSoldier->inv[bLoop].bStatus[0] >= USABLE )
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}


static INT8 FindObjExcludingSlot(const SOLDIERTYPE* pSoldier, UINT16 usItem, INT8 bExcludeSlot)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (bLoop == bExcludeSlot)
		{
			continue;
		}
		if (pSoldier->inv[bLoop].usItem == usItem)
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindExactObj( const SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj )
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (pObj == &pSoldier->inv[bLoop]) return bLoop;
	}
	return( NO_SLOT );
}


INT8 FindObjWithin( SOLDIERTYPE * pSoldier, UINT16 usItem, INT8 bLower, INT8 bUpper )
{
	INT8 bLoop;

	for (bLoop = bLower; bLoop <= bUpper; bLoop++)
	{
		if (pSoldier->inv[bLoop].usItem == usItem)
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}


INT8 FindObjInObjRange(const SOLDIERTYPE* const pSoldier, UINT16 usItem1, UINT16 usItem2)
{
	INT8   bLoop;
	UINT16 usTemp;

	if (usItem1 > usItem2 )
	{
		// swap the two...
		usTemp = usItem2;
		usItem2 = usItem1;
		usItem1 = usTemp;
	}

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		usTemp = pSoldier->inv[bLoop].usItem;
		if ( usTemp >= usItem1 && usTemp <= usItem2 )
		{
			return( bLoop );
		}
	}

	return( ITEM_NOT_FOUND );
}


INT8 FindObjClass(const SOLDIERTYPE* const pSoldier, const UINT32 usItemClass)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (GCM->getItem(pSoldier->inv[bLoop].usItem)->getItemClass() & usItemClass)
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}


INT8 FindAIUsableObjClass( const SOLDIERTYPE * pSoldier, 	UINT32 usItemClass )
{
	// finds the first object of the specified class which does NOT have
	// the "unusable by AI" flag set.

	// uses & rather than == so that this function can search for any weapon
	INT8 bLoop;

	// This is for the AI only so:

	// Do not consider tank cannons or rocket launchers to be "guns"

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( (GCM->getItem(pSoldier->inv[bLoop].usItem)->getItemClass() & usItemClass) && !(pSoldier->inv[bLoop].fFlags & OBJECT_AI_UNUSABLE) && (pSoldier->inv[bLoop].bStatus[0] >= USABLE ) )
		{
			if ( usItemClass == IC_GUN && EXPLOSIVE_GUN( pSoldier->inv[bLoop].usItem ) )
			{
				continue;
			}
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindAIUsableObjClassWithin( const SOLDIERTYPE * pSoldier, 	UINT32 usItemClass, INT8 bLower, INT8 bUpper )
{
	INT8 bLoop;

	// This is for the AI only so:
	// Do not consider tank cannons or rocket launchers to be "guns"

	for (bLoop = bLower; bLoop <= bUpper; bLoop++)
	{
		if ( (GCM->getItem(pSoldier->inv[bLoop].usItem)->getItemClass() & usItemClass) && !(pSoldier->inv[bLoop].fFlags & OBJECT_AI_UNUSABLE) && (pSoldier->inv[bLoop].bStatus[0] >= USABLE ) )
		{
			if ( usItemClass == IC_GUN && EXPLOSIVE_GUN( pSoldier->inv[bLoop].usItem ) )
			{
				continue;
			}
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindEmptySlotWithin( const SOLDIERTYPE * pSoldier, INT8 bLower, INT8 bUpper )
{
	INT8 bLoop;

	for (bLoop = bLower; bLoop <= bUpper; bLoop++)
	{
		if (pSoldier->inv[bLoop].usItem == 0)
		{
			if (bLoop == SECONDHANDPOS && GCM->getItem(pSoldier->inv[HANDPOS].usItem)->isTwoHanded())
			{
				continue;
			}
			else
			{
				return( bLoop );
			}
		}
	}
	return( ITEM_NOT_FOUND );
}


static BOOLEAN GLGrenadeInSlot(const SOLDIERTYPE* pSoldier, INT8 bSlot)
{
	switch (pSoldier->inv[bSlot].usItem)
	{
		case GL_HE_GRENADE:
		case GL_TEARGAS_GRENADE:
		case GL_STUN_GRENADE:
		case GL_SMOKE_GRENADE:
			return(TRUE);
		default:
			return(FALSE);
	}
}

// for grenade launchers
INT8 FindGLGrenade( const SOLDIERTYPE * pSoldier )
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if (GLGrenadeInSlot( pSoldier, bLoop ))
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}

INT8 FindThrowableGrenade( const SOLDIERTYPE * pSoldier )
{
	INT8 bLoop;
	BOOLEAN fCheckForFlares = FALSE;

	// JA2Gold: give some priority to looking for flares when at night
	// this is AI only so we can put in some customization for night
	if (GetTimeOfDayAmbientLightLevel() == NORMAL_LIGHTLEVEL_NIGHT)
	{
		if (pSoldier->bLife > (pSoldier->bLifeMax / 2))
		{
			fCheckForFlares = TRUE;
		}
	}
	if (fCheckForFlares)
	{
		// Do a priority check for flares first
		for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
		{
			if (pSoldier->inv[ bLoop ].usItem == BREAK_LIGHT)
			{
				return( bLoop );
			}
		}
	}

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( (GCM->getItem(pSoldier->inv[ bLoop ].usItem)->isGrenade()) && !GLGrenadeInSlot( pSoldier, bLoop ) )
		{
			return( bLoop );
		}
	}
	return( NO_SLOT );
}


INT8 FindAttachment(const OBJECTTYPE* pObj, UINT16 usItem)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++)
	{
		if (pObj->usAttachItem[bLoop] == usItem)
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}


INT8 FindAttachmentByClass(OBJECTTYPE const* const pObj, UINT32 const uiItemClass)
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++)
	{
		if (GCM->getItem(pObj->usAttachItem[bLoop])->getItemClass() == uiItemClass)
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}

INT8 FindLaunchable( const SOLDIERTYPE * pSoldier, UINT16 usWeapon )
{
	INT8 bLoop;

	for (bLoop = 0; bLoop < NUM_INV_SLOTS; bLoop++)
	{
		if ( ValidLaunchable( pSoldier->inv[ bLoop ].usItem , usWeapon ) )
		{
			return( bLoop );
		}
	}
	return( ITEM_NOT_FOUND );
}


INT8 FindLaunchableAttachment(const OBJECTTYPE* const pObj, const UINT16 usWeapon)
{
	INT8 bLoop;

	for ( bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++ )
	{
		if ( pObj->usAttachItem[ bLoop ] != NOTHING && ValidLaunchable( pObj->usAttachItem[ bLoop ], usWeapon ) )
		{
			return( bLoop );
		}
	}

	return( ITEM_NOT_FOUND );
}


bool ItemHasAttachments(OBJECTTYPE const& o)
{
	return o.usAttachItem[0] != NOTHING ||
		o.usAttachItem[1] != NOTHING ||
		o.usAttachItem[2] != NOTHING ||
		o.usAttachItem[3] != NOTHING;
}


// Determine if it is possible to add this attachment to the CLASS of this item
// (i.e. to any item in the class)
static BOOLEAN ValidAttachmentClass(UINT16 usAttachment, UINT16 usItem)
{
	for (auto const& ai : AttachmentInfo)
	{
		// see comment for AttachmentInfo array for why we skip IC_NONE
		if (ai.uiItemClass == IC_NONE) continue;

		if (ai.usItem == usAttachment && ai.uiItemClass == GCM->getItem(usItem)->getItemClass())
		{
			return TRUE;
		}
	}
	return FALSE;
}


static const AttachmentInfoStruct* GetAttachmentInfo(const UINT16 usItem)
{
	for (auto const& ai : AttachmentInfo)
	{
		if (ai.usItem == usItem) return &ai;
	}
	return NULL;
}


bool ValidAttachment(UINT16 const attachment, UINT16 const item)
{
	const ItemModel *itemModel = GCM->getItem(item);
	if (itemModel && itemModel->canBeAttached(attachment))
	{
		return true;
	}

	{
		auto const it = g_attachments.find(attachment);
		if (it != g_attachments.end() && it->second.count(item) == 1) return true;
	}

	if (gamepolicy(extra_attachments))
	{
		auto const it = g_attachments_mod.find(attachment);
		if (it != g_attachments_mod.end() && (*it->second).count(item) == 1) return true;
	}

	return false;
}


BOOLEAN ValidItemAttachment(const OBJECTTYPE* const pObj, const UINT16 usAttachment, const BOOLEAN fAttemptingAttachment)
{
	BOOLEAN fSameItem = FALSE, fSimilarItems = FALSE;
	UINT16  usSimilarItem = NOTHING;

	if ( !ValidAttachment( usAttachment, pObj->usItem ) )
	{
		// check for an underslung grenade launcher attached to the gun
		if ( (FindAttachment( pObj, UNDER_GLAUNCHER ) != ITEM_NOT_FOUND) && ValidLaunchable( usAttachment, UNDER_GLAUNCHER ) )
		{
			return ( TRUE );
		}
		else
		{
			if ( fAttemptingAttachment && ValidAttachmentClass( usAttachment, pObj->usItem ) )
			{
				// well, maybe the player thought he could
				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(g_langRes->Message[STR_CANT_ATTACH], GCM->getItem(usAttachment)->getName(), GCM->getItem(pObj->usItem)->getName()));
			}

			return( FALSE );
		}
	}
	// special conditions go here
	// can't have two of the same attachment on an item
	if (FindAttachment( pObj, usAttachment ) != ITEM_NOT_FOUND)
	{
		fSameItem = TRUE;
	}

	// special code for items which won't attach if X is present
	switch( usAttachment )
	{
		case BIPOD:
			if ( FindAttachment( pObj, UNDER_GLAUNCHER) != ITEM_NOT_FOUND )
			{
				fSimilarItems = TRUE;
				usSimilarItem = UNDER_GLAUNCHER;
			}
			break;
		case UNDER_GLAUNCHER:
			if ( FindAttachment( pObj, BIPOD ) != ITEM_NOT_FOUND )
			{
				fSimilarItems = TRUE;
				usSimilarItem = BIPOD;
			}
			break;
		case DETONATOR:
			if( FindAttachment( pObj, REMDETONATOR ) != ITEM_NOT_FOUND )
			{
				fSameItem = TRUE;
			}
			break;
		case REMDETONATOR:
			if( FindAttachment( pObj, DETONATOR ) != ITEM_NOT_FOUND )
			{
				fSameItem = TRUE;
			}
			break;
	}

	if (fAttemptingAttachment)
	{
		if (fSameItem)
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, g_langRes->Message[ STR_ATTACHMENT_ALREADY ] );
			return( FALSE );
		}
		else if (fSimilarItems)
		{
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(g_langRes->Message[ STR_CANT_USE_TWO_ITEMS ], GCM->getItem(usSimilarItem)->getName(), GCM->getItem(usAttachment)->getName()) );
			return( FALSE );
		}
	}

	return( TRUE );
}

//Determines if it is possible to equip this weapon with this ammo.
bool ValidAmmoType( UINT16 usItem, UINT16 usAmmoType )
{
	if (GCM->getItem(usItem)->getItemClass() == IC_GUN && GCM->getItem(usAmmoType)->getItemClass() == IC_AMMO)
	{
		return GCM->getWeapon(usItem)->matches(GCM->getItem(usAmmoType)->asAmmo()->calibre);
	}
	return false;
}


BOOLEAN CompatibleFaceItem(UINT16 const item1, UINT16 const item2)
{
	if (item2 == NOTHING) return TRUE;
	for (auto const& i : CompatibleFaceItems)
	{
		if (i[0] == item1 && i[1] == item2) return TRUE;
		if (i[0] == item2 && i[1] == item1) return TRUE;
	}
	return FALSE;
}


BOOLEAN ValidLaunchable( UINT16 usLaunchable, UINT16 usItem )
{
	auto const it = Launchable.find(usLaunchable);
	if (it != Launchable.end())
	{
		return it->second.count(usItem) == 1;
	}

	return FALSE;
}


UINT16 GetLauncherFromLaunchable( UINT16 usLaunchable )
{
	auto const it = Launchable.find(usLaunchable);
	return it != Launchable.end() ? *it->second.begin() : NOTHING;
}


static BOOLEAN EvaluateValidMerge(UINT16 const usMerge, UINT16 const usItem, UINT16* const pusResult, UINT8* const pubType)
{
	// NB "usMerge" is the object being merged with (e.g. compound 18)
	// "usItem" is the item being merged "onto" (e.g. kevlar vest)

	if (usMerge == usItem && GCM->getItem(usItem)->getItemClass() == IC_AMMO)
	{
		*pusResult = usItem;
		*pubType   = COMBINE_POINTS;
		return TRUE;
	}

	for (auto const& m : Merge)
	{
		if (m.item1 != usMerge) continue;
		if (m.item2 != usItem)  continue;

		*pusResult = m.result;
		*pubType   = m.action;
		return TRUE;
	}

	return FALSE;
}

BOOLEAN ValidMerge( UINT16 usMerge, UINT16 usItem )
{
	UINT16 usIgnoreResult;
	UINT8  ubIgnoreType;
	return( EvaluateValidMerge( usMerge, usItem, &usIgnoreResult, &ubIgnoreType ) );
}


UINT8 CalculateObjectWeight(OBJECTTYPE const* const o)
{
	const ItemModel *item  = GCM->getItem(o->usItem);
	UINT16          weight = item->getWeight(); // Start with base weight

	if (item->getPerPocket() <= 1)
	{
		// Account for any attachments
		FOR_EACH(UINT16 const, i, o->usAttachItem)
		{
			if (*i == NOTHING) continue;
			weight += GCM->getItem(*i)->getWeight();
		}

		if (GCM->getItem(o->usItem)->getItemClass() == IC_GUN && o->ubGunShotsLeft > 0)
		{ // Add in weight of ammo
			weight += GCM->getItem(o->usGunAmmoItem)->getWeight();
		}
	}

	// Make sure it really fits into that UINT8, in case we ever add anything real
	// heavy with attachments/ammo
	Assert(weight <= 255);
	return weight;
}


UINT32 CalculateCarriedWeight(SOLDIERTYPE const* const s)
{
	UINT32 total_weight = 0;
	CFOR_EACH_SOLDIER_INV_SLOT(i, *s)
	{
		UINT16 weight = i->ubWeight;
		if (GCM->getItem(i->usItem)->getPerPocket() > 1)
		{
			// Account for # of items
			weight *= i->ubNumberOfObjects;
		}
		total_weight += weight;
	}

	UINT8 strength_for_carrying = EffectiveStrength(s);
	if (strength_for_carrying > 80)
	{
		strength_for_carrying += strength_for_carrying - 80;
	}

	// For now, assume soldiers can carry 1/2 their strength in kg without
	// penalty. Instead of multiplying by 100 for percent, and then dividing by 10
	// to account for weight units being in 10ths of kilos, not kilos... we just
	// start with 10 instead of 100!
	UINT32 const percent = 10 * total_weight / (strength_for_carrying / 2);
	return percent;
}


void DeleteObj(OBJECTTYPE * pObj )
{
	*pObj = OBJECTTYPE{};
}


void SwapObjs( OBJECTTYPE * pObj1, OBJECTTYPE * pObj2 )
{
	OBJECTTYPE Temp = *pObj1;
	*pObj1 = *pObj2;
	*pObj2 = Temp;
}

void RemoveObjFrom( OBJECTTYPE * pObj, UINT8 ubRemoveIndex )
{
	// remove 1 object from an OBJECTTYPE, starting at index bRemoveIndex
	UINT8 ubLoop;

	if (pObj->ubNumberOfObjects < ubRemoveIndex)
	{
		// invalid index!
		return;
	}
	else if (pObj->ubNumberOfObjects == 1)
	{
		// delete!
		DeleteObj( pObj );
	}
	else
	{
		// shift down all the values that should be down
		for (ubLoop = ubRemoveIndex + 1; ubLoop < pObj->ubNumberOfObjects; ubLoop++)
		{
			pObj->bStatus[ubLoop - 1] = pObj->bStatus[ubLoop];
		}
		// and set the upper value to 0
		pObj->bStatus[pObj->ubNumberOfObjects - 1] = 0;
		// make the number of objects recorded match the array
		pObj->ubNumberOfObjects--;
	}
}

void RemoveObjs( OBJECTTYPE * pObj, UINT8 ubNumberToRemove )
{
	// remove a certain number of objects from an OBJECTTYPE, starting at index 0
	UINT8 ubLoop;

	if (ubNumberToRemove == 0)
	{
		return;
	}
	if (ubNumberToRemove >= pObj->ubNumberOfObjects)
	{
		// delete!
		DeleteObj( pObj );
	}
	else
	{
		for (ubLoop = 0; ubLoop < ubNumberToRemove; ubLoop++)
		{
			RemoveObjFrom( pObj, 0 );
		}
		pObj->ubWeight = CalculateObjectWeight( pObj );
	}
}

void GetObjFrom( OBJECTTYPE * pObj, UINT8 ubGetIndex, OBJECTTYPE * pDest )
{
	if (!pDest || ubGetIndex >= pObj->ubNumberOfObjects)
	{
		return;
	}
	if (pObj->ubNumberOfObjects == 1)
	{
		*pDest = *pObj;
		DeleteObj( pObj );
	}
	else
	{
		pDest->usItem = pObj->usItem;
		pDest->bStatus[0] = pObj->bStatus[ubGetIndex];
		pDest->ubNumberOfObjects = 1;
		pDest->ubWeight = CalculateObjectWeight( pDest );
		RemoveObjFrom( pObj, ubGetIndex );
		pObj->ubWeight = CalculateObjectWeight( pObj );
	}
}


void DamageObj( OBJECTTYPE * pObj, INT8 bAmount )
{
	if (bAmount >= pObj->bStatus[0])
	{
		pObj->bStatus[0] = 1;
	}
	else
	{
		pObj->bStatus[0] -= bAmount;
	}
}


void StackObjs(OBJECTTYPE* pSourceObj, OBJECTTYPE* pTargetObj, UINT8 ubNumberToCopy)
{
	UINT8 ubLoop;

	// copy over N status values
	for (ubLoop = 0; ubLoop < ubNumberToCopy; ubLoop++)
	{
		pTargetObj->bStatus[ubLoop + pTargetObj->ubNumberOfObjects] = pSourceObj->bStatus[ubLoop ];
	}

	// now in the source object, move the rest down N places
	for (ubLoop = ubNumberToCopy; ubLoop < pSourceObj->ubNumberOfObjects; ubLoop++)
	{
		pSourceObj->bStatus[ubLoop - ubNumberToCopy] = pSourceObj->bStatus[ubLoop];
	}

	pTargetObj->ubNumberOfObjects += ubNumberToCopy;
	RemoveObjs( pSourceObj, ubNumberToCopy );
	pSourceObj->ubWeight = CalculateObjectWeight( pSourceObj );
	pTargetObj->ubWeight = CalculateObjectWeight( pTargetObj );
}


void CleanUpStack(OBJECTTYPE* const o, OBJECTTYPE* const cursor_o)
{
	const ItemModel * item = GCM->getItem(o->usItem);
	if (!(item->isAmmo()) &&
		!(item->isKit())  &&
		!(item->isMedkit()))
	{
		return;
	}

	INT8 const max_points = item->isAmmo() ?
		item->asAmmo()->capacity : 100;

	if (cursor_o && cursor_o->usItem == o->usItem)
	{
		for (INT8 i = (INT8)cursor_o->ubNumberOfObjects - 1; i >= 0; --i)
		{
			INT8& src_status = cursor_o->bStatus[i];
			if (src_status <= 0) continue;

			// take the points here and distribute over the lower #d items
			for (INT8 k = o->ubNumberOfObjects - 1; k >= 0; --k)
			{
				INT8& dst_status = o->bStatus[k];
				if (dst_status >= max_points) continue;

				INT8 const points_to_move = std::min(max_points - dst_status, int(src_status));
				dst_status += points_to_move;
				src_status -= points_to_move;
				if (src_status != 0) continue;

				// done!
				--cursor_o->ubNumberOfObjects;
				break;
			}
		}
	}

	for (INT8 i = (INT8)o->ubNumberOfObjects - 1; i >= 0; --i)
	{
		INT8& src_status = o->bStatus[i];
		if (src_status <= 0) continue;

		// take the points here and distribute over the lower #d items
		for (INT8 k = i - 1; k >= 0; --k)
		{
			INT8& dst_status = o->bStatus[k];
			if (dst_status >= max_points) continue;

			INT8 const points_to_move = std::min(max_points - dst_status, int(src_status));
			dst_status += points_to_move;
			src_status -= points_to_move;
			if (src_status != 0) continue;

			// done!
			--o->ubNumberOfObjects;
			break;
		}
	}
}


BOOLEAN PlaceObjectAtObjectIndex( OBJECTTYPE * pSourceObj, OBJECTTYPE * pTargetObj, UINT8 ubIndex )
{
	INT8 bTemp;

	if (pSourceObj->usItem != pTargetObj->usItem)
	{
		return( TRUE );
	}
	if (ubIndex < pTargetObj->ubNumberOfObjects)
	{
		// swap
		bTemp = pSourceObj->bStatus[0];
		pSourceObj->bStatus[0] = pTargetObj->bStatus[ubIndex];
		pTargetObj->bStatus[ubIndex] = bTemp;
		return( TRUE );
	}
	else
	{
		// add to end
		StackObjs( pSourceObj, pTargetObj, 1 );
		return( FALSE );
	}
}

#define RELOAD_NONE		0
#define RELOAD_PLACE		1
#define RELOAD_SWAP		2
#define RELOAD_TOPOFF		3
#define RELOAD_AUTOPLACE_OLD	4

BOOLEAN ReloadGun( SOLDIERTYPE * pSoldier, OBJECTTYPE * pGun, OBJECTTYPE * pAmmo )
{
	OBJECTTYPE OldAmmo;
	UINT8   ubBulletsToMove;
	BOOLEAN fSameAmmoType;
	BOOLEAN fSameMagazineSize;
	BOOLEAN fReloadingWithStack;
	BOOLEAN fEmptyGun;
	INT8    bReloadType;
	UINT16  usNewAmmoItem;

	if (pGun->usItem == ROCKET_LAUNCHER) return( FALSE ); // IC_GUN but uses no ammo (LAW)

	INT8 bAPs = 0; // XXX HACK000E
	if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		bAPs = GetAPsToReloadGunWithAmmo( pGun, pAmmo );
		if ( !EnoughPoints( pSoldier, bAPs, 0,TRUE ) )
		{
			return( FALSE );
		}

	}

	if ( GCM->getItem(pGun->usItem)->getItemClass() == IC_LAUNCHER || pGun->usItem == TANK_CANNON )
	{
		if (!AttachObject(pSoldier, pGun, pAmmo))
		{
			// abort
			return( FALSE );
		}
	}
	else
	{
		fEmptyGun = (pGun->ubGunShotsLeft == 0);
		fReloadingWithStack = (pAmmo->ubNumberOfObjects > 1);
		fSameAmmoType = ( pGun->ubGunAmmoType == GCM->getItem(pAmmo->usItem)->asAmmo()->ammoType->index );
		fSameMagazineSize = ( GCM->getItem(pAmmo->usItem)->asAmmo()->capacity == GCM->getWeapon(pGun->usItem)->ubMagSize );

		if (fEmptyGun)
		{
			bReloadType = RELOAD_PLACE;
		}
		else
		{
			// record old ammo
			OldAmmo = OBJECTTYPE{};
			OldAmmo.usItem = pGun->usGunAmmoItem;
			OldAmmo.ubNumberOfObjects = 1;
			OldAmmo.ubShotsLeft[0] = pGun->ubGunShotsLeft;

			if (fSameMagazineSize)
			{
				if (fSameAmmoType)
				{
					if (gTacticalStatus.uiFlags & INCOMBAT)
					{
						bReloadType = RELOAD_SWAP;
					}
					else
					{
						bReloadType = RELOAD_TOPOFF;
					}
				}
				else
				{
					if (!fReloadingWithStack)
					{
						bReloadType = RELOAD_SWAP;
					}
					else
					{
						bReloadType = RELOAD_AUTOPLACE_OLD;
					}
				}
			}
			else  // diff sized magazines
			{
				if (fSameAmmoType)
				{
					bReloadType = RELOAD_TOPOFF;
				}
				else
				{
					bReloadType = RELOAD_AUTOPLACE_OLD;
				}
			}
		}

		if (fSameMagazineSize)
		{
			// record new ammo item for gun
			usNewAmmoItem = pAmmo->usItem;

			if (bReloadType == RELOAD_TOPOFF)
			{
				ubBulletsToMove = std::min(int(pAmmo->ubShotsLeft[0]), GCM->getWeapon(pGun->usItem)->ubMagSize - pGun->ubGunShotsLeft);
			}
			else
			{
				ubBulletsToMove = pAmmo->ubShotsLeft[0];
			}

		}
		else if (GCM->getItem(pAmmo->usItem)->asAmmo()->capacity > GCM->getWeapon(pGun->usItem)->ubMagSize)
		{
			usNewAmmoItem = pAmmo->usItem - 1;
			if (bReloadType == RELOAD_TOPOFF)
			{
				ubBulletsToMove = std::min(int(pAmmo->ubShotsLeft[0]), GCM->getWeapon(pGun->usItem)->ubMagSize - pGun->ubGunShotsLeft);
			}
			else
			{
				ubBulletsToMove = std::min(pAmmo->ubShotsLeft[0], GCM->getWeapon(pGun->usItem)->ubMagSize);
			}
		}
		else // mag is smaller than weapon mag
		{
			usNewAmmoItem = pAmmo->usItem + 1;
			if (bReloadType == RELOAD_TOPOFF)
			{
				ubBulletsToMove = std::min(int(pAmmo->ubShotsLeft[0]), GCM->getWeapon(pGun->usItem)->ubMagSize - pGun->ubGunShotsLeft);
			}
			else
			{
				ubBulletsToMove = std::min(pAmmo->ubShotsLeft[0], GCM->getWeapon(pGun->usItem)->ubMagSize);
			}
		}


		switch( bReloadType )
		{

			case RELOAD_PLACE:
				pGun->ubGunShotsLeft = ubBulletsToMove;
				pGun->ubGunAmmoType = GCM->getItem(pAmmo->usItem)->asAmmo()->ammoType->index;
				pGun->usGunAmmoItem = usNewAmmoItem;
				break;

			case RELOAD_SWAP:
				pGun->ubGunShotsLeft = ubBulletsToMove;
				pGun->ubGunAmmoType = GCM->getItem(pAmmo->usItem)->asAmmo()->ammoType->index;
				pGun->usGunAmmoItem = usNewAmmoItem;
				if (fReloadingWithStack)
				{
					// add to end of stack
					StackObjs( &OldAmmo, pAmmo, 1 );
				}
				else
				{
					// Copying the old ammo to the cursor in turnbased could screw up for the player
					// (suppose his inventory is full!)

					if ((gTacticalStatus.uiFlags & INCOMBAT) && !EnoughPoints( pSoldier, (INT8) (bAPs + AP_PICKUP_ITEM), 0, FALSE))
					{
						// try autoplace
						if ( !AutoPlaceObject( pSoldier, &OldAmmo, FALSE ) )
						{
							// put it on the ground
							AddItemToPool(pSoldier->sGridNo, &OldAmmo, VISIBLE, pSoldier->bLevel, 0 , -1);
						}
						// delete the object now in the cursor
						DeleteObj( pAmmo );
					}
					else
					{
						// copy the old ammo to the cursor
						*pAmmo = OldAmmo;
					}
				}
				break;
			case RELOAD_AUTOPLACE_OLD:
				if ( !AutoPlaceObject( pSoldier, &OldAmmo, TRUE ) )
				{
					// error msg!
					return( FALSE );
				}
				// place first ammo in gun
				pGun->ubGunShotsLeft = ubBulletsToMove;
				pGun->ubGunAmmoType = GCM->getItem(pAmmo->usItem)->asAmmo()->ammoType->index;
				pGun->usGunAmmoItem = usNewAmmoItem;

				break;

			case RELOAD_TOPOFF:
				// ADD that many bullets to gun
				pGun->ubGunShotsLeft += ubBulletsToMove;
				break;

		}

		if ( ! ( bReloadType == RELOAD_SWAP && !fReloadingWithStack ) )
		{
			// remove # of bullets, delete 1 object if necessary

			pAmmo->ubShotsLeft[0] -= ubBulletsToMove;
			if (pAmmo->ubShotsLeft[0] == 0)
			{
				RemoveObjs( pAmmo, 1 );
			}

		}
	}

	// OK, let's play a sound of reloading too...
	// If this guy is visible...
	if ( pSoldier->bVisible != -1 )
	{
		// Play some effects!
		SoundID const usReloadSound = GCM->getWeapon(pGun->usItem)->sReloadSound;
		if (usReloadSound != NO_SOUND && !IsAutoResolveActive())
		{
			PlayJA2Sample(usReloadSound, HIGHVOLUME, 1, MIDDLEPAN);
		}
	}

	if (pSoldier->bTeam == OUR_TEAM)
	{
		// spit out a message if this is one of our folks reloading
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_PLAYER_RELOADS], pSoldier->name) );
	}

	DeductPoints( pSoldier, bAPs, 0 );
	pGun->ubWeight = CalculateObjectWeight( pGun );

	if ( pGun->bGunAmmoStatus >= 0 )
	{
		// make sure gun ammo status is 100, if gun isn't jammed
		pGun->bGunAmmoStatus = 100;
	}

	return( TRUE );
}

BOOLEAN EmptyWeaponMagazine( OBJECTTYPE * pWeapon, OBJECTTYPE *pAmmo )
{
	CHECKF( pAmmo != NULL );

	if ( pWeapon->ubGunShotsLeft > 0 )
	{
		// start by erasing ammo item, just in case...
		DeleteObj( pAmmo );

		pAmmo->ubShotsLeft[0] = pWeapon->ubGunShotsLeft;
		pAmmo->usItem = pWeapon->usGunAmmoItem;
		pAmmo->ubNumberOfObjects = 1;

		pWeapon->ubGunShotsLeft = 0;
		pWeapon->ubGunAmmoType = 0;
		//pWeapon->usGunAmmoItem = 0; // leaving the ammo item the same for auto-reloading purposes

		// Play some effects!
		SoundID const usReloadSound = GCM->getWeapon(pWeapon->usItem)->sReloadSound;
		if (usReloadSound != NO_SOUND)
		{
			PlayJA2Sample(usReloadSound, HIGHVOLUME, 1, MIDDLEPAN);
		}

		pWeapon->ubWeight = CalculateObjectWeight( pWeapon );

		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


INT8 FindAmmo(const SOLDIERTYPE* s, const CalibreModel * calibre, UINT8 const mag_size, INT8 const exclude_slot)
{
	for (INT8 slot = HANDPOS; slot != NUM_INV_SLOTS; ++slot)
	{
		if (slot == exclude_slot) continue;
		const ItemModel * item = GCM->getItem(s->inv[slot].usItem);
		if (!item->isAmmo()) continue;
		const MagazineModel * m = item->asAmmo();
		if (m->calibre->index != calibre->index) continue;
		if (m->capacity != mag_size && mag_size != ANY_MAGSIZE) continue;
		return slot;
	}
	return NO_SLOT;
}


INT8 FindAmmoToReload( const SOLDIERTYPE * pSoldier, INT8 bWeaponIn, INT8 bExcludeSlot )
{
	const OBJECTTYPE *pObj;
	INT8 bSlot;

	if (pSoldier == NULL)
	{
		return( NO_SLOT );
	}
	pObj = &(pSoldier->inv[bWeaponIn]);
	if ( GCM->getItem(pObj->usItem)->getItemClass() == IC_GUN && pObj->usItem != TANK_CANNON )
	{
		// look for same ammo as before
		bSlot = FindObjExcludingSlot( pSoldier, pObj->usGunAmmoItem, bExcludeSlot );
		if (bSlot != NO_SLOT)
		{
			// reload using this ammo!
			return( bSlot );
		}
		// look for any ammo that matches which is of the same calibre and magazine size
		bSlot = FindAmmo( pSoldier, GCM->getWeapon(pObj->usItem)->calibre, GCM->getWeapon(pObj->usItem)->ubMagSize, bExcludeSlot );
		if (bSlot != NO_SLOT)
		{
			return( bSlot );
		}
		else
		{
			// look for any ammo that matches which is of the same calibre (different size okay)
			return( FindAmmo( pSoldier, GCM->getWeapon(pObj->usItem)->calibre, ANY_MAGSIZE, bExcludeSlot ) );
		}
	}
	else
	{
		switch( pObj->usItem )
		{
			case MORTAR:
				return( FindObj( pSoldier, MORTAR_SHELL ) );
			case TANK_CANNON:
				return( FindObj( pSoldier, TANK_SHELL ) );
			case GLAUNCHER:
			case UNDER_GLAUNCHER:
				return( FindObjInObjRange( pSoldier, GL_HE_GRENADE, GL_SMOKE_GRENADE ) );
			default:
				return( NO_SLOT );
		}
	}
}

BOOLEAN AutoReload( SOLDIERTYPE * pSoldier )
{
	OBJECTTYPE *pObj;
	INT8    bSlot, bAPCost;
	BOOLEAN fRet;

	CHECKF( pSoldier );
	pObj = &(pSoldier->inv[HANDPOS]);

	if (GCM->getItem(pObj->usItem)->getItemClass() == IC_GUN || GCM->getItem(pObj->usItem)->getItemClass() == IC_LAUNCHER)
	{
		bSlot = FindAmmoToReload( pSoldier, HANDPOS, NO_SLOT );
		if (bSlot != NO_SLOT)
		{
			// reload using this ammo!
			fRet = ReloadGun( pSoldier, pObj, &(pSoldier->inv[bSlot]) );
			// if we are valid for two-pistol shooting (reloading) and we have enough APs still
			// then do a reload of both guns!
			if (fRet && IsValidSecondHandShotForReloadingPurposes(pSoldier))
			{
				pObj = &(pSoldier->inv[SECONDHANDPOS]);
				bSlot = FindAmmoToReload( pSoldier, SECONDHANDPOS, NO_SLOT );
				if (bSlot != NO_SLOT)
				{
					// ce would reload using this ammo!
					bAPCost = GetAPsToReloadGunWithAmmo( pObj, &(pSoldier->inv[bSlot] ) );
					if ( EnoughPoints( pSoldier, (INT16) bAPCost, 0, FALSE ) )
					{
						// reload the 2nd gun too
						fRet = ReloadGun( pSoldier, pObj, &(pSoldier->inv[bSlot]) );
					}
					else
					{
						ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[ STR_RELOAD_ONLY_ONE_GUN ], pSoldier->name) );
					}
				}
			}

			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
			return( fRet );
		}
	}

	// couldn't reload
	return( FALSE );
}


static ComboMergeInfoStruct const* GetAttachmentComboMerge(OBJECTTYPE const& o)
{
	for (auto const& m : AttachmentComboMerge)
	{
		if (m.usItem == NOTHING) break;
		if (m.usItem != o.usItem) continue;

		// Search for all the appropriate attachments
		FOR_EACH(UINT16 const, k, m.usAttachment)
		{
			UINT16 const attachment = *k;
			if (attachment == NOTHING) continue;
			if (FindAttachment(&o, attachment) == -1) return nullptr; // Didn't find something required
		}

		return &m; // Found everything required
	}

	return nullptr;
}


static void PerformAttachmentComboMerge(OBJECTTYPE& o, ComboMergeInfoStruct const& m)
{
	// This object has been validated as available for an attachment combo merge.
	// - Find all attachments in list and destroy them
	// - Status of new object should be average of items including attachments
	// - Change object
	UINT32 total_status          = o.bStatus[0];
	INT8   n_status_contributors = 1;
	FOR_EACH(UINT16 const, i, m.usAttachment)
	{
		if (*i == NOTHING) continue;

		INT8 const attach_pos = FindAttachment(&o, *i);
		AssertMsg(attach_pos != -1, "Attachment combo merge couldn't find a necessary attachment");

		total_status += o.bAttachStatus[attach_pos];
		++n_status_contributors;

		o.usAttachItem[attach_pos]  = NOTHING;
		o.bAttachStatus[attach_pos] = 0;
	}

	o.usItem     = m.usResult;
	o.bStatus[0] = total_status / n_status_contributors;
}


bool AttachObject(SOLDIERTYPE* const s, OBJECTTYPE* const pTargetObj, OBJECTTYPE* const pAttachment, UINT8 const ubIndexInStack)
{
	OBJECTTYPE& target     = *pTargetObj;
	OBJECTTYPE& attachment = *pAttachment;
	bool const valid_launchable = ValidLaunchable(attachment.usItem, target.usItem);
	if (valid_launchable || ValidItemAttachment(&target, attachment.usItem, TRUE))
	{
		// find an attachment position...
		// second half of this 'if' is for attaching GL grenades to a gun
		INT8 attach_pos;
		if (valid_launchable || (GL_HE_GRENADE <= attachment.usItem && attachment.usItem <= GL_SMOKE_GRENADE))
		{
			// try replacing if possible
			attach_pos = FindAttachmentByClass(&target, GCM->getItem(attachment.usItem)->getItemClass());
			if (attach_pos != NO_SLOT && attachment.ubNumberOfObjects > 1)
			{
				// we can only do a swap if there is only 1 grenade being attached
				return false;
			}
		}
		else
		{
			// try replacing if possible
			attach_pos = FindAttachment(&target, attachment.usItem);
		}

		if (attach_pos == NO_SLOT)
		{
			attach_pos = FindAttachment(&target, NOTHING);
			if (attach_pos == NO_SLOT) return false;
		}

		AttachmentInfoStruct const* attach_info = 0;
		if (s)
		{
			attach_info = GetAttachmentInfo(attachment.usItem);
			// in-game (not behind the scenes) attachment
			if (attach_info && attach_info->bAttachmentSkillCheck != NO_CHECK)
			{
				INT32 const iCheckResult = SkillCheck(s, attach_info->bAttachmentSkillCheck, attach_info->bAttachmentSkillCheckMod);
				if (iCheckResult < 0)
				{
					// the attach failure damages both items
					DamageObj(&target,     (INT8)-iCheckResult);
					DamageObj(&attachment, (INT8)-iCheckResult);
					// there should be a quote here!
					DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
					if (gfInItemDescBox) DeleteItemDescriptionBox();
					return false;
				}
			}

			if (ValidItemAttachment(&target, attachment.usItem, TRUE)) // not launchable
			{
				// Attachment sounds
				const ItemModel * tgt_item = GCM->getItem(target.usItem);
				SoundID const  sound    =
					tgt_item->isWeapon() ? ATTACH_TO_GUN         :
					tgt_item->isArmour() ? ATTACH_CERAMIC_PLATES :
					tgt_item->isBomb()   ? ATTACH_DETONATOR      :
					NO_SOUND;
				if (sound != NO_SOUND) PlayLocationJA2Sample(s->sGridNo, sound, MIDVOLUME, 1);
			}
		}

		UINT16 const temp_item   = target.usAttachItem[attach_pos];
		UINT16 const temp_status = target.bAttachStatus[attach_pos];

		target.usAttachItem[attach_pos]  = attachment.usItem;
		target.bAttachStatus[attach_pos] = attachment.bStatus[0];

		// Transfer any attachment (max 1) from the grenade launcher to the gun
		if (attachment.usItem == UNDER_GLAUNCHER &&
			attachment.usAttachItem[0] != NOTHING)
		{
			INT8 const second_attach_pos = FindAttachment(&target, NOTHING);
			if (second_attach_pos == ITEM_NOT_FOUND)
			{ // Not enough room for all attachments - cancel!
				target.usAttachItem[attach_pos]  = NOTHING;
				target.bAttachStatus[attach_pos] = 0;
				return false;
			}
			else
			{
				target.usAttachItem[second_attach_pos]  = attachment.usAttachItem[0];
				target.bAttachStatus[second_attach_pos] = attachment.bAttachStatus[0];
				attachment.usAttachItem[0]  = NOTHING;
				attachment.bAttachStatus[0] = 0;
			}
		}

		if (temp_item != NOTHING)
		{
			// overwrite/swap!
			CreateItem(temp_item, temp_status, &attachment);
		}
		else
		{
			RemoveObjs(&attachment, 1);
		}

		// Check for attachment merge combos here
		if (ComboMergeInfoStruct const* const m = GetAttachmentComboMerge(target))
		{
			PerformAttachmentComboMerge(target, *m);
			if (attach_info && attach_info->bAttachmentSkillCheckMod < 20)
			{
				StatChange(*s, MECHANAMT, 20 - attach_info->bAttachmentSkillCheckMod, FROM_SUCCESS);
			}
		}

		target.ubWeight = CalculateObjectWeight(&target);
		return true;
	}

	// check for merges
	UINT16 merge_result;
	UINT8  merge_kind;
	if (!EvaluateValidMerge(attachment.usItem, target.usItem, &merge_result, &merge_kind)) return false;

	if (merge_kind != COMBINE_POINTS)
	{
		if (!EnoughPoints(s, AP_MERGE, 0, TRUE)) return false;
		DeductPoints(s, AP_MERGE, 0);
	}

	switch (merge_kind)
	{
		case COMBINE_POINTS:
		{
			// transfer points...
			const ItemModel * tgt_item = GCM->getItem(target.usItem);
			UINT8 const limit = tgt_item->getItemClass() == IC_AMMO ?
						tgt_item->asAmmo()->capacity : 100;

			// count down through # of attaching items and add to status of item in position 0
			for (INT8 bLoop = attachment.ubNumberOfObjects - 1; bLoop >= 0; --bLoop)
			{
				INT8& targetStatus = target.bStatus[ubIndexInStack];
				if (targetStatus + attachment.bStatus[bLoop] <= limit)
				{ // Consume this one totally and continue
					targetStatus += attachment.bStatus[bLoop];
					RemoveObjFrom(&attachment, bLoop);
					// reset loop limit
					bLoop = attachment.ubNumberOfObjects; // add 1 to counteract the -1 from the loop
				}
				else
				{ // Add part of this one and then we're done
					attachment.bStatus[bLoop] -= limit - targetStatus;
					targetStatus               = limit;
					break;
				}
			}
			break;
		}

		case DESTRUCTION:
			// The merge destroyed both items!
			DeleteObj(&target);
			DeleteObj(&attachment);
			DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
			break;

		case ELECTRONIC_MERGE:
			if (s)
			{
				INT32 const iCheckResult = SkillCheck(s, ATTACHING_SPECIAL_ELECTRONIC_ITEM_CHECK, -30);
				if (iCheckResult < 0)
				{
					DamageObj(&target,     (INT8)-iCheckResult);
					DamageObj(&attachment, (INT8)-iCheckResult);
					DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
					return false;
				}
				// grant experience!
			}
			goto default_merge;

		case EXPLOSIVE:
			if (s)
			{
				// requires a skill check, and gives experience
				INT32 const iCheckResult = SkillCheck(s, ATTACHING_DETONATOR_CHECK, -30);
				if (iCheckResult < 0)
				{
					// could have a chance of detonation
					// for now, damage both objects
					DamageObj(&target,     (INT8)-iCheckResult);
					DamageObj(&attachment, (INT8)-iCheckResult);
					DoMercBattleSound(s, BATTLE_SOUND_CURSE1);
					return false;
				}
				StatChange(*s, EXPLODEAMT, 25, FROM_SUCCESS);
			}
			goto default_merge;

		default:
default_merge:
			// the merge will combine the two items
			target.usItem = merge_result;
			if (merge_kind != TREAT_ARMOUR)
			{
				target.bStatus[0] = (target.bStatus[0] + attachment.bStatus[0]) / 2;
			}
			DeleteObj(&attachment);
			target.ubWeight = CalculateObjectWeight(&target);
			if (s && s->bTeam == OUR_TEAM)
			{
				DoMercBattleSound(s, BATTLE_SOUND_COOL1);
			}
			break;
	}
	return true;
}


BOOLEAN CanItemFitInPosition(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, INT8 bPos, BOOLEAN fDoingPlacement)
{
	UINT8 ubSlotLimit;
	INT8  bNewPos;

	switch( bPos )
	{
		case SECONDHANDPOS:
			if (GCM->getItem(pSoldier->inv[HANDPOS].usItem)->isTwoHanded())
			{
				return( FALSE );
			}
			break;
		case HANDPOS:
			if (GCM->getItem(pObj->usItem)->isTwoHanded())
			{
				if (pSoldier->inv[HANDPOS].usItem != NOTHING && pSoldier->inv[SECONDHANDPOS].usItem != NOTHING)
				{
					// two items in hands; try moving the second one so we can swap
					if (GCM->getItem(pSoldier->inv[SECONDHANDPOS].usItem)->getPerPocket() == 0)
					{
						bNewPos = FindEmptySlotWithin( pSoldier, BIGPOCK1POS, BIGPOCK4POS );
					}
					else
					{
						bNewPos = FindEmptySlotWithin( pSoldier, BIGPOCK1POS, SMALLPOCK8POS );
					}
					if (bNewPos == NO_SLOT)
					{
						// nowhere to put second item
						return( FALSE );
					}

					if ( fDoingPlacement )
					{
						// otherwise move it.
						pSoldier->inv[bNewPos] = pSoldier->inv[SECONDHANDPOS];
						DeleteObj( &(pSoldier->inv[SECONDHANDPOS]) );
					}
				}
			}
			break;
		case VESTPOS:
		case HELMETPOS:
		case LEGPOS:
			if (GCM->getItem(pObj->usItem)->getItemClass() != IC_ARMOUR)
			{
				return( FALSE );
			}
			switch (bPos)
			{
				case VESTPOS:
					if (Armour[GCM->getItem(pObj->usItem)->getClassIndex()].ubArmourClass != ARMOURCLASS_VEST)
					{
						return( FALSE );
					}
					break;
				case HELMETPOS:
					if (Armour[GCM->getItem(pObj->usItem)->getClassIndex()].ubArmourClass != ARMOURCLASS_HELMET)
					{
						return( FALSE );
					}
					break;
				case LEGPOS:
					if (Armour[GCM->getItem(pObj->usItem)->getClassIndex()].ubArmourClass != ARMOURCLASS_LEGGINGS)
					{
						return( FALSE );
					}
					break;
				default:
					break;
			}
			break;
		case HEAD1POS:
		case HEAD2POS:
			if (GCM->getItem(pObj->usItem)->getItemClass() != IC_FACE)
			{
				return( FALSE );
			}
		default:
			break;
	}

	ubSlotLimit = ItemSlotLimit( pObj->usItem, bPos );
	if (ubSlotLimit == 0 && bPos >= SMALLPOCK1POS )
	{
		// doesn't fit!
		return( FALSE );
	}

	return( TRUE );
}


static BOOLEAN DropObjIfThereIsRoom(SOLDIERTYPE* pSoldier, INT8 bPos, OBJECTTYPE* pObj)
{
	// try autoplacing item in bSlot elsewhere, then do a placement
	BOOLEAN fAutoPlacedOld;

	fAutoPlacedOld = AutoPlaceObject( pSoldier, &(pSoldier->inv[bPos]), FALSE );
	if ( fAutoPlacedOld )
	{
		return( PlaceObject( pSoldier, bPos, pObj ) );
	}
	else
	{
		return( FALSE );
	}
}


static void CollectKey(SOLDIERTYPE const& s, OBJECTTYPE const& o)
{
	if (!(s.uiStatusFlags & SOLDIER_PC)) return;
	KEY& k = KeyTable[o.ubKeyID];
	if (k.usDateFound != 0) return;
	k.usDateFound   = GetWorldDay();
	k.usSectorFound = s.sSector.AsByte();
}


BOOLEAN PlaceObject( SOLDIERTYPE * pSoldier, INT8 bPos, OBJECTTYPE * pObj )
{
	// returns object to have in hand after placement... same as original in the
	// case of error

	UINT8 ubNumberToDrop, ubLoop;
	OBJECTTYPE *pInSlot;
	BOOLEAN fObjectWasRobotRemote = FALSE;

	if ( pObj->usItem == ROBOT_REMOTE_CONTROL )
	{
		fObjectWasRobotRemote = TRUE;
	}

	if ( !CanItemFitInPosition( pSoldier, pObj, bPos, TRUE ) )
	{
		return( FALSE );
	}

	// If the position is either head slot, then the item must be IC_FACE (checked in
	// CanItemFitInPosition).
	if ( bPos == HEAD1POS )
	{
		if ( !CompatibleFaceItem( pObj->usItem, pSoldier->inv[ HEAD2POS ].usItem ) )
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(g_langRes->Message[STR_CANT_USE_TWO_ITEMS],
					GCM->getItem(pObj->usItem)->getName(), GCM->getItem(pSoldier->inv[HEAD2POS].usItem)->getName()));
			return( FALSE );
		}
	}
	else if ( bPos == HEAD2POS )
	{
		if ( !CompatibleFaceItem( pObj->usItem, pSoldier->inv[ HEAD1POS ].usItem ) )
		{
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_UI_FEEDBACK, st_format_printf(g_langRes->Message[STR_CANT_USE_TWO_ITEMS],
					GCM->getItem(pObj->usItem)->getName(), GCM->getItem(pSoldier->inv[HEAD1POS].usItem)->getName()));
			return( FALSE );
		}
	}

	if (GCM->getItem(pObj->usItem)->getItemClass() == IC_KEY) CollectKey(*pSoldier, *pObj);

	int ubSlotLimit = ItemSlotLimit(pObj->usItem, bPos);

	pInSlot = &(pSoldier->inv[bPos]);

	if (pInSlot->ubNumberOfObjects == 0)
	{
		// placement in an empty slot
		ubNumberToDrop = pObj->ubNumberOfObjects;

		if (ubNumberToDrop > std::max(ubSlotLimit, 1))
		{
			// drop as many as possible into pocket
			ubNumberToDrop = std::max(ubSlotLimit, 1);
		}

		// could be wrong type of object for slot... need to check...
		// but assuming it isn't
		*pInSlot = *pObj;

		if (ubNumberToDrop != pObj->ubNumberOfObjects)
		{
			// in the InSlot copy, zero out all the objects we didn't drop
			for (ubLoop = ubNumberToDrop; ubLoop < pObj->ubNumberOfObjects; ubLoop++)
			{
				pInSlot->bStatus[ubLoop] = 0;
			}
		}
		pInSlot->ubNumberOfObjects = ubNumberToDrop;

		// remove a like number of objects from pObj
		RemoveObjs( pObj, ubNumberToDrop );
		if (pObj->ubNumberOfObjects == 0)
		{
			// dropped everything
			if (bPos == HANDPOS && GCM->getItem(pInSlot->usItem)->isTwoHanded())
			{
				// We just performed a successful drop of a two-handed object into the
				// main hand
				if (pSoldier->inv[SECONDHANDPOS].usItem != 0)
				{
					// swap what WAS in the second hand into the cursor
					SwapObjs( pObj, &(pSoldier->inv[SECONDHANDPOS]));
				}
			}
		}
	}
	else
	{
		// replacement/reloading/merging/stacking
		// keys have an additional check for key ID being the same
		if ((pObj->usItem == pInSlot->usItem) && (GCM->getItem(pObj->usItem)->getItemClass() != IC_KEY ||
			pObj->ubKeyID == pInSlot->ubKeyID))
		{
			if (GCM->getItem(pObj->usItem)->getItemClass() == IC_MONEY)
			{

				UINT32 uiMoneyMax = MoneySlotLimit( bPos );

				// always allow money to be combined!
				// IGNORE STATUS!

				if (pInSlot->uiMoneyAmount + pObj->uiMoneyAmount > uiMoneyMax)
				{
					// remove X dollars
					pObj->uiMoneyAmount -= (uiMoneyMax - pInSlot->uiMoneyAmount);
					// set in slot to maximum
					pInSlot->uiMoneyAmount = uiMoneyMax;
				}
				else
				{
					pInSlot->uiMoneyAmount += pObj->uiMoneyAmount;
					DeleteObj( pObj );
				}
			}
			else if ( ubSlotLimit == 1 || (ubSlotLimit == 0 && bPos >= HANDPOS && bPos <= BIGPOCK4POS ) )
			{
				if (pObj->ubNumberOfObjects <= 1)
				{
					// swapping
					SwapObjs( pObj, pInSlot );
				}
				else
				{
					return( DropObjIfThereIsRoom( pSoldier, bPos, pObj ) );
				}
			}
			else if (ubSlotLimit == 0) // trying to drop into a small pocket
			{
				return( DropObjIfThereIsRoom( pSoldier, bPos, pObj ) );
			}
			else
			{
				// stacking
				ubNumberToDrop = ubSlotLimit - pInSlot->ubNumberOfObjects;
				if (ubNumberToDrop > pObj->ubNumberOfObjects)
				{
					ubNumberToDrop = pObj->ubNumberOfObjects;
				}
				StackObjs( pObj, pInSlot, ubNumberToDrop );
			}
		}
		else
		{
			// replacement, unless reloading...
			switch (GCM->getItem(pInSlot->usItem)->getItemClass())
			{
				case IC_GUN:
					if (GCM->getItem(pObj->usItem)->getItemClass() == IC_AMMO)
					{
						if (GCM->getWeapon(pInSlot->usItem)->matches(GCM->getItem(pObj->usItem)->asAmmo()->calibre))
						{
							// reload...
							return( ReloadGun( pSoldier, pInSlot, pObj ) );
						}
						else
						{
							// invalid ammo
							break;
						}
					}
					break;
				case IC_LAUNCHER:
				{
					if ( ValidLaunchable( pObj->usItem, pInSlot->usItem ) )
					{
						// reload...
						return( ReloadGun( pSoldier, pInSlot, pObj ) );
					}
				}
				break;
			}

			if ( (GCM->getItem(pObj->usItem)->isTwoHanded()) && (bPos == HANDPOS) )
			{
				if (pSoldier->inv[SECONDHANDPOS].usItem != 0)
				{
					// both pockets have something in them, so we can't swap
					return( FALSE );
				}
				else
				{
					SwapObjs( pObj, pInSlot );
				}
			}
			else if (pObj->ubNumberOfObjects <= std::max(ubSlotLimit, 1))
			{
				// swapping
				SwapObjs( pObj, pInSlot );
			}
			else
			{
				return( DropObjIfThereIsRoom( pSoldier, bPos, pObj ) );
			}

		}
	}

	// ATE: Put this in to see if we should update the robot, if we were given a controller...
	if ( pSoldier->bTeam == OUR_TEAM && fObjectWasRobotRemote )
	{
		UpdateRobotControllerGivenController( pSoldier );
	}

	EnsureConsistentWeaponMode(pSoldier);
	return( TRUE );
}


static BOOLEAN InternalAutoPlaceObject(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObj, BOOLEAN fNewItem, INT8 bExcludeSlot)
{
	INT8  bSlot;
	UINT8 ubPerSlot;

	// statuses of extra objects would be 0 if the # exceeds the maximum
	Assert( pObj->ubNumberOfObjects <= MAX_OBJECTS_PER_SLOT);

	const ItemModel * pItem = GCM->getItem(pObj->usItem);
	ubPerSlot = pItem->getPerPocket();

	// Overrides to the standard system: put guns in hand, armour on body (if slot empty)
	switch (pItem->getItemClass())
	{
		case IC_GUN:
		case IC_BLADE:
		case IC_LAUNCHER:
		case IC_BOMB:
		case IC_GRENADE:
			if (!(pItem->isTwoHanded()))
			{
				if (pSoldier->inv[HANDPOS].usItem == NONE)
				{
					// put the one-handed weapon in the guy's hand...
					PlaceObject( pSoldier, HANDPOS, pObj );
					SetNewItem( pSoldier, HANDPOS, fNewItem );
					if ( pObj->ubNumberOfObjects == 0 )
					{
						return( TRUE );
					}
				}
				else if ( !(GCM->getItem(pSoldier->inv[HANDPOS].usItem)->isTwoHanded()) && pSoldier->inv[SECONDHANDPOS].usItem == NONE)
				{
					// put the one-handed weapon in the guy's 2nd hand...
					PlaceObject( pSoldier, SECONDHANDPOS, pObj );
					SetNewItem( pSoldier, SECONDHANDPOS, fNewItem );
					if ( pObj->ubNumberOfObjects == 0 )
					{
						return( TRUE );
					}
				}
			}
			// two-handed objects are best handled in the main loop for large objects,
			// which checks the hands first anyhow
			break;

		case IC_ARMOUR:
			switch (Armour[GCM->getItem(pObj->usItem)->getClassIndex()].ubArmourClass)
			{
				case ARMOURCLASS_VEST:
					if (pSoldier->inv[VESTPOS].usItem == NONE)
					{
						// put on the armour!
						PlaceObject( pSoldier, VESTPOS, pObj );
						SetNewItem( pSoldier, VESTPOS, fNewItem );
						if ( pObj->ubNumberOfObjects == 0 )
						{
							return( TRUE );
						}
					}
					break;
				case ARMOURCLASS_LEGGINGS:
					if (pSoldier->inv[LEGPOS].usItem == NONE)
					{
						// put on the armour!
						PlaceObject( pSoldier, LEGPOS, pObj );
						SetNewItem( pSoldier, LEGPOS, fNewItem );
						if ( pObj->ubNumberOfObjects == 0 )
						{
							return( TRUE );
						}
					}
					break;
				case ARMOURCLASS_HELMET:
					if (pSoldier->inv[HELMETPOS].usItem == NONE)
					{
						// put on the armour!
						PlaceObject( pSoldier, HELMETPOS, pObj );
						SetNewItem( pSoldier, HELMETPOS, fNewItem );
						if ( pObj->ubNumberOfObjects == 0 )
						{
							return( TRUE );
						}
					}
					break;
				default:
					break;
			}
			// otherwise stuff it in a slot somewhere
			break;
		case IC_FACE:
			if ( (pSoldier->inv[HEAD1POS].usItem == NOTHING) && CompatibleFaceItem( pObj->usItem, pSoldier->inv[HEAD2POS].usItem ) )
			{
				PlaceObject( pSoldier, HEAD1POS, pObj );
				SetNewItem( pSoldier, HEAD1POS, fNewItem );
				if ( pObj->ubNumberOfObjects == 0 )
				{
					return( TRUE );
				}
			}
			else if ( (pSoldier->inv[HEAD2POS].usItem == NOTHING) && CompatibleFaceItem( pObj->usItem, pSoldier->inv[HEAD1POS].usItem ) )
			{
				PlaceObject( pSoldier, HEAD2POS, pObj );
				SetNewItem( pSoldier, HEAD2POS, fNewItem );
				if ( pObj->ubNumberOfObjects == 0 )
				{
					return( TRUE );
				}
			}
			break;
		default:
			break;
	}

	if (ubPerSlot == 0)
	{
		// Large object; look for an empty hand/large pocket and dump it in there
		// FindObjWithin with 0 will search for empty slots!
		bSlot = HANDPOS;
		while (1)
		{
			bSlot = FindEmptySlotWithin( pSoldier, bSlot, BIGPOCK4POS );
			if (bSlot == ITEM_NOT_FOUND)
			{
				return( FALSE );
			}
			if (bSlot == SECONDHANDPOS)
			{
				if (pSoldier->inv[HANDPOS].usItem != NONE)
				{
					bSlot++;
					continue;
				}
			}
			// this might fail if we're trying to place in HANDPOS,
			// and SECONDHANDPOS is full
			PlaceObject( pSoldier, bSlot, pObj );
			SetNewItem( pSoldier, bSlot, fNewItem );
			if (pObj->ubNumberOfObjects == 0)
			{
				return( TRUE );
			}
			bSlot++;
		}
	}
	else
	{
		// Small items; don't allow stack/dumping for keys right now as that
		// would require a bunch of functions for finding the same object by two values...
		if ( ubPerSlot > 1 || GCM->getItem(pObj->usItem)->getItemClass() == IC_KEY || GCM->getItem(pObj->usItem)->getItemClass() == IC_MONEY )
		{
			// First, look for slots with the same object, and dump into them.
			bSlot = HANDPOS;
			while( 1 )
			{
				bSlot = FindObjWithin( pSoldier, pObj->usItem, bSlot, SMALLPOCK8POS );
				if (bSlot == ITEM_NOT_FOUND)
				{
					break;
				}
				if ( bSlot != bExcludeSlot )
				{
					if ( ( (GCM->getItem(pObj->usItem)->getItemClass() == IC_MONEY) && pSoldier->inv[ bSlot ].uiMoneyAmount < MoneySlotLimit( bSlot ) ) || (GCM->getItem(pObj->usItem)->getItemClass() != IC_MONEY && pSoldier->inv[bSlot].ubNumberOfObjects < ItemSlotLimit( pObj->usItem, bSlot ) ) )
					{
						// NEW: If in SKI, don't auto-place anything into a stackable slot that's currently hatched out!  Such slots
						// will disappear in their entirety if sold/moved, causing anything added through here to vanish also!
						if (guiCurrentScreen != SHOPKEEPER_SCREEN || !ShouldSoldierDisplayHatchOnItem(pSoldier->ubProfile, bSlot))
						{
							PlaceObject( pSoldier, bSlot, pObj );
							SetNewItem( pSoldier, bSlot, fNewItem );
							if (pObj->ubNumberOfObjects == 0)
							{
								return( TRUE );
							}
						}
					}
				}
				bSlot++;
			}
		}
		// Search for empty slots to dump into, starting with small pockets
		bSlot = SMALLPOCK1POS;
		while( 1 )
		{
			bSlot = FindEmptySlotWithin( pSoldier, bSlot, SMALLPOCK8POS );
			if (bSlot == ITEM_NOT_FOUND)
			{
				break;
			}
			PlaceObject( pSoldier, bSlot, pObj );
			SetNewItem( pSoldier, bSlot, fNewItem );
			if (pObj->ubNumberOfObjects == 0)
			{
				return( TRUE );
			}
			bSlot++;
		}
		// now check hands/large pockets
		bSlot = HANDPOS;
		while (1)
		{
			bSlot = FindEmptySlotWithin( pSoldier, bSlot, BIGPOCK4POS );
			if (bSlot == ITEM_NOT_FOUND)
			{
				break;
			}
			PlaceObject( pSoldier, bSlot, pObj );
			SetNewItem( pSoldier, bSlot, fNewItem );
			if (pObj->ubNumberOfObjects == 0)
			{
				return( TRUE );
			}
			bSlot++;
		}
	}
	return( FALSE );
}

BOOLEAN AutoPlaceObject( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN fNewItem )
{
	return( InternalAutoPlaceObject( pSoldier, pObj, fNewItem, NO_SLOT ) );
}

BOOLEAN RemoveObjectFromSlot( SOLDIERTYPE * pSoldier, INT8 bPos, OBJECTTYPE * pObj )
{
	CHECKF( pObj );
	if (pSoldier->inv[bPos].ubNumberOfObjects == 0)
	{
		return( FALSE );
	}
	else
	{
		*pObj = pSoldier->inv[bPos];
		DeleteObj( &(pSoldier->inv[bPos]) );
		return( TRUE );
	}
}

BOOLEAN RemoveKeysFromSlot( SOLDIERTYPE * pSoldier, INT8 bKeyRingPosition, UINT8 ubNumberOfKeys ,OBJECTTYPE * pObj )
{
	CHECKF( pObj );

	if (pSoldier->pKeyRing[bKeyRingPosition].isValid())
	{
		if( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber < ubNumberOfKeys )
		{
			ubNumberOfKeys = pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber;
		}


		UINT8 const ubItems = pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID;
		if( pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber - ubNumberOfKeys > 0 )
		{
			pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber -= ubNumberOfKeys;
		}
		else
		{
			pSoldier->pKeyRing[ bKeyRingPosition ].ubNumber = 0;
			pSoldier->pKeyRing[ bKeyRingPosition ].ubKeyID = INVALID_KEY_NUMBER;
		}

		CreateKeyObject(pObj, ubNumberOfKeys, ubItems);
		return TRUE;
	}
	return FALSE;
}


// return number added
UINT8 AddKeysToSlot(SOLDIERTYPE& s, INT8 const key_ring_pos, OBJECTTYPE const& key)
{
	// redundant but what the hey
	CollectKey(s, key);

	KEY_ON_RING& keyring = s.pKeyRing[key_ring_pos];
	if (keyring.ubNumber == 0) keyring.ubKeyID = key.ubKeyID;
	// Only take what we can
	UINT8 const n_added = std::min(int(key.ubNumberOfObjects), GCM->getItem(key.usItem)->getPerPocket() - keyring.ubNumber);
	keyring.ubNumber += n_added;
	return n_added;
}


void SwapKeysToSlot(SOLDIERTYPE& s, INT8 const key_ring_pos, OBJECTTYPE& key)
{
	KEY_ON_RING& keyring = s.pKeyRing[key_ring_pos];
	UINT8 const  n_keys  = keyring.ubNumber;
	UINT8 const  key_id  = keyring.ubKeyID;

	keyring.ubNumber = key.ubNumberOfObjects;
	keyring.ubKeyID  = key.ubKeyID;

	CreateKeyObject(&key, n_keys, key_id);
}


void CreateKeyObject(OBJECTTYPE* const pObj, UINT8 const ubNumberOfKeys, UINT8 const ubKeyID)
{
	auto keyId = LockTable[ubKeyID].usKeyItem;
	auto item = GCM->getKeyItemForKeyId(keyId);
	if (item == NULL) {
		throw std::runtime_error(ST::format("Could not find key item for key id `{}` when creating key object", keyId).to_std_string());
	}
	CreateItems(item->getItemIndex(), 100, ubNumberOfKeys, pObj);
	pObj->ubKeyID = ubKeyID;
}


void AllocateObject(OBJECTTYPE** const pObj)
{
	// create a key object
	*pObj = new OBJECTTYPE{};
}


BOOLEAN DeleteKeyObject( OBJECTTYPE * pObj )
{
	if (pObj == NULL) return FALSE;

	// free up space
	delete pObj;

	return( TRUE );
}


UINT16 TotalPoints(const OBJECTTYPE* const pObj)
{
	UINT16 usPoints = 0;
	UINT8  ubLoop;

	for (ubLoop = 0; ubLoop < pObj->ubNumberOfObjects; ubLoop++)
	{
		usPoints += pObj->bStatus[ubLoop];
	}
	return( usPoints );
}


UINT16 UseKitPoints(OBJECTTYPE& o, UINT16 const original_points, SOLDIERTYPE const& s)
{
	// Start consuming from the last kit in, so we end up with fewer fuller kits
	// rather than lots of half-empty ones.
	UINT16 points = original_points;
	UINT8& n      = o.ubNumberOfObjects;
	for (INT8 i = n - 1; i >= 0; --i)
	{
		INT8& status = o.bStatus[i];
		if (points < (UINT16)status)
		{
			status -= (INT8)points;
			return original_points;
		}
		else
		{
			// Consume this kit totally
			points -= status;
			status  = 0;
			--n;
		}
	}

	// Pocket/hand emptied, update inventory, then update panel
	Assert(n == 0);
	DeleteObj(&o);
	DirtyMercPanelInterface(&s, DIRTYLEVEL2);
	return original_points - points;
}

UINT16 DefaultMagazine(UINT16 const gun)
{
	if (!(GCM->getItem(gun)->isGun()))
	{
		throw std::logic_error("Tried to get default ammo for item which is not a gun");
	}

	const WeaponModel * w = GCM->getWeapon(gun);
	const std::vector<const MagazineModel*>& magazines = GCM->getMagazines();
	for (const MagazineModel* mag : magazines)
	{
		if (mag->calibre->index == NOAMMO)      break;
		if (mag->dontUseAsDefaultMagazine) continue;
		if (mag->calibre->index != w->calibre->index) continue;
		if (mag->capacity != w->ubMagSize) continue;
		return mag->getItemIndex();
	}

	throw std::logic_error("Found no default ammo for gun");
}


UINT16 FindReplacementMagazine(const CalibreModel * calibre, UINT8 const mag_size, UINT8 const ammo_type)
{
	UINT16 default_mag = NOTHING;
	const std::vector<const MagazineModel*>& magazines = GCM->getMagazines();
	for (const MagazineModel* mag : magazines)
	{
		if (mag->calibre->index == NOAMMO)   break;
		if (mag->calibre->index != calibre->index)  continue;
		if (mag->capacity != mag_size) continue;

		if (mag->ammoType->index == ammo_type) return mag->getItemIndex();

		if (default_mag == NOTHING)
		{ // Store this one to use if all else fails
			default_mag = mag->getItemIndex();
		}
	}
	return default_mag;
}


UINT16 FindReplacementMagazineIfNecessary(const WeaponModel *old_gun, UINT16 const old_ammo_id, const WeaponModel *new_gun)
{
	const MagazineModel * old_mag = GCM->getMagazineByItemIndex(old_ammo_id);
	if (old_mag->calibre->index != old_gun->calibre->index) return NOTHING;
	if (old_mag->capacity != old_gun->ubMagSize) return NOTHING;
	return FindReplacementMagazine(new_gun->calibre, new_gun->ubMagSize, old_mag->ammoType->index);
}


UINT16 RandomMagazine( UINT16 usItem, UINT8 ubPercentStandard )
{
	std::vector<const MagazineModel*> possibleMags;
	UINT8 ubMagChosen;

	if (!(GCM->getItem(usItem)->isGun()))
	{
		return( 0 );
	}

	const WeaponModel *pWeapon = GCM->getWeapon(usItem);

	// find & store all possible mag types that fit this gun
	const std::vector<const MagazineModel*>& magazines = GCM->getMagazines();
	for (const MagazineModel* mag : magazines)
	{
		if (pWeapon->matches(mag))
		{
			possibleMags.push_back(mag);
		}
	}

	// no matches?
	if (possibleMags.size() == 0)
	{
		return( 0 );
	}
	else if (possibleMags.size() == 1)
	{
		// only one match?
		// use that, no choice
		return possibleMags[0]->getItemIndex();
	}
	else	// multiple choices
	{
		// Pick one at random, using supplied probability to pick the default
		if (Random(100) < ubPercentStandard)
		{
			ubMagChosen = 0;
		}
		else
		{
			// pick a non-standard type instead
			ubMagChosen = ( UINT8 ) (1 + Random(( UINT32 ) ( possibleMags.size() - 1 )));
		}

		return possibleMags[ubMagChosen]->getItemIndex();
	}
}


static void CreateGun(UINT16 usItem, INT8 bStatus, OBJECTTYPE* pObj)
{
	UINT16 usAmmo;

	pObj->usItem = usItem;
	pObj->ubNumberOfObjects = 1;
	pObj->bGunStatus = bStatus;
	pObj->ubImprintID = NO_PROFILE;
	pObj->ubWeight = CalculateObjectWeight( pObj );

	if (GCM->getWeapon( usItem )->ubWeaponClass == MONSTERCLASS)
	{
		pObj->ubGunShotsLeft = GCM->getWeapon( usItem )->ubMagSize;
		pObj->ubGunAmmoType = AMMO_MONSTER;
	}
	else if ( EXPLOSIVE_GUN( usItem ) )
	{
		if ( usItem == ROCKET_LAUNCHER )
		{
			pObj->ubGunShotsLeft = 1;
		}
		else
		{
			// cannon
			pObj->ubGunShotsLeft = 0;
		}
		pObj->bGunAmmoStatus = 100;
		pObj->ubGunAmmoType = 0;
	}
	else
	{
		usAmmo = DefaultMagazine( usItem );
		pObj->usGunAmmoItem = usAmmo;
		pObj->ubGunAmmoType = GCM->getItem(usAmmo)->asAmmo()->ammoType->index;
		pObj->bGunAmmoStatus = 100;
		pObj->ubGunShotsLeft = GCM->getItem(usAmmo)->asAmmo()->capacity;
	}
}


static void CreateMagazine(UINT16 usItem, OBJECTTYPE* pObj)
{
	pObj->usItem = usItem;
	pObj->ubNumberOfObjects = 1;
	pObj->ubShotsLeft[0] = GCM->getItem(usItem)->asAmmo()->capacity;
	pObj->ubWeight = CalculateObjectWeight( pObj );
}


void CreateItem(UINT16 const usItem, INT8 const bStatus, OBJECTTYPE* const pObj)
{
	*pObj = OBJECTTYPE{};
	if (usItem >= MAXITEMS)
	{
		throw std::logic_error("Tried to create item with invalid ID");
	}

	if (GCM->getItem(usItem)->getItemClass() == IC_GUN)
	{
		CreateGun( usItem, bStatus, pObj );
	}
	else if (GCM->getItem(usItem)->getItemClass() == IC_AMMO)
	{
		CreateMagazine(usItem, pObj);
	}
	else
	{
		pObj->usItem = usItem;
		pObj->ubNumberOfObjects = 1;
		if (usItem == MONEY)
		{
			// special case... always set status to 100 when creating
			// and use status value to determine amount!
			pObj->bStatus[0] = 100;
			pObj->uiMoneyAmount = bStatus * 50;
		}
		else
		{
			pObj->bStatus[0] = bStatus;
		}
		pObj->ubWeight = CalculateObjectWeight( pObj );
	}

	if (GCM->getItem(usItem)->getFlags() & ITEM_DEFAULT_UNDROPPABLE)
	{
		pObj->fFlags |= OBJECT_UNDROPPABLE;
	}
}


void CreateItems(UINT16 const usItem, INT8 const bStatus, UINT8 ubNumber, OBJECTTYPE* const pObj)
{
	UINT8 ubLoop;

	// can't create any more than this, the loop for setting the bStatus[] of others will overwrite memory!
	Assert( ubNumber <= MAX_OBJECTS_PER_SLOT );

	// ARM: to avoid whacking memory once Assertions are removed...  Items will be lost in this situation!
	if ( ubNumber > MAX_OBJECTS_PER_SLOT )
	{
		ubNumber = MAX_OBJECTS_PER_SLOT;
	}

	CreateItem(usItem, bStatus, pObj);
	for (ubLoop = 1; ubLoop < ubNumber; ubLoop++)
	{
		// we reference status[0] here because the status value might actually be a
		// # of rounds of ammo, in which case the value won't be the bStatus value
		// passed in.
		pObj->bStatus[ubLoop] = pObj->bStatus[0];
	}
	pObj->ubNumberOfObjects = ubNumber;
}


void CreateMoney(UINT32 const uiMoney, OBJECTTYPE* const pObj)
{
	CreateItem(MONEY, 100, pObj);
	pObj->uiMoneyAmount = uiMoney;
}


BOOLEAN ArmBomb( OBJECTTYPE * pObj, INT8 bSetting )
{
	BOOLEAN fRemote = FALSE;
	BOOLEAN fPressure = FALSE;
	BOOLEAN fTimed = FALSE;
	BOOLEAN	fSwitch = FALSE;

	if (pObj->usItem == ACTION_ITEM)
	{
		switch( pObj->bActionValue )
		{
			case ACTION_ITEM_SMALL_PIT:
			case ACTION_ITEM_LARGE_PIT:
				fPressure = TRUE;
				break;
			default:
				fRemote = TRUE;
				break;

		}
	}
	else if ( FindAttachment( pObj, DETONATOR ) != ITEM_NOT_FOUND )
	{
		fTimed = TRUE;
	}
	else if ( (FindAttachment( pObj, REMDETONATOR ) != ITEM_NOT_FOUND) || (pObj->usItem == ACTION_ITEM) )
	{
		fRemote = TRUE;
	}
	else if ( pObj->usItem == MINE || pObj->usItem == TRIP_FLARE || pObj->usItem == TRIP_KLAXON || pObj->usItem == ACTION_ITEM )
	{
		fPressure = TRUE;
	}
	else if ( pObj->usItem == SWITCH )
	{
		// this makes a remote detonator into a pressure-sensitive trigger
		if ( bSetting == PANIC_FREQUENCY )
		{
			// panic trigger is only activated by expending APs, not by
			// stepping on it... so don't define a detonator type
			fSwitch = TRUE;
		}
		else
		{
			fPressure = TRUE;
		}
	}
	else
	{
		// no sorta bomb at all!
		return( FALSE );
	}

	if (fRemote)
	{
		pObj->bDetonatorType = BOMB_REMOTE;
		pObj->bFrequency = bSetting;
	}
	else if (fPressure)
	{
		pObj->bDetonatorType = BOMB_PRESSURE;
		pObj->bFrequency = 0;
	}
	else if (fTimed)
	{
		pObj->bDetonatorType = BOMB_TIMED;
		// In realtime the player could choose to put down a bomb right before a turn expires, SO
		// add 1 to the setting in RT
		pObj->bDelay = bSetting;
		if (!(gTacticalStatus.uiFlags & INCOMBAT))
		{
			pObj->bDelay++;
		}

	}
	else if (fSwitch)
	{
		pObj->bDetonatorType = BOMB_SWITCH;
		pObj->bFrequency = bSetting;
	}
	else
	{
		return( FALSE );
	}

	pObj->fFlags |= OBJECT_ARMED_BOMB;
	pObj->usBombItem = pObj->usItem;
	return( TRUE );
}


static void RenumberAttachments(OBJECTTYPE* pObj)
{
	// loop through attachment positions and make sure we don't have any empty
	// attachment slots before filled ones
	INT8 bAttachPos;
	INT8 bFirstSpace;
	BOOLEAN fDone = FALSE;

	while (!fDone)
	{
		bFirstSpace = -1;
		for (bAttachPos = 0; bAttachPos < MAX_ATTACHMENTS; bAttachPos++)
		{
			if (pObj->usAttachItem[ bAttachPos ] == NOTHING)
			{
				if (bFirstSpace == -1)
				{
					bFirstSpace = bAttachPos;
				}
			}
			else
			{
				if (bFirstSpace != -1)
				{
					// move the attachment!
					pObj->usAttachItem[ bFirstSpace ] = pObj->usAttachItem[ bAttachPos ];
					pObj->bAttachStatus[ bFirstSpace ] = pObj->bAttachStatus[ bAttachPos ];
					pObj->usAttachItem[ bAttachPos ] = NOTHING;
					pObj->bAttachStatus[ bAttachPos ] = 0;
					// restart loop at beginning, or quit if we reached the end of the
					// attachments
					break;
				}
			}
		}
		if (bAttachPos == MAX_ATTACHMENTS)
		{
			// done!!
			fDone = TRUE;
		}
	}

}

BOOLEAN RemoveAttachment( OBJECTTYPE * pObj, INT8 bAttachPos, OBJECTTYPE * pNewObj )
{
	INT8 bGrenade;

	CHECKF( pObj );

	if (bAttachPos < 0 || bAttachPos >= MAX_ATTACHMENTS)
	{
		return( FALSE );
	}
	if (pObj->usAttachItem[bAttachPos] == NOTHING )
	{
		return( FALSE );
	}

	if ( GCM->getItem(pObj->usAttachItem[bAttachPos])->getFlags() & ITEM_INSEPARABLE )
	{
		return( FALSE );
	}

	// if pNewObj is passed in NULL, then we just delete the attachment
	if (pNewObj != NULL)
	{
		CreateItem( pObj->usAttachItem[bAttachPos], pObj->bAttachStatus[bAttachPos], pNewObj );
	}

	pObj->usAttachItem[bAttachPos] = NOTHING;
	pObj->bAttachStatus[bAttachPos] = 0;

	if (pNewObj && pNewObj->usItem == UNDER_GLAUNCHER)
	{
		// look for any grenade; if it exists, we must make it an
		// attachment of the grenade launcher
		bGrenade = FindAttachmentByClass( pObj, IC_GRENADE );
		if (bGrenade != ITEM_NOT_FOUND)
		{
			pNewObj->usAttachItem[0] = pObj->usAttachItem[bGrenade];
			pNewObj->bAttachStatus[0] = pObj->bAttachStatus[bGrenade];
			pObj->usAttachItem[bGrenade] = NOTHING;
			pObj->bAttachStatus[bGrenade] = 0;
			pNewObj->ubWeight = CalculateObjectWeight( pNewObj );
		}
	}

	RenumberAttachments( pObj );

	pObj->ubWeight = CalculateObjectWeight( pObj );
	return( TRUE );
}


void SetNewItem(SOLDIERTYPE* pSoldier, UINT8 ubInvPos, BOOLEAN fNewItem)
{
	if( fNewItem )
	{
		pSoldier->bNewItemCount[ ubInvPos ] = -1;
		pSoldier->bNewItemCycleCount[ ubInvPos ] = NEW_ITEM_CYCLE_COUNT;
		pSoldier->fCheckForNewlyAddedItems = TRUE;
	}
}


BOOLEAN PlaceObjectInSoldierProfile( UINT8 ubProfile, OBJECTTYPE *pObject )
{
	INT8    bLoop, bLoop2;
	UINT16  usItem;
	INT8    bStatus;
	BOOLEAN fReturnVal = FALSE;

	usItem	= pObject->usItem;
	bStatus = pObject->bStatus[0];
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubProfile);

	if ( GCM->getItem(usItem)->getItemClass() == IC_MONEY && gMercProfiles[ ubProfile ].uiMoney > 0 )
	{
		gMercProfiles[ ubProfile ].uiMoney += pObject->uiMoneyAmount;
		SetMoneyInSoldierProfile( ubProfile, gMercProfiles[ ubProfile ].uiMoney );
		return( TRUE );
	}

	for (bLoop = BIGPOCK1POS; bLoop < SMALLPOCK8POS; bLoop++)
	{
		if ( gMercProfiles[ ubProfile ].bInvNumber[ bLoop ] == 0 && (pSoldier == NULL || pSoldier->inv[ bLoop ].usItem == NOTHING ) )
		{

			// CJC: Deal with money by putting money into # stored in profile
			if ( GCM->getItem(usItem)->getItemClass() == IC_MONEY )
			{
				gMercProfiles[ ubProfile ].uiMoney += pObject->uiMoneyAmount;
				// change any gold/silver to money
				usItem = MONEY;
			}
			else
			{
				gMercProfiles[ ubProfile ].inv[ bLoop ] = usItem;
				gMercProfiles[ ubProfile ].bInvStatus[ bLoop ] = bStatus;
				gMercProfiles[ ubProfile ].bInvNumber[ bLoop ] = pObject->ubNumberOfObjects;
			}

			fReturnVal = TRUE;
			break;
		}
	}

	if ( fReturnVal )
	{
		// ATE: Manage soldier pointer as well....
		// Do we have a valid profile?
		if ( pSoldier != NULL )
		{
			// OK, place in soldier...
			if ( usItem == MONEY )
			{
				CreateMoney( gMercProfiles[ ubProfile ].uiMoney, &(pSoldier->inv[ bLoop ] ) );
			}
			else
			{
				if ( pSoldier->ubProfile == MADLAB )
				{
					// remove attachments and drop them
					OBJECTTYPE			Attachment;

					for ( bLoop2 = MAX_ATTACHMENTS - 1; bLoop2 >= 0; bLoop2-- )
					{
						// remove also checks for existence attachment
						if (RemoveAttachment(pObject, bLoop2, &Attachment))
						{
							// drop it in Madlab's tile
							AddItemToPool(pSoldier->sGridNo, &Attachment, VISIBLE, 0, 0, 0);
						}
					}
				}

				CreateItem( usItem, bStatus, &(pSoldier->inv[ bLoop ] ) );
			}
		}
	}

	return( fReturnVal );
}


static void RemoveInvObject(SOLDIERTYPE* pSoldier, UINT16 usItem);


BOOLEAN RemoveObjectFromSoldierProfile( UINT8 ubProfile, UINT16 usItem )
{
	INT8 bLoop;
	BOOLEAN	fReturnVal = FALSE;

	if ( usItem == NOTHING )
	{
		return( TRUE );
	}

	for (bLoop = 0; bLoop < 19; bLoop++)
	{
		if ( gMercProfiles[ ubProfile ].inv[ bLoop ] == usItem )
		{
			gMercProfiles[ ubProfile ].inv[ bLoop ] = NOTHING;
			gMercProfiles[ ubProfile ].bInvStatus[ bLoop ] = 0;
			gMercProfiles[ ubProfile ].bInvNumber[ bLoop ] = 0;

			fReturnVal = TRUE;
			break;
		}
	}

	// ATE: Manage soldier pointer as well....
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubProfile);

	// Do we have a valid profile?
	if ( pSoldier != NULL )
	{
		// Remove item...
		RemoveInvObject( pSoldier, usItem );
	}

	return( fReturnVal );
}


void SetMoneyInSoldierProfile( UINT8 ubProfile, UINT32 uiMoney )
{
	OBJECTTYPE Object;
	BOOLEAN fRet;

	// remove all money from soldier
	do
	{
		fRet = RemoveObjectFromSoldierProfile( ubProfile, MONEY );
	}
	while (fRet);

	gMercProfiles[ ubProfile ].uiMoney = 0;

	if (uiMoney > 0)
	{
		// now add the amount specified
		CreateMoney( uiMoney, &Object );
		PlaceObjectInSoldierProfile( ubProfile, &Object );
	}
}


INT8 FindObjectInSoldierProfile(MERCPROFILESTRUCT const& p, UINT16 const item_id)
{
	for (INT8 i = 0; i != NUM_INV_SLOTS; ++i)
	{
		if (p.bInvNumber[i] == 0) continue;
		if (p.inv[i] != item_id)  continue;
		return i;
	}
	return NO_SLOT;
}


static void RemoveInvObject(SOLDIERTYPE* pSoldier, UINT16 usItem)
{
	INT8 bInvPos;

	// find object
	bInvPos = FindObj( pSoldier, usItem );
	if (bInvPos != NO_SLOT)
	{

		// Erase!
		pSoldier->inv[ bInvPos ] = OBJECTTYPE{};

		//Dirty!
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}

}


static INT8 CheckItemForDamage(UINT16 usItem, INT32 iMaxDamage)
{
	INT8 bDamage = 0;

	// if the item is protective armour, reduce the amount of damage
	// by its armour value
	if (GCM->getItem(usItem)->getItemClass() == IC_ARMOUR)
	{
		iMaxDamage -= (iMaxDamage * Armour[GCM->getItem(usItem)->getClassIndex()].ubProtection) / 100;
	}
	// metal items are tough and will be damaged less
	if (GCM->getItem(usItem)->getFlags() & ITEM_METAL)
	{
		iMaxDamage /= 2;
	}
	else if ( usItem == BLOODCAT_PELT )
	{
		iMaxDamage *= 2;
	}
	if (iMaxDamage > 0)
	{
		bDamage = (INT8) PreRandom( iMaxDamage );
	}
	return( bDamage );
}


static BOOLEAN CheckForChainReaction(UINT16 usItem, INT8 bStatus, INT8 bDamage, BOOLEAN fOnGround)
{
	INT32 iChance;

	iChance = Explosive[GCM->getItem(usItem)->getClassIndex()].ubVolatility;
	if (iChance > 0)
	{

		// Scale the base chance by the damage caused to the item
		// (bigger the shock, bigger chance) and the condition of
		// the item after being hit!
		if (fOnGround)
		{
			// improve chance to make it practical to blow up explosives on the ground
			iChance = 50 + (iChance - 1) * 10;
		}

		iChance = iChance * ( 100 + ( (100 - bStatus) + bDamage ) / 2 ) / 100;
		if ((INT32) PreRandom( 100 ) < iChance)
		{
			return( TRUE );
		}
	}
	return( FALSE );
}


static BOOLEAN DamageItem(OBJECTTYPE* pObject, INT32 iDamage, BOOLEAN fOnGround)
{
	INT8 bLoop;
	INT8 bDamage;

	if ( (GCM->getItem(pObject->usItem)->getFlags() & ITEM_DAMAGEABLE || GCM->getItem(pObject->usItem)->getItemClass() == IC_AMMO) && pObject->ubNumberOfObjects > 0)
	{

		for (bLoop = 0; bLoop < pObject->ubNumberOfObjects; bLoop++)
		{
			// if the status of the item is negative then it's trapped/jammed;
			// leave it alone
			if (pObject->usItem != NOTHING && pObject->bStatus[bLoop] > 0)
			{
				bDamage = CheckItemForDamage( pObject->usItem, iDamage );
				switch( pObject->usItem )
				{
					case JAR_CREATURE_BLOOD:
					case JAR:
					case JAR_HUMAN_BLOOD:
					case JAR_ELIXIR:
						if ( PreRandom( bDamage ) > 5 )
						{
							// smash!
							bDamage = pObject->bStatus[ bLoop ];
						}
						break;
					default:
						break;
				}
				if ( GCM->getItem(pObject->usItem)->getItemClass() == IC_AMMO  )
				{
					if ( PreRandom( 100 ) < (UINT32) bDamage )
					{
						// destroy clip completely
						pObject->bStatus[ bLoop ] = 1;
					}
				}
				else
				{
					pObject->bStatus[bLoop] -= bDamage;
					if (pObject->bStatus[bLoop] < 1)
					{
						pObject->bStatus[bLoop] = 1;
					}
				}
				// I don't think we increase viewrange based on items any more
				// FUN STUFF!  Check for explosives going off as a result!
				if (GCM->getItem(pObject->usItem)->isExplosive())
				{
					if (CheckForChainReaction( pObject->usItem, pObject->bStatus[bLoop], bDamage, fOnGround ))
					{
						return( TRUE );
					}
				}

				// remove item from index AFTER checking explosions because need item data for explosion!
				if ( pObject->bStatus[bLoop] == 1 )
				{
					if ( pObject->ubNumberOfObjects > 1 )
					{
						RemoveObjFrom( pObject, bLoop );
						// since an item was just removed, the items above the current were all shifted down one;
						// to process them properly, we have to back up 1 in the counter
						bLoop = bLoop - 1;
					}
				}
			}
		}

		for (bLoop = 0; bLoop < MAX_ATTACHMENTS; bLoop++)
		{
			if (pObject->usAttachItem[bLoop] != NOTHING && pObject->bAttachStatus[bLoop] > 0)
			{
				pObject->bAttachStatus[bLoop] -= CheckItemForDamage( pObject->usAttachItem[bLoop], iDamage );
				if (pObject->bAttachStatus[bLoop] < 1)
				{
					pObject->bAttachStatus[bLoop] = 1;
				}
			}
		}
	}

	return( FALSE );
}

void CheckEquipmentForDamage( SOLDIERTYPE *pSoldier, INT32 iDamage )
{
	BOOLEAN fBlowsUp;
	UINT8   ubNumberOfObjects;

	if ( TANK( pSoldier ) )
	{
		return;
	}

	FOR_EACH_SOLDIER_INV_SLOT(i, *pSoldier)
	{
		ubNumberOfObjects = i->ubNumberOfObjects;
		fBlowsUp = DamageItem(i, iDamage, FALSE);
		if (fBlowsUp)
		{
			// blow it up!
			SOLDIERTYPE* const owner = (gTacticalStatus.ubAttackBusyCount ? pSoldier->attacker : pSoldier);
			IgniteExplosion(owner, 0, pSoldier->sGridNo, i->usItem, pSoldier->bLevel);

			// Remove item!
			DeleteObj(i);

			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
		}
		else if (ubNumberOfObjects != i->ubNumberOfObjects)
		{
			DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
		}
	}
}

void CheckEquipmentForFragileItemDamage( SOLDIERTYPE *pSoldier, INT32 iDamage )
{
	// glass jars etc can be damaged by falling over
	UINT8   ubNumberOfObjects;
	BOOLEAN fPlayedGlassBreak = FALSE;

	FOR_EACH_SOLDIER_INV_SLOT(i, *pSoldier)
	{
		switch (i->usItem)
		{
			case JAR_CREATURE_BLOOD:
			case JAR:
			case JAR_HUMAN_BLOOD:
			case JAR_ELIXIR:
				ubNumberOfObjects = i->ubNumberOfObjects;
				DamageItem(i, iDamage, FALSE);
				if (!fPlayedGlassBreak && ubNumberOfObjects != i->ubNumberOfObjects)
				{
					PlayLocationJA2Sample(pSoldier->sGridNo, GLASS_CRACK, MIDVOLUME, 1);
					fPlayedGlassBreak = TRUE;
					// only dirty once
					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
				}
				break;
			default:
				break;
		}
	}
}


BOOLEAN DamageItemOnGround(OBJECTTYPE* const pObject, const INT16 sGridNo, const INT8 bLevel, const INT32 iDamage, SOLDIERTYPE* const owner)
{
	BOOLEAN fBlowsUp;

	fBlowsUp = DamageItem( pObject, iDamage, TRUE );
	if ( fBlowsUp )
	{
		// OK, Ignite this explosion!
		IgniteExplosion(owner, 0, sGridNo, pObject->usItem, bLevel);

		// Remove item!
		return( TRUE );
	}
	else if ( (pObject->ubNumberOfObjects < 2) && (pObject->bStatus[0] < USABLE) )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


BOOLEAN IsMedicalKitItem(const OBJECTTYPE* pObject)
{
	return pObject->usItem == MEDICKIT;
}


void SwapHandItems( SOLDIERTYPE * pSoldier )
{
	BOOLEAN fOk;

	CHECKV( pSoldier );
	if (pSoldier->inv[HANDPOS].usItem == NOTHING || pSoldier->inv[SECONDHANDPOS].usItem == NOTHING)
	{
		// whatever is in the second hand can be swapped to the main hand!
		SwapObjs( &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[SECONDHANDPOS]) );
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}
	else
	{
		if ( GCM->getItem( pSoldier->inv[SECONDHANDPOS].usItem )->isTwoHanded() )
		{
			// must move the item in the main hand elsewhere in the inventory
			fOk = InternalAutoPlaceObject( pSoldier, &(pSoldier->inv[HANDPOS]), FALSE, HANDPOS );
			if (!fOk)
			{
				return;
			}
			// the main hand is now empty so a swap is going to work...
		}
		SwapObjs( &(pSoldier->inv[HANDPOS]), &(pSoldier->inv[SECONDHANDPOS]) );
		DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
	}
}


void WaterDamage(SOLDIERTYPE& s)
{
	// damage guy's equipment and camouflage due to water
	INT8   bDieSize;
	UINT32 uiRoll;

	if (s.bOverTerrainType == DEEP_WATER)
	{
		FOR_EACH_SOLDIER_INV_SLOT(i, s)
		{
			// if there's an item here that can get water damaged...
			if (i->usItem && GCM->getItem(i->usItem)->getFlags() & ITEM_WATER_DAMAGES)
			{
				// roll the 'ol 100-sided dice
				uiRoll = PreRandom(100);

				// 10% chance of getting damage!
				if (uiRoll < 10)
				{
					// lose between 1 and 10 status points each time
					// but don't let anything drop lower than 1%
					i->bStatus[0] = std::max<INT8>(1, i->bStatus[0] - 10 + uiRoll);
				}
			}
		}
	}
	if (s.bCamo > 0 && !HAS_SKILL_TRAIT(&s, CAMOUFLAGED))
	{
		// reduce camouflage by 2% per tile of deep water
		// and 1% for medium water
		if (s.bOverTerrainType == DEEP_WATER )
		{
			s.bCamo = std::max(0, s.bCamo - 2);
		}
		else
		{
			s.bCamo = std::max(0, s.bCamo - 1);
		}
		if (s.bCamo == 0)
		{
			// Reload palettes....
			if (s.bInSector) CreateSoldierPalettes(s);
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_CAMO_WASHED_OFF], s.name));
		}
	}
	if (s.bTeam == OUR_TEAM && s.bMonsterSmell > 0)
	{
		if (s.bOverTerrainType == DEEP_WATER)
		{
			bDieSize = 10;
		}
		else
		{
			bDieSize = 20;
		}
		if ( Random( bDieSize ) == 0 )
		{
			--s.bMonsterSmell;
		}
	}

	DirtyMercPanelInterface(&s, DIRTYLEVEL2);
}


BOOLEAN ApplyCamo(SOLDIERTYPE* const pSoldier, OBJECTTYPE* const pObj, BOOLEAN* const pfGoodAPs)
{
	(*pfGoodAPs) = TRUE;

	if (pObj->usItem != CAMOUFLAGEKIT)
	{
		return( FALSE );
	}

	if (!EnoughPoints( pSoldier, AP_CAMOFLAGE, 0, TRUE ) )
	{
		(*pfGoodAPs) = FALSE;
		return( TRUE );
	}

	int usTotalKitPoints = TotalPoints(pObj);
	if (usTotalKitPoints == 0)
	{
		// HUH???
		return( FALSE );
	}

	if (pSoldier->bCamo == 100)
	{
		// nothing more to add
		return( FALSE );
	}

	// points are used up at a rate of 50% kit = 100% camo on guy
	// add 1 to round off
	int bPointsToUse = (100 - pSoldier->bCamo + 1 ) / 2;
	bPointsToUse = std::min(bPointsToUse, usTotalKitPoints);
	pSoldier->bCamo = std::min(pSoldier->bCamo + bPointsToUse * 2, 100);

	UseKitPoints(*pObj, bPointsToUse, *pSoldier);

	DeductPoints( pSoldier, AP_CAMOFLAGE, 0 );

	// Reload palettes....
	if ( pSoldier->bInSector )
	{
		CreateSoldierPalettes(*pSoldier);
	}

	return( TRUE );
}

BOOLEAN ApplyCanteen( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs )
{
	(*pfGoodAPs) = TRUE;

	if (pObj->usItem != CANTEEN)
	{
		return( FALSE );
	}

	int usTotalKitPoints = TotalPoints(pObj);
	if (usTotalKitPoints == 0)
	{
		// HUH???
		return( FALSE );
	}

	if (!EnoughPoints( pSoldier, AP_DRINK, 0, TRUE ) )
	{
		(*pfGoodAPs) = FALSE;
		return( TRUE );
	}

	if ( pSoldier->bTeam == OUR_TEAM )
	{
		if ( gMercProfiles[ pSoldier->ubProfile ].bSex == MALE )
		{
			PlayJA2Sample(DRINK_CANTEEN_MALE, MIDVOLUME, 1, MIDDLEPAN);
		}
		else
		{
			PlayJA2Sample(DRINK_CANTEEN_FEMALE, MIDVOLUME, 1, MIDDLEPAN);
		}
	}

	int sPointsToUse = std::min(20, usTotalKitPoints);

	// CJC Feb 9.  Canteens don't seem effective enough, so doubled return from them
	DeductPoints( pSoldier, AP_DRINK, (INT16) (2 * sPointsToUse * -(100 - pSoldier->bBreath) ) );

	UseKitPoints(*pObj, sPointsToUse, *pSoldier);

	return( TRUE );
}

#define MAX_HUMAN_CREATURE_SMELL (NORMAL_HUMAN_SMELL_STRENGTH - 1)

BOOLEAN ApplyElixir( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN *pfGoodAPs )
{
	(*pfGoodAPs) = TRUE;

	if (pObj->usItem != JAR_ELIXIR )
	{
		return( FALSE );
	}

	int usTotalKitPoints = TotalPoints(pObj);
	if (usTotalKitPoints == 0)
	{
		// HUH???
		return( FALSE );
	}

	if (!EnoughPoints( pSoldier, AP_CAMOFLAGE, 0, TRUE ) )
	{
		(*pfGoodAPs) = FALSE;
		return( TRUE );
	}

	DeductPoints( pSoldier, AP_CAMOFLAGE, 0 );

	int sPointsToUse = (MAX_HUMAN_CREATURE_SMELL - pSoldier->bMonsterSmell) * 2;
	sPointsToUse = std::min(sPointsToUse, usTotalKitPoints);

	UseKitPoints(*pObj, sPointsToUse, *pSoldier);

	pSoldier->bMonsterSmell += sPointsToUse / 2;

	return( TRUE );
}


bool ItemIsCool(OBJECTTYPE const& o)
{
	if (o.bStatus[0] < 60) return false;
	const ItemModel * item = GCM->getItem(o.usItem);
	if (item->isWeapon())
	{
		if (GCM->getWeapon(o.usItem)->ubDeadliness >= 30) return true;
	}
	else if (item->isArmour())
	{
		if (Armour[item->getClassIndex()].ubProtection >= 20) return true;
	}

	return false;
}

void ActivateXRayDevice( SOLDIERTYPE * pSoldier )
{
	INT8 bBatteries;

	// check for batteries
	bBatteries = FindAttachment( &(pSoldier->inv[HANDPOS]), BATTERIES );
	if ( bBatteries == NO_SLOT )
	{
		// doesn't work without batteries!
		return;
	}

	// use up 8-12 percent of batteries
	pSoldier->inv[ HANDPOS ].bAttachStatus[ bBatteries ] -= (INT8) (8 + Random( 5 ));
	if ( pSoldier->inv[ HANDPOS ].bAttachStatus[ bBatteries ] <= 0 )
	{
		// destroy batteries
		pSoldier->inv[ HANDPOS ].usAttachItem[ bBatteries ] = NOTHING;
		pSoldier->inv[ HANDPOS ].bAttachStatus[ bBatteries ] = 0;
	}

	// first, scan through all mercs and turn off xrayed flag for anyone
	// previously xrayed by this guy
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const tgt = *i;
		if (tgt->ubMiscSoldierFlags &  SOLDIER_MISC_XRAYED &&
				tgt->xrayed_by          == pSoldier)
		{
			tgt->ubMiscSoldierFlags &= ~SOLDIER_MISC_XRAYED;
			tgt->xrayed_by           = NULL;
		}
	}
	// now turn on xray for anyone within range
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const tgt = *i;
		if (tgt->bTeam != pSoldier->bTeam &&
			PythSpacesAway(pSoldier->sGridNo, tgt->sGridNo) < XRAY_RANGE)
		{
			tgt->ubMiscSoldierFlags |= SOLDIER_MISC_XRAYED;
			tgt->xrayed_by           = pSoldier;
		}
	}
	pSoldier->uiXRayActivatedTime = GetWorldTotalSeconds();
}

void TurnOffXRayEffects( SOLDIERTYPE * pSoldier )
{
	if ( !pSoldier->uiXRayActivatedTime )
	{
		return;
	}

	// scan through all mercs and turn off xrayed flag for anyone
	// xrayed by this guy
	FOR_EACH_MERC(i)
	{
		SOLDIERTYPE* const tgt = *i;
		if (tgt->ubMiscSoldierFlags &  SOLDIER_MISC_XRAYED &&
			tgt->xrayed_by == pSoldier)
		{
			tgt->ubMiscSoldierFlags &= ~SOLDIER_MISC_XRAYED;
			tgt->xrayed_by           = NULL;
		}
	}
	pSoldier->uiXRayActivatedTime = 0;
}


bool HasObjectImprint(OBJECTTYPE const& o)
{
	return (o.usItem == ROCKET_RIFLE || o.usItem == AUTO_ROCKET_RIFLE) &&
		o.ubImprintID != NO_PROFILE;
}
