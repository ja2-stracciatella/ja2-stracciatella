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
#include "MessageBoxScreen.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "WorldDef.h"
#include "UILayout.h"
#include "GameMode.h"
#include "GameRes.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/format>
#include <string_theory/string>

#include <cstdarg>

ST::string gFileForIO;

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

enum{
	FILEDIALOG_BACKGROUND,
	FILEDIALOG_OK,
	FILEDIALOG_CANCEL,
	FILEDIALOG_SCROLL_UP,
	FILEDIALOG_SCROLL_DOWN,
	FILEDIALOG_LIST_BACKGROUND,
	FILEDIALOG_TITLE,
	FILEDIALOG_UPDATE_WORLD_INFO_CHECKBOX,
	NUM_FILEDIALOG_BUTTONS
};
static GUIButtonRef iFileDlgButtons[NUM_FILEDIALOG_BUTTONS];

static INT32 iTopFileShown;
static INT32 iCurrFileShown;
static INT32 iLastFileClicked;
static UINT32 uiLastClickTime;

static std::vector<FileDialogEntry> gFileList;

static INT32 iFDlgState = DIALOG_NONE;

static BOOLEAN gfLoadError;
static bool gfReadOnly;
static BOOLEAN gfFileExists;
static BOOLEAN gfIllegalName;
static BOOLEAN gfDeleteFile;

static BOOLEAN fEnteringLoadSaveScreen = TRUE;

static MOUSE_REGION BlanketRegion;

static ST::string gCurrentDirectory;
static ST::string gCurrentFilename;

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

static void ChangeDirectory(const ST::string directory, bool resetState) {
	gCurrentDirectory = directory;

	gfReadOnly = FALSE;
	gfFileExists = FALSE;
	gfLoadError = FALSE;
	gfIllegalName = FALSE;
	gfDeleteFile = FALSE;

	gFileList.clear();

	if (resetState) {
		iTopFileShown = uiLastClickTime = 0;
		iCurrFileShown = iLastFileClicked = -1;
	}

	ST::string filename = FileMan::getFileName(gFileForIO);
	gCurrentFilename = filename;

	SetInputFieldString(0, gCurrentFilename);
	SetInputFieldString(2, gCurrentDirectory);

	try
	{
		if (FileMan::getParentPath(gCurrentDirectory, false) != "") {
			gFileList.push_back(FileDialogEntry { FileType::Parent, ".." });
		}
		std::vector<ST::string> dirs = FileMan::findAllDirsInDir(directory, true, false, true);
		for (const ST::string &dir : dirs)
		{
			gFileList.push_back(FileDialogEntry { FileType::Directory, dir });
		}
		std::vector<ST::string> files = FileMan::findAllFilesInDir(directory, true, false, true);
		for (const ST::string &file : files)
		{
			gFileList.push_back(FileDialogEntry { FileType::File, file });
		}
	}
	catch (...) { /* XXX ignore */ }
}

static void LoadSaveScreenEntry(void)
{
	fEnteringLoadSaveScreen = FALSE;
	gbCurrentFileIOStatus	= IOSTATUS_NONE;

	CreateFileDialog(iCurrentAction == ACTION_SAVE_MAP ? "Save Map (*.dat)" : "Load Map (*.dat)");

	if (gCurrentDirectory == "") {
		RustPointer<char> currentDirectory(EngineOptions_getStracciatellaHome());
		ChangeDirectory(currentDirectory.get(), true);
	} else {
		ChangeDirectory(gCurrentDirectory, true);
	}

	if( gFileList.empty() )
	{
		if( iCurrentAction == ACTION_LOAD_MAP )
			DisableButton( iFileDlgButtons[FILEDIALOG_OK] );
	}

	iLastFileClicked = -1;
	uiLastClickTime = 0;
}


static void RemoveFileDialog(void);
static BOOLEAN ValidMapName(const ST::string &path);


