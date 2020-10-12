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
#include "RustInterface.h"

#include "Logger.h"

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"
#endif

#if defined _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <typeinfo>
#	include "Local.h"
#endif

#ifdef __ANDROID__
#include "jni.h"
#endif

#include "Multi_Language_Graphic_Utils.h"

#include <string_theory/format>

#include <exception>
#include <locale>
#include <new>
#include <utility>

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

				if(static_cast<int>(gameCycleMS) < msPerGameCycle)
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

/// Sets the C/C++ locale.
/// @return true if successful, false otherwise
static bool SetGlobalLocale(const char* name)
{
	try
	{
		std::locale::global(std::locale(name));
		return true;
	}
	catch(...)
	{
		return false;
	}
}

/// Tries to set the C/C++ locale to something that supports unicode.
///
/// There is no way to query available locales so you can only try them.
///
/// @return List of problems.
std::vector<ST::string> InitGlobalLocale()
{
	std::vector<ST::string> problems;

#ifdef _WIN32
	// In windows the console is a special device that accepts CP_UTF8, but needs a true type font to display it.
	if (!SetConsoleOutputCP(CP_UTF8))
	{
		problems.emplace_back(std::move(ST::format("SetConsoleOutputCP(CP_UTF8) failed, using output code page {}", GetConsoleOutputCP())));
	}
	if (!SetConsoleCP(CP_UTF8))
	{
		problems.emplace_back(std::move(ST::format("SetConsoleCP(CP_UTF8) failed, using input code page {}", GetConsoleCP())));
	}
	 
	// Ensure quick-edit mode is off, or else it will block execution
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS);
#endif

#ifdef WITH_CUSTOM_LOCALE
	if (!SetGlobalLocale(WITH_CUSTOM_LOCALE))
	{
		problems.emplace_back(std::move(ST::format("failed to set custom locale '{}'", WITH_CUSTOM_LOCALE)));
	}
	else
	{
		return problems; // the custom locale is set, assume unicode locale (no way to test)
	}
#endif

#ifndef _WIN32
	// Windows does not support the utf8 locale (".65001") or the LC_* environment variables.
	// CP_UTF8 (65001) is a pseudo code page that does not have a nls file.
	// With VS2003 setlocale would accept it but the CRT APIs would fail, now it fails directly.
	//
	// According to https://www.python.org/dev/peps/pep-0538/
	// *nix have one of "C.UTF-8", "C.utf8" or "UTF-8".
	// Mac OS X and other *BSD systems have a partial UTF-8 locale that only defines the LC_CTYPE category.

	// set locale from the process environment
	if (!SetGlobalLocale(""))
	{
		problems.emplace_back("failed to set locale from the process environment");
	}

	// TODO how to set a unicode LC_CTYPE for the C++ locale?
	if (!setlocale(LC_CTYPE, "C.UTF-8") && !setlocale(LC_CTYPE, "C.utf8") && !setlocale(LC_CTYPE, "UTF-8"))
	{
		problems.emplace_back(ST::format("failed to set unicode ctype for locale '{}', using ctype '{}'", setlocale(LC_ALL, nullptr), setlocale(LC_CTYPE, nullptr)));
	}
#endif

	return problems;
}

