#include "Font_Control.h"
#include "Timer_Control.h"
#include "Debug.h"
#include "MemMan.h"
#include "Overhead_Types.h"
#include "Soldier_Control.h"
#include "Random.h"
#include "Campaign.h"
#include "Dialogue_Control.h"
#include "Message.h"
#include "Game_Clock.h"
#include "Strategic_Mines.h"
#include "Strategic_Status.h"
#include "Sys_Globals.h"
#include "Text.h"
#include "GameSettings.h"
#include "Assignments.h"
#include "MapScreen.h"
#include "Interface.h"
#include "Game_Event_Hook.h"
#include "Overhead.h"
#include "Meanwhile.h"
#include "Quests.h"
#include "Soldier_Macros.h"
#include "Squads.h"
#include "StrategicMap.h"
#include "Town_Militia.h"
#include "Types.h"
#include "EMail.h"
#include "Logger.h"

#include <string_theory/format>
#include <string_theory/string>

#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"

// Convert hired mercs' stats subpoint changes into actual point changes where warranted
static void ProcessUpdateStats(MERCPROFILESTRUCT&, SOLDIERTYPE*);

static void ProcessStatChange(MERCPROFILESTRUCT&, StatKind, UINT16 usNumChances, StatChangeCause);


// Give soldier n_chances to improve stat. If it's from training, it doesn't
// count towards experience level gain
void StatChange(SOLDIERTYPE& s, StatKind const stat, UINT16 const n_chances, StatChangeCause const reason)
{
	Assert(s.bActive);

	// Ignore non-player soldiers
	if (!IsOnOurTeam(s)) return;

	// Ignore anything without a profile
	if (s.ubProfile == NO_PROFILE) return;

	// Ignore vehicles and robots
	if (s.uiStatusFlags & SOLDIER_VEHICLE) return;
	if (s.uiStatusFlags & SOLDIER_ROBOT)   return;

	if (s.bAssignment == ASSIGNMENT_POW)
	{
		SLOGE("StatChange: %s improving stats while POW! stat %d", s.name.c_str(), stat);
		return;
	}

	// No points earned while somebody is unconscious (for assist XPs, and such)
	if (s.bLife < CONSCIOUSNESS) return;

	MERCPROFILESTRUCT& p = GetProfile(s.ubProfile);
	ProcessStatChange(p, stat, n_chances, reason);
	// ATE: Update stats right away
	ProcessUpdateStats(p, &s);
}


static void ProfileUpdateStats(MERCPROFILESTRUCT&);


// this is the equivalent of StatChange(), but for use with mercs not currently on player's team
// give pProfile usNumChances to improve ubStat.  If it's from training, it doesn't count towards experience level gain
static void ProfileStatChange(MERCPROFILESTRUCT& p, StatKind const ubStat, UINT16 const usNumChances, StatChangeCause const ubReason)
{
	// dead guys don't do nuthin' !
	if (p.bMercStatus == MERC_IS_DEAD)
		return;

	if (p.bLife < OKLIFE)
		return;

	ProcessStatChange(p, ubStat, usNumChances, ubReason);

	// Update stats....right away... ATE
	ProfileUpdateStats(p);
}


static UINT16 SubpointsPerPoint(StatKind, INT8 bExpLevel);


