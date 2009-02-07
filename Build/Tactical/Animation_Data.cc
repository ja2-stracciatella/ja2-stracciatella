#include <stdexcept>

#include "HImage.h"
#include "Overhead.h"
#include "Structure.h"
#include "VObject.h"
#include "WCheck.h"
#include "Debug.h"
#include "Animation_Data.h"
#include "Animation_Control.h"
#include "Soldier_Control.h"
#include "Sys_Globals.h"
#include "Debug_Control.h"
#include "WorldDef.h"
#include "FileMan.h"
#include "MemMan.h"


#define EMPTY_SLOT		-1
#define TO_INIT				0

#define	ANIMPROFILEFILENAME			"BINARYDATA/JA2PROF.DAT"



ANIM_PROF		*gpAnimProfiles = NULL;
static UINT8 gubNumAnimProfiles = 0;

INT8				gbAnimUsageHistory[ NUMANIMATIONSURFACETYPES ][ MAX_NUM_SOLDIERS ];


#define M(name, file, type, flags, dir, profile) { name, file, type, flags, dir, TO_INIT, NULL, 0, profile }

AnimationSurfaceType gAnimSurfaceDatabase[NUMANIMATIONSURFACETYPES] =
{
	M(RGMBASICWALKING,      "ANIMS/S_MERC/S_R_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSTANDING,          "ANIMS/S_MERC/S_R_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCROUCHING,         "ANIMS/S_MERC/S_R_C.STI",       C_STRUCT,  0,                        8, -1),
	M(RGMSNEAKING,          "ANIMS/S_MERC/S_R_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMRUNNING,           "ANIMS/S_MERC/S_R_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMPRONE,             "ANIMS/S_MERC/S_R_PRN.STI",     P_STRUCT,  0,                        8,  0),
	M(RGMSTANDAIM,          "ANIMS/S_MERC/S_SR_AIM.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHITHARD,           "ANIMS/S_MERC/S_DIEHD.STI",     FB_STRUCT, 0,                        8, -1),
	M(RGMHITSTAND,          "ANIMS/S_MERC/S_DIEFWD.STI",    F_STRUCT,  0,                        8, -1),
	M(RGMHITHARDBLOOD,      "ANIMS/S_MERC/S_DIEHDB.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGMCROUCHAIM,         "ANIMS/S_MERC/S_CR_AIM.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHITFALLBACK,       "ANIMS/S_MERC/S_DIEBAC.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGMROLLOVER,          "ANIMS/S_MERC/S_ROLL.STI",      P_STRUCT,  0,                        8, -1),
	M(RGMCLIMBROOF,         "ANIMS/S_MERC/S_CLIMB.STI",     S_STRUCT,  0,                        4, -1),
	M(RGMFALL,              "ANIMS/S_MERC/S_FALL.STI",      S_STRUCT,  0,                        4, -1),
	M(RGMFALLF,             "ANIMS/S_MERC/S_FALLF.STI",     S_STRUCT,  0,                        4, -1),
	M(RGMHITCROUCH,         "ANIMS/S_MERC/S_C_DIE.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMHITPRONE,          "ANIMS/S_MERC/S_P_DIE.STI",     P_STRUCT,  0,                        8, -1),
	M(RGMHOPFENCE,          "ANIMS/S_MERC/S_HOP.STI",       NO_STRUCT, 0,                        4, -1),
	M(RGMPUNCH,             "ANIMS/S_MERC/S_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_STD,       "ANIMS/S_MERC/S_N_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_WALK,      "ANIMS/S_MERC/S_N_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_RUN,       "ANIMS/S_MERC/S_N_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_SWAT,      "ANIMS/S_MERC/S_N_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_CROUCH,    "ANIMS/S_MERC/S_N_CRCH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_S_SHOT,    "ANIMS/S_MERC/S_N_SHOT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_C_SHOT,    "ANIMS/S_MERC/S_N_C_AI.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_PRONE,     "ANIMS/S_MERC/S_N_PRNE.STI",    P_STRUCT,  0,                        8, -1),
	M(RGMDIE_JFK,           "ANIMS/S_MERC/S_DIEJFK.STI",    F_STRUCT,  0,                        8, -1),
	M(RGMOPEN,              "ANIMS/S_MERC/S_OPEN.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMPICKUP,            "ANIMS/S_MERC/S_PICKUP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSTAB,              "ANIMS/S_MERC/S_STAB.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMSLICE,             "ANIMS/S_MERC/S_SLICE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCSTAB,             "ANIMS/S_MERC/S_C_STB.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMMEDIC,             "ANIMS/S_MERC/S_MEDIC.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMDODGE,             "ANIMS/S_MERC/S_DODGE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMSTANDDWALAIM,      "ANIMS/S_MERC/S_DBLSHT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMRAISE,             "ANIMS/S_MERC/S_RAISE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMTHROW,             "ANIMS/S_MERC/S_LOB.STI",       S_STRUCT,  0,                        8, -1),
	M(RGMLOB,               "ANIMS/S_MERC/S_THROW.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMKICKDOOR,          "ANIMS/S_MERC/S_DR_KCK.STI",    S_STRUCT,  0,                        4, -1),
	M(RGMRHIT,              "ANIMS/S_MERC/S_R_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(RGM_SQUISH,           "ANIMS/S_MERC/A_SQUISH.STI",    S_STRUCT,  0,                        8, -1),
	M(RGM_LOOK,             "ANIMS/S_MERC/A_LOOK.STI",      S_STRUCT,  0,                        8, -1),
	M(RGM_PULL,             "ANIMS/S_MERC/A_PULL.STI",      S_STRUCT,  0,                        8, -1),
	M(RGM_SPIT,             "ANIMS/S_MERC/A_SPIT.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_WALK,      "ANIMS/S_MERC/SW_R_WALK.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_STD,       "ANIMS/S_MERC/SW_R_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_WALK,      "ANIMS/S_MERC/SW_N_WALK.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_STD,       "ANIMS/S_MERC/SW_N_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMWATER_DIE,         "ANIMS/S_MERC/SW_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_AIM,       "ANIMS/S_MERC/SW_N_SHOT.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_AIM,       "ANIMS/S_MERC/SW_SR_AIM.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_DBLSHT,      "ANIMS/S_MERC/SW_DBLSHT.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_TRANS,       "ANIMS/S_MERC/SW_FALL.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_TRED,    "ANIMS/S_MERC/S_TRED.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_SWIM,    "ANIMS/S_MERC/S_SWIM.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_DIE,     "ANIMS/S_MERC/S_D_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMMCLIMB,            "ANIMS/S_MERC/S_MCLIMB.STI",    S_STRUCT,  0,                        3, -1),
	M(RGMHELIDROP,          "ANIMS/S_MERC/S_HELIDRP.STI",   S_STRUCT,  0,                        1, -1),
	M(RGMLOWKICK,           "ANIMS/S_MERC/K_LW_KICK.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMPUNCH,             "ANIMS/S_MERC/K_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMSPINKICK,          "ANIMS/S_MERC/S_SPNKCK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSLEEP,             "ANIMS/S_MERC/S_SLEEP.STI",     P_STRUCT,  0,                        8, -1),
	M(RGMSHOOT_LOW,         "ANIMS/S_MERC/S_SHTLO.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCDBLSHOT,          "ANIMS/S_MERC/SC_DBLSH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHURTSTANDINGN,     "ANIMS/S_MERC/S_I_BR_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTSTANDINGR,     "ANIMS/S_MERC/S_I_BR_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTWALKINGN,      "ANIMS/S_MERC/S_I_WK_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTWALKINGR,      "ANIMS/S_MERC/S_I_WK_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTTRANS,         "ANIMS/S_MERC/S_I_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMTHROWKNIFE,        "ANIMS/S_MERC/S_K_THRO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMBREATHKNIFE,       "ANIMS/S_MERC/S_KNF_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLBREATH,      "ANIMS/S_MERC/S_P_BRTH.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMCOWER,             "ANIMS/S_MERC/S_COWER.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMROCKET,            "ANIMS/S_MERC/S_LAW.STI",       S_STRUCT,  0,                        8, -1),
	M(RGMMORTAR,            "ANIMS/S_MERC/S_MORTAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSIDESTEP,          "ANIMS/S_MERC/S_R_SDSP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMDBLBREATH,         "ANIMS/S_MERC/S_DBL_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPUNCHLOW,          "ANIMS/S_MERC/S_PCH_LO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLSHOOTLOW,    "ANIMS/S_MERC/S_P_SHLO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMWATERTHROW,        "ANIMS/S_MERC/SW_LOB.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMRADIO,             "ANIMS/S_MERC/S_RADIO.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCRRADIO,           "ANIMS/S_MERC/S_C_RADO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMBURN,              "ANIMS/S_MERC/S_FIRE.STI",      S_STRUCT,  0,                        1, -1),
	M(RGMDWPRONE,           "ANIMS/S_MERC/S_DB_PRN.STI",    P_STRUCT,  0,                        8, -1),
	M(RGMDRUNK,             "ANIMS/S_MERC/S_R_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLDRUNK,       "ANIMS/S_MERC/S_N_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMCROWBAR,           "ANIMS/S_MERC/S_CROBAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMJUMPOVER,          "ANIMS/S_MERC/S_N_RUN.STI",     NO_STRUCT, 0,                        8, -1),


	M(BGMWALKING,           "ANIMS/M_MERC/M_R_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSTANDING,          "ANIMS/M_MERC/M_R_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMCROUCHING,         "ANIMS/M_MERC/M_R_C.STI",       C_STRUCT,  0,                        8, -1),
	M(BGMSNEAKING,          "ANIMS/M_MERC/M_R_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMRUNNING,           "ANIMS/M_MERC/M_R_RUN2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPRONE,             "ANIMS/M_MERC/M_R_PRN.STI",     P_STRUCT,  0,                        8,  0),
	M(BGMSTANDAIM,          "ANIMS/M_MERC/M_SR_AIM.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHITHARD,           "ANIMS/M_MERC/M_DIEHD.STI",     FB_STRUCT, 0,                        8, -1),
	M(BGMHITSTAND,          "ANIMS/M_MERC/M_DIEFWD.STI",    F_STRUCT,  0,                        8, -1),
	M(BGMHITHARDBLOOD,      "ANIMS/M_MERC/M_DIEHDB.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGMCROUCHAIM,         "ANIMS/M_MERC/M_CR_AIM.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHITFALLBACK,       "ANIMS/M_MERC/M_DIEBAC.STI",    FB_STRUCT, 0,                        8, -1),
	M(BGMROLLOVER,          "ANIMS/M_MERC/M_ROLL.STI",      P_STRUCT,  0,                        8, -1),
	M(BGMCLIMBROOF,         "ANIMS/M_MERC/M_CLIMB.STI",     S_STRUCT,  0,                        4, -1),
	M(BGMFALL,              "ANIMS/M_MERC/M_FALL.STI",      S_STRUCT,  0,                        4, -1),
	M(BGMFALLF,             "ANIMS/M_MERC/M_FALLF.STI",     S_STRUCT,  0,                        4, -1),
	M(BGMHITCROUCH,         "ANIMS/M_MERC/M_C_DIE.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMHITPRONE,          "ANIMS/M_MERC/M_P_DIE.STI",     P_STRUCT,  0,                        8, -1),
	M(BGMHOPFENCE,          "ANIMS/M_MERC/M_HOP.STI",       NO_STRUCT, 0,                        4, -1),
	M(BGMPUNCH,             "ANIMS/M_MERC/M_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_STD,       "ANIMS/M_MERC/M_N_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_WALK,      "ANIMS/M_MERC/M_N_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_RUN,       "ANIMS/M_MERC/M_N_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_SWAT,      "ANIMS/M_MERC/M_N_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_CROUCH,    "ANIMS/M_MERC/M_N_CRCH.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_S_SHOT,    "ANIMS/M_MERC/M_N_SHOT.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_C_SHOT,    "ANIMS/M_MERC/M_N_C_AI.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_PRONE,     "ANIMS/M_MERC/M_N_PRNE.STI",    P_STRUCT,  0,                        8, -1),
	M(BGMDIE_JFK,           "ANIMS/M_MERC/M_DIEJFK.STI",    F_STRUCT,  0,                        8, -1),
	M(BGMOPEN,              "ANIMS/M_MERC/M_OPEN.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMPICKUP,            "ANIMS/M_MERC/M_PICKUP.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSTAB,              "ANIMS/M_MERC/M_STAB.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMSLICE,             "ANIMS/M_MERC/M_SLICE.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMCSTAB,             "ANIMS/M_MERC/M_C_STB.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMMEDIC,             "ANIMS/M_MERC/M_MEDIC.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMDODGE,             "ANIMS/M_MERC/M_DODGE.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMSTANDDWALAIM,      "ANIMS/M_MERC/M_DBLSHT.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMRAISE,             "ANIMS/M_MERC/M_RAISE.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMTHROW,             "ANIMS/M_MERC/M_THROW.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMLOB,               "ANIMS/M_MERC/M_LOB.STI",       S_STRUCT,  0,                        8, -1),
	M(BGMKICKDOOR,          "ANIMS/M_MERC/M_DR_KCK.STI",    S_STRUCT,  0,                        4, -1),
	M(BGMRHIT,              "ANIMS/M_MERC/M_R_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMSTANDAIM2,         "ANIMS/M_MERC/M_SR_AM2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMFLEX,              "ANIMS/M_MERC/M_FLEX.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMSTRECH,            "ANIMS/M_MERC/M_STRTCH.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSHOEDUST,          "ANIMS/M_MERC/M_SHOEDUST.STI",  S_STRUCT,  0,                        8, -1),
	M(BGMHEADTURN,          "ANIMS/M_MERC/M_HEDTURN.STI",   S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_WALK,      "ANIMS/M_MERC/MW_R_WAL.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_STD,       "ANIMS/M_MERC/MW_R_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_WALK,      "ANIMS/M_MERC/MW_N_WAL.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_STD,       "ANIMS/M_MERC/MW_N_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_DIE,         "ANIMS/M_MERC/MW_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_AIM,       "ANIMS/M_MERC/MW_N_SHT.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_AIM,       "ANIMS/M_MERC/MW_SR_AM.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_DBLSHT,      "ANIMS/M_MERC/MW_DBL.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMWATER_TRANS,       "ANIMS/M_MERC/MW_FALL.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_TRED,    "ANIMS/M_MERC/MW_TRED.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_SWIM,    "ANIMS/M_MERC/MW_SWIM.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_DIE,     "ANIMS/M_MERC/MW_DIE2.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMHELIDROP,          "ANIMS/M_MERC/M_HELIDRP.STI",   S_STRUCT,  0,                        1, -1),
	M(BGMSLEEP,             "ANIMS/M_MERC/M_SLEEP.STI",     P_STRUCT,  0,                        8, -1),
	M(BGMSHOOT_LOW,         "ANIMS/M_MERC/M_SHTLOW.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMTHREATENSTAND,     "ANIMS/M_MERC/M_BRETH2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMCDBLSHOT,          "ANIMS/M_MERC/MC_DBLSH.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHURTSTANDINGN,     "ANIMS/M_MERC/M_I_BR_N.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTSTANDINGR,     "ANIMS/M_MERC/M_I_BR_R.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTWALKINGN,      "ANIMS/M_MERC/M_I_WK_N.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTWALKINGR,      "ANIMS/M_MERC/M_I_WK_R.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTTRANS,         "ANIMS/M_MERC/M_I_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMTHROWKNIFE,        "ANIMS/M_MERC/M_K_THRO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMBREATHKNIFE,       "ANIMS/M_MERC/M_KNF_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLBREATH,      "ANIMS/M_MERC/M_P_BRTH.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMCOWER,             "ANIMS/M_MERC/M_COWER.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMRAISE2,            "ANIMS/M_MERC/M_RAISE2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMROCKET,            "ANIMS/M_MERC/M_LAW.STI",       S_STRUCT,  0,                        8, -1),
	M(BGMMORTAR,            "ANIMS/M_MERC/M_MORTAR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSIDESTEP,          "ANIMS/M_MERC/M_R_SDSP.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMDBLBREATH,         "ANIMS/M_MERC/M_DBL_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPUNCHLOW,          "ANIMS/M_MERC/M_PCH_LO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLSHOOTLOW,    "ANIMS/M_MERC/M_P_SHLO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATERTHROW,        "ANIMS/M_MERC/MW_THROW.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWALK2,             "ANIMS/M_MERC/M_WALK2.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMRUN2,              "ANIMS/M_MERC/M_R_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMIDLENECK,          "ANIMS/M_MERC/M_NECK.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMCROUCHTRANS,       "ANIMS/M_MERC/M_C_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMRADIO,             "ANIMS/M_MERC/M_RADIO.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMCRRADIO,           "ANIMS/M_MERC/M_C_RADO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMDWPRONE,           "ANIMS/M_MERC/M_DB_PRN.STI",    P_STRUCT,  0,                        8, -1),
	M(BGMDRUNK,             "ANIMS/M_MERC/M_R_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLDRUNK,       "ANIMS/M_MERC/M_N_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMCROWBAR,           "ANIMS/M_MERC/M_CROBAR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMJUMPOVER,          "ANIMS/M_MERC/M_N_RUN.STI",     NO_STRUCT, 0,                        8, -1),


	M(RGFWALKING,           "ANIMS/F_MERC/F_R_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSTANDING,          "ANIMS/F_MERC/F_BRETH2.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCROUCHING,         "ANIMS/F_MERC/F_R_C.STI",       C_STRUCT,  0,                        8, -1),
	M(RGFSNEAKING,          "ANIMS/F_MERC/F_R_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFRUNNING,           "ANIMS/F_MERC/F_R_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFPRONE,             "ANIMS/F_MERC/F_R_PRN.STI",     P_STRUCT,  0,                        8,  0),
	M(RGFSTANDAIM,          "ANIMS/F_MERC/F_SR_AIM.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHITHARD,           "ANIMS/F_MERC/F_DIEHD.STI",     FB_STRUCT, 0,                        8, -1),
	M(RGFHITSTAND,          "ANIMS/F_MERC/F_DIEFWD.STI",    F_STRUCT,  0,                        8, -1),
	M(RGFHITHARDBLOOD,      "ANIMS/F_MERC/F_DIEHDB.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGFCROUCHAIM,         "ANIMS/F_MERC/F_CR_AIM.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHITFALLBACK,       "ANIMS/F_MERC/F_DIEBAC.STI",    F_STRUCT,  0,                        8, -1),
	M(RGFROLLOVER,          "ANIMS/F_MERC/F_ROLL.STI",      P_STRUCT,  0,                        8, -1),
	M(RGFCLIMBROOF,         "ANIMS/F_MERC/F_CLIMB.STI",     S_STRUCT,  0,                        4, -1),
	M(RGFFALL,              "ANIMS/F_MERC/F_FALL.STI",      S_STRUCT,  0,                        4, -1),
	M(RGFFALLF,             "ANIMS/F_MERC/F_FALLF.STI",     S_STRUCT,  0,                        4, -1),
	M(RGFHITCROUCH,         "ANIMS/F_MERC/F_C_DIE.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFHITPRONE,          "ANIMS/F_MERC/F_P_DIE.STI",     P_STRUCT,  0,                        8, -1),
	M(RGFHOPFENCE,          "ANIMS/F_MERC/F_HOP.STI",       NO_STRUCT, 0,                        4, -1),
	M(RGFPUNCH,             "ANIMS/F_MERC/F_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_STD,       "ANIMS/F_MERC/N_BRETH2.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_WALK,      "ANIMS/F_MERC/F_N_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_RUN,       "ANIMS/F_MERC/F_N_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_SWAT,      "ANIMS/F_MERC/F_N_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_CROUCH,    "ANIMS/F_MERC/F_N_CRCH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_S_SHOT,    "ANIMS/F_MERC/F_N_SHOT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_C_SHOT,    "ANIMS/F_MERC/F_N_C_AI.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_PRONE,     "ANIMS/F_MERC/F_N_PRNE.STI",    P_STRUCT,  0,                        8, -1),
	M(RGFDIE_JFK,           "ANIMS/F_MERC/F_DIEJFK.STI",    F_STRUCT,  0,                        8, -1),
	M(RGFOPEN,              "ANIMS/F_MERC/F_OPEN.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFPICKUP,            "ANIMS/F_MERC/F_PICKUP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSTAB,              "ANIMS/F_MERC/F_STAB.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFSLICE,             "ANIMS/F_MERC/F_SLICE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFCSTAB,             "ANIMS/F_MERC/F_C_STB.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFMEDIC,             "ANIMS/F_MERC/F_MEDIC.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFDODGE,             "ANIMS/F_MERC/F_DODGE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFSTANDDWALAIM,      "ANIMS/F_MERC/F_DBLSHT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFRAISE,             "ANIMS/F_MERC/F_RAISE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFTHROW,             "ANIMS/F_MERC/F_THROW.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFLOB,               "ANIMS/F_MERC/F_LOB.STI",       S_STRUCT,  0,                        8, -1),
	M(RGFKICKDOOR,          "ANIMS/F_MERC/F_DR_KCK.STI",    S_STRUCT,  0,                        4, -1),
	M(RGFRHIT,              "ANIMS/F_MERC/F_R_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFCLEAN,             "ANIMS/F_MERC/A_CLEAN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFKICKSN,            "ANIMS/F_MERC/A_KICKSN.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFALOOK,             "ANIMS/F_MERC/A_LOOK.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWIPE,              "ANIMS/F_MERC/A_WIPE.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_WALK,      "ANIMS/F_MERC/FW_R_WLK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_STD,       "ANIMS/F_MERC/FW_R_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_WALK,      "ANIMS/F_MERC/FW_N_WLK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_STD,       "ANIMS/F_MERC/FW_N_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_DIE,         "ANIMS/F_MERC/FW_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_AIM,       "ANIMS/F_MERC/FW_N_SHT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_AIM,       "ANIMS/F_MERC/FW_SR_AI.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_DBLSHT,      "ANIMS/F_MERC/FW_DBL.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_TRANS,       "ANIMS/F_MERC/FW_FALL.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_TRED,    "ANIMS/F_MERC/FW_TRED.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_SWIM,    "ANIMS/F_MERC/FW_SWIM.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_DIE,     "ANIMS/F_MERC/FW_DIE2.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFHELIDROP,          "ANIMS/F_MERC/F_HELIDRP.STI",   S_STRUCT,  0,                        1, -1),
	M(RGFSLEEP,             "ANIMS/F_MERC/F_SLEEP.STI",     P_STRUCT,  0,                        8, -1),
	M(RGFSHOOT_LOW,         "ANIMS/F_MERC/F_SHTLOW.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCDBLSHOT,          "ANIMS/F_MERC/FC_DBLSH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHURTSTANDINGN,     "ANIMS/F_MERC/F_I_BR_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTSTANDINGR,     "ANIMS/F_MERC/F_I_BR_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTWALKINGN,      "ANIMS/F_MERC/F_I_WK_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTWALKINGR,      "ANIMS/F_MERC/F_I_WK_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTTRANS,         "ANIMS/F_MERC/F_I_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFTHROWKNIFE,        "ANIMS/F_MERC/F_K_THRO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFBREATHKNIFE,       "ANIMS/F_MERC/F_KNF_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLBREATH,      "ANIMS/F_MERC/F_P_BRTH.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCOWER,             "ANIMS/F_MERC/F_COWER.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFROCKET,            "ANIMS/F_MERC/F_LAW.STI",       S_STRUCT,  0,                        8, -1),
	M(RGFMORTAR,            "ANIMS/F_MERC/F_MORTAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSIDESTEP,          "ANIMS/F_MERC/F_R_SDSP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFDBLBREATH,         "ANIMS/F_MERC/F_DBL_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPUNCHLOW,          "ANIMS/F_MERC/F_PCH_LO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLSHOOTLOW,    "ANIMS/F_MERC/F_P_SHLO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATERTHROW,        "ANIMS/F_MERC/FW_THRW.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFRADIO,             "ANIMS/F_MERC/F_RADIO.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFCRRADIO,           "ANIMS/F_MERC/F_C_RADO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSLAP,              "ANIMS/F_MERC/F_SLAP.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFDWPRONE,           "ANIMS/F_MERC/F_DB_PRN.STI",    P_STRUCT,  0,                        8, -1),
	M(RGFDRUNK,             "ANIMS/F_MERC/F_R_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLDRUNK,       "ANIMS/F_MERC/F_N_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCROWBAR,           "ANIMS/F_MERC/F_CROBAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFJUMPOVER,          "ANIMS/F_MERC/F_N_RUN.STI",     NO_STRUCT, 0,                        8, -1),


	M(AFMONSTERSTANDING,    "ANIMS/MONSTERS/MN_BREAT.STI",  S_STRUCT,  0,                        8, -1),
	M(AFMONSTERWALKING,     "ANIMS/MONSTERS/MN_WALK.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMONSTERATTACK,      "ANIMS/MONSTERS/MN_ATTAK.STI",  S_STRUCT,  0,                        8, -1),
	M(AFMONSTERCLOSEATTACK, "ANIMS/MONSTERS/M_ATTK2.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMONSTERSPITATTACK,  "ANIMS/MONSTERS/M_SPIT.STI",    S_STRUCT,  0,                        8, -1),
	M(AFMONSTEREATING,      "ANIMS/MONSTERS/M_EAT.STI",     S_STRUCT,  0,                        8, -1),
	M(AFMONSTERDIE,         "ANIMS/MONSTERS/MN_DIE1.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMUP,                "ANIMS/MONSTERS/MN_UP.STI",     S_STRUCT,  0,                        8, -1),
	M(AFMJUMP,              "ANIMS/MONSTERS/MN_JUMP.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMMELT,              "ANIMS/MONSTERS/MN_MELT.STI",   S_STRUCT,  0,                        8, -1),


	M(LVBREATH,             "ANIMS/MONSTERS/L_BREATH.STI",  S_STRUCT,  0,                        8, -1),
	M(LVDIE,                "ANIMS/MONSTERS/L_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(LVWALK,               "ANIMS/MONSTERS/L_WALK.STI",    S_STRUCT,  0,                        8, -1),

	M(IBREATH,              "ANIMS/MONSTERS/I_BREATH.STI",  S_STRUCT,  0,                        8, -1),
	M(IWALK,                "ANIMS/MONSTERS/I_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(IDIE,                 "ANIMS/MONSTERS/I_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(IEAT,                 "ANIMS/MONSTERS/I_EAT.STI",     S_STRUCT,  0,                        8, -1),
	M(IATTACK,              "ANIMS/MONSTERS/I_ATTACK.STI",  S_STRUCT,  0,                        8, -1),

	M(QUEENMONSTERSTANDING, "ANIMS/MONSTERS/QMN_BREAT.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERREADY,    "ANIMS/MONSTERS/Q_READY.STI",   S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_SW,  "ANIMS/MONSTERS/Q_SPIT_SW.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_E,   "ANIMS/MONSTERS/Q_SPIT_E.STI",  S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_NE,  "ANIMS/MONSTERS/Q_SPIT_NE.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_S,   "ANIMS/MONSTERS/Q_SPIT_S.STI",  S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_SE,  "ANIMS/MONSTERS/Q_SPIT_SE.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERDEATH,    "ANIMS/MONSTERS/Q_DIE.STI",     S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSWIPE,    "ANIMS/MONSTERS/Q_SWIPE.STI",   S_STRUCT,  0,                        1, -1),

	M(FATMANSTANDING,       "ANIMS/CIVS/FT_BRTH.STI",       S_STRUCT,  0,                        8, -1),
	M(FATMANWALKING,        "ANIMS/CIVS/FT_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(FATMANRUNNING,        "ANIMS/CIVS/FT_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(FATMANDIE,            "ANIMS/CIVS/FT_DIE.STI",        F_STRUCT,  0,                        8, -1),
	M(FATMANASS,            "ANIMS/CIVS/FT_SCRTC.STI",      S_STRUCT,  0,                        8, -1),
	M(FATMANACT,            "ANIMS/CIVS/FT_ACT.STI",        S_STRUCT,  0,                        8, -1),
	M(FATMANCOWER,          "ANIMS/CIVS/FT_COWER.STI",      C_STRUCT,  0,                        8, -1),
	M(FATMANDIE2,           "ANIMS/CIVS/FT_DIE2.STI",       F_STRUCT,  0,                        8, -1),
	M(FATMANCOWERHIT,       "ANIMS/CIVS/F_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(MANCIVSTANDING,       "ANIMS/CIVS/M_BREATH.STI",      S_STRUCT,  0,                        8, -1),
	M(MANCIVWALKING,        "ANIMS/CIVS/M_WALK.STI",        S_STRUCT,  0,                        8, -1),
	M(MANCIVRUNNING,        "ANIMS/CIVS/M_RUN.STI",         S_STRUCT,  0,                        8, -1),
	M(MANCIVDIE,            "ANIMS/CIVS/M_DIE.STI",         F_STRUCT,  0,                        8, -1),
	M(MANCIVACT,            "ANIMS/CIVS/M_ACT.STI",         S_STRUCT,  0,                        8, -1),
	M(MANCIVCOWER,          "ANIMS/CIVS/M_COWER.STI",       C_STRUCT,  0,                        8, -1),
	M(MANCIVDIE2,           "ANIMS/CIVS/M_DIE2.STI",        F_STRUCT,  0,                        8, -1),
	M(MANCIVSMACKED,        "ANIMS/CIVS/M_SMCKED.STI",      S_STRUCT,  0,                        8, -1),
	M(MANCIVPUNCH,          "ANIMS/CIVS/M_PUNCH.STI",       S_STRUCT,  0,                        8, -1),
	M(MANCIVCOWERHIT,       "ANIMS/CIVS/M_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(MINICIVSTANDING,      "ANIMS/CIVS/MI_BREATH.STI",     S_STRUCT,  0,                        8, -1),
	M(MINICIVWALKING,       "ANIMS/CIVS/MI_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(MINICIVRUNNING,       "ANIMS/CIVS/MI_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(MINICIVDIE,           "ANIMS/CIVS/MI_DIE.STI",        F_STRUCT,  0,                        8, -1),
	M(MINISTOCKING,         "ANIMS/CIVS/MI_STKNG.STI",      S_STRUCT,  0,                        8, -1),
	M(MINIACT,              "ANIMS/CIVS/MI_ACT.STI",        S_STRUCT,  0,                        8, -1),
	M(MINICOWER,            "ANIMS/CIVS/MI_COWER.STI",      C_STRUCT,  0,                        8, -1),
	M(MINIDIE2,             "ANIMS/CIVS/MI_DIE2.STI",       F_STRUCT,  0,                        8, -1),
	M(MINICOWERHIT,         "ANIMS/CIVS/S_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(DRESSCIVSTANDING,     "ANIMS/CIVS/DS_BREATH.STI",     S_STRUCT,  0,                        8, -1),
	M(DRESSCIVWALKING,      "ANIMS/CIVS/DS_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(DRESSCIVRUNNING,      "ANIMS/CIVS/DS_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(DRESSCIVDIE,          "ANIMS/CIVS/DS_DIE.STI",        F_STRUCT,  0,                        8, -1),
	M(DRESSCIVACT,          "ANIMS/CIVS/DS_ACT.STI",        S_STRUCT,  0,                        8, -1),
	M(DRESSCIVCOWER,        "ANIMS/CIVS/DS_COWER.STI",      C_STRUCT,  0,                        8, -1),
	M(DRESSCIVDIE2,         "ANIMS/CIVS/DS_DIE2.STI",       F_STRUCT,  0,                        8, -1),
	M(DRESSCIVCOWERHIT,     "ANIMS/CIVS/W_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(HATKIDCIVSTANDING,    "ANIMS/CIVS/H_BREATH.STI",      S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVWALKING,     "ANIMS/CIVS/H_WALK.STI",        S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVRUNNING,     "ANIMS/CIVS/H_RUN.STI",         S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVDIE,         "ANIMS/CIVS/H_DIE2.STI",        F_STRUCT,  0,                        8, -1),
	M(HATKIDCIVJFK,         "ANIMS/CIVS/H_DIEJFK.STI",      S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVYOYO,        "ANIMS/CIVS/H_YOYO.STI",        S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVACT,         "ANIMS/CIVS/H_ACT.STI",         S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVCOWER,       "ANIMS/CIVS/H_COWER.STI",       S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVDIE2,        "ANIMS/CIVS/H_DIE.STI",         F_STRUCT,  0,                        8, -1),
	M(HATKIDCIVCOWERHIT,    "ANIMS/CIVS/H_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),
	M(HATKIDCIVSKIP,        "ANIMS/CIVS/H_SKIP.STI",        C_STRUCT,  0,                        8, -1),

	M(KIDCIVSTANDING,       "ANIMS/CIVS/K_BREATH.STI",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVWALKING,        "ANIMS/CIVS/K_WALK.STI",        S_STRUCT,  0,                        8, -1),
	M(KIDCIVRUNNING,        "ANIMS/CIVS/K_RUN.STI",         S_STRUCT,  0,                        8, -1),
	M(KIDCIVDIE,            "ANIMS/CIVS/K_DIE2.STI",        F_STRUCT,  0,                        8, -1),
	M(KIDCIVJFK,            "ANIMS/CIVS/K_DIEJFK.STI",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVARMPIT,         "ANIMS/CIVS/K_ARMPIT.STI",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVACT,            "ANIMS/CIVS/K_ACT.STI",         S_STRUCT,  0,                        8, -1),
	M(KIDCIVCOWER,          "ANIMS/CIVS/K_COWER.STI",       S_STRUCT,  0,                        8, -1),
	M(KIDCIVDIE2,           "ANIMS/CIVS/K_DIE.STI",         F_STRUCT,  0,                        8, -1),
	M(KIDCIVCOWERHIT,       "ANIMS/CIVS/K_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),
	M(KIDCIVSKIP,           "ANIMS/CIVS/K_SKIP.STI",        C_STRUCT,  0,                        8, -1),

	M(CRIPCIVSTANDING,      "ANIMS/CIVS/CP_BRETH.STI",      S_STRUCT,  0,                        8, -1),
	M(CRIPCIVWALKING,       "ANIMS/CIVS/CP_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(CRIPCIVRUNNING,       "ANIMS/CIVS/CP_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVBEG,           "ANIMS/CIVS/CP_BEG.STI",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVDIE,           "ANIMS/CIVS/CP_DIE.STI",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVDIE2,          "ANIMS/CIVS/CP_DIE2.STI",       S_STRUCT,  0,                        8, -1),
	M(CRIPCIVKICK,          "ANIMS/CIVS/CP_KICK.STI",       S_STRUCT,  0,                        8, -1),

	M(COWSTANDING,          "ANIMS/ANIMALS/C_BREATH.STI",   S_STRUCT,  0,                        8, -1),
	M(COWWALKING,           "ANIMS/ANIMALS/C_WALK.STI",     S_STRUCT,  0,                        8, -1),
	M(COWDIE,               "ANIMS/ANIMALS/C_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(COWEAT,               "ANIMS/ANIMALS/C_EAT.STI",      S_STRUCT,  0,                        8, -1),

	M(CROWWALKING,          "ANIMS/ANIMALS/CR_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(CROWFLYING,           "ANIMS/ANIMALS/CR_FLY.STI",     S_STRUCT,  0,                        8, -1),
	M(CROWEATING,           "ANIMS/ANIMALS/CR_EAT.STI",     S_STRUCT,  0,                        8, -1),
	M(CROWDYING,            "ANIMS/ANIMALS/CR_DIE.STI",     NO_STRUCT, 0,                        8, -1),

	M(CATBREATH,            "ANIMS/ANIMALS/CT_BREATH.STI",  S_STRUCT,  0,                        8, -1),
	M(CATWALK,              "ANIMS/ANIMALS/CT_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(CATRUN,               "ANIMS/ANIMALS/CT_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(CATREADY,             "ANIMS/ANIMALS/CT_READY.STI",   S_STRUCT,  0,                        8, -1),
	M(CATHIT,               "ANIMS/ANIMALS/CT_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(CATDIE,               "ANIMS/ANIMALS/CT_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(CATSWIPE,             "ANIMS/ANIMALS/CT_SWIPE.STI",   S_STRUCT,  0,                        8, -1),
	M(CATBITE,              "ANIMS/ANIMALS/CT_BITE.STI",    S_STRUCT,  0,                        8, -1),

	M(ROBOTNWBREATH,        "ANIMS/CIVS/J_R_BRET.STI",      S_STRUCT,  0,                        8, -1),
	M(ROBOTNWWALK,          "ANIMS/CIVS/J_R_WALK.STI",      S_STRUCT,  0,                        8, -1),
	M(ROBOTNWHIT,           "ANIMS/CIVS/J_R_HIT.STI"  ,     S_STRUCT,  0,                        8, -1),
	M(ROBOTNWDIE,           "ANIMS/CIVS/J_R_DIE.STI",       S_STRUCT,  0,                        8, -1),
	M(ROBOTNWSHOOT,         "ANIMS/CIVS/J_R_SHOT.STI",      S_STRUCT,  0,                        8, -1),

	M(HUMVEE_BASIC,         "ANIMS/VEHICLES/HUMMER2.STI",   S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(HUMVEE_DIE,           "ANIMS/VEHICLES/HM_WREK.STI",   S_STRUCT,  0,                        2, -1),

	M(TANKNW_READY,         "ANIMS/VEHICLES/TANK_ROT.STI",  S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(TANKNW_SHOOT,         "ANIMS/VEHICLES/TANK_SHT.STI",  S_STRUCT,  0,                        8, -1),
	M(TANKNW_DIE,           "ANIMS/VEHICLES/TK_WREK.STI",   S_STRUCT,  0,                        1, -1),

	M(TANKNE_READY,         "ANIMS/VEHICLES/TNK2_ROT.STI",  S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(TANKNE_SHOOT,         "ANIMS/VEHICLES/TNK2_SHT.STI",  S_STRUCT,  0,                        8, -1),
	M(TANKNE_DIE,           "ANIMS/VEHICLES/TK2_WREK.STI",  S_STRUCT,  0,                        1, -1),

	M(ELDORADO_BASIC,       "ANIMS/VEHICLES/HUMMER.STI",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(ELDORADO_DIE,         "ANIMS/VEHICLES/HM_WREK.STI",   NO_STRUCT, 0,                        2, -1),

	M(ICECREAMTRUCK_BASIC,  "ANIMS/VEHICLES/ICECRM.STI",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(ICECREAMTRUCK_DIE,    "ANIMS/VEHICLES/HM_WREK.STI",   NO_STRUCT, 0,                        2, -1),

	M(JEEP_BASIC,           "ANIMS/VEHICLES/HUMMER.STI",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(JEEP_DIE,             "ANIMS/VEHICLES/HM_WREK.STI",   NO_STRUCT, 0,                        2, -1),

	M(BODYEXPLODE,          "ANIMS/S_MERC/BOD_BLOW.STI",    NO_STRUCT, 0,                        1, -1),
};

#undef M


typedef struct
{
	const char* const   Filename;
	STRUCTURE_FILE_REF* pStructureFileRef;
} AnimationStructureType;


#define PATH_STRUCT "ANIMS/STRUCTDATA/"
#define SUFFIX      ".JSD"
#define ABCDEF(a, b, c, d, e, f)  \
{                                 \
	{ PATH_STRUCT a SUFFIX, NULL }, \
	{ PATH_STRUCT b SUFFIX, NULL }, \
	{ PATH_STRUCT c SUFFIX, NULL }, \
	{ PATH_STRUCT d SUFFIX, NULL }, \
	{ PATH_STRUCT e SUFFIX, NULL }, \
	{ PATH_STRUCT f SUFFIX, NULL }  \
}
#define AAAAAA(a)          ABCDEF(a, a, a, a, a, a)
#define AAAAAB(a, b)       ABCDEF(a, a, a, a, a, b)
#define AAABCA(a, b, c)    ABCDEF(a, a, a, b, c, a)
#define ABBCCC(a, b, c)    ABCDEF(a, b, b, c, c, c)
#define ABCCCD(a, b, c, d) ABCDEF(a, b, c, c, c, d)

static AnimationStructureType gAnimStructureDatabase[TOTALBODYTYPES][NUM_STRUCT_IDS] =
{
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // Normal Male
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // Big male
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // Stocky male
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // Reg Female
	AAAAAB("MN_BREAT",  "M_CROUCH"),                                                // Adult female creature
	AAAAAB("MN_BREAT",  "M_CROUCH"),                                                // Adult male creature
	AAAAAB("MN_BREAT",  "M_CROUCH"),                                                // Young Adult female creature
	AAAAAB("MN_BREAT",  "M_CROUCH"),                                                // Young Adult male creature
	AAAAAB("L_BREATH",  "M_CROUCH"),                                                // larvea creature
	AAAAAA("I_BREATH"),                                                             // infant creature
	AAAAAB("Q_READY",   "M_CROUCH"),                                                // Queen creature
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // Fat civ
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // man civ
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // miniskirt civ
	ABCDEF("M_STAND",   "M_CROUCH", "M_PRONE", "M_FALL", "M_FALLBACK", "M_CROUCH"), // dress civ
	ABBCCC("K_STAND",   "K_CROUCH", "M_PRONE"),                                     // kid civ
	ABBCCC("K_STAND",   "K_CROUCH", "M_PRONE"),                                     // hat kid civ
	AAABCA("M_CROUCH",  "M_FALL", "M_FALLBACK"),                                    // cripple civ
	AAAAAB("CW_BREATH", "M_CROUCH"),                                                // cow
	ABCCCD("CR_STAND",  "CR_CROUCH", "CR_PRONE", "M_CROUCH"),                       // crow
	AAAAAB("CT_BREATH", "M_CROUCH"),                                                // CAT
	AAAAAA("J_R_BRET"),                                                             // ROBOT1
	AAAAAB("HMMV",      "M_CROUCH"),                                                // vech 1
	AAAAAB("TNK_SHT",   "M_CROUCH"),                                                // tank 1
	AAAAAB("TNK2_ROT",  "M_CROUCH"),                                                // tank 2
	AAAAAB("HMMV",      "M_CROUCH"),                                                // ELDORADO
	AAAAAB("HMMV",      "M_CROUCH"),                                                // ICECREAMTRUCK
	AAAAAB("HMMV",      "M_CROUCH")                                                 // JEEP
};

#undef ABCCCD
#undef AAABCA
#undef ABBCCC
#undef AAAAAB
#undef AAAAAA
#undef ABCDEF
#undef SUFFIX
#undef PATH_STRUCT


static void LoadAnimationProfiles(void);


void InitAnimationSystem()
{
	INT32									cnt1, cnt2;

	LoadAnimationStateInstructions();
	InitAnimationSurfacesPerBodytype();
	LoadAnimationProfiles();

	// OK, Load all animation structures.....
	for ( cnt1 = 0; cnt1 < TOTALBODYTYPES; cnt1++ )
	{
		for ( cnt2 = 0; cnt2 < NUM_STRUCT_IDS; cnt2++ )
		{
			const char* Filename = gAnimStructureDatabase[cnt1][cnt2].Filename;

			if (FileExists(Filename))
			{
				STRUCTURE_FILE_REF* pStructureFileRef = LoadStructureFile(Filename);
				gAnimStructureDatabase[ cnt1 ][ cnt2 ].pStructureFileRef = pStructureFileRef;
			}
		}
	}
}


static void DeleteAnimationProfiles(void);


void DeInitAnimationSystem()
{
	for (AnimationSurfaceType* i = gAnimSurfaceDatabase; i != endof(gAnimSurfaceDatabase); ++i)
	{
		SGPVObject*& vo = i->hVideoObject;
		if (!vo) continue;
		DeleteVideoObject(vo);
		vo = 0;
	}

	// Delete all animation structures
	// ATE: Don't delete here, will be deleted when the structure database is destoryed
#if 0 // XXX was commented out
	for (INT32 i = 0; i < TOTALBODYTYPES; ++i)
	{
		for (INT32 k = 0; k < 3; k++) // XXX 3 seems wrong, should be NUM_STRUCT_IDS?
		{
			STRUCTURE_FILE_REF*& sfr = gAnimStructureDatabase[i][k].pStructureFileRef;
			if (!sfr) continue;
			FreeStructureFile(sfr);
			sfr = 0;
		}
	}
#endif

	DeleteAnimationProfiles();
}


static STRUCTURE_FILE_REF* InternalGetAnimationStructureRef(const SOLDIERTYPE* const s, const UINT16 usSurfaceIndex, const UINT16 usAnimState, const BOOLEAN fUseAbsolute)
{
	INT8	bStructDataType;

	if ( usSurfaceIndex == INVALID_ANIMATION_SURFACE )
	{
		return( NULL );
	}

	bStructDataType = gAnimSurfaceDatabase[ usSurfaceIndex ].bStructDataType;

	if ( bStructDataType == NO_STRUCT )
	{
		return( NULL );
	}

	// ATE: Alright - we all hate exception coding but ness here...
	// return STANDING struct for these - which start standing but end prone
	// CJC August 14 2002: added standing burst hit to this list
	if (!fUseAbsolute)
	{
		switch (usAnimState)
		{
			case FALLFORWARD_FROMHIT_STAND:
			case GENERIC_HIT_STAND:
			case FALLFORWARD_FROMHIT_CROUCH:
			case STANDING_BURST_HIT:
				bStructDataType = S_STRUCT;
		}
	}

	return gAnimStructureDatabase[s->ubBodyType][bStructDataType].pStructureFileRef;
}


STRUCTURE_FILE_REF* GetAnimationStructureRef(const SOLDIERTYPE* const s, const UINT16 usSurfaceIndex, const UINT16 usAnimState)
{
	return InternalGetAnimationStructureRef(s, usSurfaceIndex, usAnimState, FALSE);
}


// Surface mamagement functions
void LoadAnimationSurface(UINT16 const usSoldierID, UINT16 const usSurfaceIndex, UINT16 const usAnimState)
{
	if (usSurfaceIndex >= NUMANIMATIONSURFACETYPES)
	{
		throw std::logic_error("Invalid surface index");
	}

	AnimationSurfaceType* const a = &gAnimSurfaceDatabase[usSurfaceIndex];

	// Check if surface is loaded
	if (a->hVideoObject != NULL)
	{
		// just increment usage counter ( below )
		AnimDebugMsg(String("Surface Database: Hit %d", usSurfaceIndex));
	}
	else
	{
		try
		{
			// Load into memory
			AnimDebugMsg(String("Surface Database: Loading %d", usSurfaceIndex));

			AutoSGPImage   hImage(CreateImage(a->Filename, IMAGE_ALLDATA));
			AutoSGPVObject hVObject(AddVideoObjectFromHImage(hImage));

			// Get aux data
			if (hImage->uiAppDataSize != hVObject->SubregionCount() * sizeof(AuxObjectData))
			{
				throw std::runtime_error("Invalid # of animations given");
			}

			// Valid auxiliary data, so get # of frames from data
			AuxObjectData const* const pAuxData = (AuxObjectData const*)(UINT8 const*)hImage->pAppData;
			a->uiNumFramesPerDir = pAuxData->ubNumberOfFrames;

			// get structure data if any
			const STRUCTURE_FILE_REF* const pStructureFileRef = InternalGetAnimationStructureRef(ID2SOLDIER(usSoldierID), usSurfaceIndex, usAnimState, TRUE);
			if (pStructureFileRef != NULL)
			{
				INT16 sStartFrame = 0;
				if (usSurfaceIndex == RGMPRONE)
				{
					sStartFrame = 5;
				}
				else if (usSurfaceIndex >= QUEENMONSTERSTANDING && usSurfaceIndex <= QUEENMONSTERSWIPE)
				{
					sStartFrame = -1;
				}

				AddZStripInfoToVObject(hVObject, pStructureFileRef, TRUE, sStartFrame);
			}

			// Set video object index
			a->hVideoObject = hVObject.Release();

			// Determine if we have a problem with #frames + directions ( ie mismatch )
			if (a->uiNumDirections * a->uiNumFramesPerDir != a->hVideoObject->SubregionCount())
			{
				AnimDebugMsg(String("Surface Database: WARNING!!! Surface %d has #frames mismatch.", usSurfaceIndex));
			}
		}
		catch (...)
		{
			SET_ERROR("Could not load animation file: %s", a->Filename);
			throw;
		}
	}

	// Increment usage count only if history for soldier is not yet set
	if (gbAnimUsageHistory[usSurfaceIndex][usSoldierID] == 0)
	{
		AnimDebugMsg(String("Surface Database: Incrementing Usage %d ( Soldier %d )", usSurfaceIndex, usSoldierID));
		// Increment usage count
		++a->bUsageCount;
		// Set history for particular sodlier
		++gbAnimUsageHistory[usSurfaceIndex][usSoldierID];
	}
}


void UnLoadAnimationSurface(const UINT16 usSoldierID, const UINT16 usSurfaceIndex)
{
	// Decrement usage flag, only if this soldier has it currently tagged
	INT8* const in_use = &gbAnimUsageHistory[usSurfaceIndex][usSoldierID];
	if (*in_use <= 0)
	{
		// Return warning that we have not actually loaded the surface previously
		AnimDebugMsg("Surface Database: WARNING!!! Soldier has tried to unlock surface that he has not locked.");
		return;
	}
	*in_use = 0; // Set history for particular sodlier

	AnimDebugMsg(String("Surface Database: Decrementing Usage %d ( Soldier %d )", usSurfaceIndex, usSoldierID));

	AnimationSurfaceType* const a         = &gAnimSurfaceDatabase[usSurfaceIndex];
	INT8*                 const use_count = &a->bUsageCount;
	--*use_count;

	AnimDebugMsg(String("Surface Database: MercUsage: %d, Global Uasage: %d", *in_use, *use_count));

	Assert(*use_count >= 0);
	if (*use_count < 0) *use_count = 0;

	// Delete if count reched zero
	if (*use_count == 0)
	{
		AnimDebugMsg(String("Surface Database: Unloading Surface: %d", usSurfaceIndex));
		SGPVObject** const vo = &a->hVideoObject;
		CHECKV(*vo != NULL);
		DeleteVideoObject(*vo);
		*vo = NULL;
	}
}


void ClearAnimationSurfacesUsageHistory( UINT16 usSoldierID )
{
	UINT32 cnt;

	for( cnt = 0; cnt < NUMANIMATIONSURFACETYPES; cnt++ )
	{
		gbAnimUsageHistory[ cnt ][ usSoldierID ] = 0;
	}
}


static void LoadAnimationProfiles(void)
try
{
	AutoSGPFile f(FileOpen(ANIMPROFILEFILENAME, FILE_ACCESS_READ));

	FileRead(f, &gubNumAnimProfiles, sizeof(gubNumAnimProfiles));

	ANIM_PROF* const aps = MALLOCN(ANIM_PROF, gubNumAnimProfiles);
	gpAnimProfiles = aps;

	for (INT32 profile_idx = 0; profile_idx < gubNumAnimProfiles; ++profile_idx)
	{
		ANIM_PROF* const ap = &aps[profile_idx];
		for (INT32 direction_idx = 0; direction_idx < 8; ++direction_idx)
		{
			ANIM_PROF_DIR* const apd = &ap->Dirs[direction_idx];

			FileRead(f, &apd->ubNumTiles, sizeof(UINT8));
			ANIM_PROF_TILE* const apts = MALLOCN(ANIM_PROF_TILE, apd->ubNumTiles);
			apd->pTiles = apts;

			for (INT32 tile_idx = 0; tile_idx < apd->ubNumTiles; ++tile_idx)
			{
				ANIM_PROF_TILE* const apt = &apts[tile_idx];
				FileRead(f, &apt->usTileFlags, sizeof(UINT16));
				FileRead(f, &apt->bTileX,      sizeof(INT8));
				FileRead(f, &apt->bTileY,      sizeof(INT8));
			}
		}
	}
}
catch (...)
{
	SET_ERROR("Problems initializing Animation Profiles");
	throw;
}


static void DeleteAnimationProfiles(void)
{
	INT32				iProfileCount, iDirectionCount;
	ANIM_PROF_DIR			*pProfileDir;

	// Loop profiles
	for ( iProfileCount = 0; iProfileCount < gubNumAnimProfiles; iProfileCount++ )
	{
		// Loop directions
		for ( iDirectionCount = 0; iDirectionCount < 8; iDirectionCount++ )
		{
			// Get prodile direction pointer
			pProfileDir = &( gpAnimProfiles[ iProfileCount ].Dirs[ iDirectionCount ] );

			// Free tile
			MemFree( pProfileDir->pTiles );

		}
	}

	// Free profile data!
	MemFree( gpAnimProfiles );

}


void ZeroAnimSurfaceCounts( )
{
  INT32 cnt;

  for ( cnt = 0; cnt < NUMANIMATIONSURFACETYPES; cnt++ )
  {
    gAnimSurfaceDatabase[ cnt ].bUsageCount   = 0;
    gAnimSurfaceDatabase[ cnt ].hVideoObject  = NULL;
  }

  memset( gbAnimUsageHistory, 0, sizeof( gbAnimUsageHistory ) );
}
