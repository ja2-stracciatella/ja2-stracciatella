#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "SGP.h"
#include "Container.h"
#include "WCheck.h"
#include "Event_Manager.h"
#include "Timer_Control.h"
#include <string.h>
#include "Debug.h"
#include "MemMan.h"


HLIST		hEventQueue = NULL;
HLIST		hDelayEventQueue = NULL;
HLIST		hDemandEventQueue = NULL;

#define QUEUE_RESIZE		20

// LOCAL FUNCTIONS
HLIST GetQueue( UINT8 ubQueueID );
void SetQueue( UINT8 ubQueueID, HLIST hQueue );


BOOLEAN InitializeEventManager( )
{
	// Create Queue
	hEventQueue = CreateList( QUEUE_RESIZE, sizeof( PTR ) );

	if ( hEventQueue == NULL )
	{
		return( FALSE );
	}

	// Create Delay Queue
	hDelayEventQueue = CreateList( QUEUE_RESIZE, sizeof( PTR ) );

	if ( hDelayEventQueue == NULL )
	{
		return( FALSE );
	}

	// Create Demand Queue (events on this queue are only processed when specifically
	// called for by code)
	hDemandEventQueue = CreateList( QUEUE_RESIZE, sizeof( PTR ) );

	if ( hDemandEventQueue == NULL )
	{
		return( FALSE );
	}

	return( TRUE );
}

BOOLEAN ShutdownEventManager( )
{
	if ( hEventQueue != NULL )
	{
			DeleteList( hEventQueue );
	}

	if ( hDelayEventQueue != NULL )
	{
			DeleteList( hDelayEventQueue );
	}

	if ( hDemandEventQueue != NULL )
	{
			DeleteList( hDemandEventQueue );
	}

	return( TRUE );
}



BOOLEAN AddEvent( UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT32 uiDataSize, UINT8 ubQueueID )
{
	EVENT *pEvent;
	UINT32 uiEventSize = sizeof( EVENT );
	HLIST	hQueue;

	// Allocate new event
	pEvent = MemAlloc( uiEventSize + uiDataSize );

	CHECKF( pEvent != NULL );

	// Set values
	pEvent->TimeStamp  = GetJA2Clock( );
	pEvent->usDelay		 = usDelay;
	pEvent->uiEvent		 = uiEvent;
	pEvent->uiFlags		 = 0;
	pEvent->uiDataSize = uiDataSize;
	pEvent->pData			 = (BYTE*)pEvent;
	pEvent->pData			 = pEvent->pData + uiEventSize;

	memcpy( pEvent->pData, pEventData, uiDataSize );

	// Add event to queue
	hQueue = GetQueue( ubQueueID );
	hQueue = AddtoList( hQueue, &pEvent, ListSize( hQueue ) );
	SetQueue( ubQueueID, hQueue );

	return( TRUE );

}



BOOLEAN RemoveEvent( EVENT **ppEvent, UINT32 uiIndex, UINT8 ubQueueID )
{
	UINT32 uiQueueSize;
	HLIST hQueue;

	// Get an event from queue, if one exists
	//

	hQueue = GetQueue( ubQueueID );


	// Get Size
	uiQueueSize = ListSize( hQueue );

	if ( uiQueueSize > 0 )
	{
		// Get
		CHECKF( RemfromList( hQueue , ppEvent, uiIndex ) != FALSE );
	}
	else
	{
		return( FALSE );
	}

	return( TRUE );
}


BOOLEAN PeekEvent( EVENT **ppEvent, UINT32 uiIndex , UINT8 ubQueueID )
{
	UINT32 uiQueueSize;
	HLIST hQueue;

	// Get an event from queue, if one exists
	//

	hQueue = GetQueue( ubQueueID );


	// Get Size
	uiQueueSize = ListSize( hQueue );

	if ( uiQueueSize > 0 )
	{
		// Get
		CHECKF( PeekList( hQueue, ppEvent, uiIndex ) != FALSE );
	}
	else
	{
		return( FALSE );
	}

	return( TRUE );
}



BOOLEAN FreeEvent( EVENT *pEvent )
{
	CHECKF( pEvent != NULL );

	// Delete event
	MemFree( pEvent );

	return( TRUE );
}


UINT32 EventQueueSize( UINT8 ubQueueID )
{
	UINT32 uiQueueSize;
	HLIST hQueue;

	// Get an event from queue, if one exists
	//

	hQueue = GetQueue( ubQueueID );

	// Get Size
	uiQueueSize = ListSize( hQueue );

	return( uiQueueSize );
}


HLIST GetQueue( UINT8 ubQueueID )
{
	switch( ubQueueID )
	{
		case PRIMARY_EVENT_QUEUE:
			return( hEventQueue );
			break;

		case SECONDARY_EVENT_QUEUE:
			return( hDelayEventQueue );
			break;

		case DEMAND_EVENT_QUEUE:
			return( hDemandEventQueue );
			break;

		default:
			Assert( FALSE );
			return( 0 );
			break;
	}

}

void SetQueue( UINT8 ubQueueID, HQUEUE hQueue )
{
	switch( ubQueueID )
	{
		case PRIMARY_EVENT_QUEUE:
			hEventQueue = hQueue;
			break;

		case SECONDARY_EVENT_QUEUE:
			hDelayEventQueue = hQueue;
			break;

		case DEMAND_EVENT_QUEUE:
			hDemandEventQueue = hQueue;
			break;

		default:
			Assert( FALSE );
			break;
	}
}
