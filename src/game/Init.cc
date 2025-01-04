#include "HImage.h"
#include "Init.h"
#include "Screens.h"
#include "Strategic_Movement_Costs.h"
#include "Sys_Globals.h"
#include "VObject.h"
#include "RenderWorld.h"
#include "WorldDef.h"
#include "Animation_Data.h"
#include "Overhead.h"
#include "Sound_Control.h"
#include "Lighting.h"
#include "Cursor_Control.h"
#include "Video.h"
#include "Dialogue_Control.h"
#include "Laptop.h"
#include "Tile_Cache.h"
#include "StrategicMap.h"
#include "Map_Information.h"
#include "Summary_Info.h"
#include "GameSettings.h"
#include "Game_Init.h"
#include "Vehicles.h"
#include "Shading.h"
#include "VSurface.h"
#include "GameMode.h"

#include "EditScreen.h"
#include "Logger.h"

#include <string_theory/string>


// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game


ScreenID InitializeJA2(void)
try
{
	gfWorldLoaded = FALSE;

	gsRenderCenterX = 805;
	gsRenderCenterY = 805;

	// Init animation system
	InitAnimationSystem();

	// Init lighting system
	InitLightingSystem();

	// Init dialog queue system
	InitalizeDialogueControl();

	InitStrategicEngine();

	//needs to be called here to init the SectorInfo struct
	InitStrategicMovementCosts( );

	// Init tactical engine
	InitTacticalEngine();

	// Init timer system
	//Moved to the splash screen code.
	//InitializeJA2Clock( );

	// INit shade tables
	BuildShadeTable( );

	// INit intensity tables
	BuildIntensityTable( );

	// Initailize World
	InitializeWorld();

	InitTileCache( );

	if(GameMode::getInstance()->isEditorMode())
	{
		//UNCOMMENT NEXT LINE TO ALLOW FORCE UPDATES...
		//LoadGlobalSummary();
		if( gfMustForceUpdateAllMaps )
		{
			ApologizeOverrideAndForceUpdateEverything();
		}
	}

	switch (GameMode::getInstance()->getMode())
	{
		case GAME_MODE_EDITOR:
			SLOGI("Beginning JA2 using -editor commandline argument...");
			gfAutoLoadA9 = FALSE;
			goto editor;

		case GAME_MODE_EDITOR_AUTO:
			SLOGI("Beginning JA2 using -editorauto commandline argument...");
			gfAutoLoadA9 = TRUE;
editor:
			//also set the sector
			SetWorldSectorInvalid();
			gfIntendOnEnteringEditor = TRUE;
			gGameOptions.fGunNut     = TRUE;
			return GAME_SCREEN;

		default: return INIT_SCREEN;
	}
}
catch (const std::runtime_error& ex)
{
	SET_ERROR(ST::format("InitializeJA2: {}", ex.what()));
	return ERROR_SCREEN;
}


void ShutdownJA2(void)
{
	UINT32 uiIndex;

	FRAME_BUFFER->Fill(Get16BPPColor(FROMRGB(0, 0, 0)));
	InvalidateScreen( );
	// Remove cursor....
	SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );

	RefreshScreen();

	ShutdownStrategicLayer();

	// remove temp files built by laptop
	ClearOutTempLaptopFiles( );

	// Shutdown queue system
	ShutdownDialogueControl();

	// Shutdown Screens
	for (uiIndex = 0; uiIndex < MAX_SCREENS; uiIndex++)
	{
		void (*const shutdown)(void) = GameScreens[uiIndex].ShutdownScreen;
		if (shutdown != NULL) shutdown();
	}


	ShutdownLightingSystem();

	CursorDatabaseClear();

	ShutdownTacticalEngine( );

	// Shutdown Overhead
	ShutdownOverhead( );

	DeInitAnimationSystem();

	DeinitializeWorld( );

	DeleteTileCache( );

	ShutdownJA2Sound( );

	ClearOutVehicleList();
}
