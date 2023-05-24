#ifndef _GAMESCREEN_H
#define _GAMESCREEN_H

#include "JA2Types.h"
#include "ScreenIDs.h"


#define ARE_IN_FADE_IN( )			( gfFadeIn || gfFadeInitialized )



void FadeInGameScreen(void);
void FadeOutGameScreen(void);

typedef void (*MODAL_HOOK)( void );


extern BOOLEAN gfGameScreenLocateToSoldier;
extern BOOLEAN gfEnteringMapScreen;
extern SOLDIERTYPE* gPreferredInitialSelectedGuy;


void UpdateTeamPanelAssignments(void);


#define TACTICAL_MODAL_NOMOUSE			1
#define TACTICAL_MODAL_WITHMOUSE		2

extern MODAL_HOOK		gModalDoneCallback;

void EnterModalTactical( INT8 bMode );
void EndModalTactical(void);


// handle the entrance of the mercs at the beginning of the game
void InitHelicopterEntranceByMercs( void );

void InternalLeaveTacticalScreen(ScreenID uiNewScreen);

extern BOOLEAN gfBeginEndTurn;

void EnterTacticalScreen(void);
void LeaveTacticalScreen(ScreenID uiNewScreen);

void     MainGameScreenInit(void);
ScreenID MainGameScreenHandle(void);
void     MainGameScreenShutdown(void);

#endif
