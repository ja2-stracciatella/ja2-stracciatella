#ifndef _OPTIONS_SCREEN__H_
#define _OPTIONS_SCREEN__H_

#include "ScreenIDs.h"


#define OPT_BUTTON_FONT	FONT14ARIAL
#define OPT_BUTTON_ON_COLOR	73//FONT_MCOLOR_WHITE
#define OPT_BUTTON_OFF_COLOR	73//FONT_MCOLOR_WHITE


//Record the previous screen the user was in.
extern ScreenID guiPreviousOptionScreen;


ScreenID OptionsScreenHandle(void);

#endif
