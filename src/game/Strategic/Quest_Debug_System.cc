#include "Quest_Debug_System.h"
#include "Button_System.h"
#include "ContentManager.h"
#include "Cursors.h"
#include "Dialogue_Control.h"
#include "Directories.h"
#include "Environment.h"
#include "Faces.h"
#include "Font.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "GameInstance.h"
#include "Handle_Items.h"
#include "HImage.h"
#include "Input.h"
#include "Interface_Dialogue.h"
#include "ItemModel.h"
#include "Items.h"
#include "Keys.h"
#include "Line.h"
#include "MercProfile.h"
#include "Message.h"
#include "MessageBoxScreen.h"
#include "OppList.h"
#include "Overhead.h"
#include "Quests.h"
#include "QuestText.h"
#include "Random.h"
#include "Render_Dirty.h"
#include "SGP.h"
#include "Soldier_Add.h"
#include "Soldier_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Profile.h"
#include "StrategicMap.h"
#include "SysUtil.h"
#include "Text_Input.h"
#include "Types.h"
#include "UILayout.h"
#include "Video.h"
#include "VObject.h"
#include "VSurface.h"
#include "WordWrap.h"
#include <string_theory/format>
#include <string_theory/string>


#define QUEST_DBS_FONT_TITLE			FONT14ARIAL
#define QUEST_DBS_COLOR_TITLE			FONT_MCOLOR_LTGREEN
#define QUEST_DBS_COLOR_SUBTITLE		FONT_MCOLOR_DKGRAY

#define QUEST_DBS_FONT_STATIC_TEXT		FONT12ARIAL
#define QUEST_DBS_COLOR_STATIC_TEXT		FONT_MCOLOR_WHITE

#define QUEST_DBS_FONT_DYNAMIC_TEXT		FONT12ARIAL
#define QUEST_DBS_COLOR_DYNAMIC_TEXT		FONT_MCOLOR_WHITE

#define QUEST_DBS_FONT_LISTBOX_TEXT		FONT12ARIAL

#define QUEST_DBS_FONT_TEXT_ENTRY		FONT12ARIAL
#define QUEST_DBS_COLOR_TEXT_ENTRY		FONT_MCOLOR_WHITE

#define QUEST_DBS_FIRST_SECTION_WIDTH		210
#define QUEST_DBS_SECOND_SECTION_WIDTH		230
#define QUEST_DBS_THIRD_SECTION_WIDTH		200


#define QUEST_DBS_NUMBER_COL_WIDTH		40
#define QUEST_DBS_TITLE_COL_WIDTH		120
#define QUEST_DBS_STATUS_COL_WIDTH		50


#define QUEST_DBS_FIRST_COL_NUMBER_X		5
#define QUEST_DBS_FIRST_COL_NUMBER_Y		50

#define QUEST_DBS_FIRST_COL_TITLE_X		QUEST_DBS_FIRST_COL_NUMBER_X + QUEST_DBS_NUMBER_COL_WIDTH
#define QUEST_DBS_FIRST_COL_TITLE_Y		QUEST_DBS_FIRST_COL_NUMBER_Y

#define QUEST_DBS_FIRST_COL_STATUS_X		QUEST_DBS_FIRST_COL_TITLE_X + QUEST_DBS_TITLE_COL_WIDTH
#define QUEST_DBS_FIRST_COL_STATUS_Y		QUEST_DBS_FIRST_COL_NUMBER_Y


#define QUEST_DBS_SECOND_TITLE_COL_WIDTH	140


#define QUEST_DBS_SECOND_COL_NUMBER_X		QUEST_DBS_FIRST_SECTION_WIDTH + 5
#define QUEST_DBS_SECOND_COL_NUMBER_Y		QUEST_DBS_FIRST_COL_NUMBER_Y

#define QUEST_DBS_SECOND_COL_TITLE_X		QUEST_DBS_SECOND_COL_NUMBER_X + QUEST_DBS_NUMBER_COL_WIDTH
#define QUEST_DBS_SECOND_COL_TITLE_Y		QUEST_DBS_SECOND_COL_NUMBER_Y

#define QUEST_DBS_SECOND_COL_STATUS_X		QUEST_DBS_SECOND_COL_TITLE_X + QUEST_DBS_SECOND_TITLE_COL_WIDTH
#define QUEST_DBS_SECOND_COL_STATUS_Y		QUEST_DBS_SECOND_COL_NUMBER_Y


#define QUEST_DBS_SECTION_TITLE_Y		30

#define QUEST_DBS_MAX_DISPLAYED_ENTRIES		20//25

#define QUEST_DBS_THIRD_COL_TITLE_X		QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH


#define QUEST_DBS_NPC_CHCKBOX_TGL_X		QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH + 5
#define QUEST_DBS_NPC_CHCKBOX_TGL_Y		QUEST_DBS_FIRST_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_SELECTED_NPC_BUTN_X		QUEST_DBS_NPC_CHCKBOX_TGL_X
#define QUEST_DBS_SELECTED_NPC_BUTN_Y		QUEST_DBS_NPC_CHCKBOX_TGL_Y + 22

#define QUEST_DBS_SELECTED_ITEM_BUTN_X		QUEST_DBS_SELECTED_NPC_BUTN_X//QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH + 105
#define QUEST_DBS_SELECTED_ITEM_BUTN_Y		QUEST_DBS_SELECTED_NPC_BUTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_LIST_TEXT_OFFSET		26

#define QUEST_DBS_LIST_BOX_WIDTH		183//80

#define QUEST_DBS_SCROLL_BAR_WIDTH		11

#define QUEST_DBS_SCROLL_ARROW_HEIGHT		17

#define QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR	30


#define QUEST_DBS_ADD_NPC_BTN_X			QUEST_DBS_SELECTED_NPC_BUTN_X
#define QUEST_DBS_ADD_NPC_BTN_Y			QUEST_DBS_SELECTED_ITEM_BUTN_Y  + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_ADD_ITEM_BTN_X		QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_ADD_ITEM_BTN_Y		QUEST_DBS_ADD_NPC_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_X	QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_Y	QUEST_DBS_ADD_ITEM_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_CHANGE_DAY_BTN_X		QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_CHANGE_DAY_BTN_Y		QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_VIEW_NPC_INV_BTN_X		QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_VIEW_NPC_INV_BTN_Y		QUEST_DBS_CHANGE_DAY_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_RESTORE_NPC_INV_BTN_X		QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_RESTORE_NPC_INV_BTN_Y		QUEST_DBS_VIEW_NPC_INV_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_NPC_LOG_BTN_X			QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_NPC_LOG_BTN_Y			QUEST_DBS_RESTORE_NPC_INV_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_NPC_REFRESH_BTN_X		QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_NPC_REFRESH_BTN_Y		QUEST_DBS_NPC_LOG_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_START_MERC_TALKING_BTN_X	QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_START_MERC_TALKING_BTN_Y	QUEST_DBS_NPC_REFRESH_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_ADD_NPC_TO_TEAM_BTN_X		QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y		QUEST_DBS_START_MERC_TALKING_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET

#define QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_X	QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y	QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y + QUEST_DBS_LIST_TEXT_OFFSET



#define QUEST_DBS_NPC_CURRENT_GRIDNO_X	QUEST_DBS_ADD_NPC_BTN_X
#define QUEST_DBS_NPC_CURRENT_GRIDNO_Y	QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y	+ QUEST_DBS_LIST_TEXT_OFFSET


//Text Entry Box
#define QUEST_DBS_TEB_X	200
#define QUEST_DBS_TEB_Y	160

#define QUEST_DBS_TEB_WIDTH	245
#define QUEST_DBS_TEB_HEIGHT	140

#define QUEST_DBS_NUM_DISPLAYED_QUESTS	MAX_QUESTS
#define QUEST_DBS_NUM_DISPLAYED_FACTS	25

#define QUEST_DBS_TEXT_FIELD_WIDTH	7

//NPC Inventory Popup box
#define QUEST_DBS_NPC_INV_POPUP_X	150
#define QUEST_DBS_NPC_INV_POPUP_Y	110

#define QUEST_DBS_NPC_INV_POPUP_WIDTH	275
#define QUEST_DBS_NPC_INV_POPUP_HEIGHT	325

#define QUEST_DBS_SIZE_NPC_ARRAY	TOTAL_SOLDIERS

#define QUEST_DBS_FACT_LIST_OFFSET	28


#define QDS_BUTTON_HEIGHT	21


#define QDS_CURRENT_QUOTE_NUM_BOX_X	150
#define QDS_CURRENT_QUOTE_NUM_BOX_Y	300
#define QDS_CURRENT_QUOTE_NUM_BOX_WIDTH	285
#define QDS_CURRENT_QUOTE_NUM_BOX_HEIGHT	80


//
// drop down list box
//
enum
{
	QD_DROP_DOWN_NO_ACTION = 0,
	QD_DROP_DOWN_CREATE,
	QD_DROP_DOWN_DESTROY,
	QD_DROP_DOWN_DISPLAY,
	QD_DROP_DOWN_CANCEL,
};


static const ST::string QuestStates[] = {
	"N.S.",
	"In Prog.",
	"Done"
};

static const ST::string QuestDebugText[] =
{
	"Quest Debug System",
	"Quests",
	"Quest Number",
	"Quest Title",
	"Status",
	"Facts",
	"Fact Number",
	"Desc.",
	"Select Merc",
	"Select Item",
	"NPC RecordLog",
	"Exit Quest Debug",
	"NPC Info",
	"** No Item **",
	"Add Merc To Location",
	"Add Item To Location",
	"Change Day",
	"NPC log Button",
	"Please Enter the grid #",
	"Give Item To NPC",
	"View NPC's Inventory",
	"Please enter the number of days to advance.",
	"NPC Inventory",
	"View NPC's in current sector",
	"No NPC's In Sector",
	"Please Enter New Value for ",
	"0,1,2",
	"0,1",
	"Quest #",
	"Fact #",
	"Pg Facts Up",
	"Pg Facts Down",
	"No Text",
	"CurrentGridNo",
	"Refresh NPC Script",
	"Succesfully Refreshed",
	"Failed Refreshing",
	"Restore All NPC's inventory",
	"Start Merc Talking",
	"Please enter a quote number for the selected merc to start talking from.",
	"RPC is added to team",
	"RPC says Sector Desc",
	"Space:       Toggle Pausing Merc Speech",
	"Left Arrow:  Previous Quote",
	"Right Arrow: Next Quote",
	"ESC:         To Stop the merc from Talking",
	"",
	""
};


//enums for above strings
enum
{
	QUEST_DBS_TITLE=0,
	QUEST_DBS_QUESTS,
	QUEST_DBS_QUEST_NUMBER,
	QUEST_DBS_QUEST_TITLE,
	QUEST_DBS_STATUS,
	QUEST_DBS_FACTS,
	QUEST_DBS_FACT_NUMBER,
	QUEST_DBS_DESC,
	QUEST_DBS_SELECTED_NPC,
	QUEST_DBS_SELECTED_ITEM,
	QUEST_DBS_NPC_RECORDLOG,
	QUEST_DBS_EXIT_QUEST_DEBUG,
	QUEST_DBS_NPC_INFO,
	QUEST_DBS_NO_ITEM,
	QUEST_DBS_ADD_CURRENT_NPC,
	QUEST_DBS_ADD_CURRENT_ITEM,
	QUEST_DBS_CHANGE_DAY,
	QUEST_DBS_NPC_LOG_BUTTON,
	QUEST_DBS_ENTER_GRID_NUM,
	QUEST_DBS_GIVE_ITEM_TO_NPC,
	QUEST_DBS_VIEW_NPC_INVENTORY,
	QUEST_DBS_PLEASE_ENTER_DAY,
	QUEST_DBS_NPC_INVENTORY,
	QUEST_DBS_VIEW_LOCAL_NPC,
	QUEST_DBS_NO_NPC_IN_SECTOR,
	QUEST_DBS_ENTER_NEW_VALUE,
	QUEST_DBS_0_1_2,
	QUEST_DBS_0_1,
	QUEST_DBS_QUEST_NUM,
	QUEST_DBS_FACT_NUM,
	QUEST_DBS_PG_FACTS_UP,
	QUEST_DBS_PG_FACTS_DOWN,
	QUEST_DBS_NO_TEXT,
	QUEST_DBS_CURRENT_GRIDNO,
	QUEST_DBS_REFRESH_NPC,
	QUEST_DBS_REFRESH_OK,
	QUEST_DBS_REFRESH_FAILED,
	QUEST_DBS_RESTORE_NPC_INVENTORY,
	QUEST_DBS_START_MERC_TALKING,
	QUEST_DBS_START_MERC_TALKING_FROM,
	QUEST_DBS_ADD_NPC_TO_TEAM,
	QUEST_DBS_RPC_SAY_SECTOR_DESC,
	QUEST_DBS_PAUSE_SPEECH,
	QUEST_DBS_LEFT_ARROW_PREVIOUS_QUOTE,
	QUEST_DBS_RIGHT_ARROW_NEXT_QUOTE,
	QUEST_DBS_ESC_TOP_STOP_TALKING
};


static const ST::string PocketText[] = {
	"Helmet",
	"Vest",
	"Leg",
	"Head1",
	"Head2",
	"Hand",
	"Second Hand",
	"Bigpock1",
	"Bigpock2",
	"Bigpock3",
	"Bigpock4",
	"Smallpock1",
	"Smallpock2",
	"Smallpock3",
	"Smallpock4",
	"Smallpock5",
	"Smallpock6",
	"Smallpock7",
	"Smallpock8"
};


typedef void (*LISTBOX_DISPLAY_FNCTN)();    // Define Display Callback function
typedef void (*TEXT_ENTRY_CALLBACK)(INT32); // Callback for when the text entry field is finished

struct SCROLL_BOX
{
	LISTBOX_DISPLAY_FNCTN DisplayFunction; // The array of items

	UINT16 usScrollPosX;                   // Top Left Pos of list box
	UINT16 usScrollPosY;                   // Top Left Pos of list box
	UINT16 usScrollHeight;                 // Height of list box
	UINT16 usScrollWidth;                  // Width of list box

	UINT16 usScrollBarHeight;              // Height of Scroll box
	UINT16 usScrollBarWidth;               // Width of Scroll box
	UINT16 usScrollBoxY;                   // Current Vertical location of the scroll box
	UINT16 usScrollBoxEndY;                // Bottom position on the scroll box
	UINT16 usScrollArrowHeight;            // Scroll Arrow height

	INT16  sCurSelectedItem;               // Currently selected item
	UINT16 usItemDisplayedOnTopOfList;     // item at the top of displayed list
	UINT16 usStartIndex;                   // index to start at for the array of elements
	UINT16 usMaxArrayIndex;                // Max Size of the array
	UINT16 usNumDisplayedItems;            // Num of displayed item
	UINT16 usMaxNumDisplayedItems;         // Max number of Displayed items

	UINT8  ubCurScrollBoxAction;           // Holds the status of the current action ( create; destroy... )
};


//Enums for the possible panels the mercs can use
enum
{
	QDS_REGULAR_PANEL,
	QDS_NPC_PANEL,
	QDS_NO_PANEL,
};

