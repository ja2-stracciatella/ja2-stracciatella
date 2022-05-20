#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "Finances.h"
#include "Game_Clock.h"
#include "LoadSaveData.h"
#include "Map_Screen_Interface_Bottom.h"
#include "VObject.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Soldier_Profile.h"
#include "Text.h"
#include "Strategic_Mines.h"
#include "LaptopSave.h"
#include "Campaign_Types.h"
#include "StrategicMap.h"
#include "VSurface.h"
#include "Button_System.h"
#include "Font_Control.h"
#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>


#define FINANCE_HEADER_SIZE 4
#define FINANCE_RECORD_SIZE (1 + 1 + 4 + 4 + 4)


// the financial structure
struct FinanceUnit
{
	UINT8 ubCode; // the code index in the finance code table
	UINT8 ubSecondCode; // secondary code: Profile ID or sector ID
	UINT32 uiDate; // time in the world in global time
	INT32 iAmount; // the amount of the transaction
	INT32 iBalanceToDate;
	FinanceUnit* Next; // next unit in the list
};


// the global defines

// graphical positions
#define TOP_X 0+LAPTOP_SCREEN_UL_X
#define TOP_Y LAPTOP_SCREEN_UL_Y
#define BLOCK_HEIGHT 10
#define TOP_DIVLINE_Y (102 + STD_SCREEN_Y)
#define DIVLINE_X (130 + STD_SCREEN_X)
#define MID_DIVLINE_Y (205 + STD_SCREEN_Y)
#define BOT_DIVLINE_Y (180 + STD_SCREEN_Y)
#define MID_DIVLINE_Y2 (263 + 20 + STD_SCREEN_Y)
#define BOT_DIVLINE_Y2 MID_DIVLINE_Y2 + MID_DIVLINE_Y - BOT_DIVLINE_Y
#define TITLE_X (140 + STD_SCREEN_X)
#define TITLE_Y (33 + STD_SCREEN_Y)
#define TEXT_X (140 + STD_SCREEN_X)
#define PAGE_SIZE 17

// yesterdyas/todays income and balance text positions
#define YESTERDAYS_INCOME               (STD_SCREEN_Y + 114)
#define YESTERDAYS_OTHER                (STD_SCREEN_Y + 138)
#define YESTERDAYS_DEBITS               (STD_SCREEN_Y + 162)
#define YESTERDAYS_BALANCE              (STD_SCREEN_Y + 188)
#define TODAYS_INCOME                   (STD_SCREEN_Y + 215)
#define TODAYS_OTHER                    (STD_SCREEN_Y + 239)
#define TODAYS_DEBITS                   (STD_SCREEN_Y + 263)
#define TODAYS_CURRENT_BALANCE          (STD_SCREEN_Y + 263 + 28)
#define TODAYS_CURRENT_FORCAST_INCOME   (STD_SCREEN_Y + 330)
#define TODAYS_CURRENT_FORCAST_BALANCE  (STD_SCREEN_Y + 354)
#define FINANCE_HEADER_FONT FONT14ARIAL
#define FINANCE_TEXT_FONT FONT12ARIAL
#define NUM_RECORDS_PER_PAGE PAGE_SIZE

// records text positions
#define RECORD_CREDIT_WIDTH (106-47)
#define RECORD_DEBIT_WIDTH RECORD_CREDIT_WIDTH
#define RECORD_DATE_X TOP_X+10
#define RECORD_TRANSACTION_X RECORD_DATE_X+RECORD_DATE_WIDTH
#define RECORD_TRANSACTION_WIDTH 500-280
#define RECORD_DEBIT_X RECORD_TRANSACTION_X+RECORD_TRANSACTION_WIDTH
#define RECORD_CREDIT_X RECORD_DEBIT_X+RECORD_DEBIT_WIDTH
#define RECORD_Y (107-10 + STD_SCREEN_Y)
#define RECORD_DATE_WIDTH 47
#define RECORD_BALANCE_X RECORD_DATE_X+385
#define RECORD_BALANCE_WIDTH 479-385
#define RECORD_HEADER_Y (90 + STD_SCREEN_Y)


#define PAGE_NUMBER_X TOP_X+297 //345
#define PAGE_NUMBER_Y TOP_Y+33


// BUTTON defines
enum{
	PREV_PAGE_BUTTON=0,
	NEXT_PAGE_BUTTON,
	FIRST_PAGE_BUTTON,
	LAST_PAGE_BUTTON,
};


// button positions

#define FIRST_PAGE_X	(STD_SCREEN_X + 505)
#define NEXT_BTN_X	(STD_SCREEN_X + 553)
#define PREV_BTN_X	(STD_SCREEN_X + 529)
#define LAST_PAGE_X	(STD_SCREEN_X + 577)
#define BTN_Y		(STD_SCREEN_Y +  53)



// sizeof one record
#define RECORD_SIZE ( sizeof( UINT32 ) + sizeof( INT32 ) + sizeof( INT32 ) + sizeof( UINT8 ) + sizeof( UINT8 ) )




// the financial record list
static FinanceUnit* pFinanceListHead = NULL;

// current page displayed
static INT32 iCurrentPage = 0;

// video object id's
static SGPVObject* guiTITLE;
static SGPVObject* guiTOP;
static SGPVObject* guiLINE;
static SGPVObject* guiLONGLINE;
static SGPVObject* guiLISTCOLUMNS;

// are in the financial system right now?
static BOOLEAN fInFinancialMode = FALSE;


// the last page altogether
static UINT32 guiLastPageInRecordsList = 0;

