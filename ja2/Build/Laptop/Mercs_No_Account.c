#include "Laptop.h"
#include "Mercs_No_Account.h"
#include "Mercs.h"
#include "Utilities.h"
#include "WCheck.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "LaptopSave.h"
#include "Random.h"
#include "Text.h"
#include "Speck_Quotes.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"


#define		MERC_NA_TEXT_FONT									FONT12ARIAL
#define		MERC_NA_TEXT_COLOR								FONT_MCOLOR_WHITE


#define		MERC_NO_ACCOUNT_IMAGE_X						LAPTOP_SCREEN_UL_X + 23
#define		MERC_NO_ACCOUNT_IMAGE_Y						LAPTOP_SCREEN_UL_Y + 52

#define		MERC_OPEN_BUTTON_X								130
#define		MERC_OPEN_BUTTON_Y								380

#define		MERC_CANCEL_BUTTON_X							490
#define		MERC_CANCEL_BUTTON_Y							MERC_OPEN_BUTTON_Y

#define		MERC_NA_SENTENCE_X								MERC_NO_ACCOUNT_IMAGE_X + 10
#define		MERC_NA_SENTENCE_Y								MERC_NO_ACCOUNT_IMAGE_Y + 75
#define		MERC_NA_SENTENCE_WIDTH						460 - 20

UINT32		guiNoAccountImage;



// The Open Account Box button
static void BtnOpenAccountBoxButtonCallback(GUI_BUTTON *btn, INT32 reason);
UINT32	guiOpenAccountBoxButton;
INT32		guiOpenAccountBoxButtonImage;

// The Cancel Account Box button
static void BtnCancelBoxButtonCallback(GUI_BUTTON *btn, INT32 reason);
UINT32	guiCancelBoxButton;


BOOLEAN EnterMercsNoAccount()
{
	InitMercBackGround();

	// load the Account box graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP/NoAccountBox.sti", &guiNoAccountImage));


	// Open Accouint button
	guiOpenAccountBoxButtonImage = LoadButtonImage("LAPTOP/BigButtons.sti", -1,0,-1,1,-1 );

	guiOpenAccountBoxButton = CreateIconAndTextButton( guiOpenAccountBoxButtonImage, MercNoAccountText[MERC_NO_ACC_OPEN_ACCOUNT],
													 FONT12ARIAL,
													 MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW,
													 MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 MERC_OPEN_BUTTON_X, MERC_OPEN_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnOpenAccountBoxButtonCallback);
	SetButtonCursor(guiOpenAccountBoxButton, CURSOR_LAPTOP_SCREEN);
	SpecifyDisabledButtonStyle( guiOpenAccountBoxButton, DISABLED_STYLE_SHADED);


	guiCancelBoxButton = CreateIconAndTextButton( guiOpenAccountBoxButtonImage, MercNoAccountText[MERC_NO_ACC_CANCEL],
													 FONT12ARIAL,
													 MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW,
													 MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 MERC_CANCEL_BUTTON_X, MERC_CANCEL_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnCancelBoxButtonCallback);
	SetButtonCursor(guiCancelBoxButton, CURSOR_LAPTOP_SCREEN);
	SpecifyDisabledButtonStyle( guiCancelBoxButton, DISABLED_STYLE_SHADED);

	RenderMercsNoAccount();

	return(TRUE);
}

void ExitMercsNoAccount()
{
	DeleteVideoObjectFromIndex(guiNoAccountImage);

	UnloadButtonImage( guiOpenAccountBoxButtonImage );
	RemoveButton( guiOpenAccountBoxButton );
	RemoveButton( guiCancelBoxButton );

	RemoveMercBackGround();
}

void HandleMercsNoAccount()
{

}

void RenderMercsNoAccount()
{
	DrawMecBackGround();

	BltVideoObjectFromIndex(FRAME_BUFFER, guiNoAccountImage, 0,MERC_NO_ACCOUNT_IMAGE_X, MERC_NO_ACCOUNT_IMAGE_Y);

	//Display the sentence
	DisplayWrappedString(MERC_NA_SENTENCE_X, MERC_NA_SENTENCE_Y, MERC_NA_SENTENCE_WIDTH, 2, MERC_NA_TEXT_FONT, MERC_NA_TEXT_COLOR, MercNoAccountText[MERC_NO_ACC_NO_ACCOUNT_OPEN_ONE], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnOpenAccountBoxButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//open an account
		LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_VALID;

		//Get an account number
		LaptopSaveInfo.guiPlayersMercAccountNumber = Random(99999);

		gusMercVideoSpeckSpeech = SPECK_QUOTE_THANK_PLAYER_FOR_OPENING_ACCOUNT;

		guiCurrentLaptopMode = LAPTOP_MODE_MERC;
		gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;
	}
}


static void BtnCancelBoxButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_MERC;
		gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;
	}
}
