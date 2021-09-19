#include "UILayout.h"

#include "MapScreen.h"
#include "Soldier_Control.h"
#include "JAScreens.h"

#define MIN_INTERFACE_WIDTH       640
#define MIN_INTERFACE_HEIGHT      480

/**
 * Default screen layout.
 * It might be changed later when the window size is known for sure. */
UILayout g_ui(MIN_INTERFACE_WIDTH, MIN_INTERFACE_HEIGHT);


/** Constructor. */
UILayout::UILayout(UINT16 screenWidth, UINT16 screenHeight)
	:m_mapScreenWidth(MIN_INTERFACE_WIDTH), m_mapScreenHeight(MIN_INTERFACE_HEIGHT),
	m_screenWidth(screenWidth), m_screenHeight(screenHeight)
{
	recalculatePositions();
}


/** Set new screen size.
 * This method should be called before most of the application initialization is done. */
bool UILayout::setScreenSize(UINT16 width, UINT16 height)
{
	if((width >= MIN_INTERFACE_WIDTH) && (height >= MIN_INTERFACE_HEIGHT))
	{
		m_screenWidth = width;
		m_screenHeight = height;
		recalculatePositions();
		return true;
	}
	return false;
}


/** Check if the screen is bigger than original 640x480. */
bool UILayout::isBigScreen()
{
	return (m_screenWidth > 640) || (m_screenHeight > 480);
}


UINT16 UILayout::currentHeight()             { return fInMapMode ? (STD_SCREEN_Y + m_mapScreenHeight) : m_screenHeight; }
UINT16 UILayout::get_CLOCK_X()               { return fInMapMode ? (STD_SCREEN_X + 554) : 554;               }
UINT16 UILayout::get_CLOCK_Y()               { return currentHeight() - 23;                                  }
UINT16 UILayout::get_RADAR_WINDOW_X()        { return fInMapMode ? (STD_SCREEN_X + 543) : 543;               }
UINT16 UILayout::get_RADAR_WINDOW_TM_Y()     { return currentHeight() - 107;                                 }
UINT16 UILayout::get_INTERFACE_START_Y()     { return m_screenHeight - 120;                                  }
UINT16 UILayout::get_INV_INTERFACE_START_Y() { return m_screenHeight - 140;                                  }


