#include "Font.h"
#include "Laptop.h"
#include "Local.h"
#include "Personnel.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Overhead.h"
#include "Soldier_Profile.h"
#include "Text.h"
#include "MapScreen.h"
#include "Game_Clock.h"
#include "Finances.h"
#include "LaptopSave.h"
#include "AIMMembers.h"
#include "Input.h"
#include "Random.h"
#include "Line.h"
#include "Assignments.h"
#include "Gameloop.h"
#include "Soldier_Add.h"
#include "Interface_Items.h"
#include "Weapons.h"
#include "StrategicMap.h"
#include "Animation_Data.h"
#include "Merc_Contract.h"
#include "Button_System.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "EMail.h"
#include "Soldier_Macros.h"
#include "Stubs.h" // XXX


#define IMAGE_BOX_X									395
#define IMAGE_BOX_Y									LAPTOP_SCREEN_UL_Y+24
#define IMAGE_BOX_WIDTH							112
#define IMAGE_NAME_WIDTH						106
#define IMAGE_FULL_NAME_OFFSET_Y		111
#define TEXT_BOX_WIDTH							160
#define TEXT_DELTA_OFFSET						9
#define PERS_CURR_TEAM_X LAPTOP_SCREEN_UL_X + 39 - 15
#define PERS_CURR_TEAM_Y LAPTOP_SCREEN_UL_Y + 218
#define PERS_DEPART_TEAM_Y LAPTOP_SCREEN_UL_Y + 247

#define MAX_STATS 20
#define PERS_FONT						FONT10ARIAL
#define CHAR_NAME_FONT			FONT12ARIAL
#define CHAR_NAME_LOC_X     432
#define CHAR_NAME_LOC_WIDTH 124
#define CHAR_NAME_Y         177
#define CHAR_LOC_Y          189
#define PERS_TEXT_FONT_COLOR FONT_WHITE //146
#define PERS_TEXT_FONT_ALTERNATE_COLOR FONT_YELLOW
#define PERS_FONT_COLOR FONT_WHITE


#define FACES_DIR "FACES/BIGFACES/"
#define SMALL_FACES_DIR "FACES/"

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

#define SMALL_PORTRAIT_WIDTH_NO_BORDERS		48

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
enum
{
	DEPARTED_DEAD = 0,
	DEPARTED_FIRED,
	DEPARTED_OTHER,
	DEPARTED_MARRIED,
	DEPARTED_CONTRACT_EXPIRED,
	DEPARTED_QUIT,
};

// atm button positions
#define ATM_DISPLAY_X 509
#define ATM_DISPLAY_Y 58
#define ATM_DISPLAY_HEIGHT 10
#define ATM_DISPLAY_WIDTH  81


// the number of inventory items per personnel page
#define NUMBER_OF_INVENTORY_PERSONNEL 8
#define Y_SIZE_OF_PERSONNEL_SCROLL_REGION (422 - 219)
#define X_SIZE_OF_PERSONNEL_SCROLL_REGION (589 - 573)
#define Y_OF_PERSONNEL_SCROLL_REGION 219
#define X_OF_PERSONNEL_SCROLL_REGION 573
#define SIZE_OF_PERSONNEL_CURSOR 19


// enums for the buttons in the information side bar (used with giPersonnelATMStartButton[])
enum
{
	PERSONNEL_STAT_BTN,
	PERSONNEL_EMPLOYMENT_BTN,
	PERSONNEL_INV_BTN,

	PERSONNEL_NUM_BTN,
};


//enums for the current state of the information side bar (stat panel)
enum
{
	PRSNL_STATS,
	PRSNL_EMPLOYMENT,
	PRSNL_INV,
};
static UINT8 gubPersonnelInfoState = PRSNL_STATS;


//enums for the pPersonnelScreenStrings[]
enum
{
	PRSNL_TXT_HEALTH, // HEALTH OF MERC
	PRSNL_TXT_AGILITY,
	PRSNL_TXT_DEXTERITY,
	PRSNL_TXT_STRENGTH,
	PRSNL_TXT_LEADERSHIP,
	PRSNL_TXT_WISDOM, 					//5
	PRSNL_TXT_EXP_LVL, // EXPERIENCE LEVEL
	PRSNL_TXT_MARKSMANSHIP,
	PRSNL_TXT_MECHANICAL,
	PRSNL_TXT_EXPLOSIVES,
	PRSNL_TXT_MEDICAL, 				//10
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

static UINT8 uiCurrentInventoryIndex = 0;

static UINT32 guiSliderPosition;


static const INT16 pers_stat_x       = 407;
static const INT16 pers_stat_delta_x = 407 + TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET;
static const INT16 pers_stat_data_x  = 407 + 36;
static const INT16 pers_stat_y[] =
{
	215,
	225,
	235,
	245,
	255,
	265,
	325,
	280,
	290,
	300,
	310, // 10
	405,
	395,
	425,
	435,
	455,
	390, // for contract price
	445,
	 33, // Personnel Header // XXX unused
	340,
	350, // 20
	365,
	375,
	385,
	395,
	405
};


static UINT32 guiSCREEN;
static UINT32 guiTITLE;
static UINT32 guiDEPARTEDTEAM;
static UINT32 guiCURRENTTEAM;
static UINT32 guiPersonnelInventory;

static INT32 giPersonnelButton[6];
static INT32 giPersonnelButtonImage[6];
static INT32 giPersonnelInventoryButtons[2];
static INT32 giPersonnelInventoryButtonsImages[2];

// buttons for ATM
static INT32 giPersonnelATMStartButton[3];
static INT32 giPersonnelATMStartButtonImage[3];

// the id of currently displayed merc in right half of screen
static INT32 iCurrentPersonSelectedId = -1;

static INT32 giCurrentUpperLeftPortraitNumber = 0;

// which mode are we showing?..current team?...or deadly departed?
static BOOLEAN fCurrentTeamMode = TRUE;

BOOLEAN fShowAtmPanelStartButton = TRUE;

// mouse regions
static MOUSE_REGION gPortraitMouseRegions[20];

static MOUSE_REGION gTogglePastCurrentTeam[2];

static MOUSE_REGION gMouseScrollPersonnelINV;

static INT32 iCurPortraitId = 0;


static void InitPastCharactersList(void);


void GameInitPersonnel(void)
{
	// init past characters lists
	InitPastCharactersList();
}


static void CreateDestroyCurrentDepartedMouseRegions(BOOLEAN create);
static void CreateDestroyMouseRegionsForPersonnelPortraits(BOOLEAN create);
static void CreatePersonnelButtons(void);
static INT32 GetIdOfFirstDisplayedMerc(void);
static INT32 GetNumberOfMercsDeadOrAliveOnPlayersTeam(void);
static BOOLEAN LoadPersonnelGraphics(void);
static BOOLEAN LoadPersonnelScreenBackgroundGraphics(void);
static void SetPersonnelButtonStates(void);


void EnterPersonnel(void)
{
	fReDrawScreenFlag=TRUE;

	iCurrentPersonSelectedId = -1;

	uiCurrentInventoryIndex = 0;
	guiSliderPosition = 0;

	iCurPortraitId = 0;

	// load graphics for screen
	LoadPersonnelGraphics();

	// show atm panel
	fShowAtmPanelStartButton = TRUE;

	// load personnel
	LoadPersonnelScreenBackgroundGraphics();


	// render screen
	RenderPersonnel();

	// how many people do we have?..if you have someone set default to 0
	if (GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0)
	{
		iCurrentPersonSelectedId = GetIdOfFirstDisplayedMerc();
	}

	CreateDestroyMouseRegionsForPersonnelPortraits(TRUE);
	CreateDestroyCurrentDepartedMouseRegions(TRUE);

	// create buttons for screen
	CreatePersonnelButtons();

	// set states of en- dis able buttons
	SetPersonnelButtonStates();
}


static void CreateDestroyButtonsForDepartedTeamList(void);
static void CreateDestroyPersonnelInventoryScrollButtons(void);
static void CreateDestroyStartATMButton(void);
static void DeletePersonnelButtons(void);
static void DeletePersonnelScreenBackgroundGraphics(void);
static void RemovePersonnelGraphics(void);


void ExitPersonnel(void)
{
	if (fCurrentTeamMode == FALSE)
	{
		fCurrentTeamMode = TRUE;
		CreateDestroyButtonsForDepartedTeamList();
		fCurrentTeamMode = FALSE;
	}

	// get rid of atm panel buttons
	fShowAtmPanelStartButton = FALSE;
	CreateDestroyStartATMButton();

	gubPersonnelInfoState = PRSNL_STATS;

	CreateDestroyPersonnelInventoryScrollButtons();

	// get rid of graphics
	RemovePersonnelGraphics();

	DeletePersonnelScreenBackgroundGraphics();

	// delete buttons
	DeletePersonnelButtons();

	CreateDestroyMouseRegionsForPersonnelPortraits(FALSE);
	CreateDestroyCurrentDepartedMouseRegions(FALSE);
}


static void EnableDisableDeparturesButtons(void);
static void EnableDisableInventoryScrollButtons(void);
static void HandlePersonnelKeyboard(void);


void HandlePersonnel(void)
{
	// create / destroy buttons for scrolling departed list
	CreateDestroyButtonsForDepartedTeamList();

	// enable / disable departures buttons
	EnableDisableDeparturesButtons();

	// create destroy inv buttons as needed
	CreateDestroyPersonnelInventoryScrollButtons();

	// enable disable buttons as needed
	EnableDisableInventoryScrollButtons();

	HandlePersonnelKeyboard();
}


static BOOLEAN LoadPersonnelGraphics(void)
{
	// load graphics needed for personnel screen

	// title bar
	guiTITLE = AddVideoObjectFromFile("LAPTOP/programtitlebar.sti");
	CHECKF(guiTITLE != NO_VOBJECT);

	// the background grpahics
	guiSCREEN = AddVideoObjectFromFile("LAPTOP/personnelwindow.sti");
	CHECKF(guiSCREEN != NO_VOBJECT);

	guiPersonnelInventory = AddVideoObjectFromFile("LAPTOP/personnel_inventory.sti");
	CHECKF(guiPersonnelInventory != NO_VOBJECT);

	return (TRUE);
}


static void RemovePersonnelGraphics(void)
{
	// delete graphics needed for personnel screen
	DeleteVideoObjectFromIndex(guiSCREEN);
	DeleteVideoObjectFromIndex(guiTITLE);
	DeleteVideoObjectFromIndex(guiPersonnelInventory);
}


static void DisplayAmountOnCurrentMerc(void);
static void DisplayAverageStatValuesForCurrentTeam(void);
static void DisplayFaceOfDisplayedMerc(void);
static void DisplayHighestStatValuesForCurrentTeam(void);
static void DisplayInventoryForSelectedChar(void);
static void DisplayLowestStatValuesForCurrentTeam(void);
static void DisplayNumberDeparted(void);
static void DisplayNumberOnCurrentTeam(void);
static void DisplayPastMercsPortraits(void);
static void DisplayPersonnelTeamStats(void);
static void DisplayPersonnelTextOnTitleBar(void);
static BOOLEAN DisplayPicturesOfCurrentTeam(void);
static void DisplayStateOfPastTeamMembers(void);
static BOOLEAN RenderAtmPanel(void);
static void RenderPersonnelScreenBackground(void);
static void UpDateStateOfStartButton(void);


void RenderPersonnel(void)
{
	// re-renders personnel screen
	// render main background

	BltVideoObjectFromIndex(FRAME_BUFFER, guiTITLE,  0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y -  2);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiSCREEN, 0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y + 22);

	// render personnel screen background
	RenderPersonnelScreenBackground();

	// show team
	DisplayPicturesOfCurrentTeam();

	DisplayPastMercsPortraits();

	// show selected merc
	DisplayFaceOfDisplayedMerc();

	// show current team size
	DisplayNumberOnCurrentTeam();

	// show departed team size
	DisplayNumberDeparted();

	// list stats row headers for team stats list
	DisplayPersonnelTeamStats();

	// showinventory of selected guy if applicable
	DisplayInventoryForSelectedChar();

	// the average stats for the current team
	DisplayAverageStatValuesForCurrentTeam();

	// lowest stat values
	DisplayLowestStatValuesForCurrentTeam();

	// past team
	DisplayStateOfPastTeamMembers();

	// title bar
	BlitTitleBarIcons();

	// show text on titlebar
	DisplayPersonnelTextOnTitleBar();

	// the highest stats
	DisplayHighestStatValuesForCurrentTeam();

	// render the atm panel
	RenderAtmPanel();

	DisplayAmountOnCurrentMerc();

	// en-dis-able start button
	UpDateStateOfStartButton();
}


static void DisplayCharStats(INT32 iId);
static void DisplayEmploymentinformation(INT32 iId);


static void RenderPersonnelStats(INT32 iId)
{
	// will render the stats of person iId
	SetFont(PERS_FONT);
	SetFontForeground(PERS_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);

	if (gubPersonnelInfoState == PERSONNEL_STAT_BTN)
	{
		DisplayCharStats(iId);
	}
	else if (gubPersonnelInfoState == PERSONNEL_EMPLOYMENT_BTN)
	{
		DisplayEmploymentinformation(iId);
	}
}


static BOOLEAN RenderPersonnelFace(INT32 iId, BOOLEAN fDead, BOOLEAN fFired, BOOLEAN fOther)
{
	char sTemp[100];

	// draw face to soldier iId

	// special case?..player generated merc
	if (fCurrentTeamMode == TRUE)
	{
		if ((50 < MercPtrs[iId]->ubProfile) && (57 > MercPtrs[iId]->ubProfile))
		{
			sprintf(sTemp, "%s%03d.sti", FACES_DIR, gMercProfiles[MercPtrs[iId]->ubProfile].ubFaceIndex);
		}
		else
		{
			sprintf(sTemp, "%s%02d.sti", FACES_DIR, Menptr[iId].ubProfile);
		}
	}
	else
	{
		//if this is not a valid merc
		if (!fDead && !fFired && !fOther)
		{
			return (TRUE);
		}

		if ((50 < iId) && (57 > iId))
		{
			sprintf(sTemp, "%s%03d.sti", FACES_DIR, gMercProfiles[iId].ubFaceIndex);
		}
		else
		{
			sprintf(sTemp, "%s%02d.sti", FACES_DIR, iId);
		}
	}

	if (fCurrentTeamMode == TRUE)
	{
		if (MercPtrs[iId]->uiStatusFlags & SOLDIER_VEHICLE)
		{
			return (TRUE);
		}
	}

	UINT32 guiFACE = AddVideoObjectFromFile(sTemp);
	CHECKF(guiFACE != NO_VOBJECT);

	HVOBJECT hFaceHandle = GetVideoObject(guiFACE);

	if (fCurrentTeamMode == TRUE)
	{
		if (MercPtrs[iId]->bLife <= 0)
		{
			hFaceHandle->pShades[0] = Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);
			//set the red pallete to the face
			SetObjectHandleShade(guiFACE, 0);
		}
	}
	else
	{
		if (fDead == TRUE)
		{
			hFaceHandle->pShades[0] = Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);
			//set the red pallete to the face
			SetObjectHandleShade(guiFACE, 0);
		}
	}


	BltVideoObject(FRAME_BUFFER, hFaceHandle, 0, IMAGE_BOX_X, IMAGE_BOX_Y);

	//if the merc is dead, display it
	if (!fCurrentTeamMode)
	{
		INT32 iHeightOfText;

		iHeightOfText = DisplayWrappedString(IMAGE_BOX_X, IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y, IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[iId].zName, 0, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT);

		//if the string will rap
		if ((iHeightOfText - 2) > GetFontHeight(PERS_FONT))
		{
			//raise where we display it, and rap it
			DisplayWrappedString(IMAGE_BOX_X, IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y - GetFontHeight(PERS_FONT), IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[iId].zName, 0, CENTER_JUSTIFIED);
		}
		else
		{
			DrawTextToScreen(gMercProfiles[iId].zName, IMAGE_BOX_X, IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y, IMAGE_NAME_WIDTH, PERS_FONT, PERS_FONT_COLOR, 0, CENTER_JUSTIFIED);
		}
	}

	DeleteVideoObjectFromIndex(guiFACE);
	return (TRUE);
}


