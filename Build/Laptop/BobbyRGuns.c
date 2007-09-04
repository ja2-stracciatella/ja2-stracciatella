#include "Font.h"
#include "Laptop.h"
#include "BobbyRGuns.h"
#include "BobbyR.h"
#include "Local.h"
#include "Utilities.h"
#include "WCheck.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Interface_Items.h"
#include "Encrypted_File.h"
#include "Text.h"
#include "Store_Inventory.h"
#include "LaptopSave.h"
#include "Finances.h"
#include "AIMMembers.h"
#include "Overhead.h"
#include "Weapons.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "ScreenIDs.h"
#include "Font_Control.h"


#ifdef JA2BETAVERSION
#	include "Debug.h"
#endif


#define		BOBBYR_GRID_PIC_WIDTH							118
#define		BOBBYR_GRID_PIC_HEIGHT						69

#define		BOBBYR_GRID_PIC_X									BOBBYR_GRIDLOC_X + 3
#define		BOBBYR_GRID_PIC_Y									BOBBYR_GRIDLOC_Y + 3

#define		BOBBYR_GRID_OFFSET								72

#define		BOBBYR_ORDER_TITLE_FONT						FONT14ARIAL
#define		BOBBYR_ORDER_TEXT_FONT						FONT10ARIAL
#define		BOBBYR_ORDER_TEXT_COLOR						75

#define		BOBBYR_STATIC_TEXT_COLOR					75
#define   BOBBYR_ITEM_DESC_TEXT_FONT				FONT10ARIAL
#define   BOBBYR_ITEM_DESC_TEXT_COLOR				FONT_MCOLOR_WHITE
#define   BOBBYR_ITEM_NAME_TEXT_FONT				FONT10ARIAL
#define   BOBBYR_ITEM_NAME_TEXT_COLOR				FONT_MCOLOR_WHITE

#define		NUM_BOBBYRPAGE_MENU								6
#define		NUM_CATALOGUE_BUTTONS							5
#define		BOBBYR_NUM_WEAPONS_ON_PAGE				4

#define		BOBBYR_BRTITLE_X									LAPTOP_SCREEN_UL_X + 4
#define		BOBBYR_BRTITLE_Y									LAPTOP_SCREEN_WEB_UL_Y + 3
#define		BOBBYR_BRTITLE_WIDTH							46
#define		BOBBYR_BRTITLE_HEIGHT							42

#define		BOBBYR_TO_ORDER_TITLE_X						195
#define		BOBBYR_TO_ORDER_TITLE_Y						42 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		BOBBYR_TO_ORDER_TEXT_X						BOBBYR_TO_ORDER_TITLE_X + 75
#define		BOBBYR_TO_ORDER_TEXT_Y						33 + LAPTOP_SCREEN_WEB_DELTA_Y
#define		BOBBYR_TO_ORDER_TEXT_WIDTH				330

#define		BOBBYR_PREVIOUS_BUTTON_X					LAPTOP_SCREEN_UL_X + 5			//BOBBYR_HOME_BUTTON_X + BOBBYR_CATALOGUE_BUTTON_WIDTH + 5
#define		BOBBYR_PREVIOUS_BUTTON_Y					LAPTOP_SCREEN_WEB_UL_Y + 340		//BOBBYR_HOME_BUTTON_Y

#define		BOBBYR_NEXT_BUTTON_X							LAPTOP_SCREEN_UL_X + 412		//BOBBYR_ORDER_FORM_X + BOBBYR_ORDER_FORM_WIDTH + 5
#define		BOBBYR_NEXT_BUTTON_Y							BOBBYR_PREVIOUS_BUTTON_Y		//BOBBYR_PREVIOUS_BUTTON_Y

#define		BOBBYR_CATALOGUE_BUTTON_START_X		BOBBYR_PREVIOUS_BUTTON_X + 92 	//LAPTOP_SCREEN_UL_X + 93 - BOBBYR_CATALOGUE_BUTTON_WIDTH/2
#define		BOBBYR_CATALOGUE_BUTTON_GAP				( 318 - NUM_CATALOGUE_BUTTONS * BOBBYR_CATALOGUE_BUTTON_WIDTH) / (NUM_CATALOGUE_BUTTONS + 1) + BOBBYR_CATALOGUE_BUTTON_WIDTH + 1//80
#define		BOBBYR_CATALOGUE_BUTTON_Y					LAPTOP_SCREEN_WEB_UL_Y + 340
#define		BOBBYR_CATALOGUE_BUTTON_WIDTH			56//75

#define   BOBBYR_HOME_BUTTON_X							120
#define   BOBBYR_HOME_BUTTON_Y							400 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		BOBBYR_CATALOGUE_BUTTON_TEXT_Y		BOBBYR_CATALOGUE_BUTTON_Y + 5

#define		BOBBYR_PREVIOUS_PAGE							0
#define		BOBBYR_NEXT_PAGE									1

#define		BOBBYR_ITEM_DESC_START_X					BOBBYR_GRIDLOC_X + 172 + 5
#define		BOBBYR_ITEM_DESC_START_Y					BOBBYR_GRIDLOC_Y + 6
#define		BOBBYR_ITEM_DESC_START_WIDTH			214 - 10 + 20

#define		BOBBYR_ITEM_NAME_X								BOBBYR_GRIDLOC_X + 6
#define		BOBBYR_ITEM_NAME_Y_OFFSET					54

#define		BOBBYR_ORDER_NUM_WIDTH						15
#define		BOBBYR_ORDER_NUM_X								BOBBYR_GRIDLOC_X + 120 - BOBBYR_ORDER_NUM_WIDTH	//BOBBYR_ITEM_STOCK_TEXT_X
#define		BOBBYR_ORDER_NUM_Y_OFFSET					1

#define		BOBBYR_ITEM_WEIGHT_TEXT_X					BOBBYR_GRIDLOC_X + 409 + 3
#define		BOBBYR_ITEM_WEIGHT_TEXT_Y					3

#define		BOBBYR_ITEM_WEIGHT_NUM_X					BOBBYR_GRIDLOC_X + 429 - 2
#define		BOBBYR_ITEM_WEIGHT_NUM_Y					3
#define		BOBBYR_ITEM_WEIGHT_NUM_WIDTH			60

#define		BOBBYR_ITEM_SPEC_GAP							2

#define		BOBBYR_ITEM_COST_TEXT_X						BOBBYR_GRIDLOC_X + 125
#define		BOBBYR_ITEM_COST_TEXT_Y						BOBBYR_GRIDLOC_Y + 6
#define		BOBBYR_ITEM_COST_TEXT_WIDTH				42

#define		BOBBYR_ITEM_COST_NUM_X						BOBBYR_ITEM_COST_TEXT_X
#define		BOBBYR_ITEM_COST_NUM_Y						BOBBYR_ITEM_COST_TEXT_Y + 10

#define		BOBBYR_ITEM_STOCK_TEXT_X					BOBBYR_ITEM_COST_TEXT_X

#define		BOBBYR_ITEM_QTY_TEXT_X						BOBBYR_GRIDLOC_X + 5//BOBBYR_ITEM_COST_TEXT_X
#define		BOBBYR_ITEM_QTY_TEXT_Y						BOBBYR_ITEM_COST_TEXT_Y + 28
#define		BOBBYR_ITEM_QTY_WIDTH							95

#define		BOBBYR_ITEM_QTY_NUM_X							BOBBYR_GRIDLOC_X + 105//BOBBYR_ITEM_COST_TEXT_X + 1
#define		BOBBYR_ITEM_QTY_NUM_Y							BOBBYR_ITEM_QTY_TEXT_Y//BOBBYR_ITEM_COST_TEXT_Y + 40

