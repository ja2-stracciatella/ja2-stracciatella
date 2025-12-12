#include "Directories.h"
#include "Font.h"
#include "GameRes.h"
#include "HImage.h"
#include "ItemModel.h"
#include "Laptop.h"
#include "BobbyRMailOrder.h"
#include "BobbyR.h"
#include "MessageBoxScreen.h"
#include "Text.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "BobbyRGuns.h"
#include "Finances.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "Random.h"
#include "LaptopSave.h"
#include "Soldier_Profile.h"
#include "Input.h"
#include "Line.h"
#include "StrategicMap.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Debug.h"
#include "ScreenIDs.h"
#include "Font_Control.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "ShippingDestinationModel.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <vector>


//drop down menu
enum
{
	BR_DROP_DOWN_NO_ACTION,
	BR_DROP_DOWN_CREATE,
	BR_DROP_DOWN_DESTROY,
	BR_DROP_DOWN_DISPLAY,
};

#define BOBBYR_ORDER_NUM_SHIPPING_CITIES	17
#define BOBBYR_NUM_DISPLAYED_CITIES		10

#define OVERNIGHT_EXPRESS			1
#define TWO_BUSINESS_DAYS			2
#define STANDARD_SERVICE			3

#define MIN_SHIPPING_WEIGHT			20

#define BOBBYR_ORDER_TITLE_TEXT_FONT		FONT14ARIAL
#define BOBBYR_ORDER_TITLE_TEXT_COLOR		157

#define BOBBYR_FONT_BLACK			2

#define BOBBYR_ORDER_STATIC_TEXT_FONT		FONT12ARIAL
#define BOBBYR_ORDER_STATIC_TEXT_COLOR		145

#define BOBBYR_DISCLAIMER_FONT			FONT10ARIAL

#define BOBBYR_ORDER_DYNAMIC_TEXT_FONT		FONT12ARIAL
#define BOBBYR_ORDER_DYNAMIC_TEXT_COLOR		FONT_MCOLOR_WHITE

#define BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR	FONT_MCOLOR_WHITE

#define BOBBYR_DROPDOWN_FONT			FONT12ARIAL

#define BOBBYR_ORDERGRID_X			LAPTOP_SCREEN_UL_X + 2
#define BOBBYR_ORDERGRID_Y			LAPTOP_SCREEN_WEB_UL_Y + 62

#define BOBBYR_BOBBY_RAY_TITLE_X		LAPTOP_SCREEN_UL_X + 171
#define BOBBYR_BOBBY_RAY_TITLE_Y		LAPTOP_SCREEN_WEB_UL_Y + 3
#define BOBBYR_BOBBY_RAY_TITLE_WIDTH		160
#define BOBBYR_BOBBY_RAY_TITLE_HEIGHT		35

#define BOBBYR_LOCATION_BOX_X			LAPTOP_SCREEN_UL_X + 276
#define BOBBYR_LOCATION_BOX_Y			LAPTOP_SCREEN_WEB_UL_Y + 62

#define BOBBYR_DELIVERYSPEED_X			LAPTOP_SCREEN_UL_X + 276
#define BOBBYR_DELIVERYSPEED_Y			LAPTOP_SCREEN_WEB_UL_Y + 149

#define BOBBYR_CLEAR_ORDER_X			LAPTOP_SCREEN_UL_X + 309
#define BOBBYR_CLEAR_ORDER_Y			LAPTOP_SCREEN_WEB_UL_Y + 268	//LAPTOP_SCREEN_WEB_UL_Y + 252

#define BOBBYR_ACCEPT_ORDER_X			LAPTOP_SCREEN_UL_X + 299
#define BOBBYR_ACCEPT_ORDER_Y			LAPTOP_SCREEN_WEB_UL_Y + 303	//LAPTOP_SCREEN_WEB_UL_Y + 288

#define BOBBYR_GRID_ROW_OFFSET			20
#define BOBBYR_GRID_TITLE_OFFSET		27

#define BOBBYR_GRID_FIRST_COLUMN_X		3//BOBBYR_ORDERGRID_X + 3
#define BOBBYR_GRID_FIRST_COLUMN_Y		37//BOBBYR_ORDERGRID_Y + 37
#define BOBBYR_GRID_FIRST_COLUMN_WIDTH		23

#define BOBBYR_GRID_SECOND_COLUMN_X		28//BOBBYR_ORDERGRID_X + 28
#define BOBBYR_GRID_SECOND_COLUMN_Y		BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_SECOND_COLUMN_WIDTH		40

#define BOBBYR_GRID_THIRD_COLUMN_X		70//BOBBYR_ORDERGRID_X + 70
#define BOBBYR_GRID_THIRD_COLUMN_Y		BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_THIRD_COLUMN_WIDTH		111

#define BOBBYR_GRID_FOURTH_COLUMN_X		184//BOBBYR_ORDERGRID_X + 184
#define BOBBYR_GRID_FOURTH_COLUMN_Y		BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_FOURTH_COLUMN_WIDTH		40

#define BOBBYR_GRID_FIFTH_COLUMN_X		224//BOBBYR_ORDERGRID_X + 224
#define BOBBYR_GRID_FIFTH_COLUMN_Y		BOBBYR_GRID_FIRST_COLUMN_Y
#define BOBBYR_GRID_FIFTH_COLUMN_WIDTH		42

#define   BOBBYR_SUBTOTAL_WIDTH			212
#define BOBBYR_SUBTOTAL_X			BOBBYR_GRID_FIRST_COLUMN_X
#define BOBBYR_SUBTOTAL_Y			BOBBYR_GRID_FIRST_COLUMN_Y + BOBBYR_GRID_ROW_OFFSET * 10 + 3

#define BOBBYR_SHIPPING_N_HANDLE_Y		BOBBYR_SUBTOTAL_Y + 17
#define   BOBBYR_GRAND_TOTAL_Y			BOBBYR_SHIPPING_N_HANDLE_Y + 20

#define BOBBYR_SHIPPING_LOCATION_TEXT_X		BOBBYR_LOCATION_BOX_X + 8
#define BOBBYR_SHIPPING_LOCATION_TEXT_Y		BOBBYR_LOCATION_BOX_Y + 8

#define BOBBYR_SHIPPING_SPEED_X			BOBBYR_SHIPPING_LOCATION_TEXT_X
#define BOBBYR_SHIPPING_SPEED_Y			BOBBYR_DELIVERYSPEED_Y + 11

#define BOBBYR_SHIPPING_COST_X			BOBBYR_SHIPPING_SPEED_X + 130

#define BOBBYR_OVERNIGHT_EXPRESS_Y		BOBBYR_DELIVERYSPEED_Y + 42

#define BOBBYR_ORDER_FORM_TITLE_X		BOBBYR_BOBBY_RAY_TITLE_X
#define BOBBYR_ORDER_FORM_TITLE_Y		BOBBYR_BOBBY_RAY_TITLE_Y + 37
#define BOBBYR_ORDER_FORM_TITLE_WIDTH		159

#define BOBBYR_BACK_BUTTON_X			(STD_SCREEN_X + 130)
#define BOBBYR_BACK_BUTTON_Y			(STD_SCREEN_Y + 400 + LAPTOP_SCREEN_WEB_DELTA_Y + 4)

#define BOBBYR_HOME_BUTTON_X			(STD_SCREEN_X + 515)
#define BOBBYR_HOME_BUTTON_Y			BOBBYR_BACK_BUTTON_Y

#define BOBBYR_SHIPMENT_BUTTON_X		( LAPTOP_SCREEN_UL_X + ( LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X - 75 ) / 2)
#define BOBBYR_SHIPMENT_BUTTON_Y		BOBBYR_BACK_BUTTON_Y

#define SHIPPING_SPEED_LIGHT_WIDTH		9
#define SHIPPING_SPEED_LIGHT_HEIGHT		9

#define BOBBYR_CONFIRM_ORDER_X			(STD_SCREEN_X + 220)
#define BOBBYR_CONFIRM_ORDER_Y			(STD_SCREEN_Y + 170)

#define BOBBYR_CITY_START_LOCATION_X		BOBBYR_LOCATION_BOX_X + 6
#define BOBBYR_CITY_START_LOCATION_Y		BOBBYR_LOCATION_BOX_Y + 61
#define BOBBYR_DROP_DOWN_WIDTH			182//203
#define BOBBYR_DROP_DOWN_HEIGHT			19
#define BOBBYR_CITY_NAME_OFFSET			6

#define BOBBYR_SCROLL_AREA_X			BOBBYR_CITY_START_LOCATION_X + BOBBYR_DROP_DOWN_WIDTH
#define BOBBYR_SCROLL_AREA_Y			BOBBYR_CITY_START_LOCATION_Y
#define BOBBYR_SCROLL_AREA_WIDTH		22
#define BOBBYR_SCROLL_AREA_HEIGHT		139
#define BOBBYR_SCROLL_AREA_HEIGHT_MINUS_ARROWS	( BOBBYR_SCROLL_AREA_HEIGHT - (2 * BOBBYR_SCROLL_ARROW_HEIGHT) - 8 )

#define BOBBYR_SCROLL_UP_ARROW_X		BOBBYR_SCROLL_AREA_X
#define BOBBYR_SCROLL_UP_ARROW_Y		BOBBYR_SCROLL_AREA_Y+5
#define BOBBYR_SCROLL_DOWN_ARROW_X		BOBBYR_SCROLL_UP_ARROW_X
#define BOBBYR_SCROLL_DOWN_ARROW_Y		BOBBYR_SCROLL_AREA_Y+BOBBYR_SCROLL_AREA_HEIGHT-24
#define BOBBYR_SCROLL_ARROW_WIDTH		18
#define BOBBYR_SCROLL_ARROW_HEIGHT		20


#define BOBBYR_SHIPPING_LOC_AREA_L_X		BOBBYR_LOCATION_BOX_X + 9
#define BOBBYR_SHIPPING_LOC_AREA_T_Y		BOBBYR_LOCATION_BOX_Y + 39

#define BOBBYR_SHIPPING_LOC_AREA_R_X		BOBBYR_LOCATION_BOX_X + 206
#define BOBBYR_SHIPPING_LOC_AREA_B_Y		BOBBYR_LOCATION_BOX_Y + 57

#define BOBBYR_SHIPPING_SPEED_NUMBER_X		BOBBYR_SHIPPING_COST_X
#define BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH	37

#define BOBBYR_TOTAL_SAVED_AREA_X		BOBBYR_ORDERGRID_X + 221
#define BOBBYR_TOTAL_SAVED_AREA_Y		BOBBYR_ORDERGRID_Y + 237

