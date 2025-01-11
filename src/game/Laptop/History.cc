#include "Directories.h"
#include "Font.h"
#include "Font_Control.h"
#include "Laptop.h"
#include "LoadSaveData.h"
#include "History.h"
#include "Quests.h"
#include "Soldier_Control.h"
#include "Debug.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Soldier_Profile.h"
#include "StrategicMap.h"
#include "Text.h"
#include "Message.h"
#include "LaptopSave.h"
#include "Button_System.h"
#include "Object_Cache.h"
#include "VSurface.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>


#define HISTORY_QUEST_TEXT_SIZE 80


struct HistoryUnit
{
	UINT8 ubCode; // the code index in the finance code table
	UINT8 ubSecondCode; // secondary code
	UINT32 uiDate; // time in the world in global time
	SGPSector sSector; // sector this took place in
	HistoryUnit* Next; // next unit in the list
};


#define TOP_X				0+LAPTOP_SCREEN_UL_X
#define TOP_Y				LAPTOP_SCREEN_UL_Y
#define BOX_HEIGHT			14
#define TOP_DIVLINE_Y			(STD_SCREEN_Y + 101)
#define TITLE_X				(STD_SCREEN_X + 140)
#define TITLE_Y				(STD_SCREEN_Y + 33 )
#define RECORD_Y			TOP_DIVLINE_Y
#define PAGE_NUMBER_X			TOP_X+20
#define PAGE_NUMBER_Y			TOP_Y+33
#define HISTORY_DATE_X			PAGE_NUMBER_X+85
#define HISTORY_DATE_Y			PAGE_NUMBER_Y
#define RECORD_LOCATION_WIDTH		142//95

#define HISTORY_HEADER_FONT		FONT14ARIAL
#define HISTORY_TEXT_FONT		FONT12ARIAL
#define RECORD_DATE_X			TOP_X+10
#define RECORD_DATE_WIDTH		31//68
#define RECORD_HEADER_Y			(STD_SCREEN_Y + 90)


#define NUM_RECORDS_PER_PAGE		(22)
#define SIZE_OF_HISTORY_FILE_RECORD	( sizeof( UINT8 ) + sizeof( UINT8 ) + sizeof( UINT32 ) + sizeof( UINT16 ) + sizeof( UINT16 ) + sizeof( UINT8 ) + sizeof( UINT8 ) )

// button positions
#define NEXT_BTN_X			(STD_SCREEN_X + 577)
#define PREV_BTN_X			(STD_SCREEN_X + 553)
#define BTN_Y				(STD_SCREEN_Y + 53 )

// graphics handles
namespace {
// top portion of the screen background
cache_key_t const guiTOP{ LAPTOPDIR "/historywindow.sti" };

// shaded line
cache_key_t const guiSHADELINE{ LAPTOPDIR "/historylines.sti" };

// black divider line - long ( 480 length)
cache_key_t const guiLONGLINE{ LAPTOPDIR "/divisionline480.sti" };


enum{
	PREV_PAGE_BUTTON=0,
	NEXT_PAGE_BUTTON,
};
}

// the page flipping buttons
static GUIButtonRef giHistoryButton[2];

static MOUSE_REGION g_scroll_region;


static BOOLEAN fInHistoryMode = FALSE;
// current page displayed
static INT32 iCurrentHistoryPage = 1;


// the History record list
static HistoryUnit* pHistoryListHead = NULL;


void ClearHistoryList( void );


static void AppendHistoryToEndOfFile(void);
static BOOLEAN LoadInHistoryRecords(const UINT32 uiPage);
static void ProcessAndEnterAHistoryRecord(UINT8 ubCode, UINT32 uiDate, UINT8 ubSecondCode, const SGPSector& sSector);


void AddHistoryToPlayersLog(const UINT8 ubCode, const UINT8 ubSecondCode, const UINT32 uiDate, const SGPSector& sSector)
{
	ClearHistoryList();

	ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, sSector);
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_HISTORY_UPDATED]);

	AppendHistoryToEndOfFile();

	// if in history mode, reload current page
	if (fInHistoryMode) LoadInHistoryRecords(iCurrentHistoryPage);
}


void GameInitHistory()
{
	GCM->tempFiles()->deleteFile(HISTORY_DATA_FILE);
}


static void CreateHistoryButtons(void);
static void SetHistoryButtonStates(void);


