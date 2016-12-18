#include "game/Utils/Event_Manager.h"
#include "sgp/HImage.h"
#include "Init.h"
#include "Local.h"
#include "Screens.h"
#include "game/Strategic/Strategic_Movement_Costs.h"
#include "Sys_Globals.h"
#include "sgp/VObject.h"
#include "game/TileEngine/RenderWorld.h"
#include "sgp/MouseSystem.h"
#include "game/TileEngine/WorldDef.h"
#include "game/Tactical/Animation_Data.h"
#include "game/Tactical/Overhead.h"
#include "game/Utils/Font_Control.h"
#include "game/Utils/Timer_Control.h"
#include "game/TileEngine/Radar_Screen.h"
#include "game/TileEngine/Render_Dirty.h"
#include "game/Utils/Sound_Control.h"
#include "game/TileEngine/Lighting.h"
#include "sgp/Cursor_Control.h"
#include "sgp/Video.h"
#include "game/Tactical/Interface_Items.h"
#include "game/Tactical/Dialogue_Control.h"
#include "game/Utils/Text.h"
#include "game/Laptop/Laptop.h"
#include "game/TacticalAI/NPC.h"
#include "game/Utils/MercTextBox.h"
#include "game/TileEngine/Tile_Cache.h"
#include "game/Strategic/StrategicMap.h"
#include "game/Tactical/Map_Information.h"
#include "game/TileEngine/Exit_Grids.h"
#include "game/Editor/Summary_Info.h"
#include "GameSettings.h"
#include "game/Strategic/Game_Init.h"
#include "game/Strategic/Strategic_Movement.h"
#include "game/Tactical/Vehicles.h"
#include "sgp/Shading.h"
#include "sgp/VSurface.h"
#include "GameState.h"

#include "game/Editor/EditScreen.h"
#include "JAScreens.h"
#include "slog/slog.h"

// The InitializeGame function is responsible for setting up all data and Gaming Engine
// tasks which will run the game


ScreenID InitializeJA2(void)
try
{
	gfWorldLoaded = FALSE;

	// Load external text
	LoadAllExternalText();

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

	// Init Event Manager
	InitializeEventManager();

	// Initailize World
	InitializeWorld();

	InitTileCache( );

	InitMercPopupBox( );

  if(GameState::getInstance()->isEditorMode())
  {
    //UNCOMMENT NEXT LINE TO ALLOW FORCE UPDATES...
    //LoadGlobalSummary();
    if( gfMustForceUpdateAllMaps )
    {
      ApologizeOverrideAndForceUpdateEverything();
    }
  }

	switch (GameState::getInstance()->getMode())
	{
#if defined JA2BETAVERSION
		case GAME_MODE_MAP_UTILITY: return MAPUTILITY_SCREEN;
#endif

		case GAME_MODE_EDITOR:
			SLOGI(DEBUG_TAG_INIT, "Beginning JA2 using -editor commandline argument...");
			gfAutoLoadA9 = FALSE;
			goto editor;

		case GAME_MODE_EDITOR_AUTO:
			SLOGI(DEBUG_TAG_INIT, "Beginning JA2 using -editorauto commandline argument...");
			gfAutoLoadA9 = TRUE;
editor:
			//For editor purposes, need to know the default map file.
			strcpy(g_filename, "none");
			//also set the sector
			SetWorldSectorInvalid();
			gfIntendOnEnteringEditor = TRUE;
			gGameOptions.fGunNut     = TRUE;
			return GAME_SCREEN;

		default: return INIT_SCREEN;
	}
}
catch (...) { return ERROR_SCREEN; }


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

	ShutdownJA2Clock( );

	ShutdownFonts();

	ShutdownJA2Sound( );

	ShutdownEventManager( );

	ClearOutVehicleList();
}
