#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "Finances.h"
	#include "Game_Clock.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "Debug.h"
	#include "WordWrap.h"
	#include "Render_Dirty.h"
	#include "Encrypted_File.h"
	#include "Cursors.h"
	#include "Soldier_Profile.h"
	#include "Text.h"
	#include "Strategic_Mines.h"
	#include "LaptopSave.h"
	#include "Campaign_Types.h"
	#include "StrategicMap.h"
	#include "Video.h"
	#include "VSurface.h"
	#include "MemMan.h"
	#include "Button_System.h"
#endif

// the global defines

// graphical positions
#define TOP_X 0+LAPTOP_SCREEN_UL_X
#define TOP_Y LAPTOP_SCREEN_UL_Y
#define BLOCK_HEIGHT 10
#define TOP_DIVLINE_Y 102
#define DIVLINE_X 130
#define MID_DIVLINE_Y 205
#define BOT_DIVLINE_Y 180
#define MID_DIVLINE_Y2 263 + 20
#define BOT_DIVLINE_Y2 MID_DIVLINE_Y2 + MID_DIVLINE_Y - BOT_DIVLINE_Y
#define TITLE_X 140
#define TITLE_Y 33
#define TEXT_X 140
#define PAGE_SIZE 17

// yesterdyas/todays income and balance text positions
#define YESTERDAYS_INCOME 114
#define YESTERDAYS_OTHER 138
#define YESTERDAYS_DEBITS 162
#define YESTERDAYS_BALANCE 188
#define TODAYS_INCOME 215
#define TODAYS_OTHER 239
#define TODAYS_DEBITS  263
#define TODAYS_CURRENT_BALANCE  263 + 28
#define TODAYS_CURRENT_FORCAST_INCOME  330
#define TODAYS_CURRENT_FORCAST_BALANCE 354
#define SUMMARY_NUMBERS_X
#define FINANCE_HEADER_FONT FONT14ARIAL
#define FINANCE_TEXT_FONT FONT12ARIAL
#define NUM_RECORDS_PER_PAGE PAGE_SIZE

// records text positions
#define RECORD_CREDIT_WIDTH 106-47
#define RECORD_DEBIT_WIDTH RECORD_CREDIT_WIDTH
#define RECORD_DATE_X TOP_X+10
#define RECORD_TRANSACTION_X RECORD_DATE_X+RECORD_DATE_WIDTH
#define RECORD_TRANSACTION_WIDTH 500-280
#define RECORD_DEBIT_X RECORD_TRANSACTION_X+RECORD_TRANSACTION_WIDTH
#define RECORD_CREDIT_X RECORD_DEBIT_X+RECORD_DEBIT_WIDTH
#define RECORD_Y 107-10
#define RECORD_DATE_WIDTH 47
#define RECORD_BALANCE_X RECORD_DATE_X+385
#define RECORD_BALANCE_WIDTH 479-385
#define RECORD_HEADER_Y 90


#define PAGE_NUMBER_X				TOP_X+297 //345
#define PAGE_NUMBER_Y				TOP_Y+33


// BUTTON defines
enum{
	PREV_PAGE_BUTTON=0,
  NEXT_PAGE_BUTTON,
	FIRST_PAGE_BUTTON,
	LAST_PAGE_BUTTON,
};


// button positions

#define	FIRST_PAGE_X		505
#define NEXT_BTN_X			553//577
#define PREV_BTN_X			529//553
#define	LAST_PAGE_X			577
#define BTN_Y 53



// sizeof one record
#define RECORD_SIZE ( sizeof( UINT32 ) + sizeof( INT32 ) + sizeof( INT32 ) + sizeof( UINT8 ) + sizeof( UINT8 ) )




// the financial record list
FinanceUnitPtr pFinanceListHead=NULL;

// current players balance
//INT32 iCurrentBalance=0;

// current page displayed
INT32 iCurrentPage=0;

// current financial record (the one at the top of the current page)
FinanceUnitPtr pCurrentFinance=NULL;

// video object id's
UINT32 guiTITLE;
UINT32 guiGREYFRAME;
UINT32 guiTOP;
UINT32 guiMIDDLE;
UINT32 guiBOTTOM;
UINT32 guiLINE;
UINT32 guiLONGLINE;
UINT32 guiLISTCOLUMNS;

// are in the financial system right now?
BOOLEAN fInFinancialMode=FALSE;
extern BOOLEAN fMapScreenBottomDirty;


// the last page loaded
UINT32 guiLastPageLoaded = 0;

// the last page altogether
UINT32 guiLastPageInRecordsList = 0;

// finance screen buttons
INT32 giFinanceButton[4];
INT32 giFinanceButtonImage[4];

// internal functions
UINT32 ProcessAndEnterAFinacialRecord( UINT8 ubCode, UINT32 uiDate, INT32 iAmount, UINT8 ubSecondCode, INT32 iBalanceToDate);
void RenderBackGround( void );
BOOLEAN LoadFinances();
void DrawSummary( void );
void DrawSummaryLines( void );
void DrawFinanceTitleText( void );
void InvalidateLapTopScreen( void );
void RemoveFinances( void );
void DrawSummaryText( void );
INT32 GetCurrentBalance( void );
void ClearFinanceList( void );
void OpenAndReadFinancesFile( void );
void DrawAPageOfRecords( void );
void DrawRecordsBackGround( void );
void DrawRecordsText( void );
void DrawRecordsColumnHeadersText( void );
void BtnFinanceDisplayNextPageCallBack(GUI_BUTTON *btn,INT32 reason);
void BtnFinanceFirstLastPageCallBack(GUI_BUTTON *btn,INT32 reason);
void BtnFinanceDisplayPrevPageCallBack(GUI_BUTTON *btn,INT32 reason);
void CreateFinanceButtons( void );
void DestroyFinanceButtons( void );
void IncrementCurrentPageFinancialDisplay( void );
void ProcessTransactionString(STR16 pString, size_t Length, FinanceUnitPtr pFinance);
void DisplayFinancePageNumberAndDateRange( void );
void GetBalanceFromDisk( void );
BOOLEAN WriteBalanceToDisk( void );
BOOLEAN AppendFinanceToEndOfFile( FinanceUnitPtr pFinance );
UINT32 ReadInLastElementOfFinanceListAndReturnIdNumber( void );
void SetLastPageInRecords( void );
BOOLEAN LoadInRecords( UINT32 uiPage );
BOOLEAN LoadPreviousPage( void );
BOOLEAN LoadNextPage( void );

INT32 GetPreviousBalanceToDate( void );
INT32 GetPreviousDaysIncome( void );
INT32 GetPreviousDaysBalance( void );

void SetFinanceButtonStates( void );
INT32 GetTodaysBalance( void );
INT32 GetTodaysDebits( void );
INT32 GetYesterdaysOtherDeposits( void );
INT32 GetTodaysOtherDeposits( void );
INT32 GetYesterdaysDebits( void );


UINT32 AddTransactionToPlayersBook (UINT8 ubCode, UINT8 ubSecondCode, UINT32 uiDate, INT32 iAmount)
{
	// adds transaction to player's book(Financial List), returns unique id number of it
	// outside of the financial system(the code in this .c file), this is the only function you'll ever need

	INT32 iCurPage = iCurrentPage;
	UINT32 uiId=0;
  FinanceUnitPtr pFinance=pFinanceListHead;

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

	pFinance=pFinanceListHead;

	// update balance
	LaptopSaveInfo.iCurrentBalance += iAmount;

	uiId = ProcessAndEnterAFinacialRecord(ubCode, uiDate, iAmount, ubSecondCode, LaptopSaveInfo.iCurrentBalance);

	// write balance to disk
  WriteBalanceToDisk( );

  // append to end of file
  AppendFinanceToEndOfFile( pFinance );

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

	// return unique id of this transaction
	return uiId;
}

FinanceUnitPtr GetFinance(UINT32 uiId)
{
 FinanceUnitPtr pFinance=pFinanceListHead;

 // get a finance object and return a pointer to it, the obtaining of the
 // finance object is via a unique ID the programmer must store
 // , it is returned on addition of a financial transaction

 // error check
 if(!pFinance)
	 return ( NULL );

 // look for finance object with Id
 while(pFinance)
 {
	 if(pFinance->uiIdNumber == uiId)
		 break;

	 // next finance record
	 pFinance = pFinance->Next;
 }

 return (pFinance);
}

UINT32 GetTotalDebits()
{
	// returns the total of the debits
	UINT32 uiDebits=0;
  FinanceUnitPtr pFinance=pFinanceListHead;

	// run to end of list
	while(pFinance)
	{
		// if a debit, add to debit total
		if(pFinance->iAmount > 0)
			uiDebits+=( (UINT32) (pFinance->iAmount));

		// next finance record
		pFinance=pFinance->Next;
	}

	return uiDebits;
}

UINT32 GetTotalCredits()
{
 	// returns the total of the credits
	UINT32 uiCredits = 0;
  FinanceUnitPtr pFinance=pFinanceListHead;

	// run to end of list
	while( pFinance )
	{
		// if a credit, add to credit total
		if( pFinance->iAmount < 0 )
			uiCredits += ( (UINT32) ( pFinance->iAmount ));

		// next finance record
		pFinance = pFinance->Next;
	}

	return uiCredits;
}

