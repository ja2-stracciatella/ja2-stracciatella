#include <stdarg.h>
#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Handle_Items.h"
#include "Local.h"
#include "Sys_Globals.h"
#include "Timer_Control.h"
#include "Types.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "Isometric_Utils.h"
#include "VObject.h"
#include "WorldDat.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "WordWrap.h"
#include "Environment.h"
#include "Interface_Items.h"
#include "Soldier_Find.h"
#include "World_Items.h"
#include "Text.h"
#include "Overhead_Map.h"
#include "Cursor_Modes.h"
#include "EditScreen.h"
#include "EditorTerrain.h"
#include "EditorItems.h"
#include "EditorMercs.h"
#include "EditorBuildings.h"
#include "EditorMapInfo.h"
#include "EditorDefines.h"
#include "Editor_Taskbar_Utils.h"
#include "Editor_Taskbar_Creation.h"
#include "Editor_Callback_Prototypes.h"
#include "Editor_Modes.h"
#include "Text_Input.h"
#include "Item_Statistics.h"
#include "Map_Information.h"
#include "Sector_Summary.h"
#include "Pits.h"
#include "Lighting.h"
#include "Keys.h"
#include "Debug.h"
#include "JAScreens.h"
#include "Video.h"
#include "VSurface.h"

#include "ContentManager.h"
#include "GameInstance.h"


//editor icon storage vars
INT32	giEditMercDirectionIcons[2];
SGPVObject* guiMercInventoryPanel;
SGPVObject* guiOmertaMap;
SGPVSurface* guiMercInvPanelBuffers[9];
SGPVSurface* guiMercTempBuffer;
SGPVObject* guiExclamation;
SGPVObject* guiKeyImage;

//editor Mouseregion storage vars
static MOUSE_REGION TerrainTileButtonRegion[NUM_TERRAIN_TILE_REGIONS];
static MOUSE_REGION ItemsRegion;
static MOUSE_REGION MercRegion;
static MOUSE_REGION EditorRegion;

void EnableEditorRegion( INT8 bRegionID )
{
	switch( bRegionID )
	{
		case BASE_TERRAIN_TILE_REGION_ID:
		case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
			TerrainTileButtonRegion[bRegionID].Enable();
			break;
		case ITEM_REGION_ID:
			ItemsRegion.Enable();
			break;
		case MERC_REGION_ID:
			MercRegion.Enable();
			break;
	}
}

void DisableEditorRegion( INT8	bRegionID )
{
	switch( bRegionID )
	{
		case BASE_TERRAIN_TILE_REGION_ID:
		case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			TerrainTileButtonRegion[bRegionID].Disable();
			break;
		case ITEM_REGION_ID:
			ItemsRegion.Disable();
			break;
		case MERC_REGION_ID:
			MercRegion.Disable();
			break;
	}
}


static void RemoveEditorRegions(void)
{
	INT32 x;
	MSYS_RemoveRegion( &EditorRegion );
	for(x = BASE_TERRAIN_TILE_REGION_ID; x < NUM_TERRAIN_TILE_REGIONS; x++ )
	{
		MSYS_RemoveRegion( &TerrainTileButtonRegion[ x ] );
	}
	MSYS_RemoveRegion( &ItemsRegion );
	MSYS_RemoveRegion( &MercRegion );
}


