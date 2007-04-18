#include "MapScreen.h"
#include "Gameloop.h"
#include "SGP.h"
#include "VObject.h"
#include "WCheck.h"
#include "WorldDef.h"
#include "Input.h"
#include "Font.h"
#include "ScreenIDs.h"
#include "Screens.h"
#include "Overhead.h"
#include "SysUtil.h"
#include "Event_Pump.h"
#include "Font_Control.h"
#include "Timer_Control.h"
#include "Interface.h"
#include "Handle_UI.h"
#include "Interface_Items.h"
#include "Interface_Utils.h"
#include "Lighting.h"
#include "Cursors.h"
#include "Soldier_Profile.h"
#include "Interface_Cursors.h"
#include "Interface_Panels.h"
#include "Interface_Control.h"
#include "Sys_Globals.h"
#include "Environment.h"
#include "Radar_Screen.h"
#include "Render_Dirty.h"
#include "Utilities.h"
#include "Game_Init.h"
#include "RenderWorld.h"
#include "Finances.h"
#include "Message.h"
#include "VSurface.h"
#include "VObject_Blitters.h"
#include "Faces.h"
#include "PopUpBox.h"
#include "Game_Clock.h"
#include "Items.h"
#include "Cursor_Control.h"
#include "Text.h"
#include "Strategic.h"
#include "StrategicMap.h"
#include "Strategic_Pathing.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Screen_Interface.h"
#include "Assignments.h"
#include "Squads.h"
#include "Merc_Contract.h"
#include "Sound_Control.h"
#include "Strategic_Turns.h"
#include "Dialogue_Control.h"
#include "Map_Screen_Interface_TownMine_Info.h"
#include "PreBattle_Interface.h"
#include "Personnel.h"
#include "Animated_ProgressBar.h"
#include "Queen_Command.h"
#include "LaptopSave.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Vehicles.h"
#include "Map_Screen_Helicopter.h"
#include "GameScreen.h"
#include "Line.h"
#include "English.h"
#include "Fade_Screen.h"
#include "Strategic_Mines.h"
#include "GameVersion.h"
#include "SaveLoadScreen.h"
#include "Options_Screen.h"
#include "Auto_Resolve.h"
#include "Meanwhile.h"
#include "Campaign.h"
#include "Random.h"
#include "Air_Raid.h"
#include "Quests.h"
#include "Town_Militia.h"
#include "Weapons.h"
#include "Player_Command.h"
#include "Multi_Language_Graphic_Utils.h"
#include "HelpScreen.h"
#include "Soldier_Macros.h"
#include "_JA25EnglishText.h"
#include "Cheats.h"
#include "GameSettings.h"
#include "Tactical_Save.h"
#include "Explosion_Control.h"
#include "Creature_Spreading.h"
#include "Strategic_Town_Loyalty.h"
#include "EMail.h"
#include "Video.h"
#include "Debug.h"
#include "Local.h"
#include "Button_System.h"
#include "JAScreens.h"
#include "Stubs.h" // XXX

#ifdef JA2TESTVERSION
#	include "Map_Information.h"
#	include "Soldier_Create.h"
#	include "Strategic_Status.h"
#endif


#define MAX_SORT_METHODS					6

// Cursors
#define SCREEN_CURSOR CURSOR_NORMAL

// Fonts
#define CHAR_FONT BLOCKFONT2 // COMPFONT
#define ETA_FONT BLOCKFONT2

// Colors
#define CHAR_INFO_PANEL_BLOCK_COLOR 60

#define FONT_MAP_DKYELLOW 170

#define CHAR_TITLE_FONT_COLOR 6
#define CHAR_TEXT_FONT_COLOR 5

#define STARTING_COLOR_NUM	5

#define	MAP_TIME_UNDER_THIS_DISPLAY_AS_HOURS			( 3 * 24 * 60 )

#define MIN_KB_TO_DO_PRE_LOAD ( 32 * 1024 )


#define DELAY_PER_FLASH_FOR_DEPARTING_PERSONNEL 500
#define GLOW_DELAY 70
#define ASSIGNMENT_DONE_FLASH_TIME	500

#define	MINS_TO_FLASH_CONTRACT_TIME	(4 * 60)

// Coordinate defines

#define     TOWN_INFO_X           0
#define     TOWN_INFO_Y           1

#define     PLAYER_INFO_X         0
#define     PLAYER_INFO_Y         107

// item description
#define			MAP_ITEMDESC_START_X		  0
#define			MAP_ITEMDESC_START_Y			PLAYER_INFO_Y

#define     INV_REGION_X PLAYER_INFO_X
#define     INV_REGION_Y PLAYER_INFO_Y
#define     INV_REGION_WIDTH 261
#define     INV_REGION_HEIGHT 359-94
#define     INV_BTN_X PLAYER_INFO_X + 217
#define     INV_BTN_Y PLAYER_INFO_Y + 210

#define			MAP_ARMOR_LABEL_X 208
#define			MAP_ARMOR_LABEL_Y 180
#define			MAP_ARMOR_X									209
#define			MAP_ARMOR_Y									189
#define			MAP_ARMOR_PERCENT_X 229
#define			MAP_ARMOR_PERCENT_Y 190

#define			MAP_WEIGHT_LABEL_X 173
#define			MAP_WEIGHT_LABEL_Y 256
#define			MAP_WEIGHT_X								176
#define			MAP_WEIGHT_Y								266
#define			MAP_WEIGHT_PERCENT_X 196
#define			MAP_WEIGHT_PERCENT_Y 266

#define			MAP_CAMMO_LABEL_X						178
#define			MAP_CAMMO_LABEL_Y						283
#define			MAP_CAMMO_X									176
#define			MAP_CAMMO_Y									292
#define			MAP_CAMMO_PERCENT_X					196
#define			MAP_CAMMO_PERCENT_Y					293

#define			MAP_PERCENT_WIDTH						20
#define			MAP_PERCENT_HEIGHT					10

#define			MAP_INV_STATS_TITLE_FONT_COLOR				6
#define			MAP_INV_STATS_TEXT_FONT_COLOR					5

#define     PLAYER_INFO_FACE_START_X    9
#define     PLAYER_INFO_FACE_START_Y    17
#define     PLAYER_INFO_FACE_END_X			60
#define     PLAYER_INFO_FACE_END_Y			76

#define     INV_BODY_X 71
#define     INV_BODY_Y 116


#define     NAME_X                11
#define     NAME_WIDTH            62 - NAME_X
#define     ASSIGN_X              67
#define     ASSIGN_WIDTH          118 - ASSIGN_X
#define			SLEEP_X								123
#define			SLEEP_WIDTH						142 - SLEEP_X
#define     LOC_X                 147
#define     LOC_WIDTH             179 - LOC_X
#define     DEST_ETA_X            184
#define     DEST_ETA_WIDTH        217 - DEST_ETA_X
#define     TIME_REMAINING_X      222
#define     TIME_REMAINING_WIDTH  250 - TIME_REMAINING_X
#define     CLOCK_X_START         463 - 18
#define     CLOCK_Y_START         298
#define     DEST_PLOT_X						463
#define     DEST_PLOT_Y						345
#define     CLOCK_ETA_X           463 - 15 + 6 + 30
#define     CLOCK_HOUR_X_START    463 + 25 + 30
#define     CLOCK_MIN_X_START     463 + 45 + 30


// contract
#define CONTRACT_X      185
#define CONTRACT_Y      50
//#define CONTRACT_WIDTH  63
//#define CONTRACT_HEIGHT 10

// trash can
#define TRASH_CAN_X 176
#define TRASH_CAN_Y 211 + PLAYER_INFO_Y
#define TRASH_CAN_WIDTH 193 - 165
#define TRASH_CAN_HEIGHT 239 - 217

//Text offsets
#define Y_OFFSET 2

// The boxes defines
#define TRAIN_Y_OFFSET     53
#define TRAIN_X_OFF        65
#define TRAIN_WID          80
#define TRAIN_HEIG         47
#define STRING_X_OFFSET    10
#define STRING_Y_OFFSET     5
#define POP_UP_BOX_X      120
#define POP_UP_BOX_Y        0
#define POP_UP_BOX_WIDTH   60
#define POP_UP_BOX_HEIGHT  100
#define MOUSE_PTR_Y_OFFSET  3
#define POP_UP_Y_OFFSET     3
#define TRAIN_TEXT_Y_OFFSET 4


// char stat positions
#define STR_X ( 112 )
#define STR_Y 42
#define DEX_X STR_X
#define DEX_Y 32
#define AGL_X STR_X
#define AGL_Y 22
#define LDR_X STR_X
#define LDR_Y 52
#define WIS_X STR_X
#define WIS_Y 62
#define LVL_X ( 159 )
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

#define PIC_NAME_X 8
#define PIC_NAME_Y (66 + 3)
#define PIC_NAME_WID 60 - PIC_NAME_X
#define PIC_NAME_HEI 75 - PIC_NAME_Y
#define CHAR_NAME_X 14
#define CHAR_NAME_Y (2 + 3)
#define CHAR_NAME_WID 164 - CHAR_NAME_X
#define CHAR_NAME_HEI 11 - CHAR_NAME_Y
#define CHAR_LOC_X    76
#define CHAR_LOC_Y    84
#define CHAR_LOC_WID  16
#define CHAR_LOC_HEI  9
#define CHAR_TIME_REMAINING_X		207
#define CHAR_TIME_REMAINING_Y		65
#define CHAR_TIME_REMAINING_WID 258 - CHAR_TIME_REMAINING_X
#define CHAR_TIME_REMAINING_HEI GetFontHeight(CHAR_FONT)
#define CHAR_SALARY_X						CHAR_TIME_REMAINING_X
#define CHAR_SALARY_Y						79
#define CHAR_SALARY_WID					CHAR_TIME_REMAINING_WID - 8		// for right justify
#define CHAR_SALARY_HEI					CHAR_TIME_REMAINING_HEI
#define CHAR_MEDICAL_X					CHAR_TIME_REMAINING_X
#define CHAR_MEDICAL_Y					93
#define CHAR_MEDICAL_WID				CHAR_TIME_REMAINING_WID - 8		// for right justify
#define CHAR_MEDICAL_HEI				CHAR_TIME_REMAINING_HEI
#define CHAR_ASSIGN_X 182
#define CHAR_ASSIGN1_Y 18
#define CHAR_ASSIGN2_Y 31
#define CHAR_ASSIGN_WID 257 - 178
#define CHAR_ASSIGN_HEI 39 - 29
#define CHAR_HP_X 133
#define CHAR_HP_Y 77 + 3
#define CHAR_HP_WID 175 - CHAR_HP_X
#define CHAR_HP_HEI  90 - CHAR_HP_Y
#define CHAR_MORALE_X 133
#define CHAR_MORALE_Y 91 + 3
#define CHAR_MORALE_WID 175 - CHAR_MORALE_X
#define CHAR_MORALE_HEI 101 - CHAR_MORALE_Y

#define CROSS_X 195
#define CROSS_Y 83
#define CROSS_HEIGHT 20
#define CROSS_WIDTH  20
#define CHAR_PAY_X 150
#define CHAR_PAY_Y 80+4
#define CHAR_PAY_HEI GetFontHeight(CHAR_FONT)
#define CHAR_PAY_WID CROSS_X-CHAR_PAY_X
#define SOLDIER_PIC_X 9
#define SOLDIER_PIC_Y 20
#define SOLDIER_HAND_X 6
#define SOLDIER_HAND_Y 81
//#define	TM_INV_WIDTH								58
//#define	TM_INV_HEIGHT								23

#define CLOCK_X 554
#define CLOCK_Y 459


#define RGB_WHITE				( FROMRGB( 255, 255, 255 ) )
#define RGB_YELLOW			( FROMRGB( 255, 255,   0 ) )
#define RGB_NEAR_BLACK	( FROMRGB(   0,   0,   1 ) )


// ARM: NOTE that these map "events" are never actually saved in a player's game in any way
enum
{
	MAP_EVENT_NONE,
	MAP_EVENT_CLICK_SECTOR,
	MAP_EVENT_PLOT_PATH,
	MAP_EVENT_CANCEL_PATH,

#ifdef JA2BETAVERSION
	MAP_EVENT_VIEWAI
#endif
};


struct rgbcolor
{
	UINT8 ubRed;
	UINT8 ubGreen;
	UINT8 ubBlue;
};

typedef struct rgbcolor RGBCOLOR;


struct lineoftext
{
	STR16 pLineText;
	UINT32 uiFont;
	struct lineoftext *pNext;
};

typedef struct lineoftext LineText;
typedef LineText *LineTextPtr;


struct popbox
{
 UINT16 usTopX;
 UINT16 usTopY;
 UINT16 usWidth;
 UINT16 usHeight;
 LineTextPtr pBoxText;
 struct popbox *pNext;
};


RGBCOLOR GlowColorsA[]={
	{0,0,0},
	{25,0,0},
	{50,0,0},
	{75,0,0},
	{100,0,0},
	{125,0,0},
	{150,0,0},
	{175,0,0},
	{200,0,0},
	{225,0,0},
	{250,0,0},
};
/* unused
RGBCOLOR GlowColorsB[]={
	{0,0,0},
	{25,25,0},
	{50,50,0},
	{75,75,0},
	{100,100,0},
	{125,125,0},
	{150,150,0},
	{175,175,0},
	{200,200,0},
	{225,225,0},
	{255,255,0},
};
RGBCOLOR GlowColorsC[]={
	{0,0,0},
	{25,0,25},
	{50,0,50},
	{75,0,75},
	{100,0,100},
	{125,0,125},
	{150,0,150},
	{175,0,175},
	{200,0,200},
	{225,0,225},
	{255,0,255},
};
*/


SGPPoint gMapSortButtons[ MAX_SORT_METHODS ]={
	{12,125},
	{68,125},
	{124,125},
	{148,125},
	{185,125},
	{223,125},
};


// map screen's inventory panel pockets - top right corner coordinates
INV_REGION_DESC gMapScreenInvPocketXY[] =
{
	{ 204,116 }, // HELMETPOS
	{ 204,145 }, // VESTPOS
	{ 204,205 }, // LEGPOS,
	{ 21,	116 }, // HEAD1POS
	{ 21,	140 }, // HEAD2POS
	{ 21,	194 }, // HANDPOS,
	{ 21,	218 }, // SECONDHANDPOS
	{ 98,	251 }, // BIGPOCK1
	{ 98,	275 }, // BIGPOCK2
	{ 98,	299 }, // BIGPOCK3
	{ 98,	323 }, // BIGPOCK4
	{ 22,	251 }, // SMALLPOCK1
	{ 22,	275 }, // SMALLPOCK2
	{ 22,	299 }, // SMALLPOCK3
	{ 22,	323 }, // SMALLPOCK4
	{ 60,	251 }, // SMALLPOCK5
	{ 60,	275 }, // SMALLPOCK6
	{ 60,	299 }, // SMALLPOCK7
	{ 60,	323 }  // SMALLPOCK8
};

INV_REGION_DESC gSCamoXY =
{
	INV_BODY_X, INV_BODY_Y		// X, Y Location of Map screen's Camouflage region
};


// GLOBAL VARIABLES (OURS)


BOOLEAN	fFlashAssignDone = FALSE;
BOOLEAN	fInMapMode = FALSE;
BOOLEAN fMapPanelDirty=TRUE;
BOOLEAN fTeamPanelDirty = TRUE;
BOOLEAN fCharacterInfoPanelDirty = TRUE;
BOOLEAN	gfLoadPending = FALSE;
BOOLEAN fReDrawFace=FALSE;
BOOLEAN fFirstTimeInMapScreen = TRUE;
BOOLEAN fShowInventoryFlag = FALSE;
BOOLEAN fMapInventoryItem=FALSE;
BOOLEAN fShowDescriptionFlag=FALSE;

// are the graphics for mapscreen preloaded?
BOOLEAN fPreLoadedMapGraphics = FALSE;

BOOLEAN gfHotKeyEnterSector = FALSE;
static BOOLEAN fOneFrame = FALSE;
BOOLEAN fShowFaceHightLight = FALSE;
BOOLEAN fShowItemHighLight = FALSE;
BOOLEAN gfAllowSkyriderTooFarQuote = FALSE;
BOOLEAN fJustFinishedPlotting = FALSE;

// for the flashing of the contract departure time...for when mercs are leaving in an hour or less
BOOLEAN fFlashContractFlag = FALSE;

BOOLEAN fShowTrashCanHighLight = FALSE;

// the flags for display of pop up boxes/menus
BOOLEAN fEndPlotting = FALSE;

BOOLEAN gfInConfirmMapMoveMode = FALSE;
BOOLEAN gfInChangeArrivalSectorMode = FALSE;

// redraw character list
BOOLEAN fDrawCharacterList = TRUE;

// was the cursor set to the checkmark?
BOOLEAN fCheckCursorWasSet = FALSE;

BOOLEAN fShowingMapDisableBox = FALSE;
BOOLEAN fShowFrameRate = FALSE;
//BOOLEAN fMapExitDueToMessageBox = FALSE;
BOOLEAN fEndShowInventoryFlag = FALSE;

// draw the temp path
BOOLEAN fDrawTempPath = TRUE;

BOOLEAN gfCharacterListInited = FALSE;

BOOLEAN gfGlowTimerExpired = FALSE;

//not required to be saved.  The flag is set to allow mapscreen to render once, then transition the
//current tactical battle into autoresolve.
BOOLEAN gfTransitionMapscreenToAutoResolve = FALSE;

BOOLEAN gfSkyriderEmptyHelpGiven = FALSE;

BOOLEAN gfRequestGiveSkyriderNewDestination = FALSE;

BOOLEAN gfFirstMapscreenFrame = FALSE;

BOOLEAN gfMapPanelWasRedrawn = FALSE;


UINT8 gubMAP_HandInvDispText[ NUM_INV_SLOTS ];

// currently selected character's list index
INT8 bSelectedInfoChar = -1;

// map sort button images
INT32 giMapSortButtonImage[ MAX_SORT_METHODS ] = { -1, -1, -1, -1, -1, -1 };
INT32 giMapSortButton[ MAX_SORT_METHODS ] = { -1, -1, -1, -1, -1, -1 };

INT32 giCharInfoButtonImage[ 2 ];
INT32 giCharInfoButton[ 2 ] = { -1, -1 };

INT32 giMapInvButtonDoneImage;
INT32 giMapInvDoneButton = -1;

INT32 giMapContractButton = -1;
INT32 giMapContractButtonImage;

//INT32 giMapInvButton = -1;
//INT32 giMapInvButtonImage;

INT32 giSortStateForMapScreenList = 0;

INT32 giCommonGlowBaseTime = 0;
INT32 giFlashAssignBaseTime = 0;
INT32 giFlashContractBaseTime = 0;
UINT32 guiFlashCursorBaseTime = 0;
INT32 giPotCharPathBaseTime = 0;

/*
// next and prev
static INT32 giMapInvPrev;
static INT32 giMapInvNext;
INT32 giMapInvPrevImage;
INT32 giMapInvNextImage;
*/
extern UINT32 guiVObjectSize;
extern UINT32 guiVSurfaceSize;

extern UINT8 gubHandPos;
extern UINT16 gusOldItemIndex;
extern UINT16 gusNewItemIndex;
extern BOOLEAN gfDeductPoints;

extern void CleanUpStack( OBJECTTYPE * pObj, OBJECTTYPE * pCursorObj );

UINT32	guiCHARLIST;
UINT32	guiCHARINFO;
UINT32	guiSleepIcon;
UINT32	guiCROSS;
UINT32	guiMAPINV;
UINT32	guiMapInvSecondHandBlockout;
UINT32	guiULICONS;
UINT32	guiNewMailIcons;
UINT32	guiLEVELMARKER;		// the white rectangle highlighting the current level on the map border


// misc mouse regions
MOUSE_REGION gCharInfoFaceRegion;
MOUSE_REGION gCharInfoHandRegion;
MOUSE_REGION gMPanelRegion;
MOUSE_REGION gMapViewRegion;
MOUSE_REGION gMapScreenMaskRegion;
MOUSE_REGION gTrashCanRegion;

// mouse regions for team info panel
MOUSE_REGION gTeamListNameRegion[ MAX_CHARACTER_COUNT ];
MOUSE_REGION gTeamListAssignmentRegion[ MAX_CHARACTER_COUNT ];
MOUSE_REGION gTeamListSleepRegion[ MAX_CHARACTER_COUNT ];
MOUSE_REGION gTeamListLocationRegion[ MAX_CHARACTER_COUNT ];
MOUSE_REGION gTeamListDestinationRegion[ MAX_CHARACTER_COUNT ];
MOUSE_REGION gTeamListContractRegion[ MAX_CHARACTER_COUNT ];


OBJECTTYPE		gItemPointer;
SOLDIERTYPE		*gpItemPointerSoldier;

PathStPtr gpCharacterPreviousMercPath[ MAX_CHARACTER_COUNT ];
PathStPtr gpHelicopterPreviousMercPath = NULL;


// GLOBAL VARIABLES (EXTERNAL)


extern BOOLEAN fDeletedNode;
extern BOOLEAN gfRenderPBInterface;
extern BOOLEAN fMapScreenBottomDirty;
extern BOOLEAN fResetTimerForFirstEntryIntoMapScreen;
extern BOOLEAN gfStartedFromMapScreen;

// the selected list of mercs
extern BOOLEAN fSelectedListOfMercsForMapScreen[ MAX_CHARACTER_COUNT ];

extern INT32 iDialogueBox;
extern INT32 giMapInvDescButton;

extern UINT32 guiBrownBackgroundForTeamPanel;

// the town mine info box
extern INT32 ghTownMineBox;
// border and bottom buttons
extern INT32 giMapBorderButtons[];

// the mine icon
extern UINT32 guiMINEICON;
extern UINT32 guiSecItemHiddenVO;

extern UINT32	guiUIMessageTimeDelay;


extern PathStPtr pTempCharacterPath;
extern PathStPtr pTempHelicopterPath;

extern BOOLEAN gfAutoAIAware;
extern void HandlePreBattleInterfaceStates();

// the title for the contract button on the character info panel in the upper left portion of the mapscreen
extern STR16 pContractButtonString[];
extern STR16 pBullseyeStrings[];

extern OBJECTTYPE	*gpItemDescObject;


// faces stuff
extern FACETYPE	*gpCurrentTalkingFace;
//extern BOOLEAN	gfFacePanelActive;

// externs for highlighting of ammo/weapons
extern UINT32 guiMouseOverItemTime;
extern BOOLEAN gfCheckForMouseOverItem;
extern INT8 gbCheckForMouseOverItemPos;

//Autoresolve sets this variable which defaults to -1 when not needed.
extern INT16 gsEnemyGainedControlOfSectorID;
extern INT16 gsCiviliansEatenByMonsters;

extern BOOLEAN			gfFadeOutDone;

extern UINT32 guiPendingScreen;

extern CHAR16		gzUserDefinedButton1[ 128 ];
extern CHAR16		gzUserDefinedButton2[ 128 ];

extern BOOLEAN gfMilitiaPopupCreated;

#ifdef JA2TESTVERSION
	extern INT16 MSYS_CurrentMX;
	extern INT16 MSYS_CurrentMY;
#endif


BOOLEAN GetMouseMapXY( INT16 *psMapWorldX, INT16 *psMapWorldY );

void EndConfirmMapMoveMode( void );

void CancelMapUIMessage( void );


void RenderMapRegionBackground( void );

extern BOOLEAN HandleNailsVestFetish( SOLDIERTYPE *pSoldier, UINT32 uiHandPos, UINT16 usReplaceItem );

// Pop Up Boxes
void CreateContractBox( SOLDIERTYPE *pCharacter );
void CreateMercRemoveAssignBox( void );

void DetermineWhichAssignmentMenusCanBeShown( void );

// screen mask for pop up menus
void ClearScreenMaskForMapScreenExit( void );


extern void KeyRingItemPanelButtonCallback( MOUSE_REGION *pRegion, INT32 iReason );

BOOLEAN ContinueDialogue(SOLDIERTYPE *pSoldier, BOOLEAN fDone );
extern void CreateDestroyMilitiaSectorButtons( void );


// create/destroy inventory button as needed
void CreateDestroyMapInvButton();


void MapScreenDefaultOkBoxCallback( UINT8 bExitValue );

void RememberPreviousPathForAllSelectedChars( void );

BOOLEAN CanDrawSectorCursor( void );


extern BOOLEAN CanRedistributeMilitiaInSector( INT16 sClickedSectorX, INT16 sClickedSectorY, INT8 bClickedTownId );

extern INT32 GetNumberOfMercsInUpdateList( void );


#ifdef JA2TESTVERSION
void TestDumpStatChanges( void );
void DumpSectorDifficultyInfo( void );
void DumpItemsList( void );
#endif

#ifdef JA2DEMO
//void MapScreenDemoOkBoxCallback( UINT8 bExitValue );
void DisplayExitToTacticalGlowDuringDemo( void );
#endif



// the tries to select a mapscreen character by his soldier ID
BOOLEAN SetInfoChar( UINT8 ubID )
{
	INT8 bCounter;

	for ( bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++)
	{
		// skip invalid characters
		if ( gCharactersList[ bCounter ].fValid == TRUE )
		{
			if ( gCharactersList[ bCounter ].usSolID == (UINT16)ubID )
			{
				ChangeSelectedInfoChar( bCounter, TRUE );
				return( TRUE );
			}
		}
	}

	return ( FALSE );
}


static void DisplayDestinationOfCurrentDestMerc(void)
{
	// will display the dest of the current dest merc
	CHAR16 sString[ 32 ];
	INT16 sX, sY;
	INT16 sSector;

	SetFont( MAP_SCREEN_FONT );

	sSector = GetLastSectorIdInCharactersPath( &Menptr[gCharactersList[ bSelectedDestChar ].usSolID] );

	SetBoxForeground(ghVehicleBox, FONT_LTGREEN);
	SetBoxBackground(ghVehicleBox, FONT_BLACK);

	swprintf( sString, lengthof(sString), L"%ls%ls", pMapVertIndex[ sSector / MAP_WORLD_X ], pMapHortIndex[ sSector % MAP_WORLD_X ] );
	FindFontCenterCoordinates(DEST_PLOT_X, DEST_PLOT_Y ,70 ,GetFontHeight( MAP_SCREEN_FONT ) ,sString , MAP_SCREEN_FONT, &sX, &sY);

	RestoreExternBackgroundRect( DEST_PLOT_X, DEST_PLOT_Y ,70 ,GetFontHeight( MAP_SCREEN_FONT ) );
	mprintf( sX, sY, sString );
}


static void ContractBoxGlow(void)
{
/* Why not?
 static INT32 iColorNum=10;
 static BOOLEAN fDelta=FALSE;
 static BOOLEAN fOldContractGlow = FALSE;
 UINT16 usColor;
 UINT32 uiDestPitchBYTES;
 UINT8	*pDestBuf;


	// stopped glowing?
	if( ( fGlowContractRegion == FALSE )&&( ( fOldContractGlow == TRUE )||( fResetContractGlow== TRUE ) ) )
	{
		// restore background
		// RestoreExternBackgroundRect( CONTRACT_X, CONTRACT_Y, CONTRACT_WIDTH+1, CONTRACT_HEIGHT+1 );

		// reset old
		fOldContractGlow = FALSE;
	}

	// not glowing right now, leave
	if( ( fGlowContractRegion == FALSE )||( fResetContractGlow == TRUE ) )
	{
		// reset color rotation
		iColorNum =0;
		fDelta = TRUE;

		// reset
		fResetContractGlow = FALSE;
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

	usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);
	RectangleDraw( TRUE, CONTRACT_X, CONTRACT_Y, CONTRACT_X+CONTRACT_WIDTH, CONTRACT_Y+CONTRACT_HEIGHT, usColor, pDestBuf );
	InvalidateRegion(CONTRACT_X, CONTRACT_Y, CONTRACT_X+CONTRACT_WIDTH+1, CONTRACT_Y+CONTRACT_HEIGHT+1);
	UnLockVideoSurface( FRAME_BUFFER );

	// restore background
	if((iColorNum==0)||(iColorNum==1))
		RestoreExternBackgroundRect( CONTRACT_X, CONTRACT_Y, CONTRACT_WIDTH+1, CONTRACT_HEIGHT+1 );
*/
}


static void ContractListRegionBoxGlow(UINT16 usCount)
{
 static INT32 iColorNum =10;
 static BOOLEAN fDelta=FALSE;
 UINT16 usColor;
 UINT32 uiDestPitchBYTES;
 UINT8	*pDestBuf;
 INT16 usY = 0;
 INT16 sYAdd = 0;


	// if not glowing right now, leave
	if( ( giContractHighLine == -1 )||( fResetContractGlow == TRUE ) || fShowInventoryFlag )
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
	usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);
	RectangleDraw( TRUE, TIME_REMAINING_X, usY, TIME_REMAINING_X + TIME_REMAINING_WIDTH, usY + GetFontHeight( MAP_SCREEN_FONT ) + 2, usColor, pDestBuf );
	InvalidateRegion(TIME_REMAINING_X - 1, usY, TIME_REMAINING_X + TIME_REMAINING_WIDTH + 1, usY + GetFontHeight( MAP_SCREEN_FONT ) + 3 );
	UnLockVideoSurface( FRAME_BUFFER );

/*
	// restore background
	if((iColorNum==0)||(iColorNum==1))
		RestoreExternBackgroundRect( CONTRACT_X, CONTRACT_Y, CONTRACT_WIDTH+1, CONTRACT_HEIGHT+1 );
*/
}


static void GlowFace(void)
{
 static INT32 iColorNum =10;
 static BOOLEAN fDelta=FALSE;
 static BOOLEAN fOldFaceGlow = FALSE;
 UINT16 usColor;
 UINT32 uiDestPitchBYTES;
 UINT8	*pDestBuf;

	// not glowing right now, leave
	if( fShowFaceHightLight == FALSE )
	{
		iColorNum =0;
		fDelta = TRUE;

		if( fOldFaceGlow == TRUE )
		{
			RestoreExternBackgroundRect( 9, 18, ( UINT16 )( 61 - 9 ), ( UINT16 )( 64 - 18 ) );
		}

		fOldFaceGlow = FALSE;
		return;
	}

	// if not ready to change glow phase yet, leave
	if ( !gfGlowTimerExpired )
		return;


	fOldFaceGlow = TRUE;

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

	// glow contract box
	usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);
	RectangleDraw( TRUE, 9, 18, 60, 63 , usColor, pDestBuf );
	InvalidateRegion( 9, 18, 61, 64 );
	UnLockVideoSurface( FRAME_BUFFER );

	// restore background
	if((iColorNum==0)||(iColorNum==1))
		RestoreExternBackgroundRect( 9, 18, ( UINT16 )( 61 - 9 ), ( UINT16 )( 64 - 18 ) );
}


static void RenderHandPosItem(void);


static void GlowItem(void)
{
 static INT32 iColorNum =10;
 static BOOLEAN fDelta=FALSE;
 static BOOLEAN fOldItemGlow = FALSE;
 UINT16 usColor;
 UINT32 uiDestPitchBYTES;
 UINT8	*pDestBuf;

	// not glowing right now, leave
	if( fShowItemHighLight == FALSE )
	{
		iColorNum =0;
		fDelta = TRUE;

		if( fOldItemGlow == TRUE )
		{
			RestoreExternBackgroundRect( 3, 80, ( UINT16 )( 65 - 3 ), ( UINT16 )( 105 - 80 ) );
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
		RestoreExternBackgroundRect( 3, 80, ( UINT16 )( 65 - 3 ), ( UINT16 )( 105 - 80 ) );
		RenderHandPosItem();
	}

	// glow contract box
	usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);
	RectangleDraw( TRUE, 3, 80, 64, 104 , usColor, pDestBuf );
	InvalidateRegion( 3, 80, 65, 105 );
	UnLockVideoSurface( FRAME_BUFFER );
}


static void GlowTrashCan(void)
{
 static INT32 iColorNum =10;
 static BOOLEAN fDelta=FALSE;
 static BOOLEAN fOldTrashCanGlow = FALSE;
 UINT16 usColor;
 UINT32 uiDestPitchBYTES;
 UINT8	*pDestBuf;

	if( fShowInventoryFlag == FALSE )
	{
		fShowTrashCanHighLight = FALSE;
	}

	// not glowing right now, leave
	if( fShowTrashCanHighLight == FALSE )
	{
		iColorNum =0;
		fDelta = TRUE;

		if( fOldTrashCanGlow == TRUE )
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
	usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);
	RectangleDraw( TRUE, TRASH_CAN_X, TRASH_CAN_Y , TRASH_CAN_X + TRASH_CAN_WIDTH, TRASH_CAN_Y + TRASH_CAN_HEIGHT , usColor, pDestBuf );
	InvalidateRegion( TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH + 1, TRASH_CAN_Y + TRASH_CAN_HEIGHT + 1 );
	UnLockVideoSurface( FRAME_BUFFER );

	// restore background
	if((iColorNum==0)||(iColorNum==1))
		RestoreExternBackgroundRect( TRASH_CAN_X, TRASH_CAN_Y, ( UINT16 )( TRASH_CAN_WIDTH + 2 ), ( UINT16 )( TRASH_CAN_HEIGHT + 2 ) );
}



void DrawFace( INT16 sCharNumber )
{
	static INT16 sOldId = -1;

	// draws the face of the currently selected merc, being displayed int he upper left hand corner

	// grab the soldier
	const SOLDIERTYPE* pSoldier = NULL;
	if( bSelectedInfoChar != -1 )
	{
		if( gCharactersList[ bSelectedInfoChar ].fValid )
		{
			pSoldier = GetSoldier(gCharactersList[bSelectedInfoChar].usSolID);
		}
	}

	if( pSoldier == NULL )
	{
		return;
	}


	if( ( gCharactersList[ bSelectedInfoChar ].usSolID == sOldId )&&( fReDrawFace == FALSE ) )
	{
		// are the same, return
		return;
	}

	// get old id value
	sOldId = gCharactersList[ bSelectedInfoChar ].usSolID;

	// reset redraw of face
	fReDrawFace = FALSE;

	// render their face
	RenderSoldierFace( pSoldier, SOLDIER_PIC_X, SOLDIER_PIC_Y, TRUE );
}


static void RenderHandPosItem(void)
{
	// renders the inventory item in char's right hand

	// ARM: if already in the inventory panel, don't show the item again here, seeing it twice is confusing
	if ( fShowInventoryFlag )
	{
		return;
	}


	// grab the soldier
	const SOLDIERTYPE* pSoldier = NULL;
	if( bSelectedInfoChar != -1 )
	{
		if( gCharactersList[ bSelectedInfoChar ].fValid )
		{
			pSoldier = GetSoldier(gCharactersList[bSelectedInfoChar].usSolID);
		}
	}

	if( pSoldier == NULL )
	{
		return;
	}

	// check if still alive?
	if( pSoldier -> bLife == 0 )
	{
		return;
	}

	SetFont(BLOCKFONT2);
	SetFontForeground(CHAR_INFO_PANEL_BLOCK_COLOR);
	SetFontBackground(FONT_BLACK);

	INVRenderItem( guiSAVEBUFFER, pSoldier, &(pSoldier->inv[HANDPOS]), SOLDIER_HAND_X, SOLDIER_HAND_Y,	58, 23, DIRTYLEVEL2, NULL, 0, FALSE, 0);
}


static void RenderIconsForUpperLeftCornerPiece(INT8 bCharNumber)
{
	HVOBJECT hHandle = GetVideoObject(guiULICONS);

	// if merc is an AIM merc
	if( Menptr[ gCharactersList[ bCharNumber ].usSolID ].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC )
	{
		// finite contract length icon
		BltVideoObject( guiSAVEBUFFER, hHandle, 0, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y);
	}

	// if merc has life insurance
	if( Menptr[ gCharactersList[ bCharNumber ].usSolID ].usLifeInsurance > 0 )
	{
		// draw life insurance icon
		BltVideoObject( guiSAVEBUFFER, hHandle, 2, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + CHAR_ICON_SPACING);
	}

	// if merc has a medical deposit
	if( Menptr[ gCharactersList[ bCharNumber ].usSolID ].usMedicalDeposit > 0 )
	{
		// draw medical deposit icon
		BltVideoObject( guiSAVEBUFFER, hHandle, 1, CHAR_ICON_X, CHAR_ICON_CONTRACT_Y + ( 2 * CHAR_ICON_SPACING ));
	}
}


static void DrawString(const wchar_t *pString, UINT16 uiX, UINT16 uiY, UINT32 uiFont);


static void DrawPay(INT16 sCharNumber)
{
	// will draw the pay
	INT32 uiSalary;
	wchar_t sString[7];
	UINT16 usX, usY;
	INT16 usMercProfileID;


	// get merc id
	usMercProfileID = MercPtrs[ gCharactersList[ sCharNumber ].usSolID ] -> ubProfile;

	// grab salary
	uiSalary=( ( UINT32 ) gMercProfiles[ usMercProfileID ].sSalary );


	// font stuff
	SetFontForeground( CHAR_TITLE_FONT_COLOR );
	SetFontBackground( FONT_BLACK );

	// parse salary
	swprintf( sString, lengthof(sString), L"%d", uiSalary );

	// right justify salary
	FindFontRightCoordinates(CHAR_PAY_X,CHAR_PAY_Y , CHAR_PAY_WID,CHAR_PAY_HEI ,sString , CHAR_FONT, &usX, &usY);

	// draw salary
	DrawString(sString,usX, usY, CHAR_FONT);
}


static void DrawCharBars(void)
{
	UINT16 usSoldierID;

	// will draw the heath, morale and breath bars for a character being displayed in the upper left hand corner

	if( ( bSelectedInfoChar == -1 )&&( bSelectedDestChar == -1 ) )
	{
		// error, no character to display right now
		return;
	}
	else
	{
		// valid character
		if( bSelectedInfoChar != -1 )
		{
			usSoldierID=gCharactersList[ bSelectedInfoChar ].usSolID;
		}
		else
		{
			usSoldierID=gCharactersList[ bSelectedDestChar ].usSolID;
		}

		// grab soldier's id number
		const SOLDIERTYPE* pSoldier = GetSoldier(usSoldierID);
		if( pSoldier == NULL )
		{
			// no soldier
			return;
		}

		// skip POWs, dead guys
		if( ( pSoldier->bLife == 0 ) ||
				( pSoldier->bAssignment == ASSIGNMENT_DEAD ) ||
				( pSoldier->bAssignment == ASSIGNMENT_POW ) )
		{
			return;
		}

		DrawSoldierUIBars(pSoldier, BAR_INFO_X, BAR_INFO_Y, TRUE, FRAME_BUFFER);
	}
}


