#include "MapScreen.h"
#include "Animated_ProgressBar.h"
#include "Campaign.h"
#include "Cheats.h"
#include "ContentManager.h"
#include "Creature_Spreading.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "EMail.h"
#include "English.h"
#include "Event_Pump.h"
#include "Explosion_Control.h"
#include "Faces.h"
#include "Fade_Screen.h"
#include "Finances.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Game_Init.h"
#include "GameInstance.h"
#include "GameLoop.h"
#include "GamePolicy.h"
#include "GameRes.h"
#include "GameSettings.h"
#include "HelpScreen.h"
#include "HImage.h"
#include "Interface_Control.h"
#include "Interface_Items.h"
#include "Interface_Panels.h"
#include "Interface_Utils.h"
#include "Items.h"
#include "JAScreens.h"
#include "LaptopSave.h"
#include "Line.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Map_Screen_Interface_TownMine_Info.h"
#include "Meanwhile.h"
#include "Merc_Contract.h"
#include "Merc_Hiring.h"
#include "Message.h"
#include "NewStrings.h"
#include "Options_Screen.h"
#include "Overhead.h"
#include "Player_Command.h"
#include "PopUpBox.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Radar_Screen.h"
#include "Render_Dirty.h"
#include "RenderWorld.h"
#include "SaveLoadScreen.h"
#include "Soldier_Macros.h"
#include "Squads.h"
#include "StrategicMap_Secrets.h"
#include "Strategic_Movement_Costs.h"
#include "Strategic_Pathing.h"
#include "Strategic_Town_Loyalty.h"
#include "Strategic_Turns.h"
#include "Sys_Globals.h"
#include "SysUtil.h"
#include "Tactical_Save.h"
#include "Text.h"
#include "Timer_Control.h"
#include "Town_Militia.h"
#include "Video.h"
#include "VObject.h"
#include "VObject_Blitters.h"
#include "VSurface.h"

#include <string_theory/format>

struct PopUpBox;



#define MAX_SORT_METHODS					6

// Cursors
#define SCREEN_CURSOR CURSOR_NORMAL

// Fonts
#define CHAR_FONT BLOCKFONT2 // COMPFONT
#define ETA_FONT BLOCKFONT2

// Colors
#define FONT_MAP_DKYELLOW 170

#define CHAR_TITLE_FONT_COLOR 6
#define CHAR_TEXT_FONT_COLOR 5

#define STARTING_COLOR_NUM 5

#define MAP_TIME_UNDER_THIS_DISPLAY_AS_HOURS ( 3 * 24 * 60 )


#define DELAY_PER_FLASH_FOR_DEPARTING_PERSONNEL 500
#define GLOW_DELAY 70
#define ASSIGNMENT_DONE_FLASH_TIME 500

#define MINS_TO_FLASH_CONTRACT_TIME (4 * 60)

// Coordinate defines

#define TOWN_INFO_X           (STD_SCREEN_X + 0)
#define TOWN_INFO_Y           (STD_SCREEN_Y + 1)

#define PLAYER_INFO_X         (STD_SCREEN_X + 0)
#define PLAYER_INFO_Y         (STD_SCREEN_Y + 107)

// item description
#define MAP_ITEMDESC_START_X PLAYER_INFO_X
#define MAP_ITEMDESC_START_Y PLAYER_INFO_Y

#define INV_REGION_X PLAYER_INFO_X
#define INV_REGION_Y PLAYER_INFO_Y
#define INV_REGION_WIDTH 261
#define INV_REGION_HEIGHT 359-94
#define INV_BTN_X PLAYER_INFO_X + 217
#define INV_BTN_Y PLAYER_INFO_Y + 210

#define MAP_BG_WIDTH      (640 - 261)

#define MAP_ARMOR_LABEL_X (STD_SCREEN_X + 208)
#define MAP_ARMOR_LABEL_Y (STD_SCREEN_Y + 179)
#define MAP_ARMOR_X       (STD_SCREEN_X + 209)
#define MAP_ARMOR_Y       (STD_SCREEN_Y + 188)
#define MAP_ARMOR_W        28
#define MAP_ARMOR_H        10

#define MAP_WEIGHT_LABEL_X (STD_SCREEN_X + 173)
#define MAP_WEIGHT_LABEL_Y (STD_SCREEN_Y + 256)
#define MAP_WEIGHT_X       (STD_SCREEN_X + 176)
#define MAP_WEIGHT_Y       (STD_SCREEN_Y + 265)
#define MAP_WEIGHT_W        28
#define MAP_WEIGHT_H        10

#define MAP_CAMO_LABEL_X (STD_SCREEN_X + 178)
#define MAP_CAMO_LABEL_Y (STD_SCREEN_Y + 283)
#define MAP_CAMO_X       (STD_SCREEN_X + 176)
#define MAP_CAMO_Y       (STD_SCREEN_Y + 292)
#define MAP_CAMO_W        28
#define MAP_CAMO_H        10

#define MAP_INV_STATS_TITLE_FONT_COLOR 6

#define PLAYER_INFO_FACE_START_X    (STD_SCREEN_X + 9)
#define PLAYER_INFO_FACE_START_Y    (STD_SCREEN_Y + 17)
#define PLAYER_INFO_FACE_END_X			(STD_SCREEN_X + 60)
#define PLAYER_INFO_FACE_END_Y			(STD_SCREEN_Y + 76)

#define PLAYER_INFO_HAND_START_X    (STD_SCREEN_X + 4)
#define PLAYER_INFO_HAND_START_Y    (STD_SCREEN_Y + 81)
#define PLAYER_INFO_HAND_END_X      (STD_SCREEN_X + 62)
#define PLAYER_INFO_HAND_END_Y      (STD_SCREEN_Y + 103)

#define INV_BODY_X (UINT16)(STD_SCREEN_X + 71)
#define INV_BODY_Y (UINT16)(STD_SCREEN_Y + 116)

//Text offsets
#define Y_OFFSET 2


// char stat positions
#define STR_X (STD_SCREEN_X + 112)
#define STR_Y (STD_SCREEN_Y + 42)
#define DEX_X STR_X
#define DEX_Y (STD_SCREEN_Y + 32)
#define AGL_X STR_X
#define AGL_Y (STD_SCREEN_Y + 22)
#define LDR_X STR_X
#define LDR_Y (STD_SCREEN_Y + 52)
#define WIS_X STR_X
#define WIS_Y (STD_SCREEN_Y + 62)
#define LVL_X (STD_SCREEN_X + 159)
#define LVL_Y AGL_Y
#define MRK_X LVL_X
#define MRK_Y DEX_Y
#define EXP_X LVL_X
#define EXP_Y STR_Y
#define MEC_X LVL_X
#define MEC_Y LDR_Y
#define MED_X LVL_X
#define MED_Y WIS_Y

#define STAT_WID 15
#define STAT_HEI GetFontHeight(CHAR_FONT)

#define PIC_NAME_X (STD_SCREEN_X + 8)
#define PIC_NAME_Y (STD_SCREEN_Y + 66 + 3)
#define PIC_NAME_WID (STD_SCREEN_X + 60 - PIC_NAME_X)
#define PIC_NAME_HEI (STD_SCREEN_Y + 75 - PIC_NAME_Y)
#define CHAR_NAME_X (STD_SCREEN_X + 14)
#define CHAR_NAME_Y (STD_SCREEN_Y + 2 + 3)
#define CHAR_NAME_WID (STD_SCREEN_X + 164 - CHAR_NAME_X)
#define CHAR_NAME_HEI (STD_SCREEN_Y + 11 - CHAR_NAME_Y)
#define CHAR_TIME_REMAINING_X (STD_SCREEN_X + 207)
#define CHAR_TIME_REMAINING_Y (STD_SCREEN_Y + 65)
#define CHAR_TIME_REMAINING_WID (STD_SCREEN_X + 258 - CHAR_TIME_REMAINING_X)
#define CHAR_TIME_REMAINING_HEI GetFontHeight(CHAR_FONT)
#define CHAR_SALARY_X					CHAR_TIME_REMAINING_X
#define CHAR_SALARY_Y					(STD_SCREEN_Y + 79)
#define CHAR_SALARY_WID					CHAR_TIME_REMAINING_WID - 8		// for right justify
#define CHAR_SALARY_HEI					CHAR_TIME_REMAINING_HEI
#define CHAR_MEDICAL_X					CHAR_TIME_REMAINING_X
#define CHAR_MEDICAL_Y					(STD_SCREEN_Y + 93)
#define CHAR_MEDICAL_WID				CHAR_TIME_REMAINING_WID - 8		// for right justify
#define CHAR_MEDICAL_HEI				CHAR_TIME_REMAINING_HEI
#define CHAR_ASSIGN_X (STD_SCREEN_X + 182)
#define CHAR_ASSIGN1_Y (STD_SCREEN_Y + 18)
#define CHAR_ASSIGN2_Y (STD_SCREEN_Y + 31)
#define CHAR_ASSIGN_WID 257 - 178
#define CHAR_ASSIGN_HEI 39 - 29
#define CHAR_HP_X (STD_SCREEN_X + 133)
#define CHAR_HP_Y (STD_SCREEN_Y + 77 + 3)
#define CHAR_HP_WID  (STD_SCREEN_X + 175 - CHAR_HP_X)
#define CHAR_HP_HEI  (STD_SCREEN_Y + 90 - CHAR_HP_Y)
#define CHAR_MORALE_X (STD_SCREEN_X + 133)
#define CHAR_MORALE_Y (STD_SCREEN_Y + 91 + 3)
#define CHAR_MORALE_WID (STD_SCREEN_X + 175 - CHAR_MORALE_X)
#define CHAR_MORALE_HEI (STD_SCREEN_Y + 101 - CHAR_MORALE_Y)

#define SOLDIER_PIC_X (STD_SCREEN_X + 9)
#define SOLDIER_PIC_Y (STD_SCREEN_Y + 20)
#define SOLDIER_HAND_X (STD_SCREEN_X + 6)
#define SOLDIER_HAND_Y (STD_SCREEN_Y + 81)

#define CLOCK_X (STD_SCREEN_X + 554)
#define CLOCK_Y (STD_SCREEN_Y + 459)


#define RGB_WHITE	( FROMRGB( 255, 255, 255 ) )
#define RGB_YELLOW	( FROMRGB( 255, 255,   0 ) )
#define RGB_NEAR_BLACK	( FROMRGB(   0,   0,   1 ) )


// ARM: NOTE that these map "events" are never actually saved in a player's game in any way
enum MapEvent
{
	MAP_EVENT_NONE,
	MAP_EVENT_CLICK_SECTOR,
	MAP_EVENT_PLOT_PATH,
	MAP_EVENT_CANCEL_PATH,
};


static inline UINT16 GlowColor(UINT i)
{
	Assert(i <= 10);
	return Get16BPPColor(FROMRGB(25 * i, 0, 0));
}


static const SGPPoint gMapSortButtons[MAX_SORT_METHODS] =
{
	{  12, 125 },
	{  68, 125 },
	{ 124, 125 },
	{ 148, 125 },
	{ 185, 125 },
	{ 223, 125 }
};


// GLOBAL VARIABLES (OURS)


static BOOLEAN fFlashAssignDone = FALSE;
BOOLEAN	fInMapMode = FALSE;
BOOLEAN fMapPanelDirty=TRUE;
BOOLEAN fTeamPanelDirty = TRUE;
BOOLEAN fCharacterInfoPanelDirty = TRUE;
BOOLEAN fReDrawFace=FALSE;
BOOLEAN fShowInventoryFlag = FALSE;
BOOLEAN fMapInventoryItem=FALSE;
BOOLEAN fShowDescriptionFlag=FALSE;

static BOOLEAN gfHotKeyEnterSector   = FALSE;
static BOOLEAN fOneFrame             = FALSE;
static BOOLEAN fShowItemHighLight    = FALSE;
static BOOLEAN fJustFinishedPlotting = FALSE;

// for the flashing of the contract departure time...for when mercs are leaving in an hour or less
static BOOLEAN fFlashContractFlag = FALSE;

static BOOLEAN fShowTrashCanHighLight = FALSE;

// the flags for display of pop up boxes/menus
static BOOLEAN fEndPlotting = FALSE;

BOOLEAN gfInConfirmMapMoveMode = FALSE;
BOOLEAN gfInChangeArrivalSectorMode = FALSE;

// redraw character list
BOOLEAN fDrawCharacterList = TRUE;

// was the cursor set to the checkmark?
static BOOLEAN fCheckCursorWasSet = FALSE;

static BOOLEAN fEndShowInventoryFlag = FALSE;

// draw the temp path
static BOOLEAN fDrawTempPath = TRUE;

static BOOLEAN gfGlowTimerExpired = FALSE;

BOOLEAN gfSkyriderEmptyHelpGiven = FALSE;

static BOOLEAN gfRequestGiveSkyriderNewDestination = FALSE;

static BOOLEAN gfFirstMapscreenFrame = FALSE;

static BOOLEAN gfMapPanelWasRedrawn = FALSE;


// currently selected character's list index
INT8 bSelectedInfoChar = -1;

static GUIButtonRef giMapSortButton[MAX_SORT_METHODS];

GUIButtonRef giCharInfoButton[2];

GUIButtonRef giMapInvDoneButton;

GUIButtonRef giMapContractButton;

INT32 giSortStateForMapScreenList = 0;

UINT32 guiCommonGlowBaseTime = 0;
UINT32 guiFlashAssignBaseTime = 0;
UINT32 guiFlashContractBaseTime = 0;
UINT32 guiFlashCursorBaseTime = 0;
UINT32 guiPotCharPathBaseTime = 0;

static SGPVObject* guiCHARLIST;
static SGPVObject* guiCHARINFO;
static SGPVObject* guiSleepIcon;
static SGPVObject* guiMAPINV;
static SGPVObject* guiULICONS;
static SGPVObject* guiNewMailIcons;


// misc mouse regions
static MOUSE_REGION gCharInfoFaceRegion;
static MOUSE_REGION gCharInfoHandRegion;
static MOUSE_REGION gMPanelRegion;
static MOUSE_REGION gMapViewRegion;
static MOUSE_REGION gMapScreenMaskRegion;
static MOUSE_REGION gTrashCanRegion;

// mouse regions for team info panel
struct CharacterRegions
{
	MOUSE_REGION name;
	MOUSE_REGION assignment;
	MOUSE_REGION sleep;
	MOUSE_REGION location;
	MOUSE_REGION destination;
	MOUSE_REGION contract;
};

static CharacterRegions g_character_regions[MAX_CHARACTER_COUNT];


static PathSt* g_prev_path;


static bool fLockOutMapScreenInterface = false;


// GLOBAL VARIABLES (EXTERNAL)


extern BOOLEAN fDeletedNode;
extern BOOLEAN gfStartedFromMapScreen;


extern PathSt* pTempCharacterPath;
extern PathSt* pTempHelicopterPath;

extern BOOLEAN gfAutoAIAware;

extern OBJECTTYPE	*gpItemDescObject;


// externs for highlighting of ammo/weapons
extern UINT32 guiMouseOverItemTime;
extern BOOLEAN gfCheckForMouseOverItem;
extern INT8 gbCheckForMouseOverItemPos;

//Autoresolve sets this variable which defaults to -1 when not needed.
extern INT16 gsEnemyGainedControlOfSectorID;
extern INT16 gsCiviliansEatenByMonsters;

extern BOOLEAN			gfFadeOutDone;

extern BOOLEAN gfMilitiaPopupCreated;

void CancelMapUIMessage( void );



// the tries to select a mapscreen character by his soldier ID
void SetInfoChar(SOLDIERTYPE const* const s)
{
	for (INT8 i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		if (gCharactersList[i].merc != s) continue;
		ChangeSelectedInfoChar(i, TRUE);
		break;
	}
}

static void ContractListRegionBoxGlow(UINT16 usCount)
{
	static INT32 iColorNum =10;
	static BOOLEAN fDelta=FALSE;
	INT16 usY = 0;
	INT16 sYAdd = 0;


	// if not glowing right now, leave
	if (fShowInventoryFlag)
	{
		iColorNum = 0;
		fDelta = TRUE;
		return;
	}

	// if not ready to change glow phase yet, leave
	if ( !gfGlowTimerExpired )
		return;


	// change direction of glow?
	if((iColorNum==0)||(iColorNum==10))
	{
		fDelta=!fDelta;
	}

	// increment color
	if(!fDelta)
		iColorNum++;
	else
		iColorNum--;


	if( usCount >= FIRST_VEHICLE )
	{
		sYAdd = 6;
	}
	else
	{
		sYAdd = 0;
	}

	// y start position of box
	usY=(Y_OFFSET*usCount-1)+(Y_START+(usCount*Y_SIZE) + sYAdd );

	// glow contract box
	UINT16 usColor = GlowColor(iColorNum);
	SGPVSurface::Lock l(FRAME_BUFFER);
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	RectangleDraw(TRUE, TIME_REMAINING_X, usY, TIME_REMAINING_X + TIME_REMAINING_WIDTH, usY + GetFontHeight(MAP_SCREEN_FONT) + 2, usColor, l.Buffer<UINT16>());
	InvalidateRegion(TIME_REMAINING_X - 1, usY, TIME_REMAINING_X + TIME_REMAINING_WIDTH + 1, usY + GetFontHeight( MAP_SCREEN_FONT ) + 3 );
}


static void RenderHandPosItem();


static void GlowItem(void)
{
	static INT32 iColorNum =10;
	static BOOLEAN fDelta=FALSE;
	static BOOLEAN fOldItemGlow = FALSE;

	// not glowing right now, leave
	if (!fShowItemHighLight)
	{
		iColorNum =0;
		fDelta = TRUE;

		if (fOldItemGlow)
		{
			RestoreExternBackgroundRect( STD_SCREEN_X + 3, STD_SCREEN_Y + 80, ( UINT16 )( 65 - 3 ), ( UINT16 )( 105 - 80 ) );
		}

		fOldItemGlow = FALSE;
		return;
	}

	// if not ready to change glow phase yet, leave
	if ( !gfGlowTimerExpired )
		return;


	fOldItemGlow = TRUE;

	// change direction of glow?
	if((iColorNum==0)||(iColorNum==10))
	{
		fDelta=!fDelta;
	}

	// increment color
	if(!fDelta)
		iColorNum++;
	else
		iColorNum--;

	// restore background
	if((iColorNum==0)||(iColorNum==1))
	{
		RestoreExternBackgroundRect( STD_SCREEN_X + 3, STD_SCREEN_Y + 80, ( UINT16 )( 65 - 3 ), ( UINT16 )( 105 - 80 ) );
		RenderHandPosItem();
	}

	// glow contract box
	UINT16 usColor = GlowColor(iColorNum);
	SGPVSurface::Lock l(FRAME_BUFFER);
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	RectangleDraw(TRUE, STD_SCREEN_X + 3, STD_SCREEN_Y + 80, STD_SCREEN_X + 64, STD_SCREEN_Y + 104, usColor, l.Buffer<UINT16>());
	InvalidateRegion( STD_SCREEN_X + 3, STD_SCREEN_Y + 80, STD_SCREEN_X + 65, STD_SCREEN_Y + 105 );
}


static void GlowTrashCan(void)
{
	static INT32 iColorNum =10;
	static BOOLEAN fOldTrashCanGlow = FALSE;

	if (!fShowInventoryFlag) fShowTrashCanHighLight = FALSE;

	// not glowing right now, leave
	if (!fShowTrashCanHighLight)
	{
		iColorNum =0;

		if (fOldTrashCanGlow)
		{
			RestoreExternBackgroundRect( TRASH_CAN_X, TRASH_CAN_Y, ( UINT16 )( TRASH_CAN_WIDTH + 2 ), ( UINT16 )( TRASH_CAN_HEIGHT + 2 ) );
		}

		fOldTrashCanGlow = FALSE;
		return;
	}

	// if not ready to change glow phase yet, leave
	if ( !gfGlowTimerExpired )
		return;


	fOldTrashCanGlow = TRUE;

	// glow contract box
	UINT16 usColor = GlowColor(iColorNum);
	{ SGPVSurface::Lock l(FRAME_BUFFER);
		SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		RectangleDraw(TRUE, TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH, TRASH_CAN_Y + TRASH_CAN_HEIGHT, usColor, l.Buffer<UINT16>());
		InvalidateRegion( TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH + 1, TRASH_CAN_Y + TRASH_CAN_HEIGHT + 1 );
	}

	// restore background
	if((iColorNum==0)||(iColorNum==1))
		RestoreExternBackgroundRect( TRASH_CAN_X, TRASH_CAN_Y, ( UINT16 )( TRASH_CAN_WIDTH + 2 ), ( UINT16 )( TRASH_CAN_HEIGHT + 2 ) );
}


void DrawFace(void)
{
	static const SOLDIERTYPE* old_merc = NULL;

	// draws the face of the currently selected merc, being displayed int he upper left hand corner

	const SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	if( pSoldier == NULL )
	{
		return;
	}

	if (pSoldier == old_merc && !fReDrawFace)
	{
		// are the same, return
		return;
	}

	// get old id value
	old_merc = pSoldier;

	// reset redraw of face
	fReDrawFace = FALSE;

	// render their face
	RenderSoldierFace(*pSoldier, SOLDIER_PIC_X, SOLDIER_PIC_Y);
}


// Render the inventory item in char's main hand
static void RenderHandPosItem()
{
	/* ARM: if already in the inventory panel, don't show the item again here,
		* seeing it twice is confusing */
	if (fShowInventoryFlag) return;

	SOLDIERTYPE const* const s = GetSelectedInfoChar();
	if (!s || s->bLife == 0) return;

	INVRenderItem(guiSAVEBUFFER, s, s->inv[HANDPOS], SOLDIER_HAND_X, SOLDIER_HAND_Y, 58, 23, DIRTYLEVEL2, 0, SGP_TRANSPARENT);
}