static void InitEditorRegions()
{
	/* By doing this, all of the buttons underneath are blanketed and can't be
	 * used anymore. Any new buttons will cover this up as well. Think of it as a
	 * barrier between the editor buttons and the game's interface panel buttons
	 * and regions. */
	MSYS_DefineRegion(&EditorRegion, 0, EDITOR_TASKBAR_POS_Y, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_NORMAL, 0, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	// Create the regions for the terrain tile selections.
	UINT16       x   = 261;
	UINT16 const y   = EDITOR_TASKBAR_POS_Y + 9;
	INT32        idx =   0;
	FOR_EACHX(MOUSE_REGION, i, TerrainTileButtonRegion, x += 42)
	{
		MOUSE_REGION& r = *i;
		MSYS_DefineRegion(&r, x, y, x + 42, y + 22, MSYS_PRIORITY_NORMAL, 0, MSYS_NO_CALLBACK, TerrainTileButtonRegionCallback);
		MSYS_SetRegionUserData(&r, 0, idx++);
		r.Disable();
	}
	gfShowTerrainTileButtons = FALSE;

	// Create the region for the items selection window.
	MSYS_DefineRegion(&ItemsRegion, 100, EDITOR_TASKBAR_POS_Y, 540, EDITOR_TASKBAR_POS_Y + 80, MSYS_PRIORITY_NORMAL, 0, MouseMovedInItemsRegion, MouseClickedInItemsRegion);
	ItemsRegion.Disable();

	// Create the region for the merc inventory panel.
	MSYS_DefineRegion(&MercRegion, 175, EDITOR_TASKBAR_POS_Y + 1, 450, EDITOR_TASKBAR_POS_Y + 80, MSYS_PRIORITY_NORMAL, 0, MouseMovedInMercRegion, MouseClickedInMercRegion);
	MercRegion.Disable();
}


static void LoadEditorImages(void)
{
	//Set up the merc inventory panel
	guiMercInventoryPanel = AddVideoObjectFromFile(EDITORDIR "/invpanel.sti");
	//Set up small omerta map
	guiOmertaMap          = AddVideoObjectFromFile(EDITORDIR "/omerta.sti");
	//Set up the merc directional buttons.
	giEditMercDirectionIcons[0] = LoadGenericButtonIcon(EDITORDIR "/arrowsoff.sti");
	giEditMercDirectionIcons[1] = LoadGenericButtonIcon(EDITORDIR "/arrowson.sti");

	guiExclamation = AddVideoObjectFromFile(EDITORDIR "/exclamation.sti");
	guiKeyImage    = AddVideoObjectFromFile(EDITORDIR "/keyimage.sti");
}


static void DeleteEditorImages(void)
{
	//The merc inventory panel
	DeleteVideoObject(guiMercInventoryPanel);
	DeleteVideoObject(guiOmertaMap);
	//The merc directional buttons
	UnloadGenericButtonIcon( (INT16)giEditMercDirectionIcons[0] );
	UnloadGenericButtonIcon( (INT16)giEditMercDirectionIcons[1] );
}


static void CreateEditorBuffers(void)
{
	INT32						i;

	//create buffer for the transition slot for merc items.  This slot contains the newly
	//selected item graphic in it's inventory size version.  This buffer is then scaled down
	//into the associated merc inventory panel slot buffer which is approximately 20% smaller.
	guiMercTempBuffer = AddVideoSurface(60, 25, PIXEL_DEPTH);

	//create the nine buffers for the merc's inventory slots.
	for( i = 0; i < 9; i++ )
	{
		guiMercInvPanelBuffers[i] = AddVideoSurface(i < 3 ? MERCINV_SMSLOT_WIDTH : MERCINV_LGSLOT_WIDTH, MERCINV_SLOT_HEIGHT, PIXEL_DEPTH);
		guiMercInvPanelBuffers[i]->SetTransparency(0);
	}
}


static void DeleteEditorBuffers(void)
{
	INT32 i;
	DeleteVideoSurface(guiMercTempBuffer);
	for( i = 0; i < 9; i++ )
	{
		DeleteVideoSurface(guiMercInvPanelBuffers[i]);
	}
}


static void ShowEditorToolbar(TaskMode const iNewTaskMode)
{
	switch( iNewTaskMode )
	{
		case TASK_TERRAIN:
			ShowEditorButtons(FIRST_TERRAIN_BUTTON, LAST_TERRAIN_BUTTON);
			break;
		case TASK_BUILDINGS:
			ShowEditorButtons(FIRST_BUILDINGS_BUTTON, LAST_BUILDINGS_BUTTON);
			break;
		case TASK_ITEMS:
			ShowEditorButtons(FIRST_ITEMS_BUTTON, LAST_ITEMS_BUTTON);
			break;
		case TASK_MERCS:
			ShowEditorButtons(FIRST_MERCS_TEAMMODE_BUTTON, LAST_MERCS_TEAMMODE_BUTTON);
			break;
		case TASK_MAPINFO:
			ShowEditorButtons(FIRST_MAPINFO_BUTTON, LAST_MAPINFO_BUTTON);
			break;
		case TASK_OPTIONS:
			ShowEditorButtons(FIRST_OPTIONS_BUTTON, LAST_OPTIONS_BUTTON);
			break;
		default:
			return;
	}
}


static void HideEditorToolbar(TaskMode const iOldTaskMode)
{
	INT32 i, iStart, iEnd;
	switch( iOldTaskMode )
	{
		case TASK_TERRAIN:
			iStart = FIRST_TERRAIN_BUTTON;
			iEnd = LAST_TERRAIN_BUTTON;
			break;
		case TASK_BUILDINGS:
			iStart = FIRST_BUILDINGS_BUTTON;
			iEnd = LAST_BUILDINGS_BUTTON;
			break;
		case TASK_ITEMS:
			iStart = FIRST_ITEMS_BUTTON;
			iEnd = LAST_ITEMS_BUTTON;
			break;
		case TASK_MERCS:
			iStart = FIRST_MERCS_BUTTON;
			iEnd = LAST_MERCS_BUTTON;
			break;
		case TASK_MAPINFO:
			iStart = FIRST_MAPINFO_BUTTON;
			iEnd = LAST_MAPINFO_BUTTON;
			break;
		case TASK_OPTIONS:
			iStart = FIRST_OPTIONS_BUTTON;
			iEnd = LAST_OPTIONS_BUTTON;
			break;
		default:
			return;
	}
	for( i = iStart; i <= iEnd; i++ )
	{
		HideButton( iEditorButton[ i ] );
		UnclickEditorButton( i );
	}
}

void CreateEditorTaskbar()
{
	InitEditorRegions();
	LoadEditorImages();
	CreateEditorBuffers();
	CreateEditorTaskbarInternal();
	HideItemStatsPanel();
}


void DeleteEditorTaskbar()
{
	iOldTaskMode = iCurrentTaskbar;

	FOR_EACH(GUIButtonRef, i, iEditorButton) RemoveButton(*i);

	RemoveEditorRegions();
	DeleteEditorImages();
	DeleteEditorBuffers();
}


void DoTaskbar(void)
{
	if (iTaskMode == TASK_NONE || iTaskMode == iCurrentTaskbar)
	{
		return;
	}

	gfRenderTaskbar = TRUE;

	HideEditorToolbar( iCurrentTaskbar );

	//Special code when exiting previous editor tab
	switch( iCurrentTaskbar )
	{
		case TASK_TERRAIN:
			UnclickEditorButton( TAB_TERRAIN );
			HideTerrainTileButtons();
			break;
		case TASK_BUILDINGS:
			UnclickEditorButton( TAB_BUILDINGS );
			KillTextInputMode();
			break;
		case TASK_ITEMS:
			UnclickEditorButton( TAB_ITEMS );
			HideItemStatsPanel();
			if( eInfo.fActive )
				ClearEditorItemsInfo();
			gfShowPits = FALSE;
			RemoveAllPits();
			break;
		case TASK_MERCS:
			UnclickEditorButton( TAB_MERCS );
			IndicateSelectedMerc( SELECT_NO_MERC );
			SetMercEditingMode( MERC_NOMODE );
			break;
		case TASK_MAPINFO:
			UnclickEditorButton( TAB_MAPINFO );
			ExtractAndUpdateMapInfo();
			KillTextInputMode();
			HideExitGrids();
			break;
		case TASK_OPTIONS:
			UnclickEditorButton( TAB_OPTIONS );
			break;
		default:
			break;
	}

	//Setup the new tab mode
	iCurrentTaskbar = iTaskMode;
	ShowEditorToolbar( iTaskMode );
	iTaskMode = TASK_NONE;

	//Special code when entering a new editor tab
	switch( iCurrentTaskbar )
	{
		case TASK_MERCS:
			ClickEditorButton( TAB_MERCS );
			ClickEditorButton( MERCS_ENEMY);
			iDrawMode = DRAW_MODE_ENEMY;
			SetMercEditingMode( MERC_TEAMMODE );
			fBuildingShowRoofs = FALSE;
			UpdateRoofsView();
			break;
		case TASK_TERRAIN:
			ClickEditorButton( TAB_TERRAIN );
			ShowTerrainTileButtons();
			SetEditorTerrainTaskbarMode( TERRAIN_FGROUND_TEXTURES );
			break;
		case TASK_BUILDINGS:
			ClickEditorButton( TAB_BUILDINGS );
			if(fBuildingShowRoofs)
				ClickEditorButton( BUILDING_TOGGLE_ROOF_VIEW );
			if(fBuildingShowWalls)
				ClickEditorButton( BUILDING_TOGGLE_WALL_VIEW );
			if(fBuildingShowRoomInfo)
				ClickEditorButton( BUILDING_TOGGLE_INFO_VIEW );
			if( gfCaves )
			{
				ClickEditorButton( BUILDING_CAVE_DRAWING );
				iDrawMode = DRAW_MODE_CAVES;
			}
			else
			{
				ClickEditorButton( BUILDING_NEW_ROOM );
				iDrawMode = DRAW_MODE_ROOM;
			}
			TerrainTileDrawMode = TERRAIN_TILES_BRETS_STRANGEMODE;
			SetEditorSmoothingMode( gMapInformation.ubEditorSmoothingType );
			gusSelectionType = gusSavedBuildingSelectionType;
			SetupTextInputForBuildings();
			break;
		case TASK_ITEMS:
			SetFont( FONT10ARIAL );
			SetFontForeground( FONT_YELLOW );
			ClickEditorButton( TAB_ITEMS );
			ClickEditorButton( ITEMS_WEAPONS + eInfo.uiItemType - TBAR_MODE_ITEM_WEAPONS );
			InitEditorItemsInfo( eInfo.uiItemType );
			ShowItemStatsPanel();
			gfShowPits = TRUE;
			AddAllPits();
			iDrawMode = DRAW_MODE_PLACE_ITEM;
			break;
		case TASK_MAPINFO:
			ClickEditorButton( TAB_MAPINFO );
			if ( gfFakeLights )
				ClickEditorButton( MAPINFO_TOGGLE_FAKE_LIGHTS );
			ClickEditorButton( MAPINFO_ADD_LIGHT1_SOURCE );
			iDrawMode = DRAW_MODE_LIGHT;
			TerrainTileDrawMode = TERRAIN_TILES_BRETS_STRANGEMODE;
			SetupTextInputForMapInfo();
			break;
		case TASK_OPTIONS:
			ClickEditorButton( TAB_OPTIONS );
			iDrawMode           = DRAW_MODE_NOTHING;
			TerrainTileDrawMode = TERRAIN_TILES_NODRAW;
			break;
		default:
			break;
	}
}


void DisableEditorTaskbar()
{
	FOR_EACH(GUIButtonRef, i, iEditorButton) DisableButton(*i);
}


void EnableEditorTaskbar(void)
{
	INT32 x;

	for(x = 0; x < NUMBER_EDITOR_BUTTONS; x++ )
		EnableButton( iEditorButton[ x ] );
	//Keep permanent buttons disabled.
	DisableButton( iEditorButton[ MERCS_1 ] );
	DisableButton( iEditorButton[ MAPINFO_LIGHT_PANEL ] );
	DisableButton( iEditorButton[ MAPINFO_RADIO_PANEL ] );
	DisableButton( iEditorButton[ ITEMSTATS_PANEL ] );
	DisableButton( iEditorButton[ MERCS_PLAYERTOGGLE ] );
	DisableButton( iEditorButton[ MERCS_PLAYER ] );
	if( iCurrentTaskbar == TASK_ITEMS )
		DetermineItemsScrolling();
}

//A specialized mprint function that'll restore the editor panel underneath the
//string before rendering the string.  This is obviously only useful for drawing text
//in the editor taskbar.
void mprintfEditor(INT16 x, INT16 y, const wchar_t* pFontString, ...)
{
	va_list argptr;
	wchar_t	string[512];
	UINT16 uiStringLength, uiStringHeight;

	Assert( pFontString != NULL );

	va_start( argptr, pFontString );       	// Set up variable argument pointer
	vswprintf(string, lengthof(string), pFontString, argptr); // process gprintf string (get output str)
	va_end( argptr );

	uiStringLength = StringPixLength( string, FontDefault );
	uiStringHeight = GetFontHeight( FontDefault );

	ClearTaskbarRegion( x, y, (INT16)(x+uiStringLength), (INT16)(y+uiStringHeight) );
	MPrint(x, TASKBAR_Y + y, string);
}

void ClearTaskbarRegion( INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom )
{
	sTop    += TASKBAR_Y;
	sBottom += TASKBAR_Y;
	ColorFillVideoSurfaceArea( ButtonDestBuffer, sLeft, sTop, sRight, sBottom, gusEditorTaskbarColor );

	if( !sLeft )
	{
		ColorFillVideoSurfaceArea( ButtonDestBuffer, 0, sTop, 1, sBottom, gusEditorTaskbarHiColor );
		sLeft++;
	}
	if( sTop == EDITOR_TASKBAR_POS_Y )
	{
		ColorFillVideoSurfaceArea( ButtonDestBuffer, sLeft, EDITOR_TASKBAR_POS_Y, sRight, EDITOR_TASKBAR_POS_Y+1, gusEditorTaskbarHiColor );
		sTop++;
	}
	if (sBottom == SCREEN_HEIGHT)
	{
		ColorFillVideoSurfaceArea(ButtonDestBuffer, sLeft, SCREEN_HEIGHT - 1, sRight, SCREEN_HEIGHT, gusEditorTaskbarLoColor);
	}
	if (sRight == SCREEN_WIDTH)
	{
		ColorFillVideoSurfaceArea(ButtonDestBuffer, SCREEN_WIDTH - 1, sTop, SCREEN_WIDTH, sBottom, gusEditorTaskbarLoColor);
	}

	InvalidateRegion( sLeft, sTop, sRight, sBottom );
}


//Kris:
//This is a new function which duplicates the older "yellow info boxes" that
//are common throughout the editor.  This draws the yellow box with the indentation
//look.
void DrawEditorInfoBox(const wchar_t* str, SGPFont const font, UINT16 x, UINT16 y, UINT16 w, UINT16 h)
{
	UINT16 usFillColorDark, usFillColorLight, usFillColorBack;
	UINT16 x2, y2;

	y  += TASKBAR_Y;
	x2  = x + w;
	y2  = y + h;

	usFillColorDark = Get16BPPColor(FROMRGB(24, 61, 81));
	usFillColorLight = Get16BPPColor(FROMRGB(136, 138, 135));
	usFillColorBack = Get16BPPColor(FROMRGB(250, 240, 188));

	ColorFillVideoSurfaceArea(ButtonDestBuffer, x, y, x2, y2, usFillColorDark);
	ColorFillVideoSurfaceArea(ButtonDestBuffer, x + 1, y + 1, x2, y2, usFillColorLight);
	ColorFillVideoSurfaceArea(ButtonDestBuffer, x + 1, y + 1, x2 - 1, y2 - 1, usFillColorBack);

	UINT16 const usStrWidth = StringPixLength(str, font);
	if( usStrWidth > w )
	{ //the string is too long, so use the wrapped method
		y += 1;
		DisplayWrappedString(x, y, w, 2, font, FONT_BLACK, str, FONT_BLACK, CENTER_JUSTIFIED | MARK_DIRTY);
		return;
	}
	//center the string vertically and horizontally.
	SetFontAttributes(font, FONT_BLACK, NO_SHADOW);
	x += (w - StringPixLength(str, font)) / 2;
	y += (h - GetFontHeight(font)) / 2;
	MPrint(x, y, str);
	InvalidateRegion( x, y, x2, y2 );
}

void ClickEditorButton( INT32 iEditorButtonID )
{
	if ( iEditorButtonID < 0 || iEditorButtonID >= NUMBER_EDITOR_BUTTONS )
		return;
	GUIButtonRef const b = iEditorButton[iEditorButtonID];
	if (b) b->uiFlags |= BUTTON_CLICKED_ON;
}

void UnclickEditorButton( INT32 iEditorButtonID )
{
	if ( iEditorButtonID < 0 || iEditorButtonID >= NUMBER_EDITOR_BUTTONS )
		return;
	GUIButtonRef const b = iEditorButton[iEditorButtonID];
	if (b) b->uiFlags &= ~BUTTON_CLICKED_ON;
}

void HideEditorButton( INT32 iEditorButtonID )
{
	HideButton( iEditorButton[ iEditorButtonID ] );
}

void ShowEditorButton( INT32 iEditorButtonID )
{
	ShowButton( iEditorButton[ iEditorButtonID ] );
}

void DisableEditorButton( INT32 iEditorButtonID )
{
	DisableButton( iEditorButton[ iEditorButtonID ] );
}

void EnableEditorButton( INT32 iEditorButtonID )
{
	EnableButton( iEditorButton[ iEditorButtonID ] );
}


void UnclickEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID )
{
	INT32 i;
	for( i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++ )
	{
		GUIButtonRef const b = iEditorButton[i];
		Assert( b );
		b->uiFlags &= (~BUTTON_CLICKED_ON);
	}
}

void HideEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID )
{
	INT32 i;
	for( i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++ )
		HideButton( iEditorButton[ i ] );
}

void ShowEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID )
{
	INT32 i;
	for( i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++ )
		ShowButton( iEditorButton[ i ] );
}

void DisableEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID )
{
	INT32 i;
	for( i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++ )
		DisableButton( iEditorButton[ i ] );
}

void EnableEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID )
{
	INT32 i;
	for( i = iFirstEditorButtonID; i <= iLastEditorButtonID; i++ )
		EnableButton( iEditorButton[ i ] );
}


static void RenderEntryPoint(INT16 const gridno, wchar_t const* const label)
{
	if (gridno == -1) return;
	INT16 x;
	INT16 y;
	GetGridNoScreenPos(gridno, 0, &x, &y);
	if (x < -40 || SCREEN_WIDTH <= x || y < -20 || TASKBAR_Y - 20 <= y) return;
	DisplayWrappedString(x, y - 5, 40, 2, FONT10ARIAL, FONT_YELLOW, label, FONT_BLACK, CENTER_JUSTIFIED | MARK_DIRTY);
}


static void RenderMapEntryPointsAndLights(void)
{
	if (gfSummaryWindowActive) return;

	SetFontShadow(FONT_NEARBLACK);
	RenderEntryPoint(gMapInformation.sNorthGridNo,    L"North Entry Point");
	RenderEntryPoint(gMapInformation.sWestGridNo,     L"West Entry Point");
	RenderEntryPoint(gMapInformation.sEastGridNo,     L"East Entry Point");
	RenderEntryPoint(gMapInformation.sSouthGridNo,    L"South Entry Point");
	RenderEntryPoint(gMapInformation.sCenterGridNo,   L"Center Entry Point");
	RenderEntryPoint(gMapInformation.sIsolatedGridNo, L"Isolated Entry Point");

	//Do the lights now.
	CFOR_EACH_LIGHT_SPRITE(l)
	{
		INT16       x;
		INT16       y;
		INT16 const gridno = l->iY * WORLD_COLS + l->iX;
		GetGridNoScreenPos(gridno, 0, &x, &y);
		if (x < -40 || SCREEN_WIDTH <= x || y < -50 || TASKBAR_Y - 60 <= y) continue;

		UINT8          colour;
		wchar_t const* text;
		if (l->uiFlags & LIGHT_PRIMETIME)
		{
			colour = FONT_ORANGE;
			text   = L"Prime";
		}
		else if (l->uiFlags & LIGHT_NIGHTTIME)
		{
			colour = FONT_RED;
			text   = L"Night";
		}
		else
		{
			colour = FONT_YELLOW;
			text   = L"24Hour";
		}
		DisplayWrappedString(x, y - 5, 50, 2, FONT10ARIAL, colour, text, FONT_BLACK, CENTER_JUSTIFIED | MARK_DIRTY);
	}
}


