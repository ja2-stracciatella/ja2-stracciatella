#include "Editor_Callback_Prototypes.h"
#include "HImage.h"
#include "Isometric_Utils.h"
#include "TileDef.h"
#include "Edit_Sys.h"
#include "VSurface.h"
#include "VObject.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Input.h"
#include "Font.h"
#include "Font_Control.h"
#include "WorldDef.h"
#include "EditorDefines.h"
#include "EditorTerrain.h"
#include "Editor_Taskbar_Utils.h"
#include "English.h"
#include "UILayout.h"
#include "Random.h"

#include <string_theory/format>


BOOLEAN gfShowTerrainTileButtons;
UINT8 ubTerrainTileButtonWeight[NUM_TERRAIN_TILE_REGIONS];
UINT16 usTotalWeight;
BOOLEAN fPrevShowTerrainTileButtons = TRUE;
BOOLEAN fUseTerrainWeights = FALSE;
INT32 TerrainTileSelected = 0, TerrainForegroundTile, TerrainBackgroundTile;
INT32 TerrainTileDrawMode = TERRAIN_TILES_NODRAW;

void EntryInitEditorTerrainInfo()
{
	//ResetTerrainTileWeights();
	if( !fUseTerrainWeights )
	{
		ResetTerrainTileWeights();
	}
}

void ResetTerrainTileWeights()
{
	INT8 x;
	for( x=0; x < NUM_TERRAIN_TILE_REGIONS; x++ )
	{
		ubTerrainTileButtonWeight[x] = 0;
	}
	usTotalWeight = 0;
	fUseTerrainWeights = FALSE;
	gfRenderTaskbar = TRUE;
}

void HideTerrainTileButtons()
{
	INT8 x;
	if( gfShowTerrainTileButtons )
	{
		for( x = BASE_TERRAIN_TILE_REGION_ID; x < NUM_TERRAIN_TILE_REGIONS; x++ )
		{
			DisableEditorRegion( x );
		}
		gfShowTerrainTileButtons=FALSE;
	}
}

void ShowTerrainTileButtons()
{
	INT8 x;
	if( !gfShowTerrainTileButtons )
	{
		for( x = BASE_TERRAIN_TILE_REGION_ID; x < NUM_TERRAIN_TILE_REGIONS; x++ )
		{
			EnableEditorRegion( x );
		}
		gfShowTerrainTileButtons=TRUE;
	}
}

void RenderTerrainTileButtons()
{
	// If needed, display the ground tile images
	if(gfShowTerrainTileButtons)
	{
		UINT16 usFillColorDark, usFillColorLight, usFillColorRed;
		UINT16 x, usX, usX2, usY, usY2;

		usFillColorDark = Get16BPPColor(FROMRGB(24, 61, 81));
		usFillColorLight = Get16BPPColor(FROMRGB(136, 138, 135));
		usFillColorRed = Get16BPPColor(FROMRGB(255, 0, 0));

		usY = EDITOR_TASKBAR_POS_Y + 9;
		usY2 = EDITOR_TASKBAR_POS_Y + 31;

		SetFont( SMALLCOMPFONT );
		SetFontForeground( FONT_YELLOW );

		for( x = 0; x < NUM_TERRAIN_TILE_REGIONS; x++ )
		{
			usX = 261 + (x * 42);
			usX2 = usX + 42;

			if ( x == CurrentPaste && !fUseTerrainWeights )
			{
				ColorFillVideoSurfaceArea(ButtonDestBuffer , usX, usY, usX2, usY2, usFillColorRed);
			}
			else
			{
				ColorFillVideoSurfaceArea(ButtonDestBuffer, usX, usY, usX2, usY2, usFillColorDark);
				ColorFillVideoSurfaceArea(ButtonDestBuffer, usX + 1, usY + 1, usX2, usY2, usFillColorLight);
			}
			ColorFillVideoSurfaceArea(ButtonDestBuffer, usX + 1, usY + 1, usX2 - 1, usY2 - 1, 0);

			const HVOBJECT ts = TileElemFromTileType(x)->hTileSurface;
			ts->CurrentShade(DEFAULT_SHADE_LEVEL);
			BltVideoObject(ButtonDestBuffer, ts, 0, usX + 1, usY + 1);

			if( fUseTerrainWeights )
			{
				MPrint( usX+2, usY+2, ST::format("{}", ubTerrainTileButtonWeight[ x ]) );
			}
		}
	}
}

