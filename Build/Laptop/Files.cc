#include "Font.h"
#include "HImage.h"
#include "Laptop.h"
#include "Files.h"
#include "Game_Clock.h"
#include "LoadSaveData.h"
#include "VObject.h"
#include "Debug.h"
#include "WordWrap.h"
#include "Render_Dirty.h"
#include "Encrypted_File.h"
#include "Cursors.h"
#include "Text.h"
#include "Button_System.h"
#include "VSurface.h"
#include "MemMan.h"
#include "Font_Control.h"
#include "FileMan.h"


typedef struct FilesUnit FilesUnit;
struct FilesUnit
{
	UINT8 ubCode; // the code index in the files code table
	BOOLEAN fRead;
	FilesUnit* Next; // next unit in the list
};


typedef struct FileString FileString;
struct FileString
{
	wchar_t* pString;
	FileString* Next;
};


typedef struct FileRecordWidth FileRecordWidth;
struct FileRecordWidth
{
	INT32 iRecordNumber;
	INT32 iRecordWidth;
	INT32 iRecordHeightAdjustment;
	UINT8 ubFlags;
	FileRecordWidth* Next;
};


enum
{
	ENRICO_BACKGROUND = 0,
	SLAY_BACKGROUND,
	MATRON_BACKGROUND,
	IMPOSTER_BACKGROUND,
	TIFFANY_BACKGROUND,
	REXALL_BACKGROUND,
	ELGIN_BACKGROUND
};


#define TOP_X														0+LAPTOP_SCREEN_UL_X
#define TOP_Y														LAPTOP_SCREEN_UL_Y
#define TITLE_X													140
#define TITLE_Y													33
#define FILES_TITLE_FONT								FONT14ARIAL
#define FILES_TEXT_FONT									FONT10ARIAL//FONT12ARIAL
#define FILES_SENDER_TEXT_X             (FILES_LIST_X + 5)
#define MAX_FILES_LIST_LENGTH						28
#define FILE_VIEWER_X										236
#define FILE_VIEWER_Y                    81
#define FILE_VIEWER_W                   364
#define FILE_VIEWER_H                   353
#define FILE_GAP												2
#define FILE_TEXT_COLOR									FONT_BLACK
#define FILE_STRING_SIZE								400
#define MAX_FILES_PAGE									MAX_FILES_LIST_LENGTH
#define FILES_LIST_X                    (TOP_X + 10)
#define FILES_LIST_Y                     85
#define FILES_LIST_W                    107
#define FILES_LIST_H                     12
#define LENGTH_OF_ENRICO_FILE						68
#define MAX_FILE_MESSAGE_PAGE_SIZE			325
#define PREVIOUS_FILE_PAGE_BUTTON_X			553
#define PREVIOUS_FILE_PAGE_BUTTON_Y			53
#define NEXT_FILE_PAGE_BUTTON_X					577
#define NEXT_FILE_PAGE_BUTTON_Y					PREVIOUS_FILE_PAGE_BUTTON_Y

#define	FILES_COUNTER_1_WIDTH						7
#define	FILES_COUNTER_2_WIDTH						43
#define	FILES_COUNTER_3_WIDTH						45


// the highlighted line
INT32 iHighLightFileLine=-1;


// the files record list
static FilesUnit* pFilesListHead = NULL;

static FileString* pFileStringList = NULL;

// are we in files mode
static BOOLEAN fInFilesMode=FALSE;
static BOOLEAN fOnLastFilesPageFlag = FALSE;


//. did we enter due to new file icon?
BOOLEAN fEnteredFileViewerFromNewFileIcon = FALSE;
static BOOLEAN fWaitAFrame = FALSE;

// are there any new files
BOOLEAN fNewFilesInFileViewer = FALSE;

// graphics handles
static SGPVObject* guiTITLE;
static SGPVObject* guiTOP;


// currewnt page of multipage files we are on
static INT32 giFilesPage = 0;
// strings

#define SLAY_LENGTH 12
#define ENRICO_LENGTH 0


static const UINT8 ubFileRecordsLength[] =
{
	ENRICO_LENGTH,
	SLAY_LENGTH,
	SLAY_LENGTH,
	SLAY_LENGTH,
	SLAY_LENGTH,
	SLAY_LENGTH,
	SLAY_LENGTH,
};


static const UINT16 ubFileOffsets[] =
{
	0,
	ENRICO_LENGTH,
	SLAY_LENGTH + ENRICO_LENGTH,
	2 * SLAY_LENGTH + ENRICO_LENGTH,
	3 * SLAY_LENGTH + ENRICO_LENGTH,
	4 * SLAY_LENGTH + ENRICO_LENGTH,
	5 * SLAY_LENGTH + ENRICO_LENGTH,
};


