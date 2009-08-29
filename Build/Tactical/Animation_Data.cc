#include <stdexcept>

#include "Directories.h"
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

#define ANIMPROFILEFILENAME BINARYDATADIR "/ja2prof.dat"



ANIM_PROF		*gpAnimProfiles = NULL;
static UINT8 gubNumAnimProfiles = 0;

INT8				gbAnimUsageHistory[ NUMANIMATIONSURFACETYPES ][ MAX_NUM_SOLDIERS ];


#define M(name, file, type, flags, dir, profile) { name, file, type, flags, dir, TO_INIT, NULL, 0, profile }

AnimationSurfaceType gAnimSurfaceDatabase[NUMANIMATIONSURFACETYPES] =
{
	M(RGMBASICWALKING,      ANIMSDIR "/S_MERC/S_R_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSTANDING,          ANIMSDIR "/S_MERC/S_R_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCROUCHING,         ANIMSDIR "/S_MERC/S_R_C.STI",       C_STRUCT,  0,                        8, -1),
	M(RGMSNEAKING,          ANIMSDIR "/S_MERC/S_R_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMRUNNING,           ANIMSDIR "/S_MERC/S_R_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMPRONE,             ANIMSDIR "/S_MERC/S_R_PRN.STI",     P_STRUCT,  0,                        8,  0),
	M(RGMSTANDAIM,          ANIMSDIR "/S_MERC/S_SR_AIM.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHITHARD,           ANIMSDIR "/S_MERC/S_DIEHD.STI",     FB_STRUCT, 0,                        8, -1),
	M(RGMHITSTAND,          ANIMSDIR "/S_MERC/S_DIEFWD.STI",    F_STRUCT,  0,                        8, -1),
	M(RGMHITHARDBLOOD,      ANIMSDIR "/S_MERC/S_DIEHDB.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGMCROUCHAIM,         ANIMSDIR "/S_MERC/S_CR_AIM.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHITFALLBACK,       ANIMSDIR "/S_MERC/S_DIEBAC.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGMROLLOVER,          ANIMSDIR "/S_MERC/S_ROLL.STI",      P_STRUCT,  0,                        8, -1),
	M(RGMCLIMBROOF,         ANIMSDIR "/S_MERC/S_CLIMB.STI",     S_STRUCT,  0,                        4, -1),
	M(RGMFALL,              ANIMSDIR "/S_MERC/S_FALL.STI",      S_STRUCT,  0,                        4, -1),
	M(RGMFALLF,             ANIMSDIR "/S_MERC/S_FALLF.STI",     S_STRUCT,  0,                        4, -1),
	M(RGMHITCROUCH,         ANIMSDIR "/S_MERC/S_C_DIE.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMHITPRONE,          ANIMSDIR "/S_MERC/S_P_DIE.STI",     P_STRUCT,  0,                        8, -1),
	M(RGMHOPFENCE,          ANIMSDIR "/S_MERC/S_HOP.STI",       NO_STRUCT, 0,                        4, -1),
	M(RGMPUNCH,             ANIMSDIR "/S_MERC/S_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_STD,       ANIMSDIR "/S_MERC/S_N_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_WALK,      ANIMSDIR "/S_MERC/S_N_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_RUN,       ANIMSDIR "/S_MERC/S_N_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_SWAT,      ANIMSDIR "/S_MERC/S_N_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_CROUCH,    ANIMSDIR "/S_MERC/S_N_CRCH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_S_SHOT,    ANIMSDIR "/S_MERC/S_N_SHOT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_C_SHOT,    ANIMSDIR "/S_MERC/S_N_C_AI.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_PRONE,     ANIMSDIR "/S_MERC/S_N_PRNE.STI",    P_STRUCT,  0,                        8, -1),
	M(RGMDIE_JFK,           ANIMSDIR "/S_MERC/S_DIEJFK.STI",    F_STRUCT,  0,                        8, -1),
	M(RGMOPEN,              ANIMSDIR "/S_MERC/S_OPEN.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMPICKUP,            ANIMSDIR "/S_MERC/S_PICKUP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSTAB,              ANIMSDIR "/S_MERC/S_STAB.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMSLICE,             ANIMSDIR "/S_MERC/S_SLICE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCSTAB,             ANIMSDIR "/S_MERC/S_C_STB.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMMEDIC,             ANIMSDIR "/S_MERC/S_MEDIC.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMDODGE,             ANIMSDIR "/S_MERC/S_DODGE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMSTANDDWALAIM,      ANIMSDIR "/S_MERC/S_DBLSHT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMRAISE,             ANIMSDIR "/S_MERC/S_RAISE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMTHROW,             ANIMSDIR "/S_MERC/S_LOB.STI",       S_STRUCT,  0,                        8, -1),
	M(RGMLOB,               ANIMSDIR "/S_MERC/S_THROW.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMKICKDOOR,          ANIMSDIR "/S_MERC/S_DR_KCK.STI",    S_STRUCT,  0,                        4, -1),
	M(RGMRHIT,              ANIMSDIR "/S_MERC/S_R_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(RGM_SQUISH,           ANIMSDIR "/S_MERC/A_SQUISH.STI",    S_STRUCT,  0,                        8, -1),
	M(RGM_LOOK,             ANIMSDIR "/S_MERC/A_LOOK.STI",      S_STRUCT,  0,                        8, -1),
	M(RGM_PULL,             ANIMSDIR "/S_MERC/A_PULL.STI",      S_STRUCT,  0,                        8, -1),
	M(RGM_SPIT,             ANIMSDIR "/S_MERC/A_SPIT.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_WALK,      ANIMSDIR "/S_MERC/SW_R_WALK.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_STD,       ANIMSDIR "/S_MERC/SW_R_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_WALK,      ANIMSDIR "/S_MERC/SW_N_WALK.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_STD,       ANIMSDIR "/S_MERC/SW_N_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMWATER_DIE,         ANIMSDIR "/S_MERC/SW_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_AIM,       ANIMSDIR "/S_MERC/SW_N_SHOT.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_AIM,       ANIMSDIR "/S_MERC/SW_SR_AIM.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_DBLSHT,      ANIMSDIR "/S_MERC/SW_DBLSHT.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_TRANS,       ANIMSDIR "/S_MERC/SW_FALL.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_TRED,    ANIMSDIR "/S_MERC/S_TRED.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_SWIM,    ANIMSDIR "/S_MERC/S_SWIM.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_DIE,     ANIMSDIR "/S_MERC/S_D_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMMCLIMB,            ANIMSDIR "/S_MERC/S_MCLIMB.STI",    S_STRUCT,  0,                        3, -1),
	M(RGMHELIDROP,          ANIMSDIR "/S_MERC/S_HELIDRP.STI",   S_STRUCT,  0,                        1, -1),
	M(RGMLOWKICK,           ANIMSDIR "/S_MERC/K_LW_KICK.STI",   S_STRUCT,  0,                        8, -1),
	M(RGMPUNCH,             ANIMSDIR "/S_MERC/K_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMSPINKICK,          ANIMSDIR "/S_MERC/S_SPNKCK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSLEEP,             ANIMSDIR "/S_MERC/S_SLEEP.STI",     P_STRUCT,  0,                        8, -1),
	M(RGMSHOOT_LOW,         ANIMSDIR "/S_MERC/S_SHTLO.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCDBLSHOT,          ANIMSDIR "/S_MERC/SC_DBLSH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGMHURTSTANDINGN,     ANIMSDIR "/S_MERC/S_I_BR_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTSTANDINGR,     ANIMSDIR "/S_MERC/S_I_BR_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTWALKINGN,      ANIMSDIR "/S_MERC/S_I_WK_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTWALKINGR,      ANIMSDIR "/S_MERC/S_I_WK_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTTRANS,         ANIMSDIR "/S_MERC/S_I_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMTHROWKNIFE,        ANIMSDIR "/S_MERC/S_K_THRO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMBREATHKNIFE,       ANIMSDIR "/S_MERC/S_KNF_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLBREATH,      ANIMSDIR "/S_MERC/S_P_BRTH.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMCOWER,             ANIMSDIR "/S_MERC/S_COWER.STI",     C_STRUCT,  0,                        8, -1),
	M(RGMROCKET,            ANIMSDIR "/S_MERC/S_LAW.STI",       S_STRUCT,  0,                        8, -1),
	M(RGMMORTAR,            ANIMSDIR "/S_MERC/S_MORTAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMSIDESTEP,          ANIMSDIR "/S_MERC/S_R_SDSP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMDBLBREATH,         ANIMSDIR "/S_MERC/S_DBL_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPUNCHLOW,          ANIMSDIR "/S_MERC/S_PCH_LO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLSHOOTLOW,    ANIMSDIR "/S_MERC/S_P_SHLO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMWATERTHROW,        ANIMSDIR "/S_MERC/SW_LOB.STI",      S_STRUCT,  0,                        8, -1),
	M(RGMRADIO,             ANIMSDIR "/S_MERC/S_RADIO.STI",     S_STRUCT,  0,                        8, -1),
	M(RGMCRRADIO,           ANIMSDIR "/S_MERC/S_C_RADO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMBURN,              ANIMSDIR "/S_MERC/S_FIRE.STI",      S_STRUCT,  0,                        1, -1),
	M(RGMDWPRONE,           ANIMSDIR "/S_MERC/S_DB_PRN.STI",    P_STRUCT,  0,                        8, -1),
	M(RGMDRUNK,             ANIMSDIR "/S_MERC/S_R_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLDRUNK,       ANIMSDIR "/S_MERC/S_N_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMCROWBAR,           ANIMSDIR "/S_MERC/S_CROBAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGMJUMPOVER,          ANIMSDIR "/S_MERC/S_N_RUN.STI",     NO_STRUCT, 0,                        8, -1),


	M(BGMWALKING,           ANIMSDIR "/M_MERC/M_R_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSTANDING,          ANIMSDIR "/M_MERC/M_R_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMCROUCHING,         ANIMSDIR "/M_MERC/M_R_C.STI",       C_STRUCT,  0,                        8, -1),
	M(BGMSNEAKING,          ANIMSDIR "/M_MERC/M_R_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMRUNNING,           ANIMSDIR "/M_MERC/M_R_RUN2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPRONE,             ANIMSDIR "/M_MERC/M_R_PRN.STI",     P_STRUCT,  0,                        8,  0),
	M(BGMSTANDAIM,          ANIMSDIR "/M_MERC/M_SR_AIM.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHITHARD,           ANIMSDIR "/M_MERC/M_DIEHD.STI",     FB_STRUCT, 0,                        8, -1),
	M(BGMHITSTAND,          ANIMSDIR "/M_MERC/M_DIEFWD.STI",    F_STRUCT,  0,                        8, -1),
	M(BGMHITHARDBLOOD,      ANIMSDIR "/M_MERC/M_DIEHDB.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGMCROUCHAIM,         ANIMSDIR "/M_MERC/M_CR_AIM.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHITFALLBACK,       ANIMSDIR "/M_MERC/M_DIEBAC.STI",    FB_STRUCT, 0,                        8, -1),
	M(BGMROLLOVER,          ANIMSDIR "/M_MERC/M_ROLL.STI",      P_STRUCT,  0,                        8, -1),
	M(BGMCLIMBROOF,         ANIMSDIR "/M_MERC/M_CLIMB.STI",     S_STRUCT,  0,                        4, -1),
	M(BGMFALL,              ANIMSDIR "/M_MERC/M_FALL.STI",      S_STRUCT,  0,                        4, -1),
	M(BGMFALLF,             ANIMSDIR "/M_MERC/M_FALLF.STI",     S_STRUCT,  0,                        4, -1),
	M(BGMHITCROUCH,         ANIMSDIR "/M_MERC/M_C_DIE.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMHITPRONE,          ANIMSDIR "/M_MERC/M_P_DIE.STI",     P_STRUCT,  0,                        8, -1),
	M(BGMHOPFENCE,          ANIMSDIR "/M_MERC/M_HOP.STI",       NO_STRUCT, 0,                        4, -1),
	M(BGMPUNCH,             ANIMSDIR "/M_MERC/M_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_STD,       ANIMSDIR "/M_MERC/M_N_STD.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_WALK,      ANIMSDIR "/M_MERC/M_N_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_RUN,       ANIMSDIR "/M_MERC/M_N_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_SWAT,      ANIMSDIR "/M_MERC/M_N_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_CROUCH,    ANIMSDIR "/M_MERC/M_N_CRCH.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_S_SHOT,    ANIMSDIR "/M_MERC/M_N_SHOT.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_C_SHOT,    ANIMSDIR "/M_MERC/M_N_C_AI.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_PRONE,     ANIMSDIR "/M_MERC/M_N_PRNE.STI",    P_STRUCT,  0,                        8, -1),
	M(BGMDIE_JFK,           ANIMSDIR "/M_MERC/M_DIEJFK.STI",    F_STRUCT,  0,                        8, -1),
	M(BGMOPEN,              ANIMSDIR "/M_MERC/M_OPEN.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMPICKUP,            ANIMSDIR "/M_MERC/M_PICKUP.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSTAB,              ANIMSDIR "/M_MERC/M_STAB.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMSLICE,             ANIMSDIR "/M_MERC/M_SLICE.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMCSTAB,             ANIMSDIR "/M_MERC/M_C_STB.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMMEDIC,             ANIMSDIR "/M_MERC/M_MEDIC.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMDODGE,             ANIMSDIR "/M_MERC/M_DODGE.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMSTANDDWALAIM,      ANIMSDIR "/M_MERC/M_DBLSHT.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMRAISE,             ANIMSDIR "/M_MERC/M_RAISE.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMTHROW,             ANIMSDIR "/M_MERC/M_THROW.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMLOB,               ANIMSDIR "/M_MERC/M_LOB.STI",       S_STRUCT,  0,                        8, -1),
	M(BGMKICKDOOR,          ANIMSDIR "/M_MERC/M_DR_KCK.STI",    S_STRUCT,  0,                        4, -1),
	M(BGMRHIT,              ANIMSDIR "/M_MERC/M_R_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMSTANDAIM2,         ANIMSDIR "/M_MERC/M_SR_AM2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMFLEX,              ANIMSDIR "/M_MERC/M_FLEX.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMSTRECH,            ANIMSDIR "/M_MERC/M_STRTCH.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSHOEDUST,          ANIMSDIR "/M_MERC/M_SHOEDUST.STI",  S_STRUCT,  0,                        8, -1),
	M(BGMHEADTURN,          ANIMSDIR "/M_MERC/M_HEDTURN.STI",   S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_WALK,      ANIMSDIR "/M_MERC/MW_R_WAL.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_STD,       ANIMSDIR "/M_MERC/MW_R_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_WALK,      ANIMSDIR "/M_MERC/MW_N_WAL.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_STD,       ANIMSDIR "/M_MERC/MW_N_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_DIE,         ANIMSDIR "/M_MERC/MW_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_AIM,       ANIMSDIR "/M_MERC/MW_N_SHT.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_AIM,       ANIMSDIR "/M_MERC/MW_SR_AM.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_DBLSHT,      ANIMSDIR "/M_MERC/MW_DBL.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMWATER_TRANS,       ANIMSDIR "/M_MERC/MW_FALL.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_TRED,    ANIMSDIR "/M_MERC/MW_TRED.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_SWIM,    ANIMSDIR "/M_MERC/MW_SWIM.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_DIE,     ANIMSDIR "/M_MERC/MW_DIE2.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMHELIDROP,          ANIMSDIR "/M_MERC/M_HELIDRP.STI",   S_STRUCT,  0,                        1, -1),
	M(BGMSLEEP,             ANIMSDIR "/M_MERC/M_SLEEP.STI",     P_STRUCT,  0,                        8, -1),
	M(BGMSHOOT_LOW,         ANIMSDIR "/M_MERC/M_SHTLOW.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMTHREATENSTAND,     ANIMSDIR "/M_MERC/M_BRETH2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMCDBLSHOT,          ANIMSDIR "/M_MERC/MC_DBLSH.STI",    C_STRUCT,  0,                        8, -1),
	M(BGMHURTSTANDINGN,     ANIMSDIR "/M_MERC/M_I_BR_N.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTSTANDINGR,     ANIMSDIR "/M_MERC/M_I_BR_R.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTWALKINGN,      ANIMSDIR "/M_MERC/M_I_WK_N.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTWALKINGR,      ANIMSDIR "/M_MERC/M_I_WK_R.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTTRANS,         ANIMSDIR "/M_MERC/M_I_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMTHROWKNIFE,        ANIMSDIR "/M_MERC/M_K_THRO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMBREATHKNIFE,       ANIMSDIR "/M_MERC/M_KNF_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLBREATH,      ANIMSDIR "/M_MERC/M_P_BRTH.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMCOWER,             ANIMSDIR "/M_MERC/M_COWER.STI",     C_STRUCT,  0,                        8, -1),
	M(BGMRAISE2,            ANIMSDIR "/M_MERC/M_RAISE2.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMROCKET,            ANIMSDIR "/M_MERC/M_LAW.STI",       S_STRUCT,  0,                        8, -1),
	M(BGMMORTAR,            ANIMSDIR "/M_MERC/M_MORTAR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMSIDESTEP,          ANIMSDIR "/M_MERC/M_R_SDSP.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMDBLBREATH,         ANIMSDIR "/M_MERC/M_DBL_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPUNCHLOW,          ANIMSDIR "/M_MERC/M_PCH_LO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLSHOOTLOW,    ANIMSDIR "/M_MERC/M_P_SHLO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWATERTHROW,        ANIMSDIR "/M_MERC/MW_THROW.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMWALK2,             ANIMSDIR "/M_MERC/M_WALK2.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMRUN2,              ANIMSDIR "/M_MERC/M_R_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMIDLENECK,          ANIMSDIR "/M_MERC/M_NECK.STI",      S_STRUCT,  0,                        8, -1),
	M(BGMCROUCHTRANS,       ANIMSDIR "/M_MERC/M_C_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMRADIO,             ANIMSDIR "/M_MERC/M_RADIO.STI",     S_STRUCT,  0,                        8, -1),
	M(BGMCRRADIO,           ANIMSDIR "/M_MERC/M_C_RADO.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMDWPRONE,           ANIMSDIR "/M_MERC/M_DB_PRN.STI",    P_STRUCT,  0,                        8, -1),
	M(BGMDRUNK,             ANIMSDIR "/M_MERC/M_R_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLDRUNK,       ANIMSDIR "/M_MERC/M_N_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMCROWBAR,           ANIMSDIR "/M_MERC/M_CROBAR.STI",    S_STRUCT,  0,                        8, -1),
	M(BGMJUMPOVER,          ANIMSDIR "/M_MERC/M_N_RUN.STI",     NO_STRUCT, 0,                        8, -1),


	M(RGFWALKING,           ANIMSDIR "/F_MERC/F_R_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSTANDING,          ANIMSDIR "/F_MERC/F_BRETH2.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCROUCHING,         ANIMSDIR "/F_MERC/F_R_C.STI",       C_STRUCT,  0,                        8, -1),
	M(RGFSNEAKING,          ANIMSDIR "/F_MERC/F_R_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFRUNNING,           ANIMSDIR "/F_MERC/F_R_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFPRONE,             ANIMSDIR "/F_MERC/F_R_PRN.STI",     P_STRUCT,  0,                        8,  0),
	M(RGFSTANDAIM,          ANIMSDIR "/F_MERC/F_SR_AIM.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHITHARD,           ANIMSDIR "/F_MERC/F_DIEHD.STI",     FB_STRUCT, 0,                        8, -1),
	M(RGFHITSTAND,          ANIMSDIR "/F_MERC/F_DIEFWD.STI",    F_STRUCT,  0,                        8, -1),
	M(RGFHITHARDBLOOD,      ANIMSDIR "/F_MERC/F_DIEHDB.STI",    FB_STRUCT, 0,                        8, -1),
	M(RGFCROUCHAIM,         ANIMSDIR "/F_MERC/F_CR_AIM.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHITFALLBACK,       ANIMSDIR "/F_MERC/F_DIEBAC.STI",    F_STRUCT,  0,                        8, -1),
	M(RGFROLLOVER,          ANIMSDIR "/F_MERC/F_ROLL.STI",      P_STRUCT,  0,                        8, -1),
	M(RGFCLIMBROOF,         ANIMSDIR "/F_MERC/F_CLIMB.STI",     S_STRUCT,  0,                        4, -1),
	M(RGFFALL,              ANIMSDIR "/F_MERC/F_FALL.STI",      S_STRUCT,  0,                        4, -1),
	M(RGFFALLF,             ANIMSDIR "/F_MERC/F_FALLF.STI",     S_STRUCT,  0,                        4, -1),
	M(RGFHITCROUCH,         ANIMSDIR "/F_MERC/F_C_DIE.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFHITPRONE,          ANIMSDIR "/F_MERC/F_P_DIE.STI",     P_STRUCT,  0,                        8, -1),
	M(RGFHOPFENCE,          ANIMSDIR "/F_MERC/F_HOP.STI",       NO_STRUCT, 0,                        4, -1),
	M(RGFPUNCH,             ANIMSDIR "/F_MERC/F_PUNCH.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_STD,       ANIMSDIR "/F_MERC/N_BRETH2.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_WALK,      ANIMSDIR "/F_MERC/F_N_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_RUN,       ANIMSDIR "/F_MERC/F_N_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_SWAT,      ANIMSDIR "/F_MERC/F_N_SWAT.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_CROUCH,    ANIMSDIR "/F_MERC/F_N_CRCH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_S_SHOT,    ANIMSDIR "/F_MERC/F_N_SHOT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_C_SHOT,    ANIMSDIR "/F_MERC/F_N_C_AI.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_PRONE,     ANIMSDIR "/F_MERC/F_N_PRNE.STI",    P_STRUCT,  0,                        8, -1),
	M(RGFDIE_JFK,           ANIMSDIR "/F_MERC/F_DIEJFK.STI",    F_STRUCT,  0,                        8, -1),
	M(RGFOPEN,              ANIMSDIR "/F_MERC/F_OPEN.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFPICKUP,            ANIMSDIR "/F_MERC/F_PICKUP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSTAB,              ANIMSDIR "/F_MERC/F_STAB.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFSLICE,             ANIMSDIR "/F_MERC/F_SLICE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFCSTAB,             ANIMSDIR "/F_MERC/F_C_STB.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFMEDIC,             ANIMSDIR "/F_MERC/F_MEDIC.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFDODGE,             ANIMSDIR "/F_MERC/F_DODGE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFSTANDDWALAIM,      ANIMSDIR "/F_MERC/F_DBLSHT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFRAISE,             ANIMSDIR "/F_MERC/F_RAISE.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFTHROW,             ANIMSDIR "/F_MERC/F_THROW.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFLOB,               ANIMSDIR "/F_MERC/F_LOB.STI",       S_STRUCT,  0,                        8, -1),
	M(RGFKICKDOOR,          ANIMSDIR "/F_MERC/F_DR_KCK.STI",    S_STRUCT,  0,                        4, -1),
	M(RGFRHIT,              ANIMSDIR "/F_MERC/F_R_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFCLEAN,             ANIMSDIR "/F_MERC/A_CLEAN.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFKICKSN,            ANIMSDIR "/F_MERC/A_KICKSN.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFALOOK,             ANIMSDIR "/F_MERC/A_LOOK.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWIPE,              ANIMSDIR "/F_MERC/A_WIPE.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_WALK,      ANIMSDIR "/F_MERC/FW_R_WLK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_STD,       ANIMSDIR "/F_MERC/FW_R_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_WALK,      ANIMSDIR "/F_MERC/FW_N_WLK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_STD,       ANIMSDIR "/F_MERC/FW_N_STD.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_DIE,         ANIMSDIR "/F_MERC/FW_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_AIM,       ANIMSDIR "/F_MERC/FW_N_SHT.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_AIM,       ANIMSDIR "/F_MERC/FW_SR_AI.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_DBLSHT,      ANIMSDIR "/F_MERC/FW_DBL.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_TRANS,       ANIMSDIR "/F_MERC/FW_FALL.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_TRED,    ANIMSDIR "/F_MERC/FW_TRED.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_SWIM,    ANIMSDIR "/F_MERC/FW_SWIM.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_DIE,     ANIMSDIR "/F_MERC/FW_DIE2.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFHELIDROP,          ANIMSDIR "/F_MERC/F_HELIDRP.STI",   S_STRUCT,  0,                        1, -1),
	M(RGFSLEEP,             ANIMSDIR "/F_MERC/F_SLEEP.STI",     P_STRUCT,  0,                        8, -1),
	M(RGFSHOOT_LOW,         ANIMSDIR "/F_MERC/F_SHTLOW.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCDBLSHOT,          ANIMSDIR "/F_MERC/FC_DBLSH.STI",    C_STRUCT,  0,                        8, -1),
	M(RGFHURTSTANDINGN,     ANIMSDIR "/F_MERC/F_I_BR_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTSTANDINGR,     ANIMSDIR "/F_MERC/F_I_BR_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTWALKINGN,      ANIMSDIR "/F_MERC/F_I_WK_N.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTWALKINGR,      ANIMSDIR "/F_MERC/F_I_WK_R.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTTRANS,         ANIMSDIR "/F_MERC/F_I_TRAN.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFTHROWKNIFE,        ANIMSDIR "/F_MERC/F_K_THRO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFBREATHKNIFE,       ANIMSDIR "/F_MERC/F_KNF_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLBREATH,      ANIMSDIR "/F_MERC/F_P_BRTH.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCOWER,             ANIMSDIR "/F_MERC/F_COWER.STI",     C_STRUCT,  0,                        8, -1),
	M(RGFROCKET,            ANIMSDIR "/F_MERC/F_LAW.STI",       S_STRUCT,  0,                        8, -1),
	M(RGFMORTAR,            ANIMSDIR "/F_MERC/F_MORTAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSIDESTEP,          ANIMSDIR "/F_MERC/F_R_SDSP.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFDBLBREATH,         ANIMSDIR "/F_MERC/F_DBL_BR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPUNCHLOW,          ANIMSDIR "/F_MERC/F_PCH_LO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLSHOOTLOW,    ANIMSDIR "/F_MERC/F_P_SHLO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFWATERTHROW,        ANIMSDIR "/F_MERC/FW_THRW.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFRADIO,             ANIMSDIR "/F_MERC/F_RADIO.STI",     S_STRUCT,  0,                        8, -1),
	M(RGFCRRADIO,           ANIMSDIR "/F_MERC/F_C_RADO.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFSLAP,              ANIMSDIR "/F_MERC/F_SLAP.STI",      S_STRUCT,  0,                        8, -1),
	M(RGFDWPRONE,           ANIMSDIR "/F_MERC/F_DB_PRN.STI",    P_STRUCT,  0,                        8, -1),
	M(RGFDRUNK,             ANIMSDIR "/F_MERC/F_R_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLDRUNK,       ANIMSDIR "/F_MERC/F_N_DRNK.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFCROWBAR,           ANIMSDIR "/F_MERC/F_CROBAR.STI",    S_STRUCT,  0,                        8, -1),
	M(RGFJUMPOVER,          ANIMSDIR "/F_MERC/F_N_RUN.STI",     NO_STRUCT, 0,                        8, -1),


	M(AFMONSTERSTANDING,    ANIMSDIR "/MONSTERS/MN_BREAT.STI",  S_STRUCT,  0,                        8, -1),
	M(AFMONSTERWALKING,     ANIMSDIR "/MONSTERS/MN_WALK.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMONSTERATTACK,      ANIMSDIR "/MONSTERS/MN_ATTAK.STI",  S_STRUCT,  0,                        8, -1),
	M(AFMONSTERCLOSEATTACK, ANIMSDIR "/MONSTERS/M_ATTK2.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMONSTERSPITATTACK,  ANIMSDIR "/MONSTERS/M_SPIT.STI",    S_STRUCT,  0,                        8, -1),
	M(AFMONSTEREATING,      ANIMSDIR "/MONSTERS/M_EAT.STI",     S_STRUCT,  0,                        8, -1),
	M(AFMONSTERDIE,         ANIMSDIR "/MONSTERS/MN_DIE1.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMUP,                ANIMSDIR "/MONSTERS/MN_UP.STI",     S_STRUCT,  0,                        8, -1),
	M(AFMJUMP,              ANIMSDIR "/MONSTERS/MN_JUMP.STI",   S_STRUCT,  0,                        8, -1),
	M(AFMMELT,              ANIMSDIR "/MONSTERS/MN_MELT.STI",   S_STRUCT,  0,                        8, -1),


	M(LVBREATH,             ANIMSDIR "/MONSTERS/L_BREATH.STI",  S_STRUCT,  0,                        8, -1),
	M(LVDIE,                ANIMSDIR "/MONSTERS/L_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(LVWALK,               ANIMSDIR "/MONSTERS/L_WALK.STI",    S_STRUCT,  0,                        8, -1),

	M(IBREATH,              ANIMSDIR "/MONSTERS/I_BREATH.STI",  S_STRUCT,  0,                        8, -1),
	M(IWALK,                ANIMSDIR "/MONSTERS/I_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(IDIE,                 ANIMSDIR "/MONSTERS/I_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(IEAT,                 ANIMSDIR "/MONSTERS/I_EAT.STI",     S_STRUCT,  0,                        8, -1),
	M(IATTACK,              ANIMSDIR "/MONSTERS/I_ATTACK.STI",  S_STRUCT,  0,                        8, -1),

	M(QUEENMONSTERSTANDING, ANIMSDIR "/MONSTERS/QMN_BREAT.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERREADY,    ANIMSDIR "/MONSTERS/Q_READY.STI",   S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_SW,  ANIMSDIR "/MONSTERS/Q_SPIT_SW.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_E,   ANIMSDIR "/MONSTERS/Q_SPIT_E.STI",  S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_NE,  ANIMSDIR "/MONSTERS/Q_SPIT_NE.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_S,   ANIMSDIR "/MONSTERS/Q_SPIT_S.STI",  S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_SE,  ANIMSDIR "/MONSTERS/Q_SPIT_SE.STI", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERDEATH,    ANIMSDIR "/MONSTERS/Q_DIE.STI",     S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSWIPE,    ANIMSDIR "/MONSTERS/Q_SWIPE.STI",   S_STRUCT,  0,                        1, -1),

	M(FATMANSTANDING,       ANIMSDIR "/CIVS/FT_BRTH.STI",       S_STRUCT,  0,                        8, -1),
	M(FATMANWALKING,        ANIMSDIR "/CIVS/FT_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(FATMANRUNNING,        ANIMSDIR "/CIVS/FT_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(FATMANDIE,            ANIMSDIR "/CIVS/FT_DIE.STI",        F_STRUCT,  0,                        8, -1),
	M(FATMANASS,            ANIMSDIR "/CIVS/FT_SCRTC.STI",      S_STRUCT,  0,                        8, -1),
	M(FATMANACT,            ANIMSDIR "/CIVS/FT_ACT.STI",        S_STRUCT,  0,                        8, -1),
	M(FATMANCOWER,          ANIMSDIR "/CIVS/FT_COWER.STI",      C_STRUCT,  0,                        8, -1),
	M(FATMANDIE2,           ANIMSDIR "/CIVS/FT_DIE2.STI",       F_STRUCT,  0,                        8, -1),
	M(FATMANCOWERHIT,       ANIMSDIR "/CIVS/F_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(MANCIVSTANDING,       ANIMSDIR "/CIVS/M_BREATH.STI",      S_STRUCT,  0,                        8, -1),
	M(MANCIVWALKING,        ANIMSDIR "/CIVS/M_WALK.STI",        S_STRUCT,  0,                        8, -1),
	M(MANCIVRUNNING,        ANIMSDIR "/CIVS/M_RUN.STI",         S_STRUCT,  0,                        8, -1),
	M(MANCIVDIE,            ANIMSDIR "/CIVS/M_DIE.STI",         F_STRUCT,  0,                        8, -1),
	M(MANCIVACT,            ANIMSDIR "/CIVS/M_ACT.STI",         S_STRUCT,  0,                        8, -1),
	M(MANCIVCOWER,          ANIMSDIR "/CIVS/M_COWER.STI",       C_STRUCT,  0,                        8, -1),
	M(MANCIVDIE2,           ANIMSDIR "/CIVS/M_DIE2.STI",        F_STRUCT,  0,                        8, -1),
	M(MANCIVSMACKED,        ANIMSDIR "/CIVS/M_SMCKED.STI",      S_STRUCT,  0,                        8, -1),
	M(MANCIVPUNCH,          ANIMSDIR "/CIVS/M_PUNCH.STI",       S_STRUCT,  0,                        8, -1),
	M(MANCIVCOWERHIT,       ANIMSDIR "/CIVS/M_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(MINICIVSTANDING,      ANIMSDIR "/CIVS/MI_BREATH.STI",     S_STRUCT,  0,                        8, -1),
	M(MINICIVWALKING,       ANIMSDIR "/CIVS/MI_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(MINICIVRUNNING,       ANIMSDIR "/CIVS/MI_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(MINICIVDIE,           ANIMSDIR "/CIVS/MI_DIE.STI",        F_STRUCT,  0,                        8, -1),
	M(MINISTOCKING,         ANIMSDIR "/CIVS/MI_STKNG.STI",      S_STRUCT,  0,                        8, -1),
	M(MINIACT,              ANIMSDIR "/CIVS/MI_ACT.STI",        S_STRUCT,  0,                        8, -1),
	M(MINICOWER,            ANIMSDIR "/CIVS/MI_COWER.STI",      C_STRUCT,  0,                        8, -1),
	M(MINIDIE2,             ANIMSDIR "/CIVS/MI_DIE2.STI",       F_STRUCT,  0,                        8, -1),
	M(MINICOWERHIT,         ANIMSDIR "/CIVS/S_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(DRESSCIVSTANDING,     ANIMSDIR "/CIVS/DS_BREATH.STI",     S_STRUCT,  0,                        8, -1),
	M(DRESSCIVWALKING,      ANIMSDIR "/CIVS/DS_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(DRESSCIVRUNNING,      ANIMSDIR "/CIVS/DS_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(DRESSCIVDIE,          ANIMSDIR "/CIVS/DS_DIE.STI",        F_STRUCT,  0,                        8, -1),
	M(DRESSCIVACT,          ANIMSDIR "/CIVS/DS_ACT.STI",        S_STRUCT,  0,                        8, -1),
	M(DRESSCIVCOWER,        ANIMSDIR "/CIVS/DS_COWER.STI",      C_STRUCT,  0,                        8, -1),
	M(DRESSCIVDIE2,         ANIMSDIR "/CIVS/DS_DIE2.STI",       F_STRUCT,  0,                        8, -1),
	M(DRESSCIVCOWERHIT,     ANIMSDIR "/CIVS/W_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),

	M(HATKIDCIVSTANDING,    ANIMSDIR "/CIVS/H_BREATH.STI",      S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVWALKING,     ANIMSDIR "/CIVS/H_WALK.STI",        S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVRUNNING,     ANIMSDIR "/CIVS/H_RUN.STI",         S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVDIE,         ANIMSDIR "/CIVS/H_DIE2.STI",        F_STRUCT,  0,                        8, -1),
	M(HATKIDCIVJFK,         ANIMSDIR "/CIVS/H_DIEJFK.STI",      S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVYOYO,        ANIMSDIR "/CIVS/H_YOYO.STI",        S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVACT,         ANIMSDIR "/CIVS/H_ACT.STI",         S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVCOWER,       ANIMSDIR "/CIVS/H_COWER.STI",       S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVDIE2,        ANIMSDIR "/CIVS/H_DIE.STI",         F_STRUCT,  0,                        8, -1),
	M(HATKIDCIVCOWERHIT,    ANIMSDIR "/CIVS/H_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),
	M(HATKIDCIVSKIP,        ANIMSDIR "/CIVS/H_SKIP.STI",        C_STRUCT,  0,                        8, -1),

	M(KIDCIVSTANDING,       ANIMSDIR "/CIVS/K_BREATH.STI",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVWALKING,        ANIMSDIR "/CIVS/K_WALK.STI",        S_STRUCT,  0,                        8, -1),
	M(KIDCIVRUNNING,        ANIMSDIR "/CIVS/K_RUN.STI",         S_STRUCT,  0,                        8, -1),
	M(KIDCIVDIE,            ANIMSDIR "/CIVS/K_DIE2.STI",        F_STRUCT,  0,                        8, -1),
	M(KIDCIVJFK,            ANIMSDIR "/CIVS/K_DIEJFK.STI",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVARMPIT,         ANIMSDIR "/CIVS/K_ARMPIT.STI",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVACT,            ANIMSDIR "/CIVS/K_ACT.STI",         S_STRUCT,  0,                        8, -1),
	M(KIDCIVCOWER,          ANIMSDIR "/CIVS/K_COWER.STI",       S_STRUCT,  0,                        8, -1),
	M(KIDCIVDIE2,           ANIMSDIR "/CIVS/K_DIE.STI",         F_STRUCT,  0,                        8, -1),
	M(KIDCIVCOWERHIT,       ANIMSDIR "/CIVS/K_CW_HIT.STI",      C_STRUCT,  0,                        8, -1),
	M(KIDCIVSKIP,           ANIMSDIR "/CIVS/K_SKIP.STI",        C_STRUCT,  0,                        8, -1),

	M(CRIPCIVSTANDING,      ANIMSDIR "/CIVS/CP_BRETH.STI",      S_STRUCT,  0,                        8, -1),
	M(CRIPCIVWALKING,       ANIMSDIR "/CIVS/CP_WALK.STI",       S_STRUCT,  0,                        8, -1),
	M(CRIPCIVRUNNING,       ANIMSDIR "/CIVS/CP_RUN.STI",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVBEG,           ANIMSDIR "/CIVS/CP_BEG.STI",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVDIE,           ANIMSDIR "/CIVS/CP_DIE.STI",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVDIE2,          ANIMSDIR "/CIVS/CP_DIE2.STI",       S_STRUCT,  0,                        8, -1),
	M(CRIPCIVKICK,          ANIMSDIR "/CIVS/CP_KICK.STI",       S_STRUCT,  0,                        8, -1),

	M(COWSTANDING,          ANIMSDIR "/ANIMALS/C_BREATH.STI",   S_STRUCT,  0,                        8, -1),
	M(COWWALKING,           ANIMSDIR "/ANIMALS/C_WALK.STI",     S_STRUCT,  0,                        8, -1),
	M(COWDIE,               ANIMSDIR "/ANIMALS/C_DIE.STI",      S_STRUCT,  0,                        8, -1),
	M(COWEAT,               ANIMSDIR "/ANIMALS/C_EAT.STI",      S_STRUCT,  0,                        8, -1),

	M(CROWWALKING,          ANIMSDIR "/ANIMALS/CR_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(CROWFLYING,           ANIMSDIR "/ANIMALS/CR_FLY.STI",     S_STRUCT,  0,                        8, -1),
	M(CROWEATING,           ANIMSDIR "/ANIMALS/CR_EAT.STI",     S_STRUCT,  0,                        8, -1),
	M(CROWDYING,            ANIMSDIR "/ANIMALS/CR_DIE.STI",     NO_STRUCT, 0,                        8, -1),

	M(CATBREATH,            ANIMSDIR "/ANIMALS/CT_BREATH.STI",  S_STRUCT,  0,                        8, -1),
	M(CATWALK,              ANIMSDIR "/ANIMALS/CT_WALK.STI",    S_STRUCT,  0,                        8, -1),
	M(CATRUN,               ANIMSDIR "/ANIMALS/CT_RUN.STI",     S_STRUCT,  0,                        8, -1),
	M(CATREADY,             ANIMSDIR "/ANIMALS/CT_READY.STI",   S_STRUCT,  0,                        8, -1),
	M(CATHIT,               ANIMSDIR "/ANIMALS/CT_HIT.STI",     S_STRUCT,  0,                        8, -1),
	M(CATDIE,               ANIMSDIR "/ANIMALS/CT_DIE.STI",     S_STRUCT,  0,                        8, -1),
	M(CATSWIPE,             ANIMSDIR "/ANIMALS/CT_SWIPE.STI",   S_STRUCT,  0,                        8, -1),
	M(CATBITE,              ANIMSDIR "/ANIMALS/CT_BITE.STI",    S_STRUCT,  0,                        8, -1),

	M(ROBOTNWBREATH,        ANIMSDIR "/CIVS/J_R_BRET.STI",      S_STRUCT,  0,                        8, -1),
	M(ROBOTNWWALK,          ANIMSDIR "/CIVS/J_R_WALK.STI",      S_STRUCT,  0,                        8, -1),
	M(ROBOTNWHIT,           ANIMSDIR "/CIVS/J_R_HIT.STI"  ,     S_STRUCT,  0,                        8, -1),
	M(ROBOTNWDIE,           ANIMSDIR "/CIVS/J_R_DIE.STI",       S_STRUCT,  0,                        8, -1),
	M(ROBOTNWSHOOT,         ANIMSDIR "/CIVS/J_R_SHOT.STI",      S_STRUCT,  0,                        8, -1),

	M(HUMVEE_BASIC,         ANIMSDIR "/VEHICLES/HUMMER2.STI",   S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(HUMVEE_DIE,           ANIMSDIR "/VEHICLES/HM_WREK.STI",   S_STRUCT,  0,                        2, -1),

	M(TANKNW_READY,         ANIMSDIR "/VEHICLES/TANK_ROT.STI",  S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(TANKNW_SHOOT,         ANIMSDIR "/VEHICLES/TANK_SHT.STI",  S_STRUCT,  0,                        8, -1),
	M(TANKNW_DIE,           ANIMSDIR "/VEHICLES/TK_WREK.STI",   S_STRUCT,  0,                        1, -1),

	M(TANKNE_READY,         ANIMSDIR "/VEHICLES/TNK2_ROT.STI",  S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(TANKNE_SHOOT,         ANIMSDIR "/VEHICLES/TNK2_SHT.STI",  S_STRUCT,  0,                        8, -1),
	M(TANKNE_DIE,           ANIMSDIR "/VEHICLES/TK2_WREK.STI",  S_STRUCT,  0,                        1, -1),

	M(ELDORADO_BASIC,       ANIMSDIR "/VEHICLES/HUMMER.STI",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(ELDORADO_DIE,         ANIMSDIR "/VEHICLES/HM_WREK.STI",   NO_STRUCT, 0,                        2, -1),

	M(ICECREAMTRUCK_BASIC,  ANIMSDIR "/VEHICLES/ICECRM.STI",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(ICECREAMTRUCK_DIE,    ANIMSDIR "/VEHICLES/HM_WREK.STI",   NO_STRUCT, 0,                        2, -1),

	M(JEEP_BASIC,           ANIMSDIR "/VEHICLES/HUMMER.STI",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(JEEP_DIE,             ANIMSDIR "/VEHICLES/HM_WREK.STI",   NO_STRUCT, 0,                        2, -1),

	M(BODYEXPLODE,          ANIMSDIR "/S_MERC/BOD_BLOW.STI",    NO_STRUCT, 0,                        1, -1),
};

#undef M


struct AnimationStructureType
{
	const char* const   Filename;
	STRUCTURE_FILE_REF* pStructureFileRef;
};


#define PATH_STRUCT ANIMSDIR "/STRUCTDATA/"
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
