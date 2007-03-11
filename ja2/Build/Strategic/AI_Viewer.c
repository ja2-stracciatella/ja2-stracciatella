#include <stdio.h>
#include "Types.h"
#include "English.h"
#include "Timer_Control.h"
#include "VSurface.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "Simple_Render_Utils.h"
#include "Editor_Taskbar_Utils.h"
#include "Line.h"
#include "Input.h"
#include "VObject_Blitters.h"
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
#include "ScreenIDs.h"
#include "Strategic_AI.h"
#include "Strategic_Status.h"
#include "Video.h"
#include "WordWrap.h"
#include "SGP.h"


#ifdef JA2BETAVERSION

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

enum
{
	ICON_TYPE_PATROL,
	ICON_TYPE_STOPPED,
	ICON_TYPE_REINFORCEMENT,
	ICON_TYPE_ASSAULT,
	NUM_ICON_TYPES
};

enum
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


void ClearViewerRegion( INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom );
void HandleViewerInput();
void RenderViewer();
void ViewerMapMoveCallback( MOUSE_REGION *reg, INT32 reason );
void ViewerMapClickCallback( MOUSE_REGION *reg, INT32 reason );
void ViewerExitCallback( GUI_BUTTON *btn, INT32 reason );
void Compression0Callback( GUI_BUTTON *btn, INT32 reason );
void Compression5Callback( GUI_BUTTON *btn, INT32 reason );
void Compression15Callback( GUI_BUTTON *btn, INT32 reason );
void Compression60Callback( GUI_BUTTON *btn, INT32 reason );
void Compression6HCallback( GUI_BUTTON *btn, INT32 reason );
void EasyCallback( GUI_BUTTON *btn, INT32 reason );
void NormalCallback( GUI_BUTTON *btn, INT32 reason );
void HardCallback( GUI_BUTTON *btn, INT32 reason );
void TestIncoming4SidesCallback( GUI_BUTTON *btn, INT32 reason );
void StartCreatureQuestCallback( GUI_BUTTON *btn, INT32 reason );
void SpreadCreaturesCallback( GUI_BUTTON *btn, INT32 reason );
void CreatureAttackCallback( GUI_BUTTON *btn, INT32 reason );
void B1Callback( GUI_BUTTON *btn, INT32 reason );
void B2Callback( GUI_BUTTON *btn, INT32 reason );
void B3Callback( GUI_BUTTON *btn, INT32 reason );
void ReloadSectorCallback( GUI_BUTTON *btn, INT32 reason );
void ToggleQueenAwake( GUI_BUTTON *btn, INT32 reason );
void ViewEnemiesCallback( GUI_BUTTON *btn, INT32 reason );
void ViewCreaturesCallback( GUI_BUTTON *btn, INT32 reason );
void ExtractAndUpdatePopulations();
void PrintEnemyPopTable();
void PrintEnemiesKilledTable();
UINT8 ChooseEnemyIconColor( UINT8 ubAdmins, UINT8 ubTroops, UINT8 ubElites );
void BlitGroupIcon( UINT8 ubIconType, UINT8 ubIconColor, UINT32 uiX, UINT32 uiY, HVOBJECT hVObject );
void PrintDetailedEnemiesInSectorInfo( INT32 iScreenX, INT32 iScreenY, UINT8 ubSectorX, UINT8 ubSectorY );



MOUSE_REGION ViewerRegion;

UINT32 guiMapGraphicID;
UINT32 guiMapIconsID;

BOOLEAN gfViewerEntry;
BOOLEAN gfExitViewer;

BOOLEAN gfRenderViewer;
BOOLEAN gfRenderMap;

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
INT16 gsHiSectorX = 0, gsHiSectorY = 0;
//The sector coordinates of the selected sector (red)
INT16 gsSelSectorX = 0, gsSelSectorY = 0;

INT32 iViewerButton[ NUM_VIEWER_BUTTONS ];


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


