#ifndef __SYS_GLOBALS_H
#define __SYS_GLOBALS_H

#define				SHOW_MIN_FPS			0
#define				SHOW_FULL_FPS			1

UINT8					gubErrorText[ 200 ];
BOOLEAN				gfAniEditMode;
BOOLEAN				gfEditMode;
BOOLEAN				fFirstTimeInGameScreen;
BOOLEAN				fDirtyRectangleMode;
INT8					gDebugStr[128];
INT8					gSystemDebugStr[128];

BOOLEAN				gfMode;
INT16					gsCurrentActionPoints;
INT8					gbFPSDisplay;
BOOLEAN				gfResetInputCheck;
BOOLEAN				gfGlobalError;

extern UINT32	guiGameCycleCounter;


// VIDEO OVERLAYS
INT32					giFPSOverlay;
INT32					giCounterPeriodOverlay;

extern				BOOLEAN  SET_ERROR( const char *String, ...);


#endif
