#include "GameVersion.h"


//
//	Keeps track of the game version
//

const char g_version_label[] =
#ifdef JA2BETAVERSION
	//BETA/TEST BUILD VERSION
	"Beta v. 0.98";
#else
	//RELEASE BUILD VERSION
	"Release v0.11 - Liquorice";
#endif

char const g_version_number[16] = "Build 04.12.02";
CASSERT(lengthof(g_version_number) == 16);


//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file

#define			SAVE_GAME_VERSION					99

const	UINT32	guiSavedGameVersion = SAVE_GAME_VERSION;