BOOLEAN CreateAIViewer()
{
	wchar_t str[6];

	//Check to see if data exists.
	if( !FileExists( "DevTools/arulco.sti" )			||
			!FileExists( "DevTools/icons.sti" )			||
			!FileExists( "DevTools/SmCheckbox.sti" ) )
	{
		ScreenMsg( FONT_WHITE, MSG_BETAVERSION, L"AIViewer missing data.  Aborted." );
		gfExitViewer = FALSE;
		gfViewerEntry = TRUE;
		return FALSE;
	}

	DisableScrollMessages();
	giSaveTCMode = giTimeCompressMode;

	VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
	if (!AddVideoObjectFromFile("DevTools/arulco.sti", &guiMapGraphicID))
		AssertMsg( 0, "Failed to load data/DevTools/arulco.sti" );
	if (!AddVideoObjectFromFile("DevTools/icons.sti", &guiMapIconsID))
		AssertMsg( 0, "Failed to load data/DevTools/icons.sti" );

	gfRenderViewer = TRUE;

	//Create all of the buttons here
	iViewerButton[ VIEWER_EXIT ] =
		CreateTextButton(L"Exit", BLOCKFONT2, FONT_RED, FONT_BLACK, BUTTON_USE_DEFAULT,
		585, 425, 50, 30, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		ViewerExitCallback );

	iViewerButton[ VIEWER_TIMEPANEL ] =
		CreateTextButton( WORLDTIMESTR, FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 3, 0, 88, 20, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, BUTTON_NO_CALLBACK,
		BUTTON_NO_CALLBACK );
	DisableButton( iViewerButton[ VIEWER_TIMEPANEL ] );
	SpecifyDisabledButtonStyle( iViewerButton[ VIEWER_TIMEPANEL ], DISABLED_STYLE_NONE );
	iViewerButton[ COMPRESSION0 ] =
		CreateTextButton( L"0", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 3, 20, 17, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		Compression0Callback );
	iViewerButton[ COMPRESSION5 ] =
		CreateTextButton( L"5", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 20, 20, 17, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		Compression5Callback );
	iViewerButton[ COMPRESSION15 ] =
		CreateTextButton( L"15", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 37, 20, 18, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		Compression15Callback );
	iViewerButton[ COMPRESSION60 ] =
		CreateTextButton( L"60", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 55, 20, 18, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		Compression60Callback );
/*
	iViewerButton[ COMPRESSION6H ] =
		CreateTextButton( L"6H", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 73, 20, 18, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		Compression6HCallback );
*/

	iViewerButton[ VIEWER_RESET ] =
		CreateTextButton( L"Reset Enemies", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		526, 0, 114, 20, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, BUTTON_NO_CALLBACK,
		BUTTON_NO_CALLBACK );
	DisableButton( iViewerButton[ VIEWER_RESET ] );
	SpecifyDisabledButtonStyle( iViewerButton[ VIEWER_RESET ], DISABLED_STYLE_NONE );
	iViewerButton[ RESET_EASY ] =
		CreateTextButton( L"Easy", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		526, 20, 35, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		EasyCallback );
	iViewerButton[ RESET_NORMAL ] =
		CreateTextButton( L"Normal", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		561, 20, 44, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		NormalCallback );
	iViewerButton[ RESET_HARD ] =
		CreateTextButton( L"Hard", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		605, 20, 35, 16, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		HardCallback );

	iViewerButton[ TEST_INCOMING_4SIDES ] =
		CreateTextButton( L"Incoming 4 Sides", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 20, 100, 120, 18, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		TestIncoming4SidesCallback );
	iViewerButton[ START_CREATURE_QUEST ] =
		CreateTextButton( L"Start Creature Quest", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 20, 125, 120, 18, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		StartCreatureQuestCallback );
	iViewerButton[ SPREAD_CREATURES ] =
		CreateTextButton( L"Spread Creatures", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 20, 150, 120, 18, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		SpreadCreaturesCallback );
	iViewerButton[ CREATURE_ATTACK ] =
		CreateTextButton( L"Creature Attack", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 20, 175, 120, 18, BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		CreatureAttackCallback );

	iViewerButton[ QUEEN_AWAKE_TOGGLE ] =
		CreateCheckBoxButton(	104, VIEWER_BOTTOM + 22, "DevTools/SmCheckbox.sti", MSYS_PRIORITY_HIGH, ToggleQueenAwake );
	if( gfQueenAIAwake )
	{
		ButtonList[ iViewerButton[ QUEEN_AWAKE_TOGGLE ] ]->uiFlags |= BUTTON_CLICKED_ON;
	}

	iViewerButton[ RELOAD_SECTOR ] =
		CreateTextButton( L"Override Sector", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		10, VIEWER_BOTTOM + 5, 90, 18, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		ReloadSectorCallback );

	iViewerButton[ VIEW_ENEMIES ] =
		CreateTextButton( L"View Enemies", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 13, 40, 90, 20, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		ViewEnemiesCallback );
	iViewerButton[ VIEW_CREATURES ] =
		CreateTextButton( L"View Creatures", FONT12POINT1, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 103, 40, 90, 20, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		ViewCreaturesCallback );

	iViewerButton[ BASEMENT1_BTN ] =
		CreateTextButton( L"B1", FONT16ARIAL, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 58, 60, 30, 24, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		B1Callback );
	iViewerButton[ BASEMENT2_BTN ] =
		CreateTextButton( L"B2", FONT16ARIAL, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 88, 60, 30, 24, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		B2Callback );
	iViewerButton[ BASEMENT3_BTN ] =
		CreateTextButton( L"B3", FONT16ARIAL, FONT_BLACK, FONT_BLACK, BUTTON_USE_DEFAULT,
		VIEWER_RIGHT + 118, 60, 30, 24, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH, DEFAULT_MOVE_CALLBACK,
		B3Callback );
	if( gfViewEnemies )
		ButtonList[ iViewerButton[ VIEW_ENEMIES ] ]->uiFlags |= BUTTON_CLICKED_ON;
	else
		ButtonList[ iViewerButton[ VIEW_CREATURES ] ]->uiFlags |= BUTTON_CLICKED_ON;

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
	ButtonList[ iViewerButton[ RESET_EASY + gGameOptions.ubDifficultyLevel - DIF_LEVEL_EASY ] ]->uiFlags |= BUTTON_CLICKED_ON;
	ButtonList[ iViewerButton[ COMPRESSION0 ] ]->uiFlags |= BUTTON_CLICKED_ON;
	if( !GamePaused() )
		SetGameMinutesPerSecond( 0 );
	ClearViewerRegion( 0, 0, 640, 480 );

	return TRUE;
}