static const UINT16 usProfileIdsForTerroristFiles[] =
{
	0, // no body
	112, // elgin
	64, // slay
	82, // mom
	83, // imposter
	110, // tiff
	111, // t-rex
	112, // elgin
};


// buttons for next and previous pages
static GUIButtonRef giFilesPageButtons[2];
static MOUSE_REGION g_scroll_region;


// the previous and next pages buttons

enum{
	PREVIOUS_FILES_PAGE_BUTTON=0,
	NEXT_FILES_PAGE_BUTTON,
};
// mouse regions
static MOUSE_REGION pFilesRegions[MAX_FILES_PAGE];


static void CheckForUnreadFiles(void);
static void OpenAndReadFilesFile(void);
static void OpenAndWriteFilesFile(void);
static void ProcessAndEnterAFilesRecord(UINT8 ubCode, BOOLEAN fRead);


static void AddFilesToPlayersLog(UINT8 ubCode)
{
	// adds Files item to player's log(Files List)
	// outside of the Files system(the code in this .c file), this is the only function you'll ever need

	// if not in Files mode, read in from file
	if(!fInFilesMode)
   OpenAndReadFilesFile( );

	// process the actual data
	ProcessAndEnterAFilesRecord(ubCode, FALSE);

	// set unread flag, if nessacary
	CheckForUnreadFiles( );

	// write out to file if not in Files mode
	if(!fInFilesMode)
   OpenAndWriteFilesFile( );
}


static void ClearFilesList(void);


void GameInitFiles(void)
{
	FileDelete(FILES_DAT_FILE);
	ClearFilesList( );

	// add background check by RIS
	AddFilesToPlayersLog(ENRICO_BACKGROUND);
}


static void CreateButtonsForFilesPage(void);
static void HandleFileViewerButtonStates(void);
static void InitializeFilesMouseRegions(void);
static void LoadFiles(void);
static void OpenFirstUnreadFile(void);


void EnterFiles(void)
{
	// load grpahics for files system
	LoadFiles( );

  // in files mode now, set the fact
	fInFilesMode=TRUE;

	// initialize mouse regions
  InitializeFilesMouseRegions( );

  // create buttons
	CreateButtonsForFilesPage( );

	// now set start states
	HandleFileViewerButtonStates( );

	// build files list
  OpenAndReadFilesFile( );

	// render files system
  RenderFiles( );

	// entered due to icon
	if (fEnteredFileViewerFromNewFileIcon)
	{
	  OpenFirstUnreadFile( );
		fEnteredFileViewerFromNewFileIcon = FALSE;
	}
}


static void DeleteButtonsForFilesPage(void);
static void RemoveFiles(void);
static void RemoveFilesMouseRegions(void);


void ExitFiles(void)
{

	// write files list out to disk
  OpenAndWriteFilesFile( );

	// remove mouse regions
	RemoveFilesMouseRegions( );

	// delete buttons
	DeleteButtonsForFilesPage( );

	fInFilesMode = FALSE;

	// remove files
	RemoveFiles( );
}


void HandleFiles(void)
{
	CheckForUnreadFiles( );
}


static void DisplayFileMessage(void);
static void DisplayFilesList(void);
static void DrawFilesTitleText(void);
static void RenderFilesBackGround(void);


void RenderFiles(void)
{
	// render the background
	RenderFilesBackGround(  );

	// draw the title bars text
  DrawFilesTitleText( );

	// display the list of senders
  DisplayFilesList( );

	// draw the highlighted file
	DisplayFileMessage( );

	// title bar icon
	BlitTitleBarIcons(  );

	BltVideoObject(FRAME_BUFFER, guiLaptopBACKGROUND, 0, 108, 23);
}


static void RenderFilesBackGround(void)
{
	// render generic background for file system
	BltVideoObject(FRAME_BUFFER, guiTITLE, 0, TOP_X, TOP_Y -  2);
	BltVideoObject(FRAME_BUFFER, guiTOP,   0, TOP_X, TOP_Y + 22);
}


static void DrawFilesTitleText(void)
{
	// setup the font stuff
	SetFont(FILES_TITLE_FONT);
  SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);
  // reset shadow
	SetFontShadow(DEFAULT_SHADOW);

	// draw the pages title
	MPrint(TITLE_X, TITLE_Y, pFilesTitle);
}


static void LoadFiles(void)
{
  // load files video objects into memory

	// title bar
	guiTITLE = AddVideoObjectFromFile("LAPTOP/programtitlebar.sti");

	// top portion of the screen background
	guiTOP = AddVideoObjectFromFile("LAPTOP/fileviewer.sti");
}


static void RemoveFiles(void)
{
	// delete files video objects from memory
	DeleteVideoObject(guiTOP);
	DeleteVideoObject(guiTITLE);
}


