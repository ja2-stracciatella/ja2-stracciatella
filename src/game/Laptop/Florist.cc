#include "Directories.h"
#include "Laptop.h"
#include "Florist.h"
#include "Florist_Order_Form.h"
#include "Florist_Gallery.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Florist_Cards.h"
#include "Text.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"


#define FLORIST_SENTENCE_FONT				FONT12ARIAL
#define FLORIST_SENTENCE_COLOR				FONT_MCOLOR_WHITE

#define FLORIST_TITLE_FONT				FONT14ARIAL
#define FLORIST_TITLE_COLOR				FONT_MCOLOR_WHITE

#define FLORIST_BACKGROUND_WIDTH			125
#define FLORIST_BACKGROUND_HEIGHT			100

#define FLORIST_BIG_TITLE_X				LAPTOP_SCREEN_UL_X + 113
#define FLORIST_BIG_TITLE_Y				LAPTOP_SCREEN_WEB_UL_Y + 0

#define FLORIST_SMALL_TITLE_X				LAPTOP_SCREEN_UL_X + 195
#define FLORIST_SMALL_TITLE_Y				LAPTOP_SCREEN_WEB_UL_Y + 0
#define FLORIST_SMALL_TITLE_WIDTH			100
#define FLORIST_SMALL_TITLE_HEIGHT			49


#define FLORIST_FIRST_BULLET_X				LAPTOP_SCREEN_UL_X + 5
#define FLORIST_FIRST_BULLET_Y				LAPTOP_SCREEN_WEB_UL_Y + 135

#define FLORIST_BULLET_OFFSET_Y				54

#define FLORIST_NUMBER_OF_BULLETS			4

#define FLORIST_GALLERY_BUTTON_X			LAPTOP_SCREEN_UL_X + 210
#define FLORIST_GALLERY_BUTTON_Y			LAPTOP_SCREEN_WEB_UL_Y + 360

#define FLORIST_FIRST_SENTENCE_COLUMN_TEXT_X		LAPTOP_SCREEN_UL_X + 53
#define FLORIST_FIRST_SENTENCE_COLUMN_TEXT_WIDTH	136

#define FLORIST_SECOND_SENTENCE_COLUMN_TEXT_X		LAPTOP_SCREEN_UL_X + 200
#define FLORIST_SECOND_SENTENCE_COLUMN_TEXT_WIDTH	300

#define FLORIST_COMPANY_INFO_TEXT_X			LAPTOP_SCREEN_UL_X + 117
#define FLORIST_COMPANY_INFO_TEXT_WIDTH			290

#define FLORIST_COMPANY_INFO_LINE_1_Y			LAPTOP_SCREEN_WEB_UL_Y + 79
#define FLORIST_COMPANY_INFO_LINE_2_Y			LAPTOP_SCREEN_WEB_UL_Y + 94
#define FLORIST_COMPANY_INFO_LINE_3_Y			LAPTOP_SCREEN_WEB_UL_Y + 107
#define FLORIST_COMPANY_INFO_LINE_4_Y			LAPTOP_SCREEN_WEB_UL_Y + 119

static SGPVObject* guiFloristBackground;
static SGPVObject* guiHandBullet;
static SGPVObject* guiLargeTitleSymbol;
static SGPVObject* guiSmallTitleSymbol;


static BOOLEAN gfHomePageActive = FALSE; // Specifies whether or not the home page or the sub pages are active

//Buttons

//Graphic for button
static BUTTON_PICS* guiGalleryButtonImage;
static void BtnGalleryButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiGalleryButton;


//link to the flower home page by clicking on the flower title
static MOUSE_REGION gSelectedFloristTitleHomeLinkRegion;


void EnterFlorist()
{
	SetBookMark( FLORIST_BOOKMARK );

	InitFloristDefaults();

	// load the handbullet graphic and add it
	guiHandBullet = AddVideoObjectFromFile(LAPTOPDIR "/handbullet.sti");

	guiGalleryButtonImage	= LoadButtonImage(LAPTOPDIR "/floristbuttons.sti", 0, 1);

	guiGalleryButton = CreateIconAndTextButton( guiGalleryButtonImage,
							sFloristText[FLORIST_GALLERY], FLORIST_BUTTON_TEXT_FONT,
							FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
							FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
							FLORIST_GALLERY_BUTTON_X, FLORIST_GALLERY_BUTTON_Y, MSYS_PRIORITY_HIGH,
							BtnGalleryButtonCallback);
	guiGalleryButton->SetCursor(CURSOR_WWW);

	//reset the currently selected card
	gbCurrentlySelectedCard = -1;

	//Initialize the Florsit Order Page (reset some variables)
	InitFloristOrderForm();

	//Initialize the flower index for the gallery page
	gubCurFlowerIndex = 0;


	RenderFlorist();
	fReDrawScreenFlag = TRUE;

	//set some variables for the order form
	InitFloristOrderFormVariables();
}


