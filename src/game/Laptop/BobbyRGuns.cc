#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Laptop.h"
#include "BobbyRGuns.h"
#include "BobbyR.h"
#include "MessageBoxScreen.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Interface_Items.h"
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

#include "CalibreModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "MagazineModel.h"
#include "WeaponModels.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"
#include "Logger.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>

#define BOBBYR_GRID_PIC_WIDTH		118
#define BOBBYR_GRID_PIC_HEIGHT		69

#define BOBBYR_GRID_PIC_X		BOBBYR_GRIDLOC_X + 3
#define BOBBYR_GRID_PIC_Y		BOBBYR_GRIDLOC_Y + 3

#define BOBBYR_GRID_OFFSET		72

#define BOBBYR_ORDER_TITLE_FONT		FONT14ARIAL
#define BOBBYR_ORDER_TEXT_FONT		FONT10ARIAL
#define BOBBYR_ORDER_TEXT_COLOR		75

#define BOBBYR_STATIC_TEXT_COLOR	75
#define   BOBBYR_ITEM_DESC_TEXT_FONT	FONT10ARIAL
#define   BOBBYR_ITEM_DESC_TEXT_COLOR	FONT_MCOLOR_WHITE
#define   BOBBYR_ITEM_NAME_TEXT_FONT	FONT10ARIAL
#define   BOBBYR_ITEM_NAME_TEXT_COLOR	FONT_MCOLOR_WHITE

#define NUM_BOBBYRPAGE_MENU		6
#define NUM_CATALOGUE_BUTTONS		5
#define BOBBYR_NUM_WEAPONS_ON_PAGE	4

#define BOBBYR_BRTITLE_X		LAPTOP_SCREEN_UL_X + 4
#define BOBBYR_BRTITLE_Y		LAPTOP_SCREEN_WEB_UL_Y + 3
#define BOBBYR_BRTITLE_WIDTH		46
#define BOBBYR_BRTITLE_HEIGHT		42

#define BOBBYR_TO_ORDER_TITLE_X		(STD_SCREEN_X + 195)
#define BOBBYR_TO_ORDER_TITLE_Y		(STD_SCREEN_Y + 42 + LAPTOP_SCREEN_WEB_DELTA_Y)

#define BOBBYR_TO_ORDER_TEXT_X		BOBBYR_TO_ORDER_TITLE_X + 75
#define BOBBYR_TO_ORDER_TEXT_Y		(STD_SCREEN_Y + 33 + LAPTOP_SCREEN_WEB_DELTA_Y)
#define BOBBYR_TO_ORDER_TEXT_WIDTH	330

#define BOBBYR_PREVIOUS_BUTTON_X	LAPTOP_SCREEN_UL_X + 5	//BOBBYR_HOME_BUTTON_X + BOBBYR_CATALOGUE_BUTTON_WIDTH + 5
#define BOBBYR_PREVIOUS_BUTTON_Y	LAPTOP_SCREEN_WEB_UL_Y + 340	//BOBBYR_HOME_BUTTON_Y

#define BOBBYR_NEXT_BUTTON_X		LAPTOP_SCREEN_UL_X + 412	//BOBBYR_ORDER_FORM_X + BOBBYR_ORDER_FORM_WIDTH + 5
#define BOBBYR_NEXT_BUTTON_Y		BOBBYR_PREVIOUS_BUTTON_Y	//BOBBYR_PREVIOUS_BUTTON_Y

#define BOBBYR_CATALOGUE_BUTTON_START_X	BOBBYR_PREVIOUS_BUTTON_X + 92 	//LAPTOP_SCREEN_UL_X + 93 - BOBBYR_CATALOGUE_BUTTON_WIDTH/2
#define BOBBYR_CATALOGUE_BUTTON_GAP	( 318 - NUM_CATALOGUE_BUTTONS * BOBBYR_CATALOGUE_BUTTON_WIDTH) / (NUM_CATALOGUE_BUTTONS + 1) + BOBBYR_CATALOGUE_BUTTON_WIDTH + 1//80
#define BOBBYR_CATALOGUE_BUTTON_Y	LAPTOP_SCREEN_WEB_UL_Y + 340
#define BOBBYR_CATALOGUE_BUTTON_WIDTH	56//75

#define   BOBBYR_HOME_BUTTON_X		(STD_SCREEN_X + 120)
#define   BOBBYR_HOME_BUTTON_Y		(STD_SCREEN_Y + 400 + LAPTOP_SCREEN_WEB_DELTA_Y)

#define BOBBYR_CATALOGUE_BUTTON_TEXT_Y	BOBBYR_CATALOGUE_BUTTON_Y + 5

#define BOBBYR_ITEM_DESC_START_X	BOBBYR_GRIDLOC_X + 172 + 5
#define BOBBYR_ITEM_DESC_START_Y	BOBBYR_GRIDLOC_Y + 6
#define BOBBYR_ITEM_DESC_START_WIDTH	214 - 10 + 20

