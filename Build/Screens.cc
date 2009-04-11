#include "AI_Viewer.h"
#include "AniViewScreen.h"
#include "Auto_Resolve.h"
#include "Credits.h"
#include "EditScreen.h"
#include "Fade_Screen.h"
#include "GameInitOptionsScreen.h"
#include "GameScreen.h"
#include "Intro.h"
#include "JAScreens.h"
#include "Laptop.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "MapScreen.h"
#include "MapUtility.h"
#include "MessageBoxScreen.h"
#include "Options_Screen.h"
#include "Quest_Debug_System.h"
#include "SaveLoadScreen.h"
#include "Screens.h"
#include "ShopKeeper_Interface.h"


Screens const GameScreens[] =
{
#ifdef JA2EDITOR
	{ EditScreenInit,       EditScreenHandle,            EditScreenShutdown        },
#else
	{ NULL,                 NULL,                        NULL                      },
#endif
	{ NULL,                 NULL,                        NULL                      },
	{ NULL,                 NULL,                        NULL                      },
	{ NULL,                 ErrorScreenHandle,           NULL                      }, // Title Screen
	{ NULL,                 InitScreenHandle,            NULL                      }, // Title Screen
	{ MainGameScreenInit,   MainGameScreenHandle,        MainGameScreenShutdown    },
	{ NULL,                 AniEditScreenHandle,         NULL                      },
	{ NULL,                 PalEditScreenHandle,         NULL                      },
	{ NULL,                 DebugScreenHandle,           NULL                      },
	{ MapScreenInit,        MapScreenHandle,             MapScreenShutdown         },
	{ LaptopScreenInit,     LaptopScreenHandle,          LaptopScreenShutdown      },
#ifdef JA2EDITOR
	{ NULL,                 LoadSaveScreenHandle,        NULL                      },
	{ NULL,                 MapUtilScreenHandle,         NULL                      },
#else
	{ NULL,                 NULL,                        NULL                      },
	{ NULL,                 NULL,                        NULL                      },
#endif
	{ NULL,                 FadeScreenHandle,            NULL                      },
	{ NULL,                 MessageBoxScreenHandle,      MessageBoxScreenShutdown  },
	{ NULL,                 MainMenuScreenHandle,        NULL                      },
	{ NULL,                 AutoResolveScreenHandle,     AutoResolveScreenShutdown },
	{ NULL,                 SaveLoadScreenHandle,        NULL                      },
	{ OptionsScreenInit,    OptionsScreenHandle,         NULL                      },
	{ ShopKeeperScreenInit, ShopKeeperScreenHandle,      ShopKeeperScreenShutdown  },
	{ NULL,                 SexScreenHandle,             NULL                      },
	{ NULL,                 GameInitOptionsScreenHandle, NULL                      },
#ifdef JA2DEMOADS
	{ NULL,                 DemoExitScreenHandle,        NULL                      },
#else
	{ NULL,                 NULL,                        NULL                      },
#endif
	{ NULL,                 IntroScreenHandle,           NULL                      },
	{ NULL,                 CreditScreenHandle,          NULL                      },
#ifdef JA2BETAVERSION
	{ AIViewerScreenInit,   AIViewerScreenHandle,        NULL                      },
#endif
	{ QuestDebugScreenInit, QuestDebugScreenHandle,      NULL                      }
};
CASSERT(lengthof(GameScreens) == MAX_SCREENS)
