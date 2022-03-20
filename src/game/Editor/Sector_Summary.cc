#include "Directories.h"
#include "HImage.h"
#include "LoadSaveBasicSoldierCreateStruct.h"
#include "LoadSaveSoldierCreate.h"
#include "Local.h"
#include "Font.h"
#include "Overhead_Map.h"
#include "Types.h"
#include "Sector_Summary.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "Simple_Render_Utils.h"
#include "Editor_Taskbar_Utils.h"
#include "Line.h"
#include "Input.h"
#include "LoadScreen.h"
#include "Text_Input.h"
#include "MouseSystem.h"
#include "StrategicMap.h"
#include "FileMan.h"
#include "Exit_Grids.h"
#include "Map_Information.h"
#include "Summary_Info.h"
#include "Animated_ProgressBar.h"
#include "WorldDef.h"
#include "WorldDat.h"
#include "EditorDefines.h"
#include "EditScreen.h"
#include "English.h"
#include "World_Items.h"
#include "Text.h"
#include "Debug.h"
#include "MemMan.h"
#include "Soldier_Create.h"
#include "Video.h"
#include "UILayout.h"
#include "GameRes.h"
#include "GameMode.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <string_theory/format>
#include <string_theory/string>

#include <vector>

#define DEVINFO_DIR "DevInfo"


#define MAP_SIZE			208
#define MAP_LEFT			417
#define MAP_TOP				15
#define MAP_RIGHT			(MAP_LEFT+MAP_SIZE)
#define MAP_BOTTOM		(MAP_TOP+MAP_SIZE)

enum{
	PRE_ALPHA,
	ALPHA,
	DEMO,
	BETA,
	RELEASE
};
static const ST::string gszVersionType[5] = { "Pre-Alpha", "Alpha", "Demo", "Beta", "Release" };
#define GLOBAL_SUMMARY_STATE			RELEASE

//Regular masks
#define GROUND_LEVEL_MASK			0x01
#define BASEMENT1_LEVEL_MASK	0x02
#define BASEMENT2_LEVEL_MASK	0x04
#define BASEMENT3_LEVEL_MASK	0x08
#define ALL_LEVELS_MASK				0x0f
//Alternate masks
#define ALTERNATE_GROUND_MASK 0x10
#define ALTERNATE_B1_MASK			0x20
#define ALTERNATE_B2_MASK			0x40
#define ALTERNATE_B3_MASK			0x80
#define ALTERNATE_LEVELS_MASK	0xf0

static INT32 giCurrLevel;

static BOOLEAN gfOutdatedDenied;
static UINT16 gusNumEntriesWithOutdatedOrNoSummaryInfo;

BOOLEAN gfUpdatingNow;
static UINT16 gusTotal;
static UINT16 gusCurrent;

BOOLEAN gfMustForceUpdateAllMaps = FALSE;
static UINT16 gusNumberOfMapsToBeForceUpdated = 0;
BOOLEAN gfMajorUpdate = FALSE;

static INT32 giCurrentViewLevel = ALL_LEVELS_MASK;

static BOOLEAN gbSectorLevels[16][16];
static BOOLEAN gfGlobalSummaryLoaded = FALSE;

static SUMMARYFILE *gpSectorSummary[16][16][8];
static SUMMARYFILE *gpCurrentSectorSummary;

static MOUSE_REGION MapRegion;

extern INT8 gbMercSlotTypes[9];


//Set if there is an existing global summary.  The first time this is run on your computer, it
//will not exist, and will have to be generated before this will be set.
BOOLEAN gfGlobalSummaryExists;
//If you don't wish to create a global summary, you can deny it.  This safely locks the system
//from generating one.
static BOOLEAN gfDeniedSummaryCreation;
//Set whenever the entire display is to be marked dirty.
static BOOLEAN gfRenderSummary;
//Used externally to determine if the summary window is up or not.
BOOLEAN gfSummaryWindowActive;
//Used externally to determine if the current summary map should be saved.
BOOLEAN gfSummaryWindowSaveRequested;
//When set, the summary window stays up until told otherwise.  When clear, the summary will disappear
//when the assigned key (F5) is released.  The latter mode is initiated when F5 is held down for longer
//than .4 seconds, and is useful for quickly looking at the information in the current map being edited.
static BOOLEAN gfPersistantSummary;
//When set, a grid is overlayed on top of the sector.  This grid defines each of the 256 sectors.  It is
//on by default.
static BOOLEAN gfRenderGrid;
//When set, parts of the map are darkened, showing that those sectors don't exist in the currently selected
//layer.  When clear, the entire map is shown in full color.
static BOOLEAN gfRenderProgress;
//When set, only the map section is rerendered.
static BOOLEAN gfRenderMap;
//When set, then we are overriding the ability to use normal methods for selecting sectors for saving and
//loading.  Immediately upon entering the text input mode; for the temp file; then we are assuming that
//the user will type in a name that doesn't follow standard naming conventions for the purposes of the
//campaign editor.  This is useful for naming a temp file for saving or loading.
static BOOLEAN gfTempFile;
//When set, only the alternate version of the maps will be displayed.  This is used for alternate maps in
//particular sectors, such as the SkyRider quest which could be located at one of four maps randomly.  If
//that particular map is chosen, then the alternate map will be used.
static BOOLEAN gfAlternateMaps = FALSE;


enum
{
	ITEMMODE_SCIFI,
	ITEMMODE_REAL,
	ITEMMODE_ENEMY,
};
static UINT8 gubSummaryItemMode = ITEMMODE_SCIFI;

static BOOLEAN gfItemDetailsMode = FALSE;

static std::vector<WORLDITEM> gpWorldItemsSummaryArray;
static OBJECTTYPE* gpPEnemyItemsSummaryArray      = 0;
static UINT16      gusPEnemyItemsSummaryArraySize = 0;
static OBJECTTYPE* gpNEnemyItemsSummaryArray      = 0;
static UINT16      gusNEnemyItemsSummaryArraySize = 0;

static BOOLEAN gfSetupItemDetailsMode = TRUE;

//Override status.  Hide is when there is nothing to override, readonly, when checked is to override a
//readonly status file, so that you can write to it, and overwrite, when checked, allows you to save,
//replacing the existing file.  These states are not persistant, which forces the user to check the
//box before saving.
enum{ INACTIVE, READONLY, OVERWRITE };
static UINT8 gubOverrideStatus;
//Set when the a new sector/level is selected, forcing the user to reselect the override status.
static BOOLEAN gfOverrideDirty;
//The state of the override button, true if overriden intended.
static BOOLEAN gfOverride;

//The sector coordinates of the map currently loaded in memory (blue)
static SGPSector gsSector;
//The layer of the sector that is currently loaded in memory.
static INT32 gsSectorLayer;
//The sector coordinates of the mouse position (yellow)
static SGPSector gsHiSector;
//The sector coordinates of the selected sector (red)
static SGPSector gsSelSector;

//Used to determine how long the F5 key has been held down for to determine whether or not the
//summary is going to be persistant or not.
static UINT32 guiInitTimer;

static ST::string gszFilename;
static ST::string gszTempFilename;
static ST::string gszDisplayName;

enum{
	SUMMARY_BACKGROUND,
	SUMMARY_OKAY,
	SUMMARY_GRIDCHECKBOX,
	SUMMARY_PROGRESSCHECKBOX,
	SUMMARY_ALL,
	SUMMARY_G,
	SUMMARY_B1,
	SUMMARY_B2,
	SUMMARY_B3,
	SUMMARY_ALTERNATE,
	SUMMARY_LOAD,
	SUMMARY_SAVE,
	SUMMARY_OVERRIDE,
	SUMMARY_UPDATE,
	SUMMARY_SCIFI,
	SUMMARY_REAL,
	SUMMARY_ENEMY,
	NUM_SUMMARY_BUTTONS
};
static GUIButtonRef iSummaryButton[NUM_SUMMARY_BUTTONS];


static GUIButtonRef MakeCheckBox(INT16 const x, INT16 const y, GUI_CALLBACK const click, bool const checked = false)
{
	GUIButtonRef const b = CreateCheckBoxButton(x, y, EDITORDIR "/smcheckbox.sti", MSYS_PRIORITY_HIGH, click);
	if (checked) b->uiFlags |= BUTTON_CLICKED_ON;
	return b;
}


static GUIButtonRef MakeRadioButton(INT16 const x, INT16 const y, GUI_CALLBACK const click)
{
	return CreateCheckBoxButton(x, y, EDITORDIR "/radiobutton.sti", MSYS_PRIORITY_HIGH, click);
}


static void LoadGlobalSummary(void);
static void MapClickCallback(MOUSE_REGION* reg, INT32 reason);
static void MapMoveCallback(MOUSE_REGION* reg, INT32 reason);
static void ReleaseSummaryWindow(void);
static void SummaryEnemyCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryLoadMapCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryOkayCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryOverrideCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryRealCallback(GUI_BUTTON* btn, INT32 reason);
static void SummarySaveMapCallback(GUI_BUTTON* btn, INT32 reason);
static void SummarySciFiCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryToggleAlternateCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryToggleGridCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryToggleLevelCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryToggleProgressCallback(GUI_BUTTON* btn, INT32 reason);
static void SummaryUpdateCallback(GUI_BUTTON* btn, INT32 reason);


void CreateSummaryWindow()
{
	INT32 i;

	if( !gfGlobalSummaryLoaded )
	{
		LoadGlobalSummary();
		gfGlobalSummaryLoaded = TRUE;
	}

	if( gfSummaryWindowActive )
		return;

	DisableEditorTaskbar();
	DisableAllTextFields();

	gsSector = SGPSector(gWorldSectorX, gWorldSectorY);
	gsSelSector = gsSector;
	gfSummaryWindowActive = TRUE;
	gfPersistantSummary = FALSE;
	guiInitTimer = GetJA2Clock();
	gfDeniedSummaryCreation = FALSE;
	gfRenderSummary = TRUE;
	//Create all of the buttons here
	iSummaryButton[SUMMARY_BACKGROUND] = CreateLabel(ST::null, 0, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - EDITOR_TASKBAR_HEIGHT, MSYS_PRIORITY_HIGH - 1);

	iSummaryButton[SUMMARY_OKAY] = CreateTextButton("Okay", FONT12POINT1, FONT_BLACK, FONT_BLACK, 585, 325, 50, 30, MSYS_PRIORITY_HIGH, SummaryOkayCallback);

	iSummaryButton[SUMMARY_GRIDCHECKBOX] = MakeCheckBox(MAP_LEFT, MAP_BOTTOM + 5, SummaryToggleGridCallback, true);
	gfRenderGrid = TRUE;

	iSummaryButton[SUMMARY_PROGRESSCHECKBOX] = MakeCheckBox(MAP_LEFT + 50, MAP_BOTTOM + 5, SummaryToggleProgressCallback, true);
	gfRenderProgress = TRUE;

	iSummaryButton[SUMMARY_ALL] = CreateTextButton("A",  SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 110, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == ALL_LEVELS_MASK || giCurrentViewLevel == ALTERNATE_LEVELS_MASK )
		iSummaryButton[SUMMARY_ALL]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_G]   = CreateTextButton("G",  SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 128, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == GROUND_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_GROUND_MASK )
		iSummaryButton[SUMMARY_G]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_B1]  = CreateTextButton("B1", SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 146, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == BASEMENT1_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_B1_MASK )
		iSummaryButton[SUMMARY_B1]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_B2]  = CreateTextButton("B2", SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 164, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == BASEMENT2_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_B2_MASK )
		iSummaryButton[SUMMARY_B2]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_B3]  = CreateTextButton("B3", SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 182, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == BASEMENT3_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_B3_MASK )
		iSummaryButton[SUMMARY_B3]->uiFlags |= BUTTON_CLICKED_ON;

	iSummaryButton[SUMMARY_ALTERNATE] = MakeCheckBox(MAP_LEFT, MAP_BOTTOM + 25, SummaryToggleAlternateCallback, gfAlternateMaps);

	iSummaryButton[SUMMARY_LOAD] = CreateTextButton("LOAD", FONT12POINT1, FONT_BLACK, FONT_BLACK, MAP_LEFT,      MAP_BOTTOM + 45, 50, 26, MSYS_PRIORITY_HIGH, SummaryLoadMapCallback);
	iSummaryButton[SUMMARY_SAVE] = CreateTextButton("SAVE", FONT12POINT1, FONT_BLACK, FONT_BLACK, MAP_LEFT + 55, MAP_BOTTOM + 45, 50, 26, MSYS_PRIORITY_HIGH, SummarySaveMapCallback);
	iSummaryButton[SUMMARY_OVERRIDE] = MakeCheckBox(MAP_LEFT + 110, MAP_BOTTOM + 59, SummaryOverrideCallback);


	iSummaryButton[SUMMARY_UPDATE] = CreateTextButton("Update", FONT12POINT1, FONT_BLACK, FONT_BLACK, 255, 15, 40, 16, MSYS_PRIORITY_HIGH, SummaryUpdateCallback);

	iSummaryButton[SUMMARY_REAL]  = MakeRadioButton(350, 47, SummaryRealCallback);
	iSummaryButton[SUMMARY_SCIFI] = MakeRadioButton(376, 47, SummarySciFiCallback);
	iSummaryButton[SUMMARY_ENEMY] = MakeRadioButton(350, 60, SummaryEnemyCallback);

	//iSummaryButton[SUMMARY_SCIFI]->SetFastHelpText("Display items that appear in SciFi mode.");
	//iSummaryButton[SUMMARY_REAL ]->SetFastHelpText("Display items that appear in Realistic mode.");
	switch( gubSummaryItemMode )
	{
		case ITEMMODE_SCIFI: iSummaryButton[SUMMARY_SCIFI]->uiFlags |= BUTTON_CLICKED_ON; break;
		case ITEMMODE_REAL:  iSummaryButton[SUMMARY_REAL]->uiFlags  |= BUTTON_CLICKED_ON; break;
		case ITEMMODE_ENEMY: iSummaryButton[SUMMARY_ENEMY]->uiFlags |= BUTTON_CLICKED_ON; break;
	}

	//Init the textinput field.
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	AddUserInputField( NULL );  //just so we can use short cut keys while not typing.
	AddTextInputField(MAP_LEFT + 112, MAP_BOTTOM + 75, 100, 18, MSYS_PRIORITY_HIGH, ST::null, 20, INPUTTYPE_DOSFILENAME);

	for( i = 1; i < NUM_SUMMARY_BUTTONS; i++ )
		iSummaryButton[i]->Hide();

	MSYS_DefineRegion( &MapRegion, MAP_LEFT, MAP_TOP, MAP_RIGHT, MAP_BOTTOM, MSYS_PRIORITY_HIGH, 0,
		MapMoveCallback, MapClickCallback );
	MapRegion.Disable();

	//if( gfItemDetailsMode )
