#include "Debug.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "Game_Events.h"
#include "Game_Clock.h"
#include "MemMan.h"
#include "Font_Control.h"
#include "Message.h"
#include "Text.h"
#include "FileMan.h"


#ifdef JA2TESTVERSION

static wchar_t const* const gEventName[] =
{
	L"Null",
	L"ChangeLightValue",
	L"WeatherStart",
	L"WeatherEnd",
	L"CheckForQuests",
	L"Ambient",
	L"AIMResetMercAnnoyance",
	L"BobbyRayPurchase",
	L"DailyUpdateBobbyRayInventory",
	L"UpdateBobbyRayInventory",
	L"DailyUpdateOfMercSite",
	L"Day3AddEMailFromSpeck",
	L"DelayedHiringOfMerc",
	L"HandleInsuredMercs",
	L"PayLifeInsuranceForDeadMerc",
	L"MercDailyUpdate",
	L"MercAboutToLeaveComment",
	L"MercContractOver",
	L"GroupArrival",
	L"Day2AddEMailFromIMP",
	L"MercComplainEquipment",
	L"HourlyUpdate",
	L"HandleMineIncome",
	L"SetupMineIncome",
	L"QueuedBattle",
	L"LeavingMercArriveInDrassen",
	L"LeavingMercArriveInOmerta",
	L"SetByNPCSystem",
	L"SecondAirportAttendantArrived",
	L"HelicopterHoverTooLong",
	L"HelicopterHoverWayTooLong",
	L"HelicopterDoneRefuelling",
	L"MercLeaveEquipInOmerta",
	L"MercLeaveEquipInDrassen",
	L"DailyEarlyMorningEvents",
	L"GroupAboutToArrive",
	L"ProcessTacticalSchedule",
	L"BeginRainStorm",
	L"EndRainStorm",
	L"HandleTownOpinion",
	L"SetupTownOpinion",
	L"DelayedDeathHandling",
	L"BeginAirRaid",
	L"TownLoyaltyUpdate",
	L"Meanwhile",
	L"BeginCreatureQuest",
	L"CreatureSpread",
	L"DecayCreatures",
	L"CreatureNightPlanning",
	L"CreatureAttack",
	L"EvaluateQueenSituation",
	L"CheckEnemyControlledSector",
	L"TurnOnNightLights",
	L"TurnOffNightLights",
	L"TurnOnPrimeLights",
	L"TurnOffPrimeLights",
	L"MercAboutToLeaveComment",
	L"ForceTimeInterupt",
	L"EnricoEmailEvent",
	L"InsuranceInvestigationStarted",
	L"InsuranceInvestigationOver",
	L"HandleMinuteUpdate",
	L"TemperatureUpdate",
	L"Keith going out of business",
	L"MERC site back online",
	L"Investigate Sector",
	L"CheckIfMineCleared",
	L"RemoveAssassin",
	L"BandageBleedingMercs",
	L"ShowUpdateMenu",
	L"SetMenuReason",
	L"AddSoldierToUpdateBox",
	L"BeginContractRenewalSequence",
	L"RPC_WHINE_ABOUT_PAY",
	L"HaventMadeImpCharacterEmail",
	L"Rainstorm",
	L"Quarter Hour Update",
	L"MERC Merc went up level email delay",
	L".",
};
CASSERT(lengthof(gEventName) == NUMBER_OF_EVENT_TYPES);

#endif

STRATEGICEVENT									*gpEventList = NULL;

extern UINT32 guiGameClock;
BOOLEAN gfPreventDeletionOfAnyEvent = FALSE;
static BOOLEAN gfEventDeletionPending = FALSE;

static BOOLEAN gfProcessingGameEvents = FALSE;
UINT32	guiTimeStampOfCurrentlyExecutingEvent = 0;


BOOLEAN GameEventsPending(const UINT32 uiAdjustment)
{
	if( !gpEventList )
		return FALSE;
	if( gpEventList->uiTimeStamp <= GetWorldTotalSeconds() + uiAdjustment )
		return TRUE;
	return FALSE;
}