static INT32 GetNumberOfDeadOnPastTeam(void);
static INT32 GetNumberOfLeftOnPastTeam(void);
static INT32 GetNumberOfOtherOnPastTeam(void);


static BOOLEAN NextPersonnelFace(void)
{
	if (iCurrentPersonSelectedId == -1)
	{
		return (TRUE);
	}

	if (fCurrentTeamMode == TRUE)
	{
		// wrap around?
		if (iCurrentPersonSelectedId == GetNumberOfMercsDeadOrAliveOnPlayersTeam() - 1)
		{
			iCurrentPersonSelectedId = 0;
			return (FALSE); //def added 3/14/99 to enable disable buttons properly
		}
		else
		{
			iCurrentPersonSelectedId++;
		}
	}
	else
	{
	if (((iCurPortraitId + 1) == (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) - giCurrentUpperLeftPortraitNumber))
	{
		// about to go off the end
		giCurrentUpperLeftPortraitNumber = 0;
		iCurPortraitId = 0;
	}
	else if (iCurPortraitId == 19)
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

	return (TRUE);
}


static BOOLEAN PrevPersonnelFace(void)
{
	if (iCurrentPersonSelectedId == -1)
	{
		return (TRUE);
	}

	if (fCurrentTeamMode == TRUE)
	{
		// wrap around?
		if (iCurrentPersonSelectedId == 0)
		{
			iCurrentPersonSelectedId = GetNumberOfMercsDeadOrAliveOnPlayersTeam() - 1;

			if (iCurrentPersonSelectedId == 0)
			{
				return (FALSE); //def added 3/14/99 to enable disable buttons properly
			}
		}
		else
		{
			iCurrentPersonSelectedId--;
		}
	}
	else
	{
		if ((iCurPortraitId == 0) && (giCurrentUpperLeftPortraitNumber == 0))
		{
			// about to go off the end
			giCurrentUpperLeftPortraitNumber = (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) - (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) % 20;
			iCurPortraitId = (GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) % 20;
			iCurPortraitId--;
		}
		else if (iCurPortraitId == 0)
		{
			giCurrentUpperLeftPortraitNumber -= 20;
			iCurPortraitId = 19;
		}
		else
		{
			iCurPortraitId--;
		}
		// get of this merc in this slot

		iCurrentPersonSelectedId = iCurPortraitId;
		fReDrawScreenFlag = TRUE;
	}

	return (TRUE);
}


static void LeftButtonCallBack(GUI_BUTTON* btn, INT32 reason);
static void RightButtonCallBack(GUI_BUTTON* btn, INT32 reason);


static void CreatePersonnelButtons(void)
{
	// left button
	giPersonnelButtonImage[0] = LoadButtonImage("LAPTOP/personnelbuttons.sti", -1, 0, -1, 1, -1);
	giPersonnelButton[0] = QuickCreateButton(giPersonnelButtonImage[0], PREV_MERC_FACE_X, MERC_FACE_SCROLL_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, LeftButtonCallBack);

	// right button
	giPersonnelButtonImage[1] = LoadButtonImage("LAPTOP/personnelbuttons.sti", -1, 2, -1, 3, -1);
	giPersonnelButton[1] = QuickCreateButton(giPersonnelButtonImage[1], NEXT_MERC_FACE_X, MERC_FACE_SCROLL_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, RightButtonCallBack);

	SetButtonCursor(giPersonnelButton[0], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(giPersonnelButton[1], CURSOR_LAPTOP_SCREEN);
}


static void DeletePersonnelButtons(void)
{
	RemoveButton(giPersonnelButton[0]);
	UnloadButtonImage(giPersonnelButtonImage[0]);
	RemoveButton(giPersonnelButton[1]);
	UnloadButtonImage(giPersonnelButtonImage[1]);
}


static void LeftButtonCallBack(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fReDrawScreenFlag = TRUE;
		PrevPersonnelFace();
		uiCurrentInventoryIndex = 0;
		guiSliderPosition = 0;
	}
}


static void RightButtonCallBack(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fReDrawScreenFlag = TRUE;
		NextPersonnelFace();
		uiCurrentInventoryIndex = 0;
		guiSliderPosition = 0;
	}
}


static void DisplayCharName(INT32 iId)
{
	// get merc's nickName, assignment, and sector location info
	INT16 sX, sY;
	SOLDIERTYPE* pSoldier;
	INT32 iHeightOfText;

	pSoldier = MercPtrs[iId];

	SetFont(CHAR_NAME_FONT);
	SetFontForeground(PERS_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);

	if (pSoldier->uiStatusFlags & SOLDIER_VEHICLE)
	{
		return;
	}

	const wchar_t* sTownName = NULL;
	if (Menptr[iId].bAssignment != ASSIGNMENT_POW &&
			Menptr[iId].bAssignment != IN_TRANSIT)
	{
		// name of town, if any
		INT8 bTownId = GetTownIdForSector(Menptr[iId].sSectorX, Menptr[iId].sSectorY);
		if (bTownId != BLANK_SECTOR) sTownName = pTownNames[bTownId];
	}

	wchar_t sString[64];
	if (sTownName != NULL)
	{
		//nick name - town name
		swprintf(sString, lengthof(sString), L"%ls - %ls", gMercProfiles[Menptr[iId].ubProfile].zNickname, sTownName);
	}
	else
	{
		//nick name
		wcslcpy(sString, gMercProfiles[Menptr[iId].ubProfile].zNickname, lengthof(sString));
	}
	FindFontCenterCoordinates(CHAR_NAME_LOC_X, 0, CHAR_NAME_LOC_WIDTH, 0, sString, CHAR_NAME_FONT, &sX, &sY);
	mprintf(sX, CHAR_NAME_Y, sString);

	const wchar_t* Assignment = pPersonnelAssignmentStrings[Menptr[iId].bAssignment];
	FindFontCenterCoordinates(CHAR_NAME_LOC_X, 0, CHAR_NAME_LOC_WIDTH, 0, Assignment, CHAR_NAME_FONT, &sX, &sY);
	mprintf(sX, CHAR_LOC_Y, Assignment);

	//
	// Display the mercs FULL name over top of their portrait
	//

	//first get height of text to be displayed
	iHeightOfText = DisplayWrappedString(IMAGE_BOX_X, IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y, IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[Menptr[iId].ubProfile].zName, 0, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT);

	//if the string will rap
	if ((iHeightOfText - 2) > GetFontHeight(PERS_FONT))
	{
		//raise where we display it, and rap it
		DisplayWrappedString(IMAGE_BOX_X, IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y - GetFontHeight(PERS_FONT), IMAGE_NAME_WIDTH, 1, PERS_FONT, PERS_FONT_COLOR, gMercProfiles[Menptr[iId].ubProfile].zName, 0, CENTER_JUSTIFIED);
	}
	else
	{
		DrawTextToScreen(gMercProfiles[Menptr[iId].ubProfile].zName, IMAGE_BOX_X, IMAGE_BOX_Y + IMAGE_FULL_NAME_OFFSET_Y, IMAGE_NAME_WIDTH, PERS_FONT, PERS_FONT_COLOR, 0, CENTER_JUSTIFIED);
	}
}


static void DisplayCharStats(INT32 iId)
{
	wchar_t sString[50];
	INT16 sX, sY;

	const SOLDIERTYPE* s = &Menptr[iId];
	if (s->uiStatusFlags & SOLDIER_VEHICLE) return;

	const MERCPROFILESTRUCT* p = &gMercProfiles[s->ubProfile];
	BOOLEAN fAmIaRobot = AM_A_ROBOT(s);

	// display the stats for a char
	for (INT32 i = 0; i < MAX_STATS; i++)
	{
		switch (i)
		{
			case 0:
				// health
				if (s->bAssignment != ASSIGNMENT_POW)
				{
					if (p->bLifeDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bLifeDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d/%d", s->bLife, s->bLifeMax);
				}
				else
				{
					wcslcpy(sString, pPOWStrings[1], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[PRSNL_TXT_HEALTH]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 1:
				// agility
				if (!fAmIaRobot)
				{
					if (p->bAgilityDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bAgilityDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bAgility);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 2:
				// dexterity
				if (!fAmIaRobot)
				{
					if (p->bDexterityDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bDexterityDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bDexterity);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 3:
				// strength
				if (!fAmIaRobot)
				{
					if (p->bStrengthDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bStrengthDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bStrength);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 4:
				// leadership
				if (!fAmIaRobot)
				{
					if (p->bLeadershipDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bLeadershipDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bLeadership);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 5:
				// wisdom
				if (!fAmIaRobot)
				{
					if (p->bWisdomDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bWisdomDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bWisdom);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 6:
				// exper
				if (!fAmIaRobot)
				{
					if (p->bExpLevelDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bExpLevelDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bExpLevel);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 7:
				//mrkmanship
				if (!fAmIaRobot)
				{
					if (p->bMarksmanshipDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bMarksmanshipDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bMarksmanship);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 8:
				// mech
				if (!fAmIaRobot)
				{
					if (p->bMechanicDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bMechanicDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bMechanical);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 9:
				// exp
				if (!fAmIaRobot)
				{
					if (p->bExplosivesDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bExplosivesDelta);
						FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bExplosive);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 10:
				// med
				if (!fAmIaRobot)
				{
					if (p->bMedicalDelta > 0)
					{
						swprintf(sString, lengthof(sString), L"( %+d )", p->bMedicalDelta);
					FindFontRightCoordinates(pers_stat_delta_x, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
					mprintf(sX, pers_stat_y[i], sString);
					}
					swprintf(sString, lengthof(sString), L"%d", s->bMedical);
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString));
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 14:
				// kills
				mprintf(pers_stat_x, pers_stat_y[21], pPersonnelScreenStrings[PRSNL_TXT_KILLS]);
				swprintf(sString, lengthof(sString), L"%d", p->usKills);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[21], sString);
				break;

			case 15:
				// assists
				mprintf(pers_stat_x, pers_stat_y[22], pPersonnelScreenStrings[PRSNL_TXT_ASSISTS]);
				swprintf(sString, lengthof(sString), L"%d", p->usAssists);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[22], sString);
				break;

			case 16:
			{
				// shots/hits
				mprintf(pers_stat_x, pers_stat_y[23], pPersonnelScreenStrings[PRSNL_TXT_HIT_PERCENTAGE]);
				UINT32 uiHits;
				// check we have shot at least once
				if (p->usShotsFired > 0)
				{
					uiHits = 100 * p->usShotsHit / p->usShotsFired;
				}
				else
				{
					// no, set hit % to 0
					uiHits = 0;
				}
				swprintf(sString, lengthof(sString), L"%d %%", uiHits);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[23], L"%ls", sString);
				break;
			}

			case 17:
				// battles
				mprintf(pers_stat_x, pers_stat_y[24], pPersonnelScreenStrings[PRSNL_TXT_BATTLES]);
				swprintf(sString, lengthof(sString), L"%d", p->usBattlesFought);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[24], sString);
				break;

			case 18:
				// wounds
				mprintf(pers_stat_x, pers_stat_y[25], pPersonnelScreenStrings[PRSNL_TXT_TIMES_WOUNDED]);
				swprintf(sString, lengthof(sString), L"%d", p->usTimesWounded);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[25], sString);
				break;

			// The Mercs Skills
			case 19:
			{
				//Display the 'Skills' text
				mprintf(pers_stat_x, pers_stat_y[19], pPersonnelScreenStrings[PRSNL_TXT_SKILLS]);

				//KM: April 16, 1999
				//Added support for the German version, which has potential string overrun problems.  For example, the text "Skills:" can
				//overlap "NightOps (Expert)" because the German strings are much longer.  In these cases, I ensure that the right
				//justification of the traits don't overlap.  If it would, I move it over to the right.
				const INT32 iWidth = StringPixLength(pPersonnelScreenStrings[PRSNL_TXT_SKILLS], PERS_FONT);
				const INT32 iMinimumX = iWidth + pers_stat_x + 2;

				if (!fAmIaRobot)
				{
					INT8 bSkill1 = p->bSkillTrait;
					INT8 bSkill2 = p->bSkillTrait2;

					if (bSkill1 == NO_SKILLTRAIT) bSkill1 = bSkill2;
					if (bSkill1 != NO_SKILLTRAIT)
					{
						if (bSkill1 == bSkill2)
						{
							// The 2 skills are the same, add the '(expert)' at the end
							swprintf(sString, lengthof(sString), L"%ls %ls", gzMercSkillText[bSkill1], gzMercSkillText[NUM_SKILLTRAITS]);
							FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);

							//KM: April 16, 1999
							//Perform the potential overrun check
							if (sX <= iMinimumX)
							{
								FindFontRightCoordinates(pers_stat_x + TEXT_BOX_WIDTH - 20 + TEXT_DELTA_OFFSET, 0, 30, 0, sString, PERS_FONT, &sX, &sY);
								sX = (INT16)max(sX, iMinimumX);
							}

							mprintf(sX, pers_stat_y[19], sString);
						}
						else
						{
							const wchar_t* Skill = gzMercSkillText[bSkill1];

							FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, Skill, PERS_FONT, &sX, &sY);

							//KM: April 16, 1999
							//Perform the potential overrun check
							sX = max(sX, iMinimumX);

							mprintf(sX, pers_stat_y[19], Skill);

							if (bSkill2 != NO_SKILLTRAIT)
							{
								const wchar_t* Skill = gzMercSkillText[bSkill2];

								FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, Skill, PERS_FONT, &sX, &sY);

								//KM: April 16, 1999
								//Perform the potential overrun check
								sX = (INT16)max(sX, iMinimumX);

								mprintf(sX, pers_stat_y[20], Skill);
							}
						}
					}
					else
					{
						const wchar_t* NoSkill = pPersonnelScreenStrings[PRSNL_TXT_NOSKILLS];
						FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, NoSkill, PERS_FONT, &sX, &sY);
						mprintf(sX, pers_stat_y[19], NoSkill);
					}
				}
				else
				{
					wcslcpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION], lengthof(sString)); // XXX unused?
				}
				break;
			}
		}
	}
}


static void SetPersonnelButtonStates(void)
{
	// this function will look at what page we are viewing, enable and disable buttons as needed
	if (!PrevPersonnelFace())
	{
		// first page, disable left buttons
		DisableButton(giPersonnelButton[0]);
	}
	else
	{
		// enable buttons
		NextPersonnelFace();
		EnableButton(giPersonnelButton[0]);
	}

	if (!NextPersonnelFace())
	{
		DisableButton(giPersonnelButton[1]);
	}
	else
	{
		// decrement page
		PrevPersonnelFace();
		EnableButton(giPersonnelButton[1]);
	}
}


static void RenderPersonnelScreenBackground(void)
{
	// this fucntion will render the background for the personnel screen
	BltVideoObjectFromIndex(FRAME_BUFFER, fCurrentTeamMode ? guiCURRENTTEAM : guiDEPARTEDTEAM, 0, DEPARTED_X, DEPARTED_Y);
}


static BOOLEAN LoadPersonnelScreenBackgroundGraphics(void)
{
	// will load the graphics for the personeel screen background

	// departed bar
	guiDEPARTEDTEAM = AddVideoObjectFromFile("LAPTOP/departed.sti");
	CHECKF(guiDEPARTEDTEAM != NO_VOBJECT);

	// current bar
	guiCURRENTTEAM = AddVideoObjectFromFile("LAPTOP/CurrentTeam.sti");
	CHECKF(guiCURRENTTEAM != NO_VOBJECT);

	return TRUE;
}


static void DeletePersonnelScreenBackgroundGraphics(void)
{
	// delete background V/O's
	DeleteVideoObjectFromIndex(guiCURRENTTEAM);
	DeleteVideoObjectFromIndex(guiDEPARTEDTEAM);
}


static INT32 GetNumberOfMercsOnPlayersTeam(void)
{
	INT32 cnt = 0;
	INT32 iCounter = 0;

	// grab number on team
	const SOLDIERTYPE* pSoldier = MercPtrs[0];

	// no soldiers

	for (const SOLDIERTYPE* i = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, i++)
	{
		if (i->bActive && !(i->uiStatusFlags & SOLDIER_VEHICLE) && i->bLife > 0)
			iCounter++;
	}

	return iCounter;
}


