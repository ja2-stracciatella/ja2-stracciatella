#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "laptop.h"
	#include "personnel.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "Debug.h"
	#include "WordWrap.h"
	#include "Render Dirty.h"
	#include "Encrypted File.h"
	#include "cursors.h"
	#include "Overhead.h"
	#include "Soldier Profile.h"
	#include "Text.h"
	#include "mapscreen.h"
	#include "Game Clock.h"
	#include "finances.h"
	#include "LaptopSave.h"
	#include "AimMembers.h"
	#include "Map Screen Interface Map.h"
	#include "input.h"
	#include "english.h"
	#include "random.h"
	#include "line.h"
	#include "Assignments.h"
	#include "gameloop.h"
	#include "Soldier Add.h"
	#include "Interface Items.h"
	#include "Weapons.h"
#endif

#include "Soldier Macros.h"



extern struct POINT;

#define NUM_BACKGROUND_REPS					40
#define BACKGROUND_HEIGHT						10
#define BACKGROUND_WIDTH						125
#define IMAGE_BOX_X									395
#define IMAGE_BOX_Y									LAPTOP_SCREEN_UL_Y+24
#define IMAGE_BOX_WIDTH							112
#define	IMAGE_BOX_WITH_NO_BORDERS		106
#define IMAGE_BOX_COUNT							4
#define	IMAGE_NAME_WIDTH						106
#define	IMAGE_FULL_NAME_OFFSET_Y		111
#define TEXT_BOX_WIDTH							160
#define	TEXT_DELTA_OFFSET						9
#define TEXT_BOX_Y LAPTOP_SCREEN_UL_Y+188
#define PAGE_BOX_X LAPTOP_SCREEN_UL_X+250 - 10
#define PAGE_BOX_Y LAPTOP_SCREEN_UL_Y+3
#define PAGE_BOX_WIDTH  58
#define PAGE_BOX_HEIGHT 24
#define MAX_SLOTS 4
#define PERS_CURR_TEAM_X LAPTOP_SCREEN_UL_X + 39 - 15
#define PERS_CURR_TEAM_Y LAPTOP_SCREEN_UL_Y + 218
#define PERS_DEPART_TEAM_Y LAPTOP_SCREEN_UL_Y + 247

#define BUTTON_Y LAPTOP_SCREEN_UL_Y+34
#define LEFT_BUTTON_X  LAPTOP_SCREEN_UL_X+3 - 10
#define RIGHT_BUTTON_X LAPTOP_SCREEN_UL_X+476 - 10
#define PERS_COUNT 15
#define MAX_STATS 20
#define PERS_FONT						FONT10ARIAL
#define PERS_HEADER_FONT		FONT14ARIAL
#define CHAR_NAME_FONT			FONT12ARIAL
#define CHAR_NAME_Y 177
#define CHAR_LOC_Y	189
#define PERS_TEXT_FONT_COLOR FONT_WHITE //146
#define PERS_TEXT_FONT_ALTERNATE_COLOR FONT_YELLOW
#define PERS_FONT_COLOR FONT_WHITE
#define PAGE_X PAGE_BOX_X+2 - 10
#define PAGE_Y PAGE_BOX_Y+2



#define	FACES_DIR "FACES\\BIGFACES\\"
#define	SMALL_FACES_DIR "FACES\\"

#define NEXT_MERC_FACE_X  LAPTOP_SCREEN_UL_X + 448
#define MERC_FACE_SCROLL_Y LAPTOP_SCREEN_UL_Y + 150
#define PREV_MERC_FACE_X  LAPTOP_SCREEN_UL_X + 285



#define DEPARTED_X LAPTOP_SCREEN_UL_X + 29 - 10
#define DEPARTED_Y LAPTOP_SCREEN_UL_Y + 207


#define PERSONNEL_PORTRAIT_NUMBER 20
#define PERSONNEL_PORTRAIT_NUMBER_WIDTH 5

#define SMALL_PORTRAIT_WIDTH 46
#define SMALL_PORTRAIT_HEIGHT 42

#define SMALL_PORT_WIDTH 52
#define SMALL_PORT_HEIGHT 45

#define	SMALL_PORTRAIT_WIDTH_NO_BORDERS		48

#define SMALL_PORTRAIT_START_X 141 - 10
#define SMALL_PORTRAIT_START_Y 53

#define PERS_CURR_TEAM_COST_X LAPTOP_SCREEN_UL_X + 150 - 10
#define PERS_CURR_TEAM_COST_Y LAPTOP_SCREEN_UL_Y + 218

#define PERS_CURR_TEAM_HIGHEST_Y PERS_CURR_TEAM_COST_Y + 15
#define PERS_CURR_TEAM_LOWEST_Y PERS_CURR_TEAM_HIGHEST_Y + 15

#define PERS_CURR_TEAM_WIDTH 286 - 160

#define PERS_DEPART_TEAM_WIDTH PERS_CURR_TEAM_WIDTH - 20

#define PERS_STAT_AVG_X LAPTOP_SCREEN_UL_X + 157 - 10
#define PERS_STAT_AVG_Y LAPTOP_SCREEN_UL_Y + 274
#define PERS_STAT_AVG_WIDTH 202 - 159
#define PERS_STAT_LOWEST_X LAPTOP_SCREEN_UL_X + 72 - 10
#define PERS_STAT_LOWEST_WIDTH 155 - 75
#define PERS_STAT_HIGHEST_X LAPTOP_SCREEN_UL_X + 205 - 10
#define PERS_STAT_LIST_X LAPTOP_SCREEN_UL_X + 33 - 10


#define PERS_TOGGLE_CUR_DEPART_WIDTH 106 - 35
#define PERS_TOGGLE_CUR_DEPART_HEIGHT 236 - 212

#define PERS_TOGGLE_CUR_DEPART_X LAPTOP_SCREEN_UL_X + 35 - 10
#define PERS_TOGGLE_CUR_Y LAPTOP_SCREEN_UL_Y + 208
#define PERS_TOGGLE_DEPART_Y LAPTOP_SCREEN_UL_Y + 238

#define PERS_DEPARTED_UP_X  LAPTOP_SCREEN_UL_X + 265 - 10
#define PERS_DEPARTED_UP_Y  LAPTOP_SCREEN_UL_Y + 210
#define PERS_DEPARTED_DOWN_Y LAPTOP_SCREEN_UL_Y + 237


#define PERS_TITLE_X 140
#define PERS_TITLE_Y 33

#define ATM_UL_X LAPTOP_SCREEN_UL_X + 397
#define ATM_UL_Y LAPTOP_SCREEN_UL_Y + 27

/// atm font
#define ATM_FONT PERS_FONT

// departed states
enum{
	DEPARTED_DEAD = 0,
	DEPARTED_FIRED,
	DEPARTED_OTHER,
	DEPARTED_MARRIED,
	DEPARTED_CONTRACT_EXPIRED,
	DEPARTED_QUIT,

};

// atm button positions
#define ATM_BUTTONS_START_Y 110
#define ATM_BUTTONS_START_X 510
#define ATM_BUTTON_WIDTH 15
#define ATM_BUTTON_HEIGHT 15
#define ATM_DISPLAY_X 509
#define ATM_DISPLAY_Y 58
#define ATM_DISPLAY_HEIGHT 10
#define ATM_DISPLAY_WIDTH  81


// the number of inventory items per personnel page
#define NUMBER_OF_INVENTORY_PERSONNEL 8
#define Y_SIZE_OF_PERSONNEL_SCROLL_REGION ( 422 - 219 )
#define X_SIZE_OF_PERSONNEL_SCROLL_REGION ( 589 - 573 )
#define Y_OF_PERSONNEL_SCROLL_REGION 219
#define X_OF_PERSONNEL_SCROLL_REGION 573
#define SIZE_OF_PERSONNEL_CURSOR 19

// number buttons
enum{
	OK_ATM = 0,
	DEPOSIT_ATM,
	WIDTHDRAWL_ATM,
	CANCEL_ATM,
	CLEAR_ATM,
	NUMBER_ATM_BUTTONS,
};


// enums for the buttons in the information side bar ( used with giPersonnelATMStartButton[] )
enum
{
	PERSONNEL_STAT_BTN,
	PERSONNEL_EMPLOYMENT_BTN,
	PERSONNEL_INV_BTN,

	PERSONNEL_NUM_BTN,
};


//enums for the current state of the information side bar ( stat panel )
enum
{
	PRSNL_STATS,
	PRSNL_EMPLOYMENT,
	PRSNL_INV,
};
UINT8	gubPersonnelInfoState = PRSNL_STATS;


//enums for the pPersonnelScreenStrings[]
enum
{
	PRSNL_TXT_HEALTH, // HEALTH OF MERC
	PRSNL_TXT_AGILITY,
	PRSNL_TXT_DEXTERITY,
	PRSNL_TXT_STRENGTH,
	PRSNL_TXT_LEADERSHIP,
	PRSNL_TXT_WISDOM,					//5
	PRSNL_TXT_EXP_LVL, // EXPERIENCE LEVEL
	PRSNL_TXT_MARKSMANSHIP,
	PRSNL_TXT_MECHANICAL,
	PRSNL_TXT_EXPLOSIVES,
	PRSNL_TXT_MEDICAL,				//10
	PRSNL_TXT_MED_DEPOSIT, // AMOUNT OF MEDICAL DEPOSIT PUT DOWN ON THE MERC
	PRSNL_TXT_CURRENT_CONTRACT, // COST OF CURRENT CONTRACT
	PRSNL_TXT_KILLS, // NUMBER OF KILLS BY MERC
	PRSNL_TXT_ASSISTS, // NUMBER OF ASSISTS ON KILLS BY MERC
	PRSNL_TXT_DAILY_COST, // DAILY COST OF MERC			//15
	PRSNL_TXT_TOTAL_COST, // TOTAL COST OF MERC
	PRSNL_TXT_CONTRACT, // COST OF CURRENT CONTRACT
	PRSNL_TXT_TOTAL_SERVICE, // TOTAL SERVICE RENDERED BY MERC
	PRSNL_TXT_UNPAID_AMOUNT, // AMOUNT LEFT ON MERC MERC TO BE PAID
	PRSNL_TXT_HIT_PERCENTAGE, // PERCENTAGE OF SHOTS THAT HIT TARGET		//20
	PRSNL_TXT_BATTLES, // NUMBER OF BATTLES FOUGHT
	PRSNL_TXT_TIMES_WOUNDED, // NUMBER OF TIMES MERC HAS BEEN WOUNDED
	PRSNL_TXT_SKILLS,
	PRSNL_TXT_NOSKILLS,
};

extern BOOLEAN gfTemporaryDisablingOfLoadPendingFlag;
extern BOOLEAN fExitingLaptopFlag;
extern void HandleLapTopESCKey( void );
extern void HandleAltTabKeyInLaptop( void );
extern void HandleShiftAltTabKeyInLaptop( void );

//BOOLEAN fShowInventory = FALSE;
UINT8 uiCurrentInventoryIndex = 0;

UINT32 guiSliderPosition;

// the transfer funds string
CHAR16 sTransferString[ 32 ];

INT32 giPersonnelATMSideButton[ NUMBER_ATM_BUTTONS ];
INT32 giPersonnelATMSideButtonImage[ NUMBER_ATM_BUTTONS ];

INT32 iNumberPadButtons[ 10 ];
INT32 iNumberPadButtonsImages[ 10 ];

POINT pAtmSideButtonPts[]={
	{ 533, 155 },
	{ 558, 110 },
	{ 558, 125 },
	{ 558, 140 },
	{ 558, 155 },
};

#define PrsnlOffSetX	(-15) //-20
#define Prsnl_DATA_OffSetX	(36)
#define PrsnlOffSetY	10

POINT pPersonnelScreenPoints[]=
{
	{422+PrsnlOffSetX, 205+PrsnlOffSetY},
	{422+PrsnlOffSetX, 215+PrsnlOffSetY},
	{422+PrsnlOffSetX, 225+PrsnlOffSetY},
	{422+PrsnlOffSetX, 235+PrsnlOffSetY},
	{422+PrsnlOffSetX, 245+PrsnlOffSetY},
	{422+PrsnlOffSetX, 255+PrsnlOffSetY},
	{422+PrsnlOffSetX, 315+PrsnlOffSetY},
	{422+PrsnlOffSetX, 270+PrsnlOffSetY},
	{422+PrsnlOffSetX, 280+PrsnlOffSetY},
	{422+PrsnlOffSetX, 290+PrsnlOffSetY},
	{422+PrsnlOffSetX, 300+PrsnlOffSetY},			//10
	{422+PrsnlOffSetX, 395+PrsnlOffSetY},
	{422+PrsnlOffSetX, 385+PrsnlOffSetY},
	{422+PrsnlOffSetX, 415+PrsnlOffSetY},
	{422+PrsnlOffSetX, 425+PrsnlOffSetY},
	{422+PrsnlOffSetX, 445+PrsnlOffSetY},
	{422+PrsnlOffSetX, 380+PrsnlOffSetY}, // for contract price
	{422+PrsnlOffSetX, 435+PrsnlOffSetY},
	{140,33},  // Personnel Header
	{422+PrsnlOffSetX, 330+PrsnlOffSetY},
	{422+PrsnlOffSetX, 340+PrsnlOffSetY},	//20
	{422+PrsnlOffSetX, 355+PrsnlOffSetY},
	{422+PrsnlOffSetX, 365+PrsnlOffSetY},
	{422+PrsnlOffSetX, 375+PrsnlOffSetY},
	{422+PrsnlOffSetX, 385+PrsnlOffSetY},
	{422+PrsnlOffSetX, 395+PrsnlOffSetY},
};




UINT32 guiSCREEN;
UINT32 guiTITLE;
UINT32 guiFACE;
UINT32 guiDEPARTEDTEAM;
UINT32 guiCURRENTTEAM;
UINT32 guiPersonnelInventory;

INT32 giPersonnelButton[6];
INT32 giPersonnelButtonImage[6];
INT32 giPersonnelInventoryButtons[ 2 ];
INT32 giPersonnelInventoryButtonsImages[ 2 ];
INT32 iStartPersonId; // iId of the person who is leftmost on the display
INT32 iLastPersonId;
INT32 giDepartedButtonImage[ 2 ];
INT32 giDepartedButton[ 2 ];

// buttons for ATM
INT32 giPersonnelATMStartButton[ 3 ];
INT32 giPersonnelATMStartButtonImage[ 3 ];
INT32 giPersonnelATMButton;
INT32 giPersonnelATMButtonImage;

BOOLEAN fATMFlags = 0;
BOOLEAN fOldATMFlags = 0;
// the past team of the player
//INT16 ubDeadCharactersList[ 256 ];
//INT16 ubLeftCharactersList[ 256 ];
//INT16 ubOtherCharactersList[ 256 ];

// the id of currently displayed merc in right half of screen
INT32 iCurrentPersonSelectedId = -1;

INT32 giCurrentUpperLeftPortraitNumber = 0;

// which mode are we showing?..current team?...or deadly departed?
BOOLEAN fCurrentTeamMode = TRUE;

// show the atm panel?
BOOLEAN fShowAtmPanel = FALSE;
BOOLEAN fShowAtmPanelStartButton = TRUE;

// create buttons for scrolling departures
BOOLEAN fCreatePeronnelDepartureButton = FALSE;

// waitr one frame
BOOLEAN fOneFrameDelayInPersonnel = FALSE;

// whther or not we are creating mouse regions to place over portraits
BOOLEAN fCreatePersonnelPortraitMouseRegions = FALSE;

// mouse regions
MOUSE_REGION gPortraitMouseRegions[ 20 ];

MOUSE_REGION gTogglePastCurrentTeam[ 2 ];

MOUSE_REGION gMouseScrollPersonnelINV;

INT32 iCurPortraitId = 0;


// create mouse regions for past/current toggles
BOOLEAN fCreateRegionsForPastCurrentToggle = FALSE;



BOOLEAN LoadPersonnelGraphics( void );
void RemovePersonnelGraphics( void );
void RenderPersonnel( void );
void RenderPersonnelStats(INT32 iId, INT32 iSlot);
BOOLEAN RenderPersonnelFace(INT32 iId, INT32 iSlot, BOOLEAN fDead, BOOLEAN fFired, BOOLEAN fOther );
BOOLEAN RenderPersonnelPictures( void );
void LeftButtonCallBack(GUI_BUTTON *btn,INT32 reason);
void RightButtonCallBack(GUI_BUTTON *btn,INT32 reason);
void LeftFFButtonCallBack(GUI_BUTTON *btn,INT32 reason);
void RightFFButtonCallBack(GUI_BUTTON *btn,INT32 reason);
void PersonnelPortraitCallback( MOUSE_REGION * pRegion, INT32 iReason );
void CreatePersonnelButtons( void );
void DeletePersonnelButtons( void );
void DisplayHeader( void );
void DisplayCharName( INT32 iId, INT32 iSlot );
void DisplayCharStats(INT32 iId, INT32 iSlot);
INT32 GetLastMercId( void );
void DrawPageNumber( void );
void SetPersonnelButtonStates( void );
void CreateDestroyButtonsForPersonnelDepartures( void );
BOOLEAN LoadPersonnelScreenBackgroundGraphics( void );
void DeletePersonnelScreenBackgroundGraphics( void );
void RenderPersonnelScreenBackground( void );
INT32 GetNumberOfMercsOnPlayersTeam( void );
void CreateDestroyMouseRegionsForPersonnelPortraits( void );
BOOLEAN DisplayPicturesOfCurrentTeam( void );
void DisplayFaceOfDisplayedMerc( );
void DisplayNumberOnCurrentTeam( void );
void DisplayNumberDeparted( void );
INT32 GetTotalDailyCostOfCurrentTeam( void );
void DisplayCostOfCurrentTeam( void );
INT32 GetLowestDailyCostOfCurrentTeam( void );
INT32 GetHighestDailyCostOfCurrentTeam( void );
INT32 GetAvgStatOfCurrentTeamStat( INT32 iStat );
void DisplayAverageStatValuesForCurrentTeam( void );
void DisplayLowestStatValuesForCurrentTeam( void );
void DisplayHighestStatValuesForCurrentTeam( void );
void DisplayPersonnelTeamStats( void );
void InitPastCharactersList( void );
INT32 GetNumberOfPastMercsOnPlayersTeam( void );
INT32 GetNumberOfOtherOnPastTeam( void );
INT32 GetNumberOfLeftOnPastTeam( void );
INT32 GetNumberOfDeadOnPastTeam( void );
void DisplayStateOfPastTeamMembers( void );
void CreateDestroyCurrentDepartedMouseRegions( void );
void PersonnelCurrentTeamCallback( MOUSE_REGION * pRegion, INT32 iReason );
void PersonnelDepartedTeamCallback( MOUSE_REGION * pRegion, INT32 iReason );
void CreateDestroyButtonsForDepartedTeamList( void );
void DepartedDownCallBack(GUI_BUTTON *btn,INT32 reason);
void DepartedUpCallBack(GUI_BUTTON *btn,INT32 reason);
void DisplayPastMercsPortraits( void );
BOOLEAN DisplayPortraitOfPastMerc( INT32 iId , INT32 iCounter, BOOLEAN fDead, BOOLEAN fFired, BOOLEAN fOther );
INT32 GetIdOfPastMercInSlot( INT32 iSlot );
void DisplayDepartedCharStats(INT32 iId, INT32 iSlot, INT32 iState);
void EnableDisableDeparturesButtons( void );
void DisplayDepartedCharName( INT32 iId, INT32 iSlot, INT32 iState );
INT32 GetTheStateOfDepartedMerc( INT32 iId );
void DisplayPersonnelTextOnTitleBar( void );
INT32 GetIdOfDepartedMercWithHighestStat( INT32 iStat );
INT32 GetIdOfDepartedMercWithLowestStat( INT32 iStat );
void RenderInventoryForCharacter( INT32 iId, INT32 iSlot );
void DisplayInventoryForSelectedChar( void );
INT32 GetNumberOfInventoryItemsOnCurrentMerc( void );
void CreateDestroyPersonnelInventoryScrollButtons( void );
void EnableDisableInventoryScrollButtons( void );
void PersonnelINVStartButtonCallback(GUI_BUTTON *btn,INT32 reason);
void EmployementInfoButtonCallback(GUI_BUTTON *btn,INT32 reason);
void PersonnelStatStartButtonCallback(GUI_BUTTON *btn,INT32 reason);
void HandleSliderBarClickCallback( MOUSE_REGION *pRegion, INT32 iReason );
INT32 GetNumberOfMercsDeadOrAliveOnPlayersTeam( void );

void RenderSliderBarForPersonnelInventory( void );
void FindPositionOfPersInvSlider( void );


// check if current guy can have atm
void UpDateStateOfStartButton( void );
void HandlePersonnelKeyboard( void );


void DisplayEmploymentinformation( INT32 iId, INT32 iSlot );



// AIM merc:  Returns the amount of time left on mercs contract
// MERC merc: Returns the amount of time the merc has worked
// IMP merc:	Returns the amount of time the merc has worked
// else:			returns -1
INT32 CalcTimeLeftOnMercContract( SOLDIERTYPE *pSoldier );


// what state is the past merc in?
BOOLEAN IsPastMercDead( INT32 iId );
BOOLEAN IsPastMercFired( INT32 iId );
BOOLEAN IsPastMercOther( INT32 iId );


// display box around currently selected merc
BOOLEAN DisplayHighLightBox( void );

// id of merc in this slot
INT32 GetIdOfThisSlot( INT32 iSlot );

// grab appropriate id of soldier first being displayed
INT32 GetIdOfFirstDisplayedMerc( );

// get avg for this stat
INT32 GetAvgStatOfPastTeamStat( INT32 iStat );

// render atm panel
BOOLEAN RenderAtmPanel( void );
void DisplayATMAmount( void );

// create destroy ATM button
void CreateDestroyStartATMButton( void );
void CreateDestroyATMButton( void );
void ATMStartButtonCallback(GUI_BUTTON *btn,INT32 reason);
void ATMNumberButtonCallback(GUI_BUTTON *btn,INT32 reason);
void HandleStateOfATMButtons( void );
void ATMOtherButtonCallback(GUI_BUTTON *btn,INT32 reason);


// atm misc functions

void ATMOther2ButtonCallback(GUI_BUTTON *btn,INT32 reason);
void DisplayATMStrings( void );
void DisplayAmountOnCurrentMerc( void );
void RenderRectangleForPersonnelTransactionAmount( void );
void HandleTimedAtmModes( void );



void GameInitPersonnel( void )
{
	// init past characters lists
	INT32 iCounter =0;
	InitPastCharactersList( );

}

void EnterPersonnel( void )
{
	fReDrawScreenFlag=TRUE;
  iStartPersonId=-1;

	iCurrentPersonSelectedId = -1;

	uiCurrentInventoryIndex = 0;
	guiSliderPosition = 0;

	iCurPortraitId = 0;

	// load graphics for screen
	LoadPersonnelGraphics( );

	// show atm panel
	fShowAtmPanelStartButton = TRUE;

	// create buttons needed
	CreateDestroyButtonsForPersonnelDepartures( );

	// load personnel
	LoadPersonnelScreenBackgroundGraphics( );


	// render screen
	RenderPersonnel( );

	// how many people do we have?..if you have someone set default to 0
	if( GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0 )
	{
	  iCurrentPersonSelectedId = GetIdOfFirstDisplayedMerc( );
	}

	fCreatePersonnelPortraitMouseRegions = TRUE;

	CreateDestroyMouseRegionsForPersonnelPortraits( );
	// set states of en- dis able buttons
	//SetPersonnelButtonStates( );

	fCreateRegionsForPastCurrentToggle = TRUE;

	CreateDestroyCurrentDepartedMouseRegions( );

	// create buttons for screen
	CreatePersonnelButtons( );

	// set states of en- dis able buttons
	SetPersonnelButtonStates( );

  return;
}

void ExitPersonnel( void )
{


	if( fCurrentTeamMode == FALSE )
	{
		fCurrentTeamMode = TRUE;
		CreateDestroyButtonsForDepartedTeamList( );
		fCurrentTeamMode = FALSE;
	}

	// get rid of atm panel buttons
	fShowAtmPanelStartButton = FALSE;
	fShowAtmPanel = FALSE;
	fATMFlags = 0;
	CreateDestroyStartATMButton( );
	CreateDestroyATMButton( );

//	fShowInventory = FALSE;
	gubPersonnelInfoState = PRSNL_STATS;

	CreateDestroyPersonnelInventoryScrollButtons( );

	// get rid of graphics
  RemovePersonnelGraphics( );

	 DeletePersonnelScreenBackgroundGraphics(  );

	CreateDestroyButtonsForPersonnelDepartures( );

	// delete buttons
  DeletePersonnelButtons( );

	fCreatePersonnelPortraitMouseRegions = FALSE;

	// delete mouse regions
	CreateDestroyMouseRegionsForPersonnelPortraits( );

	fCreateRegionsForPastCurrentToggle = FALSE;

	CreateDestroyCurrentDepartedMouseRegions( );



	return;
}

void HandlePersonnel( void )
{
   //RenderButtonsFastHelp( );
	CreateDestroyButtonsForPersonnelDepartures( );

	// create / destroy buttons for scrolling departed list
	CreateDestroyButtonsForDepartedTeamList( );

	// enable / disable departures buttons
	EnableDisableDeparturesButtons( );

	// create destroy inv buttons as needed
	CreateDestroyPersonnelInventoryScrollButtons( );

	// enable disable buttons as needed
	EnableDisableInventoryScrollButtons( );

	HandlePersonnelKeyboard( );

	// handle timed modes for ATM
	HandleTimedAtmModes( );

	 return;
}

