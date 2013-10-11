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

#include "slog/slog.h"

#include "Button_System.h"
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
#include "Logger.h"
#include "GameState.h"
#include "Timer.h"

#include "DefaultContentManager.h"
#include "GameInstance.h"
#include "JsonUtility.h"
#include "MicroIni/MicroIni.hpp"
#include "ModPackContentManager.h"
#include "policy/DefaultGamePolicy.h"
#include "sgp/UTF8String.h"

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


#define WITH_MODS (1)

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////

// #include "rapidjson/document.h"
// #include "rapidjson/filestream.h"
// #include "rapidjson/prettywriter.h"
// #include "stdio.h"
// #include "Weapons.h"
// #include "JsonObject.h"
// #include "WeaponModels.h"

// /** Read weapons from json. */
// bool readWeaponsFromJson(const char *fileName)
// {
//   AutoSGPFile f(FileMan::openForReadWrite(fileName));
//   std::string jsonData = FileMan::fileReadText(f);

//   rapidjson::Document document;
//   if (document.Parse<0>(jsonData.c_str()).HasParseError())
//   {
//     return false;
//   }

//   if(document.IsArray()) {
//     const rapidjson::Value& a = document;
//     for (rapidjson::SizeType i = 0; i < a.Size(); i++)
//     {
//       const rapidjson::Value& item = a[i];
//       // printf("%s\n", item["internalName"].GetString());
//       // strings.push_back(a[i].GetString());
//       JsonObjectReader obj(item);
//       WeaponModel *w = WeaponModel::deserialize(obj);
//     }
//   }
//   return true;

// }

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
//       printf("%d\n", i);
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

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////

/**
 * Number of milliseconds for one game cycle.
 * 25 ms gives approx. 40 cycles per second (and 40 frames per second, since the screen
 * is updated on every cycle). */
#define MS_PER_GAME_CYCLE               (25)


static BOOLEAN gfGameInitialized = FALSE;


static std::string findRootGameResFolder(const std::string &configPath);
static void WriteDefaultConfigFile(const char* ConfigFile);

static void convertDialogQuotesToJson(const DefaultContentManager *cm,
                                      STRING_ENC_TYPE encType,
                                      const char *dialogFile, const char *outputFile);

