#include "Directories.h"
#include "Laptop.h"
#include "Florist.h"
#include "Florist_Gallery.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "Font_Control.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>

#define FLOR_GALLERY_TITLE_FONT			FONT10ARIAL
#define FLOR_GALLERY_TITLE_COLOR		FONT_MCOLOR_WHITE

#define FLOR_GALLERY_FLOWER_TITLE_FONT		FONT14ARIAL
#define FLOR_GALLERY_FLOWER_TITLE_COLOR		FONT_MCOLOR_WHITE

#define FLOR_GALLERY_FLOWER_PRICE_FONT		FONT12ARIAL
#define FLOR_GALLERY_FLOWER_PRICE_COLOR		FONT_MCOLOR_WHITE

#define FLOR_GALLERY_FLOWER_DESC_FONT		FONT12ARIAL
#define FLOR_GALLERY_FLOWER_DESC_COLOR		FONT_MCOLOR_WHITE

#define FLOR_GALLERY_NUMBER_FLORAL_BUTTONS	3
#define FLOR_GALLERY_NUMBER_FLORAL_IMAGES	10

#define FLOR_GALLERY_FLOWER_DESC_TEXT_FONT	FONT12ARIAL
#define FLOR_GALLERY_FLOWER_DESC_TEXT_COLOR	FONT_MCOLOR_WHITE

#define FLOR_GALLERY_BACK_BUTTON_X		(LAPTOP_SCREEN_UL_X +   8)
#define FLOR_GALLERY_NEXT_BUTTON_X		(LAPTOP_SCREEN_UL_X + 420)
#define FLOR_GALLERY_BUTTON_Y			(LAPTOP_SCREEN_WEB_UL_Y + 12)

#define FLOR_GALLERY_FLOWER_BUTTON_X		LAPTOP_SCREEN_UL_X + 7
#define FLOR_GALLERY_FLOWER_BUTTON_Y		LAPTOP_SCREEN_WEB_UL_Y + 74

//#define FLOR_GALLERY_FLOWER_BUTTON_OFFSET_X	250

#define FLOR_GALLERY_FLOWER_BUTTON_OFFSET_Y	112

#define FLOR_GALLERY_TITLE_TEXT_X		LAPTOP_SCREEN_UL_X + 0
#define FLOR_GALLERY_TITLE_TEXT_Y		LAPTOP_SCREEN_WEB_UL_Y + 48
#define FLOR_GALLERY_TITLE_TEXT_WIDTH		LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X

#define FLOR_GALLERY_FLOWER_TITLE_X		FLOR_GALLERY_FLOWER_BUTTON_X + 88

#define FLOR_GALLERY_DESC_WIDTH			390

#define FLOR_GALLERY_FLOWER_TITLE_OFFSET_Y	9
#define FLOR_GALLERY_FLOWER_PRICE_OFFSET_Y	FLOR_GALLERY_FLOWER_TITLE_OFFSET_Y + 17
#define FLOR_GALLERY_FLOWER_DESC_OFFSET_Y	FLOR_GALLERY_FLOWER_PRICE_OFFSET_Y + 15


static SGPVObject* guiFlowerImages[3];

UINT32  guiCurrentlySelectedFlower=0;

UINT8   gubCurFlowerIndex=0;
UINT8   gubCurNumberOfFlowers=0;
UINT8   gubPrevNumberOfFlowers=0;
BOOLEAN gfRedrawFloristGallery=FALSE;

BOOLEAN FloristGallerySubPagesVisitedFlag[ 4 ];

//Floral buttons
static BUTTON_PICS* guiGalleryButtonImage;
static GUIButtonRef guiGalleryButton[FLOR_GALLERY_NUMBER_FLORAL_BUTTONS];

//Next Previous buttons
static BUTTON_PICS* guiFloralGalleryButtonImage;
static void BtnFloralGalleryNextButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static void BtnFloralGalleryBackButtonCallback(GUI_BUTTON *btn, UINT32 reason);
GUIButtonRef guiFloralGalleryButton[2];


void EnterInitFloristGallery()
{
	std::fill_n(FloristGallerySubPagesVisitedFlag, 4, 0);
}


static GUIButtonRef MakeButton(const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	const INT16 shadow_col = FLORIST_BUTTON_TEXT_SHADOW_COLOR;
	GUIButtonRef const btn = CreateIconAndTextButton(guiFloralGalleryButtonImage, text, FLORIST_BUTTON_TEXT_FONT, FLORIST_BUTTON_TEXT_UP_COLOR, shadow_col, FLORIST_BUTTON_TEXT_DOWN_COLOR, shadow_col, x, FLOR_GALLERY_BUTTON_Y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_WWW);
	return btn;
}


static void InitFlowerButtons(void);


