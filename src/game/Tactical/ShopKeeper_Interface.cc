#include "Directories.h"
#include "Font.h"
#include "GameLoop.h"
#include "HImage.h"
#include "Handle_Items.h"
#include "Isometric_Utils.h"
#include "JAScreens.h"
#include "Local.h"
#include "Timer_Control.h"
#include "Types.h"
#include "MercPortrait.h"
#include "ShopKeeper_Interface.h"
#include "Game_Clock.h"
#include "Render_Dirty.h"
#include "VObject.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "WordWrap.h"
#include "Text_Input.h"
#include "Interface.h"
#include "Interface_Control.h"
#include "Overhead.h"
#include "Cursors.h"
#include "SysUtil.h"
#include "Interface_Panels.h"
#include "Radar_Screen.h"
#include "Interface_Items.h"
#include "Interface_Utils.h"
#include "VObject_Blitters.h"
#include "Finances.h"
#include "Text.h"
#include "Cursor_Control.h"
#include "Input.h"
#include "Arms_Dealer_Init.h"
#include "English.h"
#include "Soldier_Add.h"
#include "Faces.h"
#include "Dialogue_Control.h"
#include "ShopKeeper_Quotes.h"
#include "GameSettings.h"
#include "MercTextBox.h"
#include "Random.h"
#include "Squads.h"
#include "Soldier_Profile.h"
#include "Message.h"
#include "LaptopSave.h"
#include "Quests.h"
#include "Weapons.h"
#include "Line.h"
#include "Drugs_And_Alcohol.h"
#include "Map_Screen_Interface.h"
#include "Soldier_Macros.h"
#include "ArmsDealerInvInit.h"
#include "OppList.h"
#include "NPC.h"
#include "LOS.h"
#include "Button_System.h"
#include "Video.h"
#include "MemMan.h"
#include "Debug.h"
#include "Items.h"
#include "UILayout.h"
#include "CalibreModel.h"
#include "ContentManager.h"
#include "DealerModel.h"
#include "GameInstance.h"
#include "MagazineModel.h"
#include "WeaponModels.h"
#include "policy/GamePolicy.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>

#define SKI_BUTTON_FONT				MILITARYFONT1//FONT14ARIAL
#define SKI_BUTTON_COLOR				73

#define SKI_TITLE_FONT					MILITARYFONT1//FONT14ARIAL
#define SKI_TITLE_COLOR				169//FONT_MCOLOR_LTYELLOW

#define SKI_LABEL_FONT					MILITARYFONT1

#define SKI_ITEM_DESC_FONT				SMALLCOMPFONT
#define SKI_ITEM_PRICE_COLOR				FONT_MCOLOR_WHITE

#define SKIT_NUMBER_FONT				BLOCKFONT2

#define SKI_MAIN_BACKGROUND_X				0
#define SKI_MAIN_BACKGROUND_Y				0

#define SKI_FACE_X					13
#define SKI_FACE_Y					13
#define SKI_FACE_WIDTH					90
#define SKI_FACE_HEIGHT				100

#define SKI_PAGE_UP_ARROWS_X				121
#define SKI_PAGE_UP_ARROWS_Y				35

#define SKI_PAGE_DOWN_ARROWS_X				SKI_PAGE_UP_ARROWS_X
#define SKI_PAGE_DOWN_ARROWS_Y				102

#define SKI_TRANSACTION_BUTTON_X			147//214
#define SKI_DONE_BUTTON_X				292//414
#define SKI_BUTTON_Y					233

#define SKI_MAIN_TITLE_X				112
#define SKI_MAIN_TITLE_Y				12

#define SKI_MAIN_TITLE_WIDTH				420

#define SKI_TOTAL_COST_X				9
#define SKI_TOTAL_COST_Y				162//159
#define SKI_TOTAL_COST_WIDTH				73

#define SKI_TOTAL_VALUE_X				SKI_TOTAL_COST_X
#define SKI_TOTAL_VALUE_Y				291//268
#define SKI_TOTAL_VALUE_WIDTH				SKI_TOTAL_COST_WIDTH

#define SKI_PLAYERS_CURRENT_BALANCE_X			SKI_TOTAL_COST_X
#define SKI_PLAYERS_CURRENT_BALANCE_Y			235
#define SKI_PLAYERS_CURRENT_BALANCE_WIDTH		SKI_TOTAL_COST_WIDTH
#define SKI_PLAYERS_CURRENT_BALANCE_OFFSET_TO_VALUE	265

#define SKI_PAGE_X					112
#define SKI_PAGE_Y					70
#define SKI_PAGE_WIDTH					45
#define SKI_PAGE_HEIGHT				27


//Number of Inventory slots
#define SKI_NUM_ARMS_DEALERS_INV_SLOTS			15
#define SKI_NUM_ARMS_DEALERS_INV_COLS			5

#define SKI_NUM_TRADING_INV_SLOTS			12
#define SKI_NUM_TRADING_INV_COLS			6

//Inventory Slots size and offsets
#define SKI_INV_SLOT_WIDTH				67
#define SKI_INV_SLOT_HEIGHT				31
#define SKI_INV_HEIGHT					SKI_INV_SLOT_HEIGHT - 7
#define SKI_INV_WIDTH					60

#define SKI_INV_PRICE_OFFSET_X				1
#define SKI_INV_PRICE_OFFSET_Y				24

#define SKI_INV_OFFSET_X				74
#define SKI_INV_OFFSET_Y				36


//Start Locations for the inventory boxes
#define SKI_ARMS_DEALERS_INV_START_X			165
#define SKI_ARMS_DEALERS_INV_START_Y			30

#define SKI_ARMS_DEALERS_TRADING_INV_X			91
#define SKI_ARMS_DEALERS_TRADING_INV_Y			151
#define SKI_ARMS_DEALERS_TRADING_INV_WIDTH		436
#define SKI_ARMS_DEALERS_TRADING_INV_HEIGHT		67


#define SKI_PLAYERS_TRADING_INV_X			91
#define SKI_PLAYERS_TRADING_INV_Y			266
#define SKI_PLAYERS_TRADING_INV_HEIGHT			70
#define SKI_PLAYERS_TRADING_INV_WIDTH			440

#define SKI_ARMS_DEALER_TOTAL_COST_X			16
#define SKI_ARMS_DEALER_TOTAL_COST_Y			194//191
#define SKI_ARMS_DEALER_TOTAL_COST_WIDTH		59
#define SKI_ARMS_DEALER_TOTAL_COST_HEIGHT		20

#define SKI_PLAYERS_TOTAL_VALUE_X			16
#define SKI_PLAYERS_TOTAL_VALUE_Y			310//308
#define SKI_PLAYERS_TOTAL_VALUE_WIDTH			59
#define SKI_PLAYERS_TOTAL_VALUE_HEIGHT			20


#define SKI_TACTICAL_BACKGROUND_START_X		536
#define SKI_TACTICAL_BACKGROUND_START_Y		0
#define SKI_DROP_ITEM_TO_GROUND_START_X		SKI_TACTICAL_BACKGROUND_START_X
#define SKI_DROP_ITEM_TO_GROUND_START_Y		262
#define SKI_DROP_ITEM_TO_GROUND_TEXT_START_Y		262

#define SKI_TACTICAL_BACKGROUND_START_WIDTH		(UINT16)(SCREEN_WIDTH - SKI_TACTICAL_BACKGROUND_START_X)
#define SKI_TACTICAL_BACKGROUND_START_HEIGHT		340

#define SKI_ITEM_MOVEMENT_AREA_X			85
#define SKI_ITEM_MOVEMENT_AREA_Y			263
#define SKI_ITEM_MOVEMENT_AREA_WIDTH			(SCREEN_WIDTH - SKI_ITEM_MOVEMENT_AREA_X)
//#define SKI_ITEM_MOVEMENT_AREA_WIDTH			448
#define SKI_ITEM_MOVEMENT_AREA_HEIGHT			215//72

#define SKI_DEALER_OFFER_AREA_Y			148
//#define SKI_DEALER_OFFER_AREA_Y			148


#define SKI_ITEM_NUMBER_TEXT_OFFSET_X			50
#define SKI_ITEM_NUMBER_TEXT_OFFSET_Y			15
#define SKI_ITEM_NUMBER_TEXT_WIDTH			15

#define SKI_SUBTITLE_TEXT_SIZE				512

#define SKI_POSITION_SUBTITLES_Y			140//100

#define SKI_SMALL_FACE_WIDTH				16
#define SKI_SMALL_FACE_HEIGHT				14
#define SKI_SMALL_FACE_OFFSET_X			52

#define SKI_ATTACHMENT_SYMBOL_X_OFFSET			56
#define SKI_ATTACHMENT_SYMBOL_Y_OFFSET			14


#define SKI_MAX_AMOUNT_OF_ITEMS_DEALER_CAN_REPAIR_AT_A_TIME	4

#define SKI_DEALERS_RANDOM_QUOTE_DELAY			15000
#define SKI_DEALERS_RANDOM_QUOTE_DELAY_INCREASE_RATE	5000

#define DELAY_FOR_SHOPKEEPER_IDLE_QUOTE		20000
#define CHANCE_FOR_SHOPKEEPER_IDLE_QUOTE		40

#define MAX_SUBOBJECTS_PER_OBJECT			MAX(MAX_OBJECTS_PER_SLOT, (2 + MAX_ATTACHMENTS)) // (2nd part is main item, ammo/payload, and 4 attachments)

#define REALLY_BADLY_DAMAGED_THRESHOLD			30

#define REPAIR_DELAY_IN_HOURS				6

#define FLO_DISCOUNT_PERCENTAGE			10


static SGPVObject* guiMainTradeScreenImage;
static SGPVSurface* guiCornerWhereTacticalIsStillSeenImage; // This image is for where the corner of tactical is still seen through the shop keeper interface

static BOOLEAN gfSKIScreenEntry = TRUE;
static BOOLEAN gfSKIScreenExit  = FALSE;
static BOOLEAN gfUserHasRequestedToLeave = FALSE;

static BOOLEAN gfRenderScreenOnNextLoop = FALSE;

UINT8 gubSkiDirtyLevel = SKI_DIRTY_LEVEL0;

static ArmsDealerID gbSelectedArmsDealerID = ARMS_DEALER_INVALID; //Contains the enum value for the currently selected arms dealer

//the quote that is in progress, in certain circumstances, we don't want queuing of related but different quotes
static INT32 giShopKeepDialogueEventinProgress = -1;

INVENTORY_IN_SLOT gMoveingItem;

const OBJECTTYPE* gpHighLightedItemObject = NULL;

static BOOLEAN gfResetShopKeepIdleQuote = FALSE;
static BOOLEAN gfDoEvaluationAfterOpening = FALSE;

struct SELECTED_ARMS_DEALERS_STATS
{
	UINT32 uiNumDistinctInventoryItems;
	UINT8  ubCurrentPage;
	UINT8  ubNumberOfPages;

	UINT8  ubFirstItemIndexOnPage;
};



static SELECTED_ARMS_DEALERS_STATS gSelectArmsDealerInfo;


//This pointer is used to store the inventory the arms dealer has for sale
static INVENTORY_IN_SLOT* gpTempDealersInventory = NULL;

static INVENTORY_IN_SLOT ArmsDealerOfferArea[SKI_NUM_TRADING_INV_SLOTS];
static INVENTORY_IN_SLOT PlayersOfferArea[SKI_NUM_TRADING_INV_SLOTS];

static OBJECTTYPE gSubObject[MAX_SUBOBJECTS_PER_OBJECT];

static BOOLEAN gfDealerHasSaidTheEvaluateQuoteOnceThisSession = FALSE;
static BOOLEAN gfAlreadySaidTooMuchToRepair                   = FALSE;
static UINT32  guiRandomQuoteDelayTime                        = SKI_DEALERS_RANDOM_QUOTE_DELAY;

//Index for the shopkeepers face
static FACETYPE* giShopKeeperFaceIndex;

//Id for the popup box
static MercPopUpBox* g_popup_box;

static BOOLEAN gfIsTheShopKeeperTalking;


static BOOLEAN gfRemindedPlayerToPickUpHisStuff = FALSE;

static BOOLEAN gfDoneBusinessThisSession = FALSE;

// this is used within SKI exclusively, to handle small faces
static UINT8  gubArrayOfEmployedMercs[MAX_CHARACTER_COUNT];
static SGPVObject* guiSmallSoldiersFace[MAX_CHARACTER_COUNT];
static UINT8  gubNumberMercsInArray;

static UINT16 gusPositionOfSubTitlesX = 0;

static BOOLEAN gfExitSKIDueToMessageBox = FALSE;

OBJECTTYPE *pShopKeeperItemDescObject=NULL;

static UINT32 guiNextFreeInvSlot;

static BOOLEAN gfStartWithRepairsDelayedQuote = FALSE;

static BOOLEAN gfPerformTransactionInProgress = FALSE;

static BOOLEAN gfCommonQuoteUsedThisSession[NUM_COMMON_SK_QUOTES];


// Enums for possible evaluation results
enum
{
	EVAL_RESULT_NORMAL,
	EVAL_RESULT_OK_BUT_REALLY_DAMAGED,
	EVAL_RESULT_DONT_HANDLE,
	EVAL_RESULT_WORTHLESS,
	EVAL_RESULT_NOT_DAMAGED,
	EVAL_RESULT_NON_REPAIRABLE,
	EVAL_RESULT_ROCKET_RIFLE,

	NUM_EVAL_RESULTS
};

static BOOLEAN gfEvalResultQuoteSaid[NUM_EVAL_RESULTS];

static UINT32 guiLastTimeDealerSaidNormalEvaluationQuote = 0;

static BOOLEAN gfSkiDisplayDropItemToGroundText = FALSE;

struct ITEM_TO_ADD_AFTER_SKI_OPEN
{
	BOOLEAN fActive;
	OBJECTTYPE ItemObject;
	INT8 bPreviousInvPos;
};
static ITEM_TO_ADD_AFTER_SKI_OPEN gItemToAdd;


//Page up buttons for the merchants
static void BtnSKI_InvPageUpButtonCallback(GUI_BUTTON* btn, INT32 reason);
static BUTTON_PICS* guiSKI_InvPageUpButtonImage;
static GUIButtonRef guiSKI_InvPageUpButton;

//Page down buttons for the merchants
static void BtnSKI_InvPageDownButtonCallback(GUI_BUTTON* btn, INT32 reason);
static BUTTON_PICS* guiSKI_InvPageDownButtonImage;
static GUIButtonRef guiSKI_InvPageDownButton;


//Transaction buttons
static void BtnSKI_TransactionButtonCallback(GUI_BUTTON* btn, INT32 reason);
static BUTTON_PICS* guiSKI_TransactionButtonImage;
static GUIButtonRef guiSKI_TransactionButton;

//Done buttons
static void BtnSKI_DoneButtonCallback(GUI_BUTTON* btn, INT32 reason);
static BUTTON_PICS* guiSKI_DoneButtonImage;
static GUIButtonRef guiSKI_DoneButton;

static SGPVObject* guiItemCrossOut;

static BOOLEAN gfDisplayNoRoomMsg = FALSE;

//Blanket the entire screen
static MOUSE_REGION gSKI_EntireScreenMouseRegions;

static MOUSE_REGION g_dealer_inventory_scroll_region;

static MOUSE_REGION gDealersInventoryMouseRegions[SKI_NUM_ARMS_DEALERS_INV_SLOTS];
static MOUSE_REGION gRepairmanInventorySmallFaceMouseRegions[SKI_NUM_ARMS_DEALERS_INV_SLOTS];

static MOUSE_REGION gDealersOfferSlotsMouseRegions[SKI_NUM_TRADING_INV_SLOTS];

static MOUSE_REGION gPlayersOfferSlotsMouseRegions[SKI_NUM_TRADING_INV_SLOTS];

static MOUSE_REGION gDealersOfferSlotsSmallFaceMouseRegions[SKI_NUM_TRADING_INV_SLOTS];
static MOUSE_REGION gPlayersOfferSlotsSmallFaceMouseRegions[SKI_NUM_TRADING_INV_SLOTS];


static MOUSE_REGION gSkiInventoryMovementAreaMouseRegions;


//Mouse region for the subtitles region when the merc is talking
static MOUSE_REGION gShopKeeperSubTitleMouseRegion;

static MOUSE_REGION gArmsDealersFaceMouseRegions;


//Region to allow the user to drop items to the ground
static MOUSE_REGION gArmsDealersDropItemToGroundMouseRegions;

//
// screen handler functions
//

void ShopKeeperScreenInit()
{
	//Set so next time we come in, we can set up
	gfSKIScreenEntry = TRUE;
}


static void EnterShopKeeperInterface(void);
static void ExitShopKeeperInterface(void);
static void GetShopKeeperInterfaceUserInput(void);
static void HandleShopKeeperInterface(void);
static void RenderShopKeeperInterface(void);


const DealerModel* SelectedArmsDealer()
{
	auto dealer = GCM->getDealer(gbSelectedArmsDealerID);
	if (dealer == NULL)
	{
		ST::string err = ST::format("Dealer is NULL ({})", gbSelectedArmsDealerID);
		throw std::runtime_error(err.to_std_string());
	}
	return dealer;
}

ScreenID ShopKeeperScreenHandle()
{
	if( gfSKIScreenEntry )
	{
		PauseGame();

		try
		{
			EnterShopKeeperInterface();
		}
		catch (...) // XXX fishy, should probably propagate
		{
			gfSKIScreenExit = TRUE;
			EnterTacticalScreen();
			return SHOPKEEPER_SCREEN;
		}

		gfSKIScreenEntry = FALSE;
		gfSKIScreenExit  = FALSE;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		gfRenderScreenOnNextLoop = TRUE;
		InvalidateScreen();
	}

	if( gfRenderScreenOnNextLoop )
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

	RestoreBackgroundRects();

	GetShopKeeperInterfaceUserInput();


	// Check for any newly added items...
	if ( gpSMCurrentMerc->fCheckForNewlyAddedItems )
	{
		// Startup any newly added items....
		CheckForAnyNewlyAddedItems( gpSMCurrentMerc );
		gpSMCurrentMerc->fCheckForNewlyAddedItems = FALSE;
	}


	HandleShopKeeperInterface();


	if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
	{
		RenderShopKeeperInterface();

		fInterfacePanelDirty = DIRTYLEVEL2;

		gubSkiDirtyLevel = SKI_DIRTY_LEVEL0;
	}
	else if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL1 )
	{
		fInterfacePanelDirty = DIRTYLEVEL2;

		gubSkiDirtyLevel = SKI_DIRTY_LEVEL0;
	}

	// render buttons marked dirty
	DisableSMPpanelButtonsWhenInShopKeeperInterface();
	RenderButtons( );

	// render help
	SaveBackgroundRects( );
	RenderButtonsFastHelp( );

	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();

	if( gfSKIScreenExit )
	{
		ExitShopKeeperInterface();
		gfSKIScreenExit = FALSE;
		gfSKIScreenEntry = TRUE;
		EnterTacticalScreen( );
		UnPauseGame();
	}

	if ( gfDisplayNoRoomMsg )
	{
		// tell player there's not enough room in the player's offer area
		// ARM: message is delayed because we need the mouse restriction to be in place
		// BEFORE it comes up so it gets lifted/restored
		DoSkiMessageBox(SKI_Text[SKI_TEXT_NO_MORE_ROOM_IN_PLAYER_OFFER_AREA], SHOPKEEPER_SCREEN,
					MSG_BOX_FLAG_OK, NULL);

		gfDisplayNoRoomMsg = FALSE;
	}


	return( SHOPKEEPER_SCREEN );
}


void ShopKeeperScreenShutdown(void)
{
	ShutDownArmsDealers();
}


static GUIButtonRef MakeButton(BUTTON_PICS* img, const ST::string& text, INT16 x, INT16 prio, GUI_CALLBACK click, const ST::string& help)
{
	const INT16 text_col   = SKI_BUTTON_COLOR;
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, SKI_BUTTON_FONT, text_col, shadow_col, text_col, shadow_col, x, SKI_BUTTON_Y, prio, click);
	btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_HATCHED);
	btn->SetFastHelpText(help);
	return btn;
}


