#include "Timer_Control.h"
#include "Event_Pump.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Weapons.h"
#include "OppList.h"
#include "Debug.h"
#include "Logger.h"

#include <stdexcept>
#include <vector>

struct EVENT
{
	UINT32 TimeStamp;
	UINT16 usDelay;
	bool eventExpired;
	GAMEEVENT gameEvent;
};

enum class EventQueueID
{
	PRIMARY_EVENT_QUEUE,
	SECONDARY_EVENT_QUEUE,
	DEMAND_EVENT_QUEUE
};


typedef std::vector<EVENT*>EventList;
static EventList hEventQueue;
static EventList hDelayEventQueue;
static EventList hDemandEventQueue;


static EventList& GetQueue(EventQueueID ubQueueID);


static void AddEvent(GAMEEVENT const& gameEvent, UINT16 const usDelay, EventQueueID const ubQueueID)
{
	EVENT* pEvent = new EVENT{};
	pEvent->TimeStamp  = GetJA2Clock();
	pEvent->usDelay    = usDelay;
	pEvent->eventExpired = false;
	pEvent->gameEvent  = gameEvent;
	GetQueue(ubQueueID).push_back(pEvent);
}

// Remove the first element from a queue and return it.
// Calling this function on an empty queue is undefined behavior, don't do it.
static EVENT* PopFrontEvent(EventQueueID ubQueueID)
{
	EventList& queue = GetQueue(ubQueueID);
	Assert(!queue.empty());
	EVENT* const ret = queue.front();
	queue.erase(queue.begin());
	return ret;
}


static UINT32 EventQueueSize(EventQueueID ubQueueID)
{
	return static_cast<UINT32>(GetQueue(ubQueueID).size());
}


static EventList& GetQueue(EventQueueID const ubQueueID)
{
	switch (ubQueueID)
	{
		case EventQueueID::PRIMARY_EVENT_QUEUE:   return hEventQueue;
		case EventQueueID::SECONDARY_EVENT_QUEUE: return hDelayEventQueue;
		case EventQueueID::DEMAND_EVENT_QUEUE:    return hDemandEventQueue;
	}
	// Never reached, squelches GCC -Wreturn-type warning
	return hEventQueue;
}


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

void DequeAllGameEvents(void)
{
	// First dequeue all primary events
	while (EventQueueSize(EventQueueID::PRIMARY_EVENT_QUEUE) > 0)
	{
		EVENT* pEvent = PopFrontEvent(EventQueueID::PRIMARY_EVENT_QUEUE);

		// Check if event has a delay and add to secondary queue if so
		if (pEvent->usDelay > 0)
		{
			AddEvent(pEvent->gameEvent, pEvent->usDelay, EventQueueID::SECONDARY_EVENT_QUEUE);
		}
		else
		{
			ExecuteGameEvent(pEvent);
		}

		delete pEvent;
	}

	// NOW CHECK SECONDARY QUEUE FOR ANY EXPRIED EVENTS
	EventList& queue = GetQueue(EventQueueID::SECONDARY_EVENT_QUEUE);
	for (EVENT* const pEvent : queue)
	{
		// Check time
		if (GetJA2Clock() - pEvent->TimeStamp > pEvent->usDelay)
		{
			ExecuteGameEvent(pEvent);
			pEvent->eventExpired = true;
		}
	}

	// Remove and free all expired events from the secondary queue.
	queue.erase(std::remove_if(queue.begin(), queue.end(),
		[](EVENT* const ep)
		{
			if (!ep->eventExpired) return false;
			delete ep;
			return true;
		}), queue.end());
}


void DequeueAllDemandGameEvents(void)
{
	// Dequeue all events on the demand queue (only)

	while (EventQueueSize(EventQueueID::DEMAND_EVENT_QUEUE) > 0)
	{
		EVENT* pEvent = PopFrontEvent(EventQueueID::DEMAND_EVENT_QUEUE);

		// Check if event has a delay and add to secondary queue if so
		if (pEvent->usDelay > 0)
		{
			AddEvent(pEvent->gameEvent, pEvent->usDelay, EventQueueID::SECONDARY_EVENT_QUEUE);
		}
		else
		{
			ExecuteGameEvent(pEvent);
		}

		delete pEvent;
	}
}


static SOLDIERTYPE* GetSoldier(const UINT16 soldier_idx)
{
	if (soldier_idx >= TOTAL_SOLDIERS) return NULL; // XXX assert?
	SOLDIERTYPE& s = GetMan(soldier_idx);
	return s.bActive ? &s : 0;
}


