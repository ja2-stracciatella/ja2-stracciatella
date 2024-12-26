#include "Directories.h"
#include "Font.h"
#include "ItemModel.h"
#include "Laptop.h"
#include "BobbyR.h"
#include "BobbyRGuns.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Store_Inventory.h"
#include "Game_Event_Hook.h"
#include "Game_Clock.h"
#include "LaptopSave.h"
#include "Random.h"
#include "Text.h"
#include "GameRes.h"
#include "ArmsDealerInvInit.h"
#include "Video.h"
#include "VSurface.h"
#include "Debug.h"
#include "Font_Control.h"
#include "Items.h"

#include "ContentManager.h"
#include "DealerInventory.h"
#include "GameInstance.h"

#include <algorithm>

#define BOBBIES_SIGN_FONT			FONT14ARIAL
#define BOBBIES_SIGN_COLOR			2
#define BOBBIES_SIGN_BACKCOLOR			FONT_MCOLOR_BLACK
#define BOBBIES_SIGN_BACKGROUNDCOLOR		78//NO_SHADOW

#define BOBBIES_NUMBER_SIGNS			5

#define BOBBIES_SENTENCE_FONT			FONT12ARIAL
#define BOBBIES_SENTENCE_COLOR			FONT_MCOLOR_WHITE
#define BOBBIES_SENTENCE_BACKGROUNDCOLOR	2//NO_SHADOW//226

#define BOBBY_WOOD_BACKGROUND_X			LAPTOP_SCREEN_UL_X
#define BOBBY_WOOD_BACKGROUND_Y			LAPTOP_SCREEN_WEB_UL_Y
#define BOBBY_WOOD_BACKGROUND_WIDTH		125
#define BOBBY_WOOD_BACKGROUND_HEIGHT		100

#define BOBBY_RAYS_NAME_X			LAPTOP_SCREEN_UL_X + 77
#define BOBBY_RAYS_NAME_Y			LAPTOP_SCREEN_WEB_UL_Y + 0

#define BOBBYS_PLAQUES_X			LAPTOP_SCREEN_UL_X + 39
#define BOBBYS_PLAQUES_Y			LAPTOP_SCREEN_WEB_UL_Y + 174

#define BOBBIES_TOPHINGE_X			LAPTOP_SCREEN_UL_X
#define BOBBIES_TOPHINGE_Y			LAPTOP_SCREEN_WEB_UL_Y + 42

#define BOBBIES_BOTTOMHINGE_X			LAPTOP_SCREEN_UL_X
#define BOBBIES_BOTTOMHINGE_Y			LAPTOP_SCREEN_WEB_UL_Y + 338

#define BOBBIES_STORE_PLAQUE_X			LAPTOP_SCREEN_UL_X + 148
#define BOBBIES_STORE_PLAQUE_Y			LAPTOP_SCREEN_WEB_UL_Y + 66
#define BOBBIES_STORE_PLAQUE_HEIGHT		93

#define BOBBIES_HANDLE_X			LAPTOP_SCREEN_UL_X + 457
#define BOBBIES_HANDLE_Y			LAPTOP_SCREEN_WEB_UL_Y + 147

#define BOBBIES_FIRST_SENTENCE_X		LAPTOP_SCREEN_UL_X
#define BOBBIES_FIRST_SENTENCE_Y		BOBBIES_STORE_PLAQUE_Y + BOBBIES_STORE_PLAQUE_HEIGHT - 3
#define BOBBIES_FIRST_SENTENCE_WIDTH		500

#define BOBBIES_2ND_SENTENCE_X			LAPTOP_SCREEN_UL_X
#define BOBBIES_2ND_SENTENCE_Y			BOBBIES_FIRST_SENTENCE_Y + 13
#define BOBBIES_2ND_SENTENCE_WIDTH		500

#define BOBBIES_CENTER_SIGN_OFFSET_Y		23

#define BOBBIES_USED_SIGN_X			(UINT16)(BOBBYS_PLAQUES_X + 93)
#define BOBBIES_USED_SIGN_Y			(UINT16)(BOBBYS_PLAQUES_Y + 32)
#define BOBBIES_USED_SIGN_WIDTH			92
#define BOBBIES_USED_SIGN_HEIGHT		50
#define BOBBIES_USED_SIGN_TEXT_OFFSET		BOBBIES_USED_SIGN_Y + 10