static UINT8 CountNumberOfItemsInThePlayersOfferArea();
static void CreateSkiInventorySlotMouseRegions(void);
static void HandlePossibleRepairDelays(void);
static void HandleShopKeeperDialog(UINT8 ubInit);
static BOOLEAN InitShopKeepersFace(UINT8 ubMercID);
static void InitializeShopKeeper(BOOLEAN fResetPage);
static BOOLEAN OfferObjectToDealer(OBJECTTYPE* pComplexObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId);
static void ResetAllQuoteSaidFlags();
static void SelectArmsDealersDropItemToGroundMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectArmsDealersDropItemToGroundRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectArmsDealersFaceRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void EnterShopKeeperInterface(void)
{
	// make sure current merc is close enough and eligible to talk to the shopkeeper.
	AssertMsg(CanMercInteractWithSelectedShopkeeper(GetSelectedMan()), "Selected merc can't interact with shopkeeper.  Send save AM-1");

	// Create a video surface to blt corner of the tactical screen that still shines through
	guiCornerWhereTacticalIsStillSeenImage = AddVideoSurface(SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT, PIXEL_DEPTH);

	//Clear out all the save background rects
	EmptyBackgroundRects( );

	if( gfExitSKIDueToMessageBox )
	{
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		gfExitSKIDueToMessageBox = FALSE;
	}

	//Check to make sure the inventory is null ( should always be null if we are just coming in to the SKI )
	Assert( gpTempDealersInventory == NULL );

	//Reinitialize the team panel to be the SM panel
	SetCurrentInterfacePanel( SM_PANEL );
	SOLDIERTYPE* const sel = GetSelectedMan();
	SetCurrentTacticalPanelCurrentMerc(sel);
	SetSMPanelCurrentMerc(sel);

	// load the Main trade screen backgroiund image
	guiMainTradeScreenImage = AddVideoObjectFromFile(INTERFACEDIR "/tradescreen.sti");

	// load the Main trade screen background image
	guiItemCrossOut = AddVideoObjectFromFile(INTERFACEDIR "/itemcrossout.sti");

	//Create an array of all mercs (anywhere!) currently in the player's employ, and load their small faces
	// This is to support showing of repair item owner's faces even when they're not in the sector, as long as they still work for player
	gubNumberMercsInArray = 0;
	CFOR_EACH_IN_TEAM(pSoldier, OUR_TEAM)
	{
		if (pSoldier->ubProfile != NO_PROFILE && !IsMechanical(*pSoldier))
		{
			// remember whose face is in this slot
			gubArrayOfEmployedMercs[ gubNumberMercsInArray ] = pSoldier->ubProfile;

			//While we are at it, add their small face
			guiSmallSoldiersFace[gubNumberMercsInArray] = Load33Portrait(GetProfile(pSoldier->ubProfile));

			gubNumberMercsInArray++;
		}
	}

	//Load the graphic for the arrow button
	guiSKI_InvPageUpButtonImage   = LoadButtonImage(INTERFACEDIR "/tradescrollarrows.sti", 0, 1);
	guiSKI_InvPageDownButtonImage = UseLoadedButtonImage(guiSKI_InvPageUpButtonImage,      2, 3);


	//Page up button for the merchant inventory
	guiSKI_InvPageUpButton = QuickCreateButton(guiSKI_InvPageUpButtonImage, SKI_PAGE_UP_ARROWS_X, SKI_PAGE_UP_ARROWS_Y, MSYS_PRIORITY_HIGHEST, BtnSKI_InvPageUpButtonCallback);
	guiSKI_InvPageUpButton->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_HATCHED);

	//Page down button for the merchant inventory
	guiSKI_InvPageDownButton = QuickCreateButton(guiSKI_InvPageDownButtonImage, SKI_PAGE_DOWN_ARROWS_X, SKI_PAGE_DOWN_ARROWS_Y, MSYS_PRIORITY_HIGHEST, BtnSKI_InvPageDownButtonCallback);
	guiSKI_InvPageDownButton->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_HATCHED);


	guiSKI_TransactionButtonImage = LoadButtonImage(INTERFACEDIR "/tradebuttons.sti",   0, 1);
	guiSKI_DoneButtonImage        = UseLoadedButtonImage(guiSKI_TransactionButtonImage, 0, 1);

	//Transaction button
	//if the dealer repairs, use the repair fast help text for the transaction button
	ST::string help;
	if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
	{
		help = SkiMessageBoxText[SKI_REPAIR_TRANSACTION_BUTTON_HELP_TEXT];
	}
	else
	{
		help = SkiMessageBoxText[SKI_TRANSACTION_BUTTON_HELP_TEXT];
	}
	guiSKI_TransactionButton = MakeButton(guiSKI_TransactionButtonImage, SKI_Text[SKI_TEXT_TRANSACTION], SKI_TRANSACTION_BUTTON_X, MSYS_PRIORITY_HIGH, BtnSKI_TransactionButtonCallback, help);

	//Done button
	guiSKI_DoneButton = MakeButton(guiSKI_DoneButtonImage, SKI_Text[SKI_TEXT_DONE], SKI_DONE_BUTTON_X,  MSYS_PRIORITY_HIGH + 10, BtnSKI_DoneButtonCallback, SkiMessageBoxText[SKI_DONE_BUTTON_HELP_TEXT]);

	//Blanket the entire screen
	MSYS_DefineRegion(&gSKI_EntireScreenMouseRegions, 0, 0, SCREEN_WIDTH, 339, MSYS_PRIORITY_HIGH - 2, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	//Create the mouse regions for the inventory slot
	CreateSkiInventorySlotMouseRegions( );



	//Create the mouse region to limit the movement of the item cursos
	MSYS_DefineRegion( &gSkiInventoryMovementAreaMouseRegions, SKI_ITEM_MOVEMENT_AREA_X, SKI_ITEM_MOVEMENT_AREA_Y, (UINT16)(SKI_ITEM_MOVEMENT_AREA_X+SKI_ITEM_MOVEMENT_AREA_WIDTH), (UINT16)(SKI_ITEM_MOVEMENT_AREA_Y+SKI_ITEM_MOVEMENT_AREA_HEIGHT), MSYS_PRIORITY_HIGH-1,
				CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

	//Disable the region that limits the movement of the cursor with the item
	gSkiInventoryMovementAreaMouseRegions.Disable();


	//Create the mouse region for the shopkeeper's face
	MSYS_DefineRegion(&gArmsDealersFaceMouseRegions, SKI_FACE_X, SKI_FACE_Y,
				(UINT16)(SKI_FACE_X+SKI_FACE_WIDTH), (UINT16)(SKI_FACE_Y+SKI_FACE_HEIGHT),
				MSYS_PRIORITY_HIGH-1,
				CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectArmsDealersFaceRegionCallBack);

	std::fill_n(ArmsDealerOfferArea, SKI_NUM_TRADING_INV_SLOTS, INVENTORY_IN_SLOT{});
	std::fill_n(PlayersOfferArea, SKI_NUM_TRADING_INV_SLOTS, INVENTORY_IN_SLOT{});


	if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
	{
		HandlePossibleRepairDelays();
	}


	//Setup the currently selected arms dealer
	InitializeShopKeeper( TRUE );

	gMoveingItem = INVENTORY_IN_SLOT{};

	std::fill(std::begin(gfCommonQuoteUsedThisSession), std::end(gfCommonQuoteUsedThisSession), FALSE);

	//Init the shopkeepers face
	InitShopKeepersFace( SelectedArmsDealer()->profileID );

	gfDoneBusinessThisSession = FALSE;

	//Call this to set the fact that we just entered the screen
	HandleShopKeeperDialog( 1 );

	ResetAllQuoteSaidFlags();

	//Reset the highlighted item pointer;
	gpHighLightedItemObject = NULL;

	//Reset
	gfRemindedPlayerToPickUpHisStuff = FALSE;
	gfUserHasRequestedToLeave = FALSE;
	gfDisplayNoRoomMsg = FALSE;

	//Disable the map radar region
	gRadarRegion.Disable();

	gfDoEvaluationAfterOpening = FALSE;

	if( gItemToAdd.fActive )
	{
		BOOLEAN fAddedOK = FALSE;
		INT8 bSlotNum = gItemToAdd.bPreviousInvPos;

		//if this is NOT a repair dealer or he is is but there is enough space in the player's offer area
		// (you can't be out of space if it isn't a repairman, only they can fill it up with repaired items!)
		if( (!DoesDealerDoRepairs(gbSelectedArmsDealerID)) ||
			( CountNumberOfItemsInThePlayersOfferArea( ) < SKI_NUM_ARMS_DEALERS_INV_SLOTS ) )
		{
			// if we're supposed to store the original pocket #, but that pocket still holds more of these
			if ( ( bSlotNum != -1 ) && ( gpSMCurrentMerc->inv[ bSlotNum ].ubNumberOfObjects > 0 ) )
			{
				// then we can't store the pocket #, because our system can't return stacked objects
				bSlotNum = -1;
			}

			if ( OfferObjectToDealer( &(gItemToAdd.ItemObject), gpSMCurrentMerc->ubProfile, bSlotNum ) )
			{
				fAddedOK = TRUE;
			}
		}

		if ( fAddedOK )
		{
			// evaluate it
			gfDoEvaluationAfterOpening = TRUE;
		}
		else
		{
			//add the item back to the current PC into the slot it came from
			gpSMCurrentMerc->inv[gItemToAdd.bPreviousInvPos] = gItemToAdd.ItemObject;
		}

		//Clear the contents of the structure
		gItemToAdd = ITEM_TO_ADD_AFTER_SKI_OPEN{};
		gItemToAdd.fActive = FALSE;
	}

	// Dirty the bottom panel
	fInterfacePanelDirty = DIRTYLEVEL2;


	gfDealerHasSaidTheEvaluateQuoteOnceThisSession = FALSE;
	guiRandomQuoteDelayTime = SKI_DEALERS_RANDOM_QUOTE_DELAY;

	pShopKeeperItemDescObject = NULL;


	//Region to allow the user to drop items to the ground
	MSYS_DefineRegion(&gArmsDealersDropItemToGroundMouseRegions, SKI_DROP_ITEM_TO_GROUND_START_X,
				SKI_DROP_ITEM_TO_GROUND_START_Y, SCREEN_WIDTH, 339, MSYS_PRIORITY_HIGH,
				CURSOR_NORMAL, SelectArmsDealersDropItemToGroundMovementRegionCallBack,
				SelectArmsDealersDropItemToGroundRegionCallBack);
	//			CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectArmsDealersDropItemToGroundRegionCallBack );

	gfSkiDisplayDropItemToGroundText = FALSE;

	// by default re-enable calls to PerformTransaction()
	gfPerformTransactionInProgress = FALSE;
}


static BOOLEAN InitShopKeepersFace(UINT8 ubMercID)
{
	SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(SelectedArmsDealer()->profileID);
	FACETYPE& f = InitFace(ubMercID, pSoldier, FACE_BIGFACE);
	giShopKeeperFaceIndex = &f;

	SetAutoFaceActive(FRAME_BUFFER, FACE_AUTO_RESTORE_BUFFER, f, SKI_FACE_X, SKI_FACE_Y);

	//Set it so the face cannot be set InActive
	f.uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;

	RenderAutoFace(f);

	return(TRUE);
}


static void DestroySkiInventorySlotMouseRegions(void);
static void RemoveShopKeeperSubTitledText(void);
static void ShutUpShopKeeper(void);


static void ExitShopKeeperInterface(void)
{
	UINT8 ubCnt;

	if( gfExitSKIDueToMessageBox )
	{
		gfSKIScreenExit = FALSE;

		//gfExitSKIDueToMessageBox = FALSE;
	}

	// ItemDescriptionBox should be cleared in either case
	if( InItemDescriptionBox( ) || pShopKeeperItemDescObject != NULL )
	{
		DeleteItemDescriptionBox( );
	}

	FreeMouseCursor();

	//Delete the main shopkeep background
	DeleteVideoObject(guiMainTradeScreenImage);
	DeleteVideoObject(guiItemCrossOut);
	DeleteVideoSurface(guiCornerWhereTacticalIsStillSeenImage);

	ShutUpShopKeeper();

	UnloadButtonImage( guiSKI_InvPageUpButtonImage );
	UnloadButtonImage( guiSKI_InvPageDownButtonImage );

	UnloadButtonImage( guiSKI_TransactionButtonImage );
	UnloadButtonImage( guiSKI_DoneButtonImage );

	//loop through the area and delete small faces
	for(ubCnt=0; ubCnt<gubNumberMercsInArray; ubCnt++)
	{
		DeleteVideoObject(guiSmallSoldiersFace[ubCnt]);
	}

	RemoveButton( guiSKI_InvPageUpButton );
	RemoveButton( guiSKI_InvPageDownButton );

	RemoveButton( guiSKI_TransactionButton );
	RemoveButton( guiSKI_DoneButton );

	MSYS_RemoveRegion( &gSKI_EntireScreenMouseRegions);

	MSYS_RemoveRegion( &gSkiInventoryMovementAreaMouseRegions );

	//Remove the region for the face
	MSYS_RemoveRegion( &gArmsDealersFaceMouseRegions );

	//Region to allow the user to drop items to the ground
	MSYS_RemoveRegion( &gArmsDealersDropItemToGroundMouseRegions );

	//Destroy the mouse regions for the inventory slots
	DestroySkiInventorySlotMouseRegions( );

	//if there is a temp inventory array, destroy it
	if( gpTempDealersInventory )
	{
		delete[] gpTempDealersInventory;
		gpTempDealersInventory = NULL;
	}

	//if there is a subtitles box up, remove it
	RemoveShopKeeperSubTitledText();

	//Get rid of the ShopKeeper face
	DeleteFace( giShopKeeperFaceIndex );

	gRadarRegion.Enable();

	gfSMDisableForItems = FALSE;
}

static void DisplayArmsDealerCurrentInventoryPage(void);
static void DisplayArmsDealerOfferArea(void);
static void DisplayPlayersOfferArea(void);
static void DisplayTalkingArmsDealer(void);
static void DisplayTheSkiDropItemToGroundString(void);
static void EnableDisableEvaluateAndTransactionButtons(void);
static void EvaluateItemAddedToPlayersOfferArea(INT8 bSlotID, BOOLEAN fFirstOne);
static void HandleCheckIfEnoughOnTheTable(void);

static void HandleShopKeeperInterface(void)
{
	UINT8 ubStatusOfSkiRenderDirtyFlag = gubSkiDirtyLevel;

	INT32 iCounter = 0;


	//if we are in the item desc panel, disable the buttons
	if( InItemDescriptionBox( ) && pShopKeeperItemDescObject != NULL )
	{
		DisableSMPpanelButtonsWhenInShopKeeperInterface();
		DisableButton( guiSKI_InvPageUpButton );
		DisableButton( guiSKI_InvPageDownButton );
		DisableButton( guiSKI_TransactionButton );
		DisableButton( guiSKI_DoneButton );

		//make sure the buttons dont render
		//guiSKI_InvPageUpButton->uiFlags   |= BUTTON_FORCE_UNDIRTY;
		//guiSKI_InvPageDownButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
		guiSKI_TransactionButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
		guiSKI_DoneButton->uiFlags        |= BUTTON_FORCE_UNDIRTY;

		// make sure the shop keeper doesn't start talking ( reset the timing variable )
		HandleShopKeeperDialog( 2 );

		return;
	}


	if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
	{
		fInterfacePanelDirty = DIRTYLEVEL2;
	}

	RenderTacticalInterface( );

	if ( InItemStackPopup( ) )
	{
		if ( fInterfacePanelDirty == DIRTYLEVEL2 )
		{
			RenderItemStackPopup( TRUE );
		}
		else
		{
			RenderItemStackPopup( FALSE );
		}
	}

	// handle check if enough on the table
	HandleCheckIfEnoughOnTheTable( );

	// Render view window
	fInterfacePanelDirty = DIRTYLEVEL2;
	RenderRadarScreen( );

	if( fInterfacePanelDirty == DIRTYLEVEL2 )
	{
		fInterfacePanelDirty = DIRTYLEVEL0;
	}

	RenderClock();
	RenderTownIDString( );

	DisplayTalkingArmsDealer();


	DisplayArmsDealerCurrentInventoryPage( );

	DisplayArmsDealerOfferArea();

	DisplayPlayersOfferArea();

	EnableDisableEvaluateAndTransactionButtons();

	MarkButtonsDirty( );

	if( gfDoEvaluationAfterOpening )
	{
		BOOLEAN fFirstOne = TRUE;

		gfDoEvaluationAfterOpening = FALSE;

		for( iCounter = 0; iCounter < SKI_NUM_TRADING_INV_SLOTS; iCounter++ )
		{
			if( PlayersOfferArea[ iCounter ].fActive )
			{
				EvaluateItemAddedToPlayersOfferArea( ( INT8 ) iCounter, fFirstOne );
				fFirstOne = FALSE;
			}
		}

		gfAlreadySaidTooMuchToRepair = FALSE;
	}

	//if the Ski dirty flag was changed to a lower value, make sure it is set properly
	if( ubStatusOfSkiRenderDirtyFlag > gubSkiDirtyLevel )
		gubSkiDirtyLevel = ubStatusOfSkiRenderDirtyFlag;

	//if the merc is talking and there is an item currently being highlighted
	// ( this gets rid of the item burning through the dealers text box )
	if (gfIsTheShopKeeperTalking &&
			(gpHighLightedItemObject != NULL || gubSkiDirtyLevel != SKI_DIRTY_LEVEL0))
	{
		RenderMercPopUpBox(g_popup_box, gusPositionOfSubTitlesX, SKI_POSITION_SUBTITLES_Y, FRAME_BUFFER);
	}

	//if we are to display the drop item to ground text
	if( gfSkiDisplayDropItemToGroundText )
	{
		DisplayTheSkiDropItemToGroundString();
	}
}


static void CrossOutUnwantedItems(void);
static void RestoreTacticalBackGround(void);


static void RenderShopKeeperInterface(void)
{
	if (InItemDescriptionBox() && pShopKeeperItemDescObject != NULL) return;

	//RenderTacticalInterface( );
	// Render view window
	//RenderRadarScreen( );

	BltVideoObject(FRAME_BUFFER, guiMainTradeScreenImage, 0, SKI_MAIN_BACKGROUND_X, SKI_MAIN_BACKGROUND_Y);

	//Display the Title
	DrawTextToScreen(SKI_Text[SKI_TEXT_MERCHADISE_IN_STOCK], SKI_MAIN_TITLE_X, SKI_MAIN_TITLE_Y, SKI_MAIN_TITLE_WIDTH, SKI_TITLE_FONT, SKI_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//if the dealer repairs
	if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
	{
		//Display the Repair cost text
		DisplayWrappedString(SKI_TOTAL_COST_X, SKI_TOTAL_COST_Y, SKI_TOTAL_COST_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[SKI_TEXT_REPAIR_COST], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}
	else
	{
		//Display the Total cost text
		DisplayWrappedString(SKI_TOTAL_COST_X, SKI_TOTAL_COST_Y, SKI_TOTAL_COST_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[SKI_TEXT_TOTAL_COST], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}

	//Display the total value text
	DisplayWrappedString(SKI_TOTAL_VALUE_X, SKI_TOTAL_VALUE_Y, SKI_TOTAL_VALUE_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[SKI_TEXT_TOTAL_VALUE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Display the players current balance text
	DisplayWrappedString(SKI_PLAYERS_CURRENT_BALANCE_X, SKI_PLAYERS_CURRENT_BALANCE_Y, SKI_PLAYERS_CURRENT_BALANCE_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SkiMessageBoxText[SKI_PLAYERS_CURRENT_BALANCE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display the players current balance value
	DrawTextToScreen(SPrintMoney(LaptopSaveInfo.iCurrentBalance), SKI_PLAYERS_CURRENT_BALANCE_X, SKI_PLAYERS_CURRENT_BALANCE_OFFSET_TO_VALUE, SKI_PLAYERS_CURRENT_BALANCE_WIDTH, FONT10ARIAL, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED | MARK_DIRTY);

	BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, 0, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_HEIGHT);

	//At this point the background is pure, copy it to the save buffer
	if( gfRenderScreenOnNextLoop )
	{
	//	BlitBufferToBuffer(FRAME_BUFFER, guiCornerWhereTacticalIsStillSeenImage, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT);
		SGPBox const SrcRect =
		{
			SKI_TACTICAL_BACKGROUND_START_X,
			SKI_TACTICAL_BACKGROUND_START_Y,
			SKI_TACTICAL_BACKGROUND_START_WIDTH,
			SKI_TACTICAL_BACKGROUND_START_HEIGHT
		};
		BltVideoSurface(guiCornerWhereTacticalIsStillSeenImage, guiSAVEBUFFER, 0, 0, &SrcRect);

		gfRenderScreenOnNextLoop = FALSE;
	}

	DisplayArmsDealerCurrentInventoryPage( );

	DisplayArmsDealerOfferArea();

	DisplayPlayersOfferArea();

	//if the merc is talking and the screen has been dirtied
	if( gfIsTheShopKeeperTalking )
		RenderMercPopUpBox(g_popup_box, gusPositionOfSubTitlesX, SKI_POSITION_SUBTITLES_Y, FRAME_BUFFER);

	CrossOutUnwantedItems( );

	RenderClock();
	RenderTownIDString( );

	//RenderTacticalInterface( );

	//Restore the tactical background that is visble behind the SKI panel
	RestoreTacticalBackGround();

	InvalidateScreen();
}


static void RestoreTacticalBackGround(void)
{
	//Restore the background before blitting the text back on
	//RestoreExternBackgroundRect( SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT );

	//BlitBufferToBuffer(guiCornerWhereTacticalIsStillSeenImage, FRAME_BUFFER, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT);

	BltVideoSurface(FRAME_BUFFER, guiCornerWhereTacticalIsStillSeenImage, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, NULL);

	InvalidateScreen();
}


static void ExitSKIRequested(void);


static void GetShopKeeperInterfaceUserInput(void)
{
	InputAtom Event;

	while( DequeueEvent( &Event ) )
	{
		if( !HandleTextInput( &Event ) && Event.usEvent == KEY_DOWN )
		{
			switch( Event.usParam )
			{
				case SDLK_ESCAPE:
					// clean exits - does quotes, shuts up shopkeeper, etc.
					ExitSKIRequested();
					break;

				case 'x':
					if( Event.usKeyState & ALT_DOWN )
					{
						HandleShortCutExitState( );
					}
					break;

				case SDLK_SPACE:
					{
						DeleteItemDescriptionBox( );

						// skip Robot and EPCs
						SOLDIERTYPE* const s = FindNextActiveAndAliveMerc(gpSMCurrentMerc, FALSE, TRUE);
						gSelectSMPanelToMerc = s;
						LocateSoldier(s, DONTSETLOCATOR);
						// refresh background for player slots (in case item values change due to Flo's discount)
						gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
					}
					break;
			}
		}
	}
}

static void EnableDisableDealersInventoryPageButtons(void);

static void ShopInventoryPageUp()
{
	if (gSelectArmsDealerInfo.ubCurrentPage > 1)
	{
		--gSelectArmsDealerInfo.ubCurrentPage;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		EnableDisableDealersInventoryPageButtons();
	}
}


static void ShopInventoryPageDown()
{
	if (gSelectArmsDealerInfo.ubCurrentPage < gSelectArmsDealerInfo.ubNumberOfPages)
	{
		++gSelectArmsDealerInfo.ubCurrentPage;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		EnableDisableDealersInventoryPageButtons();
	}
}


static void BtnSKI_InvPageUpButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ShopInventoryPageUp();
	}
}


static void BtnSKI_InvPageDownButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ShopInventoryPageDown();
	}
}


static void PerformTransaction(UINT32 uiMoneyFromPlayersAccount);


static void BtnSKI_TransactionButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// if not already doing it (multiple clicks can be queued up while shopkeeper is still talking)
		if ( !gfPerformTransactionInProgress )
		{
			// shut him up
			ShutUpShopKeeper();

			giShopKeepDialogueEventinProgress = -1;

			PerformTransaction( 0 );
		}
	}
}


static void BtnSKI_DoneButtonCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		ExitSKIRequested();
	}
}


static void DealerInventoryScrollRegionCallback(MOUSE_REGION* const, INT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		ShopInventoryPageUp();
	}
	else if (reason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		ShopInventoryPageDown();
	}
}


static void SelectDealersInventoryMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectDealersInventoryRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectDealersOfferSlotsMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectDealersOfferSlotsRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectPlayersOfferSlotsMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectPlayersOfferSlotsRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateSkiInventorySlotMouseRegions(void)
{
	MSYS_DefineRegion(&g_dealer_inventory_scroll_region, 161, 27, 532, 137, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, NULL, DealerInventoryScrollRegionCallback);

	bool const does_repairs = DoesDealerDoRepairs(gbSelectedArmsDealerID);

	// Create the mouse regions for the shopkeeper's inventory
	for (UINT32 i = 0; i != SKI_NUM_ARMS_DEALERS_INV_SLOTS; ++i)
	{
		UINT16 const x = SKI_ARMS_DEALERS_INV_START_X + i % SKI_NUM_ARMS_DEALERS_INV_COLS * SKI_INV_OFFSET_X;
		UINT16 const y = SKI_ARMS_DEALERS_INV_START_Y + i / SKI_NUM_ARMS_DEALERS_INV_COLS * SKI_INV_OFFSET_Y;
		{
			MOUSE_REGION* const r = &gDealersInventoryMouseRegions[i];
			MSYS_DefineRegion(r, x, y, x + SKI_INV_SLOT_WIDTH, y + SKI_INV_SLOT_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, SelectDealersInventoryMovementRegionCallBack, SelectDealersInventoryRegionCallBack);
			MSYS_SetRegionUserData(r, 0, i);
		}
		if (does_repairs)
		{
			// Small Faces
			MOUSE_REGION* const r = &gRepairmanInventorySmallFaceMouseRegions[i];
			MSYS_DefineRegion(r, x + SKI_SMALL_FACE_OFFSET_X, y, x + SKI_SMALL_FACE_OFFSET_X + SKI_SMALL_FACE_WIDTH, y + SKI_SMALL_FACE_HEIGHT, MSYS_PRIORITY_HIGH + 1, CURSOR_NORMAL, NULL, NULL);
			MSYS_SetRegionUserData(r, 0, i);
		}
	}

	// Create the mouse regions for the shopkeeper's trading slots
	for (UINT32 i = 0; i != SKI_NUM_TRADING_INV_SLOTS; ++i)
	{
		UINT16 const x = SKI_ARMS_DEALERS_TRADING_INV_X + i % SKI_NUM_TRADING_INV_COLS * SKI_INV_OFFSET_X;
		UINT16 const y = SKI_ARMS_DEALERS_TRADING_INV_Y + i / SKI_NUM_TRADING_INV_COLS * SKI_INV_OFFSET_Y;
		{
			MOUSE_REGION* const r = &gDealersOfferSlotsMouseRegions[i];
			MSYS_DefineRegion(r, x, y, x + SKI_INV_SLOT_WIDTH, y + SKI_INV_SLOT_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, SelectDealersOfferSlotsMovementRegionCallBack, SelectDealersOfferSlotsRegionCallBack);
			MSYS_SetRegionUserData(r, 0, i);
		}
		if (does_repairs)
		{
			// Small Faces
			MOUSE_REGION* const r = &gDealersOfferSlotsSmallFaceMouseRegions[i];
			MSYS_DefineRegion(r, x + SKI_SMALL_FACE_OFFSET_X, y, x + SKI_SMALL_FACE_OFFSET_X + SKI_SMALL_FACE_WIDTH, y + SKI_SMALL_FACE_HEIGHT, MSYS_PRIORITY_HIGH + 1, CURSOR_NORMAL, SelectDealersOfferSlotsMovementRegionCallBack, SelectDealersOfferSlotsRegionCallBack);
			MSYS_SetRegionUserData(r, 0, i);
		}
	}

	// Create the mouse regions for the Players trading slots
	for (UINT32 i = 0; i != SKI_NUM_TRADING_INV_SLOTS; ++i)
	{
		UINT16 const x = SKI_PLAYERS_TRADING_INV_X + i % SKI_NUM_TRADING_INV_COLS * SKI_INV_OFFSET_X;
		UINT16 const y = SKI_PLAYERS_TRADING_INV_Y + i / SKI_NUM_TRADING_INV_COLS * SKI_INV_OFFSET_Y;
		{
			// Trading Slots
			MOUSE_REGION* const r = &gPlayersOfferSlotsMouseRegions[i];
			MSYS_DefineRegion(r, x, y, x + SKI_INV_SLOT_WIDTH, y + SKI_INV_SLOT_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, SelectPlayersOfferSlotsMovementRegionCallBack, SelectPlayersOfferSlotsRegionCallBack);
			MSYS_SetRegionUserData(r, 0, i);
		}
		{
			// Small Faces
			MOUSE_REGION* const r = &gPlayersOfferSlotsSmallFaceMouseRegions[i];
			MSYS_DefineRegion(r, x + SKI_SMALL_FACE_OFFSET_X, y, x + SKI_SMALL_FACE_OFFSET_X + SKI_SMALL_FACE_WIDTH, y + SKI_SMALL_FACE_HEIGHT, MSYS_PRIORITY_HIGH + 1, CURSOR_NORMAL, SelectPlayersOfferSlotsMovementRegionCallBack, SelectPlayersOfferSlotsRegionCallBack);
			MSYS_SetRegionUserData(r, 0, i);
		}
	}
}


static void DestroySkiInventorySlotMouseRegions(void)
{
	MSYS_RemoveRegion(&g_dealer_inventory_scroll_region);

	bool const does_repairs = DoesDealerDoRepairs(gbSelectedArmsDealerID);

	for (UINT32 i = 0; i != SKI_NUM_ARMS_DEALERS_INV_SLOTS; ++i)
	{
		MSYS_RemoveRegion(&gDealersInventoryMouseRegions[i]);
		if (does_repairs) MSYS_RemoveRegion(&gRepairmanInventorySmallFaceMouseRegions[i]);
	}

	for (UINT32 i = 0; i != SKI_NUM_TRADING_INV_SLOTS; ++i)
	{
		MSYS_RemoveRegion(&gDealersOfferSlotsMouseRegions[i]);
		if (does_repairs) MSYS_RemoveRegion(&gDealersOfferSlotsSmallFaceMouseRegions[i]);

		MSYS_RemoveRegion(&gPlayersOfferSlotsMouseRegions[i]);
		MSYS_RemoveRegion(&gPlayersOfferSlotsSmallFaceMouseRegions[i]);
	}
}


static INT8 AddItemToArmsDealerOfferArea(const INVENTORY_IN_SLOT* pInvSlot, INT8 bSlotIdInOtherLocation);
static void InitShopKeeperItemDescBox(OBJECTTYPE* pObject, UINT8 ubPocket, UINT8 ubFromLocation);


//Mouse Call back for the Arms traders inventory slot
static void SelectDealersInventoryRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8 ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		INT8  ubLocation;

		if( gpTempDealersInventory == NULL )
			return;

		ubSelectedInvSlot += gSelectArmsDealerInfo.ubFirstItemIndexOnPage;

		//if the selected slot is above any inventory we have, return
		if( ubSelectedInvSlot >= gSelectArmsDealerInfo.uiNumDistinctInventoryItems )
			return;

		//if there are any items still there
		if( gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects > 0 )
		{
			//If the item type has not already been placed
			if( !( gpTempDealersInventory[ ubSelectedInvSlot ].uiFlags & ARMS_INV_ITEM_SELECTED ) )
			{
				//if the dealer repairs
				if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
				{
					// ignore left clicks on items under repair.  Fully repaired items are moved out to player's slots automatically
				}
				else // not a repairman
				{
					ubLocation = AddItemToArmsDealerOfferArea( &gpTempDealersInventory[ ubSelectedInvSlot ], ubSelectedInvSlot );

					//if the item was added to the Dealer Offer Area correctly
					if( ubLocation != -1 )
					{
						//Set the flag indicating the item has been selected
						gpTempDealersInventory[ ubSelectedInvSlot ].uiFlags |= ARMS_INV_ITEM_SELECTED;

						//Specify the location the items went to
						gpTempDealersInventory[ ubSelectedInvSlot ].ubLocationOfObject = ARMS_DEALER_OFFER_AREA;
						gpTempDealersInventory[ ubSelectedInvSlot ].bSlotIdInOtherLocation = ubLocation;

						//if the shift key is being pressed, remove them all
						if (_KeyDown(SHIFT))
						{
							gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects = 0;
						}
						else
						{
							gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects --;
						}

						gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
					}
				}
			}
			else // some of this item are already in dealer's offer area
			{
				UINT8 ubNumToMove;

				//if the shift key is being pressed, remove them all
				if (_KeyDown(SHIFT))
				{
					ubNumToMove = gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects;
				}
				else
				{
					ubNumToMove = 1;
				}

				//Reduce the number in dealer's inventory
				gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects -= ubNumToMove;
				//Increase the number in dealer's offer area
				ArmsDealerOfferArea[ gpTempDealersInventory[ ubSelectedInvSlot ].bSlotIdInOtherLocation ].ItemObject.ubNumberOfObjects += ubNumToMove;

				gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			}
		}
	}

	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		UINT8 ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

		if( gpTempDealersInventory == NULL )
			return;

		ubSelectedInvSlot += gSelectArmsDealerInfo.ubFirstItemIndexOnPage;

		//if the selected slot is above any inventory we have, return
		if( ubSelectedInvSlot >= gSelectArmsDealerInfo.uiNumDistinctInventoryItems )
			return;

		//DEF:
		//bring up the item description box
		if ( !InItemDescriptionBox( ) )
		{
			InitShopKeeperItemDescBox( &gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject, ubSelectedInvSlot, ARMS_DEALER_INVENTORY );
		}
		else
		{
			DeleteItemDescriptionBox( );
		}



		/*
		//if the item has been seleceted
		if( gpTempDealersInventory[ ubSelectedInvSlot ].uiFlags & ARMS_INV_ITEM_SELECTED )
		{
			//Check to see it there is more then 1 item in the location
			INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[gpTempDealersInventory[ubSelectedInvSlot].bSlotIdInOtherLocation];
			if (a->ItemObject.ubNumberOfObjects > 0)
			{
				//Increase the number in the dealer inventory
				gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects ++;

				//Decrease the number in the dealer offer area
				a->ItemObject.ubNumberOfObjects--;

				//if there is nothing left in the arms dealer offer area
				if (a->ItemObject.ubNumberOfObjects == 0)
				{
					RemoveItemFromArmsDealerOfferArea( gpTempDealersInventory[ ubSelectedInvSlot ].bSlotIdInOtherLocation, FALSE );
				}

				//redraw the screen
				gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			}
		}*/
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		ShopInventoryPageUp();
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		ShopInventoryPageDown();
	}
}


static void SelectDealersInventoryMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	ubSelectedInvSlot += gSelectArmsDealerInfo.ubFirstItemIndexOnPage;

	if( gpTempDealersInventory == NULL )
		return;

	if( ubSelectedInvSlot >= gSelectArmsDealerInfo.uiNumDistinctInventoryItems )
		return;

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		//if there is nothing in the slot, exit
		if (!gpTempDealersInventory[ubSelectedInvSlot].fActive)
			return;

		gpHighLightedItemObject = &gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject;

		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, TRUE );
	}
	else if(iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//if there is nothing in the slot, exit
		if (!gpTempDealersInventory[ubSelectedInvSlot].fActive)
			return;

		gpHighLightedItemObject = NULL;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, FALSE );
	}
}


static void SelectDealersOfferSlotsMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	const INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[ubSelectedInvSlot];

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		//if there is nothing in the slot, exit
		if (!a->fActive) return;

		gpHighLightedItemObject = &a->ItemObject;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, TRUE );
	}
	else if(iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//if there is nothing in the slot, exit
		if (!a->fActive) return;

		gpHighLightedItemObject = NULL;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, FALSE );
	}
}


static void SelectPlayersOfferSlotsMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	const INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubSelectedInvSlot];

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		//if there is nothing in the slot, exit
		if (!o->fActive) return;

		gpHighLightedItemObject = &o->ItemObject;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, TRUE );
	}
	else if(iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//if there is nothing in the slot, exit
		if (!o->fActive) return;

		gpHighLightedItemObject = NULL;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, FALSE );
	}
}


static INT8 AddInventoryToSkiLocation(const INVENTORY_IN_SLOT* pInv, UINT8 ubSpotLocation, UINT8 ubWhere);
static void IfMercOwnedCopyItemToMercInv(const INVENTORY_IN_SLOT* pInv);
static void IfMercOwnedRemoveItemFromMercInv(const INVENTORY_IN_SLOT* pInv);
static BOOLEAN RemoveItemFromArmsDealerOfferArea(INT8 bSlotId, BOOLEAN fKeepItem);
static BOOLEAN RemoveRepairItemFromDealersOfferArea(INT8 bSlot);


//Mouse Call back for the dealer's OFFER slot
static void SelectDealersOfferSlotsRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[ubSelectedInvSlot];

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		//if there is something here
		if (a->fActive)
		{
			//if the dealer repairs
			if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
			{
				// return item to player
				RemoveRepairItemFromDealersOfferArea( ubSelectedInvSlot );
			}
			else
			{
				//bring up the item description box
				if ( !InItemDescriptionBox( ) )
				{
					InitShopKeeperItemDescBox(&a->ItemObject, ubSelectedInvSlot, ARMS_DEALER_OFFER_AREA);
				}
				else
				{
					DeleteItemDescriptionBox( );
				}
			}
		}
	}
	else if( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP ) //MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		/*
		//if the current merc is disabled for whatever reason
		if( gfSMDisableForItems )
			// the
			return;*/
		//if there is something here
		if (a->fActive)
		{
			//if this is a repair dealer
			if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
			{
				//if we don't have an item, pick one up
				if( gMoveingItem.sItemIndex == 0 )
				{
					//if the dealer is a repair dealer, allow the player to pick up the item
					if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
					{
						BeginSkiItemPointer( ARMS_DEALER_OFFER_AREA, ubSelectedInvSlot, FALSE );
					}
				}
				else
				{
					//swap what is in the cursor with what is in the player offer slot

					// if the slot is overloaded (holds more objects than we have room for valid statuses of)
					if (a->ItemObject.ubNumberOfObjects > MAX_OBJECTS_PER_SLOT)
					{
						// then ignore the click - we can't do the swap, or anything very useful, because we can't allow overloaded
						// items into the player's cursor - if he put them into a merc's inventory, the extra statuses are missing!
						// At best, we could do some sort of message here.
						return;
					}

					IfMercOwnedCopyItemToMercInv( &gMoveingItem );

					const INVENTORY_IN_SLOT TempSlot = *a;
					*a = gMoveingItem;
					gMoveingItem = TempSlot;

					IfMercOwnedRemoveItemFromMercInv( &gMoveingItem );

					// Change mouse cursor
					gpItemPointer = &gMoveingItem.ItemObject;
					SetSkiCursor( EXTERN_CURSOR );

					gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
				}
			}
			else //non-repairman
			{
				//if the shift key is being pressed, remove them all
				if (_KeyDown(SHIFT) || a->ItemObject.ubNumberOfObjects == 1)
				{
					RemoveItemFromArmsDealerOfferArea(ubSelectedInvSlot, TRUE);//a->bSlotIdInOtherLocation
				}
				else	// multiple items there, SHIFT isn't being pressed
				{
					// remove only one at a time
					a->ItemObject.ubNumberOfObjects--;
					gpTempDealersInventory[a->bSlotIdInOtherLocation].ItemObject.ubNumberOfObjects++;
				}

				gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			}
		}
		else // empty slot
		{
			//if the cursor has something in it
			if( gMoveingItem.sItemIndex > 0 )
			{
				// we'd better talking to a repairman, cursor is locked out of this area while full for non-repairmen!
				Assert(DoesDealerDoRepairs(gbSelectedArmsDealerID));

				//Drop the item into the current slot
				AddInventoryToSkiLocation( &gMoveingItem, ubSelectedInvSlot, ARMS_DEALER_OFFER_AREA );

				//Reset the cursor
				SetSkiCursor( CURSOR_NORMAL );

				//refresh the screen
				gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			}
		}
	}
}


