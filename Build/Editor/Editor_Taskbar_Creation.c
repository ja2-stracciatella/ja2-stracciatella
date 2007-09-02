#include "BuildDefines.h"

#ifdef JA2EDITOR

//sgp
#include "Button_System.h"
#include "Font.h"
#include "Font_Control.h"
#include "Debug.h"
//editor
#include "EditorDefines.h"
#include "Editor_Callback_Prototypes.h"
#include "Editor_Taskbar_Utils.h"
#include "EditorMercs.h"
#include "EditorMapInfo.h"
//tactical
#include "Soldier_Create.h"
#include "Overhead_Types.h"


static void InitEditorItemStatsButtons(void)
{
	iEditorButton[ITEMSTATS_PANEL] = CreateLabel(NULL, 0, 0, 0, 480, 361, 160, 99, MSYS_PRIORITY_NORMAL);
	iEditorButton[ ITEMSTATS_HIDDEN_BTN ] =
		CreateCheckBoxButton(	485, 365, "EDITOR/SmCheckbox.sti", MSYS_PRIORITY_NORMAL, ItemStatsToggleHideCallback );
	iEditorButton[ITEMSTATS_DELETE_BTN] = CreateTextButton(L"Delete", FONT10ARIAL, FONT_RED, FONT_BLACK, 600, 441, 36, 16, MSYS_PRIORITY_NORMAL + 1, DEFAULT_MOVE_CALLBACK, ItemStatsDeleteCallback);
}


static void MakeButton(UINT idx, INT16 x, INT16 y, GUI_CALLBACK click, const char* gfx, const wchar_t* help)
{
	INT32 btn = CreateEasyButton(x, y, gfx, click);
	iEditorButton[idx] = btn;
	SetButtonFastHelpText(btn, help);
}


static void MakeCheck(UINT idx, INT16 x, INT16 y, GUI_CALLBACK click, const char* gfx, const wchar_t* help)
{
	INT32 btn = CreateCheckBoxButton(x, y, gfx, MSYS_PRIORITY_NORMAL, click);
	iEditorButton[idx] = btn;
	SetButtonFastHelpText(btn, help);
}


static void MakeButtonTeam(UINT idx, INT16 y, GUI_CALLBACK click, const wchar_t* text)
{
	INT32 btn = CreateTextButton(text, BLOCKFONT, 165, FONT_BLACK, 20, y, 78, 19, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, click);
	iEditorButton[idx] = btn;
	SpecifyButtonDownTextColors(btn, FONT_YELLOW, FONT_BLACK);
}


static void MakeButtonEquipment(UINT idx, UINT level, INT16 colour, const wchar_t* text)
{
	INT32 btn = CreateTextButton(text, SMALLCOMPFONT, FONT_GRAY1, FONT_BLACK, 480, 385 + 15 * level, 40, 15, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetRelativeEquipmentCallback);
	iEditorButton[idx] = btn;
	SpecifyButtonDownTextColors(btn, colour, FONT_BLACK);
	MSYS_SetBtnUserData(btn, level);
}


static void MakeButtonAttribute(UINT idx, UINT level, INT16 colour, const wchar_t* text)
{
	INT32 btn = CreateTextButton(text, SMALLCOMPFONT, FONT_GRAY1, FONT_BLACK, 530, 385 + 15 * level, 40, 15, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetRelativeAttributesCallback);
	iEditorButton[idx] = btn;
	SpecifyButtonDownTextColors(btn, colour, FONT_BLACK);
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

	INT32 btn = CreateIconButton(giEditMercDirectionIcons[0], (dir + 1) % 8, x, y, 30, 30, MSYS_PRIORITY_NORMAL + 1, MercsDirectionSetCallback);
	iEditorButton[idx] = btn;
	wchar_t str[30];
	swprintf(str, lengthof(str), L"Set merc to face %ls", FaceDirs[dir]);
	SetButtonFastHelpText(btn, str);
	MSYS_SetBtnUserData(  btn, dir);
}


static void MakeButtonSchedule(UINT idx, INT16 x, INT16 y, INT16 w, INT16 h, GUI_CALLBACK click, const wchar_t* text)
{
	iEditorButton[idx] = CreateTextButton(text, FONT10ARIAL, FONT_YELLOW, FONT_BLACK, x, y, w, h, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, click);
}


