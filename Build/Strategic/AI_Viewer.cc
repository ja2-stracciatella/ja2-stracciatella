#ifdef JA2BETAVERSION

#include "AI_Viewer.h"
#include "Font.h"
#include "HImage.h"
#include "Local.h"
#include "English.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "Editor_Taskbar_Utils.h"
#include "Line.h"
#include "Input.h"
#include "Text_Input.h"
#include "MouseSystem.h"
#include "StrategicMap.h"
#include "FileMan.h"
#include "Map_Information.h"
#include "Render_Dirty.h"
#include "Game_Clock.h"
#include "Campaign_Types.h"
#include "Queen_Command.h"
#include "Campaign_Init.h"
#include "Overhead.h"
#include "Strategic_Movement.h"
#include "GameSettings.h"
#include "Game_Event_Hook.h"
#include "Creature_Spreading.h"
#include "Message.h"
#include "Game_Init.h"
#include "Auto_Resolve.h"
#include "Campaign.h"
#include "Debug.h"
#include "Strategic_AI.h"
#include "Strategic_Status.h"
#include "Video.h"
#include "WordWrap.h"
#include "SGP.h"


#define VIEWER_LEFT			15
#define VIEWER_TOP			15
#define VIEWER_WIDTH		417
#define VIEWER_HEIGHT		353
#define VIEWER_CELLW		26
#define VIEWER_CELLH		22
#define VIEWER_RIGHT		(VIEWER_LEFT+VIEWER_WIDTH)
#define VIEWER_BOTTOM		(VIEWER_TOP+VIEWER_HEIGHT)

enum{
	VIEWER_EXIT,
	VIEWER_TIMEPANEL,
	VIEWER_RESET,
	RESET_EASY,
	RESET_NORMAL,
	RESET_HARD,
	COMPRESSION0,
	COMPRESSION5,
	COMPRESSION15,
	COMPRESSION60,
	//COMPRESSION6H,
	TEST_INCOMING_4SIDES,
	START_CREATURE_QUEST,
	SPREAD_CREATURES,
	CREATURE_ATTACK,
	VIEW_ENEMIES,
	VIEW_CREATURES,
	BASEMENT1_BTN,
	BASEMENT2_BTN,
	BASEMENT3_BTN,
	RELOAD_SECTOR,
	QUEEN_AWAKE_TOGGLE,
	NUM_VIEWER_BUTTONS
};

enum
{
	MOVE_RED_ICON,
	MOVE_BLUE_ICON,
	MOVE_YELLOW_ICON,
	MOVE_GRAY_ICON,
	MOVE_BURGUNDY_ICON,
	MOVE_ORANGE_ICON,
	MOVE_GREEN_ICON,
	STOP_RED_ICON,
	STOP_BLUE_ICON,
	STOP_YELLOW_ICON,
	STOP_GRAY_ICON,
	STOP_BURGUNDY_ICON,
	STOP_ORANGE_ICON,
	STOP_GREEN_ICON,
	REIN_RED_ICON,
	REIN_BLUE_ICON,
	REIN_YELLOW_ICON,
	REIN_GRAY_ICON,
	REIN_BURGUNDY_ICON,
	REIN_ORANGE_ICON,
	REIN_GREEN_ICON,
	STAGE_RED_ICON,
	STAGE_BLUE_ICON,
	STAGE_YELLOW_ICON,
	STAGE_GRAY_ICON,
	STAGE_BURGUNDY_ICON,
	STAGE_ORANGE_ICON,
	STAGE_GREEN_ICON,
	SAM_ICON,
	MINING_ICON,
	GROUP_ANCHOR,
};

enum IconType
{
	ICON_TYPE_PATROL,
	ICON_TYPE_STOPPED,
	ICON_TYPE_REINFORCEMENT,
	ICON_TYPE_ASSAULT,
	NUM_ICON_TYPES
};

enum IconColour
{
	ICON_COLOR_RED,
	ICON_COLOR_BLUE,
	ICON_COLOR_YELLOW,
	ICON_COLOR_GRAY,
	ICON_COLOR_BURGUNDY,
	ICON_COLOR_ORANGE,
	ICON_COLOR_GREEN,
	NUM_ICON_COLORS
};


MOUSE_REGION ViewerRegion;

static SGPVObject* guiMapGraphicID;
static SGPVObject* guiMapIconsID;

BOOLEAN gfViewerEntry;
BOOLEAN gfExitViewer;

BOOLEAN gfRenderViewer;
static BOOLEAN gfRenderMap;

BOOLEAN gfViewEnemies = TRUE;
INT8		gbViewLevel = 0;

UINT16 gusBlue;
UINT16 gusLtBlue;
UINT16 gusDkBlue;

INT16 gsAINumAdmins = -1;
INT16 gsAINumTroops = -1;
INT16 gsAINumElites = -1;
INT16 gsAINumCreatures = -1;
BOOLEAN gfOverrideSector = FALSE;

UINT32 guiLastTime;

INT32 giSaveTCMode; //time compression mode;

//The sector coordinates of the mouse position (yellow)
static INT16 gsHiSectorX = 0;
static INT16 gsHiSectorY = 0;
//The sector coordinates of the selected sector (red)
static INT16 gsSelSectorX = 0;
static INT16 gsSelSectorY = 0;

GUIButtonRef iViewerButton[NUM_VIEWER_BUTTONS];


extern BOOLEAN gfQueenAIAwake;
extern INT32 giReinforcementPool;
extern INT32 giReinforcementPoints, giRequestPoints;
extern ARMY_COMPOSITION gArmyComp[ NUM_ARMY_COMPOSITIONS ];
extern GARRISON_GROUP *gGarrisonGroup;


static const wchar_t *gwGroupTypeString[NUM_ENEMY_INTENTIONS] =
{
	L"RETREAT",
	L"ASSAULT",
	L"STAGING",
	L"PATROL",
	L"REINFORCE"
};


static void StringFromValue(wchar_t *str, size_t n, INT32 iValue, UINT32 uiMax)
{
	if( iValue < 0 )			//a blank string is determined by a negative value.
		str[0] = '\0';
	else if( (UINT32)iValue > uiMax )	//higher than max attribute value, so convert it to the max.
		swprintf(str, n, L"%d", uiMax);
	else										//this is a valid static value, so convert it to a string.
		swprintf(str, n, L"%d", iValue);
}


static void B1Callback(GUI_BUTTON* btn, INT32 reason);
static void B2Callback(GUI_BUTTON* btn, INT32 reason);
static void B3Callback(GUI_BUTTON* btn, INT32 reason);
static void ClearViewerRegion(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom);
static void Compression0Callback(GUI_BUTTON* btn, INT32 reason);
static void Compression15Callback(GUI_BUTTON* btn, INT32 reason);
static void Compression5Callback(GUI_BUTTON* btn, INT32 reason);
static void Compression60Callback(GUI_BUTTON* btn, INT32 reason);
static void CreatureAttackCallback(GUI_BUTTON* btn, INT32 reason);
static void EasyCallback(GUI_BUTTON* btn, INT32 reason);
static void HardCallback(GUI_BUTTON* btn, INT32 reason);
static void NormalCallback(GUI_BUTTON* btn, INT32 reason);
static void ReloadSectorCallback(GUI_BUTTON* btn, INT32 reason);
static void SpreadCreaturesCallback(GUI_BUTTON* btn, INT32 reason);
static void StartCreatureQuestCallback(GUI_BUTTON* btn, INT32 reason);
static void TestIncoming4SidesCallback(GUI_BUTTON* btn, INT32 reason);
static void ToggleQueenAwake(GUI_BUTTON* btn, INT32 reason);
static void ViewCreaturesCallback(GUI_BUTTON* btn, INT32 reason);
static void ViewEnemiesCallback(GUI_BUTTON* btn, INT32 reason);
static void ViewerExitCallback(GUI_BUTTON* btn, INT32 reason);
static void ViewerMapClickCallback(MOUSE_REGION* reg, INT32 reason);
static void ViewerMapMoveCallback(MOUSE_REGION* reg, INT32 reason);


