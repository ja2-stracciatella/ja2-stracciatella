#include "Cursors.h"
#include "Font.h"
#include "Laptop.h"
#include "AIMHistory.h"
#include "AIM.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Encrypted_File.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"


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

static SGPVObject* guiContentButton;

static UINT8 gubCurPageNum;
BOOLEAN		gfInToc =  FALSE;
BOOLEAN		gfExitingAimHistory;
BOOLEAN		AimHistorySubPagesVisitedFlag[ NUM_AIM_HISTORY_PAGES ];


MOUSE_REGION    gSelectedHistoryTocMenuRegion[ NUM_AIM_HISTORY_PAGES ];

static GUIButtonRef guiHistoryMenuButton[AIM_HISTORY_MENU_BUTTON_AMOUNT];
static BUTTON_PICS* guiHistoryMenuButtonImage;


// These enums represent which paragraph they are located in the AimHist.edt file
enum AimHistoryTextLocations
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
	INCORPORATION_3
};


void EnterInitAimHistory()
{
	memset( &AimHistorySubPagesVisitedFlag, 0, NUM_AIM_HISTORY_PAGES);
}


static void DisableAimHistoryButton(void);
static void InitAimHistoryMenuBar(void);


void EnterAimHistory()
{
	gfExitingAimHistory = FALSE;
	InitAimDefaults();
	InitAimHistoryMenuBar();

	// load the Content Buttons graphic and add it
	guiContentButton = AddVideoObjectFromFile("LAPTOP/ContentButton.sti");

	gubCurPageNum = (UINT8) giCurrentSubPage;
	RenderAimHistory();


	DisableAimHistoryButton();
}


static void ExitAimHistoryMenuBar(void);
static void ExitTocMenu(void);


void ExitAimHistory()
{
	gfExitingAimHistory = TRUE;
	RemoveAimDefaults();
	ExitAimHistoryMenuBar();

	DeleteVideoObject(guiContentButton);
	giCurrentSubPage = gubCurPageNum;

	if(gfInToc)
		ExitTocMenu();

}

void HandleAimHistory()
{

}


static void LoadAIMHistoryText(wchar_t buf[], UINT32 entry)
{
	LoadEncryptedDataFromFile(AIMHISTORYFILE, buf, AIM_HISTORY_LINE_SIZE * entry, AIM_HISTORY_LINE_SIZE);
}


static void DisplayAimHistoryParagraph(UINT8 ubPageNum, UINT8 ubNumParagraphs);
static void InitTocMenu(void);


