#include "Font_Control.h"
#include "Soldier_Control.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "FileMan.h"
#include "Weapons.h"
#include "Message.h"
#include "Structure.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Rotting_Corpses.h"
#include "Points.h"
#include "Debug_Control.h"


// Defines for Anim inst reading, taken from orig Jagged
#define	ANIMFILENAME					"BINARYDATA/ja2bin.dat"

#define EMPTY_INDEX						999


struct ANIMSUBTYPE
{
	UINT16	usAnimState;
	UINT16	usAnimationSurfaces[ 4 ];
};


//Block for anim file
UINT16	gusAnimInst[ MAX_ANIMATIONS ][ MAX_FRAMES_PER_ANIM ];

// OK, this array contains definitions for random animations based on bodytype, total # allowed, and what is in their hand....
RANDOM_ANI_DEF	gRandomAnimDefs[ TOTALBODYTYPES ][ MAX_RANDOM_ANIMS_PER_BODYTYPE ];


#define S ANIM_STAND
#define C ANIM_CROUCH
#define P ANIM_PRONE
#define M(name, speed, move, flags, height, end_height) { name, move, flags, speed, height, end_height }

const ANIMCONTROLTYPE gAnimControl[NUMANIMATIONSTATES] =
{
	// NAME                             SPEED MOVE  FLAGS                                                                                                                                                    HEIGHT
	M("WALKING",                           0, 1.6f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_VARIABLE_EFFORT,                                                                 S, S), //WALKING
	M("STANDING",                          0, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_VARIABLE_EFFORT   | ANIM_BREATH,                              S, S), //Breathing
	M("KNEEL DOWN",                       90, 0.0f, ANIM_STATIONARY | ANIM_TURNING  | ANIM_NOMOVE_MARKER | ANIM_STANCECHANGEANIM  | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                  S, C), //KNEEL DOWN
	M("CROUCHED",                          0, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_RAISE_WEAPON | ANIM_NO_EFFORT   | ANIM_BREATH,                                                     C, C), //CROUCHING
	M("KNEEL UP",                         90, 0.0f, ANIM_STATIONARY | ANIM_TURNING  | ANIM_NOMOVE_MARKER | ANIM_STANCECHANGEANIM  | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                  C, S), //KNEEL UP
	M("SWAT",                              0, 2.2f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART  | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT,                                                                   C, C), //SWAT
	M("RUN",                               0, 2.6f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_MODERATE_EFFORT,                                                                 S, S), //RUN
	M("PRONE DOWN",                      100, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_STANCECHANGEANIM | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                  C, P), //PRONE DOWN
	M("CRAWL",                             0, 0.8f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_MODERATE_EFFORT,                                                                 P, P), //CRAWL
	M("PRONE UP",                        100, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_STANCECHANGEANIM | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                  P, C), //PRONE UP
	M("PRONE",                             0, 0.0f, ANIM_STATIONARY | ANIM_FASTTURN | ANIM_RAISE_WEAPON | ANIM_NO_EFFORT   | ANIM_BREATH,                                                                    P, P), //PRONE BREATHING
	M("READY AIM (R) STAND",               0, 0.0f, ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_RAISE_WEAPON | ANIM_MIN_EFFORT,                      S, S), // READY_RIFLE_STAND
	M("AIM (R) STAND",                   250, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FIREREADY | ANIM_FASTTURN | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), // AIM_RIFLE_STAND
	M("SHOOT (R) STAND",                 100, 0.0f, ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE | ANIM_RT_NONINTERRUPT,                              S, S), // SHOOT_RIFLE_STAND
	M("END RIFLE STAND",                  80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_NOCHANGE_PENDINGCOUNT | ANIM_MIN_EFFORT,                                                       S, S), // END_RIFLE_STAND
	M("CROUCH TO SWAT",                   40, 1.3f, ANIM_MOVING | ANIM_TURNING | ANIM_LIGHT_EFFORT,                                                                                                          C, C), // FROM CROUCH TO SWAT
	M("SWAT TO CROUCH",                   40, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT,                                                                                                      C, C), // FROM SWAT TO CROUCH
	M("FLYBACK HIT",                      80, 1.6f, ANIM_SPECIALMOVE | ANIM_HITSTART | ANIM_HITFINISH | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE, S, P), // FLYBACK HIT ANIMATION
	M("READY (R) PRONE",                 150, 0.0f, ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING| ANIM_RAISE_WEAPON | ANIM_MIN_EFFORT,                                       P, P), // READY_RIFLE_PRONE
	M("AIM (R) PRONE",                   250, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FIREREADY | ANIM_FASTTURN| ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT,                                                  P, P), // AIM_RIFLE_PRONE
	M("SHOOT (R) PRONE",                 100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE | ANIM_RT_NONINTERRUPT,                                               P, P), // SHOOT_RIFLE_PRONE
	M("END (R) AIM PRONE",               150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_NOCHANGE_PENDINGCOUNT | ANIM_MIN_EFFORT,                                                       P, P), // END_RIFLE_PRONE
	M("DEATH TWITCH ONE",                100, 0.0f, ANIM_STATIONARY | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                            P, P), // DEATH TWITCH ONE
	M("GENERIC HIT STAND",               130, 0.0f, ANIM_STATIONARY | ANIM_HITSTART | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,                     S, S), // GENERIC_HIT STANDING
	M("FLYBACK HIT w/B",                  80, 1.6f, ANIM_SPECIALMOVE | ANIM_HITSTART | ANIM_HITFINISH | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE, P, P), // FLYBACK HIT ANIMATION W/ BLOOD
	M("FLYBACK HIT DEATH",               120, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                        P, P), // FLYBACK HIT DEATH
	M("READY (R) CROUCH",                150, 0.0f, ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_MIN_EFFORT,                                      C, C), // READY_RIFLE_CROUCH
	M("AIM (R) CROUCH",                  250, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FIREREADY | ANIM_FASTTURN | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), // AIM_RIFLE_CROUCH
	M("SHOOT (R) CROUCH",                100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE | ANIM_RT_NONINTERRUPT,                                               C, C), // SHOOT_RIFLE_CROUCH
	M("END (R) CROUCH",                  150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_NOCHANGE_PENDINGCOUNT | ANIM_MIN_EFFORT,                                                       C, C), // END_RIFLE_CROUCH
	M("FALLBACK HIT STAND",              130, 1.6f, ANIM_SPECIALMOVE | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,                                  S, P), // FALLBACK HIT STANDING
	M("ROOLOVER",                        100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // ROLLOVER
	M("CLIMBROOF",                        30, 0.0f, ANIM_NONINTERRUPT | ANIM_NORESTART | ANIM_STATIONARY | ANIM_NOSHOW_MARKER | ANIM_MODERATE_EFFORT | ANIM_LOWER_WEAPON,                                    S, C), // CLIMBROOF
	M("FALL",                             60, 0.0f, ANIM_NONINTERRUPT | ANIM_STATIONARY | ANIM_HITFINISH | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,              S, P), // FALL
	M("GETUP FROM ROLLOVER",             100, 0.0f, ANIM_STATIONARY | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                                 P, S), //GETUP FROM ROLLOVER
	M("CLIMBDOWNROOF",                    30, 0.0f, ANIM_NONINTERRUPT | ANIM_NORESTART | ANIM_STATIONARY | ANIM_NOSHOW_MARKER | ANIM_MODERATE_EFFORT | ANIM_LOWER_WEAPON,                                    C, S), //CLIMBDOWNROOF
	M("FALL FORWARD ROOF",                60, 0.0f, ANIM_NONINTERRUPT | ANIM_STATIONARY | ANIM_HITFINISH | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,              S, P), // FALL FORWARD
	M("BELLY DEATHHIT NB",               100, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                     P, P), // GENERIC HIT DEATHTWICH NO BLOOD
	M("BELLY DEATHHIT B",                100, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                     P, P), // GENERIC HIT DEATHTWICH BLOOD
	M("FALLBACK DEATHHIT NB",            100, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                     P, P), // FALLBACK HIT DEATHTWICH NO BLOOD
	M("FALLBACK DEATHHIT B",             100, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                     P, P), // FALLBACK HIT DEATHTWICH BLOOD
	M("BELLY HIT DEATH",                 120, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                             P, P), // GENERIC HIT DEATH
	M("FALLBACK HIT DEATH",              120, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                             P, P), // FALLBACK HIT DEATH
	M("GENERIC HIT CROUCH",               80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              C, C), // GENERIC HIT CROUCHED
	M("STAND BURST SHOOT",               100, 0.0f, ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_FIRE,                              S, S), // STAND BURST
	M("STAND BUTST HIT",                  80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,                                         S, S), // STANDING BURST HIT
	M("FALL FROM STAND",                 120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // FALL FORWARD FROM HIT STAND
	M("FALL FROM CROUCH",                120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        C, P), // FALL FORWARD FROM HIT CROUCH
	M("END FALL F CROUCH",               120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH  | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                            P, P), // END FALL FORWARD FROM HIT CROUCH
	M("GENERIC HIT PRONE",                80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              P, P), // GENERIC HIT PRONE
	M("PRONE HIT DEATH",                 120, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                        P, P), // PRONE HIT DEATH
	M("PRONE LAY FROM HIT",              160, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                             P, P), // PRONE LAY FROM HIT
	M("PRONE DEATHHIT NB",               100, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                     P, P), // PRONE HIT DEATHTWICH NO BLOOD
	M("PRONE DEATHHIT B",                100, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                     P, P), // PRONE HIT DEATHTWICH BLOOD
	M("FEMADMON BREATHING",              100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                 S, S), // FEMALE MONSTER BREATHING
	M("FEMADMON WALKING",                 30, 2.5f, ANIM_MOVING | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                  S, S), // FEMALE MONSTER BREATHING
	M("FEMADMON ATTACK",                  70, 2.5f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                              S, S), // FEMALE MONSTER ATTACK
	M("FLYBACK HIT DEATH STOP",          120, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("BELLY HIT DEATH STOP",            120, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("FALLBACK HIT DEATH STOP",         120, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("PRONE HIT DEATH STOP",            120, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("FLYBACK HIT STOP",                 70, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("FALLBACK HIT STOP",               350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("FALLOFF STOP",                    350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("FALLOFF FORWARD HIT STOP",        350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("FALLFORWARD HIT STOP",            650, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("PRONE_LAYFROMHIT STOP",            70, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P),
	M("HOP FENCE",                        50, 0.0f, ANIM_NOCHANGE_PENDINGCOUNT | ANIM_NORESTART | ANIM_STATIONARY | ANIM_NOSHOW_MARKER | ANIM_MODERATE_EFFORT | ANIM_LOWER_WEAPON | ANIM_NONINTERRUPT,       S, C),
	M("FEMMONS HIT",                      80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              S, S), // GENERIC HIT CROUCHED
	M("FEMMONS DIE",                      80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // FALL FORWARD FROM HIT STAND
	M("FEMMONS DIE STOP",                350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // FALL FORWARD FROM HIT STAND
	M("PUNCH BREATH",                    150, 0.0f, ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON,          S, S), //Breathing
	M("PUNCH",                            70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NORESTART,                                                               S, S), // Punch
	M("NOTHING_STAND",                   150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                 S, S), //Breathing
	M("JFK HIT DEATH",                   160, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_HITSTART | ANIM_HITFINISH  | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                           S, P), // JFK HIT DEATH
	M("JFK HIT DEATH STOP",              120, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                             P, P),
	M("JFK DEATHHIT B",                  100, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                     P, P),
	M("STAND BURST SPREAD",              100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT,                                                                                                      S, S), // STAND BURST
	M("FALLOFF DEATH",                   120, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                             P, P), // FALLOFF DEATH
	M("FALLOFF DEATH STOP",              120, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // FALLOFF DEATH STOP
	M("FALLOFF TWITCH BLOOD",            120, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                          P, P), // FALLOFF TWITCHB
	M("FALLOFF TWITCH NOBLOOD",          120, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                          P, P), // FALLOFF TWITCH NB
	M("FALLOFF FORWARD DEATH",           120, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                             P, P), // FALLOFF FORWARD DEATH
	M("FALLOFF FORWARD DEATH STOP",      120, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // FALLOFF FORWARD DEATH STOP
	M("FALLOFF FORWARD TWITCH BLOOD",    120, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                          P, P), // FALLOFF FORWARD TWITCHB
	M("FALLOFF FORWARD TWITCH NOBLOOD",  120, 0.0f, ANIM_STATIONARY | ANIM_HITWHENDOWN | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                          P, P), // FALLOFF TWITCH NB
	M("OPEN DOOR",                        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //OPEN DOOR
	M("OPEN STRUCT",                      70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //OPEN STRUCT
	M("PICKUP ITEM",                     100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //OPEN STRUCT
	M("DROP ITEM",                       100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //DROP ITEM
	M("SLICE",                            70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     S, S), //SLICE
	M("STAB",                             70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     S, S), //STAB
	M("CROUCH STAB",                      70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     C, C), //CROUCH STAB
	M("START AID",                        90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //START GIVING AID
	M("GIVING AID",                      120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     C, C), //GIVING AID
	M("END AID",                          90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_NO_EFFORT   | ANIM_NOCHANGE_PENDINGCOUNT,                     C, C), //END GIVING AID
	M("DODGE ONE",                        40, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                 S, S), //DODGE ONE
	M("FATCIV ASS SCRATCH",              110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                        S, S), //FATCIV_ASSSCRATCH
	M("READY AIM (DW) STAND",            150, 0.0f, ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN  | ANIM_LIGHT_EFFORT,                                       S, S), // READY_RIFLE_STAND
	M("AIM (DW) STAND",                  250, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FIREREADY | ANIM_FASTTURN | ANIM_LIGHT_EFFORT,                                                                     S, S), // AIM_RIFLE_STAND
	M("SHOOT (DW) STAND",                150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                                          S, S), // SHOOT_RIFLE_STAND
	M("END DUAL STAND",                  150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NOCHANGE_PENDINGCOUNT | ANIM_LIGHT_EFFORT,                                                                         S, S), // END_RIFLE_STAND
	M("RAISE RIFLE",                      70, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                           S, S), //RAISE RIFLE
	M("LOWER RIFLE",                      70, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                           S, S), //LOWER RIFLE
	M("BOD BLOW",                        200, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NONINTERRUPT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,                              S, S), //BOD BLOW
	M("THROW ITEM",                       80, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_NONINTERRUPT | ANIM_MIN_EFFORT,                                                                             S, S), //THROW ITEM
	M("LOB ITEM",                         80, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_NONINTERRUPT | ANIM_MIN_EFFORT,                                                                             S, S), //LOB ITEM
	M("MONSTER_STATIONARY",              100, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_BREATH,                                                                                    S, S), //QUEEN MONSTER STATIONARY
	M("CROUCHED BURST SHOOT",            100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                                          C, C), // CROUCHED BURST
	M("PRONE BURST SHOOT",               100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                                          P, P), // PRONE BURST
	M("NOT USED 1",                       20, 0.9f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART  | ANIM_NO_EFFORT,                                                                                          S, S), //180
	M("BIG GUY FLEX",                    150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NO_EFFORT,                                                                                         S, S), // BIG GUY FLEX
	M("BIG GUY STRECH",                  110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NO_EFFORT,                                                                                         S, S), // BIG GUY STRECH
	M("BIG GUY SHOE DUST",                90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S), // BIG GUY SOEDUST
	M("BIG GUY HEAD TURN",               180, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S), // BIG GUY HEADTURN
	M("MINI GIRL STOCKING",              120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT,                                                                                                         S, S), // MINI GIRL STOCKING
	M("GIVE ITEM",                       110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //GIVE ITEM
	M("CLIMB MOUNTAIN",                   50, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NOSHOW_MARKER | ANIM_MODERATE_EFFORT,                                             S, C), //CLIMB MOUNTIAIN
	M("COW EATING",                      200, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT,                                                                                                         S, S),
	M("COW HIT",                         150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT,                                                                     S, S), // COW HIT
	M("COW DIE",                         150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT,                                               S, P), // COW DIE
	M("COW DIE STOP",                    350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT,                                                                                                         P, P), // COW DIE STOP
	M("WATER HIT",                        80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              S, S), // WATER HIT
	M("WATER DIE",                       160, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // WATER DIE
	M("WATER DIE STOP",                  340, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // WATER DIE STOP
	M("CROW WALK",                        80, 1.6f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                                        S, S),
	M("CROW TAKEOFF",                     80, 1.6f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                           S, S),
	M("CROW LAND",                        80, 1.6f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                           S, S),
	M("CROW FLY",                         20, 0.5f, ANIM_SPECIALMOVE |  ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                    S, S),
	M("CROW EAT",                         80, 0.0f, ANIM_STATIONARY  | ANIM_TURNING  | ANIM_NORESTART | ANIM_MIN_EFFORT,                                                                                     S, S),
	M("HELIDROP",                         60, 0.0f, ANIM_STATIONARY  | ANIM_TURNING  | ANIM_NORESTART | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT,                                                                 S, C),
	M("FEM CLEAN",                       110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("FEM KICKSN",                      110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("FEM LOOK",                        180, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("FEM WIPE",                        110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("REG SQUISH",                      110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("REG PULL",                        110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("REG SPIT",                        130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("KID YOYO",                        140, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT,                                                                                                         S, S),
	M("KID ARMPIT",                       70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT,                                                                                                         S, S),
	M("ADULT MONSTER CLOSE ATTACK",       70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                          S, S),
	M("ADULT MONSTER SPIT ATTACK",        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                          S, S),
	M("ADULT MONSTER BEGIN EATING",       70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT,                                                                    S, S),
	M("ADULT MONSTER EATING",             70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                                     S, S),
	M("ADULT MONSTER END EATING",         70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT,                                                                    S, S),
	M("BLOODCAT RUN",                     50, 5.4f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_MODERATE_EFFORT,                                                                                     S, S), //RUN
	M("BLOODCAT BEGIN READY",             70, 0.0f, ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                           S, S), //
	M("BLOODCAT READY",                  130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        S, S),
	M("BLOODCAT END READY",               70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        S, S),
	M("BLOODCAT HIT",                    110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT,                                                                     S, S), // CAT HIT
	M("BLOODCAT DIE",                    110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER,                                                                S, P), // CAT DIE
	M("BLOODCAT DIE STOP",               100, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT,                                                                                                         P, P), // CAT DIE STOP
	M("BLOODCAT SWIPE",                   70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_MIN_EFFORT,                                                                   S, S),
	M("NINJA GOTO BREATH",                70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                          S, S),
	M("NINJA BREATH",                      0, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON,                                                                             S, S),
	M("NINJA LOWKICK",                    50, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NORESTART,                                        S, S),
	M("NINJA PUNCH",                      50, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NORESTART,                                        S, S),
	M("NINJA SPIN KICK",                  70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_MODERATE_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NORESTART,                                     S, S),
	M("END OPEN DOOR",                   100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_MIN_EFFORT,                                                   S, S), //END OPEN DOOR
	M("END OPEN LOCKED DOOR",            100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_MIN_EFFORT,                               S, S), //END OPEN LOCKED DOOR
	M("KICK DOOR",                        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART |  ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                S, S), //KICK DOOR
	M("CLOSE DOOR",                      100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_MIN_EFFORT,                                                   S, S), //KICK DOOR
	M("HIT STAND RIFLE",                 150, 0.0f, ANIM_STATIONARY | ANIM_HITSTART | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                              S, S), // HIT STANDING RIFLE
	M("DEEP WATER TREAD",                200, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NOCHANGE_WEAPON | ANIM_MODERATE_EFFORT,                                                                            S, S), // DEEP WATER TREAD
	M("DEEP WATER SWIM",                 160, 1.3f, ANIM_MOVING | ANIM_TURNING  | ANIM_NORESTART | ANIM_NOCHANGE_WEAPON | ANIM_MODERATE_EFFORT,                                                              S, S), // DEEP WATER SWIM
	M("DEEP WATER HIT",                  100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT,                                                                     S, S), // DEEP WATER HIT
	M("DEEP WATER DIE",                  210, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // DEEP WATER DIE
	M("DEEP WATER DIE STOPPING",         340, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                              P, P), // DEEP WATER DIE STOPPING
	M("DEEP WATER DIE STOP",             340, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // DEEP WATER DIE STOP
	M("LOW TO DEEP WATER",               100, 0.0f, ANIM_STATIONARY | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                                                 S, S), // WATER TRANSITION
	M("DEEP TO LOW WATER",               100, 0.0f, ANIM_STATIONARY | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                                                 S, S), // WATER TRANSITION
	M("GOTO SLEEP",                      100, 0.0f, ANIM_STATIONARY | ANIM_LOWER_WEAPON | ANIM_NONINTERRUPT| ANIM_NO_EFFORT,                                                                                 C, P), // GOTO SLEEP
	M("SLEEPING",                       2000, 0.0f, ANIM_STATIONARY | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                                                 P, P), // SLEEP
	M("WAKEUP",                          100, 0.0f, ANIM_STATIONARY | ANIM_RAISE_WEAPON | ANIM_NO_EFFORT,                                                                                                    P, C), // WAKEUP
	M("SHOOT (R) STAND LOW",             100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                      S, S), // SHOOT_RIFLE_STAND LOW
	M("BURST SHOOT (R) STAND LOW",       100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                      S, S), // SHOOT_RIFLE_STAND LOW
	M("LARVAE BREATH",                   250, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                 S, S), //LARVAE Breathing
	M("LARVAE HIT",                       80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              S, S), // GENERIC HIT CROUCHED
	M("LARVAE DIE",                       80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // FALL FORWARD FROM HIT STAND
	M("LARVAE DIE STOP",                 350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // FALL FORWARD FROM HIT STAND
	M("LARVAE WALKING",                  110, 0.5f, ANIM_MOVING | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                  S, S), // FEMALE MONSTER BREATHING
	M("INFANT HIT",                       80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              S, S), // GENERIC HIT CROUCHED
	M("INFANT DIE",                       80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // FALL FORWARD FROM HIT STAND
	M("INFANT DIE STOP",                 350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // FALL FORWARD FROM HIT STAND
	M("INFANT MONSTER SPIT ATTACK",       70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                          S, S),
	M("INFANT MONSTER BEGIN EATING",      70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                             S, S),
	M("INFANT MONSTER EATING",           120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                              S, S),
	M("INFANT MONSTER END EATING",        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                             S, S),
	M("ADULT  MONSTER UP",               130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                             S, S),
	M("ADULT  MONSTER JUMP",             110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                             S, S),
	M("UNJAM STANDING",                  150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                                 S, S), // UNJAM STANDING
	M("UNJAM CROUCH",                    150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                                 C, C), // UNJAM CROUCH
	M("UNJAM PRONE",                     150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                                 P, P), // UNJAM PRONE
	M("UNJAM STAND DWEL",                150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                                 S, S), // UNJAM STAND DWEL
	M("UNJAM STAND LOW",                 150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                                 S, S), // UNJAM STAND LOW
	M("READY AIM (DW) CROUCH",           150, 0.0f, ANIM_OK_CHARGE_AP_FOR_TURN | ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN  | ANIM_LIGHT_EFFORT,                                       C, C), // READY_RIFLE_CROUCH
	M("AIM (DW) CROUCH",                 250, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FIREREADY | ANIM_FASTTURN | ANIM_LIGHT_EFFORT,                                                                     C, C), // AIM_RIFLE_CROUCH
	M("SHOOT (DW) CROUCH",               150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                                          C, C), // SHOOT_RIFLE_CROUCH
	M("END DUAL CROUCH",                 150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NOCHANGE_PENDINGCOUNT | ANIM_LIGHT_EFFORT,                                                                         C, C), // END_RIFLE_STAND
	M("UNJAM CROUCH DWEL",               150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                                 C, C), // UNJAM CROUCH DWEL
	M("GET ADJACENT ITEM",               100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //GET ADJACENT ITEM
	M("CUTTING FENCE",                    90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //CUTTING FENCE
	M("CRIPPLE BEG",                     110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("CRIPPLE HIT",                      80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              S, S), // CRIPPLE HIT
	M("CRIPPLE DIE",                      80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // CRIPPLE DIE
	M("CRIPPLE DIE STOP",                350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // CRIPPLE DIE STOP
	M("CRIPPLE FLYBACK DIE",              80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // CRIPPLE FLYBACK DIE
	M("CRIPPLE FLYBACK DIE STOP",        350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // CRIPPLE FLYBACK DIE STOP
	M("CRIPPLE KICK",                     80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_NOMOVE_MARKER | ANIM_MIN_EFFORT | ANIM_NOCHANGE_WEAPON,                                        S, S), // CRIPPLE KICK
	M("INJURED TRANSITION",               80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_NOMOVE_MARKER | ANIM_MIN_EFFORT | ANIM_NOCHANGE_WEAPON,                                        S, S), // INJURED TRANSITION
	M("THROW KNIFE",                      50, 0.0f, ANIM_STATIONARY  | ANIM_NOMOVE_MARKER | ANIM_NONINTERRUPT | ANIM_MIN_EFFORT,                                                                             S, S), //THROW KNIFE
	M("KNIFE BREATH",                    150, 0.0f, ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON,          S, S), //Breathing
	M("KNIFE GOTO BREATH",               150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NONINTERRUPT,       S, S), //Breathing
	M("KNIFE END BREATH",                150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NONINTERRUPT,       S, S), //Breathing
	M("ROBOT HIT",                        80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NONINTERRUPT | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                              S, S), // ROBOT HIT
	M("ROBOT DIE",                        80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // ROBOT DIE
	M("ROBOT DIE STOP",                  350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // ROBOT DIE STOP
	M("Catch Standing",                   80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_LOWER_WEAPON,                              S, S), //Catch Standing
	M("Catch Crouched",                   80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_LOWER_WEAPON,                              C, C), //Catch Crouched
	M("PLANT BOMB",                      120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //PLANT BOMB
	M("Use Remote",                       80, 0.0f, ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_NONINTERRUPT,         S, S), //Use remote
	M("START Cower",                      80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_NONINTERRUPT | ANIM_IGNORE_AUTOSTANCE, S, C), //COWER
	M("Cowering",                         80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,    C, C), //COWER
	M("End Cower",                        80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,    C, S), //END COWER
	M("STEAL ITEM",                      110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //STEAL ITEM
	M("LAUNCH ROCKET",                   110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     S, S), //LAUNCH ROCKET
	M("CIV ALT DIE",                      80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        S, P), // CIV ALT DIE
	M("SHOOT MORTAR",                    110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //SHOOT MORTAR
	M("CROW_DIE",                        130, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NOCHANGE_WEAPON | ANIM_LIGHT_EFFORT,                                         S, S), //CROW DIE
	M("SIDE STEPPING",                    50, 0.4f, ANIM_MOVING | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_VARIABLE_EFFORT,                                                                                 S, S), //SIDE STEPPING
	M("WALK BACKWARDS",                  110, 1.4f, ANIM_MOVING | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_VARIABLE_EFFORT | ANIM_TURNING,                                                                  S, S), //WALK BACKWARDS
	M("BEGIN OPEN STRUCT",                70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             S, S), //OPEN STRUCT
	M("END OPEN STRUCT",                  70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             S, S), //END STRUCT
	M("END OPEN LOCKED STRUCT",           70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             S, S), //END OPEN LOCKED
	M("LOW PUNCH",                        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT   | ANIM_NOCHANGE_WEAPON,                                                                                C, C), // Punch LOw
	M("SHOOT (P) STAND LOW",             100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                      S, S), // SHOOT_PISTOL_STAND LOW
	M("DECAPITATE",                       70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT   | ANIM_NONINTERRUPT,                                               C, C), //DECAPITATE
	M("BLOODCAT BITE",                    70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_MIN_EFFORT,                                                                   S, S),
	M("BIGMERC S NECK",                  110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("BIGMERC CROUCH TRANS INTO",        90, 0.0f, ANIM_STATIONARY | ANIM_TURNING  | ANIM_NOMOVE_MARKER | ANIM_STANCECHANGEANIM  | ANIM_RAISE_WEAPON | ANIM_MIN_EFFORT,                                     S, C), //CROUCH TRANSITION
	M("BIGMERC CROUCH TRANS OUTOF",       90, 0.0f, ANIM_STATIONARY | ANIM_TURNING  | ANIM_NOMOVE_MARKER | ANIM_STANCECHANGEANIM  | ANIM_RAISE_WEAPON | ANIM_MIN_EFFORT,                                     C, S), //CROUCH TRANSITION
	M("GOTO PATIENT",                   1000, 0.0f, ANIM_STATIONARY | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                                 P, P), //GOTO PATIENT
	M("BEING PATIENT",                  1000, 0.0f, ANIM_STATIONARY | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                                 P, P), //BEING PATIENT
	M("GOTO DOCTOR",                      90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //GOTO DOCTOR
	M("BEING DOCTOR",                    120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     C, C), //BEING DOCTOR
	M("END DOCTOR",                       90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_NO_EFFORT   | ANIM_NOCHANGE_PENDINGCOUNT,                     C, C), //ENDING DOCTOR
	M("GOTO REPAIRMAN",                   90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //GOTO REPAIRMAN
	M("BEING REPAIRMAN",                 120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     C, C), //BEING REPAIRMAN
	M("END REPAIRMAN",                    90, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_NO_EFFORT   | ANIM_NOCHANGE_PENDINGCOUNT,                     C, C), //ENDING REPAIRMAN
	M("FALL INTO PIT",                    60, 0.0f, ANIM_STATIONARY | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                       S, P), //FALL INTO PIT
	M("ROBOT WALKING",                    80, 1.5f, ANIM_MOVING | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                  S, S), // ROBOT WALK
	M("ROBOT SHOOT",                     100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                      S, S), // ROBOT SHOOT
	M("QUEEN HIT",                       200, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITSTART | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                  S, S), // QUEEN HIT
	M("QUEEN DIE",                       120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                            S, P), // QUEEN DIE
	M("QUEEN DIE STOP",                  350, 0.0f, ANIM_STATIONARY | ANIM_HITSTOP | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                                  P, P), // QUEEN DIE STOP
	M("QUEEN INTO READY",                130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        S, S),
	M("QUEEN READY",                     130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        S, S),
	M("QUEEN END READY",                 130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        S, S),
	M("QUEEN CALL",                      130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        S, S),
	M("QUEEN MONSTER SPIT ATTACK",       130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                          S, S),
	M("QUEEN MONSTER SWIPE ATTACK",      130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                          S, S),
	M("RELOAD ROBOT",                     70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //RELOAD ROBOT
	M("Catch Standing",                   80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NONINTERRUPT,       S, S), //End Catch Standing
	M("Catch Crouched",                   80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_FIREREADY | ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NONINTERRUPT,       C, C), //End Catch Crouched
	M("RADIO",                           130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        S, S),
	M("CROUCH RADIO",                    130, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NORESTART | ANIM_NO_EFFORT,                                                                                        C, C),
	M("TANK SHOOT",                      200, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NOCHANGE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                   S, S), // TANK SHOOT
	M("TANK BURST",                      100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NOCHANGE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                   S, S), // TANK SHOOT
	M("QUEEN SLAP",                      120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART |  ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON,                                           S, S), //Queen Slap
	M("SLAP HIT",                         70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART |  ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON,                                           S, S), //Slap hit
	M("GET BLOOD",                        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //GET BLOOD
	M("VEHICLE DIE",                     150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NOCHANGE_WEAPON | ANIM_LIGHT_EFFORT,                          S, S), //VEHICLE DIE
	M("QUEEN FRUSTRATED SLAP",           120, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART |  ANIM_LIGHT_EFFORT   | ANIM_NOCHANGE_WEAPON,                                           S, S), //Queen Frustrated Slap
	M("FIRE FIRE",                       150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_NOCHANGE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_IGNORE_AUTOSTANCE, S, S), //SOLDIER BRUN
	M("AI TRIGGER",                       70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //AI TRIGGER
	M("MONSTERMELT",                      80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NONINTERRUPT | ANIM_HITFINISH | ANIM_NOMOVE_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON,                        P, P), // MONSTER MELT
	M("MERC INJURED IDLE ANIM",          110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("END INJURED WALK",                 80, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NOMOVE_MARKER | ANIM_MIN_EFFORT | ANIM_NOCHANGE_WEAPON,                                                            S, S), // INJURED TRANSITION
	M("PASS OBJECT",                     110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //PASS OBJECT
	M("ADJACENT DROP OBJECT",            110, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //ADJACENT DROP OBJECT
	M("READY AIM (DW) PRONE",            150, 0.0f, ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN  | ANIM_LIGHT_EFFORT,                                                                    P, P), // READY_RIFLE_PRONE
	M("AIM (DW) PRONE",                  250, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FIREREADY | ANIM_FASTTURN | ANIM_LIGHT_EFFORT,                                                                     P, P), // AIM_RIFLE_PRONE
	M("SHOOT (DW) PRONE",                150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_LIGHT_EFFORT | ANIM_FIRE,                                                                                          P, P), // SHOOT_RIFLE_PRONE
	M("END DUAL PRONE",                  150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NOCHANGE_PENDINGCOUNT | ANIM_LIGHT_EFFORT,                                                                         P, P), // END_RIFLE_PRONE
	M("UNJAM PRONE DWEL",                150, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NOCHANGE_WEAPON | ANIM_MIN_EFFORT,                                                                 P, P), // UNJAM PRONE DWEL
	M("PICK LOCK",                        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 S, S), //PICK LOCK
	M("OPEN DOOR CROUCHED",               70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //OPEN DOOR CROUCHED
	M("BEGIN OPEN STRUCT CROUCHED",       70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //BEGIN OPENSTRUCT CROUCHED
	M("CLOSE DOOR CROUCHED",              70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //CLOSE DOOR CROUCHED
	M("OPEN STRUCT CROUCHED",             70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //OPEN STRUCT CROUCHED
	M("END OPEN DOOR CROUCHED",           70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //END OPEN DOOR CROUCHED
	M("END OPENSTRUCT CROUCHED",          70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //END OPENSTRUCT CROUCHED
	M("END OPNE LOCKED DOOR CR",          70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //END OPEN LOCKED DOOR CROUCHED
	M("END OPEN STRUCT LOCKED CR",        70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C), //END OPEN STRUCT LOCKED CR
	M("DRUNK IDLE",                      170, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_MERCIDLE | ANIM_NOCHANGE_WEAPON | ANIM_NO_EFFORT,                                                                  S, S),
	M("CROWBAR",                          70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_NO_EFFORT   | ANIM_NOCHANGE_WEAPON | ANIM_NORESTART,                                                               S, S), // CROWBAR
	M("COWER HIT",                        80, 1.6f, ANIM_HITSTART | ANIM_HITFINISH | ANIM_NONINTERRUPT | ANIM_NOSHOW_MARKER | ANIM_NO_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_IGNORE_AUTOSTANCE,                C, C), // COWER HIT ANIMATION
	M("BLOODCAT WALKING BACKWARDS",       30, 1.2f, ANIM_MOVING | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                  S, S), // BLOODCAT WALK BACKWARDS
	M("MONSTER WALKING BACKWARDS",        30, 2.5f, ANIM_MOVING | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                  S, S), // CREATURE WALK BACKWARDS
	M("KID SKIPPING",                     70, 2.0f, ANIM_MOVING | ANIM_TURNING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON,                                                                  S, S), // KID SKIPPING
	M("STAND BURST SHOOT",               100, 0.0f, ANIM_NORESTART | ANIM_STATIONARY | ANIM_TURNING | ANIM_RAISE_WEAPON | ANIM_LIGHT_EFFORT | ANIM_NOCHANGE_WEAPON | ANIM_FIRE,                              S, S), // STAND BURST
	M("Attach String",                    70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //Attach string
	M("SWAT BACKWARDS",                  110, 1.4f, ANIM_MOVING | ANIM_NORESTART | ANIM_LIGHT_EFFORT | ANIM_TURNING,                                                                                         C, C), //SWAT BACKWARDS
	M("JUMP OVER",                       110, 3.6f, ANIM_SPECIALMOVE | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_MODERATE_EFFORT | ANIM_TURNING,                                                             S, S), //JUMP OVER BLOCKING PERSON
	M("REFUEL VEHICLE",                   70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                             C, C), //REFUEL VEHICLE
	M("ROBOT CAMERA",                      0, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_RAISE_WEAPON | ANIM_VARIABLE_EFFORT | ANIM_BREATH,                                S, S), //Robot camera not moving
	M("CRIPPLE OPEN DOOR",                70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LIGHT_EFFORT,                                                                     S, S), //CRIPPLE OPEN DOOR
	M("CRIPPLE CLOSE DOOR",              100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_MIN_EFFORT,                                                                       S, S), //CRIPPLE CLOSE DOOR
	M("END OPEN DOOR",                   100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_MIN_EFFORT,                                                                       S, S), //CRIPPLE END OPEN DOOR
	M("END OPEN LOCKED DOOR",            100, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_NONINTERRUPT | ANIM_MIN_EFFORT,                                                   S, S), //CRIPPLE END OPEN LOCKED DOOR
	M("CROUCH PICK LOCK",                 70, 0.0f, ANIM_STATIONARY | ANIM_TURNING | ANIM_FASTTURN | ANIM_NORESTART | ANIM_LOWER_WEAPON | ANIM_LIGHT_EFFORT,                                                 C, C)  //CROUCH PICK LOCK
};
CASSERT(lengthof(gAnimControl) == NUMANIMATIONSTATES)

#undef M
#undef P
#undef C
#undef S

const ANI_SPEED_DEF gubAnimWalkSpeeds[TOTALBODYTYPES] =
{
	{  -5, 1.6f },// REGMALE
	{ -10, 1.6f },// BIGMALE
	{  -5, 1.6f },// STOCKYMALE
	{ -15, 1.6f },// REGFEMALE
	{   0, 1.6f },// ADULTMONSTER
	{   0, 1.6f },// ADULTMONSTER
	{   0, 1.6f },// ADULTMONSTER
	{   0, 1.6f },// ADULTMONSTER
	{   0, 1.6f },// ADULTMONSTER
	{   0, 2.2f },// INFANT
	{   0, 1.6f },// QUEEN MONSTER
	{  40, 1.3f },// FATCIV
	{  10, 1.3f },// MANCIV
	{ -10, 1.3f },// MINICIV
	{ -10, 1.3f },// DRESSCIV
	{ -20, 1.6f },// HAT KID
	{ -20, 1.6f },// NOHAT KID
	{ -20, 1.6f },// CRIPPLE
	{  60, 0.9f },// COW
	{  20, 1.6f },// CROW
	{   0, 1.2f },// BLOOD CAT
	{  20, 1.1f },// ROBOT1
	{ -10, 4.0f },// HUMVEE

	{ -10, 4.0f },// TANK_NW
	{ -10, 4.0f },// TANK_NE
	{ -10, 4.0f },// ELDORADO
	{ -10, 4.0f },// ICECREAMTRUCK
	{ -10, 4.0f },// JEEP
};


const ANI_SPEED_DEF gubAnimRunSpeeds[TOTALBODYTYPES] =
{
	{   0, 2.6f },// REGMALE
	{   5, 2.6f },// BIGMALE
	{   0, 2.6f },// STOCKYMALE
	{ -10, 2.6f },// REGFEMALE
	{   0, 2.6f },// ADULTMONSTER
	{   0, 2.6f },// ADULTMONSTER
	{   0, 2.6f },// ADULTMONSTER
	{   0, 2.6f },// ADULTMONSTER
	{   0, 2.6f },// ADULTMONSTER
	{   0, 2.6f },// ADULTMONSTER
	{   0, 2.8f },// INFANT
	{  10, 2.3f },// FATCIV
	{ -10, 2.3f },// MANCIV
	{ -20, 2.3f },// MINICIV
	{ -30, 2.3f },// DRESSCIV
	{ -40, 2.6f },// HAT KID
	{ -40, 2.6f },// NOHAT KID
	{ -20, 2.3f },// CRIPPLE
	{  30, 2.0f },// COW
};


// Really only the first mercs are using any of these values....
const ANI_SPEED_DEF gubAnimSwatSpeeds[TOTALBODYTYPES] =
{
	{   0, 2.2f }, // REGMALE
	{  20, 2.2f }, // BIGMALE
	{   0, 2.2f }, // STOCKYMALE
  { -10, 2.2f }  // REGFEMALE
};

const ANI_SPEED_DEF gubAnimCrawlSpeeds[TOTALBODYTYPES] = { 0 }; // XXX TODO0011 only declared

// Really only the first mercs are using any of these values....
static const ANI_SPEED_DEF gubAnimCrawlingSpeeds[TOTALBODYTYPES] = // XXX TODO0011 unused, should probably be gubAnimCrawlSpeeds[]
{
	{   0, 0.8f }, // REGMALE
	{  20, 0.8f }, // BIGMALE
	{   0, 0.8f }, // STOCKYMALE
	{ -10, 0.8f }  // REGFEMALE
};


UINT16 gusNothingBreath[ ] =
{
	RGMNOTHING_STD,
	BGMNOTHING_STD,
	RGMNOTHING_STD,
	RGFNOTHING_STD,
};


static UINT16 gubAnimSurfaceIndex[TOTALBODYTYPES][NUMANIMATIONSTATES];
UINT16 gubAnimSurfaceMidWaterSubIndex[ TOTALBODYTYPES ][ NUMANIMATIONSTATES ][2];
UINT16 gubAnimSurfaceItemSubIndex[ TOTALBODYTYPES ][ NUMANIMATIONSTATES ];
UINT16 gubAnimSurfaceCorpseID[ TOTALBODYTYPES ][ NUMANIMATIONSTATES ];


static const ANIMSUBTYPE gRifleInjuredSub   = { WALKING,  { RGMHURTWALKINGR,  BGMHURTWALKINGR,  RGMHURTWALKINGR,  RGFHURTWALKINGR } };
static const ANIMSUBTYPE gNothingInjuredSub = { WALKING,  { RGMHURTWALKINGN,  BGMHURTWALKINGN,  RGMHURTWALKINGN,  RGFHURTWALKINGN } };
static const ANIMSUBTYPE gDoubleHandledSub  = { STANDING, { RGMDBLBREATH,     BGMDBLBREATH,     RGMDBLBREATH,     RGFDBLBREATH    } };


void	InitAnimationSurfacesPerBodytype( )
{
	INT32 cnt1, cnt2;

	// Should be set to a non-init values
	for ( cnt1 = 0; cnt1 < TOTALBODYTYPES; cnt1 ++ )
	{
		for ( cnt2 = 0; cnt2 < NUMANIMATIONSTATES; cnt2 ++ )
		{
			gubAnimSurfaceIndex[ cnt1][ cnt2 ] = INVALID_ANIMATION;
			gubAnimSurfaceItemSubIndex[ cnt1][ cnt2 ] = INVALID_ANIMATION;
			gubAnimSurfaceMidWaterSubIndex[ cnt1][ cnt2 ][ 0 ] = INVALID_ANIMATION;
			gubAnimSurfaceMidWaterSubIndex[ cnt1][ cnt2 ][ 1 ] = INVALID_ANIMATION;
			gubAnimSurfaceCorpseID[ cnt1][ cnt2 ] = NO_CORPSE;
		}
	}
	memset( gRandomAnimDefs, 0, sizeof( gRandomAnimDefs ) );


	// REGULAR MALE GUY
	gubAnimSurfaceIndex[ REGMALE ][ WALKING ]								= RGMBASICWALKING;
	gubAnimSurfaceIndex[ REGMALE ][ STANDING ]							= RGMSTANDING;
	gubAnimSurfaceIndex[ REGMALE ][ KNEEL_DOWN ]						= RGMCROUCHING;
	gubAnimSurfaceIndex[ REGMALE ][ CROUCHING ]							= RGMCROUCHING;
	gubAnimSurfaceIndex[ REGMALE ][ KNEEL_UP ]							= RGMCROUCHING;
	gubAnimSurfaceIndex[ REGMALE ][ SWATTING ]							= RGMSNEAKING;
	gubAnimSurfaceIndex[ REGMALE ][ RUNNING ]								= RGMRUNNING;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_DOWN ]						= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ CRAWLING ]							= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_UP ]							= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE ]									= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ READY_RIFLE_STAND ]			= RGMSTANDAIM;
	gubAnimSurfaceIndex[ REGMALE ][ AIM_RIFLE_STAND ]				= RGMSTANDAIM;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_RIFLE_STAND ]			= RGMSTANDAIM;
	gubAnimSurfaceIndex[ REGMALE ][ END_RIFLE_STAND ]				= RGMSTANDAIM;
	gubAnimSurfaceIndex[ REGMALE ][ START_SWAT ]						= RGMSNEAKING;
	gubAnimSurfaceIndex[ REGMALE ][ END_SWAT ]							= RGMSNEAKING;
	gubAnimSurfaceIndex[ REGMALE ][ FLYBACK_HIT ]						= RGMHITHARD;
	gubAnimSurfaceIndex[ REGMALE ][ READY_RIFLE_PRONE ]			= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ AIM_RIFLE_PRONE ]				= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_RIFLE_PRONE ]			= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ END_RIFLE_PRONE ]				= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ FALLBACK_DEATHTWICH ]		= RGMHITHARD;
	gubAnimSurfaceIndex[ REGMALE ][ GENERIC_HIT_STAND ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ FLYBACK_HIT_BLOOD_STAND ]		= RGMHITHARDBLOOD;
	gubAnimSurfaceIndex[ REGMALE ][ FLYBACK_HIT_DEATH ]			= RGMHITHARD;
	gubAnimSurfaceIndex[ REGMALE ][ READY_RIFLE_CROUCH ]		= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ REGMALE ][ AIM_RIFLE_CROUCH ]			= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_RIFLE_CROUCH ]		= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ REGMALE ][ END_RIFLE_CROUCH ]			= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ REGMALE ][ FALLBACK_HIT_STAND ]		= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ REGMALE ][ ROLLOVER ]							= RGMROLLOVER;
	gubAnimSurfaceIndex[ REGMALE ][ CLIMBUPROOF ]						= RGMCLIMBROOF;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF ]								= RGMFALL;
	gubAnimSurfaceIndex[ REGMALE ][ GETUP_FROM_ROLLOVER ]		= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ CLIMBDOWNROOF ]					= RGMCLIMBROOF;
	gubAnimSurfaceIndex[ REGMALE ][ FALLFORWARD_ROOF ]			= RGMFALLF;
	gubAnimSurfaceIndex[ REGMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ GENERIC_HIT_DEATHTWITCHB ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]	= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ REGMALE ][ FALLBACK_HIT_DEATHTWITCHB ]		= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ REGMALE ][ GENERIC_HIT_DEATH ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ FALLBACK_HIT_DEATH ]		= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ REGMALE ][ GENERIC_HIT_CROUCH ]		= RGMHITCROUCH;
	gubAnimSurfaceIndex[ REGMALE ][ STANDING_BURST ]				= RGMSTANDAIM;
	gubAnimSurfaceIndex[ REGMALE ][ STANDING_BURST_HIT ]		= RGMHITHARD;
	gubAnimSurfaceIndex[ REGMALE ][ GENERIC_HIT_DEATH ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ FALLFORWARD_FROMHIT_STAND ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ FALLFORWARD_FROMHIT_STAND ]				= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ FALLFORWARD_FROMHIT_CROUCH ]			= RGMHITCROUCH;
	gubAnimSurfaceIndex[ REGMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ GENERIC_HIT_PRONE ]			= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_HIT_DEATH ]				= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_LAY_FROMHIT ]			= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_HIT_DEATHTWITCHNB ]					= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_HIT_DEATHTWITCHB ]					= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ FLYBACK_HITDEATH_STOP ]					= RGMHITHARD;
	gubAnimSurfaceIndex[ REGMALE ][ FALLFORWARD_HITDEATH_STOP ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ FALLBACK_HITDEATH_STOP ]				= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_HITDEATH_STOP ]						= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ FLYBACKHIT_STOP ]								= RGMHITHARD;
	gubAnimSurfaceIndex[ REGMALE ][ FALLBACKHIT_STOP ]							= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_STOP ]									= RGMFALL;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_FORWARD_STOP ]					= RGMFALLF;
	gubAnimSurfaceIndex[ REGMALE ][ STAND_FALLFORWARD_STOP ]				= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_LAYFROMHIT_STOP ]					= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ HOPFENCE ]											= RGMHOPFENCE;
	gubAnimSurfaceIndex[ REGMALE ][ PUNCH_BREATH ]									= RGMPUNCH;
	gubAnimSurfaceIndex[ REGMALE ][ PUNCH ]													= RGMPUNCH;
	gubAnimSurfaceIndex[ REGMALE ][ NOTHING_STAND ]									= RGMNOTHING_STD;
	gubAnimSurfaceIndex[ REGMALE ][ JFK_HITDEATH ]									= RGMDIE_JFK;
	gubAnimSurfaceIndex[ REGMALE ][ JFK_HITDEATH_STOP ]							= RGMDIE_JFK;
	gubAnimSurfaceIndex[ REGMALE ][ JFK_HITDEATH_TWITCHB ]					= RGMDIE_JFK;
	gubAnimSurfaceIndex[ REGMALE ][ FIRE_STAND_BURST_SPREAD ]				= RGMSTANDAIM;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_DEATH ]									= RGMFALL;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_DEATH_STOP ]						= RGMFALL;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_TWITCHB ]								= RGMFALL;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_TWITCHNB ]							= RGMFALL;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_FORWARD_DEATH ]					= RGMFALLF;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_FORWARD_DEATH_STOP ]		= RGMFALLF;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_FORWARD_TWITCHB ]				= RGMFALLF;
	gubAnimSurfaceIndex[ REGMALE ][ FALLOFF_FORWARD_TWITCHNB ]			= RGMFALLF;
	gubAnimSurfaceIndex[ REGMALE ][ OPEN_DOOR ]											= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ OPEN_STRUCT ]										= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ PICKUP_ITEM ]										= RGMPICKUP;
	gubAnimSurfaceIndex[ REGMALE ][ DROP_ITEM ]											= RGMPICKUP;
	gubAnimSurfaceIndex[ REGMALE ][ SLICE ]													= RGMSLICE;
	gubAnimSurfaceIndex[ REGMALE ][ STAB ]													= RGMSTAB;
	gubAnimSurfaceIndex[ REGMALE ][ CROUCH_STAB ]										= RGMCSTAB;
	gubAnimSurfaceIndex[ REGMALE ][ START_AID ]											= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ GIVING_AID ]										= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_AID ]												= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ DODGE_ONE ]											= RGMDODGE;
	gubAnimSurfaceIndex[ REGMALE ][ READY_DUAL_STAND ]							= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGMALE ][ AIM_DUAL_STAND ]								= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_DUAL_STAND ]							= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGMALE ][ END_DUAL_STAND ]								= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGMALE ][ RAISE_RIFLE ]										= RGMRAISE;
	gubAnimSurfaceIndex[ REGMALE ][ LOWER_RIFLE ]										= RGMRAISE;
	gubAnimSurfaceIndex[ REGMALE ][ BODYEXPLODING ]									= BODYEXPLODE;
	gubAnimSurfaceIndex[ REGMALE ][ THROW_ITEM ]										= RGMTHROW;
	gubAnimSurfaceIndex[ REGMALE ][ LOB_ITEM ]											= RGMLOB;
	gubAnimSurfaceIndex[ REGMALE ][ CROUCHED_BURST ]								= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_BURST ]										= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ GIVE_ITEM ]											= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ CLIMB_CLIFF ]										= RGMMCLIMB;
	gubAnimSurfaceIndex[ REGMALE ][ CLIMB_CLIFF ]										= RGMMCLIMB;
	gubAnimSurfaceIndex[ REGMALE ][ WATER_HIT ]											= RGMWATER_DIE;
	gubAnimSurfaceIndex[ REGMALE ][ WATER_DIE]											= RGMWATER_DIE;
	gubAnimSurfaceIndex[ REGMALE ][ WATER_DIE_STOP ]								= RGMWATER_DIE;
	gubAnimSurfaceIndex[ REGMALE ][ HELIDROP ]											= RGMHELIDROP;
	gubAnimSurfaceIndex[ REGMALE ][ FEM_LOOK ]											= RGM_LOOK;
	gubAnimSurfaceIndex[ REGMALE ][ REG_SQUISH ]										= RGM_SQUISH;
	gubAnimSurfaceIndex[ REGMALE ][ REG_PULL ]											= RGM_PULL;
	gubAnimSurfaceIndex[ REGMALE ][ REG_SPIT ]											= RGM_SPIT;
	gubAnimSurfaceIndex[ REGMALE ][ NINJA_GOTOBREATH ]							= RGMLOWKICK;
	gubAnimSurfaceIndex[ REGMALE ][ NINJA_BREATH ]									= RGMLOWKICK;
	gubAnimSurfaceIndex[ REGMALE ][ NINJA_LOWKICK ]									= RGMLOWKICK;
	gubAnimSurfaceIndex[ REGMALE ][ NINJA_PUNCH ]										= RGMNPUNCH;
	gubAnimSurfaceIndex[ REGMALE ][ NINJA_SPINKICK ]								= RGMSPINKICK;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPEN_DOOR ]									= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPEN_LOCKED_DOOR ]					= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ KICK_DOOR ]											= RGMKICKDOOR;
	gubAnimSurfaceIndex[ REGMALE ][ CLOSE_DOOR ]										= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ RIFLE_STAND_HIT ]								= RGMRHIT;
	gubAnimSurfaceIndex[ REGMALE ][ DEEP_WATER_TRED ]								= RGMDEEPWATER_TRED;
	gubAnimSurfaceIndex[ REGMALE ][ DEEP_WATER_SWIM ]								= RGMDEEPWATER_SWIM;
	gubAnimSurfaceIndex[ REGMALE ][ DEEP_WATER_HIT ]								= RGMDEEPWATER_DIE;
	gubAnimSurfaceIndex[ REGMALE ][ DEEP_WATER_DIE]									= RGMDEEPWATER_DIE;
	gubAnimSurfaceIndex[ REGMALE ][ DEEP_WATER_DIE_STOPPING ]				= RGMWATER_DIE;
	gubAnimSurfaceIndex[ REGMALE ][ DEEP_WATER_DIE_STOP ]						= RGMWATER_DIE;
	gubAnimSurfaceIndex[ REGMALE ][ LOW_TO_DEEP_WATER ]							= RGMWATER_TRANS;
	gubAnimSurfaceIndex[ REGMALE ][ DEEP_TO_LOW_WATER ]							= RGMWATER_TRANS;
	gubAnimSurfaceIndex[ REGMALE ][ GOTO_SLEEP ]										= RGMSLEEP;
	gubAnimSurfaceIndex[ REGMALE ][ SLEEPING ]											= RGMSLEEP;
	gubAnimSurfaceIndex[ REGMALE ][ WKAEUP_FROM_SLEEP ]							= RGMSLEEP;
	gubAnimSurfaceIndex[ REGMALE ][ FIRE_LOW_STAND ]								= RGMSHOOT_LOW;
	gubAnimSurfaceIndex[ REGMALE ][ FIRE_BURST_LOW_STAND ]					= RGMSHOOT_LOW;
	gubAnimSurfaceIndex[ REGMALE ][ STANDING_SHOOT_UNJAM ]					= RGMSTANDAIM;
	gubAnimSurfaceIndex[ REGMALE ][ CROUCH_SHOOT_UNJAM ]						= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_SHOOT_UNJAM ]							= RGMPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ STANDING_SHOOT_DWEL_UNJAM ]		  = RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGMALE ][ STANDING_SHOOT_LOW_UNJAM ]		  = RGMSHOOT_LOW;
	gubAnimSurfaceIndex[ REGMALE ][ READY_DUAL_CROUCH ]							= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ REGMALE ][ AIM_DUAL_CROUCH ]								= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_DUAL_CROUCH ]							= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ REGMALE ][ END_DUAL_CROUCH ]								= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ REGMALE ][ CROUCH_SHOOT_DWEL_UNJAM ]				= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ REGMALE ][ ADJACENT_GET_ITEM ]							= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ CUTTING_FENCE ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ FROM_INJURED_TRANSITION ]				= RGMHURTTRANS;
	gubAnimSurfaceIndex[ REGMALE ][ THROW_KNIFE ]										= RGMTHROWKNIFE;
	gubAnimSurfaceIndex[ REGMALE ][ KNIFE_BREATH ]									= RGMBREATHKNIFE;
	gubAnimSurfaceIndex[ REGMALE ][ KNIFE_GOTOBREATH ]							= RGMBREATHKNIFE;
	gubAnimSurfaceIndex[ REGMALE ][ KNIFE_ENDBREATH ]								= RGMBREATHKNIFE;
	gubAnimSurfaceIndex[ REGMALE ][ CATCH_STANDING ]								= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ CATCH_CROUCHED ]								= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ PLANT_BOMB ]										= RGMPICKUP;
	gubAnimSurfaceIndex[ REGMALE ][ USE_REMOTE ]										= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ START_COWER ]										= RGMCOWER;
	gubAnimSurfaceIndex[ REGMALE ][ COWERING ]											= RGMCOWER;
	gubAnimSurfaceIndex[ REGMALE ][ END_COWER ]											= RGMCOWER;
	gubAnimSurfaceIndex[ REGMALE ][ STEAL_ITEM ]										= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_ROCKET ]									= RGMROCKET;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_MORTAR ]									= RGMMORTAR;
	gubAnimSurfaceIndex[ REGMALE ][ SIDE_STEP ]											= RGMSIDESTEP;
	gubAnimSurfaceIndex[ REGMALE ][ WALK_BACKWARDS ]								= RGMBASICWALKING;
	gubAnimSurfaceIndex[ REGMALE ][ BEGIN_OPENSTRUCT ]							= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPENSTRUCT ]								= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPENSTRUCT_LOCKED ]					= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ PUNCH_LOW ]											= RGMPUNCHLOW;
	gubAnimSurfaceIndex[ REGMALE ][ PISTOL_SHOOT_LOW ]							= RGMPISTOLSHOOTLOW;
	gubAnimSurfaceIndex[ REGMALE ][ DECAPITATE ]										= RGMCSTAB;
	gubAnimSurfaceIndex[ REGMALE ][ GOTO_PATIENT ]									= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ BEING_PATIENT ]									= RGMHITPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ GOTO_DOCTOR ]										= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ BEING_DOCTOR ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_DOCTOR ]										= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ GOTO_REPAIRMAN ]								= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ BEING_REPAIRMAN ]								= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_REPAIRMAN ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ FALL_INTO_PIT ]									= RGMFALL;
	gubAnimSurfaceIndex[ REGMALE ][ RELOAD_ROBOT ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_CATCH ]											= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ END_CROUCH_CATCH ]							= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ AI_RADIO ]											= RGMRADIO;
	gubAnimSurfaceIndex[ REGMALE ][ AI_CR_RADIO ]										= RGMCRRADIO;
	gubAnimSurfaceIndex[ REGMALE ][ SLAP_HIT ]											= RGMHITSTAND;
	gubAnimSurfaceIndex[ REGMALE ][ TAKE_BLOOD_FROM_CORPSE ]				= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ CHARIOTS_OF_FIRE ]							= RGMBURN;
	gubAnimSurfaceIndex[ REGMALE ][ AI_PULL_SWITCH ]								= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ MERC_HURT_IDLE_ANIM ]						= RGMHURTSTANDINGR;
	gubAnimSurfaceIndex[ REGMALE ][ END_HURT_WALKING ]							= RGMHURTSTANDINGR;
	gubAnimSurfaceIndex[ REGMALE ][ PASS_OBJECT ]										= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ DROP_ADJACENT_OBJECT ]					= RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ READY_DUAL_PRONE ]							= RGMDWPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ AIM_DUAL_PRONE ]								= RGMDWPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ SHOOT_DUAL_PRONE ]							= RGMDWPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ END_DUAL_PRONE ]								= RGMDWPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PRONE_SHOOT_DWEL_UNJAM ]			  = RGMDWPRONE;
	gubAnimSurfaceIndex[ REGMALE ][ PICK_LOCK ]										  = RGMOPEN;
	gubAnimSurfaceIndex[ REGMALE ][ OPEN_DOOR_CROUCHED ]						= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ BEGIN_OPENSTRUCT_CROUCHED ]			= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ CLOSE_DOOR_CROUCHED ]						= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ OPEN_STRUCT_CROUCHED ]					= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPEN_DOOR_CROUCHED ]				= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPENSTRUCT_CROUCHED ]				= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPEN_LOCKED_DOOR_CROUCHED ]	= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ END_OPENSTRUCT_LOCKED_CROUCHED ]= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ DRUNK_IDLE ]										= RGMDRUNK;
	gubAnimSurfaceIndex[ REGMALE ][ CROWBAR_ATTACK ]								= RGMCROWBAR;
	gubAnimSurfaceIndex[ REGMALE ][ CRIPPLE_KICKOUT ]								= CRIPCIVKICK;
	gubAnimSurfaceIndex[ REGMALE ][ ATTACH_CAN_TO_STRING ]					= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ SWAT_BACKWARDS ]								= RGMSNEAKING;
	gubAnimSurfaceIndex[ REGMALE ][ JUMP_OVER_BLOCKING_PERSON ]			= RGMJUMPOVER;
	gubAnimSurfaceIndex[ REGMALE ][ REFUEL_VEHICLE ]								= RGMMEDIC;
	gubAnimSurfaceIndex[ REGMALE ][ LOCKPICK_CROUCHED ]					  	= RGMMEDIC;


	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ STANDING][0]									= RGMWATER_R_STD;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ WALKING ][0]									= RGMWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ RUNNING ][0]									= RGMWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ READY_RIFLE_STAND ][0]				= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ AIM_RIFLE_STAND ][0]					= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ SHOOT_RIFLE_STAND ][0]				= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ END_RIFLE_STAND ][0]					= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ STANDING_BURST ][0]					= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ FIRE_STAND_BURST_SPREAD ][0]	= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ STANDING][1]									= RGMWATER_N_STD;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ WALKING ][1]									= RGMWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ RUNNING ][1]									= RGMWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ READY_RIFLE_STAND ][1]				= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ AIM_RIFLE_STAND ][1]					= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ SHOOT_RIFLE_STAND ][1]				= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ END_RIFLE_STAND ][1]					= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ STANDING_BURST ][1]					= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ FIRE_STAND_BURST_SPREAD ][1]	= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ READY_DUAL_STAND ][1]				= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ AIM_DUAL_STAND ][1]					= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ SHOOT_DUAL_STAND ][1]				= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ END_DUAL_STAND ][1]					= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ STANDING_SHOOT_UNJAM ][0]		= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ STANDING_SHOOT_UNJAM ][1]		= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ THROW_ITEM ][0]							= RGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ THROW_ITEM ][1]							= RGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ LOB_ITEM ][0]								= RGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ REGMALE ][ LOB_ITEM ][1]								= RGMWATERTHROW;


	//Setup some random stuff
	gRandomAnimDefs[ REGMALE ][ 0 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ REGMALE ][ 0 ].sAnimID							= REG_SPIT;
	gRandomAnimDefs[ REGMALE ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ REGMALE ][ 0 ].ubEndRoll						= 3;
	gRandomAnimDefs[ REGMALE ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ REGMALE ][ 0 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGMALE ][ 1 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ REGMALE ][ 1 ].sAnimID							= REG_SQUISH;
	gRandomAnimDefs[ REGMALE ][ 1 ].ubStartRoll					= 10;
	gRandomAnimDefs[ REGMALE ][ 1 ].ubEndRoll						= 13;
	gRandomAnimDefs[ REGMALE ][ 1 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ REGMALE ][ 1 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGMALE ][ 2 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ REGMALE ][ 2 ].sAnimID							= FEM_LOOK;
	gRandomAnimDefs[ REGMALE ][ 2 ].ubStartRoll					= 20;
	gRandomAnimDefs[ REGMALE ][ 2 ].ubEndRoll						= 23;
	gRandomAnimDefs[ REGMALE ][ 2 ].ubFlags							= 0;
	gRandomAnimDefs[ REGMALE ][ 2 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGMALE ][ 3 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ REGMALE ][ 3 ].sAnimID							= REG_PULL;
	gRandomAnimDefs[ REGMALE ][ 3 ].ubStartRoll					= 30;
	gRandomAnimDefs[ REGMALE ][ 3 ].ubEndRoll						= 33;
	gRandomAnimDefs[ REGMALE ][ 3 ].ubFlags							= 0;
	gRandomAnimDefs[ REGMALE ][ 3 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGMALE ][ 4 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ REGMALE ][ 4 ].sAnimID							= MERC_HURT_IDLE_ANIM;
	gRandomAnimDefs[ REGMALE ][ 4 ].ubStartRoll					= 40;
	gRandomAnimDefs[ REGMALE ][ 4 ].ubEndRoll						= 100;
	gRandomAnimDefs[ REGMALE ][ 4 ].ubFlags							= RANDOM_ANIM_INJURED;
	gRandomAnimDefs[ REGMALE ][ 4 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGMALE ][ 5 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ REGMALE ][ 5 ].sAnimID							= DRUNK_IDLE;
	gRandomAnimDefs[ REGMALE ][ 5 ].ubStartRoll					= 40;
	gRandomAnimDefs[ REGMALE ][ 5 ].ubEndRoll						= 100;
	gRandomAnimDefs[ REGMALE ][ 5 ].ubFlags							= RANDOM_ANIM_DRUNK;
	gRandomAnimDefs[ REGMALE ][ 5 ].ubAnimHeight				= ANIM_STAND;


	gubAnimSurfaceItemSubIndex[ REGMALE ][ STANDING ]									= RGMPISTOLBREATH;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ WALKING ]									= RGMNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ RUNNING ]									= RGMNOTHING_RUN;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ SWATTING ]									= RGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ START_SWAT ]								= RGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ END_SWAT ]									= RGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ KNEEL_DOWN ]								= RGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ CROUCHING ]								= RGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ KNEEL_UP ]									= RGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ READY_RIFLE_STAND ]				= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ AIM_RIFLE_STAND ]					= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ SHOOT_RIFLE_STAND ]				= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ END_RIFLE_STAND ]					= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ STANDING_BURST ]						= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ FIRE_STAND_BURST_SPREAD ]	= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ READY_RIFLE_CROUCH ]				= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ AIM_RIFLE_CROUCH ]					= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ SHOOT_RIFLE_CROUCH ]				= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ END_RIFLE_CROUCH ]					= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ PRONE_DOWN ]								= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ CRAWLING ]									= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ PRONE_UP ]									= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ PRONE ]										= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ READY_RIFLE_PRONE ]				= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ AIM_RIFLE_PRONE ]					= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ SHOOT_RIFLE_PRONE ]				= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ END_RIFLE_PRONE ]					= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ GETUP_FROM_ROLLOVER ]			= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ STANDING_SHOOT_UNJAM ]			= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ CROUCH_SHOOT_UNJAM ]				= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ PRONE_SHOOT_UNJAM ]				= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ FIRE_BURST_LOW_STAND ]			= RGMPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ FIRE_LOW_STAND ]						= RGMPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ CROUCHED_BURST ]						= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ PRONE_BURST ]							= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ MERC_HURT_IDLE_ANIM ]			= RGMHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ END_HURT_WALKING ]					= RGMHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ WALK_BACKWARDS ]						= RGMNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ REGMALE ][ DRUNK_IDLE ]								= RGMPISTOLDRUNK;


	gubAnimSurfaceCorpseID[ REGMALE ][ GENERIC_HIT_DEATH ]						= SMERC_FWD;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLBACK_HIT_DEATH ]						= SMERC_BCK;
	gubAnimSurfaceCorpseID[ REGMALE ][ PRONE_HIT_DEATH ]							= SMERC_PRN;
	gubAnimSurfaceCorpseID[ REGMALE ][ WATER_DIE ]										= SMERC_WTR;
	gubAnimSurfaceCorpseID[ REGMALE ][ FLYBACK_HIT_DEATH ]						= SMERC_DHD;
	gubAnimSurfaceCorpseID[ REGMALE ][ JFK_HITDEATH ]									= SMERC_JFK;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLOFF_DEATH ]								= SMERC_FALL;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLOFF_FORWARD_DEATH ]				= SMERC_FALLF;
	gubAnimSurfaceCorpseID[ REGMALE ][ FLYBACK_HIT ]									= SMERC_DHD;
	gubAnimSurfaceCorpseID[ REGMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= SMERC_FWD;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLFORWARD_FROMHIT_STAND ]		= SMERC_FWD;
	gubAnimSurfaceCorpseID[ REGMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]= SMERC_FWD;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLBACK_HIT_STAND ]						= SMERC_BCK;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]		= SMERC_BCK;
	gubAnimSurfaceCorpseID[ REGMALE ][ PRONE_HIT_DEATHTWITCHNB ]			= SMERC_PRN;
	gubAnimSurfaceCorpseID[ REGMALE ][ PRONE_LAY_FROMHIT ]						= SMERC_PRN;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLOFF ]											= SMERC_FALLF;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLFORWARD_ROOF ]							= SMERC_FALL;
	gubAnimSurfaceCorpseID[ REGMALE ][ FLYBACKHIT_STOP ]							= SMERC_DHD;
	gubAnimSurfaceCorpseID[ REGMALE ][ STAND_FALLFORWARD_STOP ]				= SMERC_FWD;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLBACKHIT_STOP ]							= SMERC_BCK;
	gubAnimSurfaceCorpseID[ REGMALE ][ PRONE_LAYFROMHIT_STOP ]				= SMERC_PRN;
	gubAnimSurfaceCorpseID[ REGMALE ][ CHARIOTS_OF_FIRE ]							= BURNT_DEAD;
	gubAnimSurfaceCorpseID[ REGMALE ][ BODYEXPLODING ]								= EXPLODE_DEAD;


	// BIG MALE GUY
	gubAnimSurfaceIndex[ BIGMALE ][ WALKING ]								= BGMWALKING;
	gubAnimSurfaceIndex[ BIGMALE ][ STANDING ]							= BGMSTANDING;
	gubAnimSurfaceIndex[ BIGMALE ][ KNEEL_DOWN ]						= BGMCROUCHING;
	gubAnimSurfaceIndex[ BIGMALE ][ CROUCHING ]							= BGMCROUCHING;
	gubAnimSurfaceIndex[ BIGMALE ][ KNEEL_UP ]							= BGMCROUCHING;
	gubAnimSurfaceIndex[ BIGMALE ][ SWATTING ]							= BGMSNEAKING;
	gubAnimSurfaceIndex[ BIGMALE ][ RUNNING ]								= BGMRUNNING;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_DOWN ]						= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ CRAWLING ]							= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_UP ]							= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE ]									= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ READY_RIFLE_STAND ]			= BGMSTANDAIM2;
	gubAnimSurfaceIndex[ BIGMALE ][ AIM_RIFLE_STAND ]				= BGMSTANDAIM2;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_RIFLE_STAND ]			= BGMSTANDAIM2;
	gubAnimSurfaceIndex[ BIGMALE ][ END_RIFLE_STAND ]				= BGMSTANDAIM2;
	gubAnimSurfaceIndex[ BIGMALE ][ START_SWAT ]						= BGMSNEAKING;
	gubAnimSurfaceIndex[ BIGMALE ][ END_SWAT ]							= BGMSNEAKING;
	gubAnimSurfaceIndex[ BIGMALE ][ FLYBACK_HIT ]						= BGMHITHARD;
	gubAnimSurfaceIndex[ BIGMALE ][ READY_RIFLE_PRONE ]			= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ AIM_RIFLE_PRONE ]				= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_RIFLE_PRONE ]			= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ END_RIFLE_PRONE ]				= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLBACK_DEATHTWICH ]		= BGMHITHARD;
	gubAnimSurfaceIndex[ BIGMALE ][ GENERIC_HIT_STAND ]			= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ FLYBACK_HIT_BLOOD_STAND ]		= BGMHITHARDBLOOD;
	gubAnimSurfaceIndex[ BIGMALE ][ FLYBACK_HIT_DEATH ]			= BGMHITHARD;
	gubAnimSurfaceIndex[ BIGMALE ][ READY_RIFLE_CROUCH ]		= BGMCROUCHAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ AIM_RIFLE_CROUCH ]			= BGMCROUCHAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_RIFLE_CROUCH ]		= BGMCROUCHAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ END_RIFLE_CROUCH ]			= BGMCROUCHAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLBACK_HIT_STAND ]		= BGMHITFALLBACK;
	gubAnimSurfaceIndex[ BIGMALE ][ ROLLOVER ]							= BGMROLLOVER;
	gubAnimSurfaceIndex[ BIGMALE ][ CLIMBUPROOF ]						= BGMCLIMBROOF;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF ]								= BGMFALL;
	gubAnimSurfaceIndex[ BIGMALE ][ GETUP_FROM_ROLLOVER ]		= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ CLIMBDOWNROOF ]					= BGMCLIMBROOF;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLFORWARD_ROOF ]						= BGMFALLF;
	gubAnimSurfaceIndex[ BIGMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ GENERIC_HIT_DEATHTWITCHB ]		= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]	= BGMHITFALLBACK;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLBACK_HIT_DEATHTWITCHB ]		= BGMHITFALLBACK;
	gubAnimSurfaceIndex[ BIGMALE ][ GENERIC_HIT_DEATH ]			= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLBACK_HIT_DEATH ]		= BGMHITFALLBACK;
	gubAnimSurfaceIndex[ BIGMALE ][ GENERIC_HIT_CROUCH ]		= BGMHITCROUCH;
	gubAnimSurfaceIndex[ BIGMALE ][ STANDING_BURST ]				= BGMSTANDAIM2;
	gubAnimSurfaceIndex[ BIGMALE ][ STANDING_BURST_HIT ]		= BGMHITHARD;
	gubAnimSurfaceIndex[ BIGMALE ][ GENERIC_HIT_DEATH ]			= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLFORWARD_FROMHIT_STAND ]		= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLFORWARD_FROMHIT_STAND ]				= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLFORWARD_FROMHIT_CROUCH ]			= BGMHITCROUCH;
	gubAnimSurfaceIndex[ BIGMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]		= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ GENERIC_HIT_PRONE ]			= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_HIT_DEATH ]				= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_LAY_FROMHIT ]			= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_HIT_DEATHTWITCHNB ]					= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_HIT_DEATHTWITCHB ]					= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ FLYBACK_HITDEATH_STOP ]					= BGMHITHARD;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLFORWARD_HITDEATH_STOP ]			= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLBACK_HITDEATH_STOP ]				= BGMHITFALLBACK;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_HITDEATH_STOP ]						= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ FLYBACKHIT_STOP ]								= BGMHITHARD;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLBACKHIT_STOP ]							= BGMHITFALLBACK;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_STOP ]									= BGMFALL;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_FORWARD_STOP ]					= BGMFALLF;
	gubAnimSurfaceIndex[ BIGMALE ][ STAND_FALLFORWARD_STOP ]				= BGMHITSTAND;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_LAYFROMHIT_STOP ]					= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ HOPFENCE ]											= BGMHOPFENCE;
	gubAnimSurfaceIndex[ BIGMALE ][ PUNCH_BREATH ]									= BGMPUNCH;
	gubAnimSurfaceIndex[ BIGMALE ][ PUNCH ]													= BGMPUNCH;
	gubAnimSurfaceIndex[ BIGMALE ][ NOTHING_STAND ]									= BGMNOTHING_STD;
	gubAnimSurfaceIndex[ BIGMALE ][ JFK_HITDEATH ]									= BGMDIE_JFK;
	gubAnimSurfaceIndex[ BIGMALE ][ JFK_HITDEATH_STOP ]							= BGMDIE_JFK;
	gubAnimSurfaceIndex[ BIGMALE ][ JFK_HITDEATH_TWITCHB ]					= BGMDIE_JFK;
	gubAnimSurfaceIndex[ BIGMALE ][ FIRE_STAND_BURST_SPREAD ]				= BGMSTANDAIM2;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_DEATH ]									= BGMFALL;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_DEATH_STOP ]						= BGMFALL;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_TWITCHB ]								= BGMFALL;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_TWITCHNB ]							= BGMFALL;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_FORWARD_DEATH ]					= BGMFALLF;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_FORWARD_DEATH_STOP ]		= BGMFALLF;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_FORWARD_TWITCHB ]				= BGMFALLF;
	gubAnimSurfaceIndex[ BIGMALE ][ FALLOFF_FORWARD_TWITCHNB ]			= BGMFALLF;
	gubAnimSurfaceIndex[ BIGMALE ][ OPEN_STRUCT ]										= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ OPEN_DOOR ]											= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ PICKUP_ITEM ]										= BGMPICKUP;
	gubAnimSurfaceIndex[ BIGMALE ][ DROP_ITEM ]											= BGMPICKUP;
	gubAnimSurfaceIndex[ BIGMALE ][ SLICE ]													= BGMSLICE;
	gubAnimSurfaceIndex[ BIGMALE ][ STAB ]													= BGMSTAB;
	gubAnimSurfaceIndex[ BIGMALE ][ CROUCH_STAB ]										= BGMCSTAB;
	gubAnimSurfaceIndex[ BIGMALE ][ START_AID ]											= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ GIVING_AID ]										= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_AID ]												= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ DODGE_ONE ]											= BGMDODGE;
	gubAnimSurfaceIndex[ BIGMALE ][ READY_DUAL_STAND ]							= BGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ AIM_DUAL_STAND ]								= BGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_DUAL_STAND ]							= BGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ END_DUAL_STAND ]								= BGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ RAISE_RIFLE ]										= BGMRAISE;
	gubAnimSurfaceIndex[ BIGMALE ][ LOWER_RIFLE ]										= BGMRAISE;
	gubAnimSurfaceIndex[ BIGMALE ][ THROW_ITEM ]										= BGMTHROW;
	gubAnimSurfaceIndex[ BIGMALE ][ LOB_ITEM ]											= BGMLOB;
	gubAnimSurfaceIndex[ BIGMALE ][ BODYEXPLODING ]									= BODYEXPLODE;
	gubAnimSurfaceIndex[ BIGMALE ][ CROUCHED_BURST ]								= BGMCROUCHAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_BURST ]										= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ BIGBUY_FLEX ]										= BGMFLEX;
	gubAnimSurfaceIndex[ BIGMALE ][ BIGBUY_STRECH ]									= BGMSTRECH;
	gubAnimSurfaceIndex[ BIGMALE ][ BIGBUY_SHOEDUST ]								= BGMSHOEDUST;
	gubAnimSurfaceIndex[ BIGMALE ][ BIGBUY_HEADTURN ]								= BGMHEADTURN;
	gubAnimSurfaceIndex[ BIGMALE ][ GIVE_ITEM ]											= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ WATER_HIT ]											= BGMWATER_DIE;
	gubAnimSurfaceIndex[ BIGMALE ][ WATER_DIE]											= BGMWATER_DIE;
	gubAnimSurfaceIndex[ BIGMALE ][ WATER_DIE_STOP ]								= BGMWATER_DIE;
	gubAnimSurfaceIndex[ BIGMALE ][ HELIDROP ]											= BGMHELIDROP;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPEN_DOOR ]									= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPEN_LOCKED_DOOR ]					= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ KICK_DOOR ]											= BGMKICKDOOR;
	gubAnimSurfaceIndex[ BIGMALE ][ CLOSE_DOOR ]										= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ RIFLE_STAND_HIT ]								= BGMRHIT;
	gubAnimSurfaceIndex[ BIGMALE ][ DEEP_WATER_TRED ]								= BGMDEEPWATER_TRED;
	gubAnimSurfaceIndex[ BIGMALE ][ DEEP_WATER_SWIM ]								= BGMDEEPWATER_SWIM;
	gubAnimSurfaceIndex[ BIGMALE ][ DEEP_WATER_HIT ]								= BGMDEEPWATER_DIE;
	gubAnimSurfaceIndex[ BIGMALE ][ DEEP_WATER_DIE]									= BGMDEEPWATER_DIE;
	gubAnimSurfaceIndex[ BIGMALE ][ DEEP_WATER_DIE_STOPPING ]				= BGMWATER_DIE;
	gubAnimSurfaceIndex[ BIGMALE ][ DEEP_WATER_DIE_STOP ]						= BGMWATER_DIE;
	gubAnimSurfaceIndex[ BIGMALE ][ LOW_TO_DEEP_WATER ]							= BGMWATER_TRANS;
	gubAnimSurfaceIndex[ BIGMALE ][ DEEP_TO_LOW_WATER ]							= BGMWATER_TRANS;
	gubAnimSurfaceIndex[ BIGMALE ][ GOTO_SLEEP ]										= BGMSLEEP;
	gubAnimSurfaceIndex[ BIGMALE ][ SLEEPING ]											= BGMSLEEP;
	gubAnimSurfaceIndex[ BIGMALE ][ WKAEUP_FROM_SLEEP ]							= BGMSLEEP;
	gubAnimSurfaceIndex[ BIGMALE ][ FIRE_LOW_STAND ]								= BGMSHOOT_LOW;
	gubAnimSurfaceIndex[ BIGMALE ][ FIRE_BURST_LOW_STAND ]					= BGMSHOOT_LOW;
	gubAnimSurfaceIndex[ BIGMALE ][ STANDING_SHOOT_UNJAM ]					= BGMSTANDAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ CROUCH_SHOOT_UNJAM ]						= BGMCROUCHAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_SHOOT_UNJAM ]							= BGMPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ STANDING_SHOOT_DWEL_UNJAM ]		  = BGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ BIGMALE ][ STANDING_SHOOT_LOW_UNJAM ]		  = BGMSHOOT_LOW;
	gubAnimSurfaceIndex[ BIGMALE ][ READY_DUAL_CROUCH ]							= BGMCDBLSHOT;
	gubAnimSurfaceIndex[ BIGMALE ][ AIM_DUAL_CROUCH ]								= BGMCDBLSHOT;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_DUAL_CROUCH ]							= BGMCDBLSHOT;
	gubAnimSurfaceIndex[ BIGMALE ][ END_DUAL_CROUCH ]								= BGMCDBLSHOT;
	gubAnimSurfaceIndex[ BIGMALE ][ CROUCH_SHOOT_DWEL_UNJAM ]				= BGMCDBLSHOT;
	gubAnimSurfaceIndex[ BIGMALE ][ ADJACENT_GET_ITEM ]							= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ CUTTING_FENCE ]									= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ FROM_INJURED_TRANSITION ]				= BGMHURTTRANS;
	gubAnimSurfaceIndex[ BIGMALE ][ THROW_KNIFE ]										= BGMTHROWKNIFE;
	gubAnimSurfaceIndex[ BIGMALE ][ KNIFE_BREATH ]									= BGMBREATHKNIFE;
	gubAnimSurfaceIndex[ BIGMALE ][ KNIFE_GOTOBREATH ]							= BGMBREATHKNIFE;
	gubAnimSurfaceIndex[ BIGMALE ][ KNIFE_ENDBREATH ]								= BGMBREATHKNIFE;
	gubAnimSurfaceIndex[ BIGMALE ][ CATCH_STANDING ]								= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ CATCH_CROUCHED ]								= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ PLANT_BOMB ]										= BGMPICKUP;
	gubAnimSurfaceIndex[ BIGMALE ][ USE_REMOTE ]										= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ START_COWER ]										= BGMCOWER;
	gubAnimSurfaceIndex[ BIGMALE ][ COWERING ]											= BGMCOWER;
	gubAnimSurfaceIndex[ BIGMALE ][ END_COWER ]											= BGMCOWER;
	gubAnimSurfaceIndex[ BIGMALE ][ STEAL_ITEM ]										= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_ROCKET ]									= BGMROCKET;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_MORTAR ]									= BGMMORTAR;
	gubAnimSurfaceIndex[ BIGMALE ][ SIDE_STEP ]											= BGMSIDESTEP;
	gubAnimSurfaceIndex[ BIGMALE ][ WALK_BACKWARDS ]								= BGMWALKING;
	gubAnimSurfaceIndex[ BIGMALE ][ BEGIN_OPENSTRUCT ]							= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPENSTRUCT ]								= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPENSTRUCT_LOCKED ]					= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ PUNCH_LOW ]											= BGMPUNCHLOW;
	gubAnimSurfaceIndex[ BIGMALE ][ PISTOL_SHOOT_LOW ]							= BGMPISTOLSHOOTLOW;
	gubAnimSurfaceIndex[ BIGMALE ][ DECAPITATE ]										= BGMCSTAB;
	gubAnimSurfaceIndex[ BIGMALE ][ BIGMERC_IDLE_NECK ]							= BGMIDLENECK;
	gubAnimSurfaceIndex[ BIGMALE ][ BIGMERC_CROUCH_TRANS_INTO ]			= BGMCROUCHTRANS;
	gubAnimSurfaceIndex[ BIGMALE ][ BIGMERC_CROUCH_TRANS_OUTOF ]		= BGMCROUCHTRANS;
	gubAnimSurfaceIndex[ BIGMALE ][ GOTO_PATIENT ]									= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ BEING_PATIENT ]									= BGMHITPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ GOTO_DOCTOR ]										= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ BEING_DOCTOR ]									= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_DOCTOR ]										= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ GOTO_REPAIRMAN ]								= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ BEING_REPAIRMAN ]								= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_REPAIRMAN ]									= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ FALL_INTO_PIT ]									= BGMFALL;
	gubAnimSurfaceIndex[ BIGMALE ][ RELOAD_ROBOT ]									= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_CATCH ]											= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ END_CROUCH_CATCH ]							= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ AI_RADIO ]											= BGMRADIO;
	gubAnimSurfaceIndex[ BIGMALE ][ AI_CR_RADIO ]										= BGMCRRADIO;
	gubAnimSurfaceIndex[ BIGMALE ][ TAKE_BLOOD_FROM_CORPSE ]				= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ CHARIOTS_OF_FIRE ]							= RGMBURN;
	gubAnimSurfaceIndex[ BIGMALE ][ AI_PULL_SWITCH ]								= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ MERC_HURT_IDLE_ANIM ]						= BGMHURTSTANDINGR;
	gubAnimSurfaceIndex[ BIGMALE ][ END_HURT_WALKING ]							= BGMHURTSTANDINGR;
	gubAnimSurfaceIndex[ BIGMALE ][ PASS_OBJECT ]										= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ DROP_ADJACENT_OBJECT ]					= BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ READY_DUAL_PRONE ]							= BGMDWPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ AIM_DUAL_PRONE ]								= BGMDWPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ SHOOT_DUAL_PRONE ]							= BGMDWPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ END_DUAL_PRONE ]								= BGMDWPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PRONE_SHOOT_DWEL_UNJAM ]			  = BGMDWPRONE;
	gubAnimSurfaceIndex[ BIGMALE ][ PICK_LOCK ]										  = BGMOPEN;
	gubAnimSurfaceIndex[ BIGMALE ][ OPEN_DOOR_CROUCHED ]						= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ BEGIN_OPENSTRUCT_CROUCHED ]			= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ CLOSE_DOOR_CROUCHED ]						= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ OPEN_STRUCT_CROUCHED ]					= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPEN_DOOR_CROUCHED ]				= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPENSTRUCT_CROUCHED ]				= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPEN_LOCKED_DOOR_CROUCHED ]	= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ END_OPENSTRUCT_LOCKED_CROUCHED ]= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ DRUNK_IDLE ]										= BGMDRUNK;
	gubAnimSurfaceIndex[ BIGMALE ][ CROWBAR_ATTACK ]								= BGMCROWBAR;
	gubAnimSurfaceIndex[ BIGMALE ][ ATTACH_CAN_TO_STRING ]					= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ SWAT_BACKWARDS ]								= BGMSNEAKING;
	gubAnimSurfaceIndex[ BIGMALE ][ JUMP_OVER_BLOCKING_PERSON ]			= BGMJUMPOVER;
	gubAnimSurfaceIndex[ BIGMALE ][ REFUEL_VEHICLE ]								= BGMMEDIC;
	gubAnimSurfaceIndex[ BIGMALE ][ LOCKPICK_CROUCHED ]					  	= BGMMEDIC;


	gubAnimSurfaceItemSubIndex[ BIGMALE ][ STANDING ]						= BGMPISTOLBREATH;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ WALKING ]							= BGMNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ RUNNING ]							= BGMNOTHING_RUN;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ SWATTING ]						= BGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ START_SWAT ]					= BGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ END_SWAT ]						= BGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ KNEEL_DOWN ]					= BGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ CROUCHING ]						= BGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ KNEEL_UP ]						= BGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ READY_RIFLE_STAND ]				= BGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ AIM_RIFLE_STAND ]					= BGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ SHOOT_RIFLE_STAND ]				= BGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ END_RIFLE_STAND ]					= BGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ STANDING_BURST ]						= BGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ FIRE_STAND_BURST_SPREAD ]	= BGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ READY_RIFLE_CROUCH ]				= BGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ AIM_RIFLE_CROUCH ]					= BGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ SHOOT_RIFLE_CROUCH ]				= BGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ END_RIFLE_CROUCH ]					= BGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ PRONE_DOWN ]								= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ CRAWLING ]									= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ PRONE_UP ]									= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ PRONE ]										= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ READY_RIFLE_PRONE ]				= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ AIM_RIFLE_PRONE ]					= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ SHOOT_RIFLE_PRONE ]				= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ END_RIFLE_PRONE ]					= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ GETUP_FROM_ROLLOVER ]			= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ STANDING_SHOOT_UNJAM ]			= BGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ CROUCH_SHOOT_UNJAM ]				= BGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ PRONE_SHOOT_UNJAM ]				= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ FIRE_BURST_LOW_STAND ]			= BGMPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ FIRE_LOW_STAND ]						= BGMPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ CROUCHED_BURST ]						= BGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ PRONE_BURST ]							= BGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ MERC_HURT_IDLE_ANIM ]			= BGMHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ END_HURT_WALKING ]					= BGMHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ WALK_BACKWARDS ]						= BGMNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ BIGMALE ][ DRUNK_IDLE ]								= BGMPISTOLDRUNK;


	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ STANDING][0]									= BGMWATER_R_STD;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ WALKING ][0]									= BGMWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ RUNNING ][0]									= BGMWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ READY_RIFLE_STAND ][0]				= BGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ AIM_RIFLE_STAND ][0]					= BGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ SHOOT_RIFLE_STAND ][0]				= BGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ END_RIFLE_STAND ][0]					= BGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ STANDING_BURST ][0]					= BGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ FIRE_STAND_BURST_SPREAD ][0]	= BGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ STANDING][1]									= BGMWATER_N_STD;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ WALKING ][1]									= BGMWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ RUNNING ][1]									= BGMWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ READY_RIFLE_STAND ][1]				= BGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ AIM_RIFLE_STAND ][1]					= BGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ SHOOT_RIFLE_STAND ][1]				= BGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ END_RIFLE_STAND ][1]					= BGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ STANDING_BURST ][1]					= BGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ FIRE_STAND_BURST_SPREAD ][1]	= BGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ READY_DUAL_STAND ][1]				= BGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ AIM_DUAL_STAND ][1]					= BGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ SHOOT_DUAL_STAND ][1]				= BGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ END_DUAL_STAND ][1]					= BGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ STANDING_SHOOT_UNJAM ][0]		= BGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ STANDING_SHOOT_UNJAM ][1]		= BGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ THROW_ITEM ][0]							= BGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ THROW_ITEM ][1]							= BGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ LOB_ITEM ][0]								= BGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ BIGMALE ][ LOB_ITEM ][1]								= BGMWATERTHROW;


	//Setup some random stuff
	gRandomAnimDefs[ BIGMALE ][ 0 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ BIGMALE ][ 0 ].sAnimID							= BIGBUY_FLEX;
	gRandomAnimDefs[ BIGMALE ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ BIGMALE ][ 0 ].ubEndRoll						= 3;
	gRandomAnimDefs[ BIGMALE ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ BIGMALE ][ 0 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ BIGMALE ][ 1 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ BIGMALE ][ 1 ].sAnimID							= BIGBUY_STRECH;
	gRandomAnimDefs[ BIGMALE ][ 1 ].ubStartRoll					= 10;
	gRandomAnimDefs[ BIGMALE ][ 1 ].ubEndRoll						= 13;
	gRandomAnimDefs[ BIGMALE ][ 1 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ BIGMALE ][ 1 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ BIGMALE ][ 2 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ BIGMALE ][ 2 ].sAnimID							= BIGBUY_SHOEDUST;
	gRandomAnimDefs[ BIGMALE ][ 2 ].ubStartRoll					= 20;
	gRandomAnimDefs[ BIGMALE ][ 2 ].ubEndRoll						= 23;
	gRandomAnimDefs[ BIGMALE ][ 2 ].ubFlags							= RANDOM_ANIM_CASUAL | RANDOM_ANIM_FIRSTBIGMERC;
	gRandomAnimDefs[ BIGMALE ][ 2 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ BIGMALE ][ 3 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ BIGMALE ][ 3 ].sAnimID							= BIGBUY_HEADTURN;
	gRandomAnimDefs[ BIGMALE ][ 3 ].ubStartRoll					= 30;
	gRandomAnimDefs[ BIGMALE ][ 3 ].ubEndRoll						= 33;
	gRandomAnimDefs[ BIGMALE ][ 3 ].ubFlags							= RANDOM_ANIM_FIRSTBIGMERC;
	gRandomAnimDefs[ BIGMALE ][ 3 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ BIGMALE ][ 4 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ BIGMALE ][ 4 ].sAnimID							= BIGMERC_IDLE_NECK;
	gRandomAnimDefs[ BIGMALE ][ 4 ].ubStartRoll					= 30;
	gRandomAnimDefs[ BIGMALE ][ 4 ].ubEndRoll						= 36;
	gRandomAnimDefs[ BIGMALE ][ 4 ].ubFlags							= RANDOM_ANIM_SECONDBIGMERC;
	gRandomAnimDefs[ BIGMALE ][ 4 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ BIGMALE ][ 5 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ BIGMALE ][ 5 ].sAnimID							= MERC_HURT_IDLE_ANIM;
	gRandomAnimDefs[ BIGMALE ][ 5 ].ubStartRoll					= 40;
	gRandomAnimDefs[ BIGMALE ][ 5 ].ubEndRoll						= 100;
	gRandomAnimDefs[ BIGMALE ][ 5 ].ubFlags							= RANDOM_ANIM_INJURED;
	gRandomAnimDefs[ BIGMALE ][ 5 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ BIGMALE ][ 6 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ BIGMALE ][ 6 ].sAnimID							= DRUNK_IDLE;
	gRandomAnimDefs[ BIGMALE ][ 6 ].ubStartRoll					= 40;
	gRandomAnimDefs[ BIGMALE ][ 6 ].ubEndRoll						= 100;
	gRandomAnimDefs[ BIGMALE ][ 6 ].ubFlags							= RANDOM_ANIM_DRUNK;
	gRandomAnimDefs[ BIGMALE ][ 6 ].ubAnimHeight				= ANIM_STAND;


	gubAnimSurfaceCorpseID[ BIGMALE ][ GENERIC_HIT_DEATH ]						= MMERC_FWD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FALLBACK_HIT_DEATH ]						= MMERC_BCK;
	gubAnimSurfaceCorpseID[ BIGMALE ][ PRONE_HIT_DEATH ]							= MMERC_PRN;
	gubAnimSurfaceCorpseID[ BIGMALE ][ WATER_DIE ]										= MMERC_WTR;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FLYBACK_HIT_DEATH ]						= MMERC_DHD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ JFK_HITDEATH ]									= MMERC_JFK;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FALLOFF_DEATH ]								= MMERC_FALL;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FALLOFF_FORWARD_DEATH ]				= MMERC_FALLF;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FLYBACK_HIT ]									= MMERC_DHD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= MMERC_FWD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FALLFORWARD_FROMHIT_STAND ]		= MMERC_FWD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]= MMERC_FWD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FALLBACK_HIT_STAND ]						= MMERC_BCK;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]		= MMERC_BCK;
	gubAnimSurfaceCorpseID[ BIGMALE ][ PRONE_HIT_DEATHTWITCHNB ]			= MMERC_PRN;
	gubAnimSurfaceCorpseID[ BIGMALE ][ PRONE_LAY_FROMHIT ]						= MMERC_PRN;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLOFF ]											= MMERC_FALLF;
	gubAnimSurfaceCorpseID[ REGMALE ][ FALLFORWARD_ROOF ]							= MMERC_FALL;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FLYBACKHIT_STOP ]							= MMERC_DHD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ STAND_FALLFORWARD_STOP ]				= MMERC_FWD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ FALLBACKHIT_STOP ]							= MMERC_BCK;
	gubAnimSurfaceCorpseID[ BIGMALE ][ PRONE_LAYFROMHIT_STOP ]				= MMERC_PRN;
	gubAnimSurfaceCorpseID[ BIGMALE ][ CHARIOTS_OF_FIRE ]							= BURNT_DEAD;
	gubAnimSurfaceCorpseID[ BIGMALE ][ BODYEXPLODING ]								= EXPLODE_DEAD;


	// STOCKY MALE GUY
	gubAnimSurfaceIndex[ STOCKYMALE ][ WALKING ]								= RGMBASICWALKING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STANDING ]							= RGMSTANDING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ KNEEL_DOWN ]						= RGMCROUCHING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CROUCHING ]							= RGMCROUCHING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ KNEEL_UP ]							= RGMCROUCHING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SWATTING ]							= RGMSNEAKING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ RUNNING ]								= RGMRUNNING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_DOWN ]						= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CRAWLING ]							= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_UP ]							= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE ]									= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ READY_RIFLE_STAND ]			= RGMSTANDAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AIM_RIFLE_STAND ]				= RGMSTANDAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_RIFLE_STAND ]			= RGMSTANDAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_RIFLE_STAND ]				= RGMSTANDAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ START_SWAT ]						= RGMSNEAKING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_SWAT ]							= RGMSNEAKING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FLYBACK_HIT ]						= RGMHITHARD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ READY_RIFLE_PRONE ]			= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AIM_RIFLE_PRONE ]				= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_RIFLE_PRONE ]			= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_RIFLE_PRONE ]				= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLBACK_DEATHTWICH ]		= RGMHITHARD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GENERIC_HIT_STAND ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FLYBACK_HIT_BLOOD_STAND ]		= RGMHITHARDBLOOD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FLYBACK_HIT_DEATH ]			= RGMHITHARD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ READY_RIFLE_CROUCH ]		= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AIM_RIFLE_CROUCH ]			= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_RIFLE_CROUCH ]		= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_RIFLE_CROUCH ]			= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLBACK_HIT_STAND ]		= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ ROLLOVER ]							= RGMROLLOVER;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CLIMBUPROOF ]						= RGMCLIMBROOF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF ]								= RGMFALL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GETUP_FROM_ROLLOVER ]		= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CLIMBDOWNROOF ]					= RGMCLIMBROOF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLFORWARD_ROOF ]						= RGMFALLF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GENERIC_HIT_DEATHTWITCHB ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]	= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLBACK_HIT_DEATHTWITCHB ]		= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GENERIC_HIT_DEATH ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLBACK_HIT_DEATH ]		= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GENERIC_HIT_CROUCH ]		= RGMHITCROUCH;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STANDING_BURST ]				= RGMSTANDAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STANDING_BURST_HIT ]		= RGMHITHARD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GENERIC_HIT_DEATH ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLFORWARD_FROMHIT_STAND ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLFORWARD_FROMHIT_STAND ]				= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLFORWARD_FROMHIT_CROUCH ]			= RGMHITCROUCH;
	gubAnimSurfaceIndex[ STOCKYMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]		= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GENERIC_HIT_PRONE ]			= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_HIT_DEATH ]				= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_LAY_FROMHIT ]			= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_HIT_DEATHTWITCHNB ]					= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_HIT_DEATHTWITCHB ]					= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FLYBACK_HITDEATH_STOP ]					= RGMHITHARD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLFORWARD_HITDEATH_STOP ]			= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLBACK_HITDEATH_STOP ]				= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_HITDEATH_STOP ]						= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FLYBACKHIT_STOP ]								= RGMHITHARD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLBACKHIT_STOP ]							= RGMHITFALLBACK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_STOP ]									= RGMFALL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_FORWARD_STOP ]					= RGMFALLF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STAND_FALLFORWARD_STOP ]				= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_LAYFROMHIT_STOP ]					= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ HOPFENCE ]											= RGMHOPFENCE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PUNCH_BREATH ]									= RGMPUNCH;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PUNCH ]												= RGMPUNCH;
	gubAnimSurfaceIndex[ STOCKYMALE ][ NOTHING_STAND ]								= RGMNOTHING_STD;
	gubAnimSurfaceIndex[ STOCKYMALE ][ JFK_HITDEATH ]									= RGMDIE_JFK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ JFK_HITDEATH_STOP ]							= RGMDIE_JFK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ JFK_HITDEATH_TWITCHB ]					= RGMDIE_JFK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FIRE_STAND_BURST_SPREAD ]			= RGMSTANDAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_DEATH ]								= RGMFALL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_DEATH_STOP ]						= RGMFALL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_TWITCHB ]							= RGMFALL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_TWITCHNB ]							= RGMFALL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_FORWARD_DEATH ]				= RGMFALLF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_FORWARD_DEATH_STOP ]		= RGMFALLF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_FORWARD_TWITCHB ]			= RGMFALLF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALLOFF_FORWARD_TWITCHNB ]			= RGMFALLF;
	gubAnimSurfaceIndex[ STOCKYMALE ][ OPEN_DOOR ]											= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ OPEN_STRUCT ]										= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PICKUP_ITEM ]										= RGMPICKUP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DROP_ITEM ]											= RGMPICKUP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SLICE ]													= RGMSLICE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STAB ]														= RGMSTAB;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CROUCH_STAB ]										= RGMCSTAB;
	gubAnimSurfaceIndex[ STOCKYMALE ][ START_AID ]											= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GIVING_AID ]										= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_AID ]												= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DODGE_ONE ]											= RGMDODGE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ READY_DUAL_STAND ]								= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AIM_DUAL_STAND ]									= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_DUAL_STAND ]								= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_DUAL_STAND ]									= RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ RAISE_RIFLE ]										= RGMRAISE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ LOWER_RIFLE ]										= RGMRAISE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ BODYEXPLODING ]									= BODYEXPLODE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ THROW_ITEM ]											= RGMTHROW;
	gubAnimSurfaceIndex[ STOCKYMALE ][ LOB_ITEM ]												= RGMLOB;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CROUCHED_BURST ]									= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_BURST ]										= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GIVE_ITEM ]											= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ WATER_HIT ]											= RGMWATER_DIE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ WATER_DIE]												= RGMWATER_DIE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ WATER_DIE_STOP ]									= RGMWATER_DIE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ HELIDROP ]												= RGMHELIDROP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FEM_LOOK ]												= RGM_LOOK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ REG_SQUISH ]											= RGM_SQUISH;
	gubAnimSurfaceIndex[ STOCKYMALE ][ REG_PULL ]												= RGM_PULL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ REG_SPIT ]												= RGM_SPIT;
	gubAnimSurfaceIndex[ STOCKYMALE ][ NINJA_GOTOBREATH ]								= RGMLOWKICK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ NINJA_BREATH ]										= RGMLOWKICK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ NINJA_LOWKICK ]									= RGMLOWKICK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ NINJA_PUNCH ]										= RGMNPUNCH;
	gubAnimSurfaceIndex[ STOCKYMALE ][ NINJA_SPINKICK ]									= RGMSPINKICK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPEN_DOOR ]									= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPEN_LOCKED_DOOR ]						= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ KICK_DOOR ]											= RGMKICKDOOR;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CLOSE_DOOR ]											= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ RIFLE_STAND_HIT ]								= RGMRHIT;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DEEP_WATER_TRED ]								= RGMDEEPWATER_TRED;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DEEP_WATER_SWIM ]								= RGMDEEPWATER_SWIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DEEP_WATER_HIT ]									= RGMDEEPWATER_DIE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DEEP_WATER_DIE]									= RGMDEEPWATER_DIE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DEEP_WATER_DIE_STOPPING ]				= RGMWATER_DIE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DEEP_WATER_DIE_STOP ]						= RGMWATER_DIE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ LOW_TO_DEEP_WATER ]							= RGMWATER_TRANS;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DEEP_TO_LOW_WATER ]							= RGMWATER_TRANS;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GOTO_SLEEP ]											= RGMSLEEP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SLEEPING ]												= RGMSLEEP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ WKAEUP_FROM_SLEEP ]							= RGMSLEEP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FIRE_LOW_STAND ]									= RGMSHOOT_LOW;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FIRE_BURST_LOW_STAND ]						= RGMSHOOT_LOW;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STANDING_SHOOT_UNJAM ]						= RGMSTANDAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CROUCH_SHOOT_UNJAM ]							= RGMCROUCHAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_SHOOT_UNJAM ]							= RGMPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STANDING_SHOOT_DWEL_UNJAM ]		  = RGMSTANDDWALAIM;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STANDING_SHOOT_LOW_UNJAM ]				= RGMSHOOT_LOW;
	gubAnimSurfaceIndex[ STOCKYMALE ][ READY_DUAL_CROUCH ]							= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AIM_DUAL_CROUCH ]								= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_DUAL_CROUCH ]							= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_DUAL_CROUCH ]								= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CROUCH_SHOOT_DWEL_UNJAM ]				= RGMCDBLSHOT;
	gubAnimSurfaceIndex[ STOCKYMALE ][ ADJACENT_GET_ITEM ]							= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CUTTING_FENCE ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FROM_INJURED_TRANSITION ]				= RGMHURTTRANS;
	gubAnimSurfaceIndex[ STOCKYMALE ][ THROW_KNIFE ]										= RGMTHROWKNIFE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ KNIFE_BREATH ]										= RGMBREATHKNIFE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ KNIFE_GOTOBREATH ]								= RGMBREATHKNIFE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ KNIFE_ENDBREATH ]								= RGMBREATHKNIFE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CATCH_STANDING ]									= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CATCH_CROUCHED ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PLANT_BOMB ]											= RGMPICKUP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ USE_REMOTE ]											= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ START_COWER ]										= RGMCOWER;
	gubAnimSurfaceIndex[ STOCKYMALE ][ COWERING ]												= RGMCOWER;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_COWER ]											= RGMCOWER;
	gubAnimSurfaceIndex[ STOCKYMALE ][ STEAL_ITEM ]											= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_ROCKET ]										= RGMROCKET;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_MORTAR ]										= RGMMORTAR;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SIDE_STEP ]											= RGMSIDESTEP;
	gubAnimSurfaceIndex[ STOCKYMALE ][ WALK_BACKWARDS ]									= RGMBASICWALKING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ BEGIN_OPENSTRUCT ]								= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPENSTRUCT ]									= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPENSTRUCT_LOCKED ]					= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PUNCH_LOW ]											= RGMPUNCHLOW;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PISTOL_SHOOT_LOW ]								= RGMPISTOLSHOOTLOW;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DECAPITATE ]											= RGMCSTAB;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GOTO_PATIENT ]										= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ BEING_PATIENT ]									= RGMHITPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GOTO_DOCTOR ]										= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ BEING_DOCTOR ]										= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_DOCTOR ]											= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ GOTO_REPAIRMAN ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ BEING_REPAIRMAN ]								= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_REPAIRMAN ]									= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ FALL_INTO_PIT ]									= RGMFALL;
	gubAnimSurfaceIndex[ STOCKYMALE ][ RELOAD_ROBOT ]										= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_CATCH ]											= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_CROUCH_CATCH ]								= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AI_RADIO ]												= RGMRADIO;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AI_CR_RADIO ]										= RGMCRRADIO;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SLAP_HIT ]												= RGMHITSTAND;
	gubAnimSurfaceIndex[ STOCKYMALE ][ TAKE_BLOOD_FROM_CORPSE ]					= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CHARIOTS_OF_FIRE ]								= RGMBURN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AI_PULL_SWITCH ]									= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ MERC_HURT_IDLE_ANIM ]						= RGMHURTSTANDINGR;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_HURT_WALKING ]								= RGMHURTSTANDINGR;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PASS_OBJECT ]										= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DROP_ADJACENT_OBJECT ]						= RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ READY_DUAL_PRONE ]								= RGMDWPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ AIM_DUAL_PRONE ]									= RGMDWPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SHOOT_DUAL_PRONE ]								= RGMDWPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_DUAL_PRONE ]									= RGMDWPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PRONE_SHOOT_DWEL_UNJAM ]				  = RGMDWPRONE;
	gubAnimSurfaceIndex[ STOCKYMALE ][ PICK_LOCK ]										  = RGMOPEN;
	gubAnimSurfaceIndex[ STOCKYMALE ][ OPEN_DOOR_CROUCHED ]						= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ BEGIN_OPENSTRUCT_CROUCHED ]			= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CLOSE_DOOR_CROUCHED ]						= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ OPEN_STRUCT_CROUCHED ]					= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPEN_DOOR_CROUCHED ]				= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPENSTRUCT_CROUCHED ]				= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPEN_LOCKED_DOOR_CROUCHED ]	= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ END_OPENSTRUCT_LOCKED_CROUCHED ]= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ DRUNK_IDLE ]											= RGMDRUNK;
	gubAnimSurfaceIndex[ STOCKYMALE ][ CROWBAR_ATTACK ]								= RGMCROWBAR;
	gubAnimSurfaceIndex[ STOCKYMALE ][ ATTACH_CAN_TO_STRING ]					= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ SWAT_BACKWARDS ]								= RGMSNEAKING;
	gubAnimSurfaceIndex[ STOCKYMALE ][ JUMP_OVER_BLOCKING_PERSON ]		= RGMJUMPOVER;
	gubAnimSurfaceIndex[ STOCKYMALE ][ REFUEL_VEHICLE ]								= RGMMEDIC;
	gubAnimSurfaceIndex[ STOCKYMALE ][ LOCKPICK_CROUCHED ]					  = RGMMEDIC;


	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ STANDING ]						= RGMPISTOLBREATH;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ WALKING ]						= RGMNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ RUNNING]							= RGMNOTHING_RUN;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ SWATTING]						= RGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ START_SWAT ]					= RGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ END_SWAT]						= RGMNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ KNEEL_DOWN ]					= RGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ CROUCHING ]					= RGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ KNEEL_UP ]						= RGMNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ READY_RIFLE_STAND ]				= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ AIM_RIFLE_STAND ]					= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ SHOOT_RIFLE_STAND ]				= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ END_RIFLE_STAND ]					= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ STANDING_BURST ]						= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ FIRE_STAND_BURST_SPREAD ]	= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ READY_RIFLE_CROUCH ]				= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ AIM_RIFLE_CROUCH ]					= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ SHOOT_RIFLE_CROUCH ]				= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ END_RIFLE_CROUCH ]					= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ PRONE_DOWN ]								= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ CRAWLING ]									= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ PRONE_UP ]									= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ PRONE ]										= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ READY_RIFLE_PRONE ]				= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ AIM_RIFLE_PRONE ]					= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ SHOOT_RIFLE_PRONE ]				= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ END_RIFLE_PRONE ]					= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ GETUP_FROM_ROLLOVER ]			= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ STANDING_SHOOT_UNJAM ]		= RGMHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ CROUCH_SHOOT_UNJAM ]			= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ PRONE_SHOOT_UNJAM ]				= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ FIRE_BURST_LOW_STAND ]		= RGMPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ FIRE_LOW_STAND ]					= RGMPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ CROUCHED_BURST ]					= RGMHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ PRONE_BURST ]							= RGMHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ MERC_HURT_IDLE_ANIM ]			= RGMHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ END_HURT_WALKING ]				= RGMHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ WALK_BACKWARDS ]					= RGMNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ STOCKYMALE ][ DRUNK_IDLE ]							= RGMPISTOLDRUNK;


	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ STANDING][0]									= RGMWATER_R_STD;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ WALKING ][0]									= RGMWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ RUNNING ][0]									= RGMWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ READY_RIFLE_STAND ][0]				= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ AIM_RIFLE_STAND ][0]					= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ SHOOT_RIFLE_STAND ][0]				= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ END_RIFLE_STAND ][0]					= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ STANDING_BURST ][0]					= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ FIRE_STAND_BURST_SPREAD ][0]	= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ STANDING][1]									= RGMWATER_N_STD;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ WALKING ][1]									= RGMWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ RUNNING ][1]									= RGMWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ READY_RIFLE_STAND ][1]				= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ AIM_RIFLE_STAND ][1]					= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ SHOOT_RIFLE_STAND ][1]				= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ END_RIFLE_STAND ][1]					= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ STANDING_BURST ][1]					= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ FIRE_STAND_BURST_SPREAD ][1]	= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ READY_DUAL_STAND ][1]				= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ AIM_DUAL_STAND ][1]					= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ SHOOT_DUAL_STAND ][1]				= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ END_DUAL_STAND ][1]					= RGMWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ STANDING_SHOOT_UNJAM ][0]		= RGMWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ STANDING_SHOOT_UNJAM ][1]		= RGMWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ THROW_ITEM ][0]							= RGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ THROW_ITEM ][1]							= RGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ LOB_ITEM ][0]								= RGMWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ STOCKYMALE ][ LOB_ITEM ][1]								= RGMWATERTHROW;



	gubAnimSurfaceCorpseID[ STOCKYMALE ][ GENERIC_HIT_DEATH ]						= SMERC_FWD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLBACK_HIT_DEATH ]					= SMERC_BCK;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ PRONE_HIT_DEATH ]							= SMERC_PRN;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ WATER_DIE ]										= SMERC_WTR;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FLYBACK_HIT_DEATH ]						= SMERC_DHD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ JFK_HITDEATH ]								= SMERC_JFK;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLOFF_DEATH ]								= SMERC_FALL;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLOFF_FORWARD_DEATH ]				= SMERC_FALLF;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FLYBACK_HIT ]									= SMERC_DHD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= SMERC_FWD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLFORWARD_FROMHIT_STAND ]		= SMERC_FWD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]= SMERC_FWD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLBACK_HIT_STAND ]						= SMERC_BCK;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]		= SMERC_BCK;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ PRONE_HIT_DEATHTWITCHNB ]			= SMERC_PRN;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ PRONE_LAY_FROMHIT ]						= SMERC_PRN;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLOFF ]											= SMERC_FALLF;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLFORWARD_ROOF ]						= SMERC_FALL;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FLYBACKHIT_STOP ]							= SMERC_DHD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ STAND_FALLFORWARD_STOP ]			= SMERC_FWD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ FALLBACKHIT_STOP ]						= SMERC_BCK;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ PRONE_LAYFROMHIT_STOP ]				= SMERC_PRN;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ CHARIOTS_OF_FIRE ]						= BURNT_DEAD;
	gubAnimSurfaceCorpseID[ STOCKYMALE ][ BODYEXPLODING ]								= EXPLODE_DEAD;


	//Setup some random stuff
	gRandomAnimDefs[ STOCKYMALE ][ 0 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ STOCKYMALE ][ 0 ].sAnimID							= REG_SPIT;
	gRandomAnimDefs[ STOCKYMALE ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ STOCKYMALE ][ 0 ].ubEndRoll						= 3;
	gRandomAnimDefs[ STOCKYMALE ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ STOCKYMALE ][ 0 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ STOCKYMALE ][ 1 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ STOCKYMALE ][ 1 ].sAnimID							= REG_SQUISH;
	gRandomAnimDefs[ STOCKYMALE ][ 1 ].ubStartRoll					= 10;
	gRandomAnimDefs[ STOCKYMALE ][ 1 ].ubEndRoll						= 13;
	gRandomAnimDefs[ STOCKYMALE ][ 1 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ STOCKYMALE ][ 1 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ STOCKYMALE ][ 2 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ STOCKYMALE ][ 2 ].sAnimID							= FEM_LOOK;
	gRandomAnimDefs[ STOCKYMALE ][ 2 ].ubStartRoll					= 20;
	gRandomAnimDefs[ STOCKYMALE ][ 2 ].ubEndRoll						= 23;
	gRandomAnimDefs[ STOCKYMALE ][ 2 ].ubFlags							= 0;
	gRandomAnimDefs[ STOCKYMALE ][ 2 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ STOCKYMALE ][ 3 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ STOCKYMALE ][ 3 ].sAnimID							= REG_PULL;
	gRandomAnimDefs[ STOCKYMALE ][ 3 ].ubStartRoll					= 30;
	gRandomAnimDefs[ STOCKYMALE ][ 3 ].ubEndRoll						= 33;
	gRandomAnimDefs[ STOCKYMALE ][ 3 ].ubFlags							= 0;
	gRandomAnimDefs[ STOCKYMALE ][ 3 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ STOCKYMALE ][ 4 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ STOCKYMALE ][ 4 ].sAnimID							= MERC_HURT_IDLE_ANIM;
	gRandomAnimDefs[ STOCKYMALE ][ 4 ].ubStartRoll					= 40;
	gRandomAnimDefs[ STOCKYMALE ][ 4 ].ubEndRoll						= 100;
	gRandomAnimDefs[ STOCKYMALE ][ 4 ].ubFlags							= RANDOM_ANIM_INJURED;
	gRandomAnimDefs[ STOCKYMALE ][ 4 ].ubAnimHeight					= ANIM_STAND;

	gRandomAnimDefs[ STOCKYMALE ][ 5 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ STOCKYMALE ][ 5 ].sAnimID							= DRUNK_IDLE;
	gRandomAnimDefs[ STOCKYMALE ][ 5 ].ubStartRoll					= 40;
	gRandomAnimDefs[ STOCKYMALE ][ 5 ].ubEndRoll						= 100;
	gRandomAnimDefs[ STOCKYMALE ][ 5 ].ubFlags							= RANDOM_ANIM_DRUNK;
	gRandomAnimDefs[ STOCKYMALE ][ 5 ].ubAnimHeight					= ANIM_STAND;


	// REG FEMALE GUY
	gubAnimSurfaceIndex[ REGFEMALE ][ WALKING ]								= RGFWALKING;
	gubAnimSurfaceIndex[ REGFEMALE ][ STANDING ]							= RGFSTANDING;
	gubAnimSurfaceIndex[ REGFEMALE ][ KNEEL_DOWN ]						= RGFCROUCHING;
	gubAnimSurfaceIndex[ REGFEMALE ][ CROUCHING ]							= RGFCROUCHING;
	gubAnimSurfaceIndex[ REGFEMALE ][ KNEEL_UP ]							= RGFCROUCHING;
	gubAnimSurfaceIndex[ REGFEMALE ][ SWATTING ]							= RGFSNEAKING;
	gubAnimSurfaceIndex[ REGFEMALE ][ RUNNING ]								= RGFRUNNING;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_DOWN ]						= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ CRAWLING ]							= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_UP ]							= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE ]									= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ READY_RIFLE_STAND ]			= RGFSTANDAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ AIM_RIFLE_STAND ]				= RGFSTANDAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_RIFLE_STAND ]			= RGFSTANDAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_RIFLE_STAND ]				= RGFSTANDAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ START_SWAT ]						= RGFSNEAKING;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_SWAT ]							= RGFSNEAKING;
	gubAnimSurfaceIndex[ REGFEMALE ][ FLYBACK_HIT ]						= RGFHITHARD;
	gubAnimSurfaceIndex[ REGFEMALE ][ READY_RIFLE_PRONE ]			= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ AIM_RIFLE_PRONE ]				= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_RIFLE_PRONE ]			= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_RIFLE_PRONE ]				= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLBACK_DEATHTWICH ]		= RGFHITHARD;
	gubAnimSurfaceIndex[ REGFEMALE ][ GENERIC_HIT_STAND ]			= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ FLYBACK_HIT_BLOOD_STAND ]		= RGFHITHARDBLOOD;
	gubAnimSurfaceIndex[ REGFEMALE ][ FLYBACK_HIT_DEATH ]			= RGFHITHARD;
	gubAnimSurfaceIndex[ REGFEMALE ][ READY_RIFLE_CROUCH ]		= RGFCROUCHAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ AIM_RIFLE_CROUCH ]			= RGFCROUCHAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_RIFLE_CROUCH ]		= RGFCROUCHAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_RIFLE_CROUCH ]			= RGFCROUCHAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLBACK_HIT_STAND ]		= RGFHITFALLBACK;
	gubAnimSurfaceIndex[ REGFEMALE ][ ROLLOVER ]							= RGFROLLOVER;
	gubAnimSurfaceIndex[ REGFEMALE ][ CLIMBUPROOF ]						= RGFCLIMBROOF;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF ]								= RGFFALL;
	gubAnimSurfaceIndex[ REGFEMALE ][ GETUP_FROM_ROLLOVER ]		= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ CLIMBDOWNROOF ]					= RGFCLIMBROOF;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLFORWARD_ROOF ]						= RGFFALLF;
	gubAnimSurfaceIndex[ REGFEMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ GENERIC_HIT_DEATHTWITCHB ]		= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]	= RGFHITFALLBACK;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLBACK_HIT_DEATHTWITCHB ]		= RGFHITFALLBACK;
	gubAnimSurfaceIndex[ REGFEMALE ][ GENERIC_HIT_DEATH ]			= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLBACK_HIT_DEATH ]		= RGFHITFALLBACK;
	gubAnimSurfaceIndex[ REGFEMALE ][ GENERIC_HIT_CROUCH ]		= RGFHITCROUCH;
	gubAnimSurfaceIndex[ REGFEMALE ][ STANDING_BURST ]				= RGFSTANDAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ STANDING_BURST_HIT ]		= RGFHITHARD;
	gubAnimSurfaceIndex[ REGFEMALE ][ GENERIC_HIT_DEATH ]			= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLFORWARD_FROMHIT_STAND ]		= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLFORWARD_FROMHIT_STAND ]				= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLFORWARD_FROMHIT_CROUCH ]			= RGFHITCROUCH;
	gubAnimSurfaceIndex[ REGFEMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]		= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ GENERIC_HIT_PRONE ]			= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_HIT_DEATH ]				= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_LAY_FROMHIT ]			= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_HIT_DEATHTWITCHNB ]					= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_HIT_DEATHTWITCHB ]					= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ FLYBACK_HITDEATH_STOP ]					= RGFHITHARD;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLFORWARD_HITDEATH_STOP ]			= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLBACK_HITDEATH_STOP ]				= RGFHITFALLBACK;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_HITDEATH_STOP ]						= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ FLYBACKHIT_STOP ]								= RGFHITHARD;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLBACKHIT_STOP ]							= RGFHITFALLBACK;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF_STOP ]									= RGFFALL;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF_FORWARD_STOP ]					= RGFFALLF;
	gubAnimSurfaceIndex[ REGFEMALE ][ STAND_FALLFORWARD_STOP ]				= RGFHITSTAND;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_LAYFROMHIT_STOP ]					= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ HOPFENCE ]											= RGFHOPFENCE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PUNCH_BREATH ]									= RGFPUNCH;
	gubAnimSurfaceIndex[ REGFEMALE ][ PUNCH ]													= RGFPUNCH;
	gubAnimSurfaceIndex[ REGFEMALE ][ NOTHING_STAND ]									= RGFNOTHING_STD;
	gubAnimSurfaceIndex[ REGFEMALE ][ JFK_HITDEATH ]									= RGFDIE_JFK;
	gubAnimSurfaceIndex[ REGFEMALE ][ JFK_HITDEATH_STOP ]							= RGFDIE_JFK;
	gubAnimSurfaceIndex[ REGFEMALE ][ JFK_HITDEATH_TWITCHB ]					= RGFDIE_JFK;
	gubAnimSurfaceIndex[ REGFEMALE ][ FIRE_STAND_BURST_SPREAD ]				= RGFSTANDAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF_DEATH ]									= RGFFALL;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF_FORWARD_DEATH ]					= RGMFALLF;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF_FORWARD_DEATH_STOP ]		= RGMFALLF;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF_FORWARD_TWITCHB ]				= RGMFALLF;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALLOFF_FORWARD_TWITCHNB ]			= RGMFALLF;
	gubAnimSurfaceIndex[ REGFEMALE ][ OPEN_DOOR ]											= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ OPEN_STRUCT ]										= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ PICKUP_ITEM ]										= RGFPICKUP;
	gubAnimSurfaceIndex[ REGFEMALE ][ DROP_ITEM ]											= RGFPICKUP;
	gubAnimSurfaceIndex[ REGFEMALE ][ SLICE ]													= RGFSLICE;
	gubAnimSurfaceIndex[ REGFEMALE ][ STAB ]													= RGFSTAB;
	gubAnimSurfaceIndex[ REGFEMALE ][ CROUCH_STAB ]										= RGFCSTAB;
	gubAnimSurfaceIndex[ REGFEMALE ][ START_AID ]											= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ GIVING_AID ]										= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_AID ]												= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ DODGE_ONE ]											= RGFDODGE;
	gubAnimSurfaceIndex[ REGFEMALE ][ READY_DUAL_STAND ]							= RGFSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ AIM_DUAL_STAND ]								= RGFSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_DUAL_STAND ]							= RGFSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_DUAL_STAND ]								= RGFSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ RAISE_RIFLE ]										= RGFRAISE;
	gubAnimSurfaceIndex[ REGFEMALE ][ LOWER_RIFLE ]										= RGFRAISE;
	gubAnimSurfaceIndex[ REGFEMALE ][ BODYEXPLODING ]									= BODYEXPLODE;
	gubAnimSurfaceIndex[ REGFEMALE ][ THROW_ITEM ]										= RGFTHROW;
	gubAnimSurfaceIndex[ REGFEMALE ][ LOB_ITEM ]											= RGFLOB;
	gubAnimSurfaceIndex[ REGFEMALE ][ CROUCHED_BURST ]								= RGFCROUCHAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_BURST ]										= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ GIVE_ITEM ]											= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ WATER_HIT ]											= RGFWATER_DIE;
	gubAnimSurfaceIndex[ REGFEMALE ][ WATER_DIE]											= RGFWATER_DIE;
	gubAnimSurfaceIndex[ REGFEMALE ][ WATER_DIE_STOP ]								= RGFWATER_DIE;
	gubAnimSurfaceIndex[ REGFEMALE ][ HELIDROP ]											= RGFHELIDROP;
	gubAnimSurfaceIndex[ REGFEMALE ][ FEM_CLEAN ]											= RGFCLEAN;
	gubAnimSurfaceIndex[ REGFEMALE ][ FEM_KICKSN ]										= RGFKICKSN;
	gubAnimSurfaceIndex[ REGFEMALE ][ FEM_LOOK ]											= RGFALOOK;
	gubAnimSurfaceIndex[ REGFEMALE ][ FEM_WIPE ]											= RGFWIPE;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPEN_DOOR ]									= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPEN_LOCKED_DOOR ]					= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ KICK_DOOR ]											= RGFKICKDOOR;
	gubAnimSurfaceIndex[ REGFEMALE ][ CLOSE_DOOR ]										= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ RIFLE_STAND_HIT ]								= RGFRHIT;
	gubAnimSurfaceIndex[ REGFEMALE ][ DEEP_WATER_TRED ]								= RGFDEEPWATER_TRED;
	gubAnimSurfaceIndex[ REGFEMALE ][ DEEP_WATER_SWIM ]								= RGFDEEPWATER_SWIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ DEEP_WATER_HIT ]								= RGFDEEPWATER_DIE;
	gubAnimSurfaceIndex[ REGFEMALE ][ DEEP_WATER_DIE]									= RGFDEEPWATER_DIE;
	gubAnimSurfaceIndex[ REGFEMALE ][ DEEP_WATER_DIE_STOPPING ]				= RGFWATER_DIE;
	gubAnimSurfaceIndex[ REGFEMALE ][ DEEP_WATER_DIE_STOP ]						= RGFWATER_DIE;
	gubAnimSurfaceIndex[ REGFEMALE ][ LOW_TO_DEEP_WATER ]							= RGFWATER_TRANS;
	gubAnimSurfaceIndex[ REGFEMALE ][ DEEP_TO_LOW_WATER ]							= RGFWATER_TRANS;
	gubAnimSurfaceIndex[ REGFEMALE ][ GOTO_SLEEP ]										= RGFSLEEP;
	gubAnimSurfaceIndex[ REGFEMALE ][ SLEEPING ]											= RGFSLEEP;
	gubAnimSurfaceIndex[ REGFEMALE ][ WKAEUP_FROM_SLEEP ]							= RGFSLEEP;
	gubAnimSurfaceIndex[ REGFEMALE ][ FIRE_LOW_STAND ]								= RGFSHOOT_LOW;
	gubAnimSurfaceIndex[ REGFEMALE ][ FIRE_BURST_LOW_STAND ]					= RGFSHOOT_LOW;
	gubAnimSurfaceIndex[ REGFEMALE ][ STANDING_SHOOT_UNJAM ]				  = RGFSTANDAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ CROUCH_SHOOT_UNJAM ]						= RGFCROUCHAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_SHOOT_UNJAM ]							= RGFPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ STANDING_SHOOT_DWEL_UNJAM ]		  = RGFSTANDDWALAIM;
	gubAnimSurfaceIndex[ REGFEMALE ][ STANDING_SHOOT_LOW_UNJAM ]			= RGFSHOOT_LOW;
	gubAnimSurfaceIndex[ REGFEMALE ][ READY_DUAL_CROUCH ]							= RGFCDBLSHOT;
	gubAnimSurfaceIndex[ REGFEMALE ][ AIM_DUAL_CROUCH ]								= RGFCDBLSHOT;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_DUAL_CROUCH ]							= RGFCDBLSHOT;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_DUAL_CROUCH ]								= RGFCDBLSHOT;
	gubAnimSurfaceIndex[ REGFEMALE ][ CROUCH_SHOOT_DWEL_UNJAM ]				= RGFCDBLSHOT;
	gubAnimSurfaceIndex[ REGFEMALE ][ ADJACENT_GET_ITEM ]							= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ CUTTING_FENCE ]									= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ FROM_INJURED_TRANSITION ]				= RGFHURTTRANS;
	gubAnimSurfaceIndex[ REGFEMALE ][ THROW_KNIFE ]										= RGFTHROWKNIFE;
	gubAnimSurfaceIndex[ REGFEMALE ][ KNIFE_BREATH ]									= RGFBREATHKNIFE;
	gubAnimSurfaceIndex[ REGFEMALE ][ KNIFE_GOTOBREATH ]							= RGFBREATHKNIFE;
	gubAnimSurfaceIndex[ REGFEMALE ][ KNIFE_ENDBREATH ]								= RGFBREATHKNIFE;
	gubAnimSurfaceIndex[ REGFEMALE ][ CATCH_STANDING ]								= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ CATCH_CROUCHED ]								= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ PLANT_BOMB ]										= RGFPICKUP;
	gubAnimSurfaceIndex[ REGFEMALE ][ USE_REMOTE ]										= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ START_COWER ]										= RGFCOWER;
	gubAnimSurfaceIndex[ REGFEMALE ][ COWERING ]											= RGFCOWER;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_COWER ]											= RGFCOWER;
	gubAnimSurfaceIndex[ REGFEMALE ][ STEAL_ITEM ]										= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_ROCKET ]									= RGFROCKET;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_MORTAR ]									= RGFMORTAR;
	gubAnimSurfaceIndex[ REGFEMALE ][ SIDE_STEP ]											= RGFSIDESTEP;
	gubAnimSurfaceIndex[ REGFEMALE ][ WALK_BACKWARDS ]								= RGFWALKING;
	gubAnimSurfaceIndex[ REGFEMALE ][ BEGIN_OPENSTRUCT ]							= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPENSTRUCT ]								= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPENSTRUCT_LOCKED ]					= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ PUNCH_LOW ]											= RGFPUNCHLOW;
	gubAnimSurfaceIndex[ REGFEMALE ][ PISTOL_SHOOT_LOW ]							= RGFPISTOLSHOOTLOW;
	gubAnimSurfaceIndex[ REGFEMALE ][ DECAPITATE ]										= RGFCSTAB;
	gubAnimSurfaceIndex[ REGFEMALE ][ GOTO_PATIENT ]									= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ BEING_PATIENT ]									= RGFHITPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ GOTO_DOCTOR ]										= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ BEING_DOCTOR ]									= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_DOCTOR ]										= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ GOTO_REPAIRMAN ]								= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ BEING_REPAIRMAN ]								= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_REPAIRMAN ]									= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ FALL_INTO_PIT ]									= RGFFALL;
	gubAnimSurfaceIndex[ REGFEMALE ][ RELOAD_ROBOT ]									= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_CATCH ]											= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_CROUCH_CATCH ]							= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ AI_RADIO ]											= RGFRADIO;
	gubAnimSurfaceIndex[ REGFEMALE ][ AI_CR_RADIO ]										= RGFCRRADIO;
	gubAnimSurfaceIndex[ REGFEMALE ][ QUEEN_SLAP ]										= RGFSLAP;
	gubAnimSurfaceIndex[ REGFEMALE ][ TAKE_BLOOD_FROM_CORPSE ]				= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ QUEEN_FRUSTRATED_SLAP ]					= RGFSLAP;
	gubAnimSurfaceIndex[ REGFEMALE ][ CHARIOTS_OF_FIRE ]							= RGMBURN;
	gubAnimSurfaceIndex[ REGFEMALE ][ AI_PULL_SWITCH ]								= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ MERC_HURT_IDLE_ANIM ]						= RGFHURTSTANDINGR;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_HURT_WALKING ]							= RGFHURTSTANDINGR;
	gubAnimSurfaceIndex[ REGFEMALE ][ PASS_OBJECT ]										= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ DROP_ADJACENT_OBJECT ]					= RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ READY_DUAL_PRONE ]							= RGFDWPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ AIM_DUAL_PRONE ]								= RGFDWPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ SHOOT_DUAL_PRONE ]							= RGFDWPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_DUAL_PRONE ]								= RGFDWPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PRONE_SHOOT_DWEL_UNJAM ]				= RGFDWPRONE;
	gubAnimSurfaceIndex[ REGFEMALE ][ PICK_LOCK ]										  = RGFOPEN;
	gubAnimSurfaceIndex[ REGFEMALE ][ OPEN_DOOR_CROUCHED ]						= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ BEGIN_OPENSTRUCT_CROUCHED ]			= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ CLOSE_DOOR_CROUCHED ]						= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ OPEN_STRUCT_CROUCHED ]					= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPEN_DOOR_CROUCHED ]				= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPENSTRUCT_CROUCHED ]				= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPEN_LOCKED_DOOR_CROUCHED ]	= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ END_OPENSTRUCT_LOCKED_CROUCHED ]= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ DRUNK_IDLE ]										= RGFDRUNK;
	gubAnimSurfaceIndex[ REGFEMALE ][ CROWBAR_ATTACK ]								= RGFCROWBAR;
	gubAnimSurfaceIndex[ REGFEMALE ][ ATTACH_CAN_TO_STRING ]					= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ SWAT_BACKWARDS ]								= RGFSNEAKING;
	gubAnimSurfaceIndex[ REGFEMALE ][ JUMP_OVER_BLOCKING_PERSON ]			= RGFJUMPOVER;
	gubAnimSurfaceIndex[ REGFEMALE ][ REFUEL_VEHICLE ]								= RGFMEDIC;
	gubAnimSurfaceIndex[ REGFEMALE ][ LOCKPICK_CROUCHED ]					    = RGFMEDIC;


	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ STANDING ]						= RGFPISTOLBREATH;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ WALKING ]							= RGFNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ RUNNING ]							= RGFNOTHING_RUN;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ SWATTING ]						= RGFNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ START_SWAT ]					= RGFNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ END_SWAT ]						= RGFNOTHING_SWAT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ KNEEL_DOWN ]					= RGFNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ CROUCHING ]						= RGFNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ KNEEL_UP ]						= RGFNOTHING_CROUCH;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ READY_RIFLE_STAND ]				= RGFHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ AIM_RIFLE_STAND ]					= RGFHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ SHOOT_RIFLE_STAND ]				= RGFHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ END_RIFLE_STAND ]					= RGFHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ STANDING_BURST ]						= RGFHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ FIRE_STAND_BURST_SPREAD ]	= RGFHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ READY_RIFLE_CROUCH ]				= RGFHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ AIM_RIFLE_CROUCH ]					= RGFHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ SHOOT_RIFLE_CROUCH ]				= RGFHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ END_RIFLE_CROUCH ]					= RGFHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ PRONE_DOWN ]								= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ CRAWLING ]									= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ PRONE_UP ]									= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ PRONE ]										= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ READY_RIFLE_PRONE ]				= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ AIM_RIFLE_PRONE ]					= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ SHOOT_RIFLE_PRONE ]				= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ END_RIFLE_PRONE ]					= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ GETUP_FROM_ROLLOVER ]			= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ STANDING_SHOOT_UNJAM ]			= RGFHANDGUN_S_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ CROUCH_SHOOT_UNJAM ]				= RGFHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ PRONE_SHOOT_UNJAM ]				= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ FIRE_BURST_LOW_STAND ]			= RGFPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ FIRE_LOW_STAND ]						= RGFPISTOLSHOOTLOW;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ CROUCHED_BURST ]						= RGFHANDGUN_C_SHOT;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ PRONE_BURST ]							= RGFHANDGUN_PRONE;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ MERC_HURT_IDLE_ANIM ]			= RGFHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ END_HURT_WALKING ]					= RGFHURTSTANDINGN;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ WALK_BACKWARDS ]						= RGFNOTHING_WALK;
	gubAnimSurfaceItemSubIndex[ REGFEMALE ][ DRUNK_IDLE ]								= RGMPISTOLDRUNK;


	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ STANDING][0]									= RGFWATER_R_STD;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ WALKING ][0]									= RGFWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ RUNNING ][0]									= RGFWATER_R_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ READY_RIFLE_STAND ][0]				= RGFWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ AIM_RIFLE_STAND ][0]					= RGFWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ SHOOT_RIFLE_STAND ][0]				= RGFWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ END_RIFLE_STAND ][0]					= RGFWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ STANDING_BURST ][0]					= RGFWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ FIRE_STAND_BURST_SPREAD ][0]	= RGFWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ STANDING][1]									= RGFWATER_N_STD;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ WALKING ][1]									= RGFWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ RUNNING ][1]									= RGFWATER_N_WALK;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ READY_RIFLE_STAND ][1]				= RGFWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ AIM_RIFLE_STAND ][1]					= RGFWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ SHOOT_RIFLE_STAND ][1]				= RGFWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ END_RIFLE_STAND ][1]					= RGFWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ STANDING_BURST ][1]					= RGFWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ FIRE_STAND_BURST_SPREAD ][1]	= RGFWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ READY_DUAL_STAND ][1]				= RGFWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ AIM_DUAL_STAND ][1]					= RGFWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ SHOOT_DUAL_STAND ][1]				= RGFWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ END_DUAL_STAND ][1]					= RGFWATER_DBLSHT;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ STANDING_SHOOT_UNJAM ][0]		= RGFWATER_R_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ STANDING_SHOOT_UNJAM ][1]		= RGFWATER_N_AIM;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ THROW_ITEM ][0]							= RGFWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ THROW_ITEM ][1]							= RGFWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ LOB_ITEM ][0]								= RGFWATERTHROW;
	gubAnimSurfaceMidWaterSubIndex[ REGFEMALE ][ LOB_ITEM ][1]								= RGFWATERTHROW;


	gubAnimSurfaceCorpseID[ REGFEMALE ][ GENERIC_HIT_DEATH ]						= FMERC_FWD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLBACK_HIT_DEATH ]						= FMERC_BCK;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ PRONE_HIT_DEATH ]							= FMERC_PRN;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ WATER_DIE ]										= FMERC_WTR;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FLYBACK_HIT_DEATH ]						= FMERC_DHD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ JFK_HITDEATH ]									= FMERC_JFK;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLOFF_DEATH ]								= FMERC_FALL;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLOFF_FORWARD_DEATH ]				= FMERC_FALLF;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FLYBACK_HIT ]									= FMERC_DHD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ GENERIC_HIT_DEATHTWITCHNB ]		= FMERC_FWD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLFORWARD_FROMHIT_STAND ]		= FMERC_FWD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ ENDFALLFORWARD_FROMHIT_CROUCH ]= FMERC_FWD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLBACK_HIT_STAND ]						= FMERC_BCK;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLBACK_HIT_DEATHTWITCHNB ]		= FMERC_BCK;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ PRONE_HIT_DEATHTWITCHNB ]			= FMERC_PRN;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ PRONE_LAY_FROMHIT ]						= FMERC_PRN;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLOFF ]											= FMERC_FALLF;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLFORWARD_ROOF ]							= FMERC_FALL;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FLYBACKHIT_STOP ]							= FMERC_DHD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ STAND_FALLFORWARD_STOP ]				= FMERC_FWD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ FALLBACKHIT_STOP ]							= FMERC_BCK;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ PRONE_LAYFROMHIT_STOP ]				= FMERC_PRN;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ CHARIOTS_OF_FIRE ]							= BURNT_DEAD;
	gubAnimSurfaceCorpseID[ REGFEMALE ][ BODYEXPLODING ]								= EXPLODE_DEAD;


	//Setup some random stuff
	gRandomAnimDefs[ REGFEMALE ][ 0 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ REGFEMALE ][ 0 ].sAnimID							= FEM_CLEAN;
	gRandomAnimDefs[ REGFEMALE ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ REGFEMALE ][ 0 ].ubEndRoll						= 3;
	gRandomAnimDefs[ REGFEMALE ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ REGFEMALE ][ 0 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGFEMALE ][ 1 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ REGFEMALE ][ 1 ].sAnimID							= FEM_KICKSN;
	gRandomAnimDefs[ REGFEMALE ][ 1 ].ubStartRoll					= 10;
	gRandomAnimDefs[ REGFEMALE ][ 1 ].ubEndRoll						= 13;
	gRandomAnimDefs[ REGFEMALE ][ 1 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ REGFEMALE ][ 1 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGFEMALE ][ 2 ].ubHandRestriction		= RANDOM_ANIM_RIFLEINHAND;
	gRandomAnimDefs[ REGFEMALE ][ 2 ].sAnimID							= FEM_LOOK;
	gRandomAnimDefs[ REGFEMALE ][ 2 ].ubStartRoll					= 20;
	gRandomAnimDefs[ REGFEMALE ][ 2 ].ubEndRoll						= 23;
	gRandomAnimDefs[ REGFEMALE ][ 2 ].ubFlags							= 0;
	gRandomAnimDefs[ REGFEMALE ][ 2 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGFEMALE ][ 3 ].ubHandRestriction		= RANDOM_ANIM_NOTHINGINHAND;
	gRandomAnimDefs[ REGFEMALE ][ 3 ].sAnimID							= FEM_WIPE;
	gRandomAnimDefs[ REGFEMALE ][ 3 ].ubStartRoll					= 30;
	gRandomAnimDefs[ REGFEMALE ][ 3 ].ubEndRoll						= 33;
	gRandomAnimDefs[ REGFEMALE ][ 3 ].ubFlags							= 0;
	gRandomAnimDefs[ REGFEMALE ][ 3 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ REGFEMALE ][ 4 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ REGFEMALE ][ 4 ].sAnimID							= MERC_HURT_IDLE_ANIM;
	gRandomAnimDefs[ REGFEMALE ][ 4 ].ubStartRoll					= 40;
	gRandomAnimDefs[ REGFEMALE ][ 4 ].ubEndRoll						= 100;
	gRandomAnimDefs[ REGFEMALE ][ 4 ].ubFlags							= RANDOM_ANIM_INJURED;
	gRandomAnimDefs[ REGFEMALE ][ 4 ].ubAnimHeight				= ANIM_STAND;

	////////////////////////////////////////////////////
	// MONSTERS

	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ STANDING ]													= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ RUNNING ]													= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ WALKING ]													= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ ADULTMONSTER_BREATHING ]						= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ ADULTMONSTER_WALKING ]							= AFMONSTERWALKING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ ADULTMONSTER_ATTACKING ]						= AFMONSTERATTACK;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ ADULTMONSTER_HIT ]									= AFMONSTERDIE;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ ADULTMONSTER_DYING ]								= AFMONSTERDIE;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ ADULTMONSTER_DYING_STOP ]					= AFMONSTERDIE;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_CLOSE_ATTACK ]					 	  = AFMONSTERCLOSEATTACK;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_SPIT_ATTACK ]					 	  = AFMONSTERSPITATTACK;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_BEGIN_EATTING_FLESH ]		  = AFMONSTEREATING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_EATTING_FLESH ]						= AFMONSTEREATING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_END_EATTING_FLESH ]				= AFMONSTEREATING;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_UP ]												= AFMUP;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_JUMP ]											= AFMJUMP;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_MELT ]											= AFMMELT;
	gubAnimSurfaceIndex[ ADULTFEMALEMONSTER ][ MONSTER_WALK_BACKWARDS ]						= AFMONSTERWALKING;

	gubAnimSurfaceCorpseID[ ADULTFEMALEMONSTER ][ MONSTER_MELT ]									= ADULTMONSTER_DEAD;

	// Adult male
	gubAnimSurfaceIndex[ AM_MONSTER ][ STANDING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ RUNNING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ WALKING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ ADULTMONSTER_BREATHING ]										= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ ADULTMONSTER_WALKING ]											= AFMONSTERWALKING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ ADULTMONSTER_ATTACKING ]										= AFMONSTERATTACK;
	gubAnimSurfaceIndex[ AM_MONSTER ][ ADULTMONSTER_HIT ]													= AFMONSTERDIE;
	gubAnimSurfaceIndex[ AM_MONSTER ][ ADULTMONSTER_DYING ]												= AFMONSTERDIE;
	gubAnimSurfaceIndex[ AM_MONSTER ][ ADULTMONSTER_DYING_STOP ]									= AFMONSTERDIE;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_CLOSE_ATTACK ]					 						= AFMONSTERCLOSEATTACK;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_SPIT_ATTACK ]					 						= AFMONSTERSPITATTACK;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_BEGIN_EATTING_FLESH ]							= AFMONSTEREATING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_EATTING_FLESH ]										= AFMONSTEREATING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_END_EATTING_FLESH ]								= AFMONSTEREATING;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_UP ]																= AFMUP;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_JUMP ]															= AFMJUMP;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_MELT ]															= AFMMELT;
	gubAnimSurfaceIndex[ AM_MONSTER ][ MONSTER_WALK_BACKWARDS ]										= AFMONSTERWALKING;

	gubAnimSurfaceCorpseID[ AM_MONSTER ][ MONSTER_MELT ]													= ADULTMONSTER_DEAD;

	// Young adult female
	gubAnimSurfaceIndex[ YAF_MONSTER ][ STANDING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ RUNNING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ WALKING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ ADULTMONSTER_BREATHING ]										= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ ADULTMONSTER_WALKING ]											= AFMONSTERWALKING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ ADULTMONSTER_ATTACKING ]										= AFMONSTERATTACK;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ ADULTMONSTER_HIT ]													= AFMONSTERDIE;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ ADULTMONSTER_DYING ]												= AFMONSTERDIE;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ ADULTMONSTER_DYING_STOP ]									= AFMONSTERDIE;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_CLOSE_ATTACK ]					 						= AFMONSTERCLOSEATTACK;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_SPIT_ATTACK ]					 						= AFMONSTERSPITATTACK;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_BEGIN_EATTING_FLESH ]							= AFMONSTEREATING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_EATTING_FLESH ]										= AFMONSTEREATING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_END_EATTING_FLESH ]								= AFMONSTEREATING;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_UP ]															= AFMUP;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_JUMP ]														= AFMJUMP;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_MELT]												= AFMMELT;
	gubAnimSurfaceIndex[ YAF_MONSTER ][ MONSTER_WALK_BACKWARDS ]									= AFMONSTERWALKING;

	gubAnimSurfaceCorpseID[ YAF_MONSTER ][ MONSTER_MELT ]										= ADULTMONSTER_DEAD;


	// Young adult male
	gubAnimSurfaceIndex[ YAM_MONSTER ][ STANDING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ RUNNING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ WALKING ]																	= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ ADULTMONSTER_BREATHING ]										= AFMONSTERSTANDING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ ADULTMONSTER_WALKING ]											= AFMONSTERWALKING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ ADULTMONSTER_ATTACKING ]										= AFMONSTERATTACK;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ ADULTMONSTER_HIT ]													= AFMONSTERDIE;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ ADULTMONSTER_DYING ]												= AFMONSTERDIE;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ ADULTMONSTER_DYING_STOP ]									= AFMONSTERDIE;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_CLOSE_ATTACK ]					 						= AFMONSTERCLOSEATTACK;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_SPIT_ATTACK ]					 						= AFMONSTERSPITATTACK;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_BEGIN_EATTING_FLESH ]							= AFMONSTEREATING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_EATTING_FLESH ]										= AFMONSTEREATING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_END_EATTING_FLESH ]								= AFMONSTEREATING;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_UP ]															= AFMUP;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_JUMP ]														= AFMJUMP;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_MELT ]											= AFMMELT;
	gubAnimSurfaceIndex[ YAM_MONSTER ][ MONSTER_WALK_BACKWARDS ]									= AFMONSTERWALKING;

	gubAnimSurfaceCorpseID[ YAM_MONSTER ][ MONSTER_MELT ]										= ADULTMONSTER_DEAD;


	gubAnimSurfaceIndex[ LARVAE_MONSTER ][ LARVAE_BREATH ]												= LVBREATH;
	gubAnimSurfaceIndex[ LARVAE_MONSTER ][ LARVAE_HIT ]														= LVDIE;
	gubAnimSurfaceIndex[ LARVAE_MONSTER ][ LARVAE_DIE ]														= LVDIE;
	gubAnimSurfaceIndex[ LARVAE_MONSTER ][ LARVAE_DIE_STOP ]											= LVDIE;
	gubAnimSurfaceIndex[ LARVAE_MONSTER ][ LARVAE_WALK ]													= LVWALK;

	gubAnimSurfaceCorpseID[ LARVAE_MONSTER ][ LARVAE_DIE ]												= LARVAEMONSTER_DEAD;

	gubAnimSurfaceIndex[ INFANT_MONSTER ][ STANDING ]															= IBREATH;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ RUNNING ]															= IWALK;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ WALKING ]															= IWALK;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ INFANT_HIT ]														= IDIE;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ INFANT_DIE ]														= IDIE;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ INFANT_DIE_STOP ]											= IDIE;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ INFANT_ATTACK ]												= IATTACK;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ INFANT_BEGIN_EATTING_FLESH ]						= IEAT;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ INFANT_EATTING_FLESH ]									= IEAT;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ INFANT_END_EATTING_FLESH ]							= IEAT;
	gubAnimSurfaceIndex[ INFANT_MONSTER ][ WALK_BACKWARDS ]					          		= IWALK;


	gubAnimSurfaceCorpseID[ INFANT_MONSTER ][ INFANT_DIE ]												= INFANTMONSTER_DEAD;


	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_MONSTER_BREATHING ]								= QUEENMONSTERSTANDING;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_HIT ]															= QUEENMONSTERDEATH;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_DIE ]															= QUEENMONSTERDEATH;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_DIE_STOP ]													= QUEENMONSTERDEATH;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_INTO_READY ]												= QUEENMONSTERREADY;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_READY ]														= QUEENMONSTERREADY;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_CALL ]															= QUEENMONSTERREADY;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_END_READY ]												= QUEENMONSTERREADY;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_SPIT ]															= QUEENMONSTERSPIT_SW;
	gubAnimSurfaceIndex[ QUEENMONSTER ][ QUEEN_SWIPE ]														= QUEENMONSTERSWIPE;

	gubAnimSurfaceCorpseID[ QUEENMONSTER ][ QUEEN_DIE ]		    										= QUEEN_MONSTER_DEAD;


	//////
	// FAT GUY
	gubAnimSurfaceIndex[ FATCIV ][ STANDING ]																			= FATMANSTANDING;
	gubAnimSurfaceIndex[ FATCIV ][ WALKING ]																			= FATMANWALKING;
	gubAnimSurfaceIndex[ FATCIV ][ RUNNING ]																			= FATMANRUNNING;
	gubAnimSurfaceIndex[ FATCIV ][ GENERIC_HIT_STAND ]														= FATMANDIE;
	gubAnimSurfaceIndex[ FATCIV ][ GENERIC_HIT_DEATHTWITCHNB ]										= FATMANDIE;
	gubAnimSurfaceIndex[ FATCIV ][ GENERIC_HIT_DEATHTWITCHB ]											= FATMANDIE;
	gubAnimSurfaceIndex[ FATCIV ][ FALLFORWARD_FROMHIT_STAND ]										= FATMANDIE;
	gubAnimSurfaceIndex[ FATCIV ][ FALLFORWARD_HITDEATH_STOP ]										= FATMANDIE;
	gubAnimSurfaceIndex[ FATCIV ][ STAND_FALLFORWARD_STOP ]												= FATMANDIE;
	gubAnimSurfaceIndex[ FATCIV ][ GENERIC_HIT_DEATH ]														= FATMANDIE;
	gubAnimSurfaceIndex[ FATCIV ][ FATCIV_ASS_SCRATCH ]														= FATMANASS;
	gubAnimSurfaceIndex[ FATCIV ][ OPEN_DOOR ]																		= FATMANACT;
	gubAnimSurfaceIndex[ FATCIV ][ OPEN_STRUCT ]																	= FATMANACT;
	gubAnimSurfaceIndex[ FATCIV ][ GIVE_ITEM ]																		= FATMANACT;
	gubAnimSurfaceIndex[ FATCIV ][ END_OPEN_DOOR ]																= FATMANACT;
	gubAnimSurfaceIndex[ FATCIV ][ END_OPEN_LOCKED_DOOR ]													= FATMANACT;
	gubAnimSurfaceIndex[ FATCIV ][ CLOSE_DOOR ]																		= FATMANACT;
	gubAnimSurfaceIndex[ FATCIV ][ ADJACENT_GET_ITEM ]														= FATMANACT;
	gubAnimSurfaceIndex[ FATCIV ][ START_COWER ]																	= FATMANCOWER;
	gubAnimSurfaceIndex[ FATCIV ][ COWERING ]																			= FATMANCOWER;
	gubAnimSurfaceIndex[ FATCIV ][ END_COWER ]																		= FATMANCOWER;
	gubAnimSurfaceIndex[ FATCIV ][ CIV_DIE2 ]																			= FATMANDIE2;
	gubAnimSurfaceIndex[ FATCIV ][ CIV_COWER_HIT ]																= FATMANCOWERHIT;

	gubAnimSurfaceCorpseID[ FATCIV ][ GENERIC_HIT_DEATH ]													= FT_DEAD1;
	gubAnimSurfaceCorpseID[ FATCIV ][ CIV_DIE2 ]																	= FT_DEAD2;

	gRandomAnimDefs[ FATCIV ][ 0 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ FATCIV ][ 0 ].sAnimID							= FATCIV_ASS_SCRATCH;
	gRandomAnimDefs[ FATCIV ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ FATCIV ][ 0 ].ubEndRoll						= 10;
	gRandomAnimDefs[ FATCIV ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ FATCIV ][ 0 ].ubAnimHeight					= ANIM_STAND;


	// Common civ
	gubAnimSurfaceIndex[ MANCIV ][ STANDING ]																			= MANCIVSTANDING;
	gubAnimSurfaceIndex[ MANCIV ][ WALKING ]																			= MANCIVWALKING;
	gubAnimSurfaceIndex[ MANCIV ][ RUNNING ]																			= MANCIVRUNNING;
	gubAnimSurfaceIndex[ MANCIV ][ GENERIC_HIT_STAND ]														= MANCIVDIE;
	gubAnimSurfaceIndex[ MANCIV ][ GENERIC_HIT_DEATHTWITCHNB ]										= MANCIVDIE;
	gubAnimSurfaceIndex[ MANCIV ][ GENERIC_HIT_DEATHTWITCHB ]											= MANCIVDIE;
	gubAnimSurfaceIndex[ MANCIV ][ FALLFORWARD_FROMHIT_STAND ]										= MANCIVDIE;
	gubAnimSurfaceIndex[ MANCIV ][ FALLFORWARD_HITDEATH_STOP ]										= MANCIVDIE;
	gubAnimSurfaceIndex[ MANCIV ][ STAND_FALLFORWARD_STOP ]												= MANCIVDIE;
	gubAnimSurfaceIndex[ MANCIV ][ GENERIC_HIT_DEATH ]														= MANCIVDIE;
	gubAnimSurfaceIndex[ MANCIV ][ OPEN_DOOR ]																		= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ OPEN_STRUCT ]																	= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ GIVE_ITEM ]																		= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ END_OPEN_DOOR ]																= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ END_OPEN_LOCKED_DOOR ]													= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ CLOSE_DOOR ]																		= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ ADJACENT_GET_ITEM ]														= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ START_COWER ]																	= MANCIVCOWER;
	gubAnimSurfaceIndex[ MANCIV ][ COWERING ]																			= MANCIVCOWER;
	gubAnimSurfaceIndex[ MANCIV ][ END_COWER ]																		= MANCIVCOWER;
	gubAnimSurfaceIndex[ MANCIV ][ CIV_DIE2 ]																			= MANCIVDIE2;
	gubAnimSurfaceIndex[ MANCIV ][ SLAP_HIT ]																			= MANCIVSMACKED;
	gubAnimSurfaceIndex[ MANCIV ][ PUNCH ]																				= MANCIVPUNCH;
	gubAnimSurfaceIndex[ MANCIV ][ PUNCH_BREATH ]																	= MANCIVPUNCH;
	gubAnimSurfaceIndex[ MANCIV ][ BEGIN_OPENSTRUCT ]															= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ END_OPENSTRUCT ]																= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ END_OPENSTRUCT_LOCKED ]												= MANCIVACT;
	gubAnimSurfaceIndex[ MANCIV ][ CIV_COWER_HIT ]																= MANCIVCOWERHIT;


	gubAnimSurfaceCorpseID[ MANCIV ][ GENERIC_HIT_DEATH ]													= M_DEAD1;
	gubAnimSurfaceCorpseID[ MANCIV ][ CIV_DIE2 ]																	= M_DEAD2;


	// mini skirt civ
	gubAnimSurfaceIndex[ MINICIV ][ STANDING ]																		= MINICIVSTANDING;
	gubAnimSurfaceIndex[ MINICIV ][ WALKING ]																			= MINICIVWALKING;
	gubAnimSurfaceIndex[ MINICIV ][ RUNNING ]																			= MINICIVRUNNING;
	gubAnimSurfaceIndex[ MINICIV ][ GENERIC_HIT_STAND ]														= MINICIVDIE;
	gubAnimSurfaceIndex[ MINICIV ][ GENERIC_HIT_DEATHTWITCHNB ]										= MINICIVDIE;
	gubAnimSurfaceIndex[ MINICIV ][ GENERIC_HIT_DEATHTWITCHB ]										= MINICIVDIE;
	gubAnimSurfaceIndex[ MINICIV ][ FALLFORWARD_FROMHIT_STAND ]										= MINICIVDIE;
	gubAnimSurfaceIndex[ MINICIV ][ FALLFORWARD_HITDEATH_STOP ]										= MINICIVDIE;
	gubAnimSurfaceIndex[ MINICIV ][ STAND_FALLFORWARD_STOP ]											= MINICIVDIE;
	gubAnimSurfaceIndex[ MINICIV ][ GENERIC_HIT_DEATH ]														= MINICIVDIE;
	gubAnimSurfaceIndex[ MINICIV ][ MINIGIRL_STOCKING ]														= MINISTOCKING;
	gubAnimSurfaceIndex[ MINICIV ][ OPEN_DOOR ]																		= MINIACT;
	gubAnimSurfaceIndex[ MINICIV ][ OPEN_STRUCT ]																	= MINIACT;
	gubAnimSurfaceIndex[ MINICIV ][ GIVE_ITEM ]																		= MINIACT;
	gubAnimSurfaceIndex[ MINICIV ][ END_OPEN_DOOR ]																= MINIACT;
	gubAnimSurfaceIndex[ MINICIV ][ END_OPEN_LOCKED_DOOR ]												= MINIACT;
	gubAnimSurfaceIndex[ MINICIV ][ CLOSE_DOOR ]																	= MINIACT;
	gubAnimSurfaceIndex[ MINICIV ][ ADJACENT_GET_ITEM ]														= MINIACT;
	gubAnimSurfaceIndex[ MINICIV ][ START_COWER ]																	= MINICOWER;
	gubAnimSurfaceIndex[ MINICIV ][ COWERING ]																		= MINICOWER;
	gubAnimSurfaceIndex[ MINICIV ][ END_COWER ]																		= MINICOWER;
	gubAnimSurfaceIndex[ MINICIV ][ CIV_DIE2 ]																		= MINIDIE2;
	gubAnimSurfaceIndex[ MINICIV ][ CIV_COWER_HIT ]																= MINICOWERHIT;

	gubAnimSurfaceCorpseID[ MINICIV ][ GENERIC_HIT_DEATH ]												= S_DEAD1;
	gubAnimSurfaceCorpseID[ MINICIV ][ CIV_DIE2 ]																	= S_DEAD2;


	gRandomAnimDefs[ MINICIV ][ 0 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ MINICIV ][ 0 ].sAnimID							= MINIGIRL_STOCKING;
	gRandomAnimDefs[ MINICIV ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ MINICIV ][ 0 ].ubEndRoll						= 10;
	gRandomAnimDefs[ MINICIV ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ MINICIV ][ 0 ].ubAnimHeight				= ANIM_STAND;


	// dress skirt civ
	gubAnimSurfaceIndex[ DRESSCIV ][ STANDING ]																		= DRESSCIVSTANDING;
	gubAnimSurfaceIndex[ DRESSCIV ][ WALKING ]																		= DRESSCIVWALKING;
	gubAnimSurfaceIndex[ DRESSCIV ][ RUNNING ]																		= DRESSCIVRUNNING;
	gubAnimSurfaceIndex[ DRESSCIV ][ GENERIC_HIT_STAND ]													= DRESSCIVDIE;
	gubAnimSurfaceIndex[ DRESSCIV ][ GENERIC_HIT_DEATHTWITCHNB ]									= DRESSCIVDIE;
	gubAnimSurfaceIndex[ DRESSCIV ][ GENERIC_HIT_DEATHTWITCHB ]										= DRESSCIVDIE;
	gubAnimSurfaceIndex[ DRESSCIV ][ FALLFORWARD_FROMHIT_STAND ]									= DRESSCIVDIE;
	gubAnimSurfaceIndex[ DRESSCIV ][ FALLFORWARD_HITDEATH_STOP ]									= DRESSCIVDIE;
	gubAnimSurfaceIndex[ DRESSCIV ][ STAND_FALLFORWARD_STOP ]											= DRESSCIVDIE;
	gubAnimSurfaceIndex[ DRESSCIV ][ GENERIC_HIT_DEATH ]													= DRESSCIVDIE;
	gubAnimSurfaceIndex[ DRESSCIV ][ OPEN_DOOR ]																	= DRESSCIVACT;
	gubAnimSurfaceIndex[ DRESSCIV ][ OPEN_STRUCT ]																= DRESSCIVACT;
	gubAnimSurfaceIndex[ DRESSCIV ][ GIVE_ITEM ]																	= DRESSCIVACT;
	gubAnimSurfaceIndex[ DRESSCIV ][ END_OPEN_DOOR ]															= DRESSCIVACT;
	gubAnimSurfaceIndex[ DRESSCIV ][ END_OPEN_LOCKED_DOOR ]												= DRESSCIVACT;
	gubAnimSurfaceIndex[ DRESSCIV ][ CLOSE_DOOR ]																	= DRESSCIVACT;
	gubAnimSurfaceIndex[ DRESSCIV ][ ADJACENT_GET_ITEM ]													= DRESSCIVACT;
	gubAnimSurfaceIndex[ DRESSCIV ][ START_COWER ]																= DRESSCIVCOWER;
	gubAnimSurfaceIndex[ DRESSCIV ][ COWERING ]																		= DRESSCIVCOWER;
	gubAnimSurfaceIndex[ DRESSCIV ][ END_COWER ]																	= DRESSCIVCOWER;
	gubAnimSurfaceIndex[ DRESSCIV ][ CIV_DIE2 ]																		= DRESSCIVDIE2;
	gubAnimSurfaceIndex[ DRESSCIV ][ CIV_COWER_HIT ]															= DRESSCIVCOWERHIT;

	gubAnimSurfaceCorpseID[ DRESSCIV ][ GENERIC_HIT_DEATH ]												= W_DEAD1;
	gubAnimSurfaceCorpseID[ DRESSCIV ][ CIV_DIE2 ]																= W_DEAD2;

	// HATKID civ
	gubAnimSurfaceIndex[ HATKIDCIV ][ STANDING ]																	= HATKIDCIVSTANDING;
	gubAnimSurfaceIndex[ HATKIDCIV ][ WALKING ]																		= HATKIDCIVWALKING;
	gubAnimSurfaceIndex[ HATKIDCIV ][ RUNNING ]																		= HATKIDCIVRUNNING;
	gubAnimSurfaceIndex[ HATKIDCIV ][ GENERIC_HIT_STAND ]													= HATKIDCIVDIE;
	gubAnimSurfaceIndex[ HATKIDCIV ][ GENERIC_HIT_DEATHTWITCHNB ]									= HATKIDCIVDIE;
	gubAnimSurfaceIndex[ HATKIDCIV ][ GENERIC_HIT_DEATHTWITCHB ]									= HATKIDCIVDIE;
	gubAnimSurfaceIndex[ HATKIDCIV ][ FALLFORWARD_FROMHIT_STAND ]									= HATKIDCIVDIE;
	gubAnimSurfaceIndex[ HATKIDCIV ][ FALLFORWARD_HITDEATH_STOP ]									= HATKIDCIVDIE;
	gubAnimSurfaceIndex[ HATKIDCIV ][ STAND_FALLFORWARD_STOP ]										= HATKIDCIVDIE;
	gubAnimSurfaceIndex[ HATKIDCIV ][ GENERIC_HIT_DEATH ]													= HATKIDCIVDIE;
	gubAnimSurfaceIndex[ HATKIDCIV ][ JFK_HITDEATH ]															= HATKIDCIVJFK;
	gubAnimSurfaceIndex[ HATKIDCIV ][ JFK_HITDEATH_STOP ]													= HATKIDCIVJFK;
	gubAnimSurfaceIndex[ HATKIDCIV ][ JFK_HITDEATH_TWITCHB ]											= HATKIDCIVJFK;
	gubAnimSurfaceIndex[ HATKIDCIV ][ HATKID_YOYO ]																= HATKIDCIVYOYO;
	gubAnimSurfaceIndex[ HATKIDCIV ][ OPEN_DOOR ]																	= HATKIDCIVACT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ OPEN_STRUCT ]																= HATKIDCIVACT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ GIVE_ITEM ]																	= HATKIDCIVACT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ END_OPEN_DOOR ]															= HATKIDCIVACT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ END_OPEN_LOCKED_DOOR ]											= HATKIDCIVACT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ CLOSE_DOOR ]																= HATKIDCIVACT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ ADJACENT_GET_ITEM ]													= HATKIDCIVACT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ START_COWER ]																= HATKIDCIVCOWER;
	gubAnimSurfaceIndex[ HATKIDCIV ][ COWERING ]																	= HATKIDCIVCOWER;
	gubAnimSurfaceIndex[ HATKIDCIV ][ END_COWER ]																	= HATKIDCIVCOWER;
	gubAnimSurfaceIndex[ HATKIDCIV ][ CIV_DIE2 ]																	= HATKIDCIVDIE2;
	gubAnimSurfaceIndex[ HATKIDCIV ][ CIV_COWER_HIT ]															= HATKIDCIVCOWERHIT;
	gubAnimSurfaceIndex[ HATKIDCIV ][ KID_SKIPPING ]															= HATKIDCIVSKIP;


	gubAnimSurfaceCorpseID[ HATKIDCIV ][ GENERIC_HIT_DEATH ]											= H_DEAD1;
	gubAnimSurfaceCorpseID[ HATKIDCIV ][ CIV_DIE2 ]																= H_DEAD2;

	gRandomAnimDefs[ HATKIDCIV ][ 0 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ HATKIDCIV ][ 0 ].sAnimID							= HATKID_YOYO;
	gRandomAnimDefs[ HATKIDCIV ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ HATKIDCIV ][ 0 ].ubEndRoll						= 10;
	gRandomAnimDefs[ HATKIDCIV ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ HATKIDCIV ][ 0 ].ubAnimHeight				= ANIM_STAND;


	// KID civ
	gubAnimSurfaceIndex[ KIDCIV ][ STANDING ]																			= KIDCIVSTANDING;
	gubAnimSurfaceIndex[ KIDCIV ][ WALKING ]																			= KIDCIVWALKING;
	gubAnimSurfaceIndex[ KIDCIV ][ RUNNING ]																			= KIDCIVRUNNING;
	gubAnimSurfaceIndex[ KIDCIV ][ GENERIC_HIT_STAND ]														= KIDCIVDIE;
	gubAnimSurfaceIndex[ KIDCIV ][ GENERIC_HIT_DEATHTWITCHNB ]										= KIDCIVDIE;
	gubAnimSurfaceIndex[ KIDCIV ][ GENERIC_HIT_DEATHTWITCHB ]											= KIDCIVDIE;
	gubAnimSurfaceIndex[ KIDCIV ][ FALLFORWARD_FROMHIT_STAND ]										= KIDCIVDIE;
	gubAnimSurfaceIndex[ KIDCIV ][ FALLFORWARD_HITDEATH_STOP ]										= KIDCIVDIE;
	gubAnimSurfaceIndex[ KIDCIV ][ STAND_FALLFORWARD_STOP ]												= KIDCIVDIE;
	gubAnimSurfaceIndex[ KIDCIV ][ GENERIC_HIT_DEATH ]														= KIDCIVDIE;
	gubAnimSurfaceIndex[ KIDCIV ][ JFK_HITDEATH ]																	=	KIDCIVJFK;
	gubAnimSurfaceIndex[ KIDCIV ][ JFK_HITDEATH_STOP ]														= KIDCIVJFK;
	gubAnimSurfaceIndex[ KIDCIV ][ JFK_HITDEATH_TWITCHB ]													= KIDCIVJFK;
	gubAnimSurfaceIndex[ KIDCIV ][ KID_ARMPIT ]																		= KIDCIVARMPIT;
	gubAnimSurfaceIndex[ KIDCIV ][ OPEN_DOOR ]																		= KIDCIVACT;
	gubAnimSurfaceIndex[ KIDCIV ][ OPEN_STRUCT ]																	= KIDCIVACT;
	gubAnimSurfaceIndex[ KIDCIV ][ GIVE_ITEM ]																		= KIDCIVACT;
	gubAnimSurfaceIndex[ KIDCIV ][ END_OPEN_DOOR ]																= KIDCIVACT;
	gubAnimSurfaceIndex[ KIDCIV ][ END_OPEN_LOCKED_DOOR ]													= KIDCIVACT;
	gubAnimSurfaceIndex[ KIDCIV ][ CLOSE_DOOR ]																		= KIDCIVACT;
	gubAnimSurfaceIndex[ KIDCIV ][ ADJACENT_GET_ITEM ]														= KIDCIVACT;
	gubAnimSurfaceIndex[ KIDCIV ][ START_COWER ]																	= KIDCIVCOWER;
	gubAnimSurfaceIndex[ KIDCIV ][ COWERING ]																			= KIDCIVCOWER;
	gubAnimSurfaceIndex[ KIDCIV ][ END_COWER ]																		= KIDCIVCOWER;
	gubAnimSurfaceIndex[ KIDCIV ][ CIV_DIE2 ]																			= KIDCIVDIE2;
	gubAnimSurfaceIndex[ KIDCIV ][ CIV_COWER_HIT ]																= KIDCIVCOWERHIT;
	gubAnimSurfaceIndex[ KIDCIV ][ KID_SKIPPING ]																	= KIDCIVSKIP;

	gubAnimSurfaceCorpseID[ KIDCIV ][ GENERIC_HIT_DEATH ]													= K_DEAD1;
	gubAnimSurfaceCorpseID[ KIDCIV ][ CIV_DIE2 ]																	= K_DEAD2;


	gRandomAnimDefs[ KIDCIV ][ 0 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ KIDCIV ][ 0 ].sAnimID							= KID_ARMPIT;
	gRandomAnimDefs[ KIDCIV ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ KIDCIV ][ 0 ].ubEndRoll						= 10;
	gRandomAnimDefs[ KIDCIV ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ KIDCIV ][ 0 ].ubAnimHeight					= ANIM_STAND;

	//CRIPPLE
	gubAnimSurfaceIndex[ CRIPPLECIV ][ STANDING ]																	= CRIPCIVSTANDING;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ WALKING ]																	= CRIPCIVWALKING;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ RUNNING ]																	= CRIPCIVRUNNING;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_BEG ]															= CRIPCIVBEG;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_HIT ]															= CRIPCIVDIE;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_DIE ]															= CRIPCIVDIE;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_DIE_STOP ]													= CRIPCIVDIE;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_DIE_FLYBACK ]											= CRIPCIVDIE2;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_DIE_FLYBACK_STOP ]									= CRIPCIVDIE2;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_KICKOUT ]													= CRIPCIVKICK;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_OPEN_DOOR ]											  = CRIPCIVBEG;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_CLOSE_DOOR ]											  = CRIPCIVBEG;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_END_OPEN_DOOR ]									  = CRIPCIVBEG;
	gubAnimSurfaceIndex[ CRIPPLECIV ][ CRIPPLE_END_OPEN_LOCKED_DOOR ]						  = CRIPCIVBEG;

	gubAnimSurfaceCorpseID[ CRIPPLECIV ][ CRIPPLE_DIE ]														= SMERC_BCK;

	gRandomAnimDefs[ CRIPPLECIV ][ 0 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ CRIPPLECIV ][ 0 ].sAnimID							= CRIPPLE_BEG;
	gRandomAnimDefs[ CRIPPLECIV ][ 0 ].ubStartRoll					= 30;
	gRandomAnimDefs[ CRIPPLECIV ][ 0 ].ubEndRoll						= 70;
	gRandomAnimDefs[ CRIPPLECIV ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ CRIPPLECIV ][ 0 ].ubAnimHeight					= ANIM_STAND;


	// COW
	gubAnimSurfaceIndex[ COW ][ STANDING ]																				= COWSTANDING;
	gubAnimSurfaceIndex[ COW ][ WALKING ]																					= COWWALKING;
	gubAnimSurfaceIndex[ COW ][ COW_EATING ]																			= COWEAT;
	gubAnimSurfaceIndex[ COW ][ COW_HIT ]																					= COWDIE;
	gubAnimSurfaceIndex[ COW ][ COW_DYING ]																				= COWDIE;
	gubAnimSurfaceIndex[ COW ][ COW_DYING_STOP ]																	= COWDIE;

	gubAnimSurfaceCorpseID[ COW ][ COW_DYING ]																		= COW_DEAD;


	gRandomAnimDefs[ COW ][ 0 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ COW ][ 0 ].sAnimID							= COW_EATING;
	gRandomAnimDefs[ COW ][ 0 ].ubStartRoll					= 30;
	gRandomAnimDefs[ COW ][ 0 ].ubEndRoll						= 100;
	gRandomAnimDefs[ COW ][ 0 ].ubFlags							= RANDOM_ANIM_CASUAL;
	gRandomAnimDefs[ COW ][ 0 ].ubAnimHeight				= ANIM_STAND;

	gRandomAnimDefs[ COW ][ 1 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ COW ][ 1 ].sAnimID							= RANDOM_ANIM_SOUND;
	gRandomAnimDefs[ COW ][ 1 ].ubStartRoll					= 0;
	gRandomAnimDefs[ COW ][ 1 ].ubEndRoll						= 1;
	gRandomAnimDefs[ COW ][ 1 ].ubAnimHeight				= ANIM_STAND;
	gRandomAnimDefs[COW][1].zSoundFile = "SOUNDS/COWMOO1.WAV";

	gRandomAnimDefs[ COW ][ 2 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ COW ][ 2 ].sAnimID							= RANDOM_ANIM_SOUND;
	gRandomAnimDefs[ COW ][ 2 ].ubStartRoll					= 2;
	gRandomAnimDefs[ COW ][ 2 ].ubEndRoll						= 3;
	gRandomAnimDefs[ COW ][ 2 ].ubAnimHeight				= ANIM_STAND;
	gRandomAnimDefs[COW][2].zSoundFile = "SOUNDS/COWMOO2.WAV";

	gRandomAnimDefs[ COW ][ 3 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ COW ][ 3 ].sAnimID							= RANDOM_ANIM_SOUND;
	gRandomAnimDefs[ COW ][ 3 ].ubStartRoll					= 4;
	gRandomAnimDefs[ COW ][ 3 ].ubEndRoll						= 5;
	gRandomAnimDefs[ COW ][ 3 ].ubAnimHeight				= ANIM_STAND;
	gRandomAnimDefs[COW][3].zSoundFile = "SOUNDS/COWMOO3.WAV";


	gubAnimSurfaceIndex[ CROW ][ STANDING ]																				= CROWWALKING;
	gubAnimSurfaceIndex[ CROW ][ WALKING ]																				= CROWWALKING;
	gubAnimSurfaceIndex[ CROW ][ CROW_WALK ]																			= CROWWALKING;
	gubAnimSurfaceIndex[ CROW ][ CROW_FLY ]																				= CROWFLYING;
	gubAnimSurfaceIndex[ CROW ][ CROW_EAT ]																				= CROWEATING;
	gubAnimSurfaceIndex[ CROW ][ CROW_TAKEOFF ]																		= CROWFLYING;
	gubAnimSurfaceIndex[ CROW ][ CROW_LAND ]																			= CROWFLYING;
	gubAnimSurfaceIndex[ CROW ][ CROW_DIE ]																				= CROWDYING;

	gRandomAnimDefs[ CROW ][ 0 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ CROW ][ 0 ].sAnimID							= RANDOM_ANIM_SOUND;
	gRandomAnimDefs[ CROW ][ 0 ].ubStartRoll					= 0;
	gRandomAnimDefs[ CROW ][ 0 ].ubEndRoll						= 50;
	gRandomAnimDefs[ CROW ][ 0 ].ubAnimHeight					= ANIM_STAND;
	gRandomAnimDefs[CROW][0].zSoundFile = "SOUNDS/CROW3.WAV";

	gRandomAnimDefs[ CROW ][ 1 ].ubHandRestriction		= RANDOM_ANIM_IRRELEVENTINHAND;
	gRandomAnimDefs[ CROW ][ 1 ].sAnimID							= RANDOM_ANIM_SOUND;
	gRandomAnimDefs[ CROW ][ 1 ].ubStartRoll					= 51;
	gRandomAnimDefs[ CROW ][ 1 ].ubEndRoll						= 70;
	gRandomAnimDefs[ CROW ][ 1 ].ubAnimHeight					= ANIM_STAND;
	gRandomAnimDefs[CROW][1].zSoundFile = "SOUNDS/CROW2.WAV";

	// BLOOD CAT
	gubAnimSurfaceIndex[ BLOODCAT ][ STANDING ]																		= CATBREATH;
	gubAnimSurfaceIndex[ BLOODCAT ][ WALKING ]																		= CATWALK;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_RUN ]																= CATRUN;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_STARTREADY ]												= CATREADY;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_READY ]															= CATREADY;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_ENDREADY ]													= CATREADY;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_HIT ]																= CATHIT;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_DYING ]															= CATDIE;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_DYING_STOP ]												= CATDIE;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_SWIPE ]															= CATSWIPE;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_BITE_ANIM ]													= CATBITE;
	gubAnimSurfaceIndex[ BLOODCAT ][ BLOODCAT_WALK_BACKWARDS ]										= CATWALK;

	gubAnimSurfaceCorpseID[ BLOODCAT ][ BLOODCAT_DYING ]													= BLOODCAT_DEAD;

	// ROBOT
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ STANDING ]															= ROBOTNWBREATH;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ WALKING ]																= ROBOTNWBREATH;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ ROBOTNW_HIT ]														= ROBOTNWHIT;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ ROBOTNW_DIE ]														= ROBOTNWDIE;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ ROBOTNW_DIE_STOP ]											= ROBOTNWDIE;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ ROBOT_WALK ]														= ROBOTNWWALK;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ ROBOT_SHOOT ]														= ROBOTNWSHOOT;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ ROBOT_BURST_SHOOT ]											= ROBOTNWSHOOT;
	gubAnimSurfaceIndex[ ROBOTNOWEAPON ][ ROBOT_CAMERA_NOT_MOVING ]								= ROBOTNWBREATH;
	gubAnimSurfaceCorpseID[ ROBOTNOWEAPON ][ ROBOTNW_DIE ]		      						 	= ROBOT_DEAD;



	// HUMVEE
	gubAnimSurfaceIndex[ HUMVEE ][ STANDING ]																		= HUMVEE_BASIC;
	gubAnimSurfaceIndex[ HUMVEE ][ WALKING ]																		= HUMVEE_BASIC;
	gubAnimSurfaceIndex[ HUMVEE ][ VEHICLE_DIE ]																= HUMVEE_DIE;
	gubAnimSurfaceCorpseID[ HUMVEE ][ STANDING ]		      									  	= HUMMER_DEAD;

	// TANK
	gubAnimSurfaceIndex[ TANK_NW ][ STANDING ]																	= TANKNW_READY;
	gubAnimSurfaceIndex[ TANK_NW ][ WALKING ]																		= TANKNW_READY;
	gubAnimSurfaceIndex[ TANK_NW ][ TANK_SHOOT ]																= TANKNW_SHOOT;
	gubAnimSurfaceIndex[ TANK_NW ][ TANK_BURST ]																= TANKNW_SHOOT;
	gubAnimSurfaceIndex[ TANK_NW ][ VEHICLE_DIE ]																= TANKNW_DIE;
	gubAnimSurfaceCorpseID[ TANK_NW ][ STANDING ]																= TANK1_DEAD;

	// TANK
	gubAnimSurfaceIndex[ TANK_NE ][ STANDING ]																	= TANKNE_READY;
	gubAnimSurfaceIndex[ TANK_NE ][ WALKING ]																		= TANKNE_READY;
	gubAnimSurfaceIndex[ TANK_NE ][ TANK_SHOOT ]																= TANKNE_SHOOT;
	gubAnimSurfaceIndex[ TANK_NE ][ TANK_BURST ]																= TANKNE_SHOOT;
	gubAnimSurfaceIndex[ TANK_NE ][ VEHICLE_DIE ]																= TANKNE_DIE;
	gubAnimSurfaceCorpseID[ TANK_NE ][ STANDING ]																= TANK2_DEAD;

	// ELDORADO
	gubAnimSurfaceIndex[ ELDORADO ][ STANDING ]																	= HUMVEE_BASIC;
	gubAnimSurfaceIndex[ ELDORADO ][ WALKING ]																	= HUMVEE_BASIC;
	gubAnimSurfaceIndex[ ELDORADO ][ VEHICLE_DIE ]															= HUMVEE_DIE;

	// ICECREAMTRUCK
	gubAnimSurfaceIndex[ ICECREAMTRUCK ][ STANDING ]														= ICECREAMTRUCK_BASIC;
	gubAnimSurfaceIndex[ ICECREAMTRUCK ][ WALKING ]															= ICECREAMTRUCK_BASIC;
	gubAnimSurfaceIndex[ ICECREAMTRUCK ][ VEHICLE_DIE ]													= HUMVEE_DIE;
	gubAnimSurfaceCorpseID[ ICECREAMTRUCK ][ STANDING ]											  	= ICECREAM_DEAD;

	// JEEP
	gubAnimSurfaceIndex[ JEEP ][ STANDING ]																			= HUMVEE_BASIC;
	gubAnimSurfaceIndex[ JEEP ][ WALKING ]																			= HUMVEE_BASIC;
	gubAnimSurfaceIndex[ JEEP ][ VEHICLE_DIE ]																	= HUMVEE_DIE;
}