BOOLEAN LoadPersonnelGraphics( void )
{
	// load graphics needed for personnel screen
  VOBJECT_DESC    VObjectDesc;

	// load graphics

  // title bar
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\programtitlebar.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiTITLE));

	// the background grpahics
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\personnelwindow.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiSCREEN));

	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\personnel_inventory.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiPersonnelInventory));

	return(TRUE);
}

void RemovePersonnelGraphics( void )
{
	// delete graphics needed for personnel screen

	DeleteVideoObjectFromIndex(guiSCREEN);
	DeleteVideoObjectFromIndex(guiTITLE);
	DeleteVideoObjectFromIndex(guiPersonnelInventory);


	return;
}

void RenderPersonnel( void )
{
  HVOBJECT hHandle;
  INT32 iCounter=0;
  // re-renders personnel screen
	// render main background

  // blit title
  GetVideoObject(&hHandle, guiTITLE);
  BltVideoObject( FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2, VO_BLT_SRCTRANSPARENCY,NULL );

	// blit screen
	GetVideoObject(&hHandle, guiSCREEN);
  BltVideoObject( FRAME_BUFFER, hHandle, 0,LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y + 22, VO_BLT_SRCTRANSPARENCY,NULL );

  // render pictures of mercs on scnree
 	//RenderPersonnelPictures( );

	// display header for screen
	//DisplayHeader( );

	// what page are we on?..display it
	//DrawPageNumber( );

		// display border
	//GetVideoObject(&hHandle, guiLaptopBACKGROUND);
	//BltVideoObject(FRAME_BUFFER, hHandle, 0,108, 23, VO_BLT_SRCTRANSPARENCY,NULL);


	// invalidte the region we blitted to
	//InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_LR_Y);

	// render personnel screen background
	RenderPersonnelScreenBackground( );




	// show team
	DisplayPicturesOfCurrentTeam( );

	DisplayPastMercsPortraits( );

	// show selected merc
	DisplayFaceOfDisplayedMerc( );

	// show current team size
	DisplayNumberOnCurrentTeam( );


	// show departed team size
	DisplayNumberDeparted( );

	// list stats row headers for team stats list
	DisplayPersonnelTeamStats( );

	// showinventory of selected guy if applicable
	DisplayInventoryForSelectedChar( );

	// the average stats for the current team
	DisplayAverageStatValuesForCurrentTeam( );

	// lowest stat values
	DisplayLowestStatValuesForCurrentTeam( );


	// past team
	DisplayStateOfPastTeamMembers( );



	// title bar
	BlitTitleBarIcons(  );

	// show text on titlebar
	DisplayPersonnelTextOnTitleBar( );

	// the highest stats
	DisplayHighestStatValuesForCurrentTeam( );

	// render the atm panel
	RenderAtmPanel( );

	DisplayAmountOnCurrentMerc( );

	// en-dis-able start button
	UpDateStateOfStartButton( );

	return;
}


BOOLEAN RenderPersonnelPictures( void )
{
	// will render portraits of personnel onscreen
	// find person with iStartPersonId, unless it is -1, then find first bActive Merc on Staff
	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	BOOLEAN fFound=FALSE;
	INT32 iCounter=0;
	INT32 iSlot=0;
	INT32 cnt=0;
	INT32 iCurrentId = 0;


	pSoldier = MercPtrs[ cnt ];
	pTeamSoldier = pSoldier;


	if(iStartPersonId==-1)
	{
   cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;
   for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++, pSoldier++ )
	 {
		 if ( pSoldier->bLife >= OKLIFE && pSoldier->bActive )
		 {
			 fFound = TRUE;
			 iStartPersonId=cnt;
			 break;
		 }
	 }
   if(!fFound)
		return (FALSE);
	}
	else
	{
   iCurrentId=iStartPersonId;
	 fFound=TRUE;
	 cnt=iCurrentId;
	}



	while(fFound)
	{
	  // the soldier's ID is found
	  // render Face
	  fFound=FALSE;
		RenderPersonnelFace(iCurrentId, iSlot, FALSE, FALSE, FALSE );
		// draw stats
    RenderPersonnelStats(iCurrentId, iSlot);
		DisplayCharName( iCurrentId, iSlot);
		//find next guy
		pSoldier=MercPtrs[iCurrentId];
		cnt++;
    for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
		{
		  if ( pTeamSoldier->bLife >= OKLIFE && pTeamSoldier->bActive )
			{
				if( pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE )
				{
					return( FALSE );
				}

			  fFound = TRUE;
				iSlot++;
			  break;
			}
		}
		if(iSlot>=MAX_SLOTS)
			fFound=FALSE;
    iCurrentId=cnt;
	}

	return(TRUE);
}

void RenderPersonnelStats( INT32 iId, INT32 iSlot )
{

	INT32 iCounter=0;
	// will render the stats of person iId in slot iSlot
	SetFont(PERS_FONT);
	SetFontForeground(PERS_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);

	//for(iCounter=0; iCounter <PERS_COUNT; iCounter++)
	// mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*IMAGE_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);


	if( gubPersonnelInfoState == PERSONNEL_STAT_BTN )
	{
		DisplayCharStats(iId, iSlot);
	}
	else if( gubPersonnelInfoState == PERSONNEL_EMPLOYMENT_BTN )
	{
		DisplayEmploymentinformation( iId, iSlot );
	}
}

BOOLEAN RenderPersonnelFace(INT32 iId, INT32 iSlot, BOOLEAN fDead, BOOLEAN fFired, BOOLEAN fOther )
{
	char sTemp[100];
	HVOBJECT hFaceHandle;
  VOBJECT_DESC    VObjectDesc;
	INT32 iCounter = 7;

	// draw face to soldier iId in slot iSlot

	// special case?..player generated merc
	if( fCurrentTeamMode == TRUE )
	{

		if( ( 50 < 	MercPtrs[iId] -> ubProfile )&&( 57 > 	MercPtrs[iId] -> ubProfile ) )
		{
			sprintf( sTemp, "%s%03d.sti", FACES_DIR, 	gMercProfiles[ MercPtrs[iId] -> ubProfile  ].ubFaceIndex );
		}
		else
		{
			sprintf(sTemp, "%s%02d.sti", FACES_DIR,	Menptr[ iId ].ubProfile);
		}
	}
	else
	{
		//if this is not a valid merc
		if( !fDead && !fFired && !fOther )
		{
			return( TRUE );
		}

		if( ( 50 < 	iId )&&( 57 > 	iId ) )
		{
			sprintf( sTemp, "%s%03d.sti", FACES_DIR, 	gMercProfiles[ iId  ].ubFaceIndex );
		}
		else
		{
			sprintf(sTemp, "%s%02d.sti", FACES_DIR,	 iId );
		}
	}

	if( fCurrentTeamMode == TRUE )
	{
		if( MercPtrs[iId]->uiStatusFlags & SOLDIER_VEHICLE )
		{
			return( TRUE );
		}
	}

	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP(sTemp, VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiFACE));

	//Blt face to screen to
	GetVideoObject(&hFaceHandle, guiFACE);

	if( fCurrentTeamMode == TRUE )
	{
		if( MercPtrs[iId]->bLife <= 0 )
		{
			hFaceHandle->pShades[ 0 ]		= Create16BPPPaletteShaded( hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE );

			//set the red pallete to the face
			SetObjectHandleShade( guiFACE, 0 );
		}
	}
	else
	{
		if( fDead == TRUE )
		{
			hFaceHandle->pShades[ 0 ]		= Create16BPPPaletteShaded( hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE );

			//set the red pallete to the face
			SetObjectHandleShade( guiFACE, 0 );
		}
	}


  BltVideoObject(FRAME_BUFFER, hFaceHandle, 0,IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH), IMAGE_BOX_Y, VO_BLT_SRCTRANSPARENCY,NULL);


	//if the merc is dead, display it
	if( !fCurrentTeamMode )
	{
		INT32 iHeightOfText;

		iHeightOfText = DisplayWrappedString(IMAGE_BOX_X, (UINT16)(IMAGE_BOX_Y+IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[ iId  ].zName, 0, FALSE, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT );

		//if the string will rap
		if( ( iHeightOfText - 2 ) > GetFontHeight( PERS_FONT ) )
		{
			//raise where we display it, and rap it
			DisplayWrappedString(IMAGE_BOX_X, (UINT16)(IMAGE_BOX_Y+IMAGE_FULL_NAME_OFFSET_Y - GetFontHeight( PERS_FONT )), IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[ iId  ].zName, 0, FALSE, CENTER_JUSTIFIED);
		}
		else
		{
			DrawTextToScreen( gMercProfiles[ iId  ].zName, IMAGE_BOX_X, (UINT16)(IMAGE_BOX_Y+IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH, PERS_FONT, PERS_FONT_COLOR, 0, FALSE, CENTER_JUSTIFIED );
		}
//		DrawTextToScreen(gMercProfiles[ iId  ].zName, ( INT16 ) ( IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( INT16 ) ( IMAGE_BOX_Y + 107 ), IMAGE_BOX_WITH_NO_BORDERS, PERS_FONT, PERS_FONT_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
	}

/*
removed cause we already show this under the picture, instead display the mercs FULL name ( above )
	if( fCurrentTeamMode == TRUE )
	{
		if( Menptr[ iId ].bLife <= 0 )
		{
			//if the merc is dead, display it
			DrawTextToScreen(pDepartedMercPortraitStrings[0], ( INT16 ) ( IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( INT16 ) ( IMAGE_BOX_Y + 107 ), IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
		}
	}
	else
	{
		if( fDead )
		{
			//if the merc is dead, display it
			DrawTextToScreen(pDepartedMercPortraitStrings[0], ( INT16 ) ( IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( INT16 ) ( IMAGE_BOX_Y + 107 ), IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
//			DrawTextToScreen( AimPopUpText[ AIM_MEMBER_DEAD ], ( INT16 ) ( IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( INT16 ) ( IMAGE_BOX_Y + 107 ), IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
		}
		else if( fFired )
		{
			//if the merc is dead, display it
			DrawTextToScreen(pDepartedMercPortraitStrings[1], ( INT16 ) ( IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( INT16 ) ( IMAGE_BOX_Y + 107 ), IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
		}
		else if( fOther )
		{
			//if the merc is dead, display it
			DrawTextToScreen(pDepartedMercPortraitStrings[2], ( INT16 ) ( IMAGE_BOX_X+(iSlot*IMAGE_BOX_WIDTH) ), ( INT16 ) ( IMAGE_BOX_Y + 107 ), IMAGE_BOX_WITH_NO_BORDERS, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
		}
	}

*/

	DeleteVideoObjectFromIndex(guiFACE);


	return (TRUE);
}


BOOLEAN NextPersonnelFace( void )
{

	if( iCurrentPersonSelectedId == -1 )
	{
		return ( TRUE );
	}

	if( fCurrentTeamMode == TRUE )
	{
		// wrap around?
		if( iCurrentPersonSelectedId == GetNumberOfMercsDeadOrAliveOnPlayersTeam( ) - 1 )
		{
			iCurrentPersonSelectedId = 0;
			return( FALSE );	//def added 3/14/99 to enable disable buttons properly
		}
		else
		{
			iCurrentPersonSelectedId++;
		}
	}
	else
	{
		if ( ( ( iCurPortraitId + 1) == ( GetNumberOfDeadOnPastTeam( ) + GetNumberOfLeftOnPastTeam( ) + GetNumberOfOtherOnPastTeam( ) ) - giCurrentUpperLeftPortraitNumber ) )
		{
			// about to go off the end
			giCurrentUpperLeftPortraitNumber = 0;
			iCurPortraitId = 0;

		}
		else if(  iCurPortraitId == 19 )
		{
			giCurrentUpperLeftPortraitNumber += 20;
			iCurPortraitId = 0;
		}
		else
		{
			iCurPortraitId++;
		}
		// get of this merc in this slot

		iCurrentPersonSelectedId =  iCurPortraitId;
		fReDrawScreenFlag = TRUE;
	}

		return ( TRUE );
}

BOOLEAN PrevPersonnelFace( void )
{

	if( iCurrentPersonSelectedId == -1 )
	{
		return ( TRUE );
	}

  if( fCurrentTeamMode == TRUE )
	{
		// wrap around?
		if( iCurrentPersonSelectedId == 0  )
		{
			iCurrentPersonSelectedId = GetNumberOfMercsDeadOrAliveOnPlayersTeam( ) - 1;

			if( iCurrentPersonSelectedId == 0 )
			{
				return( FALSE );	//def added 3/14/99 to enable disable buttons properly
			}
		}
		else
		{
			iCurrentPersonSelectedId--;
		}
	}
	else
	{
		if  ( ( iCurPortraitId == 0 ) && ( giCurrentUpperLeftPortraitNumber == 0 ) )
		{
			// about to go off the end
			giCurrentUpperLeftPortraitNumber = ( GetNumberOfDeadOnPastTeam( ) + GetNumberOfLeftOnPastTeam( ) + GetNumberOfOtherOnPastTeam( ) ) - ( GetNumberOfDeadOnPastTeam( ) + GetNumberOfLeftOnPastTeam( ) + GetNumberOfOtherOnPastTeam( ) ) % 20;
			iCurPortraitId = ( GetNumberOfDeadOnPastTeam( ) + GetNumberOfLeftOnPastTeam( ) + GetNumberOfOtherOnPastTeam( ) ) % 20 ;
			iCurPortraitId--;

		}
		else if(  iCurPortraitId == 0 )
		{
			giCurrentUpperLeftPortraitNumber -= 20;
			iCurPortraitId = 19;
		}
		else
		{
			iCurPortraitId--;
		}
		// get of this merc in this slot

		iCurrentPersonSelectedId =  iCurPortraitId;
		fReDrawScreenFlag = TRUE;
	}

	return ( TRUE );
}


void CreatePersonnelButtons( void )
{

	// left button
	giPersonnelButtonImage[0]=  LoadButtonImage( "LAPTOP\\personnelbuttons.sti" ,-1,0,-1,1,-1 );
	giPersonnelButton[0] = QuickCreateButton( giPersonnelButtonImage[0], PREV_MERC_FACE_X, MERC_FACE_SCROLL_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)LeftButtonCallBack);

	// right button
	giPersonnelButtonImage[1]=  LoadButtonImage( "LAPTOP\\personnelbuttons.sti" ,-1,2,-1,3,-1 );
	giPersonnelButton[1] = QuickCreateButton( giPersonnelButtonImage[1], NEXT_MERC_FACE_X, MERC_FACE_SCROLL_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)RightButtonCallBack);



	/*
	// left button
	giPersonnelButtonImage[0]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,0,-1,1,-1 );
	giPersonnelButton[0] = QuickCreateButton( giPersonnelButtonImage[0], LEFT_BUTTON_X, BUTTON_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)LeftButtonCallBack);

	// right button
	giPersonnelButtonImage[1]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,6,-1,7,-1 );
	giPersonnelButton[1] = QuickCreateButton( giPersonnelButtonImage[1], RIGHT_BUTTON_X, BUTTON_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)RightButtonCallBack);

	// left FF button
	giPersonnelButtonImage[2]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,3,-1,4,-1 );
	giPersonnelButton[2] = QuickCreateButton( giPersonnelButtonImage[2], LEFT_BUTTON_X, BUTTON_Y + 22,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)LeftFFButtonCallBack);

	// right ff button
	giPersonnelButtonImage[3]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,9,-1,10,-1 );
	giPersonnelButton[3] = QuickCreateButton( giPersonnelButtonImage[3], RIGHT_BUTTON_X, BUTTON_Y + 22,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)RightFFButtonCallBack);
  */
	// set up cursors
	SetButtonCursor(giPersonnelButton[0], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(giPersonnelButton[1], CURSOR_LAPTOP_SCREEN);
	//SetButtonCursor(giPersonnelButton[2], CURSOR_LAPTOP_SCREEN);
	//SetButtonCursor(giPersonnelButton[3], CURSOR_LAPTOP_SCREEN);


	return;
}

void DeletePersonnelButtons( void )
{

  RemoveButton(giPersonnelButton[0] );
	UnloadButtonImage(giPersonnelButtonImage[0] );
	RemoveButton(giPersonnelButton[1] );
	UnloadButtonImage( giPersonnelButtonImage[1] );
	/*RemoveButton(giPersonnelButton[2] );
	UnloadButtonImage( giPersonnelButtonImage[2] );
	RemoveButton(giPersonnelButton[3] );
	UnloadButtonImage( giPersonnelButtonImage[3] );
	*/
	return;
}


void LeftButtonCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
			fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 fReDrawScreenFlag=TRUE;
		 PrevPersonnelFace( );
		 uiCurrentInventoryIndex = 0;
		 guiSliderPosition = 0;

		}
	}
}

void LeftFFButtonCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
			fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 fReDrawScreenFlag=TRUE;
		 PrevPersonnelFace( );
     PrevPersonnelFace( );
     PrevPersonnelFace( );
		 PrevPersonnelFace( );

		 // set states
		 SetPersonnelButtonStates( );
		}
	}
}

void RightButtonCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
     fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 fReDrawScreenFlag=TRUE;
     NextPersonnelFace( );
		 uiCurrentInventoryIndex = 0;
		 guiSliderPosition = 0;


		}
	}
}

void RightFFButtonCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
     fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		 btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 fReDrawScreenFlag=TRUE;
     NextPersonnelFace( );
     NextPersonnelFace( );
     NextPersonnelFace( );
     NextPersonnelFace( );

		 // set states
		 SetPersonnelButtonStates( );
		}
	}
}

void DisplayHeader( void )
{
  SetFont(PERS_HEADER_FONT);
	SetFontForeground(PERS_FONT_COLOR);
	SetFontBackground( 0 );

	mprintf(pPersonnelScreenPoints[18].x,pPersonnelScreenPoints[18].y,pPersonnelTitle[0]);

	return;
}


void DisplayCharName( INT32 iId, INT32 iSlot )
{
  // get merc's nickName, assignment, and sector location info
	INT16 sX, sY;
  SOLDIERTYPE *pSoldier;
	CHAR16 sString[ 64 ];
	CHAR16 sTownName[ 256 ];
	INT8 bTownId =  -1;
	INT32	iHeightOfText;

	sTownName[0] = L'\0';

	pSoldier=MercPtrs[iId];

  SetFont(CHAR_NAME_FONT);
	SetFontForeground(PERS_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);

	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		return;
	}

	if( Menptr[iId].bAssignment == ASSIGNMENT_POW )
	{
	}
	else if( Menptr[iId].bAssignment == IN_TRANSIT )
	{
	}
	else
  {
		// name of town, if any
		bTownId = GetTownIdForSector( Menptr[iId].sSectorX, Menptr[iId].sSectorY );

		if( bTownId != BLANK_SECTOR )
		{
			swprintf( sTownName, L"%s", pTownNames[ bTownId ] );
		}
	}



	if( sTownName[0] != L'\0' )
	{
		//nick name - town name
		swprintf( sString, L"%s - %s", gMercProfiles[Menptr[iId].ubProfile].zNickname, sTownName );
	}
	else
	{
		//nick name
		swprintf( sString, L"%s", gMercProfiles[Menptr[iId].ubProfile].zNickname );
	}


	// nick name - assignment
	FindFontCenterCoordinates(IMAGE_BOX_X-5,0,IMAGE_BOX_WIDTH + 90 , 0,sString,CHAR_NAME_FONT, &sX, &sY );

	// check to see if we are going to go off the left edge
	if( sX < pPersonnelScreenPoints[ 0 ].x )
	{
		sX = ( INT16 )pPersonnelScreenPoints[ 0 ].x;
	}

	//Display the mercs name
	mprintf(sX+iSlot*IMAGE_BOX_WIDTH, CHAR_NAME_Y, sString );

	swprintf( sString, L"%s", pPersonnelAssignmentStrings[Menptr[iId].bAssignment]);

	// nick name - assignment
	FindFontCenterCoordinates(IMAGE_BOX_X-5,0,IMAGE_BOX_WIDTH + 90 , 0,sString,CHAR_NAME_FONT, &sX, &sY );

	// check to see if we are going to go off the left edge
	if( sX < pPersonnelScreenPoints[ 0 ].x )
	{
		sX = ( INT16 )pPersonnelScreenPoints[ 0 ].x;
	}

	mprintf(sX+iSlot*IMAGE_BOX_WIDTH, CHAR_LOC_Y, sString );


	//
	// Display the mercs FULL name over top of their portrait
	//

	//first get height of text to be displayed
	iHeightOfText = DisplayWrappedString(IMAGE_BOX_X, (UINT16)(IMAGE_BOX_Y+IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[Menptr[iId].ubProfile].zName, 0, FALSE, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT );

	//if the string will rap
	if( ( iHeightOfText - 2 ) > GetFontHeight( PERS_FONT ) )
	{
		//raise where we display it, and rap it
		DisplayWrappedString(IMAGE_BOX_X, (UINT16)(IMAGE_BOX_Y+IMAGE_FULL_NAME_OFFSET_Y - GetFontHeight( PERS_FONT )), IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[Menptr[iId].ubProfile].zName, 0, FALSE, CENTER_JUSTIFIED);
	}
	else
	{
		DrawTextToScreen( gMercProfiles[Menptr[iId].ubProfile].zName, IMAGE_BOX_X, (UINT16)(IMAGE_BOX_Y+IMAGE_FULL_NAME_OFFSET_Y), IMAGE_NAME_WIDTH, PERS_FONT, PERS_FONT_COLOR, 0, FALSE, CENTER_JUSTIFIED );
	}

/*
Moved so the name of the town will be in the same line as the name


	if( Menptr[iId].bAssignment == ASSIGNMENT_POW )
	{
//		FindFontCenterCoordinates(IMAGE_BOX_X-5,0,IMAGE_BOX_WIDTH, 0,pPOWStrings[ 1 ],CHAR_NAME_FONT, &sX, &sY );
//	  mprintf(sX+iSlot*IMAGE_BOX_WIDTH, CHAR_NAME_Y+20,pPOWStrings[ 1 ] );
	}
	else if( Menptr[iId].bAssignment == IN_TRANSIT )
	{
		return;
	}
	else
  {
		// name of town, if any
		bTownId = GetTownIdForSector( Menptr[iId].sSectorX, Menptr[iId].sSectorY );

		if( bTownId != BLANK_SECTOR )
		{
			FindFontCenterCoordinates( IMAGE_BOX_X - 5, 0, IMAGE_BOX_WIDTH, 0, pTownNames[ bTownId ], CHAR_NAME_FONT, &sX, &sY );
			mprintf( sX + ( iSlot * IMAGE_BOX_WIDTH ), CHAR_NAME_Y + 20, pTownNames[ bTownId ]);
		}
	}
*/

	return;
}


