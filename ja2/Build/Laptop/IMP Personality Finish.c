#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "CharProfile.h"
	#include "IMP Personality Finish.h"
	#include "IMP MainPage.h"
	#include "IMP HomePage.h"
	#include "IMPVideoObjects.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "Timer Control.h"
	#include "Debug.h"
	#include "WordWrap.h"
	#include "Render Dirty.h"
	#include "Encrypted File.h"
	#include "cursors.h"
	#include "laptop.h"
	#include "IMP Text System.h"
	#include "IMP Compile Character.h"
#endif

// this is the amount of time, the player waits until booted back to main profileing screen

UINT8 bPersonalityEndState = 0;

#define PERSONALITY_CONFIRM_FINISH_DELAY 2500


// flag set when player hits  YES/NO button
BOOLEAN fConfirmHasBeenSelectedFlag = FALSE;
BOOLEAN fConfirmIsYesFlag = FALSE;
BOOLEAN fOkToReturnIMPMainPageFromPersonalityFlag = FALSE;
BOOLEAN fCreatedOkIMPButton = FALSE;
BOOLEAN fExitDueFrIMPPerFinToOkButton = FALSE;
BOOLEAN fExitIMPPerFinAtOk = FALSE;
BOOLEAN fCreateFinishOkButton = FALSE;


// buttons
UINT32 giIMPPersonalityFinishButton[2];
UINT32 giIMPPersonalityFinishButtonImage[2];



// function definitions
void CreateIMPPersonalityFinishButtons( void );
void CheckIfConfirmHasBeenSelectedAndTimeDelayHasPassed( void );
void DestroyIMPersonalityFinishButtons( void );
void CreatePersonalityFinishOkButton( void );
void DestroyPersonalityFinishOkButton( void );



// callbacks
void BtnIMPPersonalityFinishYesCallback(GUI_BUTTON *btn,INT32 reason);
void BtnIMPPersonalityFinishNoCallback(GUI_BUTTON *btn,INT32 reason);
void BtnIMPPersonalityFinishOkCallback(GUI_BUTTON *btn,INT32 reason);



void EnterIMPPersonalityFinish( void )
{

	// reset states
	fCreateFinishOkButton = FALSE;
	bPersonalityEndState = 0;
	fConfirmIsYesFlag = FALSE;

	// create the buttons
	CreateIMPPersonalityFinishButtons( );

	return;
}

void RenderIMPPersonalityFinish( void )
{
   // the background
	RenderProfileBackGround( );

		// indent for text
  RenderBeginIndent( 110, 93 );

	// check confirm flag to decide if we have to display appropriate response to button action
	if( fConfirmHasBeenSelectedFlag )
	{
		// confirm was yes, display yes string
		if(fConfirmIsYesFlag == TRUE)
		{
      // display yes string
			PrintImpText( );
		}
		else
		{
			// display no string
      PrintImpText( );
		}
	}
	return;
}

void ExitIMPPersonalityFinish( void )
{

	// exit at IMP Ok button
	if( fExitIMPPerFinAtOk )
	{
		// destroy the finish ok buttons
	  DestroyPersonalityFinishOkButton( );
	}


	if( ( fExitDueFrIMPPerFinToOkButton == FALSE ) &&( fExitIMPPerFinAtOk == FALSE ) )
	{
		// exit due to cancel button, not ok or Yes/no button
		// get rid of yes no
		DestroyIMPersonalityFinishButtons( );
	}


	fCreatedOkIMPButton = FALSE;
	fOkToReturnIMPMainPageFromPersonalityFlag = FALSE;
	fConfirmHasBeenSelectedFlag = FALSE;
	return;
}


void HandleIMPPersonalityFinish( void )
{

	// check if confirm and delay
	CheckIfConfirmHasBeenSelectedAndTimeDelayHasPassed( );


  return;
}


void CheckIfConfirmHasBeenSelectedAndTimeDelayHasPassed( void )
{
	// this function will check to see if player has in fact confirmed selection and delay to
	// read text has occurred



	// if not confirm selected, return
	if( fConfirmHasBeenSelectedFlag == FALSE )
	{
		return;
	}

	if( fCreateFinishOkButton == TRUE )
	{
		fCreateFinishOkButton = FALSE;
		CreatePersonalityFinishOkButton( );
		fCreatedOkIMPButton = TRUE;
	}

	// create ok button
	if( fCreatedOkIMPButton == FALSE )
	{
		DestroyIMPersonalityFinishButtons( );
		fCreateFinishOkButton = TRUE;
		fExitIMPPerFinAtOk = TRUE;
	}

	// ok to return
	if( fOkToReturnIMPMainPageFromPersonalityFlag == TRUE )
	{
		DestroyPersonalityFinishOkButton( );
		fCreatedOkIMPButton = FALSE;
		fOkToReturnIMPMainPageFromPersonalityFlag = FALSE;
		fConfirmHasBeenSelectedFlag = FALSE;
		fExitDueFrIMPPerFinToOkButton = TRUE;
	}

	return;
}


