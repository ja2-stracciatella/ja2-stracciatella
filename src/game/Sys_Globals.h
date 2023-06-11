#ifndef __SYS_GLOBALS_H
#define __SYS_GLOBALS_H

#include <string_theory/string>


inline bool gfEditMode;
inline bool fFirstTimeInGameScreen = true;
inline bool gfGlobalError;

inline unsigned guiGameCycleCounter;

void SET_ERROR(const ST::string& msg);

#endif
