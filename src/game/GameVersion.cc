#include "GameVersion.h"

#ifndef GAME_VERSION
	#define GAME_VERSION "XXXXX"
#endif

#define FULL_VERSION "Stracciatella " GAME_VERSION

//
// Keeps track of the game version
//

const char g_version_label[] = FULL_VERSION;

// This version is written into the save files.
// It should remain the same otherwise there will be warning on
// loading the game.
char const g_version_number[16] = "Build 04.12.02";


//
// Keeps track of the saved game version.  Increment the saved game version whenever
// you will invalidate the saved game file

#define SAVE_GAME_VERSION 100

const UINT32 guiSavedGameVersion = SAVE_GAME_VERSION;


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(GameVersion, asserts)
{
	EXPECT_EQ(lengthof(g_version_number), 16);
}

#endif