void DisplayCharStats(INT32 iId, INT32 iSlot)
{
	INT32 iCounter=0;
	wchar_t sString[50];
//	wchar_t sStringA[ 50 ];
	INT16 sX, sY;
	UINT32 uiHits = 0;
	SOLDIERTYPE *pSoldier = &Menptr[iId];
	BOOLEAN	fAmIaRobot = AM_A_ROBOT( pSoldier );


	if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
	{
		return;
	}

	// display the stats for a char
	for(iCounter=0;iCounter <MAX_STATS; iCounter++)
	{
		switch(iCounter)
		{
		 case 0:
			 // health
			 if( Menptr[iId].bAssignment != ASSIGNMENT_POW )
			 {
				if( gMercProfiles[Menptr[iId].ubProfile].bLifeDelta > 0 )
				{
					swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bLifeDelta);
				  FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 	  mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
				}

			 //else
			 //{
				 swprintf(sString, L"%d/%d",Menptr[iId].bLife,Menptr[iId].bLifeMax);
			 //}
			 }
			 else
			 {
				 swprintf( sString, pPOWStrings[ 1 ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[PRSNL_TXT_HEALTH]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
			 break;
		 case 1:
			 // agility
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bAgilityDelta > 0 )
				 {
					 swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bAgilityDelta);
					 FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 		 mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
					 swprintf(sString, L"%d",Menptr[iId].bAgility);
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
			 break;
		 case 2:
		  // dexterity
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bDexterityDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bDexterityDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bDexterity);
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
			break;
		 case 3:
		 // strength
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bStrengthDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bStrengthDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bStrength );
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 4:
		  // leadership
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bLeadershipDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bLeadershipDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20 +TEXT_DELTA_OFFSET),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bLeadership );
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 5:
		  // wisdom
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bWisdomDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bWisdomDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
				 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bWisdom );
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 6:
		  // exper
			 if( !fAmIaRobot )
			 {

				 if( gMercProfiles[Menptr[iId].ubProfile].bExpLevelDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bExpLevelDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20 +TEXT_DELTA_OFFSET),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
				 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bExpLevel );
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }


       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 7:
			 //mrkmanship
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bMarksmanshipDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bMarksmanshipDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bMarksmanship);
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }


       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
     case 8:
		  // mech
			 if( !fAmIaRobot )
			 {

				 if( gMercProfiles[Menptr[iId].ubProfile].bMechanicDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bMechanicDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
					 swprintf(sString, L"%d",Menptr[iId].bMechanical);
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
     case 9:
		  // exp
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bExplosivesDelta > 0 )
				 {
						swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bExplosivesDelta);
						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20+TEXT_DELTA_OFFSET ),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 			mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bExplosive);
				 //}

			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }

       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 10:
       // med
			 if( !fAmIaRobot )
			 {
				 if( gMercProfiles[Menptr[iId].ubProfile].bMedicalDelta > 0 )
				 {
					 swprintf(sString, L"( %+d )", gMercProfiles[Menptr[iId].ubProfile].bMedicalDelta);
					 FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20 +TEXT_DELTA_OFFSET),0,30,0,sString, PERS_FONT,  &sX, &sY);
			 		 mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);			 }
				 //else
				 //{
				 		swprintf(sString, L"%d",Menptr[iId].bMedical);
				 //}
			 }
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }


			 mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
			 FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
			 mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;

		 case 14:
		 // kills
      mprintf((INT16)(pPersonnelScreenPoints[21].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[21].y,pPersonnelScreenStrings[PRSNL_TXT_KILLS]);
			swprintf(sString, L"%d",gMercProfiles[Menptr[iId].ubProfile].usKills);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[21].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[21].y,sString);
		 break;
		 case 15:
			// assists
			mprintf((INT16)(pPersonnelScreenPoints[22].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[22].y,pPersonnelScreenStrings[PRSNL_TXT_ASSISTS]);
			swprintf(sString, L"%d",gMercProfiles[Menptr[iId].ubProfile].usAssists);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[22].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[22].y,sString);
		 break;
		  case 16:
			// shots/hits
			mprintf((INT16)(pPersonnelScreenPoints[23].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[23].y,pPersonnelScreenStrings[PRSNL_TXT_HIT_PERCENTAGE]);
			uiHits = ( UINT32 )gMercProfiles[Menptr[iId].ubProfile].usShotsHit;
			uiHits *= 100;

			// check we have shot at least once
			if( gMercProfiles[Menptr[iId].ubProfile].usShotsFired > 0 )
			{
			  uiHits /= ( UINT32 )gMercProfiles[Menptr[iId].ubProfile].usShotsFired;
			}
			else
			{
				// no, set hit % to 0
				uiHits = 0;
			}


			swprintf(sString, L"%d %%%%",uiHits);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[23].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      sX += StringPixLength( L"%",  PERS_FONT );
			mprintf(sX,pPersonnelScreenPoints[23].y,sString);
		 break;
		  case 17:
			// battles
			mprintf((INT16)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[24].y,pPersonnelScreenStrings[PRSNL_TXT_BATTLES]);
			swprintf(sString, L"%d",gMercProfiles[Menptr[iId].ubProfile].usBattlesFought);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[24].y,sString);
		 break;
		 case 18:
			// wounds
			mprintf((INT16)(pPersonnelScreenPoints[25].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[25].y,pPersonnelScreenStrings[PRSNL_TXT_TIMES_WOUNDED]);
			swprintf(sString, L"%d",gMercProfiles[Menptr[iId].ubProfile].usTimesWounded);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[25].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[25].y,sString);
		 break;

			// The Mercs Skills
			case 19:
			{
				INT32 iWidth;
				INT32 iMinimumX;
				INT8	bScreenLocIndex = 19;	//if you change the '19', change it below in the if statement

				//Display the 'Skills' text
				mprintf( (INT16)( pPersonnelScreenPoints[bScreenLocIndex].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[bScreenLocIndex].y,pPersonnelScreenStrings[ PRSNL_TXT_SKILLS ] );

				//KM: April 16, 1999
				//Added support for the German version, which has potential string overrun problems.  For example, the text "Skills:" can
				//overlap "NightOps (Expert)" because the German strings are much longer.  In these cases, I ensure that the right
				//justification of the traits don't overlap.  If it would, I move it over to the right.
				iWidth = StringPixLength( pPersonnelScreenStrings[ PRSNL_TXT_SKILLS ], PERS_FONT );
				iMinimumX = iWidth + pPersonnelScreenPoints[bScreenLocIndex].x + iSlot * TEXT_BOX_WIDTH + 2;

				if( !fAmIaRobot )
				{
					INT8	bSkill1 = gMercProfiles[ Menptr[iId].ubProfile ].bSkillTrait;
					INT8	bSkill2 = gMercProfiles[ Menptr[iId].ubProfile ].bSkillTrait2;

					//if the 2 skills are the same, add the '(expert)' at the end
					if( bSkill1 == bSkill2 && bSkill1 != NO_SKILLTRAIT )
					{
						swprintf( sString, L"%s %s", gzMercSkillText[bSkill1], gzMercSkillText[NUM_SKILLTRAITS] );

						FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[bScreenLocIndex].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);

						//KM: April 16, 1999
						//Perform the potential overrun check
						if( sX <= iMinimumX )
						{
							FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH) + TEXT_BOX_WIDTH-20 +TEXT_DELTA_OFFSET),0,30,0,sString, PERS_FONT,  &sX, &sY);
							sX = (INT16)max( sX, iMinimumX );
						}

						mprintf(sX,pPersonnelScreenPoints[bScreenLocIndex].y,sString);
					}
					else
					{
						//Display the first skill
						if( bSkill1 != NO_SKILLTRAIT )
						{
							swprintf( sString, L"%s", gzMercSkillText[bSkill1] );

							FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[bScreenLocIndex].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);

							//KM: April 16, 1999
							//Perform the potential overrun check
							sX = (INT16)max( sX, iMinimumX );

							mprintf(sX,pPersonnelScreenPoints[bScreenLocIndex].y,sString);

							bScreenLocIndex++;
						}

						//Display the second skill
						if( bSkill2 != NO_SKILLTRAIT )
						{
							swprintf( sString, L"%s", gzMercSkillText[bSkill2] );

							FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[bScreenLocIndex].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);

							//KM: April 16, 1999
							//Perform the potential overrun check
							sX = (INT16)max( sX, iMinimumX );

							mprintf(sX,pPersonnelScreenPoints[bScreenLocIndex].y,sString);

							bScreenLocIndex++;
						}

						//if no skill was displayed
						if( bScreenLocIndex == 19 )
						{
							swprintf( sString, L"%s", pPersonnelScreenStrings[ PRSNL_TXT_NOSKILLS ] );

							FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[bScreenLocIndex].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
							mprintf(sX,pPersonnelScreenPoints[bScreenLocIndex].y,sString);
						}
					}
				}
			 else
			 {
				 swprintf( sString, L"%s", gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
			 }
			}
			break;
/*
		 case 19:
			 // total contract time served
			mprintf((INT16)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[24].y,pPersonnelScreenStrings[18]);
			if( gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed > 0 )
			{
				swprintf(sString, L"%d %s",gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed - 1, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ] );
			}
			else
			{
				swprintf(sString, L"%d %s",gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ] );
			}
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[24].y,sString);
		 break;
*/
		}
	}
	return;
}


INT32 GetLastMercId( void )
{
	// rolls through list of mercs and returns how many on team
  SOLDIERTYPE *pSoldier, *pTeamSoldier;
  INT32 cnt=0;
	INT32 iCounter=0;
	pSoldier = MercPtrs[0];

	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
		{
		 if( ( pTeamSoldier->bActive ) && ( pTeamSoldier->bLife > 0 ) )
			 iCounter++;
		}
	return iCounter;
}


void DrawPageNumber( void )
{
	// draws the page number

	wchar_t sString[10];
	INT16 sX, sY;
  INT32 iPageNumber, iLastPage;

	return;

	// get last page number, and current page too
	iLastPage=GetLastMercId()/MAX_SLOTS;
	iPageNumber=iStartPersonId/MAX_SLOTS;
  iPageNumber++;
	if(iLastPage==0)
	 iLastPage++;

	// get current and last pages
	swprintf(sString, L"%d/%d", iPageNumber, iLastPage);

	// set up font
	SetFont(PERS_FONT);
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	// center
	FindFontCenterCoordinates(PAGE_X, PAGE_Y,PAGE_BOX_WIDTH, PAGE_BOX_HEIGHT, sString, PERS_FONT, &sX, &sY);

	// print page number
	mprintf(sX, sY, sString);

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);

	return;
}


void SetPersonnelButtonStates( void )
{
	// this function will look at what page we are viewing, enable and disable buttons as needed

	if( ! PrevPersonnelFace( ) )
	{
		// first page, disable left buttons

//		DisableButton( 	giPersonnelButton[ 2 ] );
		DisableButton( 	giPersonnelButton[ 0 ] );
	}
	else
	{
		// enable buttons
    NextPersonnelFace( );

		// enable buttons
//		EnableButton( giPersonnelButton[ 2 ] );
		EnableButton( giPersonnelButton[ 0 ] );
	}

	if( ! NextPersonnelFace( ) )
	{

//		DisableButton( 	giPersonnelButton[ 3 ] );
		DisableButton( 	giPersonnelButton[ 1 ] );
	}
	else
	{
    // decrement page
    PrevPersonnelFace( );
			// enable buttons
//		EnableButton( giPersonnelButton[ 3 ] );
		EnableButton( giPersonnelButton[ 1 ] );
	}

	return;
}


void RenderPersonnelScreenBackground( void )
{

	HVOBJECT hHandle;

	// this fucntion will render the background for the personnel screen
	if( fCurrentTeamMode == TRUE )
	{
		// blit title
    GetVideoObject(&hHandle, guiCURRENTTEAM);

	}
	else
	{
			// blit title
    GetVideoObject(&hHandle, guiDEPARTEDTEAM);
	}

	BltVideoObject( FRAME_BUFFER, hHandle, 0,DEPARTED_X, DEPARTED_Y, VO_BLT_SRCTRANSPARENCY,NULL );

	return;
}


BOOLEAN LoadPersonnelScreenBackgroundGraphics( void )
{
	// will load the graphics for the personeel screen background
	VOBJECT_DESC    VObjectDesc;

	// departed bar
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\departed.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiDEPARTEDTEAM));

	// current bar
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\CurrentTeam.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiCURRENTTEAM));


	return ( TRUE );
}

void DeletePersonnelScreenBackgroundGraphics( void )
{
	// delete background V/O's

	DeleteVideoObjectFromIndex( guiCURRENTTEAM );
	DeleteVideoObjectFromIndex( guiDEPARTEDTEAM );

}

void CreateDestroyButtonsForPersonnelDepartures( void )
{
	static BOOLEAN fCreated = FALSE;

	// create/ destroy personnel departures buttons as needed

	// create button?..if not created
	if( ( fCreatePeronnelDepartureButton == TRUE)&&( fCreated == FALSE ) )
	{
		fCreated = TRUE;
	}
	else if( ( fCreatePeronnelDepartureButton == FALSE ) && ( fCreated == TRUE ) )
	{
		fCreated = FALSE;
	}


	return;
}


INT32 GetNumberOfMercsOnPlayersTeam( void )
{

	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt=0;
	INT32 iCounter = 0;




  // grab number on team
	pSoldier = MercPtrs[0];

	// no soldiers

	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++, pTeamSoldier++)
	{
		if( ( pTeamSoldier->bActive) && !( pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && ( pTeamSoldier->bLife > 0 ) )
			iCounter++;
	}

	return ( iCounter );
}


INT32 GetNumberOfMercsDeadOrAliveOnPlayersTeam( void )
{

	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt=0;
	INT32 iCounter = 0;




  // grab number on team
	pSoldier = MercPtrs[0];

	// no soldiers

	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++, pTeamSoldier++)
	{
		if( ( pTeamSoldier->bActive) && !( pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
			iCounter++;
	}

	return ( iCounter );
}

void  CreateDestroyMouseRegionsForPersonnelPortraits( void )
{
	// creates/ destroys mouse regions for portraits

	static BOOLEAN fCreated = FALSE;
	INT16 sCounter = 0;

	if( ( fCreated == FALSE )&&( fCreatePersonnelPortraitMouseRegions == TRUE ) )
	{
		// create regions
		for( sCounter = 0; sCounter < PERSONNEL_PORTRAIT_NUMBER; sCounter++ )
		{
			MSYS_DefineRegion(&gPortraitMouseRegions[ sCounter ], ( INT16 ) ( SMALL_PORTRAIT_START_X + ( sCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y +  ( sCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT ) , ( INT16 ) ( ( SMALL_PORTRAIT_START_X ) + ( ( sCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH )  +  SMALL_PORTRAIT_WIDTH ) , ( INT16 )( SMALL_PORTRAIT_START_Y + ( sCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT + SMALL_PORTRAIT_HEIGHT ),
		    MSYS_PRIORITY_HIGHEST,CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelPortraitCallback);
			MSYS_SetRegionUserData( &gPortraitMouseRegions[ sCounter ] ,0 , sCounter );
			MSYS_AddRegion( &gPortraitMouseRegions[ sCounter ] );
		}

		fCreated = TRUE;

	}
	else if( ( fCreated == TRUE ) && ( fCreatePersonnelPortraitMouseRegions == FALSE ) )
	{
		// destroy regions
		for( sCounter = 0; sCounter < PERSONNEL_PORTRAIT_NUMBER; sCounter++ )
		{
		  MSYS_RemoveRegion( &gPortraitMouseRegions[ sCounter ] );
		}

		fCreated = FALSE;
	}
	return;
}


BOOLEAN DisplayPicturesOfCurrentTeam( void )
{
	INT32 iCounter = 0;
	INT32 iTotalOnTeam = 0;
	char sTemp[100];
	HVOBJECT hFaceHandle;
  VOBJECT_DESC    VObjectDesc;
	SOLDIERTYPE *pSoldier;
	INT32 iId = 0;
	INT32 iCnt = 0;

	// will display the 20 small portraits of the current team

	// get number of mercs on team
	iTotalOnTeam = GetNumberOfMercsDeadOrAliveOnPlayersTeam( );


	if( ( iTotalOnTeam == 0)||( fCurrentTeamMode == FALSE ) )
	{
		// nobody on team, leave
		return ( TRUE );
	}

	pSoldier = MercPtrs[ iCounter ];

	// start id
	iId= gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;

	for( iCounter = 0; iCounter < iTotalOnTeam; iCnt++ )
	{

		if( ( MercPtrs[iId + iCnt ] -> bActive == TRUE ) )
		{
			// found the next actual guy
			if( ( 50 < 	MercPtrs[iId + iCnt ]  -> ubProfile )&&( 57 > 	MercPtrs[iId + iCnt ] -> ubProfile ) )
			{
				sprintf( sTemp, "%s%03d.sti", SMALL_FACES_DIR, 	gMercProfiles[ MercPtrs[ iId + iCnt ] -> ubProfile  ].ubFaceIndex );
			}
			else
			{
				if( Menptr[ iId + iCnt ].ubProfile < 100 )
				{
					sprintf(sTemp, "%s%02d.sti", SMALL_FACES_DIR,	Menptr[ iId + iCnt ].ubProfile);
				}
				else
				{
					sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR,	Menptr[ iId + iCnt ].ubProfile);
				}
			}

		VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	  FilenameForBPP(sTemp, VObjectDesc.ImageFile);
		CHECKF(AddVideoObject(&VObjectDesc, &guiFACE));

		//Blt face to screen to
		GetVideoObject(&hFaceHandle, guiFACE);

		if( Menptr[ iId + iCnt ].bLife <= 0 )
		{
			hFaceHandle->pShades[ 0 ]		= Create16BPPPaletteShaded( hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE );

			//set the red pallete to the face
			SetObjectHandleShade( guiFACE, 0 );
		}



		BltVideoObject(FRAME_BUFFER, hFaceHandle, 0,( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT ), VO_BLT_SRCTRANSPARENCY,NULL);

		if( Menptr[ iId + iCnt ].bLife <= 0 )
		{
			//if the merc is dead, display it
			DrawTextToScreen(AimPopUpText[AIM_MEMBER_DEAD], ( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
		}

		DeleteVideoObjectFromIndex(guiFACE);
		iCounter++;
		}
	}

	return ( TRUE );
}



void PersonnelPortraitCallback( MOUSE_REGION * pRegion, INT32 iReason )
{
	INT32 iPortraitId = 0;
	INT32 iOldPortraitId;

	iPortraitId = MSYS_GetRegionUserData(pRegion, 0);
	iOldPortraitId = iCurrentPersonSelectedId;

	// callback handler for the minize region that is attatched to the laptop program icon
	if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
		// get id of portrait

		if( fCurrentTeamMode == TRUE )
		{
		  // valid portrait, set up id
			if( iPortraitId >= GetNumberOfMercsDeadOrAliveOnPlayersTeam( ) )
			{
			  // not a valid id, leave
			  return;
			}

		  iCurrentPersonSelectedId = iPortraitId;
			fReDrawScreenFlag = TRUE;
		}
		else
		{
			if( iPortraitId >= GetNumberOfPastMercsOnPlayersTeam( ) )
			{
				return;
			}
			iCurrentPersonSelectedId = iPortraitId;
			fReDrawScreenFlag = TRUE;
			iCurPortraitId = iPortraitId;
		}

		//if the selected merc is valid, and they are a POW, change to the inventory display
		if( iCurrentPersonSelectedId != -1 && Menptr[ GetIdOfThisSlot( iCurrentPersonSelectedId ) ].bAssignment == ASSIGNMENT_POW && gubPersonnelInfoState == PERSONNEL_INV_BTN )
		{
			gubPersonnelInfoState = PERSONNEL_STAT_BTN;
		}

		if( iOldPortraitId != iPortraitId )
		{
			uiCurrentInventoryIndex = 0;
			guiSliderPosition = 0;
		}
	}

	if( iReason & MSYS_CALLBACK_REASON_RBUTTON_UP )
	{
		if( fCurrentTeamMode == TRUE )
		{
		  // valid portrait, set up id
			if( iPortraitId >= GetNumberOfMercsDeadOrAliveOnPlayersTeam( ) )
			{
			  // not a valid id, leave
			  return;
			}

			//if the user is rigt clicking on the same face
			if( iCurrentPersonSelectedId == iPortraitId )
			{
				//increment the info page when the user right clicks
				if( gubPersonnelInfoState < PERSONNEL_NUM_BTN-1 )
					gubPersonnelInfoState++;
				else
					gubPersonnelInfoState = PERSONNEL_STAT_BTN;
			}

			iCurrentPersonSelectedId = iPortraitId;
			fReDrawScreenFlag = TRUE;

			uiCurrentInventoryIndex = 0;
			guiSliderPosition = 0;

			//if the selected merc is valid, and they are a POW, change to the inventory display
			if( iCurrentPersonSelectedId != -1 && Menptr[ GetIdOfThisSlot( iCurrentPersonSelectedId ) ].bAssignment == ASSIGNMENT_POW && gubPersonnelInfoState == PERSONNEL_INV_BTN)
			{
				gubPersonnelInfoState = PERSONNEL_STAT_BTN;
			}
		}
	}
}


void DisplayFaceOfDisplayedMerc( )
{
	// valid person?, display

	if( iCurrentPersonSelectedId != -1 )
	{
		// highlight it
		DisplayHighLightBox( );

		// if showing inventory, leave

		if( fCurrentTeamMode == TRUE )
		{
			RenderPersonnelFace(  GetIdOfThisSlot( iCurrentPersonSelectedId ), 0, FALSE, FALSE, FALSE );
			DisplayCharName(  GetIdOfThisSlot( iCurrentPersonSelectedId ), 0 );

//			if( fShowInventory == TRUE )
			if( gubPersonnelInfoState == PRSNL_INV )
			{
				return;
			}

	    RenderPersonnelStats(  GetIdOfThisSlot( iCurrentPersonSelectedId ), 0 );

		}
		else
		{
			RenderPersonnelFace(  GetIdOfPastMercInSlot( iCurrentPersonSelectedId ), 0, IsPastMercDead( iCurrentPersonSelectedId ), IsPastMercFired( iCurrentPersonSelectedId ), IsPastMercOther( iCurrentPersonSelectedId ) );
			DisplayDepartedCharName(  GetIdOfPastMercInSlot( iCurrentPersonSelectedId ), 0, GetTheStateOfDepartedMerc( GetIdOfPastMercInSlot( iCurrentPersonSelectedId  ) ) );

//			if( fShowInventory == TRUE )
			if( gubPersonnelInfoState == PRSNL_INV )
			{
				return;
			}

			DisplayDepartedCharStats( GetIdOfPastMercInSlot( iCurrentPersonSelectedId ), 0, GetTheStateOfDepartedMerc( GetIdOfPastMercInSlot( iCurrentPersonSelectedId  ) ) );

		}


	}

	return;
}

void DisplayInventoryForSelectedChar( void )
{

	// display the inventory for this merc
//	if( fShowInventory == FALSE )
	if( gubPersonnelInfoState != PRSNL_INV )
	{
		return;
	}

	CreateDestroyPersonnelInventoryScrollButtons( );

	if( fCurrentTeamMode == TRUE )
	{
		RenderInventoryForCharacter( GetIdOfThisSlot( iCurrentPersonSelectedId ), 0 );
	}
	else
	{
		RenderInventoryForCharacter( GetIdOfPastMercInSlot( iCurrentPersonSelectedId ), 0 );
	}

	return;
}


void RenderInventoryForCharacter( INT32 iId, INT32 iSlot )
{
	UINT8 ubCounter = 0;
	SOLDIERTYPE *pSoldier;
	INT16 sIndex;
	HVOBJECT hHandle;
	ETRLEObject	*pTrav;
	INVTYPE			*pItem;
	INT16				PosX, PosY, sCenX, sCenY;
	UINT32			usHeight, usWidth;
	UINT8 ubItemCount = 0;
	UINT8 ubUpToCount = 0;
	INT16 sX, sY;
	CHAR16 sString[ 128 ];
	INT32 cnt = 0;
	INT32 iTotalAmmo = 0;

	GetVideoObject(&hHandle, guiPersonnelInventory);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,( INT16 ) ( 397 ), ( INT16 ) ( 200 ), VO_BLT_SRCTRANSPARENCY,NULL);

	if( fCurrentTeamMode == FALSE )
	{
		return;
	}

	// render the bar for the character
	RenderSliderBarForPersonnelInventory( );

	pSoldier = &Menptr[ iId ];

	//if this is a robot, dont display any inventory
	if( AM_A_ROBOT( pSoldier ) )
	{
		return;
	}

	for( ubCounter = 0; ubCounter < NUM_INV_SLOTS; ubCounter++ )
	{
		PosX = 397 + 3;
		PosY = 200 + 8 +( ubItemCount * ( 29 ) );

		//if the character is a robot, only display the inv for the hand pos
		if( pSoldier->ubProfile == ROBOT && ubCounter != HANDPOS )
		{
			continue;
		}

		if( pSoldier->inv[ ubCounter ].ubNumberOfObjects )
		{
			if( uiCurrentInventoryIndex > ubUpToCount )
			{
				ubUpToCount++;
			}
			else
			{
				sIndex = ( pSoldier->inv[ ubCounter ].usItem );
				pItem = &Item[ sIndex ];

				GetVideoObject( &hHandle, GetInterfaceGraphicForItem( pItem ) );
				pTrav = &(hHandle->pETRLEObject[ pItem->ubGraphicNum ] );

				usHeight				= (UINT32)pTrav->usHeight;
				usWidth					= (UINT32)pTrav->usWidth;

				sCenX = PosX + ( abs( 57 - usWidth ) /  2 ) - pTrav->sOffsetX;
				sCenY = PosY + ( abs( 22 - usHeight ) / 2 ) - pTrav->sOffsetY;

				// shadow
				//BltVideoObjectOutlineShadowFromIndex( FRAME_BUFFER, GetInterfaceGraphicForItem( pItem ), pItem->ubGraphicNum, sCenX-2, sCenY+2);

				//blt the item
				BltVideoObjectOutlineFromIndex( FRAME_BUFFER, GetInterfaceGraphicForItem( pItem ), pItem->ubGraphicNum, sCenX, sCenY, 0, FALSE );

				SetFont( FONT10ARIAL );
				SetFontForeground( FONT_WHITE );
				SetFontBackground( FONT_BLACK );
				SetFontDestBuffer( FRAME_BUFFER, 0,0,640, 480, FALSE );

				// grab item name
				LoadItemInfo(sIndex, sString, NULL );

				// shorten if needed
				if( StringPixLength( sString, FONT10ARIAL) > ( 171 - 75 ) )
				{
					ReduceStringLength( sString, ( 171 - 75 ), FONT10ARIAL );
				}

				// print name
				mprintf( PosX + 65, PosY + 3, sString );

				// condition
				if ( Item[pSoldier->inv[ ubCounter ].usItem ].usItemClass & IC_AMMO )
				{
					// Ammo
					iTotalAmmo = 0;
					if( pSoldier->inv[ ubCounter ].ubNumberOfObjects > 1 )
					{
						for( cnt = 0; cnt < pSoldier->inv[ ubCounter ].ubNumberOfObjects; cnt++ )
						{
							// get total ammo
							iTotalAmmo+= pSoldier->inv[ ubCounter ].ubShotsLeft[cnt];
						}
					}
					else
					{
							iTotalAmmo= pSoldier->inv[ ubCounter ].ubShotsLeft[ 0 ];
					}

					swprintf( sString, L"%d/%d", iTotalAmmo, ( pSoldier->inv[ ubCounter ].ubNumberOfObjects * Magazine[ Item[pSoldier->inv[ ubCounter ].usItem ].ubClassIndex ].ubMagSize ) );
					FindFontRightCoordinates( ( INT16 )( PosX + 65 ), ( INT16 ) ( PosY + 15 ), ( INT16 ) ( 171 - 75 ),
					( INT16 )( GetFontHeight( FONT10ARIAL ) ), sString, FONT10ARIAL, &sX, &sY );
				}
				else
				{
						swprintf( sString, L"%2d%%%%", pSoldier->inv[ ubCounter ].bStatus[0] );
						FindFontRightCoordinates( ( INT16 )( PosX + 65 ), ( INT16 ) ( PosY + 15 ), ( INT16 ) ( 171 - 75 ),
							( INT16 )( GetFontHeight( FONT10ARIAL ) ), sString, FONT10ARIAL, &sX, &sY );

						sX += StringPixLength( L"%", FONT10ARIAL );
				}



				mprintf( sX, sY, sString );

				if ( Item[pSoldier->inv[ ubCounter ].usItem ].usItemClass & IC_GUN )
				{
					swprintf( sString, L"%s", AmmoCaliber[ Weapon[ Item[  pSoldier->inv[ ubCounter ].usItem ].ubClassIndex ].ubCalibre ]);

					// shorten if needed
					if( StringPixLength( sString, FONT10ARIAL) > ( 171 - 75 ) )
					{
						ReduceStringLength( sString, ( 171 - 75 ), FONT10ARIAL );
					}

					// print name
					mprintf( PosX + 65, PosY + 15 , sString );


				}

				// if more than 1?
				if( pSoldier->inv[ ubCounter ].ubNumberOfObjects > 1 )
				{
					swprintf( sString, L"x%d",  pSoldier->inv[ ubCounter ].ubNumberOfObjects );
					FindFontRightCoordinates( ( INT16 )( PosX ), ( INT16 ) ( PosY + 15 ), ( INT16 ) ( 58 ),
						( INT16 )( GetFontHeight( FONT10ARIAL ) ), sString, FONT10ARIAL, &sX, &sY );
					mprintf( sX, sY, sString );
				}

				// display info about it

				ubItemCount++;
			}
		}

		if( ubItemCount == NUMBER_OF_INVENTORY_PERSONNEL )
		{
			ubCounter = NUM_INV_SLOTS;
		}
	}



	return;
}


void InventoryUpButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	INT32 iValue = 0;
	SOLDIERTYPE *pSoldier = MercPtrs[ 0 ];
	INT32 cnt = 0;
	INT32 iId = 0;

	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;


	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( btn->uiFlags & (BUTTON_CLICKED_ON) )
		{
			btn->uiFlags &= ~(BUTTON_CLICKED_ON);

			if( uiCurrentInventoryIndex == 0 )
			{
				return;
			}

			// up one element
			uiCurrentInventoryIndex--;
			fReDrawScreenFlag = TRUE;

			FindPositionOfPersInvSlider( );
		}
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT )
	{
		if( uiCurrentInventoryIndex == 0 )
		{
			return;
		}

		// up one element
		uiCurrentInventoryIndex--;
		fReDrawScreenFlag = TRUE;
		FindPositionOfPersInvSlider( );
	}
}


void InventoryDownButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	INT32 iValue = 0;
	SOLDIERTYPE *pSoldier = MercPtrs[ 0 ];
	INT32 cnt = 0;
	INT32 iId = 0;

	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;


	if(reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT )
	{
		if( ( INT32 )uiCurrentInventoryIndex >= ( INT32 )( GetNumberOfInventoryItemsOnCurrentMerc( ) - NUMBER_OF_INVENTORY_PERSONNEL ) )
		{
			return;
		}

		// up one element
		uiCurrentInventoryIndex++;
		fReDrawScreenFlag = TRUE;
		FindPositionOfPersInvSlider( );

	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( btn->uiFlags & (BUTTON_CLICKED_ON) )
		{
			btn->uiFlags &= ~(BUTTON_CLICKED_ON);

			if( ( INT32 )uiCurrentInventoryIndex >= ( INT32 )( GetNumberOfInventoryItemsOnCurrentMerc( ) - NUMBER_OF_INVENTORY_PERSONNEL ) )
			{
				return;
			}

			// up one element
			uiCurrentInventoryIndex++;
			fReDrawScreenFlag = TRUE;

			FindPositionOfPersInvSlider( );

		}
	}

}


// decide which buttons can and can't be accessed based on what the current item is
void EnableDisableInventoryScrollButtons( void )
{
//	if( fShowInventory == FALSE )
	if( gubPersonnelInfoState != PRSNL_INV )
	{
		return;
	}

	if( uiCurrentInventoryIndex == 0 )
	{
		ButtonList[ giPersonnelInventoryButtons[ 0 ] ]->uiFlags &= ~( BUTTON_CLICKED_ON );
		DisableButton( giPersonnelInventoryButtons[ 0 ] );
	}
	else
	{
		EnableButton( giPersonnelInventoryButtons[ 0 ] );
	}


	if( ( INT32 )uiCurrentInventoryIndex >= ( INT32 )( GetNumberOfInventoryItemsOnCurrentMerc( ) - NUMBER_OF_INVENTORY_PERSONNEL ) )
	{
		ButtonList[ giPersonnelInventoryButtons[ 1 ] ]->uiFlags &= ~( BUTTON_CLICKED_ON );
		DisableButton( giPersonnelInventoryButtons[ 1 ] );
	}
	else
	{
		EnableButton( giPersonnelInventoryButtons[ 1 ] );
	}


	return;
}


INT32 GetNumberOfInventoryItemsOnCurrentMerc( void )
{
	INT32 iId = 0;
	UINT8 ubCounter = 0;
	UINT8 ubCount = 0;
	SOLDIERTYPE *pSoldier;

	// in current team mode?..nope...move on
	if( fCurrentTeamMode == FALSE )
	{
		return( 0 );
	}

	iId = GetIdOfThisSlot( iCurrentPersonSelectedId );

	pSoldier = &Menptr[ iId ];

	for( ubCounter = 0; ubCounter < NUM_INV_SLOTS; ubCounter++ )
	{
		if( ( pSoldier->inv[ ubCounter ].ubNumberOfObjects ) && ( pSoldier->inv[ ubCounter ].usItem) )
		{
			ubCount++;
		}
	}

	return( ubCount );
}

void CreateDestroyPersonnelInventoryScrollButtons( void )
{
	static BOOLEAN fCreated = FALSE;

//	if( ( fShowInventory == TRUE ) && ( fCreated == FALSE ) )
	if( ( gubPersonnelInfoState == PRSNL_INV ) && ( fCreated == FALSE ) )
	{
		// create buttons
		giPersonnelInventoryButtonsImages[ 0 ]=  LoadButtonImage( "LAPTOP\\personnel_inventory.sti" ,-1,1,-1,2,-1 );
	  giPersonnelInventoryButtons[ 0 ] = QuickCreateButton( giPersonnelInventoryButtonsImages[0], 176 + 397, 2 + 200,
	                    BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)InventoryUpButtonCallback);

		giPersonnelInventoryButtonsImages[ 1 ]=  LoadButtonImage( "LAPTOP\\personnel_inventory.sti" ,-1,3,-1,4,-1 );
	  giPersonnelInventoryButtons[ 1 ] = QuickCreateButton( giPersonnelInventoryButtonsImages[1], 397 + 176, 200 + 223,
	                    BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)InventoryDownButtonCallback);

			// set up cursors for these buttons
		SetButtonCursor( giPersonnelInventoryButtons[ 0 ], CURSOR_LAPTOP_SCREEN);
	  SetButtonCursor( giPersonnelInventoryButtons[ 1 ], CURSOR_LAPTOP_SCREEN);

		MSYS_DefineRegion( &gMouseScrollPersonnelINV, X_OF_PERSONNEL_SCROLL_REGION, Y_OF_PERSONNEL_SCROLL_REGION, X_OF_PERSONNEL_SCROLL_REGION + X_SIZE_OF_PERSONNEL_SCROLL_REGION, Y_OF_PERSONNEL_SCROLL_REGION + Y_SIZE_OF_PERSONNEL_SCROLL_REGION,
			MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, HandleSliderBarClickCallback );

		fCreated = TRUE;
	}
