#include "Font.h"
#include "Font_Control.h"
#include "Laptop.h"
#include "History.h"
#include "Game_Clock.h"
#include "Soldier_Control.h"
#include "VObject.h"
#include "WCheck.h"
#include "Debug.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Encrypted_File.h"
#include "Cursors.h"
#include "Soldier_Profile.h"
#include "StrategicMap.h"
#include "QuestText.h"
#include "Text.h"
#include "Message.h"
#include "LaptopSave.h"
#include "Button_System.h"
#include "VSurface.h"
#include "MemMan.h"
#include "FileMan.h"


#define HISTORY_QUEST_TEXT_SIZE 80


typedef struct HistoryUnit HistoryUnit;
struct HistoryUnit
{
	UINT8 ubCode; // the code index in the finance code table
	UINT8 ubSecondCode; // secondary code
	UINT32 uiDate; // time in the world in global time
	INT16 sSectorX; // sector X this took place in
	INT16 sSectorY; // sector Y this took place in
	INT8 bSectorZ;
	UINT8 ubColor;
	HistoryUnit* Next; // next unit in the list
};


#define TOP_X											0+LAPTOP_SCREEN_UL_X
#define TOP_Y											LAPTOP_SCREEN_UL_Y
#define BLOCK_HIST_HEIGHT					10
#define BOX_HEIGHT								14
#define TOP_DIVLINE_Y							101
#define DIVLINE_X									130
#define MID_DIVLINE_Y							155
#define BOT_DIVLINE_Y							204
#define TITLE_X										140
#define TITLE_Y										33
#define TEXT_X										140
#define PAGE_SIZE									22
#define RECORD_Y									TOP_DIVLINE_Y
#define RECORD_HISTORY_WIDTH			200
#define PAGE_NUMBER_X							TOP_X+20
#define PAGE_NUMBER_Y							TOP_Y+33
#define HISTORY_DATE_X						PAGE_NUMBER_X+85
#define HISTORY_DATE_Y						PAGE_NUMBER_Y
#define RECORD_LOCATION_WIDTH			142//95

#define HISTORY_HEADER_FONT FONT14ARIAL
#define HISTORY_TEXT_FONT FONT12ARIAL
#define RECORD_DATE_X TOP_X+10
#define RECORD_DATE_WIDTH 31//68
#define RECORD_HEADER_Y 90


#define NUM_RECORDS_PER_PAGE PAGE_SIZE
#define SIZE_OF_HISTORY_FILE_RECORD ( sizeof( UINT8 ) + sizeof( UINT8 ) + sizeof( UINT32 ) + sizeof( UINT16 ) + sizeof( UINT16 ) + sizeof( UINT8 ) + sizeof( UINT8 ) )

// button positions
#define NEXT_BTN_X 577
#define PREV_BTN_X 553
#define BTN_Y 53

// graphics handles
static SGPVObject* guiTITLE;
//UINT32 guiGREYFRAME;
static SGPVObject* guiTOP;
//UINT32 guiMIDDLE;
//UINT32 guiBOTTOM;
//UINT32 guiLINE;
static SGPVObject* guiLONGLINE;
static SGPVObject* guiSHADELINE;
//UINT32 guiVERTLINE;
//UINT32 guiBIGBOX;

enum{
	PREV_PAGE_BUTTON=0,
  NEXT_PAGE_BUTTON,
};



// the page flipping buttons
INT32 giHistoryButton[2];
BOOLEAN fInHistoryMode=FALSE;


// current page displayed
INT32 iCurrentHistoryPage=1;



// the History record list
static HistoryUnit* pHistoryListHead = NULL;

// current History record (the one at the top of the current page)
static HistoryUnit* pCurrentHistory = NULL;


// last page in list
UINT32 guiLastPageInHistoryRecordsList = 0;

void ClearHistoryList( void );


static BOOLEAN AppendHistoryToEndOfFile(void);
static BOOLEAN LoadNextHistoryPage(void);
static void ProcessAndEnterAHistoryRecord(UINT8 ubCode, UINT32 uiDate, UINT8 ubSecondCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, UINT8 ubColor);


static void InternalAddHistoryToPlayersLog(const UINT8 ubCode, const UINT8 ubSecondCode, const UINT32 uiDate, const INT16 sSectorX, const INT16 sSectorY, const UINT8 colour)
{
	ClearHistoryList();

	ProcessAndEnterAHistoryRecord(ubCode, uiDate, ubSecondCode, sSectorX, sSectorY, 0, colour);
	ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[MSG_HISTORY_UPDATED]);

	AppendHistoryToEndOfFile();

	// if in history mode, reload current page
	if (fInHistoryMode)
	{
		--iCurrentHistoryPage;
		LoadNextHistoryPage();
	}
}


void SetHistoryFact(const UINT8 ubCode, const UINT8 ubSecondCode, const UINT32 uiDate, const INT16 sSectorX, const INT16 sSectorY)
{
	const UINT8 ubColor = (ubCode == HISTORY_QUEST_FINISHED ? 0 : 1);
	InternalAddHistoryToPlayersLog(ubColor, ubSecondCode, uiDate, sSectorX, sSectorY, ubColor);
}


void AddHistoryToPlayersLog(const UINT8 ubCode, const UINT8 ubSecondCode, const UINT32 uiDate, const INT16 sSectorX, const INT16 sSectorY)
{
	InternalAddHistoryToPlayersLog(ubCode, ubSecondCode, uiDate, sSectorX, sSectorY, 0);
}


void GameInitHistory()
{
  if( ( FileExists( HISTORY_DATA_FILE ) ) )
	{
	  // unlink history file
		FileClearAttributes( HISTORY_DATA_FILE );
	  FileDelete( HISTORY_DATA_FILE );
	}

	AddHistoryToPlayersLog(HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO, 0, GetWorldTotalMin( ), -1, -1);

}


