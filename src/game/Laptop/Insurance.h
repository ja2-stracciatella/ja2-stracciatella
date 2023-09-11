#ifndef __INSURANCE_H
#define __INSURANCE_H

#include "Types.h"
#include <string_theory/string>


#define INS_FONT_COLOR			2
#define INS_FONT_COLOR_RED		FONT_MCOLOR_RED
#define INS_FONT_BIG			FONT14ARIAL
#define INS_FONT_MED			FONT12ARIAL
#define INS_FONT_SMALL			FONT10ARIAL

#define INS_FONT_BTN_COLOR		FONT_MCOLOR_WHITE
#define INS_FONT_BTN_SHADOW_COLOR	2

#define INS_FONT_SHADOW			FONT_MCOLOR_WHITE

#define INSURANCE_BULLET_TEXT_OFFSET_X	21

#define INS_INFO_LEFT_ARROW_BUTTON_X	(LAPTOP_SCREEN_UL_X     +  71)
#define INS_INFO_RIGHT_ARROW_BUTTON_X	(LAPTOP_SCREEN_UL_X     + 409)
#define INS_INFO_ARROW_BUTTON_Y		(LAPTOP_SCREEN_WEB_UL_Y + 354)


void EnterInsurance(void);
void ExitInsurance();
void RenderInsurance(void);

void InitInsuranceDefaults(void);
void DisplayInsuranceDefaults(void);
void RemoveInsuranceDefaults(void);
void DisplaySmallRedLineWithShadow( UINT16 usStartX, UINT16 usStartY, UINT16 EndX, UINT16 EndY);
ST::string GetInsuranceText(UINT8 ubNumber);

#endif