void EnterHistory()
{
	// create History buttons
	CreateHistoryButtons( );

	// reset current to first page
	iCurrentHistoryPage = LaptopSaveInfo.iCurrentHistoryPage;
	if (iCurrentHistoryPage <= 0) iCurrentHistoryPage = 1;

	LoadInHistoryRecords(iCurrentHistoryPage);

	// render hbackground
	RenderHistory( );


	// set the fact we are in the history viewer
	fInHistoryMode=TRUE;

	// build Historys list
	//OpenAndReadHistoryFile( );

	// force redraw of the entire screen
	//fReDrawScreenFlag=TRUE;

	// set inital states
	SetHistoryButtonStates( );
}


static void DestroyHistoryButtons(void);
static void RemoveHistory(void);


void ExitHistory()
{
	LaptopSaveInfo.iCurrentHistoryPage = iCurrentHistoryPage;

	// not in History system anymore
	fInHistoryMode=FALSE;

	// delete graphics
	RemoveHistory( );

	// delete buttons
	DestroyHistoryButtons( );

	ClearHistoryList( );
}


static void DrawAPageofHistoryRecords(void);
static void RenderHistoryBackGround(void);


void RenderHistory( void )
{
	//render the background to the display
	RenderHistoryBackGround( );

	// render the currentpage of records
	DrawAPageofHistoryRecords( );

	// title bar icon
	BlitTitleBarIcons(  );
}


static void RemoveHistory(void)
{
	// delete history video objects from memory
	RemoveVObject(guiLONGLINE);
	RemoveVObject(guiTOP);
	RemoveVObject(guiSHADELINE);
}


static void RenderHistoryBackGround(void)
{
	// render generic background for history system
	BltVideoObject(FRAME_BUFFER, guiTITLEBARLAPTOP, 0, TOP_X, TOP_Y - 2);
	BltVideoObject(FRAME_BUFFER, guiTOP,   0, TOP_X, TOP_Y + 22);
}


static void DrawHistoryTitleText(void)
{
	// draw the pages title
	SetFontAttributes(HISTORY_HEADER_FONT, FONT_WHITE);
	MPrint(TITLE_X, TITLE_Y, pHistoryTitle);
}


static void LoadNextHistoryPage(void);
static void LoadPreviousHistoryPage(void);


static void ScrollRegionCallback(MOUSE_REGION* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		LoadPreviousHistoryPage();
	}
	else if (reason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		LoadNextHistoryPage();
	}
}


static void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON* btn, UINT32 reason);
static void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON* btn, UINT32 reason);


static void CreateHistoryButtons(void)
{
	// the prev/next page buttons
	giHistoryButton[PREV_PAGE_BUTTON] = QuickCreateButtonImg(LAPTOPDIR "/arrows.sti", 0, 1, PREV_BTN_X, BTN_Y, MSYS_PRIORITY_HIGHEST - 1, BtnHistoryDisplayPrevPageCallBack);
	giHistoryButton[NEXT_PAGE_BUTTON] = QuickCreateButtonImg(LAPTOPDIR "/arrows.sti", 6, 7, NEXT_BTN_X, BTN_Y, MSYS_PRIORITY_HIGHEST - 1, BtnHistoryDisplayNextPageCallBack);

	// set buttons
	giHistoryButton[0]->SetCursor(CURSOR_LAPTOP_SCREEN);
	giHistoryButton[1]->SetCursor(CURSOR_LAPTOP_SCREEN);

	UINT16 const x = TOP_X +  8;
	UINT16 const y = TOP_Y + 53;
	UINT16 const w = 482;
	UINT16 const h = 354;
	MSYS_DefineRegion(&g_scroll_region, x, y, x + w, y + h, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, ScrollRegionCallback);
}


static void DestroyHistoryButtons(void)
{
	// remove History buttons and images from memory
	MSYS_RemoveRegion(&g_scroll_region);
	// next page button
	RemoveButton(giHistoryButton[1] );
	// prev page button
	RemoveButton(giHistoryButton[0] );
}


static void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		fReDrawScreenFlag = TRUE;
	}

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		LoadPreviousHistoryPage();
	}
}


static void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		fReDrawScreenFlag = TRUE;
	}

	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		LoadNextHistoryPage();
	}
}


