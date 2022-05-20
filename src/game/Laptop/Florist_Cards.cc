#include "Directories.h"
#include "Laptop.h"
#include "Florist.h"
#include "Florist_Cards.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Florist_Gallery.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/string>


#define FLORIST_CARDS_SENTENCE_FONT		FONT12ARIAL
#define FLORIST_CARDS_SENTENCE_COLOR		FONT_MCOLOR_WHITE

#define FLORIST_CARD_FIRST_POS_X		LAPTOP_SCREEN_UL_X + 7
#define FLORIST_CARD_FIRST_POS_Y		LAPTOP_SCREEN_WEB_UL_Y + 72
#define FLORIST_CARD_FIRST_OFFSET_X		174
#define FLORIST_CARD_FIRST_OFFSET_Y		109

#define FLORIST_CARD_CARD_WIDTH			135
#define FLORIST_CARD_CARD_HEIGHT		100

#define FLORIST_CARD_TEXT_WIDTH			121
#define FLORIST_CARD_TEXT_HEIGHT		90

#define FLORIST_CARD_TITLE_SENTENCE_X		LAPTOP_SCREEN_UL_X
#define FLORIST_CARD_TITLE_SENTENCE_Y		LAPTOP_SCREEN_WEB_UL_Y + 53
#define FLORIST_CARD_TITLE_SENTENCE_WIDTH	LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X

#define FLORIST_CARD_BACK_BUTTON_X		LAPTOP_SCREEN_UL_X + 8
#define FLORIST_CARD_BACK_BUTTON_Y		LAPTOP_SCREEN_WEB_UL_Y + 12

#define FLORIST_CARD_
#define FLORIST_CARD_
#define FLORIST_CARD_

static SGPVObject* guiCardBackground;

INT8 gbCurrentlySelectedCard;

//link to the card gallery
static MOUSE_REGION gSelectedFloristCardsRegion[9];


static BUTTON_PICS* guiFlowerCardsButtonImage;
static GUIButtonRef guiFlowerCardsBackButton;


static void BtnFlowerCardsBackButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void SelectFloristCardsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterFloristCards()
{
	UINT16 i, j, usPosX, usPosY;
	UINT8 ubCount;

	InitFloristDefaults();

	// load the Flower Account Box graphic and add it
	guiCardBackground = AddVideoObjectFromFile(LAPTOPDIR "/cardblank.sti");

	ubCount = 0;
	usPosY = FLORIST_CARD_FIRST_POS_Y;
	for(j=0; j<3; j++)
	{
		usPosX = FLORIST_CARD_FIRST_POS_X;
		for(i=0; i<3; i++)
		{
			MSYS_DefineRegion(&gSelectedFloristCardsRegion[ubCount], usPosX, usPosY,
						(UINT16)(usPosX + FLORIST_CARD_CARD_WIDTH),
						(UINT16)(usPosY + FLORIST_CARD_CARD_HEIGHT),
						MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
						SelectFloristCardsRegionCallBack);
			MSYS_SetRegionUserData( &gSelectedFloristCardsRegion[ubCount], 0, ubCount );
			ubCount++;
			usPosX += FLORIST_CARD_FIRST_OFFSET_X;
		}
		usPosY += FLORIST_CARD_FIRST_OFFSET_Y;
	}


	guiFlowerCardsButtonImage	= LoadButtonImage(LAPTOPDIR "/floristbuttons.sti", 0, 1);

	guiFlowerCardsBackButton = CreateIconAndTextButton(guiFlowerCardsButtonImage,
								sFloristCards[FLORIST_CARDS_BACK], FLORIST_BUTTON_TEXT_FONT,
								FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
								FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
								FLORIST_CARD_BACK_BUTTON_X,
								FLORIST_CARD_BACK_BUTTON_Y, MSYS_PRIORITY_HIGH,
								BtnFlowerCardsBackButtonCallback);
	guiFlowerCardsBackButton->SetCursor(CURSOR_WWW);

	//passing the currently selected card to -1, so it is not used
	gbCurrentlySelectedCard = -1;

	RenderFloristCards();
}


void ExitFloristCards()
{
	UINT8 i;

	RemoveFloristDefaults();
	DeleteVideoObject(guiCardBackground);

	//card gallery
	for(i=0; i<9; i++)
		MSYS_RemoveRegion( &gSelectedFloristCardsRegion[i]);

	UnloadButtonImage( guiFlowerCardsButtonImage );
	RemoveButton( guiFlowerCardsBackButton );
}


void RenderFloristCards()
{
	UINT8  i,j, ubCount;
	UINT16 usPosX, usPosY;
	UINT32 uiStartLoc=0;
	UINT16 usHeightOffset;

	DisplayFloristDefaults();

	DrawTextToScreen(sFloristCards[FLORIST_CARDS_CLICK_SELECTION], FLORIST_CARD_TITLE_SENTENCE_X, FLORIST_CARD_TITLE_SENTENCE_Y, FLORIST_CARD_TITLE_SENTENCE_WIDTH, FONT10ARIAL, FLORIST_CARDS_SENTENCE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	usPosY = FLORIST_CARD_FIRST_POS_Y;
	ubCount = 0;
	for(j=0; j<3; j++)
	{
		usPosX = FLORIST_CARD_FIRST_POS_X;
		for(i=0; i<3; i++)
		{
			//The flowe account box
			BltVideoObject(FRAME_BUFFER, guiCardBackground, 0, usPosX, usPosY);

			//Get and display the card saying
			uiStartLoc = FLOR_CARD_TEXT_TITLE_SIZE * ubCount;
			ST::string sTemp = GCM->loadEncryptedString(FLOR_CARD_TEXT_FILE, uiStartLoc, FLOR_CARD_TEXT_TITLE_SIZE);

			//DisplayWrappedString(usPosX + 7, usPosY + 15, FLORIST_CARD_TEXT_WIDTH, 2, FLORIST_CARDS_SENTENCE_FONT, FLORIST_CARDS_SENTENCE_COLOR, sTemp, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
			usHeightOffset = IanWrappedStringHeight(FLORIST_CARD_TEXT_WIDTH, 2, FLORIST_CARDS_SENTENCE_FONT, sTemp);

			usHeightOffset = ( FLORIST_CARD_TEXT_HEIGHT - usHeightOffset ) / 2;

			IanDisplayWrappedString(usPosX + 7, usPosY + 10 + usHeightOffset, FLORIST_CARD_TEXT_WIDTH, 2, FLORIST_CARDS_SENTENCE_FONT, FLORIST_CARDS_SENTENCE_COLOR, sTemp, 0, 0);

			ubCount++;
			usPosX += FLORIST_CARD_FIRST_OFFSET_X;
		}
		usPosY += FLORIST_CARD_FIRST_OFFSET_Y;
	}

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectFloristCardsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_POINTER_UP)
	{
		gbCurrentlySelectedCard = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );

		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_ORDERFORM;
	}
}


static void BtnFlowerCardsBackButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_ORDERFORM;
	}
}
