#include <stdarg.h>
#include "Types.h"
#include "ScreenIDs.h"
#include "Sys_Globals.h"
#include "Gameloop.h"


char g_filename[200];
BOOLEAN				gfEditMode = FALSE;
INT8					gbFPSDisplay = SHOW_MIN_FPS;
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
