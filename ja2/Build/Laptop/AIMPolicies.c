#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "AIMPolicies.h"
	#include "AIM.h"
	#include "WCheck.h"
	#include "Utilities.h"
	#include "WordWrap.h"
	#include "Encrypted_File.h"
	#include "Text.h"
	#include <string.h>
#endif



#define	NUM_AIM_POLICY_PAGES				11
#define	NUM_AIM_POLICY_TOC_BUTTONS	9
#define	AIMPOLICYFILE		"BINARYDATA\\AimPol.edt"
#define AIM_POLICY_LINE_SIZE 80 * 5 * 2 // 80 columns of 5 lines that are wide chars, 800 bytes total

#define AIM_POLICY_TITLE_FONT				FONT14ARIAL
#define AIM_POLICY_TITLE_COLOR			AIM_GREEN
#define AIM_POLICY_TEXT_FONT				FONT10ARIAL
#define AIM_POLICY_TEXT_COLOR				FONT_MCOLOR_WHITE
#define AIM_POLICY_TOC_FONT					FONT12ARIAL
#define AIM_POLICY_TOC_COLOR				FONT_MCOLOR_WHITE
#define	AIM_POLICY_TOC_TITLE_FONT		FONT12ARIAL
#define	AIM_POLICY_TOC_TITLE_COLOR	FONT_MCOLOR_WHITE
#define AIM_POLICY_SUBTITLE_FONT		FONT12ARIAL
#define AIM_POLICY_SUBTITLE_COLOR		FONT_MCOLOR_WHITE
#define AIM_POLICY_AGREE_TOC_COLOR_ON			FONT_MCOLOR_WHITE
#define AIM_POLICY_AGREE_TOC_COLOR_OFF			FONT_MCOLOR_DKWHITE

#define	AIM_POLICY_MENU_X						LAPTOP_SCREEN_UL_X + 40
#define	AIM_POLICY_MENU_Y						390 + LAPTOP_SCREEN_WEB_DELTA_Y
#define	AIM_POLICY_MENU_BUTTON_AMOUNT	4
#define	AIM_POLICY_GAP_X						40 + BOTTOM_BUTTON_START_WIDTH

#define	AIM_POLICY_TITLE_X						IMAGE_OFFSET_X + 149
#define AIM_POLICY_TITLE_Y						AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y + 11
#define AIM_POLICY_TITLE_WIDTH				AIM_SYMBOL_WIDTH

#define AIM_POLICY_TITLE_STATEMENT_WIDTH	300
#define	AIM_POLICY_TITLE_STATEMENT_X	IMAGE_OFFSET_X + (500 - AIM_POLICY_TITLE_STATEMENT_WIDTH) / 2 +5//80
#define AIM_POLICY_TITLE_STATEMENT_Y	AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y + 75

#define	AIM_POLICY_SUBTITLE_NUMBER	AIM_POLICY_TITLE_STATEMENT_X - 75
#define	AIM_POLICY_SUBTITLE_X				AIM_POLICY_SUBTITLE_NUMBER + 20
#define	AIM_POLICY_SUBTITLE_Y				115 + LAPTOP_SCREEN_WEB_DELTA_Y

#define	AIM_POLICY_PARAGRAPH_NUMBER	AIM_POLICY_SUBTITLE_X - 12
#define	AIM_POLICY_PARAGRAPH_X			AIM_POLICY_PARAGRAPH_NUMBER + 23
#define	AIM_POLICY_PARAGRAPH_Y			AIM_POLICY_SUBTITLE_Y + 20
#define AIM_POLICY_PARAGRAPH_WIDTH	380
#define AIM_POLICY_PARAGRAPH_GAP		6
#define	AIM_POLICY_SUBPARAGRAPH_NUMBER	AIM_POLICY_PARAGRAPH_X
#define	AIM_POLICY_SUBPARAGRAPH_X		AIM_POLICY_SUBPARAGRAPH_NUMBER + 25

#define AIM_POLICY_TOC_X									259
#define AIM_POLICY_TOC_Y									AIM_POLICY_SUBTITLE_Y
#define	AIM_POLICY_TOC_GAP_Y							25
#define AIM_POLICY_TOC_TEXT_OFFSET_X			5
#define AIM_POLICY_TOC_TEXT_OFFSET_Y			5


