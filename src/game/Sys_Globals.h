#ifndef __SYS_GLOBALS_H
#define __SYS_GLOBALS_H

#include <string_theory/string>


#define SHOW_MIN_FPS	0
#define SHOW_FULL_FPS	1

inline BOOLEAN gfEditMode;
inline BOOLEAN fFirstTimeInGameScreen = TRUE;
inline INT8    gbFPSDisplay;
inline BOOLEAN gfGlobalError;

inline UINT32  guiGameCycleCounter;

void SET_ERROR(const ST::string& msg);

#endif
