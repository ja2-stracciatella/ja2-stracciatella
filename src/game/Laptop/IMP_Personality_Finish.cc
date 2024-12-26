#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "IMP_Personality_Finish.h"
#include "IMP_MainPage.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "IMP_Compile_Character.h"
#include "Button_System.h"
#include "Font_Control.h"

#include <string_theory/string>


// this is the amount of time, the player waits until booted back to main profileing screen

UINT8 bPersonalityEndState = 0;

#define PERSONALITY_CONFIRM_FINISH_DELAY 2500


// flag set when player hits  YES/NO button
static BOOLEAN fConfirmHasBeenSelectedFlag = false;
static BOOLEAN fConfirmIsYesFlag           = false;
static BOOLEAN fCreatedOkIMPButton         = false;
static BOOLEAN fExitIMPPerFinAtOk          = false;
static BOOLEAN fCreateFinishOkButton       = false;


// buttons
static BUTTON_PICS* giIMPPersonalityFinishButtonImage[2];
static GUIButtonRef giIMPPersonalityFinishButton[2];


static void BtnIMPPersonalityFinishYesCallback(GUI_BUTTON *btn, UINT32 reason);
static void BtnIMPPersonalityFinishNoCallback(GUI_BUTTON *btn, UINT32 reason);
static void BtnIMPPersonalityFinishOkCallback(GUI_BUTTON *btn, UINT32 reason);


static void CreateIMPPersonalityFinishButtons(void);


void EnterIMPPersonalityFinish( void )
{

	// reset states
	fCreateFinishOkButton = false;
	bPersonalityEndState = 0;
	fConfirmIsYesFlag = false;

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
		if (fConfirmIsYesFlag)
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


	fCreatedOkIMPButton = false;
	fConfirmHasBeenSelectedFlag = false;
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

	if (!fConfirmHasBeenSelectedFlag) return;

	if (fCreateFinishOkButton)
	{
		fCreateFinishOkButton = false;
		CreatePersonalityFinishOkButton( );
		fCreatedOkIMPButton = true;
	}

	// create ok button
	if (!fCreatedOkIMPButton)
	{
		DestroyIMPersonalityFinishButtons( );
		fCreateFinishOkButton = true;
		fExitIMPPerFinAtOk = true;
	}
}


static void MakeButton(UINT idx, const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	BUTTON_PICS* const img = LoadButtonImage(LAPTOPDIR "/button_5.sti", 0, 1);
	giIMPPersonalityFinishButtonImage[idx] = img;
	const INT16 text_col   = FONT_WHITE;
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, FONT12ARIAL, text_col, shadow_col, text_col, shadow_col, x, LAPTOP_SCREEN_UL_Y + 224, MSYS_PRIORITY_HIGH, click);
	giIMPPersonalityFinishButton[idx] = btn;
	btn->SetCursor(CURSOR_WWW);
}


static void CreateIMPPersonalityFinishButtons(void)
{
	// this function will create the buttons needed for the IMP personality Finish Page
	MakeButton(0, pImpButtonText[ 9], LAPTOP_SCREEN_UL_X +  90, BtnIMPPersonalityFinishYesCallback); // Yes button
	MakeButton(1, pImpButtonText[10], LAPTOP_SCREEN_UL_X + 276, BtnIMPPersonalityFinishNoCallback);  // No button
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


static void BtnIMPPersonalityFinishYesCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for IMP personality quiz answer button

	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		// confirm flag set, get out of HERE!
		if (fConfirmHasBeenSelectedFlag)
		{
			// now set this button off
			btn->uiFlags&= ~BUTTON_CLICKED_ON;
			return;
		}
	}
	else if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// set fact yes was selected
		fConfirmIsYesFlag = true;

		// set fact that confirmation has been done
		fConfirmHasBeenSelectedFlag = true;

		// now make skill, personality and attitude
		CreatePlayersPersonalitySkillsAndAttitude();
		fButtonPendingFlag = true;
		bPersonalityEndState = 1;
	}
}


static void BtnIMPPersonalityFinishNoCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for IMP personality quiz answer button

	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		// confirm flag set, get out of HERE!
		if (fConfirmHasBeenSelectedFlag)
		{
			// now set this button off
			btn->uiFlags&=~(BUTTON_CLICKED_ON);
			return;
		}
	}
	else if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// set fact yes was selected
		fConfirmIsYesFlag = false;

		// set fact that confirmation has been done
		fConfirmHasBeenSelectedFlag = true;
		CreatePlayersPersonalitySkillsAndAttitude();

		bPersonalityEndState = 2;
		fButtonPendingFlag = true;
	}
}


static void CreatePersonalityFinishOkButton(void)
{
	// create personality button finish button
	MakeButton(0, pImpButtonText[24], LAPTOP_SCREEN_UL_X + 186, BtnIMPPersonalityFinishOkCallback);
}


static void DestroyPersonalityFinishOkButton(void)
{
		// the ok button
	RemoveButton(giIMPPersonalityFinishButton[ 0 ] );
	UnloadButtonImage(giIMPPersonalityFinishButtonImage[ 0 ] );
}


static void BtnIMPPersonalityFinishOkCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for IMP personality quiz answer button

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (iCurrentProfileMode < 2)
		{
			iCurrentProfileMode = 2;
		}

		// button pending, wait a frame
		fButtonPendingFlag = true;
		iCurrentImpPage = IMP_MAIN_PAGE;
	}
}
