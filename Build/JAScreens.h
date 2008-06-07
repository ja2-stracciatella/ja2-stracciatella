#ifndef __JASCREENS_H_
#define __JASCREENS_H_

#include "Types.h"


extern UINT32 ErrorScreenHandle(void);

extern UINT32 InitScreenHandle(void);

UINT32 PalEditScreenHandle(void);

UINT32 DebugScreenHandle(void);

UINT32	SexScreenHandle( void );

// External functions
void DisplayFrameRate(void);

void HandleTitleScreenAnimation(void);

//External Globals
extern UINT32 guiCurrentScreen;

typedef void (*RENDER_HOOK)( void );

void SetRenderHook( RENDER_HOOK pRenderOverride );
void SetDebugRenderHook( RENDER_HOOK pDebugRenderOverride, INT8 ubPage );

void EnableFPSOverlay(BOOLEAN fEnable);

extern BOOLEAN gfExitDebugScreen;
extern INT8    gCurDebugPage;

#ifdef JA2DEMOADS
void DoDemoIntroduction(void);
UINT32 DemoExitScreenHandle(void);
#endif

#endif
