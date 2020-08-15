#include "Soldier.h"

#include "game/Tactical/Animation_Control.h"
#include "game/Tactical/Handle_Items.h"
#include "game/Tactical/Interface.h"
#include "game/Tactical/Items.h"
#include "game/Tactical/Overhead.h"
#include "game/Tactical/Soldier_Control.h"
#include "game/Tactical/Soldier_Functions.h"
#include "game/TacticalAI/AI.h"
#include "game/Utils/Font_Control.h"
#include "game/Utils/Message.h"
#include "game/Utils/Text.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "content/NewStrings.h"
#include "content/npcs.h"
#include "internals/enums.h"

#include "Logger.h"

#include <vector>

/** Get soldier object from the structure. */
std::shared_ptr<Soldier> GetSoldier(struct SOLDIERTYPE* s)
{
	return std::make_shared<Soldier>(s);
}

/** Get soldier object from the structure. */
std::shared_ptr<const Soldier> GetSoldier(const struct SOLDIERTYPE* s)
{
	return std::make_shared<const Soldier>((struct SOLDIERTYPE*)s);
}

Soldier:: Soldier(SOLDIERTYPE* s)
	:mSoldier(s)
{
}

/** Remove pending action. */
void Soldier::removePendingAction()
{
	if(mSoldier->ubPendingAction != NO_PENDING_ACTION)
	{
		SLOGD("%s: remove pending action %s",
			getPofileName(),
			Internals::getActionName(mSoldier->ubPendingAction));

		mSoldier->ubPendingAction = NO_PENDING_ACTION;
	}
}

/** Remove any pending animation. */
void Soldier::removePendingAnimation()
{
	mSoldier->usPendingAnimation = NO_PENDING_ANIMATION;
	mSoldier->ubPendingDirection = NO_PENDING_DIRECTION;
	removePendingAction();
}

bool Soldier::hasPendingAction(UINT8 action) const
{
	if (action == NO_PENDING_ACTION)
	{
		// any action is ok
		return mSoldier->ubPendingAction != action;
	}
	else
	{
		return mSoldier->ubPendingAction == action;
	}
}

bool Soldier::anyoneHasPendingAction(UINT8 action, UINT8 team)
{
	bool anyone = false;
	CFOR_EACH_IN_TEAM(s, team)
	{
		std::shared_ptr<const Soldier> soldier = GetSoldier(s);
		if (soldier->hasPendingAction(action))
		{
			anyone = true;
			break;
		}
	}
	return anyone;
}

void Soldier::setPendingAction(UINT8 action)
{
	SLOGD("%s: set pending action %s (previous %s)",
		getPofileName(),
		Internals::getActionName(action),
		Internals::getActionName(mSoldier->ubPendingAction));

	mSoldier->ubPendingAction          = action;
	mSoldier->ubPendingActionAnimCount = 0;
}


const char* Soldier::getPofileName() const
{
	return Content::getNpcName((NPCIDs)mSoldier->ubProfile);
}


/**
 * Handle pending action.
 * Return true, when all further processing should be stopped. */
