#ifndef __GAMELOOP_
#define __GAMELOOP_

#include "Types.h"

// main game loop systems
#define INIT_SYSTEM										0

#define	NO_PENDING_SCREEN			0xFFFF


extern BOOLEAN InitializeGame(void);
extern void    ShutdownGame(void);
extern void    GameLoop(void);

// handle exit from game due to shortcut key
void HandleShortCutExitState( void );

void SetPendingNewScreen( UINT32 uiNewScreen );


#endif