static void RenderIconsForUpperLeftCornerPiece(const SOLDIERTYPE* const s)
{
	// if merc is an AIM merc
	if (s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
	{
		// finite contract length icon
		BltVideoObject(guiSAVEBUFFER, guiULICONS, 0, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y);
	}

	// if merc has life insurance
	if (s->usLifeInsurance > 0)
	{
		// draw life insurance icon
		BltVideoObject(guiSAVEBUFFER, guiULICONS, 2, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + CHAR_ICON_SPACING);
	}

	// if merc has a medical deposit
	if (s->usMedicalDeposit > 0)
	{
		// draw medical deposit icon
		BltVideoObject(guiSAVEBUFFER, guiULICONS, 1, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + 2 * CHAR_ICON_SPACING);
	}
}

static void PrintStat(UINT32 change_time, UINT16 const stat_gone_up_bit, INT8 stat_val, INT16 x, INT16 y, INT16 w, INT32 progress)
{
	UINT8 const colour =
		change_time == 0 ||
		GetJA2Clock() >= CHANGE_STAT_RECENTLY_DURATION + change_time ? CHAR_TEXT_FONT_COLOR :
		stat_gone_up_bit != 0                                        ? FONT_LTGREEN         :
		FONT_RED;

	SetFontForeground(colour);

	ST::string str = ST::format("{3d}", stat_val);
	if (gamepolicy(gui_extras))
	{
		ProgressBarBackgroundRect(x + 1, y - 2, w * progress / 100, 10, 0x514A05, progress);
	}

	DrawStringRight(str, x, y, STAT_WID, STAT_HEI, CHAR_FONT);
}

// Draw attributes & skills for given soldier
static void DrawCharStats(SOLDIERTYPE const& s)
{
	SetFontAttributes(CHAR_FONT, CHAR_TEXT_FONT_COLOR);

	UINT16 const up = s.usValueGoneUp;
	MERCPROFILESTRUCT& p = GetProfile(s.ubProfile);

	PrintStat(s.uiChangeLevelTime,        up & LVL_INCREASE,      s.bExpLevel,     LVL_X,   LVL_Y,   STAT_WID,  (100 * p.sExpLevelGain) / (350 * p.bExpLevel));
	PrintStat(s.uiChangeAgilityTime,      up & AGIL_INCREASE,     s.bAgility,      AGL_X,   AGL_Y,   STAT_WID,   2 * p.sAgilityGain);
	PrintStat(s.uiChangeDexterityTime,    up & DEX_INCREASE,      s.bDexterity,    DEX_X,   DEX_Y,   STAT_WID,   2 * p.sDexterityGain);
	PrintStat(s.uiChangeStrengthTime,     up & STRENGTH_INCREASE, s.bStrength,     STR_X,   STR_Y,   STAT_WID,   2 * p.sStrengthGain);
	PrintStat(s.uiChangeLeadershipTime,   up & LDR_INCREASE,      s.bLeadership,   LDR_X,   LDR_Y,   STAT_WID,   2 * p.sLeadershipGain);
	PrintStat(s.uiChangeWisdomTime,       up & WIS_INCREASE,      s.bWisdom,       WIS_X,   WIS_Y,   STAT_WID,   2 * p.sWisdomGain);
	PrintStat(s.uiChangeMarksmanshipTime, up & MRK_INCREASE,      s.bMarksmanship, MRK_X,   MRK_Y,   STAT_WID,   4 * p.sMarksmanshipGain);
	PrintStat(s.uiChangeExplosivesTime,   up & EXP_INCREASE,      s.bExplosive,    EXP_X,   EXP_Y,   STAT_WID,   4 * p.sExplosivesGain);
	PrintStat(s.uiChangeMechanicalTime,   up & MECH_INCREASE,     s.bMechanical,   MEC_X,   MEC_Y,   STAT_WID,   4 * p.sMechanicGain);
	PrintStat(s.uiChangeMedicalTime,      up & MED_INCREASE,      s.bMedical,      MED_X,   MED_Y,   STAT_WID,   4 * p.sMedicalGain);
}

static void DrawString(const ST::string& str, UINT16 uiX, UINT16 uiY, SGPFont);
static void DrawStringCentered(const ST::string& str, UINT16 x, UINT16 y, UINT16 w, UINT16 h, SGPFont);

static void DrawCharHealth(SOLDIERTYPE const& s)
{
	if (s.bAssignment != ASSIGNMENT_POW)
	{
		INT8 const life     = s.bLife;
		INT8 const life_max = s.bLifeMax;

		/* Find starting X coordinate by centering all 3 substrings together, then
			* print them separately (different colors) */
		ST::string buf = ST::format("{}/{}", life, life_max);
		INT16 x;
		INT16 y;
		FindFontCenterCoordinates(CHAR_HP_X, CHAR_HP_Y, CHAR_HP_WID, CHAR_HP_HEI, buf, CHAR_FONT, &x, &y);

		// How is character's life?
		UINT32 const health_percent = life_max > 0 ? 100 * life / life_max : 0;
		UINT8  const cur_colour     =
			health_percent ==  0 ? FONT_METALGRAY : // Dead
			health_percent <  25 ? FONT_RED       : // Very bad
			health_percent <  50 ? FONT_YELLOW    : // Not good
			CHAR_TEXT_FONT_COLOR;                   // Ok
		SetFontForeground(cur_colour);

		if (gamepolicy(gui_extras))
		{
			INT32 progress = 2 * GetProfile(s.ubProfile).sLifeGain;
			ProgressBarBackgroundRect(CHAR_HP_X + 1, CHAR_HP_Y - 2, CHAR_HP_WID * progress / 100, 10, 0x514A05, progress);
		}

		// Current life
		buf = ST::format("{}", life);
		DrawString(buf, x, CHAR_HP_Y, CHAR_FONT);
		x += StringPixLength(buf, CHAR_FONT);

		// Slash
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
		ST::string slash = "/";
		DrawString(slash, x, CHAR_HP_Y, CHAR_FONT);
		x += StringPixLength(slash, CHAR_FONT);

		bool  const recent_change =
			GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + s.uiChangeHealthTime &&
			s.uiChangeHealthTime != 0;
		UINT8 const max_colour =
			!recent_change                    ? CHAR_TEXT_FONT_COLOR :
			s.usValueGoneUp & HEALTH_INCREASE ? FONT_LTGREEN         :
			FONT_RED;
		SetFontForeground(max_colour);

		// Maximum life
		buf = ST::format("{}", life_max);
		DrawString(buf, x, CHAR_HP_Y, CHAR_FONT);
	}
	else
	{ // POW - health unknown
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
		DrawStringCentered(pPOWStrings[1], CHAR_HP_X, CHAR_HP_Y, CHAR_HP_WID, CHAR_HP_HEI, CHAR_FONT);
	}
}


static ST::string ConvertMinTimeToETADayHourMinString(UINT32 uiTimeInMin);


// "character" refers to hired people AND vehicles
static void DrawCharacterInfo(SOLDIERTYPE const& s)
{
	ST::string buf;

	ProfileID const pid = s.ubProfile;
	if (pid == NO_PROFILE) return;
	MERCPROFILESTRUCT const& p = GetProfile(pid);

	// Draw particular info about a character that are neither attributes nor skills
	SetFontAttributes(CHAR_FONT, CHAR_TEXT_FONT_COLOR);

	ST::string nickname; // Nickname (beneath picture)
	ST::string name;     // Full name (top box)
	if (s.uiStatusFlags & SOLDIER_VEHICLE)
	{
		VEHICLETYPE const& v = GetVehicle(s.bVehicleID);
		nickname = pShortVehicleStrings[v.ubVehicleType];
		name     = pVehicleStrings[     v.ubVehicleType];
	}
	else
	{
		nickname = p.zNickname;
		name     = p.zName;
	}
	DrawStringCentered(nickname, PIC_NAME_X,  PIC_NAME_Y,  PIC_NAME_WID,  PIC_NAME_HEI,  CHAR_FONT);
	DrawStringCentered(name,     CHAR_NAME_X, CHAR_NAME_Y, CHAR_NAME_WID, CHAR_NAME_HEI, CHAR_FONT);

	ST::string assignment =
		s.bAssignment == VEHICLE ? pShortVehicleStrings[GetVehicle(s.iVehicleId).ubVehicleType] : // Show vehicle type
		pAssignmentStrings[s.bAssignment];
	DrawStringCentered(assignment, CHAR_ASSIGN_X, CHAR_ASSIGN1_Y, CHAR_ASSIGN_WID, CHAR_ASSIGN_HEI, CHAR_FONT);

	// Second assignment line
	ST::string assignment2;
	switch (s.bAssignment)
	{
		case TRAIN_SELF:
		case TRAIN_TEAMMATE:
		case TRAIN_BY_OTHER:
			assignment2 = pAttributeMenuStrings[s.bTrainStat];
			break;

		case TRAIN_TOWN:
			assignment2 = GCM->getTownName(GetTownIdForSector(s.sSector.AsByte()));
			break;

		case REPAIR:
			assignment2 =
				s.fFixingRobot                ? pRepairStrings[3] : // Robot
				s.bVehicleUnderRepairID != -1 ? pShortVehicleStrings[GetVehicle(s.bVehicleUnderRepairID).ubVehicleType] : // Vehicle
				pRepairStrings[0]; // Items
			break;

		case IN_TRANSIT:
			// Show ETA
			buf = ConvertMinTimeToETADayHourMinString(s.uiTimeSoldierWillArrive);
			assignment2 = buf;
			break;

		default:
		{
			GROUP const* const g = GetSoldierGroup(s);
			if (g && PlayerGroupInMotion(g))
			{ // Show ETA
				UINT32 const arrival_time = GetWorldTotalMin() + CalculateTravelTimeOfGroup(g);
				buf = ConvertMinTimeToETADayHourMinString(arrival_time);
			}
			else
			{ // Show location
				buf = GetMapscreenMercLocationString(s);
			}
			assignment2 = buf;
			break;
		}
	}
	DrawStringCentered(assignment2, CHAR_ASSIGN_X, CHAR_ASSIGN2_Y, CHAR_ASSIGN_WID, CHAR_ASSIGN_HEI, CHAR_FONT);

	DrawCharHealth(s);

	// If a vehicle or robot, we're done - the remainder applies only to people
	if (IsMechanical(s)) return;

	DrawCharStats(s);

	// Remaining contract length
	ST::string contract = gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION];
	if (s.bLife > 0)
	{
		if (s.ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC || s.ubProfile == SLAY)
		{
			// Amount of time left on contract
			INT32 time_remaining = s.iEndofContractTime - GetWorldTotalMin();

			if (s.bAssignment == IN_TRANSIT &&
					time_remaining > (INT32)(s.iTotalContractLength * NUM_MIN_IN_DAY))
			{ /* If the merc is in transit and if the time left on the contract is
				* greater than the contract time */
				time_remaining = s.iTotalContractLength * NUM_MIN_IN_DAY;
			}

			if (time_remaining >= 24 * 60)
			{ // More than a day, display in green
				// Calculate the exact time left on the contract (e.g. 1.8 days)
				float          const time_left = time_remaining / (60 * 24.0);
				ST::string days      = gpStrategicString[STR_PB_DAYS_ABBREVIATION];
				buf = ST::format("{.1f}{}/{}{}", time_left, days, s.iTotalContractLength, days);
			}
			else
			{ // Less than a day, display hours left in red
				if (time_remaining > 5) time_remaining += 59;
				time_remaining /= 60;
				ST::string hours = gpStrategicString[STR_PB_HOURS_ABBREVIATION];
				ST::string days  = gpStrategicString[STR_PB_DAYS_ABBREVIATION];
				buf = ST::format("{}{}/{}{}", time_remaining, hours, s.iTotalContractLength, days);
			}
			contract = buf;
		}
		else if (s.ubWhatKindOfMercAmI == MERC_TYPE__MERC)
		{
			INT32          const been_hired_for = GetWorldTotalMin() / NUM_MIN_IN_DAY - s.iStartContractTime;
			ST::string days = gpStrategicString[STR_PB_DAYS_ABBREVIATION];
			buf = ST::format("{}{}/{}{}", p.iMercMercContractLength, days, been_hired_for, days);
			contract = buf;
		}
	}
	SetFontForeground(CHAR_TEXT_FONT_COLOR);
	DrawStringCentered(contract, CHAR_TIME_REMAINING_X, CHAR_TIME_REMAINING_Y, CHAR_TIME_REMAINING_WID, CHAR_TIME_REMAINING_HEI, CHAR_FONT);

	// Salary
	INT32 daily_cost;
	if (s.ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
	{ // Daily rate
		switch (s.bTypeOfLastContract)
		{
			case CONTRACT_EXTEND_2_WEEK: daily_cost = p.uiBiWeeklySalary / 14; break;
			case CONTRACT_EXTEND_1_WEEK: daily_cost = p.uiWeeklySalary   /  7; break;
			default:                     daily_cost = p.sSalary;               break;
		}
	}
	else
	{
		daily_cost = p.sSalary;
	}
	buf = SPrintMoney(daily_cost);
	DrawStringRight(buf, CHAR_SALARY_X, CHAR_SALARY_Y, CHAR_SALARY_WID, CHAR_SALARY_HEI, CHAR_FONT);

	// Medical deposit
	if (p.sMedicalDepositAmount > 0)
	{
		buf = SPrintMoney(p.sMedicalDepositAmount);
		DrawStringRight(buf, CHAR_MEDICAL_X, CHAR_MEDICAL_Y, CHAR_MEDICAL_WID, CHAR_MEDICAL_HEI, CHAR_FONT);
	}

	ST::string morale =
		s.bAssignment == ASSIGNMENT_POW ? pPOWStrings[1] : // POW - morale unknown
		s.bLife == 0                    ? ST::null :
		GetMoraleString(s);
	DrawStringCentered(morale, CHAR_MORALE_X, CHAR_MORALE_Y, CHAR_MORALE_WID, CHAR_MORALE_HEI, CHAR_FONT);
}


// this character is in transit has an item picked up
static bool CharacterIsInTransitAndHasItemPickedUp(SOLDIERTYPE const* const s)
{
	return s->bAssignment == IN_TRANSIT && fMapInventoryItem;
}


static void DisplayCharacterInfo(void)
{
	const SOLDIERTYPE* const s = GetSelectedInfoChar();
	if (s == NULL) return;

	SetFontDestBuffer(guiSAVEBUFFER);

	// draw character info and face
	DrawCharacterInfo(*s);

	RenderHandPosItem();

	SetFontDestBuffer(FRAME_BUFFER);

	RenderIconsForUpperLeftCornerPiece(s);

	// mark all pop ups as dirty
	MarkAllBoxesAsAltered();
}



INT32 GetPathTravelTimeDuringPlotting(PathSt* pPath)
{
	INT32 iTravelTime = 0;
	WAYPOINT pCurrent;
	WAYPOINT pNext;
	GROUP *pGroup;
	UINT8 ubGroupId = 0;
	BOOLEAN fSkipFirstNode = FALSE;


	if (bSelectedDestChar == -1 && !fPlotForHelicopter)
	{
		return( 0 );
	}

	if (!fTempPathAlreadyDrawn)
	{
		return( 0 );
	}

	if (pPath == NULL) return 0;
	Assert(pPath->pPrev == NULL);

	if (!fPlotForHelicopter)
	{
		// plotting for a character...
		SOLDIERTYPE* const s = gCharactersList[bSelectedDestChar].merc;
		if (s->bAssignment == VEHICLE)
		{
			ubGroupId = pVehicleList[s->iVehicleId].ubMovementGroup;
			pGroup = GetGroup( ubGroupId );

			if( pGroup == NULL )
			{
				SetUpMvtGroupForVehicle(s);

				// get vehicle id
				ubGroupId = pVehicleList[s->iVehicleId].ubMovementGroup;
				pGroup = GetGroup( ubGroupId );
			}
		}
		else if (s->uiStatusFlags & SOLDIER_VEHICLE)
		{
			ubGroupId = pVehicleList[s->bVehicleID].ubMovementGroup;
			pGroup = GetGroup( ubGroupId );

			if( pGroup == NULL )
			{
				SetUpMvtGroupForVehicle(s);

				// get vehicle id
				ubGroupId = pVehicleList[s->bVehicleID].ubMovementGroup;
				pGroup = GetGroup( ubGroupId );
			}
		}
		else
		{
			ubGroupId = s->ubGroupID;
			pGroup = GetGroup( ( UINT8 )( ubGroupId ) );
		}
	}
	else
	{
		ubGroupId = GetHelicopter().ubMovementGroup;
		pGroup = GetGroup( ubGroupId );
	}

	Assert(pGroup);


	// if between sectors
	if ( pGroup->fBetweenSectors )
	{
		// arrival time should always be legal!
		Assert( pGroup->uiArrivalTime >= GetWorldTotalMin( ) );

		// start with time to finish arriving in any traversal already in progress
		iTravelTime = pGroup->uiArrivalTime - GetWorldTotalMin();
		fSkipFirstNode = TRUE;
	}
	else
	{
		iTravelTime = 0;
	}

	while( pPath->pNext )
	{
		if ( !fSkipFirstNode )
		{
			// grab the current location
			pCurrent.sSector = SGPSector::FromStrategicIndex(pPath->uiSectorId);

			// grab the next location
			pNext.sSector = SGPSector::FromStrategicIndex(pPath->pNext->uiSectorId);

			iTravelTime += FindTravelTimeBetweenWaypoints( &pCurrent, &pNext, pGroup );
		}
		else
		{
			fSkipFirstNode = FALSE;
		}

		pPath = pPath->pNext;
	}

	return( iTravelTime );
}


static INT32 GetGroundTravelTimeOfSoldier(const SOLDIERTYPE* s);


static void DisplayGroundEta(void)
{
	if (fPlotForHelicopter) return;

	if( bSelectedDestChar == -1 )
	{
		return;
	}

	const SOLDIERTYPE* const s = gCharactersList[bSelectedDestChar].merc;
	if (s == NULL) return;

	const UINT32 iTotalTime = GetGroundTravelTimeOfSoldier(s);

	// now display it
	SetFontAttributes(ETA_FONT, FONT_LTGREEN);
	MPrint(CLOCK_ETA_X, CLOCK_Y_START, pEtaString);

	// if less than one day
	if (iTotalTime < 60 * 24)
	{
		// show hours and minutes
		UINT Minutes = iTotalTime % 60;
		UINT Hours   = iTotalTime / 60;
		MPrint(CLOCK_MIN_X_START  - 5, CLOCK_Y_START, ST::format("{2d}{}", Minutes, gsTimeStrings[1]));
		MPrint(CLOCK_HOUR_X_START - 8, CLOCK_Y_START, ST::format("{2d}{}", Hours,   gsTimeStrings[0]));
	}
	else
	{
		// show days and hours
		UINT Hours = iTotalTime / 60 % 24;
		UINT Days  = iTotalTime / (60 * 24);
		MPrint(CLOCK_MIN_X_START  - 5, CLOCK_Y_START, ST::format("{2d}{}", Hours, gsTimeStrings[0]));
		MPrint(CLOCK_HOUR_X_START - 9, CLOCK_Y_START, ST::format("{2d}{}", Days,  gsTimeStrings[3]));
	}
}


struct HighLightState
{
	INT32 colour_idx;
	bool  delta;
	INT32 old_highlight;
};


static void HighLightSelection(HighLightState& state, INT32 const line, UINT16 const x, UINT16 const w, BOOLEAN (& predicate)(INT16))
{
	// is this a valid line?
	if (line == -1 || fShowInventoryFlag)
	{
		state.old_highlight = MAX_CHARACTER_COUNT + 1;
		return;
	}

	// if not ready to change glow phase yet, leave
	if (!gfGlowTimerExpired) return;

	// check if we have moved lines, if so, reset
	if (state.old_highlight != line)
	{
		state.old_highlight = line;
		state.colour_idx    = STARTING_COLOR_NUM;
		state.delta         = false;
	}

	if (state.colour_idx == 0 || state.colour_idx == 10) state.delta = !state.delta;
	state.colour_idx += state.delta ? -1 : +1;

	SGPVSurface::Lock l(FRAME_BUFFER);
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	UINT16* const pDestBuf = l.Buffer<UINT16>();

	UINT16 const colour = GlowColor(state.colour_idx);
	INT32  const h      = Y_SIZE + Y_OFFSET;
	for (INT16 i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		if (!predicate(i)) continue;

		UINT16 y = Y_START - 1 + i * h;
		if (i >= FIRST_VEHICLE) y += 6;

		if (i == 0 || !predicate(i - 1) || i == FIRST_VEHICLE)
		{
			LineDraw(TRUE, x, y, x + w, y, colour, pDestBuf);
		}
		if (i == MAX_CHARACTER_COUNT - 1 || !predicate(i + 1) || i == FIRST_VEHICLE - 1)
		{
			LineDraw(TRUE, x, y + h, x + w, y + h, colour, pDestBuf);
		}
		LineDraw(TRUE, x,     y, x,     y + h, colour, pDestBuf);
		LineDraw(TRUE, x + w, y, x + w, y + h, colour, pDestBuf);

		InvalidateRegion(x, y, x + w + 1, y + h + 1);
	}
}


static void HighLightAssignLine(void)
{
	static HighLightState state = { STARTING_COLOR_NUM, false, MAX_CHARACTER_COUNT + 1 };
	HighLightSelection(state, giAssignHighLine, ASSIGN_X, ASSIGN_WIDTH, IsCharacterSelectedForAssignment);
}


static void HighLightDestLine(void)
{
	static HighLightState state = { STARTING_COLOR_NUM, false, MAX_CHARACTER_COUNT + 1 };
	HighLightSelection(state, giDestHighLine, DEST_ETA_X, DEST_ETA_WIDTH, CharacterIsGettingPathPlotted);
}


static void HighLightSleepLine(void)
{
	static HighLightState state = { STARTING_COLOR_NUM, false, MAX_CHARACTER_COUNT + 1 };
	HighLightSelection(state, giSleepHighLine, SLEEP_X, SLEEP_WIDTH, IsCharacterSelectedForSleep);
}


static void AddCharacter(SOLDIERTYPE* const s)
{
	Assert(s != NULL);
	Assert(s->bActive);

	const size_t start = (s->uiStatusFlags & SOLDIER_VEHICLE ? FIRST_VEHICLE : 0);
	for (MapScreenCharacterSt* i = gCharactersList + start;; ++i)
	{
		Assert(i != gCharactersList + MAX_CHARACTER_COUNT);
		if (i == gCharactersList + MAX_CHARACTER_COUNT) return;
		if (i->merc == NULL)
		{
			i->merc = s;
			return;
		}
	}
}


static void LoadCharacters(void)
{
	UINT16 uiCount=0;

	// fills array with pressence of player controlled characters
	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		AddCharacter(s);
		++uiCount;
	}

	// set info char if no selected
	if( bSelectedInfoChar == -1 )
	{
		if (!DialogueActive())
		{
			ChangeSelectedInfoChar( 0, TRUE );
		}
	}

	// check if ANYONE was available
	if( uiCount == 0 )
	{
		// no one to show
		ChangeSelectedInfoChar( -1, TRUE );
		bSelectedDestChar = -1;
		bSelectedAssignChar = -1;
		bSelectedContractChar = -1;
		fPlotForHelicopter = FALSE;
	}
}


static void EnableDisableTeamListRegionsAndHelpText(void);


static void DisplayCharacterList(void)
{
	if (fShowAssignmentMenu && !fTeamPanelDirty)
	{
		SetFontDestBuffer(FRAME_BUFFER);
		return;
	}

	SetFontDestBuffer(guiSAVEBUFFER);
	SetFont(MAP_SCREEN_FONT);
	SetFontBackground(FONT_BLACK);

	for (INT16 i = 0; i != MAX_CHARACTER_COUNT; ++i)
	{
		if (!gCharactersList[i].merc) continue;
		SOLDIERTYPE const& s = *gCharactersList[i].merc;

		UINT8 foreground =
			i == (INT16)giHighLine           ? FONT_WHITE     :
			s.bLife == 0                     ? FONT_METALGRAY :
			CharacterIsGettingPathPlotted(i) ? FONT_LTBLUE    :
			/* Not in current sector? */
			s.sSector.x != sSelMap.x ||
			s.sSector.x != sSelMap.y ||
			s.sSector.z != iCurrentMapSectorZ ? 5              :
			/* Mobile? */
			s.bAssignment < ON_DUTY ||
			s.bAssignment == VEHICLE         ? FONT_YELLOW    :
			FONT_MAP_DKYELLOW;
		SetFontForeground(foreground);

		UINT16 y = Y_START + i * (Y_SIZE + Y_OFFSET) + 1;
		if (i >= FIRST_VEHICLE) y += 6;

		// Name
		DrawStringCentered(s.name, NAME_X + 1, y, NAME_WIDTH, Y_SIZE, MAP_SCREEN_FONT);

		ST::string str;

		// Location
		str = GetMapscreenMercLocationString(s);
		DrawStringCentered(str, LOC_X + 1, y, LOC_WIDTH, Y_SIZE, MAP_SCREEN_FONT);

		// Destination
		str = GetMapscreenMercDestinationString(s);
		if (str[0] != '\0')
		{
			DrawStringCentered(str, DEST_ETA_X + 1, y, DEST_ETA_WIDTH, Y_SIZE, MAP_SCREEN_FONT);
		}

		// Assignment
		if (fFlashAssignDone && s.fDoneAssignmentAndNothingToDoFlag)
		{
			SetFontForeground(FONT_RED);
		}
		ST::string assignment = GetMapscreenMercAssignmentString(s);
		DrawStringCentered(assignment, ASSIGN_X + 1, y, ASSIGN_WIDTH, Y_SIZE, MAP_SCREEN_FONT);

		// Remaining contract time
		str = GetMapscreenMercDepartureString(s, foreground != FONT_WHITE ? &foreground : 0);
		SetFontForeground(foreground);
		DrawStringCentered(str, TIME_REMAINING_X + 1, y, TIME_REMAINING_WIDTH, Y_SIZE, MAP_SCREEN_FONT);
	}

	HandleDisplayOfSelectedMercArrows();
	SetFontDestBuffer(FRAME_BUFFER);

	EnableDisableTeamListRegionsAndHelpText();

	/* Mark all pop ups as dirty, so that any open assigment menus get reblitted
		* over top of the team list */
	MarkAllBoxesAsAltered();
}

static void RefreshMapScreen()
{
	fMapPanelDirty = TRUE;
}

// THIS IS STUFF THAT RUNS *ONCE* DURING APPLICATION EXECUTION, AT INITIAL STARTUP
void MapScreenInit(void)
{
	// init palettes for big map
	InitializePalettesForMap( );

	InitMapScreenInterfaceMap();

	// set up leave list arrays for dismissed mercs
	InitLeaveList( );

	guiUpdatePanelTactical = AddVideoObjectFromFile(INTERFACEDIR "/group_confirm_tactical.sti");

	OnMapSecretFound.addListener("default:MapScreen", [](UINT8) { RefreshMapScreen(); });
}


void MapScreenShutdown(void)
{
	// free up alloced mapscreen messages
	FreeGlobalMessageList( );

	ShutDownPalettesForMap( );

	// free memory for leave list arrays for dismissed mercs
	ShutDownLeaveList( );

	DeleteVideoObject(guiUpdatePanelTactical);
}


static void AddTeamPanelSortButtonsForMapScreen(void);
static void BlitBackgroundToSaveBuffer(void);
static void CheckForAndRenderNewMailOverlay(void);
static void CheckForInventoryModeCancellation(void);
static void CheckIfPlottingForCharacterWhileAirCraft(void);
static void CheckToSeeIfMouseHasLeftMapRegionDuringPathPlotting(void);
static void ContractButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void CreateDestroyMapCharacterScrollButtons(void);
static void CreateDestroyTrashCanRegion(void);
static void CreateMouseRegionsForTeamList(void);
static void DetermineIfContractMenuCanBeShown(void);
static void FaceRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void HandleAnimatedCursorsForMapScreen(void);
static void HandleAssignmentsDoneAndAwaitingFurtherOrders(void);
static void HandleChangeOfHighLightedLine(void);
static void HandleChangeOfInfoChar(void);
static void HandleCharBarRender(void);
static void HandleCommonGlowTimer(void);
static void HandleContractTimeFlashForMercThatIsAboutLeave(void);
static void HandleCursorOverRifleAmmo(void);
static void HandleHighLightingOfLinesInTeamPanel(void);
static void HandleMapInventoryCursor(void);
static UINT32 HandleMapUI(void);
static void HandlePostAutoresolveMessages(void);
static void HandlePreBattleInterfaceWithInventoryPanelUp(void);
static void HandleSpontanousTalking(void);
static void InitPreviousPaths(void);
static void InterruptTimeForMenus(void);
static void ItemRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void ItemRegionMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MapScreenMarkRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MapscreenMarkButtonsDirty(void);
static void MonitorMapUIMessage(void);
static void PlotPermanentPaths(void);
static void PlotTemporaryPaths(void);
static void RenderMapCursorsIndexesAnims(void);
static void RenderTeamRegionBackground(void);
static BOOLEAN RequestGiveSkyriderNewDestination(void);
static void ResetAllSelectedCharacterModes(void);
static void UpDateStatusOfContractBox(void);
static void UpdateBadAssignments(void);
static void UpdateCursorIfInLastSector(void);
static void UpdatePausedStatesDueToTimeCompression(void);
static void UpdateStatusOfMapSortButtons(void);
static void UpdateTheStateOfTheNextPrevMapScreenCharacterButtons(void);