//	else if( ( fCreated == TRUE ) && ( fShowInventory == FALSE ) )
	else if( ( fCreated == TRUE ) && ( gubPersonnelInfoState != PERSONNEL_INV_BTN ) )
	{
		// destroy buttons
		RemoveButton(giPersonnelInventoryButtons[0] );
	  UnloadButtonImage(giPersonnelInventoryButtonsImages[0] );
	  RemoveButton(giPersonnelInventoryButtons[1] );
	  UnloadButtonImage(giPersonnelInventoryButtonsImages[1] );

		MSYS_RemoveRegion( &gMouseScrollPersonnelINV );

		fCreated = FALSE;
	}
}

void DisplayNumberOnCurrentTeam( void )
{
	// display number on team
	CHAR16 sString[ 32 ];
	INT16 sX = 0, sY = 0;


	// font stuff
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );

	if( fCurrentTeamMode == TRUE )
	{
		swprintf( sString, L"%s ( %d )", pPersonelTeamStrings[ 0 ], GetNumberOfMercsDeadOrAliveOnPlayersTeam( ) );
		sX = PERS_CURR_TEAM_X;
	}
	else
	{
		swprintf( sString, L"%s", pPersonelTeamStrings[ 0 ] );
		FindFontCenterCoordinates( PERS_CURR_TEAM_X, 0, 65, 0,sString, FONT10ARIAL, &sX, &sY );

	}

	mprintf( sX, PERS_CURR_TEAM_Y, sString );

	// now the cost of the current team, if applicable
	DisplayCostOfCurrentTeam( );

	return;
}

void DisplayNumberDeparted( void )
{
	// display number departed from team
	CHAR16 sString[ 32 ];
	INT16 sX = 0, sY = 0;


	// font stuff
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );

	if( fCurrentTeamMode == FALSE )
	{
		swprintf( sString, L"%s ( %d )", pPersonelTeamStrings[ 1 ], GetNumberOfPastMercsOnPlayersTeam( ) );
		sX = PERS_CURR_TEAM_X;
	}
	else
	{
		swprintf( sString, L"%s", pPersonelTeamStrings[ 1 ] );
		FindFontCenterCoordinates( PERS_CURR_TEAM_X, 0, 65, 0,sString, FONT10ARIAL, &sX, &sY );
	}

	mprintf( sX, PERS_DEPART_TEAM_Y, sString );


	return;
}


INT32 GetTotalDailyCostOfCurrentTeam( void )
{
	// will return the total daily cost of the current team

	SOLDIERTYPE *pSoldier;
	INT32 cnt=0;
	INT32 iCounter = 0;
	INT32 iCostOfTeam = 0;


	 // first grunt
	pSoldier = MercPtrs[0];

	// not active?..return cost of zero

	// run through active soldiers
	for ( pSoldier = MercPtrs[ 0 ]; cnt <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; cnt++ )
	{
		pSoldier = MercPtrs[cnt];

		 if( ( pSoldier->bActive) && ( pSoldier->bLife > 0 ) )
		 {

			 // valid soldier, get cost
			 if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			 {
				 // daily rate
				 if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK )
				 {
					 // 2 week contract
				   iCostOfTeam += gMercProfiles[pSoldier->ubProfile].uiBiWeeklySalary / 14;
				 }
				 else if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK )
				 {
					 // 1 week contract
				   iCostOfTeam += gMercProfiles[ pSoldier->ubProfile ].uiWeeklySalary / 7 ;
				 }
				 else
				 {

					 iCostOfTeam += gMercProfiles[ pSoldier->ubProfile ].sSalary;
				 }
			 }
			 else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			 {
				 // MERC Merc
				 iCostOfTeam += gMercProfiles[ pSoldier->ubProfile ].sSalary;
			 }
			 else
			 {
					// no cost
				 iCostOfTeam += 0;
			 }
		}
	}
	return iCostOfTeam;
}

INT32 GetLowestDailyCostOfCurrentTeam( void )
{
	// will return the lowest daily cost of the current team

	SOLDIERTYPE *pSoldier;
	INT32 cnt=0;
	INT32 iCounter = 0;
	INT32 iLowest = 999999;
//	INT32 iId =0;
	INT32 iCost = 0;

	 // first grunt
	pSoldier = MercPtrs[0];

	// not active?..return cost of zero

	// run through active soldiers
	for ( pSoldier = MercPtrs[ 0 ]; cnt <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; cnt++ )
	{
		pSoldier = MercPtrs[cnt];

		if( ( pSoldier->bActive ) && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && ( pSoldier->bLife > 0 ) )
		{
			 // valid soldier, get cost
			 if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			 {
				 // daily rate
				 if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK )
				 {
					 // 2 week contract
				   iCost = gMercProfiles[ pSoldier->ubProfile ].uiBiWeeklySalary / 14;
				 }
				 else if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK )
				 {
					 // 1 week contract
				   iCost = gMercProfiles[ pSoldier->ubProfile ].uiWeeklySalary / 7 ;
				 }
				 else
				 {

					 iCost = gMercProfiles[ pSoldier->ubProfile ].sSalary;
				 }
			 }
			 else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			 {
				 // MERC Merc
				 iCost = gMercProfiles[ pSoldier->ubProfile ].sSalary;
			 }
			 else
			 {
					// no cost
				 iCost = 0;
			 }

			 if( iCost <= iLowest )
			 {
				 iLowest = iCost;
			 }
		 }

		}

	// if no mercs, send 0
	if( iLowest == 999999 )
	{
		iLowest = 0;
	}


	return iLowest;
}


INT32 GetHighestDailyCostOfCurrentTeam( void )
{
	// will return the lowest daily cost of the current team

	SOLDIERTYPE *pSoldier;
	INT32 cnt=0;
	INT32 iCounter = 0;
	INT32 iHighest = 0;
//	INT32 iId =0;
	INT32 iCost = 0;

	 // first grunt
	pSoldier = MercPtrs[0];

	// not active?..return cost of zero

	// run through active soldiers
	for ( pSoldier = MercPtrs[0]; cnt <= gTacticalStatus.Team[ OUR_TEAM ].bLastID; cnt++)
	{
		pSoldier = MercPtrs[cnt];

		 if( ( pSoldier->bActive) && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && ( pSoldier->bLife > 0 ) )
		 {

			 // valid soldier, get cost
			 if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			 {
				 // daily rate
				 if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK )
				 {
					 // 2 week contract
				   iCost = gMercProfiles[ pSoldier->ubProfile ].uiBiWeeklySalary / 14;
				 }
				 else if( pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK  )
				 {
					 // 1 week contract
				   iCost = gMercProfiles[ pSoldier->ubProfile ].uiWeeklySalary / 7 ;
				 }
				 else
				 {

					 iCost = gMercProfiles[ pSoldier->ubProfile ].sSalary;
				 }
			 }
			 else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			 {
				 // MERC Merc
				 iCost = gMercProfiles[ pSoldier->ubProfile ].sSalary;
			 }
			 else
			 {
					// no cost
				 iCost = 0;
			 }


			 if( iCost >= iHighest )
			 {
				 iHighest = iCost;
			 }
		 }

		}
	return iHighest;
}



void DisplayCostOfCurrentTeam( void )
{
	// display number on team
	CHAR16 sString[ 32 ];
	INT16 sX, sY;


	// font stuff
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );

	if( fCurrentTeamMode == TRUE )
	{

		// daily cost
		mprintf(  PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_COST_Y, pPersonelTeamStrings[ 2 ] );

	  swprintf( sString, L"%d", GetTotalDailyCostOfCurrentTeam( ) );
		InsertCommasForDollarFigure( sString );
		InsertDollarSignInToString( sString );

		FindFontRightCoordinates((INT16)(PERS_CURR_TEAM_COST_X),0,PERS_CURR_TEAM_WIDTH,0,sString, PERS_FONT,  &sX, &sY);

		mprintf( sX ,PERS_CURR_TEAM_COST_Y, sString );

		// highest cost
		mprintf(  PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_HIGHEST_Y, pPersonelTeamStrings[ 3 ] );

	  swprintf( sString, L"%d", GetHighestDailyCostOfCurrentTeam( ) );
		InsertCommasForDollarFigure( sString );
		InsertDollarSignInToString( sString );

		FindFontRightCoordinates((INT16)(PERS_CURR_TEAM_COST_X),0,PERS_CURR_TEAM_WIDTH,0,sString, PERS_FONT,  &sX, &sY);

		mprintf( sX ,PERS_CURR_TEAM_HIGHEST_Y, sString );

		// the lowest cost
		mprintf(  PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_LOWEST_Y, pPersonelTeamStrings[ 4 ] );

	  swprintf( sString, L"%d", GetLowestDailyCostOfCurrentTeam( ) );
		InsertCommasForDollarFigure( sString );
		InsertDollarSignInToString( sString );

		FindFontRightCoordinates((INT16)(PERS_CURR_TEAM_COST_X),0,PERS_CURR_TEAM_WIDTH,0,sString, PERS_FONT,  &sX, &sY);

		mprintf( sX ,PERS_CURR_TEAM_LOWEST_Y, sString );


	}
	else
	{
		// do nothing
		return;
	}
}

INT32 GetIdOfDepartedMercWithHighestStat( INT32 iStat )
{
	// will return the id value of the merc on the players team with highest in this stat
  // -1 means error
	INT32 iId = -1;
	INT32 iValue =0;
	MERCPROFILESTRUCT *pTeamSoldier;
	INT32 cnt=0;
	INT32 iCounter =0;
	INT8 bCurrentList = 0;
	INT16 *bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
	BOOLEAN fNotDone = TRUE;
	SOLDIERTYPE *pSoldier;
	UINT32 uiLoopCounter;


	// run through active soldiers
//	while( fNotDone )
	for( uiLoopCounter=0; fNotDone ; uiLoopCounter++ )
	{
/*
		// check if we are in fact not done
		if( ( bCurrentList == 2 ) && ( *bCurrentListValue == -1 ) )
		{
			fNotDone = FALSE;
			continue;
		}
*/
		//if we are at the end of
		if( uiLoopCounter == 255 && bCurrentList == 2 )
		{
			fNotDone = FALSE;
			continue;
		}

		// check if we need to move to the next list
//		if( *bCurrentListValue == -1 )
		if( uiLoopCounter == 255 )
		{
			if( bCurrentList == 0 )
			{
				bCurrentList = 1;
			  bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
			}
			else if( bCurrentList == 1 )
			{
				bCurrentList = 2;
				bCurrentListValue = LaptopSaveInfo.ubOtherCharactersList;
			}

			//reset the loop counter
			uiLoopCounter = 0;
		}

		// get the id of the grunt
		cnt = *bCurrentListValue;

		// do we need to reset the count?
		if( cnt == -1 )
		{
			bCurrentListValue++;
			continue;
		}

		pTeamSoldier = &( gMercProfiles[ cnt ] );

		switch( iStat )
		{
			case 0:
			// health

				//if the soldier is a pow, dont use the health cause it aint known
			pSoldier = FindSoldierByProfileID( (UINT8)cnt, FALSE );
			if( pSoldier && pSoldier->bAssignment == ASSIGNMENT_POW )
			{
				continue;
			}

			if( pTeamSoldier ->  bLife >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bLife;
			}
			break;
		  case 1:
		  // agility
			if( pTeamSoldier ->  bAgility >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bAgility;
			}
			break;
			case 2:
			// dexterity
			if( pTeamSoldier ->  bDexterity >= iValue )
			{
					iId = cnt;
					iValue = pTeamSoldier -> bDexterity;
			}
			break;
			case 3:
			// strength
      if(  pTeamSoldier -> bStrength >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bStrength;
			}
			break;
			case 4:
			// leadership
      if(  pTeamSoldier -> bLeadership >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bLeadership;
			}
			break;
			case 5:
		  // wisdom
			if(  pTeamSoldier -> bWisdom >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bWisdom;
			}
			break;
			case 6:
			// exper
      if( pTeamSoldier -> bExpLevel >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bExpLevel;
			}

			break;
			case 7:
			//mrkmanship
			if(  pTeamSoldier -> bMarksmanship >= iValue )
			{
					iId = cnt;
					iValue = pTeamSoldier -> bMarksmanship;
			}

		  break;
			case 8:
			// mech
			if(  pTeamSoldier -> bMechanical >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bMechanical;
			}
		  break;
			case 9:
			// exp
			if(pTeamSoldier -> bExplosive >= iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bExplosive;
			}
			break;
			case 10:
			// med
			if(  pTeamSoldier -> bMedical >= iValue )
			{
					iId = cnt;
					iValue = pTeamSoldier -> bMedical;
			}
			break;
		}

		bCurrentListValue++;
	}


	return( iId );
}


INT32 GetIdOfDepartedMercWithLowestStat( INT32 iStat )
{
	// will return the id value of the merc on the players team with highest in this stat
  // -1 means error
	INT32 iId = -1;
	INT32 iValue =9999999;
	MERCPROFILESTRUCT *pTeamSoldier;
	INT32 cnt=0;
	INT32 iCounter =0;
	INT8 bCurrentList = 0;
	INT16 *bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
	BOOLEAN fNotDone = TRUE;
	SOLDIERTYPE		*pSoldier;
	UINT32 uiLoopCounter;

	// run through active soldiers
//	while( fNotDone )
	for( uiLoopCounter=0; fNotDone ; uiLoopCounter++ )
	{
/*
		// check if we are in fact not done
		if( ( bCurrentList == 2 ) && ( *bCurrentListValue == -1 ) )
		{
			fNotDone = FALSE;
			continue;
		}
*/
		//if we are at the end of
		if( uiLoopCounter == 255 && bCurrentList == 2 )
		{
			fNotDone = FALSE;
			continue;
		}

		// check if we need to move to the next list
//		if( *bCurrentListValue == -1 )
		if( uiLoopCounter == 255 )
		{
			if( bCurrentList == 0 )
			{
				bCurrentList = 1;
			  bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
			}
			else if( bCurrentList == 1 )
			{
				bCurrentList = 2;
				bCurrentListValue = LaptopSaveInfo.ubOtherCharactersList;
			}

			//reset the loop counter
			uiLoopCounter = 0;
		}

		// get the id of the grunt
		cnt = *bCurrentListValue;

		// do we need to reset the count?
		if( cnt == -1 )
		{
			bCurrentListValue++;
			continue;
		}

		pTeamSoldier = &( gMercProfiles[ cnt ] );


		switch( iStat )
		{
			case 0:
			// health

			pSoldier = FindSoldierByProfileID( (UINT8)cnt, FALSE );
			if( pSoldier && pSoldier->bAssignment == ASSIGNMENT_POW )
			{
				continue;
			}

			if( pTeamSoldier ->  bLife < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bLife;
			}
			break;
		  case 1:
		  // agility
			if( pTeamSoldier ->  bAgility < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bAgility;
			}
			break;
			case 2:
			// dexterity
			if( pTeamSoldier ->  bDexterity < iValue )
			{
					iId = cnt;
					iValue = pTeamSoldier -> bDexterity;
			}
			break;
			case 3:
			// strength
      if(  pTeamSoldier -> bStrength < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bStrength;
			}
			break;
			case 4:
			// leadership
      if(  pTeamSoldier -> bLeadership < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bLeadership;
			}
			break;
			case 5:
		  // wisdom
			if(  pTeamSoldier -> bWisdom < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bWisdom;
			}
			break;
			case 6:
			// exper
      if( pTeamSoldier -> bExpLevel < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bExpLevel;
			}

			break;
			case 7:
			//mrkmanship
			if(  pTeamSoldier -> bMarksmanship < iValue )
			{
					iId = cnt;
					iValue = pTeamSoldier -> bMarksmanship;
			}

		  break;
			case 8:
			// mech
			if(  pTeamSoldier -> bMechanical < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bMechanical;
			}
		  break;
			case 9:
			// exp
			if(pTeamSoldier -> bExplosive < iValue )
			{
				iId = cnt;
				iValue = pTeamSoldier -> bExplosive;
			}
			break;
			case 10:
			// med
			if(  pTeamSoldier -> bMedical < iValue )
			{
					iId = cnt;
					iValue = pTeamSoldier -> bMedical;
			}
			break;
		}

		bCurrentListValue++;
	}


	return( iId );
}


