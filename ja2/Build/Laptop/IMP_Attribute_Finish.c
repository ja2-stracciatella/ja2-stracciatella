#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "CharProfile.h"
	#include "IMP Attribute Finish.h"
	#include "IMP MainPage.h"
	#include "IMP HomePage.h"
	#include "IMPVideoObjects.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "input.h"
	#include "Isometric Utils.h"
	#include "Debug.h"
	#include "WordWrap.h"
	#include "Render Dirty.h"
	#include "Encrypted File.h"
	#include "cursors.h"
	#include "laptop.h"
	#include "IMP Text System.h"
	#include "IMP Attribute Selection.h"
#endif

// buttons
INT32 giIMPAttributeFinishButtonImage[ 2 ];
INT32 giIMPAttributeFinishButton[ 2 ];

// function definitions
void CreateAttributeFinishButtons( void );
void DestroyAttributeFinishButtons( void );
extern void SetGeneratedCharacterAttributes( void );


// callbacks
void BtnIMPAttributeFinishYesCallback(GUI_BUTTON *btn,INT32 reason);
void BtnIMPAttributeFinishNoCallback(GUI_BUTTON *btn,INT32 reason);


void EnterIMPAttributeFinish( void )
{
   // create the needed buttons
	CreateAttributeFinishButtons( );

	// render screen
   RenderIMPAttributeFinish( );


	return;
}


void RenderIMPAttributeFinish( void )
{
  // render background
		RenderProfileBackGround( );

	// indent for text
  RenderBeginIndent( 110, 93 );

	return;
}


void ExitIMPAttributeFinish( void )
{
  // destroy the buttons for this screen
	DestroyAttributeFinishButtons( );

	return;
}

void HandleIMPAttributeFinish( void )
{

	return;
}



void CreateAttributeFinishButtons( void )
{

	// this procedure will create the buttons needed for the attribute finish screen

	// the yes button
  giIMPAttributeFinishButtonImage[0]=  LoadButtonImage( "LAPTOP\\button_2.sti" ,-1,0,-1,1,-1 );
/*	giIMPAttributeFinishButton[0] = QuickCreateButton( giIMPAttributeFinishButtonImage[0], LAPTOP_SCREEN_UL_X +  ( 90 ), LAPTOP_SCREEN_WEB_UL_Y + ( 224 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPAttributeFinishYesCallback );

*/
	 giIMPAttributeFinishButton[0] = CreateIconAndTextButton(  giIMPAttributeFinishButtonImage[0], pImpButtonText[ 20 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 130 ), LAPTOP_SCREEN_WEB_UL_Y + ( 180 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPAttributeFinishYesCallback );

	// the no button
  giIMPAttributeFinishButtonImage[1]=  LoadButtonImage( "LAPTOP\\button_2.sti" ,-1,0,-1,1,-1 );
/*	giIMPAttributeFinishButton[1] = QuickCreateButton( giIMPAttributeFinishButtonImage[1], LAPTOP_SCREEN_UL_X +  ( 276 ), LAPTOP_SCREEN_WEB_UL_Y + ( 224 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPAttributeFinishNoCallback );
*/
	 giIMPAttributeFinishButton[1] = CreateIconAndTextButton(  giIMPAttributeFinishButtonImage[1], pImpButtonText[ 21 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 130 ), LAPTOP_SCREEN_WEB_UL_Y + ( 264 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPAttributeFinishNoCallback);

  SetButtonCursor(giIMPAttributeFinishButton[0], CURSOR_WWW);
	SetButtonCursor(giIMPAttributeFinishButton[1], CURSOR_WWW);



  return;

}


void DestroyAttributeFinishButtons( void )
{

	// this procedure will destroy the buttons for the attribute finish screen

	// the yes  button
  RemoveButton(giIMPAttributeFinishButton[ 0 ] );
  UnloadButtonImage(giIMPAttributeFinishButtonImage[ 0 ] );

	// the no  button
  RemoveButton(giIMPAttributeFinishButton[ 1 ] );
  UnloadButtonImage(giIMPAttributeFinishButtonImage[ 1 ] );

	return;
}


void BtnIMPAttributeFinishYesCallback(GUI_BUTTON *btn,INT32 reason)
{

	// btn callback for IMP personality quiz answer button
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
      // gone far enough
		  iCurrentImpPage = IMP_MAIN_PAGE;
		  if( iCurrentProfileMode < 3 )
			{
        iCurrentProfileMode = 3;
			}
		  // if we are already done, leave
	    if( iCurrentProfileMode == 5)
			{
	      iCurrentImpPage = IMP_FINISH;
			}

			// SET ATTRIBUTES NOW
			SetGeneratedCharacterAttributes( );
			fButtonPendingFlag = TRUE;
		}
	}
}

void BtnIMPAttributeFinishNoCallback(GUI_BUTTON *btn,INT32 reason)
{

	// btn callback for IMP personality quiz answer button
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
		// if no, return to attribute
		iCurrentImpPage = IMP_ATTRIBUTE_PAGE;
		fReturnStatus = TRUE;
		fButtonPendingFlag = TRUE;
		}
	}
}