#define BOBBYR_USED_WARNING_X			(STD_SCREEN_X + 122)
#define BOBBYR_USED_WARNING_Y			(STD_SCREEN_Y + 382 + LAPTOP_SCREEN_WEB_DELTA_Y)


#define BOBBYR_PACKAXGE_WEIGHT_X		BOBBYR_LOCATION_BOX_X
#define BOBBYR_PACKAXGE_WEIGHT_Y		LAPTOP_SCREEN_WEB_UL_Y + 249
#define BOBBYR_PACKAXGE_WEIGHT_WIDTH		188




UINT16 gShippingSpeedAreas[6];


// Identifier for the images
static SGPVObject* guiBobbyRayTitle;
static SGPVObject* guiBobbyROrderGrid;
static SGPVObject* guiBobbyRLocationGraphic;
static SGPVObject* guiDeliverySpeedGraphic;
static SGPVObject* guiTotalSaveArea;		//used as a savebuffer for the subtotal, s&h, and grand total values
static SGPVObject* guiDropDownBorder;
static SGPVObject* guiGoldArrowImages;
static SGPVObject* guiPackageWeightImage;


static BOOLEAN gfReDrawBobbyOrder = FALSE;

static INT32  giGrandTotal;
static UINT32 guiSubTotal;

static UINT8 gubSelectedLight;

static BOOLEAN gfDrawConfirmOrderGrpahic;
static BOOLEAN gfDestroyConfirmGrphiArea;

static BOOLEAN gfCanAcceptOrder;

static UINT8 gubDropDownAction;
static INT8  gbSelectedCity = -1;				//keeps track of the currently selected city
static UINT8 gubCityAtTopOfList;

static BOOLEAN gfRemoveItemsFromStock = FALSE;

std::vector<NewBobbyRayOrderStruct> gpNewBobbyrShipments;


//
//Buttons
//

//Clear Order Button
static void BtnBobbyRClearOrderCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyRClearOrderImage;
static GUIButtonRef guiBobbyRClearOrder;

//Accept Order Button
static void BtnBobbyRAcceptOrderCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyRAcceptOrderImage;
static GUIButtonRef guiBobbyRAcceptOrder;

//Back Button
static void BtnBobbyRBackCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyRBackImage;
static GUIButtonRef guiBobbyRBack;

//Home Button
static void BtnBobbyRHomeCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyRHomeImage;
static GUIButtonRef guiBobbyRHome;

//Goto Shipment Page Button
static void BtnBobbyRGotoShipmentPageCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* giBobbyRGotoShipmentPageImage;
static GUIButtonRef guiBobbyRGotoShipmentPage;


//mouse region for the shipping speed selection area
static MOUSE_REGION gSelectedShippingSpeedRegion[3];

//mouse region for the confirm area
static MOUSE_REGION gSelectedConfirmOrderRegion;

//mouse region for the drop down city location area
static MOUSE_REGION gSelectedDropDownRegion[BOBBYR_ORDER_NUM_SHIPPING_CITIES];

//mouse region for scroll area for the drop down city location area
static MOUSE_REGION gSelectedScrollAreaDropDownRegion[BOBBYR_ORDER_NUM_SHIPPING_CITIES];


//mouse region to activate the shipping location drop down
static MOUSE_REGION gSelectedActivateCityDroDownRegion;

//mouse region to close the drop down menu
static MOUSE_REGION gSelectedCloseDropDownRegion;

//mouse region to click on the title to go to the home page
static MOUSE_REGION gSelectedTitleLinkRegion;

//mouse region to click on the up or down arrow on the scroll area
static MOUSE_REGION gSelectedUpDownArrowOnScrollAreaRegion[2];


void GameInitBobbyRMailOrder()
{
	gubSelectedLight = 0;

	gpNewBobbyrShipments.clear();
}


static GUIButtonRef MakeButton(BUTTON_PICS* img, const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	const INT16 shadow_col = BOBBYR_GUNS_SHADOW_COLOR;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, BOBBYR_ORDER_TITLE_TEXT_FONT, BOBBYR_GUNS_TEXT_COLOR_ON, shadow_col, BOBBYR_GUNS_TEXT_COLOR_OFF, shadow_col, x, y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


static void SelectActivateCityDroDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectCloseDroDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectConfirmOrderRegionCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectConfirmOrderRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectShippingSpeedRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterBobbyRMailOrder()
{
	UINT16					i;

	gfReDrawBobbyOrder = FALSE;
	gfDrawConfirmOrderGrpahic = FALSE;
	gfDestroyConfirmGrphiArea = FALSE;
	gfCanAcceptOrder = TRUE;
	gubDropDownAction = BR_DROP_DOWN_NO_ACTION;

	// load the Order Grid graphic and add it
	guiBobbyROrderGrid = AddVideoObjectFromFile(LAPTOPDIR "/bobbyordergrid.sti");

	// load the Location graphic and add it
	guiBobbyRLocationGraphic = AddVideoObjectFromFile(LAPTOPDIR "/bobbylocationbox.sti");

	// load the delivery speed graphic and add it
	guiDeliverySpeedGraphic = AddVideoObjectFromFile(LAPTOPDIR "/bobbydeliveryspeed.sti");

	// load the delivery speed graphic and add it
	guiTotalSaveArea = AddVideoObjectFromFile(LAPTOPDIR "/totalsavearea.sti");

	// border
	guiDropDownBorder = AddVideoObjectFromFile(INTERFACEDIR "/tactpopup.sti");

	// Gold Arrow for the scroll area
	guiGoldArrowImages = AddVideoObjectFromFile(LAPTOPDIR "/goldarrows.sti");

	// Package Weight Graphic
	guiPackageWeightImage = AddVideoObjectFromFile(LAPTOPDIR "/packageweight.sti");


	InitBobbyRWoodBackground();

	//
	//Init the button areas
	//

	// Clear Order button
	guiBobbyRClearOrderImage = LoadButtonImage(LAPTOPDIR "/eraseorderbutton.sti", 0, 1);
	guiBobbyRClearOrder = MakeButton(guiBobbyRClearOrderImage, BobbyROrderFormText[BOBBYR_CLEAR_ORDER], BOBBYR_CLEAR_ORDER_X, BOBBYR_CLEAR_ORDER_Y + 4, BtnBobbyRClearOrderCallback);
	guiBobbyRClearOrder->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
	guiBobbyRClearOrder->SpecifyTextOffsets(39, 10, TRUE);


	// Accept Order button
	guiBobbyRAcceptOrderImage = LoadButtonImage(LAPTOPDIR "/acceptorderbutton.sti", 2, 0, -1, 1, -1);
	guiBobbyRAcceptOrder = MakeButton(guiBobbyRAcceptOrderImage, BobbyROrderFormText[BOBBYR_ACCEPT_ORDER], BOBBYR_ACCEPT_ORDER_X, BOBBYR_ACCEPT_ORDER_Y + 4, BtnBobbyRAcceptOrderCallback);
	guiBobbyRAcceptOrder->SpecifyTextOffsets(43, 24, TRUE);
	guiBobbyRAcceptOrder->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);

	if( gbSelectedCity == -1 )
		DisableButton( guiBobbyRAcceptOrder );

	//if there is anything to buy, dont disable the accept button
	//if( !IsAnythingPurchasedFromBobbyRayPage() )
	//{
	//	DisableButton( guiBobbyRAcceptOrder );
	//}


	guiBobbyRBackImage = LoadButtonImage(LAPTOPDIR "/cataloguebutton.sti", 0, 1);
	guiBobbyRBack = MakeButton(guiBobbyRBackImage, BobbyROrderFormText[BOBBYR_BACK], BOBBYR_BACK_BUTTON_X, BOBBYR_BACK_BUTTON_Y, BtnBobbyRBackCallback);

	guiBobbyRHomeImage = UseLoadedButtonImage(guiBobbyRBackImage, 0, 1);
	guiBobbyRHome = MakeButton(guiBobbyRHomeImage, BobbyROrderFormText[BOBBYR_HOME], BOBBYR_HOME_BUTTON_X, BOBBYR_HOME_BUTTON_Y, BtnBobbyRHomeCallback);

	giBobbyRGotoShipmentPageImage = UseLoadedButtonImage(guiBobbyRBackImage, 0, 1);
	guiBobbyRGotoShipmentPage = MakeButton(giBobbyRGotoShipmentPageImage, BobbyROrderFormText[BOBBYR_GOTOSHIPMENT_PAGE], BOBBYR_SHIPMENT_BUTTON_X, BOBBYR_SHIPMENT_BUTTON_Y, BtnBobbyRGotoShipmentPageCallback);

	for(i=0; i<3; i++)
	{

		MSYS_DefineRegion(&gSelectedShippingSpeedRegion[i], gShippingSpeedAreas[i*2],
					gShippingSpeedAreas[i*2+1] ,
					(UINT16)(gShippingSpeedAreas[i*2]+SHIPPING_SPEED_LIGHT_WIDTH),
					(UINT16)(gShippingSpeedAreas[i*2+1]+SHIPPING_SPEED_LIGHT_HEIGHT), MSYS_PRIORITY_HIGH,
					CURSOR_WWW, MSYS_NO_CALLBACK, SelectShippingSpeedRegionCallBack);
		MSYS_SetRegionUserData( &gSelectedShippingSpeedRegion[ i ], 0, i);
	}

	//confirmorder mouse region, occupies the entrie screen and is present only when the confirm order graphic
	//s on screen.  When user clicks anywhere the graphic disappears
	MSYS_DefineRegion(&gSelectedConfirmOrderRegion, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y ,
				LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH+1,
				CURSOR_WWW, MSYS_NO_CALLBACK, MouseCallbackPrimarySecondary(SelectConfirmOrderRegionCallBackPrimary, SelectConfirmOrderRegionCallBackSecondary));
	gSelectedConfirmOrderRegion.Disable();

	//click on the shipping location to activate the drop down menu
	MSYS_DefineRegion(&gSelectedActivateCityDroDownRegion, BOBBYR_SHIPPING_LOC_AREA_L_X,
				BOBBYR_SHIPPING_LOC_AREA_T_Y, BOBBYR_SHIPPING_LOC_AREA_R_X,
				BOBBYR_SHIPPING_LOC_AREA_B_Y, MSYS_PRIORITY_HIGH,
				CURSOR_WWW, MSYS_NO_CALLBACK, SelectActivateCityDroDownRegionCallBack);

	//click anywhere on the screen to close the window( only when the drop down window is active)
	MSYS_DefineRegion(&gSelectedCloseDropDownRegion, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y,
				LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH-1,
				CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, SelectCloseDroDownRegionCallBack);
	gSelectedCloseDropDownRegion.Disable();

	CreateBobbyRayOrderTitle();

	gfRemoveItemsFromStock = FALSE;

	RenderBobbyRMailOrder();
}


