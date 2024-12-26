#include "CharProfile.h"
#include "ContentManager.h"
#include "Directories.h"
#include "Font.h"
#include "GamePolicy.h"
#include "GameInstance.h"
#include "IMP_Personality_Entrance.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "Cursors.h"
#include "Laptop.h"
#include "Button_System.h"
#include "Font_Control.h"


// IMP personality entrance buttons
static BUTTON_PICS* giIMPPersonalityEntranceButtonImage[1];
static GUIButtonRef giIMPPersonalityEntranceButton[1];


static void BtnIMPPersonalityEntranceDoneCallback(GUI_BUTTON *btn, UINT32 reason);


static void CreateIMPPersonalityEntranceButtons(void);


void EnterIMPPersonalityEntrance( void )
{
	// create buttons needed
	CreateIMPPersonalityEntranceButtons( );
}

void RenderIMPPersonalityEntrance( void )
{
	// the background
	RenderProfileBackGround( );

	// the IMP symbol
	//RenderIMPSymbol( 112, 30 );

	// indent
	RenderAvgMercIndentFrame(90, 40 );
}


static void DestroyIMPPersonalityEntranceButtons(void);


void ExitIMPPersonalityEntrance( void )
{
	// destroy buttons needed
	DestroyIMPPersonalityEntranceButtons( );
}


void HandleIMPPersonalityEntrance( void )
{
}


static void CreateIMPPersonalityEntranceButtons(void)
{
	// this function will create the buttons needed for the IMP personality Page

	// ths begin button
	giIMPPersonalityEntranceButtonImage[0] = LoadButtonImage(LAPTOPDIR "/button_2.sti", 0, 1);
		giIMPPersonalityEntranceButton[0] = CreateIconAndTextButton(
							giIMPPersonalityEntranceButtonImage[0],
							pImpButtonText[ 1 ], FONT12ARIAL,
							FONT_WHITE, DEFAULT_SHADOW,
							FONT_WHITE, DEFAULT_SHADOW,
							LAPTOP_SCREEN_UL_X + 136, LAPTOP_SCREEN_WEB_UL_Y + 314,
							MSYS_PRIORITY_HIGH,
							BtnIMPPersonalityEntranceDoneCallback);

	giIMPPersonalityEntranceButton[0]->SetCursor(CURSOR_WWW);
}


static void DestroyIMPPersonalityEntranceButtons(void)
{
	// this function will destroy the buttons needed for the IMP personality page

	// the begin button
	RemoveButton(giIMPPersonalityEntranceButton[0] );
	UnloadButtonImage(giIMPPersonalityEntranceButtonImage[0] );
}


static void BtnIMPPersonalityEntranceDoneCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for IMP Begin Screen done button

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// done with begin screen, next screen
		iCurrentImpPage = gamepolicy(imp_pick_skills_directly) ? IMP_SKILLTRAITS : IMP_PERSONALITY_QUIZ;
		fButtonPendingFlag = true;
	}
}