static void InitEditorMercsToolbar(void)
{
	MakeCheck(MERCS_PLAYERTOGGLE,   4, 362, MercsTogglePlayers,   "EDITOR/SmCheckbox.sti", L"Toggle viewing of players");
	MakeCheck(MERCS_ENEMYTOGGLE,    4, 382, MercsToggleEnemies,   "EDITOR/SmCheckbox.sti", L"Toggle viewing of enemies");
	MakeCheck(MERCS_CREATURETOGGLE, 4, 402, MercsToggleCreatures, "EDITOR/SmCheckbox.sti", L"Toggle viewing of creatures");
	MakeCheck(MERCS_REBELTOGGLE,    4, 422, MercsToggleRebels,    "EDITOR/SmCheckbox.sti", L"Toggle viewing of rebels");
	MakeCheck(MERCS_CIVILIANTOGGLE, 4, 442, MercsToggleCivilians, "EDITOR/SmCheckbox.sti", L"Toggle viewing of civilians");
	if (gfShowPlayers)   ClickEditorButton(MERCS_PLAYERTOGGLE);
	if (gfShowEnemies)   ClickEditorButton(MERCS_ENEMYTOGGLE);
	if (gfShowCreatures) ClickEditorButton(MERCS_CREATURETOGGLE);
	if (gfShowRebels)    ClickEditorButton(MERCS_REBELTOGGLE);
	if (gfShowCivilians) ClickEditorButton(MERCS_CIVILIANTOGGLE);
	DisableButton(iEditorButton[MERCS_PLAYERTOGGLE]);

	MakeButtonTeam(MERCS_PLAYER,   362, MercsPlayerTeamCallback,   L"Player");
	DisableButton(iEditorButton[MERCS_PLAYER]);
	MakeButtonTeam(MERCS_ENEMY,    382, MercsEnemyTeamCallback,    L"Enemy");
	MakeButtonTeam(MERCS_CREATURE, 402, MercsCreatureTeamCallback, L"Creature");
	MakeButtonTeam(MERCS_REBEL,    422, MercsRebelTeamCallback,    L"Rebels");
	MakeButtonTeam(MERCS_CIVILIAN, 442, MercsCivilianTeamCallback, L"Civilian");

	iEditorButton[MERCS_1] = CreateLabel(L"DETAILED PLACEMENT", SMALLCOMPFONT, FONT_ORANGE, 60, 100, 362, 68, 20, MSYS_PRIORITY_NORMAL);
	SpecifyButtonTextOffsets( iEditorButton[ MERCS_1 ], 20, 4, FALSE );
	SpecifyButtonTextWrappedWidth( iEditorButton[ MERCS_1 ], 46 );
	iEditorButton[ MERCS_DETAILEDCHECKBOX ] =
		CreateCheckBoxButton(	103, 365, "EDITOR/checkbox.sti", MSYS_PRIORITY_NORMAL, MercsDetailedPlacementCallback );

	MakeButton(MERCS_GENERAL,      100, 382, MercsGeneralModeCallback,    "EDITOR/MercGeneral.sti",      L"General information mode");
	MakeButton(MERCS_APPEARANCE,   134, 382, MercsAppearanceModeCallback, "EDITOR/MercAppearance.sti",   L"Physical appearance mode");
	MakeButton(MERCS_ATTRIBUTES,   100, 408, MercsAttributesModeCallback, "EDITOR/MercAttributes.sti",   L"Attributes mode");
	MakeButton(MERCS_INVENTORY,    134, 408, MercsInventoryModeCallback,  "EDITOR/MercInventory.sti",    L"Inventory mode");
	MakeButton(MERCS_PROFILE,      100, 434, MercsProfileModeCallback,    "EDITOR/MercProfile.sti",      L"Profile ID mode");
	MakeButton(MERCS_SCHEDULE,     134, 434, MercsScheduleModeCallback,   "EDITOR/MercSchedule.sti",     L"Schedule mode");
	//Workaround for identical buttons.
	ButtonList[iEditorButton[MERCS_SCHEDULE]]->uiFlags |= BUTTON_NO_DUPLICATE;
	MakeButton(MERCS_GLOWSCHEDULE, 134, 434, MercsScheduleModeCallback,   "EDITOR/MercGlowSchedule.sti", L"Schedule mode");
	HideEditorButton( MERCS_GLOWSCHEDULE );

	iEditorButton[MERCS_DELETE] = CreateTextButton(L"DELETE", SMALLCOMPFONT, FONT_DKBLUE, FONT_BLACK, 600, 362, 40, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsDeleteCallback);
	SetButtonFastHelpText( iEditorButton[ MERCS_DELETE ], L"Delete currently selected merc (DEL).");
	iEditorButton[ MERCS_NEXT ] = CreateTextButton(L"NEXT",   SMALLCOMPFONT, FONT_DKBLUE, FONT_BLACK, 600, 382, 40, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsNextCallback);
	SetButtonFastHelpText( iEditorButton[ MERCS_NEXT ], L"Find next merc (SPACE).");

	//Priority Existance
	MakeCheck(MERCS_PRIORITYEXISTANCE_CHECKBOX, 170, 365, MercsPriorityExistanceCallback, "EDITOR/checkbox.sti", L"Toggle priority existance");

	//If merc has keys
	MakeCheck(MERCS_HASKEYS_CHECKBOX,           170, 390, MercsHasKeysCallback,           "EDITOR/checkbox.sti", L"Toggle whether or not placement has/naccess to all doors.");

	//Orders
	iEditorButton[MERCS_ORDERS_STATIONARY]  = CreateTextButton(L"STATIONARY",    SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 200, 368, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_ONGUARD]     = CreateTextButton(L"ON GUARD",      SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 200, 380, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_ONCALL]      = CreateTextButton(L"ON CALL",       SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 200, 392, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_SEEKENEMY]   = CreateTextButton(L"SEEK ENEMY",    SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 200, 404, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_CLOSEPATROL] = CreateTextButton(L"CLOSE PATROL",  SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 270, 368, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_FARPATROL]   = CreateTextButton(L"FAR PATROL",    SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 270, 380, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_POINTPATROL] = CreateTextButton(L"POINT PATROL",  SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 270, 392, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	iEditorButton[MERCS_ORDERS_RNDPTPATROL] = CreateTextButton(L"RND PT PATROL", SMALLCOMPFONT, FONT_GRAY2, FONT_BLACK, 270, 404, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetOrdersCallback);
	for (INT32 x = 0; x < 8; x++)
	{
		MSYS_SetBtnUserData(iEditorButton[FIRST_MERCS_ORDERS_BUTTON + x], x);
	}

	//Attitudes
	iEditorButton[MERCS_ATTITUDE_DEFENSIVE]   = CreateTextButton(L"DEFENSIVE",    SMALLCOMPFONT, FONT_GRAY4, FONT_BLACK, 200, 424, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_BRAVESOLO]   = CreateTextButton(L"BRAVE SOLO",   SMALLCOMPFONT, FONT_GRAY4, FONT_BLACK, 200, 436, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_BRAVEAID]    = CreateTextButton(L"BRAVE AID",    SMALLCOMPFONT, FONT_GRAY4, FONT_BLACK, 200, 448, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_AGGRESSIVE]  = CreateTextButton(L"AGGRESSIVE",   SMALLCOMPFONT, FONT_GRAY4, FONT_BLACK, 270, 424, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_CUNNINGSOLO] = CreateTextButton(L"CUNNING SOLO", SMALLCOMPFONT, FONT_GRAY4, FONT_BLACK, 270, 436, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetAttitudeCallback);
	iEditorButton[MERCS_ATTITUDE_CUNNINGAID]  = CreateTextButton(L"CUNNING AID",  SMALLCOMPFONT, FONT_GRAY4, FONT_BLACK, 270, 448, 70, 12, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsSetAttitudeCallback);
	for (INT32 x = 0; x < 6; x++)
	{
		MSYS_SetBtnUserData(iEditorButton[FIRST_MERCS_ATTITUDE_BUTTON + x], x);
	}

	MakeButtonDir(MERCS_DIRECTION_N,  0, 420, 365);
	MakeButtonDir(MERCS_DIRECTION_NE, 1, 420, 395);
	MakeButtonDir(MERCS_DIRECTION_E,  2, 420, 425);
	MakeButtonDir(MERCS_DIRECTION_SE, 3, 390, 425);
	MakeButtonDir(MERCS_DIRECTION_S,  4, 360, 425);
	MakeButtonDir(MERCS_DIRECTION_SW, 5, 360, 395);
	MakeButtonDir(MERCS_DIRECTION_W,  6, 360, 365);
	MakeButtonDir(MERCS_DIRECTION_NW, 7, 390, 365);

	iEditorButton[MERCS_DIRECTION_FIND] = CreateTextButton(L"Find", FONT12POINT1, FONT_MCOLOR_BLACK, FONT_BLACK, 390, 395, 30, 30, MSYS_PRIORITY_NORMAL + 1, DEFAULT_MOVE_CALLBACK, MercsFindSelectedMercCallback);
	SetButtonFastHelpText( iEditorButton[ MERCS_DIRECTION_FIND] , L"Find selected merc" );

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

	iEditorButton[ MERCS_ARMY_CODE ] =
		CreateCheckBoxButton(	575, 410, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MercsSetEnemyColorCodeCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_ARMY_CODE], SOLDIER_CLASS_ARMY);
	iEditorButton[ MERCS_ADMIN_CODE ] =
		CreateCheckBoxButton(	575, 424, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MercsSetEnemyColorCodeCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_ADMIN_CODE], SOLDIER_CLASS_ADMINISTRATOR);
	iEditorButton[ MERCS_ELITE_CODE ] =
		CreateCheckBoxButton(	575, 438, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MercsSetEnemyColorCodeCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_ELITE_CODE], SOLDIER_CLASS_ELITE);

	iEditorButton[MERCS_CIVILIAN_GROUP] = CreateTextButton(gszCivGroupNames[0], SMALLCOMPFONT, FONT_YELLOW, FONT_BLACK, 574, 410, 60, 25, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, MercsCivilianGroupCallback);

	iEditorButton[MERCS_TOGGLECOLOR_BUTTON] = CreateCheckBoxButton(180, 364, "EDITOR/checkbox.sti", MSYS_PRIORITY_NORMAL, MercsToggleColorModeCallback);
	iEditorButton[MERCS_HAIRCOLOR_DOWN] = CreateEasyButton(200, 364, "EDITOR/leftarrow.sti",  MercsSetColorsCallback);
	iEditorButton[MERCS_HAIRCOLOR_UP]   = CreateEasyButton(360, 364, "EDITOR/rightarrow.sti", MercsSetColorsCallback);
	iEditorButton[MERCS_SKINCOLOR_DOWN] = CreateEasyButton(200, 388, "EDITOR/leftarrow.sti",  MercsSetColorsCallback);
	iEditorButton[MERCS_SKINCOLOR_UP]   = CreateEasyButton(360, 388, "EDITOR/rightarrow.sti", MercsSetColorsCallback);
	iEditorButton[MERCS_VESTCOLOR_DOWN] = CreateEasyButton(200, 412, "EDITOR/leftarrow.sti",  MercsSetColorsCallback);
	iEditorButton[MERCS_VESTCOLOR_UP]   = CreateEasyButton(360, 412, "EDITOR/rightarrow.sti", MercsSetColorsCallback);
	iEditorButton[MERCS_PANTCOLOR_DOWN] = CreateEasyButton(200, 436, "EDITOR/leftarrow.sti",  MercsSetColorsCallback);
	iEditorButton[MERCS_PANTCOLOR_UP]   = CreateEasyButton(360, 436, "EDITOR/rightarrow.sti", MercsSetColorsCallback);
	for (INT32 x = FIRST_MERCS_COLOR_BUTTON; x < LAST_MERCS_COLOR_BUTTON; x += 2)
	{
		SetButtonFastHelpText(iEditorButton[x    ], L"Previous color set");
		SetButtonFastHelpText(iEditorButton[x + 1], L"Next color set");
		DisableButton(iEditorButton[x    ]);
		DisableButton(iEditorButton[x + 1]);
	}

	MakeButton(MERCS_BODYTYPE_DOWN, 460, 364, MercsSetBodyTypeCallback, "EDITOR/leftarrow.sti",  L"Previous body type");
	MakeButton(MERCS_BODYTYPE_UP,   560, 364, MercsSetBodyTypeCallback, "EDITOR/rightarrow.sti", L"Next body type");

	MakeCheck(MERCS_SCHEDULE_VARIANCE1, 309, 375, MercsScheduleToggleVariance1Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");
	MakeCheck(MERCS_SCHEDULE_VARIANCE2, 309, 396, MercsScheduleToggleVariance2Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");
	MakeCheck(MERCS_SCHEDULE_VARIANCE3, 309, 417, MercsScheduleToggleVariance3Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");
	MakeCheck(MERCS_SCHEDULE_VARIANCE4, 309, 438, MercsScheduleToggleVariance4Callback, "EDITOR/SmCheckBox.sti", L"Toggle time variance (+ or - 15 minutes)");

	MakeButtonSchedule(MERCS_SCHEDULE_ACTION1, 186, 373, 77, 16, MercsScheduleAction1Callback, L"No action");
	MakeButtonSchedule(MERCS_SCHEDULE_ACTION2, 186, 394, 77, 16, MercsScheduleAction2Callback, L"No action");
	MakeButtonSchedule(MERCS_SCHEDULE_ACTION3, 186, 415, 77, 16, MercsScheduleAction3Callback, L"No action");
	MakeButtonSchedule(MERCS_SCHEDULE_ACTION4, 186, 436, 77, 16, MercsScheduleAction4Callback, L"No action");

	MakeButtonSchedule(MERCS_SCHEDULE_DATA1A, 331, 373, 40, 16, MercsScheduleData1ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA1B, 381, 373, 40, 16, MercsScheduleData1BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA2A, 331, 394, 40, 16, MercsScheduleData2ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA2B, 381, 394, 40, 16, MercsScheduleData2BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA3A, 331, 415, 40, 16, MercsScheduleData3ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA3B, 381, 415, 40, 16, MercsScheduleData3BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA4A, 331, 436, 40, 16, MercsScheduleData4ACallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_DATA4B, 381, 436, 40, 16, MercsScheduleData4BCallback, L"");
	MakeButtonSchedule(MERCS_SCHEDULE_CLEAR,  516, 362, 77, 16, MercsScheduleClearCallback,  L"Clear Schedule");
	HideEditorButtons(MERCS_SCHEDULE_DATA1A, MERCS_SCHEDULE_DATA4B);

	iEditorButton[ MERCS_HEAD_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+61, MERCPANEL_Y+0, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_HEAD_SLOT], HELMETPOS);
	iEditorButton[ MERCS_BODY_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+61, MERCPANEL_Y+22, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_BODY_SLOT], VESTPOS);
	iEditorButton[ MERCS_LEGS_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+62, MERCPANEL_Y+73, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_LEGS_SLOT], LEGPOS);
	iEditorButton[ MERCS_LEFTHAND_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+12, MERCPANEL_Y+43, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_LEFTHAND_SLOT], HANDPOS);
	iEditorButton[ MERCS_RIGHTHAND_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+90, MERCPANEL_Y+42, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_RIGHTHAND_SLOT], SECONDHANDPOS);
	iEditorButton[ MERCS_PACK1_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+166, MERCPANEL_Y+6, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_PACK1_SLOT], BIGPOCK1POS);
	iEditorButton[ MERCS_PACK2_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+166, MERCPANEL_Y+29, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_PACK2_SLOT], BIGPOCK2POS);
	iEditorButton[ MERCS_PACK3_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+166, MERCPANEL_Y+52, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_PACK3_SLOT], BIGPOCK3POS);
	iEditorButton[ MERCS_PACK4_SLOT ] =
		CreateCheckBoxButton(	MERCPANEL_X+166, MERCPANEL_Y+75, "EDITOR/smCheckbox.sti", MSYS_PRIORITY_NORMAL+1, MercsInventorySlotCallback );
	MSYS_SetBtnUserData(iEditorButton[MERCS_PACK4_SLOT], BIGPOCK4POS);
}


