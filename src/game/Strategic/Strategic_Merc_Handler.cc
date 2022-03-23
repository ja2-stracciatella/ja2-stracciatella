#include "Font_Control.h"
#include "Types.h"
#include "Soldier_Control.h"
#include "Strategic_Merc_Handler.h"
#include "History.h"
#include "Game_Clock.h"
#include "Insurance_Contract.h"
#include "Soldier_Profile.h"
#include "Game_Event_Hook.h"
#include "Assignments.h"
#include "Overhead.h"
#include "Merc_Contract.h"
#include "Merc_Hiring.h"
#include "Dialogue_Control.h"
#include "Random.h"
#include "Morale.h"
#include "Mercs.h"
#include "MapScreen.h"
#include "Weapons.h"
#include "Personnel.h"
#include "Strategic_Movement.h"
#include "LaptopSave.h"
#include "Message.h"
#include "Text.h"
#include "Interface.h"
#include "Strategic.h"
#include "StrategicMap.h"
#include "Strategic_Status.h"
#include "AIM.h"
#include "EMail.h"
#include "Map_Screen_Interface.h"
#include "Campaign.h"
#include "Soldier_Add.h"
#include "ScreenIDs.h"
#include "JAScreens.h"
#include "Soldier_Macros.h"
#include "Finances.h"
#include "Quests.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "MercProfile.h"
#include "WeaponModels.h"
#include "Logger.h"

#include <string_theory/string>


#define NUM_DAYS_TILL_UNPAID_RPC_QUITS 3


void StrategicHandlePlayerTeamMercDeath(SOLDIERTYPE& s)
{
	UINT32 const now = GetWorldTotalMin();

	if (s.ubProfile != NO_PROFILE)
	{ // Add to the history log the fact that the merc died and the circumstances

		// CJC Nov 11, 2002: Use the soldier's sector location unless impossible
		INT16 x = s.sSectorX;
		INT16 y = s.sSectorY;
		if (!x || !y)
		{
			x = gWorldSector.x;
			y = gWorldSector.y;
		}

		SOLDIERTYPE const* const killer = s.attacker;
		UINT8              const code   = killer && killer->bTeam == OUR_TEAM ? HISTORY_MERC_KILLED_CHARACTER : HISTORY_MERC_KILLED;
		AddHistoryToPlayersLog(code, s.ubProfile, now, SGPSector(x, y));
	}

	if (guiCurrentScreen != GAME_SCREEN)
	{
		ScreenMsg(FONT_RED, MSG_INTERFACE, st_format_printf(pMercDeadString, s.name));
	}

	/* Robot and EPCs don't count against death rate - the mercs back home don't
	 * particularly give a damn about locals & machines! */
	if (!AM_AN_EPC(&s) && !AM_A_ROBOT(&s))
	{ /* Keep track of how many mercs have died under player's command (for death
		 * rate, can't wait until removed from team) */
		++gStrategicStatus.ubMercDeaths;
	}

	s.uiStatusFlags |= SOLDIER_DEAD;
	s.bBreathMax     = 0;
	s.bBreath        = 0;
	s.fMercAsleep    = FALSE; // Not asleep, dead

	if (s.ubProfile != NO_PROFILE)
	{
		MERCPROFILESTRUCT& p = GetProfile(s.ubProfile);
		p.bMercStatus = MERC_IS_DEAD;

		if (s.usLifeInsurance)
		{
			if (guiCurrentScreen != AUTORESOLVE_SCREEN)
			{ // Check whether this was obviously a suspicious death
				if (now >= s.uiStartTimeOfInsuranceContract && now - s.uiStartTimeOfInsuranceContract < 60)
				{ // Killed within an hour of being insured
					p.ubSuspiciousDeath = VERY_SUSPICIOUS_DEATH;
				}
				else if (s.attacker->bTeam == OUR_TEAM || !gTacticalStatus.fEnemyInSector)
				{ /* Killed by someone on our team or while there weren't any opponents
					* around, cause insurance company to suspect fraud and investigate this
					* claim */
					p.ubSuspiciousDeath = SUSPICIOUS_DEATH;
				}
			}

			AddLifeInsurancePayout(&s);
		}
	}

	/* Robot and EPCs don't penalize morale - merc don't care about fighting
	 * machines and the lives of locals much */
	if (!AM_AN_EPC(&s) && !AM_A_ROBOT(&s))
	{ // Change morale of others based on this
		HandleMoraleEvent(&s, MORALE_TEAMMATE_DIED, SGPSector(s.sSectorX, s.sSectorY, s.bSectorZ));
	}

	if (s.ubWhatKindOfMercAmI == MERC_TYPE__MERC)
	{ // It's a MERC merc, record the time of his death
		s.iEndofContractTime = now;
		// Set is so Speck can say that a merc is dead
		LaptopSaveInfo.ubSpeckCanSayPlayersLostQuote = 1;
	}

	HandleStrategicDeath(s);
}