void DestroyAIViewer()
{
	INT32 i;
	gfExitViewer = FALSE;
	gfViewerEntry = TRUE;
	for( i = 0; i < NUM_VIEWER_BUTTONS; i++ )
	{
		RemoveButton( iViewerButton[ i ] );
	}
	DeleteVideoObjectFromIndex( guiMapGraphicID );
	DeleteVideoObjectFromIndex( guiMapIconsID );
	MSYS_RemoveRegion( &ViewerRegion );

	KillTextInputMode();

	SetGameTimeCompressionLevel( giSaveTCMode );
	EnableScrollMessages();
}

void ClearViewerRegion( INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom )
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
	if( sBottom == 480 )
		ColorFillVideoSurfaceArea( ButtonDestBuffer, sLeft, 479, sRight, 480, gusDkBlue );
	if( sRight == 640 )
		ColorFillVideoSurfaceArea( ButtonDestBuffer, 639, sTop, 640, sBottom, gusDkBlue );
}

void RenderStationaryGroups()
{
	SECTORINFO *pSector;
	INT32 x, y, xp, yp;
	wchar_t str[20];
	INT32 iSector = 0;
	UINT8 ubIconColor;
	UINT8 ubGroupSize = 0;


	SetFont( FONT10ARIAL );
	SetFontShadow( FONT_NEARBLACK );

	HVOBJECT hVObject = GetVideoObject(guiMapIconsID);

	//Render groups that are stationary...
	for( y = 0; y < 16; y++ )
	{
		yp = VIEWER_TOP + VIEWER_CELLH * y + 1;
		for( x = 0; x < 16; x++ )
		{
			SetFontForeground( FONT_YELLOW );
			xp = VIEWER_LEFT + VIEWER_CELLW * x + 1;
			pSector = &SectorInfo[ iSector ];

			if( pSector->uiFlags & SF_MINING_SITE )
				BltVideoObject( FRAME_BUFFER, hVObject, MINING_ICON, xp + 25, yp - 1);

			if( pSector->uiFlags & SF_SAM_SITE )
				BltVideoObject( FRAME_BUFFER, hVObject, SAM_ICON, xp + 20, yp + 4);


			if( pSector->ubNumberOfCivsAtLevel[0] + pSector->ubNumberOfCivsAtLevel[1] + pSector->ubNumberOfCivsAtLevel[2] )
			{
				// show militia
				ubIconColor = ICON_COLOR_BLUE;
				ubGroupSize = pSector->ubNumberOfCivsAtLevel[0] + pSector->ubNumberOfCivsAtLevel[1] + pSector->ubNumberOfCivsAtLevel[2];
			}
			else
			if( pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites )
			{
				// show enemies
				ubIconColor = ChooseEnemyIconColor( pSector->ubNumAdmins, pSector->ubNumTroops, pSector->ubNumElites );
				ubGroupSize = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
				if( pSector->ubGarrisonID != NO_GARRISON )
				{
					if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
					{
						if( GetJA2Clock() % 1000 < 333 )
						{
							SetFontForeground( FONT_LTRED );
						}
					}
				}
				else
				{
					if( GetJA2Clock() % 1000 < 333 )
					{
						SetFontForeground( FONT_LTKHAKI );
					}
				}
			}
			else
			{
				ubGroupSize = 0;
			}

			if ( ubGroupSize > 0 )
			{
				// draw the icon
				BlitGroupIcon( ICON_TYPE_STOPPED, ubIconColor, xp, yp, hVObject );

				//Print the group size
				swprintf(str, lengthof(str), L"%d", ubGroupSize);
				mprintf( xp + 2, yp + 2, str );
			}

			iSector++;
		}
	}
}

void RenderMovingGroupsAndMercs()
{
	GROUP *pGroup;
	INT32 x, y;
	UINT8 ubNumTroops, ubNumAdmins, ubNumElites;
	float ratio;
	INT32 minX, maxX, minY, maxY;
	INT32 iSector = 0;
	UINT8 ubIconType;
	UINT8 ubIconColor;
	UINT8 ubFontColor;


	SetFont( FONT10ARIAL );
	SetFontShadow( FONT_NEARBLACK );

	HVOBJECT hVObject = GetVideoObject(guiMapIconsID);

	//Render groups that are moving...
	pGroup = gpGroupList;
	while( pGroup )
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
					BltVideoObject( FRAME_BUFFER, hVObject, GROUP_ANCHOR, VIEWER_LEFT + VIEWER_CELLW * (pGroup->ubSectorX - 1), VIEWER_TOP + VIEWER_CELLH * (pGroup->ubSectorY - 1));
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
			BlitGroupIcon( ubIconType, ubIconColor, x, y, hVObject );

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

		pGroup = pGroup->next;
	}
}


