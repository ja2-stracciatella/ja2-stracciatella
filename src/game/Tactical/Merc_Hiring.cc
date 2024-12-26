#include "Font_Control.h"
#include "math.h"
#include "MapScreen.h"
#include "MessageBoxScreen.h"
#include "Assignments.h"
#include "Soldier_Control.h"
#include "Random.h"
#include "Overhead.h"
#include "Soldier_Profile.h"
#include "Game_Clock.h"
#include "Soldier_Create.h"
#include "Merc_Hiring.h"
#include "Game_Event_Hook.h"
#include "Message.h"
#include "StrategicMap.h"
#include "Strategic.h"
#include "Items.h"
#include "History.h"
#include "Squads.h"
#include "Strategic_Merc_Handler.h"
#include "Dialogue_Control.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Map.h"
#include "ScreenIDs.h"
#include "JAScreens.h"
#include "Text.h"
#include "Merc_Contract.h"
#include "LaptopSave.h"
#include "Personnel.h"
#include "Auto_Resolve.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Quests.h"
#include "Logger.h"
#include "GamePolicy.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "Observable.h"

#include <string_theory/string>


#define MIN_FLIGHT_PREP_TIME 6

extern BOOLEAN gfTacticalDoHeliRun;
extern BOOLEAN gfFirstHeliRun;
SGPSector g_merc_arrive_sector;

Observable<SOLDIERTYPE*> OnMercHired{};


void CreateSpecialItem(SOLDIERTYPE* const s, UINT16 item)
{
	OBJECTTYPE o;
	CreateItem(item, 100, &o);
	BOOLEAN fReturn = AutoPlaceObject(s, &o, false);
	if (!fReturn) {
		// no space, so overwrite an existing item (can happen when importing IMPs)
		s->inv[SMALLPOCK8POS] = o;
	}
}