// finance screen buttons
static GUIButtonRef giFinanceButton[4];
static BUTTON_PICS* giFinanceButtonImage[4];
static MOUSE_REGION g_scroll_region;

// internal functions
static void ProcessAndEnterAFinancialRecord(UINT8 ubCode, UINT32 uiDate, INT32 iAmount, UINT8 ubSecondCode, INT32 iBalanceToDate);
static void LoadFinances(void);
static void RemoveFinances(void);
static void ClearFinanceList(void);
static void DrawRecordsColumnHeadersText(void);
static void CreateFinanceButtons(void);
static void DestroyFinanceButtons(void);
static void GetBalanceFromDisk(void);
static void WriteBalanceToDisk(void);
static void AppendFinanceToEndOfFile(void);
static void SetLastPageInRecords(void);
static void LoadInRecords(UINT32 page);

static void SetFinanceButtonStates(void);
static INT32 GetTodaysDebits(void);
static INT32 GetYesterdaysDebits(void);


void AddTransactionToPlayersBook(UINT8 ubCode, UINT8 ubSecondCode, UINT32 uiDate, INT32 iAmount)
{
	// adds transaction to player's book(Financial List)
	// outside of the financial system(the code in this .c file), this is the only function you'll ever need

	// read in balance from file

	GetBalanceFromDisk( );
	// process the actual data


	//
	// If this transaction is for the hiring/extending of a mercs contract
	//
	if( ubCode == HIRED_MERC ||
			ubCode == IMP_PROFILE ||
			ubCode == PAYMENT_TO_NPC ||
			ubCode == EXTENDED_CONTRACT_BY_1_DAY ||
			ubCode == EXTENDED_CONTRACT_BY_1_WEEK ||
			ubCode == EXTENDED_CONTRACT_BY_2_WEEKS
		)
	{
		gMercProfiles[ ubSecondCode ].uiTotalCostToDate += -iAmount;
	}

	// clear list
	ClearFinanceList( );

	// update balance
	LaptopSaveInfo.iCurrentBalance += iAmount;

	ProcessAndEnterAFinancialRecord(ubCode, uiDate, iAmount, ubSecondCode, LaptopSaveInfo.iCurrentBalance);

	// write balance to disk
	WriteBalanceToDisk( );

	// append to end of file
	AppendFinanceToEndOfFile();

	// set number of pages
	SetLastPageInRecords( );

	if( !fInFinancialMode )
	{
		ClearFinanceList( );
	}
	else
	{
		SetFinanceButtonStates( );

		// force update
		fPausedReDrawScreenFlag = TRUE;
	}

	fMapScreenBottomDirty = TRUE;
}


INT32 GetCurrentBalance( void )
{
	// get balance to this minute
	return ( LaptopSaveInfo.iCurrentBalance );
}


INT32 GetProjectedTotalDailyIncome( void )
{
	// return total  projected income, including what is earned today already

	// CJC: I DON'T THINK SO!
	// The point is:  PredictIncomeFromPlayerMines isn't dependant on the time of day
	// (anymore) and this would report income of 0 at midnight!
	/*
	if (GetWorldMinutesInDay() <= 0)
	{
		return ( 0 );
	}
	*/
	// look at we earned today

	// then there is how many deposits have been made, now look at how many mines we have, thier rate, amount of ore left and predict if we still
	// had these mines how much more would we get?

	return( PredictIncomeFromPlayerMines() );
}


void GameInitFinances()
{
	// initialize finances on game start up
	GCM->tempFiles()->deleteFile(FINANCES_DATA_FILE);
	GetBalanceFromDisk( );
}

void EnterFinances()
{
	//entry into finanacial system, load graphics, set variables..draw screen once
	// set the fact we are in the financial display system

	fInFinancialMode=TRUE;

	// get the balance
	GetBalanceFromDisk( );

	// set number of pages
	SetLastPageInRecords( );

	// load graphics into memory
	LoadFinances( );

	// create buttons
	CreateFinanceButtons( );

	// reset page we are on
	LoadInRecords(LaptopSaveInfo.iCurrentFinancesPage);

	RenderFinances( );
}

void ExitFinances( void )
{
	LaptopSaveInfo.iCurrentFinancesPage = iCurrentPage;


	// not in finance system anymore
	fInFinancialMode=FALSE;

	// destroy buttons
	DestroyFinanceButtons( );

	// clear out list
	ClearFinanceList( );


	// remove graphics
	RemoveFinances( );
}


static void DisplayFinancePageNumberAndDateRange(void);
static void DrawAPageOfRecords(void);
static void DrawFinanceTitleText(void);
static void DrawSummary(void);
static void RenderBackGround(void);


void RenderFinances(void)
{
	RenderBackGround();

	// if we are on the first page, draw the summary
	if(iCurrentPage==0)
		DrawSummary( );
	else
		DrawAPageOfRecords( );

	DrawFinanceTitleText( );

	DisplayFinancePageNumberAndDateRange( );

	BltVideoObject(FRAME_BUFFER, guiLaptopBACKGROUND, 0, STD_SCREEN_X + 108, STD_SCREEN_Y + 23);

	BlitTitleBarIcons(  );
}


static void LoadFinances(void)
{
	// load Finance video objects into memory

	// title bar
	guiTITLE = AddVideoObjectFromFile(LAPTOPDIR "/programtitlebar.sti");

	// top portion of the screen background
	guiTOP = AddVideoObjectFromFile(LAPTOPDIR "/financeswindow.sti");

	// black divider line - long ( 480 length)
	guiLONGLINE = AddVideoObjectFromFile(LAPTOPDIR "/divisionline480.sti");

	// the records columns
	guiLISTCOLUMNS = AddVideoObjectFromFile(LAPTOPDIR "/recordcolumns.sti");

	// black divider line - long ( 480 length)
	guiLINE = AddVideoObjectFromFile(LAPTOPDIR "/divisionline.sti");
}


