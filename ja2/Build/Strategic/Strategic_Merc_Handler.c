#ifdef PRECOMPILEDHEADERS
	#include "Strategic All.h"
#else
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
	#include "Gameloop.h"
	#include "JAScreens.h"
#endif


#define		NUM_DAYS_TILL_UNPAID_RPC_QUITS				3


// can you say me too after someone has said thier contract about to end
BOOLEAN fContractOverMeTooFlag = FALSE;
BOOLEAN fContractOverAndIWontRenewMeTooFlag = FALSE;
BOOLEAN fProcessingAMerc = FALSE;
SOLDIERTYPE *pProcessingSoldier = NULL;
extern BOOLEAN gfFirstMercSayingQuoteWillLeaveNoMatterWhat = FALSE;

// build a list of mercs based on departure time
void BuildMercQuitList( SOLDIERTYPE *pMercList );


void StrategicHandlePlayerTeamMercDeath( SOLDIERTYPE *pSoldier )
{
	SOLDIERTYPE *pKiller = NULL;
	INT16 sSectorX, sSectorY;

	//if the soldier HAS a profile
	if( pSoldier->ubProfile != NO_PROFILE )
	{
		//add to the history log the fact that the merc died and the circumstances
		if( pSoldier->ubAttackerID != NOBODY )
		{
			pKiller = MercPtrs[ pSoldier->ubAttackerID ];
		}

		// CJC Nov 11, 2002
		// Use the soldier's sector location unless impossible
		if (pSoldier->sSectorX != 0 && pSoldier->sSectorY != 0)
		{
			sSectorX = pSoldier->sSectorX;
			sSectorY = pSoldier->sSectorY;
		}
		else
		{
			sSectorX = gWorldSectorX;
			sSectorY = gWorldSectorY;
		}

		if( pKiller && pKiller->bTeam == OUR_TEAM )
		{
			AddHistoryToPlayersLog( HISTORY_MERC_KILLED_CHARACTER, pSoldier->ubProfile, GetWorldTotalMin(), sSectorX, sSectorY );
		}
		else
		{
			AddHistoryToPlayersLog( HISTORY_MERC_KILLED, pSoldier->ubProfile, GetWorldTotalMin(), sSectorX, sSectorY );
		}
	}

	if ( guiCurrentScreen != GAME_SCREEN )
	{
		ScreenMsg( FONT_RED, MSG_INTERFACE, pMercDeadString[ 0 ], pSoldier->name );
	}

	// robot and EPCs don't count against death rate - the mercs back home don't particularly give a damn about locals & machines!
	if ( !AM_AN_EPC( pSoldier ) && !AM_A_ROBOT( pSoldier ) )
	{
		// keep track of how many mercs have died under player's command (for death rate, can't wait until removed from team)
		gStrategicStatus.ubMercDeaths++;
	}


	pSoldier->uiStatusFlags |= SOLDIER_DEAD;

	// Set breath to 0!
	pSoldier->bBreathMax = pSoldier->bBreath = 0;

	// not asleep, DEAD!
	pSoldier->fMercAsleep = FALSE;


	//if the merc had life insurance
	if( pSoldier->usLifeInsurance )
	{
		// if he didn't die during auto-resolve
		if( guiCurrentScreen != AUTORESOLVE_SCREEN )
		{
			// check whether this was obviously a suspicious death
			// if killed within an hour of being insured
			if ( pSoldier->uiStartTimeOfInsuranceContract <= GetWorldTotalMin() && GetWorldTotalMin() - pSoldier->uiStartTimeOfInsuranceContract < 60 )
			{
				gMercProfiles[ pSoldier->ubProfile ].ubSuspiciousDeath = VERY_SUSPICIOUS_DEATH;
			}
			// if killed by someone on our team, or while there weren't any opponents around
			else if (Menptr[ pSoldier->ubAttackerID ].bTeam == OUR_TEAM || !gTacticalStatus.fEnemyInSector )
			{
				// cause insurance company to suspect fraud and investigate this claim
				gMercProfiles[ pSoldier->ubProfile ].ubSuspiciousDeath = SUSPICIOUS_DEATH;
			}
		}

		AddLifeInsurancePayout( pSoldier );
	}


	// robot and EPCs don't penalize morale - merc don't care about fighting machines and the lives of locals much
	if ( !AM_AN_EPC( pSoldier ) && !AM_A_ROBOT( pSoldier ) )
	{
		// Change morale of others based on this
		HandleMoraleEvent( pSoldier, MORALE_TEAMMATE_DIED, pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ );
	}

	//if its a MERC merc, record the time of his death
	if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC )
	{
		pSoldier->iEndofContractTime = GetWorldTotalMin();

		//set is so Speck can say that a merc is dead
		LaptopSaveInfo.ubSpeckCanSayPlayersLostQuote = 1;
	}

	//Set the fact that the merc is DEAD!!
	gMercProfiles[ pSoldier->ubProfile ].bMercStatus = MERC_IS_DEAD;

	if( pSoldier->bAssignment != ASSIGNMENT_DEAD )
	{
		SetTimeOfAssignmentChangeForMerc( pSoldier );
	}

	// handle strategic level death
	HandleStrategicDeath( pSoldier );
}