static BOOLEAN CreateAIViewer(void)
{
	wchar_t str[6];

	//Check to see if data exists.
	if (!FileExists("devtools/arulco.sti") ||
			!FileExists("devtools/icons.sti")  ||
			!FileExists("devtools/smcheckbox.sti"))
	{
		ScreenMsg( FONT_WHITE, MSG_BETAVERSION, L"AIViewer missing data.  Aborted." );
		gfExitViewer = FALSE;
		gfViewerEntry = TRUE;
		return FALSE;
	}

	DisableScrollMessages();
	giSaveTCMode = giTimeCompressMode;

	guiMapGraphicID = AddVideoObjectFromFile("devtools/arulco.sti");
	guiMapIconsID   = AddVideoObjectFromFile("devtools/icons.sti");

	gfRenderViewer = TRUE;

	//Create all of the buttons here
	iViewerButton[VIEWER_EXIT] = CreateTextButton(L"Exit", BLOCKFONT2, FONT_RED, FONT_BLACK, 585, 425, 50, 30, MSYS_PRIORITY_HIGH, ViewerExitCallback);

	iViewerButton[VIEWER_TIMEPANEL] = CreateLabel(WORLDTIMESTR, FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 3, 0, 88, 20, MSYS_PRIORITY_HIGH);
	iViewerButton[COMPRESSION0]  = CreateTextButton(L"0",  FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT +  3, 20, 17, 16, MSYS_PRIORITY_HIGH, Compression0Callback);
	iViewerButton[COMPRESSION5]  = CreateTextButton(L"5",  FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 20, 20, 17, 16, MSYS_PRIORITY_HIGH, Compression5Callback);
	iViewerButton[COMPRESSION15] = CreateTextButton(L"15", FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 37, 20, 18, 16, MSYS_PRIORITY_HIGH, Compression15Callback);
	iViewerButton[COMPRESSION60] = CreateTextButton(L"60", FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 55, 20, 18, 16, MSYS_PRIORITY_HIGH, Compression60Callback);
/*
	iViewerButton[COMPRESSION6H] = CreateTextButton(L"6H", FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 73, 20, 18, 16, MSYS_PRIORITY_HIGH, Compression6HCallback);
*/

	iViewerButton[VIEWER_RESET] = CreateLabel(L"Reset Enemies", FONT12POINT1, FONT_BLACK, FONT_BLACK, 526, 0, 114, 20, MSYS_PRIORITY_HIGH);
	iViewerButton[RESET_EASY]   = CreateTextButton(L"Easy",   FONT12POINT1, FONT_BLACK, FONT_BLACK, 526, 20, 35, 16, MSYS_PRIORITY_HIGH, EasyCallback);
	iViewerButton[RESET_NORMAL] = CreateTextButton(L"Normal", FONT12POINT1, FONT_BLACK, FONT_BLACK, 561, 20, 44, 16, MSYS_PRIORITY_HIGH, NormalCallback);
	iViewerButton[RESET_HARD]   = CreateTextButton(L"Hard",   FONT12POINT1, FONT_BLACK, FONT_BLACK, 605, 20, 35, 16, MSYS_PRIORITY_HIGH, HardCallback);

	iViewerButton[TEST_INCOMING_4SIDES] = CreateTextButton(L"Incoming 4 Sides",     FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 20, 100, 120, 18, MSYS_PRIORITY_HIGH, TestIncoming4SidesCallback);
	iViewerButton[START_CREATURE_QUEST] = CreateTextButton(L"Start Creature Quest", FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 20, 125, 120, 18, MSYS_PRIORITY_HIGH, StartCreatureQuestCallback);
	iViewerButton[SPREAD_CREATURES]     = CreateTextButton(L"Spread Creatures",     FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 20, 150, 120, 18, MSYS_PRIORITY_HIGH, SpreadCreaturesCallback);
	iViewerButton[CREATURE_ATTACK]      = CreateTextButton(L"Creature Attack",      FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 20, 175, 120, 18, MSYS_PRIORITY_HIGH, CreatureAttackCallback);

	iViewerButton[QUEEN_AWAKE_TOGGLE] = CreateCheckBoxButton(104, VIEWER_BOTTOM + 22, "devtools/smcheckbox.sti", MSYS_PRIORITY_HIGH, ToggleQueenAwake);
	if( gfQueenAIAwake )
	{
		iViewerButton[QUEEN_AWAKE_TOGGLE]->uiFlags |= BUTTON_CLICKED_ON;
	}

	iViewerButton[RELOAD_SECTOR] = CreateTextButton(L"Override Sector", FONT12POINT1, FONT_BLACK, FONT_BLACK, 10, VIEWER_BOTTOM + 5, 90, 18, MSYS_PRIORITY_HIGH, ReloadSectorCallback);

	iViewerButton[VIEW_ENEMIES]   = CreateTextButton(L"View Enemies",   FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT +  13, 40, 90, 20, MSYS_PRIORITY_HIGH, ViewEnemiesCallback);
	iViewerButton[VIEW_CREATURES] = CreateTextButton(L"View Creatures", FONT12POINT1, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 103, 40, 90, 20, MSYS_PRIORITY_HIGH, ViewCreaturesCallback);

	iViewerButton[BASEMENT1_BTN] = CreateTextButton(L"B1", FONT16ARIAL, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT +  58, 60, 30, 24, MSYS_PRIORITY_HIGH, B1Callback);
	iViewerButton[BASEMENT2_BTN] = CreateTextButton(L"B2", FONT16ARIAL, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT +  88, 60, 30, 24, MSYS_PRIORITY_HIGH, B2Callback);
	iViewerButton[BASEMENT3_BTN] = CreateTextButton(L"B3", FONT16ARIAL, FONT_BLACK, FONT_BLACK, VIEWER_RIGHT + 118, 60, 30, 24, MSYS_PRIORITY_HIGH, B3Callback);
	if( gfViewEnemies )
		iViewerButton[VIEW_ENEMIES]->uiFlags |= BUTTON_CLICKED_ON;
	else
		iViewerButton[VIEW_CREATURES]->uiFlags |= BUTTON_CLICKED_ON;

	MSYS_DefineRegion( &ViewerRegion, VIEWER_LEFT, VIEWER_TOP, VIEWER_RIGHT, VIEWER_BOTTOM, MSYS_PRIORITY_HIGH, 0,
		ViewerMapMoveCallback, ViewerMapClickCallback );

	//Add the enemy population override fields
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	StringFromValue(str, lengthof(str), gsAINumAdmins, MAX_STRATEGIC_TEAM_SIZE);
	AddTextInputField( 10, VIEWER_BOTTOM + 30, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	StringFromValue(str, lengthof(str), gsAINumTroops, MAX_STRATEGIC_TEAM_SIZE);
	AddTextInputField( 10, VIEWER_BOTTOM + 50, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	StringFromValue(str, lengthof(str), gsAINumElites, MAX_STRATEGIC_TEAM_SIZE);
	AddTextInputField( 10, VIEWER_BOTTOM + 70, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );
	StringFromValue(str, lengthof(str), gsAINumCreatures, MAX_STRATEGIC_TEAM_SIZE);
	AddTextInputField( 10, VIEWER_BOTTOM + 90, 25, 15, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );

	//Press buttons in based on current settings
	Assert( gGameOptions.ubDifficultyLevel >= DIF_LEVEL_EASY && gGameOptions.ubDifficultyLevel <= DIF_LEVEL_HARD );
	iViewerButton[RESET_EASY + gGameOptions.ubDifficultyLevel - DIF_LEVEL_EASY]->uiFlags |= BUTTON_CLICKED_ON;
	iViewerButton[COMPRESSION0]->uiFlags |= BUTTON_CLICKED_ON;
	if( !GamePaused() )
		SetGameMinutesPerSecond( 0 );
	ClearViewerRegion(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	return TRUE;
}


static void DestroyAIViewer()
{
	gfExitViewer  = FALSE;
	gfViewerEntry = TRUE;
	FOR_EACH(GUIButtonRef, i, iViewerButton) RemoveButton(*i);
	DeleteVideoObject(guiMapGraphicID);
	DeleteVideoObject(guiMapIconsID);
	MSYS_RemoveRegion(&ViewerRegion);

	KillTextInputMode();

	SetGameTimeCompressionLevel(giSaveTCMode);
	EnableScrollMessages();
}


static void ClearViewerRegion(INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom)
{
	ColorFillVideoSurfaceArea( ButtonDestBuffer, sLeft, sTop, sRight, sBottom, gusBlue );
	InvalidateRegion( sLeft, sTop, sRight, sBottom );

	if( !sLeft )
	{
		ColorFillVideoSurfaceArea( ButtonDestBuffer, 0, sTop, 1, sBottom, gusLtBlue );
		sLeft++;
	}
	if( !sTop )
	{
		ColorFillVideoSurfaceArea( ButtonDestBuffer, sLeft, 0, sRight, 1, gusLtBlue );
		sTop++;
	}
	if (sBottom == SCREEN_HEIGHT)
	{
		ColorFillVideoSurfaceArea(ButtonDestBuffer, sLeft, SCREEN_HEIGHT - 1, sRight, SCREEN_HEIGHT, gusDkBlue);
	}
	if (sRight == SCREEN_WIDTH)
	{
		ColorFillVideoSurfaceArea(ButtonDestBuffer, SCREEN_WIDTH - 1, sTop, SCREEN_WIDTH, sBottom, gusDkBlue);
	}
}


static void BlitGroupIcon(IconType, IconColour, UINT32 uiX, UINT32 uiY, SGPVObject const*);
static IconColour ChooseEnemyIconColor(UINT8 ubAdmins, UINT8 ubTroops, UINT8 ubElites);


static void RenderStationaryGroups()
{
	SetFont(FONT10ARIAL);
	SetFontShadow(FONT_NEARBLACK);

	// Render groups that are stationary
	SGPVObject const* const icons = guiMapIconsID;
	for (INT32 y = 0; y != 16; ++y)
	{
		INT32 const yp = VIEWER_TOP + VIEWER_CELLH * y + 1;
		for (INT32 x = 0; x != 16; ++x)
		{
			INT32      const  xp = VIEWER_LEFT + VIEWER_CELLW * x + 1;
			SECTORINFO const& si = SectorInfo[SECTOR(x + 1, y + 1)];

			if (si.uiFlags & SF_MINING_SITE) BltVideoObject(FRAME_BUFFER, icons, MINING_ICON, xp + 25, yp - 1);
			if (si.uiFlags & SF_SAM_SITE)    BltVideoObject(FRAME_BUFFER, icons, SAM_ICON,    xp + 20, yp + 4);

			IconColour icon_colour;
			UINT8      text_colour = FONT_YELLOW;
			UINT8      n           = si.ubNumberOfCivsAtLevel[0] + si.ubNumberOfCivsAtLevel[1] + si.ubNumberOfCivsAtLevel[2];
			if (n != 0)
			{ // Show militia
				icon_colour = ICON_COLOR_BLUE;
			}
			else
			{ // Show enemies
				n = si.ubNumAdmins + si.ubNumTroops + si.ubNumElites;
				if (n == 0) continue;

				icon_colour = ChooseEnemyIconColor(si.ubNumAdmins, si.ubNumTroops, si.ubNumElites);
				if (si.ubGarrisonID == NO_GARRISON)
				{
					if (GetJA2Clock() % 1000 < 333)
					{
						text_colour = FONT_LTKHAKI;
					}
				}
				else if (gGarrisonGroup[si.ubGarrisonID].ubPendingGroupID != 0)
				{
					if (GetJA2Clock() % 1000 < 333)
					{
						text_colour = FONT_LTRED;
					}
				}
			}
			SetFontForeground(text_colour);
			BlitGroupIcon(ICON_TYPE_STOPPED, icon_colour, xp, yp, icons);
			mprintf(xp + 2, yp + 2, L"%d", n);
		}
	}
}


static void RenderMovingGroupsAndMercs(void)
{
	INT32 x, y;
	UINT8 ubNumTroops, ubNumAdmins, ubNumElites;
	float ratio;
	INT32 minX, maxX, minY, maxY;
	UINT8 ubFontColor;


	SetFont( FONT10ARIAL );
	SetFontShadow( FONT_NEARBLACK );

	//Render groups that are moving...
	CFOR_EACH_GROUP(pGroup)
	{
		if( pGroup->ubGroupSize && !pGroup->fVehicle)
		{
			if( pGroup->uiTraverseTime )
			{
				// display how far along to the next sector they are
				ratio = (pGroup->uiTraverseTime-pGroup->uiArrivalTime + GetWorldTotalMin())/(float)pGroup->uiTraverseTime;
				minX = VIEWER_LEFT + VIEWER_CELLW * (pGroup->ubSectorX-1);
				maxX = VIEWER_LEFT + VIEWER_CELLW * (pGroup->ubNextX-1);
				x = (UINT32)(minX + ratio *(maxX-minX));
				minY = VIEWER_TOP + VIEWER_CELLH * (pGroup->ubSectorY-1);
				maxY = VIEWER_TOP + VIEWER_CELLH * (pGroup->ubNextY-1);
				y = (UINT32)(minY + ratio *(maxY-minY));
			}
			else
			{
				x = VIEWER_LEFT + VIEWER_CELLW * (pGroup->ubSectorX-1);
				y = VIEWER_TOP + VIEWER_CELLH * (pGroup->ubSectorY-1);
			}

			IconType   ubIconType;
			IconColour ubIconColor;
			if( pGroup->fPlayer )
			{
				ubIconType = ( pGroup->uiTraverseTime ) ? ICON_TYPE_ASSAULT : ICON_TYPE_STOPPED;
				ubIconColor = ICON_COLOR_GREEN;
				ubFontColor = FONT_YELLOW;
			}
			else
			{
				//if the group was moving, then draw the anchor to visually indicate the sector of
				//influence for enemy patrol groups.
				if( pGroup->uiTraverseTime )
				{
					BltVideoObject( FRAME_BUFFER, guiMapIconsID, GROUP_ANCHOR, VIEWER_LEFT + VIEWER_CELLW * (pGroup->ubSectorX - 1), VIEWER_TOP + VIEWER_CELLH * (pGroup->ubSectorY - 1));
				}

				ubNumAdmins = pGroup->pEnemyGroup->ubNumAdmins;//+ pGroup->pEnemyGroup->ubAdminsInBattle;
				ubNumTroops = pGroup->pEnemyGroup->ubNumTroops;//+ pGroup->pEnemyGroup->ubTroopsInBattle;
				ubNumElites = pGroup->pEnemyGroup->ubNumElites;// + pGroup->pEnemyGroup->ubElitesInBattle;

				// must have one of the three, already checked groupsize!
				Assert( ubNumAdmins || ubNumTroops || ubNumElites );

				//determine icon color
				ubIconColor = ChooseEnemyIconColor( ubNumAdmins, ubNumTroops, ubNumElites );

				// must have a valid intention
				Assert( pGroup->pEnemyGroup->ubIntention < NUM_ENEMY_INTENTIONS );

				// determine icon type - shows the groups intentions
				switch ( pGroup->pEnemyGroup->ubIntention )
				{
					case REINFORCEMENTS:	ubIconType = ICON_TYPE_REINFORCEMENT;	ubFontColor = FONT_YELLOW;	break;
					case PATROL:					ubIconType = ICON_TYPE_PATROL;				ubFontColor = FONT_YELLOW;	break;
					case STAGING:					ubIconType = ICON_TYPE_PATROL;				ubFontColor = FONT_LTBLUE;	break;
					case PURSUIT:					ubIconType = ICON_TYPE_ASSAULT;				ubFontColor = FONT_YELLOW;	break;
					case ASSAULT:					ubIconType = ICON_TYPE_ASSAULT;				ubFontColor = FONT_LTBLUE;	break;

					default:
						Assert(0);
						return;
				}
			}

			// draw the icon
			BlitGroupIcon(ubIconType, ubIconColor, x, y, guiMapIconsID);

			// set color
			SetFontForeground( ubFontColor );

			//Print the group size
			if( pGroup->fPlayer && !pGroup->uiTraverseTime )
				// stationary player - count all mercs there, not just the group
				mprintf( x+11, y+11, L"%d", PlayerMercsInSector( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ ) );
			else
			{
				if( !pGroup->pWaypoints )
				{
					if( GetJA2Clock() % 1000 < 750 )
					{
						SetFontForeground( FONT_WHITE );
					}
				}
				mprintf( x+7, y+7, L"%d", pGroup->ubGroupSize );
			}
		}
	}
}


static void PrintDetailedEnemiesInSectorInfo(INT32 iScreenX, INT32 iScreenY, UINT8 ubSectorX, UINT8 ubSectorY);
static void PrintEnemiesKilledTable(void);
static void PrintEnemyPopTable(void);


static void RenderInfoInSector(void)
{
	UINT8 ubSectorX, ubSectorY;
	UINT8 ubMercs=0, ubActive=0, ubUnconcious=0, ubCollapsed=0;

	if( gfViewEnemies && !gbViewLevel )
	{
		RenderStationaryGroups();
		RenderMovingGroupsAndMercs();
		SetFontForeground( FONT_LTRED );
		mprintf( 78, 358, L"%3d", giReinforcementPool );

		//Render general enemy statistics
		ClearViewerRegion( 105, VIEWER_BOTTOM + 10, 265, VIEWER_BOTTOM + 66 );
		SetFontForeground( FONT_YELLOW );
		MPrint( 105, VIEWER_BOTTOM + 10, L"GLOBAL INFO");
		SetFontForeground( FONT_GRAY2 );
		MPrint( 118, VIEWER_BOTTOM + 24, L"Strategic AI Awake");
		mprintf( 105, VIEWER_BOTTOM + 36, L"Total Request Points:  %d", giRequestPoints );
		mprintf( 105, VIEWER_BOTTOM + 46, L"Total Reinforcement Points:  %d", giReinforcementPoints );
		mprintf( 105, VIEWER_BOTTOM + 56, L"Progress (Current/Highest): %d%%/%d%%", CurrentPlayerProgressPercentage(), HighestPlayerProgressPercentage() );

		PrintEnemyPopTable();
		PrintEnemiesKilledTable();
	}

	if( gsHiSectorX && gsHiSectorY && ( gsHiSectorX != gsSelSectorX || gsHiSectorY != gsSelSectorY ) )
	{ //Render sector info for the hilighted sector
		SetFontForeground( FONT_YELLOW );
		ubSectorX = (UINT8)gsHiSectorX;
		ubSectorY = (UINT8)gsHiSectorY;
	}
	else if( gsSelSectorX && gsSelSectorY )
	{ //Render sector info for the selected sector
		SetFontForeground( FONT_RED );
		ubSectorX = (UINT8)gsSelSectorX;
		ubSectorY = (UINT8)gsSelSectorY;
	}
	else
	{
		return;
	}

	//Count the number of mercs and their states (even for underground sectors)
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		if (s->sSectorX == ubSectorX &&
				s->sSectorY == ubSectorY &&
				s->bSectorZ == gbViewLevel &&
				s->bLife != 0)
		{
			ubMercs++;
			if (s->bLife >= OKLIFE)
			{
				if (s->bBreath < OKBREATH)
				{
					ubCollapsed++;
				}
				else
				{
					ubActive++;
				}
			}
			else
			{
				ubUnconcious++;
			}
		}
	}

	INT32 yp = 375;
	if( !gbViewLevel )
	{
		SECTORINFO *pSector;
		UINT8 ubNumAdmins=0, ubNumTroops=0, ubNumElites=0, ubAdminsInBattle=0, ubTroopsInBattle=0, ubElitesInBattle=0, ubNumGroups=0;

		pSector = &SectorInfo[ SECTOR( ubSectorX, ubSectorY ) ];

		//Now count the number of mobile groups in the sector.
		CFOR_EACH_ENEMY_GROUP(pGroup)
		{
			if (!pGroup->fVehicle &&
					pGroup->ubSectorX == ubSectorX &&
					pGroup->ubSectorY == ubSectorY)
			{
				ubNumTroops += pGroup->pEnemyGroup->ubNumTroops;
				ubNumElites += pGroup->pEnemyGroup->ubNumElites;
				ubNumAdmins += pGroup->pEnemyGroup->ubNumAdmins;
				ubTroopsInBattle += pGroup->pEnemyGroup->ubTroopsInBattle;
				ubElitesInBattle += pGroup->pEnemyGroup->ubElitesInBattle;
				ubAdminsInBattle += pGroup->pEnemyGroup->ubAdminsInBattle;
				ubNumGroups++;
			}
		}
		ClearViewerRegion(280, 375, SCREEN_WIDTH, SCREEN_HEIGHT);
		mprintf( 280, yp, L"SECTOR INFO:  %c%d  (ID: %d)", ubSectorY + 'A' - 1, ubSectorX, SECTOR( ubSectorX, ubSectorY ) );
		yp += 10;
		SetFontForeground( FONT_LTGREEN );
		mprintf( 280, yp, L"%d Player Mercs:  (%d Active, %d Unconcious, %d Collapsed)",
			ubMercs, ubActive, ubUnconcious, ubCollapsed );
		yp += 10;
		SetFontForeground( FONT_LTBLUE );
		mprintf( 280, yp, L"Militia:  (%d Green, %d Regular, %d Elite)",
			pSector->ubNumberOfCivsAtLevel[0], pSector->ubNumberOfCivsAtLevel[1], pSector->ubNumberOfCivsAtLevel[2] );
		yp += 10;
		SetFontForeground( FONT_ORANGE );
		mprintf( 280, yp, L"Garrison:  (%d:%d Admins, %d:%d Troops, %d:%d Elites)",
			pSector->ubAdminsInBattle, pSector->ubNumAdmins,
			pSector->ubTroopsInBattle, pSector->ubNumTroops,
			pSector->ubElitesInBattle, pSector->ubNumElites );
		yp += 10;
		mprintf( 280, yp, L"%d Groups:  (%d:%d Admins, %d:%d Troops, %d:%d Elites)", ubNumGroups,
			ubAdminsInBattle, ubNumAdmins,
			ubTroopsInBattle, ubNumTroops,
			ubElitesInBattle, ubNumElites );
		yp += 10;
		SetFontForeground( FONT_WHITE );

		if( gfViewEnemies )
		{
			PrintDetailedEnemiesInSectorInfo( 280, yp, ubSectorX, ubSectorY );
			yp += 10;
		}
		else
		{
			SetFontForeground( FONT_YELLOW );
			mprintf( 280, yp, L"Monsters:  (%d:%d)", pSector->ubCreaturesInBattle, pSector->ubNumCreatures );
			yp += 10;
		}
	}
	else
	{
		UNDERGROUND_SECTORINFO *pSector;
		ClearViewerRegion(280, 375, SCREEN_WIDTH, SCREEN_HEIGHT);
		pSector = FindUnderGroundSector( ubSectorX, ubSectorY, gbViewLevel );
		if( !pSector )
		{
			return;
		}
		mprintf( 280, yp, L"SECTOR INFO:  %c%d_b%d", ubSectorY + 'A' - 1, ubSectorX, gbViewLevel );
		yp += 10;
		SetFontForeground( FONT_LTGREEN );
		mprintf( 280, yp, L"%d Player Mercs:  (%d Active, %d Unconcious, %d Collapsed)",
			ubMercs, ubActive, ubUnconcious, ubCollapsed );
		yp += 10;
		SetFontForeground( FONT_YELLOW );
		mprintf( 280, yp, L"Monsters:  (%d:%d)", pSector->ubCreaturesInBattle, pSector->ubNumCreatures );
		yp += 10;
		if( pSector->uiFlags & SF_PENDING_ALTERNATE_MAP )
		{
			MPrint(280, yp, L"SF_PENDING_ALTERNATE_MAP");
			yp += 10;
		}
		if( pSector->uiFlags & SF_USE_ALTERNATE_MAP )
		{
			MPrint(280, yp, L"SF_USE_ALTERNATE_MAP");
			yp += 10;
		}
	}
}


static void RenderViewer(void)
{
	INT32 i, x, y, xp, yp;
	if( gfRenderViewer )
	{
		ClearViewerRegion( VIEWER_LEFT, VIEWER_TOP, VIEWER_RIGHT, VIEWER_BOTTOM );

		gfRenderViewer = FALSE;
		gfRenderMap = TRUE;
		for( i = 0; i < NUM_VIEWER_BUTTONS; i++ )
		{
			MarkAButtonDirty( iViewerButton[ i ] );
		}

		SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
		MPrint(38, VIEWER_BOTTOM + 33, L"Admins");
		MPrint(38, VIEWER_BOTTOM + 53, L"Troops");
		MPrint(38, VIEWER_BOTTOM + 73, L"Elites");
		MPrint(38, VIEWER_BOTTOM + 93, L"Creatures");
	}

	if( gfRenderMap )
	{
		gfRenderMap = FALSE;
		BltVideoObject(FRAME_BUFFER, guiMapGraphicID, 0, VIEWER_LEFT, VIEWER_TOP);
		InvalidateRegion( VIEWER_LEFT, VIEWER_TOP, VIEWER_RIGHT, VIEWER_BOTTOM );
		//Draw the coordinates
		ClearViewerRegion( 0, 0, VIEWER_RIGHT, 14 );
		ClearViewerRegion( 0, 0, 14, VIEWER_BOTTOM );
		SetFont( FONT10ARIAL );
		SetFontShadow( FONT_NEARBLACK );
		for( y = 0; y < 16; y++ )
		{
			if( y+1 == gsSelSectorY )
				SetFontForeground( FONT_RED );
			else if( y+1 == gsHiSectorY )
				SetFontForeground( FONT_YELLOW );
			else
				SetFontForeground( FONT_GRAY1 );
			mprintf( VIEWER_LEFT-10, VIEWER_TOP+7+y*22, L"%c", 'A' + y );
		}
		for( x = 1; x <= 16; x++ )
		{
			wchar_t str[3];
			if( x == gsSelSectorX )
				SetFontForeground( FONT_RED );
			else if( x == gsHiSectorX )
				SetFontForeground( FONT_YELLOW );
			else
				SetFontForeground( FONT_GRAY1 );
			swprintf(str, lengthof(str), L"%d", x);
			MPrint(VIEWER_LEFT + x * 26 - (26 + StringPixLength(str, FONT12POINT1)) / 2, VIEWER_TOP - 12, str);
		}
		if( gbViewLevel )
		{
			UNDERGROUND_SECTORINFO *pUnder;
			SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
			for( y = 0; y < 16; y++ )
			{
				SGPRect ClipRect;
				ClipRect.iTop = VIEWER_TOP + y*VIEWER_CELLH;
				ClipRect.iBottom = ClipRect.iTop + VIEWER_CELLH - 1;
				for( x = 0; x < 16; x++ )
				{
					pUnder = FindUnderGroundSector( (INT16)(x + 1), (INT16)(y + 1), gbViewLevel );
					if( pUnder )
					{
						xp = VIEWER_LEFT+x*VIEWER_CELLW+2;
						yp = VIEWER_TOP+y*VIEWER_CELLH+2;
						mprintf( xp, yp, L"%d", pUnder->ubNumCreatures );
					}
					else
					{ //not found, so visually shade it darker.
						ClipRect.iLeft = VIEWER_LEFT + x*VIEWER_CELLW;
						ClipRect.iRight = ClipRect.iLeft + VIEWER_CELLW - 1;
						FRAME_BUFFER->ShadowRect(ClipRect.iLeft, ClipRect.iTop, ClipRect.iRight, ClipRect.iBottom);
						FRAME_BUFFER->ShadowRect(ClipRect.iLeft, ClipRect.iTop, ClipRect.iRight, ClipRect.iBottom);
					}
				}
			}
		}
		RenderInfoInSector();
	}

	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf = l.Buffer<UINT16>();
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	//Render the grid for the sector if the mouse is over it (yellow).
	if( gsHiSectorX > 0 )
	{
		x = VIEWER_LEFT + (gsHiSectorX-1) * 26;
		y = VIEWER_TOP + (gsHiSectorY-1) * 22;
		RectangleDraw( TRUE, x, y, x+26, y+22, Get16BPPColor( FROMRGB( 200, 200, 50 ) ), pDestBuf );
	}
	//Render the grid for the sector currently in focus (red).
	if( gsSelSectorX > 0 )
	{
		x = VIEWER_LEFT + (gsSelSectorX-1) * 26 ;
		y = VIEWER_TOP + (gsSelSectorY-1) * 22 ;
		RectangleDraw( TRUE, x, y, x+26, y+22, Get16BPPColor( FROMRGB( 200, 50, 50 ) ), pDestBuf );
	}
}


static void ViewerExitCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfExitViewer = TRUE;
	}
}