static void DeleteEventsWithDeletionPending(void)
{
	if (!gfEventDeletionPending) return;
	gfEventDeletionPending = FALSE;

	STRATEGICEVENT** anchor = &gpEventList;
	for (STRATEGICEVENT* curr = gpEventList; curr;)
	{
		STRATEGICEVENT* const next = curr->next;
		if (curr->ubFlags & SEF_DELETION_PENDING)
		{
			*anchor = next;
			MemFree(curr);
		}
		else
		{
			anchor = &curr->next;
		}
		curr = next;
	}
}


static void AdjustClockToEventStamp(STRATEGICEVENT* pEvent, UINT32* puiAdjustment)
{
	UINT32 uiDiff;

	uiDiff = pEvent->uiTimeStamp - guiGameClock;
	guiGameClock += uiDiff;
	*puiAdjustment -= uiDiff;

	//Calculate the day, hour, and minutes.
	guiDay = ( guiGameClock / NUM_SEC_IN_DAY );
	guiHour = ( guiGameClock - ( guiDay * NUM_SEC_IN_DAY ) ) / NUM_SEC_IN_HOUR;
	guiMin	= ( guiGameClock - ( ( guiDay * NUM_SEC_IN_DAY ) + ( guiHour * NUM_SEC_IN_HOUR ) ) ) / NUM_SEC_IN_MIN;

	swprintf(WORLDTIMESTR, lengthof(WORLDTIMESTR), L"%ls %d, %02d:%02d", gpGameClockString, guiDay, guiHour, guiMin);
}


void ProcessPendingGameEvents(UINT32 uiAdjustment, const UINT8 ubWarpCode)
{
	STRATEGICEVENT *curr, *pEvent, *prev, *temp;
	BOOLEAN fDeleteEvent = FALSE, fDeleteQueuedEvent = FALSE;

	gfTimeInterrupt = FALSE;
	gfProcessingGameEvents = TRUE;

	//While we have events inside the time range to be updated, process them...
	curr = gpEventList;
	prev = NULL; //prev only used when warping time to target time.
	while( !gfTimeInterrupt && curr && curr->uiTimeStamp <= guiGameClock + uiAdjustment )
	{
		fDeleteEvent = FALSE;
		//Update the time by the difference, but ONLY if the event comes after the current time.
		//In the beginning of the game, series of events are created that are placed in the list
		//BEFORE the start time.  Those events will be processed without influencing the actual time.
		if( curr->uiTimeStamp > guiGameClock && ubWarpCode != WARPTIME_PROCESS_TARGET_TIME_FIRST )
		{
			AdjustClockToEventStamp( curr, &uiAdjustment );
		}
		//Process the event
		if( ubWarpCode != WARPTIME_PROCESS_TARGET_TIME_FIRST )
		{
			fDeleteEvent = ExecuteStrategicEvent( curr );
		}
		else if( curr->uiTimeStamp == guiGameClock + uiAdjustment )
		{ //if we are warping to the target time to process that event first,
			if( !curr->next || curr->next->uiTimeStamp > guiGameClock + uiAdjustment )
			{ //make sure that we are processing the last event for that second
				AdjustClockToEventStamp( curr, &uiAdjustment );

				fDeleteEvent = ExecuteStrategicEvent( curr );

				if( curr && prev && fDeleteQueuedEvent )
				{ //The only case where we are deleting a node in the middle of the list
					prev->next = curr->next;
				}
			}
			else
			{ //We are at the current target warp time however, there are still other events following in this time cycle.
				//We will only target the final event in this time.  NOTE:  Events are posted using a FIFO method
				prev = curr;
				curr = curr->next;
				continue;
			}
		}
		else
		{ //We are warping time to the target time.  We haven't found the event yet,
			//so continuing will keep processing the list until we find it.  NOTE:  Events are posted using a FIFO method
			prev = curr;
			curr = curr->next;
			continue;
		}
		if( fDeleteEvent )
		{
			//Determine if event node is a special event requiring reposting
			switch( curr->ubEventType )
			{
				case RANGED_EVENT:
					AddAdvancedStrategicEvent(ENDRANGED_EVENT, static_cast<StrategicEventKind>(curr->ubCallbackID), curr->uiTimeStamp + curr->uiTimeOffset, curr->uiParam);
					break;
				case PERIODIC_EVENT:
					pEvent = AddAdvancedStrategicEvent(PERIODIC_EVENT, static_cast<StrategicEventKind>(curr->ubCallbackID), curr->uiTimeStamp + curr->uiTimeOffset, curr->uiParam);
					if( pEvent )
						pEvent->uiTimeOffset = curr->uiTimeOffset;
					break;
				case EVERYDAY_EVENT:
					AddAdvancedStrategicEvent(EVERYDAY_EVENT, static_cast<StrategicEventKind>(curr->ubCallbackID), curr->uiTimeStamp + NUM_SEC_IN_DAY, curr->uiParam);
					break;
			}
			if( curr == gpEventList )
			{
				gpEventList = gpEventList->next;
				MemFree( curr );
				curr = gpEventList;
				prev = NULL;
				//ValidateGameEvents();
			}
			else
			{
				temp = curr;
				prev->next = curr->next;
				curr = curr->next;
				MemFree( temp );
				//ValidateGameEvents();
			}
		}
		else
		{
			prev = curr;
			curr = curr->next;
		}
	}

	gfProcessingGameEvents = FALSE;

	DeleteEventsWithDeletionPending();

	if( uiAdjustment && !gfTimeInterrupt )
		guiGameClock += uiAdjustment;
}