//	{
		gfItemDetailsMode = FALSE;
	//	gfSetupItemDetailsMode = TRUE;
	//}
	if( !gfWorldLoaded )
	{
		gfConfirmExitFirst = FALSE;
		ReleaseSummaryWindow();
		DisableButton( iSummaryButton[ SUMMARY_OKAY ] );
		DisableButton( iSummaryButton[ SUMMARY_SAVE ] );
	}
	if( gfAutoLoadA9 )
	{
		gfAutoLoadA9++;
		gsSelSector = SGPSector(9, 1);
		gpCurrentSectorSummary = gpSectorSummary[ 8 ][ 0 ][ 0 ];
		iSummaryButton[SUMMARY_LOAD]->uiFlags |= BUTTON_CLICKED_ON;
	}
}

void AutoLoadMap()
{
	SummaryLoadMapCallback(iSummaryButton[SUMMARY_LOAD], MSYS_CALLBACK_REASON_LBUTTON_UP);
	if( gfWorldLoaded )
		DestroySummaryWindow();
	gfAutoLoadA9 = FALSE;
	gfConfirmExitFirst = TRUE;
}


static void ReleaseSummaryWindow(void)
{
	INT32 i;
	UINT32 uiCurrTimer;
	if( !gfSummaryWindowActive || gfPersistantSummary )
		return;
	uiCurrTimer = GetJA2Clock();
	if( !gfWorldLoaded || uiCurrTimer - guiInitTimer < 400 )
	{ //make window persistant
		for( i = 1; i < NUM_SUMMARY_BUTTONS; i++ )
			ShowButton( iSummaryButton[ i ] );
		HideButton( iSummaryButton[ SUMMARY_UPDATE ] );
		HideButton( iSummaryButton[ SUMMARY_OVERRIDE ] );
		HideButton( iSummaryButton[ SUMMARY_REAL ] );
		HideButton( iSummaryButton[ SUMMARY_SCIFI ] );
		HideButton( iSummaryButton[ SUMMARY_ENEMY ] );
		MapRegion.Enable();
		gfPersistantSummary = TRUE;
		gfOverrideDirty = TRUE;
		gfRenderSummary = TRUE;
	}
	else
	{
		DestroySummaryWindow();
	}
}


void DestroySummaryWindow()
{
	if (!gfSummaryWindowActive) return;

	FOR_EACH(GUIButtonRef, i, iSummaryButton) RemoveButton(*i);
	MSYS_RemoveRegion(&MapRegion);

	gfSummaryWindowActive = FALSE;
	gfPersistantSummary   = FALSE;
	MarkWorldDirty();
	KillTextInputMode();
	EnableEditorTaskbar();
	EnableAllTextFields();

	gpWorldItemsSummaryArray.clear();
	FreeNullArray(gpPEnemyItemsSummaryArray);
	gusPEnemyItemsSummaryArraySize = 0;
	FreeNullArray(gpNEnemyItemsSummaryArray);
	gusNEnemyItemsSummaryArraySize = 0;

	if (gfWorldLoaded) gfConfirmExitFirst = TRUE;
}


static void RenderSectorInformation(void)
{
	//UINT16 str[ 100 ];
	MAPCREATE_STRUCT *m;
	SUMMARYFILE *s;
	UINT8 ePoints = 0;
	UINT16 usLine = 35;
	INT32 iOverall;

	s = gpCurrentSectorSummary;
	m = &gpCurrentSectorSummary->MapInfo;

	if( m->sNorthGridNo != -1 )
		ePoints++;
	if( m->sEastGridNo != -1 )
		ePoints++;
	if( m->sSouthGridNo != -1 )
		ePoints++;
	if( m->sWestGridNo != -1 )
		ePoints++;
	if( m->sCenterGridNo != -1 )
		ePoints++;
	if( m->sIsolatedGridNo != -1 )
		ePoints++;
	//start at 10,35
	SetFontAttributes(FONT10ARIAL, FONT_ORANGE);
	MPrint(10, 32, ST::format("Tileset:  {}", gTilesets[s->ubTilesetID].zName));
	if( m->ubMapVersion < 10 )
		SetFontForeground( FONT_RED );
	MPrint( 10, 42, ST::format("Version Info:  Summary:  1.{02d},  Map:  {}.{02d}", s->ubSummaryVersion, (INT32)s->dMajorMapVersion, m->ubMapVersion) );
	SetFontForeground( FONT_GRAY2 );
	MPrint( 10, 55, ST::format("Number of items:  {}", s->usNumItems) );
	MPrint( 10, 65, ST::format("Number of lights:  {}", s->usNumLights) );
	MPrint( 10, 75, ST::format("Number of entry points:  {}", ePoints) );
	if( ePoints )
	{
		INT32 x;
		x = 140;
		MPrint( x, 75, "(" );
		x += StringPixLength( "(", FONT10ARIAL ) + 2;
		if (m->sNorthGridNo    != -1) { MPrint(x, 75, "N"); x += StringPixLength("N", FONT10ARIAL) + 2; }
		if (m->sEastGridNo     != -1) { MPrint(x, 75, "E"); x += StringPixLength("E", FONT10ARIAL) + 2; }
		if (m->sSouthGridNo    != -1) { MPrint(x, 75, "S"); x += StringPixLength("S", FONT10ARIAL) + 2; }
		if (m->sWestGridNo     != -1) { MPrint(x, 75, "W"); x += StringPixLength("W", FONT10ARIAL) + 2; }
		if (m->sCenterGridNo   != -1) { MPrint(x, 75, "C"); x += StringPixLength("C", FONT10ARIAL) + 2; }
		if (m->sIsolatedGridNo != -1) { MPrint(x, 75, "I"); x += StringPixLength("I", FONT10ARIAL) + 2; }
		MPrint(x, 75, ")");
	}
	MPrint( 10, 85, ST::format("Number of rooms:  {}", s->ubNumRooms) );
	MPrint( 10, 95, ST::format("Total map population:  {}", m->ubNumIndividuals) );
	MPrint( 20, 105, ST::format("Enemies:  {}", s->EnemyTeam.ubTotal) );
	MPrint( 30, 115, ST::format("Admins:  {}", s->ubNumAdmins) );
	if( s->ubNumAdmins )
		MPrint( 100, 115, ST::format("({} detailed, {} profile -- {} have priority existance)", s->ubAdminDetailed, s->ubAdminProfile, s->ubAdminExistance) );
	MPrint( 30, 125, ST::format("Troops:  {}", s->ubNumTroops) );
	if( s->ubNumTroops )
		MPrint( 100, 125, ST::format("({} detailed, {} profile -- {} have priority existance)", s->ubTroopDetailed, s->ubTroopProfile, s->ubTroopExistance) );
	MPrint( 30, 135, ST::format("Elites:  {}", s->ubNumElites) );
	if( s->ubNumElites )
		MPrint( 100, 135, ST::format("({} detailed, {} profile -- {} have priority existance)", s->ubEliteDetailed, s->ubEliteProfile, s->ubEliteExistance) );
	MPrint( 20, 145, ST::format("Civilians:  {}", s->CivTeam.ubTotal) );
	if( s->CivTeam.ubTotal )
		MPrint( 100, 145, ST::format("({} detailed, {} profile -- {} have priority existance)", s->CivTeam.ubDetailed, s->CivTeam.ubProfile, s->CivTeam.ubExistance) );
	if( s->ubSummaryVersion >= 9 )
	{
		MPrint( 30, 155, ST::format("Humans:  {}", s->CivTeam.ubTotal - s->ubCivCows - s->ubCivBloodcats) );
		MPrint( 30, 165, ST::format("Cows:  {}", s->ubCivCows) );
		MPrint( 30, 175, ST::format("Bloodcats:  {}", s->ubCivBloodcats) );
	}
	MPrint( 20, 185, ST::format("Creatures:  {}", s->CreatureTeam.ubTotal) );
	if( s->ubSummaryVersion >= 9 )
	{
		MPrint( 30, 195, ST::format("Monsters:  {}", s->CreatureTeam.ubTotal - s->CreatureTeam.ubNumAnimals) );
		MPrint( 30, 205, ST::format("Bloodcats:  {}", s->CreatureTeam.ubNumAnimals) );
	}
	MPrint( 10, 215, ST::format("Number of locked and/or trapped doors:  {}", s->ubNumDoors) );
	MPrint( 20, 225, ST::format("Locked:  {}", s->ubNumDoorsLocked) );
	MPrint( 20, 235, ST::format("Trapped:  {}", s->ubNumDoorsTrapped) );
	MPrint( 20, 245, ST::format("Locked & Trapped:  {}", s->ubNumDoorsLockedAndTrapped) );
	if( s->ubSummaryVersion >= 8 )
		MPrint( 10, 255, ST::format("Civilians with schedules:  {}", s->ubCivSchedules) );
	if( s->ubSummaryVersion >= 10 )
	{
		if( s->fTooManyExitGridDests )
		{
			SetFontForeground( FONT_RED );
			MPrint( 10, 265, "Too many exit grid destinations (more than 4)...");
		}
		else
		{
			UINT8 i;
			UINT8 ubNumInvalid = 0;
			for( i = 0; i < 4; i++ )
			{
				if( s->fInvalidDest[i] )
					ubNumInvalid++;
			}
			if( ubNumInvalid )
			{
				SetFontForeground( FONT_RED );
				MPrint( 10, 265, ST::format("ExitGrids:  {} ({} with a long distance destination)", s->ubNumExitGridDests, ubNumInvalid) );
			}
			else switch( s->ubNumExitGridDests )
			{
				case 0:
					MPrint(10, 265, "ExitGrids:  none");
					break;
				case 1:
					MPrint( 10, 265, ST::format("ExitGrids:  1 destination using {} exitgrids", s->usExitGridSize[0]) );
					break;
				case 2:
					MPrint( 10, 265, ST::format("ExitGrids:  2 -- 1) Qty: {}, 2) Qty: {}", s->usExitGridSize[0], s->usExitGridSize[1]) );
					break;
				case 3:
					MPrint( 10, 265, ST::format("ExitGrids:  3 -- 1) Qty: {}, 2) Qty: {}, 3) Qty: {}",
						s->usExitGridSize[0], s->usExitGridSize[1], s->usExitGridSize[2]) );
					break;
				case 4:
					MPrint( 10, 265, ST::format("ExitGrids:  3 -- 1) Qty: {}, 2) Qty: {}, 3) Qty: {}, 4) Qty: {}",
						s->usExitGridSize[0], s->usExitGridSize[1], s->usExitGridSize[2], s->usExitGridSize[3]) );
					break;
			}
		}
	}
	iOverall = - ( 2 * s->EnemyTeam.ubBadA ) - s->EnemyTeam.ubPoorA + s->EnemyTeam.ubGoodA + ( 2 * s->EnemyTeam.ubGreatA );
	usLine = 275;
	MPrint( 10, usLine, ST::format("Enemy Relative Attributes:  {} bad, {} poor, {} norm, {} good, {} great ({+d} Overall)",
		s->EnemyTeam.ubBadA,
		s->EnemyTeam.ubPoorA,
		s->EnemyTeam.ubAvgA,
		s->EnemyTeam.ubGoodA,
		s->EnemyTeam.ubGreatA,
		iOverall) );
	iOverall = - ( 2 * s->EnemyTeam.ubBadE ) - s->EnemyTeam.ubPoorE + s->EnemyTeam.ubGoodE + ( 2 * s->EnemyTeam.ubGreatE );
	usLine += 10;
	MPrint( 10, usLine, ST::format("Enemy Relative Equipment:  {} bad, {} poor, {} norm, {} good, {} great ({+d} Overall)",
		s->EnemyTeam.ubBadE,
		s->EnemyTeam.ubPoorE,
		s->EnemyTeam.ubAvgE,
		s->EnemyTeam.ubGoodE,
		s->EnemyTeam.ubGreatE,
		iOverall) );
	usLine += 10;
	if( s->ubSummaryVersion >= 11 )
	{
		if( s->ubEnemiesReqWaypoints )
		{
			SetFontForeground( FONT_RED );
			MPrint( 10, usLine, ST::format("{} placements have patrol orders without any waypoints defined.", s->ubEnemiesReqWaypoints) );
			usLine += 10;
		}
	}
	if( s->ubSummaryVersion >= 13 )
	{
		if( s->ubEnemiesHaveWaypoints )
		{
			SetFontForeground( FONT_RED );
			MPrint( 10, usLine, ST::format("{} placements have waypoints, but without any patrol orders.", s->ubEnemiesHaveWaypoints) );
			usLine += 10;
		}
	}
	if( s->ubSummaryVersion >= 12 )
	{
		if( s->usWarningRoomNums )
		{
			SetFontForeground( FONT_RED );
			MPrint( 10, usLine, ST::format("{} gridnos have questionable room numbers.  Please validate.", s->usWarningRoomNums) );
		}
	}
}


