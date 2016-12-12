#ifndef __EVENT_MANAGER_H
#define __EVENT_MANAGER_H

#include "Types.h"

struct EVENT
{
	UINT32 TimeStamp;
	UINT32	uiFlags;
	UINT16	usDelay;
	UINT32	uiEvent;
	UINT32	uiDataSize;
	BYTE   Data[];
};

enum EventQueueID
{
	PRIMARY_EVENT_QUEUE,
	SECONDARY_EVENT_QUEUE,
	DEMAND_EVENT_QUEUE
};

#define			EVENT_EXPIRED						0x00000002

void InitializeEventManager(void);
void ShutdownEventManager(void);

void    AddEvent(UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT32 uiDataSize, EventQueueID);
EVENT*  RemoveEvent(UINT32 uiIndex, EventQueueID);
EVENT*  PeekEvent(UINT32 uiIndex, EventQueueID);
BOOLEAN FreeEvent(EVENT* pEvent);
UINT32	EventQueueSize(EventQueueID);

#endif
