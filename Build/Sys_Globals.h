#ifndef __SYS_GLOBALS_H
#define __SYS_GLOBALS_H

#define				SHOW_MIN_FPS			0
#define				SHOW_FULL_FPS			1

char gubErrorText[200];
BOOLEAN				gfAniEditMode;
BOOLEAN				gfEditMode;
BOOLEAN				fFirstTimeInGameScreen;

INT16					gsCurrentActionPoints;
INT8					gbFPSDisplay;
BOOLEAN				gfGlobalError;

extern UINT32	guiGameCycleCounter;

extern				BOOLEAN  SET_ERROR( const char *String, ...);

extern char g_filename[200];

#endif
