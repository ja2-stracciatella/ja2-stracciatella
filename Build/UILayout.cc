#include "UILayout.h"

#include "MapScreen.h"
#include "Soldier_Control.h"

#define MIN_INTERFACE_WIDTH       640
#define MIN_INTERFACE_HEIGHT      480

/**
 * Default screen layout.
 * It might be changed later when the window size is known for sure. */
UILayout g_ui(MIN_INTERFACE_WIDTH, MIN_INTERFACE_HEIGHT);


/** Constructor. */
UILayout::UILayout(INT16 screenWidth, INT16 screenHeight)
  :m_mapScreenWidth(MIN_INTERFACE_WIDTH), m_mapScreenHeight(MIN_INTERFACE_HEIGHT),
   m_screenWidth(screenWidth), m_screenHeight(screenHeight),
   m_progress_bar_box(5, 2, m_mapScreenWidth-10, 12)
{
  recalculatePositions();
}


/** Set screen width.
 * This method should be called before most of the application initialization is done. */
void UILayout::setScreenWidth(INT16 screenWidth)
{
  if(screenWidth > MIN_INTERFACE_WIDTH)
  {
    m_screenWidth = screenWidth;
    recalculatePositions();
  }
}

/** Set screen height.
 * This method should be called before most of the application initialization is done. */
void UILayout::setScreenHeight(INT16 screenHeight)
{
  if(screenHeight > MIN_INTERFACE_HEIGHT)
  {
    m_screenHeight = screenHeight;
    recalculatePositions();
  }
}

INT16 UILayout::currentHeight()             { return fInMapMode ? m_mapScreenHeight : m_screenHeight;       }
INT16 UILayout::get_CLOCK_Y()               { return currentHeight() - 23;                                  }
INT16 UILayout::get_RADAR_WINDOW_TM_Y()     { return currentHeight() - 107;                                 }
INT16 UILayout::get_INTERFACE_START_Y()     { return m_screenHeight - 120;                                  }
INT16 UILayout::get_INV_INTERFACE_START_Y() { return m_screenHeight - 140;                                  }


/** Recalculate UI elements' positions after changing screen size. */
void UILayout::recalculatePositions()
{
  INT16 startInvY = get_INV_INTERFACE_START_Y();

  m_invSlotPosition[HELMETPOS           ].set(344, startInvY +   6);
  m_invSlotPosition[VESTPOS             ].set(344, startInvY +  35);
  m_invSlotPosition[LEGPOS              ].set(344, startInvY +  95);
  m_invSlotPosition[HEAD1POS            ].set(226, startInvY +   6);
  m_invSlotPosition[HEAD2POS            ].set(226, startInvY +  30);
  m_invSlotPosition[HANDPOS             ].set(226, startInvY +  84);
  m_invSlotPosition[SECONDHANDPOS       ].set(226, startInvY + 108);
  m_invSlotPosition[BIGPOCK1POS         ].set(468, startInvY +   5);
  m_invSlotPosition[BIGPOCK2POS         ].set(468, startInvY +  29);
  m_invSlotPosition[BIGPOCK3POS         ].set(468, startInvY +  53);
  m_invSlotPosition[BIGPOCK4POS         ].set(468, startInvY +  77);
  m_invSlotPosition[SMALLPOCK1POS       ].set(396, startInvY +   5);
  m_invSlotPosition[SMALLPOCK2POS       ].set(396, startInvY +  29);
  m_invSlotPosition[SMALLPOCK3POS       ].set(396, startInvY +  53);
  m_invSlotPosition[SMALLPOCK4POS       ].set(396, startInvY +  77);
  m_invSlotPosition[SMALLPOCK5POS       ].set(432, startInvY +   5);
  m_invSlotPosition[SMALLPOCK6POS       ].set(432, startInvY +  29);
  m_invSlotPosition[SMALLPOCK7POS       ].set(432, startInvY +  53);
  m_invSlotPosition[SMALLPOCK8POS       ].set(432, startInvY +  7);

  m_moneyButtonLoc.set(343, startInvY + 11);

  m_VIEWPORT_END_X              = m_screenWidth;
  m_VIEWPORT_END_Y              = m_screenHeight - 120;
  m_VIEWPORT_WINDOW_END_Y       = m_screenHeight - 120;

  m_wordlClippingRect.set(0, 0, m_screenWidth, m_screenHeight - 120);
}
