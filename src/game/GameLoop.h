#ifndef GAMELOOP_H
#define GAMELOOP_H

#include "MessageBoxScreen.h"
#include "ScreenIDs.h"


void InitializeGame(void);
void ShutdownGame(void);
void GameLoop(void);

// handle exit from game due to shortcut key
void HandleShortCutExitState();

void SetPendingNewScreen(ScreenID);

extern ScreenID guiPendingScreen;

void NextLoopCheckForEnoughFreeHardDriveSpace(void);

// callback to confirm game is over
void EndGameMessageBoxCallBack(MessageBoxReturnValue);

#endif