static void CreateHistoryButtons(void);
static BOOLEAN LoadHistory(void);
static void SetHistoryButtonStates(void);


void EnterHistory()
{

	// load the graphics
  LoadHistory( );

  // create History buttons
  CreateHistoryButtons( );

	// reset current to first page
	if( LaptopSaveInfo.iCurrentHistoryPage > 0 )
		iCurrentHistoryPage = LaptopSaveInfo.iCurrentHistoryPage - 1;
	else
		iCurrentHistoryPage = 0;

	// load in first page
	LoadNextHistoryPage( );


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


	// write out history list to file
	//OpenAndWriteHistoryFile( );

	// delete graphics
  RemoveHistory( );

	// delete buttons
	DestroyHistoryButtons( );

	ClearHistoryList( );
}

void HandleHistory()
{
	// DEF 2/5/99 Dont need to update EVERY FRAME!!!!
  // check and update status of buttons
//  SetHistoryButtonStates( );
}


static void DisplayHistoryListBackground(void);
static void DisplayHistoryListHeaders(void);
static void DisplayPageNumberAndDateRange(void);
static void DrawAPageofHistoryRecords(void);
static void DrawHistoryTitleText(void);
static void RenderHistoryBackGround(void);


void RenderHistory( void )
{
	 //render the background to the display
   RenderHistoryBackGround( );

	 // the title bar text
	 DrawHistoryTitleText( );

	  // the actual lists background
	 DisplayHistoryListBackground( );

	 // the headers to each column
	 DisplayHistoryListHeaders( );

	 // render the currentpage of records
	 DrawAPageofHistoryRecords( );

	 // stuff at top of page, the date range and page numbers
   DisplayPageNumberAndDateRange( );

	 // title bar icon
	 BlitTitleBarIcons(  );
}


static BOOLEAN LoadHistory(void)
{
  // load History video objects into memory

	// title bar
	guiTITLE = AddVideoObjectFromFile("LAPTOP/programtitlebar.sti");
	CHECKF(guiTITLE != NO_VOBJECT);

	// top portion of the screen background
	guiTOP = AddVideoObjectFromFile("LAPTOP/historywindow.sti");
	CHECKF(guiTOP != NO_VOBJECT);

  // shaded line
	guiSHADELINE = AddVideoObjectFromFile("LAPTOP/historylines.sti");
	CHECKF(guiSHADELINE != NO_VOBJECT);

/*
Not being used???  DF commented out
  // vert  line
	guiVERTLINE = AddVideoObjectFromFile("LAPTOP/historyvertline.sti");
	CHECKF(guiVERTLINE != NO_VOBJECT);
*/
  // black divider line - long ( 480 length)
	guiLONGLINE = AddVideoObjectFromFile("LAPTOP/divisionline480.sti");
	CHECKF(guiLONGLINE != NO_VOBJECT);

	return (TRUE);
}


static void RemoveHistory(void)
{
	// delete history video objects from memory
  DeleteVideoObject(guiLONGLINE);
	DeleteVideoObject(guiTOP);
	DeleteVideoObject(guiTITLE);
	DeleteVideoObject(guiSHADELINE);
}


static void RenderHistoryBackGround(void)
{
	// render generic background for history system
	BltVideoObject(FRAME_BUFFER, guiTITLE, 0, TOP_X, TOP_Y -  2);
	BltVideoObject(FRAME_BUFFER, guiTOP,   0, TOP_X, TOP_Y + 22);

  // display background for history list
  DisplayHistoryListBackground( );
}


static void DrawHistoryTitleText(void)
{
	// setup the font stuff
	SetFont(HISTORY_HEADER_FONT);
  SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(DEFAULT_SHADOW);

	// draw the pages title
	mprintf(TITLE_X, TITLE_Y, pHistoryTitle);
}


static void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON* btn, INT32 reason);
static void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON* btn, INT32 reason);


static void CreateHistoryButtons(void)
{
	// the prev/next page buttons
	giHistoryButton[PREV_PAGE_BUTTON] = QuickCreateButtonImg("LAPTOP/arrows.sti", -1, 0, -1, 1, -1, PREV_BTN_X, BTN_Y, MSYS_PRIORITY_HIGHEST - 1, BtnHistoryDisplayPrevPageCallBack);
	giHistoryButton[NEXT_PAGE_BUTTON] = QuickCreateButtonImg("LAPTOP/arrows.sti", -1, 6, -1, 7, -1, NEXT_BTN_X, BTN_Y, MSYS_PRIORITY_HIGHEST - 1, BtnHistoryDisplayNextPageCallBack);

	// set buttons
	SetButtonCursor(giHistoryButton[0], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(giHistoryButton[1], CURSOR_LAPTOP_SCREEN);
}


static void DestroyHistoryButtons(void)
{
	// remove History buttons and images from memory
	// next page button
	RemoveButton(giHistoryButton[1] );
	// prev page button
	RemoveButton(giHistoryButton[0] );
}


static BOOLEAN LoadPreviousHistoryPage(void);


static void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON* btn, INT32 reason)
{
	// force redraw
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fReDrawScreenFlag=TRUE;
	}


	// force redraw
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		fReDrawScreenFlag=TRUE;
		// this page is > 0, there are pages before it, decrement

		if(iCurrentHistoryPage > 0)
		{
			LoadPreviousHistoryPage( );
			//iCurrentHistoryPage--;
		  DrawAPageofHistoryRecords( );
		}

		// set new state
		SetHistoryButtonStates( );
	}


}


static void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON* btn, INT32 reason)
{

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fReDrawScreenFlag=TRUE;
	}


	// force redraw
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
    // increment currentPage
		LoadNextHistoryPage( );
		// set new state
		SetHistoryButtonStates( );
		fReDrawScreenFlag=TRUE;
	}
}


