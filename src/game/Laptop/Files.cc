#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Laptop.h"
#include "Files.h"
#include "LoadSaveData.h"
#include "MercPortrait.h"
#include "Object_Cache.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "VObject.h"
#include "Debug.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Text.h"
#include "Button_System.h"
#include "VSurface.h"
#include "Font_Control.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/string>


struct FilesUnit
{
	UINT8 ubCode; // the code index in the files code table
	BOOLEAN fRead;
	FilesUnit* Next; // next unit in the list
};


struct FileString
{
	ST::string pString;
	FileString* Next;
};


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


#define TOP_X				0+LAPTOP_SCREEN_UL_X
#define TOP_Y				LAPTOP_SCREEN_UL_Y
#define TITLE_X				(140 + STD_SCREEN_X)
#define TITLE_Y				(33 + STD_SCREEN_Y)
#define FILES_TITLE_FONT		FONT14ARIAL
#define FILES_TEXT_FONT			FONT10ARIAL//FONT12ARIAL
#define FILES_SENDER_TEXT_X		(FILES_LIST_X + 5)
#define MAX_FILES_LIST_LENGTH		28
#define FILE_VIEWER_X			(236 + STD_SCREEN_X)
#define FILE_VIEWER_Y			( 81 + STD_SCREEN_Y)
#define FILE_VIEWER_W			364
#define FILE_VIEWER_H			353
#define FILE_GAP			2
#define FILE_TEXT_COLOR			FONT_BLACK
#define FILE_STRING_SIZE		400
#define MAX_FILES_PAGE			MAX_FILES_LIST_LENGTH
#define FILES_LIST_X			(TOP_X + 10)
#define FILES_LIST_Y			(85 + STD_SCREEN_Y)
#define FILES_LIST_W			107
#define FILES_LIST_H			12
#define LENGTH_OF_ENRICO_FILE		68
#define MAX_FILE_MESSAGE_PAGE_SIZE	325
#define PREVIOUS_FILE_PAGE_BUTTON_X	(553 + STD_SCREEN_X)
#define PREVIOUS_FILE_PAGE_BUTTON_Y	(53 + STD_SCREEN_Y)
#define NEXT_FILE_PAGE_BUTTON_X		(577 + STD_SCREEN_X)
#define NEXT_FILE_PAGE_BUTTON_Y		PREVIOUS_FILE_PAGE_BUTTON_Y

#define FILES_COUNTER_1_WIDTH		7
#define FILES_COUNTER_2_WIDTH		43
#define FILES_COUNTER_3_WIDTH		45


// the highlighted line
static INT32 iHighLightFileLine;


// the files record list
static FilesUnit* pFilesListHead = NULL;

// are we in files mode
static BOOLEAN fInFilesMode=FALSE;
static BOOLEAN fOnLastFilesPageFlag = FALSE;


//. did we enter due to new file icon?
BOOLEAN fEnteredFileViewerFromNewFileIcon = FALSE;
static BOOLEAN fWaitAFrame = FALSE;

// are there any new files
BOOLEAN fNewFilesInFileViewer = FALSE;

namespace {
// graphics handles
cache_key_t const guiTITLE{ LAPTOPDIR "/programtitlebar.sti" };
cache_key_t const guiTOP{ LAPTOPDIR "/fileviewer.sti" };
}

// currewnt page of multipage files we are on
static INT32 giFilesPage = 0;
// strings

#define SLAY_LENGTH 12


struct FileInfo
{
	UINT16 profile_id;
	UINT16 file_offset;
};


static FileInfo const g_file_info[] =
{
	{ NO_PROFILE, 0               },
	{ SLAY,       0               }, // Slay
	{ ANNIE,      SLAY_LENGTH * 1 }, // MOM
	{ CHRIS,      SLAY_LENGTH * 2 }, // Imposter
	{ TIFFANY,    SLAY_LENGTH * 3 }, // Tiff
	{ T_REX,      SLAY_LENGTH * 4 }, // T-Rex
	{ DRUGGIST,   SLAY_LENGTH * 5 }  // Elgin
};