void RenderInfoInSector()
{
	UINT8 ubSectorX, ubSectorY;
	UINT8 ubMercs=0, ubActive=0, ubUnconcious=0, ubCollapsed=0;
	INT32 i, yp;

	if( gfViewEnemies && !gbViewLevel )
	{
		RenderStationaryGroups();
		RenderMovingGroupsAndMercs();
		SetFontForeground( FONT_LTRED );
		mprintf( 78, 358, L"%3d", giReinforcementPool );

		//Render general enemy statistics
		ClearViewerRegion( 105, VIEWER_BOTTOM + 10, 265, VIEWER_BOTTOM + 66 );
		SetFontForeground( FONT_YELLOW );
		mprintf( 105, VIEWER_BOTTOM + 10, L"GLOBAL INFO" );
		SetFontForeground( FONT_GRAY2 );
		mprintf( 118, VIEWER_BOTTOM + 24, L"Strategic AI Awake" );
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
	for( i = gTacticalStatus.Team[ OUR_TEAM ].bFirstID; i <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; i++ )
	{
		SOLDIERTYPE *pSoldier;

		pSoldier = MercPtrs[ i ];
		if( pSoldier->bActive && pSoldier->sSectorX == ubSectorX && pSoldier->sSectorY == ubSectorY && pSoldier->bSectorZ == gbViewLevel )
		{
			if( pSoldier->bLife )
			{
				ubMercs++;
				if( pSoldier->bLife >= OKLIFE )
				{
					if( pSoldier->bBreath < OKBREATH )
						ubCollapsed++;
					else
						ubActive++;
				}
				else
					ubUnconcious++;
			}
		}
	}

	yp = 375;
	if( !gbViewLevel )
	{
		SECTORINFO *pSector;
		GROUP *pGroup;
		UINT8 ubNumAdmins=0, ubNumTroops=0, ubNumElites=0, ubAdminsInBattle=0, ubTroopsInBattle=0, ubElitesInBattle=0, ubNumGroups=0;

		pSector = &SectorInfo[ SECTOR( ubSectorX, ubSectorY ) ];

		//Now count the number of mobile groups in the sector.
		pGroup = gpGroupList;
		while( pGroup )
		{
			if( !pGroup->fPlayer && !pGroup->fVehicle && pGroup->ubSectorX == ubSectorX && pGroup->ubSectorY == ubSectorY )
			{
				ubNumTroops += pGroup->pEnemyGroup->ubNumTroops;
				ubNumElites += pGroup->pEnemyGroup->ubNumElites;
				ubNumAdmins += pGroup->pEnemyGroup->ubNumAdmins;
				ubTroopsInBattle += pGroup->pEnemyGroup->ubTroopsInBattle;
				ubElitesInBattle += pGroup->pEnemyGroup->ubElitesInBattle;
				ubAdminsInBattle += pGroup->pEnemyGroup->ubAdminsInBattle;
				ubNumGroups++;
			}
			pGroup = pGroup->next;
		}
		ClearViewerRegion( 280, 375, 640, 480 );
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
		ClearViewerRegion( 280, 375, 640, 480 );
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
			mprintf( 280, yp, L"SF_PENDING_ALTERNATE_MAP", pSector->ubCreaturesInBattle, pSector->ubNumCreatures );
			yp += 10;
		}
		if( pSector->uiFlags & SF_USE_ALTERNATE_MAP )
		{
			mprintf( 280, yp, L"SF_USE_ALTERNATE_MAP", pSector->ubCreaturesInBattle, pSector->ubNumCreatures );
			yp += 10;
		}
	}

}

void RenderViewer()
{
	UINT8 *pDestBuf;
	UINT32 uiDestPitchBYTES;
	SGPRect ClipRect;
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

		SetFont( FONT10ARIAL );
		SetFontForeground( FONT_YELLOW );
		SetFontShadow( FONT_NEARBLACK );

		mprintf( 38, VIEWER_BOTTOM + 33, L"Admins" );
		mprintf( 38, VIEWER_BOTTOM + 53, L"Troops" );
		mprintf( 38, VIEWER_BOTTOM + 73, L"Elites" );
		mprintf( 38, VIEWER_BOTTOM + 93, L"Creatures" );
	}

	if( gfRenderMap )
	{
		gfRenderMap = FALSE;
		BltVideoObjectFromIndex( FRAME_BUFFER, guiMapGraphicID, 0, VIEWER_LEFT, VIEWER_TOP);
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
			mprintf( VIEWER_LEFT+x*26-(26+StringPixLength( str, FONT12POINT1 ))/2, VIEWER_TOP-12, str );
		}
		if( gbViewLevel )
		{
			UNDERGROUND_SECTORINFO *pUnder;
			SetFont( FONT10ARIAL );
			SetFontForeground( FONT_YELLOW );
			SetFontShadow( FONT_NEARBLACK );
			for( y = 0; y < 16; y++ )
			{
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
						pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
						ClipRect.iLeft = VIEWER_LEFT + x*VIEWER_CELLW;
						ClipRect.iRight = ClipRect.iLeft + VIEWER_CELLW - 1;
						Blt16BPPBufferShadowRect( (UINT16*)pDestBuf, uiDestPitchBYTES, &ClipRect );
						Blt16BPPBufferShadowRect( (UINT16*)pDestBuf, uiDestPitchBYTES, &ClipRect );
						UnLockVideoSurface( FRAME_BUFFER );
					}
				}
			}
		}
		RenderInfoInSector();
	}

	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );
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
	UnLockVideoSurface( FRAME_BUFFER );
}

void ViewerExitCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfExitViewer = TRUE;
	}
}

void HandleViewerInput()
{
	SECTORINFO *pSector;
	InputAtom Event;
	while( DequeueEvent( &Event ) )
	{
		if( !HandleTextInput( &Event ) && Event.usEvent == KEY_DOWN )
		{
			switch( Event.usParam )
			{
				case ESC:
					gfExitViewer = TRUE;
					break;
				case ENTER:
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

void EasyCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ RESET_EASY ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ RESET_NORMAL ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ RESET_HARD ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ QUEEN_AWAKE_TOGGLE ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		gfRenderViewer = TRUE;
		gGameOptions.ubDifficultyLevel = DIF_LEVEL_EASY;
		ShutdownStrategicLayer();
		InitStrategicLayer();
		Compression0Callback( ButtonList[ iViewerButton[ COMPRESSION0 ] ], MSYS_CALLBACK_REASON_LBUTTON_UP );
		MarkButtonsDirty();
		WarpGameTime( 45 * 60, WARPTIME_PROCESS_EVENTS_NORMALLY );
	}
}

void NormalCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ RESET_EASY ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ RESET_NORMAL ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ RESET_HARD ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ QUEEN_AWAKE_TOGGLE ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		gfRenderViewer = TRUE;
		gGameOptions.ubDifficultyLevel = DIF_LEVEL_MEDIUM;
		ShutdownStrategicLayer();
		InitStrategicLayer();
		Compression0Callback( ButtonList[ iViewerButton[ COMPRESSION0 ] ], MSYS_CALLBACK_REASON_LBUTTON_UP );
		MarkButtonsDirty();
		WarpGameTime( 45 * 60, WARPTIME_PROCESS_EVENTS_NORMALLY );
	}
}

void HardCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ RESET_EASY ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ RESET_NORMAL ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ RESET_HARD ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ QUEEN_AWAKE_TOGGLE ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		gfRenderViewer = TRUE;
		gGameOptions.ubDifficultyLevel = DIF_LEVEL_HARD;
		ShutdownStrategicLayer();
		InitStrategicLayer();
		Compression0Callback( ButtonList[ iViewerButton[ COMPRESSION0 ] ], MSYS_CALLBACK_REASON_LBUTTON_UP );
		MarkButtonsDirty();
		WarpGameTime( 45 * 60, WARPTIME_PROCESS_EVENTS_NORMALLY );
	}
}

void ViewerMapMoveCallback( MOUSE_REGION *reg, INT32 reason )
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
	gsHiSectorX = min( (reg->RelativeXPos / 26) + 1, 16 );
	gsHiSectorY = min( (reg->RelativeYPos / 22) + 1, 16 );
	if( gsPrevX != gsHiSectorX || gsPrevY != gsHiSectorY )
	{
		gsPrevX = gsHiSectorX;
		gsPrevY = gsHiSectorY;
		gfRenderViewer = TRUE;
	}
}

void ViewerMapClickCallback( MOUSE_REGION *reg, INT32 reason )
{
	static INT16 sLastX = -1, sLastY = -1;
	//calc current sector selected.
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gsSelSectorX = min( (reg->RelativeXPos / 26) + 1, 16 );
		gsSelSectorY = min( (reg->RelativeYPos / 22) + 1, 16 );
		if( gsSelSectorX != sLastX || gsSelSectorY != sLastY )
		{ //clicked in a new sector
			sLastX = gsSelSectorX;
			sLastY = gsSelSectorY;
			gfRenderViewer = TRUE;
		}
	}
}



UINT32  AIViewerScreenInit()
{
	gfViewerEntry = TRUE;
	gusBlue		= Get16BPPColor( FROMRGB(  65,  79,  94 ) );
	gusLtBlue = Get16BPPColor( FROMRGB( 122, 124, 121 ) );
	gusDkBlue = Get16BPPColor( FROMRGB(  22,  55,  73 ) );
	return TRUE;
}

UINT32  AIViewerScreenHandle()
{
	StartFrameBufferRender();

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
		SpecifyButtonText( iViewerButton[ VIEWER_TIMEPANEL ], WORLDTIMESTR );
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

UINT32  AIViewerScreenShutdown()
{
	return TRUE;
}

void Compression0Callback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ COMPRESSION0 ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ COMPRESSION5 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION15 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION60 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
//		ButtonList[ iViewerButton[ COMPRESSION6H ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		SetGameMinutesPerSecond( 0 );
	}
}

