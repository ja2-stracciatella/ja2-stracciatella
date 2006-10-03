#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "History.h"
	#include "Game_Clock.h"
	#include "Utilities.h"
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
	#include "QuestText.h"
	#include "Button_System.h"
	#include "VSurface.h"
	#include "MemMan.h"
	#include "FileMan.h"
#endif



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
UINT32 guiTITLE;
//UINT32 guiGREYFRAME;
UINT32 guiTOP;
//UINT32 guiMIDDLE;
//UINT32 guiBOTTOM;
//UINT32 guiLINE;
UINT32 guiLONGLINE;
UINT32 guiSHADELINE;
//UINT32 guiVERTLINE;
//UINT32 guiBIGBOX;

enum{
	PREV_PAGE_BUTTON=0,
  NEXT_PAGE_BUTTON,
};



// the page flipping buttons
INT32 giHistoryButton[2];
INT32 giHistoryButtonImage[2];
BOOLEAN fInHistoryMode=FALSE;


// current page displayed
INT32 iCurrentHistoryPage=1;



// the History record list
HistoryUnitPtr pHistoryListHead=NULL;

// current History record (the one at the top of the current page)
HistoryUnitPtr pCurrentHistory=NULL;


// last page in list
UINT32 guiLastPageInHistoryRecordsList = 0;

// function definitions
BOOLEAN LoadHistory( void );
void RenderHistoryBackGround( void );
void RemoveHistory( void );
void CreateHistoryButtons( void );
void DestroyHistoryButtons( void );
void CreateHistoryButtons( void );
void DrawHistoryTitleText( void );
UINT32 ProcessAndEnterAHistoryRecord( UINT8 ubCode, UINT32 uiDate, UINT8 ubSecondCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, UINT8 ubColor );
void OpenAndReadHistoryFile( void );
BOOLEAN OpenAndWriteHistoryFile( void );
void ClearHistoryList( void );
void DisplayHistoryListHeaders( void );
void DisplayHistoryListBackground( void );
void DrawAPageofHistoryRecords( void );
BOOLEAN IncrementCurrentPageHistoryDisplay( void );
void DisplayPageNumberAndDateRange( void );
void ProcessHistoryTransactionString(STR16 pString, size_t Length, HistoryUnitPtr pHistory);
void SetHistoryButtonStates( void );
BOOLEAN LoadInHistoryRecords( UINT32 uiPage );
BOOLEAN LoadNextHistoryPage( void );
BOOLEAN LoadPreviousHistoryPage( void );
void SetLastPageInHistoryRecords( void );
UINT32 ReadInLastElementOfHistoryListAndReturnIdNumber( void );
BOOLEAN AppendHistoryToEndOfFile( HistoryUnitPtr pHistory );
BOOLEAN WriteOutHistoryRecords( UINT32 uiPage );
void		GetQuestStartedString( UINT8 ubQuestValue, STR16 sQuestString );
void		GetQuestEndedString( UINT8 ubQuestValue, STR16 sQuestString );
INT32		GetNumberOfHistoryPages();


#ifdef JA2TESTVERSION
void PerformCheckOnHistoryRecord( UINT32 uiErrorCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ );
#endif


// callbacks
void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON *btn,INT32 reason);
void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON *btn,INT32 reason);

UINT32 SetHistoryFact( UINT8 ubCode, UINT8 ubSecondCode, UINT32 uiDate, INT16 sSectorX, INT16 sSectorY )
{
	// adds History item to player's log(History List), returns unique id number of it
	// outside of the History system(the code in this .c file), this is the only function you'll ever need
  UINT32 uiId=0;
	UINT8 ubColor = 0;
  HistoryUnitPtr pHistory = pHistoryListHead;

	// clear the list
  ClearHistoryList( );

	// process the actual data
	if( ubCode == HISTORY_QUEST_FINISHED )
	{
		ubColor = 0;
	}
	else
	{
		ubColor = 1;
	}
  uiId = ProcessAndEnterAHistoryRecord(ubCode, uiDate,  ubSecondCode, sSectorX, sSectorY, 0, ubColor);
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_HISTORY_UPDATED ] );

	// history list head
  pHistory = pHistoryListHead;

	// append to end of file
  AppendHistoryToEndOfFile( pHistory );


	// if in history mode, reload current page
  if( fInHistoryMode )
	{
	  iCurrentHistoryPage--;

		// load in first page
	  LoadNextHistoryPage( );
	}


	// return unique id of this transaction
	return uiId;
}


