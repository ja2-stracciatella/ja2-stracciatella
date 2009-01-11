#ifdef JA2EDITOR

//sgp
#include "Button_System.h"
#include "Font.h"
#include "Font_Control.h"
//editor
#include "EditorDefines.h"
#include "Editor_Callback_Prototypes.h"
#include "Editor_Taskbar_Creation.h"
#include "Editor_Taskbar_Utils.h"
#include "EditorMercs.h"
#include "EditorMapInfo.h"
//tactical
#include "Soldier_Control.h"
#include "Soldier_Create.h"
#include "Overhead_Types.h"


static void InitEditorItemStatsButtons(void)
{
	INT16 const y = TASKBAR_Y;
	iEditorButton[ITEMSTATS_PANEL]      = CreateLabel(NULL, 0, 0, 0, 480, y + 1, 160, 99, MSYS_PRIORITY_NORMAL);
	iEditorButton[ITEMSTATS_HIDDEN_BTN] = CreateCheckBoxButton(485, y + 5, "EDITOR/SmCheckbox.sti", MSYS_PRIORITY_NORMAL, ItemStatsToggleHideCallback);
	iEditorButton[ITEMSTATS_DELETE_BTN] = CreateTextButton(L"Delete", FONT10ARIAL, FONT_RED, FONT_BLACK, 600, y + 81, 36, 16, MSYS_PRIORITY_NORMAL + 1, ItemStatsDeleteCallback);
}


static void MakeButton(UINT idx, INT16 x, INT16 y, GUI_CALLBACK click, const char* gfx, const wchar_t* help)
{
	GUIButtonRef const btn = QuickCreateButtonImg(gfx, -1, 1, 2, 3, 4, x, TASKBAR_Y + y, MSYS_PRIORITY_NORMAL, click);
	iEditorButton[idx] = btn;
	btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);
	btn->SetFastHelpText(help);
}


static void MakeCheck(UINT idx, INT16 x, INT16 y, GUI_CALLBACK click, const char* gfx, const wchar_t* help)
{
	GUIButtonRef const btn = CreateCheckBoxButton(x, TASKBAR_Y + y, gfx, MSYS_PRIORITY_NORMAL, click);
	iEditorButton[idx] = btn;
	btn->SetFastHelpText(help);
}


static GUIButtonRef MakeRadio(INT16 const x, INT16 const y, GUI_CALLBACK const click)
{
	return CreateCheckBoxButton(x, TASKBAR_Y + y, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, click);
}


static void MakeButtonTeam(UINT idx, INT16 y, GUI_CALLBACK click, const wchar_t* text)
{
	GUIButtonRef const btn = CreateTextButton(text, BLOCKFONT, 165, FONT_BLACK, 20, TASKBAR_Y + y, 78, 19, MSYS_PRIORITY_NORMAL, click);
	iEditorButton[idx] = btn;
	btn->SpecifyDownTextColors(FONT_YELLOW, FONT_BLACK);
}


static GUIButtonRef MakeTextButton(wchar_t const* const text, INT16 const fore_colour, INT16 const x, INT16 const y, INT16 const w, INT16 const h, GUI_CALLBACK const click)
{
	return CreateTextButton(text, SMALLCOMPFONT, fore_colour, FONT_BLACK, x, TASKBAR_Y + y, w, h, MSYS_PRIORITY_NORMAL, click);
}


static void MakeButtonEquipment(UINT idx, UINT level, INT16 colour, const wchar_t* text)
{
	GUIButtonRef const btn = MakeTextButton(text, FONT_GRAY1, 480, 20 + 15 * level, 40, 15, MercsSetRelativeEquipmentCallback);
	iEditorButton[idx] = btn;
	btn->SpecifyDownTextColors(colour, FONT_BLACK);
	MSYS_SetBtnUserData(btn, level);
}


static void MakeButtonAttribute(UINT idx, UINT level, INT16 colour, const wchar_t* text)
{
	GUIButtonRef const btn = MakeTextButton(text, FONT_GRAY1, 530, TASKBAR_Y + 20 + 15 * level, 40, 15, MercsSetRelativeAttributesCallback);
	iEditorButton[idx] = btn;
	btn->SpecifyDownTextColors(colour, FONT_BLACK);
	MSYS_SetBtnUserData(btn, level);
}


static void MakeButtonDir(UINT idx, UINT dir, INT16 x, INT16 y)
{
	const wchar_t* const FaceDirs[] = {
		L"north",
		L"northeast",
		L"east",
		L"southeast",
		L"south",
		L"southwest",
		L"west",
		L"northwest"
	};

	GUIButtonRef const btn = CreateIconButton(giEditMercDirectionIcons[0], (dir + 1) % 8, x, TASKBAR_Y + y, 30, 30, MSYS_PRIORITY_NORMAL + 1, MercsDirectionSetCallback);
	iEditorButton[idx] = btn;
	wchar_t str[30];
	swprintf(str, lengthof(str), L"Set merc to face %ls", FaceDirs[dir]);
	btn->SetFastHelpText(str);
	MSYS_SetBtnUserData(  btn, dir);
}