#define BOBBIES_MISC_SIGN_X			(UINT16)(BOBBYS_PLAQUES_X + 238)
#define BOBBIES_MISC_SIGN_Y			(UINT16)(BOBBYS_PLAQUES_Y + 27)
#define BOBBIES_MISC_SIGN_WIDTH			103
#define BOBBIES_MISC_SIGN_HEIGHT		57
#define BOBBIES_MISC_SIGN_TEXT_OFFSET		BOBBIES_MISC_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_GUNS_SIGN_X			(UINT16)(BOBBYS_PLAQUES_X + 3)
#define BOBBIES_GUNS_SIGN_Y			(UINT16)(BOBBYS_PLAQUES_Y + 102)
#define BOBBIES_GUNS_SIGN_WIDTH			116
#define BOBBIES_GUNS_SIGN_HEIGHT		75
#define BOBBIES_GUNS_SIGN_TEXT_OFFSET		BOBBIES_GUNS_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_AMMO_SIGN_X			(UINT16)(BOBBYS_PLAQUES_X + 150)
#define BOBBIES_AMMO_SIGN_Y			(UINT16)(BOBBYS_PLAQUES_Y + 105)
#define BOBBIES_AMMO_SIGN_WIDTH			112
#define BOBBIES_AMMO_SIGN_HEIGHT		71
#define BOBBIES_AMMO_SIGN_TEXT_OFFSET		BOBBIES_AMMO_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_ARMOUR_SIGN_X			(UINT16)(BOBBYS_PLAQUES_X + 290)
#define BOBBIES_ARMOUR_SIGN_Y			(UINT16)(BOBBYS_PLAQUES_Y + 108)
#define BOBBIES_ARMOUR_SIGN_WIDTH		114
#define BOBBIES_ARMOUR_SIGN_HEIGHT		70
#define BOBBIES_ARMOUR_SIGN_TEXT_OFFSET		BOBBIES_ARMOUR_SIGN_Y + BOBBIES_CENTER_SIGN_OFFSET_Y

#define BOBBIES_3RD_SENTENCE_X			LAPTOP_SCREEN_UL_X
#define BOBBIES_3RD_SENTENCE_Y			BOBBIES_BOTTOMHINGE_Y + 40
#define BOBBIES_3RD_SENTENCE_WIDTH		500

#define BOBBY_R_NEW_PURCHASE_ARRIVAL_TIME	(1 * 60 * 24) // minutes in 1 day

#define BOBBY_R_USED_PURCHASE_OFFSET		1000

#define BOBBYR_UNDERCONSTRUCTION_ANI_DELAY	150
#define BOBBYR_UNDERCONSTRUCTION_NUM_FRAMES	5

#define BOBBYR_UNDERCONSTRUCTION_X		LAPTOP_SCREEN_UL_X + ( LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X - BOBBYR_UNDERCONSTRUCTION_WIDTH ) / 2
#define BOBBYR_UNDERCONSTRUCTION_Y		175
#define BOBBYR_UNDERCONSTRUCTION1_Y		378

#define BOBBYR_UNDERCONSTRUCTION_WIDTH		414
#define BOBBYR_UNDERCONSTRUCTION_HEIGHT		64

#define BOBBYR_UNDER_CONSTRUCTION_TEXT_X	LAPTOP_SCREEN_UL_X
#define BOBBYR_UNDER_CONSTRUCTION_TEXT_Y	BOBBYR_UNDERCONSTRUCTION_Y + 62 + 60
#define BOBBYR_UNDER_CONSTRUCTION_TEXT_WIDTH	LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X


static SGPVObject* guiBobbyName;
static SGPVObject* guiPlaque;
static SGPVObject* guiTopHinge;
static SGPVObject* guiBottomHinge;
static SGPVObject* guiStorePlaque;
static SGPVObject* guiHandle;
static SGPVObject* guiWoodBackground;
static SGPVObject* guiUnderConstructionImage;


LaptopMode guiLastBobbyRayPage;


static LaptopMode const gubBobbyRPages[] =
{
	LAPTOP_MODE_BOBBY_R_USED,
	LAPTOP_MODE_BOBBY_R_MISC,
	LAPTOP_MODE_BOBBY_R_GUNS,
	LAPTOP_MODE_BOBBY_R_AMMO,
	LAPTOP_MODE_BOBBY_R_ARMOR
};


//Bobby's Sign menu mouse regions
static MOUSE_REGION gSelectedBobbiesSignMenuRegion[BOBBIES_NUMBER_SIGNS];


static void HandleBobbyRUnderConstructionAni(BOOLEAN fReset);
static void InitBobbiesMouseRegion(UINT8 ubNumerRegions, UINT16* usMouseRegionPosArray, MOUSE_REGION* MouseRegion);