//image identifiers
static SGPVObject* guiQdScrollArrowImage;


static BOOLEAN gfQuestDebugEntry = TRUE;
static BOOLEAN gfQuestDebugExit  = FALSE;

static BOOLEAN gfRedrawQuestDebugSystem = TRUE;

static UINT16 gusQuestDebugBlue;


static SCROLL_BOX gNpcListBox;  // The Npc Scroll box
static SCROLL_BOX gItemListBox;

static SCROLL_BOX* gpActiveListBox; // Only 1 scroll box is active at a time, this is set to it.

INT16 gsQdsEnteringGridNo = 0;


static UINT8   gubTextEntryAction = QD_DROP_DOWN_NO_ACTION;
static BOOLEAN gfTextEntryActive  = FALSE;

static BOOLEAN gfUseLocalNPCs = FALSE;

static UINT8 gubNPCInventoryPopupAction = QD_DROP_DOWN_NO_ACTION;

static UINT8 gubCurrentNpcInSector[QUEST_DBS_SIZE_NPC_ARRAY];
static UINT8 gubNumNPCinSector;

static UINT8  gubCurQuestSelected;
static UINT16 gusCurFactSelected;
static UINT16 gusFactAtTopOfList;

static BOOLEAN gfNpcLogButton = FALSE;


static INT32 giHaveSelectedItem = -1; // If it is not the first time in, dont reset the Selected ITem
static INT32 giHaveSelectedNPC  = -1; // If it is not the first time in, dont reset the selected NPC


static INT32        giSelectedMercCurrentQuote = -1;
static SOLDIERTYPE* gTalkingMercSoldier        = NULL;
static BOOLEAN      gfPauseTalkingMercPopup    = FALSE;
static BOOLEAN gfAddNpcToTeam                 = FALSE;
static BOOLEAN gfRpcToSaySectorDesc           = FALSE;
static BOOLEAN gfNpcPanelIsUsedForTalkingMerc = FALSE;
extern SOLDIERTYPE* gpDestSoldier;

static BOOLEAN gfBackgroundMaskEnabled = FALSE;

static BOOLEAN gfExitQdsDueToMessageBox = FALSE;

static BOOLEAN gfInDropDownBox = FALSE;

static BOOLEAN gfAddKeyNextPass   = FALSE;
static BOOLEAN gfDropDamagedItems = FALSE;

static MOUSE_REGION gQuestDebugSysScreenRegions;


static GUIButtonRef guiQuestDebugExitButton;

//checkbox for weather to show all npc or just npc in sector
static GUIButtonRef guiQuestDebugAllOrSectorNPCToggle;


static GUIButtonRef guiQuestDebugCurNPCButton;
static GUIButtonRef guiQuestDebugCurItemButton;
static GUIButtonRef guiQuestDebugAddNpcToLocationButton;
static GUIButtonRef guiQuestDebugAddItemToLocationButton;
static GUIButtonRef guiQuestDebugGiveItemToNPCButton;
static GUIButtonRef guiQuestDebugChangeDayButton;
static GUIButtonRef guiQuestDebugViewNPCInvButton;
static GUIButtonRef guiQuestDebugRestoreNPCInvButton;
static GUIButtonRef guiQuestDebugNPCLogButtonButton;
static GUIButtonRef guiQuestDebugNPCRefreshButtonButton;
static GUIButtonRef guiQuestDebugStartMercTalkingButtonButton;

// checkbox for whether to add the merc to the players team
static GUIButtonRef guiQuestDebugAddNpcToTeamToggle;

// checkbox for whether have rpc say the sector description
static GUIButtonRef guiQuestDebugRPCSaySectorDescToggle;


static MOUSE_REGION gSelectedNpcListRegion[QUEST_DBS_MAX_DISPLAYED_ENTRIES];
static MOUSE_REGION gScrollAreaRegion[QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR];
static MOUSE_REGION gScrollArrowsRegion[2];


//Text entry Disable the screen
static MOUSE_REGION gQuestTextEntryDebugDisableScreenRegion;

//Ok button on the text entry form
static GUIButtonRef guiQuestDebugTextEntryOkBtn;

//Ok button on the NPC inventory form
static GUIButtonRef guiQuestDebugNPCInventOkBtn;


// Mouse regions for the Quests
static MOUSE_REGION gQuestListRegion[QUEST_DBS_NUM_DISPLAYED_QUESTS];

// Mouse regions for the Facts
static MOUSE_REGION gFactListRegion[QUEST_DBS_NUM_DISPLAYED_FACTS];


static GUIButtonRef guiQDPgUpButtonButton;
static GUIButtonRef guiQDPgDownButtonButton;


static void DisplaySelectedItem(void);
static void DisplaySelectedNPC(void);


void QuestDebugScreenInit()
{
	UINT16	usListBoxFontHeight = GetFontHeight( QUEST_DBS_FONT_LISTBOX_TEXT ) + 2;

	//Set so next time we come in, we can set up
	gfQuestDebugEntry = TRUE;

	gusQuestDebugBlue = Get16BPPColor( FROMRGB(  65,  79,  94 ) );

	//Initialize which facts are at the top of the list
	gusFactAtTopOfList = 0;

	gubCurQuestSelected = 0;
	gusCurFactSelected = 0;



	//
	//Set the Npc List box
	//
	gNpcListBox = SCROLL_BOX{};
	gNpcListBox.DisplayFunction								= DisplaySelectedNPC;													//	The function to display the entries

	gNpcListBox.usScrollPosX									= QUEST_DBS_SELECTED_NPC_BUTN_X;
	gNpcListBox.usScrollPosY									= QUEST_DBS_SELECTED_NPC_BUTN_Y + 25;
	gNpcListBox.usScrollHeight								= usListBoxFontHeight * QUEST_DBS_MAX_DISPLAYED_ENTRIES;
	gNpcListBox.usScrollWidth									= QUEST_DBS_LIST_BOX_WIDTH;
	gNpcListBox.usScrollArrowHeight						= QUEST_DBS_SCROLL_ARROW_HEIGHT;
	gNpcListBox.usScrollBarHeight							= gNpcListBox.usScrollHeight - ( 2 * gNpcListBox.usScrollArrowHeight );
	gNpcListBox.usScrollBarWidth							= QUEST_DBS_SCROLL_BAR_WIDTH;

	gNpcListBox.sCurSelectedItem							= -1;
	gNpcListBox.usItemDisplayedOnTopOfList		= 0;
	gNpcListBox.usStartIndex									= 0;
	gNpcListBox.usMaxArrayIndex								= NUM_PROFILES;
	gNpcListBox.usNumDisplayedItems						= QUEST_DBS_MAX_DISPLAYED_ENTRIES;
	gNpcListBox.usMaxNumDisplayedItems				= QUEST_DBS_MAX_DISPLAYED_ENTRIES;

	gNpcListBox.ubCurScrollBoxAction					= QD_DROP_DOWN_NO_ACTION;



	//
	//Set the Item List box
	//
	gItemListBox = SCROLL_BOX{};
	gItemListBox.DisplayFunction									= DisplaySelectedItem;													//	The function to display the entries

	gItemListBox.usScrollPosX										= QUEST_DBS_SELECTED_ITEM_BUTN_X;
	gItemListBox.usScrollPosY										= QUEST_DBS_SELECTED_ITEM_BUTN_Y + 25;
	gItemListBox.usScrollHeight									= usListBoxFontHeight * QUEST_DBS_MAX_DISPLAYED_ENTRIES;
	gItemListBox.usScrollWidth									= QUEST_DBS_LIST_BOX_WIDTH;
	gItemListBox.usScrollArrowHeight						= QUEST_DBS_SCROLL_ARROW_HEIGHT;
	gItemListBox.usScrollBarHeight							= gItemListBox.usScrollHeight - ( 2 * gItemListBox.usScrollArrowHeight );
	gItemListBox.usScrollBarWidth								= QUEST_DBS_SCROLL_BAR_WIDTH;

	gItemListBox.sCurSelectedItem							= -1;

	gItemListBox.usItemDisplayedOnTopOfList			= 1;
	gItemListBox.usStartIndex										= 1;
	gItemListBox.usMaxArrayIndex								= MAXITEMS;
	gItemListBox.usNumDisplayedItems						= QUEST_DBS_MAX_DISPLAYED_ENTRIES;
	gItemListBox.usMaxNumDisplayedItems				= QUEST_DBS_MAX_DISPLAYED_ENTRIES;

	gItemListBox.ubCurScrollBoxAction						= QD_DROP_DOWN_NO_ACTION;


	gfUseLocalNPCs = FALSE;


	//Set up the global list box
	gpActiveListBox = &gNpcListBox;
}


static void CreateDestroyDisplayTextEntryBox(UINT8 ubAction, const ST::string& pString, TEXT_ENTRY_CALLBACK EntryCallBack);
static void EnterQuestDebugSystem(void);
static void ExitQuestDebugSystem(void);
static void GetUserInput(void);
static void HandleQuestDebugSystem(void);
static void RenderQuestDebugSystem(void);
static UINT8 WhichPanelShouldTalkingMercUse(void);


ScreenID QuestDebugScreenHandle()
{
	if( gfQuestDebugEntry )
	{
		PauseGame();

		EnterQuestDebugSystem();
		gfQuestDebugEntry = FALSE;
		gfQuestDebugExit = FALSE;

		RenderQuestDebugSystem();

		//At this point the background is pure, copy it to the save buffer
		BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
	}
	RestoreBackgroundRects();

	// ATE: Disable messages....
	DisableScrollMessages( );

	GetUserInput();

	if( gfTextEntryActive || gubTextEntryAction )
	{
		if( gubTextEntryAction != QD_DROP_DOWN_NO_ACTION )
		{
			CreateDestroyDisplayTextEntryBox( gubTextEntryAction, {}, NULL );
			gubTextEntryAction = QD_DROP_DOWN_NO_ACTION;
		}

		RenderAllTextFields();

	}
	else
		HandleQuestDebugSystem();

	if( gfRedrawQuestDebugSystem )
	{
		RenderQuestDebugSystem();

		gfRedrawQuestDebugSystem = FALSE;
	}

	//if the merc is supposed to be talking
	if( giSelectedMercCurrentQuote != -1 )
	{
		//and it is an npc
		if( WhichPanelShouldTalkingMercUse( ) == QDS_NPC_PANEL )
		{
			guiQDPgUpButtonButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
			RenderTalkingMenu( );
		}
	}


	// render buttons marked dirty
	RenderButtons( );

	//To handle the dialog
	HandleDialogue();
	HandleAutoFaces( );
	HandleTalkingAutoFaces( );

	ExecuteVideoOverlays();

	SaveBackgroundRects( );
	RenderFastHelp();

	if( gfQuestDebugExit )
	{
		ExitQuestDebugSystem();
		gfQuestDebugExit = FALSE;
		gfQuestDebugEntry = TRUE;

		UnPauseGame();
		return( GAME_SCREEN );
	}

	return( QUEST_DEBUG_SCREEN );
}


static GUIButtonRef MakeButton(const ST::string& text, INT16 x, INT16 y, INT16 w, INT16 h, INT16 priority, GUI_CALLBACK click)
{
	return CreateTextButton(text, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, FONT_BLACK, x, y, w, h, priority, click);
}


static GUIButtonRef MakeCheckBox(INT16 const x, INT16 const y, GUI_CALLBACK const click, bool const checked = false)
{
	GUIButtonRef const b = CreateCheckBoxButton(x, y, INTERFACEDIR "/checkbox.sti", MSYS_PRIORITY_HIGH + 2, click);
	if (checked) b->uiFlags |= BUTTON_CLICKED_ON;
	return b;
}