static void RemoveFinances(void)
{
	// delete Finance video objects from memory
	DeleteVideoObject(guiLONGLINE);
	DeleteVideoObject(guiLINE);
	DeleteVideoObject(guiLISTCOLUMNS);
	DeleteVideoObject(guiTOP);
	DeleteVideoObject(guiTITLE);
}


static void RenderBackGround(void)
{
	// render generic background for Finance system
	BltVideoObject(FRAME_BUFFER, guiTITLE, 0, TOP_X, TOP_Y -  2);
	BltVideoObject(FRAME_BUFFER, guiTOP,   0, TOP_X, TOP_Y + 22);
}


static void DrawSummaryLines(void);
static void DrawSummaryText(void);


static void DrawSummary(void)
{
	// draw day's summary to screen
	DrawSummaryLines( );
	DrawSummaryText( );
}


static void DrawSummaryLines(void)
{
	// draw divider lines on screen
	// blit summary LINE object to screen
	BltVideoObject(FRAME_BUFFER, guiLINE, 0,DIVLINE_X, TOP_DIVLINE_Y);
	BltVideoObject(FRAME_BUFFER, guiLINE, 0,DIVLINE_X, TOP_DIVLINE_Y+2);
	//BltVideoObject(FRAME_BUFFER, guiLINE, 0,DIVLINE_X, MID_DIVLINE_Y);
	BltVideoObject(FRAME_BUFFER, guiLINE, 0,DIVLINE_X, BOT_DIVLINE_Y);
	BltVideoObject(FRAME_BUFFER, guiLINE, 0,DIVLINE_X, MID_DIVLINE_Y2);
	//BltVideoObject(FRAME_BUFFER, guiLINE, 0,DIVLINE_X, BOT_DIVLINE_Y2);
}


static void DrawRecordsBackGround(void);
static void DrawRecordsText(void);


static void DrawAPageOfRecords(void)
{
	// this procedure will draw a series of financial records to the screen

	// (re-)render background
	DrawRecordsBackGround( );

	// error check
	if(iCurrentPage==-1)
		return;


	// current page is found, render  from here
	DrawRecordsText( );
}


static void DrawRecordsBackGround(void)
{
	// proceudre will draw the background for the list of financial records
	INT32 iCounter;

	// now the columns
	for (iCounter = 6; iCounter < 35; iCounter++)
	{
		BltVideoObject(FRAME_BUFFER, guiLISTCOLUMNS, 0, TOP_X + 10, TOP_Y + 18 + iCounter * BLOCK_HEIGHT + 1);
	}

	// the divisorLines
	BltVideoObject(FRAME_BUFFER, guiLONGLINE, 0, TOP_X + 10, TOP_Y + 17 + 6 * BLOCK_HEIGHT);
	BltVideoObject(FRAME_BUFFER, guiLONGLINE, 0, TOP_X + 10, TOP_Y + 19 + 6 * BLOCK_HEIGHT);
	BltVideoObject(FRAME_BUFFER, guiLONGLINE, 0, TOP_X + 10, TOP_Y + 19 + iCounter * BLOCK_HEIGHT);

	// the header text
	DrawRecordsColumnHeadersText( );
}


static void DrawRecordsColumnHeadersText(void)
{
	// write the headers text for each column
	SetFontAttributes(FINANCE_TEXT_FONT, FONT_BLACK, NO_SHADOW);

	INT16 usX;
	INT16 usY;

	// the date header
	FindFontCenterCoordinates(RECORD_DATE_X,0,RECORD_DATE_WIDTH,0, pFinanceHeaders[0], FINANCE_TEXT_FONT,&usX, &usY);
	MPrint(usX, RECORD_HEADER_Y, pFinanceHeaders[0]);

	// debit header
	FindFontCenterCoordinates(RECORD_DEBIT_X,0,RECORD_DEBIT_WIDTH,0, pFinanceHeaders[1], FINANCE_TEXT_FONT,&usX, &usY);
	MPrint(usX, RECORD_HEADER_Y, pFinanceHeaders[1]);

	// credit header
	FindFontCenterCoordinates(RECORD_CREDIT_X,0,RECORD_CREDIT_WIDTH,0, pFinanceHeaders[2], FINANCE_TEXT_FONT,&usX, &usY);
	MPrint(usX, RECORD_HEADER_Y, pFinanceHeaders[2]);

	// balance header
	FindFontCenterCoordinates(RECORD_BALANCE_X,0,RECORD_BALANCE_WIDTH,0, pFinanceHeaders[4], FINANCE_TEXT_FONT,&usX, &usY);
	MPrint(usX, RECORD_HEADER_Y, pFinanceHeaders[4]);

	// transaction header
	FindFontCenterCoordinates(RECORD_TRANSACTION_X,0,RECORD_TRANSACTION_WIDTH,0, pFinanceHeaders[3], FINANCE_TEXT_FONT,&usX, &usY);
	MPrint(usX, RECORD_HEADER_Y, pFinanceHeaders[3]);

	SetFontShadow(DEFAULT_SHADOW);
}