static void ProcessStatChange(MERCPROFILESTRUCT& p, StatKind const ubStat, UINT16 const usNumChances, StatChangeCause const ubReason)
{
	UINT32 uiCnt,uiEffLevel;
	INT16 sSubPointChange = 0;
	UINT16 usChance=0;
	UINT16 usSubpointsPerPoint;
	UINT16 usSubpointsPerLevel;
	INT8 bCurrentRating;
	BOOLEAN fAffectedByWisdom = TRUE;

	if (p.bEvolution == NO_EVOLUTION)
		return; // No change possible, quit right away

	// if this is a Reverse-Evolving merc who attempting to train
	if (ubReason == FROM_TRAINING && p.bEvolution == DEVOLVE)
		return; // he doesn't get any benefit, but isn't penalized either

	if (usNumChances == 0)
		return;


	usSubpointsPerPoint = SubpointsPerPoint(ubStat,   p.bExpLevel);
	usSubpointsPerLevel = SubpointsPerPoint(EXPERAMT, p.bExpLevel);

	INT16* psStatGainPtr;
	switch (ubStat)
	{
		case HEALTHAMT:
			bCurrentRating = p.bLifeMax;
			psStatGainPtr  = &p.sLifeGain;
			// NB physical stat checks not affected by wisdom, unless training is going on
			fAffectedByWisdom = FALSE;
			break;

		case AGILAMT:
			bCurrentRating = p.bAgility;
			psStatGainPtr  = &p.sAgilityGain;
			fAffectedByWisdom = FALSE;
			break;

		case DEXTAMT:
			bCurrentRating = p.bDexterity;
			psStatGainPtr  = &p.sDexterityGain;
			fAffectedByWisdom = FALSE;
			break;

		case WISDOMAMT:
			bCurrentRating = p.bWisdom;
			psStatGainPtr  = &p.sWisdomGain;
			break;

		case MEDICALAMT:
			bCurrentRating = p.bMedical;
			psStatGainPtr  = &p.sMedicalGain;
			break;

		case EXPLODEAMT:
			bCurrentRating = p.bExplosive;
			psStatGainPtr  = &p.sExplosivesGain;
			break;

		case MECHANAMT:
			bCurrentRating = p.bMechanical;
			psStatGainPtr  = &p.sMechanicGain;
			break;

		case MARKAMT:
			bCurrentRating = p.bMarksmanship;
			psStatGainPtr  = &p.sMarksmanshipGain;
			break;

		case EXPERAMT:
			bCurrentRating = p.bExpLevel;
			psStatGainPtr  = &p.sExpLevelGain;
			break;

		case STRAMT:
			bCurrentRating = p.bStrength;
			psStatGainPtr  = &p.sStrengthGain;
			fAffectedByWisdom = FALSE;
			break;

		case LDRAMT:
			bCurrentRating = p.bLeadership;
			psStatGainPtr  = &p.sLeadershipGain;
			break;

		default:
			SLOGE("ProcessStatChange: Rcvd unknown ubStat %d", ubStat);
			return;
	}


	if (ubReason == FROM_TRAINING)
	{
		// training always affected by wisdom
		fAffectedByWisdom = TRUE;
	}


	// stats/skills of 0 can NEVER be improved!
	if ( bCurrentRating <= 0 )
	{
		return;
	}


	// loop once for each chance to improve
	for (uiCnt = 0; uiCnt < usNumChances; uiCnt++)
	{
		if (p.bEvolution == NORMAL_EVOLUTION) // Evolves!
		{
			// if this is improving from a failure, and a successful roll would give us
			// enough to go up a point
			if ((ubReason == FROM_FAILURE) && ((*psStatGainPtr + 1) >= usSubpointsPerPoint))
			{
				// can't improve any more from this statchange, because Ian don't want
				// failures causin increases!
				break;
			}

			if (ubStat != EXPERAMT)
			{
				// NON-experience level changes, actual usChance depends on bCurrentRating
				// Base usChance is '100 - bCurrentRating'
				usChance = 100 - (bCurrentRating + (*psStatGainPtr / usSubpointsPerPoint));

				// prevent training beyond the training cap
				if ((ubReason == FROM_TRAINING) && (bCurrentRating + (*psStatGainPtr / usSubpointsPerPoint) >= TRAINING_RATING_CAP))
				{
					usChance = 0;
				}
			}
			else
			{
				// Experience level changes, actual usChance depends on level
				// Base usChance is '100 - (10 * current level)'
				usChance = 100 - 10 * (bCurrentRating + (*psStatGainPtr / usSubpointsPerPoint));
			}

			// if there IS a usChance, adjust it for high or low wisdom (50 is avg)
			if (usChance > 0 && fAffectedByWisdom)
			{
				usChance += (usChance * (p.bWisdom + (p.sWisdomGain / SubpointsPerPoint(WISDOMAMT, p.bExpLevel)) - 50)) / 100;
			}

			/*
			// if the stat is Marksmanship, and the guy is a hopeless shot
			if ((ubStat == MARKAMT) && (p.bSpecialTrait == HOPELESS_SHOT))
			{
				usChance /= 5; // MUCH slower to improve, divide usChance by 5
			}*/

			// maximum possible usChance is 99%
			if (usChance > 99)
			{
				usChance = 99;
			}

			if (PreRandom(100) < usChance)
			{
				(*psStatGainPtr)++;
				sSubPointChange++;

				// as long as we're not dealing with exp_level changes (already added above!)
				// and it's not from training, and the exp level isn't max'ed out already
				if ((ubStat != EXPERAMT) && (ubReason != FROM_TRAINING))
				{
					uiEffLevel = p.bExpLevel + (p.sExpLevelGain / usSubpointsPerLevel);

					// if level is not at maximum
					if (uiEffLevel < MAXEXPLEVEL)
					{
						// if this is NOT improving from a failure, OR it would
						// NOT give us enough to go up a level
						if (ubReason != FROM_FAILURE || p.sExpLevelGain + 1 < usSubpointsPerLevel)
						{
							// all other stat changes count towards experience
							// level changes (1 for 1 basis)
							p.sExpLevelGain++;
						}
					}
				}
			}
		}
		else // Regresses!
		{
			// regression can happen from both failures and successes (but not training, checked above)

			if (ubStat != EXPERAMT)
			{
				// NON-experience level changes, actual usChance depends on bCurrentRating
				switch (ubStat)
				{
					case HEALTHAMT:
					case AGILAMT:
					case DEXTAMT:
					case WISDOMAMT:
					case STRAMT:
						// Base usChance is 'bCurrentRating - 1', since these must remain at 1-100
						usChance = bCurrentRating + (*psStatGainPtr / usSubpointsPerPoint) - 1;
						break;

					case MEDICALAMT:
					case EXPLODEAMT:
					case MECHANAMT:
					case MARKAMT:
					case LDRAMT:
						// Base usChance is 'bCurrentRating', these can drop to 0
						usChance = bCurrentRating + (*psStatGainPtr / usSubpointsPerPoint);
						break;
					default:
						break;
				}
			}
			else
			{
				// Experience level changes, actual usChance depends on level
				// Base usChance is '10 * (current level - 1)'
				usChance = 10 * (bCurrentRating + (*psStatGainPtr / usSubpointsPerPoint) - 1);

				// if there IS a usChance, adjust it for high or low wisdom (50 is avg)
				if (usChance > 0 && fAffectedByWisdom)
				{
					usChance -= (usChance * (p.bWisdom + (p.sWisdomGain / SubpointsPerPoint(WISDOMAMT, p.bExpLevel)) - 50)) / 100;
				}

				// if there's ANY usChance, minimum usChance is 1% regardless of wisdom
				if (usChance < 1)
				{
					usChance = 1;
				}
			}

			if (PreRandom(100) < usChance)
			{
				(*psStatGainPtr)--;
				sSubPointChange--;

				// as long as we're not dealing with exp_level changes (already added above!)
				// and it's not from training, and the exp level isn't max'ed out already
				if ((ubStat != EXPERAMT) && (ubReason != FROM_TRAINING))
				{
					uiEffLevel = p.bExpLevel + (p.sExpLevelGain / usSubpointsPerLevel);

					// if level is not at minimum
					if (uiEffLevel > 1)
					{
						// all other stat changes count towards experience level changes (1 for 1 basis)
						p.sExpLevelGain--;
					}
				}
			}
		}
	}

	// exclude training, that's not under our control
	if (ubReason != FROM_TRAINING)
	{
		// increment counters that track how often stat changes are being awarded
		p.usStatChangeChances[ubStat]   += usNumChances;
		p.usStatChangeSuccesses[ubStat] += ABS(sSubPointChange);
	}
}