static void ProcessAndEnterAFilesRecord(const UINT8 ubCode, const BOOLEAN fRead)
{
	// Append node to list
	FilesUnit** anchor;
	for (anchor = &pFilesListHead; *anchor != NULL; anchor = &(*anchor)->Next)
	{
		// Check if the file is already there
		if ((*anchor)->ubCode == ubCode) return;
	}

	FilesUnit* const f = MALLOC(FilesUnit);
	f->Next   = NULL;
	f->ubCode = ubCode;
	f->fRead  = fRead;

	*anchor = f;
}


#define FILE_ENTRY_SIZE 263


static void OpenAndReadFilesFile(void)
{
	ClearFilesList();

	AutoSGPFile f;
	try
	{
		f = FileOpen(FILES_DAT_FILE, FILE_ACCESS_READ);
	}
	catch (...) { return; /* XXX TODO0019 ignore */ }

	// file exists, read in data, continue until file end
  for (UINT i = FileGetSize(f) / FILE_ENTRY_SIZE; i != 0; --i)
	{
		BYTE data[FILE_ENTRY_SIZE];
		FileRead(f, data, sizeof(data));

		UINT8 code;
		UINT8 already_read;

		const BYTE* d = data;
		EXTR_U8(d, code)
		EXTR_SKIP(d, 261)
		EXTR_U8(d, already_read)
		Assert(d == endof(data));

		ProcessAndEnterAFilesRecord(code, already_read);
	}
}


static void OpenAndWriteFilesFile(void)
{
	AutoSGPFile f(FileOpen(FILES_DAT_FILE, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS));

  for (const FilesUnit* i = pFilesListHead; i; i = i->Next)
	{
		BYTE  data[FILE_ENTRY_SIZE];
		BYTE* d = data;
		INJ_U8(d, i->ubCode)
		INJ_SKIP(d, 261)
		INJ_U8(d, i->fRead)
		Assert(d == endof(data));

		FileWrite(f, data, sizeof(data));
	}

	ClearFilesList();
}


static void ClearFilesList(void)
{
	// remove each element from list of transactions
	FilesUnit* pFilesList = pFilesListHead;
	FilesUnit* pFilesNode = pFilesList;

	// while there are elements in the list left, delete them
	while( pFilesList )
	{
    // set node to list head
		pFilesNode=pFilesList;

		// set list head to next node
		pFilesList=pFilesList->Next;

		// delete current node
		MemFree(pFilesNode);
	}
  pFilesListHead=NULL;
}


static void DisplayFilesList(void)
{
	// this function will run through the list of files of files and display the 'sender'
	SetFont(FILES_TEXT_FONT);
	SetFontForeground(FONT_BLACK);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	INT32       i = 0;
	INT32 const x = FILES_LIST_X;
	INT32       y = FILES_LIST_Y;
	INT32 const w = FILES_LIST_W;
	INT32 const h = FILES_LIST_H;
	for (FilesUnit const* fu = pFilesListHead; fu; ++i, fu = fu->Next)
	{
		if (i == iHighLightFileLine)
		{
			UINT16 const colour = Get16BPPColor(FROMRGB(240, 240, 200));
			ColorFillVideoSurfaceArea(FRAME_BUFFER, x, y, x + w, y + h, colour);
		}
		MPrint(FILES_SENDER_TEXT_X, y + 2, pFilesSenderList[fu->ubCode]);
		y += h;
	}

	SetFontShadow(DEFAULT_SHADOW);
}


static void DisplayFormattedText(void);


static void DisplayFileMessage(void)
{
	// get the currently selected message
  if(iHighLightFileLine!=-1)
  {
		// display text
    DisplayFormattedText( );
	}
	else
	{
		HandleFileViewerButtonStates( );
	}

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void FilesBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void InitializeFilesMouseRegions(void)
{
	UINT16 const x = FILES_LIST_X;
	UINT16       y = FILES_LIST_Y;
	UINT16 const w = FILES_LIST_W;
	UINT16 const h = FILES_LIST_H;
	for (INT32 i = 0; i != MAX_FILES_PAGE; ++i)
	{
		MOUSE_REGION* const r = &pFilesRegions[i];
		MSYS_DefineRegion(r, x, y, x + w - 1, y + h - 1, MSYS_PRIORITY_NORMAL + 2, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, FilesBtnCallBack);
		y += h;
		MSYS_SetRegionUserData(r, 0, i);
	}
}


static void RemoveFilesMouseRegions(void)
{
  INT32 iCounter=0;
  for(iCounter=0; iCounter <MAX_FILES_PAGE; iCounter++)
	{
	 MSYS_RemoveRegion( &pFilesRegions[iCounter]);
	}
}


static void FilesBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		FilesUnit* pFilesList = pFilesListHead;
		INT32 iFileId = MSYS_GetRegionUserData(pRegion, 0);
		INT32 iCounter = 0;

		// reset iHighLightListLine
		iHighLightFileLine = -1;

		if (iHighLightFileLine == iFileId) return;

		// make sure is a valid
		while (pFilesList != NULL)
		{
			if (iCounter == iFileId)
			{
				giFilesPage = 0;
				iHighLightFileLine = iFileId;
			}

			pFilesList = pFilesList->Next;
			iCounter++;
		}
		fReDrawScreenFlag = TRUE;
	}
}