// buttons for next and previous pages
// (previous button at index 0, next button at index 1)
static GUIButtonRef giFilesPageButtons[2];
static MOUSE_REGION g_scroll_region;

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
	GCM->tempFiles()->deleteFile(FILES_DATA_FILE);
	ClearFilesList( );
	iHighLightFileLine = -1;

	// add background check by RIS
	AddFilesToPlayersLog(ENRICO_BACKGROUND);
}


static void CreateButtonsForFilesPage(void);
static void HandleFileViewerButtonStates(void);
static void InitializeFilesMouseRegions(void);
static void OpenFirstUnreadFile(void);


void EnterFiles(void)
{
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
static void RemoveFilesMouseRegions();


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

	BltVideoObject(FRAME_BUFFER, guiLaptopBACKGROUND, 0, STD_SCREEN_X + 108, STD_SCREEN_Y + 23);
}


static void RenderFilesBackGround(void)
{
	// render generic background for file system
	BltVideoObject(FRAME_BUFFER, guiTITLE, 0, TOP_X, TOP_Y -  2);
	BltVideoObject(FRAME_BUFFER, guiTOP,   0, TOP_X, TOP_Y + 22);
}


static void DrawFilesTitleText(void)
{
	// draw the pages title
	SetFontAttributes(FILES_TITLE_FONT, FONT_WHITE);
	MPrint(TITLE_X, TITLE_Y, pFilesTitle);
}


static void RemoveFiles(void)
{
	// delete files video objects from memory
	RemoveVObject(guiTOP);
	RemoveVObject(guiTITLE);
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

	FilesUnit* const f = new FilesUnit{};
	f->Next   = NULL;
	f->ubCode = ubCode;
	f->fRead  = fRead;

	*anchor = f;
}


#define FILE_ENTRY_SIZE 263


static void OpenAndReadFilesFile(void)
{
	ClearFilesList();

	if (!GCM->tempFiles()->exists(FILES_DATA_FILE)) {
		return;
	}

	// file exists, read in data, continue until file end
	AutoSGPFile f(GCM->tempFiles()->openForReading(FILES_DATA_FILE));

	for (UINT i = f->size() / FILE_ENTRY_SIZE; i != 0; --i)
	{
		BYTE data[FILE_ENTRY_SIZE];
		f->read(data, sizeof(data));

		UINT8 code;
		UINT8 already_read;

		DataReader d{data};
		EXTR_U8(d, code)
		EXTR_SKIP(d, 261)
		EXTR_U8(d, already_read)
		Assert(d.getConsumed() == lengthof(data));

		ProcessAndEnterAFilesRecord(code, already_read);
	}
}


static void OpenAndWriteFilesFile(void)
{
	AutoSGPFile f(GCM->tempFiles()->openForWriting(FILES_DATA_FILE, true));

	for (const FilesUnit* i = pFilesListHead; i; i = i->Next)
	{
		BYTE  data[FILE_ENTRY_SIZE];
		DataWriter d{data};
		INJ_U8(d, i->ubCode)
		INJ_SKIP(d, 261)
		INJ_U8(d, i->fRead)
		Assert(d.getConsumed() == lengthof(data));

		f->write(data, sizeof(data));
	}

	ClearFilesList();
}


static void ClearFilesList(void)
{
	FilesUnit* i   = pFilesListHead;
	pFilesListHead = NULL;
	while (i)
	{
		FilesUnit* const del = i;
		i = i->Next;
		delete del;
	}
}


static void DisplayFilesList(void)
{
	// this function will run through the list of files of files and display the 'sender'
	SetFontAttributes(FILES_TEXT_FONT, FONT_BLACK, NO_SHADOW);

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
	if (iHighLightFileLine != -1)
	{
		DisplayFormattedText();
	}
	else
	{
		HandleFileViewerButtonStates();
	}
}


static void FilesBtnCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


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


static void RemoveFilesMouseRegions()
{
	FOR_EACH(MOUSE_REGION, i, pFilesRegions) MSYS_RemoveRegion(&*i);
}


