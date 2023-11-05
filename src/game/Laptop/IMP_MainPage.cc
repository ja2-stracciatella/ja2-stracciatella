#include "Directories.h"
#include "Font.h"
#include "IMP_MainPage.h"
#include "CharProfile.h"
#include "IMPVideoObjects.h"
#include "MessageBoxScreen.h"
#include "Overhead.h"
#include "Text.h"
#include "VObject.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Attribute_Selection.h"
#include "IMP_Finish.h"
#include "MouseSystem.h"
#include "LaptopSave.h"
#include "IMP_Portraits.h"
#include "Button_System.h"
#include "ScreenIDs.h"
#include "Font_Control.h"
#include "GamePolicy.h"
#include "GameInstance.h"
#include "ContentManager.h"


#define MAIN_PAGE_BUTTON_TEXT_WIDTH 95

// main page buttons
static BUTTON_PICS* giIMPMainPageButtonImage[6];
GUIButtonRef giIMPMainPageButton[6];

extern INT32 iCurrentVoices;

// mouse regions for not entablable warning
static MOUSE_REGION pIMPMainPageMouseRegions[4];

static SGPVObject* guiCHARACTERPORTRAITFORMAINPAGE;

void BtnIMPMainPagePortraitCallback(GUI_BUTTON *btn, UINT32 reason);
void BtnIMPMainPageVoiceCallback(GUI_BUTTON *btn, UINT32 reason);


// this is the current state of profiling the player is in.
	/*
	0 - Beginning
	1 - Personnality
	2 - Attributes and Skills
	3 - Portrait
	4 - Voice
	5 - Done
	*/
INT32 iCurrentProfileMode = 0;


static void CreateIMPMainPageButtons(void);
static void CreateMouseRegionsForIMPMainPageBasedOnCharGenStatus(void);
static void LoadCharacterPortraitForMainPage(void);
static void UpDateIMPMainPageButtons(void);


void EnterIMPMainPage( void )
{

	// turn off review mode
	fReviewStats = FALSE;


	// create buttons
	CreateIMPMainPageButtons( );

	// load portrait for face button, if applicable
	LoadCharacterPortraitForMainPage( );

	// create button masks for this screen
	CreateMouseRegionsForIMPMainPageBasedOnCharGenStatus( );

	// alter states
	UpDateIMPMainPageButtons( );

	// entry into IMP about us page
	RenderIMPMainPage( );
}


static void DeleteIMPMainPageButtons(void);
static void DestoryMouseRegionsForIMPMainPageBasedOnCharGenStatus(void);


void ExitIMPMainPage( void )
{
	// exit from IMP About us page

	// delete Buttons
	DeleteIMPMainPageButtons( );
	DestoryMouseRegionsForIMPMainPageBasedOnCharGenStatus( );
}


void RenderIMPMainPage( void )
{
	// renders the IMP about us page

	// the background
	RenderProfileBackGround( );

	// the IMP symbol
	//RenderIMPSymbol( 106, 1 );
	// indent
	RenderMainIndentFrame( 164 * g_ui.m_stdScreenScale, 74 * g_ui.m_stdScreenScale );
}


static BOOLEAN CheckIfFinishedCharacterGeneration(void);


void HandleIMPMainPage( void )
{
	// handles the IMP about main page

	if ( CheckIfFinishedCharacterGeneration( ) )
	{
		iCurrentImpPage = IMP_FINISH;
	}
}


static void MakeButton(UINT idx, const char* img_file, const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	BUTTON_PICS* const img = LoadButtonImage(img_file, 0, 1);
	giIMPMainPageButtonImage[idx] = img;
	const UINT32 text_col   = FONT_WHITE;
	const UINT32 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, FONT12ARIAL, text_col, shadow_col, text_col, shadow_col, x, y, MSYS_PRIORITY_HIGH, click);
	giIMPMainPageButton[idx] = btn;
	btn->SetCursor(CURSOR_WWW);
	if (idx >= 2)
	{
		btn->SpecifyTextOffsets(10 * g_ui.m_stdScreenScale, 40 * g_ui.m_stdScreenScale, TRUE);
		btn->SpecifyTextWrappedWidth(MAIN_PAGE_BUTTON_TEXT_WIDTH);
	}
}


static void BtnIMPMainPageAttributesCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnIMPMainPageBackCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnIMPMainPageBeginCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnIMPMainPagePersonalityCallback(GUI_BUTTON* btn, UINT32 reason);


