#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Laptop.h"
#include "Florist.h"
#include "Florist_Order_Form.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Florist_Gallery.h"
#include "Florist_Cards.h"
#include "Text_Input.h"
#include "Finances.h"
#include "Game_Clock.h"
#include "Text.h"
#include "LaptopSave.h"
#include "Random.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "Meanwhile.h"
#include "StrategicMap.h"
#include "MapScreen.h"
#include "FontCompat.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "ShippingDestinationModel.h"

#include <string_theory/format>
#include <string_theory/string>


#define FLOWER_ORDEER_TINY_FONT				FONT10ARIAL
#define FLOWER_ORDEER_SMALL_FONT			FONT12ARIAL
#define FLOWER_ORDEER_BIG_FONT				FONT12ARIAL
#define FLOWER_ORDEER_SMALL_COLOR			FONT_MCOLOR_WHITE
#define FLOWER_ORDEER_LINK_COLOR			FONT_MCOLOR_LTYELLOW
#define FLOWER_ORDEER_DROP_DOWN_FONT			FONT12ARIAL
#define FLOWER_ORDEER_DROP_DOWN_COLOR			FONT_MCOLOR_WHITE

#define FLOWER_ORDER_STATIC_TEXT_COLOR			76

#define FLOWER_ORDER_FLOWER_BOX_X			LAPTOP_SCREEN_UL_X + 7
#define FLOWER_ORDER_FLOWER_BOX_Y			LAPTOP_SCREEN_WEB_UL_Y + 63
#define FLOWER_ORDER_FLOWER_BOX_WIDTH			75
#define FLOWER_ORDER_FLOWER_BOX_HEIGHT			100


#define FLOWER_ORDER_SENTIMENT_BOX_X			LAPTOP_SCREEN_UL_X + 14
#define FLOWER_ORDER_SENTIMENT_BOX_Y			LAPTOP_SCREEN_WEB_UL_Y + 226

#define FLOWER_ORDER_NAME_BOX_X				LAPTOP_SCREEN_UL_X + 60
#define FLOWER_ORDER_NAME_BOX_Y				LAPTOP_SCREEN_WEB_UL_Y + 314 - FLOWER_ORDER_SMALLER_PS_OFFSET_Y

#define FLOWER_ORDER_SMALLER_PS_OFFSET_Y		27


#define FLOWER_ORDER_DELIVERY_LOCATION_X		LAPTOP_SCREEN_UL_X + 205
#define FLOWER_ORDER_DELIVERY_LOCATION_Y		LAPTOP_SCREEN_WEB_UL_Y + 143
#define FLOWER_ORDER_DELIVERY_LOCATION_WIDTH		252
#define FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT		20

#define FLOWER_ORDER_BACK_BUTTON_X			LAPTOP_SCREEN_UL_X + 8
#define FLOWER_ORDER_BACK_BUTTON_Y			LAPTOP_SCREEN_WEB_UL_Y + 12

#define FLOWER_ORDER_SEND_BUTTON_X			LAPTOP_SCREEN_UL_X + 124
#define FLOWER_ORDER_SEND_BUTTON_Y			LAPTOP_SCREEN_WEB_UL_Y + 364

#define FLOWER_ORDER_CLEAR_BUTTON_X			LAPTOP_SCREEN_UL_X + 215
#define FLOWER_ORDER_CLEAR_BUTTON_Y			FLOWER_ORDER_SEND_BUTTON_Y

#define FLOWER_ORDER_GALLERY_BUTTON_X			LAPTOP_SCREEN_UL_X + 305
#define FLOWER_ORDER_GALLERY_BUTTON_Y			FLOWER_ORDER_SEND_BUTTON_Y

#define FLOWER_ORDER_FLOWER_NAME_X			LAPTOP_SCREEN_UL_X + 94
#define FLOWER_ORDER_FLOWER_NAME_Y			LAPTOP_SCREEN_WEB_UL_Y + 68

#define FLOWER_ORDER_BOUQUET_NAME_X			FLOWER_ORDER_FLOWER_NAME_X
#define FLOWER_ORDER_BOUQUET_NAME_Y			FLOWER_ORDER_ORDER_NUM_NAME_Y + 15//FLOWER_ORDER_FLOWER_NAME_Y + 15

#define FLOWER_ORDER_ORDER_NUM_NAME_X			FLOWER_ORDER_BOUQUET_NAME_X
#define FLOWER_ORDER_ORDER_NUM_NAME_Y			FLOWER_ORDER_FLOWER_NAME_Y + 15//FLOWER_ORDER_BOUQUET_NAME_Y + 15

#define FLOWER_ORDER_DATE_X				FLOWER_ORDER_FLOWER_NAME_X
#define FLOWER_ORDER_DATE_Y				LAPTOP_SCREEN_WEB_UL_Y + 126

#define FLOWER_ORDER_LOCATION_X				FLOWER_ORDER_FLOWER_NAME_X
#define FLOWER_ORDER_LOCATION_Y				FLOWER_ORDER_DELIVERY_LOCATION_Y + 4

#define FLOWER_ORDER_ADDITIONAL_SERVICES_X		FLOWER_ORDER_FLOWER_BOX_X
#define FLOWER_ORDER_ADDITIONAL_SERVICES_Y		LAPTOP_SCREEN_WEB_UL_Y + 167

#define FLOWER_ORDER_PERSONAL_SENT_TEXT_X		FLOWER_ORDER_ADDITIONAL_SERVICES_X
#define FLOWER_ORDER_PERSONAL_SENT_TEXT_Y		LAPTOP_SCREEN_WEB_UL_Y + 212

#define FLOWER_ORDER_PERSONAL_SENT_BOX_X		FLOWER_ORDER_SENTIMENT_BOX_X + 5
#define FLOWER_ORDER_PERSONAL_SENT_BOX_Y		FLOWER_ORDER_SENTIMENT_BOX_Y + 5
#define FLOWER_ORDER_PERSONAL_SENT_TEXT_WIDTH		457
#define FLOWER_ORDER_PERSONAL_SENT_TEXT_HEIGHT		17//44