#define		BOBBYR_ITEMS_BOUGHT_X							BOBBYR_GRIDLOC_X + 105 - BOBBYR_ORDER_NUM_WIDTH//BOBBYR_ITEM_QTY_NUM_X

#define		BOBBY_RAY_NOT_PURCHASED						255
#define		BOBBY_RAY_MAX_AMOUNT_OF_ITEMS_TO_PURCHASE		200

#define		BOBBYR_ORDER_FORM_X								LAPTOP_SCREEN_UL_X + 200//204
#define		BOBBYR_ORDER_FORM_Y								LAPTOP_SCREEN_WEB_UL_Y + 367
#define		BOBBYR_ORDER_FORM_WIDTH						95

#define		BOBBYR_ORDER_SUBTOTAL_X						490
#define		BOBBYR_ORDER_SUBTOTAL_Y						BOBBYR_ORDER_FORM_Y+2//BOBBYR_HOME_BUTTON_Y

#define		BOBBYR_PERCENT_FUNTCIONAL_X				BOBBYR_ORDER_SUBTOTAL_X
#define		BOBBYR_PERCENT_FUNTCIONAL_Y				BOBBYR_ORDER_SUBTOTAL_Y + 15


BobbyRayPurchaseStruct BobbyRayPurchases[ MAX_PURCHASE_AMOUNT ];

//BobbyRayOrderStruct *BobbyRayOrdersOnDeliveryArray=NULL;
//UINT8	usNumberOfBobbyRayOrderItems = 0;
//UINT8	usNumberOfBobbyRayOrderUsed = 0;


extern	BOOLEAN fExitingLaptopFlag;

UINT32		guiGunBackground;
UINT32		guiGunsGrid;
UINT32		guiBrTitle;
UINT16		gusCurWeaponIndex;
UINT8			gubCurPage;
UINT8			ubCatalogueButtonValues[] = {
							LAPTOP_MODE_BOBBY_R_GUNS,
							LAPTOP_MODE_BOBBY_R_AMMO,
							LAPTOP_MODE_BOBBY_R_ARMOR,
							LAPTOP_MODE_BOBBY_R_MISC,
							LAPTOP_MODE_BOBBY_R_USED};

UINT16		gusLastItemIndex=0;
UINT16		gusFirstItemIndex=0;
UINT8			gubNumItemsOnScreen;
UINT8			gubNumPages;

BOOLEAN		gfBigImageMouseRegionCreated;
UINT16		gusItemNumberForItemsOnScreen[ BOBBYR_NUM_WEAPONS_ON_PAGE ];


BOOLEAN		gfOnUsedPage;

UINT16		gusOldItemNumOnTopOfPage=65535;

//The menu bar at the bottom that changes to different pages
static void BtnBobbyRPageMenuCallback(GUI_BUTTON* btn, INT32 reason);
UINT32	guiBobbyRPageMenu[ NUM_CATALOGUE_BUTTONS ];
INT32		guiBobbyRPageMenuImage;

//The next and previous buttons
static void BtnBobbyRNextPreviousPageCallback(GUI_BUTTON* btn, INT32 reason);
UINT32	guiBobbyRPreviousPage;
INT32		guiBobbyRPreviousPageImage;

UINT32	guiBobbyRNextPage;
INT32		guiBobbyRNextPageImage;


// Big Image Mouse region
MOUSE_REGION    gSelectedBigImageRegion[ BOBBYR_NUM_WEAPONS_ON_PAGE ];

// The order form button
static void BtnBobbyROrderFormCallback(GUI_BUTTON* btn, INT32 reason);
UINT32	guiBobbyROrderForm;
INT32		guiBobbyROrderFormImage;

// The Home button
static void BtnBobbyRHomeButtonCallback(GUI_BUTTON* btn, INT32 reason);
UINT32	guiBobbyRHome;
INT32		guiBobbyRHomeImage;


// Link from the title
MOUSE_REGION    gSelectedTitleImageLinkRegion;


void GameInitBobbyRGuns()
{
	memset(&BobbyRayPurchases, 0, MAX_PURCHASE_AMOUNT);
}


static void EnterInitBobbyRGuns(void)
{
	memset(&BobbyRayPurchases, 0, MAX_PURCHASE_AMOUNT);
}



BOOLEAN EnterBobbyRGuns()
{
	gfBigImageMouseRegionCreated = FALSE;

	// load the background graphic and add it
	guiGunBackground = AddVideoObjectFromFile("LAPTOP/gunbackground.sti");
	CHECKF(guiGunBackground != NO_VOBJECT);

	// load the gunsgrid graphic and add it
	guiGunsGrid = AddVideoObjectFromFile("LAPTOP/gunsgrid.sti");
	CHECKF(guiGunsGrid != NO_VOBJECT);

	InitBobbyBrTitle();


	SetFirstLastPagesForNew( IC_BOBBY_GUN );
//	CalculateFirstAndLastIndexs();
/*
	if(giCurrentSubPage == 0)
		gusCurWeaponIndex = gusFirstGunIndex;
	else
		gusCurWeaponIndex = (UINT8)giCurrentSubPage;
*/
	//Draw menu bar
	InitBobbyMenuBar( IC_GUN );

	// render once
	RenderBobbyRGuns( );

//	RenderBobbyRGuns();
	return(TRUE);
}

void ExitBobbyRGuns()
{
	DeleteVideoObjectFromIndex(guiGunBackground);
	DeleteVideoObjectFromIndex(guiGunsGrid);
	DeleteBobbyBrTitle();
	DeleteBobbyMenuBar();

	DeleteMouseRegionForBigImage();

	giCurrentSubPage = gusCurWeaponIndex;
	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_GUNS;
}

void HandleBobbyRGuns()
{
}

void RenderBobbyRGuns()
{
	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES, BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiGunBackground);

	//Display title at top of page
	DisplayBobbyRBrTitle();

	BltVideoObjectFromIndex(FRAME_BUFFER, guiGunsGrid, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

//	DeleteMouseRegionForBigImage();
	DisplayItemInfo( IC_BOBBY_GUN );
	UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}



BOOLEAN DisplayBobbyRBrTitle()
{
	BltVideoObjectFromIndex(FRAME_BUFFER, guiBrTitle, 0, BOBBYR_BRTITLE_X, BOBBYR_BRTITLE_Y);

	// To Order Text
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_TO_ORDER], BOBBYR_TO_ORDER_TITLE_X, BOBBYR_TO_ORDER_TITLE_Y, 0, BOBBYR_ORDER_TITLE_FONT, BOBBYR_ORDER_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//First put a shadow behind the image
	ShadowVideoSurfaceRect( FRAME_BUFFER, BOBBYR_TO_ORDER_TEXT_X-2, BOBBYR_TO_ORDER_TEXT_Y-2, BOBBYR_TO_ORDER_TEXT_X+BOBBYR_TO_ORDER_TEXT_WIDTH, BOBBYR_TO_ORDER_TEXT_Y+31);

	//To Order text
	DisplayWrappedString(BOBBYR_TO_ORDER_TEXT_X, BOBBYR_TO_ORDER_TEXT_Y, BOBBYR_TO_ORDER_TEXT_WIDTH, 2, BOBBYR_ORDER_TEXT_FONT, BOBBYR_ORDER_TEXT_COLOR, BobbyRText[BOBBYR_GUNS_CLICK_ON_ITEMS], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	return(TRUE);
}


static void SelectTitleImageLinkRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