static void DrawStringCentered(INT32 x, INT32 y, INT32 w, const ST::string& str)
{
	INT16 sx;
	INT16 sy;
	FindFontCenterCoordinates(x, 0, w, 0, str, FINANCE_TEXT_FONT, &sx, &sy);
	MPrint(sx, y, str);
}


static ST::string ProcessTransactionString(const FinanceUnit* pFinance);


// draws the text of the records
static void DrawRecordsText(void)
{
	SetFont(FINANCE_TEXT_FONT);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	const FinanceUnit* fu = pFinanceListHead;
	for (INT32 i = 0; i < NUM_RECORDS_PER_PAGE && fu != NULL; ++i, fu = fu->Next)
	{
		const INT32 y = 12 + RECORD_Y + i * (GetFontHeight(FINANCE_TEXT_FONT) + 6);

		SetFontForeground(FONT_BLACK);

		// get and write the date
		DrawStringCentered(RECORD_DATE_X, y, RECORD_DATE_WIDTH, ST::format("{}", fu->uiDate / (24 * 60)));

		// get and write debit/credit
		if (fu->iAmount >= 0)
		{
			// increase in asset - debit
			DrawStringCentered(RECORD_DEBIT_X, y, RECORD_DEBIT_WIDTH, SPrintMoney(fu->iAmount));
		}
		else
		{
			// decrease in asset - credit
			SetFontForeground(FONT_RED);
			DrawStringCentered(RECORD_CREDIT_X, y, RECORD_CREDIT_WIDTH, SPrintMoney(-fu->iAmount));
		}

		// the balance to this point
		INT32 balance = fu->iBalanceToDate;
		if (balance >= 0)
		{
			SetFontForeground(FONT_BLACK);
		}
		else
		{
			SetFontForeground(FONT_RED);
			balance = -balance;
		}
		DrawStringCentered(RECORD_BALANCE_X, y, RECORD_BALANCE_WIDTH, SPrintMoney(balance));

		// transaction string
		DrawStringCentered(RECORD_TRANSACTION_X, y, RECORD_TRANSACTION_WIDTH, ProcessTransactionString(fu));
	}
}


static void DrawFinanceTitleText(void)
{
	// draw the pages title
	SetFontAttributes(FINANCE_HEADER_FONT, FONT_WHITE);
	MPrint(TITLE_X, TITLE_Y, pFinanceTitle);
}


static INT32 GetPreviousDaysIncome(void);
static INT32 GetTodaysBalance(void);
static INT32 GetTodaysDaysIncome(void);
static INT32 GetTodaysOtherDeposits(void);
static INT32 GetYesterdaysOtherDeposits(void);
static ST::string SPrintMoneyNoDollarOnZero(INT32 Amount);


static void DrawSummaryText(void)
{
	INT16 usX, usY;
	ST::string pString;
	INT32 iBalance = 0;

	SetFontAttributes(FINANCE_TEXT_FONT, FONT_BLACK, NO_SHADOW);

	// draw summary text to the screen
	MPrint(TEXT_X, YESTERDAYS_INCOME,              pFinanceSummary[2]);
	MPrint(TEXT_X, YESTERDAYS_OTHER,               pFinanceSummary[3]);
	MPrint(TEXT_X, YESTERDAYS_DEBITS,              pFinanceSummary[4]);
	MPrint(TEXT_X, YESTERDAYS_BALANCE,             pFinanceSummary[5]);
	MPrint(TEXT_X, TODAYS_INCOME,                  pFinanceSummary[6]);
	MPrint(TEXT_X, TODAYS_OTHER,                   pFinanceSummary[7]);
	MPrint(TEXT_X, TODAYS_DEBITS,                  pFinanceSummary[8]);
	MPrint(TEXT_X, TODAYS_CURRENT_BALANCE,         pFinanceSummary[9]);
	MPrint(TEXT_X, TODAYS_CURRENT_FORCAST_INCOME,  pFinanceSummary[10]);
	MPrint(TEXT_X, TODAYS_CURRENT_FORCAST_BALANCE, pFinanceSummary[11]);

	// draw the actual numbers



	// yesterdays income
	pString = SPrintMoneyNoDollarOnZero(GetPreviousDaysIncome());
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, YESTERDAYS_INCOME, pString);

	SetFontForeground( FONT_BLACK );

	// yesterdays other
	pString = SPrintMoneyNoDollarOnZero(GetYesterdaysOtherDeposits());
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, YESTERDAYS_OTHER, pString);

	SetFontForeground( FONT_RED );

	// yesterdays debits
	iBalance =  GetYesterdaysDebits( );
	if( iBalance < 0 )
	{
		SetFontForeground( FONT_RED );
		iBalance *= -1;
	}

	pString = SPrintMoneyNoDollarOnZero(iBalance);
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, YESTERDAYS_DEBITS, pString);

	SetFontForeground( FONT_BLACK );

	// yesterdays balance..ending balance..so todays balance then
	iBalance =  GetTodaysBalance( );

	if( iBalance < 0 )
	{
		SetFontForeground( FONT_RED );
		iBalance *= -1;
	}

	pString = SPrintMoneyNoDollarOnZero(iBalance);
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, YESTERDAYS_BALANCE, pString);

	SetFontForeground( FONT_BLACK );

	// todays income
	pString = SPrintMoneyNoDollarOnZero(GetTodaysDaysIncome());
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, TODAYS_INCOME, pString);

	SetFontForeground( FONT_BLACK );

	// todays other
	pString = SPrintMoneyNoDollarOnZero(GetTodaysOtherDeposits());
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, TODAYS_OTHER, pString);

	SetFontForeground( FONT_RED );

	// todays debits
	iBalance =  GetTodaysDebits( );

	// absolute value
	if( iBalance < 0 )
	{
		iBalance *= ( -1 );
	}

	pString = SPrintMoneyNoDollarOnZero(iBalance);
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, TODAYS_DEBITS, pString);

	SetFontForeground( FONT_BLACK );

	// todays current balance
	iBalance = GetCurrentBalance( );
	if( iBalance < 0 )
	{
		iBalance *= -1;
		SetFontForeground( FONT_RED );
	}

	pString = SPrintMoneyNoDollarOnZero(iBalance);
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, TODAYS_CURRENT_BALANCE, pString);

	SetFontForeground( FONT_BLACK );

	// todays forcast income
	pString = SPrintMoneyNoDollarOnZero(GetProjectedTotalDailyIncome());
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, TODAYS_CURRENT_FORCAST_INCOME, pString);

	SetFontForeground( FONT_BLACK );


	// todays forcast balance
	iBalance = GetCurrentBalance( ) + GetProjectedTotalDailyIncome( );
	if( iBalance < 0 )
	{
		iBalance *= -1;
		SetFontForeground( FONT_RED );
	}

	pString = SPrintMoneyNoDollarOnZero(iBalance);
	FindFontRightCoordinates(STD_SCREEN_X, 0, 580, 0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	MPrint(usX, TODAYS_CURRENT_FORCAST_BALANCE, pString);

	SetFontForeground( FONT_BLACK );

	// reset the shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void ClearFinanceList(void)
{
	// remove each element from list of transactions
	FinanceUnit* pFinanceList = pFinanceListHead;
	FinanceUnit* pFinanceNode = pFinanceList;

	// while there are elements in the list left, delete them
	while( pFinanceList )
	{
		// set node to list head
		pFinanceNode=pFinanceList;

		// set list head to next node
		pFinanceList=pFinanceList->Next;

		// delete current node
		delete pFinanceNode;
	}
	pFinanceListHead = NULL;
}


static void ProcessAndEnterAFinancialRecord(const UINT8 ubCode, const UINT32 uiDate, const INT32 iAmount, const UINT8 ubSecondCode, const INT32 iBalanceToDate)
{
	FinanceUnit* const fu = new FinanceUnit{};
	fu->Next           = NULL;
	fu->ubCode         = ubCode;
	fu->ubSecondCode   = ubSecondCode;
	fu->uiDate         = uiDate;
	fu->iAmount        = iAmount;
	fu->iBalanceToDate = iBalanceToDate;

	// Append to end of list
	FinanceUnit** i = &pFinanceListHead;
	while (*i != NULL) i = &(*i)->Next;
	*i = fu;
}


static void LoadPreviousPage(void);
static void LoadNextPage(void);


static void ScrollRegionCallback(MOUSE_REGION* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		LoadPreviousPage();
	}
	else if (reason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		LoadNextPage();
	}
}


