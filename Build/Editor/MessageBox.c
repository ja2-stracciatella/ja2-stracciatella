#include "BuildDefines.h"

#ifdef JA2EDITOR

#include "Local.h"
#include "VObject.h"
#include "Video.h"
#include "Font.h"
#include "Font_Control.h"
#include "MessageBox.h"
#include "Input.h"
#include "English.h"
#include "Button_System.h"


//internal variables.
INT32 iMsgBoxOkImg, iMsgBoxCancelImg;
INT32 iMsgBoxBgrnd,iMsgBoxOk,iMsgBoxCancel;

BOOLEAN gfMessageBoxResult = FALSE;
UINT8 gubMessageBoxStatus = MESSAGEBOX_NONE;


static void MsgBoxCnclClkCallback(GUI_BUTTON* butn, INT32 reason);
static void MsgBoxOkClkCallback(GUI_BUTTON* butn, INT32 reason);


void CreateMessageBox(const wchar_t* wzString)
{
	INT16 sStartX, sStartY;

	INT16 sPixLen = StringPixLength(wzString, gpLargeFontType1) + 10;
	if ( sPixLen > 600 )
		sPixLen = 600;

	sStartX = (SCREEN_WIDTH  - sPixLen) / 2;
	sStartY = (SCREEN_HEIGHT - 96)      / 2;

	gfMessageBoxResult = FALSE;

	// Fake button for background w/ text
	iMsgBoxBgrnd = CreateLabel(wzString, gpLargeFontType1, FONT_LTKHAKI, FONT_DKKHAKI, sStartX, sStartY, sPixLen, 96, MSYS_PRIORITY_HIGHEST - 2);

	iMsgBoxOkImg = LoadButtonImage("EDITOR/ok.sti",0,1,2,3,4);
	iMsgBoxCancelImg = LoadButtonImage("EDITOR/cancel.sti",0,1,2,3,4);

	iMsgBoxOk = QuickCreateButton(iMsgBoxOkImg, (INT16)(sStartX + (sPixLen/2) - 35), (INT16)(sStartY + 58),
										BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BUTTON_NO_CALLBACK, MsgBoxOkClkCallback);

	iMsgBoxCancel = QuickCreateButton(iMsgBoxCancelImg, (INT16)(sStartX + (sPixLen/2) + 5), (INT16)(sStartY + 58),
										BUTTON_NO_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BUTTON_NO_CALLBACK, MsgBoxCnclClkCallback);

	SGPRect MsgBoxRect;
	MsgBoxRect.iLeft = sStartX;
	MsgBoxRect.iTop = sStartY;
	MsgBoxRect.iRight = sStartX + sPixLen;
	MsgBoxRect.iBottom = sStartY + 96;

	RestrictMouseCursor( &MsgBoxRect );

	gubMessageBoxStatus = MESSAGEBOX_WAIT;

}

BOOLEAN MessageBoxHandled()
{
  InputAtom DummyEvent;

	while( DequeueEvent( &DummyEvent ) )
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
//	InvalidateScreen( );
//	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender( );
	return gubMessageBoxStatus == MESSAGEBOX_DONE;
}

void RemoveMessageBox( )
{
	FreeMouseCursor( );
	RemoveButton( iMsgBoxCancel );
	RemoveButton( iMsgBoxOk );
	RemoveButton( iMsgBoxBgrnd );
	UnloadButtonImage( iMsgBoxOkImg );
	UnloadButtonImage( iMsgBoxCancelImg );
	gubMessageBoxStatus = MESSAGEBOX_NONE;
}


static void MsgBoxOkClkCallback(GUI_BUTTON* butn, INT32 reason)
{
	if ( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		butn->uiFlags |= BUTTON_CLICKED_ON;
	}
	else if ( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gubMessageBoxStatus = MESSAGEBOX_DONE;
		gfMessageBoxResult = TRUE;
	}
}


static void MsgBoxCnclClkCallback(GUI_BUTTON* butn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		butn->uiFlags |= BUTTON_CLICKED_ON;
	}
	else if ( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gubMessageBoxStatus = MESSAGEBOX_DONE;
		gfMessageBoxResult = FALSE;
	}
}

#endif
