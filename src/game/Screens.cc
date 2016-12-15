#include "game/Strategic/AI_Viewer.h"
#include "AniViewScreen.h"
#include "game/Strategic/Auto_Resolve.h"
#include "Credits.h"
#include "game/Editor/EditScreen.h"
#include "Fade_Screen.h"
#include "GameInitOptionsScreen.h"
#include "GameScreen.h"
#include "Intro.h"
#include "JAScreens.h"
#include "game/Laptop/Laptop.h"
#include "game/Editor/LoadScreen.h"
#include "MainMenuScreen.h"
#include "game/Strategic/MapScreen.h"
#include "game/Utils/MapUtility.h"
#include "MessageBoxScreen.h"
#include "Options_Screen.h"
#include "game/Strategic/Quest_Debug_System.h"
#include "SaveLoadScreen.h"
#include "Screens.h"
#include "game/Tactical/ShopKeeper_Interface.h"


Screens const GameScreens[] =
{
	{ EditScreenInit,       EditScreenHandle,            EditScreenShutdown       },
	{ NULL,                 NULL,                        NULL                     },
	{ NULL,                 NULL,                        NULL                     },
	{ NULL,                 ErrorScreenHandle,           NULL                     }, // Title Screen
	{ NULL,                 InitScreenHandle,            NULL                     }, // Title Screen
	{ MainGameScreenInit,   MainGameScreenHandle,        MainGameScreenShutdown   },
	{ NULL,                 AniEditScreenHandle,         NULL                     },
	{ NULL,                 PalEditScreenHandle,         NULL                     },
	{ NULL,                 DebugScreenHandle,           NULL                     },
	{ MapScreenInit,        MapScreenHandle,             MapScreenShutdown        },
	{ LaptopScreenInit,     LaptopScreenHandle,          LaptopScreenShutdown     },
	{ NULL,                 LoadSaveScreenHandle,        NULL                     },
	{ NULL,                 MapUtilScreenHandle,         NULL                     },
	{ NULL,                 FadeScreenHandle,            NULL                     },
	{ NULL,                 MessageBoxScreenHandle,      MessageBoxScreenShutdown },
	{ NULL,                 MainMenuScreenHandle,        NULL                     },
	{ NULL,                 AutoResolveScreenHandle,     NULL                     },
	{ NULL,                 SaveLoadScreenHandle,        NULL                     },
	{ NULL,                 OptionsScreenHandle,         NULL                     },
	{ ShopKeeperScreenInit, ShopKeeperScreenHandle,      ShopKeeperScreenShutdown },
	{ NULL,                 SexScreenHandle,             NULL                     },
	{ NULL,                 GameInitOptionsScreenHandle, NULL                     },
	{ NULL,                 NULL,                        NULL                     },
	{ NULL,                 IntroScreenHandle,           NULL                     },
	{ NULL,                 CreditScreenHandle,          NULL                     },
#ifdef JA2BETAVERSION
	{ AIViewerScreenInit,   AIViewerScreenHandle,        NULL                     },
#endif
	{ QuestDebugScreenInit, QuestDebugScreenHandle,      NULL                     }
};


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Screens, asserts)
{
  EXPECT_EQ(lengthof(GameScreens), MAX_SCREENS);
}

#endif