static void DrawCharStats(INT16 sCharNum)
{
	// will draw the characters stats, max life, strength, dex, and skills
	wchar_t sString[9];
	UINT16 usX, usY;
	//HVOBJECT hCrossHandle;
	SOLDIERTYPE *pSoldier = NULL;

	pSoldier = &Menptr[gCharactersList[sCharNum].usSolID];

	// set up font
	SetFont(CHAR_FONT);
	SetFontForeground(CHAR_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);


	// strength
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bStrength);

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeStrengthTime)&& ( pSoldier->uiChangeStrengthTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & STRENGTH_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(STR_X, STR_Y ,STAT_WID ,STAT_HEI ,sString, CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, STR_Y,CHAR_FONT );

	// dexterity
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bDexterity );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeDexterityTime ) && ( pSoldier->uiChangeDexterityTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & DEX_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(DEX_X,DEX_Y ,STAT_WID ,STAT_HEI ,sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, DEX_Y,CHAR_FONT );

	// agility
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bAgility );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeAgilityTime)&& ( pSoldier->uiChangeAgilityTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & AGIL_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(AGL_X,AGL_Y,STAT_WID ,STAT_HEI ,sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, AGL_Y,CHAR_FONT );

	// wisdom
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bWisdom );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeWisdomTime )&&( pSoldier->uiChangeWisdomTime != 0 ))
	{
		if( pSoldier->usValueGoneUp & WIS_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates( WIS_X,WIS_Y,STAT_WID ,STAT_HEI,sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, WIS_Y,CHAR_FONT );

	// leadership
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bLeadership );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeLeadershipTime ) && ( pSoldier->uiChangeLeadershipTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & LDR_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(LDR_X,LDR_Y,STAT_WID,STAT_HEI, sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, LDR_Y,CHAR_FONT );

	// experience level
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bExpLevel );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeLevelTime)&&( pSoldier->uiChangeLevelTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & LVL_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(LVL_X,LVL_Y,STAT_WID ,STAT_HEI,sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, LVL_Y,CHAR_FONT );

	// marksmanship
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bMarksmanship );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeMarksmanshipTime) && ( pSoldier->uiChangeMarksmanshipTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & MRK_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(MRK_X,MRK_Y,STAT_WID ,STAT_HEI,sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, MRK_Y,CHAR_FONT );

	// explosives
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bExplosive );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeExplosivesTime)&& ( pSoldier->uiChangeExplosivesTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & EXP_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(EXP_X,EXP_Y,STAT_WID ,STAT_HEI,sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, EXP_Y,CHAR_FONT );

	// mechanical
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bMechanical );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeMechanicalTime )&& ( pSoldier->uiChangeMechanicalTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & MECH_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(MEC_X,MEC_Y,STAT_WID ,STAT_HEI,sString , CHAR_FONT, &usX, &usY);
	DrawString(sString,usX, MEC_Y,CHAR_FONT );

	// medical
	swprintf( sString, lengthof(sString), L"%d", pSoldier->bMedical );

	if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeMedicalTime)&& ( pSoldier->uiChangeMedicalTime != 0 ) )
	{
		if( pSoldier->usValueGoneUp & MED_INCREASE )
		{
			SetFontForeground( FONT_LTGREEN );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
	}
	else
	{
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
	}

	// right justify
	FindFontRightCoordinates(MED_X,MED_Y,STAT_WID ,STAT_HEI,sString , CHAR_FONT, &usX, &usY);
 	DrawString(sString,usX, MED_Y,CHAR_FONT );

	SetFontForeground(CHAR_TEXT_FONT_COLOR);
}


static void DrawCharHealth(INT16 sCharNum)
{
	UINT32 uiHealthPercent = 0;
	wchar_t sString[9];
	UINT16 usX, usY;
	SOLDIERTYPE *pSoldier = NULL;


	pSoldier = &Menptr[gCharactersList[sCharNum].usSolID];

	if( pSoldier->bAssignment != ASSIGNMENT_POW )
	{
		// find starting X coordinate by centering all 3 substrings together, then print them separately (different colors)!
		swprintf( sString, lengthof(sString), L"%d/%d", pSoldier->bLife, pSoldier->bLifeMax );
		FindFontCenterCoordinates(CHAR_HP_X, CHAR_HP_Y, CHAR_HP_WID, CHAR_HP_HEI, sString, CHAR_FONT, &usX, &usY);


		if ( pSoldier->bLifeMax > 0 )
		{
			uiHealthPercent = ( pSoldier->bLife * 100 ) / pSoldier->bLifeMax;
		}

		// how is characters life?
		if (uiHealthPercent == 0)
		{
			// he's dead, Jim
			SetFontForeground(FONT_METALGRAY);
		}
		else
		if (uiHealthPercent < 25)
		{
			// very bad
			SetFontForeground(FONT_RED);
		}
		else
		if (uiHealthPercent < 50)
		{
			// not good
			SetFontForeground(FONT_YELLOW);
		}
		else
		{
			// ok
			SetFontForeground(CHAR_TEXT_FONT_COLOR);
		}

		// current life
		swprintf( sString, lengthof(sString), L"%d", pSoldier->bLife );
		DrawString( sString, usX, CHAR_HP_Y, CHAR_FONT );
		usX += StringPixLength( sString, CHAR_FONT );


		// slash
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
		wcscpy( sString, L"/" );
		DrawString( sString, usX, CHAR_HP_Y, CHAR_FONT );
		usX += StringPixLength( sString, CHAR_FONT );


		if( ( GetJA2Clock() < CHANGE_STAT_RECENTLY_DURATION + pSoldier->uiChangeHealthTime)&& ( pSoldier->uiChangeHealthTime != 0 ) )
		{
			if( pSoldier->usValueGoneUp & HEALTH_INCREASE )
			{
				SetFontForeground( FONT_LTGREEN );
			}
			else
			{
				SetFontForeground( FONT_RED );
			}
		}
		else
		{
			SetFontForeground(CHAR_TEXT_FONT_COLOR);
		}

		// maximum life
		swprintf( sString, lengthof(sString), L"%d", pSoldier->bLifeMax );
		DrawString( sString, usX, CHAR_HP_Y, CHAR_FONT );
	}
	else
	{
		// POW - health unknown
		SetFontForeground(CHAR_TEXT_FONT_COLOR);
		swprintf( sString, lengthof(sString), pPOWStrings[ 1 ] );
		FindFontCenterCoordinates(CHAR_HP_X, CHAR_HP_Y, CHAR_HP_WID, CHAR_HP_HEI, sString, CHAR_FONT, &usX, &usY);
		DrawString(sString, usX, CHAR_HP_Y, CHAR_FONT);
	}


	SetFontForeground(CHAR_TEXT_FONT_COLOR);
}


static void ConvertMinTimeToETADayHourMinString(UINT32 uiTimeInMin, STR16 sString, size_t Length);


// "character" refers to hired people AND vehicles
static void DrawCharacterInfo(INT16 sCharNumber)
{
	wchar_t sString[80];
	UINT16 usX, usY;
	INT16 usMercProfileID;
	INT32 iTimeRemaining=0;
	INT32 iDailyCost = 0;
	SOLDIERTYPE *pSoldier = NULL;
	UINT32 uiArrivalTime;


	if( gCharactersList[sCharNumber].fValid == FALSE )
	{
		return;
	}

	pSoldier = MercPtrs[ gCharactersList[sCharNumber].usSolID ];

	if( pSoldier->ubProfile == NO_PROFILE )
	{
		return;
	}


	// draw particular info about a character that are neither attributes nor skills


	// get profile information
	usMercProfileID = pSoldier->ubProfile;


	// set font stuff
	SetFont(CHAR_FONT);
	SetFontForeground(CHAR_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);


	// Nickname (beneath Picture)
	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// vehicle
		wcscpy(sString, pShortVehicleStrings[ pVehicleList[ pSoldier->bVehicleID ].ubVehicleType ]);
	}
	else
	{
		// soldier
		wcscpy(sString, gMercProfiles[usMercProfileID].zNickname);
	}

	FindFontCenterCoordinates(PIC_NAME_X, PIC_NAME_Y, PIC_NAME_WID, PIC_NAME_HEI, sString, CHAR_FONT, &usX, &usY);
	DrawString(sString, usX, usY, CHAR_FONT);


	// Full name (Top Box)
	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		// vehicle
		wcscpy(sString, pVehicleStrings[ pVehicleList[ pSoldier->bVehicleID ].ubVehicleType ]);
	}
	else
	{
		// soldier
		wcscpy(sString, gMercProfiles[usMercProfileID].zName);
	}

	FindFontCenterCoordinates(CHAR_NAME_X, CHAR_NAME_Y, CHAR_NAME_WID, CHAR_NAME_HEI, sString, CHAR_FONT, &usX, &usY);
	DrawString(sString, usX, usY, CHAR_FONT);


	// Assignment
	if( pSoldier->bAssignment == VEHICLE )
	{
		// show vehicle type
		wcscpy( sString, pShortVehicleStrings[ pVehicleList[ pSoldier->iVehicleId ].ubVehicleType ] );
	}
	else
	{
		wcscpy( sString, pAssignmentStrings[ pSoldier->bAssignment ] );
	}

	FindFontCenterCoordinates( CHAR_ASSIGN_X, CHAR_ASSIGN1_Y, CHAR_ASSIGN_WID, CHAR_ASSIGN_HEI, sString, CHAR_FONT, &usX, &usY );
	DrawString( sString, usX, usY, CHAR_FONT );


	// second assignment line

	// train self / teammate / by other ?
	if( ( pSoldier->bAssignment == TRAIN_SELF ) || ( pSoldier->bAssignment == TRAIN_TEAMMATE ) || ( pSoldier->bAssignment == TRAIN_BY_OTHER ) )
	{
		wcscpy(sString, pAttributeMenuStrings[pSoldier->bTrainStat]);
	}
	// train town?
	else if( pSoldier->bAssignment == TRAIN_TOWN )
	{
		wcscpy(sString,pTownNames[GetTownIdForSector( pSoldier->sSectorX, pSoldier->sSectorY ) ] );
	}
	// repairing?
	else if( pSoldier->bAssignment == REPAIR )
	{
		if ( pSoldier->fFixingRobot )
		{
			// robot
			wcscpy( sString, pRepairStrings[ 3 ] );
		}
/*
		else if ( pSoldier->fFixingSAMSite )
		{
			// SAM site
			wcscpy( sString, pRepairStrings[ 1 ] );
		}
*/
		else if ( pSoldier->bVehicleUnderRepairID != -1 )
		{
			// vehicle
			wcscpy( sString, pShortVehicleStrings[ pVehicleList[ pSoldier->bVehicleUnderRepairID ].ubVehicleType ] );
		}
		else
		{
			// items
			wcscpy( sString, pRepairStrings[ 0 ] );
		}
	}
	// in transit?
	else if( pSoldier->bAssignment == IN_TRANSIT )
	{
		// show ETA
		ConvertMinTimeToETADayHourMinString( pSoldier->uiTimeSoldierWillArrive, sString, lengthof(sString));
	}
	// traveling ?
	else if ( PlayerIDGroupInMotion( GetSoldierGroupId( pSoldier ) ) )
	{
		// show ETA
		uiArrivalTime = GetWorldTotalMin( ) + CalculateTravelTimeOfGroupId( GetSoldierGroupId( pSoldier ) );
		ConvertMinTimeToETADayHourMinString( uiArrivalTime, sString, lengthof(sString));
	}
	else
	{
		// show location
		GetMapscreenMercLocationString( pSoldier, sString, lengthof(sString));
	}

	if ( wcslen( sString ) > 0 )
	{
		FindFontCenterCoordinates(CHAR_ASSIGN_X, CHAR_ASSIGN2_Y ,CHAR_ASSIGN_WID ,CHAR_ASSIGN_HEI ,sString , CHAR_FONT, &usX, &usY);
		DrawString(sString, usX, usY, CHAR_FONT );
	}


	// draw health/condition
	DrawCharHealth( sCharNumber );


	// if a vehicle or robot
	if( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) || AM_A_ROBOT( pSoldier ) )
	{
		// we're done - the remainder applies only to people
		return;
	}


	// draw attributes & skills for currently displayed character
	DrawCharStats(sCharNumber);

	// remaining contract length

	// dead?
	if( pSoldier->bLife <= 0 )
	{
		wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
	}
	// what kind of merc
	else if(pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC || pSoldier->ubProfile == SLAY )
	{
		FLOAT dTimeLeft = 0.0;

		// amount of time left on contract
		iTimeRemaining = pSoldier->iEndofContractTime-GetWorldTotalMin();

		//if the merc is in transit
		if( pSoldier->bAssignment == IN_TRANSIT )
		{
			//and if the ttime left on the cotract is greater then the contract time
			if( iTimeRemaining > (INT32)( pSoldier->iTotalContractLength * NUM_MIN_IN_DAY ) )
			{
				iTimeRemaining = ( pSoldier->iTotalContractLength * NUM_MIN_IN_DAY );
			}
		}


		if(iTimeRemaining >= ( 24 * 60 ) )
		{
			//calculate the exact time left on the contract ( ex 1.8 days )
			dTimeLeft = (FLOAT)(iTimeRemaining / (60*24.0) );

			// more than a day, display in green
			iTimeRemaining /= (60*24);
			if( pSoldier->bLife > 0 )
			{
				SetFontForeground(FONT_LTGREEN);
			}

			swprintf(sString, lengthof(sString), L"%.1f%ls/%d%ls", dTimeLeft, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ], pSoldier->iTotalContractLength, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ]);
		}
		else
		{
			// less than a day, display hours left in red
			if( iTimeRemaining > 5 )
			{
				BOOLEAN fNeedToIncrement = FALSE;

				if( iTimeRemaining % 60 != 0 )
					fNeedToIncrement = TRUE;

				iTimeRemaining /= 60;

				if( fNeedToIncrement )
					iTimeRemaining++;
			}
			else
			{
				iTimeRemaining /= 60;
			}

			if( pSoldier->bLife > 0 )
			{
				SetFontForeground(FONT_RED);
			}

		 swprintf(sString, lengthof(sString), L"%d%ls/%d%ls",iTimeRemaining, gpStrategicString[ STR_PB_HOURS_ABBREVIATION ], pSoldier->iTotalContractLength, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ]);
		}
	}
	else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC )
	{
		INT32 iBeenHiredFor = ( GetWorldTotalMin( ) / NUM_MIN_IN_DAY ) - pSoldier->iStartContractTime;

		swprintf(sString, lengthof(sString), L"%d%ls/%d%ls",gMercProfiles[ pSoldier->ubProfile ].iMercMercContractLength, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ], iBeenHiredFor, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ] );
	}
	else
	{
		wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
	}


	// set font stuff
	SetFontForeground(CHAR_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);

	// center and draw
	FindFontCenterCoordinates(CHAR_TIME_REMAINING_X, CHAR_TIME_REMAINING_Y, CHAR_TIME_REMAINING_WID, CHAR_TIME_REMAINING_HEI, sString, CHAR_FONT, &usX, &usY);
	DrawString(sString,usX,usY, CHAR_FONT);


	// salary
	if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC )
	{
		// daily rate
		if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK )
		{
			iDailyCost = ( gMercProfiles[ pSoldier->ubProfile ].uiBiWeeklySalary / 14 );
		}
		if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK )
		{
			iDailyCost = ( gMercProfiles[ pSoldier->ubProfile ].uiWeeklySalary / 7 );
		}
		else
		{
			iDailyCost = gMercProfiles[ pSoldier->ubProfile ].sSalary;
		}
	}
	else
	{
		iDailyCost = gMercProfiles[ pSoldier->ubProfile ].sSalary;
	}

	SPrintMoney(sString, iDailyCost);
	FindFontRightCoordinates(CHAR_SALARY_X, CHAR_SALARY_Y, CHAR_SALARY_WID, CHAR_SALARY_HEI, sString, CHAR_FONT, &usX, &usY);
	DrawString(sString,usX,usY, CHAR_FONT);


	// medical deposit
	if( gMercProfiles[ Menptr[ gCharactersList[ sCharNumber ].usSolID ].ubProfile ].sMedicalDepositAmount > 0 )
	{
		SPrintMoney(sString, gMercProfiles[Menptr[gCharactersList[sCharNumber].usSolID].ubProfile].sMedicalDepositAmount);
		FindFontRightCoordinates(CHAR_MEDICAL_X, CHAR_MEDICAL_Y, CHAR_MEDICAL_WID, CHAR_MEDICAL_HEI, sString, CHAR_FONT, &usX, &usY);
		DrawString(sString,usX,CHAR_MEDICAL_Y, CHAR_FONT);
	}

/*
	// life insurance
	SPrintMoney(sString, Menptr[gCharactersList[sCharNumber].usSolID].usLifeInsuranceAmount);
	FindFontRightCoordinates(CHAR_LIFE_INSUR_X, CHAR_LIFE_INSUR_Y, CHAR_LIFE_INSUR_WID, CHAR_LIFE_INSUR_HEI, sString, CHAR_FONT, &usX, &usY);
	DrawString(sString,usX,usY, CHAR_FONT);
*/

	// morale
	if( pSoldier->bAssignment != ASSIGNMENT_POW )
	{
		if ( pSoldier->bLife != 0 )
		{
			GetMoraleString( MercPtrs[gCharactersList[sCharNumber].usSolID], sString );
		}
		else
		{
			wcscpy( sString, L"" );
		}
	}
	else
	{
		// POW - morale unknown
		swprintf( sString, lengthof(sString), pPOWStrings[ 1 ] );
	}

	FindFontCenterCoordinates(CHAR_MORALE_X, CHAR_MORALE_Y, CHAR_MORALE_WID, CHAR_MORALE_HEI, sString, CHAR_FONT, &usX, &usY);
	DrawString(sString,usX,CHAR_MORALE_Y, CHAR_FONT);
}


// this character is in transit has an item picked up
static BOOLEAN CharacterIsInTransitAndHasItemPickedUp(INT8 bCharacterNumber)
{

	// valid character?
	if( bCharacterNumber == -1 )
	{
		// nope
		return( FALSE );
	}

	// second validity check
	if( gCharactersList[ bCharacterNumber ].fValid == FALSE )
	{
		// nope
		return( FALSE );
	}

	// character in transit?
	if( Menptr[ gCharactersList[ bCharacterNumber ].usSolID ].bAssignment != IN_TRANSIT )
	{
		// nope
		return( FALSE );
	}

	// item picked up?
	if( gMPanelRegion.Cursor != EXTERN_CURSOR)
	{
		return( FALSE );
	}

	return( TRUE );
}


static void DisplayCharacterInfo(void)
{
	Assert( bSelectedInfoChar < MAX_CHARACTER_COUNT );
	Assert( gCharactersList[ bSelectedInfoChar ].fValid );

	// set font buffer
	SetFontDestBuffer( guiSAVEBUFFER, 0, 0, 640, 480, FALSE );

	// draw character info and face
	DrawCharacterInfo( bSelectedInfoChar );

	RenderHandPosItem();

	SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );

	RenderIconsForUpperLeftCornerPiece( bSelectedInfoChar );

	// mark all pop ups as dirty
	MarkAllBoxesAsAltered( );
}



INT32 GetPathTravelTimeDuringPlotting( PathStPtr pPath )
{
	INT32 iTravelTime = 0;
	WAYPOINT pCurrent;
	WAYPOINT pNext;
	GROUP *pGroup;
	UINT8 ubGroupId = 0;
	BOOLEAN fSkipFirstNode = FALSE;


	if( ( bSelectedDestChar == -1 ) && ( fPlotForHelicopter == FALSE ) )
	{
		return( 0 );
	}

	if( fTempPathAlreadyDrawn == FALSE )
	{
		return( 0 );
	}

	if( pPath == NULL )
	{
		return( 0 );
	}

	pPath = MoveToBeginningOfPathList( pPath );


	if( fPlotForHelicopter == FALSE )
	{
		// plotting for a character...
		if( Menptr[gCharactersList[bSelectedDestChar].usSolID].bAssignment == VEHICLE )
		{
			ubGroupId = pVehicleList[ Menptr[gCharactersList[bSelectedDestChar].usSolID].iVehicleId ].ubMovementGroup;
			pGroup = GetGroup( ubGroupId );

			if( pGroup == NULL )
			{
				SetUpMvtGroupForVehicle( &( Menptr[gCharactersList[bSelectedDestChar].usSolID] ) );

				// get vehicle id
				ubGroupId = pVehicleList[ Menptr[gCharactersList[bSelectedDestChar].usSolID].iVehicleId ].ubMovementGroup;
				pGroup = GetGroup( ubGroupId );
			}
		}
		else if( Menptr[gCharactersList[bSelectedDestChar].usSolID].uiStatusFlags & SOLDIER_VEHICLE )
		{
			ubGroupId = pVehicleList[ Menptr[gCharactersList[bSelectedDestChar].usSolID].bVehicleID ].ubMovementGroup;
			pGroup = GetGroup( ubGroupId );

			if( pGroup == NULL )
			{
				SetUpMvtGroupForVehicle( &( Menptr[gCharactersList[bSelectedDestChar].usSolID] ) );

				// get vehicle id
				ubGroupId = pVehicleList[ Menptr[gCharactersList[bSelectedDestChar].usSolID].bVehicleID ].ubMovementGroup;
				pGroup = GetGroup( ubGroupId );
			}
		}
		else
		{
			ubGroupId = Menptr[gCharactersList[bSelectedDestChar].usSolID].ubGroupID;
			pGroup = GetGroup( ( UINT8 )( ubGroupId ) );
		}
	}
	else
	{
		ubGroupId = pVehicleList[ iHelicopterVehicleId ].ubMovementGroup;
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
			pCurrent.x = ( UINT8 )( pPath->uiSectorId % MAP_WORLD_X );
			pCurrent.y = ( UINT8 )( pPath->uiSectorId / MAP_WORLD_X );

			// grab the next location
			pNext.x = ( UINT8 )( pPath->pNext->uiSectorId % MAP_WORLD_X );
			pNext.y = ( UINT8 )( pPath->pNext->uiSectorId / MAP_WORLD_X );

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


static INT32 GetGroundTravelTimeOfCharacter(INT8 bCharNumber);


static void DisplayGroundEta(void)
{
	UINT32 iTotalTime = 0;


	if( fPlotForHelicopter == TRUE )
	{
		return;
	}

	if( bSelectedDestChar == -1 )
	{
		return;
	}

	if( !gCharactersList[bSelectedDestChar].fValid )
	{
		return;
	}


	iTotalTime = GetGroundTravelTimeOfCharacter( bSelectedDestChar );

	// now display it
	SetFont( ETA_FONT );
	SetFontForeground( FONT_LTGREEN );
	SetFontBackground( FONT_BLACK );
	mprintf( CLOCK_ETA_X, CLOCK_Y_START, pEtaString[ 0 ] );

	// if less than one day
	if (iTotalTime < 60 * 24)
	{
		// show hours and minutes
		UINT Minutes = iTotalTime % 60;
		UINT Hours   = iTotalTime / 60;
		mprintf(CLOCK_MIN_X_START  - 5, CLOCK_Y_START, L"%2u%ls", Minutes, gsTimeStrings[1]);
		mprintf(CLOCK_HOUR_X_START - 8, CLOCK_Y_START, L"%2u%ls", Hours,   gsTimeStrings[0]);
	}
	else
	{
		// show days and hours
		UINT Hours = iTotalTime / 60 % 24;
		UINT Days  = iTotalTime / (60 * 24);
		mprintf(CLOCK_MIN_X_START  - 5, CLOCK_Y_START, L"%2u%ls", Hours, gsTimeStrings[0]);
		mprintf(CLOCK_HOUR_X_START - 9, CLOCK_Y_START, L"%2u%ls", Days,  gsTimeStrings[3]);
	}
}


static void HighLightAssignLine(void)
{
	UINT32 uiDestPitchBYTES;
	UINT8  *pDestBuf;
	UINT16 usColor;
	static INT32 iColorNum = STARTING_COLOR_NUM;
	static BOOLEAN fDelta=FALSE;
	static INT32 uiOldHighlight = MAX_CHARACTER_COUNT + 1;
	INT16 usCount = 0;
	UINT16 usX;
	UINT16 usY;


	// is this a valid line?
	if( ( giAssignHighLine == -1 ) || fShowInventoryFlag )
	{
		uiOldHighlight = MAX_CHARACTER_COUNT + 1;
		return;
	}

	// if not ready to change glow phase yet, leave
	if ( !gfGlowTimerExpired )
		return;


	// check if we have moved lines, if so, reset
	if( uiOldHighlight != giAssignHighLine )
	{
		iColorNum = STARTING_COLOR_NUM;
		fDelta = FALSE;

		uiOldHighlight = giAssignHighLine;
	}


	if((iColorNum==0)||(iColorNum==10))
	{
	 fDelta=!fDelta;
	}
	if(!fDelta)
		iColorNum++;
	else
		iColorNum--;


	//usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
	usY = ( Y_OFFSET * giAssignHighLine - 1 ) + ( Y_START + ( giAssignHighLine * Y_SIZE ) );

	if( giAssignHighLine >= FIRST_VEHICLE )
	{
		usY += 6;
	}


	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);

	for( usCount = 0; usCount < MAX_CHARACTER_COUNT; usCount++ )
	{
		if( IsCharacterSelectedForAssignment( usCount ) == TRUE )
		{
			usX=ASSIGN_X;
			//usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
			usY=(Y_OFFSET*usCount-1)+(Y_START+(usCount*Y_SIZE));
			if( usCount >= FIRST_VEHICLE )
			{
				usY += 6;
			}

			usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );

			LineDraw(TRUE, usX, usY, usX, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);
			LineDraw(TRUE, usX+ASSIGN_WIDTH, usY, usX+ASSIGN_WIDTH, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);
			if( ( usCount == 0 ) || ( usCount != 0 ? !( IsCharacterSelectedForAssignment( ( UINT16 )( usCount - 1 ) ) ) : 0 ) || ( usCount == FIRST_VEHICLE ) )
			{
				LineDraw( TRUE, usX, usY, usX+ASSIGN_WIDTH, usY, usColor,pDestBuf);
			}

			if( ( ( usCount == MAX_CHARACTER_COUNT - 1 ) ) || ( usCount != ( MAX_CHARACTER_COUNT - 1 ) ? !( IsCharacterSelectedForAssignment( ( UINT16 )( usCount + 1 ) ) ) : 0) || ( usCount == FIRST_VEHICLE - 1 ) )
			{
				LineDraw(TRUE, usX, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usX+ASSIGN_WIDTH, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);
			}

			InvalidateRegion( usX, usY, usX+ASSIGN_WIDTH+1, usY+GetFontHeight(MAP_SCREEN_FONT)+3);
		}
	}

	UnLockVideoSurface( FRAME_BUFFER );
}


static void HighLightDestLine(void)
{
	UINT32 uiDestPitchBYTES;
	UINT8  *pDestBuf;
	UINT16 usColor;
	static INT32 iColorNum = STARTING_COLOR_NUM;
	static BOOLEAN fDelta=FALSE;
	static INT32 uiOldHighlight = MAX_CHARACTER_COUNT + 1;
	UINT16 usCount = 0;
	UINT16 usX;
	UINT16 usY;


	if( ( giDestHighLine == -1 ) || fShowInventoryFlag )
	{
		uiOldHighlight = MAX_CHARACTER_COUNT + 1;
		return;
	}

	// if not ready to change glow phase yet, leave
	if ( !gfGlowTimerExpired )
		return;


	// check if we have moved lines, if so, reset
	if( uiOldHighlight != giDestHighLine )
	{
		iColorNum = STARTING_COLOR_NUM;
		fDelta = FALSE;

		uiOldHighlight = giDestHighLine;
	}


	if((iColorNum==0)||(iColorNum==10))
	{
	 fDelta=!fDelta;
	}
	if(!fDelta)
		iColorNum++;
	else
		iColorNum--;


	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);

	for( usCount = 0; usCount < MAX_CHARACTER_COUNT; usCount++ )
	{
		if( CharacterIsGettingPathPlotted( usCount ) == TRUE )
		{
			usX=DEST_ETA_X-4;
			//usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
			usY=(Y_OFFSET*usCount-1)+(Y_START+(usCount*Y_SIZE));
			if( usCount >= FIRST_VEHICLE )
			{
				usY += 6;
			}

			usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );

			if( ( usCount == 0 ) || ( usCount != 0 ? !( CharacterIsGettingPathPlotted( ( UINT16 )( usCount - 1 ) ) ) : 0 ) || ( usCount == FIRST_VEHICLE ) )
			{
				LineDraw( TRUE, usX+4, usY, usX+DEST_ETA_WIDTH+4, usY, usColor,pDestBuf);
			}
			if( ( ( usCount == MAX_CHARACTER_COUNT - 1 ) ) || ( usCount != ( MAX_CHARACTER_COUNT - 1 ) ? !( CharacterIsGettingPathPlotted( ( UINT16 )( usCount + 1 ) ) ) : 0) || ( usCount == FIRST_VEHICLE - 1 ) )
			{
				LineDraw(TRUE, usX+4, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usX+DEST_ETA_WIDTH+4, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);
			}


			LineDraw(TRUE, usX+4, usY, usX+4, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);
			LineDraw(TRUE, usX+DEST_ETA_WIDTH+4, usY, usX+DEST_ETA_WIDTH+4, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);

			InvalidateRegion( usX, usY, usX + DEST_ETA_WIDTH + 5, usY + GetFontHeight(MAP_SCREEN_FONT)+3 );
		}
	}
	//InvalidateRegion( usX+4, usY, DEST_ETA_WIDTH-10, usY+GetFontHeight(MAP_SCREEN_FONT)+3);
	//InvalidateRegion( usX+10, usY, usX+ASSIGN_WIDTH, usY+GetFontHeight(MAP_SCREEN_FONT)+3);
	UnLockVideoSurface( FRAME_BUFFER );
}


static void HighLightSleepLine(void)
{
	UINT32 uiDestPitchBYTES;
	UINT8  *pDestBuf;
	UINT16 usColor;
	static INT32 iColorNum = STARTING_COLOR_NUM;
	static BOOLEAN fDelta=FALSE;
	static INT32 uiOldHighlight = MAX_CHARACTER_COUNT + 1;
	UINT16 usCount = 0;
	UINT16 usX, usX2;
	UINT16 usY;


	// is this a valid line?
	if( ( giSleepHighLine == -1 ) || fShowInventoryFlag )
	{
		uiOldHighlight = MAX_CHARACTER_COUNT + 1;
		return;
	}

	// if not ready to change glow phase yet, leave
	if ( !gfGlowTimerExpired )
		return;


	// check if we have moved lines, if so, reset
	if( uiOldHighlight != giSleepHighLine )
	{
		iColorNum = STARTING_COLOR_NUM;
		fDelta = FALSE;

		uiOldHighlight = giSleepHighLine;
	}


	if((iColorNum==0)||(iColorNum==10))
	{
	 fDelta=!fDelta;
	}
	if(!fDelta)
		iColorNum++;
	else
		iColorNum--;


	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);

	for( usCount = 0; usCount < MAX_CHARACTER_COUNT; usCount++ )
	{
		if( IsCharacterSelectedForSleep( usCount ) == TRUE )
		{
			usX=SLEEP_X-4;
			usX2 = SLEEP_X + SLEEP_WIDTH;

			//usY=Y_START+(giHighLine*GetFontHeight((MAP_SCREEN_FONT)));
			usY=(Y_OFFSET*usCount-1)+(Y_START+(usCount*Y_SIZE));
			if( usCount >= FIRST_VEHICLE )
			{
				usY += 6;
			}

			usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );

			if( ( usCount == 0 ) || ( usCount != 0 ? !( IsCharacterSelectedForSleep( ( UINT16 )( usCount - 1 ) ) ) : 0 ) || ( usCount == FIRST_VEHICLE ) )
			{
				LineDraw( TRUE, usX+4, usY, usX2, usY, usColor,pDestBuf);
			}
			if( ( ( usCount == MAX_CHARACTER_COUNT - 1 ) ) || ( usCount != ( MAX_CHARACTER_COUNT - 1 ) ? !( IsCharacterSelectedForSleep( ( UINT16 )( usCount + 1 ) ) ) : 0) || ( usCount == FIRST_VEHICLE - 1 ) )
			{
				LineDraw(TRUE, usX+4, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usX2, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);
			}

			LineDraw(TRUE, usX+ 4, usY, usX + 4, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);
			LineDraw(TRUE, usX2, usY, usX2, usY+GetFontHeight(MAP_SCREEN_FONT)+2, usColor, pDestBuf);

			InvalidateRegion( usX, usY, usX2 + 5, usY + GetFontHeight(MAP_SCREEN_FONT)+3 );
		}
	}
	UnLockVideoSurface( FRAME_BUFFER );
}


static void AddCharacter(SOLDIERTYPE* pCharacter)
{
	UINT16 usCount=0;
	UINT16 usVehicleCount = 0, usVehicleLoop = 0;

	// is character valid?
	if( pCharacter == NULL)
	{
		// not valid, leave
		return;
	}

	// valid character?
	if( pCharacter -> bActive == FALSE )
	{
		return;
	}

	// adding a vehicle?
	if( pCharacter->uiStatusFlags & SOLDIER_VEHICLE )
	{
		while( usVehicleLoop < MAX_CHARACTER_COUNT )
		{
			if ( gCharactersList[ usVehicleLoop ].fValid )
			{
				if ( Menptr[ usVehicleLoop ].uiStatusFlags & SOLDIER_VEHICLE )
				{
					usVehicleCount++;
				}
			}
			usVehicleLoop++;
		}

		usCount = FIRST_VEHICLE + usVehicleCount;
	}
	else
	{
		// go through character list until a blank is reached
		while( (gCharactersList[usCount].fValid)&&(usCount < MAX_CHARACTER_COUNT) )
		{
			usCount++;
		}
	}


	Assert( usCount < MAX_CHARACTER_COUNT );
	if (usCount >= MAX_CHARACTER_COUNT )
	{
		return;
	}

	// copy over soldier id value
	gCharactersList[usCount].usSolID = ( UINT16 )pCharacter -> ubID;

	// valid character
	gCharactersList[usCount].fValid = TRUE;
}


/*
void MoveCharacter(UINT16 uiInitialPosition, UINT16 uiFinalPosition)
{
	if (gCharactersList[uiInitialPosition].fValid)
		gCharactersList[uiFinalPosition] = gCharactersList[uiInitialPosition];
}


void SwapCharacters(UINT16 uiInitialPosition, UINT16 uiFinalPosition)
{
	MapScreenCharacterSt pTempChar;
	pTempChar = gCharactersList[uiInitialPosition];
	gCharactersList[uiInitialPosition] = gCharactersList[uiFinalPosition];
	gCharactersList[uiFinalPosition] = pTempChar;
}


void RemoveCharacter(UINT16 uiCharPosition)
{
 memset(&gCharactersList[uiCharPosition], 0, sizeof( MapScreenCharacterSt ));
}
*/