static ScreenID ProcessLoadSaveScreenMessageBoxResult(void)
{
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
			auto curr = gFileList.begin();
			for( x = 0; x < iCurrFileShown && curr != gFileList.end(); x++ )
			{
				curr++;
			}
			if( curr != gFileList.end() )
			{
				try {
					FileMan::deleteFile(gFileForIO);
					ChangeDirectory(gCurrentDirectory, false);
					if( gFileList.empty() )
					{
						if( iCurrentAction == ACTION_LOAD_MAP )
							DisableButton( iFileDlgButtons[FILEDIALOG_OK] );
					}
					if( iCurrFileShown >= (INT32)gFileList.size() )
						iCurrFileShown--;
					if( iCurrFileShown < iTopFileShown )
						iTopFileShown -= 8;
					if( iTopFileShown < 0 )
						iTopFileShown = 0;
				} catch (const std::runtime_error& err) {
					SLOGW("Could not remove file: {}", err.what());
				}
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
static ST::string ExtractFilenameFromFields(void);
static void HandleMainKeyEvents(InputAtom* pEvent);
static ScreenID ProcessFileIO(void);


ScreenID LoadSaveScreenHandle(void)
{
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
	while( DequeueSpecificEvent(&DialogEvent, KEYBOARD_EVENTS) )
	{
		if( !HandleTextInput(&DialogEvent) && (DialogEvent.usEvent == KEY_DOWN || DialogEvent.usEvent == KEY_REPEAT) )
		{
			HandleMainKeyEvents( &DialogEvent );
		}
	}

	DrawFileDialog();

	// Skip to first filename to show
	auto curr = gFileList.begin();
	for(x=0;x<iTopFileShown && curr != gFileList.end();x++)
	{
		curr++;
	}

	// Show up to 8 filenames in the window
	SetFont( FONT12ARIAL );
	if( gFileList.empty() )
	{
		SetFontForeground( FONT_LTRED );
		SetFontBackground( 142 );
		MPrint(226 + 24, 126, "NO FILES IN CURRENT DIRECTORY");
	}
	else for(x=iTopFileShown;x<(iTopFileShown+8) && curr != gFileList.end(); x++)
	{
		if( !EditingText() && x == iCurrFileShown  )
		{
			if (curr->type == FileType::File) {
				SetFontAttributes(FONT12ARIAL, FONT_GRAY2, 2, FONT_METALGRAY);
			} else {
				SetFontAttributes(FONT12ARIAL, FONT_DKYELLOW, 2, FONT_METALGRAY);
			}
		}
		else
		{
			if (curr->type == FileType::File) {
				SetFontAttributes(FONT12ARIAL, FONT_BLACK, 0, 142);
			} else {
				SetFontAttributes(FONT12ARIAL, FONT_DKYELLOW, 0, 142);
			}
		}
		auto label = ST::format("{.35}", curr->filename);
		if (curr->filename.size() > 35) {
			label += "...";
		}
		MPrint(186, 73 + 24 + (x - iTopFileShown) * 15, label);
		curr++;
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
			auto filename = ExtractFilenameFromFields();
			auto absolutePath = FileMan::joinPaths(gCurrentDirectory, filename);
			bool readonly = false;
			try {
				readonly = FileMan::isReadOnly(absolutePath);
			} catch (const std::runtime_error& ex) {
				SLOGE("Error determining readonly status for file {}: {}", absolutePath, ex.what());
			}
			ST::string str;
			if (readonly)
			{
				str = ST::format(" Delete READ-ONLY file {}? ", filename);
			}
			else
				str = ST::format(" Delete file {}? ", filename);
			gfDeleteFile = TRUE;
			gFileForIO = absolutePath;
			CreateMessageBox( str );
			return LOADSAVE_SCREEN;
		}

		case DIALOG_SAVE:
		{

			auto filename = ExtractFilenameFromFields();
			auto absolutePath = FileMan::joinPaths(gCurrentDirectory, filename);
			if (filename == "..") {
				absolutePath = FileMan::getParentPath(gCurrentDirectory, false);
			}
			if (FileMan::isDir(absolutePath)) {
				ChangeDirectory(absolutePath, true);
				iFDlgState = DIALOG_NONE;
				return LOADSAVE_SCREEN;
			}
			if( !ValidMapName(absolutePath) )
			{
				CreateMessageBox( " Illegal filename.  Try another filename? " );
				gfIllegalName = TRUE;
				iFDlgState = DIALOG_NONE;
				return LOADSAVE_SCREEN;
			}
			gFileForIO = absolutePath;
			if ( FileMan::exists(absolutePath) )
			{
				gfFileExists = TRUE;
				gfReadOnly = false;
				try {
					gfReadOnly = FileMan::isReadOnly(absolutePath);
				} catch (const std::runtime_error& ex) {
					SLOGE("Error determining readonly status for file {}: {}", absolutePath, ex.what());
				}
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
		{
			auto filename = ExtractFilenameFromFields();
			auto absolutePath = FileMan::joinPaths(gCurrentDirectory, filename);
			if (filename == "..") {
				absolutePath = FileMan::getParentPath(gCurrentDirectory, false);
			}
			if (FileMan::isDir(absolutePath)) {
				ChangeDirectory(absolutePath, true);
				iFDlgState = DIALOG_NONE;
				return LOADSAVE_SCREEN;
			}
			if( !ValidMapName(absolutePath) )
			{
				CreateMessageBox( " Illegal filename.  Try another filename? " );
				gfIllegalName = TRUE;
				iFDlgState = DIALOG_NONE;
				return LOADSAVE_SCREEN;
			}
			RemoveFileDialog();
			CreateProgressBar(0, 118, 183, 404, 19);
			DefineProgressBarPanel( 0, 65, 79, 94, 100, 155, 540, 235 );
			zOrigName = ST::format("Loading map:  {}", filename);
			SetProgressBarTitle( 0, zOrigName, BLOCKFONT2, FONT_RED, FONT_NEARBLACK );
			gFileForIO = absolutePath;
			gbCurrentFileIOStatus = INITIATE_MAP_LOAD;
			return LOADSAVE_SCREEN;
		}
		default:
			iFDlgState = DIALOG_NONE;
	}
	iFDlgState = DIALOG_NONE;
	return LOADSAVE_SCREEN ;
}


static GUIButtonRef MakeButtonArrow(const char* const gfx, const INT16 y, const INT16 priority, const GUI_CALLBACK click)
{
	GUIButtonRef const btn = QuickCreateButtonImg(gfx, -1, 1, 2, 3, 4, 426, y, priority, click);
	btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);
	return btn;
}


static void FDlgCancelCallback(GUI_BUTTON* butn, UINT32 reason);
static void FDlgDwnCallback(GUI_BUTTON* butn, UINT32 reason);
static void FDlgNamesCallback(GUI_BUTTON* butn, UINT32 reason);
static void FDlgOkCallback(GUI_BUTTON* butn, UINT32 reason);
static void FDlgUpCallback(GUI_BUTTON* butn, UINT32 reason);
static void FileDialogModeCallback(UINT8 ubID, BOOLEAN fEntering);
static void UpdateWorldInfoCallback(GUI_BUTTON* b, UINT32 reason);


static void CreateFileDialog(const ST::string& zTitle)
{
	const INT16 basePriority = MSYS_PRIORITY_HIGH + 5;

	iFDlgState = DIALOG_NONE;

	DisableEditorTaskbar();

	MSYS_DefineRegion( &BlanketRegion, 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_END_Y, basePriority, 0, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

	//Title button
	iFileDlgButtons[FILEDIALOG_TITLE] = CreateLabel(zTitle, FONT16ARIAL, FONT_LTKHAKI, FONT_DKKHAKI, 179, 44, 281, 30, basePriority+1);

	//Background
	iFileDlgButtons[FILEDIALOG_BACKGROUND] = CreateLabel({}, FONT16ARIAL, 0, 0, 179, 69, 281, 216, basePriority+1);

	//File list window
	iFileDlgButtons[FILEDIALOG_LIST_BACKGROUND] = CreateHotSpot(179 + 4, 69 + 3 + 24, 179 + 4 + 240, 69 + 120 + 3, basePriority+2, FDlgNamesCallback);

	//Okay and cancel buttons
	iFileDlgButtons[FILEDIALOG_OK] = CreateTextButton("Okay",   FONT12POINT1, FONT_BLACK, FONT_BLACK, 354, 225 + 18, 50, 30, basePriority+3, FDlgOkCallback);
	iFileDlgButtons[FILEDIALOG_CANCEL] = CreateTextButton("Cancel", FONT12POINT1, FONT_BLACK, FONT_BLACK, 406, 225 + 18, 50, 30, basePriority+3, FDlgCancelCallback);

	//Scroll buttons
	iFileDlgButtons[FILEDIALOG_SCROLL_UP] = MakeButtonArrow(EDITORDIR "/uparrow.sti",    92 + 5, basePriority+3, FDlgUpCallback);
	iFileDlgButtons[FILEDIALOG_SCROLL_DOWN] = MakeButtonArrow(EDITORDIR "/downarrow.sti", 182 + 4, basePriority+3, FDlgDwnCallback);

	if( iCurrentAction == ACTION_SAVE_MAP )
	{	//checkboxes
		//The update world info checkbox
		iFileDlgButtons[FILEDIALOG_UPDATE_WORLD_INFO_CHECKBOX] = CreateCheckBoxButton( 183, 229 + 14, EDITORDIR "/smcheckbox.sti", basePriority+3, UpdateWorldInfoCallback );
		if( gfUpdateSummaryInfo )
			iFileDlgButtons[FILEDIALOG_UPDATE_WORLD_INFO_CHECKBOX]->uiFlags |= BUTTON_CLICKED_ON;
	}

	//Add the text input fields
	InitTextInputModeWithScheme( DEFAULT_SCHEME );
	//field 1 (filename)
	AddTextInputField(/*233*/183 + 46, 195 + 24, 273 - 46, 20, basePriority+3, gCurrentFilename, 30, INPUTTYPE_FULL_TEXT);
	//field 2 -- user field that allows mouse/key interaction with the filename list
	AddUserInputField( FileDialogModeCallback );
	//field 3 -- directory input
	AddTextInputField(183 + 46, 74, 273 - 46, 20, basePriority+3, gCurrentDirectory, 40, INPUTTYPE_FULL_TEXT);
}


static void UpdateWorldInfoCallback(GUI_BUTTON* b, UINT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_POINTER_UP )
		gfUpdateSummaryInfo = b->Clicked();
}


//This is a hook into the text input code.  This callback is called whenever the user is currently
//editing text, and presses Tab to transfer to the file dialog mode.  When this happens, we set the text
//field to the currently selected file in the list which is already know.
static void FileDialogModeCallback(UINT8 ubID, BOOLEAN fEntering)
{
	INT32 x;
	if( fEntering )
	{
		// Skip to first filename
		auto curr = gFileList.begin();
		for(x=0;x<iTopFileShown && curr != gFileList.end();x++)
		{
			curr++;
		}
		// Find the already selected filename
		for(x = iTopFileShown; x < iTopFileShown + 8 && curr != gFileList.end(); x++ )
		{
			if( iCurrFileShown == (x-iTopFileShown) )
			{
				SetInputFieldString(0, curr->filename);
				return;
			}
			curr++;
		}
	}
}


static void RemoveFileDialog(void)
{
	INT32 x;

	MSYS_RemoveRegion( &BlanketRegion );

	for(x=0; x<NUM_FILEDIALOG_BUTTONS; x++)
	{
		if (iFileDlgButtons[x]) {
			RemoveButton(iFileDlgButtons[x]);
		}
	}

	gFileList.clear();

	InvalidateScreen( );

	EnableEditorTaskbar();
	KillTextInputMode();
	MarkWorldDirty();
	RenderWorld();
}


static void DrawFileDialog(void)
{
	MarkButtonsDirty();

	iFileDlgButtons[FILEDIALOG_BACKGROUND]->Draw();

	ColorFillVideoSurfaceArea(FRAME_BUFFER, (179+4), (69+3+24), (179+4+240), (69+123+24), Get16BPPColor(FROMRGB(24, 61, 81)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER, (179+5), (69+4+24), (179+4+240), (69+123+24), Get16BPPColor(FROMRGB(136, 138, 135)) );
	ColorFillVideoSurfaceArea(FRAME_BUFFER, (179+5), (69+4+24), (179+3+240), (69+122+24), Get16BPPColor(FROMRGB(250, 240, 188)) );

	for (auto i = 1; i < NUM_FILEDIALOG_BUTTONS; i++) {
		if (iFileDlgButtons[i]) {
			iFileDlgButtons[i]->Draw();
		}
	}
	RenderFastHelp();

	SetFontAttributes(FONT10ARIAL, FONT_LTKHAKI, FONT_DKKHAKI);
	MPrint(185, 80, "Directory");
	MPrint(185, 224, "Filename");

	if (iFileDlgButtons[FILEDIALOG_UPDATE_WORLD_INFO_CHECKBOX]) MPrint(200, 231 + 14, "Update world info");
}


//The callback calls this function passing the relative y position of where
//the user clicked on the hot spot.
static void SelectFileDialogYPos(UINT16 usRelativeYPos)
{
	INT16 sSelName;
	INT32 x;

	sSelName = usRelativeYPos / 15;

	//This is a field in the text editmode, but clicked via mouse.
	SetActiveField( 1 );

	// Skip to first filename
	auto curr = gFileList.begin();
	for(x=0;x<iTopFileShown && curr != gFileList.end();x++)
	{
		curr++;
	}

	for(x=iTopFileShown;x<(iTopFileShown+8) && curr != gFileList.end(); x++)
	{
		if( (INT32)sSelName == (x-iTopFileShown) )
		{
			UINT32 uiCurrClickTime;
			iCurrFileShown = x;
			gCurrentFilename = ST::format("{}", curr->filename);
			SetInputFieldString(0, gCurrentFilename);

			RenderInactiveTextField( 0 );

			//Calculate and process any double clicking...
			uiCurrClickTime = GetJA2Clock();
			if (uiCurrClickTime - uiLastClickTime < 400 && x == iLastFileClicked)
			{ //Considered a double click, so activate load/save this filename.
				iFDlgState = iCurrentAction == ACTION_SAVE_MAP ? DIALOG_SAVE : DIALOG_LOAD;
			}
			uiLastClickTime = uiCurrClickTime;
			iLastFileClicked = x;
		}
		curr++;
	}
}

static void SetTopFileToLetter(UINT16 usLetter)
{
	UINT32 x;
	std::vector<FileDialogEntry>::iterator curr;
	std::vector<FileDialogEntry>::iterator prev;
	UINT16 usNodeLetter;

	// Skip to first filename
	x = 0;
	curr = prev = gFileList.begin();
	while( curr != gFileList.end() )
	{
		usNodeLetter = curr->filename[0]; //first letter of filename.
		if( usNodeLetter < 'a' )
			usNodeLetter += 32; //convert uppercase to lower case A=65, a=97
		if( usLetter <= usNodeLetter )
			break;
		prev = curr;
		curr++;
		x++;
	}
	if( !gFileList.empty() )
	{
		iCurrFileShown = x;
		iTopFileShown = x;
		if( iTopFileShown > (INT32)gFileList.size() - 7 )
			iTopFileShown = (INT32)gFileList.size() - 7;
		SetInputFieldString(0, prev->filename);
	}
}


static void HandleMainKeyEvents(InputAtom* pEvent)
{
	INT32 iFileListSize = gFileList.size();
	INT32 iPrevFileShown = iCurrFileShown;
	//Replace Alt-x press with ESC.
	if( pEvent->usKeyState & ALT_DOWN && pEvent->usParam == 'x' )
		pEvent->usParam = SDLK_ESCAPE;
	switch( pEvent->usParam )
	{
		case SDLK_RETURN:
			if( gFileList.empty() && iCurrentAction == ACTION_LOAD_MAP )
				break;
			if (GetActiveFieldID() == 2) {
				auto directory = GetStringFromField(2);
				ChangeDirectory(directory, true);
			} else {
				iFDlgState = iCurrentAction == ACTION_SAVE_MAP ? DIALOG_SAVE : DIALOG_LOAD;
			}
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
			if( iTopFileShown > iFileListSize-7 )
				iTopFileShown = iFileListSize - 7;
			if( iCurrFileShown >= iFileListSize )
				iCurrFileShown = iFileListSize - 1;
			break;

		case SDLK_UP:
			if( iCurrFileShown > 0 )
				iCurrFileShown--;
			if( iTopFileShown > iCurrFileShown )
				iTopFileShown = iCurrFileShown;
			break;

		case SDLK_DOWN:
			iCurrFileShown++;
			if( iCurrFileShown >= iFileListSize )
				iCurrFileShown = iFileListSize - 1;
			else if( iTopFileShown < iCurrFileShown-7 )
				iTopFileShown++;
			break;

		case SDLK_HOME:
			iTopFileShown = 0;
			iCurrFileShown = 0;
			break;

		case SDLK_END:
			iTopFileShown = iFileListSize-7;
			iCurrFileShown = iFileListSize-1;
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
		x = 0;
		auto curr = gFileList.begin();
		while( curr != gFileList.end() && x != iCurrFileShown )
		{
			curr++;
			x++;
		}
		if( curr != gFileList.end() )
		{
			gCurrentFilename = ST::format("{}", curr->filename);
			SetInputFieldString(0, gCurrentFilename);
		}
	}
}


// Editor doesn't care about the z value. It uses its own methods.
static void SetGlobalSectorValues(const ST::string& filename)
{
	{ const char* f = filename.c_str();

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

		gWorldSector = SGPSector(x, y, z);
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
	ST::string label;
	ST::string ioFilename(FileMan::getFileName(gFileForIO));
	switch( gbCurrentFileIOStatus )
	{
		case INITIATE_MAP_SAVE:	//draw save message
			SaveFontSettings();
			SetFontAttributes(FONT16ARIAL, FONT_LTKHAKI, FONT_DKKHAKI);
			label = ST::format("Saving map:  {}", ioFilename);
			usStartX = (SCREEN_WIDTH - StringPixLength(label, FONT16ARIAL)) / 2;
			usStartY = 180 - GetFontHeight(FONT16ARIAL) / 2;
			MPrint(usStartX, usStartY, label);

			InvalidateScreen( );
			gbCurrentFileIOStatus = SAVING_MAP;
			return LOADSAVE_SCREEN;
		case SAVING_MAP: //save map
			RaiseWorldLand();
			if( gfShowPits )
				RemoveAllPits();
			OptimizeSchedules();
			if (!SaveWorldAbsolute(gFileForIO))
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

			SetGlobalSectorValues(ioFilename);

			if( gfGlobalSummaryExists )
				UpdateSectorSummary( ioFilename, gfUpdateSummaryInfo );

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

			RemoveMercsInSector( );

			try
			{
				UINT32 const start = SDL_GetTicks();
				if (!FileMan::isAbsolute(gFileForIO)) {
					LoadWorld(gFileForIO);
				} else {
					LoadWorldAbsolute(gFileForIO);
				}

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
			SetGlobalSectorValues(ioFilename);

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
static void FDlgNamesCallback(GUI_BUTTON* butn, UINT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_POINTER_UP) )
	{
		SelectFileDialogYPos(butn->RelativeY());
	}
	if( reason & (MSYS_CALLBACK_REASON_WHEEL_UP) ) {
		if(iTopFileShown > 0)
			iTopFileShown--;
	}
	if( reason & (MSYS_CALLBACK_REASON_WHEEL_DOWN) ) {
		if( (iTopFileShown+7) < (INT32)gFileList.size() )
			iTopFileShown++;
	}
}


static void FDlgOkCallback(GUI_BUTTON* butn, UINT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_POINTER_UP) )
	{
		iFDlgState = iCurrentAction == ACTION_SAVE_MAP ? DIALOG_SAVE : DIALOG_LOAD;
	}
}


static void FDlgCancelCallback(GUI_BUTTON* butn, UINT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_POINTER_UP) )
	{
		iFDlgState = DIALOG_CANCEL;
	}
}


static void FDlgUpCallback(GUI_BUTTON* butn, UINT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_POINTER_UP) )
	{
		if(iTopFileShown > 0)
			iTopFileShown--;
	}
}


static void FDlgDwnCallback(GUI_BUTTON* butn, UINT32 reason)
{
	if( reason & (MSYS_CALLBACK_REASON_POINTER_UP) )
	{
		if( (iTopFileShown+7) < (INT32)gFileList.size() )
			iTopFileShown++;
	}
}


static ST::string ExtractFilenameFromFields(void)
{
	return GetStringFromField(0);
}


static BOOLEAN ValidMapName(const ST::string &path)
{
	return path.ends_with(".dat", ST::case_sensitivity_t::case_insensitive);
}

BOOLEAN ExternalLoadMap(const ST::string& szFilename)
{
	if (szFilename.empty())
		return FALSE;
	if( !ValidMapName(szFilename) )
		return FALSE;
	gFileForIO = szFilename;
	gbCurrentFileIOStatus = INITIATE_MAP_LOAD;
	ProcessFileIO(); //always returns loadsave_screen and changes iostatus to loading_map.
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();
	RefreshScreen();
	if( ProcessFileIO() == EDIT_SCREEN )
		return TRUE;
	return FALSE;
}
