#include <stdexcept>

#include "Event_Manager.h"
#include "Timer_Control.h"
#include "Event_Pump.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Weapons.h"
#include "OppList.h"
#include "Debug.h"
#include "Logger.h"


void AddGameEvent(GAMEEVENT const& gameEvent, UINT16 usDelay)
{
	if (usDelay == DEMAND_EVENT_DELAY)
	{
		AddEvent(gameEvent, 0, EventQueueID::DEMAND_EVENT_QUEUE);
	}
	else
	{
		AddEvent(gameEvent, usDelay, EventQueueID::PRIMARY_EVENT_QUEUE);
	}
}

static void ExecuteGameEvent(EVENT* pEvent);

BOOLEAN DequeAllGameEvents(void)
{
	UINT32  uiQueueSize;
	UINT32  cnt;

	// First dequeue all primary events
	while (EventQueueSize(EventQueueID::PRIMARY_EVENT_QUEUE) > 0)
	{
		EVENT* pEvent = RemoveEvent(0, EventQueueID::PRIMARY_EVENT_QUEUE);
		if (pEvent == NULL) return FALSE;

		// Check if event has a delay and add to secondary queue if so
		if (pEvent->usDelay > 0)
		{
			AddEvent(pEvent->gameEvent, pEvent->usDelay, EventQueueID::SECONDARY_EVENT_QUEUE);
		}
		else
		{
			ExecuteGameEvent(pEvent);
		}

		FreeEvent(pEvent);
	}

	// NOW CHECK SECONDARY QUEUE FOR ANY EXPRIED EVENTS
	uiQueueSize = EventQueueSize(EventQueueID::SECONDARY_EVENT_QUEUE);
	for (cnt = 0; cnt < uiQueueSize; cnt++)
	{
		EVENT* pEvent = PeekEvent(cnt, EventQueueID::SECONDARY_EVENT_QUEUE);
		if (pEvent == NULL) return FALSE;

		// Check time
		if (GetJA2Clock() - pEvent->TimeStamp > pEvent->usDelay)
		{
			ExecuteGameEvent(pEvent);
			pEvent->uiFlags = EVENT_EXPIRED;
		}
	}

	do
	{
		uiQueueSize = EventQueueSize(EventQueueID::SECONDARY_EVENT_QUEUE);
		for (cnt = 0; cnt < uiQueueSize; cnt++)
		{
			EVENT* pEvent = PeekEvent(cnt, EventQueueID::SECONDARY_EVENT_QUEUE);
			if (pEvent == NULL)
			{
				return FALSE;
			}

			// Check time
			if (pEvent->uiFlags & EVENT_EXPIRED)
			{
				pEvent = RemoveEvent(cnt, EventQueueID::SECONDARY_EVENT_QUEUE);
				FreeEvent(pEvent);
				// Restart loop
				break;
			}
		}
	} while (cnt != uiQueueSize);

	return TRUE;
}


BOOLEAN DequeueAllDemandGameEvents(void)
{
	// Dequeue all events on the demand queue (only)

	while (EventQueueSize(EventQueueID::DEMAND_EVENT_QUEUE) > 0)
	{
		EVENT* pEvent = RemoveEvent(0, EventQueueID::DEMAND_EVENT_QUEUE);
		if (pEvent == NULL) return FALSE;

		// Check if event has a delay and add to secondary queue if so
		if (pEvent->usDelay > 0)
		{
			AddEvent(pEvent->gameEvent, pEvent->usDelay, EventQueueID::SECONDARY_EVENT_QUEUE);
		}
		else
		{
			ExecuteGameEvent(pEvent);
		}

		FreeEvent(pEvent);
	}

	return TRUE;
}


static SOLDIERTYPE* GetSoldier(const UINT16 soldier_idx)
{
	if (soldier_idx >= TOTAL_SOLDIERS) return NULL; // XXX assert?
	SOLDIERTYPE& s = GetMan(soldier_idx);
	return s.bActive ? &s : 0;
}


// Copied from https://stackoverflow.com/posts/52303671/revisions
// Author: Barry
template<typename VariantType, typename T, std::size_t index = 0>
    constexpr std::size_t variant_index() {
        if constexpr (index == std::variant_size_v<VariantType>) {
            return index;
        } else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
            return index;
		} else {
			return variant_index<VariantType, T, index + 1>();
	    }
	}