// MercDailyUpdate() gets called every day at midnight.  If something is to happen to a merc that day, add an event for it.
void MercDailyUpdate()
{
	INT32		cnt;
	INT8		bLastTeamID;
	SOLDIERTYPE		*pSoldier;
	//SOLDIERTYPE *pQuitList[ 21 ];
	MERCPROFILESTRUCT *pProfile;
	UINT32 uiChance;
	INT32 iOffset = 0;
	BOOLEAN fFoundSomeOneForMenuShowing = FALSE;

	//if its the first day, leave
	if( GetWorldDay() == 1 )
		return;

	// debug message
	ScreenMsg( MSG_FONT_RED, MSG_DEBUG, L"%s - Doing MercDailyUpdate", WORLDTIMESTR );

	// if the death rate is very low (this is independent of mercs' personal deathrate tolerances)
	if (CalcDeathRate() < 5)
	{
		// everyone gets a morale bonus, which also gets player a reputation bonus.
		HandleMoraleEvent( NULL, MORALE_LOW_DEATHRATE, -1, -1, -1 );
	}


	//add an event so the merc will say the departing warning ( 2 hours prior to leaving
	// Do so for all time slots they will depart from
	AddSameDayStrategicEvent( EVENT_MERC_ABOUT_TO_LEAVE, MERC_ARRIVE_TIME_SLOT_1 - ( 2 * 60 ),	0 );
	AddSameDayStrategicEvent( EVENT_MERC_ABOUT_TO_LEAVE, MERC_ARRIVE_TIME_SLOT_2 - ( 2 * 60 ),	0 );
	AddSameDayStrategicEvent( EVENT_MERC_ABOUT_TO_LEAVE, MERC_ARRIVE_TIME_SLOT_3 - ( 2 * 60 ),	0 );


	AddSameDayStrategicEvent( EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE, MERC_ARRIVE_TIME_SLOT_1,	0 );
	AddSameDayStrategicEvent( EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE, MERC_ARRIVE_TIME_SLOT_2,	0 );
	AddSameDayStrategicEvent( EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE, MERC_ARRIVE_TIME_SLOT_3,	0 );


	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
	bLastTeamID = gTacticalStatus.Team[ gbPlayerNum ].bLastID;



	//loop though all the mercs
  for ( pSoldier = MercPtrs[ cnt ]; cnt <= bLastTeamID; cnt++,pSoldier++)
	{
		//if the merc is active
		if ( ( pSoldier->bActive )&&( pSoldier->bAssignment != ASSIGNMENT_POW ) && ( pSoldier->bAssignment != IN_TRANSIT ) )
		{
			//CJC: Reset dialogue flags for quotes that can be said once/day
			pSoldier->usQuoteSaidFlags &= ( ~SOLDIER_QUOTE_SAID_ANNOYING_MERC );
			// ATE: Reset likes gun flag
			pSoldier->usQuoteSaidFlags &= ( ~SOLDIER_QUOTE_SAID_LIKESGUN );
			// ATE: Reset seen corpse flag
			pSoldier->usQuoteSaidFlags &= ( ~SOLDIER_QUOTE_SAID_ROTTINGCORPSE );
			// ATE; Reset found something nice flag...
			pSoldier->usQuoteSaidFlags &= ( ~SOLDIER_QUOTE_SAID_FOUND_SOMETHING_NICE );

      // ATE: Decrement tolerance value...
      pSoldier->bCorpseQuoteTolerance--;

      if ( pSoldier->bCorpseQuoteTolerance < 0 )
      {
        pSoldier->bCorpseQuoteTolerance = 0;
      }

			// CJC: For some personalities, reset personality quote said flag
			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				switch( gMercProfiles[ pSoldier->ubProfile ].bPersonalityTrait )
				{
					case HEAT_INTOLERANT:
					case CLAUSTROPHOBIC:
					case NONSWIMMER:
					case FEAR_OF_INSECTS:
						// repeatable once per day
						pSoldier->usQuoteSaidFlags &= ( ~SOLDIER_QUOTE_SAID_PERSONALITY );
						break;
					default:
						break;
				}
			}


			//ATE: Try to see if our equipment sucks!
			if ( SoldierHasWorseEquipmentThanUsedTo( pSoldier ) )
			{
				// Randomly anytime between 6:00, and 10:00
				AddSameDayStrategicEvent( EVENT_MERC_COMPLAIN_EQUIPMENT, 360 + Random( 1080 ) , pSoldier->ubProfile );
			}

			// increment days served by this grunt
			gMercProfiles[pSoldier->ubProfile].usTotalDaysServed++;

			// player has hired him, so he'll eligible to get killed off on another job
			gMercProfiles[pSoldier->ubProfile].ubMiscFlags3 |= PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE;


			//if the character is an RPC
			if( pSoldier->ubProfile >= FIRST_RPC && pSoldier->ubProfile < FIRST_NPC )
			{
				INT16	sSalary = gMercProfiles[ pSoldier->ubProfile ].sSalary;
				INT32	iMoneyOwedToMerc = 0;

				//increment the number of days the mercs has been on the team
				pSoldier->iTotalContractLength++;

				//if the player owes the npc money, the balance field will be negative
				if( gMercProfiles[ pSoldier->ubProfile ].iBalance < 0 )
				{
					//the player owes the npc the salary and whatever money the player owes the npc
					iMoneyOwedToMerc = sSalary + ( - gMercProfiles[ pSoldier->ubProfile ].iBalance );
				}
				else
				{
					//else the player only owes the salary
					iMoneyOwedToMerc = sSalary;
				}

				//if the player owes money
				if( iMoneyOwedToMerc != 0 )
				{
					//if the player can afford to pay them
					if( LaptopSaveInfo.iCurrentBalance >= iMoneyOwedToMerc )
					{
						//add the transaction to the player
						AddTransactionToPlayersBook( PAYMENT_TO_NPC, pSoldier->ubProfile, GetWorldTotalMin(), -iMoneyOwedToMerc);

						//if the player owed money to the npc
						if( gMercProfiles[ pSoldier->ubProfile ].iBalance < 0 )
						{
							// reset the amount
							gMercProfiles[ pSoldier->ubProfile ].iBalance = 0;
						}
					}
					else
					{
						CHAR16	zMoney[128];

						//create a string for the salary owed to the npc
						swprintf( zMoney, L"%d", sSalary );
						InsertCommasForDollarFigure( zMoney );
						InsertDollarSignInToString( zMoney );

						//Display a screen msg indicating that the npc was NOT paid
						ScreenMsg( FONT_MCOLOR_WHITE, MSG_INTERFACE, pMessageStrings[ MSG_CANT_AFFORD_TO_PAY_NPC_DAILY_SALARY_MSG ], gMercProfiles[ pSoldier->ubProfile ].zNickname, zMoney );

						//if the merc hasnt been paid for NUM_DAYS_TILL_UNPAID_RPC_QUITS days, the merc will quit
						if( ( gMercProfiles[ pSoldier->ubProfile ].iBalance - sSalary ) <= -( sSalary * NUM_DAYS_TILL_UNPAID_RPC_QUITS ) )
						{
							//
							//Set it up so the merc quits
							//
							MercsContractIsFinished( pSoldier->ubID );
						}
						else
						{
							//set how much money the player owes the merc
							gMercProfiles[ pSoldier->ubProfile ].iBalance -= sSalary;

							// Add even for displaying a dialogue telling the player this....
							AddSameDayStrategicEvent( EVENT_RPC_WHINE_ABOUT_PAY, MERC_ARRIVE_TIME_SLOT_1, pSoldier->ubID );

						}
					}
				}
			}


			DailyMoraleUpdate( pSoldier );

			CheckIfMercGetsAnotherContract( pSoldier );
		}
		else
		{
			if( ( pSoldier->bActive ) && ( pSoldier->bAssignment == ASSIGNMENT_POW ) )
			{
				pSoldier->iEndofContractTime += 1440;
			}
		}

		// if active, here, & alive (POW is ok, don't care)
		if( ( pSoldier->bActive ) && ( pSoldier->bAssignment != ASSIGNMENT_DEAD ) &&
																 ( pSoldier->bAssignment != IN_TRANSIT ) )
		{
			// increment the "man days" played counter for each such merc in the player's employment
			gStrategicStatus.uiManDaysPlayed++;
		}
	}

	//r eset the counter
	cnt = 0;

	for ( pSoldier = MercPtrs[ cnt ]; cnt <= bLastTeamID; cnt++,pSoldier++)
	{
		//if the merc is active
		if ( ( pSoldier->bActive )&&( pSoldier->bAssignment != ASSIGNMENT_POW ) && ( pSoldier->bAssignment != IN_TRANSIT ) )
		{
			//if its a MERC merc, determine if the merc should leave ( because player refused to pay for merc )
			if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC )
			{
				//if the players account status is invalid
				if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_INVALID )
				{
					//if the soldier is alive anc concious
					if( IsTheSoldierAliveAndConcious( pSoldier ) )
					{
						//if the merc should leave today
						MercsContractIsFinished( pSoldier->ubID );
					}
				}
			}
		}
	}

	//Loop through all the profiles
	for( cnt = 0; cnt < NUM_PROFILES; cnt++)
	{
		pProfile = &(gMercProfiles[ cnt ]);

		// dead guys don't do nuthin' !
		if ( pProfile->bMercStatus == MERC_IS_DEAD )
		{
			continue;
		}

		//Every day reset this variable
		pProfile->uiPrecedentQuoteSaid = 0;

		// skip anyone currently on the player's team
		if ( IsMercOnTeam( (UINT8) cnt ))
		{
			continue;
		}

		// if he's an AIM/M.E.R.C. merc
		if ( cnt < AIM_AND_MERC_MERCS )
		{
			// if he's not just on his way home
			if ( pProfile->bMercStatus != MERC_RETURNING_HOME )
			{
				// check if any of his stats improve through working or training
				HandleUnhiredMercImprovement(pProfile);

				// if he's working on another job
				if (pProfile->bMercStatus == MERC_WORKING_ELSEWHERE)
				{
					// check if he's killed
					HandleUnhiredMercDeaths( cnt );
				}
			}
		}

		// if merc is currently unavailable
		if( pProfile->uiDayBecomesAvailable > 0 )
		{
			// reduce time til available by one day
			pProfile->uiDayBecomesAvailable--;

			// Check to see if the merc has become available
			if (pProfile->uiDayBecomesAvailable == 0)
			{
				//if the merc CAN become ready
				if( pProfile->bMercStatus != MERC_FIRED_AS_A_POW )
				{
					pProfile->bMercStatus = MERC_OK;

					// if the player has left a message for this merc
					if ( pProfile->ubMiscFlags3 & PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM )
					{
						iOffset = AIM_REPLY_BARRY;

						//remove the Flag, so if the merc goes on another assignment, the player can leave an email.
						pProfile->ubMiscFlags3 &= ~PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM;

						// TO DO: send E-mail to player telling him the merc has returned from an assignment
						AddEmail( ( UINT8 )( iOffset + ( cnt * AIM_REPLY_LENGTH_BARRY ) ), AIM_REPLY_LENGTH_BARRY, ( UINT8 )( 6 + cnt ), GetWorldTotalMin() );
					}
				}
			}
		}
		else	// was already available today
		{
			// if it's an AIM or M.E.R.C. merc
			if (cnt < AIM_AND_MERC_MERCS)
			{
				// check to see if he goes on another assignment
				if (cnt < MAX_NUMBER_MERCS)
				{
					// A.I.M. merc
					uiChance = 2 * pProfile->bExpLevel;

					// player has now had a chance to hire him, so he'll eligible to get killed off on another job
					pProfile->ubMiscFlags3 |= PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE;
				}
				else
				{
					// M.E.R.C. merc - very rarely get other work
					uiChance = 1 * pProfile->bExpLevel;

					// player doesn't have a chance to hire any M.E.R.C's until after Speck's E-mail is sent
					if (GetWorldDay() > DAYS_TIL_M_E_R_C_AVAIL)
					{
						// player has now had a chance to hire him, so he'll eligible to get killed off on another job
						pProfile->ubMiscFlags3 |= PROFILE_MISC_FLAG3_PLAYER_HAD_CHANCE_TO_HIRE;
					}
				}

				if (Random(100) < uiChance)
				{
					pProfile->bMercStatus = MERC_WORKING_ELSEWHERE;
					pProfile->uiDayBecomesAvailable = 1 + Random(6 + (pProfile->bExpLevel / 2) );		// 1-(6 to 11) days
				}
			}
		}

		// Decrement morale hangover (merc appears hirable, he just gives lame refusals during this time, though)
		if( pProfile->ubDaysOfMoraleHangover > 0 )
		{
			pProfile->ubDaysOfMoraleHangover--;
		}
	}


	// build quit list
	//BuildMercQuitList( pQuitList );
	HandleSlayDailyEvent( );

	// rebuild list for mapscreen
	ReBuildCharactersList( );
}