void LoadAnimationStateInstructions()
{
	AutoSGPFile hFile(FileOpen(ANIMFILENAME, FILE_ACCESS_READ));
	FileRead(hFile, gusAnimInst, sizeof(gusAnimInst));
}


BOOLEAN IsAnimationValidForBodyType( SOLDIERTYPE *pSoldier, UINT16 usNewState )
{
	UINT16		usAnimSurface;

	// From animation control, get surface

	// First Save value
	usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usNewState );

	if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
	{
		return( FALSE );
	}

	return( TRUE );
}


UINT16 SubstituteBodyTypeAnimation(const SOLDIERTYPE* const s, const UINT16 anim_state)
{
	switch (s->ubBodyType)
	{
		case QUEENMONSTER:
			switch (anim_state)
			{
				case STANDING:
				case WALKING:
				case RUNNING:  return QUEEN_MONSTER_BREATHING;
			}
			break;

		case LARVAE_MONSTER:
			switch (anim_state)
			{
				case STANDING: return LARVAE_BREATH;
				case WALKING:
				case RUNNING:  return LARVAE_WALK;
			}
			break;

		case CROW:
			switch (anim_state)
			{
				case WALKING:  return CROW_WALK;
				case STANDING: return CROW_EAT;
			}
			break;

		case BLOODCAT:
			switch (anim_state)
			{
				case RUNNING: return BLOODCAT_RUN;
			}
			break;

		case ADULTFEMALEMONSTER:
		case AM_MONSTER:
		case YAF_MONSTER:
		case YAM_MONSTER:
			switch (anim_state)
			{
				case WALKING:
				case RUNNING: return ADULTMONSTER_WALKING;
			}
			break;

		case ROBOTNOWEAPON:
			switch (anim_state)
			{
				case STANDING:
					// OK, if they are on the CIV_TEAM, sub for no camera moving
					if (s->bTeam == CIV_TEAM) return ROBOT_CAMERA_NOT_MOVING;
					break;

				case WALKING:
				case RUNNING: return ROBOT_WALK;
			}
			break;

		default:
			if (IS_CIV_BODY_TYPE(s))
			{
				switch (anim_state)
				{
					case KNEEL_UP:          return END_COWER;
					case KNEEL_DOWN:        return START_COWER;
					case WKAEUP_FROM_SLEEP:
					case GOTO_SLEEP:
					case SLEEPING:          return STANDING;
				}
			}
			break;
	}
	return anim_state;
}