ScreenID MapScreenHandle(void)
try
{
	UINT32 uiNewScreen;
	INT32 iCounter = 0;
	static const SGPSector startSector(gamepolicy(start_sector));

	//DO NOT MOVE THIS FUNCTION CALL!!!
	//This determines if the help screen should be active
	if( ShouldTheHelpScreenComeUp( HelpScreenDetermineWhichMapScreenHelpToShow(), FALSE ) )
	{
		// handle the help screen
		HelpScreenHandler();
		return( MAP_SCREEN );
	}


	if ( !fInMapMode )
	{
		gfFirstMapscreenFrame = TRUE;

		InitPreviousPaths();

		gfInConfirmMapMoveMode = FALSE;
		gfInChangeArrivalSectorMode = FALSE;

		fLeavingMapScreen = FALSE;
		fFlashAssignDone = FALSE;
		gfEnteringMapScreen = 0;

//		fDisabledMapBorder = FALSE;

		// handle the sort buttons
		AddTeamPanelSortButtonsForMapScreen( );

		// load bottom graphics
		LoadMapScreenInterfaceBottom( );

		MoveToEndOfMapScreenMessageList( );


		// if the current time compression mode is something legal in mapscreen, keep it
		if ( ( giTimeCompressMode >= TIME_COMPRESS_5MINS ) && ( giTimeCompressMode <= TIME_COMPRESS_60MINS ) )
		{
			// leave the current time compression mode set, but DO stop it
			StopTimeCompression();
		}
		else
		{
			// set compressed mode to X0 (which also stops time compression)
			SetGameTimeCompressionLevel( TIME_COMPRESS_X0 );
		}

		// disable video overlay for tactical scroll messages
		EnableDisableScrollStringVideoOverlay( FALSE );

		CreateDestroyInsuranceMouseRegionForMercs( TRUE );

		// ATE: Init tactical interface interface ( always to team panel )
		//SetCurrentInterfacePanel( TEAM_PANEL );
		// Do some things to this now that it's initialized
		//gViewportRegion.Disable();
		//gRadarRegion.Disable();
		//Disable all faces
		SetAllAutoFacesInactive( );

		// create buttons
		CreateButtonsForMapBorder( );

		// create mouse regions for level markers
		CreateMouseRegionsForLevelMarkers( );


		// change selected sector/level if necessary
		// NOTE: Must come after border buttons are created, since it may toggle them!
		if (!AnyMercsHired())
		{
			// Select starting sector.
			ChangeSelectedMapSector(startSector);
		}
		else if (gWorldSector.IsValid())
		{
			// select currently loaded sector as the map sector
			ChangeSelectedMapSector(gWorldSector);
		}
		else	// no loaded sector
		{
			// Only select start sector, if there is no current selection, otherwise leave it as is.
			if (!sSelMap.IsValid() || iCurrentMapSectorZ == -1)
			{
				ChangeSelectedMapSector(startSector);
			}
		}

		// we are in fact in the map, do not repeat this sequence
		fInMapMode = TRUE;

		// Refreshing the whole screen, otherwise user could see remnants of
		// the tactical screen.
		if(g_ui.isBigScreen())
		{
			InvalidateRegion(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		}

		// dirty map
		fMapPanelDirty=TRUE;

		// dirty team region
		fTeamPanelDirty = TRUE;

		// dirty info region
		fCharacterInfoPanelDirty = TRUE;

		// direty map bottom region
		fMapScreenBottomDirty = TRUE;

		// tactical scroll of messages not allowed to beep until new message is added in tactical
		fOkToBeepNewMessage = FALSE;

		// not in laptop, not about to go there either
		fLapTop=FALSE;

		// reset show aircraft flag
		//fShowAircraftFlag = FALSE;

		fShowItemHighLight = FALSE;

		// reset all selected character flags
		ResetAllSelectedCharacterModes( );

		fShowMapInventoryPool = FALSE;

		// init character list - set all values in the list to 0
		InitalizeVehicleAndCharacterList( );

		LoadCharacters();


		// set up regions
		MSYS_DefineRegion( &gMapViewRegion, MAP_VIEW_START_X + MAP_GRID_X, MAP_VIEW_START_Y + MAP_GRID_Y,MAP_VIEW_START_X + MAP_VIEW_WIDTH+MAP_GRID_X-1, MAP_VIEW_START_Y + MAP_VIEW_HEIGHT-1 + 8, MSYS_PRIORITY_HIGH - 3,
					MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		MSYS_DefineRegion( &gCharInfoHandRegion,
					PLAYER_INFO_HAND_START_X, PLAYER_INFO_HAND_START_Y,
					PLAYER_INFO_HAND_END_X, PLAYER_INFO_HAND_END_Y,
					MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR,
					ItemRegionMvtCallback , ItemRegionBtnCallback );

		MSYS_DefineRegion( &gCharInfoFaceRegion, (INT16) PLAYER_INFO_FACE_START_X, (INT16) PLAYER_INFO_FACE_START_Y, (INT16) PLAYER_INFO_FACE_END_X, (INT16) PLAYER_INFO_FACE_END_Y, MSYS_PRIORITY_HIGH,
					MSYS_NO_CURSOR, MSYS_NO_CALLBACK, FaceRegionBtnCallback );

		MSYS_DefineRegion(&gMPanelRegion, INV_REGION_X, INV_REGION_Y, INV_REGION_X + INV_REGION_WIDTH, INV_REGION_Y + INV_REGION_HEIGHT, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
		// screen mask for animated cursors
		MSYS_DefineRegion(&gMapScreenMaskRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_LOW, CURSOR_NORMAL, MSYS_NO_CALLBACK, MapScreenMarkRegionBtnCallback);

		// set help text for item glow region
		gCharInfoHandRegion.SetFastHelpText(pMiscMapScreenMouseRegionHelpText[0]);

		// init the timer menus
		InitTimersForMoveMenuMouseRegions( );

		giMapContractButton = QuickCreateButtonImg(INTERFACEDIR "/contractbutton.sti", 0, 1, CONTRACT_X + 5, CONTRACT_Y - 1, MSYS_PRIORITY_HIGHEST - 5, ContractButtonCallback);
		giMapContractButton->SpecifyGeneralTextAttributes(pContractButtonString, MAP_SCREEN_FONT, CHAR_TEXT_FONT_COLOR, FONT_BLACK);
		giMapContractButton->SpecifyTextSubOffsets(0, 0, TRUE);
		giMapContractButton->SpecifyHilitedTextColors(FONT_MCOLOR_WHITE, DEFAULT_SHADOW);
		giMapContractButton->SetFastHelpText(pMapScreenMouseRegionHelpText[3]);

		CreateMouseRegionForPauseOfClock();

		// create mouse regions
		CreateMouseRegionsForTeamList( );

		ReBuildCharactersList( );

		// create status bar region
		CreateMapStatusBarsRegion( );

		if ( !gfFadeOutDone && !gfFadeIn )
		{
			MSYS_SetCurrentCursor(SCREEN_CURSOR);
		}
		gMPanelRegion.Disable();

		// create contract box
		CreateContractBox( NULL );

		// create the permanent boxes for assignment and its submenus
		fShowAssignmentMenu = TRUE;
		CreateDestroyAssignmentPopUpBoxes( );
		fShowAssignmentMenu = FALSE;


		// create merc remove box
		CreateMercRemoveAssignBox( );

		guiSAVEBUFFER->Fill(Get16BPPColor(RGB_NEAR_BLACK));
		FRAME_BUFFER->Fill( Get16BPPColor(RGB_NEAR_BLACK));

		if( gpCurrentTalkingFace != NULL )
		{
			ContinueDialogue(gpCurrentTalkingFace->soldier, FALSE);
		}

		fOneFrame = FALSE;

		if (fEnterMapDueToContract)
		{
			if( pContractReHireSoldier )
			{
				FindAndSetThisContractSoldier( pContractReHireSoldier );
			}
			fEnterMapDueToContract = FALSE;
		}

		MoveRadarScreen();
	}


	// if not going anywhere else
	if ( guiPendingScreen == NO_PENDING_SCREEN )
	{
		if ( HandleFadeOutCallback( ) )
		{
			// force mapscreen to be reinitialized even though we're already in it
			EndMapScreen( TRUE );
			return( MAP_SCREEN );
		}

		if ( HandleBeginFadeOut( MAP_SCREEN ) )
		{
			return( MAP_SCREEN );
		}
	}


	// check to see if we need to rebuild the characterlist for map screen
	HandleRebuildingOfMapScreenCharacterList( );

	HandleStrategicTurn( );


	/*
	// update cursor based on state
	if (bSelectedDestChar == -1 && !fPlotForHelicopter && !gfInChangeArrivalSectorMode)
	{
		// reset cursor
		if ( !gfFadeIn )
		{
			ChangeMapScreenMaskCursor( CURSOR_NORMAL );
		}
	}*/

	// check if we are going to create or destroy map border graphics?
	CreateDestroyMapInventoryPoolButtons( FALSE );

	// set up buttons for mapscreen scroll
	//HandleMapScrollButtonStates( );


	// don't process any input until we've been through here once
	if (!gfFirstMapscreenFrame)
	{
		// Handle Interface
		uiNewScreen = HandleMapUI( );
		if ( uiNewScreen != MAP_SCREEN )
		{
			return( MAP_SCREEN );
		}
	}


	// handle flashing of contract column for any mercs leaving very soon
	HandleContractTimeFlashForMercThatIsAboutLeave( );

	if (!fShownAssignmentMenu && fShowAssignmentMenu)
	{
		// need a one frame pause
		fShownAssignmentMenu = fShowAssignmentMenu;
		fShowAssignmentMenu = FALSE;
		fOneFrame = TRUE;
	}
	else if (!fShownContractMenu && fShowContractMenu)
	{
		fShownContractMenu = fShowContractMenu;
		fShowContractMenu = FALSE;
		fOneFrame = TRUE;
	}
	else if( fOneFrame )
	{
		// one frame passed
		fShowContractMenu = fShownContractMenu;
		fShowAssignmentMenu = fShownAssignmentMenu;
		fOneFrame = FALSE;
	}

	if (!fShownAssignmentMenu && !fShowAssignmentMenu && !fShownContractMenu)
	{
		bSelectedAssignChar = -1;
	}

	HandlePostAutoresolveMessages();

	//	UpdateLevelButtonStates( );

	// NOTE: This must happen *before* UpdateTheStateOfTheNextPrevMapScreenCharacterButtons()
	CreateDestroyMapCharacterScrollButtons( );

	// update the prev next merc buttons
	UpdateTheStateOfTheNextPrevMapScreenCharacterButtons( );

	// handle for inventory
	HandleCursorOverRifleAmmo( );

	// check contract times, update screen if they do change
	CheckAndUpdateBasedOnContractTimes( );

	// handle flashing of assignment strings when merc's assignment is done
	HandleAssignmentsDoneAndAwaitingFurtherOrders( );

	// handle timing for the various glowing higlights
	HandleCommonGlowTimer( );

	// are we attempting to plot a foot/vehicle path during aircraft mode..if so, stop it
	CheckIfPlottingForCharacterWhileAirCraft( );

	// check to see if helicopter is available
	//CheckIfHelicopterAvailable( );
	if( fShowMapInventoryPool )
	{
		HandleFlashForHighLightedItem( );
	}


//	CreateDestroyMovementBox( 0,0,0 );

	// Deque all game events
	DequeAllGameEvents();

	// Handle Interface Stuff
	SetUpInterface( );

	// reset time compress has occured
	ResetTimeCompressHasOccured( );

	// handle change in info char
	HandleChangeOfInfoChar( );

	// update status of contract box
	UpDateStatusOfContractBox( );

	// error check of assignments
	UpdateBadAssignments( );

	// if cursor has left map..will need to update temp path plotting and cursor
	CheckToSeeIfMouseHasLeftMapRegionDuringPathPlotting( );

	// update assignment menus and submenus
	HandleShadingOfLinesForAssignmentMenus( );

	// check which menus can be shown right now
	DetermineWhichAssignmentMenusCanBeShown( );

	// determine if contract menu can be shown
	DetermineIfContractMenuCanBeShown( );

	// if pre battle and inventory up?..get rid of inventory
	HandlePreBattleInterfaceWithInventoryPanelUp( );

	// create destroy trash can region
	CreateDestroyTrashCanRegion( );

	// update these buttons
	UpdateStatusOfMapSortButtons( );

	// if in inventory mode, make sure it's still ok
	CheckForInventoryModeCancellation();

	// restore background rects
	RestoreBackgroundRects( );

	InterruptTimeForMenus( );

	// place down background
	BlitBackgroundToSaveBuffer( );

	if (fLeavingMapScreen) return MAP_SCREEN;

	if (!fDisableDueToBattleRoster)
	{
		/*
		// ATE: OK mark is rendering the item every frame - which isn't good
		// however, don't want to break the world here..
		// this line was added so that when the ItemGlow() is on,
		// we're not rendering also, else glow looks bad
		if ( !fShowItemHighLight )
		{
			RenderHandPosItem();
		}
		*/

		if( fDrawCharacterList )
		{
			if ( !fShowInventoryFlag )
			{
				// if we are not in inventory mode, show character list
				HandleHighLightingOfLinesInTeamPanel( );

				DisplayCharacterList();
			}

			fDrawCharacterList = FALSE;
		}
	}


	if (!fShowMapInventoryPool && !gfPauseDueToPlayerGamePause /* && !fDisabledMapBorder */)
	{
		RenderMapCursorsIndexesAnims( );
	}

	if (!fDisableDueToBattleRoster)
	{
		// render status bar
		HandleCharBarRender( );
	}

	if( fShowInventoryFlag || fDisableDueToBattleRoster )
	{
		for( iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++ )
		{
			UnMarkButtonDirty( giMapSortButton[ iCounter ] );
		}
	}

	if( fShowContractMenu || fDisableDueToBattleRoster )
	{
		UnMarkButtonDirty( giMapContractButton );
	}

	// handle any old messages
	ScrollString( );


	HandleSpontanousTalking( );

	if (!fDisableDueToBattleRoster)
	{
		// remove the move box once user leaves it
		CreateDestroyMovementBox( 0,0,0 );

		// this updates the move box contents when changes took place
		ReBuildMoveBox( );
	}

	if (!fDisableDueToBattleRoster &&
		(fShowAssignmentMenu || fShowContractMenu))
	{
		// highlight lines?
		HandleHighLightingOfLinesInTeamPanel( );

		// render glow for contract region
		GlowTrashCan( );

		// handle changing of highlighted lines
		HandleChangeOfHighLightedLine( );
	}

	if (!fDisableDueToBattleRoster)
	{
		// render face of current info char, for animation
		DrawFace();

		// handle autofaces
		HandleAutoFaces( );
		HandleTalkingAutoFaces( );

		//GlowItem( );

	}


	// automatically turns off mapscreen ui overlay messages when appropriate
	MonitorMapUIMessage( );


	// if heli is around, show it
	if (iHelicopterVehicleId != -1 && fShowAircraftFlag && iCurrentMapSectorZ == 0 && !fShowMapInventoryPool)
	{
		// this is done on EVERY frame, I guess it beats setting entire map dirty all the time while he's moving...
		DisplayPositionOfHelicopter( );
	}


	// display town info
	DisplayTownInfo(SGPSector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ));

	if (fShowTownInfo)
	{
		// force update of town mine info boxes
		ForceUpDateOfBox( ghTownMineBox );
		MapscreenMarkButtonsDirty();
	}

	if( fShowAttributeMenu )
	{
		// mark all popups as dirty
		MarkAllBoxesAsAltered( );
	}

	// if plotting path
	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		// plot out paths
		PlotPermanentPaths( );
		PlotTemporaryPaths( );

		// show ETA
		RenderMapBorderEtaPopUp( );
		DisplayGroundEta();
	}

	HandleContractRenewalSequence( );

	// handle dialog
	HandleDialogue( );


	// now the border corner piece
	//RenderMapBorderCorner( );


	// Display Framerate
	DisplayFrameRate( );

	// update paused states
	UpdatePausedStatesDueToTimeCompression( );

	// is there a description to be displayed?
	RenderItemDescriptionBox( );

	RenderClock();

	if (fEndShowInventoryFlag)
	{
		if (InKeyRingPopup())
		{
			DeleteKeyRingPopup( );
		}
		else
		{
			fShowInventoryFlag = FALSE;
		}

		fTeamPanelDirty = TRUE;
		fEndShowInventoryFlag = FALSE;
	}

	// handle animated cursor update
	if ( !gfFadeIn )
	{
		HandleAnimatedCursorsForMapScreen( );
	}

	// if inventory is being manipulated, update cursor
	HandleMapInventoryCursor();

	if (fShowDescriptionFlag)
	{
		// unmark done button
		if (gpItemDescObject->usItem == MONEY ||
				GCM->getItem(gpItemDescObject->usItem)->isGun())
		{
			MapscreenMarkButtonsDirty();
		}

		UnMarkButtonDirty( giMapInvDoneButton );
		//UnMarkButtonDirty( giCharInfoButton[ 0 ] );
		//UnMarkButtonDirty( giCharInfoButton[ 1 ] );
		MarkAButtonDirty( giMapInvDescButton );
	}
	else
	{
		if (fShowInventoryFlag)
		{
			MarkAButtonDirty( giMapInvDoneButton );
			MarkAButtonDirty( giCharInfoButton[ 1 ] );
			MarkAButtonDirty( giCharInfoButton[ 0 ] );
		}
	}

	DrawMilitiaPopUpBox( );

	if (!fDisableDueToBattleRoster)
	{
		CreateDestroyTheUpdateBox( );
		DisplaySoldierUpdateBox( );
	}


	// pop up display boxes
	DisplayBoxes(FRAME_BUFFER);

	// render buttons
	RenderButtons( );

	if( fShowMapScreenMovementList )
	{
		// redisplay Movement box to blit it over any border buttons, since if long enough it can overlap them
		ForceUpDateOfBox( ghMoveBox );
		DisplayOnePopupBox( ghMoveBox, FRAME_BUFFER );
	}

	if( fShowContractMenu )
	{
		// redisplay Contract box to blit it over any map sort buttons, since they overlap
		ForceUpDateOfBox( ghContractBox );
		DisplayOnePopupBox( ghContractBox, FRAME_BUFFER );
	}


	//If we have new email, blink the email icon on top of the laptop button.
	CheckForAndRenderNewMailOverlay();

	// handle video overlays
	ExecuteVideoOverlays( );

	if ( InItemStackPopup( ) )
	{
		RenderItemStackPopup( FALSE );
	}

	if( InKeyRingPopup() )
	{
		RenderKeyRingPopup( FALSE );
	}

	CheckForMeanwhileOKStart( );

	// save background rects
	// ATE: DO this BEFORE rendering help text....
	SaveBackgroundRects( );

	if (!fDisableDueToBattleRoster && !fShowAssignmentMenu && !fShowContractMenu)
	{
		// highlight lines?
		HandleHighLightingOfLinesInTeamPanel( );

		// render glow for contract region
		GlowTrashCan( );

		// handle changing of highlighted lines
		HandleChangeOfHighLightedLine( );

		GlowItem( );
	}


	RenderButtonsFastHelp();

	// execute dirty
	ExecuteBaseDirtyRectQueue( );

	// update cursor
	UpdateCursorIfInLastSector( );

	EndFrameBufferRender( );


	// if not going anywhere else
	if ( guiPendingScreen == NO_PENDING_SCREEN )
	{
		if ( HandleFadeInCallback( ) )
		{
			// force mapscreen to be reinitialized even though we're already in it
			EndMapScreen( TRUE );
		}

		if ( HandleBeginFadeIn( MAP_SCREEN ) )
		{
		}
	}

	HandlePreBattleInterfaceStates();

	if( gfHotKeyEnterSector )
	{
		gfHotKeyEnterSector = FALSE;
		ActivatePreBattleEnterSectorAction();
	}

	if ( gfRequestGiveSkyriderNewDestination )
	{
		RequestGiveSkyriderNewDestination();
		gfRequestGiveSkyriderNewDestination = FALSE;
	}


	if( gfFirstMapscreenFrame )
	{
		gfFirstMapscreenFrame = FALSE;
	}
	else
	{
		// handle exiting from mapscreen due to both exit button clicks and keyboard equivalents
		HandleExitsFromMapScreen( );
	}


	return( MAP_SCREEN );
}
catch (...) { return ERROR_SCREEN; /* XXX TODO001A originally returned FALSE */ }


static void DrawString(const ST::string& str, UINT16 uiX, UINT16 uiY, SGPFont font)
{
	// draw monochrome string
	SetFont(font);
	GDirtyPrint(uiX, uiY, str);
}


static void DrawStringCentered(const ST::string& str, UINT16 x, UINT16 y, UINT16 w, UINT16 h, SGPFont font)
{
	INT16 cx;
	INT16 cy;
	FindFontCenterCoordinates(x, y, w, h, str, font, &cx, &cy);
	DrawString(str, cx, cy, font);
}


void DrawStringRight(const ST::string& str, UINT16 x, UINT16 y, UINT16 w, UINT16 h, SGPFont font)
{
	INT16 rx;
	INT16 ry;
	FindFontRightCoordinates(x, y, w, h, str, font, &rx, &ry);
	DrawString(str, rx, ry, font);
}


static void RenderMapHighlight(const SGPSector& sMap, UINT16 usLineColor, BOOLEAN fStationary);
static void RestoreMapSectorCursor(const SGPSector& sMap);


static void RenderMapCursorsIndexesAnims(void)
{
	BOOLEAN fSelectedSectorHighlighted = FALSE;
	BOOLEAN fSelectedCursorIsYellow = TRUE;
	UINT16 usCursorColor;
	UINT32 uiDeltaTime;
	static SGPSector sPrevHighlightedMap(-1, -1);
	static SGPSector sPrevSelectedMap(-1, -1);
	static BOOLEAN fFlashCursorIsYellow = FALSE;
	BOOLEAN fDrawCursors;
	BOOLEAN fHighlightChanged = FALSE;


	HandleAnimationOfSectors( );

	if( gfBlitBattleSectorLocator )
	{
		HandleBlitOfSectorLocatorIcon(gubPBSector, LOCATOR_COLOR_RED);
	}

	fDrawCursors = CanDrawSectorCursor( );

	// if mouse cursor is over a map sector
	if (fDrawCursors && GetMouseMapXY(gsHighlightSector))
	{
		// handle highlighting of sector pointed at ( WHITE )

		// if we're over a different sector than when we previously blitted this
		if (gsHighlightSector != sPrevHighlightedMap || gfMapPanelWasRedrawn)
		{
			if (sPrevHighlightedMap.IsValid())
			{
				RestoreMapSectorCursor(sPrevHighlightedMap);
			}

			// draw WHITE highlight rectangle
			RenderMapHighlight(gsHighlightSector, Get16BPPColor(RGB_WHITE), FALSE);

			sPrevHighlightedMap = gsHighlightSector;
			fHighlightChanged = TRUE;
		}
	}
	else
	{
		// nothing now highlighted
		gsHighlightSector.x = -1;
		gsHighlightSector.y = -1;

		if (sPrevHighlightedMap.IsValid())
		{
			RestoreMapSectorCursor(sPrevHighlightedMap);
			fHighlightChanged = TRUE;
		}

		sPrevHighlightedMap.x = -1;
		sPrevHighlightedMap.y = -1;
	}


	// handle highlighting of selected sector ( YELLOW ) - don't show it while plotting movement
	if (fDrawCursors && bSelectedDestChar == -1 && !fPlotForHelicopter)
	{
		// if mouse cursor is over the currently selected sector
		if (gsHighlightSector == sSelMap)
		{
			fSelectedSectorHighlighted = TRUE;

			// do we need to flash the cursor?  get the delta in time
			uiDeltaTime = GetJA2Clock( ) - guiFlashCursorBaseTime;

			if ( uiDeltaTime > 300 )
			{
				guiFlashCursorBaseTime = GetJA2Clock();
				fFlashCursorIsYellow = !fFlashCursorIsYellow;

				fHighlightChanged = TRUE;
			}
		}

		if ( !fSelectedSectorHighlighted || fFlashCursorIsYellow )
		{
			// draw YELLOW highlight rectangle
			usCursorColor = Get16BPPColor( RGB_YELLOW );
		}
		else
		{
			// draw WHITE highlight rectangle
			usCursorColor = Get16BPPColor( RGB_WHITE );

			// index letters will also be white instead of yellow so that they flash in synch with the cursor
			fSelectedCursorIsYellow = FALSE;
		}

		// always render this one, it's too much of a pain detecting overlaps with the white cursor otherwise
		RenderMapHighlight(sSelMap, usCursorColor, TRUE);

		if (sPrevSelectedMap != sSelMap)
		{
			sPrevSelectedMap = sSelMap;
			fHighlightChanged = TRUE;
		}
	}
	else
	{
		// erase yellow highlight cursor
		if (sPrevSelectedMap.IsValid())
		{
			RestoreMapSectorCursor(sPrevSelectedMap);
			fHighlightChanged = TRUE;
		}

		sPrevSelectedMap.x = -1;
		sPrevSelectedMap.y = -1;
	}


	if ( fHighlightChanged || gfMapPanelWasRedrawn )
	{
		// redraw sector index letters and numbers
		DrawMapIndexBigMap( fSelectedCursorIsYellow );
	}
}


static BOOLEAN AnyMovableCharsInOrBetweenThisSector(const SGPSector& sSector);
static BOOLEAN CanMoveBullseyeAndClickedOnIt(const SGPSector& sMap);
static void CancelChangeArrivalSectorMode(void);
static void CancelOrShortenPlottedPath(void);
static void CreateBullsEyeOrChopperSelectionPopup(void);
static void GetMapKeyboardInput(MapEvent&);
static void PollLeftButtonInMapView(MapEvent&);
static void PollRightButtonInMapView(MapEvent&);
static void StartChangeSectorArrivalMode(void);
static void StartConfirmMapMoveMode(INT16 sMapY);


static void OpenSectorInventory()
{
	if(!fShowMapInventoryPool)
	{
		fShowMapInventoryPool = TRUE;
		CreateDestroyMapInventoryPoolButtons( TRUE );
	}
}

static void ToggleSectorInventory()
{
	if(!fShowMapInventoryPool)
	{
		OpenSectorInventory();
	}
	else
	{
		CancelSectorInventoryDisplayIfOn(FALSE);
	}
}

// Drawing the Map
static UINT32 HandleMapUI(void)
{
	MapEvent new_event = MAP_EVENT_NONE;
	SGPSector sSector, sMap;
	UINT32 uiNewScreen = MAP_SCREEN;
	BOOLEAN fWasAlreadySelected;


	// Get Input from keyboard
	GetMapKeyboardInput(new_event);


	CreateDestroyMapInvButton();

	// Get mouse
	PollLeftButtonInMapView(new_event);
	PollRightButtonInMapView(new_event);

	// Switch on event
	switch (new_event)
	{
		case MAP_EVENT_NONE:
			break;

		case MAP_EVENT_PLOT_PATH:
			GetMouseMapXY(sMap);
			// plotting for the chopper?
			if (fPlotForHelicopter)
			{
					PlotPathForHelicopter(sMap);
					fTeamPanelDirty = TRUE;
			}
			else
			{
				// plot for character

				// check for valid character
				Assert ( bSelectedDestChar != -1 );
				if ( bSelectedDestChar == -1 )
					break;

				// check if last sector in character's path is same as where mouse is
				SOLDIERTYPE* const s = gCharactersList[bSelectedDestChar].merc;
				const INT16 dst_sector = GetLastSectorIdInCharactersPath(s);
				if (dst_sector != sMap.AsStrategicIndex())
				{
					sSector = SGPSector::FromStrategicIndex(dst_sector);
					RestoreBackgroundForMapGrid(sSector);
					// fMapPanelDirty = TRUE;
				}

				if (SectorIsPassable(sMap.AsByte()))
				{
					// Can we get go there?  (NULL temp character path)
					if ( GetLengthOfPath( pTempCharacterPath ) > 0 )
					{
						PlotPathForCharacter(*s, sMap, false);

						// copy the path to every other selected character
						CopyPathToAllSelectedCharacters(GetSoldierMercPathPtr(s));

						StartConfirmMapMoveMode(sMap.y);
						fMapPanelDirty = TRUE;
						fTeamPanelDirty = TRUE;	// update team panel desinations
					}
					else
					{
						// means it's a vehicle and we've clicked an off-road sector
						BeginMapUIMessage(0, pMapErrorString[40]);
					}
				}
			}
			break;


		case MAP_EVENT_CANCEL_PATH:
			CancelOrShortenPlottedPath( );
			break;

		case MAP_EVENT_CLICK_SECTOR:

			// Get Current mouse position
			if (GetMouseMapXY(sMap))
			{
				// make sure this is a valid sector
				if (!IsTheCursorAllowedToHighLightThisSector(sMap))
				{
					// if we are in the change drop off sector mode
					if (gfInChangeArrivalSectorMode)
					{
						// display a message as to why we can't change
						const ST::string* sMsgString = GCM->getNewString(NS_INVALID_DROPOFF_SECTOR);
						BeginMapUIMessage(0, *sMsgString);
					}
					// nothing else to do, return
					return( MAP_SCREEN );
				}


				// while item in hand
				if ( fMapInventoryItem )
				{
					// if not showing item counts on the map
					if ( !fShowItemsFlag )
					{
						// turn that on
						ToggleItemsFilter( );
					}

					// if item's owner is known
					if ( gpItemPointerSoldier != NULL )
					{
						// make sure it's the owner's sector that's selected
						if ( (gpItemPointerSoldier->sSector.x != sSelMap.x ) ||
							( gpItemPointerSoldier->sSector.y != sSelMap.y ) ||
							( gpItemPointerSoldier->sSector.z != iCurrentMapSectorZ ) )
						{
							ChangeSelectedMapSector(gpItemPointerSoldier->sSector);
						}
					}

					// if not already in sector inventory
					if ( !fShowMapInventoryPool )
					{
						// start it up ( NOTE: for the item OWNER'S sector, regardless of which sector player clicks )
						OpenSectorInventory();
					}

					return( MAP_SCREEN );
				}


				// don't permit other click handling while item is in cursor (entering PBI would permit item teleports, etc.)
				Assert( !fMapInventoryItem );


				// this doesn't change selected sector
				if ( gfInChangeArrivalSectorMode )
				{
					if (SectorInfo[sMap.AsByte()].ubTraversability[ THROUGH_STRATEGIC_MOVE ] != GROUNDBARRIER)
					{
						// if it's not enemy air controlled
						if (!StrategicMap[sMap.AsStrategicIndex()].fEnemyAirControlled)
						{
							ST::string sMsgString;
							ST::string sMsgSubString;

							// move the landing zone over here
							g_merc_arrive_sector = sMap;

							// change arrival sector for all mercs currently in transit who are showing up at the landing zone
							UpdateAnyInTransitMercsWithGlobalArrivalSector();

							// we're done, cancel this mode
							CancelChangeArrivalSectorMode();

							// get the name of the sector
							sMsgSubString = GetSectorIDString(sMap, FALSE);

							// now build the string
							sMsgString = st_format_printf(pBullseyeStrings[ 1 ], sMsgSubString);

							// confirm the change with overlay message
							BeginMapUIMessage(0, sMsgString);

							// update destination column for any mercs in transit
							fTeamPanelDirty = TRUE;
						}
						else
						{
							// message: not allowed, don't have airspace secured
							BeginMapUIMessage(0, pBullseyeStrings[2]);
						}
					}

					return( MAP_SCREEN );
				}
				else	// not already changing arrival sector
				{
					if (CanMoveBullseyeAndClickedOnIt(sMap))
					{
						// if the click is ALSO over the helicopter icon
						// NOTE: The helicopter icon is NOT necessarily directly over the helicopter's current sector!!!
						if (CheckForClickOverHelicopterIcon(sMap))
						{
							CreateBullsEyeOrChopperSelectionPopup();
						}
						else
						{
							StartChangeSectorArrivalMode();
						}

						return( MAP_SCREEN );
					}
				}


				sMap.z = iCurrentMapSectorZ;
				// if new map sector was clicked on
				if (sSelMap != sMap)
				{
					fWasAlreadySelected = FALSE;

					// select the clicked sector, retaining the same sublevel depth
					ChangeSelectedMapSector(sMap);
				}
				else
				{
					fWasAlreadySelected = TRUE;
				}

				// if showing item counts on the map, and not already in sector inventory
				if ( fShowItemsFlag && !fShowMapInventoryPool )
				{
					// show sector inventory for this clicked sector
					ChangeSelectedMapSector(sMap);

					OpenSectorInventory();

					return( MAP_SCREEN );
				}


				if (gfBlitBattleSectorLocator && sMap == gubPBSector)
				{ //Bring up a non-persistant version of mapscreen if the user clicks on the sector where a
					//battle is taking place.
					InitPreBattleInterface(0, false);
					return( MAP_SCREEN );
				}


				// if we're in airspace mode
				if (fShowAircraftFlag)
				{
					// if not moving soldiers, and not yet plotting the helicopter
					if (bSelectedDestChar == -1 && !fPlotForHelicopter)
					{
						// if we're on the surface level, and the click is over the helicopter icon
						// NOTE: The helicopter icon is NOT necessarily directly over the helicopter's current sector!!!
						if (iCurrentMapSectorZ == 0 && CheckForClickOverHelicopterIcon(sMap))
						{
							RequestGiveSkyriderNewDestination( );
							return( MAP_SCREEN );
						}
					}
				}
				else	// not in airspace mode
				{
					// sector must be already selected to initiate movement plotting!  This is to allow selecting sectors with
					// mercs in them without necessarily initiating movement right away.
					if( fWasAlreadySelected )
					{
						// if there are any movable characters here
						if (AnyMovableCharsInOrBetweenThisSector(sMap))
						{
							// if showing the surface level map
							if( iCurrentMapSectorZ == 0 )
							{
								TurnOnShowTeamsMode( );

								// NOTE: must allow move box to come up, since there may be movable characters between sectors which are
								// unaffected by combat / hostiles / air raid in the sector proper itself!!
								// This also allows all strategic movement error handling to be centralized in CanCharacterMoveInStrategic()

								// start the move box menu
								SetUpMovingListsForSector(sMap);
							}
							else
							{
								// no strategic movement is possible from underground sectors
								DoMapMessageBox( MSG_BOX_BASIC_STYLE, pMapErrorString[ 1 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
								return( MAP_SCREEN );
							}
						}
					}
				}
			}
			break;

	}


	// if we pressed something that will cause a screen change
	if ( guiPendingScreen != NO_PENDING_SCREEN )
	{
		uiNewScreen = guiPendingScreen;
	}

	return( uiNewScreen );
}


static bool HandleKeyESC()
{
	gfDontStartTransitionFromLaptop = TRUE;

	if (gfPreBattleInterfaceActive && !gfPersistantPBI)
	{ //Non persistant PBI.  Allow ESC to close it and return to mapscreen.
		KillPreBattleInterface();
		gpBattleGroup = NULL;
		return true;
	}

	if (gfInChangeArrivalSectorMode)
	{
		CancelChangeArrivalSectorMode();
		BeginMapUIMessage(0, pBullseyeStrings[3]);
	}
	// ESC cancels MAP UI messages, unless we're in confirm map move mode
	else if (g_ui_message_overlay != NULL && !gfInConfirmMapMoveMode)
	{
		CancelMapUIMessage();
	}
	else if (gpCurrentTalkingFace != NULL && gpCurrentTalkingFace->fTalking)
	{
		// ATE: We want to stop speech if somebody is talking...
		StopAnyCurrentlyTalkingSpeech();
	}
	else if (fShowUpdateBox)
	{
		EndUpdateBox(FALSE); // stop time compression
	}
	else if(fShowDescriptionFlag)
	{
		DeleteItemDescriptionBox();
	}
	// plotting movement?
	else if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode();
	}
	else if (fShowAssignmentMenu)
	{
		// dirty region
		fTeamPanelDirty = TRUE;
		fMapPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		// stop showing current assignment box
		if (fShowAttributeMenu)
		{
			fShowAttributeMenu = FALSE;
			fMapPanelDirty = TRUE;
		}
		else if (fShowTrainingMenu)
		{
			fShowTrainingMenu = FALSE;
		}
		else if (fShowSquadMenu)
		{
			fShowSquadMenu = FALSE;
		}
		else if (fShowRepairMenu)
		{
			fShowRepairMenu = FALSE;
		}
		else
		{
			fShowAssignmentMenu = FALSE;
		}
		giAssignHighLine = -1;
		// restore background to glow region
		RestoreBackgroundForAssignmentGlowRegionList();
	}
	else if (fShowContractMenu)
	{
		fShowContractMenu = FALSE;

		// restore contract glow region
		RestoreBackgroundForContractGlowRegionList();
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
		giContractHighLine = -1;
	}
	// in militia popup?
	else if (sSelectedMilitiaTown != 0 && sGreensOnCursor == 0 && sRegularsOnCursor == 0 && sElitesOnCursor == 0)
	{
		sSelectedMilitiaTown = 0;
		fMapPanelDirty = TRUE;
	}
	else if (fShowTownInfo)
	{
		fShowTownInfo = FALSE;
		CreateDestroyScreenMaskForAssignmentAndContractMenus();
	}
	else if (InKeyRingPopup())
	{
		DeleteKeyRingPopup( );
		fTeamPanelDirty = TRUE;
	}
	else if (fShowInventoryFlag)
	{
		if (!fMapInventoryItem && !InItemStackPopup())
		{
			fEndShowInventoryFlag = TRUE;
		}
	}
	else if (MultipleCharacterListEntriesSelected())
	{
		ResetSelectedListForMapScreen();
		if (bSelectedInfoChar != -1)
		{
			SetEntryInSelectedCharacterList(bSelectedInfoChar);
		}
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
	else
	{
		RequestTriggerExitFromMapscreen(MAP_EXIT_TO_TACTICAL);
	}
	return false;
}


static void Teleport()
{
	if (bSelectedDestChar == -1) return;
	if (fPlotForHelicopter)      return;
	if (iCurrentMapSectorZ != 0) return;

	SGPSector sMap;
	if (!GetMouseMapXY(sMap)) return;

	SOLDIERTYPE& s = *gCharactersList[bSelectedDestChar].merc;

	// can't teleport to where we already are
	if (sMap == s.sSector) return;

	// cancel movement plotting
	AbortMovementPlottingMode();

	// nuke the UI message generated by this
	CancelMapUIMessage();

	// clear their strategic movement (mercpaths and waypoints)
	ClearMvtForThisSoldierAndGang(&s);

	// select this sector
	ChangeSelectedMapSector(sMap);

	// check to see if this person is moving, if not...then assign them to mvt group
	if (s.ubGroupID  == 0)
	{
		GROUP& g = *CreateNewPlayerGroupDepartingFromSector(s.sSector);
		AddPlayerToGroup(g, s);
	}

	// figure out where they would've come from
	const SGPSector sDelta = sMap - s.sSector;
	SGPSector sPrev = sMap;
	if (std::abs(sDelta.x) >= std::abs(sDelta.y))
	{
		// use East or West
		if (sDelta.x > 0)
		{
			// came in from the West
			sPrev.x--;
		}
		else
		{
			// came in from the East
			sPrev.x++;
		}
	}
	else
	{
		// use North or South
		if (sDelta.y > 0)
		{
			// came in from the North
			sPrev.y--;
		}
		else
		{
			// came in from the South
			sPrev.y++;
		}
	}

	// set where they are, were/are going, then make them arrive there and check for battle
	PlaceGroupInSector(*GetGroup(s.ubGroupID), sPrev, sMap, true);

	// unload the sector they teleported out of
	CheckAndHandleUnloadingOfCurrentWorld();
}


static void ChangeCharacterListSortMethod(INT32 iValue);
static void RequestContractMenu(void);
static void RequestToggleMercInventoryPanel(void);
static void SelectAllCharactersInSquad(INT8 bSquadNumber);


static void HandleModNone(UINT32 const key, MapEvent& new_event)
{
	switch (key)
	{
		case SDLK_ESCAPE: if (HandleKeyESC()) return; break;

		case SDLK_PAUSE: HandlePlayerPauseUnPauseOfGame(); break;

		case SDLK_LEFT:  GoToPrevCharacterInList(); break;
		case SDLK_RIGHT: GoToNextCharacterInList(); break;

		case SDLK_UP:   MapScreenMsgScrollUp(1);   break;
		case SDLK_DOWN: MapScreenMsgScrollDown(1); break;

		case SDLK_PAGEUP:   MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);   break;
		case SDLK_PAGEDOWN: MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM); break;

		case SDLK_HOME: ChangeCurrentMapscreenMessageIndex(0); break;
		case SDLK_END:  MoveToEndOfMapScreenMessageList();     break;

		case SDLK_INSERT: GoUpOneLevelInMap();   break;
		case SDLK_DELETE: GoDownOneLevelInMap(); break;

		case SDLK_RETURN: RequestToggleMercInventoryPanel(); break;

		case SDLK_BACKSPACE: StopAnyCurrentlyTalkingSpeech(); break;

		case SDLK_F1:
		case SDLK_F2:
		case SDLK_F3:
		case SDLK_F4:
		case SDLK_F5:
		case SDLK_F6: ChangeCharacterListSortMethod(key - SDLK_F1); break;

		case '+':
		case '=':
			if (!CommonTimeCompressionChecks()) RequestIncreaseInTimeCompression();
			break;

		case '-':
		case '_':
			if (!CommonTimeCompressionChecks()) RequestDecreaseInTimeCompression();
			break;

		case SDLK_SPACE:
			if (fShowUpdateBox)
			{ // Restart time compression
				EndUpdateBox(TRUE);
			}
			else
			{ // Toggle time compression
				if (!CommonTimeCompressionChecks()) RequestToggleTimeCompression();
			}
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{ // Select all characters in squad 1-10
			UINT const squad_no = (key - SDLK_0 + 9) % 10U;
			SelectAllCharactersInSquad(squad_no);
			break;
		}

		case 'a':
			if (gfPreBattleInterfaceActive)
			{ // Activate autoresolve in prebattle interface.
				ActivatePreBattleAutoresolveAction();
			}
			else if (!fShowMapInventoryPool) // only handle border button keyboard equivalents if the button is visible!
			{
				ToggleAirspaceMode();
			}
			break;

		case 'c': RequestContractMenu(); break;

		case 'e':
			// Activate enter sector in prebattle interface.
			if (gfPreBattleInterfaceActive) gfHotKeyEnterSector = TRUE;
			break;

		case 'h':
			// ARM: Feb01/98 - Cancel out of mapscreen movement plotting if Help subscreen is coming up
			if (bSelectedDestChar != -1 || fPlotForHelicopter)
			{
				AbortMovementPlottingMode();
			}
			ShouldTheHelpScreenComeUp(HELP_SCREEN_MAPSCREEN, TRUE);
			break;

		case 'i':
			// Only handle border button keyboard equivalents if the button is visible
			if (!fShowMapInventoryPool) ToggleItemsFilter();
			break;

		case 'l': RequestTriggerExitFromMapscreen(MAP_EXIT_TO_LAPTOP); break;

		case 'm':
			// Only handle border button keyboard equivalents if the button is visible
			if (!fShowMapInventoryPool) ToggleShowMinesMode();
			break;

		case 'o': RequestTriggerExitFromMapscreen(MAP_EXIT_TO_OPTIONS); break;

		case 'r':
			if (gfPreBattleInterfaceActive) ActivatePreBattleRetreatAction();
			break;

		case 't':
			// Only handle border button keyboard equivalents if the button is visible
			if (!fShowMapInventoryPool) ToggleShowTeamsMode();
			break;

		case 'v': DisplayGameSettings(); break;

		case 'w':
			// Only handle border button keyboard equivalents if the button is visible
			if (!fShowMapInventoryPool) ToggleShowTownsMode();
			break;

		case 'z':
			// Only handle border button keyboard equivalents if the button is visible
			if (!fShowMapInventoryPool) ToggleShowMilitiaMode();
			break;
	}
}


static void HandleModShift(UINT const key)
{
	switch (key)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{ // Select all characters in squad 11-20
			UINT const squad_no = 10 + (key - SDLK_0 + 9) % 10U;
			SelectAllCharactersInSquad(squad_no);
			break;
		}
	}
}