static INT32 GetNumberOfMercsDeadOrAliveOnPlayersTeam(void)
{
	INT32 cnt = 0;
	INT32 iCounter = 0;

	// grab number on team
	const SOLDIERTYPE* pSoldier = MercPtrs[0];

	// no soldiers

	for (const SOLDIERTYPE* i = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, i++)
	{
		if (i->bActive && !(i->uiStatusFlags & SOLDIER_VEHICLE))
			iCounter++;
	}

	return iCounter;
}


static void PersonnelPortraitCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionsForPersonnelPortraits(BOOLEAN create)
{
	// creates/destroys mouse regions for portraits
	static BOOLEAN fCreated = FALSE;

	if (!fCreated && create)
	{
		// create regions
		for (INT16 i = 0; i < PERSONNEL_PORTRAIT_NUMBER; i++)
		{
			const UINT16 tlx = SMALL_PORTRAIT_START_X + i % PERSONNEL_PORTRAIT_NUMBER_WIDTH * SMALL_PORT_WIDTH;
			const UINT16 tly = SMALL_PORTRAIT_START_Y + i / PERSONNEL_PORTRAIT_NUMBER_WIDTH * SMALL_PORT_HEIGHT;
			const UINT16 brx = tlx + SMALL_PORTRAIT_WIDTH;
			const UINT16 bry = tly + SMALL_PORTRAIT_HEIGHT;
			MSYS_DefineRegion(&gPortraitMouseRegions[i], tlx, tly, brx, bry, MSYS_PRIORITY_HIGHEST, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelPortraitCallback);
			MSYS_SetRegionUserData(&gPortraitMouseRegions[i], 0, i);
		}

		fCreated = TRUE;
	}
	else if (fCreated && !create)
	{
		// destroy regions
		for (INT16 i = 0; i < PERSONNEL_PORTRAIT_NUMBER; i++)
		{
			MSYS_RemoveRegion(&gPortraitMouseRegions[i]);
		}

		fCreated = FALSE;
	}
}


static BOOLEAN DisplayPicturesOfCurrentTeam(void)
{
	// will display the 20 small portraits of the current team

	// get number of mercs on team
	const INT32 iTotalOnTeam = GetNumberOfMercsDeadOrAliveOnPlayersTeam();

	if (iTotalOnTeam == 0 || fCurrentTeamMode == FALSE)
	{
		// nobody on team, leave
		return TRUE;
	}

	// start id
	const INT32 iId = gTacticalStatus.Team[MercPtrs[0]->bTeam].bFirstID;

	INT32 iCnt = 0;
	for (INT32 i = 0; i < iTotalOnTeam; iCnt++)
	{
		const SOLDIERTYPE* merc = MercPtrs[iId + iCnt]; // XXX TODO000C
		if (!merc->bActive) continue;

		const SOLDIERTYPE* man = &Menptr[iId + iCnt]; // XXX TODO000C
		// found the next actual guy
		char sTemp[100];
		if (50 < merc->ubProfile && merc->ubProfile < 57)
		{
			sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR, gMercProfiles[merc->ubProfile].ubFaceIndex);
		}
		else
		{
			sprintf(sTemp, "%s%02d.sti", SMALL_FACES_DIR, man->ubProfile);
		}

		UINT32 guiFACE = AddVideoObjectFromFile(sTemp);
		CHECKF(guiFACE != NO_VOBJECT);

		HVOBJECT hFaceHandle = GetVideoObject(guiFACE);

		if (man->bLife <= 0)
		{
			hFaceHandle->pShades[0] = Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);
			//set the red pallete to the face
			SetObjectHandleShade(guiFACE, 0);
		}

		const INT32 x = SMALL_PORTRAIT_START_X + (i % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH;
		const INT32 y = SMALL_PORTRAIT_START_Y + (i / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT;
		BltVideoObject(FRAME_BUFFER, hFaceHandle, 0, x, y);

		if (man->bLife <= 0)
		{
			//if the merc is dead, display it
			DrawTextToScreen(AimPopUpText[AIM_MEMBER_DEAD], x, y + SMALL_PORT_HEIGHT / 2, SMALL_PORTRAIT_WIDTH_NO_BORDERS, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
		}

		DeleteVideoObjectFromIndex(guiFACE);
		i++;
	}

	return TRUE;
}


static INT32 GetIdOfThisSlot(INT32 iSlot);
static INT32 GetNumberOfPastMercsOnPlayersTeam(void);


static void PersonnelPortraitCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iPortraitId = 0;
	INT32 iOldPortraitId;

	iPortraitId = MSYS_GetRegionUserData(pRegion, 0);
	iOldPortraitId = iCurrentPersonSelectedId;

	// callback handler for the minize region that is attatched to the laptop program icon
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// get id of portrait
		if (fCurrentTeamMode == TRUE)
		{
			// valid portrait, set up id
			if (iPortraitId >= GetNumberOfMercsDeadOrAliveOnPlayersTeam())
			{
				// not a valid id, leave
				return;
			}

			iCurrentPersonSelectedId = iPortraitId;
			fReDrawScreenFlag = TRUE;
		}
		else
		{
			if (iPortraitId >= GetNumberOfPastMercsOnPlayersTeam())
			{
				return;
			}
			iCurrentPersonSelectedId = iPortraitId;
			fReDrawScreenFlag = TRUE;
			iCurPortraitId = iPortraitId;
		}

		//if the selected merc is valid, and they are a POW, change to the inventory display
		if (iCurrentPersonSelectedId != -1 && Menptr[GetIdOfThisSlot(iCurrentPersonSelectedId)].bAssignment == ASSIGNMENT_POW && gubPersonnelInfoState == PERSONNEL_INV_BTN)
		{
			gubPersonnelInfoState = PERSONNEL_STAT_BTN;
		}

		if (iOldPortraitId != iPortraitId)
		{
			uiCurrentInventoryIndex = 0;
			guiSliderPosition = 0;
		}
	}

	if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		if (fCurrentTeamMode == TRUE)
		{
			// valid portrait, set up id
			if (iPortraitId >= GetNumberOfMercsDeadOrAliveOnPlayersTeam())
			{
				// not a valid id, leave
				return;
			}

			//if the user is rigt clicking on the same face
			if (iCurrentPersonSelectedId == iPortraitId)
			{
				//increment the info page when the user right clicks
				if (gubPersonnelInfoState < PERSONNEL_NUM_BTN - 1)
					gubPersonnelInfoState++;
				else
					gubPersonnelInfoState = PERSONNEL_STAT_BTN;
			}

			iCurrentPersonSelectedId = iPortraitId;
			fReDrawScreenFlag = TRUE;

			uiCurrentInventoryIndex = 0;
			guiSliderPosition = 0;

			//if the selected merc is valid, and they are a POW, change to the inventory display
			if (iCurrentPersonSelectedId != -1 && Menptr[GetIdOfThisSlot(iCurrentPersonSelectedId)].bAssignment == ASSIGNMENT_POW && gubPersonnelInfoState == PERSONNEL_INV_BTN)
			{
				gubPersonnelInfoState = PERSONNEL_STAT_BTN;
			}
		}
	}
}


static void DisplayDepartedCharName(INT32 iId, INT32 iState);
static void DisplayDepartedCharStats(INT32 iId, INT32 iState);
static BOOLEAN DisplayHighLightBox(void);
static INT32 GetIdOfPastMercInSlot(INT32 iSlot);
static INT32 GetTheStateOfDepartedMerc(INT32 iId);
static BOOLEAN IsPastMercDead(INT32 iId);
static BOOLEAN IsPastMercFired(INT32 iId);
static BOOLEAN IsPastMercOther(INT32 iId);


static void DisplayFaceOfDisplayedMerc(void)
{
	// valid person?, display
	if (iCurrentPersonSelectedId != -1)
	{
		// highlight it
		DisplayHighLightBox();

		// if showing inventory, leave
		if (fCurrentTeamMode == TRUE)
		{
			RenderPersonnelFace(GetIdOfThisSlot(iCurrentPersonSelectedId), FALSE, FALSE, FALSE);
			DisplayCharName(GetIdOfThisSlot(iCurrentPersonSelectedId));

			if (gubPersonnelInfoState == PRSNL_INV)
			{
				return;
			}

			RenderPersonnelStats(GetIdOfThisSlot(iCurrentPersonSelectedId));
		}
		else
		{
			RenderPersonnelFace(GetIdOfPastMercInSlot(iCurrentPersonSelectedId), IsPastMercDead(iCurrentPersonSelectedId), IsPastMercFired(iCurrentPersonSelectedId), IsPastMercOther(iCurrentPersonSelectedId));
			DisplayDepartedCharName(GetIdOfPastMercInSlot(iCurrentPersonSelectedId), GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iCurrentPersonSelectedId)));

			if (gubPersonnelInfoState == PRSNL_INV)
			{
				return;
			}

			DisplayDepartedCharStats(GetIdOfPastMercInSlot(iCurrentPersonSelectedId), GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iCurrentPersonSelectedId)));
		}
	}
}


static void RenderInventoryForCharacter(INT32 iId, INT32 iSlot);


static void DisplayInventoryForSelectedChar(void)
{
	// display the inventory for this merc
	if (gubPersonnelInfoState != PRSNL_INV)
	{
		return;
	}

	CreateDestroyPersonnelInventoryScrollButtons();

	if (fCurrentTeamMode == TRUE)
	{
		RenderInventoryForCharacter(GetIdOfThisSlot(iCurrentPersonSelectedId), 0);
	}
	else
	{
		RenderInventoryForCharacter(GetIdOfPastMercInSlot(iCurrentPersonSelectedId), 0);
	}
}


static void RenderSliderBarForPersonnelInventory(void);


static void RenderInventoryForCharacter(INT32 iId, INT32 iSlot)
{
	UINT8 ubCounter = 0;
	SOLDIERTYPE* pSoldier;
	INT16 PosX, PosY;
	UINT8 ubItemCount = 0;
	UINT8 ubUpToCount = 0;
	INT16 sX, sY;
	CHAR16 sString[128];
	INT32 cnt = 0;
	INT32 iTotalAmmo = 0;

	BltVideoObjectFromIndex(FRAME_BUFFER, guiPersonnelInventory, 0, 397, 200);

	if (fCurrentTeamMode == FALSE)
	{
		return;
	}

	// render the bar for the character
	RenderSliderBarForPersonnelInventory();

	pSoldier = &Menptr[iId];

	//if this is a robot, dont display any inventory
	if (AM_A_ROBOT(pSoldier))
	{
		return;
	}

	for (ubCounter = 0; ubCounter < NUM_INV_SLOTS; ubCounter++)
	{
		PosX = 397 + 3;
		PosY = 200 + 8 + (ubItemCount * (29));

		//if the character is a robot, only display the inv for the hand pos
		if (pSoldier->ubProfile == ROBOT && ubCounter != HANDPOS)
		{
			continue;
		}

		if (pSoldier->inv[ubCounter].ubNumberOfObjects)
		{
			if (uiCurrentInventoryIndex > ubUpToCount)
			{
				ubUpToCount++;
			}
			else
			{
				INT16 sIndex = pSoldier->inv[ubCounter].usItem;
				const INVTYPE* pItem = &Item[sIndex];
				UINT32 ItemVOIdx = GetInterfaceGraphicForItem(pItem);

				const ETRLEObject* pTrav = GetVideoObjectETRLESubregionProperties(ItemVOIdx, pItem->ubGraphicNum);
				UINT32 usHeight = pTrav->usHeight;
				UINT32 usWidth  = pTrav->usWidth;
				INT16  sCenX    = PosX + abs(57 - usWidth)  / 2 - pTrav->sOffsetX;
				INT16  sCenY    = PosY + abs(22 - usHeight) / 2 - pTrav->sOffsetY;

				BltVideoObjectOutlineFromIndex(FRAME_BUFFER, ItemVOIdx, pItem->ubGraphicNum, sCenX, sCenY, 0, FALSE);

				SetFont(FONT10ARIAL);
				SetFontForeground(FONT_WHITE);
				SetFontBackground(FONT_BLACK);
				SetFontDestBuffer(FRAME_BUFFER, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

				wcslcpy(sString, ItemNames[sIndex], lengthof(sString));
				ReduceStringLength(sString, lengthof(sString), 171 - 75, FONT10ARIAL);
				mprintf(PosX + 65, PosY + 3, sString);

				// condition
				if (Item[pSoldier->inv[ubCounter].usItem].usItemClass & IC_AMMO)
				{
					// Ammo
					iTotalAmmo = 0;
					if (pSoldier->inv[ubCounter].ubNumberOfObjects > 1)
					{
						for (cnt = 0; cnt < pSoldier->inv[ubCounter].ubNumberOfObjects; cnt++)
						{
							// get total ammo
							iTotalAmmo += pSoldier->inv[ubCounter].ubShotsLeft[cnt];
						}
					}
					else
					{
						iTotalAmmo= pSoldier->inv[ubCounter].ubShotsLeft[0];
					}

					swprintf(sString, lengthof(sString), L"%d/%d", iTotalAmmo, (pSoldier->inv[ubCounter].ubNumberOfObjects * Magazine[Item[pSoldier->inv[ubCounter].usItem].ubClassIndex].ubMagSize));
					FindFontRightCoordinates(PosX + 65, PosY + 15, 171 - 75, GetFontHeight(FONT10ARIAL), sString, FONT10ARIAL, &sX, &sY);
				}
				else
				{
					swprintf(sString, lengthof(sString), L"%2d%%", pSoldier->inv[ubCounter].bStatus[0]);
					FindFontRightCoordinates(PosX + 65, PosY + 15, 171 - 75, GetFontHeight(FONT10ARIAL), sString, FONT10ARIAL, &sX, &sY);
				}

				mprintf(sX, sY, L"%ls", sString);

				if (Item[pSoldier->inv[ubCounter].usItem].usItemClass & IC_GUN)
				{
					wcslcpy(sString, AmmoCaliber[Weapon[Item[pSoldier->inv[ubCounter].usItem].ubClassIndex].ubCalibre], lengthof(sString));
					ReduceStringLength(sString, lengthof(sString), 171 - 75, FONT10ARIAL);
					mprintf(PosX + 65, PosY + 15, sString);
				}

				// if more than 1?
				if (pSoldier->inv[ubCounter].ubNumberOfObjects > 1)
				{
					swprintf(sString, lengthof(sString), L"x%d",  pSoldier->inv[ubCounter].ubNumberOfObjects);
					FindFontRightCoordinates(PosX, PosY + 15, 58, GetFontHeight(FONT10ARIAL), sString, FONT10ARIAL, &sX, &sY);
					mprintf(sX, sY, sString);
				}

				// display info about it
				ubItemCount++;
			}
		}

		if (ubItemCount == NUMBER_OF_INVENTORY_PERSONNEL)
		{
			ubCounter = NUM_INV_SLOTS;
		}
	}
}


static void FindPositionOfPersInvSlider(void);


static void InventoryUpButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT ||
			reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (uiCurrentInventoryIndex == 0) return;
		uiCurrentInventoryIndex--;
		fReDrawScreenFlag = TRUE;
		FindPositionOfPersInvSlider();
	}
}


static INT32 GetNumberOfInventoryItemsOnCurrentMerc(void);


static void InventoryDownButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT ||
			reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if ((INT32)uiCurrentInventoryIndex >= (INT32)(GetNumberOfInventoryItemsOnCurrentMerc() - NUMBER_OF_INVENTORY_PERSONNEL))
		{
			return;
		}
		uiCurrentInventoryIndex++;
		fReDrawScreenFlag = TRUE;
		FindPositionOfPersInvSlider();
	}
}