UINT32 GetDayCredits(UINT32 usDayNumber)
{
  // returns the total of the credits for day( note resolution of usDayNumber is days)
	UINT32 uiCredits = 0;
  FinanceUnitPtr pFinance = pFinanceListHead;

	while( pFinance )
	{
		// if a credit and it occurs on day passed
		if(( pFinance->iAmount < 0)&&( (pFinance->uiDate / (60*24)) ==usDayNumber ))
			uiCredits+=((UINT32)(pFinance->iAmount));

		// next finance record
		pFinance=pFinance->Next;
	}

	return uiCredits;
}

UINT32 GetDayDebits(UINT32 usDayNumber)
{
	// returns the total of the debits
	UINT32 uiDebits=0;
  FinanceUnitPtr pFinance=pFinanceListHead;

	while(pFinance)
	{
		if(( pFinance->iAmount > 0 )&&( (pFinance->uiDate / (60*24) ) ==usDayNumber ) )
			uiDebits += ( (UINT32) (pFinance->iAmount));

    // next finance record
		pFinance=pFinance->Next;
	}

	return uiDebits;
}

INT32 GetTotalToDay( INT32 sTimeInMins )
{
	// gets the total amount to this day
  UINT32 uiTotal = 0;
  FinanceUnitPtr pFinance = pFinanceListHead;

	while(pFinance)
	{
		if(((INT32)( pFinance->uiDate / (60*24)) <= sTimeInMins/(24*60) ))
			uiTotal += ((UINT32)(pFinance->iAmount));

    // next finance record
		pFinance=pFinance->Next;
	}

	return uiTotal;
}
INT32 GetYesterdaysIncome( void )
{
	// get income for yesterday
	return ( GetDayDebits(( ( GetWorldTotalMin() - (24*60) ) / (24*60) )) + GetDayCredits(( (UINT32) ( GetWorldTotalMin() -(24*60) )/ (24*60) )));
}

INT32 GetCurrentBalance( void )
{
	// get balance to this minute
	return ( LaptopSaveInfo.iCurrentBalance );

	// return(GetTotalDebits((GetWorldTotalMin()))+GetTotalCredits((GetWorldTotalMin())));
}

INT32 GetTodaysIncome( void )
{
 // get income
 return ( GetCurrentBalance() - GetTotalToDay( GetWorldTotalMin() - ( 24*60 ) ));
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

INT32 GetProjectedBalance( void )
{
	// return the projected balance for tommorow - total for today plus the total income, projected.
	return( GetProjectedTotalDailyIncome( ) + GetCurrentBalance( ) );
}

INT32 GetConfidenceValue()
{
  // return confidence that the projected income is infact correct
  return(( ( GetWorldMinutesInDay()*100 ) / (60*24) ));
}

void GameInitFinances()
{
  // initialize finances on game start up
	// unlink Finances data file
	if( (FileExists( FINANCES_DATA_FILE ) ) )
	{
		FileClearAttributes( FINANCES_DATA_FILE );
	  FileDelete( FINANCES_DATA_FILE );
	}
	GetBalanceFromDisk( );
}

void EnterFinances()
{
 //entry into finanacial system, load graphics, set variables..draw screen once
 // set the fact we are in the financial display system

  fInFinancialMode=TRUE;
  // build finances list
  //OpenAndReadFinancesFile( );

	// reset page we are on
	iCurrentPage = LaptopSaveInfo.iCurrentFinancesPage;


  // get the balance
  GetBalanceFromDisk( );

  // clear the list
  ClearFinanceList( );

  // force redraw of the entire screen
  fReDrawScreenFlag=TRUE;

  // set number of pages
	SetLastPageInRecords( );

  // load graphics into memory
  LoadFinances( );

  // create buttons
  CreateFinanceButtons( );

	// set button state
	SetFinanceButtonStates( );

  // draw finance
  RenderFinances( );

//  DrawSummary( );

  // draw page number
  DisplayFinancePageNumberAndDateRange( );



  //InvalidateRegion(0,0,640,480);
  return;
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
	return;

}

void HandleFinances( void )
{

}

void RenderFinances( void )
{
  HVOBJECT hHandle;

	// draw background
  RenderBackGround();

	// if we are on the first page, draw the summary
	if(iCurrentPage==0)
   DrawSummary( );
	else
   DrawAPageOfRecords( );



	//title
	DrawFinanceTitleText( );

	// draw pages and dates
  DisplayFinancePageNumberAndDateRange( );


		// display border
	GetVideoObject(&hHandle, guiLaptopBACKGROUND);
	BltVideoObject(FRAME_BUFFER, hHandle, 0,108, 23, VO_BLT_SRCTRANSPARENCY,NULL);


	// title bar icon
	BlitTitleBarIcons(  );



	return;
}

BOOLEAN LoadFinances( void )
{
  VOBJECT_DESC    VObjectDesc;
  // load Finance video objects into memory

	// title bar
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\programtitlebar.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiTITLE));

	// top portion of the screen background
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\Financeswindow.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiTOP));

  // black divider line - long ( 480 length)
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\divisionline480.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiLONGLINE));

	// the records columns
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\recordcolumns.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiLISTCOLUMNS));

  // black divider line - long ( 480 length)
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\divisionline.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiLINE));

	return (TRUE);
}

void RemoveFinances( void )
{

	// delete Finance video objects from memory
  DeleteVideoObjectFromIndex(guiLONGLINE);
	DeleteVideoObjectFromIndex(guiLINE);
  DeleteVideoObjectFromIndex(guiLISTCOLUMNS);
	DeleteVideoObjectFromIndex(guiTOP);
	DeleteVideoObjectFromIndex(guiTITLE);


	return;
}

void RenderBackGround( void )
{
	// render generic background for Finance system
  HVOBJECT hHandle;
  INT32 iCounter=0;

	// get title bar object
	GetVideoObject(&hHandle, guiTITLE);
	BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X, TOP_Y - 2, VO_BLT_SRCTRANSPARENCY,NULL);

	// get and blt the top part of the screen, video object and blt to screen
  GetVideoObject(&hHandle, guiTOP);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X, TOP_Y + 22, VO_BLT_SRCTRANSPARENCY,NULL);
	DrawFinanceTitleText( );
	return;
}




void DrawSummary( void )
{
	// draw day's summary to screen
  DrawSummaryLines( );
	DrawSummaryText( );
	DrawFinanceTitleText( );
	return;
}

void DrawSummaryLines( void )
{
	// draw divider lines on screen
  HVOBJECT hHandle;

	// the summary LINE object handle
  GetVideoObject(&hHandle, guiLINE);

	// blit summary LINE object to screen
	BltVideoObject(FRAME_BUFFER, hHandle, 0,DIVLINE_X, TOP_DIVLINE_Y, VO_BLT_SRCTRANSPARENCY,NULL);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,DIVLINE_X, TOP_DIVLINE_Y+2, VO_BLT_SRCTRANSPARENCY,NULL);
  //BltVideoObject(FRAME_BUFFER, hHandle, 0,DIVLINE_X, MID_DIVLINE_Y, VO_BLT_SRCTRANSPARENCY,NULL);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,DIVLINE_X, BOT_DIVLINE_Y, VO_BLT_SRCTRANSPARENCY,NULL);
	BltVideoObject(FRAME_BUFFER, hHandle, 0,DIVLINE_X, MID_DIVLINE_Y2, VO_BLT_SRCTRANSPARENCY,NULL);
  //BltVideoObject(FRAME_BUFFER, hHandle, 0,DIVLINE_X, BOT_DIVLINE_Y2, VO_BLT_SRCTRANSPARENCY,NULL);


	return;
}

void DrawAPageOfRecords( void )
{
	// this procedure will draw a series of financial records to the screen
  INT32 iCurPage=1;
	INT32 iCount=0;
  pCurrentFinance=pFinanceListHead;

	// (re-)render background
	DrawRecordsBackGround( );

	// error check
	if(iCurrentPage==-1)
		return;


	// current page is found, render  from here
	DrawRecordsText( );
  DisplayFinancePageNumberAndDateRange( );
	return;
}

void DrawRecordsBackGround( void )
{
	// proceudre will draw the background for the list of financial records
  INT32 iCounter=6;
  HVOBJECT hHandle;

	// render the generic background
	RenderBackGround( );


	// now the columns
	for(iCounter; iCounter <35; iCounter++)
	{
		// get and blt middle background to screen
    GetVideoObject(&hHandle, guiLISTCOLUMNS);
    BltVideoObject(FRAME_BUFFER, hHandle, 0, TOP_X + 10, TOP_Y + 18 + ( iCounter * BLOCK_HEIGHT ) + 1, VO_BLT_SRCTRANSPARENCY,NULL);
	}

	// the divisorLines
  GetVideoObject(&hHandle, guiLONGLINE);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X + 10, TOP_Y + 17 + ( 6 * ( BLOCK_HEIGHT ) ), VO_BLT_SRCTRANSPARENCY,NULL);
  GetVideoObject(&hHandle, guiLONGLINE);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X + 10, TOP_Y + 19 + ( 6 * ( BLOCK_HEIGHT ) ) , VO_BLT_SRCTRANSPARENCY,NULL);
  GetVideoObject(&hHandle, guiLONGLINE);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X + 10, TOP_Y + 19 + ( ( iCounter  ) * ( BLOCK_HEIGHT ) ) , VO_BLT_SRCTRANSPARENCY,NULL);


	// the header text
  DrawRecordsColumnHeadersText( );

	return;

}

