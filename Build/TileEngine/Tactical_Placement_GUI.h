#ifndef __TACTICAL_PLACEMENT_GUI_H
#define __TACTICAL_PLACEMENT_GUI_H

#include "Soldier_Control.h"

void InitTacticalPlacementGUI();
void TacticalPlacementHandle();

void HandleTacticalPlacementClicksInOverheadMap(INT32 reason);

extern BOOLEAN gfTacticalPlacementGUIActive;
extern BOOLEAN gfEnterTacticalPlacementGUI;

extern SOLDIERTYPE *gpTacticalPlacementSelectedSoldier;
extern SOLDIERTYPE *gpTacticalPlacementHilightedSoldier;

//Saved value.  Contains the last choice for future battles.
extern UINT8	gubDefaultButton;

extern BOOLEAN gfTacticalPlacementGUIDirty;
extern BOOLEAN gfValidLocationsChanged;
extern INT32   giMercPanelImage;

#endif
