#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "Insurance_Info.h"
#include "Insurance.h"
#include "Object_Cache.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Insurance_Text.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "UILayout.h"

#include <string_theory/string>

#include <algorithm>

#define INS_INFO_FRAUD_TEXT_COLOR		FONT_MCOLOR_RED


#define INS_INFO_SUBTITLE_X			86 + LAPTOP_SCREEN_UL_X
#define INS_INFO_SUBTITLE_Y			62 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_INFO_SUBTITLE_LINE_Y		INS_INFO_SUBTITLE_Y + 14
#define INS_INFO_SUBTITLE_LINE_WIDTH		375

#define INS_INFO_FIRST_PARAGRAPH_WIDTH		INS_INFO_SUBTITLE_LINE_WIDTH
#define INS_INFO_FIRST_PARAGRAPH_X		INS_INFO_SUBTITLE_X
#define INS_INFO_FIRST_PARAGRAPH_Y		INS_INFO_SUBTITLE_LINE_Y + 9

#define INS_INFO_SPACE_BN_PARAGRAPHS		12

#define INS_INFO_INFO_TOC_TITLE_X		(STD_SCREEN_X + 170)
#define INS_INFO_INFO_TOC_TITLE_Y		54 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_INFO_TOC_SUBTITLE_X			INS_INFO_SUBTITLE_X


#define INS_INFO_LINK_TO_CONTRACT_X		235 + LAPTOP_SCREEN_UL_X
#define INS_INFO_LINK_TO_CONTRACT_Y		392 + LAPTOP_SCREEN_WEB_UL_Y
#define INS_INFO_LINK_TO_CONTRACT_WIDTH		97//107

#define INS_INFO_LINK_START_OFFSET		20//14
#define INS_INFO_LINK_START_X			(STD_SCREEN_X + 262 + INS_INFO_LINK_START_OFFSET)
#define INS_INFO_LINK_START_Y			392 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_INFO_LINK_TO_CONTRACT_TEXT_Y	355 + LAPTOP_SCREEN_WEB_UL_Y


static cache_key_t const guiBulletImage{ LAPTOPDIR "/bullet.sti" };


//The list of Info sub pages
enum
{
	INS_INFO_INFO_TOC,
	INS_INFO_SUBMIT_CLAIM,
	INS_INFO_PREMIUMS,
	INS_INFO_RENEWL,
	INS_INFO_CANCELATION,
	INS_INFO_LAST_PAGE,
};
static UINT8 gubCurrentInsInfoSubPage = 0;

static BOOLEAN InsuranceInfoSubPagesVisitedFlag[INS_INFO_LAST_PAGE];


static BUTTON_PICS* guiInsPrevButtonImage;
static void BtnInsPrevButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiInsPrevBackButton;

static BUTTON_PICS* guiInsNextButtonImage;
static void BtnInsNextButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiInsNextBackButton;


//link to the varios pages
static MOUSE_REGION gSelectedInsuranceInfoLinkRegion;

static MOUSE_REGION gSelectedInsuranceInfoHomeLinkRegion;


void EnterInitInsuranceInfo()
{
	std::fill_n(InsuranceInfoSubPagesVisitedFlag, INS_INFO_LAST_PAGE-1, 0);
}


static GUIButtonRef MakeButtonBig(BUTTON_PICS* img, const ST::string& text, INT16 x, GUI_CALLBACK click, INT8 offset_x)
{
	const INT16 text_col   = INS_FONT_COLOR;
	const INT16 shadow_col = INS_FONT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, INS_FONT_BIG, text_col, shadow_col, text_col, shadow_col, x, INS_INFO_ARROW_BUTTON_Y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_WWW);
	btn->SpecifyTextOffsets(offset_x, 16, FALSE);
	return btn;
}


static void SelectInsuranceInfoHomeLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectInsuranceLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterInsuranceInfo()
{
	UINT16					usPosX;

	InitInsuranceDefaults();

	//left arrow
	guiInsPrevButtonImage	= LoadButtonImage(LAPTOPDIR "/insleftbutton.sti", 2, 0, -1, 1, -1);
	guiInsPrevBackButton  = MakeButtonBig(guiInsPrevButtonImage, InsInfoText[INS_INFO_PREVIOUS], INS_INFO_LEFT_ARROW_BUTTON_X, BtnInsPrevButtonCallback, 17);

	//Right arrow
	guiInsNextButtonImage	= LoadButtonImage(LAPTOPDIR "/insrightbutton.sti", 2, 0, -1, 1, -1);
	guiInsNextBackButton  = MakeButtonBig(guiInsNextButtonImage, InsInfoText[INS_INFO_NEXT], INS_INFO_RIGHT_ARROW_BUTTON_X, BtnInsNextButtonCallback, 18);

	usPosX = INS_INFO_LINK_START_X;
	//link to go to the contract page
	//link to go to the home page
	MSYS_DefineRegion(&gSelectedInsuranceInfoHomeLinkRegion, usPosX, INS_INFO_LINK_TO_CONTRACT_Y-37,
				(UINT16)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH), INS_INFO_LINK_TO_CONTRACT_Y+2,
				MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
				SelectInsuranceInfoHomeLinkRegionCallBack);

	usPosX += INS_INFO_LINK_START_OFFSET + INS_INFO_LINK_TO_CONTRACT_WIDTH;
	MSYS_DefineRegion(&gSelectedInsuranceInfoLinkRegion, usPosX, INS_INFO_LINK_TO_CONTRACT_Y-37,
				(UINT16)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH), INS_INFO_LINK_TO_CONTRACT_Y+2,
				MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceLinkRegionCallBack);

	gubCurrentInsInfoSubPage = INS_INFO_INFO_TOC;

	RenderInsuranceInfo();
}


