#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "IMP_Portraits.h"
#include "IMP_MainPage.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "VObject.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "Button_System.h"
#include "VSurface.h"
#include "Font_Control.h"

#include <string_theory/string>


//current and last pages
INT32 iCurrentPortrait = 0;
INT32 iLastPicture = 7;

// buttons needed for the IMP portrait screen
GUIButtonRef giIMPPortraitButton[3];
static BUTTON_PICS* giIMPPortraitButtonImage[3];

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


static void RenderPortrait(INT16 x, INT16 y);


void RenderIMPPortraits( void )
{
	// render background
	RenderProfileBackGround( );

	// the Voices frame
	RenderPortraitFrame( 191 * g_ui.m_stdScreenScale, 167 * g_ui.m_stdScreenScale );

	// render the current portrait
	RenderPortrait( 200 * g_ui.m_stdScreenScale, 176 * g_ui.m_stdScreenScale );

	// indent for the text
	RenderAttrib1IndentFrame( 128 * g_ui.m_stdScreenScale, 65 * g_ui.m_stdScreenScale);

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
	if (fReDrawPortraitScreenFlag)
	{
		RenderIMPPortraits( );

		// reset redraw flag
		fReDrawPortraitScreenFlag = FALSE;
	}
}


static void RenderPortrait(INT16 const x, INT16 const y)
{ // Render the portrait of the current picture
	INT32 const portrait = (fCharacterIsMale ? 200 : 208) + iCurrentPortrait;
	ST::string filename = ST::format(FACESDIR "/bigfaces/{}.sti", portrait);
	BltVideoObjectOnce(FRAME_BUFFER, filename.c_str(), 0, LAPTOP_SCREEN_UL_X + x, LAPTOP_SCREEN_WEB_UL_Y + y);
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


static void MakeButton(UINT idx, const char* img_file, INT32 off_normal, INT32 on_normal, const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	BUTTON_PICS* const img = LoadButtonImage(img_file, off_normal, on_normal);
	giIMPPortraitButtonImage[idx] = img;
	const UINT32 text_col   = FONT_WHITE;
	const UINT32 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, FONT12ARIAL, text_col, shadow_col, text_col, shadow_col, x, y, MSYS_PRIORITY_HIGH, click);
	giIMPPortraitButton[idx] = btn;
	btn->SetCursor(CURSOR_WWW);
}


static void BtnIMPPortraitDoneCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnIMPPortraitNextCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnIMPPortraitPreviousCallback(GUI_BUTTON* btn, UINT32 reason);


static void CreateIMPPortraitButtons(void)
{
	// will create buttons need for the IMP portrait screen
	const INT16 dx = LAPTOP_SCREEN_UL_X;
	const INT16 dy = LAPTOP_SCREEN_WEB_UL_Y;
	MakeButton(0, LAPTOPDIR "/voicearrows.sti", 1, 3, pImpButtonText[13],
			dx + 343 * g_ui.m_stdScreenScale, dy + 205 * g_ui.m_stdScreenScale, BtnIMPPortraitNextCallback);     // Next button
	MakeButton(1, LAPTOPDIR "/voicearrows.sti", 0, 2, pImpButtonText[12],
			dx +  93 * g_ui.m_stdScreenScale, dy + 205 * g_ui.m_stdScreenScale, BtnIMPPortraitPreviousCallback); // Previous button
	MakeButton(2, LAPTOPDIR "/button_5.sti",    0, 1, pImpButtonText[11],
			dx + 187 * g_ui.m_stdScreenScale, dy + 330 * g_ui.m_stdScreenScale, BtnIMPPortraitDoneCallback);     // Done button
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


static void BtnIMPPortraitNextCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		IncrementPictureIndex();
		fReDrawPortraitScreenFlag = TRUE;
	}
}


static void BtnIMPPortraitPreviousCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		DecrementPicture();
		fReDrawPortraitScreenFlag = TRUE;
	}
}


static void BtnIMPPortraitDoneCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
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