BOOLEAN AddSameDayStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiMinStamp, UINT32 const uiParam)
{
	return( AddStrategicEvent( ubCallbackID, uiMinStamp + GetWorldDayInMinutes(), uiParam ) );
}


static BOOLEAN AddSameDayStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiSecondStamp, UINT32 const uiParam)
{
	return( AddStrategicEventUsingSeconds( ubCallbackID, uiSecondStamp + GetWorldDayInSeconds(), uiParam ) );
}

BOOLEAN AddFutureDayStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiMinStamp, UINT32 const uiParam, UINT32 const uiNumDaysFromPresent)
{
	UINT32 uiDay;
	uiDay = GetWorldDay();
	return( AddStrategicEvent( ubCallbackID, uiMinStamp + GetFutureDayInMinutes( uiDay + uiNumDaysFromPresent ), uiParam ) );
}


static BOOLEAN AddFutureDayStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiSecondStamp, UINT32 const uiParam, UINT32 const uiNumDaysFromPresent)
{
	UINT32 uiDay;
	uiDay = GetWorldDay();
	return( AddStrategicEventUsingSeconds( ubCallbackID, uiSecondStamp + GetFutureDayInMinutes( uiDay + uiNumDaysFromPresent ) * 60, uiParam ) );
}