UINT32 AddHistoryToPlayersLog(UINT8 ubCode, UINT8 ubSecondCode, UINT32 uiDate, INT16 sSectorX, INT16 sSectorY)
{
	// adds History item to player's log(History List), returns unique id number of it
	// outside of the History system(the code in this .c file), this is the only function you'll ever need
  UINT32 uiId=0;
  HistoryUnitPtr pHistory = pHistoryListHead;

	// clear the list
  ClearHistoryList( );

	// process the actual data
  uiId = ProcessAndEnterAHistoryRecord(ubCode, uiDate,  ubSecondCode, sSectorX, sSectorY, 0, 0);
	ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_HISTORY_UPDATED ] );

	// history list head
  pHistory = pHistoryListHead;

	// append to end of file
  AppendHistoryToEndOfFile( pHistory );


	// if in history mode, reload current page
  if( fInHistoryMode )
	{
	  iCurrentHistoryPage--;

		// load in first page
	  LoadNextHistoryPage( );
	}


	// return unique id of this transaction
	return uiId;
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

  return;
}

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


  return;
}

void HandleHistory()
{
	// DEF 2/5/99 Dont need to update EVERY FRAME!!!!
  // check and update status of buttons
//  SetHistoryButtonStates( );
}

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

	 return;
}


BOOLEAN LoadHistory( void )
{
  VOBJECT_DESC    VObjectDesc;
  // load History video objects into memory

	// title bar
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\programtitlebar.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiTITLE));

	// top portion of the screen background
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\historywindow.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiTOP));


  // shaded line
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\historylines.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiSHADELINE));

/*
Not being used???  DF commented out
  // vert  line
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\historyvertline.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiVERTLINE));
*/
  // black divider line - long ( 480 length)
  VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\divisionline480.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiLONGLINE));

	return (TRUE);
}

void RemoveHistory( void )
{

	// delete history video objects from memory
  DeleteVideoObjectFromIndex(guiLONGLINE);
	DeleteVideoObjectFromIndex(guiTOP);
	DeleteVideoObjectFromIndex(guiTITLE);
	DeleteVideoObjectFromIndex(guiSHADELINE);

	return;
}


void RenderHistoryBackGround( void )
{
	// render generic background for history system
  HVOBJECT hHandle;
  INT32 iCounter=0;

	// get title bar object
	GetVideoObject(&hHandle, guiTITLE);

	// blt title bar to screen
	BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X, TOP_Y -2 , VO_BLT_SRCTRANSPARENCY,NULL);


	// get and blt the top part of the screen, video object and blt to screen
  GetVideoObject(&hHandle, guiTOP);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X, TOP_Y + 22, VO_BLT_SRCTRANSPARENCY,NULL);

  // display background for history list
  DisplayHistoryListBackground( );
		return;
}

void DrawHistoryTitleText( void )
{
	// setup the font stuff
	SetFont(HISTORY_HEADER_FONT);
  SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(DEFAULT_SHADOW);

	// draw the pages title
	mprintf(TITLE_X,TITLE_Y,pHistoryTitle[0]);

	return;
}

