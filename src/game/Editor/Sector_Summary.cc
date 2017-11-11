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
#include "GameState.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "slog/slog.h"

#define DEVINFO_DIR "../DevInfo"


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
static const wchar_t* const gszVersionType[5] = { L"Pre-Alpha", L"Alpha", L"Demo", L"Beta", L"Release" };
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

static WORLDITEM*  gpWorldItemsSummaryArray       = 0;
static UINT16      gusWorldItemsSummaryArraySize  = 0;
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
static INT16 gsSectorX;
static INT16 gsSectorY;
//The layer of the sector that is currently loaded in memory.
static INT32 gsSectorLayer;
//The sector coordinates of the mouse position (yellow)
static INT16 gsHiSectorX;
static INT16 gsHiSectorY;
//The sector coordinates of the selected sector (red)
static INT16 gsSelSectorX;
static INT16 gsSelSectorY;

//Used to determine how long the F5 key has been held down for to determine whether or not the
//summary is going to be persistant or not.
static UINT32 giInitTimer;

static wchar_t gszFilename[40];
static wchar_t gszTempFilename[21];
static wchar_t gszDisplayName[21];

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

	gsSectorX    = gWorldSectorX;
	gsSectorY    = gWorldSectorY;
	gsSelSectorX = gWorldSectorX;
	gsSelSectorY = gWorldSectorY;
	gfSummaryWindowActive = TRUE;
	gfPersistantSummary = FALSE;
	giInitTimer = GetJA2Clock();
	gfDeniedSummaryCreation = FALSE;
	gfRenderSummary = TRUE;
	//Create all of the buttons here
	iSummaryButton[SUMMARY_BACKGROUND] = CreateLabel(NULL, 0, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - EDITOR_TASKBAR_HEIGHT, MSYS_PRIORITY_HIGH - 1);

	iSummaryButton[SUMMARY_OKAY] = CreateTextButton(L"Okay", FONT12POINT1, FONT_BLACK, FONT_BLACK, 585, 325, 50, 30, MSYS_PRIORITY_HIGH, SummaryOkayCallback);

	iSummaryButton[SUMMARY_GRIDCHECKBOX] = MakeCheckBox(MAP_LEFT, MAP_BOTTOM + 5, SummaryToggleGridCallback, true);
	gfRenderGrid = TRUE;

	iSummaryButton[SUMMARY_PROGRESSCHECKBOX] = MakeCheckBox(MAP_LEFT + 50, MAP_BOTTOM + 5, SummaryToggleProgressCallback, true);
	gfRenderProgress = TRUE;

	iSummaryButton[SUMMARY_ALL] = CreateTextButton(L"A",  SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 110, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == ALL_LEVELS_MASK || giCurrentViewLevel == ALTERNATE_LEVELS_MASK )
		iSummaryButton[SUMMARY_ALL]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_G]   = CreateTextButton(L"G",  SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 128, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == GROUND_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_GROUND_MASK )
		iSummaryButton[SUMMARY_G]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_B1]  = CreateTextButton(L"B1", SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 146, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == BASEMENT1_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_B1_MASK )
		iSummaryButton[SUMMARY_B1]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_B2]  = CreateTextButton(L"B2", SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 164, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == BASEMENT2_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_B2_MASK )
		iSummaryButton[SUMMARY_B2]->uiFlags |= BUTTON_CLICKED_ON;
	iSummaryButton[SUMMARY_B3]  = CreateTextButton(L"B3", SMALLCOMPFONT, FONT_BLACK, FONT_BLACK, MAP_LEFT + 182, MAP_BOTTOM + 5, 16, 16, MSYS_PRIORITY_HIGH, SummaryToggleLevelCallback);
	if( giCurrentViewLevel == BASEMENT3_LEVEL_MASK || giCurrentViewLevel == ALTERNATE_B3_MASK )
		iSummaryButton[SUMMARY_B3]->uiFlags |= BUTTON_CLICKED_ON;

	iSummaryButton[SUMMARY_ALTERNATE] = MakeCheckBox(MAP_LEFT, MAP_BOTTOM + 25, SummaryToggleAlternateCallback, gfAlternateMaps);

	iSummaryButton[SUMMARY_LOAD] = CreateTextButton(L"LOAD", FONT12POINT1, FONT_BLACK, FONT_BLACK, MAP_LEFT,      MAP_BOTTOM + 45, 50, 26, MSYS_PRIORITY_HIGH, SummaryLoadMapCallback);
	iSummaryButton[SUMMARY_SAVE] = CreateTextButton(L"SAVE", FONT12POINT1, FONT_BLACK, FONT_BLACK, MAP_LEFT + 55, MAP_BOTTOM + 45, 50, 26, MSYS_PRIORITY_HIGH, SummarySaveMapCallback);
	iSummaryButton[SUMMARY_OVERRIDE] = MakeCheckBox(MAP_LEFT + 110, MAP_BOTTOM + 59, SummaryOverrideCallback);


	iSummaryButton[SUMMARY_UPDATE] = CreateTextButton(L"Update", FONT12POINT1, FONT_BLACK, FONT_BLACK, 255, 15, 40, 16, MSYS_PRIORITY_HIGH, SummaryUpdateCallback);

	iSummaryButton[SUMMARY_REAL]  = MakeRadioButton(350, 47, SummaryRealCallback);
	iSummaryButton[SUMMARY_SCIFI] = MakeRadioButton(376, 47, SummarySciFiCallback);
	iSummaryButton[SUMMARY_ENEMY] = MakeRadioButton(350, 60, SummaryEnemyCallback);

	//iSummaryButton[SUMMARY_SCIFI]->SetFastHelpText(L"Display items that appear in SciFi mode.");
	//iSummaryButton[SUMMARY_REAL ]->SetFastHelpText(L"Display items that appear in Realistic mode.");
	switch( gubSummaryItemMode )
	{
		case ITEMMODE_SCIFI: iSummaryButton[SUMMARY_SCIFI]->uiFlags |= BUTTON_CLICKED_ON; break;
		case ITEMMODE_REAL:  iSummaryButton[SUMMARY_REAL]->uiFlags  |= BUTTON_CLICKED_ON; break;
		case ITEMMODE_ENEMY: iSummaryButton[SUMMARY_ENEMY]->uiFlags |= BUTTON_CLICKED_ON; break;
	}

	//Init the textinput field.
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	AddUserInputField( NULL );  //just so we can use short cut keys while not typing.
	AddTextInputField(MAP_LEFT + 112, MAP_BOTTOM + 75, 100, 18, MSYS_PRIORITY_HIGH, L"", 20, INPUTTYPE_DOSFILENAME);

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
		gsSelSectorX = 9;
		gsSelSectorY = 1;
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
	if( !gfWorldLoaded || uiCurrTimer - giInitTimer < 400 )
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

	FreeNull(gpWorldItemsSummaryArray);
	gusWorldItemsSummaryArraySize = 0;
	FreeNull(gpPEnemyItemsSummaryArray);
	gusPEnemyItemsSummaryArraySize = 0;
	FreeNull(gpNEnemyItemsSummaryArray);
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
	mprintf(10, 32, L"Tileset:  %ls", gTilesets[s->ubTilesetID].zName);
	if( m->ubMapVersion < 10 )
		SetFontForeground( FONT_RED );
	mprintf( 10, 42,    L"Version Info:  Summary:  1.%02d,  Map:  %d.%02d", s->ubSummaryVersion, (INT32)s->dMajorMapVersion, m->ubMapVersion );
	SetFontForeground( FONT_GRAY2 );
	mprintf( 10, 55,		L"Number of items:  %d", s->usNumItems );
	mprintf( 10, 65,		L"Number of lights:  %d", s->usNumLights );
	mprintf( 10, 75,		L"Number of entry points:  %d", ePoints );
	if( ePoints )
	{
		INT32 x;
		x = 140;
		MPrint( x, 75, L"(" );
		x += StringPixLength( L"(", FONT10ARIAL ) + 2;
		if (m->sNorthGridNo    != -1) { MPrint(x, 75, L"N"); x += StringPixLength(L"N", FONT10ARIAL) + 2; }
		if (m->sEastGridNo     != -1) { MPrint(x, 75, L"E"); x += StringPixLength(L"E", FONT10ARIAL) + 2; }
		if (m->sSouthGridNo    != -1) { MPrint(x, 75, L"S"); x += StringPixLength(L"S", FONT10ARIAL) + 2; }
		if (m->sWestGridNo     != -1) { MPrint(x, 75, L"W"); x += StringPixLength(L"W", FONT10ARIAL) + 2; }
		if (m->sCenterGridNo   != -1) { MPrint(x, 75, L"C"); x += StringPixLength(L"C", FONT10ARIAL) + 2; }
		if (m->sIsolatedGridNo != -1) { MPrint(x, 75, L"I"); x += StringPixLength(L"I", FONT10ARIAL) + 2; }
		MPrint(x, 75, L")");
	}
	mprintf( 10, 85,			L"Number of rooms:  %d", s->ubNumRooms );
	mprintf( 10, 95,			L"Total map population:  %d", m->ubNumIndividuals );
	mprintf( 20, 105,			L"Enemies:  %d", s->EnemyTeam.ubTotal );
	mprintf( 30, 115,			L"Admins:  %d", s->ubNumAdmins );
	if( s->ubNumAdmins )
		mprintf( 100, 115,		L"(%d detailed, %d profile -- %d have priority existance)", s->ubAdminDetailed, s->ubAdminProfile, s->ubAdminExistance );
	mprintf( 30, 125,			L"Troops:  %d", s->ubNumTroops );
	if( s->ubNumTroops )
		mprintf( 100, 125,		L"(%d detailed, %d profile -- %d have priority existance)", s->ubTroopDetailed, s->ubTroopProfile, s->ubTroopExistance );
	mprintf( 30, 135,			L"Elites:  %d", s->ubNumElites );
	if( s->ubNumElites )
		mprintf( 100, 135,		L"(%d detailed, %d profile -- %d have priority existance)", s->ubEliteDetailed, s->ubEliteProfile, s->ubEliteExistance );
	mprintf( 20, 145,			L"Civilians:  %d", s->CivTeam.ubTotal );
	if( s->CivTeam.ubTotal )
		mprintf( 100, 145,		L"(%d detailed, %d profile -- %d have priority existance)", s->CivTeam.ubDetailed, s->CivTeam.ubProfile, s->CivTeam.ubExistance );
	if( s->ubSummaryVersion >= 9 )
	{
		mprintf( 30, 155,		L"Humans:  %d", s->CivTeam.ubTotal - s->ubCivCows - s->ubCivBloodcats );
		mprintf( 30, 165,		L"Cows:  %d", s->ubCivCows );
		mprintf( 30, 175,		L"Bloodcats:  %d", s->ubCivBloodcats );
	}
	mprintf( 20, 185,			L"Creatures:  %d", s->CreatureTeam.ubTotal );
	if( s->ubSummaryVersion >= 9 )
	{
		mprintf( 30, 195,     L"Monsters:  %d", s->CreatureTeam.ubTotal - s->CreatureTeam.ubNumAnimals );
		mprintf( 30, 205,     L"Bloodcats:  %d", s->CreatureTeam.ubNumAnimals );
	}
	mprintf( 10, 215,		L"Number of locked and/or trapped doors:  %d", s->ubNumDoors );
	mprintf( 20, 225,			L"Locked:  %d", s->ubNumDoorsLocked );
	mprintf( 20, 235,			L"Trapped:  %d", s->ubNumDoorsTrapped );
	mprintf( 20, 245,			L"Locked & Trapped:  %d", s->ubNumDoorsLockedAndTrapped );
	if( s->ubSummaryVersion >= 8 )
		mprintf( 10, 255,			L"Civilians with schedules:  %d", s->ubCivSchedules );
	if( s->ubSummaryVersion >= 10 )
	{
		if( s->fTooManyExitGridDests )
		{
			SetFontForeground( FONT_RED );
			mprintf( 10, 265, L"Too many exit grid destinations (more than 4)...");
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
				mprintf( 10, 265, L"ExitGrids:  %d (%d with a long distance destination)", s->ubNumExitGridDests, ubNumInvalid );
			}
			else switch( s->ubNumExitGridDests )
			{
				case 0:
					MPrint(10, 265, L"ExitGrids:  none");
					break;
				case 1:
					mprintf( 10, 265, L"ExitGrids:  1 destination using %d exitgrids", s->usExitGridSize[0] );
					break;
				case 2:
					mprintf( 10, 265, L"ExitGrids:  2 -- 1) Qty: %d, 2) Qty: %d", s->usExitGridSize[0], s->usExitGridSize[1] );
					break;
				case 3:
					mprintf( 10, 265, L"ExitGrids:  3 -- 1) Qty: %d, 2) Qty: %d, 3) Qty: %d",
						s->usExitGridSize[0], s->usExitGridSize[1], s->usExitGridSize[2] );
					break;
				case 4:
					mprintf( 10, 265, L"ExitGrids:  3 -- 1) Qty: %d, 2) Qty: %d, 3) Qty: %d, 4) Qty: %d",
						s->usExitGridSize[0], s->usExitGridSize[1], s->usExitGridSize[2], s->usExitGridSize[3] );
					break;
			}
		}
	}
	iOverall = - ( 2 * s->EnemyTeam.ubBadA ) - s->EnemyTeam.ubPoorA + s->EnemyTeam.ubGoodA + ( 2 * s->EnemyTeam.ubGreatA );
	usLine = 275;
	mprintf( 10, usLine, L"Enemy Relative Attributes:  %d bad, %d poor, %d norm, %d good, %d great (%+d Overall)",
		s->EnemyTeam.ubBadA,
		s->EnemyTeam.ubPoorA,
		s->EnemyTeam.ubAvgA,
		s->EnemyTeam.ubGoodA,
		s->EnemyTeam.ubGreatA,
		iOverall );
	iOverall = - ( 2 * s->EnemyTeam.ubBadE ) - s->EnemyTeam.ubPoorE + s->EnemyTeam.ubGoodE + ( 2 * s->EnemyTeam.ubGreatE );
	usLine += 10;
	mprintf( 10, usLine, L"Enemy Relative Equipment:  %d bad, %d poor, %d norm, %d good, %d great (%+d Overall)",
		s->EnemyTeam.ubBadE,
		s->EnemyTeam.ubPoorE,
		s->EnemyTeam.ubAvgE,
		s->EnemyTeam.ubGoodE,
		s->EnemyTeam.ubGreatE,
		iOverall );
	usLine += 10;
	if( s->ubSummaryVersion >= 11 )
	{
		if( s->ubEnemiesReqWaypoints )
		{
			SetFontForeground( FONT_RED );
			mprintf( 10, usLine, L"%d placements have patrol orders without any waypoints defined.", s->ubEnemiesReqWaypoints );
			usLine += 10;
		}
	}
	if( s->ubSummaryVersion >= 13 )
	{
		if( s->ubEnemiesHaveWaypoints )
		{
			SetFontForeground( FONT_RED );
			mprintf( 10, usLine, L"%d placements have waypoints, but without any patrol orders.", s->ubEnemiesHaveWaypoints );
			usLine += 10;
		}
	}
	if( s->ubSummaryVersion >= 12 )
	{
		if( s->usWarningRoomNums )
		{
			SetFontForeground( FONT_RED );
			mprintf( 10, usLine, L"%d gridnos have questionable room numbers.  Please validate.", s->usWarningRoomNums );
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
	UINT32 uiTriggerQuantity[8], uiActionQuantity[8], uiTriggerExistChance[8], uiActionExistChance[8];
	UINT32 xp, yp;
	INT8 bFreqIndex;
	SetFontAttributes(FONT10ARIAL, FONT_GRAY2);
	MPrint(364, 49, L"R");
	MPrint(390, 49, L"S");
	MPrint(364, 62, L"Enemy");
	yp = 20;
	xp = 5;
	if (gubSummaryItemMode != ITEMMODE_ENEMY)
	{
		memset( uiTriggerQuantity, 0, 32 );
		memset( uiActionQuantity, 0, 32 );
		memset( uiTriggerExistChance, 0, 32 );
		memset( uiActionExistChance, 0, 32 );
		for( index = 1; index < MAXITEMS; index++ )
		{
			uiQuantity = 0;
			uiExistChance = 0;
			uiStatus = 0;
			for( i = 0; i < gusWorldItemsSummaryArraySize; i++ )
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
				mprintf( xp + 85, yp, L"%3.02f", dAvgExistChance );
				mprintf( xp + 110, yp, L"@ %3.02f%%", dAvgStatus );
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, L"TOO MANY ITEMS TO DISPLAY!");
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
				const wchar_t* Type; // HACK000E
				switch( i )
				{
					case 0: Type = L"Panic1";           break;
					case 1:	Type = L"Panic2";           break;
					case 2:	Type = L"Panic3";           break;
					case 3:	Type = L"Norm1";            break;
					case 4:	Type = L"Norm2";            break;
					case 5:	Type = L"Norm3";            break;
					case 6:	Type = L"Norm4";            break;
					case 7:	Type = L"Pressure Actions"; break;

					default: abort(); // HACK000E
				}
				if( i < 7 )
				{
					dAvgExistChance = (FLOAT)(uiTriggerExistChance[i] / 100.0);
					dAvgStatus = (FLOAT)(uiActionExistChance[i] / 100.0);
					mprintf(xp, yp, L"%ls:  %3.02f trigger(s), %3.02f action(s)", Type, dAvgExistChance, dAvgStatus);
				}
				else
				{
					dAvgExistChance = (FLOAT)(uiActionExistChance[i] / 100.0);
					mprintf(xp, yp, L"%ls:  %3.02f", Type, dAvgExistChance);
				}
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, L"TOO MANY ITEMS TO DISPLAY!");
						return;
					}
				}
			}
		}
	}
	else if( gubSummaryItemMode == ITEMMODE_ENEMY )
	{

		SetFontForeground( FONT_YELLOW );
		MPrint(xp, yp, L"PRIORITY ENEMY DROPPED ITEMS");
		yp += 10;

		//Do the priority existance guys first
		if( !gpPEnemyItemsSummaryArray )
		{
			SetFontForeground( FONT_DKYELLOW );
			MPrint(xp, yp, L"None");
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
				mprintf( xp + 85, yp, L"%3.02f", dAvgExistChance );
				mprintf( xp + 110, yp, L"@ %3.02f%%", dAvgStatus );
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, L"TOO MANY ITEMS TO DISPLAY!");
						return;
					}
				}
			}
		}

		yp += 5;

		SetFontForeground( FONT_YELLOW );
		MPrint(xp, yp, L"NORMAL ENEMY DROPPED ITEMS");
		yp += 10;
		if( yp >= 355 )
		{
			xp += 170;
			yp = 20;
			if( xp >= 300 )
			{
				SetFontForeground( FONT_RED );
				MPrint(350, 350, L"TOO MANY ITEMS TO DISPLAY!");
				return;
			}
		}

		//Do the priority existance guys first
		if( !gpNEnemyItemsSummaryArray )
		{
			SetFontForeground( FONT_DKYELLOW );
			MPrint(xp, yp, L"None");
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
				mprintf( xp + 85, yp, L"%3.02f", dAvgExistChance );
				mprintf( xp + 110, yp, L"@ %3.02f%%", dAvgStatus );
				yp += 10;
				if( yp >= 355 )
				{
					xp += 170;
					yp = 20;
					if( xp >= 300 )
					{
						SetFontForeground( FONT_RED );
						MPrint(350, 350, L"TOO MANY ITEMS TO DISPLAY!");
						return;
					}
				}
			}
		}



	}
	else
	{
		SetFontForeground( FONT_RED );
		MPrint(5, 50, L"ERROR:  Can't load the items for this map.  Reason unknown.");
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
		SetInputFieldStringWith16BitString( 1, L"" );
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
			mprintf(10, 5, L"CAMPAIGN EDITOR -- %ls Version 1.%02d",
				gszVersionType[ GLOBAL_SUMMARY_STATE ], GLOBAL_SUMMARY_VERSION );
		}

		//This section builds the proper header to be displayed for an existing global summary.
		if( !gfWorldLoaded )
		{
			SetFontForeground( FONT_RED );
			SetFontShadow( FONT_NEARBLACK );
			MPrint(270, 5, L"(NO MAP LOADED).");
		}
		SetFont( FONT10ARIAL );
		SetFontShadow( FONT_NEARBLACK );
		if( gfGlobalSummaryExists )
		{
			wchar_t str[100];
			BOOLEAN fSectorSummaryExists = FALSE;
			if( gusNumEntriesWithOutdatedOrNoSummaryInfo && !gfOutdatedDenied )
			{
				DisableButton( iSummaryButton[ SUMMARY_LOAD ] );
				SetFontForeground( FONT_YELLOW );
				mprintf( 10, 20, L"You currently have %d outdated maps.", gusNumEntriesWithOutdatedOrNoSummaryInfo);
				MPrint( 10, 30, L"The more maps that need to be updated, the longer it takes.  It'll take ");
				MPrint( 10, 40, L"approximately 4 minutes on a P200MMX to analyse 100 maps, so");
				MPrint( 10, 50, L"depending on your computer, it may vary.");
				SetFontForeground( FONT_LTRED );
				MPrint( 10, 65, L"Do you wish to regenerate info for ALL these maps at this time (y/n)?" );
			}
			else if( (!gsSelSectorX && !gsSectorX) || gfTempFile )
			{
				DisableButton( iSummaryButton[ SUMMARY_LOAD ] );
				SetFontForeground( FONT_LTRED );
				MPrint(10, 20, L"There is no sector currently selected.");
				if( gfTempFile )
				{
					SetFontForeground( FONT_YELLOW );
					MPrint(10, 30, L"Entering a temp file name that doesn't follow campaign editor conventions...");
					goto SPECIALCASE_LABEL;  //OUCH!!!
				}
				else if( !gfWorldLoaded )
				{
					SetFontForeground( FONT_YELLOW );
					MPrint(10, 30, L"You need to either load an existing map or create a new map before being");
					MPrint(10, 40, L"able to enter the editor, or you can quit (ESC or Alt+x).");
				}
			}
			else
			{
				//Build sector string
				if( gsSelSectorX )
					x = gsSelSectorX - 1, y = gsSelSectorY - 1;
				else
					x = gsSectorX - 1, y = gsSectorY - 1;
				swprintf(str, lengthof(str), L"%c%d", y + 'A', x + 1);
				swprintf(gszFilename, lengthof(gszFilename), str);
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
							wcscat( str, L", ground level" );
							gpCurrentSectorSummary = gpSectorSummary[x][y][0];
							break;
						case BASEMENT1_LEVEL_MASK:
							giCurrLevel = 1;
							wcscat( str, L", underground level 1" );
							gpCurrentSectorSummary = gpSectorSummary[x][y][1];
							break;
						case BASEMENT2_LEVEL_MASK:
							giCurrLevel = 2;
							wcscat( str, L", underground level 2" );
							gpCurrentSectorSummary = gpSectorSummary[x][y][2];
							break;
						case BASEMENT3_LEVEL_MASK:
							giCurrLevel = 3;
							wcscat( str, L", underground level 3" );
							gpCurrentSectorSummary = gpSectorSummary[x][y][3];
							break;
						case ALTERNATE_GROUND_MASK:
							giCurrLevel = 4;
							wcscat( str, L", alternate G level" );
							gpCurrentSectorSummary = gpSectorSummary[x][y][4];
							break;
						case ALTERNATE_B1_MASK:
							giCurrLevel = 5;
							wcscat( str, L", alternate B1 level" );
							gpCurrentSectorSummary = gpSectorSummary[x][y][5];
							break;
						case ALTERNATE_B2_MASK:
							giCurrLevel = 6;
							wcscat( str, L", alternate B2 level" );
							gpCurrentSectorSummary = gpSectorSummary[x][y][6];
							break;
						case ALTERNATE_B3_MASK:
							giCurrLevel = 7;
							wcscat( str, L", alternate B3 level" );
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
						case 0:	wcscat( gszFilename, L".dat" );				break;
						case 1:	wcscat( gszFilename, L"_b1.dat" );		break;
						case 2:	wcscat( gszFilename, L"_b2.dat" );		break;
						case 3:	wcscat( gszFilename, L"_b3.dat" );		break;
						case 4:	wcscat( gszFilename, L"_a.dat" );			break;
						case 5:	wcscat( gszFilename, L"_b1_a.dat" );	break;
						case 6:	wcscat( gszFilename, L"_b2_a.dat" );	break;
						case 7:	wcscat( gszFilename, L"_b3_a.dat" );	break;
					}
					swprintf(gszDisplayName, lengthof(gszDisplayName), gszFilename);
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
							mprintf(10, 5, L"ITEM DETAILS -- sector %ls", str);
							RenderItemDetails();
						}
						else
						{
							mprintf(10, 20, L"Summary Information for sector %ls:", str);
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
						mprintf(10, y,      L"Summary Information for sector %ls" , str);
						MPrint( 10, y + 10, L"does not exist.");
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
						mprintf(10, 5, L"No information exists for sector %ls.", str);
					else
						mprintf(10, 20, L"No information exists for sector %ls.", str);
					SetFontShadow( FONT_NEARBLACK );

					switch( giCurrentViewLevel )
					{
						case ALL_LEVELS_MASK:
						case GROUND_LEVEL_MASK:			wcscat( gszFilename, L".dat" );				break;
						case BASEMENT1_LEVEL_MASK:	wcscat( gszFilename, L"_b1.dat" );		break;
						case BASEMENT2_LEVEL_MASK:	wcscat( gszFilename, L"_b2.dat" );		break;
						case BASEMENT3_LEVEL_MASK:	wcscat( gszFilename, L"_b3.dat" );		break;
						case ALTERNATE_LEVELS_MASK:
						case ALTERNATE_GROUND_MASK:	wcscat( gszFilename, L"_a.dat" );			break;
						case ALTERNATE_B1_MASK:			wcscat( gszFilename, L"_b1_a.dat" );	break;
						case ALTERNATE_B2_MASK:			wcscat( gszFilename, L"_b2_a.dat" );	break;
						case ALTERNATE_B3_MASK:			wcscat( gszFilename, L"_b3_a.dat" );	break;
					}
					swprintf(gszDisplayName, lengthof(gszDisplayName), gszFilename);
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
					mprintf(MAP_LEFT + 110, MAP_BOTTOM + 55, L"FILE:  %ls", gszDisplayName);
				}
				else //little higher to make room for the override checkbox and text.
				{
					if( !gfAlternateMaps )
						SetFontForeground( FONT_LTKHAKI );
					else
						SetFontForeground( FONT_LTBLUE );
					mprintf(MAP_LEFT + 110, MAP_BOTTOM + 46, L"FILE:  %ls", gszDisplayName);
					if( gubOverrideStatus == READONLY )
					{
						SetFontForeground(gfOverride ? FONT_YELLOW : FONT_LTRED);
						MPrint(MAP_LEFT + 124, MAP_BOTTOM + 61, L"Override READONLY");
					}
					else
					{
						SetFontForeground(gfOverride ? FONT_YELLOW: FONT_ORANGE);
						MPrint(MAP_LEFT + 124, MAP_BOTTOM + 61, L"Overwrite File");
					}
				}
			}
		}
		else if( !gfDeniedSummaryCreation )
		{
			SetFontForeground( FONT_GRAY1 );
			MPrint(10, 20, L"You currently have no summary data.  By creating one, you will be able to keep track");
			MPrint(10, 30, L"of information pertaining to all of the sectors you edit and save.  The creation process");
			MPrint(10, 40, L"will analyse all maps in your /MAPS directory, and generate a new one.  This could");
			MPrint(10, 50, L"take a few minutes depending on how many valid maps you have.  Valid maps are");
			MPrint(10, 60, L"maps following the proper naming convention from a1.dat - p16.dat.  Underground maps");
			MPrint(10, 70, L"are signified by appending _b1 to _b3 before the .dat (ex:  a9_b1.dat).");
			SetFontForeground( FONT_LTRED );
			MPrint(10, 85, L"Do you wish to do this now (y/n)?");
		}
		else
		{
			SetFontForeground( FONT_LTRED );
			MPrint(10, 20, L"No summary info.  Creation denied.");
		}

		SetFont( FONT10ARIAL );
		SetFontForeground( FONT_GRAY3 );
		MPrint(MAP_LEFT + 15, MAP_BOTTOM +  7, L"Grid");
		MPrint(MAP_LEFT + 65, MAP_BOTTOM +  7, L"Progress");
		MPrint(MAP_LEFT + 15, MAP_BOTTOM + 27, L"Use Alternate Maps");
		//Draw the mode tabs
		SetFontForeground( FONT_YELLOW );
		MPrint(354, 18, L"Summary");
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
		MPrint(354, 33, L"Items");
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
			mprintf( MAP_LEFT-8, MAP_TOP+4+y*13, L"%c", 65 + y );
		}
		for( x = 1; x <= 16; x++ )
		{
			wchar_t str[3];
			swprintf(str, lengthof(str), L"%d", x);
			mprintf( MAP_LEFT+x*13-(13+StringPixLength( str, SMALLCOMPFONT ))/2, MAP_TOP-8, str );
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
							mprintf(MAP_LEFT + x * 13 + 4, ClipRect.iTop + 4, L"%d", ubNumUndergroundLevels);
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
							mprintf(MAP_LEFT + x * 13 + 4, ClipRect.iTop + 4, L"%d", ubNumUndergroundLevels);
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
		if( gfWorldLoaded && !gfTempFile && gsSectorX )
		{
			x = MAP_LEFT + (gsSectorX-1) * 13 + 1;
			y = MAP_TOP + (gsSectorY-1) * 13 + 1;
			RectangleDraw( TRUE, x, y, x+11, y+11, Get16BPPColor( FROMRGB( 50, 50, 200 ) ), pDestBuf );
		}

		//Render the grid for the sector currently in focus (red).
		if( gsSelSectorX > 0 && !gfTempFile )
		{
			x = MAP_LEFT + (gsSelSectorX-1) * 13 ;
			y = MAP_TOP + (gsSelSectorY-1) * 13 ;
			RectangleDraw( TRUE, x, y, x+13, y+13, Get16BPPColor( FROMRGB( 200, 50, 50 ) ), pDestBuf );
		}

		//Render the grid for the sector if the mouse is over it (yellow).
		if( gsHiSectorX > 0 )
		{
			x = MAP_LEFT + (gsHiSectorX-1) * 13 - 1;
			y = MAP_TOP + (gsHiSectorY-1) * 13 - 1;
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

void UpdateSectorSummary(const wchar_t* gszFilename, BOOLEAN fUpdate)
{
	wchar_t str[50];
	const wchar_t* ptr;
	INT16 x, y;

	gfRenderSummary = TRUE;
	//Extract the sector
	if( gszFilename[2] < L'0' || gszFilename[2] > L'9' )
		x = gszFilename[1] - L'0';
	else
		x = (gszFilename[1] - L'0') * 10 + gszFilename[2] - L'0';
	if( gszFilename[0] >= 'a' )
		y = gszFilename[0] - L'a' + 1;
	else
		y = gszFilename[0] - L'A' + 1;

	//Validate that the values extracted are in fact a sector
	if( x < 1 || x > 16 || y < 1 || y > 16 )
		return;
	gsSectorX = gsSelSectorX = x;
	gsSectorY = gsSelSectorY = y;

	//The idea here is to get a pointer to the filename's period, then
	//focus on the character previous to it.  If it is a 1, 2, or 3, then
	//the filename was in a basement level.  Otherwise, it is a ground level.
	ptr = wcsstr( gszFilename, L"_a.dat" );
	if( ptr )
	{
		ptr = wcsstr( gszFilename, L"_b" );
		if( ptr && ptr[2] >= '1' && ptr[2] <= '3' && ptr[5] == '.' )
		{ //it's a alternate basement map
			switch( ptr[2] )
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
		ptr = wcsstr( gszFilename, L"_b" );
		if( ptr && ptr[2] >= '1' && ptr[2] <= '3' && ptr[3] == '.' )
		{ //it's a alternate basement map
			switch( ptr[2] )
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
	if( !(gbSectorLevels[gsSectorX-1][gsSectorY-1] & gsSectorLayer) )
	{
		//new sector map saved, so update the global file.
		gbSectorLevels[gsSectorX-1][gsSectorY-1] |= gsSectorLayer;
	}

	if( fUpdate )
	{
		SetFontAttributes(FONT10ARIAL, FONT_LTKHAKI);
		swprintf(str, lengthof(str), L"Analyzing map:  %ls...", gszFilename);

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

		char szCoord[40];
		sprintf( szCoord, "%ls", gszFilename );
		if( gsSectorX > 9 )
			szCoord[3] = '\0';
		else
			szCoord[2] = '\0';
		gusNumEntriesWithOutdatedOrNoSummaryInfo++;
		EvaluateWorld( szCoord, (UINT8)giCurrLevel );

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
				if( !gsSelSectorY )
					gsSelSectorY = 1;
				gsSelSectorX++;
				if( gsSelSectorX > 16 )
					gsSelSectorX = 1;
				break;

			case SDLK_LEFT:
				gfRenderSummary = TRUE;
				if( !gsSelSectorY )
					gsSelSectorY = 1;
				gsSelSectorX--;
				if( gsSelSectorX < 1 )
					gsSelSectorX = 16;
				break;

			case SDLK_UP:
				gfRenderSummary = TRUE;
				if( !gsSelSectorX )
					gsSelSectorX = 1;
				gsSelSectorY--;
				if( gsSelSectorY < 1 )
					gsSelSectorY = 16;
				break;

			case SDLK_DOWN:
				gfRenderSummary = TRUE;
				if( !gsSelSectorX )
					gsSelSectorX = 1;
				gsSelSectorY++;
				if( gsSelSectorY > 16 )
					gsSelSectorY = 1;
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
	SLOGD(DEBUG_TAG_EDITOR, "Generating GlobalSummary Information...");

	gfGlobalSummaryExists = FALSE;

	FileMan::createDir(DEVINFO_DIR);

	// Generate a simple readme file.
	FILE* const f = fopen(DEVINFO_DIR "/readme.txt", "w");
	Assert(f);
	fputs(
		"This information is used in conjunction with the editor.\n"
		"This directory or its contents shouldn't be included with final release.\n",
		f
	);
	fclose(f);

	LoadGlobalSummary();
	RegenerateSummaryInfoForAllOutdatedMaps();
	gfRenderSummary = TRUE;

	SLOGD(DEBUG_TAG_EDITOR, "GlobalSummary Information generated successfully.");
}


static void MapMoveCallback(MOUSE_REGION* reg, INT32 reason)
{
	static INT16 gsPrevX = 0, gsPrevY = 0;
	//calc current sector highlighted.
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gsPrevX = gsHiSectorX = 0;
		gsPrevY = gsHiSectorY = 0;
		gfRenderMap = TRUE;
		return;
	}
	gsHiSectorX = MIN( (reg->RelativeXPos / 13) + 1, 16 );
	gsHiSectorY = MIN( (reg->RelativeYPos / 13) + 1, 16 );
	if( gsPrevX != gsHiSectorX || gsPrevY != gsHiSectorY )
	{
		gsPrevX = gsHiSectorX;
		gsPrevY = gsHiSectorY;
		gfRenderMap = TRUE;
	}
}


static void MapClickCallback(MOUSE_REGION* reg, INT32 reason)
{
	static INT16 sLastX = -1, sLastY = -1;
	//calc current sector selected.
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( GetActiveFieldID() == 1 )
		{
			gsSelSectorX = 0;
			SelectNextField();
		}
		gsSelSectorX = MIN( (reg->RelativeXPos / 13) + 1, 16 );
		gsSelSectorY = MIN( (reg->RelativeYPos / 13) + 1, 16 );
		if( gsSelSectorX != sLastX || gsSelSectorY != sLastY )
		{ //clicked in a new sector
			gfOverrideDirty = TRUE;
			sLastX = gsSelSectorX;
			sLastY = gsSelSectorY;
			switch( giCurrentViewLevel )
			{
				case ALL_LEVELS_MASK:
					if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 0 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 0 ];
					else if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 1 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 1 ];
					else if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 2 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 2 ];
					else if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 3 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 3 ];
					else
						gpCurrentSectorSummary = NULL;
					break;
				case GROUND_LEVEL_MASK: //already pointing to the correct level
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 0 ];
					break;
				case BASEMENT1_LEVEL_MASK:
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 1 ];
					break;
				case BASEMENT2_LEVEL_MASK:
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 2 ];
					break;
				case BASEMENT3_LEVEL_MASK:
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 3 ];
					break;
				case ALTERNATE_LEVELS_MASK:
					if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 4 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 4 ];
					else if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 5 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 5 ];
					else if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 6 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 6 ];
					else if( gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 7 ] )
						gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 7 ];
					else
						gpCurrentSectorSummary = NULL;
					break;
				case ALTERNATE_GROUND_MASK: //already pointing to the correct level
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 4 ];
					break;
				case ALTERNATE_B1_MASK:
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 5 ];
					break;
				case ALTERNATE_B2_MASK:
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 6 ];
					break;
				case ALTERNATE_B3_MASK:
					gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ 7 ];
					break;
			}
			if( gpWorldItemsSummaryArray )
			{
				MemFree( gpWorldItemsSummaryArray );
				gpWorldItemsSummaryArray = NULL;
				gusWorldItemsSummaryArraySize = 0;
			}
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
		const wchar_t* ptr;
		wchar_t str[50];
		gfRenderSummary = TRUE;

		SetFontAttributes(FONT10ARIAL, FONT_LTKHAKI);

		CreateProgressBar(0, MAP_LEFT + 5, MAP_BOTTOM + 110, 151, 10);

		DefineProgressBarPanel( 0, 65, 79, 94, MAP_LEFT, 318, 578, 356 );
		swprintf(str, lengthof(str), L"Loading map:  %ls", gszDisplayName);
		SetProgressBarTitle( 0, str, BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
		SetProgressBarMsgAttributes( 0, SMALLCOMPFONT, FONT_BLACK, FONT_BLACK );

		if(	ExternalLoadMap( gszDisplayName ) )
		{
			EnableButton( iSummaryButton[ SUMMARY_OKAY ] );
			gsSectorX = gsSelSectorX;
			gsSectorY = gsSelSectorY;
			gfOverrideDirty = TRUE;
		}
		RemoveProgressBar( 0 );
		ptr = wcsstr( gszDisplayName, L"_b" );
		if( !ptr || ptr[3] != L'.' )
		{
			gsSectorLayer = GROUND_LEVEL_MASK;
			giCurrLevel = 0;
		}
		else
		{
			switch( ptr[2] - L'0' )
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
		gfRenderSummary = TRUE;
		if (gubOverrideStatus == INACTIVE || gfOverride)
		{
			if( gubOverrideStatus == READONLY )
			{
				FileClearAttributes(GCM->getMapPath(gszDisplayName));
			}
			if(	ExternalSaveMap( gszDisplayName ) )
			{
				if( gsSelSectorX && gsSelSectorY )
				{
					gsSectorX = gsSelSectorX;
					gsSectorY = gsSelSectorY;
					gfOverrideDirty = TRUE;
				}
			}
		}
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
	std::string filename;
	gfOverrideDirty = FALSE;
	gfOverride = FALSE;
	if( gfTempFile )
	{
		// if empty, use "test.dat"
		if (!wcslen(gszTempFilename))
		{
			filename = GCM->getMapPath(gszTempFilename);
		}
		else
		{
			filename = GCM->getMapPath("test.dat");
		}

		filename = FileMan::replaceExtension(filename, ".dat");
	}
	else
	{
		filename = GCM->getMapPath(gszFilename);
	}

	swprintf(gszDisplayName, lengthof(gszDisplayName), L"%hs", FileMan::getFileName(filename).c_str());

	const UINT32 attr = FileGetAttributes(filename.c_str());
	if (attr != FILE_ATTR_ERROR)
	{
		if( gfWorldLoaded )
		{
			gubOverrideStatus = (attr & FILE_ATTR_READONLY ? READONLY : OVERWRITE);
			ShowButton( iSummaryButton[ SUMMARY_OVERRIDE ] );
			iSummaryButton[SUMMARY_OVERRIDE]->uiFlags &= ~BUTTON_CLICKED_ON;
			DisableButton( iSummaryButton[ SUMMARY_SAVE ] );
		}
		if( gfTempFile )
			EnableButton( iSummaryButton[ SUMMARY_LOAD ] );
	}
	else
	{
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

		FileRead(f_map, &dMajorMapVersion, sizeof(FLOAT));
	}

	FILE* const f_sum = fopen(summary_filename, "rb");
	if (!f_sum)
	{
		++gusNumEntriesWithOutdatedOrNoSummaryInfo;
	}
	else
	{
		/* Even if the info is outdated (but existing), allocate the structure, but
		 * indicate that the info is bad. */
		SUMMARYFILE* const sum = MALLOC(SUMMARYFILE);
		fread(sum, sizeof(SUMMARYFILE), 1, f_sum);
		fclose(f_sum);

		if (sum->ubSummaryVersion < MINIMUMVERSION ||
				dMajorMapVersion      < getMajorMapVersion())
		{
			++gusNumberOfMapsToBeForceUpdated;
			gfMustForceUpdateAllMaps = TRUE;
		}
		sum->dMajorMapVersion = dMajorMapVersion;
		UpdateSummaryInfo(sum);

		SUMMARYFILE** const anchor = &gpSectorSummary[x][y][level];
		if (*anchor) MemFree(*anchor);
		*anchor = sum;

		if (sum->ubSummaryVersion < GLOBAL_SUMMARY_VERSION)
			++gusNumEntriesWithOutdatedOrNoSummaryInfo;
	}

	return TRUE;
}


static void LoadGlobalSummary(void)
{
	SLOGD(DEBUG_TAG_EDITOR, "Executing LoadGlobalSummary()...");

	gfMustForceUpdateAllMaps        = FALSE;
	gusNumberOfMapsToBeForceUpdated = 0;

	const FileAttributes attr = FileGetAttributes(DEVINFO_DIR);
	gfGlobalSummaryExists = attr != FILE_ATTR_ERROR && attr & FILE_ATTR_DIRECTORY;
	if (!gfGlobalSummaryExists)
	{
		SLOGW(DEBUG_TAG_EDITOR, "LoadGlobalSummary() aborted -- doesn't exist on this local computer.");
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
		SLOGD(DEBUG_TAG_EDITOR, "Sector Row %c complete...", y + 'A');
	}

	if (gfMustForceUpdateAllMaps)
	{
		SLOGW(DEBUG_TAG_EDITOR, "A MAJOR MAP UPDATE EVENT HAS BEEN DETECTED FOR %d MAPS!", gusNumberOfMapsToBeForceUpdated);
	}

	SLOGD(DEBUG_TAG_EDITOR, "LoadGlobalSummary() finished...");
}


static void UpdateMasterProgress(void);


void WriteSectorSummaryUpdate(const char* const filename, const UINT8 ubLevel, SUMMARYFILE* const sf)
{
	const char* const ext = strstr(filename, ".dat");
	AssertMsg(ext, "Illegal sector summary filename.");

	STRING512 summary_filename;
	snprintf(summary_filename, lengthof(summary_filename), DEVINFO_DIR "/%.*s.sum", (int)(ext - filename), filename);

	FILE* const f = fopen(summary_filename, "wb");
	Assert(f);
	fwrite(sf, sizeof(*sf), 1, f);
	fclose(f);

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
			SetRelativeStartAndEndPercentage( 2, (UINT16)MasterStart, (UINT16)MasterEnd, NULL );
			RenderProgressBar( 2, 0 );
		}
		else
			SetRelativeStartAndEndPercentage( 1, (UINT16)MasterStart, (UINT16)MasterEnd, NULL );
	}
}


