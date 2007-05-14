#include "Event_Manager.h"
#include "SGP.h"
#include "WCheck.h"
#include "Event_Pump.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Weapons.h"
#include "Animation_Control.h"
#include "OppList.h"
#include "Tactical_Save.h"
#include "Debug.h"


// GLobals used here, for each event structure used,
// Used as globals for stack reasons
EV_S_GETNEWPATH				SGetNewPath;
EV_S_SETDESIREDDIRECTION			SSetDesiredDirection;
EV_S_BEGINFIREWEAPON	SBeginFireWeapon;
EV_S_FIREWEAPON				SFireWeapon;
EV_S_WEAPONHIT				SWeaponHit;
EV_S_NOISE						SNoise;


static BOOLEAN AddGameEventToQueue(UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT8 ubQueueID);


BOOLEAN AddGameEvent( UINT32 uiEvent, UINT16 usDelay, PTR pEventData )
{
	if (usDelay == DEMAND_EVENT_DELAY)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local #%d", uiEvent));
		return( AddGameEventToQueue( uiEvent, 0, pEventData, DEMAND_EVENT_QUEUE ) );
	}
	else if (uiEvent < NUM_EVENTS)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local #%d", uiEvent));
		return( AddGameEventToQueue( uiEvent, usDelay, pEventData, PRIMARY_EVENT_QUEUE ) );
	}
	// There is an error with the event
	else
		return(FALSE);
}


static BOOLEAN AddGameEventToQueue(UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT8 ubQueueID)
{
	 UINT32		uiDataSize;

	 // Check range of Event ui
	 if ( uiEvent < 0 || uiEvent > NUM_EVENTS )
	 {
		  // Set debug message!
			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Unknown event type");
			return( FALSE );
	 }

	 // Switch on event type and set size accordingly
	 switch( uiEvent )
	 {
			case S_GETNEWPATH:

				uiDataSize = sizeof( EV_S_GETNEWPATH );
				break;

			case S_SETDESIREDDIRECTION:

				uiDataSize = sizeof( EV_S_SETDESIREDDIRECTION );
				break;

			case S_FIREWEAPON:

				uiDataSize = sizeof( EV_S_FIREWEAPON );
				break;

			case S_BEGINFIREWEAPON:

				uiDataSize = sizeof( EV_S_BEGINFIREWEAPON );
				//Delay this event
				break;

			case S_WEAPONHIT:

				uiDataSize = sizeof( EV_S_WEAPONHIT );
				break;

			case S_NOISE:
				uiDataSize = sizeof( EV_S_NOISE );
				break;

			default:

				// Set debug msg: unknown message!
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Event Type mismatch");
				return( FALSE );

	 }


	 CHECKF( AddEvent( uiEvent, usDelay,  pEventData, uiDataSize, ubQueueID ) );

	 // successful
	 return( TRUE );
}


static BOOLEAN ExecuteGameEvent(EVENT* pEvent);