INT32 GetIdOfMercWithHighestStat( INT32 iStat )
{
	// will return the id value of the merc on the players team with highest in this stat
  // -1 means error
	INT32 iId = -1;
	INT32 iValue =0;
	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt=0;

	 // first grunt
	pSoldier = MercPtrs[0];


	// run through active soldiers
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
	{
		 if( ( pTeamSoldier->bActive) && !( pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && ( pTeamSoldier->bLife > 0 ) && !AM_A_ROBOT( pTeamSoldier ) )
		 {
			 switch( iStat )
			 {
				 case 0:
			   // health
						if( pTeamSoldier->bAssignment == ASSIGNMENT_POW )
						{
							continue;
						}

					 if( pTeamSoldier ->  bLifeMax >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bLifeMax;
					 }
				   break;
				 case 1:
					 // agility
					 if( pTeamSoldier ->  bAgility >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bAgility;
					 }
					 break;
				 case 2:
					// dexterity
					 if( pTeamSoldier ->  bDexterity >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bDexterity;
					 }
					break;
				 case 3:
				 // strength
       		 if(  pTeamSoldier -> bStrength >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bStrength;
					 }
				 break;
				 case 4:
					// leadership
       		 if(  pTeamSoldier -> bLeadership >= iValue )
					 {
						 iId = cnt;
						  iValue = pTeamSoldier -> bLeadership;
					 }
				 break;
				 case 5:
					// wisdom
					 if(  pTeamSoldier -> bWisdom >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bWisdom;
					 }
				 break;
				 case 6:
					// exper
           if( pTeamSoldier -> bExpLevel >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bExpLevel;
					 }

				 break;
				 case 7:
					 //mrkmanship
					 if(  pTeamSoldier -> bMarksmanship >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bMarksmanship;
					 }

				 break;
				 case 8:
					// mech
					 if(  pTeamSoldier -> bMechanical >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bMechanical;
					 }
				 break;
				 case 9:
					// exp
					 if(pTeamSoldier -> bExplosive >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bExplosive;
					 }
				 break;
				 case 10:
					 // med
					 if(  pTeamSoldier -> bMedical >= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bMedical;
					 }
				 break;
				 }
		 }
	}


	return( iId );
}

INT32 GetIdOfMercWithLowestStat( INT32 iStat )
{
	// will return the id value of the merc on the players team with highest in this stat
  // -1 means error
	INT32 iId = -1;
	INT32 iValue =999999;
	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt=0;

	 // first grunt
	pSoldier = MercPtrs[0];


	// run through active soldiers
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
	{
		 if(( pTeamSoldier->bActive) && !( pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE ) && ( pTeamSoldier->bLife > 0 ) && !AM_A_ROBOT( pTeamSoldier ) )
		 {

			 switch( iStat )
			 {
				 case 0:
			   // health

					if( pTeamSoldier->bAssignment == ASSIGNMENT_POW )
					{
						continue;
					}

					 if( pTeamSoldier -> bLifeMax <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bLifeMax;
					 }
				   break;
				 case 1:
					 // agility
					 if( pTeamSoldier -> bAgility <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bAgility;
					 }
					 break;
				 case 2:
					// dexterity
					 if(  pTeamSoldier -> bDexterity <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bDexterity;
					 }
					break;
				 case 3:
				 // strength
       		 if(  pTeamSoldier -> bStrength <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bStrength;
					 }
				 break;
				 case 4:
					// leadership
       		 if( pTeamSoldier -> bLeadership <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bLeadership;
					 }
				 break;
				 case 5:
					// wisdom
					 if( pTeamSoldier -> bWisdom <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bWisdom;
					 }
				 break;
				 case 6:
					// exper
           if(  pTeamSoldier -> bExpLevel <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bExpLevel;
					 }

				 break;
				 case 7:
					 //mrkmanship
					 if(  pTeamSoldier -> bMarksmanship <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bMarksmanship;
					 }

				 break;
				 case 8:
					// mech
					 if(  pTeamSoldier -> bMechanical <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bMechanical;
					 }
				 break;
				 case 9:
					// exp
					 if(  pTeamSoldier -> bExplosive <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bExplosive;
					 }
				 break;
				 case 10:
					 // med
					 if(  pTeamSoldier -> bMedical <= iValue )
					 {
						 iId = cnt;
						 iValue = pTeamSoldier -> bMedical;
					 }
				 break;
				 }
		 }
	}


	return( iId );
}


INT32 GetAvgStatOfCurrentTeamStat( INT32 iStat )
{
	// will return the id value of the merc on the players team with highest in this stat
  // -1 means error
	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt=0;
	INT32 iTotalStatValue = 0;
	INT8	bNumberOfPows = 0;
	UINT8	ubNumberOfMercsInCalculation = 0;


	 // first grunt
	pSoldier = MercPtrs[0];

	// run through active soldiers
	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++,pTeamSoldier++)
	{
		 if(( pTeamSoldier->bActive)&&( pTeamSoldier->bLife > 0 ) && !AM_A_ROBOT( pTeamSoldier ) )
		 {
			 switch( iStat )
			 {
				 case 0:
			   // health

						//if this is a pow, dont count his stats
						if( pTeamSoldier->bAssignment == ASSIGNMENT_POW )
						{
							bNumberOfPows++;
							continue;
						}

					  iTotalStatValue += pTeamSoldier -> bLifeMax;

				   break;
				 case 1:
					 // agility
					 iTotalStatValue +=pTeamSoldier -> bAgility;

					 break;
				 case 2:
					// dexterity
					 iTotalStatValue +=  pTeamSoldier -> bDexterity;

					break;
				 case 3:
				 // strength
				 iTotalStatValue +=  pTeamSoldier -> bStrength;

				 break;
				 case 4:
					// leadership
					 iTotalStatValue +=  pTeamSoldier -> bLeadership;

				 break;
				 case 5:
					// wisdom

					 iTotalStatValue += pTeamSoldier -> bWisdom;
				 break;
				 case 6:
					// exper

					 iTotalStatValue +=  pTeamSoldier -> bExpLevel;

				 break;
				 case 7:
					 //mrkmanship

					 iTotalStatValue +=  pTeamSoldier -> bMarksmanship;

				 break;
				 case 8:
					// mech

					 iTotalStatValue +=  pTeamSoldier -> bMechanical;
				 break;
				 case 9:
					// exp

					 iTotalStatValue +=  pTeamSoldier ->bExplosive;
				 break;
				 case 10:
					 // med

					 iTotalStatValue +=  pTeamSoldier -> bMedical;
				 break;
			 }

			 ubNumberOfMercsInCalculation++;
		 }
	}


	//if the stat is health, and there are only pow's
	if( GetNumberOfMercsOnPlayersTeam( ) != 0 && GetNumberOfMercsOnPlayersTeam( ) == bNumberOfPows && iStat == 0 )
	{
		return( - 1 );
	}
	else if( ( ubNumberOfMercsInCalculation - bNumberOfPows ) > 0 )
	{
		return( iTotalStatValue / ( ubNumberOfMercsInCalculation - bNumberOfPows ) );
	}
	else
	{
		return( 0 );
	}
}


INT32 GetAvgStatOfPastTeamStat( INT32 iStat )
{
	// will return the id value of the merc on the players team with highest in this stat
  // -1 means error
	INT32 cnt=0;
	INT32 iTotalStatValue = 0;
	INT32 iId = -1;
	MERCPROFILESTRUCT *pTeamSoldier;
	INT32 iCounter =0;
	INT8 bCurrentList = 0;
	INT16 *bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
	BOOLEAN fNotDone = TRUE;
	UINT32 uiLoopCounter;

	// run through active soldiers

	//while( fNotDone )
	for( uiLoopCounter=0; fNotDone ; uiLoopCounter++ )
	{
/*
		// check if we are in fact not done
		if( ( bCurrentList == 2 ) && ( *bCurrentListValue == -1 ) )
		{
			fNotDone = FALSE;
			continue;
		}
*/

		//if we are at the end of
		if( uiLoopCounter == 255 && bCurrentList == 2 )
		{
			fNotDone = FALSE;
			continue;
		}

		// check if we need to move to the next list
//		if( *bCurrentListValue == -1 )
		if( uiLoopCounter == 255 )
		{
			if( bCurrentList == 0 )
			{
				bCurrentList = 1;
			  bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
			}
			else if( bCurrentList == 1 )
			{
				bCurrentList = 2;
				bCurrentListValue = LaptopSaveInfo.ubOtherCharactersList;
			}

			//reset the loop counter
			uiLoopCounter = 0;
		}

		// get the id of the grunt
		cnt = *bCurrentListValue;

		// do we need to reset the count?
		if( cnt == -1 )
		{
			bCurrentListValue++;
			continue;
		}

		pTeamSoldier = &( gMercProfiles[ cnt ] );

		switch( iStat )
		{
			case 0:
			// health

				iTotalStatValue += pTeamSoldier -> bLife;

			break;
		  case 1:
		  // agility

				iTotalStatValue += pTeamSoldier -> bAgility;

			break;
			case 2:
			// dexterity

					iTotalStatValue += pTeamSoldier -> bDexterity;

			break;
			case 3:
			// strength

				iTotalStatValue += pTeamSoldier -> bStrength;

			break;
			case 4:
			// leadership

				iTotalStatValue += pTeamSoldier -> bLeadership;

			break;
			case 5:
		  // wisdom

				iTotalStatValue += pTeamSoldier -> bWisdom;

			break;
			case 6:
			// exper

				iTotalStatValue += pTeamSoldier -> bExpLevel;


			break;
			case 7:
			//mrkmanship

					iId = cnt;
					iTotalStatValue += pTeamSoldier -> bMarksmanship;


		  break;
			case 8:
			// mech

				iTotalStatValue += pTeamSoldier -> bMechanical;

		  break;
			case 9:
			// exp

				iTotalStatValue += pTeamSoldier -> bExplosive;

			break;
			case 10:
			// med

				iTotalStatValue += pTeamSoldier -> bMedical;
			break;
		}

		bCurrentListValue++;
	}

	if( GetNumberOfPastMercsOnPlayersTeam( ) > 0 )
	{
	  return( iTotalStatValue / GetNumberOfPastMercsOnPlayersTeam( ) );
	}
	else
	{
		return( 0 );
	}
}

void DisplayAverageStatValuesForCurrentTeam( void )
{
	// will display the average values for stats for the current team
	INT16 sX, sY;
	INT32 iCounter = 0;
	CHAR16 sString[ 32 ];

	// set up font
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );

	// display header

	// center
	FindFontCenterCoordinates( PERS_STAT_AVG_X, 0 ,PERS_STAT_AVG_WIDTH, 0 , pPersonnelCurrentTeamStatsStrings[ 1 ], FONT10ARIAL , &sX, &sY );

	mprintf( sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[ 1 ] );

	// nobody on team leave
  if( ( GetNumberOfMercsDeadOrAliveOnPlayersTeam( ) == 0 ) && (  fCurrentTeamMode == TRUE ) )
	{
		return;
	}

	// check if in past team and nobody on past team
	if( ( GetNumberOfPastMercsOnPlayersTeam( ) == 0 ) && (  fCurrentTeamMode == FALSE ) )
	{
		return;
	}


	for( iCounter = 0; iCounter < 11; iCounter++ )
	{

		// even or odd?..color black or yellow?
		if( iCounter % 2 == 0 )
		{
			SetFontForeground( PERS_TEXT_FONT_ALTERNATE_COLOR );
		}
		else
		{
			SetFontForeground( PERS_TEXT_FONT_COLOR );
		}

		if(  fCurrentTeamMode == TRUE )
		{
			INT32	iValue = GetAvgStatOfCurrentTeamStat( iCounter );

			//if there are no values
			if( iValue == -1 )
				swprintf( sString, L"%s", pPOWStrings[ 1 ] );
			else
				swprintf( sString, L"%d", iValue );

		}
		else
		{
			swprintf( sString, L"%d", GetAvgStatOfPastTeamStat( iCounter ) );
		}
		// center
	  FindFontCenterCoordinates( PERS_STAT_AVG_X, 0 ,PERS_STAT_AVG_WIDTH, 0 , sString, FONT10ARIAL , &sX, &sY );

	  mprintf( sX, PERS_STAT_AVG_Y + ( iCounter + 1 ) * ( GetFontHeight( FONT10ARIAL ) + 3 ), sString );
	}

	return;
}

void DisplayLowestStatValuesForCurrentTeam( void )
{
	// will display the average values for stats for the current team
	INT16 sX, sY;
	INT32 iCounter = 0;
	CHAR16 sString[ 32 ];
	INT32 iStat = 0;
	INT32	iDepartedId=0;
	INT32	iId = 0;

	// set up font
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );

	// display header

	// center
	FindFontCenterCoordinates( PERS_STAT_LOWEST_X, 0 ,PERS_STAT_LOWEST_WIDTH, 0 , pPersonnelCurrentTeamStatsStrings[ 0 ], FONT10ARIAL , &sX, &sY );

	mprintf( sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[ 0 ] );

	// nobody on team leave
  if( ( GetNumberOfMercsOnPlayersTeam( ) == 0 ) && ( fCurrentTeamMode == TRUE) )
	{
		return;
	}

	if( ( GetNumberOfPastMercsOnPlayersTeam( ) == 0 ) && ( fCurrentTeamMode == FALSE ) )
	{
		return;
	}

	for( iCounter = 0; iCounter < 11; iCounter++ )
	{
		if( fCurrentTeamMode == TRUE )
		{
			iId = GetIdOfMercWithLowestStat( iCounter );
//			if( iId == -1 )
//				continue;
		}
		else
		{
			iDepartedId = GetIdOfDepartedMercWithLowestStat( iCounter );
			if( iDepartedId == -1 )
				continue;
		}

			// even or odd?..color black or yellow?
		if( iCounter % 2 == 0 )
		{
			SetFontForeground( PERS_TEXT_FONT_ALTERNATE_COLOR );
		}
		else
		{
			SetFontForeground( PERS_TEXT_FONT_COLOR );
		}

		if( fCurrentTeamMode == TRUE )
		{
			// get name
			if( iId == -1 )
				swprintf( sString, L"%s", pPOWStrings[1] );
			else
				swprintf( sString, L"%s", MercPtrs[ iId ] -> name );
		}
		else
		{
			// get name
			swprintf( sString, L"%s", gMercProfiles[ iDepartedId ].zNickname );
		}
		// print name
		mprintf( PERS_STAT_LOWEST_X, PERS_STAT_AVG_Y + ( iCounter + 1 ) * ( GetFontHeight( FONT10ARIAL ) + 3 ), sString );

		switch( iCounter )
		{
				 case 0:
					 // health
					 if( fCurrentTeamMode == TRUE )
					 {
						 if( iId == -1 )
							 iStat = -1;
						 else
							iStat = MercPtrs[ iId ] -> bLifeMax;
					 }
					 else
					 {
						 iStat =  gMercProfiles[ iDepartedId ] . bLife;
					 }
					 break;
					 case 1:
						 // agility
						 if( fCurrentTeamMode == TRUE )
						 {
								iStat = MercPtrs[ iId ] -> bAgility;
						 }
						 else
						 {
							 	iStat =  gMercProfiles[ iDepartedId ] . bAgility;
						 }

						 break;
					 case 2:
						// dexterity
						if( fCurrentTeamMode == TRUE )
						{
							iStat = MercPtrs[ iId ] -> bDexterity;
						}
						else
						{
							iStat =  gMercProfiles[ iDepartedId ] . bDexterity;
						}

						break;
					 case 3:
					 // strength
					 if( fCurrentTeamMode == TRUE )
					 {
					   iStat = MercPtrs[ iId ] -> bStrength;
					 }
					 else
					 {
							iStat =  gMercProfiles[ iDepartedId ] . bStrength;
					 }

					 break;
					 case 4:
						// leadership
						if( fCurrentTeamMode == TRUE )
						{
							iStat = MercPtrs[ iId ] -> bLeadership;
						}
						else
						{
							iStat =  gMercProfiles[ iDepartedId ] . bLeadership;
						}
					 break;
					 case 5:
						// wisdom
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bWisdom;
						}
						else
						{
							iStat =  gMercProfiles[ iDepartedId ] . bWisdom;
						}
					 break;
					 case 6:
						// exper
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bExpLevel;
						}
						else
						{
							iStat =  gMercProfiles[ iDepartedId ] . bExpLevel;
						}
					 break;
					 case 7:
						 //mrkmanship
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bMarksmanship;
						}
						else
						{
							iStat =  gMercProfiles[ iDepartedId ] . bMarksmanship;
						}
					 break;
					 case 8:
						// mech
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bMechanical;
						}
						else
						{
							iStat =  gMercProfiles[ iDepartedId ] . bMechanical;
						}
					 break;
					 case 9:
						// exp
						if( fCurrentTeamMode == TRUE )
						{
							iStat = MercPtrs[ iId ] -> bExplosive;
						}
						else
						{
							iStat = gMercProfiles[ iDepartedId ] . bExplosive;
						}
					 break;
					 case 10:
						 // med
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bMedical;
						}
						else
						{
							iStat =  gMercProfiles[ iDepartedId ] . bMedical;
						}
					 break;
			}

			if( iStat == -1 )
				swprintf( sString, L"%s", pPOWStrings[1] );
			else
				swprintf( sString, L"%d", iStat );

		// right justify
	  FindFontRightCoordinates(  PERS_STAT_LOWEST_X, 0 ,PERS_STAT_LOWEST_WIDTH, 0 , sString, FONT10ARIAL , &sX, &sY );

	  mprintf( sX, PERS_STAT_AVG_Y + ( iCounter + 1 ) * ( GetFontHeight( FONT10ARIAL ) + 3 ), sString );
	}

	return;
}


void DisplayHighestStatValuesForCurrentTeam( void )
{
	// will display the average values for stats for the current team
	INT16 sX, sY;
	INT32 iCounter = 0;
	CHAR16 sString[ 32 ];
	INT32 iStat = 0;
	INT32 iId=0;

	// set up font
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );

	// display header

	// center
	FindFontCenterCoordinates( PERS_STAT_HIGHEST_X, 0 ,PERS_STAT_LOWEST_WIDTH, 0 , pPersonnelCurrentTeamStatsStrings[ 2 ], FONT10ARIAL , &sX, &sY );

	mprintf( sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[ 2 ] );

	// nobody on team leave
  if( ( GetNumberOfMercsOnPlayersTeam( ) == 0 ) && ( fCurrentTeamMode == TRUE) )
	{
		return;
	}

	if( ( GetNumberOfPastMercsOnPlayersTeam( ) == 0 ) && ( fCurrentTeamMode == FALSE ) )
	{
		return;
	}

	for( iCounter = 0; iCounter < 11; iCounter++ )
	{
		if( fCurrentTeamMode == TRUE )
			iId = GetIdOfMercWithHighestStat( iCounter );
		else
			iId = GetIdOfDepartedMercWithHighestStat( iCounter );

//		if( iId == -1 )
//			continue;

			// even or odd?..color black or yellow?
		if( iCounter % 2 == 0 )
		{
			SetFontForeground( PERS_TEXT_FONT_ALTERNATE_COLOR );
		}
		else
		{
			SetFontForeground( PERS_TEXT_FONT_COLOR );
		}


		if( fCurrentTeamMode == TRUE )
		{
			// get name
			if( iId == -1 )
				swprintf( sString, L"%s", pPOWStrings[1] );
			else
				swprintf( sString, L"%s", MercPtrs[ iId ] -> name );
		}
		else
		{
			// get name
			swprintf( sString, L"%s", gMercProfiles[ iId ].zNickname );
		}
		// print name
		mprintf( PERS_STAT_HIGHEST_X, PERS_STAT_AVG_Y + ( iCounter + 1 ) * ( GetFontHeight( FONT10ARIAL ) + 3 ), sString );

		switch( iCounter )
		{
				 case 0:
					 // health
					 if( fCurrentTeamMode == TRUE )
					 {
						 if( iId == -1 )
							iStat = -1;
						 else
							iStat = MercPtrs[ iId ] -> bLifeMax;
					 }
					 else
					 {
						 iStat =  gMercProfiles[ iId ] . bLife;
					 }
					 break;
					 case 1:
						 // agility
						 if( fCurrentTeamMode == TRUE )
						 {
								iStat = MercPtrs[ iId ] -> bAgility;
						 }
						 else
						 {
							 	iStat =  gMercProfiles[ iId ] . bAgility;
						 }

						 break;
					 case 2:
						// dexterity
						if( fCurrentTeamMode == TRUE )
						{
							iStat = MercPtrs[ iId ] -> bDexterity;
						}
						else
						{
							iStat =  gMercProfiles[ iId ] . bDexterity;
						}

						break;
					 case 3:
					 // strength
					 if( fCurrentTeamMode == TRUE )
					 {
					   iStat = MercPtrs[ iId ] -> bStrength;
					 }
					 else
					 {
							iStat =  gMercProfiles[ iId ] . bStrength;
					 }

					 break;
					 case 4:
						// leadership
						if( fCurrentTeamMode == TRUE )
						{
							iStat = MercPtrs[ iId ] -> bLeadership;
						}
						else
						{
							iStat =  gMercProfiles[ iId ] . bLeadership;
						}
					 break;
					 case 5:
						// wisdom
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bWisdom;
						}
						else
						{
							iStat =  gMercProfiles[ iId ] . bWisdom;
						}
					 break;
					 case 6:
						// exper
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bExpLevel;
						}
						else
						{
							iStat =  gMercProfiles[ iId ] . bExpLevel;
						}
					 break;
					 case 7:
						 //mrkmanship
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bMarksmanship;
						}
						else
						{
							iStat =  gMercProfiles[ iId ] . bMarksmanship;
						}
					 break;
					 case 8:
						// mech
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bMechanical;
						}
						else
						{
							iStat =  gMercProfiles[ iId ] . bMechanical;
						}
					 break;
					 case 9:
						// exp
						if( fCurrentTeamMode == TRUE )
						{
							iStat = MercPtrs[ iId ] -> bExplosive;
						}
						else
						{
							iStat = gMercProfiles[ iId ] . bExplosive;
						}
					 break;
					 case 10:
						 // med
						if( fCurrentTeamMode == TRUE )
						{
						  iStat = MercPtrs[ iId ] -> bMedical;
						}
						else
						{
							iStat =  gMercProfiles[ iId ] . bMedical;
						}
					 break;
			}

			if( iStat == -1 )
				swprintf( sString, L"%s", pPOWStrings[1] );
			else
				swprintf( sString, L"%d", iStat );

		// right justify
	  FindFontRightCoordinates(  PERS_STAT_HIGHEST_X, 0 ,PERS_STAT_LOWEST_WIDTH, 0 , sString, FONT10ARIAL , &sX, &sY );

	  mprintf( sX, PERS_STAT_AVG_Y + ( iCounter + 1 ) * ( GetFontHeight( FONT10ARIAL ) + 3 ), sString );
	}

	return;
}



void DisplayPersonnelTeamStats( void )
{
	// displays the stat title for each row in the team stat list
	INT32 iCounter =0;


	// set up font
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( FONT_WHITE );


	// display titles for each row
	for( iCounter = 0; iCounter < 11; iCounter++ )
	{
			// even or odd?..color black or yellow?
		if( iCounter % 2 == 0 )
		{
			SetFontForeground( PERS_TEXT_FONT_ALTERNATE_COLOR );
		}
		else
		{
			SetFontForeground( PERS_TEXT_FONT_COLOR );
		}

		mprintf( PERS_STAT_LIST_X, PERS_STAT_AVG_Y + ( iCounter + 1 ) * ( GetFontHeight( FONT10ARIAL ) + 3 ), pPersonnelTeamStatsStrings[ iCounter ] );
	}


	return;
}


INT32 GetNumberOfPastMercsOnPlayersTeam( void )
{
	INT32 iPastNumberOfMercs = 0;
	INT32 iCounter = 0;
	// will run through the alist of past mercs on the players team and return thier number

	// dead
	iPastNumberOfMercs += GetNumberOfDeadOnPastTeam( );

	// left
	iPastNumberOfMercs += GetNumberOfLeftOnPastTeam( );

	// other
	iPastNumberOfMercs += GetNumberOfOtherOnPastTeam( );


	return iPastNumberOfMercs;
}


void InitPastCharactersList( void )
{
	// inits the past characters list
	memset( &LaptopSaveInfo.ubDeadCharactersList, -1, sizeof( LaptopSaveInfo.ubDeadCharactersList ) );
	memset( &LaptopSaveInfo.ubLeftCharactersList, -1, sizeof( LaptopSaveInfo.ubLeftCharactersList ) );
	memset( &LaptopSaveInfo.ubOtherCharactersList, -1, sizeof( LaptopSaveInfo.ubOtherCharactersList ) );

	return;
}


INT32 GetNumberOfDeadOnPastTeam( void )
{

	INT32 iNumberDead = 0;
	INT32 iCounter = 0;

//	for( iCounter = 0; ( ( iCounter < 256) && ( LaptopSaveInfo.ubDeadCharactersList[ iCounter ] != -1 ) ) ; iCounter ++ )
	for( iCounter = 0; iCounter < 256 ; iCounter ++ )
	{
		if( LaptopSaveInfo.ubDeadCharactersList[ iCounter ] != -1 )
			iNumberDead++;
	}


	return( iNumberDead );

}


INT32 GetNumberOfLeftOnPastTeam( void )
{

	INT32 iNumberLeft = 0;
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < 256 ; iCounter ++ )
	{
		if( LaptopSaveInfo.ubLeftCharactersList[ iCounter ] != -1 )
			iNumberLeft++;
	}


	return( iNumberLeft );

}


INT32 GetNumberOfOtherOnPastTeam( void )
{

	INT32 iNumberOther = 0;
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < 256 ; iCounter ++ )
	{
		if( LaptopSaveInfo.ubOtherCharactersList[ iCounter ] != -1 )
			iNumberOther++;
	}


	return( iNumberOther );

}


void DisplayStateOfPastTeamMembers( void )
{
	INT16 sX, sY;
	CHAR16 sString[ 32 ];


	// font stuff
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );


	// diplsya numbers fired, dead and othered
	if( fCurrentTeamMode == FALSE )
	{

		// dead
	   mprintf(  PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_COST_Y, pPersonelTeamStrings[ 5 ] );
		 swprintf( sString, L"%d", GetNumberOfDeadOnPastTeam( ) );

		 FindFontRightCoordinates((INT16)(PERS_CURR_TEAM_COST_X),0,PERS_DEPART_TEAM_WIDTH,0,sString, PERS_FONT,  &sX, &sY);

		 mprintf( sX ,PERS_CURR_TEAM_COST_Y, sString );

		 // fired
		 mprintf(  PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_HIGHEST_Y, pPersonelTeamStrings[ 6 ] );
	   swprintf( sString, L"%d", GetNumberOfLeftOnPastTeam( ) );

		 FindFontRightCoordinates((INT16)(PERS_CURR_TEAM_COST_X),0,PERS_DEPART_TEAM_WIDTH,0,sString, PERS_FONT,  &sX, &sY);

		 mprintf( sX ,PERS_CURR_TEAM_HIGHEST_Y, sString );

		 // other
		 mprintf(  PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_LOWEST_Y, pPersonelTeamStrings[ 7 ] );
		 swprintf( sString, L"%d", GetNumberOfOtherOnPastTeam( ) );

		 FindFontRightCoordinates((INT16)(PERS_CURR_TEAM_COST_X),0,PERS_DEPART_TEAM_WIDTH,0,sString, PERS_FONT,  &sX, &sY);

		 mprintf( sX ,PERS_CURR_TEAM_LOWEST_Y, sString );
	}
	else
	{
		// do nothing
	}
	return;
}