static void HandleSpecialFiles(void);
static void HandleSpecialTerroristFile(INT32 iFileNumber);


static void DisplayFormattedText(void)
{
	fWaitAFrame = FALSE;

	UINT16 const white = Get16BPPColor(FROMRGB(255, 255, 255));
	INT32  const x     = FILE_VIEWER_X;
	INT32  const y     = FILE_VIEWER_Y;
	ColorFillVideoSurfaceArea(FRAME_BUFFER, x, y, x + FILE_VIEWER_W, y + FILE_VIEWER_H, white);

	// Get the file that was highlighted
	FilesUnit* fu = pFilesListHead;
	for (INT32 n = iHighLightFileLine; n != 0; --n)
	{
		fu = fu->Next;
	}

	fu->fRead = TRUE;

	switch (fu->ubCode)
	{
		case ENRICO_BACKGROUND: HandleSpecialFiles();                   break;
		default:                HandleSpecialTerroristFile(fu->ubCode); break;
	}

	HandleFileViewerButtonStates();
	SetFontShadow(DEFAULT_SHADOW);
}


static FileString* GetFirstStringOnThisPage(FileString* RecordList, Font const font, UINT16 usWidth, UINT8 ubGap, INT32 iPage, INT32 iPageSize, FileRecordWidth* WidthList)
{
	// get the first record on this page - build pages up until this point
	FileString* CurrentRecord = NULL;

	INT32 iCurrentPositionOnThisPage = 0;
	INT32 iCurrentPage =0;
	INT32 iCounter =0;
	FileRecordWidth* pWidthList = WidthList;
	UINT16 usCurrentWidth = usWidth;




	// null record list, nothing to do
	if( RecordList == NULL )
	{

		return ( CurrentRecord );

	}

	CurrentRecord = RecordList;

	// while we are not on the current page
	while( iCurrentPage < iPage )
	{


		usCurrentWidth = usWidth;
		pWidthList = WidthList;

		while( pWidthList )
		{

			if( iCounter == pWidthList->iRecordNumber )
			{
				usCurrentWidth = ( INT16 ) pWidthList->iRecordWidth;
//				iCurrentPositionOnThisPage += pWidthList->iRecordHeightAdjustment;


				if( pWidthList->iRecordHeightAdjustment == iPageSize )
				{
					if( iCurrentPositionOnThisPage != 0 )
						iCurrentPositionOnThisPage += iPageSize - iCurrentPositionOnThisPage;
				}
				else
					iCurrentPositionOnThisPage += pWidthList->iRecordHeightAdjustment;

			}
			pWidthList = pWidthList ->Next;

		}

		// build record list to this point
		while (iCurrentPositionOnThisPage + IanWrappedStringHeight(usCurrentWidth, ubGap, font, CurrentRecord->pString) < iPageSize)
		{
			// still room on this page
			iCurrentPositionOnThisPage += IanWrappedStringHeight(usCurrentWidth, ubGap, font, CurrentRecord->pString);

			// next record
			CurrentRecord = CurrentRecord->Next;
			iCounter++;

			usCurrentWidth = usWidth;
			pWidthList = WidthList;
			while( pWidthList )
			{

				if( iCounter == pWidthList->iRecordNumber )
				{
					usCurrentWidth = ( INT16 ) pWidthList->iRecordWidth;

					if( pWidthList->iRecordHeightAdjustment == iPageSize )
					{
						if( iCurrentPositionOnThisPage != 0 )
							iCurrentPositionOnThisPage += iPageSize - iCurrentPositionOnThisPage;
					}
					else
						iCurrentPositionOnThisPage += pWidthList->iRecordHeightAdjustment;

				}
				pWidthList = pWidthList->Next;
			}

		}

		// reset position
		iCurrentPositionOnThisPage = 0;


		// next page
		iCurrentPage++;
//		iCounter++;

	}

	return ( CurrentRecord );
}


static void AddStringToFilesList(const wchar_t* pString);
static void ClearFileStringList(void);
static void ClearOutWidthRecordsList(FileRecordWidth* pFileRecordWidthList);
static FileRecordWidth* CreateWidthRecordsForAruloIntelFile(void);