void CreateHistoryButtons( void )
{

	// the prev page button
  giHistoryButtonImage[PREV_PAGE_BUTTON]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,0,-1,1,-1 );
	giHistoryButton[PREV_PAGE_BUTTON] = QuickCreateButton( giHistoryButtonImage[PREV_PAGE_BUTTON], PREV_BTN_X, BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnHistoryDisplayPrevPageCallBack);

	// the next page button
	giHistoryButtonImage[NEXT_PAGE_BUTTON]=  LoadButtonImage( "LAPTOP\\arrows.sti" ,-1,6,-1,7,-1 );
	giHistoryButton[NEXT_PAGE_BUTTON] = QuickCreateButton( giHistoryButtonImage[NEXT_PAGE_BUTTON], NEXT_BTN_X, BTN_Y,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
											(GUI_CALLBACK)BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnHistoryDisplayNextPageCallBack);


	// set buttons
	SetButtonCursor(giHistoryButton[0], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(giHistoryButton[1], CURSOR_LAPTOP_SCREEN);

	return;
}


void DestroyHistoryButtons( void )
{

	// remove History buttons and images from memory

	// next page button
	RemoveButton(giHistoryButton[1] );
	UnloadButtonImage(giHistoryButtonImage[1] );

	// prev page button
	RemoveButton(giHistoryButton[0] );
	UnloadButtonImage(giHistoryButtonImage[0] );

	return;
}

void BtnHistoryDisplayPrevPageCallBack(GUI_BUTTON *btn,INT32 reason)
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
		btn->uiFlags&=~(BUTTON_CLICKED_ON);
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

void BtnHistoryDisplayNextPageCallBack(GUI_BUTTON *btn,INT32 reason)
{

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		fReDrawScreenFlag=TRUE;
	}


	// force redraw
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
    // increment currentPage
    btn->uiFlags&=~(BUTTON_CLICKED_ON);
		LoadNextHistoryPage( );
		// set new state
		SetHistoryButtonStates( );
		fReDrawScreenFlag=TRUE;
	}



}

BOOLEAN IncrementCurrentPageHistoryDisplay( void )
{
  // run through list, from pCurrentHistory, to NUM_RECORDS_PER_PAGE +1 HistoryUnits
  HistoryUnitPtr pTempHistory=pCurrentHistory;
	BOOLEAN fOkToIncrementPage=FALSE;
	INT32 iCounter=0;
	HWFILE hFileHandle;
	UINT32	uiFileSize=0;
	UINT32  uiSizeOfRecordsOnEachPage = 0;

	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return( FALSE );

	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

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


	/*
	// haven't reached end of list and not yet at beginning of next page
	while( ( pTempHistory )&&( ! fOkToIncrementPage ) )
	{
    // found the next page,  first record thereof
		if(iCounter==NUM_RECORDS_PER_PAGE+1)
		{
			fOkToIncrementPage=TRUE;
		  pCurrentHistory=pTempHistory->Next;
		}

		//next record
		pTempHistory=pTempHistory->Next;
    iCounter++;
	}
*/
	// if ok to increment, increment


	return( TRUE );
}


UINT32 ProcessAndEnterAHistoryRecord( UINT8 ubCode, UINT32 uiDate, UINT8 ubSecondCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ , UINT8 ubColor )
{
  UINT32 uiId=0;
  HistoryUnitPtr pHistory=pHistoryListHead;

 	// add to History list
	if(pHistory)
	{
		// go to end of list
		while(pHistory->Next)
			pHistory=pHistory->Next;

		// alloc space
		pHistory->Next=MemAlloc(sizeof(HistoryUnit));

		// increment id number
		uiId = pHistory->uiIdNumber + 1;

		// set up information passed
		pHistory = pHistory->Next;
		pHistory->Next = NULL;
		pHistory->ubCode = ubCode;
    pHistory->ubSecondCode = ubSecondCode;
		pHistory->uiDate = uiDate;
    pHistory->uiIdNumber = uiId;
		pHistory->sSectorX = sSectorX;
		pHistory->sSectorY = sSectorY;
		pHistory->bSectorZ = bSectorZ;
		pHistory->ubColor = ubColor;

	}
	else
	{
		// alloc space
		pHistory=MemAlloc(sizeof(HistoryUnit));

		// setup info passed
		pHistory->Next = NULL;
		pHistory->ubCode = ubCode;
    pHistory->ubSecondCode = ubSecondCode;
		pHistory->uiDate = uiDate;
    pHistory->uiIdNumber = uiId;
	  pHistoryListHead = pHistory;
		pHistory->sSectorX = sSectorX;
		pHistory->sSectorY = sSectorY;
		pHistory->bSectorZ = bSectorZ;
		pHistory->ubColor = ubColor;

	}

	return uiId;
}


