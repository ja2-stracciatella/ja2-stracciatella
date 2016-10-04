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
#include "MicroIni/MicroIni.hpp"
#include "ModPackContentManager.h"
#include "policy/GamePolicy.h"
#include "sgp/UTF8String.h"

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


#ifdef JA2
extern BOOLEAN gfPauseDueToPlayerGamePause;
#endif


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
// #include <boost/foreach.hpp>

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
//     BOOST_FOREACH(const MagazineModel* mag, magazines)
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


static bool getResourceVersion(const char *versionName, GameVersion *version);
static std::string findRootGameResFolder(const std::string &configPath);
static void WriteDefaultConfigFile(const char* ConfigFile);
static void convertDialogQuotesToJson(const DefaultContentManager *cm,
				      STRING_ENC_TYPE encType,
				      const char *dialogFile, const char *outputFile);

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

struct CommandLineParams
{
  CommandLineParams()
  {
    useMod = false;
    doUnitTests = false;
    showDebugMessages = false;
    resourceVersionGiven = false;
  }

  bool useMod;
  std::string modName;

  bool resourceVersionGiven;
  std::string resourceVersion;

  bool doUnitTests;
  bool showDebugMessages;
};

static BOOLEAN ParseParameters(int argc, char* const argv[],
                               CommandLineParams *params);

int main(int argc, char* argv[])
try
{
  std::string exeFolder = FileMan::getParentPath(argv[0], true);

#if defined BROKEN_SWPRINTF
	if (setlocale(LC_CTYPE, "UTF-8") == NULL)
	{
		fprintf(stderr, "WARNING: Failed to set LC_CTYPE to UTF-8. Some strings might get garbled.\n");
	}
#endif

  // init logging
  SLOG_Init(SLOG_STDERR, "ja2.log");
  SLOG_SetLevel(SLOG_WARNING, SLOG_WARNING);

  setGameVersion(GV_ENGLISH);

  CommandLineParams params;
	if (!ParseParameters(argc, argv, &params)) return EXIT_FAILURE;

  if(params.showDebugMessages)
  {
    SLOG_SetLevel(SLOG_DEBUG, SLOG_DEBUG);
  }

#ifdef WITH_UNITTESTS
  if(params.doUnitTests)
  {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  }
#endif

  GameVersion version = GV_ENGLISH;
  if(params.resourceVersionGiven)
  {
    if(!getResourceVersion(params.resourceVersion.c_str(), &version))
    {
      SLOGE(DEBUG_TAG_SGP, "Unknown version of the game: %s\n", params.resourceVersion.c_str());
      return EXIT_FAILURE;
    }
  }
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
  std::string configFolderPath = FileMan::findConfigFolderAndSwitchIntoIt();
  std::string configPath = FileMan::joinPaths(configFolderPath, "ja2.ini");
  std::string gameResRootPath = findRootGameResFolder(configPath);

  std::string extraDataDir = EXTRA_DATA_DIR;
  if(extraDataDir.empty())
  {
    // use location of the exe file
    extraDataDir = exeFolder;
  }

  std::string externalizedDataPath = FileMan::joinPaths(extraDataDir, "externalized");

  DefaultContentManager *cm;

  if(params.useMod)
  {
    std::string modName = params.modName;
    std::string modResFolder = FileMan::joinPaths(FileMan::joinPaths(FileMan::joinPaths(extraDataDir, "mods"), modName), "data");
    cm = new ModPackContentManager(version,
                                   modName, modResFolder, configFolderPath,
                                   gameResRootPath, externalizedDataPath);
    SLOGI(DEBUG_TAG_SGP,"------------------------------------------------------------------------------");
    SLOGI(DEBUG_TAG_SGP,"Configuration file:            '%s'", configPath.c_str());
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
    SLOGI(DEBUG_TAG_SGP,"Configuration file:            '%s'", configPath.c_str());
    SLOGI(DEBUG_TAG_SGP,"Root game resources directory: '%s'", gameResRootPath.c_str());
    SLOGI(DEBUG_TAG_SGP,"Extra data directory:          '%s'", extraDataDir.c_str());
    SLOGI(DEBUG_TAG_SGP,"Data directory:                '%s'", cm->getDataDir().c_str());
    SLOGI(DEBUG_TAG_SGP,"Tilecache directory:           '%s'", cm->getTileDir().c_str());
    SLOGI(DEBUG_TAG_SGP,"Saved games directory:         '%s'", cm->getSavedGamesFolder().c_str());
    SLOGI(DEBUG_TAG_SGP,"------------------------------------------------------------------------------");
  }

  std::vector<std::string> libraries = cm->getListOfGameResources();
  cm->initGameResouces(configPath, libraries);

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

#ifdef JA2
    InitJA2SplashScreen();
#endif

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

#if defined JA2
    if(isEnglishVersion())
    {
      SetIntroType(INTRO_SPLASH);
    }
#endif

    SLOGD(DEBUG_TAG_SGP, "Running Game");

    /* At this point the SGP is set up, which means all I/O, Memory, tools, etc.
     * are available. All we need to do is attend to the gaming mechanics
     * themselves */
    MainLoop(GCM->getGamePolicy()->ms_per_game_cycle);
  }

  SLOG_Deinit();

  delete cm;
  GCM = NULL;

	return EXIT_SUCCESS;
}
catch (const std::bad_alloc&)
{
  SLOGE(DEBUG_TAG_SGP, "out of memory");
  return EXIT_FAILURE;
}
catch (const LibraryFileNotFoundException& e)
{
  SLOGE(DEBUG_TAG_SGP, "%s", e.what());
  return EXIT_FAILURE;
}
catch (const std::exception& e)
{
  SLOGE(DEBUG_TAG_SGP, "caught unhandled exception:\n%s", e.what());
  return EXIT_FAILURE;
}
catch (...)
{
  SLOGE(DEBUG_TAG_SGP, "caught unhandled unknown exception");
  return EXIT_FAILURE;
}