static void MakeButtonRank(UINT idx, INT16 y, INT32 rank)
{
	GUIButtonRef const btn = MakeRadio(575, y, MercsSetEnemyColorCodeCallback);
	iEditorButton[idx] = btn;
	MSYS_SetBtnUserData(btn, rank);
}


static void MakeButtonSchedule(UINT idx, INT16 x, INT16 y, INT16 w, INT16 h, GUI_CALLBACK click, const wchar_t* text)
{
	iEditorButton[idx] = CreateTextButton(text, FONT10ARIAL, FONT_YELLOW, FONT_BLACK, x, TASKBAR_Y + y, w, h, MSYS_PRIORITY_NORMAL, click);
}


static void MakeButtonInventory(UINT idx, INT16 x, INT16 y, INT32 pos)
{
	GUIButtonRef const btn = CreateCheckBoxButton(x, y, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL + 1, MercsInventorySlotCallback);
	iEditorButton[idx] = btn;
	MSYS_SetBtnUserData(btn, pos);
}


static void InitEditorMercsToolbar(void)
{
	MakeCheck(MERCS_PLAYERTOGGLE,   4,  2, MercsTogglePlayers,   "EDITOR/SmCheckbox.sti", L"Toggle viewing of players");
	MakeCheck(MERCS_ENEMYTOGGLE,    4, 22, MercsToggleEnemies,   "EDITOR/SmCheckbox.sti", L"Toggle viewing of enemies");
	MakeCheck(MERCS_CREATURETOGGLE, 4, 42, MercsToggleCreatures, "EDITOR/SmCheckbox.sti", L"Toggle viewing of creatures");
	MakeCheck(MERCS_REBELTOGGLE,    4, 62, MercsToggleRebels,    "EDITOR/SmCheckbox.sti", L"Toggle viewing of rebels");
	MakeCheck(MERCS_CIVILIANTOGGLE, 4, 82, MercsToggleCivilians, "EDITOR/SmCheckbox.sti", L"Toggle viewing of civilians");
	if (gfShowPlayers)   ClickEditorButton(MERCS_PLAYERTOGGLE);
	if (gfShowEnemies)   ClickEditorButton(MERCS_ENEMYTOGGLE);
	if (gfShowCreatures) ClickEditorButton(MERCS_CREATURETOGGLE);
	if (gfShowRebels)    ClickEditorButton(MERCS_REBELTOGGLE);
	if (gfShowCivilians) ClickEditorButton(MERCS_CIVILIANTOGGLE);
	DisableButton(iEditorButton[MERCS_PLAYERTOGGLE]);

	MakeButtonTeam(MERCS_PLAYER,    2, MercsPlayerTeamCallback,   L"Player");
	DisableButton(iEditorButton[MERCS_PLAYER]);
	MakeButtonTeam(MERCS_ENEMY,    22, MercsEnemyTeamCallback,    L"Enemy");
	MakeButtonTeam(MERCS_CREATURE, 42, MercsCreatureTeamCallback, L"Creature");
	MakeButtonTeam(MERCS_REBEL,    62, MercsRebelTeamCallback,    L"Rebels");
	MakeButtonTeam(MERCS_CIVILIAN, 82, MercsCivilianTeamCallback, L"Civilian");

	INT16 const y = TASKBAR_Y;

	iEditorButton[MERCS_1] = CreateLabel(L"DETAILED PLACEMENT", SMALLCOMPFONT, FONT_ORANGE, 60, 100, y + 2, 68, 20, MSYS_PRIORITY_NORMAL);
	iEditorButton[MERCS_1]->SpecifyTextOffsets(20, 4, FALSE);
	iEditorButton[MERCS_1]->SpecifyTextWrappedWidth(46);
	iEditorButton[MERCS_DETAILEDCHECKBOX] = CreateCheckBoxButton(103, y + 5, "EDITOR/checkbox.sti", MSYS_PRIORITY_NORMAL, MercsDetailedPlacementCallback);

	MakeButton(MERCS_GENERAL,      100, 22, MercsGeneralModeCallback,    "EDITOR/MercGeneral.sti",      L"General information mode");
	MakeButton(MERCS_APPEARANCE,   134, 22, MercsAppearanceModeCallback, "EDITOR/MercAppearance.sti",   L"Physical appearance mode");
	MakeButton(MERCS_ATTRIBUTES,   100, 48, MercsAttributesModeCallback, "EDITOR/MercAttributes.sti",   L"Attributes mode");
	MakeButton(MERCS_INVENTORY,    134, 48, MercsInventoryModeCallback,  "EDITOR/MercInventory.sti",    L"Inventory mode");
	MakeButton(MERCS_PROFILE,      100, 74, MercsProfileModeCallback,    "EDITOR/MercProfile.sti",      L"Profile ID mode");
	MakeButton(MERCS_SCHEDULE,     134, 74, MercsScheduleModeCallback,   "EDITOR/MercSchedule.sti",     L"Schedule mode");
	//Workaround for identical buttons.
	iEditorButton[MERCS_SCHEDULE]->uiFlags |= BUTTON_NO_DUPLICATE;
	MakeButton(MERCS_GLOWSCHEDULE, 134, 74, MercsScheduleModeCallback,   "EDITOR/MercGlowSchedule.sti", L"Schedule mode");
	HideEditorButton( MERCS_GLOWSCHEDULE );

	iEditorButton[MERCS_DELETE] = MakeTextButton(L"DELETE", FONT_DKBLUE, 600,  2, 40, 20, MercsDeleteCallback);
	iEditorButton[MERCS_DELETE]->SetFastHelpText(L"Delete currently selected merc (DEL).");
	iEditorButton[MERCS_NEXT]   = MakeTextButton(L"NEXT",   FONT_DKBLUE, 600, 22, 40, 20, MercsNextCallback);
	iEditorButton[MERCS_NEXT]->SetFastHelpText(L"Find next merc (SPACE).");

	//Priority Existance
	MakeCheck(MERCS_PRIORITYEXISTANCE_CHECKBOX, 170,  5, MercsPriorityExistanceCallback, "EDITOR/checkbox.sti", L"Toggle priority existance");

	//If merc has keys
	MakeCheck(MERCS_HASKEYS_CHECKBOX,           170, 30, MercsHasKeysCallback,           "EDITOR/checkbox.sti", L"Toggle whether or not placement has/naccess to all doors.");

	//Orders
	iEditorButton[MERCS_ORDERS_STATIONARY]  = MakeTextButton(L"STATIONARY",    FONT_GRAY2, 200,  8, 70, 12, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_ONGUARD]     = MakeTextButton(L"ON GUARD",      FONT_GRAY2, 200, 20, 70, 12, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_ONCALL]      = MakeTextButton(L"ON CALL",       FONT_GRAY2, 200, 32, 70, 12, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_SEEKENEMY]   = MakeTextButton(L"SEEK ENEMY",    FONT_GRAY2, 200, 44, 70, 12, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_CLOSEPATROL] = MakeTextButton(L"CLOSE PATROL",  FONT_GRAY2, 270,  8, 70, 12, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_FARPATROL]   = MakeTextButton(L"FAR PATROL",    FONT_GRAY2, 270, 20, 70, 12, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_POINTPATROL] = MakeTextButton(L"POINT PATROL",  FONT_GRAY2, 270, 32, 70, 12, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_RNDPTPATROL] = MakeTextButton(L"RND PT PATROL", FONT_GRAY2, 270, 44, 70, 12, MercsSetOrdersCallback);
	for (INT32 x = 0; x < 8; x++)
	{
		MSYS_SetBtnUserData(iEditorButton[FIRST_MERCS_ORDERS_BUTTON + x], x);
	}

	//Attitudes
	iEditorButton[MERCS_ATTITUDE_DEFENSIVE]   = MakeTextButton(L"DEFENSIVE",    FONT_GRAY4, 200, 64, 70, 12, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_BRAVESOLO]   = MakeTextButton(L"BRAVE SOLO",   FONT_GRAY4, 200, 76, 70, 12, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_BRAVEAID]    = MakeTextButton(L"BRAVE AID",    FONT_GRAY4, 200, 88, 70, 12, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_AGGRESSIVE]  = MakeTextButton(L"AGGRESSIVE",   FONT_GRAY4, 270, 64, 70, 12, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_CUNNINGSOLO] = MakeTextButton(L"CUNNING SOLO", FONT_GRAY4, 270, 76, 70, 12, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_CUNNINGAID]  = MakeTextButton(L"CUNNING AID",  FONT_GRAY4, 270, 88, 70, 12, MercsSetAttitudeCallback);
	for (INT32 x = 0; x < 6; x++)
	{
		MSYS_SetBtnUserData(iEditorButton[FIRST_MERCS_ATTITUDE_BUTTON + x], x);
	}

	MakeButtonDir(MERCS_DIRECTION_N,  0, 420,  5);
	MakeButtonDir(MERCS_DIRECTION_NE, 1, 420, 35);
	MakeButtonDir(MERCS_DIRECTION_E,  2, 420, 65);
	MakeButtonDir(MERCS_DIRECTION_SE, 3, 390, 65);
	MakeButtonDir(MERCS_DIRECTION_S,  4, 360, 65);
	MakeButtonDir(MERCS_DIRECTION_SW, 5, 360, 35);
	MakeButtonDir(MERCS_DIRECTION_W,  6, 360,  5);
	MakeButtonDir(MERCS_DIRECTION_NW, 7, 390,  5);

	iEditorButton[MERCS_DIRECTION_FIND] = CreateTextButton(L"Find", FONT12POINT1, FONT_MCOLOR_BLACK, FONT_BLACK, 390, y + 35, 30, 30, MSYS_PRIORITY_NORMAL + 1, MercsFindSelectedMercCallback);
	iEditorButton[MERCS_DIRECTION_FIND]->SetFastHelpText(L"Find selected merc");

	MakeButtonEquipment(MERCS_EQUIPMENT_BAD,     0, FONT_LTRED,   L"BAD");
	MakeButtonEquipment(MERCS_EQUIPMENT_POOR,    1, FONT_ORANGE,  L"POOR");
	MakeButtonEquipment(MERCS_EQUIPMENT_AVERAGE, 2, FONT_YELLOW,  L"AVERAGE");
	MakeButtonEquipment(MERCS_EQUIPMENT_GOOD,    3, FONT_LTGREEN, L"GOOD");
	MakeButtonEquipment(MERCS_EQUIPMENT_GREAT,   4, FONT_LTBLUE,  L"GREAT");

	MakeButtonAttribute(MERCS_ATTRIBUTES_BAD,     0, FONT_LTRED,   L"BAD");
	MakeButtonAttribute(MERCS_ATTRIBUTES_POOR,    1, FONT_ORANGE,  L"POOR");
	MakeButtonAttribute(MERCS_ATTRIBUTES_AVERAGE, 2, FONT_YELLOW,  L"AVERAGE");
	MakeButtonAttribute(MERCS_ATTRIBUTES_GOOD,    3, FONT_LTGREEN, L"GOOD");
	MakeButtonAttribute(MERCS_ATTRIBUTES_GREAT,   4, FONT_LTBLUE,  L"GREAT");

	MakeButtonRank(MERCS_ARMY_CODE,  50, SOLDIER_CLASS_ARMY);
	MakeButtonRank(MERCS_ADMIN_CODE, 64, SOLDIER_CLASS_ADMINISTRATOR);
	MakeButtonRank(MERCS_ELITE_CODE, 78, SOLDIER_CLASS_ELITE);

	iEditorButton[MERCS_CIVILIAN_GROUP] = MakeTextButton(gszCivGroupNames[0], FONT_YELLOW, 574, 50, 60, 25, MercsCivilianGroupCallback);

	iEditorButton[MERCS_TOGGLECOLOR_BUTTON] = CreateCheckBoxButton(180, y + 4, "EDITOR/checkbox.sti", MSYS_PRIORITY_NORMAL, MercsToggleColorModeCallback);
	for (INT32 i = 0; i != 4; ++i)
	{
		UINT  const idx = FIRST_MERCS_COLOR_BUTTON +  2 * i;
		INT16 const y   = 4                        + 24 * i;
		MakeButton(idx,     200, y, MercsSetColorsCallback, "EDITOR/leftarrow.sti",  L"Previous color set");
		MakeButton(idx + 1, 360, y, MercsSetColorsCallback, "EDITOR/rightarrow.sti", L"Next color set");
		DisableButton(iEditorButton[idx    ]);
		DisableButton(iEditorButton[idx + 1]);
	}

	MakeButton(MERCS_BODYTYPE_DOWN, 460, 4, MercsSetBodyTypeCallback, "EDITOR/leftarrow.sti",  L"Previous body type");
	MakeButton(MERCS_BODYTYPE_UP,   560, 4, MercsSetBodyTypeCallback, "EDITOR/rightarrow.sti", L"Next body type");

	MakeCheck(MERCS_SCHEDULE_VARIANCE1, 309, 15, MercsScheduleToggleVariance1Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");
	MakeCheck(MERCS_SCHEDULE_VARIANCE2, 309, 36, MercsScheduleToggleVariance2Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");
	MakeCheck(MERCS_SCHEDULE_VARIANCE3, 309, 57, MercsScheduleToggleVariance3Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");
	MakeCheck(MERCS_SCHEDULE_VARIANCE4, 309, 78, MercsScheduleToggleVariance4Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");

	MakeButtonSchedule(MERCS_SCHEDULE_ACTION1, 186, 13, 77, 16, MercsScheduleAction1Callback, L"No action");
	MakeButtonSchedule(MERCS_SCHEDULE_ACTION2, 186, 34, 77, 16, MercsScheduleAction2Callback, L"No action");
	MakeButtonSchedule(MERCS_SCHEDULE_ACTION3, 186, 55, 77, 16, MercsScheduleAction3Callback, L"No action");
	MakeButtonSchedule(MERCS_SCHEDULE_ACTION4, 186, 76, 77, 16, MercsScheduleAction4Callback, L"No action");

	MakeButtonSchedule(MERCS_SCHEDULE_DATA1A, 331, 13, 40, 16, MercsScheduleData1ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA1B, 381, 13, 40, 16, MercsScheduleData1BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA2A, 331, 34, 40, 16, MercsScheduleData2ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA2B, 381, 34, 40, 16, MercsScheduleData2BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA3A, 331, 55, 40, 16, MercsScheduleData3ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA3B, 381, 55, 40, 16, MercsScheduleData3BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA4A, 331, 76, 40, 16, MercsScheduleData4ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA4B, 381, 76, 40, 16, MercsScheduleData4BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_CLEAR,  516,  2, 77, 16, MercsScheduleClearCallback,  L"Clear Schedule");
	HideEditorButtons(MERCS_SCHEDULE_DATA1A, MERCS_SCHEDULE_DATA4B);

	MakeButtonInventory(MERCS_HEAD_SLOT,      MERCPANEL_X +  61, MERCPANEL_Y +  0, HELMETPOS);
	MakeButtonInventory(MERCS_BODY_SLOT,      MERCPANEL_X +  61, MERCPANEL_Y + 22, VESTPOS);
	MakeButtonInventory(MERCS_LEGS_SLOT,      MERCPANEL_X +  62, MERCPANEL_Y + 73, LEGPOS);
	MakeButtonInventory(MERCS_LEFTHAND_SLOT,  MERCPANEL_X +  12, MERCPANEL_Y + 43, HANDPOS);
	MakeButtonInventory(MERCS_RIGHTHAND_SLOT, MERCPANEL_X +  90, MERCPANEL_Y + 42, SECONDHANDPOS);
	MakeButtonInventory(MERCS_PACK1_SLOT,     MERCPANEL_X + 166, MERCPANEL_Y +  6, BIGPOCK1POS);
	MakeButtonInventory(MERCS_PACK2_SLOT,     MERCPANEL_X + 166, MERCPANEL_Y + 29, BIGPOCK2POS);
	MakeButtonInventory(MERCS_PACK3_SLOT,     MERCPANEL_X + 166, MERCPANEL_Y + 52, BIGPOCK3POS);
	MakeButtonInventory(MERCS_PACK4_SLOT,     MERCPANEL_X + 166, MERCPANEL_Y + 75, BIGPOCK4POS);
}


static void InitEditorBuildingsToolbar(void)
{
	iEditorButton[BUILDING_TOGGLE_ROOF_VIEW] = MakeTextButton(L"ROOFS",     FONT_YELLOW, 110, 40, 50, 15, BuildingToggleRoofViewCallback);
	iEditorButton[BUILDING_TOGGLE_WALL_VIEW] = MakeTextButton(L"WALLS",     FONT_YELLOW, 110, 55, 50, 15, BuildingToggleWallViewCallback);
	iEditorButton[BUILDING_TOGGLE_INFO_VIEW] = MakeTextButton(L"ROOM INFO", FONT_YELLOW, 110, 70, 50, 15, BuildingToggleInfoViewCallback);

	//Selection method buttons
	MakeButton(BUILDING_PLACE_WALLS,        180, 10, BuildingWallCallback,           "EDITOR/wall.sti",         L"Place walls using selection method");
	MakeButton(BUILDING_PLACE_DOORS,        210, 10, BuildingDoorCallback,           "EDITOR/door.sti",         L"Place doors using selection method");
	MakeButton(BUILDING_PLACE_ROOFS,        240, 10, BuildingRoofCallback,           "EDITOR/roof.sti",         L"Place roofs using selection method");
	MakeButton(BUILDING_PLACE_WINDOWS,      180, 40, BuildingWindowCallback,         "EDITOR/window.sti",       L"Place windows using selection method");
	MakeButton(BUILDING_PLACE_BROKEN_WALLS, 210, 40, BuildingCrackWallCallback,      "EDITOR/crackwall.sti",    L"Place damaged walls using selection method.");
	MakeButton(BUILDING_PLACE_FURNITURE,    240, 40, BuildingFurnitureCallback,      "EDITOR/decor.sti",        L"Place furniture using selection method");
	MakeButton(BUILDING_PLACE_DECALS,       180, 70, BuildingDecalCallback,          "EDITOR/decal.sti",        L"Place wall decals using selection method");
	MakeButton(BUILDING_PLACE_FLOORS,       210, 70, BuildingFloorCallback,          "EDITOR/floor.sti",        L"Place floors using selection method");
	MakeButton(BUILDING_PLACE_TOILETS,      240, 70, BuildingToiletCallback,         "EDITOR/toilet.sti",       L"Place generic furniture using selection method");

	//Smart method buttons
	MakeButton(BUILDING_SMART_WALLS,        290, 10, BuildingSmartWallCallback,      "EDITOR/wall.sti",         L"Place walls using smart method");
	MakeButton(BUILDING_SMART_DOORS,        320, 10, BuildingSmartDoorCallback,      "EDITOR/door.sti",         L"Place doors using smart method");
	MakeButton(BUILDING_SMART_WINDOWS,      290, 40, BuildingSmartWindowCallback,    "EDITOR/window.sti",       L"Place windows using smart method");
	MakeButton(BUILDING_SMART_BROKEN_WALLS, 320, 40, BuildingSmartCrackWallCallback, "EDITOR/crackwall.sti",    L"Place damaged walls using smart method");
	MakeButton(BUILDING_DOORKEY,            290, 70, BuildingDoorKeyCallback,        "EDITOR/key.sti",          L"Lock or trap existing doors" );

	MakeButton(BUILDING_NEW_ROOM,           370, 10, BuildingNewRoomCallback,        "EDITOR/newroom.sti",      L"Add a new room");
	MakeButton(BUILDING_CAVE_DRAWING,       370, 10, BuildingCaveDrawingCallback,    "EDITOR/caves.sti",        L"Edit cave walls.");
	MakeButton(BUILDING_SAW_ROOM,           370, 40, BuildingSawRoomCallback,        "EDITOR/sawroom.sti",      L"Remove an area from existing building.");
	MakeButton(BUILDING_KILL_BUILDING,      370, 70, BuildingKillBuildingCallback,   "EDITOR/delroom.sti",      L"Remove a building");
	MakeButton(BUILDING_NEW_ROOF,           400, 70, BuildingNewRoofCallback,        "EDITOR/newroof.sti",      L"Add/replace building's roof with new flat roof.");
	MakeButton(BUILDING_COPY_BUILDING,      430, 70, BuildingCopyBuildingCallback,   "EDITOR/copyroom.sti",     L"Copy a building");
	MakeButton(BUILDING_MOVE_BUILDING,      460, 70, BuildingMoveBuildingCallback,   "EDITOR/moveroom.sti",     L"Move a building");
	MakeButton(BUILDING_DRAW_ROOMNUM,       410, 10, BuildingDrawRoomNumCallback,    "EDITOR/addTileRoom.sti",  L"Draw room number");
	MakeButton(BUILDING_ERASE_ROOMNUM,      440, 10, BuildingEraseRoomNumCallback,   "EDITOR/killTileRoom.sti", L"Erase room numbers");

	MakeButton(BUILDING_TOGGLE_ERASEMODE,   500, 40, BtnEraseCallback,               "EDITOR/eraser.sti",       L"Toggle erase mode");

	MakeButton(BUILDING_UNDO,            530, 40, BtnUndoCallback,  "EDITOR/undo.sti",  L"Undo last change");
	MakeButton(BUILDING_CYCLE_BRUSHSIZE, 500, 70, BtnBrushCallback, "EDITOR/paint.sti", L"Cycle brush size");
}


static void InitEditorItemsToolbar(void)
{
	INT16 const y = TASKBAR_Y;
	iEditorButton[ITEMS_WEAPONS]    = CreateTextButton(L"Weapons",    BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 100, y + 80, 59, 20, MSYS_PRIORITY_NORMAL, ItemsWeaponsCallback);
	iEditorButton[ITEMS_AMMO]       = CreateTextButton(L"Ammo",       BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 159, y + 80, 40, 20, MSYS_PRIORITY_NORMAL, ItemsAmmoCallback);
	iEditorButton[ITEMS_ARMOUR]     = CreateTextButton(L"Armour",     BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 199, y + 80, 52, 20, MSYS_PRIORITY_NORMAL, ItemsArmourCallback);
	iEditorButton[ITEMS_EXPLOSIVES] = CreateTextButton(L"Explosives", BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 251, y + 80, 69, 20, MSYS_PRIORITY_NORMAL, ItemsExplosivesCallback);
	iEditorButton[ITEMS_EQUIPMENT1] = CreateTextButton(L"E1",         BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 320, y + 80, 21, 20, MSYS_PRIORITY_NORMAL, ItemsEquipment1Callback);
	iEditorButton[ITEMS_EQUIPMENT2] = CreateTextButton(L"E2",         BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 341, y + 80, 21, 20, MSYS_PRIORITY_NORMAL, ItemsEquipment2Callback);
	iEditorButton[ITEMS_EQUIPMENT3] = CreateTextButton(L"E3",         BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 362, y + 80, 21, 20, MSYS_PRIORITY_NORMAL, ItemsEquipment3Callback);
	iEditorButton[ITEMS_TRIGGERS]   = CreateTextButton( L"Triggers",  BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 383, y + 80, 59, 20, MSYS_PRIORITY_NORMAL, ItemsTriggersCallback);
	iEditorButton[ITEMS_KEYS]       = CreateTextButton( L"Keys",      BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 442, y + 80, 38, 20, MSYS_PRIORITY_NORMAL, ItemsKeysCallback);

	MakeButton(ITEMS_LEFTSCROLL,   1, 1, ItemsLeftScrollCallback,  "EDITOR/leftscroll.sti",  NULL);
	MakeButton(ITEMS_RIGHTSCROLL, 50, 1, ItemsRightScrollCallback, "EDITOR/rightscroll.sti", NULL);
}


static void InitEditorMapInfoToolbar(void)
{
	MakeButton(MAPINFO_ADD_LIGHT1_SOURCE, 10, 2, BtnDrawLightsCallback, "EDITOR/light.sti", L"Add ambient light source");

	INT16 const y = TASKBAR_Y;
	iEditorButton[MAPINFO_LIGHT_PANEL]     = CreateLabel(NULL, 0, 0, 0, 45, y + 2, 60, 50, MSYS_PRIORITY_NORMAL);
	iEditorButton[MAPINFO_PRIMETIME_LIGHT] = MakeRadio(48,  5, MapInfoPrimeTimeRadioCallback);
	iEditorButton[MAPINFO_NIGHTTIME_LIGHT] = MakeRadio(48, 20, MapInfoNightTimeRadioCallback);
	iEditorButton[MAPINFO_24HOUR_LIGHT]    = MakeRadio(48, 35, MapInfo24HourTimeRadioCallback);
	ClickEditorButton( gbDefaultLightType + MAPINFO_PRIMETIME_LIGHT );

	MakeButton(MAPINFO_TOGGLE_FAKE_LIGHTS, 120, 2, BtnFakeLightCallback, "EDITOR/fakelight.sti", L"Toggle fake ambient lights.");

	iEditorButton[MAPINFO_RADIO_PANEL]    = CreateLabel(NULL, 0, 0, 0, 207, y + 2, 70, 50, MSYS_PRIORITY_NORMAL);
	iEditorButton[MAPINFO_RADIO_NORMAL]   = MakeRadio(210,  5, MapInfoNormalRadioCallback);
	iEditorButton[MAPINFO_RADIO_BASEMENT] = MakeRadio(210, 20, MapInfoBasementRadioCallback);
	iEditorButton[MAPINFO_RADIO_CAVES]    = MakeRadio(210, 35, MapInfoCavesRadioCallback);

	MakeButton(MAPINFO_DRAW_EXITGRIDS,   305, 12, MapInfoDrawExitGridCallback, "EDITOR/exitgridbut.sti", L"Add exit grids (r-clk to query existing).");
	MakeButton(MAPINFO_CYCLE_BRUSHSIZE,  420, 70, BtnBrushCallback,            "EDITOR/paint.sti",       L"Cycle brush size");
	MakeButton(MAPINFO_UNDO,             510, 70, BtnUndoCallback,             "EDITOR/undo.sti",        L"Undo last change");
	MakeButton(MAPINFO_TOGGLE_ERASEMODE, 540, 70, BtnEraseCallback,            "EDITOR/eraser.sti",      L"Toggle erase mode");
	MakeButton(MAPINFO_NORTH_POINT,      540,  5, MapInfoEntryPointsCallback,  "EDITOR/north.sti",       L"Specify north point for validation purposes.");
	MakeButton(MAPINFO_WEST_POINT,       525, 26, MapInfoEntryPointsCallback,  "EDITOR/west.sti",        L"Specify west point for validation purposes.");
	MakeButton(MAPINFO_EAST_POINT,       555, 26, MapInfoEntryPointsCallback,  "EDITOR/east.sti",        L"Specify east point for validation purposes.");
	MakeButton(MAPINFO_SOUTH_POINT,      540, 47, MapInfoEntryPointsCallback,  "EDITOR/south.sti",       L"Specify south point for validation purposes.");
	MakeButton(MAPINFO_CENTER_POINT,     590, 15, MapInfoEntryPointsCallback,  "EDITOR/center.sti",      L"Specify center point for validation purposes.");
	MakeButton(MAPINFO_ISOLATED_POINT,   590, 36, MapInfoEntryPointsCallback,  "EDITOR/isolated.sti",    L"Specify isolated point for validation purposes.");
}


static void InitEditorOptionsToolbar(void)
{
	MakeButton(OPTIONS_NEW_MAP,         71, 41, BtnNewMapCallback,        "EDITOR/new.sti",     L"New map");
	MakeButton(OPTIONS_NEW_BASEMENT,   101, 41, BtnNewBasementCallback,   "EDITOR/new.sti",     L"New basement");
	MakeButton(OPTIONS_NEW_CAVES,      131, 41, BtnNewCavesCallback,      "EDITOR/new.sti",     L"New cave level");
	MakeButton(OPTIONS_SAVE_MAP,       161, 41, BtnSaveCallback,          "EDITOR/save.sti",    L"Save map");
	MakeButton(OPTIONS_LOAD_MAP,       191, 41, BtnLoadCallback,          "EDITOR/load.sti",    L"Load map");
	MakeButton(OPTIONS_CHANGE_TILESET, 221, 41, BtnChangeTilesetCallback, "EDITOR/tileset.sti", L"Select tileset");
	MakeButton(OPTIONS_LEAVE_EDITOR,   251, 41, BtnCancelCallback,        "EDITOR/cancel.sti",  L"Leave Editor mode");
	MakeButton(OPTIONS_QUIT_GAME,      281, 41, BtnQuitCallback,          "EDITOR/cancel.sti",  L"Exit game.");
}


static void InitEditorTerrainToolbar(void)
{
	MakeButton(TERRAIN_FGROUND_TEXTURES, 100, 40, BtnFgGrndCallback,          "EDITOR/downgrid.sti",  L"Draw ground textures");
	MakeButton(TERRAIN_BGROUND_TEXTURES, 130, 40, BtnBkGrndCallback,          "EDITOR/upgrid.sti",    L"Set map ground textures");
	MakeButton(TERRAIN_PLACE_CLIFFS,     160, 40, BtnBanksCallback,           "EDITOR/banks.sti",     L"Place banks and cliffs");
	MakeButton(TERRAIN_PLACE_ROADS,      190, 40, BtnRoadsCallback,           "EDITOR/road.sti",      L"Draw roads");
	MakeButton(TERRAIN_PLACE_DEBRIS,     220, 40, BtnDebrisCallback,          "EDITOR/debris.sti",    L"Draw debris");
	MakeButton(TERRAIN_PLACE_TREES,      250, 40, BtnObjectCallback,          "EDITOR/tree.sti",      L"Place trees & bushes");
	MakeButton(TERRAIN_PLACE_ROCKS,      280, 40, BtnObject1Callback,         "EDITOR/num1.sti",      L"Place rocks");
	MakeButton(TERRAIN_PLACE_MISC,       310, 40, BtnObject2Callback,         "EDITOR/num2.sti",      L"Place barrels & other junk");
	MakeButton(TERRAIN_FILL_AREA,        100, 70, BtnFillCallback,            "EDITOR/fill.sti",      L"Fill area");
	MakeButton(TERRAIN_UNDO,             130, 70, BtnUndoCallback,            "EDITOR/undo.sti",      L"Undo last change");
	MakeButton(TERRAIN_TOGGLE_ERASEMODE, 160, 70, BtnEraseCallback,           "EDITOR/eraser.sti",    L"Toggle erase mode");
	MakeButton(TERRAIN_CYCLE_BRUSHSIZE,  190, 70, BtnBrushCallback,           "EDITOR/paint.sti",     L"Cycle brush size");
	MakeButton(TERRAIN_RAISE_DENSITY,    280, 70, BtnIncBrushDensityCallback, "EDITOR/uparrow.sti",   L"Raise brush density");
	MakeButton(TERRAIN_LOWER_DENSITY,    350, 70, BtnDecBrushDensityCallback, "EDITOR/downarrow.sti", L"Lower brush density");
}


static void MakeButtonTab(UINT idx, INT16 x, GUI_CALLBACK click, const wchar_t* text)
{
	GUIButtonRef const btn = CreateTextButton(text, SMALLFONT1, FONT_LTKHAKI, FONT_DKKHAKI, x, TASKBAR_Y + 100, 90, 20, MSYS_PRIORITY_HIGH, click);
	iEditorButton[idx] = btn;
	btn->SpecifyDownTextColors(FONT_YELLOW, FONT_ORANGE);
}


void CreateEditorTaskbarInternal()
{
	//Create the tabs for the editor taskbar
	MakeButtonTab(TAB_TERRAIN,   100, TaskTerrainCallback,  L"Terrain");
	MakeButtonTab(TAB_BUILDINGS, 190, TaskBuildingCallback, L"Buildings");
	MakeButtonTab(TAB_ITEMS,     280, TaskItemsCallback,    L"Items");
	MakeButtonTab(TAB_MERCS,     370, TaskMercsCallback,    L"Mercs");
	MakeButtonTab(TAB_MAPINFO,   460, TaskMapInfoCallback,  L"Map Info");
	MakeButtonTab(TAB_OPTIONS,   550, TaskOptionsCallback,  L"Options");

	//Create the buttons within each tab.
	InitEditorTerrainToolbar();
	InitEditorBuildingsToolbar();
	InitEditorItemsToolbar();
	InitEditorMercsToolbar();
	InitEditorMapInfoToolbar();
	InitEditorOptionsToolbar();
	InitEditorItemStatsButtons();
}

#endif
