#ifndef MESSAGEBOXSCREEN_H
#define MESSAGEBOXSCREEN_H

#include "Button_System.h"
#include "JA2Types.h"
#include "MouseSystem.h"
#include "ScreenIDs.h"


// Message box flags
enum MessageBoxFlags
{
	MSG_BOX_FLAG_OK,                    // Displays OK button
	MSG_BOX_FLAG_YESNO,                 // Displays YES NO buttons
	MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS, // Displays four numbered buttons, 1-4
	MSG_BOX_FLAG_OKCONTRACT,            // ok and contract buttons
	MSG_BOX_FLAG_YESNOLIE,              // ok and contract buttons
	MSG_BOX_FLAG_CONTINUESTOP,          // continue stop box
	MSG_BOX_FLAG_OKSKIP,                // Displays ok or skip (meanwhile) buttons
	MSG_BOX_FLAG_GENERICCONTRACT,       // displays contract buttoin + 2 user-defined text buttons
	MSG_BOX_FLAG_GENERIC                // 2 user-defined text buttons
};

// message box return codes
enum MessageBoxReturnValue
{
	MSG_BOX_RETURN_NONE     = 0,
	MSG_BOX_RETURN_OK       = 1, // ENTER or on OK button
	MSG_BOX_RETURN_YES      = 2, // ENTER or YES button
	MSG_BOX_RETURN_NO       = 3, // ESC, Right Click or NO button
	MSG_BOX_RETURN_CONTRACT = 4, // contract button
	MSG_BOX_RETURN_LIE      = 5, // LIE BUTTON

	MSG_BOX_RETURN_1        = 1,
	MSG_BOX_RETURN_2        = 2,
	MSG_BOX_RETURN_3        = 3,
	MSG_BOX_RETURN_4        = 4
};

typedef void (*MSGBOX_CALLBACK)(MessageBoxReturnValue);

// message box style flags
enum MessageBoxStyleID
{
	MSG_BOX_BASIC_STYLE,	// We'll have other styles, like in laptop, etc
				// Graphics are all that are required here...
	MSG_BOX_RED_ON_WHITE,
	MSG_BOX_BLUE_ON_GREY,
	MSG_BOX_BASIC_SMALL_BUTTONS,
	MSG_BOX_IMP_STYLE,
	MSG_BOX_LAPTOP_DEFAULT
};


struct MESSAGE_BOX_STRUCT
{
	MessageBoxFlags       usFlags;
	ScreenID              uiExitScreen;
	MSGBOX_CALLBACK       ExitCallback;
	UINT16                uX;
	UINT16                uY;
	SGPVSurface*          uiSaveBuffer;
	MOUSE_REGION          BackRegion;
	UINT16                usWidth;
	UINT16                usHeight;
	BUTTON_PICS*          iButtonImages;
	GUIButtonRef          uiOKButton;
	GUIButtonRef          uiYESButton;
	GUIButtonRef          uiNOButton;
	GUIButtonRef          uiButton[4];
	BOOLEAN               fRenderBox;
	MessageBoxReturnValue bHandled;
	MercPopUpBox*         box;
};


extern MESSAGE_BOX_STRUCT gMsgBox;
extern BOOLEAN            fRestoreBackgroundForMessageBox;

//this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save buffer
extern BOOLEAN gfDontOverRideSaveBuffer;

extern wchar_t gzUserDefinedButton1[128];
extern wchar_t gzUserDefinedButton2[128];

/* ubStyle:       Determines the look of graphics including buttons
 * zString:       16-bit string
 * uiExitScreen   The screen to exit to
 * ubFlags        Some flags for button style
 * ReturnCallback Callback for return. Can be NULL. Returns any above return value
 * pCenteringRect Rect to center in. Can be NULL */
void DoMessageBox(MessageBoxStyleID, wchar_t const* zString, ScreenID uiExitScreen, MessageBoxFlags, MSGBOX_CALLBACK ReturnCallback, SGPBox const* centering_rect);
void DoScreenIndependantMessageBox(const wchar_t* zString, MessageBoxFlags, MSGBOX_CALLBACK ReturnCallback);

//wrappers for other screens
void DoMapMessageBoxWithRect(MessageBoxStyleID, wchar_t const* zString, ScreenID uiExitScreen, MessageBoxFlags, MSGBOX_CALLBACK ReturnCallback, SGPBox const* centering_rect);
void DoOptionsMessageBoxWithRect(               wchar_t const* zString, ScreenID uiExitScreen, MessageBoxFlags, MSGBOX_CALLBACK ReturnCallback, SGPBox const* centering_rect);
void DoSaveLoadMessageBoxWithRect(              wchar_t const* zString, ScreenID uiExitScreen, MessageBoxFlags, MSGBOX_CALLBACK ReturnCallback, SGPBox const* centering_rect);

extern BOOLEAN gfInMsgBox;

ScreenID MessageBoxScreenHandle(void);
void     MessageBoxScreenShutdown();

#endif