/** Set game resources version. */
static bool getResourceVersion(const char *versionName, GameVersion *version)
{
  if(strcasecmp(versionName, "ENGLISH") == 0)
  {
    *version = GV_ENGLISH;
  }
  else if(strcasecmp(versionName, "DUTCH") == 0)
  {
    *version = GV_DUTCH;
  }
  else if(strcasecmp(versionName, "FRENCH") == 0)
  {
    *version = GV_FRENCH;
  }
  else if(strcasecmp(versionName, "GERMAN") == 0)
  {
    *version = GV_GERMAN;
  }
  else if(strcasecmp(versionName, "ITALIAN") == 0)
  {
    *version = GV_ITALIAN;
  }
  else if(strcasecmp(versionName, "POLISH") == 0)
  {
    *version = GV_POLISH;
  }
  else if(strcasecmp(versionName, "RUSSIAN") == 0)
  {
    *version = GV_RUSSIAN;
  }
  else if(strcasecmp(versionName, "RUSSIAN_GOLD") == 0)
  {
    *version = GV_RUSSIAN_GOLD;
  }
  else
  {
    return false;
  }
  return true;
}

static BOOLEAN ParseParameters(int argc, char* const argv[], CommandLineParams *params)
{
	const char* const name = *argv;
	if (name == NULL) return TRUE; // argv does not even contain the program name

#ifdef WITH_UNITTESTS
  for(int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-unittests") == 0)
    {
      params->doUnitTests = true;
      return true;
    }
  }
#endif

	BOOLEAN success = TRUE;
  for(int i = 1; i < argc; i++)
  {
    bool haveNextParameter = (i + 1) < argc;

		if (strcmp(argv[i], "-fullscreen") == 0)
		{
			VideoSetFullScreen(TRUE);
		}
		else if (strcmp(argv[i], "-nosound") == 0)
		{
			SoundEnableSound(FALSE);
		}
		else if (strcmp(argv[i], "-window") == 0)
		{
			VideoSetFullScreen(FALSE);
		}
    else if (strcmp(argv[i], "-debug") == 0)
    {
      params->showDebugMessages = true;
      GameState::getInstance()->setDebugging(true);
    }
		else if (strcmp(argv[i], "-res") == 0)
		{
      if(haveNextParameter)
      {
        int width = 0;
        int height = 0;
        int readFields = sscanf(argv[++i], "%dx%d", &width, &height);
        if(readFields != 2)
        {
          SLOGE(DEBUG_TAG_SGP, "Invalid value for command-line key '-res'");
          success = FALSE;
        }
        else
        {
          bool result = g_ui.setScreenSize(width, height);
          if(!result)
          {
            SLOGE(DEBUG_TAG_SGP, "Failed to set screen resolution %d x %d", width, height);
            success = FALSE;
          }
        }
      }
      else
      {
        SLOGE(DEBUG_TAG_SGP, "Missing value for command-line key '-res'");
        success = FALSE;
      }
		}
    else if (strcmp(argv[i], "-mod") == 0)
    {
      if(haveNextParameter)
      {
        params->useMod = true;
        params->modName = argv[++i];
      }
      else
      {
        SLOGE(DEBUG_TAG_SGP, "Missing value for command-line key '-mod'");
        success = FALSE;
      }
    }
#if defined JA2BETAVERSION
		else if (strcmp(argv[i], "-quicksave") == 0)
		{
			/* This allows the QuickSave Slots to be autoincremented, i.e. everytime
			 * the user saves, there will be a new quick save file */
			gfUseConsecutiveQuickSaveSlots = TRUE;
		}
		else if (strcmp(argv[i], "-domaps") == 0)
		{
      GameState::setMode(GAME_MODE_MAP_UTILITY);
		}
#endif
		else if (strcmp(argv[i], "-editor") == 0)
		{
      GameState::getInstance()->setEditorMode(false);
		}
		else if (strcmp(argv[i], "-editorauto") == 0)
		{
      GameState::getInstance()->setEditorMode(true);
		}
    else if (strcmp(argv[i], "-resversion") == 0)
    {
      if(haveNextParameter)
      {
        params->resourceVersionGiven = true;
        params->resourceVersion = argv[++i];
      }
      else
      {
        SLOGE(DEBUG_TAG_SGP, "Missing value for command-line key '-resversion'");
        success = FALSE;
      }
    }
		else
		{
			if (strcmp(argv[i], "-help") != 0)
			{
				fprintf(stderr, "Unknown switch \"%s\"\n", argv[i]);
			}
			success = FALSE;
		}
	}

	if (!success)
	{
		fprintf(stderr,
			"Usage: %s [options]\n"
			"\n"
			"  -res WxH     Screen resolution, e.g. 800x600. Default value is 640x480\n"
			"\n"
			"  -resversion  Version of the game resources.\n"
			"                 Possible values: DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD\n"
			"                 Default value is ENGLISH\n"
			"                 RUSSIAN is for BUKA Agonia Vlasty release\n"
			"                 RUSSIAN_GOLD is for Gold release\n"
      "\n"
      "  -mod NAME    Start one of the game modifications, bundled into the game.\n"
      "               NAME is the name of modification, e.g. 'from-russia-with-love'.\n"
      "               See folder mods for possible options\n"
			"\n"
			"  -debug       Show debug messages\n"
#ifdef WITH_UNITTESTS
      "  -unittests   Perform unit tests\n"
      "                 ja2.exe -unittests [gtest options]\n"
      "                 E.g. ja2.exe -unittests --gtest_output=\"xml:report.xml\" --gtest_repeat=2\n"
#endif
			"  -editor      Start the map editor (Editor.slf is required)\n"
			"  -editorauto  Start the map editor and load sector A9 (Editor.slf is required)\n"
			"  -fullscreen  Start the game in the fullscreen mode\n"
			"  -help        Display this information\n"
			"  -nosound     Turn the sound and music off\n"
			"  -window      Start the game in a window\n"
            ,
			name
		);
	}
	return success;
}

static std::string findRootGameResFolder(const std::string &configPath)
{
  MicroIni::File configFile;
  if(!configFile.load(configPath) || !configFile[""].has("data_dir"))
  {
    SLOGW(DEBUG_TAG_SGP, "Could not open configuration file (\"%s\").", configPath.c_str());
    WriteDefaultConfigFile(configPath.c_str());
    configFile.load(configPath);
  }

  return configFile[""]["data_dir"];
}

static void WriteDefaultConfigFile(const char* ConfigFile)
{
	FILE* const IniFile = fopen(ConfigFile, "a");
	if (IniFile != NULL)
	{
		fprintf(IniFile, "#Tells ja2-stracciatella where the binary datafiles are located\n");
#ifdef _WIN32
    fprintf(IniFile, "data_dir = C:\\Program Files\\Jagged Alliance 2");
#else
    fprintf(IniFile, "data_dir = /some/place/where/the/data/is");
#endif
		fclose(IniFile);
		fprintf(stderr, "Please edit \"%s\" to point to the binary data.\n", ConfigFile);
	}
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