static void AddNPCsInSectorToArray(void);
static void BtnQDPgDownButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQDPgUpButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugAddItemToLocationButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugAddNpcToTeamToggleCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugAllOrSectorNPCToggleCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugChangeDayButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugCurItemButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugCurNPCButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugAddNpcToLocationButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugExitButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugGiveItemToNPCButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugNPCLogButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugNPCRefreshButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugRPCSaySectorDescToggleCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugRestoreNPCInvButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugStartMercTalkingButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void BtnQuestDebugViewNPCInvButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void EnableQDSButtons(void);
static void ScrollFactListRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void ScrollQuestListRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static void EnterQuestDebugSystem(void)
{
	UINT8	i;
	UINT16 usPosX, usPosY;
	ST::string zName;
//	UINT16	usListBoxFontHeight = GetFontHeight( QUEST_DBS_FONT_LISTBOX_TEXT ) + 2;

	UINT16 usFontHeight = GetFontHeight( QUEST_DBS_FONT_DYNAMIC_TEXT ) + 2;

	if( gfExitQdsDueToMessageBox )
	{
		gfRedrawQuestDebugSystem = TRUE;
		gfExitQdsDueToMessageBox = FALSE;
		return;
	}

	MSYS_DefineRegion(&gQuestDebugSysScreenRegions, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	guiQuestDebugExitButton = MakeButton(QuestDebugText[QUEST_DBS_EXIT_QUEST_DEBUG], 535, 450, 100, 25, MSYS_PRIORITY_HIGH + 10, BtnQuestDebugExitButtonCallback);

	//Check box to toggle between all and local npc's
	guiQuestDebugAllOrSectorNPCToggle = MakeCheckBox(QUEST_DBS_NPC_CHCKBOX_TGL_X, QUEST_DBS_NPC_CHCKBOX_TGL_Y, BtnQuestDebugAllOrSectorNPCToggleCallback);

	//Currently Selected NPC button
	guiQuestDebugCurNPCButton = MakeButton(QuestDebugText[QUEST_DBS_SELECTED_NPC], QUEST_DBS_SELECTED_NPC_BUTN_X, QUEST_DBS_SELECTED_NPC_BUTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugCurNPCButtonCallback);



	// Currently Selected item button
	guiQuestDebugCurItemButton = MakeButton(QuestDebugText[QUEST_DBS_SELECTED_ITEM], QUEST_DBS_SELECTED_ITEM_BUTN_X, QUEST_DBS_SELECTED_ITEM_BUTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugCurItemButtonCallback);

	// Add NPC to location
	guiQuestDebugAddNpcToLocationButton = MakeButton(QuestDebugText[QUEST_DBS_ADD_CURRENT_NPC], QUEST_DBS_ADD_NPC_BTN_X, QUEST_DBS_ADD_NPC_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugAddNpcToLocationButtonCallback);

	// Add item to location
	guiQuestDebugAddItemToLocationButton = MakeButton(QuestDebugText[QUEST_DBS_ADD_CURRENT_ITEM], QUEST_DBS_ADD_ITEM_BTN_X, QUEST_DBS_ADD_ITEM_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugAddItemToLocationButtonCallback);

	// Give item to Npc
	guiQuestDebugGiveItemToNPCButton = MakeButton(QuestDebugText[QUEST_DBS_GIVE_ITEM_TO_NPC], QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_X, QUEST_DBS_GIVE_ITEM_TO_NPC_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugGiveItemToNPCButtonCallback);

	// Change Day
	guiQuestDebugChangeDayButton = MakeButton(QuestDebugText[QUEST_DBS_CHANGE_DAY], QUEST_DBS_CHANGE_DAY_BTN_X, QUEST_DBS_CHANGE_DAY_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugChangeDayButtonCallback);

	// View NPC Inventory
	guiQuestDebugViewNPCInvButton = MakeButton(QuestDebugText[QUEST_DBS_VIEW_NPC_INVENTORY], QUEST_DBS_VIEW_NPC_INV_BTN_X, QUEST_DBS_VIEW_NPC_INV_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugViewNPCInvButtonCallback);

	// Restore NPC Inventory
	guiQuestDebugRestoreNPCInvButton = MakeButton(QuestDebugText[QUEST_DBS_RESTORE_NPC_INVENTORY], QUEST_DBS_RESTORE_NPC_INV_BTN_X, QUEST_DBS_RESTORE_NPC_INV_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugRestoreNPCInvButtonCallback);


	// NPC log button
	zName = ST::format("{} - ({})", QuestDebugText[ QUEST_DBS_NPC_LOG_BUTTON ], gfNpcLogButton ? "On" : "Off");
	guiQuestDebugNPCLogButtonButton = MakeButton(zName, QUEST_DBS_NPC_LOG_BTN_X, QUEST_DBS_NPC_LOG_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugNPCLogButtonButtonCallback);

	guiQuestDebugNPCRefreshButtonButton = MakeButton(QuestDebugText[QUEST_DBS_REFRESH_NPC], QUEST_DBS_NPC_REFRESH_BTN_X, QUEST_DBS_NPC_REFRESH_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugNPCRefreshButtonButtonCallback);


	//Start the selected merc talking
	guiQuestDebugStartMercTalkingButtonButton = MakeButton(QuestDebugText[QUEST_DBS_START_MERC_TALKING], QUEST_DBS_START_MERC_TALKING_BTN_X, QUEST_DBS_START_MERC_TALKING_BTN_Y, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQuestDebugStartMercTalkingButtonButtonCallback);



	guiQDPgUpButtonButton = MakeButton(QuestDebugText[QUEST_DBS_PG_FACTS_UP], QUEST_DBS_SECOND_COL_NUMBER_X + 5, QUEST_DBS_SECOND_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQDPgUpButtonButtonCallback);



	guiQDPgDownButtonButton = MakeButton(QuestDebugText[QUEST_DBS_PG_FACTS_DOWN], QUEST_DBS_SECOND_COL_NUMBER_X + 5, QUEST_DBS_SECOND_COL_NUMBER_Y + 15 * QUEST_DBS_NUM_DISPLAYED_FACTS + QUEST_DBS_LIST_TEXT_OFFSET, QUEST_DBS_LIST_BOX_WIDTH, QDS_BUTTON_HEIGHT, MSYS_PRIORITY_HIGH + 2, BtnQDPgDownButtonButtonCallback);


	//checkbox for weather to add the merc to the players team
	guiQuestDebugAddNpcToTeamToggle = MakeCheckBox(QUEST_DBS_ADD_NPC_TO_TEAM_BTN_X, QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y, BtnQuestDebugAddNpcToTeamToggleCallback, gfAddNpcToTeam);

	//checkbox for weather have rpc say the sector description
	guiQuestDebugRPCSaySectorDescToggle = MakeCheckBox(QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_X, QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y, BtnQuestDebugRPCSaySectorDescToggleCallback, gfRpcToSaySectorDesc);

	//Setup mouse regions for the Quest list
	usPosX = QUEST_DBS_FIRST_COL_NUMBER_X;
	usPosY = QUEST_DBS_FIRST_COL_NUMBER_Y  + QUEST_DBS_LIST_TEXT_OFFSET;
	for( i=0; i< QUEST_DBS_NUM_DISPLAYED_QUESTS; i++)
	{
		MSYS_DefineRegion(&gQuestListRegion[i], usPosX, usPosY, usPosX + QUEST_DBS_FIRST_SECTION_WIDTH, usPosY + usFontHeight, MSYS_PRIORITY_HIGH + 2, CURSOR_WWW, MSYS_NO_CALLBACK, ScrollQuestListRegionCallBack); //CURSOR_LAPTOP_SCREEN
		MSYS_SetRegionUserData( &gQuestListRegion[ i ], 0, i);

		usPosY += usFontHeight;
	}


	//Setup mouse regions for the Fact lists
	usPosX = QUEST_DBS_SECOND_COL_NUMBER_X;
	usPosY = QUEST_DBS_SECOND_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET + QUEST_DBS_FACT_LIST_OFFSET;
	for( i=0; i< QUEST_DBS_NUM_DISPLAYED_FACTS; i++)
	{
		MSYS_DefineRegion(&gFactListRegion[i], usPosX, usPosY, usPosX + QUEST_DBS_SECOND_SECTION_WIDTH, usPosY + usFontHeight, MSYS_PRIORITY_HIGH + 2, CURSOR_WWW, MSYS_NO_CALLBACK, ScrollFactListRegionCallBack); //CURSOR_LAPTOP_SCREEN
		MSYS_SetRegionUserData( &gFactListRegion[ i ], 0, i);

		usPosY += usFontHeight;
	}

	// load Scroll Horizontal Arrow graphic and add it
	guiQdScrollArrowImage = AddVideoObjectFromFile(INTERFACEDIR "/qd_scrollarrows.sti");

	gfRedrawQuestDebugSystem = TRUE;



	AddNPCsInSectorToArray();

	//Remove the mouse region over the clock
	RemoveMouseRegionForPauseOfClock(  );


	//Disable the buttons the depend on a seleted item or npc
	DisableButton( guiQuestDebugAddNpcToLocationButton );
	DisableButton( guiQuestDebugStartMercTalkingButtonButton );
	DisableButton( guiQuestDebugAddItemToLocationButton );
	DisableButton( guiQuestDebugGiveItemToNPCButton );
	DisableButton( guiQuestDebugViewNPCInvButton );
	DisableButton( guiQuestDebugNPCLogButtonButton );
	DisableButton( guiQuestDebugNPCRefreshButtonButton );


	if( giHaveSelectedNPC != -1 )
	{
		const ProfileID pid = (gfUseLocalNPCs ? gubCurrentNpcInSector[giHaveSelectedNPC] : giHaveSelectedNPC);
		ST::string zItemDesc = ST::format("{} - {}", pid, GetProfile(pid).zNickname);
		guiQuestDebugCurNPCButton->SpecifyText(zItemDesc);

		gNpcListBox.sCurSelectedItem = (INT16)giHaveSelectedNPC;

		EnableQDSButtons();
	}

	if( giHaveSelectedItem != -1 )
	{
		ST::string zItemDesc = ST::format("{} - {}", giHaveSelectedItem, GCM->getItem(giHaveSelectedItem)->getShortName());
		guiQuestDebugCurItemButton->SpecifyText(zItemDesc);

		gItemListBox.sCurSelectedItem = (INT16)giHaveSelectedItem;

		EnableQDSButtons();
	}
}


static BOOLEAN CreateDestroyDisplaySelectNpcDropDownBox(void);
static void EndMercTalking(void);
static void QuestDebug_EnterTactical(void);


static void ExitQuestDebugSystem(void)
{
	UINT16 i;

	if( gfExitQdsDueToMessageBox )
	{
		return;
	}
	MSYS_RemoveRegion( &gQuestDebugSysScreenRegions );
	QuestDebug_EnterTactical();

	RemoveButton( guiQuestDebugExitButton );

	RemoveButton( guiQuestDebugCurNPCButton );
	RemoveButton( guiQuestDebugCurItemButton );
	RemoveButton( guiQuestDebugAddNpcToLocationButton );
	RemoveButton( guiQuestDebugAddItemToLocationButton );
	RemoveButton( guiQuestDebugChangeDayButton );
	RemoveButton( guiQuestDebugNPCLogButtonButton );
	RemoveButton( guiQuestDebugGiveItemToNPCButton );
	RemoveButton( guiQuestDebugViewNPCInvButton );
	RemoveButton( guiQuestDebugRestoreNPCInvButton );
	RemoveButton( guiQuestDebugAllOrSectorNPCToggle );
	RemoveButton( guiQuestDebugNPCRefreshButtonButton );
	RemoveButton( guiQuestDebugStartMercTalkingButtonButton );
	RemoveButton( guiQuestDebugAddNpcToTeamToggle );
	RemoveButton( guiQuestDebugRPCSaySectorDescToggle );

	RemoveButton( guiQDPgUpButtonButton );
	RemoveButton( guiQDPgDownButtonButton );

	DeleteVideoObject(guiQdScrollArrowImage);

	gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
	CreateDestroyDisplaySelectNpcDropDownBox();
	gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;

	//Remove the quest list mouse regions
	for( i=0; i< QUEST_DBS_NUM_DISPLAYED_QUESTS; i++)
		MSYS_RemoveRegion( &gQuestListRegion[ i ] );

	//Remove the fact list mouse regions
	for( i=0; i< QUEST_DBS_NUM_DISPLAYED_FACTS; i++)
		MSYS_RemoveRegion( &gFactListRegion[ i ] );

	CreateMouseRegionForPauseOfClock();

	giHaveSelectedNPC = gNpcListBox.sCurSelectedItem;
	giHaveSelectedItem = gItemListBox.sCurSelectedItem;

	EndMercTalking();

	giSelectedMercCurrentQuote = -1;
}


static void AddKeyToGridNo(INT32 iKeyID);
static void CreateDestroyDisplayNPCInventoryPopup(UINT8 ubAction);
static void HandleQDSTalkingMerc(void);
static void TextEntryBox(const ST::string& pString, TEXT_ENTRY_CALLBACK TextEntryCallBack);


static void HandleQuestDebugSystem(void)
{
	ST::string zTemp;

	HandleQDSTalkingMerc();


//	if( !gfTextEntryActive )
	if( gubTextEntryAction != QD_DROP_DOWN_NO_ACTION )
	{

	}

	if( gpActiveListBox->ubCurScrollBoxAction != QD_DROP_DOWN_NO_ACTION )
	{
		CreateDestroyDisplaySelectNpcDropDownBox( );

		if( gpActiveListBox->ubCurScrollBoxAction == QD_DROP_DOWN_CREATE )
			gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DISPLAY;
		else
			gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
	}

	if( gubNPCInventoryPopupAction != QD_DROP_DOWN_NO_ACTION )
	{
		CreateDestroyDisplayNPCInventoryPopup( gubNPCInventoryPopupAction );

		if( gubNPCInventoryPopupAction == QD_DROP_DOWN_CREATE )
			gubNPCInventoryPopupAction = QD_DROP_DOWN_DISPLAY;
		else
			gubNPCInventoryPopupAction = QD_DROP_DOWN_NO_ACTION;
	}

	if( gfAddKeyNextPass )
	{
		zTemp = ST::format("  Please enter the Keys ID. ( 0 - {} )", NUM_KEYS);
		TextEntryBox( zTemp, AddKeyToGridNo );
		gfAddKeyNextPass = FALSE;
	}

}


static void DisplayCurrentGridNo(void);
static void DisplayFactInformation(void);
static void DisplayFactList(void);
static void DisplayNPCInfo(void);
static void DisplayQDSCurrentlyQuoteNum(void);
static void DisplayQuestInformation(void);
static void DisplayQuestList(void);
static void DisplaySectionLine(void);


static void RenderQuestDebugSystem(void)
{
	ButtonDestBuffer->Fill(gusQuestDebugBlue);

	//display the title
	DisplayWrappedString(0, 5, SCREEN_WIDTH, 2, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_TITLE, QuestDebugText[QUEST_DBS_TITLE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Display vertical lines b/n sections
	DisplaySectionLine( );


	//Display the Quest Text
	DisplayQuestInformation();

	//Display the Fact Text
	DisplayFactInformation();

	//Display the list of quests
	DisplayQuestList();

	//Display the list of tasks
	DisplayFactList();

	//Display the NPC and Item info
	DisplayNPCInfo();

	//Display the text beside the NPC in current sector toggle box
	DrawTextToScreen(QuestDebugText[QUEST_DBS_VIEW_LOCAL_NPC], QUEST_DBS_NPC_CHCKBOX_TGL_X + 25, QUEST_DBS_NPC_CHCKBOX_TGL_Y + 1, QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the text beside the add npc to team toggle box
	DrawTextToScreen(QuestDebugText[QUEST_DBS_ADD_NPC_TO_TEAM], QUEST_DBS_NPC_CHCKBOX_TGL_X + 25, QUEST_DBS_ADD_NPC_TO_TEAM_BTN_Y + 1, QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the text beside the rpc say sector desc quotes
	DrawTextToScreen(QuestDebugText[QUEST_DBS_RPC_SAY_SECTOR_DESC], QUEST_DBS_NPC_CHCKBOX_TGL_X + 25, QUEST_DBS_RPC_TO_SAY_SECTOR_DESC_BTN_Y, QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


	DisplayCurrentGridNo();

	if( gfTextEntryActive )
	{
		gubTextEntryAction = QD_DROP_DOWN_DISPLAY;
		CreateDestroyDisplayTextEntryBox( gubTextEntryAction, {}, NULL );
		gubTextEntryAction = QD_DROP_DOWN_NO_ACTION;
	}


	//if there is a merc talking
	if( giSelectedMercCurrentQuote != -1 )
		DisplayQDSCurrentlyQuoteNum( );

	MarkButtonsDirty( );
	InvalidateScreen();
}


static void DisplayCurrentGridNo(void)
{
	if( gsQdsEnteringGridNo != 0 )
	{
		ST::string zTemp = ST::format("{}:  {}", QuestDebugText[QUEST_DBS_CURRENT_GRIDNO], gsQdsEnteringGridNo);
		DrawTextToScreen(zTemp, QUEST_DBS_NPC_CURRENT_GRIDNO_X, QUEST_DBS_NPC_CURRENT_GRIDNO_Y, QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
}


static FACETYPE* GetQDSFace(const UINT8 panel_merc_should_use)
{
	return panel_merc_should_use == QDS_REGULAR_PANEL ? gTalkingMercSoldier->face : gTalkPanel.face;
}


static void DoQDSMessageBox(const ST::string& zString, ScreenID uiExitScreen, MessageBoxFlags, MSGBOX_CALLBACK ReturnCallback);
static void IncrementActiveDropDownBox(INT16 sIncrementValue);
static void SetTalkingMercPauseState(BOOLEAN fState);


static void GetUserInput(void)
{
	InputAtom Event;
	UINT8	ubPanelMercShouldUse = WhichPanelShouldTalkingMercUse();

	while( DequeueSpecificEvent(&Event, KEYBOARD_EVENTS) )
	{
		if( !HandleTextInput( &Event ) && Event.usEvent == KEY_DOWN )
		{
			switch( Event.usParam )
			{
				case SDLK_ESCAPE:
					gubTextEntryAction = QD_DROP_DOWN_CANCEL;

					gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
					CreateDestroyDisplaySelectNpcDropDownBox();
					gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
					gfAddKeyNextPass = FALSE;



					EndMercTalking();

					break;

				case SDLK_SPACE:
					if( giSelectedMercCurrentQuote != -1 )
						SetTalkingMercPauseState(!gfPauseTalkingMercPopup);
					break;

				case SDLK_LEFT:
					if( giSelectedMercCurrentQuote != -1 )
					{
						ShutupaYoFace(GetQDSFace(ubPanelMercShouldUse));

						if( giSelectedMercCurrentQuote > 1 )
						{
							giSelectedMercCurrentQuote--;
							giSelectedMercCurrentQuote--;
						}
						else
							giSelectedMercCurrentQuote = 0;

						DisplayQDSCurrentlyQuoteNum( );

					}
					break;

				case SDLK_RIGHT:
					if( giSelectedMercCurrentQuote != -1 )
					{
						ShutupaYoFace(GetQDSFace(ubPanelMercShouldUse));

						//if( giSelectedMercCurrentQuote < GetMaxNumberOfQuotesToPlay( ) )
						//{
						//	giSelectedMercCurrentQuote++;
						//}
						DisplayQDSCurrentlyQuoteNum( );

					}
					break;

				case SDLK_F11: gfQuestDebugExit = TRUE; break;

				case 'x':
					if( Event.usKeyState & ALT_DOWN )
					{
						gfQuestDebugExit = TRUE;
						requestGameExit();
					}
					break;

				case SDLK_RETURN:
					if( gfTextEntryActive )
						gubTextEntryAction = QD_DROP_DOWN_DESTROY;
					else if( gfInDropDownBox )
					{
						gpActiveListBox->ubCurScrollBoxAction	= QD_DROP_DOWN_DESTROY;
					}

					break;

				case SDLK_PAGEDOWN:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)(gpActiveListBox->sCurSelectedItem + QUEST_DBS_MAX_DISPLAYED_ENTRIES ) );
					}
					break;

				case SDLK_PAGEUP:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)( gpActiveListBox->sCurSelectedItem - QUEST_DBS_MAX_DISPLAYED_ENTRIES ) );
					}
					break;

				case SDLK_DOWN:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)(gpActiveListBox->sCurSelectedItem + 1 ) );
					}
					break;

				case SDLK_UP:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)( gpActiveListBox->sCurSelectedItem - 1 ) );
					}
					break;

				case 'd':
					{
						// toggle whether dropped items are damaged or not
						gfDropDamagedItems ^= 1;
						ST::string zTemp = ST::format("Items dropped will be in {} condition", gfDropDamagedItems ? "DAMAGED" : "PERFECT");
						DoQDSMessageBox(zTemp, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK, NULL);
					}
					break;
			}
		}

		else if( Event.usEvent == KEY_REPEAT )
		{
			switch( Event.usParam )
			{
				case SDLK_PAGEDOWN:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)(gpActiveListBox->sCurSelectedItem + QUEST_DBS_MAX_DISPLAYED_ENTRIES ) );
					}
					break;

				case SDLK_PAGEUP:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)( gpActiveListBox->sCurSelectedItem - QUEST_DBS_MAX_DISPLAYED_ENTRIES ) );
					}
					break;

				case SDLK_DOWN:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)(gpActiveListBox->sCurSelectedItem + 1 ) );
					}
					break;

				case SDLK_UP:
					if( gfInDropDownBox )
					{
						IncrementActiveDropDownBox( (INT16)( gpActiveListBox->sCurSelectedItem - 1 ) );
					}
					break;


				case SDLK_LEFT:
					if( giSelectedMercCurrentQuote != -1 )
					{
						ShutupaYoFace(GetQDSFace(ubPanelMercShouldUse));

						if( giSelectedMercCurrentQuote > 1 )
						{
							giSelectedMercCurrentQuote--;
							giSelectedMercCurrentQuote--;
						}
						else
							giSelectedMercCurrentQuote = 0;

						DisplayQDSCurrentlyQuoteNum( );

					}
					break;

				case SDLK_RIGHT:
					if( giSelectedMercCurrentQuote != -1 )
					{
						DisplayQDSCurrentlyQuoteNum( );

						ShutupaYoFace(GetQDSFace(ubPanelMercShouldUse));
					}
					break;
			}
		}
	}
}