// UpdateStats version for mercs not currently on player's team
static void ProfileUpdateStats(MERCPROFILESTRUCT& p)
{
	ProcessUpdateStats(p, NULL);
}


static UINT32 CalcNewSalary(UINT32 uiOldSalary, BOOLEAN fIncrease, UINT32 uiMaxLimit);


static void ChangeStat(MERCPROFILESTRUCT& p, SOLDIERTYPE* const pSoldier, StatKind const ubStat, INT16 const sPtsChanged)
{
	// this function changes the stat a given amount...
	INT16 *psStatGainPtr = NULL;
	INT8 *pbStatPtr = NULL;
	INT8 *pbSoldierStatPtr = NULL;
	INT8 *pbStatDeltaPtr = NULL;
	UINT32 *puiStatTimerPtr = NULL;
	BOOLEAN fChangeTypeIncrease;
	BOOLEAN fChangeSalary;
	UINT32 uiLevelCnt;
	UINT8 ubMercMercIdValue = 0;
	UINT16 usIncreaseValue = 0;
	UINT16 usSubpointsPerPoint;

	usSubpointsPerPoint = SubpointsPerPoint(ubStat, p.bExpLevel);

	// build ptrs to appropriate profiletype stat fields
	switch( ubStat )
	{
		case HEALTHAMT:
			psStatGainPtr  = &p.sLifeGain;
			pbStatDeltaPtr = &p.bLifeDelta;
			pbStatPtr      = &p.bLifeMax;
			break;

		case AGILAMT:
			psStatGainPtr  = &p.sAgilityGain;
			pbStatDeltaPtr = &p.bAgilityDelta;
			pbStatPtr      = &p.bAgility;
			break;

		case DEXTAMT:
			psStatGainPtr  = &p.sDexterityGain;
			pbStatDeltaPtr = &p.bDexterityDelta;
			pbStatPtr      = &p.bDexterity;
			break;

		case WISDOMAMT:
			psStatGainPtr  = &p.sWisdomGain;
			pbStatDeltaPtr = &p.bWisdomDelta;
			pbStatPtr      = &p.bWisdom;
			break;

		case MEDICALAMT:
			psStatGainPtr  = &p.sMedicalGain;
			pbStatDeltaPtr = &p.bMedicalDelta;
			pbStatPtr      = &p.bMedical;
			break;

		case EXPLODEAMT:
			psStatGainPtr  = &p.sExplosivesGain;
			pbStatDeltaPtr = &p.bExplosivesDelta;
			pbStatPtr      = &p.bExplosive;
			break;

		case MECHANAMT:
			psStatGainPtr  = &p.sMechanicGain;
			pbStatDeltaPtr = &p.bMechanicDelta;
			pbStatPtr      = &p.bMechanical;
			break;

		case MARKAMT:
			psStatGainPtr  = &p.sMarksmanshipGain;
			pbStatDeltaPtr = &p.bMarksmanshipDelta;
			pbStatPtr      = &p.bMarksmanship;
			break;

		case EXPERAMT:
			psStatGainPtr  = &p.sExpLevelGain;
			pbStatDeltaPtr = &p.bExpLevelDelta;
			pbStatPtr      = &p.bExpLevel;
			break;

		case STRAMT:
			psStatGainPtr  = &p.sStrengthGain;
			pbStatDeltaPtr = &p.bStrengthDelta;
			pbStatPtr      = &p.bStrength;
			break;

		case LDRAMT:
			psStatGainPtr  = &p.sLeadershipGain;
			pbStatDeltaPtr = &p.bLeadershipDelta;
			pbStatPtr      = &p.bLeadership;
			break;

		default:
			break;
	}


	// if this merc is currently on the player's team
	if (pSoldier != NULL)
	{
		// build ptrs to appropriate soldiertype stat fields
		switch( ubStat )
		{
		case HEALTHAMT:
			pbSoldierStatPtr = &( pSoldier->bLifeMax );
			puiStatTimerPtr = &( pSoldier->uiChangeHealthTime);
			usIncreaseValue = HEALTH_INCREASE;
			break;

		case AGILAMT:
			pbSoldierStatPtr = &( pSoldier->bAgility );
			puiStatTimerPtr = &( pSoldier->uiChangeAgilityTime);
			usIncreaseValue = AGIL_INCREASE;
			break;

		case DEXTAMT:
			pbSoldierStatPtr = &( pSoldier->bDexterity );
			puiStatTimerPtr = &( pSoldier->uiChangeDexterityTime);
			usIncreaseValue = DEX_INCREASE;
			break;

		case WISDOMAMT:
			pbSoldierStatPtr = &( pSoldier->bWisdom );
			puiStatTimerPtr = &( pSoldier->uiChangeWisdomTime);
			usIncreaseValue = WIS_INCREASE;
			break;

		case MEDICALAMT:
			pbSoldierStatPtr = &( pSoldier->bMedical );
			puiStatTimerPtr = &( pSoldier->uiChangeMedicalTime);
			usIncreaseValue = MED_INCREASE;
			break;

		case EXPLODEAMT:
			pbSoldierStatPtr = &( pSoldier->bExplosive );
			puiStatTimerPtr = &( pSoldier->uiChangeExplosivesTime);
			usIncreaseValue = EXP_INCREASE;
			break;

		case MECHANAMT:
			pbSoldierStatPtr = &( pSoldier->bMechanical );
			puiStatTimerPtr = &( pSoldier->uiChangeMechanicalTime);
			usIncreaseValue = MECH_INCREASE;
			break;

		case MARKAMT:
			pbSoldierStatPtr = &( pSoldier->bMarksmanship );
			puiStatTimerPtr = &( pSoldier->uiChangeMarksmanshipTime);
			usIncreaseValue = MRK_INCREASE;
			break;

		case EXPERAMT:
			pbSoldierStatPtr = &(pSoldier->bExpLevel);
			puiStatTimerPtr = &( pSoldier->uiChangeLevelTime );
			usIncreaseValue = LVL_INCREASE;
			break;

		case STRAMT:
			pbSoldierStatPtr = &(pSoldier->bStrength);
			puiStatTimerPtr = &( pSoldier->uiChangeStrengthTime);
			usIncreaseValue = STRENGTH_INCREASE;
			break;

		case LDRAMT:
			pbSoldierStatPtr = &( pSoldier->bLeadership);
			puiStatTimerPtr = &( pSoldier->uiChangeLeadershipTime);
			usIncreaseValue = LDR_INCREASE;
			break;

		default:
			break;
		}
	}

	// ptrs set up, now handle
	// if the stat needs to change
	if ( sPtsChanged != 0 )
	{
		// if a stat improved
		if ( sPtsChanged > 0 )
		{
			fChangeTypeIncrease = TRUE;
		}
		else
		{
			fChangeTypeIncrease = FALSE;
		}

		// update merc profile stat
		*pbStatPtr += sPtsChanged;

		// if this merc is currently on the player's team (DON'T count increases earned outside the player's employ)
		if (pSoldier != NULL)
		{
			// also note the delta (how much this stat has changed since start of game)
			*pbStatDeltaPtr += sPtsChanged;
		}

		// reduce gain to the unused subpts only
		*psStatGainPtr = ( *psStatGainPtr ) % usSubpointsPerPoint;


		// if the guy is employed by player
		if (pSoldier != NULL)
		{
			// transfer over change to soldiertype structure
			*pbSoldierStatPtr = *pbStatPtr;

			// if it's a level gain, or sometimes for other stats
			// ( except health; not only will it sound silly, but
			// also we give points for health on sector traversal and this would
			// probaby mess up battle handling too )
			if ( (ubStat != HEALTHAMT) && ( (ubStat == EXPERAMT) || Random( 100 ) < 25 ) )
			//if ( (ubStat != EXPERAMT) && (ubStat != HEALTHAMT) && ( Random( 100 ) < 25 ) )
			{
				// Pipe up with "I'm getting better at this!"
				class CharacterDialogueEventDisplayStatChange : public CharacterDialogueEvent
				{
					public:
						CharacterDialogueEventDisplayStatChange(SOLDIERTYPE& soldier, BOOLEAN const change_type_increase, UINT16 const pts_changed, StatKind const stat) :
							CharacterDialogueEvent(soldier),
							change_type_increase_(change_type_increase),
							pts_changed_(pts_changed),
							stat_(stat)
						{}

						bool Execute()
						{
							if (!MayExecute()) return true;

							// Tell player about stat increase
							ST::string buf = BuildStatChangeString(soldier_.name, change_type_increase_, pts_changed_, stat_);
							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, buf);
							return false;
						}

					private:
						BOOLEAN  const change_type_increase_;
						UINT16   const pts_changed_;
						StatKind const stat_;
				};

				DialogueEvent::Add(new CharacterDialogueEventDisplayStatChange(*pSoldier, fChangeTypeIncrease, sPtsChanged, ubStat));
				TacticalCharacterDialogue( pSoldier, QUOTE_EXPERIENCE_GAIN );
			}
			else
			{
				// tell player about it
				ST::string wTempString = BuildStatChangeString(pSoldier->name, fChangeTypeIncrease, sPtsChanged, ubStat);
				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, wTempString );
			}

			// update mapscreen soldier info panel
			fCharacterInfoPanelDirty = TRUE;

			// remember what time it changed at, it's displayed in a different color for a while afterwards
			*puiStatTimerPtr = GetJA2Clock();

			if( fChangeTypeIncrease )
			{
				pSoldier->usValueGoneUp |= usIncreaseValue;
			}
			else
			{
				pSoldier->usValueGoneUp &= ~( usIncreaseValue );
			}

			fInterfacePanelDirty = DIRTYLEVEL2;
		}


		// special handling for LIFEMAX
		if( ubStat == HEALTHAMT )
		{
			// adjust current health by the same amount as max health
			p.bLife += sPtsChanged;

			// don't let this kill a guy or knock him out!!!
			if (p.bLife < OKLIFE)
			{
				p.bLife = OKLIFE;
			}

			// if the guy is employed by player
			if (pSoldier != NULL)
			{
				// adjust current health by the same amount as max health
				pSoldier->bLife += sPtsChanged;

				// don't let this kill a guy or knock him out!!!
				if (pSoldier->bLife < OKLIFE)
				{
					pSoldier->bLife = OKLIFE;
				}
			}
		}

		// special handling for EXPERIENCE LEVEL
		// merc salaries increase if level goes up (but they don't go down if level drops!)
		if( (ubStat == EXPERAMT) && fChangeTypeIncrease)
		{
			// if the guy is employed by player
			if (pSoldier != NULL)
			{
				switch (pSoldier->ubWhatKindOfMercAmI)
				{
					case MERC_TYPE__AIM_MERC:
						// A.I.M.
						pSoldier->fContractPriceHasIncreased = TRUE;
						fChangeSalary = TRUE;
						break;

					case MERC_TYPE__MERC:
						// M.E.R.C.
						ubMercMercIdValue = pSoldier->ubProfile;

						// Biff's profile id ( 40 ) is the base
						ubMercMercIdValue -= BIFF;

						// offset for the 2 profiles of Larry (we only have one email for Larry..but 2 profile entries
						if( ubMercMercIdValue >= ( LARRY_DRUNK - BIFF ) )
						{
							ubMercMercIdValue--;
						}

						//
						// Send special E-mail
						//

						// DEF: 03/06/99 Now sets an event that will be processed later in the day
						//ubEmailOffset = MERC_UP_LEVEL_BIFF + MERC_UP_LEVEL_LENGTH_BIFF * ( ubMercMercIdValue );
						//AddEmail( ubEmailOffset, MERC_UP_LEVEL_LENGTH_BIFF, SPECK_FROM_MERC, GetWorldTotalMin() );
						AddStrategicEvent( EVENT_MERC_MERC_WENT_UP_LEVEL_EMAIL_DELAY, GetWorldTotalMin( ) + 60 + Random( 60 ), ubMercMercIdValue );

						fChangeSalary = TRUE;
						break;

					default:
						// others don't increase salary
						fChangeSalary = FALSE;
						break;
				}
			}
			else // not employed by player
			{
				// only AIM and M.E.R.C.s update stats when not on player's team, and both of them DO change salary
				fChangeSalary = TRUE;
			}

			if (fChangeSalary)
			{
				// increase all salaries and medical deposits, once for each level gained
				for (uiLevelCnt = 0; uiLevelCnt < (UINT32) sPtsChanged; uiLevelCnt++)
				{
					p.sSalary = (INT16) CalcNewSalary(p.sSalary, fChangeTypeIncrease,
										MAX_DAILY_SALARY);
					p.uiWeeklySalary = CalcNewSalary(p.uiWeeklySalary, fChangeTypeIncrease,
										MAX_LARGE_SALARY);
					p.uiBiWeeklySalary = CalcNewSalary(p.uiBiWeeklySalary, fChangeTypeIncrease,
										MAX_LARGE_SALARY);
					p.sMedicalDepositAmount = (INT16) CalcNewSalary(p.sMedicalDepositAmount,
											fChangeTypeIncrease,
											MAX_DAILY_SALARY);

					//if (pSoldier != NULL)
					//	// DON'T increase the *effective* medical deposit, it's already been
					//	//paid out
					//	pSoldier->usMedicalDeposit = p.sMedicalDepositAmount;
				}
			}
		}
	}
}