static void InitEditorBuildingsToolbar(void)
{
	iEditorButton[BUILDING_TOGGLE_ROOF_VIEW] = CreateTextButton(L"ROOFS",     SMALLCOMPFONT, FONT_YELLOW, FONT_BLACK, 110, 400, 50, 15, MSYS_PRIORITY_NORMAL, BUTTON_NO_CALLBACK, BuildingToggleRoofViewCallback);
	iEditorButton[BUILDING_TOGGLE_WALL_VIEW] = CreateTextButton(L"WALLS",     SMALLCOMPFONT, FONT_YELLOW, FONT_BLACK, 110, 415, 50, 15, MSYS_PRIORITY_NORMAL, BUTTON_NO_CALLBACK, BuildingToggleWallViewCallback);
	iEditorButton[BUILDING_TOGGLE_INFO_VIEW] = CreateTextButton(L"ROOM INFO", SMALLCOMPFONT, FONT_YELLOW, FONT_BLACK, 110, 430, 50, 15, MSYS_PRIORITY_NORMAL, BUTTON_NO_CALLBACK, BuildingToggleInfoViewCallback);

	//Selection method buttons
	MakeButton(BUILDING_PLACE_WALLS,        180, 370, BuildingWallCallback,           "EDITOR/wall.sti",         L"Place walls using selection method\n");
	MakeButton(BUILDING_PLACE_DOORS,        210, 370, BuildingDoorCallback,           "EDITOR/door.sti",         L"Place doors using selection method");
	MakeButton(BUILDING_PLACE_ROOFS,        240, 370, BuildingRoofCallback,           "EDITOR/roof.sti",         L"Place roofs using selection method");
	MakeButton(BUILDING_PLACE_WINDOWS,      180, 400, BuildingWindowCallback,         "EDITOR/window.sti",       L"Place windows using selection method");
	MakeButton(BUILDING_PLACE_BROKEN_WALLS, 210, 400, BuildingCrackWallCallback,      "EDITOR/crackwall.sti",    L"Place damaged walls using selection method.");
	MakeButton(BUILDING_PLACE_FURNITURE,    240, 400, BuildingFurnitureCallback,      "EDITOR/decor.sti",        L"Place furniture using selection method");
	MakeButton(BUILDING_PLACE_DECALS,       180, 430, BuildingDecalCallback,          "EDITOR/decal.sti",        L"Place wall decals using selection method");
	MakeButton(BUILDING_PLACE_FLOORS,       210, 430, BuildingFloorCallback,          "EDITOR/floor.sti",        L"Place floors using selection method");
	MakeButton(BUILDING_PLACE_TOILETS,      240, 430, BuildingToiletCallback,         "EDITOR/toilet.sti",       L"Place generic furniture using selection method");

	//Smart method buttons
	MakeButton(BUILDING_SMART_WALLS,        290, 370, BuildingSmartWallCallback,      "EDITOR/wall.sti",         L"Place walls using smart method");
	MakeButton(BUILDING_SMART_DOORS,        320, 370, BuildingSmartDoorCallback,      "EDITOR/door.sti",         L"Place doors using smart method");
	MakeButton(BUILDING_SMART_WINDOWS,      290, 400, BuildingSmartWindowCallback,    "EDITOR/window.sti",       L"Place windows using smart method");
	MakeButton(BUILDING_SMART_BROKEN_WALLS, 320, 400, BuildingSmartCrackWallCallback, "EDITOR/crackwall.sti",    L"Place damaged walls using smart method");
	MakeButton(BUILDING_DOORKEY,            290, 430, BuildingDoorKeyCallback,        "EDITOR/key.sti",          L"Lock or trap existing doors" );

	MakeButton(BUILDING_NEW_ROOM,           370, 370, BuildingNewRoomCallback,        "EDITOR/newroom.sti",      L"Add a new room");
	MakeButton(BUILDING_CAVE_DRAWING,       370, 370, BuildingCaveDrawingCallback,    "EDITOR/caves.sti",        L"Edit cave walls.");
	MakeButton(BUILDING_SAW_ROOM,           370, 400, BuildingSawRoomCallback,        "EDITOR/sawroom.sti",      L"Remove an area from existing building.");
	MakeButton(BUILDING_KILL_BUILDING,      370, 430, BuildingKillBuildingCallback,   "EDITOR/delroom.sti",      L"Remove a building");
	MakeButton(BUILDING_NEW_ROOF,           400, 430, BuildingNewRoofCallback,        "EDITOR/newroof.sti",      L"Add/replace building's roof with new flat roof.");
	MakeButton(BUILDING_COPY_BUILDING,      430, 430, BuildingCopyBuildingCallback,   "EDITOR/copyroom.sti",     L"Copy a building");
	MakeButton(BUILDING_MOVE_BUILDING,      460, 430, BuildingMoveBuildingCallback,   "EDITOR/moveroom.sti",     L"Move a building");
	MakeButton(BUILDING_DRAW_ROOMNUM,       410, 370, BuildingDrawRoomNumCallback,    "EDITOR/addTileRoom.sti",  L"Draw room number");
	MakeButton(BUILDING_ERASE_ROOMNUM,      440, 370, BuildingEraseRoomNumCallback,   "EDITOR/killTileRoom.sti", L"Erase room numbers");

	MakeButton(BUILDING_TOGGLE_ERASEMODE,   500, 400, BtnEraseCallback,               "EDITOR/eraser.sti",       L"Toggle erase mode");

	MakeButton(BUILDING_UNDO,            530, 400, BtnUndoCallback,  "EDITOR/undo.sti",  L"Undo last change");
	MakeButton(BUILDING_CYCLE_BRUSHSIZE, 500, 430, BtnBrushCallback, "EDITOR/paint.sti", L"Cycle brush size");
}