static BOOLEAN RemoveItemFromPlayersOfferArea(INT8 bSlot);


//Mouse Call back for the Players OFFER slot
static void SelectPlayersOfferSlotsRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8 ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubSelectedInvSlot];
	INT8 bAddedToSlotID = -1;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//if the cursor has no item in it
		if( gMoveingItem.sItemIndex == 0 )
		{
			//if there is nothing here, return
			if (!o->fActive)
				return;

			// pick it up into the cursor
			BeginSkiItemPointer( PLAYERS_OFFER_AREA, ubSelectedInvSlot, FALSE );
		}
		else	// we have something in the cursor
		{
			//Drop the item into the current slot
			//if there is something already there
			if (o->fActive)
			{
				//swap what is in the cursor with what is in the player offer slot

				// if the slot is overloaded (holds more objects than we have room for valid statuses of)
				if (o->ItemObject.ubNumberOfObjects > MAX_OBJECTS_PER_SLOT)
				{
					// then ignore the click - we can't do the swap, or anything very useful, because we can't allow overloaded
					// items into the player's cursor - if he put them into a merc's inventory, the extra statuses are missing!
					// At best, we could do some sort of message here.
					return;
				}

				IfMercOwnedCopyItemToMercInv( &gMoveingItem );

				const INVENTORY_IN_SLOT TempSlot = *o;
				*o = gMoveingItem;
				gMoveingItem = TempSlot;

				IfMercOwnedRemoveItemFromMercInv( &gMoveingItem );

				// Change mouse cursor
				gpItemPointer = &gMoveingItem.ItemObject;
				SetSkiCursor( EXTERN_CURSOR );

				//if the item we are adding is money
				if (GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY)
				{
					//Since money is always evaluated
					o->uiFlags     |= ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
					o->uiItemPrice  = o->ItemObject.bMoneyStatus;
				}
			}
			else	// slot is empty
			{
				// if the item has already been evaluated, or has just been purchased
				if ( ( gMoveingItem.uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED ) ||
					( gMoveingItem.uiFlags & ARMS_INV_JUST_PURCHASED ) )
				{
					//place the item that is in the player's hand into this player offer area slot
					bAddedToSlotID = AddInventoryToSkiLocation( &gMoveingItem, ubSelectedInvSlot, PLAYERS_OFFER_AREA );
					Assert ( bAddedToSlotID != -1 );
				}
				else
				{
					// this splits complex items for repairs.  Also puts things into the first free POA slot
					OfferObjectToDealer( &gMoveingItem.ItemObject, gMoveingItem.ubIdOfMercWhoOwnsTheItem, gMoveingItem.bSlotIdInOtherLocation );
				}

				//Reset the cursor
				SetSkiCursor( CURSOR_NORMAL );
			}

			//Dirty the interface
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		}
	}
	else if(iReason & MSYS_CALLBACK_REASON_RBUTTON_UP ) //MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//if the box is active
		if (o->fActive)
		{
			RemoveItemFromPlayersOfferArea( ubSelectedInvSlot );
			/*
			item description
			else
			{
				if ( !InItemDescriptionBox( ) )
				{
					InitItemDescriptionBox(gpSMCurrentMerc, o->bSlotIdInOtherLocation, 214, 1 + INV_INTERFACE_START_Y, 0);
				}
				else
				{
					DeleteItemDescriptionBox( );
				}
			}*/
		}
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
	}
}


void EnterShopKeeperInterfaceScreen( UINT8 ubArmsDealer )
{
	//Get Dealer ID from from merc Id
	gbSelectedArmsDealerID = GetArmsDealerIDFromMercID( ubArmsDealer );

	if( gbSelectedArmsDealerID == -1 )
	{
		SLOGW("Failed to find Arms Dealer ID From Merc ID #%d", ubArmsDealer );
		gfSKIScreenExit = TRUE;
	}

	LeaveTacticalScreen( SHOPKEEPER_SCREEN );
}


static void DetermineArmsDealersSellingInventory(void);


static void InitializeShopKeeper(BOOLEAN fResetPage)
{
	// update time player last dealt with him
	gArmsDealerStatus[ gbSelectedArmsDealerID ].uiTimePlayerLastInSKI = GetWorldTotalMin();


	//Get the number of distinct items in the inventory
	gSelectArmsDealerInfo.uiNumDistinctInventoryItems = CountDistinctItemsInArmsDealersInventory( gbSelectedArmsDealerID );

	// If there are any items in the dealer's inventory
	if( gSelectArmsDealerInfo.uiNumDistinctInventoryItems > 0 )
	{
		//Create the shopkeeper's temp inventory
		DetermineArmsDealersSellingInventory( );

		/* No longer necessary. ARM
		//Some of the number might have changed in DetermineArmsDealersSellingInventory(), recalc the values
		gSelectArmsDealerInfo.uiNumDistinctInventoryItems = CountDistinctItemsInArmsDealersInventory( gbSelectedArmsDealerID );*/
	}

	gSelectArmsDealerInfo.ubNumberOfPages = (UINT8)( gSelectArmsDealerInfo.uiNumDistinctInventoryItems / SKI_NUM_ARMS_DEALERS_INV_SLOTS );
	if( gSelectArmsDealerInfo.uiNumDistinctInventoryItems % 15 != 0 )
		gSelectArmsDealerInfo.ubNumberOfPages += 1;


	//Should we reset the current inventory page being displayed
	if( gSelectArmsDealerInfo.uiNumDistinctInventoryItems == 0 )
		gSelectArmsDealerInfo.ubCurrentPage = 0;
	else
	{
		if( fResetPage )
		{
			if( gSelectArmsDealerInfo.uiNumDistinctInventoryItems == 0 )
				gSelectArmsDealerInfo.ubCurrentPage = 0;
			else
				gSelectArmsDealerInfo.ubCurrentPage = 1;
		}

		//or if the current page will be an invalid page ( before the first, and after the last )
		#if 0 /* XXX unsigned */
		else if( gSelectArmsDealerInfo.ubCurrentPage < 0 || gSelectArmsDealerInfo.ubCurrentPage > gSelectArmsDealerInfo.ubNumberOfPages )
		#else
		else if (gSelectArmsDealerInfo.ubCurrentPage > gSelectArmsDealerInfo.ubNumberOfPages)
		#endif
		{
			gSelectArmsDealerInfo.ubCurrentPage = 1;
		}

		else if( gSelectArmsDealerInfo.uiNumDistinctInventoryItems != 0 )
			gSelectArmsDealerInfo.ubCurrentPage = 1;
	}


	//if there is no inventory
	if( gSelectArmsDealerInfo.uiNumDistinctInventoryItems == 0 )
	{
		gSelectArmsDealerInfo.ubCurrentPage = 0;
		gSelectArmsDealerInfo.ubNumberOfPages = 0;

		//disable the page up/down buttons
		DisableButton( guiSKI_InvPageUpButton );
		DisableButton( guiSKI_InvPageDownButton );

		return;
	}
	else
	{
		//enable the page up/down buttons
		EnableButton( guiSKI_InvPageUpButton );
		EnableButton( guiSKI_InvPageDownButton );
	}


	EnableDisableDealersInventoryPageButtons();
}


static void CalculateFirstItemIndexOnPage(void)
{
	gSelectArmsDealerInfo.ubFirstItemIndexOnPage = ( gSelectArmsDealerInfo.ubCurrentPage -1 ) * SKI_NUM_ARMS_DEALERS_INV_SLOTS;
}


static UINT32 DisplayInvSlot(UINT8 slot_num, UINT16 usItemIndex, UINT16 x, UINT16 y, OBJECTTYPE const& pItemObject, bool hatched_out, UINT8 ubItemArea);
static void HatchOutInvSlot(UINT16 usPosX, UINT16 usPosY);
static void SetSkiFaceRegionHelpText(const INVENTORY_IN_SLOT* pInv, MOUSE_REGION* pRegion, UINT8 ubScreenArea);
static void SetSkiRegionHelpText(const INVENTORY_IN_SLOT* pInv, MOUSE_REGION* pRegion, UINT8 ubScreenArea);


static void DisplayArmsDealerCurrentInventoryPage(void)
{
	ST::string zTemp;
	UINT16  uiFontHeight;
	UINT16  usCnt=0;
	UINT16  usPosX, usPosY;
	UINT8   sItemCount=0;
	BOOLEAN fDisplayHatchOnItem=FALSE;


	usPosX = SKI_ARMS_DEALERS_INV_START_X;
	usPosY = SKI_ARMS_DEALERS_INV_START_Y;

	//if there is any inventory
	if( gpTempDealersInventory != NULL )
	{
		if( gubSkiDirtyLevel != SKI_DIRTY_LEVEL0 )
		{
			//Calculate the item that is at the top of the list
			CalculateFirstItemIndexOnPage( );

			//Restore the background before blitting the text back on
			RestoreExternBackgroundRect( SKI_PAGE_X, SKI_PAGE_Y, SKI_PAGE_WIDTH, SKI_PAGE_HEIGHT );

			//Restore the pristine region
			RestoreExternBackgroundRect( SKI_ARMS_DEALERS_INV_START_X, SKI_ARMS_DEALERS_INV_START_Y, 370, 107 );

			//Display the current inventory page
			DrawTextToScreen(SKI_Text[SKI_TEXT_PAGE], SKI_PAGE_X, SKI_PAGE_Y + 3, SKI_PAGE_WIDTH, SKI_LABEL_FONT, SKI_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

			//Display the Current Page number
			uiFontHeight = GetFontHeight( SKI_LABEL_FONT );
			zTemp = ST::format("{}/{}", gSelectArmsDealerInfo.ubCurrentPage, gSelectArmsDealerInfo.ubNumberOfPages);
			DrawTextToScreen(zTemp, SKI_PAGE_X, SKI_PAGE_Y + uiFontHeight + 6, SKI_PAGE_WIDTH, SKI_LABEL_FONT, SKI_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
		}

		//Display all the items for the current page
		for( usCnt=gSelectArmsDealerInfo.ubFirstItemIndexOnPage; ( ( usCnt<gSelectArmsDealerInfo.uiNumDistinctInventoryItems ) && ( sItemCount < SKI_NUM_ARMS_DEALERS_INV_SLOTS ) ); usCnt++ )
		{
			//if the item is still in the inventory
			if( gpTempDealersInventory[ usCnt ].sItemIndex != 0 )
			{
				// hatch it out if region is disabled
				if ( !( gDealersInventoryMouseRegions[ sItemCount ].uiFlags & MSYS_REGION_ENABLED ) )
				{
					fDisplayHatchOnItem = TRUE;
				}
				//if the dealer repairs
				else if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
				{
					// not displaying the hatch, because it would obscure the repair ETA
					fDisplayHatchOnItem = FALSE;
				}
				else // non-repairman
				{
					// check if none left
					if ( gpTempDealersInventory[ usCnt ].ItemObject.ubNumberOfObjects == 0 )
					{
						fDisplayHatchOnItem = TRUE;
					}
					else
					{
						fDisplayHatchOnItem = FALSE;
					}
				}

				// Display the inventory slot
				INVENTORY_IN_SLOT const& inv = gpTempDealersInventory[usCnt];
				DisplayInvSlot(usCnt, inv.sItemIndex, usPosX, usPosY, inv.ItemObject, fDisplayHatchOnItem, ARMS_DEALER_INVENTORY);

				if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
				{
					SetSkiRegionHelpText( &gpTempDealersInventory[ usCnt ], &gDealersInventoryMouseRegions[sItemCount], ARMS_DEALER_INVENTORY );

					//if the dealer repairs
					if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
					{
						SetSkiFaceRegionHelpText( &gpTempDealersInventory[ usCnt ], &gRepairmanInventorySmallFaceMouseRegions[sItemCount], ARMS_DEALER_INVENTORY );
					}
				}

				sItemCount++;

				usPosX += SKI_INV_OFFSET_X;

				//if we are on to the next row
				if( !( sItemCount % SKI_NUM_ARMS_DEALERS_INV_COLS ) )
				{
					usPosX = SKI_ARMS_DEALERS_INV_START_X;
					usPosY += SKI_INV_OFFSET_Y;
				}
			}
		}
	}


	if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
	{
		//This handles the remaining (empty) slots, resetting Fast Help text, and hatching out disabled ones
		while ( sItemCount < SKI_NUM_ARMS_DEALERS_INV_SLOTS )
		{
			SetSkiRegionHelpText( NULL, &gDealersInventoryMouseRegions[ sItemCount ], ARMS_DEALER_INVENTORY );

			//if the dealer repairs
			if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
			{
				SetSkiFaceRegionHelpText( NULL, &gRepairmanInventorySmallFaceMouseRegions[ sItemCount ], ARMS_DEALER_INVENTORY );
			}

			// hatch it out if region is disabled
			if ( !( gDealersInventoryMouseRegions[ sItemCount ].uiFlags & MSYS_REGION_ENABLED ) )
			{
				HatchOutInvSlot( usPosX, usPosY );
			}

			sItemCount++;

			usPosX += SKI_INV_OFFSET_X;

			//if we are on to the next row
			if( !( sItemCount % SKI_NUM_ARMS_DEALERS_INV_COLS ) )
			{
				usPosX = SKI_ARMS_DEALERS_INV_START_X;
				usPosY += SKI_INV_OFFSET_Y;
			}
		}
	}
}


static ST::string BuildDoneWhenTimeString(ArmsDealerID, UINT16 usItemIndex, UINT8 ubElement);
static UINT32 CalcShopKeeperItemPrice(BOOLEAN fDealerSelling, BOOLEAN fUnitPriceOnly, UINT16 usItemID, FLOAT dModifier, const OBJECTTYPE* pItemObject);
static INT8 GetSlotNumberForMerc(UINT8 ubProfile);
static bool IsGunOrAmmoOfSameTypeSelected(OBJECTTYPE const&);


static UINT32 DisplayInvSlot(UINT8 const slot_num, UINT16 const item_idx, UINT16 const x, UINT16 const y, OBJECTTYPE const& item_o, bool const hatched_out, UINT8 const item_area)
{
	ST::string buf;

	UINT16 outline;
	if (IsGunOrAmmoOfSameTypeSelected(item_o))
	{
		outline = Get16BPPColor(FROMRGB(255, 255, 255));
	}
	else if (gubSkiDirtyLevel != SKI_DIRTY_LEVEL0)
	{
		outline = SGP_TRANSPARENT;
	}
	else
	{
		// The item is not highlighted and we are not rerendering the screen
		return 0;
	}

	// Restore the background region
	RestoreExternBackgroundRect(x, y, SKI_INV_SLOT_WIDTH, SKI_INV_HEIGHT);

	{
		// Display the item graphic
		const ItemModel * item = GCM->getItem(item_idx);
		SGPVObject  const& item_vo = GetInterfaceGraphicForItem(item);
		ETRLEObject const& e       = item_vo.SubregionProperties(item->getGraphicNum());
		INT16              cen_x   = x + 7 + ABS(SKI_INV_WIDTH - 3 - e.usWidth)  / 2 - e.sOffsetX;
		INT16              cen_y   = y +     ABS(SKI_INV_HEIGHT    - e.usHeight) / 2 - e.sOffsetY;
		if (gamepolicy(f_draw_item_shadow))
		{
			BltVideoObjectOutlineShadow(FRAME_BUFFER, &item_vo, item->getGraphicNum(), cen_x - 2, cen_y + 2);
		}
		BltVideoObjectOutline(      FRAME_BUFFER, &item_vo, item->getGraphicNum(), cen_x,     cen_y, outline);	}

	{
		// Display the status of the item
		UINT16 const colour = Get16BPPColor(FROMRGB(140, 136, 119));
		DrawItemUIBarEx(item_o, 0, x + 2, y + 21, 20, colour, colour, FRAME_BUFFER);
	}

	// Display the item's cost
	bool      print_repaired = FALSE;
	UINT32    item_cost      = 0;
	ProfileID owner          = NO_PROFILE;
	if (item_area == PLAYERS_OFFER_AREA)
	{
		INVENTORY_IN_SLOT const& o = PlayersOfferArea[slot_num];
		if (o.uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE) item_cost = o.uiItemPrice;
		owner          = o.ubIdOfMercWhoOwnsTheItem;
		print_repaired = o.uiFlags & ARMS_INV_ITEM_REPAIRED;
	}
	else if (item_area == ARMS_DEALER_INVENTORY)
	{
		const DealerModel* dealer_info = SelectedArmsDealer();
		if (!DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			if (!hatched_out || item_o.ubNumberOfObjects != 0)
			{
				// Show the unit price, not the total value of all if stacked
				item_cost = CalcShopKeeperItemPrice(DEALER_SELLING, TRUE, item_idx, dealer_info->sellingPrice, &item_o);
			}
		}
		else
		{
			// Display the length of time needed to repair the item
			INVENTORY_IN_SLOT const& inv = gpTempDealersInventory[slot_num];
			Assert(inv.sSpecialItemElement != -1);
			buf = BuildDoneWhenTimeString(gbSelectedArmsDealerID, item_idx, inv.sSpecialItemElement);
			DrawTextToScreen(buf, x + SKI_INV_PRICE_OFFSET_X, y + SKI_INV_PRICE_OFFSET_Y, SKI_INV_SLOT_WIDTH, SKI_ITEM_DESC_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
			owner = inv.ubIdOfMercWhoOwnsTheItem;
		}
	}
	else // Dealer's offer area
	{
		const DealerModel* dealer_info = SelectedArmsDealer();
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			// The dealer repairs, there is an item here, therefore display the item's owner's face
			owner     = ArmsDealerOfferArea[slot_num].ubIdOfMercWhoOwnsTheItem;
			item_cost = CalculateObjectItemRepairCost(gbSelectedArmsDealerID, &item_o);
		}
		else
		{
			item_cost = CalcShopKeeperItemPrice(DEALER_SELLING, FALSE, item_idx, dealer_info->sellingPrice, &item_o);
		}
	}

	if (item_cost != 0)
	{
		// Display the item's price
		DrawTextToScreen(SPrintMoney(item_cost), x + SKI_INV_PRICE_OFFSET_X, y + SKI_INV_PRICE_OFFSET_Y, SKI_INV_SLOT_WIDTH, SKI_ITEM_DESC_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}

	// If the there is more then 1 or if the item is stackable and some of it has been bought and only 1 remains
	if (item_o.ubNumberOfObjects > 1 || (
		item_o.ubNumberOfObjects == 1      &&
		DealerItemIsSafeToStack(item_idx)  &&
		item_area == ARMS_DEALER_INVENTORY &&
		gpTempDealersInventory[slot_num].uiFlags & ARMS_INV_ITEM_SELECTED))
	{
		buf = ST::format("x{}", item_o.ubNumberOfObjects);
		DrawTextToScreen(buf, x + SKI_ITEM_NUMBER_TEXT_OFFSET_X, y + SKI_ITEM_NUMBER_TEXT_OFFSET_Y, SKI_ITEM_NUMBER_TEXT_WIDTH, SKIT_NUMBER_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	}

	if (owner != NO_PROFILE)
	{
		// Display the face
		INT8 const face_slot = GetSlotNumberForMerc(owner);
		if (face_slot != -1)
		{ // Still in player's employ
			BltVideoObject(FRAME_BUFFER, guiSmallSoldiersFace[face_slot], 0, x + SKI_SMALL_FACE_OFFSET_X, y);
		}
	}

	if (ItemHasAttachments(item_o))
	{
		// Display the '*' in the bottom right corner of the square
		UINT8 attachmentHintColor = GetAttachmentHintColor(&item_o);
		DrawTextToScreen("*", x + SKI_ATTACHMENT_SYMBOL_X_OFFSET, y + SKI_ATTACHMENT_SYMBOL_Y_OFFSET, 0, TINYFONT1, attachmentHintColor, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	{
		// Display 'JAMMED' or 'REPAIRED', if appropriate
		ST::string overlay_text =
			item_o.bGunAmmoStatus < 0 ? ST::string(TacticalStr[JAMMED_ITEM_STR]) :
			print_repaired            ? ST::string(SKI_Text[SKI_TEXT_REPAIRED]) :
			ST::null;
		if (!overlay_text.empty())
		{
			INT16 cen_x;
			INT16 cen_y;
			FindFontCenterCoordinates(x, y, SKI_INV_SLOT_WIDTH, SKI_INV_HEIGHT, overlay_text, TINYFONT1, &cen_x, &cen_y);
			DrawTextToScreen(overlay_text, cen_x, cen_y, SKI_INV_SLOT_WIDTH, TINYFONT1, FONT_RED, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}
	}

	if (hatched_out) HatchOutInvSlot(x, y);

	InvalidateRegion(x, y, x + SKI_INV_SLOT_WIDTH, y + SKI_INV_SLOT_HEIGHT);

	return item_cost;
}


static int RepairmanItemQsortCompare(void const* pArg1, void const* pArg2)
{
	INVENTORY_IN_SLOT const& inv_slot1 = *static_cast<INVENTORY_IN_SLOT const*>(pArg1);
	INVENTORY_IN_SLOT const& inv_slot2 = *static_cast<INVENTORY_IN_SLOT const*>(pArg2);

	Assert(inv_slot1.sSpecialItemElement != -1);
	Assert(inv_slot2.sSpecialItemElement != -1);

	DEALER_ITEM_HEADER const (& dih)[MAXITEMS] = gArmsDealersInventory[gbSelectedArmsDealerID];
	UINT32             const repair_time1      = dih[inv_slot1.sItemIndex].SpecialItem[inv_slot1.sSpecialItemElement].uiRepairDoneTime;
	UINT32             const repair_time2      = dih[inv_slot2.sItemIndex].SpecialItem[inv_slot2.sSpecialItemElement].uiRepairDoneTime;

	// lower reapir time first
	return repair_time1 < repair_time2 ? -1 :
		repair_time1 > repair_time2 ?  1 :
		0;
}


static void AddItemsToTempDealerInventory(UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner);
static BOOLEAN RepairIsDone(UINT16 usItemIndex, UINT8 ubElement);


static void DetermineArmsDealersSellingInventory(void)
{
	UINT16  usItemIndex;
	UINT8   ubElement;
	DEALER_SPECIAL_ITEM *pSpecialItem;
	BOOLEAN fAddSpecialItem;
	SPECIAL_ITEM_INFO SpclItemInfo;

	//if there is an old inventory, delete it
	if( gpTempDealersInventory )
	{
		delete[] gpTempDealersInventory;
		gpTempDealersInventory = NULL;
	}

	//allocate memory to hold the inventory in memory
	gpTempDealersInventory = new INVENTORY_IN_SLOT[gSelectArmsDealerInfo.uiNumDistinctInventoryItems]{};
	guiNextFreeInvSlot     = 0;

	//loop through the dealer's permanent inventory items, adding them all to the temp inventory list
	for( usItemIndex=1; usItemIndex<MAXITEMS; usItemIndex++)
	{
		//if the arms dealer has some of the inventory
		if( gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].ubTotalItems > 0)
		{
			// if there are any items in perfect condition
			if( gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].ubPerfectItems > 0 )
			{
				// create just ONE dealer inventory box for them all.
				// create item info describing a perfect item
				SetSpecialItemInfoToDefaults( &SpclItemInfo );
				// no special element index - it's "perfect"
				AddItemsToTempDealerInventory(usItemIndex, &SpclItemInfo, -1, gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].ubPerfectItems, NO_PROFILE );
			}

			// add all active special items
			Assert(gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].SpecialItem.size()); ubElement++)
			{
				pSpecialItem = &(gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].SpecialItem[ ubElement ]);

				if ( pSpecialItem->fActive )
				{
					fAddSpecialItem = TRUE;

					//if the item is in for repairs
					if( pSpecialItem->Info.bItemCondition < 0 )
					{
						//if the repairs are done
						if( pSpecialItem->uiRepairDoneTime <= GetWorldTotalMin() )
						{
							if (RepairIsDone( usItemIndex, ubElement ))
							{
								// don't add it here, it was put in the player's area
								fAddSpecialItem = FALSE;
							}
							else
							{
								gpTempDealersInventory[ guiNextFreeInvSlot ].uiFlags |= ARMS_INV_ITEM_REPAIRED;
							}
						}
						else
						{
							gpTempDealersInventory[ guiNextFreeInvSlot ].uiFlags |= ARMS_INV_ITEM_NOT_REPAIRED_YET;
						}
					}

					if ( fAddSpecialItem )
					{
						UINT8 ubOwner;

						if (!DoesDealerDoRepairs(gbSelectedArmsDealerID))
						{
							// no merc is the owner
							ubOwner = NO_PROFILE;
						}
						else
						{
							// retain owner so we can display this
							ubOwner = pSpecialItem->ubOwnerProfileId;
						}

						AddItemsToTempDealerInventory( usItemIndex, &(pSpecialItem->Info), ubElement, 1, ubOwner );
					}
				}
			}
		}
	}

	// if more than one item is in inventory
	if ( guiNextFreeInvSlot > 1 )
	{
		// repairmen sort differently from merchants
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			// sort this list by object category, and by ascending price within each category
			qsort( gpTempDealersInventory, guiNextFreeInvSlot, sizeof( INVENTORY_IN_SLOT ), RepairmanItemQsortCompare );
		}
		else
		{
			// sort this list by object category, and by ascending price within each category
			qsort( gpTempDealersInventory, guiNextFreeInvSlot, sizeof( INVENTORY_IN_SLOT ), ArmsDealerItemQsortCompare );
		}
	}
}


static void StoreObjectsInNextFreeDealerInvSlot(UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner);


static void AddItemsToTempDealerInventory(UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner)
{
	UINT8 ubCnt;


	Assert( ubHowMany > 0 );
	Assert( pSpclItemInfo != NULL );


	// if there's just one of them
	if ( ubHowMany == 1 )
	{
		// it gets its own box, obviously
		StoreObjectsInNextFreeDealerInvSlot( usItemIndex, pSpclItemInfo, sSpecialItemElement, 1, ubOwner );
	}
	else // more than one
	{
		// if the items can be stacked
		// NOTE: This test must match the one inside CountDistinctItemsInArmsDealersInventory() exactly!
		if ( DealerItemIsSafeToStack( usItemIndex ) )
		{
			// then we can store them all together in the same box safely, even if there's more than MAX_OBJECTS_PER_SLOT
			StoreObjectsInNextFreeDealerInvSlot( usItemIndex, pSpclItemInfo, sSpecialItemElement, ubHowMany, ubOwner );
		}
		else
		{
			// non-stacking items must be stored in one / box , because each may have unique fields besides bStatus[]
			// Example: guns all have ammo, ammo type, etc.  We need these uniquely represented for pricing & manipulation
			for ( ubCnt = 0; ubCnt < ubHowMany; ubCnt++ )
			{
				StoreObjectsInNextFreeDealerInvSlot( usItemIndex, pSpclItemInfo, sSpecialItemElement, 1, ubOwner );
			}
		}
	}
}


static void StoreObjectsInNextFreeDealerInvSlot(UINT16 usItemIndex, SPECIAL_ITEM_INFO* pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner)
{
	INVENTORY_IN_SLOT *pDealerInvSlot;


	// make sure we have the room (memory allocated for it)
	Assert( guiNextFreeInvSlot < gSelectArmsDealerInfo.uiNumDistinctInventoryItems );

	pDealerInvSlot = &(gpTempDealersInventory[ guiNextFreeInvSlot ]);
	guiNextFreeInvSlot++;

	pDealerInvSlot->fActive = TRUE;
	pDealerInvSlot->sItemIndex = usItemIndex;
	pDealerInvSlot->sSpecialItemElement = sSpecialItemElement;
	pDealerInvSlot->ubIdOfMercWhoOwnsTheItem = ubOwner;
	pDealerInvSlot->bSlotIdInOtherLocation = -1;

	// Create the item object ( with no more than MAX_OBJECTS_PER_SLOT )
	// can't use the real #, because CreateItems() will blindly set the bStatus for however many we tell it, beyond 8
	MakeObjectOutOfDealerItems( usItemIndex, pSpclItemInfo, &(pDealerInvSlot->ItemObject), ( UINT8 ) MIN( ubHowMany, MAX_OBJECTS_PER_SLOT ) );

	if ( ubHowMany > MAX_OBJECTS_PER_SLOT )
	{
		// HACK:  Override ItemObject->ubNumberOfObjects (1-8) with the REAL # of items in this box.
		// Note that this makes it an illegal OBJECTTYPE, since there ubHowMany can easily be more than MAX_OBJECTS_PER_SLOT,
		// so there's no room to store the status of all of them one.  But we only so this for perfect items, so
		// we don't care, it works & saves us a lot hassles here.  Just be careful using the damn things!!!  You can't just
		// pass them off the most functions in Items.C(), use ShopkeeperAutoPlaceObject() and ShopkeeperAddItemToPool() instead.
		pDealerInvSlot->ItemObject.ubNumberOfObjects = ubHowMany;
	}
}