static void CreateIMPMainPageButtons(void)
{
	// this function will create the buttons needed for th IMP about us page
	const INT16 dx = LAPTOP_SCREEN_UL_X;
	const INT16 dy = LAPTOP_SCREEN_WEB_UL_Y;

	// the back button button
	MakeButton(0, LAPTOPDIR "/button_3.sti", pImpButtonText[19], dx + 15 * g_ui.m_stdScreenScale, dy + 360 * g_ui.m_stdScreenScale, BtnIMPMainPageBackCallback);
	giIMPMainPageButton[0]->SpecifyTextSubOffsets(0, -1, FALSE);

	// the begin profiling button
	ST::string profiling_text = (iCurrentProfileMode == 0 || iCurrentProfileMode > 2 ? pImpButtonText[1] : pImpButtonText[22]);
	MakeButton(1, LAPTOPDIR "/button_2.sti", profiling_text, dx + 136 * g_ui.m_stdScreenScale, dy + 174 * g_ui.m_stdScreenScale, BtnIMPMainPageBeginCallback);

	// the personality/specialties button
	ST::string btnText = gamepolicy(imp_pick_skills_directly) ? pImpButtonText[26] : pImpButtonText[2];
	MakeButton(2, LAPTOPDIR "/button_8.sti", btnText, dx + 13 * g_ui.m_stdScreenScale, dy + 245 * g_ui.m_stdScreenScale, BtnIMPMainPagePersonalityCallback);

	// the attribs button
	MakeButton(3, LAPTOPDIR "/button_8.sti", pImpButtonText[3], dx + 133 * g_ui.m_stdScreenScale, dy + 245 * g_ui.m_stdScreenScale, BtnIMPMainPageAttributesCallback);

	// the portrait button
	MakeButton(4, LAPTOPDIR "/button_8.sti", pImpButtonText[4], dx + 253 * g_ui.m_stdScreenScale, dy + 245 * g_ui.m_stdScreenScale, BtnIMPMainPagePortraitCallback);

	// the voice button
	ST::string sString;
	if (iCurrentProfileMode == 5)
	{
		sString = st_format_printf(pImpButtonText[5], iCurrentVoices + 1);
	}
	else
	{
		sString = pImpButtonText[25];
	}
	MakeButton(5, LAPTOPDIR "/button_8.sti", sString, dx + 373 * g_ui.m_stdScreenScale, dy + 245 * g_ui.m_stdScreenScale, BtnIMPMainPageVoiceCallback);
}


static void DeleteIMPMainPageButtons(void)
{
	// this function destroys the buttons needed for the IMP about Us Page

	// the back  button
	RemoveButton(giIMPMainPageButton[0] );
	UnloadButtonImage(giIMPMainPageButtonImage[0] );

	// begin profiling button
	RemoveButton(giIMPMainPageButton[1] );
	UnloadButtonImage(giIMPMainPageButtonImage[1] );

	// begin personna button
	RemoveButton(giIMPMainPageButton[2] );
	UnloadButtonImage(giIMPMainPageButtonImage[2] );

	// begin attribs button
	RemoveButton(giIMPMainPageButton[3] );
	UnloadButtonImage(giIMPMainPageButtonImage[3] );

	// begin portrait button
	RemoveButton(giIMPMainPageButton[4] );
	UnloadButtonImage(giIMPMainPageButtonImage[4] );

	// begin voice button
	RemoveButton(giIMPMainPageButton[5] );
	UnloadButtonImage(giIMPMainPageButtonImage[5] );
}


static void BtnIMPMainPageBackCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for IMP Homepage About US button

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iCurrentImpPage = IMP_HOME_PAGE;
		fButtonPendingFlag = TRUE;
		iCurrentProfileMode = 0;
		fFinishedCharGeneration = FALSE;
		ResetCharacterStats();
	}
}


static void BeginMessageBoxCallBack(MessageBoxReturnValue);


static void BtnIMPMainPageBeginCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for Main Page Begin Profiling

	// too far along to change gender

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// are we going to change name, or do we have to start over from scratch
		if (iCurrentProfileMode > 2)
		{
			// too far along, restart
			DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[1], LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO, BeginMessageBoxCallBack);
		}
		else
		{
			if (LaptopSaveInfo.iCurrentBalance < COST_OF_PROFILE)
			{
				DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[3], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, BeginMessageBoxCallBack);
			}
			else if (NumberOfMercsOnPlayerTeam() >= 18)
			{
				DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[5], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, BeginMessageBoxCallBack);
			}
			else
			{
				// change name
				iCurrentImpPage = IMP_BEGIN;
				fButtonPendingFlag = TRUE;
			}
		}
	}
}


static void BtnIMPMainPagePersonalityCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for Main Page Begin Profiling

	// if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode != 1)
	{
		btn->uiFlags &= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iCurrentImpPage = IMP_PERSONALITY;
		fButtonPendingFlag = TRUE;
	}
}


static void BtnIMPMainPageAttributesCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for Main Page Begin Profiling

	// if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode < 2)
	{
		btn->uiFlags &= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iCurrentImpPage = IMP_ATTRIBUTE_ENTRANCE;
		fButtonPendingFlag = TRUE;
	}
}


void BtnIMPMainPagePortraitCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for Main Page Begin Profiling

	// if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode != 3 && iCurrentProfileMode != 4 && iCurrentProfileMode > 5)
	{
		btn->uiFlags&= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iCurrentImpPage = IMP_PORTRAIT;
		fButtonPendingFlag = TRUE;
	}
}


