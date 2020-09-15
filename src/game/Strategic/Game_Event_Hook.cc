#include "Hourly_Update.h"
#include "Types.h"
#include "Game_Events.h"
#include "SoundMan.h"
#include "Environment.h"
#include "Ambient_Control.h"
#include "Quests.h"
#include "AIMMembers.h"
#include "Strategic_Event_Handler.h"
#include "BobbyR.h"
#include "Mercs.h"
#include "EMail.h"
#include "Game_Clock.h"
#include "Merc_Hiring.h"
#include "Insurance_Contract.h"
#include "Strategic_Merc_Handler.h"
#include "Strategic_Movement.h"
#include "Assignments.h"
#include "Strategic_Mines.h"
#include "Strategic_Town_Loyalty.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Helicopter.h"
#include "Scheduling.h"
#include "BobbyRGuns.h"
#include "Arms_Dealer_Init.h"
#include "Meanwhile.h"
#include "Overhead.h"
#include "Random.h"
#include "Creature_Spreading.h"
#include "Strategic_AI.h"
#include "Merc_Contract.h"
#include "Strategic_Status.h"
#include "Soldier_Profile.h"
#include "Laptop.h"
#include "Campaign.h"
#include "Debug.h"

extern UINT32	guiTimeStampOfCurrentlyExecutingEvent;
extern BOOLEAN gfPreventDeletionOfAnyEvent;


static BOOLEAN DelayEventIfBattleInProgress(STRATEGICEVENT* pEvent)
{
	STRATEGICEVENT *pNewEvent;
	if( gTacticalStatus.fEnemyInSector )
	{
		pNewEvent = AddAdvancedStrategicEvent(static_cast<StrategicEventFrequency>(pEvent->ubEventType), static_cast<StrategicEventKind>(pEvent->ubCallbackID), pEvent->uiTimeStamp + 180 + Random(121), pEvent->uiParam);
		Assert( pNewEvent );
		pNewEvent->uiTimeOffset = pEvent->uiTimeOffset;
		return TRUE;
	}
	return FALSE;
}