/* MercDailyUpdate() gets called every day at midnight.  If something is to
 * happen to a merc that day, add an event for it. */
void MercDailyUpdate()
{
	// if its the first day, leave
	if (GetWorldDay() == 1) return;

	SLOGD("%s - Doing MercDailyUpdate", WORLDTIMESTR.c_str());

	/* if the death rate is very low (this is independent of mercs' personal
	 * deathrate tolerances) */
	if (CalcDeathRate() < 5)
	{
		// everyone gets a morale bonus, which also gets player a reputation bonus.
		static const SGPSector nowhere(-1, -1, -1);
		HandleMoraleEvent(nullptr, MORALE_LOW_DEATHRATE, nowhere);
	}

	/* add an event so the merc will say the departing warning (2 hours prior to
	 * leaving).  Do so for all time slots they will depart from */
	AddSameDayStrategicEvent(EVENT_MERC_ABOUT_TO_LEAVE, MERC_ARRIVE_TIME_SLOT_1 - 2 * 60, 0);
	AddSameDayStrategicEvent(EVENT_MERC_ABOUT_TO_LEAVE, MERC_ARRIVE_TIME_SLOT_2 - 2 * 60, 0);
	AddSameDayStrategicEvent(EVENT_MERC_ABOUT_TO_LEAVE, MERC_ARRIVE_TIME_SLOT_3 - 2 * 60, 0);

	AddSameDayStrategicEvent(EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE, MERC_ARRIVE_TIME_SLOT_1, 0);
	AddSameDayStrategicEvent(EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE, MERC_ARRIVE_TIME_SLOT_2, 0);
	AddSameDayStrategicEvent(EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE, MERC_ARRIVE_TIME_SLOT_3, 0);

	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == ASSIGNMENT_POW)
		{
			s->iEndofContractTime += 1440;
		}
		else if (s->bAssignment != IN_TRANSIT)
		{
			//CJC: Reset dialogue flags for quotes that can be said once/day
			s->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_ANNOYING_MERC;
			// ATE: Reset likes gun flag
			s->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_LIKESGUN;
			// ATE; Reset found something nice flag...
			s->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_FOUND_SOMETHING_NICE;

			// ATE: Decrement tolerance value...
			if (--s->bCorpseQuoteTolerance < 0) s->bCorpseQuoteTolerance = 0;

			MERCPROFILESTRUCT& p = GetProfile(s->ubProfile);
			MercProfile const profile(s->ubProfile);

			// CJC: For some personalities, reset personality quote said flag
			switch (p.bPersonalityTrait)
			{
				case HEAT_INTOLERANT:
				case CLAUSTROPHOBIC:
				case NONSWIMMER:
				case FEAR_OF_INSECTS:
					// repeatable once per day
					s->usQuoteSaidFlags &= ~SOLDIER_QUOTE_SAID_PERSONALITY;
					break;

				default: break;
			}

			//ATE: Try to see if our equipment sucks!
			if (SoldierHasWorseEquipmentThanUsedTo(s))
			{
				// Randomly anytime between 6:00, and 10:00
				AddSameDayStrategicEvent(EVENT_MERC_COMPLAIN_EQUIPMENT, 360 + Random(1080), s->ubProfile);
			}

			// increment days served by this grunt
			++p.usTotalDaysServed;

			// player has hired him, so he'll eligible to get killed off on another job
			p.ubMiscFlags3 |= PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE;

			//if the character is an RPC
			if (profile.isRPC())
			{
				//increment the number of days the mercs has been on the team
				++s->iTotalContractLength;

				// The player owes the salary
				INT16 const sSalary    = p.sSalary;
				INT32 iMoneyOwedToMerc = sSalary;

				//if the player owes the npc money, the balance field will be negative
				if (p.iBalance < 0) iMoneyOwedToMerc += -p.iBalance;

				//if the player owes money
				if (iMoneyOwedToMerc != 0)
				{
					//if the player can afford to pay them
					if (LaptopSaveInfo.iCurrentBalance >= iMoneyOwedToMerc)
					{
						//add the transaction to the player
						AddTransactionToPlayersBook(PAYMENT_TO_NPC, s->ubProfile, GetWorldTotalMin(), -iMoneyOwedToMerc);

						// reset the amount, if the player owed money to the npc
						if (p.iBalance < 0) p.iBalance = 0;
					}
					else
					{
						// Display a screen msg indicating that the npc was NOT paid
						ST::string zMoney = SPrintMoney(sSalary);
						ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, st_format_printf(pMessageStrings[MSG_CANT_AFFORD_TO_PAY_NPC_DAILY_SALARY_MSG], p.zNickname, zMoney));

						/* if the merc hasnt been paid for NUM_DAYS_TILL_UNPAID_RPC_QUITS
						 * days, the merc will quit */
						if (p.iBalance - sSalary <= -(sSalary * NUM_DAYS_TILL_UNPAID_RPC_QUITS))
						{
							// Set it up so the merc quits
							MercsContractIsFinished(s);
						}
						else
						{
							//set how much money the player owes the merc
							p.iBalance -= sSalary;

							// Add even for displaying a dialogue telling the player this....
							AddSameDayStrategicEvent(EVENT_RPC_WHINE_ABOUT_PAY, MERC_ARRIVE_TIME_SLOT_1, s->ubID);
						}
					}
				}
			}

			DailyMoraleUpdate(s);
			CheckIfMercGetsAnotherContract(*s);
		}

		// if active, here, & alive (POW is ok, don't care)
		if (s->bAssignment != ASSIGNMENT_DEAD &&
				s->bAssignment != IN_TRANSIT)
		{
			// increment the "man days" played counter for each such merc in the player's employment
			++gStrategicStatus.uiManDaysPlayed;
		}
	}

	/* Determine for all MERC mercs, whether they should levae, because the
	 * player refused to pay */
	if (LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_INVALID)
	{
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			if (s->bAssignment         == ASSIGNMENT_POW)  continue;
			if (s->bAssignment         == IN_TRANSIT)      continue;
			if (s->ubWhatKindOfMercAmI != MERC_TYPE__MERC) continue;
			if (s->bLife               <  CONSCIOUSNESS)   continue;
			MercsContractIsFinished(s);
		}
	}

	for (INT32 cnt = 0; cnt < NUM_PROFILES; ++cnt)
	{
		MERCPROFILESTRUCT& p = GetProfile(cnt);

		// dead guys don't do nuthin' !
		if (p.bMercStatus == MERC_IS_DEAD) continue;

		//Every day reset this variable
		p.uiPrecedentQuoteSaid = 0;

		// skip anyone currently on the player's team
		if (IsMercOnTeam(cnt)) continue;

		if (IsProfileIdAnAimOrMERCMerc((UINT8) cnt) && p.bMercStatus != MERC_RETURNING_HOME)
		{
			// check if any of his stats improve through working or training
			HandleUnhiredMercImprovement(p);

			// if he's working on another job
			if (p.bMercStatus == MERC_WORKING_ELSEWHERE)
			{
				// check if he's killed
				HandleUnhiredMercDeaths(cnt);
			}
		}

		// if merc is currently unavailable
		if (p.uiDayBecomesAvailable > 0)
		{
			if (--p.uiDayBecomesAvailable == 0 &&    // Check to see if the merc has become available
					p.bMercStatus != MERC_FIRED_AS_A_POW) // if the merc CAN become ready
			{
				p.bMercStatus = MERC_OK;

				// if the player has left a message for this merc
				if (p.ubMiscFlags3 & PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM)
				{
					//remove the Flag, so if the merc goes on another assignment, the player can leave an email.
					p.ubMiscFlags3 &= ~PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM;

					// TO DO: send E-mail to player telling him the merc has returned from an assignment
					AddEmail(AIM_REPLY_BARRY + cnt * AIM_REPLY_LENGTH_BARRY, AIM_REPLY_LENGTH_BARRY, 6 + cnt, GetWorldTotalMin());
				}
			}
		}
		else	// was already available today
		{
			if (IsProfileIdAnAimOrMERCMerc((UINT8) cnt))
			{
				// check to see if he goes on another assignment
				UINT32 uiChance;
				if (cnt < MAX_NUMBER_MERCS)
				{ // A.I.M. merc
					uiChance = 2 * p.bExpLevel;

					// player has now had a chance to hire him, so he'll eligible to get killed off on another job
					p.ubMiscFlags3 |= PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE;
				}
				else
				{ // M.E.R.C. merc - very rarely get other work
					uiChance = 1 * p.bExpLevel;

					// player doesn't have a chance to hire any M.E.R.C's until after Speck's E-mail is sent
					if (GetWorldDay() > DAYS_TIL_M_E_R_C_AVAIL)
					{
						// player has now had a chance to hire him, so he'll eligible to get killed off on another job
						p.ubMiscFlags3 |= PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE;
					}
				}

				if (Random(100) < uiChance)
				{
					p.bMercStatus = MERC_WORKING_ELSEWHERE;
					p.uiDayBecomesAvailable = 1 + Random(6 + (p.bExpLevel / 2)); // 1-(6 to 11) days
				}
			}
		}

		// Decrement morale hangover (merc appears hirable, he just gives lame refusals during this time, though)
		if (p.ubDaysOfMoraleHangover > 0) --p.ubDaysOfMoraleHangover;
	}

	HandleSlayDailyEvent();
	ReBuildCharactersList();
}