static void BtnFinanceDisplayPrevPageCallBack(GUI_BUTTON* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		LoadPreviousPage();
	}
}


static void BtnFinanceDisplayNextPageCallBack(GUI_BUTTON* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		LoadNextPage();
	}
}


static void BtnFinanceFirstPageCallBack(GUI_BUTTON* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		LoadInRecords(0);
	}
}


static void BtnFinanceLastPageCallBack(GUI_BUTTON* const, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_POINTER_UP)
	{
		LoadInRecords(guiLastPageInRecordsList + 1);
	}
}


static void MakeButton(size_t const idx, BUTTON_PICS* const img, INT16 const x, GUI_CALLBACK const callback)
{
	giFinanceButtonImage[idx] = img;
	GUIButtonRef const btn    = QuickCreateButton(img, x, BTN_Y, MSYS_PRIORITY_HIGHEST - 1, callback);
	giFinanceButton[idx]      = btn;
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
}


static void CreateFinanceButtons(void)
{
	BUTTON_PICS* const img = LoadButtonImage(LAPTOPDIR "/arrows.sti", 0, 1);
	MakeButton(PREV_PAGE_BUTTON,  img,                              PREV_BTN_X,   BtnFinanceDisplayPrevPageCallBack);
	MakeButton(NEXT_PAGE_BUTTON,  UseLoadedButtonImage(img, 6,  7), NEXT_BTN_X,   BtnFinanceDisplayNextPageCallBack);
	MakeButton(FIRST_PAGE_BUTTON, UseLoadedButtonImage(img, 3,  4), FIRST_PAGE_X, BtnFinanceFirstPageCallBack);
	MakeButton(LAST_PAGE_BUTTON,  UseLoadedButtonImage(img, 9, 10), LAST_PAGE_X,  BtnFinanceLastPageCallBack);

	UINT16 const x = TOP_X +  8;
	UINT16 const y = TOP_Y + 53;
	UINT16 const w = 482;
	UINT16 const h = 354;
	MSYS_DefineRegion(&g_scroll_region, x, y, x + w, y + h, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, ScrollRegionCallback);
}


static void DestroyFinanceButtons(void)
{
	MSYS_RemoveRegion(&g_scroll_region);
	for (UINT32 i = 0; i != 4; ++i)
	{
		RemoveButton(giFinanceButton[i]);
		UnloadButtonImage(giFinanceButtonImage[i]);
	}
}