// decide which buttons can and can't be accessed based on what the current item is
static void EnableDisableInventoryScrollButtons(void)
{
	if (gubPersonnelInfoState != PRSNL_INV)
	{
		return;
	}

	if (uiCurrentInventoryIndex == 0)
	{
		ButtonList[giPersonnelInventoryButtons[0]]->uiFlags &= ~(BUTTON_CLICKED_ON);
		DisableButton(giPersonnelInventoryButtons[0]);
	}
	else
	{
		EnableButton(giPersonnelInventoryButtons[0]);
	}


	if ((INT32)uiCurrentInventoryIndex >= (INT32)(GetNumberOfInventoryItemsOnCurrentMerc() - NUMBER_OF_INVENTORY_PERSONNEL))
	{
		ButtonList[giPersonnelInventoryButtons[1]]->uiFlags &= ~(BUTTON_CLICKED_ON);
		DisableButton(giPersonnelInventoryButtons[1]);
	}
	else
	{
		EnableButton(giPersonnelInventoryButtons[1]);
	}
}


static INT32 GetNumberOfInventoryItemsOnCurrentMerc(void)
{
	// in current team mode?..nope...move on
	if (!fCurrentTeamMode) return 0;

	INT32 iId = GetIdOfThisSlot(iCurrentPersonSelectedId);
	const SOLDIERTYPE* pSoldier = &Menptr[iId];

	UINT8 ubCount = 0;
	for (UINT8 ubCounter = 0; ubCounter < NUM_INV_SLOTS; ubCounter++)
	{
		if (pSoldier->inv[ubCounter].ubNumberOfObjects > 0 && pSoldier->inv[ubCounter].usItem)
		{
			ubCount++;
		}
	}

	return ubCount;
}


static void HandleSliderBarClickCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyPersonnelInventoryScrollButtons(void)
{
	static BOOLEAN fCreated = FALSE;

	if ((gubPersonnelInfoState == PRSNL_INV) && (fCreated == FALSE))
	{
		// create buttons
		giPersonnelInventoryButtonsImages[0] = LoadButtonImage("LAPTOP/personnel_inventory.sti", -1, 1, -1, 2, -1);
		giPersonnelInventoryButtons[0] = QuickCreateButton(giPersonnelInventoryButtonsImages[0], 176 + 397, 2 + 200, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, InventoryUpButtonCallback);

		giPersonnelInventoryButtonsImages[1] = LoadButtonImage("LAPTOP/personnel_inventory.sti", -1, 3, -1, 4, -1);
		giPersonnelInventoryButtons[1] = QuickCreateButton(giPersonnelInventoryButtonsImages[1], 397 + 176, 200 + 223, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, InventoryDownButtonCallback);

		// set up cursors for these buttons
		SetButtonCursor(giPersonnelInventoryButtons[0], CURSOR_LAPTOP_SCREEN);
		SetButtonCursor(giPersonnelInventoryButtons[1], CURSOR_LAPTOP_SCREEN);

		MSYS_DefineRegion(&gMouseScrollPersonnelINV, X_OF_PERSONNEL_SCROLL_REGION, Y_OF_PERSONNEL_SCROLL_REGION, X_OF_PERSONNEL_SCROLL_REGION + X_SIZE_OF_PERSONNEL_SCROLL_REGION, Y_OF_PERSONNEL_SCROLL_REGION + Y_SIZE_OF_PERSONNEL_SCROLL_REGION, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, HandleSliderBarClickCallback);

		fCreated = TRUE;
	}
	else if ((fCreated == TRUE) && (gubPersonnelInfoState != PERSONNEL_INV_BTN))
	{
		// destroy buttons
		RemoveButton(giPersonnelInventoryButtons[0]);
		UnloadButtonImage(giPersonnelInventoryButtonsImages[0]);
		RemoveButton(giPersonnelInventoryButtons[1]);
		UnloadButtonImage(giPersonnelInventoryButtonsImages[1]);

		MSYS_RemoveRegion(&gMouseScrollPersonnelINV);

		fCreated = FALSE;
	}
}


static void DisplayCostOfCurrentTeam(void);


static void DisplayNumberOnCurrentTeam(void)
{
	// display number on team
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	if (fCurrentTeamMode == TRUE)
	{
		mprintf(PERS_CURR_TEAM_X, PERS_CURR_TEAM_Y, L"%ls ( %d )", pPersonelTeamStrings[0], GetNumberOfMercsDeadOrAliveOnPlayersTeam());
	}
	else
	{
		const wchar_t* s = pPersonelTeamStrings[0];
		INT16 sX = 0;
		INT16 sY = 0;
		FindFontCenterCoordinates(PERS_CURR_TEAM_X, 0, 65, 0, s, FONT10ARIAL, &sX, &sY);
		mprintf(sX, PERS_CURR_TEAM_Y, s);
	}

	// now the cost of the current team, if applicable
	DisplayCostOfCurrentTeam();
}


static void DisplayNumberDeparted(void)
{
	// display number departed from team
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	if (fCurrentTeamMode == FALSE)
	{
		mprintf(PERS_CURR_TEAM_X, PERS_DEPART_TEAM_Y, L"%ls ( %d )", pPersonelTeamStrings[1], GetNumberOfPastMercsOnPlayersTeam());
	}
	else
	{
		const wchar_t* s = pPersonelTeamStrings[1];
		INT16 sX = 0;
		INT16 sY = 0;
		FindFontCenterCoordinates(PERS_CURR_TEAM_X, 0, 65, 0, s, FONT10ARIAL, &sX, &sY);
		mprintf(sX, PERS_DEPART_TEAM_Y, s);
	}
}


static INT32 GetTotalDailyCostOfCurrentTeam(void)
{
	// will return the total daily cost of the current team
	SOLDIERTYPE* pSoldier;
	INT32 cnt = 0;
	INT32 iCostOfTeam = 0;

	// first grunt
	pSoldier = MercPtrs[0];

	// not active?..return cost of zero

	// run through active soldiers
	for (pSoldier = MercPtrs[0]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++)
	{
		pSoldier = MercPtrs[cnt];

		if ((pSoldier->bActive) && (pSoldier->bLife > 0))
		{
			// valid soldier, get cost
			if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			{
				// daily rate
				if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK)
				{
					// 2 week contract
					iCostOfTeam += gMercProfiles[pSoldier->ubProfile].uiBiWeeklySalary / 14;
				}
				else if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK)
				{
					// 1 week contract
					iCostOfTeam += gMercProfiles[pSoldier->ubProfile].uiWeeklySalary / 7;
				}
				else
				{
					iCostOfTeam += gMercProfiles[pSoldier->ubProfile].sSalary;
				}
			}
			else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			{
				// MERC Merc
				iCostOfTeam += gMercProfiles[pSoldier->ubProfile].sSalary;
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


static INT32 GetLowestDailyCostOfCurrentTeam(void)
{
	// will return the lowest daily cost of the current team
	SOLDIERTYPE* pSoldier;
	INT32 cnt=0;
	INT32 iLowest = 999999;
//	INT32 iId =0;
	INT32 iCost = 0;

	// first grunt
	pSoldier = MercPtrs[0];

	// not active?..return cost of zero

	// run through active soldiers
	for (pSoldier = MercPtrs[0]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++)
	{
		pSoldier = MercPtrs[cnt];

		if ((pSoldier->bActive) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && (pSoldier->bLife > 0))
		{
			// valid soldier, get cost
			if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			{
				// daily rate
				if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK)
				{
					// 2 week contract
					iCost = gMercProfiles[pSoldier->ubProfile].uiBiWeeklySalary / 14;
				}
				else if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK)
				{
					// 1 week contract
					iCost = gMercProfiles[pSoldier->ubProfile].uiWeeklySalary / 7;
				}
				else
				{
					iCost = gMercProfiles[pSoldier->ubProfile].sSalary;
				}
			}
			else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			{
				// MERC Merc
				iCost = gMercProfiles[pSoldier->ubProfile].sSalary;
			}
			else
			{
				// no cost
				iCost = 0;
			}

			if (iCost <= iLowest)
			{
				iLowest = iCost;
			}
		}
	}

	// if no mercs, send 0
	if (iLowest == 999999)
	{
		iLowest = 0;
	}

	return iLowest;
}


static INT32 GetHighestDailyCostOfCurrentTeam(void)
{
	// will return the lowest daily cost of the current team
	SOLDIERTYPE* pSoldier;
	INT32 cnt = 0;
	INT32 iHighest = 0;
//	INT32 iId = 0;
	INT32 iCost = 0;

	// first grunt
	pSoldier = MercPtrs[0];

	// not active?..return cost of zero

	// run through active soldiers
	for (pSoldier = MercPtrs[0]; cnt <= gTacticalStatus.Team[OUR_TEAM].bLastID; cnt++)
	{
		pSoldier = MercPtrs[cnt];

		if ((pSoldier->bActive) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) && (pSoldier->bLife > 0))
		{
			// valid soldier, get cost
			if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
			{
				// daily rate
				if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK)
				{
					// 2 week contract
					iCost = gMercProfiles[pSoldier->ubProfile].uiBiWeeklySalary / 14;
				}
				else if (pSoldier->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK)
				{
					// 1 week contract
					iCost = gMercProfiles[pSoldier->ubProfile].uiWeeklySalary / 7;
				}
				else
				{
					iCost = gMercProfiles[pSoldier->ubProfile].sSalary;
				}
			}
			else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
			{
				// MERC Merc
				iCost = gMercProfiles[pSoldier->ubProfile].sSalary;
			}
			else
			{
				// no cost
				iCost = 0;
			}

			if (iCost >= iHighest)
			{
				iHighest = iCost;
			}
		}
	}
	return iHighest;
}


static void DisplayCostOfCurrentTeam(void)
{
	// display number on team
	CHAR16 sString[32];
	INT16 sX, sY;

	// font stuff
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	if (fCurrentTeamMode == TRUE)
	{
		// daily cost
		mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_COST_Y, pPersonelTeamStrings[2]);

		SPrintMoney(sString, GetTotalDailyCostOfCurrentTeam());
		FindFontRightCoordinates(PERS_CURR_TEAM_COST_X, 0, PERS_CURR_TEAM_WIDTH, 0, sString, PERS_FONT, &sX, &sY);
		mprintf(sX, PERS_CURR_TEAM_COST_Y, sString);

		// highest cost
		mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_HIGHEST_Y, pPersonelTeamStrings[3]);

		SPrintMoney(sString, GetHighestDailyCostOfCurrentTeam());
		FindFontRightCoordinates(PERS_CURR_TEAM_COST_X, 0, PERS_CURR_TEAM_WIDTH, 0, sString, PERS_FONT, &sX, &sY);

		mprintf(sX, PERS_CURR_TEAM_HIGHEST_Y, sString);

		// the lowest cost
		mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_LOWEST_Y, pPersonelTeamStrings[4]);

		SPrintMoney(sString, GetLowestDailyCostOfCurrentTeam());
		FindFontRightCoordinates(PERS_CURR_TEAM_COST_X, 0, PERS_CURR_TEAM_WIDTH, 0, sString, PERS_FONT, &sX, &sY);

		mprintf(sX, PERS_CURR_TEAM_LOWEST_Y, sString);
	}
}


static INT32 GetIdOfDepartedMercWithHighestStat(INT32 iStat)
{
	// will return the id value of the merc on the players team with highest in this stat
	// -1 means error
	INT32 iId = -1;
	INT32 iValue =0;
	MERCPROFILESTRUCT *pTeamSoldier;
	INT32 cnt=0;
	INT8 bCurrentList = 0;
	INT16* bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
	BOOLEAN fNotDone = TRUE;
	SOLDIERTYPE* pSoldier;
	UINT32 uiLoopCounter;


	// run through active soldiers
	for (uiLoopCounter = 0; fNotDone; uiLoopCounter++)
	{
		//if we are at the end of
		if (uiLoopCounter == 255 && bCurrentList == 2)
		{
			fNotDone = FALSE;
			continue;
		}

		// check if we need to move to the next list
		if (uiLoopCounter == 255)
		{
			if (bCurrentList == 0)
			{
				bCurrentList = 1;
				bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
			}
			else if (bCurrentList == 1)
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
		if (cnt == -1)
		{
			bCurrentListValue++;
			continue;
		}

		pTeamSoldier = &(gMercProfiles[cnt]);

		switch (iStat)
		{
			case 0:
				// health
				//if the soldier is a pow, dont use the health cause it aint known
				pSoldier = FindSoldierByProfileID((UINT8)cnt, FALSE);
				if (pSoldier && pSoldier->bAssignment == ASSIGNMENT_POW)
				{
					continue;
				}
				if (pTeamSoldier->bLife >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bLife;
				}
				break;

			case 1:
				// agility
				if (pTeamSoldier->bAgility >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bAgility;
				}
				break;

			case 2:
				// dexterity
				if (pTeamSoldier->bDexterity >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bDexterity;
				}
				break;

			case 3:
				// strength
				if (pTeamSoldier->bStrength >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bStrength;
				}
				break;

			case 4:
				// leadership
				if (pTeamSoldier->bLeadership >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bLeadership;
				}
				break;

			case 5:
				// wisdom
				if (pTeamSoldier->bWisdom >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bWisdom;
				}
				break;

			case 6:
				// exper
				if (pTeamSoldier->bExpLevel >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bExpLevel;
				}
				break;

			case 7:
				//mrkmanship
				if (pTeamSoldier->bMarksmanship >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bMarksmanship;
				}
				break;

			case 8:
				// mech
				if (pTeamSoldier->bMechanical >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bMechanical;
				}
				break;

			case 9:
				// exp
				if (pTeamSoldier->bExplosive >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bExplosive;
				}
				break;

			case 10:
				// med
				if (pTeamSoldier->bMedical >= iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bMedical;
				}
				break;
		}

		bCurrentListValue++;
	}

	return (iId);
}


static INT32 GetIdOfDepartedMercWithLowestStat(INT32 iStat)
{
	// will return the id value of the merc on the players team with highest in this stat
	// -1 means error
	INT32 iId = -1;
	INT32 iValue =9999999;
	MERCPROFILESTRUCT* pTeamSoldier;
	INT32 cnt=0;
	INT8 bCurrentList = 0;
	INT16* bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
	BOOLEAN fNotDone = TRUE;
	SOLDIERTYPE* pSoldier;
	UINT32 uiLoopCounter;

	// run through active soldiers
	for (uiLoopCounter = 0; fNotDone; uiLoopCounter++)
	{
		//if we are at the end of
		if (uiLoopCounter == 255 && bCurrentList == 2)
		{
			fNotDone = FALSE;
			continue;
		}

		// check if we need to move to the next list
		if (uiLoopCounter == 255)
		{
			if (bCurrentList == 0)
			{
				bCurrentList = 1;
				bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
			}
			else if (bCurrentList == 1)
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
		if (cnt == -1)
		{
			bCurrentListValue++;
			continue;
		}

		pTeamSoldier = &(gMercProfiles[cnt]);

		switch (iStat)
		{
			case 0:
				// health
				pSoldier = FindSoldierByProfileID((UINT8)cnt, FALSE);
				if (pSoldier && pSoldier->bAssignment == ASSIGNMENT_POW)
				{
					continue;
				}
				if (pTeamSoldier->bLife < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bLife;
				}
				break;

			case 1:
				// agility
				if (pTeamSoldier->bAgility < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bAgility;
				}
				break;

			case 2:
				// dexterity
				if (pTeamSoldier->bDexterity < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bDexterity;
				}
				break;

			case 3:
				// strength
				if (pTeamSoldier->bStrength < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bStrength;
				}
				break;

			case 4:
				// leadership
				if (pTeamSoldier->bLeadership < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bLeadership;
				}
				break;

			case 5:
				// wisdom
				if (pTeamSoldier->bWisdom < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bWisdom;
				}
				break;

			case 6:
				// exper
				if (pTeamSoldier->bExpLevel < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bExpLevel;
				}
				break;

			case 7:
				//mrkmanship
				if (pTeamSoldier->bMarksmanship < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bMarksmanship;
				}
				break;

			case 8:
				// mech
				if (pTeamSoldier->bMechanical < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bMechanical;
				}
				break;

			case 9:
				// exp
				if (pTeamSoldier->bExplosive < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bExplosive;
				}
				break;

			case 10:
				// med
				if (pTeamSoldier->bMedical < iValue)
				{
					iId = cnt;
					iValue = pTeamSoldier->bMedical;
				}
				break;
		}

		bCurrentListValue++;
	}

	return (iId);
}


static INT32 GetIdOfMercWithHighestStat(INT32 iStat)
{
	// will return the id value of the merc on the players team with highest in this stat
	// -1 means error
	INT32 iId = -1;
	INT32 iValue = 0;
	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt = 0;

	// first grunt
	pSoldier = MercPtrs[0];

	// run through active soldiers
	for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, pTeamSoldier++)
	{
		if ((pTeamSoldier->bActive) && !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE) && (pTeamSoldier->bLife > 0) && !AM_A_ROBOT(pTeamSoldier))
		{
			switch (iStat)
			{
				case 0:
					// health
					if (pTeamSoldier->bAssignment == ASSIGNMENT_POW)
					{
						continue;
					}
					if (pTeamSoldier->bLifeMax >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bLifeMax;
					}
					break;

				case 1:
					// agility
					if (pTeamSoldier->bAgility >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bAgility;
					}
					break;

				case 2:
					// dexterity
					if (pTeamSoldier->bDexterity >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bDexterity;
					}
					break;

				case 3:
					// strength
					if (pTeamSoldier->bStrength >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bStrength;
					}
					break;

				case 4:
					// leadership
					if (pTeamSoldier->bLeadership >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bLeadership;
					}
					break;

				case 5:
					// wisdom
					if (pTeamSoldier->bWisdom >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bWisdom;
					}
					break;

				case 6:
					// exper
					if (pTeamSoldier->bExpLevel >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bExpLevel;
					}
					break;

				case 7:
					//mrkmanship
					if (pTeamSoldier->bMarksmanship >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bMarksmanship;
					}
					break;

				case 8:
					// mech
					if (pTeamSoldier->bMechanical >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bMechanical;
					}
					break;

				case 9:
					// exp
					if (pTeamSoldier->bExplosive >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bExplosive;
					}
					break;

				case 10:
					// med
					if (pTeamSoldier->bMedical >= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bMedical;
					}
					break;
			}
		}
	}

	return (iId);
}