static void HandleModCtrl(UINT const key)
{
	switch (key)
	{
		case 'a':
			if (CHEATER_CHEAT_LEVEL())
			{
				gfAutoAmbush ^= 1;
				if(gfAutoAmbush)
				{
					SLOGD("Enemy ambush test mode enabled.");
				}
				else
				{
					SLOGD("Enemy ambush test mode disabled.");
				}
			}
			break;

	case 'i':
		if (gamepolicy(isHotkeyEnabled(UI_Map, HKMOD_CTRL, 'i')))
		{
			ToggleSectorInventory();
		}
		break;

		case 'l':
			// go to LOAD screen
			gfSaveGame = FALSE;
			RequestTriggerExitFromMapscreen(MAP_EXIT_TO_LOAD);
			break;

		case 's':
			// go to SAVE screen
			gfSaveGame = TRUE;
			RequestTriggerExitFromMapscreen(MAP_EXIT_TO_SAVE);
			break;

		case 't': if (CHEATER_CHEAT_LEVEL()) Teleport(); break;

#if defined SGP_VIDEO_DEBUGGING
		case 'v':
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, ST::format("VObjects:  {}", guiVObjectSize));
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, ST::format("VSurfaces:  {}", guiVSurfaceSize));
			ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "SGPVideoDump.txt updated...");
			PerformVideoInfoDumpIntoFile("SGPVideoDump.txt", TRUE);
			break;
#endif

		case 'z':
			if (CHEATER_CHEAT_LEVEL())
			{
				gfAutoAIAware ^= 1;
				if(gfAutoAIAware)
				{
					SLOGD("Strategic AI awareness maxed.");
				}
				else
				{
					SLOGD("Strategic AI awareness normal.");
				}
			}
			break;
	}
}


static void HandleModAlt(UINT32 const key)
{
	switch (key)
	{
		case 'a':
			if (giHighLine != -1)
			{
				if (gCharactersList[giHighLine].merc)
				{
					bSelectedAssignChar = (INT8)giHighLine;
					RebuildAssignmentsBox();
					ChangeSelectedInfoChar((INT8)giHighLine, FALSE);
					fShowAssignmentMenu = TRUE;
				}
			}
			else if (GetSelectedInfoChar())
			{
				bSelectedAssignChar = bSelectedInfoChar;
				RebuildAssignmentsBox();
				fShowAssignmentMenu = TRUE;
			}
			break;

		case 'f':
			if (INFORMATION_CHEAT_LEVEL())
			{ // Toggle Frame Rate Display
				gbFPSDisplay = !gbFPSDisplay;
				EnableFPSOverlay(gbFPSDisplay);
			}
			break;

		case 'l':
			// Although we're not actually going anywhere, we must still be in a state where this is permitted
			if (AllowedToExitFromMapscreenTo(MAP_EXIT_TO_LOAD)) DoQuickLoad();
			break;

		case 's':
			// although we're not actually going anywhere, we must still be in a state where this is permitted
			if (AllowedToExitFromMapscreenTo(MAP_EXIT_TO_SAVE))
			{
				//if the game CAN be saved
				if (CanGameBeSaved())
				{
					guiPreviousOptionScreen = guiCurrentScreen;
					DoQuickSave();
				}
				else
				{
					//Display a message saying the player cant save now
					DoMapMessageBox(MSG_BOX_BASIC_STYLE, zNewTacticalMessages[TCTL_MSG__IRON_MAN_CANT_SAVE_NOW], MAP_SCREEN, MSG_BOX_FLAG_OK, 0);
				}
			}
			break;

		case 'x': HandleShortCutExitState(); break;
	}
}


static void GetMapKeyboardInput(MapEvent& new_event)
{
	InputAtom InputEvent;
	while (DequeueEvent(&InputEvent))
	{
		SGPPoint MousePos;
		GetMousePos(&MousePos);
		MouseSystemHook(InputEvent.usEvent, MousePos.iX, MousePos.iY);

		if (InputEvent.usEvent == KEY_DOWN)
		{
			// if game is paused because of player, unpause with any key
			if (gfPauseDueToPlayerGamePause)
			{
				HandlePlayerPauseUnPauseOfGame();
				continue;
			}

			// handle for fast help text for interface stuff
			if (IsTheInterfaceFastHelpTextActive())
			{
				ShutDownUserDefineHelpTextRegions();
			}

			UINT32 const key = InputEvent.usParam;
			switch (InputEvent.usKeyState)
			{
				case 0:          HandleModNone( key, new_event); break;
				case SHIFT_DOWN: HandleModShift(key);            break;
				case CTRL_DOWN:  HandleModCtrl( key);            break;
				case ALT_DOWN:   HandleModAlt(  key);            break;
			}
		}
		else if (InputEvent.usEvent == KEY_REPEAT)
		{
			switch (InputEvent.usParam)
			{
				case SDLK_LEFT:  GoToPrevCharacterInList(); break;
				case SDLK_RIGHT: GoToNextCharacterInList(); break;

				case SDLK_UP:   MapScreenMsgScrollUp(1);   break;
				case SDLK_DOWN: MapScreenMsgScrollDown(1); break;

				case SDLK_PAGEUP:   MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);   break;
				case SDLK_PAGEDOWN: MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM); break;
			}
		}
	}
}


static void DestroyMouseRegionsForTeamList(void);
static void RemoveTeamPanelSortButtonsForMapScreen();


void EndMapScreen( BOOLEAN fDuringFade )
{
	if (!fInMapMode) return;

	fLeavingMapScreen = FALSE;

	SetRenderFlags( RENDER_FLAG_FULL );
	//gViewportRegion.Enable();
	//gRadarRegion.Enable();
	// ATE: Shutdown tactical interface panel
//	ShutdownCurrentPanel( );

	// still plotting movement?
	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}

	DestroyMouseRegionsForTeamList( );

	MSYS_RemoveRegion( &gMapViewRegion );
	MSYS_RemoveRegion( &gCharInfoFaceRegion);
	MSYS_RemoveRegion( &gCharInfoHandRegion );
	MSYS_RemoveRegion( &gMPanelRegion);
	MSYS_RemoveRegion( &gMapScreenMaskRegion );
	fInMapMode = FALSE;

	// remove team panel sort button
	RemoveTeamPanelSortButtonsForMapScreen( );

	// for th merc insurance help text
	CreateDestroyInsuranceMouseRegionForMercs( FALSE );

	// gonna need to remove the screen mask regions
	CreateDestroyMouseRegionMasksForTimeCompressionButtons( );

	RemoveButton( giMapContractButton );

	HandleShutDownOfMapScreenWhileExternfaceIsTalking( );

	fShowInventoryFlag = FALSE;
	CreateDestroyMapInvButton();

	// no longer can we show assignments menu
	fShowAssignmentMenu = FALSE;

	// clear out mouse regions for pop up boxes
	DetermineWhichAssignmentMenusCanBeShown( );

	sSelectedMilitiaTown = 0;
	CreateDestroyMilitiaPopUPRegions( );
	CreateDestroyMilitiaSectorButtons( );

	// stop showing contract menu
	fShowContractMenu = FALSE;
	// clear out contract menu
	DetermineIfContractMenuCanBeShown( );
	// remove contract pop up box (always created upon mapscreen entry)
	RemoveBox(ghContractBox);
	ghContractBox = NO_POPUP_BOX;

	CreateDestroyAssignmentPopUpBoxes( );

	// shutdown movement box
	if ( fShowMapScreenMovementList )
	{
		fShowMapScreenMovementList = FALSE;
		CreateDestroyMovementBox( 0, 0, 0 );
	}

	// the remove merc from team box
	RemoveBox( ghRemoveMercAssignBox );
	ghRemoveMercAssignBox = NO_POPUP_BOX;

	// clear screen mask if needed
	ClearScreenMaskForMapScreenExit( );

	// get rid of pause clock area
	RemoveMouseRegionForPauseOfClock( );

	// get rid of pop up for town info, if being shown
	fShowTownInfo = FALSE;
	CreateDestroyTownInfoBox( );

	// build squad list
	RebuildCurrentSquad( );

		//
	DeleteMouseRegionsForLevelMarkers( );

	if (!fShowMapInventoryPool)
	{
		// delete buttons
		DeleteMapBorderButtons( );
	}

	if(fShowDescriptionFlag)
	{
		DeleteItemDescriptionBox( );
	}


	fShowInventoryFlag = FALSE;
	CreateDestroyTrashCanRegion( );

	if ( !fDuringFade )
	{
		MSYS_SetCurrentCursor(SCREEN_CURSOR);
	}

	RemoveMapStatusBarsRegion( );

	fShowUpdateBox = FALSE;
	CreateDestroyTheUpdateBox( );


	// get rid of mapscreen bottom
	DeleteMapScreenInterfaceBottom( );


	// shutdown any mapscreen UI overlay message
	CancelMapUIMessage( );


	CreateDestroyMapCharacterScrollButtons( );

	// if time was ever compressed while we were in mapscreen
	if ( HasTimeCompressOccured( ) )
	{
		// make sure everything tactical got cleared out
		ClearTacticalStuffDueToTimeCompression();
	}


	CancelSectorInventoryDisplayIfOn( TRUE );


	SetAllAutoFacesInactive( );
	if(fLapTop)
	{
		StopAnyCurrentlyTalkingSpeech( );
		guiCurrentScreen=LAPTOP_SCREEN;
	}
	else
	{
		guiCurrentScreen = GAME_SCREEN;

		// remove the progress bar
		RemoveProgressBar( 0 );

		// enable scroll string video overlays
		EnableDisableScrollStringVideoOverlay( TRUE );
	}

	// if going to tactical next
	if ( guiPendingScreen == GAME_SCREEN )
	{
		// set compressed mode to Normal (X1)
		SetGameTimeCompressionLevel( TIME_COMPRESS_X1 );
	}
	else	// going to another screen (options, laptop, save/load)
	{
		StopTimeCompression();
	}

	// update paused states, we are exiting...need to reset for any pathing or menus displayed
	UnLockPauseState( );
	UpdatePausedStatesDueToTimeCompression( );

	if( !gfDontStartTransitionFromLaptop )
	{
		//Load a tiny graphic of the on screen and draw it to the buffer.
		PlayJA2SampleFromFile(SOUNDSDIR "/initial power up (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
		BltVideoObjectOnce(FRAME_BUFFER, INTERFACEDIR "/laptopon.sti", 0, 465, 417);
		InvalidateRegion( 465, 417, 480, 427 );
		ExecuteBaseDirtyRectQueue( );
		EndFrameBufferRender( );
		RefreshScreen();
	}

	//Kris:  Removes the pre battle interface, but only if it exists.
	//       It is internally considered.
	KillPreBattleInterface();

	// cancel request if we somehow leave first
	gfRequestGiveSkyriderNewDestination = FALSE;
}


static BOOLEAN GetMapXY(INT16 sX, INT16 sY, SGPSector& sMap);


BOOLEAN GetMouseMapXY(SGPSector& sMap)
{
	SGPPoint MousePos;
	GetMousePos(&MousePos);

	return GetMapXY(MousePos.iX, MousePos.iY, sMap);
}


static BOOLEAN GetMapXY(INT16 sX, INT16 sY, SGPSector& sMap)
{
	// Subtract start of map view
	INT16 sMapX = sX - MAP_VIEW_START_X;//+2*MAP_GRID_X;
	INT16 sMapY = sY - MAP_VIEW_START_Y;

	if ( sMapX < MAP_GRID_X || sMapY < MAP_GRID_Y )
	{
		return( FALSE );
	}

	if ( sMapX < 0 || sMapY < 0 )
	{
		return( FALSE );
	}

	if ( sMapX > MAP_VIEW_WIDTH+MAP_GRID_X-1 || sMapY > MAP_VIEW_HEIGHT+7/* +MAP_VIEW_HEIGHT */ )
	{
		return( FALSE );
	}
	if(sMapX < 1 || sMapY <1)
	{
		return (FALSE);
	}

	sMap.x = sMapX / MAP_GRID_X;
	sMap.y = sMapY / MAP_GRID_Y;

	return( TRUE );
}


static void RenderMapHighlight(const SGPSector& sMap, UINT16 usLineColor, BOOLEAN fStationary)
{
	Assert(sMap.IsValid());

	// if we are not allowed to highlight, leave
	if (!IsTheCursorAllowedToHighLightThisSector(sMap))
	{
		return;
	}

	INT16 sScreenX, sScreenY;
	GetScreenXYFromMapXY(sMap, &sScreenX, &sScreenY);

	// blit in the highlighted sector
	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf         = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	// clip to view region
	ClipBlitsToMapViewRegionForRectangleAndABit( uiDestPitchBYTES );

	// draw rectangle for zoom out
	RectangleDraw( TRUE, sScreenX, sScreenY - 1, sScreenX +  MAP_GRID_X, sScreenY +  MAP_GRID_Y - 1, usLineColor, pDestBuf );
	InvalidateRegion(    sScreenX, sScreenY - 2, sScreenX + DMAP_GRID_X + 1, sScreenY + DMAP_GRID_Y - 1 );

	RestoreClipRegionToFullScreenForRectangle( uiDestPitchBYTES );
}


static BOOLEAN CheckIfClickOnLastSectorInPath(const SGPSector& sector);
static void DestinationPlottingCompleted(void);


static void PollLeftButtonInMapView(MapEvent& new_event)
{
	static BOOLEAN	fLBBeenPressedInMapView = FALSE;

	// if the mouse is currently over the MAP area
	if ( gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		// if L-button is down at the moment
		if ( gMapViewRegion.ButtonState & MSYS_LEFT_BUTTON )
		{
			if ( !fLBBeenPressedInMapView )
			{
				fLBBeenPressedInMapView = TRUE;
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );
			}
		}
		else	// L-button is NOT down at the moment
		{
			if ( fLBBeenPressedInMapView  )
			{
				fLBBeenPressedInMapView = FALSE;
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

				// if in militia redistribution popup
				if ( sSelectedMilitiaTown != 0 )
				{
					// ignore clicks outside the box
					return;
				}

				// left click cancels MAP UI messages, unless we're in confirm map move mode
				if (g_ui_message_overlay != NULL && !gfInConfirmMapMoveMode)
				{
					CancelMapUIMessage( );

					// return unless moving the bullseye
					if ( !gfInChangeArrivalSectorMode )
						return;
				}

				// ignore left clicks in the map screen if:
				// game just started or we're in the prebattle interface or if we are about to hit pre-battle
				if (DidGameJustStart()         ||
						gfPreBattleInterfaceActive ||
						fDisableMapInterfaceDueToBattle)
				{
					return;
				}

				// if in "plot route" mode
				if (bSelectedDestChar != -1 || fPlotForHelicopter)
				{
					fEndPlotting = FALSE;

					SGPSector sMap;
					GetMouseMapXY(sMap);

					// if he clicked on the last sector in his current path
					if (CheckIfClickOnLastSectorInPath(sMap))
					{
						DestinationPlottingCompleted();
					}
					else	// clicked on a new sector
					{
						// draw new map route
						new_event = MAP_EVENT_PLOT_PATH;
					}
				}
				else	// not plotting movement
				{
					// if not plotting a path
					if (!fEndPlotting && !fJustFinishedPlotting)
					{
						// make this sector selected / trigger movement box / start helicopter plotting / changing arrival sector
						new_event = MAP_EVENT_CLICK_SECTOR;
					}

					fEndPlotting = FALSE;
				}

				// reset town info flag
				fShowTownInfo = FALSE;
			}
		}
	}

	fJustFinishedPlotting = FALSE;
}


static void HandleMilitiaRedistributionClick(void);


static void PollRightButtonInMapView(MapEvent& new_event)
{
	static BOOLEAN	fRBBeenPressedInMapView = FALSE;

	// if the mouse is currently over the MAP area
	if ( gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		// if R-button is down at the moment
		if ( gMapViewRegion.ButtonState & MSYS_RIGHT_BUTTON )
		{
			if ( !fRBBeenPressedInMapView )
			{
				fRBBeenPressedInMapView = TRUE;
				RESETCOUNTER( RMOUSECLICK_DELAY_COUNTER );
			}
		}
		else	// R-button is NOT down at the moment
		{
			if ( fRBBeenPressedInMapView )
			{
				fRBBeenPressedInMapView = FALSE;
				RESETCOUNTER( RMOUSECLICK_DELAY_COUNTER );

				// if in militia redistribution popup
				if ( sSelectedMilitiaTown != 0 )
				{
					// ignore clicks outside the box
					return;
				}

				if ( gfInChangeArrivalSectorMode )
				{
					CancelChangeArrivalSectorMode( );
					BeginMapUIMessage(0, pBullseyeStrings[3]);
					return;
				}

				// right click cancels MAP UI messages, unless we're in confirm map move mode
				if (g_ui_message_overlay != NULL && !gfInConfirmMapMoveMode)
				{
					CancelMapUIMessage( );
					return;
				}

				// ignore right clicks in the map area if:
				// game just started or we're in the prebattle interface or if we are about to hit pre-battle
				if (DidGameJustStart()         ||
						gfPreBattleInterfaceActive ||
						fDisableMapInterfaceDueToBattle)
				{
					return;
				}


				if (bSelectedDestChar != -1 || fPlotForHelicopter)
				{
					// cancel/shorten the path
					new_event = MAP_EVENT_CANCEL_PATH;
				}
				else
				{
					SGPSector sMap(1, 1, iCurrentMapSectorZ);
					if (GetMouseMapXY(sMap))
					{
						if (sSelMap != sMap)
						{
							ChangeSelectedMapSector(sMap);
						}
					}

					// sector must be selected to bring up militia or town info boxes for it
					if (sMap == sSelMap)
					{
						if (fShowMilitia)
						{
							HandleMilitiaRedistributionClick();
						}
						else // show militia is OFF
						{
							// if on the surface, or a real underground sector (we've visited it)
							if (sMap.z == 0 || GetSectorFlagStatus(sMap, SF_ALREADY_VISITED))
							{
								// toggle sector info for this sector
								fShowTownInfo = !fShowTownInfo;
								fMapPanelDirty = TRUE;
							}
						}

//						fMapScreenBottomDirty = TRUE;

						CreateDestroyScreenMaskForAssignmentAndContractMenus( );
						if (!fShowTownInfo)
						{
							// destroy town info box
							CreateDestroyTownInfoBox( );
						}
					}
				}
			}
		}
	}
}


static void MAPInvClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MAPInvClickCamoCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MAPInvMoveCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void MAPInvMoveCamoCallback(MOUSE_REGION* pRegion, INT32 iReason);


void CreateDestroyMapInvButton()
{
	static BOOLEAN fOldShowInventoryFlag=FALSE;

	if( fShowInventoryFlag && !fOldShowInventoryFlag )
	{
		// create inventory button
		fOldShowInventoryFlag=TRUE;
		// disable allmouse regions in this space
		fTeamPanelDirty=TRUE;

		INV_REGION_DESC gSCamoXY = {INV_BODY_X, INV_BODY_Y};

		InitInvSlotInterface(g_ui.m_invSlotPositionMap, &gSCamoXY, MAPInvMoveCallback, MAPInvClickCallback, MAPInvMoveCamoCallback, MAPInvClickCamoCallback);
		gMPanelRegion.Enable();

		// switch hand region help text to "Exit Inventory"
		gCharInfoHandRegion.SetFastHelpText(pMiscMapScreenMouseRegionHelpText[2]);

		// dirty character info panel  ( Why? ARM )
		fCharacterInfoPanelDirty=TRUE;
	}
	else if( !fShowInventoryFlag && fOldShowInventoryFlag )
	{
		// destroy inventory button
		ShutdownInvSlotInterface( );
		fOldShowInventoryFlag=FALSE;
		fTeamPanelDirty=TRUE;
		gMPanelRegion.Disable();

		// switch hand region help text to "Enter Inventory"
		gCharInfoHandRegion.SetFastHelpText(pMiscMapScreenMouseRegionHelpText[0]);

		// force immediate reblit of item in HANDPOS now that it's not blitted while in inventory mode
		fCharacterInfoPanelDirty=TRUE;
	}
}


static void BltCharInvPanel(void)
{
	ST::string sString;

	const SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	Assert(pSoldier);
	Assert(MapCharacterHasAccessibleInventory(*pSoldier));

	BltVideoObject(guiSAVEBUFFER, guiMAPINV, 0, PLAYER_INFO_X, PLAYER_INFO_Y);

	CreateDestroyMapInvButton();

	if( gbCheckForMouseOverItemPos != -1 )
	{
		if (HandleCompatibleAmmoUIForMapScreen(pSoldier, gbCheckForMouseOverItemPos, TRUE, TRUE))
		{
			fMapPanelDirty = TRUE;
		}
	}

	if( ( fShowMapInventoryPool ) )
	{
		if( iCurrentlyHighLightedItem != -1 )
		{
			HandleCompatibleAmmoUIForMapScreen( pSoldier, (INT32)( iCurrentlyHighLightedItem + ( iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT ) ) , TRUE, FALSE );
		}
	}

	RenderInvBodyPanel(pSoldier, INV_BODY_X, INV_BODY_Y );

	// reset font destination buffer to the save buffer
	SetFontDestBuffer(guiSAVEBUFFER);

	// render items in each of chars slots
	HandleRenderInvSlots(*pSoldier, DIRTYLEVEL2);

	// Render Values for stats!
	SetFontAttributes(BLOCKFONT2, MAP_INV_STATS_TITLE_FONT_COLOR);

	INT16 usX;
	INT16 usY;

	// Display armor
	MPrint(MAP_ARMOR_LABEL_X, MAP_ARMOR_LABEL_Y, pInvPanelTitleStrings[0]);
	sString = ST::format("{3d}%", ArmourPercent(pSoldier));
	FindFontRightCoordinates(MAP_ARMOR_X, MAP_ARMOR_Y, MAP_ARMOR_W, MAP_ARMOR_H, sString, BLOCKFONT2, &usX, &usY);
	MPrint(usX, usY, sString);

	// Display weight
	MPrint(MAP_WEIGHT_LABEL_X, MAP_WEIGHT_LABEL_Y, pInvPanelTitleStrings[1]);
	sString = ST::format("{}%", CalculateCarriedWeight(pSoldier));
	FindFontRightCoordinates(MAP_WEIGHT_X, MAP_WEIGHT_Y, MAP_WEIGHT_W, MAP_WEIGHT_H, sString, BLOCKFONT2, &usX, &usY);
	MPrint(usX, usY, sString);

	// Display camouflage
	MPrint(MAP_CAMO_LABEL_X, MAP_CAMO_LABEL_Y, pInvPanelTitleStrings[2]);
	sString = ST::format("{}%", pSoldier->bCamo);
	FindFontRightCoordinates(MAP_CAMO_X, MAP_CAMO_Y, MAP_CAMO_W, MAP_CAMO_H, sString, BLOCKFONT2, &usX, &usY);
	MPrint(usX, usY, sString);

	if( InKeyRingPopup() || InItemStackPopup() )
	{
		// shade the background
		guiSAVEBUFFER->ShadowRect(PLAYER_INFO_X, PLAYER_INFO_Y, PLAYER_INFO_X + 261,  PLAYER_INFO_Y + (359 - 107));
	}
	else
	{
		// blit gold key on top of key ring if key ring is not empty
	}

	SetFontDestBuffer(FRAME_BUFFER);
}


// check for and highlight any ammo
static void HandleCursorOverRifleAmmo(void)
{
	if (!fShowInventoryFlag) return;

	// make sure sector inventory is updated if visible
	if( fShowMapInventoryPool )	{
		if(( GetJA2Clock() - guiMouseOverItemTime ) > 100 ) {
			fMapPanelDirty = TRUE;
		}
	}

	if( gbCheckForMouseOverItemPos == -1 )
	{
		return;
	}

	if ( gfCheckForMouseOverItem )
	{
		if (HandleCompatibleAmmoUI(GetSelectedInfoChar(), gbCheckForMouseOverItemPos, TRUE))
		{
			if ( ( GetJA2Clock( ) - guiMouseOverItemTime ) > 100 )
			{
				fTeamPanelDirty = TRUE;
			}
		}

		// also highlight in sector inventory
		if( fShowMapInventoryPool )	{
			HandleCompatibleAmmoUIForMapInventory( GetSelectedInfoChar(), gbCheckForMouseOverItemPos, ( iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT ), TRUE, TRUE );
		}
	}
}


static void MAPInvClickCamoCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
}


static void MAPInvMoveCamoCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
}


// this is Map Screen's version of SMInvMoveCallback()
static void MAPInvMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32 uiHandPos;

	const SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	Assert(MapCharacterHasAccessibleInventory(*pSoldier));

	uiHandPos = MSYS_GetRegionUserData( pRegion, 0 );

	//gbCheckForMouseOverItemPos = -1;

	if ( pSoldier->inv[ uiHandPos ].usItem == NOTHING )
		return;

	if (iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE )
	//if( ( iReason == MSYS_CALLBACK_REASON_MOVE ) || ( iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE ) )
	{
		guiMouseOverItemTime = GetJA2Clock( );
		gfCheckForMouseOverItem = TRUE;
		HandleCompatibleAmmoUI( pSoldier, (INT8)uiHandPos, FALSE );
		gbCheckForMouseOverItemPos = (INT8)uiHandPos;
	}
	if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		HandleCompatibleAmmoUI( pSoldier, (INT8)uiHandPos, FALSE );
		gfCheckForMouseOverItem = FALSE;
		fTeamPanelDirty = TRUE;
		gbCheckForMouseOverItemPos = -1;
	}
}


