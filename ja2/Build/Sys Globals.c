#ifdef PRECOMPILEDHEADERS
	#include "JA2 All.h"
#else
	#include <stdarg.h>
	#include <stdio.h>
	#include <wchar.h>
	#include "types.h"
	#include "screenids.h"
	#include "Sys Globals.h"
	#include "gameloop.h"
#endif


// External globals
UINT8					gubFilename[ 200 ];
BOOLEAN				gfEditMode = FALSE;
INT8					gDebugStr[128];
INT8					gSystemDebugStr[128];
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


