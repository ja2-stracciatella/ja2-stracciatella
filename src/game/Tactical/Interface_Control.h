#ifndef __INTERFACE_CONTROL_H
#define __INTERFACE_CONTROL_H

#include "game/JA2Types.h"


void SetUpInterface();
void ResetInterface();
void RenderTopmostTacticalInterface(void);
void RenderTacticalInterface(void);

void RenderTacticalInterfaceWhileScrolling(void);

void EraseInterfaceMenus( BOOLEAN fIgnoreUIUnLock );

void ResetInterfaceAndUI(void);

bool AreWeInAUIMenu();

void HandleTacticalPanelSwitch();

bool InterfaceOKForMeanwhilePopup();

extern BOOLEAN gfRerenderInterfaceFromHelpText;

/* If given a null pointer, show the team panel. Otherwise show the given merc
 * in the single merc panel. */
void SetNewPanel(SOLDIERTYPE*);

#endif