BOOLEAN InitBobbyBrTitle()
{
	// load the br title graphic and add it
	guiBrTitle = AddVideoObjectFromFile("LAPTOP/br.sti");
	CHECKF(guiBrTitle != NO_VOBJECT);

	//initialize the link to the homepage by clicking on the title
	MSYS_DefineRegion( &gSelectedTitleImageLinkRegion, BOBBYR_BRTITLE_X, BOBBYR_BRTITLE_Y, (BOBBYR_BRTITLE_X + BOBBYR_BRTITLE_WIDTH), (UINT16)(BOBBYR_BRTITLE_Y + BOBBYR_BRTITLE_HEIGHT), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectTitleImageLinkRegionCallBack);

	gusOldItemNumOnTopOfPage=65535;

	return(TRUE);
}

BOOLEAN DeleteBobbyBrTitle()
{
	DeleteVideoObjectFromIndex(guiBrTitle);

	MSYS_RemoveRegion( &gSelectedTitleImageLinkRegion);

	DeleteMouseRegionForBigImage();

	return(TRUE);
}


static void SelectTitleImageLinkRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
	}
}







BOOLEAN InitBobbyMenuBar(  )
{
	UINT8	i;
	UINT16	usPosX;
	UINT8		bCurMode;

	// Previous button
	guiBobbyRPreviousPageImage = LoadButtonImage("LAPTOP/PreviousButton.sti", -1,0,-1,1,-1 );
	guiBobbyRPreviousPage = CreateIconAndTextButton( guiBobbyRPreviousPageImage, BobbyRText[BOBBYR_GUNS_PREVIOUS_ITEMS], BOBBYR_GUNS_BUTTON_FONT,
													 BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
													 BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR,
													 TEXT_CJUSTIFIED,
													 BOBBYR_PREVIOUS_BUTTON_X, BOBBYR_PREVIOUS_BUTTON_Y, MSYS_PRIORITY_HIGH,
													 BtnBobbyRNextPreviousPageCallback);
	SetButtonCursor(guiBobbyRPreviousPage, CURSOR_LAPTOP_SCREEN);
	MSYS_SetBtnUserData(guiBobbyRPreviousPage, BOBBYR_PREVIOUS_PAGE);
	SpecifyDisabledButtonStyle( guiBobbyRPreviousPage, DISABLED_STYLE_SHADED );


	guiBobbyRPageMenuImage =  LoadButtonImage("LAPTOP/CatalogueButton1.sti", -1,0,-1,1,-1 );

	// Next button
	guiBobbyRNextPageImage  = LoadButtonImage("LAPTOP/NextButton.sti", -1,0,-1,1,-1 );
	guiBobbyRNextPage = CreateIconAndTextButton( guiBobbyRNextPageImage, BobbyRText[BOBBYR_GUNS_MORE_ITEMS], BOBBYR_GUNS_BUTTON_FONT,
													 BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
													 BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR,
													 TEXT_CJUSTIFIED,
													 BOBBYR_NEXT_BUTTON_X, BOBBYR_NEXT_BUTTON_Y, MSYS_PRIORITY_HIGH,
													 BtnBobbyRNextPreviousPageCallback);
	SetButtonCursor(guiBobbyRNextPage, CURSOR_LAPTOP_SCREEN);
	MSYS_SetBtnUserData(guiBobbyRNextPage, BOBBYR_NEXT_PAGE);
	SpecifyDisabledButtonStyle( guiBobbyRNextPage, DISABLED_STYLE_SHADED );


	bCurMode = 0;
	usPosX = BOBBYR_CATALOGUE_BUTTON_START_X;
	for(i=0; i<NUM_CATALOGUE_BUTTONS; i++)
	{
		// Catalogue Buttons button
		guiBobbyRPageMenu[i] = CreateIconAndTextButton( guiBobbyRPageMenuImage, BobbyRText[BOBBYR_GUNS_GUNS+i], BOBBYR_GUNS_BUTTON_FONT,
													 BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
													 BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR,
													 TEXT_CJUSTIFIED,
													 usPosX, BOBBYR_CATALOGUE_BUTTON_Y, MSYS_PRIORITY_HIGH,
													 BtnBobbyRPageMenuCallback);

		SetButtonCursor(guiBobbyRPageMenu[i], CURSOR_LAPTOP_SCREEN);

		MSYS_SetBtnUserData(guiBobbyRPageMenu[i], ubCatalogueButtonValues[bCurMode]);

		usPosX += BOBBYR_CATALOGUE_BUTTON_GAP;
		bCurMode++;
	}

	// Order Form button
	guiBobbyROrderFormImage  = LoadButtonImage("LAPTOP/OrderFormButton.sti", -1,0,-1,1,-1 );
	guiBobbyROrderForm = CreateIconAndTextButton( guiBobbyROrderFormImage, BobbyRText[BOBBYR_GUNS_ORDER_FORM], BOBBYR_GUNS_BUTTON_FONT,
													 BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
													 BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR,
													 TEXT_CJUSTIFIED,
													 BOBBYR_ORDER_FORM_X, BOBBYR_ORDER_FORM_Y, MSYS_PRIORITY_HIGH,
													 BtnBobbyROrderFormCallback);

	SetButtonCursor(guiBobbyROrderForm, CURSOR_LAPTOP_SCREEN);


	// Home button

	guiBobbyRHomeImage  = LoadButtonImage("LAPTOP/CatalogueButton.sti", -1,0,-1,1,-1 );
	guiBobbyRHome = CreateIconAndTextButton( guiBobbyRHomeImage, BobbyRText[BOBBYR_GUNS_HOME], BOBBYR_GUNS_BUTTON_FONT,
													 BOBBYR_GUNS_TEXT_COLOR_ON, BOBBYR_GUNS_SHADOW_COLOR,
													 BOBBYR_GUNS_TEXT_COLOR_OFF, BOBBYR_GUNS_SHADOW_COLOR,
													 TEXT_CJUSTIFIED,
													 BOBBYR_HOME_BUTTON_X, BOBBYR_HOME_BUTTON_Y, MSYS_PRIORITY_HIGH,
													 BtnBobbyRHomeButtonCallback);
	SetButtonCursor(guiBobbyRHome, CURSOR_LAPTOP_SCREEN);

	return(TRUE);
}

BOOLEAN DeleteBobbyMenuBar()
{
	UINT8	i;

	UnloadButtonImage( guiBobbyRNextPageImage );
	UnloadButtonImage( guiBobbyRPreviousPageImage );
	UnloadButtonImage( guiBobbyRPageMenuImage );
	UnloadButtonImage( guiBobbyROrderFormImage );
	UnloadButtonImage( guiBobbyRHomeImage );

	RemoveButton( guiBobbyRNextPage );
	RemoveButton( guiBobbyRPreviousPage );
	RemoveButton( guiBobbyROrderForm );
	RemoveButton( guiBobbyRHome );

	for(i=0; i<NUM_CATALOGUE_BUTTONS; i++)
	{
		RemoveButton( guiBobbyRPageMenu[i] );
	}

	return(TRUE);
}


static void BtnBobbyRPageMenuCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UpdateButtonText(guiCurrentLaptopMode);
		guiCurrentLaptopMode = MSYS_GetBtnUserData(btn);
	}
}


static void BtnBobbyRNextPreviousPageCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//if previous screen
		UINT32 bNewValue = MSYS_GetBtnUserData(btn);
		if( bNewValue == BOBBYR_PREVIOUS_PAGE)
		{
			if( gubCurPage > 0 )
				gubCurPage--;
		}
		//else next screen
		else
		{
			if( gubCurPage < gubNumPages-1 )
				gubCurPage++;
		}


		DeleteMouseRegionForBigImage();

		fReDrawScreenFlag = TRUE;
		fPausedReDrawScreenFlag = TRUE;
	}
}