STRATEGICEVENT* AddAdvancedStrategicEvent(StrategicEventFrequency const ubEventType, StrategicEventKind const ubCallbackID, UINT32 const uiTimeStamp, UINT32 const uiParam)
{
	STRATEGICEVENT		*pNode, *pPrevNode;

	if( gfProcessingGameEvents && uiTimeStamp <= guiTimeStampOfCurrentlyExecutingEvent )
	{ //Prevents infinite loops of posting events that are the same time or earlier than the event
		//currently being processed.
		#ifdef JA2TESTVERSION
			//if( ubCallbackID == EVENT_PROCESS_TACTICAL_SCHEDULE )
			{
				ScreenMsg(FONT_RED, MSG_DEBUG, L"%ls Event Rejected:  Can't post events <= time while inside an event callback.  This is a special case situation that isn't a bug.", gEventName[ubCallbackID]);
			}
			//else
			//{
			//	AssertMsg( 0, String( "%ls Event Rejected:  Can't post events <= time while inside an event callback.", gEventName[ ubCallbackID ] ) );
			//}
		#endif
		return NULL;
	}

	STRATEGICEVENT* const pNewNode = MALLOCZ(STRATEGICEVENT);
	pNewNode->ubCallbackID		= ubCallbackID;
	pNewNode->uiParam					= uiParam;
	pNewNode->ubEventType			= ubEventType;
	pNewNode->uiTimeStamp			= uiTimeStamp;
	pNewNode->uiTimeOffset			= 0;

	// Search list for a place to insert
	pNode = gpEventList;

	// If it's the first head, do this!
	if( !pNode )
	{
		gpEventList = pNewNode;
		pNewNode->next = NULL;
	}
	else
	{
		pPrevNode = NULL;
		while( pNode )
		{
			if( uiTimeStamp < pNode->uiTimeStamp )
			{
				break;
			}
			pPrevNode = pNode;
			pNode = pNode->next;
		}

		// If we are at the end, set at the end!
		if ( !pNode )
		{
			pPrevNode->next = pNewNode;
			pNewNode->next  = NULL;
		}
		else
		{
			// We have a previous node here
			// Insert IN FRONT!
			if ( pPrevNode )
			{
				pNewNode->next = pPrevNode->next;
				pPrevNode->next = pNewNode;
			}
			else
			{	// It's the head
				pNewNode->next = gpEventList;
				gpEventList = pNewNode;
			}
		}
	}

	return pNewNode ;
}

BOOLEAN AddStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiMinStamp, UINT32 const uiParam)
{
	if( AddAdvancedStrategicEvent( ONETIME_EVENT, ubCallbackID, uiMinStamp*60, uiParam ) )
		return TRUE;
	return FALSE;
}

BOOLEAN AddStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiSecondStamp, UINT32 const uiParam)
{
	if( AddAdvancedStrategicEvent( ONETIME_EVENT, ubCallbackID, uiSecondStamp, uiParam ) )
		return TRUE;
	return FALSE;
}


static BOOLEAN AddRangedStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiStartMin, UINT32 const uiLengthMin, UINT32 const uiParam)
{
	STRATEGICEVENT *pEvent;
	pEvent = AddAdvancedStrategicEvent( RANGED_EVENT, ubCallbackID, uiStartMin*60, uiParam );
	if( pEvent )
	{
		pEvent->uiTimeOffset = uiLengthMin * 60;
		return TRUE;
	}
	return FALSE;
}


BOOLEAN AddSameDayRangedStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiStartMin, UINT32 const uiLengthMin, UINT32 const uiParam)
{
	return AddRangedStrategicEvent( ubCallbackID, uiStartMin + GetWorldDayInMinutes(), uiLengthMin, uiParam );
}


static BOOLEAN AddFutureDayRangedStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiStartMin, UINT32 const uiLengthMin, UINT32 const uiParam, UINT32 const uiNumDaysFromPresent)
{
	return AddRangedStrategicEvent( ubCallbackID, uiStartMin + GetFutureDayInMinutes( GetWorldDay() + uiNumDaysFromPresent ), uiLengthMin, uiParam );
}


static BOOLEAN AddRangedStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiStartSeconds, UINT32 const uiLengthSeconds, UINT32 const uiParam)
{
	STRATEGICEVENT *pEvent;
	pEvent = AddAdvancedStrategicEvent( RANGED_EVENT, ubCallbackID, uiStartSeconds, uiParam );
	if( pEvent )
	{
		pEvent->uiTimeOffset = uiLengthSeconds;
		return TRUE;
	}
	return FALSE;
}


static BOOLEAN AddSameDayRangedStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiStartSeconds, UINT32 const uiLengthSeconds, UINT32 const uiParam)
{
	return AddRangedStrategicEventUsingSeconds( ubCallbackID, uiStartSeconds + GetWorldDayInSeconds(), uiLengthSeconds, uiParam );
}


static BOOLEAN AddFutureDayRangedStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiStartSeconds, UINT32 const uiLengthSeconds, UINT32 const uiParam, UINT32 const uiNumDaysFromPresent)
{
	return AddRangedStrategicEventUsingSeconds( ubCallbackID, uiStartSeconds + GetFutureDayInMinutes( GetWorldDay() + uiNumDaysFromPresent ) * 60, uiLengthSeconds, uiParam );
}