static void ExtractAndUpdatePopulations(void);


static void HandleViewerInput(void)
{
	SECTORINFO *pSector;
	InputAtom Event;
	while( DequeueEvent( &Event ) )
	{
		if( !HandleTextInput( &Event ) && Event.usEvent == KEY_DOWN )
		{
			switch( Event.usParam )
			{
				case SDLK_ESCAPE: gfExitViewer = TRUE; break;

				case SDLK_RETURN:
					// this means GO! for doing overrides
					ExtractAndUpdatePopulations();
					break;
				case 'x':
					if( Event.usKeyState & ALT_DOWN )
					{
						gfExitViewer = TRUE;
						gfProgramIsRunning = FALSE;
					}
					break;
				case 'm':
					//Kill all enemies and add militia
					if( Event.usKeyState & ALT_DOWN )
					{
						pSector = NULL;
						if( gsSelSectorX && gsSelSectorY )
						{
							pSector = &SectorInfo[ SECTOR( gsSelSectorX, gsSelSectorY ) ];
							pSector->ubNumberOfCivsAtLevel[0] = 15;
							pSector->ubNumberOfCivsAtLevel[1] = 4;
							pSector->ubNumberOfCivsAtLevel[2]	= 1;
							gfRenderMap = TRUE;
							EliminateAllEnemies( (UINT8)gsSelSectorX, (UINT8)gsSelSectorY );
						}
						else if( gsHiSectorX && gsHiSectorY )
						{
							pSector = &SectorInfo[ SECTOR( gsHiSectorX, gsHiSectorY ) ];
							pSector->ubNumberOfCivsAtLevel[0] = 15;
							pSector->ubNumberOfCivsAtLevel[1] = 4;
							pSector->ubNumberOfCivsAtLevel[2]	= 1;
							gfRenderMap = TRUE;
							EliminateAllEnemies( (UINT8)gsHiSectorX, (UINT8)gsHiSectorY );
						}
					}
					break;
				case 'o':
					// kill all enemies!
					if( Event.usKeyState & ALT_DOWN )
					{
						pSector = NULL;
						if( gsSelSectorX && gsSelSectorY )
						{
							gfRenderMap = TRUE;
							EliminateAllEnemies( (UINT8)gsSelSectorX, (UINT8)gsSelSectorY );
						}
						else if( gsHiSectorX && gsHiSectorY )
						{
							gfRenderMap = TRUE;
							EliminateAllEnemies( (UINT8)gsHiSectorX, (UINT8)gsHiSectorY );
						}
					}
					break;
				case 'g':
					//Add a group of 8 stationary enemies
					if( gsSelSectorX && gsSelSectorY )
					{
						pSector = &SectorInfo[ SECTOR( gsSelSectorX, gsSelSectorY ) ];
						pSector->ubNumElites += 1;
						pSector->ubNumTroops += 7;
					}
					else if( gsHiSectorX && gsHiSectorY )
					{
						pSector = &SectorInfo[ SECTOR( gsHiSectorX, gsHiSectorY ) ];
						pSector->ubNumElites += 1;
						pSector->ubNumTroops += 7;
					}
					break;
				case 'c':
					//Add a group of 8 creatures.
					if( gsSelSectorX && gsSelSectorY )
					{
						pSector = &SectorInfo[ SECTOR( gsSelSectorX, gsSelSectorY ) ];
						pSector->ubNumCreatures += 8;
					}
					else if( gsHiSectorX && gsHiSectorY )
					{
						pSector = &SectorInfo[ SECTOR( gsHiSectorX, gsHiSectorY ) ];
						pSector->ubNumCreatures += 8;
					}
					break;
			}
		}
	}
}


