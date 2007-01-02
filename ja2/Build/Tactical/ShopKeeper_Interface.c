#ifdef PRECOMPILEDHEADERS
	#include "Tactical All.h"
#else
	#include "Types.h"
	#include "ShopKeeper_Interface.h"
	#include "Utilities.h"
	#include "Game_Clock.h"
	#include "Render_Dirty.h"
	#include "WCheck.h"
	#include "Font_Control.h"
	#include "WordWrap.h"
	#include "Text_Input.h"
	#include "Interface.h"
	#include "Interface_Control.h"
	#include "Overhead.h"
	#include "Cursors.h"
	//#include "Store_Inventory.h"
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
	#include "Personnel.h"
	#include "LaptopSave.h"
	#include "Quests.h"
	#include "Weapons.h"
	#include "MessageBoxScreen.h"
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
	#include "JAScreens.h"
	#include "ScreenIDs.h"
	#include "Items.h"
#endif

#include "BuildDefines.h"

#ifdef JA2TESTVERSION
#	include "Soldier_Create.h"
#endif


// temp
struct skirgbcolor
{
	UINT8 ubRed;
	UINT8 ubGreen;
	UINT8 ubBlue;
};
typedef struct skirgbcolor SKIRGBCOLOR;

SKIRGBCOLOR SkiGlowColorsA[]={
	{0,0,0},
	{25,0,0},
	{50,0,0},
	{75,0,0},
	{100,0,0},
	{125,0,0},
	{150,0,0},
	{175,0,0},
	{200,0,0},
	{225,0,0},
	{250,0,0},
};



////////////////////////////////////////////
//
//	global defines
//
///////////////////////////////////////////

#define		SKI_BUTTON_FONT										MILITARYFONT1//FONT14ARIAL
#define		SKI_BUTTON_COLOR									73


#define		SKI_ATM_BUTTON_FONT								FONT10ARIAL
#define		SKI_ATM_BUTTON_COLOR							FONT_MCOLOR_BLACK

#define		SKI_TITLE_FONT										MILITARYFONT1//FONT14ARIAL
#define		SKI_TITLE_COLOR										169//FONT_MCOLOR_LTYELLOW

#define		SKI_LABEL_FONT										MILITARYFONT1

#define		SKI_ITEM_DESC_FONT								SMALLCOMPFONT
#define		SKI_ITEM_PRICE_COLOR							FONT_MCOLOR_WHITE

#define		SKIT_NUMBER_FONT									BLOCKFONT2

#define		SKI_MAIN_BACKGROUND_X							0
#define		SKI_MAIN_BACKGROUND_Y							0

#define		SKI_FACE_X												13
#define		SKI_FACE_Y												13
#define		SKI_FACE_WIDTH										90
#define		SKI_FACE_HEIGHT										100

#define		SKI_PAGE_UP_ARROWS_X							121
#define		SKI_PAGE_UP_ARROWS_Y							35

#define		SKI_PAGE_DOWN_ARROWS_X						SKI_PAGE_UP_ARROWS_X
#define		SKI_PAGE_DOWN_ARROWS_Y						102

//Evaluate:
//#define		SKI_EVALUATE_BUTTON_X							15
//#define		SKI_EVALUATE_BUTTON_Y							233

#define		SKI_TRANSACTION_BUTTON_X					147//214
#define		SKI_TRANSACTION_BUTTON_Y					233//SKI_EVALUATE_BUTTON_Y

#define		SKI_DONE_BUTTON_X										292//414
#define		SKI_DONE_BUTTON_Y										233//SKI_EVALUATE_BUTTON_Y

#define		SKI_MAIN_TITLE_X										112
#define		SKI_MAIN_TITLE_Y										12

#define		SKI_MAIN_TITLE_WIDTH								420

#define		SKI_TOTAL_COST_X										9
#define		SKI_TOTAL_COST_Y										162//159
#define		SKI_TOTAL_COST_WIDTH								73

#define		SKI_TOTAL_VALUE_X										SKI_TOTAL_COST_X
#define		SKI_TOTAL_VALUE_Y										291//268
#define		SKI_TOTAL_VALUE_WIDTH								SKI_TOTAL_COST_WIDTH

#define		SKI_PLAYERS_CURRENT_BALANCE_X				SKI_TOTAL_COST_X
#define		SKI_PLAYERS_CURRENT_BALANCE_Y				235
#define		SKI_PLAYERS_CURRENT_BALANCE_WIDTH		SKI_TOTAL_COST_WIDTH
#define		SKI_PLAYERS_CURRENT_BALANCE_OFFSET_TO_VALUE			265

#define		SKI_PAGE_X													112
#define		SKI_PAGE_Y													70
#define		SKI_PAGE_WIDTH											45
#define		SKI_PAGE_HEIGHT											27


//Number of Inventory slots
#define		SKI_NUM_ARMS_DEALERS_INV_SLOTS			15
#define		SKI_NUM_ARMS_DEALERS_INV_ROWS				3
#define		SKI_NUM_ARMS_DEALERS_INV_COLS				5

//Inventory Slots size and offsets
#define		SKI_INV_SLOT_WIDTH									67
#define		SKI_INV_SLOT_HEIGHT									31
#define		SKI_INV_HEIGHT											SKI_INV_SLOT_HEIGHT - 7
#define		SKI_INV_WIDTH												60

#define		SKI_INV_PRICE_OFFSET_X							1
#define		SKI_INV_PRICE_OFFSET_Y							24

#define		SKI_INV_OFFSET_X										74
#define		SKI_INV_OFFSET_Y										36


// the delay for glow cycling in glow areas, in millisecs
#define SKI_GLOW_DELAY 70

//Start Locations for the inventory boxes
#define		SKI_ARMS_DEALERS_INV_START_X					165
#define		SKI_ARMS_DEALERS_INV_START_Y					30

#define		SKI_ARMS_DEALERS_TRADING_INV_X				91
#define		SKI_ARMS_DEALERS_TRADING_INV_Y				151
#define		SKI_ARMS_DEALERS_TRADING_INV_WIDTH		436
#define		SKI_ARMS_DEALERS_TRADING_INV_HEIGHT		67


#define		SKI_PLAYERS_TRADING_INV_X							91
#define		SKI_PLAYERS_TRADING_INV_Y							266
#define		SKI_PLAYERS_TRADING_INV_HEIGHT				70
#define		SKI_PLAYERS_TRADING_INV_WIDTH					440

#define		SKI_ARMS_DEALER_TOTAL_COST_X					16
#define		SKI_ARMS_DEALER_TOTAL_COST_Y					194//191
#define		SKI_ARMS_DEALER_TOTAL_COST_WIDTH			59
#define		SKI_ARMS_DEALER_TOTAL_COST_HEIGHT			20

#define		SKI_PLAYERS_TOTAL_VALUE_X							16
#define		SKI_PLAYERS_TOTAL_VALUE_Y							310//308
#define		SKI_PLAYERS_TOTAL_VALUE_WIDTH					59
#define		SKI_PLAYERS_TOTAL_VALUE_HEIGHT				20


#define		SKI_TACTICAL_BACKGROUND_START_X				536
#define		SKI_TACTICAL_BACKGROUND_START_Y				0
#define		SKI_DROP_ITEM_TO_GROUND_START_X				SKI_TACTICAL_BACKGROUND_START_X
#define		SKI_DROP_ITEM_TO_GROUND_START_Y				262
#define		SKI_DROP_ITEM_TO_GROUND_TEXT_START_Y	262

#define		SKI_TACTICAL_BACKGROUND_START_WIDTH		640 - SKI_TACTICAL_BACKGROUND_START_X
#define		SKI_TACTICAL_BACKGROUND_START_HEIGHT	340

#define		SKI_ITEM_MOVEMENT_AREA_X							85
#define		SKI_ITEM_MOVEMENT_AREA_Y							263
#define		SKI_ITEM_MOVEMENT_AREA_WIDTH					(640 - SKI_ITEM_MOVEMENT_AREA_X)
//#define		SKI_ITEM_MOVEMENT_AREA_WIDTH					448
#define		SKI_ITEM_MOVEMENT_AREA_HEIGHT					215//72

#define		SKI_DEALER_OFFER_AREA_Y								148
//#define		SKI_DEALER_OFFER_AREA_Y								148


#define		SKI_ITEM_NUMBER_TEXT_OFFSET_X					50
#define		SKI_ITEM_NUMBER_TEXT_OFFSET_Y					15
#define		SKI_ITEM_NUMBER_TEXT_WIDTH						15

#define		SKI_SUBTITLE_TEXT_SIZE								512

#define		SKI_POSITION_SUBTITLES_Y							140//100

#define		SKI_SMALL_FACE_WIDTH									16
#define		SKI_SMALL_FACE_HEIGHT									14
#define		SKI_SMALL_FACE_OFFSET_X								52
#define		SKI_SMALL_FACE_OFFSET_Y								17

#define		SKI_ATTACHMENT_SYMBOL_X_OFFSET				56
#define		SKI_ATTACHMENT_SYMBOL_Y_OFFSET				14


#define		SKI_ATM_PANEL_X												87
#define		SKI_ATM_PANEL_Y												342
#define		SKI_ATM_PANEL_WIDTH										127
#define		SKI_ATM_PANEL_HEIGHT									135

#define		SKI_ATM_BUTTON_X											SKI_ATM_PANEL_X + 23
#define		SKI_ATM_BUTTON_Y											SKI_ATM_PANEL_Y + 64
#define		SKI_ATM_BUTTON_HEIGHT									15
#define		SKI_ATM_NUM_BUTTON_WIDTH							15
#define		SKI_ATM_SIDE_MENU_PANEL_START_X				48

#define		SKI_TRANSFER_STRING_X									SKI_ATM_PANEL_X + 22
#define		SKI_TRANSFER_STRING_Y									SKI_ATM_PANEL_Y + 50
#define		SKI_TRANSFER_STRING_WIDTH							83
#define		SKI_TRANSFER_STRING_HEIGHT						10

#define		SKI_MERCS_MONEY_Y											SKI_ATM_PANEL_Y + 11

#define		SKI_MODE_TEXT_X												SKI_TRANSFER_STRING_X
#define		SKI_MODE_TEXT_Y												SKI_ATM_PANEL_Y + 27
#define		SKI_MODE_TEXT_WIDTH										SKI_TRANSFER_STRING_WIDTH


#define		SKI_MAX_AMOUNT_OF_ITEMS_DEALER_CAN_REPAIR_AT_A_TIME			4

#define		SKI_DEALERS_RANDOM_QUOTE_DELAY				15000
#define		SKI_DEALERS_RANDOM_QUOTE_DELAY_INCREASE_RATE		5000

#define		DELAY_FOR_SHOPKEEPER_IDLE_QUOTE 20000
#define		CHANCE_FOR_SHOPKEEPER_IDLE_QUOTE 40

#define		MAX_SUBOBJECTS_PER_OBJECT					max( MAX_OBJECTS_PER_SLOT, ( 2 + MAX_ATTACHMENTS ) )	// (2nd part is main item, ammo/payload, and 4 attachments)

#define		REALLY_BADLY_DAMAGED_THRESHOLD		30

#define		REPAIR_DELAY_IN_HOURS							6

#define		FLO_DISCOUNT_PERCENTAGE						10


////////////////////////////////////////////
//
//	Global Variables
//
///////////////////////////////////////////


UINT32		guiMainTradeScreenImage;
UINT32		guiCornerWhereTacticalIsStillSeenImage;		//This image is for where the corner of tactical is still seen through the shop keeper interface

//ATM:
//UINT32		guiSkiAtmImage;

BOOLEAN		gfSKIScreenEntry = TRUE;
BOOLEAN		gfSKIScreenExit	= FALSE;
BOOLEAN		gfUserHasRequestedToLeave = FALSE;
//BOOLEAN		gfRedrawSkiScreen = TRUE;

BOOLEAN		gfRenderScreenOnNextLoop = FALSE;

UINT8			gubSkiDirtyLevel = SKI_DIRTY_LEVEL0;
INT32			giSKIMessageBox=-1;

INT8			gbSelectedArmsDealerID = -1;		//Contains the enum value for the currently selected arms dealer

//the quote that is in progress, in certain circumstances, we don't want queuing of related but different quotes
INT32			giShopKeepDialogueEventinProgress = - 1;

//OBJECTTYPE				gpSkiItemPointer;
INVENTORY_IN_SLOT		gMoveingItem;

OBJECTTYPE					*gpHighLightedItemObject=NULL;

BOOLEAN gfResetShopKeepIdleQuote = FALSE;
BOOLEAN gfDoEvaluationAfterOpening = FALSE;

typedef struct
{
	UINT32	uiNumDistinctInventoryItems;
	UINT8		ubCurrentPage;
	UINT8		ubNumberOfPages;

	UINT8		ubFirstItemIndexOnPage;

} SELECTED_ARMS_DEALERS_STATS;



SELECTED_ARMS_DEALERS_STATS		gSelectArmsDealerInfo;


//This pointer is used to store the inventory the arms dealer has for sale
INVENTORY_IN_SLOT *gpTempDealersInventory=NULL;

INVENTORY_IN_SLOT	ArmsDealerOfferArea[ SKI_NUM_TRADING_INV_SLOTS ];
INVENTORY_IN_SLOT	PlayersOfferArea[ SKI_NUM_TRADING_INV_SLOTS ];

OBJECTTYPE	gSubObject[ MAX_SUBOBJECTS_PER_OBJECT ];

BOOLEAN		gfHavePurchasedItemsFromTony = FALSE;

BOOLEAN		gfDealerHasSaidTheEvaluateQuoteOnceThisSession=FALSE;
BOOLEAN		gfAlreadySaidTooMuchToRepair = FALSE;
UINT32		guiRandomQuoteDelayTime=SKI_DEALERS_RANDOM_QUOTE_DELAY;

//Index for the shopkeepers face
INT32			giShopKeeperFaceIndex;

//Id for the popup box
INT32			giPopUpBoxId=-1;

BOOLEAN		gfIsTheShopKeeperTalking;


// the glow for unwanted items
BOOLEAN fDeltaColorForShopkeepUnwanted = TRUE;
INT8 bShopKeepGlowIndex = 9;
BOOLEAN fDoShopkeepGlow = FALSE;


BOOLEAN		gfRemindedPlayerToPickUpHisStuff = FALSE;

BOOLEAN		gfDoneBusinessThisSession = FALSE;

// this is used within SKI exclusively, to handle small faces
UINT8			gubArrayOfEmployedMercs[ MAX_CHARACTER_COUNT ];
UINT32		guiSmallSoldiersFace[ MAX_CHARACTER_COUNT ];
UINT8			gubNumberMercsInArray;

//The subutitled text for what the merc is saying
wchar_t		gsShopKeeperTalkingText[ SKI_SUBTITLE_TEXT_SIZE ];

UINT16		gusPositionOfSubTitlesX=0;

// the transfer funds string
CHAR16 gzSkiAtmTransferString[ 32 ];

BOOLEAN	gfExitSKIDueToMessageBox=FALSE;

OBJECTTYPE	*pShopKeeperItemDescObject=NULL;

UINT32	guiNextFreeInvSlot;

BOOLEAN gfStartWithRepairsDelayedQuote = FALSE;

BOOLEAN gfPerformTransactionInProgress = FALSE;

BOOLEAN gfCommonQuoteUsedThisSession[ NUM_COMMON_SK_QUOTES ];


extern		SOLDIERTYPE			*gpSMCurrentMerc;
extern		MOUSE_REGION		gItemDescAttachmentRegions[MAX_ATTACHMENTS];
extern		MOUSE_REGION		gInvDesc;
extern		BOOLEAN					gfSMDisableForItems;
extern		void						HandleShortCutExitState( void );
extern		UINT8						gubSelectSMPanelToMerc;
extern		INT32						giItemDescAmmoButton;

extern		UINT8 gubLastSpecialItemAddedAtElement;


//Enums for the various Atm modes
enum
{
	SKI_ATM_DISABLED_MODE,
	SKI_ATM_TAKE_MODE,
	SKI_ATM_GIVE_MODE,
	SKI_ATM_ERR_TAKE_MODE,
	SKI_ATM_ERR_GIVE_MODE,
	SKI_ATM_DISPLAY_PLAYERS_BALANCE,
};

UINT8	gubCurrentSkiAtmMode = SKI_ATM_DISABLED_MODE;


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

BOOLEAN gfEvalResultQuoteSaid[ NUM_EVAL_RESULTS ];

UINT32	guiLastTimeDealerSaidNormalEvaluationQuote = 0;

BOOLEAN	gfSkiDisplayDropItemToGroundText = FALSE;

typedef struct
{
	BOOLEAN			fActive;
	OBJECTTYPE	ItemObject;
	INT8 bPreviousInvPos;

} ITEM_TO_ADD_AFTER_SKI_OPEN;
ITEM_TO_ADD_AFTER_SKI_OPEN gItemToAdd;


//Page up buttons for the merchants
void		BtnSKI_InvPageUpButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiSKI_InvPageUpButton;
INT32		guiSKI_InvPageUpButtonImage;

//Page down buttons for the merchants
void		BtnSKI_InvPageDownButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiSKI_InvPageDownButton;
INT32		guiSKI_InvPageDownButtonImage;


/*
//Evaluate:
//Evaluate buttons
void		BtnSKI_EvaluateButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiSKI_EvaluateButton;
INT32		guiSKI_EvaluateButtonImage;
*/

//Transaction buttons
void		BtnSKI_TransactionButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiSKI_TransactionButton;
INT32		guiSKI_TransactionButtonImage;

//Done buttons
void		BtnSKI_DoneButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiSKI_DoneButton;
INT32		guiSKI_DoneButtonImage;

//Atm buttons
void		BtnSKI_AtmButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiSKI_AtmButton[ NUM_SKI_ATM_BUTTONS ];
INT32		guiSKI_AtmNumButtonImage;
INT32		guiSKI_AtmOkButtonImage;
INT32		guiSKI_AtmSideMenuButtonImage;

UINT32 guiItemCrossOut;

BOOLEAN gfDisplayNoRoomMsg = FALSE;

//Blanket the entire screen
MOUSE_REGION		gSKI_EntireScreenMouseRegions;


MOUSE_REGION		gDealersInventoryMouseRegions[ SKI_NUM_ARMS_DEALERS_INV_SLOTS ];
MOUSE_REGION		gRepairmanInventorySmallFaceMouseRegions[ SKI_NUM_ARMS_DEALERS_INV_SLOTS ];
void SelectDealersInventoryRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void SelectDealersInventoryMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

MOUSE_REGION		gDealersOfferSlotsMouseRegions[ SKI_NUM_TRADING_INV_SLOTS ];
void SelectDealersOfferSlotsRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void SelectDealersOfferSlotsMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

MOUSE_REGION		gPlayersOfferSlotsMouseRegions[ SKI_NUM_TRADING_INV_SLOTS ];
void SelectPlayersOfferSlotsRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void SelectPlayersOfferSlotsMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

MOUSE_REGION		gDealersOfferSlotsSmallFaceMouseRegions[ SKI_NUM_TRADING_INV_SLOTS ];
MOUSE_REGION		gPlayersOfferSlotsSmallFaceMouseRegions[ SKI_NUM_TRADING_INV_SLOTS ];



MOUSE_REGION		gSkiInventoryMovementAreaMouseRegions;
//void SelectSkiInventoryMovementAreaRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );


//Mouse region for the subtitles region when the merc is talking
MOUSE_REGION		gShopKeeperSubTitleMouseRegion;
void ShopKeeperSubTitleRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );

//ATM:
//MOUSE_REGION		gShopKeeperCoverTacticalButtonMouseRegion;

MOUSE_REGION		gArmsDealersFaceMouseRegions;
void SelectArmsDealersFaceRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );


//Region to allow the user to drop items to the ground
MOUSE_REGION		gArmsDealersDropItemToGroundMouseRegions;
void SelectArmsDealersDropItemToGroundRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );
void SelectArmsDealersDropItemToGroundMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );


////////////////////////////////////////////
//
//	Local Function Prototypes
//
///////////////////////////////////////////

BOOLEAN		EnterShopKeeperInterface();
BOOLEAN		ExitShopKeeperInterface();
void			HandleShopKeeperInterface();
BOOLEAN		RenderShopKeeperInterface();
void			GetShopKeeperInterfaceUserInput();
void			DestroySkiInventorySlotMouseRegions( );
void			CreateSkiInventorySlotMouseRegions( );
void			InitializeShopKeeper( BOOLEAN fResetPage );
void			CalculateFirstItemIndexOnPage( );
void			DisplayArmsDealerCurrentInventoryPage( );
BOOLEAN		DetermineArmsDealersSellingInventory( );
void			StoreObjectsInNextFreeDealerInvSlot( UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner );
void			AddItemsToTempDealerInventory(UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner );
BOOLEAN		RepairIsDone(UINT16 usItemIndex, UINT8 ubElement);

UINT32		DisplayInvSlot( UINT8 ubSlotNum, UINT16 usItemIndex, UINT16 usPosX, UINT16 usPosY, OBJECTTYPE	*ItemObject, BOOLEAN fHatchedOut, UINT8	ubItemArea );
void			DisplayArmsDealerOfferArea();
INT8			AddItemToArmsDealerOfferArea( INVENTORY_IN_SLOT* pInvSlot, INT8	bSlotIdInOtherLocation );
BOOLEAN		RemoveItemFromArmsDealerOfferArea( INT8	bSlotId, BOOLEAN fKeepItem );
void			SetSkiRegionHelpText( INVENTORY_IN_SLOT *pInv, MOUSE_REGION* pRegion, UINT8 ubScreenArea );
void			SetSkiFaceRegionHelpText( INVENTORY_IN_SLOT *pInv, MOUSE_REGION* pRegion, UINT8 ubScreenArea );
void			RestoreTacticalBackGround();
void			DisplayPlayersOfferArea();
UINT32		CalcShopKeeperItemPrice( BOOLEAN fDealerSelling, BOOLEAN fUnitPriceOnly, UINT16 usItemID, FLOAT dModifier, OBJECTTYPE	*pItemObject );
FLOAT			ItemConditionModifier(UINT16 usItemIndex, INT8 bStatus);
BOOLEAN		RemoveItemFromPlayersOfferArea( INT8 bSlotIdInOtherLocation );

//Evaluate:
//void			EvaluateAllItemsInPlayersOfferArea( );

UINT32		CalculateTotalArmsDealerCost();
UINT32		CalculateTotalPlayersValue();
void			MoveAllArmsDealersItemsInOfferAreaToPlayersOfferArea( );
void			PerformTransaction( UINT32 uiMoneyFromPlayersAccount );
void			MovePlayerOfferedItemsOfValueToArmsDealersInventory( );
INT8			AddInventoryToSkiLocation( INVENTORY_IN_SLOT *pInv, UINT8 ubSpotLocation, UINT8 ubWhere );
BOOLEAN		RemoveItemFromDealersInventory( INVENTORY_IN_SLOT* pInvSlot, UINT8 ubSlot );
BOOLEAN		InitShopKeepersFace( UINT8 ubMercID );
void			DisplayTalkingArmsDealer();
BOOLEAN		StartShopKeeperTalking( UINT16 usQuoteNum );
void			HandleShopKeeperDialog( UINT8 ubInit );
BOOLEAN		IsGunOrAmmoOfSameTypeSelected( OBJECTTYPE	*pItemObject );
void			RemoveShopKeeperSubTitledText();
BOOLEAN		AreThereItemsInTheArmsDealersOfferArea( );
BOOLEAN		AreThereItemsInThePlayersOfferArea( );
void			ShutUpShopKeeper();
UINT8			CountNumberOfValuelessItemsInThePlayersOfferArea( );
UINT8			CountNumberOfItemsOfValueInThePlayersOfferArea( );
UINT8			CountNumberOfItemsInThePlayersOfferArea( );
UINT8			CountNumberOfItemsInTheArmsDealersOfferArea( );
INT8			GetSlotNumberForMerc( UINT8 ubProfile );
void			RenderSkiAtmPanel();
void			RemoveSkiAtmButtons();
void			CreateSkiAtmButtons();
void			AddNumberToSkiAtm( UINT8 ubNumber );
void			HandleSkiAtmPanel( );
void			DisplaySkiAtmTransferString();
void			HandleSkiAtmPanelInput( UINT8	ubButtonPress );
void			EnableDisableSkiAtmButtons();
void			HandleCurrentModeText( UINT8 ubMode );
void			ToggleSkiAtmButtons();
void			HandleAtmOK();
void			EnableDisableDealersInventoryPageButtons();
void			EnableDisableEvaluateAndTransactionButtons();
BOOLEAN		IsMoneyTheOnlyItemInThePlayersOfferArea( );

UINT32		CalculateHowMuchMoneyIsInPlayersOfferArea( );
void			MovePlayersItemsToBeRepairedToArmsDealersInventory();
BOOLEAN		RemoveRepairItemFromDealersOfferArea( INT8	bSlot );

INT8			GetInvSlotOfUnfullMoneyInMercInventory( SOLDIERTYPE *pSoldier );
void			ClearPlayersOfferSlot( INT32 ubSlotToClear );
void			ClearArmsDealerOfferSlot( INT32 ubSlotToClear );
void			ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack( UINT8 bExitValue );
BOOLEAN		DoSkiMessageBox( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT8 ubFlags, MSGBOX_CALLBACK ReturnCallback );
BOOLEAN		WillShopKeeperRejectObjectsFromPlayer( INT8 bDealerId, INT8 bSlotId );
void			CheckAndHandleClearingOfPlayerOfferArea( void );
void			CrossOutUnwantedItems( void );
BOOLEAN		CanShopkeeperOverrideDialogue( void );
INT16			GetNumberOfItemsInPlayerOfferArea( void );
void			HandleCheckIfEnoughOnTheTable( void );
void			InitShopKeeperItemDescBox( OBJECTTYPE *pObject, UINT8 ubPocket, UINT8	ubFromLocation );
void			StartSKIDescriptionBox( void );

BOOLEAN		ShopkeeperAutoPlaceObject( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObj, BOOLEAN fNewItem );
void			ShopkeeperAddItemToPool( INT16 sGridNo, OBJECTTYPE *pObject, INT8 bVisible, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel );

void			IfMercOwnedCopyItemToMercInv( INVENTORY_IN_SLOT *pInv );
void			IfMercOwnedRemoveItemFromMercInv( INVENTORY_IN_SLOT *pInv );
void			IfMercOwnedRemoveItemFromMercInv2( UINT8 ubOwnerProfileId, INT8 bOwnerSlotId );

void			SplitComplexObjectIntoSubObjects( OBJECTTYPE *pComplexObject );
void			CountSubObjectsInObject( OBJECTTYPE *pComplexObject, UINT8 *pubTotalSubObjects, UINT8 *pubRepairableSubObjects, UINT8 *pubNonRepairableSubObjects );
BOOLEAN		AddObjectForEvaluation(OBJECTTYPE *pObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId, BOOLEAN fFirstOne );
BOOLEAN		OfferObjectToDealer( OBJECTTYPE *pComplexObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId );

BOOLEAN		SKITryToReturnInvToOwnerOrCurrentMerc( INVENTORY_IN_SLOT *pInv );
BOOLEAN		SKITryToAddInvToMercsInventory( INVENTORY_IN_SLOT *pInv, SOLDIERTYPE *pSoldier );

void			ExitSKIRequested();
void			EvaluateItemAddedToPlayersOfferArea( INT8 bSlotID, BOOLEAN fFirstOne );
void			ResetAllQuoteSaidFlags();

INVENTORY_IN_SLOT	*GetPtrToOfferSlotWhereThisItemIs( UINT8 ubProfileID, INT8 bInvPocket );