static BOOLEAN IncrementCurrentPageHistoryDisplay(void)
{
	HWFILE hFileHandle;
	UINT32	uiFileSize=0;
	UINT32  uiSizeOfRecordsOnEachPage = 0;

	// open file
	hFileHandle = FileOpen( HISTORY_DATA_FILE, FILE_ACCESS_READ);

	// failed to get file, return
	if(!hFileHandle)
	{
		return( FALSE );
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
		return( FALSE );
	}

	uiFileSize = FileGetSize( hFileHandle ) - 1;
	uiSizeOfRecordsOnEachPage = ( NUM_RECORDS_PER_PAGE * ( sizeof( UINT8 ) + sizeof( UINT32 ) + 3*sizeof( UINT8 )+ sizeof(INT16) + sizeof( INT16 ) ) );

  // is the file long enough?
//  if( ( FileGetSize( hFileHandle ) - 1 ) / ( NUM_RECORDS_PER_PAGE * ( sizeof( UINT8 ) + sizeof( UINT32 ) + 3*sizeof( UINT8 )+ sizeof(INT16) + sizeof( INT16 ) ) ) + 1 < ( UINT32 )( iCurrentHistoryPage + 1 ) )
	if( uiFileSize / uiSizeOfRecordsOnEachPage + 1 < ( UINT32 )( iCurrentHistoryPage + 1 ) )
	{
		// nope
		FileClose( hFileHandle );
    return( FALSE );
	}
	else
	{
		iCurrentHistoryPage++;
		FileClose( hFileHandle );
	}

	return( TRUE );
}


static void ProcessAndEnterAHistoryRecord(UINT8 ubCode, UINT32 uiDate, UINT8 ubSecondCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, UINT8 ubColor)
{
	HistoryUnit* pHistory = pHistoryListHead;

 	// add to History list
	if(pHistory)
	{
		// go to end of list
		while(pHistory->Next)
			pHistory=pHistory->Next;

		// alloc space
		pHistory->Next = MALLOC(HistoryUnit);

		// set up information passed
		pHistory = pHistory->Next;
		pHistory->Next = NULL;
		pHistory->ubCode = ubCode;
    pHistory->ubSecondCode = ubSecondCode;
		pHistory->uiDate = uiDate;
		pHistory->sSectorX = sSectorX;
		pHistory->sSectorY = sSectorY;
		pHistory->bSectorZ = bSectorZ;
		pHistory->ubColor = ubColor;

	}
	else
	{
		// alloc space
		pHistory = MALLOC(HistoryUnit);

		// setup info passed
		pHistory->Next = NULL;
		pHistory->ubCode = ubCode;
    pHistory->ubSecondCode = ubSecondCode;
		pHistory->uiDate = uiDate;
	  pHistoryListHead = pHistory;
		pHistory->sSectorX = sSectorX;
		pHistory->sSectorY = sSectorY;
		pHistory->bSectorZ = bSectorZ;
		pHistory->ubColor = ubColor;

	}
}


#ifdef JA2TESTVERSION
static void PerformCheckOnHistoryRecord(UINT32 uiErrorCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);
#endif


static void OpenAndReadHistoryFile(void)
{
  // this procedure will open and read in data to the History list

	HWFILE hFileHandle;
  UINT8 ubCode, ubSecondCode;
	UINT32 uiDate;
  INT16 sSectorX, sSectorY;
	INT8 bSectorZ = 0;
	UINT8 ubColor;
  UINT32 uiByteCount=0;

	// clear out the old list
	ClearHistoryList( );

	// open file
	hFileHandle = FileOpen(HISTORY_DATA_FILE, FILE_ACCESS_READ);

	// failed to get file, return
	if(!hFileHandle)
	{
		return;
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
		return;
	}

	// file exists, read in data, continue until file end
  while( FileGetSize( hFileHandle ) > uiByteCount)
	{
		// read in other data
    FileRead(hFileHandle, &ubCode,       sizeof(UINT8));
		FileRead(hFileHandle, &ubSecondCode, sizeof(UINT8));
		FileRead(hFileHandle, &uiDate,       sizeof(UINT32));
    FileRead(hFileHandle, &sSectorX,     sizeof(INT16));
    FileRead(hFileHandle, &sSectorY,     sizeof(INT16));
		FileRead(hFileHandle, &bSectorZ,     sizeof(INT8));
		FileRead(hFileHandle, &ubColor,      sizeof(UINT8));

		#ifdef JA2TESTVERSION
		//perform a check on the data to see if it is pooched
		PerformCheckOnHistoryRecord( 1, sSectorX, sSectorY, bSectorZ );
		#endif

		// add transaction
	  ProcessAndEnterAHistoryRecord( ubCode, uiDate, ubSecondCode, sSectorX, sSectorY, bSectorZ, ubColor );

		// increment byte counter
	  uiByteCount +=  SIZE_OF_HISTORY_FILE_RECORD;
	}

	FileClose( hFileHandle );
}


static BOOLEAN OpenAndWriteHistoryFile(void)
{
  // this procedure will open and write out data from the History list

	HWFILE hFileHandle;
	HistoryUnit* pHistoryList = pHistoryListHead;


	// open file
	hFileHandle = FileOpen(HISTORY_DATA_FILE, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);

	// if no file exits, do nothing
	if(!hFileHandle)
	{
		return ( FALSE );
  }
  // write info, while there are elements left in the list
  while(pHistoryList)
	{

		#ifdef JA2TESTVERSION
		//perform a check on the data to see if it is pooched
		PerformCheckOnHistoryRecord( 2, pHistoryList->sSectorX, pHistoryList->sSectorY, pHistoryList->bSectorZ );
		#endif

    	// now write date and amount, and code
		FileWrite(hFileHandle, &pHistoryList->ubCode,       sizeof(UINT8));
		FileWrite(hFileHandle, &pHistoryList->ubSecondCode, sizeof(UINT8));
		FileWrite(hFileHandle, &pHistoryList->uiDate,       sizeof(UINT32));
		FileWrite(hFileHandle, &pHistoryList->sSectorX,     sizeof(INT16));
		FileWrite(hFileHandle, &pHistoryList->sSectorY,     sizeof(INT16));
		FileWrite(hFileHandle, &pHistoryList->bSectorZ,     sizeof(INT8));
		FileWrite(hFileHandle, &pHistoryList->ubColor,      sizeof(UINT8));

		// next element in list
		pHistoryList = pHistoryList->Next;

	}

  FileClose( hFileHandle );
  // clear out the old list
	ClearHistoryList( );

	return ( TRUE );
}