static void InitEditorItemsToolbar(void)
{
	SetFontForeground( FONT_MCOLOR_LTRED );
	iEditorButton[ITEMS_WEAPONS]    = CreateTextButton(L"Weapons",    BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 100, 440, 59, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsWeaponsCallback);
	iEditorButton[ITEMS_AMMO]       = CreateTextButton(L"Ammo",       BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 159, 440, 40, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsAmmoCallback);
	iEditorButton[ITEMS_ARMOUR]     = CreateTextButton(L"Armour",     BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 199, 440, 52, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsArmourCallback);
	iEditorButton[ITEMS_EXPLOSIVES] = CreateTextButton(L"Explosives", BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 251, 440, 69, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsExplosivesCallback);
	iEditorButton[ITEMS_EQUIPMENT1] = CreateTextButton(L"E1",         BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 320, 440, 21, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsEquipment1Callback);
	iEditorButton[ITEMS_EQUIPMENT2] = CreateTextButton(L"E2",         BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 341, 440, 21, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsEquipment2Callback);
	iEditorButton[ITEMS_EQUIPMENT3] = CreateTextButton(L"E3",         BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 362, 440, 21, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsEquipment3Callback);
	iEditorButton[ITEMS_TRIGGERS]   = CreateTextButton( L"Triggers",  BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 383, 440, 59, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsTriggersCallback);
	iEditorButton[ITEMS_KEYS]       = CreateTextButton( L"Keys",      BLOCKFONT, FONT_MCOLOR_DKWHITE, FONT_BLACK, 442, 440, 38, 20, MSYS_PRIORITY_NORMAL, DEFAULT_MOVE_CALLBACK, ItemsKeysCallback);

	iEditorButton[ITEMS_LEFTSCROLL]  = CreateEasyButton( 1, 361, "EDITOR/leftscroll.sti",  ItemsLeftScrollCallback);
	iEditorButton[ITEMS_RIGHTSCROLL] = CreateEasyButton(50, 361, "EDITOR/rightscroll.sti", ItemsRightScrollCallback);
}