void CreateDestroyCurrentDepartedMouseRegions( void )
{

	static BOOLEAN fCreated = FALSE;

	// will arbitrate the creation/deletion of mouse regions for current/past team toggles


	if( ( fCreateRegionsForPastCurrentToggle == TRUE ) && ( fCreated == FALSE ) )
	{
		// not created, create
		MSYS_DefineRegion(&gTogglePastCurrentTeam[ 0 ], PERS_TOGGLE_CUR_DEPART_X, PERS_TOGGLE_CUR_Y, PERS_TOGGLE_CUR_DEPART_X + PERS_TOGGLE_CUR_DEPART_WIDTH, PERS_TOGGLE_CUR_Y + PERS_TOGGLE_CUR_DEPART_HEIGHT,
		    MSYS_PRIORITY_HIGHEST - 3 ,CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelCurrentTeamCallback);

		MSYS_AddRegion( &gTogglePastCurrentTeam[ 0 ] );

		MSYS_DefineRegion(&gTogglePastCurrentTeam[ 1 ], PERS_TOGGLE_CUR_DEPART_X, PERS_TOGGLE_DEPART_Y , PERS_TOGGLE_CUR_DEPART_X + PERS_TOGGLE_CUR_DEPART_WIDTH, PERS_TOGGLE_DEPART_Y + PERS_TOGGLE_CUR_DEPART_HEIGHT,
		    MSYS_PRIORITY_HIGHEST - 3,CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelDepartedTeamCallback);

		MSYS_AddRegion( &gTogglePastCurrentTeam[ 1 ] );

		fCreated = TRUE;

	}
	else if( ( fCreateRegionsForPastCurrentToggle == FALSE ) && ( fCreated == TRUE ) )
	{
		// created, get rid of

		MSYS_RemoveRegion( &gTogglePastCurrentTeam[ 0 ] );
		MSYS_RemoveRegion( &gTogglePastCurrentTeam[ 1 ] );
		fCreated = FALSE;
	}

	return;
}



void PersonnelCurrentTeamCallback( MOUSE_REGION * pRegion, INT32 iReason )
{
	if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
		fCurrentTeamMode = TRUE;

		if( fCurrentTeamMode == TRUE )
		{
		  iCurrentPersonSelectedId = -1;

			// how many people do we have?..if you have someone set default to 0
			if( GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0 )
			{
				// get id of first merc in list

				iCurrentPersonSelectedId = GetIdOfFirstDisplayedMerc( );
			}
		}

		fCurrentTeamMode = TRUE;
		fReDrawScreenFlag = TRUE;
	}

}


void PersonnelDepartedTeamCallback( MOUSE_REGION * pRegion, INT32 iReason )
{
	if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {

		fCurrentTeamMode = FALSE;

		if( fCurrentTeamMode == FALSE )
		{
		  iCurrentPersonSelectedId = -1;

			// how many departed people?
			if( GetNumberOfPastMercsOnPlayersTeam( ) > 0 )
			{
				iCurrentPersonSelectedId = 0;
			}

			//Switch the panel on the right to be the stat panel
			gubPersonnelInfoState = PERSONNEL_STAT_BTN;
		}

		fReDrawScreenFlag = TRUE;
	}

}



void CreateDestroyButtonsForDepartedTeamList( void )
{
	// creates/ destroys the buttons for cdeparted team list
	static BOOLEAN fCreated = FALSE;

	if( ( fCurrentTeamMode == FALSE ) && ( fCreated == FALSE ) )
	{
		// not created. create
		giPersonnelButtonImage[ 4 ]=  LoadButtonImage( "LAPTOP\\departuresbuttons.sti" ,-1,0,-1,2,-1 );
	  giPersonnelButton[ 4 ] = QuickCreateButton( giPersonnelButtonImage[4], PERS_DEPARTED_UP_X, PERS_DEPARTED_UP_Y,
	                    BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)DepartedUpCallBack);

		// right button
		giPersonnelButtonImage[ 5 ]=  LoadButtonImage( "LAPTOP\\departuresbuttons.sti" ,-1,1,-1,3,-1 );
		giPersonnelButton[ 5 ] = QuickCreateButton( giPersonnelButtonImage[5], PERS_DEPARTED_UP_X, PERS_DEPARTED_DOWN_Y,
											BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)DepartedDownCallBack);

		// set up cursors for these buttons
		SetButtonCursor( giPersonnelButton[ 4 ], CURSOR_LAPTOP_SCREEN);
	  SetButtonCursor( giPersonnelButton[ 5 ], CURSOR_LAPTOP_SCREEN);


		fCreated = TRUE;
	}
	else if( ( fCurrentTeamMode == TRUE ) && ( fCreated == TRUE ) )
	{
		// created. destroy
		RemoveButton(giPersonnelButton[4] );
	  UnloadButtonImage(giPersonnelButtonImage[4] );
	  RemoveButton(giPersonnelButton[5] );
	  UnloadButtonImage(giPersonnelButtonImage[5] );
		fCreated = FALSE;
		fReDrawScreenFlag = TRUE;
	}
}


void DepartedUpCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{

		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
		  btn->uiFlags&=~(BUTTON_CLICKED_ON);

		  if(  giCurrentUpperLeftPortraitNumber - 20 >= 0 )
			{
			  giCurrentUpperLeftPortraitNumber-=20;
			  fReDrawScreenFlag = TRUE;
			}
		}
	}
}


void DepartedDownCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{

		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags&=~(BUTTON_CLICKED_ON);
			if( ( giCurrentUpperLeftPortraitNumber + 20 ) < ( GetNumberOfDeadOnPastTeam( ) + GetNumberOfLeftOnPastTeam( ) + GetNumberOfOtherOnPastTeam( ) ) )
			{
				giCurrentUpperLeftPortraitNumber+=20;
				fReDrawScreenFlag = TRUE;
			}
		}
	}
}

void DisplayPastMercsPortraits( void )
{
	// display past mercs portraits, starting at giCurrentUpperLeftPortraitNumber and going up 20 mercs
	// start at dead mercs, then fired, then other

	INT32 iCounter = 0;
	INT32 iCounterA = 0;
	INT32 iStartArray = 0; // 0 = dead list, 1 = fired list, 2 = other list
	BOOLEAN fFound = FALSE;

	// not time to display
	if( fCurrentTeamMode == TRUE )
	{
		return;
	}

	// go through dead list
//	for( iCounterA = 0; ( ( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ] != -1 ) && ( iCounter < giCurrentUpperLeftPortraitNumber ) ); iCounter++, iCounterA++ );
	for( iCounterA = 0; ( iCounter < giCurrentUpperLeftPortraitNumber ); iCounterA++ )
	{
		if( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ] != -1 )
			iCounter++;
	}

	if( iCounter < giCurrentUpperLeftPortraitNumber)
	{
			// now the fired list
//			for( iCounterA = 0; ( ( LaptopSaveInfo.ubLeftCharactersList[ iCounterA ] != -1 ) && ( iCounter < giCurrentUpperLeftPortraitNumber ) ); iCounter++, iCounterA++ );
			for( iCounterA = 0; ( ( iCounter < giCurrentUpperLeftPortraitNumber ) ); iCounterA++ )
			{
				if( LaptopSaveInfo.ubLeftCharactersList[ iCounterA ] != -1 )
				{
					iCounter++;
				}
			}

			if( iCounter < 20 )
			{
			  iStartArray = 0;
			}
			else
			{
				iStartArray = 1;
			}
	}
	else
	{
		iStartArray = 0;
	}

	if( ( iCounter < giCurrentUpperLeftPortraitNumber ) && ( iStartArray != 0 ) )
	{
			// now the fired list
//			for( iCounterA = 0; ( ( LaptopSaveInfo.ubOtherCharactersList[ iCounterA ] != -1 ) && ( iCounter < giCurrentUpperLeftPortraitNumber ) ); iCounter++, iCounterA++ );
			for( iCounterA = 0; ( iCounter < giCurrentUpperLeftPortraitNumber ); iCounterA++ )
			{
				if( LaptopSaveInfo.ubOtherCharactersList[ iCounterA ] != -1 )
					iCounter++;
			}

			if( iCounter < 20 )
			{
			  iStartArray = 1;
			}
			else
			{
				iStartArray = 2;
			}
	}
	else if( iStartArray != 0 )
	{
		iStartArray = 1;
	}

	//; we now have the array to start in, the position

	iCounter = 0;

	if( iStartArray == 0 )
	{
		// run through list and display
//	  for( iCounterA ; ( ( iCounter < 20 ) && ( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ] != -1 ) ); iCounter++, iCounterA++ )
	  for( iCounterA ; iCounter < 20 && iCounterA < 256; iCounterA++ )
		{
			// show dead pictures
			if( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ] != -1 )
			{
				DisplayPortraitOfPastMerc( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ], iCounter, TRUE, FALSE, FALSE );
				iCounter++;
			}
		}

		// reset counter A for the next array, if applicable
		iCounterA = 0;
	}
	if( iStartArray <= 1 )
	{
//		for( iCounterA ; ( ( iCounter < 20 ) && ( LaptopSaveInfo.ubLeftCharactersList[ iCounterA ] != -1 ) ); iCounter++, iCounterA++  )
		for( iCounterA ; ( iCounter < 20  && iCounterA < 256 ); iCounterA++  )
		{
			// show fired pics
			if( LaptopSaveInfo.ubLeftCharactersList[ iCounterA ] != -1 )
			{
				DisplayPortraitOfPastMerc( LaptopSaveInfo.ubLeftCharactersList[ iCounterA ], iCounter, FALSE, TRUE, FALSE );
				iCounter++;
			}
		}
		// reset counter A for the next array, if applicable
		iCounterA = 0;
	}

	if( iStartArray <= 2 )
	{
//		for( iCounterA ; ( ( iCounter < 20 ) && ( LaptopSaveInfo.ubOtherCharactersList[ iCounterA ] != -1 ) ) ; iCounter++, iCounterA++ )
		for( iCounterA ; ( iCounter < 20  && iCounterA < 256 ); iCounterA++ )
		{
			// show other pics
			if( LaptopSaveInfo.ubOtherCharactersList[ iCounterA ] != -1 )
			{
				DisplayPortraitOfPastMerc( LaptopSaveInfo.ubOtherCharactersList[ iCounterA ], iCounter, FALSE, FALSE, TRUE );
				iCounter++;
			}
		}
		// reset counter A for the next array, if applicable
		iCounterA = 0;
	}



	return;
}


INT32 GetIdOfPastMercInSlot( INT32 iSlot )
{

	INT32 iCounter =-1;
	INT32 iCounterA =0;
	// returns ID of Merc in this slot

	// not time to display
	if( fCurrentTeamMode == TRUE )
	{
		return -1;
	}

	if( iSlot > ( ( GetNumberOfDeadOnPastTeam( ) + GetNumberOfLeftOnPastTeam( ) + GetNumberOfOtherOnPastTeam( ) ) - giCurrentUpperLeftPortraitNumber ) )
	{
		// invalid slot
		return iCurrentPersonSelectedId;
	}
	// go through dead list
	for( iCounterA = 0; ( ( iCounter ) < iSlot + giCurrentUpperLeftPortraitNumber);  iCounterA++ )
	{
		if( LaptopSaveInfo.ubDeadCharactersList[ iCounterA ] != -1 )
			iCounter++;
	}

	if( iSlot + giCurrentUpperLeftPortraitNumber == iCounter)
	{
		return  ( LaptopSaveInfo.ubDeadCharactersList[ iCounterA - 1] );
	}

	// now the fired list
	iCounterA =0;
	for( iCounterA = 0; ( ( ( iCounter  )< iSlot + giCurrentUpperLeftPortraitNumber) ); iCounterA++ )
	{
		if( LaptopSaveInfo.ubLeftCharactersList[ iCounterA ] != -1 )
			iCounter++;
	}

	if( iSlot + giCurrentUpperLeftPortraitNumber == iCounter)
	{
		return  ( LaptopSaveInfo.ubLeftCharactersList[ iCounterA  - 1 ] );
	}


	// now the fired list
	iCounterA =0;
	for( iCounterA = 0; ( ( ( iCounter ) < ( iSlot + giCurrentUpperLeftPortraitNumber ) ) ); iCounterA++ )
	{
		if( LaptopSaveInfo.ubOtherCharactersList[ iCounterA ] != -1 )
			iCounter++;
	}

	return( LaptopSaveInfo.ubOtherCharactersList[ iCounterA  - 1] );

}





BOOLEAN DisplayPortraitOfPastMerc( INT32 iId , INT32 iCounter, BOOLEAN fDead, BOOLEAN fFired, BOOLEAN fOther )
{


	char sTemp[100];
	HVOBJECT hFaceHandle;
  VOBJECT_DESC    VObjectDesc;



	if( ( 50 < 	iId   )&&( 57 > 	iId   ) )
	{
		sprintf( sTemp, "%s%03d.sti", SMALL_FACES_DIR, 	 gMercProfiles[ iId ].ubFaceIndex );
	}
	else
	{
		if(  iId  < 100 )
		{
			 sprintf(sTemp, "%s%02d.sti", SMALL_FACES_DIR, iId );
		}
		else
		{
			sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR,	 iId  );
		}
	}

	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP(sTemp, VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiFACE));


	//Blt face to screen to
	GetVideoObject(&hFaceHandle, guiFACE);

	if( fDead )
	{
		hFaceHandle->pShades[ 0 ]		= Create16BPPPaletteShaded( hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE );

		//set the red pallete to the face
		SetObjectHandleShade( guiFACE, 0 );
	}

	BltVideoObject(FRAME_BUFFER, hFaceHandle, 0,( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT ), VO_BLT_SRCTRANSPARENCY,NULL);


/*
 text on the Small portrait
	if( fDead )
	{
		//if the merc is dead, display it
//		DrawTextToScreen(pDepartedMercPortraitStrings[0], ( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
		DrawTextToScreen( AimPopUpText[ AIM_MEMBER_DEAD ], ( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
	}
	else if( fFired )
	{
		DrawTextToScreen(pDepartedMercPortraitStrings[1], ( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
	}
	else if( fOther )
	{
		DrawTextToScreen(pDepartedMercPortraitStrings[2], ( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT + SMALL_PORT_HEIGHT / 2 ), SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
	}
*/

	DeleteVideoObjectFromIndex(guiFACE);

	return ( TRUE );
}




void DisplayDepartedCharStats(INT32 iId, INT32 iSlot, INT32 iState)
{
	INT32 iCounter=0;
	wchar_t sString[50];
	INT16 sX, sY;
	UINT32 uiHits = 0;

		// font stuff
	SetFont( FONT10ARIAL );
	SetFontBackground( FONT_BLACK );
	SetFontForeground( PERS_TEXT_FONT_COLOR );

	// display the stats for a char
	for(iCounter=0;iCounter <MAX_STATS; iCounter++)
	{
		switch(iCounter)
		{
		 case 0:
			 // health

			 // dead?
			 if( iState == 0 )
			 {
			   swprintf(sString, L"%d/%d",0,gMercProfiles[iId].bLife);
			 }
			 else
			 {
				 swprintf(sString, L"%d/%d",gMercProfiles[iId].bLife,gMercProfiles[iId].bLife);
			 }

			 mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
			 break;
		 case 1:
			 // agility
       swprintf(sString, L"%d",gMercProfiles[iId].bAgility);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
			 break;
		 case 2:
		  // dexterity
       swprintf(sString, L"%d",gMercProfiles[iId].bDexterity);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
			break;
		 case 3:
		 // strength
       swprintf(sString, L"%d",gMercProfiles[iId].bStrength);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 4:
		  // leadership
       swprintf(sString, L"%d",gMercProfiles[iId].bLeadership);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 5:
		  // wisdom
       swprintf(sString, L"%d",gMercProfiles[iId].bWisdom);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 6:
		  // exper
       swprintf(sString, L"%d",gMercProfiles[iId].bExpLevel);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 7:
			 //mrkmanship
       swprintf(sString, L"%d",gMercProfiles[iId].bMarksmanship);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
     case 8:
		  // mech
       swprintf(sString, L"%d",gMercProfiles[iId].bMechanical);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
     case 9:
		  // exp
       swprintf(sString, L"%d",gMercProfiles[iId].bExplosive);
       mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);
		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;
		 case 10:
       // med
			 mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[iCounter]);

       swprintf(sString, L"%d",gMercProfiles[iId].bMedical);


			 FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
			 mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;


		 case 14:
		 // kills
      mprintf((INT16)(pPersonnelScreenPoints[21].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[21].y,pPersonnelScreenStrings[PRSNL_TXT_KILLS]);
			swprintf(sString, L"%d",gMercProfiles[iId].usKills);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[21].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[21].y,sString);
		 break;
		 case 15:
			// assists
			mprintf((INT16)(pPersonnelScreenPoints[22].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[22].y,pPersonnelScreenStrings[PRSNL_TXT_ASSISTS]);
			swprintf(sString, L"%d",gMercProfiles[iId].usAssists);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[22].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[22].y,sString);
		 break;
		  case 16:
			// shots/hits
			mprintf((INT16)(pPersonnelScreenPoints[23].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[23].y,pPersonnelScreenStrings[PRSNL_TXT_HIT_PERCENTAGE]);
			uiHits = ( UINT32 )gMercProfiles[iId].usShotsHit;
			uiHits *= 100;

			// check we have shot at least once
			if( gMercProfiles[iId].usShotsFired > 0 )
			{
			  uiHits /= ( UINT32 )gMercProfiles[iId].usShotsFired;
			}
			else
			{
				// no, set hit % to 0
				uiHits = 0;
			}

			swprintf(sString, L"%d %%%%",uiHits);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[23].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      sX += StringPixLength( L"%",  PERS_FONT );
			mprintf(sX,pPersonnelScreenPoints[23].y,sString);
		 break;
		  case 17:
			// battles
			mprintf((INT16)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[24].y,pPersonnelScreenStrings[PRSNL_TXT_BATTLES]);
			swprintf(sString, L"%d",gMercProfiles[iId].usBattlesFought);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[24].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[24].y,sString);
		 break;
		 case 18:
			// wounds
			mprintf((INT16)(pPersonnelScreenPoints[25].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[25].y,pPersonnelScreenStrings[PRSNL_TXT_TIMES_WOUNDED]);
			swprintf(sString, L"%d",gMercProfiles[iId].usTimesWounded);
      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[25].x+(iSlot*TEXT_BOX_WIDTH)),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[25].y,sString);
		 break;
		}
	}

	return;
}


void EnableDisableDeparturesButtons( void )
{

	// will enable or disable departures buttons based on upperleft picutre index value
	if( ( fCurrentTeamMode == TRUE )||( fNewMailFlag == TRUE ) )
	{
		return;
	}

	// disable both buttons
	DisableButton( giPersonnelButton[ 4 ] );
	DisableButton( giPersonnelButton[ 5 ] );


	if( giCurrentUpperLeftPortraitNumber != 0 )
	{
		// enable up button
		EnableButton( giPersonnelButton[ 4 ] );
	}
	if( ( GetNumberOfDeadOnPastTeam( ) + GetNumberOfLeftOnPastTeam( ) + GetNumberOfOtherOnPastTeam( ) ) - giCurrentUpperLeftPortraitNumber  >= 20 )
	{
		// enable down button
		EnableButton( giPersonnelButton[ 5 ] );
	}


	return;
}


void DisplayDepartedCharName( INT32 iId, INT32 iSlot, INT32 iState )
{
  // get merc's nickName, assignment, and sector location info
	INT16 sX, sY;
	CHAR16 sString[ 32 ];

  SetFont(CHAR_NAME_FONT);
	SetFontForeground(PERS_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);

	if( ( iState == -1 )||( iId == -1 ) )
	{
		return;
	}

	swprintf( sString, L"%s", gMercProfiles[ iId ].zNickname );

		// nick name - assignment
	FindFontCenterCoordinates(IMAGE_BOX_X-5,0,IMAGE_BOX_WIDTH + 90 , 0,sString,CHAR_NAME_FONT, &sX, &sY );

	// cehck to se eif we are going to go off the left edge
	if( sX < pPersonnelScreenPoints[ 0 ].x )
	{
		sX = ( INT16 )pPersonnelScreenPoints[ 0 ].x;
	}

	mprintf( sX + iSlot * IMAGE_BOX_WIDTH, CHAR_NAME_Y, sString );


	// state
	if( gMercProfiles[ iId ].ubMiscFlags2 & PROFILE_MISC_FLAG2_MARRIED_TO_HICKS )
	{
		//displaye 'married'
		swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_MARRIED ] );
	}
	else if(  iState == DEPARTED_DEAD )
	{
		swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_DEAD ] );
	}

	//if the merc is an AIM merc
	else if( iId < BIFF )
	{
		//if dismissed
		if( iState == DEPARTED_FIRED )
			swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_FIRED ] );
		else
			swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_CONTRACT_EXPIRED ] );
	}

	//else if its a MERC merc
	else if( iId >= BIFF && iId <= BUBBA )
	{
		if( iState == DEPARTED_FIRED )
			swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_FIRED ] );
		else
			swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_QUIT ] );
	}
	//must be a RPC
	else
	{
		if( iState == DEPARTED_FIRED )
			swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_FIRED ] );
		else
			swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ DEPARTED_QUIT ] );
	}

//	swprintf( sString, L"%s", pPersonnelDepartedStateStrings[ iState ] );

		// nick name - assignment
	FindFontCenterCoordinates(IMAGE_BOX_X-5,0,IMAGE_BOX_WIDTH + 90 , 0,sString,CHAR_NAME_FONT, &sX, &sY );

	// cehck to se eif we are going to go off the left edge
	if( sX < pPersonnelScreenPoints[ 0 ].x )
	{
		sX = ( INT16 )pPersonnelScreenPoints[ 0 ].x;
	}

	mprintf( sX + iSlot * IMAGE_BOX_WIDTH, CHAR_NAME_Y + 10 , sString );



	return;
}


INT32 GetTheStateOfDepartedMerc( INT32 iId )
{

	INT32 iCounter =0;
	// will runt hrough each list until merc is found, if not a -1 is returned

	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		if ( LaptopSaveInfo.ubDeadCharactersList[ iCounter ] == iId )
		{
			return( DEPARTED_DEAD );
		}
	}

	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		if ( LaptopSaveInfo.ubLeftCharactersList[ iCounter ] == iId )
		{
			return( DEPARTED_FIRED );
		}
	}

	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		if ( LaptopSaveInfo.ubOtherCharactersList[ iCounter ] == iId )
		{
			return( DEPARTED_OTHER );
		}
	}

	return( -1 );
}


void DisplayPersonnelTextOnTitleBar( void )
{
	// draw email screen title text

	// font stuff
  SetFont( FONT14ARIAL );
  SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );

	// printf the title
	mprintf( PERS_TITLE_X, PERS_TITLE_Y, pPersTitleText[0] );

	// reset the shadow

}

BOOLEAN DisplayHighLightBox( void )
{
	// will display highlight box around selected merc
	 VOBJECT_DESC VObjectDesc;
	 UINT32 uiBox = 0;
	 HVOBJECT hHandle;

	// load graphics

	// is the current selected face valid?
	if( iCurrentPersonSelectedId == -1 )
	{
		// no, leave
		return ( FALSE );
	}

  // bounding
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\PicBorde.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &uiBox));

	// blit it
	GetVideoObject(&hHandle, uiBox);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,( INT16 ) ( SMALL_PORTRAIT_START_X+ ( iCurrentPersonSelectedId % PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_WIDTH - 2 ), ( INT16 ) ( SMALL_PORTRAIT_START_Y + ( iCurrentPersonSelectedId / PERSONNEL_PORTRAIT_NUMBER_WIDTH ) * SMALL_PORT_HEIGHT - 3 ), VO_BLT_SRCTRANSPARENCY,NULL);


	// deleteit
	DeleteVideoObjectFromIndex( uiBox );


	return ( TRUE );
}

// add to dead list
void AddCharacterToDeadList( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;


	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		if( LaptopSaveInfo.ubDeadCharactersList[ iCounter ] == -1 )
		{
			// valid slot, merc not found yet, inset here
			LaptopSaveInfo.ubDeadCharactersList[ iCounter ] = pSoldier->ubProfile;

			// leave
			return;
		}

		// are they already in the list?
		if( LaptopSaveInfo.ubDeadCharactersList[ iCounter ] == pSoldier->ubProfile )
		{

			return;
		}
	}
}


void AddCharacterToFiredList( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;


	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		if( LaptopSaveInfo.ubLeftCharactersList[ iCounter ] == -1 )
		{
			// valid slot, merc not found yet, inset here
			LaptopSaveInfo.ubLeftCharactersList[ iCounter ] = pSoldier->ubProfile;

			// leave
			return;
		}

		// are they already in the list?
		if( LaptopSaveInfo.ubLeftCharactersList[ iCounter ] == pSoldier->ubProfile )
		{

			return;
		}
	}
}


void AddCharacterToOtherList( SOLDIERTYPE *pSoldier )
{
	INT32 iCounter = 0;


	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		if( LaptopSaveInfo.ubOtherCharactersList[ iCounter ] == -1 )
		{
			// valid slot, merc not found yet, inset here
			LaptopSaveInfo.ubOtherCharactersList[ iCounter ] = pSoldier->ubProfile;

			// leave
			return;
		}

		// are they already in the list?
		if( LaptopSaveInfo.ubOtherCharactersList[ iCounter ] == pSoldier->ubProfile )
		{

			return;
		}
	}
}


// If you have hired a merc before, then the they left for whatever reason, and now you are hiring them again,
// we must get rid of them from the departed section in the personnel screen.  ( wouldnt make sense for them
//to be on your team list, and departed list )
BOOLEAN RemoveNewlyHiredMercFromPersonnelDepartedList( UINT8 ubProfile )
{
	INT32 iCounter = 0;

	for( iCounter = 0; iCounter < 256; iCounter++ )
	{
		// are they already in the Dead list?
		if( LaptopSaveInfo.ubDeadCharactersList[ iCounter ] == ubProfile )
		{
			//Reset the fact that they were once hired
			LaptopSaveInfo.ubDeadCharactersList[ iCounter ] = -1;
			return( TRUE );
		}

		// are they already in the other list?
		if( LaptopSaveInfo.ubLeftCharactersList[ iCounter ] == ubProfile )
		{
			//Reset the fact that they were once hired
			LaptopSaveInfo.ubLeftCharactersList[ iCounter ] = -1;
			return( TRUE );
		}

		// are they already in the list?
		if( LaptopSaveInfo.ubOtherCharactersList[ iCounter ] == ubProfile )
		{
			//Reset the fact that they were once hired
			LaptopSaveInfo.ubOtherCharactersList[ iCounter ] = -1;
			return( TRUE );
		}
	}

	return( FALSE );
}