static void QuestDebug_EnterTactical(void)
{
	EnterTacticalScreen( );
}


static void DisplaySectionLine(void)
{
	UINT16 usStartX;
	UINT16 usStartY;
	UINT16 usEndX;
	UINT16 usEndY;

	usStartX = usEndX = QUEST_DBS_FIRST_SECTION_WIDTH;

	usStartY = QUEST_DBS_FIRST_COL_NUMBER_Y;
	usEndY = 475;

	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf = l.Buffer<UINT16>();

	// draw the line in b/n the first and second section
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	LineDraw(FALSE, usStartX, usStartY, usEndX, usEndY, Get16BPPColor( FROMRGB( 255, 255, 255 ) ), pDestBuf);

	// draw the line in b/n the second and third section
	usStartX = usEndX = QUEST_DBS_FIRST_SECTION_WIDTH + QUEST_DBS_SECOND_SECTION_WIDTH;
	LineDraw(FALSE, usStartX, usStartY, usEndX, usEndY, Get16BPPColor( FROMRGB( 255, 255, 255 ) ), pDestBuf);


	//draw the horizopntal line under the title
	usStartX = 0;
	usEndX   = SCREEN_WIDTH - 1;
	usStartY = usEndY = 75;
	LineDraw(FALSE, usStartX, usStartY, usEndX, usEndY, Get16BPPColor( FROMRGB( 255, 255, 255 ) ), pDestBuf);
}


static void DisplayQuestInformation(void)
{
	//Display Quests
	DisplayWrappedString(0, QUEST_DBS_SECTION_TITLE_Y, QUEST_DBS_FIRST_SECTION_WIDTH, 2, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_QUESTS], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display Quest Number text
	DisplayWrappedString(QUEST_DBS_FIRST_COL_NUMBER_X, QUEST_DBS_FIRST_COL_NUMBER_Y, QUEST_DBS_NUMBER_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_QUEST_NUMBER], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display Quest title text
	DisplayWrappedString(QUEST_DBS_FIRST_COL_TITLE_X, QUEST_DBS_FIRST_COL_TITLE_Y, QUEST_DBS_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_QUEST_TITLE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display Quest status text
	DisplayWrappedString(QUEST_DBS_FIRST_COL_STATUS_X, QUEST_DBS_FIRST_COL_STATUS_Y, QUEST_DBS_STATUS_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_STATUS], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
}


static void DisplayFactInformation(void)
{
	//Display Fact
	DisplayWrappedString(QUEST_DBS_FIRST_SECTION_WIDTH, QUEST_DBS_SECTION_TITLE_Y, QUEST_DBS_SECOND_SECTION_WIDTH, 2, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_FACTS], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display Fact Number text
	DisplayWrappedString(QUEST_DBS_SECOND_COL_NUMBER_X, QUEST_DBS_SECOND_COL_NUMBER_Y, QUEST_DBS_NUMBER_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_FACT_NUMBER], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display Fact title text
	DisplayWrappedString(QUEST_DBS_SECOND_COL_TITLE_X, QUEST_DBS_SECOND_COL_TITLE_Y, QUEST_DBS_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_DESC], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display Fact status text
	DisplayWrappedString(QUEST_DBS_SECOND_COL_STATUS_X, QUEST_DBS_SECOND_COL_STATUS_Y, QUEST_DBS_STATUS_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_STATUS], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
}


static void BtnQuestDebugExitButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gfQuestDebugExit = TRUE;
	}
}


