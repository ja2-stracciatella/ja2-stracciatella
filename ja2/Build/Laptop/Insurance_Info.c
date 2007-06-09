#include "Font.h"
#include "Laptop.h"
#include "Insurance_Info.h"
#include "Insurance.h"
#include "WCheck.h"
#include "Utilities.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Insurance_Text.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"


#define		INS_INFO_FRAUD_TEXT_COLOR					FONT_MCOLOR_RED


#define		INS_INFO_SUBTITLE_X								86 + LAPTOP_SCREEN_UL_X
#define		INS_INFO_SUBTITLE_Y								62 + LAPTOP_SCREEN_WEB_UL_Y

#define		INS_INFO_SUBTITLE_LINE_Y					INS_INFO_SUBTITLE_Y + 14
#define		INS_INFO_SUBTITLE_LINE_WIDTH			375

#define		INS_INFO_FIRST_PARAGRAPH_WIDTH		INS_INFO_SUBTITLE_LINE_WIDTH
#define		INS_INFO_FIRST_PARAGRAPH_X				INS_INFO_SUBTITLE_X
#define		INS_INFO_FIRST_PARAGRAPH_Y				INS_INFO_SUBTITLE_LINE_Y + 9

#define		INS_INFO_SPACE_BN_PARAGRAPHS			12

#define		INS_INFO_INFO_TOC_TITLE_X					170
#define		INS_INFO_INFO_TOC_TITLE_Y					54 + LAPTOP_SCREEN_WEB_UL_Y

#define		INS_INFO_TOC_SUBTITLE_X						INS_INFO_SUBTITLE_X


#define		INS_INFO_LINK_TO_CONTRACT_X				235 + LAPTOP_SCREEN_UL_X
#define		INS_INFO_LINK_TO_CONTRACT_Y				392 + LAPTOP_SCREEN_WEB_UL_Y
#define		INS_INFO_LINK_TO_CONTRACT_WIDTH		97//107

#define		INS_INFO_LINK_START_OFFSET				20//14
#define		INS_INFO_LINK_START_X							262 + INS_INFO_LINK_START_OFFSET
#define		INS_INFO_LINK_START_Y							392 + LAPTOP_SCREEN_WEB_UL_Y

#define		INS_INFO_LINK_TO_CONTRACT_TEXT_Y	355 + LAPTOP_SCREEN_WEB_UL_Y


UINT32	guiBulletImage;

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
UINT8	gubCurrentInsInfoSubPage = 0;

BOOLEAN		InsuranceInfoSubPagesVisitedFlag[ INS_INFO_LAST_PAGE-1 ];



INT32		guiInsPrevButtonImage;
static void BtnInsPrevButtonCallback(GUI_BUTTON *btn, INT32 reason);
UINT32	guiInsPrevBackButton;

INT32		guiInsNextButtonImage;
static void BtnInsNextButtonCallback(GUI_BUTTON *btn, INT32 reason);
UINT32	guiInsNextBackButton;


//link to the varios pages
MOUSE_REGION    gSelectedInsuranceInfoLinkRegion;

MOUSE_REGION    gSelectedInsuranceInfoHomeLinkRegion;


void EnterInitInsuranceInfo()
{
	memset( &InsuranceInfoSubPagesVisitedFlag, 0, INS_INFO_LAST_PAGE-1);
}


static void SelectInsuranceInfoHomeLinkRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectInsuranceLinkRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