void			DealWithItemsStillOnTheTable();
void			ReturnItemToPlayerSomehow( INVENTORY_IN_SLOT *pInvSlot, SOLDIERTYPE *pDropSoldier );
void			GivePlayerSomeChange( UINT32 uiAmount );

void			HandlePossibleRepairDelays();
BOOLEAN		RepairmanFixingAnyItemsThatShouldBeDoneNow( UINT32 *puiHoursSinceOldestItemRepaired );
void			DelayRepairsInProgressBy( UINT32 uiMinutesDelayed );

BOOLEAN		CanTheDropItemToGroundStringBeDisplayed();
void			DisplayTheSkiDropItemToGroundString();

UINT32		EvaluateInvSlot( INVENTORY_IN_SLOT *pInvSlot );

void			BuildItemHelpTextString( wchar_t sString[], size_t Length, INVENTORY_IN_SLOT *pInv, UINT8 ubScreenArea );
void			BuildRepairTimeString( wchar_t sString[], size_t Length, UINT32 uiTimeInMinutesToFixItem );
void			BuildDoneWhenTimeString( wchar_t sString[], size_t Length, UINT8 ubArmsDealer, UINT16 usItemIndex, UINT8 ubElement );

void DisableAllDealersInventorySlots( void );
void EnableAllDealersInventorySlots( void );
void DisableAllDealersOfferSlots( void );
void EnableAllDealersOfferSlots( void );

void HatchOutInvSlot( UINT16 usPosX, UINT16 usPosY );


extern BOOLEAN ItemIsARocketRifle( INT16 sItemIndex );


#ifdef JA2TESTVERSION
BOOLEAN gfTestDisplayDealerCash = FALSE;
void DisplayAllDealersCash();
#endif

//ppp



//
// screen handler functions
//

UINT32	ShopKeeperScreenInit()
{
	//Set so next time we come in, we can set up
	gfSKIScreenEntry = TRUE;


	return( TRUE );
}


UINT32	ShopKeeperScreenHandle()
{
	StartFrameBufferRender();

	if( gfSKIScreenEntry )
	{
		PauseGame();

		if( !EnterShopKeeperInterface() )
		{
			gfSKIScreenExit = TRUE;
			EnterTacticalScreen( );

			return( SHOPKEEPER_SCREEN );
		}
		else
		{
			gfSKIScreenEntry = FALSE;
			gfSKIScreenExit = FALSE;
		}
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		gfRenderScreenOnNextLoop = TRUE;
		InvalidateRegion( 0, 0, 640, 480 );
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
//ATM:
	DisableSMPpanelButtonsWhenInShopKeeperInterface( FALSE );
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
		// ARM: message is delayed because we need the mouse restriction to be in place BEFORE it comes up so it gets lifted/restored
		DoSkiMessageBox( MSG_BOX_BASIC_STYLE, SKI_Text[ SKI_TEXT_NO_MORE_ROOM_IN_PLAYER_OFFER_AREA ], SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK, NULL );

		gfDisplayNoRoomMsg = FALSE;
	}


	return( SHOPKEEPER_SCREEN );
}



UINT32	ShopKeeperScreenShutdown()
{
	ShutDownArmsDealers();

	return( TRUE );
}





//
//
//





BOOLEAN EnterShopKeeperInterface()
{
	UINT8						ubCnt;
	CHAR8						zTemp[32];
	SOLDIERTYPE			*pSoldier;


	// make sure current merc is close enough and eligible to talk to the shopkeeper.
	AssertMsg( CanMercInteractWithSelectedShopkeeper( MercPtrs[ gusSelectedSoldier ] ), "Selected merc can't interact with shopkeeper.  Send save AM-1");


	// Create a video surface to blt corner of the tactical screen that still shines through
	guiCornerWhereTacticalIsStillSeenImage = AddVideoSurface(SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT, 16);
	if (guiCornerWhereTacticalIsStillSeenImage == NO_VSURFACE)
	{
		#ifdef JA2BETAVERSION
			ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Failed to create Surface where tactical map shows through" );
		#endif

		return( FALSE );
	}


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
	SetCurrentTacticalPanelCurrentMerc( (UINT8)gusSelectedSoldier );
	SetSMPanelCurrentMerc( (UINT8)gusSelectedSoldier );

	// load the Main trade screen backgroiund image
	if (!AddVideoObjectFromFile("InterFace\\TradeScreen.sti", &guiMainTradeScreenImage))
	{
#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Failed to load TradeScreen.sti" );
#endif

		return( FALSE );
	}


	// load the Main trade screen background image
	if (!AddVideoObjectFromFile("InterFace\\itemcrossout.sti", &guiItemCrossOut))
	{
#ifdef JA2BETAVERSION
		ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Failed to load itemcrossout.sti" );
#endif
		return( FALSE );
	}



/*
ATM:
	// load the Main trade screen backgroiund image
	CHECKF(AddVideoObjectFromFile("InterFace\\TradeScreenAtm.sti", &guiSkiAtmImage));
*/

	//Create an array of all mercs (anywhere!) currently in the player's employ, and load their small faces
	// This is to support showing of repair item owner's faces even when they're not in the sector, as long as they still work for player
	gubNumberMercsInArray = 0;
	for( ubCnt = gTacticalStatus.Team[ OUR_TEAM ].bFirstID; ubCnt <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; ubCnt++ )
	{
		pSoldier = MercPtrs[ ubCnt ];

		if( pSoldier->bActive && ( pSoldier->ubProfile != NO_PROFILE ) &&
			!(pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && !AM_A_ROBOT( pSoldier ) )
		{
			// remember whose face is in this slot
			gubArrayOfEmployedMercs[ gubNumberMercsInArray ] = pSoldier->ubProfile;

			//Create the string for the face file name
			sprintf( zTemp, "FACES\\33FACE\\%02d.sti", gMercProfiles[ pSoldier->ubProfile ].ubFaceIndex );

			//While we are at it, add their small face
			if (!AddVideoObjectFromFile(zTemp, &guiSmallSoldiersFace[gubNumberMercsInArray]))
			{
				#ifdef JA2BETAVERSION
						ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Failed to load %s", zTemp );
				#endif
				return( FALSE );
			}

			gubNumberMercsInArray++;
		}
	}

	//Load the graphic for the arrow button
	guiSKI_InvPageUpButtonImage = LoadButtonImage("INTERFACE\\TradeScrollArrows.sti", -1,0,-1,1,-1 );
	guiSKI_InvPageDownButtonImage = UseLoadedButtonImage( guiSKI_InvPageUpButtonImage, -1,2,-1,3,-1 );


	//Page up button for the merchant inventory
	guiSKI_InvPageUpButton = QuickCreateButton( guiSKI_InvPageUpButtonImage, SKI_PAGE_UP_ARROWS_X, SKI_PAGE_UP_ARROWS_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGHEST,
										DEFAULT_MOVE_CALLBACK, BtnSKI_InvPageUpButtonCallback );
	SpecifyDisabledButtonStyle( guiSKI_InvPageUpButton, DISABLED_STYLE_HATCHED );

	//Page down button for the merchant inventory
	guiSKI_InvPageDownButton = QuickCreateButton( guiSKI_InvPageDownButtonImage, SKI_PAGE_DOWN_ARROWS_X, SKI_PAGE_DOWN_ARROWS_Y,
										BUTTON_NEWTOGGLE, MSYS_PRIORITY_HIGHEST,
										DEFAULT_MOVE_CALLBACK, BtnSKI_InvPageDownButtonCallback );
	SpecifyDisabledButtonStyle( guiSKI_InvPageDownButton, DISABLED_STYLE_HATCHED );



//Evaluate:
	//	guiSKI_EvaluateButtonImage = LoadButtonImage("INTERFACE\\TradeButtons.sti", -1,0,-1,1,-1 );
//	guiSKI_TransactionButtonImage = UseLoadedButtonImage( guiSKI_EvaluateButtonImage, -1,0,-1,1,-1 );
	guiSKI_TransactionButtonImage = LoadButtonImage("INTERFACE\\TradeButtons.sti", -1,0,-1,1,-1 );
	guiSKI_DoneButtonImage = UseLoadedButtonImage( guiSKI_TransactionButtonImage, -1,0,-1,1,-1 );

/*
//Evaluate:
	//Evaluate button
	guiSKI_EvaluateButton = CreateIconAndTextButton( guiSKI_EvaluateButtonImage, SKI_Text[SKI_TEXT_EVALUATE], SKI_BUTTON_FONT,
													 SKI_BUTTON_COLOR, DEFAULT_SHADOW,
													 SKI_BUTTON_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 SKI_EVALUATE_BUTTON_X, SKI_EVALUATE_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnSKI_EvaluateButtonCallback);
	SpecifyDisabledButtonStyle( guiSKI_EvaluateButton, DISABLED_STYLE_HATCHED );
*/

	//Transaction button
	guiSKI_TransactionButton = CreateIconAndTextButton( guiSKI_TransactionButtonImage, SKI_Text[SKI_TEXT_TRANSACTION], SKI_BUTTON_FONT,
													 SKI_BUTTON_COLOR, DEFAULT_SHADOW,
													 SKI_BUTTON_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 SKI_TRANSACTION_BUTTON_X, SKI_TRANSACTION_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnSKI_TransactionButtonCallback);
	SpecifyDisabledButtonStyle( guiSKI_TransactionButton, DISABLED_STYLE_HATCHED );

	//if the dealer repairs, use the repair fast help text for the transaction button
	if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		SetButtonFastHelpText( guiSKI_TransactionButton, SkiMessageBoxText[ SKI_REPAIR_TRANSACTION_BUTTON_HELP_TEXT ] );
	else
		SetButtonFastHelpText( guiSKI_TransactionButton, SkiMessageBoxText[ SKI_TRANSACTION_BUTTON_HELP_TEXT ] );


	//Done button
	guiSKI_DoneButton = CreateIconAndTextButton( guiSKI_DoneButtonImage, SKI_Text[SKI_TEXT_DONE], SKI_BUTTON_FONT,
													 SKI_BUTTON_COLOR, DEFAULT_SHADOW,
													 SKI_BUTTON_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 SKI_DONE_BUTTON_X, SKI_DONE_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH+10,
													 DEFAULT_MOVE_CALLBACK, BtnSKI_DoneButtonCallback);
	SpecifyDisabledButtonStyle( guiSKI_DoneButton, DISABLED_STYLE_HATCHED );
	SetButtonFastHelpText( guiSKI_DoneButton, SkiMessageBoxText[ SKI_DONE_BUTTON_HELP_TEXT ] );


	//Blanket the entire screen
	MSYS_DefineRegion( &gSKI_EntireScreenMouseRegions, 0, 0, 639, 339, MSYS_PRIORITY_HIGH-2,
						 CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
	MSYS_AddRegion( &gSKI_EntireScreenMouseRegions );




/*
//ATM:
	//Blanket the tactical buttons where the ATM will go
	MSYS_DefineRegion( &gShopKeeperCoverTacticalButtonMouseRegion, SKI_ATM_PANEL_X, SKI_ATM_PANEL_Y, (UINT16)(SKI_ATM_PANEL_X+SKI_ATM_PANEL_WIDTH), (UINT16)(SKI_ATM_PANEL_Y+SKI_ATM_PANEL_HEIGHT), MSYS_PRIORITY_HIGH-1,
						 CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);
	MSYS_AddRegion( &gSKI_EntireScreenMouseRegions );
*/

	//Create the mouse regions for the inventory slot
	CreateSkiInventorySlotMouseRegions( );



	//Create the mouse region to limit the movement of the item cursos
	MSYS_DefineRegion( &gSkiInventoryMovementAreaMouseRegions, SKI_ITEM_MOVEMENT_AREA_X, SKI_ITEM_MOVEMENT_AREA_Y, (UINT16)(SKI_ITEM_MOVEMENT_AREA_X+SKI_ITEM_MOVEMENT_AREA_WIDTH), (UINT16)(SKI_ITEM_MOVEMENT_AREA_Y+SKI_ITEM_MOVEMENT_AREA_HEIGHT), MSYS_PRIORITY_HIGH-1,
				CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK ); //SelectSkiInventoryMovementAreaRegionCallBack
	MSYS_AddRegion( &gSkiInventoryMovementAreaMouseRegions );

	//Disable the region that limits the movement of the cursor with the item
	MSYS_DisableRegion( &gSkiInventoryMovementAreaMouseRegions );


	//Create the mouse region for the shopkeeper's face
	MSYS_DefineRegion( &gArmsDealersFaceMouseRegions, SKI_FACE_X, SKI_FACE_Y, (UINT16)(SKI_FACE_X+SKI_FACE_WIDTH), (UINT16)(SKI_FACE_Y+SKI_FACE_HEIGHT), MSYS_PRIORITY_HIGH-1,
				CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectArmsDealersFaceRegionCallBack );
	MSYS_AddRegion( &gArmsDealersFaceMouseRegions );


	//Create the atm button
//ATM:
	//	CreateSkiAtmButtons();

	memset( ArmsDealerOfferArea, 0, sizeof( INVENTORY_IN_SLOT ) * SKI_NUM_TRADING_INV_SLOTS );
	memset( PlayersOfferArea, 0, sizeof( INVENTORY_IN_SLOT ) * SKI_NUM_TRADING_INV_SLOTS );


	if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
	{
		HandlePossibleRepairDelays();
	}


	//Setup the currently selected arms dealer
	InitializeShopKeeper( TRUE );


	//Set the flag indicating that we are in the shop keeper interface
	guiTacticalInterfaceFlags |= INTERFACE_SHOPKEEP_INTERFACE;

	memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );

	memset( &gfCommonQuoteUsedThisSession, FALSE, sizeof( gfCommonQuoteUsedThisSession ) );

	//Init the shopkeepers face
	InitShopKeepersFace( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubShopKeeperID );

	gfDoneBusinessThisSession = FALSE;

	//Call this to set the fact that we just entered the screen
	HandleShopKeeperDialog( 1 );

	ResetAllQuoteSaidFlags();

	//Reset the highlighted item pointer;
	gpHighLightedItemObject = NULL;

	//Reset
	gfRemindedPlayerToPickUpHisStuff = FALSE;
	gfUserHasRequestedToLeave = FALSE;
	gubCurrentSkiAtmMode = SKI_ATM_DISABLED_MODE;
	gfDisplayNoRoomMsg = FALSE;

	//Disable the map radar region
	MSYS_DisableRegion(&gRadarRegion);

	gfDoEvaluationAfterOpening = FALSE;

	if( gItemToAdd.fActive )
	{
		BOOLEAN fAddedOK = FALSE;
		INT8 bSlotNum = gItemToAdd.bPreviousInvPos;

		//if this is NOT a repair dealer or he is is but there is enough space in the player's offer area
		// (you can't be out of space if it isn't a repairman, only they can fill it up with repaired items!)
		if( ( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_REPAIRS ) ||
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
			CopyObj( &gItemToAdd.ItemObject, &Menptr[ gpSMCurrentMerc->ubID ].inv[ gItemToAdd.bPreviousInvPos ] );
		}

		//Clear the contents of the structure
		memset( &gItemToAdd, 0, sizeof( ITEM_TO_ADD_AFTER_SKI_OPEN ) );
		gItemToAdd.fActive = FALSE;
	}

	// Dirty the bottom panel
	fInterfacePanelDirty = DIRTYLEVEL2;


	gfDealerHasSaidTheEvaluateQuoteOnceThisSession = FALSE;
	guiRandomQuoteDelayTime = SKI_DEALERS_RANDOM_QUOTE_DELAY;

	pShopKeeperItemDescObject = NULL;


//Region to allow the user to drop items to the ground
	MSYS_DefineRegion( &gArmsDealersDropItemToGroundMouseRegions, SKI_DROP_ITEM_TO_GROUND_START_X, SKI_DROP_ITEM_TO_GROUND_START_Y, 639, 339, MSYS_PRIORITY_HIGH,
						 CURSOR_NORMAL, SelectArmsDealersDropItemToGroundMovementRegionCallBack, SelectArmsDealersDropItemToGroundRegionCallBack );
//						 CURSOR_NORMAL, MSYS_NO_CALLBACK, SelectArmsDealersDropItemToGroundRegionCallBack );
	MSYS_AddRegion( &gArmsDealersDropItemToGroundMouseRegions );

	gfSkiDisplayDropItemToGroundText = FALSE;

	// by default re-enable calls to PerformTransaction()
	gfPerformTransactionInProgress = FALSE;

	return( TRUE );
}


BOOLEAN InitShopKeepersFace( UINT8 ubMercID )
{
	SOLDIERTYPE *pSoldier = FindSoldierByProfileID( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubShopKeeperID, FALSE );

	if( pSoldier == NULL )
	{
		#ifdef JA2TESTVERSION
			//Create the facial index
			giShopKeeperFaceIndex = InitFace( ubMercID, NOBODY, FACE_BIGFACE );
		#else
			return( FALSE );
		#endif
	}
	else
	{
		//Create the facial index
		giShopKeeperFaceIndex = InitFace( ubMercID, pSoldier->ubID, FACE_BIGFACE );
	}


	SetAutoFaceActive( FRAME_BUFFER, FACE_AUTO_RESTORE_BUFFER, giShopKeeperFaceIndex, SKI_FACE_X, SKI_FACE_Y );

	//Set it so the face cannot be set InActive
	gFacesData[ giShopKeeperFaceIndex ].uiFlags |= FACE_INACTIVE_HANDLED_ELSEWHERE;


	RenderAutoFace( giShopKeeperFaceIndex );

	return(TRUE);
}


BOOLEAN ExitShopKeeperInterface()
{
	UINT8	ubCnt;

	if( gfExitSKIDueToMessageBox )
	{
		gfSKIScreenExit = FALSE;

//		gfExitSKIDueToMessageBox = FALSE;
	}

	if( InItemDescriptionBox( ) && pShopKeeperItemDescObject != NULL )
	{
		DeleteItemDescriptionBox( );
	}

	FreeMouseCursor();

	//Delete the main shopkeep background
	DeleteVideoObjectFromIndex( guiMainTradeScreenImage );
	DeleteVideoObjectFromIndex( guiItemCrossOut );
	DeleteVideoSurfaceFromIndex( guiCornerWhereTacticalIsStillSeenImage );

	ShutUpShopKeeper();

	//Delete the Atm backgorund
//ATM:
//	DeleteVideoObjectFromIndex( guiSkiAtmImage );

	UnloadButtonImage( guiSKI_InvPageUpButtonImage );
	UnloadButtonImage( guiSKI_InvPageDownButtonImage );

	UnloadButtonImage( guiSKI_TransactionButtonImage );
	//	UnloadButtonImage( guiSKI_EvaluateButtonImage );
	UnloadButtonImage( guiSKI_DoneButtonImage );

	//loop through the area and delete small faces
	for(ubCnt=0; ubCnt<gubNumberMercsInArray; ubCnt++)
	{
		DeleteVideoObjectFromIndex( guiSmallSoldiersFace[ ubCnt ] );
	}

	RemoveButton( guiSKI_InvPageUpButton );
	RemoveButton( guiSKI_InvPageDownButton );

//Evaluate:
	//	RemoveButton( guiSKI_EvaluateButton );
	RemoveButton( guiSKI_TransactionButton );
	RemoveButton( guiSKI_DoneButton );

	MSYS_RemoveRegion( &gSKI_EntireScreenMouseRegions);

	MSYS_RemoveRegion( &gSkiInventoryMovementAreaMouseRegions );

//ATM:
//	MSYS_RemoveRegion( &gShopKeeperCoverTacticalButtonMouseRegion );

	//Remove the region for the face
	MSYS_RemoveRegion( &gArmsDealersFaceMouseRegions );

	//Remove the Atm buttons
//ATM:
	//	RemoveSkiAtmButtons();


	//Region to allow the user to drop items to the ground
	MSYS_RemoveRegion( &gArmsDealersDropItemToGroundMouseRegions );

	//Destroy the mouse regions for the inventory slots
	DestroySkiInventorySlotMouseRegions( );

	//if there is a temp inventory array, destroy it
	if( gpTempDealersInventory )
	{
		MemFree( gpTempDealersInventory );
		gpTempDealersInventory = NULL;
	}

	//Set the flag indicating that we are NOT in the shop keeper interface
	guiTacticalInterfaceFlags &= ~INTERFACE_SHOPKEEP_INTERFACE;

	//if there is a subtitles box up, remove it
	RemoveShopKeeperSubTitledText();

	//Get rid of the ShopKeeper face
	DeleteFace( giShopKeeperFaceIndex );

	//Enable the map region
	MSYS_EnableRegion(&gRadarRegion);

	gfSMDisableForItems = FALSE;

	return( TRUE );
}



void HandleShopKeeperInterface()
{
	UINT8 ubStatusOfSkiRenderDirtyFlag = gubSkiDirtyLevel;

	INT32 iCounter = 0;


	//if we are in the item desc panel, disable the buttons
	if( InItemDescriptionBox( ) && pShopKeeperItemDescObject != NULL )
	{
		DisableSMPpanelButtonsWhenInShopKeeperInterface( FALSE );
		DisableButton( guiSKI_InvPageUpButton );
		DisableButton( guiSKI_InvPageDownButton );
		DisableButton( guiSKI_TransactionButton );
		DisableButton( guiSKI_DoneButton );

		//make sure the buttons dont render
//		ButtonList[ guiSKI_InvPageUpButton ]->uiFlags |= BUTTON_FORCE_UNDIRTY;
//		ButtonList[ guiSKI_InvPageDownButton ]->uiFlags |= BUTTON_FORCE_UNDIRTY;
		ButtonList[ guiSKI_TransactionButton ]->uiFlags |= BUTTON_FORCE_UNDIRTY;
		ButtonList[ guiSKI_DoneButton ]->uiFlags |= BUTTON_FORCE_UNDIRTY;


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

	RenderClock( CLOCK_X, CLOCK_Y );
	RenderTownIDString( );

//ATM:
//	RenderSkiAtmPanel();

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

#ifdef JA2TESTVERSION
	if ( gfTestDisplayDealerCash )
		DisplayAllDealersCash();
#endif

	//if the Ski dirty flag was changed to a lower value, make sure it is set properly
	if( ubStatusOfSkiRenderDirtyFlag > gubSkiDirtyLevel )
		gubSkiDirtyLevel = ubStatusOfSkiRenderDirtyFlag;

	//if the merc is talking and there is an item currently being highlighted
	// ( this gets rid of the item burning through the dealers text box )
	if( gfIsTheShopKeeperTalking )
		if( gpHighLightedItemObject != NULL || gubSkiDirtyLevel != SKI_DIRTY_LEVEL0 )
			RenderMercPopUpBoxFromIndex( giPopUpBoxId, gusPositionOfSubTitlesX, SKI_POSITION_SUBTITLES_Y, FRAME_BUFFER);


	//if we are to display the drop item to ground text
	if( gfSkiDisplayDropItemToGroundText )
	{
		DisplayTheSkiDropItemToGroundString();
	}
}


BOOLEAN RenderShopKeeperInterface()
{
	CHAR16	zMoney[128];
	SGPRect		SrcRect;


	if( InItemDescriptionBox( ) && pShopKeeperItemDescObject != NULL )
	{
		return( TRUE );
	}


//	RenderSMPanel( &fDirty );

//	RenderTacticalInterface( );
	// Render view window
//	RenderRadarScreen( );

	BltVideoObjectFromIndex(FRAME_BUFFER, guiMainTradeScreenImage, 0, SKI_MAIN_BACKGROUND_X, SKI_MAIN_BACKGROUND_Y);

	//Display the Title
	DrawTextToScreen( SKI_Text[ SKI_TEXT_MERCHADISE_IN_STOCK ], SKI_MAIN_TITLE_X, SKI_MAIN_TITLE_Y, SKI_MAIN_TITLE_WIDTH, SKI_TITLE_FONT, SKI_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

	//if the dealer repairs
	if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
	{
		//Display the Repair cost text
		DisplayWrappedString( SKI_TOTAL_COST_X, SKI_TOTAL_COST_Y, SKI_TOTAL_COST_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[ SKI_TEXT_REPAIR_COST ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
	}
	else
	{
		//Display the Total cost text
		DisplayWrappedString( SKI_TOTAL_COST_X, SKI_TOTAL_COST_Y, SKI_TOTAL_COST_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[ SKI_TEXT_TOTAL_COST ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
	}

	//Display the total value text
	DisplayWrappedString( SKI_TOTAL_VALUE_X, SKI_TOTAL_VALUE_Y, SKI_TOTAL_VALUE_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[ SKI_TEXT_TOTAL_VALUE ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );


	//Display the players current balance text
	DisplayWrappedString( SKI_PLAYERS_CURRENT_BALANCE_X, SKI_PLAYERS_CURRENT_BALANCE_Y, SKI_PLAYERS_CURRENT_BALANCE_WIDTH, 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SkiMessageBoxText[ SKI_PLAYERS_CURRENT_BALANCE ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

	//Display the players current balance value
	swprintf( zMoney, lengthof(zMoney), L"%d", LaptopSaveInfo.iCurrentBalance );

	InsertCommasForDollarFigure( zMoney );
	InsertDollarSignInToString( zMoney );
	DrawTextToScreen( zMoney, SKI_PLAYERS_CURRENT_BALANCE_X, SKI_PLAYERS_CURRENT_BALANCE_OFFSET_TO_VALUE, SKI_PLAYERS_CURRENT_BALANCE_WIDTH, FONT10ARIAL, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, TRUE, CENTER_JUSTIFIED );

	BlitBufferToBuffer( guiRENDERBUFFER, guiSAVEBUFFER, 0, 0, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_HEIGHT );

	//At this point the background is pure, copy it to the save buffer
	if( gfRenderScreenOnNextLoop )
	{
	//	BlitBufferToBuffer( guiRENDERBUFFER, guiCornerWhereTacticalIsStillSeenImage, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT );
		HVSURFACE hDestVSurface = GetVideoSurface(guiCornerWhereTacticalIsStillSeenImage);
		HVSURFACE hSrcVSurface  = GetVideoSurface(guiSAVEBUFFER);

		SrcRect.iLeft = SKI_TACTICAL_BACKGROUND_START_X;
		SrcRect.iTop = SKI_TACTICAL_BACKGROUND_START_Y;
		SrcRect.iRight = SKI_TACTICAL_BACKGROUND_START_X + SKI_TACTICAL_BACKGROUND_START_WIDTH;
		SrcRect.iBottom = SKI_TACTICAL_BACKGROUND_START_Y + SKI_TACTICAL_BACKGROUND_START_HEIGHT;

		BltVSurfaceUsingDD(hDestVSurface, hSrcVSurface, 0, 0, &SrcRect);

		gfRenderScreenOnNextLoop = FALSE;
	}

	DisplayArmsDealerCurrentInventoryPage( );

	DisplayArmsDealerOfferArea();

	DisplayPlayersOfferArea();

	//if the merc is talking and the screen has been dirtied
	if( gfIsTheShopKeeperTalking )
		RenderMercPopUpBoxFromIndex( giPopUpBoxId, gusPositionOfSubTitlesX, SKI_POSITION_SUBTITLES_Y, FRAME_BUFFER);

	CrossOutUnwantedItems( );

	//Render the clock and the town name
	RenderClock( CLOCK_X, CLOCK_Y );
	RenderTownIDString( );

//	RenderTacticalInterface( );
//	RenderSMPanel( &fDirty );

	//Restore the tactical background that is visble behind the SKI panel
	RestoreTacticalBackGround();

	InvalidateRegion( 0, 0, 640, 480 );

	return( TRUE );
}

void RestoreTacticalBackGround()
{
	SGPRect		SrcRect;

	//Restore the background before blitting the text back on
//	RestoreExternBackgroundRect( SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT );

//	BlitBufferToBuffer( guiCornerWhereTacticalIsStillSeenImage, guiRENDERBUFFER, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, SKI_TACTICAL_BACKGROUND_START_WIDTH, SKI_TACTICAL_BACKGROUND_START_HEIGHT );

	HVSURFACE hDestVSurface = GetVideoSurface(guiRENDERBUFFER);
	HVSURFACE hSrcVSurface  = GetVideoSurface(guiCornerWhereTacticalIsStillSeenImage);

	SrcRect.iLeft = 0;
	SrcRect.iTop = 0;
	SrcRect.iRight = SKI_TACTICAL_BACKGROUND_START_WIDTH;
	SrcRect.iBottom = SKI_TACTICAL_BACKGROUND_START_HEIGHT;


	BltVSurfaceUsingDD(hDestVSurface, hSrcVSurface, SKI_TACTICAL_BACKGROUND_START_X, SKI_TACTICAL_BACKGROUND_START_Y, &SrcRect);

	InvalidateRegion( 0, 0, 640, 480 );
}



void		GetShopKeeperInterfaceUserInput()
{
	InputAtom Event;
	POINT MousePos;

	GetCursorPos(&MousePos);

	while( DequeueEvent( &Event ) )
	{
		// HOOK INTO MOUSE HOOKS
		switch( Event.usEvent)
		{
		}

		if( !HandleTextInput( &Event ) && Event.usEvent == KEY_DOWN )
		{

/*
//ATM:

			//if the number entered was a number
			if( (Event.usParam >= '0' ) && ( Event.usParam <= '9') )
			{
				UINT8 ubValue = 0;
				ubValue = ( INT8 )( Event.usParam - '0' );

				AddNumberToSkiAtm( ubValue );
			}
*/

			switch( Event.usParam )
			{
				case ESC:
					// clean exits - does quotes, shuts up shopkeeper, etc.
					ExitSKIRequested();

#ifdef JA2TESTVERSION
					//Instant exit - doesn't clean up much
//					gfSKIScreenExit = TRUE;
#endif
					break;

				case 'x':
					if( Event.usKeyState & ALT_DOWN )
					{
						HandleShortCutExitState( );
					}
					break;

				case SPACE:
					{
						UINT8	ubID;

						DeleteItemDescriptionBox( );

						// skip Robot and EPCs
						ubID = FindNextActiveAndAliveMerc( gpSMCurrentMerc, FALSE, TRUE );

						gubSelectSMPanelToMerc = ubID;

						LocateSoldier( ubID, DONTSETLOCATOR );
						// refresh background for player slots (in case item values change due to Flo's discount)
						gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
					}
					break;

#ifdef JA2TESTVERSION

				case 'r':
					gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
					break;
				case 'i':
					InvalidateRegion( 0, 0, 640, 480 );
					break;

				case 'd':
					//Test key to toggle the display of the money each dealer has on hand
					gfTestDisplayDealerCash ^= 1;
					break;

#endif
			}
		}
	}
}



#ifdef JA2TESTVERSION
void DisplayAllDealersCash()
{
	INT8		bArmsDealer;
	UINT16	usPosY=0;
	CHAR16	zTemp[512];
	UINT8		ubForeColor;

	//loop through all the shopkeeper's and display their money
	for( bArmsDealer=0; bArmsDealer<NUM_ARMS_DEALERS; bArmsDealer++ )
	{
		//Display the shopkeeper's name
		DrawTextToScreen( gMercProfiles[ ArmsDealerInfo[ bArmsDealer ].ubShopKeeperID ].zNickname, 540, usPosY, 0, FONT10ARIAL, SKI_TITLE_COLOR, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED );

		//Display the arms dealer cash on hand
		swprintf(zTemp, lengthof(zTemp), L"%d", gArmsDealerStatus[ bArmsDealer ].uiArmsDealersCash );

		InsertCommasForDollarFigure( zTemp );
		InsertDollarSignInToString( zTemp );
		ubForeColor = ( UINT8 ) ( ( bArmsDealer == gbSelectedArmsDealerID ) ? SKI_BUTTON_COLOR : SKI_TITLE_COLOR );
		DrawTextToScreen( zTemp, 590, usPosY, 0, FONT10ARIAL, ubForeColor, FONT_MCOLOR_BLACK, TRUE, LEFT_JUSTIFIED );
		usPosY += 17;
	}
}
#endif

void BtnSKI_InvPageUpButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );

		if( gSelectArmsDealerInfo.ubCurrentPage > 1 )
		{
			gSelectArmsDealerInfo.ubCurrentPage--;
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			EnableDisableDealersInventoryPageButtons();
		}

		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}


void BtnSKI_InvPageDownButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );

		if( gSelectArmsDealerInfo.ubCurrentPage < gSelectArmsDealerInfo.ubNumberOfPages )
		{
			gSelectArmsDealerInfo.ubCurrentPage++;
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			EnableDisableDealersInventoryPageButtons();
		}

		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}

/*
//Evaluate:
void BtnSKI_EvaluateButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );

		ShutUpShopKeeper();
		EvaluateAllItemsInPlayersOfferArea( );

		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}
*/
void BtnSKI_TransactionButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);

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

void BtnSKI_DoneButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		ExitSKIRequested();
	}

	InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
}

void CreateSkiInventorySlotMouseRegions( )
{
	UINT8	x,y,ubCnt;
	UINT16	usPosX, usPosY;


	// Create the mouse regions for the shopkeeper's inventory
	usPosY = SKI_ARMS_DEALERS_INV_START_Y;
	ubCnt=0;
	for(y=0; y<SKI_NUM_ARMS_DEALERS_INV_ROWS; y++)
	{
		usPosX = SKI_ARMS_DEALERS_INV_START_X;

		for(x=0; x<SKI_NUM_ARMS_DEALERS_INV_COLS; x++)
		{
			MSYS_DefineRegion( &gDealersInventoryMouseRegions[ ubCnt ], usPosX, usPosY, (INT16)(usPosX + SKI_INV_SLOT_WIDTH), (INT16)(usPosY + SKI_INV_SLOT_HEIGHT), MSYS_PRIORITY_HIGH,
										 CURSOR_NORMAL, SelectDealersInventoryMovementRegionCallBack, SelectDealersInventoryRegionCallBack );
			MSYS_AddRegion( &gDealersInventoryMouseRegions[ubCnt] );
			MSYS_SetRegionUserData( &gDealersInventoryMouseRegions[ ubCnt ], 0, ubCnt);
			MSYS_SetRegionUserData( &gDealersInventoryMouseRegions[ ubCnt ], 1, ARMS_DEALER_INVENTORY );

			//if the dealer repairs
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
			{
				//Small Faces
				MSYS_DefineRegion( &gRepairmanInventorySmallFaceMouseRegions[ ubCnt ], (UINT16)(usPosX+SKI_SMALL_FACE_OFFSET_X), (UINT16)(usPosY), (INT16)(usPosX + SKI_SMALL_FACE_OFFSET_X + SKI_SMALL_FACE_WIDTH), (INT16)(usPosY+ SKI_SMALL_FACE_HEIGHT), MSYS_PRIORITY_HIGH+1,
											 CURSOR_NORMAL, NULL, NULL );
				MSYS_AddRegion( &gRepairmanInventorySmallFaceMouseRegions[ ubCnt ] );
				MSYS_SetRegionUserData( &gRepairmanInventorySmallFaceMouseRegions[ ubCnt ], 0, ubCnt );
				MSYS_SetRegionUserData( &gRepairmanInventorySmallFaceMouseRegions[ ubCnt ], 1, ARMS_DEALER_INVENTORY );
			}

			usPosX += SKI_INV_OFFSET_X;
			ubCnt++;
		}
		usPosY += SKI_INV_OFFSET_Y;
	}

	// Create the mouse regions for the shopkeeper's trading slots
	usPosY = SKI_ARMS_DEALERS_TRADING_INV_Y;
	ubCnt=0;
	for(y=0; y<SKI_NUM_TRADING_INV_ROWS; y++)
	{
		usPosX = SKI_ARMS_DEALERS_TRADING_INV_X;

		for(x=0; x<SKI_NUM_TRADING_INV_COLS; x++)
		{
			MSYS_DefineRegion( &gDealersOfferSlotsMouseRegions[ ubCnt ], usPosX, usPosY, (INT16)(usPosX + SKI_INV_SLOT_WIDTH), (INT16)(usPosY + SKI_INV_SLOT_HEIGHT), MSYS_PRIORITY_HIGH,
										 CURSOR_NORMAL, SelectDealersOfferSlotsMovementRegionCallBack, SelectDealersOfferSlotsRegionCallBack );
			MSYS_AddRegion( &gDealersOfferSlotsMouseRegions[ubCnt] );
			MSYS_SetRegionUserData( &gDealersOfferSlotsMouseRegions[ ubCnt ], 0, ubCnt);


			//if the dealer repairs
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
			{
				//Small Faces
				MSYS_DefineRegion( &gDealersOfferSlotsSmallFaceMouseRegions[ ubCnt ], (UINT16)(usPosX+SKI_SMALL_FACE_OFFSET_X), (UINT16)(usPosY), (INT16)(usPosX + SKI_SMALL_FACE_OFFSET_X + SKI_SMALL_FACE_WIDTH), (INT16)(usPosY+ SKI_SMALL_FACE_HEIGHT), MSYS_PRIORITY_HIGH+1,
											 CURSOR_NORMAL, SelectDealersOfferSlotsMovementRegionCallBack, SelectDealersOfferSlotsRegionCallBack );
				MSYS_AddRegion( &gDealersOfferSlotsSmallFaceMouseRegions[ubCnt] );
				MSYS_SetRegionUserData( &gDealersOfferSlotsSmallFaceMouseRegions[ ubCnt ], 0, ubCnt);
				MSYS_SetRegionUserData( &gDealersOfferSlotsSmallFaceMouseRegions[ ubCnt ], 1, ARMS_DEALER_OFFER_AREA );
			}


			usPosX += SKI_INV_OFFSET_X;
			ubCnt++;
		}
		usPosY += SKI_INV_OFFSET_Y;
	}

	// Create the mouse regions for the Players trading slots
	usPosY = SKI_PLAYERS_TRADING_INV_Y;
	ubCnt=0;
	for(y=0; y<SKI_NUM_TRADING_INV_ROWS; y++)
	{
		usPosX = SKI_PLAYERS_TRADING_INV_X;

		for(x=0; x<SKI_NUM_TRADING_INV_COLS; x++)
		{
			//Trading Slots
			MSYS_DefineRegion( &gPlayersOfferSlotsMouseRegions[ ubCnt ], usPosX, usPosY, (INT16)(usPosX + SKI_INV_SLOT_WIDTH), (INT16)(usPosY + SKI_INV_SLOT_HEIGHT), MSYS_PRIORITY_HIGH,
										 CURSOR_NORMAL, SelectPlayersOfferSlotsMovementRegionCallBack, SelectPlayersOfferSlotsRegionCallBack );
			MSYS_AddRegion( &gPlayersOfferSlotsMouseRegions[ubCnt] );
			MSYS_SetRegionUserData( &gPlayersOfferSlotsMouseRegions[ ubCnt ], 0, ubCnt);
/*
			//if the dealer repairs
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
			{
				MSYS_SetRegionUserData( &gPlayersOfferSlotsSmallFaceMouseRegions[ ubCnt ], 1, PLAYERS_OFFER_AREA );
			}
*/
			//Small Faces
			MSYS_DefineRegion( &gPlayersOfferSlotsSmallFaceMouseRegions[ ubCnt ], (UINT16)(usPosX+SKI_SMALL_FACE_OFFSET_X), (UINT16)(usPosY), (INT16)(usPosX + SKI_SMALL_FACE_OFFSET_X + SKI_SMALL_FACE_WIDTH), (INT16)(usPosY+ SKI_SMALL_FACE_HEIGHT), MSYS_PRIORITY_HIGH+1,
										 CURSOR_NORMAL, SelectPlayersOfferSlotsMovementRegionCallBack, SelectPlayersOfferSlotsRegionCallBack );
			MSYS_AddRegion( &gPlayersOfferSlotsSmallFaceMouseRegions[ubCnt] );
			MSYS_SetRegionUserData( &gPlayersOfferSlotsSmallFaceMouseRegions[ ubCnt ], 0, ubCnt);
			MSYS_SetRegionUserData( &gPlayersOfferSlotsSmallFaceMouseRegions[ ubCnt ], 1, PLAYERS_OFFER_AREA );

			usPosX += SKI_INV_OFFSET_X;
			ubCnt++;
		}
		usPosY += SKI_INV_OFFSET_Y;
	}


}


void DestroySkiInventorySlotMouseRegions( )
{
	UINT8	i;

	for(i=0; i<SKI_NUM_ARMS_DEALERS_INV_SLOTS; i++)
	{
		MSYS_RemoveRegion( &gDealersInventoryMouseRegions[ i ] );

		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			MSYS_RemoveRegion( &gRepairmanInventorySmallFaceMouseRegions[ i ] );
		}
	}

	for(i=0; i<SKI_NUM_TRADING_INV_SLOTS; i++)
	{
		MSYS_RemoveRegion( &gDealersOfferSlotsMouseRegions[ i ] );

		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			MSYS_RemoveRegion( &gDealersOfferSlotsSmallFaceMouseRegions[ i ] );
		}

		MSYS_RemoveRegion( &gPlayersOfferSlotsMouseRegions[ i ] );

		MSYS_RemoveRegion( &gPlayersOfferSlotsSmallFaceMouseRegions[ i ] );
	}
}


//Mouse Call back for the Arms traders inventory slot
void SelectDealersInventoryRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
		INT8	ubLocation;

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
				if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
				{
					// ignore left clicks on items under repair.  Fully repaired items are moved out to player's slots automatically
				}
				else	// not a repairman
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
						if( gfKeyState[ SHIFT ] )
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
			else	// some of this item are already in dealer's offer area
			{
				UINT8 ubNumToMove;

				//if the shift key is being pressed, remove them all
				if( gfKeyState[ SHIFT ] )
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
		UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

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
			if( ArmsDealerOfferArea[ gpTempDealersInventory[ ubSelectedInvSlot ].bSlotIdInOtherLocation ].ItemObject.ubNumberOfObjects > 0 )
			{
				//Increase the number in the dealer inventory
				gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects ++;

				//Decrease the number in the dealer offer area
				ArmsDealerOfferArea[ gpTempDealersInventory[ ubSelectedInvSlot ].bSlotIdInOtherLocation ].ItemObject.ubNumberOfObjects--;

				//if there is nothing left in the arms dealer offer area
				if( ArmsDealerOfferArea[ gpTempDealersInventory[ ubSelectedInvSlot ].bSlotIdInOtherLocation ].ItemObject.ubNumberOfObjects == 0 )
				{
					RemoveItemFromArmsDealerOfferArea( gpTempDealersInventory[ ubSelectedInvSlot ].bSlotIdInOtherLocation, FALSE );
				}

				//redraw the screen
				gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			}
		}
*/
	}
}



void SelectDealersInventoryMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	ubSelectedInvSlot += gSelectArmsDealerInfo.ubFirstItemIndexOnPage;

	if( gpTempDealersInventory == NULL )
		return;

	if( ubSelectedInvSlot >= gSelectArmsDealerInfo.uiNumDistinctInventoryItems )
		return;

	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		//if there is nothing in the slot, exit
		if( gpTempDealersInventory[ ubSelectedInvSlot ].fActive == FALSE )
			return;

		gpHighLightedItemObject = &gpTempDealersInventory[ ubSelectedInvSlot ].ItemObject;

		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, TRUE );
	}
	else if(iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//if there is nothing in the slot, exit
		if( gpTempDealersInventory[ ubSelectedInvSlot ].fActive == FALSE )
			return;

		gpHighLightedItemObject = NULL;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;	//ddd
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, FALSE );
	}
}

void SelectDealersOfferSlotsMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		//if there is nothing in the slot, exit
		if( ArmsDealerOfferArea[ ubSelectedInvSlot ].fActive == FALSE )
			return;

		gpHighLightedItemObject = &ArmsDealerOfferArea[ ubSelectedInvSlot ].ItemObject;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, TRUE );
	}
	else if(iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//if there is nothing in the slot, exit
		if( ArmsDealerOfferArea[ ubSelectedInvSlot ].fActive == FALSE )
			return;

		gpHighLightedItemObject = NULL;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;//ddd
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, FALSE );
	}
}

void SelectPlayersOfferSlotsMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		//if there is nothing in the slot, exit
		if( PlayersOfferArea[ ubSelectedInvSlot ].fActive == FALSE )
			return;

		gpHighLightedItemObject = &PlayersOfferArea[ ubSelectedInvSlot ].ItemObject;
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, TRUE );
	}
	else if(iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		//if there is nothing in the slot, exit
		if( PlayersOfferArea[ ubSelectedInvSlot ].fActive == FALSE )
			return;

		gpHighLightedItemObject = NULL;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;//ddd
		HandleCompatibleAmmoUI( gpSMCurrentMerc, -1, FALSE );
	}
}



