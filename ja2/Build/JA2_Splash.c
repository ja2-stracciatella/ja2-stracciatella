#include "Types.h"
#include "VSurface.h"
#include "Timer_Control.h"
#include <stdio.h>
#include "TopicIDs.h"
#include "LibraryDataBase.h"
#include "TopicOps.h"
#include "Debug.h"
#include "MainMenuScreen.h"
#include "Video.h"

#ifndef ENGLISH
#	include "Multi_Language_Graphic_Utils.h"
#endif


#include "Libraries.inc"


UINT32 guiSplashFrameFade = 10;
UINT32 guiSplashStartTime = 0;
extern HVSURFACE ghFrameBuffer;

//Simply create videosurface, load image, and draw it to the screen.
void InitJA2SplashScreen(void)
{
	InitializeJA2Clock();

	STRING512 DataDir;
	sprintf(DataDir, "%s/Data", GetExecutableDirectory());
	if (!SetFileManCurrentDirectory(DataDir))
	{
		DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "Could not find data directory, shutting down");
		return;
	}

	InitializeFileDatabase(gGameLibaries, lengthof(gGameLibaries));

#ifdef ENGLISH
	ClearMainMenu();
#else
	SGPFILENAME ImageFile;
	GetMLGFilename(ImageFile, MLG_SPLASH);
	UINT32 uiLogoID = AddVideoSurfaceFromFile(ImageFile);
	if (uiLogoID == NO_VSURFACE)
	{
		AssertMsg(0, String("Failed to load %s", ImageFile));
		return;
	}

	BltVideoSurface(FRAME_BUFFER, uiLogoID, 0, 0, NULL);
	DeleteVideoSurfaceFromIndex(uiLogoID);
#endif

	InvalidateScreen();
	RefreshScreen();

	guiSplashStartTime = GetJA2Clock();
}