// ATE: This function deals with MERC MERC and NPC's leaving because of not getting paid...
// NOT AIM renewals....
void MercsContractIsFinished(SOLDIERTYPE* const pSoldier)
{
	//if the soldier was removed before getting into this function, return
	if( !pSoldier->bActive )
		return;

	if( fShowContractMenu )
	{
		fShowContractMenu = FALSE;
	}

	// go to mapscreen
	MakeDialogueEventEnterMapScreen();

	if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC )
	{
		//if the players account status is invalid
		if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_INVALID )
		{
			//Send the merc home

			InterruptTime();
			PauseGame();

			// Say quote for wishing to leave
			TacticalCharacterDialogue( pSoldier, QUOTE_NOT_GETTING_PAID );

			MakeCharacterDialogueEventContractEndingNoAskEquip(*pSoldier);

			pSoldier->ubLeaveHistoryCode = HISTORY_MERC_QUIT;
		}
	}
	else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC )
	{
		InterruptTime();
		PauseGame();

		TacticalCharacterDialogue( pSoldier, QUOTE_AIM_SEEN_MIKE );

		MakeCharacterDialogueEventContractEndingNoAskEquip(*pSoldier);

		pSoldier->ubLeaveHistoryCode = HISTORY_MERC_QUIT;

	}
}