//2)  CODE TRIGGER/ACTION NAMES
static void RenderItemDetails(void)
{
	FLOAT dAvgExistChance, dAvgStatus;
	OBJECTTYPE *pItem;
	INT32 index, i;
	UINT32 uiQuantity, uiExistChance, uiStatus;
	UINT32 xp, yp;
	INT8 bFreqIndex;
	SetFontAttributes(FONT10ARIAL, FONT_GRAY2);
	MPrint(364, 49, "R");
	MPrint(390, 49, "S");
	MPrint(364, 62, "Enemy");
	yp = 20;
	xp = 5;
	if (gubSummaryItemMode != ITEMMODE_ENEMY)
	{
		UINT32 uiTriggerQuantity[8] {};
		UINT32 uiActionQuantity[8] {};
		UINT32 uiTriggerExistChance[8] {};
		UINT32 uiActionExistChance[8] {};
		for( index = 1; index < MAXITEMS; index++ )
		{
			uiQuantity = 0;
			uiExistChance = 0;
			uiStatus = 0;
			Assert(gpWorldItemsSummaryArray.size() <= INT32_MAX);
			for (i = 0; i < static_cast<INT32>(gpWorldItemsSummaryArray.size()); i++)
			{
				if( index == SWITCH || index == ACTION_ITEM )
				{
					if( gpWorldItemsSummaryArray[ i ].o.usItem == index )
					{
						if( (gubSummaryItemMode == ITEMMODE_SCIFI && !(gpWorldItemsSummaryArray[ i ].usFlags & WORLD_ITEM_REALISTIC_ONLY)) ||
								(gubSummaryItemMode == ITEMMODE_REAL && !(gpWorldItemsSummaryArray[ i ].usFlags & WORLD_ITEM_SCIFI_ONLY)) )
						{
							pItem = &gpWorldItemsSummaryArray[ i ].o;
							if( !pItem->bFrequency )
								bFreqIndex = 7;
							else if( pItem->bFrequency == PANIC_FREQUENCY )
								bFreqIndex = 0;
							else if( pItem->bFrequency == PANIC_FREQUENCY_2 )
								bFreqIndex = 1;
							else if( pItem->bFrequency == PANIC_FREQUENCY_3 )
								bFreqIndex = 2;
							else if( pItem->bFrequency == FIRST_MAP_PLACED_FREQUENCY + 1 )
								bFreqIndex = 3;
							else if( pItem->bFrequency == FIRST_MAP_PLACED_FREQUENCY + 2 )
								bFreqIndex = 4;
							else if( pItem->bFrequency == FIRST_MAP_PLACED_FREQUENCY + 3 )
								bFreqIndex = 5;
							else if( pItem->bFrequency == FIRST_MAP_PLACED_FREQUENCY + 4 )
								bFreqIndex = 6;
							else
								continue;
							if( index == SWITCH )
							{
								uiTriggerQuantity[ bFreqIndex ]++;
								uiTriggerExistChance[ bFreqIndex ] += 100 - gpWorldItemsSummaryArray[ i ].ubNonExistChance;
							}
							else
							{
								uiActionQuantity[ bFreqIndex ]++;
								uiActionExistChance[ bFreqIndex ] += 100 - gpWorldItemsSummaryArray[ i ].ubNonExistChance;
							}
						}
					}
					continue;
				}
				if( gpWorldItemsSummaryArray[ i ].o.usItem == index )
				{
					if( (gubSummaryItemMode == ITEMMODE_SCIFI && !(gpWorldItemsSummaryArray[ i ].usFlags & WORLD_ITEM_REALISTIC_ONLY)) ||
						(gubSummaryItemMode == ITEMMODE_REAL  && !(gpWorldItemsSummaryArray[ i ].usFlags & WORLD_ITEM_SCIFI_ONLY)) )
					{
						pItem = &gpWorldItemsSummaryArray[ i ].o;
						uiExistChance += (100 - gpWorldItemsSummaryArray[ i ].ubNonExistChance) * pItem->ubNumberOfObjects;
						uiStatus += pItem->bStatus[0];
						uiQuantity += pItem->ubNumberOfObjects;
					}
				}
			}
			if( uiQuantity )
			{
				if( !(yp % 20) )
					SetFontForeground( FONT_LTKHAKI );
				else
					SetFontForeground( FONT_GRAY2 );
				//calc averages
				dAvgExistChance = (FLOAT)(uiExistChance / 100.0);
				dAvgStatus = uiStatus / (FLOAT)uiQuantity;
				//Display stats.
				MPrint(xp, yp, ShortItemNames[index]);
				MPrint( xp + 85, yp, ST::format("{3.02f}", dAvgExistChance) );
				MPrint( xp + 110, yp, ST::format("@ {3.02f}%", dAvgStatus) );
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, "TOO MANY ITEMS TO DISPLAY!");
						return;
					}
				}
			}
		}
		//Now list the number of actions/triggers of each type
		for( i = 0; i < 8; i++ )
		{
			if( uiTriggerQuantity[i] || uiActionQuantity[i] )
			{
				if( i == 7 )
					SetFontForeground( FONT_DKYELLOW );
				else if( !uiTriggerQuantity[i] || !uiActionQuantity[i] )
					SetFontForeground( FONT_RED );
				else
					SetFontForeground( 77 );
				ST::string Type;
				switch( i )
				{
					case 0: Type = "Panic1";           break;
					case 1:	Type = "Panic2";           break;
					case 2:	Type = "Panic3";           break;
					case 3:	Type = "Norm1";            break;
					case 4:	Type = "Norm2";            break;
					case 5:	Type = "Norm3";            break;
					case 6:	Type = "Norm4";            break;
					case 7:	Type = "Pressure Actions"; break;
				}
				if( i < 7 )
				{
					dAvgExistChance = (FLOAT)(uiTriggerExistChance[i] / 100.0);
					dAvgStatus = (FLOAT)(uiActionExistChance[i] / 100.0);
					MPrint(xp, yp, ST::format("{}:  {3.02f} trigger(s), {3.02f} action(s)", Type, dAvgExistChance, dAvgStatus));
				}
				else
				{
					dAvgExistChance = (FLOAT)(uiActionExistChance[i] / 100.0);
					MPrint(xp, yp, ST::format("{}:  {3.02f}", Type, dAvgExistChance));
				}
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, "TOO MANY ITEMS TO DISPLAY!");
						return;
					}
				}
			}
		}
	}
	else if( gubSummaryItemMode == ITEMMODE_ENEMY )
	{

		SetFontForeground( FONT_YELLOW );
		MPrint(xp, yp, "PRIORITY ENEMY DROPPED ITEMS");
		yp += 10;

		//Do the priority existance guys first
		if( !gpPEnemyItemsSummaryArray )
		{
			SetFontForeground( FONT_DKYELLOW );
			MPrint(xp, yp, "None");
			yp += 10;
		}
		else for( index = 1; index < MAXITEMS; index++ )
		{
			uiQuantity = 0;
			uiExistChance = 0;
			uiStatus = 0;
			for( i = 0; i < gusPEnemyItemsSummaryArraySize; i++ )
			{
				if( gpPEnemyItemsSummaryArray[ i ].usItem == index )
				{
					pItem = &gpPEnemyItemsSummaryArray[ i ];
					uiExistChance += 100 * pItem->ubNumberOfObjects;
					uiStatus += pItem->bStatus[0];
					uiQuantity += pItem->ubNumberOfObjects;
				}
			}
			if( uiQuantity )
			{
				if( !(yp % 20) )
					SetFontForeground( FONT_LTKHAKI );
				else
					SetFontForeground( FONT_GRAY2 );
				//calc averages
				dAvgExistChance = (FLOAT)(uiExistChance / 100.0);
				dAvgStatus = uiStatus / (FLOAT)uiQuantity;
				//Display stats.
				MPrint(xp, yp, ShortItemNames[index]);
				MPrint( xp + 85, yp, ST::format("{3.02f}", dAvgExistChance) );
				MPrint( xp + 110, yp, ST::format("@ {3.02f}%", dAvgStatus) );
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, "TOO MANY ITEMS TO DISPLAY!");
						return;
					}
				}
			}
		}

		yp += 5;

		SetFontForeground( FONT_YELLOW );
		MPrint(xp, yp, "NORMAL ENEMY DROPPED ITEMS");
		yp += 10;
		if( yp >= 355 )
		{
			xp += 170;
			yp = 20;
			if( xp >= 300 )
			{
				SetFontForeground( FONT_RED );
				MPrint(350, 350, "TOO MANY ITEMS TO DISPLAY!");
				return;
			}
		}

		//Do the priority existance guys first
		if( !gpNEnemyItemsSummaryArray )
		{
			SetFontForeground( FONT_DKYELLOW );
			MPrint(xp, yp, "None");
			yp += 10;
		}
		for( index = 1; index < MAXITEMS; index++ )
		{
			uiQuantity = 0;
			uiExistChance = 0;
			uiStatus = 0;
			for( i = 0; i < gusNEnemyItemsSummaryArraySize; i++ )
			{
				if( gpNEnemyItemsSummaryArray[ i ].usItem == index )
				{
					pItem = &gpNEnemyItemsSummaryArray[ i ];
					uiExistChance += 100 * pItem->ubNumberOfObjects;
					uiStatus += pItem->bStatus[0];
					uiQuantity += pItem->ubNumberOfObjects;
				}
			}
			if( uiQuantity )
			{
				if( !(yp % 20) )
					SetFontForeground( FONT_LTKHAKI );
				else
					SetFontForeground( FONT_GRAY2 );
				//calc averages
				dAvgExistChance = (FLOAT)(uiExistChance / 100.0);
				dAvgStatus = uiStatus / (FLOAT)uiQuantity;
				//Display stats.
				MPrint(xp, yp, ShortItemNames[index]);
				MPrint( xp + 85, yp, ST::format("{3.02f}", dAvgExistChance) );
				MPrint( xp + 110, yp, ST::format("@ {3.02f}%", dAvgStatus) );
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, "TOO MANY ITEMS TO DISPLAY!");
						return;
					}
				}
			}
		}



	}
	else
	{
		SetFontForeground( FONT_RED );
		MPrint(5, 50, "ERROR:  Can't load the items for this map.  Reason unknown.");
	}
}


static void CalculateOverrideStatus(void);
static void ExtractTempFilename(void);
static void SetupItemDetailsMode(BOOLEAN fAllowRecursion);


