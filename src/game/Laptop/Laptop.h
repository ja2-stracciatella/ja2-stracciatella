#ifndef LAPTOP_H
#define LAPTOP_H

#include "MessageBoxScreen.h"
#include "MouseSystem.h"
#include "ScreenIDs.h"
#include "Types.h"
#include "UILayout.h"

#include <string_theory/string>


void ExitLaptop(void);
void HandleLapTopESCKey(void);
void SetBookMark(INT32 iBookId);
void SetLaptopExitScreen(ScreenID uiExitScreen);
void SetLaptopNewGameFlag(void);
extern MOUSE_CALLBACK LapTopScreenCallBack;
void HandleRightButtonUpEvent(void);
void DoLapTopMessageBox(MessageBoxStyleID ubStyle, const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags ubFlags, MSGBOX_CALLBACK ReturnCallback);
void GoToWebPage(INT32 iPageId);
void WebPageTileBackground(UINT8 ubNumX, UINT8 ubNumY, UINT16 usWidth, UINT16 usHeight, const SGPVObject* background);
void BlitTitleBarIcons(void);
void HandleKeyBoardShortCutsForLapTop(UINT16 usEvent, UINT32 usParam, UINT16 usKeyState);
void RenderWWWProgramTitleBar(void);
void DisplayProgramBoundingBox(BOOLEAN fMarkButtons);
void DoLapTopSystemMessageBox(const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback);
void CreateFileAndNewEmailIconFastHelpText(UINT32 uiHelpTextID, BOOLEAN fClearHelpText);
void InitLaptopAndLaptopScreens(void);

// clear out all temp files from laptop
void ClearOutTempLaptopFiles(void);

void HaventMadeImpMercEmailCallBack(void);


enum LaptopMode
{
	LAPTOP_MODE_NONE = 0,
	LAPTOP_MODE_FINANCES,
	LAPTOP_MODE_PERSONNEL,
	LAPTOP_MODE_HISTORY,
	LAPTOP_MODE_FILES,
	LAPTOP_MODE_FILES_ENRICO,
	LAPTOP_MODE_FILES_PLANS,
	LAPTOP_MODE_EMAIL,
	LAPTOP_MODE_EMAIL_NEW,
	LAPTOP_MODE_EMAIL_VIEW,
	LAPTOP_MODE_WWW,
	LAPTOP_MODE_AIM,
	LAPTOP_MODE_AIM_MEMBERS,
	LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX,
	LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES,
	LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES_VIDEO,
	LAPTOP_MODE_AIM_MEMBERS_ARCHIVES,
	LAPTOP_MODE_AIM_POLICIES,
	LAPTOP_MODE_AIM_HISTORY,
	LAPTOP_MODE_AIM_LINKS,
	LAPTOP_MODE_MERC,
	LAPTOP_MODE_MERC_ACCOUNT,
	LAPTOP_MODE_MERC_NO_ACCOUNT,
	LAPTOP_MODE_MERC_FILES,
	LAPTOP_MODE_BOBBY_R,
	LAPTOP_MODE_BOBBY_R_GUNS,
	LAPTOP_MODE_BOBBY_R_AMMO,
	LAPTOP_MODE_BOBBY_R_ARMOR,
	LAPTOP_MODE_BOBBY_R_MISC,
	LAPTOP_MODE_BOBBY_R_USED,
	LAPTOP_MODE_BOBBY_R_MAILORDER,
	LAPTOP_MODE_CHAR_PROFILE,
	LAPTOP_MODE_CHAR_PROFILE_QUESTIONAIRE,
	LAPTOP_MODE_FLORIST,
	LAPTOP_MODE_FLORIST_FLOWER_GALLERY,
	LAPTOP_MODE_FLORIST_ORDERFORM,
	LAPTOP_MODE_FLORIST_CARD_GALLERY,
	LAPTOP_MODE_INSURANCE,
	LAPTOP_MODE_INSURANCE_INFO,
	LAPTOP_MODE_INSURANCE_CONTRACT,
	LAPTOP_MODE_INSURANCE_COMMENTS,
	LAPTOP_MODE_FUNERAL,
	LAPTOP_MODE_BROKEN_LINK,
	LAPTOP_MODE_BOBBYR_SHIPMENTS,
	LAPTOP_MODE_END
};

extern LaptopMode  guiCurrentLaptopMode;
extern LaptopMode  guiPreviousLaptopMode;
extern INT32       giCurrentSubPage;
extern BOOLEAN     fReDrawScreenFlag;
extern BOOLEAN     fPausedReDrawScreenFlag;
extern BOOLEAN     fLoadPendingFlag;
extern BOOLEAN     fReDrawPostButtonRender;
extern BOOLEAN     fCurrentlyInLaptop;
extern SGPVObject* guiLaptopBACKGROUND;
extern SGPVObject* guiTITLEBARICONS;
extern BOOLEAN     fDoneLoadPending;
extern BOOLEAN     fConnectingToSubPage;
extern BOOLEAN     fFastLoadFlag;
extern BOOLEAN     gfShowBookmarks;
extern BOOLEAN     fShowBookmarkInfo;
extern BOOLEAN     fReDrawBookMarkInfo;

// bookamrks for WWW bookmark list


#define LAPTOP_X (0 + STD_SCREEN_X)
#define LAPTOP_Y (0 + STD_SCREEN_Y)


#define LAPTOP_SCREEN_UL_X		(UINT16)(111 + STD_SCREEN_X)
#define LAPTOP_SCREEN_UL_Y		(UINT16)( 27 + STD_SCREEN_Y)
#define LAPTOP_SCREEN_LR_X		(UINT16)(613 + STD_SCREEN_X)
#define LAPTOP_SCREEN_LR_Y		(UINT16)(427 + STD_SCREEN_Y)
#define LAPTOP_UL_X 			(UINT16)(24  + STD_SCREEN_X)
#define LAPTOP_UL_Y			(UINT16)(27  + STD_SCREEN_Y)
#define LAPTOP_SCREEN_WIDTH		(UINT16)(LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X)
#define LAPTOP_SCREEN_HEIGHT		(UINT16)(LAPTOP_SCREEN_LR_Y - LAPTOP_SCREEN_UL_Y)

// new positions for web browser
#define LAPTOP_SCREEN_WEB_UL_Y		LAPTOP_SCREEN_UL_Y + 19
#define LAPTOP_SCREEN_WEB_LR_Y		LAPTOP_SCREEN_WEB_UL_Y + LAPTOP_SCREEN_HEIGHT
#define LAPTOP_SCREEN_WEB_DELTA_Y	LAPTOP_SCREEN_WEB_UL_Y - LAPTOP_SCREEN_UL_Y


// the bookmark values, move cancel down as bookmarks added

enum{
	AIM_BOOKMARK = 0,
	BOBBYR_BOOKMARK,
	IMP_BOOKMARK,
	MERC_BOOKMARK,
	FUNERAL_BOOKMARK,
	FLORIST_BOOKMARK,
	INSURANCE_BOOKMARK,
	CANCEL_STRING,
};


#define DEAD_MERC_COLOR_RED   255
#define DEAD_MERC_COLOR_GREEN  55
#define DEAD_MERC_COLOR_BLUE   55


void DoLapTopSystemMessageBoxWithRect(MessageBoxStyleID ubStyle, const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback, SGPBox const* centering_rect);

void     LaptopScreenInit(void);
ScreenID LaptopScreenHandle(void);
void     LaptopScreenShutdown(void);

#endif
