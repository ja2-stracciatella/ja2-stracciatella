/* The implementation of swprintf() is broken on FreeBSD and sometimes fails if
 * LC_TYPE is not set to UTF-8.  This happens when characters, which cannot be
 * represented by the current LC_CTYPE, are printed. */
#if defined __FreeBSD__
#	define BROKEN_SWPRINTF
#endif

#if defined BROKEN_SWPRINTF
#	include <locale.h>
#endif

#include <exception>
#include <new>

#include "Button_System.h"
#include "Cheats.h"
#include "Debug.h"
#include "FileMan.h"
#include "Font.h"
#include "GameLoop.h"
#include "Init.h" // XXX should not be used in SGP
#include "Input.h"
#include "Intro.h"
#include "JA2_Splash.h"
#include "MemMan.h"
#include "PathTools.h"
#include "Random.h"
#include "SGP.h"
#include "SaveLoadGame.h" // XXX should not be used in SGP
#include "SoundMan.h"
#include "VObject.h"
#include "Video.h"
#include "VSurface.h"
#include <SDL.h>
#include "UILayout.h"
#include "GameRes.h"
#include "GameState.h"
#include "Timer.h"

#include "DefaultContentManager.h"
#include "GameInstance.h"
#include "JsonUtility.h"
#include "ModPackContentManager.h"
#include "policy/GamePolicy.h"
#include "sgp/UTF8String.h"
#include "RustInterface.h"

#include "slog/slog.h"

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#endif

#if defined _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>

#	include "Local.h"
#endif

#include "Multi_Language_Graphic_Utils.h"

extern BOOLEAN gfPauseDueToPlayerGamePause;

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////

// #include "JsonObject.h"
// #include "MagazineModel.h"
// #include "WeaponModels.h"
// #include "Weapons.h"
// #include "rapidjson/document.h"
// #include "rapidjson/filestream.h"
// #include "rapidjson/prettywriter.h"
// #include "stdio.h"

// bool writeWeaponsToJson(const char *name/*, const struct WEAPONTYPE *weapon*/, int weaponCount)
// {
//   FILE *f = fopen(name, "wt");
//   if(f)
//   {
//     rapidjson::FileStream os(f);
//     rapidjson::PrettyWriter<rapidjson::FileStream> writer(os);

//     rapidjson::Document document;
//     document.SetArray();
//     rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

//     for(int i = 0; i < weaponCount; i++)
//     {
//       // printf("%d\n", i);
//       const WeaponModel *w = GCM->getWeapon(i);
//       JsonObject obj(allocator);
//       w->serializeTo(obj);
//       document.PushBack(obj.getValue(), allocator);
//     }

//     document.Accept(writer);

//     fputs("\n", f);
//     return fclose(f) == 0;
//   }
//   return false;
// }

// bool writeMagazinesToJson(const char *name)
// {
//   FILE *f = fopen(name, "wt");
//   if(f)
//   {
//     rapidjson::FileStream os(f);
//     rapidjson::PrettyWriter<rapidjson::FileStream> writer(os);

//     rapidjson::Document document;
//     document.SetArray();
//     rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

//     const std::vector<const MagazineModel*>& magazines = GCM->getMagazines();
//     for (const MagazineModel* mag : magazines)
//     {
//       JsonObject obj(allocator);
//       mag->serializeTo(obj);
//       document.PushBack(obj.getValue(), allocator);
//     }

//     document.Accept(writer);

//     fputs("\n", f);
//     return fclose(f) == 0;
//   }
//   return false;
// }

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////

static BOOLEAN gfGameInitialized = FALSE;

/** Deinitialize the game an exit. */
static void deinitGameAndExit()
{
	SLOGD(DEBUG_TAG_SGP, "Deinitializing Game");
	// If we are in Dead is Dead mode, save before exit
	// Does this code also fire on crash? Let's hope not!
	DoDeadIsDeadSaveIfNecessary();

	SoundServiceStreams();

	if (gfGameInitialized)
	{
		ShutdownGame();
	}
	SLOGD(DEBUG_TAG_SGP, "Shutting Down Button System");
	ShutdownButtonSystem();
	MSYS_Shutdown();

#ifndef UTIL
	SLOGD(DEBUG_TAG_SGP, "Shutting Down Sound Manager");
	ShutdownSoundManager();
#endif

#ifdef SGP_VIDEO_DEBUGGING
	SLOGD(DEBUG_TAG_SGP, "Dumping Video Info");
	PerformVideoInfoDumpIntoFile( "SGPVideoShutdownDump.txt", FALSE );
#endif

	SLOGD(DEBUG_TAG_SGP, "Shutting Down Video Surface Manager");
	ShutdownVideoSurfaceManager();
	SLOGD(DEBUG_TAG_SGP, "Shutting Down Video Object Manager");
	ShutdownVideoObjectManager();
	SLOGD(DEBUG_TAG_SGP, "Shutting Down Video Manager");
	ShutdownVideoManager();
	SLOGD(DEBUG_TAG_SGP, "Shutting Down Memory Manager");
	ShutdownMemoryManager();  // must go last, for MemDebugCounter to work right...

	SLOGD(DEBUG_TAG_SGP, "Shutting Down SDL");
	SDL_Quit();

	exit(0);
}


