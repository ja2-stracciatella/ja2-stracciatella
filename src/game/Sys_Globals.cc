#include <stdarg.h>
#include "Types.h"
#include "ScreenIDs.h"
#include "Sys_Globals.h"
#include "GameLoop.h"


ST::string gubErrorText;
BOOLEAN gfEditMode             = FALSE;
BOOLEAN fFirstTimeInGameScreen = TRUE;
INT8    gbFPSDisplay           = SHOW_MIN_FPS;
BOOLEAN gfGlobalError          = FALSE;

UINT32	guiGameCycleCounter = 0;


void SET_ERROR(const ST::string& msg)
{
	gubErrorText = msg;

	SetPendingNewScreen( ERROR_SCREEN );

	gfGlobalError = TRUE;
}