static void InitEditorMapInfoToolbar(void)
{
	MakeButton(MAPINFO_ADD_LIGHT1_SOURCE, 10, 362, BtnDrawLightsCallback, "EDITOR/light.sti", L"Add ambient light source");

	iEditorButton[MAPINFO_LIGHT_PANEL] = CreateLabel(NULL, 0, 0, 0, 45, 362, 60, 50, MSYS_PRIORITY_NORMAL);
	iEditorButton[ MAPINFO_PRIMETIME_LIGHT ] =
		CreateCheckBoxButton(	48, 365, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MapInfoPrimeTimeRadioCallback );
	iEditorButton[ MAPINFO_NIGHTTIME_LIGHT ] =
		CreateCheckBoxButton(	48, 380, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MapInfoNightTimeRadioCallback );
	iEditorButton[ MAPINFO_24HOUR_LIGHT ] =
		CreateCheckBoxButton(	48, 395, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MapInfo24HourTimeRadioCallback );
	ClickEditorButton( gbDefaultLightType + MAPINFO_PRIMETIME_LIGHT );

	MakeButton(MAPINFO_TOGGLE_FAKE_LIGHTS, 120, 362, BtnFakeLightCallback, "EDITOR/fakelight.sti", L"Toggle fake ambient lights.");

	iEditorButton[MAPINFO_RADIO_PANEL] = CreateLabel(NULL, 0, 0, 0, 207, 362, 70, 50, MSYS_PRIORITY_NORMAL);
	iEditorButton[ MAPINFO_RADIO_NORMAL ] =
		CreateCheckBoxButton(	210, 365, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MapInfoNormalRadioCallback );
	iEditorButton[ MAPINFO_RADIO_BASEMENT ] =
		CreateCheckBoxButton(	210, 380, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MapInfoBasementRadioCallback );
	iEditorButton[ MAPINFO_RADIO_CAVES ] =
		CreateCheckBoxButton(	210, 395, "EDITOR/radiobutton.sti", MSYS_PRIORITY_NORMAL, MapInfoCavesRadioCallback );

	MakeButton(MAPINFO_DRAW_EXITGRIDS,   305, 372, MapInfoDrawExitGridCallback, "EDITOR/exitgridbut.sti", L"Add exit grids (r-clk to query existing).");
	MakeButton(MAPINFO_CYCLE_BRUSHSIZE,  420, 430, BtnBrushCallback,            "EDITOR/paint.sti",       L"Cycle brush size");
	MakeButton(MAPINFO_UNDO,             510, 430, BtnUndoCallback,             "EDITOR/undo.sti",        L"Undo last change");
	MakeButton(MAPINFO_TOGGLE_ERASEMODE, 540, 430, BtnEraseCallback,            "EDITOR/eraser.sti",      L"Toggle erase mode");
	MakeButton(MAPINFO_NORTH_POINT,      540, 365, MapInfoEntryPointsCallback,  "EDITOR/north.sti",       L"Specify north point for validation purposes.");
	MakeButton(MAPINFO_WEST_POINT,       525, 386, MapInfoEntryPointsCallback,  "EDITOR/west.sti",        L"Specify west point for validation purposes.");
	MakeButton(MAPINFO_EAST_POINT,       555, 386, MapInfoEntryPointsCallback,  "EDITOR/east.sti",        L"Specify east point for validation purposes.");
	MakeButton(MAPINFO_SOUTH_POINT,      540, 407, MapInfoEntryPointsCallback,  "EDITOR/south.sti",       L"Specify south point for validation purposes.");
	MakeButton(MAPINFO_CENTER_POINT,     590, 375, MapInfoEntryPointsCallback,  "EDITOR/center.sti",      L"Specify center point for validation purposes.");
	MakeButton(MAPINFO_ISOLATED_POINT,   590, 396, MapInfoEntryPointsCallback,  "EDITOR/isolated.sti",    L"Specify isolated point for validation purposes.");
}