BOOLEAN EnterInsuranceInfo()
{
	UINT16					usPosX;

	InitInsuranceDefaults();

	// load the Insurance bullet graphic and add it
	guiBulletImage = AddVideoObjectFromFile("LAPTOP/bullet.sti");
	CHECKF(guiBulletImage != NO_VOBJECT);


	//left arrow
	guiInsPrevButtonImage	= LoadButtonImage("LAPTOP/InsLeftButton.sti", 2,0,-1,1,-1 );
	guiInsPrevBackButton = CreateIconAndTextButton( guiInsPrevButtonImage, InsInfoText[INS_INFO_PREVIOUS], INS_FONT_BIG,
													 INS_FONT_COLOR, INS_FONT_SHADOW,
													 INS_FONT_COLOR, INS_FONT_SHADOW,
													 TEXT_CJUSTIFIED,
													 INS_INFO_LEFT_ARROW_BUTTON_X, INS_INFO_LEFT_ARROW_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnInsPrevButtonCallback);
	SetButtonCursor( guiInsPrevBackButton, CURSOR_WWW );
	SpecifyButtonTextOffsets( guiInsPrevBackButton, 17, 16, FALSE );


	//Right arrow
	guiInsNextButtonImage	= LoadButtonImage("LAPTOP/InsRightButton.sti", 2,0,-1,1,-1 );
	guiInsNextBackButton = CreateIconAndTextButton( guiInsNextButtonImage, InsInfoText[INS_INFO_NEXT], INS_FONT_BIG,
													 INS_FONT_COLOR, INS_FONT_SHADOW,
													 INS_FONT_COLOR, INS_FONT_SHADOW,
													 TEXT_CJUSTIFIED,
													 INS_INFO_RIGHT_ARROW_BUTTON_X, INS_INFO_RIGHT_ARROW_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnInsNextButtonCallback);
	SetButtonCursor( guiInsNextBackButton, CURSOR_WWW );
	SpecifyButtonTextOffsets( guiInsNextBackButton, 18, 16, FALSE );


	usPosX = INS_INFO_LINK_START_X;
	//link to go to the contract page
	//link to go to the home page
	MSYS_DefineRegion( &gSelectedInsuranceInfoHomeLinkRegion, usPosX, INS_INFO_LINK_TO_CONTRACT_Y-37, (UINT16)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH), INS_INFO_LINK_TO_CONTRACT_Y+2, MSYS_PRIORITY_HIGH,
					 CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceInfoHomeLinkRegionCallBack);

	usPosX += INS_INFO_LINK_START_OFFSET + INS_INFO_LINK_TO_CONTRACT_WIDTH;
	MSYS_DefineRegion( &gSelectedInsuranceInfoLinkRegion, usPosX, INS_INFO_LINK_TO_CONTRACT_Y-37, (UINT16)(usPosX + INS_INFO_LINK_TO_CONTRACT_WIDTH), INS_INFO_LINK_TO_CONTRACT_Y+2, MSYS_PRIORITY_HIGH,
					 CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceLinkRegionCallBack);

	gubCurrentInsInfoSubPage = INS_INFO_INFO_TOC;

	RenderInsuranceInfo();

	return(TRUE);
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

	DeleteVideoObjectFromIndex( guiBulletImage );
}

void HandleInsuranceInfo()
{

}


static void DisableArrowButtonsIfOnLastOrFirstPage(void);
static void DisplayCancelationPagePage(void);
static void DisplayInfoTocPage(void);
static void DisplayPremiumPage(void);
static void DisplayRenewingPremiumPage(void);
static void DisplaySubmitClaimPage(void);


void RenderInsuranceInfo()
{
	wchar_t		sText[800];
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
	GetInsuranceText( INS_SNGL_TO_ENTER_REVIEW, sText );
	DisplayWrappedString(usPosX, INS_INFO_LINK_TO_CONTRACT_TEXT_Y, INS_INFO_LINK_TO_CONTRACT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	SetFontShadow(DEFAULT_SHADOW);

  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void ChangingInsuranceInfoSubPage(UINT8 ubSubPageNumber);


static void BtnInsPrevButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (gubCurrentInsInfoSubPage > 0) gubCurrentInsInfoSubPage--;
		ChangingInsuranceInfoSubPage(gubCurrentInsInfoSubPage);
//		fPausedReDrawScreenFlag = TRUE;
	}
}


static void BtnInsNextButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (gubCurrentInsInfoSubPage < INS_INFO_LAST_PAGE - 1) gubCurrentInsInfoSubPage++;
		ChangingInsuranceInfoSubPage(gubCurrentInsInfoSubPage);
//		fPausedReDrawScreenFlag = TRUE;
	}
}


static void SelectInsuranceLinkRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_CONTRACT;
	}
}


static void SelectInsuranceInfoHomeLinkRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
	}
}