//Mouse Call back for the dealer's OFFER slot
void SelectDealersOfferSlotsRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		//if there is something here
		if( ArmsDealerOfferArea[ ubSelectedInvSlot ].fActive )
		{
			//if the dealer repairs
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
			{
				// return item to player
				RemoveRepairItemFromDealersOfferArea( ubSelectedInvSlot );
			}
			else
			{
				//bring up the item description box
				if ( !InItemDescriptionBox( ) )
				{
					InitShopKeeperItemDescBox( &ArmsDealerOfferArea[ ubSelectedInvSlot ].ItemObject, ubSelectedInvSlot, ARMS_DEALER_OFFER_AREA );
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
			return;
*/
		//if there is something here
		if( ArmsDealerOfferArea[ ubSelectedInvSlot ].fActive )
		{
			//if this is a repair dealer
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
			{
				//if we don't have an item, pick one up
				if( gMoveingItem.sItemIndex == 0 )
				{
					//if the dealer is a repair dealer, allow the player to pick up the item
					if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
					{
						BeginSkiItemPointer( ARMS_DEALER_OFFER_AREA, ubSelectedInvSlot, FALSE );
					}
				}
				else
				{
					//swap what is in the cursor with what is in the player offer slot
					INVENTORY_IN_SLOT TempSlot;

					// if the slot is overloaded (holds more objects than we have room for valid statuses of)
					if ( ArmsDealerOfferArea[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects > MAX_OBJECTS_PER_SLOT )
					{
						// then ignore the click - we can't do the swap, or anything very useful, because we can't allow overloaded
						// items into the player's cursor - if he put them into a merc's inventory, the extra statuses are missing!
						// At best, we could do some sort of message here.
						return;
					}

					IfMercOwnedCopyItemToMercInv( &gMoveingItem );

					memcpy( &TempSlot, &ArmsDealerOfferArea[ ubSelectedInvSlot ], sizeof( INVENTORY_IN_SLOT ) );
					memcpy( &ArmsDealerOfferArea[ ubSelectedInvSlot ], &gMoveingItem, sizeof( INVENTORY_IN_SLOT ) );
					memcpy( &gMoveingItem, &TempSlot, sizeof( INVENTORY_IN_SLOT ) );

					IfMercOwnedRemoveItemFromMercInv( &gMoveingItem );

					// Change mouse cursor
					gpItemPointer = &gMoveingItem.ItemObject;
					SetSkiCursor( EXTERN_CURSOR );

					gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
				}
			}
			else	//non-repairman
			{
				//if the shift key is being pressed, remove them all
				if( gfKeyState[ SHIFT ] || ( ArmsDealerOfferArea[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects == 1 ) )
				{
					RemoveItemFromArmsDealerOfferArea( ubSelectedInvSlot, TRUE );//ArmsDealerOfferArea[ ubSelectedInvSlot ].bSlotIdInOtherLocation
				}
				else	// multiple items there, SHIFT isn't being pressed
				{
					// remove only one at a time
					ArmsDealerOfferArea[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects--;
					gpTempDealersInventory[ ArmsDealerOfferArea[ ubSelectedInvSlot ].bSlotIdInOtherLocation ].ItemObject.ubNumberOfObjects++;
				}

				gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			}
		}
		else	// empty slot
		{
			//if the cursor has something in it
			if( gMoveingItem.sItemIndex > 0 )
			{
				// we'd better talking to a repairman, cursor is locked out of this area while full for non-repairmen!
				Assert( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS );

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


//Mouse Call back for the Players OFFER slot
void SelectPlayersOfferSlotsRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	UINT8	ubSelectedInvSlot = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	INT8 bAddedToSlotID = -1;


	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		//if the cursor has no item in it
		if( gMoveingItem.sItemIndex == 0 )
		{
			//if there is nothing here, return
			if( PlayersOfferArea[ ubSelectedInvSlot ].fActive == FALSE )
				return;

			// pick it up into the cursor
			BeginSkiItemPointer( PLAYERS_OFFER_AREA, ubSelectedInvSlot, FALSE );
		}
		else	// we have something in the cursor
		{
			//Drop the item into the current slot
//			AddObjectToSkiLocation( &gpSkiItemPointer, ubSelectedInvSlot, PLAYERS_OFFER_AREA );

			//if there is something already there
			if( PlayersOfferArea[ ubSelectedInvSlot ].fActive )
			{
				//swap what is in the cursor with what is in the player offer slot
				INVENTORY_IN_SLOT TempSlot;

				// if the slot is overloaded (holds more objects than we have room for valid statuses of)
				if ( PlayersOfferArea[ ubSelectedInvSlot ].ItemObject.ubNumberOfObjects > MAX_OBJECTS_PER_SLOT )
				{
					// then ignore the click - we can't do the swap, or anything very useful, because we can't allow overloaded
					// items into the player's cursor - if he put them into a merc's inventory, the extra statuses are missing!
					// At best, we could do some sort of message here.
					return;
				}

				IfMercOwnedCopyItemToMercInv( &gMoveingItem );

				memcpy( &TempSlot, &PlayersOfferArea[ ubSelectedInvSlot ], sizeof( INVENTORY_IN_SLOT ) );
				memcpy( &PlayersOfferArea[ ubSelectedInvSlot ], &gMoveingItem, sizeof( INVENTORY_IN_SLOT ) );
				memcpy( &gMoveingItem, &TempSlot, sizeof( INVENTORY_IN_SLOT ) );

				IfMercOwnedRemoveItemFromMercInv( &gMoveingItem );

				// Change mouse cursor
				gpItemPointer = &gMoveingItem.ItemObject;
				SetSkiCursor( EXTERN_CURSOR );

				//if the item we are adding is money
				if( Item[ PlayersOfferArea[ ubSelectedInvSlot ].sItemIndex ].usItemClass == IC_MONEY )
				{
					//Since money is always evaluated
					PlayersOfferArea[ ubSelectedInvSlot ].uiFlags |= ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
					PlayersOfferArea[ ubSelectedInvSlot ].uiItemPrice = PlayersOfferArea[ ubSelectedInvSlot ].ItemObject.bMoneyStatus;
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
		if( PlayersOfferArea[ ubSelectedInvSlot ].fActive )
		{
			RemoveItemFromPlayersOfferArea( ubSelectedInvSlot );
/*
item description
			else
			{
				if ( !InItemDescriptionBox( ) )
				{
					InitItemDescriptionBox( gpSMCurrentMerc, (UINT8)PlayersOfferArea[ ubSelectedInvSlot ].bSlotIdInOtherLocation, 214, 1 + INV_INTERFACE_START_Y, 0 );
				}
				else
				{
					DeleteItemDescriptionBox( );
				}
			}
*/
		}
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
	}
}


void SelectSkiInventoryMovementAreaRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{

	}
}




void EnterShopKeeperInterfaceScreen( UINT8	ubArmsDealer )
{
	//Get Dealer ID from from merc Id
	gbSelectedArmsDealerID = GetArmsDealerIDFromMercID( ubArmsDealer );

	if( gbSelectedArmsDealerID == -1 )
	{
		#ifdef JA2BETAVERSION
			ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Failed to find Arms Dealer ID From Merc ID #%d", ubArmsDealer );
		#endif
		gfSKIScreenExit = TRUE;
	}

	LeaveTacticalScreen( SHOPKEEPER_SCREEN );
}


void InitializeShopKeeper( BOOLEAN fResetPage )
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
		gSelectArmsDealerInfo.uiNumDistinctInventoryItems = CountDistinctItemsInArmsDealersInventory( gbSelectedArmsDealerID );
*/
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


void CalculateFirstItemIndexOnPage( )
{
	gSelectArmsDealerInfo.ubFirstItemIndexOnPage = ( gSelectArmsDealerInfo.ubCurrentPage -1 ) * SKI_NUM_ARMS_DEALERS_INV_SLOTS;
}



void DisplayArmsDealerCurrentInventoryPage( )
{
	CHAR16			zTemp[32];
	UINT16			uiFontHeight;
	UINT16			usCnt=0;
	UINT16			usPosX, usPosY;
	UINT8				sItemCount=0;
	BOOLEAN			fDisplayHatchOnItem=FALSE;


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
			DrawTextToScreen( SKI_Text[ SKI_TEXT_PAGE ], SKI_PAGE_X, (UINT16)(SKI_PAGE_Y+3), SKI_PAGE_WIDTH, SKI_LABEL_FONT, SKI_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

			//Display the Current Page number
			uiFontHeight = GetFontHeight( SKI_LABEL_FONT );
			swprintf( zTemp, lengthof(zTemp), L"%d/%d", gSelectArmsDealerInfo.ubCurrentPage, gSelectArmsDealerInfo.ubNumberOfPages );
			DrawTextToScreen( zTemp, SKI_PAGE_X, (UINT16)(SKI_PAGE_Y+uiFontHeight+6), SKI_PAGE_WIDTH, SKI_LABEL_FONT, SKI_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
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
				else if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
				{
					//if the item is damaged
					if( ArmsDealerInfo[ gbSelectedArmsDealerID ].uiFlags & ARMS_INV_ITEM_NOT_REPAIRED_YET )
						fDisplayHatchOnItem = TRUE;
					else
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
				DisplayInvSlot( (UINT8)usCnt, gpTempDealersInventory[ usCnt ].sItemIndex, usPosX, usPosY,
												&gpTempDealersInventory[ usCnt ].ItemObject,
												fDisplayHatchOnItem,
												ARMS_DEALER_INVENTORY );


				if( gubSkiDirtyLevel == SKI_DIRTY_LEVEL2 )
				{
					SetSkiRegionHelpText( &gpTempDealersInventory[ usCnt ], &gDealersInventoryMouseRegions[sItemCount], ARMS_DEALER_INVENTORY );

					//if the dealer repairs
					if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
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
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
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


UINT32 DisplayInvSlot( UINT8 ubSlotNum, UINT16 usItemIndex, UINT16 usPosX, UINT16 usPosY, OBJECTTYPE	*pItemObject, BOOLEAN fHatchedOut, UINT8 ubItemArea )
{
	CHAR16			zTemp[64];
	HVOBJECT		hVObject;
	INVTYPE			*pItem;
	ETRLEObject	*pTrav;
	UINT32			usHeight, usWidth;
	INT16				sCenX, sCenY;
	BOOLEAN			fHighlighted = IsGunOrAmmoOfSameTypeSelected( pItemObject );
	BOOLEAN			fDisplayMercFace=FALSE;
	UINT8				ubMercID=0;
	UINT32			uiItemCost=0;
	BOOLEAN			fPrintRepaired = FALSE;

	//if the item is not highlighted, and we are not rerendering the screen
	if( !( fHighlighted || gubSkiDirtyLevel != SKI_DIRTY_LEVEL0 ) )
		return( 0 );

	//
//	if( fHighlighted )
//		gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;


	//Display the item graphic, and price
	pItem = &Item[ usItemIndex ];
	GetVideoObject( &hVObject, GetInterfaceGraphicForItem( pItem ) );
	pTrav = &(hVObject->pETRLEObject[ pItem->ubGraphicNum ] );

	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;

	sCenX = usPosX + 7 + ( abs( SKI_INV_WIDTH - 3 - usWidth ) / 2 ) - pTrav->sOffsetX;
	sCenY = usPosY + ( abs( SKI_INV_HEIGHT - usHeight ) / 2 ) - pTrav->sOffsetY;


	//Restore the background region
	RestoreExternBackgroundRect( usPosX, usPosY, SKI_INV_SLOT_WIDTH, SKI_INV_HEIGHT );


	//blt the shadow of the item
	BltVideoObjectOutlineShadowFromIndex( FRAME_BUFFER, GetInterfaceGraphicForItem( pItem ), pItem->ubGraphicNum, sCenX-2, sCenY+2);

	//blt the item
	BltVideoObjectOutlineFromIndex( FRAME_BUFFER, GetInterfaceGraphicForItem( pItem ), pItem->ubGraphicNum, sCenX, sCenY, Get16BPPColor( FROMRGB( 255, 255, 255 ) ), fHighlighted );

	//Display the status of the item
	DrawItemUIBarEx( pItemObject, 0, (INT16)(usPosX+2), (INT16)(usPosY+2+20), 2, 20, 	Get16BPPColor( FROMRGB( 140, 136, 119 ) ), Get16BPPColor( FROMRGB( 140, 136, 119 ) ), TRUE, guiRENDERBUFFER );//guiSAVEBUFFER


	//Display the Items Cost
	if( ubItemArea == PLAYERS_OFFER_AREA )
	{
		//if the item has value
		if( PlayersOfferArea[ ubSlotNum ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE )
		{
			uiItemCost = PlayersOfferArea[ ubSlotNum ].uiItemPrice;
		}

		//if the item belongs to a merc
		if( PlayersOfferArea[ ubSlotNum ].ubIdOfMercWhoOwnsTheItem != NO_PROFILE )
		{
			//Display the face of the merc
			fDisplayMercFace = TRUE;
			ubMercID = PlayersOfferArea[ ubSlotNum ].ubIdOfMercWhoOwnsTheItem;
		}

		// if the item has just been repaired
		if ( PlayersOfferArea[ ubSlotNum ].uiFlags & ARMS_INV_ITEM_REPAIRED )
		{
			fPrintRepaired = TRUE;
		}
	}
	else if( ubItemArea == ARMS_DEALER_INVENTORY )
	{
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_REPAIRS )
		{
			if( fHatchedOut && pItemObject->ubNumberOfObjects == 0 )
			{
				uiItemCost = 0;
			}
			else
			{
				// show the UNIT price, not the total value of all if stacked
				uiItemCost = CalcShopKeeperItemPrice( DEALER_SELLING, TRUE, usItemIndex, ArmsDealerInfo[ gbSelectedArmsDealerID ].dSellModifier, pItemObject );
			}
		}
		else // UNDER REPAIR
		{
			UINT8		ubElement;

			//display the length of time needed to repair the item
			uiItemCost = 0;

			//Get the length of time needed to fix the item
			Assert( gpTempDealersInventory[ ubSlotNum ].sSpecialItemElement != -1);
			ubElement = (UINT8) gpTempDealersInventory[ ubSlotNum ].sSpecialItemElement;

			BuildDoneWhenTimeString( zTemp, lengthof(zTemp), gbSelectedArmsDealerID, usItemIndex, ubElement );
			DrawTextToScreen( zTemp, (UINT16)(usPosX+SKI_INV_PRICE_OFFSET_X), (UINT16)(usPosY+SKI_INV_PRICE_OFFSET_Y), SKI_INV_SLOT_WIDTH, SKI_ITEM_DESC_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

			//if the item belongs to a merc
			if( gpTempDealersInventory[ ubSlotNum ].ubIdOfMercWhoOwnsTheItem != NO_PROFILE )
			{
				//Display the face of the merc
				fDisplayMercFace = TRUE;
				ubMercID = gpTempDealersInventory[ ubSlotNum ].ubIdOfMercWhoOwnsTheItem;
			}
		}
	}
	else		// DEALER'S OFFER AREA
	{
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			//if the dealer repairs, there is an item here, therefore display the items owner's face
			fDisplayMercFace = TRUE;
			ubMercID = ArmsDealerOfferArea[ ubSlotNum ].ubIdOfMercWhoOwnsTheItem;

			uiItemCost = CalculateObjectItemRepairCost( gbSelectedArmsDealerID, pItemObject );
		}
		else
			uiItemCost = CalcShopKeeperItemPrice( DEALER_SELLING, FALSE, usItemIndex, ArmsDealerInfo[ gbSelectedArmsDealerID ].dSellModifier, pItemObject );

	}

	//if the item has a price, display it
	if( uiItemCost != 0 )
	{
		swprintf( zTemp, lengthof(zTemp), L"%d", uiItemCost );
		InsertCommasForDollarFigure( zTemp );
		InsertDollarSignInToString( zTemp );
		DrawTextToScreen( zTemp, (UINT16)(usPosX+SKI_INV_PRICE_OFFSET_X), (UINT16)(usPosY+SKI_INV_PRICE_OFFSET_Y), SKI_INV_SLOT_WIDTH, SKI_ITEM_DESC_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
	}


	//if the there is more then 1 or if the item is stackable and some of it has been bought and only 1 remains
//	if( pItemObject->ubNumberOfObjects > 1 || Item[ usItemIndex ].usItemClass == IC_AMMO )
//	if( pItemObject->ubNumberOfObjects > 1 || Item[ usItemIndex ].ubPerPocket > 1 )
	if( ( pItemObject->ubNumberOfObjects > 1 ) ||
			( ( pItemObject->ubNumberOfObjects == 1 ) && DealerItemIsSafeToStack( usItemIndex ) &&
				( ubItemArea == ARMS_DEALER_INVENTORY ) && ( gpTempDealersInventory[ ubSlotNum ].uiFlags & ARMS_INV_ITEM_SELECTED ) ) )
	{
		swprintf( zTemp, lengthof(zTemp), L"x%d", pItemObject->ubNumberOfObjects );
		DrawTextToScreen( zTemp, (UINT16)(usPosX+SKI_ITEM_NUMBER_TEXT_OFFSET_X), (UINT16)(usPosY+SKI_ITEM_NUMBER_TEXT_OFFSET_Y), SKI_ITEM_NUMBER_TEXT_WIDTH, SKIT_NUMBER_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED );
	}

	//if we are to display the face
	if( fDisplayMercFace )
	{
		INT8 iFaceSlot = GetSlotNumberForMerc( ubMercID );

		// if still in player's employ
		if ( iFaceSlot != -1 )
		{
			BltVideoObjectFromIndex(FRAME_BUFFER, guiSmallSoldiersFace[iFaceSlot], 0, usPosX + SKI_SMALL_FACE_OFFSET_X, usPosY);//SKI_SMALL_FACE_OFFSET_Y
		}
	}

	//if the item has attachments
	if( ItemHasAttachments( pItemObject ) )
	{
		//Display the '*' in the bottom right corner of the square
		swprintf( zTemp, lengthof(zTemp), L"*" );
		DrawTextToScreen( zTemp, (UINT16)(usPosX+SKI_ATTACHMENT_SYMBOL_X_OFFSET), (UINT16)(usPosY+SKI_ATTACHMENT_SYMBOL_Y_OFFSET), 0, TINYFONT1, FONT_GREEN, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	}

	// Display 'JAMMED' if it's jammed
	if ( pItemObject->bGunAmmoStatus < 0 )
	{
		swprintf( zTemp, lengthof(zTemp), TacticalStr[ JAMMED_ITEM_STR ] );
		VarFindFontCenterCoordinates( usPosX, usPosY, SKI_INV_SLOT_WIDTH, SKI_INV_HEIGHT, TINYFONT1, &sCenX, &sCenY, zTemp );
		DrawTextToScreen( zTemp, sCenX, sCenY, SKI_INV_SLOT_WIDTH, TINYFONT1, FONT_RED, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	}
	else if ( fPrintRepaired )
	{
		swprintf( zTemp, lengthof(zTemp), SKI_Text[ SKI_TEXT_REPAIRED ] );
		VarFindFontCenterCoordinates( usPosX, usPosY, SKI_INV_SLOT_WIDTH, SKI_INV_HEIGHT, TINYFONT1, &sCenX, &sCenY, zTemp );
		DrawTextToScreen( zTemp, sCenX, sCenY, SKI_INV_SLOT_WIDTH, TINYFONT1, FONT_RED, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	}

	if( fHatchedOut )
	{
		HatchOutInvSlot( usPosX, usPosY );
	}

	InvalidateRegion( usPosX, usPosY, usPosX+SKI_INV_SLOT_WIDTH, usPosY+SKI_INV_SLOT_HEIGHT );

	return( uiItemCost );
}



BOOLEAN DetermineArmsDealersSellingInventory( )
{
	UINT16	usItemIndex;
	UINT8		ubElement;
	DEALER_SPECIAL_ITEM *pSpecialItem;
	BOOLEAN fAddSpecialItem;
	SPECIAL_ITEM_INFO SpclItemInfo;


	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("DEF: DetermineArmsDealer") );

	//if there is an old inventory, delete it
	if( gpTempDealersInventory )
	{
		MemFree( gpTempDealersInventory );
		gpTempDealersInventory = NULL;
	}

	//allocate memory to hold the inventory in memory
	gpTempDealersInventory = MemAlloc( sizeof( INVENTORY_IN_SLOT ) * gSelectArmsDealerInfo.uiNumDistinctInventoryItems );
	if( gpTempDealersInventory == NULL )
	{
		Assert( 0 );
		return(FALSE);
	}
	memset( gpTempDealersInventory, 0, sizeof( INVENTORY_IN_SLOT ) * gSelectArmsDealerInfo.uiNumDistinctInventoryItems );


	guiNextFreeInvSlot = 0;

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
			for( ubElement=0; ubElement< gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].ubElementsAlloced; ubElement++ )
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

						if ( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_REPAIRS )
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
		if ( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
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

	return( TRUE );
}



void AddItemsToTempDealerInventory(UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner )
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
	else	// more than one
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



void StoreObjectsInNextFreeDealerInvSlot( UINT16 usItemIndex, SPECIAL_ITEM_INFO *pSpclItemInfo, INT16 sSpecialItemElement, UINT8 ubHowMany, UINT8 ubOwner )
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
	MakeObjectOutOfDealerItems( usItemIndex, pSpclItemInfo, &(pDealerInvSlot->ItemObject), ( UINT8 ) min( ubHowMany, MAX_OBJECTS_PER_SLOT ) );

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



BOOLEAN RepairIsDone(UINT16 usItemIndex, UINT8 ubElement)
{
	INVENTORY_IN_SLOT	RepairItem;
	INT8		bSlotNum;
	UINT8		ubCnt;


	// make a new shopkeeper invslot item out of it
	memset( &RepairItem, 0, sizeof( INVENTORY_IN_SLOT ) );

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
			if ( Item[ RepairItem.ItemObject.usAttachItem[ ubCnt ] ].fFlags & ITEM_INSEPARABLE )
*/
			if ( CanDealerRepairItem( gbSelectedArmsDealerID, RepairItem.ItemObject.usAttachItem[ ubCnt ] ) )
			{
				// fix it up
				RepairItem.ItemObject.bAttachStatus[ ubCnt ] = 100;
			}
		}
	}

	// if the item is imprinted (by anyone, even player's mercs), and it's Fredo repairing it
	if ( /*( gArmsDealersInventory[ gbSelectedArmsDealerID ][ usItemIndex ].SpecialItem[ ubElement ].Info.ubImprintID == (NO_PROFILE + 1) ) && */
		 ( gbSelectedArmsDealerID == ARMS_DEALER_FREDO ) )
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


void DrawHatchOnInventory( UINT32 uiSurface, UINT16 usPosX, UINT16 usPosY, UINT16 usWidth, UINT16 usHeight )
{
	UINT8	 *pDestBuf;
	UINT32 uiDestPitchBYTES;
	SGPRect ClipRect;
	ClipRect.iLeft = usPosX;
	ClipRect.iRight = usPosX + usWidth;
	ClipRect.iTop = usPosY;
	ClipRect.iBottom = usPosY + usHeight;

	pDestBuf = LockVideoSurface( uiSurface, &uiDestPitchBYTES );
	Blt16BPPBufferHatchRect((UINT16*)pDestBuf, uiDestPitchBYTES, &ClipRect);
	UnLockVideoSurface( uiSurface );
}


UINT32 CalcShopKeeperItemPrice( BOOLEAN fDealerSelling, BOOLEAN fUnitPriceOnly, UINT16 usItemID, FLOAT dModifier, OBJECTTYPE *pItemObject )
{
	UINT8		ubCnt;
	UINT32	uiUnitPrice = 0;
	UINT32	uiTotalPrice = 0;
	UINT8		ubItemsToCount = 0;
	UINT8		ubItemsNotCounted = 0;
	UINT32	uiDiscountValue;
//	UINT32	uiDifFrom10 = 0;


	// add up value of the main item(s), exact procedure depends on its item class
	switch ( Item [ usItemID ].usItemClass )
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
				if( Item[ pItemObject->usGunAmmoItem ].usItemClass == IC_AMMO )
				{
					// add value of its remaining ammo
					uiUnitPrice += (UINT32)( CalcValueOfItemToDealer( gbSelectedArmsDealerID, pItemObject->usGunAmmoItem, fDealerSelling ) *
																		 ItemConditionModifier(pItemObject->usGunAmmoItem, pItemObject->ubGunShotsLeft) *
																		 dModifier );
				}
				else	// assume it's attached ammo (mortar shells, grenades)
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
		switch ( Item [ usItemID ].usItemClass )
		{
			case IC_GUN:
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
	}
*/

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


FLOAT ItemConditionModifier(UINT16 usItemIndex, INT8 bStatus)
{
	FLOAT dConditionModifier = 1.0f;

	//if the item is ammo, the condition modifier is based on how many shots are left
	if( Item[ usItemIndex ].usItemClass == IC_AMMO )
	{
		// # bullets left / max magazine capacity
		dConditionModifier = ( bStatus / (FLOAT) Magazine[ Item[ usItemIndex ].ubClassIndex ].ubMagSize );
	}
	else	// non-ammo
	{
		// handle non-ammo payloads in jammed guns as if they weren't
		if ( bStatus < 0 )
		{
			bStatus *= -1;
		}

		// an item at 100% is worth full price...

		if ( Item[ usItemIndex ].fFlags & ITEM_REPAIRABLE )
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


void DisplayArmsDealerOfferArea()
{
	INT16		sCnt, sCount;
	CHAR16	zTemp[32];
	UINT32	uiTotalCost;
	UINT16	usPosX, usPosY;
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
		if( ArmsDealerOfferArea[ sCnt ].fActive )
		{
			// hatch it out if region is disabled
			if ( !( gDealersOfferSlotsMouseRegions[ sCnt ].uiFlags & MSYS_REGION_ENABLED ) )
			{
				fDisplayHatchOnItem = TRUE;
			}
			else if( ArmsDealerOfferArea[ sCnt ].uiFlags & ARMS_INV_ITEM_SELECTED )
			{
				fDisplayHatchOnItem = TRUE;
			}
			else
				fDisplayHatchOnItem = FALSE;

			// Display the inventory slot
			uiTotalCost += DisplayInvSlot( (UINT8)sCnt, ArmsDealerOfferArea[ sCnt ].sItemIndex, usPosX, usPosY,
											&ArmsDealerOfferArea[ sCnt ].ItemObject,
											fDisplayHatchOnItem,
											ARMS_DEALER_OFFER_AREA );
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
		swprintf( zTemp, lengthof(zTemp), L"%d", uiTotalCost );
		InsertCommasForDollarFigure( zTemp );
		InsertDollarSignInToString( zTemp );
		DrawTextToScreen( zTemp, SKI_ARMS_DEALER_TOTAL_COST_X, (UINT16)(SKI_ARMS_DEALER_TOTAL_COST_Y+5), SKI_INV_SLOT_WIDTH, SKI_LABEL_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
	}
}


INT8 AddItemToArmsDealerOfferArea( INVENTORY_IN_SLOT* pInvSlot, INT8 bSlotIdInOtherLocation )
{
	INT8	bCnt;

	for( bCnt=0; bCnt<SKI_NUM_TRADING_INV_SLOTS; bCnt++)
	{
		//if there are no items here, copy the data in
		if( ArmsDealerOfferArea[bCnt].fActive == FALSE )
		{
			//Copy the inventory items
			memcpy( &ArmsDealerOfferArea[bCnt], pInvSlot, sizeof( INVENTORY_IN_SLOT ) );

			//if the shift key is being pressed, add them all
			if( gfKeyState[ SHIFT ] )
				ArmsDealerOfferArea[bCnt].ItemObject.ubNumberOfObjects = pInvSlot->ItemObject.ubNumberOfObjects;
			//If there was more then 1 item, reduce it to only 1 item moved
			else if( pInvSlot->ItemObject.ubNumberOfObjects > 1 )
				ArmsDealerOfferArea[bCnt].ItemObject.ubNumberOfObjects = 1;

			//Remember where the item came from
			ArmsDealerOfferArea[bCnt].bSlotIdInOtherLocation = bSlotIdInOtherLocation;
			ArmsDealerOfferArea[bCnt].ubIdOfMercWhoOwnsTheItem = pInvSlot->ubIdOfMercWhoOwnsTheItem;
			ArmsDealerOfferArea[bCnt].fActive = TRUE;

			SetSkiRegionHelpText( &ArmsDealerOfferArea[bCnt], &gDealersOfferSlotsMouseRegions[ bCnt ], ARMS_DEALER_OFFER_AREA );
			SetSkiFaceRegionHelpText( &ArmsDealerOfferArea[bCnt], &gDealersOfferSlotsSmallFaceMouseRegions[ bCnt ], ARMS_DEALER_OFFER_AREA );

			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
			return( bCnt );
		}
	}

	return( -1 );
}

BOOLEAN RemoveItemFromArmsDealerOfferArea( INT8	bSlotId, BOOLEAN fKeepItem )
{
//	UINT16	usCnt;

	//Loop through all the slot to see if the requested one is here
//	for( usCnt=0; usCnt<SKI_NUM_TRADING_INV_SLOTS; usCnt++)
	{
		//if this is the requested slot
		if( ArmsDealerOfferArea[bSlotId].fActive )//bSlotIdInOtherLocation == bSlotIdInOtherLocation )
		{
			//if there are more then 1 item
//			if( ArmsDealerOfferArea[ usCnt ].ItemObject.ubNumberOfObjects > 1 )
			if( fKeepItem )
			{
				gpTempDealersInventory[ ArmsDealerOfferArea[bSlotId].bSlotIdInOtherLocation ].ItemObject.ubNumberOfObjects += ArmsDealerOfferArea[ bSlotId ].ItemObject.ubNumberOfObjects;
			}

			//Clear the flag that hatches out the item
			gpTempDealersInventory[ ArmsDealerOfferArea[bSlotId].bSlotIdInOtherLocation ].uiFlags &= ~ARMS_INV_ITEM_SELECTED;

			ClearArmsDealerOfferSlot( bSlotId );

			return( TRUE );
		}
	}

	return( FALSE );
}


void SetSkiRegionHelpText( INVENTORY_IN_SLOT *pInv, MOUSE_REGION *pRegion, UINT8 ubScreenArea )
{
	CHAR16 zHelpText[ 512 ];

	Assert( pRegion );

	BuildItemHelpTextString( zHelpText, lengthof(zHelpText), pInv, ubScreenArea );

	SetRegionFastHelpText( pRegion, zHelpText );
	SetRegionHelpEndCallback( pRegion, SkiHelpTextDoneCallBack );
}


void SetSkiFaceRegionHelpText( INVENTORY_IN_SLOT *pInv, MOUSE_REGION *pRegion, UINT8 ubScreenArea )
{
	CHAR16 zTempText[ 512 ];
	CHAR16 zHelpText[ 512 ];

	Assert( pRegion );

	//if the item isn't NULL, and is owned by a merc
	if( ( pInv != NULL ) && ( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE ) )
	{
		BuildItemHelpTextString( zTempText, lengthof(zTempText), pInv, ubScreenArea );
		// add who owns it
		swprintf( zHelpText, lengthof(zHelpText), L"%S%S %S", gMercProfiles[ pInv->ubIdOfMercWhoOwnsTheItem ].zNickname, pMessageStrings[ MSG_DASH_S ], zTempText );
	}
	else
	{
		zHelpText[ 0 ] = '\0';
	}

	SetRegionFastHelpText( pRegion, zHelpText );
	SetRegionHelpEndCallback( pRegion, SkiHelpTextDoneCallBack );
}


void SkiHelpTextDoneCallBack( void )
{
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}



INT8 AddItemToPlayersOfferArea( UINT8 ubProfileID, INVENTORY_IN_SLOT* pInvSlot, INT8 bSlotIdInOtherLocation )
{
	INT8	bCnt;

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
		//if there are no items here, copy the data in
		if( PlayersOfferArea[bCnt].fActive == FALSE )
		{
			memcpy( &PlayersOfferArea[bCnt], pInvSlot, sizeof( INVENTORY_IN_SLOT ) );

			PlayersOfferArea[bCnt].fActive = TRUE;

			//Specify the owner of the merc
			PlayersOfferArea[bCnt].ubIdOfMercWhoOwnsTheItem = ubProfileID;
			PlayersOfferArea[bCnt].bSlotIdInOtherLocation = bSlotIdInOtherLocation;

			IfMercOwnedCopyItemToMercInv( &(PlayersOfferArea[bCnt]) );

			SetSkiRegionHelpText( &PlayersOfferArea[bCnt], &gPlayersOfferSlotsMouseRegions[ bCnt ], PLAYERS_OFFER_AREA );
			SetSkiFaceRegionHelpText( &PlayersOfferArea[bCnt], &gPlayersOfferSlotsSmallFaceMouseRegions[ bCnt ], PLAYERS_OFFER_AREA );


			//if the item we are adding is money
			if( Item[ PlayersOfferArea[ bCnt ].sItemIndex ].usItemClass == IC_MONEY )
			{
				//Since money is always evaluated
				PlayersOfferArea[ bCnt ].uiFlags |= ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
				PlayersOfferArea[ bCnt ].uiItemPrice = PlayersOfferArea[ bCnt ].ItemObject.uiMoneyAmount;
			}
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

			return( bCnt );
		}
	}

	return( -1 );
}

BOOLEAN RemoveItemFromPlayersOfferArea( INT8 bSlot )
{
	//if the item doesn't have a duplicate copy in its owner merc's inventory slot
	if( PlayersOfferArea[ bSlot ].bSlotIdInOtherLocation == -1 )
	{
		if ( !SKITryToReturnInvToOwnerOrCurrentMerc( &( PlayersOfferArea[ bSlot ] ) ) )
		{
			//failed to add item, inventory probably filled up or item is unowned and current merc ineligible
			return( FALSE );
		}
	}

	// Clear the contents
	ClearPlayersOfferSlot( bSlot );

	// Dirty
	fInterfacePanelDirty = DIRTYLEVEL2;
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
	return( TRUE );
}


void DisplayPlayersOfferArea()
{
	INT16		sCnt, sCount;
	CHAR16	zTemp[32];
	UINT32	uiTotalCost;
	UINT16	usPosX, usPosY;
	BOOLEAN	fDisplayHatchOnItem=FALSE;


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
		if( PlayersOfferArea[ sCnt ].fActive )
		{
			//if the item is money
			if( Item[ PlayersOfferArea[ sCnt ].sItemIndex ].usItemClass == IC_MONEY )
			{
				//get an updated status from the amount in the pocket
				if( PlayersOfferArea[ sCnt ].bSlotIdInOtherLocation != -1 && PlayersOfferArea[ sCnt ].ubIdOfMercWhoOwnsTheItem != NO_PROFILE )
				{
					INT16 sSoldierID;

					sSoldierID = GetSoldierIDFromMercID( PlayersOfferArea[ sCnt ].ubIdOfMercWhoOwnsTheItem );
					Assert(sSoldierID != -1);

					PlayersOfferArea[ sCnt ].ItemObject.uiMoneyAmount = Menptr[ sSoldierID ].inv[ PlayersOfferArea[ sCnt ].bSlotIdInOtherLocation ].uiMoneyAmount;
					PlayersOfferArea[ sCnt ].uiItemPrice = PlayersOfferArea[ sCnt ].ItemObject.uiMoneyAmount;
				}
			}
			else	// not money
			{
				//if non-repairman
				if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_REPAIRS )
				{
					// don't evaluate anything he wouldn't buy!
					if( WillShopKeeperRejectObjectsFromPlayer( gbSelectedArmsDealerID, ( INT8 ) sCnt ) == FALSE )
					{
						// skip purchased items!
						if ( !( PlayersOfferArea[ sCnt ].uiFlags & ARMS_INV_JUST_PURCHASED ) )
						{
							// re-evaluate the value of the item (needed for Flo's discount handling)
							EvaluateInvSlot( &( PlayersOfferArea[ sCnt ] ) );
						}
					}
				}
			}

			// hatch it out if it hasn't been evaluated or just purchased
//			fDisplayHatchOnItem = ( PlayersOfferArea[ sCnt ].uiFlags & ( ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED | ARMS_INV_JUST_PURCHASED ) ) == 0;

			// Display the inventory slot
			DisplayInvSlot( (UINT8)sCnt, PlayersOfferArea[ sCnt ].sItemIndex, usPosX, usPosY,
											&PlayersOfferArea[ sCnt ].ItemObject,
											fDisplayHatchOnItem,
											PLAYERS_OFFER_AREA );

			if( PlayersOfferArea[ sCnt ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE )
				uiTotalCost += PlayersOfferArea[ sCnt ].uiItemPrice;
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
		swprintf( zTemp, lengthof(zTemp), L"%d", uiTotalCost );
		InsertCommasForDollarFigure( zTemp );
		InsertDollarSignInToString( zTemp );
		DrawTextToScreen( zTemp, SKI_PLAYERS_TOTAL_VALUE_X, (UINT16)(SKI_PLAYERS_TOTAL_VALUE_Y+5), SKI_INV_SLOT_WIDTH, SKI_LABEL_FONT, SKI_ITEM_PRICE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
	}

	CrossOutUnwantedItems( );
}


INVENTORY_IN_SLOT	*GetPtrToOfferSlotWhereThisItemIs( UINT8 ubProfileID, INT8 bInvPocket )
{
	UINT8 ubCnt = 0;

	for( ubCnt = 0; ubCnt < SKI_NUM_TRADING_INV_SLOTS; ubCnt++ )
	{
		if( ( PlayersOfferArea[ ubCnt ].bSlotIdInOtherLocation == bInvPocket ) &&
				( PlayersOfferArea[ ubCnt ].ubIdOfMercWhoOwnsTheItem == ubProfileID ) &&
				( PlayersOfferArea[ ubCnt ].ItemObject.ubNumberOfObjects != 0 ) )
		{
			return( &( PlayersOfferArea[ ubCnt ] ) );
		}

		if( ( ArmsDealerOfferArea[ ubCnt ].bSlotIdInOtherLocation == bInvPocket ) &&
				( ArmsDealerOfferArea[ ubCnt ].ubIdOfMercWhoOwnsTheItem == ubProfileID ) &&
				( ArmsDealerOfferArea[ ubCnt ].ItemObject.ubNumberOfObjects != 0 ) )
		{
			return( &( ArmsDealerOfferArea[ ubCnt ] ) );
		}
	}

	// not found (perfectly valid result)
	return( NULL );
}


BOOLEAN ShouldSoldierDisplayHatchOnItem( UINT8 ubProfileID, INT16 sSlotNum )
{
	INVENTORY_IN_SLOT	*pInvSlot = NULL;

	pInvSlot = GetPtrToOfferSlotWhereThisItemIs( ubProfileID, ( INT8 ) sSlotNum );

	if( pInvSlot == NULL )
		// not found in either offer area
		return( FALSE );
	else
		// found it, display it hatched out
		return( TRUE );
}


UINT32 CalculateTotalArmsDealerCost()
{
	UINT32	uiCnt;
	UINT32	uiTotal=0;

	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		if( ArmsDealerOfferArea[ uiCnt ].fActive )
		{
			//if the dealer repairs
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
				uiTotal += CalculateObjectItemRepairCost( gbSelectedArmsDealerID, &( ArmsDealerOfferArea[ uiCnt ].ItemObject ) );
			else
				uiTotal += CalcShopKeeperItemPrice( DEALER_SELLING, FALSE, ArmsDealerOfferArea[ uiCnt ].sItemIndex, ArmsDealerInfo[ gbSelectedArmsDealerID ].dSellModifier, &ArmsDealerOfferArea[uiCnt].ItemObject );
		}
	}
	return( uiTotal );
}


UINT32 CalculateTotalPlayersValue()
{
	UINT8	ubCnt;
	UINT32 uiTotal = 0;

	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		if( ( PlayersOfferArea[ ubCnt ].fActive ) && ( PlayersOfferArea[ ubCnt ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE ) )
		{
			//Calculate a price for the item
			if( Item[ PlayersOfferArea[ ubCnt ].sItemIndex ].usItemClass == IC_MONEY )
				uiTotal += PlayersOfferArea[ ubCnt ].ItemObject.uiMoneyAmount;
			else
				uiTotal += PlayersOfferArea[ ubCnt ].uiItemPrice;
		}
	}

	return( uiTotal );
}


void PerformTransaction( UINT32 uiMoneyFromPlayersAccount )
{
	UINT32	uiPlayersTotalMoneyValue = CalculateTotalPlayersValue() + uiMoneyFromPlayersAccount;
	UINT32	uiArmsDealersItemsCost = CalculateTotalArmsDealerCost();
	UINT32	uiMoneyInPlayersOfferArea = CalculateHowMuchMoneyIsInPlayersOfferArea( );
	INT32		iChangeToGiveToPlayer = 0;
	UINT32	uiAvailablePlayerOfferSlots;


	//if the player has already requested to leave, get out
	if( gfUserHasRequestedToLeave )
		return;


	// handle bribing... if the player is giving the dealer money, without buying anything
	if( IsMoneyTheOnlyItemInThePlayersOfferArea( ) && CountNumberOfItemsInTheArmsDealersOfferArea( ) == 0 )
	{
		// accept/refuse money (varies by dealer according to ACCEPTS_GIFTS flag)
		StartShopKeeperTalking( SK_QUOTES_DEALER_OFFERED_MONEY_AS_A_GIFT );

		// if the arms dealer is the kind of person who accepts gifts
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].uiFlags & ARMS_DEALER_ACCEPTS_GIFTS )
		{
			//Move all the players evaluated items to the dealer (also adds it to dealer's cash)
			MovePlayerOfferedItemsOfValueToArmsDealersInventory();

			DealerGetsBribed( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubShopKeeperID, uiMoneyInPlayersOfferArea );
		}
	}
	else	// not a bribe
	{
		//if the dealer is not a repairman
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_REPAIRS )
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
				DoSkiMessageBox( MSG_BOX_BASIC_STYLE, SKI_Text[ SKI_TEXT_NO_MORE_ROOM_IN_PLAYER_OFFER_AREA ], SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK, NULL );

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
				SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 6,0, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );
				SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SKIP_A_FRAME, 0,0, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );
				SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 0, ( uiArmsDealersItemsCost - ( LaptopSaveInfo.iCurrentBalance + uiPlayersTotalMoneyValue ) ), 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );
			}
			else
			{
				// player doesn't have enough on the table, but can pay for it from his balance
				/// ask player if wants to subtract the shortfall directly from his balance
				SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SKIP_A_FRAME, 0,0, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );
				SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 6,0, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );

				if( uiPlayersTotalMoneyValue )
					SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 1, ( uiArmsDealersItemsCost - uiPlayersTotalMoneyValue ), 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );
				else
					SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 2, ( uiArmsDealersItemsCost - uiPlayersTotalMoneyValue ), 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );
			}

			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 7,0, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );

			gfResetShopKeepIdleQuote = TRUE;

			// disable further calls to PerformTransaction() until callback calls us again to free this flag
			gfPerformTransactionInProgress = TRUE;

			return;
		}


		// to get this far, player should have the money needed!
		Assert( uiPlayersTotalMoneyValue >= uiArmsDealersItemsCost );


		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			//Make sure there is enough money in the Player's offer area to cover the repair charge
			if( ( uiMoneyFromPlayersAccount + uiMoneyInPlayersOfferArea ) >= uiArmsDealersItemsCost )
			{
				//Move all the players evaluated items to the dealer ( For a repairman, this means just: get rid of the money )
				MovePlayerOfferedItemsOfValueToArmsDealersInventory();

				//if the arms dealer is the type of person to give change
				if( ArmsDealerInfo[ gbSelectedArmsDealerID ].uiFlags & ARMS_DEALER_GIVES_CHANGE )
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
		else	// non-repairman
		{
			// DON'T include uiMoneyFromPlayersAccount in change given to player.  The only way this can happen is if he agrees
			// to subtract from his balance, but there isn't enough room.  In that case, the cost of items not transfered is
			// used to reduce his balance deduction instead (not here, in the calling function).
			iChangeToGiveToPlayer = CalculateTotalPlayersValue() - uiArmsDealersItemsCost;


			//if the arms dealer buys stuff
			if( ( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_BUYS_ONLY ) ||
					( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_BUYS_SELLS ) )
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
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].uiFlags & ARMS_DEALER_GIVES_CHANGE )
			{
				if( iChangeToGiveToPlayer > 0 )
				{
					GivePlayerSomeChange( iChangeToGiveToPlayer );

/*
					//Remove the change the arms dealer is returning to the player
					if( ( gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash - uiChangeToGiveToPlayer ) >= 0 )
						gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash -= uiChangeToGiveToPlayer;
					else
						gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash = 0;
*/

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


			//if the arms dealer is tony
			if( gbSelectedArmsDealerID == ARMS_DEALER_TONY )
				gfHavePurchasedItemsFromTony = TRUE;

			//if the arms dealer is Howard( 125), set fact 222
			if( gbSelectedArmsDealerID == ARMS_DEALER_HOWARD )
				SetFactTrue( 222 );
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


void MoveAllArmsDealersItemsInOfferAreaToPlayersOfferArea( )
{
	//for all items in the dealers items offer area
	UINT32	uiCnt;
	INT8		bSlotID=0;

	//loop through all the slots in the shopkeeper's offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		//if there is an item here
		if( ArmsDealerOfferArea[ uiCnt ].fActive )
		{
			bSlotID = AddItemToPlayersOfferArea( NO_PROFILE, &ArmsDealerOfferArea[ uiCnt ], -1 );

			if( bSlotID != -1 )
			{
				PlayersOfferArea[ bSlotID ].uiFlags |= ARMS_INV_JUST_PURCHASED;

				//if the player has just purchased a VIDEO_CAMERA from Franz Hinkle
				if( ( gbSelectedArmsDealerID == ARMS_DEALER_FRANZ ) && ( ArmsDealerOfferArea[ uiCnt ].sItemIndex == VIDEO_CAMERA ) )
				{
					// set a special flag
					gArmsDealerStatus[ gbSelectedArmsDealerID ].ubSpecificDealerFlags |= ARMS_DEALER_FLAG__FRANZ_HAS_SOLD_VIDEO_CAMERA_TO_PLAYER;
				}
			}


			//Remove the items out of the dealers inventory
			RemoveItemFromDealersInventory( &ArmsDealerOfferArea[ uiCnt ], (UINT8)ArmsDealerOfferArea[ uiCnt ].bSlotIdInOtherLocation );

			//Remove the items from the Shopkeepers Offer area
			if( !RemoveItemFromArmsDealerOfferArea( (UINT8)uiCnt, FALSE ) )//ArmsDealerOfferArea[uiCnt].bSlotIdInOtherLocation
				Assert( 0 );

			Assert(!ArmsDealerOfferArea[uiCnt].fActive);
		}
	}

	//loop through all the slots in the shopkeeper's offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		Assert(!ArmsDealerOfferArea[uiCnt].fActive);
	}

}

BOOLEAN RemoveItemFromDealersInventory( INVENTORY_IN_SLOT* pInvSlot, UINT8 ubSlot )
{
	INT16		sInvSlot;
	INT16		sItemID;
	SPECIAL_ITEM_INFO SpclItemInfo;

	sInvSlot = ubSlot;
//	sInvSlot = ( gSelectArmsDealerInfo.ubCurrentPage - 1 ) * SKI_NUM_ARMS_DEALERS_INV_SLOTS + ubSlot;

	//Remove all of this item out of the specified inventory slot
	sItemID = gpTempDealersInventory[ sInvSlot ].sItemIndex;
	SetSpecialItemInfoFromObject( &SpclItemInfo, &(pInvSlot->ItemObject) );
	RemoveItemFromArmsDealerInventory( gbSelectedArmsDealerID, sItemID, &SpclItemInfo, pInvSlot->ItemObject.ubNumberOfObjects );

	gfResetShopKeepIdleQuote = TRUE;
	return( TRUE );
}

void MovePlayerOfferedItemsOfValueToArmsDealersInventory()
{
	UINT32	uiCnt;

	//loop through all the slots in the players offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		//if there is an item here
		if( PlayersOfferArea[ uiCnt ].fActive )
		{
			// and it has value
			if( PlayersOfferArea[ uiCnt ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE )
			{
				//Remove the item from the owner merc's inventory
				IfMercOwnedRemoveItemFromMercInv( &(PlayersOfferArea[ uiCnt ]) );

				//if the item is money
				if( Item[ PlayersOfferArea[ uiCnt ].sItemIndex ].usItemClass == IC_MONEY )
				{
					//add the money to the dealers 'cash'
					gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash += PlayersOfferArea[ uiCnt ].ItemObject.uiMoneyAmount;
				}
				else
				{
					//if the dealer doesn't strictly buy items from the player, give the item to the dealer
					if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_BUYS_ONLY )
					{
						// item cease to be merc-owned during this operation
						AddObjectToArmsDealerInventory( gbSelectedArmsDealerID, &( PlayersOfferArea[ uiCnt ].ItemObject ) );
					}
				}

				//erase the item from the player's offer area
				ClearPlayersOfferSlot( uiCnt );
			}
		}
	}

	gfResetShopKeepIdleQuote = TRUE;
}



void BeginSkiItemPointer( UINT8 ubSource, INT8 bSlotNum, BOOLEAN fOfferToDealerFirst )
{
	SGPRect			Rect;
	OBJECTTYPE	TempObject;

/*
	// If we are already moving an item
	if ( gMoveingItem.sItemIndex != 0 )
	{
		return;
	}
*/

	switch( ubSource )
	{
		case ARMS_DEALER_INVENTORY:
			//Should never get in here
			Assert( 0 );
			return;
			break;

		case ARMS_DEALER_OFFER_AREA:
			//Get the item from the slot.
			memcpy( &gMoveingItem, &ArmsDealerOfferArea[ bSlotNum ], sizeof( INVENTORY_IN_SLOT ) );
			IfMercOwnedRemoveItemFromMercInv( &gMoveingItem );

			//remove the item from the slot
			ClearArmsDealerOfferSlot( bSlotNum );


			//Restrict the cursor to the arms dealer offer area, players offer area and the players inventory
			Rect.iLeft = 0;//SKI_ITEM_MOVEMENT_AREA_X;
			Rect.iTop = SKI_DEALER_OFFER_AREA_Y;
			Rect.iRight = SKI_ITEM_MOVEMENT_AREA_X + SKI_ITEM_MOVEMENT_AREA_WIDTH;
			Rect.iBottom = SKI_ITEM_MOVEMENT_AREA_Y + SKI_ITEM_MOVEMENT_AREA_HEIGHT;

			gpItemPointer = &gMoveingItem.ItemObject;

			//if there is an owner of the item
			#if 0 /* XXX -1 */
			if( gMoveingItem.ubIdOfMercWhoOwnsTheItem != -1 )
			#else
			if (gMoveingItem.ubIdOfMercWhoOwnsTheItem != (UINT8)-1)
			#endif
			{
				gpItemPointerSoldier = FindSoldierByProfileID( gMoveingItem.ubIdOfMercWhoOwnsTheItem, TRUE );
				//make sure the soldier is not null
				if( gpItemPointerSoldier == NULL )
				{
					gpItemPointerSoldier = gpSMCurrentMerc;
				}
			}
			else
				gpItemPointerSoldier = gpSMCurrentMerc;

			break;


		case PLAYERS_OFFER_AREA:
			//Get the item from the slot.
			memcpy( &gMoveingItem, &PlayersOfferArea[ bSlotNum ], sizeof( INVENTORY_IN_SLOT ) );

			// if the slot is overloaded (holds more objects than we have room for valid statuses of)
			if ( PlayersOfferArea[ bSlotNum ].ItemObject.ubNumberOfObjects > MAX_OBJECTS_PER_SLOT )
			{
				// allow only MAX_OBJECTS_PER_SLOT of those objects to be picked up at a time
				// (sure it kind of sucks, but it's a lot easier than handling overloaded cursor objects in Interface Items!
				gMoveingItem.ItemObject.ubNumberOfObjects = MAX_OBJECTS_PER_SLOT;

				// decrease the number objects left in the slot by that much instead of deleting it
				PlayersOfferArea[ bSlotNum ].ItemObject.ubNumberOfObjects -= MAX_OBJECTS_PER_SLOT;
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

			gpItemPointer = &gMoveingItem.ItemObject;

			//if there is an owner of the item
			#if 0 /* XXX -1 */
			if( gMoveingItem.ubIdOfMercWhoOwnsTheItem != -1 )
			#else
			if (gMoveingItem.ubIdOfMercWhoOwnsTheItem != (UINT8)-1)
			#endif
			{
				gpItemPointerSoldier = FindSoldierByProfileID( gMoveingItem.ubIdOfMercWhoOwnsTheItem, TRUE );
				//make sure the soldier is not null
				if( gpItemPointerSoldier == NULL )
				{
					gpItemPointerSoldier = gpSMCurrentMerc;
				}
			}
			else
				gpItemPointerSoldier = gpSMCurrentMerc;


			break;


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
				CopyObj( gpItemPointer, &TempObject );

				//ARM: The memset was previously commented out, in order to preserve the owning merc's inv slot # during a swap of
				// items in an inventory slot.  However, that leads to other bugs: if you picked the thing you're swapping in from
				// a restricted inv slot (headgear, vest, etc.), the item swapped out will end up belonging to an illegal slot, and
				// return there with a right click on it in the player's offer area.  So now ALL items picked up here are unowned.
				memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );

				//Get the item from the pointer
				memcpy( &gMoveingItem.ItemObject, &TempObject, sizeof( OBJECTTYPE ) );

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

				gpItemPointer = &gMoveingItem.ItemObject;
				gpItemPointerSoldier = gpSMCurrentMerc;
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
		MSYS_EnableRegion( &gSkiInventoryMovementAreaMouseRegions );

		RestrictMouseCursor( &Rect );

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
	SGPRect			Rect;

	Rect.iLeft = 0;//SKI_ITEM_MOVEMENT_AREA_X;
	Rect.iTop = SKI_ITEM_MOVEMENT_AREA_Y;
	Rect.iRight = SKI_ITEM_MOVEMENT_AREA_X + SKI_ITEM_MOVEMENT_AREA_WIDTH;
	Rect.iBottom = SKI_ITEM_MOVEMENT_AREA_Y + SKI_ITEM_MOVEMENT_AREA_HEIGHT;

	RestrictMouseCursor( &Rect );
}


void SetSkiCursor( UINT16	usCursor )
{
	UINT8	ubCnt;

	//if we are setting up an item as a cursor
	if( usCursor == EXTERN_CURSOR )
	{
//		EnableSMPanelButtons( FALSE, FALSE );

		// if the current merc is in range
		if( !gfSMDisableForItems )
		{
			// hatch out unavailable merc inventory slots
			ReevaluateItemHatches( gpSMCurrentMerc, FALSE );
		}

		// Set mouse
		guiExternVo = GetInterfaceGraphicForItem( &(Item[ gMoveingItem.sItemIndex ]) );
		gusExternVoSubIndex = Item[ gMoveingItem.sItemIndex ].ubGraphicNum;
		SetCurrentCursorFromDatabase( EXTERN_CURSOR );

		MSYS_ChangeRegionCursor( &gSMPanelRegion, usCursor );

		MSYS_ChangeRegionCursor( &gSKI_EntireScreenMouseRegions, usCursor );

		MSYS_ChangeRegionCursor( &gArmsDealersDropItemToGroundMouseRegions, usCursor );

		MSYS_SetCurrentCursor( usCursor );

		//if the item desc window is up
		if( gInvDesc.uiFlags & MSYS_REGION_EXISTS )
			MSYS_ChangeRegionCursor( &gInvDesc, usCursor );

		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++)
		{
			if( gItemDescAttachmentRegions[ubCnt].uiFlags & MSYS_REGION_EXISTS )
				MSYS_ChangeRegionCursor( &gItemDescAttachmentRegions[ubCnt], usCursor );
		}

		for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
		{
			MSYS_ChangeRegionCursor( &gPlayersOfferSlotsMouseRegions[ ubCnt], usCursor );
			MSYS_ChangeRegionCursor( &gPlayersOfferSlotsSmallFaceMouseRegions[ ubCnt], usCursor );

			MSYS_ChangeRegionCursor( &gDealersOfferSlotsMouseRegions[ ubCnt], usCursor );

			//if the dealer repairs
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
			{
				MSYS_ChangeRegionCursor( &gDealersOfferSlotsSmallFaceMouseRegions[ ubCnt], usCursor );
			}
		}

		MSYS_ChangeRegionCursor( &gSkiInventoryMovementAreaMouseRegions, usCursor );
	}

	//else we are restoring the old cursor
	else
	{
		memset( &gMoveingItem, 0, sizeof( INVENTORY_IN_SLOT ) );

//		gpSkiItemPointer = NULL;
		gpItemPointer = NULL;

		DisableTacticalTeamPanelButtons( FALSE );

//		EnableSMPanelButtons( TRUE, FALSE );
//		CheckForDisabledForGiveItem( );

		// if the current merc is in range
		if( !gfSMDisableForItems )
		{
			// make all merc inventory slots available again
			ReevaluateItemHatches( gpSMCurrentMerc, TRUE );
		}

		MSYS_ChangeRegionCursor( &gSMPanelRegion, usCursor );

		MSYS_ChangeRegionCursor( &gSKI_EntireScreenMouseRegions, usCursor );

		MSYS_ChangeRegionCursor( &gArmsDealersDropItemToGroundMouseRegions, usCursor );

		for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
		{
			MSYS_ChangeRegionCursor( &gPlayersOfferSlotsMouseRegions[ ubCnt ], usCursor );
			MSYS_ChangeRegionCursor( &gPlayersOfferSlotsSmallFaceMouseRegions[ ubCnt ], usCursor );

			MSYS_ChangeRegionCursor( &gDealersOfferSlotsMouseRegions[ ubCnt ], usCursor );

			//if the dealer repairs
			if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
			{
				MSYS_ChangeRegionCursor( &gDealersOfferSlotsSmallFaceMouseRegions[ ubCnt ], usCursor );
			}
		}

		//if the item desc window is up
		if( gInvDesc.uiFlags & MSYS_REGION_EXISTS )
			MSYS_ChangeRegionCursor( &gInvDesc, usCursor );

		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++)
		{
			if( gItemDescAttachmentRegions[ubCnt].uiFlags & MSYS_REGION_EXISTS )
				MSYS_ChangeRegionCursor( &gItemDescAttachmentRegions[ubCnt], usCursor );
		}


		MSYS_ChangeRegionCursor( &gSkiInventoryMovementAreaMouseRegions, usCursor );

		MSYS_SetCurrentCursor( usCursor );

		SetCurrentCursorFromDatabase( usCursor );

		FreeMouseCursor();

		EnableAllDealersInventorySlots();
		EnableAllDealersOfferSlots();

		//Disable the region that limits the movement of the cursor with the item
		MSYS_DisableRegion( &gSkiInventoryMovementAreaMouseRegions );
	}

	SetCurrentCursorFromDatabase( usCursor );

	// make sure disabled slot hatching gets updated when items picked up / dropped
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}


INT8 AddInventoryToSkiLocation( INVENTORY_IN_SLOT *pInv, UINT8 ubSpotLocation, UINT8 ubWhere )
{
	INT8 bSlotAddedTo = -1;


	switch( ubWhere )
	{
		case ARMS_DEALER_INVENTORY:
		case PLAYERS_INVENTORY:
			// not used this way
			Assert( 0 );
			return( bSlotAddedTo );
			break;

		case ARMS_DEALER_OFFER_AREA:

			//If we can add the item into the slot that was clicked on
			if( ArmsDealerOfferArea[ ubSpotLocation ].fActive == FALSE )
			{
				memcpy( &ArmsDealerOfferArea[ ubSpotLocation ], pInv, sizeof( INVENTORY_IN_SLOT ) );
				IfMercOwnedCopyItemToMercInv( pInv );

				SetSkiRegionHelpText( &ArmsDealerOfferArea[ ubSpotLocation ], &gDealersOfferSlotsMouseRegions[ ubSpotLocation ], ARMS_DEALER_OFFER_AREA );
				SetSkiFaceRegionHelpText( &ArmsDealerOfferArea[ ubSpotLocation ], &gDealersOfferSlotsSmallFaceMouseRegions[ ubSpotLocation ], ARMS_DEALER_OFFER_AREA );

				bSlotAddedTo = ubSpotLocation;
			}
			else
			{
				bSlotAddedTo = AddItemToArmsDealerOfferArea( pInv, -1 );
			}
			break;

		case PLAYERS_OFFER_AREA:

			//If we can add the item into the slot that was clicked on
			if( PlayersOfferArea[ ubSpotLocation ].fActive == FALSE )
			{
				// put it down in that player offer area slot
				memcpy( &PlayersOfferArea[ ubSpotLocation ], pInv, sizeof( INVENTORY_IN_SLOT ) );
				IfMercOwnedCopyItemToMercInv( pInv );

				//if the item is money
				if( Item[ PlayersOfferArea[ ubSpotLocation ].sItemIndex ].usItemClass == IC_MONEY )
				{
					//Since money is always evaluated
					PlayersOfferArea[ ubSpotLocation ].uiFlags |= ARMS_INV_PLAYERS_ITEM_HAS_VALUE;
					PlayersOfferArea[ ubSpotLocation ].uiItemPrice = PlayersOfferArea[ ubSpotLocation ].ItemObject.uiMoneyAmount;
				}

				SetSkiRegionHelpText( &PlayersOfferArea[ ubSpotLocation ], &gPlayersOfferSlotsMouseRegions[ ubSpotLocation ], PLAYERS_OFFER_AREA );
				SetSkiFaceRegionHelpText( &PlayersOfferArea[ ubSpotLocation ], &gPlayersOfferSlotsSmallFaceMouseRegions[ ubSpotLocation ], PLAYERS_OFFER_AREA );

				bSlotAddedTo = ubSpotLocation;
			}
//			else if( )
//			{
				//check to see the type we are adding is the same as the type that is already there
//			}
			else	// that slot is full
			{
				// add it elsewhere
				bSlotAddedTo = AddItemToPlayersOfferArea( pInv->ubIdOfMercWhoOwnsTheItem, pInv, pInv->bSlotIdInOtherLocation );
			}

		break;
	}

	//Redraw the screen
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL1;
	gfResetShopKeepIdleQuote = TRUE;
	fInterfacePanelDirty = DIRTYLEVEL2;

	return( bSlotAddedTo );
}


void DisplayTalkingArmsDealer()
{
	static BOOLEAN	fWasTheMercTalking= FALSE;
//	static UINT32		uiLastTime=0;
//	UINT32					uiCurTime = GetJA2Clock();
//	static	UINT32	uiMinimumLengthForTalkingText;


	//Make sure the Dealers doesn't get disabled
	gFacesData[ giShopKeeperFaceIndex ].fDisabled = FALSE;

	HandleDialogue();

	//Gets handled when we render the tactical interface
	HandleAutoFaces( );

	HandleTalkingAutoFaces( );
	HandleShopKeeperDialog( 0 );


 	gfIsTheShopKeeperTalking = gFacesData[ giShopKeeperFaceIndex ].fTalking;

	//if the merc just started talking
//	if( gfIsTheShopKeeperTalking && !fWasTheMercTalking )
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


void HandleShopKeeperDialog( UINT8 ubInit )
{
	UINT32	uiCurTime = GetJA2Clock();

	static	UINT32	uiLastTime = 0;
	static	UINT32	uiLastTimeSpoke = 0;
	static	INT8		bSpeech = -1;


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
			uiLastTimeSpoke = GetJA2Clock();
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
		if( !gFacesData[ giShopKeeperFaceIndex ].fTalking )
		{
			SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 5, 0, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );
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
				if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
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



BOOLEAN StartShopKeeperTalking( UINT16 usQuoteNum )
{
	BOOLEAN fSuccess;

	// if already in the process of leaving, don't start any additional quotes
	if ( gfSKIScreenExit || gfRemindedPlayerToPickUpHisStuff || gfUserHasRequestedToLeave )
	{
		return( FALSE );
	}

	// post event to mark shopkeeper dialogue in progress
	SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 3, usQuoteNum, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );

	// post quote dialogue
	fSuccess = CharacterDialogue( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubShopKeeperID, usQuoteNum, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI, FALSE, FALSE );

	// post event to mark shopkeeper dialogue as ended
	SpecialCharacterDialogueEvent( DIALOGUE_SPECIAL_EVENT_SHOPKEEPER, 4, usQuoteNum, 0, giShopKeeperFaceIndex, DIALOGUE_SHOPKEEPER_UI );

	if ( !fSuccess )
	{
		return( FALSE );
	}

	gfResetShopKeepIdleQuote = TRUE;
	return( TRUE );
}



BOOLEAN IsGunOrAmmoOfSameTypeSelected( OBJECTTYPE	*pItemObject )
{
	//if there is no item selected, return
	if( gpHighLightedItemObject == NULL )
		return( FALSE );

	//if the item is ammo
	if( Item[ gpHighLightedItemObject->usItem ].usItemClass == IC_AMMO )
	{
		//if there is a gun
		if( Item[ pItemObject->usItem ].usItemClass == IC_GUN )
		{
			//of the same caliber
			if( Weapon[ pItemObject->usItem ].ubCalibre == Magazine[ Item[ gpHighLightedItemObject->usItem ].ubClassIndex ].ubCalibre )
			{
				return( TRUE );
			}
		}
	}

	//else if the item is a gun
	else if( Item[ gpHighLightedItemObject->usItem ].usItemClass == IC_GUN )
	{
		//if there is a gun
		if( Item[ pItemObject->usItem ].usItemClass == IC_AMMO )
		{
			//of the same caliber
			if( Weapon[ gpHighLightedItemObject->usItem ].ubCalibre == Magazine[ Item[ pItemObject->usItem ].ubClassIndex ].ubCalibre )
			{
				return( TRUE );
			}
		}
	}


	//if the highlighted object is an attachment
	if( Item[ pItemObject->usItem ].fFlags & ITEM_ATTACHMENT )
	{
		if( ValidAttachment( pItemObject->usItem, gpHighLightedItemObject->usItem ) )
			return( TRUE );
	}
	else
	{
		if( ValidAttachment( gpHighLightedItemObject->usItem, pItemObject->usItem ) )
			return( TRUE );
	}


	return( FALSE );
}


void InitShopKeeperSubTitledText( STR16 pString )
{
	//Clear the contents of the subtitle text
	memset( gsShopKeeperTalkingText, 0, SKI_SUBTITLE_TEXT_SIZE );

	swprintf( gsShopKeeperTalkingText, lengthof(gsShopKeeperTalkingText), L"\"%S\"", pString );

	//Now setup the popup box
	if( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
	{
		UINT16 usActualWidth=0;
		UINT16 usActualHeight=0;

		giPopUpBoxId = PrepareMercPopupBox( giPopUpBoxId, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, gsShopKeeperTalkingText, 300, 0, 0, 0, &usActualWidth, &usActualHeight);

//		gusPositionOfSubTitlesX = ( 640 - usActualWidth ) / 2 ;
		//position it to start under the guys face
		gusPositionOfSubTitlesX = 13;

		RenderMercPopUpBoxFromIndex( giPopUpBoxId, gusPositionOfSubTitlesX, SKI_POSITION_SUBTITLES_Y, FRAME_BUFFER);

		//check to make sure the region is not already initialized
		if( !( gShopKeeperSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS ) )
		{
			MSYS_DefineRegion( &gShopKeeperSubTitleMouseRegion, gusPositionOfSubTitlesX, SKI_POSITION_SUBTITLES_Y, (INT16)(gusPositionOfSubTitlesX + usActualWidth), (INT16)(SKI_POSITION_SUBTITLES_Y + usActualHeight), MSYS_PRIORITY_HIGH,
										 CURSOR_NORMAL, MSYS_NO_CALLBACK, ShopKeeperSubTitleRegionCallBack );
			MSYS_AddRegion( &gShopKeeperSubTitleMouseRegion );
		}

		//redraw the screen
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

	}
}

void RemoveShopKeeperSubTitledText()
{
	if( giPopUpBoxId == -1 )
		return;

	if( RemoveMercPopupBoxFromIndex( giPopUpBoxId ) )
	{
		giPopUpBoxId = -1;
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

		//Get rid of the subtitles region
		if( gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
			MSYS_RemoveRegion( &gShopKeeperSubTitleMouseRegion );
	}
}

BOOLEAN AreThereItemsInTheArmsDealersOfferArea( )
{
	UINT8	ubCnt;

	//loop through the players offer area and see if there are any items there
	for( ubCnt = 0; ubCnt < SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if is an item here
		if( ArmsDealerOfferArea[ ubCnt ].fActive )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}

BOOLEAN AreThereItemsInThePlayersOfferArea( )
{
	UINT8	ubCnt;

	//loop through the players offer area and see if there are any items there
	for( ubCnt = 0; ubCnt < SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if is an item here
		if( PlayersOfferArea[ ubCnt ].fActive )
		{
			return( TRUE );
		}
	}
	return( FALSE );
}

//Mouse Call back for the Arms traders inventory slot
void ShopKeeperSubTitleRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP || iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		ShutUpShopKeeper();
	}
}

//Mouse Call back for the Arms delaers face
void SelectArmsDealersFaceRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP || iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		ShutUpShopKeeper();
	}
}

void ShutUpShopKeeper()
{
//	RemoveShopKeeperSubTitledText();

	ShutupaYoFace( giShopKeeperFaceIndex );
	gfIsTheShopKeeperTalking = FALSE;

	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}



UINT8 CountNumberOfValuelessItemsInThePlayersOfferArea( )
{
	UINT8	ubCnt;
	UINT8	ubCount=0;

	//loop through the players offer area and see if there are any items there
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if is an item here
		if( PlayersOfferArea[ubCnt].fActive )
		{
			//and if it has no value
			if( !( PlayersOfferArea[ubCnt].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE ) )
			{
				ubCount++;
			}
		}
	}
	return( ubCount );
}

UINT8 CountNumberOfItemsOfValueInThePlayersOfferArea( )
{
	UINT8	ubCnt;
	UINT8	ubCount=0;

	//loop through the players offer area and see if there are any items there
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if is an item here
		if( PlayersOfferArea[ubCnt].fActive )
		{
			//and if it has not been evaluated
			if( PlayersOfferArea[ubCnt].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE )
			{
				ubCount++;
			}
		}
	}
	return( ubCount );
}


UINT8 CountNumberOfItemsInThePlayersOfferArea( )
{
	UINT8	ubCnt;
	UINT8	ubItemCount=0;

	//loop through the player's offer area and see if there are any items there
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if is an item here
		if( PlayersOfferArea[ubCnt].fActive )
		{
			ubItemCount++;
		}
	}
	return( ubItemCount );
}


UINT8 CountNumberOfItemsInTheArmsDealersOfferArea( )
{
	UINT8	ubCnt;
	UINT8	ubItemCount=0;

	//loop through the dealer's offer area and see if there are any items there
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if is an item here
		if( ArmsDealerOfferArea[ubCnt].fActive )
		{
			ubItemCount++;
		}
	}
	return( ubItemCount );
}


INT8 GetSlotNumberForMerc( UINT8 ubProfile )
{
	INT8	bCnt;

	for( bCnt = 0; bCnt < gubNumberMercsInArray; bCnt++ )
	{
		if( gubArrayOfEmployedMercs[ bCnt ] == ubProfile )
			return( bCnt );
	}

	// not found - not currently working for the player
	return( -1 );
}


void RenderSkiAtmPanel()
{
//	BltVideoObjectFromIndex(FRAME_BUFFER, guiSkiAtmImage, 0, SKI_ATM_PANEL_X, SKI_ATM_PANEL_Y);
}

void CreateSkiAtmButtons()
{
	UINT16	usPosX, usPosY;
	UINT8		ubCount=0;
	UINT8		ubCnt;

	guiSKI_AtmNumButtonImage			= LoadButtonImage("INTERFACE\\TradeScreenAtm.sti", 1,2,-1,3,-1 );
	guiSKI_AtmOkButtonImage				= UseLoadedButtonImage( guiSKI_AtmNumButtonImage, 4,5,-1,6,-1 );
	guiSKI_AtmSideMenuButtonImage	= UseLoadedButtonImage( guiSKI_AtmNumButtonImage, 7,8,-1,9,-1 );


	//Create the number buttons
	usPosX = SKI_ATM_BUTTON_X;
	usPosY = SKI_ATM_BUTTON_Y;
	ubCount=0;
	for( ubCnt=SKI_ATM_1; ubCnt<=SKI_ATM_9;ubCnt++)
	{
		guiSKI_AtmButton[ubCnt] = CreateIconAndTextButton( guiSKI_AtmNumButtonImage, SkiAtmText[ubCnt], SKI_ATM_BUTTON_FONT,
														 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
														 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
														 TEXT_CJUSTIFIED,
														 usPosX, usPosY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH+2,
														 DEFAULT_MOVE_CALLBACK, BtnSKI_AtmButtonCallback );

		SpecifyDisabledButtonStyle( guiSKI_AtmButton[ubCnt], DISABLED_STYLE_SHADED );

		MSYS_SetBtnUserData(guiSKI_AtmButton[ubCnt], 0, ubCnt );
		ubCount++;

		usPosX += SKI_ATM_NUM_BUTTON_WIDTH;

		if( ( ubCount % 3 ) == 0 )
		{
			usPosX = SKI_ATM_BUTTON_X;
			usPosY += SKI_ATM_BUTTON_HEIGHT;
		}
	}

	//Create the zero button
	usPosX = SKI_ATM_BUTTON_X;
	guiSKI_AtmButton[SKI_ATM_0] = CreateIconAndTextButton( guiSKI_AtmOkButtonImage, SkiAtmText[SKI_ATM_0], SKI_ATM_BUTTON_FONT,
													 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
													 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
													 TEXT_CJUSTIFIED,
													 usPosX, usPosY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH+2,
													 DEFAULT_MOVE_CALLBACK, BtnSKI_AtmButtonCallback );
	MSYS_SetBtnUserData(guiSKI_AtmButton[SKI_ATM_0], 0, SKI_ATM_0 );



	//Create the ok button
	usPosX = SKI_ATM_BUTTON_X + SKI_ATM_NUM_BUTTON_WIDTH + 8;
	guiSKI_AtmButton[SKI_ATM_OK] = CreateIconAndTextButton( guiSKI_AtmOkButtonImage, SkiAtmText[SKI_ATM_OK], SKI_ATM_BUTTON_FONT,
													 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
													 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
													 TEXT_CJUSTIFIED,
													 usPosX, usPosY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH+2,
													 DEFAULT_MOVE_CALLBACK, BtnSKI_AtmButtonCallback );
	MSYS_SetBtnUserData(guiSKI_AtmButton[SKI_ATM_OK], 0, SKI_ATM_OK );


	//Create the side menu text buttons
	usPosX = SKI_ATM_BUTTON_X + SKI_ATM_SIDE_MENU_PANEL_START_X;
	usPosY = SKI_ATM_BUTTON_Y;
	for( ubCnt=SKI_ATM_TAKE; ubCnt<=SKI_ATM_CLEAR;ubCnt++ )
	{
		guiSKI_AtmButton[ubCnt] = CreateIconAndTextButton( guiSKI_AtmSideMenuButtonImage, SkiAtmText[ubCnt], SKI_ATM_BUTTON_FONT,
														 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
														 SKI_ATM_BUTTON_COLOR, NO_SHADOW,
														 TEXT_CJUSTIFIED,
														 usPosX, usPosY, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH+2,
														 DEFAULT_MOVE_CALLBACK, BtnSKI_AtmButtonCallback );

		MSYS_SetBtnUserData(guiSKI_AtmButton[ubCnt], 0, ubCnt );

		usPosY += SKI_ATM_BUTTON_HEIGHT;
	}

	//Enable/disable the appropriate buttons
//ATM:
//	EnableDisableSkiAtmButtons();
}


void RemoveSkiAtmButtons()
{
	UINT8		ubCnt;

	//Remove atm images
	UnloadButtonImage( guiSKI_AtmNumButtonImage );
	UnloadButtonImage( guiSKI_AtmOkButtonImage );
	UnloadButtonImage( guiSKI_AtmSideMenuButtonImage );


	//Remove the atm buttons
	for( ubCnt=0; ubCnt<NUM_SKI_ATM_BUTTONS;ubCnt++)
	{
		RemoveButton( guiSKI_AtmButton[ubCnt] );
	}
}


void BtnSKI_AtmButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		UINT8	ubButton = (UINT8) MSYS_GetBtnUserData( btn, 0 );
		btn->uiFlags &= (~BUTTON_CLICKED_ON );

//ATM:

//		HandleSkiAtmPanelInput( ubButton );

		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}