static void InitEditorOptionsToolbar(void)
{
	MakeButton(OPTIONS_NEW_MAP,         71, 401, BtnNewMapCallback,        "EDITOR/new.sti",     L"New map");
	MakeButton(OPTIONS_NEW_BASEMENT,   101, 401, BtnNewBasementCallback,   "EDITOR/new.sti",     L"New basement");
	MakeButton(OPTIONS_NEW_CAVES,      131, 401, BtnNewCavesCallback,      "EDITOR/new.sti",     L"New cave level");
	MakeButton(OPTIONS_SAVE_MAP,       161, 401, BtnSaveCallback,          "EDITOR/save.sti",    L"Save map");
	MakeButton(OPTIONS_LOAD_MAP,       191, 401, BtnLoadCallback,          "EDITOR/load.sti",    L"Load map");
	MakeButton(OPTIONS_CHANGE_TILESET, 221, 401, BtnChangeTilesetCallback, "EDITOR/tileset.sti", L"Select tileset");
	MakeButton(OPTIONS_LEAVE_EDITOR,   251, 401, BtnCancelCallback,        "EDITOR/cancel.sti",  L"Leave Editor mode");
	MakeButton(OPTIONS_QUIT_GAME,      281, 401, BtnQuitCallback,          "EDITOR/cancel.sti",  L"Exit game.");
}


