#ifndef __BOBBYRGUNS_H
#define __BOBBYRGUNS_H

#include "Types.h"

#define BOBBYRDESCFILE BINARYDATADIR "/braydesc.edt"

#define BOBBYR_ITEM_DESC_NAME_SIZE  80
#define BOBBYR_ITEM_DESC_INFO_SIZE 320
#define BOBBYR_ITEM_DESC_FILE_SIZE 400

#define BOBBYR_USED_ITEMS		0xFFFFFFFF


#define BOBBYR_GUNS_BUTTON_FONT		FONT10ARIAL
#define BOBBYR_GUNS_TEXT_COLOR_ON	FONT_NEARBLACK
#define BOBBYR_GUNS_TEXT_COLOR_OFF	FONT_NEARBLACK
//#define BOBBYR_GUNS_TEXT_COLOR_ON	FONT_MCOLOR_DKWHITE
//#define BOBBYR_GUNS_TEXT_COLOR_OFF	FONT_MCOLOR_WHITE

#define BOBBYR_GUNS_SHADOW_COLOR	169

#define BOBBYR_NO_ITEMS			65535





extern UINT16 gusCurWeaponIndex;
extern UINT8  gubLastGunIndex;



void GameInitBobbyRGuns(void);
void EnterBobbyRGuns(void);
void ExitBobbyRGuns(void);
void RenderBobbyRGuns(void);


void DisplayBobbyRBrTitle(void);
void DeleteBobbyBrTitle(void);
void InitBobbyBrTitle(void);
void InitBobbyMenuBar();
void DeleteBobbyMenuBar();

//BOOLEAN DisplayWeaponInfo();
void DisplayItemInfo(UINT32 uiItemClass);
void DeleteMouseRegionForBigImage(void);
void UpdateButtonText(UINT32	uiCurPage);
UINT16 CalcBobbyRayCost( UINT16 usIndex, UINT16 usBobbyIndex, BOOLEAN fUsed);
void SetFirstLastPagesForUsed(void);
void SetFirstLastPagesForNew( UINT32 uiClass );

#endif