#define FLOWER_ORDER_BILLING_INFO_X			FLOWER_ORDER_ADDITIONAL_SERVICES_X
#define FLOWER_ORDER_BILLING_INFO_Y			LAPTOP_SCREEN_WEB_UL_Y + 296 - FLOWER_ORDER_SMALLER_PS_OFFSET_Y

#define FLOWER_ORDER_NAME_TEXT_X			FLOWER_ORDER_ADDITIONAL_SERVICES_X
#define FLOWER_ORDER_NAME_TEXT_Y			FLOWER_ORDER_NAME_BOX_Y+4
#define FLOWER_ORDER_NAME_TEXT_WIDTH			50

#define FLOWER_ORDER_NAME_TEXT_BOX_X			FLOWER_ORDER_NAME_BOX_X + 3
#define FLOWER_ORDER_NAME_TEXT_BOX_Y			FLOWER_ORDER_NAME_BOX_Y + 3
#define FLOWER_ORDER_NAME_TEXT_BOX_WIDTH		257
#define FLOWER_ORDER_NAME_TEXT_BOX_HEIGHT		15

#define FLOWER_ORDER_CHECK_WIDTH			20
#define FLOWER_ORDER_CHECK_HEIGHT			17

#define FLOWER_ORDER_CHECK_BOX_0_X			LAPTOP_SCREEN_UL_X + 186
#define FLOWER_ORDER_CHECK_BOX_0_Y			FLOWER_ORDER_DATE_Y	- 3

#define FLOWER_ORDER_CHECK_BOX_1_X			LAPTOP_SCREEN_UL_X + 270
#define FLOWER_ORDER_CHECK_BOX_1_Y			FLOWER_ORDER_CHECK_BOX_0_Y

#define FLOWER_ORDER_CHECK_BOX_2_X			LAPTOP_SCREEN_UL_X + 123
#define FLOWER_ORDER_CHECK_BOX_2_Y			FLOWER_ORDER_ADDITIONAL_SERVICES_Y

#define FLOWER_ORDER_CHECK_BOX_3_X			LAPTOP_SCREEN_UL_X + 269
#define FLOWER_ORDER_CHECK_BOX_3_Y			FLOWER_ORDER_CHECK_BOX_2_Y

#define FLOWER_ORDER_CHECK_BOX_4_X			FLOWER_ORDER_CHECK_BOX_2_X
#define FLOWER_ORDER_CHECK_BOX_4_Y			FLOWER_ORDER_CHECK_BOX_2_Y + 25

#define FLOWER_ORDER_CHECK_BOX_5_X			FLOWER_ORDER_CHECK_BOX_3_X
#define FLOWER_ORDER_CHECK_BOX_5_Y			FLOWER_ORDER_CHECK_BOX_4_Y

#define FLOWER_ORDER_LINK_TO_CARD_GALLERY_X		LAPTOP_SCREEN_UL_X + 190
#define FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y		LAPTOP_SCREEN_WEB_UL_Y + 284 - FLOWER_ORDER_SMALLER_PS_OFFSET_Y

#define FLOWER_ORDER_DROP_DOWN_LOCATION_X		FLOWER_ORDER_DELIVERY_LOCATION_X
#define FLOWER_ORDER_DROP_DOWN_LOCATION_Y		FLOWER_ORDER_DELIVERY_LOCATION_Y + 19
#define FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH		230

#define FLOWER_ORDER_DROP_DOWN_CITY_START_X		FLOWER_ORDER_DROP_DOWN_LOCATION_X + 5
#define FLOWER_ORDER_DROP_DOWN_CITY_START_Y		FLOWER_ORDER_DROP_DOWN_LOCATION_Y + 3

#define FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS	75
#define FLOWER_ORDER_NAME_FIELD_NUM_CHARS		35

// Checkbox positions.
// They will be initialized at run time when we know
// the screen size.
static SGPPoint g_order_check_box_pos[6];

static SGPVObject* guiDeliveryLocation;
static SGPVObject* guiFlowerFrame;
static SGPVObject* guiCurrentlySelectedFlowerImage;
static SGPVObject* guiNameBox;
static SGPVObject* guiPersonalSentiments;
static SGPVObject* guiFlowerOrderCheckBoxButtonImage;
static SGPVObject* guiDropDownBorder;


static BOOLEAN gfFLoristCheckBox0Down = FALSE; // next day delviery
static BOOLEAN gfFLoristCheckBox1Down = TRUE;  // when it gets there delivery
static BOOLEAN gfFLoristCheckBox2Down = FALSE;
static BOOLEAN gfFLoristCheckBox3Down = FALSE;
static BOOLEAN gfFLoristCheckBox4Down = FALSE;
static BOOLEAN gfFLoristCheckBox5Down = FALSE;

static UINT32 guiFlowerPrice;

//drop down menu
enum
{
	FLOWER_ORDER_DROP_DOWN_NO_ACTION,
	FLOWER_ORDER_DROP_DOWN_CREATE,
	FLOWER_ORDER_DROP_DOWN_DESTROY,
	FLOWER_ORDER_DROP_DOWN_DISPLAY,
};
//the current mode of the drop down display
static UINT8 gubFlowerDestDropDownMode;
static UINT8 gubCurrentlySelectedFlowerLocation;


static ST::string gsSentimentTextField;
static ST::string gsNameTextField;


//buttons
static BUTTON_PICS* guiFlowerOrderButtonImage;

static UINT8 const gubFlowerOrder_AdditioanalServicePrices[] = { 10, 20, 10, 10 };

static void BtnFlowerOrderBackButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiFlowerOrderBackButton;

static void BtnFlowerOrderSendButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiFlowerOrderSendButton;

static void BtnFlowerOrderClearButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiFlowerOrderClearButton;

static void BtnFlowerOrderGalleryButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiFlowerOrderGalleryButton;


//Clicking on OrderCheckBox
static MOUSE_REGION gSelectedFloristCheckBoxRegion[6];

//link to the card gallery
static MOUSE_REGION gSelectedFloristCardGalleryLinkRegion;

//link to the flower gallery by clicking on the flower
static MOUSE_REGION gSelectedFloristGalleryLinkRegion;


//the drop down for the city
static MOUSE_REGION gSelectedFloristDropDownRegion;

