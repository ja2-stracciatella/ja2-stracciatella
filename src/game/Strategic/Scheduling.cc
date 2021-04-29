#include "FileMan.h"
#include "Font_Control.h"
#include "LoadSaveData.h"
#include "Types.h"
#include "Scheduling.h"
#include "Soldier_Control.h"
#include "Message.h"
#include "Overhead.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "WorldDef.h"
#include "Soldier_Init_List.h"
#include "Isometric_Utils.h"
#include "AI.h"
#include "Debug.h"
#include "Random.h"
#include "Animation_Data.h"
#include "Map_Information.h"
#include "Keys.h"
#include "Structure_Wrap.h"
#include "JAScreens.h"
#include "StrategicMap.h"
#include "WorldMan.h"
#include "Soldier_Add.h"
#include "Animation_Control.h"
#include "Soldier_Profile.h"
#include "Quests.h"
#include "MemMan.h"
#include "ScreenIDs.h"
#include "GameMode.h"
#include "Logger.h"
#include "EditorMercs.h"

#include <string_theory/string>


#define FOURPM 960

// waketime is the # of minutes in the day minus the sleep time
#define WAKETIME( x ) (NUM_SEC_IN_DAY/NUM_SEC_IN_MIN - x)

//#define DISABLESCHEDULES

SCHEDULENODE *gpScheduleList = NULL;
UINT8				gubScheduleID = 0;


//IMPORTANT:
//This function adds a NEWLY allocated schedule to the list.  The pointer passed is totally
//separate.  So make sure that you delete the pointer if you don't need it anymore.  The editor
//uses a single static node to copy data from, hence this method.
void CopyScheduleToList( SCHEDULENODE *pSchedule, SOLDIERINITNODE *pNode )
{
	SCHEDULENODE *curr;
	curr = gpScheduleList;
	gpScheduleList = new SCHEDULENODE{};
	*gpScheduleList = *pSchedule;
	gpScheduleList->next = curr;
	gubScheduleID++;
	//Assign all of the links
	gpScheduleList->ubScheduleID = gubScheduleID;
	gpScheduleList->soldier      = pNode->pSoldier;
	pNode->pDetailedPlacement->ubScheduleID = gubScheduleID;
	pNode->pSoldier->ubScheduleID = gubScheduleID;
	if( gubScheduleID > 40 )
	{ //Too much fragmentation, clean it up...
		OptimizeSchedules();
		if( gubScheduleID > 32 )
		{
			SLOGW("too many Schedules posted." );
		}
	}
}

SCHEDULENODE* GetSchedule( UINT8 ubScheduleID )
{
	SCHEDULENODE *curr;
	curr = gpScheduleList;
	while( curr )
	{
		if( curr->ubScheduleID == ubScheduleID )
			return curr;
		curr = curr->next;
	}
	return NULL;
}

//Removes all schedules from the event list, and cleans out the list.
void DestroyAllSchedules()
{
	SCHEDULENODE *curr;
	//First remove all of the events.
	DeleteAllStrategicEventsOfType( EVENT_PROCESS_TACTICAL_SCHEDULE );
	//Now, delete all of the schedules.
	while( gpScheduleList )
	{
		curr = gpScheduleList;
		gpScheduleList = gpScheduleList->next;
		delete curr;
	}
	gpScheduleList = NULL;
	gubScheduleID = 0;
}

// cleans out the schedule list without touching events, for saving & loading games
void DestroyAllSchedulesWithoutDestroyingEvents()
{
	SCHEDULENODE *curr;

	//delete all of the schedules.
	while( gpScheduleList )
	{
		curr = gpScheduleList;
		gpScheduleList = gpScheduleList->next;
		delete curr;
	}
	gpScheduleList = NULL;
	gubScheduleID = 0;
}

void DeleteSchedule( UINT8 ubScheduleID )
{
	SCHEDULENODE *curr, *temp = NULL;

	if (!gpScheduleList)
	{
		SLOGW("Attempting to delete schedule that doesn't exist");
		return;
	}

	curr = gpScheduleList;

	if( gpScheduleList->ubScheduleID == ubScheduleID )
	{ //Deleting the head
		temp = gpScheduleList;
		gpScheduleList = gpScheduleList->next;
	}
	else while( curr->next )
	{
		if( curr->next->ubScheduleID == ubScheduleID )
		{
			temp = curr->next;
			curr->next = temp->next;
			break;
		}
		curr = curr->next;
	}
	if( temp )
	{
		DeleteStrategicEvent( EVENT_PROCESS_TACTICAL_SCHEDULE, temp->ubScheduleID );
		delete temp;
	}
}


static void PrepareScheduleForAutoProcessing(SCHEDULENODE* pSchedule, UINT32 uiStartTime, UINT32 uiEndTime);


