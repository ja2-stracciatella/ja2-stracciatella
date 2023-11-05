#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "BobbyRShipments.h"
#include "BobbyR.h"
#include "VObject.h"
#include "Text.h"
#include "BobbyRGuns.h"
#include "Cursors.h"
#include "BobbyRMailOrder.h"
#include "WordWrap.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "Debug.h"

#include <string_theory/string>


#define BOBBYR_SHIPMENT_TITLE_TEXT_FONT		FONT14ARIAL
#define BOBBYR_SHIPMENT_TITLE_TEXT_COLOR	157

#define BOBBYR_SHIPMENT_STATIC_TEXT_FONT	FONT12ARIAL
#define BOBBYR_SHIPMENT_STATIC_TEXT_COLOR	145


#define BOBBYR_BOBBY_RAY_TITLE_X		LAPTOP_SCREEN_UL_X + 171
#define BOBBYR_BOBBY_RAY_TITLE_Y		LAPTOP_SCREEN_WEB_UL_Y + 3

#define BOBBYR_ORDER_FORM_TITLE_X		BOBBYR_BOBBY_RAY_TITLE_X
#define BOBBYR_ORDER_FORM_TITLE_Y		BOBBYR_BOBBY_RAY_TITLE_Y + 37
#define BOBBYR_ORDER_FORM_TITLE_WIDTH		159

#define BOBBYR_SHIPMENT_DELIVERY_GRID_X		LAPTOP_SCREEN_UL_X + 2
#define BOBBYR_SHIPMENT_DELIVERY_GRID_Y		BOBBYR_SHIPMENT_ORDER_GRID_Y
#define BOBBYR_SHIPMENT_DELIVERY_GRID_WIDTH	183

#define BOBBYR_SHIPMENT_ORDER_GRID_X		LAPTOP_SCREEN_UL_X + 223
#define BOBBYR_SHIPMENT_ORDER_GRID_Y		LAPTOP_SCREEN_WEB_UL_Y + 62


#define BOBBYR_SHIPMENT_BACK_BUTTON_X		(STD_SCREEN_X + 130)
#define BOBBYR_SHIPMENT_HOME_BUTTON_X		(STD_SCREEN_Y + 515)
#define BOBBYR_SHIPMENT_BUTTON_Y		(STD_SCREEN_Y + 400 + LAPTOP_SCREEN_WEB_DELTA_Y + 4)

#define BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS	13



#define BOBBYR_SHIPMENT_ORDER_NUM_X	(STD_SCREEN_X + 116)
#define BOBBYR_SHIPMENT_ORDER_NUM_START_Y	(STD_SCREEN_Y + 144)
#define BOBBYR_SHIPMENT_ORDER_NUM_WIDTH	64

#define BOBBYR_SHIPMENT_GAP_BTN_LINES	20


#define BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_X	BOBBYR_SHIPMENT_ORDER_NUM_X
#define BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_Y	(STD_SCREEN_Y + 117)

#define BOBBYR_SHIPMENT_NUM_ITEMS_X	(STD_SCREEN_X + 183)
#define BOBBYR_SHIPMENT_NUM_ITEMS_Y	BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_Y
#define BOBBYR_SHIPMENT_NUM_ITEMS_WIDTH	116


static SGPVObject* guiBobbyRShipmentGrid;

static BOOLEAN gfBobbyRShipmentsDirty = FALSE;

static INT32 giBobbyRShipmentSelectedShipment = -1;

//Back Button
static void BtnBobbyRShipmentBackCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyRShipmentBackImage;
static GUIButtonRef guiBobbyRShipmetBack;

//Home Button
static void BtnBobbyRShipmentHomeCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* giBobbyRShipmentHomeImage;
static GUIButtonRef guiBobbyRShipmentHome;


static MOUSE_REGION gSelectedPreviousShipmentsRegion[BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS];