static INT32 GetIdOfMercWithLowestStat(INT32 iStat)
{
	// will return the id value of the merc on the players team with highest in this stat
	// -1 means error
	INT32 iId = -1;
	INT32 iValue = 999999;
	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt = 0;

	// first grunt
	pSoldier = MercPtrs[0];

	// run through active soldiers
	for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, pTeamSoldier++)
	{
		if ((pTeamSoldier->bActive) && !(pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE) && (pTeamSoldier->bLife > 0) && !AM_A_ROBOT(pTeamSoldier))
		{
			switch (iStat)
			{
				case 0:
					// health
					if (pTeamSoldier->bAssignment == ASSIGNMENT_POW)
					{
						continue;
					}
					if (pTeamSoldier->bLifeMax <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bLifeMax;
					}
					break;

				case 1:
					// agility
					if (pTeamSoldier->bAgility <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bAgility;
					}
					break;

				case 2:
					// dexterity
					if (pTeamSoldier->bDexterity <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bDexterity;
					}
					break;

				case 3:
					// strength
					if (pTeamSoldier->bStrength <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bStrength;
					}
					break;

				case 4:
					// leadership
					if (pTeamSoldier->bLeadership <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bLeadership;
					}
					break;

				case 5:
					// wisdom
					if (pTeamSoldier->bWisdom <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bWisdom;
					}
					break;

				case 6:
					// exper
					if (pTeamSoldier->bExpLevel <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bExpLevel;
					}
					break;

				case 7:
					//mrkmanship
					if (pTeamSoldier->bMarksmanship <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bMarksmanship;
					}
					break;

				case 8:
					// mech
					if (pTeamSoldier->bMechanical <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bMechanical;
					}
					break;

				case 9:
					// exp
					if (pTeamSoldier->bExplosive <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bExplosive;
					}
					break;

				case 10:
					// med
					if (pTeamSoldier->bMedical <= iValue)
					{
						iId = cnt;
						iValue = pTeamSoldier->bMedical;
					}
					break;
			}
		}
	}

	return (iId);
}


static INT32 GetAvgStatOfCurrentTeamStat(INT32 iStat)
{
	// will return the id value of the merc on the players team with highest in this stat
	// -1 means error
	SOLDIERTYPE *pTeamSoldier, *pSoldier;
	INT32 cnt = 0;
	INT32 iTotalStatValue = 0;
	INT8 bNumberOfPows = 0;
	UINT8 ubNumberOfMercsInCalculation = 0;

	// first grunt
	pSoldier = MercPtrs[0];

	// run through active soldiers
	for (pTeamSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, pTeamSoldier++)
	{
		if ((pTeamSoldier->bActive) && (pTeamSoldier->bLife > 0) && !AM_A_ROBOT(pTeamSoldier))
		{
			switch (iStat)
			{
				case 0:
					// health
					//if this is a pow, dont count his stats
					if (pTeamSoldier->bAssignment == ASSIGNMENT_POW)
					{
						bNumberOfPows++;
						continue;
					}
					iTotalStatValue += pTeamSoldier->bLifeMax;
					break;

				case 1:
					// agility
					iTotalStatValue +=pTeamSoldier->bAgility;
					break;

				case 2:
					// dexterity
					iTotalStatValue +=  pTeamSoldier->bDexterity;
					break;

				case 3:
					// strength
					iTotalStatValue +=  pTeamSoldier->bStrength;
					break;

				case 4:
					// leadership
					iTotalStatValue +=  pTeamSoldier->bLeadership;
					break;

				case 5:
					// wisdom
					iTotalStatValue += pTeamSoldier->bWisdom;
					break;

				case 6:
					// exper
					iTotalStatValue +=  pTeamSoldier->bExpLevel;
					break;

				case 7:
					//mrkmanship
					iTotalStatValue +=  pTeamSoldier->bMarksmanship;
					break;

				case 8:
					// mech
					iTotalStatValue +=  pTeamSoldier->bMechanical;
					break;

				case 9:
					// exp
					iTotalStatValue +=  pTeamSoldier->bExplosive;
					break;

				case 10:
					// med
					iTotalStatValue +=  pTeamSoldier->bMedical;
					break;
			}

			ubNumberOfMercsInCalculation++;
		}
	}

	//if the stat is health, and there are only pow's
	if (GetNumberOfMercsOnPlayersTeam() != 0 && GetNumberOfMercsOnPlayersTeam() == bNumberOfPows && iStat == 0)
	{
		return (-1);
	}
	else if ((ubNumberOfMercsInCalculation - bNumberOfPows) > 0)
	{
		return (iTotalStatValue / (ubNumberOfMercsInCalculation - bNumberOfPows));
	}
	else
	{
		return (0);
	}
}


// get avg for this stat
static INT32 GetAvgStatOfPastTeamStat(INT32 iStat)
{
	// will return the id value of the merc on the players team with highest in this stat
	// -1 means error
	INT32 cnt=0;
	INT32 iTotalStatValue = 0;
	MERCPROFILESTRUCT* pTeamSoldier;
	INT8 bCurrentList = 0;
	INT16* bCurrentListValue = LaptopSaveInfo.ubDeadCharactersList;
	BOOLEAN fNotDone = TRUE;
	UINT32 uiLoopCounter;

	// run through active soldiers
	for (uiLoopCounter = 0; fNotDone; uiLoopCounter++)
	{
		//if we are at the end of
		if (uiLoopCounter == 255 && bCurrentList == 2)
		{
			fNotDone = FALSE;
			continue;
		}

		// check if we need to move to the next list
		if (uiLoopCounter == 255)
		{
			if (bCurrentList == 0)
			{
				bCurrentList = 1;
				bCurrentListValue = LaptopSaveInfo.ubLeftCharactersList;
			}
			else if (bCurrentList == 1)
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
		if (cnt == -1)
		{
			bCurrentListValue++;
			continue;
		}

		pTeamSoldier = &(gMercProfiles[cnt]);

		switch (iStat)
		{
			case 0:
				// health
				iTotalStatValue += pTeamSoldier->bLife;
				break;

			case 1:
				// agility
				iTotalStatValue += pTeamSoldier->bAgility;
				break;

			case 2:
				// dexterity
				iTotalStatValue += pTeamSoldier->bDexterity;
				break;

			case 3:
				// strength
				iTotalStatValue += pTeamSoldier->bStrength;
				break;

			case 4:
				// leadership
				iTotalStatValue += pTeamSoldier->bLeadership;
				break;

			case 5:
				// wisdom
				iTotalStatValue += pTeamSoldier->bWisdom;
				break;

			case 6:
				// exper
				iTotalStatValue += pTeamSoldier->bExpLevel;
				break;

			case 7:
				//mrkmanship
				iTotalStatValue += pTeamSoldier->bMarksmanship;
				break;

			case 8:
				// mech
				iTotalStatValue += pTeamSoldier->bMechanical;
				break;

			case 9:
				// exp
				iTotalStatValue += pTeamSoldier->bExplosive;
				break;

			case 10:
				// med
				iTotalStatValue += pTeamSoldier->bMedical;
				break;
		}

		bCurrentListValue++;
	}

	if (GetNumberOfPastMercsOnPlayersTeam() > 0)
	{
		return (iTotalStatValue / GetNumberOfPastMercsOnPlayersTeam());
	}
	else
	{
		return (0);
	}
}


static void DisplayAverageStatValuesForCurrentTeam(void)
{
	// will display the average values for stats for the current team
	INT16 sX, sY;
	INT32 iCounter = 0;
	CHAR16 sString[32];

	// set up font
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	// display header

	// center
	FindFontCenterCoordinates(PERS_STAT_AVG_X, 0, PERS_STAT_AVG_WIDTH, 0, pPersonnelCurrentTeamStatsStrings[1], FONT10ARIAL, &sX, &sY);

	mprintf(sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[1]);

	// nobody on team leave
	if ((GetNumberOfMercsDeadOrAliveOnPlayersTeam() == 0) && (fCurrentTeamMode == TRUE))
	{
		return;
	}

	// check if in past team and nobody on past team
	if ((GetNumberOfPastMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == FALSE))
	{
		return;
	}


	for (iCounter = 0; iCounter < 11; iCounter++)
	{
		// even or odd?..color black or yellow?
		if (iCounter % 2 == 0)
		{
			SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
		}
		else
		{
			SetFontForeground(PERS_TEXT_FONT_COLOR);
		}

		if (fCurrentTeamMode == TRUE)
		{
			INT32 iValue = GetAvgStatOfCurrentTeamStat(iCounter);

			//if there are no values
			if (iValue == -1)
				wcslcpy(sString, pPOWStrings[1], lengthof(sString));
			else
				swprintf(sString, lengthof(sString), L"%d", iValue);

		}
		else
		{
			swprintf(sString, lengthof(sString), L"%d", GetAvgStatOfPastTeamStat(iCounter));
		}
		// center
		FindFontCenterCoordinates(PERS_STAT_AVG_X, 0, PERS_STAT_AVG_WIDTH, 0, sString, FONT10ARIAL, &sX, &sY);

		mprintf(sX, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), sString);
	}
}


static void DisplayLowestStatValuesForCurrentTeam(void)
{
	// will display the average values for stats for the current team
	INT16 sX, sY;
	INT32 iCounter = 0;
	INT32 iStat = 0;
	INT32 iDepartedId = 0;
	INT32 iId = 0;

	// set up font
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	// display header

	// center
	FindFontCenterCoordinates(PERS_STAT_LOWEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0, pPersonnelCurrentTeamStatsStrings[0], FONT10ARIAL, &sX, &sY);

	mprintf(sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[0]);

	// nobody on team leave
	if ((GetNumberOfMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == TRUE))
	{
		return;
	}

	if ((GetNumberOfPastMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == FALSE))
	{
		return;
	}

	for (iCounter = 0; iCounter < 11; iCounter++)
	{
		if (fCurrentTeamMode == TRUE)
		{
			iId = GetIdOfMercWithLowestStat(iCounter);
		}
		else
		{
			iDepartedId = GetIdOfDepartedMercWithLowestStat(iCounter);
			if (iDepartedId == -1)
				continue;
		}

		// even or odd?..color black or yellow?
		if (iCounter % 2 == 0)
		{
			SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
		}
		else
		{
			SetFontForeground(PERS_TEXT_FONT_COLOR);
		}

		const wchar_t* Name;
		if (fCurrentTeamMode == TRUE)
		{
			Name = (iId == -1 ? pPOWStrings[1] : MercPtrs[iId]->name);
		}
		else
		{
			Name = gMercProfiles[iDepartedId].zNickname;
		}
		// print name
		mprintf(PERS_STAT_LOWEST_X, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), Name);

		switch (iCounter)
		{
			case 0:
				// health
				if (fCurrentTeamMode == TRUE)
				{
					if (iId == -1)
						iStat = -1;
					else
						iStat = MercPtrs[iId]->bLifeMax;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bLife;
				}
				break;

			case 1:
				// agility
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bAgility;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bAgility;
				}
				break;

			case 2:
				// dexterity
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bDexterity;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bDexterity;
				}

				break;

			case 3:
				// strength
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bStrength;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bStrength;
				}

				break;

			case 4:
				// leadership
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bLeadership;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bLeadership;
				}
				break;

			case 5:
				// wisdom
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bWisdom;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bWisdom;
				}
				break;

			case 6:
				// exper
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bExpLevel;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bExpLevel;
				}
				break;

			case 7:
				//mrkmanship
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bMarksmanship;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bMarksmanship;
				}
				break;

			case 8:
				// mech
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bMechanical;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bMechanical;
				}
				break;

			case 9:
				// exp
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bExplosive;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bExplosive;
				}
				break;

			case 10:
				// med
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bMedical;
				}
				else
				{
					iStat = gMercProfiles[iDepartedId].bMedical;
				}
				break;
		}

		CHAR16 sString[32];
		if (iStat == -1)
			wcslcpy(sString, pPOWStrings[1], lengthof(sString));
		else
			swprintf(sString, lengthof(sString), L"%d", iStat);

		// right justify
		FindFontRightCoordinates(PERS_STAT_LOWEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0, sString, FONT10ARIAL, &sX, &sY);

		mprintf(sX, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), sString);
	}
}


static void DisplayHighestStatValuesForCurrentTeam(void)
{
	// will display the average values for stats for the current team
	INT16 sX, sY;
	INT32 iCounter = 0;
	INT32 iStat = 0;
	INT32 iId = 0;

	// set up font
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	// display header

	// center
	FindFontCenterCoordinates(PERS_STAT_HIGHEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0, pPersonnelCurrentTeamStatsStrings[2], FONT10ARIAL, &sX, &sY);

	mprintf(sX, PERS_STAT_AVG_Y, pPersonnelCurrentTeamStatsStrings[2]);

	// nobody on team leave
	if ((GetNumberOfMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == TRUE))
	{
		return;
	}

	if ((GetNumberOfPastMercsOnPlayersTeam() == 0) && (fCurrentTeamMode == FALSE))
	{
		return;
	}

	for (iCounter = 0; iCounter < 11; iCounter++)
	{
		if (fCurrentTeamMode == TRUE)
			iId = GetIdOfMercWithHighestStat(iCounter);
		else
			iId = GetIdOfDepartedMercWithHighestStat(iCounter);

		// even or odd?..color black or yellow?
		if (iCounter % 2 == 0)
		{
			SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
		}
		else
		{
			SetFontForeground(PERS_TEXT_FONT_COLOR);
		}

		const wchar_t* Name;
		if (fCurrentTeamMode == TRUE)
		{
			Name = (iId == -1 ? pPOWStrings[1] : MercPtrs[iId]->name);
		}
		else
		{
		// get name
			Name = gMercProfiles[iId].zNickname;
		}
		// print name
		mprintf(PERS_STAT_HIGHEST_X, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), Name);

		switch (iCounter)
		{
			case 0:
				// health
				if (fCurrentTeamMode == TRUE)
				{
					if (iId == -1)
						iStat = -1;
					else
						iStat = MercPtrs[iId]->bLifeMax;
				}
				else
				{
					iStat = gMercProfiles[iId].bLife;
				}
				break;

			case 1:
				// agility
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bAgility;
				}
				else
				{
					iStat = gMercProfiles[iId].bAgility;
				}
				break;

			case 2:
				// dexterity
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bDexterity;
				}
				else
				{
					iStat = gMercProfiles[iId].bDexterity;
				}
				break;

			case 3:
				// strength
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bStrength;
				}
				else
				{
					iStat = gMercProfiles[iId].bStrength;
				}
				break;

			case 4:
				// leadership
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bLeadership;
				}
				else
				{
					iStat = gMercProfiles[iId].bLeadership;
				}
				break;

			case 5:
				// wisdom
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bWisdom;
				}
				else
				{
					iStat = gMercProfiles[iId].bWisdom;
				}
				break;

			case 6:
				// exper
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bExpLevel;
				}
				else
				{
					iStat = gMercProfiles[iId].bExpLevel;
				}
				break;

			case 7:
				//mrkmanship
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bMarksmanship;
				}
				else
				{
					iStat = gMercProfiles[iId].bMarksmanship;
				}
				break;

			case 8:
				// mech
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bMechanical;
				}
				else
				{
					iStat = gMercProfiles[iId].bMechanical;
				}
				break;

			case 9:
				// exp
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bExplosive;
				}
				else
				{
					iStat = gMercProfiles[iId].bExplosive;
				}
				break;

			case 10:
				// med
				if (fCurrentTeamMode == TRUE)
				{
					iStat = MercPtrs[iId]->bMedical;
				}
				else
				{
					iStat = gMercProfiles[iId].bMedical;
				}
				break;
		}

		CHAR16 sString[32];
		if (iStat == -1)
			wcslcpy(sString, pPOWStrings[1], lengthof(sString));
		else
			swprintf(sString, lengthof(sString), L"%d", iStat);

		// right justify
		FindFontRightCoordinates(PERS_STAT_HIGHEST_X, 0, PERS_STAT_LOWEST_WIDTH, 0, sString, FONT10ARIAL, &sX, &sY);

		mprintf(sX, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), sString);
	}
}


