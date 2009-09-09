/*
 *
 * This file contains code for the AI linked list which
 * (in TURNBASED COMBAT) keeps track of which AI guy should
 * be handled next.
 *
 * The basic plan is to insert everyone into this list by their
 * alert status at the beginning of the turn, in descending order
 * (BLACK, then RED, then YELLOW, then GREEN)
 *
 */
#include <stdexcept>

#include "AIList.h"
#include "Animation_Data.h"
#include "Overhead.h"
#include "Debug.h"
#include "AIInternals.h"
#include "AI.h"
#include "OppList.h"
#include "Interface.h"


#define MAX_AI_PRIORITY 100


struct AILIST
{
	SOLDIERTYPE* soldier;
	INT8    bPriority;
	AILIST* pNext;
};


#define AI_LIST_SIZE TOTAL_SOLDIERS

AILIST		gAIList[ AI_LIST_SIZE ];
AILIST *	gpFirstAIListEntry = NULL;


static void DeleteAIListEntry(AILIST* pEntry)
{
	pEntry->soldier   = NULL;
	pEntry->bPriority = 0;
	pEntry->pNext     = NULL;
}


static void ClearAIList(void)
{
	UINT8	ubLoop;

	for ( ubLoop = 0; ubLoop < AI_LIST_SIZE; ubLoop++ )
	{
		DeleteAIListEntry(&gAIList[ubLoop]);
	}
	gpFirstAIListEntry = NULL; // ??
}


static UINT8 FindEmptyAIListEntry(void)
{
	UINT8	ubLoop;

	for ( ubLoop = 0; ubLoop < AI_LIST_SIZE; ubLoop++ )
	{
		if (gAIList[ubLoop].soldier == NULL) return ubLoop;
	}
	throw std::runtime_error("out of AI list entries");
}


static AILIST* CreateNewAIListEntry(UINT8 ubNewEntry, SOLDIERTYPE* s, INT8 bPriority)
{
	AILIST* const e = &gAIList[ubNewEntry];
	e->soldier   = s;
	e->bPriority = bPriority;
	e->pNext     = NULL;
	return e;
}


static BOOLEAN SatisfiesAIListConditions(SOLDIERTYPE* pSoldier, UINT8* pubDoneCount, BOOLEAN fDoRandomChecks);


SOLDIERTYPE* RemoveFirstAIListEntry(void)
{
	AILIST *	pOldFirstEntry;

	while ( gpFirstAIListEntry != NULL)
	{
		// record pointer to start of list, and advance head ptr
		pOldFirstEntry = gpFirstAIListEntry;
		gpFirstAIListEntry = gpFirstAIListEntry->pNext;

		// record ID, and delete old now unused entry
		SOLDIERTYPE* const s = pOldFirstEntry->soldier;
		DeleteAIListEntry( pOldFirstEntry );

		// make sure conditions still met
		if (SatisfiesAIListConditions(s, NULL, FALSE)) return s;
	}

	return NULL;
}


static void RemoveAIListEntryForID(const SOLDIERTYPE* s)
{
	AILIST *	pEntry;
	AILIST *	pPrevEntry;

	pEntry = gpFirstAIListEntry;
	pPrevEntry = NULL;

	while( pEntry != NULL )
	{
		if (pEntry->soldier == s)
		{
			if ( pEntry == gpFirstAIListEntry )
			{
				RemoveFirstAIListEntry();
			}
			else
			{
				pPrevEntry->pNext = pEntry->pNext;
				DeleteAIListEntry( pEntry );
			}
			return;
		}
		pPrevEntry = pEntry;
		pEntry = pEntry->pNext;
	}
	// none found, that's okay
}


static BOOLEAN InsertIntoAIList(SOLDIERTYPE* s, INT8 bPriority)
{
	UINT8			ubNewEntry;
	AILIST *	pEntry, * pNewEntry, * pPrevEntry = NULL;

	ubNewEntry = FindEmptyAIListEntry();

	pNewEntry = CreateNewAIListEntry(ubNewEntry, s, bPriority);

	// look through the list now to see where to insert the entry
	if (gpFirstAIListEntry == NULL)
	{
		// empty list!
		gpFirstAIListEntry = pNewEntry;
		// new entry's next ptr is null already
		return( TRUE );
	}
	else
	{
		pEntry = gpFirstAIListEntry;
		do
		{
			if ( bPriority > pEntry->bPriority )
			{
				// insert before this entry
				pNewEntry->pNext = pEntry;
				if (pEntry == gpFirstAIListEntry)
				{
					// inserting at head of list
					gpFirstAIListEntry = pNewEntry;
				}
				else
				{
					Assert(pPrevEntry != NULL);
					pPrevEntry->pNext = pNewEntry;
				}
				return( TRUE );
			}
			else if (pEntry->pNext == NULL)
			{
				// end of list!
				pEntry->pNext = pNewEntry;
				return( TRUE );
			}
			pPrevEntry = pEntry;
			pEntry = pEntry->pNext;
			AssertMsg( pEntry != gpFirstAIListEntry, "Fatal error: Loop in AI list!" );
		}
		while( pEntry != NULL );
	}

	// I don't know how the heck we would get here...
	return( FALSE );
}


