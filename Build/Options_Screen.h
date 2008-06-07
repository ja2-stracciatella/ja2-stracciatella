#ifndef _OPTIONS_SCREEN__H_
#define _OPTIONS_SCREEN__H_

#include "Types.h"

#define		OPT_BUTTON_FONT												FONT14ARIAL
#define		OPT_BUTTON_ON_COLOR										73//FONT_MCOLOR_WHITE
#define		OPT_BUTTON_OFF_COLOR									73//FONT_MCOLOR_WHITE


//Record the previous screen the user was in.
extern UINT32 guiPreviousOptionScreen;


void   OptionsScreenInit(void);
UINT32	OptionsScreenHandle( void );

#endif