static void LoadCharacters(void)
{
	UINT16 uiCount=0;
	SOLDIERTYPE *pSoldier, *pTeamSoldier;
	INT32 cnt=0;

	pSoldier = MercPtrs[ 0 ];

	// fills array with pressence of player controlled characters
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
	{
		if(pTeamSoldier->bActive)
		{
			AddCharacter( pTeamSoldier );
			uiCount++;
		}
	}

	// set info char if no selected
	if( bSelectedInfoChar == -1 )
	{
		if( DialogueActive() == FALSE )
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


static void DrawAssignment(INT16 sCharNumber, INT16 sRowIndex, INT32 iFont);
static void DrawDestination(INT16 sCharNumber, INT16 sRowIndex, INT32 iFont);
static void DrawLocation(INT16 sCharNumber, INT16 sRowIndex, INT32 iFont);
static void DrawName(STR16 pName, INT16 sRowIndex, INT32 iFont);
static void DrawTimeRemaining(INT16 sCharNumber, INT32 iFont, UINT8 ubFontColor);
static void EnableDisableTeamListRegionsAndHelpText(void);


static void DisplayCharacterList(void)
{
	INT16 sCount=0;
	UINT8 ubForegroundColor = 0;

	if( ( fShowAssignmentMenu == TRUE ) && ( fTeamPanelDirty == FALSE ) )
	{
		SetFontDestBuffer(FRAME_BUFFER ,0,0,640,480,FALSE);
		return;
	}


	// set dest buffer
	SetFontDestBuffer(guiSAVEBUFFER,0,0,640,480,FALSE);
	SetFont( MAP_SCREEN_FONT );
	SetFontBackground(FONT_BLACK);


	for (sCount = 0; sCount < MAX_CHARACTER_COUNT; sCount++)
	{
		// skip invalid characters
		if ( gCharactersList[sCount].fValid == TRUE )
		{
			if( sCount == ( INT16 ) giHighLine )
			{
				ubForegroundColor = FONT_WHITE;
			}
			// check to see if character is still alive
			else if( Menptr[gCharactersList[sCount].usSolID].bLife == 0 )
			{
				ubForegroundColor = FONT_METALGRAY;
			}
			else if( CharacterIsGettingPathPlotted( sCount ) == TRUE )
			{
				ubForegroundColor = FONT_LTBLUE;
			}
			// in current sector?
			else if( ( Menptr[gCharactersList[sCount].usSolID].sSectorX == sSelMapX ) &&
							 ( Menptr[gCharactersList[sCount].usSolID].sSectorY == sSelMapY ) &&
							 ( Menptr[gCharactersList[sCount].usSolID].bSectorZ == iCurrentMapSectorZ ) )
			{
				// mobile ?
				if( ( Menptr[gCharactersList[sCount].usSolID].bAssignment < ON_DUTY ) ||
						( Menptr[gCharactersList[sCount].usSolID].bAssignment == VEHICLE ) )
					ubForegroundColor = FONT_YELLOW;
				else
					ubForegroundColor = FONT_MAP_DKYELLOW;
			}
			else
			{
				// not in current sector
				ubForegroundColor = 5;
			}

			SetFontForeground( ubForegroundColor );

			DrawName( Menptr[gCharactersList[sCount].usSolID].name, sCount, MAP_SCREEN_FONT);
			DrawLocation(sCount, sCount, MAP_SCREEN_FONT);
			DrawDestination(sCount, sCount, MAP_SCREEN_FONT);
			DrawAssignment(sCount, sCount, MAP_SCREEN_FONT);
			DrawTimeRemaining(sCount, MAP_SCREEN_FONT, ubForegroundColor );
		}
	}

	HandleDisplayOfSelectedMercArrows( );
	SetFontDestBuffer(FRAME_BUFFER ,0,0,640,480,FALSE);

	EnableDisableTeamListRegionsAndHelpText( );

	// mark all pop ups as dirty, so that any open assigment menus get reblitted over top of the team list
	MarkAllBoxesAsAltered( );
}



// THIS IS STUFF THAT RUNS *ONCE* DURING APPLICATION EXECUTION, AT INITIAL STARTUP
UINT32 MapScreenInit(void)
{
	SetUpBadSectorsList( );

	// setup message box system
	InitGlobalMessageList( );

	// init palettes for big map
	InitializePalettesForMap( );

	// set up mapscreen fast help text
	SetUpMapScreenFastHelpText( );

	// set up leave list arrays for dismissed mercs
	InitLeaveList( );

	guiUpdatePanelTactical = AddVideoObjectFromFile("INTERFACE/group_confirm_tactical.sti");
	CHECKF(guiUpdatePanelTactical != NO_VOBJECT);

	return TRUE;
}



UINT32 MapScreenShutdown(void)
{
	// free up alloced mapscreen messages
	FreeGlobalMessageList( );

	ShutDownPalettesForMap( );

	// free memory for leave list arrays for dismissed mercs
	ShutDownLeaveList( );

	DeleteVideoObjectFromIndex(guiUpdatePanelTactical);

	return TRUE;
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
static void InvmaskRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
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
static void UpdateTownMinePopUpDisplay(void);


UINT32 MapScreenHandle(void)
{
	UINT32 uiNewScreen;
//	static BOOLEAN fSecondFrame = FALSE;
	INT32 iCounter = 0;

	//DO NOT MOVE THIS FUNCTION CALL!!!
	//This determines if the help screen should be active
	if( ShouldTheHelpScreenComeUp( HelpScreenDetermineWhichMapScreenHelpToShow(), FALSE ) )
	{
		// handle the help screen
		HelpScreenHandler();
		return( MAP_SCREEN );
	}



	// shaded screen, leave
	if ( gfLoadPending == 2 )
	{
		gfLoadPending = FALSE;

		// Load Sector
		// BigCheese
		if ( !SetCurrentWorldSector( sSelMapX, sSelMapY , ( INT8 )iCurrentMapSectorZ ) )
		{
			// Cannot load!
		}
		else
		{
			CreateDestroyMapInvButton();
			// define our progress bar
			//CreateProgressBar( 0, 118, 183, 522, 202 );

		}
		return( MAP_SCREEN );
	}



//	if ( (fInMapMode == FALSE ) && ( fMapExitDueToMessageBox == FALSE ) )
	if ( !fInMapMode )
	{
		gfFirstMapscreenFrame = TRUE;

		InitPreviousPaths();

		// if arrival sector is invalid, reset to A9
		if ( ( gsMercArriveSectorX <  1 ) || ( gsMercArriveSectorY <  1 ) ||
				 ( gsMercArriveSectorX > 16 ) || ( gsMercArriveSectorY > 16 ) )
		{
			gsMercArriveSectorX = 9;
			gsMercArriveSectorY = 1;
		}

		gfInConfirmMapMoveMode = FALSE;
		gfInChangeArrivalSectorMode = FALSE;

		fLeavingMapScreen = FALSE;
		fResetTimerForFirstEntryIntoMapScreen = TRUE;
		fFlashAssignDone = FALSE;
		gfEnteringMapScreen = 0;

		guiTacticalInterfaceFlags |= INTERFACE_MAPSCREEN;

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
		//MSYS_DisableRegion( &gViewportRegion );
		//MSYS_DisableRegion( &gRadarRegion );
		//Disable all faces
		SetAllAutoFacesInactive( );


		if(fPreLoadedMapGraphics == FALSE )
		{

			// load border graphics
			LoadMapBorderGraphics( );

		 //fInterfacePanelDirty=DIRTYLEVEL2;
		 //RenderTacticalInterface();
		 // Grab the Map image

#ifndef JA2DEMO
		guiBIGMAP = AddVideoSurfaceFromFile("INTERFACE/b_map.pcx");
		CHECKF(guiBIGMAP != NO_VSURFACE);
#endif

		guiPOPUPTEX = AddVideoSurfaceFromFile("INTERFACE/popupbackground.pcx");
		CHECKF(guiPOPUPTEX != NO_VSURFACE);

#ifndef JA2DEMO
			guiSAMICON = AddVideoObjectFromFile("INTERFACE/SAM.sti");
			CHECKF(guiSAMICON != NO_VOBJECT);
#endif
			// CHECKF(AddVideoObjectFromFile("INTERFACE/s_map.sti", &guiMAP));
			guiMAPCURSORS = AddVideoObjectFromFile("INTERFACE/mapcursr.sti");
			CHECKF(guiMAPCURSORS != NO_VOBJECT);

#ifndef JA2DEMO
			guiSubLevel1 = AddVideoObjectFromFile("INTERFACE/Mine_1.sti");
			CHECKF(guiSubLevel1 != NO_VOBJECT);
			guiSubLevel2 = AddVideoObjectFromFile("INTERFACE/Mine_2.sti");
			CHECKF(guiSubLevel2 != NO_VOBJECT);
			guiSubLevel3 = AddVideoObjectFromFile("INTERFACE/Mine_3.sti");
			CHECKF(guiSubLevel3 != NO_VOBJECT);
#endif
			//CHECKF(AddVideoObjectFromFile("INTERFACE/addonslcp.sti", &guiCORNERADDONS));
			//CHECKF(AddVideoObjectFromFile("INTERFACE/mapborder.sti", &guiMAPBORDER));

			guiSleepIcon = AddVideoObjectFromFile("INTERFACE/sleepicon.sti");
			CHECKF(guiSleepIcon != NO_VOBJECT);
			guiCHARINFO = AddVideoObjectFromFile("INTERFACE/charinfo.sti");
			CHECKF(guiCHARINFO != NO_VOBJECT);
			guiCHARLIST = AddVideoObjectFromFile("INTERFACE/newgoldpiece3.sti");
			CHECKF(guiCHARLIST != NO_VOBJECT);

			//CHECKF(AddVideoObjectFromFile("INTERFACE/mapbordercorner.sti", &guiMAPCORNER));
			//CHECKF(AddVideoObjectFromFile("INTERFACE/popup.sti", &guiPOPUPBORDERS));

			guiCHARICONS = AddVideoObjectFromFile("INTERFACE/boxes.sti");
			CHECKF(guiCHARICONS != NO_VOBJECT);
			guiCROSS = AddVideoObjectFromFile("INTERFACE/incross.sti");
			CHECKF(guiCROSS != NO_VOBJECT);
			guiMAPINV = AddVideoObjectFromFile("INTERFACE/mapinv.sti");
			CHECKF(guiMAPINV != NO_VOBJECT);
			guiMapInvSecondHandBlockout = AddVideoObjectFromFile("INTERFACE/map_inv_2nd_gun_cover.sti");
			CHECKF(guiMapInvSecondHandBlockout != NO_VOBJECT);

		 // the upper left corner piece icons
			guiULICONS = AddVideoObjectFromFile("INTERFACE/top_left_corner_icons.sti");
			CHECKF(guiULICONS != NO_VOBJECT);
			guiORTAICON = AddVideoObjectFromFile("INTERFACE/map_item.sti");
			CHECKF(guiORTAICON != NO_VOBJECT);
			guiTIXAICON = AddVideoObjectFromFile("INTERFACE/prison.sti");
			CHECKF(guiTIXAICON != NO_VOBJECT);
			guiCHARBETWEENSECTORICONS = AddVideoObjectFromFile("INTERFACE/merc_between_sector_icons.sti");
			CHECKF(guiCHARBETWEENSECTORICONS != NO_VOBJECT);
			guiCHARBETWEENSECTORICONSCLOSE = AddVideoObjectFromFile("INTERFACE/merc_mvt_green_arrows.sti");
			CHECKF(guiCHARBETWEENSECTORICONSCLOSE != NO_VOBJECT);

			guiLEVELMARKER = AddVideoObjectFromFile("INTERFACE/GreenArr.sti");
			CHECKF(guiLEVELMARKER != NO_VOBJECT);
			guiHelicopterIcon = AddVideoObjectFromFile("INTERFACE/Helicop.sti");
			CHECKF(guiHelicopterIcon != NO_VOBJECT);
			guiMapBorderEtaPopUp = AddVideoObjectFromFile("INTERFACE/eta_pop_up.sti");
			CHECKF(guiMapBorderEtaPopUp != NO_VOBJECT);
			guiMapBorderHeliSectors = AddVideoObjectFromFile("INTERFACE/pos2.sti");
			CHECKF(guiMapBorderHeliSectors != NO_VOBJECT);

			guiSecItemHiddenVO = AddVideoObjectFromFile("INTERFACE/secondary_gun_hidden.sti");
			CHECKF(guiSecItemHiddenVO != NO_VOBJECT);
			guiSelectedCharArrow = AddVideoObjectFromFile("INTERFACE/selectedchararrow.sti");
			CHECKF(guiSelectedCharArrow != NO_VOBJECT);

			guiMINEICON = AddVideoObjectFromFile("INTERFACE/mine.sti");
			CHECKF(guiMINEICON != NO_VOBJECT);
			guiSectorLocatorGraphicID = AddVideoObjectFromFile("INTERFACE/hilite.sti");
			guiBULLSEYE = AddVideoObjectFromFile("INTERFACE/BullsEye.sti");
			CHECKF(guiBULLSEYE != NO_VOBJECT);

			HandleLoadOfMapBottomGraphics( );

			// load the militia pop up box
			LoadMilitiaPopUpBox( );

				// graphic for pool inventory
				LoadInventoryPoolGraphic( );

			//Kris:  Added this because I need to blink the icons button.
			guiNewMailIcons = AddVideoObjectFromFile("INTERFACE/newemail.sti");
		}


		// create buttons
		CreateButtonsForMapBorder( );

		// create mouse regions for level markers
		CreateMouseRegionsForLevelMarkers( );


		// change selected sector/level if necessary
		// NOTE: Must come after border buttons are created, since it may toggle them!
		if( AnyMercsHired( ) == FALSE )
		{
			// select starting sector (A9 - Omerta)
			ChangeSelectedMapSector( 9, 1, 0 );
		}
		else if( ( gWorldSectorX > 0 ) && ( gWorldSectorY > 0 ) && ( gbWorldSectorZ != -1 ) )
		{
			// select currently loaded sector as the map sector
			ChangeSelectedMapSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		}
		else	// no loaded sector
		{
			// only select A9 - Omerta IF there is no current selection, otherwise leave it as is
			if ( ( sSelMapX == 0 ) || ( sSelMapY == 0 ) || ( iCurrentMapSectorZ == -1 ) )
			{
				ChangeSelectedMapSector( 9, 1, 0 );
			}
		}

		guiBrownBackgroundForTeamPanel = AddVideoObjectFromFile("INTERFACE/Bars.sti");
		CHECKF(guiBrownBackgroundForTeamPanel != NO_VOBJECT);

		// we are in fact in the map, do not repeat this sequence
		fInMapMode = TRUE;

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

		// reset fact we are showing white bounding box around face
		fShowFaceHightLight = FALSE;
		fShowItemHighLight = FALSE;

		// reset all selected character flags
		ResetAllSelectedCharacterModes( );

		if( fFirstTimeInMapScreen == TRUE )
		{
			fFirstTimeInMapScreen = FALSE;
//			fShowMapScreenHelpText = TRUE;
		}

		fShowMapInventoryPool = FALSE;

		// init character list - set all values in the list to 0
		InitalizeVehicleAndCharacterList( );

		// deselect all entries
		ResetSelectedListForMapScreen( );

		LoadCharacters();


		// set up regions
		MSYS_DefineRegion( &gMapViewRegion, MAP_VIEW_START_X + MAP_GRID_X, MAP_VIEW_START_Y + MAP_GRID_Y,MAP_VIEW_START_X + MAP_VIEW_WIDTH+MAP_GRID_X-1, MAP_VIEW_START_Y + MAP_VIEW_HEIGHT-1 + 8, MSYS_PRIORITY_HIGH - 3,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );
		MSYS_DefineRegion( &gCharInfoHandRegion, ((INT16)( 4 )), ((INT16)( 81 )) ,((INT16)(62)), ((INT16)(103)), MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, ItemRegionMvtCallback , ItemRegionBtnCallback );
		MSYS_DefineRegion( &gCharInfoFaceRegion, (INT16) PLAYER_INFO_FACE_START_X, (INT16) PLAYER_INFO_FACE_START_Y, (INT16) PLAYER_INFO_FACE_END_X, (INT16) PLAYER_INFO_FACE_END_Y, MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK, FaceRegionBtnCallback );

		MSYS_DefineRegion(&gMPanelRegion, INV_REGION_X, INV_REGION_Y, INV_REGION_X+INV_REGION_WIDTH, INV_REGION_Y+INV_REGION_HEIGHT, MSYS_PRIORITY_HIGH,
							 MSYS_NO_CURSOR, MSYS_NO_CALLBACK,InvmaskRegionBtnCallBack);
		// screen mask for animated cursors
		MSYS_DefineRegion( &gMapScreenMaskRegion, 0, 0, 640, 480, MSYS_PRIORITY_LOW,
							 CURSOR_NORMAL, MSYS_NO_CALLBACK, MapScreenMarkRegionBtnCallback);

		// set help text for item glow region
		SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );

		// init the timer menus
		InitTimersForMoveMenuMouseRegions( );

		giMapContractButtonImage = LoadButtonImage( "INTERFACE/contractbutton.sti" ,-1,0,-1,1,-1 );

		// buttonmake
		giMapContractButton= QuickCreateButton( giMapContractButtonImage, CONTRACT_X + 5, CONTRACT_Y - 1,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 5,
										BtnGenericMouseMoveButtonCallback, ContractButtonCallback);

		SpecifyButtonText( giMapContractButton, pContractButtonString[ 0 ] );
		SpecifyButtonFont( giMapContractButton, MAP_SCREEN_FONT );
		SpecifyButtonUpTextColors( giMapContractButton, CHAR_TEXT_FONT_COLOR, FONT_BLACK );
		SpecifyButtonDownTextColors( giMapContractButton, CHAR_TEXT_FONT_COLOR, FONT_BLACK );

		// create mouse region for pause clock
		CreateMouseRegionForPauseOfClock( CLOCK_REGION_START_X, CLOCK_REGION_START_Y );

		// create mouse regions
		CreateMouseRegionsForTeamList( );

		ReBuildCharactersList( );

		// create status bar region
		CreateMapStatusBarsRegion( );

    if ( !gfFadeOutDone && !gfFadeIn )
    {
		  MSYS_SetCurrentCursor(SCREEN_CURSOR);
    }
		MSYS_DisableRegion(&gMPanelRegion);

		// create contract box
		CreateContractBox( NULL );

		// create the permanent boxes for assignment and its submenus
		fShowAssignmentMenu = TRUE;
		CreateDestroyAssignmentPopUpBoxes( );
		fShowAssignmentMenu = FALSE;


		// create merc remove box
		CreateMercRemoveAssignBox( );

		// fill in
		ColorFillVideoSurfaceArea( guiSAVEBUFFER, 0, 0, 640, 480, Get16BPPColor( RGB_NEAR_BLACK ) );
		ColorFillVideoSurfaceArea(  FRAME_BUFFER, 0, 0, 640, 480, Get16BPPColor( RGB_NEAR_BLACK ) );


		if( ( fFirstTimeInMapScreen == TRUE ) && ( AnyMercsHired( ) == FALSE ) )
		{
			// render both panels for the restore
			RenderMapRegionBackground( );
			RenderTeamRegionBackground( );

			// now do the warning box
			DoMapMessageBox( MSG_BOX_BASIC_STYLE, pMapErrorString[ 4 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
		}


		fFirstTimeInMapScreen = FALSE;

		if( gpCurrentTalkingFace != NULL )
		{
			// GO FROM GAMESCREEN TO MAPSCREEN
			// REMOVE OLD UI
			// Set face inactive!
			//gpCurrentTalkingFace->fCanHandleInactiveNow = TRUE;
			//SetAutoFaceInActive( gpCurrentTalkingFace->iID );
			//gfFacePanelActive = FALSE;


			// make him continue talking
			ContinueDialogue( MercPtrs[ gpCurrentTalkingFace -> ubSoldierID ], FALSE );

			// reset diabled flag
			//gpCurrentTalkingFace->fDisabled = FALSE;

			// Continue his talking!


		}

		fOneFrame = FALSE;

		if( fEnterMapDueToContract == TRUE )
		{
			if( pContractReHireSoldier )
			{
				FindAndSetThisContractSoldier( pContractReHireSoldier );
			}
			fEnterMapDueToContract = FALSE;
		}

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
	if( ( bSelectedDestChar == -1 ) && ( fPlotForHelicopter == FALSE ) && ( gfInChangeArrivalSectorMode == FALSE ) )
	{
		// reset cursor
    if ( !gfFadeIn )
    {
		  ChangeMapScreenMaskCursor( CURSOR_NORMAL );
    }
	}
*/

	// check if we are going to create or destroy map border graphics?
	CreateDestroyMapInventoryPoolButtons( FALSE );

	// set up buttons for mapscreen scroll
//	HandleMapScrollButtonStates( );


	// don't process any input until we've been through here once
	if( gfFirstMapscreenFrame == FALSE )
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

	if( ( fShownAssignmentMenu == FALSE ) && ( fShowAssignmentMenu == TRUE ) )
	{
		// need a one frame pause
		fShownAssignmentMenu = fShowAssignmentMenu;
		fShowAssignmentMenu = FALSE;
		fOneFrame = TRUE;
	}
	else if( ( fShownContractMenu == FALSE ) && ( fShowContractMenu == TRUE ) )
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

	if( ( fShownAssignmentMenu == FALSE ) && ( fShowAssignmentMenu == FALSE ) )
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
	DequeAllGameEvents( TRUE );

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

	if( fLeavingMapScreen == TRUE )
	{
		return( MAP_SCREEN );
	}

	if( fDisableDueToBattleRoster == FALSE )
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


	if(	!fShowMapInventoryPool && !gfPauseDueToPlayerGamePause && !IsMapScreenHelpTextUp( )	/* && !fDisabledMapBorder */ )
	{
		RenderMapCursorsIndexesAnims( );
	}


	if( fDisableDueToBattleRoster == FALSE )
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

	if( ( fDisableDueToBattleRoster == FALSE ) )
	{
		// remove the move box once user leaves it
		CreateDestroyMovementBox( 0,0,0 );

		// this updates the move box contents when changes took place
		ReBuildMoveBox( );
	}

	if( ( fDisableDueToBattleRoster == FALSE )&&( ( fShowAssignmentMenu == TRUE )||( fShowContractMenu == TRUE ) ) )
	{
		// highlight lines?
		HandleHighLightingOfLinesInTeamPanel( );

		// render glow for contract region
		ContractBoxGlow( );
		GlowTrashCan( );

		// handle changing of highlighted lines
		HandleChangeOfHighLightedLine( );
	}

	if( fDisableDueToBattleRoster == FALSE )
	{
		// render face of current info char, for animation
		DrawFace( bSelectedInfoChar );

		// handle autofaces
		HandleAutoFaces( );
		HandleTalkingAutoFaces( );
/*
		GlowFace( );
		GlowItem( );
*/

	}


	// automatically turns off mapscreen ui overlay messages when appropriate
	MonitorMapUIMessage( );


	// if heli is around, show it
	if( fHelicopterAvailable && fShowAircraftFlag && ( iCurrentMapSectorZ == 0 ) && !fShowMapInventoryPool )
	{
		// this is done on EVERY frame, I guess it beats setting entire map dirty all the time while he's moving...
		DisplayPositionOfHelicopter( );
	}


	// display town info
	DisplayTownInfo( sSelMapX, sSelMapY, ( INT8 ) iCurrentMapSectorZ );

	if( fShowTownInfo == TRUE )
	{
		// force update of town mine info boxes
		ForceUpDateOfBox( ghTownMineBox );
		MapscreenMarkButtonsDirty();
	}

	// update town mine pop up display
	UpdateTownMinePopUpDisplay( );

	if( fShowAttributeMenu )
	{
		// mark all popups as dirty
		MarkAllBoxesAsAltered( );
	}


	// if plotting path
	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
	{
		// plot out paths
		PlotPermanentPaths( );
		PlotTemporaryPaths( );

		// show ETA
		RenderMapBorderEtaPopUp( );
		DisplayGroundEta();

		//DisplayDestinationOfCurrentDestMerc( );
	}


	HandleContractRenewalSequence( );

	// handle dialog
	HandleDialogue( );


	// now the border corner piece
//	RenderMapBorderCorner( );


	// handle display of inventory pop up
	HandleDisplayOfItemPopUpForSector( 9, 1, 0 );

	// Display Framerate
	DisplayFrameRate( );

	// update paused states
	UpdatePausedStatesDueToTimeCompression( );

	// is there a description to be displayed?
	RenderItemDescriptionBox( );

	// render clock
	RenderClock(CLOCK_X, CLOCK_Y + 1 );

	#ifdef JA2TESTVERSION
	if( !gfWorldLoaded )
	{
		SetFont( FONT10ARIAL );
		if( GetJA2Clock() % 1000 < 500 )
			SetFontForeground( FONT_DKRED );
		else
			SetFontForeground( FONT_RED );
		mprintf( 530, 2, L"TESTVERSION MSG" );
		if( GetJA2Clock() % 1000 < 500 )
			SetFontForeground( FONT_DKYELLOW );
		else
			SetFontForeground( FONT_YELLOW );
		mprintf( 530, 12, L"NO WORLD LOADED" );
		InvalidateRegion( 530, 2, 640, 23 );
	}
	#endif

	if( fEndShowInventoryFlag == TRUE )
	{
		if( InKeyRingPopup( ) == TRUE )
		{
			DeleteKeyRingPopup( );
		}
		else
		{
			fShowInventoryFlag = FALSE;
			// set help text for item glow region
			SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );
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



	if( fShowDescriptionFlag == TRUE )
	{
		// unmark done button
		if( gpItemDescObject->usItem == MONEY )
		{
			MapscreenMarkButtonsDirty();
		}

		if( Item[ gpItemDescObject->usItem ].usItemClass & IC_GUN )
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
		if( fShowInventoryFlag == TRUE )
		{
			MarkAButtonDirty( giMapInvDoneButton );
			MarkAButtonDirty( giCharInfoButton[ 1 ] );
			MarkAButtonDirty( giCharInfoButton[ 0 ] );
		}
	}

	DrawMilitiaPopUpBox( );

	if( fDisableDueToBattleRoster == FALSE )
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

	if( ( fDisableDueToBattleRoster == FALSE ) && ( fShowAssignmentMenu == FALSE )&&( fShowContractMenu == FALSE ) )
	{
		// highlight lines?
		HandleHighLightingOfLinesInTeamPanel( );

		// render glow for contract region
		ContractBoxGlow( );
		GlowTrashCan( );

		// handle changing of highlighted lines
		HandleChangeOfHighLightedLine( );

		GlowFace( );
		GlowItem( );
	}


#ifdef JA2DEMO
	DisplayExitToTacticalGlowDuringDemo( );
#endif

	if( fShowMapScreenHelpText )
	{
		// display map screen fast help
		DisplayMapScreenFastHelpList( );
	}
	else
	{
		// render help
		RenderButtonsFastHelp( );
	}

/*
	if( fSecondFrame )
	{
		if( gTacticalStatus.fDidGameJustStart )
		{
			// game just started, check what state player is in......
			HandlePlayerEnteringMapScreenBeforeGoingToTactical( );
		}
	}
	fSecondFrame = FALSE;
*/

	// execute dirty
	ExecuteBaseDirtyRectQueue( );

	// update cursor
	UpdateCursorIfInLastSector( );


	// about to leave for new map
	if ( gfLoadPending == 1 )
	{
		gfLoadPending++;

		// Shade this frame!
		// Remove cursor
		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

		//Shadow area
		ShadowVideoSurfaceRect( FRAME_BUFFER, 0, 0, 640, 480 );
		InvalidateScreen( );
	}

	//InvalidateRegion( 0,0, 640, 480);
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

#ifdef JA2BETAVERSION
	DebugValidateSoldierData( );
#endif

	if ( gfRequestGiveSkyriderNewDestination )
	{
		RequestGiveSkyriderNewDestination();
		gfRequestGiveSkyriderNewDestination = FALSE;
	}


	if( gfFirstMapscreenFrame )
	{
//		fSecondFrame = TRUE;
		gfFirstMapscreenFrame = FALSE;
	}
	else
	{
		// handle exiting from mapscreen due to both exit button clicks and keyboard equivalents
		HandleExitsFromMapScreen( );
	}


	return( MAP_SCREEN );
}


static void DrawString(const wchar_t *pString, UINT16 uiX, UINT16 uiY, UINT32 uiFont)
{
	// draw monochrome string
	SetFont(uiFont);
	gprintfdirty(uiX,uiY,pString);
	mprintf(uiX,uiY,pString);
}


static void DrawName(STR16 pName, INT16 sRowIndex, INT32 iFont)
{
	UINT16 usX=0;
	UINT16 usY=0;

	if( sRowIndex < FIRST_VEHICLE )
	{
		FindFontCenterCoordinates((short)NAME_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE)), (short)NAME_WIDTH, (short)Y_SIZE, pName, (long)iFont, &usX, &usY);
	}
	else
	{
		FindFontCenterCoordinates((short)NAME_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE) + 6), (short)NAME_WIDTH, (short)Y_SIZE, pName, (long)iFont, &usX, &usY);
	}

	//RestoreExternBackgroundRect(NAME_X, ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), NAME_WIDTH, Y_SIZE);
	DrawString( pName, (UINT16)usX, ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), iFont);
}


static void DrawAssignment(INT16 sCharNumber, INT16 sRowIndex, INT32 iFont)
{
	UINT16 usX=0;
	UINT16 usY=0;
	wchar_t sString[32];


	GetMapscreenMercAssignmentString( MercPtrs[ gCharactersList[ sCharNumber ].usSolID ], sString );

	if( sRowIndex < FIRST_VEHICLE )
	{
		FindFontCenterCoordinates((short)ASSIGN_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE)), (short)ASSIGN_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}
	else
	{
		FindFontCenterCoordinates((short)ASSIGN_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE) + 6), (short)ASSIGN_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}

	if( fFlashAssignDone == TRUE )
	{
		if( Menptr[gCharactersList[sCharNumber].usSolID].fDoneAssignmentAndNothingToDoFlag )
		{
			SetFontForeground( FONT_RED );
		}
	}

	//RestoreExternBackgroundRect(ASSIGN_X-2, ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), ASSIGN_WIDTH+2, Y_SIZE);
	DrawString(sString, (UINT16)usX, ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), iFont);
}


static void DrawLocation(INT16 sCharNumber, INT16 sRowIndex, INT32 iFont)
{
	UINT16 usX=0;
	UINT16 usY=0;
	wchar_t sString[32];

	GetMapscreenMercLocationString( MercPtrs[ gCharactersList[ sCharNumber ].usSolID ], sString, lengthof(sString));

	if( sRowIndex < FIRST_VEHICLE )
	{
		// center
		FindFontCenterCoordinates((short)LOC_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE)), (short)LOC_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}
	else
	{
		FindFontCenterCoordinates((short)LOC_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE) + 6), (short)LOC_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}
	// restore background
	//RestoreExternBackgroundRect(LOC_X, ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), LOC_WIDTH, Y_SIZE);

	// draw string
	DrawString( sString,((UINT16)(usX)), ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), ((UINT32)iFont));
}


static void DrawDestination(INT16 sCharNumber, INT16 sRowIndex, INT32 iFont)
{
	UINT16 usX=0;
	UINT16 usY=0;
	wchar_t sString[32];

	GetMapscreenMercDestinationString( MercPtrs[ gCharactersList[ sCharNumber ].usSolID ], sString, lengthof(sString));

	if ( wcslen( sString ) == 0 )
	{
		return;
	}

	if( sRowIndex < FIRST_VEHICLE )
	{
		FindFontCenterCoordinates((short)DEST_ETA_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE)), (short)DEST_ETA_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}
	else
	{
		FindFontCenterCoordinates((short)DEST_ETA_X + 1, (short)(Y_START+(sRowIndex*Y_SIZE) + 6 ), (short)DEST_ETA_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}

	//RestoreExternBackgroundRect(DEST_ETA_X+1, ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), DEST_ETA_WIDTH-1, Y_SIZE);
	// ShowDestinationOfPlottedPath( sString );
	DrawString( sString,((UINT16)(usX)), ((UINT16)(usY+(Y_OFFSET*sRowIndex+1))), ((UINT32)iFont));
}


static void DrawTimeRemaining(INT16 sCharNumber, INT32 iFont, UINT8 ubFontColor)
{
	UINT16 usX=0;
	UINT16 usY=0;
	wchar_t sString[32];


	GetMapscreenMercDepartureString( MercPtrs[ gCharactersList[ sCharNumber ].usSolID ], sString, lengthof(sString), &ubFontColor );

	// if merc is highlighted, override the color decided above with bright white
	if( sCharNumber == ( INT16 ) giHighLine )
	{
		ubFontColor = FONT_WHITE;
	}

	SetFont(iFont);
	SetFontForeground( ubFontColor );

	if( sCharNumber < FIRST_VEHICLE )
	{
		FindFontCenterCoordinates((short)TIME_REMAINING_X + 1, (short)(Y_START+(sCharNumber*Y_SIZE)), (short)TIME_REMAINING_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}
	else
	{
		FindFontCenterCoordinates((short)TIME_REMAINING_X + 1, (short)(Y_START+(sCharNumber*Y_SIZE) + 6 ), (short)TIME_REMAINING_WIDTH, (short)Y_SIZE, sString, (long)iFont, &usX, &usY);
	}

	//RestoreExternBackgroundRect(TIME_REMAINING_X, ((UINT16)(usY+(Y_OFFSET*sCharNumber+1))), TIME_REMAINING_WIDTH, Y_SIZE);
	DrawString(sString,((UINT16)(usX)), ((UINT16)(usY+(Y_OFFSET*sCharNumber+1))), ((UINT32)iFont));
}


static void RenderMapHighlight(INT16 sMapX, INT16 sMapY, UINT16 usLineColor, BOOLEAN fStationary);
static void RestoreMapSectorCursor(INT16 sMapX, INT16 sMapY);


static void RenderMapCursorsIndexesAnims(void)
{
	BOOLEAN fSelectedSectorHighlighted = FALSE;
	BOOLEAN fSelectedCursorIsYellow = TRUE;
	UINT16 usCursorColor;
	UINT32 uiDeltaTime;
	static INT16 sPrevHighlightedMapX = -1, sPrevHighlightedMapY = -1;
	static INT16 sPrevSelectedMapX = -1, sPrevSelectedMapY = -1;
	static BOOLEAN fFlashCursorIsYellow = FALSE;
	BOOLEAN fDrawCursors;
	BOOLEAN fHighlightChanged = FALSE;


	HandleAnimationOfSectors( );

	if( gfBlitBattleSectorLocator )
	{
		HandleBlitOfSectorLocatorIcon( gubPBSectorX, gubPBSectorY, gubPBSectorZ, LOCATOR_COLOR_RED );
	}

	fDrawCursors = CanDrawSectorCursor( );

	// if mouse cursor is over a map sector
	if ( fDrawCursors && ( GetMouseMapXY( &gsHighlightSectorX, &gsHighlightSectorY ) ) )
	{
		// handle highlighting of sector pointed at ( WHITE )

		// if we're over a different sector than when we previously blitted this
		if ( ( gsHighlightSectorX != sPrevHighlightedMapX ) || ( gsHighlightSectorY != sPrevHighlightedMapY ) || gfMapPanelWasRedrawn )
		{
			if ( sPrevHighlightedMapX != -1 && sPrevHighlightedMapY != -1 )
			{
				RestoreMapSectorCursor( sPrevHighlightedMapX, sPrevHighlightedMapY );
			}

			// draw WHITE highlight rectangle
			RenderMapHighlight( gsHighlightSectorX, gsHighlightSectorY, Get16BPPColor( RGB_WHITE ), FALSE );

			sPrevHighlightedMapX = gsHighlightSectorX;
			sPrevHighlightedMapY = gsHighlightSectorY;

			fHighlightChanged = TRUE;
		}
	}
	else
	{
		// nothing now highlighted
		gsHighlightSectorX = -1;
		gsHighlightSectorY = -1;

		if ( sPrevHighlightedMapX != -1 && sPrevHighlightedMapY != -1 )
		{
			RestoreMapSectorCursor( sPrevHighlightedMapX, sPrevHighlightedMapY );
			fHighlightChanged = TRUE;
		}

		sPrevHighlightedMapX = -1;
		sPrevHighlightedMapY = -1;
	}


	// handle highlighting of selected sector ( YELLOW ) - don't show it while plotting movement
	if ( fDrawCursors && ( bSelectedDestChar == -1 ) && ( fPlotForHelicopter == FALSE ) )
	{
		// if mouse cursor is over the currently selected sector
		if( ( gsHighlightSectorX == sSelMapX ) && ( gsHighlightSectorY == sSelMapY ) )
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
		RenderMapHighlight( sSelMapX, sSelMapY, usCursorColor, TRUE );

		if ( ( sPrevSelectedMapX != sSelMapX ) || ( sPrevSelectedMapY != sSelMapY ) )
		{
			sPrevSelectedMapX = sSelMapX;
			sPrevSelectedMapY = sSelMapY;

			fHighlightChanged = TRUE;
		}
	}
	else
	{
		// erase yellow highlight cursor
		if ( sPrevSelectedMapX != -1 && sPrevSelectedMapY != -1 )
		{
			RestoreMapSectorCursor( sPrevSelectedMapX, sPrevSelectedMapY );
			fHighlightChanged = TRUE;
		}

		sPrevSelectedMapX = -1;
		sPrevSelectedMapY = -1;
	}


	if ( fHighlightChanged || gfMapPanelWasRedrawn )
	{
		// redraw sector index letters and numbers
/*
		if( fZoomFlag )
			DrawMapIndexSmallMap( fSelectedCursorIsYellow );
		else
*/
			DrawMapIndexBigMap( fSelectedCursorIsYellow );
	}
}


static BOOLEAN AnyMovableCharsInOrBetweenThisSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);
static BOOLEAN CanMoveBullseyeAndClickedOnIt(INT16 sMapX, INT16 sMapY);
static void CancelChangeArrivalSectorMode(void);
static void CancelOrShortenPlottedPath(void);
static void CreateBullsEyeOrChopperSelectionPopup(void);
static void GetMapKeyboardInput(UINT32* puiNewEvent);
static void PollLeftButtonInMapView(UINT32* puiNewEvent);
static void PollRightButtonInMapView(UINT32* puiNewEvent);
static void StartChangeSectorArrivalMode(void);
static void StartConfirmMapMoveMode(INT16 sMapY);