static ST::string ProcessTransactionString(const FinanceUnit* f)
{
	UINT8 code = f->ubCode;
	switch (code)
	{
		case DEPOSIT_FROM_SILVER_MINE:
			code = DEPOSIT_FROM_GOLD_MINE;
			/* FALLTHROUGH */

		case ACCRUED_INTEREST:
		case ANONYMOUS_DEPOSIT:
		case BOBBYR_PURCHASE:
		case DEPOSIT_FROM_GOLD_MINE:
		case IMP_PROFILE:
		case PAY_SPECK_FOR_MERC:
		case PURCHASED_FLOWERS:
		case TRANSACTION_FEE:
			return pTransactionText[code];

		case CANCELLED_INSURANCE:
		case EXTENDED_CONTRACT_BY_1_DAY:
		case EXTENDED_CONTRACT_BY_1_WEEK:
		case EXTENDED_CONTRACT_BY_2_WEEKS:
		case EXTENDED_INSURANCE:
		case FULL_MEDICAL_REFUND:
		case HIRED_MERC:
		case INSURANCE_PAYOUT:
		case MEDICAL_DEPOSIT:
		case MERC_DEPOSITED_MONEY_TO_PLAYER_ACCOUNT:
		case NO_MEDICAL_REFUND:
		case PARTIAL_MEDICAL_REFUND:
		case PAYMENT_TO_NPC:
		case PURCHASED_INSURANCE:
		case PURCHASED_ITEM_FROM_DEALER:
		case REDUCED_INSURANCE:
		case TRANSFER_FUNDS_FROM_MERC:
		case TRANSFER_FUNDS_TO_MERC:
			return st_format_printf(pTransactionText[code], GetProfile(f->ubSecondCode).zNickname);

		case TRAIN_TOWN_MILITIA:
		{
			return st_format_printf(pTransactionText[TRAIN_TOWN_MILITIA], GetSectorIDString(f->ubSecondCode, TRUE));
		}

		default:
			return ST::null;
	}
}


static void DisplayFinancePageNumberAndDateRange(void)
{
	SetFontAttributes(FINANCE_TEXT_FONT, FONT_BLACK, NO_SHADOW);
	MPrint(PAGE_NUMBER_X, PAGE_NUMBER_Y, ST::format("{} {} / {}", pFinanceHeaders[5], iCurrentPage + 1, guiLastPageInRecordsList + 2));
	SetFontShadow(DEFAULT_SHADOW);
}


static void WriteBalanceToDisk(void)
{
	// will write the current balance to disk
	AutoSGPFile hFileHandle(GCM->tempFiles()->openForWriting(FINANCES_DATA_FILE, false));
	hFileHandle->write(&LaptopSaveInfo.iCurrentBalance, sizeof(INT32));
}


// will grab the current blanace from disk
// this procedure will open and read in data to the finance list
static void GetBalanceFromDisk(void)
{
	if (!GCM->tempFiles()->exists(FINANCES_DATA_FILE)) {
		LaptopSaveInfo.iCurrentBalance = 0;
		return;
	}
	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));
	// get balance from disk first
	f->read(&LaptopSaveInfo.iCurrentBalance, sizeof(INT32));
}


// will write the current finance to disk
static void AppendFinanceToEndOfFile(void)
{
	AutoSGPFile f(GCM->tempFiles()->openForAppend(FINANCES_DATA_FILE));

	const FinanceUnit* const fu = pFinanceListHead;
	BYTE  data[FINANCE_RECORD_SIZE];
	DataWriter d{data};
	INJ_U8(d, fu->ubCode);
	INJ_U8(d, fu->ubSecondCode);
	INJ_U32(d, fu->uiDate);
	INJ_I32(d, fu->iAmount);
	INJ_I32(d, fu->iBalanceToDate);
	Assert(d.getConsumed() == lengthof(data));

	f->write(data, sizeof(data));
}


// Grabs the size of the file and interprets number of pages it will take up
static void SetLastPageInRecords(void)
{
	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	const UINT32 size = f->size();

	if (size < FINANCE_HEADER_SIZE + FINANCE_RECORD_SIZE)
	{
		guiLastPageInRecordsList = 0;
		return;
	}

	guiLastPageInRecordsList =
		(size - FINANCE_HEADER_SIZE - FINANCE_RECORD_SIZE) /
		(FINANCE_RECORD_SIZE * NUM_RECORDS_PER_PAGE);
}


static void LoadPreviousPage(void)
{
	if (iCurrentPage == 0) return;
	LoadInRecords(iCurrentPage - 1);
}


static void LoadNextPage(void)
{
	if (static_cast<UINT32>(iCurrentPage) > guiLastPageInRecordsList) return;
	LoadInRecords(iCurrentPage + 1);
}


// Loads in records belonging to page
static void LoadInRecords(UINT32 const page)
{
	iCurrentPage      = page;
	fReDrawScreenFlag = TRUE;
	SetFinanceButtonStates();
	ClearFinanceList();
	if (page == 0) return; // check if bad page

	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	UINT32 const size = f->size();
	if (size < FINANCE_HEADER_SIZE) return;

	UINT32       records      = (size - FINANCE_HEADER_SIZE) / FINANCE_RECORD_SIZE;
	UINT32 const skip_records = NUM_RECORDS_PER_PAGE * (page - 1);
	if (records <= skip_records) return;

	records -= skip_records;
	f->seek(FINANCE_HEADER_SIZE + FINANCE_RECORD_SIZE * skip_records, FILE_SEEK_FROM_START);

	if (records > NUM_RECORDS_PER_PAGE) records = NUM_RECORDS_PER_PAGE;
	for (; records > 0; --records)
	{
		BYTE data[FINANCE_RECORD_SIZE];
		f->read(data, sizeof(data));

		UINT8  code;
		UINT8  second_code;
		UINT32 date;
		INT32  amount;
		INT32  balance_to_date;
		DataReader d{data};
		EXTR_U8(d, code);
		EXTR_U8(d, second_code);
		EXTR_U32(d, date);
		EXTR_I32(d, amount);
		EXTR_I32(d, balance_to_date);
		Assert(d.getConsumed() == lengthof(data));

		ProcessAndEnterAFinancialRecord(code, date, amount, second_code, balance_to_date);
	}
}