// pSoldier may be NULL!
static void ProcessUpdateStats(MERCPROFILESTRUCT& p, SOLDIERTYPE* const pSoldier)
{
	// this function will run through the soldier's profile and update their stats based on any accumulated gain pts.
	INT16 *psStatGainPtr = NULL;
	INT8 *pbStatPtr = NULL;
	INT8 bMinStatValue;
	INT8 bMaxStatValue;
	UINT16 usSubpointsPerPoint;
	INT16 sPtsChanged;


	// if hired, not back at AIM
	if ( pSoldier != NULL )
	{
		// ATE: if in the midst of an attack, if in the field, delay all stat changes until the check made after the 'attack'...
		if ((gTacticalStatus.ubAttackBusyCount > 0) && pSoldier->bInSector &&
			(gTacticalStatus.uiFlags & INCOMBAT))
		{
			return;
		}

		// ignore non-player soldiers
		if (!IsOnOurTeam(*pSoldier))
			return;

		// ignore anything without a profile
		if (pSoldier->ubProfile == NO_PROFILE)
			return;

		// ignore vehicles and robots
		if((pSoldier->uiStatusFlags & SOLDIER_VEHICLE) || (pSoldier->uiStatusFlags & SOLDIER_ROBOT))
			return;

		// delay increases while merc is dying
		if (pSoldier->bLife < OKLIFE)
			return;

		// ignore POWs - shouldn't ever be getting this far
		if( pSoldier->bAssignment == ASSIGNMENT_POW )
		{
			return;
		}
	}
	else
	{
		// dead guys don't do nuthin' !
		if (p.bMercStatus == MERC_IS_DEAD)
			return;

		if (p.bLife < OKLIFE)
			return;
	}


	// check every attribute, skill, and exp.level, too
	for (StatKind ubStat = FIRST_CHANGEABLE_STAT; ubStat <= LAST_CHANGEABLE_STAT; ++ubStat)
	{
		// set default min & max, subpoints/pt.
		bMinStatValue = 1;
		bMaxStatValue = MAX_STAT_VALUE;
		usSubpointsPerPoint = SubpointsPerPoint(ubStat, p.bExpLevel);

		// build ptrs to appropriate profiletype stat fields
		switch( ubStat )
		{
			case HEALTHAMT:
				psStatGainPtr = &p.sLifeGain;
				pbStatPtr     = &p.bLifeMax;

				bMinStatValue = OKLIFE;
				break;

			case AGILAMT:
				psStatGainPtr = &p.sAgilityGain;
				pbStatPtr     = &p.bAgility;
				break;

			case DEXTAMT:
				psStatGainPtr = &p.sDexterityGain;
				pbStatPtr     = &p.bDexterity;
				break;

			case WISDOMAMT:
				psStatGainPtr = &p.sWisdomGain;
				pbStatPtr     = &p.bWisdom;
				break;

			case MEDICALAMT:
				psStatGainPtr = &p.sMedicalGain;
				pbStatPtr     = &p.bMedical;

				bMinStatValue = 0;
				break;

			case EXPLODEAMT:
				psStatGainPtr = &p.sExplosivesGain;
				pbStatPtr     = &p.bExplosive;

				bMinStatValue = 0;
				break;

			case MECHANAMT:
				psStatGainPtr = &p.sMechanicGain;
				pbStatPtr     = &p.bMechanical;

				bMinStatValue = 0;
				break;

			case MARKAMT:
				psStatGainPtr = &p.sMarksmanshipGain;
				pbStatPtr     = &p.bMarksmanship;

				bMinStatValue = 0;
				break;

			case EXPERAMT:
				psStatGainPtr = &p.sExpLevelGain;
				pbStatPtr     = &p.bExpLevel;

				bMaxStatValue = MAXEXPLEVEL;
				break;

			case STRAMT:
				psStatGainPtr = &p.sStrengthGain;
				pbStatPtr     = &p.bStrength;
				break;

			case LDRAMT:
				psStatGainPtr = &p.sLeadershipGain;
				pbStatPtr     = &p.bLeadership;
				break;

			default:
				break;
		}

		// ptrs set up, now handle

		// Calc how many full points worth of stat changes we have accumulated in
		// this stat (positive OR negative!)
		// NOTE: for simplicity, this hopes nobody will go up more than one
		//       level at once, which would change the subpoints/pt
		sPtsChanged = ( *psStatGainPtr ) / usSubpointsPerPoint;

		// gone too high or too low?..handle the fact
		if( (*pbStatPtr + sPtsChanged) > bMaxStatValue )
		{
			// reduce change to reach max value and reset stat gain ptr
			sPtsChanged = bMaxStatValue - *pbStatPtr;
			*psStatGainPtr = 0;
		}
		else
		if( (*pbStatPtr + sPtsChanged) < bMinStatValue )
		{
			// reduce change to reach min value and reset stat gain ptr
			sPtsChanged = bMinStatValue - *pbStatPtr;
			*psStatGainPtr = 0;
		}


		// if the stat needs to change
		if ( sPtsChanged != 0 )
		{
			// Otherwise, use normal stat increase stuff...
			ChangeStat(p, pSoldier, ubStat, sPtsChanged);
		}
	}
}