void EnterBobbyR()
{
	UINT8 i;

	// an array of mouse regions for the bobbies signs.  Top Left corner, bottom right corner
	UINT16  usMouseRegionPosArray[] = {
		BOBBIES_USED_SIGN_X,
		BOBBIES_USED_SIGN_Y,
		(UINT16)(BOBBIES_USED_SIGN_X+BOBBIES_USED_SIGN_WIDTH),
		(UINT16)(BOBBIES_USED_SIGN_Y+BOBBIES_USED_SIGN_HEIGHT),
		BOBBIES_MISC_SIGN_X,
		BOBBIES_MISC_SIGN_Y,
		(UINT16)(BOBBIES_MISC_SIGN_X+BOBBIES_MISC_SIGN_WIDTH),
		(UINT16)(BOBBIES_MISC_SIGN_Y+BOBBIES_MISC_SIGN_HEIGHT),
		BOBBIES_GUNS_SIGN_X,
		BOBBIES_GUNS_SIGN_Y,
		(UINT16)(BOBBIES_GUNS_SIGN_X+BOBBIES_GUNS_SIGN_WIDTH),
		(UINT16)(BOBBIES_GUNS_SIGN_Y+BOBBIES_GUNS_SIGN_HEIGHT),
		BOBBIES_AMMO_SIGN_X,
		BOBBIES_AMMO_SIGN_Y,
		(UINT16)(BOBBIES_AMMO_SIGN_X+BOBBIES_AMMO_SIGN_WIDTH),
		(UINT16)(BOBBIES_AMMO_SIGN_Y+BOBBIES_AMMO_SIGN_HEIGHT),
		BOBBIES_ARMOUR_SIGN_X,
		BOBBIES_ARMOUR_SIGN_Y,
		(UINT16)(BOBBIES_ARMOUR_SIGN_X+BOBBIES_ARMOUR_SIGN_WIDTH),
		(UINT16)(BOBBIES_ARMOUR_SIGN_Y+BOBBIES_ARMOUR_SIGN_HEIGHT)
	};

	InitBobbyRWoodBackground();

	// load the Bobbyname graphic and add it
	guiBobbyName = AddVideoObjectFromFile(MLG_BOBBYNAME);

	// load the plaque graphic and add it
	guiPlaque = AddVideoObjectFromFile(LAPTOPDIR "/bobbyplaques.sti");

	// load the TopHinge graphic and add it
	guiTopHinge = AddVideoObjectFromFile(LAPTOPDIR "/bobbytophinge.sti");

	// load the BottomHinge graphic and add it
	guiBottomHinge = AddVideoObjectFromFile(LAPTOPDIR "/bobbybottomhinge.sti");

	// load the Store Plaque graphic and add it
	guiStorePlaque = AddVideoObjectFromFile(MLG_STOREPLAQUE);

	// load the Handle graphic and add it
	guiHandle = AddVideoObjectFromFile(LAPTOPDIR "/bobbyhandle.sti");


	InitBobbiesMouseRegion(BOBBIES_NUMBER_SIGNS, usMouseRegionPosArray, gSelectedBobbiesSignMenuRegion);


	if( !LaptopSaveInfo.fBobbyRSiteCanBeAccessed )
	{
		// load the Handle graphic and add it
		guiUnderConstructionImage = AddVideoObjectFromFile(LAPTOPDIR "/underconstruction.sti");

		for(i=0; i<BOBBIES_NUMBER_SIGNS; i++)
		{
			gSelectedBobbiesSignMenuRegion[i].Disable();
		}

		LaptopSaveInfo.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction = BOBBYR_BEEN_TO_SITE_ONCE;
	}


	SetBookMark(BOBBYR_BOOKMARK);
	HandleBobbyRUnderConstructionAni( true );

	RenderBobbyR();
}


void ExitBobbyR()
{

	DeleteVideoObject(guiBobbyName);
	DeleteVideoObject(guiPlaque);
	DeleteVideoObject(guiTopHinge);
	DeleteVideoObject(guiBottomHinge);
	DeleteVideoObject(guiStorePlaque);
	DeleteVideoObject(guiHandle);

	if( !LaptopSaveInfo.fBobbyRSiteCanBeAccessed )
	{
		DeleteVideoObject(guiUnderConstructionImage);
	}


	DeleteBobbyRWoodBackground();

	FOR_EACH(MOUSE_REGION, i, gSelectedBobbiesSignMenuRegion) MSYS_RemoveRegion(&*i);

	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R;
}

void HandleBobbyR()
{
	HandleBobbyRUnderConstructionAni( false );
}