static void ProcessAndEnterAHistoryRecord(const UINT8 ubCode, const UINT32 uiDate, const UINT8 ubSecondCode, const SGPSector& sSector)
{
	HistoryUnit* const h = new HistoryUnit{};
	h->Next         = NULL;
	h->ubCode       = ubCode;
	h->ubSecondCode = ubSecondCode;
	h->uiDate       = uiDate;
	h->sSector      = sSector;

	// Append node to list
	HistoryUnit** anchor = &pHistoryListHead;
	while (*anchor != NULL) anchor = &(*anchor)->Next;
	*anchor = h;
}

// open and read in data to the History list
static void OpenAndReadHistoryFile(void)
{
	ClearHistoryList();

	AutoSGPFile f(GCM->tempFiles()->openForReading(HISTORY_DATA_FILE));

	UINT entry_count = f->size() / SIZE_OF_HISTORY_FILE_RECORD;
	while (entry_count-- > 0)
	{
		UINT8  ubCode;
		UINT8  ubSecondCode;
		UINT32 uiDate;
		SGPSector sSector;

		f->read(&ubCode,       sizeof(UINT8));
		f->read(&ubSecondCode, sizeof(UINT8));
		f->read(&uiDate,       sizeof(UINT32));
		f->read(&sSector.x,    sizeof(INT16));
		f->read(&sSector.y,    sizeof(INT16));
		f->read(&sSector.z,    sizeof(INT8));
		f->seek(1, FILE_SEEK_FROM_CURRENT);

		ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, sSector);
	}
}


void ClearHistoryList(void)
{
	for (HistoryUnit* h = pHistoryListHead; h != NULL;)
	{
		HistoryUnit* const next = h->Next;
		delete h;
		h = next;
	}
	pHistoryListHead = NULL;
}


static void DisplayHistoryListHeaders(void)
{
	// this procedure will display the headers to each column in History
	SetFontAttributes(HISTORY_TEXT_FONT, FONT_BLACK, NO_SHADOW);

	// the date header
	int x{ RECORD_DATE_X + 5 };
	MPrint(x, RECORD_HEADER_Y, pHistoryHeaders[0], CenterAlign(RECORD_DATE_WIDTH));

	// Location header
	x += RECORD_DATE_WIDTH;
	MPrint(x, RECORD_HEADER_Y, pHistoryHeaders[3], CenterAlign(RECORD_LOCATION_WIDTH));

	// event header
	x += RECORD_LOCATION_WIDTH;
	// 471 is the width in pixels of one row (the width of guiSHADELINE index 0).
	constexpr int RECORD_EVENT_WIDTH{ 471 - RECORD_DATE_WIDTH - RECORD_LOCATION_WIDTH };
	MPrint(x, RECORD_HEADER_Y, pHistoryHeaders[4], CenterAlign(RECORD_EVENT_WIDTH));

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void DisplayHistoryListBackground(void)
{
	// this function will display the History list display background
	INT32 iCounter=0;

	// get shaded line object
	for(iCounter=0; iCounter <11; iCounter++)
	{
		// blt title bar to screen
		BltVideoObject(FRAME_BUFFER, guiSHADELINE, 0, TOP_X + 15, TOP_DIVLINE_Y + BOX_HEIGHT * 2 * iCounter);
	}

	// the long hortizontal line int he records list display region
	BltVideoObject(FRAME_BUFFER, guiLONGLINE, 0,TOP_X + 9, TOP_DIVLINE_Y);
	BltVideoObject(FRAME_BUFFER, guiLONGLINE, 0,TOP_X + 9, TOP_DIVLINE_Y + BOX_HEIGHT * 2 * 11);
}


static ST::string ProcessHistoryTransactionString(const HistoryUnit* h);


// draw the text of the records
static void DrawHistoryRecordsText(void)
{
	ST::string sString;
	INT16   sX;
	INT16   sY;

	SetFont(HISTORY_TEXT_FONT);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	UINT entry_count = 0;
	for (const HistoryUnit* h = pHistoryListHead; h != NULL; h = h->Next)
	{
		const UINT8 colour =
			h->ubCode  == HISTORY_CHEAT_ENABLED ||
			(h->ubCode == HISTORY_QUEST_STARTED && gubQuest[h->ubSecondCode] == QUESTINPROGRESS) ?
				FONT_RED : FONT_BLACK;
		SetFontForeground(colour);

		const INT32 y = RECORD_Y + entry_count * BOX_HEIGHT + 3;

		// get and write the date
		sString = ST::format("{}", h->uiDate / (24 * 60));
		INT16 usX;
		INT16 usY;
		FindFontCenterCoordinates(RECORD_DATE_X + 5, 0, RECORD_DATE_WIDTH, 0, sString, HISTORY_TEXT_FONT, &usX, &usY);
		MPrint(usX, y, sString);

		if (!h->sSector.IsValid())
		{
			// no location
			FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH, 0, RECORD_LOCATION_WIDTH + 10, 0, pHistoryLocations, HISTORY_TEXT_FONT, &sX, &sY);
			MPrint(sX, y, pHistoryLocations);
		}
		else
		{
			sString = GetSectorIDString(h->sSector, TRUE);
			FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH, 0, RECORD_LOCATION_WIDTH + 10, 0,  sString, HISTORY_TEXT_FONT, &sX, &sY);
			sString = ReduceStringLength(sString, RECORD_LOCATION_WIDTH + 10, HISTORY_TEXT_FONT);
			MPrint(sX, y, sString);
		}

		// the actual history text
		sString = ProcessHistoryTransactionString(h);
		MPrint(RECORD_DATE_X + RECORD_LOCATION_WIDTH + RECORD_DATE_WIDTH + 15, y, sString);

		if (++entry_count == NUM_RECORDS_PER_PAGE) break;
	}

	// restore shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void DisplayPageNumberAndDateRange(void);