// Drawing the Map
static UINT32 HandleMapUI(void)
{
	UINT32 uiNewEvent = MAP_EVENT_NONE;
	INT16 sMapX = 0, sMapY = 0;
	INT8 bMapZ = 0;
	INT16 sX, sY;
	UINT32 uiNewScreen = MAP_SCREEN;
	BOOLEAN fWasAlreadySelected;


	// Get Input from keyboard
	GetMapKeyboardInput( &uiNewEvent );


	CreateDestroyMapInvButton();

	// Get mouse
	PollLeftButtonInMapView( &uiNewEvent );
	PollRightButtonInMapView( &uiNewEvent );

	// Switch on event
	switch( uiNewEvent )
	{
		case MAP_EVENT_NONE:
			break;

		case MAP_EVENT_PLOT_PATH:
		         GetMouseMapXY(&sMapX, &sMapY);

/*
						 // translate screen values to map grid values for zoomed in
						 if(fZoomFlag)
						 {
								 sMapX=(UINT16)iZoomX/MAP_GRID_X+sMapX;
								 sMapX=sMapX/2;
								 sMapY=(UINT16)iZoomY/MAP_GRID_Y+sMapY;
								 sMapY=sMapY/2;
						 }
*/


						 // plotting for the chopper?
						 if( fPlotForHelicopter == TRUE )
						 {
/*
							 if( IsSectorOutOfTheWay( sMapX, sMapY ) == TRUE )
							 {
								 if( gfAllowSkyriderTooFarQuote == TRUE )
								 {
									 SkyRiderTalk( DESTINATION_TOO_FAR );
								 }

								 return( MAP_SCREEN );
							 }
*/

							 PlotPathForHelicopter( sMapX, sMapY );
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
							 if( GetLastSectorIdInCharactersPath( &Menptr[gCharactersList[bSelectedDestChar].usSolID] ) != ( sMapX + ( sMapY * MAP_WORLD_X ) ) )
							 {
								 sX = ( GetLastSectorIdInCharactersPath( &Menptr[gCharactersList[bSelectedDestChar].usSolID]  ) % MAP_WORLD_X );
								 sY = ( GetLastSectorIdInCharactersPath( &Menptr[gCharactersList[bSelectedDestChar].usSolID]  ) / MAP_WORLD_X );
								 RestoreBackgroundForMapGrid( sX, sY );
								// fMapPanelDirty = TRUE;
							 }

							//SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );

							if( ( IsTheCursorAllowedToHighLightThisSector( sMapX, sMapY ) == TRUE ) &&
									( SectorInfo[ ( SECTOR( sMapX, sMapY ) ) ].ubTraversability[ THROUGH_STRATEGIC_MOVE ] != GROUNDBARRIER ) )
							{
								// Can we get go there?  (NULL temp character path)
								if ( GetLengthOfPath( pTempCharacterPath ) > 0 )
								{
									PlotPathForCharacter( &Menptr[gCharactersList[bSelectedDestChar].usSolID], sMapX, sMapY, FALSE );

									// copy the path to every other selected character
									CopyPathToAllSelectedCharacters( GetSoldierMercPathPtr( MercPtrs[ gCharactersList[ bSelectedDestChar ].usSolID ] ) );

									StartConfirmMapMoveMode( sMapY );
									fMapPanelDirty = TRUE;
									fTeamPanelDirty = TRUE;	// update team panel desinations
								}
								else
								{
									// means it's a vehicle and we've clicked an off-road sector
									MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapErrorString[ 40 ] );
								}
							}
						}
			break;


		case MAP_EVENT_CANCEL_PATH:
			CancelOrShortenPlottedPath( );
			break;


/*
    case MAP_EVENT_SELECT_SECTOR:
			// will select the sector the selected merc is in

			sMapX=Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorX;
			sMapY=Menptr[gCharactersList[bSelectedInfoChar].usSolID].sSectorY;
			bMapZ=Menptr[gCharactersList[bSelectedInfoChar].usSolID].bSectorZ;

			if( ( sSelMapX != sMapX || sSelMapY != sMapY || iCurrentMapSectorZ != bMapZ ) &&
					( gTacticalStatus.fDidGameJustStart == FALSE ) && ( gfPreBattleInterfaceActive == FALSE ) )
			{
				ChangeSelectedMapSector( sMapX, sMapY, bMapZ );

				fTeamPanelDirty = TRUE;

				fMapScreenBottomDirty = TRUE;
        bSelectedDestChar=-1;
			}

			break;
*/

		case MAP_EVENT_CLICK_SECTOR:

			// Get Current mouse position
			if ( GetMouseMapXY( &sMapX, &sMapY) )
			{
/*
				if( fZoomFlag == TRUE )
				{
					// convert to zoom out coords from screen coords
		      sMapX = ( INT16 )( iZoomX / MAP_GRID_X + sMapX ) / 2;
		      sMapY = ( INT16 )( iZoomY / MAP_GRID_Y + sMapY ) / 2;
					//sMapX = ( INT16 ) ( ( ( iZoomX ) / ( MAP_GRID_X * 2) ) + sMapX / 2 );
					//sMapX = ( INT16 ) ( ( ( iZoomY ) / ( MAP_GRID_Y * 2) ) + sMapY / 2 );
				}
*/

				// not zoomed out, make sure this is a valid sector
        if( IsTheCursorAllowedToHighLightThisSector( sMapX, sMapY ) == FALSE )
				{
          // do nothing, return
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
						if ( ( gpItemPointerSoldier->sSectorX != sSelMapX ) ||
								 ( gpItemPointerSoldier->sSectorY != sSelMapY ) ||
								 ( gpItemPointerSoldier->bSectorZ != iCurrentMapSectorZ ) )
						{
							ChangeSelectedMapSector( gpItemPointerSoldier->sSectorX, gpItemPointerSoldier->sSectorY, gpItemPointerSoldier->bSectorZ );
						}
					}

					// if not already in sector inventory
					if ( !fShowMapInventoryPool )
					{
						// start it up ( NOTE: for the item OWNER'S sector, regardless of which sector player clicks )
						fShowMapInventoryPool = TRUE;
						CreateDestroyMapInventoryPoolButtons( TRUE );
					}

					return( MAP_SCREEN );
				}


				// don't permit other click handling while item is in cursor (entering PBI would permit item teleports, etc.)
				Assert( !fMapInventoryItem );


				// this doesn't change selected sector
				if ( gfInChangeArrivalSectorMode )
				{
					if( SectorInfo[ ( SECTOR( sMapX, sMapY ) ) ].ubTraversability[ THROUGH_STRATEGIC_MOVE ] != GROUNDBARRIER )
					{
						// if it's not enemy air controlled
						if ( StrategicMap[ CALCULATE_STRATEGIC_INDEX( sMapX, sMapY ) ].fEnemyAirControlled == FALSE )
						{
							CHAR16 sMsgString[ 128 ], sMsgSubString[ 64 ];

							// move the landing zone over here
							gsMercArriveSectorX = sMapX;
							gsMercArriveSectorY = sMapY;

							// change arrival sector for all mercs currently in transit who are showing up at the landing zone
							UpdateAnyInTransitMercsWithGlobalArrivalSector();

							// we're done, cancel this mode
							CancelChangeArrivalSectorMode();

							// get the name of the sector
							GetSectorIDString( sMapX, sMapY, 0, sMsgSubString, lengthof(sMsgSubString), FALSE );

							// now build the string
							swprintf( sMsgString, lengthof(sMsgString), pBullseyeStrings[ 1 ], sMsgSubString );

							// confirm the change with overlay message
							MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, sMsgString );

							// update destination column for any mercs in transit
							fTeamPanelDirty = TRUE;
						}
						else
						{
							// message: not allowed, don't have airspace secured
							MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pBullseyeStrings[ 2 ] );
						}
					}

					return( MAP_SCREEN );
				}
				else	// not already changing arrival sector
				{
					if ( CanMoveBullseyeAndClickedOnIt( sMapX, sMapY ) )
					{
						// if the click is ALSO over the helicopter icon
						// NOTE: The helicopter icon is NOT necessarily directly over the helicopter's current sector!!!
						if( CheckForClickOverHelicopterIcon( sMapX, sMapY ) == TRUE )
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


				// if new map sector was clicked on
				if( ( sSelMapX != sMapX ) || ( sSelMapY != sMapY ) )
				{
					fWasAlreadySelected = FALSE;

					// select the clicked sector, retaining the same sublevel depth
					ChangeSelectedMapSector( sMapX, sMapY, ( INT8 )iCurrentMapSectorZ );
				}
				else
				{
					fWasAlreadySelected = TRUE;
				}


				// if showing item counts on the map, and not already in sector inventory
				if ( fShowItemsFlag && !fShowMapInventoryPool )
				{
					// show sector inventory for this clicked sector
					ChangeSelectedMapSector( sMapX, sMapY, ( INT8 ) iCurrentMapSectorZ );

					fShowMapInventoryPool = TRUE;
					CreateDestroyMapInventoryPoolButtons( TRUE );

					return( MAP_SCREEN );
				}


				if( gfBlitBattleSectorLocator &&
						sMapX == gubPBSectorX && sMapY == gubPBSectorY && iCurrentMapSectorZ == gubPBSectorZ )
				{ //Bring up a non-persistant version of mapscreen if the user clicks on the sector where a
					//battle is taking place.
					InitPreBattleInterface( NULL, FALSE );
					return( MAP_SCREEN );
				}


				// if we're in airspace mode
				if( fShowAircraftFlag == TRUE )
				{
					// if not moving soldiers, and not yet plotting the helicopter
					if ( ( bSelectedDestChar == -1 ) && ( fPlotForHelicopter == FALSE ) )
					{
						// if we're on the surface level, and the click is over the helicopter icon
						// NOTE: The helicopter icon is NOT necessarily directly over the helicopter's current sector!!!
						if( ( iCurrentMapSectorZ == 0 ) && CheckForClickOverHelicopterIcon( sMapX, sMapY ) == TRUE )
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
						if ( AnyMovableCharsInOrBetweenThisSector( sMapX, sMapY, ( INT8 )iCurrentMapSectorZ ) )
						{
							// if showing the surface level map
							if( iCurrentMapSectorZ == 0 )
							{
								TurnOnShowTeamsMode( );

								// NOTE: must allow move box to come up, since there may be movable characters between sectors which are
								// unaffected by combat / hostiles / air raid in the sector proper itself!!
								// This also allows all strategic movement error handling to be centralized in CanCharacterMoveInStrategic()

								// start the move box menu
								SetUpMovingListsForSector( sMapX, sMapY, ( INT16 )iCurrentMapSectorZ );
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

//Kris -- added hook so I can access AIView in non-release mode.
#ifdef JA2BETAVERSION
		case MAP_EVENT_VIEWAI:
			SetPendingNewScreen( AIVIEWER_SCREEN );
			CreateDestroyMapInvButton();
			break;
#endif
	}


	// if we pressed something that will cause a screen change
	if ( guiPendingScreen != NO_PENDING_SCREEN )
	{
		uiNewScreen = guiPendingScreen;
	}

	return( uiNewScreen );
}


static void ChangeCharacterListSortMethod(INT32 iValue);
static void RequestContractMenu(void);
static void RequestToggleMercInventoryPanel(void);
static void SelectAllCharactersInSquad(INT8 bSquadNumber);


static void GetMapKeyboardInput(UINT32* puiNewEvent)
{
  InputAtom					InputEvent;
	POINT  MousePos;
	INT8 bSquadNumber;
	UINT8 ubGroupId = 0;
	BOOLEAN fCtrl, fAlt;

#ifndef JA2DEMO
	INT16 sMapX, sMapY;
#endif


	fCtrl = _KeyDown( CTRL );
	fAlt = _KeyDown( ALT );

  while( DequeueEvent( &InputEvent ) )
//		while( DequeueSpecificEvent( &InputEvent, KEY_DOWN ) )		// doesn't work for some reason
  {
		GetCursorPos(&MousePos);

		// HOOK INTO MOUSE HOOKS
		switch(InputEvent.usEvent)
	  {
			case LEFT_BUTTON_DOWN:
			case LEFT_BUTTON_UP:
			case RIGHT_BUTTON_DOWN:
			case RIGHT_BUTTON_UP:
			case RIGHT_BUTTON_REPEAT:
			case LEFT_BUTTON_REPEAT:
				MouseSystemHook(InputEvent.usEvent, MousePos.x, MousePos.y, _LeftButtonDown, _RightButtonDown);
				break;
	  }


		if( InputEvent.usEvent == KEY_DOWN )
		{
			// if game is paused because of player, unpause with any key
			if ( gfPauseDueToPlayerGamePause )
			{
				HandlePlayerPauseUnPauseOfGame( );
				continue;
			}

			if( IsMapScreenHelpTextUp() )
			{
				// stop mapscreen text
				StopMapScreenHelpText( );
				continue;
			}

			// handle for fast help text for interface stuff
			if( IsTheInterfaceFastHelpTextActive() )
			{
				ShutDownUserDefineHelpTextRegions( );
			}


			switch( InputEvent.usParam )
			{
				case SDLK_ESCAPE:
					gfDontStartTransitionFromLaptop = TRUE;


					if( gfPreBattleInterfaceActive && !gfPersistantPBI )
					{ //Non persistant PBI.  Allow ESC to close it and return to mapscreen.
						KillPreBattleInterface();
						gpBattleGroup = NULL;
						return;
					}

					#ifdef JA2DEMO
						HandleLeavingOfMapScreenDuringDemo( );
						return;
					#else

					if ( gfInChangeArrivalSectorMode )
					{
						CancelChangeArrivalSectorMode( );
						MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pBullseyeStrings[ 3 ] );
					}
					// ESC cancels MAP UI messages, unless we're in confirm map move mode
					else if( ( giUIMessageOverlay != -1 ) && !gfInConfirmMapMoveMode )
					{
						CancelMapUIMessage( );
					}
					else if( IsMapScreenHelpTextUp( ) )
					{
						StopMapScreenHelpText( );
					}
        	else if ( gpCurrentTalkingFace != NULL && gpCurrentTalkingFace->fTalking )
          {
            // ATE: We want to stop speech if somebody is talking...
            StopAnyCurrentlyTalkingSpeech( );
          }
					else if( fShowUpdateBox )
					{
						if( fShowUpdateBox )
						{
							EndUpdateBox( FALSE );  // stop time compression
						}
					}
					else if(fShowDescriptionFlag)
					{
						DeleteItemDescriptionBox( );
					}
					// plotting movement?
					else if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
					{
						AbortMovementPlottingMode( );
					}
					else if( fShowAssignmentMenu )
					{
						// dirty region
						fTeamPanelDirty = TRUE;
						fMapPanelDirty = TRUE;
						fCharacterInfoPanelDirty = TRUE;

						// stop showing current assignment box
						if( fShowAttributeMenu == TRUE )
						{
							fShowAttributeMenu = FALSE;
							fMapPanelDirty = TRUE;
						}
						else if( fShowTrainingMenu == TRUE )
						{
							fShowTrainingMenu = FALSE;
						}
						else if( fShowSquadMenu == TRUE )
						{
							fShowSquadMenu = FALSE;
						}
						else if( fShowRepairMenu == TRUE )
						{
							fShowRepairMenu = FALSE;
						}
						else
						{
							fShowAssignmentMenu = FALSE;
						}
						giAssignHighLine = -1;
						// restore background to glow region
						RestoreBackgroundForAssignmentGlowRegionList( );
					}
					else if( fShowContractMenu == TRUE )
					{
						fShowContractMenu = FALSE;

						// restore contract glow region
						RestoreBackgroundForContractGlowRegionList( );
						fTeamPanelDirty = TRUE;
						fCharacterInfoPanelDirty = TRUE;
						giContractHighLine = -1;
					}
					// in militia popup?
					else if( ( sSelectedMilitiaTown != 0 ) && ( sGreensOnCursor == 0 ) && ( sRegularsOnCursor == 0 ) && ( sElitesOnCursor == 0 ) )
					{
						sSelectedMilitiaTown = 0;
						fMapPanelDirty = TRUE;
					}
					else if( fShowTownInfo == TRUE )
					{
						fShowTownInfo = FALSE;
						CreateDestroyScreenMaskForAssignmentAndContractMenus( );
					}
					else if(  fShowDescriptionFlag )
					{
						if( gMPanelRegion.Cursor != EXTERN_CURSOR )
						{
							DeleteItemDescriptionBox( );
						}
					}
					else if( InKeyRingPopup( ) == TRUE )
					{
						DeleteKeyRingPopup( );
						fTeamPanelDirty = TRUE;
					}
					else if( fShowInventoryFlag == TRUE )
					{
						if ( gMPanelRegion.Cursor != EXTERN_CURSOR && !InItemStackPopup() )
						{
							fEndShowInventoryFlag = TRUE;
						}
					}
					else if( MultipleCharacterListEntriesSelected( ) )
					{
						ResetSelectedListForMapScreen( );
						if( bSelectedInfoChar != -1 )
						{
							SetEntryInSelectedCharacterList( bSelectedInfoChar );
						}
						fTeamPanelDirty = TRUE;
						fCharacterInfoPanelDirty = TRUE;
					}
					else
					{
						RequestTriggerExitFromMapscreen( MAP_EXIT_TO_TACTICAL );
					}
					break; // end of ESC

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
				case SDLK_F6:
					ChangeCharacterListSortMethod(InputEvent.usParam - SDLK_F1);
					break;

				case SDLK_F7:
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							if ( bSelectedInfoChar != -1 )
							{
								SOLDIERTYPE *pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];
								if ( pSoldier->inv[ HANDPOS ].usItem != 0 )
								{
									pSoldier->inv[ HANDPOS ].bStatus[ 0 ] = 2;
								}
							}
						}
						if( fCtrl )
						{
							if ( bSelectedInfoChar != -1 )
							{
								SOLDIERTYPE *pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];
								if ( pSoldier->inv[ HANDPOS ].usItem != 0 )
								{
									pSoldier->inv[ HANDPOS ].usItem = GUN_BARREL_EXTENDER;
								}
							}
						}
					#endif
					break;

				case SDLK_F8:
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							// reduce balance to $500
							AddTransactionToPlayersBook( PAYMENT_TO_NPC, SKYRIDER, GetWorldTotalMin( ), - ( LaptopSaveInfo.iCurrentBalance - 500 ) );
						}
					#endif
					break;

				case SDLK_F9:
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							UINT8 ubSamIndex;

							// ALT-F9: Reveal all SAM sites
							for( ubSamIndex = 0; ubSamIndex < NUMBER_OF_SAM_SITES; ubSamIndex++ )
							{
								SetSAMSiteAsFound( ubSamIndex );
							}
						}
					#endif
					break;

				case SDLK_F10:
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							if ( bSelectedInfoChar != -1 )
							{
								// ALT-F10: force selected character asleep (ignores breathmax)
								PutMercInAsleepState( MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ] );
							}
						}
					#endif
					break;

/*
				case SDLK_F11:
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							// ALT-F11: make all sectors player controlled
							ClearMapControlledFlags( );
							fMapPanelDirty = TRUE;
						}
					#endif
					break;
*/

				case SDLK_F12:
					#ifdef JA2BETAVERSION
						*puiNewEvent = MAP_EVENT_VIEWAI;
					#endif
					break;

				case '+':
				case '=':
					if ( CommonTimeCompressionChecks() == FALSE )
						RequestIncreaseInTimeCompression();
					break;

				case '-':
				case '_':
					if ( CommonTimeCompressionChecks() == FALSE )
						RequestDecreaseInTimeCompression();
					break;

				case SDLK_SPACE:
						if( fShowUpdateBox )
						{
							EndUpdateBox( TRUE );  // restart time compression
						}
						else
						{
							// toggle time compression
							if ( CommonTimeCompressionChecks() == FALSE )
								RequestToggleTimeCompression();
						}
					break;

				case '`':
					#ifdef JA2TESTVERSION
						if( fCtrl )
						{
							if( bSelectedInfoChar != -1 )
							{
								TownMilitiaTrainingCompleted( &Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ], sSelMapX, sSelMapY );
							}
						}
					#endif
					break;

				case '\\':
					#ifdef JA2TESTVERSION
						if( fCtrl )
						{
							DumpItemsList();
						}
					#endif
					break;

				case '>':
					#ifdef JA2TESTVERSION
						if( fCtrl )
						{
							// free
						}
					#endif
					break;

				case '?':
					#ifdef JA2TESTVERSION
						if( fCtrl )
						 MapScreenMessage( 0, MSG_DEBUG, L"JA2Clock = %d", GetJA2Clock() );
						else
						 MapScreenMessage( 0, MSG_DEBUG, L"Mouse X,Y = %d,%d", MSYS_CurrentMX, MSYS_CurrentMY );
					#endif
					break;

				case '/':
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							if( bSelectedInfoChar != -1 )
							{
								StatChange( &Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ], EXPERAMT, 1000, FROM_SUCCESS );
							}
						}
					#endif
					break;


				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					// multi-selects all characters in that squad.  SHIFT key and 1-0 for squads 11-20
					bSquadNumber = ( INT8 ) ( InputEvent.usParam - '1' );	// internal squad #s start at 0
					SelectAllCharactersInSquad( bSquadNumber );
					break;

				case '0':
					SelectAllCharactersInSquad( 9 ); // internal squad #s start at 0
					break;

				case '!':
					SelectAllCharactersInSquad( 10 ); // internal squad #s start at 0
					break;
				case '@':
					SelectAllCharactersInSquad( 11 ); // internal squad #s start at 0
					break;
				case '#':
					SelectAllCharactersInSquad( 12 ); // internal squad #s start at 0
					break;
				case '$':
					SelectAllCharactersInSquad( 13 ); // internal squad #s start at 0
					break;
				case '%':
					SelectAllCharactersInSquad( 14 ); // internal squad #s start at 0
					break;
				case '^':
					SelectAllCharactersInSquad( 15 ); // internal squad #s start at 0
					break;
				case '&':
					SelectAllCharactersInSquad( 16 ); // internal squad #s start at 0
					break;
				case '*':
					SelectAllCharactersInSquad( 17 ); // internal squad #s start at 0
					break;
				case '(':
					SelectAllCharactersInSquad( 18 ); // internal squad #s start at 0
					break;
				case ')':
					SelectAllCharactersInSquad( 19 ); // internal squad #s start at 0
					break;


				case 'a':
						if( fAlt )
						{
							if( giHighLine != -1 )
							{
								if( gCharactersList[ giHighLine ].fValid == TRUE )
								{
									bSelectedAssignChar = ( INT8 )giHighLine;
									RebuildAssignmentsBox( );
									ChangeSelectedInfoChar( ( INT8 ) giHighLine, FALSE );
									fShowAssignmentMenu = TRUE;
								}
							}
							else if( bSelectedInfoChar != -1 )
							{
								if( gCharactersList[ bSelectedInfoChar ].fValid == TRUE )
								{
									bSelectedAssignChar = ( INT8 )bSelectedInfoChar;
									RebuildAssignmentsBox( );
									fShowAssignmentMenu = TRUE;
								}
							}
						}
						else if( fCtrl )
						{
							if( CHEATER_CHEAT_LEVEL( ) )
							{
								if(	gfAutoAmbush ^= 1 )
									ScreenMsg( FONT_WHITE, MSG_TESTVERSION, L"Enemy ambush test mode enabled." );
								else
									ScreenMsg( FONT_WHITE, MSG_TESTVERSION, L"Enemy ambush test mode disabled." );
							}
						}
						else
						{
							if( gfPreBattleInterfaceActive )
							{
								//activate autoresolve in prebattle interface.
								ActivatePreBattleAutoresolveAction();
							}
							else
							{
								// only handle border button keyboard equivalents if the button is visible!
								if ( !fShowMapInventoryPool )
								{
									ToggleAirspaceMode();
								}
							}
						}
					break;


				case 'b':
/*
					#ifndef JA2DEMO
						// CTRL-B: make player's perception of all sectors correct!
						if( ( fCtrl )&&( CHEATER_CHEAT_LEVEL( ) ) )
						{
							for ( sMapX = 1; sMapX <= 16; sMapX++ )
							{
								for ( sMapY = 1; sMapY <= 16; sMapY++ )
								{
									MakePlayerPerceptionOfSectorControlCorrect( sMapX, sMapY, 0 );
								}
							}
						}
					#endif
*/
					break;

				case 'c':
					RequestContractMenu();
					break;

				case 'd':
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							// prints out a text file in C:\TEMP telling you how many stat change chances/successes each profile merc got
							TestDumpStatChanges();
						}
					#endif
					break;

				case 'e':
					if( gfPreBattleInterfaceActive )
					{ //activate enter sector in prebattle interface.
						gfHotKeyEnterSector = TRUE;
					}
					break;
				case 'f':
#ifdef JA2TESTVERSION
					// CTRL-F: Refuel vehicle
					if( ( fCtrl ) && ( bSelectedInfoChar != -1 ) )
					{
						SOLDIERTYPE *pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];

						if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
						{
							pSoldier->sBreathRed = 10000;
							pSoldier->bBreath = 100;
							ScreenMsg( FONT_MCOLOR_RED, MSG_TESTVERSION, L"Vehicle refueled" );

							fTeamPanelDirty = TRUE;
							fCharacterInfoPanelDirty = TRUE;
						}
					}
#endif
					if( fAlt )
					{
						if ( INFORMATION_CHEAT_LEVEL( ) )
						{
						  //Toggle Frame Rate Display
						  gbFPSDisplay = !gbFPSDisplay;
						  DisableFPSOverlay( (BOOLEAN)!gbFPSDisplay );
            }
					}
					break;
				#endif
				case 'h':
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							// set up the helicopter over Omerta (if it's not already set up)
							SetUpHelicopterForPlayer( 9,1 );
							// raise Drassen loyalty to minimum that will allow Skyrider to fly
							if ( gTownLoyalty[ DRASSEN ].fStarted && ( gTownLoyalty[ DRASSEN ].ubRating < LOYALTY_LOW_THRESHOLD ) )
							{
								SetTownLoyalty( DRASSEN, LOYALTY_LOW_THRESHOLD );
							}
							TurnOnAirSpaceMode( );
						}
						else
					#endif
						{
							// ARM: Feb01/98 - Cancel out of mapscreen movement plotting if Help subscreen is coming up
							if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
							{
								AbortMovementPlottingMode( );
							}

							ShouldTheHelpScreenComeUp( HELP_SCREEN_MAPSCREEN, TRUE );
						}

//					fShowMapScreenHelpText = TRUE;
					break;

				#ifndef JA2DEMO
				case 'i':
					#ifdef JA2TESTVERSION