void DrawRecordsColumnHeadersText( void )
{
  // write the headers text for each column
  UINT16 usX, usY;

	// font stuff
	SetFont(FINANCE_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	// the date header
  FindFontCenterCoordinates(RECORD_DATE_X,0,RECORD_DATE_WIDTH,0, pFinanceHeaders[0], FINANCE_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pFinanceHeaders[0]);

  // debit header
	FindFontCenterCoordinates(RECORD_DEBIT_X,0,RECORD_DEBIT_WIDTH,0, pFinanceHeaders[1], FINANCE_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pFinanceHeaders[1]);

	// credit header
	FindFontCenterCoordinates(RECORD_CREDIT_X,0,RECORD_CREDIT_WIDTH,0, pFinanceHeaders[2], FINANCE_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pFinanceHeaders[2]);

	// balance header
  FindFontCenterCoordinates(RECORD_BALANCE_X,0,RECORD_BALANCE_WIDTH,0, pFinanceHeaders[4], FINANCE_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pFinanceHeaders[4]);

	// transaction header
  FindFontCenterCoordinates(RECORD_TRANSACTION_X,0,RECORD_TRANSACTION_WIDTH,0, pFinanceHeaders[3], FINANCE_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pFinanceHeaders[3]);

	SetFontShadow(DEFAULT_SHADOW);
	return;
}

void DrawRecordsText( void )
{
  // draws the text of the records
  FinanceUnitPtr pCurFinance=pCurrentFinance;
  FinanceUnitPtr pTempFinance=pFinanceListHead;
	wchar_t sString[512];
  INT32 iCounter=0;
	UINT16 usX, usY;
  INT32 iBalance=0;

  // setup the font stuff
	SetFont(FINANCE_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);


	// anything to print
	if( pCurrentFinance == NULL )
	{
		// nothing to print
		return;
	}

	// get balance to this point
  while( pTempFinance !=pCurFinance)
	{
		// increment balance by amount of transaction
    iBalance += pTempFinance->iAmount;

		// next element
		pTempFinance = pTempFinance->Next;
	}

	// loop through record list
	for(iCounter; iCounter <NUM_RECORDS_PER_PAGE; iCounter++)
	{
		// get and write the date
		swprintf(sString, lengthof(sString), L"%d", pCurFinance->uiDate / ( 24*60 ) );



		FindFontCenterCoordinates(RECORD_DATE_X,0,RECORD_DATE_WIDTH,0, sString, FINANCE_TEXT_FONT,&usX, &usY);
		mprintf(usX, 12+RECORD_Y + ( iCounter * ( GetFontHeight( FINANCE_TEXT_FONT ) + 6 ) ), sString);

		// get and write debit/ credit
		if(pCurFinance->iAmount >=0)
		{
			// increase in asset - debit
     swprintf(sString, lengthof(sString), L"%d", pCurFinance->iAmount);
		 // insert commas
		 InsertCommasForDollarFigure( sString );
		 // insert dollar sight for first record in the list
//DEF: 3/19/99: removed cause we want to see the dollar sign on ALL entries
//		 if( iCounter == 0 )
		 {
			 InsertDollarSignInToString( sString );
		 }

		 FindFontCenterCoordinates(RECORD_DEBIT_X,0,RECORD_DEBIT_WIDTH,0, sString, FINANCE_TEXT_FONT,&usX, &usY);
		 mprintf(usX, 12+RECORD_Y + (iCounter * ( GetFontHeight( FINANCE_TEXT_FONT ) + 6 ) ), sString);
		}
		else
		{
			// decrease in asset - credit
     swprintf(sString, lengthof(sString), L"%d", pCurFinance->iAmount * (-1));
		 SetFontForeground(FONT_RED);
		 InsertCommasForDollarFigure( sString );
		 // insert dollar sight for first record in the list
//DEF: 3/19/99: removed cause we want to see the dollar sign on ALL entries
//		 if( iCounter == 0 )
		 {
			 InsertDollarSignInToString( sString );
		 }

		 FindFontCenterCoordinates(RECORD_CREDIT_X ,0 , RECORD_CREDIT_WIDTH,0, sString, FINANCE_TEXT_FONT,&usX, &usY);
		 mprintf(usX, 12+RECORD_Y + (iCounter * ( GetFontHeight( FINANCE_TEXT_FONT ) + 6 ) ), sString);
		 SetFontForeground(FONT_BLACK);
		}

		// the balance to this point
    iBalance = pCurFinance->iBalanceToDate;

		// set font based on balance
		if(iBalance >=0)
		{
      SetFontForeground(FONT_BLACK);
		}
		else
		{
      SetFontForeground(FONT_RED);
			iBalance = ( iBalance ) * ( -1 );
		}

		// transaction string
		ProcessTransactionString(sString, lengthof(sString), pCurFinance);
    FindFontCenterCoordinates(RECORD_TRANSACTION_X,0,RECORD_TRANSACTION_WIDTH,0, sString, FINANCE_TEXT_FONT,&usX, &usY);
		mprintf(usX, 12+RECORD_Y + (iCounter * ( GetFontHeight( FINANCE_TEXT_FONT ) + 6 ) ), sString);


		// print the balance string
    swprintf(sString, lengthof(sString), L"%d", iBalance);
		InsertCommasForDollarFigure( sString );
		// insert dollar sight for first record in the list
//DEF: 3/19/99: removed cause we want to see the dollar sign on ALL entries
//		if( iCounter == 0 )
		{
			InsertDollarSignInToString( sString );
		}

		FindFontCenterCoordinates(RECORD_BALANCE_X,0,RECORD_BALANCE_WIDTH,0, sString, FINANCE_TEXT_FONT,&usX, &usY);
		mprintf(usX, 12+RECORD_Y + (iCounter * ( GetFontHeight( FINANCE_TEXT_FONT ) + 6 ) ), sString);

		// restore font color
		SetFontForeground(FONT_BLACK);

		// next finance
		pCurFinance = pCurFinance->Next;

		// last page, no finances left, return
		if( ! pCurFinance )
		{

			// restore shadow
      SetFontShadow(DEFAULT_SHADOW);
			return;
		}

	}

	// restore shadow
  SetFontShadow(DEFAULT_SHADOW);
	return;
}
void DrawFinanceTitleText( void )
{
	// setup the font stuff
	SetFont(FINANCE_HEADER_FONT);
  SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);
  // reset shadow
	SetFontShadow(DEFAULT_SHADOW);

	// draw the pages title
	mprintf(TITLE_X,TITLE_Y,pFinanceTitle[0]);


	return;
}

void InvalidateLapTopScreen( void )
{
	// invalidates blit region to force refresh of screen

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_LR_Y);

	return;
}