// ATE: Called for RPCs who should now complain about no pay
void RPCWhineAboutNoPay(SOLDIERTYPE& s)
{
	// If the soldier was removed before getting into this function, return
	if (!s.bActive) return;
	if (s.ubWhatKindOfMercAmI != MERC_TYPE__NPC) return;

	TacticalCharacterDialogue(&s, QUOTE_NOT_GETTING_PAID);
}


// OK loop through and check!
BOOLEAN SoldierHasWorseEquipmentThanUsedTo( SOLDIERTYPE *pSoldier )
{
	UINT16 usItem;
	INT8   bBestArmour = -1;
	INT8   bBestGun = -1;

	CFOR_EACH_SOLDIER_INV_SLOT(i, *pSoldier)
	{
		usItem = i->usItem;

		// Look for best gun/armour
		if ( usItem != NOTHING )
		{
			// Check if it's a gun
			if ( GCM->getItem(usItem)->isGun())
			{
				if ( GCM->getWeapon( usItem )->ubDeadliness > bBestGun )
				{
					bBestGun = GCM->getWeapon( usItem )->ubDeadliness;
				}
			}

			// If it's armour
			if ( GCM->getItem(usItem)->isArmour() )
			{
				if ( Armour[ GCM->getItem(usItem)->getClassIndex() ].ubProtection > bBestArmour )
				{
					bBestArmour = Armour[ GCM->getItem(usItem)->getClassIndex() ].ubProtection;
				}
			}
		}
	}

	// Modify these values based on morale - lower opinion of equipment if morale low, increase if high
	// this of course assumes default morale is 50
	if ( bBestGun != -1 )
	{
		bBestGun = (bBestGun  * (50 + pSoldier->bMorale)) / 100;
	}
	if ( bBestArmour != -1 )
	{
		bBestArmour = (bBestArmour * (50 + pSoldier->bMorale)) / 100;
	}

	// OK, check values!
	if ((bBestGun != -1 && bBestGun < ( gMercProfiles[ pSoldier->ubProfile ].bMainGunAttractiveness / 2 )) ||
		(bBestArmour != -1 && bBestArmour < ( gMercProfiles[ pSoldier->ubProfile ].bArmourAttractiveness / 2 )) )
	{
		// Pipe up!
		return( TRUE );
	}

	return( FALSE );
}


