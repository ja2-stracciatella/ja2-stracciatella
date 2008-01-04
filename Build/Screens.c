#include "Screens.h"
#include "JAScreens.h"


const Screens GameScreens[MAX_SCREENS] =
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
	{ LoadSaveScreenInit,   LoadSaveScreenHandle,        NULL                      },
	{ NULL,                 MapUtilScreenHandle,         NULL                      },
#else
	{ NULL,                 NULL,                        NULL                      },
	{ NULL,                 NULL,                        NULL                      },
#endif
	{ NULL,                 FadeScreenHandle,            NULL                      },
	{ NULL,                 MessageBoxScreenHandle,      NULL                      },
	{ MainMenuScreenInit,   MainMenuScreenHandle,        NULL                      },
	{ NULL,                 AutoResolveScreenHandle,     AutoResolveScreenShutdown },
	{ SaveLoadScreenInit,   SaveLoadScreenHandle,        NULL                      },
	{ OptionsScreenInit,    OptionsScreenHandle,         NULL                      },
	{ ShopKeeperScreenInit, ShopKeeperScreenHandle,      ShopKeeperScreenShutdown  },
	{ NULL,                 SexScreenHandle,             NULL                      },
	{ NULL,                 GameInitOptionsScreenHandle, NULL                      },
	{ NULL,                 DemoExitScreenHandle,        NULL                      },
	{ IntroScreenInit,      IntroScreenHandle,           NULL                      },
	{ CreditScreenInit,     CreditScreenHandle,          NULL                      },
#ifdef JA2BETAVERSION
	{ AIViewerScreenInit,   AIViewerScreenHandle,        NULL                      },
#endif
	{ QuestDebugScreenInit, QuestDebugScreenHandle,      NULL                      }
};