void ProcessTacticalSchedule( UINT8 ubScheduleID )
{
	SCHEDULENODE *pSchedule;
	INT32 iScheduleIndex=0;
	BOOLEAN fAutoProcess;

	//Attempt to locate the schedule.
	pSchedule = GetSchedule( ubScheduleID );
	if( !pSchedule )
	{
		SLOGW("Schedule callback:  Schedule ID of %d not found.", ubScheduleID );
		return;
	}
	//Attempt to access the soldier involved
	SOLDIERTYPE* const pSoldier = pSchedule->soldier;
	if (pSoldier == NULL)
	{
		SLOGW("Schedule callback:  Illegal NULL soldier.");
		return;
	}

	//Validate the existance of the soldier.
	if ( pSoldier->bLife < OKLIFE )
	{
		// dead or dying!
		return;
	}

	if ( !pSoldier->bActive )
	{
		SLOGW("Schedule callback:  Soldier isn't active.  Name is %s.", pSoldier->name.c_str());
	}

	//Okay, now we have good pointers to the soldier and the schedule.
	//Now, determine which time in this schedule that we are processing.
	fAutoProcess = FALSE;
	if( guiCurrentScreen != GAME_SCREEN )
	{
		SLOGD("Schedule callback occurred outside of tactical -- Auto processing!" );
		fAutoProcess = TRUE;
	}
	else
	{
		for( iScheduleIndex = 0; iScheduleIndex < MAX_SCHEDULE_ACTIONS; iScheduleIndex++ )
		{
			if( pSchedule->usTime[ iScheduleIndex ] == GetWorldMinutesInDay() )
			{
				SLOGD("Processing schedule on time -- AI processing!" );
				break;
			}
		}
		if( iScheduleIndex == MAX_SCHEDULE_ACTIONS )
		{
			fAutoProcess = TRUE;
			SLOGD("Possible timewarp causing schedule callback to occur late -- Auto processing!" );
		}
	}
	if ( fAutoProcess )
	{
		UINT32 uiStartTime, uiEndTime;
		//Grab the last time the eventlist was queued.  This will tell us how much time has passed since that moment,
		//and how long we need to auto process this schedule.
		uiStartTime = (guiTimeOfLastEventQuery / 60) % NUM_MIN_IN_DAY;
		uiEndTime = GetWorldMinutesInDay();
		if( uiStartTime != uiEndTime )
		{
			PrepareScheduleForAutoProcessing( pSchedule, uiStartTime, uiEndTime );
		}
	}
	else
	{
		// turn off all active-schedule flags before setting
		// the one that should be active!
		pSchedule->usFlags &= ~SCHEDULE_FLAGS_ACTIVE_ALL;

		switch( iScheduleIndex )
		{
			case 0:			pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE1;			break;
			case 1:			pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE2;			break;
			case 2:			pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE3;			break;
			case 3:			pSchedule->usFlags |= SCHEDULE_FLAGS_ACTIVE4;			break;
		}
		pSoldier->fAIFlags |= AI_CHECK_SCHEDULE;
		pSoldier->bAIScheduleProgress = 0;
	}

}

//Called before leaving the editor, or saving the map.  This recalculates
//all of the schedule IDs from scratch and adjusts the effected structures accordingly.
void OptimizeSchedules()
{
	SCHEDULENODE *pSchedule;
	UINT8 ubOldScheduleID;
	gubScheduleID = 0;
	pSchedule = gpScheduleList;
	while( pSchedule )
	{
		gubScheduleID++;
		ubOldScheduleID = pSchedule->ubScheduleID;
		if( ubOldScheduleID != gubScheduleID )
		{ //The schedule ID has changed, so change all links accordingly.
			pSchedule->ubScheduleID = gubScheduleID;
			CFOR_EACH_SOLDIERINITNODE(pNode)
			{
				if( pNode->pDetailedPlacement && pNode->pDetailedPlacement->ubScheduleID == ubOldScheduleID )
				{
					//Temporarily add 100 to the ID number to ensure that it doesn't get used again later.
					//We will remove it immediately after this loop is complete.
					pNode->pDetailedPlacement->ubScheduleID = gubScheduleID + 100;
					if( pNode->pSoldier )
					{
						pNode->pSoldier->ubScheduleID = gubScheduleID;
					}
					break;
				}
			}
		}
		pSchedule = pSchedule->next;
	}
	//Remove the +100 IDs.
	CFOR_EACH_SOLDIERINITNODE(pNode)
	{
		if( pNode->pDetailedPlacement && pNode->pDetailedPlacement->ubScheduleID > 100 )
		{
			pNode->pDetailedPlacement->ubScheduleID -= 100;
		}
	}
}

//Called when transferring from the game to the editor.
void PrepareSchedulesForEditorEntry()
{
	SCHEDULENODE *curr, *prev, *temp;

	//Delete all schedule events.  The editor will automatically warp all civilians to their starting locations.
	DeleteAllStrategicEventsOfType( EVENT_PROCESS_TACTICAL_SCHEDULE );

	//Now, delete all of the temporary schedules.
	curr = gpScheduleList;
	prev = NULL;
	while( curr )
	{
		if( curr->usFlags & SCHEDULE_FLAGS_TEMPORARY )
		{
			if( prev )
				prev->next = curr->next;
			else
				gpScheduleList = gpScheduleList->next;
			curr->soldier->ubScheduleID = 0;
			temp = curr;
			curr = curr->next;
			delete temp;
			gubScheduleID--;
		}
		else
		{
			if( curr->usFlags & SCHEDULE_FLAGS_SLEEP_CONVERTED )
			{ //uncovert it!
				INT32 i;
				for( i = 0 ; i < MAX_SCHEDULE_ACTIONS; i++ )
				{
					//if( i
				}
			}
			prev = curr;
			curr = curr->next;
		}
	}
}

//Called when leaving the editor to enter the game.  This posts all of the events that apply.
void PrepareSchedulesForEditorExit()
{
	PostSchedules();
}


