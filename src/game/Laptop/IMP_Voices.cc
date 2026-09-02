#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "IMP_Compile_Character.h"
#include "IMP_Voices.h"
#include "IMP_MainPage.h"
#include "IMPVideoObjects.h"
#include "Cursors.h"
#include "Laptop.h"
#include "Sound_Control.h"
#include "IMP_Text_System.h"
#include "Text.h"
#include "LaptopSave.h"
#include "Button_System.h"
#include "SoundMan.h"
#include "Font_Control.h"


#include <string_theory/format>
#include <string_theory/string>


// which of the voices offered for this gender is on show
INT32 iCurrentVoices = 0;


static UINT32 uiVocVoiceSound = 0;
// buttons needed for the IMP Voices screen
static GUIButtonRef giIMPVoicesButton[3];
static BUTTON_PICS* giIMPVoicesButtonImage[3];


// redraw protrait screen
static BOOLEAN fReDrawVoicesScreenFlag = FALSE;

// the portrait region, for player to click on and re-hear voice
static MOUSE_REGION gVoicePortraitRegion;


static void CreateIMPVoiceMouseRegions(void);
static void CreateIMPVoicesButtons(void);
static void PlayVoice();


void EnterIMPVoices( void )
{
	// the data may offer a different number of voices per gender, so an index
	// left over from profiling the other one can be past the end
	if (iCurrentVoices >= GetNumberOfIMPVoices(fCharacterIsMale)) iCurrentVoices = 0;

	// create buttons
	CreateIMPVoicesButtons( );

	// create mouse regions
	CreateIMPVoiceMouseRegions( );

	// render background
	RenderIMPVoices( );

	// play voice once
	PlayVoice();
}


static void RenderVoiceIndex(void);


void RenderIMPVoices( void )
{
	// render background
	RenderProfileBackGround( );

	// the Voices frame
	RenderPortraitFrame( 191, 167 );

	// the sillouette
	RenderLargeSilhouette( 200, 176 );

	// indent for the text
	RenderAttrib1IndentFrame( 128, 65);

	// render voice index value
	RenderVoiceIndex( );

	// text
	PrintImpText( );
}


static void DestroyIMPVoiceMouseRegions(void);
static void DestroyIMPVoicesButtons(void);


void ExitIMPVoices( void )
{
	// destroy buttons for IMP Voices page
	DestroyIMPVoicesButtons( );

	// destroy mouse regions for this screen
	DestroyIMPVoiceMouseRegions( );
}

void HandleIMPVoices( void )
{
	// do we need to re write screen
	if (fReDrawVoicesScreenFlag)
	{
		RenderIMPVoices( );

		// reset redraw flag
		fReDrawVoicesScreenFlag = FALSE;
	}
}


static void IncrementVoice(void)
{
	iCurrentVoices++;

	// gone too far?
	if( iCurrentVoices >= GetNumberOfIMPVoices(fCharacterIsMale) )
	{
		iCurrentVoices = 0;
	}
}


static void DecrementVoice(void)
{
	iCurrentVoices--;

	// gone too far?
	if( iCurrentVoices < 0 )
	{
		iCurrentVoices = GetNumberOfIMPVoices(fCharacterIsMale) - 1;
	}
}


static void MakeButton(UINT idx, const char* img_file, INT32 off_normal, INT32 on_normal, const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	BUTTON_PICS* const img = LoadButtonImage(img_file, off_normal, on_normal);
	giIMPVoicesButtonImage[idx] = img;
	const INT16 text_col   = FONT_WHITE;
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, FONT12ARIAL, text_col, shadow_col, text_col, shadow_col, x, y, MSYS_PRIORITY_HIGH, click);
	giIMPVoicesButton[idx] = btn;
	btn->SetCursor(CURSOR_WWW);
}


static void BtnIMPVoicesDoneCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnIMPVoicesNextCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnIMPVoicesPreviousCallback(GUI_BUTTON* btn, UINT32 reason);


static void CreateIMPVoicesButtons(void)
{
	// will create buttons need for the IMP Voices screen
	const INT16 dx = LAPTOP_SCREEN_UL_X;
	const INT16 dy = LAPTOP_SCREEN_WEB_UL_Y;
	MakeButton(0, LAPTOPDIR "/voicearrows.sti", 1, 3, pImpButtonText[13], dx + 343, dy + 205, BtnIMPVoicesNextCallback);     // Next button
	MakeButton(1, LAPTOPDIR "/voicearrows.sti", 0, 2, pImpButtonText[12], dx +  93, dy + 205, BtnIMPVoicesPreviousCallback); // Previous button
	MakeButton(2, LAPTOPDIR "/button_5.sti",    0, 1, pImpButtonText[11], dx + 187, dy + 330, BtnIMPVoicesDoneCallback);     // Done button
}


