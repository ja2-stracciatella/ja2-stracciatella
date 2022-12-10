#include "Cursors.h"
#include "Directories.h"
#include "Timer_Control.h"
#include "Touch_UI.h"
#include "Font.h"
#include "Font_Control.h"
#include "Handle_UI.h"
#include "Interface.h"
#include "Overhead.h"
#include "Cursor_Control.h"
#include "Sound_Control.h"
#include "Video.h"
#include "VSurface.h"

#include <string_theory/format>
#include <string_theory/string>


#define SCURSOR(name)         { name, NULL, 0,                   0 }
#define ECURSOR()             { NULL, NULL, USE_OUTLINE_BLITTER, 0 }

static CursorFileData CursorFileDatabase[] =
{
	SCURSOR(CURSORSDIR "/cursor.sti"),
	SCURSOR(CURSORSDIR "/cur_targ.sti"),
	SCURSOR(CURSORSDIR "/cur_tagr.sti"),
	SCURSOR(CURSORSDIR "/targblak.sti"),
	SCURSOR(CURSORSDIR "/cur_bst.sti"),
	SCURSOR(CURSORSDIR "/cur_rbst.sti"),
	SCURSOR(CURSORSDIR "/burstblk.sti"),
	SCURSOR(CURSORSDIR "/cur_tr.sti"),
	SCURSOR(CURSORSDIR "/cur_trw.sti"),
	SCURSOR(CURSORSDIR "/cur_tb.sti"),

	SCURSOR(CURSORSDIR "/punch.sti"),
	SCURSOR(CURSORSDIR "/punchr.sti"),
	SCURSOR(CURSORSDIR "/cur_run.sti"),
	SCURSOR(CURSORSDIR "/cur_walk.sti"),
	SCURSOR(CURSORSDIR "/cur_swat.sti"),
	SCURSOR(CURSORSDIR "/cur_pron.sti"),
	SCURSOR(CURSORSDIR "/grabsr.sti"),
	SCURSOR(CURSORSDIR "/grabs.sti"),
	SCURSOR(CURSORSDIR "/stab.sti"),
	SCURSOR(CURSORSDIR "/stabr.sti"),

	SCURSOR(CURSORSDIR "/cross1.sti"),
	SCURSOR(CURSORSDIR "/cross2.sti"),
	SCURSOR(LAPTOPDIR "/fingercursor.sti"),
	SCURSOR(LAPTOPDIR "/laptopcursor.sti"),
	SCURSOR(CURSORSDIR "/ibeam.sti"),
	SCURSOR(CURSORSDIR "/cur_look.sti"),
	SCURSOR(CURSORSDIR "/cur_talk.sti"),
	SCURSOR(CURSORSDIR "/cur_talkb.sti"),
	SCURSOR(CURSORSDIR "/cur_talkr.sti"),
	SCURSOR(CURSORSDIR "/cur_exit.sti"),

	SCURSOR(CURSORSDIR "/vehiclecursor.sti"),
	SCURSOR(CURSORSDIR "/walkingcursor.sti"),
	SCURSOR(CURSORSDIR "/que.sti"),
	SCURSOR(CURSORSDIR "/chopper.sti"),
	SCURSOR(CURSORSDIR "/check.sti"),
	SCURSOR(CURSORSDIR "/cur_try.sti"),
	SCURSOR(CURSORSDIR "/wirecut.sti"),
	SCURSOR(CURSORSDIR "/wirecutr.sti"),
	SCURSOR(CURSORSDIR "/bullet_g.sti"),
	SCURSOR(CURSORSDIR "/bullet_d.sti"),
	SCURSOR(CURSORSDIR "/ibeamwhite.sti"),
	SCURSOR(CURSORSDIR "/throwg.sti"),
	SCURSOR(CURSORSDIR "/throwb.sti"),
	SCURSOR(CURSORSDIR "/throwr.sti"),
	ECURSOR(),
	SCURSOR(CURSORSDIR "/bombg.sti"),
	SCURSOR(CURSORSDIR "/bombr.sti"),
	SCURSOR(CURSORSDIR "/remoteg.sti"),
	SCURSOR(CURSORSDIR "/remoter.sti"),
	SCURSOR(CURSORSDIR "/steering.sti"),
	SCURSOR(CURSORSDIR "/cur_car.sti"),
	SCURSOR(CURSORSDIR "/cur_wait.sti"),

	//Tactical GUI cursors
	SCURSOR(CURSORSDIR "/singlecursor.sti"),
	SCURSOR(CURSORSDIR "/groupcursor.sti"),
	SCURSOR(CURSORSDIR "/singledcursor.sti"),
	SCURSOR(CURSORSDIR "/groupdcursor.sti"),
	SCURSOR(CURSORSDIR "/repair.sti"),
	SCURSOR(CURSORSDIR "/repairr.sti"),
	SCURSOR(CURSORSDIR "/jar_cur.sti"),
	SCURSOR(CURSORSDIR "/jar_cur_red.sti"),
	SCURSOR(CURSORSDIR "/cur_x.sti"),
	SCURSOR(CURSORSDIR "/can_01.sti"),
	SCURSOR(CURSORSDIR "/can_02.sti"),
	SCURSOR(CURSORSDIR "/cur_swit.sti"),
	SCURSOR(CURSORSDIR "/bullseye.sti"),
	SCURSOR(CURSORSDIR "/deadleap.sti"),
	SCURSOR(CURSORSDIR "/can_01.sti"),
	SCURSOR(CURSORSDIR "/can_02.sti")
};