static void EasyCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[RESET_EASY        ]->uiFlags |=  BUTTON_CLICKED_ON;
		iViewerButton[RESET_NORMAL      ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[RESET_HARD        ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[QUEEN_AWAKE_TOGGLE]->uiFlags &= ~BUTTON_CLICKED_ON;
		gfRenderViewer = TRUE;
		gGameOptions.ubDifficultyLevel = DIF_LEVEL_EASY;
		ShutdownStrategicLayer();
		InitStrategicLayer();
		Compression0Callback(iViewerButton[COMPRESSION0], MSYS_CALLBACK_REASON_LBUTTON_UP);
		MarkButtonsDirty();
		WarpGameTime( 45 * 60, WARPTIME_PROCESS_EVENTS_NORMALLY );
	}
}


static void NormalCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[RESET_EASY        ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[RESET_NORMAL      ]->uiFlags |=  BUTTON_CLICKED_ON;
		iViewerButton[RESET_HARD        ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[QUEEN_AWAKE_TOGGLE]->uiFlags &= ~BUTTON_CLICKED_ON;
		gfRenderViewer = TRUE;
		gGameOptions.ubDifficultyLevel = DIF_LEVEL_MEDIUM;
		ShutdownStrategicLayer();
		InitStrategicLayer();
		Compression0Callback(iViewerButton[COMPRESSION0], MSYS_CALLBACK_REASON_LBUTTON_UP);
		MarkButtonsDirty();
		WarpGameTime( 45 * 60, WARPTIME_PROCESS_EVENTS_NORMALLY );
	}
}


