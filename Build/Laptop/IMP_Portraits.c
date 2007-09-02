#include "CharProfile.h"
#include "Font.h"
#include "IMP_Portraits.h"
#include "IMP_MainPage.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Input.h"
#include "Isometric_Utils.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "IMP_Compile_Character.h"
#include "Button_System.h"
#include "VSurface.h"
#include "Font_Control.h"


//current and last pages
INT32 iCurrentPortrait = 0;
INT32 iLastPicture = 7;

// buttons needed for the IMP portrait screen
INT32 giIMPPortraitButton[ 3 ];
INT32 giIMPPortraitButtonImage[ 3 ];

// redraw protrait screen
BOOLEAN fReDrawPortraitScreenFlag = FALSE;

// face index
INT32 iPortraitNumber = 0;


static void CreateIMPPortraitButtons(void);


void EnterIMPPortraits( void )
{

		// create buttons
	CreateIMPPortraitButtons( );

	// render background
	RenderIMPPortraits( );
}


static BOOLEAN RenderPortrait(INT16 sX, INT16 sY);


void RenderIMPPortraits( void )
{


  // render background
	RenderProfileBackGround( );

		// the Voices frame
	RenderPortraitFrame( 191, 167 );

	// render the current portrait
	RenderPortrait( 200, 176 );

	// indent for the text
	RenderAttrib1IndentFrame( 128, 65);

	// text
	PrintImpText( );
}


static void DestroyIMPPortraitButtons(void);


void ExitIMPPortraits( void )
{
	// destroy buttons for IMP portrait page
  DestroyIMPPortraitButtons( );
}

void HandleIMPPortraits( void )
{

	// do we need to re write screen
	if ( fReDrawPortraitScreenFlag == TRUE )
	{
    RenderIMPPortraits( );

		// reset redraw flag
		fReDrawPortraitScreenFlag = FALSE;
	}
}


static BOOLEAN RenderPortrait(INT16 sX, INT16 sY)
{
  // render the portrait of the current picture
	UINT32 uiGraphicHandle;

  if( fCharacterIsMale  )
	{
	  uiGraphicHandle = AddVideoObjectFromFile(pPlayerSelectedBigFaceFileNames[iCurrentPortrait]);
	  CHECKF(uiGraphicHandle != NO_VOBJECT);
    BltVideoObjectFromIndex(FRAME_BUFFER, uiGraphicHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
    DeleteVideoObjectFromIndex( uiGraphicHandle );
	}
	else
	{
		uiGraphicHandle = AddVideoObjectFromFile(pPlayerSelectedBigFaceFileNames[iCurrentPortrait + 8]);
		CHECKF(uiGraphicHandle != NO_VOBJECT);
    BltVideoObjectFromIndex(FRAME_BUFFER, uiGraphicHandle, 0, LAPTOP_SCREEN_UL_X + sX, LAPTOP_SCREEN_WEB_UL_Y + sY);
    DeleteVideoObjectFromIndex( uiGraphicHandle );
	}

	return ( TRUE );
}


static void IncrementPictureIndex(void)
{
	// cycle to next picture

	iCurrentPortrait++;

	// gone too far?
	if( iCurrentPortrait > iLastPicture )
	{
		iCurrentPortrait = 0;
	}
}


static void DecrementPicture(void)
{
  // cycle to previous picture

	iCurrentPortrait--;

	// gone too far?
  if( iCurrentPortrait < 0 )
	{
    iCurrentPortrait = iLastPicture;
	}
}


static void BtnIMPPortraitDoneCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnIMPPortraitNextCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnIMPPortraitPreviousCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateIMPPortraitButtons(void)
{
  // will create buttons need for the IMP portrait screen

	// next button
  giIMPPortraitButtonImage[0]=  LoadButtonImage( "LAPTOP/voicearrows.sti" ,-1,1,-1,3,-1 );
	giIMPPortraitButton[0] = CreateIconAndTextButton( giIMPPortraitButtonImage[0], pImpButtonText[ 13 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 343 ), LAPTOP_SCREEN_WEB_UL_Y + ( 205 ),BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnIMPPortraitNextCallback);


	// previous button
	giIMPPortraitButtonImage[ 1 ]=  LoadButtonImage( "LAPTOP/voicearrows.sti" ,-1,0,-1,2,-1 );
  giIMPPortraitButton[ 1 ] = CreateIconAndTextButton( giIMPPortraitButtonImage[ 1 ], pImpButtonText[ 12 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														  LAPTOP_SCREEN_UL_X +  ( 93), LAPTOP_SCREEN_WEB_UL_Y + ( 205 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnIMPPortraitPreviousCallback);


	// done button
  giIMPPortraitButtonImage[ 2 ]=  LoadButtonImage( "LAPTOP/button_5.sti" ,-1,0,-1,1,-1 );
  giIMPPortraitButton[ 2 ] = CreateIconAndTextButton( giIMPPortraitButtonImage[ 2 ], pImpButtonText[ 11 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 187 ), LAPTOP_SCREEN_WEB_UL_Y + ( 330 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnIMPPortraitDoneCallback);



   SetButtonCursor(giIMPPortraitButton[0], CURSOR_WWW);
	 SetButtonCursor(giIMPPortraitButton[1], CURSOR_WWW);
	 SetButtonCursor(giIMPPortraitButton[2], CURSOR_WWW);
}


static void DestroyIMPPortraitButtons(void)
{

	// will destroy buttons created for IMP Portrait screen

	// the next button
  RemoveButton(giIMPPortraitButton[ 0 ] );
  UnloadButtonImage(giIMPPortraitButtonImage[ 0 ] );

	// the previous button
  RemoveButton(giIMPPortraitButton[ 1 ] );
  UnloadButtonImage(giIMPPortraitButtonImage[ 1 ] );

	// the done button
  RemoveButton(giIMPPortraitButton[ 2 ] );
  UnloadButtonImage(giIMPPortraitButtonImage[ 2 ] );
}


static void BtnIMPPortraitNextCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		IncrementPictureIndex();
		fReDrawPortraitScreenFlag = TRUE;
	}
}


static void BtnIMPPortraitPreviousCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		DecrementPicture();
		fReDrawPortraitScreenFlag = TRUE;
	}
}


static void BtnIMPPortraitDoneCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_MAIN_PAGE;

		// current mode now is voice
		if (iCurrentProfileMode < 4) iCurrentProfileMode = 4;

		// if we are already done, leave
		if (iCurrentProfileMode == 5) iCurrentImpPage = IMP_FINISH;

		// grab picture number
		iPortraitNumber = iCurrentPortrait + (fCharacterIsMale ? 0 : 8);

		fButtonPendingFlag = TRUE;
	}
}
