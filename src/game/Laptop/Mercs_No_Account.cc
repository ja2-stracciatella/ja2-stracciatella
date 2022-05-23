#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "Mercs_No_Account.h"
#include "Mercs.h"
#include "VObject.h"
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

#include <string_theory/string>


#define MERC_NA_TEXT_FONT	FONT12ARIAL
#define MERC_NA_TEXT_COLOR	FONT_MCOLOR_WHITE


#define MERC_NO_ACCOUNT_IMAGE_X	LAPTOP_SCREEN_UL_X + 23
#define MERC_NO_ACCOUNT_IMAGE_Y	LAPTOP_SCREEN_UL_Y + 52

#define MERC_OPEN_BUTTON_X	(STD_SCREEN_X + 130)
#define MERC_CANCEL_BUTTON_X	(STD_SCREEN_X + 490)
#define MERC_BUTTON_Y		(STD_SCREEN_Y + 380)

#define MERC_NA_SENTENCE_X	MERC_NO_ACCOUNT_IMAGE_X + 10
#define MERC_NA_SENTENCE_Y	MERC_NO_ACCOUNT_IMAGE_Y + 75
#define MERC_NA_SENTENCE_WIDTH	460 - 20


static SGPVObject* guiNoAccountImage;


// The Open Account Box button
static void BtnOpenAccountBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static BUTTON_PICS* guiOpenAccountBoxButtonImage;
GUIButtonRef guiOpenAccountBoxButton;

// The Cancel Account Box button
static void BtnCancelBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason);
GUIButtonRef guiCancelBoxButton;


static GUIButtonRef MakeButton(const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(guiOpenAccountBoxButtonImage, text, FONT12ARIAL, MERC_BUTTON_UP_COLOR, shadow_col, MERC_BUTTON_DOWN_COLOR, shadow_col, x, MERC_BUTTON_Y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


void EnterMercsNoAccount()
{
	InitMercBackGround();

	// load the Account box graphic and add it
	guiNoAccountImage = AddVideoObjectFromFile(LAPTOPDIR "/noaccountbox.sti");

	// Open Accouint button
	guiOpenAccountBoxButtonImage = LoadButtonImage(LAPTOPDIR "/bigbuttons.sti", 0, 1);
	guiOpenAccountBoxButton = MakeButton(MercNoAccountText[MERC_NO_ACC_OPEN_ACCOUNT], MERC_OPEN_BUTTON_X,   BtnOpenAccountBoxButtonCallback);
	guiCancelBoxButton      = MakeButton(MercNoAccountText[MERC_NO_ACC_CANCEL],       MERC_CANCEL_BUTTON_X, BtnCancelBoxButtonCallback);

	RenderMercsNoAccount();
}


void ExitMercsNoAccount()
{
	DeleteVideoObject(guiNoAccountImage);

	UnloadButtonImage( guiOpenAccountBoxButtonImage );
	RemoveButton( guiOpenAccountBoxButton );
	RemoveButton( guiCancelBoxButton );

	RemoveMercBackGround();
}


void RenderMercsNoAccount()
{
	DrawMecBackGround();

	BltVideoObject(FRAME_BUFFER, guiNoAccountImage, 0, MERC_NO_ACCOUNT_IMAGE_X, MERC_NO_ACCOUNT_IMAGE_Y);

	//Display the sentence
	DisplayWrappedString(MERC_NA_SENTENCE_X, MERC_NA_SENTENCE_Y, MERC_NA_SENTENCE_WIDTH, 2, MERC_NA_TEXT_FONT, MERC_NA_TEXT_COLOR, MercNoAccountText[MERC_NO_ACC_NO_ACCOUNT_OPEN_ONE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnOpenAccountBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
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


static void BtnCancelBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_MERC;
		gubArrivedFromMercSubSite = MERC_CAME_FROM_ACCOUNTS_PAGE;
	}
}