void RenderBobbyR()
{
	DrawBobbyRWoodBackground();

	BltVideoObject(FRAME_BUFFER, guiBobbyName,   0, BOBBY_RAYS_NAME_X,      BOBBY_RAYS_NAME_Y);
	BltVideoObject(FRAME_BUFFER, guiPlaque,      0, BOBBYS_PLAQUES_X,       BOBBYS_PLAQUES_Y);
	BltVideoObject(FRAME_BUFFER, guiTopHinge,    0, BOBBIES_TOPHINGE_X,     BOBBIES_TOPHINGE_Y);
	BltVideoObject(FRAME_BUFFER, guiBottomHinge, 0, BOBBIES_BOTTOMHINGE_X,  BOBBIES_BOTTOMHINGE_Y);
	BltVideoObject(FRAME_BUFFER, guiStorePlaque, 0, BOBBIES_STORE_PLAQUE_X, BOBBIES_STORE_PLAQUE_Y);
	BltVideoObject(FRAME_BUFFER, guiHandle,      0, BOBBIES_HANDLE_X,       BOBBIES_HANDLE_Y);

	SetFontShadow(BOBBIES_SENTENCE_BACKGROUNDCOLOR);


	if( LaptopSaveInfo.fBobbyRSiteCanBeAccessed )
	{
		//Bobbys first sentence
		//ShadowText( FRAME_BUFFER, BobbyRaysFrontText[BOBBYR_ADVERTISMENT_1], BOBBIES_SENTENCE_FONT, BOBBIES_FIRST_SENTENCE_X, BOBBIES_FIRST_SENTENCE_Y );
		DrawTextToScreen(BobbyRaysFrontText[BOBBYR_ADVERTISMENT_1], BOBBIES_FIRST_SENTENCE_X, BOBBIES_FIRST_SENTENCE_Y, BOBBIES_FIRST_SENTENCE_WIDTH, BOBBIES_SENTENCE_FONT, BOBBIES_SENTENCE_COLOR, BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED | TEXT_SHADOWED);

		//Bobbys second sentence
		DrawTextToScreen(BobbyRaysFrontText[BOBBYR_ADVERTISMENT_2], BOBBIES_2ND_SENTENCE_X, BOBBIES_2ND_SENTENCE_Y, BOBBIES_2ND_SENTENCE_WIDTH, BOBBIES_SENTENCE_FONT, BOBBIES_SENTENCE_COLOR, BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED | TEXT_SHADOWED);
	}


	SetFontShadow(BOBBIES_SIGN_BACKGROUNDCOLOR);
	//Text on the Used Sign
	DisplayWrappedString(BOBBIES_USED_SIGN_X, BOBBIES_USED_SIGN_TEXT_OFFSET, BOBBIES_USED_SIGN_WIDTH - 5, 2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_USED], BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED);
	//Text on the Misc Sign
	DisplayWrappedString(BOBBIES_MISC_SIGN_X, BOBBIES_MISC_SIGN_TEXT_OFFSET, BOBBIES_MISC_SIGN_WIDTH, 2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_MISC], BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED);
	//Text on the Guns Sign
	DisplayWrappedString(BOBBIES_GUNS_SIGN_X, BOBBIES_GUNS_SIGN_TEXT_OFFSET, BOBBIES_GUNS_SIGN_WIDTH, 2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_GUNS], BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED);
	//Text on the Ammo Sign
	DisplayWrappedString(BOBBIES_AMMO_SIGN_X, BOBBIES_AMMO_SIGN_TEXT_OFFSET, BOBBIES_AMMO_SIGN_WIDTH, 2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_AMMO], BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED);
	//Text on the Armour Sign
	DisplayWrappedString(BOBBIES_ARMOUR_SIGN_X, BOBBIES_ARMOUR_SIGN_TEXT_OFFSET, BOBBIES_ARMOUR_SIGN_WIDTH, 2, BOBBIES_SIGN_FONT, BOBBIES_SIGN_COLOR, BobbyRaysFrontText[BOBBYR_ARMOR], BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED);

	if( LaptopSaveInfo.fBobbyRSiteCanBeAccessed )
	{
		//Bobbys Third sentence
		SetFontShadow(BOBBIES_SENTENCE_BACKGROUNDCOLOR);
		DrawTextToScreen(BobbyRaysFrontText[BOBBYR_ADVERTISMENT_3], BOBBIES_3RD_SENTENCE_X, BOBBIES_3RD_SENTENCE_Y, BOBBIES_3RD_SENTENCE_WIDTH, BOBBIES_SENTENCE_FONT, BOBBIES_SENTENCE_COLOR, BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED | TEXT_SHADOWED);
	}

	SetFontShadow(DEFAULT_SHADOW);

	//if we cant go to any sub pages, darken the page out
	if( !LaptopSaveInfo.fBobbyRSiteCanBeAccessed )
	{
		FRAME_BUFFER->ShadowRect(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y);
	}

	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