void RenderSummaryWindow()
{
	SGPRect ClipRect;
	INT32 i, x, y;
	if( (GetActiveFieldID() == 1 ) != gfTempFile )
	{
		gfTempFile ^= 1;
		SetInputFieldString( 1, ST::null );
		gfRenderSummary = TRUE;
	}
	if( gfTempFile ) //constantly extract the temp filename for updating purposes.
		ExtractTempFilename();
	if( gfRenderSummary )
	{
		gfRenderSummary = FALSE;
		gfRenderMap = TRUE;
		for( i = 1; i < NUM_SUMMARY_BUTTONS; i++ )
		{
			MarkAButtonDirty( iSummaryButton[ i ] );
		}

		iSummaryButton[SUMMARY_BACKGROUND]->Draw();
		InvalidateRegion(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - EDITOR_TASKBAR_HEIGHT);

		SetFontAttributes(BLOCKFONT2, FONT_LTKHAKI, FONT_DKKHAKI);
		if( !gfItemDetailsMode )
		{
			MPrint(10, 5, ST::format("CAMPAIGN EDITOR -- {} Version 1.{02d}",
				gszVersionType[ GLOBAL_SUMMARY_STATE ], GLOBAL_SUMMARY_VERSION) );
		}

		//This section builds the proper header to be displayed for an existing global summary.
		if( !gfWorldLoaded )
		{
			SetFontForeground( FONT_RED );
			SetFontShadow( FONT_NEARBLACK );
			MPrint(270, 5, "(NO MAP LOADED).");
		}
		SetFont( FONT10ARIAL );
		SetFontShadow( FONT_NEARBLACK );
		if( gfGlobalSummaryExists )
		{
			ST::string str;
			BOOLEAN fSectorSummaryExists = FALSE;
			if( gusNumEntriesWithOutdatedOrNoSummaryInfo && !gfOutdatedDenied )
			{
				DisableButton( iSummaryButton[ SUMMARY_LOAD ] );
				SetFontForeground( FONT_YELLOW );
				MPrint( 10, 20, ST::format("You currently have {} outdated maps.", gusNumEntriesWithOutdatedOrNoSummaryInfo));
				MPrint( 10, 30, "The more maps that need to be updated, the longer it takes.  It'll take ");
				MPrint( 10, 40, "approximately 4 minutes on a P200MMX to analyse 100 maps, so");
				MPrint( 10, 50, "depending on your computer, it may vary.");
				SetFontForeground( FONT_LTRED );
				MPrint( 10, 65, "Do you wish to regenerate info for ALL these maps at this time (y/n)?" );
			}
			else if ((!gsSelSector.x && !gsSector.x) || gfTempFile)
			{
				DisableButton( iSummaryButton[ SUMMARY_LOAD ] );
				SetFontForeground( FONT_LTRED );
				MPrint(10, 20, "There is no sector currently selected.");
				if( gfTempFile )
				{
					SetFontForeground( FONT_YELLOW );
					MPrint(10, 30, "Entering a temp file name that doesn't follow campaign editor conventions...");
					goto SPECIALCASE_LABEL;  //OUCH!!!
				}
				else if( !gfWorldLoaded )
				{
					SetFontForeground( FONT_YELLOW );
					MPrint(10, 30, "You need to either load an existing map or create a new map before being");
					MPrint(10, 40, "able to enter the editor, or you can quit (ESC or Alt+x).");
				}
			}
			else
			{
				//Build sector string
				if (gsSelSector.x)
					x = gsSelSector.x - 1, y = gsSelSector.y - 1;
				else
					x = gsSector.x - 1, y = gsSector.y - 1;
				str = ST::format("{c}{}", y + 'A', x + 1);
				gszFilename = str;
				giCurrLevel = giCurrentViewLevel;
				switch( giCurrentViewLevel )
				{
					case ALL_LEVELS_MASK:
						//search for the highest level
						fSectorSummaryExists = TRUE;
						if( gbSectorLevels[x][y] & GROUND_LEVEL_MASK )
							giCurrLevel = GROUND_LEVEL_MASK;
						else if( gbSectorLevels[x][y] & BASEMENT1_LEVEL_MASK )
							giCurrLevel = BASEMENT1_LEVEL_MASK;
						else if( gbSectorLevels[x][y] & BASEMENT2_LEVEL_MASK )
							giCurrLevel = BASEMENT2_LEVEL_MASK;
						else if( gbSectorLevels[x][y] & BASEMENT3_LEVEL_MASK )
							giCurrLevel = BASEMENT3_LEVEL_MASK;
						else
							fSectorSummaryExists = FALSE;
						break;
					case ALTERNATE_LEVELS_MASK:
						//search for the highest alternate level
						fSectorSummaryExists = TRUE;
						if( gbSectorLevels[x][y] & ALTERNATE_GROUND_MASK )
							giCurrLevel = ALTERNATE_GROUND_MASK;
						else if( gbSectorLevels[x][y] & ALTERNATE_B1_MASK )
							giCurrLevel = ALTERNATE_B1_MASK;
						else if( gbSectorLevels[x][y] & ALTERNATE_B2_MASK )
							giCurrLevel = ALTERNATE_B2_MASK;
						else if( gbSectorLevels[x][y] & ALTERNATE_B3_MASK )
							giCurrLevel = ALTERNATE_B3_MASK;
						else
							fSectorSummaryExists = FALSE;
						break;
					case GROUND_LEVEL_MASK:
						if( gbSectorLevels[x][y] & GROUND_LEVEL_MASK )
							fSectorSummaryExists = TRUE;
						break;
					case BASEMENT1_LEVEL_MASK:
						if( gbSectorLevels[x][y] & BASEMENT1_LEVEL_MASK )
							fSectorSummaryExists = TRUE;
						break;
					case BASEMENT2_LEVEL_MASK:
						if( gbSectorLevels[x][y] & BASEMENT2_LEVEL_MASK )
							fSectorSummaryExists = TRUE;
						break;
					case BASEMENT3_LEVEL_MASK:
						if( gbSectorLevels[x][y] & BASEMENT3_LEVEL_MASK )
							fSectorSummaryExists = TRUE;
						break;
					case ALTERNATE_GROUND_MASK:
						if( gbSectorLevels[x][y] & ALTERNATE_GROUND_MASK )
							fSectorSummaryExists = TRUE;
						break;
					case ALTERNATE_B1_MASK:
						if( gbSectorLevels[x][y] & ALTERNATE_B1_MASK )
							fSectorSummaryExists = TRUE;
						break;
					case ALTERNATE_B2_MASK:
						if( gbSectorLevels[x][y] & ALTERNATE_B2_MASK )
							fSectorSummaryExists = TRUE;
						break;
					case ALTERNATE_B3_MASK:
						if( gbSectorLevels[x][y] & ALTERNATE_B3_MASK )
							fSectorSummaryExists = TRUE;
						break;
				}
				if( gbSectorLevels[x][y] )
				{
					switch( giCurrLevel )
					{
						case GROUND_LEVEL_MASK:
							giCurrLevel = 0;
							str += ", ground level";
							gpCurrentSectorSummary = gpSectorSummary[x][y][0];
							break;
						case BASEMENT1_LEVEL_MASK:
							giCurrLevel = 1;
							str += ", underground level 1";
							gpCurrentSectorSummary = gpSectorSummary[x][y][1];
							break;
						case BASEMENT2_LEVEL_MASK:
							giCurrLevel = 2;
							str += ", underground level 2";
							gpCurrentSectorSummary = gpSectorSummary[x][y][2];
							break;
						case BASEMENT3_LEVEL_MASK:
							giCurrLevel = 3;
							str += ", underground level 3";
							gpCurrentSectorSummary = gpSectorSummary[x][y][3];
							break;
						case ALTERNATE_GROUND_MASK:
							giCurrLevel = 4;
							str += ", alternate G level";
							gpCurrentSectorSummary = gpSectorSummary[x][y][4];
							break;
						case ALTERNATE_B1_MASK:
							giCurrLevel = 5;
							str += ", alternate B1 level";
							gpCurrentSectorSummary = gpSectorSummary[x][y][5];
							break;
						case ALTERNATE_B2_MASK:
							giCurrLevel = 6;
							str += ", alternate B2 level";
							gpCurrentSectorSummary = gpSectorSummary[x][y][6];
							break;
						case ALTERNATE_B3_MASK:
							giCurrLevel = 7;
							str += ", alternate B3 level";
							gpCurrentSectorSummary = gpSectorSummary[x][y][7];
							break;
					}
				}
				else
				{
					DisableButton( iSummaryButton[ SUMMARY_LOAD ] );
				}
				if( fSectorSummaryExists )
				{
					switch( giCurrLevel )
					{
						case 0:	gszFilename += ".dat";				break;
						case 1:	gszFilename += "_b1.dat";		break;
						case 2:	gszFilename += "_b2.dat";		break;
						case 3:	gszFilename += "_b3.dat";		break;
						case 4:	gszFilename += "_a.dat";			break;
						case 5:	gszFilename += "_b1_a.dat";	break;
						case 6:	gszFilename += "_b2_a.dat";	break;
						case 7:	gszFilename += "_b3_a.dat";	break;
					}
					gszDisplayName = gszFilename;
					EnableButton( iSummaryButton[ SUMMARY_LOAD ] );
					if( gpCurrentSectorSummary )
					{
						if( gpCurrentSectorSummary->ubSummaryVersion < GLOBAL_SUMMARY_VERSION )
							ShowButton( iSummaryButton[ SUMMARY_UPDATE ] );
						else
							HideButton( iSummaryButton[ SUMMARY_UPDATE ] );
						if( !gfAlternateMaps )
							SetFontForeground( FONT_YELLOW );
						else
							SetFontForeground( FONT_LTBLUE );
						if( gfItemDetailsMode )
						{
							if( gfSetupItemDetailsMode )
							{
								SetupItemDetailsMode( TRUE );
								gfSetupItemDetailsMode = FALSE;
							}
							MPrint(10, 5, ST::format("ITEM DETAILS -- sector {}", str));
							RenderItemDetails();
						}
						else
						{
							MPrint(10, 20, ST::format("Summary Information for sector {}:", str));
							HideButton( iSummaryButton[ SUMMARY_REAL ] );
							HideButton( iSummaryButton[ SUMMARY_SCIFI ] );
							HideButton( iSummaryButton[ SUMMARY_ENEMY ] );
							RenderSectorInformation();
						}
					}
					else
					{
						SetFontForeground( FONT_RED );
						INT32 const y = (gfItemDetailsMode ? 5 : 20);
						MPrint(10, y, ST::format("Summary Information for sector {}" , str));
						MPrint( 10, y + 10, "does not exist.");
						ShowButton( iSummaryButton[ SUMMARY_UPDATE ] );
					}
				}
				else
				{
					if( !gfAlternateMaps )
						SetFontForeground( FONT_ORANGE );
					else
					{
						SetFontForeground( FONT_DKBLUE );
						SetFontShadow( 0 );
					}
					if( gfItemDetailsMode )
						MPrint(10, 5, ST::format("No information exists for sector {}.", str));
					else
						MPrint(10, 20, ST::format("No information exists for sector {}.", str));
					SetFontShadow( FONT_NEARBLACK );

					switch( giCurrentViewLevel )
					{
						case ALL_LEVELS_MASK:
						case GROUND_LEVEL_MASK:			gszFilename += ".dat";				break;
						case BASEMENT1_LEVEL_MASK:	gszFilename += "_b1.dat";		break;
						case BASEMENT2_LEVEL_MASK:	gszFilename += "_b2.dat";		break;
						case BASEMENT3_LEVEL_MASK:	gszFilename += "_b3.dat";		break;
						case ALTERNATE_LEVELS_MASK:
						case ALTERNATE_GROUND_MASK:	gszFilename += "_a.dat";			break;
						case ALTERNATE_B1_MASK:			gszFilename += "_b1_a.dat";	break;
						case ALTERNATE_B2_MASK:			gszFilename += "_b2_a.dat";	break;
						case ALTERNATE_B3_MASK:			gszFilename += "_b3_a.dat";	break;
					}
					gszDisplayName = gszFilename;
					DisableButton( iSummaryButton[ SUMMARY_LOAD ] );
				}
				SPECIALCASE_LABEL:
				if( gfOverrideDirty && gfPersistantSummary )
					CalculateOverrideStatus();
				if( gubOverrideStatus == INACTIVE )
				{
					if( !gfAlternateMaps )
						SetFontForeground( FONT_LTKHAKI );
					else
						SetFontForeground( FONT_LTBLUE );
					MPrint(MAP_LEFT + 110, MAP_BOTTOM + 55, ST::format("FILE:  {}", gszDisplayName));
				}
				else //little higher to make room for the override checkbox and text.
				{
					if( !gfAlternateMaps )
						SetFontForeground( FONT_LTKHAKI );
					else
						SetFontForeground( FONT_LTBLUE );
					MPrint(MAP_LEFT + 110, MAP_BOTTOM + 46, ST::format("FILE:  {}", gszDisplayName));
					if( gubOverrideStatus == READONLY )
					{
						SetFontForeground(gfOverride ? FONT_YELLOW : FONT_LTRED);
						MPrint(MAP_LEFT + 124, MAP_BOTTOM + 61, "Override READONLY");
					}
					else
					{
						SetFontForeground(gfOverride ? FONT_YELLOW: FONT_ORANGE);
						MPrint(MAP_LEFT + 124, MAP_BOTTOM + 61, "Overwrite File");
					}
				}
			}
		}
		else if( !gfDeniedSummaryCreation )
		{
			SetFontForeground( FONT_GRAY1 );
			MPrint(10, 20, "You currently have no summary data.  By creating one, you will be able to keep track");
			MPrint(10, 30, "of information pertaining to all of the sectors you edit and save.  The creation process");
			MPrint(10, 40, "will analyse all maps in your /MAPS directory, and generate a new one.  This could");
			MPrint(10, 50, "take a few minutes depending on how many valid maps you have.  Valid maps are");
			MPrint(10, 60, "maps following the proper naming convention from a1.dat - p16.dat.  Underground maps");
			MPrint(10, 70, "are signified by appending _b1 to _b3 before the .dat (ex:  a9_b1.dat).");
			SetFontForeground( FONT_LTRED );
			MPrint(10, 85, "Do you wish to do this now (y/n)?");
		}
		else
		{
			SetFontForeground( FONT_LTRED );
			MPrint(10, 20, "No summary info.  Creation denied.");
		}

		SetFont( FONT10ARIAL );
		SetFontForeground( FONT_GRAY3 );
		MPrint(MAP_LEFT + 15, MAP_BOTTOM +  7, "Grid");
		MPrint(MAP_LEFT + 65, MAP_BOTTOM +  7, "Progress");
		MPrint(MAP_LEFT + 15, MAP_BOTTOM + 27, "Use Alternate Maps");
		//Draw the mode tabs
		SetFontForeground( FONT_YELLOW );
		MPrint(354, 18, "Summary");
		{ SGPVSurface::Lock l(FRAME_BUFFER);
			SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			RectangleDraw(TRUE, 350, 15, 405, 28, 0, l.Buffer<UINT16>());
		}
		FRAME_BUFFER->ShadowRectUsingLowPercentTable(351, 16, 404, 27);
		if( gpCurrentSectorSummary )
			/*&& gpCurrentSectorSummary->usNumItems ||
				gpPEnemyItemsSummaryArray && gusPEnemyItemsSummaryArraySize ||
				gpNEnemyItemsSummaryArray && gusNEnemyItemsSummaryArraySize )*/
		{
			SetFontForeground( FONT_YELLOW );
		}
		else
		{
			SetFontForeground( FONT_RED );
		}
		MPrint(354, 33, "Items");
		{ SGPVSurface::Lock l(FRAME_BUFFER);
			SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			RectangleDraw(TRUE, 350, 30, 405, 43, 0, l.Buffer<UINT16>());
		}
		if( gpCurrentSectorSummary )
			/*&& gpCurrentSectorSummary->usNumItems ||
				gpPEnemyItemsSummaryArray && gusPEnemyItemsSummaryArraySize ||
				gpNEnemyItemsSummaryArray && gusNEnemyItemsSummaryArraySize )
				*/
		{
			FRAME_BUFFER->ShadowRectUsingLowPercentTable(351, 31, 404, 42);
		}
		else
		{
			FRAME_BUFFER->ShadowRect(351, 31, 404, 42);
		}
		SetFontForeground( FONT_GRAY2 );
	}

	RenderButtons();

	if( gfRenderMap )
	{
		gfRenderMap = FALSE;
		BltVideoObject(FRAME_BUFFER, guiOmertaMap, 0, MAP_LEFT - 2, MAP_TOP - 2);
		InvalidateRegion( MAP_LEFT-1, MAP_TOP-1, MAP_RIGHT+1, MAP_BOTTOM+1 );
		//Draw the coordinates
		SetFont( SMALLCOMPFONT );
		SetFontForeground( FONT_BLACK );
		for( y = 0; y < 16; y++ )
		{
			MPrint( MAP_LEFT-8, MAP_TOP+4+y*13, ST::format("{c}", 65 + y) );
		}
		for( x = 1; x <= 16; x++ )
		{
			ST::string str = ST::format("{}", x);
			MPrint( MAP_LEFT+x*13-(13+StringPixLength( str, SMALLCOMPFONT ))/2, MAP_TOP-8, str );
		}
		if( gfRenderGrid )
		{
			UINT16 pos;
			SGPVSurface::Lock l(FRAME_BUFFER);
			UINT16* const pDestBuf = l.Buffer<UINT16>();
			SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			for( i = 1; i <= 15; i++ )
			{
				//draw vertical lines
				pos = (UINT16)(i * 13 + MAP_LEFT);
				LineDraw( TRUE, pos, MAP_TOP, pos, MAP_BOTTOM-1, 0, pDestBuf );
				//draw horizontal lines
				pos = (UINT16)(i * 13 + MAP_TOP);
				LineDraw( TRUE, MAP_LEFT, pos, MAP_RIGHT-1, pos, 0, pDestBuf );
			}
		}
		if( gfRenderProgress )
		{
			UINT8 ubNumUndergroundLevels;
			for( y = 0; y < 16; y++ )
			{
				ClipRect.iTop = MAP_TOP + y*13;
				ClipRect.iBottom = ClipRect.iTop + 12;
				for( x = 0; x < 16; x++ )
				{
					if( giCurrentViewLevel == ALL_LEVELS_MASK )
					{
						ubNumUndergroundLevels = 0;
						if( gbSectorLevels[x][y] & BASEMENT1_LEVEL_MASK )		ubNumUndergroundLevels++;
						if( gbSectorLevels[x][y] & BASEMENT2_LEVEL_MASK )		ubNumUndergroundLevels++;
						if( gbSectorLevels[x][y] & BASEMENT3_LEVEL_MASK )		ubNumUndergroundLevels++;
						if( ubNumUndergroundLevels )
						{ //display the number of underground levels.  If there
							//is no ground level, then it'll be shadowed.
							SetFont( SMALLCOMPFONT );
							SetFontForeground( FONT_YELLOW );
							MPrint(MAP_LEFT + x * 13 + 4, ClipRect.iTop + 4, ST::format("{}", ubNumUndergroundLevels));
						}
						if( gbSectorLevels[x][y] & GROUND_LEVEL_MASK )
						{
							if( !gfItemDetailsMode || !gpSectorSummary[x][y][0] || gpSectorSummary[x][y][0]->usNumItems )
								continue;
						}
					}
					else if( giCurrentViewLevel == ALTERNATE_LEVELS_MASK )
					{
						ubNumUndergroundLevels = 0;
						if( gbSectorLevels[x][y] & ALTERNATE_B1_MASK )		ubNumUndergroundLevels++;
						if( gbSectorLevels[x][y] & ALTERNATE_B2_MASK )		ubNumUndergroundLevels++;
						if( gbSectorLevels[x][y] & ALTERNATE_B3_MASK )		ubNumUndergroundLevels++;
						if( ubNumUndergroundLevels )
						{ //display the number of underground levels.  If there
							//is no ground level, then it'll be shadowed.
							SetFont( SMALLCOMPFONT );
							SetFontForeground( FONT_YELLOW );
							MPrint(MAP_LEFT + x * 13 + 4, ClipRect.iTop + 4, ST::format("{}", ubNumUndergroundLevels));
						}
						if( gbSectorLevels[x][y] & ALTERNATE_GROUND_MASK )
						{
							if( !gfItemDetailsMode || !gpSectorSummary[x][y][4] || gpSectorSummary[x][y][4]->usNumItems )
								continue;
						}
					}
					else if( gbSectorLevels[x][y] & giCurrentViewLevel )
					{
						if( !gfItemDetailsMode || !gpSectorSummary[x][y][giCurrLevel] || gpSectorSummary[x][y][giCurrLevel]->usNumItems )
							continue;
					}
					ClipRect.iLeft = MAP_LEFT + x*13;
					ClipRect.iRight = ClipRect.iLeft + 12;
					FRAME_BUFFER->ShadowRect(ClipRect.iLeft, ClipRect.iTop, ClipRect.iRight, ClipRect.iBottom);
					if( giCurrentViewLevel == BASEMENT1_LEVEL_MASK ||
						giCurrentViewLevel == BASEMENT2_LEVEL_MASK ||
						giCurrentViewLevel == BASEMENT3_LEVEL_MASK ||
						giCurrentViewLevel == ALTERNATE_B1_MASK ||
						giCurrentViewLevel == ALTERNATE_B2_MASK ||
						giCurrentViewLevel == ALTERNATE_B3_MASK)
					{
						FRAME_BUFFER->ShadowRect(ClipRect.iLeft, ClipRect.iTop, ClipRect.iRight, ClipRect.iBottom);
					}
				}
			}
		}
	}

	if( gfGlobalSummaryExists )
	{
		SGPVSurface::Lock l(FRAME_BUFFER);
		UINT16* const pDestBuf = l.Buffer<UINT16>();
		SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		//Render the grid for the map currently residing in memory (blue).
		if (gfWorldLoaded && !gfTempFile && gsSector.x)
		{
			x = MAP_LEFT + (gsSector.x - 1) * 13 + 1;
			y = MAP_TOP + (gsSector.y - 1) * 13 + 1;
			RectangleDraw( TRUE, x, y, x+11, y+11, Get16BPPColor( FROMRGB( 50, 50, 200 ) ), pDestBuf );
		}

		//Render the grid for the sector currently in focus (red).
		if (gsSelSector.x > 0 && !gfTempFile)
		{
			x = MAP_LEFT + (gsSelSector.x - 1) * 13;
			y = MAP_TOP + (gsSelSector.y - 1) * 13;
			RectangleDraw( TRUE, x, y, x+13, y+13, Get16BPPColor( FROMRGB( 200, 50, 50 ) ), pDestBuf );
		}

		//Render the grid for the sector if the mouse is over it (yellow).
		if (gsHiSector.x > 0)
		{
			x = MAP_LEFT + (gsHiSector.x - 1) * 13 - 1;
			y = MAP_TOP + (gsHiSector.y - 1) * 13 - 1;
			RectangleDraw( TRUE, x, y, x+15, y+15, Get16BPPColor( FROMRGB( 200, 200, 50 ) ), pDestBuf );
		}
	}
	//Check to see if the user clicked on one of the hot spot mode change areas.
	if( gfLeftButtonState )
	{
		if( !gfItemDetailsMode )
		{
			if( gpCurrentSectorSummary )
				/*&& gpCurrentSectorSummary->usNumItems ||
					gpPEnemyItemsSummaryArray && gusPEnemyItemsSummaryArraySize ||
					gpNEnemyItemsSummaryArray && gusNEnemyItemsSummaryArraySize )*/
			{
				if( gusMouseXPos >= 350 && gusMouseYPos >= 30 && gusMouseXPos <= 404 && gusMouseYPos <= 42 )
				{
					gfItemDetailsMode = TRUE;
					gfSetupItemDetailsMode = TRUE;
					gfRenderSummary = TRUE;
				}
			}
		}
		else if( gfItemDetailsMode && gusMouseXPos >= 350 && gusMouseYPos >= 15 && gusMouseXPos <= 404 && gusMouseYPos <= 27 )
		{
			gfItemDetailsMode = FALSE;
			gfRenderSummary = TRUE;
		}
	}
}

