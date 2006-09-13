#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "AIMHistory.h"
	#include "AIM.h"
	#include "WCheck.h"
	#include "Utilities.h"
	#include "WordWrap.h"
	#include "Encrypted_File.h"
	#include "Text.h"
	#include <string.h>
#endif



// Defines

#define	NUM_AIM_HISTORY_PAGES					5

#define AIM_HISTORY_TITLE_FONT				FONT14ARIAL
#define AIM_HISTORY_TITLE_COLOR				AIM_GREEN
#define AIM_HISTORY_TEXT_FONT					FONT10ARIAL
#define AIM_HISTORY_TEXT_COLOR				FONT_MCOLOR_WHITE
#define AIM_HISTORY_TOC_TEXT_FONT			FONT12ARIAL
#define AIM_HISTORY_TOC_TEXT_COLOR					FONT_MCOLOR_WHITE
#define	AIM_HISTORY_PARAGRAPH_TITLE_FONT		FONT12ARIAL
#define	AIM_HISTORY_PARAGRAPH_TITLE_COLOR		FONT_MCOLOR_WHITE


#define	AIM_HISTORY_MENU_X						LAPTOP_SCREEN_UL_X + 40
#define	AIM_HISTORY_MENU_Y						370 + LAPTOP_SCREEN_WEB_DELTA_Y
#define	AIM_HISTORY_MENU_BUTTON_AMOUNT	4
#define	AIM_HISTORY_GAP_X							40 + BOTTOM_BUTTON_START_WIDTH
#define	AIM_HISTORY_MENU_END_X				AIM_HISTORY_MENU_X + AIM_HISTORY_GAP_X * (AIM_HISTORY_MENU_BUTTON_AMOUNT+2)
#define	AIM_HISTORY_MENU_END_Y				AIM_HISTORY_MENU_Y + BOTTOM_BUTTON_START_HEIGHT

#define	AIM_HISTORY_TEXT_X						IMAGE_OFFSET_X + 149
#define AIM_HISTORY_TEXT_Y						AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y + 45
#define AIM_HISTORY_TEXT_WIDTH				AIM_SYMBOL_WIDTH

#define	AIM_HISTORY_PARAGRAPH_X				LAPTOP_SCREEN_UL_X + 20
#define	AIM_HISTORY_PARAGRAPH_Y				AIM_HISTORY_SUBTITLE_Y + 18
#define	AIM_HISTORY_SUBTITLE_Y				150 + LAPTOP_SCREEN_WEB_DELTA_Y
#define AIM_HISTORY_PARAGRAPH_WIDTH		460

#define AIM_HISTORY_CONTENTBUTTON_X		259
#define AIM_HISTORY_CONTENTBUTTON_Y		AIM_HISTORY_SUBTITLE_Y

#define AIM_HISTORY_TOC_X							AIM_HISTORY_CONTENTBUTTON_X
#define AIM_HISTORY_TOC_Y							5
#define	AIM_HISTORY_TOC_GAP_Y					25

#define	AIM_HISTORY_SPACE_BETWEEN_PARAGRAPHS	8

UINT32		guiBottomButton;
UINT32		guiBottomButton2;
UINT32		guiContentButton;

UINT8			gubCurPageNum;
BOOLEAN		gfInToc =  FALSE;
UINT8			gubAimHistoryMenuButtonDown=255;
BOOLEAN		gfExitingAimHistory;
BOOLEAN		AimHistorySubPagesVisitedFlag[ NUM_AIM_HISTORY_PAGES ];



void ResetAimHistoryButtons();
void DisableAimHistoryButton();


MOUSE_REGION    gSelectedHistoryTocMenuRegion[ NUM_AIM_HISTORY_PAGES ];
void SelectHistoryTocMenuRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

//Bottom Menu Buttons
void		BtnHistoryMenuButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiHistoryMenuButton[ AIM_HISTORY_MENU_BUTTON_AMOUNT ];
INT32		guiHistoryMenuButtonImage;