static INT8 AddItemToPlayersOfferArea(UINT8 ubProfileID, const INVENTORY_IN_SLOT* pInvSlot, INT8 bSlotIdInOtherLocation);


static BOOLEAN RepairIsDone(UINT16 usItemIndex, UINT8 ubElement)
{
	INVENTORY_IN_SLOT RepairItem;
	INT8  bSlotNum;
	UINT8 ubCnt;


	// make a new shopkeeper invslot item out of it
	RepairItem = INVENTORY_IN_SLOT{};

	RepairItem.fActive = TRUE;
	RepairItem.sItemIndex = usItemIndex;

	// set the owner of the item.  Slot is always -1 of course.
	RepairItem.ubIdOfMercWhoOwnsTheItem = gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].SpecialItem[ ubElement ].ubOwnerProfileId;
	RepairItem.bSlotIdInOtherLocation = -1;

	// Create the item object
	MakeObjectOutOfDealerItems( usItemIndex, &( gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].SpecialItem[ ubElement ].Info ), &RepairItem.ItemObject, 1 );

	if ( CanDealerRepairItem( gbSelectedArmsDealerID, RepairItem.ItemObject.usItem ) )
	{
		// make its condition 100%
		RepairItem.ItemObject.bStatus[ 0 ] = 100;
	}

	// max condition of all permanent attachments on it
	for ( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
	{
		if ( RepairItem.ItemObject.usAttachItem[ ubCnt ] != NONE )
		{
			/* ARM: Can now repair with removeable attachments still attached...
			// If the attachment is a permanent one
			if ( GCM->getItem(RepairItem.ItemObject.usAttachItem[ ubCnt ])->getFlags() & ITEM_INSEPARABLE )*/
			if ( CanDealerRepairItem( gbSelectedArmsDealerID, RepairItem.ItemObject.usAttachItem[ ubCnt ] ) )
			{
				// fix it up
				RepairItem.ItemObject.bAttachStatus[ ubCnt ] = 100;
			}
		}
	}

	// if the item is imprinted (by anyone, even player's mercs), and it's the elctronics guy repairing it
	if (
		/*( gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].SpecialItem[ ubElement ].Info.ubImprintID == (NO_PROFILE + 1) ) && */
		GetDealer(gbSelectedArmsDealerID)->hasFlag(ArmsDealerFlag::REPAIRS_ELECTRONICS))
	{
		// then reset the imprinting!
		RepairItem.ItemObject.ubImprintID = NO_PROFILE;
	}

	//try to add the item to the players offer area
	bSlotNum = AddItemToPlayersOfferArea( RepairItem.ubIdOfMercWhoOwnsTheItem, &RepairItem, -1 );
	// if there wasn't room for it
	if( bSlotNum == -1 )
	{
		// then we have to treat it like it's not done yet!
		return(FALSE);
	}

	PlayersOfferArea[ bSlotNum ].uiFlags |= ARMS_INV_ITEM_REPAIRED;

	// Remove the repaired item from the dealer's permanent inventory list
	RemoveSpecialItemFromArmsDealerInventoryAtElement( gbSelectedArmsDealerID, usItemIndex, ubElement );

	// one less slot is needed.  Don't bother ReMemAllocating, though
	gSelectArmsDealerInfo.uiNumDistinctInventoryItems--;

	// there was room to add it to player's area
	return(TRUE);
}


void DrawHatchOnInventory(SGPVSurface* const uiSurface, const UINT16 usPosX, const UINT16 usPosY, const UINT16 usWidth, const UINT16 usHeight)
{
	SGPRect ClipRect;
	ClipRect.iLeft   = usPosX;
	ClipRect.iRight  = usPosX + usWidth  - 1;
	ClipRect.iTop    = usPosY;
	ClipRect.iBottom = usPosY + usHeight - 1;

	SGPVSurface::Lock l(uiSurface);
	Blt16BPPBufferHatchRect(l.Buffer<UINT16>(), l.Pitch(), &ClipRect);
}


static FLOAT ItemConditionModifier(UINT16 usItemIndex, INT8 bStatus);


static UINT32 CalcShopKeeperItemPrice(BOOLEAN fDealerSelling, BOOLEAN fUnitPriceOnly, UINT16 usItemID, FLOAT dModifier, const OBJECTTYPE* pItemObject)
{
	UINT8  ubCnt;
	UINT32 uiUnitPrice = 0;
	UINT32 uiTotalPrice = 0;
	UINT8  ubItemsToCount = 0;
	UINT8  ubItemsNotCounted = 0;
	UINT32 uiDiscountValue;
	//UINT32 uiDifFrom10 = 0;


	// add up value of the main item(s), exact procedure depends on its item class
	switch ( GCM->getItem(usItemID)->getItemClass() )
	{
		case IC_GUN:
			// add value of the gun
			uiUnitPrice += (UINT32)( CalcValueOfItemToDealer( gbSelectedArmsDealerID, usItemID, fDealerSelling ) *
					ItemConditionModifier(usItemID, pItemObject->bGunStatus) *
					dModifier );

			// if any ammo is loaded
			if( pItemObject->usGunAmmoItem != NONE)
			{
				// if it's regular ammo
				if( GCM->getItem(pItemObject->usGunAmmoItem)->getItemClass() == IC_AMMO )
				{
					// add value of its remaining ammo
					uiUnitPrice += (UINT32)( CalcValueOfItemToDealer( gbSelectedArmsDealerID, pItemObject->usGunAmmoItem, fDealerSelling ) *
								ItemConditionModifier(pItemObject->usGunAmmoItem, pItemObject->ubGunShotsLeft) *
								dModifier );
				}
				else // assume it's attached ammo (mortar shells, grenades)
				{
					// add its value (uses normal status 0-100)
					uiUnitPrice += (UINT32)( CalcValueOfItemToDealer( gbSelectedArmsDealerID, pItemObject->usGunAmmoItem, fDealerSelling ) *
							ItemConditionModifier(pItemObject->usGunAmmoItem, pItemObject->bGunAmmoStatus) *
							dModifier );
				}
			}

			// if multiple guns are stacked, we've only counted the first one
			ubItemsNotCounted = pItemObject->ubNumberOfObjects - 1;
			break;

		case IC_AMMO:
		default:
			// this must handle overloaded objects from dealer boxes!
			if ( pItemObject->ubNumberOfObjects <= MAX_OBJECTS_PER_SLOT )
			{
				// legal amount, count them all normally (statuses could be different)
				ubItemsToCount = pItemObject->ubNumberOfObjects;
				ubItemsNotCounted = 0;
				// in this situation, uiUnitPrice will actually be the sum of the values of ALL the multiple objects
			}
			else
			{
				// overloaded amount, count just the first, the others must all be identical
				ubItemsToCount = 1;
				ubItemsNotCounted = pItemObject->ubNumberOfObjects - 1;
			}

			// add the value of each magazine (multiple mags may have vastly different #bullets left)
			for (ubCnt = 0; ubCnt < ubItemsToCount; ubCnt++ )
			{
				// for bullets, ItemConditionModifier will convert the #ShotsLeft into a percentage
				uiUnitPrice += (UINT32)( CalcValueOfItemToDealer( gbSelectedArmsDealerID, usItemID, fDealerSelling ) *
							ItemConditionModifier(usItemID, pItemObject->bStatus[ ubCnt ]) *
							dModifier );

				if ( fUnitPriceOnly )
				{
					// want price for only one of them.  All statuses must be the same in order to use this flag!
					break;
				}
			}
			break;
	}


	// loop through any attachments and add in their price
	for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++)
	{
		if( pItemObject->usAttachItem[ ubCnt ] != NONE )
		{
			// add value of this particular attachment
			uiUnitPrice += (UINT32)( CalcValueOfItemToDealer( gbSelectedArmsDealerID, pItemObject->usAttachItem[ ubCnt ], fDealerSelling ) *
					ItemConditionModifier(pItemObject->usAttachItem[ ubCnt ], pItemObject->bAttachStatus[ ubCnt ]) *
					dModifier );
		}
	}


	// if Flo is doing the dealin' and wheelin'
	if ( gpSMCurrentMerc->ubProfile == FLO )
	{
		// if it's a GUN or AMMO (but not Launchers, and all attachments and payload is included)
		switch ( GCM->getItem(usItemID)->getItemClass() )
		{
			// start components of IC_WEAPON:
			case IC_GUN:
			case IC_BLADE:
			case IC_THROWING_KNIFE:
			case IC_LAUNCHER:
			// end components of IC_WEAPON
			case IC_AMMO:
				uiDiscountValue = ( uiUnitPrice * FLO_DISCOUNT_PERCENTAGE ) / 100;

				// she gets a discount!  Read her M.E.R.C. profile to understand why
				if ( fDealerSelling )
				{
					// she buys for less...
					uiUnitPrice -= uiDiscountValue;
				}
				else
				{
					// and sells for more!
					uiUnitPrice += uiDiscountValue;
				}
				break;
		}
	}


	// if it's the dealer selling this, make sure the item is worth at least $1
	// if he is buying this from a player, then we allow a value of 0, since that has a special "worthless" quote #18
	if( fDealerSelling && ( uiUnitPrice == 0 ) )
	{
		uiUnitPrice = 1;
	}

	/*
	//if the price is not diviseble by 10, make it so
	uiDifFrom10 = 10 - uiUnitPrice % 10;
	if( uiDifFrom10 != 0 && uiDifFrom10 != 10 )
	{
		uiUnitPrice += uiDifFrom10;
	}*/

	// we're always count the first one
	uiTotalPrice = uiUnitPrice;

	// if NOT pricing just one
	if ( !fUnitPriceOnly )
	{
		// add value of all that weren't already counted
		uiTotalPrice += ( ubItemsNotCounted * uiUnitPrice );
	}

	return( uiTotalPrice );
}


static FLOAT ItemConditionModifier(UINT16 usItemIndex, INT8 bStatus)
{
	FLOAT dConditionModifier = 1.0f;

	//if the item is ammo, the condition modifier is based on how many shots are left
	if( GCM->getItem(usItemIndex)->getItemClass() == IC_AMMO )
	{
		// # bullets left / max magazine capacity
		dConditionModifier = bStatus / ((FLOAT) GCM->getItem(usItemIndex)->asAmmo()->capacity? (FLOAT) GCM->getItem(usItemIndex)->asAmmo()->capacity: 1);
	}
	else	// non-ammo
	{
		// handle non-ammo payloads in jammed guns as if they weren't
		if ( bStatus < 0 )
		{
			bStatus *= -1;
		}

		// an item at 100% is worth full price...

		if ( GCM->getItem(usItemIndex)->getFlags() & ITEM_REPAIRABLE )
		{
			// a REPAIRABLE item at 0% is still worth 50% of its full price, not 0%
			dConditionModifier = 0.5f + ( bStatus / (FLOAT)200 );
		}
		else
		{
			// an UNREPAIRABLE item is worth precisely its condition percentage
			dConditionModifier = bStatus / (FLOAT)100;
		}
	}

	return( dConditionModifier );
}


static void DisplayArmsDealerOfferArea(void)
{
	INT16   sCnt, sCount;
	UINT32  uiTotalCost;
	UINT16  usPosX, usPosY;
	BOOLEAN fDisplayHatchOnItem;


	usPosX = SKI_ARMS_DEALERS_TRADING_INV_X;
	usPosY = SKI_ARMS_DEALERS_TRADING_INV_Y;

	if( gubSkiDirtyLevel != SKI_DIRTY_LEVEL0 )
	{
		//Restore the area before blitting the new data on
		RestoreExternBackgroundRect( usPosX, usPosY, SKI_ARMS_DEALERS_TRADING_INV_WIDTH, SKI_ARMS_DEALERS_TRADING_INV_HEIGHT );
	}

	sCount = 0;

	//Display all the items that are in the Arms traders offered area
	uiTotalCost = 0;
	for( sCnt=0; sCnt<SKI_NUM_TRADING_INV_SLOTS; sCnt++)
	{
		const INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[sCnt];
		if (a->fActive)
		{
			// hatch it out if region is disabled
			if ( !( gDealersOfferSlotsMouseRegions[ sCnt ].uiFlags & MSYS_REGION_ENABLED ) )
			{
				fDisplayHatchOnItem = TRUE;
			}
			else if (a->uiFlags & ARMS_INV_ITEM_SELECTED)
			{
				fDisplayHatchOnItem = TRUE;
			}
			else
				fDisplayHatchOnItem = FALSE;

			// Display the inventory slot
			uiTotalCost += DisplayInvSlot(sCnt, a->sItemIndex, usPosX, usPosY, a->ItemObject, fDisplayHatchOnItem, ARMS_DEALER_OFFER_AREA);
		}
		else	// empty
		{
			// hatch it out if region is disabled
			if ( !( gDealersOfferSlotsMouseRegions[ sCnt ].uiFlags & MSYS_REGION_ENABLED ) )
			{
				HatchOutInvSlot( usPosX, usPosY );
			}
		}

		usPosX += SKI_INV_OFFSET_X;

		sCount++;

		//if we are on to the next row
		if( !( sCount % SKI_NUM_TRADING_INV_COLS ) )
		{
			usPosX = SKI_ARMS_DEALERS_TRADING_INV_X;
			usPosY += SKI_INV_OFFSET_Y;
		}
	}

	if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
	{
		//
		//Display the Total cost text and number
		//

		//Restore the previous area
		RestoreExternBackgroundRect( SKI_ARMS_DEALER_TOTAL_COST_X, SKI_ARMS_DEALER_TOTAL_COST_Y, SKI_ARMS_DEALER_TOTAL_COST_WIDTH, SKI_ARMS_DEALER_TOTAL_COST_HEIGHT );

		//Display the total cost text
		DrawTextToScreen(SPrintMoney(uiTotalCost), SKI_ARMS_DEALER_TOTAL_COST_X, SKI_ARMS_DEALER_TOTAL_COST_Y + 5, SKI_INV_SLOT_WIDTH, SKI_LABEL_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}
}


static INT8 AddItemToArmsDealerOfferArea(const INVENTORY_IN_SLOT* pInvSlot, INT8 bSlotIdInOtherLocation)
{
	INT8 bCnt;

	for( bCnt=0; bCnt<SKI_NUM_TRADING_INV_SLOTS; bCnt++)
	{
		INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[bCnt];
		//if there are no items here, copy the data in
		if (!a->fActive)
		{
			//Copy the inventory items
			*a = *pInvSlot;

			//if the shift key is being pressed, add them all
			if (_KeyDown(SHIFT))
			{
				a->ItemObject.ubNumberOfObjects = pInvSlot->ItemObject.ubNumberOfObjects;
			}
			else if( pInvSlot->ItemObject.ubNumberOfObjects > 1 )
			{
				//If there was more then 1 item, reduce it to only 1 item moved
				a->ItemObject.ubNumberOfObjects = 1;
			}

			//Remember where the item came from
			a->bSlotIdInOtherLocation   = bSlotIdInOtherLocation;
			a->ubIdOfMercWhoOwnsTheItem = pInvSlot->ubIdOfMercWhoOwnsTheItem;
			a->fActive                  = TRUE;

			SetSkiRegionHelpText(    a, &gDealersOfferSlotsMouseRegions[bCnt],          ARMS_DEALER_OFFER_AREA);
			SetSkiFaceRegionHelpText(a, &gDealersOfferSlotsSmallFaceMouseRegions[bCnt], ARMS_DEALER_OFFER_AREA);

			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			return( bCnt );
		}
	}

	return( -1 );
}


static void ClearArmsDealerOfferSlot(INT32 ubSlotToClear);


static BOOLEAN RemoveItemFromArmsDealerOfferArea(INT8 bSlotId, BOOLEAN fKeepItem)
{
	//UINT16 usCnt;

	//Loop through all the slot to see if the requested one is here
	//for( usCnt=0; usCnt<SKI_NUM_TRADING_INV_SLOTS; usCnt++)
	{
		const INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[bSlotId];
		//if this is the requested slot
		if (a->fActive)//bSlotIdInOtherLocation == bSlotIdInOtherLocation )
		{
			//if there are more then 1 item
			//if( ArmsDealerOfferArea[ usCnt ].ItemObject.ubNumberOfObjects > 1 )
			if( fKeepItem )
			{
				gpTempDealersInventory[a->bSlotIdInOtherLocation].ItemObject.ubNumberOfObjects += a->ItemObject.ubNumberOfObjects;
			}

			//Clear the flag that hatches out the item
			gpTempDealersInventory[a->bSlotIdInOtherLocation].uiFlags &= ~ARMS_INV_ITEM_SELECTED;

			ClearArmsDealerOfferSlot( bSlotId );

			return( TRUE );
		}
	}

	return( FALSE );
}


static ST::string BuildItemHelpTextString(const INVENTORY_IN_SLOT* pInv, UINT8 ubScreenArea);


static void SetSkiRegionHelpText(const INVENTORY_IN_SLOT* pInv, MOUSE_REGION* pRegion, UINT8 ubScreenArea)
{
	Assert( pRegion );

	ST::string zHelpText = BuildItemHelpTextString(pInv, ubScreenArea );
	pRegion->SetFastHelpText(zHelpText);
}


static void SetSkiFaceRegionHelpText(const INVENTORY_IN_SLOT* pInv, MOUSE_REGION* pRegion, UINT8 ubScreenArea)
{
	ST::string zTempText;
	ST::string zHelpText;

	Assert( pRegion );

	//if the item isn't NULL, and is owned by a merc
	if( ( pInv != NULL ) && ( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE ) )
	{
		zTempText = BuildItemHelpTextString(pInv, ubScreenArea);
		// add who owns it
		zHelpText = ST::format("{}{} {}", gMercProfiles[ pInv->ubIdOfMercWhoOwnsTheItem ].zNickname, pMessageStrings[ MSG_DASH_S ], zTempText);
	}
	else
	{
		zHelpText = ST::null;
	}
	pRegion->SetFastHelpText(zHelpText);
}


static INVENTORY_IN_SLOT* GetPtrToOfferSlotWhereThisItemIs(UINT8 ubProfileID, INT8 bInvPocket);


static INT8 AddItemToPlayersOfferArea(UINT8 ubProfileID, const INVENTORY_IN_SLOT* pInvSlot, INT8 bSlotIdInOtherLocation)
{
	INT8 bCnt;

	//if we are to check for a previous slot
	if( bSlotIdInOtherLocation != -1 )
	{
		//if the item has already been added somewhere. don't add it again
		if( GetPtrToOfferSlotWhereThisItemIs( ubProfileID, bSlotIdInOtherLocation ) != NULL )
			return( -1 );
	}


	//look for the first free slot
	for( bCnt=0; bCnt<SKI_NUM_TRADING_INV_SLOTS; bCnt++)
	{
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[bCnt];
		//if there are no items here, copy the data in
		if (!o->fActive)
		{
			*o = *pInvSlot;

			o->fActive = TRUE;

			//Specify the owner of the merc
			o->ubIdOfMercWhoOwnsTheItem = ubProfileID;
			o->bSlotIdInOtherLocation = bSlotIdInOtherLocation;

			IfMercOwnedCopyItemToMercInv(o);

			SetSkiRegionHelpText(    o, &gPlayersOfferSlotsMouseRegions[bCnt],          PLAYERS_OFFER_AREA);
			SetSkiFaceRegionHelpText(o, &gPlayersOfferSlotsSmallFaceMouseRegions[bCnt], PLAYERS_OFFER_AREA);


			//if the item we are adding is money
			if( GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY )
			{
				//Since money is always evaluated
				o->uiFlags     |= ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
				o->uiItemPrice  = o->ItemObject.uiMoneyAmount;
			}
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

			return( bCnt );
		}
	}

	return( -1 );
}


static void ClearPlayersOfferSlot(INT32 ubSlotToClear);
static BOOLEAN SKITryToReturnInvToOwnerOrCurrentMerc(INVENTORY_IN_SLOT* pInv);


static BOOLEAN RemoveItemFromPlayersOfferArea(INT8 bSlot)
{
	INVENTORY_IN_SLOT* const o = &PlayersOfferArea[bSlot];
	//if the item doesn't have a duplicate copy in its owner merc's inventory slot
	if (o->bSlotIdInOtherLocation == -1 &&
		!SKITryToReturnInvToOwnerOrCurrentMerc(o))
	{
		//failed to add item, inventory probably filled up or item is unowned and current merc ineligible
		return( FALSE );
	}

	// Clear the contents
	ClearPlayersOfferSlot( bSlot );

	// Dirty
	fInterfacePanelDirty = DIRTYLEVEL2;
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
	return( TRUE );
}


static UINT32 EvaluateInvSlot(INVENTORY_IN_SLOT* pInvSlot);
static BOOLEAN WillShopKeeperRejectObjectsFromPlayer(INT8 bDealerId, INT8 bSlotId);


static void DisplayPlayersOfferArea(void)
{
	INT16   sCnt, sCount;
	UINT32  uiTotalCost;
	UINT16  usPosX, usPosY;
	BOOLEAN fDisplayHatchOnItem=FALSE;


	usPosX = SKI_PLAYERS_TRADING_INV_X;
	usPosY = SKI_PLAYERS_TRADING_INV_Y;

	if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
	{
		//Restore the area before blitting the new data on
		RestoreExternBackgroundRect( SKI_PLAYERS_TRADING_INV_X, SKI_PLAYERS_TRADING_INV_Y, SKI_PLAYERS_TRADING_INV_WIDTH, SKI_PLAYERS_TRADING_INV_HEIGHT );
	}

	sCount = 0;


	//Display all the items that are in the Players offered area
	uiTotalCost = 0;
	for( sCnt=0; sCnt<SKI_NUM_TRADING_INV_SLOTS; sCnt++)
	{
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[sCnt];
		if (o->fActive)
		{
			//if the item is money
			if (GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY)
			{
				//get an updated status from the amount in the pocket
				if (o->bSlotIdInOtherLocation != -1 && o->ubIdOfMercWhoOwnsTheItem != NO_PROFILE)
				{
					const SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(o->ubIdOfMercWhoOwnsTheItem);
					Assert(s != NULL);
					o->ItemObject.uiMoneyAmount = s->inv[o->bSlotIdInOtherLocation].uiMoneyAmount;
					o->uiItemPrice = o->ItemObject.uiMoneyAmount;
				}
			}
			else	// not money
			{
				//if non-repairman
				if(!DoesDealerDoRepairs(gbSelectedArmsDealerID))
				{
					// don't evaluate anything he wouldn't buy!
					if (!WillShopKeeperRejectObjectsFromPlayer(gbSelectedArmsDealerID, sCnt))
					{
						// skip purchased items!
						if (!(o->uiFlags & ARMS_INV_JUST_PURCHASED))
						{
							// re-evaluate the value of the item (needed for Flo's discount handling)
							EvaluateInvSlot(o);
						}
					}
				}
			}

			// hatch it out if it hasn't been evaluated or just purchased
			//fDisplayHatchOnItem = (o->uiFlags & (ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED | ARMS_INV_JUST_PURCHASED)) == 0;

			// Display the inventory slot
			DisplayInvSlot(sCnt, o->sItemIndex, usPosX, usPosY, o->ItemObject, fDisplayHatchOnItem, PLAYERS_OFFER_AREA);

			if (o->uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE)
			{
				uiTotalCost += o->uiItemPrice;
			}
		}
		usPosX += SKI_INV_OFFSET_X;

		sCount++;

		//if we are on to the next row
		if( !( sCount % SKI_NUM_TRADING_INV_COLS ) )
		{
			usPosX = SKI_ARMS_DEALERS_TRADING_INV_X;
			usPosY += SKI_INV_OFFSET_Y;
		}
	}

	if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
	{
		//
		//Display the Total value text and number
		//

		//Restore the previous area
		RestoreExternBackgroundRect( SKI_PLAYERS_TOTAL_VALUE_X, SKI_PLAYERS_TOTAL_VALUE_Y, SKI_PLAYERS_TOTAL_VALUE_WIDTH, SKI_PLAYERS_TOTAL_VALUE_HEIGHT );

		//Display the total cost text
		DrawTextToScreen(SPrintMoney(uiTotalCost), SKI_PLAYERS_TOTAL_VALUE_X, SKI_PLAYERS_TOTAL_VALUE_Y + 5, SKI_INV_SLOT_WIDTH, SKI_LABEL_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}

	CrossOutUnwantedItems( );
}


static INVENTORY_IN_SLOT* GetPtrToOfferSlotWhereThisItemIs(UINT8 ubProfileID, INT8 bInvPocket)
{
	UINT8 ubCnt = 0;

	for( ubCnt = 0; ubCnt < SKI_NUM_TRADING_INV_SLOTS; ubCnt++ )
	{
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubCnt];
		if (o->bSlotIdInOtherLocation == bInvPocket &&
			o->ubIdOfMercWhoOwnsTheItem == ubProfileID &&
			o->ItemObject.ubNumberOfObjects != 0)
		{
			return o;
		}

		INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[ubCnt];
		if (a->bSlotIdInOtherLocation == bInvPocket &&
			a->ubIdOfMercWhoOwnsTheItem == ubProfileID &&
			a->ItemObject.ubNumberOfObjects != 0)
		{
			return a;
		}
	}

	// not found (perfectly valid result)
	return( NULL );
}


BOOLEAN ShouldSoldierDisplayHatchOnItem( UINT8 ubProfileID, INT16 sSlotNum )
{
	INVENTORY_IN_SLOT *pInvSlot = NULL;

	pInvSlot = GetPtrToOfferSlotWhereThisItemIs( ubProfileID, ( INT8 ) sSlotNum );

	if( pInvSlot == NULL )
		// not found in either offer area
		return( FALSE );
	else
		// found it, display it hatched out
		return( TRUE );
}


static UINT32 CalculateTotalArmsDealerCost(void)
{
	UINT32 uiCnt;
	UINT32 uiTotal=0;

	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		const INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[uiCnt];
		if (a->fActive)
		{
			//if the dealer repairs
			if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
			{
				uiTotal += CalculateObjectItemRepairCost(gbSelectedArmsDealerID, &a->ItemObject);
			}
			else
			{
				uiTotal += CalcShopKeeperItemPrice(DEALER_SELLING, FALSE, a->sItemIndex, SelectedArmsDealer()->sellingPrice, &a->ItemObject);
			}
		}
	}
	return( uiTotal );
}


static UINT32 CalculateTotalPlayersValue(void)
{
	UINT8  ubCnt;
	UINT32 uiTotal = 0;

	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		const INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubCnt];
		if (o->fActive && o->uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE)
		{
			//Calculate a price for the item
			if (GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY)
			{
				uiTotal += o->ItemObject.uiMoneyAmount;
			}
			else
			{
				uiTotal += o->uiItemPrice;
			}
		}
	}

	return( uiTotal );
}


class DialogueEventSkipAFrame : public DialogueEvent
{
	public:
		bool Execute() { return false; }
};


class DialogueEventShopkeeperLockTransactionButton : public DialogueEvent
{
	public:
		DialogueEventShopkeeperLockTransactionButton(bool const lock) : lock_(lock) {}

		bool Execute()
		{
			if (guiCurrentScreen == SHOPKEEPER_SCREEN)
			{
				EnableButton(guiSKI_TransactionButton, !lock_);
			}
			return false;
		}

	private:
		bool const lock_;
};


class DialogueEventShopkeeperMoney : public DialogueEvent
{
	public:
		DialogueEventShopkeeperMoney(const ST::string& message, UINT32 money_amount, MessageBoxFlags flags, MSGBOX_CALLBACK callback) :
			message_(message),
			money_amount_(money_amount),
			flags_(flags),
			callback_(callback)
		{}

		bool Execute()
		{
			ST::string zMoney = SPrintMoney(money_amount_);
			ST::string zText = st_format_printf(message_, zMoney);
			DoSkiMessageBox(zText, SHOPKEEPER_SCREEN, flags_, callback_);
			return false;
		}

	private:
		ST::string      const message_;
		UINT32          const money_amount_;
		MessageBoxFlags const flags_;
		MSGBOX_CALLBACK const callback_;
};


static UINT32 CalculateHowMuchMoneyIsInPlayersOfferArea();
static UINT8 CountNumberOfItemsInTheArmsDealersOfferArea();
static UINT8 CountNumberOfValuelessItemsInThePlayersOfferArea();
static void DealerGetsBribed(UINT8 ubProfileId, UINT32 uiMoneyAmount);
static void GivePlayerSomeChange(UINT32 uiAmount);
static BOOLEAN IsMoneyTheOnlyItemInThePlayersOfferArea(void);
static void MoveAllArmsDealersItemsInOfferAreaToPlayersOfferArea(void);
static void MovePlayerOfferedItemsOfValueToArmsDealersInventory(void);
static void MovePlayersItemsToBeRepairedToArmsDealersInventory(void);
static BOOLEAN StartShopKeeperTalking(UINT16 usQuoteNum);