void LoadSchedules(HWFILE const f)
{
	/* Delete all the schedules we might have loaded (though we shouldn't have any
		* loaded!) */
	if (gpScheduleList) DestroyAllSchedules();

	UINT8 n_schedules;
	FileRead(f, &n_schedules, sizeof(n_schedules));

	gubScheduleID = 1;
	SCHEDULENODE** anchor = &gpScheduleList;
	for (UINT8 n = n_schedules; n != 0; --n)
	{
		BYTE data[36];
		FileRead(f, data, sizeof(data));

		SCHEDULENODE* const node = new SCHEDULENODE{};

		DataReader d{data};
		EXTR_SKIP(d, 4)
		EXTR_U16A(d, node->usTime,   lengthof(node->usTime))
		EXTR_U16A(d, node->usData1,  lengthof(node->usData1))
		EXTR_U16A(d, node->usData2,  lengthof(node->usData2))
		EXTR_U8A( d, node->ubAction, lengthof(node->ubAction))
		EXTR_SKIP(d, 2) // skip schedule ID and soldier ID, they get overwritten
		EXTR_U16( d, node->usFlags)
		Assert(d.getConsumed() == lengthof(data));

		node->ubScheduleID = gubScheduleID++;

		// Add node to the list
		*anchor = node;
		anchor  = &node->next;
	}
	// Schedules are posted when the soldier is added
}


void LoadSchedulesFromSave(HWFILE const f)
{
	UINT8 n_schedules_saved;
	FileRead(f, &n_schedules_saved, sizeof(n_schedules_saved));

	// Hack problem with schedules getting misaligned.
	UINT32 n_schedules = n_schedules_saved;

	gubScheduleID = 1;
	SCHEDULENODE** anchor = &gpScheduleList;
	for (; n_schedules != 0; --n_schedules)
	{
		BYTE data[36];
		FileRead(f, data, sizeof(data));

		SCHEDULENODE* const node = new SCHEDULENODE{};

		DataReader s{data};
		EXTR_SKIP(   s, 4)
		EXTR_U16A(   s, node->usTime,   lengthof(node->usTime))
		EXTR_U16A(   s, node->usData1,  lengthof(node->usData1))
		EXTR_U16A(   s, node->usData2,  lengthof(node->usData2))
		EXTR_U8A(    s, node->ubAction, lengthof(node->ubAction))
		EXTR_U8(     s, node->ubScheduleID)
		EXTR_SOLDIER(s, node->soldier)
		EXTR_U16(    s, node->usFlags)
		Assert(s.getConsumed() == lengthof(data));

		// Add node to the list
		*anchor = node;
		anchor  = &node->next;

		++gubScheduleID;
	}
	// Schedules are posted when the soldier is added
}


void ReverseSchedules()
{
	SCHEDULENODE *pReverseHead, *pPrevReverseHead, *pPrevScheduleHead;
	UINT8	ubOppositeID = gubScheduleID;
	//First, remove any gaps which would mess up the reverse ID assignment by optimizing
	//the schedules.
	OptimizeSchedules();
	pReverseHead = NULL;
	while( gpScheduleList )
	{
		//reverse the ID
		gpScheduleList->ubScheduleID = ubOppositeID;
		ubOppositeID--;
		//detach current schedule head from list and advance it
		pPrevScheduleHead = gpScheduleList;
		gpScheduleList = gpScheduleList->next;
		//get previous reversed list head (even if null)
		pPrevReverseHead = pReverseHead;
		//Assign the previous schedule head to the reverse head
		pReverseHead = pPrevScheduleHead;
		//Point the next to the previous reverse head.
		pReverseHead->next = pPrevReverseHead;
	}
	//Now assign the schedule list to the reverse head.
	gpScheduleList = pReverseHead;
}

//Another debug feature.
void ClearAllSchedules()
{
	DestroyAllSchedules();
	CFOR_EACH_SOLDIERINITNODE(pNode)
	{
		if( pNode->pDetailedPlacement && pNode->pDetailedPlacement->ubScheduleID )
		{
			pNode->pDetailedPlacement->ubScheduleID = 0;
			if( pNode->pSoldier )
			{
				pNode->pSoldier->ubScheduleID = 0;
			}
		}
	}
}


void SaveSchedules(HWFILE const f)
{
	// Count the number of schedules in the list
	INT32 n_schedules = 0;
	for (SCHEDULENODE const* i = gpScheduleList; i; i = i->next)
	{
		// Skip all default schedules
		if (i->usFlags & SCHEDULE_FLAGS_TEMPORARY) continue;
		++n_schedules;
	}

	UINT8 n_to_save = MIN(n_schedules, 32);
	FileWrite(f, &n_to_save, sizeof(UINT8));

	// Save each schedule
	for (SCHEDULENODE const* i = gpScheduleList; i; i = i->next)
	{
		// Skip all default schedules
		if (i->usFlags & SCHEDULE_FLAGS_TEMPORARY) continue;

		if (n_to_save-- == 0) return;

		BYTE data[36];
		DataWriter d{data};
		INJ_SKIP(   d, 4)
		INJ_U16A(   d, i->usTime,   lengthof(i->usTime))
		INJ_U16A(   d, i->usData1,  lengthof(i->usData1))
		INJ_U16A(   d, i->usData2,  lengthof(i->usData2))
		INJ_U8A(    d, i->ubAction, lengthof(i->ubAction))
		INJ_U8(     d, i->ubScheduleID)
		INJ_SOLDIER(d, i->soldier)
		INJ_U16(    d, i->usFlags)
		Assert(d.getConsumed() == lengthof(data));

		FileWrite(f, data, sizeof(data));
	}
}


