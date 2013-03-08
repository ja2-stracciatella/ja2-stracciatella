#include "Edit_Sys.h"
#include "MouseSystem.h"
#include "Line.h"
#include "Input.h"
#include "SysUtil.h"
#include "Font.h"
#include "Font_Control.h"
#include "EditScreen.h"
#include "SelectWin.h"
#include "Interface_Items.h"
#include "World_Items.h"
#include "Animation_Data.h"
#include "Lighting.h"
#include "EditorDefines.h"
#include "EditorMercs.h"
#include "EditorTerrain.h" //for access to TerrainTileDrawMode
#include "Soldier_Create.h" //The stuff that connects the editor generated information
#include "Soldier_Init_List.h"
#include "StrategicMap.h"
#include "Soldier_Add.h"
#include "Soldier_Control.h"
#include "Soldier_Profile_Type.h"
#include "Soldier_Profile.h"
#include "Text_Input.h"
#include "Random.h"
#include "EditorItems.h"
#include "Editor_Taskbar_Utils.h"
#include "Exit_Grids.h"
#include "Editor_Undo.h"
#include "Item_Statistics.h"
#include "Map_Information.h"
#include "EditorMapInfo.h"
#include "Environment.h"
#include "Simple_Render_Utils.h"


INT8 gbDefaultLightType = PRIMETIME_LIGHT;

SGPPaletteEntry	gEditorLightColor;

BOOLEAN gfEditorForceShadeTableRebuild = FALSE;

void SetupTextInputForMapInfo()
{
	wchar_t str[10];

	InitTextInputModeWithScheme( DEFAULT_SCHEME );

	AddUserInputField( NULL );  //just so we can use short cut keys while not typing.

	//light rgb fields
	swprintf(str, lengthof(str), L"%d", gEditorLightColor.r);
	AddTextInputField( 10, 394, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gEditorLightColor.g);
	AddTextInputField( 10, 414, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gEditorLightColor.b);
	AddTextInputField( 10, 434, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );

	swprintf(str, lengthof(str), L"%d", gsLightRadius);
	AddTextInputField( 120, 394, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gusLightLevel);
	AddTextInputField( 120, 414, 25, 18, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );

	//Scroll restriction ID
	swprintf(str, lengthof(str), L"%.d", gMapInformation.ubRestrictedScrollID);
	AddTextInputField( 210, 420, 30, 20, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );

	//exit grid input fields
	swprintf(str, lengthof(str), L"%c%d", gExitGrid.ubGotoSectorY + 'A' - 1, gExitGrid.ubGotoSectorX);
	AddTextInputField(338, 363, 30, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_COORDINATE);
	swprintf(str, lengthof(str), L"%d", gExitGrid.ubGotoSectorZ);
	AddTextInputField( 338, 383, 30, 18, MSYS_PRIORITY_NORMAL, str, 1, INPUTTYPE_NUMERICSTRICT );
	swprintf(str, lengthof(str), L"%d", gExitGrid.usGridNo);
	AddTextInputField( 338, 403, 40, 18, MSYS_PRIORITY_NORMAL, str, 5, INPUTTYPE_NUMERICSTRICT );
}

void UpdateMapInfo()
{
	SetFont( FONT10ARIAL );
	SetFontShadow( FONT_NEARBLACK );

	SetFontForeground( FONT_RED );
	MPrint( 38, 399, L"R");
	SetFontForeground( FONT_GREEN );
	MPrint( 38, 419, L"G");
	SetFontForeground( FONT_DKBLUE );
	MPrint( 38, 439, L"B");

	SetFontForeground( FONT_YELLOW );
	MPrint( 65, 369, L"Prime");
	MPrint( 65, 382, L"Night");
	MPrint( 65, 397, L"24Hrs");

	SetFontForeground( FONT_YELLOW );
	MPrint(148, 399, L"Radius");

	if( !gfBasement && !gfCaves )
		SetFontForeground( FONT_DKYELLOW );
	MPrint(148, 414, L"Underground");
	MPrint(148, 423, L"Light Level");

	SetFontForeground( FONT_YELLOW );
	MPrint(230, 369, L"Outdoors");
	MPrint(230, 384, L"Basement");
	MPrint(230, 399, L"Caves");

	SetFontForeground( FONT_ORANGE );
	MPrint(250, 420, L"Restricted");
	MPrint(250, 430, L"Scroll ID");

	SetFontForeground( FONT_YELLOW );
	MPrint(368, 363, L"Destination");
	MPrint(368, 372, L"Sector");
	MPrint(368, 383, L"Destination");
	MPrint(368, 392, L"Bsmt. Level");
	MPrint(378, 403, L"Dest.");
	MPrint(378, 412, L"GridNo");
}

void UpdateMapInfoFields()
{
	wchar_t str[10];
	//Update the text fields to reflect the validated values.
	//light rgb fields
	swprintf(str, lengthof(str), L"%d", gEditorLightColor.r);
	SetInputFieldStringWith16BitString( 1, str );
	swprintf(str, lengthof(str), L"%d", gEditorLightColor.g);
	SetInputFieldStringWith16BitString( 2, str );
	swprintf(str, lengthof(str), L"%d", gEditorLightColor.b);
	SetInputFieldStringWith16BitString( 3, str );

	swprintf(str, lengthof(str), L"%d", gsLightRadius);
	SetInputFieldStringWith16BitString( 4, str );
	swprintf(str, lengthof(str), L"%d", gusLightLevel);
	SetInputFieldStringWith16BitString( 5, str );

	swprintf(str, lengthof(str), L"%.d", gMapInformation.ubRestrictedScrollID);
	SetInputFieldStringWith16BitString( 6, str );

	ApplyNewExitGridValuesToTextFields();
}

