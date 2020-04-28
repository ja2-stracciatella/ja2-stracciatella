#include "Button_System.h"
#include "Directories.h"
#include "FileMan.h"
#include "Font.h"
#include "Font_Control.h"
#include "HImage.h"
#include "Local.h"
#include "RenderWorld.h"
#include "Render_Dirty.h"
#include "LoadScreen.h"
#include "SelectWin.h"
#include "EditorDefines.h"
#include "MessageBox.h"
#include "Text_Input.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "EditorBuildings.h"
#include "Editor_Taskbar_Utils.h"
#include "Editor_Undo.h"
#include "EditScreen.h"
#include "StrategicMap.h"
#include "Editor_Modes.h"
#include "Map_Information.h"
#include "Sys_Globals.h"
#include "Sector_Summary.h"
#include "NewSmooth.h"
#include "Simple_Render_Utils.h"
#include "Animated_ProgressBar.h"
#include "EditorMercs.h"
#include "Lighting.h"
#include "EditorMapInfo.h"
#include "Environment.h"
#include "Edit_Sys.h"
#include "EditorItems.h"
#include "English.h"
#include "GameLoop.h"
#include "Message.h"
#include "Pits.h"
#include "Item_Statistics.h"
#include "Scheduling.h"
#include "Debug.h"
#include "JAScreens.h"
#include "MemMan.h"
#include "MessageBoxScreen.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "WorldDef.h"
#include "UILayout.h"
#include "GameState.h"
#include "GameRes.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <cstdarg>


static BOOLEAN gfErrorCatch            = FALSE;
static ST::string gzErrorCatchString;


void SetErrorCatchString(const ST::string& str)
{
	gzErrorCatchString = str;
	gfErrorCatch = TRUE;
}


enum{
	DIALOG_NONE,
	DIALOG_SAVE,
	DIALOG_LOAD,
	DIALOG_CANCEL,
	DIALOG_DELETE
};

static INT32 iTotalFiles;
static INT32 iTopFileShown;
static INT32 iCurrFileShown;
static INT32	iLastFileClicked;
static INT32 iLastClickTime;

static ST::string gzFilename;

static FDLG_LIST* FileList = NULL;

static INT32 iFDlgState = DIALOG_NONE;
static GUIButtonRef iFileDlgButtons[7];

static BOOLEAN gfLoadError;
static bool gfReadOnly;
static BOOLEAN gfFileExists;
static BOOLEAN gfIllegalName;
static BOOLEAN gfDeleteFile;
static BOOLEAN gfNoFiles;

static BOOLEAN fEnteringLoadSaveScreen = TRUE;

static MOUSE_REGION BlanketRegion;

static ST::string gMapFileForRemoval;

enum{
	IOSTATUS_NONE,
	INITIATE_MAP_SAVE,
	SAVING_MAP,
	INITIATE_MAP_LOAD,
	LOADING_MAP
};
static INT8 gbCurrentFileIOStatus; // 1 init saving message, 2 save, 3 init loading message, 4 load, 0 none

static bool gfUpdateSummaryInfo = true;


static void CreateFileDialog(const ST::string& zTitle);
static void TrashFDlgList(FDLG_LIST*);


static void LoadSaveScreenEntry(void)
{
	fEnteringLoadSaveScreen = FALSE;
	gbCurrentFileIOStatus	= IOSTATUS_NONE;

	gfReadOnly = false;
	gfFileExists = FALSE;
	gfLoadError = FALSE;
	gfIllegalName = FALSE;
	gfDeleteFile = FALSE;
	gfNoFiles = FALSE;

	// setup filename dialog box
	// (*.dat and *.map) as file filter

	// If user clicks on a filename in the window, then turn off string focus and re-init the string with the new name.
	// If user hits ENTER or presses OK, then continue with the file loading/saving

	if( FileList )
		TrashFDlgList( FileList );

	iTopFileShown = iTotalFiles = 0;
	try
	{
		std::vector<ST::string> files = GCM->getAllMaps();
		for (const ST::string &file : files)
		{
			FileList = AddToFDlgList(FileList, file.c_str());
			++iTotalFiles;
		}
	}
	catch (...) { /* XXX ignore */ }

	gzFilename = ST::format("{}", g_filename);

	CreateFileDialog(iCurrentAction == ACTION_SAVE_MAP ? "Save Map (*.dat)" : "Load Map (*.dat)");

	if( !iTotalFiles )
	{
		gfNoFiles = TRUE;
		if( iCurrentAction == ACTION_LOAD_MAP )
			DisableButton( iFileDlgButtons[0] );
	}

	iLastFileClicked = -1;
	iLastClickTime = 0;

}


