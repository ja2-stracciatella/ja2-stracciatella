#ifndef _GAME_VERSION_H_
#define _GAME_VERSION_H_

#include "Types.h"


#ifdef __cplusplus
extern "C" {
#endif


//
//	Keeps track of the game version
//

extern const char g_version_label[];
extern const char czVersionNumber[];


//
//		Keeps track of the saved game version.  Increment the saved game version whenever
//	you will invalidate the saved game file
//

extern const UINT32 guiSavedGameVersion;

#ifdef __cplusplus
}
#endif


#endif
