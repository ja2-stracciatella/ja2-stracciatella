#include "Container.h"
#include "WCheck.h"
#include "Event_Manager.h"
#include "Timer_Control.h"
#include "Debug.h"
#include "MemMan.h"


typedef SGP::List<EVENT*> EventList;


static EventList* hEventQueue       = NULL;
static EventList* hDelayEventQueue  = NULL;
static EventList* hDemandEventQueue = NULL;


#define QUEUE_RESIZE		20


BOOLEAN InitializeEventManager(void)
{
	try
	{
		hEventQueue       = new EventList(QUEUE_RESIZE);
		hDelayEventQueue  = new EventList(QUEUE_RESIZE);
		/* Events on this queue are only processed when specifically called for by
		 * code */
		hDemandEventQueue = new EventList(QUEUE_RESIZE);
		return TRUE;
	}
	catch (const std::exception&)
	{
		return FALSE;
	}
}


void ShutdownEventManager(void)
{
	delete hEventQueue;
	delete hDelayEventQueue;
	delete hDemandEventQueue;
}


static EventList* GetQueue(UINT8 ubQueueID);


BOOLEAN AddEvent(UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT32 uiDataSize, UINT8 ubQueueID)
{
	EVENT* pEvent = MALLOCE(EVENT, uiDataSize);
	CHECKF(pEvent != NULL);

	// Set values
	pEvent->TimeStamp  = GetJA2Clock();
	pEvent->usDelay    = usDelay;
	pEvent->uiEvent    = uiEvent;
	pEvent->uiFlags    = 0;
	pEvent->uiDataSize = uiDataSize;
	memcpy(pEvent->Data, pEventData, uiDataSize);

	// Add event to queue
	EventList* const hQueue = GetQueue(ubQueueID);
	hQueue->Add(pEvent, hQueue->Size());
	return TRUE;
}


EVENT* RemoveEvent(UINT32 uiIndex, UINT8 ubQueueID)
{
	try
	{
		return GetQueue(ubQueueID)->Remove(uiIndex);
	}
	catch (const std::exception&)
	{
		return 0;
	}
}


EVENT* PeekEvent(UINT32 uiIndex, UINT8 ubQueueID)
{
	try
	{
		return GetQueue(ubQueueID)->Peek(uiIndex);
	}
	catch (const std::exception&)
	{
		return 0;
	}
}



BOOLEAN FreeEvent(EVENT* pEvent)
{
	CHECKF(pEvent != NULL);
	MemFree(pEvent);
	return TRUE;
}


UINT32 EventQueueSize(UINT8 ubQueueID)
{
	return GetQueue(ubQueueID)->Size();
}


static EventList* GetQueue(UINT8 ubQueueID)
{
	switch (ubQueueID)
	{
		case PRIMARY_EVENT_QUEUE:   return hEventQueue;
		case SECONDARY_EVENT_QUEUE: return hDelayEventQueue;
		case DEMAND_EVENT_QUEUE:    return hDemandEventQueue;

		default:
			Assert(FALSE);
			return NULL;
	}
}