static void RemoveFileDialog(void);
static void RemoveFromFDlgList(FDLG_LIST** head, FDLG_LIST* node);
static BOOLEAN ValidFilename(void);


static ScreenID ProcessLoadSaveScreenMessageBoxResult(void)
{
	FDLG_LIST *curr, *temp;
	gfRenderWorld = TRUE;
	RemoveMessageBox();
	if( gfIllegalName )
	{
		fEnteringLoadSaveScreen = TRUE;
		RemoveFileDialog();
		MarkWorldDirty();
		return gfMessageBoxResult ? LOADSAVE_SCREEN : EDIT_SCREEN;
	}
	if( gfDeleteFile )
	{
		if( gfMessageBoxResult )
		{ //delete file
			INT32 x;
			curr = FileList;
			for( x = 0; x < iCurrFileShown && x < iTotalFiles && curr; x++ )
			{
				curr = curr->pNext;
			}
			if( curr )
			{
				FileDelete(gMapFileForRemoval);

				//File is deleted so redo the text fields so they show the
				//next file in the list.
				temp = curr->pNext;
				if( !temp )
					temp = curr->pPrev;
				if( !temp )
					gzFilename = ST::null;
				else
					gzFilename = ST::format("{}", temp->filename);
				if (!ValidFilename()) gzFilename = ST::null;
				SetInputFieldString(0, gzFilename);
				RemoveFromFDlgList( &FileList, curr );
				iTotalFiles--;
				if( !iTotalFiles )
				{
					gfNoFiles = TRUE;
					if( iCurrentAction == ACTION_LOAD_MAP )
						DisableButton( iFileDlgButtons[0] );
				}
				if( iCurrFileShown >= iTotalFiles )
					iCurrFileShown--;
				if( iCurrFileShown < iTopFileShown )
					iTopFileShown -= 8;
				if( iTopFileShown < 0 )
					iTopFileShown = 0;
			}
		}
		MarkWorldDirty();
		RenderWorld();
		gfDeleteFile = FALSE;
		iFDlgState = DIALOG_NONE;
		return LOADSAVE_SCREEN;
	}
	if( gfLoadError )
	{
		fEnteringLoadSaveScreen = TRUE;
		return gfMessageBoxResult ? LOADSAVE_SCREEN : EDIT_SCREEN;
	}
	if( gfReadOnly )
	{ //file is readonly.  Result will determine if the file dialog stays up.
		fEnteringLoadSaveScreen = TRUE;
		RemoveFileDialog();
		return gfMessageBoxResult ? LOADSAVE_SCREEN : EDIT_SCREEN;
	}
	if( gfFileExists )
	{
		if( gfMessageBoxResult )
		{ //okay to overwrite file
			RemoveFileDialog();
			gbCurrentFileIOStatus = INITIATE_MAP_SAVE;
			return LOADSAVE_SCREEN;
		}
		fEnteringLoadSaveScreen = TRUE;
		RemoveFileDialog();
		return EDIT_SCREEN ;
	}
	SLOGA("ProcessLoadSaveScreenMessageBoxResult: none of the global flags set");
	return LOADSAVE_SCREEN;
}


static void DrawFileDialog(void);
static BOOLEAN ExtractFilenameFromFields(void);
static void HandleMainKeyEvents(InputAtom* pEvent);
static ScreenID ProcessFileIO(void);


