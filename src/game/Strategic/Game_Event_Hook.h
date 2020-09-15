#ifndef __GAME_EVENT_HOOK_H
#define __GAME_EVENT_HOOK_H

#include "Types.h"

enum StrategicEventKind
{
	EVENT_CHANGELIGHTVAL                                 =  1,
	EVENT_CHECKFORQUESTS                                 =  4,
	EVENT_AMBIENT                                        =  5,
	EVENT_AIM_RESET_MERC_ANNOYANCE                       =  6,
	EVENT_BOBBYRAY_PURCHASE                              =  7,
	EVENT_DAILY_UPDATE_BOBBY_RAY_INVENTORY               =  8,
	EVENT_UPDATE_BOBBY_RAY_INVENTORY                     =  9,
	EVENT_DAILY_UPDATE_OF_MERC_SITE                      = 10,
	EVENT_DAY3_ADD_EMAIL_FROM_SPECK                      = 11,
	EVENT_DELAYED_HIRING_OF_MERC                         = 12,
	EVENT_HANDLE_INSURED_MERCS                           = 13,
	EVENT_PAY_LIFE_INSURANCE_FOR_DEAD_MERC               = 14,
	EVENT_MERC_DAILY_UPDATE                              = 15,
	EVENT_GROUP_ARRIVAL                                  = 18,
	EVENT_DAY2_ADD_EMAIL_FROM_IMP                        = 19,
	EVENT_MERC_COMPLAIN_EQUIPMENT                        = 20,
	EVENT_HOURLY_UPDATE                                  = 21,
	EVENT_HANDLE_MINE_INCOME                             = 22,
	EVENT_SETUP_MINE_INCOME                              = 23,
	EVENT_SET_BY_NPC_SYSTEM                              = 27,
	EVENT_SECOND_AIRPORT_ATTENDANT_ARRIVED               = 28,
	EVENT_HELICOPTER_HOVER_TOO_LONG                      = 29,
	EVENT_HELICOPTER_HOVER_WAY_TOO_LONG                  = 30,
	EVENT_MERC_LEAVE_EQUIP_IN_OMERTA                     = 32,
	EVENT_MERC_LEAVE_EQUIP_IN_DRASSEN                    = 33,
	EVENT_DAILY_EARLY_MORNING_EVENTS                     = 34,
	EVENT_GROUP_ABOUT_TO_ARRIVE                          = 35,
	EVENT_PROCESS_TACTICAL_SCHEDULE                      = 36,
	EVENT_BEGINRAINSTORM                                 = 37,
	EVENT_ENDRAINSTORM                                   = 38,
	EVENT_MAKE_CIV_GROUP_HOSTILE_ON_NEXT_SECTOR_ENTRANCE = 41,
	EVENT_BEGIN_AIR_RAID                                 = 42, /** unused **/
	EVENT_MEANWHILE                                      = 44,
	EVENT_CREATURE_SPREAD                                = 46,
	EVENT_CREATURE_NIGHT_PLANNING                        = 48,
	EVENT_CREATURE_ATTACK                                = 49,
	EVENT_EVALUATE_QUEEN_SITUATION                       = 50,
	EVENT_CHECK_ENEMY_CONTROLLED_SECTOR                  = 51,
	EVENT_TURN_ON_NIGHT_LIGHTS                           = 52,
	EVENT_TURN_OFF_NIGHT_LIGHTS                          = 53,
	EVENT_TURN_ON_PRIME_LIGHTS                           = 54,
	EVENT_TURN_OFF_PRIME_LIGHTS                          = 55,
	EVENT_MERC_ABOUT_TO_LEAVE                            = 56,
	EVENT_ENRICO_MAIL                                    = 58,
	EVENT_INSURANCE_INVESTIGATION_STARTED                = 59,
	EVENT_INSURANCE_INVESTIGATION_OVER                   = 60,
	EVENT_MINUTE_UPDATE                                  = 61,
	EVENT_TEMPERATURE_UPDATE                             = 62,
	EVENT_KEITH_GOING_OUT_OF_BUSINESS                    = 63,
	EVENT_MERC_SITE_BACK_ONLINE                          = 64,
	EVENT_CHECK_IF_MINE_CLEARED                          = 66,
	EVENT_REMOVE_ASSASSIN                                = 67,
	EVENT_BANDAGE_BLEEDING_MERCS                         = 68,
	EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE                = 72,
	EVENT_RPC_WHINE_ABOUT_PAY                            = 73,
	EVENT_HAVENT_MADE_IMP_CHARACTER_EMAIL                = 74,
	EVENT_RAINSTORM                                      = 75,
	EVENT_QUARTER_HOUR_UPDATE                            = 76,
	EVENT_MERC_MERC_WENT_UP_LEVEL_EMAIL_DELAY            = 77,
	EVENT_MERC_SITE_NEW_MERC_AVAILABLE                   = 78,