void RenderAimHistory()
{
	wchar_t	sText[AIM_HISTORY_LINE_SIZE];

	DrawAimDefaults();
//	DrawAimHistoryMenuBar();
	DisplayAimSlogan();
	DisplayAimCopyright();

	DrawTextToScreen(AimHistoryText[AIM_HISTORY_TITLE], AIM_HISTORY_TEXT_X, AIM_HISTORY_TEXT_Y, AIM_HISTORY_TEXT_WIDTH, AIM_HISTORY_TITLE_FONT, AIM_HISTORY_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

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
			LoadAIMHistoryText(sText, COLONEL_MOHANNED);
			DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, 210 + LAPTOP_SCREEN_WEB_DELTA_Y, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
			break;

		//Load and Display the incorporation
		case 5:
			DisplayAimHistoryParagraph(INCORPORATION, 2);

			// display dunn and bradbord...
			LoadAIMHistoryText(sText, DUNN_AND_BRADROAD);
			DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, 270 + LAPTOP_SCREEN_WEB_DELTA_Y, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

			//AIM_HISTORY_PARAGRAPH_Y
			LoadAIMHistoryText(sText, INCORPORATION_3);
			DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, 290 + LAPTOP_SCREEN_WEB_DELTA_Y, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
			break;
	}

  MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnHistoryMenuButtonCallback(GUI_BUTTON* btn, INT32 reason);


static void InitAimHistoryMenuBar(void)
{
	UINT16					i, usPosX;

	guiHistoryMenuButtonImage = LoadButtonImage("LAPTOP/BottomButtons2.sti", 0, 1);
	usPosX = AIM_HISTORY_MENU_X;
	for(i=0; i<AIM_HISTORY_MENU_BUTTON_AMOUNT; i++)
	{
		guiHistoryMenuButton[i] = CreateIconAndTextButton( guiHistoryMenuButtonImage, AimHistoryText[i+AIM_HISTORY_PREVIOUS], FONT10ARIAL,
														 AIM_BUTTON_ON_COLOR, DEFAULT_SHADOW,
														 AIM_BUTTON_OFF_COLOR, DEFAULT_SHADOW,
														 usPosX, AIM_HISTORY_MENU_Y, MSYS_PRIORITY_HIGH,
														 BtnHistoryMenuButtonCallback);
		guiHistoryMenuButton[i]->SetCursor(CURSOR_WWW);
		guiHistoryMenuButton[i]->SetUserData(i + 1);


		usPosX += AIM_HISTORY_GAP_X;

	}
}


static void ExitAimHistoryMenuBar(void)
{
	int i;

	UnloadButtonImage( guiHistoryMenuButtonImage );


	for(i=0; i<AIM_HISTORY_MENU_BUTTON_AMOUNT; i++)
 		RemoveButton( guiHistoryMenuButton[i] );
}


static void DisplayAimHistoryParagraph(UINT8 ubPageNum, UINT8 ubNumParagraphs)
{
	wchar_t	sText[AIM_HISTORY_LINE_SIZE];
	UINT16	usPosY=0;
	UINT16	usNumPixels=0;

	//title
	LoadAIMHistoryText(sText, ubPageNum);
	DrawTextToScreen(sText, AIM_HISTORY_PARAGRAPH_X, AIM_HISTORY_SUBTITLE_Y, 0, AIM_HISTORY_PARAGRAPH_TITLE_FONT, AIM_HISTORY_PARAGRAPH_TITLE_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	if(ubNumParagraphs >= 1)
	{
		usPosY = AIM_HISTORY_PARAGRAPH_Y;
		//1st paragraph
		LoadAIMHistoryText(sText, ubPageNum + 1);
		usNumPixels = DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, usPosY, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	if(ubNumParagraphs >= 2)
	{
		//2nd paragraph
		usPosY += usNumPixels + AIM_HISTORY_SPACE_BETWEEN_PARAGRAPHS;
		LoadAIMHistoryText(sText, ubPageNum + 2);
		DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, usPosY, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	if(ubNumParagraphs >= 3)
	{
		//3rd paragraph
		usPosY += usNumPixels + AIM_HISTORY_SPACE_BETWEEN_PARAGRAPHS;
		LoadAIMHistoryText(sText, ubPageNum + 3);
		DisplayWrappedString(AIM_HISTORY_PARAGRAPH_X, usPosY, AIM_HISTORY_PARAGRAPH_WIDTH, 2, AIM_HISTORY_TEXT_FONT, AIM_HISTORY_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
}


static void SelectHistoryTocMenuRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void InitTocMenu(void)
{
	UINT16			i, usPosY;
	UINT8				ubLocInFile[]=
								{IN_THE_BEGINNING,
								 THE_ISLAND_METAVIRA,
								 GUS_TARBALLS,
								 WORD_FROM_FOUNDER,
								 INCORPORATION};

	usPosY = AIM_HISTORY_CONTENTBUTTON_Y;
	for(i=0; i<NUM_AIM_HISTORY_PAGES; i++)
	{
		wchar_t sText[AIM_HISTORY_LINE_SIZE];
		LoadAIMHistoryText(sText, ubLocInFile[i]);

		//if the mouse regions havent been inited, init them
		if( !gfInToc )
		{
			//Mouse region for the history toc buttons
			MSYS_DefineRegion( &gSelectedHistoryTocMenuRegion[i], AIM_HISTORY_TOC_X, usPosY, (UINT16)(AIM_HISTORY_TOC_X + AIM_CONTENTBUTTON_WIDTH), (UINT16)(usPosY + AIM_CONTENTBUTTON_HEIGHT), MSYS_PRIORITY_HIGH,
									 CURSOR_WWW, MSYS_NO_CALLBACK, SelectHistoryTocMenuRegionCallBack);
			MSYS_SetRegionUserData( &gSelectedHistoryTocMenuRegion[i], 0, i+1);
		}

		BltVideoObject(FRAME_BUFFER, guiContentButton, 0,AIM_HISTORY_TOC_X, usPosY);
		DrawTextToScreen(sText, AIM_HISTORY_TOC_X, usPosY + AIM_HISTORY_TOC_Y, AIM_CONTENTBUTTON_WIDTH, AIM_HISTORY_TOC_TEXT_FONT, AIM_HISTORY_TOC_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


		usPosY += AIM_HISTORY_TOC_GAP_Y;
	}
	gfInToc = TRUE;
}


static void ExitTocMenu(void)
{
	UINT16 i;

	if( gfInToc )
	{
		gfInToc = FALSE;
		for(i=0; i<NUM_AIM_HISTORY_PAGES; i++)
			MSYS_RemoveRegion( &gSelectedHistoryTocMenuRegion[i]);
	}
}


static void ChangingAimHistorySubPage(UINT8 ubSubPageNumber);
static void ResetAimHistoryButtons(void);


static void SelectHistoryTocMenuRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if(gfInToc)
	{
		if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
		{
			gubCurPageNum = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
			ChangingAimHistorySubPage( gubCurPageNum );

			ExitTocMenu();
			ResetAimHistoryButtons();
//			RenderAimHistory();
			DisableAimHistoryButton();
		}
	}
}


static void BtnHistoryMenuButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	UINT8	const ubRetValue = btn->GetUserData();

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ResetAimHistoryButtons();

		switch (ubRetValue)
		{
			case 1:
				if (gubCurPageNum > 0)
				{
					gubCurPageNum--;
					ChangingAimHistorySubPage(gubCurPageNum);
					ResetAimHistoryButtons();
				}
				else
				{
					btn->uiFlags |= BUTTON_CLICKED_ON;
				}
				break;

			case 2: // Home Page
				guiCurrentLaptopMode = LAPTOP_MODE_AIM;
				break;

			case 3: //Company policies
				guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_ARCHIVES;
				break;

			case 4: //Next Page
				if (gubCurPageNum < NUM_AIM_HISTORY_PAGES)
				{
					gubCurPageNum++;
					ChangingAimHistorySubPage(gubCurPageNum);
					if (gfInToc) ExitTocMenu();
				}
				else
				{
					btn->uiFlags |= BUTTON_CLICKED_ON;
				}
				break;
		}
		DisableAimHistoryButton();
	}
}


static void ResetAimHistoryButtons(void)
{
	int i=0;

	for(i=0; i<AIM_HISTORY_MENU_BUTTON_AMOUNT; i++)
	{
		guiHistoryMenuButton[i]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


static void DisableAimHistoryButton(void)
{
	if (gfExitingAimHistory) return;

	if( (gubCurPageNum == 0 ) )
	{
		guiHistoryMenuButton[0]->uiFlags |= BUTTON_CLICKED_ON;
	}
	else if(  ( gubCurPageNum == 5) )
	{
		guiHistoryMenuButton[AIM_HISTORY_MENU_BUTTON_AMOUNT - 1]->uiFlags |= BUTTON_CLICKED_ON;
	}
}


static void ChangingAimHistorySubPage(UINT8 ubSubPageNumber)
{
	fLoadPendingFlag = TRUE;

	if (!AimHistorySubPagesVisitedFlag[ubSubPageNumber])
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