ScreenID LoadSaveScreenHandle(void)
{
	FDLG_LIST *FListNode;
	INT32 x;
	InputAtom DialogEvent;
	ST::string zOrigName;

	if( fEnteringLoadSaveScreen )
	{
		LoadSaveScreenEntry();
	}

	if( gbCurrentFileIOStatus ) //loading or saving map
	{
		ScreenID const uiScreen = ProcessFileIO();
		if( uiScreen == EDIT_SCREEN && gbCurrentFileIOStatus == LOADING_MAP )
			RemoveProgressBar( 0 );
		return uiScreen;
	}

	if( gubMessageBoxStatus )
	{
		if( MessageBoxHandled() )
			return ProcessLoadSaveScreenMessageBoxResult();
		return LOADSAVE_SCREEN;
	}

	//handle all key input.
	while( DequeueEvent(&DialogEvent) )
	{
		if( !HandleTextInput(&DialogEvent) && (DialogEvent.usEvent == KEY_DOWN || DialogEvent.usEvent == KEY_REPEAT) )
		{
			HandleMainKeyEvents( &DialogEvent );
		}
	}

	DrawFileDialog();

	// Skip to first filename to show
	FListNode = FileList;
	for(x=0;x<iTopFileShown && x<iTotalFiles && FListNode != NULL;x++)
	{
		FListNode = FListNode->pNext;
	}

	// Show up to 8 filenames in the window
	SetFont( FONT12POINT1 );
	if( gfNoFiles )
	{
		SetFontForeground( FONT_LTRED );
		SetFontBackground( 142 );
		MPrint(226, 126, "NO FILES IN /MAPS DIRECTORY");
	}
	else for(x=iTopFileShown;x<(iTopFileShown+8) && x<iTotalFiles && FListNode != NULL; x++)
	{
		if( !EditingText() && x == iCurrFileShown  )
		{
			SetFontForeground( FONT_GRAY2 );
			SetFontBackground( FONT_METALGRAY );
		}
		else
		{
			SetFontForeground( FONT_BLACK );
			SetFontBackground( 142 );
		}
		MPrint(186, 73 + (x - iTopFileShown) * 15, ST::format("{}", FListNode->filename));
		FListNode = FListNode->pNext;
	}

	RenderAllTextFields();

	InvalidateScreen();

	ExecuteBaseDirtyRectQueue();

	switch( iFDlgState )
	{
		case DIALOG_CANCEL:
			RemoveFileDialog();
			fEnteringLoadSaveScreen = TRUE;
			return EDIT_SCREEN;

		case DIALOG_DELETE:
		{
			gMapFileForRemoval = GCM->getMapPath(gzFilename);
			bool readonly = false;
			if (Fs_getReadOnly(gMapFileForRemoval.c_str(), &readonly))
			{
				ST::string str;
				if (readonly)
				{
					str = ST::format(" Delete READ-ONLY file {}? ", gzFilename);
				}
				else
					str = ST::format(" Delete file {}? ", gzFilename);
				gfDeleteFile = TRUE;
				CreateMessageBox( str );
			}
			return LOADSAVE_SCREEN;
		}

		case DIALOG_SAVE:
		{
			if( !ExtractFilenameFromFields() )
			{
				CreateMessageBox( " Illegal filename.  Try another filename? " );
				gfIllegalName = TRUE;
				iFDlgState = DIALOG_NONE;
				return LOADSAVE_SCREEN;
			}
			ST::string filename(GCM->getMapPath(gzFilename));
			if ( GCM->doesGameResExists(filename.c_str()) )
			{
				gfFileExists = TRUE;
				gfReadOnly = false;
				Fs_getReadOnly(filename.c_str(), &gfReadOnly);
				if( gfReadOnly )
					CreateMessageBox( " File is read only!  Choose a different name? " );
				else
					CreateMessageBox( " File exists, Overwrite? " );
				return( LOADSAVE_SCREEN );
			}
			RemoveFileDialog();
			gbCurrentFileIOStatus = INITIATE_MAP_SAVE;
			return LOADSAVE_SCREEN ;
		}
		case DIALOG_LOAD:
			if( !ExtractFilenameFromFields() )
			{
				CreateMessageBox( " Illegal filename.  Try another filename? " );
				gfIllegalName = TRUE;
				iFDlgState = DIALOG_NONE;
				return LOADSAVE_SCREEN;
			}
			RemoveFileDialog();
			CreateProgressBar(0, 118, 183, 404, 19);
			DefineProgressBarPanel( 0, 65, 79, 94, 100, 155, 540, 235 );
			zOrigName = ST::format("Loading map:  {}", gzFilename);
			SetProgressBarTitle( 0, zOrigName, BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
			gbCurrentFileIOStatus = INITIATE_MAP_LOAD;
			return LOADSAVE_SCREEN ;
		default:
			iFDlgState = DIALOG_NONE;
	}
	iFDlgState = DIALOG_NONE;
	return LOADSAVE_SCREEN ;
}


static GUIButtonRef MakeButtonArrow(const char* const gfx, const INT16 y, const GUI_CALLBACK click)
{
	GUIButtonRef const btn = QuickCreateButtonImg(gfx, -1, 1, 2, 3, 4, 426, y, MSYS_PRIORITY_HIGH, click);
	btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);
	return btn;
}