void ExitInsuranceInfo()
{
	RemoveInsuranceDefaults();


	UnloadButtonImage( guiInsPrevButtonImage );
	RemoveButton( guiInsPrevBackButton );

	UnloadButtonImage( guiInsNextButtonImage );
	RemoveButton( guiInsNextBackButton );

	MSYS_RemoveRegion( &gSelectedInsuranceInfoLinkRegion);
	MSYS_RemoveRegion( &gSelectedInsuranceInfoHomeLinkRegion);

	RemoveVObject(guiBulletImage);
}


static void DisableArrowButtonsIfOnLastOrFirstPage(void);
static void DisplayCancelationPagePage(void);
static void DisplayInfoTocPage(void);
static void DisplayPremiumPage(void);
static void DisplayRenewingPremiumPage(void);
static void DisplaySubmitClaimPage(void);


void RenderInsuranceInfo()
{
	ST::string sText;
	UINT16		usPosX;

	DisableArrowButtonsIfOnLastOrFirstPage();

	DisplayInsuranceDefaults();

	SetFontShadow( INS_FONT_SHADOW );


	//Display the red bar under the link at the bottom
	DisplaySmallRedLineWithShadow( INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_LINE_Y, INS_INFO_SUBTITLE_X+INS_INFO_SUBTITLE_LINE_WIDTH, INS_INFO_SUBTITLE_LINE_Y);

	switch( gubCurrentInsInfoSubPage )
	{
		case INS_INFO_INFO_TOC:
			DisplayInfoTocPage();
			break;

		case INS_INFO_SUBMIT_CLAIM:
			DisplaySubmitClaimPage();
			break;

		case INS_INFO_PREMIUMS:
			DisplayPremiumPage();
			break;

		case INS_INFO_RENEWL:
			DisplayRenewingPremiumPage();
			break;

		case INS_INFO_CANCELATION:
			DisplayCancelationPagePage();
			break;
	}

	usPosX = INS_INFO_LINK_START_X;

	//Display the red bar under the link at the bottom.  and the text
	DisplaySmallRedLineWithShadow( usPosX, INS_INFO_LINK_TO_CONTRACT_Y, (UINT16)(usPosX+INS_INFO_LINK_TO_CONTRACT_WIDTH), INS_INFO_LINK_TO_CONTRACT_Y);
	DisplayWrappedString(usPosX, INS_INFO_LINK_TO_CONTRACT_TEXT_Y + 14, INS_INFO_LINK_TO_CONTRACT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, pMessageStrings[MSG_HOMEPAGE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	usPosX += INS_INFO_LINK_START_OFFSET + INS_INFO_LINK_TO_CONTRACT_WIDTH;

	//Display the red bar under the link at the bottom.  and the text
	DisplaySmallRedLineWithShadow( usPosX, INS_INFO_LINK_TO_CONTRACT_Y, (UINT16)(usPosX+INS_INFO_LINK_TO_CONTRACT_WIDTH), INS_INFO_LINK_TO_CONTRACT_Y);
	sText = GetInsuranceText(INS_SNGL_TO_ENTER_REVIEW);
	DisplayWrappedString(usPosX, INS_INFO_LINK_TO_CONTRACT_TEXT_Y, INS_INFO_LINK_TO_CONTRACT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	SetFontShadow(DEFAULT_SHADOW);

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void ChangingInsuranceInfoSubPage(UINT8 ubSubPageNumber);


static void BtnInsPrevButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (gubCurrentInsInfoSubPage > 0) gubCurrentInsInfoSubPage--;
		ChangingInsuranceInfoSubPage(gubCurrentInsInfoSubPage);
		//fPausedReDrawScreenFlag = TRUE;
	}
}


static void BtnInsNextButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (gubCurrentInsInfoSubPage < INS_INFO_LAST_PAGE - 1) gubCurrentInsInfoSubPage++;
		ChangingInsuranceInfoSubPage(gubCurrentInsInfoSubPage);
		//fPausedReDrawScreenFlag = TRUE;
	}
}


static void SelectInsuranceLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_CONTRACT;
	}
}


static void SelectInsuranceInfoHomeLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
	}
}


