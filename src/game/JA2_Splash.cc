#include "Directories.h"
#include "JA2_Splash.h"
#include "VSurface.h"
#include "Timer_Control.h"
#include "MainMenuScreen.h"
#include "Video.h"
#include "GameRes.h"

UINT32 guiSplashFrameFade = 10;
UINT32 guiSplashStartTime = 0;


//Simply create videosurface, load image, and draw it to the screen.
void InitJA2SplashScreen(void)
{
	InitializeJA2Clock();

	if(isEnglishVersion() || isChineseVersion())
	{
		ClearMainMenu();
	}
	else
	{
		const char* const ImageFile = GetMLGFilename(MLG_SPLASH);
		BltVideoSurfaceOnceWithStretch(FRAME_BUFFER, ImageFile);
	}

	InvalidateScreen();
	RefreshScreen();

	guiSplashStartTime = GetJA2Clock();
}