	NUMBER_OF_EVENT_TYPES
};

// This value is added to the param value for NPC-system-created events which are based on an
// action rather than a fact:
#define NPC_SYSTEM_EVENT_ACTION_PARAM_BONUS 10000

//One Time Events
//These events are scheduled to happen, and when their time is up, they are processed
//once, and deleted.  The first prototype requires the actual world time in minutes
//included the current day, and the additional ones are wrappers which calculate the
//actual world time for you.
BOOLEAN AddStrategicEvent(StrategicEventKind, UINT32 uiMinStamp, UINT32 uiParam);
BOOLEAN AddSameDayStrategicEvent(StrategicEventKind, UINT32 uiMinStamp, UINT32 uiParam);
BOOLEAN AddFutureDayStrategicEvent(StrategicEventKind, UINT32 uiMinStamp, UINT32 uiParam, UINT32 uiNumDaysFromPresent);
//Same but uses seconds instead of minutes.
BOOLEAN AddStrategicEventUsingSeconds(StrategicEventKind, UINT32 uiSecondStamp, UINT32 uiParam);

//Ranged Events
//In certain cases, you may wish to create an event that has a start time and an end
//time.  If this is used, then the execution of the event will happen once at the
//beginning of the event and once at the expiration time of the event.  A flag,
//pEvent->fEndEvent is set for the second call.  It is up to you process it
//separately.  An example use of ranged events would be for ambient sounds.  The
//start time would be called to define the sounds for use within the game.  The end
//time would be used to remove the sound from the system.
BOOLEAN AddSameDayRangedStrategicEvent(StrategicEventKind, UINT32 uiStartMin, UINT32 uiLengthMin, UINT32 uiParam);

//EveryDay Events
//A second special case event, this event will get processed automatically at the
//same time every day.  Time of day lighting is a perfect example.  Other events
//such as checking for quests, can also be automated using this system.  NOTE:
//Only specify the time of day in minutes (don't add the actual day to this value)
BOOLEAN AddEveryDayStrategicEvent(StrategicEventKind, UINT32 uiStartMin, UINT32 uiParam);

//Periodic Events
//Event will get processed automatically once every X minutes.  The period is added to the current time for
//the time stamp.
BOOLEAN AddPeriodStrategicEvent(StrategicEventKind, UINT32 uiOnceEveryXMinutes, UINT32 uiParam);
//Hybrids that aren't based from the current time.  They are offsetted from the current time first.
BOOLEAN AddPeriodStrategicEventWithOffset(StrategicEventKind, UINT32 uiOnceEveryXMinutes, UINT32 uiOffsetFromCurrent, UINT32 uiParam);

/* Search for and remove the first event matching the supplied information.
	* There may very well be a need for more specific event removal, so let me know
	* (Kris), of any support needs. */
void DeleteStrategicEvent(StrategicEventKind, UINT32 param);
void DeleteAllStrategicEventsOfType(StrategicEventKind);
void DeleteAllStrategicEvents(void);

#endif