void UpdateSectorSummary(const ST::string& gszFilename, BOOLEAN fUpdate)
{
	ST::string str;
	INT16 x, y;

	gfRenderSummary = TRUE;
	//Extract the sector
	if( gszFilename[2] < '0' || gszFilename[2] > '9' )
		x = gszFilename[1] - '0';
	else
		x = (gszFilename[1] - '0') * 10 + gszFilename[2] - '0';
	if( gszFilename[0] >= 'a' )
		y = gszFilename[0] - 'a' + 1;
	else
		y = gszFilename[0] - 'A' + 1;

	//Validate that the values extracted are in fact a sector
	if( x < 1 || x > 16 || y < 1 || y > 16 )
		return;
	gsSector.x = gsSelSector.x = x;
	gsSector.y = gsSelSector.y = y;

	//The idea here is to get a pointer to the filename's period, then
	//focus on the character previous to it.  If it is a 1, 2, or 3, then
	//the filename was in a basement level.  Otherwise, it is a ground level.
	if (gszFilename.ends_with("_a.dat"))
	{
		auto i = gszFilename.find("_b");
		if (i >= 0 && gszFilename[i + 2] >= '1' && gszFilename[i + 2] <= '3' && gszFilename[i + 5] == '.')
		{ //it's a alternate basement map
			switch (gszFilename[i + 2])
			{
				case '1':
					gsSectorLayer = ALTERNATE_B1_MASK;
					giCurrLevel = 5;
					break;
				case '2':
					gsSectorLayer = ALTERNATE_B2_MASK;
					giCurrLevel = 6;
					break;
				case '3':
					gsSectorLayer = ALTERNATE_B3_MASK;
					giCurrLevel = 7;
					break;
			}
		}
		else
		{
			gsSectorLayer = ALTERNATE_GROUND_MASK;
			giCurrLevel = 4;
		}
	}
	else
	{
		auto i = gszFilename.find("_b");
		if (i >= 0 && gszFilename[i + 2] >= '1' && gszFilename[i + 2] <= '3' && gszFilename[i + 3] == '.')
		{ //it's a alternate basement map
			switch (gszFilename[i + 2])
			{
				case '1':
					gsSectorLayer = BASEMENT1_LEVEL_MASK;
					giCurrLevel = 1;
					break;
				case '2':
					gsSectorLayer = BASEMENT2_LEVEL_MASK;
					giCurrLevel = 2;
					break;
				case '3':
					gsSectorLayer = BASEMENT3_LEVEL_MASK;
					giCurrLevel = 3;
					break;
			}
		}
		else
		{
			gsSectorLayer = GROUND_LEVEL_MASK;
			giCurrLevel = 0;
		}
	}

	giCurrentViewLevel = gsSectorLayer;
	if (!(gbSectorLevels[gsSector.x - 1][gsSector.y - 1] & gsSectorLayer))
	{
		//new sector map saved, so update the global file.
		gbSectorLevels[gsSector.x - 1][gsSector.y - 1] |= gsSectorLayer;
	}

	if( fUpdate )
	{
		SetFontAttributes(FONT10ARIAL, FONT_LTKHAKI);
		str = ST::format("Analyzing map:  {}...", gszFilename);

		if( gfSummaryWindowActive )
		{
			MPrint(MAP_LEFT, MAP_BOTTOM + 100, str);
			InvalidateRegion( MAP_LEFT, MAP_BOTTOM+100, MAP_LEFT+150,	MAP_BOTTOM+110 );
			CreateProgressBar(0, MAP_LEFT, MAP_BOTTOM + 110, 140, 10);
		}
		else
		{
			MPrint((SCREEN_WIDTH - StringPixLength(str, FONT10ARIAL)) / 2, 190, str);
			InvalidateRegion( 200, 190, 400, 200 );
			CreateProgressBar(0, 250, 200, 140, 10);
		}

		ST::string szCoord = ST::format("{}", gszFilename);
		if (gsSector.x > 9)
			szCoord = szCoord.left(3);
		else
			szCoord = szCoord.left(2);
		gusNumEntriesWithOutdatedOrNoSummaryInfo++;
		EvaluateWorld( szCoord.c_str(), (UINT8)giCurrLevel );

		RemoveProgressBar( 0 );
	}
	else
		gusNumEntriesWithOutdatedOrNoSummaryInfo++;
}