BOOLEAN EnterFloristGallery()
{
	InitFloristDefaults();

	//the next previous buttons
	guiFloralGalleryButtonImage	= LoadButtonImage(LAPTOPDIR "/floristbuttons.sti", 0, 1);
	guiFloralGalleryButton[0] = MakeButton(sFloristGalleryText[FLORIST_GALLERY_PREV], FLOR_GALLERY_BACK_BUTTON_X, BtnFloralGalleryBackButtonCallback);
	guiFloralGalleryButton[1] = MakeButton(sFloristGalleryText[FLORIST_GALLERY_NEXT], FLOR_GALLERY_NEXT_BUTTON_X, BtnFloralGalleryNextButtonCallback);

	RenderFloristGallery();

	InitFlowerButtons();

	return(TRUE);
}


static void DeleteFlowerButtons(void);


void ExitFloristGallery()
{
	UINT16 i;

	RemoveFloristDefaults();

	for(i=0; i<2; i++)
		RemoveButton( guiFloralGalleryButton[i] );

	UnloadButtonImage( guiFloralGalleryButtonImage  );

	DeleteFlowerButtons();

}

void HandleFloristGallery()
{
	if( gfRedrawFloristGallery )
	{
		gfRedrawFloristGallery=FALSE;

		//
		DeleteFlowerButtons();
		InitFlowerButtons();

		fPausedReDrawScreenFlag = TRUE;
	}

}


static BOOLEAN DisplayFloralDescriptions(void);


void RenderFloristGallery()
{
	DisplayFloristDefaults();

	DrawTextToScreen(sFloristGalleryText[FLORIST_GALLERY_CLICK_TO_ORDER],  FLOR_GALLERY_TITLE_TEXT_X, FLOR_GALLERY_TITLE_TEXT_Y,      FLOR_GALLERY_TITLE_TEXT_WIDTH, FLOR_GALLERY_TITLE_FONT, FLOR_GALLERY_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(sFloristGalleryText[FLORIST_GALLERY_ADDIFTIONAL_FEE], FLOR_GALLERY_TITLE_TEXT_X, FLOR_GALLERY_TITLE_TEXT_Y + 11, FLOR_GALLERY_TITLE_TEXT_WIDTH, FLOR_GALLERY_TITLE_FONT, FLOR_GALLERY_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	DisplayFloralDescriptions();

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void ChangingFloristGallerySubPage(UINT8 ubSubPageNumber);


static void BtnFloralGalleryNextButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (gubCurFlowerIndex + 3 <= FLOR_GALLERY_NUMBER_FLORAL_IMAGES)
			gubCurFlowerIndex += 3;

		ChangingFloristGallerySubPage(gubCurFlowerIndex);
		gfRedrawFloristGallery = TRUE;
	}
}


static void BtnFloralGalleryBackButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (gubCurFlowerIndex != 0)
		{
			if (gubCurFlowerIndex >= 3)
				gubCurFlowerIndex -= 3;
			else
				gubCurFlowerIndex = 0;
			ChangingFloristGallerySubPage(gubCurFlowerIndex);
		}
		else
		{
			guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
		}
		gfRedrawFloristGallery = TRUE;
	}
}


static void BtnGalleryFlowerButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentlySelectedFlower = btn->GetUserData();
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_ORDERFORM;
		gfShowBookmarks = FALSE;
	}
}


static void InitFlowerButtons(void)
{
	UINT16 i,j, count;
	UINT16 usPosY;

	if( (FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex) >= 3 )
		gubCurNumberOfFlowers = 3;
	else
		gubCurNumberOfFlowers = FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex;

	gubPrevNumberOfFlowers = gubCurNumberOfFlowers;

	//the 10 pictures of the flowers
	count = gubCurFlowerIndex;
	for(i=0; i<gubCurNumberOfFlowers; i++)
	{
		// load the handbullet graphic and add it
		ST::string sTemp = ST::format(LAPTOPDIR "/flower_{}.sti", count);
		guiFlowerImages[i] = AddVideoObjectFromFile(sTemp);
		count++;
	}

	//the buttons with the flower pictures on them
	usPosY = FLOR_GALLERY_FLOWER_BUTTON_Y;
	//usPosX = FLOR_GALLERY_FLOWER_BUTTON_X;
	count = gubCurFlowerIndex;
	guiGalleryButtonImage	= LoadButtonImage(LAPTOPDIR "/gallerybuttons.sti", 0, 1);
	for(j=0; j<gubCurNumberOfFlowers; j++)
	{
		guiGalleryButton[j] = QuickCreateButton(guiGalleryButtonImage, FLOR_GALLERY_FLOWER_BUTTON_X, usPosY, MSYS_PRIORITY_HIGH, BtnGalleryFlowerButtonCallback);
		guiGalleryButton[j]->SetCursor(CURSOR_WWW);
		guiGalleryButton[j]->SetUserData(count);
		guiGalleryButton[j]->SpecifyIcon(guiFlowerImages[j], 0, 5, 5, FALSE);

		usPosY += FLOR_GALLERY_FLOWER_BUTTON_OFFSET_Y;
		count ++;
	}

	//if its the first page, display the 'back' text  in place of the 'prev' text on the top left button
	ST::string text = gubCurFlowerIndex == 0 ?
		sFloristGalleryText[FLORIST_GALLERY_HOME] :
		sFloristGalleryText[FLORIST_GALLERY_PREV];
	guiFloralGalleryButton[0]->SpecifyText(text);

	//if it is the last page disable the next button
	EnableButton(guiFloralGalleryButton[1], gubCurFlowerIndex != FLOR_GALLERY_NUMBER_FLORAL_IMAGES - 1);
}