static wchar_t const* BuildTriggerName(OBJECTTYPE const& o, wchar_t* const buf, size_t const length)
{
	if (o.usItem == SWITCH)
	{
		switch (o.bFrequency)
		{
			case PANIC_FREQUENCY:   return L"Panic Trigger1";
			case PANIC_FREQUENCY_2: return L"Panic Trigger2";
			case PANIC_FREQUENCY_3: return L"Panic Trigger3";
			default: swprintf(buf, length, L"Trigger%d", o.bFrequency - 50); break;
		}
	}
	else
	{ // Action item
		if (o.bDetonatorType == BOMB_PRESSURE)
		{
			return L"Pressure Action";
		}
		else switch (o.bFrequency)
		{
			case PANIC_FREQUENCY:   return L"Panic Action1";
			case PANIC_FREQUENCY_2: return L"Panic Action2";
			case PANIC_FREQUENCY_3: return L"Panic Action3";
			default: swprintf(buf, length, L"Action%d", o.bFrequency - 50); break;
		}
	}
	return buf;
}


static void RenderDoorLockInfo()
{
	wchar_t str[50];
	FOR_EACH_DOOR(i)
	{
		DOOR const& d = *i;

		INT16 screen_x;
		INT16 screen_y;
		GetGridNoScreenPos(d.sGridNo, 0, &screen_x, &screen_y);
		INT16 const x = screen_x;
		INT16 const y = screen_y;
		if (y > 390) continue;

		if (d.ubLockID != 255)
			swprintf(str, lengthof(str), L"%hs", LockTable[d.ubLockID].ubEditorName);
		else
			wcslcpy(str, L"No Lock ID", lengthof(str));
		DisplayWrappedString(x - 10, y - 40, 60, 2, FONT10ARIAL, FONT_LTKHAKI, str, FONT_BLACK, CENTER_JUSTIFIED | MARK_DIRTY);

		wchar_t const* trap_type; // HACK000E
		switch (d.ubTrapID)
		{
			case NO_TRAP:        continue;
			case EXPLOSION:      trap_type = L"Explosion Trap";      break;
			case ELECTRIC:       trap_type = L"Electric Trap";       break;
			case SIREN:          trap_type = L"Siren Trap";          break;
			case SILENT_ALARM:   trap_type = L"Silent Alarm";        break;
			case BROTHEL_SIREN:  trap_type = L"Brothel Siren Trap";  break;
			case SUPER_ELECTRIC: trap_type = L"Super Electric Trap"; break;

			default: abort(); // HACK000E
		}
		SetFontAttributes(FONT10ARIAL, FONT_RED);
		MPrint(x + 20 - StringPixLength(trap_type, FONT10ARIAL) / 2, y, trap_type);
		swprintf(str, lengthof(str), L"Trap Level %d", d.ubTrapLevel);
		MPrint(x + 20 - StringPixLength(str, FONT10ARIAL) / 2, y + 10, str);
	}
}


