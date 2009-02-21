#ifdef JA2EDITOR
#ifndef EDITSCREEN_H
#define EDITSCREEN_H

#include "JA2Types.h"
#include "ScreenIDs.h"


#define EDITOR_LIGHT_MAX		(SHADE_MIN + SHADE_MAX)
#define EDITOR_LIGHT_FAKE		(EDITOR_LIGHT_MAX - SHADE_MAX - 2)

extern BOOLEAN gfFakeLights;

void DisplayWayPoints(void);

/* For all lights that are in the world (except lights attached to mercs), this
 * function places a marker at its location for editing purposes. */
void ShowLightPositionHandles(void);

extern void ShowCurrentDrawingMode( void );

// Create and place a light of selected radius into the world.
BOOLEAN PlaceLight(INT16 radius, GridNo pos);

BOOLEAN RemoveLight(GridNo pos);
extern BOOLEAN gfMercResetUponEditorEntry;

//These go together.  The taskbar has a specific color scheme.
extern UINT16 gusEditorTaskbarColor;
extern UINT16 gusEditorTaskbarHiColor;
extern UINT16 gusEditorTaskbarLoColor;

extern INT32 iOldTaskMode;
extern INT32 iCurrentTaskbar;
extern INT32 iTaskMode;

void ProcessAreaSelection( BOOLEAN fWithLeftButton );

void ShowEntryPoints(void);

extern BOOLEAN			gfConfirmExitFirst;
extern BOOLEAN			gfIntendOnEnteringEditor;

void     EditScreenInit(void);
ScreenID EditScreenHandle(void);
void     EditScreenShutdown(void);

#endif
#endif
