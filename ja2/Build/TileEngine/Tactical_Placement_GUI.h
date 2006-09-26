#ifndef __TACTICAL_PLACEMENT_GUI_H
#define __TACTICAL_PLACEMENT_GUI_H

#include "Soldier_Control.h"

void InitTacticalPlacementGUI();
void KillTacticalPlacementGUI();
void TacticalPlacementHandle();
void RenderTacticalPlacementGUI();

void HandleTacticalPlacementClicksInOverheadMap(INT32 reason);

extern BOOLEAN gfTacticalPlacementGUIActive;
extern BOOLEAN gfEnterTacticalPlacementGUI;

extern SOLDIERTYPE *gpTacticalPlacementSelectedSoldier;
extern SOLDIERTYPE *gpTacticalPlacementHilightedSoldier;

//Saved value.  Contains the last choice for future battles.
extern UINT8	gubDefaultButton;

#endif
