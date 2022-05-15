#include "AI.h"
#include "AIInternals.h"
#include "Animation_Control.h"
#include "Isometric_Utils.h"
#include "Points.h"
#include "Overhead.h"
#include "OppList.h"
#include "Rotting_Corpses.h"
#include "Soldier_Add.h"
#include "Soldier_Profile_Type.h"
#include "Items.h"
#include "Weapons.h"
#include "Soldier_Macros.h"
#include "NPC.h"
#include "Render_Fun.h"
#include "Quests.h"


static INT8 RTPlayerDecideAction(SOLDIERTYPE* pSoldier)
{
	INT8 bAction=AI_ACTION_NONE;

	if (gTacticalStatus.fAutoBandageMode)
	{
		bAction = DecideAutoBandage( pSoldier );
	}
	else
	{
		bAction = DecideAction( pSoldier );
	}
	SLOGD("DecideAction: selected action {}, actionData {}\n\n",
				bAction, pSoldier->usActionData);

	return(bAction);
}


static INT8 RTDecideAction(SOLDIERTYPE* pSoldier)
{
	if (CREATURE_OR_BLOODCAT( pSoldier ) )
	{
		return( CreatureDecideAction( pSoldier ) );
	}
	else if (pSoldier->ubBodyType == CROW)
	{
		return( CrowDecideAction( pSoldier ) );
	}
	else if (pSoldier->bTeam == OUR_TEAM)
	{
		return( RTPlayerDecideAction( pSoldier ) );
	}
	else
	{
		// handle traversal
		if ( (pSoldier->ubProfile != NO_PROFILE) && (gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_HANDLE_DONE_TRAVERSAL ) )
		{
			TriggerNPCWithGivenApproach(pSoldier->ubProfile, APPROACH_DONE_TRAVERSAL);
			gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 &= (~PROFILE_MISC_FLAG3_HANDLE_DONE_TRAVERSAL);
			pSoldier->ubQuoteActionID = 0;
			// wait a tiny bit
			pSoldier->usActionData = 100;
			return( AI_ACTION_WAIT );
		}

		return( DecideAction( pSoldier ) );
	}
}

UINT16 RealtimeDelay( SOLDIERTYPE * pSoldier )
{
	if ( PTR_CIV_OR_MILITIA && !(pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP ) )
	{
		return( (UINT16) REALTIME_CIV_AI_DELAY );
	}
	else if ( CREATURE_OR_BLOODCAT( pSoldier ) && !( pSoldier->bHunting ) )
	{
		return( (UINT16) REALTIME_CREATURE_AI_DELAY );
	}
	else
	{

		if ( pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP )
		{
			UINT8 const room = GetRoom(pSoldier->sGridNo);
			if (IN_BROTHEL(room))
			{
				return( (UINT16) (REALTIME_AI_DELAY / 3) );
			}
		}

		return( (UINT16) REALTIME_AI_DELAY );
	}

}


