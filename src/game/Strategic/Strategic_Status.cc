#include "Strategic_Status.h"
#include "Inventory_Choosing.h"
#include "FileMan.h"
#include "Soldier_Profile.h"
#include "Campaign.h"
#include "Strategic_Mines.h"
#include "EMail.h"
#include "Game_Clock.h"
#include "GameSettings.h"
#include "History.h"
#include "Strategic_Town_Loyalty.h"
#include "Debug.h"


STRATEGIC_STATUS gStrategicStatus;

void InitStrategicStatus(void)
{
	memset( &gStrategicStatus, 0, sizeof( STRATEGIC_STATUS ) );
	//Add special non-zero start conditions here...

	InitArmyGunTypes();
}


void SaveStrategicStatusToSaveGameFile(HWFILE const hFile)
{
	//Save the Strategic Status structure to the saved game file
	FileWrite(hFile, &gStrategicStatus, sizeof(STRATEGIC_STATUS));
}


void LoadStrategicStatusFromSaveGameFile(HWFILE const hFile)
{
	//Load the Strategic Status structure from the saved game file
	FileRead(hFile, &gStrategicStatus, sizeof(STRATEGIC_STATUS));
}


#define DEATH_RATE_SEVERITY 1.0f			// increase to make death rates higher for same # of deaths/time

UINT8 CalcDeathRate(void)
{
	UINT32 uiDeathRate = 0;

	// give the player a grace period of 1 day
	if (gStrategicStatus.uiManDaysPlayed > 0)
	{
		// calculates the player's current death rate
		uiDeathRate = (UINT32) ((gStrategicStatus.ubMercDeaths * DEATH_RATE_SEVERITY * 100) / gStrategicStatus.uiManDaysPlayed);
	}

	return((UINT8) uiDeathRate);
}


void ModifyPlayerReputation(INT8 bRepChange)
{
	INT32 iNewBadRep;

	// subtract, so that a negative reputation change results in an increase in bad reputation
	iNewBadRep = (INT32) gStrategicStatus.ubBadReputation - bRepChange;

	// keep within a 0-100 range (0 = Saint, 100 = Satan)
	iNewBadRep = __max(   0, iNewBadRep );
	iNewBadRep = __min( 100, iNewBadRep );

	gStrategicStatus.ubBadReputation = (UINT8) iNewBadRep;
}


BOOLEAN MercThinksDeathRateTooHigh(MERCPROFILESTRUCT const& p)
{
	INT8 const bDeathRateTolerance = p.bDeathRate;

	// if he couldn't care less what it is
	if (bDeathRateTolerance == 101)
	{
		// then obviously it CAN'T be too high...
		return(FALSE);
	}

	if (CalcDeathRate() > bDeathRateTolerance)
	{
		// too high - sorry
		return(TRUE);
	}
	else
	{
		// within tolerance
		return(FALSE);
	}
}


BOOLEAN MercThinksBadReputationTooHigh(MERCPROFILESTRUCT const& p)
{
	INT8 const bRepTolerance = p.bReputationTolerance;

	// if he couldn't care less what it is
	if (bRepTolerance == 101)
	{
		// then obviously it CAN'T be too high...
		return(FALSE);
	}

	if (gStrategicStatus.ubBadReputation > bRepTolerance)
	{
		// too high - sorry
		return(TRUE);
	}
	else
	{
		// within tolerance
		return(FALSE);
	}
}


// only meaningful for already hired mercs
BOOLEAN MercThinksHisMoraleIsTooLow(SOLDIERTYPE const* const pSoldier)
{
	INT8	bRepTolerance;
	INT8	bMoraleTolerance;

	bRepTolerance = gMercProfiles[ pSoldier->ubProfile ].bReputationTolerance;

	// if he couldn't care less what it is
	if (bRepTolerance == 101)
	{
		// that obviously it CAN'T be too low...
		return(FALSE);
	}


	// morale tolerance is based directly upon reputation tolerance
	// above 50, morale is GOOD, never below tolerance then
	bMoraleTolerance = (100 - bRepTolerance) / 2;

	if (pSoldier->bMorale < bMoraleTolerance)
	{
		// too low - sorry
		return(TRUE);
	}
	else
	{
		// within tolerance
		return(FALSE);
	}
}

void UpdateLastDayOfPlayerActivity( UINT16 usDay )
{
	if ( usDay > gStrategicStatus.usLastDayOfPlayerActivity )
	{
		gStrategicStatus.usLastDayOfPlayerActivity = usDay;
		gStrategicStatus.ubNumberOfDaysOfInactivity = 0;
	}
}


