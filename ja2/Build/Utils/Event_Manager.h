#ifndef __EVENT_MANAGER_H
#define __EVENT_MANAGER_H

typedef struct
{
	TIMER		TimeStamp;
	UINT32	uiFlags;
	UINT16	usDelay;
	UINT32	uiEvent;
	UINT32	uiDataSize;
	BYTE		*pData;

} EVENT;

#define			PRIMARY_EVENT_QUEUE			0
#define			SECONDARY_EVENT_QUEUE		1
#define			DEMAND_EVENT_QUEUE			2

#define			EVENT_EXPIRED						0x00000002

// Management fucntions
BOOLEAN InitializeEventManager( );
BOOLEAN ShutdownEventManager( );

BOOLEAN AddEvent( UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT32 uiDataSize, UINT8 ubQueueID );
BOOLEAN RemoveEvent( EVENT **ppEvent, UINT32 uiIndex , UINT8 ubQueueID );
BOOLEAN PeekEvent( EVENT **ppEvent, UINT32 uiIndex , UINT8 ubQueueID );
BOOLEAN FreeEvent( EVENT *pEvent );
UINT32	EventQueueSize( UINT8 ubQueueID );


#endif