static void CreateDestroyBobbyRDropDown(UINT8 ubDropDownAction);
static void RemovePurchasedItemsFromBobbyRayInventory();


void ExitBobbyRMailOrder()
{
	UINT16 i;

	//if we are to remove the items from stock
	if( gfRemoveItemsFromStock )
	{
		//Remove the items for Boby Rqys Inventory
		RemovePurchasedItemsFromBobbyRayInventory();
	}

	DestroyBobbyROrderTitle();

	DeleteVideoObject(guiBobbyROrderGrid);
	DeleteVideoObject(guiBobbyRLocationGraphic);
	DeleteVideoObject(guiDeliverySpeedGraphic);
	RemoveVObject(MLG_CONFIRMORDER);
	DeleteVideoObject(guiTotalSaveArea);
	DeleteVideoObject(guiDropDownBorder);
	DeleteVideoObject(guiGoldArrowImages);
	DeleteVideoObject(guiPackageWeightImage);

	UnloadButtonImage( guiBobbyRClearOrderImage );
	RemoveButton( guiBobbyRClearOrder );

	UnloadButtonImage( guiBobbyRAcceptOrderImage );
	RemoveButton( guiBobbyRAcceptOrder );

	UnloadButtonImage( guiBobbyRBackImage );
	RemoveButton( guiBobbyRBack );

	UnloadButtonImage( giBobbyRGotoShipmentPageImage );
	RemoveButton( guiBobbyRGotoShipmentPage );

	RemoveButton( guiBobbyRHome );
	UnloadButtonImage( guiBobbyRHomeImage );

	DeleteBobbyRWoodBackground();

	for(i=0; i<3; i++)
	{
		MSYS_RemoveRegion( &gSelectedShippingSpeedRegion[i]);
	}

	MSYS_RemoveRegion( &gSelectedConfirmOrderRegion);
	MSYS_RemoveRegion( &gSelectedActivateCityDroDownRegion);
	MSYS_RemoveRegion( &gSelectedCloseDropDownRegion);


	//if the drop down box is active, destroy it
	gubDropDownAction = BR_DROP_DOWN_DESTROY;
	CreateDestroyBobbyRDropDown( BR_DROP_DOWN_DESTROY );
}

void HandleBobbyRMailOrder()
{
	if(gfReDrawBobbyOrder)
	{
		//RenderBobbyRMailOrder();
		fPausedReDrawScreenFlag = TRUE;
		gfReDrawBobbyOrder = FALSE;
	}

	if(gfDrawConfirmOrderGrpahic)
	{
		// Bobbyray title
		SGPVObject const * const vo{ GetVObject(MLG_CONFIRMORDER) };
		BltVideoObjectOutlineShadow(FRAME_BUFFER, vo, 0, BOBBYR_CONFIRM_ORDER_X + 3, BOBBYR_CONFIRM_ORDER_Y + 3);
		BltVideoObject(FRAME_BUFFER, vo, 0, BOBBYR_CONFIRM_ORDER_X,     BOBBYR_CONFIRM_ORDER_Y);
		InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);

		gfDrawConfirmOrderGrpahic = FALSE;
	}

	if( gfDestroyConfirmGrphiArea )
	{
		gfDestroyConfirmGrphiArea = FALSE;
		gfReDrawBobbyOrder = TRUE;
		gSelectedConfirmOrderRegion.Disable();
		gfCanAcceptOrder = TRUE;
	}

	if( gubDropDownAction != BR_DROP_DOWN_NO_ACTION )
	{
		CreateDestroyBobbyRDropDown( gubDropDownAction );

		if( gubDropDownAction == BR_DROP_DOWN_CREATE )
			gubDropDownAction = BR_DROP_DOWN_DISPLAY;
		else
			gubDropDownAction = BR_DROP_DOWN_NO_ACTION;
	}

}


static void DisplayPackageWeight(void);
static void DisplayShippingLocationCity(void);
static void DrawShippingSpeedLights(UINT8 ubSelected);


void RenderBobbyRMailOrder()
{
	UINT16 usPosY;
	UINT16 usHeight;//usWidth,
	ST::string sTemp;

	DrawBobbyRWoodBackground();

	DrawBobbyROrderTitle();

	BltVideoObject(FRAME_BUFFER, guiBobbyROrderGrid,       0, BOBBYR_ORDERGRID_X,       BOBBYR_ORDERGRID_Y);
	BltVideoObject(FRAME_BUFFER, guiBobbyRLocationGraphic, 0, BOBBYR_LOCATION_BOX_X,    BOBBYR_LOCATION_BOX_Y);
	BltVideoObject(FRAME_BUFFER, guiDeliverySpeedGraphic,  0, BOBBYR_DELIVERYSPEED_X,   BOBBYR_DELIVERYSPEED_Y);
	BltVideoObject(FRAME_BUFFER, guiPackageWeightImage,    0, BOBBYR_PACKAXGE_WEIGHT_X, BOBBYR_PACKAXGE_WEIGHT_Y);

	//
	// Display the STATIC text
	//

	//Output the title
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_ORDER_FORM], BOBBYR_ORDER_FORM_TITLE_X, BOBBYR_ORDER_FORM_TITLE_Y, BOBBYR_ORDER_FORM_TITLE_WIDTH, BOBBYR_ORDER_TITLE_TEXT_FONT, BOBBYR_ORDER_TITLE_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Output the shipping location
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_SHIPPING_LOCATION], BOBBYR_SHIPPING_LOCATION_TEXT_X, BOBBYR_SHIPPING_LOCATION_TEXT_Y, 0, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Output the shiupping speed
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_SHIPPING_SPEED], BOBBYR_SHIPPING_SPEED_X, BOBBYR_SHIPPING_SPEED_Y, 0, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Create a string for the weight ( %s ) ( where %s is the weight string, either kg or lbs )
	sTemp = st_format_printf(BobbyROrderFormText[BOBBYR_COST], GetWeightUnitString( ));

	//Output the cost
	DrawTextToScreen(sTemp, BOBBYR_SHIPPING_COST_X, BOBBYR_SHIPPING_SPEED_Y, 0, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Output the overnight, business days, standard service
	usPosY = BOBBYR_OVERNIGHT_EXPRESS_Y;
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_OVERNIGHT_EXPRESS], BOBBYR_SHIPPING_SPEED_X, usPosY, 0, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY +=BOBBYR_GRID_ROW_OFFSET;
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_BUSINESS_DAYS], BOBBYR_SHIPPING_SPEED_X, usPosY, 0, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY +=BOBBYR_GRID_ROW_OFFSET;
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_STANDARD_SERVICE], BOBBYR_SHIPPING_SPEED_X, usPosY, 0, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//DisplayPurchasedItems();
	DisplayPurchasedItems( TRUE, BOBBYR_ORDERGRID_X, BOBBYR_ORDERGRID_Y, BobbyRayPurchases, FALSE, -1 );

	DrawShippingSpeedLights(gubSelectedLight);

	DisplayShippingLocationCity();

	//Display the 'used' text at the bottom of the screen
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_USED_TEXT], BOBBYR_USED_WARNING_X, BOBBYR_USED_WARNING_Y+1, 0, BOBBYR_DISCLAIMER_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | TEXT_SHADOWED );

	//Display the minimum weight disclaimer at the bottom of the page
	usHeight = GetFontHeight( BOBBYR_DISCLAIMER_FONT ) + 2;
	sTemp = ST::format("{} {2.1f} {}.", BobbyROrderFormText[BOBBYR_MINIMUM_WEIGHT], GetWeightBasedOnMetricOption( MIN_SHIPPING_WEIGHT ) / 10.0, GetWeightUnitString());
	DrawTextToScreen(sTemp, BOBBYR_USED_WARNING_X, BOBBYR_USED_WARNING_Y + usHeight + 1, 0, BOBBYR_DISCLAIMER_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | TEXT_SHADOWED);


	//Calculate and display the total package weight
	DisplayPackageWeight( );

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnBobbyRClearOrderCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		std::fill_n(BobbyRayPurchases, MAX_PURCHASE_AMOUNT, BobbyRayPurchaseStruct{});
		gubSelectedLight = 0;
		gfReDrawBobbyOrder = TRUE;
		gbSelectedCity = -1;
		gubCityAtTopOfList = 0;

		//Get rid of the city drop dowm, if it is being displayed
		gubDropDownAction = BR_DROP_DOWN_DESTROY;

		//disable the accept order button
		DisableButton( guiBobbyRAcceptOrder );
	}
}


static void ConfirmBobbyRPurchaseMessageBoxCallBack(MessageBoxReturnValue);


static void BtnBobbyRAcceptOrderCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( guiSubTotal && gfCanAcceptOrder )
		{
			//if the player doesnt have enough money
			if( LaptopSaveInfo.iCurrentBalance < giGrandTotal )
			{
				DoLapTopMessageBox(
					MSG_BOX_LAPTOP_DEFAULT,
					BobbyROrderFormText[BOBBYR_CANT_AFFORD_PURCHASE],
					LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL
				);
			}
			else
			{
				ST::string zTemp;

				auto dest = GCM->getPrimaryShippingDestination();
				auto destSectorIndex = SGPSector(dest->getDeliverySector()).AsStrategicIndex();
				//if the city is Drassen, and the airport sector is player controlled
				if( gbSelectedCity == dest->locationId && !StrategicMap[destSectorIndex].fEnemyControlled )
				{
					//Quick hack to bypass the confirmation box
					ConfirmBobbyRPurchaseMessageBoxCallBack( MSG_BOX_RETURN_YES );
				}
				else
				{
					//else pop up a confirmation box
					zTemp = st_format_printf(BobbyROrderFormText[BOBBYR_CONFIRM_DEST], GCM->getShippingDestination(gbSelectedCity)->name);
					DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, zTemp, LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO, ConfirmBobbyRPurchaseMessageBoxCallBack );
				}
			}
		}
	}
}


static void DisplayShippingCosts(BOOLEAN fCalledFromOrderPage, INT32 iSubTotal, UINT16 usGridX, UINT16 usGridY, INT32 iOrderNum);


