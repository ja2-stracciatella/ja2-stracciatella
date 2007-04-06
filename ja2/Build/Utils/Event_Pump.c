#include <time.h>
#include "SGP.h"
#include "WCheck.h"
#include "Event_Pump.h"
#include "Soldier_Control.h"
#include "Sound_Control.h"
#include "Overhead.h"
#include "Weapons.h"
#include "Animation_Control.h"
#include "OppList.h"
#include "Tactical_Save.h"
#include "Debug.h"

#ifdef NETWORKED
#	include "Networking.h"
#	include "NetworkEvent.h"
#endif


// GLobals used here, for each event structure used,
// Used as globals for stack reasons
EV_E_PLAYSOUND			EPlaySound;

EV_S_CHANGESTATE			SChangeState;
EV_S_CHANGEDEST				SChangeDest;
EV_S_SETPOSITION			SSetPosition;
EV_S_GETNEWPATH				SGetNewPath;
EV_S_BEGINTURN				SBeginTurn;
EV_S_CHANGESTANCE			SChangeStance;
EV_S_SETDIRECTION			SSetDirection;
EV_S_SETDESIREDDIRECTION			SSetDesiredDirection;
EV_S_BEGINFIREWEAPON	SBeginFireWeapon;
EV_S_FIREWEAPON				SFireWeapon;
EV_S_WEAPONHIT				SWeaponHit;
EV_S_STRUCTUREHIT			SStructureHit;
EV_S_WINDOWHIT				SWindowHit;
EV_S_MISS							SMiss;
EV_S_NOISE						SNoise;
EV_S_STOP_MERC				SStopMerc;
EV_S_SENDPATHTONETWORK SUpdateNetworkSoldier;

extern	BOOLEAN				gfAmINetworked;

static BOOLEAN AddGameEventToQueue(UINT32 uiEvent, UINT16 usDelay, PTR pEventData, UINT8 ubQueueID);


BOOLEAN AddGameEvent( UINT32 uiEvent, UINT16 usDelay, PTR pEventData )
{
	if (usDelay == DEMAND_EVENT_DELAY)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local and network #%d", uiEvent));
		#ifdef NETWORKED
		if(gfAmINetworked)
			SendEventToNetwork(uiEvent, usDelay, pEventData);
		#endif
		return( AddGameEventToQueue( uiEvent, 0, pEventData, DEMAND_EVENT_QUEUE ) );
	}
	else if( uiEvent < EVENTS_LOCAL_AND_NETWORK)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local and network #%d", uiEvent));
		#ifdef NETWORKED
		if(gfAmINetworked)
			SendEventToNetwork(uiEvent, usDelay, pEventData);
		#endif
		return( AddGameEventToQueue( uiEvent, usDelay, pEventData, PRIMARY_EVENT_QUEUE ) );
	}
	else if( uiEvent < EVENTS_ONLY_USED_LOCALLY)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending Local #%d", uiEvent));
		return( AddGameEventToQueue( uiEvent, usDelay, pEventData, PRIMARY_EVENT_QUEUE ) );
	}
	else if( uiEvent < EVENTS_ONLY_SENT_OVER_NETWORK)
	{
		//DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("AddGameEvent: Sending network #%d", uiEvent));
		#ifdef NETWORKED
		if(gfAmINetworked)
			SendEventToNetwork(uiEvent, usDelay, pEventData);
		#endif
		return(TRUE);
	}
	// There is an error with the event
	else
		return(FALSE);
}