static void PerformTransaction(UINT32 uiMoneyFromPlayersAccount)
{
	UINT32 uiPlayersTotalMoneyValue = CalculateTotalPlayersValue() + uiMoneyFromPlayersAccount;
	UINT32 uiArmsDealersItemsCost = CalculateTotalArmsDealerCost();
	UINT32 uiMoneyInPlayersOfferArea = CalculateHowMuchMoneyIsInPlayersOfferArea( );
	INT32  iChangeToGiveToPlayer = 0;
	UINT32 uiAvailablePlayerOfferSlots;


	//if the player has already requested to leave, get out
	if( gfUserHasRequestedToLeave )
		return;


	// handle bribing... if the player is giving the dealer money, without buying anything
	if( IsMoneyTheOnlyItemInThePlayersOfferArea( ) && CountNumberOfItemsInTheArmsDealersOfferArea( ) == 0 )
	{
		// accept/refuse money (varies by dealer according to ACCEPTS_GIFTS flag)
		StartShopKeeperTalking( SK_QUOTES_DEALER_OFFERED_MONEY_AS_A_GIFT );

		// if the arms dealer is the kind of person who accepts gifts
		if( SelectedArmsDealer()->hasFlag(ArmsDealerFlag::ACCEPTS_GIFTS) )
		{
			//Move all the players evaluated items to the dealer (also adds it to dealer's cash)
			MovePlayerOfferedItemsOfValueToArmsDealersInventory();

			DealerGetsBribed( SelectedArmsDealer()->profileID, uiMoneyInPlayersOfferArea );
		}
	}
	else // not a bribe
	{
		//if the dealer is not a repairman
		if(!DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			uiAvailablePlayerOfferSlots = ( SKI_NUM_TRADING_INV_SLOTS - CountNumberOfValuelessItemsInThePlayersOfferArea( ) );

			// always reserve an empty slot for change.  We can't tell for sure whether player is getting change because that
			// could depend on how many items he's trying to buy won't fit - he could only be getting change because of that!
			if ( uiAvailablePlayerOfferSlots > 0 )
			{
				uiAvailablePlayerOfferSlots--;
			}

			//if there is NOT enough room in the players offer area
			if( CountNumberOfItemsInTheArmsDealersOfferArea( ) > uiAvailablePlayerOfferSlots )
			{
				// tell player there's not enough room in the player's offer area
				DoSkiMessageBox(SKI_Text[SKI_TEXT_NO_MORE_ROOM_IN_PLAYER_OFFER_AREA], SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK, NULL);

				return;
			}
		}

		//if the player doesn't have enough money to pay for what he's buying
		if( uiArmsDealersItemsCost > uiPlayersTotalMoneyValue )
		{
			//if the player doesn't have enough money in his account to pay the rest
			if( uiArmsDealersItemsCost > uiPlayersTotalMoneyValue + LaptopSaveInfo.iCurrentBalance )
			{
				// tell player he can't possibly afford this
				DialogueEvent::Add(new DialogueEventShopkeeperLockTransactionButton(true));
				DialogueEvent::Add(new DialogueEventSkipAFrame());
				UINT32 const amount = uiArmsDealersItemsCost - (LaptopSaveInfo.iCurrentBalance + uiPlayersTotalMoneyValue);
				DialogueEvent::Add(new DialogueEventShopkeeperMoney(SkiMessageBoxText[SKI_SHORT_FUNDS_TEXT], amount, MSG_BOX_FLAG_OK, ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack));
			}
			else
			{
				// player doesn't have enough on the table, but can pay for it from his balance
				/// ask player if wants to subtract the shortfall directly from his balance
				DialogueEvent::Add(new DialogueEventSkipAFrame());
				DialogueEvent::Add(new DialogueEventShopkeeperLockTransactionButton(true));

				ST::string message =
					uiPlayersTotalMoneyValue != 0 ? SkiMessageBoxText[SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_DIFFERENCE] :
					SkiMessageBoxText[SKI_QUESTION_TO_DEDUCT_MONEY_FROM_PLAYERS_ACCOUNT_TO_COVER_COST];
				UINT32 const amount = uiArmsDealersItemsCost - uiPlayersTotalMoneyValue;
				DialogueEvent::Add(new DialogueEventShopkeeperMoney(message, amount, MSG_BOX_FLAG_YESNO, ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack));
			}

			DialogueEvent::Add(new DialogueEventShopkeeperLockTransactionButton(false));

			gfResetShopKeepIdleQuote = TRUE;

			// disable further calls to PerformTransaction() until callback calls us again to free this flag
			gfPerformTransactionInProgress = TRUE;

			return;
		}


		// to get this far, player should have the money needed!
		Assert( uiPlayersTotalMoneyValue >= uiArmsDealersItemsCost );


		//if the dealer repairs
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			//Make sure there is enough money in the Player's offer area to cover the repair charge
			if( ( uiMoneyFromPlayersAccount + uiMoneyInPlayersOfferArea ) >= uiArmsDealersItemsCost )
			{
				//Move all the players evaluated items to the dealer ( For a repairman, this means just: get rid of the money )
				MovePlayerOfferedItemsOfValueToArmsDealersInventory();

				//if the arms dealer is the type of person to give change
				if( SelectedArmsDealer()->hasFlag(ArmsDealerFlag::GIVES_CHANGE) )
				{
					//determine the amount of change to give
					iChangeToGiveToPlayer = ( uiMoneyFromPlayersAccount + uiMoneyInPlayersOfferArea ) - uiArmsDealersItemsCost;

					//if there is change
					if( iChangeToGiveToPlayer > 0 )
					{
						GivePlayerSomeChange( iChangeToGiveToPlayer );

						if( uiArmsDealersItemsCost > 0 )
						{
							StartShopKeeperTalking( SK_QUOTES_PLAYER_HAS_TOO_MUCH_MONEY_FOR_TRANSACTION );
						}
					}
					else
						StartShopKeeperTalking( SK_QUOTES_PLAYER_HAS_EXACTLY_ENOUGH_MONEY_FOR_TRANSACTION );
				}


				// move the offered items to the repairman's inventory
				MovePlayersItemsToBeRepairedToArmsDealersInventory();
			}
			else
			{
				//the player doesn't have enough money
				return;
			}
		}
		else // non-repairman
		{
			// DON'T include uiMoneyFromPlayersAccount in change given to player.  The only way this can happen is if he agrees
			// to subtract from his balance, but there isn't enough room.  In that case, the cost of items not transfered is
			// used to reduce his balance deduction instead (not here, in the calling function).
			iChangeToGiveToPlayer = CalculateTotalPlayersValue() - uiArmsDealersItemsCost;


			//if the arms dealer buys stuff
			if( ( SelectedArmsDealer()->type == ARMS_DEALER_BUYS_ONLY ) ||
				( SelectedArmsDealer()->type == ARMS_DEALER_BUYS_SELLS ) )
			{
				// but the dealer can't afford this transaction
				if( iChangeToGiveToPlayer > ( INT32 ) gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash )
				{
					// HACK HACK HACK: We forgot to write/record quote 27 for Jake, so he ALWAYS must have enough money!
					if ( gbSelectedArmsDealerID != ARMS_DEALER_JAKE )
					{
						// no deal - dealer can't afford it
						StartShopKeeperTalking( SK_QUOTES_CANT_AFFORD_TO_BUY_OR_TOO_MUCH_TO_REPAIR );
						gfResetShopKeepIdleQuote = TRUE;

						return;
					}
				}
			}


			//Move all the player's evaluated items to the dealer
			MovePlayerOfferedItemsOfValueToArmsDealersInventory();

			//Move all dealers offered items to the player
			MoveAllArmsDealersItemsInOfferAreaToPlayersOfferArea( );


			//if the arms dealer is the type of person to give change
			if( SelectedArmsDealer()->hasFlag(ArmsDealerFlag::GIVES_CHANGE) )
			{
				if( iChangeToGiveToPlayer > 0 )
				{
					GivePlayerSomeChange( iChangeToGiveToPlayer );

					/*
					//Remove the change the arms dealer is returning to the player
					if( ( gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash - uiChangeToGiveToPlayer ) >= 0 )
						gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash -= uiChangeToGiveToPlayer;
					else
						gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash = 0;*/

					if( uiArmsDealersItemsCost == 0 )
					{
						StartShopKeeperTalking( SK_QUOTES_PLAYER_HAS_EXACTLY_ENOUGH_MONEY_FOR_TRANSACTION );
					}
					else
					{
						StartShopKeeperTalking( SK_QUOTES_PLAYER_HAS_TOO_MUCH_MONEY_FOR_TRANSACTION );
					}
				}
				else if( iChangeToGiveToPlayer == 0 )
					StartShopKeeperTalking( SK_QUOTES_PLAYER_HAS_EXACTLY_ENOUGH_MONEY_FOR_TRANSACTION );
			}

			//if the arms dealer is Howard( 125), set fact 222
			if( gbSelectedArmsDealerID == ARMS_DEALER_HOWARD )
				SetFactTrue(FACT_222);
		}


		// add cash transfered from player's account directly to dealer's cash balance
		if( uiMoneyFromPlayersAccount > 0 )
		{
			gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash += uiMoneyFromPlayersAccount;
		}

		//Redraw the screen
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

		gfDoneBusinessThisSession = TRUE;

		//The shopkeeper's inventory has changed, reinitialize
		InitializeShopKeeper( FALSE );
	}


	gfResetShopKeepIdleQuote = TRUE;
}


static void RemoveItemFromDealersInventory(const INVENTORY_IN_SLOT* pInvSlot, UINT8 ubSlot);


static void MoveAllArmsDealersItemsInOfferAreaToPlayersOfferArea(void)
{
	//for all items in the dealers items offer area
	UINT32 uiCnt;
	INT8   bSlotID=0;

	//loop through all the slots in the shopkeeper's offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		const INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[uiCnt];
		//if there is an item here
		if (a->fActive)
		{
			bSlotID = AddItemToPlayersOfferArea(NO_PROFILE, a, -1);

			if( bSlotID != -1 )
			{
				PlayersOfferArea[ bSlotID ].uiFlags |= ARMS_INV_JUST_PURCHASED;

				//if the player has just purchased a VIDEO_CAMERA from Franz Hinkle
				if (gbSelectedArmsDealerID == ARMS_DEALER_FRANZ && a->sItemIndex == VIDEO_CAMERA)
				{
					// set a special flag
					gArmsDealerStatus[ gbSelectedArmsDealerID ].ubSpecificDealerFlags |= ARMS_DEALER_FLAG__FRANZ_HAS_SOLD_VIDEO_CAMERA_TO_PLAYER;
				}
			}

			//Remove the items out of the dealers inventory
			RemoveItemFromDealersInventory(a, a->bSlotIdInOtherLocation);

			//Remove the items from the Shopkeepers Offer area
			if( !RemoveItemFromArmsDealerOfferArea( (UINT8)uiCnt, FALSE ) )//a->bSlotIdInOtherLocation
				SLOGA("MoveAllArmsDealersItemsInOfferAreaToPlayersOfferArea: problem removing an item from dealers offer area");

			Assert(!a->fActive);
		}
	}

	//loop through all the slots in the shopkeeper's offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		Assert(!ArmsDealerOfferArea[uiCnt].fActive);
	}
}


static void RemoveItemFromDealersInventory(const INVENTORY_IN_SLOT* pInvSlot, UINT8 ubSlot)
{
	INT16 sInvSlot;
	INT16 sItemID;
	SPECIAL_ITEM_INFO SpclItemInfo;

	sInvSlot = ubSlot;
	//sInvSlot = ( gSelectArmsDealerInfo.ubCurrentPage - 1 ) * SKI_NUM_ARMS_DEALERS_INV_SLOTS + ubSlot;

	//Remove all of this item out of the specified inventory slot
	sItemID = gpTempDealersInventory[ sInvSlot ].sItemIndex;
	SetSpecialItemInfoFromObject( &SpclItemInfo, &(pInvSlot->ItemObject) );
	RemoveItemFromArmsDealerInventory( gbSelectedArmsDealerID, sItemID, &SpclItemInfo, pInvSlot->ItemObject.ubNumberOfObjects );

	gfResetShopKeepIdleQuote = TRUE;
}


static void MovePlayerOfferedItemsOfValueToArmsDealersInventory(void)
{
	UINT32	uiCnt;

	//loop through all the slots in the players offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[uiCnt];
		//if there is an item here
		if (o->fActive)
		{
			// and it has value
			if (o->uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE)
			{
				//Remove the item from the owner merc's inventory
				IfMercOwnedRemoveItemFromMercInv(o);

				//if the item is money
				if (GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY)
				{
					//add the money to the dealers 'cash'
					gArmsDealerStatus[gbSelectedArmsDealerID].uiArmsDealersCash += o->ItemObject.uiMoneyAmount;
				}
				else
				{
					//if the dealer doesn't strictly buy items from the player, give the item to the dealer
					if( SelectedArmsDealer()->type != ARMS_DEALER_BUYS_ONLY )
					{
						// item cease to be merc-owned during this operation
						AddObjectToArmsDealerInventory(gbSelectedArmsDealerID, &o->ItemObject);
					}
				}

				//erase the item from the player's offer area
				ClearPlayersOfferSlot( uiCnt );
			}
		}
	}

	gfResetShopKeepIdleQuote = TRUE;
}


static SOLDIERTYPE* GetMovingItemOwner()
{
	ProfileID const owner_pid = gMoveingItem.ubIdOfMercWhoOwnsTheItem;
#if 0 /* XXX -1 */
	if (owner_pid == -1) return gpSMCurrentMerc;
#else
	if (owner_pid == (UINT8)-1) return gpSMCurrentMerc;
#endif
	SOLDIERTYPE* const owner = FindSoldierByProfileIDOnPlayerTeam(owner_pid);
	return owner ? owner : gpSMCurrentMerc;
}


static void DisableAllDealersInventorySlots(void);
static void DisableAllDealersOfferSlots(void);


void BeginSkiItemPointer( UINT8 ubSource, INT8 bSlotNum, BOOLEAN fOfferToDealerFirst )
{
	BOOLEAN fRestrictMouseToRect = FALSE;
	SGPRect Rect;
	OBJECTTYPE TempObject;

	/*
	// If we are already moving an item
	if ( gMoveingItem.sItemIndex != 0 )
	{
		return;
	}*/

	switch( ubSource )
	{
		case ARMS_DEALER_INVENTORY:
			//Should never get in here
			SLOGA("BeginSkiItemPointer: invalid Source");
			return;

		case ARMS_DEALER_OFFER_AREA:
		{
			//Get the item from the slot.
			gMoveingItem = ArmsDealerOfferArea[bSlotNum];
			IfMercOwnedRemoveItemFromMercInv( &gMoveingItem );

			//remove the item from the slot
			ClearArmsDealerOfferSlot( bSlotNum );


			//Restrict the cursor to the arms dealer offer area, players offer area and the players inventory
			Rect.iLeft = 0;//SKI_ITEM_MOVEMENT_AREA_X;
			Rect.iTop = SKI_DEALER_OFFER_AREA_Y;
			Rect.iRight = SKI_ITEM_MOVEMENT_AREA_X + SKI_ITEM_MOVEMENT_AREA_WIDTH;
			Rect.iBottom = SKI_ITEM_MOVEMENT_AREA_Y + SKI_ITEM_MOVEMENT_AREA_HEIGHT;
			fRestrictMouseToRect = TRUE;

			SOLDIERTYPE* const owner = GetMovingItemOwner();
			SetItemPointer(&gMoveingItem.ItemObject, owner);
			break;
		}

		case PLAYERS_OFFER_AREA:
		{
			INVENTORY_IN_SLOT* const o = &PlayersOfferArea[bSlotNum];
			//Get the item from the slot.
			gMoveingItem = *o;

			// if the slot is overloaded (holds more objects than we have room for valid statuses of)
			if (o->ItemObject.ubNumberOfObjects > MAX_OBJECTS_PER_SLOT )
			{
				// allow only MAX_OBJECTS_PER_SLOT of those objects to be picked up at a time
				// (sure it kind of sucks, but it's a lot easier than handling overloaded cursor objects in Interface Items!
				gMoveingItem.ItemObject.ubNumberOfObjects = MAX_OBJECTS_PER_SLOT;

				// decrease the number objects left in the slot by that much instead of deleting it
				o->ItemObject.ubNumberOfObjects -= MAX_OBJECTS_PER_SLOT;
			}
			else	// completely legal object
			{
				// NOTE: a merc-owned object can never be "overloaded", so no problem here
				IfMercOwnedRemoveItemFromMercInv( &gMoveingItem );

				//remove the item from the slot
				ClearPlayersOfferSlot( bSlotNum );
			}

			//Restrict the cursor to the players offer area and the players inventory
			Rect.iLeft = 0;//SKI_ITEM_MOVEMENT_AREA_X;
			Rect.iTop = SKI_ITEM_MOVEMENT_AREA_Y;
			Rect.iRight = SKI_ITEM_MOVEMENT_AREA_X + SKI_ITEM_MOVEMENT_AREA_WIDTH;
			Rect.iBottom = SKI_ITEM_MOVEMENT_AREA_Y + SKI_ITEM_MOVEMENT_AREA_HEIGHT;
			fRestrictMouseToRect = TRUE;

			SOLDIERTYPE* const owner = GetMovingItemOwner();
			SetItemPointer(&gMoveingItem.ItemObject, owner);
			break;
		}

		case PLAYERS_INVENTORY:
			// better be a valid merc pocket index, or -1
			Assert( ( bSlotNum >= -1 ) && ( bSlotNum < NUM_INV_SLOTS ) );

			// if we're supposed to store the original pocket #, but that pocket still holds more of these
			if ( ( bSlotNum != -1 ) && ( gpSMCurrentMerc->inv[ bSlotNum ].ubNumberOfObjects > 0 ) )
			{
				// then we can't store the pocket #, because our system can't return stacked objects
				bSlotNum = -1;
			}

			// First try to evaluate the item immediately like any other left-click on a merc's inventory slot would be.  Only
			// if that doesn't work (because there isn't enough room in the player's offer area), the item is picked up into
			// the cursor, and may then get placed into the player's offer area directly, but it will NOT get evaluated that
			// way, and so has no possibility of entering the dealer's inventory (where complex items aren't permitted).
			if ( fOfferToDealerFirst && OfferObjectToDealer( gpItemPointer, gpSMCurrentMerc->ubProfile, bSlotNum ) )
			{
				//Reset the cursor
				SetSkiCursor( CURSOR_NORMAL );
			}
			else	// not supposed to offer it, or dealer has no more room
			{
				// store the current contents of the cursor in a temporary object structure.
				// We have to do this before memsetting gMoveingItem, 'cause during swaps, gpItemPointer == &gMoveingItem.ItemObject!
				TempObject = *gpItemPointer;

				//ARM: The memset was previously commented out, in order to preserve the owning merc's inv slot # during a swap of
				// items in an inventory slot.  However, that leads to other bugs: if you picked the thing you're swapping in from
				// a restricted inv slot (headgear, vest, etc.), the item swapped out will end up belonging to an illegal slot, and
				// return there with a right click on it in the player's offer area.  So now ALL items picked up here are unowned.
				gMoveingItem = INVENTORY_IN_SLOT{};

				//Get the item from the pointer
				gMoveingItem.ItemObject = TempObject;

				gMoveingItem.fActive = TRUE;
				gMoveingItem.sItemIndex = TempObject.usItem;
				gMoveingItem.ubLocationOfObject = ubSource;

				// By necessity, these items don't belong to a slot (so you can't return them via a right click),
				// because it would be too much work to handle attachments, members of a stack, or even items swapped out of slots.
				gMoveingItem.ubIdOfMercWhoOwnsTheItem = gpSMCurrentMerc->ubProfile;
				gMoveingItem.bSlotIdInOtherLocation = bSlotNum;

				//Restrict the cursor to the players offer area and the players inventory
				Rect.iLeft = 0;//SKI_ITEM_MOVEMENT_AREA_X;
				Rect.iTop = SKI_ITEM_MOVEMENT_AREA_Y;
				Rect.iRight = SKI_ITEM_MOVEMENT_AREA_X + SKI_ITEM_MOVEMENT_AREA_WIDTH;
				Rect.iBottom = SKI_ITEM_MOVEMENT_AREA_Y + SKI_ITEM_MOVEMENT_AREA_HEIGHT;
				fRestrictMouseToRect = TRUE;

				SetItemPointer(&gMoveingItem.ItemObject, gpSMCurrentMerc);
			}

			break;
	}

	// if we have something in hand
	if ( gpItemPointer != NULL )
	{
		//make sure the soldier is not null
		Assert( gpItemPointerSoldier != NULL );

		// Set mouse
		SetSkiCursor( EXTERN_CURSOR );

		//Enable the region that limits the movement of the cursor with the item
		gSkiInventoryMovementAreaMouseRegions.Enable();

		if ( fRestrictMouseToRect )
		{
			RestrictMouseCursor( &Rect );
		}

		DisableAllDealersInventorySlots();

		if ( ubSource != ARMS_DEALER_OFFER_AREA )
		{
			DisableAllDealersOfferSlots();
		}

		gfResetShopKeepIdleQuote = TRUE;
	}

	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}


void RestrictSkiMouseCursor()
{
	SGPRect Rect;

	Rect.iLeft = 0;//SKI_ITEM_MOVEMENT_AREA_X;
	Rect.iTop = SKI_ITEM_MOVEMENT_AREA_Y;
	Rect.iRight = SKI_ITEM_MOVEMENT_AREA_X + SKI_ITEM_MOVEMENT_AREA_WIDTH;
	Rect.iBottom = SKI_ITEM_MOVEMENT_AREA_Y + SKI_ITEM_MOVEMENT_AREA_HEIGHT;

	RestrictMouseCursor( &Rect );
}


static void EnableAllDealersInventorySlots(void);
static void EnableAllDealersOfferSlots(void);


void SetSkiCursor( UINT16 usCursor )
{
	UINT8 ubCnt;

	//if we are setting up an item as a cursor
	if( usCursor == EXTERN_CURSOR )
	{
		//EnableSMPanelButtons( FALSE, FALSE );

		// if the current merc is in range
		if( !gfSMDisableForItems )
		{
			// hatch out unavailable merc inventory slots
			ReevaluateItemHatches( gpSMCurrentMerc, FALSE );
		}

		SetMouseCursorFromItem(gMoveingItem.sItemIndex);

		gSMPanelRegion.ChangeCursor(usCursor);
		gSKI_EntireScreenMouseRegions.ChangeCursor(usCursor);
		gArmsDealersDropItemToGroundMouseRegions.ChangeCursor(usCursor);
		MSYS_SetCurrentCursor( usCursor );

		//if the item desc window is up
		if( gInvDesc.uiFlags & MSYS_REGION_EXISTS )
			gInvDesc.ChangeCursor(usCursor);

		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++)
		{
			if( gItemDescAttachmentRegions[ubCnt].uiFlags & MSYS_REGION_EXISTS )
				gItemDescAttachmentRegions[ubCnt].ChangeCursor(usCursor);
		}

		for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
		{
			gPlayersOfferSlotsMouseRegions[ubCnt].ChangeCursor(usCursor);
			gPlayersOfferSlotsSmallFaceMouseRegions[ubCnt].ChangeCursor(usCursor);
			gDealersOfferSlotsMouseRegions[ubCnt].ChangeCursor(usCursor);

			//if the dealer repairs
			if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
			{
				gDealersOfferSlotsSmallFaceMouseRegions[ubCnt].ChangeCursor(usCursor);
			}
		}

		gSkiInventoryMovementAreaMouseRegions.ChangeCursor(usCursor);
	}

	//else we are restoring the old cursor
	else
	{
		gMoveingItem = INVENTORY_IN_SLOT{};

		gpItemPointer = NULL;

		DisableTacticalTeamPanelButtons( FALSE );

		//EnableSMPanelButtons( TRUE, FALSE );
		//CheckForDisabledForGiveItem( );

		// if the current merc is in range
		if( !gfSMDisableForItems )
		{
			// make all merc inventory slots available again
			ReevaluateItemHatches( gpSMCurrentMerc, TRUE );
		}

		gSMPanelRegion.ChangeCursor(usCursor);
		gSKI_EntireScreenMouseRegions.ChangeCursor(usCursor);
		gArmsDealersDropItemToGroundMouseRegions.ChangeCursor(usCursor);

		for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
		{
			gPlayersOfferSlotsMouseRegions[ubCnt].ChangeCursor(usCursor);
			gPlayersOfferSlotsSmallFaceMouseRegions[ubCnt].ChangeCursor(usCursor);
			gDealersOfferSlotsMouseRegions[ubCnt].ChangeCursor(usCursor);

			//if the dealer repairs
			if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
			{
				gDealersOfferSlotsSmallFaceMouseRegions[ubCnt].ChangeCursor(usCursor);
			}
		}

		//if the item desc window is up
		if( gInvDesc.uiFlags & MSYS_REGION_EXISTS )
			gInvDesc.ChangeCursor(usCursor);

		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++)
		{
			if( gItemDescAttachmentRegions[ubCnt].uiFlags & MSYS_REGION_EXISTS )
				gItemDescAttachmentRegions[ubCnt].ChangeCursor(usCursor);
		}

		gSkiInventoryMovementAreaMouseRegions.ChangeCursor(usCursor);

		MSYS_SetCurrentCursor( usCursor );

		SetCurrentCursorFromDatabase( usCursor );

		FreeMouseCursor();

		EnableAllDealersInventorySlots();
		EnableAllDealersOfferSlots();

		//Disable the region that limits the movement of the cursor with the item
		gSkiInventoryMovementAreaMouseRegions.Disable();
	}

	SetCurrentCursorFromDatabase( usCursor );

	// make sure disabled slot hatching gets updated when items picked up / dropped
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}


static INT8 AddInventoryToSkiLocation(const INVENTORY_IN_SLOT* pInv, UINT8 ubSpotLocation, UINT8 ubWhere)
{
	INT8 bSlotAddedTo = -1;


	switch( ubWhere )
	{
		case ARMS_DEALER_INVENTORY:
		case PLAYERS_INVENTORY:
			// not used this way
			SLOGA("AddInventoryToSkiLocation: invalid Where");
			return( bSlotAddedTo );

		case ARMS_DEALER_OFFER_AREA:
		{
			INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[ubSpotLocation];
			//If we can add the item into the slot that was clicked on
			if (!a->fActive)
			{
				*a = *pInv;
				IfMercOwnedCopyItemToMercInv( pInv );

				SetSkiRegionHelpText(    a, &gDealersOfferSlotsMouseRegions[ubSpotLocation],          ARMS_DEALER_OFFER_AREA);
				SetSkiFaceRegionHelpText(a, &gDealersOfferSlotsSmallFaceMouseRegions[ubSpotLocation], ARMS_DEALER_OFFER_AREA);

				bSlotAddedTo = ubSpotLocation;
			}
			else
			{
				bSlotAddedTo = AddItemToArmsDealerOfferArea( pInv, -1 );
			}
			break;
		}

		case PLAYERS_OFFER_AREA:
		{
			//If we can add the item into the slot that was clicked on
			INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubSpotLocation];
			if (!o->fActive)
			{
				// put it down in that player offer area slot
				*o = *pInv;
				IfMercOwnedCopyItemToMercInv( pInv );

				//if the item is money
				if (GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY)
				{
					//Since money is always evaluated
					o->uiFlags     |= ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
					o->uiItemPrice  = o->ItemObject.uiMoneyAmount;
				}

				SetSkiRegionHelpText(    o, &gPlayersOfferSlotsMouseRegions[ubSpotLocation],          PLAYERS_OFFER_AREA);
				SetSkiFaceRegionHelpText(o, &gPlayersOfferSlotsSmallFaceMouseRegions[ubSpotLocation], PLAYERS_OFFER_AREA);

				bSlotAddedTo = ubSpotLocation;
			}
			//else if( )
			//{
				//check to see the type we are adding is the same as the type that is already there
			//}
			else	// that slot is full
			{
				// add it elsewhere
				bSlotAddedTo = AddItemToPlayersOfferArea( pInv->ubIdOfMercWhoOwnsTheItem, pInv, pInv->bSlotIdInOtherLocation );
			}
			break;
		}
	}

	//Redraw the screen
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
	gfResetShopKeepIdleQuote = TRUE;
	fInterfacePanelDirty = DIRTYLEVEL2;

	return( bSlotAddedTo );
}


static void DisplayTalkingArmsDealer(void)
{
	static BOOLEAN fWasTheMercTalking= FALSE;
	//static UINT32 uiLastTime=0;
	//UINT32 uiCurTime = GetJA2Clock();
	//static UINT32 uiMinimumLengthForTalkingText;


	//Make sure the Dealers doesn't get disabled
	giShopKeeperFaceIndex->fDisabled = FALSE;

	HandleDialogue();

	//Gets handled when we render the tactical interface
	HandleAutoFaces( );

	HandleTalkingAutoFaces( );
	HandleShopKeeperDialog( 0 );

	gfIsTheShopKeeperTalking = giShopKeeperFaceIndex->fTalking;

	//if the merc just started talking
	//if( gfIsTheShopKeeperTalking && !fWasTheMercTalking )
	{

	}

	//if the merc is talking
	if( gfIsTheShopKeeperTalking )
		fWasTheMercTalking = TRUE;

	//if the merc just finished talking
	if( !gfIsTheShopKeeperTalking && fWasTheMercTalking )
	{
		RemoveShopKeeperSubTitledText();
		fWasTheMercTalking = FALSE;
	}
}