BOOLEAN  DequeAllGameEvents( BOOLEAN fExecute )
{
	EVENT					*pEvent;
	UINT32				uiQueueSize, cnt;
	BOOLEAN				fCompleteLoop = FALSE;
	// First dequeue all primary events


	while( EventQueueSize( PRIMARY_EVENT_QUEUE ) > 0 )
	{
		// Get Event
		if ( RemoveEvent( &pEvent, 0, PRIMARY_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}

		if ( fExecute )
		{
			// Check if event has a delay and add to secondary queue if so
			if ( pEvent->usDelay > 0 )
			{
				AddGameEventToQueue( pEvent->uiEvent, pEvent->usDelay, pEvent->pData, SECONDARY_EVENT_QUEUE );
			}
			else
			{
				ExecuteGameEvent( pEvent );
			}
		}

		// Delete event
		FreeEvent( pEvent );

	};

	// NOW CHECK SECONDARY QUEUE FOR ANY EXPRIED EVENTS
	// Get size of queue
	uiQueueSize = EventQueueSize( SECONDARY_EVENT_QUEUE );

	for ( cnt = 0; cnt < uiQueueSize; cnt++ )
	{
		if ( PeekEvent( &pEvent, cnt, SECONDARY_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}

		// Check time
		if ( ( GetJA2Clock() - pEvent->TimeStamp ) > pEvent->usDelay )
		{
			if ( fExecute )
			{
				ExecuteGameEvent( pEvent );
			}

			// FLag as expired
			pEvent->uiFlags = EVENT_EXPIRED;

		}

	}

	do
	{
		uiQueueSize = EventQueueSize( SECONDARY_EVENT_QUEUE );

		for ( cnt = 0; cnt < uiQueueSize; cnt++ )
		{
			if ( PeekEvent( &pEvent, cnt, SECONDARY_EVENT_QUEUE) == FALSE )
			{
				return( FALSE );
			}

			// Check time
			if ( pEvent->uiFlags & EVENT_EXPIRED )
			{
				RemoveEvent( &pEvent, cnt, SECONDARY_EVENT_QUEUE );
				FreeEvent( pEvent );
				// Restart loop
				break;

			}

		}

		if ( cnt == uiQueueSize )
		{
			fCompleteLoop = TRUE;
		}

	} while( fCompleteLoop == FALSE );

	return( TRUE );
}

BOOLEAN DequeueAllDemandGameEvents( BOOLEAN fExecute )
{
	EVENT					*pEvent;

	// Dequeue all events on the demand queue (only)

	while( EventQueueSize( DEMAND_EVENT_QUEUE ) > 0 )
	{
		// Get Event
		if ( RemoveEvent( &pEvent, 0, DEMAND_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}

		if ( fExecute )
		{
			// Check if event has a delay and add to secondary queue if so
			if ( pEvent->usDelay > 0 )
			{
				AddGameEventToQueue( pEvent->uiEvent, pEvent->usDelay, pEvent->pData, SECONDARY_EVENT_QUEUE );
			}
			else
			{
				ExecuteGameEvent( pEvent );
			}
		}

		// Delete event
		FreeEvent( pEvent );

	};

	return( TRUE );
}


static BOOLEAN ExecuteGameEvent(EVENT* pEvent)
{
	// Switch on event type
	switch( pEvent->uiEvent )
	{
			case S_GETNEWPATH:
			{
				memcpy( &SGetNewPath, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SGetNewPath.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SGetNewPath.uiUniqueId )
				{
					break;
				}
				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: GetNewPath");
				EVENT_GetNewSoldierPath( pSoldier, SGetNewPath.sDestGridNo, SGetNewPath.usMovementAnim );
				break;
			}

			case S_SETDESIREDDIRECTION:
			{
				memcpy( &SSetDesiredDirection, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SSetDesiredDirection.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SSetDesiredDirection.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: SetDesiredDirection: Dir( %d )", SSetDesiredDirection.usDesiredDirection)  );
				EVENT_SetSoldierDesiredDirection( pSoldier, SSetDesiredDirection.usDesiredDirection );
				break;
			}

			case S_BEGINFIREWEAPON:
			{
				memcpy( &SBeginFireWeapon, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SBeginFireWeapon.usSoldierID);
				if (pSoldier == NULL)
				{
					 pSoldier = NULL;
					 break;
					 // Handle Error?
					 // DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SBeginFireWeapon.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Begin Fire Weapon");
				pSoldier->sTargetGridNo = SBeginFireWeapon.sTargetGridNo;
				pSoldier->bTargetLevel = SBeginFireWeapon.bTargetLevel;
				pSoldier->bTargetCubeLevel = SBeginFireWeapon.bTargetCubeLevel;
				EVENT_FireSoldierWeapon( pSoldier, SBeginFireWeapon.sTargetGridNo );
				break;
			}

			case S_FIREWEAPON:
			{
				memcpy( &SFireWeapon, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SFireWeapon.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

					// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SFireWeapon.uiUniqueId )
				{
					break;
				}


				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: FireWeapon");
				pSoldier->sTargetGridNo = SFireWeapon.sTargetGridNo;
				pSoldier->bTargetLevel = SFireWeapon.bTargetLevel;
				pSoldier->bTargetCubeLevel = SFireWeapon.bTargetCubeLevel;
				FireWeapon( pSoldier, SFireWeapon.sTargetGridNo  );
				break;
			}

			case S_WEAPONHIT:

				memcpy( &SWeaponHit, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: WeaponHit %d Damage", SWeaponHit.sDamage ) );
				WeaponHit(SWeaponHit.usSoldierID, SWeaponHit.usWeaponIndex, SWeaponHit.sDamage, SWeaponHit.sBreathLoss, SWeaponHit.usDirection, SWeaponHit.sXPos, SWeaponHit.sYPos, SWeaponHit.sZPos, SWeaponHit.sRange, SWeaponHit.ubAttackerID, SWeaponHit.ubSpecial, SWeaponHit.ubLocation);
				break;

			case S_NOISE:
				memcpy( &SNoise, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: Noise from %d at %d/%d, type %d volume %d", SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel, SNoise.ubNoiseType, SNoise.ubVolume ) );
				OurNoise( SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel, SNoise.ubTerrType, SNoise.ubVolume, SNoise.ubNoiseType );
				break;

			default:

				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Event Received");
				return( FALSE );

	}

	return( TRUE );
}


BOOLEAN ClearEventQueue( void )
{
	// clear out the event queue
	EVENT					*pEvent;
	while( EventQueueSize( PRIMARY_EVENT_QUEUE ) > 0 )
	{
		// Get Event
		if ( RemoveEvent( &pEvent, 0, PRIMARY_EVENT_QUEUE) == FALSE )
		{
			return( FALSE );
		}
	}

	return( TRUE );
}