void HandleSkiAtmPanel( )
{
	//Dispaly the appropriate text for the the curret atm mode
	HandleCurrentModeText( gubCurrentSkiAtmMode );

	//Display Atm transfer string
	DisplaySkiAtmTransferString();

	ToggleSkiAtmButtons();
}

void HandleSkiAtmPanelInput( UINT8	ubButtonPress )
{
	//switch on the current mode
	switch( ubButtonPress )
	{
		case SKI_ATM_1:
		case SKI_ATM_2:
		case SKI_ATM_3:
		case SKI_ATM_4:
		case SKI_ATM_5:
		case SKI_ATM_6:
		case SKI_ATM_7:
		case SKI_ATM_8:
		case SKI_ATM_9:
		case SKI_ATM_0:

			AddNumberToSkiAtm( ubButtonPress );
			break;


		case SKI_ATM_OK:
		{
			HandleAtmOK();
			break;
		}


		case SKI_ATM_TAKE:
			gubCurrentSkiAtmMode = SKI_ATM_TAKE_MODE;
			EnableDisableSkiAtmButtons();
			break;


		case SKI_ATM_GIVE:
			gubCurrentSkiAtmMode = SKI_ATM_GIVE_MODE;
			EnableDisableSkiAtmButtons();
			break;


		case SKI_ATM_CANCEL:
			memset( gzSkiAtmTransferString, 0, 32 );
			gubCurrentSkiAtmMode = SKI_ATM_DISABLED_MODE;
			EnableDisableSkiAtmButtons();
			break;


		case SKI_ATM_CLEAR:
			memset( gzSkiAtmTransferString, 0, 32 );
			break;

		default:
			Assert( 0 );
	}
}