/*
void BuildMercQuitList( SOLDIERTYPE *pMercList )
{
	// go through list of mercs on players team, fill pMercList sorted from most recent leave time, to furthest leave time

	INT32		cnt;
	INT8		bLastTeamID;
	SOLDIERTYPE		*pSoldier;
	INT32 iCounter = 0;

	cnt = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
	bLastTeamID = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

	//loop though all the mercs
  for ( pSoldier = MercPtrs[ cnt ]; cnt <= bLastTeamID; cnt++,pSoldier++, iCounter++ )
	{
		if ( pSoldier->bActive )
		{
			// copy to list
			pMercList[ iCounter ] = pSoldier;
		}
	}

	// now sort based on departure time..simple bubble sort
	for( cnt = 0; cnt <=iCounter; cnt++ )
	{
		for( cntB = 0; cntB <=iCounter; cntB++ )
		{
			if( MercPtrs[ cntB ]->ubWhatKindOfMercAmI == MERC_TYPE__MERC )
			{
				//if the players account status is invalid
				if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_INVALID )
				{
					//if the soldier is alive anc concious
					if( IsTheSoldierAliveAndConcious( MercPtrs[ cntB ] ) )
					{
						// swap
						pSoldier =  MercPtrs[ cntB ];
						MercPtrs[ cntB ] =  MercPtrs[ cnt ];
						MercPtrs[ cnt ] = pSoldier;
					}
				}
			}
			else if(  MercPtrs[ cntB ]->ubWhatKindOfMercAmI == MERC_TYPE__AIM )
			{
				if( MercPtrs[ cntB ]->iEndofContractTime < MercPtrs[ cnt ]->iEndofContractTime )
				{
					// swap
					pSoldier =  MercPtrs[ cntB ];
					MercPtrs[ cntB ] =  MercPtrs[ cnt ];
					MercPtrs[ cnt ] = pSoldier;
				}
			}
			else
			{
				// not a hirable guy, move to the botton
				pSoldier =  MercPtrs[ cnt ];
				MercPtrs[ cnt ] =  MercPtrs[ cntB ];
				MercPtrs[ cntB ] = pSoldier;
			}
		}
	}

	return;
}
*/
/*
void HandleMercsAboutToLeave( SOLDIERTYPE *pMercList )
{
	// run from top of list to bottom..if merc is gonna leave within a defined time frame of the previous guy and they are leaving the same day (today)
	// then post then with the quote at the same time

	for( iCounter = 0; iCounter < 21; iCounter++ )
	{
		if( iCounter == 0 )
		{
			// first guy, if he no leave today, no one is leave, go home
			if( ( pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__MERC ) && (  pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__AIM ) )
			{
				return;
			}
			else
			{
				if( ( pSoldier->iEndofContractTime / 1440 ) > (INT32)GetWorldDay( ) )
				{
					return;
				}
				else
				{
					//if the soldier is alive anc concious
				if( IsTheSoldierAliveAndConcious( pSoldier ) )
				{
					//if the soldier is an AIM merc
					if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC )
					{
						//add an event so the merc will say the departing warning ( 2 hours prior to leaving
						AddSameDayStrategicEvent( EVENT_MERC_ABOUT_TO_LEAVE_COMMENT, MERC_DEPARTURE_TIME_OF_DAY - 2 * 60,	(UINT32) pSoldier->ubID );
					}
					else
					{
						// merc merc, they'll just leave

					}
				}
			}
		}
	}
}

	*/


