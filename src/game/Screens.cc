#include "Auto_Resolve.h"
#include "EditScreen.h"
#include "Fade_Screen.h"
#include "GameScreen.h"
#include "Intro.h"
#include "JAScreens.h"
#include "Laptop.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "MapScreen.h"
#include "MessageBoxScreen.h"
#include "Options_Screen.h"
#include "Quest_Debug_System.h"
#include "SaveLoadScreen.h"
#include "Screens.h"
#include "ScreenIDs.h"
#include "ShopKeeper_Interface.h"


Screens const GameScreens[] =
{
	{ EditScreenInit,       EditScreenHandle,            EditScreenShutdown       },
	{ NULL,                 NULL,                        NULL                     },
	{ NULL,                 NULL,                        NULL                     },
	{ NULL,                 ErrorScreenHandle,           NULL                     }, // Title Screen
	{ NULL,                 InitScreenHandle,            NULL                     }, // Title Screen
	{ MainGameScreenInit,   MainGameScreenHandle,        MainGameScreenShutdown   },
	{ NULL,                 NULL,                        NULL                     }, // was AniEditScreen
	{ NULL,                 PalEditScreenHandle,         NULL                     },
	{ NULL,                 DebugScreenHandle,           NULL                     },
	{ MapScreenInit,        MapScreenHandle,             MapScreenShutdown        },
	{ LaptopScreenInit,     LaptopScreenHandle,          LaptopScreenShutdown     },
	{ NULL,                 LoadSaveScreenHandle,        NULL                     },
	{ nullptr,              HandleScreen<MAPUTILITY_SCREEN>, nullptr              },
	{ NULL,                 FadeScreenHandle,            NULL                     },
	{ NULL,                 MessageBoxScreenHandle,      MessageBoxScreenShutdown },
	{ NULL,                 MainMenuScreenHandle,        NULL                     },
	{ NULL,                 AutoResolveScreenHandle,     NULL                     },
	{ NULL,                 SaveLoadScreenHandle,        NULL                     },
	{ NULL,                 OptionsScreenHandle,         NULL                     },
	{ ShopKeeperScreenInit, ShopKeeperScreenHandle,      ShopKeeperScreenShutdown },
	{ NULL,                 SexScreenHandle,             NULL                     },
	{ nullptr,              HandleScreen<GAME_INIT_OPTIONS_SCREEN>, nullptr       },
	{ NULL,                 NULL,                        NULL                     },
	{ NULL,                 IntroScreenHandle,           NULL                     },
	{ nullptr,              HandleScreen<CREDIT_SCREEN>, nullptr                  },
	{ QuestDebugScreenInit, QuestDebugScreenHandle,      NULL                     }
};


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Screens, asserts)
{
	EXPECT_EQ(lengthof(GameScreens), MAX_SCREENS);
}

#endif