// grab the id of the first merc being displayed
INT32 GetIdOfFirstDisplayedMerc( )
{
	SOLDIERTYPE *pSoldier;
	INT32 cnt = 0, iCounter = 0;

	// set current soldier
	pSoldier = MercPtrs[ cnt ];

  if( fCurrentTeamMode == TRUE )
	{
		// run through list of soldiers on players current team
		//cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;
    for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++, pSoldier++ )
		{
		   if( ( pSoldier->bActive )&&( pSoldier->bLife > 0 ) )
			 {
				 return( 0 );
			 }
		}
		return( -1 );
	}
	else
	{
		// run through list of soldier on players old team...the slot id will be translated
		return 0;
	}
}

INT32 GetIdOfThisSlot( INT32 iSlot )
{
		SOLDIERTYPE *pSoldier;
	INT32 cnt = 0;
	INT32 iCounter = 0;

	// set current soldier
	pSoldier = MercPtrs[ cnt ];

  if( fCurrentTeamMode == TRUE )
	{
		// run through list of soldiers on players current team
		cnt = gTacticalStatus.Team[ pSoldier->bTeam ].bFirstID;
    for ( pSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++, pSoldier++ )
		{
		   if( ( pSoldier->bActive )  )
			 {


				 // same character as slot, return this value
				 if( iCounter == iSlot )
				 {
						return( cnt );
				 }

				 // found another soldier
				 iCounter++;
			 }
		}
	}
	else
	{
		// run through list of soldier on players old team...the slot id will be translated
		return iSlot;
	}


	return( 0 );
}


BOOLEAN RenderAtmPanel( void )
{

	 VOBJECT_DESC VObjectDesc;
	 UINT32 uiBox = 0;
	 HVOBJECT hHandle;


	// render the ATM panel
	if( fShowAtmPanel )
	{
		VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
		FilenameForBPP("LAPTOP\\AtmButtons.sti", VObjectDesc.ImageFile);
		CHECKF(AddVideoObject(&VObjectDesc, &uiBox));

		// blit it
		GetVideoObject(&hHandle, uiBox);
		BltVideoObject(FRAME_BUFFER, hHandle, 0,( INT16 ) ( ATM_UL_X ), ( INT16 ) ( ATM_UL_Y ), VO_BLT_SRCTRANSPARENCY,NULL);

		DeleteVideoObjectFromIndex( uiBox );

		// show amount
		DisplayATMAmount( );
		RenderRectangleForPersonnelTransactionAmount( );

		// create destroy
		CreateDestroyStartATMButton( );
		CreateDestroyATMButton( );

		// display strings for ATM
		DisplayATMStrings( );

		// handle states
		HandleStateOfATMButtons( );

		//DisplayAmountOnCurrentMerc( );

	}
	else
	{
		// just show basic panel
		// bounding
		VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
		FilenameForBPP("LAPTOP\\AtmButtons.sti", VObjectDesc.ImageFile);
		CHECKF(AddVideoObject(&VObjectDesc, &uiBox));

		GetVideoObject(&hHandle, uiBox);
		BltVideoObject(FRAME_BUFFER, hHandle, 0,( INT16 ) ( ATM_UL_X ), ( INT16 ) ( ATM_UL_Y ), VO_BLT_SRCTRANSPARENCY,NULL);

		// blit it
		GetVideoObject(&hHandle, uiBox);
		BltVideoObject(FRAME_BUFFER, hHandle, 1,( INT16 ) ( ATM_UL_X + 1 ), ( INT16 ) ( ATM_UL_Y + 18), VO_BLT_SRCTRANSPARENCY,NULL);


		DeleteVideoObjectFromIndex( uiBox );

		// display strings for ATM
		DisplayATMStrings( );

		//DisplayAmountOnCurrentMerc( );

		// create destroy
		CreateDestroyStartATMButton( );
		CreateDestroyATMButton( );
	}
	return( TRUE );
}


void CreateDestroyStartATMButton( void )
{
	static BOOLEAN fCreated = FALSE;
	// create/destroy atm start button as needed


	if( ( fCreated == FALSE ) && ( fShowAtmPanelStartButton == TRUE ) )
	{
		// not created, must create

		/*
		// the ATM start button
		giPersonnelATMStartButtonImage[ 0 ]=  LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,2,-1,3,-1 );
		giPersonnelATMStartButton[ 0 ] = QuickCreateButton( giPersonnelATMStartButtonImage[ 0 ] , 519,87,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)ATMStartButtonCallback );

		// set text and what not
		SpecifyButtonText( giPersonnelATMStartButton[ 0 ] ,gsAtmStartButtonText[ 0 ] );
		SpecifyButtonUpTextColors( giPersonnelATMStartButton[ 0 ], FONT_BLACK, FONT_BLACK );
		SpecifyButtonFont( giPersonnelATMStartButton[ 0 ], PERS_FONT );
		SetButtonCursor(giPersonnelATMStartButton[ 0 ], CURSOR_LAPTOP_SCREEN);
*/
		// the stats button
		giPersonnelATMStartButtonImage[ PERSONNEL_STAT_BTN ]=  LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,2,-1,3,-1 );
		giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] = QuickCreateButton( giPersonnelATMStartButtonImage[ PERSONNEL_STAT_BTN ] , 519, 80,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										MSYS_NO_CALLBACK, (GUI_CALLBACK)PersonnelStatStartButtonCallback );

		// set text and what not
		SpecifyButtonText( giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] ,gsAtmStartButtonText[ 1 ] );
		SpecifyButtonUpTextColors( giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ], FONT_BLACK, FONT_BLACK );
		SpecifyButtonFont( giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ], PERS_FONT );
		SetButtonCursor(giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ], CURSOR_LAPTOP_SCREEN);

		// the Employment selection button
		giPersonnelATMStartButtonImage[ PERSONNEL_EMPLOYMENT_BTN ]=  LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,2,-1,3,-1 );
		giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] = QuickCreateButton( giPersonnelATMStartButtonImage[ PERSONNEL_EMPLOYMENT_BTN ] , 519, 110,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										MSYS_NO_CALLBACK, (GUI_CALLBACK)EmployementInfoButtonCallback );

		// set text and what not
		SpecifyButtonText( giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] ,gsAtmStartButtonText[ 3 ] );
		SpecifyButtonUpTextColors( giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ], FONT_BLACK, FONT_BLACK );
		SpecifyButtonFont( giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ], PERS_FONT );
		SetButtonCursor(giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ], CURSOR_LAPTOP_SCREEN);

		// the inventory selection button
		giPersonnelATMStartButtonImage[ PERSONNEL_INV_BTN ]=  LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,2,-1,3,-1 );
		giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] = QuickCreateButton( giPersonnelATMStartButtonImage[ PERSONNEL_INV_BTN ] , 519, 140,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										MSYS_NO_CALLBACK, (GUI_CALLBACK)PersonnelINVStartButtonCallback );

		// set text and what not
		SpecifyButtonText( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] ,gsAtmStartButtonText[ 2 ] );
		SpecifyButtonUpTextColors( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ], FONT_BLACK, FONT_BLACK );
		SpecifyButtonFont( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ], PERS_FONT );
		SetButtonCursor(giPersonnelATMStartButton[ PERSONNEL_INV_BTN ], CURSOR_LAPTOP_SCREEN);


		fCreated = TRUE;
	}
	else if( ( fCreated == TRUE ) && ( fShowAtmPanelStartButton == FALSE ) )
	{
		// stop showing
		/*
		RemoveButton( giPersonnelATMStartButton[ 0 ] );
		UnloadButtonImage( giPersonnelATMStartButtonImage[ 0 ] );
		*/
		RemoveButton( giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] );
		UnloadButtonImage( giPersonnelATMStartButtonImage[ PERSONNEL_STAT_BTN ] );
		RemoveButton( giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] );
		UnloadButtonImage( giPersonnelATMStartButtonImage[ PERSONNEL_EMPLOYMENT_BTN ] );
		RemoveButton( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] );
		UnloadButtonImage( giPersonnelATMStartButtonImage[ PERSONNEL_INV_BTN ] );

		fCreated = FALSE;
	}
}

void FindPositionOfPersInvSlider( void )
{
	INT32 iValue = 0;
	INT32 iNumberOfItems = 0;
	INT16 sSizeOfEachSubRegion = 0;
	INT16 sYPositionOnBar = 0;
	INT16 iCurrentItemValue = 0;

	// find out how many there are
	iValue = ( INT32 )( GetNumberOfInventoryItemsOnCurrentMerc( ) );

	// otherwise there are more than one item
	iNumberOfItems = iValue - NUMBER_OF_INVENTORY_PERSONNEL;

	if( iValue <= 0 )
	{
		iValue = 1;
	}

	// get the subregion sizes
	sSizeOfEachSubRegion = ( INT16 )( ( INT32 )( Y_SIZE_OF_PERSONNEL_SCROLL_REGION - SIZE_OF_PERSONNEL_CURSOR ) / ( INT32 )( iNumberOfItems  ) );

	// get slider position
	guiSliderPosition = uiCurrentInventoryIndex * sSizeOfEachSubRegion;

}


void HandleSliderBarClickCallback( MOUSE_REGION *pRegion, INT32 iReason )
{
	INT32 iValue = 0;
	INT32 iNumberOfItems = 0;
	POINT MousePos;
	INT16 sSizeOfEachSubRegion = 0;
	INT16 sYPositionOnBar = 0;
	INT16 iCurrentItemValue = 0;

	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN ) || ( iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT ) )
  {
		// find out how many there are
		iValue = ( INT32 )( GetNumberOfInventoryItemsOnCurrentMerc( ) );

		// make sure there are more than one page
		if( ( INT32 )uiCurrentInventoryIndex >=  iValue - NUMBER_OF_INVENTORY_PERSONNEL + 1 )
		{
			return;
		}

		// otherwise there are more than one item
		iNumberOfItems = iValue - NUMBER_OF_INVENTORY_PERSONNEL;

		// number of items is 0
		if( iNumberOfItems == 0 )
		{
			return;
		}

		// find the x,y on the slider bar
	  GetCursorPos(&MousePos);

		// get the subregion sizes
		sSizeOfEachSubRegion = ( INT16 )( ( INT32 )( Y_SIZE_OF_PERSONNEL_SCROLL_REGION - SIZE_OF_PERSONNEL_CURSOR ) / ( INT32 )( iNumberOfItems  ) );

		// get the cursor placement
		sYPositionOnBar = MousePos.y - Y_OF_PERSONNEL_SCROLL_REGION;

		if( sSizeOfEachSubRegion == 0 )
		{
			return;
		}

		// get the actual item position
		iCurrentItemValue = sYPositionOnBar / sSizeOfEachSubRegion;

		if( uiCurrentInventoryIndex != iCurrentItemValue )
		{
			// get slider position
			guiSliderPosition = iCurrentItemValue * sSizeOfEachSubRegion;

			// set current inventory value
			uiCurrentInventoryIndex = ( UINT8 )iCurrentItemValue;

			// force update
			fReDrawScreenFlag = TRUE;
		}
	}

}



void RenderSliderBarForPersonnelInventory( void )
{
	HVOBJECT hHandle;

	// render slider bar for personnel
	GetVideoObject(&hHandle, guiPersonnelInventory );
	BltVideoObject( FRAME_BUFFER, hHandle, 5,( INT16 ) ( X_OF_PERSONNEL_SCROLL_REGION ), ( INT16 ) ( guiSliderPosition + Y_OF_PERSONNEL_SCROLL_REGION), VO_BLT_SRCTRANSPARENCY,NULL);

}

void CreateDestroyATMButton( void )
{
	/*
	static BOOLEAN fCreated = FALSE;
	CHAR16 sString[ 32 ];



	// create/destroy atm start button as needed
	INT32 iCounter = 0;

	if( ( fCreated == FALSE ) && ( fShowAtmPanel == TRUE ) )
	{

		for( iCounter = 0; iCounter < 10; iCounter++ )
		{
			if( iCounter != 9 )
			{
				iNumberPadButtonsImages[ iCounter ]=LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,4,-1,6,-1 );
				swprintf( sString, L"%d", iCounter+1 );
			}
			else
			{
				iNumberPadButtonsImages[ iCounter ]=LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,7,-1,9,-1 );
				swprintf( sString, L"%d", iCounter - 9 );
			}

			iNumberPadButtons[ iCounter ] = QuickCreateButton( iNumberPadButtonsImages[ iCounter ], ( INT16 )( ATM_BUTTONS_START_X + ( ATM_BUTTON_WIDTH * ( INT16 )( iCounter % 3 )) ), ( INT16 )( ATM_BUTTONS_START_Y + ( INT16 )( ATM_BUTTON_HEIGHT * ( iCounter / 3 ))) ,
									BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
									BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)ATMNumberButtonCallback );

			if( iCounter != 9)
			{
				MSYS_SetBtnUserData(iNumberPadButtons[iCounter],0,iCounter + 1 );
			}
			else
			{
				MSYS_SetBtnUserData(iNumberPadButtons[iCounter],0, 0 );
			}
			SetButtonCursor(iNumberPadButtons[iCounter], CURSOR_LAPTOP_SCREEN);
			SpecifyButtonFont( iNumberPadButtons[iCounter], PERS_FONT );
			SpecifyButtonText( iNumberPadButtons[iCounter], sString );
			SpecifyButtonUpTextColors( iNumberPadButtons[iCounter], FONT_BLACK, FONT_BLACK );

		}


		// now slap down done, cancel, dep, withdraw
		for( iCounter = OK_ATM; iCounter < NUMBER_ATM_BUTTONS ;iCounter++ )
		{
			if( iCounter == OK_ATM )
			{
				giPersonnelATMSideButtonImage[ iCounter ]=  LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,7,-1,9,-1 );
			}
			else
			{
				giPersonnelATMSideButtonImage[ iCounter ]=  LoadButtonImage( "LAPTOP\\AtmButtons.sti" ,-1,10,-1,12,-1 );
			}

			if( ( iCounter != DEPOSIT_ATM ) && ( iCounter != WIDTHDRAWL_ATM ) )
			{
				giPersonnelATMSideButton[ iCounter ] = QuickCreateButton( giPersonnelATMSideButtonImage[ iCounter ], ( INT16 )( pAtmSideButtonPts[ iCounter ].x ), ( INT16 )( pAtmSideButtonPts[ iCounter ].y ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)ATMOtherButtonCallback );
			}
			else
			{
				giPersonnelATMSideButton[ iCounter ] = QuickCreateButton( giPersonnelATMSideButtonImage[ iCounter ], ( INT16 )( pAtmSideButtonPts[ iCounter ].x ), ( INT16 )( pAtmSideButtonPts[ iCounter ].y ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)ATMOther2ButtonCallback );
			}
			MSYS_SetBtnUserData(giPersonnelATMSideButton[iCounter],0,iCounter );
			SpecifyButtonFont( giPersonnelATMSideButton[iCounter], PERS_FONT );
			SetButtonCursor(giPersonnelATMSideButton[iCounter], CURSOR_LAPTOP_SCREEN);
			SpecifyButtonUpTextColors( giPersonnelATMSideButton[iCounter], FONT_BLACK, FONT_BLACK );
			SpecifyButtonText( giPersonnelATMSideButton[iCounter], gsAtmSideButtonText[ iCounter ] );
		}


		//SetButtonCursor(giPersonnelATMStartButton, CURSOR_LAPTOP_SCREEN);
		fCreated = TRUE;
	}
	else if( ( fCreated == TRUE ) && ( fShowAtmPanel == FALSE ) )
	{
		// stop showing
		//RemoveButton( giPersonnelATMButton );
		//UnloadButtonImage( giPersonnelATMButtonImage );

		for( iCounter = 0; iCounter < 10; iCounter++ )
		{
			UnloadButtonImage( iNumberPadButtonsImages[ iCounter ] );
			RemoveButton( iNumberPadButtons[ iCounter ] );
		}

		for( iCounter = OK_ATM; iCounter < NUMBER_ATM_BUTTONS ;iCounter++ )
		{
			RemoveButton( giPersonnelATMSideButton[ iCounter ]  );
			UnloadButtonImage( giPersonnelATMSideButtonImage[ iCounter ] );
		}

		fCreated = FALSE;
	}

	*/
}


void ATMStartButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
			fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags&=~(BUTTON_CLICKED_ON);
			fReDrawScreenFlag=TRUE;
			fShowAtmPanel = TRUE;
			fShowAtmPanelStartButton = FALSE;
			fATMFlags = 0;

		}
	}
}

void PersonnelINVStartButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fReDrawScreenFlag=TRUE;
    btn->uiFlags |= (BUTTON_CLICKED_ON);
		ButtonList[giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] ]->uiFlags &= ~(BUTTON_CLICKED_ON);
		ButtonList[giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] ]->uiFlags &= ~(BUTTON_CLICKED_ON);
//		fShowInventory = TRUE;
		gubPersonnelInfoState = PRSNL_INV;
	}
}

void PersonnelStatStartButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fReDrawScreenFlag=TRUE;
    btn->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] ]->uiFlags &= ~(BUTTON_CLICKED_ON);
		ButtonList[giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] ]->uiFlags &= ~(BUTTON_CLICKED_ON);
//		fShowInventory = FALSE;
		gubPersonnelInfoState = PRSNL_STATS;
	}
}


void EmployementInfoButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fReDrawScreenFlag=TRUE;
    btn->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] ]->uiFlags &= ~(BUTTON_CLICKED_ON);
		ButtonList[giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] ]->uiFlags &= ~(BUTTON_CLICKED_ON);
		gubPersonnelInfoState = PRSNL_EMPLOYMENT;
	}
}