void DisplayPurchasedItems( BOOLEAN fCalledFromOrderPage, UINT16 usGridX, UINT16 usGridY, BobbyRayPurchaseStruct *pBobbyRayPurchase, BOOLEAN fJustDisplayTitles, INT32 iOrderNum )
{
	UINT16  i;
	ST::string sTemp;
	UINT16  usPosY;
	UINT32  uiStartLoc=0;
	UINT32  uiTotal;

	//Output the qty
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_QTY], usGridX + BOBBYR_GRID_FIRST_COLUMN_X, usGridY + BOBBYR_GRID_FIRST_COLUMN_Y - BOBBYR_GRID_TITLE_OFFSET, BOBBYR_GRID_FIRST_COLUMN_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Create a string for the weight ( %s ) ( where %s is the weight string, either kg or lbs )
	sTemp = st_format_printf(BobbyROrderFormText[BOBBYR_WEIGHT], GetWeightUnitString( ));

	//Output the Weight
	DisplayWrappedString(usGridX + BOBBYR_GRID_SECOND_COLUMN_X, usGridY + BOBBYR_GRID_SECOND_COLUMN_Y - 30, BOBBYR_GRID_SECOND_COLUMN_WIDTH, 2, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, sTemp, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Output the name
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_NAME], usGridX + BOBBYR_GRID_THIRD_COLUMN_X, usGridY + BOBBYR_GRID_THIRD_COLUMN_Y - BOBBYR_GRID_TITLE_OFFSET, BOBBYR_GRID_THIRD_COLUMN_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Output the unit price
	DisplayWrappedString(usGridX + BOBBYR_GRID_FOURTH_COLUMN_X, usGridY + BOBBYR_GRID_FOURTH_COLUMN_Y - 30, BOBBYR_GRID_FOURTH_COLUMN_WIDTH, 2, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, BobbyROrderFormText[BOBBYR_UNIT_PRICE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Output the total
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_TOTAL], usGridX + BOBBYR_GRID_FIFTH_COLUMN_X, usGridY + BOBBYR_GRID_FIFTH_COLUMN_Y - BOBBYR_GRID_TITLE_OFFSET, BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Output the sub total, shipping and handling, and the grand total
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_SUB_TOTAL],   usGridX + BOBBYR_SUBTOTAL_X, usGridY + BOBBYR_SUBTOTAL_Y,          BOBBYR_SUBTOTAL_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_S_H],         usGridX + BOBBYR_SUBTOTAL_X, usGridY + BOBBYR_SHIPPING_N_HANDLE_Y, BOBBYR_SUBTOTAL_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_GRAND_TOTAL], usGridX + BOBBYR_SUBTOTAL_X, usGridY + BOBBYR_GRAND_TOTAL_Y,       BOBBYR_SUBTOTAL_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	if( fJustDisplayTitles )
	{
		return;
	}

	INT32 iSubTotal = 0; // XXX HACK000E
	if( fCalledFromOrderPage )
	{
		guiSubTotal = 0;
		giGrandTotal = 0;
	}
	else
	{
		iSubTotal = 0;
	}


	if( pBobbyRayPurchase == NULL )
	{
		return;
	}

	//loop through the array of purchases to display only the items that are purchased
	usPosY = usGridY+BOBBYR_GRID_FIRST_COLUMN_Y + 4;
	for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
	{
		//if the item was purchased
		if( pBobbyRayPurchase[i].ubNumberPurchased )
		{
			uiTotal = 0;

			//Display the qty, order#, item name, unit price and the total

			//qty
			sTemp = ST::format("{3d}", pBobbyRayPurchase[i].ubNumberPurchased);
			DrawTextToScreen(sTemp, usGridX + BOBBYR_GRID_FIRST_COLUMN_X - 2, usPosY, BOBBYR_GRID_FIRST_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

			//weight
			sTemp = ST::format("{3.1f}", GetWeightBasedOnMetricOption( GCM->getItem(pBobbyRayPurchase[i].usItemIndex)->getWeight() ) / (FLOAT)( 10.0 ) * pBobbyRayPurchase[i].ubNumberPurchased);
			DrawTextToScreen(sTemp, usGridX + BOBBYR_GRID_SECOND_COLUMN_X - 2, usPosY, BOBBYR_GRID_SECOND_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

			//Display Items Name
			if( pBobbyRayPurchase[i].fUsed )
			{
				uiStartLoc = BOBBYR_ITEM_DESC_FILE_SIZE * LaptopSaveInfo.BobbyRayUsedInventory[ pBobbyRayPurchase[i].usBobbyItemIndex ].usItemIndex;
			}
			else
			{
				uiStartLoc = BOBBYR_ITEM_DESC_FILE_SIZE * LaptopSaveInfo.BobbyRayInventory[ pBobbyRayPurchase[i].usBobbyItemIndex ].usItemIndex;
			}


			ST::string sText;
			if( pBobbyRayPurchase[i].fUsed )
			{
				ST::string sBack = GCM->loadEncryptedString(BOBBYRDESCFILE, uiStartLoc, BOBBYR_ITEM_DESC_NAME_SIZE);
				sText = ST::format("* {}", sBack);
			}
			else
				sText = GCM->loadEncryptedString(BOBBYRDESCFILE, uiStartLoc, BOBBYR_ITEM_DESC_NAME_SIZE);

			sText = ReduceStringLength(sText, BOBBYR_GRID_THIRD_COLUMN_WIDTH - 4, BOBBYR_ORDER_DYNAMIC_TEXT_FONT);

			DrawTextToScreen(sText, usGridX + BOBBYR_GRID_THIRD_COLUMN_X + 2, usPosY, BOBBYR_GRID_THIRD_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			//unit price
			const BobbyRayPurchaseStruct* Purchase = &pBobbyRayPurchase[i];
			DrawTextToScreen(SPrintMoney(CalcBobbyRayCost(Purchase->usItemIndex, Purchase->usBobbyItemIndex, Purchase->fUsed)), usGridX + BOBBYR_GRID_FOURTH_COLUMN_X - 2, usPosY, BOBBYR_GRID_FOURTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

			uiTotal += CalcBobbyRayCost( pBobbyRayPurchase[i].usItemIndex, pBobbyRayPurchase[i].usBobbyItemIndex, pBobbyRayPurchase[i].fUsed ) * pBobbyRayPurchase[i].ubNumberPurchased;

			DrawTextToScreen(SPrintMoney(uiTotal), usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2, usPosY, BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

			//add the current item total to the sub total
			if( fCalledFromOrderPage )
			{
				guiSubTotal += uiTotal;
			}
			else
			{
				iSubTotal += uiTotal;
			}

			usPosY += BOBBYR_GRID_ROW_OFFSET;
		}
	}

	DisplayShippingCosts( fCalledFromOrderPage, iSubTotal, usGridX, usGridY, iOrderNum );
}


static UINT32 CalcCostFromWeightOfPackage(UINT8 ubTypeOfService);


static void DisplayShippingCosts(BOOLEAN fCalledFromOrderPage, INT32 iSubTotal, UINT16 usGridX, UINT16 usGridY, INT32 iOrderNum)
{
	INT32   iShippingCost = 0;

	if( fCalledFromOrderPage )
	{
		iSubTotal = guiSubTotal;

		if( gubSelectedLight < 3 )
		{
			iShippingCost = CalcCostFromWeightOfPackage( gubSelectedLight );
		} else {
			iShippingCost = 0;
		}
	}
	else
	{
		UINT16	usStandardCost;
		auto   dest = GCM->getShippingDestination(gpNewBobbyrShipments[iOrderNum].ubDeliveryLoc);
		switch( gpNewBobbyrShipments[ iOrderNum ].ubDeliveryMethod )
		{
			case 0:
				usStandardCost = dest->chargeRateOverNight;
				break;
			case 1:
				usStandardCost = dest->chargeRate2Days;
				break;
			case 2:
				usStandardCost = dest->chargeRateStandard;
				break;

			default:
				usStandardCost = 0;
		}

		iShippingCost = (INT32)( ( gpNewBobbyrShipments[ iOrderNum ].uiPackageWeight / (FLOAT)10 ) * usStandardCost + .5 );
	}


	//erase the old area
	// blit the total Saved area onto the grid
	if( fCalledFromOrderPage )
	{
		BltVideoObject(FRAME_BUFFER, guiTotalSaveArea, 0, BOBBYR_TOTAL_SAVED_AREA_X, BOBBYR_TOTAL_SAVED_AREA_Y);
	}


	//if there is a shipment, display the s&h charge
	if( iSubTotal )
	{
		//Display the subtotal
		DrawTextToScreen(SPrintMoney(iSubTotal), usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2, usGridY + BOBBYR_SUBTOTAL_Y, BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

		//Display the shipping and handling charge
		DrawTextToScreen(SPrintMoney(iShippingCost), usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2, usGridY + BOBBYR_SHIPPING_N_HANDLE_Y, BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);


		//Display the grand total
		giGrandTotal = iSubTotal + iShippingCost;
		DrawTextToScreen(SPrintMoney(giGrandTotal), usGridX + BOBBYR_GRID_FIFTH_COLUMN_X - 2, usGridY + BOBBYR_GRAND_TOTAL_Y, BOBBYR_GRID_FIFTH_COLUMN_WIDTH, BOBBYR_ORDER_DYNAMIC_TEXT_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	}

	InvalidateRegion(STD_SCREEN_X + 333, STD_SCREEN_Y + 326, STD_SCREEN_X + 376, STD_SCREEN_Y + 400);
}


static void BtnBobbyRBackCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = guiLastBobbyRayPage;
	}
}


static void BtnBobbyRHomeCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode  = LAPTOP_MODE_BOBBY_R;
	}
}


static void SelectShippingSpeedRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubSelectedLight = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		DrawShippingSpeedLights( gubSelectedLight );
		DisplayShippingCosts( TRUE, 0, BOBBYR_ORDERGRID_X, BOBBYR_ORDERGRID_Y, -1 );
	}
}


static void DrawShippingSpeedLights(UINT8 ubSelected)
{
	UINT8 green = 0;

	for (UINT8 i=0; i<3; i++) {
		if (ubSelected == i) {
			green = 255;
		} else {
			green = 0;
		}
		ColorFillVideoSurfaceArea( FRAME_BUFFER, gShippingSpeedAreas[2*i], gShippingSpeedAreas[2*i+1], gShippingSpeedAreas[2*i]+SHIPPING_SPEED_LIGHT_WIDTH,	gShippingSpeedAreas[2*i+1]+SHIPPING_SPEED_LIGHT_HEIGHT, Get16BPPColor( FROMRGB( 0, green, 0 ) ) );
	}

	InvalidateRegion(STD_SCREEN_X + 585, STD_SCREEN_Y + 218, STD_SCREEN_X + 594, STD_SCREEN_Y + 287);
}


static void SelectConfirmOrderRegionCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	//Remove the items for Boby Rqys Inventory
	RemovePurchasedItemsFromBobbyRayInventory();

	//delete the order
	std::fill_n(BobbyRayPurchases, MAX_PURCHASE_AMOUNT, BobbyRayPurchaseStruct{});
	gubSelectedLight = 0;
	gfDestroyConfirmGrphiArea = TRUE;

	//Goto The homepage
	guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
}