void OpenAndReadHistoryFile( void )
{
  // this procedure will open and read in data to the History list

	HWFILE hFileHandle;
  UINT8 ubCode, ubSecondCode;
	UINT32 uiDate;
  INT16 sSectorX, sSectorY;
	INT8 bSectorZ = 0;
	UINT8 ubColor;
  INT32 iBytesRead=0;
  UINT32 uiByteCount=0;

	// clear out the old list
	ClearHistoryList( );

	// no file, return
	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return;

	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

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
    FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
    FileRead( hFileHandle, &sSectorX, sizeof(INT16), &iBytesRead );
    FileRead( hFileHandle, &sSectorY, sizeof(INT16), &iBytesRead );
		FileRead( hFileHandle, &bSectorZ, sizeof(INT8), &iBytesRead );
		FileRead( hFileHandle, &ubColor, sizeof( UINT8 ), &iBytesRead );

		#ifdef JA2TESTVERSION
		//perform a check on the data to see if it is pooched
		PerformCheckOnHistoryRecord( 1, sSectorX, sSectorY, bSectorZ );
		#endif

		// add transaction
	  ProcessAndEnterAHistoryRecord( ubCode, uiDate, ubSecondCode, sSectorX, sSectorY, bSectorZ, ubColor );

		// increment byte counter
	  uiByteCount +=  SIZE_OF_HISTORY_FILE_RECORD;
	}

  // close file
	FileClose( hFileHandle );

	return;
}

BOOLEAN OpenAndWriteHistoryFile( void )
{
  // this procedure will open and write out data from the History list

	HWFILE hFileHandle;
  INT32 iBytesWritten=0;
  HistoryUnitPtr pHistoryList=pHistoryListHead;


	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE, FILE_ACCESS_WRITE|FILE_CREATE_ALWAYS, FALSE);

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
    FileWrite( hFileHandle, &(pHistoryList->ubCode ),  sizeof ( UINT8 ), NULL );
    FileWrite( hFileHandle, &(pHistoryList->ubSecondCode ),  sizeof ( UINT8 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->uiDate ),  sizeof ( UINT32 ), NULL );
    FileWrite( hFileHandle, &(pHistoryList->sSectorX ),  sizeof ( INT16 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->sSectorY ),  sizeof ( INT16 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->bSectorZ ),  sizeof ( INT8 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->ubColor ),  sizeof ( UINT8 ), NULL );

		// next element in list
		pHistoryList = pHistoryList->Next;

	}

	// close file
  FileClose( hFileHandle );
  // clear out the old list
	ClearHistoryList( );

	return ( TRUE );
}


void ClearHistoryList( void )
{
	// remove each element from list of transactions

	HistoryUnitPtr pHistoryList=pHistoryListHead;
  HistoryUnitPtr pHistoryNode=pHistoryList;

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

	return;
}

void DisplayHistoryListHeaders( void )
{
  // this procedure will display the headers to each column in History
  UINT16 usX, usY;

	// font stuff
	SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

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
	return;
}


