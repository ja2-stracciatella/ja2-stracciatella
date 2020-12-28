#include "Event_Manager.h"
#include "Timer_Control.h"
#include <stdexcept>
#include <vector>


typedef std::vector<EVENT*>EventList;

static EventList hEventQueue;
static EventList hDelayEventQueue;
static EventList hDemandEventQueue;


void InitializeEventManager(void)
{
}


void ShutdownEventManager(void)
{
	hEventQueue.clear();
	hDelayEventQueue.clear();
	hDemandEventQueue.clear();
}


static EventList& GetQueue(EventQueueID ubQueueID);


void AddEvent(GAMEEVENT const& gameEvent, UINT16 const usDelay, EventQueueID const ubQueueID)
{
	EVENT* pEvent = new EVENT{};
	pEvent->TimeStamp  = GetJA2Clock();
	pEvent->usDelay    = usDelay;
	pEvent->uiFlags    = 0;
	pEvent->gameEvent  = gameEvent;
	GetQueue(ubQueueID).push_back(pEvent);
}


EVENT* RemoveEvent(UINT32 uiIndex, EventQueueID ubQueueID)
try
{
	EventList& queue = GetQueue(ubQueueID);
	EVENT* ret = queue[uiIndex];
	queue.erase(queue.begin() + uiIndex);
	return ret;
}
catch (const std::exception&)
{
	return 0;
}


EVENT* PeekEvent(UINT32 uiIndex, EventQueueID ubQueueID)
try
{
	return GetQueue(ubQueueID)[uiIndex];
}
catch (const std::exception&)
{
	return 0;
}


BOOLEAN FreeEvent(EVENT* pEvent)
{
	if (!pEvent) return FALSE;
	delete pEvent;
	return TRUE;
}


UINT32 EventQueueSize(EventQueueID ubQueueID)
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
}