void ClearHistoryList( void )
{
	// remove each element from list of transactions

	HistoryUnit* pHistoryList = pHistoryListHead;
	HistoryUnit* pHistoryNode = pHistoryList;

	// while there are elements in the list left, delete them
	while( pHistoryList )
	{
    // set node to list head
		pHistoryNode=pHistoryList;

		// set list head to next node
		pHistoryList=pHistoryList->Next;

		// delete current node
		MemFree(pHistoryNode);
	}
  pHistoryListHead=NULL;
}


static void DisplayHistoryListHeaders(void)
{
  // this procedure will display the headers to each column in History

	// font stuff
	SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

  INT16 usX;
  INT16 usY;

  // the date header
  FindFontCenterCoordinates(RECORD_DATE_X + 5,0,RECORD_DATE_WIDTH,0, pHistoryHeaders[0], HISTORY_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pHistoryHeaders[0]);

	// the date header
  FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH + 5,0,RECORD_LOCATION_WIDTH,0, pHistoryHeaders[ 3 ], HISTORY_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pHistoryHeaders[3]);

	// event header
  FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH + RECORD_LOCATION_WIDTH + 5,0,RECORD_LOCATION_WIDTH,0, pHistoryHeaders[ 3 ], HISTORY_TEXT_FONT,&usX, &usY);
	mprintf(usX, RECORD_HEADER_Y, pHistoryHeaders[4]);
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


static void ProcessHistoryTransactionString(wchar_t* pString, size_t Length, const HistoryUnit* pHistory);


static void DrawHistoryRecordsText(void)
{
  // draws the text of the records
	HistoryUnit* pCurHistory = pHistoryListHead;
	wchar_t sString[512];
	INT16 sX =0, sY =0;

  // setup the font stuff
	SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

	// error check
	if( !pCurHistory)
		return;


	// loop through record list
	for (INT32 iCounter = 0; iCounter < NUM_RECORDS_PER_PAGE; iCounter++)
	{
		if( pCurHistory->ubColor == 0 )
		{
			SetFontForeground(FONT_BLACK);
		}
		else
		{
			SetFontForeground(FONT_RED);
		}
		// get and write the date
		swprintf(sString, lengthof(sString), L"%d", ( pCurHistory->uiDate / ( 24 * 60 ) ) );
		INT16 usX;
		INT16 usY;
		FindFontCenterCoordinates(RECORD_DATE_X + 5, 0, RECORD_DATE_WIDTH,0, sString, HISTORY_TEXT_FONT,&usX, &usY);
		mprintf(usX, RECORD_Y + ( iCounter * ( BOX_HEIGHT ) ) + 3, sString);

		// now the actual history text
    //FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH,0,RECORD_HISTORY_WIDTH,0,  pHistoryStrings[pCurHistory->ubCode], HISTORY_TEXT_FONT,&usX, &usY);
		ProcessHistoryTransactionString(sString, lengthof(sString), pCurHistory);
//	mprintf(RECORD_DATE_X + RECORD_DATE_WIDTH + 25, RECORD_Y + ( iCounter * ( BOX_HEIGHT ) ) + 3, pHistoryStrings[pCurHistory->ubCode] );
		mprintf(RECORD_DATE_X + RECORD_LOCATION_WIDTH +RECORD_DATE_WIDTH + 15, RECORD_Y + ( iCounter * ( BOX_HEIGHT ) ) + 3, sString );


		// no location
		if( ( pCurHistory->sSectorX == -1 )||( pCurHistory->sSectorY == -1 ) )
		{
			FindFontCenterCoordinates(RECORD_DATE_X + RECORD_DATE_WIDTH, 0, RECORD_LOCATION_WIDTH + 10, 0, pHistoryLocations, HISTORY_TEXT_FONT, &sX, &sY);
		  mprintf(sX, RECORD_Y + iCounter * BOX_HEIGHT + 3, pHistoryLocations);
    }
		else
		{
			GetSectorIDString( pCurHistory->sSectorX, pCurHistory->sSectorY, pCurHistory->bSectorZ, sString, lengthof(sString), TRUE );
			FindFontCenterCoordinates( RECORD_DATE_X + RECORD_DATE_WIDTH, 0, RECORD_LOCATION_WIDTH + 10, 0,  sString ,HISTORY_TEXT_FONT, &sX, &sY );

			ReduceStringLength( sString, lengthof(sString), RECORD_LOCATION_WIDTH + 10, HISTORY_TEXT_FONT );

			mprintf(sX, RECORD_Y + ( iCounter * ( BOX_HEIGHT ) ) + 3, sString );
		}

		// restore font color
		SetFontForeground(FONT_BLACK);

		// next History
		pCurHistory = pCurHistory->Next;

		// last page, no Historys left, return
		if( ! pCurHistory )
		{

			// restore shadow
      SetFontShadow(DEFAULT_SHADOW);
			return;
		}

	}

	// restore shadow
  SetFontShadow(DEFAULT_SHADOW);
}