static void DealWithItemsStillOnTheTable(void);


static void HandleShopKeeperDialog(UINT8 ubInit)
{
	UINT32 uiCurTime = GetJA2Clock();

	static UINT32 uiLastTime = 0;
	static INT8   bSpeech = -1;


	if( ubInit >= 1 )
	{
		uiLastTime = GetJA2Clock();
	}

	if( ubInit == 1 )
	{
		// special: if it's Arnie, and we have stuff in for repairs, but it's not fixed yet, use a different opening quote!
		if ( ( gbSelectedArmsDealerID == ARMS_DEALER_ARNIE ) && RepairmanIsFixingItemsButNoneAreDoneYet( ARNIE ) )
		{
			bSpeech = ARNIE_QUOTE_NOT_REPAIRED_YET;
		}
		// if repairs were delayed
		if ( gfStartWithRepairsDelayedQuote )
		{
			bSpeech = FREDO_PERKO_SORRY_REPAIR_DELAYED;
			gfStartWithRepairsDelayedQuote = FALSE;

			// treat this as having done business - the player WAS here for a good reason, and it's the dealers fault...
			gfDoneBusinessThisSession = TRUE;
		}
		else
		{
			bSpeech = SK_QUOTES_PLAYER_FIRST_ENTERS_SKI;
		}
		return;
	}


	//if its the first time in
	if( bSpeech != -1 )
	{
		if( ( uiCurTime - uiLastTime ) > 800 )
		{
			StartShopKeeperTalking( bSpeech );
			bSpeech = -1;
			uiLastTime = GetJA2Clock();
		}
	}


	//Handle Idle converstions
	if( gfIsTheShopKeeperTalking )
	{
		uiLastTime = uiCurTime;
	}
	//if the player has requested to leave
	else if( gfUserHasRequestedToLeave && ( giShopKeepDialogueEventinProgress == -1 ) )
	{
		// to see if the player has finished talking
		if(!giShopKeeperFaceIndex->fTalking)
		{
			class DialogueEventShopkeeperExit : public DialogueEvent
			{
				public:
					bool Execute()
					{
						gfSKIScreenExit = TRUE;
						return false;
					}
			};

			DialogueEvent::Add(new DialogueEventShopkeeperExit());
		}

		DealWithItemsStillOnTheTable();
	}
	else
	{
		// Determine if the shopkeeper should say a quote
		if( ( uiCurTime - uiLastTime ) > ( guiRandomQuoteDelayTime + Random( guiRandomQuoteDelayTime ) ) )
		{
			//Only say it occasionally
			if( Chance ( 35 ) )
			{
				INT16 sRandomQuoteToUse = -1;

				// first check if one of the situation warrants one of the more precise quotes

				//if the dealer repairs
				if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
				{
					// if there are items in the arms dealer's offer area (must be awaiting repairs)
					if ( CountNumberOfItemsInTheArmsDealersOfferArea() > 0)
					{
						sRandomQuoteToUse = SK_QUOTES_RANDOM_QUOTE_WHILE_ITEMS_CHOSEN_TO_SELL_OR_REPAIR;
					}
				}
				else	// non-repairman
				{
					// if there is any items in the player's area with value to this dealer other than just money
					if ( ( CalculateTotalPlayersValue() - CalculateHowMuchMoneyIsInPlayersOfferArea( ) ) > 0 )
					{
						// if the player has also selected some items for purchase
						if ( CountNumberOfItemsInTheArmsDealersOfferArea() > 0 )
						{
							sRandomQuoteToUse = SK_QUOTES_RANDOM_QUOTE_WHILE_ITEMS_CHOSEN_TO_TRADE;
						}
						else
						{
							sRandomQuoteToUse = SK_QUOTES_RANDOM_QUOTE_WHILE_ITEMS_CHOSEN_TO_SELL_OR_REPAIR;
						}
					}
				}

				// if neither of the more precise quotes fit, or 33 percent of the time anyways
				if ( ( sRandomQuoteToUse == -1 ) || Chance( 33 ) )
				{
					if( Chance ( 50 ) )
						sRandomQuoteToUse = SK_QUOTES_RANDOM_QUOTE_WHILE_PLAYER_DECIDING_1;
					else
						sRandomQuoteToUse = SK_QUOTES_RANDOM_QUOTE_WHILE_PLAYER_DECIDING_2;
				}

				Assert( sRandomQuoteToUse != -1 );
				Assert( sRandomQuoteToUse < NUM_COMMON_SK_QUOTES );

				if ( !gfCommonQuoteUsedThisSession[ sRandomQuoteToUse ] )
				{
					StartShopKeeperTalking( (UINT16) sRandomQuoteToUse );

					gfCommonQuoteUsedThisSession[ sRandomQuoteToUse ] = TRUE;

					//increase the random quote delay
					guiRandomQuoteDelayTime += SKI_DEALERS_RANDOM_QUOTE_DELAY_INCREASE_RATE;
				}
			}

			uiLastTime = GetJA2Clock();
		}
	}
}


static BOOLEAN StartShopKeeperTalking(UINT16 usQuoteNum)
{
	// if already in the process of leaving, don't start any additional quotes
	if ( gfSKIScreenExit || gfRemindedPlayerToPickUpHisStuff || gfUserHasRequestedToLeave )
	{
		return( FALSE );
	}

	class DialogueEventShopkeeperSetQuote : public DialogueEvent
	{
		public:
			DialogueEventShopkeeperSetQuote(INT32 const quote) : quote_(quote) {}

			bool Execute()
			{
				giShopKeepDialogueEventinProgress = quote_;
				return false;
			}

		private:
			INT32 const quote_;
	};

	// post event to mark shopkeeper dialogue in progress
	DialogueEvent::Add(new DialogueEventShopkeeperSetQuote(usQuoteNum));

	// post quote dialogue
	CharacterDialogue(SelectedArmsDealer()->profileID, usQuoteNum, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI, FALSE);

	// post event to mark shopkeeper dialogue as ended
	DialogueEvent::Add(new DialogueEventShopkeeperSetQuote(-1));

	gfResetShopKeepIdleQuote = TRUE;
	return( TRUE );
}


static bool IsGunOrAmmoOfSameTypeSelected(OBJECTTYPE const& o)
{
	if (!gpHighLightedItemObject) return false; // No item selected
	OBJECTTYPE const& highlighted_o    = *gpHighLightedItemObject;
	const ItemModel * highlighted_item = GCM->getItem(highlighted_o.usItem);
	const ItemModel * o_item = GCM->getItem(o.usItem);

	// Is one ammo for the other?
	if (highlighted_item->getItemClass() == IC_AMMO)
	{
		if (o_item->getItemClass() == IC_GUN &&
				GCM->getWeapon(o.usItem)->matches(highlighted_item->asAmmo()->calibre))
		{
			return true;
		}
	}
	else if (highlighted_item->getItemClass() == IC_GUN)
	{
		if (o_item->getItemClass() == IC_AMMO &&
				GCM->getWeapon(highlighted_o.usItem)->matches(o_item->asAmmo()->calibre))
		{
			return true;
		}
	}

	// Is one an attachment for the other?
	if (o_item->getFlags() & ITEM_ATTACHMENT)
	{
		if (ValidAttachment(o.usItem, highlighted_o.usItem)) return true;
	}
	else
	{
		if (ValidAttachment(highlighted_o.usItem, o.usItem)) return true;
	}

	return false;
}


static void ShopKeeperSubTitleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


void InitShopKeeperSubTitledText(const ST::string& str)
{
	//Now setup the popup box
	if( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
	{
		UINT16 usActualWidth=0;
		UINT16 usActualHeight=0;

		// The subutitled text for what the merc is saying
		ST::string ShopKeeperTalkingText = ST::format("\"{}\"", str);
		g_popup_box = PrepareMercPopupBox(g_popup_box, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, ShopKeeperTalkingText, 300, 0, 0, 0, &usActualWidth, &usActualHeight);

		//position it to start under the guys face
		gusPositionOfSubTitlesX = 13;

		RenderMercPopUpBox(g_popup_box, gusPositionOfSubTitlesX, SKI_POSITION_SUBTITLES_Y, FRAME_BUFFER);

		//check to make sure the region is not already initialized
		if( !( gShopKeeperSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS ) )
		{
			MSYS_DefineRegion(&gShopKeeperSubTitleMouseRegion, gusPositionOfSubTitlesX,
						SKI_POSITION_SUBTITLES_Y,
						(INT16)(gusPositionOfSubTitlesX + usActualWidth),
						(INT16)(SKI_POSITION_SUBTITLES_Y + usActualHeight),
						MSYS_PRIORITY_HIGH,
						CURSOR_NORMAL, MSYS_NO_CALLBACK,
						ShopKeeperSubTitleRegionCallBack);
		}

		//redraw the screen
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

	}
}


static void RemoveShopKeeperSubTitledText(void)
{
	if (!g_popup_box) return;

	RemoveMercPopupBox(g_popup_box);
	g_popup_box = 0;

	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

	//Get rid of the subtitles region
	if( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
		MSYS_RemoveRegion( &gShopKeeperSubTitleMouseRegion );
}


static bool AreThereItemsInTheArmsDealersOfferArea()
{
	FOR_EACH(INVENTORY_IN_SLOT const, i, ArmsDealerOfferArea)
	{
		if (i->fActive) return true;
	}
	return false;
}


static bool AreThereItemsInThePlayersOfferArea()
{
	FOR_EACH(INVENTORY_IN_SLOT const, i, PlayersOfferArea)
	{
		if (i->fActive) return true;
	}
	return false;
}


//Mouse Call back for the Arms traders inventory slot
static void ShopKeeperSubTitleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP || iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		ShutUpShopKeeper();
	}
}


//Mouse Call back for the Arms delaers face
static void SelectArmsDealersFaceRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP || iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		ShutUpShopKeeper();
	}
}


static void ShutUpShopKeeper(void)
{
	//RemoveShopKeeperSubTitledText();

	ShutupaYoFace( giShopKeeperFaceIndex );
	gfIsTheShopKeeperTalking = FALSE;

	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}


static UINT8 CountNumberOfValuelessItemsInThePlayersOfferArea()
{
	UINT8 n = 0;
	FOR_EACH(INVENTORY_IN_SLOT const, i, PlayersOfferArea)
	{
		INVENTORY_IN_SLOT const& o = *i;
		if (!o.fActive)                                  continue;
		if (o.uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE) continue;
		++n;
	}
	return n;
}


static UINT8 CountNumberOfItemsInThePlayersOfferArea()
{
	UINT8 n = 0;
	FOR_EACH(INVENTORY_IN_SLOT const, i, PlayersOfferArea)
	{
		if (i->fActive) ++n;
	}
	return n;
}


static UINT8 CountNumberOfItemsInTheArmsDealersOfferArea()
{
	UINT8 n = 0;
	FOR_EACH(INVENTORY_IN_SLOT const, i, ArmsDealerOfferArea)
	{
		if (i->fActive) ++n;
	}
	return n;
}


static INT8 GetSlotNumberForMerc(UINT8 ubProfile)
{
	INT8 bCnt;

	for( bCnt = 0; bCnt < gubNumberMercsInArray; bCnt++ )
	{
		if( gubArrayOfEmployedMercs[ bCnt ] == ubProfile )
			return( bCnt );
	}

	// not found - not currently working for the player
	return( -1 );
}


static void EnableDisableDealersInventoryPageButtons(void)
{
	//if we are on the first page, disable the page up arrow
	EnableButton(guiSKI_InvPageUpButton, gSelectArmsDealerInfo.ubCurrentPage > 1);

	//if we are on the last page, disable the page down button
	EnableButton(guiSKI_InvPageDownButton, gSelectArmsDealerInfo.ubCurrentPage != gSelectArmsDealerInfo.ubNumberOfPages);
}


static void EnableDisableEvaluateAndTransactionButtons(void)
{
	UINT8   ubCnt;
	BOOLEAN fItemEvaluated=FALSE;
	UINT32  uiArmsDealerTotalCost = CalculateTotalArmsDealerCost();
	UINT32  uiPlayersOfferAreaTotalCost = CalculateTotalPlayersValue();
	UINT32  uiPlayersOfferAreaTotalMoney = CalculateHowMuchMoneyIsInPlayersOfferArea( );


	//loop through the players offer area
	//loop through all the items in the players offer area and determine if they can be sold here.
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubCnt];
		//if there is an item here
		if (o->fActive)
		{
			//if the item has value
			if (o->uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE)
			{
				//if the item isnt money ( which is always evaluated )
				if (GCM->getItem(o->sItemIndex)->getItemClass() != IC_MONEY)
				{
					fItemEvaluated = TRUE;
				}
				else if (!DoesDealerDoRepairs(gbSelectedArmsDealerID) && GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY)
				{
					//else if it is not a repair dealer, and the item is money
					fItemEvaluated = TRUE;
				}
			}
		}

		//if the dealer is a repair dealer
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			//if there is an item here, the item has to have been evaluated
			if( ArmsDealerOfferArea[ ubCnt ].fActive )
			{
				fItemEvaluated = TRUE;
			}
		}
	}

	//if there are evaluated items here
	if( fItemEvaluated )
	{
		////and if the players offer area value exceeds the shopkeeper's area
		//if( uiPlayersOfferAreaTotalCost >= uiArmsDealerTotalCost )
			EnableButton( guiSKI_TransactionButton );
		//else
		//	DisableButton( guiSKI_TransactionButton );

		/*
		//if the only item in the players offer area is money, and there is nothing in the dealers offer area
		if( IsMoneyTheOnlyItemInThePlayersOfferArea( ) && uiArmsDealerTotalCost == 0 )
			DisableButton( guiSKI_TransactionButton );*/

		//If its a repair dealer, and there is no items in the Dealer Offer area,
		if(DoesDealerDoRepairs(gbSelectedArmsDealerID)&&
				CountNumberOfItemsInTheArmsDealersOfferArea( ) == 0 &&
				uiPlayersOfferAreaTotalMoney < uiArmsDealerTotalCost
			)
			DisableButton( guiSKI_TransactionButton );
	}
	//else if there is
	else
	{
		EnableButton(guiSKI_TransactionButton, uiArmsDealerTotalCost != 0);
	}


	if( uiArmsDealerTotalCost > uiPlayersOfferAreaTotalCost + LaptopSaveInfo.iCurrentBalance )
	{
		DisableButton( guiSKI_TransactionButton );
	}
	/* Allow transaction attempt when dealer can't afford to buy that much - he'll reject it with a special quote!
	else if( ( gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash + uiArmsDealerTotalCost ) < ( uiPlayersOfferAreaTotalCost - uiPlayersOfferAreaTotalMoney ) )
	{
		DisableButton( guiSKI_TransactionButton );
	}*/
	else if( ( uiPlayersOfferAreaTotalCost == uiPlayersOfferAreaTotalMoney ) && ( uiPlayersOfferAreaTotalMoney > 0 ) && ( uiArmsDealerTotalCost == 0 ) )
	{
		DisableButton( guiSKI_TransactionButton );
	}


	//if the player is currently moving an item, disable the transaction button
	if( gMoveingItem.sItemIndex != 0 )
	{
		DisableButton( guiSKI_TransactionButton );
		DisableButton( guiSKI_DoneButton );
	}
	else
	{
		EnableButton( guiSKI_DoneButton );
	}


	//ARM: Always permit trying bribes, even if they don't work on a given dealer!
	// if the arms dealer is the kind of person who accepts gifts, and there is stuff to take
	//if( SelectedArmsDealer()->hasFlag(ArmsDealerFlag::ACCEPTS_GIFTS )
	{
		//if the player is giving the dealer money, without buying anything
		if( IsMoneyTheOnlyItemInThePlayersOfferArea( ) && CountNumberOfItemsInTheArmsDealersOfferArea( ) == 0 )
		{
			EnableButton( guiSKI_TransactionButton );
		}
	}

	if( gfUserHasRequestedToLeave )
		DisableButton( guiSKI_TransactionButton );


	//if the player is in the item desc for the arms dealer items
	if( InItemDescriptionBox( ) && pShopKeeperItemDescObject != NULL )
	{
		guiSKI_TransactionButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
		guiSKI_DoneButton->uiFlags        |= BUTTON_FORCE_UNDIRTY;
	}
	else
	{
		guiSKI_TransactionButton->uiFlags &= ~BUTTON_FORCE_UNDIRTY;
		guiSKI_DoneButton->uiFlags        &= ~BUTTON_FORCE_UNDIRTY;
	}
}


void AddItemToPlayersOfferAreaAfterShopKeeperOpen( OBJECTTYPE *pItemObject, INT8 bPreviousInvPos )
{
	gItemToAdd.fActive = TRUE;
	gItemToAdd.ItemObject = *pItemObject;
	gItemToAdd.bPreviousInvPos	= bPreviousInvPos;
}


static BOOLEAN IsMoneyTheOnlyItemInThePlayersOfferArea(void)
{
	UINT8   ubCnt;
	BOOLEAN fFoundMoney = FALSE;

	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubCnt];
		//if there is an item here
		if (o->fActive)
		{
			if (GCM->getItem(o->sItemIndex)->getItemClass() != IC_MONEY) return FALSE;
			fFoundMoney = TRUE;
		}
	}

	Assert(!fFoundMoney || CalculateHowMuchMoneyIsInPlayersOfferArea() > 0);

	// only return TRUE if there IS money in the POA
	return( fFoundMoney );
}


/*

void MoveRepairEvaluatedPlayerOfferedItemsToArmsDealersOfferArea()
{
	UINT32	uiCnt;

	//loop through all the slots in the players offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		//if there is an item here
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[uiCnt];
		if (o->uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE)
		{
			AddItemToArmsDealerOfferArea(o, -1);
		}
	}
}

*/


static UINT32 CalculateHowMuchMoneyIsInPlayersOfferArea()
{
	UINT32 total = 0;
	FOR_EACH(INVENTORY_IN_SLOT const, i, PlayersOfferArea)
	{
		INVENTORY_IN_SLOT const& o = *i;
		if (!o.fActive)                                 continue;
		if (GCM->getItem(o.sItemIndex)->getItemClass() != IC_MONEY) continue;
		total += o.ItemObject.uiMoneyAmount;
	}
	return total;
}


static void MovePlayersItemsToBeRepairedToArmsDealersInventory(void)
{
	//for all items in the dealers items offer area
	UINT32	uiCnt;

	//loop through all the slots in the shopkeeper's offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[uiCnt];
		//if there is a item here
		if (a->fActive)
		{
			// NOTE:  Any items that make it into a repairman's dealer offer area are guaranteed to be:
			//   a) Repairable
			//   b) Actually damaged
			//   c) Already stripped of all attachments
			//   d) If a gun, stripped of any non-ammo-class GunAmmoItems, and bullets

			// add it to the arms dealer's inventory
			GiveObjectToArmsDealerForRepair(gbSelectedArmsDealerID, &a->ItemObject, a->ubIdOfMercWhoOwnsTheItem);
			a->sSpecialItemElement = gubLastSpecialItemAddedAtElement;

			//Remove the item from the owner merc's inventory
			IfMercOwnedRemoveItemFromMercInv(a);

			//erase the item from the dealer's offer area
			ClearArmsDealerOfferSlot( uiCnt );
		}
	}
	gfResetShopKeepIdleQuote = TRUE;
}


static BOOLEAN RemoveRepairItemFromDealersOfferArea(INT8 bSlot)
{
	INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[bSlot];
	//if the item doesn't have a duplicate copy in its owner merc's inventory slot
	if (a->bSlotIdInOtherLocation == -1)
	{
		if (!SKITryToReturnInvToOwnerOrCurrentMerc(a))
		{
			//failed to add item, inventory probably filled up or item is unowned and current merc ineligible
			return( FALSE );
		}
	}

	// Clear the contents of the dealer's offer slot and its help text
	ClearArmsDealerOfferSlot( bSlot );

	// Dirty
	fInterfacePanelDirty = DIRTYLEVEL2;
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
	return( TRUE );
}


static INT8 GetInvSlotOfUnfullMoneyInMercInventory(SOLDIERTYPE* pSoldier)
{
	UINT8	ubCnt;

	//loop through the soldier's inventory
	for( ubCnt=0; ubCnt < NUM_INV_SLOTS; ubCnt++)
	{
		// Look for MONEY only, not Gold or Silver!!!  And look for a slot not already full
		if( ( pSoldier->inv[ ubCnt ].usItem == MONEY ) && ( pSoldier->inv[ ubCnt ].uiMoneyAmount < MoneySlotLimit( ubCnt ) ) )
		{
			return( ubCnt );
		}
	}
	return( - 1 );
}


static void ClearArmsDealerOfferSlot(INT32 ubSlotToClear)
{
	// Clear the contents
	ArmsDealerOfferArea[ ubSlotToClear ] = INVENTORY_IN_SLOT{};

	//Remove the mouse help text from the region
	gDealersOfferSlotsMouseRegions[ubSlotToClear].SetFastHelpText(ST::null);

	//if the dealer repairs
	if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
	{
		gDealersOfferSlotsSmallFaceMouseRegions[ubSlotToClear].SetFastHelpText(ST::null);
	}
}


static void CheckAndHandleClearingOfPlayerOfferArea(void);


static void ClearPlayersOfferSlot(INT32 ubSlotToClear)
{
	// Clear the contents
	PlayersOfferArea[ ubSlotToClear ] = INVENTORY_IN_SLOT{};

	//Clear the text for the item
	gPlayersOfferSlotsMouseRegions[ubSlotToClear].SetFastHelpText(ST::null);
	gPlayersOfferSlotsSmallFaceMouseRegions[ubSlotToClear].SetFastHelpText(ST::null);

	// if the player offer area is clear, reset flags for transaction
	CheckAndHandleClearingOfPlayerOfferArea( );
}


static BOOLEAN CanShopkeeperOverrideDialogue(void);


static void EvaluateItemAddedToPlayersOfferArea(INT8 bSlotID, BOOLEAN fFirstOne)
{
	UINT32  uiEvalResult = EVAL_RESULT_NORMAL;
	BOOLEAN fRocketRifleWasEvaluated = FALSE;
	UINT8   ubNumberOfItemsAddedToRepairDuringThisEvaluation=0;

	INVENTORY_IN_SLOT* const o = &PlayersOfferArea[bSlotID];

	// there better be an item there
	Assert(o->fActive);

	//if money is the item being evaluated, leave
	if (GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY) return;

	// if already evaluated, don't do it again
	if (o->uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED) return;

	// say "Hmm... Let's see" once per trading session to start evaluation
	// SPECIAL: Devin doesn't have this quote (he's the only one)
	if( !gfDealerHasSaidTheEvaluateQuoteOnceThisSession && ( gbSelectedArmsDealerID != ARMS_DEALER_DEVIN ) )
	{
		gfDealerHasSaidTheEvaluateQuoteOnceThisSession = TRUE;
		StartShopKeeperTalking( SK_QUOTES_PLAYER_REQUESTED_EVALUATION );
	}


	//Can this particular kind of item be sold/repaired here
	if (!WillShopKeeperRejectObjectsFromPlayer(gbSelectedArmsDealerID, bSlotID))
	{
		//if the dealer repairs
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			UINT32 uiNumberOfItemsInForRepairs = CountTotalItemsRepairDealerHasInForRepairs( gbSelectedArmsDealerID );
			UINT32 uiNumberOfItemsAlreadyEvaluated = CountNumberOfItemsInTheArmsDealersOfferArea();

			//Get the number of items being evaluated
			ubNumberOfItemsAddedToRepairDuringThisEvaluation = o->ItemObject.ubNumberOfObjects;

			//if there is already enough items in for repairs, complain about it and DON'T accept the item for repairs
			if( ( uiNumberOfItemsAlreadyEvaluated + ubNumberOfItemsAddedToRepairDuringThisEvaluation + uiNumberOfItemsInForRepairs ) > SKI_MAX_AMOUNT_OF_ITEMS_DEALER_CAN_REPAIR_AT_A_TIME )
			{
				if ( !gfAlreadySaidTooMuchToRepair )
				{
					//Start the dealer talking
					StartShopKeeperTalking( SK_QUOTES_CANT_AFFORD_TO_BUY_OR_TOO_MUCH_TO_REPAIR );
					gfAlreadySaidTooMuchToRepair = TRUE;
				}
				return;
			}

			//if the item is a rocket rifle
			if (ItemIsARocketRifle(o->sItemIndex)) fRocketRifleWasEvaluated = TRUE;

			//if the item is damaged, or is a rocket rifle (which always "need repairing" even at 100%, to reset imprinting)
			if (o->ItemObject.bStatus[0] < 100 || fRocketRifleWasEvaluated)
			{
				INT8	bSlotAddedTo;

				// Move the item to the Dealer's Offer Area
				bSlotAddedTo = AddItemToArmsDealerOfferArea(o, o->bSlotIdInOtherLocation);

				if( bSlotAddedTo != -1 )
				{
					// Clear the contents
					ClearPlayersOfferSlot( bSlotID );

					INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[bSlotAddedTo];

					/* ARM: Leave it there, until transaction occurs it should be recallable
					//Remove the item from the owner merc's inventory
					IfMercOwnedRemoveItemFromMercInv(a);*/

					//Mark the item as unselected, signifying that it can be moved
					a->uiFlags &= ~ARMS_INV_ITEM_SELECTED;

					//increment the number of items being added
					ubNumberOfItemsAddedToRepairDuringThisEvaluation++;

					// check if the item is really badly damaged
					if (GCM->getItem(a->sItemIndex)->getItemClass() != IC_AMMO &&
						a->ItemObject.bStatus[0] < REALLY_BADLY_DAMAGED_THRESHOLD)
					{
						uiEvalResult = EVAL_RESULT_OK_BUT_REALLY_DAMAGED;
					}

					// check if it's the first time a rocket rifle is being submitted to the electronics guy
					if (fRocketRifleWasEvaluated && GetDealer(gbSelectedArmsDealerID)->hasFlag(ArmsDealerFlag::REPAIRS_ELECTRONICS))
					{
						//if he hasn't yet said his quote
						if( !( gArmsDealerStatus[ gbSelectedArmsDealerID ].ubSpecificDealerFlags & ARMS_DEALER_FLAG__FREDO_HAS_SAID_ROCKET_RIFLE_QUOTE ) )
						{
							// use this special evaluation result instead (has a unique quote)
							uiEvalResult = EVAL_RESULT_ROCKET_RIFLE;
						}
					}
				}
				else
				{
					SLOGW("Failed to add repair item to ArmsDealerOfferArea.");
					return;
				}
			}
			else
			{
				uiEvalResult = EVAL_RESULT_NOT_DAMAGED;
			}
		}
		else // not a repairman
		{
			uiEvalResult = EvaluateInvSlot(o);
		}
	}
	else // dealer doesn't handle this type of object
	{
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			// only otherwise repairable items count as actual rejections
			if (GCM->getItem(o->sItemIndex)->getFlags() & ITEM_REPAIRABLE)
			{
				uiEvalResult = EVAL_RESULT_DONT_HANDLE;
			}
			else
			{
				uiEvalResult = EVAL_RESULT_NON_REPAIRABLE;
			}
		}
		else
		{
			uiEvalResult = EVAL_RESULT_DONT_HANDLE;
		}
	}

	// mark this item as having been evaluated
	o->uiFlags |= ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED;

	// when evaluating complex items that get split into multiple subobjects, dealer
	// will talk only about the first one!
	// don't bother with any of this if shopkeeper can't talk right now
	if ( fFirstOne && CanShopkeeperOverrideDialogue( ) )
	{
		INT16 sQuoteNum = -1;
		UINT8 ubChanceToSayNormalQuote;


		switch ( uiEvalResult )
		{
			case EVAL_RESULT_DONT_HANDLE:
				if( SelectedArmsDealer()->type == ARMS_DEALER_SELLS_ONLY )
				{
					// then he doesn't have quotes 17, 19, or 20, always use 4.  Devin doesn't have 18 either,
					// while the text of 18 seems wrong for Sam & Howard if offered something they should consider valuable.
					sQuoteNum = SK_QUOTES_NOT_INTERESTED_IN_THIS_ITEM;
				}
				else
				{
					// he accepts items, but not this one
					sQuoteNum = SK_QUOTES_DURING_EVALUATION_STUFF_REJECTED;
				}
				break;

			case EVAL_RESULT_NON_REPAIRABLE:
				// use quote 4 for this situation...  It's a bit weird but better than "that looks fine / ain't broke"
				sQuoteNum = SK_QUOTES_NOT_INTERESTED_IN_THIS_ITEM;
				break;

			case EVAL_RESULT_NOT_DAMAGED:
			case EVAL_RESULT_WORTHLESS:
				sQuoteNum = SK_QUOTES_EVALUATION_RESULT_VALUE_OF_ZERO;
				break;

			case EVAL_RESULT_OK_BUT_REALLY_DAMAGED:
				sQuoteNum = SK_QUOTES_EVALUATION_RESULT_SOME_REALLY_DAMAGED_ITEMS;
				break;

			case EVAL_RESULT_ROCKET_RIFLE:
				sQuoteNum = FREDO_QUOTE_CAN_RESET_IMPRINTING;
				break;

			case EVAL_RESULT_NORMAL:
				//if it has been a long time since saying the last quote
				if( guiLastTimeDealerSaidNormalEvaluationQuote == 0 )
					ubChanceToSayNormalQuote = 100;
				else if( ( GetJA2Clock() - guiLastTimeDealerSaidNormalEvaluationQuote ) > 25000 )
					ubChanceToSayNormalQuote = 60;
				else if( ( GetJA2Clock() - guiLastTimeDealerSaidNormalEvaluationQuote ) > 12000 )
					ubChanceToSayNormalQuote = 25;
				else if( ( GetJA2Clock() - guiLastTimeDealerSaidNormalEvaluationQuote ) > 7000 )
					ubChanceToSayNormalQuote = 10;
				else
					ubChanceToSayNormalQuote = 0;

				if( Chance( ubChanceToSayNormalQuote ) )
				{
					sQuoteNum = SK_QUOTES_EVALUATION_RESULT_NORMAL;
				}
				break;

			default:
				SLOGW("Invalid evaluation result of %d.", uiEvalResult );
				break;
		}

		// if a valid quote was selected
		if ( sQuoteNum != -1 )
		{
			// if the appropriate quote hasn't already been used during this evaluation cycle
			if ( !gfEvalResultQuoteSaid[ uiEvalResult ] )
			{
				// try to say it
				if ( StartShopKeeperTalking( ( UINT16 ) sQuoteNum ) )
				{
					// set flag to keep from repeating it
					gfEvalResultQuoteSaid[ uiEvalResult ] = TRUE;

					// if it's the normal result
					if ( uiEvalResult == EVAL_RESULT_NORMAL )
					{
						// set new time of last usage
						guiLastTimeDealerSaidNormalEvaluationQuote = GetJA2Clock();
					}

					if ( sQuoteNum == FREDO_QUOTE_CAN_RESET_IMPRINTING )
					{
						//Set the fact that we have said this (to prevent that quote from coming up again)
						gArmsDealerStatus[ gbSelectedArmsDealerID ].ubSpecificDealerFlags |= ARMS_DEALER_FLAG__FREDO_HAS_SAID_ROCKET_RIFLE_QUOTE;
					}
				}
			}
		}
	}
}