static void SummaryOkayCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		DestroySummaryWindow();
	}
}


static void SummaryToggleGridCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderGrid = btn->Clicked();
		gfRenderMap  = TRUE;
	}
}


static void SummaryToggleAlternateCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->Clicked())
		{
			giCurrentViewLevel <<= 4;
			gfAlternateMaps = TRUE;
		}
		else
		{
			giCurrentViewLevel >>= 4;
			gfAlternateMaps = FALSE;
		}
		gfRenderSummary = TRUE;
	}
}


static void SummarySciFiCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iSummaryButton[SUMMARY_SCIFI]->uiFlags |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
		iSummaryButton[SUMMARY_REAL ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iSummaryButton[SUMMARY_REAL ]->uiFlags |= BUTTON_DIRTY;
		iSummaryButton[SUMMARY_ENEMY]->uiFlags &= ~BUTTON_CLICKED_ON;
		iSummaryButton[SUMMARY_ENEMY]->uiFlags |= BUTTON_DIRTY;
		gubSummaryItemMode = ITEMMODE_SCIFI;
		gfRenderSummary = TRUE;
	}
}


static void SummaryRealCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iSummaryButton[SUMMARY_SCIFI]->uiFlags &= ~BUTTON_CLICKED_ON;
		iSummaryButton[SUMMARY_SCIFI]->uiFlags |= BUTTON_DIRTY;
		iSummaryButton[SUMMARY_REAL ]->uiFlags |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
		iSummaryButton[SUMMARY_ENEMY]->uiFlags &= ~BUTTON_CLICKED_ON;
		iSummaryButton[SUMMARY_ENEMY]->uiFlags |= BUTTON_DIRTY;
		gubSummaryItemMode = ITEMMODE_REAL;
		gfRenderSummary = TRUE;
	}
}


static void SummaryEnemyCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iSummaryButton[SUMMARY_SCIFI]->uiFlags &= ~BUTTON_CLICKED_ON;
		iSummaryButton[SUMMARY_SCIFI]->uiFlags |= BUTTON_DIRTY;
		iSummaryButton[SUMMARY_REAL ]->uiFlags &= ~BUTTON_CLICKED_ON;
		iSummaryButton[SUMMARY_REAL ]->uiFlags |= BUTTON_DIRTY;
		iSummaryButton[SUMMARY_ENEMY]->uiFlags |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
		gubSummaryItemMode = ITEMMODE_ENEMY;
		gfRenderSummary = TRUE;
	}
}


static void SummaryToggleProgressCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfRenderProgress = btn->Clicked();
		gfRenderMap      = TRUE;
	}
}


static void CreateGlobalSummary(void);
static void RegenerateSummaryInfoForAllOutdatedMaps(void);


BOOLEAN HandleSummaryInput( InputAtom *pEvent )
{
	if( !gfSummaryWindowActive )
		return FALSE;
	if( (!HandleTextInput( pEvent ) && pEvent->usEvent == KEY_DOWN) || pEvent->usEvent == KEY_REPEAT )
	{
		switch( pEvent->usParam )
		{
			case SDLK_ESCAPE:
				if( !gfWorldLoaded )
				{
					DestroySummaryWindow();
					pEvent->usParam = 'x';
					pEvent->usKeyState |= ALT_DOWN;
					gfOverheadMapDirty = TRUE;
					return FALSE;
				}
				break;
			case SDLK_RETURN:
				if( GetActiveFieldID() == 1 )
					SelectNextField();
				else if( gfWorldLoaded )
					DestroySummaryWindow();
				break;

			case SDLK_y:
				if( gusNumEntriesWithOutdatedOrNoSummaryInfo && !gfOutdatedDenied )
				{
					gfRenderSummary = TRUE;
					RegenerateSummaryInfoForAllOutdatedMaps();
				}
				if( !gfGlobalSummaryExists && !gfDeniedSummaryCreation )
				{
					gfGlobalSummaryExists = TRUE;
					CreateGlobalSummary();
					gfRenderSummary = TRUE;
				}
				break;

			case SDLK_n:
				if( gusNumEntriesWithOutdatedOrNoSummaryInfo && !gfOutdatedDenied )
				{
					gfOutdatedDenied = TRUE;
					gfRenderSummary = TRUE;
				}
				if( !gfGlobalSummaryExists && !gfDeniedSummaryCreation )
				{
					gfDeniedSummaryCreation = TRUE;
					gfRenderSummary = TRUE;
				}
				break;
			case 'x':
				if (pEvent->usKeyState & ALT_DOWN )
				{
					DestroySummaryWindow();
					return FALSE;
				}
				break;

			case SDLK_RIGHT:
				gfRenderSummary = TRUE;
				if (!gsSelSector.y)
					gsSelSector.y = 1;
				gsSelSector.x++;
				if (gsSelSector.x > 16)
					gsSelSector.x = 1;
				break;

			case SDLK_LEFT:
				gfRenderSummary = TRUE;
				if (!gsSelSector.y)
					gsSelSector.y = 1;
				gsSelSector.x--;
				if (gsSelSector.x < 1)
					gsSelSector.x = 16;
				break;

			case SDLK_UP:
				gfRenderSummary = TRUE;
				if (!gsSelSector.x)
					gsSelSector.x = 1;
				gsSelSector.y--;
				if (gsSelSector.y < 1)
					gsSelSector.y = 16;
				break;

			case SDLK_DOWN:
				gfRenderSummary = TRUE;
				if (!gsSelSector.x)
					gsSelSector.x = 1;
				gsSelSector.y++;
				if (gsSelSector.y > 16)
					gsSelSector.y = 1;
				break;
		}
	}
	else if( pEvent->usEvent == KEY_UP )
	{ //for releasing modes requiring persistant state keys
		switch( pEvent->usParam )
		{
			case SDLK_F5: ReleaseSummaryWindow(); break;
		}
	}
	return TRUE;
}


/* This function can be very time consuming as it loads every map file with a
 * valid coordinate name, analyses it, and builds a new global summary file. */
static void CreateGlobalSummary(void)
{
	SLOGD("Generating GlobalSummary Information...");

	gfGlobalSummaryExists = FALSE;

	GCM->userPrivateFiles()->createDir(DEVINFO_DIR);

	// Generate a simple readme file.
	const char* readme = ""
		"This information is used in conjunction with the editor.\n"
		"This directory or its contents shouldn't be included with final release.\n";
	AutoSGPFile file{GCM->userPrivateFiles()->openForWriting(DEVINFO_DIR "/readme.txt", true)};

	file->write(readme, strlen(readme));

	LoadGlobalSummary();
	RegenerateSummaryInfoForAllOutdatedMaps();
	gfRenderSummary = TRUE;

	SLOGD("GlobalSummary Information generated successfully.");
}


static void MapMoveCallback(MOUSE_REGION* reg, INT32 reason)
{
	static SGPSector gsPrev;
	//calc current sector highlighted.
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gsPrev.x = gsHiSector.x = 0;
		gsPrev.y = gsHiSector.y = 0;
		gfRenderMap = TRUE;
		return;
	}
	gsHiSector.x = MIN((reg->RelativeXPos / 13) + 1, 16);
	gsHiSector.y = MIN((reg->RelativeYPos / 13) + 1, 16);
	if (gsPrev != gsHiSector)
	{
		gsPrev = gsHiSector;
		gfRenderMap = TRUE;
	}
}


static void MapClickCallback(MOUSE_REGION* reg, INT32 reason)
{
	static SGPSector sLast(-1, -1);
	//calc current sector selected.
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( GetActiveFieldID() == 1 )
		{
			gsSelSector.x = 0;
			SelectNextField();
		}
		gsSelSector.x = MIN( (reg->RelativeXPos / 13) + 1, 16 );
		gsSelSector.y = MIN( (reg->RelativeYPos / 13) + 1, 16 );
		if (gsSelSector != sLast)
		{ //clicked in a new sector
			gfOverrideDirty = TRUE;
			sLast = gsSelSector;
			auto sectorSummary = gpSectorSummary[gsSelSector.x - 1][gsSelSector.y - 1];
			switch( giCurrentViewLevel )
			{
				case ALL_LEVELS_MASK:
					if (sectorSummary[0])
						gpCurrentSectorSummary = sectorSummary[0];
					else if (sectorSummary[1])
						gpCurrentSectorSummary = sectorSummary[1];
					else if (sectorSummary[2])
						gpCurrentSectorSummary = sectorSummary[2];
					else if (sectorSummary[3])
						gpCurrentSectorSummary = sectorSummary[3];
					else
						gpCurrentSectorSummary = NULL;
					break;
				case GROUND_LEVEL_MASK: //already pointing to the correct level
					gpCurrentSectorSummary = sectorSummary[0];
					break;
				case BASEMENT1_LEVEL_MASK:
					gpCurrentSectorSummary = sectorSummary[1];
					break;
				case BASEMENT2_LEVEL_MASK:
					gpCurrentSectorSummary = sectorSummary[2];
					break;
				case BASEMENT3_LEVEL_MASK:
					gpCurrentSectorSummary = sectorSummary[3];
					break;
				case ALTERNATE_LEVELS_MASK:
					if (sectorSummary[4])
						gpCurrentSectorSummary = sectorSummary[4];
					else if(sectorSummary[5])
						gpCurrentSectorSummary = sectorSummary[5];
					else if(sectorSummary[6])
						gpCurrentSectorSummary = sectorSummary[6];
					else if(sectorSummary[7])
						gpCurrentSectorSummary = sectorSummary[7];
					else
						gpCurrentSectorSummary = NULL;
					break;
				case ALTERNATE_GROUND_MASK: //already pointing to the correct level
					gpCurrentSectorSummary = sectorSummary[4];
					break;
				case ALTERNATE_B1_MASK:
					gpCurrentSectorSummary = sectorSummary[5];
					break;
				case ALTERNATE_B2_MASK:
					gpCurrentSectorSummary = sectorSummary[6];
					break;
				case ALTERNATE_B3_MASK:
					gpCurrentSectorSummary = sectorSummary[7];
					break;
			}
			gpWorldItemsSummaryArray.clear();
			if( gfItemDetailsMode )
			{
				if( gpCurrentSectorSummary )
					/*&& gpCurrentSectorSummary->usNumItems ||
						gpPEnemyItemsSummaryArray && gusPEnemyItemsSummaryArraySize ||
						gpNEnemyItemsSummaryArray && gusNEnemyItemsSummaryArraySize )*/
				{
					gfSetupItemDetailsMode = TRUE;
				}
			}
		}
		gfRenderSummary = TRUE;
	}
}