// ATE: This function deals with MERC MERC and NPC's leaving because of not getting paid...
// NOT AIM renewals....
void MercsContractIsFinished( UINT8	ubID )
{
	SOLDIERTYPE *pSoldier;

	#ifndef JA2DEMO

	pSoldier = &Menptr[ ubID ];

	//if the soldier was removed before getting into this function, return
	if( !pSoldier->bActive )
		return;

	if( fShowContractMenu )
	{
		fShowContractMenu = FALSE;
	}

	// go to mapscreen
	SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_ENTER_MAPSCREEN,0,0,0,0,0 );


	if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC )
	{
		//if the players account status is invalid
		if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_INVALID )
		{
			//Send the merc home

			InterruptTime( );
			PauseGame();
			LockPauseState( 9 );

			// Say quote for wishing to leave
			TacticalCharacterDialogue( pSoldier, QUOTE_NOT_GETTING_PAID );

			TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING_NO_ASK_EQUIP, 0, 0 );

			pSoldier->ubLeaveHistoryCode = HISTORY_MERC_QUIT;
		}
	}
	else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC )
	{
		InterruptTime( );
		PauseGame();
		LockPauseState( 10 );

		TacticalCharacterDialogue( pSoldier, QUOTE_AIM_SEEN_MIKE );

		TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING_NO_ASK_EQUIP, 0, 0 );

		pSoldier->ubLeaveHistoryCode = HISTORY_MERC_QUIT;

	}

	#endif
}

