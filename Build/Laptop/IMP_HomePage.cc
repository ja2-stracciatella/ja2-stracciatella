#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "IMP_HomePage.h"
#include "IMPVideoObjects.h"
#include "Local.h"
#include "MessageBoxScreen.h"
#include "Text.h"
#include "VObject.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "Timer_Control.h"
#include "Text_Input.h"
#include "LaptopSave.h"
#include "Line.h"
#include "English.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "ScreenIDs.h"
#include "Font_Control.h"
#include "UILayout.h"


const UINT32 GlowColorsList[] = {
	FROMRGB(0,   0, 0),
	FROMRGB(0,  25, 0),
	FROMRGB(0,  50, 0),
	FROMRGB(0,  75, 0),
	FROMRGB(0, 100, 0),
	FROMRGB(0, 125, 0),
	FROMRGB(0, 150, 0),
	FROMRGB(0, 175, 0),
	FROMRGB(0, 200, 0),
	FROMRGB(0, 225, 0),
	FROMRGB(0, 255, 0)
};

static void BtnIMPAboutUsCallback(GUI_BUTTON *btn, INT32 reason);

// position defines
#define IMP_PLAYER_ACTIVATION_STRING_X LAPTOP_SCREEN_UL_X + 261
#define IMP_PLAYER_ACTIVATION_STRING_Y LAPTOP_SCREEN_WEB_UL_Y + 336
#define CURSOR_Y IMP_PLAYER_ACTIVATION_STRING_Y - 5


// IMP homepage buttons
GUIButtonRef giIMPHomePageButton[1];
static BUTTON_PICS* giIMPHomePageButtonImage[1];

// the player activation string
wchar_t pPlayerActivationString[32];

// position within player activation string
INT32 iStringPos=0;
UINT16 uiCursorPosition = IMP_PLAYER_ACTIVATION_STRING_X;


// has a new char been added or deleted?
BOOLEAN fNewCharInActivationString = FALSE;


static void CreateIMPHomePageButtons(void);


void EnterImpHomePage( void )
{
   // upon entry to Imp home page
   memset(pPlayerActivationString, 0, sizeof(pPlayerActivationString));

	 // reset string position
	 iStringPos =0;

	 // reset activation  cursor position
   uiCursorPosition = IMP_PLAYER_ACTIVATION_STRING_X;

	 // load buttons
   CreateIMPHomePageButtons( );

	 // render screen once
	 RenderImpHomePage( );
}


static void DisplayPlayerActivationString(void);


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


	// render the  activation string
	DisplayPlayerActivationString( );
}


static void RemoveIMPHomePageButtons(void);


void ExitImpHomePage( void )
{

	// remove buttons
  RemoveIMPHomePageButtons( );
}


static void DisplayActivationStringCursor(void);
static void GetPlayerKeyBoardInputForIMPHomePage(void);


void HandleImpHomePage( void )
{

	// handle keyboard input for this screen
  GetPlayerKeyBoardInputForIMPHomePage( );

	// has a new char been added to activation string
  if( fNewCharInActivationString )
	{
		// display string
    DisplayPlayerActivationString( );
	}

	// render the cursor
	DisplayActivationStringCursor( );
}


static void DisplayPlayerActivationString(void)
{

	// this function will grab the string that the player will enter for activation

	// player gone too far, move back
	if(iStringPos > 64)
	{
		iStringPos = 64;
	}

	// restore background
	RenderActivationIndent( 257, 328 );

	SetFontAttributes(FONT14ARIAL, 184);
	MPrint(IMP_PLAYER_ACTIVATION_STRING_X, IMP_PLAYER_ACTIVATION_STRING_Y, pPlayerActivationString);

	fNewCharInActivationString = FALSE;
  fReDrawScreenFlag = TRUE;
}


static void DisplayActivationStringCursor(void)
{
	// this procdure will draw the activation string cursor on the screen at position cursorx cursory
	static UINT32 uiBaseTime = 0;
	UINT32 uiDeltaTime = 0;
  static UINT32 iCurrentState = 0;
  static BOOLEAN fIncrement = TRUE;

	if(uiBaseTime == 0)
	{
		uiBaseTime = GetJA2Clock();
	}

	// get difference
	uiDeltaTime = GetJA2Clock() - uiBaseTime;

	// if difference is long enough, rotate colors
	if(uiDeltaTime > MIN_GLOW_DELTA)
  {
		if( iCurrentState == 10)
		{
		  // start rotating downward
			fIncrement = FALSE;
		}
		if( iCurrentState == 0)
		{
			// rotate colors upward
			fIncrement = TRUE;
		}
		// if increment upward, increment iCurrentState
    if(fIncrement)
		{
			iCurrentState++;
		}
		else
		{
			// else downwards
			iCurrentState--;
		}
	  // reset basetime to current clock
		uiBaseTime = GetJA2Clock( );
	}

	{ SGPVSurface::Lock l(FRAME_BUFFER);
		SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		// draw line in current state
		LineDraw(TRUE, uiCursorPosition, CURSOR_Y, uiCursorPosition, CURSOR_Y + CURSOR_HEIGHT, Get16BPPColor(GlowColorsList[iCurrentState]), l.Buffer<UINT16>());
	}

  InvalidateRegion((UINT16) uiCursorPosition , CURSOR_Y , (UINT16)uiCursorPosition + 1, CURSOR_Y + CURSOR_HEIGHT + 1);
}