void MAPInternalInitItemDescriptionBox(OBJECTTYPE* pObject, UINT8 ubStatusIndex, SOLDIERTYPE* pSoldier)
{
	InternalInitItemDescriptionBox(pObject, MAP_ITEMDESC_START_X, MAP_ITEMDESC_START_Y, ubStatusIndex, pSoldier);

	fShowDescriptionFlag=TRUE;
	fTeamPanelDirty=TRUE;
	fInterfacePanelDirty = DIRTYLEVEL2;
}


static void MAPBeginItemPointer(SOLDIERTYPE* pSoldier, UINT8 ubHandPos);


// this is Map Screen's version of SMInvClickCallback()
static void MAPInvClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32 uiHandPos;
	UINT16	usOldItemIndex, usNewItemIndex;
	static BOOLEAN	fRightDown = FALSE;

	SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	Assert(MapCharacterHasAccessibleInventory(*pSoldier));

	uiHandPos = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// If we do not have an item in hand, start moving it
		if ( gpItemPointer == NULL )
		{
			// Return if empty
			if ( pSoldier->inv[ uiHandPos ].usItem == NOTHING )
			{
				return;
			}

			//ATE: Put this here to handle Nails refusal....
			if ( HandleNailsVestFetish( pSoldier, uiHandPos, NOTHING ) )
			{
				return;
			}

			if ( _KeyDown(CTRL) )
			{
				CleanUpStack( &( pSoldier->inv[ uiHandPos ] ), NULL );
			}

			// remember what it was
			usOldItemIndex = pSoldier->inv[ uiHandPos ].usItem;

			// pick it up
			MAPBeginItemPointer( pSoldier, (UINT8)uiHandPos );

			// remember which gridno the object came from
			sObjectSourceGridNo = pSoldier->sGridNo;

			HandleTacticalEffectsOfEquipmentChange( pSoldier, uiHandPos, usOldItemIndex, NOTHING );

			fInterfacePanelDirty = DIRTYLEVEL2;
			fCharacterInfoPanelDirty = TRUE;
		}
		else	// item in cursor
		{
			// can we pass this part due to booby traps
			if (!ContinuePastBoobyTrapInMapScreen(gpItemPointer, pSoldier))
			{
				return;
			}

			usOldItemIndex = pSoldier->inv[ uiHandPos ].usItem;
			usNewItemIndex = gpItemPointer->usItem;

			//ATE: Put this here to handle Nails refusal....
			if ( HandleNailsVestFetish( pSoldier, uiHandPos, usNewItemIndex ) )
			{
				return;
			}

			if ( _KeyDown(CTRL) )
			{
				CleanUpStack( &( pSoldier->inv[ uiHandPos ] ), gpItemPointer );
				if ( gpItemPointer->ubNumberOfObjects == 0 )
				{
					MAPEndItemPointer( );
				}
				return;
			}

			// !!! ATTACHING/MERGING ITEMS IN MAP SCREEN IS NOT SUPPORTED !!!
			if ( uiHandPos == HANDPOS || uiHandPos == SECONDHANDPOS || uiHandPos == HELMETPOS || uiHandPos == VESTPOS || uiHandPos == LEGPOS )
			{
				//if ( ValidAttachmentClass( usNewItemIndex, usOldItemIndex ) )
				if ( ValidAttachment( usNewItemIndex, usOldItemIndex ) )
				{
					// it's an attempt to attach; bring up the inventory panel
					if ( !InItemDescriptionBox( ) )
					{
						MAPInternalInitItemDescriptionBox( &(pSoldier->inv[ uiHandPos ]), 0, pSoldier );
					}
					return;
				}
				else if ( ValidMerge( usNewItemIndex, usOldItemIndex ) )
				{
					// bring up merge requestor
					// TOO PAINFUL TO DO!! --CC
					if ( !InItemDescriptionBox( ) )
					{
						MAPInternalInitItemDescriptionBox( &(pSoldier->inv[ uiHandPos ]), 0, pSoldier );
					}

					/*
					gubHandPos = (UINT8) uiHandPos;
					gusOldItemIndex = usOldItemIndex;
					gusNewItemIndex = usNewItemIndex;
					gfDeductPoints = fDeductPoints;

					DoScreenIndependantMessageBox( Message[ STR_MERGE_ITEMS ], MSG_BOX_FLAG_YESNO, MergeMessageBoxCallBack );
					return;
					*/
				}
				// else handle normally
			}

			// Else, try to place here
			if ( PlaceObject( pSoldier, (UINT8)uiHandPos, gpItemPointer ) )
			{

				HandleTacticalEffectsOfEquipmentChange( pSoldier, uiHandPos, usOldItemIndex, usNewItemIndex );

				// Dirty
				fInterfacePanelDirty = DIRTYLEVEL2;
				fCharacterInfoPanelDirty = TRUE;
				fMapPanelDirty = TRUE;

				// Check if cursor is empty now
				if ( gpItemPointer->ubNumberOfObjects == 0 )
				{
					MAPEndItemPointer( );
				}
				else	// items swapped
				{
					SetMapCursorItem();
					fTeamPanelDirty=TRUE;

					// remember which gridno the object came from
					sObjectSourceGridNo = pSoldier->sGridNo;
					// and who owned it last
					gpItemPointerSoldier = pSoldier;

					ReevaluateItemHatches( pSoldier, FALSE );
				}

				// re-evaluate repairs
				gfReEvaluateEveryonesNothingToDo = TRUE;

				// if item came from another merc
				if ( gpItemPointerSoldier != pSoldier )
				{
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, st_format_printf(pMessageStrings[ MSG_ITEM_PASSED_TO_MERC ], ShortItemNames[ usNewItemIndex ], pSoldier->name) );
				}

			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		// if there is a map UI message being displayed
		if (g_ui_message_overlay != NULL)
		{
			CancelMapUIMessage( );
			return;
		}

		fRightDown = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP && fRightDown )
	{
		fRightDown = FALSE;

		// Return if empty
		if (pSoldier->inv[ uiHandPos ].usItem == NOTHING )
		{
			return;
		}

		// Some global stuff here - for esc, etc
		// Check for # of slots in item
		if ( ( pSoldier->inv[ uiHandPos ].ubNumberOfObjects > 1 ) && ( ItemSlotLimit( pSoldier->inv[ uiHandPos ].usItem, (UINT8)uiHandPos ) > 0 ) )
		{
			if ( !InItemStackPopup( ) )
			{
				InitItemStackPopup( pSoldier, (UINT8)uiHandPos, INV_REGION_X, INV_REGION_Y, 261, 248 );
				fTeamPanelDirty=TRUE;
				fInterfacePanelDirty = DIRTYLEVEL2;
			}
		}
		else
		{
			if ( !InItemDescriptionBox( ) )
			{
				InitItemDescriptionBox( pSoldier, (UINT8)uiHandPos, MAP_ITEMDESC_START_X, MAP_ITEMDESC_START_Y, 0 );
				fShowDescriptionFlag=TRUE;
				fTeamPanelDirty=TRUE;
				fInterfacePanelDirty = DIRTYLEVEL2;
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fRightDown = FALSE;
	}
}


void InternalMAPBeginItemPointer(SOLDIERTYPE* pSoldier)
{
	// If not null return
	if ( gpItemPointer != NULL )
	{
		return;
	}

	SetItemPointer(&gItemPointer, pSoldier);
	SetMapCursorItem();
	fTeamPanelDirty=TRUE;

	// hatch out incompatible inventory slots
	ReevaluateItemHatches( pSoldier, FALSE );

	// re-evaluate repairs
	gfReEvaluateEveryonesNothingToDo = TRUE;
}


static void MAPBeginItemPointer(SOLDIERTYPE* pSoldier, UINT8 ubHandPos)
{
	BOOLEAN	fOk;

	// If not null return
	if ( gpItemPointer != NULL )
	{
		return;
	}

	if (_KeyDown( SHIFT ))
	{
		// Remove all from soldier's slot
		fOk = RemoveObjectFromSlot( pSoldier, ubHandPos, &gItemPointer );
	}
	else
	{
		GetObjFrom( &(pSoldier->inv[ubHandPos]), 0, &gItemPointer );
		fOk = (gItemPointer.ubNumberOfObjects == 1);
	}

	if ( fOk )
	{
		InternalMAPBeginItemPointer( pSoldier );
	}
}


void MAPEndItemPointer(void)
{
	if ( gpItemPointer != NULL )
	{
		gpItemPointer = NULL;
		gMPanelRegion.ChangeCursor(CURSOR_NORMAL);
		MSYS_SetCurrentCursor( CURSOR_NORMAL );
		fMapInventoryItem=FALSE;
		fTeamPanelDirty=TRUE;

		if ( fShowMapInventoryPool )
		{
			HandleButtonStatesWhileMapInventoryActive();
		}

		if (fShowInventoryFlag)
		{
			SOLDIERTYPE* const s = GetSelectedInfoChar();
			if (s != NULL) ReevaluateItemHatches(s, FALSE);
		}
	}
}


// handle cursor for invenetory mode..update to object selected, if needed
static void HandleMapInventoryCursor(void)
{
	if(fMapInventoryItem)
		MSYS_SetCurrentCursor( EXTERN_CURSOR );
}


// will place down the upper left hand corner attribute strings
static void RenderAttributeStringsForUpperLeftHandCorner(SGPVSurface* const uiBufferToRenderTo)
{
	INT32 iCounter = 0;

	SetFontAttributes(CHAR_FONT, CHAR_TITLE_FONT_COLOR);
	SetFontDestBuffer(uiBufferToRenderTo);

	// assignment strings
	DrawString(pUpperLeftMapScreenStrings[0], STD_SCREEN_X + 220 - StringPixLength(pUpperLeftMapScreenStrings[0], CHAR_FONT) / 2, STD_SCREEN_Y + 6, CHAR_FONT);

	// vehicles and robot don't have attributes, contracts, or morale
	const SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	if (!pSoldier || !IsMechanical(*pSoldier))
	{
		// health
		DrawString(pUpperLeftMapScreenStrings[1], STD_SCREEN_X + 87, STD_SCREEN_Y + 80, CHAR_FONT);

		for( iCounter = 0; iCounter < 5; iCounter++ )
		{
			DrawString(pShortAttributeStrings[iCounter],     STD_SCREEN_X +  88, STD_SCREEN_Y + 22 + iCounter * 10, CHAR_FONT);
			DrawString(pShortAttributeStrings[iCounter + 5], STD_SCREEN_X + 133, STD_SCREEN_Y + 22 + iCounter * 10, CHAR_FONT);
		}

		// morale
		DrawString(pUpperLeftMapScreenStrings[2], STD_SCREEN_X + 87, STD_SCREEN_Y + 94,  CHAR_FONT);
	}
	else
	{
		// condition
		DrawString(pUpperLeftMapScreenStrings[3], STD_SCREEN_X + 87, STD_SCREEN_Y + 80, CHAR_FONT);
	}


	// restore buffer
	SetFontDestBuffer(FRAME_BUFFER);
}


static void DisplayThePotentialPathForCurrentDestinationCharacterForMapScreenInterface(const SGPSector& sMap)
{
	// simply check if we want to refresh the screen to display path
	static INT8 bOldDestChar = -1;
	static SGPSector sPrevMap;

	if( bOldDestChar != bSelectedDestChar )
	{
		bOldDestChar = bSelectedDestChar;
		guiPotCharPathBaseTime = GetJA2Clock( );

		sPrevMap = sMap;
		fTempPathAlreadyDrawn = FALSE;
		fDrawTempPath = FALSE;
	}

	if (sMap != sPrevMap)
	{
		guiPotCharPathBaseTime = GetJA2Clock( );
		sPrevMap = sMap;

		// path was plotted and we moved, re draw map..to clean up mess
		if (fTempPathAlreadyDrawn) fMapPanelDirty = TRUE;

		fTempPathAlreadyDrawn = FALSE;
		fDrawTempPath = FALSE;
	}

	UINT32 uiDifference = GetJA2Clock() - guiPotCharPathBaseTime;

	if (fTempPathAlreadyDrawn) return;

	if (uiDifference > MIN_WAIT_TIME_FOR_TEMP_PATH)
	{
		fDrawTempPath = TRUE;
		guiPotCharPathBaseTime = GetJA2Clock( );
		fTempPathAlreadyDrawn = TRUE;
	}
}


static void ChangeMapScreenMaskCursor(UINT16 usCursor);


void SetUpCursorForStrategicMap(void)
{
	if (!gfInChangeArrivalSectorMode)
	{
		// check if character is in destination plotting mode
		if (!fPlotForHelicopter)
		{
			if( bSelectedDestChar == -1 )
			{
				// no plot mode, reset cursor to normal
				ChangeMapScreenMaskCursor( CURSOR_NORMAL );
			}
			else	// yes - by character
			{
				// set cursor based on foot or vehicle
				const SOLDIERTYPE* const s = gCharactersList[bSelectedDestChar].merc;
				if (s->bAssignment != VEHICLE && !(s->uiStatusFlags & SOLDIER_VEHICLE))
				{
					ChangeMapScreenMaskCursor( CURSOR_STRATEGIC_FOOT );
				}
				else
				{
					ChangeMapScreenMaskCursor( CURSOR_STRATEGIC_VEHICLE );
				}
			}
		}
		else	// yes - by helicopter
		{
			// set cursor to chopper
			ChangeMapScreenMaskCursor( CURSOR_CHOPPER );
		}
	}
	else
	{
		// set cursor to bullseye
		ChangeMapScreenMaskCursor( CURSOR_STRATEGIC_BULLSEYE );
	}
}


static void HandleAnimatedCursorsForMapScreen(void)
{
	if ( COUNTERDONE( CURSORCOUNTER ) )
	{
		RESETCOUNTER( CURSORCOUNTER );
		UpdateAnimatedCursorFrames( gMapScreenMaskRegion.Cursor );
		SetCurrentCursorFromDatabase(  gMapScreenMaskRegion.Cursor  );
	}
}


static void ClearPreviousPaths(void);
static void RestorePreviousPaths(void);


void AbortMovementPlottingMode( void )
{
	// invalid if we're not plotting movement
	Assert(bSelectedDestChar != -1 || fPlotForHelicopter);

	// make everybody go back to where they were going before this plotting session started
	RestorePreviousPaths();

	// don't need the previous paths any more
	ClearPreviousPaths();

	// clear the character's temporary path (this is the route being constantly updated on the map)
	pTempCharacterPath = ClearStrategicPathList(pTempCharacterPath, 0);

	// clear the helicopter's temporary path (this is the route being constantly updated on the map)
	pTempHelicopterPath = ClearStrategicPathList(pTempHelicopterPath, 0);

	EndConfirmMapMoveMode( );

	// cancel destination line highlight
	giDestHighLine = -1;

	// cancel movement mode
	bSelectedDestChar = -1;
	fPlotForHelicopter = FALSE;

	// tell player the route was UNCHANGED
	BeginMapUIMessage(0, pMapPlotStrings[2]);

	// reset cursors
	ChangeMapScreenMaskCursor( CURSOR_NORMAL );
	SetUpCursorForStrategicMap( );

	// restore glow region
	RestoreBackgroundForDestinationGlowRegionList( );

	// we might be on the map, redraw to remove old path stuff
	fMapPanelDirty = TRUE;
	fTeamPanelDirty = TRUE;

	gfRenderPBInterface = TRUE;
}


static void CheckToSeeIfMouseHasLeftMapRegionDuringPathPlotting(void)
{
	static BOOLEAN fInArea = FALSE;

	if ( ( gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA ) == 0 )
	{
		if (fInArea)
		{
			fInArea = FALSE;

			// plotting path, clean up
			if (fTempPathAlreadyDrawn &&
				(
					bSelectedDestChar != -1 ||
					fPlotForHelicopter      ||
					fDrawTempHeliPath
				))
			{
				fDrawTempHeliPath = FALSE;
				fMapPanelDirty = TRUE;
				gfRenderPBInterface = TRUE;

				// clear the temp path
				pTempCharacterPath = ClearStrategicPathList(pTempCharacterPath, 0);
			}

			// reset fact temp path has been drawn
			fTempPathAlreadyDrawn = FALSE;
		}
	}
	else
	{
		fInArea = TRUE;
	}
}


static void RenderCharacterInfoBackground(void);


static void BlitBackgroundToSaveBuffer(void)
{
	// render map
	RenderMapRegionBackground( );

	if (!fDisableDueToBattleRoster)
	{
		// render team
		RenderTeamRegionBackground( );

		// render character info
		RenderCharacterInfoBackground( );
	}
	else if( gfPreBattleInterfaceActive )
	{
		ForceButtonUnDirty( giMapContractButton );
		ForceButtonUnDirty( giCharInfoButton[ 0 ] );
		ForceButtonUnDirty( giCharInfoButton[ 1 ] );
		RenderPreBattleInterface();
	}

	// now render lower panel
	RenderMapScreenInterfaceBottom( );
}


static void MakeRegion(MOUSE_REGION* r, UINT idx, UINT16 x, UINT16 y, UINT16 w, MOUSE_CALLBACK move, MOUSE_CALLBACK click, const ST::string& help)
{
	MSYS_DefineRegion(r, x, y, x + w, y + Y_SIZE + 1, MSYS_PRIORITY_NORMAL + 1, MSYS_NO_CURSOR, move, click);
	MSYS_SetRegionUserData(r, 0, idx);
	r->SetFastHelpText(help);
}


static void TeamListAssignmentRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListAssignmentRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListContractRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListContractRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListDestinationRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListDestinationRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListInfoRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListInfoRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListSleepRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TeamListSleepRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateMouseRegionsForTeamList(void)
{
	// will create mouse regions for assignments, path plotting, character info selection

	// the info region...is the background for the list itself
	for (UINT i = 0; i < MAX_CHARACTER_COUNT; ++i)
	{
		const UINT16 y = Y_START + i * (Y_SIZE + 2) + (i >= FIRST_VEHICLE ? 6 : 0);

		const UINT16 w = NAME_WIDTH;
		CharacterRegions& r = g_character_regions[i];
		MakeRegion(&r.name,        i, NAME_X,           y, w,                    TeamListInfoRegionMvtCallBack,        TeamListInfoRegionBtnCallBack,        pMapScreenMouseRegionHelpText[0]); // name region
		MakeRegion(&r.assignment,  i, ASSIGN_X,         y, ASSIGN_WIDTH,         TeamListAssignmentRegionMvtCallBack,  TeamListAssignmentRegionBtnCallBack,  pMapScreenMouseRegionHelpText[1]); // assignment region
		MakeRegion(&r.sleep,       i, SLEEP_X,          y, SLEEP_WIDTH,          TeamListSleepRegionMvtCallBack,       TeamListSleepRegionBtnCallBack,       pMapScreenMouseRegionHelpText[5]); // sleep region
		// same function as name regions, so uses the same callbacks
		MakeRegion(&r.location,    i, LOC_X,            y, LOC_WIDTH,            TeamListInfoRegionMvtCallBack,        TeamListInfoRegionBtnCallBack,        pMapScreenMouseRegionHelpText[0]); // location region
		MakeRegion(&r.destination, i, DEST_ETA_X,       y, DEST_ETA_WIDTH,       TeamListDestinationRegionMvtCallBack, TeamListDestinationRegionBtnCallBack, pMapScreenMouseRegionHelpText[2]); // destination region
		MakeRegion(&r.contract,    i, TIME_REMAINING_X, y, TIME_REMAINING_WIDTH, TeamListContractRegionMvtCallBack,    TeamListContractRegionBtnCallBack,    pMapScreenMouseRegionHelpText[3]); // contract region
	}
}


static void DestroyMouseRegionsForTeamList(void)
{
	// will destroy mouse regions overlaying the team list area
	for (UINT i = 0; i < MAX_CHARACTER_COUNT; ++i)
	{
		CharacterRegions& r = g_character_regions[i];
		MSYS_RemoveRegion(&r.name);
		MSYS_RemoveRegion(&r.assignment);
		MSYS_RemoveRegion(&r.sleep);
		MSYS_RemoveRegion(&r.location);
		MSYS_RemoveRegion(&r.destination);
		MSYS_RemoveRegion(&r.contract);
	}
}


static void MapScreenMarkRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// reset selected characters
		ResetAllSelectedCharacterModes( );
	}
	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		// reset selected characters
		ResetAllSelectedCharacterModes( );
	}
}


static void ContractButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (g_dialogue_box) return;

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
#if 0 // XXX was commented out
		if (bSelectedDestChar != -1 || fPlotForHelicopter)
		{
			AbortMovementPlottingMode();
			return;
		}
#endif

		// redraw region
		if (btn->Area.uiFlags & MSYS_HAS_BACKRECT) fCharacterInfoPanelDirty = TRUE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		RequestContractMenu();
	}
}


static BOOLEAN HandleCtrlOrShiftInTeamPanel(INT8 bCharNumber);


static void TeamListInfoRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;

	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set to new info character...make sure is valid
		const SOLDIERTYPE* const pSoldier = gCharactersList[iValue].merc;
		if (pSoldier != NULL)
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			ChangeSelectedInfoChar( ( INT8 ) iValue, TRUE );

			// highlight
			giDestHighLine = -1;

			// reset character
			bSelectedAssignChar = -1;
			bSelectedDestChar = -1;
			bSelectedContractChar = -1;
			fPlotForHelicopter = FALSE;

			// if not dead or POW, select his sector
			if( ( pSoldier->bLife > 0 ) && ( pSoldier->bAssignment != ASSIGNMENT_POW ) )
			{
				ChangeSelectedMapSector(pSoldier->sSector);
			}

			// unhilight contract line
			giContractHighLine = -1;

			// can't assign highlight line
			giAssignHighLine = -1;

			// dirty team and map regions
			fTeamPanelDirty = TRUE;
			fMapPanelDirty = TRUE;
			//fMapScreenBottomDirty = TRUE;
			gfRenderPBInterface = TRUE;
		}
		else
		{
			// reset selected characters
			ResetAllSelectedCharacterModes( );
		}
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		ResetAllSelectedCharacterModes();

		const SOLDIERTYPE* const pSoldier = gCharactersList[iValue].merc;
		if (pSoldier != NULL)
		{
			// select this character
			ChangeSelectedInfoChar( ( INT8 ) iValue, TRUE );


			RequestToggleMercInventoryPanel();

			// if not dead or POW, select his sector
			if( ( pSoldier->bLife > 0 ) && ( pSoldier->bAssignment != ASSIGNMENT_POW ) )
			{
				ChangeSelectedMapSector(pSoldier->sSector);
			}
		}
	}
}


static void TeamListInfoRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		giHighLine = (gCharactersList[iValue].merc != NULL ? iValue : -1);
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		giHighLine = -1;
	}
}


static void TeamListAssignmentRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;

	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set to new info character...make sure is valid
		const SOLDIERTYPE* const pSoldier = gCharactersList[iValue].merc;
		if (pSoldier != NULL)
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			// reset list if the clicked character isn't also selected
			ChangeSelectedInfoChar(iValue, !IsEntryInSelectedListSet(iValue));

			// if alive (dead guys keep going, use remove menu instead),
			// and it's between sectors and it can be reassigned (non-vehicles)
			if ( ( pSoldier->bAssignment != ASSIGNMENT_DEAD ) && ( pSoldier->bLife > 0 ) && ( pSoldier->fBetweenSectors ) && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
			{
				// can't reassign mercs while between sectors
				DoScreenIndependantMessageBox( pMapErrorString[ 41 ], MSG_BOX_FLAG_OK, NULL );
				return;
			}

			bSelectedAssignChar = ( INT8 ) iValue;
			RebuildAssignmentsBox( );

			// reset dest character
			bSelectedDestChar = -1;
			fPlotForHelicopter = FALSE;

			// reset contract char
			bSelectedContractChar = -1;
			giContractHighLine = -1;

			// can't highlight line, anymore..if we were
			giDestHighLine = -1;

			// dirty team and map regions
			fTeamPanelDirty = TRUE;
			fMapPanelDirty = TRUE;
			gfRenderPBInterface = TRUE;

			// if this thing can be re-assigned
			if( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
			{
				giAssignHighLine = iValue;

				fShowAssignmentMenu = TRUE;
			}
			else
			{
				// can't highlight line
				giAssignHighLine = -1;

				// we can't highlight this line
//				giHighLine = -1;
			}
		}
		else
		{
			// reset selected characters
			ResetAllSelectedCharacterModes( );
		}
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		// reset selected characters
		ResetAllSelectedCharacterModes( );
	}
}


static void TeamListAssignmentRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		const SOLDIERTYPE* const s = gCharactersList[iValue].merc;
		if (s != NULL)
		{
			giHighLine = iValue;

			if (!(s->uiStatusFlags & SOLDIER_VEHICLE))
			{
				giAssignHighLine = iValue;
			}
			else
			{
				giAssignHighLine = -1;
			}
		}
		else
		{
			// can't highlight line
			giHighLine = -1;
			giAssignHighLine = -1;
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		giHighLine = -1;

		if( bSelectedAssignChar == -1 )
		{
			giAssignHighLine = -1;
		}

		// restore background
		RestoreBackgroundForAssignmentGlowRegionList( );
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		const SOLDIERTYPE* const s = gCharactersList[iValue].merc;
		if (s != NULL && !(s->uiStatusFlags & SOLDIER_VEHICLE))
		{
			// play click
			PlayGlowRegionSound( );
		}
	}
}


static bool CanChangeDestinationForChar(SOLDIERTYPE&);
static void MakeMapModesSuitableForDestPlotting(const SOLDIERTYPE*);


static void TeamListDestinationRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive || fShowMapInventoryPool )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SOLDIERTYPE* const s = gCharactersList[iValue].merc;
		if (s != NULL)
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			// reset list if the clicked character isn't also selected
			ChangeSelectedInfoChar(iValue, !IsEntryInSelectedListSet(iValue));

			// deselect any characters/vehicles that can't accompany the clicked merc
			DeselectSelectedListMercsWhoCantMoveWithThisGuy(s);

			// select all characters/vehicles that MUST accompany the clicked merc (same squad/vehicle)
			SelectUnselectedMercsWhoMustMoveWithThisGuy( );

			// Find out if this guy and everyone travelling with him is allowed to move strategically
			// NOTE: errors are reported within...
			if (CanChangeDestinationForChar(*s))
			{
				// turn off sector inventory, turn on show teams filter, etc.
				MakeMapModesSuitableForDestPlotting(s);

				if (InHelicopter(*s))
				{
					TurnOnAirSpaceMode();
					if (!RequestGiveSkyriderNewDestination())
					{
						// not allowed to change destination of the helicopter
						return;
					}
				}

				// select this character as the one plotting strategic movement
				bSelectedDestChar = ( INT8 )iValue;

				// remember the current paths for all selected characters so we can restore them if need be
				RememberPreviousPathForAllSelectedChars();

				// highlight
				giDestHighLine = iValue;

				// can't assign highlight line
				giAssignHighLine = -1;

				// reset assign character
				bSelectedAssignChar = -1;

				// reset contract char
				bSelectedContractChar = -1;
				giContractHighLine = -1;

				// dirty team and map regions
				fTeamPanelDirty = TRUE;
				fMapPanelDirty = TRUE;
				gfRenderPBInterface = TRUE;


				// set cursor
				SetUpCursorForStrategicMap( );
			}
			else	// problem - this guy can't move
			{
				// cancel destination highlight
				giDestHighLine = -1;
			}
		}
		else	// empty char list slot
		{
			// reset selected characters
			ResetAllSelectedCharacterModes( );
		}
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		CancelPathsOfAllSelectedCharacters();
		ResetAllSelectedCharacterModes();
	}
}


static void TeamListDestinationRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = -1;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		if (gCharactersList[iValue].merc != NULL)
		{
			giHighLine = iValue;
			giDestHighLine = iValue;
		}
		else
		{
			// can't highlight line
			giHighLine = -1;
			giDestHighLine = -1;
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		giHighLine = -1;

		if( bSelectedDestChar == -1 )
		{
			giDestHighLine = -1;
		}

		// restore background
		RestoreBackgroundForDestinationGlowRegionList( );
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if (gCharactersList[iValue].merc != NULL)
		{
			// play click
			PlayGlowRegionSound( );
		}
	}
}


static void TeamListSleepRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;

	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set to new info character...make sure is valid.. not in transit and alive and concious
		SOLDIERTYPE* const pSoldier = gCharactersList[iValue].merc;
		if (pSoldier != NULL)
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			// reset list if the clicked character isn't also selected
			ChangeSelectedInfoChar(iValue, !IsEntryInSelectedListSet(iValue));

			if (CanChangeSleepStatusForSoldier(pSoldier))
			{
				if (pSoldier->fMercAsleep)
				{
					// try to wake him up
					if( SetMercAwake( pSoldier, TRUE, FALSE ) )
					{
						// propagate
						HandleSelectedMercsBeingPutAsleep( TRUE, TRUE );
						return;
					}
					else
					{
						HandleSelectedMercsBeingPutAsleep( TRUE, FALSE );
					}
				}
				else	// awake
				{
					// try to put him to sleep
					if (SetMercAsleep(*pSoldier, true))
					{
						// propagate
						HandleSelectedMercsBeingPutAsleep( FALSE, TRUE );
						return;
					}
					else
					{
						HandleSelectedMercsBeingPutAsleep( FALSE, FALSE );
					}
				}
			}
		}
		else
		{
			// reset selected characters
			ResetAllSelectedCharacterModes( );
		}
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		// reset selected characters
		ResetAllSelectedCharacterModes( );
	}
}


static void TeamListSleepRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = -1;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		const SOLDIERTYPE* const s = gCharactersList[iValue].merc;
		if (s != NULL)
		{
			giHighLine = iValue;
			giSleepHighLine = (CanChangeSleepStatusForSoldier(s) ? iValue : -1);
		}
		else
		{
			// can't highlight line
			giHighLine = -1;
			giSleepHighLine = -1;
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		giHighLine = -1;
		giSleepHighLine = -1;

		// restore background
		RestoreBackgroundForSleepGlowRegionList( );
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		const SOLDIERTYPE* const s = gCharactersList[iValue].merc;
		if (s != NULL && CanChangeSleepStatusForSoldier(s)) PlayGlowRegionSound();
	}
}