#define BOBBYR_ITEM_NAME_X		BOBBYR_GRIDLOC_X + 6
#define BOBBYR_ITEM_NAME_Y_OFFSET	54

#define BOBBYR_ORDER_NUM_WIDTH		15
#define BOBBYR_ORDER_NUM_X		BOBBYR_GRIDLOC_X + 120 - BOBBYR_ORDER_NUM_WIDTH	//BOBBYR_ITEM_STOCK_TEXT_X
#define BOBBYR_ORDER_NUM_Y_OFFSET	1

#define BOBBYR_ITEM_WEIGHT_TEXT_X	BOBBYR_GRIDLOC_X + 409 + 3
#define BOBBYR_ITEM_WEIGHT_TEXT_Y	3

#define BOBBYR_ITEM_WEIGHT_NUM_X	BOBBYR_GRIDLOC_X + 429 - 2
#define BOBBYR_ITEM_WEIGHT_NUM_Y	3
#define BOBBYR_ITEM_WEIGHT_NUM_WIDTH	60

#define BOBBYR_ITEM_SPEC_GAP		2

#define BOBBYR_ITEM_COST_TEXT_X		BOBBYR_GRIDLOC_X + 125
#define BOBBYR_ITEM_COST_TEXT_Y		BOBBYR_GRIDLOC_Y + 6
#define BOBBYR_ITEM_COST_TEXT_WIDTH	42

#define BOBBYR_ITEM_COST_NUM_X		BOBBYR_ITEM_COST_TEXT_X
#define BOBBYR_ITEM_COST_NUM_Y		BOBBYR_ITEM_COST_TEXT_Y + 10

#define BOBBYR_ITEM_STOCK_TEXT_X	BOBBYR_ITEM_COST_TEXT_X

#define BOBBYR_ITEM_QTY_TEXT_X		BOBBYR_GRIDLOC_X + 5//BOBBYR_ITEM_COST_TEXT_X
#define BOBBYR_ITEM_QTY_TEXT_Y		BOBBYR_ITEM_COST_TEXT_Y + 28
#define BOBBYR_ITEM_QTY_WIDTH		95

#define BOBBYR_ITEM_QTY_NUM_X		BOBBYR_GRIDLOC_X + 105//BOBBYR_ITEM_COST_TEXT_X + 1
#define BOBBYR_ITEM_QTY_NUM_Y		BOBBYR_ITEM_QTY_TEXT_Y//BOBBYR_ITEM_COST_TEXT_Y + 40

#define BOBBYR_ITEMS_BOUGHT_X		BOBBYR_GRIDLOC_X + 105 - BOBBYR_ORDER_NUM_WIDTH//BOBBYR_ITEM_QTY_NUM_X

#define BOBBY_RAY_NOT_PURCHASED		255
#define BOBBY_RAY_MAX_AMOUNT_OF_ITEMS_TO_PURCHASE	200

#define BOBBYR_ORDER_FORM_X		LAPTOP_SCREEN_UL_X + 200//204
#define BOBBYR_ORDER_FORM_Y		LAPTOP_SCREEN_WEB_UL_Y + 367
#define BOBBYR_ORDER_FORM_WIDTH		95

#define BOBBYR_ORDER_SUBTOTAL_X		STD_SCREEN_X + 490
#define BOBBYR_ORDER_SUBTOTAL_Y		BOBBYR_ORDER_FORM_Y+2//BOBBYR_HOME_BUTTON_Y

#define BOBBYR_PERCENT_FUNTCIONAL_X	BOBBYR_ORDER_SUBTOTAL_X
#define BOBBYR_PERCENT_FUNTCIONAL_Y	BOBBYR_ORDER_SUBTOTAL_Y + 15


BobbyRayPurchaseStruct BobbyRayPurchases[ MAX_PURCHASE_AMOUNT ];


extern	BOOLEAN fExitingLaptopFlag;

static SGPVObject* guiGunBackground;
static SGPVObject* guiGunsGrid;
static SGPVObject* guiBrTitle;

UINT16 gusCurWeaponIndex;
static UINT8 gubCurPage;
static LaptopMode const ubCatalogueButtonValues[] =
{
	LAPTOP_MODE_BOBBY_R_GUNS,
	LAPTOP_MODE_BOBBY_R_AMMO,
	LAPTOP_MODE_BOBBY_R_ARMOR,
	LAPTOP_MODE_BOBBY_R_MISC,
	LAPTOP_MODE_BOBBY_R_USED
};

static UINT16 gusLastItemIndex  = 0;
static UINT16 gusFirstItemIndex = 0;
static UINT8  gubNumItemsOnScreen;
static UINT8  gubNumPages;

static BOOLEAN gfBigImageMouseRegionCreated;
static UINT16  gusItemNumberForItemsOnScreen[ BOBBYR_NUM_WEAPONS_ON_PAGE ];


static BOOLEAN gfOnUsedPage;

static UINT16 gusOldItemNumOnTopOfPage = 65535;

//The menu bar at the bottom that changes to different pages
static void BtnBobbyRPageMenuCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyRPageMenuImage;
static GUIButtonRef guiBobbyRPageMenu[NUM_CATALOGUE_BUTTONS];