BOOLEAN AddEveryDayStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiStartMin, UINT32 const uiParam)
{
	if( AddAdvancedStrategicEvent( EVERYDAY_EVENT, ubCallbackID, GetWorldDayInSeconds() + uiStartMin * 60, uiParam ) )
		return TRUE;
	return FALSE;
}


static BOOLEAN AddEveryDayStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiStartSeconds, UINT32 const uiParam)
{
	if( AddAdvancedStrategicEvent( EVERYDAY_EVENT, ubCallbackID, GetWorldDayInSeconds() + uiStartSeconds, uiParam ) )
		return TRUE;
	return FALSE;
}

//NEW:  Period Events
//Event will get processed automatically once every X minutes.
BOOLEAN AddPeriodStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiOnceEveryXMinutes, UINT32 const uiParam)
{
	STRATEGICEVENT *pEvent;
	pEvent = AddAdvancedStrategicEvent( PERIODIC_EVENT, ubCallbackID, GetWorldDayInSeconds() + uiOnceEveryXMinutes * 60, uiParam );
	if( pEvent )
	{
		pEvent->uiTimeOffset = uiOnceEveryXMinutes * 60;
		return TRUE;
	}
	return FALSE;
}


static BOOLEAN AddPeriodStrategicEventUsingSeconds(StrategicEventKind const ubCallbackID, UINT32 const uiOnceEveryXSeconds, UINT32 const uiParam)
{
	STRATEGICEVENT *pEvent;
	pEvent = AddAdvancedStrategicEvent( PERIODIC_EVENT, ubCallbackID, GetWorldDayInSeconds() + uiOnceEveryXSeconds, uiParam );
	if( pEvent )
	{
		pEvent->uiTimeOffset = uiOnceEveryXSeconds;
		return TRUE;
	}
	return FALSE;
}

BOOLEAN AddPeriodStrategicEventWithOffset(StrategicEventKind const ubCallbackID, UINT32 const uiOnceEveryXMinutes, UINT32 const uiOffsetFromCurrent, UINT32 const uiParam)
{
	STRATEGICEVENT *pEvent;
	pEvent = AddAdvancedStrategicEvent( PERIODIC_EVENT, ubCallbackID, GetWorldDayInSeconds() + uiOffsetFromCurrent * 60, uiParam );
	if( pEvent )
	{
		pEvent->uiTimeOffset = uiOnceEveryXMinutes * 60;
		return TRUE;
	}
	return FALSE;
}


static BOOLEAN AddPeriodStrategicEventUsingSecondsWithOffset(StrategicEventKind const ubCallbackID, UINT32 const uiOnceEveryXSeconds, UINT32 const uiOffsetFromCurrent, UINT32 const uiParam)
{
	STRATEGICEVENT *pEvent;
	pEvent = AddAdvancedStrategicEvent( PERIODIC_EVENT, ubCallbackID, GetWorldDayInSeconds() + uiOffsetFromCurrent, uiParam );
	if( pEvent )
	{
		pEvent->uiTimeOffset = uiOnceEveryXSeconds;
		return TRUE;
	}
	return FALSE;
}

void DeleteAllStrategicEventsOfType(StrategicEventKind const ubCallbackID)
{
	STRATEGICEVENT	*curr, *prev, *temp;
	prev = NULL;
	curr = gpEventList;
	while( curr )
	{
		if( curr->ubCallbackID == ubCallbackID && !(curr->ubFlags & SEF_DELETION_PENDING) )
		{
			if( gfPreventDeletionOfAnyEvent )
			{
				curr->ubFlags |= SEF_DELETION_PENDING;
				gfEventDeletionPending = TRUE;
				prev = curr;
				curr = curr->next;
				continue;
			}
			//Detach the node
			if( prev )
				prev->next = curr->next;
			else
				gpEventList = curr->next;

			//isolate and remove curr
			temp = curr;
			curr = curr->next;
			MemFree( temp );
			//ValidateGameEvents();
		}
		else
		{	//Advance all the nodes
			prev = curr;
			curr = curr->next;
		}
	}
}

