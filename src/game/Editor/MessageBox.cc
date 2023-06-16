#include "Directories.h"
#include "Local.h"
#include "Video.h"
#include "Font.h"
#include "Font_Control.h"
#include "MessageBox.h"
#include "Input.h"
#include "English.h"
#include "Button_System.h"
#include "UILayout.h"

#include <string_theory/string>


GUIButtonRef iMsgBoxBgrnd;
GUIButtonRef iMsgBoxOk;
GUIButtonRef iMsgBoxCancel;

BOOLEAN gfMessageBoxResult = FALSE;
UINT8 gubMessageBoxStatus = MESSAGEBOX_NONE;


static void MsgBoxCnclClkCallback(GUI_BUTTON* butn, UINT32 reason);
static void MsgBoxOkClkCallback(GUI_BUTTON* butn, UINT32 reason);


void CreateMessageBox(const ST::string& msg)
{
	SGPFont  const font   = gpLargeFontType1;
	INT16       w      = StringPixLength(msg, font) + 10;
	INT16 const h      = 96;
	if (w > 600) w = 600;

	INT16 const x = (SCREEN_WIDTH  - w) / 2;
	INT16 const y = (SCREEN_HEIGHT - h) / 2;

	// Fake button for background with text
	iMsgBoxBgrnd = CreateLabel(msg, font, FONT_LTKHAKI, FONT_DKKHAKI, x, y, w, h, MSYS_PRIORITY_HIGHEST - 2);

	INT16 const bx = x + w / 2;
	INT16 const by = y + 58;
	iMsgBoxOk     = QuickCreateButtonImg(EDITORDIR "/ok.sti",     0, 1, 2, 3, 4, bx - 35, by, MSYS_PRIORITY_HIGHEST - 1, MsgBoxOkClkCallback);
	iMsgBoxCancel = QuickCreateButtonImg(EDITORDIR "/cancel.sti", 0, 1, 2, 3, 4, bx +  5, by, MSYS_PRIORITY_HIGHEST - 1, MsgBoxCnclClkCallback);

	SGPRect msg_box_rect;
	msg_box_rect.iLeft   = x;
	msg_box_rect.iTop    = y;
	msg_box_rect.iRight  = x + w;
	msg_box_rect.iBottom = y + h;
	RestrictMouseCursor(&msg_box_rect);

	gfMessageBoxResult  = FALSE;
	gubMessageBoxStatus = MESSAGEBOX_WAIT;
}


BOOLEAN MessageBoxHandled()
{
	InputAtom DummyEvent;

	while( DequeueSpecificEvent(&DummyEvent, KEYBOARD_EVENTS) )
	{
		if ( DummyEvent.usEvent == KEY_DOWN )
		{
			switch( DummyEvent.usParam )
			{
				case SDLK_RETURN:
				case SDLK_y:
					gubMessageBoxStatus = MESSAGEBOX_DONE;
					gfMessageBoxResult = TRUE;
					break;

				case SDLK_ESCAPE:
				case SDLK_n:
					gubMessageBoxStatus = MESSAGEBOX_DONE;
					gfMessageBoxResult = FALSE;
					break;
			}
		}
	}

	if( gubMessageBoxStatus == MESSAGEBOX_DONE )
	{
		while( DequeueEvent( &DummyEvent ) )
			continue;
	}
	MarkButtonsDirty();
	RenderButtons( );
	return gubMessageBoxStatus == MESSAGEBOX_DONE;
}

void RemoveMessageBox( )
{
	FreeMouseCursor( );
	RemoveButton( iMsgBoxCancel );
	RemoveButton( iMsgBoxOk );
	RemoveButton( iMsgBoxBgrnd );
	gubMessageBoxStatus = MESSAGEBOX_NONE;
}


static void MsgBoxOkClkCallback(GUI_BUTTON* butn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubMessageBoxStatus = MESSAGEBOX_DONE;
		gfMessageBoxResult = TRUE;
	}
}


static void MsgBoxCnclClkCallback(GUI_BUTTON* butn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubMessageBoxStatus = MESSAGEBOX_DONE;
		gfMessageBoxResult = FALSE;
	}
}