//The next and previous buttons
static BUTTON_PICS* guiBobbyRPreviousPageImage;
static GUIButtonRef guiBobbyRPreviousPage;

static BUTTON_PICS* guiBobbyRNextPageImage;
static GUIButtonRef guiBobbyRNextPage;


static MOUSE_REGION g_scroll_region;

// Big Image Mouse region
static MOUSE_REGION gSelectedBigImageRegion[BOBBYR_NUM_WEAPONS_ON_PAGE];

// The order form button
static void BtnBobbyROrderFormCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyROrderFormImage;
static GUIButtonRef guiBobbyROrderForm;

// The Home button
static void BtnBobbyRHomeButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static BUTTON_PICS* guiBobbyRHomeImage;
static GUIButtonRef guiBobbyRHome;


// Link from the title
static MOUSE_REGION gSelectedTitleImageLinkRegion;


void GameInitBobbyRGuns()
{
	std::fill_n(BobbyRayPurchases, MAX_PURCHASE_AMOUNT, BobbyRayPurchaseStruct{});
}


void EnterBobbyRGuns()
{
	gfBigImageMouseRegionCreated = FALSE;

	// load the background graphic and add it
	guiGunBackground = AddVideoObjectFromFile(LAPTOPDIR "/gunbackground.sti");

	// load the gunsgrid graphic and add it
	guiGunsGrid = AddVideoObjectFromFile(LAPTOPDIR "/gunsgrid.sti");

	InitBobbyBrTitle();


	SetFirstLastPagesForNew( IC_BOBBY_GUN );
	//Draw menu bar
	InitBobbyMenuBar();

	// render once
	RenderBobbyRGuns( );

	//RenderBobbyRGuns();
}


void ExitBobbyRGuns()
{
	DeleteVideoObject(guiGunBackground);
	DeleteVideoObject(guiGunsGrid);
	DeleteBobbyBrTitle();
	DeleteBobbyMenuBar();

	DeleteMouseRegionForBigImage();

	giCurrentSubPage = gusCurWeaponIndex;
	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_GUNS;
}


void RenderBobbyRGuns()
{
	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES, BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiGunBackground);

	//Display title at top of page
	DisplayBobbyRBrTitle();

	BltVideoObject(FRAME_BUFFER, guiGunsGrid, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

	//DeleteMouseRegionForBigImage();
	DisplayItemInfo( IC_BOBBY_GUN );
	UpdateButtonText(guiCurrentLaptopMode);
	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateScreen();
}


void DisplayBobbyRBrTitle()
{
	BltVideoObject(FRAME_BUFFER, guiBrTitle, 0, BOBBYR_BRTITLE_X, BOBBYR_BRTITLE_Y);

	// To Order Text
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_TO_ORDER], BOBBYR_TO_ORDER_TITLE_X, BOBBYR_TO_ORDER_TITLE_Y, 0, BOBBYR_ORDER_TITLE_FONT, BOBBYR_ORDER_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//First put a shadow behind the image
	FRAME_BUFFER->ShadowRect(BOBBYR_TO_ORDER_TEXT_X - 2, BOBBYR_TO_ORDER_TEXT_Y - 2, BOBBYR_TO_ORDER_TEXT_X + BOBBYR_TO_ORDER_TEXT_WIDTH, BOBBYR_TO_ORDER_TEXT_Y + 31);

	//To Order text
	DisplayWrappedString(BOBBYR_TO_ORDER_TEXT_X, BOBBYR_TO_ORDER_TEXT_Y, BOBBYR_TO_ORDER_TEXT_WIDTH, 2, BOBBYR_ORDER_TEXT_FONT, BOBBYR_ORDER_TEXT_COLOR, BobbyRText[BOBBYR_GUNS_CLICK_ON_ITEMS], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
}


static void SelectTitleImageLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void InitBobbyBrTitle()
{
	// load the br title graphic and add it
	guiBrTitle = AddVideoObjectFromFile(LAPTOPDIR "/br.sti");

	//initialize the link to the homepage by clicking on the title
	MSYS_DefineRegion(&gSelectedTitleImageLinkRegion, BOBBYR_BRTITLE_X, BOBBYR_BRTITLE_Y,
				(BOBBYR_BRTITLE_X + BOBBYR_BRTITLE_WIDTH),
				(UINT16)(BOBBYR_BRTITLE_Y + BOBBYR_BRTITLE_HEIGHT),
				MSYS_PRIORITY_HIGH,
				CURSOR_WWW, MSYS_NO_CALLBACK, SelectTitleImageLinkRegionCallBack);

	gusOldItemNumOnTopOfPage=65535;
}


void DeleteBobbyBrTitle()
{
	DeleteVideoObject(guiBrTitle);
	MSYS_RemoveRegion(&gSelectedTitleImageLinkRegion);
	DeleteMouseRegionForBigImage();
}


static void SelectTitleImageLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
	}
}