static void FDlgCancelCallback(GUI_BUTTON* butn, INT32 reason);
static void FDlgDwnCallback(GUI_BUTTON* butn, INT32 reason);
static void FDlgNamesCallback(GUI_BUTTON* butn, INT32 reason);
static void FDlgOkCallback(GUI_BUTTON* butn, INT32 reason);
static void FDlgUpCallback(GUI_BUTTON* butn, INT32 reason);
static void FileDialogModeCallback(UINT8 ubID, BOOLEAN fEntering);
static void UpdateWorldInfoCallback(GUI_BUTTON* b, INT32 reason);


static void CreateFileDialog(const ST::string& zTitle)
{

	iFDlgState = DIALOG_NONE;

	DisableEditorTaskbar();

	MSYS_DefineRegion( &BlanketRegion, 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_END_Y, MSYS_PRIORITY_HIGH - 5, 0, 0, 0 );

	//Okay and cancel buttons
	iFileDlgButtons[0] = CreateTextButton("Okay",   FONT12POINT1, FONT_BLACK, FONT_BLACK, 354, 225, 50, 30, MSYS_PRIORITY_HIGH, FDlgOkCallback);
	iFileDlgButtons[1] = CreateTextButton("Cancel", FONT12POINT1, FONT_BLACK, FONT_BLACK, 406, 225, 50, 30, MSYS_PRIORITY_HIGH, FDlgCancelCallback);

	//Scroll buttons
	iFileDlgButtons[2] = MakeButtonArrow(EDITORDIR "/uparrow.sti",    92, FDlgUpCallback);
	iFileDlgButtons[3] = MakeButtonArrow(EDITORDIR "/downarrow.sti", 182, FDlgDwnCallback);

	//File list window
	iFileDlgButtons[4] = CreateHotSpot(179 + 4, 69 + 3, 179 + 4 + 240, 69 + 120 + 3, MSYS_PRIORITY_HIGH - 1, FDlgNamesCallback);
	//Title button
	iFileDlgButtons[5] = CreateLabel(zTitle, HUGEFONT, FONT_LTKHAKI, FONT_DKKHAKI, 179, 39, 281, 30, MSYS_PRIORITY_HIGH - 2);

	if( iCurrentAction == ACTION_SAVE_MAP )
	{	//checkboxes
		//The update world info checkbox
		iFileDlgButtons[6] = CreateCheckBoxButton( 183, 229, EDITORDIR "/smcheckbox.sti", MSYS_PRIORITY_HIGH, UpdateWorldInfoCallback );
		if( gfUpdateSummaryInfo )
			iFileDlgButtons[6]->uiFlags |= BUTTON_CLICKED_ON;
	}

	//Add the text input fields
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	//field 1 (filename)
	AddTextInputField(/*233*/183, 195, 190, 20, MSYS_PRIORITY_HIGH, gzFilename, 30, INPUTTYPE_DOSFILENAME);
	//field 2 -- user field that allows mouse/key interaction with the filename list
	AddUserInputField( FileDialogModeCallback );
}


static void UpdateWorldInfoCallback(GUI_BUTTON* b, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
		gfUpdateSummaryInfo = b->Clicked();
}


//This is a hook into the text input code.  This callback is called whenever the user is currently
//editing text, and presses Tab to transfer to the file dialog mode.  When this happens, we set the text
//field to the currently selected file in the list which is already know.
static void FileDialogModeCallback(UINT8 ubID, BOOLEAN fEntering)
{
	INT32 x;
	FDLG_LIST *FListNode;
	if( fEntering )
	{
		// Skip to first filename
		FListNode = FileList;
		for(x=0;x<iTopFileShown && x<iTotalFiles && FListNode != NULL;x++)
		{
			FListNode = FListNode->pNext;
		}
		// Find the already selected filename
		for(x = iTopFileShown; x < iTopFileShown + 8 && x < iTotalFiles && FListNode != NULL; x++ )
		{
			if( iCurrFileShown == (x-iTopFileShown) )
			{
				FListNode->filename[30] = '\0';
				SetInputFieldString(0, FListNode->filename);
				return;
			}
			FListNode = FListNode->pNext;
		}
	}
}


static void RemoveFileDialog(void)
{
	INT32 x;

	MSYS_RemoveRegion( &BlanketRegion );

	for(x=0; x<6; x++)
	{
		RemoveButton(iFileDlgButtons[x]);
	}

	if (iFileDlgButtons[6]) RemoveButton(iFileDlgButtons[6]);

	TrashFDlgList( FileList );
	FileList = NULL;

	InvalidateScreen( );

	EnableEditorTaskbar();
	KillTextInputMode();
	MarkWorldDirty();
	RenderWorld();
}


