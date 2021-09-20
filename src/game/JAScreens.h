#ifndef __JASCREENS_H_
#define __JASCREENS_H_

#include "ScreenIDs.h"
#include "Types.h"


#define INTRO_SPLASH_DURATION 500

ScreenID ErrorScreenHandle(void);

ScreenID InitScreenHandle(void);

ScreenID PalEditScreenHandle(void);

ScreenID DebugScreenHandle(void);

ScreenID SexScreenHandle(void);

// External functions
void DisplayFrameRate(void);

void HandleTitleScreenAnimation(void);

//External Globals
extern ScreenID guiCurrentScreen;

typedef void (*RENDER_HOOK)( void );

void SetRenderHook( RENDER_HOOK pRenderOverride );
void SetDebugRenderHook( RENDER_HOOK pDebugRenderOverride, INT8 ubPage );

void EnableFPSOverlay(BOOLEAN fEnable);

extern BOOLEAN gfExitDebugScreen;
extern INT8    gCurDebugPage;

#endif