static void DrawAPageofHistoryRecords(void)
{
	// this procedure will draw a series of history records to the screen
  pCurrentHistory=pHistoryListHead;

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


static void DisplayPageNumberAndDateRange(void)
{
	// this function will go through the list of 'histories' starting at current until end or
	// MAX_PER_PAGE...it will get the date range and the page number
	INT32 iLastPage=0;
	INT32 iCounter=0;
  UINT32 uiLastDate;
	HistoryUnit* pTempHistory = pHistoryListHead;

  // setup the font stuff
	SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

	if( !pCurrentHistory )
	{
		mprintf(PAGE_NUMBER_X,  PAGE_NUMBER_Y,  L"%ls  %d / %d", pHistoryHeaders[1], 1, 1);
		mprintf(HISTORY_DATE_X, HISTORY_DATE_Y, L"%ls %d - %d",  pHistoryHeaders[2], 1, 1);

    // reset shadow
	  SetFontShadow(DEFAULT_SHADOW);

		return;
  }

	uiLastDate=pCurrentHistory->uiDate;

/*
	// find last page
	while(pTempHistory)
	{
		iCounter++;
		pTempHistory=pTempHistory->Next;
	}

  // set last page
	iLastPage=iCounter/NUM_RECORDS_PER_PAGE;
*/

	iLastPage = GetNumberOfHistoryPages();

	// set temp to current, to get last date
  pTempHistory=pCurrentHistory;

	// reset counter
	iCounter=0;

	// run through list until end or num_records, which ever first
  while((pTempHistory)&&(iCounter < NUM_RECORDS_PER_PAGE))
	{
		uiLastDate=pTempHistory->uiDate;
		iCounter++;

		pTempHistory = pTempHistory->Next;
	}



	// get the last page

	mprintf(PAGE_NUMBER_X,  PAGE_NUMBER_Y,  L"%ls  %d / %d", pHistoryHeaders[1], iCurrentHistoryPage, iLastPage + 1);
	mprintf(HISTORY_DATE_X, HISTORY_DATE_Y, L"%ls %d - %d",  pHistoryHeaders[2], pCurrentHistory->uiDate / (24 * 60), uiLastDate / (24 * 60));

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void GetQuestEndedString(UINT8 ubQuestValue, wchar_t* sQuestString);
static void GetQuestStartedString(UINT8 ubQuestValue, wchar_t* sQuestString);


static void ProcessHistoryTransactionString(wchar_t* const pString, const size_t Length, const HistoryUnit* const pHistory)
{
	switch( pHistory->ubCode)
	{
		case HISTORY_ENTERED_HISTORY_MODE:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_ENTERED_HISTORY_MODE ]);
			break;

		case HISTORY_HIRED_MERC_FROM_AIM:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_HIRED_MERC_FROM_AIM ], gMercProfiles[pHistory->ubSecondCode].zName  );
			break;

		case HISTORY_MERC_KILLED:
			if( pHistory->ubSecondCode != NO_PROFILE )
				swprintf(pString, Length, pHistoryStrings[ HISTORY_MERC_KILLED ], gMercProfiles[pHistory->ubSecondCode].zName );
#ifdef JA2BETAVERSION
			else
			{
				swprintf(pString, Length, pHistoryStrings[ HISTORY_MERC_KILLED ], L"ERROR!!!  NO_PROFILE" );
			}
