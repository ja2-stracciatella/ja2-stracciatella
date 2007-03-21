#include "BuildDefines.h"

#ifdef JA2EDITOR
#ifndef EDITSCREEN_H
#define EDITSCREEN_H

#include "Button_System.h"

#define EDITOR_LIGHT_MAX		(SHADE_MIN + SHADE_MAX)
#define EDITOR_LIGHT_FAKE		(EDITOR_LIGHT_MAX - SHADE_MAX - 2)

extern BOOLEAN gfFakeLights;
extern GUI_BUTTON *gpPersistantButton;

extern UINT16 GenericButtonFillColors[40];

void DisplayWayPoints(void);

void ShowLightPositionHandles( void );

extern void ShowCurrentDrawingMode( void );

void CreateNewMap();

void DisplayTilesetPage(void);

BOOLEAN PlaceLight( INT16 sRadius, INT16 iMapX, INT16 iMapY, INT16 sType );
BOOLEAN RemoveLight( INT16 iMapX, INT16 iMapY );
extern BOOLEAN gfMercResetUponEditorEntry;

void ProcessSelectionArea();

extern UINT16 GenericButtonFillColors[40];

//These go together.  The taskbar has a specific color scheme.
extern UINT16 gusEditorTaskbarColor;
extern UINT16 gusEditorTaskbarHiColor;
extern UINT16 gusEditorTaskbarLoColor;

extern INT32 iOldTaskMode;
extern INT32 iCurrentTaskbar;
extern INT32 iTaskMode;

void ProcessAreaSelection( BOOLEAN fWithLeftButton );

void ShowEntryPoints();

extern BOOLEAN			gfConfirmExitFirst;
extern BOOLEAN			gfConfirmExitPending;
extern BOOLEAN			gfIntendOnEnteringEditor;

#endif
#endif