//to disable the drop down window
static MOUSE_REGION gSelectedFloristDisableDropDownRegion;


//mouse region for the drop down city location area
static std::vector<MOUSE_REGION> gSelectedFlowerDropDownRegion;


static GUIButtonRef MakeButton(const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	const INT16 shadow_col = FLORIST_BUTTON_TEXT_SHADOW_COLOR;
	GUIButtonRef const btn = CreateIconAndTextButton(guiFlowerOrderButtonImage, text, FLORIST_BUTTON_TEXT_FONT, FLORIST_BUTTON_TEXT_UP_COLOR, shadow_col, FLORIST_BUTTON_TEXT_DOWN_COLOR, shadow_col, x, y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_WWW);
	return btn;
}


static void InitFlowerOrderTextInputBoxes(void);
static void SelectFloristCardGalleryLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectFloristDisableDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectFloristDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectFloristGalleryLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectFlorsitCheckBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterFloristOrderForm()
{
	InitFloristDefaults();

	// runtime initialization of the checkbox positions
	g_order_check_box_pos[0].set(FLOWER_ORDER_CHECK_BOX_0_X, FLOWER_ORDER_CHECK_BOX_0_Y);
	g_order_check_box_pos[1].set(FLOWER_ORDER_CHECK_BOX_1_X, FLOWER_ORDER_CHECK_BOX_1_Y);
	g_order_check_box_pos[2].set(FLOWER_ORDER_CHECK_BOX_2_X, FLOWER_ORDER_CHECK_BOX_2_Y);
	g_order_check_box_pos[3].set(FLOWER_ORDER_CHECK_BOX_3_X, FLOWER_ORDER_CHECK_BOX_3_Y);
	g_order_check_box_pos[4].set(FLOWER_ORDER_CHECK_BOX_4_X, FLOWER_ORDER_CHECK_BOX_4_Y);
	g_order_check_box_pos[5].set(FLOWER_ORDER_CHECK_BOX_5_X, FLOWER_ORDER_CHECK_BOX_5_Y);

	guiDeliveryLocation               = AddVideoObjectFromFile(LAPTOPDIR "/deliverylocation.sti");
	guiFlowerFrame                    = AddVideoObjectFromFile(LAPTOPDIR "/flowerframe.sti");
	guiPersonalSentiments             = AddVideoObjectFromFile(LAPTOPDIR "/personalsentiments.sti");
	guiNameBox                        = AddVideoObjectFromFile(LAPTOPDIR "/namebox.sti");
	guiFlowerOrderCheckBoxButtonImage = AddVideoObjectFromFile(LAPTOPDIR "/ordercheckbox.sti");

	// load the currently selected flower bouquet
	ST::string sTemp = ST::format(LAPTOPDIR "/flower_{}.sti", guiCurrentlySelectedFlower);
	guiCurrentlySelectedFlowerImage = AddVideoObjectFromFile(sTemp);

	guiDropDownBorder = AddVideoObjectFromFile(INTERFACEDIR "/tactpopup.sti");

	guiFlowerOrderButtonImage   = LoadButtonImage(LAPTOPDIR "/floristbuttons.sti", 0, 1);
	guiFlowerOrderBackButton    = MakeButton(sOrderFormText[FLORIST_ORDER_BACK],    FLOWER_ORDER_BACK_BUTTON_X,    FLOWER_ORDER_BACK_BUTTON_Y,    BtnFlowerOrderBackButtonCallback);
	guiFlowerOrderSendButton    = MakeButton(sOrderFormText[FLORIST_ORDER_SEND],    FLOWER_ORDER_SEND_BUTTON_X,    FLOWER_ORDER_SEND_BUTTON_Y,    BtnFlowerOrderSendButtonCallback);
	guiFlowerOrderClearButton   = MakeButton(sOrderFormText[FLORIST_ORDER_CLEAR],   FLOWER_ORDER_CLEAR_BUTTON_X,   FLOWER_ORDER_CLEAR_BUTTON_Y,   BtnFlowerOrderClearButtonCallback);
	guiFlowerOrderGalleryButton = MakeButton(sOrderFormText[FLORIST_ORDER_GALLERY], FLOWER_ORDER_GALLERY_BUTTON_X, FLOWER_ORDER_GALLERY_BUTTON_Y, BtnFlowerOrderGalleryButtonCallback);

	// The check box mouse regions
	for (UINT32 i = 0; i < lengthof(g_order_check_box_pos); ++i)
	{
		MOUSE_REGION*   const r  = &gSelectedFloristCheckBoxRegion[i];
		SGPPoint const* const xy = &g_order_check_box_pos[i];
		MSYS_DefineRegion(r, xy->iX, xy->iY, xy->iX + FLOWER_ORDER_CHECK_WIDTH, xy->iY + FLOWER_ORDER_CHECK_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectFlorsitCheckBoxRegionCallBack);
		MSYS_SetRegionUserData(r, 0, i);
	}

	UINT16 const x = StringPixLength(sOrderFormText[FLORIST_ORDER_SELECT_FROM_OURS],   FLOWER_ORDEER_SMALL_FONT) + 2 + FLOWER_ORDER_LINK_TO_CARD_GALLERY_X;
	UINT16 const w = StringPixLength(sOrderFormText[FLORIST_ORDER_STANDARDIZED_CARDS], FLOWER_ORDEER_SMALL_FONT);
	UINT16 const h = GetFontHeight(FLOWER_ORDEER_SMALL_FONT);
	MSYS_DefineRegion(&gSelectedFloristCardGalleryLinkRegion, x, FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y, x + w, FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y + h, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristCardGalleryLinkRegionCallBack);

	//flower link
	MSYS_DefineRegion(&gSelectedFloristGalleryLinkRegion, FLOWER_ORDER_FLOWER_BOX_X, FLOWER_ORDER_FLOWER_BOX_Y, FLOWER_ORDER_FLOWER_BOX_X + FLOWER_ORDER_FLOWER_BOX_WIDTH, FLOWER_ORDER_FLOWER_BOX_Y + FLOWER_ORDER_FLOWER_BOX_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristGalleryLinkRegionCallBack);

	//drop down city location
	MSYS_DefineRegion(&gSelectedFloristDropDownRegion, FLOWER_ORDER_DELIVERY_LOCATION_X, FLOWER_ORDER_DELIVERY_LOCATION_Y, FLOWER_ORDER_DELIVERY_LOCATION_X + FLOWER_ORDER_DELIVERY_LOCATION_WIDTH, FLOWER_ORDER_DELIVERY_LOCATION_Y + FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectFloristDropDownRegionCallBack);

	//to disable the drop down city location
	MSYS_DefineRegion(&gSelectedFloristDisableDropDownRegion, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH + 2, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, SelectFloristDisableDropDownRegionCallBack);
	gSelectedFloristDisableDropDownRegion.Disable();

	InitFlowerOrderTextInputBoxes();

	LaptopSaveInfo.uiFlowerOrderNumber += Random(5) + 1;

	RenderFloristOrderForm();
}