/*
						if( fAlt )
						{
							InitializeMines();
							fMapPanelDirty = TRUE;
						}
						else
*/
						if( fCtrl )
						{
							fDisableJustForIan = ! fDisableJustForIan;
						}
						else
						{
							// only handle border button keyboard equivalents if the button is visible!
							if ( !fShowMapInventoryPool )
							{
								ToggleItemsFilter();
							}
						}
					#else
						// only handle border button keyboard equivalents if the button is visible!
						if ( !fShowMapInventoryPool )
						{
							ToggleItemsFilter();
						}
					#endif

					break;
				case 'l':
					if( fAlt )
					{
						// although we're not actually going anywhere, we must still be in a state where this is permitted
						if ( AllowedToExitFromMapscreenTo( MAP_EXIT_TO_LOAD ) )
						{
							DoQuickLoad();
						}
					}
					else if( fCtrl )
					{
						// go to LOAD screen
						gfSaveGame = FALSE;
						RequestTriggerExitFromMapscreen( MAP_EXIT_TO_LOAD );
					}
					else
					{
						// go to LAPTOP
						RequestTriggerExitFromMapscreen( MAP_EXIT_TO_LAPTOP );
					}
					break;
				case 'm':
					// only handle border button keyboard equivalents if the button is visible!
					if ( !fShowMapInventoryPool )
					{
						// toggle show mines flag
						ToggleShowMinesMode();
					}

					break;
				case 'n':
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							static UINT16 gQuoteNum = 0;
							// Get Soldier
							TacticalCharacterDialogue( MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ], gQuoteNum );
							gQuoteNum++;
						}
						else if( fCtrl )
						{
							static UINT16 gQuoteNum = 0;
							// Get Soldier
							if ( giHighLine != -1 )
							{
								TacticalCharacterDialogue( MercPtrs[ gCharactersList[ giHighLine ].usSolID ], gQuoteNum );
								gQuoteNum++;
							}
						}
					#endif
					break;
				case 'o':
					if( fAlt )
					{	// toggle if Orta & Tixa have been found
						#ifdef JA2TESTVERSION
							fFoundOrta = !fFoundOrta;
							fFoundTixa = !fFoundTixa;
							fMapPanelDirty = TRUE;
						#endif
					}
					else
					{
						// go to OPTIONS screen
						RequestTriggerExitFromMapscreen( MAP_EXIT_TO_OPTIONS );
					}
					break;

					case 'p':
					#ifdef JA2TESTVERSION
						#ifndef JA2DEMO
							if( fCtrl )
							{
								// CTRL-P: Display player's highest progress percentage
								DumpSectorDifficultyInfo();
							}
						#endif
					#endif

					#ifdef JA2TESTVERSION
						// ALT-P: Make the selected character a POW!
						if( ( fAlt ) && ( bSelectedInfoChar != -1 ) )
						{
							SOLDIERTYPE *pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];

							EnemyCapturesPlayerSoldier( pSoldier );

							if ( pSoldier->bInSector )
							{
								RemoveSoldierFromTacticalSector( pSoldier, TRUE );
							}

							fTeamPanelDirty = TRUE;
							fCharacterInfoPanelDirty = TRUE;
							fMapPanelDirty = TRUE;
						}
					#endif
					break;

				case 'q':
					#ifdef JA2TESTVERSION
						if( fAlt )
						{
							// initialize miners if not already done so (fakes entering Drassen mine first)
							HandleQuestCodeOnSectorEntry( 13, 4, 0);
							// test miner quote system
							IssueHeadMinerQuote( (INT8) (1 + Random(MAX_NUMBER_OF_MINES - 1)), (UINT8) (1 + Random(2)));
						}
					#endif
					break;
				case 'r':
					if( gfPreBattleInterfaceActive )
					{ //activate autoresolve in prebattle interface.
						ActivatePreBattleRetreatAction();
					}
					break;
				case 's':
					if( fAlt )
					{
						// although we're not actually going anywhere, we must still be in a state where this is permitted
						if ( AllowedToExitFromMapscreenTo( MAP_EXIT_TO_SAVE ) )
						{
							//if the game CAN be saved
							if( CanGameBeSaved() )
							{
								guiPreviousOptionScreen = guiCurrentScreen;
								DoQuickSave();
							}
							else
							{
								//Display a message saying the player cant save now
								DoMapMessageBox( MSG_BOX_BASIC_STYLE, zNewTacticalMessages[ TCTL_MSG__IRON_MAN_CANT_SAVE_NOW ], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL );
							}
						}
					}
					else if( fCtrl )
					{
						// go to SAVE screen
						gfSaveGame = TRUE;
						RequestTriggerExitFromMapscreen( MAP_EXIT_TO_SAVE );
					}
					break;
				case 't':
					// Teleport: CTRL-T
					if( ( fCtrl )&&( CHEATER_CHEAT_LEVEL( ) ) )
					{
						// check if selected dest char,
						if( ( bSelectedDestChar != -1 ) && ( fPlotForHelicopter == FALSE ) && ( iCurrentMapSectorZ == 0 ) && ( GetMouseMapXY( &sMapX, &sMapY ) ) )
						{
							INT16 sDeltaX, sDeltaY;
							INT16 sPrevX, sPrevY;
							SOLDIERTYPE *pSoldier = MercPtrs[ gCharactersList[ bSelectedDestChar ].usSolID ];

							// can't teleport to where we already are
							if ( ( sMapX == pSoldier->sSectorX ) && ( sMapY == pSoldier->sSectorY ) )
								break;

/*
							if( fZoomFlag == TRUE )
							{
								// convert to zoom out coords from screen coords
								sMapX = ( INT16 )( iZoomX / MAP_GRID_X + sMapX ) / 2;
								sMapY = ( INT16 )( iZoomY / MAP_GRID_Y + sMapY ) / 2;
							}
*/

							// cancel movement plotting
							AbortMovementPlottingMode( );

							// nuke the UI message generated by this
							CancelMapUIMessage( );

							// clear their strategic movement (mercpaths and waypoints)
							ClearMvtForThisSoldierAndGang( pSoldier );

							// select this sector
							ChangeSelectedMapSector( sMapX, sMapY, 0 );

							// check to see if this person is moving, if not...then assign them to mvt group
							if( pSoldier->ubGroupID  == 0 )
							{
								ubGroupId = CreateNewPlayerGroupDepartingFromSector( ( INT8 ) ( pSoldier->sSectorX ) , ( INT8 ) ( pSoldier->sSectorY ) );
								// assign to a group
								AddPlayerToGroup( ubGroupId, pSoldier );
							}

							// figure out where they would've come from
							sDeltaX = sMapX - pSoldier->sSectorX;
							sDeltaY = sMapY - pSoldier->sSectorY;

							if ( abs( sDeltaX ) >= abs( sDeltaY ) )
							{
								// use East or West
								if( sDeltaX > 0 )
								{
									// came in from the West
									sPrevX = sMapX - 1;
									sPrevY = sMapY;
								}
								else
								{
									// came in from the East
									sPrevX = sMapX + 1;
									sPrevY = sMapY;
								}
							}
							else
							{
								// use North or South
								if( sDeltaY > 0 )
								{
									// came in from the North
									sPrevX = sMapX;
									sPrevY = sMapY - 1;
								}
								else
								{
									// came in from the South
									sPrevX = sMapX;
									sPrevY = sMapY + 1;
								}
							}

							// set where they are, were/are going, then make them arrive there and check for battle
							PlaceGroupInSector( pSoldier->ubGroupID, sPrevX, sPrevY, sMapX, sMapY, 0, TRUE );

							// unload the sector they teleported out of
							CheckAndHandleUnloadingOfCurrentWorld();
						}
					}
					else
					{
						// only handle border button keyboard equivalents if the button is visible!
						if ( !fShowMapInventoryPool )
						{
							// Toggle show teams flag
							ToggleShowTeamsMode();
						}
					}
					break;
				case 'u':
					#ifdef JA2TESTVERSION
					{
						if( fAlt )
						{
							UINT32 uiCnt;
							// initialize miners if not already done so (fakes entering Drassen mine first)
							HandleQuestCodeOnSectorEntry( 13, 4, 0);
							// test running out
							for (uiCnt = 0; uiCnt < 10; uiCnt++)
							{
								HandleIncomeFromMines();
							}
						}
					}
					#endif
					break;
				case 'v':
					if( fCtrl )
					{
						#ifdef SGP_VIDEO_DEBUGGING
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"VObjects:  %d", guiVObjectSize );
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"VSurfaces:  %d", guiVSurfaceSize );
							ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"SGPVideoDump.txt updated..." );
							PerformVideoInfoDumpIntoFile( "SGPVideoDump.txt", TRUE );
						#endif
					}
					else
					{
						DisplayGameSettings( );
					}
					break;
				#endif
				case 'w':
					// only handle border button keyboard equivalents if the button is visible!
					if ( !fShowMapInventoryPool )
					{
						// toggle show towns filter
						ToggleShowTownsMode();
					}
					break;
				case 'x':
					if( fAlt )
					{
						HandleShortCutExitState( );
					}
					break;

				case 'y':
					// ALT-Y: toggles SAM sites disable
					if( fAlt )
					{
						#ifdef JA2TESTVERSION
							fSAMSitesDisabledFromAttackingPlayer = !fSAMSitesDisabledFromAttackingPlayer;
						#endif
					}
					break;

				case 'z':
					// only handle border button keyboard equivalents if the button is visible!
					if( fCtrl )
					{
						if( CHEATER_CHEAT_LEVEL( ) )
						{
							if(	gfAutoAIAware ^= 1 )
								ScreenMsg( FONT_WHITE, MSG_TESTVERSION, L"Strategic AI awareness maxed." );
							else
								ScreenMsg( FONT_WHITE, MSG_TESTVERSION, L"Strategic AI awareness normal." );
						}
					}
					else if ( !fShowMapInventoryPool )
					{
						// Toggle Show Militia ON/OFF
						ToggleShowMilitiaMode();
					}
					break;
			}
		}
		else if( InputEvent.usEvent == KEY_REPEAT )
		{
			switch( InputEvent.usParam )
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
static void RemoveTeamPanelSortButtonsForMapScreen(void);


void EndMapScreen( BOOLEAN fDuringFade )
{
	if ( fInMapMode == FALSE )
	{
		// shouldn't be here
		return;
	}

/*
	// exit is called due to message box, leave
	if( fMapExitDueToMessageBox )
	{
		fMapExitDueToMessageBox = FALSE;
		return;
	}
*/

	fLeavingMapScreen = FALSE;

	SetRenderFlags( RENDER_FLAG_FULL );
  //MSYS_EnableRegion( &gViewportRegion );
  //MSYS_EnableRegion( &gRadarRegion );
	// ATE: Shutdown tactical interface panel
//	ShutdownCurrentPanel( );

	if( IsMapScreenHelpTextUp() )
	{
		// stop mapscreen text
		StopMapScreenHelpText( );

		return;
	}


	// still plotting movement?
	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
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

	UnloadButtonImage( giMapContractButtonImage );
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
	ghContractBox = -1;

  CreateDestroyAssignmentPopUpBoxes( );

	// shutdown movement box
	if ( fShowMapScreenMovementList )
	{
		fShowMapScreenMovementList = FALSE;
		CreateDestroyMovementBox( 0, 0, 0 );
	}

	// the remove merc from team box
	RemoveBox( ghRemoveMercAssignBox );
	ghRemoveMercAssignBox = -1;

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

	if( fShowMapInventoryPool == FALSE )
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

	if(fPreLoadedMapGraphics == FALSE )
	{
		DeleteMapBottomGraphics( );

#ifndef JA2DEMO
		DeleteVideoObjectFromIndex(guiSubLevel1);
		DeleteVideoObjectFromIndex(guiSubLevel2);
		DeleteVideoObjectFromIndex(guiSubLevel3);
#endif
		DeleteVideoObjectFromIndex( guiSleepIcon );
		DeleteVideoObjectFromIndex(guiMAPCURSORS);
		//DeleteVideoObjectFromIndex(guiMAPBORDER);
		DeleteVideoObjectFromIndex(guiCHARLIST);
		//DeleteVideoObjectFromIndex(guiCORNERADDONS);
		//DeleteVideoObjectFromIndex(guiMAPCORNER);
		//DeleteVideoObjectFromIndex(guiPOPUPBORDERS);
		DeleteVideoObjectFromIndex(guiCHARINFO);
		DeleteVideoObjectFromIndex(guiCHARICONS);
		DeleteVideoObjectFromIndex(guiCROSS);
#ifndef JA2DEMO
		DeleteVideoSurfaceFromIndex(guiBIGMAP);
	//	DeleteVideoSurfaceFromIndex(guiPOPUPTEX);
		DeleteVideoObjectFromIndex(guiSAMICON);
#endif
		DeleteVideoObjectFromIndex(guiMAPINV);
		DeleteVideoObjectFromIndex(guiMapInvSecondHandBlockout);
		DeleteVideoObjectFromIndex(guiULICONS);
		DeleteVideoObjectFromIndex(guiORTAICON);
		DeleteVideoObjectFromIndex(guiTIXAICON);
		DeleteVideoObjectFromIndex( guiCHARBETWEENSECTORICONS );
		DeleteVideoObjectFromIndex( guiCHARBETWEENSECTORICONSCLOSE );
		DeleteVideoObjectFromIndex( guiLEVELMARKER );
		DeleteVideoObjectFromIndex( guiMapBorderEtaPopUp );

		DeleteVideoObjectFromIndex( guiSecItemHiddenVO );
		DeleteVideoObjectFromIndex( guiSelectedCharArrow );
		DeleteVideoObjectFromIndex( guiMapBorderHeliSectors );
		DeleteVideoObjectFromIndex( guiHelicopterIcon );
		DeleteVideoObjectFromIndex( guiMINEICON );
		DeleteVideoObjectFromIndex( guiSectorLocatorGraphicID );

		DeleteVideoObjectFromIndex( guiBULLSEYE );

		// remove the militia pop up box
		RemoveMilitiaPopUpBox( );

		// remove inventory pool graphic
		RemoveInventoryPoolGraphic( );

		// get rid of border stuff
		DeleteMapBorderGraphics( );

		//Kris:  Remove the email icons.
		DeleteVideoObjectFromIndex( guiNewMailIcons );
	}

	DeleteVideoObjectFromIndex( guiBrownBackgroundForTeamPanel );

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
		#ifndef JA2DEMO
		UINT32 uiLaptopOn;

		//Load a tiny graphic of the on screen and draw it to the buffer.
		PlayJA2SampleFromFile("SOUNDS/Initial Power Up (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
		uiLaptopOn = AddVideoObjectFromFile("INTERFACE/LaptopOn.sti");
		AssertMsg(uiLaptopOn != NO_VOBJECT, "Failed to load data/Interface/LaptopOn.sti");
		BltVideoObjectFromIndex( FRAME_BUFFER, uiLaptopOn, 0, 465, 417);
		InvalidateRegion( 465, 417, 480, 427 );
		ExecuteBaseDirtyRectQueue( );
		EndFrameBufferRender( );
		DeleteVideoObjectFromIndex( uiLaptopOn );
		RefreshScreen();
		#endif
	}


	//Kris:  Removes the pre battle interface, but only if it exists.
	//		   It is internally considered.
	KillPreBattleInterface();

	// cancel request if we somehow leave first
	gfRequestGiveSkyriderNewDestination = FALSE;
}


static BOOLEAN GetMapXY(INT16 sX, INT16 sY, INT16* psMapWorldX, INT16* psMapWorldY);


BOOLEAN GetMouseMapXY( INT16 *psMapWorldX, INT16 *psMapWorldY )
{
	POINT  MousePos;

	if( IsMapScreenHelpTextUp( ) )
	{
		// don't show highlight while global help text is up
		return( FALSE );
	}


	GetCursorPos(&MousePos);

  if(fZoomFlag)
	{
   if(MousePos.x >MAP_GRID_X+MAP_VIEW_START_X)
	  MousePos.x-=MAP_GRID_X;
	 if(MousePos.x >MAP_VIEW_START_X+MAP_VIEW_WIDTH)
		MousePos.x=-1;
	 if(MousePos.y > MAP_GRID_Y+MAP_VIEW_START_Y)
	  MousePos.y-=MAP_GRID_Y;
   if(MousePos.y >MAP_VIEW_START_Y+MAP_VIEW_HEIGHT-11)
		MousePos.y=-11;
   if(MousePos.y < MAP_VIEW_START_Y)
		 MousePos.y=-1;
	}

	return( GetMapXY( (INT16)MousePos.x, (INT16)MousePos.y, psMapWorldX, psMapWorldY ) );
}


static BOOLEAN GetMapXY(INT16 sX, INT16 sY, INT16* psMapWorldX, INT16* psMapWorldY)
{
	INT16 sMapX, sMapY;

	// Subtract start of map view
	sMapX = sX - MAP_VIEW_START_X;//+2*MAP_GRID_X;
	sMapY = sY - MAP_VIEW_START_Y;

	if(!fZoomFlag)
	{
   	if ( sMapX < MAP_GRID_X || sMapY < MAP_GRID_Y )
	{
		return( FALSE );
	}
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

	*psMapWorldX = ( sMapX / MAP_GRID_X );
 	*psMapWorldY = ( sMapY / MAP_GRID_Y );

	return( TRUE );
}


static void RenderMapHighlight(INT16 sMapX, INT16 sMapY, UINT16 usLineColor, BOOLEAN fStationary)
{
#ifndef JA2DEMO
	INT16												sScreenX, sScreenY;
	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;


	Assert( ( sMapX >= 1 ) && ( sMapX <= 16 ) );
	Assert( ( sMapY >= 1 ) && ( sMapY <= 16 ) );

/*
	if((fZoomFlag)&&((sMapX > MAP_WORLD_X-1)||(sMapY> MAP_WORLD_Y-1)))
   return;
*/

	// if we are not allowed to highlight, leave
	if( ( IsTheCursorAllowedToHighLightThisSector( sMapX, sMapY ) == FALSE )&&( fZoomFlag == FALSE ) )
	{
		return;
	}
/*
	else if( ( IsTheCursorAllowedToHighLightThisSector( sMapX , sMapY ) == FALSE )&&( fZoomFlag == TRUE ) && ( fStationary == TRUE ) )
	{
		return;
	}
	else if( ( IsTheCursorAllowedToHighLightThisSector( ( INT16 ) ( ( ( iZoomX ) / ( MAP_GRID_X * 2 ) ) + sMapX / 2 ) ,( INT16 ) ( ( ( iZoomY ) / ( MAP_GRID_Y * 2 ) ) + sMapY / 2 ) ) == FALSE ) && ( fZoomFlag == TRUE ) &&( fStationary == FALSE ) )
  {
		return;
	}
*/


//	if((!fStationary)||(!fZoomFlag))
	{
	  GetScreenXYFromMapXY( sMapX, sMapY, &sScreenX, &sScreenY );
	}
/*
  else
	{
    GetScreenXYFromMapXYStationary( sMapX, sMapY, &sScreenX, &sScreenY );
	}
*/

	// blit in the highlighted sector
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );

	// clip to view region
	ClipBlitsToMapViewRegionForRectangleAndABit( uiDestPitchBYTES );

/*
	if (fZoomFlag)
	{
		// draw rectangle for zoom in
		RectangleDraw( TRUE, sScreenX-MAP_GRID_X,     sScreenY-MAP_GRID_Y - 1, sScreenX +  MAP_GRID_ZOOM_X - MAP_GRID_X, sScreenY +  MAP_GRID_ZOOM_Y - MAP_GRID_Y - 1, usLineColor, pDestBuf );
		InvalidateRegion(    sScreenX-MAP_GRID_X - 3, sScreenY-MAP_GRID_Y - 4, sScreenX + DMAP_GRID_ZOOM_X - MAP_GRID_X, sScreenY + DMAP_GRID_ZOOM_Y - MAP_GRID_Y - 1 );
	}
	else
*/
	{
		// draw rectangle for zoom out
		RectangleDraw( TRUE, sScreenX, sScreenY - 1, sScreenX +  MAP_GRID_X, sScreenY +  MAP_GRID_Y - 1, usLineColor, pDestBuf );
		InvalidateRegion(    sScreenX, sScreenY - 2, sScreenX + DMAP_GRID_X + 1, sScreenY + DMAP_GRID_Y - 1 );
	}

	RestoreClipRegionToFullScreenForRectangle( uiDestPitchBYTES );
	UnLockVideoSurface( FRAME_BUFFER );

#endif
}


static BOOLEAN CheckIfClickOnLastSectorInPath(INT16 sX, INT16 sY);
static void DestinationPlottingCompleted(void);


static void PollLeftButtonInMapView(UINT32* puiNewEvent)
{
	static BOOLEAN	fLBBeenPressedInMapView = FALSE;
  INT16 sMapX, sMapY;


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

				gfAllowSkyriderTooFarQuote = FALSE;
			}
		}
		else	// L-button is NOT down at the moment
		{
			if ( fLBBeenPressedInMapView  )
			{
				fLBBeenPressedInMapView = FALSE;
				RESETCOUNTER( LMOUSECLICK_DELAY_COUNTER );

				// if we are showing help text in mapscreen
				if( fShowMapScreenHelpText )
				{
					fShowMapScreenHelpText = FALSE;
					fCharacterInfoPanelDirty = TRUE;
					fMapPanelDirty = TRUE;
					return;
				}

				// if in militia redistribution popup
				if ( sSelectedMilitiaTown != 0 )
				{
					// ignore clicks outside the box
					return;
				}

				// left click cancels MAP UI messages, unless we're in confirm map move mode
				if( ( giUIMessageOverlay != -1 ) && !gfInConfirmMapMoveMode )
				{
					CancelMapUIMessage( );

					// return unless moving the bullseye
					if ( !gfInChangeArrivalSectorMode )
						return;
				}

				// ignore left clicks in the map screen if:
				// game just started or we're in the prebattle interface or if we are about to hit pre-battle
				if ( ( gTacticalStatus.fDidGameJustStart == TRUE ) ||
						 ( gfPreBattleInterfaceActive == TRUE ) ||
						 ( fDisableMapInterfaceDueToBattle  == TRUE ) )
				{
					return;
				}


				// if in "plot route" mode
				if ( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
				{
					fEndPlotting = FALSE;

					GetMouseMapXY(&sMapX, &sMapY);

/*
					// translate screen values to map grid values for zoomed in
					if(fZoomFlag)
					{
						sMapX=(UINT16)iZoomX/MAP_GRID_X+sMapX;
						sMapX=sMapX/2;
						sMapY=(UINT16)iZoomY/MAP_GRID_Y+sMapY;
						sMapY=sMapY/2;
					}
*/

					// if he clicked on the last sector in his current path
					if( CheckIfClickOnLastSectorInPath( sMapX, sMapY ) )
					{
						DestinationPlottingCompleted();
					}
					else	// clicked on a new sector
					{
						gfAllowSkyriderTooFarQuote = TRUE;

						// draw new map route
						*puiNewEvent = MAP_EVENT_PLOT_PATH;
					}
				}
	      else	// not plotting movement
				{
					// if not plotting a path
					if( ( fEndPlotting == FALSE ) && ( fJustFinishedPlotting == FALSE ) )
					{
						// make this sector selected / trigger movement box / start helicopter plotting / changing arrival sector
						*puiNewEvent = MAP_EVENT_CLICK_SECTOR;
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


static void PollRightButtonInMapView(UINT32* puiNewEvent)
{
	static BOOLEAN	fRBBeenPressedInMapView = FALSE;
  INT16 sMapX, sMapY;


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

				// if we are showing help text in mapscreen
				if( fShowMapScreenHelpText )
				{
					fShowMapScreenHelpText = FALSE;
					fCharacterInfoPanelDirty = TRUE;
					fMapPanelDirty = TRUE;
					return;
				}

				// if in militia redistribution popup
				if ( sSelectedMilitiaTown != 0 )
				{
					// ignore clicks outside the box
					return;
				}

				if ( gfInChangeArrivalSectorMode )
				{
					CancelChangeArrivalSectorMode( );
					MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pBullseyeStrings[ 3 ] );
					return;
				}

				// right click cancels MAP UI messages, unless we're in confirm map move mode
				if( ( giUIMessageOverlay != -1 ) && !gfInConfirmMapMoveMode )
				{
					CancelMapUIMessage( );
					return;
				}

				// ignore right clicks in the map area if:
				// game just started or we're in the prebattle interface or if we are about to hit pre-battle
				if ( ( gTacticalStatus.fDidGameJustStart == TRUE ) ||
						 ( gfPreBattleInterfaceActive == TRUE ) ||
						 ( fDisableMapInterfaceDueToBattle  == TRUE ) )
				{
					return;
				}


				if ( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
				{
					// cancel/shorten the path
					*puiNewEvent = MAP_EVENT_CANCEL_PATH;
				}
				else
				{
					if ( GetMouseMapXY( &sMapX, &sMapY ) )
					{
/*
						if(fZoomFlag)
						{
							sMapX=(UINT16)iZoomX/MAP_GRID_X+sMapX;
							sMapX=sMapX/2;
							sMapY=(UINT16)iZoomY/MAP_GRID_Y+sMapY;
							sMapY=sMapY/2;
						}
*/

						if( ( sSelMapX != sMapX ) || ( sSelMapY != sMapY ) )
						{
							ChangeSelectedMapSector( sMapX, sMapY, ( INT8 )iCurrentMapSectorZ );
						}
					}

					// sector must be selected to bring up militia or town info boxes for it
					if ( ( sMapX == sSelMapX ) && ( sSelMapY == sMapY ) )
					{
						if ( fShowMilitia == TRUE )
						{
							HandleMilitiaRedistributionClick();
						}
						else // show militia is OFF
						{
							// if on the surface, or a real underground sector (we've visited it)
							if ( ( iCurrentMapSectorZ == 0 ) ||
									 ( GetSectorFlagStatus( sMapX, sMapY, ( UINT8 ) iCurrentMapSectorZ, SF_ALREADY_VISITED ) == TRUE ) )
							{
								// toggle sector info for this sector
								fShowTownInfo = !fShowTownInfo;
								fMapPanelDirty = TRUE;
							}
						}

//						fMapScreenBottomDirty = TRUE;

						CreateDestroyScreenMaskForAssignmentAndContractMenus( );
						if( fShowTownInfo == FALSE )
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


/*
void BtnINVCallback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(fMapInventoryItem)
			return;
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
		 fCharacterInfoPanelDirty = TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&= ~(BUTTON_CLICKED_ON);

		 if(!fMapInventoryItem)
		 {
		   fShowInventoryFlag = FALSE;
		 }

		 // set help text for item glow region
		 if( fShowInventoryFlag )
		 {
			 SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 2 ] );
		 }
		 else
		 {
			 SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );
		 }

		 fTeamPanelDirty = TRUE;
		}
	}
}
*/


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
  //giMapInvButtonImage=  LoadButtonImage( "INTERFACE/mapinv.sti" ,-1,1,-1,2,-1 );
  //giMapInvButton= QuickCreateButton( giMapInvButtonImage, INV_BTN_X-1, INV_BTN_Y,
						//				BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
						//				BUTTON_NO_CALLBACK, BtnINVCallback);
  // disable allmouse regions in this space
  fTeamPanelDirty=TRUE;

  InitInvSlotInterface( gMapScreenInvPocketXY, &gSCamoXY, MAPInvMoveCallback, MAPInvClickCallback, MAPInvMoveCamoCallback, MAPInvClickCamoCallback, FALSE );
  MSYS_EnableRegion(&gMPanelRegion);

	// switch hand region help text to "Exit Inventory"
	SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 2 ] );

	// reset inventory item help text
	memset( gubMAP_HandInvDispText, 0, sizeof( gubMAP_HandInvDispText ) );

	// dirty character info panel  ( Why? ARM )
	fCharacterInfoPanelDirty=TRUE;
 }
 else if( !fShowInventoryFlag && fOldShowInventoryFlag )
 {
	 // destroy inventory button
	 ShutdownInvSlotInterface( );
	 fOldShowInventoryFlag=FALSE;
   //RemoveButton( giMapInvButton );
   //UnloadButtonImage( giMapInvButtonImage );
   fTeamPanelDirty=TRUE;
   MSYS_DisableRegion(&gMPanelRegion);

	// switch hand region help text to "Enter Inventory"
	SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );

	// force immediate reblit of item in HANDPOS now that it's not blitted while in inventory mode
	fCharacterInfoPanelDirty=TRUE;
 }
}


static void BltCharInvPanel(void)
{
	UINT32 uiDestPitchBYTES;
	UINT16  *pDestBuf;
	CHAR16 sString[ 32 ];
	UINT16 usX, usY;

	// make sure we're here legally
	Assert( MapCharacterHasAccessibleInventory( bSelectedInfoChar ) );

	const SOLDIERTYPE* pSoldier = GetSoldier(gCharactersList[bSelectedInfoChar].usSolID);

  pDestBuf = (UINT16*)LockVideoSurface( guiSAVEBUFFER, &uiDestPitchBYTES);
	HVOBJECT hCharListHandle = GetVideoObject(guiMAPINV);
  Blt8BPPDataTo16BPPBufferTransparent( pDestBuf, uiDestPitchBYTES, hCharListHandle, PLAYER_INFO_X, PLAYER_INFO_Y, 0);
  UnLockVideoSurface( guiSAVEBUFFER );

  Assert( pSoldier );
  CreateDestroyMapInvButton();

	if( gbCheckForMouseOverItemPos != -1 )
	{
		if( HandleCompatibleAmmoUIForMapScreen( pSoldier, (INT32)gbCheckForMouseOverItemPos, TRUE, TRUE ) == TRUE )
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
	SetFontDestBuffer( guiSAVEBUFFER, 0, 0, 640, 480, FALSE );

	// render items in each of chars slots
  HandleRenderInvSlots( pSoldier, DIRTYLEVEL2 );

	// reset font destination buffer
	SetFontDestBuffer( FRAME_BUFFER, 0,0, 640, 480, FALSE );

 	SetFont( BLOCKFONT2 );

	// Render Values for stats!
	// Set font drawing to saved buffer
	SetFontDestBuffer( guiSAVEBUFFER, 0, 0, 640, 480, FALSE );

	SetFontBackground( FONT_MCOLOR_BLACK );
	SetFontForeground( MAP_INV_STATS_TITLE_FONT_COLOR );

	// print armor/weight/camo labels
  mprintf( MAP_ARMOR_LABEL_X, MAP_ARMOR_LABEL_Y, pInvPanelTitleStrings[ 0 ] );
  mprintf( MAP_ARMOR_PERCENT_X, MAP_ARMOR_PERCENT_Y, L"%%" );

  mprintf( MAP_WEIGHT_LABEL_X, MAP_WEIGHT_LABEL_Y, pInvPanelTitleStrings[ 1 ]  );
  mprintf( MAP_WEIGHT_PERCENT_X, MAP_WEIGHT_PERCENT_Y, L"%%" );

	mprintf( MAP_CAMMO_LABEL_X, MAP_CAMMO_LABEL_Y, pInvPanelTitleStrings[ 2 ]  );
  mprintf( MAP_CAMMO_PERCENT_X, MAP_CAMMO_PERCENT_Y, L"%%" );

	// display armor value
	swprintf( sString, lengthof(sString), L"%3d", ArmourPercent( pSoldier ) );
	FindFontRightCoordinates(MAP_ARMOR_X, MAP_ARMOR_Y, MAP_PERCENT_WIDTH, MAP_PERCENT_HEIGHT, sString, BLOCKFONT2, &usX, &usY);
	mprintf( usX, usY, sString );

	// Display weight value
	swprintf( sString, lengthof(sString), L"%3d", CalculateCarriedWeight( pSoldier ) );
	FindFontRightCoordinates(MAP_WEIGHT_X, MAP_WEIGHT_Y, MAP_PERCENT_WIDTH, MAP_PERCENT_HEIGHT, sString, BLOCKFONT2, &usX, &usY);
	mprintf( usX, usY, sString );

	// Display camo value
	swprintf( sString, lengthof(sString), L"%3d", pSoldier->bCamo );
	FindFontRightCoordinates(MAP_CAMMO_X, MAP_CAMMO_Y, MAP_PERCENT_WIDTH, MAP_PERCENT_HEIGHT, sString, BLOCKFONT2, &usX, &usY);
	mprintf( usX, usY, sString );

	if( InKeyRingPopup( ) )
	{
		// shade the background
		ShadowVideoSurfaceRect( guiSAVEBUFFER , PLAYER_INFO_X, PLAYER_INFO_Y, PLAYER_INFO_X + 261,  PLAYER_INFO_Y + ( 359 - 107 ));
	}
	else
	{
		// blit gold key on top of key ring if key ring is not empty
	}

	SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );
}


// check for and highlight any ammo
static void HandleCursorOverRifleAmmo(void)
{
	if( fShowInventoryFlag == FALSE )
	{
		return;
	}

	if( gbCheckForMouseOverItemPos == -1 )
	{
		return;
	}

	if ( gfCheckForMouseOverItem )
	{
		if ( HandleCompatibleAmmoUI( &Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ], (INT8)gbCheckForMouseOverItemPos, TRUE ) )
		{
			if ( ( GetJA2Clock( ) - guiMouseOverItemTime ) > 100 )
			{
				fTeamPanelDirty = TRUE;
			}
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

	// make sure we're here legally
	Assert( MapCharacterHasAccessibleInventory( bSelectedInfoChar ) );

	const SOLDIERTYPE* pSoldier = GetSoldier(gCharactersList[bSelectedInfoChar].usSolID);

	uiHandPos = MSYS_GetRegionUserData( pRegion, 0 );

	//gbCheckForMouseOverItemPos = -1;

	if ( pSoldier->inv[ uiHandPos ].usItem == NOTHING )
		return;

	if (iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE )
//  if( ( iReason == MSYS_CALLBACK_REASON_MOVE ) || ( iReason == MSYS_CALLBACK_REASON_GAIN_MOUSE ) )
	{
		gubMAP_HandInvDispText[ uiHandPos ] = 2;
		guiMouseOverItemTime = GetJA2Clock( );
		gfCheckForMouseOverItem = TRUE;
		HandleCompatibleAmmoUI( pSoldier, (INT8)uiHandPos, FALSE );
		gbCheckForMouseOverItemPos = (INT8)uiHandPos;
	}
	if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gubMAP_HandInvDispText[ uiHandPos ] = 1;
		HandleCompatibleAmmoUI( pSoldier, (INT8)uiHandPos, FALSE );
		gfCheckForMouseOverItem = FALSE;
		fTeamPanelDirty = TRUE;
	  gbCheckForMouseOverItemPos = -1;
	}
}

// mapscreen wrapper to init the item description box
BOOLEAN MAPInternalInitItemDescriptionBox( OBJECTTYPE *pObject, UINT8 ubStatusIndex, SOLDIERTYPE *pSoldier )
{
	BOOLEAN fRet;

	fRet = InternalInitItemDescriptionBox( pObject, MAP_ITEMDESC_START_X, MAP_ITEMDESC_START_Y, ubStatusIndex, pSoldier );

	fShowDescriptionFlag=TRUE;
	fTeamPanelDirty=TRUE;
	fInterfacePanelDirty = DIRTYLEVEL2;

	return( fRet );
}


static void MAPBeginItemPointer(SOLDIERTYPE* pSoldier, UINT8 ubHandPos);


// this is Map Screen's version of SMInvClickCallback()
static void MAPInvClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT32 uiHandPos;
	UINT16	usOldItemIndex, usNewItemIndex;
	static BOOLEAN	fRightDown = FALSE;

	// make sure we're here legally
	Assert( MapCharacterHasAccessibleInventory( bSelectedInfoChar ) );

	SOLDIERTYPE* pSoldier = GetSoldier(gCharactersList[bSelectedInfoChar].usSolID);

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
			if( ContinuePastBoobyTrapInMapScreen( gpItemPointer, pSoldier ) == FALSE )
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
					// Update mouse cursor
					guiExternVo = GetInterfaceGraphicForItem( &(Item[ gpItemPointer->usItem ]) );
					gusExternVoSubIndex = Item[ gpItemPointer->usItem ].ubGraphicNum;

					MSYS_ChangeRegionCursor( &gMPanelRegion , EXTERN_CURSOR );
					MSYS_SetCurrentCursor( EXTERN_CURSOR );
					fMapInventoryItem=TRUE;
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
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_ITEM_PASSED_TO_MERC ], ShortItemNames[ usNewItemIndex ], pSoldier->name );
				}

			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		// if there is a map UI message being displayed
		if( giUIMessageOverlay != -1 )
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
				InitItemStackPopup( pSoldier, (UINT8)uiHandPos, 0, INV_REGION_Y, 261, 248 );
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


void InternalMAPBeginItemPointer( SOLDIERTYPE *pSoldier )
{
	// If not null return
	if ( gpItemPointer != NULL )
	{
		return;
	}

	// Set global indicator
	gpItemPointer = &gItemPointer;
	gpItemPointerSoldier = pSoldier;

	// Set mouse
	guiExternVo = GetInterfaceGraphicForItem( &(Item[ gpItemPointer->usItem ]) );
	gusExternVoSubIndex = Item[ gpItemPointer->usItem ].ubGraphicNum;

	MSYS_ChangeRegionCursor( &gMPanelRegion , EXTERN_CURSOR );
	MSYS_SetCurrentCursor( EXTERN_CURSOR );
	fMapInventoryItem=TRUE;
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


static void MAPBeginKeyRingItemPointer(SOLDIERTYPE* pSoldier, UINT8 uiKeySlot)
{
	// If not null return
	if ( gpItemPointer != NULL )
	{
		return;
	}

	// Set mouse
	guiExternVo = GetInterfaceGraphicForItem( &(Item[ gpItemPointer->usItem ]) );
	gusExternVoSubIndex = Item[ gpItemPointer->usItem ].ubGraphicNum;

	MSYS_ChangeRegionCursor( &gMPanelRegion , EXTERN_CURSOR );
	MSYS_SetCurrentCursor( EXTERN_CURSOR );
  fMapInventoryItem=TRUE;
	fTeamPanelDirty=TRUE;
}

void MAPEndItemPointer( )
{
	if ( gpItemPointer != NULL )
	{
		gpItemPointer = NULL;
		MSYS_ChangeRegionCursor( &gMPanelRegion , CURSOR_NORMAL );
		MSYS_SetCurrentCursor( CURSOR_NORMAL );
		fMapInventoryItem=FALSE;
		fTeamPanelDirty=TRUE;

		if ( fShowMapInventoryPool )
		{
			HandleButtonStatesWhileMapInventoryActive();
		}

		if ( fShowInventoryFlag && bSelectedInfoChar >= 0 )
		{
			ReevaluateItemHatches( MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ], FALSE );
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
static void RenderAttributeStringsForUpperLeftHandCorner(UINT32 uiBufferToRenderTo)
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;


	if ( ( bSelectedInfoChar != - 1) && ( gCharactersList[ bSelectedInfoChar ].fValid ) )
	{
		pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];
	}

	SetFont( CHAR_FONT );
	SetFontForeground( CHAR_TITLE_FONT_COLOR );
	SetFontBackground( FONT_BLACK );
	SetFontDestBuffer( uiBufferToRenderTo, 0, 0, 640, 480, FALSE );


	// assignment strings
	DrawString( pUpperLeftMapScreenStrings[ 0 ], (UINT16)(220 - StringPixLength( pUpperLeftMapScreenStrings[0], CHAR_FONT)/2), 6, CHAR_FONT);

	// vehicles and robot don't have attributes, contracts, or morale
	if ( ( pSoldier == NULL ) || ( !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && !AM_A_ROBOT( pSoldier ) ) )
	{
		// health
		DrawString(pUpperLeftMapScreenStrings[ 2 ], 87, 80, CHAR_FONT);

		for( iCounter = 0; iCounter < 5; iCounter++ )
		{
			DrawString(pShortAttributeStrings[ iCounter  ], 88, ( INT16 )( 22 + iCounter * 10 ) ,CHAR_FONT );
			DrawString(pShortAttributeStrings[ iCounter + 5 ] ,133, ( INT16 )( 22 + iCounter * 10 ), CHAR_FONT );
		}

		// contract
		//DrawString(pUpperLeftMapScreenStrings[ 1 ], 194, 52,  CHAR_FONT);

		// morale
		DrawString(pUpperLeftMapScreenStrings[ 3 ], 87, 94,  CHAR_FONT);
	}
	else
	{
		// condition
		DrawString(pUpperLeftMapScreenStrings[ 4 ], 96, 80, CHAR_FONT);
	}


	// restore buffer
	SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE );
}


static void DisplayThePotentialPathForCurrentDestinationCharacterForMapScreenInterface(INT16 sMapX, INT16 sMapY)
{
	// simply check if we want to refresh the screen to display path
	static INT8 bOldDestChar = -1;
  static INT16  sPrevMapX, sPrevMapY;
	INT32 iDifference = 0;


	if( bOldDestChar != bSelectedDestChar )
	{
		bOldDestChar = bSelectedDestChar;
	  giPotCharPathBaseTime = GetJA2Clock( );

		sPrevMapX = sMapX;
		sPrevMapY = sMapY;
		fTempPathAlreadyDrawn = FALSE;
		fDrawTempPath = FALSE;

	}

	if( ( sMapX != sPrevMapX) || ( sMapY != sPrevMapY ) )
	{
		giPotCharPathBaseTime = GetJA2Clock( );

		sPrevMapX = sMapX;
		sPrevMapY = sMapY;

		// path was plotted and we moved, re draw map..to clean up mess
		if( fTempPathAlreadyDrawn == TRUE )
		{
			fMapPanelDirty = TRUE;
		}

		fTempPathAlreadyDrawn = FALSE;
		fDrawTempPath = FALSE;
	}

	iDifference = GetJA2Clock( ) - giPotCharPathBaseTime ;

	if( fTempPathAlreadyDrawn == TRUE )
	{
		return;
	}

	if( iDifference > MIN_WAIT_TIME_FOR_TEMP_PATH )
	{
		fDrawTempPath = TRUE;
		giPotCharPathBaseTime = GetJA2Clock( );
		fTempPathAlreadyDrawn = TRUE;
	}
}


static void ChangeMapScreenMaskCursor(UINT16 usCursor);


void SetUpCursorForStrategicMap( void )
{
	if ( gfInChangeArrivalSectorMode == FALSE )
	{
		// check if character is in destination plotting mode
		if( fPlotForHelicopter == FALSE )
		{
			if( bSelectedDestChar == -1 )
			{
				// no plot mode, reset cursor to normal
				ChangeMapScreenMaskCursor( CURSOR_NORMAL );
			}
			else	// yes - by character
			{
				// set cursor based on foot or vehicle
				if( ( Menptr[gCharactersList[ bSelectedDestChar ].usSolID].bAssignment != VEHICLE ) && !( Menptr[gCharactersList[ bSelectedDestChar ].usSolID].uiStatusFlags & SOLDIER_VEHICLE ) )
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
	Assert( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) );

	// make everybody go back to where they were going before this plotting session started
	RestorePreviousPaths();

	// don't need the previous paths any more
	ClearPreviousPaths();

	// clear the character's temporary path (this is the route being constantly updated on the map)
	if( pTempCharacterPath )
	{
		// make sure we're at the beginning
		pTempCharacterPath = MoveToBeginningOfPathList( pTempCharacterPath );
		pTempCharacterPath = ClearStrategicPathList( pTempCharacterPath, 0 );
	}

	// clear the helicopter's temporary path (this is the route being constantly updated on the map)
	if( pTempHelicopterPath )
	{
		// make sure we're at the beginning
		pTempHelicopterPath = MoveToBeginningOfPathList( pTempHelicopterPath );
		pTempHelicopterPath = ClearStrategicPathList( pTempHelicopterPath, 0 );
	}


	EndConfirmMapMoveMode( );

	// cancel destination line highlight
	giDestHighLine = -1;

	// cancel movement mode
	bSelectedDestChar = -1;
	fPlotForHelicopter = FALSE;

	// tell player the route was UNCHANGED
	MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[ 2 ] );


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

		if( fInArea == TRUE )
		{
			fInArea = FALSE;

			// plotting path, clean up
			if( ( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) || ( fDrawTempHeliPath == TRUE ) ) && ( fTempPathAlreadyDrawn == TRUE ) )
			{
				fDrawTempHeliPath = FALSE;
			  fMapPanelDirty = TRUE;
				gfRenderPBInterface = TRUE;

				// clear the temp path
				if( pTempCharacterPath )
				{
					pTempCharacterPath = ClearStrategicPathList( pTempCharacterPath, 0 );
				}
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

	if( fDisableDueToBattleRoster == FALSE )
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
	INT16 sCounter = 0;
	INT16 sYAdd = 0;

	// the info region...is the background for the list itself

	for( sCounter = 0; sCounter < MAX_CHARACTER_COUNT; sCounter++ )
	{
		if( sCounter >= FIRST_VEHICLE )
		{
			sYAdd = 6;
		}
		else
		{
			sYAdd = 0;
		}

#ifdef JA2DEMO
			// name region across the whole width of the team panel at HIGHEST priority, thus overriding the others
		MSYS_DefineRegion( &gTeamListNameRegion[ sCounter ] , NAME_X, ( INT16 )( Y_START + ( sCounter ) * ( Y_SIZE + 2 ) + sYAdd ), 236, ( INT16 )( 145 + ( sCounter + 1 ) * ( Y_SIZE + 2 ) + sYAdd ), MSYS_PRIORITY_HIGHEST,
							 MSYS_NO_CURSOR, TeamListInfoRegionMvtCallBack, TeamListInfoRegionBtnCallBack );
#else
		// name region
		MSYS_DefineRegion( &gTeamListNameRegion[ sCounter ] , NAME_X, ( INT16 )( Y_START + ( sCounter ) * ( Y_SIZE + 2 ) + sYAdd ), NAME_X + NAME_WIDTH, ( INT16 )( 145 + ( sCounter + 1 ) * ( Y_SIZE + 2 ) + sYAdd ), MSYS_PRIORITY_NORMAL,
							 MSYS_NO_CURSOR, TeamListInfoRegionMvtCallBack, TeamListInfoRegionBtnCallBack );
#endif

		// assignment region
		MSYS_DefineRegion( &gTeamListAssignmentRegion[ sCounter ] ,ASSIGN_X , ( INT16 )( Y_START + ( sCounter ) * ( Y_SIZE + 2 ) + sYAdd), ASSIGN_X + ASSIGN_WIDTH, ( INT16 )( 145 + ( sCounter + 1 ) * ( Y_SIZE + 2 ) + sYAdd ), MSYS_PRIORITY_NORMAL + 1,
							 MSYS_NO_CURSOR, TeamListAssignmentRegionMvtCallBack, TeamListAssignmentRegionBtnCallBack );

		// location region (same function as name regions, so uses the same callbacks)
		MSYS_DefineRegion( &gTeamListLocationRegion[ sCounter ] , LOC_X, ( INT16 )( Y_START + ( sCounter ) * ( Y_SIZE + 2 ) + sYAdd ), LOC_X + LOC_WIDTH, ( INT16 )( 145 + ( sCounter + 1 ) * ( Y_SIZE + 2 ) + sYAdd ), MSYS_PRIORITY_NORMAL + 1,
							 MSYS_NO_CURSOR, TeamListInfoRegionMvtCallBack, TeamListInfoRegionBtnCallBack );

		// destination region
		MSYS_DefineRegion( &gTeamListDestinationRegion[ sCounter ] ,DEST_ETA_X , ( INT16 )( Y_START + ( sCounter ) * ( Y_SIZE + 2 ) + sYAdd ), DEST_ETA_X + DEST_ETA_WIDTH, ( INT16 )( 145 + ( sCounter + 1 ) * ( Y_SIZE + 2 ) + sYAdd ), MSYS_PRIORITY_NORMAL + 1,
							 MSYS_NO_CURSOR, TeamListDestinationRegionMvtCallBack, TeamListDestinationRegionBtnCallBack );

		// contract region
		MSYS_DefineRegion( &gTeamListContractRegion[ sCounter ] ,TIME_REMAINING_X , ( INT16 )( Y_START + ( sCounter ) * ( Y_SIZE + 2 ) + sYAdd), TIME_REMAINING_X + TIME_REMAINING_WIDTH, ( INT16 )( 145 + ( sCounter + 1 ) * ( Y_SIZE + 2 ) + sYAdd ), MSYS_PRIORITY_NORMAL + 1,
							 MSYS_NO_CURSOR, TeamListContractRegionMvtCallBack, TeamListContractRegionBtnCallBack );

		// contract region
		MSYS_DefineRegion( &gTeamListSleepRegion[ sCounter ] ,SLEEP_X, ( INT16 )( Y_START + ( sCounter ) * ( Y_SIZE + 2 ) + sYAdd), SLEEP_X + SLEEP_WIDTH, ( INT16 )( 145 + ( sCounter + 1 ) * ( Y_SIZE + 2 ) + sYAdd ), MSYS_PRIORITY_NORMAL + 1,
							 MSYS_NO_CURSOR, TeamListSleepRegionMvtCallBack, TeamListSleepRegionBtnCallBack );


		MSYS_SetRegionUserData(&gTeamListNameRegion[sCounter],0,sCounter);
		MSYS_SetRegionUserData(&gTeamListAssignmentRegion[sCounter],0,sCounter);
		MSYS_SetRegionUserData(&gTeamListSleepRegion[sCounter],0,sCounter);
		MSYS_SetRegionUserData(&gTeamListLocationRegion[sCounter],0,sCounter);
		MSYS_SetRegionUserData(&gTeamListDestinationRegion[sCounter],0,sCounter);
		MSYS_SetRegionUserData(&gTeamListContractRegion[sCounter],0,sCounter);


		// set up help boxes
		SetRegionFastHelpText( &gTeamListNameRegion[sCounter], pMapScreenMouseRegionHelpText[ 0 ] );
		SetRegionFastHelpText( &gTeamListAssignmentRegion[sCounter], pMapScreenMouseRegionHelpText[ 1 ] );
		SetRegionFastHelpText( &gTeamListSleepRegion[sCounter], pMapScreenMouseRegionHelpText[ 5 ] );
		SetRegionFastHelpText( &gTeamListLocationRegion[sCounter], pMapScreenMouseRegionHelpText[ 0 ] );
		SetRegionFastHelpText( &gTeamListDestinationRegion[sCounter], pMapScreenMouseRegionHelpText[ 2 ] );
		SetRegionFastHelpText( &gTeamListContractRegion[sCounter], pMapScreenMouseRegionHelpText[ 3 ] );
	}
}


static void DestroyMouseRegionsForTeamList(void)
{
	// will destroy mouse regions overlaying the team list area
	INT32 sCounter = 0;

	for( sCounter = 0; sCounter < MAX_CHARACTER_COUNT; sCounter++ )
	{
	  MSYS_RemoveRegion( &gTeamListNameRegion[ sCounter ]);
		MSYS_RemoveRegion( &gTeamListAssignmentRegion[ sCounter ]);
		MSYS_RemoveRegion( &gTeamListSleepRegion[ sCounter ]);
		MSYS_RemoveRegion( &gTeamListDestinationRegion[ sCounter ]);
		MSYS_RemoveRegion( &gTeamListLocationRegion[ sCounter ]);
		MSYS_RemoveRegion( &gTeamListContractRegion[ sCounter ]);
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
	if (iDialogueBox != -1) return;

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		if (IsMapScreenHelpTextUp()) StopMapScreenHelpText();

#if 0 // XXX was commented out
		if (bSelectedDestChar != -1 || fPlotForHelicopter == TRUE)
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
		if (IsMapScreenHelpTextUp()) StopMapScreenHelpText();
		RequestContractMenu();
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		if (IsMapScreenHelpTextUp()) StopMapScreenHelpText( );
	}
}


static BOOLEAN HandleCtrlOrShiftInTeamPanel(INT8 bCharNumber);


static void TeamListInfoRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;
	SOLDIERTYPE *pSoldier = NULL;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set to new info character...make sure is valid
		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			ChangeSelectedInfoChar( ( INT8 ) iValue, TRUE );

			pSoldier = &Menptr[ gCharactersList[ iValue ].usSolID ];

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
				ChangeSelectedMapSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ );
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

#ifndef JA2DEMO

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			pSoldier = &Menptr[ gCharactersList[ iValue ].usSolID ];

			// select this character
			ChangeSelectedInfoChar( ( INT8 ) iValue, TRUE );


			RequestToggleMercInventoryPanel();


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
				ChangeSelectedMapSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ );
			}

			// unhilight contract line
			giContractHighLine = -1;

			// can't assign highlight line
			giAssignHighLine = -1;

			// dirty team and map regions
			fTeamPanelDirty = TRUE;
			fMapPanelDirty = TRUE;