static ST::string InternalSPrintMoney(bool dollar, INT32 amount)
{
	ST::utf32_buffer codepoints = ST::format("{}", amount).to_utf32();
	size_t start = amount < 0 ? 1 : 0;
	size_t end = codepoints.size();
	ST::string money;
	if (dollar)
	{
		money += U'$';
	}
	for (size_t i = 0; i < end; i++)
	{
		if (i > start && (end - i) % 3 == 0)
		{
			money += U',';
		}
		money += codepoints[i];
	}
	return money;
}


ST::string SPrintMoney(INT32 amount)
{
	return InternalSPrintMoney(true, amount);
}


static ST::string SPrintMoneyNoDollarOnZero(INT32 amount)
{
	return InternalSPrintMoney(amount != 0, amount);
}


// find out what today is, then go back 2 days, get balance for that day
static INT32 GetPreviousDaysBalance(void)
{
	const UINT32 date_in_minutes = GetWorldTotalMin() - 60 * 24;
	const UINT32 date_in_days    = date_in_minutes / (24 * 60);

	if (date_in_days < 2) return 0;

	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	INT32 balance = 0;
	// start at the end, move back until Date / 24 * 60 on the record equals date_in_days - 2
	// loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	for (UINT32 pos = f->size(); pos >= FINANCE_HEADER_SIZE + RECORD_SIZE;)
	{
		f->seek(pos -= RECORD_SIZE, FILE_SEEK_FROM_START);

		BYTE data[RECORD_SIZE];
		f->read(data, sizeof(data));

		UINT32 date;
		INT32 balance_to_date;
		DataReader d{data};
		EXTR_SKIP(d, 2);
		EXTR_U32(d, date);
		EXTR_SKIP(d, 4);
		EXTR_I32(d, balance_to_date);
		Assert(d.getConsumed() == lengthof(data));

		// check to see if we are far enough
		if (date / (24 * 60) == date_in_days - 2)
		{
			balance = balance_to_date;
			break;
		}

		// there are no entries for the previous day
		if (date / (24 * 60) < date_in_days - 2) break;
	}

	return balance;
}


static INT32 GetTodaysBalance(void)
{
	const UINT32 date_in_minutes = GetWorldTotalMin();
	const UINT32 date_in_days    = date_in_minutes / (24 * 60);

	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	INT32 balance = 0;
	// loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	for (UINT32 pos = f->size(); pos >= FINANCE_HEADER_SIZE + RECORD_SIZE;)
	{
		f->seek(pos -= RECORD_SIZE, FILE_SEEK_FROM_START);

		BYTE data[RECORD_SIZE];
		f->read(data, sizeof(data));

		UINT32 date;
		INT32 balance_to_date;
		DataReader d{data};
		EXTR_SKIP(d, 2);
		EXTR_U32(d, date);
		EXTR_SKIP(d, 4);
		EXTR_I32(d, balance_to_date);
		Assert(d.getConsumed() == lengthof(data));

		// check to see if we are far enough
		if (date / (24 * 60) == date_in_days - 1)
		{
			balance = balance_to_date;
			break;
		}
	}

	return balance;
}


/* will return the income from the previous day, which is todays starting
 * balance - yesterdays starting balance */
static INT32 GetPreviousDaysIncome(void)
{
	const UINT32 date_in_minutes = GetWorldTotalMin();
	const UINT32 date_in_days    = date_in_minutes / (24 * 60);

	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	INT32 iTotalPreviousIncome = 0;
	// start at the end, move back until Date / 24 * 60 on the record is = date_in_days - 2
	// loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	BOOLEAN fOkToIncrement = FALSE;
	for (UINT32 pos = f->size(); pos >= FINANCE_HEADER_SIZE + RECORD_SIZE;)
	{
		f->seek(pos -= RECORD_SIZE, FILE_SEEK_FROM_START);

		BYTE data[RECORD_SIZE];
		f->read(data, sizeof(data));

		UINT8  code;
		UINT32 date;
		INT32  amount;
		DataReader d{data};
		EXTR_U8(d, code);
		EXTR_SKIP(d, 1);
		EXTR_U32(d, date);
		EXTR_I32(d, amount);
		EXTR_SKIP(d, 4);
		Assert(d.getConsumed() == lengthof(data));

		// now ok to increment amount
		if (date / (24 * 60) == date_in_days - 1) fOkToIncrement = TRUE;

		if (fOkToIncrement && (code == DEPOSIT_FROM_GOLD_MINE || code == DEPOSIT_FROM_SILVER_MINE))
		{
			// increment total
			iTotalPreviousIncome += amount;
		}

		// check to see if we are far enough
		if (date / (24 * 60) <= date_in_days - 2) break;
	}

	return iTotalPreviousIncome;
}