/** Request game exit.
 * Call this function if you want to exit the game. */
void requestGameExit()
{
	SDL_Event event;
	event.type = SDL_QUIT;
	SDL_PushEvent(&event);
}

static void MainLoop(int msPerGameCycle)
{
	BOOLEAN s_doGameCycles = TRUE;

	while (true)
	{
		// cycle until SDL_Quit is received

		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_APP_WILLENTERBACKGROUND:
					s_doGameCycles = false;
					break;

				case SDL_APP_WILLENTERFOREGROUND:
					s_doGameCycles = true;
					break;

				case SDL_KEYDOWN: KeyDown(&event.key.keysym); break;
				case SDL_KEYUP:   KeyUp(  &event.key.keysym); break;
				case SDL_TEXTINPUT: TextInput(&event.text); break;

				case SDL_MOUSEBUTTONDOWN: MouseButtonDown(&event.button); break;
				case SDL_MOUSEBUTTONUP:   MouseButtonUp(&event.button);   break;

				case SDL_MOUSEMOTION:
					SetSafeMousePosition(event.motion.x, event.motion.y);
					break;

				case SDL_MOUSEWHEEL: MouseWheelScroll(&event.wheel); break;

				case SDL_QUIT: deinitGameAndExit(); break;
			}
		}
		else
		{
			if (s_doGameCycles)
			{
				UINT32 gameCycleMS = GetClock();
#if DEBUG_PRINT_GAME_CYCLE_TIME
				UINT32 totalGameCycleMS = gameCycleMS;
#endif
				GameLoop();
				gameCycleMS = GetClock() - gameCycleMS;

				if(gameCycleMS < msPerGameCycle)
				{
					SDL_Delay(msPerGameCycle - gameCycleMS);
				}

#if DEBUG_PRINT_GAME_CYCLE_TIME
				totalGameCycleMS = GetClock() - totalGameCycleMS;
				printf("game cycle: %4d %4d\n", gameCycleMS, totalGameCycleMS);
#endif
			}
			else
			{
				SDL_WaitEvent(NULL);
			}
		}
	}
}

////////////////////////////////////////////////////////////

ContentManager *GCM = NULL;

////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	std::string exeFolder = PathTools::getParentPath(argv[0], true);

#if defined BROKEN_SWPRINTF
	if (setlocale(LC_CTYPE, "UTF-8") == NULL)
	{
		fprintf(stderr, "WARNING: Failed to set LC_CTYPE to UTF-8. Some strings might get garbled.\n");
	}