static void SelectConfirmOrderRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	//Remove the items for Boby Rqys Inventory
	RemovePurchasedItemsFromBobbyRayInventory();

	//delete the order
	std::fill_n(BobbyRayPurchases, MAX_PURCHASE_AMOUNT, BobbyRayPurchaseStruct{});
	gubSelectedLight = 0;
	gfDestroyConfirmGrphiArea = TRUE;

}


static void DrawGoldRectangle(INT8 bCityNum);
static void DrawSelectedCity(UINT8 ubCityNumber);
static void SelectDropDownMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason);
static void SelectDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectScrollAreaDropDownMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason);
static void SelectScrollAreaDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectUpDownArrowOnScrollAreaRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateDestroyBobbyRDropDown(UINT8 ubDropDownAction)
{
	static UINT16 usHeight;
	static BOOLEAN fMouseRegionsCreated=FALSE;

	switch( ubDropDownAction )
	{
		case BR_DROP_DOWN_NO_ACTION:
			break;

		case BR_DROP_DOWN_CREATE:
		{
			UINT8 i;
			UINT16 usPosX, usPosY, usHeight;
			UINT16 usTemp;
			UINT16 usFontHeight = GetFontHeight( BOBBYR_DROPDOWN_FONT );

			if( fMouseRegionsCreated )
			{
				gubDropDownAction = BR_DROP_DOWN_DESTROY;

				break;
			}
			fMouseRegionsCreated = TRUE;

			usPosX = BOBBYR_CITY_START_LOCATION_X;
			usPosY = BOBBYR_CITY_START_LOCATION_Y;
			for( i=0; i< BOBBYR_NUM_DISPLAYED_CITIES; i++)
			{
				MSYS_DefineRegion(&gSelectedDropDownRegion[i], usPosX, usPosY + 4, usPosX + BOBBYR_DROP_DOWN_WIDTH - 6, usPosY + usFontHeight + 7, MSYS_PRIORITY_HIGH, CURSOR_WWW, SelectDropDownMovementCallBack, SelectDropDownRegionCallBack);
				MSYS_SetRegionUserData( &gSelectedDropDownRegion[ i ], 0, i);

				usPosY += usFontHeight + 2;
			}
			usTemp = BOBBYR_CITY_START_LOCATION_Y;
			usHeight = usPosY - usTemp + 10;


			//create the scroll bars regions
			//up arrow
			usPosX = BOBBYR_SCROLL_UP_ARROW_X;
			usPosY = BOBBYR_SCROLL_UP_ARROW_Y;
			for(i=0; i<2; i++)
			{
				MSYS_DefineRegion(&gSelectedUpDownArrowOnScrollAreaRegion[i], usPosX, usPosY, usPosX + BOBBYR_SCROLL_ARROW_WIDTH, usPosY + BOBBYR_SCROLL_ARROW_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectUpDownArrowOnScrollAreaRegionCallBack);
				MSYS_SetRegionUserData( &gSelectedUpDownArrowOnScrollAreaRegion[ i ], 0, i);
				usPosX = BOBBYR_SCROLL_DOWN_ARROW_X;
				usPosY = BOBBYR_SCROLL_DOWN_ARROW_Y;
			}

			//the scroll area itself
			usPosX = BOBBYR_SCROLL_AREA_X;
			usPosY = BOBBYR_SCROLL_UP_ARROW_Y + BOBBYR_SCROLL_ARROW_HEIGHT;
			usHeight = BOBBYR_SCROLL_AREA_HEIGHT_MINUS_ARROWS / BOBBYR_ORDER_NUM_SHIPPING_CITIES;
			for(i=0; i<BOBBYR_ORDER_NUM_SHIPPING_CITIES-1; i++)
			{
				MSYS_DefineRegion(&gSelectedScrollAreaDropDownRegion[i], usPosX, usPosY,
							(UINT16)(usPosX+BOBBYR_SCROLL_ARROW_WIDTH),
							(UINT16)(usPosY+usHeight),
							MSYS_PRIORITY_HIGH+1, CURSOR_LAPTOP_SCREEN,
							SelectScrollAreaDropDownMovementCallBack,
							SelectScrollAreaDropDownRegionCallBack);
				MSYS_SetRegionUserData( &gSelectedScrollAreaDropDownRegion[ i ], 0, i);
				usPosY += usHeight;
			}
			//put the last one down to cover the remaining area
			MSYS_DefineRegion(&gSelectedScrollAreaDropDownRegion[i], usPosX, usPosY,
						(UINT16)(usPosX+BOBBYR_SCROLL_ARROW_WIDTH),
						BOBBYR_SCROLL_DOWN_ARROW_Y,
						MSYS_PRIORITY_HIGH+1, CURSOR_LAPTOP_SCREEN,
						SelectScrollAreaDropDownMovementCallBack,
						SelectScrollAreaDropDownRegionCallBack);
			MSYS_SetRegionUserData( &gSelectedScrollAreaDropDownRegion[ i ], 0, i);

			gSelectedCloseDropDownRegion.Enable();

			//disable the clear order and accept order buttons, (their rendering interferes with the drop down graphics)
			DisableButton(guiBobbyRClearOrder);
		}
			break;

		case BR_DROP_DOWN_DESTROY:
		{
			UINT8 i;

			if( !fMouseRegionsCreated )
				break;

			for( i=0; i< BOBBYR_NUM_DISPLAYED_CITIES; i++)
				MSYS_RemoveRegion( &gSelectedDropDownRegion[i]);


			//destroy the scroll bars arrow regions
			for( i=0; i< 2; i++)
				MSYS_RemoveRegion( &gSelectedUpDownArrowOnScrollAreaRegion[i]);

			//destroy the scroll bars regions
			for( i=0; i<BOBBYR_ORDER_NUM_SHIPPING_CITIES; i++)
				MSYS_RemoveRegion( &gSelectedScrollAreaDropDownRegion[i]);

			//display the name on the title bar
			ColorFillVideoSurfaceArea( FRAME_BUFFER, BOBBYR_SHIPPING_LOC_AREA_L_X, BOBBYR_SHIPPING_LOC_AREA_T_Y, BOBBYR_SHIPPING_LOC_AREA_L_X+175,	BOBBYR_SHIPPING_LOC_AREA_T_Y+BOBBYR_DROP_DOWN_HEIGHT, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

			if( gbSelectedCity == -1 )
				DrawTextToScreen(BobbyROrderFormText[BOBBYR_SELECT_DEST], BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, BOBBYR_SHIPPING_LOC_AREA_T_Y + 3, 0, BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
			else
				DrawTextToScreen(GCM->getShippingDestination(gbSelectedCity)->name, BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, BOBBYR_SHIPPING_LOC_AREA_T_Y + 3, 0, BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

			//disable the r\close regiuon
			gSelectedCloseDropDownRegion.Disable();

			//enable the clear order and accept order buttons, (because their rendering interferes with the drop down graphics)
			EnableButton(guiBobbyRClearOrder);

			gfReDrawBobbyOrder = TRUE;
			fMouseRegionsCreated  = FALSE;
			gubDropDownAction = BR_DROP_DOWN_NO_ACTION;
		}
			break;

		case BR_DROP_DOWN_DISPLAY:
		{
			UINT8 i;
			UINT16 usPosY, usPosX;

			//Display the background for the drop down window
			ColorFillVideoSurfaceArea( FRAME_BUFFER, BOBBYR_CITY_START_LOCATION_X, BOBBYR_CITY_START_LOCATION_Y, BOBBYR_CITY_START_LOCATION_X+BOBBYR_DROP_DOWN_WIDTH,	BOBBYR_CITY_START_LOCATION_Y+BOBBYR_SCROLL_AREA_HEIGHT, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

			//
			// Place the border around the background
			//
			usHeight = BOBBYR_SCROLL_AREA_HEIGHT;

			const SGPVObject* const hImageHandle = guiDropDownBorder;

			usPosX = usPosY = 0;
			//blit top & bottom row of images
			for(i=10; i< BOBBYR_DROP_DOWN_WIDTH-10; i+=10)
			{
				//TOP ROW
				BltVideoObject(FRAME_BUFFER, hImageHandle, 1,i+BOBBYR_CITY_START_LOCATION_X, usPosY+BOBBYR_CITY_START_LOCATION_Y);
				//BOTTOM ROW
				BltVideoObject(FRAME_BUFFER, hImageHandle, 6,i+BOBBYR_CITY_START_LOCATION_X, usHeight-10+6+BOBBYR_CITY_START_LOCATION_Y);
			}

			//blit the left and right row of images
			usPosX = 0;
			for(i=10; i< usHeight-10; i+=10)
			{
				BltVideoObject(FRAME_BUFFER, hImageHandle, 3,usPosX+BOBBYR_CITY_START_LOCATION_X, i+BOBBYR_CITY_START_LOCATION_Y);
				BltVideoObject(FRAME_BUFFER, hImageHandle, 4,usPosX+BOBBYR_DROP_DOWN_WIDTH-4+BOBBYR_CITY_START_LOCATION_X, i+BOBBYR_CITY_START_LOCATION_Y);
			}

			//blt the corner images for the row
			//top left
			BltVideoObject(FRAME_BUFFER, hImageHandle, 0, 0+BOBBYR_CITY_START_LOCATION_X, usPosY+BOBBYR_CITY_START_LOCATION_Y);
			//top right
			BltVideoObject(FRAME_BUFFER, hImageHandle, 2, BOBBYR_DROP_DOWN_WIDTH-10+BOBBYR_CITY_START_LOCATION_X, usPosY+BOBBYR_CITY_START_LOCATION_Y);
			//bottom left
			BltVideoObject(FRAME_BUFFER, hImageHandle, 5, 0+BOBBYR_CITY_START_LOCATION_X, usHeight-10+BOBBYR_CITY_START_LOCATION_Y);
			//bottom right
			BltVideoObject(FRAME_BUFFER, hImageHandle, 7, BOBBYR_DROP_DOWN_WIDTH-10+BOBBYR_CITY_START_LOCATION_X, usHeight-10+BOBBYR_CITY_START_LOCATION_Y);


			DrawSelectedCity( gbSelectedCity );


			//display the scroll bars regions
			ColorFillVideoSurfaceArea( FRAME_BUFFER, BOBBYR_SCROLL_AREA_X, BOBBYR_SCROLL_AREA_Y, BOBBYR_SCROLL_AREA_X+BOBBYR_SCROLL_AREA_WIDTH,	BOBBYR_SCROLL_AREA_Y+BOBBYR_SCROLL_AREA_HEIGHT, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

			//blt right bar of scroll area
			usPosX = 0;
			for(i=10; i< BOBBYR_SCROLL_AREA_HEIGHT-10; i+=10)
			{
				BltVideoObject(FRAME_BUFFER, hImageHandle, 3,BOBBYR_SCROLL_AREA_X+BOBBYR_SCROLL_AREA_WIDTH-4, i+BOBBYR_CITY_START_LOCATION_Y);
			}

			//blit top row of images
			for(i=0; i< BOBBYR_SCROLL_AREA_WIDTH; i+=10)
			{
				//TOP ROW
				BltVideoObject(FRAME_BUFFER, hImageHandle, 1,i+BOBBYR_SCROLL_AREA_X-10, BOBBYR_SCROLL_AREA_Y);
				//BOTTOM ROW
				BltVideoObject(FRAME_BUFFER, hImageHandle, 6,i+BOBBYR_SCROLL_AREA_X-10, BOBBYR_SCROLL_AREA_Y-10+6+BOBBYR_SCROLL_AREA_HEIGHT);
			}

			//top right
			BltVideoObject(FRAME_BUFFER, hImageHandle, 2, BOBBYR_SCROLL_AREA_X+BOBBYR_SCROLL_AREA_WIDTH-10, BOBBYR_SCROLL_AREA_Y);
			//bottom right
			BltVideoObject(FRAME_BUFFER, hImageHandle, 7, BOBBYR_SCROLL_AREA_X+BOBBYR_SCROLL_AREA_WIDTH-10, BOBBYR_SCROLL_AREA_Y+BOBBYR_SCROLL_AREA_HEIGHT-10);

			//fix
			BltVideoObject(FRAME_BUFFER, hImageHandle, 4, BOBBYR_DROP_DOWN_WIDTH-4+BOBBYR_CITY_START_LOCATION_X, BOBBYR_CITY_START_LOCATION_Y+2);


			// display the up and down arrows
			//top arrow
			BltVideoObject(FRAME_BUFFER, guiGoldArrowImages, 1, BOBBYR_SCROLL_UP_ARROW_X, BOBBYR_SCROLL_UP_ARROW_Y);

			//top arrow
			BltVideoObject(FRAME_BUFFER, guiGoldArrowImages, 0, BOBBYR_SCROLL_DOWN_ARROW_X, BOBBYR_SCROLL_DOWN_ARROW_Y);

			//display the scroll rectangle
			DrawGoldRectangle( gbSelectedCity );


			InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
		}
			break;
	}
}


static void SelectDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT8 ubSelected = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		gbSelectedCity = ubSelected + gubCityAtTopOfList ;

		DrawSelectedCity( gbSelectedCity );

		gubDropDownAction = BR_DROP_DOWN_DESTROY;
	}
}


static void SelectActivateCityDroDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubDropDownAction = BR_DROP_DOWN_CREATE;
	}
}


static void SelectDropDownMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		gbSelectedCity = (UINT8)MSYS_GetRegionUserData( pRegion, 0 ) + gubCityAtTopOfList;

		gubDropDownAction = BR_DROP_DOWN_DISPLAY;

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
}