void DrawSummaryText( void )
{
	INT16 usX, usY;
  wchar_t pString[100];
	INT32 iBalance = 0;


	// setup the font stuff
  SetFont(FINANCE_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	// draw summary text to the screen
	mprintf(TEXT_X,YESTERDAYS_INCOME,pFinanceSummary[2]);
  mprintf(TEXT_X,YESTERDAYS_OTHER,pFinanceSummary[3]);
  mprintf(TEXT_X,YESTERDAYS_DEBITS,pFinanceSummary[4]);
  mprintf(TEXT_X,YESTERDAYS_BALANCE,pFinanceSummary[5]);
  mprintf(TEXT_X,TODAYS_INCOME,pFinanceSummary[6]);
  mprintf(TEXT_X,TODAYS_OTHER,pFinanceSummary[7]);
  mprintf(TEXT_X,TODAYS_DEBITS,pFinanceSummary[8]);
	mprintf(TEXT_X,TODAYS_CURRENT_BALANCE, pFinanceSummary[9]);
	mprintf(TEXT_X,TODAYS_CURRENT_FORCAST_INCOME, pFinanceSummary[10]);
	mprintf(TEXT_X,TODAYS_CURRENT_FORCAST_BALANCE, pFinanceSummary[11]);

	// draw the actual numbers



	// yesterdays income
	iBalance =  GetPreviousDaysIncome( );
	swprintf(pString, lengthof(pString), L"%d", iBalance );

	InsertCommasForDollarFigure( pString );

	if( iBalance != 0 )
		InsertDollarSignInToString( pString );

	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,YESTERDAYS_INCOME,pString);

	SetFontForeground( FONT_BLACK );

	// yesterdays other
	iBalance =  GetYesterdaysOtherDeposits( );
	swprintf(pString, lengthof(pString), L"%d", iBalance );

	InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,YESTERDAYS_OTHER,pString);

	SetFontForeground( FONT_RED );

	// yesterdays debits
	iBalance =  GetYesterdaysDebits( );
	if( iBalance < 0 )
	{
		SetFontForeground( FONT_RED );
		iBalance *= -1;
	}

	swprintf(pString, lengthof(pString), L"%d", iBalance );

	InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,YESTERDAYS_DEBITS,pString);

	SetFontForeground( FONT_BLACK );

	// yesterdays balance..ending balance..so todays balance then
	iBalance =  GetTodaysBalance( );

	if( iBalance < 0 )
	{
		SetFontForeground( FONT_RED );
		iBalance *= -1;
	}

	swprintf(pString, lengthof(pString), L"%d", iBalance );
	InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,YESTERDAYS_BALANCE,pString);

	SetFontForeground( FONT_BLACK );

	// todays income
	iBalance =  GetTodaysDaysIncome( );
	swprintf(pString, lengthof(pString), L"%d", iBalance );

	InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,TODAYS_INCOME,pString);

	SetFontForeground( FONT_BLACK );

	// todays other
	iBalance =  GetTodaysOtherDeposits( );
	swprintf(pString, lengthof(pString), L"%d", iBalance );

	InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,TODAYS_OTHER,pString);

	SetFontForeground( FONT_RED );

	// todays debits
	iBalance =  GetTodaysDebits( );

	// absolute value
	if( iBalance < 0 )
	{
		iBalance *= ( -1 );
	}

	swprintf(pString, lengthof(pString), L"%d", iBalance );

	InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,TODAYS_DEBITS,pString);

	SetFontForeground( FONT_BLACK );

	// todays current balance
	iBalance = GetCurrentBalance( );
	if( iBalance < 0 )
	{
		iBalance *= -1;
		SetFontForeground( FONT_RED );
		swprintf(pString, lengthof(pString), L"%d", iBalance );
		iBalance *= -1;
	}
	else
	{
		swprintf(pString, lengthof(pString), L"%d", iBalance );
	}

  InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	mprintf(usX,TODAYS_CURRENT_BALANCE,pString);
  SetFontForeground( FONT_BLACK );


	// todays forcast income
	iBalance =  GetProjectedTotalDailyIncome( );
	swprintf(pString, lengthof(pString), L"%d", iBalance );

	InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);

	mprintf(usX,TODAYS_CURRENT_FORCAST_INCOME,pString);

	SetFontForeground( FONT_BLACK );


	// todays forcast balance
	iBalance = GetCurrentBalance( ) + GetProjectedTotalDailyIncome( );
	if( iBalance < 0 )
	{
		iBalance *= -1;
		SetFontForeground( FONT_RED );
		swprintf(pString, lengthof(pString), L"%d", iBalance );
		iBalance *= -1;
	}
	else
	{
		swprintf(pString, lengthof(pString), L"%d", iBalance );
	}

  InsertCommasForDollarFigure( pString );
	if( iBalance != 0 )
		InsertDollarSignInToString( pString );
	FindFontRightCoordinates(0,0,580,0,pString,FINANCE_TEXT_FONT, &usX, &usY);
	mprintf(usX,TODAYS_CURRENT_FORCAST_BALANCE,pString);
  SetFontForeground( FONT_BLACK );



	// reset the shadow
	SetFontShadow(DEFAULT_SHADOW);

	return;
}


void OpenAndReadFinancesFile( void )
{
  // this procedure will open and read in data to the finance list
  HWFILE hFileHandle;
  UINT8 ubCode, ubSecondCode;
	UINT32 uiDate;
	INT32 iAmount;
	INT32 iBalanceToDate;
  INT32 iBytesRead=0;
  UINT32 uiByteCount=0;

	// clear out the old list
	ClearFinanceList( );

	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return;
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
		return;
	}

	// read in balance
	// write balance to disk first
  FileRead( hFileHandle, &(LaptopSaveInfo.iCurrentBalance),  sizeof ( INT32 ), &iBytesRead );
	uiByteCount += sizeof( INT32 );

	AssertMsg( iBytesRead, "Failed To Read Data Entry");

	// file exists, read in data, continue until file end
  while( FileGetSize( hFileHandle ) > uiByteCount)
	{

		// read in other data
    FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		AssertMsg( iBytesRead, "Failed To Read Data Entry");

		// add transaction
	  ProcessAndEnterAFinacialRecord(ubCode, uiDate, iAmount, ubSecondCode, iBalanceToDate);

		// increment byte counter
	  uiByteCount += sizeof( INT32 ) + sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8) + sizeof( INT32 );
	}

  // close file
	FileClose( hFileHandle );

	return;
}


void ClearFinanceList( void )
{
	// remove each element from list of transactions
  FinanceUnitPtr pFinanceList=pFinanceListHead;
  FinanceUnitPtr pFinanceNode=pFinanceList;

	// while there are elements in the list left, delete them
	while( pFinanceList )
	{
    // set node to list head
		pFinanceNode=pFinanceList;

		// set list head to next node
		pFinanceList=pFinanceList->Next;

		// delete current node
		MemFree(pFinanceNode);
	}
  pCurrentFinance = NULL;
  pFinanceListHead = NULL;
	return;
}


UINT32 ProcessAndEnterAFinacialRecord( UINT8 ubCode, UINT32 uiDate, INT32 iAmount, UINT8 ubSecondCode, INT32 iBalanceToDate )
{
  UINT32 uiId = 0;
  FinanceUnitPtr pFinance=pFinanceListHead;

 	// add to finance list
	if(pFinance)
	{
		// go to end of list
		while(pFinance->Next)
			pFinance=pFinance->Next;

		// alloc space
		pFinance->Next=MemAlloc(sizeof(FinanceUnit));

		// increment id number
		uiId = pFinance->uiIdNumber + 1;

		// set up information passed
		pFinance = pFinance->Next;
		pFinance->Next = NULL;
		pFinance->ubCode = ubCode;
    pFinance->ubSecondCode = ubSecondCode;
		pFinance->uiDate = uiDate;
		pFinance->iAmount = iAmount;
    pFinance->uiIdNumber = uiId;
		pFinance->iBalanceToDate = iBalanceToDate;


	}
	else
	{
		// alloc space
		uiId = ReadInLastElementOfFinanceListAndReturnIdNumber( );
		pFinance=MemAlloc(sizeof(FinanceUnit));

		// setup info passed
		pFinance->Next = NULL;
		pFinance->ubCode = ubCode;
    pFinance->ubSecondCode = ubSecondCode;
		pFinance->uiDate = uiDate;
		pFinance->iAmount= iAmount;
    pFinance->uiIdNumber = uiId;
		pFinance->iBalanceToDate = iBalanceToDate;
	  pFinanceListHead = pFinance;
	}
  pCurrentFinance = pFinanceListHead;

	return uiId;
}

void CreateFinanceButtons( void )
{
  giFinanceButtonImage[PREV_PAGE_BUTTON] =  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,0,-1,1,-1  );
	giFinanceButton[PREV_PAGE_BUTTON] = QuickCreateButton( giFinanceButtonImage[PREV_PAGE_BUTTON], PREV_BTN_X, BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnFinanceDisplayPrevPageCallBack);


	giFinanceButtonImage[NEXT_PAGE_BUTTON]=  UseLoadedButtonImage( giFinanceButtonImage[PREV_PAGE_BUTTON] ,-1,6,-1,7,-1 );
	giFinanceButton[NEXT_PAGE_BUTTON] = QuickCreateButton( giFinanceButtonImage[NEXT_PAGE_BUTTON], NEXT_BTN_X, BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnFinanceDisplayNextPageCallBack);


	//button to go to the first page
  giFinanceButtonImage[FIRST_PAGE_BUTTON]=  UseLoadedButtonImage( giFinanceButtonImage[PREV_PAGE_BUTTON], -1,3,-1,4,-1  );
	giFinanceButton[FIRST_PAGE_BUTTON] = QuickCreateButton( giFinanceButtonImage[FIRST_PAGE_BUTTON], FIRST_PAGE_X, BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnFinanceFirstLastPageCallBack);

	MSYS_SetBtnUserData( giFinanceButton[FIRST_PAGE_BUTTON], 0, 0 );

	//button to go to the last page
  giFinanceButtonImage[LAST_PAGE_BUTTON]=  UseLoadedButtonImage( giFinanceButtonImage[PREV_PAGE_BUTTON], -1,9,-1,10,-1  );
	giFinanceButton[LAST_PAGE_BUTTON] = QuickCreateButton( giFinanceButtonImage[LAST_PAGE_BUTTON], LAST_PAGE_X, BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnFinanceFirstLastPageCallBack);
	MSYS_SetBtnUserData( giFinanceButton[LAST_PAGE_BUTTON], 0, 1 );




	SetButtonCursor(giFinanceButton[0], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giFinanceButton[1], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giFinanceButton[2], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(giFinanceButton[3], CURSOR_LAPTOP_SCREEN);
	return;
}


void DestroyFinanceButtons( void )
{
	UINT32 uiCnt;

	for( uiCnt=0; uiCnt<4; uiCnt++ )
	{
		RemoveButton( giFinanceButton[ uiCnt ] );
		UnloadButtonImage( giFinanceButtonImage[ uiCnt ] );
	}
}
void BtnFinanceDisplayPrevPageCallBack(GUI_BUTTON *btn,INT32 reason)
{

   if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	 {

		 btn->uiFlags&=~(BUTTON_CLICKED_ON);

		 // if greater than page zero, we can move back, decrement iCurrentPage counter
		 LoadPreviousPage( );
		 pCurrentFinance=pFinanceListHead;

		 // set button state
	   SetFinanceButtonStates( );
		 fReDrawScreenFlag=TRUE;
	 }

}

void BtnFinanceDisplayNextPageCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
     btn->uiFlags&=~(BUTTON_CLICKED_ON);
		 // increment currentPage
		 //IncrementCurrentPageFinancialDisplay( );
     LoadNextPage( );

		 // set button state
	   SetFinanceButtonStates( );

		 pCurrentFinance=pFinanceListHead;
		 // redraw screen
		 fReDrawScreenFlag=TRUE;
	}
}

void BtnFinanceFirstLastPageCallBack(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		UINT32	uiButton = MSYS_GetBtnUserData( btn, 0 );

		btn->uiFlags&=~(BUTTON_CLICKED_ON);

		//if its the first page button
		if( uiButton == 0 )
		{
			iCurrentPage = 0;
			LoadInRecords( iCurrentPage );
		}

		//else its the last page button
		else
		{
			LoadInRecords( guiLastPageInRecordsList + 1 );

			iCurrentPage = guiLastPageInRecordsList + 1;
		}

		// set button state
		SetFinanceButtonStates( );

		pCurrentFinance=pFinanceListHead;
		// redraw screen
		fReDrawScreenFlag=TRUE;
	}
}


void IncrementCurrentPageFinancialDisplay( void )
{
  // run through list, from pCurrentFinance, to NUM_RECORDS_PER_PAGE +1 FinancialUnits
  FinanceUnitPtr pTempFinance=pCurrentFinance;
	BOOLEAN fOkToIncrementPage=FALSE;
	INT32 iCounter=0;

	// on the overview page, simply set iCurrent to head of list, and page to 1
  if(iCurrentPage==0)
	{

		pCurrentFinance=pFinanceListHead;
		iCurrentPage=1;

		return;
	}

	// no list, we are on page 2
	if( pTempFinance == NULL )
	{
		iCurrentPage = 2;
		return;
	}

	// haven't reached end of list and not yet at beginning of next page
	while( ( pTempFinance )&&( ! fOkToIncrementPage ) )
	{
    // found the next page,  first record thereof
		if(iCounter==NUM_RECORDS_PER_PAGE+1)
		{
			fOkToIncrementPage=TRUE;
		  pCurrentFinance=pTempFinance->Next;
		}

		//next record
		pTempFinance=pTempFinance->Next;
    iCounter++;
	}

	// if ok to increment, increment
  if(fOkToIncrementPage)
	{
		iCurrentPage++;

	}

	return;
}

void ProcessTransactionString(STR16 pString, size_t Length, FinanceUnitPtr pFinance)
{

	switch( pFinance->ubCode)
	{
		case ACCRUED_INTEREST:
			swprintf(pString, Length, L"%s", pTransactionText[ACCRUED_INTEREST]);
			break;

		case ANONYMOUS_DEPOSIT:
			swprintf(pString, Length, L"%s", pTransactionText[ANONYMOUS_DEPOSIT]);
			break;

		case TRANSACTION_FEE:
			swprintf(pString, Length, L"%s", pTransactionText[TRANSACTION_FEE]);
			break;

		case HIRED_MERC:
			swprintf(pString, Length, pMessageStrings[ MSG_HIRED_MERC ], gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case BOBBYR_PURCHASE:
			swprintf(pString, Length, L"%s", pTransactionText[ BOBBYR_PURCHASE ]);
			break;

		case PAY_SPECK_FOR_MERC:
			swprintf(pString, Length, L"%s", pTransactionText[ PAY_SPECK_FOR_MERC ], gMercProfiles[pFinance->ubSecondCode].zName);
			break;

		case MEDICAL_DEPOSIT:
			swprintf(pString, Length, pTransactionText[ MEDICAL_DEPOSIT ] , gMercProfiles[pFinance->ubSecondCode].zNickname);
			break;

		case IMP_PROFILE:
			swprintf(pString, Length, L"%s", pTransactionText[ IMP_PROFILE ] );
			break;

		case PURCHASED_INSURANCE:
			swprintf(pString, Length, pTransactionText[ PURCHASED_INSURANCE ], gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case REDUCED_INSURANCE:
			swprintf(pString,  Length, pTransactionText[ REDUCED_INSURANCE ],  gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case EXTENDED_INSURANCE:
			swprintf(pString, Length, pTransactionText[ EXTENDED_INSURANCE ], gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case CANCELLED_INSURANCE:
			swprintf(pString, Length, pTransactionText[ CANCELLED_INSURANCE ], gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case INSURANCE_PAYOUT:
			swprintf(pString, Length, pTransactionText[ INSURANCE_PAYOUT ], gMercProfiles[pFinance->ubSecondCode].zNickname);
			break;

		case EXTENDED_CONTRACT_BY_1_DAY:
			swprintf(pString, Length, pTransactionAlternateText[ 1 ], gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case EXTENDED_CONTRACT_BY_1_WEEK:
			swprintf(pString, Length, pTransactionAlternateText[ 2 ], gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case EXTENDED_CONTRACT_BY_2_WEEKS:
			swprintf(pString, Length, pTransactionAlternateText[ 3 ],  gMercProfiles[pFinance->ubSecondCode].zNickname );
			break;

		case DEPOSIT_FROM_GOLD_MINE:
		case DEPOSIT_FROM_SILVER_MINE:
			swprintf(pString, Length, pTransactionText[ 16 ] );
			break;

		case PURCHASED_FLOWERS:
			swprintf(pString, Length, L"%s", pTransactionText[ PURCHASED_FLOWERS ] );
			break;

		case FULL_MEDICAL_REFUND:
			swprintf(pString, Length, pTransactionText[ FULL_MEDICAL_REFUND ], gMercProfiles[pFinance->ubSecondCode].zNickname);
			break;

		case PARTIAL_MEDICAL_REFUND:
			swprintf(pString, Length, pTransactionText[ PARTIAL_MEDICAL_REFUND ],  gMercProfiles[pFinance->ubSecondCode].zNickname);
			break;

		case NO_MEDICAL_REFUND:
			swprintf(pString, Length, pTransactionText[ NO_MEDICAL_REFUND ], gMercProfiles[pFinance->ubSecondCode].zNickname);
			break;

		case TRANSFER_FUNDS_TO_MERC:
			swprintf(pString, Length, pTransactionText[ TRANSFER_FUNDS_TO_MERC ],  gMercProfiles[pFinance->ubSecondCode].zNickname);
			break;
		case TRANSFER_FUNDS_FROM_MERC:
			swprintf(pString, Length, pTransactionText[ TRANSFER_FUNDS_FROM_MERC ], gMercProfiles[pFinance->ubSecondCode].zNickname);
			break;
		case 	PAYMENT_TO_NPC:
			swprintf(pString, Length, pTransactionText[ PAYMENT_TO_NPC ], gMercProfiles[ pFinance->ubSecondCode ].zNickname );
			break;
		case( TRAIN_TOWN_MILITIA ):
			{
				wchar_t str[ 128 ];
				UINT8 ubSectorX;
				UINT8 ubSectorY;
				ubSectorX = (UINT8)SECTORX( pFinance->ubSecondCode );
				ubSectorY = (UINT8)SECTORY( pFinance->ubSecondCode );
				GetSectorIDString( ubSectorX, ubSectorY, 0, str, lengthof(str), TRUE );
				swprintf(pString, Length, pTransactionText[ TRAIN_TOWN_MILITIA ], str );
			}
			break;

		case( PURCHASED_ITEM_FROM_DEALER ):
			swprintf(pString, Length, pTransactionText[ PURCHASED_ITEM_FROM_DEALER ],  gMercProfiles[ pFinance->ubSecondCode ].zNickname );
			break;

		case( MERC_DEPOSITED_MONEY_TO_PLAYER_ACCOUNT ):
			swprintf(pString, Length, pTransactionText[ MERC_DEPOSITED_MONEY_TO_PLAYER_ACCOUNT ],  gMercProfiles[ pFinance->ubSecondCode ].zNickname );
			break;

	}


}


void DisplayFinancePageNumberAndDateRange( void )
{
	// this function will go through the list of 'histories' starting at current until end or
	// MAX_PER_PAGE...it will get the date range and the page number
	INT32 iLastPage=0;
	INT32 iCounter=0;
  UINT32 uiLastDate;
	FinanceUnitPtr pTempFinance=pFinanceListHead;
  wchar_t sString[50];


  // setup the font stuff
	SetFont(FINANCE_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

	if( !pCurrentFinance )
	{
		pCurrentFinance = pFinanceListHead;
    if( !pCurrentFinance )
		{
     swprintf( sString, lengthof(sString), L"%s %d / %d",pFinanceHeaders[5], iCurrentPage + 1 , guiLastPageInRecordsList + 2 );
	   mprintf( PAGE_NUMBER_X, PAGE_NUMBER_Y, sString );
		 return;
		}
	}

	uiLastDate=pCurrentFinance->uiDate;
	// find last page
	while(pTempFinance)
	{
		iCounter++;
		pTempFinance=pTempFinance->Next;
	}

	// get the last page

  swprintf( sString, lengthof(sString), L"%s %d / %d",pFinanceHeaders[5], iCurrentPage + 1 , guiLastPageInRecordsList + 2 );
	mprintf( PAGE_NUMBER_X, PAGE_NUMBER_Y, sString );

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
}


BOOLEAN WriteBalanceToDisk( void )
{
	// will write the current balance to disk
  HWFILE hFileHandle;
  INT32 iBytesWritten=0;
  FinanceUnitPtr pFinanceList=pFinanceListHead;


	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE, FILE_ACCESS_WRITE|FILE_CREATE_ALWAYS, FALSE);

	// write balance to disk
  FileWrite( hFileHandle, &(LaptopSaveInfo.iCurrentBalance),  sizeof ( INT32 ), NULL );

	// close file
  FileClose( hFileHandle );


  return( TRUE );
}

void GetBalanceFromDisk( void )
{
	// will grab the current blanace from disk
	// assuming file already openned
  // this procedure will open and read in data to the finance list
  HWFILE hFileHandle;
  INT32 iBytesRead=0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		LaptopSaveInfo.iCurrentBalance = 0;
		// close file
    FileClose( hFileHandle );
		return;
  }

	// start at beginning
	FileSeek( hFileHandle, 0, FILE_SEEK_FROM_START);

	// get balance from disk first
  FileRead( hFileHandle, &(LaptopSaveInfo.iCurrentBalance),  sizeof ( INT32 ), &iBytesRead );

	AssertMsg( iBytesRead, "Failed To Read Data Entry");

	// close file
  FileClose( hFileHandle );

	return;
}


BOOLEAN AppendFinanceToEndOfFile( FinanceUnitPtr pFinance )
{
  	// will write the current finance to disk
  HWFILE hFileHandle;
  INT32 iBytesWritten=0;
  FinanceUnitPtr pFinanceList=pFinanceListHead;


	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE, FILE_ACCESS_WRITE|FILE_OPEN_ALWAYS, FALSE);

	// if no file exits, do nothing
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

    return ( FALSE );
	}

	// go to the end
	if( FileSeek( hFileHandle,0,FILE_SEEK_FROM_END ) == FALSE )
	{
		// error
    FileClose( hFileHandle );
		return( FALSE );
	}


	// write finance to disk
  // now write date and amount, and code
  FileWrite( hFileHandle, &(pFinanceList->ubCode),  sizeof ( UINT8 ), NULL );
  FileWrite( hFileHandle, &(pFinanceList->ubSecondCode),  sizeof ( UINT8 ), NULL );
  FileWrite( hFileHandle, &(pFinanceList->uiDate),  sizeof ( UINT32 ), NULL );
  FileWrite( hFileHandle, &(pFinanceList->iAmount),  sizeof ( INT32 ), NULL );
	FileWrite( hFileHandle, &(pFinanceList->iBalanceToDate),  sizeof ( INT32 ), NULL );

		// close file
  FileClose( hFileHandle );

  return( TRUE );
}

UINT32 ReadInLastElementOfFinanceListAndReturnIdNumber( void )
{
	// this function will read in the last unit in the finance list, to grab it's id number


  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  INT32 iFileSize = 0;

	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return 0;
  }

	// make sure file is more than balance size + length of 1 record - 1 byte
  if ( FileGetSize( hFileHandle ) < sizeof( INT32 ) + sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8) + sizeof( INT32 )  )
	{
    FileClose( hFileHandle );
    return 0;
	}

	// size is?
  iFileSize = FileGetSize( hFileHandle );

	// done with file, close it
	FileClose( hFileHandle );

  // file size -1 / sizeof record in bytes is id
	return ( (  iFileSize - 1 ) / ( sizeof( INT32 ) + sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8) + sizeof( INT32 )) );

}

void SetLastPageInRecords( void )
{
	// grabs the size of the file and interprets number of pages it will take up
   HWFILE hFileHandle;
  INT32 iBytesRead=0;

	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		LaptopSaveInfo.iCurrentBalance = 0;

		return;
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
    guiLastPageInRecordsList = 1;
		return;
	}


	// done with file, close it
	FileClose( hFileHandle );

	guiLastPageInRecordsList = ( ReadInLastElementOfFinanceListAndReturnIdNumber( ) - 1 )/ NUM_RECORDS_PER_PAGE;

	return;
}