BOOLEAN DrawAimHistoryMenuBar(void);
BOOLEAN ExitAimHistoryMenuBar(void);
BOOLEAN InitAimHistoryMenuBar(void);
BOOLEAN DisplayAimHistoryParagraph(UINT8	ubPageNum, UINT8 ubNumParagraphs);
BOOLEAN InitTocMenu();
BOOLEAN ExitTocMenu();
void ChangingAimHistorySubPage( UINT8 ubSubPageNumber );


// These enums represent which paragraph they are located in the AimHist.edt file
enum
{
	IN_THE_BEGINNING =6,
	IN_THE_BEGINNING_1,
	IN_THE_BEGINNING_2,

	THE_ISLAND_METAVIRA,
	THE_ISLAND_METAVIRA_1,
	THE_ISLAND_METAVIRA_2,

	GUS_TARBALLS,
	GUS_TARBALLS_1,
	GUS_TARBALLS_2,

	WORD_FROM_FOUNDER,
	WORD_FROM_FOUNDER_1,
	COLONEL_MOHANNED,

	INCORPORATION,
	INCORPORATION_1,
	INCORPORATION_2,
	DUNN_AND_BRADROAD,
	INCORPORATION_3,
} AimHistoryTextLocatoins;



void GameInitAimHistory()
{

}

void EnterInitAimHistory()
{
	memset( &AimHistorySubPagesVisitedFlag, 0, NUM_AIM_HISTORY_PAGES);
}


BOOLEAN EnterAimHistory()
{
  VOBJECT_DESC    VObjectDesc;

	gfExitingAimHistory = FALSE;
	InitAimDefaults();
	InitAimHistoryMenuBar();

	// load the Content Buttons graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\ContentButton.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiContentButton));

	gubCurPageNum = (UINT8) giCurrentSubPage;
	RenderAimHistory();


	DisableAimHistoryButton();
	gubAimHistoryMenuButtonDown	= 255;

	return(TRUE);
}

void ExitAimHistory()
{
	gfExitingAimHistory = TRUE;
	RemoveAimDefaults();
	ExitAimHistoryMenuBar();

	DeleteVideoObjectFromIndex(guiContentButton);
	giCurrentSubPage = gubCurPageNum;

	if(gfInToc)
		ExitTocMenu();

}

void HandleAimHistory()
{

}

void RenderAimHistory()
{
	wchar_t	sText[400];
	UINT32	uiStartLoc=0;
	UINT16	usPosY;

	DrawAimDefaults();
//	DrawAimHistoryMenuBar();
	DisplayAimSlogan();
	DisplayAimCopyright();

	DrawTextToScreen(AimHistoryText[AIM_HISTORY_TITLE], AIM_HISTORY_TEXT_X, AIM_HISTORY_TEXT_Y, AIM_HISTORY_TEXT_WIDTH, AIM_HISTORY_TITLE_FONT, AIM_HISTORY_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);


	switch(gubCurPageNum)
	{
		// History Page TOC
		case 0:
			InitTocMenu();
			break;

		//Load and Display the begining
		case 1:
			DisplayAimHistoryParagraph(IN_THE_BEGINNING, 2);
			break;

		//Load and Display the island of metavira
		case 2:
			DisplayAimHistoryParagraph(THE_ISLAND_METAVIRA, 2);
			break;

		//Load and Display the gus tarballs
		case 3:
			DisplayAimHistoryParagraph(GUS_TARBALLS, 2);
			break;

		//Load and Display the founder
		case 4:
			DisplayAimHistoryParagraph(WORD_FROM_FOUNDER, 1);

			// display coloniel Mohanned...
			usPosY = AIM_HISTORY_PARAGRAPH_Y + (GetFontHeight(AIM_HISTORY_TEXT_FONT) + 2)* 5 + LAPTOP_SCREEN_WEB_DELTA_Y;
			uiStartLoc = AIM_HISTORY_LINE_SIZE * COLONEL_MOHANNED;
			LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
			DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, 210+LAPTOP_SCREEN_WEB_DELTA_Y, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
			break;

		//Load and Display the incorporation
		case 5:
			DisplayAimHistoryParagraph(INCORPORATION, 2);

			// display dunn and bradbord...
			uiStartLoc = AIM_HISTORY_LINE_SIZE * DUNN_AND_BRADROAD;
			LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
			DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, 270+LAPTOP_SCREEN_WEB_DELTA_Y, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);

			//AIM_HISTORY_PARAGRAPH_Y
			uiStartLoc = AIM_HISTORY_LINE_SIZE * INCORPORATION_3;
			LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
			DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, 290+LAPTOP_SCREEN_WEB_DELTA_Y, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
			break;
	}

  MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}