//Each schedule has upto four parts to it, so sort them chronologically.
//Happily, the fields with no times actually are the highest.
BOOLEAN SortSchedule( SCHEDULENODE *pSchedule )
{
	INT32 index, i, iBestIndex;
	UINT16 usTime;
	UINT16 usData1;
	UINT16 usData2;
	UINT8 ubAction;
	BOOLEAN fSorted = FALSE;

	//Use a bubblesort method (max:  3 switches).
	index = 0;
	while( index < 3 )
	{
		usTime = 0xffff;
		iBestIndex = index;
		for( i = index; i < MAX_SCHEDULE_ACTIONS; i++ )
		{
			if( pSchedule->usTime[ i ] < usTime )
			{
				usTime = pSchedule->usTime[ i ];
				iBestIndex = i;
			}
		}
		if( iBestIndex != index )
		{ //we will swap the best index with the current index.
			fSorted = TRUE;
			usTime		= pSchedule->usTime[ index ];
			usData1		= pSchedule->usData1[ index ];
			usData2		= pSchedule->usData2[ index ];
			ubAction	= pSchedule->ubAction[ index ];
			pSchedule->usTime[ index ]		= pSchedule->usTime[ iBestIndex ];
			pSchedule->usData1[ index ]		= pSchedule->usData1[ iBestIndex ];
			pSchedule->usData2[ index ]		= pSchedule->usData2[ iBestIndex ];
			pSchedule->ubAction[ index ]	= pSchedule->ubAction[ iBestIndex ];
			pSchedule->usTime[ iBestIndex ]		= usTime;
			pSchedule->usData1[ iBestIndex ]	= usData1;
			pSchedule->usData2[ iBestIndex ]	= usData2;
			pSchedule->ubAction[ iBestIndex ]	= ubAction;
		}
		index++;
	}
	return fSorted;
}

BOOLEAN BumpAnyExistingMerc( INT16 sGridNo )
{
	// this is for autoprocessing schedules...
	// there could be someone in the destination location, in which case
	// we want to 'bump' them to the nearest available spot

	if ( !GridNoOnVisibleWorldTile( sGridNo ) )
	{
		return( TRUE );
	}

	SOLDIERTYPE* const pSoldier = WhoIsThere2(sGridNo, 0);
	if (pSoldier == NULL) return TRUE;

	// what if the existing merc is prone?
	const INT16 sNewGridNo = FindGridNoFromSweetSpotWithStructDataFromSoldier(pSoldier, STANDING, 5, 1, pSoldier);
	//const INT16 sNewGridNo = FindGridNoFromSweetSpotExcludingSweetSpot(pSoldier, sGridNo, 10);

	if ( sNewGridNo == NOWHERE )
	{
		return( FALSE );
	}

	EVENT_SetSoldierPositionNoCenter(pSoldier, sNewGridNo, SSP_FORCE_DELETE);

	return( TRUE );
}


static void PerformActionOnDoorAdjacentToGridNo(UINT8 ubScheduleAction, UINT16 usGridNo);