#define AIM_POLICY_AGREEMENT_X			IMAGE_OFFSET_X + 150
#define AIM_POLICY_AGREEMENT_Y			350 + LAPTOP_SCREEN_WEB_DELTA_Y

#define AIM_POLICY_DISAGREEMENT_X		AIM_POLICY_AGREEMENT_X + 125
#define AIM_POLICY_DISAGREEMENT_Y		AIM_POLICY_AGREEMENT_Y

#define AIM_POLICY_TOC_PAGE					1
#define	AIM_POLICY_LAST_PAGE				10


#define AIM_POLICY_AGREE_PAGE				0


// These enums represent which paragraph they are located in the AimPol.edt file
enum
{
	AIM_STATEMENT_OF_POLICY,
	AIM_STATEMENT_OF_POLICY_1,
	AIM_STATEMENT_OF_POLICY_2,

	DEFINITIONS,
	DEFINITIONS_1,
	DEFINITIONS_2,
	DEFINITIONS_3,
	DEFINITIONS_4,

	LENGTH_OF_ENGAGEMENT,
	LENGTH_OF_ENGAGEMENT_1,
	LENGTH_OF_ENGAGEMENT_1_1,
	LENGTH_OF_ENGAGEMENT_1_2,
	LENGTH_OF_ENGAGEMENT_1_3,
	LENGTH_OF_ENGAGEMENT_2,

	LOCATION_0F_ENGAGEMENT,
	LOCATION_0F_ENGAGEMENT_1,
	LOCATION_0F_ENGAGEMENT_2,
	LOCATION_0F_ENGAGEMENT_2_1,
	LOCATION_0F_ENGAGEMENT_2_2,
	LOCATION_0F_ENGAGEMENT_2_3,
	LOCATION_0F_ENGAGEMENT_2_4,
	LOCATION_0F_ENGAGEMENT_3,

	CONTRACT_EXTENSIONS,
	CONTRACT_EXTENSIONS_1,
	CONTRACT_EXTENSIONS_2,
	CONTRACT_EXTENSIONS_3,

	TERMS_OF_PAYMENT,
	TERMS_OF_PAYMENT_1,

	TERMS_OF_ENGAGEMENT,
	TERMS_OF_ENGAGEMENT_1,
	TERMS_OF_ENGAGEMENT_2A,
	TERMS_OF_ENGAGEMENT_2B,

	ENGAGEMENT_TERMINATION,
	ENGAGEMENT_TERMINATION_1,
	ENGAGEMENT_TERMINATION_1_1,
	ENGAGEMENT_TERMINATION_1_2,
	ENGAGEMENT_TERMINATION_1_3,

	EQUIPMENT_AND_INVENTORY,
	EQUIPMENT_AND_INVENTORY_1,
	EQUIPMENT_AND_INVENTORY_2,

	POLICY_MEDICAL,
	POLICY_MEDICAL_1,
	POLICY_MEDICAL_2,
	POLICY_MEDICAL_3A,
	POLICY_MEDICAL_3B,
	POLICY_MEDICAL_4,

	NUM_AIM_POLICY_LOCATIONS

}AimPolicyTextLocatoins;


//Toc menu mouse regions
MOUSE_REGION    gSelectedPolicyTocMenuRegion[ NUM_AIM_POLICY_TOC_BUTTONS ];
void SelectPolicyTocMenuRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

//Agree/Disagree menu Buttons regions
void		BtnPoliciesAgreeButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiPoliciesAgreeButton[ 2 ];
INT32		guiPoliciesButtonImage;

//Bottom Menu Buttons
void		BtnPoliciesMenuButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiPoliciesMenuButton[ AIM_POLICY_MENU_BUTTON_AMOUNT ];
INT32		guiPoliciesMenuButtonImage;




UINT32		guiBottomButton;
UINT32		guiBottomButton2;
UINT8			gubCurPageNum;
BOOLEAN		gfInPolicyToc =  FALSE;
BOOLEAN		gfInAgreementPage = FALSE;
BOOLEAN		gfAimPolicyMenuBarLoaded = FALSE;
UINT32		guiContentButton;
BOOLEAN		gfExitingPolicesAgreeButton;
UINT8			gubPoliciesAgreeButtonDown;
UINT8			gubAimPolicyMenuButtonDown=255;
BOOLEAN		gfExitingAimPolicy;
BOOLEAN		AimPoliciesSubPagesVisitedFlag[NUM_AIM_POLICY_PAGES];