#endif
			break;

		case HISTORY_HIRED_MERC_FROM_MERC:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_HIRED_MERC_FROM_MERC ],  gMercProfiles[pHistory->ubSecondCode].zName );
			break;

		case HISTORY_SETTLED_ACCOUNTS_AT_MERC:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_SETTLED_ACCOUNTS_AT_MERC ] );
			break;
		case HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_ACCEPTED_ASSIGNMENT_FROM_ENRICO ] );
			break;
		case( HISTORY_CHARACTER_GENERATED ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_CHARACTER_GENERATED ] );
		  break;
		case( HISTORY_PURCHASED_INSURANCE ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_PURCHASED_INSURANCE ], gMercProfiles[pHistory->ubSecondCode].zNickname );
		  break;
		case( HISTORY_CANCELLED_INSURANCE ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_CANCELLED_INSURANCE ], gMercProfiles[pHistory->ubSecondCode].zNickname );
		  break;
		case( HISTORY_INSURANCE_CLAIM_PAYOUT ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_INSURANCE_CLAIM_PAYOUT ], gMercProfiles[pHistory->ubSecondCode].zNickname );
		  break;

		case HISTORY_EXTENDED_CONTRACT_1_DAY:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_EXTENDED_CONTRACT_1_DAY ], gMercProfiles[pHistory->ubSecondCode].zNickname );
			break;

		case HISTORY_EXTENDED_CONTRACT_1_WEEK:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_EXTENDED_CONTRACT_1_WEEK ], gMercProfiles[pHistory->ubSecondCode].zNickname );
			break;

		case HISTORY_EXTENDED_CONTRACT_2_WEEK:
			swprintf(pString, Length, pHistoryStrings[ HISTORY_EXTENDED_CONTRACT_2_WEEK ], gMercProfiles[pHistory->ubSecondCode].zNickname );
			break;

		case( HISTORY_MERC_FIRED ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_MERC_FIRED ], gMercProfiles[pHistory->ubSecondCode].zNickname );
		  break;

		case( HISTORY_MERC_QUIT ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_MERC_QUIT ], gMercProfiles[pHistory->ubSecondCode].zNickname );
		  break;

		case( HISTORY_QUEST_STARTED ):
		{
			wchar_t sString[HISTORY_QUEST_TEXT_SIZE];
			GetQuestStartedString(pHistory->ubSecondCode, sString);
			swprintf(pString, Length, sString);
			break;
		}

		case( HISTORY_QUEST_FINISHED ):
		{
			wchar_t sString[HISTORY_QUEST_TEXT_SIZE];
			GetQuestEndedString(pHistory->ubSecondCode, sString);
			swprintf(pString, Length, sString);
			break;
		}

		case( HISTORY_TALKED_TO_MINER ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_TALKED_TO_MINER ], pTownNames[ pHistory->ubSecondCode ] );
		  break;
		case( HISTORY_LIBERATED_TOWN ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_LIBERATED_TOWN ], pTownNames[ pHistory->ubSecondCode ] );
			break;
		case( HISTORY_CHEAT_ENABLED ):
			swprintf(pString, Length, pHistoryStrings[ HISTORY_CHEAT_ENABLED ] );
			break;
		case HISTORY_TALKED_TO_FATHER_WALKER:
			swprintf( pString, Length, pHistoryStrings[ HISTORY_TALKED_TO_FATHER_WALKER ] );
			break;
		case HISTORY_MERC_MARRIED_OFF:
			swprintf( pString, Length, pHistoryStrings[ HISTORY_MERC_MARRIED_OFF ], gMercProfiles[pHistory->ubSecondCode].zNickname );
			break;
		case HISTORY_MERC_CONTRACT_EXPIRED:
			swprintf( pString, Length, pHistoryStrings[ HISTORY_MERC_CONTRACT_EXPIRED ], gMercProfiles[pHistory->ubSecondCode].zName );
			break;
		case HISTORY_RPC_JOINED_TEAM:
			swprintf( pString, Length, pHistoryStrings[ HISTORY_RPC_JOINED_TEAM ], gMercProfiles[pHistory->ubSecondCode].zName );
			break;
		case HISTORY_ENRICO_COMPLAINED:
			swprintf( pString, Length, pHistoryStrings[ HISTORY_ENRICO_COMPLAINED ] );
			break;
		case HISTORY_MINE_RUNNING_OUT:
		case HISTORY_MINE_RAN_OUT:
		case HISTORY_MINE_SHUTDOWN:
		case HISTORY_MINE_REOPENED:
			// all the same format
			swprintf(pString, Length, pHistoryStrings[ pHistory->ubCode ], pTownNames[ pHistory->ubSecondCode ] );
			break;
		case HISTORY_LOST_BOXING:
		case HISTORY_WON_BOXING:
		case HISTORY_DISQUALIFIED_BOXING:
		case HISTORY_NPC_KILLED:
		case HISTORY_MERC_KILLED_CHARACTER:
			swprintf( pString, Length, pHistoryStrings[ pHistory->ubCode ], gMercProfiles[pHistory->ubSecondCode].zNickname );
			break;

		// ALL SIMPLE HISTORY LOG MSGS, NO PARAMS
		case HISTORY_FOUND_MONEY:
		case HISTORY_ASSASSIN:
		case HISTORY_DISCOVERED_TIXA:
		case HISTORY_DISCOVERED_ORTA:
		case HISTORY_GOT_ROCKET_RIFLES:
		case HISTORY_DEIDRANNA_DEAD_BODIES:
		case HISTORY_BOXING_MATCHES:
		case HISTORY_SOMETHING_IN_MINES:
		case HISTORY_DEVIN:
		case HISTORY_MIKE:
		case HISTORY_TONY:
		case HISTORY_KROTT:
		case HISTORY_KYLE:
		case HISTORY_MADLAB:
		case HISTORY_GABBY:
		case HISTORY_KEITH_OUT_OF_BUSINESS:
		case HISTORY_HOWARD_CYANIDE:
		case HISTORY_KEITH:
		case HISTORY_HOWARD:
		case HISTORY_PERKO:
		case HISTORY_SAM:
		case HISTORY_FRANZ:
		case HISTORY_ARNOLD:
		case HISTORY_FREDO:
		case HISTORY_RICHGUY_BALIME:
		case HISTORY_JAKE:
		case HISTORY_BUM_KEYCARD:
		case HISTORY_WALTER:
		case HISTORY_DAVE:
		case HISTORY_PABLO:
		case HISTORY_KINGPIN_MONEY:
		//VARIOUS BATTLE CONDITIONS
		case HISTORY_LOSTTOWNSECTOR:
		case HISTORY_DEFENDEDTOWNSECTOR:
		case HISTORY_LOSTBATTLE:
		case HISTORY_WONBATTLE:
		case HISTORY_FATALAMBUSH:
		case HISTORY_WIPEDOUTENEMYAMBUSH:
		case HISTORY_UNSUCCESSFULATTACK:
		case HISTORY_SUCCESSFULATTACK:
		case HISTORY_CREATURESATTACKED:
		case HISTORY_KILLEDBYBLOODCATS:
		case HISTORY_SLAUGHTEREDBLOODCATS:
		case HISTORY_GAVE_CARMEN_HEAD:
		case HISTORY_SLAY_MYSTERIOUSLY_LEFT:
			swprintf( pString, Length, pHistoryStrings[ pHistory->ubCode ] );
			break;
	}
}


static void SetHistoryButtonStates(void)
{
	// this function will look at what page we are viewing, enable and disable buttons as needed

	if( iCurrentHistoryPage == 1 )
	{
		// first page, disable left buttons
		DisableButton( 	giHistoryButton[PREV_PAGE_BUTTON] );

	}
	else
	{
		// enable buttons
		EnableButton( giHistoryButton[PREV_PAGE_BUTTON] );

	}

	if( IncrementCurrentPageHistoryDisplay( ) )
	{
		// decrement page
    iCurrentHistoryPage--;
		DrawAPageofHistoryRecords( );

		// enable buttons
		EnableButton( giHistoryButton[ NEXT_PAGE_BUTTON ] );

	}
	else
	{
    DisableButton( 	giHistoryButton[ NEXT_PAGE_BUTTON ] );
	}
}


