#ifndef __INTERFACE_CONTROL_H
#define __INTERFACE_CONTROL_H

#include "Types.h"

#define		INTERFACE_MAPSCREEN							0x00000001
#define		INTERFACE_SHOPKEEP_INTERFACE		0x00000008

extern UINT32 guiTacticalInterfaceFlags;


void SetTacticalInterfaceFlags( UINT32 uiFlags );

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
