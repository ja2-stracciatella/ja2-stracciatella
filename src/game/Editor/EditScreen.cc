#include "Editor_Callback_Prototypes.h"
#include "Font.h"
#include "Font_Control.h"
#include "HImage.h"
#include "Local.h"
#include "SGP.h"
#include "Button_System.h"
#include "Structure.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "Input.h"
#include "Smooth.h"
#include "WorldMan.h"
#include "Edit_Sys.h"
#include "EditScreen.h"
#include "Sys_Globals.h"
#include "SmartMethod.h"
#include "SelectWin.h"
#include "Interface.h"
#include "Lighting.h"
#include "Overhead_Types.h"
#include "Overhead.h"
#include "Soldier_Control.h"
#include "Handle_UI.h"
#include "Event_Pump.h"
#include "World_Items.h"
#include "LoadScreen.h"
#include "Render_Dirty.h"
#include "Isometric_Utils.h"
#include "Message.h"
#include "Render_Fun.h"
#include "PopupMenu.h"
#include "Overhead_Map.h"
#include "EditorDefines.h"
#include "EditorTerrain.h"
#include "EditorBuildings.h"
#include "EditorItems.h"
#include "EditorMercs.h"
#include "EditorMapInfo.h"
#include "NewSmooth.h"
#include "Smoothing_Utils.h"
#include "MessageBox.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "Text_Input.h"
#include "Cursor_Modes.h"
#include "Editor_Taskbar_Utils.h"
#include "Editor_Modes.h"
#include "Editor_Undo.h"
#include "Exit_Grids.h"
#include "Item_Statistics.h"
#include "Map_Information.h"
#include "Sector_Summary.h"
#include "Game_Clock.h"
#include "Game_Init.h"
#include "Environment.h"
#include "Simple_Render_Utils.h"
#include "Map_Edgepoints.h"
#include "Line.h"
#include "English.h"
#include "Random.h"
#include "Scheduling.h"
#include "Road_Smoothing.h"
#include "StrategicMap.h"
#include "Pits.h"
#include "Inventory_Choosing.h"
#include "Music_Control.h"
#include "Soldier_Profile.h"
#include "GameSettings.h"
#include "JAScreens.h"
#include "Shading.h"
#include "Debug.h"
#include "Video.h"
#include "VObject_Blitters.h"
#include "UILayout.h"
#include "Logger.h"


#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>


static BOOLEAN gfCorruptMap        = FALSE;
static BOOLEAN gfCorruptSchedules  = FALSE;
BOOLEAN        gfProfileDataLoaded = FALSE;


// These are global variables used by the main game loop

static UINT32 guiSaveTacticalStatusFlags; // saves the tactical status flags when entering the editor.

BOOLEAN			gfAutoLoadA9 = FALSE;
//new vars added by Kris
BOOLEAN			gfRenderWorld = FALSE;
BOOLEAN			gfRenderTaskbar = FALSE;
BOOLEAN			gfRenderDrawingMode = FALSE;
static BOOLEAN gfFirstPlacement          = TRUE;
BOOLEAN			gfPendingBasement = FALSE;
BOOLEAN			gfPendingCaves = FALSE;
static BOOLEAN gfNeedToInitGame          = FALSE;
static BOOLEAN gfScheduleReversalPending = FALSE;
static BOOLEAN gfRemoveLightsPending     = FALSE;
static BOOLEAN gfScheduleClearPending    = FALSE;
BOOLEAN			gfConfirmExitFirst = TRUE;
static BOOLEAN gfConfirmExitPending      = FALSE;
BOOLEAN			gfIntendOnEnteringEditor = FALSE;

//original
static FLOAT gShadePercent = 0.65f;

UINT16			gusLightLevel = 0;
static UINT16 gusGameLightLevel  = 0;
static UINT16 gusSavedLightLevel = 0;
BOOLEAN			gfFakeLights = FALSE;

INT16				gsLightRadius = 5;

static BOOLEAN gfOldDoVideoScroll;     // Saved for returning to previous settings
static UINT8   gubOldCurScrollSpeedID; // Saved for returning to previous settings

TaskMode iOldTaskMode    = TASK_OPTIONS;
TaskMode iCurrentTaskbar = TASK_NONE;
TaskMode iTaskMode       = TASK_NONE;

BOOLEAN gfMercResetUponEditorEntry;


static BOOLEAN fHelpScreen = FALSE;

BOOLEAN fDontUseRandom = FALSE;

static LEVELNODE* gCursorNode = NULL;

static MusicMode giMusicMode = MUSIC_MAIN_MENU;


INT32 iDrawMode = DRAW_MODE_NOTHING;
INT32 iCurrentAction,iActionParam;

GUIButtonRef iEditorButton[NUMBER_EDITOR_BUTTONS];
ToolbarMode  iEditorToolbarState = TBAR_MODE_NONE;

static UINT16 iCurBankMapIndex;

static InputAtom EditorInputEvent;
static BOOLEAN fBeenWarned              = FALSE;
static BOOLEAN fEditModeFirstTime       = TRUE;
static BOOLEAN fFirstTimeInEditModeInit = TRUE;
static BOOLEAN fSelectionWindow         = FALSE;
static BOOLEAN gfRealGunNut             = TRUE;

static BOOLEAN fNewMap = FALSE;

static INT32       iPrevDrawMode          = DRAW_MODE_NOTHING;
static UINT16      PrevCurrentPaste       = FIRSTTEXTURE;
static INT32       gPrevCurrentBackground = FIRSTTEXTURE;
static ToolbarMode iPrevJA2ToolbarState   = TBAR_MODE_NONE;

UINT16 gusEditorTaskbarColor;
UINT16 gusEditorTaskbarHiColor;
UINT16 gusEditorTaskbarLoColor;

BOOLEAN gfGotoGridNoUI = FALSE;
static GUIButtonRef guiGotoGridNoUIButtonID;
static MOUSE_REGION GotoGridNoUIRegion;

//----------------------------------------------------------------------------------------------
//	EditScreenInit
//
//	This function is called once at SGP (and game) startup
//
void EditScreenInit(void)
{
	gfFakeLights = FALSE;

	eInfo.fGameInit = TRUE;
	GameInitEditorBuildingInfo();
	GameInitEditorMercsInfo();

	//Set the editor colors.
	//gusEditorTaskbarColor = 9581;
	//gusEditorTaskbarColor =		Get16BPPColor( FROMRGB(  72,  88, 104 ) );
	//gusEditorTaskbarHiColor = Get16BPPColor( FROMRGB( 136, 138, 135 ) );
	//gusEditorTaskbarLoColor = Get16BPPColor( FROMRGB(  24,  61,  81 ) );

	gusEditorTaskbarColor   = Get16BPPColor( FROMRGB(  65,  79,  94 ) );
	gusEditorTaskbarHiColor = Get16BPPColor( FROMRGB( 122, 124, 121 ) );
	gusEditorTaskbarLoColor = Get16BPPColor( FROMRGB(  22,  55,  73 ) );

	InitializeRoadMacros();

	InitArmyGunTypes();
}


//	This function is called once at shutdown of the game
void EditScreenShutdown(void)
{
	GameShutdownEditorMercsInfo();
	RemoveAllFromUndoList();
}


//	Editor's Init code. Called each time we enter edit mode from the game.
static void EditModeInit(void)
{
	UINT32 x;

	SLOGI("Entering editor mode...");

	gfRealGunNut = gGameOptions.fGunNut;
	gGameOptions.fGunNut = TRUE;


	if( !gfProfileDataLoaded )
		LoadMercProfiles();

	ClearTacticalMessageQueue( );

	PauseGame();
	fEditModeFirstTime = FALSE;

	if( fFirstTimeInEditModeInit )
	{
		if( gfWorldLoaded )
			InitJA2SelectionWindow();
		fFirstTimeInEditModeInit = FALSE;
	}

	//Initialize editor specific stuff for each Taskbar.
	EntryInitEditorTerrainInfo();
	EntryInitEditorItemsInfo();
	EntryInitEditorMercsInfo();

	gEditorLightColor = *LightGetColor();

	//essentially, we are turning the game off so the game doesn't process in conjunction with the
	//editor.
	guiSaveTacticalStatusFlags = gTacticalStatus.uiFlags;
	gTacticalStatus.uiFlags |= SHOW_ALL_ITEMS;
	gTacticalStatus.uiTimeOfLastInput = GetJA2Clock();
	gTacticalStatus.uiTimeSinceDemoOn = gTacticalStatus.uiTimeOfLastInput;

	//Remove the radar from the interface.
	RemoveCurrentTacticalPanelButtons( );
	gRadarRegion.Disable();

	CreateEditorTaskbar();

	//Hide all of the buttons here.  DoTaskbar() will handle the
	//showing and hiding of the buttons.
	for( x = LAST_EDITORTAB_BUTTON+1; x < NUMBER_EDITOR_BUTTONS; x++ )
		HideButton( iEditorButton[x] );

	iEditorToolbarState = iPrevJA2ToolbarState;
	iDrawMode = iPrevDrawMode;
	CurrentPaste = PrevCurrentPaste;
	gCurrentBackground = gPrevCurrentBackground;

	iCurrentTaskbar = TASK_NONE;
	iTaskMode = iOldTaskMode;

	DoTaskbar();

	fDontUseRandom = FALSE;
	fSelectionWindow = FALSE;

	// Set renderer to ignore redundency
	gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;

	fHelpScreen = FALSE;

	gfEditMode = TRUE;
	fNewMap = FALSE;

	gfEditingDoor = FALSE;

	gusGameLightLevel = LightGetAmbient();
	if( !gfBasement && !gfCaves )
		gusLightLevel =  12;//EDITOR_LIGHT_MAX - (UINT16)LightGetAmbient();
	else
		gusLightLevel = EDITOR_LIGHT_MAX - (UINT16)LightGetAmbient();

	if( gfFakeLights )
	{
		gusSavedLightLevel = gusLightLevel;
		gusLightLevel = EDITOR_LIGHT_FAKE;
		ClickEditorButton( MAPINFO_TOGGLE_FAKE_LIGHTS );
	}

	gfRenderWorld = TRUE;
	gfRenderTaskbar = TRUE;

	//Reset the corruption detection flags.
	gfCorruptMap = FALSE;
	gfCorruptSchedules = FALSE;

	// save old scrolling mode, set renderer to not use video scroll
	gfOldDoVideoScroll = gfDoVideoScroll;
	gubOldCurScrollSpeedID = gubCurScrollSpeedID;
	/// set new ones

	gfRoofPlacement = FALSE;

	EnableUndo();

	RemoveMercsInSector( );
	if( gfWorldLoaded )
	{
		gfConfirmExitFirst = TRUE;
		ShowEntryPoints();
		PrepareSchedulesForEditorEntry();
		if( gfMercResetUponEditorEntry )
		{
			UseEditorOriginalList();
			ResetAllMercPositions();
		}
		else
			UseEditorAlternateList();
		BuildItemPoolList();
		SetEditorSmoothingMode( gMapInformation.ubEditorSmoothingType );
		AddLockedDoorCursors();

		FOR_EACH_LIGHT_SPRITE(l)
		{
			if (!(l->uiFlags & (LIGHT_SPR_ON | MERC_LIGHT)))
			{
				LightSpritePower(l, TRUE);
			}
		}

		if( gfShowPits )
		{
			AddAllPits();
		}

		LightSetBaseLevel( (UINT8)(EDITOR_LIGHT_MAX - gusLightLevel) );
		ShowLightPositionHandles();
		LightSpriteRenderAll();
	}
	else
	{
		SLOGD("Creating summary window...");
		CreateSummaryWindow();
		gfNeedToInitGame = TRUE;
	}

	SetMercTeamVisibility( ENEMY_TEAM, gfShowEnemies );
	SetMercTeamVisibility( CREATURE_TEAM, gfShowCreatures );
	SetMercTeamVisibility( MILITIA_TEAM, gfShowRebels );
	SetMercTeamVisibility( CIV_TEAM, gfShowCivilians );

	// remove mouse region for pause of game
	RemoveMouseRegionForPauseOfClock( );

	gfIntendOnEnteringEditor = FALSE;

	SLOGD("Finished entering editor mode...");
}


static void HideEntryPoints(void);
static void MapOptimize(void);
static void RemoveLightPositionHandles(void);
static void UpdateLastActionBeforeLeaving(void);


//	The above function's counterpart. Called when exiting the editor back to the game.
static BOOLEAN EditModeShutdown(void)
{
	if( gfConfirmExitFirst )
	{
		gfConfirmExitPending = TRUE;
		CreateMessageBox( "Exit editor?" );
		return FALSE;
	}

	gfEditMode = FALSE;
	fEditModeFirstTime = TRUE;

	UpdateLastActionBeforeLeaving();

	DeleteEditorTaskbar();

	CreateMouseRegionForPauseOfClock();

	iOldTaskMode = iCurrentTaskbar;
	gTacticalStatus.uiFlags = guiSaveTacticalStatusFlags;

	RemoveLightPositionHandles( );

	MapOptimize();

	RemoveCursors();

	fHelpScreen = FALSE;
	// Set render back to normal mode
	gTacticalStatus.uiFlags &= ~(NOHIDE_REDUNDENCY | SHOW_ALL_ITEMS);
	UpdateRoofsView( );
	UpdateWallsView( );
	// Invalidate all redundency checks
	InvalidateWorldRedundency();


	// If false lighting is on in the editor, turn it off!
	if ( gfFakeLights )
	{
		gusLightLevel = gusSavedLightLevel;
	}

	LightSpriteRenderAll();

	if( gfWorldLoaded )
	{
		ClearRenderFlags( RENDER_FLAG_SAVEOFF );
		MarkWorldDirty();
		RenderWorld( );
	}

	InvalidateScreen( );
	ExecuteBaseDirtyRectQueue();

	gRadarRegion.Enable();
	CreateCurrentTacticalPanelButtons( );

	// Make sure to turn off demo mode!
	gTacticalStatus.uiTimeOfLastInput = GetJA2Clock();
	gTacticalStatus.uiTimeSinceDemoOn = GetJA2Clock();

	// RETURN TO PREVIOUS SCROLL MODE
	gfDoVideoScroll = gfOldDoVideoScroll;
	gubCurScrollSpeedID = gubOldCurScrollSpeedID;


	DisableUndo();

	SetFontShadow( DEFAULT_SHADOW );


	if( gfWorldLoaded )
	{
		CompileWorldMovementCosts();
		RaiseWorldLand();
		HideEntryPoints();
		KillItemPoolList();
		PrepareSchedulesForEditorExit();
		RemoveAllObjectsOfTypeRange( gsSelectedMercGridNo, CONFIRMMOVE, CONFIRMMOVE );
		RemoveLockedDoorCursors();
		if( gfShowPits )
		{
			RemoveAllPits();
		}
	}

	if( gfNeedToInitGame )
	{
		InitStrategicLayer();
		WarpGameTime( 1 , WARPTIME_PROCESS_EVENTS_NORMALLY ); //to avoid helicopter setup
		gfNeedToInitGame = FALSE;
	}
	else
	{
		if( !gfBasement && !gfCaves )
			LightSetBaseLevel( (UINT8)gusGameLightLevel );
		UpdateMercsInSector();
	}

	gGameOptions.fGunNut = gfRealGunNut;

	DeleteBuildingLayout();

	HideExitGrids();

	UnPauseGame();

	return TRUE;
}


