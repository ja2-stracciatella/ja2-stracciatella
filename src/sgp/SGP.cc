#include <clocale>
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

#include "Logger.h"

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
	SLOGD("Deinitializing Game");
	// If we are in Dead is Dead mode, save before exit
	// Does this code also fire on crash? Let's hope not!
	DoDeadIsDeadSaveIfNecessary();

	SoundServiceStreams();

	if (gfGameInitialized)
	{
		ShutdownGame();
	}
	SLOGD("Shutting Down Button System");
	ShutdownButtonSystem();
	MSYS_Shutdown();

#ifndef UTIL
	SLOGD("Shutting Down Sound Manager");
	ShutdownSoundManager();
#endif

#ifdef SGP_VIDEO_DEBUGGING
	SLOGD("Dumping Video Info");
	PerformVideoInfoDumpIntoFile( "SGPVideoShutdownDump.txt", FALSE );
#endif

	SLOGD("Shutting Down Video Surface Manager");
	ShutdownVideoSurfaceManager();
	SLOGD("Shutting Down Video Object Manager");
	ShutdownVideoObjectManager();
	SLOGD("Shutting Down Video Manager");
	ShutdownVideoManager();
	SLOGD("Shutting Down Memory Manager");
	ShutdownMemoryManager();  // must go last, for MemDebugCounter to work right...

	SLOGD("Shutting Down SDL");
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

/// Sets C locale to something that supports unicode.
/// The C++ locale is unchanged (VS2015 hangs for unknown reasons).
///
/// There is no way to query available locales so this is the most portable
/// way to get a locale that supports unicode.
void SetUnicodeLocale()
{
	static const char* LOCALES[] = {
		// custom locale set at configure time
#if defined WITH_CUSTOM_LOCALE
		WITH_CUSTOM_LOCALE,
#endif
		// the preferred locale with UTF-8 encoding
		".UTF8", ".UTF-8",
		// a specific locale with UTF-8 encoding
		"C.UTF-8", "en_US.UTF8", "en_US.UTF-8", "English_US.UTF8", "en_GB.UTF-8",
		// give up
		nullptr
	};

	for (const char* name : LOCALES)
	{
		if (name == nullptr)
		{
			if (setlocale(LC_ALL, "C") != nullptr)
			{
				fprintf(stderr, "WARNING: Failed to set a unicode locale, using the C locale (might accept only ASCII).\n");
			}
			else
			{
				fprintf(stderr, "WARNING: Failed to set a unicode locale and the C locale...\n");
			}
			return;
		}
		if (setlocale(LC_ALL, name) != nullptr)
		{
			return;
		}
		// try next locale
	}
}