void HandleAtmOK()
{
	INT32	iAmountToTransfer;
	BOOLEAN fOkToClear=FALSE;

	//Get the amount to transfer
	swscanf( gzSkiAtmTransferString, L"%d", &iAmountToTransfer );

	if( iAmountToTransfer == 0 )
		return;

	//if we are in an error mode
	if( gubCurrentSkiAtmMode == SKI_ATM_ERR_TAKE_MODE )
		gubCurrentSkiAtmMode = SKI_ATM_TAKE_MODE;
	else if( gubCurrentSkiAtmMode == SKI_ATM_ERR_GIVE_MODE )
		gubCurrentSkiAtmMode = SKI_ATM_GIVE_MODE;

	//if we are in either the give or take mode, exit
	if( gubCurrentSkiAtmMode != SKI_ATM_TAKE_MODE && gubCurrentSkiAtmMode != SKI_ATM_GIVE_MODE )
		return;


	//if we are taking money
	if( gubCurrentSkiAtmMode == SKI_ATM_TAKE_MODE )
	{
		//are we trying to take more then the merc has
		if( iAmountToTransfer > GetFundsOnMerc( gpSMCurrentMerc ) )
		{
			UINT32	uiFundsOnCurrentMerc = GetFundsOnMerc( gpSMCurrentMerc );

			if( uiFundsOnCurrentMerc == 0 )
				memset( gzSkiAtmTransferString, 0, 32 );
			else
			{
				//Set the amount to transfer
				swprintf( gzSkiAtmTransferString, lengthof(gzSkiAtmTransferString), L"%d", uiFundsOnCurrentMerc );
			}

			gubCurrentSkiAtmMode = SKI_ATM_ERR_TAKE_MODE;
			HandleCurrentModeText( gubCurrentSkiAtmMode );
		}
		else
		{
			TransferFundsFromMercToBank( gpSMCurrentMerc, iAmountToTransfer );

			fOkToClear=TRUE;
		}

	}
	else if( gubCurrentSkiAtmMode == SKI_ATM_GIVE_MODE )
	{
		//are we tring to take more then we have?
		if( iAmountToTransfer > LaptopSaveInfo.iCurrentBalance )
		{
			if( LaptopSaveInfo.iCurrentBalance == 0 )
				memset( gzSkiAtmTransferString, 0, 32 );
			else
			{
				//Set the amount to transfer
				swprintf( gzSkiAtmTransferString, lengthof(gzSkiAtmTransferString), L"%d", LaptopSaveInfo.iCurrentBalance );
			}

			gubCurrentSkiAtmMode = SKI_ATM_ERR_GIVE_MODE;
			HandleCurrentModeText( gubCurrentSkiAtmMode );
		}
		else
		{
			TransferFundsFromBankToMerc( gpSMCurrentMerc, iAmountToTransfer );

			fOkToClear=TRUE;
		}
	}

	if( fOkToClear )
	{
		gubCurrentSkiAtmMode = SKI_ATM_DISABLED_MODE;
		EnableDisableSkiAtmButtons();
		memset( gzSkiAtmTransferString, 0, 32 );
	}

	//Redraw the screen
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}


void AddNumberToSkiAtm( UINT8 ubNumber )
{
	CHAR16	zTemp[16];


	//make sure to durt the panel
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

	//if the mode is an error mode
	if( gubCurrentSkiAtmMode == SKI_ATM_ERR_TAKE_MODE )
	{
		gubCurrentSkiAtmMode = SKI_ATM_TAKE_MODE;
		HandleCurrentModeText( gubCurrentSkiAtmMode );
	}
	//else if the mode was
	else if( gubCurrentSkiAtmMode == SKI_ATM_ERR_GIVE_MODE )
	{
		gubCurrentSkiAtmMode = SKI_ATM_GIVE_MODE;
		HandleCurrentModeText( gubCurrentSkiAtmMode );
	}


	//if we are not in the correct mode, return
	if( gubCurrentSkiAtmMode != SKI_ATM_TAKE_MODE && gubCurrentSkiAtmMode != SKI_ATM_GIVE_MODE )
		return;

	//make sure we arent going over the numbers max size
	if( wcslen( gzSkiAtmTransferString ) >= 9 )
		return;

	//if its the first number being added and the number is a zero
	if( gzSkiAtmTransferString[0] == L'\0' && ubNumber == SKI_ATM_0 )
		return;

	swprintf( zTemp, lengthof(zTemp), L"%d", ubNumber );

	//add the number to the current amount
	wcscat( gzSkiAtmTransferString, zTemp );
}


void DisplaySkiAtmTransferString()
{
	CHAR16 zSkiAtmTransferString[ 32 ];
	UINT32	uiMoney;

	//Erase the background behind the string
	ColorFillVideoSurfaceArea( FRAME_BUFFER, SKI_TRANSFER_STRING_X, SKI_TRANSFER_STRING_Y, SKI_TRANSFER_STRING_X+SKI_TRANSFER_STRING_WIDTH,	SKI_TRANSFER_STRING_Y+SKI_TRANSFER_STRING_HEIGHT, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

	wcscpy( zSkiAtmTransferString, gzSkiAtmTransferString );
	InsertCommasForDollarFigure( zSkiAtmTransferString );
	InsertDollarSignInToString( zSkiAtmTransferString );


	//Display the transfer string
	DrawTextToScreen( zSkiAtmTransferString, SKI_TRANSFER_STRING_X, SKI_TRANSFER_STRING_Y, SKI_TRANSFER_STRING_WIDTH, SKI_ATM_BUTTON_FONT, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED );


	//
	//Get and Display the money on the merc
	//
	uiMoney = GetFundsOnMerc( gpSMCurrentMerc );

	swprintf( zSkiAtmTransferString, lengthof(zSkiAtmTransferString), L"%d", uiMoney );

	InsertCommasForDollarFigure( zSkiAtmTransferString );
	InsertDollarSignInToString( zSkiAtmTransferString );

	DrawTextToScreen( zSkiAtmTransferString, SKI_TRANSFER_STRING_X, SKI_MERCS_MONEY_Y, SKI_TRANSFER_STRING_WIDTH, SKI_ATM_BUTTON_FONT, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED );
}



void EnableDisableSkiAtmButtons()
{
	UINT8	ubCnt;

	switch( gubCurrentSkiAtmMode )
	{
		case SKI_ATM_DISABLED_MODE:

			//Disable number positions
			for( ubCnt=SKI_ATM_0; ubCnt<=SKI_ATM_9; ubCnt++)
				DisableButton( guiSKI_AtmButton[ubCnt] );

			DisableButton( guiSKI_AtmButton[SKI_ATM_OK] );
			DisableButton( guiSKI_AtmButton[SKI_ATM_CANCEL] );
			DisableButton( guiSKI_AtmButton[SKI_ATM_CLEAR] );

			EnableButton( guiSKI_AtmButton[ SKI_ATM_TAKE ] );
			EnableButton( guiSKI_AtmButton[ SKI_ATM_GIVE ] );


			break;

		case SKI_ATM_TAKE_MODE:

			//Make sure the numbers are not disbaled
			for( ubCnt=SKI_ATM_0; ubCnt<=SKI_ATM_9; ubCnt++)
				EnableButton( guiSKI_AtmButton[ubCnt] );


			//enable other
			EnableButton( guiSKI_AtmButton[SKI_ATM_OK] );
			EnableButton( guiSKI_AtmButton[SKI_ATM_CANCEL] );
			EnableButton( guiSKI_AtmButton[SKI_ATM_CLEAR] );


			break;


		case SKI_ATM_GIVE_MODE:

			//Make sure the numbers are not disbaled
			for( ubCnt=SKI_ATM_0; ubCnt<=SKI_ATM_9; ubCnt++)
				EnableButton( guiSKI_AtmButton[ubCnt] );

			EnableButton( guiSKI_AtmButton[SKI_ATM_OK] );
			EnableButton( guiSKI_AtmButton[SKI_ATM_CANCEL] );
			EnableButton( guiSKI_AtmButton[SKI_ATM_CLEAR] );



			break;

		default:
			Assert( 0 );
			break;
	}
}



void HandleCurrentModeText( UINT8 ubMode )
{
	CHAR16					zTemp[128];
	CHAR16					zMoney[128];
	static UINT32		uiLastTime=0;
	UINT32					uiCurTime=GetJA2Clock();
	static UINT8		ubLastMode=0;
	static UINT8		ubDisplayMode=0;

	if( ubMode != ubLastMode )
	{
		uiLastTime = uiCurTime;
		ubLastMode = ubMode;
		ubDisplayMode = ubMode;
	}
	else
	{
		//if the current mode is an error mode
		if( ubLastMode != SKI_ATM_ERR_TAKE_MODE && ubLastMode != SKI_ATM_ERR_GIVE_MODE )
		{
			//has the timer gone off?
			if( ( uiCurTime - uiLastTime ) > DELAY_PER_MODE_CHANGE_IN_ATM )
			{
				//display the players current balance

				//if the modes are the same, change to display the players balance
				if( ubLastMode == ubMode )
					if( ubMode == ubDisplayMode )
						ubMode = SKI_ATM_DISPLAY_PLAYERS_BALANCE;

				ubDisplayMode = ubMode;

				uiLastTime = GetJA2Clock();
			}
			else
				ubMode = ubDisplayMode;
		}
	}

	//if the current mode is an error mode
	if( ubLastMode == SKI_ATM_ERR_TAKE_MODE || ubLastMode == SKI_ATM_ERR_GIVE_MODE )
	{
		//if it is time to get rid of the error message
		if( ( uiCurTime - uiLastTime ) > DELAY_PER_MODE_CHANGE_IN_ATM )
		{
			if( ubLastMode == SKI_ATM_ERR_TAKE_MODE )
			{
				ubLastMode = SKI_ATM_TAKE_MODE;
				gubCurrentSkiAtmMode = ubLastMode;
			}
			else
			{
				ubLastMode = SKI_ATM_GIVE_MODE;
				gubCurrentSkiAtmMode = ubLastMode;
			}
		}
	}

	switch( ubMode )
	{
		case SKI_ATM_DISABLED_MODE:
			wcscpy( zTemp, gzSkiAtmText[ SKI_ATM_MODE_TEXT_SELECT_MODE ] );
			break;

		case SKI_ATM_TAKE_MODE:
			//if the player has selected any money yet
			if( gzSkiAtmTransferString[0] == L'\0' )
				wcscpy( zTemp, gzSkiAtmText[ SKI_ATM_MODE_TEXT_ENTER_AMOUNT ] );
			else
				wcscpy( zTemp, gzSkiAtmText[ SKI_ATM_MODE_TEXT_SELECT_FROM_MERC ] );
			break;


		case SKI_ATM_GIVE_MODE:
			//if the player has selected any money yet
			if( gzSkiAtmTransferString[0] == L'\0' )
				wcscpy( zTemp, gzSkiAtmText[ SKI_ATM_MODE_TEXT_ENTER_AMOUNT ] );
			else
				wcscpy( zTemp, gzSkiAtmText[ SKI_ATM_MODE_TEXT_SELECT_TO_MERC ] );
			break;

		case SKI_ATM_ERR_TAKE_MODE:
		case SKI_ATM_ERR_GIVE_MODE:
			wcscpy( zTemp, gzSkiAtmText[ SKI_ATM_MODE_TEXT_SELECT_INUSUFFICIENT_FUNDS ] );
			break;

		case SKI_ATM_DISPLAY_PLAYERS_BALANCE:
			swprintf( zMoney, lengthof(zMoney), L"%d", LaptopSaveInfo.iCurrentBalance );
			InsertCommasForDollarFigure( zMoney );
			InsertDollarSignInToString( zMoney );

			swprintf( zTemp, lengthof(zTemp), L"%S: %S", gzSkiAtmText[ SKI_ATM_MODE_TEXT_BALANCE ], zMoney );
			break;
	}

	DisplayWrappedString( SKI_MODE_TEXT_X, SKI_MODE_TEXT_Y, SKI_MODE_TEXT_WIDTH, 2, SKI_ATM_BUTTON_FONT, FONT_MCOLOR_WHITE, zTemp, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

	//invalidate the atm panel area
	InvalidateRegion( SKI_ATM_PANEL_X, SKI_ATM_PANEL_Y, SKI_ATM_PANEL_X+SKI_ATM_PANEL_WIDTH, SKI_ATM_PANEL_Y+SKI_ATM_PANEL_HEIGHT );
}


void	ToggleSkiAtmButtons()
{
	switch( gubCurrentSkiAtmMode )
	{
		case SKI_ATM_DISABLED_MODE:
			ButtonList[ guiSKI_AtmButton[SKI_ATM_TAKE] ]->uiFlags &= ~BUTTON_CLICKED_ON;
			ButtonList[ guiSKI_AtmButton[SKI_ATM_GIVE] ]->uiFlags &= ~BUTTON_CLICKED_ON;
			ButtonList[ guiSKI_AtmButton[SKI_ATM_CANCEL] ]->uiFlags &= ~BUTTON_CLICKED_ON;
			break;
		case SKI_ATM_TAKE_MODE:
		case SKI_ATM_ERR_TAKE_MODE:
			ButtonList[ guiSKI_AtmButton[SKI_ATM_GIVE] ]->uiFlags &= ~BUTTON_CLICKED_ON;
			ButtonList[ guiSKI_AtmButton[SKI_ATM_CANCEL] ]->uiFlags &= ~BUTTON_CLICKED_ON;

			ButtonList[ guiSKI_AtmButton[SKI_ATM_TAKE] ]->uiFlags |= BUTTON_CLICKED_ON;
			break;

		case SKI_ATM_GIVE_MODE:
		case SKI_ATM_ERR_GIVE_MODE:
			ButtonList[ guiSKI_AtmButton[SKI_ATM_TAKE] ]->uiFlags &= ~BUTTON_CLICKED_ON;
			ButtonList[ guiSKI_AtmButton[SKI_ATM_CANCEL] ]->uiFlags &= ~BUTTON_CLICKED_ON;

			ButtonList[ guiSKI_AtmButton[SKI_ATM_GIVE] ]->uiFlags |= BUTTON_CLICKED_ON;
			break;
	}
}

void EnableDisableDealersInventoryPageButtons()
{
	//if we are on the first page, disable the page up arrow
	if( gSelectArmsDealerInfo.ubCurrentPage <= 1 )
	{
		DisableButton( guiSKI_InvPageUpButton );
	}
	else
	{
		EnableButton( guiSKI_InvPageUpButton );
	}


	//if we are on the last page, disable the page down button
	if( gSelectArmsDealerInfo.ubCurrentPage == gSelectArmsDealerInfo.ubNumberOfPages )
	{
		DisableButton( guiSKI_InvPageDownButton );
	}
	else
	{
		EnableButton( guiSKI_InvPageDownButton );
	}
}

void EnableDisableEvaluateAndTransactionButtons()
{
	UINT8	ubCnt;
	BOOLEAN	fItemsHere=FALSE;
	BOOLEAN	fItemEvaluated=FALSE;
	UINT32	uiArmsDealerTotalCost = CalculateTotalArmsDealerCost();
	UINT32	uiPlayersOfferAreaTotalCost = CalculateTotalPlayersValue();
	UINT32	uiPlayersOfferAreaTotalMoney = CalculateHowMuchMoneyIsInPlayersOfferArea( );


	//loop through the players offer area
	//loop through all the items in the players offer area and determine if they can be sold here.
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if there is an item here
		if( PlayersOfferArea[ ubCnt ].fActive )
		{
			fItemsHere = TRUE;

			//if the item has value
			if( PlayersOfferArea[ ubCnt ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE )
			{
				//if the item isnt money ( which is always evaluated )
				if( Item[ PlayersOfferArea[ ubCnt ].sItemIndex ].usItemClass != IC_MONEY )
					fItemEvaluated = TRUE;

				//else if it is not a repair dealer, and the item is money
				else if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_REPAIRS && Item[ PlayersOfferArea[ ubCnt ].sItemIndex ].usItemClass == IC_MONEY )
					fItemEvaluated = TRUE;
			}
		}

		//if the dealer is a repair dealer
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			//if there is an item here, the item has to have been evaluated
			if( ArmsDealerOfferArea[ ubCnt ].fActive )
			{
				fItemEvaluated = TRUE;
			}
		}
	}

/*
//Evaluate:
	//if there is an item present
	if( fItemsHere )
	{
		EnableButton( guiSKI_EvaluateButton );
	}
	else
	{
		DisableButton( guiSKI_EvaluateButton );
	}
*/


	//if there are evaluated items here
	if( fItemEvaluated )
	{
//		//and if the players offer area value exceeds the shopkeeper's area
//		if( uiPlayersOfferAreaTotalCost >= uiArmsDealerTotalCost )
			EnableButton( guiSKI_TransactionButton );
//		else
//			DisableButton( guiSKI_TransactionButton );

/*
		//if the only item in the players offer area is money, and there is nothing in the dealers offer area
		if( IsMoneyTheOnlyItemInThePlayersOfferArea( ) && uiArmsDealerTotalCost == 0 )
			DisableButton( guiSKI_TransactionButton );
*/

		//If its a repair dealer, and there is no items in the Dealer Offer area,
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS &&
				CountNumberOfItemsInTheArmsDealersOfferArea( ) == 0 &&
				uiPlayersOfferAreaTotalMoney < uiArmsDealerTotalCost
			)
			DisableButton( guiSKI_TransactionButton );
	}
	//else if there is
	else if( uiArmsDealerTotalCost != 0 )
	{
		EnableButton( guiSKI_TransactionButton );
	}
	else
	{
		DisableButton( guiSKI_TransactionButton );
	}


	if( uiArmsDealerTotalCost > uiPlayersOfferAreaTotalCost + LaptopSaveInfo.iCurrentBalance )
	{
		DisableButton( guiSKI_TransactionButton );
	}