static void SummaryToggleLevelCallback(GUI_BUTTON* btn, INT32 reason)
{
	INT8 i;
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( GetActiveFieldID() == 1 )
			SelectNextField();
		gfRenderSummary = TRUE;
		for( i = SUMMARY_ALL; i <= SUMMARY_B3; i++ )
		{
			if (btn == iSummaryButton[i])
			{
				switch( i )
				{
					case SUMMARY_ALL:
						giCurrentViewLevel = ALL_LEVELS_MASK;
						break;
					case SUMMARY_G:
						giCurrentViewLevel = GROUND_LEVEL_MASK;
						break;
					case SUMMARY_B1:
						giCurrentViewLevel = BASEMENT1_LEVEL_MASK;
						break;
					case SUMMARY_B2:
						giCurrentViewLevel = BASEMENT2_LEVEL_MASK;
						break;
					case SUMMARY_B3:
						giCurrentViewLevel = BASEMENT3_LEVEL_MASK;
						break;
				}
				if( gfAlternateMaps )
					giCurrentViewLevel <<= 4;
			}
			else
			{
				iSummaryButton[i]->uiFlags &= ~BUTTON_CLICKED_ON;
			}
		}
	}
}


static void SummaryLoadMapCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		ST::string str;
		gfRenderSummary = TRUE;

		SetFontAttributes(FONT10ARIAL, FONT_LTKHAKI);

		CreateProgressBar(0, MAP_LEFT + 5, MAP_BOTTOM + 110, 151, 10);

		DefineProgressBarPanel( 0, 65, 79, 94, MAP_LEFT, 318, 578, 356 );
		str = ST::format("Loading map:  {}", gszDisplayName);
		SetProgressBarTitle( 0, str, BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
		SetProgressBarMsgAttributes( 0, SMALLCOMPFONT, FONT_BLACK, FONT_BLACK );

		if(	ExternalLoadMap( gszDisplayName ) )
		{
			EnableButton( iSummaryButton[ SUMMARY_OKAY ] );
			gsSector = gsSelSector;
			gfOverrideDirty = TRUE;
		}
		RemoveProgressBar( 0 );
		auto i = gszDisplayName.find("_b");
		if (i < 0 || gszDisplayName[i + 3] != '.')
		{
			gsSectorLayer = GROUND_LEVEL_MASK;
			giCurrLevel = 0;
		}
		else
		{
			switch (gszDisplayName[i + 2] - '0')
			{
				case 1:		gsSectorLayer = BASEMENT1_LEVEL_MASK;	break;
				case 2:		gsSectorLayer = BASEMENT2_LEVEL_MASK;	break;
				case 3:		gsSectorLayer = BASEMENT3_LEVEL_MASK;	break;
				default:	gsSectorLayer = GROUND_LEVEL_MASK;		break;
			}
		}
	}
}


static void SummarySaveMapCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfSummaryWindowSaveRequested = TRUE;
		gfRenderSummary = TRUE;
	}
}


static void SummaryOverrideCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfOverride ^= TRUE;
		gfRenderSummary = TRUE;
		EnableButton(iSummaryButton[SUMMARY_SAVE], gfOverride);
	}
}


static void CalculateOverrideStatus(void)
{
	ST::string filename;
	gfOverrideDirty = FALSE;
	gfOverride = FALSE;
	if( gfTempFile )
	{
		// if empty, use "test.dat"
		if (gszTempFilename.empty())
		{
			filename = GCM->getMapPath(gszTempFilename);
		}
		else
		{
			filename = GCM->getMapPath("test.dat");
		}

		filename = FileMan::replaceExtension(filename, "dat");
	}
	else
	{
		filename = GCM->getMapPath(gszFilename);
	}

	gszDisplayName = ST::format("{}", FileMan::getFileName(filename));

	try {
		bool readonly = FileMan::isReadOnly(filename);

		if( gfWorldLoaded )
		{
			gubOverrideStatus = (readonly ? READONLY : OVERWRITE);
			ShowButton( iSummaryButton[ SUMMARY_OVERRIDE ] );
			iSummaryButton[SUMMARY_OVERRIDE]->uiFlags &= ~BUTTON_CLICKED_ON;
			DisableButton( iSummaryButton[ SUMMARY_SAVE ] );
		}
		if( gfTempFile )
			EnableButton( iSummaryButton[ SUMMARY_LOAD ] );
	} catch (const std::runtime_error& ex) {
		gubOverrideStatus = INACTIVE;
		HideButton( iSummaryButton[ SUMMARY_OVERRIDE ] );
		if( gfWorldLoaded )
			EnableButton( iSummaryButton[ SUMMARY_SAVE ] );
	}
}


static BOOLEAN LoadSummary(const INT32 x, const INT32 y, const UINT8 level, const char* const suffix)
{
	char summary_filename[40];
	sprintf(summary_filename, DEVINFO_DIR "/%c%d%s.sum", 'A' + y, x + 1, suffix);

	FLOAT dMajorMapVersion;
	{
		char filename[40];
		sprintf(filename, "%c%d%s.dat", 'A' + y, x + 1, suffix);

		AutoSGPFile f_map;
		try
		{
			f_map = GCM->openMapForReading(filename);
		}
		catch (...)
		{
			return FALSE;
		}

		f_map->read(&dMajorMapVersion, sizeof(FLOAT));
	}

	try {
		AutoSGPFile file{GCM->userPrivateFiles()->openForReading(summary_filename)};

		/* Even if the info is outdated (but existing), allocate the structure, but
		 * indicate that the info is bad. */
		SUMMARYFILE* const sum = new SUMMARYFILE{};
		try {
			file->read(reinterpret_cast<uint8_t*>(sum), sizeof(SUMMARYFILE));
		} catch (const std::runtime_error& err) {
			// failed, initialize and force update
			*sum = SUMMARYFILE{};
		}

		if (sum->ubSummaryVersion < MINIMUMVERSION ||
				dMajorMapVersion      < getMajorMapVersion())
		{
			++gusNumberOfMapsToBeForceUpdated;
			gfMustForceUpdateAllMaps = TRUE;
		}
		sum->dMajorMapVersion = dMajorMapVersion;
		UpdateSummaryInfo(sum);

		SUMMARYFILE** const anchor = &gpSectorSummary[x][y][level];
		if (*anchor) delete *anchor;
		*anchor = sum;

		if (sum->ubSummaryVersion < GLOBAL_SUMMARY_VERSION)
			++gusNumEntriesWithOutdatedOrNoSummaryInfo;
	} catch (const std::runtime_error& ex) {
		++gusNumEntriesWithOutdatedOrNoSummaryInfo;
	}

	return TRUE;
}


static void LoadGlobalSummary(void)
{
	SLOGD("Executing LoadGlobalSummary()...");

	gfMustForceUpdateAllMaps        = FALSE;
	gusNumberOfMapsToBeForceUpdated = 0;

	gfGlobalSummaryExists = GCM->userPrivateFiles()->isDir(DEVINFO_DIR);
	if (!gfGlobalSummaryExists)
	{
		SLOGI("LoadGlobalSummary() aborted -- doesn't exist on this local computer.");
		return;
	}

	/* Analyse all sectors to see if matching maps exist.  For any maps found, the
	 * information will be stored in the gbSectorLevels array.  Also, it attempts
	 * to load summaries for those maps.  If the summary information isn't found,
	 * then the occurrences are recorded and reported to the user when finished to
	 * give the option to generate them. */
	for (INT32 y = 0; y < 16; ++y)
	{
		for (INT32 x = 0; x < 16; ++x)
		{
			BOOLEAN sector_levels = 0;
			if (LoadSummary(x, y, 0, ""))       sector_levels |= GROUND_LEVEL_MASK;     // main ground level
			if (LoadSummary(x, y, 1, "_b1"))    sector_levels |= BASEMENT1_LEVEL_MASK;  // main B1 level
			if (LoadSummary(x, y, 2, "_b2"))    sector_levels |= BASEMENT2_LEVEL_MASK;  // main B2 level
			if (LoadSummary(x, y, 3, "_b3"))    sector_levels |= BASEMENT3_LEVEL_MASK;  // main B3 level
			if (LoadSummary(x, y, 4, "_a"))     sector_levels |= ALTERNATE_GROUND_MASK; // alternate ground level
			if (LoadSummary(x, y, 5, "_b1_a"))  sector_levels |= ALTERNATE_B1_MASK;     // alternate B1 level
			if (LoadSummary(x, y, 6, "_b2_a"))  sector_levels |= ALTERNATE_B2_MASK;     // alternate B2 level
			if (LoadSummary(x, y, 7, "_b3_a"))  sector_levels |= ALTERNATE_B3_MASK;     // alternate B2 level
			gbSectorLevels[x][y] = sector_levels;
		}
		SLOGD("Sector Row %c complete...", y + 'A');
	}

	if (gfMustForceUpdateAllMaps)
	{
		SLOGW("A MAJOR MAP UPDATE EVENT HAS BEEN DETECTED FOR %d MAPS!", gusNumberOfMapsToBeForceUpdated);
	}

	SLOGD("LoadGlobalSummary() finished...");
}


static void UpdateMasterProgress(void);


void WriteSectorSummaryUpdate(const ST::string &filename, const UINT8 ubLevel, SUMMARYFILE* const sf)
{
	bool ext = filename.ends_with(".dat", ST::case_insensitive);
	AssertMsg(ext, "Illegal sector summary filename.");

	ST::string summary_filename = ST::format("{}/{}.sum", DEVINFO_DIR, FileMan::getFileNameWithoutExt(filename));

	AutoSGPFile file{GCM->userPrivateFiles()->openForWriting(summary_filename, true)};

	file->write(sf, sizeof(*sf));

	--gusNumEntriesWithOutdatedOrNoSummaryInfo;
	UpdateMasterProgress();

	// Extract the sector information out of the filename.
	INT8 y;
	if (filename[0] >= 'a')
		y = filename[0] - 'a';
	else
		y = filename[0] - 'A';

	INT8 x;
	if (filename[2] < '0' || filename[2] > '9')
		x = filename[1] - '0' - 1;
	else
		x = (filename[1] - '0') * 10 + filename[2] - '0' - 1;

	gpSectorSummary[x][y][ubLevel] = sf;
}


double MasterStart, MasterEnd;


static void UpdateMasterProgress(void)
{
	if( gfUpdatingNow && gusTotal )
	{
		MasterStart = (gusCurrent / (double)gusTotal) * 100.0;
		gusCurrent++;
		MasterEnd = (gusCurrent / (double)gusTotal) * 100.0;
		if( gfMajorUpdate )
		{
			SetRelativeStartAndEndPercentage( 2, (UINT16)MasterStart, (UINT16)MasterEnd, ST::null );
			RenderProgressBar( 2, 0 );
		}
		else
			SetRelativeStartAndEndPercentage( 1, (UINT16)MasterStart, (UINT16)MasterEnd, ST::null );
	}
}


static void ReportError(const char* pSector, UINT8 ubLevel)
{
	static INT32 yp = 180;
	ST::string str;

	//Make sure the file exists... if not, then return false
	str = ST::format("{}", pSector);
	if( ubLevel % 4  )
	{
		str += ST::format("_b{}.dat", ubLevel % 4);
	}
	MPrint(10, yp, ST::format("Skipping update for {}.  Probably due to tileset conflicts...", str));
	InvalidateScreen();
	yp++;
}


static void RegenerateSummaryInfoForAllOutdatedMaps(void)
{
	INT32 x, y;
	SUMMARYFILE *pSF;
	//CreateProgressBar(0, 20, 120, 280, 12); //slave (individual)
	//CreateProgressBar(1, 20, 100, 280, 12); //master (total)
	//DefineProgressBarPanel( 0, 65, 79, 94, 10, 80, 310, 152 );
	CreateProgressBar(0, 20, 100, 280, 12); //master (total)
	DefineProgressBarPanel( 0, 65, 79, 94, 10, 80, 310, 132 );
	SetProgressBarTitle( 0, "Generating map information", BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
	SetProgressBarMsgAttributes( 0, SMALLCOMPFONT, FONT_BLACK, FONT_BLACK );
	gfUpdatingNow = TRUE;

	gusCurrent = 0;
	gusTotal = gusNumEntriesWithOutdatedOrNoSummaryInfo;
	UpdateMasterProgress();

	for( y = 0; y < 16; y++ ) for( x = 0; x < 16; x++ )
	{
		char str[40];
		sprintf( str, "%c%d", y + 'A', x + 1 );
		if( gbSectorLevels[x][y] & GROUND_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][0];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 0 ) )
					ReportError( str, 0 );
		}
		if( gbSectorLevels[x][y] & BASEMENT1_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][1];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 1 ) )
					ReportError( str, 1 );
		}
		if( gbSectorLevels[x][y] & BASEMENT2_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][2];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 2 ) )
					ReportError( str, 2 );
		}
		if( gbSectorLevels[x][y] & BASEMENT3_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][3];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 3 ) )
					ReportError( str, 3 );
		}
		if( gbSectorLevels[x][y] & ALTERNATE_GROUND_MASK )
		{
			pSF = gpSectorSummary[x][y][4];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 4 ) )
					ReportError( str, 4 );
		}
		if( gbSectorLevels[x][y] & ALTERNATE_B1_MASK )
		{
			pSF = gpSectorSummary[x][y][5];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 5 ) )
					ReportError( str, 5 );
		}
		if( gbSectorLevels[x][y] & ALTERNATE_B2_MASK )
		{
			pSF = gpSectorSummary[x][y][6];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 6 ) )
					ReportError( str, 6 );
		}
		if( gbSectorLevels[x][y] & ALTERNATE_B3_MASK )
		{
			pSF = gpSectorSummary[x][y][7];
			if( !pSF || pSF->ubSummaryVersion != GLOBAL_SUMMARY_VERSION )
				if( !EvaluateWorld( str, 7 ) )
					ReportError( str, 7 );
		}
	}
	RemoveProgressBar( 0 );
	RemoveProgressBar( 1 );
	gfUpdatingNow = FALSE;
}