void ExitFlorist()
{
	DeleteVideoObject(guiHandBullet);
	RemoveFloristDefaults();
	UnloadButtonImage(guiGalleryButtonImage);
	RemoveButton(guiGalleryButton);
}


void RenderFlorist()
{
	UINT16 i, usPosY;
	UINT8 ubTextCounter;

	DisplayFloristDefaults();

	//compnay info
	DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_1_Y, FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT, FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_DROP_ANYWHERE],  FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_2_Y, FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT, FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_PHONE_NUMBER],   FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_3_Y, FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT, FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_STREET_ADDRESS], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DisplayWrappedString(FLORIST_COMPANY_INFO_TEXT_X, FLORIST_COMPANY_INFO_LINE_4_Y, FLORIST_COMPANY_INFO_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT, FLORIST_SENTENCE_COLOR, sFloristText[FLORIST_WWW_ADDRESS],    FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	usPosY = FLORIST_FIRST_BULLET_Y;
	ubTextCounter = FLORIST_ADVERTISEMENT_1;
	for( i=0; i<FLORIST_NUMBER_OF_BULLETS; i++)
	{
		BltVideoObject(FRAME_BUFFER, guiHandBullet, 0, FLORIST_FIRST_BULLET_X, usPosY);

		DisplayWrappedString(FLORIST_FIRST_SENTENCE_COLUMN_TEXT_X, usPosY + 20, FLORIST_FIRST_SENTENCE_COLUMN_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT, FLORIST_SENTENCE_COLOR, sFloristText[ubTextCounter], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		ubTextCounter++;

		DisplayWrappedString(FLORIST_SECOND_SENTENCE_COLUMN_TEXT_X, usPosY + 15, FLORIST_SECOND_SENTENCE_COLUMN_TEXT_WIDTH, 2, FLORIST_SENTENCE_FONT, FLORIST_SENTENCE_COLOR, sFloristText[ubTextCounter], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		ubTextCounter++;

		usPosY += FLORIST_BULLET_OFFSET_Y;
	}

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_UL_Y);
}


static void SelectFloristTitleHomeLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void InitFloristDefaults()
{
	// load the Florist background graphic and add it
	guiFloristBackground = AddVideoObjectFromFile(LAPTOPDIR "/leafback.sti");

	//if its the first page
	if( guiCurrentLaptopMode == LAPTOP_MODE_FLORIST )
	{
		// load the small title graphic and add it
		const char* const ImageFile = GetMLGFilename(MLG_LARGEFLORISTSYMBOL);
		guiLargeTitleSymbol = AddVideoObjectFromFile(ImageFile);
	}
	else
	{
		// load the leaf back graphic and add it
		const char* const ImageFile = GetMLGFilename(MLG_SMALLFLORISTSYMBOL);
		guiSmallTitleSymbol = AddVideoObjectFromFile(ImageFile);

		//flower title homepage link
		MSYS_DefineRegion(&gSelectedFloristTitleHomeLinkRegion, FLORIST_SMALL_TITLE_X, FLORIST_SMALL_TITLE_Y,
					(UINT16)(FLORIST_SMALL_TITLE_X + FLORIST_SMALL_TITLE_WIDTH),
					(UINT16)(FLORIST_SMALL_TITLE_Y + FLORIST_SMALL_TITLE_HEIGHT),
					MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
					SelectFloristTitleHomeLinkRegionCallBack );
	}
}


void DisplayFloristDefaults()
{
	WebPageTileBackground(4, 4, FLORIST_BACKGROUND_WIDTH, FLORIST_BACKGROUND_HEIGHT, guiFloristBackground);

	//if its the first page
	if( guiCurrentLaptopMode == LAPTOP_MODE_FLORIST )
	{
		gfHomePageActive = TRUE;
		BltVideoObject(FRAME_BUFFER, guiLargeTitleSymbol, 0, FLORIST_BIG_TITLE_X, FLORIST_BIG_TITLE_Y);
	}
	else
	{
		gfHomePageActive = FALSE;
		BltVideoObject(FRAME_BUFFER, guiSmallTitleSymbol, 0, FLORIST_SMALL_TITLE_X, FLORIST_SMALL_TITLE_Y);
	}
}

void RemoveFloristDefaults()
{
	DeleteVideoObject(guiFloristBackground);

	//if its the first page
	if( gfHomePageActive )
	{
		//delete the big title
		DeleteVideoObject(guiLargeTitleSymbol);
	}
	else
	{
		//delete the little title
		DeleteVideoObject(guiSmallTitleSymbol);
		MSYS_RemoveRegion(&gSelectedFloristTitleHomeLinkRegion);
	}
}


static void BtnGalleryButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_FLOWER_GALLERY;
	}
}


static void SelectFloristTitleHomeLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
	}
}