static BOOLEAN SatisfiesAIListConditions(SOLDIERTYPE* pSoldier, UINT8* pubDoneCount, BOOLEAN fDoRandomChecks)
{
	if ( (gTacticalStatus.bBoxingState == BOXING) && !(pSoldier->uiStatusFlags & SOLDIER_BOXER) )
	{
		return( FALSE );
	}

	if ( ! ( pSoldier->bActive && pSoldier->bInSector ) )
	{
		// the check for
		return( FALSE );
	}

	if ( ! ( pSoldier->bLife >= OKLIFE && pSoldier->bBreath >= OKBREATH ) )
	{
		return( FALSE );
	}

	if ( pSoldier->bMoved )
	{
		if ( pSoldier->bActionPoints <= 1 && pubDoneCount )
		{
			(*pubDoneCount)++;
		}
		return( FALSE );
	}

	// do we need to re-evaluate alert status here?
	DecideAlertStatus( pSoldier );

	// if we are dealing with the civ team and this person
	// hasn't heard any gunfire, handle only 1 time in 10
	if (IsOnCivTeam(pSoldier))
	{
		if ( pSoldier->ubBodyType == CROW || pSoldier->ubBodyType == COW )
		{
			// don't handle cows and crows in TB!
			return( FALSE );
		}

		// if someone in a civ group is neutral but the civ group is non-neutral, should be handled all the time
		if ( pSoldier->bNeutral && (pSoldier->ubCivilianGroup == NON_CIV_GROUP || gTacticalStatus.fCivGroupHostile[pSoldier->ubCivilianGroup] == CIV_GROUP_NEUTRAL ) )
		{
			if ( pSoldier->bAlertStatus < STATUS_RED )
			{
				// unalerted, barely handle
				if ( fDoRandomChecks && PreRandom( 10 ) && !(pSoldier->ubQuoteRecord) )
				{
					return( FALSE );
				}
			}
			else
			{
				// heard gunshots
				if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
				{
					if ( pSoldier->bVisible )
					{
						// if have profile, don't handle, don't want them running around
						if ( pSoldier->ubProfile != NO_PROFILE )
						{
							return( FALSE );
						}
						// else don't handle much
						if ( fDoRandomChecks && PreRandom( 3 ) )
						{
							return( FALSE );
						}
					}
					else
					{
						// never handle
						return( FALSE );
					}
				}
				else if ( pSoldier->ubBodyType == COW || pSoldier->ubBodyType == CRIPPLECIV )
				{
					// don't handle much
					if ( fDoRandomChecks && PreRandom( 3 ) )
					{
						return( FALSE );
					}
				}
			}
		}
		// non-neutral civs should be handled all the time, right?
		// reset last action if cowering

		if ( pSoldier->uiStatusFlags & SOLDIER_COWERING )
		{
			pSoldier->bLastAction = AI_ACTION_NONE;
		}

	}

	return( TRUE );
}


BOOLEAN MoveToFrontOfAIList(SOLDIERTYPE* const s)
{
	// we'll have to fake this guy's alert status (in the list) to be the same as the current
	// front of the list
	INT8			bPriority;
	UINT8			ubNewEntry;
	AILIST *	pNewEntry;

	if ( !SatisfiesAIListConditions(s, NULL, FALSE))
	{
		// can't do dat!
		return( FALSE );
	}

	RemoveAIListEntryForID(s);

	if (gpFirstAIListEntry == NULL)
	{
		return InsertIntoAIList(s, MAX_AI_PRIORITY);
	}
	else
	{
		bPriority = gpFirstAIListEntry->bPriority;
		ubNewEntry = FindEmptyAIListEntry();
		pNewEntry = CreateNewAIListEntry(ubNewEntry, s, bPriority);

		// insert at front
		pNewEntry->pNext = gpFirstAIListEntry;
		gpFirstAIListEntry = pNewEntry;
		return( TRUE );
	}
}

BOOLEAN BuildAIListForTeam( INT8 bTeam )
{
	// loop through all non-player-team guys and add to list
	BOOLEAN					fInsertRet;
	BOOLEAN					fRet = FALSE;
	UINT8						ubCount = 0;
	UINT8						ubDoneCount = 0;
	INT8						bPriority;

	// this team is being given control so reset their muzzle flashes
	TurnOffTeamsMuzzleFlashes( bTeam );

	// clear the AI list
	ClearAIList();

	// create a new list
	FOR_ALL_MERCS(i)
	{
		SOLDIERTYPE* const pSoldier = *i;
		// non-null merc slot ensures active
		if (pSoldier->bTeam == bTeam)
		{
			if ( !SatisfiesAIListConditions( pSoldier, &ubDoneCount, TRUE ) )
			{
				continue;
			}

			bPriority = pSoldier->bAlertStatus;
			if ( pSoldier->bVisible == TRUE )
			{
				bPriority += 3;
			}

			fInsertRet = InsertIntoAIList(pSoldier, bPriority);
			if (!fInsertRet)
			{
				// wtf???
				break;
			}
			else
			{
				fRet = TRUE;
				ubCount++;
			}
		}
	}

	if (fRet && ubCount > 0)
	{
		InitEnemyUIBar( ubCount, ubDoneCount );
	}
	return( fRet );
}