void InitBobbyRWoodBackground()
{
	// load the Wood bacground graphic and add it
	guiWoodBackground = AddVideoObjectFromFile(LAPTOPDIR "/bobbywood.sti");
}


void DeleteBobbyRWoodBackground()
{
	DeleteVideoObject(guiWoodBackground);
}


void DrawBobbyRWoodBackground()
{
	UINT16	x,y, uiPosX, uiPosY;

	uiPosY = BOBBY_WOOD_BACKGROUND_Y;
	for(y=0; y<4; y++)
	{
		uiPosX = BOBBY_WOOD_BACKGROUND_X;
		for(x=0; x<4; x++)
		{
			BltVideoObject(FRAME_BUFFER, guiWoodBackground, 0, uiPosX, uiPosY);
			uiPosX += BOBBY_WOOD_BACKGROUND_WIDTH;
		}
		uiPosY += BOBBY_WOOD_BACKGROUND_HEIGHT;
	}
}


static void SelectBobbiesSignMenuRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static void InitBobbiesMouseRegion(UINT8 ubNumerRegions, UINT16* usMouseRegionPosArray, MOUSE_REGION* MouseRegion)
{
	UINT8 i,ubCount=0;

	for(i=0; i<ubNumerRegions; i++)
	{
		//Mouse region for the toc buttons
		MSYS_DefineRegion(&MouseRegion[i], usMouseRegionPosArray[ubCount], usMouseRegionPosArray[ubCount+1],
					usMouseRegionPosArray[ubCount+2], usMouseRegionPosArray[ubCount+3],
					MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
					SelectBobbiesSignMenuRegionCallBack);
		MSYS_SetRegionUserData( &MouseRegion[i], 0, gubBobbyRPages[i]);

		ubCount +=4;
	}
}


static void SelectBobbiesSignMenuRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = static_cast<LaptopMode>(MSYS_GetRegionUserData(pRegion, 0));
	}
}


static void HandleBobbyRUnderConstructionAni(BOOLEAN fReset)
{
	static UINT32	uiLastTime=1;
	static UINT16	usCount=0;
	UINT32	uiCurTime=GetJA2Clock();


	if( LaptopSaveInfo.fBobbyRSiteCanBeAccessed )
		return;

	if( fReset )
		usCount =1;

	if( fShowBookmarkInfo )
	{
		fReDrawBookMarkInfo = true;
	}

	if( ( ( uiCurTime - uiLastTime ) > BOBBYR_UNDERCONSTRUCTION_ANI_DELAY )||( fReDrawScreenFlag ) )
	{
		// The undercontsruction graphic
		BltVideoObject(FRAME_BUFFER, guiUnderConstructionImage, usCount, BOBBYR_UNDERCONSTRUCTION_X, BOBBYR_UNDERCONSTRUCTION_Y);
		BltVideoObject(FRAME_BUFFER, guiUnderConstructionImage, usCount, BOBBYR_UNDERCONSTRUCTION_X, BOBBYR_UNDERCONSTRUCTION1_Y);

		DrawTextToScreen(BobbyRaysFrontText[BOBBYR_UNDER_CONSTRUCTION], BOBBYR_UNDER_CONSTRUCTION_TEXT_X, BOBBYR_UNDER_CONSTRUCTION_TEXT_Y, BOBBYR_UNDER_CONSTRUCTION_TEXT_WIDTH, FONT16ARIAL, BOBBIES_SENTENCE_COLOR, BOBBIES_SIGN_BACKCOLOR, CENTER_JUSTIFIED | INVALIDATE_TEXT);

		InvalidateRegion( BOBBYR_UNDERCONSTRUCTION_X, BOBBYR_UNDERCONSTRUCTION_Y, BOBBYR_UNDERCONSTRUCTION_X+BOBBYR_UNDERCONSTRUCTION_WIDTH, BOBBYR_UNDERCONSTRUCTION_Y+BOBBYR_UNDERCONSTRUCTION_HEIGHT );
		InvalidateRegion( BOBBYR_UNDERCONSTRUCTION_X, BOBBYR_UNDERCONSTRUCTION1_Y, BOBBYR_UNDERCONSTRUCTION_X+BOBBYR_UNDERCONSTRUCTION_WIDTH, BOBBYR_UNDERCONSTRUCTION1_Y+BOBBYR_UNDERCONSTRUCTION_HEIGHT );

		uiLastTime = GetJA2Clock();

		usCount++;

		if( usCount >= BOBBYR_UNDERCONSTRUCTION_NUM_FRAMES )
			usCount = 0;
	}
}


