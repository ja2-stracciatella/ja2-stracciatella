#ifndef __INTERFACE_CONTROL_H
#define __INTERFACE_CONTROL_H


#define		INTERFACE_MAPSCREEN							0x00000001
#define		INTERFACE_NORENDERBUTTONS				0x00000002
#define		INTERFACE_LOCKEDLEVEL1					0x00000004
#define		INTERFACE_SHOPKEEP_INTERFACE		0x00000008

UINT32		guiTacticalInterfaceFlags;


void SetTacticalInterfaceFlags( UINT32 uiFlags );

void SetUpInterface( );
void ResetInterface( );
void RenderTopmostTacticalInterface( );
void RenderTacticalInterface( );

void RenderTacticalInterfaceWhileScrolling( );

void EraseInterfaceMenus( BOOLEAN fIgnoreUIUnLock );

void ResetInterfaceAndUI( );

BOOLEAN AreWeInAUIMenu( );

void HandleTacticalPanelSwitch( );

BOOLEAN InterfaceOKForMeanwhilePopup();


#endif
