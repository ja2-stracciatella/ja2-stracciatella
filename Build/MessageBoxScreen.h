#ifndef MESSAGEBOXSCREEN_H
#define MESSAGEBOXSCREEN_H

#include "MouseSystem.h"

// Message box flags
#define MSG_BOX_FLAG_OK                    0x0002 // Displays OK button
#define MSG_BOX_FLAG_YESNO                 0x0004 // Displays YES NO buttons
#define MSG_BOX_FLAG_CANCEL                0x0008 // Displays YES NO buttons
#define MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS 0x0010 // Displays four numbered buttons, 1-4
#define MSG_BOX_FLAG_YESNOCONTRACT         0x0020 // yes no and contract buttons
#define MSG_BOX_FLAG_OKCONTRACT            0x0040 // ok and contract buttons
#define MSG_BOX_FLAG_YESNOLIE              0x0080 // ok and contract buttons
#define MSG_BOX_FLAG_CONTINUESTOP          0x0100 // continue stop box
#define MSG_BOX_FLAG_OKSKIP                0x0200 // Displays ok or skip (meanwhile) buttons
#define MSG_BOX_FLAG_GENERICCONTRACT       0x0400 // displays contract buttoin + 2 user-defined text buttons
#define MSG_BOX_FLAG_GENERIC               0x0800 // 2 user-defined text buttons

// message box return codes
#define MSG_BOX_RETURN_OK       1 // ENTER or on OK button
#define MSG_BOX_RETURN_YES      2 // ENTER or YES button
#define MSG_BOX_RETURN_NO       3 // ESC, Right Click or NO button
#define MSG_BOX_RETURN_CONTRACT 4 // contract button
#define MSG_BOX_RETURN_LIE      5 // LIE BUTTON

// message box style flags
enum
{
	MSG_BOX_BASIC_STYLE,         // We'll have other styles, like in laptop, etc
	                             // Graphics are all that are required here...
	MSG_BOX_RED_ON_WHITE,
	MSG_BOX_BLUE_ON_GREY,
	MSG_BOX_BASIC_SMALL_BUTTONS,
	MSG_BOX_IMP_STYLE,
	MSG_BOX_LAPTOP_DEFAULT
};


typedef void (*MSGBOX_CALLBACK)(UINT8 bExitValue);


typedef struct
{
	UINT16          usFlags;
	UINT32          uiExitScreen;
	MSGBOX_CALLBACK ExitCallback;
	INT16           sX;
	INT16           sY;
	UINT32          uiSaveBuffer;
	MOUSE_REGION    BackRegion;
	UINT16          usWidth;
	UINT16          usHeight;
	INT32           iButtonImages;
	union
	{
		struct
		{
			UINT32 uiOKButton;
			UINT32 uiYESButton;
			UINT32 uiNOButton;
		};
		struct
		{
			UINT32 uiButton[4];
		};
	};
	BOOLEAN fRenderBox;
	INT8    bHandled;
	INT32   iBoxId;
} MESSAGE_BOX_STRUCT;


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
INT32 DoMessageBox(UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect);
void DoScreenIndependantMessageBox(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback);
void DoLowerScreenIndependantMessageBox(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback);

//wrappers for other screens
INT32   DoMapMessageBoxWithRect(     UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect);
BOOLEAN DoOptionsMessageBoxWithRect( UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect);
BOOLEAN DoSaveLoadMessageBoxWithRect(UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect);

#endif
