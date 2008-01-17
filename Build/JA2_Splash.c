#include "Types.h"
#include "VSurface.h"
#include "Timer_Control.h"
#include "LibraryDataBase.h"
#include "Debug.h"
#include "MainMenuScreen.h"
#include "Video.h"

#ifndef ENGLISH
#	include "Multi_Language_Graphic_Utils.h"
#endif


#include "Libraries.inc"


UINT32 guiSplashFrameFade = 10;
UINT32 guiSplashStartTime = 0;


//Simply create videosurface, load image, and draw it to the screen.
BOOLEAN InitJA2SplashScreen(void)
{
	InitializeJA2Clock();

	STRING512 DataDir;
	sprintf(DataDir, "%s/Data", GetExecutableDirectory());
	if (!SetFileManCurrentDirectory(DataDir))
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Could not find data directory, shutting down");
		return FALSE;
	}

	if (!InitializeFileDatabase(gGameLibaries, lengthof(gGameLibaries))) return FALSE;

#ifdef ENGLISH
	ClearMainMenu();
#else
	SGPFILENAME ImageFile;
	GetMLGFilename(ImageFile, MLG_SPLASH);
	if (!BltVideoSurfaceOnce(FRAME_BUFFER, ImageFile, 0, 0))
	{
		AssertMsg(0, String("Failed to load %s", ImageFile));
		return FALSE;
	}
#endif

	InvalidateScreen();
	RefreshScreen();

	guiSplashStartTime = GetJA2Clock();
	return TRUE;
}
