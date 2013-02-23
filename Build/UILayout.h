#ifndef _UI_LAYOUT_H_
#define _UI_LAYOUT_H_

#include "Types.h"

/////////////////////////////////////////////////////////////
// defines
/////////////////////////////////////////////////////////////

#define NUM_INVENTORY_SLOTS     (19)

/* Following defines allow us to not change the old code too much.
 * It will help to preserve original Stracciatella codebase. */

#define SCREEN_HEIGHT                   (g_ui.m_screenHeight)
#define SCREEN_WIDTH                    (g_ui.m_screenWidth)
#define INV_INTERFACE_START_Y           (g_ui.get_INV_INTERFACE_START_Y())
#define INTERFACE_START_Y               (g_ui.get_INTERFACE_START_Y())
#define gsVIEWPORT_END_X                (g_ui.m_VIEWPORT_END_X)
#define gsVIEWPORT_END_Y                (g_ui.m_VIEWPORT_END_Y)
#define gsVIEWPORT_WINDOW_END_Y         (g_ui.m_VIEWPORT_WINDOW_END_Y)

/////////////////////////////////////////////////////////////
// type definitions
/////////////////////////////////////////////////////////////

// USED TO SETUP REGION POSITIONS, ETC
struct INV_REGION_DESC
{
  INT16     sX;
  INT16     sY;

  void set(INT16 x, INT16 y)
  {
    sX = x;
    sY = y;
  }
};


struct MoneyLoc
{
  UINT16 x;
  UINT16 y;

  void set(UINT16 _x, UINT16 _y)
  {
    x = _x;
    y = _y;
  }
};


/** User Interface layout definition. */
struct UILayout
{
public:
  INT16                 m_mapScreenWidth;
  INT16                 m_mapScreenHeight;
  INT16                 m_screenWidth;
  INT16                 m_screenHeight;
  INV_REGION_DESC       m_invSlotPosition[NUM_INVENTORY_SLOTS];         /**< Inventory slots positions */
  SGPBoxEx              m_progress_bar_box;
  MoneyLoc              m_moneyButtonLoc;

  INT16                 m_VIEWPORT_END_X;
  INT16                 m_VIEWPORT_END_Y;
  INT16                 m_VIEWPORT_WINDOW_END_Y;

  SGPRect               m_wordlClippingRect;

  /** Constructor.
   * @param screenWidth Screen width
   * @param screenHeight Screen height */
  UILayout(INT16 screenWidth, INT16 screenHeight);

  /** Set screen width.
   * This method should be called before most of the application initialization is done. */
  void setScreenWidth(INT16 screenWidth);

  /** Set screen height.
   * This method should be called before most of the application initialization is done. */
  void setScreenHeight(INT16 screenHeight);

  INT16 currentHeight();
  INT16 get_CLOCK_Y();
  INT16 get_INTERFACE_START_Y();
  INT16 get_INV_INTERFACE_START_Y();
  INT16 get_RADAR_WINDOW_TM_Y();

protected:
  /** Recalculate UI elements' positions after changing screen size. */
  void recalculatePositions();
};

/////////////////////////////////////////////////////////////
// external declarations
/////////////////////////////////////////////////////////////

extern UILayout g_ui;

/////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////

#endif