static BOOLEAN LoadInHistoryRecords(UINT32 uiPage)
{
	// loads in records belogning, to page uiPage
  // no file, return
	BOOLEAN fOkToContinue=TRUE;
  INT32 iCount =0;
  HWFILE hFileHandle;
  UINT8 ubCode, ubSecondCode;
	INT16 sSectorX, sSectorY;
	INT8  bSectorZ;
	UINT32 uiDate;
	UINT8 ubColor;
  UINT32 uiByteCount=0;

	// check if bad page
	if( uiPage == 0 )
	{
		return ( FALSE );
	}

	// open file
	hFileHandle = FileOpen(HISTORY_DATA_FILE, FILE_ACCESS_READ);

	// failed to get file, return
	if(!hFileHandle)
	{
		return( FALSE );
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
		return( FALSE );
	}

  // is the file long enough?
  if( ( FileGetSize( hFileHandle ) - 1 ) / ( NUM_RECORDS_PER_PAGE * SIZE_OF_HISTORY_FILE_RECORD ) + 1 < uiPage )
	{
		// nope
		FileClose( hFileHandle );
    return( FALSE );
	}

	FileSeek( hFileHandle, ( uiPage - 1 ) * NUM_RECORDS_PER_PAGE * ( SIZE_OF_HISTORY_FILE_RECORD), FILE_SEEK_FROM_START );

	uiByteCount = ( uiPage  - 1 ) * NUM_RECORDS_PER_PAGE * (SIZE_OF_HISTORY_FILE_RECORD );
	// file exists, read in data, continue until end of page
  while( ( iCount < NUM_RECORDS_PER_PAGE )&&( fOkToContinue ) )
	{

		// read in other data
    FileRead(hFileHandle, &ubCode,       sizeof(UINT8));
		FileRead(hFileHandle, &ubSecondCode, sizeof(UINT8));
		FileRead(hFileHandle, &uiDate,       sizeof(UINT32));
    FileRead(hFileHandle, &sSectorX,     sizeof(INT16));
    FileRead(hFileHandle, &sSectorY,     sizeof(INT16));
		FileRead(hFileHandle, &bSectorZ,     sizeof(INT8));
		FileRead(hFileHandle, &ubColor,      sizeof(UINT8));

		#ifdef JA2TESTVERSION
		//perform a check on the data to see if it is pooched
		PerformCheckOnHistoryRecord( 3, sSectorX, sSectorY, bSectorZ );
		#endif

		// add transaction
    ProcessAndEnterAHistoryRecord(ubCode, uiDate,  ubSecondCode, sSectorX, sSectorY, bSectorZ, ubColor);

		// increment byte counter
	  uiByteCount += SIZE_OF_HISTORY_FILE_RECORD;

		// we've overextended our welcome, and bypassed end of file, get out
		if( uiByteCount >=  FileGetSize( hFileHandle ) )
		{
			// not ok to continue
			fOkToContinue = FALSE;
		}

		iCount++;
	}

	FileClose( hFileHandle );

	// check to see if we in fact have a list to display
  if( pHistoryListHead == NULL )
	{
		// got no records, return false
		return( FALSE );
	}

	// set up current finance
	pCurrentHistory = pHistoryListHead;

	return( TRUE );
}


static BOOLEAN LoadNextHistoryPage(void)
{

	// clear out old list of records, and load in previous page worth of records
  ClearHistoryList( );



	// now load in previous page's records, if we can
  if ( LoadInHistoryRecords( iCurrentHistoryPage + 1 ) )
	{
		iCurrentHistoryPage++;
	  return ( TRUE );
	}
	else
	{
		LoadInHistoryRecords( iCurrentHistoryPage );
	  return ( FALSE );
	}

}


static BOOLEAN LoadPreviousHistoryPage(void)
{

	// clear out old list of records, and load in previous page worth of records
  ClearHistoryList( );

	// load previous page
	if( ( iCurrentHistoryPage == 1 ) )
	{
		return ( FALSE );
	}

	// now load in previous page's records, if we can
  if ( LoadInHistoryRecords( iCurrentHistoryPage - 1 ) )
	{
		iCurrentHistoryPage--;
		return ( TRUE );
	}
	else
	{
    LoadInHistoryRecords( iCurrentHistoryPage );
		return ( FALSE );
	}
}


static UINT32 ReadInLastElementOfHistoryListAndReturnIdNumber(void);


static void SetLastPageInHistoryRecords(void)
{
	// grabs the size of the file and interprets number of pages it will take up
  HWFILE hFileHandle;

	// no file, return
	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return;

	// open file
	hFileHandle = FileOpen(HISTORY_DATA_FILE, FILE_ACCESS_READ);

	// failed to get file, return
	if(!hFileHandle)
	{
		guiLastPageInHistoryRecordsList = 1;
		return;
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
    guiLastPageInHistoryRecordsList = 1;
		return;
	}


	// done with file, close it
	FileClose( hFileHandle );

	guiLastPageInHistoryRecordsList = ReadInLastElementOfHistoryListAndReturnIdNumber( ) / NUM_RECORDS_PER_PAGE;
}


static UINT32 ReadInLastElementOfHistoryListAndReturnIdNumber(void)
{
	// this function will read in the last unit in the history list, to grab it's id number


  HWFILE hFileHandle;
  INT32 iFileSize = 0;

	// open file
	hFileHandle = FileOpen(HISTORY_DATA_FILE, FILE_ACCESS_READ);

	// failed to get file, return
	if(!hFileHandle)
	{
		return 0;
  }

	// make sure file is more than balance size + length of 1 record - 1 byte
  if ( FileGetSize( hFileHandle ) < SIZE_OF_HISTORY_FILE_RECORD )
	{
    FileClose( hFileHandle );
    return 0;
	}

	// size is?
  iFileSize = FileGetSize( hFileHandle );

	// done with file, close it
	FileClose( hFileHandle );

  // file size  / sizeof record in bytes is id
	return ( (  iFileSize  ) / ( SIZE_OF_HISTORY_FILE_RECORD ) );

}


