#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "IMP AboutUs.h"
	#include "CharProfile.h"
	#include "IMPVideoObjects.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "Debug.h"
	#include "WordWrap.h"
	#include "Render Dirty.h"
	#include "Encrypted File.h"
	#include "cursors.h"
	#include "laptop.h"
	#include "IMP Text System.h"
#endif

// IMP AboutUs buttons
INT32 giIMPAboutUsButton[1];
INT32 giIMPAboutUsButtonImage[1];
void CreateIMPAboutUsButtons( void );
void DeleteIMPAboutUsButtons( void );;

// fucntions
void BtnIMPBackCallback(GUI_BUTTON *btn,INT32 reason);


void EnterIMPAboutUs( void )
{

	// create buttons
	CreateIMPAboutUsButtons( );

	// entry into IMP about us page
	RenderIMPAboutUs( );

	return;
}


void ExitIMPAboutUs( void )
{
  // exit from IMP About us page

	// delete Buttons
	DeleteIMPAboutUsButtons( );

	return;
}


void RenderIMPAboutUs( void )
{
  // rneders the IMP about us page

	// the background
	RenderProfileBackGround( );

	// the IMP symbol
	RenderIMPSymbol( 106, 1 );

	// about us indent
	RenderAboutUsIndentFrame( 8, 130 );
  // about us indent
	RenderAboutUsIndentFrame( 258, 130 );

	return;
}


void HandleIMPAboutUs( void )
{
  // handles the IMP about us page

	return;
}


void CreateIMPAboutUsButtons( void )
{

  // this function will create the buttons needed for th IMP about us page
  // the back button button
  giIMPAboutUsButtonImage[0]=  LoadButtonImage( "LAPTOP\\button_3.sti" ,-1,0,-1,1,-1 );
	/*giIMPAboutUsButton[0] = QuickCreateButton( giIMPAboutUsButtonImage[0], LAPTOP_SCREEN_UL_X +  426 , LAPTOP_SCREEN_WEB_UL_Y + ( 360 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPBackCallback); */

   giIMPAboutUsButton[0] = CreateIconAndTextButton( giIMPAboutUsButtonImage[0], pImpButtonText[6], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  216, LAPTOP_SCREEN_WEB_UL_Y + ( 360 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPBackCallback);

	 SetButtonCursor(giIMPAboutUsButton[0], CURSOR_WWW);

	 return;
}


void DeleteIMPAboutUsButtons( void )
{
  // this function destroys the buttons needed for the IMP about Us Page

  // the about back button
  RemoveButton(giIMPAboutUsButton[0] );
  UnloadButtonImage(giIMPAboutUsButtonImage[0] );


	return;
}

void BtnIMPBackCallback(GUI_BUTTON *btn,INT32 reason)
{

	// btn callback for IMP Homepage About US button
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		 btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{
      btn->uiFlags&=~(BUTTON_CLICKED_ON);
      iCurrentImpPage = IMP_HOME_PAGE;
		}
	}
}