char const* GetBodyTypePaletteSubstitution(SOLDIERTYPE const* const s, UINT8 const ubBodyType)
{
	switch (ubBodyType)
	{
		case REGMALE:
		case BIGMALE:
		case STOCKYMALE:
		case REGFEMALE:
			if (s)
			{
				// Are we on fire?
				if (s->usAnimState == CHARIOTS_OF_FIRE ||
						s->usAnimState == BODYEXPLODING)
				{
					return "";
				}

				if (s->bCamo != 0) return "ANIMS/camo.COL";
			}
			return 0;

		case YAF_MONSTER:        return "ANIMS/MONSTERS/fm_brite.COL";
		case YAM_MONSTER:        return "ANIMS/MONSTERS/mn_brite.COL";
		case ADULTFEMALEMONSTER: return "ANIMS/MONSTERS/femmon.COL";
		case AM_MONSTER:         return "ANIMS/MONSTERS/monster.COL";

		case QUEENMONSTER:
		case COW:
		case CROW:
		case BLOODCAT:
		case HUMVEE:
		case ELDORADO:
		case ICECREAMTRUCK:
		case JEEP:
		case ROBOTNOWEAPON:
		case TANK_NW:
		case TANK_NE:
			return "";

		default: return 0;
	}
}



BOOLEAN SetSoldierAnimationSurface( SOLDIERTYPE *pSoldier, UINT16 usAnimState )
{
	UINT16 usAnimSurface;

		// Delete any structure info!
	if ( pSoldier->pLevelNode != NULL )
	{
		DeleteStructureFromWorld( pSoldier->pLevelNode->pStructureData );
		pSoldier->pLevelNode->pStructureData = NULL;
	}


	usAnimSurface = LoadSoldierAnimationSurface( pSoldier, usAnimState );

	// Add structure info!
	if ( pSoldier->pLevelNode != NULL && !( pSoldier->uiStatusFlags & SOLDIER_PAUSEANIMOVE ) )
	{
		AddMercStructureInfoFromAnimSurface( pSoldier->sGridNo, pSoldier, usAnimSurface, usAnimState );
	}

	// Set
	pSoldier->usAnimSurface = usAnimSurface;

	if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
	{
		return( FALSE );
	}

	return( TRUE );
}


