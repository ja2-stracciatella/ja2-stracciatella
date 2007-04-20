#include "CharProfile.h"
#include "IMP_Attribute_Entrance.h"
#include "IMP_MainPage.h"
#include "IMP_HomePage.h"
#include "IMPVideoObjects.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Encrypted_File.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "Button_System.h"
#include "Font_Control.h"


// the buttons
UINT32 giIMPAttributeEntranceButtonImage[ 1 ];
UINT32 giIMPAttributeEntranceButton[ 1 ];


static void BtnIMPAttributeBeginCallback(GUI_BUTTON *btn, INT32 reason);


static void CreateIMPAttributeEntranceButtons(void);


void EnterIMPAttributeEntrance( void )
{

	CreateIMPAttributeEntranceButtons( );
}

void RenderIMPAttributeEntrance( void )
{
   // the background
	RenderProfileBackGround( );

	// avg merc indent
	RenderAvgMercIndentFrame(90, 40 );
}


static void DestroyIMPAttributeEntranceButtons(void);


void ExitIMPAttributeEntrance( void )
{
  // destroy the finish buttons
	DestroyIMPAttributeEntranceButtons( );
}


void HandleIMPAttributeEntrance( void )
{
}


static void CreateIMPAttributeEntranceButtons(void)
{

	// the begin button
  giIMPAttributeEntranceButtonImage[0]=  LoadButtonImage( "LAPTOP/button_2.sti" ,-1,0,-1,1,-1 );
	/*
	giIMPAttributeEntranceButton[0] = QuickCreateButton( giIMPAttributeEntranceButtonImage[0], LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPAttributeBeginCallback ); */

	 giIMPAttributeEntranceButton[0] = CreateIconAndTextButton( giIMPAttributeEntranceButtonImage[ 0 ], pImpButtonText[ 13 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, BtnIMPAttributeBeginCallback);

	SetButtonCursor(giIMPAttributeEntranceButton[0], CURSOR_WWW);
}


static void DestroyIMPAttributeEntranceButtons(void)
{
	// this function will destroy the buttons needed for the IMP attrib enter page

	// the begin  button
  RemoveButton(giIMPAttributeEntranceButton[ 0 ] );
  UnloadButtonImage(giIMPAttributeEntranceButtonImage[ 0 ] );
}


static void BtnIMPAttributeBeginCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_ATTRIBUTE_PAGE;
		fButtonPendingFlag = TRUE;
	}
}