static void FilesBtnCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
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
static void HandleSpecialTerroristFile(INT32 file_idx);


static void DisplayFormattedText(void)
{
	fWaitAFrame = FALSE;

	UINT16 const white = Get16BPPColor(FROMRGB(255, 255, 255));
	INT32  const x     = FILE_VIEWER_X;
	INT32  const y     = FILE_VIEWER_Y;
	ColorFillVideoSurfaceArea(FRAME_BUFFER, x, y, x + FILE_VIEWER_W, y + FILE_VIEWER_H, white);

	// Get the file that was highlighted
	FilesUnit* fu = pFilesListHead;
	for (INT32 n = iHighLightFileLine; n != 0 && fu != nullptr; --n)
	{
		fu = fu->Next;
	}

	if (fu == nullptr)
	{
		// iHighLiftFileLine can be stale if one quits out of a game where the
		// terrorist files were available and then loads a game where they are
		// not. Reset state to initial in this case.
		iHighLightFileLine = -1;
		return;
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


static FileString const* GetFirstStringOnThisPage(FileString const* RecordList, SGPFont const font, UINT16 usWidth, UINT8 ubGap, INT32 iPage, INT32 iPageSize, FileRecordWidth* WidthList)
{
	// get the first record on this page - build pages up until this point
	FileString const* CurrentRecord = NULL;

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
		for (;;)
		{
			UINT16 const h = IanWrappedStringHeight(usCurrentWidth, ubGap, font, CurrentRecord->pString);
			if (iCurrentPositionOnThisPage + h >= iPageSize) break;

			// still room on this page
			iCurrentPositionOnThisPage += h;

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
		//iCounter++;

	}

	return ( CurrentRecord );
}


static FileString* LoadStringsIntoFileList(char const* const filename, UINT32 offset, size_t n)
{
	FileString*  head   = 0;
	FileString** anchor = &head;
	for (; n != 0; ++offset, --n)
	{
		ST::string str = GCM->loadEncryptedString(filename, FILE_STRING_SIZE * offset, FILE_STRING_SIZE);

		FileString* const fs = new FileString{};
		fs->Next    = 0;
		fs->pString = str;

		// Append node to list
		*anchor = fs;
		anchor  = &fs->Next;
	}
	return head;
}


namespace
{
	void ClearFileStringList(FileString* i)
	{
		while (i)
		{
			FileString* const del = i;
			i = i->Next;
			delete del;
		}
	}

	typedef SGP::AutoObj<FileString, ClearFileStringList> AutoStringList;
}


static void ClearOutWidthRecordsList(FileRecordWidth* pFileRecordWidthList);
static FileRecordWidth* CreateWidthRecordsForAruloIntelFile(void);


static void HandleSpecialFiles(void)
{
	FileRecordWidth*  const width_list = CreateWidthRecordsForAruloIntelFile();
	AutoStringList    const head(LoadStringsIntoFileList(BINARYDATADIR "/ris.edt", 0, LENGTH_OF_ENRICO_FILE));
	FileString const* const start      = GetFirstStringOnThisPage(head, FILES_TEXT_FONT, 350, FILE_GAP, giFilesPage, MAX_FILE_MESSAGE_PAGE_SIZE, width_list);
	ClearOutWidthRecordsList(width_list);

	// Find out where this string is
	FileString const* i      = head;
	INT32             clause = 0;
	for (; i != start; i = i->Next)
	{
		++clause;
	}

	// Move through list and display
	for (INT32 y = 0; i; clause++, i = i->Next)
	{
		SGPFont const font = giFilesPage == 0 && clause == 0 ?
			FILES_TITLE_FONT : FILES_TEXT_FONT;

		// Based on the record we are at, selected X start position and the width to
		// wrap the line, to fit around pictures
		INT32 max_width = 350;
		INT32 start_x   = FILE_VIEWER_X +  10;
		switch (clause)
		{
			case FILES_COUNTER_1_WIDTH:
				if (giFilesPage == 0) y = MAX_FILE_MESSAGE_PAGE_SIZE;
				break;

			case FILES_COUNTER_2_WIDTH:
			case FILES_COUNTER_3_WIDTH:
				max_width = 200;
				start_x   = FILE_VIEWER_X + 150;
				break;
		}

		ST::string txt = i->pString;
		if (y + IanWrappedStringHeight(max_width, FILE_GAP, font, txt) >= MAX_FILE_MESSAGE_PAGE_SIZE)
		{
			// gonna get cut off, end now
			break;
		}

		y += IanDisplayWrappedString(start_x, FILE_VIEWER_Y + 4 + y, max_width, FILE_GAP, font, FILE_TEXT_COLOR, txt, 0, IAN_WRAP_NO_SHADOW);
	}
	fOnLastFilesPageFlag = !i;

	// place pictures
	switch (giFilesPage)
	{
		case 0: BltVideoObjectOnce(FRAME_BUFFER, LAPTOPDIR "/arucofilesmap.sti", 0, STD_SCREEN_X + 300, STD_SCREEN_Y + 270); break; // Picture of country
		case 4: BltVideoObjectOnce(FRAME_BUFFER, LAPTOPDIR "/enrico_y.sti",      0, STD_SCREEN_X + 260, STD_SCREEN_Y + 225); break; // Kid pic
		case 5: BltVideoObjectOnce(FRAME_BUFFER, LAPTOPDIR "/enrico_w.sti",      0, STD_SCREEN_X + 260, STD_SCREEN_Y +  85); break; // Wedding pic
	}
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


static void BtnNextFilePageCallback(GUI_BUTTON *btn, UINT32 reason);
static void BtnPreviousFilePageCallback(GUI_BUTTON *btn, UINT32 reason);


static void CreateButtonsForFilesPage(void)
{
	// will create buttons for the files page
	giFilesPageButtons[0] = QuickCreateButtonImg(LAPTOPDIR "/arrows.sti", 0, 1, PREVIOUS_FILE_PAGE_BUTTON_X, PREVIOUS_FILE_PAGE_BUTTON_Y, MSYS_PRIORITY_HIGHEST - 1, BtnPreviousFilePageCallback);
	giFilesPageButtons[1] = QuickCreateButtonImg(LAPTOPDIR "/arrows.sti", 6, 7, NEXT_FILE_PAGE_BUTTON_X,     NEXT_FILE_PAGE_BUTTON_Y,     MSYS_PRIORITY_HIGHEST - 1, BtnNextFilePageCallback);

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


static void BtnPreviousFilePageCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		LoadPreviousPage();
	}
}


static void BtnNextFilePageCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		LoadNextPage();
	}
}


