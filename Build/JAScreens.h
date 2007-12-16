#ifndef __JASCREENS_H_
#define __JASCREENS_H_

#include "Types.h"

extern UINT32 EditScreenInit(void);
extern UINT32 EditScreenHandle(void);
extern UINT32 EditScreenShutdown(void);

extern UINT32 LoadSaveScreenInit(void);
extern UINT32 LoadSaveScreenHandle(void);
extern UINT32 LoadSaveScreenShutdown(void);

extern UINT32 SavingScreenInitialize(void);
extern UINT32 SavingScreenHandle(void);
extern UINT32 SavingScreenShutdown(void);

extern UINT32 LoadingScreenInitialize(void);
extern UINT32 LoadingScreenHandle(void);
extern UINT32 LoadingScreenShutdown(void);

extern UINT32 ErrorScreenInitialize(void);
extern UINT32 ErrorScreenHandle(void);
extern UINT32 ErrorScreenShutdown(void);

extern UINT32 InitScreenInitialize(void);
extern UINT32 InitScreenHandle(void);
extern UINT32 InitScreenShutdown(void);

extern UINT32 MainGameScreenInit(void);
extern UINT32 MainGameScreenHandle(void);
extern UINT32 MainGameScreenShutdown(void);

#ifdef JA2BETAVERSION
extern UINT32 AIViewerScreenInit(void);
extern UINT32 AIViewerScreenHandle(void);
extern UINT32 AIViewerScreenShutdown(void);
#endif


extern	UINT32	QuestDebugScreenInit(void);
extern	UINT32	QuestDebugScreenHandle(void);
extern	UINT32	QuestDebugScreenShutdown(void);

UINT32 AniEditScreenInit(void);
UINT32 AniEditScreenHandle(void);
UINT32 AniEditScreenShutdown(void);

UINT32 PalEditScreenInit(void);
UINT32 PalEditScreenHandle(void);
UINT32 PalEditScreenShutdown(void);

UINT32 DebugScreenInit(void);
UINT32 DebugScreenHandle(void);
UINT32 DebugScreenShutdown(void);

UINT32 MapScreenInit(void);
UINT32 MapScreenHandle(void);
UINT32 MapScreenShutdown(void);

UINT32 LaptopScreenInit(void);
UINT32 LaptopScreenHandle(void);
UINT32 LaptopScreenShutdown(void);


UINT32	MapUtilScreenInit( void );
UINT32	MapUtilScreenHandle( void );
UINT32	MapUtilScreenShutdown( void );

UINT32	FadeScreenInit( void );
UINT32	FadeScreenHandle( void );
UINT32	FadeScreenShutdown( void );

UINT32	MessageBoxScreenInit( void );
UINT32	MessageBoxScreenHandle( void );
UINT32	MessageBoxScreenShutdown( void );

UINT32	MainMenuScreenInit( void );
UINT32	MainMenuScreenHandle( void );
UINT32	MainMenuScreenShutdown( void );

UINT32	AutoResolveScreenInit( void );
UINT32	AutoResolveScreenHandle( void );
UINT32	AutoResolveScreenShutdown( void );


UINT32	SaveLoadScreenShutdown( void );
UINT32	SaveLoadScreenHandle( void );
UINT32	SaveLoadScreenInit( void );

UINT32	OptionsScreenShutdown( void );
UINT32	OptionsScreenHandle( void );
UINT32	OptionsScreenInit( void );

UINT32	ShopKeeperScreenInit( void );
UINT32	ShopKeeperScreenHandle( void );
UINT32	ShopKeeperScreenShutdown( void );

UINT32	SexScreenInit( void );
UINT32	SexScreenHandle( void );
UINT32	SexScreenShutdown( void );

UINT32	GameInitOptionsScreenInit( void );
UINT32	GameInitOptionsScreenHandle( void );
UINT32	GameInitOptionsScreenShutdown( void );

UINT32	DemoExitScreenInit( void );
UINT32	DemoExitScreenHandle( void );
UINT32	DemoExitScreenShutdown( void );

extern	UINT32	IntroScreenShutdown( void );
extern	UINT32	IntroScreenHandle( void );
extern	UINT32	IntroScreenInit( void );

extern	UINT32	CreditScreenInit( void );
extern	UINT32	CreditScreenHandle( void );
extern	UINT32	CreditScreenShutdown( void );


// External functions
void DisplayFrameRate(void);

void HandleTitleScreenAnimation(void);

//External Globals
UINT8					gubFilename[ 200 ];
UINT32				guiCurrentScreen;

typedef void (*RENDER_HOOK)( void );

void SetRenderHook( RENDER_HOOK pRenderOverride );
void SetCurrentScreen( UINT32 uiNewScreen );
void SetDebugRenderHook( RENDER_HOOK pDebugRenderOverride, INT8 ubPage );

void EnableFPSOverlay(BOOLEAN fEnable);

BOOLEAN gfExitDebugScreen;
INT8 gCurDebugPage;

#endif