INT8 HireMerc(MERC_HIRE_STRUCT& h)
{
	ProfileID  const   pid = h.ubProfileID;
	MERCPROFILESTRUCT& p   = GetProfile(pid);

	// If we are to disregard the status of the merc
	switch (p.bMercStatus)
	{
		case 0:
		case MERC_ANNOYED_BUT_CAN_STILL_CONTACT:
		case MERC_HIRED_BUT_NOT_ARRIVED_YET:
			break;

		default:
			return MERC_HIRE_FAILED;
	}

	if (NumberOfMercsOnPlayerTeam() >= 18) return MERC_HIRE_OVER_20_MERCS_HIRED;

	// ATE: if we are to use landing zone, update to latest value
	// they will be updated again just before arrival...
	if (h.fUseLandingZoneForArrival)
	{
		h.sSector = g_merc_arrive_sector;
	}

	SOLDIERCREATE_STRUCT MercCreateStruct;
	MercCreateStruct = SOLDIERCREATE_STRUCT{};
	MercCreateStruct.ubProfile             = pid;
	MercCreateStruct.sSector               = h.sSector;
	MercCreateStruct.bTeam                 = OUR_TEAM;
	MercCreateStruct.fCopyProfileItemsOver = h.fCopyProfileItemsOver;
	SOLDIERTYPE* const s = TacticalCreateSoldier(MercCreateStruct);
	if (s == NULL)
	{
		SLOGW("TacticalCreateSoldier in HireMerc():  Failed to Add Merc");
		return MERC_HIRE_FAILED;
	}

	if (DidGameJustStart())
	{
		// OK, CHECK FOR FIRST GUY, GIVE HIM SPECIAL ITEM!
		if (s->ubID == 0)
		{
			// OK, give this item to our merc!
			CreateSpecialItem(s, LETTER);
		}

		// Set insertion for first time in chopper
		h.ubInsertionCode = INSERTION_CODE_CHOPPER;
	}

	// Record how long the merc will be gone for
	p.bMercStatus = (UINT8)h.iTotalContractLength;

	// Copy over insertion data
	s->ubStrategicInsertionCode = h.ubInsertionCode;
	s->usStrategicInsertionData = h.usInsertionData;
	// ATE: Copy over value for using landing zone to soldier type
	s->fUseLandingZoneForArrival = h.fUseLandingZoneForArrival;

	// Set assignment
	if (s->bAssignment != IN_TRANSIT) SetTimeOfAssignmentChangeForMerc(s);
	ChangeSoldiersAssignment(s, IN_TRANSIT);

	// Set the contract length
	s->iTotalContractLength = h.iTotalContractLength;

	// Reset the insurance values
	s->iStartOfInsuranceContract       = 0;
	s->iTotalLengthOfInsuranceContract = 0;

	// Store arrival time in soldier structure so map screen can display it
	s->uiTimeSoldierWillArrive = h.uiTimeTillMercArrives;

	if (DidGameJustStart())
	{
		// Set time of initial merc arrival in minutes
		h.uiTimeTillMercArrives = (STARTING_TIME + FIRST_ARRIVAL_DELAY) / NUM_SEC_IN_MIN;

		//set when the merc's contract is finished
		s->iEndofContractTime = GetMidnightOfFutureDayInMinutes(s->iTotalContractLength) + GetHourWhenContractDone(s) * 60;
	}
	else
	{
		//set when the merc's contract is finished (+1 cause it takes a day for the merc to arrive)
		s->iEndofContractTime = GetMidnightOfFutureDayInMinutes(1 + s->iTotalContractLength) + GetHourWhenContractDone(s) * 60;
	}

	// Set the time and ID of the last hired merc will arrive
	LaptopSaveInfo.sLastHiredMerc.iIdOfMerc     = pid;
	LaptopSaveInfo.sLastHiredMerc.uiArrivalTime = h.uiTimeTillMercArrives;

	// If we are trying to hire a merc that should arrive later, put the merc in the queue
	if (h.uiTimeTillMercArrives != 0)
	{
		AddStrategicEvent(EVENT_DELAYED_HIRING_OF_MERC, h.uiTimeTillMercArrives, s->ubID);
		// Specify that the merc is hired but has not arrived yet
		p.bMercStatus = MERC_HIRED_BUT_NOT_ARRIVED_YET;
	}

	// Set the type of merc
	if (h.bWhatKindOfMerc == MERC_TYPE__AIM_MERC)
	{
		s->ubWhatKindOfMercAmI = MERC_TYPE__AIM_MERC;

		// Determine how much the contract is, and remember what type of contract he got
		switch (h.iTotalContractLength)
		{
			case 1:
				s->bTypeOfLastContract   = CONTRACT_EXTEND_1_DAY;
				s->iTimeCanSignElsewhere = GetWorldTotalMin();
				break;

			case 7:
				s->bTypeOfLastContract   = CONTRACT_EXTEND_1_WEEK;
				s->iTimeCanSignElsewhere = GetWorldTotalMin();
				break;

			case 14:
				s->bTypeOfLastContract   = CONTRACT_EXTEND_2_WEEK;
				// This fellow needs to stay the whole duration!
				s->iTimeCanSignElsewhere = s->iEndofContractTime;
				break;
		}

		// remember the medical deposit we PAID.  The one in his profile can increase when he levels!
		s->usMedicalDeposit = p.sMedicalDepositAmount;
	}
	else if (h.bWhatKindOfMerc == MERC_TYPE__MERC)
	{
		s->ubWhatKindOfMercAmI = MERC_TYPE__MERC;

		p.iMercMercContractLength = 1;

		// Set starting conditions for the merc
		s->iStartContractTime = GetWorldDay();

		AddHistoryToPlayersLog(HISTORY_HIRED_MERC_FROM_MERC, pid, GetWorldTotalMin(), SGPSector(-1, -1));
	}
	else if (h.bWhatKindOfMerc == MERC_TYPE__PLAYER_CHARACTER)
	{
		s->ubWhatKindOfMercAmI = MERC_TYPE__PLAYER_CHARACTER;
	}
	else
	{
		s->ubWhatKindOfMercAmI = MERC_TYPE__NPC;
	}

	// remove the merc from the Personnel screens departed list (if they have never been hired before, its ok to call it)
	RemoveNewlyHiredMercFromPersonnelDepartedList(s->ubProfile);

	OnMercHired(s);

	gfAtLeastOneMercWasHired = true;
	return MERC_HIRE_OK;
}


static void CheckForValidArrivalSector(void);