static GUIButtonRef MakeButton(BUTTON_PICS* img, const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	const INT16 shadow_col = BOBBYR_GUNS_SHADOW_COLOR;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, BOBBYR_GUNS_BUTTON_FONT, BOBBYR_GUNS_TEXT_COLOR_ON, shadow_col, BOBBYR_GUNS_TEXT_COLOR_OFF, shadow_col, x, y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


static void BtnBobbyRNextPageCallback(GUI_BUTTON*, UINT32 reason);
static void BtnBobbyRPreviousPageCallback(GUI_BUTTON*, UINT32 reason);


void InitBobbyMenuBar()
{
	// Previous button
	guiBobbyRPreviousPageImage = LoadButtonImage(LAPTOPDIR "/previousbutton.sti", 0, 1);
	guiBobbyRPreviousPage      = MakeButton(guiBobbyRPreviousPageImage, BobbyRText[BOBBYR_GUNS_PREVIOUS_ITEMS], BOBBYR_PREVIOUS_BUTTON_X, BOBBYR_PREVIOUS_BUTTON_Y, BtnBobbyRPreviousPageCallback);
	guiBobbyRPreviousPage->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);

	// Next button
	guiBobbyRNextPageImage = LoadButtonImage(LAPTOPDIR "/nextbutton.sti", 0, 1);
	guiBobbyRNextPage      = MakeButton(guiBobbyRNextPageImage, BobbyRText[BOBBYR_GUNS_MORE_ITEMS], BOBBYR_NEXT_BUTTON_X, BOBBYR_NEXT_BUTTON_Y, BtnBobbyRNextPageCallback);
	guiBobbyRNextPage->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);

	BUTTON_PICS* const gfx = LoadButtonImage(LAPTOPDIR "/cataloguebutton1.sti", 0, 1);
	guiBobbyRPageMenuImage = gfx;

	UINT16             x    = BOBBYR_CATALOGUE_BUTTON_START_X;
	UINT16     const   y    = BOBBYR_CATALOGUE_BUTTON_Y;
	const ST::string* text = BobbyRText + BOBBYR_GUNS_GUNS;
	LaptopMode const*  mode = ubCatalogueButtonValues;
	FOR_EACHX(GUIButtonRef, i, guiBobbyRPageMenu, x += BOBBYR_CATALOGUE_BUTTON_GAP)
	{
		// Catalogue buttons
		GUIButtonRef const b = MakeButton(gfx, *text++, x, y, BtnBobbyRPageMenuCallback);
		b->SetUserData(*mode++);
		*i = b;
	}

	// Order Form button
	guiBobbyROrderFormImage = LoadButtonImage(LAPTOPDIR "/orderformbutton.sti", 0, 1);
	guiBobbyROrderForm      = MakeButton(guiBobbyROrderFormImage, BobbyRText[BOBBYR_GUNS_ORDER_FORM], BOBBYR_ORDER_FORM_X, BOBBYR_ORDER_FORM_Y, BtnBobbyROrderFormCallback);

	// Home button
	guiBobbyRHomeImage = LoadButtonImage(LAPTOPDIR "/cataloguebutton.sti", 0, 1);
	guiBobbyRHome      = MakeButton(guiBobbyRHomeImage, BobbyRText[BOBBYR_GUNS_HOME], BOBBYR_HOME_BUTTON_X, BOBBYR_HOME_BUTTON_Y, BtnBobbyRHomeButtonCallback);
}


void DeleteBobbyMenuBar()
{
	RemoveButton(guiBobbyRPreviousPage);
	UnloadButtonImage(guiBobbyRPreviousPageImage);

	RemoveButton(guiBobbyRNextPage);
	UnloadButtonImage(guiBobbyRNextPageImage);

	FOR_EACH(GUIButtonRef, i, guiBobbyRPageMenu) RemoveButton(*i);
	UnloadButtonImage(guiBobbyRPageMenuImage);

	RemoveButton(guiBobbyROrderForm);
	UnloadButtonImage(guiBobbyROrderFormImage);

	RemoveButton(guiBobbyRHome);
	UnloadButtonImage(guiBobbyRHomeImage);
}


static void BtnBobbyRPageMenuCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		UpdateButtonText(guiCurrentLaptopMode);
		guiCurrentLaptopMode = static_cast<LaptopMode>(btn->GetUserData());
	}
}


static void NextPage()
{
	if (gubCurPage == gubNumPages - 1) return;
	++gubCurPage;
	DeleteMouseRegionForBigImage();
	fReDrawScreenFlag       = TRUE;
	fPausedReDrawScreenFlag = TRUE;
}


static void BtnBobbyRNextPageCallback(GUI_BUTTON* const btn, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		NextPage();
	}
}


static void PrevPage()
{
	if (gubCurPage == 0) return;
	--gubCurPage;
	DeleteMouseRegionForBigImage();
	fReDrawScreenFlag       = TRUE;
	fPausedReDrawScreenFlag = TRUE;
}


static void BtnBobbyRPreviousPageCallback(GUI_BUTTON* const btn, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		PrevPage();
	}
}


