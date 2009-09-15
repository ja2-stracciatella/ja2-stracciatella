#ifndef __INTERFACE_CONTROL_H
#define __INTERFACE_CONTROL_H

#include "Types.h"


void SetUpInterface();
void ResetInterface();
void RenderTopmostTacticalInterface(void);
void RenderTacticalInterface(void);

void RenderTacticalInterfaceWhileScrolling(void);

void EraseInterfaceMenus( BOOLEAN fIgnoreUIUnLock );

void ResetInterfaceAndUI(void);

bool AreWeInAUIMenu();

void HandleTacticalPanelSwitch(void);

bool InterfaceOKForMeanwhilePopup();

extern BOOLEAN gfRerenderInterfaceFromHelpText;

#endif