int main(int argc, char* argv[])
{
    try {
		// init locale and logging
		{
			std::vector<ST::string> problems = InitGlobalLocale();
			Logger_initialize("ja2.log");
			for (const ST::string& msg : problems)
			{
				SLOGW("%s", msg.c_str());
			}
		}

		#ifdef __ANDROID__
		JNIEnv* jniEnv = (JNIEnv*)SDL_AndroidGetJNIEnv();
		
		if (setGlobalJniEnv(jniEnv) == FALSE) {
			auto rustError = getRustError();
			if (rustError != NULL) {
				SLOGE("Failed to set global JNI env for Android: %s", rustError);
			}
			return EXIT_FAILURE;
		}
		#endif
		RustPointer<char> configFolderPath(EngineOptions_getStracciatellaHome());
		if (configFolderPath.get() == NULL) {
			auto rustError = getRustError();
			if (rustError != NULL) {
				SLOGE("Failed to find home directory: %s", rustError);
			}
			return EXIT_FAILURE;
		}

		ST::string exeFolder = FileMan::getParentPath(argv[0], true);

		RustPointer<EngineOptions> params(EngineOptions_create(configFolderPath.get(), argv, argc));
		if (params == NULL) {
			auto rustError = getRustError();
			if (rustError != NULL) {
				SLOGE("Failed to load configuration: %s", rustError);
			}
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

		if (EngineOptions_shouldStartInDebugMode(params.get())) {
			Logger_setLevel(LogLevel::Debug);
			GameState::getInstance()->setDebugging(true);
		}

		if (EngineOptions_shouldRunEditor(params.get())) {
			GameState::getInstance()->setEditorMode(false);
		}

		uint16_t width = EngineOptions_getResolutionX(params.get());
		uint16_t height = EngineOptions_getResolutionY(params.get());
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
		RustPointer<char> gameResRootPath(EngineOptions_getVanillaGameDir(params.get()));

		RustPointer<char> extraDataDir(Env_assetsDir());
		ST::string externalizedDataPath = FileMan::joinPaths(extraDataDir.get(), "externalized");

		FileMan::switchTmpFolder(configFolderPath.get());

		DefaultContentManager *cm;

		uint32_t n = EngineOptions_getModsLength(params.get());
		if(n > 0)
		{
			std::vector<ST::string> enabledMods;
			for (uint32_t i = 0; i < n; ++i)
			{
				RustPointer<char> modName(EngineOptions_getMod(params.get(), i));
				enabledMods.emplace_back(modName.get());
			}
			cm = new ModPackContentManager(version,
							enabledMods, extraDataDir.get(), configFolderPath.get(),
							gameResRootPath.get(), externalizedDataPath);
			SLOGI("------------------------------------------------------------------------------");
			SLOGI("JA2 Home Dir:                  '%s'", configFolderPath.get());
			SLOGI("Root game resources directory: '%s'", gameResRootPath.get());
			SLOGI("Extra data directory:          '%s'", extraDataDir.get());
			SLOGI("Data directory:                '%s'", cm->getDataDir().c_str());
			SLOGI("Tilecache directory:           '%s'", cm->getTileDir().c_str());
			SLOGI("Saved games directory:         '%s'", cm->getSavedGamesFolder().c_str());
			SLOGI("------------------------------------------------------------------------------");
		}
		else
		{
			cm = new DefaultContentManager(version,
							configFolderPath.get(),
							gameResRootPath.get(), externalizedDataPath);
			SLOGI("------------------------------------------------------------------------------");
			SLOGI("JA2 Home Dir:                  '%s'", configFolderPath.get());
			SLOGI("Root game resources directory: '%s'", gameResRootPath.get());
			SLOGI("Extra data directory:          '%s'", extraDataDir.get());
			SLOGI("Data directory:                '%s'", cm->getDataDir().c_str());
			SLOGI("Tilecache directory:           '%s'", cm->getTileDir().c_str());
			SLOGI("Saved games directory:         '%s'", cm->getSavedGamesFolder().c_str());
			SLOGI("------------------------------------------------------------------------------");
		}

		cm->init(params.get());

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

	} catch (...) {
        TerminationHandler();
        return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifdef __ANDROID__
//  Sets the exception on the NativeExceptionContainer in Android
void TerminationHandler()
{
	auto ex = std::current_exception();
	auto jniEnv = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jclass exceptionContainer = jniEnv->FindClass("io/github/ja2stracciatella/NativeExceptionContainer");
    jfieldID singletonFieldId = jniEnv->GetStaticFieldID(exceptionContainer, "INSTANCE", "Lio/github/ja2stracciatella/NativeExceptionContainer;");
    jobject exceptionContainerSingleton = jniEnv->GetStaticObjectField(exceptionContainer, singletonFieldId);
    jmethodID setAndroidExceptionMethodId = jniEnv->GetMethodID(exceptionContainer, "setException","(Ljava/lang/String;)V");

	if (ex)
	{
		try
		{
			std::rethrow_exception(ex);
		}
		catch (const std::exception& e)
		{
			auto errorMessage = ST::format("Game has been terminated due to an unrecoverable error: {} ({})", e.what(), typeid(e).name());
            SLOGE(errorMessage.c_str());
            jniEnv->CallVoidMethod(exceptionContainerSingleton, setAndroidExceptionMethodId,
                                   jniEnv->NewStringUTF(errorMessage.c_str()));
            return;
		}
		catch (...)
		{
			SLOGE("Game has been terminated due to an unknown error");
            jniEnv->CallVoidMethod(exceptionContainerSingleton, setAndroidExceptionMethodId,
                                   jniEnv->NewStringUTF("Game has been terminated due to an unknown error"));
            return;
		}
	}
    jniEnv->CallVoidMethod(exceptionContainerSingleton, setAndroidExceptionMethodId,
                           jniEnv->NewStringUTF("Game has been terminated due to an unknown error"));
}
#else
//  Prints the exception message (if any) and abort
void TerminationHandler()
{
	auto ex = std::current_exception();
	if (ex)
	{
		try
		{
			std::rethrow_exception(ex);
		}
		catch (const std::exception& e) 
		{
			SLOGE(ST::format("Game has been terminated due to an unrecoverable error: {} ({})", e.what(), typeid(e).name()));
		}
		catch (...)
		{
			SLOGE("Game has been terminated due to an unknown error");
		}
	}
	std::abort();
}
#endif

/*static void convertDialogQuotesToJson(const DefaultContentManager *cm,
					STRING_ENC_TYPE encType,
					const char *dialogFile, const char *outputFile)
{
	std::vector<ST::string*> quotes;
	std::vector<ST::string> quotes_str;
	cm->loadAllDialogQuotes(encType, dialogFile, quotes);
	for(int i = 0; i < quotes.size(); i++)
	{
		quotes_str.push_back(quotes[i]->to_std_string());
		delete quotes[i];
		quotes[i] = nullptr;
	}
	JsonUtility::writeToFile(outputFile, quotes_str);
}*/

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

struct TestStruct {
	int a;
	int b;
	int c[2];
};

struct NonTrivialTestStruct {
	NonTrivialTestStruct() : a(0) {}
	NonTrivialTestStruct(int a) : a(a) {}
	int a;
	int b = 0;
};

TEST(cpp_language, list_initialization)
{
	// since C++11: https://en.cppreference.com/w/cpp/language/list_initialization
	{
		TestStruct tmp{};
		EXPECT_EQ(tmp.a, 0);
		EXPECT_EQ(tmp.b, 0);
		EXPECT_EQ(tmp.c[0], 0);
		EXPECT_EQ(tmp.c[1], 0);
	}
	{
		TestStruct tmp{1, 2, {3, 4}};
		EXPECT_EQ(tmp.a, 1);
		EXPECT_EQ(tmp.b, 2);
		EXPECT_EQ(tmp.c[0], 3);
		EXPECT_EQ(tmp.c[1], 4);
	}
	{
		TestStruct tmp = TestStruct{};
		EXPECT_EQ(tmp.a, 0);
		EXPECT_EQ(tmp.b, 0);
		EXPECT_EQ(tmp.c[0], 0);
		EXPECT_EQ(tmp.c[1], 0);
	}
	{
		TestStruct tmp = TestStruct{1, 2, {3, 4}};
		EXPECT_EQ(tmp.a, 1);
		EXPECT_EQ(tmp.b, 2);
		EXPECT_EQ(tmp.c[0], 3);
		EXPECT_EQ(tmp.c[1], 4);
	}
	{
		TestStruct tmp = {};
		EXPECT_EQ(tmp.a, 0);
		EXPECT_EQ(tmp.b, 0);
		EXPECT_EQ(tmp.c[0], 0);
		EXPECT_EQ(tmp.c[1], 0);
	}
	{
		TestStruct tmp = {1, 2, {3, 4}};
		EXPECT_EQ(tmp.a, 1);
		EXPECT_EQ(tmp.b, 2);
		EXPECT_EQ(tmp.c[0], 3);
		EXPECT_EQ(tmp.c[1], 4);
	}
}

// get initialized memory from new
TEST(cpp_language, new_initialization)
{
	{
		int* tmp = new int();
		EXPECT_EQ(*tmp, 0);
		delete tmp;
	}
	{
		int* tmp = new int(123);
		EXPECT_EQ(*tmp, 123);
		delete tmp;
	}
	{
		int* tmp = new int{123};
		EXPECT_EQ(*tmp, 123);
		delete tmp;
	}
	{
		// avoid this pattern, it's uninitialized memory for trivial structs (PODs)
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct;
		EXPECT_EQ(tmp->a, 0);
		EXPECT_EQ(tmp->b, 0);
		delete tmp;
	}
	{
		TestStruct* tmp = new TestStruct();
		EXPECT_EQ(tmp->a, 0);
		EXPECT_EQ(tmp->b, 0);
		EXPECT_EQ(tmp->c[0], 0);
		EXPECT_EQ(tmp->c[1], 0);
		delete tmp;
	}
	{
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct();
		EXPECT_EQ(tmp->a, 0);
		EXPECT_EQ(tmp->b, 0);
		delete tmp;
	}
	{
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct(123);
		EXPECT_EQ(tmp->a, 123);
		EXPECT_EQ(tmp->b, 0);
		delete tmp;
	}
	{
		TestStruct* tmp = new TestStruct{};
		EXPECT_EQ(tmp->a, 0);
		EXPECT_EQ(tmp->b, 0);
		EXPECT_EQ(tmp->c[0], 0);
		EXPECT_EQ(tmp->c[1], 0);
		delete tmp;
	}
	{
		TestStruct* tmp = new TestStruct{1, 2, {3, 4}};
		EXPECT_EQ(tmp->a, 1);
		EXPECT_EQ(tmp->b, 2);
		EXPECT_EQ(tmp->c[0], 3);
		EXPECT_EQ(tmp->c[1], 4);
		delete tmp;
	}
	{
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct{};
		EXPECT_EQ(tmp->a, 0);
		EXPECT_EQ(tmp->b, 0);
		delete tmp;
	}
	{
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct{123};
		EXPECT_EQ(tmp->a, 123);
		EXPECT_EQ(tmp->b, 0);
		delete tmp;
	}
}

// get initialized memory from new[]
TEST(cpp_language, new_array_initialization)
{
	{
		int* tmp = new int[2]();
		EXPECT_EQ(tmp[0], 0);
		EXPECT_EQ(tmp[1], 0);
		delete[] tmp;
	}
	{
		int* tmp = new int[2]{};
		EXPECT_EQ(tmp[0], 0);
		EXPECT_EQ(tmp[1], 0);
		delete[] tmp;
	}
	{
		int* tmp = new int[2]{123};
		EXPECT_EQ(tmp[0], 123);
		EXPECT_EQ(tmp[1], 0);
		delete[] tmp;
	}
	{
		int* tmp = new int[2]{123, 456};
		EXPECT_EQ(tmp[0], 123);
		EXPECT_EQ(tmp[1], 456);
		delete[] tmp;
	}
	{
		// avoid this pattern, it's uninitialized memory for trivial structs (PODs)
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct[2];
		EXPECT_EQ(tmp[0].a, 0);
		EXPECT_EQ(tmp[0].b, 0);
		EXPECT_EQ(tmp[1].a, 0);
		EXPECT_EQ(tmp[1].b, 0);
		delete[] tmp;
	}
	{
		TestStruct* tmp = new TestStruct[2]();
		EXPECT_EQ(tmp[0].a, 0);
		EXPECT_EQ(tmp[0].b, 0);
		EXPECT_EQ(tmp[0].c[0], 0);
		EXPECT_EQ(tmp[0].c[1], 0);
		EXPECT_EQ(tmp[1].a, 0);
		EXPECT_EQ(tmp[1].b, 0);
		EXPECT_EQ(tmp[1].c[0], 0);
		EXPECT_EQ(tmp[1].c[1], 0);
		delete[] tmp;
	}
	{
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct[2]();
		EXPECT_EQ(tmp[0].a, 0);
		EXPECT_EQ(tmp[0].b, 0);
		EXPECT_EQ(tmp[1].a, 0);
		EXPECT_EQ(tmp[1].b, 0);
		delete[] tmp;
	}
	{
		TestStruct* tmp = new TestStruct[2]{};
		EXPECT_EQ(tmp[0].a, 0);
		EXPECT_EQ(tmp[0].b, 0);
		EXPECT_EQ(tmp[0].c[0], 0);
		EXPECT_EQ(tmp[0].c[1], 0);
		EXPECT_EQ(tmp[1].a, 0);
		EXPECT_EQ(tmp[1].b, 0);
		EXPECT_EQ(tmp[1].c[0], 0);
		EXPECT_EQ(tmp[1].c[1], 0);
		delete[] tmp;
	}
	{
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct[2]{};
		EXPECT_EQ(tmp[0].a, 0);
		EXPECT_EQ(tmp[0].b, 0);
		EXPECT_EQ(tmp[1].a, 0);
		EXPECT_EQ(tmp[1].b, 0);
		delete[] tmp;
	}
	{
		// avoid this pattern, it's uninitialized memory for trivial structs (PODs) in VS2015
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct[2]{
			{123}
		};
		EXPECT_EQ(tmp[0].a, 123);
		EXPECT_EQ(tmp[0].b, 0);
		EXPECT_EQ(tmp[1].a, 0);
		EXPECT_EQ(tmp[1].b, 0);
		delete[] tmp;
	}
	{
		// avoid this pattern, it's uninitialized memory for trivial structs (PODs) in VS2015
		NonTrivialTestStruct* tmp = new NonTrivialTestStruct[2]{
			{123},
			{456}
		};
		EXPECT_EQ(tmp[0].a, 123);
		EXPECT_EQ(tmp[0].b, 0);
		EXPECT_EQ(tmp[1].a, 456);
		EXPECT_EQ(tmp[1].b, 0);
		delete[] tmp;
	}
}

TEST(cpp_language, sizeof_type)
{
	EXPECT_EQ(sizeof(char), 1);
	EXPECT_EQ(sizeof(char16_t), 2);
	EXPECT_EQ(sizeof(char32_t), 4);
}

#endif // WITH_UNITTESTS