BOOLEAN LoadPreviousPage( void )
{

	// clear out old list of records, and load in previous page worth of records
  ClearFinanceList( );

	// load previous page
	if( ( iCurrentPage == 1 )||( iCurrentPage == 0 ) )
	{
		iCurrentPage = 0;
		return ( FALSE );
	}

	// now load in previous page's records, if we can
  if ( LoadInRecords( iCurrentPage - 1 ) )
	{
		iCurrentPage--;
		return ( TRUE );
	}
	else
	{
    LoadInRecords( iCurrentPage );
		return ( FALSE );
	}
}

BOOLEAN LoadNextPage( void )
{

	// clear out old list of records, and load in previous page worth of records
  ClearFinanceList( );



	// now load in previous page's records, if we can
  if ( LoadInRecords( iCurrentPage + 1 ) )
	{
		iCurrentPage++;
	  return ( TRUE );
	}
	else
	{
		LoadInRecords( iCurrentPage );
	  return ( FALSE );
	}

}

BOOLEAN LoadInRecords( UINT32 uiPage )
{
	// loads in records belogning, to page uiPage
  // no file, return
	BOOLEAN fOkToContinue=TRUE;
  INT32 iCount =0;
  HWFILE hFileHandle;
  UINT8 ubCode, ubSecondCode;
	INT32 iBalanceToDate;
	UINT32 uiDate;
	INT32 iAmount;
  INT32 iBytesRead=0;
  UINT32 uiByteCount=0;

	// check if bad page
	if( uiPage == 0 )
	{
		return ( FALSE );
	}


	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return( FALSE );

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return( FALSE );
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
		return( FALSE );
	}

  // is the file long enough?
  if( ( FileGetSize( hFileHandle ) - sizeof( INT32 ) - 1) / ( NUM_RECORDS_PER_PAGE * ( sizeof( INT32 ) + sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8) + sizeof( INT32 ))  ) + 1 < uiPage )
	{
		// nope
		FileClose( hFileHandle );
    return( FALSE );
	}

	FileSeek( hFileHandle, sizeof( INT32 ) + ( uiPage - 1 ) * NUM_RECORDS_PER_PAGE * ( sizeof( INT32 )+ sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8) + sizeof( INT32)), FILE_SEEK_FROM_START );

	uiByteCount = sizeof( INT32 )+( uiPage - 1 ) * NUM_RECORDS_PER_PAGE * ( sizeof( INT32 ) + sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8)  + sizeof( INT32 ) );
	// file exists, read in data, continue until end of page
  while( ( iCount < NUM_RECORDS_PER_PAGE )&&( fOkToContinue ) &&( uiByteCount < FileGetSize( hFileHandle ) ) )
	{

		// read in data
    FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		AssertMsg( iBytesRead, "Failed To Read Data Entry");

		// add transaction
	  ProcessAndEnterAFinacialRecord(ubCode, uiDate, iAmount, ubSecondCode, iBalanceToDate);

		// increment byte counter
	  uiByteCount += sizeof( INT32 ) + sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8) + sizeof( INT32 );

		// we've overextended our welcome, and bypassed end of file, get out
		if( uiByteCount >=  FileGetSize( hFileHandle ) )
		{
			// not ok to continue
			fOkToContinue = FALSE;
		}

		iCount++;
	}

  // close file
	FileClose( hFileHandle );

	// check to see if we in fact have a list to display
  if( pFinanceListHead == NULL )
	{
		// got no records, return false
		return( FALSE );
	}

	// set up current finance
	pCurrentFinance = pFinanceListHead;

	return( TRUE );
}


void InsertCommasForDollarFigure( STR16 pString )
{
  INT16 sCounter =0;
  INT16 sZeroCount =0;
	INT16 sTempCounter = 0;
	INT16 sEndPosition = 0;

	// go to end of dollar figure
	while( pString[ sCounter ] != 0 )
	{
		sCounter++;
	}

	// negative?
	if( pString[0] == '-' )
	{
		// stop one slot in advance of normal
		sEndPosition = 1;
	}

	// is there under $1,000?
	if( sCounter < 4 )
	{
		// can't do anything, return
		return;
	}

	// at end, start backing up until beginning
  while( sCounter > sEndPosition )
	{


		// enough for a comma?
		if( sZeroCount==3 )
		{
			// reset count
			sZeroCount =0;
      // set tempcounter to current counter
			sTempCounter = sCounter;

			// run until end
			while( pString[ sTempCounter ] != 0 )
			{
				sTempCounter++;
			}
			// now shift everything over ot the right one place until sTempCounter = sCounter
			while( sTempCounter >= sCounter )
			{
				pString[ sTempCounter+1 ] = pString[ sTempCounter ];
					sTempCounter--;
			}
			// now insert comma
			pString[ sCounter ]=L',';
		}

		// increment count of digits
		sZeroCount++;

		// decrement counter
		sCounter--;
	}

	return;

}