static void DeleteFlowerButtons(void)
{
	UINT16 i;

	for(i=0; i<gubPrevNumberOfFlowers; i++)
	{
		delete guiFlowerImages[i];
	}

	UnloadButtonImage( guiGalleryButtonImage );

	for(i=0; i<gubPrevNumberOfFlowers; i++)
	{
		RemoveButton( guiGalleryButton[i] );
	}

}


static BOOLEAN DisplayFloralDescriptions(void)
{
	UINT32	uiStartLoc=0, i;
	UINT16	usPosY, usPrice;

	if( (FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex) >= 3 )
		gubCurNumberOfFlowers = 3;
	else
		gubCurNumberOfFlowers = FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex;

	usPosY = FLOR_GALLERY_FLOWER_BUTTON_Y;
	for(i=0; i<gubCurNumberOfFlowers; i++)
	{
		{
			//Display Flower title
			uiStartLoc = FLOR_GALLERY_TEXT_TOTAL_SIZE * (i + gubCurFlowerIndex);
			ST::string sTemp = GCM->loadEncryptedString(FLOR_GALLERY_TEXT_FILE, uiStartLoc, FLOR_GALLERY_TEXT_TITLE_SIZE);
			DrawTextToScreen(sTemp, FLOR_GALLERY_FLOWER_TITLE_X, usPosY + FLOR_GALLERY_FLOWER_TITLE_OFFSET_Y, 0, FLOR_GALLERY_FLOWER_TITLE_FONT, FLOR_GALLERY_FLOWER_TITLE_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}

		{
			//Display Flower Price
			ST::string sTemp;
			uiStartLoc += FLOR_GALLERY_TEXT_TITLE_SIZE;
			sTemp = GCM->loadEncryptedString(FLOR_GALLERY_TEXT_FILE, uiStartLoc, FLOR_GALLERY_TEXT_PRICE_SIZE);
			sscanf(sTemp.c_str(), "%hu", &usPrice);
			sTemp = ST::format("${}.00 {}", usPrice, pMessageStrings[MSG_USDOLLAR_ABBREVIATION]);
			DrawTextToScreen(sTemp, FLOR_GALLERY_FLOWER_TITLE_X, usPosY + FLOR_GALLERY_FLOWER_PRICE_OFFSET_Y, 0, FLOR_GALLERY_FLOWER_PRICE_FONT, FLOR_GALLERY_FLOWER_PRICE_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}

		{
			//Display Flower Desc
			uiStartLoc += FLOR_GALLERY_TEXT_PRICE_SIZE;
			ST::string sTemp = GCM->loadEncryptedString(FLOR_GALLERY_TEXT_FILE, uiStartLoc, FLOR_GALLERY_TEXT_DESC_SIZE);
			DisplayWrappedString(FLOR_GALLERY_FLOWER_TITLE_X, usPosY + FLOR_GALLERY_FLOWER_DESC_OFFSET_Y, FLOR_GALLERY_DESC_WIDTH, 2, FLOR_GALLERY_FLOWER_DESC_FONT, FLOR_GALLERY_FLOWER_DESC_COLOR, sTemp, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}

		usPosY += FLOR_GALLERY_FLOWER_BUTTON_OFFSET_Y;
	}

	return(TRUE);
}


static void ChangingFloristGallerySubPage(UINT8 ubSubPageNumber)
{
	fLoadPendingFlag = TRUE;

	//there are 3 flowers per page, 4 pages in total
	UINT8 ubPageNumber = std::min(ubSubPageNumber / 3, 3);

	if (!FloristGallerySubPagesVisitedFlag[ubPageNumber])
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = FALSE;

		FloristGallerySubPagesVisitedFlag[ubPageNumber] = TRUE;
	}
	else
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = TRUE;
	}
}