void HandleAnyStatChangesAfterAttack( void )
{
	// must check everyone on player's team, not just the shooter
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		ProcessUpdateStats(GetProfile(s->ubProfile), s);
	}
}


static UINT32 RoundOffSalary(UINT32 uiSalary);


static UINT32 CalcNewSalary(UINT32 uiOldSalary, BOOLEAN fIncrease, UINT32 uiMaxLimit)
{
	UINT32 uiNewSalary;

	// if he was working for free, it's still free!
	if (uiOldSalary == 0)
	{
		return(0);
	}

	if (fIncrease)
	{
		uiNewSalary = (UINT32) (uiOldSalary * SALARY_CHANGE_PER_LEVEL);
	}
	else
	{
		uiNewSalary = (UINT32) (uiOldSalary / SALARY_CHANGE_PER_LEVEL);
	}

	// round it off to a reasonable multiple
	uiNewSalary = RoundOffSalary(uiNewSalary);

	// let's set some reasonable limits here, lest it get silly one day
	if (uiNewSalary > uiMaxLimit)
		uiNewSalary = uiMaxLimit;

	if (uiNewSalary < 5)
		uiNewSalary = 5;


	return(uiNewSalary);
}


static UINT32 RoundOffSalary(UINT32 uiSalary)
{
	UINT32 uiMultiple;


	// determine what multiple value the salary should be rounded off to
	if      (uiSalary <=   250)
		uiMultiple =    5;
	else if (uiSalary <=   500)
		uiMultiple =   10;
	else if (uiSalary <=  1000)
		uiMultiple =   25;
	else if (uiSalary <=  2000)
		uiMultiple =   50;
	else if (uiSalary <=  5000)
		uiMultiple =  100;
	else if (uiSalary <= 10000)
		uiMultiple =  500;
	else if (uiSalary <= 25000)
		uiMultiple = 1000;
	else if (uiSalary <= 50000)
		uiMultiple = 2000;
	else
		uiMultiple = 5000;


	// if the salary doesn't divide evenly by the multiple
	if (uiSalary % uiMultiple)
	{
		// then we have to make it so, as Picard would say <- We have to wonder how much Alex gets out
		// and while we're at it, we round up to next higher multiple if halfway
		uiSalary = ((uiSalary + (uiMultiple / 2)) / uiMultiple) * uiMultiple;
	}

	return(uiSalary);
}