static void DisplayPersonnelTeamStats(void)
{
	// displays the stat title for each row in the team stat list
	INT32 iCounter =0;

	// set up font
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(FONT_WHITE);

	// display titles for each row
	for (iCounter = 0; iCounter < 11; iCounter++)
	{
		// even or odd?..color black or yellow?
		if (iCounter % 2 == 0)
		{
			SetFontForeground(PERS_TEXT_FONT_ALTERNATE_COLOR);
		}
		else
		{
			SetFontForeground(PERS_TEXT_FONT_COLOR);
		}

		mprintf(PERS_STAT_LIST_X, PERS_STAT_AVG_Y + (iCounter + 1) * (GetFontHeight(FONT10ARIAL) + 3), pPersonnelTeamStatsStrings[iCounter]);
	}
}


static INT32 GetNumberOfPastMercsOnPlayersTeam(void)
{
	INT32 iPastNumberOfMercs = 0;
	// will run through the alist of past mercs on the players team and return thier number

	// dead
	iPastNumberOfMercs += GetNumberOfDeadOnPastTeam();

	// left
	iPastNumberOfMercs += GetNumberOfLeftOnPastTeam();

	// other
	iPastNumberOfMercs += GetNumberOfOtherOnPastTeam();

	return iPastNumberOfMercs;
}


static void InitPastCharactersList(void)
{
	// inits the past characters list
	memset(&LaptopSaveInfo.ubDeadCharactersList,  -1, sizeof(LaptopSaveInfo.ubDeadCharactersList));
	memset(&LaptopSaveInfo.ubLeftCharactersList,  -1, sizeof(LaptopSaveInfo.ubLeftCharactersList));
	memset(&LaptopSaveInfo.ubOtherCharactersList, -1, sizeof(LaptopSaveInfo.ubOtherCharactersList));
}


static INT32 GetNumberOfDeadOnPastTeam(void)
{
	INT32 iNumberDead = 0;
	INT32 iCounter = 0;

//	for (iCounter = 0; ((iCounter < 256) && (LaptopSaveInfo.ubDeadCharactersList[iCounter] != -1)); iCounter ++)
	for (iCounter = 0; iCounter < 256; iCounter ++)
	{
		if (LaptopSaveInfo.ubDeadCharactersList[iCounter] != -1)
			iNumberDead++;
	}

	return (iNumberDead);
}


static INT32 GetNumberOfLeftOnPastTeam(void)
{
	INT32 iNumberLeft = 0;
	INT32 iCounter = 0;

	for (iCounter = 0; iCounter < 256; iCounter ++)
	{
		if (LaptopSaveInfo.ubLeftCharactersList[iCounter] != -1)
			iNumberLeft++;
	}

	return (iNumberLeft);
}


static INT32 GetNumberOfOtherOnPastTeam(void)
{
	INT32 iNumberOther = 0;
	INT32 iCounter = 0;

	for (iCounter = 0; iCounter < 256; iCounter ++)
	{
		if (LaptopSaveInfo.ubOtherCharactersList[iCounter] != -1)
			iNumberOther++;
	}

	return (iNumberOther);
}


static void DisplayStateOfPastTeamMembers(void)
{
	INT16 sX, sY;
	CHAR16 sString[32];

	// font stuff
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	// diplsya numbers fired, dead and othered
	if (fCurrentTeamMode == FALSE)
	{
		// dead
		mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_COST_Y, pPersonelTeamStrings[5]);
		swprintf(sString, lengthof(sString), L"%d", GetNumberOfDeadOnPastTeam());

		FindFontRightCoordinates(PERS_CURR_TEAM_COST_X, 0, PERS_DEPART_TEAM_WIDTH, 0, sString, PERS_FONT, &sX, &sY);

		mprintf(sX, PERS_CURR_TEAM_COST_Y, sString);

		// fired
		mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_HIGHEST_Y, pPersonelTeamStrings[6]);
		swprintf(sString, lengthof(sString), L"%d", GetNumberOfLeftOnPastTeam());

		FindFontRightCoordinates(PERS_CURR_TEAM_COST_X, 0, PERS_DEPART_TEAM_WIDTH, 0, sString, PERS_FONT, &sX, &sY);

		mprintf(sX, PERS_CURR_TEAM_HIGHEST_Y, sString);

		// other
		mprintf(PERS_CURR_TEAM_COST_X, PERS_CURR_TEAM_LOWEST_Y, pPersonelTeamStrings[7]);
		swprintf(sString, lengthof(sString), L"%d", GetNumberOfOtherOnPastTeam());

		FindFontRightCoordinates(PERS_CURR_TEAM_COST_X, 0, PERS_DEPART_TEAM_WIDTH, 0, sString, PERS_FONT, &sX, &sY);

		mprintf(sX, PERS_CURR_TEAM_LOWEST_Y, sString);
	}
	else
	{
		// do nothing
	}
}


static void PersonnelCurrentTeamCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void PersonnelDepartedTeamCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyCurrentDepartedMouseRegions(BOOLEAN create)
{
	static BOOLEAN fCreated = FALSE;

	// will arbitrate the creation/deletion of mouse regions for current/past team toggles

	if (create && !fCreated)
	{
		// not created, create
		UINT16 tlx = PERS_TOGGLE_CUR_DEPART_X;
		UINT16 tly = PERS_TOGGLE_CUR_Y;
		UINT16 brx = tlx + PERS_TOGGLE_CUR_DEPART_WIDTH;
		UINT16 bry = tly + PERS_TOGGLE_CUR_DEPART_HEIGHT;
		MSYS_DefineRegion(&gTogglePastCurrentTeam[0], tlx, tly, brx, bry, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelCurrentTeamCallback);

		tly = PERS_TOGGLE_DEPART_Y;
		bry = tly + PERS_TOGGLE_CUR_DEPART_HEIGHT;
		MSYS_DefineRegion(&gTogglePastCurrentTeam[1], tlx, tly, brx, bry, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, PersonnelDepartedTeamCallback);

		fCreated = TRUE;
	}
	else if (!create && fCreated)
	{
		// created, get rid of

		MSYS_RemoveRegion(&gTogglePastCurrentTeam[0]);
		MSYS_RemoveRegion(&gTogglePastCurrentTeam[1]);
		fCreated = FALSE;
	}
}


static void PersonnelCurrentTeamCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fCurrentTeamMode = TRUE;

		if (fCurrentTeamMode == TRUE)
		{
			iCurrentPersonSelectedId = -1;

			// how many people do we have?..if you have someone set default to 0
			if (GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0)
			{
				// get id of first merc in list
				iCurrentPersonSelectedId = GetIdOfFirstDisplayedMerc();
			}
		}

		fCurrentTeamMode = TRUE;
		fReDrawScreenFlag = TRUE;
	}
}


static void PersonnelDepartedTeamCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fCurrentTeamMode = FALSE;

		if (fCurrentTeamMode == FALSE)
		{
			iCurrentPersonSelectedId = -1;

			// how many departed people?
			if (GetNumberOfPastMercsOnPlayersTeam() > 0)
			{
				iCurrentPersonSelectedId = 0;
			}

			//Switch the panel on the right to be the stat panel
			gubPersonnelInfoState = PERSONNEL_STAT_BTN;
		}

		fReDrawScreenFlag = TRUE;
	}
}


static void DepartedDownCallBack(GUI_BUTTON* btn, INT32 reason);
static void DepartedUpCallBack(GUI_BUTTON* btn, INT32 reason);


static void CreateDestroyButtonsForDepartedTeamList(void)
{
	// creates/ destroys the buttons for cdeparted team list
	static BOOLEAN fCreated = FALSE;

	if ((fCurrentTeamMode == FALSE) && (fCreated == FALSE))
	{
		// not created. create
		giPersonnelButtonImage[4]=  LoadButtonImage("LAPTOP/departuresbuttons.sti", -1, 0, -1, 2, -1);
		giPersonnelButton[4] = QuickCreateButton(giPersonnelButtonImage[4], PERS_DEPARTED_UP_X, PERS_DEPARTED_UP_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, DepartedUpCallBack);

		// right button
		giPersonnelButtonImage[5]=  LoadButtonImage("LAPTOP/departuresbuttons.sti", -1, 1, -1, 3, -1);
		giPersonnelButton[5] = QuickCreateButton(giPersonnelButtonImage[5], PERS_DEPARTED_UP_X, PERS_DEPARTED_DOWN_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, BtnGenericMouseMoveButtonCallback, DepartedDownCallBack);

		// set up cursors for these buttons
		SetButtonCursor(giPersonnelButton[4], CURSOR_LAPTOP_SCREEN);
		SetButtonCursor(giPersonnelButton[5], CURSOR_LAPTOP_SCREEN);

		fCreated = TRUE;
	}
	else if ((fCurrentTeamMode == TRUE) && (fCreated == TRUE))
	{
		// created. destroy
		RemoveButton(giPersonnelButton[4]);
		UnloadButtonImage(giPersonnelButtonImage[4]);
		RemoveButton(giPersonnelButton[5]);
		UnloadButtonImage(giPersonnelButtonImage[5]);
		fCreated = FALSE;
		fReDrawScreenFlag = TRUE;
	}
}


static void DepartedUpCallBack(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (giCurrentUpperLeftPortraitNumber - 20 >= 0)
		{
			giCurrentUpperLeftPortraitNumber -= 20;
			fReDrawScreenFlag = TRUE;
		}
	}
}


static void DepartedDownCallBack(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (giCurrentUpperLeftPortraitNumber + 20 < GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam())
		{
			giCurrentUpperLeftPortraitNumber += 20;
			fReDrawScreenFlag = TRUE;
		}
	}
}


static BOOLEAN DisplayPortraitOfPastMerc(INT32 iId, INT32 iCounter, BOOLEAN fDead, BOOLEAN fFired, BOOLEAN fOther);