/* Allow transaction attempt when dealer can't afford to buy that much - he'll reject it with a special quote!
	else if( ( gArmsDealerStatus[ gbSelectedArmsDealerID ].uiArmsDealersCash + uiArmsDealerTotalCost ) < ( uiPlayersOfferAreaTotalCost - uiPlayersOfferAreaTotalMoney ) )
	{
		DisableButton( guiSKI_TransactionButton );
	}
*/
	else if( ( uiPlayersOfferAreaTotalCost == uiPlayersOfferAreaTotalMoney ) && ( uiPlayersOfferAreaTotalMoney > 0 ) && ( uiArmsDealerTotalCost == 0 ) )
	{
		DisableButton( guiSKI_TransactionButton );
	}


	//if the player is currently moving an item, disable the transaction button
	if( gMoveingItem.sItemIndex != 0 )
	{
//Evaluate:
//		DisableButton( guiSKI_EvaluateButton );
		DisableButton( guiSKI_TransactionButton );
		DisableButton( guiSKI_DoneButton );
	}
	else
	{
		EnableButton( guiSKI_DoneButton );
	}


//ARM: Always permit trying bribes, even if they don't work on a given dealer!
	// if the arms dealer is the kind of person who accepts gifts, and there is stuff to take
//	if( ArmsDealerInfo[ gbSelectedArmsDealerID ].uiFlags & ARMS_DEALER_ACCEPTS_GIFTS )
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
		ButtonList[ guiSKI_TransactionButton ]->uiFlags |= BUTTON_FORCE_UNDIRTY;
		ButtonList[ guiSKI_DoneButton ]->uiFlags |= BUTTON_FORCE_UNDIRTY;
	}
	else
	{
		ButtonList[ guiSKI_TransactionButton ]->uiFlags &= ~BUTTON_FORCE_UNDIRTY;
		ButtonList[ guiSKI_DoneButton ]->uiFlags &= ~BUTTON_FORCE_UNDIRTY;
	}


}


void AddItemToPlayersOfferAreaAfterShopKeeperOpen( OBJECTTYPE	*pItemObject, INT8 bPreviousInvPos )
{
	gItemToAdd.fActive						= TRUE;
	memcpy( &gItemToAdd.ItemObject, pItemObject, sizeof( OBJECTTYPE ) );
	gItemToAdd.bPreviousInvPos	= bPreviousInvPos;
}


BOOLEAN IsMoneyTheOnlyItemInThePlayersOfferArea( )
{
	UINT8	ubCnt;
	BOOLEAN fFoundMoney = FALSE;

	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if there is an item here
		if( PlayersOfferArea[ ubCnt ].fActive )
		{
			if( Item[ PlayersOfferArea[ ubCnt ].sItemIndex ].usItemClass != IC_MONEY )
				return( FALSE );
			else
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
		if( PlayersOfferArea[ uiCnt ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE )
		{
			AddItemToArmsDealerOfferArea( &PlayersOfferArea[ uiCnt ], -1 );
		}
	}
}

*/



UINT32 CalculateHowMuchMoneyIsInPlayersOfferArea( )
{
	UINT8	ubCnt;
	UINT32	uiTotalMoneyValue=0;


	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if there is an item here
		if( PlayersOfferArea[ ubCnt ].fActive )
		{
			if( Item[ PlayersOfferArea[ ubCnt ].sItemIndex ].usItemClass == IC_MONEY )
			{
				uiTotalMoneyValue += PlayersOfferArea[ ubCnt ].ItemObject.uiMoneyAmount;
			}
		}
	}

	return( uiTotalMoneyValue );
}



void MovePlayersItemsToBeRepairedToArmsDealersInventory()
{
	//for all items in the dealers items offer area
	UINT32	uiCnt;

	//loop through all the slots in the shopkeeper's offer area
	for( uiCnt=0; uiCnt<SKI_NUM_TRADING_INV_SLOTS; uiCnt++)
	{
		//if there is a item here
		if( ArmsDealerOfferArea[ uiCnt ].fActive )
		{
			// NOTE:  Any items that make it into a repairman's dealer offer area are guaranteed to be:
			//				a) Repairable
			//				b) Actually damaged
			//				c) Already stripped of all attachments
			//				d) If a gun, stripped of any non-ammo-class GunAmmoItems, and bullets

			// add it to the arms dealer's inventory
			GiveObjectToArmsDealerForRepair( gbSelectedArmsDealerID, &( ArmsDealerOfferArea[ uiCnt ].ItemObject ), ArmsDealerOfferArea[ uiCnt ].ubIdOfMercWhoOwnsTheItem );
			ArmsDealerOfferArea[ uiCnt ].sSpecialItemElement = gubLastSpecialItemAddedAtElement;

			//Remove the item from the owner merc's inventory
			IfMercOwnedRemoveItemFromMercInv( &( ArmsDealerOfferArea[ uiCnt ]) );

			//erase the item from the dealer's offer area
			ClearArmsDealerOfferSlot( uiCnt );
		}
	}
	gfResetShopKeepIdleQuote = TRUE;
}