/** Deinitialize the game an exit. */
static void deinitGameAndExit()
{
	FastDebugMsg("Exiting Game");

	SoundServiceStreams();

	if (gfGameInitialized)
	{
		ShutdownGame();
	}

	ShutdownButtonSystem();
	MSYS_Shutdown();

#ifndef UTIL
  ShutdownSoundManager();
#endif

#ifdef SGP_VIDEO_DEBUGGING
	PerformVideoInfoDumpIntoFile( "SGPVideoShutdownDump.txt", FALSE );
#endif

	ShutdownVideoSurfaceManager();
  ShutdownVideoObjectManager();
  ShutdownVideoManager();

#ifdef EXTREME_MEMORY_DEBUGGING
	DumpMemoryInfoIntoFile( "ExtremeMemoryDump.txt", FALSE );
#endif

  ShutdownMemoryManager();  // must go last, for MemDebugCounter to work right...

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

static void MainLoop()
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
				case SDL_ACTIVEEVENT:
					if (event.active.state & SDL_APPACTIVE)
					{
						s_doGameCycles = (event.active.gain != 0);
						break;
					}
					break;

				case SDL_KEYDOWN: KeyDown(&event.key.keysym); break;
				case SDL_KEYUP:   KeyUp(  &event.key.keysym); break;

				case SDL_MOUSEBUTTONDOWN: MouseButtonDown(&event.button); break;
				case SDL_MOUSEBUTTONUP:   MouseButtonUp(&event.button);   break;

				case SDL_MOUSEMOTION:
					gusMouseXPos = event.motion.x;
					gusMouseYPos = event.motion.y;
					break;

				case SDL_QUIT:
          deinitGameAndExit();
					break;
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

        if(gameCycleMS < MS_PER_GAME_CYCLE)
        {
          SDL_Delay(MS_PER_GAME_CYCLE - gameCycleMS);
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


static int Failure(char const* const msg, bool showInfoIcon=false)
{
	fprintf(stderr, "%s\n", msg);
#if defined _WIN32
	MessageBox(0, msg, APPLICATION_NAME, MB_OK | (showInfoIcon ? MB_ICONINFORMATION : MB_ICONERROR) | MB_TASKMODAL);
#endif
	return EXIT_FAILURE;
}

////////////////////////////////////////////////////////////

static DefaultGamePolicy g_gamePolicy;

const GamePolicy *GGP = &g_gamePolicy;
ContentManager *GCM = NULL;

////////////////////////////////////////////////////////////

struct CommandLineParams
{
  CommandLineParams()
  {
#ifdef WITH_MODS
    useMod = false;
#endif
    doUnitTests = false;
    showDebugMessages = false;
  }

#ifdef WITH_MODS
  bool useMod;
  std::string modName;
#endif

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

  ////////////////////////////////////////////////////////////

	SDL_Init(SDL_INIT_VIDEO);
	SDL_EnableUNICODE(SDL_ENABLE);

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
	FastDebugMsg("Initializing Memory Manager");
	InitializeMemoryManager();

  FastDebugMsg("Initializing Game Resources");
  std::string configFolderPath = FileMan::findConfigFolderAndSwitchIntoIt();
  std::string configPath = FileMan::joinPaths(configFolderPath, "ja2.ini");
  std::string gameResRootPath = findRootGameResFolder(configPath);

  std::string externalizedDataPath = FileMan::joinPaths(exeFolder, "externalized");

  DefaultContentManager *cm;

#ifdef WITH_MODS
  if(params.useMod)
  {
    std::string modName = params.modName;
    std::string modResFolder = FileMan::joinPaths(FileMan::joinPaths(FileMan::joinPaths(exeFolder, "mods"), modName), "data");
    cm = new ModPackContentManager(modName, modResFolder, configFolderPath,
                                   configPath, gameResRootPath,
                                   externalizedDataPath);
    LOG_INFO("------------------------------------------------------------------------------\n");
    LOG_INFO("Configuration file:            '%s'\n", configPath.c_str());
    LOG_INFO("Root game resources directory: '%s'\n", gameResRootPath.c_str());
    LOG_INFO("Externalized data directory:   '%s'\n", cm->getExternalizedDataDir().c_str());
    LOG_INFO("Data directory:                '%s'\n", cm->getDataDir().c_str());
    LOG_INFO("Tilecache directory:           '%s'\n", cm->getTileDir().c_str());
    LOG_INFO("Saved games directory:         '%s'\n", cm->getSavedGamesFolder().c_str());
    LOG_INFO("------------------------------------------------------------------------------\n");
    LOG_INFO("MOD name:                      '%s'\n", modName.c_str());
    LOG_INFO("MOD resource directory:        '%s'\n", modResFolder.c_str());
    LOG_INFO("------------------------------------------------------------------------------\n");
  }
  else
#endif
  {
    cm = new DefaultContentManager(configFolderPath, configPath,
                                   gameResRootPath, externalizedDataPath);
    LOG_INFO("------------------------------------------------------------------------------\n");
    LOG_INFO("Configuration file:            '%s'\n", configPath.c_str());
    LOG_INFO("Root game resources directory: '%s'\n", gameResRootPath.c_str());
    LOG_INFO("Externalized data directory:   '%s'\n", cm->getExternalizedDataDir().c_str());
    LOG_INFO("Data directory:                '%s'\n", cm->getDataDir().c_str());
    LOG_INFO("Tilecache directory:           '%s'\n", cm->getTileDir().c_str());
    LOG_INFO("Saved games directory:         '%s'\n", cm->getSavedGamesFolder().c_str());
    LOG_INFO("------------------------------------------------------------------------------\n");
  }

  if(!cm->loadGameData())
  {
    LOG_INFO("Failed to load the game data.\n");
  }
  else
  {

    GCM = cm;

    FastDebugMsg("Initializing Video Manager");
    InitializeVideoManager();

    FastDebugMsg("Initializing Video Object Manager");
    InitializeVideoObjectManager();

    FastDebugMsg("Initializing Video Surface Manager");
    InitializeVideoSurfaceManager();

#ifdef JA2
    InitJA2SplashScreen();
#endif

    // Initialize Font Manager
    FastDebugMsg("Initializing the Font Manager");
    // Init the manager and copy the TransTable stuff into it.
    InitializeFontManager();

    FastDebugMsg("Initializing Sound Manager");
#ifndef UTIL
    InitializeSoundManager();
#endif

    FastDebugMsg("Initializing Random");
    // Initialize random number generator
    InitializeRandom(); // no Shutdown

    FastDebugMsg("Initializing Game Manager");
    // Initialize the Game
    InitializeGame();

    gfGameInitialized = TRUE;

    ////////////////////////////////////////////////////////////

    // some data convertion
    // convertDialogQuotesToJson(cm, SE_RUSSIAN, "mercedt/051.edt", FileMan::joinPaths(exeFolder, "051.edt.json").c_str());
    // convertDialogQuotesToJson(cm, SE_RUSSIAN, "mercedt/052.edt", FileMan::joinPaths(exeFolder, "052.edt.json").c_str());
    // convertDialogQuotesToJson(cm, SE_RUSSIAN, "mercedt/055.edt", FileMan::joinPaths(exeFolder, "055.edt.json").c_str());

    // writeWeaponsToJson(FileMan::joinPaths(exeFolder, "externalized/weapons.json").c_str(), MAX_WEAPONS);
    // readWeaponsFromJson(FileMan::joinPaths(exeFolder, "weapon.json").c_str());
    // readWeaponsFromJson(FileMan::joinPaths(exeFolder, "weapon2.json").c_str());

    ////////////////////////////////////////////////////////////

#if defined JA2
    if(isEnglishVersion())
    {
      SetIntroType(INTRO_SPLASH);
    }
#endif

    FastDebugMsg("Running Game");

    /* At this point the SGP is set up, which means all I/O, Memory, tools, etc.
     * are available. All we need to do is attend to the gaming mechanics
     * themselves */
    MainLoop();
  }

  SLOG_Deinit();

  delete cm;
  GCM = NULL;

	return EXIT_SUCCESS;
}
catch (const std::bad_alloc&)
{
	return Failure("ERROR: out of memory");
}
catch (const LibraryFileNotFoundException& e)
{
	return Failure(e.what(), true);
}
catch (const std::exception& e)
{
	char msg[2048];
	snprintf(msg, lengthof(msg), "ERROR: caught unhandled exception:\n%s", e.what());
	return Failure(msg);
}
catch (...)
{
	return Failure("ERROR: caught unhandled unknown exception");
}


/** Set game resources version. */
static BOOLEAN setResourceVersion(const char *version)
{
  if(strcasecmp(version, "ENGLISH") == 0)
  {
    setGameVersion(GV_ENGLISH);
  }
  else if(strcasecmp(version, "DUTCH") == 0)
  {
    setGameVersion(GV_DUTCH);
  }
  else if(strcasecmp(version, "FRENCH") == 0)
  {
    setGameVersion(GV_FRENCH);
  }
  else if(strcasecmp(version, "GERMAN") == 0)
  {
    setGameVersion(GV_GERMAN);
  }
  else if(strcasecmp(version, "ITALIAN") == 0)
  {
    setGameVersion(GV_ITALIAN);
  }
  else if(strcasecmp(version, "POLISH") == 0)
  {
    setGameVersion(GV_POLISH);
  }
  else if(strcasecmp(version, "RUSSIAN") == 0)
  {
    setGameVersion(GV_RUSSIAN);
  }
  else if(strcasecmp(version, "RUSSIAN_GOLD") == 0)
  {
    setGameVersion(GV_RUSSIAN_GOLD);
  }
  else
  {
    LOG_ERROR("Unknown version of the game: %s\n", version);
    return false;
  }
  LOG_INFO("Game version: %s\n", version);
  return true;
}

static BOOLEAN ParseParameters(int argc, char* const argv[],
                               CommandLineParams *params)
{
	const char* const name = *argv;
	if (name == NULL) return TRUE; // argv does not even contain the program name

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
#ifdef WITH_UNITTESTS
    else if (strcmp(argv[i], "-unittests") == 0)
    {
      params->doUnitTests = true;
      return true;
    }
#endif
    else if (strcmp(argv[i], "-debug") == 0)
    {
      params->showDebugMessages = true;
      return true;
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
          LOG_ERROR("Invalid value for command-line key '-res'\n");
          success = FALSE;
        }
        else
        {
          bool result = g_ui.setScreenSize(width, height);
          if(!result)
          {
            LOG_ERROR("Failed to set screen resolution %d x %d\n", width, height);
            success = FALSE;
          }
        }
      }
      else
      {
        LOG_ERROR("Missing value for command-line key '-res'\n");
        success = FALSE;
      }
		}
#ifdef WITH_MODS
    else if (strcmp(argv[i], "-mod") == 0)
    {
      if(haveNextParameter)
      {
        params->useMod = true;
        params->modName = argv[++i];
      }
      else
      {
        LOG_ERROR("Missing value for command-line key '-res'\n");
        success = FALSE;
      }
    }
#endif
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
        success = setResourceVersion(argv[++i]);
      }
      else
      {
        LOG_ERROR("Missing value for command-line key '-resversion'\n");
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
			"  -resversion  Version of the game resources (data files)\n"
			"                 Possible values: DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD\n"
			"                 Default value is ENGLISH\n"
			"                 RUSSIAN is for BUKA Agonia Vlasty release\n"
			"                 RUSSIAN_GOLD is for Gold release\n"
#ifdef WITH_MODS
      "\n"
      "  -mod NAME    MOD name (see folder mods for options, e.g. 'from-russia-with-love')\n"
#endif
			"\n"
			"  -debug       Show debug messages\n"
#ifdef WITH_UNITTESTS
      "  -unittests   Perform unit tests\n"
      "                 ja2.exe -unittests [gtest options]\n"
      "                 E.g. ja2.exe -unittests --gtest_output=\"xml:report.xml\" --gtest_repeat=2\n"
#endif
			"  -editor      Start the map editor (Editor.slf is necessary)\n"
			"  -editorauto  Start the map editor and load sector A9 (Editor.slf is necessary)\n"
			"  -fullscreen  Start the game in fullscreen mode\n"
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
    LOG_WARNING("WARNING: Could not open configuration file (\"%s\").\n", configPath.c_str());
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

////////////////////////////////////////////////////////////////////////////
// some data convertion
////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////