BOOLEAN InitAimPolicyMenuBar(void);
BOOLEAN ExitAimPolicyMenuBar(void);
BOOLEAN InitAimPolicyTocMenu(void);
BOOLEAN ExitAimPolicyTocMenu(void);
BOOLEAN DrawAimPolicyMenu();
BOOLEAN  DisplayAimPolicyStatement(void);
BOOLEAN  DisplayAimPolicyTitleText(void);
BOOLEAN InitAgreementRegion(void);
BOOLEAN ExitAgreementButton(void);
void DisableAimPolicyButton();
void ResetAimPolicyButtons();
void ChangingAimPoliciesSubPage( UINT8 ubSubPageNumber );



BOOLEAN  DisplayAimPolicyTitle(UINT16 usPosY, UINT8	ubPageNum, FLOAT fNumber);
UINT16 DisplayAimPolicyParagraph(UINT16 usPosY, UINT8	ubPageNum, FLOAT fNumber);
UINT16 DisplayAimPolicySubParagraph(UINT16 usPosY, UINT8	ubPageNum, FLOAT fNumber);



void GameInitAimPolicies()
{

}

void EnterInitAimPolicies()
{
	memset( &AimPoliciesSubPagesVisitedFlag, 0, NUM_AIM_POLICY_PAGES);
}


BOOLEAN EnterAimPolicies()
{
  VOBJECT_DESC    VObjectDesc;

	InitAimDefaults();

	gubCurPageNum = (UINT8) giCurrentSubPage;

	gfAimPolicyMenuBarLoaded = FALSE;
	gfExitingAimPolicy = FALSE;

	gubPoliciesAgreeButtonDown = 255;
	gubAimPolicyMenuButtonDown	= 255;

	if( gubCurPageNum != 0)
		InitAimPolicyMenuBar();

	gfInPolicyToc = FALSE;

	// load the Bottom Buttons graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\BottomButton.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiBottomButton));

	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\BottomButton2.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiBottomButton2));

	// load the Content Buttons graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\ContentButton.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiContentButton));

	RenderAimPolicies();
	return(TRUE);
}

void ExitAimPolicies()
{
	gfExitingAimPolicy = TRUE;

	DeleteVideoObjectFromIndex(guiBottomButton);
	DeleteVideoObjectFromIndex(guiBottomButton2);
	DeleteVideoObjectFromIndex(guiContentButton);

	if( gfAimPolicyMenuBarLoaded )
		ExitAimPolicyMenuBar();

	if(gfInPolicyToc)
		ExitAimPolicyTocMenu();

	if( gfInAgreementPage )
		ExitAgreementButton();
	RemoveAimDefaults();

	giCurrentSubPage = gubCurPageNum;

}

void HandleAimPolicies()
{
	if( (gfAimPolicyMenuBarLoaded != TRUE) && gubCurPageNum != 0)
	{
		InitAimPolicyMenuBar();
//		RenderAimPolicies();
		fPausedReDrawScreenFlag = TRUE;
	}

}

void RenderAimPolicies()
{
	UINT16	usNumPixles;

	DrawAimDefaults();

	DisplayAimPolicyTitleText();

	if( gfInAgreementPage )
		ExitAgreementButton();

	switch( gubCurPageNum )
	{
		case 0:
			DisplayAimPolicyStatement();
			InitAgreementRegion();
			break;

		case 1:
			InitAimPolicyTocMenu();
			InitAimPolicyMenuBar();
			DisableAimPolicyButton();
			DrawAimPolicyMenu();
			break;

		case 2:
			//Display the Definitions title
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, DEFINITIONS, (FLOAT)1.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, DEFINITIONS_1, (FLOAT)1.1) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, DEFINITIONS_2, (FLOAT)1.2) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, DEFINITIONS_3, (FLOAT)1.3) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, DEFINITIONS_4, (FLOAT)1.4);
			break;

		case 3:
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, LENGTH_OF_ENGAGEMENT, (FLOAT)2.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, LENGTH_OF_ENGAGEMENT_1, (FLOAT)2.1) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, LENGTH_OF_ENGAGEMENT_1_1, (FLOAT)2.11) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, LENGTH_OF_ENGAGEMENT_1_2, (FLOAT)2.12) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, LENGTH_OF_ENGAGEMENT_1_3, (FLOAT)2.13) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, LENGTH_OF_ENGAGEMENT_2, (FLOAT)2.2) + AIM_POLICY_PARAGRAPH_GAP;
			break;

		case 4:
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, LOCATION_0F_ENGAGEMENT, (FLOAT)3.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_1, (FLOAT)3.1) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_2, (FLOAT)3.2) + AIM_POLICY_PARAGRAPH_GAP;

			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_2_1, (FLOAT)3.21) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_2_2, (FLOAT)3.22) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_2_3, (FLOAT)3.23) + AIM_POLICY_PARAGRAPH_GAP;