static void AutoProcessSchedule(SCHEDULENODE* pSchedule, INT32 index)
{
	INT8						bDirection;

	if ( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
	{
		// CJC, November 28th:  when reloading a saved game we want events posted but no events autoprocessed since
		// that could change civilian positions.  So rather than doing a bunch of checks outside of this function,
		// I thought it easier to screen them out here.
		return;
	}

	SOLDIERTYPE* const pSoldier = pSchedule->soldier;

	if(GameMode::getInstance()->isEditorMode())
	{
		if ( pSoldier->ubProfile != NO_PROFILE )
		{
				SLOGD("Autoprocessing schedule action %s for %s (%d) at time %02ld:%02ld (set for %02d:%02d), data1 = %d",
				gszScheduleActions[ pSchedule->ubAction[ index ] ].c_str(),
				pSoldier->name.c_str(),
				pSoldier->ubID,
				GetWorldHour(),
				guiMin,
				pSchedule->usTime[ index ] / 60,
				pSchedule->usTime[ index ] % 60,
				pSchedule->usData1[ index ]);
		}
		else
		{
			SLOGD("Autoprocessing schedule action %s for civ (%d) at time %02ld:%02ld (set for %02d:%02d), data1 = %d",
				gszScheduleActions[ pSchedule->ubAction[ index ] ].c_str(),
				pSoldier->ubID,
				GetWorldHour(),
				guiMin,
				pSchedule->usTime[ index ] / 60,
				pSchedule->usTime[ index ] % 60,
				pSchedule->usData1[ index ]);
		}
	}

	// always assume the merc is going to wake, unless the event is a sleep
	pSoldier->fAIFlags &= ~(AI_ASLEEP);

	switch( pSchedule->ubAction[ index ] )
	{
		case SCHEDULE_ACTION_LOCKDOOR:
		case SCHEDULE_ACTION_UNLOCKDOOR:
		case SCHEDULE_ACTION_OPENDOOR:
		case SCHEDULE_ACTION_CLOSEDOOR:
			PerformActionOnDoorAdjacentToGridNo( pSchedule->ubAction[ index ], pSchedule->usData1[ index ] );
			BumpAnyExistingMerc( pSchedule->usData2[ index ] );

			EVENT_SetSoldierPositionNoCenter(pSoldier, pSchedule->usData2[index], SSP_FORCE_DELETE);
			if ( GridNoOnEdgeOfMap( pSchedule->usData2[ index ], &bDirection ) )
			{
				// civ should go off map; this tells us where the civ will return
				pSoldier->sOffWorldGridNo = pSchedule->usData2[ index ];
				MoveSoldierFromMercToAwaySlot( pSoldier );
				pSoldier->bInSector = FALSE;
			}
			else
			{
				// let this person patrol from here from now on
				pSoldier->usPatrolGrid[0] = pSchedule->usData2[ index ];
			}
			break;
		case SCHEDULE_ACTION_GRIDNO:
			BumpAnyExistingMerc( pSchedule->usData1[ index ] );
			EVENT_SetSoldierPositionNoCenter(pSoldier, pSchedule->usData1[index], SSP_FORCE_DELETE);
			// let this person patrol from here from now on
			pSoldier->usPatrolGrid[0] = pSchedule->usData1[ index ];
			break;
		case SCHEDULE_ACTION_ENTERSECTOR:
			if ( pSoldier->ubProfile != NO_PROFILE && gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR )
			{
				// never process enter if flag is set
				break;
			}
			BumpAnyExistingMerc( pSchedule->usData1[ index ] );
			EVENT_SetSoldierPositionNoCenter(pSoldier, pSchedule->usData1[index], SSP_FORCE_DELETE);
			MoveSoldierFromAwayToMercSlot( pSoldier );
			pSoldier->bInSector = TRUE;
			// let this person patrol from here from now on
			pSoldier->usPatrolGrid[0] = pSchedule->usData1[ index ];
			break;
		case SCHEDULE_ACTION_WAKE:
			BumpAnyExistingMerc( pSoldier->sInitialGridNo );
			EVENT_SetSoldierPositionNoCenter(pSoldier, pSoldier->sInitialGridNo, SSP_FORCE_DELETE);
			// let this person patrol from here from now on
			pSoldier->usPatrolGrid[0] = pSoldier->sInitialGridNo;
			break;
		case SCHEDULE_ACTION_SLEEP:
			pSoldier->fAIFlags |= AI_ASLEEP;
			// check for someone else in the location
			BumpAnyExistingMerc( pSchedule->usData1[ index ] );
			EVENT_SetSoldierPositionNoCenter(pSoldier, pSchedule->usData1[index], SSP_FORCE_DELETE);
			pSoldier->usPatrolGrid[0] = pSchedule->usData1[ index ];
			break;
		case SCHEDULE_ACTION_LEAVESECTOR:
		{
			INT16 sGridNo;
			sGridNo = FindNearestEdgePoint( pSoldier->sGridNo );
			BumpAnyExistingMerc( sGridNo );
			EVENT_SetSoldierPositionNoCenter(pSoldier, sGridNo, SSP_FORCE_DELETE);

			sGridNo = FindNearbyPointOnEdgeOfMap( pSoldier, &bDirection );
			BumpAnyExistingMerc( sGridNo );
			EVENT_SetSoldierPositionNoCenter(pSoldier, sGridNo, SSP_FORCE_DELETE);

			// ok, that tells us where the civ will return
			pSoldier->sOffWorldGridNo = sGridNo;
			MoveSoldierFromMercToAwaySlot( pSoldier );
			pSoldier->bInSector = FALSE;
			break;
		}
	}
}


static INT8 GetEmptyScheduleEntry(SCHEDULENODE* pSchedule);
static BOOLEAN ScheduleHasMorningNonSleepEntries(SCHEDULENODE* pSchedule);
static void SecureSleepSpot(SOLDIERTYPE* pSoldier, UINT16 usSleepSpot);


static void PostSchedule(SOLDIERTYPE* pSoldier)
{
	UINT32 uiStartTime, uiEndTime;
	INT32 i;
	INT8	bEmpty;
	SCHEDULENODE *pSchedule;
	UINT8	ubTempAction;
	UINT16	usTemp;

	if ( (pSoldier->ubCivilianGroup == KINGPIN_CIV_GROUP) && ( gTacticalStatus.fCivGroupHostile[ KINGPIN_CIV_GROUP ] || ( (gubQuest[ QUEST_KINGPIN_MONEY ] == QUESTINPROGRESS) && (CheckFact( FACT_KINGPIN_CAN_SEND_ASSASSINS, KINGPIN )) ) ) && (gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_C) && (pSoldier->ubProfile == NO_PROFILE) )
	{
		// no schedules for people guarding Tony's!
		return;
	}

	pSchedule = GetSchedule( pSoldier->ubScheduleID );
	if( !pSchedule )
		return;

	if ( pSoldier->ubProfile != NO_PROFILE && gMercProfiles[ pSoldier->ubProfile ].ubMiscFlags3 & PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE )
	{
		// don't process schedule
		return;
	}

	//if this schedule doesn't have a time associated with it, then generate a time, but only
	//if it is a sleep schedule.
	for( i = 0; i < MAX_SCHEDULE_ACTIONS; i++ )
	{
		if ( pSchedule->ubAction[ i ] == SCHEDULE_ACTION_SLEEP )
		{
			// first make sure that this merc has a unique spot to sleep in
			SecureSleepSpot( pSoldier, pSchedule->usData1[ i ] );

			if( pSchedule->usTime[ i ] == 0xffff )
			{
				pSchedule->usTime[ i ] = (UINT16)( (21*60) + Random( (3*60) )); //9PM - 11:59PM

				if ( ScheduleHasMorningNonSleepEntries( pSchedule ) )
				{
					// this guy will sleep until the next non-sleep event
				}
				else
				{
					bEmpty = GetEmptyScheduleEntry( pSchedule );
					if ( bEmpty != -1 )
					{
						// there is an empty entry for the wakeup call

						// NB the wakeup call must be ordered first! so we have to create the
						// wake action and then swap the two.
						pSchedule->ubAction[ bEmpty ] = SCHEDULE_ACTION_WAKE;
						pSchedule->usTime[ bEmpty ] = (pSchedule->usTime[ i ] + (8*60)) % NUM_MIN_IN_DAY; // sleep for 8 hours

						ubTempAction = pSchedule->ubAction[ bEmpty ];
						pSchedule->ubAction[ bEmpty ] = pSchedule->ubAction[ i ];
						pSchedule->ubAction[ i ] = ubTempAction;

						usTemp = pSchedule->usTime[ bEmpty ];
						pSchedule->usTime[ bEmpty ] = pSchedule->usTime[ i ];
						pSchedule->usTime[ i ] = usTemp;

						usTemp = pSchedule->usData1[ bEmpty ];
						pSchedule->usData1[ bEmpty ] = pSchedule->usData1[ i ];
						pSchedule->usData1[ i ] = usTemp;

						usTemp = pSchedule->usData2[ bEmpty ];
						pSchedule->usData2[ bEmpty ] = pSchedule->usData2[ i ];
						pSchedule->usData2[ i ] = usTemp;
					}
					else
					{
						// no morning entries but no space for a wakeup either, will sleep till
						// next non-sleep event
					}

				}
				break; //The break is here because nobody should have more than one sleep schedule with no time specified.
			}
		}
	}

	pSchedule->soldier = pSoldier;

	// always process previous 24 hours
	uiEndTime = GetWorldTotalMin();
	uiStartTime = uiEndTime - (NUM_MIN_IN_DAY - 1);

	/*
	//First thing we need is to get the time that the map was last loaded.  If more than 24 hours,
	//then process only 24 hours.  If less, then process all the schedules that would have happened within
	//that period of time.
	uiEndTime = GetWorldTotalMin();
	if( GetWorldTotalMin() - guiTimeCurrentSectorWasLastLoaded > NUM_MIN_IN_DAY )
	{ //Process the last 24 hours
		uiStartTime = uiEndTime - (NUM_MIN_IN_DAY - 1);
	}
	else
	{ //Process the time since we were last here.
		uiStartTime = guiTimeCurrentSectorWasLastLoaded;
	}
	*/

	//Need a way to determine if the player has actually modified doors since this civilian was last loaded
	uiEndTime %= NUM_MIN_IN_DAY;
	uiStartTime %= NUM_MIN_IN_DAY;
	PrepareScheduleForAutoProcessing( pSchedule, uiStartTime, uiEndTime );
}


static void PrepareScheduleForAutoProcessing(SCHEDULENODE* pSchedule, UINT32 uiStartTime, UINT32 uiEndTime)
{
	INT32 i;
	BOOLEAN	fPostedNextEvent = FALSE;

	if ( uiStartTime > uiEndTime )
	{ //The start time is later in the day than the end time, which means we'll be wrapping
		//through midnight and continuing to the end time.
		for( i = 0; i < MAX_SCHEDULE_ACTIONS; i++ )
		{
			if( pSchedule->usTime[i] == 0xffff )
				break;
			if( pSchedule->usTime[i] >= uiStartTime )
			{
				AutoProcessSchedule( pSchedule, i );
			}
		}
		for( i = 0; i < MAX_SCHEDULE_ACTIONS; i++ )
		{
			if( pSchedule->usTime[i] == 0xffff )
				break;
			if( pSchedule->usTime[i] <= uiEndTime )
			{
				AutoProcessSchedule( pSchedule, i );
			}
			else
			{
				// CJC: Note that end time is always passed in here as the current time so GetWorldDayInMinutes will be for the correct day
				AddStrategicEvent( EVENT_PROCESS_TACTICAL_SCHEDULE, GetWorldDayInMinutes() + pSchedule->usTime[i], pSchedule->ubScheduleID );
				fPostedNextEvent = TRUE;
				break;
			}
		}
	}
	else
	{ //Much simpler:  start at the start and continue to the end.
		for( i = 0; i < MAX_SCHEDULE_ACTIONS; i++ )
		{
			if( pSchedule->usTime[i] == 0xffff )
				break;

			if( pSchedule->usTime[i] >= uiStartTime && pSchedule->usTime[i] <= uiEndTime )
			{
				AutoProcessSchedule( pSchedule, i );
			}
			else if ( pSchedule->usTime[i] >= uiEndTime )
			{
				fPostedNextEvent = TRUE;
				AddStrategicEvent( EVENT_PROCESS_TACTICAL_SCHEDULE, GetWorldDayInMinutes() + pSchedule->usTime[i], pSchedule->ubScheduleID );
				break;
			}

		}
	}

	if ( !fPostedNextEvent )
	{
		// reached end of schedule, post first event for soldier in the next day
		// 0th event will be first.
		// Feb 1:  ONLY IF THERE IS A VALID EVENT TO POST WITH A VALID TIME!
		if ( pSchedule->usTime[0] != 0xffff )
		{
			AddStrategicEvent( EVENT_PROCESS_TACTICAL_SCHEDULE, GetWorldDayInMinutes() + NUM_MIN_IN_DAY + pSchedule->usTime[0], pSchedule->ubScheduleID );
		}
	}
}


//Leave at night, come back in the morning.  The time variances are a couple hours, so
//the town doesn't turn into a ghost town in 5 minutes.
static void PostDefaultSchedule(SOLDIERTYPE* pSoldier)
{
	INT32 i;
	SCHEDULENODE *curr;

	if( gbWorldSectorZ )
	{ //People in underground sectors don't get schedules.
		return;
	}
	//Create a new node at the head of the list.  The head will become the new schedule
	//we are about to add.
	curr = gpScheduleList;
	gpScheduleList = new SCHEDULENODE{};
	gpScheduleList->next = curr;
	gubScheduleID++;
	//Assign all of the links
	gpScheduleList->ubScheduleID = gubScheduleID;
	gpScheduleList->soldier      = pSoldier;
	pSoldier->ubScheduleID = gubScheduleID;

	//Clear the data inside the schedule
	for( i = 0; i < MAX_SCHEDULE_ACTIONS; i++ )
	{
		gpScheduleList->usTime[i] = 0xffff;
		gpScheduleList->usData1[i] = 0xffff;
		gpScheduleList->usData2[i] = 0xffff;
	}
	//Have the default schedule enter between 7AM and 8AM
	gpScheduleList->ubAction[0] = SCHEDULE_ACTION_ENTERSECTOR;
	gpScheduleList->usTime[0] = (UINT16)(420 + Random( 61 ));
	gpScheduleList->usData1[0] = pSoldier->sInitialGridNo;
	//Have the default schedule leave between 6PM and 8PM
	gpScheduleList->ubAction[1] = SCHEDULE_ACTION_LEAVESECTOR;
	gpScheduleList->usTime[1] = (UINT16)(1080 + Random( 121 ));
	gpScheduleList->usFlags |= SCHEDULE_FLAGS_TEMPORARY;

	if( gubScheduleID == 255 )
	{ //Too much fragmentation, clean it up...
		OptimizeSchedules();
		if( gubScheduleID == 255 )
		{
			SLOGA("Too many schedules posted" );
		}
	}

	PostSchedule( pSoldier );
}


void PostSchedules()
{
	BOOLEAN fDefaultSchedulesPossible = FALSE;

	#if defined( DISABLESCHEDULES ) //definition found at top of this .c file.

		return;

	#endif
	//If no way to leave the map, then don't post default schedules.
	if( gMapInformation.sNorthGridNo != -1 || gMapInformation.sEastGridNo != -1 ||
		gMapInformation.sSouthGridNo != -1 || gMapInformation.sWestGridNo != -1 )
	{
		fDefaultSchedulesPossible = TRUE;
	}
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		if( curr->pSoldier && curr->pSoldier->bTeam == CIV_TEAM )
		{
			if( curr->pDetailedPlacement && curr->pDetailedPlacement->ubScheduleID )
			{
				PostSchedule( curr->pSoldier );
			}
			else if( fDefaultSchedulesPossible )
			{
				// ATE: There should be a better way here...
				if( curr->pSoldier->ubBodyType != COW &&
					curr->pSoldier->ubBodyType != BLOODCAT &&
					curr->pSoldier->ubBodyType != HUMVEE &&
					curr->pSoldier->ubBodyType != ELDORADO &&
					curr->pSoldier->ubBodyType != ICECREAMTRUCK &&
					curr->pSoldier->ubBodyType != JEEP )
				{
					PostDefaultSchedule( curr->pSoldier );
				}
			}
		}
	}
}