static UINT16 SubpointsPerPoint(StatKind const ubStat, INT8 const bExpLevel)
{
	UINT16 usSubpointsPerPoint;

	// figure out how many subpoints this type of stat needs to change
	switch (ubStat)
	{
		case HEALTHAMT:
		case AGILAMT:
		case DEXTAMT:
		case WISDOMAMT:
		case STRAMT:
			// attributes
			usSubpointsPerPoint = ATTRIBS_SUBPOINTS_TO_IMPROVE;
			break;

		case MEDICALAMT:
		case EXPLODEAMT:
		case MECHANAMT:
		case MARKAMT:
		case LDRAMT:
			// skills
			usSubpointsPerPoint = SKILLS_SUBPOINTS_TO_IMPROVE;
			break;

		case EXPERAMT:
			usSubpointsPerPoint = LEVEL_SUBPOINTS_TO_IMPROVE * bExpLevel;
			break;

		default:
			SLOGE("SubpointsPerPoint: Unknown ubStat %d", ubStat);
			return(100);
	}

	return(usSubpointsPerPoint);
}


// handles stat changes for mercs not currently working for the player
void HandleUnhiredMercImprovement(MERCPROFILESTRUCT& p)
{
	UINT8 ubNumStats;
	UINT16 usNumChances;

	ubNumStats = LAST_CHANGEABLE_STAT - FIRST_CHANGEABLE_STAT + 1;

	// if he's working on another job
	if (p.bMercStatus == MERC_WORKING_ELSEWHERE)
	{
		// if he did't do anything interesting today
		if (Random(100) < 20)
		{
			// no chance to change today
			return;
		}

		// it's real on the job experience, counts towards experience

		// all stats (including experience itself) get an equal chance to improve
		// 80 wisdom gives 8 rolls per stat per day, 10 stats, avg success rate 40% = 32pts per day,
		// so about 10 working days to hit lvl 2.  This seems high, but mercs don't actually "work" that often,
		// and it's twice as long to hit level 3.  If we go lower, attribs & skills will barely move.
		usNumChances = p.bWisdom / 10;
		for (StatKind ubStat = FIRST_CHANGEABLE_STAT; ubStat <= LAST_CHANGEABLE_STAT; ++ubStat)
		{
			ProfileStatChange(p, ubStat, usNumChances, FROM_SUCCESS);
		}
	}
	else
	{
		// if the merc just takes it easy (high level or stupid mercs are more likely to)
		if ((INT8)Random(10) < p.bExpLevel || (INT8)Random(100) > p.bWisdom)
		{
			// no chance to change today
			return;
		}

		// it's just practise/training back home
		StatKind ubStat;
		do
		{
			// pick ONE stat at random to focus on (it may be beyond training cap, but so what,
			// too hard to weed those out)
			ubStat = static_cast<StatKind>(FIRST_CHANGEABLE_STAT + Random(ubNumStats));
			// except experience - can't practise that!
		} while (ubStat == EXPERAMT);

		// try to improve that one stat
		ProfileStatChange(p, ubStat, (UINT16)(p.bWisdom / 2), FROM_TRAINING);
	}

	ProfileUpdateStats(p);
}


