#include "CharProfile.h"
#include "Font.h"
#include "IMP_Personality_Finish.h"
#include "IMP_MainPage.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "WCheck.h"
#include "Timer_Control.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "IMP_Compile_Character.h"
#include "Button_System.h"
#include "Font_Control.h"


// this is the amount of time, the player waits until booted back to main profileing screen

UINT8 bPersonalityEndState = 0;

#define PERSONALITY_CONFIRM_FINISH_DELAY 2500


// flag set when player hits  YES/NO button
static BOOLEAN fConfirmHasBeenSelectedFlag = FALSE;
static BOOLEAN fConfirmIsYesFlag           = FALSE;
static BOOLEAN fCreatedOkIMPButton         = FALSE;
static BOOLEAN fExitIMPPerFinAtOk          = FALSE;
static BOOLEAN fCreateFinishOkButton       = FALSE;


// buttons
static UINT32 giIMPPersonalityFinishButton[2];
static UINT32 giIMPPersonalityFinishButtonImage[2];


static void BtnIMPPersonalityFinishYesCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityFinishNoCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityFinishOkCallback(GUI_BUTTON *btn, INT32 reason);


static void CreateIMPPersonalityFinishButtons(void);


void EnterIMPPersonalityFinish( void )
{

	// reset states
	fCreateFinishOkButton = FALSE;
	bPersonalityEndState = 0;
	fConfirmIsYesFlag = FALSE;

	// create the buttons
	CreateIMPPersonalityFinishButtons( );
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
}


static void DestroyIMPersonalityFinishButtons(void);
static void DestroyPersonalityFinishOkButton(void);


void ExitIMPPersonalityFinish( void )
{

	// exit at IMP Ok button
	if( fExitIMPPerFinAtOk )
	{
		// destroy the finish ok buttons
	  DestroyPersonalityFinishOkButton( );
	}

	if (!fExitIMPPerFinAtOk)
	{
		// exit due to cancel button, not ok or Yes/no button
		// get rid of yes no
		DestroyIMPersonalityFinishButtons( );
	}


	fCreatedOkIMPButton = FALSE;
	fConfirmHasBeenSelectedFlag = FALSE;
}


static void CheckIfConfirmHasBeenSelectedAndTimeDelayHasPassed(void);


void HandleIMPPersonalityFinish( void )
{

	// check if confirm and delay
	CheckIfConfirmHasBeenSelectedAndTimeDelayHasPassed( );
}


static void CreatePersonalityFinishOkButton(void);


static void CheckIfConfirmHasBeenSelectedAndTimeDelayHasPassed(void)
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
}


static void CreateIMPPersonalityFinishButtons(void)
{
  // this function will create the buttons needed for the IMP personality Finish Page

	// ths Yes button
  giIMPPersonalityFinishButtonImage[0]=  LoadButtonImage( "LAPTOP/button_5.sti" ,-1,0,-1,1,-1 );
	giIMPPersonalityFinishButton[0] = CreateIconAndTextButton( giIMPPersonalityFinishButtonImage[0], pImpButtonText[ 9 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 LAPTOP_SCREEN_UL_X + 90, LAPTOP_SCREEN_WEB_UL_Y + 224, MSYS_PRIORITY_HIGH,
														 	BtnIMPPersonalityFinishYesCallback);

  // the no Button
	giIMPPersonalityFinishButtonImage[ 1 ]=  LoadButtonImage( "LAPTOP/button_5.sti" ,-1,0,-1,1,-1 );
	giIMPPersonalityFinishButton[ 1 ] = CreateIconAndTextButton( giIMPPersonalityFinishButtonImage[ 1 ], pImpButtonText[ 10 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 LAPTOP_SCREEN_UL_X + 276, LAPTOP_SCREEN_WEB_UL_Y + 224, MSYS_PRIORITY_HIGH,
														 	BtnIMPPersonalityFinishNoCallback);

	 SetButtonCursor(giIMPPersonalityFinishButton[0], CURSOR_WWW);
	 SetButtonCursor(giIMPPersonalityFinishButton[1], CURSOR_WWW);
}


static void DestroyIMPersonalityFinishButtons(void)
{
	// this function will destroy the buttons needed for the IMP personality Finish page

	// the yes button
  RemoveButton(giIMPPersonalityFinishButton[ 0 ] );
  UnloadButtonImage(giIMPPersonalityFinishButtonImage[ 0 ] );

  // the no button
	RemoveButton(giIMPPersonalityFinishButton[ 1 ] );
  UnloadButtonImage(giIMPPersonalityFinishButtonImage[ 1 ] );
}


static void BtnIMPPersonalityFinishYesCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for IMP personality quiz answer button

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// confirm flag set, get out of HERE!
	   if (fConfirmHasBeenSelectedFlag)
		 {
			 // now set this button off
       btn->uiFlags&= ~BUTTON_CLICKED_ON;
			 return;
		 }
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set fact yes was selected
		fConfirmIsYesFlag = TRUE;

		// set fact that confirmation has been done
		fConfirmHasBeenSelectedFlag = TRUE;

		// now make skill, personality and attitude
		CreatePlayersPersonalitySkillsAndAttitude();
		fButtonPendingFlag = TRUE;
		bPersonalityEndState = 1;
	}
}


static void BtnIMPPersonalityFinishNoCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for IMP personality quiz answer button

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
     // confirm flag set, get out of HERE!
	   if (fConfirmHasBeenSelectedFlag)
		 {
			 // now set this button off
       btn->uiFlags&=~(BUTTON_CLICKED_ON);
			 return;
		 }
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set fact yes was selected
		fConfirmIsYesFlag = FALSE;

		// set fact that confirmation has been done
		fConfirmHasBeenSelectedFlag = TRUE;
		CreatePlayersPersonalitySkillsAndAttitude();

		bPersonalityEndState = 2;
		fButtonPendingFlag = TRUE;
	}
}


static void CreatePersonalityFinishOkButton(void)
{

	// create personality button finish button
	giIMPPersonalityFinishButtonImage[0]=  LoadButtonImage( "LAPTOP/button_5.sti" ,-1,0,-1,1,-1 );
  giIMPPersonalityFinishButton[0] = CreateIconAndTextButton( giIMPPersonalityFinishButtonImage[0], pImpButtonText[ 24 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 LAPTOP_SCREEN_UL_X + 186, LAPTOP_SCREEN_WEB_UL_Y + 224, MSYS_PRIORITY_HIGH,
														 BtnIMPPersonalityFinishOkCallback);

	SetButtonCursor(giIMPPersonalityFinishButton[0], CURSOR_WWW);
}


static void DestroyPersonalityFinishOkButton(void)
{
		// the ok button
  RemoveButton(giIMPPersonalityFinishButton[ 0 ] );
  UnloadButtonImage(giIMPPersonalityFinishButtonImage[ 0 ] );
}


static void BtnIMPPersonalityFinishOkCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for IMP personality quiz answer button

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (iCurrentProfileMode < 2)
		{
			iCurrentProfileMode = 2;
		}

		// button pending, wait a frame
		fButtonPendingFlag = TRUE;
		iCurrentImpPage = IMP_MAIN_PAGE;
	}
}