static void RenderSelectedItemBlownUp(void)
{
	INT16 screen_x;
	INT16 screen_y;
	GetGridNoScreenPos(gsItemGridNo, 0, &screen_x, &screen_y);
	if (screen_y > (EDITOR_TASKBAR_POS_Y - 20)) return;

	OBJECTTYPE const& o    = *gpItem;
	const ItemModel * item = GCM->getItem(o.usItem);
	INT16             x;
	INT16             y;

	// Display the enlarged item graphic
	SGPVObject  const& vo = GetInterfaceGraphicForItem(item);
	ETRLEObject const& e  = vo.SubregionProperties(item->getGraphicNum());
	x = screen_x - e.sOffsetX + (40 - e.usWidth)  / 2;
	y = screen_y - e.sOffsetY + (20 - e.usHeight) / 2;
	BltVideoObjectOutline(FRAME_BUFFER, &vo, item->getGraphicNum(), x, y, Get16BPPColor(FROMRGB(0, 140, 170)));

	// Display the item name above it
	SetFontAttributes(FONT10ARIAL, FONT_YELLOW);
	wchar_t const* item_name;
	wchar_t        buf[SIZE_ITEM_NAME];
	if (o.usItem == ACTION_ITEM || o.usItem == SWITCH)
	{
		item_name = BuildTriggerName(o, buf, lengthof(buf));
	}
	else if (item->getItemClass() == IC_KEY)
	{
		swprintf(buf, lengthof(buf), L"%hs", LockTable[o.ubKeyID].ubEditorName);
		item_name = buf;
	}
	else
	{
		item_name = ItemNames[o.usItem];
	}
	x  = screen_x - (StringPixLength(item_name, FONT10ARIAL) - 40) / 2;
	y -= 10;
	MPrint(x, y, item_name);

	if (o.usItem == ACTION_ITEM)
	{
		SetFont(FONT10ARIALBOLD);
		SetFontForeground(FONT_LTKHAKI);
		wchar_t const* const name = GetActionItemName(&o);
		x  = screen_x - (StringPixLength(name, FONT10ARIALBOLD) - 40) / 2;
		y += 10;
		MPrint(x, y, name);
		SetFontForeground(FONT_YELLOW);
	}

	// Count the number of items in the current pool, and display that.
	INT32 n = 0;
	for (ITEM_POOL const* i = GetItemPool(gsItemGridNo, 0); i; i = i->pNext)
	{
		++n;
	}
	mprintf(screen_x, screen_y + 10, L"%d", n);

	// If the item is hidden, render a blinking H (just like DG)
	WORLDITEM const& wi = GetWorldItem(gpItemPool->iItemIndex);
	if (wi.bVisible == HIDDEN_ITEM || wi.bVisible == BURIED)
	{
		SetFont(FONT10ARIALBOLD);
		if (GetJA2Clock() % 1000 > 500) SetFontForeground(249);
		MPrint(screen_x + 16, screen_y + 7, L"H");
		InvalidateRegion(screen_x + 16, screen_y + 7, screen_x + 24, screen_y + 27);
	}
}


