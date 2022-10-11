#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "IMP_HomePage.h"
#include "IMPVideoObjects.h"
#include "MessageBoxScreen.h"
#include "Text.h"
#include "VObject.h"
#include "Cursors.h"
#include "Laptop.h"
#include "Text_Input.h"
#include "LaptopSave.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "IMPPolicy.h"

#include <string_theory/string>


static void BtnIMPAboutUsCallback(GUI_BUTTON *btn, UINT32 reason);

// position defines
#define IMP_PLAYER_ACTIVATION_STRING_X LAPTOP_SCREEN_UL_X + 259
#define IMP_PLAYER_ACTIVATION_STRING_Y LAPTOP_SCREEN_WEB_UL_Y + 330
#define IMP_PLAYER_ACTIVATION_STRING_WIDTH 99
#define IMP_PLAYER_ACTIVATION_STRING_HEIGHT 23
#define IMP_PLAYER_ACTIVATION_STRING_LENGTH 6

// IMP homepage buttons
GUIButtonRef giIMPHomePageButton[1];
static BUTTON_PICS* giIMPHomePageButtonImage[1];

static void InitImpHomepageTextInputBoxes(void) {
	InitTextInputMode();

	SetTextInputFont(FONT14ARIAL);
	Set16BPPTextFieldColor( Get16BPPColor(FROMRGB( 0, 0, 0) ) );
	SetTextInputRegularColors( FONT_LTGREEN, FONT_BLACK );
	SetTextInputHilitedColors( FONT_BLACK, FONT_LTGREEN, FONT_LTGREEN  );
	SetCursorColor( Get16BPPColor(FROMRGB(0, 255, 0) ) );

	AddTextInputField(
		IMP_PLAYER_ACTIVATION_STRING_X,
		IMP_PLAYER_ACTIVATION_STRING_Y,
		IMP_PLAYER_ACTIVATION_STRING_WIDTH,
		IMP_PLAYER_ACTIVATION_STRING_HEIGHT,
		MSYS_PRIORITY_HIGH + 2,
		L"",
		IMP_PLAYER_ACTIVATION_STRING_LENGTH,
		INPUTTYPE_FULL_TEXT
	);

	SetActiveField(0);
}

static void ProcessPlayerInputActivationString(void)
{
	ST::string str = GetStringFromField(0);
	bool stringMatchesCode = GCM->getIMPPolicy()->isCodeAccepted(str);

	if (stringMatchesCode && !LaptopSaveInfo.fIMPCompletedFlag) {
		iCurrentImpPage = IMP_MAIN_PAGE;
		return;
	}

	DoLapTopMessageBox(
		MSG_BOX_IMP_STYLE,
		stringMatchesCode ? pImpPopUpStrings[6] : pImpPopUpStrings[0],
		LAPTOP_SCREEN,
		MSG_BOX_FLAG_OK,
		0
	);
	SetActiveField(0);
}

static void GetPlayerKeyBoardInputForIMPHomePage(void)
{
	InputAtom					InputEvent;
	while (DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS))
	{
		if(!HandleTextInput( &InputEvent ) && (InputEvent.usEvent == KEY_DOWN || InputEvent.usEvent == KEY_REPEAT || InputEvent.usEvent == KEY_UP ) )
		{
			switch( InputEvent.usParam )
			{
				case SDLK_RETURN:
					if(InputEvent.usEvent == KEY_UP)
					{
						// return hit, check to see if current player activation string is a valid one
						ProcessPlayerInputActivationString( );
					}
					break;

				case SDLK_ESCAPE:
					HandleLapTopESCKey();
					break;

				default:
					break;
			}
		}
	}
}

static void CreateIMPHomePageButtons(void)
{
	// this procedure will create the buttons needed for the IMP homepage

	// ths about us button
	giIMPHomePageButtonImage[0] = LoadButtonImage(LAPTOPDIR "/button_1.sti", 0, 1);
	giIMPHomePageButton[0] = CreateIconAndTextButton(
					giIMPHomePageButtonImage[0],
					pImpButtonText[ 0 ], FONT12ARIAL,
					FONT_WHITE, DEFAULT_SHADOW,
					FONT_WHITE, DEFAULT_SHADOW,
					LAPTOP_SCREEN_UL_X + 286 - 106,
					LAPTOP_SCREEN_WEB_UL_Y + 248 - 48,
					MSYS_PRIORITY_HIGH,
					BtnIMPAboutUsCallback);

	giIMPHomePageButton[0]->SetCursor(CURSOR_WWW);
}


static void RemoveIMPHomePageButtons(void)
{
	// this procedure will destroy the already created buttosn for the IMP homepage

	// the about us button
	RemoveButton(giIMPHomePageButton[0] );
	UnloadButtonImage(giIMPHomePageButtonImage[0] );
}


static void BtnIMPAboutUsCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iCurrentImpPage = IMP_ABOUT_US;
		fButtonPendingFlag = TRUE;
	}
}

void EnterImpHomePage( void )
{
	// load buttons
	CreateIMPHomePageButtons( );

	InitImpHomepageTextInputBoxes();

	// render screen once
	RenderImpHomePage( );
}

void RenderImpHomePage( void )
{
	// the background
	RenderProfileBackGround( );

	// the IMP symbol
	RenderIMPSymbol( 107, 45 );

	// the second button image
	RenderButton2Image( 134, 314);

	// render the indents

	//activation indents
	RenderActivationIndent( 257, 328 );

	// the two font page indents
	RenderFrontPageIndent( 3, 64 );
	RenderFrontPageIndent( 396,64 );

	RenderAllTextFields();
}

void ExitImpHomePage( void )
{

	// remove buttons
	RemoveIMPHomePageButtons( );

	KillTextInputMode();
}

void HandleImpHomePage( void )
{
	// handle keyboard input for this screen
	GetPlayerKeyBoardInputForIMPHomePage( );

	RenderAllTextFields();
}