void Compression5Callback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ COMPRESSION0 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION5 ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ COMPRESSION15 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION60 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
//		ButtonList[ iViewerButton[ COMPRESSION6H ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		SetGameMinutesPerSecond( 5 );
		SetFactTimeCompressHasOccured( );
	}
}

void Compression15Callback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ COMPRESSION0 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION5 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION15 ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ COMPRESSION60 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
//		ButtonList[ iViewerButton[ COMPRESSION6H ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		SetGameMinutesPerSecond( 15 );
		SetFactTimeCompressHasOccured( );
	}
}

void Compression60Callback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ COMPRESSION0 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION5 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION15 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION60 ] ]->uiFlags |= BUTTON_CLICKED_ON;
//		ButtonList[ iViewerButton[ COMPRESSION6H ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		SetGameHoursPerSecond( 1 );
		SetFactTimeCompressHasOccured( );
	}
}

void Compression6HCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ButtonList[ iViewerButton[ COMPRESSION0 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION5 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION15 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
		ButtonList[ iViewerButton[ COMPRESSION60 ] ]->uiFlags &= (~BUTTON_CLICKED_ON);
//		ButtonList[ iViewerButton[ COMPRESSION6H ] ]->uiFlags |= BUTTON_CLICKED_ON;
		SetGameHoursPerSecond( 6 );
		SetFactTimeCompressHasOccured( );
	}
}

void TestIncoming4SidesCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		GROUP *pGroup;
		UINT8 ubSector;
		UINT32 uiWorldMin;
		Compression0Callback( ButtonList[ iViewerButton[ COMPRESSION0 ] ], MSYS_CALLBACK_REASON_LBUTTON_UP );
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
			SetGroupArrivalTime( pGroup, uiWorldMin + 10 );
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
			SetGroupArrivalTime( pGroup, uiWorldMin + 12 );
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
			SetGroupArrivalTime( pGroup, uiWorldMin + 11 );
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
			SetGroupArrivalTime( pGroup, uiWorldMin + 13 );
			pGroup->ubMoveType = ONE_WAY;
			pGroup->fDebugGroup = TRUE;
			AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );
		}
	}
}

void StartCreatureQuestCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gGameOptions.fSciFi = TRUE;
		gfRenderMap = TRUE;
		ClearCreatureQuest();
		InitCreatureQuest();
	}
}

void SpreadCreaturesCallback( GUI_BUTTON *btn, INT32 reason )
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

void CreatureAttackCallback( GUI_BUTTON *btn, INT32 reason )
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

void ViewEnemiesCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfViewEnemies = TRUE;
		gfRenderMap = TRUE;
		ButtonList[ iViewerButton[ VIEW_CREATURES ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		gbViewLevel = 0;
		ButtonList[ iViewerButton[ BASEMENT1_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ BASEMENT2_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ BASEMENT3_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}

void ViewCreaturesCallback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfViewEnemies = FALSE;
		gfRenderMap = TRUE;
		ButtonList[ iViewerButton[ VIEW_ENEMIES] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		if( !gbViewLevel )
		{
			gbViewLevel = 1;
			ButtonList[ iViewerButton[ BASEMENT1_BTN ] ]->uiFlags |= BUTTON_CLICKED_ON;
			ButtonList[ iViewerButton[ BASEMENT2_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
			ButtonList[ iViewerButton[ BASEMENT3_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
	}
}

void B1Callback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMap = TRUE;
		gbViewLevel = 1;
		ButtonList[ iViewerButton[ BASEMENT2_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ BASEMENT3_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}

void B2Callback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMap = TRUE;
		gbViewLevel = 2;
		ButtonList[ iViewerButton[ BASEMENT1_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ BASEMENT3_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}

void B3Callback( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderMap = TRUE;
		gbViewLevel = 3;
		ButtonList[ iViewerButton[ BASEMENT1_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[ iViewerButton[ BASEMENT2_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}

void ReloadSectorCallback( GUI_BUTTON *btn, INT32 reason )
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

void ExtractAndUpdatePopulations()
{
	gsAINumAdmins = min( GetNumericStrictValueFromField( 0 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 0, gsAINumAdmins );

	gsAINumTroops = min( GetNumericStrictValueFromField( 1 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 1, gsAINumTroops );

	gsAINumElites = min( GetNumericStrictValueFromField( 2 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 2, gsAINumElites );

	gsAINumCreatures = min( GetNumericStrictValueFromField( 3 ), MAX_STRATEGIC_TEAM_SIZE );
	SetInputFieldStringWithNumericStrictValue( 3, gsAINumCreatures );
}

void ToggleQueenAwake( GUI_BUTTON *btn, INT32 reason )
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( btn->uiFlags & BUTTON_CLICKED_ON )
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
	L"Trp"
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

void PrintEnemyPopTable()
{
	UINT16 usX, usY;
	UINT16 usEnemyPopTable[ TABLE_ENEMY_RANKS ][ POP_TABLE_ENEMY_TYPES ];
	UINT32 uiSector = 0;
	UINT8		ubEnemyRank;
	UINT8		ubEnemyType;
	SECTORINFO *pSector;
	GROUP *pGroup;
	wchar_t wPrintSpec[10];
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
	pGroup = gpGroupList;
	while( pGroup )
	{
		if( !pGroup->fPlayer && !pGroup->fDebugGroup )
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

		pGroup = pGroup->next;
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
	ClearViewerRegion( usX, usY, 640, ( INT16 ) ( usY + ( POP_TABLE_Y_GAP * ( POP_TABLE_ENEMY_TYPES + 2 )) + 11 ) );

	// print table title
	SetFontForeground( FONT_RED );
	mprintf( usX, usY, L"ENEMY POPULATION:" );
	usY += POP_TABLE_Y_GAP;

	// print horizontal labels
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		DrawTextToScreen( EnemyRankString[ ubEnemyRank ], ( UINT16 ) ( usX + POP_TABLE_X_OFFSET + ( POP_TABLE_X_GAP * ubEnemyRank ) ), usY, POP_TABLE_X_GAP, FONT10ARIAL, FONT_LTBLUE, 0, FALSE, RIGHT_JUSTIFIED );
	}


	// print vertical labels
	for ( ubEnemyType = 0; ubEnemyType < POP_TABLE_ENEMY_TYPES; ubEnemyType++ )
	{
		DrawTextToScreen( EnemyTypeString[ ubEnemyType ], usX, ( UINT16 ) ( usY + POP_TABLE_Y_GAP + ( POP_TABLE_Y_GAP * ubEnemyType ) ), POP_TABLE_X_OFFSET, FONT10ARIAL, FONT_LTBLUE, 0, FALSE, RIGHT_JUSTIFIED );
	}

	// over to first column, and down 1 line
	usY += POP_TABLE_Y_GAP;
	usX += POP_TABLE_X_OFFSET;

	SetFontForeground( FONT_YELLOW );

	// print table values
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		for ( ubEnemyType = 0; ubEnemyType < POP_TABLE_ENEMY_TYPES; ubEnemyType++ )
		{
			// an exclusive OR operator, how often do ya see that, huh?  :-)
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) ^ ( ubEnemyType == ENEMY_TYPE_PERCENT ) )
			{
				wcscpy( wPrintSpec, L"%3d%%%%" );
			}
			else
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) && ( ubEnemyType == ENEMY_TYPE_PERCENT ) )
			{
				wcscpy( wPrintSpec, L"" );
			}
			else
			{
				wcscpy( wPrintSpec, L"%4d" );
			}

			swprintf(wTempString, lengthof(wTempString), wPrintSpec, usEnemyPopTable[ubEnemyRank][ubEnemyType]);
			DrawTextToScreen( wTempString, ( UINT16 ) ( usX + ( POP_TABLE_X_GAP * ubEnemyRank ) ), ( UINT16 ) ( usY + ( POP_TABLE_Y_GAP * ubEnemyType ) ), POP_TABLE_X_GAP, FONT10ARIAL, FONT_YELLOW, 0, FALSE, RIGHT_JUSTIFIED );
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

void PrintEnemiesKilledTable()
{
	UINT16 usX, usY;
	UINT16 usEnemiesKilledTable[ TABLE_ENEMY_RANKS ][ KILLED_TABLE_ROWS ];
	UINT8	ubEnemyRank;
	UINT8 ubKillType;
	wchar_t wPrintSpec[10];
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
	ClearViewerRegion( usX, usY, 640, ( INT16 ) ( usY + ( KILLED_TABLE_Y_GAP * ( KILLED_TABLE_ROWS + 2 ) ) + 11) );

	// print table title
	SetFontForeground( FONT_RED );
	mprintf( usX, usY, L"ENEMIES KILLED:" );
	usY += KILLED_TABLE_Y_GAP;

	// print horizontal labels
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		DrawTextToScreen( EnemyRankString[ ubEnemyRank ], ( UINT16 ) ( usX + KILLED_TABLE_X_OFFSET + ( KILLED_TABLE_X_GAP * ubEnemyRank ) ), usY, KILLED_TABLE_X_GAP, FONT10ARIAL, FONT_LTBLUE, 0, FALSE, RIGHT_JUSTIFIED );
	}


	// print vertical labels
	for ( ubKillType = 0; ubKillType < KILLED_TABLE_ROWS; ubKillType++ )
	{
		DrawTextToScreen( EnemiesKilledString[ ubKillType ], usX, ( UINT16 ) ( usY + KILLED_TABLE_Y_GAP + ( KILLED_TABLE_Y_GAP * ubKillType ) ), KILLED_TABLE_X_OFFSET, FONT10ARIAL, FONT_LTBLUE, 0, FALSE, RIGHT_JUSTIFIED );
	}

	// over to first column, and down 1 line
	usY += KILLED_TABLE_Y_GAP;
	usX += KILLED_TABLE_X_OFFSET;

	SetFontForeground( FONT_YELLOW );

	// print table values
	for ( ubEnemyRank = 0; ubEnemyRank < TABLE_ENEMY_RANKS; ubEnemyRank++ )
	{
		for ( ubKillType = 0; ubKillType < KILLED_TABLE_ROWS; ubKillType++ )
		{
			// an exclusive OR operator, how often do ya see that, huh?  :-)
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) ^ ( ubKillType == ENEMIES_KILLED_PERCENT ) )
			{
				wcscpy( wPrintSpec, L"%3d%%%%" );
			}
			else
			if ( ( ubEnemyRank == ENEMY_RANK_PERCENT ) && ( ubKillType == ENEMIES_KILLED_PERCENT ) )
			{
				wcscpy( wPrintSpec, L"" );
			}
			else
			{
				wcscpy( wPrintSpec, L"%4d" );
			}

			swprintf(wTempString, lengthof(wTempString), wPrintSpec, usEnemiesKilledTable[ubEnemyRank][ubKillType]);
			DrawTextToScreen( wTempString, ( UINT16 ) ( usX + ( KILLED_TABLE_X_GAP * ubEnemyRank ) ), ( UINT16 ) ( usY + ( KILLED_TABLE_Y_GAP * ubKillType ) ), KILLED_TABLE_X_GAP, FONT10ARIAL, FONT_YELLOW, 0, FALSE, RIGHT_JUSTIFIED );
		}
	}
}



UINT8 ChooseEnemyIconColor( UINT8 ubAdmins, UINT8 ubTroops, UINT8 ubElites )
{
	UINT8 ubIconColor;

	// The colors are:
	//	Yellow		Admins only
	//	Red				Troops only
	//	Gray			Elites only
	//	Orange		Mixed, no elites (Admins + Troops)
	//	Burgundy	Mixed, with elites (Elites + (Admins OR Troops))

	Assert( ubAdmins || ubTroops || ubElites );

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


void BlitGroupIcon( UINT8 ubIconType, UINT8 ubIconColor, UINT32 uiX, UINT32 uiY, HVOBJECT hVObject )
{
	UINT8 ubObjectIndex;

	Assert( ubIconType < NUM_ICON_TYPES );
	Assert( ubIconColor < NUM_ICON_COLORS );

	ubObjectIndex = ( ubIconType * NUM_ICON_COLORS ) + ubIconColor;
	BltVideoObject( FRAME_BUFFER, hVObject, ubObjectIndex, uiX, uiY);
}



void PrintDetailedEnemiesInSectorInfo( INT32 iScreenX, INT32 iScreenY, UINT8 ubSectorX, UINT8 ubSectorY )
{
	SECTORINFO *pSector;
	GROUP *pGroup;
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
		mprintf( iScreenX, iScreenY, wString );
		iScreenY += 10;

		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{
			pGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
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
			mprintf( iScreenX, iScreenY, L"No pending reinforcements for this sector." );
		}
		iScreenY += 10;
	}
	else
	{
		SetFontForeground( FONT_GRAY2 );
		mprintf( iScreenX, iScreenY, L"No garrison information for this sector." );
		iScreenY += 10;
	}


	// handle mobile enemies anchored in this sector
	pGroup = gpGroupList;
	while( pGroup )
	{
		if( !pGroup->fPlayer && !pGroup->fVehicle )
		{
			if ( ( pGroup->ubSectorX == ubSectorX ) && ( pGroup->ubSectorY == ubSectorY ) )
			{
				Assert( pGroup->pEnemyGroup->ubIntention < NUM_ENEMY_INTENTIONS );

				swprintf(wString, lengthof(wString), L"Group %c: %s", 'A' + ubGroupCnt, gwGroupTypeString[pGroup->pEnemyGroup->ubIntention]);

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
									swprintf(wSubString, lengthof(wSubString), L" - group returning to pool.");
								}
								else
								{
									swprintf(wSubString, lengthof(wSubString), L" - moving to %c%d", pFinalWaypoint->y + 'A' - 1, pFinalWaypoint->x);
								}
							}
							else
							{
								swprintf(wSubString, lengthof(wSubString), L" - can't determine target sector");
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
										swprintf(wSubString, lengthof(wSubString), L" - group returning to pool.");
									}
									else
									{
										swprintf(wSubString, lengthof(wSubString), L" - lost group moving to %c%d", pFinalWaypoint->y + 'A' - 1, pFinalWaypoint->x);
									}
								}
								else
								{
									swprintf(wSubString, lengthof(wSubString), L" (LOST GROUP!)");
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

				mprintf( iScreenX, iScreenY, wString );
				iScreenY += 10;

				ubGroupCnt++;

				// no room on screen to display info for more than 3 groups in one sector!
				if ( ubGroupCnt >= 3 )
				{
					break;
				}
			}
		}

		pGroup = pGroup->next;
	}
}


#endif
