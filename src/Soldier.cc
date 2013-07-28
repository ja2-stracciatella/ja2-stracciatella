#include "Soldier.h"

#include "Tactical/Animation_Control.h"
#include "Tactical/Handle_Items.h"
#include "Tactical/Overhead.h"
#include "Tactical/Soldier_Control.h"
#include "Tactical/Soldier_Functions.h"
#include "TacticalAI/AI.h"
#include "content/npcs.h"
#include "internals/enums.h"

#include "boost/make_shared.hpp"

#include "slog/slog.h"
#define TAG "Soldier"

/** Get soldier object from the structure. */
boost::shared_ptr<Soldier> GetSoldier(struct SOLDIERTYPE* s)
{
  return boost::make_shared<Soldier>(s);
}

/** Get soldier object from the structure. */
boost::shared_ptr<const Soldier> GetSoldier(const struct SOLDIERTYPE* s)
{
  return boost::make_shared<const Soldier>((struct SOLDIERTYPE*)s);
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
    SLOGI(TAG, "%s: remove pending action %s",
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

bool Soldier::hasPendingAction() const
{
  return mSoldier->ubPendingAction != NO_PENDING_ACTION;
}

void Soldier::setPendingAction(UINT8 action)
{
  SLOGI(TAG, "%s: set pending action %s (previous %s)",
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