static void CalcFirstIndexForPage(STORE_INVENTORY* pInv, UINT32 uiItemClass);
static UINT32 CalculateTotalPurchasePrice();
static void CreateMouseRegionForBigImage(UINT16 usPosY, UINT8 ubCount, const ItemModel* const items[]);
static void DisableBobbyRButtons(void);
static void DisplayAmmoInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static void DisplayArmourInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static void DisplayBigItemImage(const ItemModel* item, UINT16 PosY);
static void DisplayGunInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static void DisplayItemNameAndInfo(UINT16 usPosY, UINT16 usIndex, UINT16 usBobbyIndex, BOOLEAN fUsed);
static void DisplayMiscInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);
static void DisplayNonGunWeaponInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex);


void DisplayItemInfo(UINT32 uiItemClass)
{
	UINT16  i;
	UINT8   ubCount=0;
	UINT16  PosY, usTextPosY;
	UINT16  usItemIndex;
	ST::string sDollarTemp;
	ST::string sTemp;

	PosY = BOBBYR_GRID_PIC_Y;
	usTextPosY = BOBBYR_ITEM_DESC_START_Y;

	//if there are no items then return
	if( gusFirstItemIndex == BOBBYR_NO_ITEMS )
	{
		if (fExitingLaptopFlag) return;
		if (gfShowBookmarks)	return;
		if (fLoadPendingFlag)	return;

		DisableBobbyRButtons();

		//Display a popup saying we are out of stock
		DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, BobbyRText[BOBBYR_NO_MORE_STOCK], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, 0);
		return;
	}


	if( uiItemClass == BOBBYR_USED_ITEMS )
		CalcFirstIndexForPage(LaptopSaveInfo.BobbyRayUsedInventory, IC_ALL);
	else
		CalcFirstIndexForPage( LaptopSaveInfo.BobbyRayInventory, uiItemClass );

	DisableBobbyRButtons();

	if( gusOldItemNumOnTopOfPage != gusCurWeaponIndex )
	{
		DeleteMouseRegionForBigImage();

	}

	const ItemModel* items[BOBBYR_NUM_WEAPONS_ON_PAGE];
	std::fill(std::begin(items), std::end(items), nullptr);
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
		const ItemModel * item = GCM->getItem(usItemIndex);
		if (!(item->getItemClass() & uiItemClass)) continue;

		items[ubCount] = item;

		switch (item->getItemClass())
		{
			case IC_GUN:
			case IC_LAUNCHER:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage(item, PosY);

				//Display Items Name
				DisplayItemNameAndInfo(usTextPosY, usItemIndex, i, gfOnUsedPage);

				DisplayGunInfo(usItemIndex, usTextPosY, gfOnUsedPage, i);

				PosY += BOBBYR_GRID_OFFSET;
				usTextPosY += BOBBYR_GRID_OFFSET;
				ubCount++;
				break;

			case IC_AMMO:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage(item, PosY);

				//Display Items Name
				DisplayItemNameAndInfo(usTextPosY, usItemIndex, i, gfOnUsedPage);

				DisplayAmmoInfo( usItemIndex, usTextPosY, gfOnUsedPage, i);

				PosY += BOBBYR_GRID_OFFSET;
				usTextPosY += BOBBYR_GRID_OFFSET;
				ubCount++;
				break;

			case IC_ARMOUR:
				gusItemNumberForItemsOnScreen[ ubCount ] = i;

				DisplayBigItemImage(item, PosY);

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

				DisplayBigItemImage(item, PosY);

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

				DisplayBigItemImage(item, PosY);

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
		CreateMouseRegionForBigImage(BOBBYR_GRID_PIC_Y, ubCount, items);
		gusOldItemNumOnTopOfPage = gusCurWeaponIndex;
	}

	//Display the subtotal at the bottom of the screen
	sDollarTemp = SPrintMoney(CalculateTotalPurchasePrice());
	sTemp = ST::format("{} {}", BobbyRText[BOBBYR_GUNS_SUB_TOTAL], sDollarTemp);
	DrawTextToScreen(sTemp, BOBBYR_ORDER_SUBTOTAL_X, BOBBYR_ORDER_SUBTOTAL_Y, 0, BOBBYR_ORDER_TITLE_FONT, BOBBYR_ORDER_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | TEXT_SHADOWED);

	//Display the Used item disclaimer
	if( gfOnUsedPage )
	{
		DrawTextToScreen(BobbyRText[BOBBYR_GUNS_PERCENT_FUNCTIONAL], BOBBYR_PERCENT_FUNTCIONAL_X, BOBBYR_PERCENT_FUNTCIONAL_Y, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ORDER_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | TEXT_SHADOWED);
	}
}