//			fMapScreenBottomDirty = TRUE;
			gfRenderPBInterface = TRUE;
		}
	}
#endif
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
		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			giHighLine = iValue;
		}
		else
		{
			giHighLine = -1;
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		giHighLine = -1;
	}
}


static void TeamListAssignmentRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;
	SOLDIERTYPE *pSoldier = NULL;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set to new info character...make sure is valid
		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			// reset list if the clicked character isn't also selected
			ChangeSelectedInfoChar( ( INT8 ) iValue, ( BOOLEAN )( IsEntryInSelectedListSet( ( INT8 ) iValue ) == FALSE ) );

			pSoldier = &Menptr[ gCharactersList[ iValue ].usSolID ];

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

				if( ( pSoldier->bLife == 0 ) || ( pSoldier->bAssignment == ASSIGNMENT_POW ) )
				{
					fShowRemoveMenu = TRUE;
				}
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
		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			giHighLine = iValue;

			if( !( Menptr[ gCharactersList[ iValue ].usSolID ].uiStatusFlags & SOLDIER_VEHICLE ) )
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
		if( ( gCharactersList[ iValue ].fValid == TRUE ) &&
		   !( Menptr[ gCharactersList[ iValue ].usSolID ].uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			// play click
		  PlayGlowRegionSound( );
		}
	}
}


static BOOLEAN CanChangeDestinationForCharSlot(INT8 bCharNumber, BOOLEAN fShowErrorMessage);
static void MakeMapModesSuitableForDestPlotting(INT8 bCharNumber);


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
		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			// reset list if the clicked character isn't also selected
			ChangeSelectedInfoChar( ( INT8 ) iValue, ( BOOLEAN )( IsEntryInSelectedListSet( ( INT8 ) iValue ) == FALSE ) );

			// deselect any characters/vehicles that can't accompany the clicked merc
			DeselectSelectedListMercsWhoCantMoveWithThisGuy( &( Menptr[ gCharactersList[ iValue ].usSolID ] ) );

			// select all characters/vehicles that MUST accompany the clicked merc (same squad/vehicle)
			SelectUnselectedMercsWhoMustMoveWithThisGuy( );

			// Find out if this guy and everyone travelling with him is allowed to move strategically
			// NOTE: errors are reported within...
			if ( CanChangeDestinationForCharSlot( ( INT8 ) iValue, TRUE ) )
			{
				// turn off sector inventory, turn on show teams filter, etc.
				MakeMapModesSuitableForDestPlotting( ( INT8 ) iValue );

				// check if person is in a vehicle
				if( Menptr[gCharactersList[ iValue ].usSolID].bAssignment == VEHICLE )
				{
					// if he's in the helicopter
					if( Menptr[gCharactersList[ iValue ].usSolID].iVehicleId == iHelicopterVehicleId )
					{
						TurnOnAirSpaceMode( );
						if( RequestGiveSkyriderNewDestination( ) == FALSE )
						{
							// not allowed to change destination of the helicopter
							return;
						}
					}
				}


				// select this character as the one plotting strategic movement
				bSelectedDestChar = ( INT8 )iValue;

				// remember the current paths for all selected characters so we can restore them if need be
				RememberPreviousPathForAllSelectedChars();

				// check each person in this mvt group, if any bleeding, have them complain
				//CheckMembersOfMvtGroupAndComplainAboutBleeding( &( Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ] ) );

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
		MakeMapModesSuitableForDestPlotting( ( INT8 ) iValue );

		// reset list if the clicked character isn't also selected
		ChangeSelectedInfoChar( ( INT8 ) iValue, ( BOOLEAN )( IsEntryInSelectedListSet( ( INT8 ) iValue ) == FALSE ) );

		CancelPathsOfAllSelectedCharacters();

		// reset selected characters
		ResetAllSelectedCharacterModes( );
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
		if( gCharactersList[ iValue ].fValid == TRUE )
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
		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			// play click
			PlayGlowRegionSound( );
		}
	}
}


static void TeamListSleepRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;
	SOLDIERTYPE *pSoldier = NULL;


	if( fLockOutMapScreenInterface || gfPreBattleInterfaceActive )
	{
		return;
	}

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set to new info character...make sure is valid.. not in transit and alive and concious

		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		if( ( gCharactersList[ iValue ].fValid == TRUE ) )
		{
			if ( HandleCtrlOrShiftInTeamPanel( ( INT8 ) iValue ) )
			{
				return;
			}

			// reset list if the clicked character isn't also selected
			ChangeSelectedInfoChar( ( INT8 ) iValue, ( BOOLEAN )( IsEntryInSelectedListSet( ( INT8 ) iValue ) == FALSE ) );

			// if this slot's sleep status can be changed
			if ( CanChangeSleepStatusForCharSlot( (INT8) iValue ) )
			{
				pSoldier = &Menptr[ gCharactersList[ iValue ].usSolID ];

				if( pSoldier->fMercAsleep == TRUE )
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
					if( SetMercAsleep( pSoldier, TRUE ) )
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
		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			giHighLine = iValue;

			if ( CanChangeSleepStatusForCharSlot( (INT8) iValue ) )
			{
				giSleepHighLine = iValue;
			}
			else
			{
				giSleepHighLine = -1;
			}
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
		if ( CanChangeSleepStatusForCharSlot( (INT8) iValue ) )
		{
			// play click
			PlayGlowRegionSound( );
		}
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

	if( gCharactersList[ iValue ].fValid == TRUE )
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
		if( gCharactersList[ iValue ].fValid == TRUE )
		{
			giHighLine = iValue;

			if( CanExtendContractForCharSlot( (INT8) iValue ) )
			{
				giContractHighLine = iValue;
			}
			else
			{
				giContractHighLine = -1;
			}
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
		if( fShowContractMenu == FALSE )
		{
			giContractHighLine  = -1;
		}

		// restore background
		RestoreBackgroundForContractGlowRegionList( );
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if( CanExtendContractForCharSlot( (INT8) iValue ) )
		{
			// play click
			PlayGlowRegionSound( );
		}
	}
}


// mouse position test
static BOOLEAN IsCursorWithInRegion(INT16 sLeft, INT16 sRight, INT16 sTop, INT16 sBottom)
{
	POINT MousePos;

	// get cursor position
	GetCursorPos(&MousePos);

	// is it within region?

	if( ( sLeft < MousePos.x ) && ( sRight > MousePos.x) && (sTop < MousePos.y ) && ( sBottom > MousePos.y ) )
	{
		return ( TRUE );
	}
	else
	{
		return ( FALSE );
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
	if( fPlotForHelicopter == TRUE )
	{
		DisplayHelicopterPath( );
	}
	else if( bSelectedDestChar != -1 )
	{
	  DisplaySoldierPath( &Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ] );
	}
}


static void PlotTemporaryPaths(void)
{
	INT16 sMapX, sMapY;


	// check to see if we have in fact moved are are plotting a path?
	if ( GetMouseMapXY( &sMapX, &sMapY ) )
	{
		if ( fPlotForHelicopter == TRUE )
		{
			Assert( fShowAircraftFlag == TRUE );
/*
			if( fZoomFlag )
			{
				sMapX =  ( INT16 )( ( ( iZoomX ) / ( WORLD_MAP_X ) ) + sMapX );
				sMapX /= 2;

				sMapY =  ( INT16 )( ( ( iZoomY ) / ( WORLD_MAP_X ) ) + sMapY );
				sMapY /= 2;
			}
*/

			// plot temp path
			PlotATemporaryPathForHelicopter( sMapX, sMapY);

			// check if potential path is allowed
			DisplayThePotentialPathForHelicopter( sMapX, sMapY );

			if( fDrawTempHeliPath == TRUE )
			{
				// clip region
				ClipBlitsToMapViewRegion( );
				// display heli temp path
				DisplayHelicopterTempPath( );
				//restore
				RestoreClipRegionToFullScreen( );
			}
		}
		else
		// dest char has been selected,
		if( bSelectedDestChar != -1 )
		{
/*
			if( fZoomFlag )
			{
				sMapX =  ( INT16 )( ( ( iZoomX ) / ( MAP_GRID_X ) ) + sMapX );
				sMapX /= 2;

				sMapY =  ( INT16 )( ( ( iZoomY ) / ( MAP_GRID_Y ) ) + sMapY );
				sMapY /= 2;
			}
*/

			PlotATemporaryPathForCharacter( &Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ], sMapX, sMapY );

			// check to see if we are drawing path
		  DisplayThePotentialPathForCurrentDestinationCharacterForMapScreenInterface( sMapX, sMapY );

			// if we need to draw path, do it
			if( fDrawTempPath == TRUE )
			{
				// clip region
				ClipBlitsToMapViewRegion( );
				// blit
				DisplaySoldierTempPath( &Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ] );
				// restore
				RestoreClipRegionToFullScreen( );
			}
		}
	}
}



void RenderMapRegionBackground( void )
{
	// renders to save buffer when dirty flag set

	if( fMapPanelDirty == FALSE )
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

	if ( ghAttributeBox != - 1 )
	{
		ForceUpDateOfBox( ghAttributeBox );
	}

	if ( ghTownMineBox != - 1 )
	{
		// force update of town mine info boxes
		ForceUpDateOfBox( ghTownMineBox );
	}


	MapscreenMarkButtonsDirty();

	RestoreExternBackgroundRect( 261, 0, 640 - 261, 359 );

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


static void RenderTeamRegionBackground(void)
{
	// renders to save buffer when dirty flag set
	if( fTeamPanelDirty == FALSE )
	{
		// not dirty, leave
		return;
	}

	// show inventory or the team list?
	if(fShowInventoryFlag == FALSE )
	{
		BltVideoObjectFromIndex(guiSAVEBUFFER , guiCHARLIST, 0, PLAYER_INFO_X, PLAYER_INFO_Y);
	}
	else
	{
		BltCharInvPanel();
	}


	if ( !fShowInventoryFlag )
	{
		// if we are not in inventory mode, show character list
		HandleHighLightingOfLinesInTeamPanel( );

		DisplayCharacterList();
	}

	fDrawCharacterList = FALSE;


	// display arrows by selected people
	HandleDisplayOfSelectedMercArrows( );
  DisplayIconsForMercsAsleep(  );


	// reset dirty flag
	fTeamPanelDirty = FALSE;
	gfRenderPBInterface = TRUE;

	// mark all pop ups as dirty
	MarkAllBoxesAsAltered( );

	// restore background for area
	RestoreExternBackgroundRect( 0, 107, 261 - 0, 359 - 107 );

#ifdef JA2DEMO
	if( ( fShowingMapDisableBox == FALSE ) && ( IsMapScreenHelpTextUp() == FALSE ) )
	{
	//	fShowingMapDisableBox = TRUE;
		// restore background for area

		//DoMapMessageBox( MSG_BOX_BASIC_STYLE, pMapErrorString[ 14 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDemoOkBoxCallback );
	}
#endif

	MapscreenMarkButtonsDirty();
}


static void RenderCharacterInfoBackground(void)
{
	// will render the background for the character info panel

	if( fCharacterInfoPanelDirty == FALSE )
	{
		// not dirty, leave
		return;
	}


	// the upleft hand corner character info panel
	BltVideoObjectFromIndex(guiSAVEBUFFER , guiCHARINFO, 0, TOWN_INFO_X, TOWN_INFO_Y);

	UpdateHelpTextForMapScreenMercIcons( );

	if( ( bSelectedInfoChar != -1) && ( fDisableDueToBattleRoster == FALSE ) )
	{
		// valid char to display
		DisplayCharacterInfo();
	}

	if( ( fDisableDueToBattleRoster == FALSE ) )
	{
		// draw attributes
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
	RestoreExternBackgroundRect( 0, 0, 261, 107 );

}


static void HandleShadingOfLinesForContractMenu(void);


static void DetermineIfContractMenuCanBeShown(void)
{
	if( fShowContractMenu == FALSE )
	{

		// destroy menus for contract region
		CreateDestroyMouseRegionsForContractMenu( );

			// hide all boxes
		HideBox( ghContractBox );

		// make sure, absolutly sure we want to hide this box
		if( fShowAssignmentMenu == FALSE )
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

	if( Menptr[gCharactersList[ bSelectedInfoChar ].usSolID].bLife == 0 )
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

	if( fShowAircraftFlag == TRUE )
	{
		// if plotting, but not for heli
		if ( ( bSelectedDestChar != -1 ) && ( fPlotForHelicopter == FALSE ) )
		{
			// abort
			AbortMovementPlottingMode();
		}
	}
	else	// not in airspace mode
	{
		if( fPlotForHelicopter == TRUE )
		{
			// abort
			AbortMovementPlottingMode();
		}
	}
}


static void ContractRegionBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	SOLDIERTYPE *pSoldier = NULL;


	// btn callback handler for contract region

	if( ( iDialogueBox != -1 ) )
	{
		return;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{

		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		if( CanExtendContractForCharSlot( bSelectedInfoChar ) )
		{
			pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];

			// create
			RebuildContractBoxForMerc( pSoldier );

			// reset selected characters
			ResetAllSelectedCharacterModes( );

			bSelectedContractChar = bSelectedInfoChar;
			giContractHighLine = bSelectedContractChar;


			// if not triggered internally
			if ( CheckIfSalaryIncreasedAndSayQuote( pSoldier, TRUE ) == FALSE )
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


static void ContractRegionMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// mvt callback handler for contract region
	if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		if( fGlowContractRegion == TRUE )
		{
			// not showing box and lost mouse?..stop glowing
			if( fShowContractMenu == FALSE )
			{
		    fGlowContractRegion = FALSE;
			  fCharacterInfoPanelDirty = TRUE;
			  giContractHighLine = -1;

			  // reset glow
			  fResetContractGlow = TRUE;
			}
		}

	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if( bSelectedInfoChar != -1 )
		{
			if( gCharactersList[ bSelectedInfoChar ].fValid == TRUE )
			{
				if( fShowContractMenu == FALSE )
				{
					// glow region
					fGlowContractRegion = TRUE;

					giContractHighLine = bSelectedInfoChar;

					PlayGlowRegionSound( );
				}
			}
		}
	}
}


static void HandleShadingOfLinesForContractMenu(void)
{
	SOLDIERTYPE *pSoldier;
	MERCPROFILESTRUCT *pProfile;


	if( ( fShowContractMenu == FALSE ) || ( ghContractBox == - 1 ) )
	{
		return;
	}

	// error check, return if not a valid character
	if( bSelectedContractChar == -1 )
	{
		return;
	}

	// check if this character is a valid character
	if( gCharactersList[ bSelectedContractChar ].fValid == FALSE )
	{
		return;
	}

	Assert( CanExtendContractForCharSlot( bSelectedContractChar ) );

	// grab the character
	pSoldier = &Menptr[ gCharactersList[ bSelectedContractChar ].usSolID ];


	// is guy in AIM? and well enough to talk and make such decisions?
	if( ( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC ) && ( pSoldier->bLife >= OKLIFE ) )
	{
		pProfile = &( gMercProfiles[ pSoldier->ubProfile ] );

		// one day
		if( pProfile->sSalary > LaptopSaveInfo.iCurrentBalance )
		{
			ShadeStringInBox( ghContractBox, CONTRACT_MENU_DAY );
		}
		else
		{
			UnShadeStringInBox( ghContractBox, CONTRACT_MENU_DAY );
		}

		// one week
		if( ( INT32 )( pProfile->uiWeeklySalary ) > LaptopSaveInfo.iCurrentBalance )
		{
			ShadeStringInBox( ghContractBox, CONTRACT_MENU_WEEK );
		}
		else
		{
			UnShadeStringInBox( ghContractBox, CONTRACT_MENU_WEEK );
		}

		// two weeks
		if( ( INT32 )( pProfile->uiBiWeeklySalary ) > LaptopSaveInfo.iCurrentBalance )
		{
			ShadeStringInBox( ghContractBox, CONTRACT_MENU_TWO_WEEKS );
		}
		else
		{
			UnShadeStringInBox( ghContractBox, CONTRACT_MENU_TWO_WEEKS );
		}
	}
	else
	{
		// can't extend contract duration
		ShadeStringInBox( ghContractBox, CONTRACT_MENU_DAY );
		ShadeStringInBox( ghContractBox, CONTRACT_MENU_WEEK );
		ShadeStringInBox( ghContractBox, CONTRACT_MENU_TWO_WEEKS );
	}

	// if THIS soldier is involved in a fight (dismissing in a hostile sector IS ok...)
	if( ( gTacticalStatus.uiFlags & INCOMBAT ) && pSoldier->bInSector )
	{
		ShadeStringInBox( ghContractBox, CONTRACT_MENU_TERMINATE );
	}
	else
	{
		UnShadeStringInBox( ghContractBox, CONTRACT_MENU_TERMINATE );
	}
}


static void SortListOfMercsInTeamPanel(BOOLEAN fRetainSelectedMercs);


void ReBuildCharactersList( void )
{
	// rebuild character's list
	INT16 sCount = 0;


	// add in characters
  for ( sCount = 0; sCount < MAX_CHARACTER_COUNT; sCount++)
  {
		// clear this slot
		gCharactersList[ sCount ].fValid = FALSE;
		gCharactersList[ sCount ].usSolID = 0;
	}

	for ( sCount = 0; sCount < MAX_CHARACTER_COUNT; sCount++)
	{
	  // add character into the cleared slot
	  AddCharacter( &Menptr[ gTacticalStatus.Team[ OUR_TEAM ].bFirstID + sCount ] );
	}


	// sort them according to current sorting method
	SortListOfMercsInTeamPanel( FALSE );


	// if nobody is selected, or the selected merc has somehow become invalid
	if ( ( bSelectedInfoChar == -1 ) || ( !gCharactersList[ bSelectedInfoChar ].fValid ) )
	{
		// is the first character in the list valid?
		if ( gCharactersList[ 0 ].fValid )
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
	// switch hand region help text to "Enter Inventory"
	SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );
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
			if( gCharactersList[ bOldInfoChar ].fValid == TRUE )
			{
				// set face in active
				SetAutoFaceInActiveFromSoldier( Menptr[ gCharactersList[ bOldInfoChar ].usSolID ].ubID );
			}
		}

		// stop showing contract box
		//fShowContractMenu = FALSE;

		// update old info char value
		bOldInfoChar = bSelectedInfoChar;
	}
}


void RebuildContractBoxForMerc( SOLDIERTYPE *pCharacter )
{
	// rebuild contractbox for this merc
	RemoveBox( ghContractBox );
	ghContractBox = -1;

	// recreate
	CreateContractBox( pCharacter );
}


static void EnableDisableTeamListRegionsAndHelpText(void)
{
	// check if valid character here, if so, then do nothing..other wise set help text timer to a gazillion
	INT8 bCharNum;


	for( bCharNum = 0; bCharNum < MAX_CHARACTER_COUNT; bCharNum++ )
	{
		if( gCharactersList[ bCharNum ].fValid == FALSE )
		{
			// disable regions in all team list columns
			MSYS_DisableRegion( &gTeamListNameRegion[ bCharNum ] );
			MSYS_DisableRegion( &gTeamListAssignmentRegion[ bCharNum ] );
			MSYS_DisableRegion( &gTeamListLocationRegion[ bCharNum ] );
			MSYS_DisableRegion( &gTeamListSleepRegion[ bCharNum ] );
			MSYS_DisableRegion( &gTeamListDestinationRegion[ bCharNum ] );
			MSYS_DisableRegion( &gTeamListContractRegion[ bCharNum ] );
		}
		else
		{
			// always enable Name and Location regions
			MSYS_EnableRegion( &gTeamListNameRegion[ bCharNum ] );
			MSYS_EnableRegion( &gTeamListLocationRegion[ bCharNum ] );

			// valid character.  If it's a vehicle, however
			if ( Menptr[ gCharactersList[ bCharNum ].usSolID ].uiStatusFlags & SOLDIER_VEHICLE )
			{
				// Can't change assignment for vehicles
				MSYS_DisableRegion( &gTeamListAssignmentRegion[ bCharNum ] );
			}
			else
			{
				MSYS_EnableRegion( &gTeamListAssignmentRegion[ bCharNum ] );

				// POW or dead ?
				if ( ( Menptr[ gCharactersList[ bCharNum ].usSolID ].bAssignment == ASSIGNMENT_POW ) ||
						 ( Menptr[ gCharactersList[ bCharNum ].usSolID ].bLife == 0 ) )
				{
					// "Remove Merc"
					SetRegionFastHelpText( &gTeamListAssignmentRegion[ bCharNum ], pRemoveMercStrings[ 0 ] );

					SetRegionFastHelpText( &gTeamListDestinationRegion[ bCharNum ], L"" );
				}
				else
				{
					// "Assign Merc"
					SetRegionFastHelpText( &gTeamListAssignmentRegion[ bCharNum ], pMapScreenMouseRegionHelpText[ 1 ] );
					// "Plot Travel Route"
					SetRegionFastHelpText( &gTeamListDestinationRegion[ bCharNum ], pMapScreenMouseRegionHelpText[ 2 ] );
				}
			}


			if ( CanExtendContractForCharSlot( bCharNum ) )
			{
				MSYS_EnableRegion( &gTeamListContractRegion[ bCharNum ] );
			}
			else
			{
				MSYS_DisableRegion( &gTeamListContractRegion[ bCharNum ] );
			}


			if ( CanChangeSleepStatusForCharSlot( bCharNum ) )
			{
				MSYS_EnableRegion( &gTeamListSleepRegion[ bCharNum ] );
			}
			else
			{
				MSYS_DisableRegion( &gTeamListSleepRegion[ bCharNum ] );
			}

			// destination region is always enabled for all valid character slots.
			// if the character can't move at this time, then the region handler must be able to tell the player why not
			MSYS_EnableRegion( &gTeamListDestinationRegion[ bCharNum ] );
		}
	}
}


static void ResetAllSelectedCharacterModes(void)
{
	if( IsMapScreenHelpTextUp() )
	{
		// stop mapscreen text
		StopMapScreenHelpText( );
		return;
	}

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
	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
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


BOOLEAN ContinueDialogue(SOLDIERTYPE *pSoldier, BOOLEAN fDone )
{
	// continue this grunts dialogue, restore when done
	static INT8 bOldSelectedInfoChar = -1;
	static BOOLEAN fTalkingingGuy = FALSE;

	INT8 bCounter = 0;




	if( fDone == TRUE )
	{
		if( fTalkingingGuy == TRUE )
		{
			/*
			// done, restore
			if( bOldSelectedInfoChar != -1 )
			{
				ChangeSelectedInfoChar( bOldSelectedInfoChar, TRUE );

				SetAutoFaceInActive( MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ]->iFaceIndex );
			}

	*/
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
		if( gCharactersList[bCounter].fValid == TRUE )
		{
			if( ( &Menptr[gCharactersList[bCounter].usSolID]) == pSoldier )
			{
				if( bSelectedInfoChar != bCounter )
				{
					bOldSelectedInfoChar = bSelectedInfoChar;
					ChangeSelectedInfoChar( bCounter, TRUE );
				}
				fTalkingingGuy = TRUE;
				return ( FALSE );
			}
		}
	}

	return ( FALSE );
}


static void HandleSpontanousTalking(void)
{
	// simply polls if the talking guy is done, if so...send an end command to continue dialogue

	if( DialogueActive() == FALSE )
	{
		if( ( bSelectedInfoChar != -1 ) && ( bSelectedInfoChar < MAX_CHARACTER_COUNT ) )
		{
			ContinueDialogue( ( &Menptr[gCharactersList[bSelectedInfoChar].usSolID] ), TRUE );
		}
	}
}


static void HandleNewDestConfirmation(INT16 sMapX, INT16 sMapY);
static void RebuildWayPointsForAllSelectedCharsGroups(void);


static BOOLEAN CheckIfClickOnLastSectorInPath(INT16 sX, INT16 sY)
{
	PathStPtr *ppMovePath = NULL;
	BOOLEAN fLastSectorInPath = FALSE;
	INT32 iVehicleId = -1;
	PathStPtr pPreviousMercPath = NULL;


	// see if we have clicked on the last sector in the characters path

	// check if helicopter
	if( fPlotForHelicopter == TRUE )
	{
		if( sX + ( sY * MAP_WORLD_X ) == GetLastSectorOfHelicoptersPath( ) )
		{
			// helicopter route confirmed - take off
			TakeOffHelicopter( );

			// rebuild waypoints - helicopter
			ppMovePath = &( pVehicleList[ iHelicopterVehicleId ].pMercPath );
			RebuildWayPointsForGroupPath( *ppMovePath, pVehicleList[ iHelicopterVehicleId ].ubMovementGroup );

			// pointer to previous helicopter path
			pPreviousMercPath = gpHelicopterPreviousMercPath;

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

		// invalid soldier?  we shouldn't be here!
		if( gCharactersList[ bSelectedDestChar ].fValid == FALSE )
		{
			bSelectedDestChar = -1;
			return( FALSE );
		}

		if( sX + ( sY * MAP_WORLD_X ) == GetLastSectorIdInCharactersPath( ( &Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ] ) ) )
		{
			// clicked on last sector, reset plotting mode

			// if he's IN a vehicle or IS a vehicle
			if( ( Menptr[gCharactersList[ bSelectedDestChar ].usSolID].bAssignment == VEHICLE ) || ( Menptr[gCharactersList[ bSelectedDestChar ].usSolID].uiStatusFlags & SOLDIER_VEHICLE ) )
			{
				if( Menptr[gCharactersList[ bSelectedDestChar ].usSolID].bAssignment == VEHICLE )
				{
					// IN a vehicle
					iVehicleId = Menptr[gCharactersList[ bSelectedDestChar ].usSolID].iVehicleId;
				}
				else
				{
					// IS a vehicle
					iVehicleId = Menptr[gCharactersList[ bSelectedDestChar ].usSolID].bVehicleID;
				}

				// rebuild waypoints - vehicles
				ppMovePath = &( pVehicleList[ iVehicleId ].pMercPath );
			}
			else
			{
				// rebuild waypoints - mercs on foot
				ppMovePath = &( Menptr[gCharactersList[ bSelectedDestChar ].usSolID].pMercPath );
			}

			RebuildWayPointsForAllSelectedCharsGroups( );

			// pointer to previous character path
			pPreviousMercPath = gpCharacterPreviousMercPath[ bSelectedDestChar ];

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
			HandleNewDestConfirmation( sX, sY );
		}
		else	// NULL path confirmed
		{
			// if previously there was a path
			if ( pPreviousMercPath != NULL )
			{
				// then this means we've CANCELED it
				MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[ 3 ] );
			}
			else	// no previous path
			{
				// then it means the route was UNCHANGED
				MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[ 2 ] );
			}
		}
	}


	return( fLastSectorInPath );
}


// rebuild waypoints for selected character list
static void RebuildWayPointsForAllSelectedCharsGroups(void)
{
	// rebuild the waypoints for everyone in the selected character list
	INT32 iCounter = 0;
	BOOLEAN fGroupIDRebuilt[ 256 ];
	SOLDIERTYPE *pSoldier = NULL;
	INT32 iVehicleId;
	PathStPtr *ppMovePath = NULL;
	UINT8 ubGroupId;


	memset( fGroupIDRebuilt, FALSE, sizeof( fGroupIDRebuilt ) );

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( ( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE ) )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];


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
				ppMovePath = &( pVehicleList[ iVehicleId ].pMercPath );
				ubGroupId = pVehicleList[ iVehicleId ].ubMovementGroup;
			}
			else
			{
				// mercs on foot
				ppMovePath = &( Menptr[gCharactersList[ bSelectedDestChar ].usSolID].pMercPath );
				ubGroupId = pSoldier->ubGroupID;
			}

			// if we haven't already rebuilt this group
			if ( !fGroupIDRebuilt[ ubGroupId ] )
			{
				// rebuild it now
				RebuildWayPointsForGroupPath( *ppMovePath, ubGroupId );

				// mark it as rebuilt
				fGroupIDRebuilt[ ubGroupId ] = TRUE;
			}
		}
	}
}


// check if cursor needs to be set to checkmark or to the walking guy?
static void UpdateCursorIfInLastSector(void)
{

	INT16 sMapX = 0, sMapY = 0;

	// check to see if we are plotting a path, if so, see if we are highlighting the last sector int he path, if so, change the cursor
	if ( ( bSelectedDestChar != -1) || ( fPlotForHelicopter == TRUE ) )
	{
		GetMouseMapXY(&sMapX, &sMapY);

		// translate screen values to map grid values for zoomed in
		if(fZoomFlag)
		{
			sMapX=(UINT16)iZoomX/MAP_GRID_X+sMapX;
			sMapX=sMapX/2;
			sMapY=(UINT16)iZoomY/MAP_GRID_Y+sMapY;
			sMapY=sMapY/2;
		}

		if( fShowAircraftFlag == FALSE )
		{
			if( bSelectedDestChar != -1 )
			{
				//c heck if we are in the last sector of the characters path?
				if( sMapX + ( sMapY * MAP_WORLD_X ) == GetLastSectorIdInCharactersPath( ( &Menptr[ gCharactersList[ bSelectedDestChar ].usSolID ] ) ) )
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
				if( sMapX + ( sMapY * MAP_WORLD_X ) == GetLastSectorOfHelicoptersPath( ) )
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
	if( bSelectedInfoChar == -1 )
	{
		return;
	}
	else if( gCharactersList[ bSelectedInfoChar ].fValid == FALSE )
	{
		return;
	}

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		if( gfPreBattleInterfaceActive == TRUE )
		{
			return;
		}

		// now stop any dialogue in progress
		StopAnyCurrentlyTalkingSpeech( );
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		RequestToggleMercInventoryPanel();
	}
}


static void FaceRegionMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( bSelectedInfoChar == -1 )
	{
		fShowFaceHightLight = FALSE;
		return;
	}
	else if( gCharactersList[ bSelectedInfoChar ].fValid == FALSE )
	{
		fShowFaceHightLight = FALSE;
		return;
	}

	if( ( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE ) )
	{
		fShowFaceHightLight = TRUE;
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fShowFaceHightLight = FALSE;
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
			if( ( bSelectedDestChar == -1 ) && ( fPlotForHelicopter == FALSE ) )
			{
				giDestHighLine = -1;
			}
		}

		fDrawCharacterList = TRUE;
	}
}


// update town mine pop up
static void UpdateTownMinePopUpDisplay(void)
{
	if ( gMapViewRegion.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		ForceUpDateOfBox( ghTownMineBox );
		MapscreenMarkButtonsDirty();
	}
}


static void HandleCharBarRender(void)
{
	 // check if the panel is disbled, if so, do not render
	if( ( bSelectedInfoChar !=-1) && ( fDisableDueToBattleRoster == FALSE ) )
	{
		// valid character?...render
		if( gCharactersList[ bSelectedInfoChar ].fValid == TRUE )
		{
			//if( !( ( fShowContractMenu)||( fShowAssignmentMenu ) ) )
			//{
				// draw bars for them
				DrawCharBars( );

				UpdateCharRegionHelpText( );
			//}
		}
	}
}


// update the status of the contract box
static void UpDateStatusOfContractBox(void)
{
	if( fShowContractMenu == TRUE )
	{
		ForceUpDateOfBox( ghContractBox );

		if( ( Menptr[gCharactersList[ bSelectedInfoChar ].usSolID].bLife == 0 )||( Menptr[gCharactersList[bSelectedInfoChar].usSolID].bAssignment == ASSIGNMENT_POW ) )
		{
			ForceUpDateOfBox( ghRemoveMercAssignBox );
		}
	}
}


static void DestroyTheItemInCursor(void)
{
	// actually destroy this item
	// End Item pickup
	MAPEndItemPointer( );
	gpItemPointer = NULL;
}


static void TrashItemMessageBoxCallBack(UINT8 bExitValue)
{
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		// find the item and get rid of it

		DestroyTheItemInCursor( );

		// reset cursor
		MSYS_ChangeRegionCursor( &gSMPanelRegion , CURSOR_NORMAL );
		SetCurrentCursorFromDatabase( CURSOR_NORMAL );

		HandleButtonStatesWhileMapInventoryActive( );
	}
}


static void TrashCanBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
			return;
		}

		// check if an item is in the cursor, if so, warn player
		if( gpItemPointer != NULL )
		{
		  // set up for mapscreen
			if( gpItemPointer -> ubMission )
			{
				DoMapMessageBox( MSG_BOX_BASIC_STYLE, pTrashItemText[ 1 ], MAP_SCREEN, MSG_BOX_FLAG_YESNO, TrashItemMessageBoxCallBack );
			}
			else
			{
				DoMapMessageBox( MSG_BOX_BASIC_STYLE, pTrashItemText[ 0 ], MAP_SCREEN, MSG_BOX_FLAG_YESNO, TrashItemMessageBoxCallBack );
			}
		}
	}
}


static void TrashCanMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if( gMPanelRegion.Cursor == EXTERN_CURSOR )
		{
			fShowTrashCanHighLight = TRUE;
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		fShowTrashCanHighLight = FALSE;
	}
}


static void MapInvDoneButtonfastHelpCall(void)
{
	SetPendingNewScreen( LAPTOP_SCREEN );
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


static INT8 GetLastValidCharacterInTeamPanelList(void)
{
	INT8 iCounter = 0, iValue = 0;

	// run through the list and find the last valid guy in the list
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			if(( Menptr[ gCharactersList[ iCounter ].usSolID ].bLife >= OKLIFE ) )
			{
				if( fShowMapInventoryPool )
				{
					if( Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX == sSelMapX && Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == sSelMapY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ == ( INT8 )( iCurrentMapSectorZ ) )
					{
						iValue = iCounter;
					}
				}
				else
				{
					if( fShowInventoryFlag && ( gMPanelRegion.Cursor == EXTERN_CURSOR ) )
					{
						if( bSelectedInfoChar != -1 )
						{
							if( gCharactersList[ bSelectedInfoChar ].fValid == TRUE )
							{
								if( Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorX &&  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ ==Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].bSectorZ )
								{
									iValue = iCounter;
								}
							}
						}
					}
					else
					{
						iValue = iCounter;
					}
				}
			}
		}
	}

	// return the character
	return( iValue );
}

/*
// NB These functions weren't being called anywhere!  Use GoToNextCharacterInList etc instead
INT8 GetPrevValidCharacterInTeamPanelList( INT8 bCurrentIndex )
{
	INT8 iCounter = bCurrentIndex, iValue = 0;

	// run through the list and find the last valid guy in the list
	for( iCounter = bCurrentIndex; iCounter > 0; iCounter-- )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			if( ( Menptr[ gCharactersList[ iCounter ].usSolID ].bLife >= OKLIFE ) )
			{
				if( fShowMapInventoryPool )
				{
					if(  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX == sSelMapX &&  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == sSelMapY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ == ( INT8 )( iCurrentMapSectorZ ) )
					{
						iValue = iCounter;
					}
				}
				else
				{
					if( fShowInventoryFlag && ( gMPanelRegion.Cursor == EXTERN_CURSOR ) )
					{
						if( bSelectedInfoChar != -1 )
						{
							if( gCharactersList[ bSelectedInfoChar ].fValid == TRUE )
							{
								if( Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorX &&  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ ==Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].bSectorZ )
								{
									iValue = iCounter;
									iCounter = 0;
								}
							}
						}
					}
					else
					{
						iValue = iCounter;
					}
				}
			}
		}
	}

	// return the character
	return( iValue );
}

INT8 GetNextValidCharacterInTeamPanelList( INT8 bCurrentIndex )
{
	INT8 iCounter = bCurrentIndex, iValue = 0;

	// run through the list and find the last valid guy in the list
	for( iCounter = bCurrentIndex; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			if( ( Menptr[ gCharactersList[ iCounter ].usSolID ].bLife >= OKLIFE ) )
			{
				if( fShowMapInventoryPool )
				{
					if(  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX == sSelMapX &&  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == sSelMapY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ == ( INT8 )( iCurrentMapSectorZ ) )
					{
						iValue = iCounter;
					}
				}
				else
				{
					if( fShowInventoryFlag && ( gMPanelRegion.Cursor == EXTERN_CURSOR ) )
					{
						if( bSelectedInfoChar != -1 )
						{
							if( gCharactersList[ bSelectedInfoChar ].fValid == TRUE )
							{
								if( Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorX == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorX &&  Menptr[ gCharactersList[ iCounter ].usSolID ].sSectorY == Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].sSectorY && Menptr[ gCharactersList[ iCounter ].usSolID ].bSectorZ ==Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ].bSectorZ )
								{
									iValue = iCounter;
								}
							}
						}
					}
					else
					{
						iValue = iCounter;
					}
				}
			}
		}
	}

	// return the character
	return( iValue );
}
*/


static void DoneInventoryMapBtnCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateDestroyTrashCanRegion(void)
{
	static BOOLEAN fCreated = FALSE;

	if( fShowInventoryFlag && ( fCreated == FALSE ) )
	{

		fCreated = TRUE;

		// trash can
		MSYS_DefineRegion( &gTrashCanRegion, 	TRASH_CAN_X, TRASH_CAN_Y, TRASH_CAN_X + TRASH_CAN_WIDTH, TRASH_CAN_Y + TRASH_CAN_HEIGHT , MSYS_PRIORITY_HIGHEST - 4 ,
							 MSYS_NO_CURSOR, TrashCanMoveCallback, TrashCanBtnCallback );

		// done inventory button define
		giMapInvButtonDoneImage = LoadButtonImage( "INTERFACE/done_button2.sti" ,-1,0,-1,1,-1 );
    giMapInvDoneButton = QuickCreateButton( giMapInvButtonDoneImage, INV_BTN_X, INV_BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, DoneInventoryMapBtnCallback );

		SetButtonFastHelpText( giMapInvDoneButton ,pMiscMapScreenMouseRegionHelpText[ 2 ] );
		SetRegionFastHelpText( &gTrashCanRegion, pMiscMapScreenMouseRegionHelpText[ 1 ] );

		InitMapKeyRingInterface( KeyRingItemPanelButtonCallback );
/*
		giMapInvNextImage=  LoadButtonImage( "INTERFACE/inventory_buttons.sti" ,-1,20,-1,22,-1 );
    giMapInvNext= QuickCreateButton( giMapInvNextImage, ( 2 ), ( 79 ) ,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, NextInventoryMapBtnCallback );


		giMapInvPrevImage=  LoadButtonImage( "INTERFACE/inventory_buttons.sti" ,-1,21,-1,23,-1 );
    giMapInvPrev= QuickCreateButton( giMapInvPrevImage, ( 30 ) , ( 79 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, PrevInventoryMapBtnCallback );

	*/

			// reset the compatable item array at this point
		ResetCompatibleItemArray( );

	}
	else if( ( fShowInventoryFlag == FALSE ) && ( fCreated == TRUE ) )
	{
		// trash can region
		fCreated = FALSE;
		MSYS_RemoveRegion( &gTrashCanRegion );

		// map inv done button
		RemoveButton( giMapInvDoneButton );

		// get rid of button image
		UnloadButtonImage( giMapInvButtonDoneImage );

		ShutdownKeyRingInterface( );

		if( fShowDescriptionFlag == TRUE )
		{
			// kill description
			DeleteItemDescriptionBox( );
		}

	}
}


static void InvmaskRegionBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	// CJC, December 15 1998: do NOTHING for clicks here
}


static void DoneInventoryMapBtnCallback(GUI_BUTTON* btn, INT32 reason)
{
	// prevent inventory from being closed while stack popup up!
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (gMPanelRegion.Cursor != EXTERN_CURSOR && !InItemStackPopup())
		{
			fEndShowInventoryFlag = TRUE;
		}
	}
}


static void StartConfirmMapMoveMode(INT16 sMapY)
{
	UINT8 ubPosition = ( sMapY < 8 ) ? MSG_MAP_UI_POSITION_LOWER : MSG_MAP_UI_POSITION_UPPER;

	// tell player what to do - to click again to confirm move
	MapScreenMessage( FONT_MCOLOR_LTYELLOW, ubPosition, pMapPlotStrings[ 0 ] );

	gfInConfirmMapMoveMode = TRUE;
}


void EndConfirmMapMoveMode( void )
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
	if( giUIMessageOverlay != -1 )
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
	if( ( gfPreBattleInterfaceActive == TRUE ) && ( fShowInventoryFlag == TRUE ) )
	{
		if( fShowDescriptionFlag == TRUE )
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
	UINT32 iCounter;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			CheckIfSoldierUnassigned( &Menptr[ gCharactersList[ iCounter ].usSolID ] );
		}
	}
}


