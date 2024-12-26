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
#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>

#define EMPTY_SLOT					-1
#define TO_INIT					0

#define ANIMPROFILEFILENAME				BINARYDATADIR "/ja2prof.dat"

#define PATH_STRUCT					ANIMSDIR "/STRUCTDATA/"
#define SUFFIX						".JSD"



ANIM_PROF *gpAnimProfiles = NULL;
static UINT8 gubNumAnimProfiles = 0;

INT8 gbAnimUsageHistory[ NUMANIMATIONSURFACETYPES ][ MAX_NUM_SOLDIERS ];


#define M(name, file, type, flags, dir, profile)	{ name, file, type, flags, dir, TO_INIT, NULL, 0, profile }

AnimationSurfaceType gAnimSurfaceDatabase[NUMANIMATIONSURFACETYPES] =
{
	M(RGMBASICWALKING,      ANIMSDIR "/s_merc/s_r_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMSTANDING,          ANIMSDIR "/s_merc/s_r_std.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMCROUCHING,         ANIMSDIR "/s_merc/s_r_c.sti",       C_STRUCT,  0,                        8, -1),
	M(RGMSNEAKING,          ANIMSDIR "/s_merc/s_r_swat.sti",    C_STRUCT,  0,                        8, -1),
	M(RGMRUNNING,           ANIMSDIR "/s_merc/s_r_run.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMPRONE,             ANIMSDIR "/s_merc/s_r_prn.sti",     P_STRUCT,  0,                        8,  0),
	M(RGMSTANDAIM,          ANIMSDIR "/s_merc/s_sr_aim.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMHITHARD,           ANIMSDIR "/s_merc/s_diehd.sti",     FB_STRUCT, 0,                        8, -1),
	M(RGMHITSTAND,          ANIMSDIR "/s_merc/s_diefwd.sti",    F_STRUCT,  0,                        8, -1),
	M(RGMHITHARDBLOOD,      ANIMSDIR "/s_merc/s_diehdb.sti",    FB_STRUCT, 0,                        8, -1),
	M(RGMCROUCHAIM,         ANIMSDIR "/s_merc/s_cr_aim.sti",    C_STRUCT,  0,                        8, -1),
	M(RGMHITFALLBACK,       ANIMSDIR "/s_merc/s_diebac.sti",    FB_STRUCT, 0,                        8, -1),
	M(RGMROLLOVER,          ANIMSDIR "/s_merc/s_roll.sti",      P_STRUCT,  0,                        8, -1),
	M(RGMCLIMBROOF,         ANIMSDIR "/s_merc/s_climb.sti",     S_STRUCT,  0,                        4, -1),
	M(RGMFALL,              ANIMSDIR "/s_merc/s_fall.sti",      S_STRUCT,  0,                        4, -1),
	M(RGMFALLF,             ANIMSDIR "/s_merc/s_fallf.sti",     S_STRUCT,  0,                        4, -1),
	M(RGMHITCROUCH,         ANIMSDIR "/s_merc/s_c_die.sti",     C_STRUCT,  0,                        8, -1),
	M(RGMHITPRONE,          ANIMSDIR "/s_merc/s_p_die.sti",     P_STRUCT,  0,                        8, -1),
	M(RGMHOPFENCE,          ANIMSDIR "/s_merc/s_hop.sti",       NO_STRUCT, 0,                        4, -1),
	M(RGMPUNCH,             ANIMSDIR "/s_merc/s_punch.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_STD,       ANIMSDIR "/s_merc/s_n_std.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_WALK,      ANIMSDIR "/s_merc/s_n_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_RUN,       ANIMSDIR "/s_merc/s_n_run.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_SWAT,      ANIMSDIR "/s_merc/s_n_swat.sti",    C_STRUCT,  0,                        8, -1),
	M(RGMNOTHING_CROUCH,    ANIMSDIR "/s_merc/s_n_crch.sti",    C_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_S_SHOT,    ANIMSDIR "/s_merc/s_n_shot.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_C_SHOT,    ANIMSDIR "/s_merc/s_n_c_ai.sti",    C_STRUCT,  0,                        8, -1),
	M(RGMHANDGUN_PRONE,     ANIMSDIR "/s_merc/s_n_prne.sti",    P_STRUCT,  0,                        8, -1),
	M(RGMDIE_JFK,           ANIMSDIR "/s_merc/s_diejfk.sti",    F_STRUCT,  0,                        8, -1),
	M(RGMOPEN,              ANIMSDIR "/s_merc/s_open.sti",      S_STRUCT,  0,                        8, -1),
	M(RGMPICKUP,            ANIMSDIR "/s_merc/s_pickup.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMSTAB,              ANIMSDIR "/s_merc/s_stab.sti",      S_STRUCT,  0,                        8, -1),
	M(RGMSLICE,             ANIMSDIR "/s_merc/s_slice.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMCSTAB,             ANIMSDIR "/s_merc/s_c_stb.sti",     C_STRUCT,  0,                        8, -1),
	M(RGMMEDIC,             ANIMSDIR "/s_merc/s_medic.sti",     C_STRUCT,  0,                        8, -1),
	M(RGMDODGE,             ANIMSDIR "/s_merc/s_dodge.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMSTANDDWALAIM,      ANIMSDIR "/s_merc/s_dblsht.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMRAISE,             ANIMSDIR "/s_merc/s_raise.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMTHROW,             ANIMSDIR "/s_merc/s_lob.sti",       S_STRUCT,  0,                        8, -1),
	M(RGMLOB,               ANIMSDIR "/s_merc/s_throw.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMKICKDOOR,          ANIMSDIR "/s_merc/s_dr_kck.sti",    S_STRUCT,  0,                        4, -1),
	M(RGMRHIT,              ANIMSDIR "/s_merc/s_r_hit.sti",     S_STRUCT,  0,                        8, -1),
	M(RGM_SQUISH,           ANIMSDIR "/s_merc/a_squish.sti",    S_STRUCT,  0,                        8, -1),
	M(RGM_LOOK,             ANIMSDIR "/s_merc/a_look.sti",      S_STRUCT,  0,                        8, -1),
	M(RGM_PULL,             ANIMSDIR "/s_merc/a_pull.sti",      S_STRUCT,  0,                        8, -1),
	M(RGM_SPIT,             ANIMSDIR "/s_merc/a_spit.sti",      S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_WALK,      ANIMSDIR "/s_merc/sw_r_walk.sti",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_STD,       ANIMSDIR "/s_merc/sw_r_std.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_WALK,      ANIMSDIR "/s_merc/sw_n_walk.sti",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_STD,       ANIMSDIR "/s_merc/sw_n_std.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMWATER_DIE,         ANIMSDIR "/s_merc/sw_die.sti",      S_STRUCT,  0,                        8, -1),
	M(RGMWATER_N_AIM,       ANIMSDIR "/s_merc/sw_n_shot.sti",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_R_AIM,       ANIMSDIR "/s_merc/sw_sr_aim.sti",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_DBLSHT,      ANIMSDIR "/s_merc/sw_dblsht.sti",   S_STRUCT,  0,                        8, -1),
	M(RGMWATER_TRANS,       ANIMSDIR "/s_merc/sw_fall.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_TRED,    ANIMSDIR "/s_merc/s_tred.sti",      S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_SWIM,    ANIMSDIR "/s_merc/s_swim.sti",      S_STRUCT,  0,                        8, -1),
	M(RGMDEEPWATER_DIE,     ANIMSDIR "/s_merc/s_d_die.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMMCLIMB,            ANIMSDIR "/s_merc/s_mclimb.sti",    S_STRUCT,  0,                        3, -1),
	M(RGMHELIDROP,          ANIMSDIR "/s_merc/s_helidrp.sti",   S_STRUCT,  0,                        1, -1),
	M(RGMLOWKICK,           ANIMSDIR "/s_merc/k_lw_kick.sti",   S_STRUCT,  0,                        8, -1),
	M(RGMPUNCH,             ANIMSDIR "/s_merc/k_punch.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMSPINKICK,          ANIMSDIR "/s_merc/s_spnkck.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMSLEEP,             ANIMSDIR "/s_merc/s_sleep.sti",     P_STRUCT,  0,                        8, -1),
	M(RGMSHOOT_LOW,         ANIMSDIR "/s_merc/s_shtlo.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMCDBLSHOT,          ANIMSDIR "/s_merc/sc_dblsh.sti",    C_STRUCT,  0,                        8, -1),
	M(RGMHURTSTANDINGN,     ANIMSDIR "/s_merc/s_i_br_n.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTSTANDINGR,     ANIMSDIR "/s_merc/s_i_br_r.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTWALKINGN,      ANIMSDIR "/s_merc/s_i_wk_n.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTWALKINGR,      ANIMSDIR "/s_merc/s_i_wk_r.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMHURTTRANS,         ANIMSDIR "/s_merc/s_i_tran.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMTHROWKNIFE,        ANIMSDIR "/s_merc/s_k_thro.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMBREATHKNIFE,       ANIMSDIR "/s_merc/s_knf_br.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLBREATH,      ANIMSDIR "/s_merc/s_p_brth.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMCOWER,             ANIMSDIR "/s_merc/s_cower.sti",     C_STRUCT,  0,                        8, -1),
	M(RGMROCKET,            ANIMSDIR "/s_merc/s_law.sti",       S_STRUCT,  0,                        8, -1),
	M(RGMMORTAR,            ANIMSDIR "/s_merc/s_mortar.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMSIDESTEP,          ANIMSDIR "/s_merc/s_r_sdsp.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMDBLBREATH,         ANIMSDIR "/s_merc/s_dbl_br.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMPUNCHLOW,          ANIMSDIR "/s_merc/s_pch_lo.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLSHOOTLOW,    ANIMSDIR "/s_merc/s_p_shlo.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMWATERTHROW,        ANIMSDIR "/s_merc/sw_lob.sti",      S_STRUCT,  0,                        8, -1),
	M(RGMRADIO,             ANIMSDIR "/s_merc/s_radio.sti",     S_STRUCT,  0,                        8, -1),
	M(RGMCRRADIO,           ANIMSDIR "/s_merc/s_c_rado.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMBURN,              ANIMSDIR "/s_merc/s_fire.sti",      S_STRUCT,  0,                        1, -1),
	M(RGMDWPRONE,           ANIMSDIR "/s_merc/s_db_prn.sti",    P_STRUCT,  0,                        8, -1),
	M(RGMDRUNK,             ANIMSDIR "/s_merc/s_r_drnk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMPISTOLDRUNK,       ANIMSDIR "/s_merc/s_n_drnk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMCROWBAR,           ANIMSDIR "/s_merc/s_crobar.sti",    S_STRUCT,  0,                        8, -1),
	M(RGMJUMPOVER,          ANIMSDIR "/s_merc/s_n_run.sti",     NO_STRUCT, 0,                        8, -1),


	M(BGMWALKING,           ANIMSDIR "/m_merc/m_r_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMSTANDING,          ANIMSDIR "/m_merc/m_r_std.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMCROUCHING,         ANIMSDIR "/m_merc/m_r_c.sti",       C_STRUCT,  0,                        8, -1),
	M(BGMSNEAKING,          ANIMSDIR "/m_merc/m_r_swat.sti",    C_STRUCT,  0,                        8, -1),
	M(BGMRUNNING,           ANIMSDIR "/m_merc/m_r_run2.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMPRONE,             ANIMSDIR "/m_merc/m_r_prn.sti",     P_STRUCT,  0,                        8,  0),
	M(BGMSTANDAIM,          ANIMSDIR "/m_merc/m_sr_aim.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMHITHARD,           ANIMSDIR "/m_merc/m_diehd.sti",     FB_STRUCT, 0,                        8, -1),
	M(BGMHITSTAND,          ANIMSDIR "/m_merc/m_diefwd.sti",    F_STRUCT,  0,                        8, -1),
	M(BGMHITHARDBLOOD,      ANIMSDIR "/m_merc/m_diehdb.sti",    FB_STRUCT, 0,                        8, -1),
	M(RGMCROUCHAIM,         ANIMSDIR "/m_merc/m_cr_aim.sti",    C_STRUCT,  0,                        8, -1),
	M(BGMHITFALLBACK,       ANIMSDIR "/m_merc/m_diebac.sti",    FB_STRUCT, 0,                        8, -1),
	M(BGMROLLOVER,          ANIMSDIR "/m_merc/m_roll.sti",      P_STRUCT,  0,                        8, -1),
	M(BGMCLIMBROOF,         ANIMSDIR "/m_merc/m_climb.sti",     S_STRUCT,  0,                        4, -1),
	M(BGMFALL,              ANIMSDIR "/m_merc/m_fall.sti",      S_STRUCT,  0,                        4, -1),
	M(BGMFALLF,             ANIMSDIR "/m_merc/m_fallf.sti",     S_STRUCT,  0,                        4, -1),
	M(BGMHITCROUCH,         ANIMSDIR "/m_merc/m_c_die.sti",     C_STRUCT,  0,                        8, -1),
	M(BGMHITPRONE,          ANIMSDIR "/m_merc/m_p_die.sti",     P_STRUCT,  0,                        8, -1),
	M(BGMHOPFENCE,          ANIMSDIR "/m_merc/m_hop.sti",       NO_STRUCT, 0,                        4, -1),
	M(BGMPUNCH,             ANIMSDIR "/m_merc/m_punch.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_STD,       ANIMSDIR "/m_merc/m_n_std.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_WALK,      ANIMSDIR "/m_merc/m_n_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_RUN,       ANIMSDIR "/m_merc/m_n_run.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_SWAT,      ANIMSDIR "/m_merc/m_n_swat.sti",    C_STRUCT,  0,                        8, -1),
	M(BGMNOTHING_CROUCH,    ANIMSDIR "/m_merc/m_n_crch.sti",    C_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_S_SHOT,    ANIMSDIR "/m_merc/m_n_shot.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_C_SHOT,    ANIMSDIR "/m_merc/m_n_c_ai.sti",    C_STRUCT,  0,                        8, -1),
	M(BGMHANDGUN_PRONE,     ANIMSDIR "/m_merc/m_n_prne.sti",    P_STRUCT,  0,                        8, -1),
	M(BGMDIE_JFK,           ANIMSDIR "/m_merc/m_diejfk.sti",    F_STRUCT,  0,                        8, -1),
	M(BGMOPEN,              ANIMSDIR "/m_merc/m_open.sti",      S_STRUCT,  0,                        8, -1),
	M(BGMPICKUP,            ANIMSDIR "/m_merc/m_pickup.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMSTAB,              ANIMSDIR "/m_merc/m_stab.sti",      S_STRUCT,  0,                        8, -1),
	M(BGMSLICE,             ANIMSDIR "/m_merc/m_slice.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMCSTAB,             ANIMSDIR "/m_merc/m_c_stb.sti",     C_STRUCT,  0,                        8, -1),
	M(BGMMEDIC,             ANIMSDIR "/m_merc/m_medic.sti",     C_STRUCT,  0,                        8, -1),
	M(BGMDODGE,             ANIMSDIR "/m_merc/m_dodge.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMSTANDDWALAIM,      ANIMSDIR "/m_merc/m_dblsht.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMRAISE,             ANIMSDIR "/m_merc/m_raise.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMTHROW,             ANIMSDIR "/m_merc/m_throw.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMLOB,               ANIMSDIR "/m_merc/m_lob.sti",       S_STRUCT,  0,                        8, -1),
	M(BGMKICKDOOR,          ANIMSDIR "/m_merc/m_dr_kck.sti",    S_STRUCT,  0,                        4, -1),
	M(BGMRHIT,              ANIMSDIR "/m_merc/m_r_hit.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMSTANDAIM2,         ANIMSDIR "/m_merc/m_sr_am2.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMFLEX,              ANIMSDIR "/m_merc/m_flex.sti",      S_STRUCT,  0,                        8, -1),
	M(BGMSTRECH,            ANIMSDIR "/m_merc/m_strtch.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMSHOEDUST,          ANIMSDIR "/m_merc/m_shoedust.sti",  S_STRUCT,  0,                        8, -1),
	M(BGMHEADTURN,          ANIMSDIR "/m_merc/m_hedturn.sti",   S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_WALK,      ANIMSDIR "/m_merc/mw_r_wal.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_STD,       ANIMSDIR "/m_merc/mw_r_std.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_WALK,      ANIMSDIR "/m_merc/mw_n_wal.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_STD,       ANIMSDIR "/m_merc/mw_n_std.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_DIE,         ANIMSDIR "/m_merc/mw_die.sti",      S_STRUCT,  0,                        8, -1),
	M(BGMWATER_N_AIM,       ANIMSDIR "/m_merc/mw_n_sht.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_R_AIM,       ANIMSDIR "/m_merc/mw_sr_am.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWATER_DBLSHT,      ANIMSDIR "/m_merc/mw_dbl.sti",      S_STRUCT,  0,                        8, -1),
	M(BGMWATER_TRANS,       ANIMSDIR "/m_merc/mw_fall.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_TRED,    ANIMSDIR "/m_merc/mw_tred.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_SWIM,    ANIMSDIR "/m_merc/mw_swim.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMDEEPWATER_DIE,     ANIMSDIR "/m_merc/mw_die2.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMHELIDROP,          ANIMSDIR "/m_merc/m_helidrp.sti",   S_STRUCT,  0,                        1, -1),
	M(BGMSLEEP,             ANIMSDIR "/m_merc/m_sleep.sti",     P_STRUCT,  0,                        8, -1),
	M(BGMSHOOT_LOW,         ANIMSDIR "/m_merc/m_shtlow.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMTHREATENSTAND,     ANIMSDIR "/m_merc/m_breth2.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMCDBLSHOT,          ANIMSDIR "/m_merc/mc_dblsh.sti",    C_STRUCT,  0,                        8, -1),
	M(BGMHURTSTANDINGN,     ANIMSDIR "/m_merc/m_i_br_n.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTSTANDINGR,     ANIMSDIR "/m_merc/m_i_br_r.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTWALKINGN,      ANIMSDIR "/m_merc/m_i_wk_n.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTWALKINGR,      ANIMSDIR "/m_merc/m_i_wk_r.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMHURTTRANS,         ANIMSDIR "/m_merc/m_i_tran.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMTHROWKNIFE,        ANIMSDIR "/m_merc/m_k_thro.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMBREATHKNIFE,       ANIMSDIR "/m_merc/m_knf_br.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLBREATH,      ANIMSDIR "/m_merc/m_p_brth.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMCOWER,             ANIMSDIR "/m_merc/m_cower.sti",     C_STRUCT,  0,                        8, -1),
	M(BGMRAISE2,            ANIMSDIR "/m_merc/m_raise2.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMROCKET,            ANIMSDIR "/m_merc/m_law.sti",       S_STRUCT,  0,                        8, -1),
	M(BGMMORTAR,            ANIMSDIR "/m_merc/m_mortar.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMSIDESTEP,          ANIMSDIR "/m_merc/m_r_sdsp.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMDBLBREATH,         ANIMSDIR "/m_merc/m_dbl_br.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMPUNCHLOW,          ANIMSDIR "/m_merc/m_pch_lo.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLSHOOTLOW,    ANIMSDIR "/m_merc/m_p_shlo.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWATERTHROW,        ANIMSDIR "/m_merc/mw_throw.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMWALK2,             ANIMSDIR "/m_merc/m_walk2.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMRUN2,              ANIMSDIR "/m_merc/m_r_run.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMIDLENECK,          ANIMSDIR "/m_merc/m_neck.sti",      S_STRUCT,  0,                        8, -1),
	M(BGMCROUCHTRANS,       ANIMSDIR "/m_merc/m_c_tran.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMRADIO,             ANIMSDIR "/m_merc/m_radio.sti",     S_STRUCT,  0,                        8, -1),
	M(BGMCRRADIO,           ANIMSDIR "/m_merc/m_c_rado.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMDWPRONE,           ANIMSDIR "/m_merc/m_db_prn.sti",    P_STRUCT,  0,                        8, -1),
	M(BGMDRUNK,             ANIMSDIR "/m_merc/m_r_drnk.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMPISTOLDRUNK,       ANIMSDIR "/m_merc/m_n_drnk.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMCROWBAR,           ANIMSDIR "/m_merc/m_crobar.sti",    S_STRUCT,  0,                        8, -1),
	M(BGMJUMPOVER,          ANIMSDIR "/m_merc/m_n_run.sti",     NO_STRUCT, 0,                        8, -1),


	M(RGFWALKING,           ANIMSDIR "/f_merc/f_r_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFSTANDING,          ANIMSDIR "/f_merc/f_breth2.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFCROUCHING,         ANIMSDIR "/f_merc/f_r_c.sti",       C_STRUCT,  0,                        8, -1),
	M(RGFSNEAKING,          ANIMSDIR "/f_merc/f_r_swat.sti",    C_STRUCT,  0,                        8, -1),
	M(RGFRUNNING,           ANIMSDIR "/f_merc/f_r_run.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFPRONE,             ANIMSDIR "/f_merc/f_r_prn.sti",     P_STRUCT,  0,                        8,  0),
	M(RGFSTANDAIM,          ANIMSDIR "/f_merc/f_sr_aim.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFHITHARD,           ANIMSDIR "/f_merc/f_diehd.sti",     FB_STRUCT, 0,                        8, -1),
	M(RGFHITSTAND,          ANIMSDIR "/f_merc/f_diefwd.sti",    F_STRUCT,  0,                        8, -1),
	M(RGFHITHARDBLOOD,      ANIMSDIR "/f_merc/f_diehdb.sti",    FB_STRUCT, 0,                        8, -1),
	M(RGFCROUCHAIM,         ANIMSDIR "/f_merc/f_cr_aim.sti",    C_STRUCT,  0,                        8, -1),
	M(RGFHITFALLBACK,       ANIMSDIR "/f_merc/f_diebac.sti",    F_STRUCT,  0,                        8, -1),
	M(RGFROLLOVER,          ANIMSDIR "/f_merc/f_roll.sti",      P_STRUCT,  0,                        8, -1),
	M(RGFCLIMBROOF,         ANIMSDIR "/f_merc/f_climb.sti",     S_STRUCT,  0,                        4, -1),
	M(RGFFALL,              ANIMSDIR "/f_merc/f_fall.sti",      S_STRUCT,  0,                        4, -1),
	M(RGFFALLF,             ANIMSDIR "/f_merc/f_fallf.sti",     S_STRUCT,  0,                        4, -1),
	M(RGFHITCROUCH,         ANIMSDIR "/f_merc/f_c_die.sti",     C_STRUCT,  0,                        8, -1),
	M(RGFHITPRONE,          ANIMSDIR "/f_merc/f_p_die.sti",     P_STRUCT,  0,                        8, -1),
	M(RGFHOPFENCE,          ANIMSDIR "/f_merc/f_hop.sti",       NO_STRUCT, 0,                        4, -1),
	M(RGFPUNCH,             ANIMSDIR "/f_merc/f_punch.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_STD,       ANIMSDIR "/f_merc/n_breth2.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_WALK,      ANIMSDIR "/f_merc/f_n_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_RUN,       ANIMSDIR "/f_merc/f_n_run.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_SWAT,      ANIMSDIR "/f_merc/f_n_swat.sti",    C_STRUCT,  0,                        8, -1),
	M(RGFNOTHING_CROUCH,    ANIMSDIR "/f_merc/f_n_crch.sti",    C_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_S_SHOT,    ANIMSDIR "/f_merc/f_n_shot.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_C_SHOT,    ANIMSDIR "/f_merc/f_n_c_ai.sti",    C_STRUCT,  0,                        8, -1),
	M(RGFHANDGUN_PRONE,     ANIMSDIR "/f_merc/f_n_prne.sti",    P_STRUCT,  0,                        8, -1),
	M(RGFDIE_JFK,           ANIMSDIR "/f_merc/f_diejfk.sti",    F_STRUCT,  0,                        8, -1),
	M(RGFOPEN,              ANIMSDIR "/f_merc/f_open.sti",      S_STRUCT,  0,                        8, -1),
	M(RGFPICKUP,            ANIMSDIR "/f_merc/f_pickup.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFSTAB,              ANIMSDIR "/f_merc/f_stab.sti",      S_STRUCT,  0,                        8, -1),
	M(RGFSLICE,             ANIMSDIR "/f_merc/f_slice.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFCSTAB,             ANIMSDIR "/f_merc/f_c_stb.sti",     C_STRUCT,  0,                        8, -1),
	M(RGFMEDIC,             ANIMSDIR "/f_merc/f_medic.sti",     C_STRUCT,  0,                        8, -1),
	M(RGFDODGE,             ANIMSDIR "/f_merc/f_dodge.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFSTANDDWALAIM,      ANIMSDIR "/f_merc/f_dblsht.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFRAISE,             ANIMSDIR "/f_merc/f_raise.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFTHROW,             ANIMSDIR "/f_merc/f_throw.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFLOB,               ANIMSDIR "/f_merc/f_lob.sti",       S_STRUCT,  0,                        8, -1),
	M(RGFKICKDOOR,          ANIMSDIR "/f_merc/f_dr_kck.sti",    S_STRUCT,  0,                        4, -1),
	M(RGFRHIT,              ANIMSDIR "/f_merc/f_r_hit.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFCLEAN,             ANIMSDIR "/f_merc/a_clean.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFKICKSN,            ANIMSDIR "/f_merc/a_kicksn.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFALOOK,             ANIMSDIR "/f_merc/a_look.sti",      S_STRUCT,  0,                        8, -1),
	M(RGFWIPE,              ANIMSDIR "/f_merc/a_wipe.sti",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_WALK,      ANIMSDIR "/f_merc/fw_r_wlk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_STD,       ANIMSDIR "/f_merc/fw_r_std.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_WALK,      ANIMSDIR "/f_merc/fw_n_wlk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_STD,       ANIMSDIR "/f_merc/fw_n_std.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_DIE,         ANIMSDIR "/f_merc/fw_die.sti",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_N_AIM,       ANIMSDIR "/f_merc/fw_n_sht.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_R_AIM,       ANIMSDIR "/f_merc/fw_sr_ai.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFWATER_DBLSHT,      ANIMSDIR "/f_merc/fw_dbl.sti",      S_STRUCT,  0,                        8, -1),
	M(RGFWATER_TRANS,       ANIMSDIR "/f_merc/fw_fall.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_TRED,    ANIMSDIR "/f_merc/fw_tred.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_SWIM,    ANIMSDIR "/f_merc/fw_swim.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFDEEPWATER_DIE,     ANIMSDIR "/f_merc/fw_die2.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFHELIDROP,          ANIMSDIR "/f_merc/f_helidrp.sti",   S_STRUCT,  0,                        1, -1),
	M(RGFSLEEP,             ANIMSDIR "/f_merc/f_sleep.sti",     P_STRUCT,  0,                        8, -1),
	M(RGFSHOOT_LOW,         ANIMSDIR "/f_merc/f_shtlow.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFCDBLSHOT,          ANIMSDIR "/f_merc/fc_dblsh.sti",    C_STRUCT,  0,                        8, -1),
	M(RGFHURTSTANDINGN,     ANIMSDIR "/f_merc/f_i_br_n.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTSTANDINGR,     ANIMSDIR "/f_merc/f_i_br_r.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTWALKINGN,      ANIMSDIR "/f_merc/f_i_wk_n.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTWALKINGR,      ANIMSDIR "/f_merc/f_i_wk_r.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFHURTTRANS,         ANIMSDIR "/f_merc/f_i_tran.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFTHROWKNIFE,        ANIMSDIR "/f_merc/f_k_thro.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFBREATHKNIFE,       ANIMSDIR "/f_merc/f_knf_br.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLBREATH,      ANIMSDIR "/f_merc/f_p_brth.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFCOWER,             ANIMSDIR "/f_merc/f_cower.sti",     C_STRUCT,  0,                        8, -1),
	M(RGFROCKET,            ANIMSDIR "/f_merc/f_law.sti",       S_STRUCT,  0,                        8, -1),
	M(RGFMORTAR,            ANIMSDIR "/f_merc/f_mortar.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFSIDESTEP,          ANIMSDIR "/f_merc/f_r_sdsp.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFDBLBREATH,         ANIMSDIR "/f_merc/f_dbl_br.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFPUNCHLOW,          ANIMSDIR "/f_merc/f_pch_lo.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLSHOOTLOW,    ANIMSDIR "/f_merc/f_p_shlo.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFWATERTHROW,        ANIMSDIR "/f_merc/fw_thrw.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFRADIO,             ANIMSDIR "/f_merc/f_radio.sti",     S_STRUCT,  0,                        8, -1),
	M(RGFCRRADIO,           ANIMSDIR "/f_merc/f_c_rado.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFSLAP,              ANIMSDIR "/f_merc/f_slap.sti",      S_STRUCT,  0,                        8, -1),
	M(RGFDWPRONE,           ANIMSDIR "/f_merc/f_db_prn.sti",    P_STRUCT,  0,                        8, -1),
	M(RGFDRUNK,             ANIMSDIR "/f_merc/f_r_drnk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFPISTOLDRUNK,       ANIMSDIR "/f_merc/f_n_drnk.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFCROWBAR,           ANIMSDIR "/f_merc/f_crobar.sti",    S_STRUCT,  0,                        8, -1),
	M(RGFJUMPOVER,          ANIMSDIR "/f_merc/f_n_run.sti",     NO_STRUCT, 0,                        8, -1),


	M(AFMONSTERSTANDING,    ANIMSDIR "/monsters/mn_breat.sti",  S_STRUCT,  0,                        8, -1),
	M(AFMONSTERWALKING,     ANIMSDIR "/monsters/mn_walk.sti",   S_STRUCT,  0,                        8, -1),
	M(AFMONSTERATTACK,      ANIMSDIR "/monsters/mn_attak.sti",  S_STRUCT,  0,                        8, -1),
	M(AFMONSTERCLOSEATTACK, ANIMSDIR "/monsters/m_attk2.sti",   S_STRUCT,  0,                        8, -1),
	M(AFMONSTERSPITATTACK,  ANIMSDIR "/monsters/m_spit.sti",    S_STRUCT,  0,                        8, -1),
	M(AFMONSTEREATING,      ANIMSDIR "/monsters/m_eat.sti",     S_STRUCT,  0,                        8, -1),
	M(AFMONSTERDIE,         ANIMSDIR "/monsters/mn_die1.sti",   S_STRUCT,  0,                        8, -1),
	M(AFMUP,                ANIMSDIR "/monsters/mn_up.sti",     S_STRUCT,  0,                        8, -1),
	M(AFMJUMP,              ANIMSDIR "/monsters/mn_jump.sti",   S_STRUCT,  0,                        8, -1),
	M(AFMMELT,              ANIMSDIR "/monsters/mn_melt.sti",   S_STRUCT,  0,                        8, -1),


	M(LVBREATH,             ANIMSDIR "/monsters/l_breath.sti",  S_STRUCT,  0,                        8, -1),
	M(LVDIE,                ANIMSDIR "/monsters/l_die.sti",     S_STRUCT,  0,                        8, -1),
	M(LVWALK,               ANIMSDIR "/monsters/l_walk.sti",    S_STRUCT,  0,                        8, -1),

	M(IBREATH,              ANIMSDIR "/monsters/i_breath.sti",  S_STRUCT,  0,                        8, -1),
	M(IWALK,                ANIMSDIR "/monsters/i_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(IDIE,                 ANIMSDIR "/monsters/i_die.sti",     S_STRUCT,  0,                        8, -1),
	M(IEAT,                 ANIMSDIR "/monsters/i_eat.sti",     S_STRUCT,  0,                        8, -1),
	M(IATTACK,              ANIMSDIR "/monsters/i_attack.sti",  S_STRUCT,  0,                        8, -1),

	M(QUEENMONSTERSTANDING, ANIMSDIR "/monsters/qmn_breat.sti", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERREADY,    ANIMSDIR "/monsters/q_ready.sti",   S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_SW,  ANIMSDIR "/monsters/q_spit_sw.sti", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_E,   ANIMSDIR "/monsters/q_spit_e.sti",  S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_NE,  ANIMSDIR "/monsters/q_spit_ne.sti", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_S,   ANIMSDIR "/monsters/q_spit_s.sti",  S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSPIT_SE,  ANIMSDIR "/monsters/q_spit_se.sti", S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERDEATH,    ANIMSDIR "/monsters/q_die.sti",     S_STRUCT,  0,                        1, -1),
	M(QUEENMONSTERSWIPE,    ANIMSDIR "/monsters/q_swipe.sti",   S_STRUCT,  0,                        1, -1),

	M(FATMANSTANDING,       ANIMSDIR "/civs/ft_brth.sti",       S_STRUCT,  0,                        8, -1),
	M(FATMANWALKING,        ANIMSDIR "/civs/ft_walk.sti",       S_STRUCT,  0,                        8, -1),
	M(FATMANRUNNING,        ANIMSDIR "/civs/ft_run.sti",        S_STRUCT,  0,                        8, -1),
	M(FATMANDIE,            ANIMSDIR "/civs/ft_die.sti",        F_STRUCT,  0,                        8, -1),
	M(FATMANASS,            ANIMSDIR "/civs/ft_scrtc.sti",      S_STRUCT,  0,                        8, -1),
	M(FATMANACT,            ANIMSDIR "/civs/ft_act.sti",        S_STRUCT,  0,                        8, -1),
	M(FATMANCOWER,          ANIMSDIR "/civs/ft_cower.sti",      C_STRUCT,  0,                        8, -1),
	M(FATMANDIE2,           ANIMSDIR "/civs/ft_die2.sti",       F_STRUCT,  0,                        8, -1),
	M(FATMANCOWERHIT,       ANIMSDIR "/civs/f_cw_hit.sti",      C_STRUCT,  0,                        8, -1),

	M(MANCIVSTANDING,       ANIMSDIR "/civs/m_breath.sti",      S_STRUCT,  0,                        8, -1),
	M(MANCIVWALKING,        ANIMSDIR "/civs/m_walk.sti",        S_STRUCT,  0,                        8, -1),
	M(MANCIVRUNNING,        ANIMSDIR "/civs/m_run.sti",         S_STRUCT,  0,                        8, -1),
	M(MANCIVDIE,            ANIMSDIR "/civs/m_die.sti",         F_STRUCT,  0,                        8, -1),
	M(MANCIVACT,            ANIMSDIR "/civs/m_act.sti",         S_STRUCT,  0,                        8, -1),
	M(MANCIVCOWER,          ANIMSDIR "/civs/m_cower.sti",       C_STRUCT,  0,                        8, -1),
	M(MANCIVDIE2,           ANIMSDIR "/civs/m_die2.sti",        F_STRUCT,  0,                        8, -1),
	M(MANCIVSMACKED,        ANIMSDIR "/civs/m_smcked.sti",      S_STRUCT,  0,                        8, -1),
	M(MANCIVPUNCH,          ANIMSDIR "/civs/m_punch.sti",       S_STRUCT,  0,                        8, -1),
	M(MANCIVCOWERHIT,       ANIMSDIR "/civs/m_cw_hit.sti",      C_STRUCT,  0,                        8, -1),

	M(MINICIVSTANDING,      ANIMSDIR "/civs/mi_breath.sti",     S_STRUCT,  0,                        8, -1),
	M(MINICIVWALKING,       ANIMSDIR "/civs/mi_walk.sti",       S_STRUCT,  0,                        8, -1),
	M(MINICIVRUNNING,       ANIMSDIR "/civs/mi_run.sti",        S_STRUCT,  0,                        8, -1),
	M(MINICIVDIE,           ANIMSDIR "/civs/mi_die.sti",        F_STRUCT,  0,                        8, -1),
	M(MINISTOCKING,         ANIMSDIR "/civs/mi_stkng.sti",      S_STRUCT,  0,                        8, -1),
	M(MINIACT,              ANIMSDIR "/civs/mi_act.sti",        S_STRUCT,  0,                        8, -1),
	M(MINICOWER,            ANIMSDIR "/civs/mi_cower.sti",      C_STRUCT,  0,                        8, -1),
	M(MINIDIE2,             ANIMSDIR "/civs/mi_die2.sti",       F_STRUCT,  0,                        8, -1),
	M(MINICOWERHIT,         ANIMSDIR "/civs/s_cw_hit.sti",      C_STRUCT,  0,                        8, -1),

	M(DRESSCIVSTANDING,     ANIMSDIR "/civs/ds_breath.sti",     S_STRUCT,  0,                        8, -1),
	M(DRESSCIVWALKING,      ANIMSDIR "/civs/ds_walk.sti",       S_STRUCT,  0,                        8, -1),
	M(DRESSCIVRUNNING,      ANIMSDIR "/civs/ds_run.sti",        S_STRUCT,  0,                        8, -1),
	M(DRESSCIVDIE,          ANIMSDIR "/civs/ds_die.sti",        F_STRUCT,  0,                        8, -1),
	M(DRESSCIVACT,          ANIMSDIR "/civs/ds_act.sti",        S_STRUCT,  0,                        8, -1),
	M(DRESSCIVCOWER,        ANIMSDIR "/civs/ds_cower.sti",      C_STRUCT,  0,                        8, -1),
	M(DRESSCIVDIE2,         ANIMSDIR "/civs/ds_die2.sti",       F_STRUCT,  0,                        8, -1),
	M(DRESSCIVCOWERHIT,     ANIMSDIR "/civs/w_cw_hit.sti",      C_STRUCT,  0,                        8, -1),

	M(HATKIDCIVSTANDING,    ANIMSDIR "/civs/h_breath.sti",      S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVWALKING,     ANIMSDIR "/civs/h_walk.sti",        S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVRUNNING,     ANIMSDIR "/civs/h_run.sti",         S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVDIE,         ANIMSDIR "/civs/h_die2.sti",        F_STRUCT,  0,                        8, -1),
	M(HATKIDCIVJFK,         ANIMSDIR "/civs/h_diejfk.sti",      S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVYOYO,        ANIMSDIR "/civs/h_yoyo.sti",        S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVACT,         ANIMSDIR "/civs/h_act.sti",         S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVCOWER,       ANIMSDIR "/civs/h_cower.sti",       S_STRUCT,  0,                        8, -1),
	M(HATKIDCIVDIE2,        ANIMSDIR "/civs/h_die.sti",         F_STRUCT,  0,                        8, -1),
	M(HATKIDCIVCOWERHIT,    ANIMSDIR "/civs/h_cw_hit.sti",      C_STRUCT,  0,                        8, -1),
	M(HATKIDCIVSKIP,        ANIMSDIR "/civs/h_skip.sti",        C_STRUCT,  0,                        8, -1),

	M(KIDCIVSTANDING,       ANIMSDIR "/civs/k_breath.sti",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVWALKING,        ANIMSDIR "/civs/k_walk.sti",        S_STRUCT,  0,                        8, -1),
	M(KIDCIVRUNNING,        ANIMSDIR "/civs/k_run.sti",         S_STRUCT,  0,                        8, -1),
	M(KIDCIVDIE,            ANIMSDIR "/civs/k_die2.sti",        F_STRUCT,  0,                        8, -1),
	M(KIDCIVJFK,            ANIMSDIR "/civs/k_diejfk.sti",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVARMPIT,         ANIMSDIR "/civs/k_armpit.sti",      S_STRUCT,  0,                        8, -1),
	M(KIDCIVACT,            ANIMSDIR "/civs/k_act.sti",         S_STRUCT,  0,                        8, -1),
	M(KIDCIVCOWER,          ANIMSDIR "/civs/k_cower.sti",       S_STRUCT,  0,                        8, -1),
	M(KIDCIVDIE2,           ANIMSDIR "/civs/k_die.sti",         F_STRUCT,  0,                        8, -1),
	M(KIDCIVCOWERHIT,       ANIMSDIR "/civs/k_cw_hit.sti",      C_STRUCT,  0,                        8, -1),
	M(KIDCIVSKIP,           ANIMSDIR "/civs/k_skip.sti",        C_STRUCT,  0,                        8, -1),

	M(CRIPCIVSTANDING,      ANIMSDIR "/civs/cp_breth.sti",      S_STRUCT,  0,                        8, -1),
	M(CRIPCIVWALKING,       ANIMSDIR "/civs/cp_walk.sti",       S_STRUCT,  0,                        8, -1),
	M(CRIPCIVRUNNING,       ANIMSDIR "/civs/cp_run.sti",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVBEG,           ANIMSDIR "/civs/cp_beg.sti",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVDIE,           ANIMSDIR "/civs/cp_die.sti",        S_STRUCT,  0,                        8, -1),
	M(CRIPCIVDIE2,          ANIMSDIR "/civs/cp_die2.sti",       S_STRUCT,  0,                        8, -1),
	M(CRIPCIVKICK,          ANIMSDIR "/civs/cp_kick.sti",       S_STRUCT,  0,                        8, -1),

	M(COWSTANDING,          ANIMSDIR "/animals/c_breath.sti",   S_STRUCT,  0,                        8, -1),
	M(COWWALKING,           ANIMSDIR "/animals/c_walk.sti",     S_STRUCT,  0,                        8, -1),
	M(COWDIE,               ANIMSDIR "/animals/c_die.sti",      S_STRUCT,  0,                        8, -1),
	M(COWEAT,               ANIMSDIR "/animals/c_eat.sti",      S_STRUCT,  0,                        8, -1),

	M(CROWWALKING,          ANIMSDIR "/animals/cr_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(CROWFLYING,           ANIMSDIR "/animals/cr_fly.sti",     S_STRUCT,  0,                        8, -1),
	M(CROWEATING,           ANIMSDIR "/animals/cr_eat.sti",     S_STRUCT,  0,                        8, -1),
	M(CROWDYING,            ANIMSDIR "/animals/cr_die.sti",     NO_STRUCT, 0,                        8, -1),

	M(CATBREATH,            ANIMSDIR "/animals/ct_breath.sti",  S_STRUCT,  0,                        8, -1),
	M(CATWALK,              ANIMSDIR "/animals/ct_walk.sti",    S_STRUCT,  0,                        8, -1),
	M(CATRUN,               ANIMSDIR "/animals/ct_run.sti",     S_STRUCT,  0,                        8, -1),
	M(CATREADY,             ANIMSDIR "/animals/ct_ready.sti",   S_STRUCT,  0,                        8, -1),
	M(CATHIT,               ANIMSDIR "/animals/ct_hit.sti",     S_STRUCT,  0,                        8, -1),
	M(CATDIE,               ANIMSDIR "/animals/ct_die.sti",     S_STRUCT,  0,                        8, -1),
	M(CATSWIPE,             ANIMSDIR "/animals/ct_swipe.sti",   S_STRUCT,  0,                        8, -1),
	M(CATBITE,              ANIMSDIR "/animals/ct_bite.sti",    S_STRUCT,  0,                        8, -1),

	M(ROBOTNWBREATH,        ANIMSDIR "/civs/j_r_bret.sti",      S_STRUCT,  0,                        8, -1),
	M(ROBOTNWWALK,          ANIMSDIR "/civs/j_r_walk.sti",      S_STRUCT,  0,                        8, -1),
	M(ROBOTNWHIT,           ANIMSDIR "/civs/j_r_hit.sti"  ,     S_STRUCT,  0,                        8, -1),
	M(ROBOTNWDIE,           ANIMSDIR "/civs/j_r_die.sti",       S_STRUCT,  0,                        8, -1),
	M(ROBOTNWSHOOT,         ANIMSDIR "/civs/j_r_shot.sti",      S_STRUCT,  0,                        8, -1),

	M(HUMVEE_BASIC,         ANIMSDIR "/vehicles/hummer2.sti",   S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(HUMVEE_DIE,           ANIMSDIR "/vehicles/hm_wrek.sti",   S_STRUCT,  0,                        2, -1),

	M(TANKNW_READY,         ANIMSDIR "/vehicles/tank_rot.sti",  S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(TANKNW_SHOOT,         ANIMSDIR "/vehicles/tank_sht.sti",  S_STRUCT,  0,                        8, -1),
	M(TANKNW_DIE,           ANIMSDIR "/vehicles/tk_wrek.sti",   S_STRUCT,  0,                        1, -1),

	M(TANKNE_READY,         ANIMSDIR "/vehicles/tnk2_rot.sti",  S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(TANKNE_SHOOT,         ANIMSDIR "/vehicles/tnk2_sht.sti",  S_STRUCT,  0,                        8, -1),
	M(TANKNE_DIE,           ANIMSDIR "/vehicles/tk2_wrek.sti",  S_STRUCT,  0,                        1, -1),

	M(ELDORADO_BASIC,       ANIMSDIR "/vehicles/hummer.sti",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(ELDORADO_DIE,         ANIMSDIR "/vehicles/hm_wrek.sti",   NO_STRUCT, 0,                        2, -1),

	M(ICECREAMTRUCK_BASIC,  ANIMSDIR "/vehicles/icecrm.sti",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(ICECREAMTRUCK_DIE,    ANIMSDIR "/vehicles/hm_wrek.sti",   NO_STRUCT, 0,                        2, -1),

	M(JEEP_BASIC,           ANIMSDIR "/vehicles/hummer.sti",    S_STRUCT,  ANIM_DATA_FLAG_NOFRAMES, 32, -1),
	M(JEEP_DIE,             ANIMSDIR "/vehicles/hm_wrek.sti",   NO_STRUCT, 0,                        2, -1),

	M(BODYEXPLODE,          ANIMSDIR "/s_merc/bod_blow.sti",    NO_STRUCT, 0,                        1, -1),
};

#undef M


struct AnimationStructureType
{
	const char* const   Filename;
	STRUCTURE_FILE_REF* pStructureFileRef;
};


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
	INT32 cnt1, cnt2;

	LoadAnimationStateInstructions();
	InitAnimationSurfacesPerBodytype();
	LoadAnimationProfiles();

	// OK, Load all animation structures.....
	for ( cnt1 = 0; cnt1 < TOTALBODYTYPES; cnt1++ )
	{
		for ( cnt2 = 0; cnt2 < NUM_STRUCT_IDS; cnt2++ )
		{
			ST::string const Filename{gAnimStructureDatabase[cnt1][cnt2].Filename};

			if (GCM->doesGameResExists(Filename))
			{
				STRUCTURE_FILE_REF* pStructureFileRef = LoadStructureFile(Filename);

				// fix non-base prone tiles by making them all passable (#2116)
				if ((cnt1 >= REGMALE && cnt1 <= REGFEMALE) && cnt2 == P_STRUCT)
				{
					for (UINT8 dirIdx = 0; dirIdx < 8; dirIdx++)
					{
						DB_STRUCTURE_REF const* const pDBStructureRef = &pStructureFileRef->pDBStructureRef[dirIdx];
						DB_STRUCTURE_TILE** ppTile = pDBStructureRef->ppTile;
						UINT8 const n_tiles = pDBStructureRef->pDBStructure->ubNumberOfTiles;

						for (UINT8 tileIdx = 0; tileIdx < n_tiles; tileIdx++)
						{
							// if not a base tile
							if (ppTile[tileIdx]->sPosRelToBase != 0)
							{
								ppTile[tileIdx]->fFlags |= TILE_PASSABLE;
							}
						}
					}
				}

				gAnimStructureDatabase[ cnt1 ][ cnt2 ].pStructureFileRef = pStructureFileRef;
			}
		}
	}
}


static void DeleteAnimationProfiles(void);


void DeInitAnimationSystem()
{
	FOR_EACH(AnimationSurfaceType, i, gAnimSurfaceDatabase)
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
	INT8 bStructDataType;

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
	return InternalGetAnimationStructureRef(s, usSurfaceIndex, usAnimState, false);
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
		SLOGD("Surface Database: Hit {}", usSurfaceIndex);
	}
	else
	{
		try
		{
			// Load into memory
			SLOGD("Surface Database: Loading {}", usSurfaceIndex);

			AutoSGPImage   hImage(CreateImage(a->Filename, IMAGE_ALLDATA));
			AutoSGPVObject hVObject(AddVideoObjectFromHImage(hImage.get()));

			// Get aux data
			if (hImage->uiAppDataSize != hVObject->SubregionCount() * sizeof(AuxObjectData))
			{
				throw std::runtime_error("Invalid # of animations given");
			}

			// Valid auxiliary data, so get # of frames from data
			AuxObjectData const* const pAuxData = (AuxObjectData const*)(UINT8 const*)hImage->pAppData;
			a->uiNumFramesPerDir = pAuxData->ubNumberOfFrames;

			// get structure data if any
			const STRUCTURE_FILE_REF* const pStructureFileRef = InternalGetAnimationStructureRef(ID2SOLDIER(usSoldierID), usSurfaceIndex, usAnimState, true);
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

				AddZStripInfoToVObject(hVObject.get(), pStructureFileRef, true, sStartFrame);
			}

			// Set video object index
			a->hVideoObject = hVObject.release();

			// Determine if we have a problem with #frames + directions ( ie mismatch )
			if (a->uiNumDirections * a->uiNumFramesPerDir != a->hVideoObject->SubregionCount())
			{
				SLOGW("Surface Database: Surface {} has #frames mismatch.", usSurfaceIndex);
			}
		}
		catch (...)
		{
			SLOGE("Could not load animation file: {}", a->Filename);
			throw;
		}
	}

	// Increment usage count only if history for soldier is not yet set
	if (gbAnimUsageHistory[usSurfaceIndex][usSoldierID] == 0)
	{
		SLOGD("Surface Database: Incrementing Usage {} ( Soldier {} )", usSurfaceIndex, usSoldierID);
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
		SLOGW("Surface Database: Soldier has tried to unlock surface that he has not locked.");
		return;
	}
	*in_use = 0; // Set history for particular sodlier

	SLOGD("Surface Database: Decrementing Usage {} ( Soldier {} )", usSurfaceIndex, usSoldierID);

	AnimationSurfaceType* const a         = &gAnimSurfaceDatabase[usSurfaceIndex];
	INT8*                 const use_count = &a->bUsageCount;
	--*use_count;

	SLOGD("Surface Database: MercUsage: {}, Global Usage: {}", *in_use, *use_count);

	Assert(*use_count >= 0);
	if (*use_count < 0) *use_count = 0;

	// Delete if count reched zero
	if (*use_count == 0)
	{
		SLOGD("Surface Database: Unloading Surface: {}", usSurfaceIndex);
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
	AutoSGPFile f(GCM->openGameResForReading(ANIMPROFILEFILENAME));

	f->read(&gubNumAnimProfiles, sizeof(gubNumAnimProfiles));

	ANIM_PROF* const aps = new ANIM_PROF[gubNumAnimProfiles]{};
	gpAnimProfiles = aps;

	for (INT32 profile_idx = 0; profile_idx < gubNumAnimProfiles; ++profile_idx)
	{
		ANIM_PROF* const ap = &aps[profile_idx];
		for (INT32 direction_idx = 0; direction_idx < 8; ++direction_idx)
		{
			ANIM_PROF_DIR* const apd = &ap->Dirs[direction_idx];

			f->read(&apd->ubNumTiles, sizeof(UINT8));
			ANIM_PROF_TILE* const apts = new ANIM_PROF_TILE[apd->ubNumTiles]{};
			apd->pTiles = apts;

			for (INT32 tile_idx = 0; tile_idx < apd->ubNumTiles; ++tile_idx)
			{
				ANIM_PROF_TILE* const apt = &apts[tile_idx];
				f->read(&apt->usTileFlags, sizeof(UINT16));
				f->read(&apt->bTileX,      sizeof(INT8));
				f->read(&apt->bTileY,      sizeof(INT8));
			}
		}
	}
}
catch (...)
{
	SLOGE("Problems initializing Animation Profiles");
	throw;
}


static void DeleteAnimationProfiles(void)
{
	INT32 iProfileCount, iDirectionCount;
	ANIM_PROF_DIR *pProfileDir;

	// Loop profiles
	for ( iProfileCount = 0; iProfileCount < gubNumAnimProfiles; iProfileCount++ )
	{
		// Loop directions
		for ( iDirectionCount = 0; iDirectionCount < 8; iDirectionCount++ )
		{
			// Get prodile direction pointer
			pProfileDir = &( gpAnimProfiles[ iProfileCount ].Dirs[ iDirectionCount ] );

			// Free tile
			delete[] pProfileDir->pTiles;

		}
	}

	// Free profile data!
	delete[] gpAnimProfiles;

}


void ZeroAnimSurfaceCounts( )
{
	INT32 cnt;

	for ( cnt = 0; cnt < NUMANIMATIONSURFACETYPES; cnt++ )
	{
		gAnimSurfaceDatabase[ cnt ].bUsageCount   = 0;
		gAnimSurfaceDatabase[ cnt ].hVideoObject  = NULL;
	}

	for (auto& i : gbAnimUsageHistory)
	{
		std::fill(std::begin(i), std::end(i), 0);
	}
}