// ATE: Called for RPCs who should now complain about no pay...
void RPCWhineAboutNoPay( UINT8	ubID )
{
	SOLDIERTYPE *pSoldier;

	#ifndef JA2DEMO

	pSoldier = &Menptr[ ubID ];

	//if the soldier was removed before getting into this function, return
	if( !pSoldier->bActive )
		return;

	if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC )
	{
		// Say quote for needing pay!
		TacticalCharacterDialogue( pSoldier, QUOTE_NOT_GETTING_PAID );
	}

	#endif
}


// OK loop through and check!
BOOLEAN SoldierHasWorseEquipmentThanUsedTo( SOLDIERTYPE *pSoldier )
{
	INT32		cnt;
	UINT16	usItem;
	INT8		bBestArmour = -1;
	INT8		bBestArmourIndex = -1;
	INT8		bBestGun = -1;
	INT8		bBestGunIndex = -1;


	for ( cnt = 0; cnt < NUM_INV_SLOTS; cnt++ )
	{
		usItem = pSoldier->inv[ cnt ].usItem;

		// Look for best gun/armour
		if ( usItem != NOTHING )
		{
			// Check if it's a gun
			if ( Item[ usItem ].usItemClass & IC_GUN )
			{
				if ( Weapon[ usItem ].ubDeadliness > bBestGun )
				{
					bBestGunIndex = (INT8)cnt;
					bBestGun = Weapon[ usItem ].ubDeadliness;
				}
			}

			// If it's armour
			if ( Item[ usItem ].usItemClass & IC_ARMOUR )
			{
				if ( Armour[ Item[ usItem ].ubClassIndex ].ubProtection > bBestArmour )
				{
					bBestArmourIndex = (INT8)cnt;
					bBestArmour = Armour[ Item[ usItem ].ubClassIndex ].ubProtection;
				}
			}
		}
	}

	// Modify these values based on morale - lower opinion of equipment if morale low, increase if high
	// this of course assumes default morale is 50
	if ( bBestGun != -1 )
	{
		bBestGun		= (bBestGun		 * (50 + pSoldier->bMorale)) / 100;
	}
	if ( bBestArmour != -1 )
	{
		bBestArmour = (bBestArmour * (50 + pSoldier->bMorale)) / 100;
	}

	// OK, check values!
	if ( 	(bBestGun != -1 && bBestGun < ( gMercProfiles[ pSoldier->ubProfile ].bMainGunAttractiveness / 2 )) ||
				(bBestArmour != -1 && bBestArmour < ( gMercProfiles[ pSoldier->ubProfile ].bArmourAttractiveness / 2 )) )
	{
		// Pipe up!
		return( TRUE );
	}

	return( FALSE );
}