// handles possible death of mercs not currently working for the player
void HandleUnhiredMercDeaths( INT32 iProfileID )
{
	UINT8 ubMaxDeaths;
	INT16 sChance;
	MERCPROFILESTRUCT& p = GetProfile(iProfileID);


	// if the player has never yet had the chance to hire this merc
	if (!(p.ubMiscFlags3 & PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE))
	{
		// then we're not allowed to kill him (to avoid really pissing off player by killing his very favorite merc)
		return;
	}

	// how many in total can be killed like this depends on player's difficulty setting
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			ubMaxDeaths = gamepolicy(unhired_merc_deaths_difficulty_0);
			break;
		case DIF_LEVEL_MEDIUM:
			ubMaxDeaths = gamepolicy(unhired_merc_deaths_difficulty_1);
			break;
		case DIF_LEVEL_HARD:
			ubMaxDeaths = gamepolicy(unhired_merc_deaths_difficulty_2);
			break;
		default:
			Assert(FALSE);
			ubMaxDeaths = 0;
			break;
	}

	// if we've already hit the limit in this game, skip these checks
	if (gStrategicStatus.ubUnhiredMercDeaths >= ubMaxDeaths)
	{
		return;
	}


	// calculate this merc's (small) chance to get killed today (out of 1000)
	sChance = 10 - p.bExpLevel;

	switch (p.bPersonalityTrait)
	{
		case FORGETFUL:
		case NERVOUS:
		case PSYCHO:
			// these guys are somewhat more likely to get killed (they have "problems")
			sChance += 2;
			break;
	}

	// stealthy guys are slightly less likely to get killed (they're careful)
	if (p.bSkillTrait  == STEALTHY)
		sChance -= 1;
	if (p.bSkillTrait2 == STEALTHY)
		sChance -= 1;


	if ((INT16) PreRandom(1000) < sChance)
	{
		// this merc gets Killed In Action!!!
		p.bMercStatus           = MERC_IS_DEAD;
		p.uiDayBecomesAvailable = 0;

		// keep count of how many there have been
		gStrategicStatus.ubUnhiredMercDeaths++;

		//send an email as long as the merc is from aim
		if( iProfileID < BIFF )
		{
			//send an email to the player telling the player that a merc died
			AddEmailWithSpecialData(MERC_DIED_ON_OTHER_ASSIGNMENT, MERC_DIED_ON_OTHER_ASSIGNMENT_LENGTH, AIM_SITE, GetWorldTotalMin(), 0, iProfileID );
		}
	}
}


static UINT8 CalcImportantSectorControl(void);


// These HAVE to total 100% at all times!!!
#define PROGRESS_PORTION_TOTAL		(gamepolicy(progress_weight_kills) + gamepolicy(progress_weight_control) + gamepolicy(progress_weight_income))
#define PROGRESS_PORTION_KILLS		(100 * gamepolicy(progress_weight_kills) / PROGRESS_PORTION_TOTAL)
#define PROGRESS_PORTION_CONTROL	(100 * gamepolicy(progress_weight_control) / PROGRESS_PORTION_TOTAL)
#define PROGRESS_PORTION_INCOME		(100 * gamepolicy(progress_weight_income) / PROGRESS_PORTION_TOTAL)


// returns a number between 0-100, this is an estimate of how far a player has progressed through the game
UINT8 CurrentPlayerProgressPercentage(void)
{
	UINT32 uiCurrentIncome;
	UINT32 uiPossibleIncome;
	UINT8 ubCurrentProgress;
	UINT8 ubKillsPerPoint;
	UINT16 usKillsProgress;
	UINT16 usControlProgress;


	if( gfEditMode )
		return 0;

	// figure out the player's current mine income
	uiCurrentIncome = PredictIncomeFromPlayerMines();

	// figure out the player's potential mine income
	uiPossibleIncome = CalcMaxPlayerIncomeFromMines();

	// either of these indicates a critical failure of some sort
	Assert(uiPossibleIncome > 0);
	Assert(uiCurrentIncome <= uiPossibleIncome);

	// for a rough guess as to how well the player is doing,
	// we'll take the current mine income / potential mine income as a percentage

	//Kris:  Make sure you don't divide by zero!!!
	if( uiPossibleIncome > 0)
	{
		ubCurrentProgress = (UINT8) ((uiCurrentIncome * PROGRESS_PORTION_INCOME) / uiPossibleIncome);
	}
	else
	{
		ubCurrentProgress = 0;
	}

	// kills per point depends on difficulty, and should match the ratios of starting enemy populations (730/1050/1500)
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			ubKillsPerPoint = gamepolicy(kills_per_point_0) != 0 ? gamepolicy(kills_per_point_0) : 7;
			break;
		case DIF_LEVEL_MEDIUM:
			ubKillsPerPoint = gamepolicy(kills_per_point_1) != 0 ? gamepolicy(kills_per_point_1) : 10;
			break;
		case DIF_LEVEL_HARD:
			ubKillsPerPoint = gamepolicy(kills_per_point_2) != 0 ? gamepolicy(kills_per_point_2) : 15;
			break;
		default:
			Assert(FALSE);
			ubKillsPerPoint = 10;
			break;
	}

	if(ubKillsPerPoint > 0)
	{
		usKillsProgress = gStrategicStatus.usPlayerKills / ubKillsPerPoint;
		if (usKillsProgress > PROGRESS_PORTION_KILLS)
		{
			usKillsProgress = PROGRESS_PORTION_KILLS;
		}
	}
	else
	{
		usKillsProgress = PROGRESS_PORTION_KILLS;
	}

	// add kills progress to income progress
	ubCurrentProgress += usKillsProgress;


	// 19 sectors in mining towns + 3 wilderness SAMs each count double.  Balime & Meduna are extra and not required
	usControlProgress = CalcImportantSectorControl();
	if (usControlProgress > PROGRESS_PORTION_CONTROL)
	{
		usControlProgress = PROGRESS_PORTION_CONTROL;
	}

	// add control progress
	ubCurrentProgress += usControlProgress;

	return(ubCurrentProgress);
}