static void DrawAPageofHistoryRecords(void)
{
	// this procedure will draw a series of history records to the screen

	// (re-)render background

	// the title bar text
	DrawHistoryTitleText( );

	// the actual lists background
	DisplayHistoryListBackground( );

	// the headers to each column
	DisplayHistoryListHeaders( );


	// error check
	if(iCurrentHistoryPage==-1)
	{
		iCurrentHistoryPage=0;
	}


	// current page is found, render  from here
	DrawHistoryRecordsText( );

	// update page numbers, and date ranges
	DisplayPageNumberAndDateRange( );
}


static INT32 GetNumberOfHistoryPages(void);


/* go through the list of 'histories' starting at current until end or
 * NUM_RECORDS_PER_PAGE and get the date range and the page number */
static void DisplayPageNumberAndDateRange(void)
{
	UINT current_page;
	UINT count_pages;
	UINT first_date;
	UINT last_date;
	const HistoryUnit* h = pHistoryListHead;
	if (h == NULL)
	{
		current_page = 1;
		count_pages  = 1;
		first_date   = 1;
		last_date    = 1;
	}
	else
	{
		current_page     = iCurrentHistoryPage;
		count_pages      = GetNumberOfHistoryPages();
		first_date       = h->uiDate / (24 * 60);

		UINT entry_count = NUM_RECORDS_PER_PAGE;
		while (--entry_count != 0 && h->Next != NULL) h = h->Next;

		last_date        = h->uiDate / (24 * 60);
	}

	SetFontAttributes(HISTORY_TEXT_FONT, FONT_BLACK, NO_SHADOW);
	MPrint(PAGE_NUMBER_X,  PAGE_NUMBER_Y, ST::format("{}  {} / {}", pHistoryHeaders[1], current_page, count_pages));
	MPrint(HISTORY_DATE_X, HISTORY_DATE_Y, ST::format("{} {} - {}",  pHistoryHeaders[2], first_date, last_date));
	SetFontShadow(DEFAULT_SHADOW);
}


static ST::string GetQuestEndedString(UINT8 ubQuestValue);
static ST::string GetQuestStartedString(UINT8 ubQuestValue);