static void DrawFileDialog(void)
{
	ColorFillVideoSurfaceArea(FRAME_BUFFER,	179, 69, (179+281), 261, Get16BPPColor(FROMRGB(136, 138, 135)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER,	180, 70, (179+281), 261, Get16BPPColor(FROMRGB(24, 61, 81)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER,	180, 70, (179+280), 260, Get16BPPColor(FROMRGB(65, 79, 94)) );

	ColorFillVideoSurfaceArea(FRAME_BUFFER, (179+4), (69+3), (179+4+240), (69+123), Get16BPPColor(FROMRGB(24, 61, 81)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER, (179+5), (69+4), (179+4+240), (69+123), Get16BPPColor(FROMRGB(136, 138, 135)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER, (179+5), (69+4), (179+3+240), (69+122), Get16BPPColor(FROMRGB(250, 240, 188)) );

	MarkButtonsDirty();
	RenderButtons();
	RenderButtonsFastHelp();

	SetFontAttributes(FONT10ARIAL, FONT_LTKHAKI, FONT_DKKHAKI);
	MPrint(183, 217, "Filename");

	if (iFileDlgButtons[6]) MPrint(200, 231, "Update world info");
}


//The callback calls this function passing the relative y position of where
//the user clicked on the hot spot.
static void SelectFileDialogYPos(UINT16 usRelativeYPos)
{
	INT16 sSelName;
	INT32 x;
	FDLG_LIST *FListNode;

	sSelName = usRelativeYPos / 15;

	//This is a field in the text editmode, but clicked via mouse.
	SetActiveField( 1 );

	// Skip to first filename
	FListNode = FileList;
	for(x=0;x<iTopFileShown && x<iTotalFiles && FListNode != NULL;x++)
	{
		FListNode = FListNode->pNext;
	}

	for(x=iTopFileShown;x<(iTopFileShown+8) && x<iTotalFiles && FListNode != NULL; x++)
	{
		if( (INT32)sSelName == (x-iTopFileShown) )
		{
			INT32 iCurrClickTime;
			iCurrFileShown = x;
			FListNode->filename[30] = '\0';
			gzFilename = ST::format("{}", FListNode->filename);
			if (!ValidFilename()) gzFilename = ST::null;
			SetInputFieldString(0, gzFilename);

			RenderInactiveTextField( 0 );

			//Calculate and process any double clicking...
			iCurrClickTime = GetJA2Clock();
			if( iCurrClickTime - iLastClickTime < 400 && x == iLastFileClicked )
			{ //Considered a double click, so activate load/save this filename.
				iFDlgState = iCurrentAction == ACTION_SAVE_MAP ? DIALOG_SAVE : DIALOG_LOAD;
			}
			iLastClickTime = iCurrClickTime;
			iLastFileClicked = x;
		}
		FListNode = FListNode->pNext;
	}
}


FDLG_LIST* AddToFDlgList(FDLG_LIST* const list, char const* const filename)
{ // Add and sort alphabetically without regard to case
	FDLG_LIST* prev = 0;
	FDLG_LIST* i;
	for (i = list; i; prev = i, i = i->pNext)
	{
		if (strcasecmp(i->filename, filename) > 0) break;
	}
	FDLG_LIST* const n = new FDLG_LIST{};
	strlcpy(n->filename, filename, lengthof(n->filename));
	n->pPrev = prev;
	n->pNext = i;
	if (i)    i->pPrev    = n;
	if (prev) prev->pNext = n;
	return prev ? list : n;
}


static void RemoveFromFDlgList(FDLG_LIST** const head, FDLG_LIST* const node)
{
	for (FDLG_LIST* i = *head; i; i = i->pNext)
	{
		if (i != node) continue;
		if (*head == node) *head = (*head)->pNext;
		FDLG_LIST* const prev = i->pPrev;
		FDLG_LIST* const next = i->pNext;
		if (prev) prev->pNext = next;
		if (next) next->pPrev = prev;
		delete node;
		break;
	}
}


static void TrashFDlgList(FDLG_LIST* i)
{
	while (i)
	{
		FDLG_LIST* const del = i;
		i = i->pNext;
		delete del;
	}
}


static void SetTopFileToLetter(UINT16 usLetter)
{
	UINT32 x;
	FDLG_LIST *curr;
	FDLG_LIST *prev;
	UINT16 usNodeLetter;

	// Skip to first filename
	x = 0;
	curr = prev = FileList;
	while( curr )
	{
		usNodeLetter = curr->filename[0]; //first letter of filename.
		if( usNodeLetter < 'a' )
			usNodeLetter += 32; //convert uppercase to lower case A=65, a=97
		if( usLetter <= usNodeLetter )
			break;
		prev = curr;
		curr = curr->pNext;
		x++;
	}
	if( FileList )
	{
		iCurrFileShown = x;
		iTopFileShown = x;
		if( iTopFileShown > iTotalFiles - 7 )
			iTopFileShown = iTotalFiles - 7;
		SetInputFieldString(0, prev->filename);
	}
}


static void HandleMainKeyEvents(InputAtom* pEvent)
{
	INT32 iPrevFileShown = iCurrFileShown;
	//Replace Alt-x press with ESC.
	if( pEvent->usKeyState & ALT_DOWN && pEvent->usParam == 'x' )
		pEvent->usParam = SDLK_ESCAPE;
	switch( pEvent->usParam )
	{
		case SDLK_RETURN:
			if( gfNoFiles && iCurrentAction == ACTION_LOAD_MAP )
				break;
			iFDlgState = iCurrentAction == ACTION_SAVE_MAP ? DIALOG_SAVE : DIALOG_LOAD;
			break;

		case SDLK_ESCAPE:
			iFDlgState = DIALOG_CANCEL;
			break;

		case SDLK_PAGEUP:
			if( iTopFileShown > 7 )
			{
				iTopFileShown -= 7;
				iCurrFileShown -= 7;
			}
			else
			{
				iTopFileShown = 0;
				iCurrFileShown = 0;
			}
			break;

		case SDLK_PAGEDOWN:
			iTopFileShown += 7;
			iCurrFileShown += 7;
			if( iTopFileShown > iTotalFiles-7 )
				iTopFileShown = iTotalFiles - 7;
			if( iCurrFileShown >= iTotalFiles )
				iCurrFileShown = iTotalFiles - 1;
			break;

		case SDLK_UP:
			if( iCurrFileShown > 0 )
				iCurrFileShown--;
			if( iTopFileShown > iCurrFileShown )
				iTopFileShown = iCurrFileShown;
			break;

		case SDLK_DOWN:
			iCurrFileShown++;
			if( iCurrFileShown >= iTotalFiles )
				iCurrFileShown = iTotalFiles - 1;
			else if( iTopFileShown < iCurrFileShown-7 )
				iTopFileShown++;
			break;

		case SDLK_HOME:
			iTopFileShown = 0;
			iCurrFileShown = 0;
			break;

		case SDLK_END:
			iTopFileShown = iTotalFiles-7;
			iCurrFileShown = iTotalFiles-1;
			break;

		case SDLK_DELETE: iFDlgState = DIALOG_DELETE; break;

		default:
			//This case handles jumping the file list to display the file with the letter pressed.
			if (pEvent->usParam >= SDLK_a && pEvent->usParam <= SDLK_z)
			{
				SetTopFileToLetter( (UINT16)pEvent->usParam );
			}
			break;
	}
	//Update the text field if the file value has changed.
	if( iCurrFileShown != iPrevFileShown )
	{
		INT32 x;
		FDLG_LIST *curr;
		x = 0;
		curr = FileList;
		while( curr && x != iCurrFileShown )
		{
			curr = curr->pNext;
			x++;
		}
		if( curr )
		{
			SetInputFieldString(0, curr->filename);
			gzFilename = ST::format("{}", curr->filename);
		}
	}
}


// Editor doesn't care about the z value. It uses its own methods.
static void SetGlobalSectorValues()
{
	{ const char* f = gzFilename.c_str();

		INT16 y;
		if ('A' <= f[0] && f[0] <= 'P')
		{
			y = f[0] - 'A' + 1;
		}
		else if ('a' <= f[0] && f[0] <= 'p')
		{
			y = f[0] - 'a' + 1;
		}
		else goto invalid;
		++f;

		INT16 x;
		if ('1' <= f[0] && f[0] <= '9' && (f[1] < '0' || '9' < f[1]))
		{ // 1 ... 9
			x = f[0] - '0';
			++f;
		}
		else if (f[0] == '1' && '0' <= f[1] && f[1] <= '6')
		{ // 10 ... 16
			x = (f[0] - '0') * 10 + f[1] - '0';
			f += 2;
		}
		else goto invalid;

		INT8 z = 0;
		if (f[0] == '_' && f[1] == 'b' && '1' <= f[2] && f[2] <= '3')
		{
			z = f[2] - '0';
		}

		gWorldSectorX  = x;
		gWorldSectorY  = y;
		gbWorldSectorZ = z;
		return;
	}
invalid:
	SetWorldSectorInvalid();
}


static void InitErrorCatchDialog(void)
{
	DoMessageBox(MSG_BOX_BASIC_STYLE, gzErrorCatchString, EDIT_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
	gfErrorCatch = FALSE;
}


//Because loading and saving the map takes a few seconds, we want to post a message
//on the screen and then update it which requires passing the screen back to the main loop.
//When we come back for the next frame, we then actually save or load the map.  So this
//process takes two full screen cycles.
static ScreenID ProcessFileIO(void)
{
	INT16 usStartX, usStartY;
	ST::string ubNewFilename;
	ST::string zOrigName;
	switch( gbCurrentFileIOStatus )
	{
		case INITIATE_MAP_SAVE:	//draw save message
			SaveFontSettings();
			SetFontAttributes(HUGEFONT, FONT_LTKHAKI, FONT_DKKHAKI);
			zOrigName = ST::format("Saving map:  {}", gzFilename);
			usStartX = (SCREEN_WIDTH - StringPixLength(zOrigName, HUGEFONT)) / 2;
			usStartY = 180 - GetFontHeight(HUGEFONT) / 2;
			MPrint(usStartX, usStartY, zOrigName);

			InvalidateScreen( );
			gbCurrentFileIOStatus = SAVING_MAP;
			return LOADSAVE_SCREEN;
		case SAVING_MAP: //save map
			ubNewFilename = ST::format("{}", gzFilename);
			RaiseWorldLand();
			if( gfShowPits )
				RemoveAllPits();
			OptimizeSchedules();
			if ( !SaveWorld( ubNewFilename.c_str() ) )
			{
				if( gfErrorCatch )
				{
					InitErrorCatchDialog();
					return EDIT_SCREEN;
				}
				return ERROR_SCREEN;
			}
			if( gfShowPits )
				AddAllPits();

			SetGlobalSectorValues();

			if( gfGlobalSummaryExists )
				UpdateSectorSummary( gzFilename, gfUpdateSummaryInfo );

			iCurrentAction = ACTION_NULL;
			gbCurrentFileIOStatus = IOSTATUS_NONE;
			gfRenderWorld = TRUE;
			gfRenderTaskbar = TRUE;
			fEnteringLoadSaveScreen = TRUE;
			RestoreFontSettings();
			if( gfErrorCatch )
			{
				InitErrorCatchDialog();
				return EDIT_SCREEN;
			}
			if( gMapInformation.ubMapVersion != gubMinorMapVersion )
				SLOGE("Map data has just been corrupted!!! What did you just do? KM : 0");
			return EDIT_SCREEN;
		case INITIATE_MAP_LOAD: //draw load message
			SaveFontSettings();
			gbCurrentFileIOStatus = LOADING_MAP;
			if( gfEditMode && iCurrentTaskbar == TASK_MERCS )
				IndicateSelectedMerc( SELECT_NO_MERC );
			SpecifyItemToEdit( NULL, -1 );
			return LOADSAVE_SCREEN;
		case LOADING_MAP: //load map
			DisableUndo();
			ubNewFilename = ST::format("{}", gzFilename);

			RemoveMercsInSector( );

			try
			{
				UINT32 const start = SDL_GetTicks();
				LoadWorld(ubNewFilename.c_str());
				fprintf(stderr, "---> %u\n", SDL_GetTicks() - start);
			}
			catch (...)
			{ //Want to override crash, so user can do something else.
				EnableUndo();
				SetPendingNewScreen( LOADSAVE_SCREEN );
				gbCurrentFileIOStatus = IOSTATUS_NONE;
				gfGlobalError = FALSE;
				gfLoadError = TRUE;
				//RemoveButton( iTempButton );
				CreateMessageBox( " Error loading file.  Try another filename?" );
				return LOADSAVE_SCREEN;
			}
			SetGlobalSectorValues();

			RestoreFontSettings();

			//Load successful, update necessary information.

			AddSoldierInitListTeamToWorld(ENEMY_TEAM);
			AddSoldierInitListTeamToWorld(CREATURE_TEAM);
			AddSoldierInitListTeamToWorld(MILITIA_TEAM);
			AddSoldierInitListTeamToWorld(CIV_TEAM);
			iCurrentAction = ACTION_NULL;
			gbCurrentFileIOStatus = IOSTATUS_NONE;
			if( !gfCaves && !gfBasement )
			{
				gusLightLevel = 12;
				if( ubAmbientLightLevel != 4 )
				{
					ubAmbientLightLevel = 4;
					LightSetBaseLevel( ubAmbientLightLevel );
				}
			}
			else
				gusLightLevel = (UINT16)(EDITOR_LIGHT_MAX - ubAmbientLightLevel );
			gEditorLightColor = g_light_color;
			gfRenderWorld = TRUE;
			gfRenderTaskbar = TRUE;
			fEnteringLoadSaveScreen = TRUE;
			InitJA2SelectionWindow();
			ShowEntryPoints();
			EnableUndo();
			RemoveAllFromUndoList();
			SetEditorSmoothingMode( gMapInformation.ubEditorSmoothingType );
			if( gMapInformation.ubEditorSmoothingType == SMOOTHING_CAVES )
				AnalyseCaveMapForStructureInfo();

			AddLockedDoorCursors();
			gubCurrRoomNumber = gubMaxRoomNumber;
			UpdateRoofsView();
			UpdateWallsView();
			ShowLightPositionHandles();
			SetMercTeamVisibility( ENEMY_TEAM, gfShowEnemies );
			SetMercTeamVisibility( CREATURE_TEAM, gfShowCreatures );
			SetMercTeamVisibility( MILITIA_TEAM, gfShowRebels );
			SetMercTeamVisibility( CIV_TEAM, gfShowCivilians );
			BuildItemPoolList();
			if( gfShowPits )
				AddAllPits();

			if( iCurrentTaskbar == TASK_MAPINFO )
			{ //We have to temporarily remove the current textinput mode,
				//update the disabled text field values, then restore the current
				//text input fields.
				SaveAndRemoveCurrentTextInputMode();
				UpdateMapInfoFields();
				RestoreSavedTextInputMode();
			}
			return EDIT_SCREEN;
	}
	gbCurrentFileIOStatus = IOSTATUS_NONE;
	return LOADSAVE_SCREEN;
}


//LOADSCREEN
static void FDlgNamesCallback(GUI_BUTTON* butn, INT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_LBUTTON_UP) )
	{
		SelectFileDialogYPos(butn->RelativeY());
	}
}


static void FDlgOkCallback(GUI_BUTTON* butn, INT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_LBUTTON_UP) )
	{
		iFDlgState = iCurrentAction == ACTION_SAVE_MAP ? DIALOG_SAVE : DIALOG_LOAD;
	}
}


static void FDlgCancelCallback(GUI_BUTTON* butn, INT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_LBUTTON_UP) )
	{
		iFDlgState = DIALOG_CANCEL;
	}
}


