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

#ifdef PRECOMPILEDHEADERS
	#include "AI All.h"
#else
	#include "AIList.h"
	#include "Overhead.h"
	#include "debug.h"
	#include "AIInternals.h"
#endif

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

UINT8 gubEncryptionArray1[ BASE_NUMBER_OF_ROTATION_ARRAYS * 3 ][ NEW_ROTATION_ARRAY_SIZE ] =
{
    {
    11,129,18,136,163,80,128,
    53,174,146,188,240,208,162,
    127,192,251,6,52,128,119,
    123,234,131,60,66,171,237,
    89,192,17,37,139,20,185,
    48,218,176,116,87,91,156,
    166,224,215,100,237,71,157
    },

    {
    99,93,179,25,113,109,38,
    135,144,248,17,108,178,84,
    210,254,54,240,209,92,35,
    121,166,153,203,239,27,134,
    231,193,97,73,188,59,87,
    146,111,103,48,161,25,196,
    84,201,179,208,220,189,113
    },

    {
    217,122,77,162,22,25,203,
    190,115,43,235,154,27,227,
    44,125,34,58,157,17,83,
    26,63,221,159,135,149,85,
    26,32,201,22,253,189,250,
    113,185,171,252,214,115,41,
    65,49,3,174,27,220,206
    },

    {
    180,29,63,146,101,192,164,
    128,7,28,111,172,125,203,
    63,77,184,220,27,184,98,
    224,64,156,178,229,167,105,
    169,147,215,103,168,25,40,
    114,156,45,253,198,179,207,
    35,235,225,20,205,34,14
    },

    {
    144,113,83,76,208,239,2,
    236,97,66,38,195,185,52,
    121,123,196,66,34,78,50,
    66,57,247,79,216,38,63,
    155,107,62,234,218,142,175,
    125,209,159,31,135,99,143,
    250,243,90,127,249,32,121
    },

    {
    76,189,22,40,143,9,197,
    91,66,142,145,3,138,202,
    4,170,214,219,20,140,212,
    214,183,197,98,142,57,235,
    16,71,13,186,77,72,147,
    14,206,204,152,226,86,66,
    219,148,100,89,148,22,243
    },

    {
    206,52,86,217,117,60,134,
    215,114,154,137,4,37,66,
    48,147,170,113,137,175,130,
    112,246,87,69,40,64,249,
    227,97,91,216,102,11,40,
    195,173,91,205,85,32,40,
    248,63,194,73,172,181,184
    },

    {
    95,234,168,147,217,213,204,
    132,140,158,22,201,182,222,
    191,104,206,165,108,154,226,
    236,135,245,247,166,112,180,
    49,43,146,43,23,40,249,
    206,228,226,135,211,33,241,
    74,214,104,47,44,132,187
    },

    {
    26,121,193,40,90,98,61,
    42,168,13,248,146,203,80,
    166,90,223,172,107,45,168,
    161,239,10,34,239,59,209,
    4,231,81,23,120,85,12,
    12,182,35,39,229,187,83,
    243,142,218,94,179,241,90
    },

    {
    142,9,212,220,121,125,115,
    191,40,144,216,56,6,76,
    46,90,51,206,241,201,77,
    202,90,67,73,116,203,199,
    115,197,174,133,56,174,132,
    91,185,8,143,5,221,216,
    232,211,225,219,86,5,16
    },

    {
    195,200,131,175,42,189,78,
    186,174,122,45,142,55,106,
    248,10,204,112,38,82,5,
    154,209,193,124,244,5,29,
    79,237,97,175,16,8,250,
    93,44,79,143,63,64,84,
    205,244,74,6,242,42,58
    },

    {
    82,107,124,208,214,175,77,
    77,123,32,162,220,87,3,
    156,62,171,114,178,161,88,
    102,51,47,152,147,54,41,
    105,91,137,53,107,125,183,
    174,142,207,195,133,127,164,
    165,161,2,37,44,65,60
    },

    {
    79,86,68,37,213,189,232,
    15,143,41,230,136,191,222,
    103,133,217,205,204,23,10,
    45,197,140,55,160,166,99,
    169,119,117,198,35,51,178,
    15,139,203,66,8,252,236,
    125,90,88,169,222,244,35
    },

    {
    206,62,201,224,92,24,183,
    166,85,215,246,29,100,157,
    224,164,126,118,116,219,18,
    120,92,60,230,206,224,4,
    51,19,146,37,19,255,11,
    123,233,236,157,232,207,13,
    66,177,158,229,169,180,180
    },

    {
    254,188,196,16,50,108,215,
    198,166,37,72,138,227,191,
    212,60,245,70,171,57,30,
    97,27,98,4,211,66,152,
    225,198,91,154,166,86,234,
    10,124,31,130,130,182,56,
    85,134,152,104,78,51,237
    },

    {
    236,233,113,237,175,131,167,
    99,63,81,142,139,103,199,
    62,171,185,154,14,203,229,
    21,89,153,52,2,198,224,
    253,212,40,155,123,88,134,
    5,188,144,130,185,34,11,
    131,71,64,82,241,56,101
    },

    {
    201,241,254,183,146,209,209,
    56,204,180,211,122,211,146,
    154,117,190,49,116,68,216,
    29,125,35,65,69,224,144,
    123,84,91,223,20,83,90,
    22,36,186,200,111,220,126,
    240,96,223,53,112,101,142
    },

    {
    243,182,138,24,233,13,236,
    111,101,139,30,233,72,14,
    184,190,152,139,8,27,43,
    209,11,1,28,32,184,180,
    23,136,221,224,23,172,92,
    54,164,174,55,183,64,226,
    55,110,252,49,136,184,237
    },

    {
    168,31,242,128,40,66,176,
    76,175,24,54,137,56,123,
    117,172,160,14,64,79,145,
    189,229,248,232,177,1,228,
    140,236,147,75,46,114,76,
    98,236,88,11,153,87,150,
    113,38,187,224,182,142,66
    },

    {
    122,108,52,202,69,109,124,
    11,236,192,188,209,141,143,
    107,2,161,242,191,43,149,
    213,80,56,214,152,203,56,
    184,181,95,171,8,24,230,
    90,155,103,126,27,205,246,
    138,151,25,142,214,114,214
    },

    {
    213,206,212,75,128,138,215,
    3,9,165,183,182,42,136,
    95,160,201,65,37,150,152,
    120,86,194,18,154,95,72,
    32,216,51,191,18,14,196,
    210,34,39,38,205,70,33,
    49,149,117,58,69,184,47
    },

    {
    103,250,19,106,176,94,193,
    170,129,145,215,119,136,99,
    150,230,68,189,140,121,205,
    122,113,250,249,128,25,160,
    111,175,78,31,194,145,244,
    241,50,63,74,129,28,126,
    145,114,195,134,27,192,129
    },

    {
    105,226,237,237,149,175,252,
    47,105,12,92,232,88,227,
    150,6,135,43,231,207,108,
    96,125,141,94,67,149,47,
    140,149,47,26,91,70,142,
    191,156,210,71,9,152,66,
    2,50,96,134,186,44,99
    },

    {
    170,175,37,193,148,175,69,
    82,195,14,57,200,212,237,
    173,12,48,17,66,33,172,
    253,26,162,149,146,154,80,
    201,219,119,146,111,85,150,
    130,251,78,244,97,242,176,
    24,248,51,221,36,223,33
    },

    {
    213,62,25,152,108,57,4,
    234,182,62,27,201,109,115,
    108,127,138,80,90,52,185,
    155,125,184,249,199,67,143,
    9,170,32,163,226,66,228,
    119,8,136,33,43,190,251,
    25,44,15,242,48,231,170
    },

    {
    69,91,100,123,125,126,3,
    16,15,53,81,162,184,255,
    71,231,166,75,180,107,111,
    70,76,164,128,12,27,134,
    15,3,189,185,48,3,58,
    65,38,37,200,190,124,195,
    106,2,12,153,166,58,138
    },

    {
    123,65,90,159,168,71,9,
    59,29,75,192,44,243,30,
    96,151,51,235,228,107,25,
    10,122,99,77,82,172,172,
    238,98,218,213,14,151,193,
    17,99,33,53,103,222,249,
    211,243,248,47,243,195,56
    },

    {
    56,101,106,235,205,98,227,
    149,9,73,232,235,35,111,
    57,2,186,181,5,123,226,
    25,5,73,234,254,89,240,
    29,109,202,196,100,234,133,
    21,35,57,225,202,22,171,
    86,96,154,71,43,190,51
    },

    {
    113,4,220,161,78,222,190,
    105,166,150,161,29,113,135,
    244,63,197,107,244,34,109,
    12,130,139,204,212,9,130,
    89,234,19,21,88,222,93,
    119,22,135,8,75,90,48,
    186,108,73,157,75,30,108
    },

    {
    17,212,231,18,216,185,255,
    206,139,136,155,69,129,54,
    249,247,173,184,136,88,86,
    87,234,205,238,209,80,233,
    120,180,224,128,177,228,157,
    229,131,214,148,251,230,103,
    174,80,190,201,4,2,208
    },

    {
    146,254,166,111,69,45,224,
    229,101,23,235,77,130,235,
    201,210,60,113,78,20,166,
    239,198,159,25,140,106,57,
    210,220,85,242,120,14,58,
    186,211,127,152,111,219,20,
    147,21,114,47,54,65,221
    },

    {
    155,85,87,94,68,199,179,
    247,16,43,80,193,182,242,
    2,222,241,243,234,109,211,
    36,48,131,165,204,150,166,
    31,224,17,175,90,178,226,
    156,73,193,106,70,25,207,
    45,110,76,9,57,33,236
    },

    {
    92,120,224,167,215,16,147,
    48,40,91,171,186,7,158,
    131,243,194,20,101,62,158,
    82,46,144,65,190,23,223,
    233,24,132,74,164,166,105,
    255,50,68,41,33,225,183,
    114,136,18,144,18,185,137
    },

    {
    17,38,234,111,64,195,105,
    13,188,100,59,18,103,53,
    94,165,126,168,103,216,69,
    76,238,220,198,89,149,145,
    208,249,2,180,199,181,161,
    111,85,255,154,106,166,101,
    176,38,55,72,130,223,209
    },

    {
    167,241,154,215,233,21,164,
    109,34,121,61,150,197,43,
    152,236,80,57,1,247,197,
    38,56,196,145,26,241,225,
    184,26,125,57,215,61,108,
    5,217,25,106,86,248,82,
    244,93,228,157,51,255,155
    },

    {
    17,171,212,222,22,253,38,
    199,218,104,6,108,146,77,
    24,18,109,109,32,217,136,
    11,245,112,196,192,233,176,
    199,40,69,165,19,35,228,
    253,123,233,162,79,218,165,
    141,47,92,202,150,152,45
    },

    {
    225,196,143,248,9,135,132,
    255,208,73,103,33,197,201,
    226,174,65,101,190,110,210,
    217,216,142,238,169,181,244,
    21,167,20,177,57,211,138,
    7,141,14,153,186,125,207,
    52,40,255,101,49,219,7
    },

    {
    221,161,17,57,52,123,239,
    174,209,155,220,246,5,150,
    183,9,120,65,88,110,87,
    121,50,70,26,37,101,116,
    179,214,27,159,28,19,139,
    66,223,181,127,121,199,102,
    162,222,47,153,130,15,38
    },

    {
    17,153,36,244,88,45,209,
    13,52,102,102,28,128,115,
    92,56,99,16,189,213,111,
    73,11,146,130,213,230,111,
    254,221,49,151,65,127,210,
    67,208,249,241,212,65,196,
    197,202,67,119,183,143,207
    },

    {
    215,34,213,248,221,72,132,
    67,21,208,217,15,31,125,
    206,1,200,69,102,231,110,
    158,231,21,29,36,182,156,
    61,82,201,119,35,142,102,
    8,89,59,54,229,1,5,
    70,123,34,41,23,101,57
    },

    {
    237,73,8,231,245,255,96,
    23,192,156,114,102,247,212,
    103,60,202,216,189,92,25,
    135,142,70,147,87,94,74,
    147,3,108,158,231,159,5,
    78,65,36,186,99,128,2,
    18,25,152,230,144,115,68
    },

    {
    67,160,17,62,161,74,151,
    54,117,56,133,66,108,253,
    205,246,229,26,15,240,160,
    164,184,238,53,49,8,79,
    128,206,34,19,134,160,42,
    46,244,13,175,78,210,242,
    70,225,118,232,164,133,95
    },

    {
    234,31,155,10,134,212,66,
    76,236,106,93,102,226,169,
    245,73,20,225,147,59,178,
    82,227,106,224,136,73,140,
    225,8,166,24,129,254,2,
    60,177,128,86,36,201,254,
    46,168,120,115,83,251,187
    },

    {
    250,68,115,1,74,167,252,
    185,210,119,96,136,228,20,
    226,83,178,12,92,93,137,
    176,99,183,16,82,157,33,
    240,58,214,190,190,59,230,
    162,107,243,19,160,47,144,
    237,98,48,50,92,59,202
    },

    {
    200,215,171,200,126,118,10,
    108,131,226,88,154,98,215,
    172,254,156,39,46,143,200,
    255,29,50,226,118,222,231,
    11,231,124,91,113,98,162,
    28,148,147,126,109,164,97,
    127,240,42,199,123,23,180
    },

    {
    12,31,155,207,220,35,247,
    228,132,83,16,83,176,193,
    3,101,248,53,107,111,184,
    58,242,191,69,225,126,151,
    54,140,231,245,181,216,122,
    88,139,10,38,53,235,254,
    183,99,170,32,231,44,224
    },

    {
    58,165,53,170,203,99,6,
    7,176,212,91,123,166,220,
    253,142,168,30,92,181,147,
    230,151,199,9,241,63,145,
    18,185,118,166,114,193,176,
    248,40,177,209,100,8,139,
    202,181,248,231,144,70,195
    },

    {
    130,125,204,233,239,251,121,
    125,221,146,233,122,172,27,
    30,135,191,89,246,172,84,
    164,101,232,68,230,205,26,
    131,152,212,69,170,4,10,
    157,124,120,171,85,158,175,
    253,165,128,27,100,7,165
    },

    {
    73,196,31,75,40,86,63,
    169,84,115,12,176,43,123,
    51,162,223,112,169,80,73,
    82,110,99,145,226,70,121,
    158,222,108,131,72,70,172,
    32,48,207,219,106,99,30,
    51,249,130,195,120,123,171
    },

    {
    200,109,185,213,166,141,95,
    219,114,120,103,244,57,23,
    111,193,41,233,225,180,176,
    121,66,10,13,97,114,5,
    125,233,83,167,164,179,138,
    170,44,99,209,215,82,191,
    160,149,104,78,138,201,227
    },

    {
    213,42,215,30,187,239,231,
    72,204,179,60,193,67,75,
    188,94,173,32,126,173,132,
    254,121,236,7,2,33,168,
    68,249,31,180,211,23,228,
    135,44,63,68,159,20,222,
    221,240,43,142,123,23,244
    },

    {
    94,48,61,112,51,92,118,
    161,242,95,128,16,237,38,
    159,179,81,76,164,62,254,
    14,169,65,160,197,179,44,
    81,30,183,138,178,196,54,
    141,197,193,234,76,97,64,
    186,179,158,235,115,169,11
    },

    {
    168,114,74,10,218,199,193,
    41,245,250,186,205,131,243,
    215,15,105,59,42,17,150,
    13,40,194,220,108,22,72,
    37,197,83,249,187,57,148,
    64,234,33,109,51,156,254,
    53,72,55,68,174,240,117
    },

    {
    55,124,124,107,105,194,22,
    114,242,8,53,46,244,181,
    96,111,133,70,140,37,165,
    131,1,12,65,220,11,103,
    233,72,92,101,91,163,116,
    125,83,15,224,107,242,66,
    178,12,37,63,4,213,48
    },

    {
    115,221,24,12,172,92,193,
    137,224,105,238,239,70,221,
    224,35,164,110,174,229,104,
    50,86,88,100,25,108,68,
    195,55,41,87,21,47,249,
    212,245,152,96,232,85,190,
    77,39,177,136,180,82,111
    },

    {
    231,138,6,145,159,235,88,
    248,198,164,9,130,209,202,
    96,14,128,170,71,217,142,
    180,183,53,110,173,41,252,
    249,214,242,217,100,66,95,
    181,71,11,111,109,35,175,
    240,32,51,69,251,41,220
    },

    {
    177,114,53,161,177,151,101,
    226,222,87,255,85,102,86,
    161,114,178,99,106,206,188,
    98,29,157,53,65,169,67,
    74,31,24,83,60,48,80,
    186,116,113,179,144,90,220,
    252,71,167,25,110,167,94
    },

};