//			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_2_4, (FLOAT)3.24) + AIM_POLICY_PARAGRAPH_GAP;

			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_2_4, (FLOAT)3.3) + AIM_POLICY_PARAGRAPH_GAP;

			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, LOCATION_0F_ENGAGEMENT_3, (FLOAT)3.4) + AIM_POLICY_PARAGRAPH_GAP;
			break;

		case 5:
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, CONTRACT_EXTENSIONS, (FLOAT)4.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, CONTRACT_EXTENSIONS_1, (FLOAT)4.1) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, CONTRACT_EXTENSIONS_2, (FLOAT)4.2) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, CONTRACT_EXTENSIONS_3, (FLOAT)4.3) + AIM_POLICY_PARAGRAPH_GAP;
			break;

		case 6:
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, TERMS_OF_PAYMENT, (FLOAT)5.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, TERMS_OF_PAYMENT_1, (FLOAT)5.1) + AIM_POLICY_PARAGRAPH_GAP;
			break;

		case 7:
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, TERMS_OF_ENGAGEMENT, (FLOAT)6.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, TERMS_OF_ENGAGEMENT_1, (FLOAT)6.1) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, TERMS_OF_ENGAGEMENT_2A, (FLOAT)6.2) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, TERMS_OF_ENGAGEMENT_2B, (FLOAT)0.0) + AIM_POLICY_PARAGRAPH_GAP;
			break;

		case 8:
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, ENGAGEMENT_TERMINATION, (FLOAT)7.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, ENGAGEMENT_TERMINATION_1, (FLOAT)7.1) + AIM_POLICY_PARAGRAPH_GAP;

			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, ENGAGEMENT_TERMINATION_1_1, (FLOAT)7.11) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, ENGAGEMENT_TERMINATION_1_2, (FLOAT)7.12) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicySubParagraph(usNumPixles, ENGAGEMENT_TERMINATION_1_3, (FLOAT)7.13) + AIM_POLICY_PARAGRAPH_GAP;
			break;

		case 9:
			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, EQUIPMENT_AND_INVENTORY, (FLOAT)8.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, EQUIPMENT_AND_INVENTORY_1, (FLOAT)8.1) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, EQUIPMENT_AND_INVENTORY_2, (FLOAT)8.2) + AIM_POLICY_PARAGRAPH_GAP;
			break;

		case 10:
			DisableAimPolicyButton();

			DisplayAimPolicyTitle(AIM_POLICY_SUBTITLE_Y, POLICY_MEDICAL, (FLOAT)9.0);
			usNumPixles = AIM_POLICY_PARAGRAPH_Y;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, POLICY_MEDICAL_1, (FLOAT)9.1) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, POLICY_MEDICAL_2, (FLOAT)9.2) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, POLICY_MEDICAL_3A, (FLOAT)9.3) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, POLICY_MEDICAL_3B, (FLOAT)0.0) + AIM_POLICY_PARAGRAPH_GAP;
			usNumPixles += DisplayAimPolicyParagraph(usNumPixles, POLICY_MEDICAL_4, (FLOAT)9.4) + AIM_POLICY_PARAGRAPH_GAP;
			break;
	}

  MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}