void InsertDollarSignInToString( STR16 pString )
{
	// run to end of string, copy everything in string 2 places right, insert a space at pString[ 1 ] and a L'$' at pString[ 0 ]

	INT32 iCounter =0;

	// run to end of string
	while( pString[ iCounter ] != 0 )
	{
		iCounter++;
	}

	// now copy over
	while( iCounter >= 0 )
	{
    pString[ iCounter + 1] = pString[ iCounter ];
		iCounter--;
	}

	pString[ 0 ] = L'$';

	return;
}

INT32 GetPreviousBalanceToDate( void )
{

	// will grab balance to date of previous record
	// grabs the size of the file and interprets number of pages it will take up
   HWFILE hFileHandle;
  INT32 iBytesRead=0;
  INT32 iBalanceToDate=0;

	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return 0;
  }

  if ( FileGetSize( hFileHandle ) < sizeof( INT32 ) + sizeof( UINT32 ) + sizeof( UINT8 )+ sizeof(UINT8) + sizeof( INT32 )  )
	{
    FileClose( hFileHandle );
    return 0;
	}

  FileSeek( hFileHandle,  ( sizeof( INT32 ) ) , FILE_SEEK_FROM_END );

	// get balnce to date
  FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

	FileClose( hFileHandle );

	return iBalanceToDate;
}


INT32 GetPreviousDaysBalance( void )
{
	// find out what today is, then go back 2 days, get balance for that day
  INT32 iPreviousDaysBalance = 0;
  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  UINT32 iDateInMinutes = 0;
	BOOLEAN fOkToContinue = FALSE;
	UINT32 iByteCount = 0;
  INT32 iCounter =1;
	UINT8 ubCode;
	UINT8 ubSecondCode;
	UINT32 uiDate;
	INT32 iAmount;
	INT32 iBalanceToDate;
  BOOLEAN fGoneTooFar= FALSE;
	INT32 iFileSize = 0;

	// what day is it?
  iDateInMinutes = GetWorldTotalMin( ) - ( 60 * 24 );

	// error checking
	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return 0;
  }

  // start at the end, move back until Date / 24 * 60 on the record is =  ( iDateInMinutes /  ( 24 * 60 ) ) - 2
  iByteCount+= sizeof( INT32 );
  // loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	while( ( iByteCount < FileGetSize( hFileHandle ) ) && ( ! fOkToContinue ) && ( ! fGoneTooFar ) )
	{
		FileSeek( hFileHandle,  RECORD_SIZE * iCounter , FILE_SEEK_FROM_END );

    // incrment byte count
    iByteCount += RECORD_SIZE;

		FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		// check to see if we are far enough
		if( ( uiDate / ( 24 * 60 ) ) == ( iDateInMinutes / ( 24 * 60 ) ) - 2 )
		{
			fOkToContinue = TRUE;
		}

		if( iDateInMinutes / ( 24 * 60 ) >= 2 )
		{
			// there are no entries for the previous day
			if(  ( uiDate / ( 24 * 60 ) ) < ( iDateInMinutes / ( 24 * 60 ) ) - 2 )
			{
				fGoneTooFar = TRUE;

			}
		}
		else
		{
			fGoneTooFar = TRUE;
		}
		iCounter++;
	}

	if( fOkToContinue == FALSE )
	{
		// reached beginning of file, nothing found, return 0
    // close file
	  FileClose( hFileHandle );
		return 0;
	}

  FileClose( hFileHandle );

	// reached 3 days ago, or beginning of file
	return iBalanceToDate;

}



INT32 GetTodaysBalance( void )
{
	// find out what today is, then go back 2 days, get balance for that day
  INT32 iPreviousDaysBalance = 0;
  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  UINT32 iDateInMinutes = 0;
	BOOLEAN fOkToContinue = FALSE;
	UINT32 iByteCount = 0;
  INT32 iCounter = 1;
	UINT8 ubCode;
	UINT8 ubSecondCode;
	UINT32 uiDate;
	INT32 iAmount;
	INT32 iBalanceToDate;
  BOOLEAN fGoneTooFar= FALSE;



	// what day is it?
  iDateInMinutes = GetWorldTotalMin( );

	// error checking
	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return 0;
  }

  // start at the end, move back until Date / 24 * 60 on the record is =  ( iDateInMinutes /  ( 24 * 60 ) ) - 2
  iByteCount+= sizeof( INT32 );

  // loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	while( ( iByteCount < FileGetSize( hFileHandle ) ) && ( ! fOkToContinue ) && ( ! fGoneTooFar ) )
	{
		FileSeek( hFileHandle,  RECORD_SIZE * iCounter , FILE_SEEK_FROM_END );

    // incrment byte count
    iByteCount += RECORD_SIZE;

		FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		AssertMsg( iBytesRead, "Failed To Read Data Entry");
		// check to see if we are far enough
		if( ( uiDate / ( 24 * 60 ) ) == ( iDateInMinutes / ( 24 * 60 ) ) - 1 )
		{
			fOkToContinue = TRUE;
		}

		iCounter++;
	}


  FileClose( hFileHandle );

	// not found ?
	if( fOkToContinue == FALSE )
	{
		iBalanceToDate = 0;
	}

	// reached 3 days ago, or beginning of file
	return iBalanceToDate;
}



INT32 GetPreviousDaysIncome( void )
{
  // will return the income from the previous day
  // which is todays starting balance - yesterdays starting balance
  INT32 iPreviousDaysBalance = 0;
  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  UINT32 iDateInMinutes = 0;
	BOOLEAN fOkToContinue = FALSE;
	BOOLEAN fOkToIncrement = FALSE;
	UINT32 iByteCount = 0;
  INT32 iCounter = 1;
	UINT8 ubCode;
	UINT8 ubSecondCode;
	UINT32 uiDate;
	INT32 iAmount;
	INT32 iBalanceToDate;
  BOOLEAN fGoneTooFar = FALSE;
	INT32 iTotalPreviousIncome = 0;

	// what day is it?
  iDateInMinutes = GetWorldTotalMin( );

	// error checking
	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return 0;
  }

  // start at the end, move back until Date / 24 * 60 on the record is =  ( iDateInMinutes /  ( 24 * 60 ) ) - 2
  iByteCount+= sizeof( INT32 );

  // loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	while( ( iByteCount < FileGetSize( hFileHandle ) ) && ( ! fOkToContinue ) &&( !fGoneTooFar ) )
	{
    FileGetPos( hFileHandle );

		FileSeek( hFileHandle,  RECORD_SIZE * iCounter , FILE_SEEK_FROM_END );

    // incrment byte count
    iByteCount += RECORD_SIZE;

    FileGetPos( hFileHandle );

		FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		AssertMsg( iBytesRead, "Failed To Read Data Entry");
		// check to see if we are far enough
		if( ( uiDate / ( 24 * 60 ) )== ( iDateInMinutes / ( 24 * 60 ) ) - 2 )
		{
			fOkToContinue = TRUE;
		}

			// there are no entries for the previous day
		if(  ( uiDate / ( 24 * 60 ) ) < ( iDateInMinutes / ( 24 * 60 ) ) - 2 )
    {
			fGoneTooFar = TRUE;

		}

		if( ( uiDate / ( 24 * 60 ) ) == ( iDateInMinutes / ( 24 * 60 ) ) - 1 )
		{
			// now ok to increment amount
			fOkToIncrement = TRUE;
		}

		if( ( fOkToIncrement ) && ( ( ubCode == DEPOSIT_FROM_GOLD_MINE ) || ( ubCode == DEPOSIT_FROM_SILVER_MINE) ) )
		{
			// increment total
			iTotalPreviousIncome += iAmount;
		}

		iCounter++;
	}


	// now run back one more day and add up the total of deposits

	 // close file
	FileClose( hFileHandle );

	return( iTotalPreviousIncome );

}


