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

#include <string_theory/format>
#include <string_theory/string>


INT8 gbDefaultLightType = PRIMETIME_LIGHT;

SGPPaletteEntry	gEditorLightColor;

BOOLEAN gfEditorForceShadeTableRebuild = FALSE;

void SetupTextInputForMapInfo()
{
	ST::string str;

	InitTextInputModeWithScheme( DEFAULT_SCHEME );

	AddUserInputField( NULL );  //just so we can use short cut keys while not typing.

	//light rgb fields
	str = ST::format("{}", gEditorLightColor.r);
	AddTextInputField( 10, EDITOR_TASKBAR_POS_Y + 34, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = ST::format("{}", gEditorLightColor.g);
	AddTextInputField( 10, EDITOR_TASKBAR_POS_Y + 54, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = ST::format("{}", gEditorLightColor.b);
	AddTextInputField( 10, EDITOR_TASKBAR_POS_Y + 74, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );

	str = ST::format("{}", gsLightRadius);
	AddTextInputField( 120, EDITOR_TASKBAR_POS_Y + 34, 25, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_NUMERICSTRICT );
	str = ST::format("{}", gusLightLevel);
	AddTextInputField( 120, EDITOR_TASKBAR_POS_Y + 54, 25, 18, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );

	//Scroll restriction ID
	str = ST::format("{.d}", gMapInformation.ubRestrictedScrollID);
	AddTextInputField( 210, EDITOR_TASKBAR_POS_Y + 60, 30, 20, MSYS_PRIORITY_NORMAL, str, 2, INPUTTYPE_NUMERICSTRICT );

	//exit grid input fields
	str = ST::format("{}", gExitGrid.ubGotoSector.AsShortString());
	AddTextInputField(338, EDITOR_TASKBAR_POS_Y +  3, 30, 18, MSYS_PRIORITY_NORMAL, str, 3, INPUTTYPE_COORDINATE);
	str = ST::format("{}", gExitGrid.ubGotoSector.z);
	AddTextInputField( 338, EDITOR_TASKBAR_POS_Y + 23, 30, 18, MSYS_PRIORITY_NORMAL, str, 1, INPUTTYPE_NUMERICSTRICT );
	str = ST::format("{}", gExitGrid.usGridNo);
	AddTextInputField( 338, EDITOR_TASKBAR_POS_Y + 43, 40, 18, MSYS_PRIORITY_NORMAL, str, 5, INPUTTYPE_NUMERICSTRICT );
}

void UpdateMapInfo()
{
	SetFont( FONT10ARIAL );
	SetFontShadow( FONT_NEARBLACK );

	SetFontForeground( FONT_RED );
	MPrint( 38, EDITOR_TASKBAR_POS_Y + 39, "R");
	SetFontForeground( FONT_GREEN );
	MPrint( 38, EDITOR_TASKBAR_POS_Y + 59, "G");
	SetFontForeground( FONT_DKBLUE );
	MPrint( 38, EDITOR_TASKBAR_POS_Y + 79, "B");

	SetFontForeground( FONT_YELLOW );
	MPrint( 65, EDITOR_TASKBAR_POS_Y +  9, "Prime");
	MPrint( 65, EDITOR_TASKBAR_POS_Y + 22, "Night");
	MPrint( 65, EDITOR_TASKBAR_POS_Y + 37, "24Hrs");

	SetFontForeground( FONT_YELLOW );
	MPrint(148, EDITOR_TASKBAR_POS_Y + 39, "Radius");

	if( !gfBasement && !gfCaves )
		SetFontForeground( FONT_DKYELLOW );
	MPrint(148, EDITOR_TASKBAR_POS_Y + 54, "Underground");
	MPrint(148, EDITOR_TASKBAR_POS_Y + 63, "Light Level");

	SetFontForeground( FONT_YELLOW );
	MPrint(230, EDITOR_TASKBAR_POS_Y +  9, "Outdoors");
	MPrint(230, EDITOR_TASKBAR_POS_Y + 24, "Basement");
	MPrint(230, EDITOR_TASKBAR_POS_Y + 39, "Caves");

	SetFontForeground( FONT_ORANGE );
	MPrint(250, EDITOR_TASKBAR_POS_Y + 60, "Restricted");
	MPrint(250, EDITOR_TASKBAR_POS_Y + 70, "Scroll ID");

	SetFontForeground( FONT_YELLOW );
	MPrint(368, EDITOR_TASKBAR_POS_Y +  3, "Destination");
	MPrint(368, EDITOR_TASKBAR_POS_Y + 12, "Sector");
	MPrint(368, EDITOR_TASKBAR_POS_Y + 23, "Destination");
	MPrint(368, EDITOR_TASKBAR_POS_Y + 32, "Bsmt. Level");
	MPrint(378, EDITOR_TASKBAR_POS_Y + 43, "Dest.");
	MPrint(378, EDITOR_TASKBAR_POS_Y + 52, "GridNo");
}

void UpdateMapInfoFields()
{
	ST::string str;
	//Update the text fields to reflect the validated values.
	//light rgb fields
	str = ST::format("{}", gEditorLightColor.r);
	SetInputFieldString( 1, str );
	str = ST::format("{}", gEditorLightColor.g);
	SetInputFieldString( 2, str );
	str = ST::format("{}", gEditorLightColor.b);
	SetInputFieldString( 3, str );

	str = ST::format("{}", gsLightRadius);
	SetInputFieldString( 4, str );
	str = ST::format("{}", gusLightLevel);
	SetInputFieldString( 5, str );

	str = ST::format("{.d}", gMapInformation.ubRestrictedScrollID);
	SetInputFieldString( 6, str );

	ApplyNewExitGridValuesToTextFields();
}

void ExtractAndUpdateMapInfo()
{
	INT32 temp;
	BOOLEAN fUpdateLight1 = FALSE;
	//extract light1 colors
	temp = std::min(GetNumericStrictValueFromField( 1 ), 255);
	if (temp != -1 && temp != gEditorLightColor.r)
	{
		fUpdateLight1 = TRUE;
		gEditorLightColor.r = (UINT8)temp;
	}
	temp = std::min(GetNumericStrictValueFromField( 2 ), 255);
	if (temp != -1 && temp != gEditorLightColor.g)
	{
		fUpdateLight1 = TRUE;
		gEditorLightColor.g = (UINT8)temp;
	}
	temp = std::min(GetNumericStrictValueFromField( 3 ), 255);
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
	temp = std::clamp(GetNumericStrictValueFromField( 4 ), 1, 8);
	if( temp != -1 )
		gsLightRadius = (INT16)temp;
	temp = std::clamp(GetNumericStrictValueFromField( 5 ), 1, 15);
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
	ST::string str = GetStringFromField(7);
	char row = str[0];
	if ('a' <= row && row <= 'z' ) row -= 32; //uppercase it!
	if ('A' <= row && row <= 'Z' && '0' <= str[1] && str[1] <= '9')
	{ //only update, if coordinate is valid.
		gExitGrid.ubGotoSector.y = (UINT8)(row    - 'A' + 1);
		gExitGrid.ubGotoSector.x = (UINT8)(str[1] - '0');
		if( str[2] >= '0' && str[2] <= '9' )
			gExitGrid.ubGotoSector.x = (UINT8)(gExitGrid.ubGotoSector.x * 10 + str[2] - '0' );
		gExitGrid.ubGotoSector.x = (UINT8)std::clamp(int(gExitGrid.ubGotoSector.x), 1, 16);
		gExitGrid.ubGotoSector.y = (UINT8)std::clamp(int(gExitGrid.ubGotoSector.y), 1, 16);
	}
	gExitGrid.ubGotoSector.z = (UINT8)std::clamp(GetNumericStrictValueFromField(8), 0, 3);
	gExitGrid.usGridNo      = (UINT16)std::clamp(GetNumericStrictValueFromField(9), 0, 25600);

	UpdateMapInfoFields();
}

BOOLEAN ApplyNewExitGridValuesToTextFields()
{
	ST::string str;
	//exit grid input fields
	if( iCurrentTaskbar != TASK_MAPINFO )
		return FALSE;
	str = gExitGrid.ubGotoSector.AsShortString();
	SetInputFieldString( 7, str );
	str = ST::format("{}", gExitGrid.ubGotoSector.z);
	SetInputFieldString( 8, str );
	str = ST::format("{}", gExitGrid.usGridNo);
	SetInputFieldString( 9, str );
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