static ST::string ProcessHistoryTransactionString(const HistoryUnit* h)
{
	const UINT8 code = h->ubCode;
	switch (code)
	{
		case HISTORY_QUEST_STARTED:
			return GetQuestStartedString(h->ubSecondCode);

		case HISTORY_QUEST_FINISHED:
			return GetQuestEndedString(h->ubSecondCode);

		case HISTORY_LIBERATED_TOWN:
		case HISTORY_MINE_RAN_OUT:
		case HISTORY_MINE_REOPENED:
		case HISTORY_MINE_RUNNING_OUT:
		case HISTORY_MINE_SHUTDOWN:
		case HISTORY_TALKED_TO_MINER:
			return st_format_printf(pHistoryStrings[code], GCM->getTownName(h->ubSecondCode));

		case HISTORY_MERC_KILLED:
			if (h->ubSecondCode == NO_PROFILE)
			{
				break;
			}
			return st_format_printf(pHistoryStrings[code], GetProfile(h->ubSecondCode).zName);

		case HISTORY_HIRED_MERC_FROM_AIM:
		case HISTORY_HIRED_MERC_FROM_MERC:
		case HISTORY_MERC_CONTRACT_EXPIRED:
		case HISTORY_RPC_JOINED_TEAM:
			return st_format_printf(pHistoryStrings[code], GetProfile(h->ubSecondCode).zName);

		case HISTORY_CANCELLED_INSURANCE:
		case HISTORY_DISQUALIFIED_BOXING:
		case HISTORY_EXTENDED_CONTRACT_1_DAY:
		case HISTORY_EXTENDED_CONTRACT_1_WEEK:
		case HISTORY_EXTENDED_CONTRACT_2_WEEK:
		case HISTORY_INSURANCE_CLAIM_PAYOUT:
		case HISTORY_LOST_BOXING:
		case HISTORY_MERC_FIRED:
		case HISTORY_MERC_KILLED_CHARACTER:
		case HISTORY_MERC_MARRIED_OFF:
		case HISTORY_MERC_QUIT:
		case HISTORY_NPC_KILLED:
		case HISTORY_PURCHASED_INSURANCE:
		case HISTORY_WON_BOXING:
			return st_format_printf(pHistoryStrings[code], GetProfile(h->ubSecondCode).zNickname);

		// all simple history log msgs, no params
		case HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO:
		case HISTORY_ARNOLD:
		case HISTORY_ASSASSIN:
		case HISTORY_BOXING_MATCHES:
		case HISTORY_BUM_KEYCARD:
		case HISTORY_CHARACTER_GENERATED:
		case HISTORY_CHEAT_ENABLED:
		case HISTORY_CREATURESATTACKED:
		case HISTORY_DAVE:
		case HISTORY_DEFENDEDTOWNSECTOR:
		case HISTORY_DEIDRANNA_DEAD_BODIES:
		case HISTORY_DEVIN:
		case HISTORY_DISCOVERED_ORTA:
		case HISTORY_DISCOVERED_TIXA:
		case HISTORY_ENRICO_COMPLAINED:
		case HISTORY_ENTERED_HISTORY_MODE:
		case HISTORY_FATALAMBUSH:
		case HISTORY_FOUND_MONEY:
		case HISTORY_FRANZ:
		case HISTORY_FREDO:
		case HISTORY_GABBY:
		case HISTORY_GAVE_CARMEN_HEAD:
		case HISTORY_GOT_ROCKET_RIFLES:
		case HISTORY_HOWARD:
		case HISTORY_HOWARD_CYANIDE:
		case HISTORY_JAKE:
		case HISTORY_KEITH:
		case HISTORY_KEITH_OUT_OF_BUSINESS:
		case HISTORY_KILLEDBYBLOODCATS:
		case HISTORY_KINGPIN_MONEY:
		case HISTORY_KROTT:
		case HISTORY_KYLE:
		case HISTORY_LOSTBATTLE:
		case HISTORY_LOSTTOWNSECTOR:
		case HISTORY_MADLAB:
		case HISTORY_MIKE:
		case HISTORY_PABLO:
		case HISTORY_PERKO:
		case HISTORY_RICHGUY_BALIME:
		case HISTORY_SAM:
		case HISTORY_SETTLED_ACCOUNTS_AT_MERC:
		case HISTORY_SLAUGHTEREDBLOODCATS:
		case HISTORY_SLAY_MYSTERIOUSLY_LEFT:
		case HISTORY_SOMETHING_IN_MINES:
		case HISTORY_SUCCESSFULATTACK:
		case HISTORY_TALKED_TO_FATHER_WALKER:
		case HISTORY_TONY:
		case HISTORY_UNSUCCESSFULATTACK:
		case HISTORY_WALTER:
		case HISTORY_WIPEDOUTENEMYAMBUSH:
		case HISTORY_WONBATTLE:
			return pHistoryStrings[code];
	}
	return {};
}


// look at what page we are viewing, enable and disable buttons as needed
static void SetHistoryButtonStates(void)
{
	EnableButton(giHistoryButton[PREV_PAGE_BUTTON], iCurrentHistoryPage != 1);
	EnableButton(giHistoryButton[NEXT_PAGE_BUTTON], iCurrentHistoryPage < GetNumberOfHistoryPages());
}


