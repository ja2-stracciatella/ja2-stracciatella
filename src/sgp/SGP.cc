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
#include "GameMode.h"
#include "Timer.h"
#include "Font.h"

#include "DefaultContentManager.h"
#include "GameInstance.h"
#include "JsonUtility.h"
#include "ModPackContentManager.h"
#include "policy/GamePolicy.h"
#include "RustInterface.h"
#include "EnumCodeGen.h"

#include "Logger.h"
#include <iostream>

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
// #include "ItemModel.h"
// #include "MagazineModel.h"
// #include "WeaponModels.h"
// #include "Weapons.h"
// #include "rapidjson/document.h"
// #include <rapidjson/ostreamwrapper.h>
// #include "rapidjson/prettywriter.h"
// #include "stdio.h"
// #include <fstream>

//bool writeItemsToJson(const char *name, uint16_t from, uint16_t until)
//{
//	std::ofstream ofs(name);
//	rapidjson::OStreamWrapper os(ofs);
//	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(os);
//
//	rapidjson::Document document;
//	document.SetArray();
//	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();
//
//	for (int i = from; i < until; i++)
//	{
//		// printf("%d\n", i);
//		auto item = GCM->getItem(i);
//		JsonObject obj(allocator);
//		item->serializeTo(obj);
//		document.PushBack(obj.getValue(), allocator);
//	}
//
//	document.Accept(writer);
//
//	ofs << std::endl;
//	ofs.close();
//
//	return true;
//}

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
static void shutdownGame()
{
	if (gfGameInitialized)
	{
		SLOGD("Shutting Down Game Manager");
		ShutdownGame();
	}

	delete GCM;
	GCM = NULL;

	SLOGD("Shutting Down Button System");
	ShutdownButtonSystem();
	MSYS_Shutdown();

	SLOGD("Shutting Down Sound Manager");
	ShutdownSoundManager();

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
}

/** Deinitialize the game an exit. */
static void deinitGameAndExit()
{
	SLOGD("Deinitializing Game");
	// If we are in Dead is Dead mode, save before exit
	// Does this code also fire on crash? Let's hope not!
	DoDeadIsDeadSaveIfNecessary();

	shutdownGame();

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
				SLOGW("{}", msg);
			}
		}

		#ifdef __ANDROID__
		JNIEnv* jniEnv = (JNIEnv*)SDL_AndroidGetJNIEnv();

		if (setGlobalJniEnv(jniEnv) == FALSE) {
			auto rustError = getRustError();
			if (rustError != NULL) {
				SLOGE("Failed to set global JNI env for Android: {}", rustError);
			}
			return EXIT_FAILURE;
		}
		#endif
		RustPointer<char> configFolderPath(EngineOptions_getStracciatellaHome());
		if (configFolderPath.get() == NULL) {
			auto rustError = getRustError();
			if (rustError != NULL) {
				SLOGE("Failed to find home directory: {}", rustError);
			}
			return EXIT_FAILURE;
		}

		RustPointer<EngineOptions> params(EngineOptions_create(configFolderPath.get(), argv, argc));
		if (params == NULL) {
			return EXIT_FAILURE;
		}

		if (EngineOptions_shouldShowHelp(params.get())) {
			return EXIT_SUCCESS;
		}

		if (EngineOptions_shouldRunEnumGen(params.get())) {
			PrintAllJA2Enums(std::cout);
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
			GameMode::getInstance()->setDebugging(true);
		}

		if (EngineOptions_shouldRunEditor(params.get())) {
			GameMode::getInstance()->setEditorMode(false);
		}

		uint16_t width = EngineOptions_getResolutionX(params.get());
		uint16_t height = EngineOptions_getResolutionY(params.get());
		g_ui.setScreenSize(width, height);

		if (EngineOptions_shouldRunUnittests(params.get())) {
	#ifdef WITH_UNITTESTS
			Logger_setLevel(LogLevel::Error);
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

		// this one needs to go ahead of all others (except Debug), for MemDebugCounter to work right...
		SLOGD("Initializing Memory Manager");
		InitializeMemoryManager();

		SLOGD("Initializing Game Resources");

		DefaultContentManager *cm;

		uint32_t n = EngineOptions_getModsLength(params.get());
		if(n > 0)
		{
			cm = new ModPackContentManager(move(params));
		}
		else
		{
			cm = new DefaultContentManager(move(params));
		}

		cm->logConfiguration();

		if (!cm->loadGameData())
		{
			throw std::runtime_error("Failed to load the game data.");
		}

		GCM = cm;

		g_ui.recalculatePositions();

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
		InitializeSoundManager();

		SLOGD("Initializing Random");
		// Initialize random number generator
		InitializeRandom(); // no Shutdown

		SLOGD("Initializing Game Manager");
		// Initialize the Game
		InitializeGame();

		gfGameInitialized = TRUE;

		if(isEnglishVersion() || isChineseVersion())
		{
			SetIntroType(INTRO_SPLASH);
		}

		SLOGD("Running Game");

		/* At this point the SGP is set up, which means all I/O, Memory, tools, etc.
		* are available. All we need to do is attend to the gaming mechanics
		* themselves */
		MainLoop(gamepolicy(ms_per_game_cycle));

		delete cm;
		GCM = NULL;

		return EXIT_SUCCESS;
	} catch (...) {
		TerminationHandler();
		return EXIT_FAILURE;
	}
}

void TerminationHandler()
{
	auto ex = std::current_exception();
	auto errorMessage = ST::string("Game has been terminated due to an unknown error");
	#ifdef __ANDROID__
	// Pull out some methods from JNI to set error on NativeExceptionContainer
	auto jniEnv = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jclass exceptionContainer = jniEnv->FindClass("io/github/ja2stracciatella/NativeExceptionContainer");
    jfieldID singletonFieldId = jniEnv->GetStaticFieldID(exceptionContainer, "INSTANCE", "Lio/github/ja2stracciatella/NativeExceptionContainer;");
    jobject exceptionContainerSingleton = jniEnv->GetStaticObjectField(exceptionContainer, singletonFieldId);
    jmethodID setAndroidExceptionMethodId = jniEnv->GetMethodID(exceptionContainer, "setException","(Ljava/lang/String;)V");
	#endif

	if (ex)
	{
		try
		{
			std::rethrow_exception(ex);
		}
		catch (const std::exception& e)
		{
			errorMessage = ST::format("Game has been terminated due to an unrecoverable error: {} ({})", e.what(), typeid(e).name());
		}
		catch (...)
		{
		}
	}
	SLOGE(errorMessage.c_str());
	#ifdef __ANDROID__
	jniEnv->CallVoidMethod(exceptionContainerSingleton, setAndroidExceptionMethodId,
                                   jniEnv->NewStringUTF(errorMessage.c_str()));
	#endif
	shutdownGame();
	#ifndef __ANDROID__
	std::abort();
	#endif
}

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