void DoSkiMessageBox(const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags ubFlags, MSGBOX_CALLBACK ReturnCallback)
{
	// reset exit mode
	gfExitSKIDueToMessageBox = TRUE;

	// do message box and return
	SGPBox const centering_rect = { 0, 0, SCREEN_WIDTH, 339 };
	DoMessageBox(MSG_BOX_BASIC_STYLE, str, uiExitScreen, ubFlags, ReturnCallback, &centering_rect);
}


void ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// simply redraw the panel
	if( bExitValue == MSG_BOX_RETURN_OK )
	{
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
	}

	// done, re-enable calls to PerformTransaction()
	gfPerformTransactionInProgress = FALSE;
}



void ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	// yes, deduct the money
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		UINT32 uiPlayersOfferAreaValue = CalculateTotalPlayersValue();
		UINT32 uiArmsDealersItemsCost = CalculateTotalArmsDealerCost();
		INT32  iMoneyToDeduct = (INT32)( uiArmsDealersItemsCost - uiPlayersOfferAreaValue );

		//Perform the transaction with the extra money from the players account
		PerformTransaction( iMoneyToDeduct );

		AddTransactionToPlayersBook( PURCHASED_ITEM_FROM_DEALER, SelectedArmsDealer()->profileID, GetWorldTotalMin(), -iMoneyToDeduct );
	}

	// done, re-enable calls to PerformTransaction()
	gfPerformTransactionInProgress = FALSE;

	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}


// run through what the player has on the table and see if the shop keep will accept it or not
static BOOLEAN WillShopKeeperRejectObjectsFromPlayer(INT8 bDealerId, INT8 bSlotId)
{
	BOOLEAN fRejected = TRUE;

	const INVENTORY_IN_SLOT* const o = &PlayersOfferArea[bSlotId];
	if (GCM->getItem(o->sItemIndex)->getItemClass() == IC_MONEY)
	{
		fRejected = FALSE;
	}
	else if (CanDealerTransactItem(gbSelectedArmsDealerID, o->sItemIndex, TRUE))
	{
		fRejected = FALSE;
	}

	else
	{
		fRejected = TRUE;
	}

	return( fRejected );
}


static void CheckAndHandleClearingOfPlayerOfferArea(void)
{
	INT32 iCounter = 0;
	BOOLEAN fActiveSlot = FALSE;

	// find out if all the player trade slots/offer area is empty
	for( iCounter=0; iCounter<SKI_NUM_TRADING_INV_SLOTS; iCounter++)
	{
		if (PlayersOfferArea[iCounter].fActive)
		{
			// nope, there is an active slot
			fActiveSlot = TRUE;
		}
	}

	// if all player's offer area slots are empty
	if (!fActiveSlot)
	{
		ResetAllQuoteSaidFlags();
		gfRemindedPlayerToPickUpHisStuff = FALSE;
	}
}


static BOOLEAN CanShopkeeperOverrideDialogue(void)
{
	// if he's not currently saying anything
	if( !gfIsTheShopKeeperTalking && ( giShopKeepDialogueEventinProgress == -1 ) )
	{
		return( TRUE );
	}

	// if the quote he is currently saying is unimportant (fluff),
	// then shut him up & allow the override
	if( giShopKeepDialogueEventinProgress < SK_QUOTES_PLAYER_REQUESTED_EVALUATION )
	{
		// shutup the shop keep and do this quote
		ShutUpShopKeeper( );
		return( TRUE );
	}

	// he's currently saying something important
	return( FALSE );
}


static void CrossOutUnwantedItems(void)
{
	INT8  bSlotId = 0;
	INT16 sBoxStartX = 0, sBoxStartY = 0;
	INT16 sBoxWidth = 0, sBoxHeight = 0;

	// get the box height and width
	sBoxWidth  = SKI_INV_SLOT_WIDTH;
	sBoxHeight = SKI_INV_SLOT_HEIGHT;

	for( bSlotId = 0; bSlotId < SKI_NUM_TRADING_INV_SLOTS; bSlotId++ )
	{
		const INVENTORY_IN_SLOT* const o = &PlayersOfferArea[bSlotId];
		// now run through what's on the players offer area
		if (o->fActive)
		{
			// skip purchased items!
			if (!(o->uiFlags & ARMS_INV_JUST_PURCHASED))
			{
				//If item can't be sold here, or it's completely worthless (very cheap / very broken)
				if (WillShopKeeperRejectObjectsFromPlayer(gbSelectedArmsDealerID, bSlotId) ||
						!(o->uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE))
				{
					// get x and y positions
					sBoxStartX = SKI_PLAYERS_TRADING_INV_X + ( bSlotId % SKI_NUM_TRADING_INV_COLS ) * ( SKI_INV_OFFSET_X );
					sBoxStartY = SKI_PLAYERS_TRADING_INV_Y + ( bSlotId / SKI_NUM_TRADING_INV_COLS ) * ( SKI_INV_OFFSET_Y );

					BltVideoObject(FRAME_BUFFER, guiItemCrossOut, 0, sBoxStartX + 22, sBoxStartY);

					// invalidate the region
					InvalidateRegion(sBoxStartX - 1, sBoxStartY - 1, sBoxStartX + sBoxWidth + 1, sBoxStartY + sBoxHeight + 1 );
				}
			}
		}
	}
}


static void HandleCheckIfEnoughOnTheTable(void)
{
	static INT32 iLastTime = 0;
	INT32  iDifference = 0, iRand = 0;
	UINT32 uiPlayersOfferAreaValue = CalculateTotalPlayersValue();
	UINT32 uiArmsDealersItemsCost = CalculateTotalArmsDealerCost();

	if( ( iLastTime == 0 ) || gfResetShopKeepIdleQuote )
	{
		iLastTime = GetJA2Clock();
		gfResetShopKeepIdleQuote = FALSE;
	}

	iDifference = GetJA2Clock() - iLastTime;

	iRand = Random( 100 );

	// delay for shopkeeper passed?
	if( iDifference > DELAY_FOR_SHOPKEEPER_IDLE_QUOTE )
	{
		// random chance enough?
		if( iRand > CHANCE_FOR_SHOPKEEPER_IDLE_QUOTE )
		{
			// is there enough on the table
			if( ( uiArmsDealersItemsCost > uiPlayersOfferAreaValue ) && ( uiPlayersOfferAreaValue ) )
			{
				StartShopKeeperTalking( SK_QUOTES_EVAULATION_PLAYER_DOESNT_HAVE_ENOUGH_VALUE );
			}
		}

		gfResetShopKeepIdleQuote = TRUE;
	}
}


static void InitShopKeeperItemDescBox(OBJECTTYPE* pObject, UINT8 ubPocket, UINT8 ubFromLocation)
{
	INT16 sPosX, sPosY;


	switch( ubFromLocation )
	{
		case ARMS_DEALER_INVENTORY:
		{
			UINT8	ubSelectedInvSlot = ubPocket - gSelectArmsDealerInfo.ubFirstItemIndexOnPage;

			sPosX = SKI_ARMS_DEALERS_INV_START_X + ( SKI_INV_OFFSET_X * ( ubSelectedInvSlot % SKI_NUM_ARMS_DEALERS_INV_COLS ) - ( 358 / 2 ) ) + SKI_INV_SLOT_WIDTH / 2;

			sPosY = SKI_ARMS_DEALERS_INV_START_Y + ( ( SKI_INV_OFFSET_Y * ubSelectedInvSlot / SKI_NUM_ARMS_DEALERS_INV_COLS ) + 1 ) - ( 128 / 2 ) + SKI_INV_SLOT_HEIGHT / 2;

			//if the start position + the height of the box is off the screen, reposition
			if( sPosY < 0 )
				sPosY = 0;


			//if the start position + the width of the box is off the screen, reposition
			if (sPosX + 358 > SCREEN_WIDTH)
				sPosX = SCREEN_WIDTH - 358 - 5;

			//if it is starting to far to the left
			else if( sPosX < 0 )
				sPosX = 0;

			//if the box will appear over the mercs face, move the box over so it doesn't obstruct the face
			if( sPosY < SKI_FACE_Y + SKI_FACE_HEIGHT + 20 )
				if( sPosX < 160 )
					sPosX = 160;

		}
		break;

		case ARMS_DEALER_OFFER_AREA:
		{
			sPosX = SKI_ARMS_DEALERS_TRADING_INV_X + ( SKI_INV_OFFSET_X * ( ubPocket % ( SKI_NUM_TRADING_INV_SLOTS/2) ) - ( 358 / 2 ) ) + SKI_INV_SLOT_WIDTH / 2;

			sPosY = SKI_ARMS_DEALERS_TRADING_INV_Y + ( ( SKI_INV_OFFSET_Y * ubPocket / ( SKI_NUM_TRADING_INV_SLOTS/2) ) + 1 ) - ( 128 / 2 ) + SKI_INV_SLOT_HEIGHT / 2;

			//if the start position + the height of the box is off the screen, reposition
			if( sPosY < 0 )
				sPosY = 0;


			//if the start position + the width of the box is off the screen, reposition
			if (sPosX + 358 > SCREEN_WIDTH)
				sPosX = SCREEN_WIDTH - 358 - 5;

			//if it is starting to far to the left
			else if( sPosX < 0 )
				sPosX = 10;

			//if the box will appear over the mercs face, move the box over so it doesn't obstruct the face
			if( sPosY < SKI_FACE_Y + SKI_FACE_HEIGHT + 20 )
				if( sPosX < 160 )
					sPosY = 140;
		}
		break;

		default:
			SLOGA("InitShopKeeperItemDescBox: invalid FromLocation");
			return;
	}


	pShopKeeperItemDescObject = pObject;

	InitItemDescriptionBox( gpSMCurrentMerc, 255, sPosX, sPosY, 0 );

	StartSKIDescriptionBox( );
}


void StartSKIDescriptionBox(void)
{
	INT32 iCnt;

	//if the current merc is too far away, dont shade the SM panel because it is already shaded
	const UINT16 h = (gfSMDisableForItems ? INV_INTERFACE_START_Y : SCREEN_HEIGHT);
	DrawHatchOnInventory(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, h);

	InvalidateScreen();

	// disable almost everything!

	gfSMDisableForItems = TRUE;
	DisableInvRegions( gfSMDisableForItems );

	DisableButton( guiSKI_InvPageUpButton );
	DisableButton( guiSKI_InvPageDownButton );
	DisableButton( guiSKI_TransactionButton );
	DisableButton( guiSKI_DoneButton );

	DisableAllDealersInventorySlots();
	DisableAllDealersOfferSlots();

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		gPlayersOfferSlotsMouseRegions[iCnt].Disable();
		gPlayersOfferSlotsSmallFaceMouseRegions[iCnt].Disable();
	}

	if( gShopKeeperSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS )
	{
		gShopKeeperSubTitleMouseRegion.Disable();
	}

	RenderItemDescriptionBox( );
}


void DeleteShopKeeperItemDescBox()
{
	INT32 iCnt;


	pShopKeeperItemDescObject = NULL;
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

	//Redraw the face
	giShopKeeperFaceIndex->uiFlags |= FACE_REDRAW_WHOLE_FACE_NEXT_FRAME;

	// enable almost everything!

	CheckForDisabledForGiveItem( );
	DisableInvRegions( gfSMDisableForItems );	//actually does an enable if current merc is in range

	EnableDisableDealersInventoryPageButtons();
	EnableDisableEvaluateAndTransactionButtons();

	EnableAllDealersInventorySlots();
	EnableAllDealersOfferSlots();

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		gPlayersOfferSlotsMouseRegions[iCnt].Enable();
		gPlayersOfferSlotsSmallFaceMouseRegions[iCnt].Enable();
	}

	if( gShopKeeperSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS )
	{
		gShopKeeperSubTitleMouseRegion.Enable();
	}
}


static BOOLEAN AddObjectForEvaluation(OBJECTTYPE* pObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId, BOOLEAN fFirstOne);
static void CountSubObjectsInObject(OBJECTTYPE* pComplexObject, UINT8* pubTotalSubObjects, UINT8* pubRepairableSubObjects, UINT8* pubNonRepairableSubObjects);
static void SplitComplexObjectIntoSubObjects(OBJECTTYPE* pComplexObject);


static BOOLEAN OfferObjectToDealer(OBJECTTYPE* pComplexObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId)
{
	UINT8   ubTotalSubObjects;
	UINT8   ubRepairableSubObjects;
	UINT8   ubNonRepairableSubObjects;
	UINT8   ubDealerOfferAreaSlotsNeeded;
	UINT8   ubPlayerOfferAreaSlotsNeeded;
	UINT8   ubDiff;
	UINT8   ubHowManyMoreItemsCanDealerTake;
	UINT8   ubSubObject;
	BOOLEAN fFirstOne = TRUE;
	BOOLEAN fSuccess = FALSE;


	if (!DoesDealerDoRepairs(gbSelectedArmsDealerID))
	{
		// if not actually doing repairs, there's no need to split objects up at all
		if ( !AddObjectForEvaluation( pComplexObject, ubOwnerProfileId, bOwnerSlotId, TRUE ) )
		{
			gfDisplayNoRoomMsg = TRUE;
			return(FALSE);	// no room
		}
	}
	else // dealing with a repairman
	{
		// split the object into its components
		SplitComplexObjectIntoSubObjects( pComplexObject );

		// determine how many subobjects of each category this complex object will have to be broken up into
		CountSubObjectsInObject( pComplexObject, &ubTotalSubObjects, &ubRepairableSubObjects, &ubNonRepairableSubObjects );

		// in the simplest situation, the # subobjects of each type gives us how many slots of each type are needed
		ubDealerOfferAreaSlotsNeeded = ubRepairableSubObjects;
		ubPlayerOfferAreaSlotsNeeded = ubNonRepairableSubObjects;

		// consider that if dealer is at or will reach his max # of items repaired limit, not everything repairable will move
		ubHowManyMoreItemsCanDealerTake = SKI_MAX_AMOUNT_OF_ITEMS_DEALER_CAN_REPAIR_AT_A_TIME -
																			CountTotalItemsRepairDealerHasInForRepairs( gbSelectedArmsDealerID ) -
																			CountNumberOfItemsInTheArmsDealersOfferArea();

		// if he can't repair everything repairable that we're about to submit, the space we'll need changes
		if ( ubDealerOfferAreaSlotsNeeded > ubHowManyMoreItemsCanDealerTake )
		{
			ubDiff = ubDealerOfferAreaSlotsNeeded - ubHowManyMoreItemsCanDealerTake;
			ubDealerOfferAreaSlotsNeeded -= ubDiff;
			ubPlayerOfferAreaSlotsNeeded += ubDiff;
		}

		// if there is anything repairable that dealer will accept
		if ( ubDealerOfferAreaSlotsNeeded > 0 )
		{
			// we need at least one EXTRA empty slot in player's area to pass repairable objects through for their evaluation
			ubPlayerOfferAreaSlotsNeeded++;
		}

		Assert( SKI_MAX_AMOUNT_OF_ITEMS_DEALER_CAN_REPAIR_AT_A_TIME < SKI_NUM_TRADING_INV_SLOTS );
		/*
		This code is commented out because a repair dealer will never be allowed to repair over more then
		SKI_MAX_AMOUNT_OF_ITEMS_DEALER_CAN_REPAIR_AT_A_TIME ( 4 ) items at a time, therefore, cant fill up the dealer offer area

		// check if we have room for all of each type
		if ( ( ubDealerOfferAreaSlotsNeeded + CountNumberOfItemsInTheArmsDealersOfferArea() ) > SKI_NUM_TRADING_INV_SLOTS )
		{
			// tell player there's not enough room in the dealer's offer area
			DoSkiMessageBox("There is not enough room in the dealers repair area.", SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK, NULL);
			return(FALSE);	// no room
		}*/

		if ( ( ubPlayerOfferAreaSlotsNeeded + CountNumberOfItemsInThePlayersOfferArea( ) ) > SKI_NUM_TRADING_INV_SLOTS )
		{
			gfDisplayNoRoomMsg = TRUE;
			return(FALSE);	// no room
		}

		// we have room, so move them all to the appropriate slots
		for ( ubSubObject = 0; ubSubObject < MAX_SUBOBJECTS_PER_OBJECT; ubSubObject++ )
		{
			// if there is something stored there
			if ( gSubObject[ ubSubObject ].usItem != NONE )
			{
				// if it's the main item itself (always in the very first subobject),
				// and it has no other subobjects
				if ( ( ubSubObject == 0 ) && ( ubTotalSubObjects == 1) )
				{
					// store its owner merc as the owner, and store the correct slot
					fSuccess = AddObjectForEvaluation( &gSubObject[ ubSubObject ], ubOwnerProfileId, bOwnerSlotId, fFirstOne );
				}
				else	// attachments, bullets/payload
				{
					// store it with a valid owner, but an invalid slot, so it still shows
					// who owns it, but can't return to its slot
					// ARM: New code will be needed here if we add parent/child item
					// support & interface
					fSuccess = AddObjectForEvaluation( &gSubObject[ ubSubObject ], ubOwnerProfileId, -1, fFirstOne );
				}

				// it has to succeed, or we have a bug in our earlier check for sufficient room
				Assert( fSuccess );

				fFirstOne = FALSE;
			}
		}

		gfAlreadySaidTooMuchToRepair = FALSE;
	}

	//ARM: This comment isn't true unless parent/child support is added.  Right now repairmen don't do this!
	// NOTE that, either way, if owned, the item remains in the merc's inventory (hatched out) until the transaction is completed

	// Dirty
	fInterfacePanelDirty = DIRTYLEVEL2;

	return(TRUE); // it worked
}


static void SplitComplexObjectIntoSubObjects(OBJECTTYPE* pComplexObject)
{
	OBJECTTYPE *pNextObj = &gSubObject[ 0 ];
	UINT8 ubNextFreeSlot = 0;
	UINT8 ubCnt;


	Assert( pComplexObject );
	Assert( pComplexObject->ubNumberOfObjects > 0 );
	Assert( pComplexObject->ubNumberOfObjects <= MAX_OBJECTS_PER_SLOT );


	// clear subobject array
	std::fill_n(gSubObject, MAX_SUBOBJECTS_PER_OBJECT, OBJECTTYPE{});


	// if it isn't stacked
	if ( pComplexObject->ubNumberOfObjects == 1 )
	{
		// make the main item into the very first subobject
		*pNextObj = *pComplexObject;

		// strip off any loaded ammo/payload
		if ( GCM->getItem(pComplexObject->usItem)->getItemClass() == IC_GUN )
		{
			// Exception: don't do this with rocket launchers, their "shots left" are fake and this screws 'em up!
			if ( pComplexObject->usItem != ROCKET_LAUNCHER )
			{
				pNextObj->ubGunShotsLeft = 0;
				pNextObj->usGunAmmoItem = NONE;
			}

			/* gunAmmoStatus is currently not being used that way, it's strictly used as a jammed/unjammed, and so should never be 0
			// if jammed, must remember that, so leave it
			if ( pNextObj->bGunAmmoStatus > 0 )
			{
				pNextObj->bGunAmmoStatus = 0;
			}*/
		}

		/* ARM: Can now repair with removeable attachments still attached...
		// strip off any seperable attachments
		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
		{
			if ( pComplexObject->usAttachItem[ ubCnt ] != NONE )
			{
				// If the attachment is detachable
				if (! (GCM->getItem(pComplexObject->usAttachItem[ ubCnt ])->getFlags() & ITEM_INSEPARABLE ) )
				{
					pNextObj->usAttachItem[ ubCnt ] = NONE;
					pNextObj->bAttachStatus[ ubCnt ] = 0;
				}
			}
		}*/

		// advance to next available subobject
		pNextObj = &gSubObject[ ++ubNextFreeSlot ];


		// if it's a gun
		if ( GCM->getItem(pComplexObject->usItem)->getItemClass() == IC_GUN )
		{
			// and it has ammo/payload
			if ( pComplexObject->usGunAmmoItem != NONE )
			{
				// if it's bullets
				if ( GCM->getItem(pComplexObject->usGunAmmoItem)->getItemClass() == IC_AMMO )
				{
					// and there are some left
					if ( pComplexObject->ubGunShotsLeft > 0 )
					{
						// make the bullets into another subobject
						CreateItem( pComplexObject->usGunAmmoItem, 100, pNextObj );
						// set how many are left
						pNextObj->bStatus[ 0 ] = pComplexObject->ubGunShotsLeft;

						pNextObj = &gSubObject[ ++ubNextFreeSlot ];
					}
					// ignore this if it's out of bullets
				}
				else // non-ammo payload
				{
					// make the payload into another subobject
					CreateItem( pComplexObject->usGunAmmoItem, pComplexObject->bGunAmmoStatus, pNextObj );

					// if the gun was jammed, fix up the payload's status
					if ( pNextObj->bStatus[ 0 ] < 0 )
					{
						pNextObj->bStatus[ 0 ] *= -1;
					}

					pNextObj = &gSubObject[ ++ubNextFreeSlot ];
				}
			}
		}


		/* ARM: Can now repair with removeable attachments still attached...
		// make each detachable attachment into a separate subobject
		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
		{
			if ( pComplexObject->usAttachItem[ ubCnt ] != NONE )
			{
				// If the attachment is detachable
				if (! (GCM->getItem(pComplexObject->usAttachItem[ ubCnt ])->getFlags() & ITEM_INSEPARABLE ) )
				{
					CreateItem( pComplexObject->usAttachItem[ ubCnt ], pComplexObject->bAttachStatus[ ubCnt ], pNextObj );

					pNextObj = &gSubObject[ ++ubNextFreeSlot ];
				}
			}
		}*/
	}
	else // stacked
	{
		// these can't be guns, can't have any attachments, can't be imprinted, etc.
		Assert ( GCM->getItem(pComplexObject->usItem)->getItemClass() != IC_GUN );

		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
		{
			Assert( pComplexObject->usAttachItem[ ubCnt ] == NONE );
		}

		// make each item in the stack into a separate subobject
		for( ubCnt = 0; ubCnt < pComplexObject->ubNumberOfObjects; ubCnt++ )
		{
			CreateItem( pComplexObject->usItem, pComplexObject->bStatus[ ubCnt ], pNextObj );

			// advance to next available subobject
			pNextObj = &gSubObject[ ++ubNextFreeSlot ];
		}
	}

	// make sure we didn't screw up and honk something!
	Assert( ubNextFreeSlot <= MAX_SUBOBJECTS_PER_OBJECT );
}


static void CountSubObjectsInObject(OBJECTTYPE* pComplexObject, UINT8* pubTotalSubObjects, UINT8* pubRepairableSubObjects, UINT8* pubNonRepairableSubObjects)
{
	UINT8 ubSubObject;

	*pubTotalSubObjects = 0;
	*pubRepairableSubObjects = 0;
	*pubNonRepairableSubObjects = 0;

	// check every subobject and count it as either repairable or non-
	for ( ubSubObject = 0; ubSubObject < MAX_SUBOBJECTS_PER_OBJECT; ubSubObject++ )
	{
		// if there is something stored there
		if ( gSubObject[ ubSubObject ].usItem != NONE )
		{
			( *pubTotalSubObjects )++;

			// is it in need of fixing, and also repairable by this dealer?
			// A jammed gun with a 100% status is NOT repairable - shouldn't ever happen
			if ( ( gSubObject[ ubSubObject ].bStatus[ 0 ] != 100 ) &&
				CanDealerRepairItem( gbSelectedArmsDealerID, gSubObject[ ubSubObject ].usItem ) )

			{
				( *pubRepairableSubObjects )++;
			}
			else
			{
				( *pubNonRepairableSubObjects )++;
			}
		}
	}
}