#undef SCURSOR
#undef ECURSOR


#define SUBNONE()         0,   0,   0, 0,                0
#define SUBCENT(cur, idx) cur, idx, 0, CENTER_SUBCURSOR, CENTER_SUBCURSOR
#define SUBCNTX(cur, idx) cur, idx, 0, CENTER_SUBCURSOR, 0
#define SUBHIDE(cur, idx) cur, idx, 0, HIDE_SUBCURSOR,   HIDE_SUBCURSOR
#define SUBNORM(cur, idx) cur, idx, 0, 0,                0
// Strictly speaking the CursorDataBrace is missing lots of braces,
// but globally disabling the warning seems overblown.
#pragma GCC diagnostic ignored "-Wmissing-braces"

static CursorData CursorDatabase[] =
{
	{ SUBNORM(C_MISC, 0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	0,  0, 0, 0									, 0, 0, TRUE },

	{ SUBHIDE(C_TRINGS,     6), //SUBCENT(C_TRINGS, 6),
		SUBCENT(C_ACTIONMODE, 0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

		// TARGET ( NORMAL W/ RINGS )
	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              5),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              1),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },


	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              2),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              3),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              7),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              8),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TRINGS,              4),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

		// TARGET WITH WHITE RINGS
	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TWRINGS,             0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },


	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TWRINGS,             1),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_TWRINGS,             2),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_TWRINGS,             3),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_TWRINGS,             4),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

		// TARGET RED CURSOR
	{ SUBCENT(C_ACTIONMODERED,       0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },


		// TARGET BLACK CURSOR
	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_BLACKTARGET,         0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },


		// TARGET DARK BLACK CURSOR
	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

		// TARGET RED CURSOR

	{
		SUBCENT(C_TARGMODEBURSTRED,    0),
		SUBCENT(C_TARGMODEBURSTBLACK,  0),
		SUBCENT(C_TRINGS,              1),
		SUBCENT(C_TRINGS,              2),
		SUBCENT(C_TRINGS,              3),
		5,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{ SUBCENT(C_TARGMODEBURST,       0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_TARGMODEBURSTRED,    0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },


	{ SUBCENT(C_TARGMODEBURSTBLACK,  0),
		SUBHIDE(C_TRINGS,                                        6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_PUNCHGRAY,           0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_PUNCHRED,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBCENT(C_PUNCHRED,            0),
		SUBCENT(C_TRINGS,              1),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{	SUBCENT(C_PUNCHRED,            0),
		SUBCENT(C_TRINGS,              2),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBCENT(C_PUNCHRED,            0),
		SUBCENT(C_YELLOWRINGS,         1),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{	SUBCENT(C_PUNCHRED,            0),
		SUBCENT(C_YELLOWRINGS,         2),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },


	{
		SUBCENT(C_PUNCHRED,            0),
		SUBCENT(C_TWRINGS,             1),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{
		SUBCENT(C_PUNCHRED,            0),
		SUBCENT(C_TWRINGS,             2),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },


	{ SUBHIDE(C_TRINGS,              6),
		SUBCNTX(C_RUN1,                0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, 20  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCNTX(C_WALK1,               0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, 20 , 0, 0					, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCNTX(C_SWAT1,               0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, 10  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCNTX(C_PRONE1,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, 10 , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              0),
		SUBCENT(C_GRAB1,               0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              0),
		SUBCENT(C_GRAB2,               0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_KNIFE1,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0							, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_KNIFE2,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{	SUBCENT(C_KNIFE2,              0),
		SUBCENT(C_TRINGS,              1),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0									, 0, 0 },

	{	SUBCENT(C_KNIFE2,              0),
		SUBCENT(C_TRINGS,              2),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0									, 0, 0 },

	{	SUBCENT(C_KNIFE2,              0),
		SUBCENT(C_YELLOWRINGS,         1),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0									, 0, 0 },

	{	SUBCENT(C_KNIFE2,              0),
		SUBCENT(C_YELLOWRINGS,         2),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0									, 0, 0 },

	{	SUBCENT(C_KNIFE2,              0),
		SUBCENT(C_TWRINGS,             1),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0									, 0, 0 },

	{	SUBCENT(C_KNIFE2,              0),
		SUBCENT(C_TWRINGS,             2),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0									, 0, 0 },

	{ SUBNORM(C_CROSS1,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBNORM(C_CROSS2,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBNORM(C_WWW,                 0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	4,  0, 0, 0									, 0, 0, TRUE },

	{ SUBNORM(C_LAPTOPSCREEN,        0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	0,  0, 0, 0									, 0, 0, TRUE },

	{ SUBNORM(C_IBEAM,               0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, CENTER_CURSOR, 0, 0 , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBNORM(C_LOOK,                0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR, 0, 0 , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_TALK,                0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR, 0, 0 , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_BLACKTALK,           0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR, 0, 0 , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_REDTALK,             0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR, 0, 0 , 0, 0 },

		// Exit arrows
	{ SUBNORM(C_EXITARROWS,          0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, TOP_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          1),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, BOTTOM_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          2),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	RIGHT_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          3),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	LEFT_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          4),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, TOP_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          5),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, BOTTOM_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	RIGHT_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          7),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	LEFT_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          8),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, TOP_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,          9),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, BOTTOM_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,         10),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	RIGHT_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,         11),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	LEFT_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_STRATVEH,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_STRATFOOT,           0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_INVALIDACTION,       0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },
	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_CHOPPER,             0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_ACTIONMODE,          0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0,  CURSOR_TO_FLASH, 0 },

	{ SUBCENT(C_TARGMODEBURST,       0),
		SUBCENT(C_TARGMODEBURSTBLACK,  0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0,  CURSOR_TO_FLASH, 0 },

	{
		SUBCENT(C_TALK,                0),
		SUBCENT(C_BLACKTALK,           0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3, CENTER_CURSOR, CENTER_CURSOR, 0, 0, CURSOR_TO_FLASH, 0 },

	{
		SUBCENT(C_REDTALK,             0),
		SUBCENT(C_BLACKTALK,           0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3, CENTER_CURSOR, CENTER_CURSOR, 0, 0, CURSOR_TO_FLASH, 0 },

	{
		SUBNORM(C_CHECKMARK,           0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, CENTER_CURSOR, 0, 0, 0, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_YELLOWRINGS,         5),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_YELLOWRINGS,         0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_YELLOWRINGS,         1),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_YELLOWRINGS,         2),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_ACTIONMODERED,       0),
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_YELLOWRINGS,         3),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{ SUBNORM(C_EXITARROWS,         12),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,         13),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBNORM(C_EXITARROWS,         14),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_WIRECUTR,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_WIRECUT,             0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_RELOAD,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , CURSOR_TO_FLASH, 0 },

	{
		SUBCENT(C_ACTIONMODEBLACK,     0),
		SUBCENT(C_RELOADR,             0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , CURSOR_TO_FLASH, 0 },

	{ SUBNORM(C_IBEAM_WHITE,         0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, CENTER_CURSOR, 0, 0 , 0, 0 },

	{
		SUBCENT(C_THROWG,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_THROWB,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_THROWG,              0),
		SUBCENT(C_THROWB,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , CURSOR_TO_FLASH, 0 },


	// THROW CURSORS W/ RINGS
	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              5),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              1),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },


	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              2),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              3),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              7),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              8),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TRINGS,              4),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

		// TARGET WITH WHITE RINGS
	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TWRINGS,             0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TWRINGS,             1),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_TWRINGS,             2),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_TWRINGS,             3),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_TWRINGS,             4),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

		// YELLOW RINGS
	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_YELLOWRINGS,         5),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_YELLOWRINGS,         0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_YELLOWRINGS,         1),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_YELLOWRINGS,         2),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	{
		SUBCENT(C_THROWR,              0),
		SUBCENT(C_THROWB,              0),
		SUBCENT(C_YELLOWRINGS,         3),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0   , CURSOR_TO_SUB_CONDITIONALLY, 0 },

	// ITEM THROW ONES...
	{
		SUBCENT(C_ITEMTHROW,           0),
		SUBCENT(C_THROWG,              0),
		SUBHIDE(C_TRINGS,              0),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_THROWB,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_ITEMTHROW,           0),
		SUBCENT(C_THROWR,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		3,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_ITEMTHROW,           0),
		SUBCENT(C_THROWG,              0),
		SUBCENT(C_THROWB,              0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		4,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , CURSOR_TO_FLASH2, 0 },

	{
		SUBCENT(C_ITEMTHROW,           0),
		SUBHIDE(C_TRINGS,              6),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , CURSOR_TO_FLASH2, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_BOMB_GREY,           0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_BOMB_RED,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_REMOTE_GREY,         0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_REMOTE_RED,          0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_ENTERV,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_MOVEV,               0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0					, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_WAIT,                0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0					, DELAY_START_CURSOR, 0 },

	//Tactical Placement GUI cursors
	{ SUBNORM(C_PLACEMERC,           0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, BOTTOM_CURSOR, 0, 0, 0, 0 },
	{ SUBNORM(C_PLACEGROUP,          0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, BOTTOM_CURSOR, 0, 0, 0, 0 },
	{ SUBNORM(C_DPLACEMERC,          0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, BOTTOM_CURSOR, 0, 0, 0, 0 },

	{ SUBNORM(C_DPLACEGROUP,         0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, BOTTOM_CURSOR, 0, 0, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_REPAIR,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_REPAIRR,             0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_JAR,                 0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_JARRED,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_CAN,                 0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_CANRED,              0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{
		SUBCENT(C_X,                   0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1, CENTER_CURSOR, CENTER_CURSOR  , 0, 0									, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_WAIT,                0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0					, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_EXCHANGE,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0					, 0, 0 },

	{ SUBCENT(C_BULLSEYE,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		1,	CENTER_CURSOR, CENTER_CURSOR, 0, 0,									0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_JUMPOVER,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2, CENTER_CURSOR, CENTER_CURSOR , 0, 0					, 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_FUEL,                0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },

	{ SUBHIDE(C_TRINGS,              6),
		SUBCENT(C_FUEL_RED,            0),
		SUBNONE(),
		SUBNONE(),
		SUBNONE(),
		2,	CENTER_CURSOR, CENTER_CURSOR, 0, 0  , 0, 0 },
};

#undef SUBNONE
#undef SUBCENT
#undef SUBCNTX
#undef SUBHIDE
#undef SUBNORM


static void BltJA2CursorData(void);


void InitCursors(void)
{
	InitCursorDatabase(CursorFileDatabase, CursorDatabase, NUM_CURSOR_FILES);
	SetMouseBltHook(BltJA2CursorData);
}


static void UpdateFlashingCursorFrames(UINT32 uiCursorIndex);


void HandleAnimatedCursors(void)
{
	if (COUNTERDONE(CURSORCOUNTER))
	{
		RESETCOUNTER(CURSORCOUNTER);

		if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA)
		{
			UpdateAnimatedCursorFrames(gViewportRegion.Cursor);
			SetCurrentCursorFromDatabase(gViewportRegion.Cursor);
		}

		if (gDisableRegion.uiFlags & MSYS_MOUSE_IN_AREA)
		{
			UpdateAnimatedCursorFrames(gDisableRegion.Cursor);
			SetCurrentCursorFromDatabase(gDisableRegion.Cursor);
		}

		if (gUserTurnRegion.uiFlags & MSYS_MOUSE_IN_AREA)
		{
			UpdateAnimatedCursorFrames(gUserTurnRegion.Cursor);
			SetCurrentCursorFromDatabase(gUserTurnRegion.Cursor);
		}
	}

	if (COUNTERDONE(CURSORFLASHUPDATE))
	{
		RESETCOUNTER(CURSORFLASHUPDATE);

		if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA)
		{
			UpdateFlashingCursorFrames(gViewportRegion.Cursor);
			SetCurrentCursorFromDatabase(gViewportRegion.Cursor);
		}
	}
}


static void DrawMouseText(void);


static void BltJA2CursorData(void)
{
	if (gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA || IsPointerOnTacticalTouchUI())
	{
		DrawMouseText();
	}
}


static ST::string gzLocation;
static ST::string gzIntTileLocation;
static ST::string gzIntTileLocation2;
static ST::string gzHitChance;


void SetHitLocationText(const ST::string& str)
{
	gzLocation = str;
}


void SetIntTileLocationText(const ST::string& str)
{
	gzIntTileLocation = str;
}


void SetIntTileLocation2Text(const ST::string& str)
{
	gzIntTileLocation2 = str;
}


const ST::string& GetIntTileLocationText(void)
{
	return gzIntTileLocation;
}


const ST::string& GetIntTileLocation2Text(void)
{
	return gzIntTileLocation2;
}

void SetChanceToHitText(const ST::string& str)
{
	gzHitChance = str;
}

static void DrawMouseText(void)
{
	static BOOLEAN fShow = FALSE;
	static BOOLEAN fHoldInvalid = TRUE;

	INT16 sX;
	INT16 sY;

	gsMouseSizeYModifier = 0;

	if (!gzLocation.empty())
	{
		// Set dest for gprintf to be different
		SetFontDestBuffer(MOUSE_BUFFER);

		FindFontCenterCoordinates(0, 0, gsCurMouseWidth, gsCurMouseHeight, gzLocation, TINYFONT1, &sX, &sY);
		SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
		MPrint(sX, sY + 12, gzLocation); // Below cursor
		// reset
		SetFontDestBuffer(FRAME_BUFFER);
	}

	if (!gzIntTileLocation.empty())
	{
		// Set dest for gprintf to be different
		SetFontDestBuffer(MOUSE_BUFFER);

		FindFontCenterCoordinates(0, 0, gsCurMouseWidth, gsCurMouseHeight, gzIntTileLocation, TINYFONT1, &sX, &sY);
		SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
		MPrint(sX, sY + 6, gzIntTileLocation);
		// reset
		SetFontDestBuffer(FRAME_BUFFER);
	}

	if (!gzIntTileLocation2.empty())
	{
		// Set dest for gprintf to be different
		SetFontDestBuffer(MOUSE_BUFFER);

		FindFontCenterCoordinates(0, 0, gsCurMouseWidth, gsCurMouseHeight, gzIntTileLocation2, TINYFONT1, &sX, &sY);
		SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
		MPrint(sX, sY - 2, gzIntTileLocation2);
		// reset
		SetFontDestBuffer(FRAME_BUFFER);
	}

	if (!gzHitChance.empty())
	{
		// Set dest for gprintf to be different
		SetFontDestBuffer(MOUSE_BUFFER);
		FindFontCenterCoordinates(0, 0, gsCurMouseWidth, gsCurMouseHeight, gzHitChance, TINYFONT1, &sX, &sY);
		SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
		if(gzLocation.empty())
			MPrint(sX, sY + 12, gzHitChance); // Below cursor
		else
		{
			MPrint(sX, sY + 20, gzHitChance); // Below hit location text
			gsMouseSizeYModifier = 8 + GetFontHeight(TINYFONT1);
		}

		// reset
		SetFontDestBuffer(FRAME_BUFFER);
	}

	//if (gTacticalStatus.uiFlags & INCOMBAT)
	{
		if (gfUIDisplayActionPoints)
		{
			if (gfUIDisplayActionPointsInvalid)
			{
				if (!fHoldInvalid)
				{
					if (COUNTERDONE(INVALID_AP_HOLD))
					{
						RESETCOUNTER(INVALID_AP_HOLD);
						RESETCOUNTER(CURSORFLASH);

						fShow = !fShow;
						fHoldInvalid = !fHoldInvalid;
					}
				}
				else
				{
					if (COUNTERDONE(CURSORFLASH))
					{
						RESETCOUNTER(CURSORFLASH);
						RESETCOUNTER(INVALID_AP_HOLD);

						fShow = !fShow;
						fHoldInvalid = !fHoldInvalid;
					}
				}
			}
			else
			{
				fShow = TRUE;
				fHoldInvalid = FALSE;
			}

			// Set dest for gprintf to be different
			SetFontDestBuffer(MOUSE_BUFFER);

			ST::string pStr = ST::format("{}", gsCurrentActionPoints);

			if (gfUIDisplayActionPointsCenter)
			{
				FindFontCenterCoordinates(0, 0, gsCurMouseWidth, gsCurMouseHeight, pStr, TINYFONT1, &sX, &sY);
			}
			else
			{
				FindFontCenterCoordinates(gUIDisplayActionPointsOffX, gUIDisplayActionPointsOffY, 1, 1, pStr, TINYFONT1, &sX, &sY);
			}

			SetFont(TINYFONT1);

			if (fShow)
			{
				SetFontBackground(FONT_MCOLOR_BLACK);
				SetFontForeground(gfUIDisplayActionPointsInvalid ? 141 : FONT_MCOLOR_WHITE);
				SetFontShadow(DEFAULT_SHADOW);
			}
			else
			{
				gfUIDisplayActionPointsBlack = TRUE;
			}

			if (gfUIDisplayActionPointsBlack)
			{
				SetFontForeground(FONT_MCOLOR_WHITE);
				SetFontShadow(DEFAULT_SHADOW);
			}

			MPrint(sX, sY, pStr);

			SetFontShadow(DEFAULT_SHADOW);

			// reset
			SetFontDestBuffer(FRAME_BUFFER);
		}
	}

#if 0
	if (gpItemPointer != NULL && gpItemPointer->ubNumberOfObjects > 1)
	{
		if (!(gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA))
		{
			SetFontDestBuffer(MOUSE_BUFFER);

			ST::string pStr = ST::format("x{}", gpItemPointer->ubNumberOfObjects);

			FindFontCenterCoordinates(0, 0, gsCurMouseWidth, gsCurMouseHeight, pStr, TINYFONT1, &sX, &sY);

			SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
			MPrint(sX + 10, sY - 10, pStr);

			SetFontDestBuffer(FRAME_BUFFER);
		}
	}
#endif
}


void UpdateAnimatedCursorFrames(UINT32 uiCursorIndex)
{
	uiCursorIndex = ModifyCursorIndex(uiCursorIndex);
	if (uiCursorIndex == VIDEO_NO_CURSOR) return;

	CursorData* pCurData = &CursorDatabase[uiCursorIndex];
	for (UINT32 cnt = 0; cnt < pCurData->usNumComposites; cnt++)
	{
		CursorImage*          pCurImage = &pCurData->Composites[cnt];
		const CursorFileData* CFData    = &CursorFileDatabase[pCurImage->uiFileIndex];

		if (CFData->ubNumberOfFrames != 0)
		{
			pCurImage->uiCurrentFrame++;
			if (pCurImage->uiCurrentFrame == CFData->ubNumberOfFrames)
			{
				pCurImage->uiCurrentFrame = 0;
			}
		}
	}
}


static void UpdateFlashingCursorFrames(UINT32 uiCursorIndex)
{
	uiCursorIndex = ModifyCursorIndex(uiCursorIndex);
	if (uiCursorIndex == VIDEO_NO_CURSOR) return;

	CursorData* pCurData = &CursorDatabase[uiCursorIndex];
	if (pCurData->bFlags & (CURSOR_TO_FLASH | CURSOR_TO_FLASH2))
	{
		pCurData->bFlashIndex = !pCurData->bFlashIndex;

		// Should we play a sound?
		if (pCurData->bFlags & CURSOR_TO_PLAY_SOUND && pCurData->bFlashIndex)
		{
			PlayJA2Sample(TARGET_OUT_OF_RANGE, MIDVOLUME, 1, MIDDLEPAN);
		}
	}
}


void SetCursorSpecialFrame(UINT32 uiCursor, UINT8 ubFrame)
{
	CursorDatabase[uiCursor].bFlashIndex = ubFrame;
}


void SetCursorFlags(UINT32 uiCursor, UINT8 ubFlags)
{
	CursorDatabase[uiCursor].bFlags |= ubFlags;
}


void RemoveCursorFlags(UINT32 uiCursor, UINT8 ubFlags)
{
	CursorDatabase[uiCursor].bFlags &= ~ubFlags;
}
