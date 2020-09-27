Items = {
    SUNGOGGLE = 212,
    CANTEEN   = 214,
}

Visibility = {
	BURIED               =  -3,
	HIDDEN_IN_OBJECT     =  -2,
	INVISIBLE            =  -1,
	VISIBLE              =   1,
}

SectorFlags = {
    SF_ALREADY_VISITED = 32,
    SF_USE_ALTERNATE_MAP = 64,
    SF_HAS_ENTERED_TACTICAL = 512,
}

CivGroups = {
    NON_CIV_GROUP = 0,
    REBEL_CIV_GROUP = 1,
    KINGPIN_CIV_GROUP = 2,
    SANMONA_ARMS_GROUP = 3,
    ALMA_MILITARY_CIV_GROUP = 7,
    DOCTORS_CIV_GROUP = 8,
    HICKS_CIV_GROUP = 10,
    WARDEN_CIV_GROUP = 11,
}

Teams = {
    OUR_TEAM = 0,
    ENEMY_TEAM = 1,
    CREATURE_TEAM = 2,
    MILITIA_TEAM = 3,
    CIV_TEAM = 4,
}

SoldierClasses = {
	SOLDIER_CLASS_NONE = 0,
	SOLDIER_CLASS_ADMINISTRATOR = 1,
	SOLDIER_CLASS_ELITE = 2,
	SOLDIER_CLASS_ARMY = 3,
	SOLDIER_CLASS_GREEN_MILITIA = 4,
	SOLDIER_CLASS_REG_MILITIA = 5,
	SOLDIER_CLASS_ELITE_MILITIA = 6,
	SOLDIER_CLASS_CREATURE = 7,
	SOLDIER_CLASS_MINER = 8,
}

MilitiaRanks = {
    GREEN_MILITIA = 0,
    REGULAR_MILITIA = 1,
    ELITE_MILITIA = 2,
}

SoldierBodyTypes = {
    BODY_RANDOM = -1,
    REGMALE = 0,
    BIGMALE = 1,
    STOCKYMALE = 2,
    REGFEMALE = 3,
    ADULTFEMALEMONSTER = 4,
    AM_MONSTER = 5,
    YAF_MONSTER = 6,
    YAM_MONSTER = 7,
    LARVAE_MONSTER = 8,
    INFANT_MONSTER = 9,
    QUEENMONSTER = 10,
    FATCIV = 11,
    MANCIV = 12,
    MINICIV = 13,
    DRESSCIV = 14,
    HATKIDCIV = 15,
    KIDCIV = 16,
    CRIPPLECIV = 17,
    COW = 18,
    CROW = 19,
    BLOODCAT = 20,
    ROBOTNOWEAPON = 21,
    HUMVEE = 22,
    TANK_NW = 23,
    TANK_NE = 24,
    ELDORADO = 25,
    ICECREAMTRUCK = 26,
    JEEP = 27,
}

MercProfiles = {
    MIGUEL = 57,
    CARLOS = 58,
    IRA = 59,
    DIMITRI = 60,

    QUEEN = 75,

    SKYRIDER = 97,
    PABLO = 98,
    FATIMA = 101,

    ELLIOT = 135,

    NO_PROFILE = 200,
}

SkillTraits =  {
    NO_SKILLTRAIT = 0,
    LOCKPICKING = 1,
    HANDTOHAND = 2,
    ELECTRONICS = 3,
    NIGHTOPS = 4,
    THROWING = 5,
    TEACHING = 6,
    HEAVY_WEAPS = 7,
    AUTO_WEAPS = 8,
    STEALTHY = 9,
    AMBIDEXT = 10,
    THIEF = 11,
    MARTIALARTS = 12,
    KNIFING = 13,
    ONROOF = 14,
    CAMOUFLAGED = 15,
}

StrategicEventFrequencies = {
    ONETIME_EVENT = 0,
    RANGED_EVENT = 1,
    ENDRANGED_EVENT = 2,
    EVERYDAY_EVENT = 3,
    PERIODIC_EVENT = 4,
    QUEUED_EVENT = 5,
};

StrategicEventKinds = {
    EVENT_CHECKFORQUESTS                                 =  4,
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
    EVENT_MAKE_CIV_GROUP_HOSTILE_ON_NEXT_SECTOR_ENTRANCE = 41,
    EVENT_MEANWHILE                                      = 44,
    EVENT_CREATURE_SPREAD                                = 46,
    EVENT_CREATURE_NIGHT_PLANNING                        = 48,
    EVENT_CREATURE_ATTACK                                = 49,
    EVENT_EVALUATE_QUEEN_SITUATION                       = 50,
    EVENT_CHECK_ENEMY_CONTROLLED_SECTOR                  = 51,
    EVENT_MERC_ABOUT_TO_LEAVE                            = 56,
    EVENT_ENRICO_MAIL                                    = 58,
    EVENT_INSURANCE_INVESTIGATION_STARTED                = 59,
    EVENT_INSURANCE_INVESTIGATION_OVER                   = 60,
    EVENT_MINUTE_UPDATE                                  = 61,
    EVENT_KEITH_GOING_OUT_OF_BUSINESS                    = 63,
    EVENT_MERC_SITE_BACK_ONLINE                          = 64,
    EVENT_CHECK_IF_MINE_CLEARED                          = 66,
    EVENT_REMOVE_ASSASSIN                                = 67,
    EVENT_BANDAGE_BLEEDING_MERCS                         = 68,
    EVENT_BEGIN_CONTRACT_RENEWAL_SEQUENCE                = 72,
    EVENT_RPC_WHINE_ABOUT_PAY                            = 73,
    EVENT_HAVENT_MADE_IMP_CHARACTER_EMAIL                = 74,
    EVENT_QUARTER_HOUR_UPDATE                            = 76,
    EVENT_MERC_MERC_WENT_UP_LEVEL_EMAIL_DELAY            = 77,
    EVENT_MERC_SITE_NEW_MERC_AVAILABLE                   = 78,
}

DifficultyLevels = {
    DIF_LEVEL_EASY = 1,
    DIF_LEVEL_MEDIUM = 2,
    DIF_LEVEL_HARD = 3,
}

GameSaveMmodes = {
    DIF_CAN_SAVE = 0,
    DIF_IRON_MAN = 1,
    DIF_DEAD_IS_DEAD = 2,
}