static void ContractRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void TeamListContractRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (gCharactersList[iValue].merc != NULL)
	{
		if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			// select ONLY this dude
			ChangeSelectedInfoChar( ( INT8 ) iValue, TRUE );

			// reset character
			giDestHighLine = -1;
			bSelectedAssignChar = -1;
			bSelectedDestChar = -1;
			bSelectedContractChar = -1;
			fPlotForHelicopter = FALSE;

			fTeamPanelDirty = TRUE;
		}

		ContractRegionBtnCallback( pRegion, iReason );
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		// reset selected characters
		ResetAllSelectedCharacterModes( );
	}
}


static void TeamListContractRegionMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = -1;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		const SOLDIERTYPE* const s = gCharactersList[iValue].merc;
		if (s != NULL)
		{
			giHighLine = iValue;
			giContractHighLine = (CanExtendContractForSoldier(s) ? iValue : -1);
		}
		else
		{
			// can't highlight line
			giHighLine = -1;
			giContractHighLine = -1;
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		giHighLine = -1;

		// no longer valid char?...reset display of highlight boxes
		if (!fShowContractMenu) giContractHighLine = -1;

		// restore background
		RestoreBackgroundForContractGlowRegionList( );
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		const SOLDIERTYPE* const s = gCharactersList[iValue].merc;
		if (s != NULL && CanExtendContractForSoldier(s)) PlayGlowRegionSound();
	}
}


static void HandleHighLightingOfLinesInTeamPanel(void)
{
	if ( fShowInventoryFlag )
	{
		return;
	}

	// will highlight or restore backgrounds to highlighted lines

	// restore backgrounds, if need be
	RestoreBackgroundForAssignmentGlowRegionList( );
	RestoreBackgroundForDestinationGlowRegionList( );
	RestoreBackgroundForContractGlowRegionList( );
	RestoreBackgroundForSleepGlowRegionList( );

	HighLightAssignLine();
	HighLightDestLine();
	HighLightSleepLine();

	// contracts?
	if( giContractHighLine != -1 )
	{
		ContractListRegionBoxGlow( ( UINT16 ) giContractHighLine );
	}
}


static void PlotPermanentPaths(void)
{
	if (fPlotForHelicopter)
	{
		DisplayHelicopterPath( );
	}
	else if( bSelectedDestChar != -1 )
	{
		DisplaySoldierPath(gCharactersList[bSelectedDestChar].merc);
	}
}


static void PlotTemporaryPaths(void)
{
	SGPSector sMap;
	// check to see if we have in fact moved are are plotting a path?
	if (!GetMouseMapXY(sMap)) return;

	if (fPlotForHelicopter)
	{
		Assert(fShowAircraftFlag);

		// plot temp path
		PlotATemporaryPathForHelicopter(sMap);

		// check if potential path is allowed
		DisplayThePotentialPathForHelicopter(sMap);

		if (fDrawTempHeliPath)
		{
			// clip region
			ClipBlitsToMapViewRegion();
			// display heli temp path
			DisplayHelicopterTempPath();
			//restore
			RestoreClipRegionToFullScreen();
		}
	}
	else if (bSelectedDestChar != -1) // dest char has been selected
	{
		PlotATemporaryPathForCharacter(gCharactersList[bSelectedDestChar].merc, sMap);

		// check to see if we are drawing path
		DisplayThePotentialPathForCurrentDestinationCharacterForMapScreenInterface(sMap);

		// if we need to draw path, do it
		if (fDrawTempPath)
		{
			// clip region
			ClipBlitsToMapViewRegion();
			// blit
			DisplaySoldierTempPath();
			// restore
			RestoreClipRegionToFullScreen();
		}
	}
}



void RenderMapRegionBackground( void )
{
	// renders to save buffer when dirty flag set

	if (!fMapPanelDirty)
	{
		gfMapPanelWasRedrawn = FALSE;

		// not dirty, leave
		return;
	}

	// don't bother if showing sector inventory instead of the map!!!
	if( !fShowMapInventoryPool )
	{
		// draw map
		DrawMap( );
	}


	// blit in border
	RenderMapBorder( );

	if (ghAttributeBox != NO_POPUP_BOX) ForceUpDateOfBox(ghAttributeBox);
	if (ghTownMineBox  != NO_POPUP_BOX) ForceUpDateOfBox(ghTownMineBox);

	MapscreenMarkButtonsDirty();

	RestoreExternBackgroundRect(STD_SCREEN_X + 261, STD_SCREEN_Y + 0, MAP_BG_WIDTH, 359);

	// don't bother if showing sector inventory instead of the map!!!
	if( !fShowMapInventoryPool )
	{
		// if Skyrider can and wants to talk to us
		if( IsHelicopterPilotAvailable( ) )
		{
			// see if Skyrider has anything new to tell us
			CheckAndHandleSkyriderMonologues( );
		}
	}

	// reset dirty flag
	fMapPanelDirty = FALSE;

	gfMapPanelWasRedrawn = TRUE;
}


static void DisplayIconsForMercsAsleep(void);


static void RenderTeamRegionBackground()
{
	// Render to save buffer when dirty flag set
	if (!fTeamPanelDirty) return;

	// Show inventory or the team list?
	if (!fShowInventoryFlag)
	{
		BltVideoObject(guiSAVEBUFFER, guiCHARLIST, 0, PLAYER_INFO_X, PLAYER_INFO_Y);
		HandleHighLightingOfLinesInTeamPanel();
		DisplayCharacterList();
		DisplayIconsForMercsAsleep();
	}
	else
	{
		BltCharInvPanel();
	}

	fDrawCharacterList  = FALSE;
	fTeamPanelDirty     = FALSE;
	gfRenderPBInterface = TRUE;

	MarkAllBoxesAsAltered();
	RestoreExternBackgroundRect(STD_SCREEN_X + 0, STD_SCREEN_Y + 107, 261 - 0, 359 - 107);
	MapscreenMarkButtonsDirty();
}


static void RenderCharacterInfoBackground(void)
{
	// will render the background for the character info panel

	if (!fCharacterInfoPanelDirty)
	{
		// not dirty, leave
		return;
	}

	// the upleft hand corner character info panel
	BltVideoObject(guiSAVEBUFFER, guiCHARINFO, 0, TOWN_INFO_X, TOWN_INFO_Y);

	UpdateHelpTextForMapScreenMercIcons( );

	if (!fDisableDueToBattleRoster)
	{
		DisplayCharacterInfo();
		RenderAttributeStringsForUpperLeftHandCorner( guiSAVEBUFFER );
	}

	// reset dirty flag
	fCharacterInfoPanelDirty = FALSE;

	// redraw face
	fReDrawFace = TRUE;

	MapscreenMarkButtonsDirty();

	// mark all pop ups as dirty
	MarkAllBoxesAsAltered( );

	// restore background for area
	RestoreExternBackgroundRect( STD_SCREEN_X + 0, STD_SCREEN_Y + 0, 261, 107 );

}


static void HandleShadingOfLinesForContractMenu(void);


static void DetermineIfContractMenuCanBeShown(void)
{
	if (!fShowContractMenu)
	{

		// destroy menus for contract region
		CreateDestroyMouseRegionsForContractMenu( );

		// hide all boxes
		HideBox( ghContractBox );

		// make sure, absolutly sure we want to hide this box
		if (!fShowAssignmentMenu)
		{
			HideBox( ghRemoveMercAssignBox );
		}


		return;
	}

	// create mask, if needed
	CreateDestroyScreenMaskForAssignmentAndContractMenus( );

	// create mouse regions for contract region
	CreateDestroyMouseRegionsForContractMenu( );

	// determine which lines selectable
	HandleShadingOfLinesForContractMenu( );

	if (GetSelectedInfoChar()->bLife == 0)
	{
		// show basic assignment menu
		ShowBox( ghRemoveMercAssignBox );
	}
	else
	{
		// show basic contract menu
		ShowBox( ghContractBox );
	}
}


static void CheckIfPlottingForCharacterWhileAirCraft(void)
{
	// if we are in aircraft mode and plotting for character, reset plotting character
	if (fShowAircraftFlag)
	{
		// if plotting, but not for heli
		if (bSelectedDestChar != -1 && !fPlotForHelicopter)
		{
			// abort
			AbortMovementPlottingMode();
		}
	}
	else	// not in airspace mode
	{
		if (fPlotForHelicopter)
		{
			// abort
			AbortMovementPlottingMode();
		}
	}
}


static void ContractRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for contract region

	if (g_dialogue_box) return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
		if (CanExtendContractForSoldier(pSoldier))
		{
			// create
			RebuildContractBoxForMerc( pSoldier );

			// reset selected characters
			ResetAllSelectedCharacterModes( );

			bSelectedContractChar = bSelectedInfoChar;
			giContractHighLine = bSelectedContractChar;

			// if not triggered internally
			if (!CheckIfSalaryIncreasedAndSayQuote(pSoldier, TRUE))
			{
				// show contract box
				fShowContractMenu = TRUE;

				// stop any active dialogue
				StopAnyCurrentlyTalkingSpeech( );
			}

			//fCharacterInfoPanelDirty = TRUE;
		}
		else
		{
			// reset selected characters
			ResetAllSelectedCharacterModes( );
		}
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		// reset selected characters
		ResetAllSelectedCharacterModes( );
	}
}


static void HandleShadingOfLinesForContractMenu(void)
{
	if (!fShowContractMenu) return;

	PopUpBox* const box = ghContractBox;
	if (box == NO_POPUP_BOX) return;

	// error check, return if not a valid character
	if (bSelectedContractChar == -1) return;
	const SOLDIERTYPE* const s = gCharactersList[bSelectedContractChar].merc;
	if (s == NULL) return;

	Assert(CanExtendContractForSoldier(s));

	// is guy in AIM? and well enough to talk and make such decisions?
	if (s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC && s->bLife >= OKLIFE)
	{
		MERCPROFILESTRUCT const& p       = GetProfile(s->ubProfile);
		INT32             const  balance = LaptopSaveInfo.iCurrentBalance;
		ShadeStringInBox(box, CONTRACT_MENU_DAY,              p.sSalary          > balance);
		ShadeStringInBox(box, CONTRACT_MENU_WEEK,      (INT32)p.uiWeeklySalary   > balance);
		ShadeStringInBox(box, CONTRACT_MENU_TWO_WEEKS, (INT32)p.uiBiWeeklySalary > balance);
	}
	else
	{
		// can't extend contract duration
		ShadeStringInBox(box, CONTRACT_MENU_DAY,       true);
		ShadeStringInBox(box, CONTRACT_MENU_WEEK,      true);
		ShadeStringInBox(box, CONTRACT_MENU_TWO_WEEKS, true);
	}

	// if THIS soldier is involved in a fight (dismissing in a hostile sector IS ok...)
	ShadeStringInBox(box, CONTRACT_MENU_TERMINATE, gTacticalStatus.uiFlags & INCOMBAT && s->bInSector);
}


static void SortListOfMercsInTeamPanel(BOOLEAN fRetainSelectedMercs);


void ReBuildCharactersList( void )
{
	// rebuild character's list

	// add in characters
	for (INT16 sCount = 0; sCount < MAX_CHARACTER_COUNT; ++sCount)
	{
		// clear this slot
		gCharactersList[sCount].merc = NULL;
	}

	FOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		AddCharacter(s);
	}

	// sort them according to current sorting method
	SortListOfMercsInTeamPanel( FALSE );


	// if nobody is selected, or the selected merc has somehow become invalid
	if (GetSelectedInfoChar() == NULL)
	{
		// is the first character in the list valid?
		if (gCharactersList[0].merc != NULL)
		{
			// select him
			ChangeSelectedInfoChar( 0, TRUE );
		}
		else
		{
			// select no one
			ChangeSelectedInfoChar( -1, TRUE );
		}
	}

	// exit inventory mode
	fShowInventoryFlag = FALSE;
}


// handle change in info char
static void HandleChangeOfInfoChar(void)
{
	static INT8 bOldInfoChar = -1;

	if( bSelectedInfoChar != bOldInfoChar )
	{
		// set auto faces inactive

		// valid character?
		if( bOldInfoChar != -1 )
		{
			const SOLDIERTYPE* const s = gCharactersList[bOldInfoChar].merc;
			if (s && s->face) SetAutoFaceInActive(*s->face);
		}

		// stop showing contract box
		//fShowContractMenu = FALSE;

		// update old info char value
		bOldInfoChar = bSelectedInfoChar;
	}
}


void RebuildContractBoxForMerc(const SOLDIERTYPE* const pCharacter)
{
	// rebuild contractbox for this merc
	RemoveBox( ghContractBox );
	ghContractBox = NO_POPUP_BOX;

	// recreate
	CreateContractBox( pCharacter );
}


static void EnableDisableTeamListRegionsAndHelpText(void)
{
	// check if valid character here, if so, then do nothing..other wise set help text timer to a gazillion
	INT8 bCharNum;


	for( bCharNum = 0; bCharNum < MAX_CHARACTER_COUNT; bCharNum++ )
	{
		SOLDIERTYPE const* const s = gCharactersList[bCharNum].merc;
		CharacterRegions&        r = g_character_regions[bCharNum];
		if (s == NULL)
		{
			// disable regions in all team list columns
			r.name.Disable();
			r.assignment.Disable();
			r.sleep.Disable();
			r.location.Disable();
			r.destination.Disable();
			r.contract.Disable();
		}
		else
		{
			// always enable Name and Location regions
			r.name.Enable();
			r.location.Enable();

			// valid character.  If it's a vehicle, however
			if (s->uiStatusFlags & SOLDIER_VEHICLE)
			{
				// Can't change assignment for vehicles
				r.assignment.Disable();
			}
			else
			{
				r.assignment.Enable();

				// POW or dead ?
				if (s->bAssignment == ASSIGNMENT_POW || s->bLife == 0)
				{
					// "Remove Merc"
					r.assignment.SetFastHelpText(pRemoveMercStrings[0]);
					r.destination.SetFastHelpText(ST::null);
				}
				else
				{
					// "Assign Merc"
					r.assignment.SetFastHelpText(pMapScreenMouseRegionHelpText[1]);
					// "Plot Travel Route"
					r.destination.SetFastHelpText(pMapScreenMouseRegionHelpText[2]);
				}
			}

			if (CanExtendContractForSoldier(s))
			{
				r.contract.Enable();
			}
			else
			{
				r.contract.Disable();
			}

			if (CanChangeSleepStatusForSoldier(s))
			{
				r.sleep.Enable();
			}
			else
			{
				r.sleep.Disable();
			}

			// destination region is always enabled for all valid character slots.
			// if the character can't move at this time, then the region handler must be able to tell the player why not
			r.destination.Enable();
		}
	}
}


static void ResetAllSelectedCharacterModes(void)
{
	// if in militia redistribution popup
	if ( sSelectedMilitiaTown != 0 )
	{
		sSelectedMilitiaTown = 0;
	}


	// cancel destination line highlight
	giDestHighLine = -1;

	// cancel assign line highlight
	giAssignHighLine = -1;

	// unhilight contract line
	giContractHighLine = -1;


	// if we were plotting movement
	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode();
	}

	// reset assign character
	bSelectedAssignChar = -1;

	// reset contract character
	bSelectedContractChar = -1;


	// reset map cursor to normal
	if ( !gfFadeOutDone && !gfFadeIn )
	{
		SetUpCursorForStrategicMap( );
	}
}


// automatically pause/unpause time compression during certain events
static void UpdatePausedStatesDueToTimeCompression(void)
{
	// this executes every frame, so keep it optimized for speed!

	// if time is being compressed
	if( IsTimeBeingCompressed() )
	{
		// but it shouldn't be
		if ( !AllowedToTimeCompress( ) )
		{
			// pause game to (temporarily) stop time compression
			PauseGame( );
		}
	}
	else	// time is NOT being compressed
	{
		// but the player would like it to be compressing
		if ( IsTimeCompressionOn() && !gfPauseDueToPlayerGamePause )
		{
			// so check if it's legal to start time compressing again
			if ( AllowedToTimeCompress( ) )
			{
				// unpause game to restart time compresssion
				UnPauseGame( );
			}
		}
	}
}


BOOLEAN ContinueDialogue(SOLDIERTYPE* pSoldier, BOOLEAN fDone)
{
	// continue this grunts dialogue, restore when done
	static BOOLEAN fTalkingingGuy = FALSE;

	INT8 bCounter = 0;

	if (fDone)
	{
		if (fTalkingingGuy)
		{
			fCharacterInfoPanelDirty = TRUE;
			fTalkingingGuy = FALSE;
		}

		return( TRUE );
	}

	// check if valid character talking?
	if( pSoldier == NULL )
	{
		return FALSE;
	}

	// otherwise, find this character
	for( bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++ )
	{
		if (gCharactersList[bCounter].merc == pSoldier)
		{
			if (bSelectedInfoChar != bCounter)
			{
				ChangeSelectedInfoChar(bCounter, TRUE);
			}
			fTalkingingGuy = TRUE;
			return FALSE;
		}
	}

	return ( FALSE );
}


static void HandleSpontanousTalking(void)
{
	// simply polls if the talking guy is done, if so...send an end command to continue dialogue
	if (!DialogueActive())
	{
		SOLDIERTYPE* const s = GetSelectedInfoChar();
		if (s != NULL) ContinueDialogue(s, TRUE);
	}
}


static void HandleNewDestConfirmation(const SGPSector& sMap);
static void RebuildWayPointsForAllSelectedCharsGroups(void);


static BOOLEAN CheckIfClickOnLastSectorInPath(const SGPSector& sector)
{
	PathSt*const* ppMovePath = NULL;
	BOOLEAN fLastSectorInPath = FALSE;
	INT32 iVehicleId = -1;

	// see if we have clicked on the last sector in the characters path

	// check if helicopter
	if (fPlotForHelicopter)
	{
		if (sector.AsStrategicIndex() == GetLastSectorOfHelicoptersPath())
		{
			// helicopter route confirmed - take off
			TakeOffHelicopter( );

			// rebuild waypoints - helicopter
			VEHICLETYPE& v = GetHelicopter();
			ppMovePath     = &v.pMercPath;
			RebuildWayPointsForGroupPath(*ppMovePath, *GetGroup(v.ubMovementGroup));

			fLastSectorInPath = TRUE;
		}
	}
	else	// not doing helicopter movement
	{
		// if not doing a soldier either, we shouldn't be here!
		if( bSelectedDestChar == -1 )
		{
			return( FALSE );
		}

		MapScreenCharacterSt* const c = &gCharactersList[bSelectedDestChar];
		const SOLDIERTYPE*    const s = c->merc;

		// invalid soldier?  we shouldn't be here!
		if (s == NULL)
		{
			bSelectedDestChar = -1;
			return( FALSE );
		}

		if (sector.AsStrategicIndex() == GetLastSectorIdInCharactersPath(s))
		{
			// clicked on last sector, reset plotting mode

			// if he's IN a vehicle or IS a vehicle
			if (s->bAssignment == VEHICLE || s->uiStatusFlags & SOLDIER_VEHICLE)
			{
				if (s->bAssignment == VEHICLE)
				{
					// IN a vehicle
					iVehicleId = s->iVehicleId;
				}
				else
				{
					// IS a vehicle
					iVehicleId = s->bVehicleID;
				}

				// rebuild waypoints - vehicles
				ppMovePath = &( pVehicleList[ iVehicleId ].pMercPath );
			}
			else
			{
				// rebuild waypoints - mercs on foot
				ppMovePath = &s->pMercPath;
			}

			RebuildWayPointsForAllSelectedCharsGroups( );

			fLastSectorInPath = TRUE;
		}
	}


	// if the click was over the last sector
	if ( fLastSectorInPath )
	{
		// route has been confirmed
		EndConfirmMapMoveMode( );

		// if we really did plot a path (this will skip message if left click on current sector with no path)
		if ( GetLengthOfPath( *ppMovePath ) > 0 )
		{
			// then verbally confirm this destination!
			HandleNewDestConfirmation(sector);
		}
		else	// NULL path confirmed
		{
			// if previously there was a path
			if (g_prev_path)
			{
				// then this means we've CANCELED it
				BeginMapUIMessage(0, pMapPlotStrings[3]);
			}
			else	// no previous path
			{
				// then it means the route was UNCHANGED
				BeginMapUIMessage(0, pMapPlotStrings[2]);
			}
		}
	}


	return( fLastSectorInPath );
}


// rebuild waypoints for selected character list
static void RebuildWayPointsForAllSelectedCharsGroups(void)
{
	// rebuild the waypoints for everyone in the selected character list
	BOOLEAN fGroupIDRebuilt[ 256 ];
	INT32 iVehicleId;
	PathSt** ppMovePath = NULL;
	UINT8 ubGroupId;


	std::fill(std::begin(fGroupIDRebuilt), std::end(fGroupIDRebuilt), FALSE);

	CFOR_EACH_SELECTED_IN_CHAR_LIST(c)
	{
		const SOLDIERTYPE* const pSoldier = c->merc;

		// if he's IN a vehicle or IS a vehicle
		if( ( pSoldier->bAssignment == VEHICLE ) || ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			if( pSoldier->bAssignment == VEHICLE )
			{
				// IN a vehicle
				iVehicleId = pSoldier->iVehicleId;
			}
			else
			{
				// IS a vehicle
				iVehicleId = pSoldier->bVehicleID;
			}

			// vehicles
			VEHICLETYPE* const v = &pVehicleList[iVehicleId];
			ppMovePath = &v->pMercPath;
			ubGroupId  = v->ubMovementGroup;
		}
		else
		{
			// mercs on foot
			ppMovePath = &gCharactersList[bSelectedDestChar].merc->pMercPath;
			ubGroupId = pSoldier->ubGroupID;
		}

		// if we haven't already rebuilt this group
		if ( !fGroupIDRebuilt[ ubGroupId ] )
		{
			// rebuild it now
			RebuildWayPointsForGroupPath(*ppMovePath, *GetGroup(ubGroupId));

			// mark it as rebuilt
			fGroupIDRebuilt[ ubGroupId ] = TRUE;
		}
	}
}


// check if cursor needs to be set to checkmark or to the walking guy?
static void UpdateCursorIfInLastSector(void)
{
	// check to see if we are plotting a path, if so, see if we are highlighting the last sector int he path, if so, change the cursor
	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		SGPSector sMap;
		GetMouseMapXY(sMap);

		if (!fShowAircraftFlag)
		{
			if( bSelectedDestChar != -1 )
			{
				//c heck if we are in the last sector of the characters path?
				if (sMap.AsStrategicIndex() == GetLastSectorIdInCharactersPath(gCharactersList[bSelectedDestChar].merc))
				{
					// set cursor to checkmark
					ChangeMapScreenMaskCursor( CURSOR_CHECKMARK );
				}
				else if( fCheckCursorWasSet )
				{
					// reset to walking guy/vehicle
					SetUpCursorForStrategicMap( );
				}
			}
		}
		else
		{
			// check for helicopter
			if( fPlotForHelicopter )
			{
				if (sMap.AsStrategicIndex() == GetLastSectorOfHelicoptersPath())
				{
					// set cursor to checkmark
					ChangeMapScreenMaskCursor( CURSOR_CHECKMARK );
				}
				else if( fCheckCursorWasSet )
				{
					// reset to walking guy/vehicle
					SetUpCursorForStrategicMap( );
				}
			}
			else
			{
				// reset to walking guy/vehicle
				SetUpCursorForStrategicMap( );
			}
		}
	}
}


static void FaceRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// error checking, make sure someone is there
	if (GetSelectedInfoChar() == NULL) return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (gfPreBattleInterfaceActive) return;

		// now stop any dialogue in progress
		StopAnyCurrentlyTalkingSpeech( );
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		RequestToggleMercInventoryPanel();
	}
}


static void ItemRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// left AND right button are handled the same way
	if (iReason & ( MSYS_CALLBACK_REASON_RBUTTON_UP | MSYS_CALLBACK_REASON_LBUTTON_UP ) )
	{
		RequestToggleMercInventoryPanel();
	}
}


static bool CanToggleSelectedCharInventory();


static void ItemRegionMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if ( !CanToggleSelectedCharInventory() )
	{
		fShowItemHighLight = FALSE;
		return;
	}

	if( ( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE ) )
	{
		fShowItemHighLight = TRUE;
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fShowItemHighLight = FALSE;
	}
}


// handle highlighting of team panel lines
static void HandleChangeOfHighLightedLine(void)
{
	static INT32 iOldHighLine;

	if ( fShowInventoryFlag )
	{
		return;
	}

	// check if change in highlight line
	if( giHighLine != iOldHighLine )
	{
		iOldHighLine = giHighLine;

		if( giHighLine == -1 )
		{
			giSleepHighLine = -1;
			giAssignHighLine = -1;
			giContractHighLine = -1;
			giSleepHighLine = -1;

			// don't do during plotting, allowing selected character to remain highlighted and their destination column to glow!
			if (bSelectedDestChar == -1 && !fPlotForHelicopter)
			{
				giDestHighLine = -1;
			}
		}

		fDrawCharacterList = TRUE;
	}
}


static void HandleCharBarRender(void)
{
	if (fDisableDueToBattleRoster) return; // check if the panel is disbled, if so, do not render

	const SOLDIERTYPE* const s = GetSelectedInfoChar();
	if (s == NULL) return;

	if (s->bLife       != 0               &&
			s->bAssignment != ASSIGNMENT_DEAD &&
			s->bAssignment != ASSIGNMENT_POW)
	{
		DrawSoldierUIBars(*s, BAR_INFO_X, BAR_INFO_Y, TRUE, FRAME_BUFFER);
	}

	UpdateCharRegionHelpText();
}


// update the status of the contract box
static void UpDateStatusOfContractBox(void)
{
	if (fShowContractMenu)
	{
		ForceUpDateOfBox( ghContractBox );

		const SOLDIERTYPE* const s = GetSelectedInfoChar();
		if (s->bLife == 0 || s->bAssignment == ASSIGNMENT_POW)
		{
			ForceUpDateOfBox( ghRemoveMercAssignBox );
		}
	}
}


static void TrashItemMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		// find the item and get rid of it
		MAPEndItemPointer();

		// reset cursor
		gSMPanelRegion.ChangeCursor(CURSOR_NORMAL);
		SetCurrentCursorFromDatabase( CURSOR_NORMAL );

		HandleButtonStatesWhileMapInventoryActive( );
	}
}


static void TrashCanBtnCallback(MOUSE_REGION*, INT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// Check if an item is in the cursor, if so, warn player
		if (OBJECTTYPE* const o = gpItemPointer)
		{
			ST::string msg = o->ubMission ? pTrashItemText[1] : pTrashItemText[0];
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, msg, MAP_SCREEN, MSG_BOX_FLAG_YESNO, TrashItemMessageBoxCallBack);
		}
	}
}


static void TrashCanMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if (fMapInventoryItem)
		{
			fShowTrashCanHighLight = TRUE;
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fShowTrashCanHighLight = FALSE;
	}
}


static void UpdateStatusOfMapSortButtons(void)
{
	INT32 iCounter = 0;
	static BOOLEAN fShownLastTime = FALSE;


	if( ( gfPreBattleInterfaceActive ) || fShowInventoryFlag )
	{
		if ( fShownLastTime )
		{
			for( iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++ )
			{
				HideButton( giMapSortButton[ iCounter ] );
			}
			if ( gfPreBattleInterfaceActive )
			{
				HideButton( giCharInfoButton[ 0 ] );
				HideButton( giCharInfoButton[ 1 ] );
			}

			fShownLastTime = FALSE;
		}
	}
	else
	{
		if ( !fShownLastTime )
		{
			for( iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++ )
			{
				ShowButton( giMapSortButton[ iCounter ] );
			}

			ShowButton( giCharInfoButton[ 0 ] );
			ShowButton( giCharInfoButton[ 1 ] );

			fShownLastTime = TRUE;
		}
	}
}


static void DoneInventoryMapBtnCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateDestroyTrashCanRegion(void)
{
	static BOOLEAN fCreated = FALSE;

	if (fShowInventoryFlag && !fCreated)
	{

		fCreated = TRUE;

		// trash can
		MSYS_DefineRegion( &gTrashCanRegion, 	TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH, TRASH_CAN_Y + TRASH_CAN_HEIGHT , MSYS_PRIORITY_HIGHEST - 4 ,
					MSYS_NO_CURSOR, TrashCanMoveCallback, TrashCanBtnCallback );

		// done inventory button define
		giMapInvDoneButton = QuickCreateButtonImg(INTERFACEDIR "/done_button2.sti", 0, 1, INV_BTN_X, INV_BTN_Y, MSYS_PRIORITY_HIGHEST - 1, DoneInventoryMapBtnCallback);
		giMapInvDoneButton->SetFastHelpText(pMiscMapScreenMouseRegionHelpText[2]);

		gTrashCanRegion.SetFastHelpText(pMiscMapScreenMouseRegionHelpText[1]);

		InitMapKeyRingInterface( KeyRingItemPanelButtonCallback );

			// reset the compatable item array at this point
		ResetCompatibleItemArray( );

	}
	else if (!fShowInventoryFlag && fCreated)
	{
		// trash can region
		fCreated = FALSE;
		MSYS_RemoveRegion( &gTrashCanRegion );

		// map inv done button
		RemoveButton( giMapInvDoneButton );

		ShutdownKeyRingInterface( );

		if (fShowDescriptionFlag)
		{
			// kill description
			DeleteItemDescriptionBox( );
		}

	}
}


static void DoneInventoryMapBtnCallback(GUI_BUTTON* btn, INT32 reason)
{
	// prevent inventory from being closed while stack popup up!
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (!fMapInventoryItem && !InItemStackPopup())
		{
			fEndShowInventoryFlag = TRUE;
		}
	}
}


static void StartConfirmMapMoveMode(INT16 sMapY)
{
	// tell player what to do - to click again to confirm move
	BeginMapUIMessage(sMapY < 8 ? 100 : -100, pMapPlotStrings[0]);
	gfInConfirmMapMoveMode = TRUE;
}


void EndConfirmMapMoveMode(void)
{
	CancelMapUIMessage( );

	gfInConfirmMapMoveMode = FALSE;
}


void CancelMapUIMessage( void )
{
	// and kill the message overlay
	EndUIMessage( );

	fMapPanelDirty = TRUE;
}