static void DisplayQuestList(void)
{
	UINT16	usLoop1, usCount;
	UINT16	usTextHeight = GetFontHeight( QUEST_DBS_FONT_DYNAMIC_TEXT ) + 2;
	ST::string sTemp;
	UINT16	usPosY;

	usPosY = QUEST_DBS_FIRST_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET;	//&& (usCount < QUEST_DBS_MAX_DISPLAYED_ENTRIES )
	for( usLoop1=0, usCount=0; (usLoop1<MAX_QUESTS)  ; usLoop1++)
	{
		//Display Quest Number text
		sTemp = ST::format("{02d}", usLoop1);
		DrawTextToScreen(sTemp, QUEST_DBS_FIRST_COL_NUMBER_X, usPosY, QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

		//Display Quest title text
		DisplayWrappedString(QUEST_DBS_FIRST_COL_TITLE_X, usPosY, QUEST_DBS_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, QuestDescText[usLoop1], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

		//Display Quest status text
		DisplayWrappedString(QUEST_DBS_FIRST_COL_STATUS_X, usPosY, QUEST_DBS_STATUS_COL_WIDTH, 2, QUEST_DBS_FONT_STATIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, QuestStates[gubQuest[usLoop1]], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

		usPosY += usTextHeight;
		usCount++;
	}
}


static void DisplayFactList(void)
{
	UINT16	usLoop1, usCount;
	UINT16	usTextHeight = GetFontHeight( QUEST_DBS_FONT_DYNAMIC_TEXT ) + 2;
	ST::string sTemp;
	UINT16	usPosY;

	usPosY = QUEST_DBS_SECOND_COL_NUMBER_Y + QUEST_DBS_LIST_TEXT_OFFSET + QUEST_DBS_FACT_LIST_OFFSET;	//

	if( gusFactAtTopOfList+QUEST_DBS_NUM_DISPLAYED_FACTS > NUM_FACTS )
		gusFactAtTopOfList = NUM_FACTS - QUEST_DBS_NUM_DISPLAYED_FACTS;


	for( usLoop1=gusFactAtTopOfList, usCount=0; (usLoop1<NUM_FACTS) && (usCount < QUEST_DBS_NUM_DISPLAYED_FACTS ); usLoop1++)
	{
		//Display Quest Number text
		sTemp = ST::format("{02d}", usLoop1);
		DrawTextToScreen(sTemp, QUEST_DBS_SECOND_COL_NUMBER_X, usPosY, QUEST_DBS_NUMBER_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

		//Display Quest title text
		if( FactDescText[ usLoop1 ][0] == '\0' )
		{
			sTemp = ST::format("No Fact {03d} Yet", usLoop1);
			DisplayWrappedString(QUEST_DBS_SECOND_COL_TITLE_X, usPosY, QUEST_DBS_SECOND_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, sTemp, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
		}
		else
		{
			sTemp = FactDescText[ usLoop1 ];
			sTemp = ReduceStringLength(sTemp, QUEST_DBS_SECOND_TITLE_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT);
//			DisplayWrappedString(QUEST_DBS_SECOND_COL_TITLE_X, usPosY, QUEST_DBS_SECOND_TITLE_COL_WIDTH, 2, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_STATIC_TEXT, FactDescText[usLoop1], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
			DrawTextToScreen(sTemp, QUEST_DBS_SECOND_COL_TITLE_X, usPosY, QUEST_DBS_SECOND_TITLE_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
		}

		DrawTextToScreen(gubFact[usLoop1] ? "True" : "False", QUEST_DBS_SECOND_COL_STATUS_X, usPosY, QUEST_DBS_STATUS_COL_WIDTH, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

		usPosY += usTextHeight;
		usCount++;
	}
}


static void BtnQuestDebugCurNPCButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//if there is an old list box active, destroy the new one
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
		CreateDestroyDisplaySelectNpcDropDownBox();
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;


		//Set up the global list box
		gpActiveListBox = &gNpcListBox;

		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_CREATE;
	}
}


static void BtnQuestDebugCurItemButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//if there is an old list box active, destroy the new one
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
		CreateDestroyDisplaySelectNpcDropDownBox();
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;


		//Set up the global list box
		gpActiveListBox = &gItemListBox;

		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_CREATE;
	}
}


static void DisplayNPCInfo(void)
{
	//display section title
	DisplayWrappedString(QUEST_DBS_THIRD_COL_TITLE_X, QUEST_DBS_SECTION_TITLE_Y, QUEST_DBS_THIRD_SECTION_WIDTH, 2, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_SUBTITLE, QuestDebugText[QUEST_DBS_NPC_INFO], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
}


static void DisplaySelectedListBox(void);
static void QuestDebugTextEntryDisableScreenRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void ScrollAreaMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason);
static void ScrollAreaRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void ScrollArrowsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectNpcListMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason);
static void SelectNpcListRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static BOOLEAN CreateDestroyDisplaySelectNpcDropDownBox(void)
{
	static	BOOLEAN fMouseRegionsCreated=FALSE;
	UINT16	i;
	UINT16	usPosX, usPosY;

	//if there are
	if( gpActiveListBox->usMaxArrayIndex == 0 )
		return( FALSE );

	switch( gpActiveListBox->ubCurScrollBoxAction )
	{
		case 	QD_DROP_DOWN_NO_ACTION:
		{

		}
		break;

		case 	QD_DROP_DOWN_CREATE:
		{
			UINT16	usFontHeight = GetFontHeight( QUEST_DBS_FONT_LISTBOX_TEXT ) + 2;

			//if the mouse regions have already been creates, return
			if( fMouseRegionsCreated )
				break;

			usPosX = gpActiveListBox->usScrollPosX;
			usPosY = gpActiveListBox->usScrollPosY;


			// create the scroll regions
			for( i=0; i< gpActiveListBox->usNumDisplayedItems; i++)
			{
				MSYS_DefineRegion(&gSelectedNpcListRegion[i], usPosX, usPosY, usPosX + gpActiveListBox->usScrollWidth, usPosY + usFontHeight, MSYS_PRIORITY_HIGH + 20, CURSOR_WWW, SelectNpcListMovementCallBack, SelectNpcListRegionCallBack);
				MSYS_SetRegionUserData( &gSelectedNpcListRegion[ i ], 0, i);

				usPosY += usFontHeight;
			}

			fMouseRegionsCreated = TRUE;

			//Scroll bars
			usPosX = gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth;
			usPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollArrowHeight + 2 ;

			for(i=0; i<QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR; i++ )
			{
				MSYS_DefineRegion( &gScrollAreaRegion[i], usPosX, usPosY, (UINT16)(usPosX+gpActiveListBox->usScrollBarWidth), (UINT16)(usPosY + gpActiveListBox->usScrollBarHeight ), MSYS_PRIORITY_HIGH+20,
							CURSOR_WWW, ScrollAreaMovementCallBack, ScrollAreaRegionCallBack);
				MSYS_SetRegionUserData( &gScrollAreaRegion[ i ], 0, i );
			}

			//Top Scroll arrow
			usPosX = gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth;
			usPosY = gpActiveListBox->usScrollPosY + 2;

			MSYS_DefineRegion( &gScrollArrowsRegion[0], usPosX, (UINT16)(usPosY), (UINT16)(usPosX+gpActiveListBox->usScrollBarWidth), (UINT16)(usPosY+gpActiveListBox->usScrollArrowHeight), MSYS_PRIORITY_HIGH+20,
						CURSOR_WWW, MSYS_NO_CALLBACK, ScrollArrowsRegionCallBack);
			MSYS_SetRegionUserData( &gScrollArrowsRegion[ 0 ], 0, 0 );

			//Bottom Scroll arrow
			usPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollHeight - gpActiveListBox->usScrollArrowHeight - 2;

			MSYS_DefineRegion( &gScrollArrowsRegion[1], usPosX, usPosY, (UINT16)(usPosX+gpActiveListBox->usScrollBarWidth), (UINT16)(usPosY + gpActiveListBox->usScrollArrowHeight), MSYS_PRIORITY_HIGH+20,
						CURSOR_WWW, MSYS_NO_CALLBACK, ScrollArrowsRegionCallBack);
			MSYS_SetRegionUserData( &gScrollArrowsRegion[ 1 ], 0, 1 );


			//create a mask to block out the screen
			if( !gfBackgroundMaskEnabled )
			{
				MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH + 15, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, QuestDebugTextEntryDisableScreenRegionCallBack);
				gfBackgroundMaskEnabled = TRUE;
			}


			gfInDropDownBox = TRUE;

			if( gpActiveListBox->sCurSelectedItem == -1 )
			{
				gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usStartIndex;
				gpActiveListBox->sCurSelectedItem = gpActiveListBox->usStartIndex;
			}
			else
				gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->sCurSelectedItem;

		}
		break;

		case 	QD_DROP_DOWN_DESTROY:
		{
			//if the mouse regions are creates, destroy them
			if( fMouseRegionsCreated )
			{
				// delete the mouse regions for the words
				for( i=0; i< gpActiveListBox->usNumDisplayedItems; i++)
					MSYS_RemoveRegion( &gSelectedNpcListRegion[i]);

				fMouseRegionsCreated = FALSE;

				//scroll arrows
				for( i=0; i< 2; i++)
					MSYS_RemoveRegion( &gScrollArrowsRegion[i]);

				for(i=0; i<QUEST_DBS_NUM_INCREMENTS_IN_SCROLL_BAR; i++ )
				{
					MSYS_RemoveRegion( &gScrollAreaRegion[i] );
				}

				//remove the mask of the entire screen
				if( gfBackgroundMaskEnabled )
				{
					MSYS_RemoveRegion( &gQuestTextEntryDebugDisableScreenRegion);
					gfBackgroundMaskEnabled = FALSE;
				}


				EnableQDSButtons();
			}
			gfRedrawQuestDebugSystem = TRUE;
			gfInDropDownBox = FALSE;
		}
		break;

		case 	QD_DROP_DOWN_DISPLAY:
		{
//			( *(gpActiveListBox->DisplayFunction))();
//			(*(MSYS_CurrRegion->ButtonCallback))(MSYS_CurrRegion,ButtonReason);
			DisplaySelectedListBox( );
		}
		break;

	}
	return( TRUE );
}


static void DrawQdsScrollRectangle(void);


static void DisplaySelectedListBox(void)
{
	UINT16	usPosX, usPosY;

	//DEBUG: make sure it wont go over array bounds
	if( gpActiveListBox->usMaxArrayIndex == 0 )
	{
		return;
	}
	else
	{
		if( gpActiveListBox->sCurSelectedItem >= gpActiveListBox->usMaxArrayIndex )
		{
			if( gpActiveListBox->usMaxArrayIndex > 0 )
				gpActiveListBox->sCurSelectedItem = gpActiveListBox->usMaxArrayIndex - 1;
			else
				gpActiveListBox->sCurSelectedItem = 0;

			if( (INT16)( gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usNumDisplayedItems -1 ) < 0 )
				gpActiveListBox->usItemDisplayedOnTopOfList = 0;
			else
				gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usNumDisplayedItems - 1;
		}
		else if( !gfUseLocalNPCs && ( ( gpActiveListBox->usItemDisplayedOnTopOfList + gpActiveListBox->usMaxNumDisplayedItems ) >= gpActiveListBox->usMaxArrayIndex ) )
		{
			gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems - 1;
		}
	}

	usPosX = gpActiveListBox->usScrollPosX;
	usPosY = gpActiveListBox->usScrollPosY + 2;

	//clear the background
	ColorFillVideoSurfaceArea( FRAME_BUFFER, usPosX, usPosY-1, usPosX+gpActiveListBox->usScrollWidth,	usPosY + gpActiveListBox->usScrollHeight, Get16BPPColor( FROMRGB(  45,  59,  74 ) ) );


	//Display the selected list box's display function
	( *(gpActiveListBox->DisplayFunction))();

	//Display the Scroll BAr area
	//clear the scroll bar background
	usPosX = gpActiveListBox->usScrollPosX+gpActiveListBox->usScrollWidth;
	usPosY = gpActiveListBox->usScrollPosY + 2;

	ColorFillVideoSurfaceArea( FRAME_BUFFER, usPosX, usPosY-1, usPosX+gpActiveListBox->usScrollBarWidth,	usPosY+ gpActiveListBox->usScrollHeight, Get16BPPColor( FROMRGB(  192,  192,  192 ) ) );


	// display the up and down arrows
	//top arrow
	BltVideoObject(FRAME_BUFFER, guiQdScrollArrowImage, 0, usPosX-5, usPosY-1);

	//Bottom arrow
	BltVideoObject(FRAME_BUFFER, guiQdScrollArrowImage, 1, usPosX, usPosY + gpActiveListBox->usScrollHeight - gpActiveListBox->usScrollArrowHeight);

	//display the scroll rectangle
	DrawQdsScrollRectangle();

	InvalidateScreen();
}


static ST::string GetDebugLocationString(UINT16 usProfileID);


static void DisplaySelectedNPC(void)
{
	UINT16	i;
	UINT16	usPosX, usPosY;
	INT16   usLocationX;
	INT16   usLocationY;
	UINT16	usFontHeight = GetFontHeight( QUEST_DBS_FONT_LISTBOX_TEXT ) + 2;
	ST::string sTempString;
	ST::string zButtonName;



	usPosX = gpActiveListBox->usScrollPosX;
	usPosY = gpActiveListBox->usScrollPosY + 2;

	// display the names of the NPC's
	for( i=gpActiveListBox->usItemDisplayedOnTopOfList; i< gpActiveListBox->usItemDisplayedOnTopOfList+gpActiveListBox->usNumDisplayedItems; i++)
	{
		ProfileID         const  pid = (gfUseLocalNPCs ? gubCurrentNpcInSector[i] : i);
		MERCPROFILESTRUCT const& p   = GetProfile(pid);
		DrawTextToScreen(p.zNickname, usPosX, usPosY, 0, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		sTempString = GetDebugLocationString(pid);

		FindFontRightCoordinates( gpActiveListBox->usScrollPosX, usPosY, gpActiveListBox->usScrollWidth, 0, sTempString, QUEST_DBS_FONT_LISTBOX_TEXT, &usLocationX, &usLocationY );

		// the location value
		DrawTextToScreen(sTempString, usLocationX - 2, usPosY, 0, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

		usPosY += usFontHeight;
	}

	//if there is a selected item, highlight it.
	if( gpActiveListBox->sCurSelectedItem >= 0 )
	{
		usPosY = usFontHeight * ( gpActiveListBox->sCurSelectedItem - gpActiveListBox->usItemDisplayedOnTopOfList ) + gpActiveListBox->usScrollPosY + 2;

		//display the name in the list
		ColorFillVideoSurfaceArea( FRAME_BUFFER, gpActiveListBox->usScrollPosX, usPosY - 1, gpActiveListBox->usScrollPosX+gpActiveListBox->usScrollWidth,	usPosY+usFontHeight - 1, Get16BPPColor( FROMRGB( 255, 255, 255 ) ) );

		SetFontShadow(NO_SHADOW);

		ProfileID         const  pid = (gfUseLocalNPCs ? gubCurrentNpcInSector[gpActiveListBox->sCurSelectedItem] : gpActiveListBox->sCurSelectedItem);
		MERCPROFILESTRUCT const& p   = GetProfile(pid);

		// the highlighted name
		DrawTextToScreen(p.zNickname, gpActiveListBox->usScrollPosX, usPosY, 0, QUEST_DBS_FONT_LISTBOX_TEXT, 2, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		sTempString = GetDebugLocationString(pid);

		FindFontRightCoordinates(gpActiveListBox->usScrollPosX, usPosY, gpActiveListBox->usScrollWidth, 0, sTempString, QUEST_DBS_FONT_LISTBOX_TEXT, &usLocationX, &usLocationY);

		// the location value
		DrawTextToScreen(sTempString, usLocationX, usPosY, 0, QUEST_DBS_FONT_LISTBOX_TEXT, 2, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

		SetFontShadow(DEFAULT_SHADOW);

		zButtonName = ST::format("{} - {}", pid, p.zNickname);

		guiQuestDebugCurNPCButton->SpecifyText(zButtonName);
	}

	SetFontShadow( DEFAULT_SHADOW );
}


static void DisplaySelectedItem(void)
{
	UINT16	i;
	UINT16	usPosX, usPosY;
	UINT16	usFontHeight = GetFontHeight( QUEST_DBS_FONT_LISTBOX_TEXT ) + 2;

	ST::string zButtonName;


	usPosX = gpActiveListBox->usScrollPosX;
	usPosY = gpActiveListBox->usScrollPosY + 2;

		// display the names of the NPC's
	for( i=gpActiveListBox->usItemDisplayedOnTopOfList; i< gpActiveListBox->usItemDisplayedOnTopOfList+gpActiveListBox->usNumDisplayedItems; i++)
	{
		auto ItemName = GCM->getItem(i)->getShortName();
		if (ItemName.empty()) ItemName = QuestDebugText[QUEST_DBS_NO_ITEM];

		DrawTextToScreen(ItemName, usPosX, usPosY, 0, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		usPosY += usFontHeight;
	}

	//if there is a selected item, highlight it.
	if( gpActiveListBox->sCurSelectedItem >= 0 )
	{
		usPosY = usFontHeight * ( gpActiveListBox->sCurSelectedItem - gpActiveListBox->usItemDisplayedOnTopOfList ) + gpActiveListBox->usScrollPosY + 2;

		//display the name in the list
		ColorFillVideoSurfaceArea( FRAME_BUFFER, gpActiveListBox->usScrollPosX, usPosY - 1, gpActiveListBox->usScrollPosX+gpActiveListBox->usScrollWidth,	usPosY+usFontHeight - 1, Get16BPPColor( FROMRGB( 255, 255, 255 ) ) );

		SetFontShadow(NO_SHADOW);

		ST::string ItemName = GCM->getItem(gpActiveListBox->sCurSelectedItem)->getShortName();
		if (ItemName.empty()) ItemName = QuestDebugText[QUEST_DBS_NO_ITEM];

		DrawTextToScreen(ItemName, gpActiveListBox->usScrollPosX, usPosY, 0, QUEST_DBS_FONT_LISTBOX_TEXT, 2, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		SetFontShadow(DEFAULT_SHADOW);

		zButtonName = ST::format("{} - {}", gpActiveListBox->sCurSelectedItem, ItemName);
		guiQuestDebugCurItemButton->SpecifyText(zButtonName);
	}

	SetFontShadow(DEFAULT_SHADOW);

}


static void SelectNpcListRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT8 ubSelected = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

		gpActiveListBox->ubCurScrollBoxAction	= QD_DROP_DOWN_DESTROY;
		gpActiveListBox->sCurSelectedItem			=	ubSelected + gpActiveListBox->usItemDisplayedOnTopOfList;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
		CreateDestroyDisplaySelectNpcDropDownBox();
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
	}
}


static void SelectNpcListMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		INT16	sSelected = (INT16)MSYS_GetRegionUserData( pRegion, 0 );// + gubCityAtTopOfList;

		gpActiveListBox->sCurSelectedItem = sSelected + gpActiveListBox->usItemDisplayedOnTopOfList;

		//if we are at the top of the list
//		if( sSelected == 0 )
//			IncrementActiveDropDownBox( (INT16)(gpActiveListBox->sCurSelectedItem - 1 ) );

		//else we are at the bottom of the list
//		else if( sSelected == gpActiveListBox->usMaxNumDisplayedItems - 1 )
//			IncrementActiveDropDownBox( (INT16)(gpActiveListBox->sCurSelectedItem + 1 ) );


		DisplaySelectedListBox( );

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_MOVE )
	{
		INT16	sSelected = (INT16)MSYS_GetRegionUserData( pRegion, 0 );// + gubCityAtTopOfList;

		if( gpActiveListBox->sCurSelectedItem != ( sSelected + gpActiveListBox->usItemDisplayedOnTopOfList ))
		{
			gpActiveListBox->sCurSelectedItem = sSelected + gpActiveListBox->usItemDisplayedOnTopOfList;

			DisplaySelectedListBox( );
		}

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}

}


static void DrawQdsScrollRectangle(void)
{
	UINT16 usWidth, usTempPosY;
	UINT16	usHeight, usPosY, usPosX;

	UINT16	usNumEntries = gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usStartIndex - 1;


	UINT16 temp;

	usTempPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollArrowHeight;
	usPosX = gpActiveListBox->usScrollPosX + gpActiveListBox->usScrollWidth;
	usWidth = gpActiveListBox->usScrollBarWidth;

	usHeight = (UINT16)(gpActiveListBox->usScrollBarHeight / (FLOAT)( usNumEntries ) + .5 );//qq+ 1 );

	if( usNumEntries > gpActiveListBox->usMaxNumDisplayedItems )
		usPosY = usTempPosY + (UINT16)( ( gpActiveListBox->usScrollBarHeight /  (FLOAT)(usNumEntries +1) ) * ( gpActiveListBox->sCurSelectedItem - gpActiveListBox->usStartIndex ) );
	else
		usPosY = usTempPosY;

	//bottom
	temp = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollBarHeight + gpActiveListBox->usScrollArrowHeight;

	if( usPosY >= temp )
		usPosY = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollBarHeight + gpActiveListBox->usScrollArrowHeight - usHeight;

	gpActiveListBox->usScrollBoxY = usPosY;
	gpActiveListBox->usScrollBoxEndY = usPosY + usHeight;

	ColorFillVideoSurfaceArea( FRAME_BUFFER, usPosX, usPosY, usPosX+usWidth-1,	usPosY+usHeight, Get16BPPColor( FROMRGB( 130, 132, 128 ) ) );

	//display the line
	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf = l.Buffer<UINT16>();
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// draw the gold highlite line on the top and left
	LineDraw(FALSE, usPosX, usPosY, usPosX+usWidth-1, usPosY, Get16BPPColor( FROMRGB( 255, 255, 255 ) ), pDestBuf);
	LineDraw(FALSE, usPosX, usPosY, usPosX, usPosY+usHeight, Get16BPPColor( FROMRGB( 255, 255, 255 ) ), pDestBuf);

	// draw the shadow line on the bottom and right
	LineDraw(FALSE, usPosX, usPosY+usHeight, usPosX+usWidth-1, usPosY+usHeight, Get16BPPColor( FROMRGB( 112, 110, 112 ) ), pDestBuf);
	LineDraw(FALSE, usPosX+usWidth-1, usPosY, usPosX+usWidth-1, usPosY+usHeight, Get16BPPColor( FROMRGB( 112, 110, 112 ) ), pDestBuf);
}


static void ScrollArrowsRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP || iReason & MSYS_CALLBACK_REASON_POINTER_REPEAT)
	{
		UINT8 ubSelected = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

		//if down arrow
		if( ubSelected )
		{
			//if not at end of list
			if( gpActiveListBox->sCurSelectedItem < gpActiveListBox->usMaxArrayIndex-1 )
				gpActiveListBox->sCurSelectedItem ++;

			//if end of displayed list, increment top of list
			if( ( gpActiveListBox->sCurSelectedItem - gpActiveListBox->usItemDisplayedOnTopOfList ) >= gpActiveListBox->usNumDisplayedItems )
				gpActiveListBox->usItemDisplayedOnTopOfList++;
		}

		//else, up arrow
		else
		{
			//if not at end of list
			if( gpActiveListBox->sCurSelectedItem > gpActiveListBox->usStartIndex )
				gpActiveListBox->sCurSelectedItem --;

			//if top of displayed list
			if( gpActiveListBox->sCurSelectedItem < gpActiveListBox->usItemDisplayedOnTopOfList )
				gpActiveListBox->usItemDisplayedOnTopOfList--;
		}

		DisplaySelectedListBox( );
	}
}


static void CalcPositionOfNewScrollBoxLocation(void);


static void ScrollAreaRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & (MSYS_CALLBACK_REASON_POINTER_UP | MSYS_CALLBACK_REASON_POINTER_REPEAT))
	{
		CalcPositionOfNewScrollBoxLocation();
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
		CreateDestroyDisplaySelectNpcDropDownBox();
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
	}
}


