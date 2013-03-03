#include "GameVersion.h"

#ifndef GAME_VERSION
  #define GAME_VERSION "XXXXX"
#endif

#ifdef JA2BETAVERSION
  #define FULL_VERSION "Beta v. 0.98";
#else
  #define FULL_VERSION "Stracciatella " GAME_VERSION
#endif

//
//	Keeps track of the game version
//

const char g_version_label[] = FULL_VERSION;

// This version is written into the save files.
// It should remain the same otherwise there will be warning on
// loading the game.
char const g_version_number[16] = "Build 04.12.02";
CASSERT(lengthof(g_version_number) == 16)


//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file

#define			SAVE_GAME_VERSION					99

const	UINT32	guiSavedGameVersion = SAVE_GAME_VERSION;