static void InterruptTimeForMenus(void)
{
	if( ( fShowAssignmentMenu == TRUE )||( fShowContractMenu == TRUE ) )
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


static BOOLEAN AnyMercsLeavingRealSoon(void);


static void HandleContractTimeFlashForMercThatIsAboutLeave(void)
{
	INT32 iCurrentTime;

	// grab the current time
	iCurrentTime = GetJA2Clock();

	// only bother checking once flash interval has elapsed
	if( ( iCurrentTime - giFlashContractBaseTime ) >= DELAY_PER_FLASH_FOR_DEPARTING_PERSONNEL )
	{
		// update timer so that we only run check so often
		giFlashContractBaseTime = iCurrentTime;
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
static BOOLEAN AnyMercsLeavingRealSoon(void)
{
	UINT32 uiCounter = 0;
	UINT32 uiTimeInMin = GetWorldTotalMin();
	BOOLEAN fFoundOne = FALSE;

	for( uiCounter = 0; uiCounter < MAX_CHARACTER_COUNT; uiCounter++ )
	{
		if( gCharactersList[ uiCounter ].fValid == TRUE )
		{
			if( ( Menptr[gCharactersList[uiCounter].usSolID].iEndofContractTime - uiTimeInMin ) <= MINS_TO_FLASH_CONTRACT_TIME )
			{
				fFoundOne = TRUE;
				break;
			}
		}
	}

	return( fFoundOne );
}



BOOLEAN HandlePreloadOfMapGraphics( void )
{
	// check amt of memory, if above required amt...use it

	fPreLoadedMapGraphics = TRUE;

	guiBIGMAP = AddVideoSurfaceFromFile("INTERFACE/b_map.pcx");
	CHECKF(guiBIGMAP != NO_VSURFACE);
	guiMAPCURSORS = AddVideoObjectFromFile("INTERFACE/mapcursr.sti");
	CHECKF(guiMAPCURSORS != NO_VOBJECT);
	guiSAMICON = AddVideoObjectFromFile("INTERFACE/SAM.sti");
	CHECKF(guiSAMICON != NO_VOBJECT);
	// CHECKF(AddVideoObjectFromFile("INTERFACE/s_map.sti", &guiMAP));
	guiSleepIcon = AddVideoObjectFromFile("INTERFACE/sleepicon.sti");
	CHECKF(guiSleepIcon != NO_VOBJECT);
	//CHECKF(AddVideoObjectFromFile("INTERFACE/addonslcp.sti", &guiCORNERADDONS));
	//CHECKF(AddVideoObjectFromFile("INTERFACE/mapborder.sti", &guiMAPBORDER));
	guiCHARINFO = AddVideoObjectFromFile("INTERFACE/charinfo.sti");
	CHECKF(guiCHARINFO != NO_VOBJECT);
	guiCHARLIST = AddVideoObjectFromFile("INTERFACE/newgoldpiece3.sti");
	CHECKF(guiCHARLIST != NO_VOBJECT);
	//CHECKF(AddVideoObjectFromFile("INTERFACE/mapbordercorner.sti", &guiMAPCORNER));
	// CHECKF(AddVideoObjectFromFile("INTERFACE/popup.sti", &guiPOPUPBORDERS));

	// the sublevels
	guiSubLevel1 = AddVideoObjectFromFile("INTERFACE/Mine_1.sti");
	CHECKF(guiSubLevel1 != NO_VOBJECT);
	guiSubLevel2 = AddVideoObjectFromFile("INTERFACE/Mine_2.sti");
	CHECKF(guiSubLevel2 != NO_VOBJECT);
	guiSubLevel3 = AddVideoObjectFromFile("INTERFACE/Mine_3.sti");
	CHECKF(guiSubLevel3 != NO_VOBJECT);
	guiCHARICONS = AddVideoObjectFromFile("INTERFACE/boxes.sti");
	CHECKF(guiCHARICONS != NO_VOBJECT);
	guiCROSS = AddVideoObjectFromFile("INTERFACE/incross.sti");
	CHECKF(guiCROSS != NO_VOBJECT);
	guiMAPINV = AddVideoObjectFromFile("INTERFACE/mapinv.sti");
	CHECKF(guiMAPINV != NO_VOBJECT);
	guiMapInvSecondHandBlockout = AddVideoObjectFromFile("INTERFACE/map_inv_2nd_gun_cover.sti");
	CHECKF(guiMapInvSecondHandBlockout != NO_VOBJECT);

	// the upper left corner piece icons
	guiULICONS = AddVideoObjectFromFile("INTERFACE/top_left_corner_icons.sti");
	CHECKF(guiULICONS != NO_VOBJECT);
	guiTIXAICON = AddVideoObjectFromFile("INTERFACE/prison.sti");
	CHECKF(guiTIXAICON != NO_VOBJECT);

	HandleLoadOfMapBottomGraphics( );

	guiORTAICON = AddVideoObjectFromFile("INTERFACE/map_item.sti");
	CHECKF(guiORTAICON != NO_VOBJECT);
	guiCHARBETWEENSECTORICONS = AddVideoObjectFromFile("INTERFACE/merc_between_sector_icons.sti");
	CHECKF(guiCHARBETWEENSECTORICONS != NO_VOBJECT);
	guiCHARBETWEENSECTORICONSCLOSE = AddVideoObjectFromFile("INTERFACE/merc_mvt_green_arrows.sti");
	CHECKF(guiCHARBETWEENSECTORICONSCLOSE != NO_VOBJECT);

	guiLEVELMARKER = AddVideoObjectFromFile("INTERFACE/GreenArr.sti");
	CHECKF(guiLEVELMARKER != NO_VOBJECT);
	guiHelicopterIcon = AddVideoObjectFromFile("INTERFACE/Helicop.sti");
	CHECKF(guiHelicopterIcon != NO_VOBJECT);
	guiMapBorderEtaPopUp = AddVideoObjectFromFile("INTERFACE/eta_pop_up.sti");
	CHECKF(guiMapBorderEtaPopUp != NO_VOBJECT);
	guiMapBorderHeliSectors = AddVideoObjectFromFile("INTERFACE/pos2.sti");
	CHECKF(guiMapBorderHeliSectors != NO_VOBJECT);
	guiSecItemHiddenVO = AddVideoObjectFromFile("INTERFACE/secondary_gun_hidden.sti");
	CHECKF(guiSecItemHiddenVO != NO_VOBJECT);
	guiSelectedCharArrow = AddVideoObjectFromFile("INTERFACE/selectedchararrow.sti");
	CHECKF(guiSelectedCharArrow != NO_VOBJECT);
	guiMINEICON = AddVideoObjectFromFile("INTERFACE/mine.sti");
	CHECKF(guiMINEICON != NO_VOBJECT);
	guiSectorLocatorGraphicID = AddVideoObjectFromFile("INTERFACE/hilite.sti");

	//Kris:  Added this because I need to blink the icons button.
	guiNewMailIcons = AddVideoObjectFromFile("INTERFACE/newemail.sti");

	guiBULLSEYE = AddVideoObjectFromFile("INTERFACE/BullsEye.sti");
	CHECKF(guiBULLSEYE != NO_VOBJECT);

	// graphic for pool inventory
	LoadInventoryPoolGraphic( );

	// load border graphics
	LoadMapBorderGraphics( );

	// load the pop up for the militia pop up box
	LoadMilitiaPopUpBox( );

	return( TRUE );
}


void HandleRemovalOfPreLoadedMapGraphics( void )
{

	if( fPreLoadedMapGraphics == TRUE )
	{
		DeleteMapBottomGraphics( );
		DeleteVideoObjectFromIndex( guiMAPCURSORS );
		DeleteVideoObjectFromIndex( guiSleepIcon );

		//DeleteVideoObjectFromIndex(guiMAPBORDER);
		DeleteVideoObjectFromIndex(guiCHARLIST);
		//DeleteVideoObjectFromIndex(guiCORNERADDONS);
		//DeleteVideoObjectFromIndex(guiMAPCORNER);
	//	DeleteVideoObjectFromIndex(guiPOPUPBORDERS);
		DeleteVideoObjectFromIndex(guiCHARINFO);
		DeleteVideoObjectFromIndex(guiCHARICONS);
		DeleteVideoObjectFromIndex(guiCROSS);
#ifndef JA2DEMO
		DeleteVideoSurfaceFromIndex(guiBIGMAP);
		DeleteVideoObjectFromIndex(guiSubLevel1);
		DeleteVideoObjectFromIndex(guiSubLevel2);
		DeleteVideoObjectFromIndex(guiSubLevel3);

	//	DeleteVideoSurfaceFromIndex(guiPOPUPTEX);
		DeleteVideoObjectFromIndex(guiSAMICON);
#endif
		DeleteVideoObjectFromIndex(guiMAPINV);
		DeleteVideoObjectFromIndex(guiMapInvSecondHandBlockout);
		DeleteVideoObjectFromIndex(guiULICONS);
		DeleteVideoObjectFromIndex(guiORTAICON);
		DeleteVideoObjectFromIndex(guiTIXAICON);
		DeleteVideoObjectFromIndex( guiCHARBETWEENSECTORICONS );
		DeleteVideoObjectFromIndex( guiCHARBETWEENSECTORICONSCLOSE );
		DeleteVideoObjectFromIndex( guiLEVELMARKER );
		DeleteVideoObjectFromIndex( guiMapBorderEtaPopUp );
		DeleteVideoObjectFromIndex( guiSecItemHiddenVO );
		DeleteVideoObjectFromIndex( guiSelectedCharArrow );
		DeleteVideoObjectFromIndex( guiMapBorderHeliSectors );
		DeleteVideoObjectFromIndex( guiHelicopterIcon );
		DeleteVideoObjectFromIndex( guiMINEICON );
		DeleteVideoObjectFromIndex( guiSectorLocatorGraphicID );

		//Kris:  Remove the email icons.
		DeleteVideoObjectFromIndex( guiNewMailIcons );

		DeleteVideoObjectFromIndex( guiBULLSEYE );


		// remove the graphic for the militia pop up box
		RemoveMilitiaPopUpBox( );

		// remove inventory pool graphic
		RemoveInventoryPoolGraphic( );

		// get rid of border stuff
		DeleteMapBorderGraphics( );
	}
}


static BOOLEAN CharacterIsInLoadedSectorAndWantsToMoveInventoryButIsNotAllowed(INT8 bCharId)
{
	UINT16 usSoldierId = 0;

	// invalid char id
	if( bCharId == -1 )
	{
		return( FALSE );
	}

	// valid char?
	if( gCharactersList[ bCharId ].fValid == FALSE )
	{
		return( FALSE );
	}

	// get the soldier id
	usSoldierId = gCharactersList[ bCharId ].usSolID;

	// char is in loaded sector
	if( Menptr[ usSoldierId ].sSectorX != gWorldSectorX || Menptr[ usSoldierId ].sSectorY != gWorldSectorY || Menptr[ usSoldierId ].bSectorZ != gbWorldSectorZ )
	{
		return( FALSE );
	}

	// not showing inventory?
	if( fShowInventoryFlag == FALSE )
	{
		// nope
		return( FALSE );
	}

	// picked something up?
	if( gMPanelRegion.Cursor != EXTERN_CURSOR )
	{
		// nope
		return( FALSE );
	}

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
	if( gfPreBattleInterfaceActive )
	{
		if( IsMapScreenHelpTextUp() )
		{
			// stop mapscreen text
			StopMapScreenHelpText( );
		}
	}
	else if( bSelectedInfoChar == -1 )
	{
		DisableButton( giCharInfoButton[ 0 ] );
		DisableButton( giCharInfoButton[ 1 ] );
		DisableButton( giMapContractButton );
	}
/* ARM: Commented out at KM's request, it won't reenabled them when coming back from PBI, on Feb. 22, 99
	else if ( fShowInventoryFlag == FALSE ) // make sure that we are in fact showing the mapscreen inventory
	{
		return;
	}
*/
	else
	{
		// standard checks
		if ( ( GetNumberOfPeopleInCharacterList( ) < 2 ) ||
				 ( CharacterIsInLoadedSectorAndWantsToMoveInventoryButIsNotAllowed( bSelectedInfoChar ) ) ||
				 ( CharacterIsInTransitAndHasItemPickedUp( bSelectedInfoChar ) ) ||
				 ( fShowDescriptionFlag ) )
		{
			DisableButton( giCharInfoButton[ 0 ] );
			DisableButton( giCharInfoButton[ 1 ] );
		}
		else
		{
			EnableButton( giCharInfoButton[ 0 ] );
			EnableButton( giCharInfoButton[ 1 ] );
		}
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

	if( ( fInMapMode == TRUE ) && ( fCreated == FALSE ) )
	{
		// set the button image
		giCharInfoButtonImage[ 0 ]=  LoadButtonImage( "INTERFACE/map_screen_bottom_arrows.sti" ,11,4,-1,6,-1 );

#ifndef JA2DEMO

		// set the button value
		giCharInfoButton[ 0 ] = QuickCreateButton( giCharInfoButtonImage[ 0 ], 67, 69,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 5,
										BtnGenericMouseMoveButtonCallback, PrevInventoryMapBtnCallback );
#else

		// set the button value
		giCharInfoButton[ 0 ] = QuickCreateButton( giCharInfoButtonImage[ 0 ], 67, 69,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
										BtnGenericMouseMoveButtonCallback, PrevInventoryMapBtnCallback );

#endif
			// set the button image
		giCharInfoButtonImage[ 1 ]=  LoadButtonImage( "INTERFACE/map_screen_bottom_arrows.sti" ,12,5,-1,7,-1 );

#ifndef JA2DEMO
		// set the button value
		giCharInfoButton[ 1 ] = QuickCreateButton( giCharInfoButtonImage[ 1 ], 67, 87,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 5,
										BtnGenericMouseMoveButtonCallback, NextInventoryMapBtnCallback );

#else
		// set the button value
		giCharInfoButton[ 1 ] = QuickCreateButton( giCharInfoButtonImage[ 1 ], 67, 87,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST,
										BtnGenericMouseMoveButtonCallback, NextInventoryMapBtnCallback );

#endif

		SetButtonFastHelpText( giCharInfoButton[ 0 ], pMapScreenPrevNextCharButtonHelpText[ 0 ] );
		SetButtonFastHelpText( giCharInfoButton[ 1 ], pMapScreenPrevNextCharButtonHelpText[ 1 ] );

		fCreated = TRUE;

	}
	else if( ( ( fInMapMode == FALSE ) ) && ( fCreated == TRUE ) )
	{
		UnloadButtonImage( giCharInfoButtonImage[ 0 ] );
		UnloadButtonImage( giCharInfoButtonImage[ 1 ] );
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
#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_RED, MSG_BETAVERSION, L"(BETA) Can't compress time, pause state locked (reason %d). OK unless permanent.", guiLockPauseStateLastReasonId );
		ScreenMsg( FONT_MCOLOR_RED, MSG_BETAVERSION, L"(BETA) If permanent, take screenshot now, send with *previous* save & describe what happened since.");
#endif
	}
	else if( gfAtLeastOneMercWasHired == FALSE )
	{
		// no mercs hired, ever
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, pMapScreenJustStartedHelpText[ 0 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
	else if ( !AnyUsableRealMercenariesOnTeam() )
	{
		// no usable mercs left on team
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, pMapErrorString[ 39 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
	else if ( ActiveTimedBombExists() )
	{
		// can't time compress when a bomb is about to go off!
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, gzLateLocalizedString[ 2 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
	else if ( gfContractRenewalSquenceOn )
	{
#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_RED, MSG_BETAVERSION, L"(BETA) Can't compress time while contract renewal sequence is on.");
#endif
	}
	else if( fDisableMapInterfaceDueToBattle )
	{
#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_RED, MSG_BETAVERSION, L"(BETA) Can't compress time while disabled due to battle.");
#endif
	}
	else if( fDisableDueToBattleRoster )
	{
#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_RED, MSG_BETAVERSION, L"(BETA) Can't compress time while in battle roster.");
#endif
	}
	else if ( fMapInventoryItem )
	{
#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_RED, MSG_BETAVERSION, L"(BETA) Can't compress time while still holding an inventory item.");
#endif
	}
	else if( fShowMapInventoryPool )
	{
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, gzLateLocalizedString[ 55 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
	// ARM: THIS TEST SHOULD BE THE LAST ONE, BECAUSE IT ACTUALLY RESULTS IN SOMETHING HAPPENING NOW.
	// KM:  Except if we are in a creature lair and haven't loaded the sector yet (no battle yet)
	else if( gTacticalStatus.uiFlags & INCOMBAT || gTacticalStatus.fEnemyInSector )
	{
		if( OnlyHostileCivsInSector() )
		{
			wchar_t str[ 256 ];
			wchar_t pSectorString[ 128 ];
			GetSectorIDString( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, pSectorString, lengthof(pSectorString), TRUE );
			swprintf( str, lengthof(str), gzLateLocalizedString[ 27 ], pSectorString );
			DoMapMessageBox( MSG_BOX_BASIC_STYLE, str, MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
		}
		else
		{
			//The NEW non-persistant PBI is used instead of a dialog box explaining why we can't compress time.
			InitPreBattleInterface( NULL, FALSE );
		}
	}
	else if( PlayerGroupIsInACreatureInfestedMine() )
	{
		DoMapMessageBox( MSG_BOX_BASIC_STYLE, gzLateLocalizedString[ 28 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
}



void MapScreenDefaultOkBoxCallback( UINT8 bExitValue )
{
	// yes, load the game
  if( bExitValue == MSG_BOX_RETURN_OK )
	{
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
}



#ifdef JA2DEMO
/*
void MapScreenDemoOkBoxCallback( UINT8 bExitValue )
{
	// yes, load the game
  if( ( bExitValue == MSG_BOX_RETURN_OK ) || ( bExitValue == MSG_BOX_RETURN_NO ) )
	{

		fShowingMapDisableBox = FALSE;
		//SetPendingNewScreen( GAME_SCREEN );
	}
}
*/
#endif


static void MapSortBtnCallback(GUI_BUTTON *btn, INT32 reason)
{
	// grab the button index value for the sort buttons
	INT32 iValue = MSYS_GetBtnUserData(btn, 0);

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		if (IsMapScreenHelpTextUp())
		{
			StopMapScreenHelpText( );
			return;
		}
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
		ChangeCharacterListSortMethod( iValue );
	}
}


static void AddTeamPanelSortButtonsForMapScreen(void)
{
	INT32 iCounter = 0;
	SGPFILENAME filename;
	INT32 iImageIndex[ MAX_SORT_METHODS ] = { 0, 1, 5, 2, 3, 4 };		// sleep image is out or order (last)


	GetMLGFilename( filename, MLG_GOLDPIECEBUTTONS );

	for( iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++ )
	{
		giMapSortButtonImage[ iCounter ] = LoadButtonImage( filename, -1, iImageIndex[ iCounter ] , -1, iImageIndex[ iCounter ] + 6 , -1 );

		// buttonmake
		giMapSortButton[ iCounter ]= QuickCreateButton( giMapSortButtonImage[ iCounter ], ( INT16 )( gMapSortButtons[ iCounter ].iX ), ( INT16 )( gMapSortButtons[ iCounter ].iY ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 5,
										BtnGenericMouseMoveButtonCallback, MapSortBtnCallback );

		MSYS_SetBtnUserData(giMapSortButton[ iCounter ], 0, iCounter );

		SetButtonFastHelpText( giMapSortButton[ iCounter ], wMapScreenSortButtonHelpText[ iCounter ] );
	}
}


static INT16 CalcLocationValueForChar(INT32 iCounter);
static INT32 GetContractExpiryTime(SOLDIERTYPE* pSoldier);
static void SwapCharactersInList(INT32 iCharA, INT32 iCharB);


static void SortListOfMercsInTeamPanel(BOOLEAN fRetainSelectedMercs)
{
	INT32 iCounter = 0, iCounterA = 0;
	INT16 sEndSectorA, sEndSectorB;
	INT32 iExpiryTime, iExpiryTimeA;
	SOLDIERTYPE *pSelectedSoldier[ MAX_CHARACTER_COUNT ];
	SOLDIERTYPE *pCurrentSoldier = NULL;
	SOLDIERTYPE *pPreviousSelectedInfoChar = NULL;


	if ( fRetainSelectedMercs )
	{
		// if we have anyone valid selected
		if ( ( bSelectedInfoChar != -1 ) && ( gCharactersList[ bSelectedInfoChar ].fValid ) )
		{
			pPreviousSelectedInfoChar = &Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ];
		}


		for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
		{
			// set current entry to null
			pSelectedSoldier[ iCounter ] = NULL;


			// is this entry even valid
			if( gCharactersList[ iCounter ].fValid == FALSE )
			{
				continue;
			}

			// get soldier assoc. with entry
			pCurrentSoldier = &Menptr[ gCharactersList[ iCounter ].usSolID ];

			// check if soldier is active
			if( pCurrentSoldier->bActive == FALSE )
			{
				continue;
			}

			// are they selected?...
			if( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )
			{
				// yes, store pointer to them
				pSelectedSoldier[ iCounter ] = pCurrentSoldier;
			}
		}
	}


	// do the sort
	for( iCounter = 1; iCounter < FIRST_VEHICLE ; iCounter++ )
	{
		// have we gone too far
		if( gCharactersList[ iCounter ].fValid == FALSE )
		{
			break;
		}

		switch( giSortStateForMapScreenList )
		{
			case( 0 ):
				// by name
				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					if( gCharactersList[ iCounterA ].fValid == FALSE )
					{
						break;
					}

					if( ( wcscmp( Menptr[ gCharactersList[ iCounterA ].usSolID ].name, Menptr[ gCharactersList[ iCounter ].usSolID ].name ) > 0 ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 1 ):
				// by assignment
				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					if( gCharactersList[ iCounterA ].fValid == FALSE )
					{
						break;
					}

					if( ( Menptr[ gCharactersList[ iCounterA ].usSolID ].bAssignment > Menptr[ gCharactersList[ iCounter ].usSolID ].bAssignment ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
					else if( ( Menptr[ gCharactersList[ iCounterA ].usSolID ].bAssignment == Menptr[ gCharactersList[ iCounter ].usSolID ].bAssignment ) && ( iCounterA < iCounter ) )
					{
						// same assignment

						// if it's in a vehicle
						if( Menptr[ gCharactersList[ iCounterA ].usSolID ].bAssignment == VEHICLE )
						{
							// then also compare vehicle IDs
							if( ( Menptr[ gCharactersList[ iCounterA ].usSolID ].iVehicleId > Menptr[ gCharactersList[ iCounter ].usSolID ].iVehicleId ) && ( iCounterA < iCounter ) )
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
					if( gCharactersList[ iCounterA ].fValid == FALSE )
					{
						break;
					}

					if( ( Menptr[ gCharactersList[ iCounterA ].usSolID ].fMercAsleep == TRUE ) && ( Menptr[ gCharactersList[ iCounter ].usSolID ].fMercAsleep == FALSE ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 3 ):
				//by location

				sEndSectorA = CalcLocationValueForChar( iCounter );

				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					if( gCharactersList[ iCounterA ].fValid == FALSE )
					{
						break;
					}

					sEndSectorB = CalcLocationValueForChar( iCounterA );

					if( ( sEndSectorB > sEndSectorA ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 4 ):
				// by destination sector
				if( GetLengthOfMercPath( MercPtrs[ gCharactersList[ iCounter ].usSolID ] ) == 0 )
				{
					sEndSectorA = 9999;
				}
				else
				{
					sEndSectorA = GetLastSectorIdInCharactersPath( &Menptr[ gCharactersList[ iCounter ].usSolID ] );
				}

				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					if( gCharactersList[ iCounterA ].fValid == FALSE )
					{
						break;
					}

					if( GetLengthOfMercPath( MercPtrs[ gCharactersList[ iCounterA ].usSolID ] ) == 0 )
					{
						sEndSectorB = 9999;
					}
					else
					{
						sEndSectorB = GetLastSectorIdInCharactersPath( &Menptr[ gCharactersList[ iCounterA ].usSolID ] );
					}

					if( ( sEndSectorB > sEndSectorA ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			case( 5 ):
				iExpiryTime = GetContractExpiryTime( &( Menptr[ gCharactersList[ iCounter ].usSolID ] ) );

				//by contract expiry
				for( iCounterA = 0; iCounterA < FIRST_VEHICLE; iCounterA++ )
				{
					if( gCharactersList[ iCounterA ].fValid == FALSE )
					{
						break;
					}

					iExpiryTimeA = GetContractExpiryTime( &( Menptr[ gCharactersList[ iCounterA ].usSolID ] ) );

					if( ( iExpiryTimeA > iExpiryTime ) && ( iCounterA < iCounter ) )
					{
						SwapCharactersInList( iCounter, iCounterA );
					}
				}
				break;

			default:
				Assert(0);
				return;
		}
	}


	if ( fRetainSelectedMercs )
	{
		// select nobody & reset the selected list
		ChangeSelectedInfoChar( -1, TRUE );


		// now select all the soldiers that were selected before
		for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
		{
			if( pSelectedSoldier[ iCounter ] )
			{
				for( iCounterA = 0; iCounterA < MAX_CHARACTER_COUNT; iCounterA++ )
				{
					// is this entry even valid
					if( gCharactersList[ iCounterA ].fValid == FALSE )
					{
						continue;
					}

					// grab current soldier
					pCurrentSoldier = &Menptr[ gCharactersList[ iCounterA ].usSolID ];

					// check if soldier is active
					if( pCurrentSoldier->bActive == FALSE )
					{
						continue;
					}

					// this guy is selected
					if( pSelectedSoldier[ iCounter ] == pCurrentSoldier )
					{
						SetEntryInSelectedCharacterList( ( INT8 ) iCounterA );
					}

					// update who the currently selected info guy is
					if( pPreviousSelectedInfoChar == pCurrentSoldier )
					{
						ChangeSelectedInfoChar( ( INT8 ) iCounterA, FALSE );
					}
				}
			}
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
	UINT16 usTempSoldID;

	// swap
	usTempSoldID = gCharactersList[ iCharA ].usSolID;
	gCharactersList[ iCharA ].usSolID =  gCharactersList[ iCharB ].usSolID;
	gCharactersList[ iCharB ].usSolID = usTempSoldID;
}


static void RemoveTeamPanelSortButtonsForMapScreen(void)
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < MAX_SORT_METHODS; iCounter++ )
	{
		UnloadButtonImage( giMapSortButtonImage[ iCounter ] );
		RemoveButton( giMapSortButton[ iCounter ] );

		giMapSortButtonImage[ iCounter ] = -1;
		giMapSortButton[ iCounter ] = -1;
	}
}


static void HandleCommonGlowTimer(void)
{
	INT32 iCurrentTime = 0;

	// grab the current time
	iCurrentTime = GetJA2Clock();

	// only bother checking once flash interval has elapsed
	if( ( iCurrentTime - giCommonGlowBaseTime ) >= GLOW_DELAY )
	{
		// update timer so that we only run check so often
		giCommonGlowBaseTime = iCurrentTime;

		// set flag to trigger glow higlight updates
		gfGlowTimerExpired = TRUE;
	}
	else
	{
		gfGlowTimerExpired = FALSE;
	}
}


static void HandleAssignmentsDoneAndAwaitingFurtherOrders(void)
{
	// run through list of grunts and handle awating further orders
	INT32 iCounter = 0, iCurrentTime = 0;
	SOLDIERTYPE *pSoldier = NULL;


	// update "nothing to do" flags if necessary
	if ( gfReEvaluateEveryonesNothingToDo )
	{
		ReEvaluateEveryonesNothingToDo();
	}

	// grab the current time
	iCurrentTime = GetJA2Clock();

	// only bother checking once flash interval has elapsed
	if( ( iCurrentTime - giFlashAssignBaseTime ) >= ASSIGNMENT_DONE_FLASH_TIME )
	{
		// update timer so that we only run check so often
		giFlashAssignBaseTime = iCurrentTime;

		for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
		{
			if( gCharactersList[ iCounter ].fValid == FALSE )
			{
				break;
			}

			pSoldier = &( Menptr[ gCharactersList[ iCounter ].usSolID ] );

			// toggle and redraw if flash was left ON even though the flag is OFF
			if( pSoldier->fDoneAssignmentAndNothingToDoFlag || fFlashAssignDone )
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
	SOLDIERTYPE *pSoldier;

	// if we are in inventory
	if( fShowInventoryFlag == TRUE )
	{
		return;
	}

	HVOBJECT hHandle = GetVideoObject(guiSleepIcon);

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( gCharactersList[ iCounter ].fValid == TRUE )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];
			if( pSoldier->bActive && pSoldier->fMercAsleep && CanChangeSleepStatusForSoldier( pSoldier ) )
			{
				BltVideoObject( guiSAVEBUFFER , hHandle, 0, 125, ( INT16 )( Y_START+(iCounter * ( Y_SIZE + 2 ) ) ));
			}
		}
	}
}

#ifdef JA2DEMO

void DisplayExitToTacticalGlowDuringDemo( void )
{
  static INT32 iColorNum=10;
  static BOOLEAN fDelta=FALSE;
	UINT16 usColor = 0;
	UINT32 uiDestPitchBYTES;
  UINT8  *pDestBuf;


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

	usColor=Get16BPPColor( FROMRGB( GlowColorsA[iColorNum].ubRed, GlowColorsA[iColorNum].ubGreen, GlowColorsA[iColorNum].ubBlue ) );
  pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480);
  RectangleDraw( TRUE, 496, 409, 528, 442, usColor, pDestBuf );
	InvalidateRegion(495, 408, 529+1, 442+1);
	UnLockVideoSurface( FRAME_BUFFER );
}

#endif

//Kris:  Added this function to blink the email icon on top of the laptop button whenever we are in
//       mapscreen and we have new email to read.
static void CheckForAndRenderNewMailOverlay(void)
{
	if( fNewMailFlag )
	{
		if( GetJA2Clock() % 1000 < 667 )
		{
			if( ButtonList[ guiMapBottomExitButtons[ MAP_EXIT_TO_LAPTOP ] ]->uiFlags & BUTTON_CLICKED_ON )
			{ //button is down, so offset the icon
				BltVideoObjectFromIndex( FRAME_BUFFER, guiNewMailIcons, 1, 465, 418);
				InvalidateRegion( 465, 418, 480, 428 );
			}
			else
			{ //button is up, so draw the icon normally
				BltVideoObjectFromIndex( FRAME_BUFFER, guiNewMailIcons, 0, 464, 417);
				if( !(ButtonList[ guiMapBottomExitButtons[ MAP_EXIT_TO_LAPTOP ] ]->uiFlags & BUTTON_ENABLED ) )
				{
					UINT32 uiDestPitchBYTES;
					UINT8  *pDestBuf;
					SGPRect area = { 463, 417, 477, 425 };

				  pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
					Blt16BPPBufferHatchRect( (UINT16*)pDestBuf, uiDestPitchBYTES, &area );
					UnLockVideoSurface( FRAME_BUFFER );
				}
				InvalidateRegion( 463, 417, 481, 430 );

			}
		}
		else
		{ //The blink is now off, so mark the button dirty so that it'll render next frame.
			MarkAButtonDirty( guiMapBottomExitButtons[ MAP_EXIT_TO_LAPTOP ] );
		}
	}
}



BOOLEAN CanToggleSelectedCharInventory( void )
{
	SOLDIERTYPE *pSoldier = NULL;


	if( gfPreBattleInterfaceActive == TRUE )
	{
		return(FALSE);
	}

	// already in inventory and an item picked up?
	if( fShowInventoryFlag && ( gMPanelRegion.Cursor == EXTERN_CURSOR ) )
	{
		return(FALSE);
	}


	// nobody selected?
	if( bSelectedInfoChar == -1 )
	{
		return(FALSE);
	}

	// does the selected guy have inventory and can we get at it?
	if ( !MapCharacterHasAccessibleInventory( bSelectedInfoChar ) )
	{
		return(FALSE);
	}

	pSoldier = MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ];

	// if not in inventory, and holding an item from sector inventory
	if( !fShowInventoryFlag &&
			( ( gMPanelRegion.Cursor == EXTERN_CURSOR ) || gpItemPointer || fMapInventoryItem ) &&
			( gpItemPointerSoldier == NULL ) )
	{
		// make sure he's in that sector
		if ( ( pSoldier->sSectorX != sSelMapX ) ||
				 ( pSoldier->sSectorY != sSelMapY ) ||
				 ( pSoldier->bSectorZ != iCurrentMapSectorZ ) ||
				 pSoldier->fBetweenSectors )
		{
			return(FALSE);
		}
	}


	// passed!
	return(TRUE);
}



BOOLEAN MapCharacterHasAccessibleInventory( INT8 bCharNumber )
{
	SOLDIERTYPE *pSoldier = NULL;


	Assert( bCharNumber >= 0 );
	Assert( bCharNumber < MAX_CHARACTER_COUNT );

	// invalid character slot selected?
	if( gCharactersList[ bCharNumber ].fValid == FALSE )
	{
		return(FALSE);
	}

	pSoldier = MercPtrs[ gCharactersList[ bCharNumber ].usSolID ];

	if( ( pSoldier->bAssignment == IN_TRANSIT ) ||
			( pSoldier->bAssignment == ASSIGNMENT_POW ) ||
			( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) ||
			( AM_A_ROBOT( pSoldier ) ) ||
			( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__EPC ) ||
			( pSoldier->bLife < OKLIFE )
		)
	{
		return(FALSE);
	}

	return( TRUE );
}


static void CheckForInventoryModeCancellation(void)
{
	if ( fShowInventoryFlag || fShowDescriptionFlag )
	{
		// can't bail while player has an item in hand...
		if( gMPanelRegion.Cursor == EXTERN_CURSOR )
		{
			return;
		}

		if ( !CanToggleSelectedCharInventory( ) )
		{
			// get out of inventory mode if it's on!  (could have just bled below OKLIFE)
			if ( fShowInventoryFlag )
			{
				fShowInventoryFlag = FALSE;
				SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );
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


void ChangeSelectedMapSector( INT16 sMapX, INT16 sMapY, INT8 bMapZ )
{
	// ignore while map inventory pool is showing, or else items can be replicated, since sector inventory always applies
	// only to the currently selected sector!!!
	if( fShowMapInventoryPool )
		return;

	if ( gfPreBattleInterfaceActive )
		return;

	if( !IsTheCursorAllowedToHighLightThisSector( sMapX, sMapY ) )
		return;

	// disallow going underground while plotting (surface) movement
	if ( ( bMapZ != 0 ) && ( ( bSelectedDestChar != -1 ) || fPlotForHelicopter ) )
		return;


	sSelMapX = sMapX;
	sSelMapY = sMapY;
	iCurrentMapSectorZ = bMapZ;

	// if going underground while in airspace mode
	if ( ( bMapZ > 0 ) && ( fShowAircraftFlag == TRUE ) )
	{
		// turn off airspace mode
		ToggleAirspaceMode( );
	}

	fMapPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;

	// also need this, to update the text coloring of mercs in this sector
	fTeamPanelDirty = TRUE;
}


static BOOLEAN CanChangeDestinationForCharSlot(INT8 bCharNumber, BOOLEAN fShowErrorMessage)
{
	SOLDIERTYPE *pSoldier = NULL;
	INT8 bErrorNumber = -1;


	if ( bCharNumber == -1 )
		return( FALSE );

	if ( gCharactersList[ bCharNumber ].fValid == FALSE )
		return (FALSE);


	pSoldier = MercPtrs[ gCharactersList[ bCharNumber ].usSolID ];

	// valid soldier?
	Assert( pSoldier );
	Assert( pSoldier->bActive );


	if ( CanEntireMovementGroupMercIsInMove( pSoldier, &bErrorNumber ) )
	{
		return( TRUE );
	}
	else
	{
		// function may fail without returning any specific error # (-1).
		// if it gave us the # of an error msg, and we were told to display it
		if ( ( bErrorNumber != -1 ) && fShowErrorMessage )
		{
			ReportMapScreenMovementError( bErrorNumber );
		}

		return( FALSE );
	}
}



BOOLEAN CanExtendContractForCharSlot( INT8 bCharNumber )
{
	SOLDIERTYPE *pSoldier = NULL;


	if ( bCharNumber == -1 )
		return( FALSE );

	if ( gCharactersList[ bCharNumber ].fValid == FALSE )
		return (FALSE);

	pSoldier = MercPtrs[ gCharactersList[ bCharNumber ].usSolID ];

	// valid soldier?
	Assert( pSoldier );
	Assert( pSoldier->bActive );

	// if a vehicle, in transit, or a POW
	if( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) ||
			( pSoldier->bAssignment == IN_TRANSIT ) ||
			( pSoldier->bAssignment == ASSIGNMENT_POW ) )
	{
		// can't extend contracts at this time
		return (FALSE);
	}

	// mercs below OKLIFE, M.E.R.C. mercs, EPCs, and the Robot use the Contract menu so they can be DISMISSED/ABANDONED!

	// everything OK
	return( TRUE );
}


BOOLEAN CanChangeSleepStatusForCharSlot( INT8 bCharNumber )
{
	SOLDIERTYPE *pSoldier = NULL;

	if ( bCharNumber == -1 )
		return( FALSE );

	if ( gCharactersList[ bCharNumber ].fValid == FALSE )
		return (FALSE);


	pSoldier = MercPtrs[ gCharactersList[ bCharNumber ].usSolID ];

	return( CanChangeSleepStatusForSoldier( pSoldier ) );
}


BOOLEAN CanChangeSleepStatusForSoldier( SOLDIERTYPE *pSoldier )
{
	// valid soldier?
	Assert( pSoldier );
	Assert( pSoldier->bActive );

	// if a vehicle, robot, in transit, or a POW
	if( ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) || AM_A_ROBOT( pSoldier ) ||
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
	MSYS_ChangeRegionCursor( &gMapScreenMaskRegion, usCursor );

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


static void CancelOrShortenPlottedPath(void)
{
	INT16 sMapX, sMapY;
	UINT32 uiReturnValue;


	GetMouseMapXY(&sMapX, &sMapY);

/*
	// translate zoom in to zoom out coords
	if(fZoomFlag)
	{
		sMapX=(UINT16)iZoomX/MAP_GRID_X+sMapX;
		sMapX=sMapX/2;
		sMapY=(UINT16)iZoomY/MAP_GRID_Y+sMapY;
		sMapY=sMapY/2;
	 }
*/

	// check if we are in aircraft mode
	if( fShowAircraftFlag == TRUE )
	{
		// check for helicopter path being plotted
		if( !fPlotForHelicopter )
			return;

		// if player can't redirect it
		if ( CanHelicopterFly( ) == FALSE )
		{
			// explain & ignore
			ExplainWhySkyriderCantFly();
			return;
		}


		// try to delete portion of path AFTER the current sector for the helicopter
		uiReturnValue = ClearPathAfterThisSectorForHelicopter( sMapX, sMapY );
	}
	else
	{
		// check for character path being plotted
		if( bSelectedDestChar == -1 )
			return;

		// try to delete portion of path AFTER the current sector for the helicopter
		uiReturnValue = ClearPathAfterThisSectorForCharacter( &Menptr[gCharactersList[bSelectedDestChar].usSolID], sMapX, sMapY );
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
			MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[ 4 ] );
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


static INT32 GetContractExpiryTime(SOLDIERTYPE* pSoldier)
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



void ChangeSelectedInfoChar( INT8 bCharNumber, BOOLEAN fResetSelectedList )
{
	Assert( ( bCharNumber >= -1 ) && ( bCharNumber < MAX_CHARACTER_COUNT ) );

	if( ( bCharNumber != -1 ) && ( gCharactersList[ bCharNumber ].fValid == FALSE ) )
		return;


	// if holding an item
	if ( ( gMPanelRegion.Cursor == EXTERN_CURSOR ) || gpItemPointer || fMapInventoryItem )
	{
		// make sure we can give it to this guy, otherwise don't allow the change
		if ( !MapscreenCanPassItemToCharNum( bCharNumber ) )
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
			if ( ( bCharNumber == -1 ) || !MapCharacterHasAccessibleInventory( bCharNumber ) )
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



void CopyPathToAllSelectedCharacters( PathStPtr pPath )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;


	// run through list and copy paths for each selected character
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

			// skip itself!
			if ( GetSoldierMercPathPtr( pSoldier ) != pPath )
			{
				if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
				{
					pVehicleList[ pSoldier->bVehicleID ].pMercPath = CopyPaths( pPath, pVehicleList[ pSoldier->bVehicleID ].pMercPath );
				}
				else if( pSoldier->bAssignment == VEHICLE )
				{
					pVehicleList[ pSoldier->iVehicleId ].pMercPath = CopyPaths( pPath, pVehicleList[ pSoldier->iVehicleId ].pMercPath );
				}
				else
				{
					pSoldier->pMercPath = CopyPaths( pPath, pSoldier->pMercPath );
				}

				// don't use CopyPathToCharactersSquadIfInOne(), it will whack the original pPath by replacing that merc's path!
			}
		}
	}
}



void CancelPathsOfAllSelectedCharacters()
{
	INT8 bCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;
	BOOLEAN fSkyriderMsgShown = FALSE;


	// cancel destination for the clicked and ALL other valid & selected characters with a route set
	for( bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++ )
	{
		// if we've clicked on a selected valid character
		if( ( gCharactersList[ bCounter ].fValid == TRUE ) && IsEntryInSelectedListSet( bCounter ) )
		{
			pSoldier = MercPtrs[ gCharactersList[ bCounter ].usSolID ];

			// and he has a route set
			if ( GetLengthOfMercPath( pSoldier ) > 0 )
			{
				// if he's in the chopper, but player can't redirect it
				if ( ( pSoldier->bAssignment == VEHICLE ) && ( pSoldier->iVehicleId == iHelicopterVehicleId ) && ( CanHelicopterFly( ) == FALSE ) )
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
					CancelPathForVehicle( &( pVehicleList[ pSoldier->bVehicleID ] ), FALSE );
				}
				else
				{
					CancelPathForCharacter( pSoldier );
				}
			}
		}
	}
}


static void ConvertMinTimeToETADayHourMinString(UINT32 uiTimeInMin, STR16 sString, size_t Length)
{
	UINT32 uiDay, uiHour, uiMin;

	uiDay  = ( uiTimeInMin / NUM_MIN_IN_DAY );
	uiHour = ( uiTimeInMin - ( uiDay * NUM_MIN_IN_DAY ) ) / NUM_MIN_IN_HOUR;
	uiMin	 = uiTimeInMin - ( ( uiDay * NUM_MIN_IN_DAY ) + ( uiHour * NUM_MIN_IN_HOUR ) );

	// there ain't enough room to show both the day and ETA: and without ETA it's confused as the current time
//	swprintf(sString, L"%ls %ls %d, %02d:%02d", pEtaString[0], pDayStrings[0], uiDay, uiHour, uiMin);
//	swprintf(sString, L"%ls %d, %02d:%02d", pDayStrings[0], uiDay, uiHour, uiMin);
	swprintf( sString, Length, L"%ls %02d:%02d", pEtaString[ 0 ], uiHour, uiMin );
}


static INT32 GetGroundTravelTimeOfCharacter(INT8 bCharNumber)
{
	INT32 iTravelTime = 0;

	if( bCharNumber == -1 )
		return(0);

	if ( !gCharactersList[ bCharNumber ].fValid )
		return(0);

	// get travel time for the last path segment (stored in pTempCharacterPath)
	iTravelTime = GetPathTravelTimeDuringPlotting( pTempCharacterPath );

	// add travel time for any prior path segments (stored in the selected character's mercpath, but waypoints aren't built)
	iTravelTime += GetPathTravelTimeDuringPlotting( GetSoldierMercPathPtr( MercPtrs[ gCharactersList[ bCharNumber ].usSolID ] ) );

	return( iTravelTime );
}


static INT16 CalcLocationValueForChar(INT32 iCounter)
{
	SOLDIERTYPE *pSoldier = NULL;
	INT16 sLocValue = 0;

	Assert( iCounter < MAX_CHARACTER_COUNT );

	if( gCharactersList[ iCounter ].fValid == FALSE )
		return( sLocValue );

	pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

	// don't reveal location of POWs!
	if( pSoldier->bAssignment != ASSIGNMENT_POW )
	{
		sLocValue = SECTOR( pSoldier->sSectorX, pSoldier->sSectorY );
		// underground: add 1000 per sublevel
		sLocValue += 1000 * ( pSoldier->bSectorZ );
	}

	return( sLocValue );
}


static void CancelChangeArrivalSectorMode(void)
{
	// "put him in change arrival sector" mode
	gfInChangeArrivalSectorMode = FALSE;

	// change the cursor to that mode
	SetUpCursorForStrategicMap();

	fMapPanelDirty = TRUE;
}


static void MakeMapModesSuitableForDestPlotting(INT8 bCharNumber)
{
	SOLDIERTYPE *pSoldier = NULL;


	if( gCharactersList[ bCharNumber ].fValid == TRUE )
	{
		pSoldier = MercPtrs[ gCharactersList[ bCharNumber ].usSolID ];

		CancelSectorInventoryDisplayIfOn( FALSE );

		TurnOnShowTeamsMode( );

		if ( ( pSoldier->bAssignment == VEHICLE ) && ( pSoldier->iVehicleId == iHelicopterVehicleId ) )
		{
			if ( fShowAircraftFlag == FALSE )
			{
				// turn on airspace mode automatically
				ToggleAirspaceMode();
			}
		}
		else
		{
			if ( fShowAircraftFlag == TRUE )
			{
				// turn off airspace mode automatically
				ToggleAirspaceMode();
			}
		}

		// if viewing a different sublevel
		if ( iCurrentMapSectorZ != pSoldier->bSectorZ )
		{
			// switch to that merc's sublevel
			JumpToLevel( pSoldier->bSectorZ );
		}
	}
}


static BOOLEAN AnyMovableCharsInOrBetweenThisSector(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ)
{
	INT32 iFirstId = 0, iLastId = 0;
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;


	// to speed it up a little?
	iFirstId = gTacticalStatus.Team[ OUR_TEAM ].bFirstID;
	iLastId = gTacticalStatus.Team[ OUR_TEAM ].bLastID;

	for( iCounter = iFirstId; iCounter <= iLastId; iCounter++ )
	{
		// get the soldier
		pSoldier = &Menptr[ iCounter ];

		// is the soldier active
		if( pSoldier->bActive == FALSE )
		{
			continue;
		}

		// POWs, dead guys, guys in transit can't move
		if ( ( pSoldier->bAssignment == IN_TRANSIT ) ||
				 ( pSoldier->bAssignment == ASSIGNMENT_POW ) ||
				 ( pSoldier->bAssignment == ASSIGNMENT_DEAD ) ||
				 ( pSoldier->bLife == 0 ) )
		{
			continue;
		}

		// don't count mercs aboard Skyrider
		if ( ( pSoldier->bAssignment == VEHICLE ) && ( pSoldier->iVehicleId == iHelicopterVehicleId ) )
		{
			continue;
		}

		// don't count vehicles - in order for them to move, somebody must be in the sector with them
		if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
		{
			continue;
		}


		// is he here?
		if( ( pSoldier->sSectorX == sSectorX ) && ( pSoldier->sSectorY == sSectorY ) && ( pSoldier->bSectorZ == bSectorZ ) )
		{
			// NOTE that we consider mercs between sectors, mercs < OKLIFE, and sleeping mercs to be "movable".
			// This lets CanCharacterMoveInStrategic() itself report the appropriate error message when character is clicked
			return( TRUE );
		}
	}


	return( FALSE );
}


static UINT8 PlayerMercsInHelicopterSector(void);


static BOOLEAN RequestGiveSkyriderNewDestination(void)
{
	// should we allow it?
	if( CanHelicopterFly( ) == TRUE )
	{
		// if not warned already, and chopper empty, but mercs are in this sector
		if ( !gfSkyriderEmptyHelpGiven &&
			 ( GetNumberOfPassengersInHelicopter() == 0 ) &&
			 ( PlayerMercsInHelicopterSector() > 0 ) )
		{
			DoMapMessageBox( MSG_BOX_BASIC_STYLE, pSkyriderText[ 6 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
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
		gpHelicopterPreviousMercPath = CopyPaths( pVehicleList[ iHelicopterVehicleId ].pMercPath, gpHelicopterPreviousMercPath );

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



void ExplainWhySkyriderCantFly( void )
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
	if ( CanHelicopterTakeOff() == FALSE )
	{
		SkyRiderTalk( CHOPPER_NOT_ACCESSIBLE );
//		DoMapMessageBox( MSG_BOX_BASIC_STYLE, pSkyriderText[ 4 ], MAP_SCREEN, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
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


static UINT8 PlayerMercsInHelicopterSector(void)
{
	GROUP *pGroup = NULL;

	Assert( iHelicopterVehicleId != -1);
	pGroup = GetGroup( pVehicleList[ iHelicopterVehicleId ].ubMovementGroup );

	if ( pGroup->fBetweenSectors )
	{
		return( 0 );
	}

	return( PlayerMercsInSector( pGroup->ubSectorX, pGroup->ubSectorY, 0 ) );
}


static void RandomAwakeSelectedMercConfirmsStrategicMove(void);
static void WakeUpAnySleepingSelectedMercsOnFootOrDriving(void);


static void HandleNewDestConfirmation(INT16 sMapX, INT16 sMapY)
{
	UINT8 ubCurrentProgress;


	// if moving the chopper itself, or moving a character aboard the chopper
	if( fPlotForHelicopter )
	{
		// if there are no enemies in destination sector, or we don't know
		if ( ( NumEnemiesInSector( sMapX, sMapY ) == 0 ) ||
				 (	WhatPlayerKnowsAboutEnemiesInSector( sMapX, sMapY ) == KNOWS_NOTHING ) )
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
		MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pMapPlotStrings[ 1 ] );
	}


	// wake up anybody who needs to be awake to travel
	WakeUpAnySleepingSelectedMercsOnFootOrDriving();
}


static void RandomAwakeSelectedMercConfirmsStrategicMove(void)
{
	SOLDIERTYPE *pSoldier = NULL;
	INT32 iCounter;
	UINT8	ubSelectedMercID[ 20 ];
	UINT8	ubSelectedMercIndex[ 20 ];
	UINT8	ubNumMercs = 0;
	UINT8	ubChosenMerc;


	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( ( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE ) )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

			if ( pSoldier->bLife >= OKLIFE && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) &&
						!AM_A_ROBOT( pSoldier ) && !AM_AN_EPC( pSoldier ) && !pSoldier->fMercAsleep )
			{
				ubSelectedMercID[ ubNumMercs ] = pSoldier->ubID;
				ubSelectedMercIndex[ ubNumMercs ] = (UINT8)iCounter;

				ubNumMercs++;
			}
		}
	}

	if ( ubNumMercs > 0 )
	{
		ubChosenMerc = (UINT8)Random( ubNumMercs );

		// select that merc so that when he speaks we're showing his portrait and not someone else
		ChangeSelectedInfoChar( ubSelectedMercIndex[ ubChosenMerc ], FALSE );

		DoMercBattleSound( MercPtrs[ ubSelectedMercID[ ubChosenMerc ] ], ( UINT8 ) ( Random(2) ? BATTLE_SOUND_OK1 : BATTLE_SOUND_OK2 ) );
		//TacticalCharacterDialogue( MercPtrs[ ubSelectedMercID[ ubChosenMerc ] ], ubQuoteNum );
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
	INT8 bTownId;
	BOOLEAN fTownStillHidden;
	CHAR16 sString[ 128 ];


	// if on the surface
	if ( iCurrentMapSectorZ == 0 )
	{
		bTownId = GetTownIdForSector( sSelMapX, sSelMapY );
		fTownStillHidden = ( ( bTownId == TIXA ) && !fFoundTixa ) || ( ( bTownId == ORTA ) && !fFoundOrta );

		if( ( bTownId != BLANK_SECTOR ) && !fTownStillHidden )
		{
			if ( MilitiaTrainingAllowedInSector( sSelMapX, sSelMapY, ( INT8 )iCurrentMapSectorZ ) )
			{
				if( fShowTownInfo == TRUE )
				{
					fShowTownInfo = FALSE;
				}
				fMapPanelDirty = TRUE;

				// check if there's combat in any of the town's sectors
				if ( CanRedistributeMilitiaInSector( sSelMapX, sSelMapY, bTownId ) )
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
				swprintf( sString, lengthof(sString), pMapErrorString[ 31 ], pTownNames [ bTownId ] );
				DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, NULL );
			}
		}
		else if( IsThisSectorASAMSector( sSelMapX, sSelMapY, 0 ) && fSamSiteFound[ GetSAMIdFromSector( sSelMapX, sSelMapY, 0 ) ] )
		{
			// can't move militia around sam sites
			DoScreenIndependantMessageBox( pMapErrorString[ 30 ], MSG_BOX_FLAG_OK, NULL );
		}
	}
}



#ifdef JA2TESTVERSION
void DumpSectorDifficultyInfo( void )
{
	// NOTE: This operates on the selected map sector!
	CHAR16 wSectorName[ 128 ];

	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Playing Difficulty: %ls", gzGIOScreenText[GIO_DIF_LEVEL_TEXT + gGameOptions.ubDifficultyLevel]);
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Highest Progress (0-100) = %d%%", HighestPlayerProgressPercentage() );
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Player Kills = %d", gStrategicStatus.usPlayerKills );

	GetSectorIDString(sSelMapX, sSelMapY, (INT8)iCurrentMapSectorZ, wSectorName, lengthof(wSectorName), TRUE);
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"SECTOR: %ls", wSectorName );

	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Pyth. Distance From Meduna (0-20) = %d", GetPythDistanceFromPalace( sSelMapX, sSelMapY ) );

	if ( ( gWorldSectorX == sSelMapX ) && ( gWorldSectorY == sSelMapY ) && ( gbWorldSectorZ == iCurrentMapSectorZ ) )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Enemy Difficulty Factor (0 to 100) = %d%%", CalcDifficultyModifier( SOLDIER_CLASS_ARMY ) );
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"Avg Regular Enemy Exp. Level (2-6) = %d", 2 + ( CalcDifficultyModifier( SOLDIER_CLASS_ARMY ) / 20 ) );
	}
	else
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"--Must load sector to calculate difficulty--" );
	}
}
#endif


static void StartChangeSectorArrivalMode(void)
{
	// "put him in change arrival sector" mode
	gfInChangeArrivalSectorMode = TRUE;

	// redraw map with bullseye removed
	fMapPanelDirty = TRUE;

	// change the cursor to that mode
	SetUpCursorForStrategicMap();

	// give instructions as overlay message
	MapScreenMessage( FONT_MCOLOR_LTYELLOW, MSG_MAP_UI_POSITION_MIDDLE, pBullseyeStrings[ 0 ] );
}


static BOOLEAN CanMoveBullseyeAndClickedOnIt(INT16 sMapX, INT16 sMapY)
{
	// if in airspace mode, and not plotting paths
	if( ( fShowAircraftFlag == TRUE ) && ( bSelectedDestChar == -1 ) && ( fPlotForHelicopter == FALSE ) )
	{
		// don't allow moving bullseye until after initial arrival
		if ( gTacticalStatus.fDidGameJustStart == FALSE )
		{
			// if he clicked on the bullseye, and we're on the surface level
			if ( ( sMapX == gsMercArriveSectorX ) && ( sMapY == gsMercArriveSectorY ) && ( iCurrentMapSectorZ == 0 ) )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


static void BullsEyeOrChopperSelectionPopupCallback(UINT8 ubExitValue);


static void CreateBullsEyeOrChopperSelectionPopup(void)
{
	wcscpy( gzUserDefinedButton1, pHelicopterEtaStrings[ 8 ] );
	wcscpy( gzUserDefinedButton2, pHelicopterEtaStrings[ 9 ] );

	// do a BULLSEYE/CHOPPER message box
	DoScreenIndependantMessageBox( pHelicopterEtaStrings[ 7 ], MSG_BOX_FLAG_GENERIC, BullsEyeOrChopperSelectionPopupCallback );
}


static void BullsEyeOrChopperSelectionPopupCallback(UINT8 ubExitValue)
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


// wake up anybody who needs to be awake to travel
static void WakeUpAnySleepingSelectedMercsOnFootOrDriving(void)
{
	SOLDIERTYPE *pSoldier = NULL;
	INT32 iCounter;
	BOOLEAN fSuccess = FALSE;


	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( ( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE ) )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

			// if asleep
			if ( pSoldier->fMercAsleep )
			{
				// and on foot or driving
				if ( ( pSoldier->bAssignment < ON_DUTY ) ||
						 ( ( pSoldier->bAssignment == VEHICLE ) && SoldierMustDriveVehicle( pSoldier, pSoldier->iVehicleId, FALSE ) ) )
				{
					// we should be guaranteed that he CAN wake up to get this far, so report errors, but don't force it
					fSuccess = SetMercAwake( pSoldier, TRUE, FALSE );
					Assert( fSuccess );
				}
			}
		}
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
		AdjustLoyaltyForCivsEatenByMonsters( (UINT8)SECTORX( gsEnemyGainedControlOfSectorID ),
																				 (UINT8)SECTORY( gsEnemyGainedControlOfSectorID ),
																				 (UINT8)gsCiviliansEatenByMonsters );
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
		SetThisSectorAsEnemyControlled( (UINT8)SECTORX( gsEnemyGainedControlOfSectorID ), (UINT8)SECTORY( gsEnemyGainedControlOfSectorID ), 0, TRUE );
		gsEnemyGainedControlOfSectorID = -2;
	}
	else if( gsEnemyGainedControlOfSectorID == -2 )
	{ //dirty the mapscreen after the dialog box goes away.
		fMapPanelDirty = TRUE;
		gsEnemyGainedControlOfSectorID = -1;
	}
	else if( gbMilitiaPromotions )
	{
		wchar_t str[ 512 ];
		BuildMilitiaPromotionsString(str, lengthof(str));
		DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, MapScreenDefaultOkBoxCallback );
	}
}


void GetMapscreenMercAssignmentString( SOLDIERTYPE *pSoldier, wchar_t sString[] )
{
	if( pSoldier->bAssignment != VEHICLE )
	{
		wcscpy(sString, pAssignmentStrings[ pSoldier->bAssignment ] );
	}
	else
	{
		wcscpy( sString, pShortVehicleStrings[ pVehicleList[ pSoldier->iVehicleId ].ubVehicleType ] );
	}
}


void GetMapscreenMercLocationString( SOLDIERTYPE *pSoldier, wchar_t sString[], size_t Length)
{
	if( pSoldier->bAssignment == IN_TRANSIT )
	{
		// show blank
		wcscpy( sString, L"--" );
	}
	else
	{
		if( pSoldier->bAssignment == ASSIGNMENT_POW )
		{
			// POW - location unknown
			wcslcpy(sString, pPOWStrings[1], Length);
		}
		else
		{
			// put brackets around it when he's between sectors!
			swprintf(sString, Length, pSoldier->fBetweenSectors ? L"(%ls%ls%ls)" : L"%ls%ls%ls",
						pMapVertIndex[ pSoldier->sSectorY ], pMapHortIndex[ pSoldier->sSectorX ], pMapDepthIndex[ pSoldier->bSectorZ ] );
		}
	}
}


void GetMapscreenMercDestinationString( SOLDIERTYPE *pSoldier, wchar_t sString[], size_t Length)
{
	INT32 iSectorX, iSectorY;
	INT16 sSector=0;
	GROUP *pGroup = NULL;


	// by default, show nothing
	wcscpy( sString, L"" );

	// if dead or POW - has no destination (no longer part of a group, for that matter)
	if( ( pSoldier->bAssignment == ASSIGNMENT_DEAD ) ||
			( pSoldier->bAssignment == ASSIGNMENT_POW ) ||
			( pSoldier->bLife == 0 ) )
	{
		return;
	}

	if( pSoldier->bAssignment == IN_TRANSIT )
	{
		// show the sector he'll be arriving in
		iSectorX = gsMercArriveSectorX;
		iSectorY = gsMercArriveSectorY;
	}
	else
	{
		// if he's going somewhere
		if ( GetLengthOfMercPath( pSoldier ) > 0 )
		{
			sSector = GetLastSectorIdInCharactersPath( pSoldier );
			// convert
			iSectorX = sSector % MAP_WORLD_X;
			iSectorY = sSector / MAP_WORLD_Y;
		}
		else // no movement path is set...
		{
			if ( pSoldier->fBetweenSectors )
			{
				// he must be returning to his previous (reversed so as to be the next) sector, so show that as his destination
				// individual soldiers don't store previous/next sector coordinates, must go to his group for that
				pGroup = GetGroup( GetSoldierGroupId( pSoldier ) );
				Assert( pGroup );
				iSectorX = pGroup->ubNextX;
				iSectorY = pGroup->ubNextY;
			}
			else
			{
				// show nothing
				return;
			}
		}
	}


	swprintf( sString, Length, L"%ls%ls", pMapVertIndex[ iSectorY ], pMapHortIndex[ iSectorX ] );
}


void GetMapscreenMercDepartureString( SOLDIERTYPE *pSoldier, wchar_t sString[], size_t Length , UINT8 *pubFontColor )
{
	INT32 iMinsRemaining = 0;
	INT32 iDaysRemaining = 0;
	INT32 iHoursRemaining = 0;


	if( ( pSoldier->ubWhatKindOfMercAmI != MERC_TYPE__AIM_MERC && pSoldier->ubProfile != SLAY ) || pSoldier->bLife == 0 )
	{
		wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], Length);
	}
	else
	{
		iMinsRemaining = pSoldier->iEndofContractTime - GetWorldTotalMin();

		//if the merc is in transit
		if( pSoldier->bAssignment == IN_TRANSIT )
		{
			//and if the time left on the cotract is greater then the contract time
			if( iMinsRemaining > (INT32)( pSoldier->iTotalContractLength * NUM_MIN_IN_DAY ) )
			{
				iMinsRemaining = ( pSoldier->iTotalContractLength * NUM_MIN_IN_DAY );
			}
		}

		// if 3 or more days remain
		if( iMinsRemaining >= MAP_TIME_UNDER_THIS_DISPLAY_AS_HOURS )
		{
			iDaysRemaining = iMinsRemaining / (24*60);

			*pubFontColor = FONT_LTGREEN;

			swprintf(sString, Length, L"%d%ls", iDaysRemaining, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ] );
		}
		else	// less than 3 days
		{
			if( iMinsRemaining > 5 )
			{
				iHoursRemaining = ( iMinsRemaining + 59 ) / 60;
			}
			else
			{
				iHoursRemaining = 0;
			}

		 // last 3 days is Red, last 4 hours start flashing red/white!
		 if( ( iMinsRemaining <= MINS_TO_FLASH_CONTRACT_TIME ) && ( fFlashContractFlag == TRUE ) )
		 {
			 *pubFontColor = FONT_WHITE;
		 }
		 else
		 {
			 *pubFontColor = FONT_RED;
		 }

		 swprintf(sString, Length, L"%d%ls", iHoursRemaining, gpStrategicString[ STR_PB_HOURS_ABBREVIATION ] );
		}
	}
}


static void InitPreviousPaths(void)
{
	INT32 iCounter = 0;

	// init character previous paths
	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		gpCharacterPreviousMercPath[ iCounter ] = NULL;
	}

	// init helicopter previous path
	gpHelicopterPreviousMercPath = NULL;
}


void RememberPreviousPathForAllSelectedChars( void )
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )
		{
			pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

			// remember his previous path by copying it to his slot in the array kept for that purpose
			gpCharacterPreviousMercPath[ iCounter ] = CopyPaths( GetSoldierMercPathPtr( MercPtrs[ gCharactersList[ iCounter ].usSolID ] ), gpCharacterPreviousMercPath[ iCounter ] );
		}
	}
}


static void RestorePreviousPaths(void)
{
	INT32 iCounter = 0;
	SOLDIERTYPE *pSoldier = NULL;
	PathStPtr *ppMovePath = NULL;
	UINT8 ubGroupId = 0;
	BOOLEAN fPathChanged = FALSE;


	// invalid if we're not plotting movement
	Assert( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) );


	if ( fPlotForHelicopter == TRUE )
	{
		ppMovePath = &( pVehicleList[ iHelicopterVehicleId ].pMercPath );
		ubGroupId = pVehicleList[ iHelicopterVehicleId ].ubMovementGroup;

		// if the helicopter had a previous path
		if( gpHelicopterPreviousMercPath != NULL )
		{
			gpHelicopterPreviousMercPath = MoveToBeginningOfPathList( gpHelicopterPreviousMercPath );

			// clear current path
			*ppMovePath = ClearStrategicPathList( *ppMovePath, ubGroupId );
			// replace it with the previous one
			*ppMovePath = CopyPaths( gpHelicopterPreviousMercPath, *ppMovePath );
			// will need to rebuild waypoints
			fPathChanged = TRUE;
		}
		else	// no previous path
		{
			// if he currently has a path
			if( *ppMovePath )
			{
				// wipe it out!
				*ppMovePath = MoveToBeginningOfPathList( *ppMovePath );
				*ppMovePath = ClearStrategicPathList( *ppMovePath, ubGroupId );
				// will need to rebuild waypoints
				fPathChanged = TRUE;
			}
		}

		if ( fPathChanged )
		{
			// rebuild waypoints
			RebuildWayPointsForGroupPath( *ppMovePath, ubGroupId );

			// copy his path to all selected characters
			CopyPathToAllSelectedCharacters( *ppMovePath );
		}
	}
	else	// character(s) plotting
	{
		for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
		{
			// if selected
			if( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )
			{
				pSoldier = MercPtrs[ gCharactersList[ iCounter ].usSolID ];

				if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
				{
					ppMovePath = &( pVehicleList[ pSoldier->bVehicleID ].pMercPath );
					ubGroupId = pVehicleList[ pSoldier->bVehicleID ].ubMovementGroup;
				}
				else if( pSoldier->bAssignment == VEHICLE )
				{
					ppMovePath = &( pVehicleList[ pSoldier->iVehicleId ].pMercPath );
					ubGroupId = pVehicleList[ pSoldier->iVehicleId ].ubMovementGroup;
				}
				else if( pSoldier->bAssignment < ON_DUTY )
				{
					ppMovePath = &( pSoldier->pMercPath );
					ubGroupId = pSoldier->ubGroupID;
				}
				else
				{
					// invalid pSoldier - that guy can't possibly be moving, he's on a non-vehicle assignment!
					Assert( 0 );
					continue;
				}


				fPathChanged = FALSE;

				// if we have the previous path stored for the dest char
				if( gpCharacterPreviousMercPath[ iCounter ] )
				{
					gpCharacterPreviousMercPath[ iCounter ] = MoveToBeginningOfPathList( gpCharacterPreviousMercPath[ iCounter ] );

					// clear current path
					*ppMovePath = ClearStrategicPathList( *ppMovePath, ubGroupId );
					// replace it with the previous one
					*ppMovePath = CopyPaths( gpCharacterPreviousMercPath[ iCounter ], *ppMovePath );
					// will need to rebuild waypoints
					fPathChanged = TRUE;
				}
				else	// no previous path stored
				{
					// if he has one now, wipe it out
					if( *ppMovePath )
					{
						// wipe it out!
						*ppMovePath = MoveToBeginningOfPathList( *ppMovePath );
						*ppMovePath = ClearStrategicPathList( *ppMovePath, ubGroupId );
						// will need to rebuild waypoints
						fPathChanged = TRUE;
					}
				}


				if ( fPathChanged )
				{
					// rebuild waypoints
					RebuildWayPointsForGroupPath( *ppMovePath, ubGroupId );
				}
			}
		}
	}
}


static void ClearPreviousPaths(void)
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < MAX_CHARACTER_COUNT; iCounter++ )
	{
		if( fSelectedListOfMercsForMapScreen[ iCounter ] == TRUE )
		{
			gpCharacterPreviousMercPath[ iCounter ] = ClearStrategicPathList( gpCharacterPreviousMercPath[ iCounter ], 0 );
		}
	}
	gpHelicopterPreviousMercPath = ClearStrategicPathList( gpHelicopterPreviousMercPath, 0 );
}


static void SelectAllCharactersInSquad(INT8 bSquadNumber)
{
	INT8 bCounter;
	BOOLEAN fFirstOne = TRUE;
	SOLDIERTYPE *pSoldier;

	// ignore if that squad is empty
	if( SquadIsEmpty( bSquadNumber ) == TRUE )
	{
		return;
	}

	// select nobody & reset the selected list
	ChangeSelectedInfoChar( -1, TRUE );

	// now select all the soldiers that are in this squad
	for( bCounter = 0; bCounter < MAX_CHARACTER_COUNT; bCounter++ )
	{
		// is this entry is valid
		if( gCharactersList[ bCounter ].fValid == TRUE )
		{
			pSoldier = MercPtrs[ gCharactersList[ bCounter ].usSolID ];

			// if this guy is on that squad or in a vehicle which is assigned to that squad
			// NOTE: There's no way to select everyone aboard Skyrider with this function...
			if ( ( pSoldier->bAssignment == bSquadNumber ) || IsSoldierInThisVehicleSquad( pSoldier, bSquadNumber ) )
			{
				if ( fFirstOne )
				{
					// make the first guy in the list who is in this squad the selected info char
					ChangeSelectedInfoChar( bCounter, FALSE );

					// select his sector
					ChangeSelectedMapSector( pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ );

					fFirstOne = FALSE;
				}

				SetEntryInSelectedCharacterList( bCounter );
			}
		}
	}
}



BOOLEAN CanDrawSectorCursor( void )
{
	if( /*( fCursorIsOnMapScrollButtons == FALSE ) && */
			( fShowTownInfo == FALSE ) && ( ghTownMineBox == -1 ) &&
			( fShowUpdateBox == FALSE ) && ( GetNumberOfMercsInUpdateList() == 0 ) &&
			( sSelectedMilitiaTown == 0 ) && ( gfMilitiaPopupCreated == FALSE ) &&
			( gfStartedFromMapScreen == FALSE ) &&
			( fShowMapScreenMovementList == FALSE ) && ( ghMoveBox == - 1 ) &&
			( fMapInventoryItem == FALSE )
		)
	{
		return( TRUE );
	}

	return( FALSE );
}


static void RestoreMapSectorCursor(INT16 sMapX, INT16 sMapY)
{
	INT16 sScreenX, sScreenY;


	Assert( ( sMapX >= 1 ) && ( sMapX <= 16 ) );
	Assert( ( sMapY >= 1 ) && ( sMapY <= 16 ) );

	GetScreenXYFromMapXY( sMapX, sMapY, &sScreenX, &sScreenY );

	sScreenY -= 1;

/*
	if(fZoomFlag)
		RestoreExternBackgroundRect( ((INT16)( sScreenX - MAP_GRID_X )), ((INT16)( sScreenY - MAP_GRID_Y )), DMAP_GRID_ZOOM_X, DMAP_GRID_ZOOM_Y);
	else
*/
	RestoreExternBackgroundRect( sScreenX, sScreenY, DMAP_GRID_X, DMAP_GRID_Y );
}


static void RequestToggleMercInventoryPanel(void)
{
	if( IsMapScreenHelpTextUp() )
	{
		// stop mapscreen text
		StopMapScreenHelpText( );
		return;
	}

	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
	{
		AbortMovementPlottingMode( );
	}


	if ( !CanToggleSelectedCharInventory() )
	{
		return;
	}

	if( fShowDescriptionFlag == TRUE )
	{
		// turn off item description
		DeleteItemDescriptionBox( );
	}
	else
	{
		// toggle inventory mode
		fShowInventoryFlag = !fShowInventoryFlag;

		// set help text for item glow region
		if( fShowInventoryFlag )
		{
			SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 2 ] );
		}
		else
		{
			SetRegionFastHelpText( &gCharInfoHandRegion, pMiscMapScreenMouseRegionHelpText[ 0 ] );
		}
	}

	fTeamPanelDirty = TRUE;
}


static void RequestContractMenu(void)
{
	if( IsMapScreenHelpTextUp() )
	{
		// stop mapscreen text
		StopMapScreenHelpText( );
		return;
	}

	if( gfPreBattleInterfaceActive == TRUE )
	{
		return;
	}

	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
	{
		AbortMovementPlottingMode( );
	}


	// in case we have multiple guys selected, turn off everyone but the guy we're negotiating with
	ChangeSelectedInfoChar( bSelectedInfoChar, TRUE );

	if ( CanExtendContractForCharSlot( bSelectedInfoChar ) )
	{
		// create
		RebuildContractBoxForMerc( &Menptr[ gCharactersList[ bSelectedInfoChar ].usSolID ] );

		// reset selected characters
		ResetAllSelectedCharacterModes( );

		bSelectedContractChar = bSelectedInfoChar;
		giContractHighLine = bSelectedContractChar;

		// if not triggered internally
		if ( CheckIfSalaryIncreasedAndSayQuote( MercPtrs[ gCharactersList[ bSelectedInfoChar ].usSolID ], TRUE ) == FALSE )
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

	if( IsMapScreenHelpTextUp() )
	{
		// stop mapscreen text
		StopMapScreenHelpText( );
		return;
	}

	if( gfPreBattleInterfaceActive == TRUE )
	{
		return;
	}

	if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
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
