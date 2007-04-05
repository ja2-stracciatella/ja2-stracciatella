#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include "Types.h"
#include "ScreenIDs.h"
#include "Sys_Globals.h"
#include "Gameloop.h"


// External globals
UINT8					gubFilename[ 200 ];
BOOLEAN				gfEditMode = FALSE;
INT8					gDebugStr[128];
INT8					gbFPSDisplay = SHOW_MIN_FPS;
BOOLEAN				gfResetInputCheck = FALSE;
BOOLEAN				gfGlobalError			= FALSE;

UINT32	guiGameCycleCounter = 0;

BOOLEAN  SET_ERROR( const char *String, ...)
{
  va_list  ArgPtr;

  va_start(ArgPtr, String);
  vsprintf( gubErrorText, String, ArgPtr);
  va_end(ArgPtr);

	SetPendingNewScreen( ERROR_SCREEN );

	gfGlobalError = TRUE;

	return( FALSE );
}