void CreateIMPPersonalityFinishButtons( void )
{
  // this function will create the buttons needed for the IMP personality Finish Page

	// ths Yes button
  giIMPPersonalityFinishButtonImage[0]=  LoadButtonImage( "LAPTOP\\button_5.sti" ,-1,0,-1,1,-1 );
/*	giIMPPersonalityFinishButton[0] = QuickCreateButton( giIMPPersonalityFinishButtonImage[0], LAPTOP_SCREEN_UL_X +  ( 90 ), LAPTOP_SCREEN_WEB_UL_Y + ( 224 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPPersonalityFinishYesCallback);
*/
	giIMPPersonalityFinishButton[0] = CreateIconAndTextButton( giIMPPersonalityFinishButtonImage[0], pImpButtonText[ 9 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 90 ), LAPTOP_SCREEN_WEB_UL_Y + ( 224 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPPersonalityFinishYesCallback);

  // the no Button
	giIMPPersonalityFinishButtonImage[ 1 ]=  LoadButtonImage( "LAPTOP\\button_5.sti" ,-1,0,-1,1,-1 );
/*	giIMPPersonalityFinishButton[ 1 ] = QuickCreateButton( giIMPPersonalityFinishButtonImage[1], LAPTOP_SCREEN_UL_X +  ( 276 ), LAPTOP_SCREEN_WEB_UL_Y + ( 224 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPPersonalityFinishNoCallback);
  */
	giIMPPersonalityFinishButton[ 1 ] = CreateIconAndTextButton( giIMPPersonalityFinishButtonImage[ 1 ], pImpButtonText[ 10 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 276 ), LAPTOP_SCREEN_WEB_UL_Y + ( 224 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPPersonalityFinishNoCallback);

	 SetButtonCursor(giIMPPersonalityFinishButton[0], CURSOR_WWW);
	 SetButtonCursor(giIMPPersonalityFinishButton[1], CURSOR_WWW);


	return;
}


void DestroyIMPersonalityFinishButtons( void )
{
	// this function will destroy the buttons needed for the IMP personality Finish page

	// the yes button
  RemoveButton(giIMPPersonalityFinishButton[ 0 ] );
  UnloadButtonImage(giIMPPersonalityFinishButtonImage[ 0 ] );

  // the no button
	RemoveButton(giIMPPersonalityFinishButton[ 1 ] );
  UnloadButtonImage(giIMPPersonalityFinishButtonImage[ 1 ] );

	return;
}


void BtnIMPPersonalityFinishYesCallback(GUI_BUTTON *btn,INT32 reason)
{

	// btn callback for IMP personality quiz answer button
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		// confirm flag set, get out of HERE!
	   if( fConfirmHasBeenSelectedFlag )
		 {
			 // now set this button off
       btn->uiFlags&=~(BUTTON_CLICKED_ON);

			 return;
		 }

		 btn->uiFlags|=(BUTTON_CLICKED_ON);

	}
	else if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{

		  // now set this button off
      btn->uiFlags&=~(BUTTON_CLICKED_ON);

			// set fact yes was selected
		  fConfirmIsYesFlag = TRUE;

			// set fact that confirmation has been done
			fConfirmHasBeenSelectedFlag = TRUE;

			// now make skill, personality and attitude
			CreatePlayersPersonalitySkillsAndAttitude( );
			fButtonPendingFlag = TRUE;
      bPersonalityEndState = 1;
		}
	}
}



void BtnIMPPersonalityFinishNoCallback(GUI_BUTTON *btn,INT32 reason)
{

	// btn callback for IMP personality quiz answer button
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
     // confirm flag set, get out of HERE!
	   if( fConfirmHasBeenSelectedFlag )
		 {
			 // now set this button off
       btn->uiFlags&=~(BUTTON_CLICKED_ON);

			 return;
		 }

		 btn->uiFlags|=(BUTTON_CLICKED_ON);

	}
	else if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{

			// now set this button on
      btn->uiFlags&=~(BUTTON_CLICKED_ON);

			// set fact yes was selected
		  fConfirmIsYesFlag = FALSE;

			// set fact that confirmation has been done
			fConfirmHasBeenSelectedFlag = TRUE;
			CreatePlayersPersonalitySkillsAndAttitude( );

      bPersonalityEndState = 2;
			fButtonPendingFlag = TRUE;
		}
	}
}



void CreatePersonalityFinishOkButton( void )
{

	// create personality button finish button
	giIMPPersonalityFinishButtonImage[0]=  LoadButtonImage( "LAPTOP\\button_5.sti" ,-1,0,-1,1,-1 );
  giIMPPersonalityFinishButton[0] = CreateIconAndTextButton( giIMPPersonalityFinishButtonImage[0], pImpButtonText[ 24 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 186 ), LAPTOP_SCREEN_WEB_UL_Y + ( 224 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPPersonalityFinishOkCallback);

	SetButtonCursor(giIMPPersonalityFinishButton[0], CURSOR_WWW);


	return;
}


void DestroyPersonalityFinishOkButton( void )
{
		// the ok button
  RemoveButton(giIMPPersonalityFinishButton[ 0 ] );
  UnloadButtonImage(giIMPPersonalityFinishButtonImage[ 0 ] );
}



void BtnIMPPersonalityFinishOkCallback(GUI_BUTTON *btn,INT32 reason)
{

	// btn callback for IMP personality quiz answer button
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{

		 btn->uiFlags|=(BUTTON_CLICKED_ON);

	}
	else if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{
			// now set this button on
      btn->uiFlags&=~(BUTTON_CLICKED_ON);

			if( iCurrentProfileMode < 2 )
			{
				iCurrentProfileMode = 2;
			}

			// button pending, wait a frame
			fButtonPendingFlag = TRUE;
			iCurrentImpPage = IMP_MAIN_PAGE;
		}
	}
}
