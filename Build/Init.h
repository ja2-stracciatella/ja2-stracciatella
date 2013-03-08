#ifndef _INIT_H
#define _INIT_H

#include "ScreenIDs.h"


enum GameMode
{
	GAME_MODE_GAME,
#if defined JA2BETAVERSION
	GAME_MODE_MAP_UTILITY,
#endif
#if defined JA2EDITOR
	GAME_MODE_EDITOR,
	GAME_MODE_EDITOR_AUTO,
#endif
	GAME_MODE_END
};

extern GameMode g_game_mode;


ScreenID InitializeJA2(void);
void ShutdownJA2(void);

#endif
