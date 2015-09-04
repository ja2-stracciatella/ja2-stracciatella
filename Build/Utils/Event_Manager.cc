#include <stdexcept>
#include <vector>

#include "WCheck.h"
#include "Event_Manager.h"
#include "Timer_Control.h"
#include "MemMan.h"

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


void AddEvent(UINT32 const uiEvent, UINT16 const usDelay, PTR const pEventData, UINT32 const uiDataSize, EventQueueID const ubQueueID)
{
	EVENT* pEvent = MALLOCE(EVENT, Data, uiDataSize);
	pEvent->TimeStamp  = GetJA2Clock();
	pEvent->usDelay    = usDelay;
	pEvent->uiEvent    = uiEvent;
	pEvent->uiFlags    = 0;
	pEvent->uiDataSize = uiDataSize;
	memcpy(pEvent->Data, pEventData, uiDataSize);

	// Add event to queue
	GetQueue(ubQueueID).push_back(pEvent);
	MemFree(pEvent);
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
	CHECKF(pEvent != NULL);
	MemFree(pEvent);
	return TRUE;
}


UINT32 EventQueueSize(EventQueueID ubQueueID)
{
	return (UINT32)GetQueue(ubQueueID).size();
}


static EventList& GetQueue(EventQueueID const ubQueueID)
{
	switch (ubQueueID)
	{
		case PRIMARY_EVENT_QUEUE:   return hEventQueue;
		case SECONDARY_EVENT_QUEUE: return hDelayEventQueue;
		case DEMAND_EVENT_QUEUE:    return hDemandEventQueue;

		default: throw std::logic_error("Tried to get non-existent event queue");
	}
}
