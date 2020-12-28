#ifndef __EVENT_MANAGER_H
#define __EVENT_MANAGER_H

#include "Event_Pump.h"

struct EVENT
{
	UINT32 TimeStamp;
	UINT32	uiFlags;
	UINT16	usDelay;
	GAMEEVENT gameEvent;
};

enum class EventQueueID
{
	PRIMARY_EVENT_QUEUE,
	SECONDARY_EVENT_QUEUE,
	DEMAND_EVENT_QUEUE
};

constexpr UINT32 EVENT_EXPIRED = 0x00000002;

void InitializeEventManager(void);
void ShutdownEventManager(void);

void    AddEvent(GAMEEVENT const& gameEvent, UINT16 usDelay, EventQueueID);
EVENT*  RemoveEvent(UINT32 uiIndex, EventQueueID);
EVENT*  PeekEvent(UINT32 uiIndex, EventQueueID);
BOOLEAN FreeEvent(EVENT* pEvent);
UINT32	EventQueueSize(EventQueueID);

#endif