static void InitBobbyRayNewInventory(void);
static void InitBobbyRayUsedInventory(void);


void InitBobbyRayInventory()
{
	//Initializes which NEW items can be bought at Bobby Rays
	InitBobbyRayNewInventory();

	//Initializes the starting values for Bobby Rays NEW Inventory
	SetupStoreInventory( LaptopSaveInfo.BobbyRayInventory, false );

	//Initializes which USED items can be bought at Bobby Rays
	InitBobbyRayUsedInventory();

	//Initializes the starting values for Bobby Rays USED Inventory
	SetupStoreInventory( LaptopSaveInfo.BobbyRayUsedInventory, true);
}


static void InitBobbyRayNewInventory(void)
{
	UINT16	usBobbyrIndex = 0;


	std::fill_n(LaptopSaveInfo.BobbyRayInventory, static_cast<size_t>(MAXITEMS), STORE_INVENTORY{});

	// add all the NEW items he can ever sell into his possible inventory list, for now in order by item #
	for (auto item : GCM->getItems())
	{
		const auto itemIndex = item->getItemIndex();

		//if Bobby Ray sells this, it can be sold, and it's allowed into this game (some depend on e.g. gun-nut option)
		if( (GCM->getBobbyRayNewInventory()->getMaxItemAmount(item) != 0) && !(item->getFlags() & ITEM_NOT_BUYABLE) && ItemIsLegal(itemIndex) )
		{
			LaptopSaveInfo.BobbyRayInventory[ usBobbyrIndex ].usItemIndex = itemIndex;
			usBobbyrIndex++;
		}
	}

	if ( usBobbyrIndex > 1 )
	{
		// sort this list by object category, and by ascending price within each category
		qsort( LaptopSaveInfo.BobbyRayInventory, usBobbyrIndex, sizeof( STORE_INVENTORY ), BobbyRayItemQsortCompare );
	}


	// remember how many entries in the list are valid
	LaptopSaveInfo.usInventoryListLength[ BOBBY_RAY_NEW ] = usBobbyrIndex;
	// also mark the end of the list of valid item entries
	LaptopSaveInfo.BobbyRayInventory[ usBobbyrIndex ].usItemIndex = BOBBYR_NO_ITEMS;
}


static void InitBobbyRayUsedInventory(void)
{
	UINT16	usBobbyrIndex = 0;


	std::fill_n(LaptopSaveInfo.BobbyRayUsedInventory, static_cast<size_t>(MAXITEMS), STORE_INVENTORY{});

	// add all the NEW items he can ever sell into his possible inventory list, for now in order by item #
	for (auto item : GCM->getItems())
	{
		const auto itemIndex = item->getItemIndex();

		//if Bobby Ray sells this, it can be sold, and it's allowed into this game (some depend on e.g. gun-nut option)
		if( (GCM->getBobbyRayUsedInventory()->getMaxItemAmount(item) != 0) && !(item->getFlags() & ITEM_NOT_BUYABLE) && ItemIsLegal(itemIndex) )
		{
			// in case his store inventory list is wrong, make sure this category of item can be sold used
			if ( CanDealerItemBeSoldUsed(itemIndex) )
			{
				LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyrIndex ].usItemIndex = itemIndex;
				usBobbyrIndex++;
			}
		}
	}

	if ( usBobbyrIndex > 1 )
	{
		// sort this list by object category, and by ascending price within each category
		qsort( LaptopSaveInfo.BobbyRayUsedInventory, usBobbyrIndex, sizeof( STORE_INVENTORY ), BobbyRayItemQsortCompare );
	}

	// remember how many entries in the list are valid
	LaptopSaveInfo.usInventoryListLength[BOBBY_RAY_USED] = usBobbyrIndex;
	// also mark the end of the list of valid item entries
	LaptopSaveInfo.BobbyRayUsedInventory[ usBobbyrIndex ].usItemIndex = BOBBYR_NO_ITEMS;
}


static UINT8 HowManyBRItemsToOrder(UINT16 usItemIndex, UINT8 ubCurrentlyOnHand, UINT8 ubBobbyRayNewUsed);
static void OrderBobbyRItem(UINT16 usItemIndex);
static void SimulateBobbyRayCustomer(STORE_INVENTORY* pInventoryArray, BOOLEAN fUsed);