static UINT16 DisplayCaliber(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayCostAndQty(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight, UINT16 usBobbyIndex, BOOLEAN fUsed);
static UINT16 DisplayDamage(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayMagazine(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayRange(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);
static UINT16 DisplayRof(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight);


static void DisplayGunInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16	usHeight;
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
	//DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	usHeight = usTextPosY;
	//Display the weight, caliber, mag, rng, dam, rof text

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
}


static void DisplayNonGunWeaponInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16	usHeight;
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
	//DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	usHeight = usTextPosY;
	//Display the weight, caliber, mag, rng, dam, rof text

	//Damage
	usHeight = DisplayDamage(usHeight, usIndex, usFontHeight);

	//Display the Cost and the qty bought and on hand
	usHeight = DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);
}


static void DisplayAmmoInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16	usHeight;
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
	//DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	usHeight = usTextPosY;
	//Display the weight, caliber, mag, rng, dam, rof text

	//Caliber
	usHeight = DisplayCaliber(usHeight, usIndex, usFontHeight);

	//Magazine
	//usHeight = DisplayMagazine(usHeight, usIndex, usFontHeight);

	//Display the Cost and the qty bought and on hand
	usHeight = DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);
}


static void DisplayBigItemImage(const ItemModel* item, const UINT16 PosY)
{
	INT16 PosX = BOBBYR_GRID_PIC_X;

	auto graphic = GetBigInventoryGraphicForItem(item);
	AutoSGPVObject uiImage(graphic.first);
	auto subImageIndex = graphic.second;

	//center picture in frame
	ETRLEObject const& pTrav   = uiImage->SubregionProperties(subImageIndex);
	UINT32      const  usWidth = pTrav.usWidth;
	INT16       const  sCenX   = PosX + std::abs(int(BOBBYR_GRID_PIC_WIDTH - usWidth)) / 2 - pTrav.sOffsetX;
	INT16       const  sCenY   = PosY + 8;

	if (gamepolicy(f_draw_item_shadow))
	{
		//blt the shadow of the item
		BltVideoObjectOutlineShadow(FRAME_BUFFER, uiImage.get(), subImageIndex, sCenX - 2, sCenY + 2);
	}

	BltVideoObject(FRAME_BUFFER, uiImage.get(), subImageIndex, sCenX, sCenY);
}


static void DisplayArmourInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16 usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display the Cost and the qty bought and on hand
	DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);
}


static void DisplayMiscInfo(UINT16 usIndex, UINT16 usTextPosY, BOOLEAN fUsed, UINT16 usBobbyIndex)
{
	UINT16 usFontHeight;
	usFontHeight = GetFontHeight(BOBBYR_ITEM_DESC_TEXT_FONT);

	//Display Items Name
	//DisplayItemNameAndInfo(usTextPosY, usIndex, fUsed);

	//Display the Cost and the qty bought and on hand
	DisplayCostAndQty(usTextPosY, usIndex, usFontHeight, usBobbyIndex, fUsed);
}


static UINT8 CheckIfItemIsPurchased(UINT16 usItemNumber);


static UINT16 DisplayCostAndQty(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight, UINT16 usBobbyIndex, BOOLEAN fUsed)
{
	ST::string sTemp;
	//UINT8	ubPurchaseNumber;

	//
	//Display the cost and the qty
	//

	//Display the cost
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_COST], BOBBYR_ITEM_COST_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += usFontHeight + 2;

	DrawTextToScreen(SPrintMoney(CalcBobbyRayCost(usIndex, usBobbyIndex, fUsed)), BOBBYR_ITEM_COST_NUM_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	//Display Weight Number
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_WGHT], BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	sTemp = ST::format("{3.2f} {}", GetWeightBasedOnMetricOption(GCM->getItem(usIndex)->getWeight()) / 10.0f, GetWeightUnitString());
	DrawTextToScreen(sTemp, BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	//Display the # In Stock
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_IN_STOCK], BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	usPosY += usFontHeight + 2;

	if( fUsed )
		sTemp = ST::format("{_ 4d}", LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyIndex ].ubQtyOnHand);
	else
		sTemp = ST::format("{_ 4d}", LaptopSaveInfo.BobbyRayInventory[ usBobbyIndex ].ubQtyOnHand);

	DrawTextToScreen(sTemp, BOBBYR_ITEM_STOCK_TEXT_X, usPosY, BOBBYR_ITEM_COST_TEXT_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;


	return(usPosY);
}