static UINT8 LackOfProgressTolerance(void)
{
	if ( gGameOptions.ubDifficultyLevel >= DIF_LEVEL_HARD )
	{
		// give an EXTRA day over normal
		return( 7 - DIF_LEVEL_MEDIUM + gStrategicStatus.ubHighestProgress / 42 );
	}
	else
	{
		return( 6 - gGameOptions.ubDifficultyLevel + gStrategicStatus.ubHighestProgress / 42 );
	}
}


// called once per day in the morning, decides whether Enrico should send any new E-mails to the player
void HandleEnricoEmail(void)
{
	UINT8 ubCurrentProgress = CurrentPlayerProgressPercentage();
	UINT8 ubHighestProgress = HighestPlayerProgressPercentage();

	// if creatures have attacked a mine (doesn't care if they're still there or not at the moment)
	if (HasAnyMineBeenAttackedByMonsters() && !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_CREATURES))
	{
		AddEmail(ENRICO_CREATURES, ENRICO_CREATURES_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
		gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_CREATURES;
		return;	// avoid any other E-mail at the same time
	}


	if ((ubCurrentProgress >= SOME_PROGRESS_THRESHOLD) && !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_SOME_PROGRESS))
	{
		AddEmail(ENRICO_PROG_20, ENRICO_PROG_20_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
		gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_SOME_PROGRESS;
		return;	// avoid any setback E-mail at the same time
	}

	if ((ubCurrentProgress >= ABOUT_HALFWAY_THRESHOLD) && !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_ABOUT_HALFWAY))
	{
		AddEmail(ENRICO_PROG_55, ENRICO_PROG_55_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
		gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_ABOUT_HALFWAY;
		return;	// avoid any setback E-mail at the same time
	}

	if ((ubCurrentProgress >= NEARLY_DONE_THRESHOLD) && !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_NEARLY_DONE))
	{
		AddEmail(ENRICO_PROG_80, ENRICO_PROG_80_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
		gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_NEARLY_DONE;
		return;	// avoid any setback E-mail at the same time
	}

	// test for a major setback OR a second minor setback
	if ((((ubHighestProgress - ubCurrentProgress) >= MAJOR_SETBACK_THRESHOLD) ||
		(((ubHighestProgress - ubCurrentProgress) >= MINOR_SETBACK_THRESHOLD) && (gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_FLAG_SETBACK_OVER))) &&
		!(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_MAJOR_SETBACK))
	{
		AddEmail(ENRICO_SETBACK, ENRICO_SETBACK_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
		gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_MAJOR_SETBACK;
	}
	else
	// test for a first minor setback
	if (((ubHighestProgress - ubCurrentProgress) >= MINOR_SETBACK_THRESHOLD) &&
		!(gStrategicStatus.usEnricoEmailFlags & (ENRICO_EMAIL_SENT_MINOR_SETBACK | ENRICO_EMAIL_SENT_MAJOR_SETBACK)))
	{
		AddEmail(ENRICO_SETBACK_2, ENRICO_SETBACK_2_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
		gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_MINOR_SETBACK;
	}
	else
	// if player is back at his maximum progress after having suffered a minor setback
	if ((ubHighestProgress == ubCurrentProgress) && (gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_MINOR_SETBACK))
	{
		// remember that the original setback has been overcome, so another one can generate another E-mail
		gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_FLAG_SETBACK_OVER;
	}
	else if ( GetWorldDay() > (UINT32) (gStrategicStatus.usLastDayOfPlayerActivity) )
	{
		INT8			bComplaint = 0;
		UINT8			ubTolerance;

		gStrategicStatus.ubNumberOfDaysOfInactivity++;
		ubTolerance = LackOfProgressTolerance();

		if ( gStrategicStatus.ubNumberOfDaysOfInactivity >= ubTolerance )
		{
			if ( gStrategicStatus.ubNumberOfDaysOfInactivity == ubTolerance )
			{
				// send email
				if ( !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_LACK_PROGRESS1 ) )
				{
					bComplaint = 1;
				}
				else if ( !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_LACK_PROGRESS2 ) )
				{
					bComplaint = 2;
				}
				else if ( !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_LACK_PROGRESS3 ) )
				{
					bComplaint = 3;
				}
			}
			else if ( gStrategicStatus.ubNumberOfDaysOfInactivity == (UINT32) ubTolerance * 2 )
			{
				// six days? send 2nd or 3rd message possibly
				if ( !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_LACK_PROGRESS2 ) )
				{
					bComplaint = 2;
				}
				else if ( !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_LACK_PROGRESS3 ) )
				{
					bComplaint = 3;
				}

			}
			else if ( gStrategicStatus.ubNumberOfDaysOfInactivity == ubTolerance * 3 )
			{
				// nine days??? send 3rd message possibly
				if ( !(gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_LACK_PROGRESS3 ) )
				{
					bComplaint = 3;
				}
			}

			if ( bComplaint != 0 )
			{
				switch( bComplaint )
				{
					case 3:
						AddEmail(LACK_PLAYER_PROGRESS_3, LACK_PLAYER_PROGRESS_3_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
						gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_LACK_PROGRESS3;
						break;
					case 2:
						AddEmail(LACK_PLAYER_PROGRESS_2, LACK_PLAYER_PROGRESS_2_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
						gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_LACK_PROGRESS2;
						break;
					default:
						AddEmail(LACK_PLAYER_PROGRESS_1, LACK_PLAYER_PROGRESS_1_LENGTH, MAIL_ENRICO, GetWorldTotalMin());
						gStrategicStatus.usEnricoEmailFlags |= ENRICO_EMAIL_SENT_LACK_PROGRESS1;
						break;

				}

				AddHistoryToPlayersLog( HISTORY_ENRICO_COMPLAINED, 0, GetWorldTotalMin(), -1, -1 );
			}

			// penalize loyalty!
			if ( gStrategicStatus.usEnricoEmailFlags & ENRICO_EMAIL_SENT_LACK_PROGRESS2 )
			{
				DecrementTownLoyaltyEverywhere( LOYALTY_PENALTY_INACTIVE * (gStrategicStatus.ubNumberOfDaysOfInactivity - LackOfProgressTolerance() + 1) );
			}
			else
			{
				// on first complaint, give a day's grace...
				DecrementTownLoyaltyEverywhere( LOYALTY_PENALTY_INACTIVE * (gStrategicStatus.ubNumberOfDaysOfInactivity - LackOfProgressTolerance() ) );
			}
		}
	}

	// reset # of new sectors visited 'today'
	// grant some leeway for the next day, could have started moving
	// at night...
	gStrategicStatus.ubNumNewSectorsVisitedToday = __min( gStrategicStatus.ubNumNewSectorsVisitedToday, NEW_SECTORS_EQUAL_TO_ACTIVITY ) / 3;
}