static void HardCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[RESET_EASY        ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[RESET_NORMAL      ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[RESET_HARD        ]->uiFlags |=  BUTTON_CLICKED_ON;
		iViewerButton[QUEEN_AWAKE_TOGGLE]->uiFlags &= ~BUTTON_CLICKED_ON;
		gfRenderViewer = TRUE;
		gGameOptions.ubDifficultyLevel = DIF_LEVEL_HARD;
		ShutdownStrategicLayer();
		InitStrategicLayer();
		Compression0Callback(iViewerButton[COMPRESSION0], MSYS_CALLBACK_REASON_LBUTTON_UP);
		MarkButtonsDirty();
		WarpGameTime( 45 * 60, WARPTIME_PROCESS_EVENTS_NORMALLY );
	}
}


static void ViewerMapMoveCallback(MOUSE_REGION* reg, INT32 reason)
{
	static INT16 gsPrevX = 0, gsPrevY = 0;
	//calc current sector highlighted.
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gsPrevX = gsHiSectorX = 0;
		gsPrevY = gsHiSectorY = 0;
		gfRenderViewer = TRUE;
		return;
	}
	gsHiSectorX = MIN( (reg->RelativeXPos / 26) + 1, 16 );
	gsHiSectorY = MIN( (reg->RelativeYPos / 22) + 1, 16 );
	if( gsPrevX != gsHiSectorX || gsPrevY != gsHiSectorY )
	{
		gsPrevX = gsHiSectorX;
		gsPrevY = gsHiSectorY;
		gfRenderViewer = TRUE;
	}
}


static void ViewerMapClickCallback(MOUSE_REGION* reg, INT32 reason)
{
	static INT16 sLastX = -1, sLastY = -1;
	//calc current sector selected.
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gsSelSectorX = MIN( (reg->RelativeXPos / 26) + 1, 16 );
		gsSelSectorY = MIN( (reg->RelativeYPos / 22) + 1, 16 );
		if( gsSelSectorX != sLastX || gsSelSectorY != sLastY )
		{ //clicked in a new sector
			sLastX = gsSelSectorX;
			sLastY = gsSelSectorY;
			gfRenderViewer = TRUE;
		}
	}
}


void AIViewerScreenInit()
{
	gfViewerEntry = TRUE;
	gusBlue		= Get16BPPColor( FROMRGB(  65,  79,  94 ) );
	gusLtBlue = Get16BPPColor( FROMRGB( 122, 124, 121 ) );
	gusDkBlue = Get16BPPColor( FROMRGB(  22,  55,  73 ) );
}


ScreenID AIViewerScreenHandle()
{
	RestoreBackgroundRects();

	if( gfViewerEntry )
	{
		gfViewerEntry = FALSE;
		if( !CreateAIViewer() )
		{
			return MAP_SCREEN;
		}
	}

	if( GetWorldTotalSeconds() != guiLastTime )
	{
		guiLastTime = GetWorldTotalSeconds();
		gfRenderViewer = TRUE;
		iViewerButton[VIEWER_TIMEPANEL]->SpecifyText(WORLDTIMESTR);
	}

	HandleViewerInput();
	RenderViewer();

	RenderAllTextFields();
	RenderButtons();

	SaveBackgroundRects( );

	RenderButtonsFastHelp();

	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();

	if( gfExitViewer )
	{
		DestroyAIViewer();
		return MAP_SCREEN;
	}
	return AIVIEWER_SCREEN;
}


static void Compression0Callback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[COMPRESSION0 ]->uiFlags |=  BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION5 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION15]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION60]->uiFlags &= ~BUTTON_CLICKED_ON;
//		iViewerButton[COMPRESSION6H]->uiFlags &= ~BUTTON_CLICKED_ON;
		SetGameMinutesPerSecond( 0 );
	}
}


static void Compression5Callback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[COMPRESSION0 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION5 ]->uiFlags |=  BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION15]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION60]->uiFlags &= ~BUTTON_CLICKED_ON;
//		iViewerButton[COMPRESSION6H]->uiFlags &= ~BUTTON_CLICKED_ON;
		SetGameMinutesPerSecond( 5 );
		SetFactTimeCompressHasOccured( );
	}
}


static void Compression15Callback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[COMPRESSION0 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION5 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION15]->uiFlags |=  BUTTON_CLICKED_ON;
		iViewerButton[COMPRESSION60]->uiFlags &= ~BUTTON_CLICKED_ON;
//		iViewerButton[COMPRESSION6H]->uiFlags &= ~BUTTON_CLICKED_ON;
		SetGameMinutesPerSecond( 15 );
		SetFactTimeCompressHasOccured( );
	}
}


static void Compression60Callback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[ COMPRESSION0 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[ COMPRESSION5 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[ COMPRESSION15]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[ COMPRESSION60]->uiFlags |=  BUTTON_CLICKED_ON;
//		iViewerButton[COMPRESSION6H]->uiFlags &= ~BUTTON_CLICKED_ON;
		SetGameHoursPerSecond( 1 );
		SetFactTimeCompressHasOccured( );
	}
}


static void Compression6HCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iViewerButton[ COMPRESSION0 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[ COMPRESSION5 ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[ COMPRESSION15]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[ COMPRESSION60]->uiFlags &= ~BUTTON_CLICKED_ON;
//		iViewerButton[COMPRESSION6H]->uiFlags |=  BUTTON_CLICKED_ON;
		SetGameHoursPerSecond( 6 );
		SetFactTimeCompressHasOccured( );
	}
}