// loads in records belogning, to page uiPage
static BOOLEAN LoadInHistoryRecords(const UINT32 uiPage)
try
{
	ClearHistoryList();

	// check if bad page
	if (uiPage == 0) return FALSE;

	AutoSGPFile f(GCM->tempFiles()->openForReading(HISTORY_DATA_FILE));

	UINT       entry_count = f->size() / SIZE_OF_HISTORY_FILE_RECORD;
	UINT const skip        = (uiPage - 1) * NUM_RECORDS_PER_PAGE;
	if (entry_count <= skip) return FALSE;

	f->seek(skip * SIZE_OF_HISTORY_FILE_RECORD, FILE_SEEK_FROM_START);
	entry_count -= skip;

	if (entry_count > NUM_RECORDS_PER_PAGE) entry_count = NUM_RECORDS_PER_PAGE;

	while (entry_count-- > 0)
	{
		UINT8  ubCode;
		UINT8  ubSecondCode;
		UINT32 uiDate;
		SGPSector sSector;

		f->read(&ubCode,       sizeof(UINT8));
		f->read(&ubSecondCode, sizeof(UINT8));
		f->read(&uiDate,       sizeof(UINT32));
		f->read(&sSector.x,    sizeof(INT16));
		f->read(&sSector.y,    sizeof(INT16));
		f->read(&sSector.z,    sizeof(INT8));
		f->seek(1, FILE_SEEK_FROM_CURRENT);

		ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, sSector);
	}

	return TRUE;
}
catch (...) { return FALSE; }


// clear out old list of records, and load in next page worth of records
static void LoadNextHistoryPage(void)
{
	// now load in previous page's records, if we can
	if ( LoadInHistoryRecords( iCurrentHistoryPage + 1 ) )
	{
		iCurrentHistoryPage++;
	}
	else
	{
		LoadInHistoryRecords( iCurrentHistoryPage );
	}
	SetHistoryButtonStates();
	fReDrawScreenFlag = TRUE;
}


// clear out old list of records and load in previous page worth of records
static void LoadPreviousHistoryPage(void)
{
	if (iCurrentHistoryPage <= 1) return;
	LoadInHistoryRecords(--iCurrentHistoryPage);
	SetHistoryButtonStates();
	fReDrawScreenFlag = TRUE;
}


static void AppendHistoryToEndOfFile(void)
{
	AutoSGPFile f(GCM->tempFiles()->openForAppend(HISTORY_DATA_FILE));

	const HistoryUnit* const h = pHistoryListHead;

	BYTE  data[12];
	DataWriter d{data};
	INJ_U8(d, h->ubCode)
	INJ_U8(d, h->ubSecondCode)
	INJ_U32(d, h->uiDate)
	INJ_I16(d, h->sSector.x)
	INJ_I16(d, h->sSector.y)
	INJ_I8(d, h->sSector.z)
	INJ_SKIP(d, 1)
	Assert(d.getConsumed() == lengthof(data));

	f->write(data, sizeof(data));
}


UINT32 GetTimeQuestWasStarted(const UINT8 ubCode)
{
	iCurrentHistoryPage = 0;
	OpenAndReadHistoryFile();

	UINT32 uiTime = 0;
	for (const HistoryUnit* h = pHistoryListHead; h != NULL; h = h->Next)
	{
		if (h->ubSecondCode == ubCode && h->ubCode == HISTORY_QUEST_STARTED)
		{
			uiTime = h->uiDate;
			break;
		}
	}

	if (fInHistoryMode) LoadInHistoryRecords(iCurrentHistoryPage);

	return uiTime;
}


static ST::string GetQuestStartedString(const UINT8 ubQuestValue)
{
	// open the file and copy the string
	return GCM->loadEncryptedString(BINARYDATADIR "/quests.edt", HISTORY_QUEST_TEXT_SIZE * ubQuestValue * 2, HISTORY_QUEST_TEXT_SIZE);
}


static ST::string GetQuestEndedString(const UINT8 ubQuestValue)
{
	// open the file and copy the string
	return GCM->loadEncryptedString(BINARYDATADIR "/quests.edt", HISTORY_QUEST_TEXT_SIZE * (ubQuestValue * 2 + 1), HISTORY_QUEST_TEXT_SIZE);
}

static INT32 GetNumberOfHistoryPages(void)
{
	AutoSGPFile f(GCM->tempFiles()->openForReading(HISTORY_DATA_FILE));

	const UINT32 uiFileSize = f->size();

	if (uiFileSize == 0) return 1;

	return (uiFileSize / SIZE_OF_HISTORY_FILE_RECORD + NUM_RECORDS_PER_PAGE - 1) / NUM_RECORDS_PER_PAGE;
}