static void PerformActionOnDoorAdjacentToGridNo(UINT8 ubScheduleAction, UINT16 usGridNo)
{
	INT16			sDoorGridNo;
	DOOR *		pDoor;

	sDoorGridNo = FindDoorAtGridNoOrAdjacent( (INT16) usGridNo );
	if (sDoorGridNo != NOWHERE)
	{
		switch( ubScheduleAction )
		{
			case SCHEDULE_ACTION_LOCKDOOR:
				pDoor = FindDoorInfoAtGridNo( sDoorGridNo );
				if (pDoor)
				{
					pDoor->fLocked = TRUE;
				}
				// make sure it's closed as well
				ModifyDoorStatus( sDoorGridNo, FALSE, DONTSETDOORSTATUS );
				break;
			case SCHEDULE_ACTION_UNLOCKDOOR:
				pDoor = FindDoorInfoAtGridNo( sDoorGridNo );
				if (pDoor)
				{
					pDoor->fLocked = FALSE;
				}
				break;
			case SCHEDULE_ACTION_OPENDOOR:
				ModifyDoorStatus( sDoorGridNo, TRUE, DONTSETDOORSTATUS );
				break;
			case SCHEDULE_ACTION_CLOSEDOOR:
				ModifyDoorStatus( sDoorGridNo, FALSE, DONTSETDOORSTATUS );
				break;
		}
	}
}