static void TestIncoming4SidesCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		GROUP *pGroup;
		UINT8 ubSector;
		UINT32 uiWorldMin;
		Compression0Callback(iViewerButton[COMPRESSION0], MSYS_CALLBACK_REASON_LBUTTON_UP);
		if( ( gsSelSectorX == 0 ) || ( gsSelSectorY == 0 ) )
			gsSelSectorX = 9, gsSelSectorY = 1;

		ubSector = SECTOR( gsSelSectorX, gsSelSectorY );
		uiWorldMin = GetWorldTotalMin();
		gfRenderViewer = TRUE;
		if( gsSelSectorY > 1 )
		{
			pGroup = CreateNewEnemyGroupDepartingFromSector( ubSector-16, 0, 11, 5 );
			pGroup->ubNextX = (UINT8)gsSelSectorX;
			pGroup->ubNextY = (UINT8)gsSelSectorY;
			pGroup->uiTraverseTime = 10;
			pGroup->pEnemyGroup->ubIntention = ASSAULT;
			SetGroupArrivalTime(*pGroup, uiWorldMin + 10);
			pGroup->ubMoveType = ONE_WAY;
			pGroup->fDebugGroup = TRUE;
			AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );
		}
		if( gsSelSectorY < 16 )
		{
			pGroup = CreateNewEnemyGroupDepartingFromSector( ubSector+16, 0, 8, 8 );
			pGroup->ubNextX = (UINT8)gsSelSectorX;
			pGroup->ubNextY = (UINT8)gsSelSectorY;
			pGroup->uiTraverseTime = 12;
			pGroup->pEnemyGroup->ubIntention = ASSAULT;
			SetGroupArrivalTime(*pGroup, uiWorldMin + 12);
			pGroup->ubMoveType = ONE_WAY;
			pGroup->fDebugGroup = TRUE;
			AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );
		}
		if( gsSelSectorX > 1 )
		{
			pGroup = CreateNewEnemyGroupDepartingFromSector( ubSector-1, 0, 11, 5 );
			pGroup->ubNextX = (UINT8)gsSelSectorX;
			pGroup->ubNextY = (UINT8)gsSelSectorY;
			pGroup->uiTraverseTime = 11;
			pGroup->pEnemyGroup->ubIntention = ASSAULT;
			SetGroupArrivalTime(*pGroup, uiWorldMin + 11);
			pGroup->ubMoveType = ONE_WAY;
			pGroup->fDebugGroup = TRUE;
			AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );
		}
		if( gsSelSectorX < 16 )
		{
			pGroup = CreateNewEnemyGroupDepartingFromSector( ubSector+1, 0, 14, 0 );
			pGroup->ubNextX = (UINT8)gsSelSectorX;
			pGroup->ubNextY = (UINT8)gsSelSectorY;
			pGroup->uiTraverseTime = 13;
			pGroup->pEnemyGroup->ubIntention = ASSAULT;
			SetGroupArrivalTime(*pGroup, uiWorldMin + 13);
			pGroup->ubMoveType = ONE_WAY;
			pGroup->fDebugGroup = TRUE;
			AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );
		}
	}
}


static void StartCreatureQuestCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gGameOptions.fSciFi = TRUE;
		gfRenderMap = TRUE;
		ClearCreatureQuest();
		InitCreatureQuest();
	}
}


static void SpreadCreaturesCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMap = TRUE;
		if( _KeyDown( ALT ) )
		{
			INT32 i;
			// spread 10 times
			for( i = 0; i < 10; i++ )
			{
				SpreadCreatures();
			}
		}
		SpreadCreatures();
	}
}


static void CreatureAttackCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( ( gsSelSectorX != 0 ) && ( gsSelSectorX != 0 ) )
		{
			if( _KeyDown( ALT ) )
			{
				AddStrategicEventUsingSeconds( EVENT_CREATURE_ATTACK, GetWorldTotalSeconds()+4, SECTOR( gsSelSectorX, gsSelSectorY ) );
			}
			else
			{
				CreatureAttackTown( (UINT8)SECTOR( gsSelSectorX, gsSelSectorY ), TRUE );
			}
		}
	}
}


