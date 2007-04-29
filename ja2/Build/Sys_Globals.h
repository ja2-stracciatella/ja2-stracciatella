#ifndef __SYS_GLOBALS_H
#define __SYS_GLOBALS_H

#define				SHOW_MIN_FPS			0
#define				SHOW_FULL_FPS			1

char gubErrorText[200];
BOOLEAN				gfAniEditMode;
BOOLEAN				gfEditMode;
BOOLEAN				fFirstTimeInGameScreen;
extern char gDebugStr[128];

INT16					gsCurrentActionPoints;
INT8					gbFPSDisplay;
BOOLEAN				gfGlobalError;

extern UINT32	guiGameCycleCounter;


// VIDEO OVERLAYS
INT32					giFPSOverlay;
INT32					giCounterPeriodOverlay;

extern				BOOLEAN  SET_ERROR( const char *String, ...);


#endif