#endif

	// init logging
	SLOG_Init(SLOG_STDERR, "ja2.log");
	SLOG_SetLevel(SLOG_WARNING, SLOG_WARNING);

	engine_options_t* params = create_engine_options(argv, argc);
	if (params == NULL) {
		return EXIT_FAILURE;
	}

	if (should_show_help(params)) {
		return EXIT_SUCCESS;
	}

	if (should_start_in_fullscreen(params)) {
		VideoSetFullScreen(TRUE);
	} else if (should_start_in_window(params)) {
		VideoSetFullScreen(FALSE);
	}

	if (should_start_without_sound(params)) {
		SoundEnableSound(FALSE);
	}

	// Disable sound when using SDL2 2.0.6:
	SDL_version sdl_version_linked;
	SDL_GetVersion(&sdl_version_linked);
	if (sdl_version_linked.major == 2 && sdl_version_linked.minor == 0 && sdl_version_linked.patch == 6) {
		SLOGE(DEBUG_TAG_SGP, "Detected SDL2 2.0.6. Disabled sound.\n"
							 "This version of SDL2 has a fatal bug in the audio conversion routines.\n"
							 "Either downgrade to version 2.0.5 or upgrade to version 2.0.7 or later.");
		SoundEnableSound(FALSE);
	}

	if (should_start_in_debug_mode(params)) {
		SLOG_SetLevel(SLOG_DEBUG, SLOG_DEBUG);
		GameState::getInstance()->setDebugging(true);
	}

	if (should_run_editor(params)) {
		GameState::getInstance()->setEditorMode(false);
	}

	bool result = g_ui.setScreenSize(get_resolution_x(params), get_resolution_y(params));
	if(!result)
	{
		SLOGE(DEBUG_TAG_SGP, "Failed to set screen resolution %d x %d", get_resolution_x(params), get_resolution_y(params));
		return EXIT_FAILURE;
	}

	if (should_run_unittests(params)) {
#ifdef WITH_UNITTESTS
		testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
#else
		SLOGW(DEBUG_TAG_SGP, "This executable does not include unit tests.");
#endif
	}

	GameVersion version = get_resource_version(params);;
	setGameVersion(version);

	////////////////////////////////////////////////////////////

	SDL_Init(SDL_INIT_VIDEO);

	// restore output to the console (on windows when built with MINGW)
#ifdef __MINGW32__
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
#endif

#ifdef SGP_DEBUG
	// Initialize the Debug Manager - success doesn't matter
	InitializeDebugManager();