static void HandleFileViewerButtonStates(void)
{
	if (iHighLightFileLine == -1)
	{
		// not displaying message, leave
		DisableButton(giFilesPageButtons[0]);
		DisableButton(giFilesPageButtons[1]);
		return;
	}

	// Turn on/off previous page button
	EnableButton(giFilesPageButtons[0], giFilesPage != 0);
	// Turn on/off next page button
	EnableButton(giFilesPageButtons[1], !fOnLastFilesPageFlag);
}


static FileRecordWidth* CreateRecordWidth(INT32 iRecordNumber, INT32 iRecordWidth, INT32 iRecordHeightAdjustment, UINT8 ubFlags)
{
	// allocs and inits a width info record for the multipage file viewer...this will tell the procedure that does inital computation on which record is the start of the current page
	// how wide special records are ( ones that share space with pictures )
	FileRecordWidth* const pTempRecord = new FileRecordWidth{};
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
	//pTempRecord = CreateRecordWidth( 7, 350, 200,0 );
	pTempRecord = CreateRecordWidth( FILES_COUNTER_1_WIDTH, 350, MAX_FILE_MESSAGE_PAGE_SIZE,0 );

	// set up head of list now
	pRecordListHead = pTempRecord;

	// next record
	//pTempRecord -> Next = CreateRecordWidth( 43, 200,0, 0 );
	pTempRecord -> Next = CreateRecordWidth( FILES_COUNTER_2_WIDTH, 200,0, 0 );
	pTempRecord = pTempRecord->Next;

	// and the next..
	//pTempRecord -> Next = CreateRecordWidth( 45, 200,0, 0 );
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


static void ClearOutWidthRecordsList(FileRecordWidth* i)
{
	while (i)
	{
		FileRecordWidth* const del = i;
		i = i->Next;
		delete del;
	}
}


// open new files for viewing
static void OpenFirstUnreadFile(void)
{
	// open the first unread file in the list
	INT32 i = 0;
	for (FilesUnit* fu = pFilesListHead; fu; ++i, fu = fu->Next)
	{
		if (fu->fRead) continue;
		iHighLightFileLine = i;
	}
}


static void CheckForUnreadFiles(void)
{
	// will check for any unread files and set flag if any
	BOOLEAN any_unread = FALSE;
	for (FilesUnit const* i = pFilesListHead; i; i = i->Next)
	{
		if (i->fRead) continue;
		any_unread = TRUE;
		break;
	}

	// If the old flag and the new flag aren't the same, either create or destory
	// the fast help region
	if (fNewFilesInFileViewer == any_unread) return;

	fNewFilesInFileViewer = any_unread;
	CreateFileAndNewEmailIconFastHelpText(LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE, !fNewFilesInFileViewer);
}


static void HandleSpecialTerroristFile(INT32 const file_idx)
{

	FileRecordWidth*  const width_list = CreateWidthRecordsForTerroristFile();
	FileInfo   const&       info       = g_file_info[file_idx];
	AutoStringList    const head(LoadStringsIntoFileList(BINARYDATADIR "/files.edt", info.file_offset, SLAY_LENGTH));
	FileString const* const start      = GetFirstStringOnThisPage(head, FILES_TEXT_FONT, 350, FILE_GAP, giFilesPage, MAX_FILE_MESSAGE_PAGE_SIZE, width_list);
	ClearOutWidthRecordsList(width_list);

	// Find out where this string is
	FileString const* i      = head;
	INT32             clause = 0;
	for (; i != start; i = i->Next)
	{
		++clause;
	}

	// Move through list and display
	for (INT32 y = 0; i; ++clause, i = i->Next)
	{
		// Show picture
		if (giFilesPage == 0 && clause == 4)
		{
			AutoSGPVObject vo(LoadBigPortrait(GetProfile(info.profile_id)));
			BltVideoObject(    FRAME_BUFFER, vo.get(),                         0, FILE_VIEWER_X + 30, FILE_VIEWER_Y + 136);
			BltVideoObjectOnce(FRAME_BUFFER, LAPTOPDIR "/interceptborder.sti", 0, FILE_VIEWER_X + 25, FILE_VIEWER_Y + 131);
		}

		SGPFont const font = giFilesPage == 0 && clause == 0 ?
			FILES_TITLE_FONT : FILES_TEXT_FONT;

		// Based on the record we are at, selected X start position and the width to
		// wrap the line, to fit around pictures
		INT32 max_width;
		INT32 start_x;
		if (4 <= clause && clause < 7)
		{
			max_width = 170;
			start_x   = FILE_VIEWER_X + 180;
		}
		else
		{
			max_width = 350;
			start_x   = FILE_VIEWER_X + 10;
		}

		ST::string txt = i->pString;
		if (y + IanWrappedStringHeight(max_width, FILE_GAP, font, txt) >= MAX_FILE_MESSAGE_PAGE_SIZE)
		{
			// gonna get cut off, end now
			break;
		}

		y += IanDisplayWrappedString(start_x, FILE_VIEWER_Y + 4 + y, max_width, FILE_GAP, font, FILE_TEXT_COLOR, txt, 0, IAN_WRAP_NO_SHADOW);
	}
	fOnLastFilesPageFlag = !i;
}


void AddFilesAboutTerrorists()
{
	for (INT32 i = 1; i != lengthof(g_file_info); ++i)
	{
		AddFilesToPlayersLog(i);
	}
}