/** Recalculate UI elements' positions after changing screen size. */
void UILayout::recalculatePositions()
{
	UINT16 startInvY = get_INV_INTERFACE_START_Y();

	m_stdScreenOffsetX            = (m_screenWidth - MIN_INTERFACE_WIDTH) / 2;
	m_stdScreenOffsetY            = (m_screenHeight - MIN_INTERFACE_HEIGHT) / 2;

	// tactical screen inventory position
	m_invSlotPositionTac[HELMETPOS           ].set(344, startInvY +   6);
	m_invSlotPositionTac[VESTPOS             ].set(344, startInvY +  35);
	m_invSlotPositionTac[LEGPOS              ].set(344, startInvY +  95);
	m_invSlotPositionTac[HEAD1POS            ].set(226, startInvY +   6);
	m_invSlotPositionTac[HEAD2POS            ].set(226, startInvY +  30);
	m_invSlotPositionTac[HANDPOS             ].set(226, startInvY +  84);
	m_invSlotPositionTac[SECONDHANDPOS       ].set(226, startInvY + 108);
	m_invSlotPositionTac[BIGPOCK1POS         ].set(468, startInvY +   5);
	m_invSlotPositionTac[BIGPOCK2POS         ].set(468, startInvY +  29);
	m_invSlotPositionTac[BIGPOCK3POS         ].set(468, startInvY +  53);
	m_invSlotPositionTac[BIGPOCK4POS         ].set(468, startInvY +  77);
	m_invSlotPositionTac[SMALLPOCK1POS       ].set(396, startInvY +   5);
	m_invSlotPositionTac[SMALLPOCK2POS       ].set(396, startInvY +  29);
	m_invSlotPositionTac[SMALLPOCK3POS       ].set(396, startInvY +  53);
	m_invSlotPositionTac[SMALLPOCK4POS       ].set(396, startInvY +  77);
	m_invSlotPositionTac[SMALLPOCK5POS       ].set(432, startInvY +   5);
	m_invSlotPositionTac[SMALLPOCK6POS       ].set(432, startInvY +  29);
	m_invSlotPositionTac[SMALLPOCK7POS       ].set(432, startInvY +  53);
	m_invSlotPositionTac[SMALLPOCK8POS       ].set(432, startInvY +  77);

	// map screen inventory position
	m_invSlotPositionMap[HELMETPOS           ].set(m_stdScreenOffsetX + 204, m_stdScreenOffsetY + 116);
	m_invSlotPositionMap[VESTPOS             ].set(m_stdScreenOffsetX + 204, m_stdScreenOffsetY + 145);
	m_invSlotPositionMap[LEGPOS              ].set(m_stdScreenOffsetX + 204, m_stdScreenOffsetY + 205);
	m_invSlotPositionMap[HEAD1POS            ].set(m_stdScreenOffsetX +  21, m_stdScreenOffsetY + 116);
	m_invSlotPositionMap[HEAD2POS            ].set(m_stdScreenOffsetX +  21, m_stdScreenOffsetY + 140);
	m_invSlotPositionMap[HANDPOS             ].set(m_stdScreenOffsetX +  21, m_stdScreenOffsetY + 194);
	m_invSlotPositionMap[SECONDHANDPOS       ].set(m_stdScreenOffsetX +  21, m_stdScreenOffsetY + 218);
	m_invSlotPositionMap[BIGPOCK1POS         ].set(m_stdScreenOffsetX +  98, m_stdScreenOffsetY + 251);
	m_invSlotPositionMap[BIGPOCK2POS         ].set(m_stdScreenOffsetX +  98, m_stdScreenOffsetY + 275);
	m_invSlotPositionMap[BIGPOCK3POS         ].set(m_stdScreenOffsetX +  98, m_stdScreenOffsetY + 299);
	m_invSlotPositionMap[BIGPOCK4POS         ].set(m_stdScreenOffsetX +  98, m_stdScreenOffsetY + 323);
	m_invSlotPositionMap[SMALLPOCK1POS       ].set(m_stdScreenOffsetX +  22, m_stdScreenOffsetY + 251);
	m_invSlotPositionMap[SMALLPOCK2POS       ].set(m_stdScreenOffsetX +  22, m_stdScreenOffsetY + 275);
	m_invSlotPositionMap[SMALLPOCK3POS       ].set(m_stdScreenOffsetX +  22, m_stdScreenOffsetY + 299);
	m_invSlotPositionMap[SMALLPOCK4POS       ].set(m_stdScreenOffsetX +  22, m_stdScreenOffsetY + 323);
	m_invSlotPositionMap[SMALLPOCK5POS       ].set(m_stdScreenOffsetX +  60, m_stdScreenOffsetY + 251);
	m_invSlotPositionMap[SMALLPOCK6POS       ].set(m_stdScreenOffsetX +  60, m_stdScreenOffsetY + 275);
	m_invSlotPositionMap[SMALLPOCK7POS       ].set(m_stdScreenOffsetX +  60, m_stdScreenOffsetY + 299);
	m_invSlotPositionMap[SMALLPOCK8POS       ].set(m_stdScreenOffsetX +  60, m_stdScreenOffsetY + 323);

	m_invCamoRegion.set(SM_BODYINV_X, SM_BODYINV_Y);

	m_progress_bar_box.set(STD_SCREEN_X + 5, 2, MIN_INTERFACE_WIDTH - 10, 12);
	m_moneyButtonLoc.set(343, startInvY + 11);
	m_MoneyButtonLocMap.set(m_stdScreenOffsetX + 174, m_stdScreenOffsetY + 115);

	m_VIEWPORT_START_X            = 0;
	m_VIEWPORT_START_Y            = 0;
	m_VIEWPORT_WINDOW_START_Y     = 0;
	m_VIEWPORT_END_X              = m_screenWidth;
	m_VIEWPORT_END_Y              = m_screenHeight - 120;
	m_VIEWPORT_WINDOW_END_Y       = m_screenHeight - 120;
	m_tacticalMapCenterX          = (m_VIEWPORT_END_X - m_VIEWPORT_START_X) / 2;
	m_tacticalMapCenterY          = (m_VIEWPORT_END_Y - m_VIEWPORT_START_Y) / 2;

	m_worldClippingRect.set(0, 0, m_screenWidth, m_screenHeight - 120);

	m_contractPosition.set(       m_stdScreenOffsetX + 120, m_stdScreenOffsetY +  50);
	m_attributePosition.set(      m_stdScreenOffsetX + 220, m_stdScreenOffsetY + 150);
	m_trainPosition.set(          m_stdScreenOffsetX + 160, m_stdScreenOffsetY + 150);
	m_vehiclePosition.set(        m_stdScreenOffsetX + 160, m_stdScreenOffsetY + 150);
	m_repairPosition.set(         m_stdScreenOffsetX + 160, m_stdScreenOffsetY + 150);
	m_assignmentPosition.set(     m_stdScreenOffsetX + 120, m_stdScreenOffsetY + 150);
	m_squadPosition.set(          m_stdScreenOffsetX + 160, m_stdScreenOffsetY + 150);
	m_versionPosition.set(        10, m_screenHeight - 15);
}

/** Get X position of tactical textbox. */
UINT16 UILayout::getTacticalTextBoxX()
{

	if ( guiCurrentScreen == MAP_SCREEN )
	{
		return STD_SCREEN_X + 110;
	}
	else
	{
		return 110;
	}
}

/** Get Y position of tactical textbox. */
UINT16 UILayout::getTacticalTextBoxY()
{
	if ( guiCurrentScreen == MAP_SCREEN )
	{
		return DEFAULT_EXTERN_PANEL_Y_POS;
	}
	else
	{
		return 20;
	}
}