static void HandleSpecialFiles(void)
{
	INT32 iCounter = 0;
	FileString* pTempString = NULL;
	FileString* pLocatorString = NULL;
	INT32 iYPositionOnPage = 0;
	INT32 iFileLineWidth = 0;
	INT32 iFileStartX = 0;
	BOOLEAN fGoingOffCurrentPage = FALSE;
	FileRecordWidth* WidthList = NULL;

	ClearFileStringList( );

	// load data
	// read one record from file manager file

	WidthList = CreateWidthRecordsForAruloIntelFile( );
	while( iCounter < LENGTH_OF_ENRICO_FILE )
	{
		wchar_t sString[FILE_STRING_SIZE];
		LoadEncryptedDataFromFile("BINARYDATA/RIS.EDT", sString, FILE_STRING_SIZE * iCounter, FILE_STRING_SIZE);
		AddStringToFilesList( sString );
		iCounter++;
	}

	pTempString = pFileStringList;


	iYPositionOnPage = 0;
	iCounter = 0;
	pLocatorString = pTempString;

	pTempString = GetFirstStringOnThisPage( pFileStringList,FILES_TEXT_FONT,  350, FILE_GAP, giFilesPage, MAX_FILE_MESSAGE_PAGE_SIZE, WidthList);

	// find out where this string is
	while( pLocatorString != pTempString )
	{
		iCounter++;
		pLocatorString = pLocatorString->Next;
	}


	// move through list and display
	while( pTempString )
	{
		const wchar_t* String = pTempString->pString;

		if (String[0] == L'\0')
		{
			// on last page
			fOnLastFilesPageFlag = TRUE;
		}


		// set up font
		Font font = FILES_TEXT_FONT;
		if( giFilesPage == 0 )
		{
			switch (iCounter)
			{
				case 0: font = FILES_TITLE_FONT; break;
			}
		}

		// reset width
		iFileLineWidth = 350;
		iFileStartX = (UINT16) ( FILE_VIEWER_X +  10 );

		// based on the record we are at, selected X start position and the width to wrap the line, to fit around pictures

		if( iCounter == 0 )
		{
			// title
			iFileLineWidth = 350;
			iFileStartX = (UINT16) ( FILE_VIEWER_X  +  10 );

		}
		else if( iCounter == 1 )
		{
			// opening on first page
			iFileLineWidth = 350;
			iFileStartX = (UINT16) ( FILE_VIEWER_X  +  10 );

		}
		else if( ( iCounter > 1) &&( iCounter < FILES_COUNTER_1_WIDTH ) )
		{
			iFileLineWidth = 350;
			iFileStartX = (UINT16) ( FILE_VIEWER_X  +  10 );

		}
		else if( iCounter == FILES_COUNTER_1_WIDTH )
		{
			if( giFilesPage == 0 )
			{
				iYPositionOnPage += ( MAX_FILE_MESSAGE_PAGE_SIZE - iYPositionOnPage );
			}
			iFileLineWidth = 350;
			iFileStartX = (UINT16) ( FILE_VIEWER_X  +  10 );
		}

		else if( iCounter == FILES_COUNTER_2_WIDTH )
		{
			iFileLineWidth = 200;
			iFileStartX = (UINT16) ( FILE_VIEWER_X  +  150 );
		}
		else if( iCounter == FILES_COUNTER_3_WIDTH )
		{
			iFileLineWidth = 200;
			iFileStartX = (UINT16) ( FILE_VIEWER_X  +  150 );
		}

		else
		{
			iFileLineWidth = 350;
			iFileStartX = (UINT16) ( FILE_VIEWER_X +  10 );
		}
		// not far enough, advance

		if (iYPositionOnPage + IanWrappedStringHeight(iFileLineWidth, FILE_GAP, font, String) < MAX_FILE_MESSAGE_PAGE_SIZE)
		{
			 // now print it
			 iYPositionOnPage += IanDisplayWrappedString(iFileStartX, FILE_VIEWER_Y + 4 + iYPositionOnPage, iFileLineWidth, FILE_GAP, font, FILE_TEXT_COLOR, String, 0, IAN_WRAP_NO_SHADOW);
			 fGoingOffCurrentPage = FALSE;
		}
		else
		{
			 // gonna get cut off...end now
			 fGoingOffCurrentPage = TRUE;
		}

		pTempString = pTempString ->Next;

		if( pTempString == NULL )
		{
			// on last page
			fOnLastFilesPageFlag = TRUE;
		}
		else
		{
			fOnLastFilesPageFlag = FALSE;
		}

		// going over the edge, stop now
		if (fGoingOffCurrentPage)
		{
			pTempString = NULL;
		}
		iCounter++;
	}
	ClearOutWidthRecordsList( WidthList );
	ClearFileStringList( );

	// place pictures
	// page 1 picture of country
	if( giFilesPage == 0 )
	{
		// title bar
		BltVideoObjectOnce(FRAME_BUFFER, "LAPTOP/ArucoFilesMap.sti", 0, 300, 270);
	}
	else if( giFilesPage == 4 )
	{
		// kid pic
		BltVideoObjectOnce(FRAME_BUFFER, "LAPTOP/Enrico_Y.sti", 0, 260, 225);
	}
	else if( giFilesPage == 5 )
	{
			// wedding pic
		BltVideoObjectOnce(FRAME_BUFFER, "LAPTOP/Enrico_W.sti", 0, 260, 85);
	}
}