INT32 GetTodaysDaysIncome( void )
{
  // will return the income from the previous day
  // which is todays starting balance - yesterdays starting balance
  INT32 iPreviousDaysBalance = 0;
  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  UINT32 iDateInMinutes = 0;
	BOOLEAN fOkToContinue = FALSE;
	BOOLEAN fOkToIncrement = FALSE;
	UINT32 iByteCount = 0;
  INT32 iCounter = 1;
	UINT8 ubCode;
	UINT8 ubSecondCode;
	UINT32 uiDate;
	INT32 iAmount;
	INT32 iBalanceToDate;
  BOOLEAN fGoneTooFar = FALSE;
	INT32 iTotalIncome = 0;

	// what day is it?
  iDateInMinutes = GetWorldTotalMin( );

	// error checking
	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
	  FileClose( hFileHandle );

		return 0;
  }

  // start at the end, move back until Date / 24 * 60 on the record is =  ( iDateInMinutes /  ( 24 * 60 ) ) - 2
  iByteCount+=sizeof( INT32 );

  // loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	while( ( iByteCount < FileGetSize( hFileHandle ) ) && ( ! fOkToContinue ) &&( !fGoneTooFar ) )
	{
		FileSeek( hFileHandle,  RECORD_SIZE * iCounter , FILE_SEEK_FROM_END );

    // incrment byte count
    iByteCount += RECORD_SIZE;

		FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		AssertMsg( iBytesRead, "Failed To Read Data Entry");
		// check to see if we are far enough
		if( ( uiDate / ( 24 * 60 ) ) == ( iDateInMinutes / ( 24 * 60 ) ) - 1 )
		{
			fOkToContinue = TRUE;
		}

		if( ( uiDate / ( 24 * 60 ) ) > ( iDateInMinutes / ( 24 * 60 ) ) - 1 )
		{
			// now ok to increment amount
			fOkToIncrement = TRUE;
		}

		if( ( fOkToIncrement ) && ( ( ubCode == DEPOSIT_FROM_GOLD_MINE ) || ( ubCode == DEPOSIT_FROM_SILVER_MINE) ) )
		{
			// increment total
			iTotalIncome += iAmount;
			fOkToIncrement = FALSE;
		}

		iCounter++;
	}

		// no entries, return nothing - no income for the day
	if( fGoneTooFar == TRUE )
	{
    FileClose( hFileHandle );
		return 0;
	}

	// now run back one more day and add up the total of deposits

	 // close file
	FileClose( hFileHandle );

	return( iTotalIncome );

}

void SetFinanceButtonStates( void )
{
	// this function will look at what page we are viewing, enable and disable buttons as needed

	if( iCurrentPage == 0 )
	{
		// first page, disable left buttons
		DisableButton( 	giFinanceButton[PREV_PAGE_BUTTON] );
		DisableButton( 	giFinanceButton[FIRST_PAGE_BUTTON] );
	}
	else
	{
		// enable buttons
		EnableButton( giFinanceButton[PREV_PAGE_BUTTON] );
		EnableButton( giFinanceButton[FIRST_PAGE_BUTTON] );
	}

	if( LoadNextPage( ) )
	{
		// decrement page
    LoadPreviousPage( );


		// enable buttons
		EnableButton( giFinanceButton[ NEXT_PAGE_BUTTON ] );
		EnableButton( giFinanceButton[ LAST_PAGE_BUTTON ] );

	}
	else
	{
    DisableButton( 	giFinanceButton[ NEXT_PAGE_BUTTON ] );
    DisableButton( 	giFinanceButton[ LAST_PAGE_BUTTON ] );
	}
}


INT32 GetTodaysOtherDeposits( void )
{
	// grab todays other deposits

  INT32 iPreviousDaysBalance = 0;
  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  UINT32 iDateInMinutes = 0;
	BOOLEAN fOkToContinue = FALSE;
	BOOLEAN fOkToIncrement = FALSE;
	UINT32 iByteCount = 0;
  INT32 iCounter = 1;
	UINT8 ubCode;
	UINT8 ubSecondCode;
	UINT32 uiDate;
	INT32 iAmount;
	INT32 iBalanceToDate;
  BOOLEAN fGoneTooFar = FALSE;
	INT32 iTotalIncome = 0;

	// what day is it?
  iDateInMinutes = GetWorldTotalMin( );

	// error checking
	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		// close file
		FileClose( hFileHandle );

		return 0;
  }

  // start at the end, move back until Date / 24 * 60 on the record is =  ( iDateInMinutes /  ( 24 * 60 ) ) - 2
  iByteCount+= sizeof( INT32 );

  // loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	while( ( iByteCount < FileGetSize( hFileHandle ) ) && ( ! fOkToContinue ) &&( !fGoneTooFar ) )
	{
		FileSeek( hFileHandle,  RECORD_SIZE * iCounter , FILE_SEEK_FROM_END );

    // incrment byte count
    iByteCount += RECORD_SIZE;

		FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		AssertMsg( iBytesRead, "Failed To Read Data Entry");
		// check to see if we are far enough
		if( ( uiDate / ( 24 * 60 ) ) == ( iDateInMinutes / ( 24 * 60 ) ) - 1 )
		{
			fOkToContinue = TRUE;
		}

		if( ( uiDate / ( 24 * 60 ) ) > ( iDateInMinutes / ( 24 * 60 ) ) - 1 )
		{
			// now ok to increment amount
			fOkToIncrement = TRUE;
		}

		if( ( fOkToIncrement ) && ( ( ubCode != DEPOSIT_FROM_GOLD_MINE ) && ( ubCode != DEPOSIT_FROM_SILVER_MINE) ) )
		{
			if( iAmount > 0 )
			{
				// increment total
				iTotalIncome += iAmount;
				fOkToIncrement = FALSE;
			}
		}

		iCounter++;
	}

		// no entries, return nothing - no income for the day
	if( fGoneTooFar == TRUE )
	{
    FileClose( hFileHandle );
		return 0;
	}

	// now run back one more day and add up the total of deposits

	 // close file
	FileClose( hFileHandle );

	return( iTotalIncome );
}


INT32 GetYesterdaysOtherDeposits( void )
{

	 INT32 iPreviousDaysBalance = 0;
  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  UINT32 iDateInMinutes = 0;
	BOOLEAN fOkToContinue = FALSE;
	BOOLEAN fOkToIncrement = FALSE;
	UINT32 iByteCount = 0;
  INT32 iCounter = 1;
	UINT8 ubCode;
	UINT8 ubSecondCode;
	UINT32 uiDate;
	INT32 iAmount;
	INT32 iBalanceToDate;
  BOOLEAN fGoneTooFar = FALSE;
	INT32 iTotalPreviousIncome = 0;

	// what day is it?
  iDateInMinutes = GetWorldTotalMin( );

	// error checking
	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		 // close file
		FileClose( hFileHandle );

		return 0;
  }

  // start at the end, move back until Date / 24 * 60 on the record is =  ( iDateInMinutes /  ( 24 * 60 ) ) - 2
  iByteCount+= sizeof( INT32 );

  // loop, make sure we don't pass beginning of file, if so, we have an error, and check for condifition above
	while( ( iByteCount < FileGetSize( hFileHandle ) ) && ( ! fOkToContinue ) &&( !fGoneTooFar ) )
	{
		FileSeek( hFileHandle,  RECORD_SIZE * iCounter , FILE_SEEK_FROM_END );

    // incrment byte count
    iByteCount += RECORD_SIZE;

		FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
	  FileRead( hFileHandle, &iAmount, sizeof(INT32), &iBytesRead );
    FileRead( hFileHandle, &iBalanceToDate, sizeof(INT32), &iBytesRead );

		AssertMsg( iBytesRead, "Failed To Read Data Entry");
		// check to see if we are far enough
		if( ( uiDate / ( 24 * 60 ) )== ( iDateInMinutes / ( 24 * 60 ) ) - 2 )
		{
			fOkToContinue = TRUE;
		}

			// there are no entries for the previous day
		if(  ( uiDate / ( 24 * 60 ) ) < ( iDateInMinutes / ( 24 * 60 ) ) - 2 )
    {
			fGoneTooFar = TRUE;

		}

		if( ( uiDate / ( 24 * 60 ) ) == ( iDateInMinutes / ( 24 * 60 ) ) - 1 )
		{
			// now ok to increment amount
			fOkToIncrement = TRUE;
		}

		if( ( fOkToIncrement ) && ( ( ubCode != DEPOSIT_FROM_GOLD_MINE ) && ( ubCode != DEPOSIT_FROM_SILVER_MINE) ) )
		{
			if( iAmount > 0 )
			{
				// increment total
				iTotalPreviousIncome += iAmount;
			}
		}

		iCounter++;
	}

	 // close file
	FileClose( hFileHandle );

	return( iTotalPreviousIncome );
}


INT32 GetTodaysDebits( void )
{
	// return the expenses for today

	// currentbalance - todays balance - Todays income - other deposits

	return( GetCurrentBalance( ) - GetTodaysBalance( ) - GetTodaysDaysIncome( ) - GetTodaysOtherDeposits( ) );
}

INT32 GetYesterdaysDebits( void )
{
	// return the expenses for yesterday

	return( GetTodaysBalance( ) - GetPreviousDaysBalance( ) - GetPreviousDaysIncome( ) - GetYesterdaysOtherDeposits( ) );
}


void LoadCurrentBalance( void )
{
	// will load the current balance from finances.dat file
	HWFILE hFileHandle;
  INT32 iBytesRead=0;

	// is the first record in the file
		// error checking
	// no file, return
	if ( ! (FileExists( FINANCES_DATA_FILE ) ) )
	{
		LaptopSaveInfo.iCurrentBalance= 0;
		return;
	}

	// open file
 	hFileHandle=FileOpen( FINANCES_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

	// failed to get file, return
	if(!hFileHandle)
	{
		LaptopSaveInfo.iCurrentBalance= 0;

		// close file
		FileClose( hFileHandle );

		return;
	}

	FileSeek( hFileHandle,  0 , FILE_SEEK_FROM_START );
	FileRead( hFileHandle, &LaptopSaveInfo.iCurrentBalance, sizeof( INT32 ), &iBytesRead );

	AssertMsg( iBytesRead, "Failed To Read Data Entry");
	 // close file
	FileClose( hFileHandle );


	return;
}