static void ReportError(const char* pSector, UINT8 ubLevel)
{
	static INT32 yp = 180;
	wchar_t str[40];
	wchar_t temp[10];

	//Make sure the file exists... if not, then return false
	swprintf(str, lengthof(str), L"%hs", pSector);
	if( ubLevel % 4  )
	{
		swprintf(temp, lengthof(temp), L"_b%d.dat", ubLevel % 4);
		wcscat( str, temp );
	}
	mprintf(10, yp, L"Skipping update for %ls.  Probably due to tileset conflicts...", str);
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
	SetProgressBarTitle( 0, L"Generating map information", BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
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
		SetProgressBarTitle( 0, L"Generating map summary", BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
		SetProgressBarMsgAttributes( 0, SMALLCOMPFONT, FONT_BLACK, FONT_BLACK );

		if( gpCurrentSectorSummary )
		{
			MemFree( gpCurrentSectorSummary );
			gpCurrentSectorSummary = NULL;
		}

		char str[40];
		sprintf( str, "%c%d", gsSelSectorY + 'A' - 1, gsSelSectorX );
		EvaluateWorld( str, (UINT8)giCurrLevel );

		gpSectorSummary[ gsSelSectorX ][ gsSelSectorY ][ giCurrLevel ] = gpCurrentSectorSummary;

		gfRenderSummary = TRUE;

		RemoveProgressBar( 0 );
	}
}


static void ExtractTempFilename(void)
{
	wchar_t const* const str = GetStringFromField(1);
	if( wcscmp( gszTempFilename, str ) )
	{
		wcscpy( gszTempFilename, str );
		gfRenderSummary = TRUE;
		gfOverrideDirty = TRUE;
	}
	if( !wcslen( str ) )
		wcslcpy(gszDisplayName, L"test.dat", lengthof(gszDisplayName));
}

void ApologizeOverrideAndForceUpdateEverything()
{
	INT32 x, y;
	wchar_t str[50];
	SUMMARYFILE *pSF;
	//Create one huge assed button
	gfMajorUpdate = TRUE;
	iSummaryButton[SUMMARY_BACKGROUND] = CreateLabel(NULL, 0, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH - 1);
	//Draw it
	iSummaryButton[SUMMARY_BACKGROUND]->Draw();
	InvalidateScreen();
	SetFontAttributes(HUGEFONT, FONT_RED);
	const wchar_t* UpdateInfo = L"MAJOR VERSION UPDATE";
	MPrint((SCREEN_WIDTH - StringPixLength(UpdateInfo, HUGEFONT)) / 2, 105, UpdateInfo);
	SetFont( FONT10ARIAL );
	SetFontForeground( FONT_YELLOW );
	swprintf(str, lengthof(str), L"There are %d maps requiring a major version update.", gusNumberOfMapsToBeForceUpdated);
	MPrint((SCREEN_WIDTH - StringPixLength(str, FONT10ARIAL)) / 2, 130, str);

	CreateProgressBar(2, 120, 170, 400, 32);
	DefineProgressBarPanel( 2, 65, 79, 94, 100, 150, 540, 222 );
	SetProgressBarTitle( 2, L"Updating all outdated maps", BLOCKFONT2, FONT_RED, 0 );
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
	if( gpWorldItemsSummaryArray )
	{
		MemFree( gpWorldItemsSummaryArray );
		gpWorldItemsSummaryArray = NULL;
		gusWorldItemsSummaryArraySize = 0;
	}
	if( gpPEnemyItemsSummaryArray )
	{
		MemFree( gpPEnemyItemsSummaryArray );
		gpPEnemyItemsSummaryArray = NULL;
		gusPEnemyItemsSummaryArraySize = 0;
	}
	if( gpNEnemyItemsSummaryArray )
	{
		MemFree( gpNEnemyItemsSummaryArray );
		gpNEnemyItemsSummaryArray = NULL;
		gusNEnemyItemsSummaryArraySize = 0;
	}

	if( !gpCurrentSectorSummary->uiNumItemsPosition )
	{	//Don't have one, so generate them
		if( gpCurrentSectorSummary->ubSummaryVersion == GLOBAL_SUMMARY_VERSION )
			gusNumEntriesWithOutdatedOrNoSummaryInfo++;
		SummaryUpdateCallback(iSummaryButton[SUMMARY_UPDATE], MSYS_CALLBACK_REASON_LBUTTON_UP);
		gpCurrentSectorSummary = gpSectorSummary[ gsSelSectorX - 1 ][ gsSelSectorY - 1 ][ giCurrLevel ];
	}
	//Open the original map for the sector
	AutoSGPFile hfile(GCM->openMapForReading(gszFilename));
	//Now fileseek directly to the file position where the number of world items are stored
	FileSeek(hfile, gpCurrentSectorSummary->uiNumItemsPosition, FILE_SEEK_FROM_START);
	//Now load the number of world items from the map.
	FileRead(hfile, &uiNumItems, 4);
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
	gusWorldItemsSummaryArraySize = gpCurrentSectorSummary->usNumItems;
	if (gusWorldItemsSummaryArraySize != 0)
	{
		gpWorldItemsSummaryArray = MALLOCN(WORLDITEM, uiNumItems);
		FileRead(hfile, gpWorldItemsSummaryArray, sizeof(WORLDITEM) * uiNumItems);
	}

	//NOW, do the enemy's items!
	//We need to do two passes.  The first pass simply processes all the enemies and counts all the droppable items
	//keeping track of two different values.  The first value is the number of droppable items that come off of
	//enemy detailed placements, the other counter keeps track of the number of droppable items that come off of
	//normal enemy placements.  After doing this, the memory is allocated for the tables that will store all the item
	//summary information, then the second pass will repeat the process, except it will record the actual items.

	//PASS #1
	FileSeek(hfile, gpCurrentSectorSummary->uiEnemyPlacementPosition, FILE_SEEK_FROM_START);
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
		gpPEnemyItemsSummaryArray = MALLOCNZ(OBJECTTYPE, gusPEnemyItemsSummaryArraySize);
	}
	if( gusNEnemyItemsSummaryArraySize )
	{
		gpNEnemyItemsSummaryArray = MALLOCNZ(OBJECTTYPE, gusNEnemyItemsSummaryArraySize);
	}

	//PASS #2
	//During this pass, simply copy all the data instead of counting it, now that we have already done so.
	usPEnemyIndex = usNEnemyIndex = 0;
	FileSeek(hfile, gpCurrentSectorSummary->uiEnemyPlacementPosition, FILE_SEEK_FROM_START);
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