static void CalcFirstIndexForPage(STORE_INVENTORY* pInv, UINT32 uiItemClass);
static UINT32 CalculateTotalPurchasePrice(void);
static void CreateMouseRegionForBigImage(UINT16 usPosY, UINT8 ubCount, INT16* pItemNumbers);
static void DisableBobbyRButtons(void);
static BOOLEAN DisplayAmmoInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static BOOLEAN DisplayArmourInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static BOOLEAN DisplayBigItemImage(UINT16 usIndex, UINT16 PosY);
static BOOLEAN DisplayGunInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static void DisplayItemNameAndInfo(UINT16 usPosY, UINT16 usIndex, UINT16 usBobbyIndex, BOOLEAN fUsed);
static BOOLEAN DisplayMiscInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static BOOLEAN DisplayNonGunWeaponInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static void OutOfStockMessageBoxCallBack(UINT8 bExitValue);


BOOLEAN DisplayItemInfo(UINT32 uiItemClass)
{
	UINT16	i;
	UINT8		ubCount=0;
	UINT16	PosY, usTextPosY;
	UINT16	usItemIndex;
	wchar_t	sDollarTemp[60];
	wchar_t	sTemp[60];
	INT16		pItemNumbers[ BOBBYR_NUM_WEAPONS_ON_PAGE ];

	PosY = BOBBYR_GRID_PIC_Y;
	usTextPosY = BOBBYR_ITEM_DESC_START_Y;

//	InitFirstAndLastGlobalIndex( uiItemClass );

	//if there are no items then return
	if( gusFirstItemIndex == BOBBYR_NO_ITEMS )
	{
		if( fExitingLaptopFlag )
			return( TRUE );

		DisableBobbyRButtons();

		//Display a popup saying we are out of stock
		DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, BobbyRText[ BOBBYR_NO_MORE_STOCK ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, OutOfStockMessageBoxCallBack );
		return( TRUE );
	}


	if( uiItemClass == BOBBYR_USED_ITEMS )
		CalcFirstIndexForPage( LaptopSaveInfo.BobbyRayUsedInventory, uiItemClass );
	else
		CalcFirstIndexForPage( LaptopSaveInfo.BobbyRayInventory, uiItemClass );

	DisableBobbyRButtons();

	if( gusOldItemNumOnTopOfPage != gusCurWeaponIndex )
	{
		DeleteMouseRegionForBigImage();

	}



	for(i=gusCurWeaponIndex; ((i<=gusLastItemIndex) && (ubCount < 4)); i++)
	{
		if( uiItemClass == BOBBYR_USED_ITEMS )
		{
			//If there is not items in stock
			if( LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnHand == 0 )
				continue;

			usItemIndex = LaptopSaveInfo.BobbyRayUsedInventory[ i ].usItemIndex;
			gfOnUsedPage = TRUE;
		}
		else
		{
			//If there is not items in stock
			if( LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnHand == 0 )
				continue;

			usItemIndex = LaptopSaveInfo.BobbyRayInventory[ i ].usItemIndex;
			gfOnUsedPage = FALSE;
		}

		// skip items that aren't of the right item class
		if( ! ( Item[ usItemIndex ].usItemClass & uiItemClass ) )
		{
			continue;
		}


		pItemNumbers[ ubCount ] = usItemIndex;

		switch( Item[ usItemIndex ].usItemClass )
		{
			case IC_GUN:
			case IC_LAUNCHER:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage( usItemIndex, PosY);

				//Display Items Name
				DisplayItemNameAndInfo(usTextPosY, usItemIndex, i, gfOnUsedPage);

				DisplayGunInfo(usItemIndex, usTextPosY, gfOnUsedPage, i);

				PosY += BOBBYR_GRID_OFFSET;
				usTextPosY += BOBBYR_GRID_OFFSET;
				ubCount++;
				break;

			case IC_AMMO:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage( usItemIndex, PosY);

				//Display Items Name
				DisplayItemNameAndInfo(usTextPosY, usItemIndex, i, gfOnUsedPage);

				DisplayAmmoInfo( usItemIndex, usTextPosY, gfOnUsedPage, i);

				PosY += BOBBYR_GRID_OFFSET;
				usTextPosY += BOBBYR_GRID_OFFSET;
				ubCount++;
				break;

			case IC_ARMOUR:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage( usItemIndex, PosY);

				//Display Items Name
				DisplayItemNameAndInfo(usTextPosY, usItemIndex, i, gfOnUsedPage);

				DisplayArmourInfo( usItemIndex, usTextPosY, gfOnUsedPage, i);

				PosY += BOBBYR_GRID_OFFSET;
				usTextPosY += BOBBYR_GRID_OFFSET;
				ubCount++;
				break;

			case IC_BLADE:
			case IC_THROWING_KNIFE:
			case IC_PUNCH:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage( usItemIndex, PosY);

				//Display Items Name
				DisplayItemNameAndInfo(usTextPosY, usItemIndex, i, gfOnUsedPage);

				DisplayNonGunWeaponInfo(usItemIndex, usTextPosY, gfOnUsedPage, i);

				PosY += BOBBYR_GRID_OFFSET;
				usTextPosY += BOBBYR_GRID_OFFSET;
				ubCount++;
				break;

			case IC_GRENADE:
			case IC_BOMB:
			case IC_MISC:
			case IC_MEDKIT:
			case IC_KIT:
			case IC_FACE:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage( usItemIndex, PosY);

				//Display Items Name
				DisplayItemNameAndInfo(usTextPosY, usItemIndex, i, gfOnUsedPage);

				DisplayMiscInfo( usItemIndex, usTextPosY, gfOnUsedPage, i);

				PosY += BOBBYR_GRID_OFFSET;
				usTextPosY += BOBBYR_GRID_OFFSET;
				ubCount++;
				break;
		}
	}

	if( gusOldItemNumOnTopOfPage != gusCurWeaponIndex )
	{
		CreateMouseRegionForBigImage(BOBBYR_GRID_PIC_Y, ubCount, pItemNumbers );

		gusOldItemNumOnTopOfPage = gusCurWeaponIndex;
	}

	//Display the subtotal at the bottom of the screen
	SPrintMoney(sDollarTemp, CalculateTotalPurchasePrice());
	swprintf( sTemp, lengthof(sTemp), L"%ls %ls", BobbyRText[BOBBYR_GUNS_SUB_TOTAL], sDollarTemp );
	DrawTextToScreen(sTemp, BOBBYR_ORDER_SUBTOTAL_X, BOBBYR_ORDER_SUBTOTAL_Y, 0, BOBBYR_ORDER_TITLE_FONT, BOBBYR_ORDER_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | TEXT_SHADOWED);

	//Display the Used item disclaimer
	if( gfOnUsedPage )
	{
		DrawTextToScreen(BobbyRText[BOBBYR_GUNS_PERCENT_FUNCTIONAL], BOBBYR_PERCENT_FUNTCIONAL_X, BOBBYR_PERCENT_FUNTCIONAL_Y, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ORDER_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | TEXT_SHADOWED);
	}

	return(TRUE);
}