static void DisplayPastMercsPortraits(void)
{
	// display past mercs portraits, starting at giCurrentUpperLeftPortraitNumber and going up 20 mercs
	// start at dead mercs, then fired, then other

	INT32 iCounter = 0;
	INT32 iCounterA = 0;
	INT32 iStartArray = 0; // 0 = dead list, 1 = fired list, 2 = other list

	// not time to display
	if (fCurrentTeamMode == TRUE)
	{
		return;
	}

	// go through dead list
	for (iCounterA = 0; (iCounter < giCurrentUpperLeftPortraitNumber); iCounterA++)
	{
		if (LaptopSaveInfo.ubDeadCharactersList[iCounterA] != -1)
			iCounter++;
	}

	if (iCounter < giCurrentUpperLeftPortraitNumber)
	{
		// now the fired list
		for (iCounterA = 0; ((iCounter < giCurrentUpperLeftPortraitNumber)); iCounterA++)
		{
			if (LaptopSaveInfo.ubLeftCharactersList[iCounterA] != -1)
			{
				iCounter++;
			}
		}

		if (iCounter < 20)
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

	if ((iCounter < giCurrentUpperLeftPortraitNumber) && (iStartArray != 0))
	{
		// now the fired list
		for (iCounterA = 0; (iCounter < giCurrentUpperLeftPortraitNumber); iCounterA++)
		{
			if (LaptopSaveInfo.ubOtherCharactersList[iCounterA] != -1)
				iCounter++;
		}

		if (iCounter < 20)
		{
			iStartArray = 1;
		}
		else
		{
			iStartArray = 2;
		}
	}
	else if (iStartArray != 0)
	{
		iStartArray = 1;
	}

	//; we now have the array to start in, the position
	iCounter = 0;

	if (iStartArray == 0)
	{
		// run through list and display
		for (iCounterA; iCounter < 20 && iCounterA < 256; iCounterA++)
		{
			// show dead pictures
			if (LaptopSaveInfo.ubDeadCharactersList[iCounterA] != -1)
			{
				DisplayPortraitOfPastMerc(LaptopSaveInfo.ubDeadCharactersList[iCounterA], iCounter, TRUE, FALSE, FALSE);
				iCounter++;
			}
		}

		// reset counter A for the next array, if applicable
		iCounterA = 0;
	}
	if (iStartArray <= 1)
	{
		for (iCounterA; (iCounter < 20  && iCounterA < 256); iCounterA++)
		{
			// show fired pics
			if (LaptopSaveInfo.ubLeftCharactersList[iCounterA] != -1)
			{
				DisplayPortraitOfPastMerc(LaptopSaveInfo.ubLeftCharactersList[iCounterA], iCounter, FALSE, TRUE, FALSE);
				iCounter++;
			}
		}
		// reset counter A for the next array, if applicable
		iCounterA = 0;
	}

	if (iStartArray <= 2)
	{
		for (iCounterA; (iCounter < 20  && iCounterA < 256); iCounterA++)
		{
			// show other pics
			if (LaptopSaveInfo.ubOtherCharactersList[iCounterA] != -1)
			{
				DisplayPortraitOfPastMerc(LaptopSaveInfo.ubOtherCharactersList[iCounterA], iCounter, FALSE, FALSE, TRUE);
				iCounter++;
			}
		}
		// reset counter A for the next array, if applicable
		iCounterA = 0;
	}
}


static INT32 GetIdOfPastMercInSlot(INT32 iSlot)
{
	INT32 iCounter =-1;
	INT32 iCounterA = 0;
	// returns ID of Merc in this slot

	// not time to display
	if (fCurrentTeamMode == TRUE)
	{
		return -1;
	}

	if (iSlot > ((GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) - giCurrentUpperLeftPortraitNumber))
	{
		// invalid slot
		return iCurrentPersonSelectedId;
	}
	// go through dead list
	for (iCounterA = 0; ((iCounter) < iSlot + giCurrentUpperLeftPortraitNumber);  iCounterA++)
	{
		if (LaptopSaveInfo.ubDeadCharactersList[iCounterA] != -1)
			iCounter++;
	}

	if (iSlot + giCurrentUpperLeftPortraitNumber == iCounter)
	{
		return (LaptopSaveInfo.ubDeadCharactersList[iCounterA - 1]);
	}

	// now the fired list
	iCounterA = 0;
	for (iCounterA = 0; (((iCounter) < iSlot + giCurrentUpperLeftPortraitNumber)); iCounterA++)
	{
		if (LaptopSaveInfo.ubLeftCharactersList[iCounterA] != -1)
			iCounter++;
	}

	if (iSlot + giCurrentUpperLeftPortraitNumber == iCounter)
	{
		return (LaptopSaveInfo.ubLeftCharactersList[iCounterA  - 1]);
	}

	// now the fired list
	iCounterA =0;
	for (iCounterA = 0; (((iCounter) < (iSlot + giCurrentUpperLeftPortraitNumber))); iCounterA++)
	{
		if (LaptopSaveInfo.ubOtherCharactersList[iCounterA] != -1)
			iCounter++;
	}

	return (LaptopSaveInfo.ubOtherCharactersList[iCounterA  - 1]);
}


static BOOLEAN DisplayPortraitOfPastMerc(INT32 iId, INT32 iCounter, BOOLEAN fDead, BOOLEAN fFired, BOOLEAN fOther)
{
	char sTemp[100];
	if ((50 < iId) && (57 > iId))
	{
		sprintf(sTemp, "%s%03d.sti", SMALL_FACES_DIR, gMercProfiles[iId].ubFaceIndex);
	}
	else
	{
		sprintf(sTemp, "%s%02d.sti", SMALL_FACES_DIR, iId);
	}

	UINT32 guiFACE = AddVideoObjectFromFile(sTemp);
	CHECKF(guiFACE != NO_VOBJECT);

	HVOBJECT hFaceHandle = GetVideoObject(guiFACE);

	if (fDead)
	{
		hFaceHandle->pShades[0] = Create16BPPPaletteShaded(hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);
		//set the red pallete to the face
		SetObjectHandleShade(guiFACE, 0);
	}

	BltVideoObject(FRAME_BUFFER, hFaceHandle, 0, (INT16)(SMALL_PORTRAIT_START_X + (iCounter % PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_WIDTH), (INT16)(SMALL_PORTRAIT_START_Y + (iCounter / PERSONNEL_PORTRAIT_NUMBER_WIDTH) * SMALL_PORT_HEIGHT));

	DeleteVideoObjectFromIndex(guiFACE);

	return (TRUE);
}


static void DisplayDepartedCharStats(INT32 iId, INT32 iState)
{
	wchar_t sString[50];
	INT16 sX, sY;
	UINT32 uiHits = 0;

	// font stuff
	SetFont(FONT10ARIAL);
	SetFontBackground(FONT_BLACK);
	SetFontForeground(PERS_TEXT_FONT_COLOR);

	// display the stats for a char
	for (INT32 i = 0; i < MAX_STATS; i++)
	{
		switch (i)
		{
			case 0:
				// health
				// dead?
				if (iState == 0)
				{
					swprintf(sString, lengthof(sString), L"%d/%d", 0, gMercProfiles[iId].bLife);
				}
				else
				{
					swprintf(sString, lengthof(sString), L"%d/%d", gMercProfiles[iId].bLife, gMercProfiles[iId].bLife);
				}
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 1:
				// agility
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bAgility);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 2:
				// dexterity
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bDexterity);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 3:
				// strength
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bStrength);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 4:
				// leadership
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bLeadership);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 5:
				// wisdom
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bWisdom);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 6:
				// exper
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bExpLevel);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 7:
				//mrkmanship
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bMarksmanship);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 8:
				// mech
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bMechanical);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 9:
				// exp
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bExplosive);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 10:
				// med
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[i]);
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].bMedical);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 14:
				// kills
				mprintf(pers_stat_x, pers_stat_y[21], pPersonnelScreenStrings[PRSNL_TXT_KILLS]);
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].usKills);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[21], sString);
				break;

			case 15:
				// assists
				mprintf(pers_stat_x, pers_stat_y[22], pPersonnelScreenStrings[PRSNL_TXT_ASSISTS]);
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].usAssists);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[22], sString);
				break;

			case 16:
				// shots/hits
				mprintf(pers_stat_x, pers_stat_y[23], pPersonnelScreenStrings[PRSNL_TXT_HIT_PERCENTAGE]);
				uiHits = (UINT32)gMercProfiles[iId].usShotsHit;
				uiHits *= 100;
				// check we have shot at least once
				if (gMercProfiles[iId].usShotsFired > 0)
				{
					uiHits /= (UINT32)gMercProfiles[iId].usShotsFired;
				}
				else
				{
					// no, set hit % to 0
					uiHits = 0;
				}
				swprintf(sString, lengthof(sString), L"%d %%", uiHits);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[23], L"%ls", sString);
				break;

			case 17:
				// battles
				mprintf(pers_stat_x, pers_stat_y[24], pPersonnelScreenStrings[PRSNL_TXT_BATTLES]);
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].usBattlesFought);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[24], sString);
				break;

			case 18:
				// wounds
				mprintf(pers_stat_x, pers_stat_y[25], pPersonnelScreenStrings[PRSNL_TXT_TIMES_WOUNDED]);
				swprintf(sString, lengthof(sString), L"%d", gMercProfiles[iId].usTimesWounded);
				FindFontRightCoordinates(pers_stat_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[25], sString);
				break;
		}
	}
}


static void EnableDisableDeparturesButtons(void)
{
	// will enable or disable departures buttons based on upperleft picutre index value
	if ((fCurrentTeamMode == TRUE) || (fNewMailFlag == TRUE))
	{
		return;
	}

	// disable both buttons
	DisableButton(giPersonnelButton[4]);
	DisableButton(giPersonnelButton[5]);


	if (giCurrentUpperLeftPortraitNumber != 0)
	{
		// enable up button
		EnableButton(giPersonnelButton[4]);
	}
	if ((GetNumberOfDeadOnPastTeam() + GetNumberOfLeftOnPastTeam() + GetNumberOfOtherOnPastTeam()) - giCurrentUpperLeftPortraitNumber  >= 20)
	{
		// enable down button
		EnableButton(giPersonnelButton[5]);
	}
}


static void DisplayDepartedCharName(INT32 iId, INT32 iState)
{
	// get merc's nickName, assignment, and sector location info
	INT16 sX, sY;

	SetFont(CHAR_NAME_FONT);
	SetFontForeground(PERS_TEXT_FONT_COLOR);
	SetFontBackground(FONT_BLACK);

	if (iState == -1 || iId == -1)
	{
		return;
	}

	const wchar_t* Name = gMercProfiles[iId].zNickname;
	FindFontCenterCoordinates(CHAR_NAME_LOC_X, 0, CHAR_NAME_LOC_WIDTH, 0, Name, CHAR_NAME_FONT, &sX, &sY);
	mprintf(sX, CHAR_NAME_Y, Name);

	const wchar_t* State;
	if (gMercProfiles[iId].ubMiscFlags2 & PROFILE_MISC_FLAG2_MARRIED_TO_HICKS)
	{
		//displaye 'married'
		State = pPersonnelDepartedStateStrings[DEPARTED_MARRIED];
	}
	else if (iState == DEPARTED_DEAD)
	{
		State = pPersonnelDepartedStateStrings[DEPARTED_DEAD];
	}
	else if (iId < BIFF) // if the merc is an AIM merc
	{
		//if dismissed
		if (iState == DEPARTED_FIRED)
			State = pPersonnelDepartedStateStrings[DEPARTED_FIRED];
		else
			State = pPersonnelDepartedStateStrings[DEPARTED_CONTRACT_EXPIRED];
	}

	//else if its a MERC merc
	else if (iId >= BIFF && iId <= BUBBA)
	{
		if (iState == DEPARTED_FIRED)
			State = pPersonnelDepartedStateStrings[DEPARTED_FIRED];
		else
			State = pPersonnelDepartedStateStrings[DEPARTED_QUIT];
	}
	//must be a RPC
	else
	{
		if (iState == DEPARTED_FIRED)
			State = pPersonnelDepartedStateStrings[DEPARTED_FIRED];
		else
			State = pPersonnelDepartedStateStrings[DEPARTED_QUIT];
	}

	FindFontCenterCoordinates(CHAR_NAME_LOC_X, 0, CHAR_NAME_LOC_WIDTH, 0, State, CHAR_NAME_FONT, &sX, &sY);
	mprintf(sX, CHAR_LOC_Y, State);
}


static INT32 GetTheStateOfDepartedMerc(INT32 iId)
{
	// will run through each list until merc is found, if not a -1 is returned
	for (INT32 i = 0; i < 256; i++)
	{
		if (LaptopSaveInfo.ubDeadCharactersList[i] == iId) return DEPARTED_DEAD;
	}

	for (INT32 i = 0; i < 256; i++)
	{
		if (LaptopSaveInfo.ubLeftCharactersList[i] == iId) return DEPARTED_FIRED;
	}

	for (INT32 i = 0; i < 256; i++)
	{
		if (LaptopSaveInfo.ubOtherCharactersList[i] == iId) return DEPARTED_OTHER;
	}

	return -1;
}


static void DisplayPersonnelTextOnTitleBar(void)
{
	SetFont(FONT14ARIAL);
	SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);
	mprintf(PERS_TITLE_X, PERS_TITLE_Y, pPersTitleText[0]);
}


// display box around currently selected merc
static BOOLEAN DisplayHighLightBox(void)
{
	// will display highlight box around selected merc

	// is the current selected face valid?
	if (iCurrentPersonSelectedId == -1)
	{
		// no, leave
		return FALSE;
	}

	UINT32 uiBox = AddVideoObjectFromFile("LAPTOP/PicBorde.sti");
	CHECKF(uiBox != NO_VOBJECT);
	BltVideoObjectFromIndex(FRAME_BUFFER, uiBox, 0, SMALL_PORTRAIT_START_X + iCurrentPersonSelectedId % PERSONNEL_PORTRAIT_NUMBER_WIDTH * SMALL_PORT_WIDTH - 2, SMALL_PORTRAIT_START_Y + iCurrentPersonSelectedId / PERSONNEL_PORTRAIT_NUMBER_WIDTH * SMALL_PORT_HEIGHT - 3);
	DeleteVideoObjectFromIndex(uiBox);

	return TRUE;
}


// add to dead list
void AddCharacterToDeadList(SOLDIERTYPE *pSoldier)
{
	for (INT32 i = 0; i < 256; i++)
	{
		if (LaptopSaveInfo.ubDeadCharactersList[i] == -1)
		{
			// valid slot, merc not found yet, inset here
			LaptopSaveInfo.ubDeadCharactersList[i] = pSoldier->ubProfile;
			return;
		}

		// are they already in the list?
		if (LaptopSaveInfo.ubDeadCharactersList[i] == pSoldier->ubProfile)
		{
			return;
		}
	}
}


void AddCharacterToFiredList(SOLDIERTYPE *pSoldier)
{
	for (INT32 i = 0; i < 256; i++)
	{
		if (LaptopSaveInfo.ubLeftCharactersList[i] == -1)
		{
			// valid slot, merc not found yet, inset here
			LaptopSaveInfo.ubLeftCharactersList[i] = pSoldier->ubProfile;
			return;
		}

		// are they already in the list?
		if (LaptopSaveInfo.ubLeftCharactersList[i] == pSoldier->ubProfile)
		{
			return;
		}
	}
}


void AddCharacterToOtherList(SOLDIERTYPE *pSoldier)
{
	for (INT32 i = 0; i < 256; i++)
	{
		if (LaptopSaveInfo.ubOtherCharactersList[i] == -1)
		{
			// valid slot, merc not found yet, inset here
			LaptopSaveInfo.ubOtherCharactersList[i] = pSoldier->ubProfile;
			return;
		}

		// are they already in the list?
		if (LaptopSaveInfo.ubOtherCharactersList[i] == pSoldier->ubProfile)
		{
			return;
		}
	}
}


// If you have hired a merc before, then the they left for whatever reason, and now you are hiring them again,
// we must get rid of them from the departed section in the personnel screen.  (wouldnt make sense for them
//to be on your team list, and departed list)
BOOLEAN RemoveNewlyHiredMercFromPersonnelDepartedList(UINT8 ubProfile)
{
	for (INT32 i = 0; i < 256; i++)
	{
		// are they already in the Dead list?
		if (LaptopSaveInfo.ubDeadCharactersList[i] == ubProfile)
		{
			//Reset the fact that they were once hired
			LaptopSaveInfo.ubDeadCharactersList[i] = -1;
			return TRUE;
		}

		// are they already in the other list?
		if (LaptopSaveInfo.ubLeftCharactersList[i] == ubProfile)
		{
			//Reset the fact that they were once hired
			LaptopSaveInfo.ubLeftCharactersList[i] = -1;
			return TRUE;
		}

		// are they already in the list?
		if (LaptopSaveInfo.ubOtherCharactersList[i] == ubProfile)
		{
			//Reset the fact that they were once hired
			LaptopSaveInfo.ubOtherCharactersList[i] = -1;
			return TRUE;
		}
	}

	return FALSE;
}


// grab the id of the first merc being displayed
static INT32 GetIdOfFirstDisplayedMerc(void)
{
	// set current soldier
	if (fCurrentTeamMode)
	{
		// run through list of soldiers on players current team
		const SOLDIERTYPE* pSoldier = MercPtrs[0];
		INT32 cnt = 0;
		//cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;
		for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, pSoldier++)
		{
			if ((pSoldier->bActive) && (pSoldier->bLife > 0))
			{
				return (0);
			}
		}
		return -1;
	}
	else
	{
		// run through list of soldier on players old team...the slot id will be translated
		return 0;
	}
}


// id of merc in this slot
static INT32 GetIdOfThisSlot(INT32 iSlot)
{
	if (fCurrentTeamMode)
	{
		// run through list of soldiers on players current team
		const SOLDIERTYPE* pSoldier = MercPtrs[0];
		INT32 cnt = gTacticalStatus.Team[pSoldier->bTeam].bFirstID;
		INT32 iCounter = 0;
		for (pSoldier = MercPtrs[cnt]; cnt <= gTacticalStatus.Team[pSoldier->bTeam].bLastID; cnt++, pSoldier++)
		{
			if (pSoldier->bActive)
			{
				// same character as slot, return this value
				if (iCounter == iSlot) return cnt;

				// found another soldier
				iCounter++;
			}
		}

		return 0;
	}
	else
	{
		// run through list of soldier on players old team...the slot id will be translated
		return iSlot;
	}
}


static BOOLEAN RenderAtmPanel(void)
{
	// just show basic panel
	// bounding
	UINT32 uiBox = AddVideoObjectFromFile("LAPTOP/AtmButtons.sti");
	CHECKF(uiBox != NO_VOBJECT);
	BltVideoObjectFromIndex(FRAME_BUFFER, uiBox, 0, ATM_UL_X, ATM_UL_Y);
	BltVideoObjectFromIndex(FRAME_BUFFER, uiBox, 1, ATM_UL_X + 1, ATM_UL_Y + 18);
	DeleteVideoObjectFromIndex(uiBox);

	// create destroy
	CreateDestroyStartATMButton();
	return (TRUE);
}


