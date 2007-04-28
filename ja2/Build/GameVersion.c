#include "Types.h"
#include "GameVersion.h"


//
//	Keeps track of the game version
//

#ifdef JA2BETAVERSION

//BETA/TEST BUILD VERSION
wchar_t		zVersionLabel[16]		= L"Beta v. 0.98";

#else

	//RELEASE BUILD VERSION
	wchar_t		zVersionLabel[16]		= L"Release v1.12";


#endif

char		czVersionNumber[16]	= "Build 04.12.02";
wchar_t		zTrackingNumber[16]	= L"Z";


//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file

#define			SAVE_GAME_VERSION					99

const	UINT32	guiSavedGameVersion = SAVE_GAME_VERSION;