void DisplayHistoryListBackground( void )
{
  // this function will display the History list display background
  HVOBJECT hHandle;
  INT32 iCounter=0;



	// get shaded line object
	GetVideoObject(&hHandle, guiSHADELINE);
	for(iCounter=0; iCounter <11; iCounter++)
	{
    // blt title bar to screen
	  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X + 15, (TOP_DIVLINE_Y + BOX_HEIGHT * 2 * iCounter), VO_BLT_SRCTRANSPARENCY,NULL);
	}

	// the long hortizontal line int he records list display region
  GetVideoObject(&hHandle, guiLONGLINE);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X + 9, (TOP_DIVLINE_Y ), VO_BLT_SRCTRANSPARENCY,NULL);
  BltVideoObject(FRAME_BUFFER, hHandle, 0,TOP_X + 9, (TOP_DIVLINE_Y + BOX_HEIGHT * 2 * 11  ), VO_BLT_SRCTRANSPARENCY,NULL);


	return;
}

void DrawHistoryRecordsText( void )
{
  // draws the text of the records
  HistoryUnitPtr pCurHistory=pHistoryListHead;
  HistoryUnitPtr pTempHistory=pHistoryListHead;
	wchar_t sString[512];
  INT32 iCounter=0;
	UINT16 usX, usY;
  INT32 iBalance=0;
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
	for(iCounter; iCounter <NUM_RECORDS_PER_PAGE; iCounter++)
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
			FindFontCenterCoordinates( RECORD_DATE_X + RECORD_DATE_WIDTH, 0,RECORD_LOCATION_WIDTH + 10, 0,  pHistoryLocations[0] ,HISTORY_TEXT_FONT, &sX, &sY );
		  mprintf(sX, RECORD_Y + ( iCounter * ( BOX_HEIGHT ) ) + 3, pHistoryLocations[0] );
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
	return;
}


void DrawAPageofHistoryRecords( void )
{
	// this procedure will draw a series of history records to the screen
  INT32 iCurPage=1;
	INT32 iCount=0;
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

	return;
}

void DisplayPageNumberAndDateRange( void )
{
	// this function will go through the list of 'histories' starting at current until end or
	// MAX_PER_PAGE...it will get the date range and the page number
	INT32 iLastPage=0;
	INT32 iCounter=0;
  UINT32 uiLastDate;
	HistoryUnitPtr pTempHistory=pHistoryListHead;
  wchar_t sString[50];



  // setup the font stuff
	SetFont(HISTORY_TEXT_FONT);
  SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

	if( !pCurrentHistory )
	{
    swprintf( sString, lengthof(sString), L"%s  %d / %d",pHistoryHeaders[1], 1, 1 );
	  mprintf( PAGE_NUMBER_X, PAGE_NUMBER_Y, sString );

    swprintf( sString, lengthof(sString), L"%s %d - %d",pHistoryHeaders[2], 1 , 1 );
    mprintf( HISTORY_DATE_X, HISTORY_DATE_Y, sString );

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

  swprintf( sString, lengthof(sString), L"%s  %d / %d",pHistoryHeaders[1], iCurrentHistoryPage , iLastPage +1 );
	mprintf( PAGE_NUMBER_X, PAGE_NUMBER_Y, sString );

  swprintf( sString, lengthof(sString), L"%s %d - %d",pHistoryHeaders[2], pCurrentHistory->uiDate / ( 24 * 60 ) , uiLastDate/( 24 * 60 ) );
  mprintf( HISTORY_DATE_X, HISTORY_DATE_Y, sString );


	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);

	return;
}