static void ScrollAreaMovementCallBack(MOUSE_REGION* pRegion, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
//		CalcPositionOfNewScrollBoxLocation();

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
//		CalcPositionOfNewScrollBoxLocation();

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_MOVE )
	{
		if( IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMainFingerDown() )
		{
			CalcPositionOfNewScrollBoxLocation();
		}

		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
}


static void CalcPositionOfNewScrollBoxLocation(void)
{
	INT16 sMouseYPos;
	INT16	sIncrementValue;
	FLOAT	dValue;
	INT16	sHeight=0;
//	INT16	sHeightOfScrollBox = (INT16)(gpActiveListBox->usScrollBarHeight / (FLOAT)(gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usStartIndex ) + .5);
	//INT16	sHeightOfScrollBox = (INT16)(gpActiveListBox->usScrollBarHeight / (FLOAT)(gpActiveListBox->usMaxArrayIndex ) + .5);
	INT16	sStartPosOfScrollArea = gpActiveListBox->usScrollPosY + gpActiveListBox->usScrollArrowHeight;

	sMouseYPos = gusMouseYPos;

	//if we have to scroll
	if( sMouseYPos > sStartPosOfScrollArea || sMouseYPos < ( sStartPosOfScrollArea + gpActiveListBox->usScrollBarHeight ) )
	{
		//Calculate the number of items we have to move
		sHeight = sMouseYPos - sStartPosOfScrollArea;


		dValue = sHeight / (FLOAT)( gpActiveListBox->usScrollBarHeight );
		sIncrementValue = (INT16)( ( dValue ) * ( gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usStartIndex ) + .5 ) + gpActiveListBox->usStartIndex;
//		sIncrementValue = (INT16)( ( dValue ) * ( gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usStartIndex ) + .5 );

		IncrementActiveDropDownBox( sIncrementValue );
/*
		//if the mouse was clicked above the scroll box
		if( sIncrementValue < gpActiveListBox->sCurSelectedItem )
		{
			if( ( gpActiveListBox->usItemDisplayedOnTopOfList - sIncrementValue ) <= 0 )
				gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usStartIndex;
			else
				gpActiveListBox->usItemDisplayedOnTopOfList = sIncrementValue;

		}
		// else the mouse was clicked below the scroll box
		else
		{
			if( sIncrementValue >= ( gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems ) )
				gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems;
			else
				gpActiveListBox->usItemDisplayedOnTopOfList = sIncrementValue;
		}

		gpActiveListBox->sCurSelectedItem = sIncrementValue;
*/
	}

/*
	if( sMouseYPos < gpActiveListBox->usScrollBoxY )
	{
		if( ( gpActiveListBox->sCurSelectedItem - 10 ) > 0 )
			sIncrementValue = 10;
		else
			sIncrementValue = 1;

		gpActiveListBox->sCurSelectedItem -= sIncrementValue;

		//if we dont have to scroll,
		if( gpActiveListBox->usNumDisplayedItems < gpActiveListBox->usMaxNumDisplayedItems )
		{

		}
		else
		{
			if( gpActiveListBox->sCurSelectedItem < gpActiveListBox->usItemDisplayedOnTopOfList )
				gpActiveListBox->usItemDisplayedOnTopOfList -= sIncrementValue;
		}
	}
	else if( sMouseYPos > ( gpActiveListBox->usScrollBoxY + sHeightOfScrollBox ) )//usScrollBoxEndY
	{
		if( ( gpActiveListBox->sCurSelectedItem + 10 ) < gpActiveListBox->usMaxArrayIndex-1 )
			sIncrementValue = 10;
		else
			sIncrementValue = 1;

		gpActiveListBox->sCurSelectedItem += sIncrementValue;

		//if we dont have to scroll,
		if( gpActiveListBox->usNumDisplayedItems < gpActiveListBox->usMaxNumDisplayedItems )
		{

		}
		else
		{
			if( ( gpActiveListBox->sCurSelectedItem - gpActiveListBox->usItemDisplayedOnTopOfList ) >= gpActiveListBox->usNumDisplayedItems )
				gpActiveListBox->usItemDisplayedOnTopOfList += sIncrementValue;
		}
	}
*/

	DisplaySelectedListBox( );
}


static void AddNPCToGridNo(INT32 iGridNo);


static void BtnQuestDebugAddNpcToLocationButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		ST::string zTemp = ST::format("{} where {} will be added.", QuestDebugText[QUEST_DBS_ENTER_GRID_NUM], gMercProfiles[gNpcListBox.sCurSelectedItem].zNickname);
		TextEntryBox( zTemp, AddNPCToGridNo );
	}
}


static void AddItemToGridNo(INT32 iGridNo);


static void BtnQuestDebugAddItemToLocationButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		ST::string zTemp = ST::format("{} where the {} will be added.", QuestDebugText[QUEST_DBS_ENTER_GRID_NUM], GCM->getItem(gItemListBox.sCurSelectedItem)->getShortName());
		TextEntryBox( zTemp, AddItemToGridNo );
	}
}


static void BtnQuestDebugGiveItemToNPCButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		OBJECTTYPE		Object;

		CreateItem( gItemListBox.sCurSelectedItem, 100, &Object );

		//if the selected merc is created
		const ProfileID    pid = (gfUseLocalNPCs ? gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem] : gNpcListBox.sCurSelectedItem);
		SOLDIERTYPE* const s   = FindSoldierByProfileID(pid);
		if (!s)
		{
			//Failed to get npc, put error message
			return;
		}

		//Give the selected item to the selected merc
		if (!AutoPlaceObject(s, &Object, TRUE))
		{
			//failed to add item, put error message to screen
		}
	}
}


static void ChangeDayNumber(INT32 iDayToChangeTo);


static void BtnQuestDebugChangeDayButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		ST::string zTemp = ST::format("{}   Current Day is {}", QuestDebugText[QUEST_DBS_PLEASE_ENTER_DAY], GetWorldDay());

		//get the day to change the game day to
		TextEntryBox( zTemp, ChangeDayNumber );
	}
}


static void BtnQuestDebugViewNPCInvButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubNPCInventoryPopupAction = QD_DROP_DOWN_CREATE;
	}
}


static void RefreshAllNPCInventory(void);


static void BtnQuestDebugRestoreNPCInvButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//loop through all the active NPC's and refresh their inventory
		RefreshAllNPCInventory();
	}
}


static void BtnQuestDebugNPCLogButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		ST::string zName;

		gfNpcLogButton = !gfNpcLogButton;
		if (gfNpcLogButton)
		{
			btn->uiFlags |= BUTTON_CLICKED_ON;
		}
		else
		{
			btn->uiFlags &= ~BUTTON_CLICKED_ON;
		}

		zName = ST::format("{} - ({})", QuestDebugText[QUEST_DBS_NPC_LOG_BUTTON], gfNpcLogButton ? "On" : "Off");
		btn->SpecifyText(zName);
	}
}


static void BtnQuestDebugNPCRefreshButtonButtonCallback(GUI_BUTTON* const btn, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		bool  success  = false;
		UINT8 ubMercID = 0;
		if (gfUseLocalNPCs)
		{
			ubMercID = gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem];
			success  = ReloadQuoteFile(ubMercID);
		}
		else if (gNpcListBox.sCurSelectedItem != -1)
		{
			// NB ubMercID is really profile ID
			ubMercID = (UINT8)gNpcListBox.sCurSelectedItem;
			success  = ReloadQuoteFile(ubMercID);
			gMercProfiles[ubMercID].ubLastDateSpokenTo = 0;
		}

		ST::string msg =
			success ? QuestDebugText[QUEST_DBS_REFRESH_OK] :
			QuestDebugText[QUEST_DBS_REFRESH_FAILED];
		ST::string buf = ST::format("{} {}", msg, gMercProfiles[ubMercID].zNickname);
		DoQDSMessageBox(buf, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK, NULL);
	}
}


static void StartMercTalkingFromQuoteNum(INT32 iQuoteToStartTalkingFrom);


static void BtnQuestDebugStartMercTalkingButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//Ask for the initial quote num to start talking from
//		DoQDSMessageBox(zTemp, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK, NULL);

		//set the initial value
		gsQdsEnteringGridNo = 0;

		TextEntryBox( QuestDebugText[ QUEST_DBS_START_MERC_TALKING_FROM ], StartMercTalkingFromQuoteNum );
	}
}


static void BtnQuestDebugTextEntryOkBtnButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void DestroyQuestDebugTextInputBoxes(void);
static void InitQuestDebugTextInputBoxes(void);


static void CreateDestroyDisplayTextEntryBox(UINT8 ubAction, const ST::string& pString, TEXT_ENTRY_CALLBACK EntryCallBack)
{
	static BOOLEAN	fMouseRegionCreated = FALSE;
	static ST::string zString;
	static TEXT_ENTRY_CALLBACK TextEntryCallback;

	switch( ubAction )
	{
		case QD_DROP_DOWN_NO_ACTION:
		{
		}
		break;

		case QD_DROP_DOWN_CREATE:
		{
			if( fMouseRegionCreated )
				break;

			fMouseRegionCreated = TRUE;

			//create a mask to block out the screen
			if( !gfBackgroundMaskEnabled )
			{
				MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH + 40, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, QuestDebugTextEntryDisableScreenRegionCallBack);
				gfBackgroundMaskEnabled = TRUE;
			}

			//create the ok button
			guiQuestDebugTextEntryOkBtn = MakeButton("OK", QUEST_DBS_TEB_X + QUEST_DBS_TEB_WIDTH / 2 - 12, QUEST_DBS_TEB_Y + QUEST_DBS_TEB_HEIGHT - 30, 30, 25, MSYS_PRIORITY_HIGH + 50, BtnQuestDebugTextEntryOkBtnButtonCallback);
			guiQuestDebugTextEntryOkBtn->SetCursor(CURSOR_WWW);

			zString = pString;

			gfTextEntryActive = TRUE;

			InitQuestDebugTextInputBoxes();

			TextEntryCallback = EntryCallBack;
		}
		break;

		case QD_DROP_DOWN_CANCEL:
		case QD_DROP_DOWN_DESTROY:
		{
			INT32		iTextEntryNumber;

			if( !fMouseRegionCreated )
				break;

			//Remove the mouse region that disables the screen
			if( gfBackgroundMaskEnabled )
			{
				MSYS_RemoveRegion( &gQuestTextEntryDebugDisableScreenRegion);
				gfBackgroundMaskEnabled = FALSE;
			}

			//remove the 'ok' button on the text entry field
			RemoveButton( guiQuestDebugTextEntryOkBtn );

			// Mouse regions are removed
			fMouseRegionCreated = FALSE;
			gfTextEntryActive = FALSE;

			//redraw the entire screen
			gfRedrawQuestDebugSystem = TRUE;

			ST::string zText = GetStringFromField(0);
			iTextEntryNumber = 0;
			if( zText[0] != '\0' )
			{
				//get the number from the string
				sscanf( zText.c_str(), "%d", &iTextEntryNumber );
			}



			// remove the text input field
			DestroyQuestDebugTextInputBoxes();

			if( ubAction != QD_DROP_DOWN_CANCEL )
				( *(TextEntryCallback))( iTextEntryNumber );

		}
		break;

		case QD_DROP_DOWN_DISPLAY:
		{
			//Display the text entry box frame
			ColorFillVideoSurfaceArea( FRAME_BUFFER, QUEST_DBS_TEB_X, QUEST_DBS_TEB_Y, QUEST_DBS_TEB_X+QUEST_DBS_TEB_WIDTH,	QUEST_DBS_TEB_Y+QUEST_DBS_TEB_HEIGHT, Get16BPPColor( FROMRGB(  45,  59,  74 ) ) );

			//Display the text box caption
			DisplayWrappedString(QUEST_DBS_TEB_X + 10, QUEST_DBS_TEB_Y + 10, QUEST_DBS_TEB_WIDTH - 20, 2, QUEST_DBS_FONT_TEXT_ENTRY, QUEST_DBS_COLOR_TEXT_ENTRY, zString, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

			InvalidateRegion( QUEST_DBS_TEB_X, QUEST_DBS_TEB_Y, QUEST_DBS_TEB_X+QUEST_DBS_TEB_WIDTH,	QUEST_DBS_TEB_Y+QUEST_DBS_TEB_HEIGHT );
		}
		break;
	}
}


static void QuestDebugTextEntryDisableScreenRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
		CreateDestroyDisplaySelectNpcDropDownBox();
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
	}
	else if(iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DESTROY;
		CreateDestroyDisplaySelectNpcDropDownBox();
		gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_NO_ACTION;
	}
}


static void BtnQuestDebugTextEntryOkBtnButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubTextEntryAction = QD_DROP_DOWN_DESTROY;
	}
}


static void TextEntryBox(const ST::string& pString, TEXT_ENTRY_CALLBACK TextEntryCallBack)
{
	CreateDestroyDisplayTextEntryBox( QD_DROP_DOWN_CREATE, pString, TextEntryCallBack );
	gubTextEntryAction = QD_DROP_DOWN_DISPLAY;
}


static void ChangeQuestState(INT32 iNumber);


static void ScrollQuestListRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		ST::string String;

		gubCurQuestSelected = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );

		String = ST::format("{} {} {} \"{}\" ( {} )", QuestDebugText[QUEST_DBS_ENTER_NEW_VALUE], QuestDebugText[QUEST_DBS_QUEST_NUM], gubCurQuestSelected, QuestDescText[gubCurQuestSelected], QuestDebugText[QUEST_DBS_0_1_2]);

		TextEntryBox( String, ChangeQuestState );
	}
}


static void ChangeFactState(INT32 iNumber);


static void ScrollFactListRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		ST::string String;

		gusCurFactSelected = (UINT8)MSYS_GetRegionUserData( pRegion, 0 ) + gusFactAtTopOfList;

		if( FactDescText[ gusCurFactSelected ][0] == '\0' )
			String = ST::format("{} {} {} \"{}\" ( {} )", QuestDebugText[QUEST_DBS_ENTER_NEW_VALUE], QuestDebugText[QUEST_DBS_FACT_NUM], gusCurFactSelected, QuestDebugText[QUEST_DBS_NO_TEXT], QuestDebugText[QUEST_DBS_0_1]);
		else
			String = ST::format("{} {} {} \"{}\" ( {} )", QuestDebugText[QUEST_DBS_ENTER_NEW_VALUE], QuestDebugText[QUEST_DBS_FACT_NUM], gusCurFactSelected, FactDescText[gusCurFactSelected], QuestDebugText[QUEST_DBS_0_1]);

		TextEntryBox( String, ChangeFactState );
	}
}


