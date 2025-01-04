#ifndef __EDITOR_TASKBAR_UTILS_H
#define __EDITOR_TASKBAR_UTILS_H

#include <string_theory/string>
#include "Types.h"

//These are utilities that are used within the editor.  This function absorbs the expensive
//compile time of the SGP dependencies, while allowing the other editor files to hook into it
//without paying, so to speak.


enum
{
	GUI_CLEAR_EVENT,
	GUI_LCLICK_EVENT,
	GUI_RCLICK_EVENT,
	GUI_MOVE_EVENT
};

//Taskbar manipulation functions
void DoTaskbar(void);
void ProcessEditorRendering(void);
void EnableEditorTaskbar(void);

/* Disable the task bar, but leave it on screen. Used when a selection window is
 * up. */
void DisableEditorTaskbar();

void CreateEditorTaskbar(void);
void DeleteEditorTaskbar();

//Button manipulation functions
void ClickEditorButton( INT32 iEditorButtonID );
void UnclickEditorButton( INT32 iEditorButtonID );
void HideEditorButton( INT32 iEditorButtonID );
void ShowEditorButton( INT32 iEditorButtonID );
void DisableEditorButton( INT32 iEditorButtonID );
void EnableEditorButton( INT32 iEditorButtonID );

void UnclickEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID );
void HideEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID );
void ShowEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID );
void DisableEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID );
void EnableEditorButtons( INT32 iFirstEditorButtonID, INT32 iLastEditorButtonID );

//Region Utils
#define NUM_TERRAIN_TILE_REGIONS		9
enum{
	BASE_TERRAIN_TILE_REGION_ID,
	ITEM_REGION_ID = NUM_TERRAIN_TILE_REGIONS,
	MERC_REGION_ID,
};

void EnableEditorRegion( INT8	bRegionID );
void DisableEditorRegion( INT8 bRegionID );


//Rendering Utils
void MPrintEditor(INT16 x, INT16 y, const ST::string& str);
void ClearTaskbarRegion( INT16 sLeft, INT16 sTop, INT16 sRight, INT16 sBottom );
void DrawEditorInfoBox(const ST::string& str, SGPFont, UINT16 x, UINT16 y, UINT16 w, UINT16 h);

extern INT32	giEditMercDirectionIcons[2];
extern SGPVObject* guiMercInventoryPanel;
extern SGPVObject* guiOmertaMap;
extern SGPVSurface* guiMercInvPanelBuffers[9];
extern SGPVSurface* guiMercTempBuffer;

#endif
