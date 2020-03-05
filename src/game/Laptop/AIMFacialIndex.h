#ifndef __AIMFACIALINDEX_H_
#define __AIMFACIALINDEX_H_


#define AIM_FI_LEFT_CLICK_TEXT_X	(STD_SCREEN_X + 116 * g_ui.m_stdScreenScale)
#define AIM_FI_LEFT_CLICK_TEXT_Y	(STD_SCREEN_Y + 35 * g_ui.m_stdScreenScale + LAPTOP_SCREEN_WEB_DELTA_Y)

#define AIM_FI_CLICK_DESC_TEXT_Y_OFFSET	(16 * g_ui.m_stdScreenScale)

#define AIM_FI_RIGHT_CLICK_TEXT_X	(STD_SCREEN_X + 500 * g_ui.m_stdScreenScale)

#define AIM_FI_CLICK_TEXT_WIDTH		(110 * g_ui.m_stdScreenScale)


#define AIM_FI_HELP_FONT		FONT10ARIAL
#define AIM_FI_HELP_TITLE_FONT		FONT12ARIAL
#define AIM_FI_HELP_COLOR		FONT_MCOLOR_WHITE


void EnterAimFacialIndex(void);
void ExitAimFacialIndex();
void RenderAimFacialIndex(void);

#endif