BOOLEAN InitAimHistoryMenuBar(void)
{
  VOBJECT_DESC    VObjectDesc;
	UINT16					i, usPosX;

	// load the Bottom Buttons graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\BottomButton.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiBottomButton));

	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\BottomButton2.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiBottomButton2));

	guiHistoryMenuButtonImage =  LoadButtonImage("LAPTOP\\BottomButtons2.sti", -1,0,-1,1,-1 );
	usPosX = AIM_HISTORY_MENU_X;
	for(i=0; i<AIM_HISTORY_MENU_BUTTON_AMOUNT; i++)
	{
//		guiHistoryMenuButton[i] = QuickCreateButton(guiHistoryMenuButtonImage, usPosX, AIM_HISTORY_MENU_Y,
//																	BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
//																	DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnHistoryMenuButtonCallback);
//		SetButtonCursor(guiHistoryMenuButton[i], CURSOR_WWW);
//		MSYS_SetBtnUserData( guiHistoryMenuButton[i], 0, i+1);

		guiHistoryMenuButton[i] = CreateIconAndTextButton( guiHistoryMenuButtonImage, AimHistoryText[i+AIM_HISTORY_PREVIOUS], FONT10ARIAL,
														 AIM_BUTTON_ON_COLOR, DEFAULT_SHADOW,
														 AIM_BUTTON_OFF_COLOR, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 usPosX, AIM_HISTORY_MENU_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 DEFAULT_MOVE_CALLBACK, BtnHistoryMenuButtonCallback);
		SetButtonCursor(guiHistoryMenuButton[i], CURSOR_WWW);
		MSYS_SetBtnUserData( guiHistoryMenuButton[i], 0, i+1);


		usPosX += AIM_HISTORY_GAP_X;

	}

	return(TRUE);
}


BOOLEAN ExitAimHistoryMenuBar(void)
{
	int i;

//	DeleteVideoObjectFromIndex(guiHistoryMenuButtonImage);
	UnloadButtonImage( guiHistoryMenuButtonImage );


	for(i=0; i<AIM_HISTORY_MENU_BUTTON_AMOUNT; i++)
 		RemoveButton( guiHistoryMenuButton[i] );

	return(TRUE);
}


void SelectHistoryMenuButtonsRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	UINT8	rValue;
	static BOOLEAN fOnPage=TRUE;

	if(fOnPage)
	{
		if (iReason & MSYS_CALLBACK_REASON_INIT)
		{
		}
		else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			rValue = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
			//Previous Page
			if( rValue == 1 )
			{
				if( gubCurPageNum > 0)
				{
					gubCurPageNum--;
					ChangingAimHistorySubPage( gubCurPageNum );
//					RenderAimHistory();
				}
			}

			// Home Page
			else if( rValue == 2 )
			{
				guiCurrentLaptopMode = LAPTOP_MODE_AIM;
			}
			//Company policies
			else if( rValue == 3 )
			{
				guiCurrentLaptopMode = LAPTOP_MODE_AIM_POLICIES;
			}
			//Next Page
			else if( rValue == 4 )
			{
				if( gubCurPageNum < NUM_AIM_HISTORY_PAGES )
				{
					gubCurPageNum++;
					ChangingAimHistorySubPage( gubCurPageNum );

					fOnPage = FALSE;
					if(gfInToc)
					{
						ExitTocMenu();
					}
					fOnPage = TRUE;
//					RenderAimHistory();
				}
			}
		}
		else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
		{
		}
	}
}