void MercComplainAboutEquipment( UINT8 ubProfile )
{
	if ( ubProfile == LARRY_NORMAL  )
	{
		if ( CheckFact( FACT_LARRY_CHANGED, 0 ) )
		{
			ubProfile = LARRY_DRUNK;
		}
	}
	else if ( ubProfile == LARRY_DRUNK )
	{
		if (!CheckFact(FACT_LARRY_CHANGED, 0))
		{
			ubProfile = LARRY_NORMAL;
		}
	}
	// Are we dead/ does merc still exist?
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(ubProfile);
	if ( pSoldier != NULL )
	{
		if (!pSoldier->fMercAsleep          &&
			pSoldier->bLife       >= OKLIFE &&
			pSoldier->bAssignment <  ON_DUTY)
		{
			//ATE: Double check that this problem still exists!
			if ( SoldierHasWorseEquipmentThanUsedTo( pSoldier ) )
			{
				// Say quote!
				TacticalCharacterDialogue( pSoldier, QUOTE_WHINE_EQUIPMENT );
			}
		}
	}
}


void UpdateBuddyAndHatedCounters(void)
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// If the merc is active and on a combat assignment
		if (s->bAssignment >= ON_DUTY) continue;

		MERCPROFILESTRUCT& p = GetProfile(s->ubProfile);

		// If we're moving, we only check vs other people in our squad
		bool const same_group_only =
			s->ubGroupID != 0 && PlayerIDGroupInMotion(s->ubGroupID);

		bool fUpdatedTimeTillNextHatedComplaint = false;

		CFOR_EACH_IN_TEAM(other, OUR_TEAM)
		{
			// Is this guy in the same sector and on active duty (or in the same moving group)
			if (other != s && other->bAssignment < ON_DUTY)
			{
				if (same_group_only)
				{ // All we have to check is the group ID
					if (s->ubGroupID != other->ubGroupID) continue;
				}
				else
				{ // Check to see if the location is the same
					if (other->sSectorX != s->sSectorX) continue;
					if (other->sSectorY != s->sSectorY) continue;
					if (other->bSectorZ != s->bSectorZ) continue;

					// if the OTHER soldier is in motion then we don't do anything!
					if (other->ubGroupID != 0 && PlayerIDGroupInMotion(other->ubGroupID))
					{
						continue;
					}
				}

				ProfileID const ubOtherProfileID = other->ubProfile;

				for (INT32 i = 0; i < 4; ++i)
				{
					switch (i)
					{
						case 0:
						case 1:
							if (p.bHated[i] == ubOtherProfileID)
							{
								// arrgs, we're on assignment with the person we loathe!
								INT8& hated_count = p.bHatedCount[i];
								if (hated_count > 0)
								{
									hated_count--;
									if (hated_count == 0 && s->bInSector && gTacticalStatus.fEnemyInSector)
									{ // Just reduced count to 0 but we have enemy in sector
										hated_count = 1;
									}
									else if (hated_count > 0 && (
										hated_count == p.bHatedTime[i] / 2 || (
										hated_count < p.bHatedTime[i] / 2 &&
										hated_count % TIME_BETWEEN_HATED_COMPLAINTS == 0)))
									{ // Complain!
										UINT16 const quote = i == 0 ?
											QUOTE_HATED_MERC_ONE : QUOTE_HATED_MERC_TWO;
										TacticalCharacterDialogue(s, quote);
										StopTimeCompression();
									}
									else if (hated_count == 0)
									{ // Zero count!
										if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC || s->ubWhatKindOfMercAmI == MERC_TYPE__NPC)
										{ // MERC mercs leave now!
											UINT16 const quote = i == 0 ?
												QUOTE_MERC_QUIT_HATED1 : QUOTE_MERC_QUIT_HATED2;
											TacticalCharacterDialogue(s, quote);

											// Leave now! (handle equipment too)
											MakeCharacterDialogueEventContractEnding(*s, false);

											s->ubLeaveHistoryCode = HISTORY_MERC_QUIT;
										}
										else
										{ // Complain!
											UINT16 const quote = i == 0 ?
												QUOTE_HATED_MERC_ONE : QUOTE_HATED_MERC_TWO;
											TacticalCharacterDialogue(s, quote);
											p.ubTimeTillNextHatedComplaint = TIME_BETWEEN_HATED_COMPLAINTS - 1;
										}
									}
								}
								else
								{
									// if we haven't updated the time till our next complaint, do so
									// if it's 0, gripe.
									if (!fUpdatedTimeTillNextHatedComplaint)
									{
										if (p.ubTimeTillNextHatedComplaint == 0)
										{
											p.ubTimeTillNextHatedComplaint = TIME_BETWEEN_HATED_COMPLAINTS - 1;
										}
										else
										{
											p.ubTimeTillNextHatedComplaint--;
										}
										fUpdatedTimeTillNextHatedComplaint = true;
									}

									if (p.ubTimeTillNextHatedComplaint == 0)
									{ // Complain!
										UINT16 const quote = i == 0 ?
											QUOTE_HATED_MERC_ONE : QUOTE_HATED_MERC_TWO;
										TacticalCharacterDialogue(s, quote);
									}
								}
							}
							break;

						case 2:
							if (p.bLearnToHate == ubOtherProfileID)
							{
								if (p.bLearnToHateCount > 0)
								{
									p.bLearnToHateCount--;
									if (p.bLearnToHateCount == 0 && s->bInSector && gTacticalStatus.fEnemyInSector)
									{ // Just reduced count to 0 but we have enemy in sector...
										p.bLearnToHateCount = 1;
									}
									else if (p.bLearnToHateCount > 0 && (
											p.bLearnToHateCount == p.bLearnToHateTime / 2 ||
											(
												p.bLearnToHateCount < p.bLearnToHateTime / 2 &&
												p.bLearnToHateCount % TIME_BETWEEN_HATED_COMPLAINTS == 0
											)))
									{ // Complain!
										TacticalCharacterDialogue(s, QUOTE_LEARNED_TO_HATE_MERC);
										StopTimeCompression();
									}
									else if (p.bLearnToHateCount == 0)
									{ // Set as bHated[2];
										p.bHated[2] = p.bLearnToHate;
										p.bMercOpinion[ubOtherProfileID] = HATED_OPINION;

										if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC || (
											s->ubWhatKindOfMercAmI == MERC_TYPE__NPC && (
											s->ubProfile == DEVIN ||
											s->ubProfile == SLAY  ||
											s->ubProfile == IGGY  ||
											s->ubProfile == CONRAD)))
										{ // Leave now! (handle equipment too)
											TacticalCharacterDialogue(s, QUOTE_MERC_QUIT_LEARN_TO_HATE);
											MakeCharacterDialogueEventContractEnding(*s, false);
											s->ubLeaveHistoryCode = HISTORY_MERC_QUIT;

										}
										else if (s->ubWhatKindOfMercAmI == MERC_TYPE__NPC)
										{ // Whine again
											TacticalCharacterDialogue(s, QUOTE_LEARNED_TO_HATE_MERC);
										}
									}
									if (p.bLearnToHateCount < p.bLearnToHateTime / 2)
									{ // Gradual opinion drop
										p.bMercOpinion[ubOtherProfileID] += (HATED_OPINION - p.bMercOpinion[ubOtherProfileID]) / (p.bLearnToHateCount + 1);
									}
								}
								else
								{
									if (!fUpdatedTimeTillNextHatedComplaint)
									{
										if (p.ubTimeTillNextHatedComplaint == 0)
										{
											p.ubTimeTillNextHatedComplaint = TIME_BETWEEN_HATED_COMPLAINTS - 1;
										}
										else
										{
											p.ubTimeTillNextHatedComplaint--;
										}
										fUpdatedTimeTillNextHatedComplaint = true;
									}

									if (p.ubTimeTillNextHatedComplaint == 0)
									{ // Complain!
										TacticalCharacterDialogue(s, QUOTE_LEARNED_TO_HATE_MERC);
									}
								}
							}
							break;

						case 3:
							if (p.bLearnToLikeCount > 0	&& p.bLearnToLike == ubOtherProfileID)
							{
								p.bLearnToLikeCount--;
								if (p.bLearnToLikeCount == 0)
								{ // Add to liked!
									p.bBuddy[2] = p.bLearnToLike;
									p.bMercOpinion[ubOtherProfileID] = BUDDY_OPINION;
								}
								else if (p.bLearnToLikeCount < p.bLearnToLikeTime / 2)
								{ // Increase opinion of them!
									p.bMercOpinion[ubOtherProfileID] += (BUDDY_OPINION - p.bMercOpinion[ubOtherProfileID]) / (p.bLearnToLikeCount + 1);
									break;
								}
							}
							break;
					}
				}
			}
		}
	}
}


void HourlyCamouflageUpdate()
{
	FOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE& s = *i;

		// If the merc has non-zero camo, degrade it by 1%
		if (s.bCamo > 0 && !HAS_SKILL_TRAIT(&s, CAMOUFLAGED))
		{
			if (s.bCamo <= 2)
			{
				s.bCamo = 0;
				if (s.bInSector) CreateSoldierPalettes(s);

				ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(g_langRes->Message[STR_CAMO_WORN_OFF], s.name));
				DirtyMercPanelInterface(&s, DIRTYLEVEL2);
			}
			else
			{
				s.bCamo -= 2;
			}
		}

		// If the merc has non-zero monster smell, degrade it by 1
		if (s.bMonsterSmell > 0) --s.bMonsterSmell;
	}
}