static void DisplaySubmitClaimPage(void)
{
	ST::string sText;
	UINT16 usNewLineOffset = 0;
	UINT16	usPosX;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_SUBMITTING_CLAIM);
	DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the title slogan
	sText = GetInsuranceText(INS_MLTI_U_CAN_REST_ASSURED);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	sText = GetInsuranceText(INS_MLTI_HAD_U_HIRED_AN_INDIVIDUAL);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	//display the BIG FRAUD
	sText = GetInsuranceText(INS_SNGL_FRAUD);
	DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset - 1, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_BIG, INS_INFO_FRAUD_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosX = INS_INFO_FIRST_PARAGRAPH_X + StringPixLength( sText, INS_FONT_BIG ) + 2;
	sText = GetInsuranceText(INS_MLTI_WE_RESERVE_THE_RIGHT);
	usNewLineOffset += DisplayWrappedString(usPosX, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	sText = GetInsuranceText(INS_MLTI_SHOULD_THERE_BE_GROUNDS);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	sText = GetInsuranceText(INS_MLTI_SHOULD_SUCH_A_SITUATION);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

}


static void DisplayPremiumPage(void)
{
	ST::string sText;
	UINT16 usNewLineOffset = 0;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;


	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_PREMIUMS);
	DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	sText = GetInsuranceText(INS_MLTI_EACH_TIME_U_COME_TO_US);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	BltVideoObject(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	sText = GetInsuranceText(INS_MLTI_LENGTH_OF_EMPLOYMENT_CONTRACT);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	BltVideoObject(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	sText = GetInsuranceText(INS_MLTI_EMPLOYEES_AGE_AND_HEALTH);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	BltVideoObject(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	sText = GetInsuranceText(INS_MLTI_EMPLOOYEES_TRAINING_AND_EXP);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}


static void DisplayRenewingPremiumPage(void)
{
	ST::string sText;
	UINT16 usNewLineOffset = 0;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_RENEWL_PREMIUMS);
	DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	sText = GetInsuranceText(INS_MLTI_WHEN_IT_COMES_TIME_TO_RENEW);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	sText = GetInsuranceText(INS_MLTI_SHOULD_THE_PROJECT_BE_GOING_WELL);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	//display the LOWER PREMIUM FOR RENWING EARLY
	sText = GetInsuranceText(INS_SNGL_LOWER_PREMIUMS_4_RENEWING);
	DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset - 1, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_BIG, INS_INFO_FRAUD_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS + 2;
}


static void DisplayCancelationPagePage(void)
{
	ST::string sText;
	UINT16 usNewLineOffset = 0;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_POLICY_CANCELATIONS);
	DrawTextToScreen(sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	sText = GetInsuranceText(INS_MLTI_WE_WILL_ACCEPT_INS_CANCELATION);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	sText = GetInsuranceText(INS_MLTI_1_HOUR_EXCLUSION_A);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	sText = GetInsuranceText(INS_MLTI_1_HOUR_EXCLUSION_B);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}


static void DisableArrowButtonsIfOnLastOrFirstPage(void)
{
	EnableButton(guiInsPrevBackButton, gubCurrentInsInfoSubPage != INS_INFO_INFO_TOC);
	EnableButton(guiInsNextBackButton, gubCurrentInsInfoSubPage != INS_INFO_LAST_PAGE - 1);
}


static void ChangingInsuranceInfoSubPage(UINT8 ubSubPageNumber)
{
	fLoadPendingFlag = TRUE;

	if (!InsuranceInfoSubPagesVisitedFlag[ubSubPageNumber])
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = FALSE;

		InsuranceInfoSubPagesVisitedFlag[ ubSubPageNumber ] = TRUE;
	}
	else
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = TRUE;
	}
}


static void DisplayInfoTocPage(void)
{
	ST::string sText;
	UINT16 usNewLineOffset = 0;
	UINT16		usPosY;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK);
	DrawTextToScreen(sText, INS_INFO_INFO_TOC_TITLE_X, INS_INFO_INFO_TOC_TITLE_Y, 439, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display the First paragraph
	sText = GetInsuranceText(INS_MLTI_HIRING_4_SHORT_TERM_HIGH_RISK_1);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	//Display the 2nd paragraph
	sText = GetInsuranceText(INS_MLTI_HIRING_4_SHORT_TERM_HIGH_RISK_2);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;


	//Display the sub title
	sText = GetInsuranceText(INS_SNGL_WE_CAN_OFFER_U);
	DrawTextToScreen(sText, INS_INFO_TOC_SUBTITLE_X, usNewLineOffset, SCREEN_WIDTH - INS_INFO_INFO_TOC_TITLE_X, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY = usNewLineOffset + 12;
	DisplaySmallRedLineWithShadow( INS_INFO_SUBTITLE_X, usPosY, (UINT16)(INS_INFO_SUBTITLE_X+INS_INFO_SUBTITLE_LINE_WIDTH), usPosY );

	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;


	//
	// Premiuns bulleted sentence
	//

	BltVideoObject(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	sText = GetInsuranceText(INS_MLTI_REASONABLE_AND_FLEXIBLE);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;



	//
	// Quick and efficient claims
	//

	BltVideoObject(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	sText = GetInsuranceText(INS_MLTI_QUICKLY_AND_EFFICIENT);
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}
