#ifdef PRECOMPILEDHEADERS
	#include "JA2 All.h"
#else
	#include "Types.h"
	#include "GameVersion.h"
#endif


//
//	Keeps track of the game version
//

#ifdef JA2BETAVERSION

//BETA/TEST BUILD VERSION
INT16		zVersionLabel[16]		= { L"Beta v. 0.98" };

#elif defined CRIPPLED_VERSION

//RELEASE BUILD VERSION
INT16		zVersionLabel[16]		= { L"Beta v. 0.98" };

#else

	//RELEASE BUILD VERSION
	INT16		zVersionLabel[16]		= { L"Release v1.12" };


#endif

INT8		czVersionNumber[16]	= { "Build 04.12.02" };
INT16		zTrackingNumber[16]	= { L"Z" };


//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file

#define			SAVE_GAME_VERSION					99

const	UINT32	guiSavedGameVersion = SAVE_GAME_VERSION;
