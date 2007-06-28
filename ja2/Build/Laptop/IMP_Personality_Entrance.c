#include "CharProfile.h"
#include "Font.h"
#include "IMP_Personality_Entrance.h"
#include "IMP_MainPage.h"
#include "IMPVideoObjects.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "Button_System.h"
#include "Font_Control.h"


// IMP personality entrance buttons
INT32 giIMPPersonalityEntranceButton[1];
INT32 giIMPPersonalityEntranceButtonImage[1];


static void BtnIMPPersonalityEntranceDoneCallback(GUI_BUTTON *btn, INT32 reason);


static void CreateIMPPersonalityEntranceButtons(void);


void EnterIMPPersonalityEntrance( void )
{
  // create buttons needed
	CreateIMPPersonalityEntranceButtons( );
}

void RenderIMPPersonalityEntrance( void )
{
   // the background
	RenderProfileBackGround( );

	// the IMP symbol
	//RenderIMPSymbol( 112, 30 );

	// indent
  RenderAvgMercIndentFrame(90, 40 );
}


static void DestroyIMPPersonalityEntranceButtons(void);


void ExitIMPPersonalityEntrance( void )
{
  // destroy buttons needed
	DestroyIMPPersonalityEntranceButtons( );
}


void HandleIMPPersonalityEntrance( void )
{
}


static void CreateIMPPersonalityEntranceButtons(void)
{
  // this function will create the buttons needed for the IMP personality Page

	// ths begin button
  giIMPPersonalityEntranceButtonImage[0]=  LoadButtonImage( "LAPTOP/button_2.sti" ,-1,0,-1,1,-1 );
	/*giIMPPersonalityEntranceButton[0] = QuickCreateButton( giIMPPersonalityEntranceButtonImage[0], LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPPersonalityEntranceDoneCallback);
*/
		giIMPPersonalityEntranceButton[0] = CreateIconAndTextButton( giIMPPersonalityEntranceButtonImage[0], pImpButtonText[ 1 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, BtnIMPPersonalityEntranceDoneCallback);


	SetButtonCursor(giIMPPersonalityEntranceButton[0], CURSOR_WWW);
}


static void DestroyIMPPersonalityEntranceButtons(void)
{
	// this function will destroy the buttons needed for the IMP personality page

	// the begin button
  RemoveButton(giIMPPersonalityEntranceButton[0] );
  UnloadButtonImage(giIMPPersonalityEntranceButtonImage[0] );
}


static void BtnIMPPersonalityEntranceDoneCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for IMP Begin Screen done button

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// done with begin screen, next screen
		iCurrentImpPage = IMP_PERSONALITY_QUIZ;
		fButtonPendingFlag = TRUE;
	}
}