static void SummaryUpdateCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		CreateProgressBar(0, 20, 100, 280, 12); //slave (individual)
		DefineProgressBarPanel( 0, 65, 79, 94, 10, 80, 310, 132 );
		SetProgressBarTitle( 0, "Generating map summary", BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
		SetProgressBarMsgAttributes( 0, SMALLCOMPFONT, FONT_BLACK, FONT_BLACK );

		if( gpCurrentSectorSummary )
		{
			delete gpCurrentSectorSummary;
			gpCurrentSectorSummary = NULL;
		}

		char str[40];
		sprintf(str, "%c%d", gsSelSector.y + 'A' - 1, gsSelSector.x);
		EvaluateWorld( str, (UINT8)giCurrLevel );

		gpSectorSummary[gsSelSector.x][gsSelSector.y][ giCurrLevel ] = gpCurrentSectorSummary;

		gfRenderSummary = TRUE;

		RemoveProgressBar( 0 );
	}
}


static void ExtractTempFilename(void)
{
	ST::string str = GetStringFromField(1);
	if (gszTempFilename != str)
	{
		gszTempFilename = str;
		gfRenderSummary = TRUE;
		gfOverrideDirty = TRUE;
	}
	if (str.empty())
		gszDisplayName = "test.dat";
}

void ApologizeOverrideAndForceUpdateEverything()
{
	INT32 x, y;
	ST::string str;
	SUMMARYFILE *pSF;
	//Create one huge assed button
	gfMajorUpdate = TRUE;
	iSummaryButton[SUMMARY_BACKGROUND] = CreateLabel(ST::null, 0, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH - 1);
	//Draw it
	iSummaryButton[SUMMARY_BACKGROUND]->Draw();
	InvalidateScreen();
	SetFontAttributes(HUGEFONT, FONT_RED);
	const ST::string UpdateInfo = "MAJOR VERSION UPDATE";
	MPrint((SCREEN_WIDTH - StringPixLength(UpdateInfo, HUGEFONT)) / 2, 105, UpdateInfo);
	SetFont( FONT10ARIAL );
	SetFontForeground( FONT_YELLOW );
	str = ST::format("There are {} maps requiring a major version update.", gusNumberOfMapsToBeForceUpdated);
	MPrint((SCREEN_WIDTH - StringPixLength(str, FONT10ARIAL)) / 2, 130, str);

	CreateProgressBar(2, 120, 170, 400, 32);
	DefineProgressBarPanel( 2, 65, 79, 94, 100, 150, 540, 222 );
	SetProgressBarTitle( 2, "Updating all outdated maps", BLOCKFONT2, FONT_RED, 0 );
	SetProgressBarMsgAttributes( 2, SMALLCOMPFONT, FONT_BLACK, FONT_BLACK );

	gusCurrent = 0;
	gusTotal = gusNumberOfMapsToBeForceUpdated;
	gfUpdatingNow = TRUE;
	UpdateMasterProgress();

	for( y = 0; y < 16; y++ ) for( x = 0; x < 16; x++ )
	{
		char name[50];
		sprintf( name, "%c%d", y + 'A', x + 1 );
		if( gbSectorLevels[x][y] & GROUND_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][0];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 0 ) )
					return;
			}
		}
		if( gbSectorLevels[x][y] & BASEMENT1_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][1];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 1 ) )
					return;
			}
		}
		if( gbSectorLevels[x][y] & BASEMENT2_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][2];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 2 ) )
					return;
			}
		}
		if( gbSectorLevels[x][y] & BASEMENT3_LEVEL_MASK )
		{
			pSF = gpSectorSummary[x][y][3];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 3 ) )
					return;
			}
		}
		if( gbSectorLevels[x][y] & ALTERNATE_GROUND_MASK )
		{
			pSF = gpSectorSummary[x][y][4];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 4 ) )
					return;
			}
		}
		if( gbSectorLevels[x][y] & ALTERNATE_B1_MASK )
		{
			pSF = gpSectorSummary[x][y][5];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 5 ) )
					return;
			}
		}
		if( gbSectorLevels[x][y] & ALTERNATE_B2_MASK )
		{
			pSF = gpSectorSummary[x][y][6];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 6 ) )
					return;
			}
		}
		if( gbSectorLevels[x][y] & ALTERNATE_B3_MASK )
		{
			pSF = gpSectorSummary[x][y][7];
			if( !pSF || pSF->ubSummaryVersion < MINIMUMVERSION || pSF->dMajorMapVersion < getMajorMapVersion() )
			{
				gpCurrentSectorSummary = pSF;
				if( !EvaluateWorld( name, 7 ) )
					return;
			}
		}
	}

	EvaluateWorld( "p3_m.dat", 0 );

	RemoveProgressBar( 2 );
	gfUpdatingNow = FALSE;
	InvalidateScreen();

	RemoveButton( iSummaryButton[ SUMMARY_BACKGROUND ] );
	gfMajorUpdate = FALSE;
	gfMustForceUpdateAllMaps = FALSE;
	gusNumberOfMapsToBeForceUpdated = 0;
}


static void SetupItemDetailsMode(BOOLEAN fAllowRecursion)
{
	UINT32 uiNumItems;
	SOLDIERCREATE_STRUCT priority;
	INT32 i, j;
	UINT16 usNumItems;
	OBJECTTYPE *pItem;
	UINT16 usPEnemyIndex, usNEnemyIndex;

	//Clear memory for all the item summaries loaded
	gpWorldItemsSummaryArray.clear();
	if( gpPEnemyItemsSummaryArray )
	{
		delete[] gpPEnemyItemsSummaryArray;
		gpPEnemyItemsSummaryArray = NULL;
		gusPEnemyItemsSummaryArraySize = 0;
	}
	if( gpNEnemyItemsSummaryArray )
	{
		delete[] gpNEnemyItemsSummaryArray;
		gpNEnemyItemsSummaryArray = NULL;
		gusNEnemyItemsSummaryArraySize = 0;
	}

	if( !gpCurrentSectorSummary->uiNumItemsPosition )
	{	//Don't have one, so generate them
		if( gpCurrentSectorSummary->ubSummaryVersion == GLOBAL_SUMMARY_VERSION )
			gusNumEntriesWithOutdatedOrNoSummaryInfo++;
		SummaryUpdateCallback(iSummaryButton[SUMMARY_UPDATE], MSYS_CALLBACK_REASON_LBUTTON_UP);
		gpCurrentSectorSummary = gpSectorSummary[gsSelSector.x - 1][gsSelSector.y - 1][ giCurrLevel ];
	}
	//Open the original map for the sector
	AutoSGPFile hfile(GCM->openMapForReading(gszFilename));
	//Now fileseek directly to the file position where the number of world items are stored
	hfile->seek(gpCurrentSectorSummary->uiNumItemsPosition, FILE_SEEK_FROM_START);
	//Now load the number of world items from the map.
	hfile->read(&uiNumItems, 4);
	//Now compare this number with the number the summary thinks we should have.  If they are different,
	//the the summary doesn't match the map.  What we will do is force regenerate the map so that they do
	//match
	if( uiNumItems != gpCurrentSectorSummary->usNumItems && fAllowRecursion )
	{
		gpCurrentSectorSummary->uiNumItemsPosition = 0;
		SetupItemDetailsMode( FALSE );
		return;
	}
	//Passed the gauntlet, so now allocate memory for it, and load all the world items into this array.
	ShowButton( iSummaryButton[ SUMMARY_SCIFI ] );
	ShowButton( iSummaryButton[ SUMMARY_REAL ] );
	ShowButton( iSummaryButton[ SUMMARY_ENEMY ] );
	Assert(uiNumItems == gpCurrentSectorSummary->usNumItems);
	if (gpCurrentSectorSummary->usNumItems != 0)
	{
		gpWorldItemsSummaryArray.assign(uiNumItems, WORLDITEM{});
		hfile->read(gpWorldItemsSummaryArray.data(), sizeof(WORLDITEM) * uiNumItems);
	}

	//NOW, do the enemy's items!
	//We need to do two passes.  The first pass simply processes all the enemies and counts all the droppable items
	//keeping track of two different values.  The first value is the number of droppable items that come off of
	//enemy detailed placements, the other counter keeps track of the number of droppable items that come off of
	//normal enemy placements.  After doing this, the memory is allocated for the tables that will store all the item
	//summary information, then the second pass will repeat the process, except it will record the actual items.

	//PASS #1
	hfile->seek(gpCurrentSectorSummary->uiEnemyPlacementPosition, FILE_SEEK_FROM_START);
	for( i = 0; i < gpCurrentSectorSummary->MapInfo.ubNumIndividuals ; i++ )
	{
		BASIC_SOLDIERCREATE_STRUCT basic;
		ExtractBasicSoldierCreateStructFromFile(hfile, basic);
		if( basic.fDetailedPlacement )
		{ //skip static priority placement

			// Always use windows format because here we are loading a map
			// file, not a user save
			ExtractSoldierCreateFromFile(hfile, &priority, false);
		}
		else
		{ //non detailed placements don't have items, so skip
			continue;
		}
		if( basic.bTeam == ENEMY_TEAM )
		{
			//Count the items that this enemy placement drops
			usNumItems = 0;
			for( j = 0; j < 9; j++ )
			{
				pItem = &priority.Inv[ gbMercSlotTypes[ j ] ];
				if( pItem->usItem != NOTHING && !( pItem->fFlags & OBJECT_UNDROPPABLE ) )
				{
					usNumItems++;
				}
			}
			if( basic.fPriorityExistance )
			{
				gusPEnemyItemsSummaryArraySize += usNumItems;
			}
			else
			{
				gusNEnemyItemsSummaryArraySize += usNumItems;
			}
		}
	}

	//Pass 1 completed, so now allocate enough space to hold all the items
	if( gusPEnemyItemsSummaryArraySize )
	{
		gpPEnemyItemsSummaryArray = new OBJECTTYPE[gusPEnemyItemsSummaryArraySize]{};
	}
	if( gusNEnemyItemsSummaryArraySize )
	{
		gpNEnemyItemsSummaryArray = new OBJECTTYPE[gusNEnemyItemsSummaryArraySize]{};
	}

	//PASS #2
	//During this pass, simply copy all the data instead of counting it, now that we have already done so.
	usPEnemyIndex = usNEnemyIndex = 0;
	hfile->seek(gpCurrentSectorSummary->uiEnemyPlacementPosition, FILE_SEEK_FROM_START);
	for( i = 0; i < gpCurrentSectorSummary->MapInfo.ubNumIndividuals ; i++ )
	{
		BASIC_SOLDIERCREATE_STRUCT basic;
		ExtractBasicSoldierCreateStructFromFile(hfile, basic);
		if( basic.fDetailedPlacement )
		{ //skip static priority placement

			// Always use windows format because here we are loading a map
			// file, not a user save
			ExtractSoldierCreateFromFile(hfile, &priority, false);
		}
		else
		{ //non detailed placements don't have items, so skip
			continue;
		}
		if( basic.bTeam == ENEMY_TEAM )
		{
			//Copy the items that this enemy placement drops
			usNumItems = 0;
			for( j = 0; j < 9; j++ )
			{
				pItem = &priority.Inv[ gbMercSlotTypes[ j ] ];
				if( pItem->usItem != NOTHING && !( pItem->fFlags & OBJECT_UNDROPPABLE ) )
				{
					if( basic.fPriorityExistance )
					{
						gpPEnemyItemsSummaryArray[usPEnemyIndex] = *pItem;
						usPEnemyIndex++;
					}
					else
					{
						gpNEnemyItemsSummaryArray[usNEnemyIndex] = *pItem;
						usNEnemyIndex++;
					}
				}
			}
		}
	}
}