static BOOLEAN AddGameEventFromNetwork(UINT32 uiEvent, UINT16 usDelay, PTR pEventData)
{
		return( AddGameEventToQueue( uiEvent, usDelay, pEventData, PRIMARY_EVENT_QUEUE ) );
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
			case E_PLAYSOUND:

				uiDataSize = sizeof( EV_E_PLAYSOUND );
				break;

			case S_CHANGESTATE:

				uiDataSize = sizeof( EV_S_CHANGESTATE );
				break;


			case S_CHANGEDEST:

				uiDataSize = sizeof( EV_S_CHANGEDEST );
				break;


			case S_SETPOSITION:

				uiDataSize = sizeof( EV_S_SETPOSITION );
				break;

			case S_GETNEWPATH:

				uiDataSize = sizeof( EV_S_GETNEWPATH );
				break;

			case S_BEGINTURN:

				uiDataSize = sizeof( EV_S_BEGINTURN );
				break;

			case S_CHANGESTANCE:

				uiDataSize = sizeof( EV_S_CHANGESTANCE );
				break;

			case S_SETDIRECTION:

				uiDataSize = sizeof( EV_S_SETDIRECTION );
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

			case S_STRUCTUREHIT:
				uiDataSize = sizeof( EV_S_STRUCTUREHIT );
				break;

			case S_WINDOWHIT:
				uiDataSize = sizeof( EV_S_STRUCTUREHIT );
				break;

			case S_MISS:
				uiDataSize = sizeof( EV_S_MISS );
				break;

			case S_NOISE:
				uiDataSize = sizeof( EV_S_NOISE );
				break;

			case S_STOP_MERC:
				uiDataSize = sizeof( EV_S_STOP_MERC );
				break;

			case S_SENDPATHTONETWORK:
				uiDataSize = sizeof(EV_S_SENDPATHTONETWORK);
				break;

			case 	S_UPDATENETWORKSOLDIER:
				uiDataSize = sizeof(EV_S_UPDATENETWORKSOLDIER);
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
			case E_PLAYSOUND:

				memcpy( &EPlaySound, pEvent->pData, pEvent->uiDataSize );

				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Play Sound");
				PlayJA2Sample(EPlaySound.usIndex, EPlaySound.ubVolume, EPlaySound.ubLoops, EPlaySound.uiPan);
				break;

			case S_CHANGESTATE:
			{
				memcpy( &SChangeState, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SChangeState.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SChangeState.uiUniqueId )
				{
					break;
				}

				// Call soldier function
//				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Event Pump: ChangeState %S (%d)", gAnimControl[ SChangeState.ubNewState ].zAnimStr, SChangeState.usSoldierID ) );
				EVENT_InitNewSoldierAnim( pSoldier, SChangeState.usNewState, SChangeState.usStartingAniCode, SChangeState.fForce );
				break;
			}

			case S_CHANGEDEST:
			{
				memcpy( &SChangeDest, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SChangeDest.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Event Pump: Invalid Soldier ID #%d", SChangeDest.usSoldierID) );
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SChangeDest.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Change Dest");
				EVENT_SetSoldierDestination( pSoldier, SChangeDest.usNewDestination );
				break;
			}

			case S_SETPOSITION:
			{
				memcpy( &SSetPosition, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SSetPosition.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SSetPosition.uiUniqueId )
				{
					break;
				}

				// Call soldier function
//				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: SetPosition ( %f %f ) ( %d )", SSetPosition.dNewXPos, SSetPosition.dNewYPos, SSetPosition.usSoldierID ) );
				EVENT_SetSoldierPosition( pSoldier, SSetPosition.dNewXPos, SSetPosition.dNewYPos );
				break;
			}

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

			case S_BEGINTURN:
			{
				memcpy( &SBeginTurn, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SBeginTurn.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SBeginTurn.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: BeginTurn");
				EVENT_BeginMercTurn( pSoldier, FALSE, 0 );
				break;
			}

			case S_CHANGESTANCE:
			{
				memcpy( &SChangeStance, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SChangeStance.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SChangeStance.uiUniqueId )
				{
					break;
				}
				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: ChangeStance");
				ChangeSoldierStance( pSoldier, SChangeStance.ubNewStance );
				break;
			}

			case S_SETDIRECTION:
			{
				memcpy( &SSetDirection, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SSetDirection.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				// check for error
				if( pSoldier-> uiUniqueSoldierIdValue != SSetDirection.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: SetDirection: Dir( %d )", SSetDirection.usNewDirection)  );
				EVENT_SetSoldierDirection( pSoldier, SSetDirection.usNewDirection );
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
				WeaponHit( SWeaponHit.usSoldierID, SWeaponHit.usWeaponIndex, SWeaponHit.sDamage, SWeaponHit.sBreathLoss, SWeaponHit.usDirection, SWeaponHit.sXPos, SWeaponHit.sYPos, SWeaponHit.sZPos, SWeaponHit.sRange, SWeaponHit.ubAttackerID, SWeaponHit.fHit, SWeaponHit.ubSpecial, SWeaponHit.ubLocation );
				break;

			case S_STRUCTUREHIT:

				memcpy( &SStructureHit, pEvent->pData, pEvent->uiDataSize );
				DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Event Pump: StructureHit");
				StructureHit( SStructureHit.iBullet, SStructureHit.usWeaponIndex, SStructureHit.bWeaponStatus, SStructureHit.ubAttackerID, SStructureHit.sXPos, SStructureHit.sYPos, SStructureHit.sZPos, SStructureHit.usStructureID, SStructureHit.iImpact, TRUE );
				break;

			case S_WINDOWHIT:

				memcpy( &SWindowHit, pEvent->pData, pEvent->uiDataSize );
				DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Event Pump: WindowHit");
				WindowHit( SWindowHit.sGridNo, SWindowHit.usStructureID, SWindowHit.fBlowWindowSouth, SWindowHit.fLargeForce );
				break;

			case S_MISS:

				memcpy( &SMiss, pEvent->pData, pEvent->uiDataSize );
				DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Shot Miss ( obsolete )");
				//ShotMiss( SMiss.ubAttackerID );
				break;

			case S_NOISE:
				memcpy( &SNoise, pEvent->pData, pEvent->uiDataSize );
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String( "Event Pump: Noise from %d at %d/%d, type %d volume %d", SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel, SNoise.ubNoiseType, SNoise.ubVolume ) );
				OurNoise( SNoise.ubNoiseMaker, SNoise.sGridNo, SNoise.bLevel, SNoise.ubTerrType, SNoise.ubVolume, SNoise.ubNoiseType );
				break;

			case S_STOP_MERC:
			{
				memcpy( &SStopMerc, pEvent->pData, pEvent->uiDataSize );

				SOLDIERTYPE* pSoldier = GetSoldier(SStopMerc.usSoldierID);
				if (pSoldier == NULL)
				{
					 // Handle Error?
					 DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Event Pump: Invalid Soldier ID");
					 break;
				}

				if( pSoldier-> uiUniqueSoldierIdValue != SStopMerc.uiUniqueId )
				{
					break;
				}

				// Call soldier function
				DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Event Pump: Stop Merc at Gridno %d", SStopMerc.sGridNo ));
				EVENT_StopMerc( pSoldier, SStopMerc.sGridNo, SStopMerc.bDirection );
				break;
			}

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