void ProcessHistoryTransactionString(STR16 pString, size_t Length, HistoryUnitPtr pHistory)
{
	CHAR16 sString[ 128 ];

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
			GetQuestStartedString( pHistory->ubSecondCode, sString );
			swprintf(pString, Length, sString );

		  break;
		case( HISTORY_QUEST_FINISHED ):
			GetQuestEndedString( pHistory->ubSecondCode, sString );
			swprintf(pString, Length, sString );

		  break;
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


void DrawHistoryLocation( INT16 sSectorX, INT16 sSectorY )
{
  // will draw the location of the history event


	return;
}


void SetHistoryButtonStates( void )
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


BOOLEAN LoadInHistoryRecords( UINT32 uiPage )
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
  INT32 iBytesRead=0;
  UINT32 uiByteCount=0;

	// check if bad page
	if( uiPage == 0 )
	{
		return ( FALSE );
	}


	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return( FALSE );

	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

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
    FileRead( hFileHandle, &ubCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &ubSecondCode, sizeof(UINT8), &iBytesRead );
		FileRead( hFileHandle, &uiDate, sizeof(UINT32), &iBytesRead );
    FileRead( hFileHandle, &sSectorX, sizeof(INT16), &iBytesRead );
    FileRead( hFileHandle, &sSectorY, sizeof(INT16), &iBytesRead );
		FileRead( hFileHandle, &bSectorZ, sizeof(INT8), &iBytesRead );
		FileRead( hFileHandle, &ubColor, sizeof(UINT8), &iBytesRead );

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

  // close file
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


BOOLEAN WriteOutHistoryRecords( UINT32 uiPage )
{
	// loads in records belogning, to page uiPage
  // no file, return
	BOOLEAN fOkToContinue=TRUE;
  INT32 iCount =0;
  HWFILE hFileHandle;
	HistoryUnitPtr pList;
  INT32 iBytesRead=0;
  UINT32 uiByteCount=0;

	// check if bad page
	if( uiPage == 0 )
	{
		return ( FALSE );
	}


	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return( FALSE );

	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_WRITE ), FALSE );

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

	pList = pHistoryListHead;

	if( pList == NULL )
	{
		return( FALSE );
	}

	FileSeek( hFileHandle, sizeof( INT32 ) + ( uiPage - 1 ) * NUM_RECORDS_PER_PAGE * SIZE_OF_HISTORY_FILE_RECORD, FILE_SEEK_FROM_START );

	uiByteCount = /*sizeof( INT32 )+ */( uiPage - 1 ) * NUM_RECORDS_PER_PAGE * SIZE_OF_HISTORY_FILE_RECORD;
	// file exists, read in data, continue until end of page
  while( ( iCount < NUM_RECORDS_PER_PAGE )&&( fOkToContinue ) )
	{

		#ifdef JA2TESTVERSION
		//perform a check on the data to see if it is pooched
		PerformCheckOnHistoryRecord( 4, pList->sSectorX, pList->sSectorY, pList->bSectorZ );
		#endif

		FileWrite( hFileHandle, &(pList->ubCode ),  sizeof ( UINT8 ), NULL );
    FileWrite( hFileHandle, &(pList->ubSecondCode ),  sizeof ( UINT8 ), NULL );
		FileWrite( hFileHandle, &(pList->uiDate ),  sizeof ( UINT32 ), NULL );
    FileWrite( hFileHandle, &(pList->sSectorX ),  sizeof ( INT16 ), NULL );
		FileWrite( hFileHandle, &(pList->sSectorY ),  sizeof ( INT16 ), NULL );
		FileWrite( hFileHandle, &(pList->bSectorZ ),  sizeof ( INT8 ), NULL );
		FileWrite( hFileHandle, &(pList->ubColor ),  sizeof ( UINT8 ), NULL );

		pList = pList->Next;

		// we've overextended our welcome, and bypassed end of file, get out
		if( pList == NULL )
		{
			// not ok to continue
			fOkToContinue = FALSE;
		}

		iCount++;
	}

  // close file
	FileClose( hFileHandle );

	ClearHistoryList( );

	return( TRUE );
}

BOOLEAN LoadNextHistoryPage( void )
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


BOOLEAN LoadPreviousHistoryPage( void )
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


void SetLastPageInHistoryRecords( void )
{
	// grabs the size of the file and interprets number of pages it will take up
  HWFILE hFileHandle;
  INT32 iBytesRead=0;

	// no file, return
	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return;

	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

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

	return;
}

