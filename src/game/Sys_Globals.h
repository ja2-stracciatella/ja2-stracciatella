#ifndef __SYS_GLOBALS_H
#define __SYS_GLOBALS_H

#define				SHOW_MIN_FPS			0
#define				SHOW_FULL_FPS			1

extern char    gubErrorText[200];
extern BOOLEAN gfAniEditMode;
extern BOOLEAN gfEditMode;
extern BOOLEAN fFirstTimeInGameScreen;
extern INT8    gbFPSDisplay;
extern BOOLEAN gfGlobalError;

extern UINT32	guiGameCycleCounter;

void SET_ERROR(char const* const String, ...);

extern char g_filename[200];

#endif