static void AddStringToFilesList(wchar_t const* const pString)
{
	FileString* const fs = MALLOC(FileString);
	fs->Next    = 0;
	fs->pString = MALLOCN(wchar_t, wcslen(pString) + 1);
	wcscpy(fs->pString, pString);

	// Append node to list
	FileString** anchor = &pFileStringList;
	while (*anchor) anchor = &(*anchor)->Next;
	*anchor = fs;
}


static void ClearFileStringList(void)
{
	for (FileString* i = pFileStringList; i;)
	{
		FileString* const del = i;
		i = i->Next;
		MemFree(del->pString);
		MemFree(del);
	}
	pFileStringList = 0;
}


static void LoadPreviousPage()
{
	if (fWaitAFrame) return;
	if (giFilesPage == 0) return;

	--giFilesPage;
	fWaitAFrame       = TRUE;
	fReDrawScreenFlag = TRUE;
	MarkButtonsDirty();
}


static void LoadNextPage()
{
	if (fWaitAFrame) return;
	if (fOnLastFilesPageFlag) return;

	++giFilesPage;
	fWaitAFrame       = TRUE;
	fReDrawScreenFlag = TRUE;
	MarkButtonsDirty();
}


static void ScrollRegionCallback(MOUSE_REGION* const, INT32 const reason)
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


static void BtnNextFilePageCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnPreviousFilePageCallback(GUI_BUTTON *btn, INT32 reason);


static void CreateButtonsForFilesPage(void)
{
	// will create buttons for the files page
	giFilesPageButtons[0] = QuickCreateButtonImg("LAPTOP/arrows.sti", -1, 0, -1, 1, -1, PREVIOUS_FILE_PAGE_BUTTON_X, PREVIOUS_FILE_PAGE_BUTTON_Y, MSYS_PRIORITY_HIGHEST - 1, BtnPreviousFilePageCallback);
	giFilesPageButtons[1] = QuickCreateButtonImg("LAPTOP/arrows.sti", -1, 6, -1, 7, -1, NEXT_FILE_PAGE_BUTTON_X,     NEXT_FILE_PAGE_BUTTON_Y,     MSYS_PRIORITY_HIGHEST - 1, BtnNextFilePageCallback);

	giFilesPageButtons[0]->SetCursor(CURSOR_LAPTOP_SCREEN);
	giFilesPageButtons[1]->SetCursor(CURSOR_LAPTOP_SCREEN);

	UINT16 const x = FILE_VIEWER_X;
	UINT16 const y = FILE_VIEWER_Y;
	MSYS_DefineRegion(&g_scroll_region, x, y, x + FILE_VIEWER_W - 1, y + FILE_VIEWER_H - 1, MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, ScrollRegionCallback);
}


static void DeleteButtonsForFilesPage(void)
{
	// destroy buttons for the files page
	MSYS_RemoveRegion(&g_scroll_region);
	RemoveButton(giFilesPageButtons[ 0 ] );
	RemoveButton(giFilesPageButtons[ 1 ] );
}


static void BtnPreviousFilePageCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		LoadPreviousPage();
	}
}


static void BtnNextFilePageCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		LoadNextPage();
	}
}