void MercArrivesCallback(SOLDIERTYPE& s)
{
	UINT32 uiTimeOfPost;
	static const SGPSector start(gamepolicy(start_sector));

	if (!DidGameJustStart() && g_merc_arrive_sector == start)
	{
		// Mercs arriving in start sector. This sector has been deemed as the always
		// safe sector. Seeing we don't support entry into a hostile sector (except
		// for the beginning), we will nuke any enemies in this sector first.
		if (gWorldSector != start)
		{
			EliminateAllEnemies(g_merc_arrive_sector);
		}
	}

	// This will update ANY soldiers currently schedules to arrive too
	CheckForValidArrivalSector( );

	// stop time compression until player restarts it
	StopTimeCompression();

	MERCPROFILESTRUCT& p = GetProfile(s.ubProfile);

	// add the guy to a squad
	AddCharacterToAnySquad(&s);

	// ATE: Make sure we use global.....
	if (s.fUseLandingZoneForArrival)
	{
		s.sSector = g_merc_arrive_sector;
	}

	// Add merc to sector ( if it's the current one )
	if (gWorldSector == s.sSector)
	{
		// OK, If this sector is currently loaded, and guy does not have CHOPPER insertion code....
		// ( which means we are at beginning of game if so )
		// Setup chopper....
		if (s.ubStrategicInsertionCode != INSERTION_CODE_CHOPPER &&
				s.sSector.AsByte() == gamepolicy(start_sector))
		{
			gfTacticalDoHeliRun = true;

			// OK, If we are in mapscreen, get out...
			if ( guiCurrentScreen == MAP_SCREEN )
			{
				// ATE: Make sure the current one is selected!
				ChangeSelectedMapSector(gWorldSector);

				RequestTriggerExitFromMapscreen( MAP_EXIT_TO_TACTICAL );
			}

			s.ubStrategicInsertionCode = INSERTION_CODE_CHOPPER;
		}

		UpdateMercInSector(s, s.sSector);
	}
	else
	{
		// OK, otherwise, set them in north area, so once we load again, they are here.
		s.ubStrategicInsertionCode = INSERTION_CODE_NORTH;
	}


	if (s.ubStrategicInsertionCode != INSERTION_CODE_CHOPPER)
	{
		ScreenMsg(FONT_MCOLOR_WHITE, MSG_INTERFACE, st_format_printf(TacticalStr[MERC_HAS_ARRIVED_STR], s.name));

		// ATE: He's going to say something, now that they've arrived...
		if (!gTacticalStatus.bMercArrivingQuoteBeingUsed && !gfFirstHeliRun)
		{
			gTacticalStatus.bMercArrivingQuoteBeingUsed = true;

			TacticalCharacterDialogue(&s, QUOTE_MERC_REACHED_DESTINATION);

			class DialogueEventUnsetArrivesFlag : public DialogueEvent
			{
				public:
					bool Execute()
					{
						gTacticalStatus.bMercArrivingQuoteBeingUsed = false;
						return false;
					}
			};

			DialogueEvent::Add(new DialogueEventUnsetArrivesFlag());
		}
	}

	//record how long the merc will be gone for
	p.bMercStatus = (UINT8)s.iTotalContractLength;

	// remember when excatly he ARRIVED in Arulco, in case he gets fired early
	s.uiTimeOfLastContractUpdate = GetWorldTotalMin();

	//set when the merc's contract is finished
	s.iEndofContractTime = GetMidnightOfFutureDayInMinutes(s.iTotalContractLength) + GetHourWhenContractDone(&s) * 60;

	// Do initial check for bad items
	if (s.bTeam == OUR_TEAM)
	{
		//ATE: Try to see if our equipment sucks!
		if (SoldierHasWorseEquipmentThanUsedTo(&s))
		{
			// Randomly anytime between 9:00, and 10:00
			uiTimeOfPost =  540 + Random( 660 );

			if ( GetWorldMinutesInDay() < uiTimeOfPost )
			{
				AddSameDayStrategicEvent(EVENT_MERC_COMPLAIN_EQUIPMENT, uiTimeOfPost, s.ubProfile);
			}
		}
	}

	HandleMercArrivesQuotes(s);

	fTeamPanelDirty = true;

	// if the currently selected sector has no one in it, select this one instead
	if (!CanGoToTacticalInSector(sSelMap))
	{
		ChangeSelectedMapSector(s.sSector);
	}
}


bool IsMercHireable(MERCPROFILESTRUCT const& p)
{
	// If the merc has an .edt file, is not away on assignment, and isn't already
	// hired (but not arrived yet), he is not dead and he isn't returning home
	return
		p.bMercStatus <= 0                              &&
		p.bMercStatus != MERC_HAS_NO_TEXT_FILE          &&
		p.bMercStatus != MERC_HIRED_BUT_NOT_ARRIVED_YET &&
		p.bMercStatus != MERC_IS_DEAD                   &&
		p.bMercStatus != MERC_RETURNING_HOME            &&
		p.bMercStatus != MERC_WORKING_ELSEWHERE         &&
		p.bMercStatus != MERC_FIRED_AS_A_POW            &&
		p.uiDayBecomesAvailable == 0;
}


bool IsMercDead(MERCPROFILESTRUCT const& p)
{
	return p.bMercStatus == MERC_IS_DEAD;
}


void HandleMercArrivesQuotes(SOLDIERTYPE& s)
{
	// If we are approaching with helicopter, don't say any ( yet )
	if (s.ubStrategicInsertionCode == INSERTION_CODE_CHOPPER) return;

	// Player-generated characters issue a comment about arriving in Omerta.
	if (s.ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER &&
		gubQuest[QUEST_DELIVER_LETTER] == QUESTINPROGRESS)
	{
		TacticalCharacterDialogue(&s, QUOTE_PC_DROPPED_OMERTA);
	}

	// Check to see if anyone hates this merc and will now complain
	FOR_EACH_IN_TEAM(other, OUR_TEAM)
	{
		if (other->ubWhatKindOfMercAmI != MERC_TYPE__AIM_MERC) continue;

		// hates the merc who has arrived and is going to gripe about it!
		switch (WhichHated(other->ubProfile, s.ubProfile))
		{
			case HATED_SLOT1:  TacticalCharacterDialogue(other, QUOTE_HATED_1_ARRIVES); break;
			case HATED_SLOT2:  TacticalCharacterDialogue(other, QUOTE_HATED_2_ARRIVES); break;
			default: break;
		}
	}
}