static GUIButtonRef MakeButton(BUTTON_PICS* img, const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	const INT16 shadow_col = BOBBYR_GUNS_SHADOW_COLOR;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, BOBBYR_GUNS_BUTTON_FONT, BOBBYR_GUNS_TEXT_COLOR_ON, shadow_col, BOBBYR_GUNS_TEXT_COLOR_OFF, shadow_col, x, BOBBYR_SHIPMENT_BUTTON_Y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


static void CreatePreviousShipmentsMouseRegions(void);


void EnterBobbyRShipments()
{
	InitBobbyRWoodBackground();

	// load the Order Grid graphic and add it
	guiBobbyRShipmentGrid = AddVideoObjectFromFile(LAPTOPDIR "/bobbyray_onorder.sti");

	guiBobbyRShipmentBackImage = LoadButtonImage(LAPTOPDIR "/cataloguebutton.sti", 0, 1);
	guiBobbyRShipmetBack = MakeButton(guiBobbyRShipmentBackImage, BobbyROrderFormText[BOBBYR_BACK], BOBBYR_SHIPMENT_BACK_BUTTON_X, BtnBobbyRShipmentBackCallback);

	giBobbyRShipmentHomeImage = UseLoadedButtonImage(guiBobbyRShipmentBackImage, 0, 1);
	guiBobbyRShipmentHome = MakeButton(giBobbyRShipmentHomeImage, BobbyROrderFormText[BOBBYR_HOME], BOBBYR_SHIPMENT_HOME_BUTTON_X, BtnBobbyRShipmentHomeCallback);

	CreateBobbyRayOrderTitle();

	giBobbyRShipmentSelectedShipment = -1;

	//if there are shipments
	if (gpNewBobbyrShipments.size() != 0)
	{
		INT32 iCnt;

		//get the first shipment #
		Assert(gpNewBobbyrShipments.size() <= INT32_MAX);
		for (iCnt = 0; iCnt < static_cast<INT32>(gpNewBobbyrShipments.size()); iCnt++)
		{
			if( gpNewBobbyrShipments[iCnt].fActive )
				giBobbyRShipmentSelectedShipment = iCnt;
		}
	}

	CreatePreviousShipmentsMouseRegions();
}


static void RemovePreviousShipmentsMouseRegions(void);


void ExitBobbyRShipments()
{
	DeleteBobbyRWoodBackground();
	DestroyBobbyROrderTitle();

	delete guiBobbyRShipmentGrid;

	UnloadButtonImage( guiBobbyRShipmentBackImage );
	UnloadButtonImage( giBobbyRShipmentHomeImage );
	RemoveButton( guiBobbyRShipmetBack );
	RemoveButton( guiBobbyRShipmentHome );

	RemovePreviousShipmentsMouseRegions();
}

void HandleBobbyRShipments()
{
	if( gfBobbyRShipmentsDirty )
	{
		gfBobbyRShipmentsDirty = FALSE;

		RenderBobbyRShipments();
	}
}


static void DisplayPreviousShipments(void);
static void DisplayShipmentGrid(void);
static void DisplayShipmentTitles(void);


void RenderBobbyRShipments()
{
//  HVOBJECT hPixHandle;

	DrawBobbyRWoodBackground();

	DrawBobbyROrderTitle();

	//Output the title
	DrawTextToScreen(gzBobbyRShipmentText[BOBBYR_SHIPMENT__TITLE], BOBBYR_ORDER_FORM_TITLE_X, BOBBYR_ORDER_FORM_TITLE_Y, BOBBYR_ORDER_FORM_TITLE_WIDTH, BOBBYR_SHIPMENT_TITLE_TEXT_FONT, BOBBYR_SHIPMENT_TITLE_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	DisplayShipmentGrid();

	if( giBobbyRShipmentSelectedShipment != -1 &&
		gpNewBobbyrShipments[ giBobbyRShipmentSelectedShipment ].fActive &&
		gpNewBobbyrShipments[ giBobbyRShipmentSelectedShipment ].fDisplayedInShipmentPage )
	{
		//DisplayPurchasedItems( FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X, BOBBYR_SHIPMENT_ORDER_GRID_Y, &LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[giBobbyRShipmentSelectedShipment].BobbyRayPurchase[0], FALSE );
		DisplayPurchasedItems( FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X, BOBBYR_SHIPMENT_ORDER_GRID_Y, &gpNewBobbyrShipments[giBobbyRShipmentSelectedShipment].BobbyRayPurchase[0], FALSE, giBobbyRShipmentSelectedShipment );
	}
	else
	{
		//DisplayPurchasedItems( FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X, BOBBYR_SHIPMENT_ORDER_GRID_Y, &LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[giBobbyRShipmentSelectedShipment].BobbyRayPurchase[0], TRUE );
		DisplayPurchasedItems( FALSE, BOBBYR_SHIPMENT_ORDER_GRID_X, BOBBYR_SHIPMENT_ORDER_GRID_Y, NULL, TRUE, giBobbyRShipmentSelectedShipment );
	}

	DisplayShipmentTitles();
	DisplayPreviousShipments();

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnBobbyRShipmentBackCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R_MAILORDER;
	}
}


static void BtnBobbyRShipmentHomeCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
	}
}