void InitFloristOrderFormVariables()
{
	guiFlowerPrice = 0;
	gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_NO_ACTION;
	gubCurrentlySelectedFlowerLocation = 0;
}


static void DestroyFlowerOrderTextInputBoxes(void);


void ExitFloristOrderForm()
{
	UINT8	i;
	RemoveFloristDefaults();

	DeleteVideoObject(guiDeliveryLocation);
	DeleteVideoObject(guiFlowerFrame);
	DeleteVideoObject(guiNameBox);
	DeleteVideoObject(guiPersonalSentiments);
	DeleteVideoObject(guiFlowerOrderCheckBoxButtonImage);
	DeleteVideoObject(guiCurrentlySelectedFlowerImage);
	DeleteVideoObject(guiDropDownBorder);

	for(i=0; i<6; i++)
		MSYS_RemoveRegion( &gSelectedFloristCheckBoxRegion[i]);

	//card gallery link
	MSYS_RemoveRegion( &gSelectedFloristCardGalleryLinkRegion);

	//flower link
	MSYS_RemoveRegion( &gSelectedFloristGalleryLinkRegion);

	//flower link
	MSYS_RemoveRegion( &gSelectedFloristDropDownRegion);

	//disable the drop down window
	MSYS_RemoveRegion( &gSelectedFloristDisableDropDownRegion);

	UnloadButtonImage( guiFlowerOrderButtonImage );

	RemoveButton( guiFlowerOrderBackButton );
	RemoveButton( guiFlowerOrderSendButton );
	RemoveButton( guiFlowerOrderClearButton );
	RemoveButton( guiFlowerOrderGalleryButton );

	//Store the text fields
	gsSentimentTextField = GetStringFromField(1);
	gsNameTextField = GetStringFromField(2);
	gbCurrentlySelectedCard = -1;


	DestroyFlowerOrderTextInputBoxes();
}


static BOOLEAN CreateDestroyFlowerOrderDestDropDown(UINT8 ubDropDownMode);
static void HandleFloristOrderKeyBoardInput(void);


void HandleFloristOrderForm()
{
	if( gubFlowerDestDropDownMode != FLOWER_ORDER_DROP_DOWN_NO_ACTION )
	{
		CreateDestroyFlowerOrderDestDropDown( gubFlowerDestDropDownMode );
	}
	HandleFloristOrderKeyBoardInput();

	RenderAllTextFields();
}


static void DisplayFloristCheckBox(void);
static void DisplayFlowerDynamicItems(void);
static void FlowerOrderDisplayShippingLocationCity(void);