#endif

	// this one needs to go ahead of all others (except Debug), for MemDebugCounter to work right...
	SLOGD(DEBUG_TAG_SGP, "Initializing Memory Manager");
	InitializeMemoryManager();

	SLOGD(DEBUG_TAG_SGP, "Initializing Game Resources");
	char* rustConfigFolderPath = get_stracciatella_home(params);
	char* rustResRootPath = get_vanilla_data_dir(params);
	std::string configFolderPath = std::string(rustConfigFolderPath);
	std::string gameResRootPath = std::string(rustResRootPath);
	free_rust_string(rustConfigFolderPath);
	free_rust_string(rustResRootPath);

	std::string extraDataDir = EXTRA_DATA_DIR;
	if(extraDataDir.empty())
	{
		// use location of the exe file
		extraDataDir = exeFolder;
	}

	std::string externalizedDataPath = PathTools::joinPaths(extraDataDir, "externalized");

	FileMan::switchTmpFolder(configFolderPath);

	DefaultContentManager *cm;

	if(get_number_of_mods(params) > 0)
	{
		char* rustModName = get_mod(params, 0);
		std::string modName = std::string(rustModName);
		free_rust_string(rustModName);
		std::string modResFolder = PathTools::joinPaths(PathTools::joinPaths(PathTools::joinPaths(extraDataDir, "mods"), modName), "data");
		cm = new ModPackContentManager(version,
						modName, modResFolder, configFolderPath,
						gameResRootPath, externalizedDataPath);
		SLOGI(DEBUG_TAG_SGP,"------------------------------------------------------------------------------");
		SLOGI(DEBUG_TAG_SGP,"JA2 Home Dir:                  '%s'", configFolderPath.c_str());
		SLOGI(DEBUG_TAG_SGP,"Root game resources directory: '%s'", gameResRootPath.c_str());
		SLOGI(DEBUG_TAG_SGP,"Extra data directory:          '%s'", extraDataDir.c_str());
		SLOGI(DEBUG_TAG_SGP,"Data directory:                '%s'", cm->getDataDir().c_str());
		SLOGI(DEBUG_TAG_SGP,"Tilecache directory:           '%s'", cm->getTileDir().c_str());
		SLOGI(DEBUG_TAG_SGP,"Saved games directory:         '%s'", cm->getSavedGamesFolder().c_str());
		SLOGI(DEBUG_TAG_SGP,"------------------------------------------------------------------------------");
		SLOGI(DEBUG_TAG_SGP,"MOD name:                      '%s'", modName.c_str());
		SLOGI(DEBUG_TAG_SGP,"MOD resource directory:        '%s'", modResFolder.c_str());
		SLOGI(DEBUG_TAG_SGP,"------------------------------------------------------------------------------");
	}
	else
	{
		cm = new DefaultContentManager(version,
						configFolderPath,
						gameResRootPath, externalizedDataPath);
		SLOGI(DEBUG_TAG_SGP,"------------------------------------------------------------------------------");
		SLOGI(DEBUG_TAG_SGP,"JA2 Home Dir:                  '%s'", configFolderPath.c_str());
		SLOGI(DEBUG_TAG_SGP,"Root game resources directory: '%s'", gameResRootPath.c_str());
		SLOGI(DEBUG_TAG_SGP,"Extra data directory:          '%s'", extraDataDir.c_str());
		SLOGI(DEBUG_TAG_SGP,"Data directory:                '%s'", cm->getDataDir().c_str());
		SLOGI(DEBUG_TAG_SGP,"Tilecache directory:           '%s'", cm->getTileDir().c_str());
		SLOGI(DEBUG_TAG_SGP,"Saved games directory:         '%s'", cm->getSavedGamesFolder().c_str());
		SLOGI(DEBUG_TAG_SGP,"------------------------------------------------------------------------------");
	}

		free_engine_options(params);

	std::vector<std::string> libraries = cm->getListOfGameResources();
	cm->initGameResouces(configFolderPath, libraries);

	if(!cm->loadGameData())
	{
		SLOGI(DEBUG_TAG_SGP,"Failed to load the game data.");
	}
	else
	{

		GCM = cm;

		SLOGD(DEBUG_TAG_SGP, "Initializing Video Manager");
		InitializeVideoManager();

		SLOGD(DEBUG_TAG_SGP, "Initializing Video Object Manager");
		InitializeVideoObjectManager();

		SLOGD(DEBUG_TAG_SGP, "Initializing Video Surface Manager");
		InitializeVideoSurfaceManager();

		InitJA2SplashScreen();

		// Initialize Font Manager
		SLOGD(DEBUG_TAG_SGP, "Initializing the Font Manager");
		// Init the manager and copy the TransTable stuff into it.
		InitializeFontManager();

		SLOGD(DEBUG_TAG_SGP, "Initializing Sound Manager");
#ifndef UTIL
		InitializeSoundManager();
#endif

		SLOGD(DEBUG_TAG_SGP, "Initializing Random");
		// Initialize random number generator
		InitializeRandom(); // no Shutdown

		SLOGD(DEBUG_TAG_SGP, "Initializing Game Manager");
		// Initialize the Game
		InitializeGame();

		gfGameInitialized = TRUE;

		////////////////////////////////////////////////////////////

		// some data convertion
		// convertDialogQuotesToJson(cm, SE_RUSSIAN, "mercedt/051.edt", FileMan::joinPaths(exeFolder, "051.edt.json").c_str());
		// convertDialogQuotesToJson(cm, SE_RUSSIAN, "mercedt/052.edt", FileMan::joinPaths(exeFolder, "052.edt.json").c_str());
		// convertDialogQuotesToJson(cm, SE_RUSSIAN, "mercedt/055.edt", FileMan::joinPaths(exeFolder, "055.edt.json").c_str());

		// writeWeaponsToJson(FileMan::joinPaths(exeFolder, "externalized/weapons.json").c_str(), MAX_WEAPONS+1);
		// writeMagazinesToJson(FileMan::joinPaths(exeFolder, "externalized/magazines.json").c_str());

		// readWeaponsFromJson(FileMan::joinPaths(exeFolder, "weapon.json").c_str());
		// readWeaponsFromJson(FileMan::joinPaths(exeFolder, "weapon2.json").c_str());

		////////////////////////////////////////////////////////////

		if(isEnglishVersion())
		{
			SetIntroType(INTRO_SPLASH);
		}

		SLOGD(DEBUG_TAG_SGP, "Running Game");

		/* At this point the SGP is set up, which means all I/O, Memory, tools, etc.
		 * are available. All we need to do is attend to the gaming mechanics
		 * themselves */
		MainLoop(gamepolicy(ms_per_game_cycle));
	}

	SLOG_Deinit();

	delete cm;
	GCM = NULL;

	return EXIT_SUCCESS;
}

static void convertDialogQuotesToJson(const DefaultContentManager *cm,
					STRING_ENC_TYPE encType,
					const char *dialogFile, const char *outputFile)
{
	std::vector<UTF8String*> quotes;
	std::vector<std::string> quotes_str;
	cm->loadAllDialogQuotes(encType, dialogFile, quotes);
	for(int i = 0; i < quotes.size(); i++)
	{
		quotes_str.push_back(std::string(quotes[i]->getUTF8()));
		delete quotes[i];
		quotes[i] = NULL;
	}
	JsonUtility::writeToFile(outputFile, quotes_str);
}