static void InitQuestDebugTextInputBoxes(void)
{
	InitTextInputMode();
	SetTextInputFont(FONT12ARIAL);
	Set16BPPTextFieldColor( Get16BPPColor(FROMRGB( 255, 255, 255) ) );
	SetBevelColors( Get16BPPColor(FROMRGB(136, 138, 135)), Get16BPPColor(FROMRGB(24, 61, 81)) );
	SetTextInputRegularColors( 2, FONT_WHITE );
	SetTextInputHilitedColors( FONT_WHITE, 2, 141  );
	SetCursorColor( Get16BPPColor(FROMRGB(0, 0, 0) ) );

	ST::string sTemp = ST::format("{}", gsQdsEnteringGridNo);

	//Text entry field
	AddTextInputField( QUEST_DBS_TEB_X+QUEST_DBS_TEB_WIDTH/2-30, QUEST_DBS_TEB_Y+65, 60, 15, MSYS_PRIORITY_HIGH+60, sTemp, QUEST_DBS_TEXT_FIELD_WIDTH, INPUTTYPE_NUMERICSTRICT );
}


static void DestroyQuestDebugTextInputBoxes(void)
{
	KillTextInputMode();
}


static void AddNPCToGridNo(INT32 iGridNo)
{
	SOLDIERCREATE_STRUCT		MercCreateStruct;

	MercCreateStruct = SOLDIERCREATE_STRUCT{};
	MercCreateStruct.bTeam				= CIV_TEAM;
	MercCreateStruct.ubProfile		= (UINT8)gpActiveListBox->sCurSelectedItem;
	MercCreateStruct.sSector			= gWorldSector;
	MercCreateStruct.sInsertionGridNo		= (UINT16) iGridNo;

//	RandomizeNewSoldierStats( &MercCreateStruct );

	SOLDIERTYPE* const s = TacticalCreateSoldier(MercCreateStruct);
	if (s != NULL)
	{
		AddSoldierToSector(s);

		// So we can see them!
		AllTeamsLookForAll(NO_INTERRUPTS);
	}

	//Add all the npc in the current sectory the npc array
	AddNPCsInSectorToArray();

	gsQdsEnteringGridNo = (INT16)iGridNo;
}


static void AddItemToGridNo(INT32 iGridNo)
{
	OBJECTTYPE		Object;

	gsQdsEnteringGridNo = (INT16)iGridNo;


	if( GCM->getItem(gItemListBox.sCurSelectedItem)->getItemClass() == IC_KEY )
	{
		gfAddKeyNextPass = TRUE;
//		ST::string zTemp = "Please enter the Key ID";
//		TextEntryBox( zTemp, AddKeyToGridNo );
	}
	else
	{
		CreateItem( gItemListBox.sCurSelectedItem, (UINT8)( gfDropDamagedItems ? ( 20 + Random( 60 ) ) : 100 ), &Object );

		//add the item to the world
		AddItemToPool(iGridNo, &Object, INVISIBLE, 0, 0, 0);
	}
}


static void AddKeyToGridNo(INT32 iKeyID)
{
	OBJECTTYPE		Object;

	if( iKeyID < NUM_KEYS )
	{
		CreateKeyObject( &Object, 1, (UINT8)iKeyID );

		//add the item to the world
		AddItemToPool(gsQdsEnteringGridNo, &Object, INVISIBLE, 0, 0, 0);
	}
	else
		gfAddKeyNextPass = TRUE;
}


static void ChangeDayNumber(INT32 iDayToChangeTo)
{
	INT32  uiDiff;
	UINT32 uiNewDayTimeInSec;

	if( iDayToChangeTo )
	{
		uiNewDayTimeInSec = (guiDay+iDayToChangeTo)*NUM_SEC_IN_DAY + 8*NUM_SEC_IN_HOUR + 15*NUM_SEC_IN_MIN;
		uiDiff = uiNewDayTimeInSec - guiGameClock;
		WarpGameTime( uiDiff, WARPTIME_PROCESS_EVENTS_NORMALLY );

		ForecastDayEvents( );

		//empty dialogue que of all sounds ( guys complain about being tired )
		//
		//	ATE: Please Fix Me!
		//		EmptyDialogueQueue();
	}
}


static void BtnQuestDebugNPCInventOkBtnButtonCallback(GUI_BUTTON* btn, UINT32 reason);


static void CreateDestroyDisplayNPCInventoryPopup(UINT8 ubAction)
{
	static BOOLEAN	fMouseRegionCreated = FALSE;
	UINT16	usPosY, i;

	switch( ubAction )
	{
		case QD_DROP_DOWN_NO_ACTION:
			break;

		case QD_DROP_DOWN_CREATE:
		{
			//if the soldier is active
			const ProfileID          pid = (gfUseLocalNPCs ? gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem] : gNpcListBox.sCurSelectedItem);
			const SOLDIERTYPE* const s   = FindSoldierByProfileID(pid);
			if (!s)
			{
				//qq Display error box

				gubNPCInventoryPopupAction = QD_DROP_DOWN_NO_ACTION;
				break;
			}

			if( fMouseRegionCreated )
				break;

			fMouseRegionCreated = TRUE;

			//create a mask to block out the screen
			if( !gfBackgroundMaskEnabled )
			{
				MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH + 40, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, QuestDebugTextEntryDisableScreenRegionCallBack);
				gfBackgroundMaskEnabled = TRUE;
			}

			//create the ok button
			guiQuestDebugNPCInventOkBtn = MakeButton("OK", QUEST_DBS_NPC_INV_POPUP_X + QUEST_DBS_NPC_INV_POPUP_WIDTH / 2 - 12, QUEST_DBS_NPC_INV_POPUP_Y + QUEST_DBS_NPC_INV_POPUP_HEIGHT - 30, 30, 25, MSYS_PRIORITY_HIGH + 50, BtnQuestDebugNPCInventOkBtnButtonCallback);
			guiQuestDebugNPCInventOkBtn->SetCursor(CURSOR_WWW);
			break;
		}

		case QD_DROP_DOWN_DESTROY:
			RemoveButton( guiQuestDebugNPCInventOkBtn );

			if( gfBackgroundMaskEnabled )
				MSYS_RemoveRegion( &gQuestTextEntryDebugDisableScreenRegion );
			gfBackgroundMaskEnabled = FALSE;

			gfRedrawQuestDebugSystem = TRUE;

			fMouseRegionCreated = FALSE;

			break;

		case QD_DROP_DOWN_DISPLAY:
		{
			UINT16	usFontHeight = GetFontHeight( QUEST_DBS_FONT_LISTBOX_TEXT ) + 2;

			//if the soldier is active
			const ProfileID          pid = (gfUseLocalNPCs ? gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem] : gNpcListBox.sCurSelectedItem);
			const SOLDIERTYPE* const s   = FindSoldierByProfileID(pid);
			if (s)
			{
				//color the background of the popup
				ColorFillVideoSurfaceArea( FRAME_BUFFER, QUEST_DBS_NPC_INV_POPUP_X, QUEST_DBS_NPC_INV_POPUP_Y, QUEST_DBS_NPC_INV_POPUP_X+QUEST_DBS_NPC_INV_POPUP_WIDTH,	QUEST_DBS_NPC_INV_POPUP_Y+QUEST_DBS_NPC_INV_POPUP_HEIGHT, Get16BPPColor( FROMRGB(  45,  59,  74 ) ) );

				//Dispaly the NPC inve title
				DrawTextToScreen(QuestDebugText[QUEST_DBS_NPC_INVENTORY], QUEST_DBS_NPC_INV_POPUP_X, QUEST_DBS_NPC_INV_POPUP_Y+5, QUEST_DBS_NPC_INV_POPUP_WIDTH, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_TITLE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

				//Dispaly the current npc name
				DrawTextToScreen(GetProfile(pid).zNickname, QUEST_DBS_NPC_INV_POPUP_X, QUEST_DBS_NPC_INV_POPUP_Y + 20, QUEST_DBS_NPC_INV_POPUP_WIDTH, QUEST_DBS_FONT_TITLE, QUEST_DBS_COLOR_SUBTITLE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

				usPosY = QUEST_DBS_NPC_INV_POPUP_Y + 40;
				for( i=0; i<NUM_INV_SLOTS; i++)
				{
					//Display Name of the pocket
					DrawTextToScreen(PocketText[i], QUEST_DBS_NPC_INV_POPUP_X+10, usPosY, 0, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_SUBTITLE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

					//Display the contents of the pocket
					DrawTextToScreen(GCM->getItem(s->inv[i].usItem)->getShortName(), QUEST_DBS_NPC_INV_POPUP_X + 140, usPosY, 0, QUEST_DBS_FONT_DYNAMIC_TEXT, QUEST_DBS_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
					usPosY += usFontHeight;
				}
			}
			InvalidateScreen();
			MarkButtonsDirty();
		}
		break;
	}
}


static void BtnQuestDebugNPCInventOkBtnButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubNPCInventoryPopupAction = QD_DROP_DOWN_DESTROY;
	}
}


static INT16 IsMercInTheSector(UINT16 usMercID);


static void BtnQuestDebugAllOrSectorNPCToggleCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if( gfUseLocalNPCs )
		{
			gfUseLocalNPCs = FALSE;


			gNpcListBox.sCurSelectedItem							= gubCurrentNpcInSector[ gNpcListBox.sCurSelectedItem ];
			gNpcListBox.usItemDisplayedOnTopOfList		= gNpcListBox.sCurSelectedItem;

			gNpcListBox.usMaxArrayIndex								= NUM_PROFILES;
			gNpcListBox.usNumDisplayedItems						= QUEST_DBS_MAX_DISPLAYED_ENTRIES;
			gNpcListBox.usMaxNumDisplayedItems				= QUEST_DBS_MAX_DISPLAYED_ENTRIES;
		}
		else
		{
			gfUseLocalNPCs = TRUE;

			gNpcListBox.sCurSelectedItem							= -1;
			gNpcListBox.usItemDisplayedOnTopOfList		= 0;
			gNpcListBox.usStartIndex									= 0;
			gNpcListBox.usMaxArrayIndex								= gubNumNPCinSector;

			if( gubNumNPCinSector < QUEST_DBS_MAX_DISPLAYED_ENTRIES )
			{
				gNpcListBox.usNumDisplayedItems					= gubNumNPCinSector;
				gNpcListBox.usMaxNumDisplayedItems			= gubNumNPCinSector;
			}
			else
			{
				gNpcListBox.usNumDisplayedItems					= QUEST_DBS_MAX_DISPLAYED_ENTRIES;
				gNpcListBox.usMaxNumDisplayedItems			= QUEST_DBS_MAX_DISPLAYED_ENTRIES;
			}

			if( gNpcListBox.sCurSelectedItem == -1 )
			{
				DisableButton( guiQuestDebugAddNpcToLocationButton );
				DisableButton( guiQuestDebugViewNPCInvButton );
				DisableButton( guiQuestDebugStartMercTalkingButtonButton );
			}

			if( IsMercInTheSector( gNpcListBox.sCurSelectedItem ) == -1 )
				DisableButton( guiQuestDebugViewNPCInvButton );

			EnableQDSButtons();
		}

		gfRedrawQuestDebugSystem = TRUE;
	}
}


static void AddNPCsInSectorToArray(void)
{
	//Setup array of merc who are in the current sector
	UINT16 i = 0;
	CFOR_EACH_SOLDIER(pSoldier)
	{
		//if soldier is a NPC, add him to the local NPC array
		if (pSoldier->ubProfile != NO_PROFILE)
		{
			MercProfile profile(pSoldier->ubProfile);
			if (profile.isNPC() || profile.isRPC() || profile.isVehicle())
			{
				gubCurrentNpcInSector[i++] = pSoldier->ubProfile;
			}
		}
	}
	gubNumNPCinSector = (UINT8)i;
}


static void ChangeQuestState(INT32 iNumber)
{
	if( ( iNumber >= 0 ) && ( iNumber <= 2 ) )
	{
		gubQuest[ gubCurQuestSelected ] = (UINT8) iNumber;
		gfRedrawQuestDebugSystem = TRUE;
	}
}


static void ChangeFactState(INT32 iNumber)
{
	if( ( iNumber >= 0 ) && ( iNumber <= 1 ) )
	{
		gubFact[ gusCurFactSelected ] = (UINT8) iNumber;
		gfRedrawQuestDebugSystem = TRUE;
	}
}


static void BtnQDPgUpButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( ( gusFactAtTopOfList - QUEST_DBS_NUM_DISPLAYED_FACTS ) >= 0 )
		{
			gusFactAtTopOfList -= QUEST_DBS_NUM_DISPLAYED_FACTS;
		}
		else
			gusFactAtTopOfList = 0;

		gfRedrawQuestDebugSystem = TRUE;
	}
}


static void BtnQDPgDownButtonButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if( ( gusFactAtTopOfList + QUEST_DBS_NUM_DISPLAYED_FACTS ) < NUM_FACTS )
		{
			gusFactAtTopOfList += QUEST_DBS_NUM_DISPLAYED_FACTS;
		}
		else
			gusFactAtTopOfList = NUM_FACTS - QUEST_DBS_NUM_DISPLAYED_FACTS;

		gfRedrawQuestDebugSystem = TRUE;
	}
}

static void EnableQDSButtons(void)
{
	{ bool const enable = gNpcListBox.sCurSelectedItem != -1;
		EnableButton(guiQuestDebugAddNpcToLocationButton,       enable);
		EnableButton(guiQuestDebugStartMercTalkingButtonButton, enable);
		EnableButton(guiQuestDebugNPCLogButtonButton,           enable);
		EnableButton(guiQuestDebugNPCRefreshButtonButton,       enable);
	}

	EnableButton(guiQuestDebugAddItemToLocationButton, gItemListBox.sCurSelectedItem != -1);

	EnableButton(guiQuestDebugGiveItemToNPCButton, gItemListBox.sCurSelectedItem != -1 && gNpcListBox.sCurSelectedItem != 0);

	if( gfUseLocalNPCs )
	{
		bool const enable = gNpcListBox.sCurSelectedItem != -1 && IsMercInTheSector(gubCurrentNpcInSector[gNpcListBox.sCurSelectedItem]) != -1;
		EnableButton(guiQuestDebugViewNPCInvButton,       enable);
		EnableButton(guiQuestDebugNPCRefreshButtonButton, enable);
		EnableButton(guiQuestDebugAddNpcToLocationButton, enable);
	}
}


static void DoQDSMessageBox(const ST::string& zString, ScreenID uiExitScreen, MessageBoxFlags ubFlags, MSGBOX_CALLBACK ReturnCallback)
{
	// reset exit mode
	gfExitQdsDueToMessageBox = TRUE;
	gfQuestDebugEntry = TRUE;

	// do message box and return
	DoMessageBox(MSG_BOX_BASIC_STYLE, zString, uiExitScreen, ubFlags, ReturnCallback, NULL);
}