BOOLEAN ExecuteStrategicEvent( STRATEGICEVENT *pEvent )
{
	BOOLEAN fOrigPreventFlag;

	fOrigPreventFlag = gfPreventDeletionOfAnyEvent;
	gfPreventDeletionOfAnyEvent = TRUE;
	//No events can be posted before this time when gfProcessingGameEvents is set, otherwise,
	//we have a chance of running into an infinite loop.
	guiTimeStampOfCurrentlyExecutingEvent = pEvent->uiTimeStamp;

	if( pEvent->ubFlags & SEF_DELETION_PENDING)
	{
		gfPreventDeletionOfAnyEvent = fOrigPreventFlag;
		return FALSE;
	}

	// Look at the ID of event and do stuff according to that!
	switch( pEvent->ubCallbackID )
	{
		case EVENT_CHANGELIGHTVAL:
			// Change light to value
			gubEnvLightValue = (UINT8)pEvent->uiParam;
			if( !gfBasement && !gfCaves )
				gfDoLighting = TRUE;
			break;
		case EVENT_CHECKFORQUESTS:
			CheckForQuests( GetWorldDay() );
			break;
		case EVENT_AMBIENT:
			if( pEvent->ubEventType == ENDRANGED_EVENT )
			{
				if (pEvent->uiParam != NO_SAMPLE) SoundStopRandom(pEvent->uiParam);
			}
			else
			{
				pEvent->uiParam = SetupNewAmbientSound( pEvent->uiParam );
			}
			break;
		case EVENT_AIM_RESET_MERC_ANNOYANCE:
			ResetMercAnnoyanceAtPlayer( (UINT8) pEvent->uiParam );
			break;
		//The players purchase from Bobby Ray has arrived
		case EVENT_BOBBYRAY_PURCHASE:
			BobbyRayPurchaseEventCallback( (UINT8) pEvent->uiParam);
			break;
		//Gets called once a day ( at BOBBYRAY_UPDATE_TIME).  To simulate the items being bought and sold at bobby rays
		case EVENT_DAILY_UPDATE_BOBBY_RAY_INVENTORY:
			DailyUpdateOfBobbyRaysNewInventory();
			DailyUpdateOfBobbyRaysUsedInventory();
			DailyUpdateOfArmsDealersInventory();
			break;
		//Add items to BobbyR's new/used inventory
		case EVENT_UPDATE_BOBBY_RAY_INVENTORY:
			AddFreshBobbyRayInventory( (UINT16) pEvent->uiParam );
			break;
		//Called once a day to update the number of days that a hired merc from M.E.R.C. has been on contract.
		// Also if the player hasn't paid for a while Specks will start sending e-mails to the player
		case EVENT_DAILY_UPDATE_OF_MERC_SITE:
			DailyUpdateOfMercSite( (UINT16)GetWorldDay() );
			break;
		case EVENT_DAY3_ADD_EMAIL_FROM_SPECK:
			AddEmail(MERC_INTRO, MERC_INTRO_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin( ) );
			break;
		case EVENT_DAY2_ADD_EMAIL_FROM_IMP:
			AddEmail(IMP_EMAIL_PROFILE_RESULTS, IMP_EMAIL_PROFILE_RESULTS_LENGTH, IMP_PROFILE_RESULTS, GetWorldTotalMin( ) );
			break;
		//If a merc gets hired and they dont show up immediately, the merc gets added to the queue and shows up
		// uiTimeTillMercArrives  minutes later
		case EVENT_DELAYED_HIRING_OF_MERC:
			MercArrivesCallback(GetMan(pEvent->uiParam));
			break;
		//handles the life insurance contract for a merc from AIM.
		case EVENT_HANDLE_INSURED_MERCS:
			DailyUpdateOfInsuredMercs();
			break;
		//handles when a merc is killed an there is a life insurance payout
		case EVENT_PAY_LIFE_INSURANCE_FOR_DEAD_MERC:
			InsuranceContractPayLifeInsuranceForDeadMerc( (UINT8) pEvent->uiParam );
			break;
		//gets called every day at midnight.
		case EVENT_MERC_DAILY_UPDATE:
			MercDailyUpdate();
			break;
		case EVENT_MERC_ABOUT_TO_LEAVE:
			FindOutIfAnyMercAboutToLeaveIsGonnaRenew( );
			break;
		//Whenever any group (player or enemy) arrives in a new sector during movement.
		case EVENT_GROUP_ARRIVAL:
			GroupArrivedAtSector(*GetGroup((UINT8)pEvent->uiParam), TRUE, FALSE);
			break;
		case EVENT_MERC_COMPLAIN_EQUIPMENT:
			MercComplainAboutEquipment( (UINT8) pEvent->uiParam );
			break;
		case EVENT_HOURLY_UPDATE:
			HandleHourlyUpdate();
			break;
		case EVENT_MINUTE_UPDATE:
			/* unused, used to be empty */
			break;
		case EVENT_HANDLE_MINE_INCOME:
			HandleIncomeFromMines( );
			//ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, ST::format("Income From Mines at {}", GetWorldTotalMin()) );
			break;
		case EVENT_SETUP_MINE_INCOME:
			PostEventsForMineProduction();
			break;
		case EVENT_SET_BY_NPC_SYSTEM:
			HandleNPCSystemEvent( pEvent->uiParam );
			break;
		case EVENT_SECOND_AIRPORT_ATTENDANT_ARRIVED:
			AddSecondAirportAttendant();
			break;
		case EVENT_HELICOPTER_HOVER_TOO_LONG:
			HandleHeliHoverLong( );
			break;
		case EVENT_HELICOPTER_HOVER_WAY_TOO_LONG:
			HandleHeliHoverTooLong( );
			break;
		case EVENT_MERC_LEAVE_EQUIP_IN_DRASSEN:
			HandleEquipmentLeftInDrassen( pEvent->uiParam );
			break;
		case EVENT_MERC_LEAVE_EQUIP_IN_OMERTA:
			HandleEquipmentLeftInOmerta( pEvent->uiParam );
			break;
		case EVENT_BANDAGE_BLEEDING_MERCS:
			BandageBleedingDyingPatientsBeingTreated( );
			break;
		case EVENT_DAILY_EARLY_MORNING_EVENTS:
			HandleEarlyMorningEvents();
			break;
		case EVENT_GROUP_ABOUT_TO_ARRIVE:
			HandleGroupAboutToArrive();
			break;
		case EVENT_PROCESS_TACTICAL_SCHEDULE:
			ProcessTacticalSchedule( (UINT8)pEvent->uiParam );
			break;
		case EVENT_BEGINRAINSTORM:
			//EnvBeginRainStorm( (UINT8)pEvent->uiParam );
			break;
		case EVENT_ENDRAINSTORM:
			//EnvEndRainStorm( );
			break;
		case EVENT_RAINSTORM:

			// ATE: Disabled
			//
			//if( pEvent->ubEventType == ENDRANGED_EVENT )
			//{
			//	EnvEndRainStorm( );
			//}
			//else
			//{
			//	EnvBeginRainStorm( (UINT8)pEvent->uiParam );
			//}
			break;

		case EVENT_MAKE_CIV_GROUP_HOSTILE_ON_NEXT_SECTOR_ENTRANCE:
			MakeCivGroupHostileOnNextSectorEntrance( (UINT8)pEvent->uiParam );
			break;
		case EVENT_MEANWHILE:
			if( !DelayEventIfBattleInProgress( pEvent ) )
			{
				BeginMeanwhile( (UINT8)pEvent->uiParam );
				InterruptTime();
			}
			break;
		case EVENT_CREATURE_SPREAD:
			SpreadCreatures();
			break;
		case EVENT_CREATURE_NIGHT_PLANNING:
			CreatureNightPlanning();
			break;
		case EVENT_CREATURE_ATTACK:
			CreatureAttackTown( (UINT8)pEvent->uiParam, FALSE );
			break;
		case EVENT_EVALUATE_QUEEN_SITUATION:
			EvaluateQueenSituation();
			break;
		case EVENT_CHECK_ENEMY_CONTROLLED_SECTOR:
			CheckEnemyControlledSector( (UINT8)pEvent->uiParam );
			break;
		case EVENT_TURN_ON_NIGHT_LIGHTS:
			TurnOnNightLights();
			break;
		case EVENT_TURN_OFF_NIGHT_LIGHTS:
			TurnOffNightLights();
			break;
		case EVENT_TURN_ON_PRIME_LIGHTS:
			TurnOnPrimeLights();
			break;
		case EVENT_TURN_OFF_PRIME_LIGHTS:
			TurnOffPrimeLights();
			break;
		case EVENT_ENRICO_MAIL:
			HandleEnricoEmail();
			break;
		case EVENT_INSURANCE_INVESTIGATION_STARTED:
			StartInsuranceInvestigation( (UINT8) pEvent->uiParam );
			break;
		case EVENT_INSURANCE_INVESTIGATION_OVER:
			EndInsuranceInvestigation( (UINT8) pEvent->uiParam );
			break;
		case EVENT_TEMPERATURE_UPDATE:
			UpdateTemperature( (UINT8) pEvent->uiParam );
			break;
		case EVENT_KEITH_GOING_OUT_OF_BUSINESS:
			// make sure killbillies are still alive, if so, set fact 274 true
			if (!CheckFact(FACT_HILLBILLIES_KILLED, KEITH))
			{
				//s et the fact true keith is out of business
				SetFactTrue( FACT_KEITH_OUT_OF_BUSINESS );
			}
			break;
		case EVENT_MERC_SITE_BACK_ONLINE:
			GetMercSiteBackOnline();
			break;
		case EVENT_CHECK_IF_MINE_CLEARED:
			// If so, the head miner will say so, and the mine's shutdown will be ended.
			HourlyMinesUpdate();		// not-so hourly, in this case!
			break;
		case EVENT_REMOVE_ASSASSIN:
			RemoveAssassin( (UINT8) pEvent->uiParam );
			break;
		case EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE:
			BeginContractRenewalSequence( );
			break;
		case EVENT_RPC_WHINE_ABOUT_PAY:
			RPCWhineAboutNoPay(GetMan(pEvent->uiParam));
			break;

		case EVENT_HAVENT_MADE_IMP_CHARACTER_EMAIL:
			HaventMadeImpMercEmailCallBack();
			break;

		case EVENT_QUARTER_HOUR_UPDATE:
			HandleQuarterHourUpdate();
			break;

		case EVENT_MERC_MERC_WENT_UP_LEVEL_EMAIL_DELAY:
			MERCMercWentUpALevelSendEmail( (UINT8) pEvent->uiParam );
			break;

		case EVENT_MERC_SITE_NEW_MERC_AVAILABLE:
			NewMercsAvailableAtMercSiteCallBack( );
			break;
	}
	gfPreventDeletionOfAnyEvent = fOrigPreventFlag;
	return TRUE;
}