UINT32 GetMercArrivalTimeOfDay( )
{
	UINT32 uiCurrHour;
	UINT32 uiMinHour;

	// Pick a time...

	// First get the current time of day.....
	uiCurrHour = GetWorldHour( );

	// Subtract the min time for any arrival....
	uiMinHour = uiCurrHour + MIN_FLIGHT_PREP_TIME;

	// OK, first check if we need to advance a whole day's time...
	// See if we have missed the last flight for the day...
	if ( ( uiCurrHour ) > 13  ) // ( > 1:00 pm - too bad )
	{
		// 7:30 flight....
		return( GetMidnightOfFutureDayInMinutes( 1 ) + MERC_ARRIVE_TIME_SLOT_1 );
	}

	// Well, now we can handle flights all in one day....
	// Find next possible flight
	if ( uiMinHour <= 7 )
	{
		return( GetWorldDayInMinutes() + MERC_ARRIVE_TIME_SLOT_1 ); // 7:30 am
	}
	else if ( uiMinHour <= 13 )
	{
		return( GetWorldDayInMinutes() + MERC_ARRIVE_TIME_SLOT_2 ); // 1:30 pm
	}
	else
	{
		return( GetWorldDayInMinutes() + MERC_ARRIVE_TIME_SLOT_3 ); // 7:30 pm
	}
}


void UpdateAnyInTransitMercsWithGlobalArrivalSector( )
{
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bAssignment == IN_TRANSIT && s->fUseLandingZoneForArrival)
		{
			s->sSector = g_merc_arrive_sector;
		}
	}
}


// Return the line of the length from origin to dest, truncated to the nearest integer
static INT16 StrategicPythSpacesAway(INT16 sOrigin, INT16 sDest)
{
	SGPSector delta = SGPSector::FromStrategicIndex(sOrigin) - SGPSector::FromStrategicIndex(sDest);

	return static_cast<INT16>(std::hypot(delta.x, delta.y));
}


// ATE: This function will check if the current arrival sector
// is valid
// if there are enemies present, it's invalid
// if so, search around for nearest non-occupied sector.
static void CheckForValidArrivalSector(void)
{
	INT16   sTop, sBottom;
	INT16   sLeft, sRight;
	INT16   cnt1, cnt2;
	UINT8   ubRadius = 4;
	INT32   leftmost;
	INT16   sSectorGridNo, sSectorGridNo2;
	INT32   uiRange, uiLowestRange = 999999;
	BOOLEAN fFound = false;
	ST::string sString;
	ST::string zShortTownIDString1;

	sSectorGridNo = g_merc_arrive_sector.AsStrategicIndex();

	// Check if valid...
	if ( !StrategicMap[ sSectorGridNo ].fEnemyControlled )
	{
		return;
	}

	zShortTownIDString1 = g_merc_arrive_sector.AsShortString();

	// If here - we need to do a search!
	sTop    = ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	SGPSector sGood; // XXX HACK000E
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSectorGridNo + ( MAP_WORLD_X * cnt1 ) )/ MAP_WORLD_X ) * MAP_WORLD_X;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sSectorGridNo2 = sSectorGridNo + ( MAP_WORLD_X * cnt1 ) + cnt2;

			if( sSectorGridNo2 >=1 && sSectorGridNo2 < ( ( MAP_WORLD_X - 1 ) * ( MAP_WORLD_X - 1 ) ) && sSectorGridNo2 >= leftmost && sSectorGridNo2 < ( leftmost + MAP_WORLD_X ) )
			{
				if ( !StrategicMap[ sSectorGridNo2 ].fEnemyControlled && !StrategicMap[ sSectorGridNo2 ].fEnemyAirControlled )
				{
					uiRange = StrategicPythSpacesAway( sSectorGridNo2, sSectorGridNo );

					if ( uiRange < uiLowestRange )
					{
						sGood.y = cnt1;
						sGood.x = cnt2;
						uiLowestRange = uiRange;
						fFound = true;
					}
				}
			}
		}
	}

	if ( fFound )
	{
		g_merc_arrive_sector += sGood;
		UpdateAnyInTransitMercsWithGlobalArrivalSector( );

		sString = st_format_printf(str_arrival_rerouted, g_merc_arrive_sector.AsShortString(), zShortTownIDString1);

		DoScreenIndependantMessageBox(  sString, MSG_BOX_FLAG_OK, NULL );

	}
}
