#include "Directories.h"
#include "WorldDat.h"
#include "WorldDef.h"
#include "TileDef.h"
#include "Sys_Globals.h"
#include "Tile_Surface.h"
#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>


// THIS FILE CONTAINS DEFINITIONS FOR TILESET FILES

TILESET gTilesets[NUM_TILESETS];


static void SetTilesetFourTerrainValues(void);
static void SetTilesetThreeTerrainValues(void);
static void SetTilesetTwoTerrainValues(void);


void InitEngineTilesets(void)
try
{
	AutoSGPFile f(GCM->openGameResForReading(GCM->getTilesetDBResName()));

	// READ # TILESETS and compare
	UINT8 ubNumSets;
	FileRead(f, &ubNumSets, sizeof(ubNumSets));
	if (ubNumSets != NUM_TILESETS)
	{
		SET_ERROR("Number of tilesets in code does not match data file");
		return;
	}

	// READ #files
	UINT32 uiNumFiles;
	FileRead(f, &uiNumFiles, sizeof(uiNumFiles));
	if (uiNumFiles != NUMBEROFTILETYPES)
	{
		SET_ERROR("Number of tilesets slots in code does not match data file");
		return;
	}

	// Loop through each tileset, load name then files
	FOR_EACH(TILESET, ts, gTilesets)
	{
		//Read name
		ts->zName = FileReadString(f, TILESET_NAME_LENGTH);

		// Read ambience value
		FileRead(f, &ts->ubAmbientID, sizeof(UINT8));

		// Loop for files
		for (UINT32 cnt2 = 0; cnt2 < uiNumFiles; ++cnt2)
		{
			// Read file name
			ts->zTileSurfaceFilenames[cnt2] = FileReadString(f, TILE_SURFACE_FILENAME_LENGTH);
		}
	}

	// Set callbacks
	gTilesets[CAVES_1      ].MovementCostFnc = SetTilesetTwoTerrainValues;
	gTilesets[AIRSTRIP     ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[DEAD_AIRSTRIP].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TEMP_14      ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TEMP_18      ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TEMP_19      ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TEMP_26      ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TEMP_27      ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TEMP_28      ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TEMP_29      ].MovementCostFnc = SetTilesetThreeTerrainValues;
	gTilesets[TROPICAL_1   ].MovementCostFnc = SetTilesetFourTerrainValues;
	gTilesets[TEMP_20      ].MovementCostFnc = SetTilesetFourTerrainValues;
}
catch (...)
{
	SET_ERROR("Failed to load tileset data file");
}


void SetTilesetOneTerrainValues( )
{
		// FIRST TEXUTRES
		gTileSurfaceArray[ FIRSTTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ SECONDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ THIRDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FOURTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FIFTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SIXTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SEVENTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ REGWATERTEXTURE ]->ubTerrainID = LOW_WATER;
		gTileSurfaceArray[ DEEPWATERTEXTURE ]->ubTerrainID = DEEP_WATER;

		// NOW ROADS
		gTileSurfaceArray[ FIRSTROAD ]->ubTerrainID = DIRT_ROAD;
		gTileSurfaceArray[ ROADPIECES ]->ubTerrainID = DIRT_ROAD;

		// NOW FLOORS
		gTileSurfaceArray[ FIRSTFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ SECONDFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ THIRDFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ FOURTHFLOOR ]->ubTerrainID = FLAT_FLOOR;

		// NOW ANY TERRAIN MODIFYING DEBRIS

}


static void SetTilesetTwoTerrainValues(void)
{
		// FIRST TEXUTRES
		gTileSurfaceArray[ FIRSTTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ SECONDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ THIRDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FOURTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FIFTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SIXTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SEVENTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ REGWATERTEXTURE ]->ubTerrainID = LOW_WATER;
		gTileSurfaceArray[ DEEPWATERTEXTURE ]->ubTerrainID = DEEP_WATER;

		// NOW ROADS
		gTileSurfaceArray[ FIRSTROAD ]->ubTerrainID = DIRT_ROAD;
		gTileSurfaceArray[ ROADPIECES ]->ubTerrainID = DIRT_ROAD;


		// NOW FLOORS
		gTileSurfaceArray[ FIRSTFLOOR ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ SECONDFLOOR ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ THIRDFLOOR ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FOURTHFLOOR ]->ubTerrainID = FLAT_GROUND;

}


static void SetTilesetThreeTerrainValues(void)
{
		// DIFFERENCE FROM #1 IS THAT ROADS ARE PAVED

		// FIRST TEXUTRES
		gTileSurfaceArray[ FIRSTTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ SECONDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ THIRDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FOURTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FIFTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SIXTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SEVENTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ REGWATERTEXTURE ]->ubTerrainID = LOW_WATER;
		gTileSurfaceArray[ DEEPWATERTEXTURE ]->ubTerrainID = DEEP_WATER;

		// NOW ROADS
		gTileSurfaceArray[ FIRSTROAD ]->ubTerrainID = PAVED_ROAD;
		gTileSurfaceArray[ ROADPIECES ]->ubTerrainID = PAVED_ROAD;


		// NOW FLOORS
		gTileSurfaceArray[ FIRSTFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ SECONDFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ THIRDFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ FOURTHFLOOR ]->ubTerrainID = FLAT_FLOOR;

		// NOW ANY TERRAIN MODIFYING DEBRIS

}


static void SetTilesetFourTerrainValues(void)
{
		// DIFFERENCE FROM #1 IS THAT FLOOR2 IS NOT FLAT_FLOOR BUT FLAT_GROUND

		// FIRST TEXUTRES
		gTileSurfaceArray[ FIRSTTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ SECONDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ THIRDTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FOURTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ FIFTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SIXTHTEXTURE ]->ubTerrainID = LOW_GRASS;
		gTileSurfaceArray[ SEVENTHTEXTURE ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ REGWATERTEXTURE ]->ubTerrainID = LOW_WATER;
		gTileSurfaceArray[ DEEPWATERTEXTURE ]->ubTerrainID = DEEP_WATER;

		// NOW ROADS
		gTileSurfaceArray[ FIRSTROAD ]->ubTerrainID = DIRT_ROAD;
		gTileSurfaceArray[ ROADPIECES ]->ubTerrainID = DIRT_ROAD;


		// NOW FLOORS
		gTileSurfaceArray[ FIRSTFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ SECONDFLOOR ]->ubTerrainID = FLAT_GROUND;
		gTileSurfaceArray[ THIRDFLOOR ]->ubTerrainID = FLAT_FLOOR;
		gTileSurfaceArray[ FOURTHFLOOR ]->ubTerrainID = FLAT_FLOOR;

		// NOW ANY TERRAIN MODIFYING DEBRIS

}