static BOOLEAN AddObjectForEvaluation(OBJECTTYPE* pObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId, BOOLEAN fFirstOne)
{
	INVENTORY_IN_SLOT InvSlot;
	INT8 bAddedToSlotID;

	// Make a new inv slot out of the subobject
	InvSlot = INVENTORY_IN_SLOT{};
	InvSlot.ItemObject = *pObject;

	InvSlot.sItemIndex = pObject->usItem;
	InvSlot.ubLocationOfObject = PLAYERS_INVENTORY;
	InvSlot.ubIdOfMercWhoOwnsTheItem = ubOwnerProfileId;

	//Add the item to the Players Offer Area
	bAddedToSlotID = AddItemToPlayersOfferArea( ubOwnerProfileId, &InvSlot, bOwnerSlotId );

	//Do the evaluation for the item if it was added correctly
	if( bAddedToSlotID != -1 )
	{
		// This will move any repairable (sub)objects into the dealer's offer area immediately
		EvaluateItemAddedToPlayersOfferArea( bAddedToSlotID, fFirstOne );
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


// The Shopkeeper interface *MUST* use this intermediary function instead of calling AutoPlaceObject() directly!
// This is because the OBJECTTYPEs used within Shopkeeper may contain an illegal ubNumberOfObjects
static BOOLEAN ShopkeeperAutoPlaceObject(SOLDIERTYPE* pSoldier, OBJECTTYPE* pObject, BOOLEAN fNewItem)
{
	OBJECTTYPE CopyOfObject;
	UINT8 ubObjectsLeftToPlace;

	// the entire pObj will get memset to 0 by RemoveObjs() if all the items are successfully placed,
	// so we have to keep a copy to retrieve with every iteration of the loop
	CopyOfObject = *pObject;


	ubObjectsLeftToPlace = pObject->ubNumberOfObjects;

	while ( ubObjectsLeftToPlace > 0 )
	{
		// figure out how many to place during this loop iteration.  Can't do more than MAX_OBJECTS_PER_SLOT at a time
		pObject->ubNumberOfObjects = MIN( MAX_OBJECTS_PER_SLOT, ubObjectsLeftToPlace);
		ubObjectsLeftToPlace -= pObject->ubNumberOfObjects;

		if (!AutoPlaceObject( pSoldier, pObject, fNewItem ))
		{
			// no more room, didn't all fit - add back in any that we didn't even get to yet
			pObject->ubNumberOfObjects += ubObjectsLeftToPlace;
			return( FALSE );
		}

		// restore object properties from our backup copy
		*pObject = CopyOfObject;
	}

	return( TRUE );
}


// The Shopkeeper interface *MUST* use this intermediary function instead of calling AddItemToPool() directly!
// This is because the OBJECTTYPEs used within Shopkeeper may contain an illegal ubNumberOfObjects
static void ShopkeeperAddItemToPool(INT16 const sGridNo, OBJECTTYPE* const pObject, UINT8 const ubLevel)
{
	OBJECTTYPE CopyOfObject;
	UINT8 ubObjectsLeftToPlace;

	// the entire pObj will get memset to 0 by RemoveObjs() if all the items are successfully placed,
	// so we have to keep a copy to retrieve with every iteration of the loop
	CopyOfObject = *pObject;

	ubObjectsLeftToPlace = pObject->ubNumberOfObjects;

	while ( ubObjectsLeftToPlace > 0 )
	{
		// figure out how many to place during this loop iteration.  Can't do more than MAX_OBJECTS_PER_SLOT at a time
		pObject->ubNumberOfObjects = MIN( MAX_OBJECTS_PER_SLOT, ubObjectsLeftToPlace);
		ubObjectsLeftToPlace -= pObject->ubNumberOfObjects;

		AddItemToPool(sGridNo, pObject, VISIBLE, ubLevel, 0, 0);

		// restore object properties from our backup copy
		*pObject = CopyOfObject;
	}
}


static void IfMercOwnedCopyItemToMercInv(const INVENTORY_IN_SLOT* pInv)
{
	//if the item picked up was in a previous location, and that location is on a merc's inventory
	if ( ( pInv->bSlotIdInOtherLocation != -1 ) && ( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE ) )
	{
		// then it better be a valid slot #
		Assert( pInv->bSlotIdInOtherLocation < NUM_INV_SLOTS );
		// and it better have a valid merc who owned it
		Assert( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE );

		// get soldier
		SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(pInv->ubIdOfMercWhoOwnsTheItem);
		Assert(s != NULL);
		Assert(CanMercInteractWithSelectedShopkeeper(s));

		//Copy the object back into that merc's original inventory slot
		s->inv[pInv->bSlotIdInOtherLocation] = pInv->ItemObject;
	}
}


static void IfMercOwnedRemoveItemFromMercInv2(UINT8 ubOwnerProfileId, INT8 bOwnerSlotId);


static void IfMercOwnedRemoveItemFromMercInv(const INVENTORY_IN_SLOT* pInv)
{
	IfMercOwnedRemoveItemFromMercInv2( pInv->ubIdOfMercWhoOwnsTheItem, pInv->bSlotIdInOtherLocation );
}


static void IfMercOwnedRemoveItemFromMercInv2(UINT8 ubOwnerProfileId, INT8 bOwnerSlotId)
{
	BOOLEAN fSuccess;
	OBJECTTYPE ObjectToRemove;

	//if this item was in a previous location, and that location is on a merc's inventory
	if ( ( bOwnerSlotId != -1 ) && ( ubOwnerProfileId != NO_PROFILE ) )
	{
		// then it better be a valid slot #
		Assert( bOwnerSlotId < NUM_INV_SLOTS );
		// and it better have a valid merc who owned it
		Assert( ubOwnerProfileId != NO_PROFILE );

		SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(ubOwnerProfileId);
		Assert(s != NULL);
		Assert(CanMercInteractWithSelectedShopkeeper(s));

		//remove the object from that merc's original inventory slot
		fSuccess = RemoveObjectFromSlot(s, bOwnerSlotId, &ObjectToRemove);
		Assert(fSuccess);
	}
}


static BOOLEAN SKITryToAddInvToMercsInventory(INVENTORY_IN_SLOT* pInv, SOLDIERTYPE* pSoldier);


static BOOLEAN SKITryToReturnInvToOwnerOrCurrentMerc(INVENTORY_IN_SLOT* pInv)
{
	// don't use this if the item has a copy in merc's inventory!!  It would create a duplicate!!
	Assert( pInv->bSlotIdInOtherLocation == -1 );

	// if it does have an owner
	if( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE )
	{
		SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(pInv->ubIdOfMercWhoOwnsTheItem);
		// if that soldier is not in player's hire any longer
		if (s == NULL) return FALSE;

		// For owners of repaired items, this checks that owner is still hired, in sector,
		// on current squad, close enough to the shopkeeper, etc.
		if (!CanMercInteractWithSelectedShopkeeper(s)) return FALSE;

		// Try to find a place to put in its owner's inventory (regardless of which merc is currently displayed!)
		if (SKITryToAddInvToMercsInventory(pInv, s)) return TRUE;

		// owner's inventory is full, so we'll try to give it to the current merc instead
	}


	//if the current merc is not disabled
	if( !gfSMDisableForItems )
	{
		// Try to find a place to put in current merc's inventory
		if ( SKITryToAddInvToMercsInventory( pInv, gpSMCurrentMerc ) )
		{
			return( TRUE );
		}
	}

	//failed to add item, either inventory was filled up, or nobody was even eligible to receive it
	return( FALSE );
}


static BOOLEAN SKITryToAddInvToMercsInventory(INVENTORY_IN_SLOT* pInv, SOLDIERTYPE* pSoldier)
{
	INT8    bMoneyInvPos;
	BOOLEAN fNewItem = FALSE;


	//if the item is money
	if( GCM->getItem(pInv->sItemIndex)->getItemClass() == IC_MONEY )
	{
		// search through the merc's inventory for a pocket of money which isn't full already
		bMoneyInvPos = GetInvSlotOfUnfullMoneyInMercInventory( pSoldier );

		// if he has a money pocket going
		if ( bMoneyInvPos != -1 )
		{
			// try to add to it.  If successful, it will delete the object...  It returns TRUE even if not all would fit!
			PlaceObject( pSoldier, bMoneyInvPos, &( pInv->ItemObject ) );

			// if the money is all gone
			if ( pInv->ItemObject.uiMoneyAmount == 0 )
			{
				// we've been succesful!
				return(TRUE);
			}
			// otherwise we'll try to place the rest seperately
		}
	}


	// If it's just been purchased or repaired, mark it as a "new item"
	fNewItem = ( BOOLEAN ) ( pInv->uiFlags & ( ARMS_INV_JUST_PURCHASED | ARMS_INV_ITEM_REPAIRED ) );

	//try autoplacing the item in this soldier's inventory.
	if( !ShopkeeperAutoPlaceObject( pSoldier, &pInv->ItemObject, fNewItem ) )
	{
		//failed to add item, inventory probably filled up
		return( FALSE );
	}

	return(TRUE);
}


BOOLEAN CanMercInteractWithSelectedShopkeeper(const SOLDIERTYPE* pSoldier)
{
	INT16  sDestGridNo;
	INT8   bDestLevel;
	INT16  sDistVisible;
	UINT32 uiRange;


	Assert( pSoldier!= NULL );
	Assert( gbSelectedArmsDealerID != -1 );

	const SOLDIERTYPE* const pShopkeeper = FindSoldierByProfileID(SelectedArmsDealer()->profileID);
	Assert( pShopkeeper != NULL );
	Assert( pShopkeeper->bInSector );

	if ( pShopkeeper->bLife < OKLIFE )
	{
		return( FALSE );
	}

	if ( pSoldier->bActive && pSoldier->bInSector && IsMercOnCurrentSquad( pSoldier ) && ( pSoldier->bLife >= OKLIFE ) &&
		!IsMechanical(*pSoldier))
	{
		sDestGridNo = pShopkeeper->sGridNo;
		bDestLevel = pShopkeeper->bLevel;

		// is he close enough to see that gridno if he turns his head?
		sDistVisible = DistanceVisible( pSoldier, DIRECTION_IRRELEVANT, DIRECTION_IRRELEVANT, sDestGridNo, bDestLevel );

		// If he has LOS...
		if ( SoldierTo3DLocationLineOfSightTest( pSoldier, sDestGridNo, bDestLevel, 3, (UINT8) sDistVisible, TRUE ) )
		{
			// Get range to shopkeeper
			uiRange = GetRangeFromGridNoDiff( pSoldier->sGridNo, sDestGridNo );

			// and is close enough to talk to the shopkeeper (use this define INSTEAD of PASSING_ITEM_DISTANCE_OKLIFE!)
			if ( uiRange <= NPC_TALK_RADIUS )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}

static void ExitSKIRequested(void)
{
	BOOLEAN fPlayerOwnedStuffOnTable = FALSE;


	ShutUpShopKeeper();

	if( !gfRemindedPlayerToPickUpHisStuff )
	{
		if( AreThereItemsInThePlayersOfferArea( ) )
		{
			fPlayerOwnedStuffOnTable = TRUE;
		}

		if( (DoesDealerDoRepairs(gbSelectedArmsDealerID)) &&
				AreThereItemsInTheArmsDealersOfferArea( ) )
		{
			fPlayerOwnedStuffOnTable = TRUE;
		}

		//if there are any items belonging to player still on the table
		if( fPlayerOwnedStuffOnTable )
		{
			// remind player to pick them up
			ShutUpShopKeeper( );
			StartShopKeeperTalking( SK_QUOTES_PRESSED_DONE_STILL_HAS_STUFF_IN_OFFER_AREA );
			gfRemindedPlayerToPickUpHisStuff = TRUE;
			return;
		}
		else
		{
			//if the player has already requested to leave
			if( gfUserHasRequestedToLeave )
			{
				// has already asked to leave, shut the dealer up and allow the player to leave
				ShutUpShopKeeper();
				return;
			}
		}
	}


	// if the player hasn't already requested to leave
	if( !gfUserHasRequestedToLeave )
	{
		UINT16 usQuoteNum;

		if( gfDoneBusinessThisSession )
			usQuoteNum = SK_QUOTES_PRESSES_DONE_HAS_AT_LEAST_1_TRANSACTION;
		else
			usQuoteNum = SK_QUOTES_PRESSED_DONE_HASNT_MADE_TRANSACTION;

		StartShopKeeperTalking( usQuoteNum );

		//Set the fact that the player wants to leave
		gfUserHasRequestedToLeave = TRUE;
	}
}


static void ResetAllQuoteSaidFlags()
{
	// Reset flags for quotes said.
	FOR_EACH(BOOLEAN, i, gfEvalResultQuoteSaid) *i = FALSE;

	guiLastTimeDealerSaidNormalEvaluationQuote = 0;
}


static void ReturnItemToPlayerSomehow(INVENTORY_IN_SLOT* pInvSlot, SOLDIERTYPE* pDropSoldier);


static void DealWithItemsStillOnTheTable(void)
{
	UINT8 ubCnt;
	SOLDIERTYPE *pDropSoldier;


	// in case we have have to drop stuff off at someone's feet, figure out where it's all gonna go

	// use the current merc, unless he's ineligible, then use the selected merc instead.
	if( !gfSMDisableForItems )
	{
		pDropSoldier = gpSMCurrentMerc;
	}
	else
	{
		pDropSoldier = GetSelectedMan();
	}

	// this guy HAS to be valid!
	Assert( CanMercInteractWithSelectedShopkeeper( pDropSoldier ) );


	//loop through the players offer area and add return any items there to the player
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		INVENTORY_IN_SLOT* const o = &PlayersOfferArea[ubCnt];
		//if there is an item here, give it back somehow
		if (o->fActive)
		{
			ReturnItemToPlayerSomehow(o, pDropSoldier);
			ClearPlayersOfferSlot( ubCnt );
		}
	}


	//if the dealer repairs
	if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
	{
		//loop through the arms dealers' offer area and return any items there to the player
		for( ubCnt = 0; ubCnt < SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
		{
			INVENTORY_IN_SLOT* const a = &ArmsDealerOfferArea[ubCnt];
		//if there is an item here, give it back somehow
			if (a->fActive)
			{
				ReturnItemToPlayerSomehow(a, pDropSoldier);
				ClearArmsDealerOfferSlot( ubCnt );
			}
		}
	}
}


static void ReturnItemToPlayerSomehow(INVENTORY_IN_SLOT* pInvSlot, SOLDIERTYPE* pDropSoldier)
{
	//if the item doesn't have a duplicate copy in its owner merc's inventory slot
	if( pInvSlot->bSlotIdInOtherLocation == -1 )
	{
		// first try to give it to its owner, or if he's unavailable, the current merc
		if ( !SKITryToReturnInvToOwnerOrCurrentMerc( pInvSlot ) )
		{
			// failed to add item, inventory probably filled up or item is unowned and current merc ineligible.
			// drop it at the specified guy's feet instead
			ShopkeeperAddItemToPool(pDropSoldier->sGridNo, &pInvSlot->ItemObject, pDropSoldier->bLevel);
		}
	}
}


static void GivePlayerSomeChange(UINT32 uiAmount)
{
	INVENTORY_IN_SLOT MoneyInvSlot;

	MoneyInvSlot = INVENTORY_IN_SLOT{};

	CreateMoney( uiAmount, &MoneyInvSlot.ItemObject );
	MoneyInvSlot.sItemIndex = MoneyInvSlot.ItemObject.usItem;
	MoneyInvSlot.ubLocationOfObject = PLAYERS_INVENTORY;
	MoneyInvSlot.ubIdOfMercWhoOwnsTheItem = NO_PROFILE;

	AddItemToPlayersOfferArea( NO_PROFILE, &MoneyInvSlot, -1 );

	if ( ( gbSelectedArmsDealerID == ARMS_DEALER_JAKE ) &&
		( gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash < uiAmount ) )
	{
		// HACK HACK HACK: We forgot to write/record quote 27 for Jake, so he ALWAYS must have enough money!
		gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash += ( uiAmount - gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash );
	}

	// he must have enough cash left to give change
	Assert( gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash >= uiAmount );
	// subtract change from dealer's cash
	gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash -= uiAmount;
}


static void DealerGetsBribed(UINT8 ubProfileId, UINT32 uiMoneyAmount)
{
	BOOLEAN fBribable = FALSE;
	UINT32 uiMinBribe = 0;

	// this is only for dealers
	if (!IsMercADealer( ubProfileId ) )
	{
		return;
	}

	Fact usFact = FACT_NONE;
	switch ( ubProfileId )
	{
		case FRANK:
			fBribable = TRUE;
			uiMinBribe = 10; // This should match the handling of a bribe submitted outside the SKI, using the GIVE system
			usFact = FACT_FRANK_HAS_BEEN_BRIBED;
			break;
	}

	// if this guy can be bribed, and we gave enough to bribe him
	if ( fBribable && ( uiMoneyAmount >= uiMinBribe ) )
	{
		// set the appropriate fact
		SetFactTrue( usFact );
	}
}


static void DelayRepairsInProgressBy(UINT32 uiMinutesDelayed);
static BOOLEAN RepairmanFixingAnyItemsThatShouldBeDoneNow(UINT32* puiHoursSinceOldestItemRepaired);


static void HandlePossibleRepairDelays(void)
{
	// assume there won't be a delay
	gfStartWithRepairsDelayedQuote = FALSE;

	if (!GetDealer(gbSelectedArmsDealerID)->hasFlag(ArmsDealerFlag::DELAYS_REPAIR)) return;

	ARMS_DEALER_STATUS& status = gArmsDealerStatus[gbSelectedArmsDealerID];
	// because the quotes are so specific, we'll only use them once per game per repairman
	if (status.fRepairDelayBeenUsed) return;

	// and it's been a while since the player last dealt with this repairman (within SKI that is)
	// this serves 2 purposes:
	// a) reduces delays being much more likely if player checks time remaining very frequently, AND
	// b) gives time for the events described in the text of the dealers' excuses to happen (e.g. scouting trip)
	if (GetWorldTotalMin() - status.uiTimePlayerLastInSKI < 3 * 60) return;

	// if he should have been finished, but it's only been a few hours since then (not days!)
	UINT32 uiHoursSinceAnyItemsShouldHaveBeenRepaired = 0;
	if (!RepairmanFixingAnyItemsThatShouldBeDoneNow(&uiHoursSinceAnyItemsShouldHaveBeenRepaired)) return;
	if (uiHoursSinceAnyItemsShouldHaveBeenRepaired >= REPAIR_DELAY_IN_HOURS) return;

	// then there's a fair chance he'll be delayed.  Use pre-chance to hopefully preserve across reloads
	if (!PreChance(50)) return;

	DelayRepairsInProgressBy((REPAIR_DELAY_IN_HOURS + Random(REPAIR_DELAY_IN_HOURS)) * 60);

	// this triggers a different opening quote
	gfStartWithRepairsDelayedQuote = TRUE;

	// set flag so it doesn't happen again
	status.fRepairDelayBeenUsed = TRUE;
}


static BOOLEAN RepairmanFixingAnyItemsThatShouldBeDoneNow(UINT32* puiHoursSinceOldestItemRepaired)
{
	UINT16 usItemIndex;
	UINT8  ubElement;
	DEALER_ITEM_HEADER *pDealerItem;
	DEALER_SPECIAL_ITEM *pSpecialItem;
	BOOLEAN fFoundOne = FALSE;
	UINT32 uiMinutesSinceItWasDone;
	UINT32 uiMinutesShopClosedSinceItWasDone;
	UINT32 uiWorkingHoursSinceThisItemRepaired;


	//if the dealer is not a repair dealer, return
	if( !DoesDealerDoRepairs( gbSelectedArmsDealerID ) )
		return( FALSE );

	*puiHoursSinceOldestItemRepaired = 0;

	//loop through the dealers inventory and check if there are only unrepaired items
	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		pDealerItem = &( gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ] );

		//if there is some items in stock
		if( pDealerItem->ubTotalItems )
		{
			//loop through the array of items
			Assert(pDealerItem->SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(pDealerItem->SpecialItem.size()); ubElement++)
			{
				pSpecialItem = &( pDealerItem->SpecialItem[ ubElement ] );

				if ( pSpecialItem->fActive )
				{
					//if the items status is below 0, the item is being repaired
					if( pSpecialItem->Info.bItemCondition < 0 )
					{
						//if the repairs are done
						if( pSpecialItem->uiRepairDoneTime <= GetWorldTotalMin() )
						{
							// at least one item is supposed to be done by now
							fFoundOne = TRUE;

							uiMinutesSinceItWasDone = GetWorldTotalMin() - pSpecialItem->uiRepairDoneTime;
							uiMinutesShopClosedSinceItWasDone = CalculateMinutesClosedBetween( gbSelectedArmsDealerID, pSpecialItem->uiRepairDoneTime, GetWorldTotalMin() );

							// calculate how many WORKING hours since this item's been repaired
							uiWorkingHoursSinceThisItemRepaired = ( uiMinutesSinceItWasDone - uiMinutesShopClosedSinceItWasDone ) / 60;

							// we need to determine how long it's been since the item that's been repaired for the longest time was done
							if ( uiWorkingHoursSinceThisItemRepaired > *puiHoursSinceOldestItemRepaired )
							{
								*puiHoursSinceOldestItemRepaired = uiWorkingHoursSinceThisItemRepaired;
							}
						}
					}
				}
			}
		}
	}


	// if FALSE returned here, he's either not repairing anything, or none of it's done yet
	return( fFoundOne );
}


static void DelayRepairsInProgressBy(UINT32 uiMinutesDelayed)
{
	UINT16 usItemIndex;
	UINT8  ubElement;
	DEALER_ITEM_HEADER *pDealerItem;
	DEALER_SPECIAL_ITEM *pSpecialItem;
	UINT32 uiMinutesShopClosedBeforeItsDone;


	//if the dealer is not a repair dealer, return
	if( !DoesDealerDoRepairs( gbSelectedArmsDealerID ) )
		return;

	//loop through the dealers inventory and check if there are only unrepaired items
	for( usItemIndex = 1; usItemIndex < MAXITEMS; usItemIndex++ )
	{
		pDealerItem = &( gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ] );

		//if there is some items in stock
		if( pDealerItem->ubTotalItems )
		{
			//loop through the array of items
			Assert(pDealerItem->SpecialItem.size() <= UINT8_MAX);
			for (ubElement = 0; ubElement < static_cast<UINT8>(pDealerItem->SpecialItem.size()); ubElement++)
			{
				pSpecialItem = &( pDealerItem->SpecialItem[ ubElement ] );

				if ( pSpecialItem->fActive )
				{
					//if the items status is below 0, the item is being repaired
					if( pSpecialItem->Info.bItemCondition < 0 )
					{
						uiMinutesShopClosedBeforeItsDone = CalculateOvernightRepairDelay( gbSelectedArmsDealerID, pSpecialItem->uiRepairDoneTime, uiMinutesDelayed );

						// add this many minutes to the repair time estimate
						pSpecialItem->uiRepairDoneTime += ( uiMinutesShopClosedBeforeItsDone + uiMinutesDelayed );
					}
				}
			}
		}
	}
}


//Mouse Call back for the Arms delaers face
static void SelectArmsDealersDropItemToGroundRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SOLDIERTYPE *pDropSoldier;

		// use the current merc, unless he's ineligible, then use the selected merc instead.
		if( !gfSMDisableForItems )
		{
			pDropSoldier = gpSMCurrentMerc;
		}
		else
		{
			pDropSoldier = GetSelectedMan();
		}

		//if we don't have an item, pick one up
		if( gMoveingItem.sItemIndex != 0 )
		{
			//add the item to the ground
			ShopkeeperAddItemToPool(pDropSoldier->sGridNo, &gMoveingItem.ItemObject, pDropSoldier->bLevel);

			//Reset the cursor
			SetSkiCursor( CURSOR_NORMAL );

			//refresh the screen
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		}
	}
}


static BOOLEAN CanTheDropItemToGroundStringBeDisplayed(void);


static void SelectArmsDealersDropItemToGroundMovementRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gfSkiDisplayDropItemToGroundText = FALSE;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if( CanTheDropItemToGroundStringBeDisplayed() )
		{
			gfSkiDisplayDropItemToGroundText = TRUE;
		}
		else
		{
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			gfSkiDisplayDropItemToGroundText = FALSE;
		}
	}
}


static BOOLEAN CanTheDropItemToGroundStringBeDisplayed(void)
{
	//if we don't have an item, pick one up
	if( gMoveingItem.sItemIndex != 0 )
		return( TRUE );
	else
		return( FALSE );
}


static void DisplayTheSkiDropItemToGroundString(void)
{
	UINT16 usHeight;

	//get the height of the displayed text
	usHeight = DisplayWrappedString(SKI_DROP_ITEM_TO_GROUND_START_X, SKI_DROP_ITEM_TO_GROUND_TEXT_START_Y, SCREEN_WIDTH - SKI_DROP_ITEM_TO_GROUND_START_X, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[SKI_TEXT_DROP_ITEM_TO_GROUND], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT);

	//display the 'drop item to ground' text
	DisplayWrappedString(SKI_DROP_ITEM_TO_GROUND_START_X, SKI_DROP_ITEM_TO_GROUND_TEXT_START_Y - usHeight, SCREEN_WIDTH - SKI_DROP_ITEM_TO_GROUND_START_X, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[SKI_TEXT_DROP_ITEM_TO_GROUND], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED | INVALIDATE_TEXT);
}


static UINT32 EvaluateInvSlot(INVENTORY_IN_SLOT* pInvSlot)
{
	UINT32 uiEvalResult = EVAL_RESULT_NORMAL;
	FLOAT  dPriceModifier;
	UINT32 uiBuyingPrice;


	//if the dealer is Micky
	if( gbSelectedArmsDealerID == ARMS_DEALER_MICKY )
	{
		const SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(SelectedArmsDealer()->profileID);
		if (s != NULL && GetDrunkLevel(s) == DRUNK)
		{
			//Micky is DRUNK, pays more!
			dPriceModifier = SelectedArmsDealer()->sellingPrice;
		}
		else
		{
			// Micky isn't drunk, charge regular price
			dPriceModifier = SelectedArmsDealer()->buyingPrice;
		}
	}
	else
	{
		dPriceModifier = SelectedArmsDealer()->buyingPrice;
	}


	// Calculate dealer's buying price for the item
	uiBuyingPrice = CalcShopKeeperItemPrice( DEALER_BUYING, FALSE, pInvSlot->sItemIndex, dPriceModifier, &( pInvSlot->ItemObject ) );

	pInvSlot->uiItemPrice = uiBuyingPrice;

	if ( uiBuyingPrice > 0 )
	{
		// check if the item is really badly damaged
		if( GCM->getItem(pInvSlot->sItemIndex)->getItemClass() != IC_AMMO )
		{
			if( pInvSlot->ItemObject.bStatus[ 0 ] < REALLY_BADLY_DAMAGED_THRESHOLD )
			{
				uiEvalResult = EVAL_RESULT_OK_BUT_REALLY_DAMAGED;
			}
		}

		//the item can be sold here, so mark it as such
		pInvSlot->uiFlags |= ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
	}
	else
	{
		// he normally buys this, but it's in such bad shape that he won't, it's worthless (different quote)
		uiEvalResult = EVAL_RESULT_WORTHLESS;

		pInvSlot->uiFlags &= ~ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
	}

	return( uiEvalResult );
}



// round off reapir times shown to the near quarter-hour
#define REPAIR_MINUTES_INTERVAL 15


static ST::string BuildRepairTimeString(UINT32 uiTimeInMinutesToFixItem)
{
	UINT16 usNumberOfHoursToFixItem = 0;


	// if it's 0, it shouldn't be up here any more!
	Assert ( uiTimeInMinutesToFixItem > 0 );


	// round off to next higher 15 minutes
	if ( ( uiTimeInMinutesToFixItem % REPAIR_MINUTES_INTERVAL ) > 0 )
	{
		uiTimeInMinutesToFixItem += REPAIR_MINUTES_INTERVAL - ( uiTimeInMinutesToFixItem % REPAIR_MINUTES_INTERVAL );
	}

	if ( uiTimeInMinutesToFixItem < REPAIR_MINUTES_INTERVAL )
	{
		uiTimeInMinutesToFixItem = REPAIR_MINUTES_INTERVAL;
	}

	// show up to 1.5 hrs in minutes
	if ( uiTimeInMinutesToFixItem <= 90 )
	{
		// show minutes
		return st_format_printf(SKI_Text[ SKI_TEXT_MINUTES ], uiTimeInMinutesToFixItem);
	}
	else
	{
		// show hours
		// round fractions of 15+ minutes up to next full hour
		usNumberOfHoursToFixItem = (UINT16) ( ( uiTimeInMinutesToFixItem + 45 ) / 60 );
		return st_format_printf(SKI_Text[ SKI_TEXT_PLURAL_HOURS ], usNumberOfHoursToFixItem);
	}
}


static ST::string BuildDoneWhenTimeString(ArmsDealerID ubArmsDealer, UINT16 usItemIndex, UINT8 ubElement)
{
	UINT32 uiDoneTime;
	UINT32 uiDay, uiHour, uiMin;


	//dealer must be a repair dealer
	Assert( DoesDealerDoRepairs( ubArmsDealer ) );
	// element index must be valid
	Assert( ubElement < gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem.size() );
	// that item must be active
	Assert( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive );
	// that item must be in repair
	Assert( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].Info.bItemCondition < 0 );

	//if the item has already been repaired
	if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].uiRepairDoneTime <= GetWorldTotalMin() )
	{
		return ST::null;
	}

	uiDoneTime = gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].uiRepairDoneTime;

	// round off to next higher 15 minutes
	if ( ( uiDoneTime % REPAIR_MINUTES_INTERVAL ) > 0 )
	{
		uiDoneTime += REPAIR_MINUTES_INTERVAL - ( uiDoneTime % REPAIR_MINUTES_INTERVAL );
	}

	// uiDoneTime is in minutes...
	uiDay  = ( uiDoneTime / NUM_MIN_IN_DAY );
	uiHour = ( uiDoneTime - ( uiDay * NUM_MIN_IN_DAY ) ) / NUM_MIN_IN_HOUR;
	uiMin  = uiDoneTime - ( ( uiDay * NUM_MIN_IN_DAY ) + ( uiHour * NUM_MIN_IN_HOUR ) );

	// only show day if it's gonna take overnight
	if ( GetWorldDay() != uiDay )
	{
		return ST::format("{} {} {02d}:{02d}", pDayStrings, uiDay, uiHour, uiMin);
	}
	else
	{
		return ST::format("{02d}:{02d}", uiHour, uiMin);
	}
}


static ST::string BuildItemHelpTextString(const INVENTORY_IN_SLOT* pInv, UINT8 ubScreenArea)
{
	ST::string zHelpText;
	ST::string zRepairTime;

	if( pInv != NULL )
	{
		zHelpText = GetHelpTextForItem(pInv->ItemObject);

		// add repair time for items in a repairman's offer area
		if ( ( ubScreenArea == ARMS_DEALER_OFFER_AREA ) &&
			(DoesDealerDoRepairs(gbSelectedArmsDealerID)) )
		{
			zRepairTime = BuildRepairTimeString(CalculateObjectItemRepairTime( gbSelectedArmsDealerID, &( pInv->ItemObject ) ));
			return ST::format("{}\n({}: {})", zHelpText, gzLateLocalizedString[STR_LATE_44], zRepairTime);
		}
		else
		{
			return zHelpText;
		}
	}
	else
	{
		return ST::null;
	}
}


static void DisableAllDealersInventorySlots(void)
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_ARMS_DEALERS_INV_SLOTS; iCnt++)
	{
		gDealersInventoryMouseRegions[iCnt].Disable();

		//if the dealer repairs
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			gRepairmanInventorySmallFaceMouseRegions[iCnt].Disable();
		}
	}
}


static void EnableAllDealersInventorySlots(void)
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_ARMS_DEALERS_INV_SLOTS; iCnt++)
	{
		gDealersInventoryMouseRegions[iCnt].Enable();

		//if the dealer repairs
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			gRepairmanInventorySmallFaceMouseRegions[iCnt].Enable();
		}
	}
}


static void DisableAllDealersOfferSlots(void)
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		gDealersOfferSlotsMouseRegions[iCnt].Disable();

		//if the dealer repairs
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			gDealersOfferSlotsSmallFaceMouseRegions[iCnt].Disable();
		}
	}
}


static void EnableAllDealersOfferSlots(void)
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		gDealersOfferSlotsMouseRegions[iCnt].Enable();

		//if the dealer repairs
		if (DoesDealerDoRepairs(gbSelectedArmsDealerID))
		{
			gDealersOfferSlotsSmallFaceMouseRegions[iCnt].Enable();
		}
	}
}


static void HatchOutInvSlot(UINT16 usPosX, UINT16 usPosY)
{
	//if we are in the item desc panel
	if( InItemDescriptionBox( ) && pShopKeeperItemDescObject != NULL )
	{
		// do nothing, don't wanna shade things twice!
		return;
	}

	const UINT16 usSlotWidth  = SKI_INV_SLOT_WIDTH;
	const UINT16 usSlotHeight = SKI_INV_SLOT_HEIGHT;

	//Hatch it out
	DrawHatchOnInventory(FRAME_BUFFER, usPosX, usPosY, usSlotWidth, usSlotHeight);
	InvalidateRegion(usPosX, usPosY, usPosX + usSlotWidth, usPosY + usSlotHeight);
}