BOOLEAN InitAimPolicyMenuBar(void)
{
	UINT16					i, usPosX;

	if(gfAimPolicyMenuBarLoaded)
		return(TRUE);

	//Load graphic for buttons
	guiPoliciesMenuButtonImage =  LoadButtonImage("LAPTOP\\BottomButtons2.sti", -1,0,-1,1,-1 );

	usPosX = AIM_POLICY_MENU_X;
	for(i=0; i<AIM_POLICY_MENU_BUTTON_AMOUNT; i++)
	{

//		guiPoliciesMenuButton[i] = QuickCreateButton(guiPoliciesMenuButtonImage, usPosX, AIM_POLICY_MENU_Y,
//																	BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
//																	DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnPoliciesMenuButtonCallback);
//		SetButtonCursor(guiPoliciesMenuButton[i], CURSOR_WWW);
//		MSYS_SetBtnUserData( guiPoliciesMenuButton[i], 0, i);

		guiPoliciesMenuButton[i] = CreateIconAndTextButton( guiPoliciesMenuButtonImage, AimPolicyText[i], FONT10ARIAL,
														 AIM_BUTTON_ON_COLOR, DEFAULT_SHADOW,
														 AIM_BUTTON_OFF_COLOR, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 usPosX, AIM_POLICY_MENU_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 DEFAULT_MOVE_CALLBACK, BtnPoliciesMenuButtonCallback);
		SetButtonCursor(guiPoliciesMenuButton[i], CURSOR_WWW);
		MSYS_SetBtnUserData( guiPoliciesMenuButton[i], 0, i);



		usPosX += AIM_POLICY_GAP_X;
	}




	gfAimPolicyMenuBarLoaded = TRUE;

	return(TRUE);
}

BOOLEAN ExitAimPolicyMenuBar(void)
{
	int i;

	if( !gfAimPolicyMenuBarLoaded )
		return( FALSE );

	for(i=0; i<AIM_POLICY_MENU_BUTTON_AMOUNT; i++)
		RemoveButton( guiPoliciesMenuButton[i]);

	UnloadButtonImage( guiPoliciesMenuButtonImage );

	gfAimPolicyMenuBarLoaded = FALSE;

	return(TRUE);
}



BOOLEAN DrawAimPolicyMenu()
{
	UINT16			i, usPosY;
	UINT16			usHeight;
	UINT32			uiStartLoc=0;
	wchar_t			sText[400];
  HVOBJECT		hContentButtonHandle;
	UINT8				ubLocInFile[]=
								{	DEFINITIONS,
									LENGTH_OF_ENGAGEMENT,
									LOCATION_0F_ENGAGEMENT,
									CONTRACT_EXTENSIONS,
									TERMS_OF_PAYMENT,
									TERMS_OF_ENGAGEMENT,
									ENGAGEMENT_TERMINATION,
									EQUIPMENT_AND_INVENTORY,
									POLICY_MEDICAL};

	GetVideoObject(&hContentButtonHandle, guiContentButton);

	usHeight = GetFontHeight(AIM_POLICY_TOC_FONT);
	usPosY = AIM_POLICY_TOC_Y;
	for(i=0; i<NUM_AIM_POLICY_TOC_BUTTONS; i++)
	{
	  BltVideoObject(FRAME_BUFFER, hContentButtonHandle, 0,AIM_POLICY_TOC_X, usPosY, VO_BLT_SRCTRANSPARENCY,NULL);

		uiStartLoc = AIM_POLICY_LINE_SIZE * ubLocInFile[i];
		LoadEncryptedDataFromFile(AIMPOLICYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
		DrawTextToScreen(sText, AIM_POLICY_TOC_X + AIM_POLICY_TOC_TEXT_OFFSET_X, (UINT16)(usPosY + AIM_POLICY_TOC_TEXT_OFFSET_Y), AIM_CONTENTBUTTON_WIDTH, AIM_POLICY_TOC_FONT, AIM_POLICY_TOC_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

		usPosY += AIM_POLICY_TOC_GAP_Y;
	}
	gfInPolicyToc = TRUE;

	return(TRUE);
}

BOOLEAN InitAimPolicyTocMenu(void)
{
	UINT16			i, usPosY;
	UINT16			usHeight;
	UINT32			uiStartLoc=0;

	if(gfInPolicyToc)
		return(TRUE);

	usHeight = GetFontHeight(AIM_POLICY_TOC_FONT);
	usPosY = AIM_POLICY_TOC_Y;
	for(i=0; i<NUM_AIM_POLICY_TOC_BUTTONS; i++)
	{
		//Mouse region for the toc buttons
		MSYS_DefineRegion( &gSelectedPolicyTocMenuRegion[i], AIM_POLICY_TOC_X, usPosY, (UINT16)(AIM_POLICY_TOC_X + AIM_CONTENTBUTTON_WIDTH), (UINT16)(usPosY + AIM_CONTENTBUTTON_HEIGHT), MSYS_PRIORITY_HIGH,
								 CURSOR_WWW, MSYS_NO_CALLBACK, SelectPolicyTocMenuRegionCallBack);
		MSYS_AddRegion(&gSelectedPolicyTocMenuRegion[i]);
		MSYS_SetRegionUserData( &gSelectedPolicyTocMenuRegion[i], 0, i+2);

		usPosY += AIM_POLICY_TOC_GAP_Y;
	}
	gfInPolicyToc = TRUE;

	return(TRUE);
}



BOOLEAN ExitAimPolicyTocMenu()
{
	UINT16 i;

	gfInPolicyToc = FALSE;
	for(i=0; i<NUM_AIM_POLICY_TOC_BUTTONS; i++)
		MSYS_RemoveRegion( &gSelectedPolicyTocMenuRegion[i]);

	return(TRUE);
}



void SelectPolicyTocMenuRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if(gfInPolicyToc)
	{
		if (iReason & MSYS_CALLBACK_REASON_INIT)
		{
		}
		else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			gubCurPageNum = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

			ChangingAimPoliciesSubPage( gubCurPageNum );

			ExitAimPolicyTocMenu();
			ResetAimPolicyButtons();
			DisableAimPolicyButton();
		}
		else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
		{
		}
	}
}


