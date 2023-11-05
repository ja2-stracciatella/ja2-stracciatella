#ifndef __AIM_H
#define __AIM_H

#include "Types.h"
#include "FontCompat.h"
#include "Laptop.h"

#define AIMHISTORYFILE			BINARYDATADIR "/aimhist.edt"
#define AIM_HISTORY_LINE_SIZE		400


#define MAX_NUMBER_MERCS		40
extern UINT8 AimMercArray[MAX_NUMBER_MERCS];

#define NUM_AIM_SCREENS			6

#define IMAGE_OFFSET_X			LAPTOP_SCREEN_UL_X//111
#define IMAGE_OFFSET_Y			LAPTOP_SCREEN_WEB_UL_Y//24


#define AIM_LOGO_TEXT_X			(175 * g_ui.m_stdScreenScale + STD_SCREEN_X)
#define AIM_LOGO_TEXT_Y			(77 * g_ui.m_stdScreenScale + LAPTOP_SCREEN_WEB_DELTA_Y + 4 * g_ui.m_stdScreenScale + STD_SCREEN_Y)
#define AIM_LOGO_TEXT_WIDTH		(360 * g_ui.m_stdScreenScale)

// Aim Symbol 203, 51
#define AIM_SYMBOL_SIZE_Y		(51 * g_ui.m_stdScreenScale)

//262, 28
#define AIM_SYMBOL_X			(IMAGE_OFFSET_X + 149 * g_ui.m_stdScreenScale)
#define AIM_SYMBOL_Y			(IMAGE_OFFSET_Y + 3 * g_ui.m_stdScreenScale)
#define AIM_SYMBOL_WIDTH		(203 * g_ui.m_stdScreenScale)
#define AIM_SYMBOL_HEIGHT		(51 * g_ui.m_stdScreenScale)

// RustBackGround
#define RUSTBACKGROUND_SIZE_X		(125 * g_ui.m_stdScreenScale)
#define RUSTBACKGROUND_SIZE_Y		(100 * g_ui.m_stdScreenScale)

#define RUSTBACKGROUND_1_X		IMAGE_OFFSET_X
#define RUSTBACKGROUND_1_Y		IMAGE_OFFSET_Y

//Bottom Buttons
#define NUM_AIM_BOTTOMBUTTONS		6
#define BOTTOM_BUTTON_START_WIDTH	(75 * g_ui.m_stdScreenScale)
#define BOTTOM_BUTTON_START_HEIGHT	(18 * g_ui.m_stdScreenScale)
#define BOTTOM_BUTTON_START_X		(LAPTOP_SCREEN_UL_X + 25 * g_ui.m_stdScreenScale)
#define BOTTOM_BUTTON_START_Y		(LAPTOP_SCREEN_WEB_LR_Y - BOTTOM_BUTTON_START_HEIGHT - 3 * g_ui.m_stdScreenScale)
#define BOTTOM_BUTTON_AMOUNT		NUM_AIM_SCREENS

#define AIM_LOGO_FONT			FONT10ARIAL
#define AIM_COPYRIGHT_FONT		FONT10ARIAL
#define AIM_WARNING_FONT		FONT12ARIAL
#define AIM_FONT12ARIAL			FONT12ARIAL
#define AIM_FONT_MCOLOR_WHITE		FONT_MCOLOR_WHITE
#define AIM_GREEN			FONT_COLOR_P157
#define AIM_MAINTITLE_COLOR		AIM_GREEN
#define AIM_MAINTITLE_FONT		FONT14ARIAL
#define AIM_BUTTON_ON_COLOR		FONT_MCOLOR_DKWHITE
#define AIM_BUTTON_OFF_COLOR		FONT_COLOR_P138
#define AIM_CONTENTBUTTON_WIDTH		(205 * g_ui.m_stdScreenScale)
#define AIM_CONTENTBUTTON_HEIGHT	(19 * g_ui.m_stdScreenScale)
#define AIM_FONT_GOLD			FONT_COLOR_P170


enum
{
	AIM_SLOGAN,
	AIM_WARNING_1,
	AIM_WARNING_2,
	AIM_COPYRIGHT_1,
	AIM_COPYRIGHT_2,
	AIM_COPYRIGHT_3
};

void GameInitAIM(void);
void EnterAIM(void);
void ExitAIM(void);
void HandleAIM(void);
void RenderAIM(void);

void ExitAimMenuBar();
void InitAimMenuBar();

void RemoveAimDefaults(void);
void InitAimDefaults(void);
void DrawAimDefaults(void);

void DisplayAimSlogan(void);
void DisplayAimCopyright(void);

void DisableAimButton(void);

#endif