void RenderFloristOrderForm()
{
	UINT16 usPosX;
	UINT32	uiStartLoc=0;

	DisplayFloristDefaults();

	BltVideoObject(FRAME_BUFFER, guiDeliveryLocation,             0, FLOWER_ORDER_DELIVERY_LOCATION_X, FLOWER_ORDER_DELIVERY_LOCATION_Y);
	BltVideoObject(FRAME_BUFFER, guiFlowerFrame,                  0, FLOWER_ORDER_FLOWER_BOX_X,        FLOWER_ORDER_FLOWER_BOX_Y);
	BltVideoObject(FRAME_BUFFER, guiCurrentlySelectedFlowerImage, 0, FLOWER_ORDER_FLOWER_BOX_X + 5,    FLOWER_ORDER_FLOWER_BOX_Y + 5);
	BltVideoObject(FRAME_BUFFER, guiNameBox,                      0, FLOWER_ORDER_NAME_BOX_X,          FLOWER_ORDER_NAME_BOX_Y);
	BltVideoObject(FRAME_BUFFER, guiPersonalSentiments,           0, FLOWER_ORDER_SENTIMENT_BOX_X,     FLOWER_ORDER_SENTIMENT_BOX_Y);

	//Bouquet name, price and order number,text
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_NAME_BOUQUET], FLOWER_ORDER_FLOWER_NAME_X,    FLOWER_ORDER_FLOWER_NAME_Y,    0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_PRICE],        FLOWER_ORDER_BOUQUET_NAME_X,   FLOWER_ORDER_BOUQUET_NAME_Y,   0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_ORDER_NUMBER], FLOWER_ORDER_ORDER_NUM_NAME_X, FLOWER_ORDER_ORDER_NUM_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//The flower name
	usPosX = StringPixLength( sOrderFormText[FLORIST_ORDER_NAME_BOUQUET], FLOWER_ORDEER_SMALL_FONT) + 5 + FLOWER_ORDER_FLOWER_NAME_X;
	uiStartLoc = FLOR_GALLERY_TEXT_TOTAL_SIZE * guiCurrentlySelectedFlower;
	ST::string sTemp = GCM->loadEncryptedString(FLOR_GALLERY_TEXT_FILE, uiStartLoc, FLOR_GALLERY_TEXT_TITLE_SIZE);
	DrawTextToScreen(sTemp, usPosX, FLOWER_ORDER_FLOWER_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


	//Deliverry Date
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_DELIVERY_DATE], FLOWER_ORDER_ORDER_NUM_NAME_X, FLOWER_ORDER_DATE_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Next day
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_NEXT_DAY], FLOWER_ORDER_CHECK_BOX_0_X + FLOWER_ORDER_CHECK_WIDTH + 3, FLOWER_ORDER_CHECK_BOX_0_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//When it get there
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_GETS_THERE], FLOWER_ORDER_CHECK_BOX_1_X + FLOWER_ORDER_CHECK_WIDTH + 3, FLOWER_ORDER_CHECK_BOX_1_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Deliverry locatiuon
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_DELIVERY_LOCATION], FLOWER_ORDER_ORDER_NUM_NAME_X, FLOWER_ORDER_LOCATION_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Additional services
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_ADDITIONAL_CHARGES], FLOWER_ORDER_ADDITIONAL_SERVICES_X, FLOWER_ORDER_ADDITIONAL_SERVICES_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//crushed bouquet
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_CRUSHED], FLOWER_ORDER_CHECK_BOX_2_X + FLOWER_ORDER_CHECK_WIDTH + 3, FLOWER_ORDER_CHECK_BOX_2_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//black roses
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_BLACK_ROSES], FLOWER_ORDER_CHECK_BOX_3_X + FLOWER_ORDER_CHECK_WIDTH + 3, FLOWER_ORDER_CHECK_BOX_3_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//wilted bouquet
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_WILTED], FLOWER_ORDER_CHECK_BOX_4_X + FLOWER_ORDER_CHECK_WIDTH + 3, FLOWER_ORDER_CHECK_BOX_4_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//fruit cake
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_FRUIT_CAKE], FLOWER_ORDER_CHECK_BOX_5_X + FLOWER_ORDER_CHECK_WIDTH + 3, FLOWER_ORDER_CHECK_BOX_5_Y + 2, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


	//Personal sentiment
	usPosX = FLOWER_ORDER_PERSONAL_SENT_TEXT_X + StringPixLength( sOrderFormText[FLORIST_ORDER_PERSONAL_SENTIMENTS], FLOWER_ORDEER_BIG_FONT) + 5;
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_PERSONAL_SENTIMENTS], FLOWER_ORDER_PERSONAL_SENT_TEXT_X, FLOWER_ORDER_PERSONAL_SENT_TEXT_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_CARD_LENGTH], usPosX, FLOWER_ORDER_PERSONAL_SENT_TEXT_Y +2, 0, FLOWER_ORDEER_TINY_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Billing information
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_BILLING_INFO], FLOWER_ORDER_BILLING_INFO_X, FLOWER_ORDER_BILLING_INFO_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Billing Name
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_NAME], FLOWER_ORDER_NAME_TEXT_X, FLOWER_ORDER_NAME_TEXT_Y, FLOWER_ORDER_NAME_TEXT_WIDTH, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	//the text to link to the card gallery
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_SELECT_FROM_OURS], FLOWER_ORDER_LINK_TO_CARD_GALLERY_X, FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosX = StringPixLength( sOrderFormText[FLORIST_ORDER_SELECT_FROM_OURS], FLOWER_ORDEER_SMALL_FONT) + 5 + FLOWER_ORDER_LINK_TO_CARD_GALLERY_X;
	DrawTextToScreen(sOrderFormText[FLORIST_ORDER_STANDARDIZED_CARDS], usPosX, FLOWER_ORDER_LINK_TO_CARD_GALLERY_Y, 0, FLOWER_ORDEER_BIG_FONT, FLOWER_ORDEER_LINK_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	DisplayFloristCheckBox();

	//display all the things that change for the different bouquet collections
	DisplayFlowerDynamicItems();

	//Display the currently selected city
	FlowerOrderDisplayShippingLocationCity();

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnFlowerOrderBackButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_FLOWER_GALLERY;
	}
}


static void BtnFlowerOrderSendButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//add an entry in the finacial page for the medical deposit
		AddTransactionToPlayersBook(PURCHASED_FLOWERS, 0, GetWorldTotalMin(), -(INT32)guiFlowerPrice);

		auto destination = GCM->getShippingDestination(gubCurrentlySelectedFlowerLocation);
		if (GetTownIdForSector(destination->getDeliverySector()) == MEDUNA)
		{
			// sent to meduna!
			if (gfFLoristCheckBox0Down)
			{
				HandleFlowersMeanwhileScene(0);
			}
			else
			{
				HandleFlowersMeanwhileScene(1);
			}
		}

		//increment the order number
		LaptopSaveInfo.uiFlowerOrderNumber += 1 + Random(2);

		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
		InitFloristOrderForm();
	}
}


static void BtnFlowerOrderClearButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
		InitFloristOrderForm();
	}
}


static void BtnFlowerOrderGalleryButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_FLOWER_GALLERY;
		//reset the gallery back to page 0
		gubCurFlowerIndex = 0;
	}
}


static void SelectFlorsitCheckBoxRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT32	uiUserData;

		uiUserData = MSYS_GetRegionUserData( pRegion, 0 );

		switch( uiUserData )
		{
			case 0:
				if( gfFLoristCheckBox0Down )
				{
					gfFLoristCheckBox0Down = FALSE;
					gfFLoristCheckBox1Down = TRUE;
				}
				else
				{
					gfFLoristCheckBox0Down = TRUE;
					gfFLoristCheckBox1Down = FALSE;
				}
				break;
			case 1:
				if( gfFLoristCheckBox1Down )
				{
					gfFLoristCheckBox1Down = FALSE;
					gfFLoristCheckBox0Down = TRUE;
				}
				else
				{
					gfFLoristCheckBox1Down = TRUE;
					gfFLoristCheckBox0Down = FALSE;
				}
				break;
			case 2:
				if( gfFLoristCheckBox2Down )
					gfFLoristCheckBox2Down = FALSE;
				else
					gfFLoristCheckBox2Down = TRUE;
				break;
			case 3:
				if( gfFLoristCheckBox3Down )
					gfFLoristCheckBox3Down = FALSE;
				else
					gfFLoristCheckBox3Down = TRUE;
				break;
			case 4:
				if( gfFLoristCheckBox4Down )
					gfFLoristCheckBox4Down = FALSE;
				else
					gfFLoristCheckBox4Down = TRUE;
				break;
			case 5:
				if( gfFLoristCheckBox5Down )
					gfFLoristCheckBox5Down = FALSE;
				else
					gfFLoristCheckBox5Down = TRUE;
				break;
		}
		DisplayFloristCheckBox();
		fPausedReDrawScreenFlag = TRUE;
	}
}


