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

#include "AIList.h"
#include "Overhead.h"
#include "Debug.h"
#include "AIInternals.h"
#include "AI.h"
#include "OppList.h"
#include "Interface.h"
#include "Tactical_Save.h"


#define AI_LIST_SIZE TOTAL_SOLDIERS

AILIST		gAIList[ AI_LIST_SIZE ];
AILIST *	gpFirstAIListEntry = NULL;

BOOLEAN SatisfiesAIListConditions( SOLDIERTYPE * pSoldier, UINT8 * pubDoneCount, BOOLEAN fDoRandomChecks );

void ClearAIList( void )
{
	UINT8	ubLoop;

	for ( ubLoop = 0; ubLoop < AI_LIST_SIZE; ubLoop++ )
	{
		gAIList[ ubLoop ].ubID = NOBODY;
		gAIList[ ubLoop ].bPriority = 0;
		gAIList[ ubLoop ].pNext = NULL;
	}
	gpFirstAIListEntry = NULL; // ??
}

void DeleteAIListEntry( AILIST *	pEntry )
{
	pEntry->ubID = NOBODY;
	pEntry->bPriority = 0;
	pEntry->pNext = NULL;
}

UINT8	FindEmptyAIListEntry( void )
{
	UINT8	ubLoop;

	for ( ubLoop = 0; ubLoop < AI_LIST_SIZE; ubLoop++ )
	{
		if ( gAIList[ ubLoop ].ubID == NOBODY )
		{
			return( ubLoop );
		}
	}
	AssertMsg( 0, "Fatal error: Could not find empty AI list entry." );
	return( AI_LIST_SIZE );
}

AILIST * CreateNewAIListEntry( UINT8 ubNewEntry, UINT8 ubID, INT8 bPriority )
{
	gAIList[ ubNewEntry ].ubID = ubID;
	gAIList[ ubNewEntry ].bPriority = bPriority;
	gAIList[ ubNewEntry ].pNext = NULL;
	return( &(gAIList[ ubNewEntry ]) );
}

UINT8 RemoveFirstAIListEntry( void )
{
	AILIST *	pOldFirstEntry;
	UINT8			ubID;

	while ( gpFirstAIListEntry != NULL)
	{
		// record pointer to start of list, and advance head ptr
		pOldFirstEntry = gpFirstAIListEntry;
		gpFirstAIListEntry = gpFirstAIListEntry->pNext;

		// record ID, and delete old now unused entry
		ubID = pOldFirstEntry->ubID;
		DeleteAIListEntry( pOldFirstEntry );

		// make sure conditions still met
		if ( SatisfiesAIListConditions( MercPtrs[ ubID ], NULL, FALSE ) )
		{
			return( ubID );
		}
	}

	return( NOBODY );
}

void RemoveAIListEntryForID( UINT8 ubID )
{
	AILIST *	pEntry;
	AILIST *	pPrevEntry;

	pEntry = gpFirstAIListEntry;
	pPrevEntry = NULL;

	while( pEntry != NULL )
	{
		if ( pEntry->ubID == ubID )
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

BOOLEAN InsertIntoAIList( UINT8 ubID, INT8 bPriority )
{
	UINT8			ubNewEntry;
	AILIST *	pEntry, * pNewEntry, * pPrevEntry = NULL;

	ubNewEntry = FindEmptyAIListEntry();

	pNewEntry = CreateNewAIListEntry( ubNewEntry, ubID, bPriority );

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

BOOLEAN SatisfiesAIListConditions( SOLDIERTYPE * pSoldier, UINT8 * pubDoneCount, BOOLEAN fDoRandomChecks )
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
	if (PTR_CIVILIAN)
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

BOOLEAN MoveToFrontOfAIList( UINT8 ubID )
{
	// we'll have to fake this guy's alert status (in the list) to be the same as the current
	// front of the list
	INT8			bPriority;
	UINT8			ubNewEntry;
	AILIST *	pNewEntry;

	if ( !SatisfiesAIListConditions( MercPtrs[ ubID ], NULL, FALSE ) )
	{
		// can't do dat!
		return( FALSE );
	}

	RemoveAIListEntryForID( ubID );

	if (gpFirstAIListEntry == NULL)
	{
		return( InsertIntoAIList( ubID, MAX_AI_PRIORITY ) );
	}
	else
	{
		bPriority = gpFirstAIListEntry->bPriority;
		ubNewEntry = FindEmptyAIListEntry();
		pNewEntry = CreateNewAIListEntry( ubNewEntry, ubID, bPriority );

		// insert at front
		pNewEntry->pNext = gpFirstAIListEntry;
		gpFirstAIListEntry = pNewEntry;
		return( TRUE );
	}
}

BOOLEAN BuildAIListForTeam( INT8 bTeam )
{
	// loop through all non-player-team guys and add to list
	UINT32					uiLoop;
	BOOLEAN					fInsertRet;
	SOLDIERTYPE *		pSoldier;
	BOOLEAN					fRet = FALSE;
	UINT8						ubCount = 0;
	UINT8						ubDoneCount = 0;
	INT8						bPriority;

	// this team is being given control so reset their muzzle flashes
	TurnOffTeamsMuzzleFlashes( bTeam );

	// clear the AI list
	ClearAIList();

	// create a new list
	for( uiLoop = 0; uiLoop < guiNumMercSlots; uiLoop++ )
	{
		// non-null merc slot ensures active
		pSoldier = MercSlots[ uiLoop ];
		if ( pSoldier && pSoldier->bTeam == bTeam )
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

			fInsertRet = InsertIntoAIList( pSoldier->ubID, bPriority );
			if (fInsertRet == FALSE)
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