static INT32 GetTodaysDaysIncome(void)
{
	const UINT32 date_in_minutes = GetWorldTotalMin();
	const UINT32 date_in_days    = date_in_minutes / (24 * 60);

	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	INT32 iTotalIncome = 0;
	// loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	BOOLEAN fOkToIncrement = FALSE;
	for (UINT32 pos = f->size(); pos >= FINANCE_HEADER_SIZE + RECORD_SIZE;)
	{
		f->seek(pos -= RECORD_SIZE, FILE_SEEK_FROM_START);

		BYTE data[RECORD_SIZE];
		f->read(data, sizeof(data));

		UINT8  code;
		UINT32 date;
		INT32  amount;
		DataReader d{data};
		EXTR_U8(d, code);
		EXTR_SKIP(d, 1);
		EXTR_U32(d, date);
		EXTR_I32(d, amount);
		EXTR_SKIP(d, 4);
		Assert(d.getConsumed() == lengthof(data));

		// now ok to increment amount
		if (date / (24 * 60) > date_in_days - 1) fOkToIncrement = TRUE;

		if (fOkToIncrement && (code == DEPOSIT_FROM_GOLD_MINE || code == DEPOSIT_FROM_SILVER_MINE))
		{
			// increment total
			iTotalIncome += amount;
			fOkToIncrement = FALSE;
		}

		// check to see if we are far enough
		if (date / (24 * 60) == date_in_days - 1) break;
	}

	return iTotalIncome;
}


static void SetFinanceButtonStates(void)
{
	// this function will look at what page we are viewing, enable and disable buttons as needed

	bool const has_prev = iCurrentPage != 0;
	EnableButton(giFinanceButton[PREV_PAGE_BUTTON],  has_prev);
	EnableButton(giFinanceButton[FIRST_PAGE_BUTTON], has_prev);

	bool const has_next = iCurrentPage <= static_cast<INT32>(guiLastPageInRecordsList);
	EnableButton(giFinanceButton[NEXT_PAGE_BUTTON], has_next);
	EnableButton(giFinanceButton[LAST_PAGE_BUTTON], has_next);
}


// grab todays other deposits
static INT32 GetTodaysOtherDeposits(void)
{
	const UINT32 date_in_minutes = GetWorldTotalMin();
	const UINT32 date_in_days    = date_in_minutes / (24 * 60);

	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	INT32 iTotalIncome = 0;
	// loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	BOOLEAN fOkToIncrement = FALSE;
	for (UINT32 pos = f->size(); pos >= FINANCE_HEADER_SIZE + RECORD_SIZE;)
	{
		f->seek(pos -= RECORD_SIZE, FILE_SEEK_FROM_START);

		BYTE data[RECORD_SIZE];
		f->read(data, sizeof(data));

		UINT8  code;
		UINT32 date;
		INT32  amount;
		DataReader d{data};
		EXTR_U8(d, code);
		EXTR_SKIP(d, 1);
		EXTR_U32(d, date);
		EXTR_I32(d, amount);
		EXTR_SKIP(d, 4);
		Assert(d.getConsumed() == lengthof(data));

		// now ok to increment amount
		if (date / (24 * 60) > date_in_days - 1) fOkToIncrement = TRUE;

		if (fOkToIncrement &&
				(code != DEPOSIT_FROM_GOLD_MINE && code != DEPOSIT_FROM_SILVER_MINE) &&
				amount > 0)
		{
			// increment total
			iTotalIncome += amount;
			fOkToIncrement = FALSE;
		}

		// check to see if we are far enough
		if (date / (24 * 60) == date_in_days - 1) break;
	}

	return iTotalIncome;
}


static INT32 GetYesterdaysOtherDeposits(void)
{
	const UINT32 iDateInMinutes = GetWorldTotalMin();
	const UINT32 date_in_days   = iDateInMinutes / (24 * 60);

	AutoSGPFile f(GCM->tempFiles()->openForReading(FINANCES_DATA_FILE));

	INT32 iTotalPreviousIncome = 0;
	// start at the end, move back until Date / 24 * 60 on the record is =  date_in_days - 2
	// loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	BOOLEAN fOkToIncrement = FALSE;
	for (UINT32 pos = f->size(); pos >= FINANCE_HEADER_SIZE + RECORD_SIZE;)
	{
		f->seek(pos -= RECORD_SIZE, FILE_SEEK_FROM_START);

		BYTE data[RECORD_SIZE];
		f->read(data, sizeof(data));

		UINT8  code;
		UINT32 date;
		INT32  amount;
		DataReader d{data};
		EXTR_U8(d, code);
		EXTR_SKIP(d, 1);
		EXTR_U32(d, date);
		EXTR_I32(d, amount);
		EXTR_SKIP(d, 4);
		Assert(d.getConsumed() == lengthof(data));

		// now ok to increment amount
		if (date / (24 * 60) == date_in_days - 1) fOkToIncrement = TRUE;

		if (fOkToIncrement &&
				(code != DEPOSIT_FROM_GOLD_MINE && code != DEPOSIT_FROM_SILVER_MINE) &&
				amount > 0)
		{
			// increment total
			iTotalPreviousIncome += amount;
		}

		// check to see if we are far enough
		if (date / (24 * 60) <= date_in_days - 2) break;
	}

	return iTotalPreviousIncome;
}


static INT32 GetTodaysDebits(void)
{
	// return the expenses for today

	// currentbalance - todays balance - Todays income - other deposits

	return( GetCurrentBalance( ) - GetTodaysBalance( ) - GetTodaysDaysIncome( ) - GetTodaysOtherDeposits( ) );
}


static INT32 GetYesterdaysDebits(void)
{
	// return the expenses for yesterday

	return( GetTodaysBalance( ) - GetPreviousDaysBalance( ) - GetPreviousDaysIncome( ) - GetYesterdaysOtherDeposits( ) );
}