static void ViewEnemiesCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfViewEnemies = TRUE;
		gfRenderMap = TRUE;
		iViewerButton[VIEW_CREATURES]->uiFlags &= ~BUTTON_CLICKED_ON;
		gbViewLevel = 0;
		iViewerButton[BASEMENT1_BTN ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[BASEMENT2_BTN ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[BASEMENT3_BTN ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


static void ViewCreaturesCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfViewEnemies = FALSE;
		gfRenderMap = TRUE;
		iViewerButton[VIEW_ENEMIES]->uiFlags &= ~BUTTON_CLICKED_ON;
		if( !gbViewLevel )
		{
			gbViewLevel = 1;
			iViewerButton[BASEMENT1_BTN]->uiFlags |=  BUTTON_CLICKED_ON;
			iViewerButton[BASEMENT2_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
			iViewerButton[BASEMENT3_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
	}
}


static void B1Callback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMap = TRUE;
		gbViewLevel = 1;
		iViewerButton[BASEMENT2_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[BASEMENT3_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


static void B2Callback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMap = TRUE;
		gbViewLevel = 2;
		iViewerButton[BASEMENT1_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[BASEMENT3_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


static void B3Callback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMap = TRUE;
		gbViewLevel = 3;
		iViewerButton[BASEMENT1_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
		iViewerButton[BASEMENT2_BTN]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


static void ReloadSectorCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfOverrideSector = TRUE;
		ExtractAndUpdatePopulations();
		SetCurrentWorldSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		gfOverrideSector = FALSE;
		DestroyAIViewer();
	}
}


static void ExtractAndUpdatePopulations(void)
{
	gsAINumAdmins = MIN( GetNumericStrictValueFromField( 0 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 0, gsAINumAdmins );

	gsAINumTroops = MIN( GetNumericStrictValueFromField( 1 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 1, gsAINumTroops );

	gsAINumElites = MIN( GetNumericStrictValueFromField( 2 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 2, gsAINumElites );

	gsAINumCreatures = MIN( GetNumericStrictValueFromField( 3 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 3, gsAINumCreatures );
}


static void ToggleQueenAwake(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->Clicked())
		{
			WakeUpQueen();
		}
		else
		{
			gfQueenAIAwake = FALSE;
		}
	}
}



enum
{
	ENEMY_RANK_TOTAL = NUM_ENEMY_RANKS,
	ENEMY_RANK_PERCENT,
	TABLE_ENEMY_RANKS
};

static const wchar_t *EnemyRankString[TABLE_ENEMY_RANKS] =
{
	L"Adm",
	L"Trp",
	L"Elt",
	L"TOT",
	L"%%"
};


enum
{
	ENEMY_TYPE_POOL,
	ENEMY_TYPE_GARRISON,
	ENEMY_TYPE_PATROL,
	ENEMY_TYPE_REINFORCEMENTS,
	ENEMY_TYPE_ASSAULT,
	ENEMY_TYPE_STAGING,
	ENEMY_TYPE_RETREATING,
	ENEMY_TYPE_TOTAL,
	ENEMY_TYPE_PERCENT,
	POP_TABLE_ENEMY_TYPES
};

static const wchar_t *EnemyTypeString[POP_TABLE_ENEMY_TYPES] =
{
	L"Pool",
	L"Garr",
	L"Ptrl",
	L"Rein",
	L"Aslt",
	L"Stag",
	L"Rtrt",
	L" TOT",
	L"   %%",
};


#define POP_TABLE_X_OFFSET	30
#define POP_TABLE_X_GAP			30
#define POP_TABLE_Y_GAP			10


static void PrintEnemyPopTable(void)
{
	UINT16 usX, usY;
	UINT16 usEnemyPopTable[ TABLE_ENEMY_RANKS ][ POP_TABLE_ENEMY_TYPES ];
	UINT32 uiSector = 0;
	UINT8		ubEnemyRank;
	UINT8		ubEnemyType;
	SECTORINFO *pSector;
	wchar_t wTempString[10];


	memset( &usEnemyPopTable, 0, sizeof ( usEnemyPopTable ) );

	// count how many enemies of each type & rank there are

	// this is quite inaccurate, since elites can also come from the pool
	usEnemyPopTable[ ENEMY_RANK_TROOP ][ ENEMY_TYPE_POOL ] += ( UINT16 ) giReinforcementPool;

	// count stationary enemies (garrisons)
	for ( uiSector = 0; uiSector < 256; uiSector++ )
	{
		pSector = &SectorInfo[ uiSector ];

		usEnemyPopTable[ ENEMY_RANK_ADMIN ][ ENEMY_TYPE_GARRISON ] += pSector->ubNumAdmins;
		usEnemyPopTable[ ENEMY_RANK_TROOP ][ ENEMY_TYPE_GARRISON ] += pSector->ubNumTroops;
		usEnemyPopTable[ ENEMY_RANK_ELITE ][ ENEMY_TYPE_GARRISON ] += pSector->ubNumElites;
	}

	// count moving enemies
	CFOR_EACH_ENEMY_GROUP(pGroup)
	{
		if (!pGroup->fDebugGroup)
		{
			Assert( pGroup->pEnemyGroup != NULL );

			switch ( pGroup->pEnemyGroup->ubIntention )
			{
				case REINFORCEMENTS:	ubEnemyType = ENEMY_TYPE_REINFORCEMENTS;	break;
				case ASSAULT:					ubEnemyType = ENEMY_TYPE_ASSAULT;					break;
				case STAGING:					ubEnemyType = ENEMY_TYPE_STAGING;					break;
				case PATROL:					ubEnemyType = ENEMY_TYPE_PATROL;					break;
				case PURSUIT:					ubEnemyType = ENEMY_TYPE_ASSAULT;					break;

				default:
					AssertMsg( 0, String( "Unknown moving group intention %d", pGroup->pEnemyGroup->ubIntention ) );
					continue;
			}

			usEnemyPopTable[ ENEMY_RANK_ADMIN ][ ubEnemyType ] += pGroup->pEnemyGroup->ubNumAdmins;
			usEnemyPopTable[ ENEMY_RANK_TROOP ][ ubEnemyType ] += pGroup->pEnemyGroup->ubNumTroops;
			usEnemyPopTable[ ENEMY_RANK_ELITE ][ ubEnemyType ] += pGroup->pEnemyGroup->ubNumElites;
		}
	}


	// add up totals across rows (ranks) by type
	for ( ubEnemyType = 0; ubEnemyType < ENEMY_TYPE_TOTAL; ubEnemyType++ )
	{
		for ( ubEnemyRank = 0; ubEnemyRank < ENEMY_RANK_TOTAL; ubEnemyRank++ )
		{
			usEnemyPopTable[ ENEMY_RANK_TOTAL ][ ubEnemyType ] += usEnemyPopTable[ ubEnemyRank ][ ubEnemyType ];
		}
	}

	// add up totals by rank across columns (types)
	for ( ubEnemyRank = 0; ubEnemyRank <= ENEMY_RANK_TOTAL; ubEnemyRank++ )
	{
		for ( ubEnemyType = 0; ubEnemyType < ENEMY_TYPE_TOTAL; ubEnemyType++ )
		{
			usEnemyPopTable[ ubEnemyRank ][ ENEMY_TYPE_TOTAL ] += usEnemyPopTable[ ubEnemyRank ][ ubEnemyType ];
		}
	}


	// avoid division by zero
	if ( usEnemyPopTable[ ENEMY_RANK_TOTAL ][ ENEMY_TYPE_TOTAL ] > 0 )
	{
		// calculate rank percentages
		for ( ubEnemyRank = 0; ubEnemyRank < ENEMY_RANK_PERCENT; ubEnemyRank++ )
		{
			usEnemyPopTable[ ubEnemyRank ][ ENEMY_TYPE_PERCENT ] = ( ( 100 * usEnemyPopTable[ ubEnemyRank ][ ENEMY_TYPE_TOTAL ] ) /
																																 usEnemyPopTable[ ENEMY_RANK_TOTAL ][ ENEMY_TYPE_TOTAL ] );
		}

		// calculate type percentages
		for ( ubEnemyType = 0; ubEnemyType < ENEMY_TYPE_PERCENT; ubEnemyType++ )
		{
			usEnemyPopTable[ ENEMY_RANK_PERCENT ][ ubEnemyType ] = ( ( 100 * usEnemyPopTable[ ENEMY_RANK_TOTAL ][ ubEnemyType ] ) /
																																 usEnemyPopTable[ ENEMY_RANK_TOTAL ][ ENEMY_TYPE_TOTAL ] );
		}
	}

	usX = VIEWER_RIGHT + 10;
	usY = 200;

	// titles and headings mean 2 extra rows
	ClearViewerRegion(usX, usY, SCREEN_WIDTH, usY + POP_TABLE_Y_GAP * (POP_TABLE_ENEMY_TYPES + 2) + 11);

	// print table title
	SetFontForeground( FONT_RED );
	MPrint(usX, usY, L"ENEMY POPULATION:");
	usY += POP_TABLE_Y_GAP;

	// print horizontal labels
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		DrawTextToScreen(EnemyRankString[ubEnemyRank], usX + POP_TABLE_X_OFFSET + POP_TABLE_X_GAP * ubEnemyRank, usY, POP_TABLE_X_GAP, FONT10ARIAL, FONT_LTBLUE, 0, RIGHT_JUSTIFIED);
	}


	// print vertical labels
	for ( ubEnemyType = 0; ubEnemyType < POP_TABLE_ENEMY_TYPES; ubEnemyType++ )
	{
		DrawTextToScreen(EnemyTypeString[ubEnemyType], usX, usY + POP_TABLE_Y_GAP + POP_TABLE_Y_GAP * ubEnemyType, POP_TABLE_X_OFFSET, FONT10ARIAL, FONT_LTBLUE, 0, RIGHT_JUSTIFIED);
	}

	// over to first column, and down 1 line
	usY += POP_TABLE_Y_GAP;
	usX += POP_TABLE_X_OFFSET;

	// print table values
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		for ( ubEnemyType = 0; ubEnemyType < POP_TABLE_ENEMY_TYPES; ubEnemyType++ )
		{
			const wchar_t* PrintSpec;
			// an exclusive OR operator, how often do ya see that, huh?  :-)
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) ^ ( ubEnemyType == ENEMY_TYPE_PERCENT ) )
			{
				PrintSpec = L"%3d%%";
			}
			else
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) && ( ubEnemyType == ENEMY_TYPE_PERCENT ) )
			{
				PrintSpec = L"";
			}
			else
			{
				PrintSpec = L"%4d";
			}

			swprintf(wTempString, lengthof(wTempString), PrintSpec, usEnemyPopTable[ubEnemyRank][ubEnemyType]);
			DrawTextToScreen(wTempString, usX + POP_TABLE_X_GAP * ubEnemyRank, usY + POP_TABLE_Y_GAP * ubEnemyType, POP_TABLE_X_GAP, FONT10ARIAL, FONT_YELLOW, 0, RIGHT_JUSTIFIED);
		}
	}
}



enum
{
	ENEMIES_KILLED_IN_TACTICAL,
	ENEMIES_KILLED_IN_AUTO_RESOLVE,
	ENEMIES_KILLED_TOTAL,
	ENEMIES_KILLED_PERCENT,
	KILLED_TABLE_ROWS
};

static const wchar_t *EnemiesKilledString[KILLED_TABLE_ROWS] =
{
	L"Tact",
	L"Auto",
	L" TOT",
	L"   %%",
};


#define KILLED_TABLE_X_OFFSET		30
#define KILLED_TABLE_X_GAP			30
#define KILLED_TABLE_Y_GAP			10


static void PrintEnemiesKilledTable(void)
{
	UINT16 usX, usY;
	UINT16 usEnemiesKilledTable[ TABLE_ENEMY_RANKS ][ KILLED_TABLE_ROWS ];
	UINT8	ubEnemyRank;
	UINT8 ubKillType;
	wchar_t wTempString[10];


	memset( &usEnemiesKilledTable, 0, sizeof ( usEnemiesKilledTable ) );

	// fill table with raw data
	for ( ubKillType = 0; ubKillType < ENEMIES_KILLED_TOTAL; ubKillType++ )
	{
		for ( ubEnemyRank = 0; ubEnemyRank < ENEMY_RANK_TOTAL; ubEnemyRank++ )
		{
			usEnemiesKilledTable[ ubEnemyRank ][ ubKillType ] = gStrategicStatus.usEnemiesKilled[ ubKillType ][ ubEnemyRank ];
		}
	}

	// count how many enemies of each type & rank there are

	// add up totals across rows (ranks) by type
	for ( ubKillType = 0; ubKillType < ENEMIES_KILLED_TOTAL; ubKillType++ )
	{
		for ( ubEnemyRank = 0; ubEnemyRank < ENEMY_RANK_TOTAL; ubEnemyRank++ )
		{
			usEnemiesKilledTable[ ENEMY_RANK_TOTAL ][ ubKillType ] += usEnemiesKilledTable[ ubEnemyRank ][ ubKillType ];
		}
	}

	// add up totals by rank across columns (types)
	for ( ubEnemyRank = 0; ubEnemyRank <= ENEMY_RANK_TOTAL; ubEnemyRank++ )
	{
		for ( ubKillType = 0; ubKillType < ENEMIES_KILLED_TOTAL; ubKillType++ )
		{
			usEnemiesKilledTable[ ubEnemyRank ][ ENEMIES_KILLED_TOTAL ] += usEnemiesKilledTable[ ubEnemyRank ][ ubKillType ];
		}
	}

	// avoid division by zero
	if ( usEnemiesKilledTable[ ENEMY_RANK_TOTAL ][ ENEMIES_KILLED_TOTAL ] > 0 )
	{
		// calculate rank percentages
		for ( ubEnemyRank = 0; ubEnemyRank < ENEMY_RANK_PERCENT; ubEnemyRank++ )
		{
			usEnemiesKilledTable[ ubEnemyRank ][ ENEMIES_KILLED_PERCENT ] = ( ( 100 * usEnemiesKilledTable[ ubEnemyRank ][ ENEMIES_KILLED_TOTAL ] ) /
																																 usEnemiesKilledTable[ ENEMY_RANK_TOTAL ][ ENEMIES_KILLED_TOTAL ] );
		}

		// calculate kill type percentages
		for ( ubKillType = 0; ubKillType < ENEMIES_KILLED_PERCENT; ubKillType++ )
		{
			usEnemiesKilledTable[ ENEMY_RANK_PERCENT ][ ubKillType ] = ( ( 100 * usEnemiesKilledTable[ ENEMY_RANK_TOTAL ][ ubKillType ] ) /
																																 usEnemiesKilledTable[ ENEMY_RANK_TOTAL ][ ENEMIES_KILLED_TOTAL ] );
		}
	}

 	usX = VIEWER_RIGHT + 10;
	usY = 310;

	// titles and headings mean 2 extra rows
	ClearViewerRegion(usX, usY, SCREEN_WIDTH, usY + KILLED_TABLE_Y_GAP * (KILLED_TABLE_ROWS + 2) + 11);

	// print table title
	SetFontForeground( FONT_RED );
	MPrint(usX, usY, L"ENEMIES KILLED:");
	usY += KILLED_TABLE_Y_GAP;

	// print horizontal labels
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		DrawTextToScreen(EnemyRankString[ubEnemyRank], usX + KILLED_TABLE_X_OFFSET + KILLED_TABLE_X_GAP * ubEnemyRank, usY, KILLED_TABLE_X_GAP, FONT10ARIAL, FONT_LTBLUE, 0, RIGHT_JUSTIFIED);
	}


	// print vertical labels
	for ( ubKillType = 0; ubKillType < KILLED_TABLE_ROWS; ubKillType++ )
	{
		DrawTextToScreen(EnemiesKilledString[ubKillType], usX, usY + KILLED_TABLE_Y_GAP + KILLED_TABLE_Y_GAP * ubKillType, KILLED_TABLE_X_OFFSET, FONT10ARIAL, FONT_LTBLUE, 0, RIGHT_JUSTIFIED);
	}

	// over to first column, and down 1 line
	usY += KILLED_TABLE_Y_GAP;
	usX += KILLED_TABLE_X_OFFSET;

	// print table values
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		for ( ubKillType = 0; ubKillType < KILLED_TABLE_ROWS; ubKillType++ )
		{
			const wchar_t* PrintSpec;
			// an exclusive OR operator, how often do ya see that, huh?  :-)
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) ^ ( ubKillType == ENEMIES_KILLED_PERCENT ) )
			{
				PrintSpec = L"%3d%%";
			}
			else
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) && ( ubKillType == ENEMIES_KILLED_PERCENT ) )
			{
				PrintSpec = L"";
			}
			else
			{
				PrintSpec = L"%4d";
			}

			swprintf(wTempString, lengthof(wTempString), PrintSpec, usEnemiesKilledTable[ubEnemyRank][ubKillType]);
			DrawTextToScreen(wTempString, usX + KILLED_TABLE_X_GAP * ubEnemyRank, usY + KILLED_TABLE_Y_GAP * ubKillType, KILLED_TABLE_X_GAP, FONT10ARIAL, FONT_YELLOW, 0, RIGHT_JUSTIFIED);
		}
	}
}


