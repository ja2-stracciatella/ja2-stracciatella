#ifndef GAMELOOP_H
#define GAMELOOP_H

#include "ScreenIDs.h"
#include "Types.h"


// main game loop systems
#define INIT_SYSTEM										0


void InitializeGame(void);
void ShutdownGame(void);
void GameLoop(void);

// handle exit from game due to shortcut key
void HandleShortCutExitState( void );

void SetPendingNewScreen(ScreenID);

extern ScreenID guiPendingScreen;

void NextLoopCheckForEnoughFreeHardDriveSpace(void);

#endif