UINT8 HighestPlayerProgressPercentage(void)
{
	if( gfEditMode )
		return 0;

	return(gStrategicStatus.ubHighestProgress);
}


// monitors the highest level of progress that player has achieved so far (checking hourly),
// as opposed to his immediate situation (which may be worse if he's suffered a setback).
void HourlyProgressUpdate(void)
{
	UINT8 ubCurrentProgress;

	ubCurrentProgress = CurrentPlayerProgressPercentage();

	// if this is new high, remember it as that
	if (ubCurrentProgress > gStrategicStatus.ubHighestProgress)
	{
		// CJC:  note when progress goes above certain values for the first time
		#define first_event_trigger( progress_threshold ) (ubCurrentProgress >= progress_threshold && gStrategicStatus.ubHighestProgress < progress_threshold)

		// at 35% start the Madlab quest
		if ( first_event_trigger(gamepolicy(progress_event_madlab_min)) )
		{
			HandleScientistAWOLMeanwhileScene();
		}

		// at 50% make Mike available to the strategic AI
		if ( first_event_trigger(gamepolicy(progress_event_mike_min)) )
		{
			SetFactTrue( FACT_MIKE_AVAILABLE_TO_ARMY );
		}

		// at 70% add Iggy to the world
		if ( first_event_trigger(gamepolicy(progress_event_iggy_min)) )
		{
			gMercProfiles[ IGGY ].sSectorX = 5;
			gMercProfiles[ IGGY ].sSectorY = MAP_ROW_C;
		}

		gStrategicStatus.ubHighestProgress = ubCurrentProgress;

		// debug message
		SLOGD("New player progress record: %d%%", gStrategicStatus.ubHighestProgress );
	}
}

void AwardExperienceBonusToActiveSquad( UINT8 ubExpBonusType )
{
	UINT16 usXPs = 0;

	Assert ( ubExpBonusType < NUM_EXP_BONUS_TYPES );

	switch ( ubExpBonusType )
	{
		case EXP_BONUS_MINIMUM:
			usXPs =   25;
			break;
		case EXP_BONUS_SMALL:
			usXPs =   50;
			break;
		case EXP_BONUS_AVERAGE:
			usXPs =  100;
			break;
		case EXP_BONUS_LARGE:
			usXPs =  200;
			break;
		case EXP_BONUS_MAXIMUM:
			usXPs =  400;
			break;
	}

	// to do: find guys in sector on the currently active squad, those that are conscious get this amount in XPs
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bInSector &&
			IsMercOnCurrentSquad(s) &&
			s->bLife >= CONSCIOUSNESS &&
			!IsMechanical(*s))
		{
			StatChange(*s, EXPERAMT, usXPs, FROM_SUCCESS);
		}
	}
}


ST::string BuildStatChangeString(const ST::string& name, BOOLEAN fIncrease, INT16 sPtsChanged, StatKind ubStat)
{
	UINT8 ubStringIndex;
	UINT16 absPointsChanged = ABS( (int)sPtsChanged );


	Assert( sPtsChanged != 0 );
	Assert( ubStat >= FIRST_CHANGEABLE_STAT );
	Assert( ubStat <= LAST_CHANGEABLE_STAT );

	// if just a 1 point change
	if ( absPointsChanged == 1 )
	{
		// use singular
		ubStringIndex = 2;
	}
	else
	{
		ubStringIndex = 3;
		// use plural
	}

	if ( ubStat == EXPERAMT )
	{
		// use "level/levels instead of point/points
		ubStringIndex += 2;
	}

	return ST::format("{} {} {} {} {}", name,
			sPreStatBuildString[fIncrease ? 1 : 0], absPointsChanged,
			sPreStatBuildString[ubStringIndex],
			sStatGainStrings[ubStat - FIRST_CHANGEABLE_STAT]);
}


static UINT8 CalcImportantSectorControl(void)
{
	UINT8 ubMapX, ubMapY;
	UINT8 ubSectorControlPts = 0;


	for ( ubMapX = 1; ubMapX < MAP_WORLD_X - 1; ubMapX++ )
	{
		for ( ubMapY = 1; ubMapY < MAP_WORLD_Y - 1; ubMapY++ )
		{
			// if player controlled
			if (!StrategicMap[CALCULATE_STRATEGIC_INDEX(ubMapX, ubMapY)].fEnemyControlled)
			{
				// towns where militia can be trained and SAM sites are important sectors
				if ( MilitiaTrainingAllowedInSector( ubMapX, ubMapY, 0 ) )
				{
					ubSectorControlPts++;

					// SAM sites count double - they have no income, but have significant
					// air control value
					if ( IsThisSectorASAMSector( ubMapX, ubMapY, 0 ) )
					{
						ubSectorControlPts++;
					}
				}
			}
		}
	}

	return( ubSectorControlPts );
}


void MERCMercWentUpALevelSendEmail( UINT8 ubMercMercIdValue )
{
	UINT8 ubEmailOffset = 0;

	ubEmailOffset = MERC_UP_LEVEL_BIFF + MERC_UP_LEVEL_LENGTH_BIFF * ( ubMercMercIdValue );
	AddEmail( ubEmailOffset, MERC_UP_LEVEL_LENGTH_BIFF, SPECK_FROM_MERC, GetWorldTotalMin() );
}