static void RenderEditorInfo(void)
{
	wchar_t					FPSText[ 50 ];

	SetFontAttributes(FONT12POINT1, FONT_BLACK, NO_SHADOW);

	//Display the mapindex position
	const GridNo iMapIndex = GetMouseMapPos();
	if (iMapIndex != NOWHERE)
		swprintf(FPSText, lengthof(FPSText), L"   (%d)   ", iMapIndex);
	else
		swprintf(FPSText, lengthof(FPSText), L"          ");
	mprintfEditor(50 - StringPixLength(FPSText, FONT12POINT1) / 2, 103, FPSText);

	switch( iCurrentTaskbar )
	{
		case TASK_OPTIONS:
			if( !gfWorldLoaded || giCurrentTilesetID < 0 )
				MPrint(260, EDITOR_TASKBAR_POS_Y + 85, L"No map currently loaded.");
			else
				mprintf(260, EDITOR_TASKBAR_POS_Y + 85, L"File:  %hs, Current Tileset:  %ls", g_filename, gTilesets[giCurrentTilesetID].zName);
			break;
		case TASK_TERRAIN:
			if( gusSelectionType == LINESELECTION )
				swprintf(SelTypeWidth, lengthof(SelTypeWidth), L"Width: %d", gusSelectionWidth );
			DrawEditorInfoBox(wszSelType[gusSelectionType], FONT12POINT1, 220, 70, 60, 30);
			swprintf(FPSText, lengthof(FPSText), L"%d%%", gusSelectionDensity);
			DrawEditorInfoBox(FPSText, FONT12POINT1, 310, 70, 40, 30);
			break;
		case TASK_ITEMS:
			RenderEditorItemsInfo();
			UpdateItemStatsPanel();
			break;
		case TASK_BUILDINGS:
			UpdateBuildingsInfo();
			if( gusSelectionType == LINESELECTION )
				swprintf(SelTypeWidth, lengthof(SelTypeWidth), L"Width: %d", gusSelectionWidth );
			DrawEditorInfoBox(wszSelType[gusSelectionType], FONT12POINT1, 530, 70, 60, 30);
			break;
		case TASK_MERCS:
			UpdateMercsInfo();
			break;
		case TASK_MAPINFO:
			UpdateMapInfo();
			if( gusSelectionType == LINESELECTION )
				swprintf(SelTypeWidth, lengthof(SelTypeWidth), L"Width: %d", gusSelectionWidth );
			DrawEditorInfoBox(wszSelType[gusSelectionType], FONT12POINT1, 450, 70, 60, 30);
			break;
		default:
			break;
	}
}