//	Forces all land on the map to have the same ground texture.
static void SetBackgroundTexture(void)
{
	int						cnt;
	UINT16				usIndex;

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		// Erase old layers
		RemoveAllLandsOfTypeRange( cnt, FIRSTTEXTURE, DEEPWATERTEXTURE );

		// Add level
		usIndex = (UINT16)Random(10);

		// Adjust for type
		usIndex += gTileTypeStartIndex[ gCurrentBackground ];

		// Set land index
		if (TypeRangeExistsInLandLayer(cnt, FIRSTFLOOR, LASTFLOOR))
			AddLandToTail( cnt, usIndex ); //show the land below the floor.
		else
			AddLandToHead( cnt, usIndex ); //no floor so no worries.
	}
}


//	Displays the selection window and handles it's exit condition. Used by WaitForSelectionWindow
static BOOLEAN DoWindowSelection(void)
{
	RenderSelectionWindow( );
	RenderFastHelp( );
	if ( fAllDone )
	{
		switch ( iDrawMode )
		{
			case DRAW_MODE_WALLS:
			case DRAW_MODE_DOORS:
			case DRAW_MODE_WINDOWS:
			case DRAW_MODE_ROOFS:
			case DRAW_MODE_BROKEN_WALLS:
			case DRAW_MODE_DECOR:
			case DRAW_MODE_DECALS:
			case DRAW_MODE_FLOORS:
			case DRAW_MODE_TOILET:
			case DRAW_MODE_ROOM:
			case DRAW_MODE_NEWROOF:
			case DRAW_MODE_OSTRUCTS:
			case DRAW_MODE_OSTRUCTS1:
			case DRAW_MODE_OSTRUCTS2:
			case DRAW_MODE_DEBRIS:
			case DRAW_MODE_BANKS:
			case DRAW_MODE_ROADS:
				break;
			default:
				iDrawMode = DRAW_MODE_NOTHING;
				break;
 		}
		RemoveJA2SelectionWindow( );
		return ( TRUE );
	}
	return ( FALSE );
}


//Whenever the mouse attaches an object to the cursor, it has to be removed, so it doesn't stay
//in the world.
static void RemoveTempMouseCursorObject(void)
{
	if ( iCurBankMapIndex < GRIDSIZE )
	{
		ForceRemoveStructFromTail( iCurBankMapIndex );
		gCursorNode = NULL;
	}
}


//Whenever the editor wishes to show an object in the world, it will temporarily attach it to
//the mouse cursor, to indicate what is about to be drawn.
static BOOLEAN DrawTempMouseCursorObject(void)
{
	UINT16	usUseIndex;
	UINT16	usUseObjIndex;

	switch ( iDrawMode )
	{
		case DRAW_MODE_ROOM:
			pSelList = SelRoom;
			pNumSelList = &iNumRoomsSelected;
			return FALSE;  //a special case where we just want to get the info and not display a cursor.
		case DRAW_MODE_NEWROOF:
			pSelList = SelSingleNewRoof;
			pNumSelList = &iNumNewRoofsSelected;
			break;
		case DRAW_MODE_WALLS:
			pSelList = SelSingleWall;
			pNumSelList = &iNumWallsSelected;
			break;
		case DRAW_MODE_DOORS:
			pSelList = SelSingleDoor;
			pNumSelList = &iNumDoorsSelected;
			break;
		case DRAW_MODE_WINDOWS:
			pSelList = SelSingleWindow;
			pNumSelList = &iNumWindowsSelected;
			break;
		case DRAW_MODE_ROOFS:
			pSelList = SelSingleRoof;
			pNumSelList = &iNumRoofsSelected;
			break;
		case DRAW_MODE_BROKEN_WALLS:
			pSelList = SelSingleBrokenWall;
			pNumSelList = &iNumBrokenWallsSelected;
			break;
		case DRAW_MODE_DECOR:
			pSelList = SelSingleDecor;
			pNumSelList = &iNumDecorSelected;
			break;
		case DRAW_MODE_DECALS:
			pSelList = SelSingleDecal;
			pNumSelList = &iNumDecalsSelected;
			break;
		case DRAW_MODE_FLOORS:
			pSelList = SelSingleFloor;
			pNumSelList = &iNumFloorsSelected;
			break;
		case DRAW_MODE_TOILET:
			pSelList = SelSingleToilet;
			pNumSelList = &iNumToiletsSelected;
			break;
		case DRAW_MODE_BANKS:
			pSelList = SelBanks;
			pNumSelList = &iNumBanksSelected;
			break;
		case DRAW_MODE_ROADS:
			pSelList = SelRoads;
			pNumSelList = &iNumRoadsSelected;
			break;
		case DRAW_MODE_OSTRUCTS:
			pSelList = SelOStructs;
			pNumSelList = &iNumOStructsSelected;
			break;
		case DRAW_MODE_OSTRUCTS1:
			pSelList = SelOStructs1;
			pNumSelList = &iNumOStructs1Selected;
			break;
		case DRAW_MODE_OSTRUCTS2:
			pSelList = SelOStructs2;
			pNumSelList = &iNumOStructs2Selected;
			break;
		case DRAW_MODE_DEBRIS:
			pSelList = SelDebris;
			pNumSelList = &iNumDebrisSelected;
			break;
	}

	const GridNo pos = guiCurrentCursorGridNo;
	if (pos == NOWHERE) return FALSE;

	iCurBankMapIndex = pos;
	if (iCurBankMapIndex >= GRIDSIZE) return FALSE;

	//Hook into the smart methods to override the selection window methods.
	switch (iDrawMode)
	{
		case DRAW_MODE_SMART_WALLS:
			if (!CalcWallInfoUsingSmartMethod(iCurBankMapIndex, &usUseObjIndex, &usUseIndex))
			{
				return FALSE;
			}
			break;

		case DRAW_MODE_SMART_DOORS:
			if (!CalcDoorInfoUsingSmartMethod(iCurBankMapIndex, &usUseObjIndex, &usUseIndex))
			{
				return FALSE;
			}
			break;

		case DRAW_MODE_SMART_WINDOWS:
			if (!CalcWindowInfoUsingSmartMethod(iCurBankMapIndex, &usUseObjIndex, &usUseIndex))
			{
				return FALSE;
			}
			break;

		case DRAW_MODE_SMART_BROKEN_WALLS:
			if (!CalcBrokenWallInfoUsingSmartMethod(iCurBankMapIndex, &usUseObjIndex, &usUseIndex))
			{
				return FALSE;
			}
			if (usUseObjIndex == 0xffff || usUseIndex == 0xffff)
			{
				return FALSE;
			}
			break;

		default:
			usUseIndex    = pSelList[iCurBank].usIndex;
			usUseObjIndex = pSelList[iCurBank].uiObject;
			break;
	}
	gCursorNode = ForceStructToTail(iCurBankMapIndex, (UINT16)(gTileTypeStartIndex[usUseObjIndex] + usUseIndex));
	// ATE: Set this levelnode as dynamic!
	gCursorNode->uiFlags |= LEVELNODE_DYNAMIC;
	return TRUE;
}