static INT8 SoldierClassToRankIndex(UINT8 ubSoldierClass);


void TrackEnemiesKilled( UINT8 ubKilledHow, UINT8 ubSoldierClass )
{
	INT8 bRankIndex;

	bRankIndex = SoldierClassToRankIndex( ubSoldierClass );

	// if it's not a standard enemy-class soldier
	if ( bRankIndex == -1 )
	{
		// don't count him as an enemy
		return;
	}

	gStrategicStatus.usEnemiesKilled[ ubKilledHow ][ bRankIndex ]++;

	if ( ubKilledHow != ENEMY_KILLED_TOTAL )
	{
		gStrategicStatus.usEnemiesKilled[ ENEMY_KILLED_TOTAL ][ bRankIndex ]++;
	}
}


static INT8 SoldierClassToRankIndex(UINT8 ubSoldierClass)
{
	INT8 bRankIndex = -1;

	// the soldier class defines are not in natural ascending order, elite comes before army!
	switch ( ubSoldierClass )
	{
		case SOLDIER_CLASS_ADMINISTRATOR:	bRankIndex = 0;	break;
		case SOLDIER_CLASS_ELITE:					bRankIndex = 2;	break;
		case SOLDIER_CLASS_ARMY:					bRankIndex = 1;	break;

		default:
			// this happens when an NPC joins the enemy team (e.g. Conrad, Iggy, Mike)
			break;
	}

	return( bRankIndex );
}



UINT8 RankIndexToSoldierClass( UINT8 ubRankIndex )
{
	UINT8 ubSoldierClass = 0;

	Assert( ubRankIndex < NUM_ENEMY_RANKS );

	switch ( ubRankIndex )
	{
		case 0:	ubSoldierClass = SOLDIER_CLASS_ADMINISTRATOR;	break;
		case 1: ubSoldierClass = SOLDIER_CLASS_ARMY;					break;
		case 2: ubSoldierClass = SOLDIER_CLASS_ELITE;					break;
	}

	return( ubSoldierClass );
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(StrategicStatus, asserts)
{
	EXPECT_EQ(sizeof(STRATEGIC_STATUS), 192);
}

#endif