BOOLEAN  DisplayAimPolicyTitleText(void)
{
	wchar_t	sText[400];
	UINT32	uiStartLoc = 0;

	//Load anfd display title
	uiStartLoc = AIM_POLICY_LINE_SIZE * AIM_STATEMENT_OF_POLICY;
	LoadEncryptedDataFromFile(AIMPOLICYFILE, sText, uiStartLoc, AIM_POLICY_LINE_SIZE);

	if(gubCurPageNum == 0)
		DrawTextToScreen(sText, AIM_POLICY_TITLE_X, AIM_POLICY_TITLE_STATEMENT_Y-25, AIM_POLICY_TITLE_WIDTH, AIM_POLICY_TITLE_FONT, AIM_POLICY_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
	else
		DrawTextToScreen(sText, AIM_POLICY_TITLE_X, AIM_POLICY_TITLE_Y, AIM_POLICY_TITLE_WIDTH, AIM_POLICY_TITLE_FONT, AIM_POLICY_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

	return(TRUE);
}


BOOLEAN  DisplayAimPolicyStatement(void)
{
	wchar_t	sText[400];
	UINT32	uiStartLoc = 0;
	UINT16	usNumPixels;

	//load and display the statment of policies
	uiStartLoc = AIM_POLICY_LINE_SIZE * AIM_STATEMENT_OF_POLICY_1;
	LoadEncryptedDataFromFile(AIMPOLICYFILE, sText, uiStartLoc, AIM_POLICY_LINE_SIZE);
	usNumPixels = DisplayWrappedString(AIM_POLICY_TITLE_STATEMENT_X, AIM_POLICY_TITLE_STATEMENT_Y, AIM_POLICY_TITLE_STATEMENT_WIDTH, 2, AIM_POLICY_TEXT_FONT, AIM_POLICY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	//load and display the statment of policies
	uiStartLoc = AIM_POLICY_LINE_SIZE * AIM_STATEMENT_OF_POLICY_2;
	LoadEncryptedDataFromFile(AIMPOLICYFILE, sText, uiStartLoc, AIM_POLICY_LINE_SIZE);
	DisplayWrappedString(AIM_POLICY_TITLE_STATEMENT_X, (UINT16)(AIM_POLICY_TITLE_STATEMENT_Y + usNumPixels+15), AIM_POLICY_TITLE_STATEMENT_WIDTH, 2, AIM_POLICY_TEXT_FONT, AIM_POLICY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	return(TRUE);
}




BOOLEAN InitAgreementRegion(void)
{
	UINT16	usPosX,i;

	gfExitingPolicesAgreeButton = FALSE;

	//Load graphic for buttons
	guiPoliciesButtonImage =  LoadButtonImage("LAPTOP\\BottomButtons2.sti", -1,0,-1,1,-1 );

	usPosX = AIM_POLICY_AGREEMENT_X;
	for(i=0; i < 2; i++)
	{
//		guiPoliciesAgreeButton[i] = QuickCreateButton(guiPoliciesButtonImage, usPosX, AIM_POLICY_AGREEMENT_Y,
//																	BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
//																	BUTTON_NO_CALLBACK, (GUI_CALLBACK)BtnPoliciesAgreeButtonCallback);
//		SetButtonCursor(guiPoliciesAgreeButton[i], CURSOR_WWW);
//		MSYS_SetBtnUserData( guiPoliciesAgreeButton[i], 0, i);

		guiPoliciesAgreeButton[i] = CreateIconAndTextButton( guiPoliciesButtonImage, AimPolicyText[i+AIM_POLICIES_DISAGREE], AIM_POLICY_TOC_FONT,
														 AIM_POLICY_AGREE_TOC_COLOR_ON, DEFAULT_SHADOW,
														 AIM_POLICY_AGREE_TOC_COLOR_OFF, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 usPosX, AIM_POLICY_AGREEMENT_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 DEFAULT_MOVE_CALLBACK, BtnPoliciesAgreeButtonCallback);
		SetButtonCursor(guiPoliciesAgreeButton[i], CURSOR_WWW);
		MSYS_SetBtnUserData( guiPoliciesAgreeButton[i], 0, i);



		usPosX += 125;
	}
	gfInAgreementPage = TRUE;
	return(TRUE);
}

BOOLEAN ExitAgreementButton(void)
{
	UINT8 i;

	gfExitingPolicesAgreeButton = TRUE;

	UnloadButtonImage( guiPoliciesButtonImage);

	for(i=0; i<2; i++)
	  RemoveButton( guiPoliciesAgreeButton[ i ]);

	gfInAgreementPage = FALSE;

	return(TRUE);
}



BOOLEAN  DisplayAimPolicyTitle(UINT16 usPosY, UINT8	ubPageNum, FLOAT fNumber)
{
	wchar_t	sText[400];
	UINT32	uiStartLoc = 0;

	//Load and display title
	uiStartLoc = AIM_POLICY_LINE_SIZE * ubPageNum;
	LoadEncryptedDataFromFile(AIMPOLICYFILE, sText, uiStartLoc, AIM_POLICY_LINE_SIZE);
	DrawTextToScreen(sText, AIM_POLICY_SUBTITLE_NUMBER, usPosY, 0, AIM_POLICY_SUBTITLE_FONT, AIM_POLICY_SUBTITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	return(TRUE);
}


UINT16 DisplayAimPolicyParagraph(UINT16 usPosY, UINT8	ubPageNum, FLOAT fNumber)
{
	wchar_t	sText[400];
	wchar_t	sTemp[20];
	UINT32	uiStartLoc=0;
	UINT16	usNumPixels;

	uiStartLoc = AIM_POLICY_LINE_SIZE * ubPageNum;
	LoadEncryptedDataFromFile(AIMPOLICYFILE, sText, uiStartLoc, AIM_POLICY_LINE_SIZE);

	if(fNumber != 0.0)
	{
		//Display the section number
		swprintf(sTemp, lengthof(sTemp), L"%2.1f", fNumber);
		DrawTextToScreen(sTemp, AIM_POLICY_PARAGRAPH_NUMBER, usPosY, 0, AIM_POLICY_TEXT_FONT, AIM_POLICY_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	}

	//Display the text beside the section number
	usNumPixels = DisplayWrappedString(AIM_POLICY_PARAGRAPH_X, usPosY, AIM_POLICY_PARAGRAPH_WIDTH, 2, AIM_POLICY_TEXT_FONT, AIM_POLICY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	return(usNumPixels);
}

UINT16 DisplayAimPolicySubParagraph(UINT16 usPosY, UINT8	ubPageNum, FLOAT fNumber)
{
	wchar_t	sText[400];
	wchar_t	sTemp[20];
	UINT32	uiStartLoc=0;
	UINT16	usNumPixels;

	uiStartLoc = AIM_POLICY_LINE_SIZE * ubPageNum;
	LoadEncryptedDataFromFile(AIMPOLICYFILE, sText, uiStartLoc, AIM_POLICY_LINE_SIZE);

	//Display the section number
	swprintf(sTemp, lengthof(sTemp), L"%2.2f", fNumber);
	DrawTextToScreen(sTemp, AIM_POLICY_SUBPARAGRAPH_NUMBER, usPosY, 0, AIM_POLICY_TEXT_FONT, AIM_POLICY_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	//Display the text beside the section number
	usNumPixels = DisplayWrappedString(AIM_POLICY_SUBPARAGRAPH_X, usPosY, AIM_POLICY_PARAGRAPH_WIDTH, 2, AIM_POLICY_TEXT_FONT, AIM_POLICY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	return(usNumPixels);
}




void BtnPoliciesAgreeButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	UINT8	ubRetValue;
	static BOOLEAN fOnPage=TRUE;
	if(fOnPage)
	{
		ubRetValue = (UINT8)MSYS_GetBtnUserData( btn, 0 );
		if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
		{
			btn->uiFlags |= BUTTON_CLICKED_ON;
			InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
			gubPoliciesAgreeButtonDown = ubRetValue;
		}

		if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			if (btn->uiFlags & BUTTON_CLICKED_ON)
			{
				btn->uiFlags &= (~BUTTON_CLICKED_ON );

				//Agree
				fOnPage = FALSE;
				if(ubRetValue == 1)
				{
					gubCurPageNum++;
					ChangingAimPoliciesSubPage( gubCurPageNum );
				}

				//Disagree
				else
				{
					guiCurrentLaptopMode = LAPTOP_MODE_AIM;
				}
				InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
				fOnPage = TRUE;
				gubPoliciesAgreeButtonDown = 255;
			}
		}
		if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );
			gubPoliciesAgreeButtonDown = 255;
			InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
		}
	}
}

void BtnPoliciesMenuButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	UINT8	ubRetValue;
	static BOOLEAN fOnPage=TRUE;
	if(fOnPage)
	{
		ubRetValue = (UINT8)MSYS_GetBtnUserData( btn, 0 );
		if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
		{
			btn->uiFlags |= BUTTON_CLICKED_ON;
			gubAimPolicyMenuButtonDown	= ubRetValue;
			InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
		}

		if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		{
			if (btn->uiFlags & BUTTON_CLICKED_ON)
			{
				btn->uiFlags &= (~BUTTON_CLICKED_ON );

				gubAimPolicyMenuButtonDown	= 255;
				//If previous Page
				if( ubRetValue == 0 )
				{
					if( gubCurPageNum > 1)
					{
						gubCurPageNum--;
						ChangingAimPoliciesSubPage( gubCurPageNum );
					}
				}

				// Home Page
				else if( ubRetValue == 1 )
				{
					guiCurrentLaptopMode = LAPTOP_MODE_AIM;
				}

				//Company policies index
				else if( ubRetValue == 2 )
				{
					if( gubCurPageNum != 1 )
					{
						gubCurPageNum=1;
						ChangingAimPoliciesSubPage( gubCurPageNum );
					}
				}

				//Next Page
				else if( ubRetValue == 3 )
				{
					if( gubCurPageNum < NUM_AIM_POLICY_PAGES-1 )
					{
						gubCurPageNum++;
						ChangingAimPoliciesSubPage( gubCurPageNum );

						fOnPage = FALSE;
						if(gfInPolicyToc)
						{
							ExitAimPolicyTocMenu();
						}
						fOnPage = TRUE;
					}
				}
				InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
				ResetAimPolicyButtons();
				DisableAimPolicyButton();
				fOnPage = TRUE;
			}
		}
		if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );
			gubAimPolicyMenuButtonDown	= 255;
			DisableAimPolicyButton();
			InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
		}
	}
}


void ResetAimPolicyButtons()
{
	int i=0;

	for(i=0; i<AIM_POLICY_MENU_BUTTON_AMOUNT; i++)
	{
		ButtonList[ guiPoliciesMenuButton[i] ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


void DisableAimPolicyButton()
{
	if( gfExitingAimPolicy == TRUE || gfAimPolicyMenuBarLoaded == FALSE )
		return;

	if( (gubCurPageNum == AIM_POLICY_TOC_PAGE ) )
	{
		ButtonList[ guiPoliciesMenuButton[ 0 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
		ButtonList[ guiPoliciesMenuButton[ 2 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
	}
	else if(  ( gubCurPageNum == AIM_POLICY_LAST_PAGE ) )
	{
		ButtonList[ guiPoliciesMenuButton[ 3 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
	}
}


void ChangingAimPoliciesSubPage( UINT8 ubSubPageNumber )
{
	fLoadPendingFlag = TRUE;

	if( AimPoliciesSubPagesVisitedFlag[ ubSubPageNumber ] == FALSE )
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = FALSE;

		AimPoliciesSubPagesVisitedFlag[ ubSubPageNumber ] = TRUE;
	}
	else
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = TRUE;
	}
}
