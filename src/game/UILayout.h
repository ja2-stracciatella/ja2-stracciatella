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
#define gsVIEWPORT_START_X              (g_ui.m_VIEWPORT_START_X)
#define gsVIEWPORT_START_Y              (g_ui.m_VIEWPORT_START_Y)
#define gsVIEWPORT_WINDOW_START_Y       (g_ui.m_VIEWPORT_WINDOW_START_Y)
#define gsVIEWPORT_END_X                (g_ui.m_VIEWPORT_END_X)
#define gsVIEWPORT_END_Y                (g_ui.m_VIEWPORT_END_Y)
#define gsVIEWPORT_WINDOW_END_Y         (g_ui.m_VIEWPORT_WINDOW_END_Y)
#define STD_SCREEN_X                    (g_ui.m_stdScreenOffsetX)
#define STD_SCREEN_Y                    (g_ui.m_stdScreenOffsetY)
#define MAP_SCREEN_WIDTH                (g_ui.m_mapScreenWidth)
#define MAP_SCREEN_HEIGHT               (g_ui.m_mapScreenHeight)

#define SM_BODYINV_X                    (244)
#define SM_BODYINV_Y                    (INV_INTERFACE_START_Y + 6)

#define EDITOR_TASKBAR_HEIGHT           (120)
#define EDITOR_TASKBAR_POS_Y            (UINT16)(SCREEN_HEIGHT - EDITOR_TASKBAR_HEIGHT)

#define DEFAULT_EXTERN_PANEL_X_POS      (STD_SCREEN_X + 320)
#define DEFAULT_EXTERN_PANEL_Y_POS      (STD_SCREEN_Y + 40)

/////////////////////////////////////////////////////////////
// type definitions
/////////////////////////////////////////////////////////////

// USED TO SETUP REGION POSITIONS, ETC
struct INV_REGION_DESC
{
	UINT16     uX;
	UINT16     uY;

	void set(UINT16 x, UINT16 y)
	{
		uX = x;
		uY = y;
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
	UINT16                m_mapScreenWidth;
	UINT16                m_mapScreenHeight;
	UINT16                m_screenWidth;
	UINT16                m_screenHeight;
	INV_REGION_DESC       m_invSlotPositionMap[NUM_INVENTORY_SLOTS];      /**< Map screen inventory slots positions  */
	INV_REGION_DESC       m_invSlotPositionTac[NUM_INVENTORY_SLOTS];      /**< Tactical screen Inventory slots positions */
	INV_REGION_DESC       m_invCamoRegion;                                /**< Camo (body) region in the inventory. */

	SGPBox                m_progress_bar_box;
	MoneyLoc              m_moneyButtonLoc;
	MoneyLoc              m_MoneyButtonLocMap;

	/** Viewport coordiantes.
	 * Viewport is the area of the screen where tactical map is displayed.
	 * For 640x480 it is (320, 180) */
	UINT16                m_VIEWPORT_START_X;
	UINT16                m_VIEWPORT_START_Y;
	UINT16                m_VIEWPORT_WINDOW_START_Y;

	UINT16                m_VIEWPORT_END_X;
	UINT16                m_VIEWPORT_END_Y;
	UINT16                m_tacticalMapCenterX;                           /**< Center of the tactical map (for 640x480 it is (320, 180)). */
	UINT16                m_tacticalMapCenterY;                           /**< Center of the tactical map (for 640x480 it is (320, 180)). */

	UINT16                m_VIEWPORT_WINDOW_END_Y;

	SGPRect               m_worldClippingRect;

	SGPPoint              m_versionPosition;
	SGPPoint              m_contractPosition;
	SGPPoint              m_attributePosition;
	SGPPoint              m_trainPosition;
	SGPPoint              m_vehiclePosition;
	SGPPoint              m_repairPosition;
	SGPPoint              m_assignmentPosition ;
	SGPPoint              m_squadPosition ;

	UINT16                m_stdScreenOffsetX;             /** Offset of the standard (640x480) window */
	UINT16                m_stdScreenOffsetY;             /** Offset of the standard (640x480) window */

	/** Constructor.
	 * @param screenWidth Screen width
	 * @param screenHeight Screen height */
	UILayout(UINT16 screenWidth, UINT16 screenHeight);

	/** Set new screen size.
	 * This method should be called before most of the application initialization is done. */
	bool setScreenSize(UINT16 width, UINT16 height);

	/** Check if the screen is bigger than original 640x480. */
	bool isBigScreen();

	UINT16 currentHeight();
	UINT16 get_CLOCK_X();
	UINT16 get_CLOCK_Y();
	UINT16 get_INTERFACE_START_Y();
	UINT16 get_INV_INTERFACE_START_Y();
	UINT16 get_RADAR_WINDOW_X();
	UINT16 get_RADAR_WINDOW_TM_Y();

	/** Get X position of tactical textbox. */
	UINT16 getTacticalTextBoxX();

	/** Get Y position of tactical textbox. */
	UINT16 getTacticalTextBoxY();

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