static void DisplayFloristCheckBox(void)
{
	BltVideoObject(FRAME_BUFFER, guiFlowerOrderCheckBoxButtonImage, gfFLoristCheckBox0Down ? 1 : 0, FLOWER_ORDER_CHECK_BOX_0_X, FLOWER_ORDER_CHECK_BOX_0_Y);
	BltVideoObject(FRAME_BUFFER, guiFlowerOrderCheckBoxButtonImage, gfFLoristCheckBox1Down ? 1 : 0, FLOWER_ORDER_CHECK_BOX_1_X, FLOWER_ORDER_CHECK_BOX_1_Y);
	BltVideoObject(FRAME_BUFFER, guiFlowerOrderCheckBoxButtonImage, gfFLoristCheckBox2Down ? 1 : 0, FLOWER_ORDER_CHECK_BOX_2_X, FLOWER_ORDER_CHECK_BOX_2_Y);
	BltVideoObject(FRAME_BUFFER, guiFlowerOrderCheckBoxButtonImage, gfFLoristCheckBox3Down ? 1 : 0, FLOWER_ORDER_CHECK_BOX_3_X, FLOWER_ORDER_CHECK_BOX_3_Y);
	BltVideoObject(FRAME_BUFFER, guiFlowerOrderCheckBoxButtonImage, gfFLoristCheckBox4Down ? 1 : 0, FLOWER_ORDER_CHECK_BOX_4_X, FLOWER_ORDER_CHECK_BOX_4_Y);
	BltVideoObject(FRAME_BUFFER, guiFlowerOrderCheckBoxButtonImage, gfFLoristCheckBox5Down ? 1 : 0, FLOWER_ORDER_CHECK_BOX_5_X, FLOWER_ORDER_CHECK_BOX_5_Y);

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectFloristCardGalleryLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_CARD_GALLERY;
	}
}


//display the things that change on the screen
static void DisplayFlowerDynamicItems(void)
{
	UINT32	uiStartLoc=0;
	UINT16	usPosX;
	ST::string sTemp;
	UINT16	usPrice;

	//order number
	usPosX = StringPixLength( sOrderFormText[FLORIST_ORDER_ORDER_NUMBER], FLOWER_ORDEER_SMALL_FONT) + 5 + FLOWER_ORDER_ORDER_NUM_NAME_X;
	sTemp = ST::format("{}", LaptopSaveInfo.uiFlowerOrderNumber);
	DrawTextToScreen(sTemp, usPosX, FLOWER_ORDER_ORDER_NUM_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	guiFlowerPrice = 0;
	// the user selected crushed bouquet
	if( gfFLoristCheckBox2Down )
		guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[0];

	// the user selected blak roses
	if( gfFLoristCheckBox3Down )
		guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[1];

	// the user selected wilted bouquet
	if( gfFLoristCheckBox4Down )
		guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[2];

	// the user selected fruit cake
	if( gfFLoristCheckBox5Down )
		guiFlowerPrice += gubFlowerOrder_AdditioanalServicePrices[3];

	//price
	usPosX = StringPixLength( sOrderFormText[FLORIST_ORDER_PRICE], FLOWER_ORDEER_SMALL_FONT) + 5 + FLOWER_ORDER_BOUQUET_NAME_X;
	uiStartLoc = FLOR_GALLERY_TEXT_TOTAL_SIZE * guiCurrentlySelectedFlower + FLOR_GALLERY_TEXT_TITLE_SIZE;
	sTemp = GCM->loadEncryptedString(FLOR_GALLERY_TEXT_FILE, uiStartLoc, FLOR_GALLERY_TEXT_PRICE_SIZE);
	sscanf(sTemp.c_str(), "%hu", &usPrice);

	//if its the next day delivery
	auto destination = GCM->getShippingDestination(gubCurrentlySelectedFlowerLocation);
	if( gfFLoristCheckBox0Down )
		guiFlowerPrice += usPrice + destination->flowersNextDayDeliveryCost;
	//else its the 'when it gets there' delivery
	else
		guiFlowerPrice += usPrice + destination->flowersWhenItGetsThereCost;

	sTemp = ST::format("${}.00 {}", guiFlowerPrice, pMessageStrings[MSG_USDOLLAR_ABBREVIATION]);
	DrawTextToScreen(sTemp, usPosX, FLOWER_ORDER_BOUQUET_NAME_Y, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
}


static void SelectFloristGalleryLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_FLOWER_GALLERY;
	}
}


static void SelectFloristDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_CREATE;
	}
}


static void SelectFloristDisableDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_DESTROY;
	}
}


static void FlowerOrderDrawSelectedCity(UINT8 ubNumber);


static void SelectFlowerDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubCurrentlySelectedFlowerLocation = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		FlowerOrderDrawSelectedCity( gubCurrentlySelectedFlowerLocation );
		gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_DESTROY;
	}
}


static void SelectFlowerDropDownMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		gubCurrentlySelectedFlowerLocation = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		FlowerOrderDrawSelectedCity( gubCurrentlySelectedFlowerLocation );

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
}


static void GetInputText()
{
	// Get the current text from the text box
	UINT8 const text_field_id = GetActiveFieldID();
	if (text_field_id == 1)
	{ // The personel sentiment field
		gsSentimentTextField = GetStringFromField(text_field_id);
	}
	else if (text_field_id == 2)
	{ // The name field
		gsNameTextField = GetStringFromField(text_field_id);
	}

	SetActiveField(0);
}