static UINT16 DisplayRof(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	ST::string sTemp;

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_ROF], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	sTemp = ST::format("{3d}/{}", GCM->getWeapon(usIndex)->getRateOfFire(), pMessageStrings[ MSG_MINUTE_ABBREVIATION ]);


	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayDamage(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	ST::string sTemp;

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_DAMAGE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	sTemp = ST::format("{4d}", GCM->getWeapon( usIndex )->ubImpact);
	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayRange(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	ST::string sTemp;

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_RANGE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	sTemp = ST::format("{3d} {}", GCM->getWeapon( usIndex )->usRange, pMessageStrings[ MSG_METER_ABBREVIATION ]);
	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayMagazine(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	ST::string sTemp;

	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_MAGAZINE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	sTemp = ST::format("{3d} {}", GCM->getWeapon(usIndex)->ubMagSize, pMessageStrings[ MSG_ROUNDS_ABBREVIATION ]);
	DrawTextToScreen(sTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	usPosY += usFontHeight + 2;
	return(usPosY);
}


static UINT16 DisplayCaliber(UINT16 usPosY, UINT16 usIndex, UINT16 usFontHeight)
{
	const ItemModel * item = GCM->getItem(usIndex);
	ST::string zTemp;
	DrawTextToScreen(BobbyRText[BOBBYR_GUNS_CALIBRE], BOBBYR_ITEM_WEIGHT_TEXT_X, usPosY, 0, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	// ammo or gun?
	const CalibreModel *calibre = item->getItemClass() == IC_AMMO ? item->asAmmo()->calibre : item->asWeapon()->calibre;
	zTemp = *GCM->getCalibreNameForBobbyRay(calibre->index);

	zTemp = ReduceStringLength(zTemp, BOBBYR_GRID_PIC_WIDTH, BOBBYR_ITEM_NAME_TEXT_FONT);
	DrawTextToScreen(zTemp, BOBBYR_ITEM_WEIGHT_NUM_X, usPosY, BOBBYR_ITEM_WEIGHT_NUM_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	usPosY += usFontHeight + 2;
	return(usPosY);
}


static void DisplayItemNameAndInfo(UINT16 usPosY, UINT16 usIndex, UINT16 usBobbyIndex, BOOLEAN fUsed)
{
	ST::string sTemp;
	UINT32 uiStartLoc;
	UINT8	ubPurchaseNumber;

	{
		//Display Items Name
		uiStartLoc = BOBBYR_ITEM_DESC_FILE_SIZE * usIndex;
		ST::string sText = GCM->loadEncryptedString(BOBBYRDESCFILE, uiStartLoc, BOBBYR_ITEM_DESC_NAME_SIZE);
		sText = ReduceStringLength(sText, BOBBYR_GRID_PIC_WIDTH - 6, BOBBYR_ITEM_NAME_TEXT_FONT);
		DrawTextToScreen(sText, BOBBYR_ITEM_NAME_X, usPosY + BOBBYR_ITEM_NAME_Y_OFFSET, 0, BOBBYR_ITEM_NAME_TEXT_FONT, BOBBYR_ITEM_NAME_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	//number bought
	//Display the # bought
	ubPurchaseNumber = CheckIfItemIsPurchased(usBobbyIndex);
	if( ubPurchaseNumber != BOBBY_RAY_NOT_PURCHASED)
	{
		DrawTextToScreen(BobbyRText[BOBBYR_GUNS_QTY_ON_ORDER], BOBBYR_ITEM_QTY_TEXT_X, usPosY, BOBBYR_ITEM_QTY_WIDTH, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_STATIC_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

		if( ubPurchaseNumber != BOBBY_RAY_NOT_PURCHASED)
		{
			sTemp = ST::format("{_ 4d}", BobbyRayPurchases[ ubPurchaseNumber ].ubNumberPurchased);
			DrawTextToScreen(sTemp, BOBBYR_ITEMS_BOUGHT_X, usPosY, 0, FONT14ARIAL, BOBBYR_ITEM_DESC_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}
	}




	//if it's a used item, display how damaged the item is
	if( fUsed )
	{
		sTemp = ST::format("*{3d}%", LaptopSaveInfo.BobbyRayUsedInventory[usBobbyIndex].ubItemQuality);
		DrawTextToScreen(sTemp, BOBBYR_ITEM_NAME_X - 2, usPosY - BOBBYR_ORDER_NUM_Y_OFFSET, BOBBYR_ORDER_NUM_WIDTH, BOBBYR_ITEM_NAME_TEXT_FONT, BOBBYR_ITEM_NAME_TEXT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	{
		//Display Items description
		uiStartLoc += BOBBYR_ITEM_DESC_NAME_SIZE;
		ST::string sText = GCM->loadEncryptedString(BOBBYRDESCFILE, uiStartLoc, BOBBYR_ITEM_DESC_INFO_SIZE);
		DisplayWrappedString(BOBBYR_ITEM_DESC_START_X, usPosY, BOBBYR_ITEM_DESC_START_WIDTH, 2, BOBBYR_ITEM_DESC_TEXT_FONT, BOBBYR_ITEM_DESC_TEXT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
}


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
			if( GCM->getItem(LaptopSaveInfo.BobbyRayInventory[ i ].usItemIndex)->getItemClass() & uiClassMask )
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


static void ScrollRegionCallback(MOUSE_REGION* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		PrevPage();
	}
	else if (reason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		NextPage();
	}
}


static UINT8 CheckPlayersInventoryForGunMatchingGivenAmmoID(const ItemModel* ammo);
static void SelectBigImageRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateMouseRegionForBigImage(UINT16 y, const UINT8 n_regions, const ItemModel* const items[])
{
	if (gfBigImageMouseRegionCreated) return;

	{
		UINT16 const x = BOBBYR_GRIDLOC_X;
		UINT16 const y = BOBBYR_GRIDLOC_Y;
		UINT16 const w = 493;
		UINT16 const h = 290;
		MSYS_DefineRegion(&g_scroll_region, x, y, x + w, y + h, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, ScrollRegionCallback);
	}

	UINT16 const x = BOBBYR_GRID_PIC_X;
	UINT16 const w = BOBBYR_GRID_PIC_WIDTH;
	UINT16 const h = BOBBYR_GRID_PIC_HEIGHT;
	for (UINT8 i = 0; i != n_regions; y += BOBBYR_GRID_OFFSET, ++i)
	{
		// Mouse region for the Big Item Image
		MOUSE_REGION& r = gSelectedBigImageRegion[i];
		MSYS_DefineRegion(&r, x, y, x + w, y + h, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectBigImageRegionCallBack);
		MSYS_SetRegionUserData(&r, 0, i);

		// Specify the help text only if the items is ammo
		ItemModel const* const item = items[i];
		if (item->getItemClass() != IC_AMMO) continue;
		// And only if the user has an item that can use the particular type of ammo
		UINT8 const n_guns = CheckPlayersInventoryForGunMatchingGivenAmmoID(item);
		if (n_guns == 0) continue;

		ST::string buf = st_format_printf(str_bobbyr_guns_num_guns_that_use_ammo, n_guns);
		r.SetFastHelpText(buf);
	}

	gubNumItemsOnScreen          = n_regions;
	gfBigImageMouseRegionCreated = TRUE;
}


void DeleteMouseRegionForBigImage()
{
	if (!gfBigImageMouseRegionCreated) return;

	MSYS_RemoveRegion(&g_scroll_region);

	for (UINT8 i = 0; i != gubNumItemsOnScreen; ++i)
	{
		MSYS_RemoveRegion(&gSelectedBigImageRegion[i]);
	}

	gfBigImageMouseRegionCreated = FALSE;
	gusOldItemNumOnTopOfPage     = 65535;
	gubNumItemsOnScreen          = 0;
}


static void PurchaseBobbyRayItem(UINT16 usItemNumber);
static void UnPurchaseBobbyRayItem(UINT16 usItemNumber);


static void SelectBigImageRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_POINTER_UP)
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
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		PrevPage();
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		NextPage();
	}
}


static UINT8 GetNextPurchaseNumber(void);

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


static void BtnBobbyROrderFormCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R_MAILORDER;
	}
}


static void BtnBobbyRHomeButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
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
		value = GCM->getItem(LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyIndex ].usItemIndex)->getPrice() *
								( .5 + .5 * ( LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyIndex ].ubItemQuality ) / 100 ) + .5;
	else
		value = GCM->getItem(LaptopSaveInfo.BobbyRayInventory[ usBobbyIndex ].usItemIndex)->getPrice();

	return( (UINT16) value);
}


static UINT32 CalculateTotalPurchasePrice()
{
	UINT32 total = 0;
	FOR_EACH(BobbyRayPurchaseStruct const, i, BobbyRayPurchases)
	{
		BobbyRayPurchaseStruct const& p = *i;
		if (p.ubNumberPurchased == 0) continue;
		total += CalcBobbyRayCost(p.usItemIndex, p.usBobbyItemIndex, p.fUsed) * p.ubNumberPurchased;
	}
	return total;
}


static void DisableBobbyRButtons(void)
{
	//if it is the last page, disable the next page button
	EnableButton(guiBobbyRNextPage, gubNumPages != 0 && gubCurPage < gubNumPages - 1);

	// if it is the first page, disable the prev page buitton
	EnableButton(guiBobbyRPreviousPage, gubCurPage != 0);
}


static void CalcFirstIndexForPage(STORE_INVENTORY* const pInv, UINT32 const item_class)
{
	// Reset the Current weapon Index
	gusCurWeaponIndex = 0;

	// Get to the first index on the page
	UINT16 inv_idx = 0;
	for (UINT16 i = gusFirstItemIndex; i <= gusLastItemIndex; ++i)
	{
		if (!(GCM->getItem(pInv[i].usItemIndex)->getItemClass() & item_class)) continue;
		// If we have some of the inventory on hand
		if (pInv[i].ubQtyOnHand == 0) continue;

		gusCurWeaponIndex = i;
		if (inv_idx++ == gubCurPage * 4) break;
	}
}


static UINT8 CheckPlayersInventoryForGunMatchingGivenAmmoID(ItemModel const* const ammo)
{
	UINT8 n_items = 0;
	const CalibreModel *calibre = ammo->asAmmo()->calibre;
	CFOR_EACH_IN_TEAM(s, OUR_TEAM)
	{
		// Loop through all the pockets on the merc
		CFOR_EACH_SOLDIER_INV_SLOT(i, *s)
		{
			OBJECTTYPE const& o = *i;
			// If there is a weapon here
			if (GCM->getItem(o.usItem)->getItemClass() != IC_GUN) continue;
			// If the weapon uses the same kind of ammo as the one passed in
			if (!GCM->getWeapon(o.usItem)->matches(calibre)) continue;

			++n_items;
		}
	}
	return n_items;
}
