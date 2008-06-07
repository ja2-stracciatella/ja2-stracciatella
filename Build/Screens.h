#ifndef __SCREEN_MANAGER
#define __SCREEN_MANAGER

#include "Types.h"


// Each screen in the game comes with a Status flag (what was the last thing the screen was doing), an Initialization
// function (which loads up the screen if necessary), a Handler function which is called while the screen is showing and
// a shutdown function which is called when the screen is getting ready to make another screen active.

typedef struct Screens
{
  void   (*InitializeScreen)(void);
  UINT32         (*HandleScreen)(void);
	void (*ShutdownScreen)(void);
} Screens;

// These defines are used as flags for each screen state. The only legal transition of states is
// SCR_INACTIVE -> SCR_INITIALIZING -> SCR_ACTIVE -> SCR_SHUTTING_DOWN -> SCR_INACTIVE ... Anything else
// will cause the system to yak.

#define SCR_INACTIVE         0x00
#define SCR_INITIALIZING     0x01
#define SCR_ACTIVE           0x02
#define SCR_SHUTTING_DOWN    0x04

// This extern is made available to make sure that external modules will have access to the screen information

extern Screens const GameScreens[];

#endif