bool Soldier::handlePendingAction(bool inCombat)
{
	if (mSoldier->ubPendingAction == MERC_PICKUPITEM)
	{
		const INT16 sGridNo = mSoldier->sPendingActionData2;
		if (sGridNo == mSoldier->sGridNo)
		{
			// OK, now, if in realtime
			if (!inCombat)
			{
				// If the two gridnos are not the same, check to see if we can
				// now go into it
				if (sGridNo != (INT16)mSoldier->uiPendingActionData4)
				{
					if (NewOKDestination(mSoldier, (INT16)mSoldier->uiPendingActionData4, TRUE, mSoldier->bLevel))
					{
						// GOTO NEW TILE!
						SoldierPickupItem(mSoldier, mSoldier->uiPendingActionData1, (INT16)mSoldier->uiPendingActionData4, mSoldier->bPendingActionData3);
						return true;
					}
				}
			}

			PickPickupAnimation(mSoldier, mSoldier->uiPendingActionData1, (INT16)mSoldier->uiPendingActionData4, mSoldier->bPendingActionData3);
		}
		else
		{
			SoldierGotoStationaryStance(mSoldier);
		}
	}
	else if (mSoldier->ubPendingAction == MERC_PUNCH)
	{
		// for the benefit of the AI
		mSoldier->bAction = AI_ACTION_KNIFE_STAB;

		EVENT_SoldierBeginPunchAttack(mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_TALK)
	{
		PlayerSoldierStartTalking(mSoldier, (UINT8)mSoldier->uiPendingActionData1, TRUE);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_DROPBOMB)
	{
		EVENT_SoldierBeginDropBomb(mSoldier);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_STEAL)
	{
		//mSoldier->bDesiredDirection = mSoldier->bPendingActionData3;
		EVENT_SetSoldierDesiredDirection(mSoldier, mSoldier->bPendingActionData3);

		EVENT_InitNewSoldierAnim(mSoldier, STEAL_ITEM, 0, FALSE);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_KNIFEATTACK)
	{
		// for the benefit of the AI
		mSoldier->bAction = AI_ACTION_KNIFE_STAB;

		EVENT_SoldierBeginBladeAttack(mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_GIVEAID)
	{
		EVENT_SoldierBeginFirstAid(mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_REPAIR)
	{
		EVENT_SoldierBeginRepair(*mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_FUEL_VEHICLE)
	{
		EVENT_SoldierBeginRefuel(mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_RELOADROBOT)
	{
		EVENT_SoldierBeginReloadRobot(mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3, (INT8)mSoldier->uiPendingActionData1);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_TAKEBLOOD)
	{
		EVENT_SoldierBeginTakeBlood( mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3 );
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_ATTACH_CAN)
	{
		EVENT_SoldierBeginAttachCan(mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_ENTER_VEHICLE)
	{
		EVENT_SoldierEnterVehicle(*mSoldier, mSoldier->sPendingActionData2);
		removePendingAction();
		return true;
	}
	else if (mSoldier->ubPendingAction == MERC_CUTFFENCE)
	{
		EVENT_SoldierBeginCutFence(mSoldier, mSoldier->sPendingActionData2, mSoldier->bPendingActionData3);
		removePendingAction();
	}
	else if (mSoldier->ubPendingAction == MERC_GIVEITEM)
	{
		EVENT_SoldierBeginGiveItem(mSoldier);
		removePendingAction();
	}
	return false;
}

int8_t Soldier::getFreeHeadSlot() const
{
	if(mSoldier->inv[HEAD1POS].usItem == NONE)
	{
		return HEAD1POS;
	}
	if(mSoldier->inv[HEAD2POS].usItem == NONE)
	{
		return HEAD2POS;
	}
	return NO_SLOT;
}

int8_t Soldier::getFreePocket() const
{
	if(mSoldier->inv[SMALLPOCK1POS].usItem == NONE)  return SMALLPOCK1POS;
	if(mSoldier->inv[SMALLPOCK2POS].usItem == NONE)  return SMALLPOCK2POS;
	if(mSoldier->inv[SMALLPOCK3POS].usItem == NONE)  return SMALLPOCK3POS;
	if(mSoldier->inv[SMALLPOCK4POS].usItem == NONE)  return SMALLPOCK4POS;
	if(mSoldier->inv[SMALLPOCK5POS].usItem == NONE)  return SMALLPOCK5POS;
	if(mSoldier->inv[SMALLPOCK6POS].usItem == NONE)  return SMALLPOCK6POS;
	if(mSoldier->inv[SMALLPOCK7POS].usItem == NONE)  return SMALLPOCK7POS;
	if(mSoldier->inv[SMALLPOCK8POS].usItem == NONE)  return SMALLPOCK8POS;
	if(mSoldier->inv[BIGPOCK1POS].usItem == NONE)    return BIGPOCK1POS;
	if(mSoldier->inv[BIGPOCK2POS].usItem == NONE)    return BIGPOCK2POS;
	if(mSoldier->inv[BIGPOCK3POS].usItem == NONE)    return BIGPOCK3POS;
	if(mSoldier->inv[BIGPOCK4POS].usItem == NONE)    return BIGPOCK4POS;
	return NO_SLOT;
}


/** Swap inventory slots. */
void Soldier::swapInventorySlots(int8_t firstSlot, int8_t secondSlot)
{
	SwapObjs( &(mSoldier->inv[firstSlot]), &(mSoldier->inv[secondSlot]) );
}

static bool isHeadPosition(int8_t pos)
{
	return (pos == HEAD1POS) || (pos == HEAD2POS);
}

static void showGearEquipMessage(const SOLDIERTYPE* s, uint16_t usItem)
{
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(*GCM->getNewString(NS_SOLDIER_EQUIPS_ITEM), s->name, ItemNames[usItem]));
}

static void showGearRemoveMessage(const SOLDIERTYPE* s, uint16_t usItem)
{
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(*GCM->getNewString(NS_SOLDIER_REMOVES_ITEM), s->name, ItemNames[usItem]));
}

const std::vector<ITEMDEFINE> HEAD_GEARS_DAY   { SUNGOGGLES };
const std::vector<ITEMDEFINE> HEAD_GEARS_NIGHT { UVGOGGLES, NIGHTGOGGLES };

#define SWITCH_TO_NIGHT_GEAR 0
#define SWITCH_TO_DAY_GEAR   1

void Soldier::putNightHeadGear()
{
	switchHeadGear(SWITCH_TO_NIGHT_GEAR);
}

void Soldier::putDayHeadGear()
{
	switchHeadGear(SWITCH_TO_DAY_GEAR);
}

void Soldier::switchHeadGear(int switchDirection)
{
	if (IsWearingHeadGear(*mSoldier, GASMASK))
	{
		// Gas mask is not compatible with eye gears
		return;
	}

	const std::vector<ITEMDEFINE>& fromGears = (switchDirection == SWITCH_TO_NIGHT_GEAR) ? HEAD_GEARS_DAY : HEAD_GEARS_NIGHT;
	const std::vector<ITEMDEFINE>& toGears   = (switchDirection == SWITCH_TO_NIGHT_GEAR) ? HEAD_GEARS_NIGHT : HEAD_GEARS_DAY;
	OBJECTTYPE* helmet = &mSoldier->inv[HELMETPOS];
	OBJECTTYPE  detachedGear{};

	// find the gear we want to wear
	OBJECTTYPE* optimalEyeGear = NULL;
	for (ITEMDEFINE ic : fromGears)
	{
		INT8 slot = FindObj(mSoldier, ic);
		if (slot != NO_SLOT)
		{
			// found one in inventory
			optimalEyeGear = &(mSoldier->inv[slot]);
			break;
		}
		slot = FindAttachment(helmet, ic);
		if (slot != ITEM_NOT_FOUND)
		{
			// found one in helmet attachment
			RemoveAttachment(helmet, slot, &detachedGear);
			optimalEyeGear = &detachedGear;
		}
	}
	if (optimalEyeGear == NULL)
	{
		// no usable gear found
		return;
	}

	// find the eye gear we are currently wearing
	OBJECTTYPE* currentEyeGear = NULL;
	for (ITEMDEFINE ic : toGears)
	{
		if (mSoldier->inv[HEAD1POS].usItem == ic) currentEyeGear = &mSoldier->inv[HEAD1POS];
		if (mSoldier->inv[HEAD2POS].usItem == ic) currentEyeGear = &mSoldier->inv[HEAD2POS];
	}
	if (currentEyeGear == NULL && getFreeHeadSlot() != NO_SLOT)
	{
		// not wearing eye gear but slot available
		currentEyeGear = &mSoldier->inv[getFreeHeadSlot()];
	}
	if (currentEyeGear == NULL)
	{
		// no room to wear any eye gear
		return;
	}

	showGearEquipMessage(mSoldier, optimalEyeGear->usItem);
	SwapObjs(optimalEyeGear, currentEyeGear);

	if (detachedGear.usItem != NONE)
	{
		// the gear we want had been detached from helmet and put on; 
		// now after the object swap, we attach the gear we just put off
		AttachObject(mSoldier, helmet, &detachedGear);
	}

	DirtyMercPanelInterface(mSoldier, DIRTYLEVEL2);
}