static void IncrementActiveDropDownBox(INT16 sIncrementValue)
{
	if( sIncrementValue < 0 )
		sIncrementValue = 0;

	//if the mouse was clicked above the scroll box
	if( sIncrementValue < gpActiveListBox->sCurSelectedItem )
	{
		if( ( sIncrementValue ) <= gpActiveListBox->usStartIndex )
		{
			gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usStartIndex;
			sIncrementValue = gpActiveListBox->usStartIndex;
		}
		else if( sIncrementValue < gpActiveListBox->usItemDisplayedOnTopOfList && gpActiveListBox->usItemDisplayedOnTopOfList > gpActiveListBox->usStartIndex )
		{
			gpActiveListBox->usItemDisplayedOnTopOfList = sIncrementValue;
		}
	}
	// else the mouse was clicked below the scroll box
	else
	{
		if( sIncrementValue >= ( gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems ) )
		{
			if( gpActiveListBox->usItemDisplayedOnTopOfList >= gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems )
				gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems;
			else if( ( sIncrementValue - gpActiveListBox->usItemDisplayedOnTopOfList ) >= gpActiveListBox->usMaxNumDisplayedItems )
			{
				gpActiveListBox->usItemDisplayedOnTopOfList = sIncrementValue - gpActiveListBox->usMaxNumDisplayedItems + 1;
			}

			if( sIncrementValue >= gpActiveListBox->usMaxArrayIndex )
				sIncrementValue = gpActiveListBox->usMaxArrayIndex - 1;
		}
		else if( sIncrementValue >=  gpActiveListBox->usMaxArrayIndex )
		{
			sIncrementValue = gpActiveListBox->usMaxArrayIndex - 1;
			gpActiveListBox->usItemDisplayedOnTopOfList = gpActiveListBox->usMaxArrayIndex - gpActiveListBox->usMaxNumDisplayedItems;
		}
		else if( sIncrementValue >= gpActiveListBox->usItemDisplayedOnTopOfList + gpActiveListBox->usMaxNumDisplayedItems )
		{
			gpActiveListBox->usItemDisplayedOnTopOfList += sIncrementValue - ( gpActiveListBox->usItemDisplayedOnTopOfList + gpActiveListBox->usMaxNumDisplayedItems - 1 );
		}
	}

	gpActiveListBox->sCurSelectedItem = sIncrementValue;

	gpActiveListBox->ubCurScrollBoxAction = QD_DROP_DOWN_DISPLAY;
}


static INT16 IsMercInTheSector(UINT16 usMercID)
{
	if( usMercID == (UINT16)-1 ) /* XXX */
		return( FALSE );

	CFOR_EACH_SOLDIER(s)
	{
		if (s->ubProfile == usMercID) return s->ubID;
	}

	return( -1 );
}


static void RefreshAllNPCInventory(void)
{
	UINT16	usItemCnt;
	OBJECTTYPE	TempObject;
	UINT16		usItem;

	FOR_EACH_SOLDIER(s)
	{
		//is the merc a rpc or npc
		if (s->ubProfile != NO_PROFILE && MercProfile(s->ubProfile).isNPCorRPC())
		{
			//refresh the mercs inventory
			for ( usItemCnt = 0; usItemCnt< NUM_INV_SLOTS; usItemCnt++ )
			{
				//null out the items in the npc inventory
				s->inv[usItemCnt] = OBJECTTYPE{};

				MERCPROFILESTRUCT const& p = GetProfile(s->ubProfile);
				if (p.inv[usItemCnt] != NOTHING)
				{
					//get the item
					usItem = p.inv[usItemCnt];

					//Create the object
					CreateItem( usItem, 100, &TempObject );

					//copy the item into the soldiers inventory
					s->inv[usItemCnt] = TempObject;
				}
			}
		}
	}
}


static void DisableFactMouseRegions();
static INT32 GetMaxNumberOfQuotesToPlay(void);
static void SetQDSMercProfile(void);


static void StartMercTalkingFromQuoteNum(INT32 iQuoteToStartTalkingFrom)
{
	ST::string zTemp;
	INT32		uiMaxNumberOfQuotes = GetMaxNumberOfQuotesToPlay( );

	//make sure the current character is created
	SetQDSMercProfile();

	SetTalkingMercPauseState( FALSE );

	//do some error checks
	if( iQuoteToStartTalkingFrom < 0 || iQuoteToStartTalkingFrom > uiMaxNumberOfQuotes )
	{
		zTemp = ST::format("Please enter a value between 0 and {}", uiMaxNumberOfQuotes);
		DoQDSMessageBox(zTemp, QUEST_DEBUG_SCREEN, MSG_BOX_FLAG_OK, NULL);
	}
	else
	{
		//Start the merc talking from the selected quote number
		giSelectedMercCurrentQuote = iQuoteToStartTalkingFrom;
	}

		//create a mask to block out the screen
	if( !gfBackgroundMaskEnabled )
	{
		MSYS_DefineRegion(&gQuestTextEntryDebugDisableScreenRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGH + 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, QuestDebugTextEntryDisableScreenRegionCallBack);
		gfBackgroundMaskEnabled = TRUE;
	}

	DisableFactMouseRegions();

}


static void EnableFactMouseRegions();


static void EndMercTalking(void)
{
	//remove the talking dialogue
	if( gfNpcPanelIsUsedForTalkingMerc )
		DeleteTalkingMenu( );
	gfNpcPanelIsUsedForTalkingMerc = FALSE;

	//remove the mask of the entire screen
	if( gfBackgroundMaskEnabled )
	{
		MSYS_RemoveRegion( &gQuestTextEntryDebugDisableScreenRegion);
		gfBackgroundMaskEnabled = FALSE;
	}

	giSelectedMercCurrentQuote = -1;

	//make sure we can dirty the button
	if( !gfQuestDebugExit )
		guiQDPgUpButtonButton->uiFlags &= ~BUTTON_FORCE_UNDIRTY;


	//enable the fact mouse regions
	EnableFactMouseRegions();
}


static void HandleQDSTalkingMerc(void)
{
//	static BOOLEAN	fWas
	UINT8		ubPanelMercShouldUse;

	if( giSelectedMercCurrentQuote != -1 )
	{
		if( gTalkingMercSoldier == NULL )
			return;

		//Call this function to enable or disable the flags in the faces struct ( without modifing the pause state )
		SetTalkingMercPauseState( gfPauseTalkingMercPopup );

		ubPanelMercShouldUse = WhichPanelShouldTalkingMercUse();

		//if the merc is not talking
		if (!GetQDSFace(ubPanelMercShouldUse)->fTalking)
		{
			//if we still have more quotes to say
			if( giSelectedMercCurrentQuote < GetMaxNumberOfQuotesToPlay( ) )
			{
				//if the user has paused the playing
				if( gfPauseTalkingMercPopup )
				{
					//get out
					return;
				}

				//Start the merc talking
				if( ubPanelMercShouldUse == QDS_REGULAR_PANEL )
					TacticalCharacterDialogue( gTalkingMercSoldier, (UINT16)giSelectedMercCurrentQuote );
				else if (gfRpcToSaySectorDesc && MIGUEL <= gTalkingMercSoldier->ubProfile && gTalkingMercSoldier->ubProfile <= DIMITRI)
				{
					//ATE: Trigger the sector desc here
					CharacterDialogueUsingAlternateFile(*gTalkingMercSoldier, giSelectedMercCurrentQuote, DIALOGUE_NPC_UI);
				}
				else
				{
					CharacterDialogue(gTalkingMercSoldier->ubProfile, giSelectedMercCurrentQuote, gTalkPanel.face, DIALOGUE_NPC_UI, FALSE);
				}

				//Incremenet the current quote number
				giSelectedMercCurrentQuote++;
			}
			else
			{
				//Stop the merc from talking
				giSelectedMercCurrentQuote = -1;

				EndMercTalking();
			}
		}

		//Redraw the screen
		gfRedrawQuestDebugSystem = TRUE;
	}
}


static void SetTalkingMercPauseState(BOOLEAN fState)
{
	if( fState )
	{
		gfPauseTalkingMercPopup = TRUE;
		if (gTalkingMercSoldier) gTalkingMercSoldier->face->uiFlags |= FACE_POTENTIAL_KEYWAIT;
	}
	else
	{
		gfPauseTalkingMercPopup = FALSE;
		if (gTalkingMercSoldier) gTalkingMercSoldier->face->uiFlags &= ~FACE_POTENTIAL_KEYWAIT;
	}
}


static void SetQDSMercProfile(void)
{
	// Get selected soldier
	gTalkingMercSoldier = GetSelectedMan();
	if (gTalkingMercSoldier == NULL) return;

	// Change guy!
	ForceSoldierProfileID( gTalkingMercSoldier, (UINT8)gNpcListBox.sCurSelectedItem );

	//if it is an rpc
	if (MIGUEL <= gTalkingMercSoldier->ubProfile && gTalkingMercSoldier->ubProfile <= MADDOG)
	{
		if( gfAddNpcToTeam )
			gMercProfiles[ gTalkingMercSoldier->ubProfile ].ubMiscFlags |= PROFILE_MISC_FLAG_RECRUITED;
		else
			gMercProfiles[ gTalkingMercSoldier->ubProfile ].ubMiscFlags &= ~PROFILE_MISC_FLAG_RECRUITED;
	}
	else
	{
	}

	if( WhichPanelShouldTalkingMercUse( ) == QDS_NPC_PANEL )
	{
		//remove the talking dialogue
		if( gfNpcPanelIsUsedForTalkingMerc )
			DeleteTalkingMenu( );

		gfNpcPanelIsUsedForTalkingMerc = TRUE;

		InternalInitTalkingMenu( gTalkingMercSoldier->ubProfile, 10, 10 );
		gpDestSoldier = &GetMan(21);
	}
}


static void DisplayQDSCurrentlyQuoteNum(void)
{
	ST::string zTemp;
	UINT16	usPosY;
	UINT16	usFontHeight = GetFontHeight( QUEST_DBS_FONT_TEXT_ENTRY ) + 2;

	//Display the box frame
	ColorFillVideoSurfaceArea( FRAME_BUFFER, QDS_CURRENT_QUOTE_NUM_BOX_X, QDS_CURRENT_QUOTE_NUM_BOX_Y, QDS_CURRENT_QUOTE_NUM_BOX_X+QDS_CURRENT_QUOTE_NUM_BOX_WIDTH,	QDS_CURRENT_QUOTE_NUM_BOX_Y+QDS_CURRENT_QUOTE_NUM_BOX_HEIGHT, Get16BPPColor( FROMRGB(  32,  41,  53 ) ) );

	zTemp = ST::format("'{}' is currently saying quote #{}", gMercProfiles[gTalkingMercSoldier->ubProfile].zNickname, giSelectedMercCurrentQuote - 1);

	//Display the text box caption
	usPosY = QDS_CURRENT_QUOTE_NUM_BOX_Y+4;
	DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY, QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY, FONT_MCOLOR_WHITE, zTemp, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display the Pause speech text
	usPosY += usFontHeight + 4;
	DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY, QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY, FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_PAUSE_SPEECH], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the left arrow quote
	usPosY += usFontHeight;
	DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY, QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY, FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_LEFT_ARROW_PREVIOUS_QUOTE], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the right arrow quote
	usPosY += usFontHeight;
	DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY, QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY, FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_RIGHT_ARROW_NEXT_QUOTE], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the right arrow quote
	usPosY += usFontHeight;
	DisplayWrappedString(QDS_CURRENT_QUOTE_NUM_BOX_X + 5, usPosY, QDS_CURRENT_QUOTE_NUM_BOX_WIDTH - 10, 2, QUEST_DBS_FONT_TEXT_ENTRY, FONT_MCOLOR_WHITE, QuestDebugText[QUEST_DBS_ESC_TOP_STOP_TALKING], FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);


	InvalidateRegion( QDS_CURRENT_QUOTE_NUM_BOX_X, QDS_CURRENT_QUOTE_NUM_BOX_Y, QDS_CURRENT_QUOTE_NUM_BOX_X+QDS_CURRENT_QUOTE_NUM_BOX_WIDTH,	QDS_CURRENT_QUOTE_NUM_BOX_Y+QDS_CURRENT_QUOTE_NUM_BOX_HEIGHT );
}


static void BtnQuestDebugAddNpcToTeamToggleCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if( gfAddNpcToTeam )
			gfAddNpcToTeam = FALSE;
		else
			gfAddNpcToTeam = TRUE;
	}
}


static void BtnQuestDebugRPCSaySectorDescToggleCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
	{
		if( gfRpcToSaySectorDesc )
			gfRpcToSaySectorDesc = FALSE;
		else
			gfRpcToSaySectorDesc = TRUE;
	}
}


static UINT8 WhichPanelShouldTalkingMercUse(void)
{
	if ( gTalkingMercSoldier == NULL )
	{
		return( QDS_NO_PANEL );
	}

	if (gTalkingMercSoldier->ubProfile != NO_PROFILE && MercProfile(gTalkingMercSoldier->ubProfile).isPlayerMerc())
	{
		return( QDS_REGULAR_PANEL );
	}
	else
	{
		return( QDS_NPC_PANEL );
	}
}


static void DisableFactMouseRegions()
{
	FOR_EACH(MOUSE_REGION, i, gFactListRegion) i->Disable();
}


static void EnableFactMouseRegions()
{
	FOR_EACH(MOUSE_REGION, i, gFactListRegion) i->Enable();
}


static INT32 GetMaxNumberOfQuotesToPlay(void)
{
	const ProfileID pid = gNpcListBox.sCurSelectedItem;

	INT32	iNumberOfQuotes;
	if (gfRpcToSaySectorDesc && MIGUEL <= pid && pid <= DIMITRI)
	{
		// If it is the RPCs and they are to say the sector descs
		iNumberOfQuotes = 34;
	}
	else if (GetProfile(pid).ubMiscFlags & PROFILE_MISC_FLAG_RECRUITED &&
			MIGUEL <= pid && pid <= MADDOG)
	{
		// It is a RPC who is on our team
		iNumberOfQuotes = 119;
	}
	else if (pid == QUEEN)
	{
		iNumberOfQuotes = 138;
	}
	else if (pid == SPECK)
	{
		iNumberOfQuotes  = 72;
	}
	else
	{
		iNumberOfQuotes  = 138;
	}

	return iNumberOfQuotes + 1;
}


static ST::string GetDebugLocationString(UINT16 usProfileID)
{
	const SOLDIERTYPE* const pSoldier = FindSoldierByProfileID(usProfileID);

	//if their is a soldier, the soldier is alive and the soldier is off the map
	if (pSoldier != NULL && pSoldier->uiStatusFlags & SOLDIER_OFF_MAP)
	{
		//the soldier is on schedule
		return "On Schdl.";
	}

	//if the soldier is dead
	else if( gMercProfiles[ usProfileID ].bMercStatus == MERC_IS_DEAD )
	{
		return "Dead";
	}

	//the soldier is in this sector
	else if( pSoldier != NULL )
	{
		return pSoldier->sSector.AsShortString();
	}

	//else the soldier is in a different map
	else
	{
		return gMercProfiles[usProfileID].sSector.AsShortString();
	}
}
