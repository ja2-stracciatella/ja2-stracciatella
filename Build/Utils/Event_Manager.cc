#include "Container.h"
#include "WCheck.h"
#include "Event_Manager.h"
#include "Timer_Control.h"
#include "MemMan.h"


typedef SGP::List<EVENT*> EventList;


static EventList* hEventQueue       = NULL;
static EventList* hDelayEventQueue  = NULL;
static EventList* hDemandEventQueue = NULL;


#define QUEUE_RESIZE		20


void InitializeEventManager(void)
{
	hEventQueue       = new EventList(QUEUE_RESIZE);
	hDelayEventQueue  = new EventList(QUEUE_RESIZE);
	/* Events on this queue are only processed when specifically called for by
	 * code */
	hDemandEventQueue = new EventList(QUEUE_RESIZE);
}


void ShutdownEventManager(void)
{
	delete hEventQueue;
	delete hDelayEventQueue;
	delete hDemandEventQueue;
}


static EventList* GetQueue(EventQueueID ubQueueID);


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
	EventList* const hQueue = GetQueue(ubQueueID);
	hQueue->Add(pEvent, hQueue->Size());
}


EVENT* RemoveEvent(UINT32 uiIndex, EventQueueID ubQueueID)
try
{
	return GetQueue(ubQueueID)->Remove(uiIndex);
}
catch (const std::exception&)
{
	return 0;
}


EVENT* PeekEvent(UINT32 uiIndex, EventQueueID ubQueueID)
try
{
	return GetQueue(ubQueueID)->Peek(uiIndex);
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
	return (UINT32)GetQueue(ubQueueID)->Size();
}


static EventList* GetQueue(EventQueueID const ubQueueID)
{
	switch (ubQueueID)
	{
		case PRIMARY_EVENT_QUEUE:   return hEventQueue;
		case SECONDARY_EVENT_QUEUE: return hDelayEventQueue;
		case DEMAND_EVENT_QUEUE:    return hDemandEventQueue;

		default: throw std::logic_error("Tried to get non-existent event queue");
	}
}