UINT16 LoadSoldierAnimationSurface( SOLDIERTYPE *pSoldier, UINT16 usAnimState )
{
	UINT16 usAnimSurface;

	usAnimSurface = DetermineSoldierAnimationSurface( pSoldier, usAnimState );

	if ( usAnimSurface != INVALID_ANIMATION_SURFACE )
	try
	{
		// Ensure that it's been loaded!
		GetCachedAnimationSurface(pSoldier->ubID, &pSoldier->AnimCache, usAnimSurface, pSoldier->usAnimState);
	}
	catch (...)
	{
		usAnimSurface = INVALID_ANIMATION_SURFACE;
	}

	return( usAnimSurface );
}


UINT16	gusQueenMonsterSpitAnimPerDir[] =
{
	QUEENMONSTERSPIT_NE,			//NORTH
	QUEENMONSTERSPIT_E,
	QUEENMONSTERSPIT_SE,			// EAST
	QUEENMONSTERSPIT_S,
	QUEENMONSTERSPIT_SW,			// SOUTH
	QUEENMONSTERSPIT_SW,
	QUEENMONSTERSPIT_SW,			// WEST
	QUEENMONSTERSPIT_NE,
};


UINT16 DetermineSoldierAnimationSurface(const SOLDIERTYPE* pSoldier, UINT16 usAnimState)
{
	UINT16 usAnimSurface;
	UINT16 usAltAnimSurface;
	UINT16	usItem;
	UINT8		ubWaterHandIndex = 1;
	BOOLEAN	fAdjustedForItem	= FALSE;

	usAnimState = SubstituteBodyTypeAnimation(pSoldier, usAnimState);

	usAnimSurface	=	gubAnimSurfaceIndex[pSoldier->ubBodyType][usAnimState];

	// CHECK IF WE CAN DO THIS ANIMATION, IE WE HAVE IT AVAILIBLE
	if ( usAnimSurface == INVALID_ANIMATION	)
	{
		// WE SHOULD NOT BE USING THIS ANIMATION
		ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION, L"Invalid Animation File for Body %d, animation %hs.", pSoldier->ubBodyType, gAnimControl[usAnimState].zAnimStr);
		// Set index to FOUND_INVALID_ANIMATION
		gubAnimSurfaceIndex[pSoldier->ubBodyType][usAnimState] = FOUND_INVALID_ANIMATION;
		return( INVALID_ANIMATION_SURFACE );
	}
	if ( usAnimSurface == FOUND_INVALID_ANIMATION	)
	{
		return( INVALID_ANIMATION_SURFACE );
	}


	// OK - DO SOME MAGIC HERE TO SWITCH BODY TYPES IF WE WANT!


	// If we are a queen, pick the 'real' anim surface....
	if ( usAnimSurface == QUEENMONSTERSPIT_SW )
	{
		INT8	bDir;

		// Assume a target gridno is here.... get direction...
    // ATE: use +2 in gridno because here head is far from body
		bDir = (INT8)GetDirectionToGridNoFromGridNo( (INT16)( pSoldier->sGridNo + 2 ), pSoldier->sTargetGridNo );

		return( gusQueenMonsterSpitAnimPerDir[ bDir ] );
	}


	// IF we are not a merc, return
	if ( pSoldier->ubBodyType > REGFEMALE )
	{
		return( usAnimSurface );
	}

	// SWITCH TO DIFFERENT AIM ANIMATION FOR BIG GUY!
	if ( usAnimSurface == BGMSTANDAIM2 )
	{
		if ( pSoldier->uiAnimSubFlags & SUB_ANIM_BIGGUYSHOOT2 )
		{
			usAnimSurface = BGMSTANDAIM;
		}
	}

	// SWITCH TO DIFFERENT STAND ANIMATION FOR BIG GUY!
	if ( usAnimSurface == BGMSTANDING )
	{
		if ( pSoldier->uiAnimSubFlags & SUB_ANIM_BIGGUYTHREATENSTANCE )
		{
			usAnimSurface = BGMTHREATENSTAND;
		}
	}

	if ( usAnimSurface == BGMWALKING )
	{
		if ( pSoldier->uiAnimSubFlags & SUB_ANIM_BIGGUYTHREATENSTANCE )
		{
			usAnimSurface = BGMWALK2;
		}
	}

	if ( usAnimSurface == BGMRUNNING )
	{
		if ( pSoldier->uiAnimSubFlags & SUB_ANIM_BIGGUYTHREATENSTANCE )
		{
			usAnimSurface = BGMRUN2;
		}
	}

	if ( usAnimSurface == BGMRAISE )
	{
		if ( pSoldier->uiAnimSubFlags & SUB_ANIM_BIGGUYTHREATENSTANCE )
		{
			usAnimSurface = BGMRAISE2;
		}
	}


	// ADJUST ANIMATION SURFACE BASED ON TERRAIN

	// CHECK FOR WATER
	if ( MercInWater( pSoldier ) )
	{

		// ADJUST BASED ON ITEM IN HAND....
		usItem = pSoldier->inv[ HANDPOS ].usItem;

		// Default it to the 1 ( ie: no rifle )
		if ( usItem != NOTHING )
		{
			if ( ( Item[ usItem ].usItemClass == IC_GUN || Item[ usItem ].usItemClass == IC_LAUNCHER ) && usItem != ROCKET_LAUNCHER )
			{
				if ( (Item[ usItem ].fFlags & ITEM_TWO_HANDED) )
				{
					ubWaterHandIndex = 0;
				}
			}
		}

		// CHANGE BASED ON HIEGHT OF WATER
		usAltAnimSurface = gubAnimSurfaceMidWaterSubIndex[pSoldier->ubBodyType][usAnimState][ ubWaterHandIndex ];

		if ( usAltAnimSurface != INVALID_ANIMATION )
		{
			usAnimSurface = usAltAnimSurface;
		}

	}
	else
	{
		// ADJUST BASED ON ITEM IN HAND....
		usItem = pSoldier->inv[ HANDPOS ].usItem;

		if ( !(Item[ usItem ].usItemClass == IC_GUN ) && !(Item[ usItem ].usItemClass == IC_LAUNCHER ) || usItem == ROCKET_LAUNCHER )
		{
			if ( usAnimState == STANDING )
			{
				usAnimSurface			= gusNothingBreath[ pSoldier->ubBodyType ];
				fAdjustedForItem	= TRUE;
			}
			else
			{
				usAltAnimSurface = gubAnimSurfaceItemSubIndex[pSoldier->ubBodyType][usAnimState];

				if ( usAltAnimSurface != INVALID_ANIMATION )
				{
					usAnimSurface			= usAltAnimSurface;
					fAdjustedForItem	= TRUE;
				}
			}
		}
		else
		{
			// CHECK FOR HANDGUN
			if ( ( Item[ usItem ].usItemClass == IC_GUN || Item[ usItem ].usItemClass == IC_LAUNCHER ) && usItem != ROCKET_LAUNCHER )
			{
				if ( !(Item[ usItem ].fFlags & ITEM_TWO_HANDED) )
				{
					usAltAnimSurface = gubAnimSurfaceItemSubIndex[pSoldier->ubBodyType][usAnimState];
					if ( usAltAnimSurface != INVALID_ANIMATION )
					{
						usAnimSurface = usAltAnimSurface;
						fAdjustedForItem	= TRUE;
					}

					// Look for good two pistols sub anim.....
					if ( gDoubleHandledSub.usAnimState == usAnimState )
					{
						// Do we carry two pistols...
						if ( Item[ pSoldier->inv[ SECONDHANDPOS ].usItem ].usItemClass == IC_GUN )
						{
							usAnimSurface = gDoubleHandledSub.usAnimationSurfaces[ pSoldier->ubBodyType ];
							fAdjustedForItem	= TRUE;
						}
					}

				}
			}
			else
			{
				usAltAnimSurface = gubAnimSurfaceItemSubIndex[pSoldier->ubBodyType][usAnimState];

				if ( usAltAnimSurface != INVALID_ANIMATION )
				{
					usAnimSurface = usAltAnimSurface;
					fAdjustedForItem	= TRUE;
				}
			}
		}

		// Based on if we have adjusted for item or not... check for injured status...
		if ( fAdjustedForItem )
		{
			// If life below thresthold for being injured
			if ( pSoldier->bLife < INJURED_CHANGE_THREASHOLD )
			{
				// ADJUST FOR INJURED....
				if (gNothingInjuredSub.usAnimState == usAnimState)
				{
					usAnimSurface = gNothingInjuredSub.usAnimationSurfaces[pSoldier->ubBodyType];
				}
			}
		}
		else
		{
			// If life below thresthold for being injured
			if ( pSoldier->bLife < INJURED_CHANGE_THREASHOLD )
			{
				// ADJUST FOR INJURED....
				if (gRifleInjuredSub.usAnimState == usAnimState)
				{
					usAnimSurface = gRifleInjuredSub.usAnimationSurfaces[pSoldier->ubBodyType];
				}
			}
		}



	}
	return( usAnimSurface );
}


UINT16 GetSoldierAnimationSurface(SOLDIERTYPE const* const pSoldier)
{
	UINT16 usAnimSurface;

	usAnimSurface = pSoldier->usAnimSurface;

	if ( usAnimSurface != INVALID_ANIMATION_SURFACE )
	{
		// Ensure that it's loaded!
		if ( gAnimSurfaceDatabase[ usAnimSurface ].hVideoObject == NULL )
		{
			ScreenMsg(FONT_MCOLOR_RED, MSG_BETAVERSION, L"IAnimation Surface for Body %d, animation %hs, surface %d not loaded.", pSoldier->ubBodyType, gAnimControl[pSoldier->usAnimState].zAnimStr, usAnimSurface);
			AnimDebugMsg("Surface Database: PROBLEMS!!!!!!");
    	usAnimSurface = INVALID_ANIMATION_SURFACE;
		}
	}

	return( usAnimSurface );
}