void ATMOther2ButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	INT32 iValue = 0;
	SOLDIERTYPE *pSoldier = MercPtrs[ 0 ];
	INT32 cnt = 0;
	INT32 iId = 0;

	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;


	iValue = MSYS_GetBtnUserData( btn, 0 );

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
			fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);

		switch( iValue )
		{
			case( DEPOSIT_ATM ):
				fATMFlags = 2;
				fReDrawScreenFlag=TRUE;
				ButtonList[ giPersonnelATMSideButton[ WIDTHDRAWL_ATM ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
			break;
			case( WIDTHDRAWL_ATM ):
				fATMFlags = 3;
				fReDrawScreenFlag=TRUE;
				ButtonList[ giPersonnelATMSideButton[ DEPOSIT_ATM ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
			break;
		}
	}

}

void ATMOtherButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	INT32 iValue = 0;
	SOLDIERTYPE *pSoldier = MercPtrs[ 0 ];
	INT32 cnt = 0;
	INT32 iId = 0;

	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;


	iValue = MSYS_GetBtnUserData( btn, 0 );

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
			fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags&=~(BUTTON_CLICKED_ON);

			if( iCurrentPersonSelectedId != -1 )
			{
				if( fCurrentTeamMode == TRUE )
				{
					iId = GetIdOfThisSlot( iCurrentPersonSelectedId );
					cnt = 0;

					// set soldier
					pSoldier = MercPtrs[ iId ];


					switch( iValue )
					{
						case( OK_ATM ):
							if( fATMFlags == 0 )
							{
								fATMFlags = 1;
								fReDrawScreenFlag=TRUE;
								fOneFrameDelayInPersonnel = TRUE;
							}
							else if( fATMFlags == 2 )
							{
								// deposit from merc to account
								if( GetFundsOnMerc( pSoldier ) >= wcstol( sTransferString, NULL, 10 ) )
								{
									if( ( wcstol( sTransferString, NULL, 10 ) % 10 ) != 0 )
									{
										fOldATMFlags = fATMFlags;
										fATMFlags = 5;

										iValue = ( wcstol( sTransferString, NULL, 10 ) - ( wcstol( sTransferString, NULL, 10 ) % 10 ) );
										swprintf( sTransferString, L"%d", iValue );
										fReDrawScreenFlag=TRUE;
									}
									else
									{
										// transfer
										TransferFundsFromMercToBank( pSoldier, wcstol( sTransferString, NULL, 10 ) );
										sTransferString[ 0 ] = 0;
										fReDrawScreenFlag=TRUE;
									}
								}
								else
								{
									fOldATMFlags = fATMFlags;
									fATMFlags = 4;
									iValue = GetFundsOnMerc( pSoldier );
									swprintf( sTransferString, L"%d", iValue );
									fReDrawScreenFlag=TRUE;
								}
							}
							else if( fATMFlags == 3 )
							{
								// deposit from merc to account
								if( LaptopSaveInfo.iCurrentBalance >= wcstol( sTransferString, NULL, 10 ) )
								{
									if( ( wcstol( sTransferString, NULL, 10 ) % 10 ) != 0 )
									{
										fOldATMFlags = fATMFlags;
										fATMFlags = 5;

										iValue = ( wcstol( sTransferString, NULL, 10 ) - ( wcstol( sTransferString, NULL, 10 ) % 10 ) );
										swprintf( sTransferString, L"%d", iValue );
										fReDrawScreenFlag=TRUE;
									}
									else
									{
										// transfer
										TransferFundsFromBankToMerc( pSoldier, wcstol( sTransferString, NULL, 10 ) );
										sTransferString[ 0 ] = 0;
										fReDrawScreenFlag=TRUE;
									}
								}
								else
								{
									fOldATMFlags = fATMFlags;
									fATMFlags = 4;
									iValue = LaptopSaveInfo.iCurrentBalance;
									swprintf( sTransferString, L"%d", iValue );
									fReDrawScreenFlag=TRUE;
								}
							}
							else if( fATMFlags == 4 )
							{
								fATMFlags = fOldATMFlags;
								fReDrawScreenFlag=TRUE;
							}
						break;
						case( DEPOSIT_ATM ):
							fATMFlags = 2;
							fReDrawScreenFlag=TRUE;

						break;
						case( WIDTHDRAWL_ATM ):
							fATMFlags = 3;
							fReDrawScreenFlag=TRUE;
						break;
						case( CANCEL_ATM ):
							if( sTransferString[ 0 ] != 0 )
							{
								sTransferString[ 0 ] = 0;
							}
							else if( fATMFlags != 0 )
							{
								fATMFlags = 0;
								ButtonList[ giPersonnelATMSideButton[ WIDTHDRAWL_ATM ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
								ButtonList[ giPersonnelATMSideButton[ DEPOSIT_ATM ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
							}
							else
							{
								fShowAtmPanel = FALSE;
								fShowAtmPanelStartButton = TRUE;

							}
							fReDrawScreenFlag=TRUE;
						break;
						case( CLEAR_ATM ):
							sTransferString[ 0 ] = 0;
							fReDrawScreenFlag=TRUE;
						break;
					}
				}
			}
		}
	}
}

void ATMNumberButtonCallback(GUI_BUTTON *btn,INT32 reason)
{

	INT32 iValue = 0;
	INT32 iCounter = 0;
	CHAR16 sZero[ 2 ] = L"0";

	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	iValue = MSYS_GetBtnUserData( btn, 0 );

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		if(!(btn->uiFlags & BUTTON_CLICKED_ON))
		{
			fReDrawScreenFlag=TRUE;
		}
    btn->uiFlags|=(BUTTON_CLICKED_ON);
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if(btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags&=~(BUTTON_CLICKED_ON);
			// find position in value string, append character at end
			for( iCounter = 0; iCounter < ( INT32 )wcslen( sTransferString ) ; iCounter++ );
			sTransferString[ iCounter ] = ( sZero[ 0 ] + ( UINT16 )iValue );
			sTransferString[ iCounter + 1 ] = 0;
			fReDrawScreenFlag=TRUE;

			// gone too far
			if( StringPixLength( sTransferString, ATM_FONT ) >= ATM_DISPLAY_WIDTH - 10 )
			{
				sTransferString[ iCounter ] = 0;
			}
		}
	}
}

void DisplayATMAmount( void )
{

	INT16 sX = 0, sY = 0;
	CHAR16 sTempString[ 32 ];
	CHAR16 sZero[ 2 ] = L"0";
	INT32 iCounter = 0;

	if( fShowAtmPanel == FALSE )
	{
		return;
	}

	wcscpy( sTempString, sTransferString );

	if( ( sTempString[ 0 ] == 48 ) && ( sTempString[ 1 ] != 0 ) )
	{
		// strip the zero from the beginning
		for(iCounter = 1; iCounter < ( INT32 )wcslen( sTempString ); iCounter++ )
		{
			sTempString[ iCounter - 1 ] = sTempString[ iCounter ];
		}
	}

	// insert commas and dollar sign
	InsertCommasForDollarFigure( sTempString );
	InsertDollarSignInToString( sTempString );

	// set font
	SetFont( ATM_FONT );

	// set back and foreground
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );


	// right justify
	FindFontRightCoordinates( ATM_DISPLAY_X, ATM_DISPLAY_Y + 37, ATM_DISPLAY_WIDTH, ATM_DISPLAY_HEIGHT, sTempString, ATM_FONT, &sX, &sY );

	// print string
	mprintf( sX, sY, sTempString );

	return;
}


void HandleStateOfATMButtons( void )
{
	INT32 iCounter = 0;

	// disable buttons based on state
	if( ( fATMFlags == 0 ) )
	{
		for( iCounter = 0; iCounter < 10 ; iCounter++ )
		{
			DisableButton( iNumberPadButtons[ iCounter ] );
		}

		for( iCounter = 0; iCounter < NUMBER_ATM_BUTTONS; iCounter++ )
		{
			if( ( iCounter != DEPOSIT_ATM) && ( iCounter != WIDTHDRAWL_ATM ) && ( iCounter != CANCEL_ATM )  )
			{
				DisableButton( giPersonnelATMSideButton[ iCounter ] );
			}
		}
	}
	else
	{
		for( iCounter = 0; iCounter < 10; iCounter++ )
		{
			EnableButton( iNumberPadButtons[ iCounter ] );
		}

		for( iCounter = 0; iCounter < NUMBER_ATM_BUTTONS; iCounter++ )
		{
			EnableButton( giPersonnelATMSideButton[ iCounter ] );
		}
	}
}


INT32 GetFundsOnMerc( SOLDIERTYPE *pSoldier )
{
	INT32 iCurrentAmount = 0;
	INT32 iCurrentPocket = 0;
	// run through mercs pockets, if any money in them, add to total

	// error check
	if( pSoldier == NULL )
	{
		return 0;
	}

	// run through grunts pockets and count all the spare change
	for( iCurrentPocket = 0; iCurrentPocket < NUM_INV_SLOTS; iCurrentPocket++ )
	{
		if ( Item[ pSoldier->inv[ iCurrentPocket ] .usItem ].usItemClass == IC_MONEY )
		{
			iCurrentAmount += pSoldier->inv[ iCurrentPocket ].uiMoneyAmount;
		}
	}

	return iCurrentAmount;
}


BOOLEAN TransferFundsFromMercToBank( SOLDIERTYPE *pSoldier, INT32 iCurrentBalance )
{
	INT32 iCurrentPocket = 0;
	INT32 iAmountLeftToTake = iCurrentBalance;
	OBJECTTYPE ObjectToRemove;


	// move this amount of money from the grunt to the bank
	// error check
	if( pSoldier == NULL )
	{
		return FALSE;
	}

	// run through grunts pockets and count all the spare change
	for( iCurrentPocket = 0; iCurrentPocket < NUM_INV_SLOTS; iCurrentPocket++ )
	{
		if ( Item[ pSoldier->inv[ iCurrentPocket ] .usItem ].usItemClass == IC_MONEY )
		{

			// is there more left to go, or does this pocket finish it off?
			if( pSoldier->inv[ iCurrentPocket ].uiMoneyAmount > ( UINT32 )iAmountLeftToTake )
			{
				pSoldier->inv[ iCurrentPocket ].uiMoneyAmount -= iAmountLeftToTake;
				iAmountLeftToTake = 0;
			}
			else
			{
				iAmountLeftToTake  -= pSoldier->inv[ iCurrentPocket ].uiMoneyAmount;
				pSoldier->inv[ iCurrentPocket ].uiMoneyAmount = 0;

				//Remove the item out off the merc
				RemoveObjectFromSlot( pSoldier, (INT8)iCurrentPocket, &ObjectToRemove );
			}
		}
	}

	if( iAmountLeftToTake != 0 )
	{
		// something wrong
		AddTransactionToPlayersBook ( TRANSFER_FUNDS_FROM_MERC, 	pSoldier->ubProfile, GetWorldTotalMin() , ( iCurrentBalance - iAmountLeftToTake ) );
		return ( FALSE );
	}
	else
	{
		// everything ok
		AddTransactionToPlayersBook ( TRANSFER_FUNDS_FROM_MERC, 	pSoldier->ubProfile, GetWorldTotalMin() , ( iCurrentBalance ) );
		return ( TRUE );
	}
}


BOOLEAN TransferFundsFromBankToMerc( SOLDIERTYPE *pSoldier, INT32 iCurrentBalance )
{
	OBJECTTYPE pMoneyObject;

	// move this amount of money from the grunt to the bank
	// error check
	if( pSoldier == NULL )
	{
		return FALSE;
	}

	// make sure we are giving them some money
	if( iCurrentBalance <= 0 )
	{
		return ( FALSE );
	}

	// current balance
	if( iCurrentBalance > LaptopSaveInfo.iCurrentBalance )
	{
		iCurrentBalance = LaptopSaveInfo.iCurrentBalance;
	}


	// set up object
	memset( &( pMoneyObject ), 0, sizeof( OBJECTTYPE ) );

	// set up money object
	pMoneyObject.usItem = MONEY;
	pMoneyObject.ubNumberOfObjects = 1;
	pMoneyObject.bMoneyStatus = 100;
	pMoneyObject.	bStatus[0] = 100;
	pMoneyObject.uiMoneyAmount = iCurrentBalance;


	// now auto place money object
	if( AutoPlaceObject( pSoldier, &( pMoneyObject ), TRUE ) == TRUE )
	{
		// now place transaction
		AddTransactionToPlayersBook ( TRANSFER_FUNDS_TO_MERC, pSoldier->ubProfile, GetWorldTotalMin() , -( iCurrentBalance ) );
	}
	else
	{
		// error, notify player that merc doesn't have the spce for this much cash
	}

	return( TRUE );
}

void DisplayATMStrings( void )
{
	// display strings for ATM

	switch( fATMFlags )
	{
		case( 0 ):
			if( fShowAtmPanelStartButton == FALSE )
			{
				DisplayWrappedString(509, ( INT16 )( 80 ), 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 3 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
			}
		break;
		case( 2 ):
			if( sTransferString[ 0 ] != 0 )
			{
				DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 0 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
				//DisplayWrappedString(509, ( INT16 )( 80 + GetFontHeight( ATM_FONT ) ), 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 1 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

			}
			else
			{
				DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 2 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
			}
		break;
		case( 3 ):
			if( sTransferString[ 0 ] != 0 )
			{
				DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 0 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
				//DisplayWrappedString(509, ( INT16 )( 80 + GetFontHeight( ATM_FONT ) ), 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 1 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
			}
			else
			{
				DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 2 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
			}
		break;
		case( 4 ):
			// not enough money
			DisplayWrappedString(509, 80, 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 4 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
		break;
		case( 5 ):
			// not enough money
			DisplayWrappedString(509, 73, 81, 2, ATM_FONT, FONT_WHITE, sATMText[ 5 ], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
		break;

	}
}

void UpDateStateOfStartButton( void )
{
	INT32 iId = 0;

	// start button being shown?
	if( fShowAtmPanelStartButton == FALSE )
	{
		return;
	}

//	if( fShowInventory == TRUE )
	if( gubPersonnelInfoState == PRSNL_INV )
	{
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] ]->uiFlags &= ~( BUTTON_CLICKED_ON );
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] ]->uiFlags &= ~( BUTTON_CLICKED_ON );
	}
	else if( gubPersonnelInfoState == PRSNL_STATS )
	{
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] ]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] ]->uiFlags &= ~( BUTTON_CLICKED_ON );
	}
	else
	{
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] ]->uiFlags &= ~( BUTTON_CLICKED_ON );
		ButtonList[ giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] ]->uiFlags |= BUTTON_CLICKED_ON;
	}

	// if in current mercs and the currently selected guy is valid, enable button, else disable it
	if( fCurrentTeamMode == TRUE )
	{
		// is the current guy valid
		if( GetNumberOfMercsDeadOrAliveOnPlayersTeam( ) > 0 )
		{
			//EnableButton( giPersonnelATMStartButton[ 0 ] );
			EnableButton( giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] );
			EnableButton( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] );
			EnableButton( giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] );

			iId = GetIdOfThisSlot( iCurrentPersonSelectedId );

			if( iId != -1 )
			{
				if( Menptr[ iId ].bAssignment == ASSIGNMENT_POW )
				{
					//DisableButton( giPersonnelATMStartButton[ 0 ] );
					DisableButton( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] );

//					if( fShowInventory == TRUE )
					if( gubPersonnelInfoState == PRSNL_INV )
					{
//						fShowInventory = FALSE;
						gubPersonnelInfoState = PRSNL_STATS;

						fPausedReDrawScreenFlag = TRUE;
					}

					if( fATMFlags )
					{
						fATMFlags = 0;
						fPausedReDrawScreenFlag = TRUE;
					}
				}
			}


		}
		else
		{
			// not valid, disable
			//DisableButton( giPersonnelATMStartButton[ 0 ] );
			DisableButton( giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] );
			DisableButton( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] );
			DisableButton( giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] );
		}


	}
	else
	{
		// disable button
		//DisableButton( giPersonnelATMStartButton[ 0 ] );
		EnableButton( giPersonnelATMStartButton[ PERSONNEL_STAT_BTN ] );
		DisableButton( giPersonnelATMStartButton[ PERSONNEL_INV_BTN ] );
		DisableButton( giPersonnelATMStartButton[ PERSONNEL_EMPLOYMENT_BTN ] );
	}
}

void DisplayAmountOnCurrentMerc( void )
{
	// will display the amount that the merc is carrying on him or herself
	INT32 iId;
	SOLDIERTYPE *pSoldier = NULL;
	INT32 iFunds;
	CHAR16 sString[ 64 ];
	INT16 sX, sY;

	iId = GetIdOfThisSlot( iCurrentPersonSelectedId );

	if( iId == -1 )
	{
		pSoldier = NULL;
	}
	else
	{
		// set soldier
		pSoldier = MercPtrs[ iId ];
	}

	iFunds = GetFundsOnMerc( pSoldier );

	swprintf( sString, L"%d", iFunds );

	// insert commas and dollar sign
	InsertCommasForDollarFigure( sString );
	InsertDollarSignInToString( sString );

	// set font
	SetFont( ATM_FONT );

	// set back and foreground
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );

	// right justify
	FindFontRightCoordinates( ATM_DISPLAY_X, ATM_DISPLAY_Y, ATM_DISPLAY_WIDTH, ATM_DISPLAY_HEIGHT, sString, ATM_FONT, &sX, &sY );

	// print string
	mprintf( sX, sY, sString );

	return;
}

void HandlePersonnelKeyboard( void )
{
	INT32 iCounter = 0;
	INT32 iValue = 0;
	CHAR16 sZero[ 2 ] = L"0";

	InputAtom					InputEvent;
	POINT  MousePos;

	GetCursorPos(&MousePos);

  while (DequeueEvent(&InputEvent) == TRUE)
  {
		if ( (InputEvent.usEvent == KEY_DOWN ) && (InputEvent.usParam >= '0' ) && ( InputEvent.usParam <= '9') )
		{
			if( ( fShowAtmPanel ) && ( fATMFlags != 0 ) )
			{
				iValue = ( INT32 )( InputEvent.usParam - '0' );

				for( iCounter = 0; iCounter < ( INT32 )wcslen( sTransferString ) ; iCounter++ );
				sTransferString[ iCounter ] = ( sZero[ 0 ] + ( UINT16 )iValue );
				sTransferString[ iCounter + 1 ] = 0;
				fPausedReDrawScreenFlag=TRUE;

				// gone too far
				if( StringPixLength( sTransferString, ATM_FONT ) >= ATM_DISPLAY_WIDTH - 10 )
				{
					sTransferString[ iCounter ] = 0;
				}
			}
		}

		HandleKeyBoardShortCutsForLapTop( InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState );
	}
}

void RenderRectangleForPersonnelTransactionAmount( void )
{
	INT32 iLength = 0;
	INT32 iHeight = GetFontHeight( ATM_FONT );
	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;
	CHAR16 sTempString[ 32 ];
	CHAR16 sZero[ 2 ] = L"0";
	INT32	 iCounter = 0;


	wcscpy( sTempString, sTransferString );

	if( ( sTempString[ 0 ] == 48 ) && ( sTempString[ 1 ] != 0 ) )
	{
		// strip the zero from the beginning
		for(iCounter = 1; iCounter < ( INT32 )wcslen( sTempString ); iCounter++ )
		{
			sTempString[ iCounter - 1 ] = sTempString[ iCounter ];
		}
	}

	// insert commas and dollar sign
	InsertCommasForDollarFigure( sTempString );
	InsertDollarSignInToString( sTempString );

	// string not worth worrying about?
	if( wcslen( sTempString ) < 2 )
	{
		return;
	}

	// grab total length
	iLength = StringPixLength( sTempString, ATM_FONT );

	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	RestoreClipRegionToFullScreenForRectangle( uiDestPitchBYTES );
	RectangleDraw( TRUE, ( ATM_DISPLAY_X + ATM_DISPLAY_WIDTH ) - iLength - 2,  ATM_DISPLAY_Y + 35, ATM_DISPLAY_X + ATM_DISPLAY_WIDTH + 1, ATM_DISPLAY_Y + iHeight + 36, Get16BPPColor( FROMRGB( 255, 255, 255 ) ), pDestBuf );
	UnLockVideoSurface( FRAME_BUFFER );

}

void HandleTimedAtmModes( void )
{
	static BOOLEAN fOldAtmMode = 0;
	static UINT32 uiBaseTime = 0;

	if( fShowAtmPanel == FALSE )
	{
		return;
	}

	// update based on modes
	if( fATMFlags != fOldAtmMode )
	{
		uiBaseTime = GetJA2Clock();
		fOldAtmMode = fATMFlags;
		fPausedReDrawScreenFlag = TRUE;
 	}


	if( ( GetJA2Clock() - uiBaseTime ) > DELAY_PER_MODE_CHANGE_IN_ATM )
	{
		switch( fATMFlags )
		{
			case( 4 ):
			case( 5 ):
				// insufficient funds ended
				fATMFlags = fOldATMFlags;
				fPausedReDrawScreenFlag = TRUE;
			break;
		}
	}
}

BOOLEAN IsPastMercDead( INT32 iId )
{
	if( GetTheStateOfDepartedMerc( GetIdOfPastMercInSlot( iId ) ) == DEPARTED_DEAD )
	{
		return( TRUE );
		}
	else
	{
		return( FALSE );
	}

}


BOOLEAN IsPastMercFired( INT32 iId )
{
	if( GetTheStateOfDepartedMerc( GetIdOfPastMercInSlot( iId )  ) == DEPARTED_FIRED )
	{
		return( TRUE );
		}
	else
	{
		return( FALSE );
	}
}

BOOLEAN IsPastMercOther( INT32 iId )
{
	if( GetTheStateOfDepartedMerc( GetIdOfPastMercInSlot( iId )  ) == DEPARTED_OTHER )
	{
		return( TRUE );
		}
	else
	{
		return( FALSE );
	}
}



void DisplayEmploymentinformation( INT32 iId, INT32 iSlot )
{
	INT32 iCounter=0;
	wchar_t sString[50];
	wchar_t sStringA[ 50 ];
	INT16 sX, sY;
	UINT32 uiHits = 0;


	if( Menptr[iId].uiStatusFlags & SOLDIER_VEHICLE )
	{
		return;
	}

	// display the stats for a char
	for(iCounter=0;iCounter <MAX_STATS; iCounter++)
	{
		switch(iCounter)
		{

//		 case 12:

		//Remaining Contract:
		case 0:
		{
			UINT32 uiTimeUnderThisDisplayAsHours = 24*60;
			UINT32 uiMinutesInDay = 24 * 60;

				if(Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC || Menptr[iId].ubProfile == SLAY )
				{
					INT32 iTimeLeftOnContract = CalcTimeLeftOnMercContract( &Menptr[iId] );

					//if the merc is in transit
					if( Menptr[iId ].bAssignment == IN_TRANSIT )
					{
						//and if the ttime left on the cotract is greater then the contract time
						if( iTimeLeftOnContract > (INT32)( Menptr[iId].iTotalContractLength * uiMinutesInDay ) )
						{
							iTimeLeftOnContract = ( Menptr[iId].iTotalContractLength * uiMinutesInDay );
						}
					}
					// if there is going to be a both days and hours left on the contract
					if( iTimeLeftOnContract / uiMinutesInDay )
					{
						swprintf(sString, L"%d%s %d%s / %d%s",( iTimeLeftOnContract / uiMinutesInDay ), gpStrategicString[ STR_PB_DAYS_ABBREVIATION ], (iTimeLeftOnContract % uiMinutesInDay)/60, gpStrategicString[ STR_PB_HOURS_ABBREVIATION ], Menptr[iId].iTotalContractLength, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ]);
						mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
					}

					//else there is under a day left
					else
					{
						//DEF: removed 2/7/99
						swprintf(sString, L"%d%s / %d%s", (iTimeLeftOnContract % uiMinutesInDay)/60, gpStrategicString[ STR_PB_HOURS_ABBREVIATION ], Menptr[iId].iTotalContractLength, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ]);
						mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
					}

				}
				else if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC)
				{
//					swprintf(sString, L"%d%s / %d%s",Menptr[iId].iTotalContractLength, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ], ( GetWorldTotalMin( ) -Menptr[iId].iStartContractTime ) / ( 24 * 60 ), gpStrategicString[ STR_PB_DAYS_ABBREVIATION ] );

					wcscpy( sString, gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
					mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
				}
				else
				{
					wcscpy( sString, gpStrategicString[ STR_PB_NOTAPPLICABLE_ABBREVIATION ] );
					mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
				}

		   FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)+Prsnl_DATA_OffSetX),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
       mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		}
		break;

//		 case 11:
//		 case 19:
		 case 1:

			 // total contract time served
			mprintf((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter].y,pPersonnelScreenStrings[PRSNL_TXT_TOTAL_SERVICE]);

			//./DEF 2/4/99: total service days used to be calced as 'days -1'

			swprintf(sString, L"%d %s",gMercProfiles[Menptr[iId].ubProfile].usTotalDaysServed, gpStrategicString[ STR_PB_DAYS_ABBREVIATION ] );

      FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)+Prsnl_DATA_OffSetX),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
      mprintf(sX,pPersonnelScreenPoints[iCounter].y,sString);
		 break;

//		 case 13:
		 case 3:
		   // cost (PRSNL_TXT_TOTAL_COST)

/*
			 if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			 {
				 UINT32 uiDailyCost = 0;

				 if( Menptr[iId].bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK )
				 {
					 // 2 week contract
					 uiDailyCost = gMercProfiles[ Menptr[ iId ].ubProfile ].uiBiWeeklySalary / 14;
				 }
				 else if( Menptr[iId].bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK )
				 {
					 // 1 week contract
					 uiDailyCost = gMercProfiles[ Menptr[ iId ].ubProfile ].uiWeeklySalary / 7;
				 }
				 else
				 {
					 uiDailyCost = gMercProfiles[ Menptr[ iId ].ubProfile ].sSalary;
				 }

//				 swprintf( sString, L"%d",uiDailyCost * Menptr[ iId ].iTotalContractLength );
				 swprintf( sString, L"%d", gMercProfiles[ Menptr[ iId ].ubProfile ].uiTotalCostToDate );
			 }
			 else if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			 {
//					swprintf( sString, L"%d",gMercProfiles[ Menptr[ iId ].ubProfile ].sSalary * gMercProfiles[ Menptr[ iId ].ubProfile ].iMercMercContractLength );
					swprintf( sString, L"%d", gMercProfiles[ Menptr[ iId ].ubProfile ].uiTotalCostToDate );
			 }
			 else
			 {
				 //Display a $0 amount
//				 swprintf( sString, L"0" );

				 swprintf( sString, L"%d", gMercProfiles[ Menptr[ iId ].ubProfile ].uiTotalCostToDate );
			 }
*/
				swprintf( sString, L"%d", gMercProfiles[ Menptr[ iId ].ubProfile ].uiTotalCostToDate );

				// insert commas and dollar sign
				InsertCommasForDollarFigure( sString );
				InsertDollarSignInToString( sString );

/*
DEF:3/19/99:
			 if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC )
			 {
			   swprintf( sStringA, L"%s", pPersonnelScreenStrings[ PRSNL_TXT_UNPAID_AMOUNT ] );
			 }
			 else
*/
			 {
				 swprintf( sStringA, L"%s", pPersonnelScreenStrings[ PRSNL_TXT_TOTAL_COST ]  );
			 }

			 FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)+Prsnl_DATA_OffSetX),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
			 mprintf( (INT16)(pPersonnelScreenPoints[iCounter].x +(iSlot*TEXT_BOX_WIDTH) ),pPersonnelScreenPoints[ iCounter ].y,sStringA);

			 // print contract cost
			 mprintf( ( INT16 ) ( sX ) , pPersonnelScreenPoints[iCounter].y,sString);

			 if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			 {
				 // daily rate
				 if( Menptr[iId].bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK )
				 {
					 // 2 week contract
					 swprintf( sStringA, L"%d", gMercProfiles[Menptr[ iId ].ubProfile].uiBiWeeklySalary / 14 );
				   InsertCommasForDollarFigure( sStringA );
					 InsertDollarSignInToString( sStringA );
					 swprintf( sString, L"%s", sStringA );
				 }
				 else if( Menptr[iId].bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK )
				 {
					 // 1 week contract
					 swprintf( sStringA, L"%d", gMercProfiles[Menptr[ iId ].ubProfile].uiWeeklySalary / 7 );
				   InsertCommasForDollarFigure( sStringA );
					 InsertDollarSignInToString( sStringA );
					 swprintf( sString, L"%s",  sStringA );
				 }
				 else
				 {

					 swprintf( sStringA, L"%d", gMercProfiles[Menptr[ iId ].ubProfile].sSalary );
					 InsertCommasForDollarFigure( sStringA );
				   InsertDollarSignInToString( sStringA );
					 swprintf( sString,  L"%s", sStringA );
				 }
			 }
			 else if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			 {
//DEF: 99/2/7
//				 swprintf( sStringA, L"%d", gMercProfiles[Menptr[ iId ].ubProfile].sSalary * Menptr[ iId ].iTotalContractLength);
				 swprintf( sStringA, L"%d", gMercProfiles[Menptr[ iId ].ubProfile].sSalary );
				 InsertCommasForDollarFigure( sStringA );
				 InsertDollarSignInToString( sStringA );
				 swprintf( sString,   L"%s", sStringA );
			 }

			 else
			 {
				 //Display a $0 amount
//				 swprintf( sString, L"0" );
//				 InsertDollarSignInToString( sString );
				 swprintf( sStringA, L"%d", gMercProfiles[Menptr[ iId ].ubProfile].sSalary );
				 InsertCommasForDollarFigure( sStringA );
				 InsertDollarSignInToString( sStringA );
				 swprintf( sString,   L"%s", sStringA );
			 }

			 FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter].x+(iSlot*TEXT_BOX_WIDTH)+Prsnl_DATA_OffSetX),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);

//			 iCounter++;
			 iCounter++;

       // now print daily rate
			 mprintf( ( INT16 )( sX ),pPersonnelScreenPoints[ iCounter+1 ].y,sString);
			 mprintf( (INT16)(pPersonnelScreenPoints[iCounter+1].x +(iSlot*TEXT_BOX_WIDTH) ),pPersonnelScreenPoints[ iCounter +1].y, pPersonnelScreenStrings[PRSNL_TXT_DAILY_COST]);

			 break;

		 case 5:
		   // medical deposit

			 //if its a merc merc, display the salary oweing
			if( Menptr[iId].ubWhatKindOfMercAmI == MERC_TYPE__MERC )
			{
				mprintf((INT16)(pPersonnelScreenPoints[iCounter-1].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter-1].y,pPersonnelScreenStrings[PRSNL_TXT_UNPAID_AMOUNT]);

				swprintf( sString, L"%d", gMercProfiles[Menptr[ iId ].ubProfile].sSalary * gMercProfiles[Menptr[ iId ].ubProfile ].iMercMercContractLength );
				InsertCommasForDollarFigure( sString );
				InsertDollarSignInToString( sString );

				FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter-1].x+(iSlot*TEXT_BOX_WIDTH)+Prsnl_DATA_OffSetX),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
				mprintf(sX,pPersonnelScreenPoints[iCounter-1].y,sString);
			}
			else
			{
				mprintf((INT16)(pPersonnelScreenPoints[iCounter-1].x+(iSlot*TEXT_BOX_WIDTH)),pPersonnelScreenPoints[iCounter-1].y,pPersonnelScreenStrings[PRSNL_TXT_MED_DEPOSIT]);

				swprintf(sString, L"%d",gMercProfiles[Menptr[iId].ubProfile].sMedicalDepositAmount);

				// insert commas and dollar sign
				InsertCommasForDollarFigure( sString );
				InsertDollarSignInToString( sString );

				FindFontRightCoordinates((INT16)(pPersonnelScreenPoints[iCounter-1].x+(iSlot*TEXT_BOX_WIDTH)+Prsnl_DATA_OffSetX),0,TEXT_BOX_WIDTH-20,0,sString, PERS_FONT,  &sX, &sY);
				mprintf(sX,pPersonnelScreenPoints[iCounter-1].y,sString);
			}


		 break;

		}
	}
}

// AIM merc:  Returns the amount of time left on mercs contract
// MERC merc: Returns the amount of time the merc has worked
// IMP merc:	Returns the amount of time the merc has worked
// else:			returns -1
INT32 CalcTimeLeftOnMercContract( SOLDIERTYPE *pSoldier )
{
	INT32 iTimeLeftOnContract = -1;

	if(pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
	{
		iTimeLeftOnContract = pSoldier->iEndofContractTime-GetWorldTotalMin();

		if( iTimeLeftOnContract < 0 )
			iTimeLeftOnContract = 0;
	}
	else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
	{
		iTimeLeftOnContract = gMercProfiles[ pSoldier->ubProfile ].iMercMercContractLength;
	}

	else if( pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER )
	{
		iTimeLeftOnContract = pSoldier->iTotalContractLength;
	}

	else
	{
		iTimeLeftOnContract = -1;
	}

	return( iTimeLeftOnContract );
}
