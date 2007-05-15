#include "SGP.h"
#include "Container.h"
#include "WCheck.h"
#include "Event_Manager.h"
#include "Timer_Control.h"
#include "Debug.h"
#include "MemMan.h"


static HLIST hEventQueue       = NULL;
static HLIST hDelayEventQueue  = NULL;
static HLIST hDemandEventQueue = NULL;


#define QUEUE_RESIZE		20


BOOLEAN InitializeEventManager(void)
{
	// Create Queue
	hEventQueue = CreateList(QUEUE_RESIZE, sizeof(EVENT*));
	if (hEventQueue == NULL) return FALSE;

	// Create Delay Queue
	hDelayEventQueue = CreateList(QUEUE_RESIZE, sizeof(EVENT*));
	if (hDelayEventQueue == NULL) return FALSE;

	// Create Demand Queue (events on this queue are only processed when specifically
	// called for by code)
	hDemandEventQueue = CreateList(QUEUE_RESIZE, sizeof(EVENT*));
	if (hDemandEventQueue == NULL) return FALSE;

	return TRUE;
}


BOOLEAN ShutdownEventManager(void)
{
	if (hEventQueue       != NULL) DeleteList(hEventQueue);
	if (hDelayEventQueue  != NULL) DeleteList(hDelayEventQueue);
	if (hDemandEventQueue != NULL) DeleteList(hDemandEventQueue);
	return TRUE;
}


static HLIST GetQueue(UINT8 ubQueueID);
static void SetQueue(UINT8 ubQueueID, HQUEUE hQueue);


BOOLEAN AddEvent(UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT32 uiDataSize, UINT8 ubQueueID)
{
	EVENT* pEvent = MemAlloc(sizeof(*pEvent) + uiDataSize);
	CHECKF(pEvent != NULL);

	// Set values
	pEvent->TimeStamp  = GetJA2Clock();
	pEvent->usDelay    = usDelay;
	pEvent->uiEvent    = uiEvent;
	pEvent->uiFlags    = 0;
	pEvent->uiDataSize = uiDataSize;
	memcpy(pEvent->Data, pEventData, uiDataSize);

	// Add event to queue
	HLIST hQueue = GetQueue(ubQueueID);
	hQueue = AddtoList(hQueue, &pEvent, ListSize(hQueue));
	SetQueue(ubQueueID, hQueue);

	return TRUE;
}


EVENT* RemoveEvent(UINT32 uiIndex, UINT8 ubQueueID)
{
	// Get an event from queue, if one exists
	HLIST hQueue = GetQueue(ubQueueID);

	EVENT* Event;
	CHECKN(RemfromList(hQueue, &Event, uiIndex));
	return Event;
}


EVENT* PeekEvent(UINT32 uiIndex, UINT8 ubQueueID)
{
	// Get an event from queue, if one exists
	HLIST hQueue = GetQueue(ubQueueID);

	EVENT* Event;
	CHECKN(PeekList(hQueue, &Event, uiIndex));
	return Event;
}



BOOLEAN FreeEvent(EVENT* pEvent)
{
	CHECKF(pEvent != NULL);
	MemFree(pEvent);
	return TRUE;
}


UINT32 EventQueueSize(UINT8 ubQueueID)
{
	HLIST hQueue = GetQueue(ubQueueID);
	UINT32 uiQueueSize = ListSize(hQueue);
	return uiQueueSize;
}


static HLIST GetQueue(UINT8 ubQueueID)
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


static void SetQueue(UINT8 ubQueueID, HQUEUE hQueue)
{
	switch (ubQueueID)
	{
		case PRIMARY_EVENT_QUEUE:   hEventQueue       = hQueue; break;
		case SECONDARY_EVENT_QUEUE: hDelayEventQueue  = hQueue; break;
		case DEMAND_EVENT_QUEUE:    hDemandEventQueue = hQueue; break;

		default:
			Assert(FALSE);
			break;
	}
}
