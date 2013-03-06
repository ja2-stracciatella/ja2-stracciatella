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
#define STD_SCREEN_X                    (g_ui.m_stdScreenOffsetX)
#define STD_SCREEN_Y                    (g_ui.m_stdScreenOffsetY)
#define MAP_SCREEN_WIDTH                (g_ui.m_mapScreenWidth)
#define MAP_SCREEN_HEIGHT               (g_ui.m_mapScreenHeight)

#define SM_BODYINV_X                    (244)
#define SM_BODYINV_Y                    (INV_INTERFACE_START_Y + 6)

#define			DEFAULT_EXTERN_PANEL_X_POS      (STD_SCREEN_X + 320)
#define			DEFAULT_EXTERN_PANEL_Y_POS      (STD_SCREEN_Y + 40)

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
  INV_REGION_DESC       m_invSlotPositionMap[NUM_INVENTORY_SLOTS];      /**< Map screen inventory slots positions  */
  INV_REGION_DESC       m_invSlotPositionTac[NUM_INVENTORY_SLOTS];      /**< Tactical screen Inventory slots positions */
  INV_REGION_DESC       m_invCamoRegion;                                /**< Camo (body) region in the inventory. */

  SGPBox                m_progress_bar_box;
  MoneyLoc              m_moneyButtonLoc;

  INT16                 m_VIEWPORT_END_X;
  INT16                 m_VIEWPORT_END_Y;
  INT16                 m_VIEWPORT_WINDOW_END_Y;

  SGPRect               m_wordlClippingRect;

  SGPPoint              m_contractPosition;
  SGPPoint              m_attributePosition;
  SGPPoint              m_trainPosition;
  SGPPoint              m_vehiclePosition;
  SGPPoint              m_repairPosition;
  SGPPoint              m_assignmentPosition ;
  SGPPoint              m_squadPosition ;

  INT16                 m_stdScreenOffsetX;             /** Offset of the standard (640x480) window */
  INT16                 m_stdScreenOffsetY;             /** Offset of the standard (640x480) window */

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

  /** Check if the screen is bigger than original 640x480. */
  bool isBigScreen();

  INT16 currentHeight();
  INT16 get_CLOCK_X();
  INT16 get_CLOCK_Y();
  INT16 get_INTERFACE_START_Y();
  INT16 get_INV_INTERFACE_START_Y();
  INT16 get_RADAR_WINDOW_X();
  INT16 get_RADAR_WINDOW_TM_Y();

  /** Get X position of tactical textbox. */
  INT16 getTacticalTextBoxX();

  /** Get Y position of tactical textbox. */
  INT16 getTacticalTextBoxY();

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