static void DisplaySubmitClaimPage(void)
{
	wchar_t		sText[800];
	UINT16 usNewLineOffset = 0;
	UINT16	usPosX;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	GetInsuranceText( INS_SNGL_SUBMITTING_CLAIM, sText );
	DrawTextToScreen( sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	//Display the title slogan
	GetInsuranceText( INS_MLTI_U_CAN_REST_ASSURED, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	GetInsuranceText( INS_MLTI_HAD_U_HIRED_AN_INDIVIDUAL, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	//display the BIG FRAUD
	GetInsuranceText( INS_SNGL_FRAUD, sText );
	DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset - 1, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_BIG, INS_INFO_FRAUD_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	usPosX = INS_INFO_FIRST_PARAGRAPH_X + StringPixLength( sText, INS_FONT_BIG ) + 2;
	GetInsuranceText( INS_MLTI_WE_RESERVE_THE_RIGHT, sText );
	usNewLineOffset += DisplayWrappedString(usPosX, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	GetInsuranceText( INS_MLTI_SHOULD_THERE_BE_GROUNDS, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	GetInsuranceText( INS_MLTI_SHOULD_SUCH_A_SITUATION, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

}


static void DisplayPremiumPage(void)
{
	wchar_t		sText[800];
	UINT16 usNewLineOffset = 0;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;


	//Display the title slogan
	GetInsuranceText( INS_SNGL_PREMIUMS, sText );
	DrawTextToScreen( sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	GetInsuranceText( INS_MLTI_EACH_TIME_U_COME_TO_US, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	BltVideoObjectFromIndex(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	GetInsuranceText( INS_MLTI_LENGTH_OF_EMPLOYMENT_CONTRACT, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	BltVideoObjectFromIndex(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	GetInsuranceText( INS_MLTI_EMPLOYEES_AGE_AND_HEALTH, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	BltVideoObjectFromIndex(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	GetInsuranceText( INS_MLTI_EMPLOOYEES_TRAINING_AND_EXP, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}


static void DisplayRenewingPremiumPage(void)
{
	wchar_t		sText[800];
	UINT16 usNewLineOffset = 0;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	GetInsuranceText( INS_SNGL_RENEWL_PREMIUMS, sText );
	DrawTextToScreen( sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	GetInsuranceText( INS_MLTI_WHEN_IT_COMES_TIME_TO_RENEW, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	GetInsuranceText( INS_MLTI_SHOULD_THE_PROJECT_BE_GOING_WELL, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	//display the LOWER PREMIUM FOR RENWING EARLY
	GetInsuranceText( INS_SNGL_LOWER_PREMIUMS_4_RENEWING, sText );
	DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset - 1, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_BIG, INS_INFO_FRAUD_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS + 2;

/*
	BltVideoObjectFromIndex(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	GetInsuranceText( INS_MLTI_IF_U_EXTEND_THE_CONTRACT, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
*/
}


static void DisplayCancelationPagePage(void)
{
	wchar_t		sText[800];
	UINT16 usNewLineOffset = 0;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	GetInsuranceText( INS_SNGL_POLICY_CANCELATIONS, sText );
	DrawTextToScreen( sText, INS_INFO_SUBTITLE_X, INS_INFO_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

	GetInsuranceText( INS_MLTI_WE_WILL_ACCEPT_INS_CANCELATION, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	GetInsuranceText( INS_MLTI_1_HOUR_EXCLUSION_A, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	GetInsuranceText( INS_MLTI_1_HOUR_EXCLUSION_B, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}


static void DisableArrowButtonsIfOnLastOrFirstPage(void)
{
	if( gubCurrentInsInfoSubPage == INS_INFO_INFO_TOC)
		DisableButton( guiInsPrevBackButton);
	else
		EnableButton( guiInsPrevBackButton );

	if( gubCurrentInsInfoSubPage == INS_INFO_LAST_PAGE - 1 )
		DisableButton( guiInsNextBackButton);
	else
		EnableButton( guiInsNextBackButton );
}


static void ChangingInsuranceInfoSubPage(UINT8 ubSubPageNumber)
{
	fLoadPendingFlag = TRUE;

	if( InsuranceInfoSubPagesVisitedFlag[ ubSubPageNumber ] == FALSE )
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
	wchar_t		sText[800];
	UINT16 usNewLineOffset = 0;
	UINT16		usPosY;

	usNewLineOffset = INS_INFO_FIRST_PARAGRAPH_Y;

	//Display the title slogan
	GetInsuranceText( INS_SNGL_HOW_DOES_INS_WORK, sText );
	DrawTextToScreen( sText, INS_INFO_INFO_TOC_TITLE_X, INS_INFO_INFO_TOC_TITLE_Y, 439, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

	//Display the First paragraph
	GetInsuranceText( INS_MLTI_HIRING_4_SHORT_TERM_HIGH_RISK_1, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;

	//Display the 2nd paragraph
	GetInsuranceText( INS_MLTI_HIRING_4_SHORT_TERM_HIGH_RISK_2, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;


	//Display the sub title
	GetInsuranceText( INS_SNGL_WE_CAN_OFFER_U, sText );
	DrawTextToScreen( sText, INS_INFO_TOC_SUBTITLE_X, usNewLineOffset, 640-INS_INFO_INFO_TOC_TITLE_X, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	usPosY = usNewLineOffset + 12;
	DisplaySmallRedLineWithShadow( INS_INFO_SUBTITLE_X, usPosY, (UINT16)(INS_INFO_SUBTITLE_X+INS_INFO_SUBTITLE_LINE_WIDTH), usPosY );

	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;


	//
	// Premiuns bulleted sentence
	//

	BltVideoObjectFromIndex(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	GetInsuranceText( INS_MLTI_REASONABLE_AND_FLEXIBLE, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;



	//
	// Quick and efficient claims
	//

	BltVideoObjectFromIndex(FRAME_BUFFER, guiBulletImage, 0, INS_INFO_FIRST_PARAGRAPH_X, usNewLineOffset);

	GetInsuranceText( INS_MLTI_QUICKLY_AND_EFFICIENT, sText );
	usNewLineOffset += DisplayWrappedString(INS_INFO_FIRST_PARAGRAPH_X + INSURANCE_BULLET_TEXT_OFFSET_X, usNewLineOffset, INS_INFO_FIRST_PARAGRAPH_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usNewLineOffset += INS_INFO_SPACE_BN_PARAGRAPHS;
}