static void FDlgUpCallback(GUI_BUTTON* butn, INT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_LBUTTON_UP) )
	{
		if(iTopFileShown > 0)
			iTopFileShown--;
	}
}


static void FDlgDwnCallback(GUI_BUTTON* butn, INT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_LBUTTON_UP) )
	{
		if( (iTopFileShown+7) < iTotalFiles )
			iTopFileShown++;
	}
}


static BOOLEAN ExtractFilenameFromFields(void)
{
	gzFilename = GetStringFromField(0);
	return ValidFilename();
}


static BOOLEAN ValidFilename(void)
{
	if (!gzFilename.empty())
	{
		auto pos = gzFilename.find(".dat");
		if (pos > 0 && gzFilename[pos + 4] == '\0' )
			return TRUE;
	}
	return FALSE;
}

BOOLEAN ExternalLoadMap(const ST::string& szFilename)
{
	if (szFilename.empty())
		return FALSE;
	gzFilename = szFilename;
	if( !ValidFilename() )
		return FALSE;
	gbCurrentFileIOStatus = INITIATE_MAP_LOAD;
	ProcessFileIO(); //always returns loadsave_screen and changes iostatus to loading_map.
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();
	RefreshScreen();
	if( ProcessFileIO() == EDIT_SCREEN )
		return TRUE;
	return FALSE;
}

BOOLEAN ExternalSaveMap(const ST::string& szFilename)
{
	if (szFilename.empty())
		return FALSE;
	gzFilename = szFilename;
	if( !ValidFilename() )
		return FALSE;
	gbCurrentFileIOStatus = INITIATE_MAP_SAVE;
	if( ProcessFileIO() == ERROR_SCREEN )
		return FALSE;
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();
	RefreshScreen();
	if( ProcessFileIO() == EDIT_SCREEN )
		return TRUE;
	return FALSE;
}