void DeleteAllStrategicEvents()
{
	STRATEGICEVENT *temp;
	while( gpEventList )
	{
		temp = gpEventList;
		gpEventList = gpEventList->next;
		MemFree( temp );
		//ValidateGameEvents();
		temp = NULL;
	}
	gpEventList = NULL;
}

//Searches for and removes the first event matching the supplied information.  There may very well be a need
//for more specific event removal, so let me know (Kris), of any support needs.  Function returns FALSE if
//no events were found or if the event wasn't deleted due to delete lock,
BOOLEAN DeleteStrategicEvent(StrategicEventKind const ubCallbackID, UINT32 const uiParam)
{
	STRATEGICEVENT *curr, *prev;
	curr = gpEventList;
	prev = NULL;
	while( curr )
	{ //deleting middle
		if( curr->ubCallbackID == ubCallbackID && curr->uiParam == uiParam )
		{
			if( !(curr->ubFlags & SEF_DELETION_PENDING) )
			{
				if( gfPreventDeletionOfAnyEvent )
				{
					curr->ubFlags |= SEF_DELETION_PENDING;
					gfEventDeletionPending = TRUE;
					return FALSE;
				}
				if( prev )
				{
					prev->next = curr->next;
				}
				else
				{
					gpEventList = gpEventList->next;
				}
				MemFree( curr );
				//ValidateGameEvents();
				return TRUE;
			}
		}
		prev = curr;
		curr = curr->next;
	}
	return FALSE;
}


//part of the game.sav files (not map files)
void SaveStrategicEventsToSavedGame(HWFILE const f)
{
	// Determine the number of events
	UINT32 n_game_events = 0;
	for (STRATEGICEVENT* i = gpEventList; i; i = i->next)
	{
		++n_game_events;
	}
	FileWrite(f, &n_game_events, sizeof(UINT32));

	for (STRATEGICEVENT* i = gpEventList; i; i = i->next)
	{
		BYTE  data[28];
		BYTE* d = data;
		INJ_SKIP(d, 4)
		INJ_U32( d, i->uiTimeStamp)
		INJ_U32( d, i->uiParam)
		INJ_U32( d, i->uiTimeOffset)
		INJ_U8(  d, i->ubEventType)
		INJ_U8(  d, i->ubCallbackID)
		INJ_U8(  d, i->ubFlags)
		INJ_SKIP(d, 9)
		Assert(d == endof(data));

		FileWrite(f, data, sizeof(data));
	}
}


void LoadStrategicEventsFromSavedGame(HWFILE const f)
{
	// Erase the old Game Event queue
	DeleteAllStrategicEvents();

	// Read the number of strategic events
	UINT32 n_game_events;
	FileRead(f, &n_game_events, sizeof(UINT32));

	STRATEGICEVENT** anchor = &gpEventList;
	for (size_t n = n_game_events; n != 0; --n)
	{
		BYTE data[28];
		FileRead(f, data, sizeof(data));

		STRATEGICEVENT* const sev = MALLOCZ(STRATEGICEVENT);
		BYTE const*           d   = data;
		EXTR_SKIP(d, 4)
		EXTR_U32( d, sev->uiTimeStamp)
		EXTR_U32( d, sev->uiParam)
		EXTR_U32( d, sev->uiTimeOffset)
		EXTR_U8(  d, sev->ubEventType)
		EXTR_U8(  d, sev->ubCallbackID)
		EXTR_U8(  d, sev->ubFlags)
		EXTR_SKIP(d, 9)
		Assert(d == endof(data));

		*anchor = sev;
		anchor  = &sev->next;
	}
}


static void ValidateGameEvents(void)
{
	STRATEGICEVENT *curr;
	curr = gpEventList;
	while( curr )
	{
		curr = curr->next;
		if( curr == (STRATEGICEVENT*)0xdddddddd )
		{
			return;
		}
	}
}