void DailyUpdateOfBobbyRaysNewInventory()
{
	//simulate other buyers by reducing the current quantity on hand
	SimulateBobbyRayCustomer(LaptopSaveInfo.BobbyRayInventory, BOBBY_RAY_NEW);

	//loop through all items BR can stock to see what needs reordering
	for(auto i = 0; i < LaptopSaveInfo.usInventoryListLength[BOBBY_RAY_NEW]; i++)
	{
		// the index is NOT the item #, get that from the table
		auto usItemIndex = LaptopSaveInfo.BobbyRayInventory[ i ].usItemIndex;
		auto item = GCM->getItem(usItemIndex);

		// make sure this item is still sellable in the latest version of the store inventory
		if (GCM->getBobbyRayNewInventory()->getMaxItemAmount(item) == 0 )
		{
			continue;
		}

		//if the item isn't already on order
		if( LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnOrder == 0)
		{
			//if the qty on hand is half the desired amount or fewer
			if( LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnHand <= (GCM->getBobbyRayNewInventory()->getMaxItemAmount(item) / 2 ) )
			{
				// remember value of the "previously eligible" flag
				auto fPrevElig = LaptopSaveInfo.BobbyRayInventory[ i ].fPreviouslyEligible;

				//determine if any can/should be ordered, and how many
				LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnOrder = HowManyBRItemsToOrder( usItemIndex, LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnHand, BOBBY_RAY_NEW);

				//if he found some to buy
				if( LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnOrder > 0 )
				{
					// if this is the first day the player is eligible to have access to this thing
					if ( !fPrevElig )
					{
						// eliminate the ordering delay and stock the items instantly!
						// This is just a way to reward the player right away for making progress without the reordering lag...
						AddFreshBobbyRayInventory( usItemIndex );
					}
					else
					{
						OrderBobbyRItem(usItemIndex);
					}
				}
			}
		}
	}
}


void DailyUpdateOfBobbyRaysUsedInventory()
{
	//simulate other buyers by reducing the current quantity on hand
	SimulateBobbyRayCustomer(LaptopSaveInfo.BobbyRayUsedInventory, BOBBY_RAY_USED);

	for(auto i = 0; i < LaptopSaveInfo.usInventoryListLength[BOBBY_RAY_USED]; i++)
	{
		//if the used item isn't already on order
		if( LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnOrder == 0 )
		{
			//if we don't have ANY
			if( LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnHand == 0 )
			{
				// the index is NOT the item #, get that from the table
				auto usItemIndex = LaptopSaveInfo.BobbyRayUsedInventory[ i ].usItemIndex;
				auto item = GCM->getItem(usItemIndex);

				// make sure this item is still sellable in the latest version of the store inventory
				if (GCM->getBobbyRayUsedInventory()->getMaxItemAmount(item) == 0 )
				{
					continue;
				}

				// remember value of the "previously eligible" flag
				auto fPrevElig = LaptopSaveInfo.BobbyRayUsedInventory[ i ].fPreviouslyEligible;

				//determine if any can/should be ordered, and how many
				LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnOrder = HowManyBRItemsToOrder(usItemIndex, LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnHand, BOBBY_RAY_USED);

				//if he found some to buy
				if( LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnOrder > 0 )
				{
					// if this is the first day the player is eligible to have access to this thing
					if ( !fPrevElig )
					{
						// eliminate the ordering delay and stock the items instantly!
						// This is just a way to reward the player right away for making progress without the reordering lag...
						AddFreshBobbyRayInventory( usItemIndex );
					}
					else
					{
						OrderBobbyRItem((INT16) (usItemIndex + BOBBY_R_USED_PURCHASE_OFFSET));
					}
				}
			}
		}
	}
}


//returns the number of items to order
static UINT8 HowManyBRItemsToOrder(UINT16 usItemIndex, UINT8 ubCurrentlyOnHand, UINT8 ubBobbyRayNewUsed)
{
	UINT8	ubItemsOrdered = 0;

	auto inv = ubBobbyRayNewUsed ? GCM->getBobbyRayUsedInventory() : GCM->getBobbyRayNewInventory();
	auto item = GCM->getItem(usItemIndex);

	// formulas below will fail if there are more items already in stock than optimal
	Assert(ubCurrentlyOnHand <= inv->getMaxItemAmount(item));
	Assert(ubBobbyRayNewUsed < BOBBY_RAY_LISTS);


	// decide if he can get stock for this item (items are reordered an entire batch at a time)
	if (ItemTransactionOccurs(ARMS_DEALER_BOBBYR, usItemIndex, DEALER_BUYING, ubBobbyRayNewUsed))
	{
		if (ubBobbyRayNewUsed == BOBBY_RAY_NEW)
		{
			ubItemsOrdered = HowManyItemsToReorder(inv->getMaxItemAmount(item), ubCurrentlyOnHand);
		}
		else
		{
			//Since these are used items we only should order 1 of each type
			ubItemsOrdered = 1;
		}
	}
	else
	{
		// can't obtain this item from suppliers
		ubItemsOrdered = 0;
	}


	return(ubItemsOrdered);
}