//Assumes that a schedule has just been processed.  This takes the current time, and compares it to the
//schedule, and looks for the next schedule action that would get processed and posts it.
void PostNextSchedule( SOLDIERTYPE *pSoldier )
{
	SCHEDULENODE *pSchedule;
	INT32 i, iBestIndex;
	UINT16 usTime, usBestTime;
	pSchedule = GetSchedule( pSoldier->ubScheduleID );
	if( !pSchedule )
	{ //post default?
		return;
	}
	usTime = (UINT16)GetWorldMinutesInDay();
	usBestTime = 0xffff;
	iBestIndex = -1;
	for( i = 0; i < MAX_SCHEDULE_ACTIONS; i++ )
	{
		if( pSchedule->usTime[i] == 0xffff )
			continue;
		if( pSchedule->usTime[i] == usTime )
			continue;
		if( pSchedule->usTime[i] > usTime )
		{
			if( pSchedule->usTime[i] - usTime < usBestTime )
			{
				usBestTime = pSchedule->usTime[ i ] - usTime;
				iBestIndex = i;
			}
		}
		else if ( (NUM_MIN_IN_DAY - (usTime - pSchedule->usTime[ i ])) < usBestTime )
		{
			usBestTime = NUM_MIN_IN_DAY - (usTime - pSchedule->usTime[ i ]);
			iBestIndex = i;
		}
	}
	Assert( iBestIndex >= 0 );

	AddStrategicEvent( EVENT_PROCESS_TACTICAL_SCHEDULE, GetWorldDayInMinutes() + pSchedule->usTime[iBestIndex], pSchedule->ubScheduleID );
}