// automatically turns off mapscreen ui overlay messages when appropriate
static void MonitorMapUIMessage(void)
{
	// if there is a map UI message being displayed
	if (g_ui_message_overlay != NULL)
	{
		// and if we're not in the middle of the "confirm move" sequence
//		if( !gfInConfirmMapMoveMode || bSelectedDestChar == -1 )
		{
			// and we've now exceed its period of maximum persistance (without user input)
			if ( ( GetJA2Clock( ) - guiUIMessageTime ) > guiUIMessageTimeDelay )
			{
				// then cancel the message now
				CancelMapUIMessage( );
			}
		}
	}
}


// handle pre battle interface in relation to inventory
static void HandlePreBattleInterfaceWithInventoryPanelUp(void)
{
	if (gfPreBattleInterfaceActive && fShowInventoryFlag)
	{
		if (fShowDescriptionFlag)
		{
			// kill description
			DeleteItemDescriptionBox( );
		}

		// kill inventory panel
		fShowInventoryFlag = FALSE;
		CreateDestroyMapInvButton();
	}
}


//update any bad assignments..error checking
// this puts anyone who is on NO_ASSIGNMENT onto a free squad
static void UpdateBadAssignments(void)
{
	CFOR_EACH_IN_CHAR_LIST(c)
	{
		CheckIfSoldierUnassigned(c->merc);
	}
}


static void InterruptTimeForMenus(void)
{
	if (fShowAssignmentMenu || fShowContractMenu)
	{
		InterruptTime( );
		PauseTimeForInterupt( );
	}
	else if( fOneFrame )
	{
		InterruptTime( );
		PauseTimeForInterupt( );
	}
}


static bool AnyMercsLeavingRealSoon();


static void HandleContractTimeFlashForMercThatIsAboutLeave(void)
{
	// grab the current time
	UINT32 uiCurrentTime = GetJA2Clock();

	// only bother checking once flash interval has elapsed
	if( ( uiCurrentTime - guiFlashContractBaseTime ) >= DELAY_PER_FLASH_FOR_DEPARTING_PERSONNEL )
	{
		// update timer so that we only run check so often
		guiFlashContractBaseTime = uiCurrentTime;
		fFlashContractFlag = !fFlashContractFlag;

		// don't redraw unless we have to!
		if ( AnyMercsLeavingRealSoon() )
		{
			// redraw character list
			fDrawCharacterList = TRUE;
		}
	}
}


// merc about to leave, flash contract time
static bool AnyMercsLeavingRealSoon()
{
	UINT32 const now = GetWorldTotalMin();
	CFOR_EACH_IN_CHAR_LIST(c)
	{
		if (c->merc->iEndofContractTime - now > MINS_TO_FLASH_CONTRACT_TIME) continue;
		return true;
	}
	return false;
}


void HandlePreloadOfMapGraphics(void)
{
	guiSleepIcon                = AddVideoObjectFromFile(INTERFACEDIR "/sleepicon.sti");
	guiCHARINFO                 = AddVideoObjectFromFile(INTERFACEDIR "/charinfo.sti");
	guiCHARLIST                 = AddVideoObjectFromFile(INTERFACEDIR "/newgoldpiece3.sti");

	guiMAPINV                   = AddVideoObjectFromFile(INTERFACEDIR "/mapinv.sti");

	// the upper left corner piece icons
	guiULICONS                  = AddVideoObjectFromFile(INTERFACEDIR "/top_left_corner_icons.sti");

	HandleLoadOfMapBottomGraphics( );

	//Kris:  Added this because I need to blink the icons button.
	guiNewMailIcons             = AddVideoObjectFromFile(INTERFACEDIR "/newemail.sti");

	// graphic for pool inventory
	LoadInventoryPoolGraphic( );

	// load border graphics
	LoadMapBorderGraphics( );

	LoadInterfaceItemsGraphics();
	LoadInterfaceUtilsGraphics();
	LoadMapScreenInterfaceGraphics();
	LoadMapScreenInterfaceMapGraphics();
}


void HandleRemovalOfPreLoadedMapGraphics( void )
{
	DeleteMapBottomGraphics();
	DeleteVideoObject(guiSleepIcon);

	DeleteVideoObject(guiCHARLIST);
	DeleteVideoObject(guiCHARINFO);

	DeleteVideoObject(guiMAPINV);
	DeleteVideoObject(guiULICONS);

	//Kris:  Remove the email icons.
	DeleteVideoObject(guiNewMailIcons);

	// remove inventory pool graphic
	RemoveInventoryPoolGraphic();

	// get rid of border stuff
	DeleteMapBorderGraphics();

	DeleteInterfaceItemsGraphics();
	DeleteInterfaceUtilsGraphics();
	DeleteMapScreenInterfaceGraphics();
	DeleteMapScreenInterfaceMapGraphics();
}


static BOOLEAN CharacterIsInLoadedSectorAndWantsToMoveInventoryButIsNotAllowed(const SOLDIERTYPE* const s)
{
	// char is in loaded sector
	if (s->sSector != gWorldSector)
	{
		return( FALSE );
	}

	// not showing inventory?
	if (!fShowInventoryFlag)
	{
		// nope
		return( FALSE );
	}

	// picked something up?
	if (!fMapInventoryItem) return FALSE; // no

	// only disallow when enemies in sector
	if ( !gTacticalStatus.fEnemyInSector )
	{
		return( FALSE );
	}

	return( TRUE );
}


// how many on team, if less than 2, disable prev/next merc buttons
static void UpdateTheStateOfTheNextPrevMapScreenCharacterButtons(void)
{
	if (gfPreBattleInterfaceActive) return;

	const SOLDIERTYPE* const s = GetSelectedInfoChar();
	if (s == NULL)
	{
		DisableButton(giCharInfoButton[0]);
		DisableButton(giCharInfoButton[1]);
		DisableButton(giMapContractButton);
	}
/* ARM: Commented out at KM's request, it won't reenabled them when coming back from PBI, on Feb. 22, 99
	else if (!fShowInventoryFlag) // make sure that we are in fact showing the mapscreen inventory
	{
		return;
	}
*/
	else
	{
		// standard checks
		bool const enable =
			!fShowDescriptionFlag                                               &&
			GetNumberOfPeopleInCharacterList() >= 2                             &&
			!CharacterIsInLoadedSectorAndWantsToMoveInventoryButIsNotAllowed(s) &&
			!CharacterIsInTransitAndHasItemPickedUp(s);
		EnableButton(giCharInfoButton[0], enable);
		EnableButton(giCharInfoButton[1], enable);
	}
}


static void PrevInventoryMapBtnCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		GoToPrevCharacterInList();
	}
}


static void NextInventoryMapBtnCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		GoToNextCharacterInList();
	}
}


static void CreateDestroyMapCharacterScrollButtons(void)
{
	static BOOLEAN fCreated = FALSE;

	if (fInMapMode && !fCreated)
	{
		const INT16 prio = MSYS_PRIORITY_HIGHEST - 5;

		giCharInfoButton[0] = QuickCreateButtonImg(INTERFACEDIR "/map_screen_bottom_arrows.sti", 11, 4, -1, 6, -1, STD_SCREEN_X + 67, STD_SCREEN_Y + 69, prio, PrevInventoryMapBtnCallback);
		giCharInfoButton[1] = QuickCreateButtonImg(INTERFACEDIR "/map_screen_bottom_arrows.sti", 12, 5, -1, 7, -1, STD_SCREEN_X + 67, STD_SCREEN_Y + 87, prio, NextInventoryMapBtnCallback);

		giCharInfoButton[0]->SetFastHelpText(pMapScreenPrevNextCharButtonHelpText[0]);
		giCharInfoButton[1]->SetFastHelpText(pMapScreenPrevNextCharButtonHelpText[1]);

		fCreated = TRUE;

	}
	else if (!fInMapMode && fCreated)
	{
		RemoveButton( giCharInfoButton[ 0 ]);
		RemoveButton( giCharInfoButton[ 1 ]);

		fCreated = FALSE;
	}
}



void TellPlayerWhyHeCantCompressTime( void )
{
	// if we're locked into paused time compression by some event that enforces that
	if ( PauseStateLocked() )
	{
		SLOGD("Can't compress time, pause state locked (reason %d). OK unless permanent.\n\
			If permanent, take screenshot now, send with *previous* save & describe what happened since.",
			guiLockPauseStateLastReasonId);
	}
	else if (!gfAtLeastOneMercWasHired)
	{
		// no mercs hired, ever
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, pMapScreenJustStartedHelpText, MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
	}
	else if ( !AnyUsableRealMercenariesOnTeam() )
	{
		// no usable mercs left on team
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, pMapErrorString[ 39 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
	else if ( ActiveTimedBombExists() )
	{
		// can't time compress when a bomb is about to go off!
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[STR_LATE_02], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
	}
	else if ( gfContractRenewalSquenceOn )
	{
		SLOGD("Can't compress time while contract renewal sequence is on.");
	}
	else if( fDisableMapInterfaceDueToBattle )
	{
		SLOGD("Can't compress time while disabled due to battle.");
	}
	else if( fDisableDueToBattleRoster )
	{
		SLOGD("Can't compress time while in battle roster.");
	}
	else if ( fMapInventoryItem )
	{
		SLOGD("Can't compress time while still holding an inventory item.");
	}
	else if( fShowMapInventoryPool )
	{
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[STR_LATE_55], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
	}
	// ARM: THIS TEST SHOULD BE THE LAST ONE, BECAUSE IT ACTUALLY RESULTS IN SOMETHING HAPPENING NOW.
	// KM:  Except if we are in a creature lair and haven't loaded the sector yet (no battle yet)
	else if( gTacticalStatus.uiFlags & INCOMBAT || gTacticalStatus.fEnemyInSector )
	{
		if( OnlyHostileCivsInSector() )
		{
			ST::string str;
			ST::string pSectorString;
			pSectorString = GetSectorIDString(gWorldSector, TRUE);
			str = st_format_printf(gzLateLocalizedString[STR_LATE_27], pSectorString);
			DoMapMessageBox( MSG_BOX_BASIC_STYLE, str, MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
		}
		else
		{
			//The NEW non-persistant PBI is used instead of a dialog box explaining why we can't compress time.
			InitPreBattleInterface(0, false);
		}
	}
	else if( PlayerGroupIsInACreatureInfestedMine() )
	{
		DoMapMessageBox(MSG_BOX_BASIC_STYLE, gzLateLocalizedString[STR_LATE_28], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
	}
}


void MapScreenDefaultOkBoxCallback(MessageBoxReturnValue const bExitValue)
{
	if( bExitValue == MSG_BOX_RETURN_OK )
	{
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
}


static void MapSortBtnCallback(GUI_BUTTON *btn, INT32 reason)
{
	// grab the button index value for the sort buttons
	INT32 const iValue = btn->GetUserData();

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ChangeCharacterListSortMethod( iValue );
	}
}


static void AddTeamPanelSortButtonsForMapScreen(void)
{
	INT32 iImageIndex[ MAX_SORT_METHODS ] = { 0, 1, 5, 2, 3, 4 };		// sleep image is out or order (last)

	const char* const filename = GetMLGFilename(MLG_GOLDPIECEBUTTONS);

	for (INT32 i = 0; i < MAX_SORT_METHODS; ++i)
	{
		giMapSortButton[i] = QuickCreateButtonImg(filename, iImageIndex[i], iImageIndex[i] + 6, STD_SCREEN_X + gMapSortButtons[i].iX, STD_SCREEN_Y + gMapSortButtons[i].iY, MSYS_PRIORITY_HIGHEST - 5, MapSortBtnCallback);
		giMapSortButton[i]->SetUserData(i);
		giMapSortButton[i]->SetFastHelpText(wMapScreenSortButtonHelpText[i]);
	}
}


static INT16 CalcLocationValueForChar(const SOLDIERTYPE*);
static INT32 GetContractExpiryTime(const SOLDIERTYPE* s);
static void SwapCharactersInList(INT32 iCharA, INT32 iCharB);


static void SortListOfMercsInTeamPanel(BOOLEAN fRetainSelectedMercs)
{
	INT32 iCounter = 0, iCounterA = 0;
	INT16 sEndSectorA, sEndSectorB;
	INT32 iExpiryTime, iExpiryTimeA;

	SOLDIERTYPE* prev_selected_char = NULL;
	if (fRetainSelectedMercs) prev_selected_char = GetSelectedInfoChar();

	// do the sort
	for( iCounter = 1; iCounter < FIRST_VEHICLE ; iCounter++ )
	{
		const SOLDIERTYPE* const a = gCharactersList[iCounter].merc;
		if (a == NULL) break;

		switch( giSortStateForMapScreenList )
		{
			case( 0 ):
				// by name
				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					const SOLDIERTYPE* const b = gCharactersList[iCounterA].merc;
					if (b == NULL) break;

					if (b->name.compare(a->name) > 0 && iCounterA < iCounter)
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 1 ):
				// by assignment
				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					const SOLDIERTYPE* const b = gCharactersList[iCounterA].merc;
					if (b == NULL) break;

					if (b->bAssignment > a->bAssignment && iCounterA < iCounter)
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
					else if (b->bAssignment == a->bAssignment && iCounterA < iCounter)
					{
						// same assignment

						// if it's in a vehicle
						if (b->bAssignment == VEHICLE)
						{
							// then also compare vehicle IDs
							if (b->iVehicleId > a->iVehicleId && iCounterA < iCounter)
							{
								SwapCharactersInList( iCounter, iCounterA );
							}
						}
					}
				}
				break;

			case( 2 ):
				// by sleep status
				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					const SOLDIERTYPE* const b = gCharactersList[iCounterA].merc;
					if (b == NULL) break;

					if (b->fMercAsleep && !a->fMercAsleep && iCounterA < iCounter)
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 3 ):
				//by location
				sEndSectorA = CalcLocationValueForChar(a);

				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					const SOLDIERTYPE* const b = gCharactersList[iCounterA].merc;
					if (b == NULL) break;

					sEndSectorB = CalcLocationValueForChar(b);

					if( ( sEndSectorB > sEndSectorA ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 4 ):
				// by destination sector
				if (GetLengthOfMercPath(a) == 0)
				{
					sEndSectorA = 9999;
				}
				else
				{
					sEndSectorA = GetLastSectorIdInCharactersPath(a);
				}

				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					const SOLDIERTYPE* const b = gCharactersList[iCounterA].merc;
					if (b == NULL) break;

					if (GetLengthOfMercPath(b) == 0)
					{
						sEndSectorB = 9999;
					}
					else
					{
						sEndSectorB = GetLastSectorIdInCharactersPath(b);
					}

					if( ( sEndSectorB > sEndSectorA ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 5 ):
				iExpiryTime = GetContractExpiryTime(a);

				//by contract expiry
				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					const SOLDIERTYPE* const b = gCharactersList[iCounterA].merc;
					if (b == NULL) break;

					iExpiryTimeA = GetContractExpiryTime(b);

					if( ( iExpiryTimeA > iExpiryTime ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			default:
				SLOGA("Invalid sorting mode for Merc List");
				return;
		}
	}


	if ( fRetainSelectedMercs )
	{
		for (size_t i = 0; i < MAX_CHARACTER_COUNT; ++i)
		{
			const SOLDIERTYPE* const s = gCharactersList[i].merc;
			if (s == NULL || !s->bActive) continue;

			if (prev_selected_char == s) ChangeSelectedInfoChar(static_cast<INT8>(i), FALSE);
		}
	}
	else
	{
		// keep currently selected merc, but reset the selected list (which isn't saved/restored, that's why)
		ResetSelectedListForMapScreen( );
	}


	// reset blinking animations
	SetAllAutoFacesInactive( );

	// dirty the screen parts affected
	fTeamPanelDirty = TRUE;
	fCharacterInfoPanelDirty = TRUE;
}


static void SwapCharactersInList(INT32 iCharA, INT32 iCharB)
{
	const MapScreenCharacterSt temp = gCharactersList[iCharA];
	gCharactersList[iCharA]         = gCharactersList[iCharB];
	gCharactersList[iCharB]         = temp;
}


static void RemoveTeamPanelSortButtonsForMapScreen()
{
	FOR_EACH(GUIButtonRef, i, giMapSortButton) RemoveButton(*i);
}


static void HandleCommonGlowTimer(void)
{
	// grab the current time
	UINT32 uiCurrentTime = GetJA2Clock();

	// only bother checking once flash interval has elapsed
	if( ( uiCurrentTime - guiCommonGlowBaseTime ) >= GLOW_DELAY )
	{
		// update timer so that we only run check so often
		guiCommonGlowBaseTime = uiCurrentTime;

		// set flag to trigger glow higlight updates
		gfGlowTimerExpired = TRUE;
	}
	else
	{
		gfGlowTimerExpired = FALSE;
	}
}


// run through list of grunts and handle awating further orders
static void HandleAssignmentsDoneAndAwaitingFurtherOrders(void)
{
	// update "nothing to do" flags if necessary
	if ( gfReEvaluateEveryonesNothingToDo )
	{
		ReEvaluateEveryonesNothingToDo();
	}

	// grab the current time
	const UINT32 uiCurrentTime = GetJA2Clock();

	// only bother checking once flash interval has elapsed
	if( ( uiCurrentTime - guiFlashAssignBaseTime ) >= ASSIGNMENT_DONE_FLASH_TIME )
	{
		// update timer so that we only run check so often
		guiFlashAssignBaseTime = uiCurrentTime;

		CFOR_EACH_IN_CHAR_LIST(c)
		{
			// toggle and redraw if flash was left ON even though the flag is OFF
			if (c->merc->fDoneAssignmentAndNothingToDoFlag || fFlashAssignDone)
			{
				fFlashAssignDone = !fFlashAssignDone;
				fDrawCharacterList = TRUE;

				// only need to find one
				break;
			}
		}
	}
}


static void DisplayIconsForMercsAsleep(void)
{
	// run throught he list of grunts to see who is asleep and who isn't
	INT32 iCounter;

	// if we are in inventory
	if (fShowInventoryFlag) return;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const SOLDIERTYPE* const pSoldier = gCharactersList[iCounter].merc;
		if (pSoldier == NULL) continue;

		if (pSoldier->bActive && pSoldier->fMercAsleep && CanChangeSleepStatusForSoldier(pSoldier))
		{
			BltVideoObject(guiSAVEBUFFER, guiSleepIcon, 0, STD_SCREEN_X + 125, Y_START + iCounter * (Y_SIZE + 2));
		}
	}
}

//Kris:  Added this function to blink the email icon on top of the laptop button whenever we are in
//       mapscreen and we have new email to read.
static void CheckForAndRenderNewMailOverlay(void)
{
	if( fNewMailFlag )
	{
		if( GetJA2Clock() % 1000 < 667 )
		{
			if (guiMapBottomExitButtons[MAP_EXIT_TO_LAPTOP]->Clicked())
			{ //button is down, so offset the icon
				BltVideoObject(FRAME_BUFFER, guiNewMailIcons, 1, STD_SCREEN_X + 465, STD_SCREEN_Y + 418);
				InvalidateRegion( STD_SCREEN_X + 465, STD_SCREEN_Y + 418, STD_SCREEN_X + 480, STD_SCREEN_Y + 428 );
			}
			else
			{ //button is up, so draw the icon normally
				BltVideoObject(FRAME_BUFFER, guiNewMailIcons, 0, STD_SCREEN_X + 464, STD_SCREEN_Y + 417);
				if (!guiMapBottomExitButtons[MAP_EXIT_TO_LAPTOP]->Enabled())
				{
					SGPRect area = { (UINT16)(STD_SCREEN_X + 463), (UINT16)(STD_SCREEN_Y + 417), (UINT16)(STD_SCREEN_X + 477), (UINT16)(STD_SCREEN_Y + 425) };

					SGPVSurface::Lock l(FRAME_BUFFER);
					Blt16BPPBufferHatchRect(l.Buffer<UINT16>(), l.Pitch(), &area);
				}
				InvalidateRegion( STD_SCREEN_X + 463, STD_SCREEN_Y + 417, STD_SCREEN_X + 481, STD_SCREEN_Y + 430 );

			}
		}
		else
		{ //The blink is now off, so mark the button dirty so that it'll render next frame.
			MarkAButtonDirty( guiMapBottomExitButtons[ MAP_EXIT_TO_LAPTOP ] );
		}
	}
}


static bool CanToggleSelectedCharInventory()
{
	if (gfPreBattleInterfaceActive) return FALSE;

	// already in inventory and an item picked up?
	if (fShowInventoryFlag && fMapInventoryItem)
	{
		return(FALSE);
	}

	const SOLDIERTYPE* const pSoldier = GetSelectedInfoChar();
	// nobody selected?
	if (pSoldier == NULL) return FALSE;

	// does the selected guy have inventory and can we get at it?
	if (!MapCharacterHasAccessibleInventory(*pSoldier)) return FALSE;

	// if not in inventory, and holding an item from sector inventory
	if( !fShowInventoryFlag &&
			(gpItemPointer || fMapInventoryItem) &&
			( gpItemPointerSoldier == NULL ) )
	{
		// make sure he's in that sector
		if ( (pSoldier->sSector.x != sSelMap.x ) ||
			( pSoldier->sSector.y != sSelMap.y ) ||
			( pSoldier->sSector.z != iCurrentMapSectorZ ) ||
			pSoldier->fBetweenSectors )
		{
			return(FALSE);
		}
	}


	// passed!
	return(TRUE);
}


bool MapCharacterHasAccessibleInventory(SOLDIERTYPE const& s)
{
	return
		s.bAssignment         != IN_TRANSIT     &&
		s.bAssignment         != ASSIGNMENT_POW &&
		!IsMechanical(s)                        &&
		s.ubWhatKindOfMercAmI != MERC_TYPE__EPC &&
		s.bLife               >= OKLIFE;
}


static void CheckForInventoryModeCancellation(void)
{
	if ( fShowInventoryFlag || fShowDescriptionFlag )
	{
		// can't bail while player has an item in hand...
		if (fMapInventoryItem) return;

		if ( !CanToggleSelectedCharInventory( ) )
		{
			// get out of inventory mode if it's on!  (could have just bled below OKLIFE)
			if ( fShowInventoryFlag )
			{
				fShowInventoryFlag = FALSE;
				fTeamPanelDirty = TRUE;
			}

			// get out of inventory description if it's on!
			if ( fShowDescriptionFlag )
			{
				DeleteItemDescriptionBox( );
			}
		}
	}
}

void ChangeSelectedMapSector(const SGPSector& sMap)
{
	// ignore while map inventory pool is showing, or else items can be replicated, since sector inventory always applies
	// only to the currently selected sector!!!
	if( fShowMapInventoryPool )
		return;

	if ( gfPreBattleInterfaceActive )
		return;

	if (!IsTheCursorAllowedToHighLightThisSector(sMap))
		return;

	// disallow going underground while plotting (surface) movement
	if (sMap.z != 0 && (bSelectedDestChar != -1 || fPlotForHelicopter))
		return;

	sSelMap = sMap;
	iCurrentMapSectorZ = sMap.z;

	// if going underground while in airspace mode
	if (sMap.z > 0 && fShowAircraftFlag)
	{
		// turn off airspace mode
		ToggleAirspaceMode( );
	}

	fMapPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;

	// also need this, to update the text coloring of mercs in this sector
	fTeamPanelDirty = TRUE;
}


static bool CanChangeDestinationForChar(SOLDIERTYPE& s)
{
	MoveError const ret = CanEntireMovementGroupMercIsInMove(s);
	if (ret == ME_OK) return true;

	ReportMapScreenMovementError(ret);
	return false;
}


BOOLEAN CanExtendContractForSoldier(const SOLDIERTYPE* const s)
{
	Assert(s);
	Assert(s->bActive);

	// if a vehicle, in transit, or a POW
	if (s->uiStatusFlags & SOLDIER_VEHICLE ||
			s->bAssignment == IN_TRANSIT ||
			s->bAssignment == ASSIGNMENT_POW)
	{
		// can't extend contracts at this time
		return (FALSE);
	}

	// mercs below OKLIFE, M.E.R.C. mercs, EPCs, and the Robot use the Contract menu so they can be DISMISSED/ABANDONED!

	// everything OK
	return( TRUE );
}


BOOLEAN CanChangeSleepStatusForSoldier(const SOLDIERTYPE* const pSoldier)
{
	// valid soldier?
	Assert( pSoldier );
	Assert( pSoldier->bActive );

	// if a vehicle, robot, in transit, or a POW
	if (IsMechanical(*pSoldier) ||
			( pSoldier->bAssignment == IN_TRANSIT ) || ( pSoldier->bAssignment == ASSIGNMENT_POW ) )
	{
		// can't change the sleep status of such mercs
		return ( FALSE );
	}

	// if dead
	if( ( pSoldier->bLife <= 0 ) || ( pSoldier->bAssignment == ASSIGNMENT_DEAD ) )
	{
		return ( FALSE );
	}

	// this merc MAY be able to sleep/wake up - we'll allow player to click and find out
	return( TRUE );
}


static void ChangeMapScreenMaskCursor(UINT16 usCursor)
{
	MSYS_SetCurrentCursor( usCursor );
	gMapScreenMaskRegion.ChangeCursor(usCursor);

	if ( usCursor == CURSOR_CHECKMARK )
		fCheckCursorWasSet = TRUE;
	else
		fCheckCursorWasSet = FALSE;

	if ( usCursor == CURSOR_NORMAL )
	{
		if ( !InItemStackPopup( ) )
		{
			// cancel mouse restriction
			FreeMouseCursor();
		}
	}
	else
	{
		// restrict mouse cursor to the map area
		RestrictMouseCursor( &MapScreenRect );
	}
}


static void ExplainWhySkyriderCantFly(void);


static void CancelOrShortenPlottedPath(void)
{
	UINT32 uiReturnValue;

	SGPSector sMap;
	GetMouseMapXY(sMap);

	// check if we are in aircraft mode
	if (fShowAircraftFlag)
	{
		// check for helicopter path being plotted
		if( !fPlotForHelicopter )
			return;

		// if player can't redirect it
		if (!CanHelicopterFly())
		{
			// explain & ignore
			ExplainWhySkyriderCantFly();
			return;
		}


		// try to delete portion of path AFTER the current sector for the helicopter
		uiReturnValue = ClearPathAfterThisSectorForHelicopter(sMap);
	}
	else
	{
		// check for character path being plotted
		if( bSelectedDestChar == -1 )
			return;

		// try to delete portion of path AFTER the current sector for the helicopter
		uiReturnValue = ClearPathAfterThisSectorForCharacter(gCharactersList[bSelectedDestChar].merc, sMap);
	}


	switch ( uiReturnValue )
	{
		case ABORT_PLOTTING:
			AbortMovementPlottingMode( );
			break;

		case PATH_CLEARED:	// movement was canceled
			// message was already issued when path was cleared
			DestinationPlottingCompleted();
			break;

		case PATH_SHORTENED:	// route was shortened but isn't completely gone
			// display "route shortened" message
			BeginMapUIMessage(0, pMapPlotStrings[4]);
			break;

		default:
			Assert( FALSE );
			break;
	}

	// this triggers the path node animation to reset itself back to the first node
	fDeletedNode = TRUE;

	fMapPanelDirty = TRUE;

	fTeamPanelDirty = TRUE;
	fCharacterInfoPanelDirty = TRUE;		// to update ETAs if path reversed or shortened
}


static BOOLEAN HandleCtrlOrShiftInTeamPanel(INT8 bCharNumber)
{
	// check if shift or ctrl held down, if so, set values in list
	if (_KeyDown(CTRL))
	{
		ToggleEntryInSelectedList( bCharNumber );

		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		return( TRUE);
	}
	else if( _KeyDown(SHIFT) )
	{
		// build a list from the bSelectedInfoChar To here, reset everyone

		//empty the list
		ResetSelectedListForMapScreen( );
		// rebuild the list
		BuildSelectedListFromAToB( bSelectedInfoChar, bCharNumber );

		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;

		return( TRUE );
	}

	return( FALSE );
}


static INT32 GetContractExpiryTime(const SOLDIERTYPE* const pSoldier)
{
	if( ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ) || ( pSoldier->ubProfile == SLAY ) )
	{
		return ( pSoldier->iEndofContractTime );
	}
	else
	{
		// never - really high number
		return ( 999999 );
	}
}


SOLDIERTYPE* GetSelectedInfoChar(void)
{
	if (bSelectedInfoChar == -1) return NULL;
	Assert(0 <= bSelectedInfoChar && bSelectedInfoChar < MAX_CHARACTER_COUNT);
	SOLDIERTYPE* const s = gCharactersList[bSelectedInfoChar].merc;
	if (s == NULL) return NULL;
	Assert(s->bActive);
	return s;
}


void ChangeSelectedInfoChar( INT8 bCharNumber, BOOLEAN fResetSelectedList )
{
	Assert( ( bCharNumber >= -1 ) && ( bCharNumber < MAX_CHARACTER_COUNT ) );

	const SOLDIERTYPE* s;
	if (bCharNumber != -1)
	{
		s = gCharactersList[bCharNumber].merc;
		if (s == NULL) return;
	}
	else
	{
		s = NULL;
	}

	// if holding an item
	if (gpItemPointer || fMapInventoryItem)
	{
		// make sure we can give it to this guy, otherwise don't allow the change
		if (s == NULL || !MapscreenCanPassItemToChar(s))
		{
			return;
		}
	}


	if ( fResetSelectedList )
	{
		// reset selections of all listed characters.  Do this even if this guy is already selected.
		// NOTE: this keeps the currently selected info char selected
		ResetSelectedListForMapScreen( );
	}


	// if this is really a change
	if ( bSelectedInfoChar != bCharNumber )
	{
		// if resetting, and another guy was selected
		if ( fResetSelectedList && ( bSelectedInfoChar != -1 ) )
		{
			// deselect previously selected character
			ResetEntryForSelectedList( bSelectedInfoChar );
		}

		bSelectedInfoChar = bCharNumber;

		if ( bCharNumber != -1 )
		{
			// the selected guy must always be ON in the list of selected chars
			SetEntryInSelectedCharacterList( bCharNumber );
		}

		// if we're in the inventory panel
		if ( fShowInventoryFlag )
		{
			// and we're changing to nobody or a guy whose inventory can't be accessed
			if (!s || !MapCharacterHasAccessibleInventory(*s))
			{
				// then get out of inventory mode
				fShowInventoryFlag = FALSE;
			}
		}

		fCharacterInfoPanelDirty = TRUE;

		// if showing sector inventory
		if ( fShowMapInventoryPool )
		{
			// redraw right side to update item hatches
			fMapPanelDirty = TRUE;
		}
	}

	fTeamPanelDirty = TRUE;
}



void CopyPathToAllSelectedCharacters(PathSt* pPath)
{
	// run through list and copy paths for each selected character
	CFOR_EACH_IN_CHAR_LIST(c)
	{
		if (!c->selected) continue;

		SOLDIERTYPE* const pSoldier = c->merc;
		PathSt*      const cur_path = GetSoldierMercPathPtr(pSoldier);
		// skip itself!
		if (cur_path != pPath)
		{
			ClearStrategicPathList(cur_path, 0);
			if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE)
			{
				pVehicleList[pSoldier->bVehicleID].pMercPath = CopyPaths(pPath);
			}
			else if( pSoldier->bAssignment == VEHICLE )
			{
				pVehicleList[pSoldier->iVehicleId].pMercPath = CopyPaths(pPath);
			}
			else
			{
				pSoldier->pMercPath = CopyPaths(pPath);
			}

			// don't use CopyPathToCharactersSquadIfInOne(), it will whack the original pPath by replacing that merc's path!
		}
	}
}



void CancelPathsOfAllSelectedCharacters()
{
	BOOLEAN fSkyriderMsgShown = FALSE;

	// cancel destination for the clicked and ALL other valid & selected characters with a route set
	CFOR_EACH_SELECTED_IN_CHAR_LIST(c)
	{
		SOLDIERTYPE* const pSoldier = c->merc;
		// and he has a route set
		if ( GetLengthOfMercPath( pSoldier ) > 0 )
		{
			// if he's in the chopper, but player can't redirect it
			if (InHelicopter(*pSoldier) && !CanHelicopterFly())
			{
				if ( !fSkyriderMsgShown )
				{
					// explain
					ExplainWhySkyriderCantFly();
					fSkyriderMsgShown = TRUE;
				}

				// don't cancel, ignore
				continue;
			}


			// cancel the entire path (also clears vehicles for any passengers selected, and handles reversing directions)
			if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				CancelPathForVehicle(pVehicleList[pSoldier->bVehicleID], FALSE);
			}
			else
			{
				CancelPathForCharacter( pSoldier );
			}
		}
	}
}


static ST::string ConvertMinTimeToETADayHourMinString(const UINT32 uiTimeInMin)
{
	UINT32 uiDay, uiHour, uiMin;

	uiDay  = ( uiTimeInMin / NUM_MIN_IN_DAY );
	uiHour = ( uiTimeInMin - ( uiDay * NUM_MIN_IN_DAY ) ) / NUM_MIN_IN_HOUR;
	uiMin  = uiTimeInMin - ( ( uiDay * NUM_MIN_IN_DAY ) + ( uiHour * NUM_MIN_IN_HOUR ) );

	// there ain't enough room to show both the day and ETA: and without ETA it's confused as the current time
	//return ST::format("{} {} {}, {02d}:{02d}", pEtaString, pDayStrings, uiDay, uiHour, uiMin);
	//return ST::format("{} {}, {02d}:{02d}", pDayStrings, uiDay, uiHour, uiMin);
	return ST::format("{} {02d}:{02d}", pEtaString, uiHour, uiMin);
}


static INT32 GetGroundTravelTimeOfSoldier(const SOLDIERTYPE* const s)
{
	INT32 iTravelTime = 0;

	// get travel time for the last path segment (stored in pTempCharacterPath)
	iTravelTime = GetPathTravelTimeDuringPlotting( pTempCharacterPath );

	// add travel time for any prior path segments (stored in the selected character's mercpath, but waypoints aren't built)
	iTravelTime += GetPathTravelTimeDuringPlotting(GetSoldierMercPathPtr(s));

	return( iTravelTime );
}


static INT16 CalcLocationValueForChar(const SOLDIERTYPE* const s)
{
	// don't reveal location of POWs!
	if (s->bAssignment == ASSIGNMENT_POW) return 0;

	return s->sSector.AsByte() + s->sSector.z * 1000; // underground: add 1000 per sublevel
}


static void CancelChangeArrivalSectorMode(void)
{
	// "put him in change arrival sector" mode
	gfInChangeArrivalSectorMode = FALSE;

	// change the cursor to that mode
	SetUpCursorForStrategicMap();

	fMapPanelDirty = TRUE;
}


static void MakeMapModesSuitableForDestPlotting(const SOLDIERTYPE* const pSoldier)
{
	CancelSectorInventoryDisplayIfOn(FALSE);

	TurnOnShowTeamsMode();

	if (InHelicopter(*pSoldier))
	{
		if (!fShowAircraftFlag)
		{
			// turn on airspace mode automatically
			ToggleAirspaceMode();
		}
	}
	else
	{
		if (fShowAircraftFlag)
		{
			// turn off airspace mode automatically
			ToggleAirspaceMode();
		}
	}

	// if viewing a different sublevel
	if (iCurrentMapSectorZ != pSoldier->sSector.z)
	{
		// switch to that merc's sublevel
		JumpToLevel(pSoldier->sSector.z);
	}
}


static BOOLEAN AnyMovableCharsInOrBetweenThisSector(const SGPSector& sSector)
{
	CFOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		// POWs, dead guys, guys in transit can't move
		if ( ( pSoldier->bAssignment == IN_TRANSIT ) ||
			( pSoldier->bAssignment == ASSIGNMENT_POW ) ||
			( pSoldier->bAssignment == ASSIGNMENT_DEAD ) ||
			( pSoldier->bLife == 0 ) )
		{
			continue;
		}

		// don't count mercs aboard Skyrider
		if (InHelicopter(*pSoldier)) continue;

		// don't count vehicles - in order for them to move, somebody must be in the sector with them
		if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
		{
			continue;
		}


		// is he here?
		if (pSoldier->sSector == sSector)
		{
			// NOTE that we consider mercs between sectors, mercs < OKLIFE, and sleeping mercs to be "movable".
			// This lets CanCharacterMoveInStrategic() itself report the appropriate error message when character is clicked
			return( TRUE );
		}
	}


	return( FALSE );
}