static void OrderBobbyRItem(UINT16 usItemIndex)
{
	UINT32 uiArrivalTime;

	//add the new item to the queue.  The new item will arrive in 'uiArrivalTime' minutes.
	uiArrivalTime = BOBBY_R_NEW_PURCHASE_ARRIVAL_TIME + Random( BOBBY_R_NEW_PURCHASE_ARRIVAL_TIME / 2 );
	uiArrivalTime += GetWorldTotalMin();
	AddStrategicEvent( EVENT_UPDATE_BOBBY_RAY_INVENTORY, uiArrivalTime, usItemIndex);
}


void AddFreshBobbyRayInventory( UINT16 usItemIndex )
{
	INT16 sInventorySlot;
	STORE_INVENTORY *pInventoryArray;
	BOOLEAN fUsed;
	UINT8 ubItemQuality;


	if( usItemIndex >= BOBBY_R_USED_PURCHASE_OFFSET )
	{
		usItemIndex -= BOBBY_R_USED_PURCHASE_OFFSET;
		pInventoryArray = LaptopSaveInfo.BobbyRayUsedInventory;
		fUsed = BOBBY_RAY_USED;
		ubItemQuality = 20 + (UINT8) Random( 60 );
	}
	else
	{
		pInventoryArray = LaptopSaveInfo.BobbyRayInventory;
		fUsed = BOBBY_RAY_NEW;
		ubItemQuality = 100;
	}


	// find out which inventory slot that item is stored in
	sInventorySlot = GetInventorySlotForItem(pInventoryArray, usItemIndex, fUsed);
	if (sInventorySlot == -1)
	{
		AssertMsg(false, ST::format("AddFreshBobbyRayInventory(), Item {} not found. AM-0.", usItemIndex));
		return;
	}


	pInventoryArray[ sInventorySlot ].ubQtyOnHand += pInventoryArray[ sInventorySlot ].ubQtyOnOrder;
	pInventoryArray[ sInventorySlot ].ubItemQuality = ubItemQuality;

	// cancel order
	pInventoryArray[ sInventorySlot ].ubQtyOnOrder = 0;
}


INT16 GetInventorySlotForItem(STORE_INVENTORY *pInventoryArray, UINT16 usItemIndex, BOOLEAN fUsed)
{
	INT16 i;

	for(i = 0; i < LaptopSaveInfo.usInventoryListLength[fUsed]; i++)
	{
		//if we have some of this item in stock
		if( pInventoryArray[ i ].usItemIndex == usItemIndex)
		{
			return(i);
		}
	}

	// not found!
	return(-1);
}


static void SimulateBobbyRayCustomer(STORE_INVENTORY* pInventoryArray, BOOLEAN fUsed)
{
	INT16 i;
	UINT8 ubItemsSold;

	//loop through all items BR can stock to see what gets sold
	for(i = 0; i < LaptopSaveInfo.usInventoryListLength[fUsed]; i++)
	{
		//if we have some of this item in stock
		if( pInventoryArray[ i ].ubQtyOnHand > 0)
		{
			ubItemsSold = HowManyItemsAreSold(ARMS_DEALER_BOBBYR, pInventoryArray[i].usItemIndex, pInventoryArray[i].ubQtyOnHand, fUsed);
			pInventoryArray[ i ].ubQtyOnHand -= ubItemsSold;
		}
	}
}


void CancelAllPendingBRPurchaseOrders(void)
{
	INT16 i;

	// remove all the BR-Order events off the event queue
	DeleteAllStrategicEventsOfType( EVENT_UPDATE_BOBBY_RAY_INVENTORY );

	// zero out all the quantities on order
	for(i = 0; i < MAXITEMS; i++)
	{
		LaptopSaveInfo.BobbyRayInventory[ i ].ubQtyOnOrder = 0;
		LaptopSaveInfo.BobbyRayUsedInventory[ i ].ubQtyOnOrder = 0;
	}

	// do an extra daily update immediately to create new reorders ASAP
	DailyUpdateOfBobbyRaysNewInventory();
	DailyUpdateOfBobbyRaysUsedInventory();
}