UINT32 ReadInLastElementOfHistoryListAndReturnIdNumber( void )
{
	// this function will read in the last unit in the history list, to grab it's id number


  HWFILE hFileHandle;
  INT32 iBytesRead=0;
  INT32 iFileSize = 0;

	// no file, return
	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return 0;

	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

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


BOOLEAN AppendHistoryToEndOfFile( HistoryUnitPtr pHistory )
{
  	// will write the current finance to disk
  HWFILE hFileHandle;
  INT32 iBytesWritten=0;
  HistoryUnitPtr pHistoryList=pHistoryListHead;


	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE, FILE_ACCESS_WRITE|FILE_OPEN_ALWAYS, FALSE);

	// if no file exits, do nothing
	if(!hFileHandle)
	{
    return ( FALSE );
	}

	// go to the end
	if( FileSeek( hFileHandle,0,FILE_SEEK_FROM_END ) == FALSE )
	{
		// error
    FileClose( hFileHandle );
		return( FALSE );
	}

		#ifdef JA2TESTVERSION
		//perform a check on the data to see if it is pooched
		PerformCheckOnHistoryRecord( 5, pHistoryList->sSectorX, pHistoryList->sSectorY, pHistoryList->bSectorZ );
		#endif


	 	// now write date and amount, and code
    FileWrite( hFileHandle, &(pHistoryList->ubCode ),  sizeof ( UINT8 ), NULL );
    FileWrite( hFileHandle, &(pHistoryList->ubSecondCode ),  sizeof ( UINT8 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->uiDate ),  sizeof ( UINT32 ), NULL );
    FileWrite( hFileHandle, &(pHistoryList->sSectorX ),  sizeof ( INT16 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->sSectorY ),  sizeof ( INT16 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->bSectorZ ),  sizeof ( INT8 ), NULL );
		FileWrite( hFileHandle, &(pHistoryList->ubColor ),  sizeof ( UINT8 ), NULL );


		// close file
  FileClose( hFileHandle );

  return( TRUE );
}

void ResetHistoryFact( UINT8 ubCode, INT16 sSectorX, INT16 sSectorY )
{
	// run through history list
	INT32 iOldHistoryPage = iCurrentHistoryPage;
	HistoryUnitPtr pList = pHistoryListHead;
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
	return;
}


UINT32 GetTimeQuestWasStarted( UINT8 ubCode )
{
	// run through history list
	INT32 iOldHistoryPage = iCurrentHistoryPage;
	HistoryUnitPtr pList = pHistoryListHead;
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

void GetQuestStartedString( UINT8 ubQuestValue, STR16 sQuestString )
{
	// open the file and copy the string
	LoadEncryptedDataFromFile( "BINARYDATA\\quests.edt", sQuestString, 160 * ( ubQuestValue * 2  ), 160 );
}


void GetQuestEndedString( UINT8 ubQuestValue, STR16 sQuestString )
{
	// open the file and copy the string
	LoadEncryptedDataFromFile( "BINARYDATA\\quests.edt", sQuestString, 160 * ( ( ubQuestValue  * 2 ) + 1), 160 );
}


#ifdef JA2TESTVERSION
void PerformCheckOnHistoryRecord( UINT32 uiErrorCode, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ )
{
	CHAR	zString[512];

	if( sSectorX > 16 || sSectorY > 16 || bSectorZ > 3 || sSectorX < -1 || sSectorY < -1 || bSectorZ < 0 )
	{
		sprintf( zString, "History page is pooched, please remember what you were just doing, send your latest save to dave, and tell him this number, Error #%d.", uiErrorCode );
		AssertMsg( 0, zString );
	}
}
#endif

INT32 GetNumberOfHistoryPages()
{
	HWFILE hFileHandle;
	UINT32	uiFileSize=0;
	UINT32  uiSizeOfRecordsOnEachPage = 0;
	INT32		iNumberOfHistoryPages = 0;

	if ( ! (FileExists( HISTORY_DATA_FILE ) ) )
		return( 0 );

	// open file
 	hFileHandle=FileOpen( HISTORY_DATA_FILE,( FILE_OPEN_EXISTING |  FILE_ACCESS_READ ), FALSE );

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