static UINT16 DisplayCaliber(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayCostAndQty(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight, UINT16 usBobbyIndex, BOOLEAN fUsed);
static UINT16 DisplayDamage(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayMagazine(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayRange(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayRof(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayWeight(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);


static BOOLEAN DisplayGunInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16	usHeight;
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
	//DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	usHeight = usTextPosY;
	//Display the weight, caliber, mag, rng, dam, rof text

	//Weight
	usHeight = DisplayWeight(usHeight, usIndex, usFontHeight);

	//Caliber
	usHeight = DisplayCaliber(usHeight, usIndex, usFontHeight);

	//Magazine
	usHeight = DisplayMagazine(usHeight, usIndex, usFontHeight);

	//Range
	usHeight = DisplayRange(usHeight, usIndex, usFontHeight);

	//Damage
	usHeight = DisplayDamage(usHeight, usIndex, usFontHeight);

	//ROF
	usHeight = DisplayRof(usHeight, usIndex, usFontHeight);

	//Display the Cost and the qty bought and on hand
	usHeight = DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);

	return(TRUE);
}


static BOOLEAN DisplayNonGunWeaponInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16	usHeight;
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
//	DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	usHeight = usTextPosY;
	//Display the weight, caliber, mag, rng, dam, rof text

	//Weight
	usHeight = DisplayWeight(usHeight, usIndex, usFontHeight);

	//Damage
	usHeight = DisplayDamage(usHeight, usIndex, usFontHeight);

	//Display the Cost and the qty bought and on hand
	usHeight = DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);

	return(TRUE);
}


static BOOLEAN DisplayAmmoInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16	usHeight;
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
//	DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	usHeight = usTextPosY;
	//Display the weight, caliber, mag, rng, dam, rof text

	//Caliber
	usHeight = DisplayCaliber(usHeight, usIndex, usFontHeight);

	//Magazine
//	usHeight = DisplayMagazine(usHeight, usIndex, usFontHeight);

	//Display the Cost and the qty bought and on hand
	usHeight = DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);

	return(TRUE);
}


static BOOLEAN DisplayBigItemImage(UINT16 usIndex, UINT16 PosY)
{
	INT16 PosX = BOBBYR_GRID_PIC_X;

	const INVTYPE* pItem = &Item[usIndex];
	UINT32 uiImage = LoadTileGraphicForItem(pItem);

	//center picture in frame
	const ETRLEObject* pTrav = GetVideoObjectETRLESubregionProperties(uiImage, 0);
	UINT32 usWidth = pTrav->usWidth;
	INT16  sCenX   = PosX + abs(BOBBYR_GRID_PIC_WIDTH - usWidth) / 2 - pTrav->sOffsetX;
	INT16  sCenY   = PosY + 8;

	//blt the shadow of the item
	BltVideoObjectOutlineShadowFromIndex(FRAME_BUFFER, uiImage, 0, sCenX - 2, sCenY + 2);

	BltVideoObjectFromIndex(FRAME_BUFFER, uiImage, 0, sCenX, sCenY);
	DeleteVideoObjectFromIndex(uiImage);

	return(TRUE);
}


static BOOLEAN DisplayArmourInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16	usHeight;
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
//	DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	usHeight = usTextPosY;
	//Display the weight, caliber, mag, rng, dam, rof text

	//Weight
	usHeight = DisplayWeight(usHeight, usIndex, usFontHeight);

	//Display the Cost and the qty bought and on hand
	usHeight = DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);

	return(TRUE);
}


static BOOLEAN DisplayMiscInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
//	DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	//Display the Cost and the qty bought and on hand
	DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);

	return(TRUE);
}


static UINT8 CheckIfItemIsPurchased(UINT16 usItemNumber);


static UINT16 DisplayCostAndQty(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight, UINT16 usBobbyIndex, BOOLEAN fUsed)
{
	wchar_t	sTemp[20];
//	UINT8	ubPurchaseNumber;

	//
	//Display the cost and the qty
	//

	//Display the cost
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_COST], BOBBYR_ITEM_COST_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += usFontHeight + 2;

	SPrintMoney(sTemp, CalcBobbyRayCost(usIndex, usBobbyIndex, fUsed));
	DrawTextToScreen(sTemp, BOBBYR_ITEM_COST_NUM_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	//Display Weight Number
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_WGHT], BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	swprintf( sTemp, lengthof(sTemp), L"%3.2f", GetWeightBasedOnMetricOption( Item[ usIndex ].ubWeight ) / (FLOAT)( 10.0 ) );
	DrawTextToScreen(sTemp, BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	//Display the # In Stock
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_IN_STOCK], BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += usFontHeight + 2;

	if( fUsed )
		swprintf(sTemp, lengthof(sTemp), L"% 4d", LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyIndex ].ubQtyOnHand);
	else
		swprintf(sTemp, lengthof(sTemp), L"% 4d", LaptopSaveInfo.BobbyRayInventory[ usBobbyIndex ].ubQtyOnHand);

	DrawTextToScreen(sTemp, BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	return(usPosY);
}