static void DisplayShipmentGrid(void)
{
	// Shipment Order Grid
	BltVideoObject(FRAME_BUFFER, guiBobbyRShipmentGrid, 0, BOBBYR_SHIPMENT_DELIVERY_GRID_X, BOBBYR_SHIPMENT_DELIVERY_GRID_Y);

	// Order Grid
	BltVideoObject(FRAME_BUFFER, guiBobbyRShipmentGrid, 1, BOBBYR_SHIPMENT_ORDER_GRID_X,    BOBBYR_SHIPMENT_ORDER_GRID_Y);
}


static void DisplayShipmentTitles(void)
{
	//output the order #
	DrawTextToScreen(gzBobbyRShipmentText[BOBBYR_SHIPMENT__ORDERED_ON], BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_X, BOBBYR_SHIPMENT_SHIPMENT_ORDER_NUM_Y, BOBBYR_SHIPMENT_ORDER_NUM_WIDTH, BOBBYR_SHIPMENT_STATIC_TEXT_FONT, BOBBYR_SHIPMENT_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Output the # of items
	DrawTextToScreen(gzBobbyRShipmentText[BOBBYR_SHIPMENT__NUM_ITEMS], BOBBYR_SHIPMENT_NUM_ITEMS_X, BOBBYR_SHIPMENT_NUM_ITEMS_Y, BOBBYR_SHIPMENT_NUM_ITEMS_WIDTH, BOBBYR_SHIPMENT_STATIC_TEXT_FONT, BOBBYR_SHIPMENT_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
}


static INT32 CountNumberValidShipmentForTheShipmentsPage(void);


static void DisplayPreviousShipments(void)
{
	UINT32  uiCnt;
	ST::string zText;
	UINT16  usPosY = BOBBYR_SHIPMENT_ORDER_NUM_START_Y;
	UINT32  uiNumItems = CountNumberValidShipmentForTheShipmentsPage();
	UINT32  uiNumberItemsInShipments = 0;
	UINT32  uiItemCnt;
	UINT32  ubFontColor = BOBBYR_SHIPMENT_STATIC_TEXT_COLOR;

	//loop through all the shipments
	for( uiCnt=0; uiCnt<uiNumItems; uiCnt++ )
	{
		//if it is a valid shipment, and can be displayed at bobby r
		if( gpNewBobbyrShipments[ uiCnt ].fActive &&
				gpNewBobbyrShipments[ giBobbyRShipmentSelectedShipment ].fDisplayedInShipmentPage )
		{
			if( uiCnt == (UINT32)giBobbyRShipmentSelectedShipment )
			{
				ubFontColor = FONT_MCOLOR_WHITE;
			}
			else
			{
				ubFontColor = BOBBYR_SHIPMENT_STATIC_TEXT_COLOR;
			}

			//Display the "ordered on day num"
			zText = ST::format("{} {}", gpGameClockString, gpNewBobbyrShipments[uiCnt].uiOrderedOnDayNum);
			DrawTextToScreen(zText, BOBBYR_SHIPMENT_ORDER_NUM_X, usPosY, BOBBYR_SHIPMENT_ORDER_NUM_WIDTH, BOBBYR_SHIPMENT_STATIC_TEXT_FONT, ubFontColor, 0, CENTER_JUSTIFIED);

			uiNumberItemsInShipments = 0;

			//for( uiItemCnt=0; uiItemCnt<LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ uiCnt ].ubNumberPurchases; uiItemCnt++ )
			for( uiItemCnt=0; uiItemCnt<gpNewBobbyrShipments[ uiCnt ].ubNumberPurchases; uiItemCnt++ )
			{
				//uiNumberItemsInShipments += LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray[ uiCnt ].BobbyRayPurchase[uiItemCnt].ubNumberPurchased;
				uiNumberItemsInShipments += gpNewBobbyrShipments[ uiCnt ].BobbyRayPurchase[uiItemCnt].ubNumberPurchased;
			}

			//Display the # of items
			zText = ST::format("{}", uiNumberItemsInShipments);
			DrawTextToScreen(zText, BOBBYR_SHIPMENT_NUM_ITEMS_X, usPosY, BOBBYR_SHIPMENT_NUM_ITEMS_WIDTH, BOBBYR_SHIPMENT_STATIC_TEXT_FONT, ubFontColor, 0, CENTER_JUSTIFIED);
			usPosY += BOBBYR_SHIPMENT_GAP_BTN_LINES;
		}
	}
}


static void SelectPreviousShipmentsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreatePreviousShipmentsMouseRegions(void)
{
	UINT32 uiCnt;
	UINT16	usPosY = BOBBYR_SHIPMENT_ORDER_NUM_START_Y;
	UINT16	usWidth = BOBBYR_SHIPMENT_DELIVERY_GRID_WIDTH;
	UINT16	usHeight = GetFontHeight( BOBBYR_SHIPMENT_STATIC_TEXT_FONT );
	UINT32	uiNumItems = CountNumberOfBobbyPurchasesThatAreInTransit();

	for( uiCnt=0; uiCnt<uiNumItems; uiCnt++ )
	{
		MSYS_DefineRegion(&gSelectedPreviousShipmentsRegion[uiCnt], BOBBYR_SHIPMENT_ORDER_NUM_X, usPosY,
					(UINT16)(BOBBYR_SHIPMENT_ORDER_NUM_X+usWidth),
					(UINT16)(usPosY+usHeight),
					MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
					SelectPreviousShipmentsRegionCallBack);
		MSYS_SetRegionUserData( &gSelectedPreviousShipmentsRegion[uiCnt], 0, uiCnt);

		usPosY += BOBBYR_SHIPMENT_GAP_BTN_LINES;
	}
}


static void RemovePreviousShipmentsMouseRegions(void)
{
	UINT32 uiCnt;
	UINT32 uiNumItems = CountNumberOfBobbyPurchasesThatAreInTransit();


	for( uiCnt=0; uiCnt<uiNumItems; uiCnt++ )
	{
		MSYS_RemoveRegion( &gSelectedPreviousShipmentsRegion[uiCnt] );
	}
}


static void SelectPreviousShipmentsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		INT32 iSlotID = MSYS_GetRegionUserData( pRegion, 0 );


		if( CountNumberOfBobbyPurchasesThatAreInTransit() > iSlotID )
		{
			INT32 iCnt;
			INT32	iValidShipmentCounter=0;

			giBobbyRShipmentSelectedShipment = -1;

			//loop through and get the "x" iSlotID shipment
			Assert(gpNewBobbyrShipments.size() <= INT32_MAX);
			for (iCnt = 0; iCnt < static_cast<INT32>(gpNewBobbyrShipments.size()); iCnt++)
			{
				if( gpNewBobbyrShipments[iCnt].fActive )
				{
					if( iValidShipmentCounter == iSlotID )
					{
						giBobbyRShipmentSelectedShipment = iCnt;
					}

					iValidShipmentCounter++;
				}
			}
		}

		gfBobbyRShipmentsDirty = TRUE;
	}
}


static INT32 CountNumberValidShipmentForTheShipmentsPage(void)
{
	Assert(gpNewBobbyrShipments.size() <= INT32_MAX);
	if (gpNewBobbyrShipments.size() > BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS)
		return( BOBBYR_SHIPMENT_NUM_PREVIOUS_SHIPMENTS );
	else
		return( static_cast<INT32>(gpNewBobbyrShipments.size()) );
}