static void ExecuteGameEvent(EVENT* pEvent)
{
	// Switch on event type
	switch (pEvent->gameEvent.index())
	{
		case variant_index<GAMEEVENT, EV_S_GETNEWPATH>():
		{
			auto const& SGetNewPath = std::get<EV_S_GETNEWPATH>(pEvent->gameEvent);

			SOLDIERTYPE* pSoldier = GetSoldier(SGetNewPath.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				break;
			}

			// check for error
			if (pSoldier->uiUniqueSoldierIdValue != SGetNewPath.uiUniqueId)
			{
				break;
			}
			// Call soldier function
			SLOGD("GetNewPath");
			EVENT_GetNewSoldierPath(pSoldier, SGetNewPath.sDestGridNo, SGetNewPath.usMovementAnim);
			break;
		}

		case variant_index<GAMEEVENT, EV_S_SETDESIREDDIRECTION>():
		{
			auto const& SSetDesiredDirection = std::get<EV_S_SETDESIREDDIRECTION>(pEvent->gameEvent);

			SOLDIERTYPE* pSoldier = GetSoldier(SSetDesiredDirection.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				break;
			}

			// check for error
			if (pSoldier-> uiUniqueSoldierIdValue != SSetDesiredDirection.uiUniqueId)
			{
				break;
			}

			// Call soldier function
			SLOGD("SetDesiredDirection: Dir( %d )", SSetDesiredDirection.usDesiredDirection);
			EVENT_SetSoldierDesiredDirection(pSoldier, SSetDesiredDirection.usDesiredDirection);
			break;
		}

		case variant_index<GAMEEVENT, EV_S_BEGINFIREWEAPON>():
		{
			auto const& SBeginFireWeapon = std::get<EV_S_BEGINFIREWEAPON>(pEvent->gameEvent);

			SOLDIERTYPE* pSoldier = GetSoldier(SBeginFireWeapon.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				break;
			}

			// check for error
			if (pSoldier->uiUniqueSoldierIdValue != SBeginFireWeapon.uiUniqueId)
			{
				break;
			}

			// Call soldier function
			SLOGD("Begin Fire Weapon");
			pSoldier->sTargetGridNo    = SBeginFireWeapon.sTargetGridNo;
			pSoldier->bTargetLevel     = SBeginFireWeapon.bTargetLevel;
			pSoldier->bTargetCubeLevel = SBeginFireWeapon.bTargetCubeLevel;
			EVENT_FireSoldierWeapon(pSoldier, SBeginFireWeapon.sTargetGridNo);
			break;
		}

		case variant_index<GAMEEVENT, EV_S_FIREWEAPON>():
		{
			auto const& SFireWeapon = std::get<EV_S_FIREWEAPON>(pEvent->gameEvent);

			SOLDIERTYPE* pSoldier = GetSoldier(SFireWeapon.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				break;
			}

			// check for error
			if (pSoldier->uiUniqueSoldierIdValue != SFireWeapon.uiUniqueId)
			{
				break;
			}


			// Call soldier function
			SLOGD("FireWeapon");
			pSoldier->sTargetGridNo    = SFireWeapon.sTargetGridNo;
			pSoldier->bTargetLevel     = SFireWeapon.bTargetLevel;
			pSoldier->bTargetCubeLevel = SFireWeapon.bTargetCubeLevel;
			FireWeapon(pSoldier, SFireWeapon.sTargetGridNo);
			break;
		}

		case variant_index<GAMEEVENT, EV_S_WEAPONHIT>():
		{
			auto const& SWeaponHit = std::get<EV_S_WEAPONHIT>(pEvent->gameEvent);
			SLOGD("WeaponHit %d Damage", SWeaponHit.sDamage);
			WeaponHit(&GetMan(SWeaponHit.usSoldierID), SWeaponHit.usWeaponIndex, SWeaponHit.sDamage, SWeaponHit.sBreathLoss, SWeaponHit.usDirection, SWeaponHit.sXPos, SWeaponHit.sYPos, SWeaponHit.sZPos, SWeaponHit.sRange, &GetMan(SWeaponHit.ubAttackerID), SWeaponHit.ubSpecial, SWeaponHit.ubLocation);
			break;
		}

		case variant_index<GAMEEVENT, EV_S_NOISE>():
		{
			auto const& SNoise = std::get<EV_S_NOISE>(pEvent->gameEvent);
			SLOGD("Noise from %d at %d/%d, type %d volume %d",
						SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel,
						SNoise.ubNoiseType, SNoise.ubVolume);
			OurNoise(ID2SOLDIER(SNoise.ubNoiseMaker), SNoise.sGridNo, SNoise.bLevel, SNoise.ubVolume, static_cast<NoiseKind>(SNoise.ubNoiseType));
			break;
		}
	}
}


BOOLEAN ClearEventQueue(void)
{
	// clear out the event queue
	while (EventQueueSize(EventQueueID::PRIMARY_EVENT_QUEUE) > 0)
	{
		EVENT* Event = RemoveEvent(0, EventQueueID::PRIMARY_EVENT_QUEUE);
		if (Event == NULL) return FALSE;
		FreeEvent(Event);
	}

	return TRUE;
}