int main(int argc, char* argv[])
{
	SetUnicodeLocale();

	std::string exeFolder = FileMan::getParentPath(argv[0], true);

	// init logging
	Logger_initialize("ja2.log");

	RustPointer<EngineOptions> params(EngineOptions_create(argv, argc));
	if (params == NULL) {
		return EXIT_FAILURE;
	}

	if (EngineOptions_shouldShowHelp(params.get())) {
		return EXIT_SUCCESS;
	}

	if (EngineOptions_shouldStartInFullscreen(params.get())) {
		VideoSetFullScreen(TRUE);
	} else if (EngineOptions_shouldStartInWindow(params.get())) {
		VideoSetFullScreen(FALSE);
	}

	if (EngineOptions_shouldStartWithoutSound(params.get())) {
		SoundEnableSound(FALSE);
	}

	// Disable sound when using SDL2 2.0.6:
	SDL_version sdl_version_linked;
	SDL_GetVersion(&sdl_version_linked);
	if (sdl_version_linked.major == 2 && sdl_version_linked.minor == 0 && sdl_version_linked.patch == 6) {
		SLOGE("Detected SDL2 2.0.6. Disabled sound.\n"
							 "This version of SDL2 has a fatal bug in the audio conversion routines.\n"
							 "Either downgrade to version 2.0.5 or upgrade to version 2.0.7 or later.");
		SoundEnableSound(FALSE);
	}

	if (EngineOptions_shouldStartInDebugMode(params.get())) {
		Logger_setLevel(LogLevel::Debug);
		GameState::getInstance()->setDebugging(true);
	}

	if (EngineOptions_shouldRunEditor(params.get())) {
		GameState::getInstance()->setEditorMode(false);
	}

	auto width = EngineOptions_getResolutionX(params.get());
	auto height = EngineOptions_getResolutionY(params.get());
	bool result = g_ui.setScreenSize(width, height);
	if(!result)
	{
		SLOGE("Failed to set screen resolution %d x %d", width, height);
		return EXIT_FAILURE;
	}

	if (EngineOptions_shouldRunUnittests(params.get())) {
#ifdef WITH_UNITTESTS
		testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
#else
		SLOGW("This executable does not include unit tests.");
#endif
	}

	GameVersion version = EngineOptions_getResourceVersion(params.get());
	setGameVersion(version);

	VideoScaleQuality scalingQuality = EngineOptions_getScalingQuality(params.get());

	FLOAT brightness = EngineOptions_getBrightness(params.get());

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
	SLOGD("Initializing Memory Manager");
	InitializeMemoryManager();

	SLOGD("Initializing Game Resources");
	RustPointer<char> configFolderPath(EngineOptions_getStracciatellaHome(params.get()));
	RustPointer<char> gameResRootPath(EngineOptions_getVanillaGameDir(params.get()));

	std::string extraDataDir = EXTRA_DATA_DIR;
	if(extraDataDir.empty())
	{
		// use location of the exe file
		extraDataDir = exeFolder;
	}

	std::string externalizedDataPath = FileMan::joinPaths(extraDataDir, "externalized");

	FileMan::switchTmpFolder(configFolderPath.get());

	DefaultContentManager *cm;

	auto n = EngineOptions_getModsLength(params.get());
	if(n > 0)
	{
		std::vector<std::string> modNames;
		std::vector<std::string> modResFolders;
		for (auto i = 0; i < n; ++i)
		{
			RustPointer<char> modName(EngineOptions_getMod(params.get(), i));
			std::string modResFolder = FileMan::joinPaths(FileMan::joinPaths(FileMan::joinPaths(extraDataDir, "mods"), modName.get()), "data");
			modNames.emplace_back(modName.get());
			modResFolders.emplace_back(modResFolder);
		}
		cm = new ModPackContentManager(version,
						modNames, modResFolders, configFolderPath.get(),
						gameResRootPath.get(), externalizedDataPath);
		SLOGI("------------------------------------------------------------------------------");
		SLOGI("JA2 Home Dir:                  '%s'", configFolderPath.get());
		SLOGI("Root game resources directory: '%s'", gameResRootPath.get());
		SLOGI("Extra data directory:          '%s'", extraDataDir.c_str());
		SLOGI("Data directory:                '%s'", cm->getDataDir().c_str());
		SLOGI("Tilecache directory:           '%s'", cm->getTileDir().c_str());
		SLOGI("Saved games directory:         '%s'", cm->getSavedGamesFolder().c_str());
		SLOGI("------------------------------------------------------------------------------");
		for (auto i = 0; i < n; ++i)
		{
			SLOGI("MOD name:                      '%s'", modNames[i].c_str());
			SLOGI("MOD resource directory:        '%s'", modResFolders[i].c_str());
			SLOGI("------------------------------------------------------------------------------");
		}
	}
	else
	{
		cm = new DefaultContentManager(version,
						configFolderPath.get(),
						gameResRootPath.get(), externalizedDataPath);
		SLOGI("------------------------------------------------------------------------------");
		SLOGI("JA2 Home Dir:                  '%s'", configFolderPath.get());
		SLOGI("Root game resources directory: '%s'", gameResRootPath.get());
		SLOGI("Extra data directory:          '%s'", extraDataDir.c_str());
		SLOGI("Data directory:                '%s'", cm->getDataDir().c_str());
		SLOGI("Tilecache directory:           '%s'", cm->getTileDir().c_str());
		SLOGI("Saved games directory:         '%s'", cm->getSavedGamesFolder().c_str());
		SLOGI("------------------------------------------------------------------------------");
	}

	std::vector<std::string> libraries = cm->getListOfGameResources();
	cm->initGameResouces(configFolderPath.get(), libraries);

	// free editor.slf has the lowest priority (last library) and is optional
	if(EngineOptions_shouldRunEditor(params.get()))
	{
		try
		{
			cm->addExtraResources(extraDataDir, "editor.slf");
			SLOGI("Free editor.slf loaded from '%s'", extraDataDir.c_str());
		}
		catch(const LibraryFileNotFoundException& ex)
		{
			SLOGI("%s", ex.what());
		}
	}

	if(!cm->loadGameData())
	{
		SLOGI("Failed to load the game data.");
	}
	else
	{

		GCM = cm;

		SLOGD("Initializing Video Manager");
		InitializeVideoManager(scalingQuality);
		VideoSetBrightness(brightness);

		SLOGD("Initializing Video Object Manager");
		InitializeVideoObjectManager();

		SLOGD("Initializing Video Surface Manager");
		InitializeVideoSurfaceManager();

		InitJA2SplashScreen();

		// Initialize Font Manager
		SLOGD("Initializing the Font Manager");
		// Init the manager and copy the TransTable stuff into it.
		InitializeFontManager();

		SLOGD("Initializing Sound Manager");
#ifndef UTIL
		InitializeSoundManager();
#endif

		SLOGD("Initializing Random");
		// Initialize random number generator
		InitializeRandom(); // no Shutdown

		SLOGD("Initializing Game Manager");
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

		SLOGD("Running Game");

		/* At this point the SGP is set up, which means all I/O, Memory, tools, etc.
		 * are available. All we need to do is attend to the gaming mechanics
		 * themselves */
		MainLoop(gamepolicy(ms_per_game_cycle));
	}

	delete cm;
	GCM = NULL;

	return EXIT_SUCCESS;
}

/*static void convertDialogQuotesToJson(const DefaultContentManager *cm,
					STRING_ENC_TYPE encType,
					const char *dialogFile, const char *outputFile)
{
	std::vector<ST::string*> quotes;
	std::vector<std::string> quotes_str;
	cm->loadAllDialogQuotes(encType, dialogFile, quotes);
	for(int i = 0; i < quotes.size(); i++)
	{
		quotes_str.push_back(quotes[i]->to_std_string());
		delete quotes[i];
		quotes[i] = nullptr;
	}
	JsonUtility::writeToFile(outputFile, quotes_str);
}*/