// This is for determining shopkeeper's opening/closing hours
BOOLEAN ExtractScheduleDoorLockAndUnlockInfo( SOLDIERTYPE * pSoldier, UINT32 * puiOpeningTime, UINT32 * puiClosingTime )
{
	INT32			iLoop;
	BOOLEAN		fFoundOpeningTime = FALSE, fFoundClosingTime = FALSE;
	SCHEDULENODE *pSchedule;

	*puiOpeningTime = 0;
	*puiClosingTime = 0;

	pSchedule = GetSchedule( pSoldier->ubScheduleID );
	if ( !pSchedule )
	{
		// If person had default schedule then would have been assigned and this would
		// have succeeded.
		// Hence this is an error.
		return( FALSE );
	}

	for ( iLoop = 0; iLoop < MAX_SCHEDULE_ACTIONS; iLoop++ )
	{
		if ( pSchedule->ubAction[ iLoop ] == SCHEDULE_ACTION_UNLOCKDOOR )
		{
			fFoundOpeningTime = TRUE;
			*puiOpeningTime = pSchedule->usTime[ iLoop ];
		}
		else if ( pSchedule->ubAction[ iLoop ] == SCHEDULE_ACTION_LOCKDOOR )
		{
			fFoundClosingTime = TRUE;
			*puiClosingTime = pSchedule->usTime[ iLoop ];
		}
	}

	if ( fFoundOpeningTime && fFoundClosingTime )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


static BOOLEAN ScheduleHasMorningNonSleepEntries(SCHEDULENODE* pSchedule)
{
	INT8			bLoop;

	for ( bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++ )
	{
		if ( pSchedule->ubAction[ bLoop ] != SCHEDULE_ACTION_NONE && pSchedule->ubAction[ bLoop ] != SCHEDULE_ACTION_SLEEP )
		{
			if ( pSchedule->usTime[ bLoop ] < (12*60) )
			{
				return( TRUE );
			}
		}
	}
	return( FALSE );
}


static INT8 GetEmptyScheduleEntry(SCHEDULENODE* pSchedule)
{
	INT8			bLoop;

	for ( bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++ )
	{
		if ( pSchedule->ubAction[ bLoop ] == SCHEDULE_ACTION_NONE )
		{
			return( bLoop );
		}
	}

	return( -1 );
}


static UINT16 FindSleepSpot(SCHEDULENODE* pSchedule)
{
	INT8			bLoop;

	for ( bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++ )
	{
		if ( pSchedule->ubAction[ bLoop ] == SCHEDULE_ACTION_SLEEP )
		{
			return( pSchedule->usData1[ bLoop ] );
		}
	}
	return( NOWHERE );
}


static void ReplaceSleepSpot(SCHEDULENODE* pSchedule, UINT16 usNewSpot)
{
	INT8			bLoop;

	for ( bLoop = 0; bLoop < MAX_SCHEDULE_ACTIONS; bLoop++ )
	{
		if ( pSchedule->ubAction[ bLoop ] == SCHEDULE_ACTION_SLEEP )
		{
			pSchedule->usData1[ bLoop ] = usNewSpot;
			break;
		}
	}
}


static void SecureSleepSpot(SOLDIERTYPE* const pSoldier, UINT16 const usSleepSpot)
{
	// start after this soldier's ID so we don't duplicate work done in previous passes
	for (UINT32 i = pSoldier->ubID + 1; i <= gTacticalStatus.Team[CIV_TEAM].bLastID; ++i)
	{
		SOLDIERTYPE const& s2 = GetMan(i);
		if (!s2.bActive || !s2.bInSector || s2.ubScheduleID == 0) continue;

		SCHEDULENODE* const pSchedule = GetSchedule(s2.ubScheduleID);
		if (!pSchedule) continue;

		UINT16 const usSleepSpot2 = FindSleepSpot(pSchedule);
		if (usSleepSpot2 != usSleepSpot) continue;

		// conflict!
		//UINT8 ubDirection;
		//const UINT16 usNewSleepSpot = FindGridNoFromSweetSpotWithStructData(&s2, s2.usAnimState, usSleepSpot2, 3, &ubDirection, FALSE);
		UINT16 const usNewSleepSpot = FindGridNoFromSweetSpotExcludingSweetSpot(&s2, usSleepSpot2, 3);
		if (usNewSleepSpot == NOWHERE) continue;

		ReplaceSleepSpot(pSchedule, usNewSleepSpot);
	}
}