BOOLEAN DisplayAimHistoryParagraph(UINT8	ubPageNum, UINT8 ubNumParagraphs)
{
	wchar_t	sText[400];
	UINT32	uiStartLoc=0;
	UINT16	usPosY=0;
	UINT16	usNumPixels=0;

	//title
	uiStartLoc = AIM_HISTORY_LINE_SIZE * ubPageNum;
	LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
	DrawTextToScreen(sText, AIM_HISTORY_PARAGRAPH_X, AIM_HISTORY_SUBTITLE_Y, 0, AIM_HISTORY_PARAGRAPH_TITLE_FONT, AIM_HISTORY_PARAGRAPH_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	if(ubNumParagraphs >= 1)
	{
		usPosY = AIM_HISTORY_PARAGRAPH_Y;
		//1st paragraph
		uiStartLoc = AIM_HISTORY_LINE_SIZE * (ubPageNum + 1 );
		LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
		usNumPixels = DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, usPosY, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	}

	if(ubNumParagraphs >= 2)
	{
		//2nd paragraph
		usPosY += usNumPixels + AIM_HISTORY_SPACE_BETWEEN_PARAGRAPHS;
		uiStartLoc = AIM_HISTORY_LINE_SIZE * (ubPageNum + 2 );
		LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
		DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, usPosY, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	}

	if(ubNumParagraphs >= 3)
	{
		//3rd paragraph
		usPosY += usNumPixels + AIM_HISTORY_SPACE_BETWEEN_PARAGRAPHS;

		uiStartLoc = AIM_HISTORY_LINE_SIZE * (ubPageNum + 3 );
		LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);
		DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, usPosY, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	}

	return(TRUE);
}



BOOLEAN InitTocMenu()
{
	UINT16			i, usPosY;
	UINT16			usHeight;
	UINT16			usWidth=0;
	UINT32			uiStartLoc=0;
	wchar_t			sText[400];
	UINT8				ubLocInFile[]=
								{IN_THE_BEGINNING,
								 THE_ISLAND_METAVIRA,
								 GUS_TARBALLS,
								 WORD_FROM_FOUNDER,
								 INCORPORATION};

  HVOBJECT		hContentButtonHandle;

	GetVideoObject(&hContentButtonHandle, guiContentButton);

	usHeight = GetFontHeight(AIM_HISTORY_TOC_TEXT_FONT);
	usPosY = AIM_HISTORY_CONTENTBUTTON_Y;
	for(i=0; i<NUM_AIM_HISTORY_PAGES; i++)
	{
		uiStartLoc = AIM_HISTORY_LINE_SIZE * ubLocInFile[i];
		LoadEncryptedDataFromFile(AIMHISTORYFILE, sText, uiStartLoc, AIM_HISTORY_LINE_SIZE);

		usWidth = StringPixLength(sText, AIM_HISTORY_TOC_TEXT_FONT);

		//if the mouse regions havent been inited, init them
		if( !gfInToc )
		{
			//Mouse region for the history toc buttons
			MSYS_DefineRegion( &gSelectedHistoryTocMenuRegion[i], AIM_HISTORY_TOC_X, usPosY, (UINT16)(AIM_HISTORY_TOC_X + AIM_CONTENTBUTTON_WIDTH), (UINT16)(usPosY + AIM_CONTENTBUTTON_HEIGHT), MSYS_PRIORITY_HIGH,
									 CURSOR_WWW, MSYS_NO_CALLBACK, SelectHistoryTocMenuRegionCallBack);
			MSYS_AddRegion(&gSelectedHistoryTocMenuRegion[i]);
			MSYS_SetRegionUserData( &gSelectedHistoryTocMenuRegion[i], 0, i+1);
		}

	  BltVideoObject(FRAME_BUFFER, hContentButtonHandle, 0,AIM_HISTORY_TOC_X, usPosY, VO_BLT_SRCTRANSPARENCY,NULL);
		DrawTextToScreen(sText, AIM_HISTORY_TOC_X, (UINT16)(usPosY + AIM_HISTORY_TOC_Y), AIM_CONTENTBUTTON_WIDTH, AIM_HISTORY_TOC_TEXT_FONT, AIM_HISTORY_TOC_TEXT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);


		usPosY += AIM_HISTORY_TOC_GAP_Y;
	}
	gfInToc = TRUE;
	return(TRUE);
}





