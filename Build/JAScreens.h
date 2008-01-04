#ifndef __JASCREENS_H_
#define __JASCREENS_H_

#include "Types.h"

#ifdef JA2EDITOR
extern UINT32 EditScreenInit(void);
extern UINT32 EditScreenHandle(void);
extern UINT32 EditScreenShutdown(void);

extern UINT32 LoadSaveScreenInit(void);
extern UINT32 LoadSaveScreenHandle(void);
#endif

extern UINT32 ErrorScreenHandle(void);

extern UINT32 InitScreenHandle(void);

extern UINT32 MainGameScreenInit(void);
extern UINT32 MainGameScreenHandle(void);
extern UINT32 MainGameScreenShutdown(void);

#ifdef JA2BETAVERSION
extern UINT32 AIViewerScreenInit(void);
extern UINT32 AIViewerScreenHandle(void);
#endif


extern	UINT32	QuestDebugScreenInit(void);
extern	UINT32	QuestDebugScreenHandle(void);

UINT32 AniEditScreenHandle(void);

UINT32 PalEditScreenHandle(void);

UINT32 DebugScreenHandle(void);

UINT32 MapScreenInit(void);
UINT32 MapScreenHandle(void);
UINT32 MapScreenShutdown(void);

UINT32 LaptopScreenInit(void);
UINT32 LaptopScreenHandle(void);
UINT32 LaptopScreenShutdown(void);


#ifdef JA2EDITOR
UINT32	MapUtilScreenHandle( void );
#endif

UINT32	FadeScreenHandle( void );

UINT32	MessageBoxScreenHandle( void );

UINT32	MainMenuScreenInit( void );
UINT32	MainMenuScreenHandle( void );

UINT32	AutoResolveScreenHandle( void );
UINT32	AutoResolveScreenShutdown( void );


UINT32	SaveLoadScreenHandle( void );
UINT32	SaveLoadScreenInit( void );

UINT32	OptionsScreenHandle( void );
UINT32	OptionsScreenInit( void );

UINT32	ShopKeeperScreenInit( void );
UINT32	ShopKeeperScreenHandle( void );
UINT32	ShopKeeperScreenShutdown( void );

UINT32	SexScreenHandle( void );

UINT32	GameInitOptionsScreenHandle( void );

UINT32	DemoExitScreenHandle( void );

extern	UINT32	IntroScreenHandle( void );
extern	UINT32	IntroScreenInit( void );

extern	UINT32	CreditScreenInit( void );
extern	UINT32	CreditScreenHandle( void );


// External functions
void DisplayFrameRate(void);

void HandleTitleScreenAnimation(void);

//External Globals
UINT32				guiCurrentScreen;

typedef void (*RENDER_HOOK)( void );

void SetRenderHook( RENDER_HOOK pRenderOverride );
void SetCurrentScreen( UINT32 uiNewScreen );
void SetDebugRenderHook( RENDER_HOOK pDebugRenderOverride, INT8 ubPage );

void EnableFPSOverlay(BOOLEAN fEnable);

BOOLEAN gfExitDebugScreen;
INT8 gCurDebugPage;

#ifdef JA2DEMOADS
void DoDemoIntroduction(void);
#endif

#endif