// Source: https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static void ExecuteGameEvent(EVENT* pEvent)
{
	// Switch on event type
	std::visit(overloaded{
		[](EV_S_GETNEWPATH const& SGetNewPath)
		{
			SOLDIERTYPE* pSoldier = GetSoldier(SGetNewPath.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				return;
			}

			// check for error
			if (pSoldier->uiUniqueSoldierIdValue != SGetNewPath.uiUniqueId)
			{
				return;
			}
			// Call soldier function
			SLOGD("GetNewPath");
			EVENT_GetNewSoldierPath(pSoldier, SGetNewPath.sDestGridNo, SGetNewPath.usMovementAnim);
		},
		[](EV_S_SETDESIREDDIRECTION const& SSetDesiredDirection)
		{
			SOLDIERTYPE* pSoldier = GetSoldier(SSetDesiredDirection.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				return;
			}

			// check for error
			if (pSoldier-> uiUniqueSoldierIdValue != SSetDesiredDirection.uiUniqueId)
			{
				return;
			}

			// Call soldier function
			SLOGD("SetDesiredDirection: Dir( {} )", SSetDesiredDirection.usDesiredDirection);
			EVENT_SetSoldierDesiredDirection(pSoldier, SSetDesiredDirection.usDesiredDirection);
		},
		[](EV_S_BEGINFIREWEAPON const& SBeginFireWeapon)
		{
			SOLDIERTYPE* pSoldier = GetSoldier(SBeginFireWeapon.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				return;
			}

			// check for error
			if (pSoldier->uiUniqueSoldierIdValue != SBeginFireWeapon.uiUniqueId)
			{
				return;
			}

			// Call soldier function
			SLOGD("Begin Fire Weapon");
			pSoldier->sTargetGridNo    = SBeginFireWeapon.sTargetGridNo;
			pSoldier->bTargetLevel     = SBeginFireWeapon.bTargetLevel;
			pSoldier->bTargetCubeLevel = SBeginFireWeapon.bTargetCubeLevel;
			EVENT_FireSoldierWeapon(pSoldier, SBeginFireWeapon.sTargetGridNo);
		},
		[](EV_S_FIREWEAPON const& SFireWeapon)
		{
			SOLDIERTYPE* pSoldier = GetSoldier(SFireWeapon.usSoldierID);
			if (pSoldier == NULL)
			{
				// Handle Error?
				SLOGE("Invalid Soldier ID");
				return;
			}

			// check for error
			if (pSoldier->uiUniqueSoldierIdValue != SFireWeapon.uiUniqueId)
			{
				return;
			}

			// Call soldier function
			SLOGD("FireWeapon");
			pSoldier->sTargetGridNo    = SFireWeapon.sTargetGridNo;
			pSoldier->bTargetLevel     = SFireWeapon.bTargetLevel;
			pSoldier->bTargetCubeLevel = SFireWeapon.bTargetCubeLevel;
			FireWeapon(pSoldier, SFireWeapon.sTargetGridNo);

		},
		[](EV_S_WEAPONHIT const& SWeaponHit)
		{
			SLOGD("WeaponHit {} Damage", SWeaponHit.sDamage);
			auto & s = GetMan(SWeaponHit.usSoldierID);
			WeaponHit(&s, SWeaponHit.usWeaponIndex, SWeaponHit.sDamage, SWeaponHit.sBreathLoss, SWeaponHit.usDirection,
			          SWeaponHit.sXPos, SWeaponHit.sYPos, SWeaponHit.sZPos, SWeaponHit.sRange, &GetMan(SWeaponHit.ubAttackerID),
			          SWeaponHit.ubSpecial, s.bAimShotLocation);
		},
		[](EV_S_NOISE const& SNoise)
		{
			SLOGD("Noise from {} at {}/{}, type {} volume {}",
						SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel,
						SNoise.ubNoiseType, SNoise.ubVolume);
			OurNoise(ID2SOLDIER(SNoise.ubNoiseMaker), SNoise.sGridNo, SNoise.bLevel, SNoise.ubVolume, static_cast<NoiseKind>(SNoise.ubNoiseType));
		}
	}, pEvent->gameEvent);
}


void ClearEventQueue(void)
{
	// clear out the event queue
	EventList& queue = GetQueue(EventQueueID::PRIMARY_EVENT_QUEUE);
	for (EVENT *pEvent : queue) delete pEvent;
	queue.clear();
}