static void HandleFileViewerButtonStates(void)
{
	// handle state of email viewer buttons

	if( iHighLightFileLine == -1 )
	{
		// not displaying message, leave
		DisableButton( giFilesPageButtons[ 0 ] );
		DisableButton( giFilesPageButtons[ 1 ] );
		giFilesPageButtons[0]->uiFlags &= ~BUTTON_CLICKED_ON;
		giFilesPageButtons[1]->uiFlags &= ~BUTTON_CLICKED_ON;


		return;
	}

	// turn off previous page button
	if( giFilesPage == 0 )
	{
		DisableButton( giFilesPageButtons[ 0 ] );
		giFilesPageButtons[0]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
	else
	{
		EnableButton( giFilesPageButtons[ 0 ] );
	}


	// turn off next page button
	if (fOnLastFilesPageFlag)
	{
		DisableButton( giFilesPageButtons[ 1 ] );
		giFilesPageButtons[1]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
	else
	{
		EnableButton( giFilesPageButtons[ 1 ] );
	}
}


static FileRecordWidth* CreateRecordWidth(INT32 iRecordNumber, INT32 iRecordWidth, INT32 iRecordHeightAdjustment, UINT8 ubFlags)
{
	// allocs and inits a width info record for the multipage file viewer...this will tell the procedure that does inital computation on which record is the start of the current page
	// how wide special records are ( ones that share space with pictures )
	FileRecordWidth* const pTempRecord = MALLOC(FileRecordWidth);
	pTempRecord -> Next = NULL;
	pTempRecord -> iRecordNumber = iRecordNumber;
	pTempRecord -> iRecordWidth = iRecordWidth;
	pTempRecord -> iRecordHeightAdjustment = iRecordHeightAdjustment;
	pTempRecord -> ubFlags = ubFlags;

	return ( pTempRecord );
}


static FileRecordWidth* CreateWidthRecordsForAruloIntelFile(void)
{
	// this fucntion will create the width list for the Arulco intelligence file
	FileRecordWidth* pTempRecord = NULL;
	FileRecordWidth* pRecordListHead = NULL;


		// first record width
//	pTempRecord = CreateRecordWidth( 7, 350, 200,0 );
	pTempRecord = CreateRecordWidth( FILES_COUNTER_1_WIDTH, 350, MAX_FILE_MESSAGE_PAGE_SIZE,0 );

	// set up head of list now
	pRecordListHead = pTempRecord;

	// next record
//	pTempRecord -> Next = CreateRecordWidth( 43, 200,0, 0 );
	pTempRecord -> Next = CreateRecordWidth( FILES_COUNTER_2_WIDTH, 200,0, 0 );
	pTempRecord = pTempRecord->Next;

	// and the next..
//	pTempRecord -> Next = CreateRecordWidth( 45, 200,0, 0 );
	pTempRecord -> Next = CreateRecordWidth( FILES_COUNTER_3_WIDTH, 200,0, 0 );
	pTempRecord = pTempRecord->Next;

	return( pRecordListHead );

}


static FileRecordWidth* CreateWidthRecordsForTerroristFile(void)
{
	// this fucntion will create the width list for the Arulco intelligence file
	FileRecordWidth* pTempRecord = NULL;
	FileRecordWidth* pRecordListHead = NULL;


		// first record width
	pTempRecord = CreateRecordWidth( 4, 170, 0,0 );

	// set up head of list now
	pRecordListHead = pTempRecord;

	// next record
	pTempRecord -> Next = CreateRecordWidth( 5, 170,0, 0 );
	pTempRecord = pTempRecord->Next;

	pTempRecord -> Next = CreateRecordWidth( 6, 170,0, 0 );
	pTempRecord = pTempRecord->Next;


	return( pRecordListHead );

}


static void ClearOutWidthRecordsList(FileRecordWidth* pFileRecordWidthList)
{
	FileRecordWidth* pTempRecord = NULL;
	FileRecordWidth* pDeleteRecord = NULL;

	// set up to head of the list
	pTempRecord = pDeleteRecord = pFileRecordWidthList;

	// error check
	if( pFileRecordWidthList == NULL )
	{
		return;
	}

	while( pTempRecord -> Next )
	{
		// set up delete record
		pDeleteRecord = pTempRecord;

		// move to next record
		pTempRecord = pTempRecord -> Next;

		MemFree( pDeleteRecord );
	}

	// now get the last element
	MemFree( pTempRecord );

	// null out passed ptr
	pFileRecordWidthList = NULL;
}


// open new files for viewing
static void OpenFirstUnreadFile(void)
{
	// open the first unread file in the list
	INT32 iCounter = 0;
	FilesUnit* pFilesList = pFilesListHead;

	// make sure is a valid
	 while( pFilesList )
   {

		 // if iCounter = iFileId, is a valid file
		if (!pFilesList->fRead)
		 {
			 iHighLightFileLine = iCounter;
		 }

		 // next element in list
		 pFilesList = pFilesList->Next;

		 // increment counter
		 iCounter++;
	 }
}


static void CheckForUnreadFiles(void)
{
	BOOLEAN	fStatusOfNewFileFlag = fNewFilesInFileViewer;

	// willc heck for any unread files and set flag if any
	FilesUnit* pFilesList = pFilesListHead;

	fNewFilesInFileViewer = FALSE;


	while( pFilesList )
  {
		// unread?...if so, set flag
		if (!pFilesList->fRead)
		{
			fNewFilesInFileViewer = TRUE;
		}
		// next element in list
		pFilesList = pFilesList->Next;
	}

	//if the old flag and the new flag arent the same, either create or destory the fast help region
	if( fNewFilesInFileViewer != fStatusOfNewFileFlag )
	{
		CreateFileAndNewEmailIconFastHelpText(LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE, !fNewFilesInFileViewer);
	}
}


static void HandleSpecialTerroristFile(INT32 const iFileNumber)
{
	INT32 iCounter = 0;
	FileString* pTempString = NULL;
	FileString* pLocatorString = NULL;
	INT32 iYPositionOnPage = 0;
	INT32 iFileLineWidth = 0;
	INT32 iFileStartX = 0;
	BOOLEAN fGoingOffCurrentPage = FALSE;
	FileRecordWidth* WidthList = NULL;
	INT32 iOffset = 0;

	iOffset = ubFileOffsets[ iFileNumber ] ;

	// grab width list
	WidthList = CreateWidthRecordsForTerroristFile( );


	while( iCounter < ubFileRecordsLength[ iFileNumber ] )
	{
		wchar_t sString[FILE_STRING_SIZE];
		LoadEncryptedDataFromFile("BINARYDATA/files.EDT", sString, FILE_STRING_SIZE * (iOffset + iCounter), FILE_STRING_SIZE);
		AddStringToFilesList( sString );
		iCounter++;
	}

	pTempString = pFileStringList;


	iYPositionOnPage = 0;
	iCounter = 0;
	pLocatorString = pTempString;

	pTempString = GetFirstStringOnThisPage( pFileStringList,FILES_TEXT_FONT,  350, FILE_GAP, giFilesPage, MAX_FILE_MESSAGE_PAGE_SIZE, WidthList);

		// find out where this string is
		while( pLocatorString != pTempString )
		{
			iCounter++;
			pLocatorString = pLocatorString -> Next;
		}


		// move through list and display
		while( pTempString )
		{
			const wchar_t* String = pTempString->pString;
			if (String[0] == L'\0')
			{
				// on last page
				fOnLastFilesPageFlag = TRUE;
			}

			// show picture
			if (giFilesPage == 0 && iCounter == 4)
			{
				char filename[128];
				sprintf(filename, "FACES/BIGFACES/%02d.sti", usProfileIdsForTerroristFiles[iFileNumber + 1]);
				BltVideoObjectOnce(FRAME_BUFFER, filename,                     0, FILE_VIEWER_X + 30, iYPositionOnPage + 76);
				BltVideoObjectOnce(FRAME_BUFFER, "LAPTOP/InterceptBorder.sti", 0, FILE_VIEWER_X + 25, iYPositionOnPage + 71);
			}

			// set up font
			Font font = FILES_TEXT_FONT;
			if( giFilesPage == 0 )
			{
				switch (iCounter)
				{
				  case 0: font = FILES_TITLE_FONT; break;
				}
			}

			if( ( iCounter > 3 ) && ( iCounter < 7 ) )
			{
				iFileLineWidth = 170;
				iFileStartX = (UINT16) ( FILE_VIEWER_X  +  180 );
			}
			else
			{
				// reset width
				iFileLineWidth = 350;
				iFileStartX = (UINT16) ( FILE_VIEWER_X +  10 );
			}

			// based on the record we are at, selected X start position and the width to wrap the line, to fit around pictures
			if (iYPositionOnPage + IanWrappedStringHeight(iFileLineWidth, FILE_GAP, font, String) < MAX_FILE_MESSAGE_PAGE_SIZE)
			{
     	   // now print it
		     iYPositionOnPage += IanDisplayWrappedString(iFileStartX, FILE_VIEWER_Y + 4 + iYPositionOnPage, iFileLineWidth, FILE_GAP, font, FILE_TEXT_COLOR, String, 0, IAN_WRAP_NO_SHADOW);
				 fGoingOffCurrentPage = FALSE;
			}
			else
			{
				 // gonna get cut off...end now
				 fGoingOffCurrentPage = TRUE;
			}

			pTempString = pTempString ->Next;

			if (pTempString == NULL && !fGoingOffCurrentPage)
			{
				// on last page
				fOnLastFilesPageFlag = TRUE;
			}
			else
			{
				fOnLastFilesPageFlag = FALSE;
			}

			// going over the edge, stop now
			if (fGoingOffCurrentPage)
			{
				pTempString = NULL;
			}

			iCounter++;
		}



		ClearOutWidthRecordsList( WidthList );
		ClearFileStringList( );
}


// add a file about this terrorist
BOOLEAN AddFileAboutTerrorist( INT32 iProfileId )
{
	INT32 iCounter = 0;

	for( iCounter = 1; iCounter < 7; iCounter++ )
	{
		if( usProfileIdsForTerroristFiles[ iCounter ] == iProfileId )
		{
			// checked, and this file is there
			AddFilesToPlayersLog(iCounter);
				return( TRUE );
		}
	}

	return( FALSE );
}