static bool IsAnythingPurchasedFromBobbyRayPage();


static void DrawSelectedCity(UINT8 ubCityNumber)
{
	UINT16 usPosY;
	UINT16 usFontHeight = GetFontHeight( BOBBYR_DROPDOWN_FONT );
	UINT8 i;

	//DEBUG: make sure it wont go over array bounds
	if( gubCityAtTopOfList+BOBBYR_NUM_DISPLAYED_CITIES > BOBBYR_ORDER_NUM_SHIPPING_CITIES )
		gubCityAtTopOfList = BOBBYR_ORDER_NUM_SHIPPING_CITIES - BOBBYR_NUM_DISPLAYED_CITIES - 1;


	//Display the list of cities
	usPosY = BOBBYR_CITY_START_LOCATION_Y + 5;
	for( i=gubCityAtTopOfList; i< gubCityAtTopOfList+BOBBYR_NUM_DISPLAYED_CITIES; i++)
	{
		DrawTextToScreen(GCM->getShippingDestination(i)->name, BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, usPosY, 0, BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		usPosY += usFontHeight + 2;
	}

	if( ubCityNumber != 255 )
		usPosY = (usFontHeight + 2) * (ubCityNumber - gubCityAtTopOfList) + BOBBYR_CITY_START_LOCATION_Y;
	else
		usPosY = (usFontHeight + 2) * (gubCityAtTopOfList) + BOBBYR_CITY_START_LOCATION_Y;


	//display the name in the list
	ColorFillVideoSurfaceArea( FRAME_BUFFER, BOBBYR_CITY_START_LOCATION_X+4, usPosY+4, BOBBYR_CITY_START_LOCATION_X+BOBBYR_DROP_DOWN_WIDTH-4,	usPosY+usFontHeight+6, Get16BPPColor( FROMRGB( 200, 169, 87 ) ) );

	SetFontShadow(NO_SHADOW);
	ST::string city = GCM->getShippingDestination(ubCityNumber == 255 ? 0 : ubCityNumber)->name;
	DrawTextToScreen(city, BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, usPosY + 5, 0, BOBBYR_DROPDOWN_FONT, BOBBYR_FONT_BLACK, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	SetFontShadow(DEFAULT_SHADOW);

	DisplayShippingLocationCity();

	if (guiBobbyRAcceptOrder)
	{
		//if there is anything to buy, dont disable the accept button
		if( IsAnythingPurchasedFromBobbyRayPage() && gbSelectedCity != -1 )
			EnableButton( guiBobbyRAcceptOrder );
	}
}


static void DisplayShippingLocationCity(void)
{
	UINT16 usPosY;

	//display the name on the title bar
	ColorFillVideoSurfaceArea( FRAME_BUFFER, BOBBYR_SHIPPING_LOC_AREA_L_X, BOBBYR_SHIPPING_LOC_AREA_T_Y, BOBBYR_SHIPPING_LOC_AREA_L_X+175,	BOBBYR_SHIPPING_LOC_AREA_T_Y+BOBBYR_DROP_DOWN_HEIGHT, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

	//if there is no city selected
	ST::string dest = (gbSelectedCity == -1 ? BobbyROrderFormText[BOBBYR_SELECT_DEST] : GCM->getShippingDestination(gbSelectedCity)->name);
	DrawTextToScreen(dest, BOBBYR_CITY_START_LOCATION_X + BOBBYR_CITY_NAME_OFFSET, BOBBYR_SHIPPING_LOC_AREA_T_Y + 3, 0, BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_DROP_DOWN_SELEC_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	DisplayShippingCosts( TRUE, 0, BOBBYR_ORDERGRID_X, BOBBYR_ORDERGRID_Y, -1 );

	if( gubDropDownAction == BR_DROP_DOWN_DISPLAY)
		return;


	//Display the shipping cost
	usPosY = BOBBYR_OVERNIGHT_EXPRESS_Y;

	ST::string sTemp = "$0";
	const ShippingDestinationModel* shippingDest = (gbSelectedCity != -1) ? GCM->getShippingDestination(gbSelectedCity) : NULL;
	if (shippingDest)
	{
		sTemp = SPrintMoney(shippingDest->chargeRateOverNight / GetWeightBasedOnMetricOption(1));
	}

	DrawTextToScreen(sTemp, BOBBYR_SHIPPING_SPEED_NUMBER_X, usPosY, BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH, BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY +=BOBBYR_GRID_ROW_OFFSET;

	if (shippingDest)
	{
		sTemp = SPrintMoney(shippingDest->chargeRate2Days / GetWeightBasedOnMetricOption(1));
	}

	DrawTextToScreen(sTemp, BOBBYR_SHIPPING_SPEED_NUMBER_X, usPosY, BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH, BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY +=BOBBYR_GRID_ROW_OFFSET;

	if (shippingDest)
	{
		sTemp = SPrintMoney(shippingDest->chargeRateStandard / GetWeightBasedOnMetricOption(1));
	}

	DrawTextToScreen(sTemp, BOBBYR_SHIPPING_SPEED_NUMBER_X, usPosY, BOBBYR_SHIPPING_SPEED_NUMBER_WIDTH, BOBBYR_DROPDOWN_FONT, BOBBYR_ORDER_DYNAMIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
}


static void SelectCloseDroDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubDropDownAction = BR_DROP_DOWN_DESTROY;
	}
}


static void RemovePurchasedItemsFromBobbyRayInventory()
{
	FOR_EACH(BobbyRayPurchaseStruct const, i, BobbyRayPurchases)
	{
		BobbyRayPurchaseStruct const& p = *i;
		if (p.ubNumberPurchased == 0) continue;
		// Is the item used?
		STORE_INVENTORY *inv = p.fUsed ? LaptopSaveInfo.BobbyRayUsedInventory : LaptopSaveInfo.BobbyRayInventory;
		UINT8&            qty            = inv[p.usBobbyItemIndex].ubQtyOnHand;
		// Remove it from Bobby Rays Inventory
		qty = qty > p.ubNumberPurchased ? qty - p.ubNumberPurchased : 0;
	}
	gfRemoveItemsFromStock = FALSE;
}


static bool IsAnythingPurchasedFromBobbyRayPage()
{
	FOR_EACH(BobbyRayPurchaseStruct const, i, BobbyRayPurchases)
	{
		if (i->ubNumberPurchased != 0) return true;
	}
	return false;
}


static void SelectTitleLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if(iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
	}
}


static void SelectScrollAreaDropDownRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT8 ubCityNum = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

		if( ubCityNum < gbSelectedCity )
		{
			gbSelectedCity--;
			if( gbSelectedCity < gubCityAtTopOfList )
				gubCityAtTopOfList--;
		}

		if( ubCityNum > gbSelectedCity )
		{
			gbSelectedCity++;
			if( ( gbSelectedCity - gubCityAtTopOfList ) >= BOBBYR_NUM_DISPLAYED_CITIES )
				gubCityAtTopOfList++;
		}

		gubDropDownAction = BR_DROP_DOWN_DISPLAY;
	}
	else if (iReason & MSYS_CALLBACK_REASON_POINTER_REPEAT)
	{
		UINT8	ubCityNum = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

		if( ubCityNum < gbSelectedCity )
		{
			gbSelectedCity--;
			if( gbSelectedCity < gubCityAtTopOfList )
				gubCityAtTopOfList--;
		}

		if( ubCityNum > gbSelectedCity )
		{
			gbSelectedCity++;
			if( ( gbSelectedCity - gubCityAtTopOfList ) >= BOBBYR_NUM_DISPLAYED_CITIES )
				gubCityAtTopOfList++;
		}

		gubDropDownAction = BR_DROP_DOWN_DISPLAY;

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
}


static void SelectScrollAreaDropDownMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if (reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if( IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMainFingerDown() )
		{
			UINT8	ubCityNum = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

			if( ubCityNum < gbSelectedCity )
			{
				gbSelectedCity = ubCityNum;
				if( gbSelectedCity < gubCityAtTopOfList )
					gubCityAtTopOfList = gbSelectedCity;
			}

			if( ubCityNum > gbSelectedCity )
			{
				gbSelectedCity = ubCityNum;
				if( ( gbSelectedCity - gubCityAtTopOfList ) >= BOBBYR_NUM_DISPLAYED_CITIES )
					gubCityAtTopOfList = gbSelectedCity - BOBBYR_NUM_DISPLAYED_CITIES + 1;
			}

			gubDropDownAction = BR_DROP_DOWN_DISPLAY;

			InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
		}
	}
}


static void SelectUpDownArrowOnScrollAreaRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP || iReason & MSYS_CALLBACK_REASON_POINTER_REPEAT)
	{
		UINT8	ubUpArrow = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

		if( ubUpArrow )
		{
			if( gbSelectedCity < BOBBYR_ORDER_NUM_SHIPPING_CITIES-1 )
			{
				gbSelectedCity++;
			}

			if( ( gbSelectedCity - gubCityAtTopOfList ) >= BOBBYR_NUM_DISPLAYED_CITIES )
			{
				gubCityAtTopOfList++;
			}
		}
		else
		{
			if( gbSelectedCity != -1 )
			{
				if( gbSelectedCity > 0 )
					gbSelectedCity--;

				if( gbSelectedCity < gubCityAtTopOfList )
					gubCityAtTopOfList--;
			}
		}

		gubDropDownAction = BR_DROP_DOWN_DISPLAY;
	}
}