static void EmployementInfoButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void PersonnelINVStartButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void PersonnelStatStartButtonCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateDestroyStartATMButton(void)
{
	static BOOLEAN fCreated = FALSE;
	// create/destroy atm start button as needed

	if ((fCreated == FALSE) && (fShowAtmPanelStartButton == TRUE))
	{
		// not created, must create

		// the stats button
		giPersonnelATMStartButtonImage[PERSONNEL_STAT_BTN] = LoadButtonImage("LAPTOP/AtmButtons.sti", -1, 2, -1, 3, -1);
		giPersonnelATMStartButton[PERSONNEL_STAT_BTN] = QuickCreateButton(giPersonnelATMStartButtonImage[PERSONNEL_STAT_BTN], 519, 80, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CALLBACK, PersonnelStatStartButtonCallback);

		// set text and what not
		SpecifyButtonText(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], gsAtmStartButtonText[0]);
		SpecifyButtonUpTextColors(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], FONT_BLACK, FONT_BLACK);
		SpecifyButtonFont(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], PERS_FONT);
		SetButtonCursor(giPersonnelATMStartButton[PERSONNEL_STAT_BTN], CURSOR_LAPTOP_SCREEN);

		// the Employment selection button
		giPersonnelATMStartButtonImage[PERSONNEL_EMPLOYMENT_BTN] = LoadButtonImage("LAPTOP/AtmButtons.sti", -1, 2, -1, 3, -1);
		giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN] = QuickCreateButton(giPersonnelATMStartButtonImage[PERSONNEL_EMPLOYMENT_BTN], 519, 110, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CALLBACK, EmployementInfoButtonCallback);

		// set text and what not
		SpecifyButtonText(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], gsAtmStartButtonText[2]);
		SpecifyButtonUpTextColors(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], FONT_BLACK, FONT_BLACK);
		SpecifyButtonFont(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], PERS_FONT);
		SetButtonCursor(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN], CURSOR_LAPTOP_SCREEN);

		// the inventory selection button
		giPersonnelATMStartButtonImage[PERSONNEL_INV_BTN] = LoadButtonImage("LAPTOP/AtmButtons.sti", -1, 2, -1, 3, -1);
		giPersonnelATMStartButton[PERSONNEL_INV_BTN] = QuickCreateButton(giPersonnelATMStartButtonImage[PERSONNEL_INV_BTN], 519, 140, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CALLBACK, PersonnelINVStartButtonCallback);

		// set text and what not
		SpecifyButtonText(giPersonnelATMStartButton[PERSONNEL_INV_BTN], gsAtmStartButtonText[1]);
		SpecifyButtonUpTextColors(giPersonnelATMStartButton[PERSONNEL_INV_BTN], FONT_BLACK, FONT_BLACK);
		SpecifyButtonFont(giPersonnelATMStartButton[PERSONNEL_INV_BTN], PERS_FONT);
		SetButtonCursor(giPersonnelATMStartButton[PERSONNEL_INV_BTN], CURSOR_LAPTOP_SCREEN);

		fCreated = TRUE;
	}
	else if ((fCreated == TRUE) && (fShowAtmPanelStartButton == FALSE))
	{
		// stop showing
		RemoveButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
		UnloadButtonImage(giPersonnelATMStartButtonImage[PERSONNEL_STAT_BTN]);
		RemoveButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);
		UnloadButtonImage(giPersonnelATMStartButtonImage[PERSONNEL_EMPLOYMENT_BTN]);
		RemoveButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
		UnloadButtonImage(giPersonnelATMStartButtonImage[PERSONNEL_INV_BTN]);

		fCreated = FALSE;
	}
}


static void FindPositionOfPersInvSlider(void)
{
	INT32 iValue = 0;
	INT32 iNumberOfItems = 0;
	INT16 sSizeOfEachSubRegion = 0;

	// find out how many there are
	iValue = (INT32)(GetNumberOfInventoryItemsOnCurrentMerc());

	// otherwise there are more than one item
	iNumberOfItems = iValue - NUMBER_OF_INVENTORY_PERSONNEL;

	if (iValue <= 0)
	{
		iValue = 1;
	}

	// get the subregion sizes
	sSizeOfEachSubRegion = (INT16)((INT32)(Y_SIZE_OF_PERSONNEL_SCROLL_REGION - SIZE_OF_PERSONNEL_CURSOR) / (INT32)(iNumberOfItems));

	// get slider position
	guiSliderPosition = uiCurrentInventoryIndex * sSizeOfEachSubRegion;
}


static void HandleSliderBarClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = 0;
	INT32 iNumberOfItems = 0;
	POINT MousePos;
	INT16 sSizeOfEachSubRegion = 0;
	INT16 sYPositionOnBar = 0;
	INT16 iCurrentItemValue = 0;

	if ((iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN) || (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT))
	{
		// find out how many there are
		iValue = (INT32)(GetNumberOfInventoryItemsOnCurrentMerc());

		// make sure there are more than one page
		if ((INT32)uiCurrentInventoryIndex >= iValue - NUMBER_OF_INVENTORY_PERSONNEL + 1)
		{
			return;
		}

		// otherwise there are more than one item
		iNumberOfItems = iValue - NUMBER_OF_INVENTORY_PERSONNEL;

		// number of items is 0
		if (iNumberOfItems == 0)
		{
			return;
		}

		// find the x, y on the slider bar
		GetCursorPos(&MousePos);

		// get the subregion sizes
		sSizeOfEachSubRegion = (INT16)((INT32)(Y_SIZE_OF_PERSONNEL_SCROLL_REGION - SIZE_OF_PERSONNEL_CURSOR) / (INT32)(iNumberOfItems));

		// get the cursor placement
		sYPositionOnBar = MousePos.y - Y_OF_PERSONNEL_SCROLL_REGION;

		if (sSizeOfEachSubRegion == 0)
		{
			return;
		}

		// get the actual item position
		iCurrentItemValue = sYPositionOnBar / sSizeOfEachSubRegion;

		if (uiCurrentInventoryIndex != iCurrentItemValue)
		{
			// get slider position
			guiSliderPosition = iCurrentItemValue * sSizeOfEachSubRegion;

			// set current inventory value
			uiCurrentInventoryIndex = (UINT8)iCurrentItemValue;

			// force update
			fReDrawScreenFlag = TRUE;
		}
	}
}


static void RenderSliderBarForPersonnelInventory(void)
{
	// render slider bar for personnel
	BltVideoObjectFromIndex(FRAME_BUFFER, guiPersonnelInventory, 5, X_OF_PERSONNEL_SCROLL_REGION, guiSliderPosition + Y_OF_PERSONNEL_SCROLL_REGION);
}


static void PersonnelINVStartButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		fReDrawScreenFlag = TRUE;
		btn->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags       &= ~BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &= ~BUTTON_CLICKED_ON;
		gubPersonnelInfoState = PRSNL_INV;
	}
}


static void PersonnelStatStartButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		fReDrawScreenFlag = TRUE;
		btn->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags        &= ~BUTTON_CLICKED_ON;
		gubPersonnelInfoState = PRSNL_STATS;
	}
}


static void EmployementInfoButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		fReDrawScreenFlag = TRUE;
		btn->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags  &= ~BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags &= ~BUTTON_CLICKED_ON;
		gubPersonnelInfoState = PRSNL_EMPLOYMENT;
	}
}


// get the total amt of money on this guy
static INT32 GetFundsOnMerc(const SOLDIERTYPE* pSoldier)
{
	INT32 iCurrentAmount = 0;
	INT32 iCurrentPocket = 0;
	// run through mercs pockets, if any money in them, add to total

	// error check
	if (pSoldier == NULL)
	{
		return 0;
	}

	// run through grunts pockets and count all the spare change
	for (iCurrentPocket = 0; iCurrentPocket < NUM_INV_SLOTS; iCurrentPocket++)
	{
		if (Item[pSoldier->inv[iCurrentPocket].usItem].usItemClass == IC_MONEY)
		{
			iCurrentAmount += pSoldier->inv[iCurrentPocket].uiMoneyAmount;
		}
	}

	return iCurrentAmount;
}


// check if current guy can have atm
static void UpDateStateOfStartButton(void)
{
	INT32 iId = 0;

	// start button being shown?
	if (fShowAtmPanelStartButton == FALSE)
	{
		return;
	}

	if (gubPersonnelInfoState == PRSNL_INV)
	{
		ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
		ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
	}
	else if (gubPersonnelInfoState == PRSNL_STATS)
	{
		ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags |= BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
	}
	else
	{
		ButtonList[giPersonnelATMStartButton[PERSONNEL_STAT_BTN]]->uiFlags &= ~BUTTON_CLICKED_ON;
		ButtonList[giPersonnelATMStartButton[PERSONNEL_INV_BTN]]->uiFlags &= ~(BUTTON_CLICKED_ON);
		ButtonList[giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]]->uiFlags |= BUTTON_CLICKED_ON;
	}

	// if in current mercs and the currently selected guy is valid, enable button, else disable it
	if (fCurrentTeamMode == TRUE)
	{
		// is the current guy valid
		if (GetNumberOfMercsDeadOrAliveOnPlayersTeam() > 0)
		{
			EnableButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
			EnableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
			EnableButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);

			iId = GetIdOfThisSlot(iCurrentPersonSelectedId);

			if (iId != -1)
			{
				if (Menptr[iId].bAssignment == ASSIGNMENT_POW)
				{
					DisableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);

					if (gubPersonnelInfoState == PRSNL_INV)
					{
						gubPersonnelInfoState = PRSNL_STATS;
						fPausedReDrawScreenFlag = TRUE;
					}
				}
			}
		}
		else
		{
			// not valid, disable
			DisableButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
			DisableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
			DisableButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);
		}
	}
	else
	{
		// disable button
		EnableButton(giPersonnelATMStartButton[PERSONNEL_STAT_BTN]);
		DisableButton(giPersonnelATMStartButton[PERSONNEL_INV_BTN]);
		DisableButton(giPersonnelATMStartButton[PERSONNEL_EMPLOYMENT_BTN]);
	}
}


static void DisplayAmountOnCurrentMerc(void)
{
	// will display the amount that the merc is carrying on him or herself
	INT32 iId = GetIdOfThisSlot(iCurrentPersonSelectedId);
	const SOLDIERTYPE* pSoldier = (iId == -1 ? NULL : MercPtrs[iId]);

	CHAR16 sString[64];
	SPrintMoney(sString, GetFundsOnMerc(pSoldier));

	SetFont(ATM_FONT);
	SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);

	INT16 sX;
	INT16 sY;
	FindFontRightCoordinates(ATM_DISPLAY_X, ATM_DISPLAY_Y, ATM_DISPLAY_WIDTH, ATM_DISPLAY_HEIGHT, sString, ATM_FONT, &sX, &sY);
	mprintf(sX, sY, sString);
}


static void HandlePersonnelKeyboard(void)
{
	InputAtom InputEvent;
	while (DequeueEvent(&InputEvent))
	{
		HandleKeyBoardShortCutsForLapTop(InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState);
	}
}


static BOOLEAN IsPastMercDead(INT32 iId)
{
	return GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iId)) == DEPARTED_DEAD;
}


static BOOLEAN IsPastMercFired(INT32 iId)
{
	return GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iId)) == DEPARTED_FIRED;
}


static BOOLEAN IsPastMercOther(INT32 iId)
{
	return GetTheStateOfDepartedMerc(GetIdOfPastMercInSlot(iId)) == DEPARTED_OTHER;
}


static INT32 CalcTimeLeftOnMercContract(const SOLDIERTYPE* pSoldier);


static void DisplayEmploymentinformation(INT32 iId)
{
	wchar_t sString[50];
	wchar_t sStringA[50];
	INT16 sX, sY;

	const SOLDIERTYPE* s = &Menptr[iId];
	if (s->uiStatusFlags & SOLDIER_VEHICLE) return;

	const MERCPROFILESTRUCT* p = &gMercProfiles[s->ubProfile];

	// display the stats for a char
	for (INT32 i = 0; i < MAX_STATS; i++)
	{
		switch (i)
		{
			case 0: //Remaining Contract:
			{
				if (s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC || s->ubProfile == SLAY)
				{
					const UINT32 uiMinutesInDay = 24 * 60;
					INT32 iTimeLeftOnContract = CalcTimeLeftOnMercContract(s);

					//if the merc is in transit
					if (s->bAssignment == IN_TRANSIT)
					{
						//and if the ttime left on the cotract is greater then the contract time
						if (iTimeLeftOnContract > (INT32)(s->iTotalContractLength * uiMinutesInDay))
						{
							iTimeLeftOnContract = (s->iTotalContractLength * uiMinutesInDay);
						}
					}
					// if there is going to be a both days and hours left on the contract
					if (iTimeLeftOnContract / uiMinutesInDay)
					{
						swprintf(sString, lengthof(sString), L"%d%ls %d%ls / %d%ls", (iTimeLeftOnContract / uiMinutesInDay), gpStrategicString[STR_PB_DAYS_ABBREVIATION], (iTimeLeftOnContract % uiMinutesInDay)/60, gpStrategicString[STR_PB_HOURS_ABBREVIATION], s->iTotalContractLength, gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
					}
					else //else there is under a day left
					{
						//DEF: removed 2/7/99
						swprintf(sString, lengthof(sString), L"%d%ls / %d%ls", (iTimeLeftOnContract % uiMinutesInDay)/60, gpStrategicString[STR_PB_HOURS_ABBREVIATION], s->iTotalContractLength, gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
					}
				}
				else if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
				{
					wcscpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
				}
				else
				{
					wcscpy(sString, gpStrategicString[STR_PB_NOTAPPLICABLE_ABBREVIATION]);
				}

				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[PRSNL_TXT_CURRENT_CONTRACT]);
				FindFontRightCoordinates(pers_stat_data_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
			}
			break;

			case 1: // total contract time served
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[PRSNL_TXT_TOTAL_SERVICE]);
				//./DEF 2/4/99: total service days used to be calced as 'days -1'
				swprintf(sString, lengthof(sString), L"%d %ls", p->usTotalDaysServed, gpStrategicString[STR_PB_DAYS_ABBREVIATION]);
				FindFontRightCoordinates(pers_stat_data_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i], sString);
				break;

			case 3: // cost (PRSNL_TXT_TOTAL_COST)
			{
				SPrintMoney(sString, p->uiTotalCostToDate);
				FindFontRightCoordinates(pers_stat_data_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(pers_stat_x, pers_stat_y[i], pPersonnelScreenStrings[PRSNL_TXT_TOTAL_COST]);

				// print contract cost
				mprintf((INT16)(sX), pers_stat_y[i], sString);

				INT32 salary;
				if (s->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
				{
					// daily rate
					if (s->bTypeOfLastContract == CONTRACT_EXTEND_2_WEEK)
					{
						// 2 week contract
						salary = p->uiBiWeeklySalary / 14;
					}
					else if (s->bTypeOfLastContract == CONTRACT_EXTEND_1_WEEK)
					{
						// 1 week contract
						salary = p->uiWeeklySalary / 7;
					}
					else
					{
						salary = p->sSalary;
					}
				}
				else if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
				{
					salary = p->sSalary;
				}
				else
				{
					salary = p->sSalary;
				}

				SPrintMoney(sStringA, salary);
				FindFontRightCoordinates(pers_stat_data_x, 0, TEXT_BOX_WIDTH - 20, 0, sStringA, PERS_FONT,  &sX, &sY);

				i++;

				// now print daily rate
				mprintf(sX, pers_stat_y[i + 1], sStringA);
				mprintf(pers_stat_x, pers_stat_y[i + 1], pPersonnelScreenStrings[PRSNL_TXT_DAILY_COST]);
				break;
			}

			case 5: // medical deposit
				//if its a merc merc, display the salary oweing
				if (s->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
				{
					mprintf(pers_stat_x, pers_stat_y[i - 1], pPersonnelScreenStrings[PRSNL_TXT_UNPAID_AMOUNT]);
					SPrintMoney(sString, p->sSalary * p->iMercMercContractLength);
				}
				else
				{
					mprintf(pers_stat_x, pers_stat_y[i - 1], pPersonnelScreenStrings[PRSNL_TXT_MED_DEPOSIT]);
					SPrintMoney(sString, p->sMedicalDepositAmount);
				}
				FindFontRightCoordinates(pers_stat_data_x, 0, TEXT_BOX_WIDTH - 20, 0, sString, PERS_FONT, &sX, &sY);
				mprintf(sX, pers_stat_y[i - 1], sString);
				break;
		}
	}
}


// AIM merc:  Returns the amount of time left on mercs contract
// MERC merc: Returns the amount of time the merc has worked
// IMP merc:	Returns the amount of time the merc has worked
// else:			returns -1
static INT32 CalcTimeLeftOnMercContract(const SOLDIERTYPE* pSoldier)
{
	INT32 iTimeLeftOnContract = -1;

	if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__AIM_MERC)
	{
		iTimeLeftOnContract = pSoldier->iEndofContractTime-GetWorldTotalMin();

		if (iTimeLeftOnContract < 0)
			iTimeLeftOnContract = 0;
	}
	else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__MERC)
	{
		iTimeLeftOnContract = gMercProfiles[pSoldier->ubProfile].iMercMercContractLength;
	}

	else if (pSoldier->ubWhatKindOfMercAmI == MERC_TYPE__PLAYER_CHARACTER)
	{
		iTimeLeftOnContract = pSoldier->iTotalContractLength;
	}

	else
	{
		iTimeLeftOnContract = -1;
	}

	return iTimeLeftOnContract;
}