//Displays the current drawing object in the small, lower left window of the editor's toolbar.
void ShowCurrentDrawingMode( void )
{
	INT32				iShowMode;
	UINT16			usUseIndex;
	UINT16			usObjIndex;
	INT32				iIndexToUse;

	INT32 const x =   0;
	INT32 const y =  40 + TASKBAR_Y;
	INT32 const w = 100;
	INT32 const h =  58;

	// Set up a clipping rectangle for the display window.
	SGPRect NewRect;
	NewRect.iLeft   = x;
	NewRect.iTop    = y;
	NewRect.iRight  = x + w;
	NewRect.iBottom = y + h;

	SGPRect const ClipRect = SetClippingRect(NewRect);

	// Clear it out
	ColorFillVideoSurfaceArea(FRAME_BUFFER, x, y, x + w, y + h, 0);

	iShowMode = iDrawMode;
	if ( iDrawMode >= DRAW_MODE_ERASE )
		iShowMode -= DRAW_MODE_ERASE;

	usUseIndex = usObjIndex = 0xffff;

	iIndexToUse = 0;
	if ( fDontUseRandom )
		iIndexToUse = iCurBank;

	// Select object to be displayed depending on the current editing mode.
	switch ( iShowMode )
	{
		case DRAW_MODE_ROOM:
		case DRAW_MODE_SLANTED_ROOF:
			if ( iNumWallsSelected > 0 )
			{
				usUseIndex = SelRoom[0].usIndex;
				usObjIndex = (UINT16)SelRoom[0].uiObject;
			}
			break;

		case DRAW_MODE_GROUND:
		case (DRAW_MODE_GROUND + DRAW_MODE_FILL_AREA):
			usUseIndex = 0;
			usObjIndex = CurrentPaste;
			break;

		case DRAW_MODE_OSTRUCTS:
			if ( iNumOStructsSelected > 0 )
			{
				usUseIndex = SelOStructs[ iIndexToUse ].usIndex;
				usObjIndex = (UINT16)SelOStructs[ iIndexToUse ].uiObject;
			}
			break;

		case DRAW_MODE_OSTRUCTS1:
			if ( iNumOStructs1Selected > 0 )
			{
				usUseIndex = SelOStructs1[ iIndexToUse ].usIndex;
				usObjIndex = (UINT16)SelOStructs1[ iIndexToUse ].uiObject;
			}
			break;

		case DRAW_MODE_OSTRUCTS2:
			if ( iNumOStructs2Selected > 0 )
			{
				usUseIndex = SelOStructs2[ iIndexToUse ].usIndex;
				usObjIndex = (UINT16)SelOStructs2[ iIndexToUse ].uiObject;
			}
			break;

		case DRAW_MODE_DEBRIS:
			if ( iNumDebrisSelected > 0 )
			{
				usUseIndex = SelDebris[ iIndexToUse ].usIndex;
				usObjIndex = (UINT16)SelDebris[ iIndexToUse ].uiObject;
			}
			break;

		case DRAW_MODE_BANKS:
			if ( iNumBanksSelected > 0 )
			{
				usUseIndex = SelBanks[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelBanks[ iCurBank ].uiObject;
			}
			break;

		case DRAW_MODE_ROADS:
			if ( iNumRoadsSelected > 0 )
			{
				usUseIndex = SelRoads[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelRoads[ iCurBank ].uiObject;
			}
			break;

		case DRAW_MODE_WALLS:
			if ( iNumWallsSelected > 0 )
			{
				usUseIndex = SelSingleWall[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleWall[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_DOORS:
			if ( iNumDoorsSelected > 0 )
			{
				usUseIndex = SelSingleDoor[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleDoor[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_WINDOWS:
			if( iNumWindowsSelected > 0 )
			{
				usUseIndex = SelSingleWindow[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleWindow[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_ROOFS:
			if ( iNumRoofsSelected > 0 )
			{
				usUseIndex = SelSingleRoof[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleRoof[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_NEWROOF:
			if( iNumNewRoofsSelected > 0 )
			{
				usUseIndex = SelSingleNewRoof[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleNewRoof[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_BROKEN_WALLS:
			if ( iNumBrokenWallsSelected > 0 )
			{
				usUseIndex = SelSingleBrokenWall[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleBrokenWall[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_DECOR:
			if ( iNumDecorSelected > 0 )
			{
				usUseIndex = SelSingleDecor[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleDecor[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_DECALS:
			if ( iNumDecalsSelected > 0 )
			{
				usUseIndex = SelSingleDecal[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleDecal[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_FLOORS:
			if ( iNumFloorsSelected > 0 )
			{
				usUseIndex = SelSingleFloor[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleFloor[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_TOILET:
			if( iNumToiletsSelected > 0 )
			{
				usUseIndex = SelSingleToilet[ iCurBank ].usIndex;
				usObjIndex = (UINT16)SelSingleToilet[ iCurBank ].uiObject;
			}
			break;
		case DRAW_MODE_SMART_WALLS:
		{
			if (guiCurrentCursorGridNo == NOWHERE ||
					!CalcWallInfoUsingSmartMethod(guiCurrentCursorGridNo, &usObjIndex, &usUseIndex))
			{
				CalcSmartWallDefault(&usObjIndex, &usUseIndex);
			}
			break;
		}

		case DRAW_MODE_SMART_DOORS:
		{
			if (guiCurrentCursorGridNo == NOWHERE ||
					!CalcDoorInfoUsingSmartMethod(guiCurrentCursorGridNo, &usObjIndex, &usUseIndex))
			{
				CalcSmartDoorDefault(&usObjIndex, &usUseIndex);
			}
			break;
		}

		case DRAW_MODE_SMART_WINDOWS:
		{
			if (guiCurrentCursorGridNo == NOWHERE ||
					!CalcWindowInfoUsingSmartMethod(guiCurrentCursorGridNo, &usObjIndex, &usUseIndex))
			{
				CalcSmartWindowDefault(&usObjIndex, &usUseIndex);
			}
			break;
		}

		case DRAW_MODE_SMART_BROKEN_WALLS:
		{
			if (guiCurrentCursorGridNo == NOWHERE ||
					!CalcBrokenWallInfoUsingSmartMethod(guiCurrentCursorGridNo, &usObjIndex, &usUseIndex))
			{
				CalcSmartBrokenWallDefault(&usObjIndex, &usUseIndex);
			}
			break;
		}

		case DRAW_MODE_PLACE_ITEM:
			DisplayItemStatistics();
			break;
	}

	// If we actually have something to draw, draw it
	if ( (usUseIndex != 0xffff) && (usObjIndex != 0xffff) )
	{
		HVOBJECT    const  ts = TileElemFromTileType(usObjIndex)->hTileSurface;
		ETRLEObject const& e  = ts->SubregionProperties(usUseIndex);

		// Center the picture in the display window.
		INT32 const iStartX = (w - e.usWidth)  / 2 - e.sOffsetX;
		INT32 const iStartY = (h - e.usHeight) / 2 - e.sOffsetY;

		ts->CurrentShade(DEFAULT_SHADE_LEVEL);
		BltVideoObject(FRAME_BUFFER, ts, usUseIndex, x + iStartX, y + iStartY);
	}

	// Set the color for the window's border. Blueish color = Normal, Red = Fake lighting is turned on
	UINT16 usFillColor = GetGenericButtonFillColor();

	{ SGPVSurface::Lock l(FRAME_BUFFER);
		RectangleDraw(FALSE, x, y, x + w - 1, y + h, usFillColor, l.Buffer<UINT16>());
	}

	InvalidateRegion(x, y, x + w, y + h);
	SetClippingRect(ClipRect);
}


//	Select action to be taken based on user's toolbar selection.
static void HandleJA2ToolbarSelection(void)
{
	BOOLEAN fPrevState;

	fPrevState = gfRenderTaskbar;
	gfRenderTaskbar = TRUE;

	switch (iEditorToolbarState)
	{

		case TBAR_MODE_SET_BGRND:
			iCurrentAction = ACTION_SET_NEW_BACKGROUND;
			break;

		case TBAR_MODE_DENS_DWN:
			iCurrentAction = ACTION_DENSITY_DOWN;
			break;

		case TBAR_MODE_DENS_UP:
			iCurrentAction = ACTION_DENSITY_UP;
			break;

		case TBAR_MODE_RAISE_LAND:
			iCurrentAction = ACTION_RAISE_LAND;
			break;

		case TBAR_MODE_LOWER_LAND:
			iCurrentAction = ACTION_LOWER_LAND;
			break;

		case TBAR_MODE_FAKE_LIGHTING:
			gfFakeLights ^= TRUE;
			if ( gfFakeLights )
			{
				gusSavedLightLevel = gusLightLevel;
				gusLightLevel = EDITOR_LIGHT_FAKE;
			}
			else
				gusLightLevel = gusSavedLightLevel;
			LightSetBaseLevel( (UINT8)(EDITOR_LIGHT_MAX - gusLightLevel) );
			LightSpriteRenderAll();
			break;

		case TBAR_MODE_CHANGE_TILESET:
			InitPopupMenu( iEditorButton[OPTIONS_CHANGE_TILESET], CHANGETSET_POPUP, DIR_UPRIGHT );
			break;

		case TBAR_MODE_CIVILIAN_GROUP:
			InitPopupMenu( iEditorButton[MERCS_CIVILIAN_GROUP], CHANGECIVGROUP_POPUP, DIR_UPLEFT );
			break;

		case TBAR_MODE_GET_WALL:
			iCurrentAction = ACTION_GET_WALL;
			iDrawMode = DRAW_MODE_WALLS;
			break;
		case TBAR_MODE_GET_DOOR:
			iCurrentAction = ACTION_GET_DOOR;
			iDrawMode = DRAW_MODE_DOORS;
			break;
		case TBAR_MODE_GET_WINDOW:
			iCurrentAction = ACTION_GET_WINDOW;
			iDrawMode = DRAW_MODE_WINDOWS;
			break;
		case TBAR_MODE_GET_ROOF:
			iCurrentAction = ACTION_GET_ROOF;
			iDrawMode = DRAW_MODE_ROOFS;
			break;
		case TBAR_MODE_GET_BROKEN_WALL:
			iCurrentAction = ACTION_GET_BROKEN_WALL;
			iDrawMode = DRAW_MODE_BROKEN_WALLS;
			break;
		case TBAR_MODE_GET_DECOR:
			iCurrentAction = ACTION_GET_DECOR;
			iDrawMode = DRAW_MODE_DECOR;
			break;
		case TBAR_MODE_GET_DECAL:
			iCurrentAction = ACTION_GET_DECAL;
			iDrawMode = DRAW_MODE_DECALS;
			break;
		case TBAR_MODE_GET_FLOOR:
			iCurrentAction = ACTION_GET_FLOOR;
			iDrawMode = DRAW_MODE_FLOORS;
			break;
		case TBAR_MODE_GET_TOILET:
			iCurrentAction = ACTION_GET_TOILET;
			iDrawMode = DRAW_MODE_TOILET;
			break;
		case TBAR_MODE_GET_ROOM:
			iCurrentAction = ACTION_GET_ROOM;
			break;
		case TBAR_MODE_GET_NEW_ROOF:
			iCurrentAction = ACTION_GET_NEW_ROOF;
			iDrawMode = DRAW_MODE_NEWROOF;
			break;

		case TBAR_MODE_FILL_AREA:
			iDrawMode = DRAW_MODE_GROUND + DRAW_MODE_FILL_AREA;
			ClickEditorButton( TERRAIN_FGROUND_TEXTURES );
			TerrainTileDrawMode = TERRAIN_TILES_FOREGROUND;
			break;

		case TBAR_MODE_FILL_AREA_OFF:
			iDrawMode = DRAW_MODE_GROUND;
			ClickEditorButton( TERRAIN_FGROUND_TEXTURES );
			TerrainTileDrawMode = TERRAIN_TILES_FOREGROUND;
			break;

		case TBAR_MODE_GET_BANKS:
			iCurrentAction = ACTION_NEXT_BANK;
			iDrawMode = DRAW_MODE_BANKS;
			break;

		case TBAR_MODE_GET_ROADS:
			iCurrentAction = ACTION_NEXT_ROAD;
			iDrawMode = DRAW_MODE_ROADS;
			break;

		case TBAR_MODE_DRAW_BANKS:
			iCurrentAction = ACTION_NULL;
			iDrawMode = DRAW_MODE_BANKS;
			break;

		case TBAR_MODE_GET_DEBRIS:
			iCurrentAction = ACTION_NEXT_DEBRIS;
			iDrawMode = DRAW_MODE_DEBRIS;
			break;

		case TBAR_MODE_DRAW_DEBRIS:
			iCurrentAction = ACTION_NULL;
			iDrawMode = DRAW_MODE_DEBRIS;
			break;

		case TBAR_MODE_GET_OSTRUCTS:
			iCurrentAction = ACTION_NEXT_STRUCT;
			iDrawMode = DRAW_MODE_OSTRUCTS;
			break;

		case TBAR_MODE_DRAW_OSTRUCTS:
			iCurrentAction = ACTION_NULL;
			iDrawMode = DRAW_MODE_OSTRUCTS;
			break;

		case TBAR_MODE_GET_OSTRUCTS1:
			iCurrentAction = ACTION_NEXT_STRUCT1;
			iDrawMode = DRAW_MODE_OSTRUCTS1;
			break;

		case TBAR_MODE_DRAW_OSTRUCTS1:
			iCurrentAction = ACTION_NULL;
			iDrawMode = DRAW_MODE_OSTRUCTS1;
			break;

		case TBAR_MODE_GET_OSTRUCTS2:
			iCurrentAction = ACTION_NEXT_STRUCT2;
			iDrawMode = DRAW_MODE_OSTRUCTS2;
			break;

		case TBAR_MODE_DRAW_OSTRUCTS2:
			iCurrentAction = ACTION_NULL;
			iDrawMode = DRAW_MODE_OSTRUCTS2;
			break;

		case TBAR_MODE_EXIT_EDIT:
			iCurrentAction = ACTION_EXIT_EDITOR;
			break;

		case TBAR_MODE_QUIT_GAME:
			iCurrentAction = ACTION_QUIT_GAME;
			break;

		case TBAR_MODE_SAVE:
			iCurrentAction = ACTION_SAVE_MAP;
			break;

		case TBAR_MODE_LOAD:
			iCurrentAction = ACTION_LOAD_MAP;
			break;

		case TBAR_MODE_UNDO:
			iCurrentAction = ACTION_UNDO;
			break;

		case TBAR_MODE_CHANGE_BRUSH:
			iCurrentAction = ACTION_NEXT_SELECTIONTYPE;
			break;

		case TBAR_MODE_ERASE:
			switch ( iDrawMode )
			{
				case DRAW_MODE_EXITGRID:
				case DRAW_MODE_LIGHT:
				case DRAW_MODE_GROUND:
				case DRAW_MODE_OSTRUCTS:
				case DRAW_MODE_OSTRUCTS1:
				case DRAW_MODE_OSTRUCTS2:
				case DRAW_MODE_DEBRIS:
				case DRAW_MODE_BANKS:
				case DRAW_MODE_ROADS:

				case DRAW_MODE_WALLS:
				case DRAW_MODE_DOORS:
				case DRAW_MODE_WINDOWS:
				case DRAW_MODE_ROOFS:
				case DRAW_MODE_BROKEN_WALLS:
				case DRAW_MODE_DECALS:
				case DRAW_MODE_DECOR:
				case DRAW_MODE_FLOORS:
				case DRAW_MODE_TOILET:

				case DRAW_MODE_ROOM:
				case DRAW_MODE_SLANTED_ROOF:
				case DRAW_MODE_ROOMNUM:
					iDrawMode += DRAW_MODE_ERASE;
					break;
			}
			break;

		case TBAR_MODE_ERASE_OFF:
			if ( iDrawMode >= DRAW_MODE_ERASE )
				iDrawMode -= DRAW_MODE_ERASE;
			break;

		case TBAR_MODE_NEW_MAP:
			iCurrentAction = ACTION_NEW_MAP;
			break;

		case TBAR_MODE_ITEM_WEAPONS:
		case TBAR_MODE_ITEM_AMMO:
		case TBAR_MODE_ITEM_ARMOUR:
		case TBAR_MODE_ITEM_EXPLOSIVES:
		case TBAR_MODE_ITEM_EQUIPMENT1:
		case TBAR_MODE_ITEM_EQUIPMENT2:
		case TBAR_MODE_ITEM_EQUIPMENT3:
		case TBAR_MODE_ITEM_TRIGGERS:
		case TBAR_MODE_ITEM_KEYS:
			//Set up the items by type.
			InitEditorItemsInfo(iEditorToolbarState);
			if( gubCurrMercMode != MERC_GETITEMMODE )
				iDrawMode = DRAW_MODE_PLACE_ITEM;
			break;
		case TBAR_MODE_NONE:
			iCurrentAction = ACTION_NULL;
			gfRenderTaskbar = fPrevState;
			break;
		default:
			iCurrentAction = ACTION_NULL;
			iDrawMode = DRAW_MODE_NOTHING;
			gfRenderTaskbar = fPrevState;
			break;
	}
	iEditorToolbarState = TBAR_MODE_NONE;
}


extern INT8 gbCurrSelect;


static BOOLEAN CheckForSlantRoofs(void);
static void CreateGotoGridNoUI(void);
static void ReloadMap(void);
static void RemoveGotoGridNoUI(void);


// Select action to be taken based on the user's current key press (if any)
static void HandleKeyboardShortcuts(void)
{
	static BOOLEAN fShowTrees = TRUE;
	while( DequeueSpecificEvent(&EditorInputEvent, KEYBOARD_EVENTS) )
	{
		if( !HandleSummaryInput( &EditorInputEvent ) && !HandleTextInput( &EditorInputEvent ) && EditorInputEvent.usEvent == KEY_DOWN )
		{
			if( gfGotoGridNoUI )
			{
				switch( EditorInputEvent.usParam )
				{
					case SDLK_ESCAPE:
						SetInputFieldString( 0, ST::null );
						RemoveGotoGridNoUI();
						break;

					case SDLK_RETURN: RemoveGotoGridNoUI(); break;

					case 'x':
						if( EditorInputEvent.usKeyState & ALT_DOWN )
						{
							SetInputFieldString( 0, ST::null );
							RemoveGotoGridNoUI();
							iCurrentAction = ACTION_QUIT_GAME;
						}
						break;
				}
			}
			else switch( EditorInputEvent.usParam )
			{
				case SDLK_HOME:
					gfFakeLights ^= TRUE;
					if ( gfFakeLights )
					{
						gusSavedLightLevel = gusLightLevel;
						gusLightLevel = EDITOR_LIGHT_FAKE;
						ClickEditorButton( MAPINFO_TOGGLE_FAKE_LIGHTS );
					}
					else
					{
						gusLightLevel = gusSavedLightLevel;
						UnclickEditorButton( MAPINFO_TOGGLE_FAKE_LIGHTS );
					}
					LightSetBaseLevel( (UINT8)(EDITOR_LIGHT_MAX - gusLightLevel) );
					LightSpriteRenderAll();
					break;

				case SDLK_SPACE:
					if( iCurrentTaskbar == TASK_MERCS )
						IndicateSelectedMerc( SELECT_NEXT_MERC );
					else if( iCurrentTaskbar == TASK_ITEMS )
						SelectNextItemPool();
					else if( gfShowTerrainTileButtons && fUseTerrainWeights )
						ResetTerrainTileWeights();
					else
						LightSpriteRenderAll();
					break;

				case SDLK_INSERT:
					if ( iDrawMode == DRAW_MODE_GROUND )
					{
						iDrawMode += DRAW_MODE_FILL_AREA;
						ClickEditorButton( TERRAIN_FILL_AREA );
						iEditorToolbarState = TBAR_MODE_FILL_AREA;
					}
					else if ( iDrawMode == (DRAW_MODE_GROUND + DRAW_MODE_FILL_AREA) )
					{
						iDrawMode -= DRAW_MODE_FILL_AREA;
						UnclickEditorButton( TERRAIN_FILL_AREA );
						iEditorToolbarState = TBAR_MODE_FILL_AREA_OFF;
					}
					break;

				case SDLK_RETURN:
					if( gfEditingDoor )
					{
						ExtractAndUpdateDoorInfo();
						KillDoorEditing();
					}
					else if (iCurrentTaskbar == TASK_MERCS && g_selected_merc != NULL)
						ExtractCurrentMercModeInfo( FALSE );
					else if( iCurrentTaskbar == TASK_MAPINFO )
						ExtractAndUpdateMapInfo();
					else if( iCurrentTaskbar == TASK_BUILDINGS )
						ExtractAndUpdateBuildingInfo();
					else if( gfShowItemStatsPanel && EditingText() )
						ExecuteItemStatsCmd( ITEMSTATS_APPLY );
					break;

				case SDLK_BACKSPACE: iCurrentAction = ACTION_UNDO; break;

				case SDLK_DELETE:
					if( iCurrentTaskbar == TASK_ITEMS )
						DeleteSelectedItem();
					else if (g_selected_merc != NULL)
					{
						if( gubCurrMercMode == MERC_INVENTORYMODE && gbCurrSelect != -1 )
						{
							DeleteSelectedMercsItem();
							gbCurrSelect = -1;
							gfRenderTaskbar = TRUE;
						}
						else
							DeleteSelectedMerc();
					}
					else
						iCurrentAction = ACTION_QUICK_ERASE;
					break;

				case SDLK_ESCAPE:
					if ( InOverheadMap() )
					{
						KillOverheadMap();
					}
					if( iDrawMode == DRAW_MODE_SCHEDULEACTION )
					{
						CancelCurrentScheduleAction();
					}
					else if( gfMercGetItem )
					{ //cancel getting an item for the merc.
						gfMercGetItem = FALSE;
						gusMercsNewItemIndex = 0xffff;
						SetMercEditingMode( MERC_INVENTORYMODE );
						ClearEditorItemsInfo();
					}
					else if( gfShowItemStatsPanel && EditingText() )
						ExecuteItemStatsCmd( ITEMSTATS_CANCEL );
					else if( gfEditingDoor )
						KillDoorEditing();
					else
						iCurrentAction = ACTION_EXIT_EDITOR;
					break;

				//Select next/prev terrain tile to draw with.
				case SDLK_LEFT:
					if (!(EditorInputEvent.usKeyState & SHIFT_DOWN)) break;
					CurrentPaste -= ( gfShowTerrainTileButtons && CurrentPaste > 0 ) ? 1 : 0;
					break;

				case SDLK_RIGHT:
					if (!(EditorInputEvent.usKeyState & SHIFT_DOWN)) break;
					CurrentPaste += ( gfShowTerrainTileButtons && CurrentPaste < 8 ) ? 1 : 0;
					break;

				case SDLK_PAGEUP:
					if( iCurrentTaskbar == TASK_MERCS && !fBuildingShowRoofs )
					{
						gfRoofPlacement = TRUE;
						fBuildingShowRoofs = TRUE;
						UpdateRoofsView();
						gfRenderWorld = TRUE;
					}
					else switch( iDrawMode )
					{
						case DRAW_MODE_SMART_WALLS:					DecSmartWallUIValue();									break;
						case DRAW_MODE_SMART_DOORS:					DecSmartDoorUIValue();									break;
						case DRAW_MODE_SMART_WINDOWS:				DecSmartWindowUIValue();								break;
						case DRAW_MODE_SMART_BROKEN_WALLS:	DecSmartBrokenWallUIValue();						break;
						case DRAW_MODE_PLACE_ITEM:					SelectPrevItemInPool();									break;
						default:														iCurrentAction = ACTION_SUB_INDEX_UP;		break;
					}
					gfRenderDrawingMode = TRUE;
					break;

				case SDLK_PAGEDOWN:
					if( iCurrentTaskbar == TASK_MERCS && fBuildingShowRoofs )
					{
						gfRoofPlacement = FALSE;
						fBuildingShowRoofs = FALSE;
						UpdateRoofsView();
						gfRenderWorld = TRUE;
					}
					else switch( iDrawMode )
					{
						case DRAW_MODE_SMART_WALLS:					IncSmartWallUIValue();									break;
						case DRAW_MODE_SMART_DOORS:					IncSmartDoorUIValue();									break;
						case DRAW_MODE_SMART_WINDOWS:				IncSmartWindowUIValue();								break;
						case DRAW_MODE_SMART_BROKEN_WALLS:	IncSmartBrokenWallUIValue();						break;
						case DRAW_MODE_PLACE_ITEM:					SelectNextItemInPool();									break;
						default:														iCurrentAction = ACTION_SUB_INDEX_DWN;	break;
					}
					gfRenderDrawingMode = TRUE;
					break;

				case SDLK_F1:
					gfRenderWorld = TRUE;
					gfRenderTaskbar = TRUE;
					break;

				case SDLK_F2:
					if (EditorInputEvent.usKeyState & ALT_DOWN )
					{
						ReloadMap();
					}
					break;

				case SDLK_F3:
					if( EditorInputEvent.usKeyState & CTRL_DOWN )
					{
						ReplaceObsoleteRoads();
						MarkWorldDirty();
					}
					break;

				case SDLK_F4:
					MusicPlay( GCM->getMusicForMode(giMusicMode) );
					SLOGD("Testing music {}", GCM->getMusicForMode(giMusicMode));
					giMusicMode = (MusicMode)(giMusicMode + 1);
					if( giMusicMode >= MAX_MUSIC_MODES )
						giMusicMode = MUSIC_MAIN_MENU;
					break;

				case SDLK_F5:
					UpdateLastActionBeforeLeaving();
					CreateSummaryWindow();
					break;

				case SDLK_F6:
					break;

				case SDLK_F7:
					if( gfBasement )
					{
						INT32 i;
						UINT16 usRoofIndex, usRoofType;
						pSelList = SelSingleRoof;
						pNumSelList = &iNumRoofsSelected;
						usRoofType = GetRandomIndexByRange( FIRSTROOF, LASTROOF );
						if( usRoofType == 0xffff )
							usRoofType = FIRSTROOF;
						for( i = 0; i < WORLD_MAX; i++ )
						{
							if( gubWorldRoomInfo[ i ] )
							{
								AddToUndoList( i );
								RemoveAllRoofsOfTypeRange( i, FIRSTTEXTURE, LASTITEM );
								RemoveAllOnRoofsOfTypeRange( i, FIRSTTEXTURE, LASTITEM );
								RemoveAllShadowsOfTypeRange( i, FIRSTROOF, LASTSLANTROOF );
								usRoofIndex = 9 + Random(3);
								UINT16 usTileIndex = GetTileIndexFromTypeSubIndex(usRoofType, usRoofIndex);
								AddRoofToHead( i, usTileIndex );
							}
						}
						MarkWorldDirty();
					}
					break;

				case SDLK_F8: SmoothAllTerrainWorld(); break;

				case SDLK_F9: break;

				case SDLK_F10:
					CreateMessageBox( "Are you sure you wish to remove all lights?" );
					gfRemoveLightsPending = TRUE;
					break;

				case SDLK_F11:
					CreateMessageBox( "Are you sure you wish to reverse the schedules?" );
					gfScheduleReversalPending = TRUE;
					break;

				case SDLK_F12:
					CreateMessageBox( "Are you sure you wish to clear all of the schedules?" );
					gfScheduleClearPending = TRUE;
					break;

				case '[':
					iCurrentAction = ACTION_DENSITY_DOWN;
					break;

				case ']':
					iCurrentAction = ACTION_DENSITY_UP;
					break;

				case '+':
					//if ( iDrawMode == DRAW_MODE_SHOW_TILESET )
					//	iCurrentAction = ACTION_MLIST_DWN;
					//else
						iCurrentAction = ACTION_SHADE_UP;
					break;

				case '-':
					//if ( iDrawMode == DRAW_MODE_SHOW_TILESET )
					//	iCurrentAction = ACTION_MLIST_UP;
					iCurrentAction = ACTION_SHADE_DWN;
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
						if ( iCurrentTaskbar == TASK_MERCS )
						{
							if ( iDrawMode >= DRAW_MODE_ERASE )
								iCurrentAction = ACTION_ERASE_WAYPOINT;
							else
								iCurrentAction = ACTION_SET_WAYPOINT;
							iActionParam = EditorInputEvent.usParam - '0';
						}
						else
						{
							iCurrentAction = ACTION_SET_FNAME;
							iActionParam = EditorInputEvent.usParam - '0';
						}
					break;

				case 'a':
					iCurrentAction = ACTION_PREV_SELECTIONTYPE;
					break;
				case 'b':
					if ( iCurrentTaskbar != TASK_BUILDINGS )
					{
						iTaskMode = TASK_BUILDINGS;
						DoTaskbar();
					}

					if( CheckForSlantRoofs() )
						iDrawMode = DRAW_MODE_SLANTED_ROOF;
					else
					{
						gusSelectionType = gusSavedBuildingSelectionType;
						iDrawMode = DRAW_MODE_ROOM;
					}
					if( gfCaves )
						SetEditorBuildingTaskbarMode( BUILDING_CAVE_DRAWING );
					else
						SetEditorBuildingTaskbarMode( BUILDING_NEW_ROOM );
					TerrainTileDrawMode = TERRAIN_TILES_BRETS_STRANGEMODE;
					break;
				case 'c':
					if( EditorInputEvent.usKeyState & CTRL_DOWN && iCurrentTaskbar == TASK_MERCS )
					{
						iCurrentAction = ACTION_COPY_MERC_PLACEMENT;
					}
					break;

				case 'd': // debris
					if( iCurrentTaskbar != TASK_TERRAIN )
					{
						iTaskMode = TASK_TERRAIN;
						DoTaskbar();
					}

					iCurrentAction = ACTION_NULL;
					iDrawMode = DRAW_MODE_DEBRIS;
					ClickEditorButton( TERRAIN_PLACE_DEBRIS );
					iEditorToolbarState = TBAR_MODE_DRAW_DEBRIS;
					TerrainTileDrawMode = TERRAIN_TILES_NODRAW;
					break;
				case 'e':
					if ( iDrawMode >= DRAW_MODE_ERASE )
					{
						iDrawMode -= DRAW_MODE_ERASE;
					}
					else
					{
						switch ( iDrawMode )
						{
							case DRAW_MODE_NORTHPOINT:
							case DRAW_MODE_WESTPOINT:
							case DRAW_MODE_EASTPOINT:
							case DRAW_MODE_SOUTHPOINT:
							case DRAW_MODE_CENTERPOINT:
							case DRAW_MODE_ISOLATEDPOINT:
							case DRAW_MODE_EXITGRID:
							case DRAW_MODE_LIGHT:
							case DRAW_MODE_GROUND:
							case DRAW_MODE_OSTRUCTS:
							case DRAW_MODE_OSTRUCTS1:
							case DRAW_MODE_OSTRUCTS2:
							case DRAW_MODE_DEBRIS:
							case DRAW_MODE_BANKS:
							case DRAW_MODE_ROADS:
							case DRAW_MODE_WALLS:
							case DRAW_MODE_DOORS:
							case DRAW_MODE_WINDOWS:
							case DRAW_MODE_ROOFS:
							case DRAW_MODE_BROKEN_WALLS:
							case DRAW_MODE_DECOR:
							case DRAW_MODE_DECALS:
							case DRAW_MODE_FLOORS:
							case DRAW_MODE_TOILET:
							case DRAW_MODE_ROOM:
							case DRAW_MODE_SLANTED_ROOF:
							case DRAW_MODE_ROOMNUM:
								iDrawMode += DRAW_MODE_ERASE;
								break;
						}
					}
					break;
				case 'f':
					gbFPSDisplay = !gbFPSDisplay;
					EnableFPSOverlay(gbFPSDisplay);
					break;
				case 'g':	// ground
					if( EditorInputEvent.usKeyState & CTRL_DOWN )
					{
						CreateGotoGridNoUI();
					}
					else
					{
						if ( iCurrentTaskbar != TASK_TERRAIN )
						{
							iTaskMode = TASK_TERRAIN;
							DoTaskbar();
						}
						iCurrentAction = ACTION_NULL;
						SetEditorTerrainTaskbarMode( TERRAIN_FGROUND_TEXTURES );
					}
					break;

				case 'h':
					if( fBuildingShowRoofs ^= 1 )
						ClickEditorButton( BUILDING_TOGGLE_ROOF_VIEW );
					else
						UnclickEditorButton( BUILDING_TOGGLE_ROOF_VIEW );
					UpdateRoofsView();
					break;

				case 'i':
					if( !InOverheadMap() )
						GoIntoOverheadMap();
					else
						KillOverheadMap();
					break;

				case 'l':
					if (EditorInputEvent.usKeyState & CTRL_DOWN )
					{
						UpdateLastActionBeforeLeaving();
						iCurrentAction = ACTION_LOAD_MAP;
						break;
					}
					if ( iCurrentTaskbar != TASK_TERRAIN )
					{
						iTaskMode = TASK_TERRAIN;
						DoTaskbar();
					}
					break;
				case 'n':
					if( fBuildingShowRoomInfo ^= 1 )
					{
						SetRenderFlags( RENDER_FLAG_ROOMIDS );
						ClickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
					}
					else
					{
						ClearRenderFlags( RENDER_FLAG_ROOMIDS );
						UnclickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
					}
					break;
				case 'o':
					if ( iCurrentTaskbar != TASK_TERRAIN )
					{
						iTaskMode = TASK_TERRAIN;
						DoTaskbar();
					}
					iCurrentAction = ACTION_NULL;
					iDrawMode = DRAW_MODE_OSTRUCTS2;
					ClickEditorButton( TERRAIN_PLACE_MISC );
					iEditorToolbarState = TBAR_MODE_DRAW_OSTRUCTS2;
					break;
				case 'r': // rocks
					if ( iCurrentTaskbar != TASK_TERRAIN )
					{
						iTaskMode = TASK_TERRAIN;
						DoTaskbar();
					}
					iCurrentAction = ACTION_NULL;
					iDrawMode = DRAW_MODE_OSTRUCTS1;
					ClickEditorButton( TERRAIN_PLACE_ROCKS );
					iEditorToolbarState = TBAR_MODE_DRAW_OSTRUCTS1;
					break;
				case 's':
					if (EditorInputEvent.usKeyState & CTRL_DOWN )
					{
						iCurrentAction = ACTION_SAVE_MAP;
					}
					break;

				case SDLK_t: // Trees
				if (EditorInputEvent.usKeyState & SHIFT_DOWN)
					{
						if (fShowTrees)
						{
							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "Removing Treetops");
							WorldHideTrees();
						}
						else
						{
							ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, "Showing Treetops");
							WorldShowTrees();
						}
						fShowTrees = !fShowTrees;
					}
					else
					{
						if (iCurrentTaskbar != TASK_TERRAIN)
						{
							iTaskMode = TASK_TERRAIN;
							DoTaskbar();
						}
						iCurrentAction = ACTION_NULL;
						iDrawMode = DRAW_MODE_OSTRUCTS;
						ClickEditorButton(TERRAIN_PLACE_TREES);
						iEditorToolbarState = TBAR_MODE_DRAW_OSTRUCTS;
					}
					break;

				case 'u':
					RaiseWorldLand();
					break;
				case 'w': // walls (full building)
					if( fBuildingShowWalls ^= 1 )
						ClickEditorButton( BUILDING_TOGGLE_WALL_VIEW );
					else
						UnclickEditorButton( BUILDING_TOGGLE_WALL_VIEW );
					UpdateWallsView();
					break;
				case 'v':
					if( EditorInputEvent.usKeyState & CTRL_DOWN && iCurrentTaskbar == TASK_MERCS )
					{
						iCurrentAction = ACTION_PASTE_MERC_PLACEMENT;
					}
					break;

				case 'x':
					if (EditorInputEvent.usKeyState & ALT_DOWN )
					{
						if( InOverheadMap() )
							KillOverheadMap();
						if( gfEditingDoor )
							KillDoorEditing();
						iCurrentAction = ACTION_QUIT_GAME;
						return;
					}
					break;
				case 'z':
					iCurrentAction = ACTION_NEXT_SELECTIONTYPE;
					break;
				case ',':
					gusSelectionType = LINESELECTION;
					gusPreserveSelectionWidth--;
					if( !gusPreserveSelectionWidth )
						gusPreserveSelectionWidth = 8;
					gfRenderTaskbar = TRUE;
					break;
				case '.':
					gusSelectionType = LINESELECTION;
					gusPreserveSelectionWidth++;
					if( gusPreserveSelectionWidth > 8 )
						gusPreserveSelectionWidth = 1;
					gfRenderTaskbar = TRUE;
					break;
				default:
					iCurrentAction = ACTION_NULL;
					break;
			}
		}
	}
}


//	Perform the current user selected action, if any (or at least set things up for doing that)
static ScreenID PerformSelectedAction(void)
{
	switch ( iCurrentAction )
	{
		case ACTION_DENSITY_DOWN:
			DecreaseSelectionDensity();
			break;
		case ACTION_DENSITY_UP:
			IncreaseSelectionDensity();
			break;

		case ACTION_ERASE_WAYPOINT:
			EraseMercWaypoint();
			break;

		case ACTION_SET_WAYPOINT:
		{
			AddMercWaypoint(guiCurrentCursorGridNo);
			break;
		}

		case ACTION_RAISE_LAND:
			RaiseWorldLand();
			break;

		case ACTION_HELPSCREEN:
			DisableEditorTaskbar( );
			fHelpScreen = TRUE;
			fAllDone = FALSE;
			break;

		case ACTION_QUICK_ERASE:
			if (guiCurrentCursorGridNo != NOWHERE)
			{
				QuickEraseMapTile(guiCurrentCursorGridNo);
			}
			break;

		case ACTION_QUIT_GAME:
			requestGameExit();
			break;
		case ACTION_EXIT_EDITOR:
			if( EditModeShutdown( ) )
			{
				iPrevJA2ToolbarState = iEditorToolbarState;
				iPrevDrawMode = iDrawMode;
				PrevCurrentPaste = CurrentPaste;
				gPrevCurrentBackground = gCurrentBackground;
				fFirstTimeInGameScreen = TRUE;
				return GAME_SCREEN;
			}
			return EDIT_SCREEN;
		case ACTION_GET_WALL:
		case ACTION_GET_DOOR:
		case ACTION_GET_WINDOW:
		case ACTION_GET_ROOF:
		case ACTION_GET_NEW_ROOF:
		case ACTION_GET_BROKEN_WALL:
		case ACTION_GET_DECOR:
		case ACTION_GET_DECAL:
		case ACTION_GET_FLOOR:
		case ACTION_GET_TOILET:
		case ACTION_GET_ROOM:
			switch( iCurrentAction )
			{
				case ACTION_GET_WALL:				CreateJA2SelectionWindow( SELWIN_SINGLEWALL );			break;
				case ACTION_GET_DOOR:				CreateJA2SelectionWindow( SELWIN_SINGLEDOOR );			break;
				case ACTION_GET_WINDOW:			CreateJA2SelectionWindow( SELWIN_SINGLEWINDOW );		break;
				case ACTION_GET_ROOF:				CreateJA2SelectionWindow( SELWIN_SINGLEROOF );			break;
				case ACTION_GET_NEW_ROOF:		CreateJA2SelectionWindow( SELWIN_SINGLENEWROOF );		break;
				case ACTION_GET_BROKEN_WALL:CreateJA2SelectionWindow( SELWIN_SINGLEBROKENWALL );break;
				case ACTION_GET_DECOR:			CreateJA2SelectionWindow( SELWIN_SINGLEDECOR );			break;
				case ACTION_GET_DECAL:			CreateJA2SelectionWindow( SELWIN_SINGLEDECAL );			break;
				case ACTION_GET_FLOOR:			CreateJA2SelectionWindow( SELWIN_SINGLEFLOOR );			break;
				case ACTION_GET_TOILET:			CreateJA2SelectionWindow( SELWIN_SINGLETOILET );		break;
				case ACTION_GET_ROOM:				CreateJA2SelectionWindow( SELWIN_ROOM );						break;
			}
			fSelectionWindow = TRUE;
			fAllDone = FALSE;
			break;

		case ACTION_NEW_MAP:
			fNewMap = TRUE;
			if( gfPendingBasement )
				CreateMessageBox( "Delete current map and start a new basement level?" );
			else if( gfPendingCaves )
				CreateMessageBox( "Delete current map and start a new cave level?" );
			else
				CreateMessageBox( "Delete current map and start a new outdoor level?" );
			break;

		case ACTION_SET_NEW_BACKGROUND:
			if (!fBeenWarned)
				CreateMessageBox( " Wipe out ground textures? " );
			else
			{
				gCurrentBackground = TerrainBackgroundTile;
				SetBackgroundTexture( );
				fBeenWarned = FALSE;
			}
			break;

		case ACTION_SUB_INDEX_UP:
			gusSelectionType = SMALLSELECTION;
			switch ( iDrawMode )
			{
				case DRAW_MODE_BANKS:
					if ( iNumBanksSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumBanksSelected;
					break;
				case DRAW_MODE_ROADS:
					if ( iNumRoadsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumRoadsSelected;
					break;
				case DRAW_MODE_WALLS:
					if ( iNumWallsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumWallsSelected;
					break;
				case DRAW_MODE_DOORS:
					if ( iNumDoorsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumDoorsSelected;
					break;
				case DRAW_MODE_WINDOWS:
					if ( iNumWindowsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumWindowsSelected;
					break;
				case DRAW_MODE_ROOFS:
					if( iNumRoofsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumRoofsSelected;
					break;
				case DRAW_MODE_BROKEN_WALLS:
					if( iNumBrokenWallsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumBrokenWallsSelected;
					break;
				case DRAW_MODE_DECOR:
					if ( iNumDecorSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumDecorSelected;
					break;
				case DRAW_MODE_DECALS:
					if ( iNumDecalsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumDecalsSelected;
					break;
				case DRAW_MODE_FLOORS:
					if( iNumFloorsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumFloorsSelected;
					break;
				case DRAW_MODE_TOILET:
					if( iNumToiletsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumToiletsSelected;
					break;
				case DRAW_MODE_NEWROOF:
					if( iNumNewRoofsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumNewRoofsSelected;
					break;
				case DRAW_MODE_OSTRUCTS:
					fDontUseRandom = TRUE;
					if ( iNumOStructsSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumOStructsSelected;
					break;

				case DRAW_MODE_OSTRUCTS1:
					fDontUseRandom = TRUE;
					if ( iNumOStructs1Selected > 0 )
						iCurBank = (iCurBank + 1) % iNumOStructs1Selected;
					break;

				case DRAW_MODE_OSTRUCTS2:
					fDontUseRandom = TRUE;
					if ( iNumOStructs2Selected > 0 )
						iCurBank = (iCurBank + 1) % iNumOStructs2Selected;
					break;

				case DRAW_MODE_DEBRIS:
					fDontUseRandom = TRUE;
					if ( iNumDebrisSelected > 0 )
						iCurBank = (iCurBank + 1) % iNumDebrisSelected;
					break;
			}
			break;

		case ACTION_SUB_INDEX_DWN:
			gusSelectionType = SMALLSELECTION;
			switch ( iDrawMode )
			{
				case DRAW_MODE_BANKS:
					if ( iNumBanksSelected > 0 )
						iCurBank = (iCurBank + (iNumBanksSelected - 1)) % iNumBanksSelected;
					break;
				case DRAW_MODE_ROADS:
					if ( iNumRoadsSelected > 0 )
						iCurBank = (iCurBank + (iNumRoadsSelected - 1)) % iNumRoadsSelected;
					break;
				case DRAW_MODE_WALLS:
					if ( iNumWallsSelected > 0 )
						iCurBank = (iCurBank + (iNumWallsSelected - 1)) % iNumWallsSelected;
					break;
				case DRAW_MODE_DOORS:
					if ( iNumDoorsSelected > 0 )
						iCurBank = (iCurBank + (iNumDoorsSelected - 1)) % iNumDoorsSelected;
					break;
				case DRAW_MODE_WINDOWS:
					if ( iNumWindowsSelected > 0 )
						iCurBank = (iCurBank + (iNumWindowsSelected - 1)) % iNumWindowsSelected;
					break;
				case DRAW_MODE_ROOFS:
					if( iNumRoofsSelected > 0 )
						iCurBank = (iCurBank + (iNumRoofsSelected - 1)) % iNumRoofsSelected;
					break;
				case DRAW_MODE_BROKEN_WALLS:
					if ( iNumBrokenWallsSelected > 0 )
						iCurBank = (iCurBank + (iNumBrokenWallsSelected - 1)) % iNumBrokenWallsSelected;
					break;
				case DRAW_MODE_DECOR:
					if ( iNumDecorSelected > 0 )
						iCurBank = (iCurBank + (iNumDecorSelected - 1)) % iNumDecorSelected;
					break;
				case DRAW_MODE_DECALS:
					if ( iNumDecalsSelected > 0 )
						iCurBank = (iCurBank + (iNumDecalsSelected - 1)) % iNumDecalsSelected;
					break;
				case DRAW_MODE_FLOORS:
					if( iNumFloorsSelected > 0 )
						iCurBank = (iCurBank + (iNumFloorsSelected - 1)) % iNumFloorsSelected;
					break;
				case DRAW_MODE_TOILET:
					if( iNumToiletsSelected > 0 )
						iCurBank = (iCurBank + (iNumToiletsSelected - 1)) % iNumToiletsSelected;
					break;
				case DRAW_MODE_NEWROOF:
					if( iNumNewRoofsSelected > 0 )
						iCurBank = (iCurBank + (iNumNewRoofsSelected - 1)) % iNumNewRoofsSelected;
					break;
				case DRAW_MODE_OSTRUCTS:
					fDontUseRandom = TRUE;
					if ( iNumOStructsSelected > 0 )
						iCurBank = (iCurBank + (iNumOStructsSelected - 1)) % iNumOStructsSelected;
					break;

				case DRAW_MODE_OSTRUCTS1:
					fDontUseRandom = TRUE;
					if ( iNumOStructs1Selected > 0 )
						iCurBank = (iCurBank + (iNumOStructs1Selected - 1)) % iNumOStructs1Selected;
					break;

				case DRAW_MODE_OSTRUCTS2:
					fDontUseRandom = TRUE;
					if ( iNumOStructs2Selected > 0 )
						iCurBank = (iCurBank + (iNumOStructs2Selected - 1)) % iNumOStructs2Selected;
					break;

				case DRAW_MODE_DEBRIS:
					fDontUseRandom = TRUE;
					if ( iNumDebrisSelected > 0 )
						iCurBank = (iCurBank + (iNumDebrisSelected - 1)) % iNumDebrisSelected;
					break;
			}
			break;
		case ACTION_NEXT_FGRND:
			break;

		case ACTION_NEXT_DEBRIS:
			CreateJA2SelectionWindow( SELWIN_DEBRIS );
			fSelectionWindow = TRUE;
			fAllDone = FALSE;
			break;

		case ACTION_PREV_SELECTIONTYPE:
			gusSelectionType--;
			if( gusSelectionType > AREASELECTION )
				gusSelectionType = AREASELECTION;
			gfRenderTaskbar = TRUE;
			break;
		case ACTION_NEXT_SELECTIONTYPE:
			gusSelectionType++;
			if( gusSelectionType > AREASELECTION )
				gusSelectionType = SMALLSELECTION;
			gfRenderTaskbar = TRUE;
			break;

		case ACTION_COPY_MERC_PLACEMENT:
		{
			if (guiCurrentCursorGridNo != NOWHERE)
			{
				CopyMercPlacement(guiCurrentCursorGridNo);
			}
			break;
		}

		case ACTION_PASTE_MERC_PLACEMENT:
		{
			if (guiCurrentCursorGridNo != NOWHERE)
			{
				PasteMercPlacement(guiCurrentCursorGridNo);
			}
			break;
		}

		case ACTION_SAVE_MAP:
			UpdateLastActionBeforeLeaving();
			return LOADSAVE_SCREEN;

		case ACTION_LOAD_MAP:
			UpdateLastActionBeforeLeaving();
			return LOADSAVE_SCREEN;

		case ACTION_UNDO:
			ExecuteUndoList( );
			gfRenderWorld = TRUE;
			break;

		case ACTION_NEXT_BANK:
			CreateJA2SelectionWindow( SELWIN_BANKS );
			fSelectionWindow = TRUE;
			fAllDone = FALSE;
			break;

		case ACTION_NEXT_ROAD:
			CreateJA2SelectionWindow( SELWIN_ROADS );
			fSelectionWindow = TRUE;
			fAllDone = FALSE;
			break;

		case ACTION_SHADE_UP:
			if ( EditorInputEvent.usKeyState & SHIFT_DOWN )
			{
				gShadePercent+= (FLOAT).05;
			}
			else
			{
				gShadePercent+= (FLOAT).01;
			}

			if ( gShadePercent > 1 )
			{
				gShadePercent = (FLOAT)0;
			}
			SetShadeTablePercent( gShadePercent );
			break;

		case ACTION_SHADE_DWN:
			if ( EditorInputEvent.usKeyState & SHIFT_DOWN )
			{
				gShadePercent-= (FLOAT).05;
			}
			else
			{
				gShadePercent-= (FLOAT).01;
			}

			if ( gShadePercent < 0 )
			{
				gShadePercent = (FLOAT)1;
			}
			SetShadeTablePercent( gShadePercent );
			break;

		case ACTION_WALL_PASTE1:	// Doors		//** Changes needed
		{
			if (guiCurrentCursorGridNo != NOWHERE)
			{
				AddWallToStructLayer(guiCurrentCursorGridNo, FIRSTWALL18, TRUE);
			}
			break;
		}

		case ACTION_WALL_PASTE2:	// Windows	//** Changes Needed
		{
			if (guiCurrentCursorGridNo != NOWHERE)
			{
				AddWallToStructLayer(guiCurrentCursorGridNo, FIRSTWALL19, TRUE);
			}
			break;
		}

		case ACTION_NEXT_STRUCT:
			CreateJA2SelectionWindow( SELWIN_OSTRUCTS );
			fSelectionWindow = TRUE;
			fAllDone = FALSE;
			break;

		case ACTION_NEXT_STRUCT1:
			CreateJA2SelectionWindow( SELWIN_OSTRUCTS1 );
			fSelectionWindow = TRUE;
			fAllDone = FALSE;
			break;

		case ACTION_NEXT_STRUCT2:
			CreateJA2SelectionWindow( SELWIN_OSTRUCTS2 );
			fSelectionWindow = TRUE;
			fAllDone = FALSE;
			break;

		default:
			break;
	}

	return EDIT_SCREEN;
}


static void CreateNewMap(void)
{
	if( gfSummaryWindowActive )
		DestroySummaryWindow();

	if( !gfWorldLoaded )
		LoadMapTileset(GENERIC_1);

	LightReset();
	NewWorld( );
	if( gfPendingBasement )
	{
		UINT32 i;
		for ( i = 0; i < WORLD_MAX; i++ )
		{
			// Set land index 9 + 3 variants
			UINT16 usIndex = GetTileIndexFromTypeSubIndex(FIRSTROOF, 9 + Random(3));
			AddRoofToHead( i, usIndex );
		}
		SetEditorSmoothingMode( SMOOTHING_BASEMENT );
	}
	else if( gfPendingCaves )
	{
		UINT32 i;
		for ( i = 0; i < WORLD_MAX; i++ )
		{
			//Set up the default cave here.
			UINT16 usIndex = GetTileIndexFromTypeSubIndex(FIRSTWALL, 60 + Random(6));
			AddCave( i, usIndex );
		}
		SetEditorSmoothingMode( SMOOTHING_CAVES );
	}
	else
		SetEditorSmoothingMode( SMOOTHING_NORMAL );
	fNewMap = FALSE;
	RemoveAllFromUndoList( );
	UseEditorAlternateList();
	KillSoldierInitList();
	UseEditorOriginalList();
	KillSoldierInitList();
	HideEntryPoints();
	gMapInformation.sNorthGridNo = -1;
	gMapInformation.sSouthGridNo = -1;
	gMapInformation.sWestGridNo = -1;
	gMapInformation.sEastGridNo = -1;
}


static ScreenID ProcessEditscreenMessageBoxResponse(void)
{
	RemoveMessageBox();
	gfRenderWorld = TRUE;
	if( gfConfirmExitPending )
	{
		gfConfirmExitPending = FALSE;
		if( gfMessageBoxResult )
		{
			gfConfirmExitFirst = FALSE;
			iEditorToolbarState = TBAR_MODE_EXIT_EDIT;
		}
		return EDIT_SCREEN;
	}
	if( !gfMessageBoxResult )
		return EDIT_SCREEN;
	if( gfRemoveLightsPending )
	{
		INT32 i;
		LightReset();
		for( i = 0; i < WORLD_MAX; i++ )
		{
			RemoveAllObjectsOfTypeRange( i, GOODRING, GOODRING );
		}
		MarkWorldDirty();
		LightSetBaseLevel( (UINT8)(15 - ubAmbientLightLevel) );
		gfRemoveLightsPending = FALSE;
	}
	if( gfScheduleReversalPending )
	{
		IndicateSelectedMerc( SELECT_NO_MERC );
		ReverseSchedules();
		gfScheduleReversalPending = FALSE;
	}
	else if( gfScheduleClearPending )
	{
		IndicateSelectedMerc( SELECT_NO_MERC );
		ClearAllSchedules();
		gfScheduleClearPending = FALSE;
	}
	if( fNewMap )
	{
		CreateNewMap();
		return EDIT_SCREEN;
	}
	else if( iDrawMode == DRAW_MODE_NEW_GROUND )
	{
		gCurrentBackground = TerrainBackgroundTile;
		SetBackgroundTexture( );
		SetEditorTerrainTaskbarMode( TERRAIN_FGROUND_TEXTURES );
		return EDIT_SCREEN;
	}
	return EDIT_SCREEN;
}


//	Displays a help screen and waits for the user to wisk it away.
static ScreenID WaitForHelpScreenResponse(void)
{
	InputAtom DummyEvent;
	BOOLEAN fLeaveScreen;

	ColorFillVideoSurfaceArea(FRAME_BUFFER,	50, 50, 590, 310,
													Get16BPPColor(FROMRGB(136, 138, 135)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER,	51, 51, 590, 310,
													Get16BPPColor(FROMRGB(24, 61, 81)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER,	51, 51, 589, 309, GetGenericButtonFillColor());

	SetFont( gp12PointFont1 );

	GPrint( 55,  55, "HOME" );
	GPrint( 205, 55, "Toggle fake editor lighting ON/OFF" );

	GPrint( 55,  67, "INSERT" );
	GPrint( 205, 67, "Toggle fill mode ON/OFF" );

	GPrint( 55,  79, "BKSPC" );
	GPrint( 205, 79, "Undo last change" );

	GPrint( 55,  91, "DEL" );
	GPrint( 205, 91, "Quick erase object under mouse cursor" );

	GPrint( 55,  103, "ESC" );
	GPrint( 205, 103, "Exit editor" );

	GPrint( 55,  115, "PGUP/PGDN" );
	GPrint( 205, 115, "Change object to be pasted" );

	GPrint( 55,  127, "F1" );
	GPrint( 205, 127, "This help screen" );

	GPrint( 55,  139, "F10" );
	GPrint( 205, 139, "Save current map" );

	GPrint( 55,  151, "F11" );
	GPrint( 205, 151, "Load map as current" );

	GPrint( 55,  163, "+/-" );
	GPrint( 205, 163, "Change shadow darkness by .01" );

	GPrint( 55,  175, "SHFT +/-" );
	GPrint( 205, 175, "Change shadow darkness by .05" );

	GPrint( 55,  187, "0 - 9" );
	GPrint( 205, 187, "Change map/tileset filename" );

	GPrint( 55,  199, "b" );
	GPrint( 205, 199, "Change brush size" );

	GPrint( 55,  211, "d" );
	GPrint( 205, 211, "Draw debris" );

	GPrint( 55,  223, "o" );
	GPrint( 205, 223, "Draw obstacle" );

	GPrint( 55,  235, "r" );
	GPrint( 205, 235, "Draw rocks" );

	GPrint( 55,  247, "t" );
	GPrint( 205, 247, "Toggle trees display ON/OFF" );

	GPrint( 55,  259, "g" );
	GPrint( 205, 259, "Draw ground textures" );

	GPrint( 55,  271, "w" );
	GPrint( 205, 271, "Draw building walls" );

	GPrint( 55,  283, "e" );
	GPrint( 205, 283, "Toggle erase mode ON/OFF" );

	GPrint( 55,  295, "h" );
	GPrint( 205, 295, "Toggle roofs ON/OFF" );


	fLeaveScreen = FALSE;

	while (DequeueSpecificEvent(&DummyEvent, KEYBOARD_EVENTS))
	{
		if ( DummyEvent.usEvent == KEY_DOWN )
		{
			switch( DummyEvent.usParam )
			{
				case SDLK_SPACE:
				case SDLK_ESCAPE:
				case SDLK_RETURN:
				case SDLK_F1:
					fLeaveScreen = TRUE;
					break;
			}
		}
	}


	if ( (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) || (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) || fLeaveScreen )
	{
		fHelpScreen = FALSE;

		while( DequeueEvent( &DummyEvent ) )
			continue;

		EnableEditorTaskbar();
	}

	InvalidateScreen( );
	ExecuteBaseDirtyRectQueue();

	return( EDIT_SCREEN );
}


//	Handles all keyboard input and display for a selection window.
static ScreenID WaitForSelectionWindowResponse(void)
{
	InputAtom DummyEvent;

	while (DequeueSpecificEvent(&DummyEvent, KEYBOARD_EVENTS))
	{
		if ( DummyEvent.usEvent == KEY_DOWN )
		{
			switch( DummyEvent.usParam )
			{
				case SDLK_SPACE: ClearSelectionList(); break;
				case SDLK_DOWN:  ScrollSelWinDown();   break;
				case SDLK_UP:    ScrollSelWinUp();     break;

				case SDLK_ESCAPE:
					RestoreSelectionList();
					// fallthrough
				case SDLK_RETURN:
					fAllDone = TRUE;
					break;
			}
		}
	}

	if ( DoWindowSelection( ) )
	{
		fSelectionWindow = FALSE;
		ShutdownJA2SelectionWindow( );
		// Quick hack to trash the mouse event queue.
		while( DequeueEvent( &DummyEvent ) )
			continue;

		iCurBank = 0;

		if( iDrawMode == DRAW_MODE_SLANTED_ROOF || iDrawMode == DRAW_MODE_ROOM )
		{
			if( CheckForSlantRoofs() )
				iDrawMode = DRAW_MODE_SLANTED_ROOF;
		}
		InvalidateScreen( );
		ExecuteBaseDirtyRectQueue();
	}
	else
	{
		DisplaySelectionWindowGraphicalInformation();
		InvalidateScreen( );
		ExecuteBaseDirtyRectQueue();
	}

	return( EDIT_SCREEN );
}


BOOLEAN PlaceLight(INT16 const radius, GridNo const pos)
try
{
	ST::string filename = ST::format("L-R{02d}.LHT", radius);

	// Attempt to create light
	LIGHT_SPRITE* l = LightSpriteCreate(filename);
	if (!l)
	{
		// Couldn't load file because it doesn't exist. So let's make the file
		UINT8 const intensity = radius / LIGHT_DECAY;
		LightTemplate* const t = LightCreateOmni(intensity, radius);

		LightSave(t, filename);

		l = LightSpriteCreate(filename);
		if (!l)
		{
			// Can't create sprite
			SLOGW("PlaceLight: Can't create light sprite of radius {}", radius);
			return FALSE;
		}
	}

	LightSpritePower(l, TRUE);
	INT16 x;
	INT16 y;
	ConvertGridNoToXY(pos, &x, &y);
	LightSpritePosition(l, x, y);

	switch (gbDefaultLightType)
	{
		case PRIMETIME_LIGHT: l->uiFlags |= LIGHT_PRIMETIME; break;
		case NIGHTTIME_LIGHT: l->uiFlags |= LIGHT_NIGHTTIME; break;
	}

	if (!FindTypeInObjectLayer(pos, GOODRING))
	{
		LEVELNODE* const n = AddObjectToHead(pos, GOODRING1);
		n->ubShadeLevel = DEFAULT_SHADE_LEVEL;
	}

	AddLightToUndoList(pos, 0);

	return TRUE;
}
catch (...) { return FALSE; }


//----------------------------------------------------------------------------------------------
//	RemoveLight
//
//	Removes (erases) all lights at a given map tile location. Lights that are attached to a merc
//	are not deleted.
//
//	Returns TRUE if deleted the light, otherwise, returns FALSE.
//	i.e. FALSE is not an error condition!
//
BOOLEAN RemoveLight(const GridNo pos)
{
	// Check all lights if any at this given position
	const char* pLastLightName = NULL;
	FOR_EACH_LIGHT_SPRITE(l)
	{
		if (MAPROWCOLTOPOS(l->iY, l->iX) == pos)
		{
			if (!IsSoldierLight(l))
			{
				// Ok, it's not a merc's light so kill it!
				pLastLightName = LightSpriteGetTypeName(l);
				LightSpritePower(l, FALSE);
				LightSpriteDestroy(l);
				RemoveAllObjectsOfTypeRange(pos, GOODRING, GOODRING);
			}
		}
	}

	if (pLastLightName == NULL) return FALSE;

	/* Assuming that the light naming convention doesn't change, then this
	 * following conversion should work.  Basically, the radius values aren't
	 * stored in the lights, so I have pull the radius out of the filename.
	 * Ex:  L-RO5.LHT */
	const UINT16 usRadius = pLastLightName[4] - 0x30;
	AddLightToUndoList(pos, usRadius);
	return TRUE;
}


void ShowLightPositionHandles(void)
{
	CFOR_EACH_LIGHT_SPRITE(l)
	{
		if (IsSoldierLight(l)) continue;

		INT32 const iMapIndex = (INT32)l->iY * WORLD_COLS + (INT32)l->iX;
		if (FindTypeInObjectLayer(iMapIndex, GOODRING)) continue;

		LEVELNODE* const n = AddObjectToHead(iMapIndex, GOODRING1);
		n->ubShadeLevel = DEFAULT_SHADE_LEVEL;
	}
}


/* Scan through all light currently in the world and remove any light markers
 * that may be present. */
static void RemoveLightPositionHandles(void)
{
	CFOR_EACH_LIGHT_SPRITE(l)
	{
		if (IsSoldierLight(l)) continue;

		INT32 const iMapIndex = (INT32)l->iY * WORLD_COLS + (INT32)l->iX;
		RemoveAllObjectsOfTypeRange(iMapIndex, GOODRING, GOODRING);
	}
}


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
//	End of button callback functions
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
//	CheckForSlantRoofs
//
//	Verifies whether there are any slant roofs selected in the "multiwall" or "room" selection list.
//	if so, and no flat roofs were selected, then we return TRUE, otherwise we return FALSE.
//
//	This function is used to determine if we should force the area selection of rooms to handle
//	slant roofs (which require at least one side to be exactly 8 tiles wide)
//
static BOOLEAN CheckForSlantRoofs(void)
{
	UINT16 usCheck;

	pSelList = SelRoom;
	pNumSelList = &iNumRoomsSelected;

	usCheck = GetRandomIndexByRange( FIRSTROOF, LASTROOF );
	if ( usCheck != 0xffff )
		return( FALSE );

	usCheck = GetRandomIndexByRange( FIRSTSLANTROOF, LASTSLANTROOF );
	return ( usCheck != 0xffff );
}


//	Runs through all map locations, and if it's outside the visible world, then we remove
//	EVERYTHING from it since it will never be seen!
//
//	If it can be seen, then we remove all extraneous land tiles. We find the tile that has the first
//	FULL TILE indicator, and delete anything that may come after it (it'll never be seen anyway)
//
//	Doing the above has shown to free up about 1.1 Megs on the default map. Deletion of non-viewable
//	land pieces alone gained us about 600 K of memory.
static void MapOptimize(void)
{
#if 0
	INT16 gridno;
	LEVELNODE *start, *head, *end, *node, *temp;
	MAP_ELEMENT		*pMapTile;
	BOOLEAN fFound, fChangedHead, fChangedTail;

	for( gridno = 0; gridno < WORLD_MAX; gridno++ )
	{
		if ( !GridNoOnVisibleWorldTile( gridno ) )
		{
			// Tile isn't in viewable area so trash everything in it
			TrashMapTile( gridno );
		}
		else
		{
			// Tile is in viewable area so try to optimize any extra land pieces
			pMapTile = &gpWorldLevelData[ gridno ];

			node = start = pMapTile->pLandStart;
			head = pMapTile->pLandHead;

			if ( start == NULL )
				node = start = head;

			end = pMapTile->pLandTail;

			fChangedHead = fChangedTail = fFound = FALSE;
			while ( !fFound && node != NULL )
			{
				if ( gTileDatabase[node->usIndex].ubFullTile == 1 )
					fFound = TRUE;
				else
					node = node->pNext;
			}

			if(fFound)
			{
				// Delete everything up to the start node
/*
// Not having this means we still keep the smoothing

				while( head != start && head != NULL )
				{
					fChangedHead = TRUE;
					temp = head->pNext;
					delete head;
					head = temp;
					if ( head )
						head->pPrev = NULL;
				}
*/

				// Now delete from the end to "node"
				while( end != node && end != NULL )
				{
					fChangedTail = TRUE;
					temp = end->pPrev;
					delete end;
					end = temp;
					if ( end )
						end->pNext = NULL;
				}

				if ( fChangedHead )
					pMapTile->pLandHead = head;

				if ( fChangedTail )
					pMapTile->pLandTail = end;
			}
		}
	}

#endif
}



//----------------------------------------------------------------------------------------------
//	CheckForFences
//
//	Verifies whether the selection list has only fence pieces. If so, turns off random selection
//	for that selection list.
//
//	if ANY piece other than a fence piece exists in the selection list, then we want to keep
//	random selections.
//
static BOOLEAN CheckForFences(void)
{
	UINT16 usCheck;
	BOOLEAN fFence;

	pSelList = SelOStructs2;
	pNumSelList = &iNumOStructs2Selected;

	fFence = TRUE;

	for ( usCheck = 0; usCheck < iNumOStructs2Selected; usCheck++ )
	{
		const TILE_ELEMENT* const T = TileElemFromTileType(pSelList[usCheck].uiObject);
		if ( T->pDBStructureRef == NULL )
			fFence = FALSE;
		else if ( !(T->pDBStructureRef->pDBStructure->fFlags & STRUCTURE_ANYFENCE) )
			fFence = FALSE;
	}

	return( fFence );
}


static void EnsureStatusOfEditorButtons(void)
{
	if ( iDrawMode >= DRAW_MODE_ERASE )
	{
		ClickEditorButton( TERRAIN_TOGGLE_ERASEMODE );
		ClickEditorButton( BUILDING_TOGGLE_ERASEMODE );
		ClickEditorButton( MAPINFO_TOGGLE_ERASEMODE );
	}
	else
	{
		UnclickEditorButton( TERRAIN_TOGGLE_ERASEMODE );
		UnclickEditorButton( BUILDING_TOGGLE_ERASEMODE );
		UnclickEditorButton( MAPINFO_TOGGLE_ERASEMODE );
	}

	//disable erase buttons if drawing caves
	if( iDrawMode == DRAW_MODE_CAVES || iDrawMode == DRAW_MODE_NEWROOF )
		DisableEditorButton( BUILDING_TOGGLE_ERASEMODE );
	else
		EnableEditorButton( BUILDING_TOGGLE_ERASEMODE );

	// Ensure that the fill button is on or off depending on our current mode
	if ( iDrawMode == (DRAW_MODE_GROUND + DRAW_MODE_FILL_AREA) )
		ClickEditorButton( TERRAIN_FILL_AREA );
	else
		UnclickEditorButton( TERRAIN_FILL_AREA );

	// Make sure that the fake-lighting button is down if appropriate
	if ( gfFakeLights )
		ClickEditorButton( MAPINFO_TOGGLE_FAKE_LIGHTS );
	else
		UnclickEditorButton( MAPINFO_TOGGLE_FAKE_LIGHTS );
}


static void DrawObjectsBasedOnSelectionRegion(void);


static void HandleMouseClicksInGameScreen()
{
	GridNo const map_idx = guiCurrentCursorGridNo;
	if (map_idx == NOWHERE) return;

	// If in taskbar modes which don't process clicks in the world
	if (iCurrentTaskbar == TASK_OPTIONS) return;
	if (iCurrentTaskbar == TASK_NONE)    return;

	// If mouse cursor not in the game screen.
	if (!(gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA)) return;

	BOOLEAN const prev_state = gfRenderWorld;

	if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
	{
		gfRenderWorld = TRUE;
		// Are we trying to erase something?
		if (iDrawMode >= DRAW_MODE_ERASE)
		{
			// Erasing can have a brush size larger than 1 tile
			for (INT16 y = gSelectRegion.iTop; y <= gSelectRegion.iBottom; ++y)
			{
				for (INT16 x = gSelectRegion.iLeft; x <= gSelectRegion.iRight; ++x)
				{
					GridNo const pos = MAPROWCOLTOPOS(y, x);
					if (iDrawMode == DRAW_MODE_LIGHT + DRAW_MODE_ERASE)
					{
						RemoveLight(pos);
					}
					else
					{
						EraseMapTile(pos);
					}
				}
			}

			if (iDrawMode == DRAW_MODE_LIGHT + DRAW_MODE_ERASE)
				LightSpriteRenderAll();	// To adjust building's lighting
			return;
		}

		switch (iDrawMode)
		{
			case DRAW_MODE_SCHEDULEACTION:
				if (IsLocationSittableExcludingPeople(map_idx, FALSE))
				{
					iDrawMode = DRAW_MODE_SCHEDULECONFIRM;
					gfFirstPlacement = FALSE;
				}
				break;

			case DRAW_MODE_NORTHPOINT:
			case DRAW_MODE_WESTPOINT:
			case DRAW_MODE_EASTPOINT:
			case DRAW_MODE_SOUTHPOINT:
			case DRAW_MODE_CENTERPOINT:
			case DRAW_MODE_ISOLATEDPOINT:
				SpecifyEntryPoint(map_idx);
				break;

			case DRAW_MODE_ENEMY:
			case DRAW_MODE_CREATURE:
			case DRAW_MODE_REBEL:
			case DRAW_MODE_CIVILIAN:
				// Handle adding mercs to the world
				if (gfFirstPlacement)
				{
					AddMercToWorld(map_idx);
					gfFirstPlacement = FALSE;
				}
				break;

			case DRAW_MODE_LIGHT:
				// Add a normal light to the world
				if (gfFirstPlacement)
				{
					PlaceLight(gsLightRadius, map_idx);
					gfFirstPlacement = FALSE;
				}
				break;

			case DRAW_MODE_SAW_ROOM:
			case DRAW_MODE_ROOM:
			case DRAW_MODE_CAVES:
				if (gusSelectionType <= XLARGESELECTION) ProcessAreaSelection(TRUE);
				break;

			case DRAW_MODE_NEWROOF:
				ReplaceBuildingWithNewRoof(map_idx);
				break;

			case DRAW_MODE_WALLS:              PasteSingleWall(      map_idx); break;
			case DRAW_MODE_DOORS:              PasteSingleDoor(      map_idx); break;
			case DRAW_MODE_WINDOWS:            PasteSingleWindow(    map_idx); break;
			case DRAW_MODE_ROOFS:              PasteSingleRoof(      map_idx); break;
			case DRAW_MODE_BROKEN_WALLS:       PasteSingleBrokenWall(map_idx); break;
			case DRAW_MODE_DECOR:              PasteSingleDecoration(map_idx); break;

			case DRAW_MODE_DECALS:
				if (ValidDecalPlacement(map_idx)) PasteSingleDecal(map_idx);
				break;

			case DRAW_MODE_TOILET:             PasteSingleToilet(    map_idx); break;
			case DRAW_MODE_SMART_WALLS:        PasteSmartWall(       map_idx); break;
			case DRAW_MODE_SMART_DOORS:        PasteSmartDoor(       map_idx); break;
			case DRAW_MODE_SMART_WINDOWS:      PasteSmartWindow(     map_idx); break;
			case DRAW_MODE_SMART_BROKEN_WALLS: PasteSmartBrokenWall( map_idx); break;

			case DRAW_MODE_EXITGRID:
			case DRAW_MODE_FLOORS:
			case DRAW_MODE_GROUND:
			case DRAW_MODE_OSTRUCTS:
			case DRAW_MODE_OSTRUCTS1:
			case DRAW_MODE_OSTRUCTS2:
			case DRAW_MODE_DEBRIS:
				if (gusSelectionType <= XLARGESELECTION)
				{
					DrawObjectsBasedOnSelectionRegion();
				}
				else
				{
					gfRenderWorld = prev_state;
				}
				break;

			case DRAW_MODE_DOORKEYS:
				InitDoorEditing(map_idx);
				break;

			case DRAW_MODE_KILL_BUILDING:
				KillBuilding(map_idx);
				break;

			case DRAW_MODE_COPY_BUILDING:
			case DRAW_MODE_MOVE_BUILDING:
				if (gfFirstPlacement)
				{
					CopyBuilding(map_idx);
					gfFirstPlacement = FALSE;
				}
				gfRenderWorld = prev_state;
				break;

			case DRAW_MODE_BANKS:
				PasteBanks(map_idx, TRUE);
				break;

			case DRAW_MODE_ROADS:
				PasteRoads(map_idx);
				break;

			case DRAW_MODE_GROUND + DRAW_MODE_FILL_AREA:
				TerrainFill(map_idx);
				break;

			case DRAW_MODE_PLACE_ITEM:
				if (gfFirstPlacement)
				{
					AddSelectedItemToWorld(map_idx);
					gfFirstPlacement = FALSE;
				}
				break;

			default:
				gfRenderWorld = prev_state;
				break;
		}
	}
	else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		gfRenderWorld = TRUE;
		switch (iDrawMode)
		{
			// Handle right clicking on a merc (for editing/moving him)
			case DRAW_MODE_ENEMY:
			case DRAW_MODE_CREATURE:
			case DRAW_MODE_REBEL:
			case DRAW_MODE_CIVILIAN:
				HandleRightClickOnMerc(map_idx);
				break;

			case DRAW_MODE_PLACE_ITEM:
				HandleRightClickOnItem(map_idx);
				break;

			// Handle the right clicks in the main window to bring up the appropriate selection window
			case DRAW_MODE_WALLS:        iEditorToolbarState = TBAR_MODE_GET_WALL;        break;
			case DRAW_MODE_DOORS:        iEditorToolbarState = TBAR_MODE_GET_DOOR;        break;
			case DRAW_MODE_WINDOWS:      iEditorToolbarState = TBAR_MODE_GET_WINDOW;      break;
			case DRAW_MODE_ROOFS:        iEditorToolbarState = TBAR_MODE_GET_ROOF;        break;
			case DRAW_MODE_BROKEN_WALLS: iEditorToolbarState = TBAR_MODE_GET_BROKEN_WALL; break;
			case DRAW_MODE_DECOR:        iEditorToolbarState = TBAR_MODE_GET_DECOR;       break;
			case DRAW_MODE_DECALS:       iEditorToolbarState = TBAR_MODE_GET_DECAL;       break;
			case DRAW_MODE_FLOORS:       iEditorToolbarState = TBAR_MODE_GET_FLOOR;       break;
			case DRAW_MODE_TOILET:       iEditorToolbarState = TBAR_MODE_GET_TOILET;      break;

			case DRAW_MODE_ROOM:         iEditorToolbarState = TBAR_MODE_GET_ROOM;        break;
			case DRAW_MODE_NEWROOF:      iEditorToolbarState = TBAR_MODE_GET_NEW_ROOF;    break;
			case DRAW_MODE_SLANTED_ROOF: iEditorToolbarState = TBAR_MODE_GET_ROOM;        break;
			case DRAW_MODE_DEBRIS:       iEditorToolbarState = TBAR_MODE_GET_DEBRIS;      break;
			case DRAW_MODE_OSTRUCTS:     iEditorToolbarState = TBAR_MODE_GET_OSTRUCTS;    break;
			case DRAW_MODE_OSTRUCTS1:    iEditorToolbarState = TBAR_MODE_GET_OSTRUCTS1;   break;
			case DRAW_MODE_OSTRUCTS2:    iEditorToolbarState = TBAR_MODE_GET_OSTRUCTS2;   break;
			case DRAW_MODE_BANKS:        iEditorToolbarState = TBAR_MODE_GET_BANKS;       break;
			case DRAW_MODE_ROADS:        iEditorToolbarState = TBAR_MODE_GET_ROADS;       break;

			case DRAW_MODE_CAVES:
				if (gusSelectionType <= XLARGESELECTION) ProcessAreaSelection(FALSE);
				break;

			case DRAW_MODE_SMART_WALLS:
				EraseWalls(map_idx);
				break;

			case DRAW_MODE_SMART_BROKEN_WALLS:
			case DRAW_MODE_SMART_WINDOWS:
			case DRAW_MODE_SMART_DOORS:
				RestoreWalls(map_idx);
				break;

			case DRAW_MODE_EXITGRID:
				if (GetExitGrid(map_idx, &gExitGrid)) ApplyNewExitGridValuesToTextFields();
				break;

			default:
				gfRenderWorld = prev_state;
				break;
		}
	}
	else if (!gfFirstPlacement)
	{
		switch (iDrawMode)
		{
			case DRAW_MODE_SCHEDULECONFIRM:
				if (IsLocationSittableExcludingPeople(map_idx, FALSE))
				{
					RegisterCurrentScheduleAction(map_idx);
				}
				break;

			case DRAW_MODE_COPY_BUILDING: PasteBuilding(map_idx); break;
			case DRAW_MODE_MOVE_BUILDING: MoveBuilding(map_idx);  break;
		}
	}
}


static BOOLEAN DoIRenderASpecialMouseCursor(void)
{
	if ( iCurrentTaskbar != TASK_OPTIONS )
	{
		switch ( iDrawMode )
		{
			case DRAW_MODE_OSTRUCTS2:
				if(CheckForFences())
					fDontUseRandom = TRUE;
				// fallthrough
			case DRAW_MODE_DEBRIS:							// These only show if you first hit PGUP/PGDOWN keys
			case DRAW_MODE_OSTRUCTS:
			case DRAW_MODE_OSTRUCTS1:
				if(!fDontUseRandom)
					break;
				// fallthrough
			case DRAW_MODE_BANKS:
			case DRAW_MODE_ROADS:
			case DRAW_MODE_WALLS:
			case DRAW_MODE_DOORS:
			case DRAW_MODE_WINDOWS:
			case DRAW_MODE_ROOFS:
			case DRAW_MODE_BROKEN_WALLS:
			case DRAW_MODE_DECOR:
			case DRAW_MODE_DECALS:
			case DRAW_MODE_FLOORS:
			case DRAW_MODE_TOILET:

			case DRAW_MODE_SMART_WALLS:
			case DRAW_MODE_SMART_DOORS:
			case DRAW_MODE_SMART_WINDOWS:
			case DRAW_MODE_SMART_BROKEN_WALLS:
			case DRAW_MODE_ROOM:
			case DRAW_MODE_NEWROOF:
				return DrawTempMouseCursorObject( );

			default:
				break;
		}
	}
	return FALSE;
}


void ShowEntryPoints()
{ // Make entry points visible
	MAPCREATE_STRUCT const& m = gMapInformation;
	if (m.sNorthGridNo != -1) AddTopmostToTail(m.sNorthGridNo, FIRSTPOINTERS2);
	if (m.sEastGridNo  != -1) AddTopmostToTail(m.sEastGridNo,  FIRSTPOINTERS2);
	if (m.sSouthGridNo != -1) AddTopmostToTail(m.sSouthGridNo, FIRSTPOINTERS2);
	if (m.sWestGridNo  != -1) AddTopmostToTail(m.sWestGridNo,  FIRSTPOINTERS2);
}


static void HideEntryPoints()
{ // Remove entry point indicators
	MAPCREATE_STRUCT const& m = gMapInformation;
	if (m.sNorthGridNo != -1) RemoveAllTopmostsOfTypeRange(m.sNorthGridNo, FIRSTPOINTERS, FIRSTPOINTERS);
	if (m.sEastGridNo  != -1) RemoveAllTopmostsOfTypeRange(m.sEastGridNo,  FIRSTPOINTERS, FIRSTPOINTERS);
	if (m.sSouthGridNo != -1) RemoveAllTopmostsOfTypeRange(m.sSouthGridNo, FIRSTPOINTERS, FIRSTPOINTERS);
	if (m.sWestGridNo  != -1) RemoveAllTopmostsOfTypeRange(m.sWestGridNo,  FIRSTPOINTERS, FIRSTPOINTERS);
}

void TaskOptionsCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iTaskMode = TASK_OPTIONS;
	}
}

void TaskTerrainCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iTaskMode = TASK_TERRAIN;
	}
}

void TaskBuildingCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iTaskMode = TASK_BUILDINGS;
	}
}

void TaskItemsCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iTaskMode = TASK_ITEMS;
	}
}

void TaskMercsCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iTaskMode = TASK_MERCS;
	}
}

void TaskMapInfoCallback(GUI_BUTTON *btn,UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iTaskMode = TASK_MAPINFO;
	}
}

void ProcessAreaSelection( BOOLEAN fWithLeftButton )
{
	BOOLEAN fPrevState = gfRenderWorld;
	gfRenderWorld = TRUE;
	switch( iDrawMode )
	{
		case DRAW_MODE_ROOM:
		case DRAW_MODE_SLANTED_ROOF:
			AddBuildingSectionToWorld( &gSelectRegion );
			break;
		case DRAW_MODE_SAW_ROOM:
			RemoveBuildingSectionFromWorld( &gSelectRegion );
			break;
		case DRAW_MODE_CAVES:
			if( fWithLeftButton )
				AddCaveSectionToWorld( &gSelectRegion );
			else
				RemoveCaveSectionFromWorld( &gSelectRegion );
			break;
		case DRAW_MODE_ROOMNUM:
			DrawObjectsBasedOnSelectionRegion();
			if( gubCurrRoomNumber > 0 )
			{
				gubCurrRoomNumber++;
				gubMaxRoomNumber++;
				if( iCurrentTaskbar == TASK_BUILDINGS && TextInputMode() )
				{
					ST::string str = ST::format("{}", gubCurrRoomNumber);
					SetInputFieldString( 1, str );
					SetActiveField( 0 );
				}
			}
			break;
		case DRAW_MODE_EXITGRID:
		case DRAW_MODE_FLOORS:
		case DRAW_MODE_GROUND:
		case DRAW_MODE_OSTRUCTS:
		case DRAW_MODE_OSTRUCTS1:
		case DRAW_MODE_OSTRUCTS2:
		case DRAW_MODE_DEBRIS:
			//These aren't normally supported by area selection, so
			//call the equivalent function that is normally associated with them.
			DrawObjectsBasedOnSelectionRegion();
			break;
		default:
			gfRenderWorld = fPrevState;
			break;
	}
}


//For any drawing modes that support large cursors, or even area selection, this function calls the
//appropriate paste function for every gridno within the cursor.  This is not used for functions that
//rely completely on selection areas, such as buildings.
static void DrawObjectsBasedOnSelectionRegion(void)
{
	INT32 x, y, iMapIndex;
	BOOLEAN fSkipTest;

	//Certain drawing modes are placed with 100% density.  Those cases are checked here,
	//so the density test can be skipped.
	fSkipTest = FALSE;
	if( gusSelectionType == SMALLSELECTION ||
		iDrawMode == DRAW_MODE_GROUND ||
		iDrawMode == DRAW_MODE_FLOORS ||
		iDrawMode == DRAW_MODE_ROOMNUM ||
		iDrawMode == DRAW_MODE_EXITGRID )
	{
		fSkipTest = TRUE;
	}

	//The reason why I process the region from top to bottom then to the right is
	//to even out the binary tree undo placements.  Otherwise, the placements within
	//the undo binary tree would alway choose the right branch because the imapindex is
	//always greater than the previously positioned one.
	//Process the cursor area
	for ( x = gSelectRegion.iLeft; x <= gSelectRegion.iRight; x++ )
	{
		//process the region from
		for ( y = gSelectRegion.iTop; y <= (INT16)gSelectRegion.iBottom; y++ )
		{
			if( fSkipTest || PerformDensityTest() )
			{
				iMapIndex = MAPROWCOLTOPOS( y, x );
				if (iMapIndex < 0 || iMapIndex >= WORLD_MAX) continue;

				switch( iDrawMode )
				{
					case DRAW_MODE_EXITGRID:
						AddToUndoList( iMapIndex );
						AddExitGridToWorld( iMapIndex, &gExitGrid );
						AddTopmostToTail( (UINT16)iMapIndex, FIRSTPOINTERS8 );
						break;
					case DRAW_MODE_DEBRIS:		PasteDebris( iMapIndex );													break;
					case DRAW_MODE_FLOORS:		PasteSingleFloor( iMapIndex );										break;
					case DRAW_MODE_GROUND:		PasteTexture( iMapIndex );												break;
					case DRAW_MODE_OSTRUCTS:	PasteStructure( iMapIndex );											break;
					case DRAW_MODE_OSTRUCTS1: PasteStructure1( iMapIndex );											break;
					case DRAW_MODE_OSTRUCTS2: PasteStructure2( iMapIndex );											break;
					case DRAW_MODE_ROOMNUM:   PasteRoomNumber( iMapIndex, gubCurrRoomNumber );	break;
					default: return; //no point in continuing...
				}
			}
		}
	}
}


//The main loop of the editor.
ScreenID EditScreenHandle(void)
{
	BOOLEAN fShowingCursor;

	if( gfWorldLoaded && gMapInformation.ubMapVersion <= 7 && !gfCorruptMap )
	{
		SLOGE("Map data has just been corrupted. Don't save, don't quit, get Kris!  If he's not here, save the map using a temp filename and document everything you just did, especially your last action!");
		gfCorruptMap = TRUE;
	}
	if( gfWorldLoaded && gubScheduleID > 40 && !gfCorruptSchedules )
	{
		OptimizeSchedules();
		if( gubScheduleID > 32 )
		{
			SLOGE("Schedule data has just been corrupted. Don't save, don't quit, get Kris!  If he's not here, save the map using a temp filename and document everything you just did, especially your last action!");
			gfCorruptSchedules = TRUE;
		}
	}

	if( gfAutoLoadA9 == 2 )
		AutoLoadMap();

	if ( fEditModeFirstTime )
	{
		EditModeInit( );
	}

	if ( InOverheadMap( ) && !gfSummaryWindowActive )
	{
		HandleOverheadMap( );
	}

	DetermineUndoState();

	fBeenWarned = FALSE;

	// Handle the bottom task bar menu.
	DoTaskbar();

	//Process the variety of popup menus, dialogs, etc.

	if( gubMessageBoxStatus )
	{
		if( MessageBoxHandled( ) )
			return ProcessEditscreenMessageBoxResponse();
		return EDIT_SCREEN;
	}

	if( ProcessPopupMenuIfActive() )
		return EDIT_SCREEN;
	if ( fHelpScreen )
		return( WaitForHelpScreenResponse( ) );
	if ( fSelectionWindow )
		return( WaitForSelectionWindowResponse( ) );

	if (gfSummaryWindowSaveRequested) {
		gfSummaryWindowSaveRequested = FALSE;
		iCurrentAction = ACTION_SAVE_MAP;
		return LOADSAVE_SCREEN;
	}

	// If editing mercs, handle that stuff
	ProcessMercEditing();

	EnsureStatusOfEditorButtons();

	// Handle scrolling of the map if needed
	if( !gfGotoGridNoUI && iDrawMode != DRAW_MODE_SHOW_TILESET && !gfSummaryWindowActive &&
			!gfEditingDoor && !gfNoScroll && !InOverheadMap() )
		ScrollWorld();

	iCurrentAction = ACTION_NULL;

	UpdateCursorAreas();

	HandleMouseClicksInGameScreen();

	if( !gfFirstPlacement && !IsMouseButtonDown(MOUSE_BUTTON_LEFT) )
		gfFirstPlacement = TRUE;

	//If we are copying or moving a building, we process, then delete the building layout immediately
	//after releasing the mouse button.  If released in the world, then the building would have been
	//processed in above function, HandleMouseClicksInGameScreen().
	if( !IsMouseButtonDown(MOUSE_BUTTON_LEFT) && gpBuildingLayoutList )
		DeleteBuildingLayout();

	fShowingCursor = DoIRenderASpecialMouseCursor();

	DequeAllGameEvents();

	if ( fBuildingShowRoomInfo )
	{
		SetRenderFlags( RENDER_FLAG_ROOMIDS );
	}

	if ( gfRenderWorld )
	{
		if( gCursorNode )
			gCursorNode->uiFlags &= (~LEVELNODE_REVEAL);

		// This flag is the beast that makes the renderer do everything
		MarkWorldDirty();

		gfRenderWorld = FALSE;
	}

	// The default here for the renderer is to just do dynamic stuff...
	if( !gfSummaryWindowActive && !gfEditingDoor && !InOverheadMap() )
		RenderWorld();

	DisplayWayPoints();

	if ( fShowingCursor )
		RemoveTempMouseCursorObject();

	ProcessEditorRendering();

	// Handle toolbar selections
	HandleJA2ToolbarSelection( );

	// Handle keyboard shortcuts / selections
	HandleKeyboardShortcuts( );

	// Perform action based on current selection
	ScreenID const uiRetVal = PerformSelectedAction();
	if (uiRetVal != EDIT_SCREEN) return uiRetVal;

	// Display Framerate
	DisplayFrameRate( );

	// Handle video overlays, for FPS and screen message stuff
	if ( gfScrollPending )
	{
		AllocateVideoOverlaysArea( );
		SaveVideoOverlaysArea( FRAME_BUFFER );
	}
	ExecuteVideoOverlays( );

	ScrollString( );

	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender( );

	return EDIT_SCREEN;
}


static void CreateGotoGridNoUI(void)
{
	gfGotoGridNoUI = TRUE;
	//Disable the rest of the editor
	DisableEditorTaskbar();
	//Create the background panel.
	guiGotoGridNoUIButtonID = CreateLabel("Enter gridno:", FONT10ARIAL, FONT_YELLOW, FONT_BLACK, 290, 155, 60, 50, MSYS_PRIORITY_NORMAL);
	guiGotoGridNoUIButtonID->SpecifyTextOffsets(5, 5, FALSE);
	//Create a blanket region so nobody can use
	MSYS_DefineRegion(&GotoGridNoUIRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_NORMAL + 1, 0, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
	//Init a text input field.
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	AddTextInputField( 300, 180, 40, 18, MSYS_PRIORITY_HIGH, ST::null, 6, INPUTTYPE_NUMERICSTRICT );
}


static void RemoveGotoGridNoUI(void)
{
	INT32 iMapIndex;
	gfGotoGridNoUI = FALSE;
	//Enable the rest of the editor
	EnableEditorTaskbar();
	RemoveButton( guiGotoGridNoUIButtonID );
	iMapIndex = GetNumericStrictValueFromField( 0 );
	KillTextInputMode();
	MSYS_RemoveRegion( &GotoGridNoUIRegion );
	if( iMapIndex != -1 )
	{ //Warp the screen to the location of this gridno.
		CenterScreenAtMapIndex( iMapIndex );
	}
	else
	{
		MarkWorldDirty();
	}
}


static void UpdateLastActionBeforeLeaving(void)
{
	if( iCurrentTaskbar == TASK_MERCS )
		IndicateSelectedMerc( SELECT_NO_MERC );
	SpecifyItemToEdit( NULL, -1 );
}


static void ReloadMap(void)
{
	ExternalLoadMap( gFileForIO );
}