static IconColour ChooseEnemyIconColor(UINT8 const ubAdmins, UINT8 const ubTroops, UINT8 const ubElites)
{
	// The colors are:
	//	Yellow		Admins only
	//	Red				Troops only
	//	Gray			Elites only
	//	Orange		Mixed, no elites (Admins + Troops)
	//	Burgundy	Mixed, with elites (Elites + (Admins OR Troops))

	Assert( ubAdmins || ubTroops || ubElites );

	IconColour ubIconColor;
	if ( ubElites )
	{
		if ( ubTroops || ubAdmins )
			ubIconColor = ICON_COLOR_BURGUNDY;
		else
			ubIconColor = ICON_COLOR_GRAY;
	}
	else	// no elites
	{
		if ( ubTroops )
		{
			if ( ubAdmins )
				ubIconColor = ICON_COLOR_ORANGE;
			else
				ubIconColor = ICON_COLOR_RED;
		}
		else // admins only
		{
			ubIconColor = ICON_COLOR_YELLOW;
		}
	}

	return ( ubIconColor );
}


static void BlitGroupIcon(IconType const ubIconType, IconColour const ubIconColor, UINT32 const uiX, UINT32 const uiY, SGPVObject const* const hVObject)
{
	UINT8 ubObjectIndex;

	Assert( ubIconType < NUM_ICON_TYPES );
	Assert( ubIconColor < NUM_ICON_COLORS );

	ubObjectIndex = ( ubIconType * NUM_ICON_COLORS ) + ubIconColor;
	BltVideoObject( FRAME_BUFFER, hVObject, ubObjectIndex, uiX, uiY);
}


static void PrintDetailedEnemiesInSectorInfo(INT32 iScreenX, INT32 iScreenY, UINT8 ubSectorX, UINT8 ubSectorY)
{
	SECTORINFO *pSector;
	INT32 iDesired, iSurplus;
	UINT8 ubGroupCnt = 0;
	UINT8 ubSectorID;
	wchar_t wString[120];
	wchar_t wSubString[120];
	INT16 iGarrisonIndex;
	INT16 iPatrolIndex;
	WAYPOINT *pFinalWaypoint;


	pSector = &SectorInfo[ SECTOR( ubSectorX, ubSectorY ) ];

	// handle garrisoned enemies
	if( pSector->ubGarrisonID != NO_GARRISON )
	{
		iDesired = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bDesiredPopulation;
		iSurplus = pSector->ubNumTroops + pSector->ubNumAdmins + pSector->ubNumElites - iDesired;
		SetFontForeground( FONT_WHITE );

		swprintf(wString, lengthof(wString), L"Garrison #%d: %d desired, ", pSector->ubGarrisonID, iDesired);

		if( iSurplus >= 0 )
		{
			swprintf(wSubString, lengthof(wSubString), L"%d surplus troops", iSurplus);
			wcscat( wString, wSubString );
		}
		else
		{
			swprintf(wSubString, lengthof(wSubString), L"%d reinforcements requested", -iSurplus);
			wcscat( wString, wSubString );
		}
		MPrint(iScreenX, iScreenY, wString);
		iScreenY += 10;

		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{
			const GROUP* const pGroup = GetGroup(gGarrisonGroup[pSector->ubGarrisonID].ubPendingGroupID);
			if( pGroup )
			{
				mprintf( iScreenX, iScreenY, L"%d reinforcements on route from group %d in %c%d", pGroup->ubGroupSize, pGroup->ubGroupID,
																																		pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX );
			}
			else
			{ //ERROR!  Should be a valid group...
			}
		}
		else
		{
			MPrint(iScreenX, iScreenY, L"No pending reinforcements for this sector.");
		}
		iScreenY += 10;
	}
	else
	{
		SetFontForeground( FONT_GRAY2 );
		MPrint(iScreenX, iScreenY, L"No garrison information for this sector.");
		iScreenY += 10;
	}


	// handle mobile enemies anchored in this sector
	CFOR_EACH_ENEMY_GROUP(pGroup)
	{
		if (!pGroup->fVehicle)
		{
			if ( ( pGroup->ubSectorX == ubSectorX ) && ( pGroup->ubSectorY == ubSectorY ) )
			{
				Assert( pGroup->pEnemyGroup->ubIntention < NUM_ENEMY_INTENTIONS );

				swprintf(wString, lengthof(wString), L"Group %c: %ls", 'A' + ubGroupCnt, gwGroupTypeString[pGroup->pEnemyGroup->ubIntention]);

				switch ( pGroup->pEnemyGroup->ubIntention )
				{
					case ASSAULT:
					case PURSUIT:
						iGarrisonIndex = FindGarrisonIndexForGroupIDPending( pGroup->ubGroupID );
						if ( iGarrisonIndex != -1 )
						{
							ubSectorID = gGarrisonGroup[ iGarrisonIndex ].ubSectorID;
							swprintf(wSubString, lengthof(wSubString), L", target sector %c%d", SECTORY(ubSectorID) + 'A' - 1, SECTORX(ubSectorID));
						}
						else
						{
							pFinalWaypoint = GetFinalWaypoint( pGroup );
							if( pFinalWaypoint )
							{
								if( pFinalWaypoint->x == 3 && pFinalWaypoint->y == 16 )
								{
									wcslcpy(wSubString, L" - group returning to pool.", lengthof(wSubString));
								}
								else
								{
									swprintf(wSubString, lengthof(wSubString), L" - moving to %c%d", pFinalWaypoint->y + 'A' - 1, pFinalWaypoint->x);
								}
							}
							else
							{
								wcslcpy(wSubString, L" - can't determine target sector", lengthof(wSubString));
							}
						}
						wcscat( wString, wSubString );
						break;
					case REINFORCEMENTS:
					case STAGING:
						// check if it's reinforcing a garrison
						iGarrisonIndex = FindGarrisonIndexForGroupIDPending( pGroup->ubGroupID );
						if ( iGarrisonIndex != -1 )
						{
							ubSectorID = gGarrisonGroup[ iGarrisonIndex ].ubSectorID;
							swprintf(wSubString, lengthof(wSubString), L", dest sector %c%d", SECTORY(ubSectorID) + 'A' - 1, SECTORX(ubSectorID));
							wcscat( wString, wSubString );
						}
						else	// must be reinforcing a patrol
						{
							iPatrolIndex = FindPatrolGroupIndexForGroupIDPending( pGroup->ubGroupID );
							if( iPatrolIndex != -1 )
							{
								pFinalWaypoint = GetFinalWaypoint( pGroup );
								Assert( pFinalWaypoint );

								swprintf(wSubString, lengthof(wSubString), L", Patrol #%d, dest sector %c%d", iPatrolIndex, pFinalWaypoint->y + 'A' - 1, pFinalWaypoint->x);
								wcscat( wString, wSubString );
							}
							else
							{
								pFinalWaypoint = GetFinalWaypoint( pGroup );
								if( pFinalWaypoint )
								{
									if( pFinalWaypoint->x == 3 && pFinalWaypoint->y == 16 )
									{
										wcslcpy(wSubString, L" - group returning to pool.", lengthof(wSubString));
									}
									else
									{
										swprintf(wSubString, lengthof(wSubString), L" - lost group moving to %c%d", pFinalWaypoint->y + 'A' - 1, pFinalWaypoint->x);
									}
								}
								else
								{
									wcslcpy(wSubString, L" (LOST GROUP!)", lengthof(wSubString));
								}
								wcscat( wString, wSubString );
							}
						}
						break;
					case PATROL:
						iPatrolIndex = FindPatrolGroupIndexForGroupID( pGroup->ubGroupID );
						if( iPatrolIndex != -1 )
						{
							swprintf(wSubString, lengthof(wSubString), L"#%d, next sector %c%d", iPatrolIndex, pGroup->ubNextY + 'A' - 1, pGroup->ubNextX);
						}
						else
						{
							swprintf(wSubString, lengthof(wSubString), L"#err, FLOATING GROUP!");
						}
						wcscat( wString, wSubString );
						break;
				}

				MPrint(iScreenX, iScreenY, wString);
				iScreenY += 10;

				ubGroupCnt++;

				// no room on screen to display info for more than 3 groups in one sector!
				if ( ubGroupCnt >= 3 )
				{
					break;
				}
			}
		}
	}
}


#endif