void RTHandleAI( SOLDIERTYPE * pSoldier )
{
#ifdef AI_PROFILING
	INT32 iLoop;
#endif

	if ((pSoldier->bAction != AI_ACTION_NONE) && pSoldier->bActionInProgress)
	{
		// if action should remain in progress
		if (ActionInProgress(pSoldier))
		{
			// let it continue
			return;
		}
	}

	// if man has nothing to do
	if (pSoldier->bAction == AI_ACTION_NONE)
	{
		if (pSoldier->bNextAction == AI_ACTION_NONE)
		{
			// make sure this flag is turned off (it already should be!)
			pSoldier->bActionInProgress = FALSE;

			// truly nothing to do!
			RefreshAI( pSoldier );
		}

		// Since we're NEVER going to "continue" along an old path at this point,
		// then it would be nice place to reinitialize "pathStored" flag for
		// insurance purposes.
		//
		// The "pathStored" variable controls whether it's necessary to call
		// findNewPath() after you've called NewDest(). Since the AI calls
		// findNewPath() itself, a speed gain can be obtained by avoiding
		// redundancy.
		//
		// The "normal" way for pathStored to be reset is inside
		// SetNewCourse() [which gets called after NewDest()].
		//
		// The only reason we would NEED to reinitialize it here is if I've
		// incorrectly set pathStored to TRUE in a process that doesn't end up
		// calling NewDest()
		pSoldier->bPathStored = FALSE;

		// decide on the next action
#ifdef AI_PROFILING
		for (iLoop = 0; iLoop < 1000; iLoop++)
#endif
		{
			if (pSoldier->bNextAction != AI_ACTION_NONE)
			{
				if ( pSoldier->bNextAction == AI_ACTION_END_COWER_AND_MOVE )
				{
					if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
					{
						pSoldier->bAction = AI_ACTION_STOP_COWERING;
						pSoldier->usActionData = ANIM_STAND;
					}
					else if ( gAnimControl[ pSoldier->usAnimState ].ubEndHeight < ANIM_STAND )
					{
						// stand up!
						pSoldier->bAction = AI_ACTION_CHANGE_STANCE;
						pSoldier->usActionData = ANIM_STAND;
					}
					else
					{
						pSoldier->bAction = AI_ACTION_NONE;
					}
					if ( pSoldier->sGridNo == pSoldier->usNextActionData )
					{
						// no need to walk after this
						pSoldier->bNextAction = AI_ACTION_NONE;
						pSoldier->usNextActionData = NOWHERE;
					}
					else
					{
						pSoldier->bNextAction = AI_ACTION_WALK;
						// leave next-action-data as is since that's where we want to go
					}
				}
				else
				{
					// do the next thing we have to do...
					pSoldier->bAction = pSoldier->bNextAction;
					pSoldier->usActionData = pSoldier->usNextActionData;
					pSoldier->bTargetLevel = pSoldier->bNextTargetLevel;
					pSoldier->bNextAction = AI_ACTION_NONE;
					pSoldier->usNextActionData = 0;
					pSoldier->bNextTargetLevel = 0;
				}
				if (pSoldier->bAction == AI_ACTION_PICKUP_ITEM)
				{
					// the item pool index was stored in the special data field
					pSoldier->uiPendingActionData1 = pSoldier->iNextActionSpecialData;
				}
			}
			else if ( pSoldier->sAbsoluteFinalDestination != NOWHERE )
			{
				if ( ACTING_ON_SCHEDULE( pSoldier ) )
				{
					pSoldier->bAction = AI_ACTION_SCHEDULE_MOVE;
				}
				else
				{
					pSoldier->bAction = AI_ACTION_WALK;
				}
				pSoldier->usActionData = pSoldier->sAbsoluteFinalDestination;
			}
			else
			{
				if (!(gTacticalStatus.uiFlags & ENGAGED_IN_CONV))
				{
					pSoldier->bAction = RTDecideAction( pSoldier );
				}
			}
		}
		// if he chose to continue doing nothing
		if (pSoldier->bAction == AI_ACTION_NONE)
		{
			// do a standard wait before doing anything else!
			pSoldier->bAction = AI_ACTION_WAIT;
			if ( PTR_CIV_OR_MILITIA && !(pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP ) )
			{
				pSoldier->usActionData = (UINT16) REALTIME_CIV_AI_DELAY;
			}
			else if ( CREATURE_OR_BLOODCAT( pSoldier ) && !( pSoldier->bHunting ) )
			{
				pSoldier->usActionData = (UINT16) REALTIME_CREATURE_AI_DELAY;
			}
			else
			{
				pSoldier->usActionData = (UINT16) REALTIME_AI_DELAY;
				if ( pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP )
				{
					UINT8 const room = GetRoom(pSoldier->sGridNo);
					if (IN_BROTHEL(room))
					{
						pSoldier->usActionData /= 3;
					}
				}
			}
		}
		else if (pSoldier->bAction == AI_ACTION_ABSOLUTELY_NONE)
		{
			pSoldier->bAction = AI_ACTION_NONE;
		}

	}

	// to get here, we MUST have an action selected, but not in progress...
	NPCDoesAct(pSoldier);

	// perform the chosen action
	pSoldier->bActionInProgress = ExecuteAction(pSoldier); // if started, mark us as busy
}