BOOLEAN ExitTocMenu()
{
	UINT16 i;

	if( gfInToc )
	{
		gfInToc = FALSE;
		for(i=0; i<NUM_AIM_HISTORY_PAGES; i++)
			MSYS_RemoveRegion( &gSelectedHistoryTocMenuRegion[i]);
	}

	return(TRUE);
}



void SelectHistoryTocMenuRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if(gfInToc)
	{
		if (iReason & MSYS_CALLBACK_REASON_INIT)
		{
		}
		else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			gubCurPageNum = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
			ChangingAimHistorySubPage( gubCurPageNum );

			ExitTocMenu();
			ResetAimHistoryButtons();
//			RenderAimHistory();
			DisableAimHistoryButton();
		}
		else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
		{
		}
	}
}


void BtnHistoryMenuButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	UINT8	ubRetValue = (UINT8)MSYS_GetBtnUserData( btn, 0 );
	gubAimHistoryMenuButtonDown = 255;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;

		gubAimHistoryMenuButtonDown = ubRetValue;

		InvalidateRegion(AIM_HISTORY_MENU_X,AIM_HISTORY_MENU_Y, AIM_HISTORY_MENU_END_X,AIM_HISTORY_MENU_END_Y);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );
			ResetAimHistoryButtons();

				if( ubRetValue == 1 )
				{
					if( gubCurPageNum > 0)
					{
						gubCurPageNum--;
						ChangingAimHistorySubPage( gubCurPageNum );

//						RenderAimHistory();
						ResetAimHistoryButtons();
					}
					else
						btn->uiFlags |= (BUTTON_CLICKED_ON );
				}

				// Home Page
				else if( ubRetValue == 2 )
				{
					guiCurrentLaptopMode = LAPTOP_MODE_AIM;
				}
				//Company policies
				else if( ubRetValue == 3 )
				{
					guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_ARCHIVES;
				}
				//Next Page
				else if( ubRetValue == 4 )
				{
					if( gubCurPageNum < NUM_AIM_HISTORY_PAGES )
					{
						gubCurPageNum++;
						ChangingAimHistorySubPage( gubCurPageNum );

						if(gfInToc)
						{
							ExitTocMenu();
						}
//						RenderAimHistory();
					}
					else
						btn->uiFlags |= (BUTTON_CLICKED_ON );

				}

			DisableAimHistoryButton();

			InvalidateRegion(AIM_HISTORY_MENU_X,AIM_HISTORY_MENU_Y, AIM_HISTORY_MENU_END_X,AIM_HISTORY_MENU_END_Y);
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );

		DisableAimHistoryButton();

		InvalidateRegion(AIM_HISTORY_MENU_X,AIM_HISTORY_MENU_Y, AIM_HISTORY_MENU_END_X,AIM_HISTORY_MENU_END_Y);
	}
}


void ResetAimHistoryButtons()
{
	int i=0;

	for(i=0; i<AIM_HISTORY_MENU_BUTTON_AMOUNT; i++)
	{
		ButtonList[ guiHistoryMenuButton[i] ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}

void DisableAimHistoryButton()
{
	if( gfExitingAimHistory == TRUE)
		return;

	if( (gubCurPageNum == 0 ) )
	{
		ButtonList[ guiHistoryMenuButton[ 0 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
	}
	else if(  ( gubCurPageNum == 5) )
	{
		ButtonList[ guiHistoryMenuButton[ AIM_HISTORY_MENU_BUTTON_AMOUNT-1 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
	}
}



void ChangingAimHistorySubPage( UINT8 ubSubPageNumber )
{
	fLoadPendingFlag = TRUE;

	if( AimHistorySubPagesVisitedFlag[ ubSubPageNumber ] == FALSE )
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = FALSE;

		AimHistorySubPagesVisitedFlag[ ubSubPageNumber ] = TRUE;
	}
	else
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = TRUE;
	}
}