BOOLEAN RemoveRepairItemFromDealersOfferArea( INT8	bSlot )
{
	//if the item doesn't have a duplicate copy in its owner merc's inventory slot
	if( ArmsDealerOfferArea[ bSlot ].bSlotIdInOtherLocation == -1 )
	{
		if ( !SKITryToReturnInvToOwnerOrCurrentMerc( &( ArmsDealerOfferArea[ bSlot ] ) ) )
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


INT8	GetInvSlotOfUnfullMoneyInMercInventory( SOLDIERTYPE *pSoldier )
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


void ClearArmsDealerOfferSlot( INT32 ubSlotToClear )
{
	// Clear the contents
	memset( &ArmsDealerOfferArea[ ubSlotToClear ], 0, sizeof( INVENTORY_IN_SLOT ) );

	//Remove the mouse help text from the region
	SetRegionFastHelpText( &gDealersOfferSlotsMouseRegions[ ubSlotToClear ], L"" );

	//if the dealer repairs
	if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
	{
		SetRegionFastHelpText( &gDealersOfferSlotsSmallFaceMouseRegions[ ubSlotToClear ], L"" );
	}
}


void ClearPlayersOfferSlot( INT32 ubSlotToClear )
{
	// Clear the contents
	memset( &PlayersOfferArea[ ubSlotToClear ], 0, sizeof( INVENTORY_IN_SLOT ) );

	//Clear the text for the item
	SetRegionFastHelpText( &gPlayersOfferSlotsMouseRegions[ ubSlotToClear ], L"" );
	SetRegionFastHelpText( &gPlayersOfferSlotsSmallFaceMouseRegions[ ubSlotToClear ], L"" );

	// if the player offer area is clear, reset flags for transaction
	CheckAndHandleClearingOfPlayerOfferArea( );
}



void EvaluateItemAddedToPlayersOfferArea( INT8 bSlotID, BOOLEAN fFirstOne )
{
	UINT32	uiEvalResult = EVAL_RESULT_NORMAL;
	BOOLEAN	fRocketRifleWasEvaluated = FALSE;
	UINT8		ubNumberOfItemsAddedToRepairDuringThisEvaluation=0;


	// there better be an item there
	Assert ( PlayersOfferArea[ bSlotID ].fActive );

	//if money is the item being evaluated, leave
	if( Item[ PlayersOfferArea[ bSlotID ].sItemIndex ].usItemClass == IC_MONEY )
		return;

	// if already evaluated, don't do it again
	if( PlayersOfferArea[ bSlotID ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED )
	{
		return;
	}


	// say "Hmm... Let's see" once per trading session to start evaluation
	// SPECIAL: Devin doesn't have this quote (he's the only one)
	if( !gfDealerHasSaidTheEvaluateQuoteOnceThisSession && ( gbSelectedArmsDealerID != ARMS_DEALER_DEVIN ) )
	{
		gfDealerHasSaidTheEvaluateQuoteOnceThisSession = TRUE;
		StartShopKeeperTalking( SK_QUOTES_PLAYER_REQUESTED_EVALUATION );
	}


	//Can this particular kind of item be sold/repaired here
	if( WillShopKeeperRejectObjectsFromPlayer( gbSelectedArmsDealerID, bSlotID ) == FALSE )
	{
		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			UINT32	uiNumberOfItemsInForRepairs = CountTotalItemsRepairDealerHasInForRepairs( gbSelectedArmsDealerID );
			UINT32	uiNumberOfItemsAlreadyEvaluated = CountNumberOfItemsInTheArmsDealersOfferArea();

			//Get the number of items being evaluated
			ubNumberOfItemsAddedToRepairDuringThisEvaluation = PlayersOfferArea[ bSlotID ].ItemObject.ubNumberOfObjects;

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
			if( ItemIsARocketRifle( PlayersOfferArea[ bSlotID ].sItemIndex ) )
			{
				fRocketRifleWasEvaluated = TRUE;
			}


			//if the item is damaged, or is a rocket rifle (which always "need repairing" even at 100%, to reset imprinting)
			if( ( PlayersOfferArea[ bSlotID ].ItemObject.bStatus[ 0 ] < 100 ) || fRocketRifleWasEvaluated )
			{
				INT8	bSlotAddedTo;

				// Move the item to the Dealer's Offer Area
				bSlotAddedTo = AddItemToArmsDealerOfferArea( &PlayersOfferArea[ bSlotID ], PlayersOfferArea[ bSlotID ].bSlotIdInOtherLocation );

				if( bSlotAddedTo != -1 )
				{
					// Clear the contents
					ClearPlayersOfferSlot( bSlotID );

/*	ARM: Leave it there, until transaction occurs it should be recallable
					//Remove the item from the owner merc's inventory
					IfMercOwnedRemoveItemFromMercInv( &(ArmsDealerOfferArea[ bSlotAddedTo ]) );
*/

					//Mark the item as unselected, signifying that it can be moved
					ArmsDealerOfferArea[ bSlotAddedTo ].uiFlags &= ~ARMS_INV_ITEM_SELECTED;

					//increment the number of items being added
					ubNumberOfItemsAddedToRepairDuringThisEvaluation++;

					// check if the item is really badly damaged
					if( Item[ ArmsDealerOfferArea[ bSlotAddedTo ].sItemIndex ].usItemClass != IC_AMMO )
					{
						if( ArmsDealerOfferArea[ bSlotAddedTo ].ItemObject.bStatus[ 0 ] < REALLY_BADLY_DAMAGED_THRESHOLD )
						{
							uiEvalResult = EVAL_RESULT_OK_BUT_REALLY_DAMAGED;
						}
					}

					// check if it's the first time a rocket rifle is being submitted to Fredo
					if( fRocketRifleWasEvaluated && ( gbSelectedArmsDealerID == ARMS_DEALER_FREDO ) )
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
					#ifdef JA2BETAVERSION
						ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Failed to add repair item to ArmsDealerOfferArea.  AM-0");
					#endif
					return;
				}
			}
			else
			{
				uiEvalResult = EVAL_RESULT_NOT_DAMAGED;
			}
		}
		else	// not a repairman
		{
			uiEvalResult = EvaluateInvSlot( & ( PlayersOfferArea[ bSlotID ] ) );
		}
	}
	else	// dealer doesn't handle this type of object
	{
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			// only otherwise repairable items count as actual rejections
			if ( Item[ PlayersOfferArea[ bSlotID ].sItemIndex ].fFlags & ITEM_REPAIRABLE )
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
	PlayersOfferArea[ bSlotID ].uiFlags |= ARMS_INV_PLAYERS_ITEM_HAS_BEEN_EVALUATED;


	// when evaluating complex items that get split into multiple subobjects, dealer will talk only about the first one!
	// don't bother with any of this if shopkeeper can't talk right now
	if ( fFirstOne && CanShopkeeperOverrideDialogue( ) )
	{
		INT16 sQuoteNum = -1;
		UINT8 ubChanceToSayNormalQuote;


		switch ( uiEvalResult )
		{
			case EVAL_RESULT_DONT_HANDLE:
				if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_SELLS_ONLY )
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
				#ifdef JA2BETAVERSION
					ScreenMsg( FONT_MCOLOR_WHITE, MSG_BETAVERSION, L"Invalid evaluation result of %d.  AM-0", uiEvalResult );
				#endif
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



BOOLEAN DoSkiMessageBox( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT8 ubFlags, MSGBOX_CALLBACK ReturnCallback )
{
	SGPRect pCenteringRect= {0, 0, 639, 339 };

	// reset exit mode
	gfExitSKIDueToMessageBox = TRUE;

	// do message box and return
	giSKIMessageBox = DoMessageBox( ubStyle, zString, uiExitScreen, ( UINT8 ) ( ubFlags| MSG_BOX_FLAG_USE_CENTERING_RECT ), ReturnCallback, &pCenteringRect );

	// send back return state
	return( ( giSKIMessageBox != -1 ) );
}

void ConfirmDontHaveEnoughForTheDealerMessageBoxCallBack( UINT8 bExitValue )
{
	// simply redraw the panel
	if( bExitValue == MSG_BOX_RETURN_OK )
	{
		gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
	}

	// done, re-enable calls to PerformTransaction()
	gfPerformTransactionInProgress = FALSE;
}



void ConfirmToDeductMoneyFromPlayersAccountMessageBoxCallBack( UINT8 bExitValue )
{
	// yes, deduct the money
	if( bExitValue == MSG_BOX_RETURN_YES )
	{
		UINT32	uiPlayersOfferAreaValue = CalculateTotalPlayersValue();
		UINT32	uiArmsDealersItemsCost = CalculateTotalArmsDealerCost();
		INT32		iMoneyToDeduct = (INT32)( uiArmsDealersItemsCost - uiPlayersOfferAreaValue );

		//Perform the transaction with the extra money from the players account
		PerformTransaction( iMoneyToDeduct );

		AddTransactionToPlayersBook( PURCHASED_ITEM_FROM_DEALER, ArmsDealerInfo[ gbSelectedArmsDealerID ].ubShopKeeperID, GetWorldTotalMin(), -iMoneyToDeduct );
	}

	// done, re-enable calls to PerformTransaction()
	gfPerformTransactionInProgress = FALSE;

	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
}



// run through what the player has on the table and see if the shop keep will aceept it or not
BOOLEAN WillShopKeeperRejectObjectsFromPlayer( INT8 bDealerId, INT8 bSlotId )
{
	BOOLEAN fRejected = TRUE;

	if( Item[ PlayersOfferArea[ bSlotId ].sItemIndex ].usItemClass == IC_MONEY )
	{
		fRejected = FALSE;
	}
	else if( CanDealerTransactItem( gbSelectedArmsDealerID, PlayersOfferArea[ bSlotId ].sItemIndex, TRUE ) )
	{
		fRejected = FALSE;
	}

	else
	{
		fRejected = TRUE;
	}

	return( fRejected );
}


void CheckAndHandleClearingOfPlayerOfferArea( void )
{
	INT32 iCounter = 0;
	BOOLEAN fActiveSlot = FALSE;

	// find out if all the player trade slots/offer area is empty
	for( iCounter=0; iCounter<SKI_NUM_TRADING_INV_SLOTS; iCounter++)
	{
		if( PlayersOfferArea[ iCounter ].fActive == TRUE )
		{
			// nope, there is an active slot
			fActiveSlot = TRUE;
		}
	}

	// if all player's offer area slots are empty
	if( fActiveSlot == FALSE )
	{
		ResetAllQuoteSaidFlags();
		gfRemindedPlayerToPickUpHisStuff = FALSE;
	}
}


BOOLEAN CanShopkeeperOverrideDialogue( void )
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


void CrossOutUnwantedItems( void )
{
	HVOBJECT hHandle;
	INT8 bSlotId = 0;
	INT16 sBoxStartX = 0, sBoxStartY = 0;
	INT16 sBoxWidth = 0, sBoxHeight = 0;


	// load the "cross out" graphic
	GetVideoObject( &hHandle, guiItemCrossOut );

	// get the box height and width
	sBoxWidth = SKI_INV_SLOT_WIDTH;
	sBoxHeight = SKI_INV_SLOT_HEIGHT;

	for( bSlotId = 0; bSlotId < SKI_NUM_TRADING_INV_SLOTS; bSlotId++ )
	{
		// now run through what's on the players offer area
		if( PlayersOfferArea[ bSlotId ].fActive )
		{
			// skip purchased items!
			if ( !( PlayersOfferArea[ bSlotId ].uiFlags & ARMS_INV_JUST_PURCHASED ) )
			{
				//If item can't be sold here, or it's completely worthless (very cheap / very broken)
				if( ( WillShopKeeperRejectObjectsFromPlayer( gbSelectedArmsDealerID, bSlotId ) == TRUE ) ||
						!( PlayersOfferArea[ bSlotId ].uiFlags & ARMS_INV_PLAYERS_ITEM_HAS_VALUE ) )
				{
					// get x and y positions
					sBoxStartX = SKI_PLAYERS_TRADING_INV_X + ( bSlotId % SKI_NUM_TRADING_INV_COLS ) * ( SKI_INV_OFFSET_X );
					sBoxStartY = SKI_PLAYERS_TRADING_INV_Y + ( bSlotId / SKI_NUM_TRADING_INV_COLS ) * ( SKI_INV_OFFSET_Y );

					BltVideoObject(FRAME_BUFFER, hHandle, 0,( sBoxStartX + 22 ), ( sBoxStartY ));

					// invalidate the region
					InvalidateRegion(sBoxStartX - 1, sBoxStartY - 1, sBoxStartX + sBoxWidth + 1, sBoxStartY + sBoxHeight + 1 );
				}
			}
		}
	}
}


INT16 GetNumberOfItemsInPlayerOfferArea( void )
{
	INT16 sCounter = 0;
	INT32 iCounter = 0;


	// find number of active slot in player offer area
	for( iCounter = 0; iCounter < SKI_NUM_TRADING_INV_SLOTS; iCounter++ )
	{
		if( PlayersOfferArea[ iCounter ].fActive )
		{
			sCounter++;
		}
	}

	return( sCounter );
}

void HandleCheckIfEnoughOnTheTable( void )
{
	static INT32 iLastTime = 0;
	INT32 iDifference = 0, iRand = 0;
	UINT32	uiPlayersOfferAreaValue = CalculateTotalPlayersValue();
	UINT32	uiArmsDealersItemsCost = CalculateTotalArmsDealerCost();

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



void InitShopKeeperItemDescBox( OBJECTTYPE *pObject, UINT8 ubPocket, UINT8 ubFromLocation )
{
	INT16	sPosX, sPosY;


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
			if( ( sPosX + 358 ) > 640 )
				sPosX = sPosX - ( ( sPosX + 358 ) - 640 ) - 5;

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
			if( ( sPosX + 358 ) > 640 )
				sPosX = sPosX - ( ( sPosX + 358 ) - 640 ) - 5;

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
			Assert( 0 );
			return;
	}


	pShopKeeperItemDescObject = pObject;

	InitItemDescriptionBox( gpSMCurrentMerc, 255, sPosX, sPosY, 0 );

	StartSKIDescriptionBox( );
}


void StartSKIDescriptionBox( void )
{
	INT32 iCnt;


	//shadow the entire screen
//	ShadowVideoSurfaceRect( FRAME_BUFFER, 0, 0, 640, 480 );

	//if the current merc is too far away, dont shade the SM panel because it is already shaded
	if( gfSMDisableForItems )
		DrawHatchOnInventory( FRAME_BUFFER, 0, 0, 640, 338 );
	else
		DrawHatchOnInventory( FRAME_BUFFER, 0, 0, 640, 480 );

	InvalidateRegion( 0, 0, 640, 480 );

	// disable almost everything!

	gfSMDisableForItems = TRUE;
	DisableInvRegions( gfSMDisableForItems );

	DisableButton( guiSKI_InvPageUpButton );
	DisableButton( guiSKI_InvPageDownButton );
//	DisableButton( guiSKI_EvaluateButton );
	DisableButton( guiSKI_TransactionButton );
	DisableButton( guiSKI_DoneButton );
/*
	for (iCnt = 0; iCnt < NUM_SKI_ATM_BUTTONS; iCnt++)
		DisableButton( guiSKI_AtmButton[ iCnt ] );
*/

	DisableAllDealersInventorySlots();
	DisableAllDealersOfferSlots();

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		MSYS_DisableRegion( &gPlayersOfferSlotsMouseRegions[ iCnt ] );
		MSYS_DisableRegion( &gPlayersOfferSlotsSmallFaceMouseRegions[ iCnt ] );
	}

	if( gShopKeeperSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS )
	{
		MSYS_DisableRegion( &gShopKeeperSubTitleMouseRegion );
	}

//	MSYS_DisableRegion( &gShopKeeperCoverTacticalButtonMouseRegion );

//	if( giItemDescAmmoButton >= 0 && ButtonList[ giItemDescAmmoButton ].
//	DisableButton( giItemDescAmmoButton );


	RenderItemDescriptionBox( );
}


void DeleteShopKeeperItemDescBox()
{
	INT32 iCnt;


	pShopKeeperItemDescObject = NULL;
	gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;

	//Redraw the face
	gFacesData[ giShopKeeperFaceIndex ].uiFlags |= FACE_REDRAW_WHOLE_FACE_NEXT_FRAME;


	// enable almost everything!

	CheckForDisabledForGiveItem( );
	DisableInvRegions( gfSMDisableForItems );	//actually does an enable if current merc is in range

	EnableButton( guiSKI_InvPageUpButton );
	EnableButton( guiSKI_InvPageDownButton );
	EnableButton( guiSKI_TransactionButton );
	EnableButton( guiSKI_DoneButton );

	EnableDisableDealersInventoryPageButtons();
	EnableDisableEvaluateAndTransactionButtons();

//	EnableButton( guiSKI_EvaluateButton );

/*
	for (iCnt = 0; iCnt < NUM_SKI_ATM_BUTTONS; iCnt++)
		EnableButton( guiSKI_AtmButton[ iCnt ] );
*/

	EnableAllDealersInventorySlots();
	EnableAllDealersOfferSlots();

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		MSYS_EnableRegion( &gPlayersOfferSlotsMouseRegions[ iCnt ] );
		MSYS_EnableRegion( &gPlayersOfferSlotsSmallFaceMouseRegions[ iCnt ] );
	}

	if( gShopKeeperSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS )
	{
		MSYS_EnableRegion( &gShopKeeperSubTitleMouseRegion );
	}

//	MSYS_EnableRegion( &gShopKeeperCoverTacticalButtonMouseRegion );

//	if( giItemDescAmmoButton >= 0 && ButtonList[ giItemDescAmmoButton ].
//	EnableButton( giItemDescAmmoButton );

}



BOOLEAN OfferObjectToDealer( OBJECTTYPE *pComplexObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId )
{
	UINT8		ubTotalSubObjects;
	UINT8		ubRepairableSubObjects;
	UINT8		ubNonRepairableSubObjects;
	UINT8		ubDealerOfferAreaSlotsNeeded;
	UINT8		ubPlayerOfferAreaSlotsNeeded;
	UINT8		ubDiff;
	UINT8		ubHowManyMoreItemsCanDealerTake;
	UINT8		ubSubObject;
	BOOLEAN	fFirstOne = TRUE;
	BOOLEAN fSuccess = FALSE;


	if ( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer != ARMS_DEALER_REPAIRS )
	{
		// if not actually doing repairs, there's no need to split objects up at all
		if ( !AddObjectForEvaluation( pComplexObject, ubOwnerProfileId, bOwnerSlotId, TRUE ) )
		{
			gfDisplayNoRoomMsg = TRUE;
			return(FALSE);	// no room
		}
	}
	else	// dealing with a repairman
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
			DoSkiMessageBox( MSG_BOX_BASIC_STYLE, L"There is not enough room in the dealers repair area.", SHOPKEEPER_SCREEN, MSG_BOX_FLAG_OK, NULL );
			return(FALSE);	// no room
		}
*/

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
				// if it's the main item itself (always in the very first subobject), and it has no other subobjects
				if ( ( ubSubObject == 0 ) && ( ubTotalSubObjects == 1) )
				{
					// store its owner merc as the owner, and store the correct slot
					fSuccess = AddObjectForEvaluation( &gSubObject[ ubSubObject ], ubOwnerProfileId, bOwnerSlotId, fFirstOne );
				}
				else	// attachments, bullets/payload
				{
					// store it with a valid owner, but an invalid slot, so it still shows who owns it, but can't return to its slot
// ARM: New code will be needed here if we add parent/child item support & interface
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

	return(TRUE);		// it worked
}



void SplitComplexObjectIntoSubObjects( OBJECTTYPE *pComplexObject )
{
	OBJECTTYPE *pNextObj = &gSubObject[ 0 ];
	UINT8 ubNextFreeSlot = 0;
	UINT8 ubCnt;


	Assert( pComplexObject );
	Assert( pComplexObject->ubNumberOfObjects > 0 );
	Assert( pComplexObject->ubNumberOfObjects <= MAX_OBJECTS_PER_SLOT );


	// clear subobject array
	memset (gSubObject, 0, sizeof( OBJECTTYPE ) * MAX_SUBOBJECTS_PER_OBJECT );


	// if it isn't stacked
	if ( pComplexObject->ubNumberOfObjects == 1 )
	{
		// make the main item into the very first subobject
		CopyObj( pComplexObject, pNextObj );

		// strip off any loaded ammo/payload
		if ( Item [ pComplexObject->usItem ].usItemClass == IC_GUN )
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
			}
*/
		}

/* ARM: Can now repair with removeable attachments still attached...
		// strip off any seperable attachments
		for( ubCnt = 0; ubCnt < MAX_ATTACHMENTS; ubCnt++ )
		{
			if ( pComplexObject->usAttachItem[ ubCnt ] != NONE )
			{
				// If the attachment is detachable
				if (! (Item[ pComplexObject->usAttachItem[ ubCnt ] ].fFlags & ITEM_INSEPARABLE ) )
				{
					pNextObj->usAttachItem[ ubCnt ] = NONE;
					pNextObj->bAttachStatus[ ubCnt ] = 0;
				}
			}
		}
*/

		// advance to next available subobject
		pNextObj = &gSubObject[ ++ubNextFreeSlot ];


		// if it's a gun
		if ( Item [ pComplexObject->usItem ].usItemClass == IC_GUN )
		{
			// and it has ammo/payload
			if ( pComplexObject->usGunAmmoItem != NONE )
			{
				// if it's bullets
				if ( Item[ pComplexObject->usGunAmmoItem ].usItemClass == IC_AMMO )
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
				else	// non-ammo payload
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
				if (! (Item[ pComplexObject->usAttachItem[ ubCnt ] ].fFlags & ITEM_INSEPARABLE ) )
				{
					CreateItem( pComplexObject->usAttachItem[ ubCnt ], pComplexObject->bAttachStatus[ ubCnt ], pNextObj );

					pNextObj = &gSubObject[ ++ubNextFreeSlot ];
				}
			}
		}
*/
	}
	else	// stacked
	{
		// these can't be guns, can't have any attachments, can't be imprinted, etc.
		Assert ( Item [ pComplexObject->usItem ].usItemClass != IC_GUN );

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


void CountSubObjectsInObject( OBJECTTYPE *pComplexObject, UINT8 *pubTotalSubObjects, UINT8 *pubRepairableSubObjects, UINT8 *pubNonRepairableSubObjects )
{
	UINT8 ubSubObject;

	*pubTotalSubObjects					= 0;
	*pubRepairableSubObjects		= 0;
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


BOOLEAN AddObjectForEvaluation(OBJECTTYPE *pObject, UINT8 ubOwnerProfileId, INT8 bOwnerSlotId, BOOLEAN fFirstOne )
{
	INVENTORY_IN_SLOT InvSlot;
	INT8	bAddedToSlotID;

	// Make a new inv slot out of the subobject
	memset( &InvSlot, 0, sizeof( INVENTORY_IN_SLOT ) );
	memcpy( &InvSlot.ItemObject, pObject, sizeof( OBJECTTYPE ) );

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
BOOLEAN ShopkeeperAutoPlaceObject( SOLDIERTYPE * pSoldier, OBJECTTYPE * pObject, BOOLEAN fNewItem )
{
	OBJECTTYPE CopyOfObject;
	UINT8 ubObjectsLeftToPlace;

	// the entire pObj will get memset to 0 by RemoveObjs() if all the items are successfully placed,
	// so we have to keep a copy to retrieve with every iteration of the loop
	memcpy( &CopyOfObject, pObject, sizeof( OBJECTTYPE ) );


	ubObjectsLeftToPlace = pObject->ubNumberOfObjects;

	while ( ubObjectsLeftToPlace > 0 )
	{
		// figure out how many to place during this loop iteration.  Can't do more than MAX_OBJECTS_PER_SLOT at a time
		pObject->ubNumberOfObjects = min( MAX_OBJECTS_PER_SLOT, ubObjectsLeftToPlace);
		ubObjectsLeftToPlace -= pObject->ubNumberOfObjects;

		if (!AutoPlaceObject( pSoldier, pObject, fNewItem ))
		{
			// no more room, didn't all fit - add back in any that we didn't even get to yet
			pObject->ubNumberOfObjects += ubObjectsLeftToPlace;
			return( FALSE );
		}

		// restore object properties from our backup copy
		memcpy( pObject, &CopyOfObject, sizeof( OBJECTTYPE ) );
	}

	return( TRUE );
}



// The Shopkeeper interface *MUST* use this intermediary function instead of calling AddItemToPool() directly!
// This is because the OBJECTTYPEs used within Shopkeeper may contain an illegal ubNumberOfObjects
void ShopkeeperAddItemToPool( INT16 sGridNo, OBJECTTYPE *pObject, INT8 bVisible, UINT8 ubLevel, UINT16 usFlags, INT8 bRenderZHeightAboveLevel )
{
	OBJECTTYPE CopyOfObject;
	UINT8 ubObjectsLeftToPlace;

	// the entire pObj will get memset to 0 by RemoveObjs() if all the items are successfully placed,
	// so we have to keep a copy to retrieve with every iteration of the loop
	memcpy( &CopyOfObject, pObject, sizeof( OBJECTTYPE ) );

	ubObjectsLeftToPlace = pObject->ubNumberOfObjects;

	while ( ubObjectsLeftToPlace > 0 )
	{
		// figure out how many to place during this loop iteration.  Can't do more than MAX_OBJECTS_PER_SLOT at a time
		pObject->ubNumberOfObjects = min( MAX_OBJECTS_PER_SLOT, ubObjectsLeftToPlace);
		ubObjectsLeftToPlace -= pObject->ubNumberOfObjects;

		AddItemToPool( sGridNo, pObject, bVisible, ubLevel, usFlags, bRenderZHeightAboveLevel );

		// restore object properties from our backup copy
		memcpy( pObject, &CopyOfObject, sizeof( OBJECTTYPE ) );
	}
}



void IfMercOwnedCopyItemToMercInv( INVENTORY_IN_SLOT *pInv )
{
	INT16 sSoldierID;

	//if the item picked up was in a previous location, and that location is on a merc's inventory
	if ( ( pInv->bSlotIdInOtherLocation != -1 ) && ( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE ) )
	{
		// then it better be a valid slot #
		Assert( pInv->bSlotIdInOtherLocation < NUM_INV_SLOTS );
		// and it better have a valid merc who owned it
		Assert( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE );

		// get soldier
		sSoldierID = GetSoldierIDFromMercID( pInv->ubIdOfMercWhoOwnsTheItem );
		Assert( sSoldierID != -1 );
		Assert( CanMercInteractWithSelectedShopkeeper( MercPtrs[ sSoldierID ] ) );

		//Copy the object back into that merc's original inventory slot
		CopyObj( &( pInv->ItemObject ), &( Menptr[ sSoldierID ].inv[ pInv->bSlotIdInOtherLocation ] ) );
	}
}


void IfMercOwnedRemoveItemFromMercInv( INVENTORY_IN_SLOT *pInv )
{
	IfMercOwnedRemoveItemFromMercInv2( pInv->ubIdOfMercWhoOwnsTheItem, pInv->bSlotIdInOtherLocation );
}


void IfMercOwnedRemoveItemFromMercInv2( UINT8 ubOwnerProfileId, INT8 bOwnerSlotId )
{
	INT16 sSoldierID;
	BOOLEAN fSuccess;
	OBJECTTYPE ObjectToRemove;

	//if this item was in a previous location, and that location is on a merc's inventory
	if ( ( bOwnerSlotId != -1 ) && ( ubOwnerProfileId != NO_PROFILE ) )
	{
		// then it better be a valid slot #
		Assert( bOwnerSlotId < NUM_INV_SLOTS );
		// and it better have a valid merc who owned it
		Assert( ubOwnerProfileId != NO_PROFILE );

		sSoldierID = GetSoldierIDFromMercID( ubOwnerProfileId );
		Assert( sSoldierID != -1 );
		Assert( CanMercInteractWithSelectedShopkeeper( MercPtrs[ sSoldierID ] ) );

		//remove the object from that merc's original inventory slot
		fSuccess = RemoveObjectFromSlot( &Menptr[ sSoldierID ], bOwnerSlotId, &ObjectToRemove );
		Assert(fSuccess);
	}
}



BOOLEAN SKITryToReturnInvToOwnerOrCurrentMerc( INVENTORY_IN_SLOT *pInv )
{
	// don't use this if the item has a copy in merc's inventory!!  It would create a duplicate!!
	Assert( pInv->bSlotIdInOtherLocation == -1 );

	// if it does have an owner
	if( pInv->ubIdOfMercWhoOwnsTheItem != NO_PROFILE )
	{
		INT16 sSoldierID;

		sSoldierID = GetSoldierIDFromMercID( pInv->ubIdOfMercWhoOwnsTheItem );
		// if that soldier is not in player's hire any longer
		if ( sSoldierID == -1 )
		{
			return(FALSE);
		}

		// For owners of repaired items, this checks that owner is still hired, in sector,
		// on current squad, close enough to the shopkeeper, etc.
		if ( !CanMercInteractWithSelectedShopkeeper( MercPtrs[ sSoldierID ] ) )
		{
			return(FALSE);
		}

		// Try to find a place to put in its owner's inventory (regardless of which merc is currently displayed!)
		if ( SKITryToAddInvToMercsInventory( pInv, MercPtrs[ sSoldierID ] ) )
		{
			return( TRUE );
		}

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


BOOLEAN SKITryToAddInvToMercsInventory( INVENTORY_IN_SLOT *pInv, SOLDIERTYPE *pSoldier )
{
	INT8	bMoneyInvPos;
	BOOLEAN fNewItem = FALSE;


	//if the item is money
	if( Item[ pInv->sItemIndex ].usItemClass == IC_MONEY )
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



BOOLEAN CanMercInteractWithSelectedShopkeeper( SOLDIERTYPE *pSoldier )
{
	SOLDIERTYPE *pShopkeeper;
	INT16			sDestGridNo;
	INT8			bDestLevel;
	INT16			sDistVisible;
	UINT32		uiRange;


	Assert( pSoldier!= NULL );
	Assert( gbSelectedArmsDealerID != -1 );

	pShopkeeper = FindSoldierByProfileID( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubShopKeeperID, FALSE );
	Assert( pShopkeeper != NULL );
	Assert( pShopkeeper->bActive );
	Assert( pShopkeeper->bInSector );

	if ( pShopkeeper->bLife < OKLIFE )
	{
		return( FALSE );
	}

	if ( pSoldier->bActive && pSoldier->bInSector && IsMercOnCurrentSquad( pSoldier ) && ( pSoldier->bLife >= OKLIFE ) &&
		!( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && !AM_A_ROBOT( pSoldier ) )
	{
		sDestGridNo = pShopkeeper->sGridNo;
		bDestLevel	= pShopkeeper->bLevel;

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



#ifdef JA2TESTVERSION

void AddShopkeeperToGridNo( UINT8 ubProfile, INT16 sGridNo )
{
	SOLDIERCREATE_STRUCT		MercCreateStruct;
	INT16										sSectorX, sSectorY;
	UINT8									ubID;

	GetCurrentWorldSector( &sSectorX, &sSectorY );

	memset( &MercCreateStruct, 0, sizeof( MercCreateStruct ) );
	MercCreateStruct.bTeam				= CIV_TEAM;
	MercCreateStruct.ubProfile		= ubProfile;
	MercCreateStruct.sSectorX			= sSectorX;
	MercCreateStruct.sSectorY			= sSectorY;
	MercCreateStruct.bSectorZ			= gbWorldSectorZ;
	MercCreateStruct.sInsertionGridNo		= sGridNo;

	if ( TacticalCreateSoldier( &MercCreateStruct, &ubID ) )
	{
		AddSoldierToSector( ubID );

		// So we can see them!
		AllTeamsLookForAll(NO_INTERRUPTS);
	}
}

#endif


void ExitSKIRequested()
{
	BOOLEAN	fPlayerOwnedStuffOnTable = FALSE;


	ShutUpShopKeeper();

	if( !gfRemindedPlayerToPickUpHisStuff )
	{
		if( AreThereItemsInThePlayersOfferArea( ) )
		{
			fPlayerOwnedStuffOnTable = TRUE;
		}

		if( ( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS ) &&
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



void ResetAllQuoteSaidFlags()
{
	UINT8 uiCnt;

	// reset flags for quotes said
	for ( uiCnt = 0; uiCnt < NUM_EVAL_RESULTS; uiCnt++ )
	{
		gfEvalResultQuoteSaid[ uiCnt ] = FALSE;
	}

	guiLastTimeDealerSaidNormalEvaluationQuote = 0;
}



void DealWithItemsStillOnTheTable()
{
	UINT8	ubCnt;
	SOLDIERTYPE *pDropSoldier;


	// in case we have have to drop stuff off at someone's feet, figure out where it's all gonna go

	// use the current merc, unless he's ineligible, then use the selected merc instead.
	if( !gfSMDisableForItems )
	{
		pDropSoldier = gpSMCurrentMerc;
	}
	else
	{
		pDropSoldier = MercPtrs[ gusSelectedSoldier ];
	}

	// this guy HAS to be valid!
	Assert( CanMercInteractWithSelectedShopkeeper( pDropSoldier ) );


	//loop through the players offer area and add return any items there to the player
	for( ubCnt=0; ubCnt<SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
	{
		//if there is an item here, give it back somehow
		if( PlayersOfferArea[ ubCnt ].fActive )
		{
			ReturnItemToPlayerSomehow( &( PlayersOfferArea[ ubCnt ] ), pDropSoldier );
			ClearPlayersOfferSlot( ubCnt );
		}
	}


	//if the dealer repairs
	if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
	{
		//loop through the arms dealers' offer area and return any items there to the player
		for( ubCnt = 0; ubCnt < SKI_NUM_TRADING_INV_SLOTS; ubCnt++)
		{
		//if there is an item here, give it back somehow
			if( ArmsDealerOfferArea[ ubCnt ].fActive )
			{
				ReturnItemToPlayerSomehow( &( ArmsDealerOfferArea[ ubCnt ] ), pDropSoldier );
				ClearArmsDealerOfferSlot( ubCnt );
			}
		}
	}
}



void ReturnItemToPlayerSomehow( INVENTORY_IN_SLOT *pInvSlot, SOLDIERTYPE *pDropSoldier )
{
	//if the item doesn't have a duplicate copy in its owner merc's inventory slot
	if( pInvSlot->bSlotIdInOtherLocation == -1 )
	{
		// first try to give it to its owner, or if he's unavailable, the current merc
		if ( !SKITryToReturnInvToOwnerOrCurrentMerc( pInvSlot ) )
		{
			// failed to add item, inventory probably filled up or item is unowned and current merc ineligible.
			// drop it at the specified guy's feet instead
			ShopkeeperAddItemToPool( pDropSoldier->sGridNo, &pInvSlot->ItemObject, VISIBLE, pDropSoldier->bLevel, 0, 0 );
		}
	}
}



void GivePlayerSomeChange( UINT32 uiAmount )
{
	INVENTORY_IN_SLOT	MoneyInvSlot;

	memset( &MoneyInvSlot, 0, sizeof ( MoneyInvSlot ) );

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



void DealerGetsBribed( UINT8 ubProfileId, UINT32 uiMoneyAmount )
{
	BOOLEAN fBribable = FALSE;
	UINT32 uiMinBribe = 0;
	UINT16 usFact = 0;

	// this is only for dealers
	if (!IsMercADealer( ubProfileId ) )
	{
		return;
	}

	switch ( ubProfileId )
	{
		case FRANK:
			fBribable = TRUE;
			uiMinBribe = 10;		// This should match the handling of a bribe submitted outside the SKI, using the GIVE system
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



void HandlePossibleRepairDelays()
{
	UINT32 uiHoursSinceAnyItemsShouldHaveBeenRepaired = 0;


	// assume there won't be a delay
	gfStartWithRepairsDelayedQuote = FALSE;

	// if it's Fredo or Perko
	if ( ( gbSelectedArmsDealerID == ARMS_DEALER_FREDO ) || ( gbSelectedArmsDealerID == ARMS_DEALER_PERKO ) )
	{
		// because the quotes are so specific, we'll only use them once per game per repairman
		if( !gArmsDealerStatus[ gbSelectedArmsDealerID ].fRepairDelayBeenUsed )
		{
			// and it's been a while since the player last dealt with this repairman (within SKI that is)
			// this serves 2 purposes:
			// a) reduces delays being much more likely if player checks time remaining very frequently, AND
			// b) gives time for the events described in the text of the dealers' excuses to happen (e.g. scouting trip)
			if ( ( GetWorldTotalMin() - gArmsDealerStatus[ gbSelectedArmsDealerID ].uiTimePlayerLastInSKI ) >= ( 3 * 60 ) )
			{
				// if he should have been finished, but it's only been a few hours since then (not days!)
				if ( RepairmanFixingAnyItemsThatShouldBeDoneNow( &uiHoursSinceAnyItemsShouldHaveBeenRepaired ) &&
						 ( uiHoursSinceAnyItemsShouldHaveBeenRepaired < REPAIR_DELAY_IN_HOURS ) )
				{
					// then there's a fair chance he'll be delayed.  Use pre-chance to hopefully preserve across reloads
					if ( PreChance( 50 ) )
					{
						DelayRepairsInProgressBy( ( REPAIR_DELAY_IN_HOURS + Random( REPAIR_DELAY_IN_HOURS ) ) * 60 );

						// this triggers a different opening quote
						gfStartWithRepairsDelayedQuote = TRUE;

						// set flag so it doesn't happen again
						gArmsDealerStatus[ gbSelectedArmsDealerID ].fRepairDelayBeenUsed = TRUE;
					}
				}
			}
		}
	}
}



BOOLEAN RepairmanFixingAnyItemsThatShouldBeDoneNow( UINT32 *puiHoursSinceOldestItemRepaired )
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
			for( ubElement=0; ubElement< pDealerItem->ubElementsAlloced; ubElement++ )
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


void DelayRepairsInProgressBy( UINT32 uiMinutesDelayed )
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
			for( ubElement=0; ubElement< pDealerItem->ubElementsAlloced; ubElement++ )
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
void SelectArmsDealersDropItemToGroundRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		SOLDIERTYPE *pDropSoldier;

		// use the current merc, unless he's ineligible, then use the selected merc instead.
		if( !gfSMDisableForItems )
		{
			pDropSoldier = gpSMCurrentMerc;
		}
		else
		{
			pDropSoldier = MercPtrs[ gusSelectedSoldier ];
		}

		//if we don't have an item, pick one up
		if( gMoveingItem.sItemIndex != 0 )
		{
			//add the item to the ground
			ShopkeeperAddItemToPool( pDropSoldier->sGridNo, &gMoveingItem.ItemObject, VISIBLE, pDropSoldier->bLevel, 0, 0 );

			//Reset the cursor
			SetSkiCursor( CURSOR_NORMAL );

			//refresh the screen
			gubSkiDirtyLevel = SKI_DIRTY_LEVEL2;
		}
	}
}

void SelectArmsDealersDropItemToGroundMovementRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
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
	else if( iReason & MSYS_CALLBACK_REASON_MOVE )
	{
	}
}


BOOLEAN	CanTheDropItemToGroundStringBeDisplayed()
{
	//if we don't have an item, pick one up
	if( gMoveingItem.sItemIndex != 0 )
		return( TRUE );
	else
		return( FALSE );
}

void DisplayTheSkiDropItemToGroundString()
{
	UINT16	usHeight;

	//get the height of the displayed text
	usHeight = DisplayWrappedString( SKI_DROP_ITEM_TO_GROUND_START_X, SKI_DROP_ITEM_TO_GROUND_TEXT_START_Y, (UINT16)(640-SKI_DROP_ITEM_TO_GROUND_START_X), 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[ SKI_TEXT_DROP_ITEM_TO_GROUND ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT );

	//display the 'drop item to ground' text
	DisplayWrappedString( SKI_DROP_ITEM_TO_GROUND_START_X, (UINT16)(SKI_DROP_ITEM_TO_GROUND_TEXT_START_Y-usHeight), (UINT16)(640-SKI_DROP_ITEM_TO_GROUND_START_X), 2, SKI_LABEL_FONT, SKI_TITLE_COLOR, SKI_Text[ SKI_TEXT_DROP_ITEM_TO_GROUND ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED | INVALIDATE_TEXT );
}



UINT32 EvaluateInvSlot( INVENTORY_IN_SLOT *pInvSlot )
{
	UINT32	uiEvalResult = EVAL_RESULT_NORMAL;
	FLOAT		dPriceModifier;
	UINT32	uiBuyingPrice;


	//if the dealer is Micky
	if( gbSelectedArmsDealerID == ARMS_DEALER_MICKY )
	{
		INT16	sSoldierID;
		sSoldierID = GetSoldierIDFromMercID( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubShopKeeperID );
		if( ( sSoldierID != -1 ) && ( GetDrunkLevel( &Menptr[ sSoldierID ] ) == DRUNK ) )
		{
			//Micky is DRUNK, pays more!
			dPriceModifier = ArmsDealerInfo[ gbSelectedArmsDealerID ].dSellModifier;
		}
		else
		{
			// Micky isn't drunk, charge regular price
			dPriceModifier = ArmsDealerInfo[ gbSelectedArmsDealerID ].dBuyModifier;
		}
	}
	else
	{
		dPriceModifier = ArmsDealerInfo[ gbSelectedArmsDealerID ].dBuyModifier;
	}


	// Calculate dealer's buying price for the item
	uiBuyingPrice = CalcShopKeeperItemPrice( DEALER_BUYING, FALSE, pInvSlot->sItemIndex, dPriceModifier, &( pInvSlot->ItemObject ) );

	pInvSlot->uiItemPrice = uiBuyingPrice;

	if ( uiBuyingPrice > 0 )
	{
		// check if the item is really badly damaged
		if( Item[ pInvSlot->sItemIndex ].usItemClass != IC_AMMO )
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
#define REPAIR_MINUTES_INTERVAL	15


void BuildRepairTimeString( wchar_t sString[], size_t Length, UINT32 uiTimeInMinutesToFixItem )
{
	UINT16	usNumberOfHoursToFixItem = 0;


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
		swprintf( sString, Length, SKI_Text[ SKI_TEXT_MINUTES ], uiTimeInMinutesToFixItem );
	}
	else
	{
		// show hours

		// round fractions of 15+ minutes up to next full hour
		usNumberOfHoursToFixItem = (UINT16) ( ( uiTimeInMinutesToFixItem + 45 ) / 60 );

		if ( usNumberOfHoursToFixItem > 1 )
		{
			swprintf( sString, Length, SKI_Text[ SKI_TEXT_PLURAL_HOURS ], usNumberOfHoursToFixItem );
		}
		else
		{
			wcscpy( sString, SKI_Text[ SKI_TEXT_ONE_HOUR ] );
		}
	}
}



void BuildDoneWhenTimeString( wchar_t sString[], size_t Length, UINT8 ubArmsDealer, UINT16 usItemIndex, UINT8 ubElement )
{
	UINT32 uiDoneTime;
	UINT32 uiDay, uiHour, uiMin;


	//dealer must be a repair dealer
	Assert( DoesDealerDoRepairs( ubArmsDealer ) );
	// element index must be valid
	Assert( ubElement < gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].ubElementsAlloced );
	// that item must be active
	Assert( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].fActive );
	// that item must be in repair
	Assert( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].Info.bItemCondition < 0 );

	//if the item has already been repaired
	if( gArmsDealersInventory[ ubArmsDealer ][ usItemIndex ].SpecialItem[ ubElement ].uiRepairDoneTime <= GetWorldTotalMin() )
	{
		wcscpy( sString, L"" );
		return;
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
	uiMin	 = uiDoneTime - ( ( uiDay * NUM_MIN_IN_DAY ) + ( uiHour * NUM_MIN_IN_HOUR ) );

	// only show day if it's gonna take overnight
	if ( GetWorldDay() != uiDay )
	{
		swprintf( sString, Length, L"%S %d %02d:%02d", pDayStrings[ 0 ], uiDay, uiHour, uiMin );
	}
	else
	{
		swprintf( sString, Length, L"%02d:%02d", uiHour, uiMin );
	}
}


void BuildItemHelpTextString( wchar_t sString[], size_t Length, INVENTORY_IN_SLOT *pInv, UINT8 ubScreenArea )
{
	CHAR16 zHelpText[ 512 ];
	CHAR16 zRepairTime[ 64 ];

	if( pInv != NULL )
	{
		GetHelpTextForItem( zHelpText, lengthof(zHelpText), &( pInv->ItemObject ), NULL );

		// add repair time for items in a repairman's offer area
		if ( ( ubScreenArea == ARMS_DEALER_OFFER_AREA ) &&
				 ( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS ) )
		{
			BuildRepairTimeString( zRepairTime, lengthof(zRepairTime), CalculateObjectItemRepairTime( gbSelectedArmsDealerID, &( pInv->ItemObject ) ) );
			swprintf( sString, Length, L"%S\n(%S: %S)", zHelpText, gzLateLocalizedString[ 44 ], zRepairTime );
		}
		else
		{
			wcscpy( sString, zHelpText );
		}
	}
	else
	{
		sString[ 0 ] = '\0';
	}
}



void DisableAllDealersInventorySlots( void )
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_ARMS_DEALERS_INV_SLOTS; iCnt++)
	{
		MSYS_DisableRegion( &gDealersInventoryMouseRegions[ iCnt ] );

		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			MSYS_DisableRegion( &gRepairmanInventorySmallFaceMouseRegions[ iCnt ] );
		}
	}
}


void EnableAllDealersInventorySlots( void )
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_ARMS_DEALERS_INV_SLOTS; iCnt++)
	{
		MSYS_EnableRegion( &gDealersInventoryMouseRegions[ iCnt ] );

		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			MSYS_EnableRegion( &gRepairmanInventorySmallFaceMouseRegions[ iCnt ] );
		}
	}
}



void DisableAllDealersOfferSlots( void )
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		MSYS_DisableRegion( &gDealersOfferSlotsMouseRegions[ iCnt ] );

		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			MSYS_DisableRegion( &gDealersOfferSlotsSmallFaceMouseRegions[ iCnt ] );
		}
	}
}


void EnableAllDealersOfferSlots( void )
{
	INT32 iCnt;

	for (iCnt = 0; iCnt < SKI_NUM_TRADING_INV_SLOTS; iCnt++)
	{
		MSYS_EnableRegion( &gDealersOfferSlotsMouseRegions[ iCnt ] );

		//if the dealer repairs
		if( ArmsDealerInfo[ gbSelectedArmsDealerID ].ubTypeOfArmsDealer == ARMS_DEALER_REPAIRS )
		{
			MSYS_EnableRegion( &gDealersOfferSlotsSmallFaceMouseRegions[ iCnt ] );
		}
	}
}



void HatchOutInvSlot( UINT16 usPosX, UINT16 usPosY )
{
	UINT16 usSlotX, usSlotY;
	UINT16 usSlotWidth, usSlotHeight;

	//if we are in the item desc panel
	if( InItemDescriptionBox( ) && pShopKeeperItemDescObject != NULL )
	{
		// do nothing, don't wanna shade things twice!
		return;
	}

	usSlotX = ( UINT16 ) ( usPosX - 1 );
	usSlotY = ( UINT16 ) ( usPosY - 1 );
	usSlotWidth = SKI_INV_SLOT_WIDTH;
	usSlotHeight = SKI_INV_SLOT_HEIGHT;

	//Hatch it out
	DrawHatchOnInventory( guiRENDERBUFFER, usSlotX, usSlotY, usSlotWidth, usSlotHeight );
	InvalidateRegion( usSlotX, usSlotY, usSlotX + usSlotWidth, usSlotY + usSlotHeight );
}