static UINT8 PlayerMercsInHelicopterSector();


static BOOLEAN RequestGiveSkyriderNewDestination(void)
{
	// should we allow it?
	if (CanHelicopterFly())
	{
		// if not warned already, and chopper empty, but mercs are in this sector
		if ( !gfSkyriderEmptyHelpGiven &&
			( GetNumberInVehicle(GetHelicopter()) == 0 ) &&
			( PlayerMercsInHelicopterSector() > 0 ) )
		{
			DoMapMessageBox(MSG_BOX_BASIC_STYLE, pSkyriderText[2], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback);
			gfSkyriderEmptyHelpGiven = TRUE;
			return( FALSE );
		}

		// say Yo!
		SkyRiderTalk( SKYRIDER_SAYS_HI );

		// start plotting helicopter movement
		fPlotForHelicopter = TRUE;

		// change cursor to the helicopter
		SetUpCursorForStrategicMap();

		// remember the helicopter's current path so we can restore it if need be
		ClearStrategicPathList(g_prev_path, 0);
		g_prev_path = CopyPaths(GetHelicopter().pMercPath);

		// affects Skyrider's dialogue
		SetFactTrue( FACT_SKYRIDER_USED_IN_MAPSCREEN );

		return( TRUE );
	}
	else // not allowed to reroute the chopper right now
	{
		// tell player why not
		ExplainWhySkyriderCantFly();

		return( FALSE );
	}
}


static void ExplainWhySkyriderCantFly(void)
{
	// do we owe him money?
	if( gMercProfiles[ SKYRIDER ].iBalance < 0 )
	{
		// overdue cash
		SkyRiderTalk( OWED_MONEY_TO_SKYRIDER );
		return;
	}

	// is he returning to base?
	if( fHeliReturnStraightToBase )
	{
		// returning to base
		SkyRiderTalk( RETURN_TO_BASE );
		return;
	}

	// grounded by enemies in sector?
	if (!CanHelicopterTakeOff())
	{
		SkyRiderTalk( CHOPPER_NOT_ACCESSIBLE );
		return;
	}

	// Drassen too disloyal to wanna help player?
	if ( CheckFact( FACT_LOYALTY_LOW, SKYRIDER ) )
	{
		SkyRiderTalk( DOESNT_WANT_TO_FLY );
		return;
	}

	// no explainable reason
}


static UINT8 PlayerMercsInHelicopterSector()
{
	GROUP& g = *GetGroup(GetHelicopter().ubMovementGroup);
	if (g.fBetweenSectors) return 0;
	return PlayerMercsInSector(g.ubSector);
}


static void RandomAwakeSelectedMercConfirmsStrategicMove(void);
static void WakeUpAnySleepingSelectedMercsOnFootOrDriving();


static void HandleNewDestConfirmation(const SGPSector& sMap)
{
	UINT8 ubCurrentProgress;


	// if moving the chopper itself, or moving a character aboard the chopper
	if( fPlotForHelicopter )
	{
		// if there are no enemies in destination sector, or we don't know
		if (NumEnemiesInSector(sMap) == 0 ||
			WhatPlayerKnowsAboutEnemiesInSector(sMap) == KNOWS_NOTHING)
		{
			// no problem

			// get current player progress
			ubCurrentProgress = CurrentPlayerProgressPercentage();

			// if we're doing a lot better than last time he said anything
			if ( ( ubCurrentProgress > gubPlayerProgressSkyriderLastCommentedOn ) &&
				( ( ubCurrentProgress - gubPlayerProgressSkyriderLastCommentedOn ) >= MIN_PROGRESS_FOR_SKYRIDER_QUOTE_DOING_WELL ) )
			{
				// kicking ass!
				SkyRiderTalk( THINGS_ARE_GOING_WELL );

				gubPlayerProgressSkyriderLastCommentedOn = ubCurrentProgress;
			}
			// if we're doing noticably worse than last time he said anything
			else if ( ( ubCurrentProgress < gubPlayerProgressSkyriderLastCommentedOn ) &&
					( ( gubPlayerProgressSkyriderLastCommentedOn - ubCurrentProgress ) >= MIN_REGRESS_FOR_SKYRIDER_QUOTE_DOING_BADLY ) )
			{
				// sucking rocks!
				SkyRiderTalk( THINGS_ARE_GOING_BADLY );

				gubPlayerProgressSkyriderLastCommentedOn = ubCurrentProgress;
			}
			else
			{
				// ordinary confirmation quote
				SkyRiderTalk( CONFIRM_DESTINATION );
			}
		}
		else
		{
			// ok, but... you know there are enemies there...
			SkyRiderTalk( BELIEVED_ENEMY_SECTOR );
		}
	}
	else
	{
		RandomAwakeSelectedMercConfirmsStrategicMove( );

		// tell player the route was CONFIRMED
		// NOTE: We don't this this for the helicopter any more, since it clashes with Skyrider's own confirmation msg
		BeginMapUIMessage(0, pMapPlotStrings[1]);
	}


	// wake up anybody who needs to be awake to travel
	WakeUpAnySleepingSelectedMercsOnFootOrDriving();
}


static void RandomAwakeSelectedMercConfirmsStrategicMove(void)
{
	INT32 iCounter;
	SOLDIERTYPE* selected_merc[20];
	UINT8	ubSelectedMercIndex[ 20 ];
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		const MapScreenCharacterSt* const c = &gCharactersList[iCounter];
		if (!c->selected) continue;

		SOLDIERTYPE* const pSoldier = c->merc;
		if (pSoldier->bLife >= OKLIFE &&
				!IsMechanical(*pSoldier)  &&
				!AM_AN_EPC(pSoldier)      &&
				!pSoldier->fMercAsleep)
		{
			selected_merc[ubNumMercs] = pSoldier;
			ubSelectedMercIndex[ ubNumMercs ] = (UINT8)iCounter;

			ubNumMercs++;
		}
	}

	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );

		// select that merc so that when he speaks we're showing his portrait and not someone else
		ChangeSelectedInfoChar( ubSelectedMercIndex[ ubChosenMerc ], FALSE );

		DoMercBattleSound(selected_merc[ubChosenMerc], BATTLE_SOUND_OK1);
		//TacticalCharacterDialogue(selected_merc[ubChosenMerc], ubQuoteNum);
	}
}


static void DestinationPlottingCompleted(void)
{
	// clear previous paths for selected characters and helicopter
	ClearPreviousPaths();

	fPlotForHelicopter = FALSE;
	bSelectedDestChar = - 1;
	giDestHighLine = -1;

	fMapPanelDirty = TRUE;

	// reset cursor
	SetUpCursorForStrategicMap( );

	fJustFinishedPlotting = TRUE;
	fEndPlotting = TRUE;
}


static void HandleMilitiaRedistributionClick(void)
{
	ST::string sString;
	SGPSector sector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ);

	// if on the surface
	if (sector.z == 0)
	{
		INT8 const bTownId = GetTownIdForSector(sector);
		BOOLEAN fTownStillHidden = !IsTownFound(bTownId);

		if( ( bTownId != BLANK_SECTOR ) && !fTownStillHidden )
		{
			if (MilitiaTrainingAllowedInSector(sector))
			{
				fShowTownInfo  = FALSE;
				fMapPanelDirty = TRUE;

				// check if there's combat in any of the town's sectors
				if (CanRedistributeMilitiaInSector(bTownId))
				{
					// Nope, ok, set selected militia town
					sSelectedMilitiaTown = bTownId;
				}
				else
				{
					// can't redistribute militia during combat!
					DoScreenIndependantMessageBox( pMilitiaString[ 2 ], MSG_BOX_FLAG_OK, NULL );
				}
			}
			else
			{
				// can't have militia in this town
				sString = st_format_printf(pMapErrorString[ 31 ], GCM->getTownName(bTownId));
				DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
			}
		}
		else
		{
			INT8 const sam_id = GetSAMIdFromSector(sector);
			if (sam_id != -1 && IsSecretFoundAt(sector.AsByte()))
			{ // Cannot move militia around sam sites.
				DoScreenIndependantMessageBox(pMapErrorString[30], MSG_BOX_FLAG_OK, NULL);
			}
		}
	}
}

static void StartChangeSectorArrivalMode(void)
{
	// "put him in change arrival sector" mode
	gfInChangeArrivalSectorMode = TRUE;

	// redraw map with bullseye removed
	fMapPanelDirty = TRUE;

	// change the cursor to that mode
	SetUpCursorForStrategicMap();

	// give instructions as overlay message
	BeginMapUIMessage(0, pBullseyeStrings[0]);
}


static BOOLEAN CanMoveBullseyeAndClickedOnIt(const SGPSector& sMap)
{
	// if in airspace mode, and not plotting paths
	if (fShowAircraftFlag       &&
			bSelectedDestChar == -1 &&
			!fPlotForHelicopter)
	{
		// don't allow moving bullseye until after initial arrival
		if (!DidGameJustStart())
		{
			// if he clicked on the bullseye, and we're on the surface level
			if (g_merc_arrive_sector == sMap && iCurrentMapSectorZ == 0)
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


static void BullsEyeOrChopperSelectionPopupCallback(MessageBoxReturnValue);


static void CreateBullsEyeOrChopperSelectionPopup(void)
{
	gzUserDefinedButton1 = pHelicopterEtaStrings[ 8 ];
	gzUserDefinedButton2 = pHelicopterEtaStrings[ 9 ];

	// do a BULLSEYE/CHOPPER message box
	DoScreenIndependantMessageBox( pHelicopterEtaStrings[ 7 ], MSG_BOX_FLAG_GENERIC, BullsEyeOrChopperSelectionPopupCallback );
}


static void BullsEyeOrChopperSelectionPopupCallback(MessageBoxReturnValue const ubExitValue)
{
	// button 1 pressed?
	if ( ubExitValue == MSG_BOX_RETURN_YES )
	{
		// chose chopper
		// have to set a flag 'cause first call to RequestGiveSkyriderNewDestination() triggers another msg box & won't work
		gfRequestGiveSkyriderNewDestination = TRUE;
	}
	// button 2 pressed?
	else if( ubExitValue == MSG_BOX_RETURN_NO )
	{
		// chose bullseye
		StartChangeSectorArrivalMode();
	}
}


// Wake up anybody who needs to be awake to travel
static void WakeUpAnySleepingSelectedMercsOnFootOrDriving()
{
	CFOR_EACH_SELECTED_IN_CHAR_LIST(i)
	{
		SOLDIERTYPE& s = *i->merc;
		if (!s.fMercAsleep) continue;

		// On foot or driving?
		if (s.bAssignment >= ON_DUTY && (s.bAssignment != VEHICLE || !SoldierMustDriveVehicle(s, false))) continue;

		/* We should be guaranteed that he CAN wake up to get this far, so report
			* errors, but don't force it */
		bool const success = SetMercAwake(&s, TRUE, FALSE);
		(void)success;
		Assert(success);
	}
}


static void HandlePostAutoresolveMessages(void)
{
	//KM: Autoresolve sets up this global sector value whenever the enemy gains control of a sector.  As soon as
	//we leave autoresolve and enter mapscreen, then this gets called and handles ownership change for the sector.
	//It also brings up a dialog stating to the player what happened, however, the internals of those functions
	//breaks autoresolve and the game crashes after autoresolve is finished.  The value doesn't need to be saved.
	//
	//An additional case is when creatures kill all opposition in the sector.  For each surviving monster, civilians
	//are "virtually" murdered and loyalty hits will be processed.
	if( gsCiviliansEatenByMonsters >= 1 )
	{
		SGPSector sSector(gsEnemyGainedControlOfSectorID);
		AdjustLoyaltyForCivsEatenByMonsters(sSector, gsCiviliansEatenByMonsters);
		gsCiviliansEatenByMonsters = -2;
	}
	else if( gsCiviliansEatenByMonsters == -2 )
	{
		fMapPanelDirty = TRUE;
		gsCiviliansEatenByMonsters = -1;
		gsEnemyGainedControlOfSectorID = -1;
	}
	else if( gsEnemyGainedControlOfSectorID >= 0 )
	{ //bring up the dialog box
		SetThisSectorAsEnemyControlled(SGPSector(gsEnemyGainedControlOfSectorID));
		gsEnemyGainedControlOfSectorID = -2;
	}
	else if( gsEnemyGainedControlOfSectorID == -2 )
	{ //dirty the mapscreen after the dialog box goes away.
		fMapPanelDirty = TRUE;
		gsEnemyGainedControlOfSectorID = -1;
	}
	else if( gbMilitiaPromotions )
	{
		ST::string str = BuildMilitiaPromotionsString();
		DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
}


ST::string GetMapscreenMercAssignmentString(SOLDIERTYPE const& s)
{
	return
		s.bAssignment == VEHICLE ? pShortVehicleStrings[GetVehicle(s.iVehicleId).ubVehicleType] :
		pAssignmentStrings[s.bAssignment];
}


ST::string GetMapscreenMercLocationString(SOLDIERTYPE const& s)
{
	if (s.bAssignment == IN_TRANSIT)
	{ // Show blank
		return "--";
	}
	else if (s.bAssignment == ASSIGNMENT_POW)
	{ // POW - location unknown
		return pPOWStrings[1];
	}
	else
	{ // Put parentheses around it when he's between sectors
		return ST::format(s.fBetweenSectors ? "({}{}{})" : "{}{}{}", pMapVertIndex[s.sSector.y], pMapHortIndex[s.sSector.x], pMapDepthIndex[s.sSector.z]);
	}
}


ST::string GetMapscreenMercDestinationString(SOLDIERTYPE const& s)
{
	/* If dead or POW - has no destination (no longer part of a group, for that
		* matter) */
	if (s.bAssignment != ASSIGNMENT_DEAD &&
			s.bAssignment != ASSIGNMENT_POW  &&
			s.bLife != 0)
	{
		SGPSector sSector;
		if (s.bAssignment == IN_TRANSIT)
		{ // Show the sector he'll be arriving in
			sSector = g_merc_arrive_sector;
		}
		else if (GetLengthOfMercPath(&s) > 0)
		{ // He's going somewhere
			INT16 const sector = GetLastSectorIdInCharactersPath(&s);
			sSector = SGPSector::FromStrategicIndex(sector);
		}
		else // no movement path is set
		{
			if (!s.fBetweenSectors) goto no_destination;
			/* He must be returning to his previous (reversed so as to be the next)
			 * sector, so show that as his destination individual soldiers don't
			 * store previous/next sector coordinates, must go to his group for that
			 */
			GROUP const& g = *GetSoldierGroup(s);
			sSector = g.ubNext;
		}
		return ST::format("{}{}", pMapVertIndex[sSector.y], pMapHortIndex[sSector.x]);
	}
	else
	{
no_destination:
		return ST::null;
	}
}


ST::string GetMapscreenMercDepartureString(SOLDIERTYPE const& s, UINT8* text_colour)
{
	if ((s.ubWhatKindOfMercAmI != MERC_TYPE__AIM_MERC && s.ubProfile != SLAY) ||
			s.bLife == 0)
	{
		return gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION];
	}

	INT32 mins_remaining = s.iEndofContractTime - GetWorldTotalMin();
	if (s.bAssignment == IN_TRANSIT)
	{
		INT32 const contract_length = s.iTotalContractLength * NUM_MIN_IN_DAY;
		if (mins_remaining > contract_length)
		{
			mins_remaining = contract_length;
		}
	}

	if (mins_remaining >= MAP_TIME_UNDER_THIS_DISPLAY_AS_HOURS)
	{ // 3 or more days remain
		INT32 const days_remaining = mins_remaining / (24*60);
		if (text_colour) *text_colour = FONT_LTGREEN;
		return ST::format("{}{}", days_remaining, gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
	}

	// less than 3 days
	INT32 const hours_remaining = mins_remaining > 5 ?  (mins_remaining + 59) / 60 : 0;

	// last 3 days is Red, last 4 hours start flashing red/white!
	if (text_colour)
	{
		*text_colour = mins_remaining <= MINS_TO_FLASH_CONTRACT_TIME && fFlashContractFlag ?
			FONT_WHITE : FONT_RED;
	}
	return ST::format("{}{}", hours_remaining, gpStrategicString[STR_PB_HOURS_ABBREVIATION]);
}


static void InitPreviousPaths(void)
{
	g_prev_path = NULL;
}


void RememberPreviousPathForAllSelectedChars(void)
{
	Assert(0 <= bSelectedDestChar && bSelectedDestChar < MAX_CHARACTER_COUNT);
	ClearStrategicPathList(g_prev_path, 0);
	g_prev_path = CopyPaths(GetSoldierMercPathPtr(gCharactersList[bSelectedDestChar].merc));
}


static bool RestorePath(PathSt*& path, UINT8 const group_id)
{
	if (g_prev_path)
	{
		ClearStrategicPathList(path, group_id);
		path = CopyPaths(g_prev_path);
	}
	else if (path) // if he currently has a path
	{
		// wipe it out!
		path = ClearStrategicPathList(path, group_id);
	}
	else
	{
		return false;
	}

	RebuildWayPointsForGroupPath(path, *GetGroup(group_id));
	return true;
}


static void RestorePreviousPaths(void)
{
	// invalid if we're not plotting movement
	Assert(bSelectedDestChar != -1 || fPlotForHelicopter);

	if (fPlotForHelicopter)
	{
		VEHICLETYPE&   v    = GetHelicopter();
		PathSt** const path = &v.pMercPath;

		if (RestorePath(*path, v.ubMovementGroup))
		{
			CopyPathToAllSelectedCharacters(*path);
		}
	}
	else	// character(s) plotting
	{
		CFOR_EACH_SELECTED_IN_CHAR_LIST(c)
		{
			SOLDIERTYPE* const pSoldier = c->merc;

			PathSt** ppMovePath;
			UINT8    ubGroupId;
			if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
			{
				VEHICLETYPE* const v = &pVehicleList[pSoldier->bVehicleID];
				ppMovePath = &v->pMercPath;
				ubGroupId  = v->ubMovementGroup;
			}
			else if( pSoldier->bAssignment == VEHICLE )
			{
				VEHICLETYPE* const v = &pVehicleList[pSoldier->iVehicleId];
				ppMovePath = &v->pMercPath;
				ubGroupId  = v->ubMovementGroup;
			}
			else if( pSoldier->bAssignment < ON_DUTY )
			{
				ppMovePath = &( pSoldier->pMercPath );
				ubGroupId = pSoldier->ubGroupID;
			}
			else
			{
				// invalid pSoldier - that guy can't possibly be moving, he's on a non-vehicle assignment!
				SLOGA("RestorePreviousPaths: invalid pSoldier: %d", pSoldier->ubID);
				continue;
			}

			RestorePath(*ppMovePath, ubGroupId);
		}
	}
}


static void ClearPreviousPaths(void)
{
	g_prev_path = ClearStrategicPathList(g_prev_path, 0);
}


static void SelectAllCharactersInSquad(INT8 bSquadNumber)
{
	INT8 bCounter;
	BOOLEAN fFirstOne = TRUE;

	// ignore if that squad is empty
	if (SquadIsEmpty(bSquadNumber)) return;

	// select nobody & reset the selected list
	ChangeSelectedInfoChar( -1, TRUE );

	// now select all the soldiers that are in this squad
	for( bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++ )
	{
		const SOLDIERTYPE* const pSoldier = gCharactersList[bCounter].merc;
		if (pSoldier == NULL) continue;

		// if this guy is on that squad or in a vehicle which is assigned to that squad
		// NOTE: There's no way to select everyone aboard Skyrider with this function...
		if (pSoldier->bAssignment == bSquadNumber ||
				IsSoldierInThisVehicleSquad(pSoldier, bSquadNumber))
		{
			if (fFirstOne)
			{
				// make the first guy in the list who is in this squad the selected info char
				ChangeSelectedInfoChar(bCounter, FALSE);

				// select his sector
				ChangeSelectedMapSector(pSoldier->sSector);

				fFirstOne = FALSE;
			}

			SetEntryInSelectedCharacterList(bCounter);
		}
	}
}


BOOLEAN CanDrawSectorCursor(void)
{
	return
		/* !fCursorIsOnMapScrollButtons        && */
		!fShowTownInfo                      &&
		ghTownMineBox == NO_POPUP_BOX       &&
		!fShowUpdateBox                     &&
		GetNumberOfMercsInUpdateList() == 0 &&
		sSelectedMilitiaTown == 0           &&
		!gfMilitiaPopupCreated              &&
		!gfStartedFromMapScreen             &&
		!fShowMapScreenMovementList         &&
		ghMoveBox == NO_POPUP_BOX           &&
		!fMapInventoryItem;
}


static void RestoreMapSectorCursor(const SGPSector& sMap)
{
	INT16 sScreenX, sScreenY;
	Assert(sMap.IsValid());

	GetScreenXYFromMapXY(sMap, &sScreenX, &sScreenY);
	sScreenY -= 1;

	RestoreExternBackgroundRect( sScreenX, sScreenY, DMAP_GRID_X, DMAP_GRID_Y );
}


static void RequestToggleMercInventoryPanel(void)
{
	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}


	if ( !CanToggleSelectedCharInventory() )
	{
		return;
	}

	if (fShowDescriptionFlag)
	{
		// turn off item description
		DeleteItemDescriptionBox( );
	}
	else
	{
		// toggle inventory mode
		fShowInventoryFlag = !fShowInventoryFlag;
	}

	fTeamPanelDirty = TRUE;
}


static void RequestContractMenu(void)
{
	if (gfPreBattleInterfaceActive) return;

	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}


	// in case we have multiple guys selected, turn off everyone but the guy we're negotiating with
	ChangeSelectedInfoChar( bSelectedInfoChar, TRUE );

	SOLDIERTYPE* const s = GetSelectedInfoChar();
	if (s && CanExtendContractForSoldier(s))
	{
		// create
		RebuildContractBoxForMerc(s);

		// reset selected characters
		ResetAllSelectedCharacterModes( );

		bSelectedContractChar = bSelectedInfoChar;
		giContractHighLine = bSelectedContractChar;

		// if not triggered internally
		if (!CheckIfSalaryIncreasedAndSayQuote(s, TRUE))
		{
			// show contract box
			fShowContractMenu = TRUE;

			// stop any dialogue by character
			StopAnyCurrentlyTalkingSpeech( );
		}

		//fCharacterInfoPanelDirty = TRUE;
	}
	else
	{
		// reset selected characters
		ResetAllSelectedCharacterModes( );
	}
}


static void ChangeCharacterListSortMethod(INT32 iValue)
{
	Assert( iValue >= 0 );
	Assert( iValue < MAX_SORT_METHODS );

	if (gfPreBattleInterfaceActive) return;

	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		AbortMovementPlottingMode( );
	}


	giSortStateForMapScreenList = iValue;
	SortListOfMercsInTeamPanel( TRUE );
}


static void MapscreenMarkButtonsDirty(void)
{
	// redraw buttons
	MarkButtonsDirty( );

	// if border buttons are created
	if( !fShowMapInventoryPool )
	{
		// if the attribute assignment menu is showing
		if ( fShowAttributeMenu )
		{
			// don't redraw the town button, it would wipe out a chunk of the attribute menu
			UnMarkButtonDirty( giMapBorderButtons[ MAP_BORDER_TOWN_BTN ] );
		}
	}
}


void LockMapScreenInterface(bool const lock)
{
	class DialogueEventLockMapScreenInterface : public DialogueEvent
	{
		public:
			DialogueEventLockMapScreenInterface(bool const lock) : lock_(lock) {}

			bool Execute()
			{
				fLockOutMapScreenInterface = lock_;
				return false;
			}

		private:
			bool const lock_;
	};

	DialogueEvent::Add(new DialogueEventLockMapScreenInterface(lock));
}


void MakeDialogueEventEnterMapScreen()
{
	class DialogueEventEnterMapScreen : public DialogueEvent
	{
		public:
			bool Execute()
			{
				if (guiCurrentScreen != MAP_SCREEN)
				{
					gfEnteringMapScreen    = TRUE;
					fEnterMapDueToContract = TRUE;
				}
				return false;
			}
	};

	DialogueEvent::Add(new DialogueEventEnterMapScreen());
}


void SetMapCursorItem()
{
	SetMouseCursorFromCurrentItem();
	gMPanelRegion.ChangeCursor(EXTERN_CURSOR);
	fMapInventoryItem = TRUE;
}