void ExtractAndUpdateMapInfo()
{
	INT32 temp;
	BOOLEAN fUpdateLight1 = FALSE;
	//extract light1 colors
	temp = MIN( GetNumericStrictValueFromField( 1 ), 255 );
	if (temp != -1 && temp != gEditorLightColor.r)
	{
		fUpdateLight1 = TRUE;
		gEditorLightColor.r = (UINT8)temp;
	}
	temp = MIN( GetNumericStrictValueFromField( 2 ), 255 );
	if (temp != -1 && temp != gEditorLightColor.g)
	{
		fUpdateLight1 = TRUE;
		gEditorLightColor.g = (UINT8)temp;
	}
	temp = MIN( GetNumericStrictValueFromField( 3 ), 255 );
	if (temp != -1 && temp != gEditorLightColor.b)
	{
		fUpdateLight1 = TRUE;
		gEditorLightColor.b = (UINT8)temp;
	}
	if( fUpdateLight1 )
	{
		gfEditorForceShadeTableRebuild = TRUE;
		LightSetColor(&gEditorLightColor);
		gfEditorForceShadeTableRebuild = FALSE;
	}

	//extract radius
	temp = MAX( MIN( GetNumericStrictValueFromField( 4 ), 8 ), 1 );
	if( temp != -1 )
		gsLightRadius = (INT16)temp;
	temp = MAX( MIN( GetNumericStrictValueFromField( 5 ), 15 ), 1 );
	if( temp != -1 && temp != gusLightLevel )
	{
		gusLightLevel = (UINT16)temp;
		gfRenderWorld = TRUE;
		ubAmbientLightLevel = (UINT8)(EDITOR_LIGHT_MAX - gusLightLevel);
		LightSetBaseLevel( ubAmbientLightLevel );
		LightSpriteRenderAll();
	}

	temp = (INT8)GetNumericStrictValueFromField( 6 );
	gMapInformation.ubRestrictedScrollID = temp != -1 ? temp : 0;

	//set up fields for exitgrid information
	wchar_t const* const str = GetStringFromField(7);
	wchar_t row = str[0];
	if ('a' <= row && row <= 'z' ) row -= 32; //uppercase it!
	if ('A' <= row && row <= 'Z' && '0' <= str[1] && str[1] <= '9')
	{ //only update, if coordinate is valid.
		gExitGrid.ubGotoSectorY = (UINT8)(row    - 'A' + 1);
		gExitGrid.ubGotoSectorX = (UINT8)(str[1] - '0');
		if( str[2] >= '0' && str[2] <= '9' )
			gExitGrid.ubGotoSectorX = (UINT8)(gExitGrid.ubGotoSectorX * 10 + str[2] - '0' );
		gExitGrid.ubGotoSectorX = (UINT8)MAX( MIN( gExitGrid.ubGotoSectorX, 16 ), 1 );
		gExitGrid.ubGotoSectorY = (UINT8)MAX( MIN( gExitGrid.ubGotoSectorY, 16 ), 1 );
	}
	gExitGrid.ubGotoSectorZ    = (UINT8)MAX( MIN( GetNumericStrictValueFromField( 8 ), 3 ), 0 );
	gExitGrid.usGridNo					 = (UINT16)MAX( MIN( GetNumericStrictValueFromField( 9 ), 25600 ), 0 );

	UpdateMapInfoFields();
}

BOOLEAN ApplyNewExitGridValuesToTextFields()
{
	wchar_t str[10];
	//exit grid input fields
	if( iCurrentTaskbar != TASK_MAPINFO )
		return FALSE;
	swprintf(str, lengthof(str), L"%c%d", gExitGrid.ubGotoSectorY + 'A' - 1, gExitGrid.ubGotoSectorX);
	SetInputFieldStringWith16BitString( 7, str );
	swprintf(str, lengthof(str), L"%d", gExitGrid.ubGotoSectorZ);
	SetInputFieldStringWith16BitString( 8, str );
	swprintf(str, lengthof(str), L"%d", gExitGrid.usGridNo);
	SetInputFieldStringWith16BitString( 9, str );
	SetActiveField( 0 );
	return TRUE;
}


void LocateNextExitGrid()
{
	static UINT16 usCurrentExitGridNo = 0;

	EXITGRID ExitGrid;
	UINT16 i;
	for( i = usCurrentExitGridNo + 1; i < WORLD_MAX; i++ )
	{
		if( GetExitGrid( i, &ExitGrid ) )
		{
			usCurrentExitGridNo = i;
			CenterScreenAtMapIndex( i );
			return;
		}
	}
	for( i = 0; i < usCurrentExitGridNo; i++ )
	{
		if( GetExitGrid( i, &ExitGrid ) )
		{
			usCurrentExitGridNo = i;
			CenterScreenAtMapIndex( i );
			return;
		}
	}
}

void ChangeLightDefault( INT8 bLightType )
{
	UnclickEditorButton( MAPINFO_PRIMETIME_LIGHT + gbDefaultLightType );
	gbDefaultLightType = bLightType;
	ClickEditorButton( MAPINFO_PRIMETIME_LIGHT + gbDefaultLightType );
}