static void DrawGoldRectangle(INT8 bCityNum)
{
	UINT16 usWidth, usTempHeight, usTempPosY, usHeight;
	UINT16 usPosX, usPosY;

	UINT16 temp;

	if( bCityNum == -1 )
		bCityNum = 0;

	usTempPosY = BOBBYR_SCROLL_UP_ARROW_Y;
	usTempPosY += BOBBYR_SCROLL_ARROW_HEIGHT;
	usPosX = BOBBYR_SCROLL_AREA_X;
	usWidth = BOBBYR_SCROLL_AREA_WIDTH - 5;
	usTempHeight = ( BOBBYR_SCROLL_AREA_HEIGHT - 2 * BOBBYR_SCROLL_ARROW_HEIGHT ) - 8;

	usHeight = usTempHeight / (BOBBYR_ORDER_NUM_SHIPPING_CITIES+1);

	usPosY = usTempPosY + (UINT16)( ( ( BOBBYR_SCROLL_AREA_HEIGHT - 2 * BOBBYR_SCROLL_ARROW_HEIGHT ) /  (FLOAT)(BOBBYR_ORDER_NUM_SHIPPING_CITIES +1) ) * bCityNum );

	temp = BOBBYR_SCROLL_AREA_Y + BOBBYR_SCROLL_AREA_HEIGHT - BOBBYR_SCROLL_ARROW_HEIGHT - usHeight - 1;

	if( usPosY >= temp )
		usPosY = BOBBYR_SCROLL_AREA_Y + BOBBYR_SCROLL_AREA_HEIGHT - BOBBYR_SCROLL_ARROW_HEIGHT - usHeight - 5;

	ColorFillVideoSurfaceArea( FRAME_BUFFER, BOBBYR_SCROLL_AREA_X, usPosY, BOBBYR_SCROLL_AREA_X+usWidth,	usPosY+usHeight, Get16BPPColor( FROMRGB( 186, 165, 68 ) ) );

	//display the line
	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf = l.Buffer<UINT16>();
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// draw the gold highlite line on the top and left
	LineDraw(FALSE, usPosX, usPosY, usPosX+usWidth, usPosY, Get16BPPColor( FROMRGB( 235, 222, 171 ) ), pDestBuf);
	LineDraw(FALSE, usPosX, usPosY, usPosX, usPosY+usHeight, Get16BPPColor( FROMRGB( 235, 222, 171 ) ), pDestBuf);

	// draw the shadow line on the bottom and right
	LineDraw(FALSE, usPosX, usPosY+usHeight, usPosX+usWidth, usPosY+usHeight, Get16BPPColor( FROMRGB( 65, 49, 6 ) ), pDestBuf);
	LineDraw(FALSE, usPosX+usWidth, usPosY, usPosX+usWidth, usPosY+usHeight, Get16BPPColor( FROMRGB( 65, 49, 6 ) ), pDestBuf);
}


static UINT32 CalcPackageTotalWeight();


static UINT32 CalcCostFromWeightOfPackage(UINT8 ubTypeOfService)
{
	UINT32	uiTotalWeight = 0;
	UINT16	usStandardCost=0;
	UINT32	uiTotalCost=0;

	if( gbSelectedCity == -1 )
	{
		// shipping rates unknown until destination selected
		return(0);
	}

	//Get the package's weight
	uiTotalWeight = CalcPackageTotalWeight();

	/*
	for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
	{
		//if the item was purchased
		if( BobbyRayPurchases[ i ].ubNumberPurchased )
		{
			//add the current weight to the total
			uiTotalWeight += GCM->getItem(BobbyRayPurchases[ i ].usItemIndex)->getWeight() * BobbyRayPurchases[ i ].ubNumberPurchased;
		}
	}*/
	Assert ( ubTypeOfService < 3);

	auto destination = GCM->getShippingDestination(gbSelectedCity);
	switch( ubTypeOfService )
	{
		case 0:
			usStandardCost = destination->chargeRateOverNight;
			break;
		case 1:
			usStandardCost = destination->chargeRate2Days;
			break;
		case 2:
			usStandardCost = destination->chargeRateStandard;
			break;

		default:
			usStandardCost = 0;
	}

	//Get the actual weight ( either in lbs or metric )
	//usStandardCost = (UINT16) GetWeightBasedOnMetricOption( usStandardCost );

	//if the total weight is below a set minimum amount ( 2 kg )
	if( uiTotalWeight < MIN_SHIPPING_WEIGHT )
	{
		//bring up the base cost
		uiTotalWeight = MIN_SHIPPING_WEIGHT;
	}

	uiTotalCost = (UINT32)( ( uiTotalWeight / (FLOAT)10 ) * usStandardCost + .5 );

	return( uiTotalCost );
}


static void ShutDownBobbyRNewMailOrders(void);


void BobbyRayMailOrderEndGameShutDown()
{
	ShutDownBobbyRNewMailOrders();
	/*
	LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray.clear();
	*/
}


static void ShutDownBobbyRNewMailOrders(void)
{
	gpNewBobbyrShipments.clear();
}


static INT8 CalculateOrderDelay(UINT8 ubSelectedService)
{
	INT8 bDaysAhead;

	//get the length of time to receive the shipment
	if( ubSelectedService == 0 )
	{
		bDaysAhead = OVERNIGHT_EXPRESS;
	}
	else if( ubSelectedService == 1 )
	{
		bDaysAhead = TWO_BUSINESS_DAYS;
	}
	else if( ubSelectedService == 2 )
	{
		bDaysAhead = STANDARD_SERVICE;
	}
	else
	{
		bDaysAhead = 0;
	}

	if ( gMercProfiles[ SAL ].bLife == 0 )
	{
		// Sal is dead, so Pablo is dead, so the airport is badly run
		// CJC comment: this seems really extreme!! maybe delay by 1 day randomly but that's it!
		bDaysAhead += (UINT8) Random( 5 ) + 1;
	}

	return( bDaysAhead );
}


static void AddNewBobbyRShipment(BobbyRayPurchaseStruct* pPurchaseStruct, UINT8 ubDeliveryLoc, UINT8 ubDeliveryMethod, BOOLEAN fPruchasedFromBobbyR, UINT32 uiPackageWeight);


static void PurchaseBobbyOrder(void)
{
	auto dest = GCM->getShippingDestination(gbSelectedCity);
	if (dest->canDeliver)
	{
		gfCanAcceptOrder = FALSE;

		//add the delivery
		AddNewBobbyRShipment( BobbyRayPurchases, gbSelectedCity, gubSelectedLight, TRUE, CalcPackageTotalWeight() );
	}

	//Add the transaction to the finance page
	AddTransactionToPlayersBook(BOBBYR_PURCHASE, 0, GetWorldTotalMin(), -giGrandTotal);

	//display the confirm order graphic
	gfDrawConfirmOrderGrpahic = TRUE;

	//Get rid of the city drop dowm, if it is being displayed
	gubDropDownAction = BR_DROP_DOWN_DESTROY;

	gSelectedConfirmOrderRegion.Enable();
	gfRemoveItemsFromStock = TRUE;

	gbSelectedCity = -1;
}

void AddJohnsGunShipment()
{
	BobbyRayPurchaseStruct Temp[ MAX_PURCHASE_AMOUNT ];
	INT8 bDaysAhead;

	//clear out the memory
	std::fill_n(Temp, MAX_PURCHASE_AMOUNT, BobbyRayPurchaseStruct{});

	/*
	//if we need to add more array elements for the Order Array
	if (LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray.size() <= LaptopSaveInfo.usNumberOfBobbyRayOrderUsed)
	{
		LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray.push_back(BobbyRayOrderStruct{});
		Assert(LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray.size() <= UINT8_MAX);
	}

	for (size_t cnt =0; cnt < LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray.size(); cnt++)
	{
		//get an empty element in the array
		if( !LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].fActive )
			break;
	}*/

	// want to add two guns (Automags, AUTOMAG_III), and four clips of ammo.

	Temp[0].usItemIndex = AUTOMAG_III;
	Temp[0].ubNumberPurchased = 2;
	Temp[0].bItemQuality = 100;
	Temp[0].usBobbyItemIndex = 0;// does this get used anywhere???
	Temp[0].fUsed = FALSE;

	//LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[cnt].BobbyRayPurchase[0] = Temp;

	Temp[1].usItemIndex = CLIP762N_5_AP;
	Temp[1].ubNumberPurchased = 2;
	Temp[1].bItemQuality = 5;
	Temp[1].usBobbyItemIndex = 0;// does this get used anywhere???
	Temp[1].fUsed = FALSE;

	/*
	LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[cnt].BobbyRayPurchase[1] = Temp;


	LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].ubNumberPurchases = 2;
	LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ cnt ].fActive = TRUE;
	LaptopSaveInfo.usNumberOfBobbyRayOrderUsed++;*/
	bDaysAhead = CalculateOrderDelay( 2 ) + 2;

	//add a random amount between so it arrives between 8:00 am and noon
	//AddFutureDayStrategicEvent( EVENT_BOBBYRAY_PURCHASE, (8 + Random(4) ) * 60, cnt, bDaysAhead );

	//add the delivery ( weight is not needed as it will not be displayed )
	auto dest = GCM->getPrimaryShippingDestination();
	AddNewBobbyRShipment( Temp, dest->locationId, bDaysAhead, FALSE, 0 );
}


static void ConfirmBobbyRPurchaseMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		PurchaseBobbyOrder();
	}
}

void EnterInitBobbyRayOrder()
{
	std::fill_n(BobbyRayPurchases, MAX_PURCHASE_AMOUNT, BobbyRayPurchaseStruct{});
	gubSelectedLight = 0;
	gfReDrawBobbyOrder = TRUE;
	gbSelectedCity = -1;
	gubCityAtTopOfList = 0;

	//Get rid of the city drop dowm, if it is being displayed
	gubDropDownAction = BR_DROP_DOWN_DESTROY;

	int cnt = 0;
	gShippingSpeedAreas[cnt++] = STD_SCREEN_X + 585;
	gShippingSpeedAreas[cnt++] = STD_SCREEN_Y + 218 + LAPTOP_SCREEN_WEB_DELTA_Y;
	gShippingSpeedAreas[cnt++] = STD_SCREEN_X + 585;
	gShippingSpeedAreas[cnt++] = STD_SCREEN_Y + 238 + LAPTOP_SCREEN_WEB_DELTA_Y;
	gShippingSpeedAreas[cnt++] = STD_SCREEN_X + 585;
	gShippingSpeedAreas[cnt++] = STD_SCREEN_Y + 258 + LAPTOP_SCREEN_WEB_DELTA_Y;
}


static UINT32 CalcPackageTotalWeight()
{
	UINT32 mass = 0;
	FOR_EACH(BobbyRayPurchaseStruct const, i, BobbyRayPurchases)
	{
		BobbyRayPurchaseStruct const& p = *i;
		if (p.ubNumberPurchased == 0) continue;
		mass += GCM->getItem(p.usItemIndex)->getWeight() * p.ubNumberPurchased;
	}
	return mass;
}


static void DisplayPackageWeight(void)
{
	ST::string zTemp;
	UINT32  uiTotalWeight = CalcPackageTotalWeight();
	//FLOAT  fWeight = (FLOAT)(uiTotalWeight / 10.0);

	//Display the 'Package Weight' text
	DrawTextToScreen(BobbyROrderFormText[BOBBYR_PACKAGE_WEIGHT], BOBBYR_PACKAXGE_WEIGHT_X + 8, BOBBYR_PACKAXGE_WEIGHT_Y + 4, BOBBYR_PACKAXGE_WEIGHT_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the weight
	//zTemp = ST::format("{3.1f} {}", fWeight, pMessageStrings[MSG_KILOGRAM_ABBREVIATION]);
	zTemp = ST::format("{3.1f} {}", ( GetWeightBasedOnMetricOption( uiTotalWeight ) / 10.0f ), GetWeightUnitString());
	DrawTextToScreen(zTemp, BOBBYR_PACKAXGE_WEIGHT_X + 3, BOBBYR_PACKAXGE_WEIGHT_Y + 4, BOBBYR_PACKAXGE_WEIGHT_WIDTH, BOBBYR_ORDER_STATIC_TEXT_FONT, BOBBYR_ORDER_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
}


static void BtnBobbyRGotoShipmentPageCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode  = LAPTOP_MODE_BOBBYR_SHIPMENTS;
	}
}


void CreateBobbyRayOrderTitle()
{
	// load BobbyRayTitle graphic and add it
	guiBobbyRayTitle = AddVideoObjectFromFile(LAPTOPDIR "/bobbyraytitle.sti");

	//the link to home page from the title
	MSYS_DefineRegion(&gSelectedTitleLinkRegion, BOBBYR_BOBBY_RAY_TITLE_X, BOBBYR_BOBBY_RAY_TITLE_Y,
				(BOBBYR_BOBBY_RAY_TITLE_X + BOBBYR_BOBBY_RAY_TITLE_WIDTH),
				(UINT16)(BOBBYR_BOBBY_RAY_TITLE_Y + BOBBYR_BOBBY_RAY_TITLE_HEIGHT),
				MSYS_PRIORITY_HIGH,
				CURSOR_WWW, MSYS_NO_CALLBACK, SelectTitleLinkRegionCallBack);
}


void DestroyBobbyROrderTitle()
{
	MSYS_RemoveRegion( &gSelectedTitleLinkRegion);
	DeleteVideoObject(guiBobbyRayTitle);
}


void DrawBobbyROrderTitle()
{
	BltVideoObject(FRAME_BUFFER, guiBobbyRayTitle, 0, BOBBYR_BOBBY_RAY_TITLE_X, BOBBYR_BOBBY_RAY_TITLE_Y);
}


static void AddNewBobbyRShipment(BobbyRayPurchaseStruct* pPurchaseStruct, UINT8 ubDeliveryLoc, UINT8 ubDeliveryMethod, BOOLEAN fPruchasedFromBobbyR, UINT32 uiPackageWeight)
{
	INT32 iCnt;
	INT32 iFoundSpot = -1;
	UINT8 ubItemCount=0;
	UINT8 i;
	INT8  bDaysAhead=0;
	//UINT32 uiPackageWeight;
	//gpNewBobbyrShipments.clear();

	//loop through and see if there is a free spot to insert the new order
	Assert(gpNewBobbyrShipments.size() <= INT32_MAX);
	for (iCnt = 0; iCnt < static_cast<INT32>(gpNewBobbyrShipments.size()); iCnt++)
	{
		if (!gpNewBobbyrShipments[0].fActive)// FIXME shipment iCnt instead of 0
		{
			iFoundSpot = iCnt;
			break;
		}
	}

	if( iFoundSpot == -1 )
	{
		gpNewBobbyrShipments.push_back(NewBobbyRayOrderStruct{});

		Assert(gpNewBobbyrShipments.size() <= INT32_MAX);
		iFoundSpot = static_cast<INT32>(gpNewBobbyrShipments.size()) - 1;
	}

	//memset the memory
	gpNewBobbyrShipments[ iFoundSpot ] = NewBobbyRayOrderStruct{};

	gpNewBobbyrShipments[ iFoundSpot ].fActive = TRUE;
	gpNewBobbyrShipments[ iFoundSpot ].ubDeliveryLoc = ubDeliveryLoc;
	gpNewBobbyrShipments[ iFoundSpot ].ubDeliveryMethod = ubDeliveryMethod;

	if( fPruchasedFromBobbyR )
		gpNewBobbyrShipments[ iFoundSpot ].fDisplayedInShipmentPage = TRUE;
	else
		gpNewBobbyrShipments[ iFoundSpot ].fDisplayedInShipmentPage = FALSE;

	//get the apckage weight, if the weight is "below" the minimum, use the minimum
	if(  uiPackageWeight < MIN_SHIPPING_WEIGHT )
	{
		gpNewBobbyrShipments[ iFoundSpot ].uiPackageWeight = MIN_SHIPPING_WEIGHT;
	}
	else
	{
		gpNewBobbyrShipments[ iFoundSpot ].uiPackageWeight = uiPackageWeight;
	}

	gpNewBobbyrShipments[ iFoundSpot ].uiOrderedOnDayNum = GetWorldDay();

	//count the number of purchases
	ubItemCount = 0;
	for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
	{
		//if the item was purchased
		if( pPurchaseStruct[ i ].ubNumberPurchased )
		{
			// copy the new data into the order struct
			gpNewBobbyrShipments[iFoundSpot].BobbyRayPurchase[ubItemCount] = pPurchaseStruct[i];

			//copy the purchases into the struct that will be added to the queue
			//LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[cnt].BobbyRayPurchase[ubCount] = BobbyRayPurchases[i];
			ubItemCount++;
		}
	}

	gpNewBobbyrShipments[ iFoundSpot ].ubNumberPurchases = ubItemCount;


	//get the length of time to receive the shipment
	if( fPruchasedFromBobbyR )
		bDaysAhead = CalculateOrderDelay( ubDeliveryMethod );
	else
		bDaysAhead = ubDeliveryMethod;


	//AddStrategicEvent( EVENT_BOBBYRAY_PURCHASE, uiResetTimeSec, cnt);
	AddFutureDayStrategicEvent( EVENT_BOBBYRAY_PURCHASE, (8 + Random(4) ) * 60, iFoundSpot, bDaysAhead );
}


UINT16	CountNumberOfBobbyPurchasesThatAreInTransit()
{
	UINT16	usItemCount=0;
	INT32		iCnt;

	Assert(gpNewBobbyrShipments.size() <= INT32_MAX);
	for (iCnt = 0; iCnt < static_cast<INT32>(gpNewBobbyrShipments.size()); iCnt++)
	{
		if( gpNewBobbyrShipments[iCnt].fActive )
		{
			usItemCount++;
		}
	}

	return( usItemCount );
}


void NewWayOfSavingBobbyRMailOrdersToSaveGameFile(HWFILE const hFile)
{
	INT32 iCnt;

	//Write the number of orders
	Assert(gpNewBobbyrShipments.size() <= INT32_MAX);
	INT32 numNewBobbyrShipments = static_cast<INT32>(gpNewBobbyrShipments.size());
	hFile->write(&numNewBobbyrShipments, sizeof(INT32));

	//loop through and save all the mail order slots
	for (iCnt = 0; iCnt < numNewBobbyrShipments; iCnt++)
	{
		//Write the order
		hFile->write(&gpNewBobbyrShipments[iCnt], sizeof(NewBobbyRayOrderStruct));
	}
}


void NewWayOfLoadingBobbyRMailOrdersToSaveGameFile(HWFILE const hFile)
{
	INT32 iCnt;

	//clear out the old list
	ShutDownBobbyRNewMailOrders();


	//Read the number of orders
	INT32 numNewBobbyrShipments = 0;
	hFile->read(&numNewBobbyrShipments, sizeof(INT32));

	if (numNewBobbyrShipments == 0)
	{
		gpNewBobbyrShipments.clear();
	}
	else
	{
		gpNewBobbyrShipments.assign(numNewBobbyrShipments, NewBobbyRayOrderStruct{});

		//loop through and load all the mail order slots
		for (iCnt = 0; iCnt < numNewBobbyrShipments; iCnt++)
		{
			//Read the order
			hFile->read(&gpNewBobbyrShipments[iCnt], sizeof(NewBobbyRayOrderStruct));
		}
	}
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(BobbyRMailOrder, asserts)
{
	EXPECT_EQ(sizeof(NewBobbyRayOrderStruct), 104u);
}

#endif