static void InitEditorTerrainToolbar(void)
{
	MakeButton(TERRAIN_FGROUND_TEXTURES, 100, 400, BtnFgGrndCallback,          "EDITOR/downgrid.sti",  L"Draw ground textures");
	MakeButton(TERRAIN_BGROUND_TEXTURES, 130, 400, BtnBkGrndCallback,          "EDITOR/upgrid.sti",    L"Set map ground textures");
	MakeButton(TERRAIN_PLACE_CLIFFS,     160, 400, BtnBanksCallback,           "EDITOR/banks.sti",     L"Place banks and cliffs");
	MakeButton(TERRAIN_PLACE_ROADS,      190, 400, BtnRoadsCallback,           "EDITOR/road.sti",      L"Draw roads");
	MakeButton(TERRAIN_PLACE_DEBRIS,     220, 400, BtnDebrisCallback,          "EDITOR/debris.sti",    L"Draw debris");
	MakeButton(TERRAIN_PLACE_TREES,      250, 400, BtnObjectCallback,          "EDITOR/tree.sti",      L"Place trees & bushes");
	MakeButton(TERRAIN_PLACE_ROCKS,      280, 400, BtnObject1Callback,         "EDITOR/num1.sti",      L"Place rocks");
	MakeButton(TERRAIN_PLACE_MISC,       310, 400, BtnObject2Callback,         "EDITOR/num2.sti",      L"Place barrels & other junk");
	MakeButton(TERRAIN_FILL_AREA,        100, 430, BtnFillCallback,            "EDITOR/fill.sti",      L"Fill area");
	MakeButton(TERRAIN_UNDO,             130, 430, BtnUndoCallback,            "EDITOR/undo.sti",      L"Undo last change");
	MakeButton(TERRAIN_TOGGLE_ERASEMODE, 160, 430, BtnEraseCallback,           "EDITOR/eraser.sti",    L"Toggle erase mode");
	MakeButton(TERRAIN_CYCLE_BRUSHSIZE,  190, 430, BtnBrushCallback,           "EDITOR/paint.sti",     L"Cycle brush size");
	MakeButton(TERRAIN_RAISE_DENSITY,    280, 430, BtnIncBrushDensityCallback, "EDITOR/uparrow.sti",   L"Raise brush density");
	MakeButton(TERRAIN_LOWER_DENSITY,    350, 430, BtnDecBrushDensityCallback, "EDITOR/downarrow.sti", L"Lower brush density");
}


static void MakeButtonTab(UINT idx, INT16 x, GUI_CALLBACK click, const wchar_t* text)
{
	INT32 btn = CreateTextButton(text, SMALLFONT1, FONT_LTKHAKI, FONT_DKKHAKI, x, 460, 90, 20, MSYS_PRIORITY_HIGH, BUTTON_NO_CALLBACK, click);
	iEditorButton[idx] = btn;
	SpecifyButtonDownTextColors(btn, FONT_YELLOW, FONT_ORANGE);
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