static UINT16 DisplayRof(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	wchar_t	sTemp[20];

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_ROF], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	if( WeaponROF[ usIndex ] == -1 )
		swprintf(sTemp, lengthof(sTemp), L"? %ls", pMessageStrings[ MSG_RPM ] );
	else
		swprintf(sTemp, lengthof(sTemp), L"%3d/%ls", WeaponROF[ usIndex ], pMessageStrings[ MSG_MINUTE_ABBREVIATION ]);


	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayDamage(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	wchar_t	sTemp[20];

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_DAMAGE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	swprintf(sTemp, lengthof(sTemp), L"%4d", Weapon[ usIndex ].ubImpact);
	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayRange(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	wchar_t	sTemp[20];

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_RANGE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	swprintf(sTemp, lengthof(sTemp), L"%3d %ls", Weapon[ usIndex ].usRange, pMessageStrings[ MSG_METER_ABBREVIATION ] );
	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayMagazine(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	wchar_t	sTemp[20];

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_MAGAZINE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	swprintf(sTemp, lengthof(sTemp), L"%3d %ls", Weapon[usIndex].ubMagSize, pMessageStrings[ MSG_ROUNDS_ABBREVIATION ] );
	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayCaliber(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	CHAR16	zTemp[128];
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_CALIBRE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//	if ammo is begin drawn
	if( Item[ usIndex].usItemClass == IC_AMMO )
	{
		wcslcpy(zTemp, BobbyRayAmmoCaliber[Magazine[Item[usIndex].ubClassIndex].ubCalibre], lengthof(zTemp));
	}
	else
	{
		//else a gun is being displayed
		wcslcpy(zTemp, BobbyRayAmmoCaliber[Weapon[Item[usIndex].ubClassIndex].ubCalibre], lengthof(zTemp));
	}

	ReduceStringLength(zTemp, lengthof(zTemp), BOBBYR_GRID_PIC_WIDTH, BOBBYR_ITEM_NAME_TEXT_FONT);
	DrawTextToScreen(zTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayWeight(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	wchar_t	sTemp[20];

	//display the 'weight' string
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_WEIGHT], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	swprintf(sTemp, lengthof(sTemp), L"%3.2f %ls", GetWeightBasedOnMetricOption(Item[ usIndex ].ubWeight)/10, GetWeightUnitString() );
	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static void DisplayItemNameAndInfo(UINT16 usPosY, UINT16 usIndex, UINT16 usBobbyIndex, BOOLEAN fUsed)
{
	wchar_t	sText[400];
	wchar_t	sTemp[20];
	UINT32	uiStartLoc=0;

	UINT8	ubPurchaseNumber;

		//Display Items Name
	uiStartLoc = BOBBYR_ITEM_DESC_FILE_SIZE * usIndex;
	LoadEncryptedDataFromFile(BOBBYRDESCFILE, sText, uiStartLoc, BOBBYR_ITEM_DESC_NAME_SIZE);
	ReduceStringLength(sText, lengthof(sText), BOBBYR_GRID_PIC_WIDTH - 6, BOBBYR_ITEM_NAME_TEXT_FONT);
	DrawTextToScreen(sText, BOBBYR_ITEM_NAME_X, usPosY + BOBBYR_ITEM_NAME_Y_OFFSET, 0, BOBBYR_ITEM_NAME_TEXT_FONT, BOBBYR_ITEM_NAME_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//number bought
	//Display the # bought
	ubPurchaseNumber = CheckIfItemIsPurchased(usBobbyIndex);
	if( ubPurchaseNumber != BOBBY_RAY_NOT_PURCHASED)
	{
		DrawTextToScreen(BobbyRText[BOBBYR_GUNS_QTY_ON_ORDER], BOBBYR_ITEM_QTY_TEXT_X, usPosY, BOBBYR_ITEM_QTY_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

		if( ubPurchaseNumber != BOBBY_RAY_NOT_PURCHASED)
		{
			swprintf(sTemp, lengthof(sTemp), L"% 4d", BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased);
			DrawTextToScreen(sTemp, BOBBYR_ITEMS_BOUGHT_X, usPosY, 0, FONT14ARIAL, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}
	}




	//if it's a used item, display how damaged the item is
	if( fUsed )
	{
		swprintf(sTemp, lengthof(sTemp), L"*%3d%%", LaptopSaveInfo.BobbyRayUsedInventory[usBobbyIndex].ubItemQuality);
		DrawTextToScreen(sTemp, BOBBYR_ITEM_NAME_X - 2, usPosY - BOBBYR_ORDER_NUM_Y_OFFSET, BOBBYR_ORDER_NUM_WIDTH, BOBBYR_ITEM_NAME_TEXT_FONT, BOBBYR_ITEM_NAME_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	//Display Items description
	uiStartLoc = (BOBBYR_ITEM_DESC_FILE_SIZE * usIndex) + BOBBYR_ITEM_DESC_NAME_SIZE;
	LoadEncryptedDataFromFile(BOBBYRDESCFILE, sText, uiStartLoc, BOBBYR_ITEM_DESC_INFO_SIZE);
	DisplayWrappedString(BOBBYR_ITEM_DESC_START_X, usPosY, BOBBYR_ITEM_DESC_START_WIDTH, 2, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
}

/*
void InitFirstAndLastGlobalIndex(UINT32 uiItemClass)
{
	switch(uiItemClass)
	{
		case IC_BOBBY_GUN:
			gusLastItemIndex = gusLastGunIndex ;
			gusFirstItemIndex = gusFirstGunIndex;
			gubNumPages = gubNumGunPages;
			break;
		case IC_BOBBY_MISC:
			gusLastItemIndex = gusLastMiscIndex ;
			gusFirstItemIndex = gusFirstMiscIndex;
			gubNumPages = gubNumMiscPages;
			break;
		case IC_AMMO:
			gusLastItemIndex = gusLastAmmoIndex ;
			gusFirstItemIndex = gusFirstAmmoIndex;
			gubNumPages = gubNumAmmoPages;
			break;
		case IC_ARMOUR:
			gusLastItemIndex = gusLastArmourIndex;
			gusFirstItemIndex = gusFirstArmourIndex;
			gubNumPages = gubNumArmourPages;
			break;
		case BOBBYR_USED_ITEMS:
			gusLastItemIndex = gusLastUsedIndex;
			gusFirstItemIndex = gusFirstUsedIndex;
			gubNumPages = gubNumUsedPages;
			break;
		default:
			Assert(0);
			break;
	}
}

void CalculateFirstAndLastIndexs()
{
	//Get the first and last gun index
	SetFirstLastPagesForNew( IC_BOBBY_GUN, &gusFirstGunIndex, &gusLastGunIndex, &gubNumGunPages );

	//Get the first and last misc index
	SetFirstLastPagesForNew( IC_BOBBY_MISC, &gusFirstMiscIndex, &gusLastMiscIndex, &gubNumMiscPages );

	//Get the first and last Ammo index
	SetFirstLastPagesForNew( IC_AMMO, &gusFirstAmmoIndex, &gusLastAmmoIndex, &gubNumAmmoPages );

	//Get the first and last Armour index
	SetFirstLastPagesForNew( IC_ARMOUR, &gusFirstArmourIndex, &gusLastArmourIndex, &gubNumArmourPages );

	//Get the first and last Used index
	SetFirstLastPagesForUsed( &gusFirstUsedIndex, &gusLastUsedIndex, &gubNumUsedPages );
}
*/


//Loops through Bobby Rays Inventory to find the first and last index
void SetFirstLastPagesForNew( UINT32 uiClassMask )
{
	UINT16 i;
	INT16	sFirst = -1;
	INT16	sLast = -1;
	UINT8	ubNumItems=0;

	gubCurPage = 0;

	//First loop through to get the first and last index indexs
	for(i=0; i<MAXITEMS; i++)
	{
		//If we have some of the inventory on hand
		if( LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnHand != 0 )
		{
			if( Item[ LaptopSaveInfo.BobbyRayInventory[ i ].usItemIndex ].usItemClass & uiClassMask )
			{
				ubNumItems++;

				if( sFirst == -1 )
					sFirst = i;
				sLast = i;
			}
		}
	}

	if( ubNumItems == 0 )
	{
		gusFirstItemIndex = BOBBYR_NO_ITEMS;
		gusLastItemIndex = BOBBYR_NO_ITEMS;
		gubNumPages = 0;
		return;
	}

	gusFirstItemIndex = (UINT16)sFirst;
	gusLastItemIndex = (UINT16)sLast;
	gubNumPages = (UINT8)( ubNumItems / (FLOAT)BOBBYR_NUM_WEAPONS_ON_PAGE );
	if( (ubNumItems % BOBBYR_NUM_WEAPONS_ON_PAGE ) != 0 )
		gubNumPages += 1;
}

//Loops through Bobby Rays Used Inventory to find the first and last index
void SetFirstLastPagesForUsed()
{
	UINT16 i;
	INT16	sFirst = -1;
	INT16	sLast = -1;
	UINT8	ubNumItems=0;

	gubCurPage = 0;

	//First loop through to get the first and last index indexs
	for(i=0; i<MAXITEMS; i++)
	{
		//If we have some of the inventory on hand
		if( LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnHand != 0 )
		{
			ubNumItems++;

			if( sFirst == -1 )
				sFirst = i;
			sLast = i;
		}
	}
	if( sFirst == -1 )
	{
		gusFirstItemIndex = BOBBYR_NO_ITEMS;
		gusLastItemIndex = BOBBYR_NO_ITEMS;
		gubNumPages = 0;
		return;
	}

	gusFirstItemIndex = (UINT16)sFirst;
	gusLastItemIndex = (UINT16)sLast;
	gubNumPages = (UINT8)( ubNumItems / (FLOAT)BOBBYR_NUM_WEAPONS_ON_PAGE );
	if( (ubNumItems % BOBBYR_NUM_WEAPONS_ON_PAGE ) != 0 )
		gubNumPages += 1;
}


static UINT8 CheckPlayersInventoryForGunMatchingGivenAmmoID(INT16 sItemID);
static void SelectBigImageRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateMouseRegionForBigImage(UINT16 usPosY, UINT8 ubCount, INT16* pItemNumbers)
{
	UINT8	i;
	CHAR16		zItemName[ SIZE_ITEM_NAME ];
	UINT8			ubItemCount=0;

	if( gfBigImageMouseRegionCreated )
		return;

	for(i=0; i<ubCount; i++)
	{
		//Mouse region for the Big Item Image
		MSYS_DefineRegion( &gSelectedBigImageRegion[ i ], BOBBYR_GRID_PIC_X, usPosY, (BOBBYR_GRID_PIC_X + BOBBYR_GRID_PIC_WIDTH), (UINT16)(usPosY + BOBBYR_GRID_PIC_HEIGHT), MSYS_PRIORITY_HIGH,
								 CURSOR_WWW, MSYS_NO_CALLBACK, SelectBigImageRegionCallBack);
		MSYS_SetRegionUserData( &gSelectedBigImageRegion[ i ], 0, i);

		//specify the help text only if the items is ammo
		if( Item[ pItemNumbers[ i ] ].usItemClass == IC_AMMO )
		{
			//and only if the user has an item that can use the particular type of ammo
			ubItemCount = CheckPlayersInventoryForGunMatchingGivenAmmoID( pItemNumbers[ i ] );
			if( ubItemCount != 0 )
			{
				swprintf( zItemName, lengthof(zItemName), L"%ls %d %ls",BobbyRText[BOBBYR_GUNS_NUM_GUNS_THAT_USE_AMMO_1], ubItemCount, BobbyRText[BOBBYR_GUNS_NUM_GUNS_THAT_USE_AMMO_2] );
			}
			else
				zItemName[0] = '\0';
		}
		else
			zItemName[0] = '\0';

		SetRegionFastHelpText( &gSelectedBigImageRegion[ i ], zItemName );

		usPosY += BOBBYR_GRID_OFFSET;
	}

	gubNumItemsOnScreen = ubCount;
	gfBigImageMouseRegionCreated = TRUE;
}

void DeleteMouseRegionForBigImage()
{
	UINT8 i;

	if( !gfBigImageMouseRegionCreated )
		return;

	for(i=0; i<gubNumItemsOnScreen; i++)
		MSYS_RemoveRegion( &gSelectedBigImageRegion[i]);

	gfBigImageMouseRegionCreated = FALSE;
	gusOldItemNumOnTopOfPage = 65535;
	gubNumItemsOnScreen = 0;
}


static void PurchaseBobbyRayItem(UINT16 usItemNumber);
static void UnPurchaseBobbyRayItem(UINT16 usItemNumber);


static void SelectBigImageRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT16 usItemNum = (UINT16)MSYS_GetRegionUserData( pRegion, 0 );

		PurchaseBobbyRayItem( gusItemNumberForItemsOnScreen[ usItemNum] );

		fReDrawScreenFlag = TRUE;
		fPausedReDrawScreenFlag = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		UINT16 usItemNum = (UINT16)MSYS_GetRegionUserData( pRegion, 0 );

		UnPurchaseBobbyRayItem( gusItemNumberForItemsOnScreen[ usItemNum] );
		fReDrawScreenFlag = TRUE;
		fPausedReDrawScreenFlag = TRUE;
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
		UINT16 usItemNum = (UINT16)MSYS_GetRegionUserData( pRegion, 0 );

		PurchaseBobbyRayItem( gusItemNumberForItemsOnScreen[ usItemNum] );
		fReDrawScreenFlag = TRUE;
		fPausedReDrawScreenFlag = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_REPEAT)
	{
		UINT16 usItemNum = (UINT16)MSYS_GetRegionUserData( pRegion, 0 );

		UnPurchaseBobbyRayItem( gusItemNumberForItemsOnScreen[ usItemNum] );
		fReDrawScreenFlag = TRUE;
		fPausedReDrawScreenFlag = TRUE;
	}
}


static UINT8 GetNextPurchaseNumber(void);

#ifdef JA2BETAVERSION
static void ReportBobbyROrderError(UINT16 usItemNumber, UINT8 ubPurchaseNum, UINT8 ubQtyOnHand, UINT8 ubNumPurchasing);
#endif


static void PurchaseBobbyRayItem(UINT16 usItemNumber)
{
	UINT8	ubPurchaseNumber;

	ubPurchaseNumber = CheckIfItemIsPurchased(usItemNumber);

	//if we are in the used page
	if( guiCurrentLaptopMode == LAPTOP_MODE_BOBBY_R_USED )
	{
		//if there is enough inventory in stock to cover the purchase
		if( ubPurchaseNumber == BOBBY_RAY_NOT_PURCHASED || LaptopSaveInfo.BobbyRayUsedInventory[ usItemNumber ].ubQtyOnHand >= ( BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased + 1) )
		{
			// If the item has not yet been purchased
			if( ubPurchaseNumber == BOBBY_RAY_NOT_PURCHASED )
			{
				ubPurchaseNumber = GetNextPurchaseNumber();

				if( ubPurchaseNumber != BOBBY_RAY_NOT_PURCHASED )
				{
					BobbyRayPurchases[ ubPurchaseNumber ].usItemIndex = LaptopSaveInfo.BobbyRayUsedInventory[ usItemNumber ].usItemIndex;
					BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased = 1;
					BobbyRayPurchases[ ubPurchaseNumber ].bItemQuality = LaptopSaveInfo.BobbyRayUsedInventory[ usItemNumber ].ubItemQuality;
					BobbyRayPurchases[ ubPurchaseNumber ].usBobbyItemIndex = usItemNumber;
					BobbyRayPurchases[ ubPurchaseNumber ].fUsed = TRUE;
				}
				else
				{
					//display error popup because the player is trying to purchase more thenn 10 items
					DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, BobbyRText[ BOBBYR_MORE_THEN_10_PURCHASES ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);

				}
			}
			// Else If the item is already purchased increment purchase amount.  Only if ordering less then the max amount!
			else
			{
				if( BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased <= BOBBY_RAY_MAX_AMOUNT_OF_ITEMS_TO_PURCHASE)
					BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased++;
			}
		}
		else
		{
			DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, BobbyRText[ BOBBYR_MORE_NO_MORE_IN_STOCK ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);

			#ifdef JA2BETAVERSION
				ReportBobbyROrderError( usItemNumber, ubPurchaseNumber, LaptopSaveInfo.BobbyRayUsedInventory[ usItemNumber ].ubQtyOnHand, BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased );
			#endif
		}
	}
	//else the player is on a any other page except the used page
	else
	{
		//if there is enough inventory in stock to cover the purchase
		if( ubPurchaseNumber == BOBBY_RAY_NOT_PURCHASED || LaptopSaveInfo.BobbyRayInventory[ usItemNumber ].ubQtyOnHand >= ( BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased + 1) )
		{
			// If the item has not yet been purchased
			if( ubPurchaseNumber == BOBBY_RAY_NOT_PURCHASED )
			{
				ubPurchaseNumber = GetNextPurchaseNumber();

				if( ubPurchaseNumber != BOBBY_RAY_NOT_PURCHASED )
				{
					BobbyRayPurchases[ ubPurchaseNumber ].usItemIndex = LaptopSaveInfo.BobbyRayInventory[ usItemNumber ].usItemIndex;
					BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased = 1;
					BobbyRayPurchases[ ubPurchaseNumber ].bItemQuality = 100;
					BobbyRayPurchases[ ubPurchaseNumber ].usBobbyItemIndex = usItemNumber;
					BobbyRayPurchases[ ubPurchaseNumber ].fUsed = FALSE;
				}
				else
				{
					//display error popup because the player is trying to purchase more thenn 10 items
					DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, BobbyRText[ BOBBYR_MORE_THEN_10_PURCHASES ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
				}
			}
			// Else If the item is already purchased increment purchase amount.  Only if ordering less then the max amount!
			else
			{
				if( BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased <= BOBBY_RAY_MAX_AMOUNT_OF_ITEMS_TO_PURCHASE)
					BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased++;
			}
		}
		else
		{
			DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, BobbyRText[ BOBBYR_MORE_NO_MORE_IN_STOCK ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);

			#ifdef JA2BETAVERSION
				ReportBobbyROrderError( usItemNumber, ubPurchaseNumber, LaptopSaveInfo.BobbyRayInventory[ usItemNumber ].ubQtyOnHand, BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased );
			#endif
		}
	}
}


// Checks to see if the clicked item is already bought or not.
static UINT8 CheckIfItemIsPurchased(UINT16 usItemNumber)
{
	UINT8	i;

	for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
	{
		if( ( usItemNumber == BobbyRayPurchases[i].usBobbyItemIndex ) && ( BobbyRayPurchases[i].ubNumberPurchased != 0 ) && ( BobbyRayPurchases[i].fUsed == gfOnUsedPage ) )
			return(i);
	}
	return(BOBBY_RAY_NOT_PURCHASED);
}


static UINT8 GetNextPurchaseNumber(void)
{
	UINT8	i;

	for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
	{
		if( ( BobbyRayPurchases[i].usBobbyItemIndex == 0) && ( BobbyRayPurchases[i].ubNumberPurchased == 0 ) )
			return(i);
	}
	return(BOBBY_RAY_NOT_PURCHASED);
}


static void UnPurchaseBobbyRayItem(UINT16 usItemNumber)
{
	UINT8	ubPurchaseNumber;

	ubPurchaseNumber = CheckIfItemIsPurchased(usItemNumber);

	if( ubPurchaseNumber != BOBBY_RAY_NOT_PURCHASED )
	{
		if( BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased > 1)
			BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased--;
		else
		{
			BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased = 0;
			BobbyRayPurchases[ ubPurchaseNumber ].usBobbyItemIndex = 0;
		}
	}
}


static void BtnBobbyROrderFormCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R_MAILORDER;
	}
}


static void BtnBobbyRHomeButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
	}
}


void UpdateButtonText(UINT32	uiCurPage)
{
	switch( uiCurPage )
	{
		case LAPTOP_MODE_BOBBY_R_GUNS:
			DisableButton( guiBobbyRPageMenu[0] );
			break;

		case LAPTOP_MODE_BOBBY_R_AMMO:
			DisableButton( guiBobbyRPageMenu[1] );
			break;

		case LAPTOP_MODE_BOBBY_R_ARMOR:
			DisableButton( guiBobbyRPageMenu[2] );
			break;

		case LAPTOP_MODE_BOBBY_R_MISC:
			DisableButton( guiBobbyRPageMenu[3] );
			break;

		case LAPTOP_MODE_BOBBY_R_USED:
			DisableButton( guiBobbyRPageMenu[4] );
			break;
	}
}


UINT16 CalcBobbyRayCost( UINT16 usIndex, UINT16 usBobbyIndex, BOOLEAN fUsed)
{
	DOUBLE value;
	if( fUsed )
		value = Item[ LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyIndex ].usItemIndex ].usPrice *
								( .5 + .5 * ( LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyIndex ].ubItemQuality ) / 100 ) + .5;
	else
		value = Item[ LaptopSaveInfo.BobbyRayInventory[ usBobbyIndex ].usItemIndex ].usPrice;

	return( (UINT16) value);
}


static UINT32 CalculateTotalPurchasePrice(void)
{
	UINT16	i;
	UINT32	uiTotal = 0;

	for(i=0; i<MAX_PURCHASE_AMOUNT; i++)
	{
		//if the item was purchased
		if( BobbyRayPurchases[ i ].ubNumberPurchased )
		{
			uiTotal += CalcBobbyRayCost( BobbyRayPurchases[ i ].usItemIndex, BobbyRayPurchases[ i ].usBobbyItemIndex, BobbyRayPurchases[ i ].fUsed ) * BobbyRayPurchases[ i ].ubNumberPurchased;
		}
	}

	return(uiTotal);
}


static void DisableBobbyRButtons(void)
{
	//if it is the last page, disable the next page button
	if( gubNumPages == 0 )
		DisableButton( guiBobbyRNextPage );
	else
	{
		if( gubCurPage >= gubNumPages-1 )
			DisableButton( guiBobbyRNextPage );
		else
			EnableButton( guiBobbyRNextPage );
	}


	// if it is the first page, disable the prev page buitton
	if( gubCurPage == 0 )
		DisableButton( guiBobbyRPreviousPage );
	else
		EnableButton( guiBobbyRPreviousPage );

/*
	//if it is the last page, disable the next page button
	if( !(gusCurWeaponIndex < (gusLastItemIndex - BOBBYR_NUM_WEAPONS_ON_PAGE) ) )
		DisableButton( guiBobbyRNextPage );
	else
		EnableButton( guiBobbyRNextPage );


	// if it is the first page, disable the prev page buitton
	if( (gusCurWeaponIndex == gusFirstItemIndex ) )
		DisableButton( guiBobbyRPreviousPage );
	else
		EnableButton( guiBobbyRPreviousPage );
*/
}


static void CalcFirstIndexForPage(STORE_INVENTORY* pInv, UINT32 uiItemClass)
{
	UINT16	i;
	UINT16	usNumItems=0;

	//Reset the Current weapon Index
	gusCurWeaponIndex = 0;

	if( uiItemClass == BOBBYR_USED_ITEMS )
	{
		//Get to the first index on the page
		for(i=gusFirstItemIndex; i<=gusLastItemIndex; i++)
		{
			//If we have some of the inventory on hand
			if( pInv[ i ].ubQtyOnHand != 0 )
			{
				if( gubCurPage == 0 )
				{
					gusCurWeaponIndex = i;
					break;
				}

				if( usNumItems <= ( gubCurPage * 4 ) )
					gusCurWeaponIndex = i;

				usNumItems++;
			}
		}
	}
	else
	{
		//Get to the first index on the page
		for(i=gusFirstItemIndex; i<=gusLastItemIndex; i++)
		{
			if( Item[ pInv[ i ].usItemIndex ].usItemClass & uiItemClass )
			{
				//If we have some of the inventory on hand
				if( pInv[ i ].ubQtyOnHand != 0 )
				{
					if( gubCurPage == 0 )
					{
						gusCurWeaponIndex = i;
						break;
					}

					if( usNumItems <= ( gubCurPage * 4 ) )
						gusCurWeaponIndex = i;

					usNumItems++;
				}
			}
		}
	}
}


static void OutOfStockMessageBoxCallBack(UINT8 bExitValue)
{
	// yes, load the game
  if( bExitValue == MSG_BOX_RETURN_OK )
	{
//		guiCurrentLaptopMode  = LAPTOP_MODE_BOBBY_R;
	}
}


static UINT8 CheckPlayersInventoryForGunMatchingGivenAmmoID(INT16 sItemID)
{
	UINT8	ubItemCount=0;
	UINT8	ubMercCount;
	UINT8	ubPocketCount;

	UINT8	ubFirstID = gTacticalStatus.Team[ OUR_TEAM ].bFirstID;
	UINT8	ubLastID = gTacticalStatus.Team[ OUR_TEAM ].bLastID;


	//loop through all the mercs on the team
	for( ubMercCount = ubFirstID; ubMercCount <= ubLastID; ubMercCount++ )
	{
		if( Menptr[ ubMercCount ].bActive )
		{
			//loop through all the pockets on the merc
			for( ubPocketCount=0; ubPocketCount<NUM_INV_SLOTS; ubPocketCount++)
			{
				//if there is a weapon here
				if( Item[ Menptr[ ubMercCount ].inv[ ubPocketCount ].usItem ].usItemClass == IC_GUN )
				{
					//if the weapon uses the same kind of ammo as the one passed in, return true
					if( Weapon[ Menptr[ ubMercCount ].inv[ ubPocketCount ].usItem ].ubCalibre == Magazine[ Item[ sItemID ].ubClassIndex ].ubCalibre )
					{
						ubItemCount++;
					}
				}
			}
		}
	}

	return( ubItemCount );
}


#ifdef JA2BETAVERSION
static void ReportBobbyROrderError(UINT16 usItemNumber, UINT8 ubPurchaseNum, UINT8 ubQtyOnHand, UINT8 ubNumPurchasing)
{
	DebugMsg(TOPIC_JA2, DBG_LEVEL_3, "**** Bobby Rays Ordering Error ****");
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("usItemNumber = %d", usItemNumber ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("ubPurchaseNum = %d", ubPurchaseNum ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("ubQtyOnHand = %d", ubQtyOnHand ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("ubNumPurchasing = %d", ubNumPurchasing ) );
}
#endif
