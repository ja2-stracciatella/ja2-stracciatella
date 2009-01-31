#ifndef _SAVE_LOAD_SCREEN__H_
#define _SAVE_LOAD_SCREEN__H_

#include "JA2Types.h"
#include "MessageBoxScreen.h"
#include "ScreenIDs.h"


#define		NUM_SAVE_GAMES											11


//This flag is used to diferentiate between loading a game and saveing a game.
// gfSaveGame=TRUE		For saving a game
// gfSaveGame=FALSE		For loading a game
extern BOOLEAN gfSaveGame;

//if there is to be a slot selected when entering this screen
extern INT8			gbSetSlotToBeSelected;

extern	BOOLEAN			gbSaveGameArray[ NUM_SAVE_GAMES ];

extern	BOOLEAN			gfCameDirectlyFromGame;

void     SaveLoadScreenInit(void);
ScreenID SaveLoadScreenHandle(void);

void DoSaveLoadMessageBox(wchar_t const* zString, ScreenID uiExitScreen, MessageBoxFlags, MSGBOX_CALLBACK ReturnCallback);

void DoQuickSave(void);
void DoQuickLoad(void);

BOOLEAN IsThereAnySavedGameFiles(void);

void			DeleteSaveGameNumber( UINT8 ubSaveGameSlotID );

#if defined JA2BETAVERSION
void ErrorDetectedInSaveCallback(MessageBoxReturnValue);
#endif

#endif