void BtnIMPMainPageVoiceCallback(GUI_BUTTON *btn, UINT32 reason)
{
	// btn callback for Main Page Begin Profiling

	// if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode != 4 && iCurrentProfileMode > 5)
	{
		btn->uiFlags&= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		iCurrentImpPage = IMP_VOICE;
		fButtonPendingFlag = TRUE;
	}
}


static BOOLEAN CheckIfFinishedCharacterGeneration(void)
{
	// this function checks to see if character is done character generation

	// are we done character generation
	if( iCurrentProfileMode == 5 )
	{
		// yes
		return ( TRUE );
	}
	else
	{
		// no
		return ( FALSE );
	}
}


static void UpDateIMPMainPageButtons(void)
{
	// update mainpage button states
	INT32 iCount = 0;

	// disable all
	for( iCount = 2; iCount < 6; iCount++)
	{
		DisableButton( giIMPMainPageButton[ iCount ] );
	}

	for( iCount = 0; iCount < 4; iCount++ )
	{
		pIMPMainPageMouseRegions[iCount].Disable();
	}
	// enable
	switch(  iCurrentProfileMode )
	{
		case 0:
			pIMPMainPageMouseRegions[0].Enable();
			pIMPMainPageMouseRegions[1].Enable();
			pIMPMainPageMouseRegions[2].Enable();
			pIMPMainPageMouseRegions[3].Enable();
			break;
		case( 1 ):
			EnableButton( giIMPMainPageButton[2] );
			pIMPMainPageMouseRegions[1].Enable();
			pIMPMainPageMouseRegions[2].Enable();
			pIMPMainPageMouseRegions[3].Enable();
			break;
		case( 2 ):
			EnableButton( giIMPMainPageButton[3] );
			pIMPMainPageMouseRegions[0].Enable();
			pIMPMainPageMouseRegions[2].Enable();
			pIMPMainPageMouseRegions[3].Enable();
			break;
		case( 3 ):
			EnableButton( giIMPMainPageButton[3] );
			EnableButton( giIMPMainPageButton[4] );
			pIMPMainPageMouseRegions[0].Enable();
			//pIMPMainPageMouseRegions[1].Enable();
			pIMPMainPageMouseRegions[3].Enable();
			break;
		case( 4 ):
			//pIMPMainPageMouseRegions[1].Enable();
			pIMPMainPageMouseRegions[0].Enable();
			EnableButton( giIMPMainPageButton[3] );
			EnableButton( giIMPMainPageButton[4] );
			EnableButton( giIMPMainPageButton[5] );
			break;
	}
}


static void BeginMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, so start over, else stay here and do nothing for now
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		iCurrentImpPage = IMP_BEGIN;
		iCurrentProfileMode = 0;
	}

	else if( bExitValue == MSG_BOX_RETURN_OK )
	{
		// if ok, then we are coming from financial warning, allow continue
	}
}


static void IMPMainPageNotSelectableBtnCallback(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateMouseRegionsForIMPMainPageBasedOnCharGenStatus(void)
{
	// this procedure will create masks for the char generation main page
	// create masks for the personality, attrib, portrait and page buttons on the
	// character generation main page
	UINT16       x = LAPTOP_SCREEN_UL_X     +  13 * g_ui.m_stdScreenScale;
	UINT16 const y = LAPTOP_SCREEN_WEB_UL_Y + 245 * g_ui.m_stdScreenScale;
	UINT16 const w = 115 * g_ui.m_stdScreenScale;
	UINT16 const h =  93 * g_ui.m_stdScreenScale;
	FOR_EACHX(MOUSE_REGION, r, pIMPMainPageMouseRegions, x += 120 * g_ui.m_stdScreenScale)
	{
		MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGH + 5, CURSOR_WWW, MSYS_NO_CALLBACK, IMPMainPageNotSelectableBtnCallback);
	}
}


static void DestoryMouseRegionsForIMPMainPageBasedOnCharGenStatus(void)
{
	// will destroy button masks for the char gen pages
	FOR_EACH(MOUSE_REGION, r, pIMPMainPageMouseRegions)
	{
		MSYS_RemoveRegion(r);
	}
}


static void IMPMainPageNotSelectableBtnCallback(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if(iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		DoLapTopMessageBox( MSG_BOX_IMP_STYLE, pImpPopUpStrings[ 4 ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, BeginMessageBoxCallBack);
	}
}


SGPVObject* LoadIMPPortait()
{
	ST::string filename = ST::format(FACESDIR "/{}.sti", 200 + iPortraitNumber);
	return AddVideoObjectFromFile(filename);
}


static void LoadCharacterPortraitForMainPage(void)
{
	// this function will load the character's portrait, to be used on portrait button
	if( iCurrentProfileMode >= 4 )
	{
		guiCHARACTERPORTRAITFORMAINPAGE = LoadIMPPortait();
		giIMPMainPageButton[4]->SpecifyIcon(guiCHARACTERPORTRAITFORMAINPAGE, 0, 33 * g_ui.m_stdScreenScale, 23 * g_ui.m_stdScreenScale, FALSE);
	}
}
