TRUE = 1
FALSE = 0

TacticalStatusFlags = {
    INCOMBAT = 0x000040000,
    ENGAGED_IN_CONV = 0x000800000,
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

SoldierAnimState = {
    STAND_FALLFORWARD_STOP = -1, -- TODO: to be added
}

MercTypes = {
    PLAYER_CHARACTER = 0,
    AIM_MERC = 1,
    MERC = 2,
    NPC = 3,
    EPC = 4,
    NPC_WITH_UNEXTENDABLE_CONTRACT = 5,
    VEHICLE = 6,
}

MercStatuses = {
    MERC_OK = 0,
    MERC_IS_DEAD = -5,
}

StrategicEventFrequencies = {
    ONETIME_EVENT = 0,
    RANGED_EVENT = 1,
    ENDRANGED_EVENT = 2,
    EVERYDAY_EVENT = 3,
    PERIODIC_EVENT = 4,
    QUEUED_EVENT = 5,
};

EventTypes = {
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

QuestStatuses = {
    QUESTNOTSTARTED = 0,
    QUESTINPROGRESS = 1,
    QUESTDONE = 2,
}

Bookmarks = {
    AIM = 0,
    BOBBYR = 1,
    IMP = 2,
    MERC = 3,
    FUNERAL = 4,
    FLORIST = 5,
    INSURANCE = 6,
    CANCEL_STRING = 7,
}

TransactionTypes = {
    ACCRUED_INTEREST = 0,
    ANONYMOUS_DEPOSIT = 1,
    TRANSACTION_FEE = 2,
    HIRED_MERC = 3,
    BOBBYR_PURCHASE = 4,
    PAY_SPECK_FOR_MERC = 5,
    MEDICAL_DEPOSIT = 6,
    IMP_PROFILE = 7,
    PURCHASED_INSURANCE = 8,
    REDUCED_INSURANCE = 9,
    EXTENDED_INSURANCE = 10,
    CANCELLED_INSURANCE = 11,
    INSURANCE_PAYOUT = 12,
    EXTENDED_CONTRACT_BY_1_DAY = 13,
    EXTENDED_CONTRACT_BY_1_WEEK = 14,
    EXTENDED_CONTRACT_BY_2_WEEKS = 15,
    DEPOSIT_FROM_GOLD_MINE = 16,
    DEPOSIT_FROM_SILVER_MINE = 17,
    PURCHASED_FLOWERS = 18,
    FULL_MEDICAL_REFUND = 19,
    PARTIAL_MEDICAL_REFUND = 20,
    NO_MEDICAL_REFUND = 21,
    PAYMENT_TO_NPC = 22,
    TRANSFER_FUNDS_TO_MERC = 23,
    TRANSFER_FUNDS_FROM_MERC = 24,
    TRAIN_TOWN_MILITIA = 25,
    PURCHASED_ITEM_FROM_DEALER = 26,
    MERC_DEPOSITED_MONEY_TO_PLAYER_ACCOUNT = 27,
}

HistoryTypes = {
    ENTERED_HISTORY_MODE = 0,
    HIRED_MERC_FROM_AIM = 1,
    HIRED_MERC_FROM_MERC = 2,
    MERC_KILLED = 3,
    SETTLED_ACCOUNTS_AT_MERC = 4,
    MERC_FIRED = 13,
    MERC_QUIT = 14,
    QUEST_STARTED = 15,
    QUEST_FINISHED = 16,
}

BattleSound = {
    BATTLE_SOUND_OK1 = 0,
    BATTLE_SOUND_OK2 = 1,
    BATTLE_SOUND_COOL1 = 2,
    BATTLE_SOUND_CURSE1 = 3,
    BATTLE_SOUND_HIT1 = 4,
    BATTLE_SOUND_HIT2 = 5,
    BATTLE_SOUND_LAUGH1 = 6,
    BATTLE_SOUND_ATTN1 = 7,
    BATTLE_SOUND_DIE1 = 8,
    BATTLE_SOUND_HUMM = 9,
    BATTLE_SOUND_NOTHING = 10,
    BATTLE_SOUND_GOTIT = 11,
    BATTLE_SOUND_LOWMARALE_OK1 = 12,
    BATTLE_SOUND_LOWMARALE_OK2 = 13,
    BATTLE_SOUND_LOWMARALE_ATTN1 = 14,
    BATTLE_SOUND_LOCKED = 15,
    BATTLE_SOUND_ENEMY = 16,
    NUM_MERC_BATTLE_SOUNDS = 17,
}