static void DestroyIMPVoicesButtons(void)
{

	// will destroy buttons created for IMP Voices screen

	// the next button
	RemoveButton(giIMPVoicesButton[ 0 ] );
	UnloadButtonImage(giIMPVoicesButtonImage[ 0 ] );

	// the previous button
	RemoveButton(giIMPVoicesButton[ 1 ] );
	UnloadButtonImage(giIMPVoicesButtonImage[ 1 ] );

	// the done button
	RemoveButton(giIMPVoicesButton[ 2 ] );
	UnloadButtonImage(giIMPVoicesButtonImage[ 2 ] );
}


static void BtnIMPVoicesNextCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		IncrementVoice();
		if (SoundIsPlaying(uiVocVoiceSound)) SoundStop(uiVocVoiceSound);
		PlayVoice();
		fReDrawVoicesScreenFlag = TRUE;
	}
}


static void BtnIMPVoicesPreviousCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		DecrementVoice();
		if (SoundIsPlaying(uiVocVoiceSound)) SoundStop(uiVocVoiceSound);
		PlayVoice();
		fReDrawVoicesScreenFlag = TRUE;
	}
}


static void BtnIMPVoicesDoneCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iCurrentImpPage = IMP_MAIN_PAGE;

		// if we are already done, leave
		if (iCurrentProfileMode == 5)
		{
			iCurrentImpPage = IMP_FINISH;
		}
		else if (iCurrentProfileMode < 4)
		{
			// current mode now is voice
			iCurrentProfileMode = 4;
		}
		else if (iCurrentProfileMode == 4)
		{
			// all done profiling
			iCurrentProfileMode = 5;
			iCurrentImpPage = IMP_FINISH;
		}

		// the voice the character will speak with, counted over both genders
		LaptopSaveInfo.iVoiceId = GetIMPVoiceIndex(fCharacterIsMale, iCurrentVoices);

		// set button up image pending
		fButtonPendingFlag = TRUE;
	}
}


static void PlayVoice()
{
	INT32 const iVoice = GetIMPVoiceIndex(fCharacterIsMale, iCurrentVoices);
	if (iVoice < 0) return;

	// the first quote of the voice's own speech files
	ST::string filename = ST::format(SPEECHDIR "/{03d}_001.wav", GetIMPVoices()[iVoice].profile);
	uiVocVoiceSound = PlayJA2SampleFromFile(filename.c_str(), MIDVOLUME, 1, MIDDLEPAN);
}


static void IMPPortraitRegionButtonCallback(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateIMPVoiceMouseRegions(void)
{
	// will create mouse regions needed for the IMP voices page
	MSYS_DefineRegion( &gVoicePortraitRegion, LAPTOP_SCREEN_UL_X + 200, LAPTOP_SCREEN_WEB_UL_Y + 176 ,
				LAPTOP_SCREEN_UL_X + 200 + 100, LAPTOP_SCREEN_WEB_UL_Y + 176 + 100,MSYS_PRIORITY_HIGH,
				MSYS_NO_CURSOR, MSYS_NO_CALLBACK, IMPPortraitRegionButtonCallback );
}


static void DestroyIMPVoiceMouseRegions(void)
{
	// will destroy already created mouse reiogns for IMP voices page
	MSYS_RemoveRegion( &gVoicePortraitRegion );
}


static void IMPPortraitRegionButtonCallback(MOUSE_REGION* pRegion, UINT32 iReason)
{
	// callback handler for imp portrait region button events
	if(iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( ! SoundIsPlaying( uiVocVoiceSound ) )
		{
			PlayVoice();
		}
	}
}


static void RenderVoiceIndex(void)
{
	// render the voice index value on the the blank portrait
	SetFontAttributes(FONT12ARIAL, FONT_WHITE);
	MPrint(290 + LAPTOP_UL_X, 320,
		ST::format("{} {}", pIMPVoicesStrings, iCurrentVoices + 1),
		CenterAlign(100));
}