static BOOLEAN AppendHistoryToEndOfFile(void)
{
  	// will write the current finance to disk
	HistoryUnit* pHistoryList = pHistoryListHead;

	const HWFILE hFileHandle = FileOpen(HISTORY_DATA_FILE, FILE_ACCESS_APPEND | FILE_OPEN_ALWAYS);

	// if no file exits, do nothing
	if(!hFileHandle)
	{
    return ( FALSE );
	}

		#ifdef JA2TESTVERSION
		//perform a check on the data to see if it is pooched
		PerformCheckOnHistoryRecord( 5, pHistoryList->sSectorX, pHistoryList->sSectorY, pHistoryList->bSectorZ );
		#endif


	 	// now write date and amount, and code
	FileWrite(hFileHandle, &pHistoryList->ubCode,       sizeof(UINT8));
	FileWrite(hFileHandle, &pHistoryList->ubSecondCode, sizeof(UINT8));
	FileWrite(hFileHandle, &pHistoryList->uiDate,       sizeof(UINT32));
	FileWrite(hFileHandle, &pHistoryList->sSectorX,     sizeof(INT16));
	FileWrite(hFileHandle, &pHistoryList->sSectorY,     sizeof(INT16));
	FileWrite(hFileHandle, &pHistoryList->bSectorZ,     sizeof(INT8));
	FileWrite(hFileHandle, &pHistoryList->ubColor,      sizeof(UINT8));

  FileClose( hFileHandle );
  return( TRUE );
}

void ResetHistoryFact( UINT8 ubCode, INT16 sSectorX, INT16 sSectorY )
{
	// run through history list
	HistoryUnit* pList = pHistoryListHead;
	BOOLEAN fFound = FALSE;

	// set current page to before list
	iCurrentHistoryPage = 0;

	SetLastPageInHistoryRecords( );

	OpenAndReadHistoryFile( );

	pList = pHistoryListHead;

	while( pList )
	{
		if( ( pList -> ubSecondCode == ubCode ) && ( pList->ubCode == HISTORY_QUEST_STARTED ) )
		{
			// reset color
			pList -> ubColor = 0;
			fFound = TRUE;

			// save
			OpenAndWriteHistoryFile( );
			pList = NULL;
		}

		if( fFound != TRUE )
		{
			pList = pList->Next;
		}
	}

	if( fInHistoryMode )
	{
	  iCurrentHistoryPage--;

		// load in first page
	  LoadNextHistoryPage( );
	}

	SetHistoryFact( HISTORY_QUEST_FINISHED, ubCode, GetWorldTotalMin(), sSectorX, sSectorY );
}


UINT32 GetTimeQuestWasStarted( UINT8 ubCode )
{
	// run through history list
	HistoryUnit* pList = pHistoryListHead;
	BOOLEAN fFound = FALSE;
	UINT32 uiTime = 0;

	// set current page to before list
	iCurrentHistoryPage = 0;

	SetLastPageInHistoryRecords( );

	OpenAndReadHistoryFile( );

	pList = pHistoryListHead;

	while( pList )
	{
		if( ( pList -> ubSecondCode == ubCode ) && ( pList->ubCode == HISTORY_QUEST_STARTED ) )
		{
			uiTime = pList->uiDate;
			fFound = TRUE;

			pList = NULL;
		}

		if( fFound != TRUE )
		{
			pList = pList->Next;
		}
	}

	if( fInHistoryMode )
	{
	  iCurrentHistoryPage--;

		// load in first page
	  LoadNextHistoryPage( );
	}

	return( uiTime );
}


static void GetQuestStartedString(const UINT8 ubQuestValue, wchar_t* const sQuestString)
{
	// open the file and copy the string
	LoadEncryptedDataFromFile("BINARYDATA/quests.edt", sQuestString, HISTORY_QUEST_TEXT_SIZE * ubQuestValue * 2, HISTORY_QUEST_TEXT_SIZE);
}


static void GetQuestEndedString(const UINT8 ubQuestValue, wchar_t* const sQuestString)
{
	// open the file and copy the string
	LoadEncryptedDataFromFile("BINARYDATA/quests.edt", sQuestString, HISTORY_QUEST_TEXT_SIZE * (ubQuestValue * 2 + 1), HISTORY_QUEST_TEXT_SIZE);
}


#ifdef JA2TESTVERSION
static void PerformCheckOnHistoryRecord(UINT32 uiErrorCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ)
{
	if( sSectorX > 16 || sSectorY > 16 || bSectorZ > 3 || sSectorX < -1 || sSectorY < -1 || bSectorZ < 0 )
	{
		char zString[512];
		sprintf( zString, "History page is pooched, please remember what you were just doing, send your latest save to dave, and tell him this number, Error #%d.", uiErrorCode );
		AssertMsg( 0, zString );
	}
}
#endif


static INT32 GetNumberOfHistoryPages(void)
{
	HWFILE hFileHandle;
	UINT32	uiFileSize=0;
	UINT32  uiSizeOfRecordsOnEachPage = 0;
	INT32		iNumberOfHistoryPages = 0;

	// open file
	hFileHandle = FileOpen(HISTORY_DATA_FILE, FILE_ACCESS_READ);

	// failed to get file, return
	if(!hFileHandle)
	{
		return( 0 );
  }

	// make sure file is more than 0 length
  if ( FileGetSize( hFileHandle ) == 0 )
	{
    FileClose( hFileHandle );
		return( 0 );
	}

	uiFileSize = FileGetSize( hFileHandle ) - 1;
	uiSizeOfRecordsOnEachPage = ( NUM_RECORDS_PER_PAGE * ( sizeof( UINT8 ) + sizeof( UINT32 ) + 3*sizeof( UINT8 )+ sizeof(INT16) + sizeof( INT16 ) ) );

	iNumberOfHistoryPages = (INT32)( uiFileSize / uiSizeOfRecordsOnEachPage );

	FileClose( hFileHandle );

	return( iNumberOfHistoryPages );
}