static void HandleTextEvent(const InputAtom* Inp);
static void ProcessPlayerInputActivationString(void);


static void GetPlayerKeyBoardInputForIMPHomePage(void)
{
	InputAtom					InputEvent;
	while (DequeueEvent(&InputEvent))
  {
		if(	!HandleTextInput( &InputEvent ) && (InputEvent.usEvent == KEY_DOWN || InputEvent.usEvent == KEY_REPEAT || InputEvent.usEvent == KEY_UP ) )
		{
		  switch( InputEvent.usParam )
			{
				case SDLK_RETURN:
					if(( InputEvent.usEvent == KEY_UP ) )
					{
						// return hit, check to see if current player activation string is a valid one
						ProcessPlayerInputActivationString( );

					  fNewCharInActivationString = TRUE;
					}
				break;

				case SDLK_ESCAPE: HandleLapTopESCKey(); break;

				default:
					if(InputEvent.usEvent == KEY_DOWN || InputEvent.usEvent == KEY_REPEAT )
					{
						HandleTextEvent(&InputEvent);
					}
				break;
			}
		}
	}
}


static void HandleTextEvent(const InputAtom* Inp)
{
   // this function checks to see if a letter or a backspace was pressed, if so, either put char to screen
	 // or delete it

  switch (Inp->usParam)
	{
		case SDLK_BACKSPACE:
			if( iStringPos >= 0 )
			{

				if( iStringPos > 0 )
				{
					// decrement iStringPosition
					iStringPos-=1;
				}

				// null out char
        pPlayerActivationString[iStringPos ] = 0;

				// move back cursor
        uiCursorPosition = StringPixLength( pPlayerActivationString, FONT14ARIAL ) + IMP_PLAYER_ACTIVATION_STRING_X;



				// string has been altered, redisplay
        fNewCharInActivationString = TRUE;

      }

		break;

	  default:
		{
			wchar_t Char = Inp->Char;
			if (Char >= 'A' && Char <= 'Z' ||
					Char >= 'a' && Char <= 'z' ||
					Char >= '0' && Char <= '9' ||
					Char == '_' || Char == '.')
			{
				// if the current string position is at max or great, do nothing
        if( iStringPos >= 6 )
        {
					break;
				}
				else
				{
					if(iStringPos < 0 )
					{
						iStringPos = 0;
					}
          // valid char, capture and convert to wchar_t
					pPlayerActivationString[iStringPos] = Char;

					// null out next char position
					pPlayerActivationString[iStringPos + 1] = 0;

					// move cursor position ahead
          uiCursorPosition = StringPixLength( pPlayerActivationString, FONT14ARIAL ) + IMP_PLAYER_ACTIVATION_STRING_X;

					// increment string position
					iStringPos +=1;

				  // string has been altered, redisplay
          fNewCharInActivationString = TRUE;
				}
			}
			break;
		}
	}
}


static void ProcessPlayerInputActivationString(void)
{
	wchar_t const* msg;
	if (wcscmp(pPlayerActivationString, L"XEP624") != 0 &&
			wcscmp(pPlayerActivationString, L"xep624") != 0)
	{
		msg = pImpPopUpStrings[0];
	}
	else if (LaptopSaveInfo.fIMPCompletedFlag)
	{
		msg = pImpPopUpStrings[6];
	}
	else
	{
	  iCurrentImpPage = IMP_MAIN_PAGE;
	  return;
	}
	DoLapTopMessageBox(MSG_BOX_IMP_STYLE, msg, LAPTOP_SCREEN, MSG_BOX_FLAG_OK, 0);
}


static void CreateIMPHomePageButtons(void)
{
  // this procedure will create the buttons needed for the IMP homepage

	// ths about us button
	giIMPHomePageButtonImage[0] = LoadButtonImage(LAPTOPDIR "/button_1.sti", 0, 1);
	giIMPHomePageButton[0] = CreateIconAndTextButton( giIMPHomePageButtonImage[0], pImpButtonText[ 0 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 LAPTOP_SCREEN_UL_X + 286 - 106, LAPTOP_SCREEN_WEB_UL_Y + 248 - 48, MSYS_PRIORITY_HIGH,
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


static void BtnIMPAboutUsCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_ABOUT_US;
		fButtonPendingFlag = TRUE;
	}
}