//This is in ButtonSystem.c as a hack.  Because we need to save the buffer whenever we do a full
//taskbar render, we need to draw the buttons without hilites, hence this flag.  This flag is
//always true in ButtonSystem.c, so it won't effect anything else.
extern BOOLEAN gfGotoGridNoUI;

void ProcessEditorRendering()
{
	BOOLEAN fSaveBuffer = FALSE;
	if( gfRenderTaskbar ) //do a full taskbar render.
	{
		ClearTaskbarRegion(0, 0, SCREEN_WIDTH, EDITOR_TASKBAR_HEIGHT);
		RenderTerrainTileButtons();
		MarkButtonsDirty();
		gfRenderTaskbar = FALSE;
		fSaveBuffer = TRUE;
		gfRenderDrawingMode = TRUE;
		gfRenderHilights = FALSE;
		gfRenderMercInfo = TRUE;
	}
	if( gfRenderDrawingMode )
	{
		if( iCurrentTaskbar == TASK_BUILDINGS || iCurrentTaskbar == TASK_TERRAIN || iCurrentTaskbar == TASK_ITEMS )
		{
			ShowCurrentDrawingMode();
			gfRenderDrawingMode = FALSE;
		}
	}
	//render dynamically changed buttons only
	RenderButtons( );

	if( gfSummaryWindowActive )
		RenderSummaryWindow();
	else if( !gfGotoGridNoUI && !InOverheadMap() )
		RenderMercStrings();

	if( gfEditingDoor )
		RenderDoorEditingWindow();

	if( TextInputMode() )
		RenderAllTextFields();
	RenderEditorInfo();

	if( !gfSummaryWindowActive && !gfGotoGridNoUI && !InOverheadMap() )
	{
		if( gpItem && gsItemGridNo != -1 )
			RenderSelectedItemBlownUp();
		if( iCurrentTaskbar == TASK_MAPINFO )
			RenderMapEntryPointsAndLights();
		if( (iDrawMode == DRAW_MODE_PLACE_ITEM && eInfo.uiItemType == TBAR_MODE_ITEM_KEYS) ||
			iDrawMode == DRAW_MODE_DOORKEYS )
		{
			RenderDoorLockInfo();
		}
	}



	if( fSaveBuffer )
		BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, EDITOR_TASKBAR_POS_Y, SCREEN_WIDTH, EDITOR_TASKBAR_HEIGHT);

	//Make sure this is TRUE at all times.
	//It is set to false when before we save the buffer, so the buttons don't get
	//rendered with hilites, in case the mouse is over one.
	gfRenderHilights = TRUE;

	RenderButtonsFastHelp();

}