void MercComplainAboutEquipment( UINT8 ubProfile )
{
	SOLDIERTYPE *pSoldier;

	if ( ubProfile == LARRY_NORMAL  )
	{
		if ( CheckFact( FACT_LARRY_CHANGED, 0 ) )
		{
			ubProfile = LARRY_DRUNK;
		}
	}
	else if ( ubProfile == LARRY_DRUNK )
	{
		if ( CheckFact( FACT_LARRY_CHANGED, 0 ) == FALSE )
		{
			ubProfile = LARRY_NORMAL;
		}
	}
	// Are we dead/ does merc still exist?
	pSoldier = FindSoldierByProfileID( ubProfile, FALSE );

	if ( pSoldier != NULL )
	{
		if ( pSoldier->bLife >= OKLIFE && pSoldier->fMercAsleep != TRUE && pSoldier->bAssignment < ON_DUTY )
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



void UpdateBuddyAndHatedCounters( void )
{
	INT8									bMercID;
	INT32									iLoop;
	INT8									bOtherID;
	INT8									bLastTeamID;
	UINT8									ubOtherProfileID;
	SOLDIERTYPE						*pSoldier;
	SOLDIERTYPE						*pOtherSoldier;
	MERCPROFILESTRUCT			*pProfile;
	BOOLEAN								fSameGroupOnly;

	BOOLEAN								fUpdatedTimeTillNextHatedComplaint = FALSE;

	bMercID = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
	bLastTeamID = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

	//loop though all the mercs
  for ( pSoldier = MercPtrs[ bMercID ]; bMercID <= bLastTeamID; bMercID++,pSoldier++)
	{
		fSameGroupOnly = FALSE;

		//if the merc is active and on a combat assignment
		if ( pSoldier->bActive && pSoldier->bAssignment < ON_DUTY )
		{
			pProfile = &(gMercProfiles[ pSoldier->ubProfile ]);

			// if we're moving, we only check vs other people in our squad
			if (pSoldier->ubGroupID != 0 && PlayerIDGroupInMotion( pSoldier->ubGroupID ))
			{
				fSameGroupOnly = TRUE;
			}

			fUpdatedTimeTillNextHatedComplaint = FALSE;

			bOtherID = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;

			for ( pOtherSoldier = MercPtrs[ bOtherID ]; bOtherID <= bLastTeamID; bOtherID++, pOtherSoldier++)
			{
				// is this guy in the same sector and on active duty (or in the same moving group)

				if (bOtherID != bMercID && pOtherSoldier->bActive && pOtherSoldier->bAssignment < ON_DUTY )
				{
					if (fSameGroupOnly)
					{
						// all we have to check is the group ID
						if (pSoldier->ubGroupID != pOtherSoldier->ubGroupID)
						{
							continue;
						}
					}
					else
					{
						// check to see if the location is the same
						if (pOtherSoldier->sSectorX != pSoldier->sSectorX ||
							  pOtherSoldier->sSectorY != pSoldier->sSectorY ||
								pOtherSoldier->bSectorZ != pSoldier->bSectorZ)
						{
							continue;
						}

						// if the OTHER soldier is in motion then we don't do anything!
						if (pOtherSoldier->ubGroupID != 0 && PlayerIDGroupInMotion( pOtherSoldier->ubGroupID ))
						{
							continue;
						}
					}

					ubOtherProfileID = pOtherSoldier->ubProfile;

					for ( iLoop = 0; iLoop < 4; iLoop++ )
					{
						switch( iLoop )
						{
							case 0:
							case 1:
								if (pProfile->bHated[iLoop] == ubOtherProfileID)
								{
									// arrgs, we're on assignment with the person we loathe!
									if ( pProfile->bHatedCount[iLoop] > 0 )
									{
										pProfile->bHatedCount[iLoop]--;
										if ( pProfile->bHatedCount[iLoop] == 0 && pSoldier->bInSector && gTacticalStatus.fEnemyInSector )
										{
											// just reduced count to 0 but we have enemy in sector...
											pProfile->bHatedCount[iLoop] = 1;
										}
										else if (pProfile->bHatedCount[iLoop] > 0 && (pProfile->bHatedCount[iLoop] == pProfile->bHatedTime[iLoop] / 2 || ( pProfile->bHatedCount[iLoop] < pProfile->bHatedTime[iLoop] / 2 && pProfile->bHatedCount[iLoop] % TIME_BETWEEN_HATED_COMPLAINTS == 0 ) ) )
										{
											// complain!
											if (iLoop == 0)
											{
												TacticalCharacterDialogue( pSoldier, QUOTE_HATED_MERC_ONE );
											}
											else
											{
												TacticalCharacterDialogue( pSoldier, QUOTE_HATED_MERC_TWO );
											}
											StopTimeCompression();
										}
										else if ( pProfile->bHatedCount[iLoop] == 0 )
										{
											// zero count!
											if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC || pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC )
											{
												// MERC mercs leave now!
												if (iLoop == 0)
												{
													TacticalCharacterDialogue( pSoldier, QUOTE_MERC_QUIT_HATED1 );
												}
												else
												{
													TacticalCharacterDialogue( pSoldier, QUOTE_MERC_QUIT_HATED2 );
												}

												// Leave now! ( handle equipment too )....
												TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING, 0,0 );

												pSoldier->ubLeaveHistoryCode = HISTORY_MERC_QUIT;
											}
											else
											{
												// complain!
												if (iLoop == 0)
												{
													TacticalCharacterDialogue( pSoldier, QUOTE_HATED_MERC_ONE );
												}
												else
												{
													TacticalCharacterDialogue( pSoldier, QUOTE_HATED_MERC_TWO );
												}
												pProfile->ubTimeTillNextHatedComplaint = TIME_BETWEEN_HATED_COMPLAINTS - 1;
											}
										}
									}
									else
									{
										// if we haven't updated the time till our next complaint, do so
										// if it's 0, gripe.
										if ( !fUpdatedTimeTillNextHatedComplaint )
										{
											if ( pProfile->ubTimeTillNextHatedComplaint == 0 )
											{
												pProfile->ubTimeTillNextHatedComplaint = TIME_BETWEEN_HATED_COMPLAINTS - 1;
											}
											else
											{
												pProfile->ubTimeTillNextHatedComplaint--;
											}
											fUpdatedTimeTillNextHatedComplaint = TRUE;
										}

										if ( pProfile->ubTimeTillNextHatedComplaint == 0 )
										{
											// complain!
											if (iLoop == 0)
											{
												TacticalCharacterDialogue( pSoldier, QUOTE_HATED_MERC_ONE );
											}
											else
											{
												TacticalCharacterDialogue( pSoldier, QUOTE_HATED_MERC_TWO );
											}
										}
									}
								}
								break;
							case 2:
								if (pProfile->bLearnToHate == ubOtherProfileID)
								{
									if ( pProfile->bLearnToHateCount > 0 )
									{
										pProfile->bLearnToHateCount--;
										if ( pProfile->bLearnToHateCount == 0 && pSoldier->bInSector && gTacticalStatus.fEnemyInSector )
										{
											// just reduced count to 0 but we have enemy in sector...
											pProfile->bLearnToHateCount = 1;
										}
										else if (pProfile->bLearnToHateCount > 0 && (pProfile->bLearnToHateCount == pProfile->bLearnToHateTime / 2 || pProfile->bLearnToHateCount < pProfile->bLearnToHateTime / 2 && pProfile->bLearnToHateCount % TIME_BETWEEN_HATED_COMPLAINTS == 0 ) )
										{
											// complain!
											TacticalCharacterDialogue( pSoldier, QUOTE_LEARNED_TO_HATE_MERC );
											StopTimeCompression();
										}
										else if (pProfile->bLearnToHateCount == 0)
										{
											// set as bHated[2];
											pProfile->bHated[2] = pProfile->bLearnToHate;
											pProfile->bMercOpinion[ubOtherProfileID] = HATED_OPINION;

											if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC || (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC && (pSoldier->ubProfile == DEVIN || pSoldier->ubProfile == SLAY || pSoldier->ubProfile == IGGY || pSoldier->ubProfile == CONRAD ) ) )
											{
												// Leave now! ( handle equipment too )....
												TacticalCharacterDialogue( pSoldier, QUOTE_MERC_QUIT_LEARN_TO_HATE );
												TacticalCharacterDialogueWithSpecialEvent( pSoldier, 0, DIALOGUE_SPECIAL_EVENT_CONTRACT_ENDING, 0,0 );
												pSoldier->ubLeaveHistoryCode = HISTORY_MERC_QUIT;

											}
											else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__NPC)
											{
												// whine again
												TacticalCharacterDialogue( pSoldier, QUOTE_LEARNED_TO_HATE_MERC );
											}

										}
										if (pProfile->bLearnToHateCount < pProfile->bLearnToHateTime / 2)
										{
											// gradual opinion drop
											pProfile->bMercOpinion[ubOtherProfileID] += (HATED_OPINION - pProfile->bMercOpinion[ubOtherProfileID]) / (pProfile->bLearnToHateCount + 1);
										}
									}
									else
									{
										if ( !fUpdatedTimeTillNextHatedComplaint )
										{
											if ( pProfile->ubTimeTillNextHatedComplaint == 0 )
											{
												pProfile->ubTimeTillNextHatedComplaint = TIME_BETWEEN_HATED_COMPLAINTS - 1;
											}
											else
											{
												pProfile->ubTimeTillNextHatedComplaint--;
											}
											fUpdatedTimeTillNextHatedComplaint = TRUE;
										}

										if ( pProfile->ubTimeTillNextHatedComplaint == 0 )
										{
											// complain!
											TacticalCharacterDialogue( pSoldier, QUOTE_LEARNED_TO_HATE_MERC );
										}
									}
								}
								break;
							case 3:
								if (pProfile->bLearnToLikeCount > 0	&& pProfile->bLearnToLike == ubOtherProfileID)
								{
									pProfile->bLearnToLikeCount--;
									if (pProfile->bLearnToLikeCount == 0)
									{
										// add to liked!
										pProfile->bBuddy[2] = pProfile->bLearnToLike;
										pProfile->bMercOpinion[ubOtherProfileID] = BUDDY_OPINION;
									}
									else if (pProfile->bLearnToLikeCount < pProfile->bLearnToLikeTime / 2)
									{
										// increase opinion of them!
										pProfile->bMercOpinion[ubOtherProfileID] += (BUDDY_OPINION - pProfile->bMercOpinion[ubOtherProfileID]) / (pProfile->bLearnToLikeCount + 1);
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
}

void HourlyCamouflageUpdate( void )
{
	INT8 bMercID, bLastTeamID;
	SOLDIERTYPE * pSoldier;

	bMercID = gTacticalStatus.Team[ gbPlayerNum ].bFirstID;
	bLastTeamID = gTacticalStatus.Team[ gbPlayerNum ].bLastID;

	// loop through all mercs
  for ( pSoldier = MercPtrs[ bMercID ]; bMercID <= bLastTeamID; bMercID++,pSoldier++)
	{
		if ( pSoldier->bActive )
		{
			// if the merc has non-zero camo, degrade it by 1%
			if( ( pSoldier->bCamo > 0) && ( !( HAS_SKILL_TRAIT( pSoldier, CAMOUFLAGED) ) ) )
			{
				pSoldier->bCamo -= 2;
				if (pSoldier->bCamo <= 0)
				{
					pSoldier->bCamo = 0;
					// Reload palettes....
					if ( pSoldier->bInSector )
					{
						CreateSoldierPalettes( pSoldier );
					}

					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_CAMMO_WORN_OFF], pSoldier->name );
					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
				}
			}
			// if the merc has non-zero monster smell, degrade it by 1
			if ( pSoldier->bMonsterSmell > 0 )
			{
				pSoldier->bMonsterSmell--;

				/*
				if (pSoldier->bMonsterSmell == 0)
				{
					// Reload palettes....

					if ( pSoldier->bInSector )
					{
						CreateSoldierPalettes( pSoldier );
					}

					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, Message[STR_CAMMO_WORN_OFF], pSoldier->name );
					DirtyMercPanelInterface( pSoldier, DIRTYLEVEL2 );
				}
				*/
			}
		}
	}
}
