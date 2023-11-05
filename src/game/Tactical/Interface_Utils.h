#ifndef __INTERFACE_UTILS_H
#define __INTERFACE_UTILS_H

#include "JA2Types.h"


#define DRAW_ITEM_STATUS_ATTACHMENT1	200
#define DRAW_ITEM_STATUS_ATTACHMENT2	201
#define DRAW_ITEM_STATUS_ATTACHMENT3	202
#define DRAW_ITEM_STATUS_ATTACHMENT4	203

void DrawSoldierUIBars(const SOLDIERTYPE &s, const INT16 sXPos, const INT16 sYPos, const BOOLEAN fErase, SGPVSurface *uiBuffer);

void DrawItemUIBarEx(const OBJECTTYPE &o, const UINT8 ubStatus, const INT16 x, const INT16 y, const INT16 w, INT16 max_h, const UINT32 sColor1, const UINT32 sColor2, SGPVSurface *uiBuffer);

void RenderSoldierFace(SOLDIERTYPE const&, INT16 sFaceX, INT16 sFaceY);

// load portraits for cars
void LoadCarPortraitValues(void);

// get rid of the loaded portraits for cars
void UnLoadCarPortraits( void );

void LoadInterfaceUtilsGraphics();
void DeleteInterfaceUtilsGraphics();

#endif
