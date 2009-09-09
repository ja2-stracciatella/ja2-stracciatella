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

static AILIST  gAIList[AI_LIST_SIZE];
static AILIST* gpFirstAIListEntry;


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


static AILIST* CreateNewAIListEntry(SOLDIERTYPE* const s, INT8 const priority)
{
	for (AILIST* i = gAIList; i != endof(gAIList); ++i)
	{
		AILIST& e = *i;
		if (e.soldier) continue;
		e.soldier   = s;
		e.bPriority = priority;
		e.pNext     = 0;
		return &e;
	}
	throw std::runtime_error("out of AI list entries");
}


static BOOLEAN SatisfiesAIListConditions(SOLDIERTYPE* pSoldier, UINT8* pubDoneCount, BOOLEAN fDoRandomChecks);


SOLDIERTYPE* RemoveFirstAIListEntry()
{
	while (AILIST* const e = gpFirstAIListEntry)
	{
		gpFirstAIListEntry = e->pNext;
		SOLDIERTYPE* const s = e->soldier;
		DeleteAIListEntry(e);
		// Make sure conditions still met
		if (SatisfiesAIListConditions(s, 0, FALSE)) return s;
	}
	return 0;
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


static void InsertIntoAIList(SOLDIERTYPE* const s, INT8 const priority)
{
	AILIST* const e = CreateNewAIListEntry(s, priority);
	// Look through the list to see where to insert the entry
	AILIST** anchor;
	for (anchor = &gpFirstAIListEntry; *anchor; anchor = &(*anchor)->pNext)
	{
		if (priority > (*anchor)->bPriority) break;
	}
	e->pNext = *anchor;
	*anchor  = e;
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
	AILIST *	pNewEntry;

	if ( !SatisfiesAIListConditions(s, NULL, FALSE))
	{
		// can't do dat!
		return( FALSE );
	}

	RemoveAIListEntryForID(s);

	if (gpFirstAIListEntry == NULL)
	{
		InsertIntoAIList(s, MAX_AI_PRIORITY);
	}
	else
	{
		bPriority = gpFirstAIListEntry->bPriority;
		pNewEntry = CreateNewAIListEntry(s, bPriority);

		// insert at front
		pNewEntry->pNext = gpFirstAIListEntry;
		gpFirstAIListEntry = pNewEntry;
	}
	return TRUE;
}


bool BuildAIListForTeam(INT8 const team)
{
	// This team is being given control so reset their muzzle flashes
	TurnOffTeamsMuzzleFlashes(team);

	ClearAIList();

	// Create a new list
	UINT8 n      = 0;
	UINT8 n_done = 0;
	FOR_ALL_MERCS(i)
	{
		SOLDIERTYPE& s = **i;
		// non-null merc slot ensures active
		if (s.bTeam != team) continue;
		if (!SatisfiesAIListConditions(&s, &n_done, TRUE)) continue;

		INT8 priority = s.bAlertStatus;
		if (s.bVisible == TRUE) priority += 3;

		InsertIntoAIList(&s, priority);
		++n;
	}

	if (n > 0) InitEnemyUIBar(n, n_done);
	return n > 0;
}