//This callback is used for each of the terrain tile buttons.  The userData[0] field
//contains the terrain button's index value.
void TerrainTileButtonRegionCallback(MOUSE_REGION *reg, UINT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gfRenderTaskbar = TRUE;
		TerrainTileSelected = MSYS_GetRegionUserData(reg,0);
		if(TerrainTileDrawMode == TERRAIN_TILES_FOREGROUND )
		{
			TerrainForegroundTile = TerrainTileSelected;
			CurrentPaste = (UINT16)TerrainForegroundTile;
			//iEditorToolbarState = TBAR_MODE_DRAW;
			if( _KeyDown( SHIFT ) )
			{
				fUseTerrainWeights = TRUE;
			}
			if( fUseTerrainWeights )
			{
				//SHIFT+LEFTCLICK adds weight to the selected terrain tile.
				if( ubTerrainTileButtonWeight[ TerrainTileSelected ] < 10 )
				{
					ubTerrainTileButtonWeight[ TerrainTileSelected ]++;
					usTotalWeight++;
				}
			}
			else
			{ //Regular LEFTCLICK selects only that terrain tile.
				//When total weight is 0, then the only selected tile is drawn.
				ResetTerrainTileWeights();
			}
		}
		else if(TerrainTileDrawMode == TERRAIN_TILES_BACKGROUND )
		{
			TerrainBackgroundTile = TerrainTileSelected;
			iEditorToolbarState = TBAR_MODE_SET_BGRND;
		}
	}
	if(reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		gfRenderTaskbar = TRUE;
		TerrainTileSelected = MSYS_GetRegionUserData( reg, 0 );
		if(TerrainTileDrawMode == TERRAIN_TILES_FOREGROUND )
		{
			TerrainForegroundTile = TerrainTileSelected;
			iEditorToolbarState = TBAR_MODE_DRAW;
			if( ubTerrainTileButtonWeight[ TerrainTileSelected ] )
			{
				ubTerrainTileButtonWeight[ TerrainTileSelected ]--;
				usTotalWeight--;
			}
		}
	}
}

void ChooseWeightedTerrainTile()
{
	UINT16 x, usWeight;
	INT16 sRandomNum;
	if(!usTotalWeight)
	{ //Not in the weighted mode.  CurrentPaste will already contain the selected tile.
		return;
	}
	sRandomNum = Random(usTotalWeight);
	for( x = 0; x < NUM_TERRAIN_TILE_REGIONS; x++ )
	{
		usWeight = ubTerrainTileButtonWeight[ x ];
		sRandomNum -= usWeight;
		if( sRandomNum <= 0 && usWeight )
		{
			CurrentPaste = x;
			return;
		}
	}
}


UINT32 guiSearchType;
UINT32 count, maxCount=0, calls=0;


static void Fill(INT32 x, INT32 y)
{
	INT32 iMapIndex;

	count++;
	calls++;

	if( count > maxCount )
		maxCount = count;

	iMapIndex = y * WORLD_COLS + x;
	if( !GridNoOnVisibleWorldTile( (INT16)iMapIndex ) )
	{
		count--;
		return;
	}
	const UINT32 uiCheckType = GetTileType(gpWorldLevelData[iMapIndex].pLandHead->usIndex);
	if( guiSearchType == uiCheckType )
		PasteTextureCommon( iMapIndex );
	else
	{
		count--;
		return;
	}

	if( y > 0 )
		Fill( x, y-1 );
	if( y < WORLD_ROWS - 1 )
		Fill( x, y+1 );
	if( x > 0 )
		Fill( x-1, y );
	if( x < WORLD_COLS - 1 )
		Fill( x+1, y );
	count--;
}


void TerrainFill( UINT32 iMapIndex )
{
	INT16 sX, sY;
	//determine what we should be looking for to replace...
	guiSearchType = GetTileType(gpWorldLevelData[iMapIndex].pLandHead->usIndex);

	//check terminating conditions
	if( guiSearchType == CurrentPaste )
		return;

	ConvertGridNoToXY( (INT16)iMapIndex, &sX, &sY );

	count = 0;

	Fill( sX, sY );

}