static BOOLEAN CreateDestroyFlowerOrderDestDropDown(UINT8 ubDropDownMode)
{
	static UINT16 usHeight;
	static BOOLEAN fMouseRegionsCreated=FALSE;

	switch( ubDropDownMode )
	{
		case 	FLOWER_ORDER_DROP_DOWN_NO_ACTION:
		{

		}
			break;

		case 	FLOWER_ORDER_DROP_DOWN_CREATE:
		{
			UINT8 i;
			UINT16 usPosX, usPosY;
			UINT16 usTemp;
			UINT16 usFontHeight = GetFontHeight( FLOWER_ORDEER_DROP_DOWN_FONT );

			if( fMouseRegionsCreated )
			{
				return(FALSE);
			}

			GetInputText();

			fMouseRegionsCreated = TRUE;

			usPosX = FLOWER_ORDER_DROP_DOWN_CITY_START_X;
			usPosY = FLOWER_ORDER_DROP_DOWN_CITY_START_Y;

			gSelectedFlowerDropDownRegion.resize(GCM->getShippingDestinations().size(),	{});
			for (i = 0; i < gSelectedFlowerDropDownRegion.size(); i++)
			{
				MSYS_DefineRegion(&gSelectedFlowerDropDownRegion[i], usPosX, usPosY + 4, usPosX + FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH, usPosY + usFontHeight, MSYS_PRIORITY_HIGH + 3, CURSOR_WWW, SelectFlowerDropDownMovementCallBack, SelectFlowerDropDownRegionCallBack);
				MSYS_SetRegionUserData( &gSelectedFlowerDropDownRegion[ i ], 0, i);

				usPosY += usFontHeight + 2;
			}
			usTemp = FLOWER_ORDER_DROP_DOWN_CITY_START_Y;
			usHeight = usPosY - usTemp + 10;


			gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_DISPLAY;
			gSelectedFloristDisableDropDownRegion.Enable();


				//disable the text entry fields
				//DisableAllTextFields();
				gsSentimentTextField = GetStringFromField(1);
				KillTextInputMode();

				//disable the clear order and accept order buttons, (their rendering interferes with the drop down graphics)
			}
			break;

		case FLOWER_ORDER_DROP_DOWN_DESTROY:
		{
			if( !fMouseRegionsCreated )
				break;

			for (auto& region : gSelectedFlowerDropDownRegion)
			{
				MSYS_RemoveRegion(&region);
			}

			//display the name on the title bar
			ColorFillVideoSurfaceArea( FRAME_BUFFER, FLOWER_ORDER_DROP_DOWN_LOCATION_X+3, FLOWER_ORDER_DELIVERY_LOCATION_Y+3, FLOWER_ORDER_DROP_DOWN_LOCATION_X+FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH,	FLOWER_ORDER_DELIVERY_LOCATION_Y+FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT-2, RGB(0, 0, 0) );
			DrawTextToScreen(*(GCM->getShippingDestinationName(gubCurrentlySelectedFlowerLocation)), FLOWER_ORDER_DROP_DOWN_CITY_START_X + 6, FLOWER_ORDER_DROP_DOWN_CITY_START_Y + 3, 0, FLOWER_ORDEER_DROP_DOWN_FONT, FLOWER_ORDEER_DROP_DOWN_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			//enable the drop down region
			gSelectedFloristDisableDropDownRegion.Disable();

			fPausedReDrawScreenFlag = TRUE;
			fMouseRegionsCreated  = FALSE;
			gubFlowerDestDropDownMode = FLOWER_ORDER_DROP_DOWN_NO_ACTION;

			//enable the text entry fields
			InitFlowerOrderTextInputBoxes();
		}
			break;

		case  FLOWER_ORDER_DROP_DOWN_DISPLAY:
		{
			UINT8 i;
			UINT16 usPosY, usPosX;
			UINT16 usFontHeight = GetFontHeight( FLOWER_ORDEER_DROP_DOWN_FONT );

			//Display the background for the drop down window
			ColorFillVideoSurfaceArea( FRAME_BUFFER, FLOWER_ORDER_DROP_DOWN_LOCATION_X, FLOWER_ORDER_DROP_DOWN_LOCATION_Y, FLOWER_ORDER_DROP_DOWN_LOCATION_X+FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH,	FLOWER_ORDER_DROP_DOWN_LOCATION_Y+usHeight, RGB(0, 0, 0) );

			//
			// Place the border around the background
			//

			SGPVObject* const hImageHandle = guiDropDownBorder;

			usPosX = usPosY = 0;
			//blit top row of images
			for(i=10; i< FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH-10; i+=10)
			{
				//TOP ROW
				BltVideoObject(FRAME_BUFFER, hImageHandle, 1,i+FLOWER_ORDER_DROP_DOWN_LOCATION_X, usPosY+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);

				//BOTTOM ROW
				BltVideoObject(FRAME_BUFFER, hImageHandle, 6,i+FLOWER_ORDER_DROP_DOWN_LOCATION_X, usHeight-10+6+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
			}

			//blit the left and right row of images
			usPosX = 0;
			for(i=10; i< usHeight-10; i+=10)
			{
				BltVideoObject(FRAME_BUFFER, hImageHandle, 3,usPosX+FLOWER_ORDER_DROP_DOWN_LOCATION_X, i+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
				BltVideoObject(FRAME_BUFFER, hImageHandle, 4,usPosX+FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH-4+FLOWER_ORDER_DROP_DOWN_LOCATION_X, i+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
			}

			//blt the corner images for the row
			//top left
			BltVideoObject(FRAME_BUFFER, hImageHandle, 0, 0+FLOWER_ORDER_DROP_DOWN_LOCATION_X, usPosY+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
			//top right
			BltVideoObject(FRAME_BUFFER, hImageHandle, 2, FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH-10+FLOWER_ORDER_DROP_DOWN_LOCATION_X, usPosY+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
			//bottom left
			BltVideoObject(FRAME_BUFFER, hImageHandle, 5, 0+FLOWER_ORDER_DROP_DOWN_LOCATION_X, usHeight-10+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);
			//bottom right
			BltVideoObject(FRAME_BUFFER, hImageHandle, 7, FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH-10+FLOWER_ORDER_DROP_DOWN_LOCATION_X, usHeight-10+FLOWER_ORDER_DROP_DOWN_LOCATION_Y);


			//Display the list of cities
			usPosY = FLOWER_ORDER_DROP_DOWN_CITY_START_Y + 3;
			for (i = 0; i < GCM->getShippingDestinations().size(); i++)
			{
				DrawTextToScreen(*(GCM->getShippingDestinationName(i)), FLOWER_ORDER_DROP_DOWN_CITY_START_X + 6, usPosY, 0, FLOWER_ORDEER_DROP_DOWN_FONT, FLOWER_ORDEER_DROP_DOWN_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
				usPosY += usFontHeight + 2;
			}

			FlowerOrderDrawSelectedCity( gubCurrentlySelectedFlowerLocation );

			InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
		}
			break;
	}
	return(TRUE);
}


static void FlowerOrderDrawSelectedCity(UINT8 ubNumber)
{
	UINT16 usPosY;
	UINT16 usFontHeight = GetFontHeight( FLOWER_ORDEER_DROP_DOWN_FONT );

	usPosY = (usFontHeight + 2) * ubNumber + FLOWER_ORDER_DROP_DOWN_CITY_START_Y;

	//display the name in the list
	ColorFillVideoSurfaceArea( FRAME_BUFFER, FLOWER_ORDER_DROP_DOWN_CITY_START_X, usPosY+2, FLOWER_ORDER_DROP_DOWN_CITY_START_X+FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH-9,	usPosY+usFontHeight+4, RGB(255, 255, 255) );

	SetFontShadow(NO_SHADOW);
	DrawTextToScreen(*(GCM->getShippingDestinationName(ubNumber)), FLOWER_ORDER_DROP_DOWN_CITY_START_X + 6, usPosY + 3, 0, FLOWER_ORDEER_DROP_DOWN_FONT, 2, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	SetFontShadow(DEFAULT_SHADOW);

	FlowerOrderDisplayShippingLocationCity();
}


static void FlowerOrderDisplayShippingLocationCity(void)
{
	//display the name on the title bar
	ColorFillVideoSurfaceArea( FRAME_BUFFER, FLOWER_ORDER_DROP_DOWN_LOCATION_X+3, FLOWER_ORDER_DELIVERY_LOCATION_Y+3, FLOWER_ORDER_DROP_DOWN_LOCATION_X+FLOWER_ORDER_DROP_DOWN_LOCATION_WIDTH,	FLOWER_ORDER_DELIVERY_LOCATION_Y+FLOWER_ORDER_DELIVERY_LOCATION_HEIGHT-2, RGB(0, 0, 0) );
	DrawTextToScreen(*(GCM->getShippingDestinationName(gubCurrentlySelectedFlowerLocation)), FLOWER_ORDER_DELIVERY_LOCATION_X + 5, FLOWER_ORDER_DELIVERY_LOCATION_Y + 5, 0, FLOWER_ORDEER_SMALL_FONT, FLOWER_ORDEER_SMALL_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
}


static void InitFlowerOrderTextInputBoxes(void)
{
	InitTextInputMode();
	SetTextInputFont(FONT12ARIAL);
	Set16BPPTextFieldColor( RGB(255, 255, 255) );
	SetBevelColors( RGB(136, 138, 135), RGB(24, 61, 81) );
	SetTextInputRegularColors( 2, FONT_WHITE );
	SetTextInputHilitedColors( FONT_WHITE, FONT_COLOR_P2, FONT_COLOR_P141  );
	SetCursorColor( RGB(0, 0, 0) );

	AddUserInputField(NULL);

	if( gbCurrentlySelectedCard != -1 )
	{
		//Get and display the card saying
		//Display Flower Desc

		const UINT32 uiStartLoc = FLOR_CARD_TEXT_TITLE_SIZE * gbCurrentlySelectedCard;
		ST::string sTemp = GCM->loadEncryptedString( FLOR_CARD_TEXT_FILE, uiStartLoc, FLOR_CARD_TEXT_TITLE_SIZE);
		ST::string sText = CleanOutControlCodesFromString(sTemp);

		gsSentimentTextField = sText;

		gbCurrentlySelectedCard = -1;

	}

	//personal sentiment box
	AddTextInputField(FLOWER_ORDER_PERSONAL_SENT_BOX_X, FLOWER_ORDER_PERSONAL_SENT_BOX_Y, FLOWER_ORDER_PERSONAL_SENT_TEXT_WIDTH, FLOWER_ORDER_PERSONAL_SENT_TEXT_HEIGHT, MSYS_PRIORITY_HIGH + 2, gsSentimentTextField, FLOWER_ORDER_PERSONEL_SENTIMENT_NUM_CHARS, INPUTTYPE_FULL_TEXT);

	//Name box
	AddTextInputField(FLOWER_ORDER_NAME_TEXT_BOX_X, FLOWER_ORDER_NAME_TEXT_BOX_Y, FLOWER_ORDER_NAME_TEXT_BOX_WIDTH, FLOWER_ORDER_NAME_TEXT_BOX_HEIGHT, MSYS_PRIORITY_HIGH + 2, gsNameTextField, FLOWER_ORDER_NAME_FIELD_NUM_CHARS, INPUTTYPE_FULL_TEXT);
}


static void DestroyFlowerOrderTextInputBoxes(void)
{
	KillTextInputMode();
}


static void HandleFloristOrderKeyBoardInput(void)
{
	InputAtom InputEvent;
	while (DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS))
	{
		if( !HandleTextInput( &InputEvent ) && InputEvent.usEvent == KEY_DOWN )
		{
			switch (InputEvent.usParam)
			{
				case SDLK_RETURN:
					GetInputText();
					break;

				case SDLK_ESCAPE: SetActiveField(0); break;

				default:
					HandleKeyBoardShortCutsForLapTop( InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState );
					break;
			}
		}
	}
}


//Initialize the Florsit Order Page (reset some variables)
void InitFloristOrderForm()
{
	gfFLoristCheckBox0Down = FALSE; // next day delviery
	gfFLoristCheckBox1Down = TRUE; // when it gets there delivery
	gfFLoristCheckBox2Down = FALSE;
	gfFLoristCheckBox3Down = FALSE;
	gfFLoristCheckBox4Down = FALSE;
	gfFLoristCheckBox5Down = FALSE;

	guiFlowerPrice = 0;

	gubCurrentlySelectedFlowerLocation = 0;
	gbCurrentlySelectedCard = -1;

	gsSentimentTextField.clear();
	gsNameTextField.clear();
}
