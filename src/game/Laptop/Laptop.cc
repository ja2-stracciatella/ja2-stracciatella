#include "Directories.h"
#include "Font.h"
#include "Font_Control.h"
#include "Input.h"
#include "Interface.h"
#include "LoadSaveData.h"
#include "Local.h"
#include "Map_Screen_Interface.h"
#include "Object_Cache.h"
#include "Timer.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "VSurface.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "Cursors.h"
#include "Event_Pump.h"
#include "Laptop.h"
#include "AIM.h"
#include "AIMMembers.h"
#include "AIMFacialIndex.h"
#include "AIMSort.h"
#include "AIMArchives.h"
#include "AIMHistory.h"
#include "AIMLinks.h"
#include "AIMPolicies.h"
#include "Mercs.h"
#include "Mercs_Files.h"
#include "Mercs_Account.h"
#include "Mercs_No_Account.h"
#include "BobbyR.h"
#include "BobbyRGuns.h"
#include "BobbyRAmmo.h"
#include "BobbyRArmour.h"
#include "BobbyRMisc.h"
#include "BobbyRUsed.h"
#include "BobbyRMailOrder.h"
#include "CharProfile.h"
#include "ContentManager.h"
#include "Florist.h"
#include "Florist_Cards.h"
#include "Florist_Gallery.h"
#include "Florist_Order_Form.h"
#include "GameInstance.h"
#include "Insurance.h"
#include "Insurance_Contract.h"
#include "Insurance_Info.h"
#include "Insurance_Comments.h"
#include "Funeral.h"
#include "Finances.h"
#include "Personnel.h"
#include "History.h"
#include "Files.h"
#include "EMail.h"
#include "Interface_Control.h"
#include "Game_Event_Hook.h"
#include "WordWrap.h"
#include "Game_Init.h"
#include "Game_Clock.h"
#include "Overhead.h"
#include "Environment.h"
#include "Music_Control.h"
#include "ContentMusic.h"
#include "LaptopSave.h"
#include "RenderWorld.h"
#include "GameLoop.h"
#include "Ambient_Control.h"
#include "Sound_Control.h"
#include "Text.h"
#include "Message.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Cursor_Control.h"
#include "GameRes.h"
#include "BrokenLink.h"
#include "BobbyRShipments.h"
#include "Dialogue_Control.h"
#include "HelpScreen.h"
#include "Cheats.h"
#include "Video.h"
#include "Debug.h"
#include "Button_System.h"
#include "UILayout.h"

#include "policy/GamePolicy.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>
#include <iterator>

// laptop programs
enum
{
	LAPTOP_PROGRAM_MAILER,
	LAPTOP_PROGRAM_WEB_BROWSER,
	LAPTOP_PROGRAM_FILES,
	LAPTOP_PROGRAM_PERSONNEL,
	LAPTOP_PROGRAM_FINANCES,
	LAPTOP_PROGRAM_HISTORY
};

// laptop program states
enum
{
	LAPTOP_PROGRAM_MINIMIZED,
	LAPTOP_PROGRAM_OPEN
};

#define BOOK_FONT     FONT10ARIAL
#define DOWNLOAD_FONT FONT12ARIAL


#define BOOK_X      (111 + STD_SCREEN_X)
#define BOOK_TOP_Y  ( 79 + STD_SCREEN_Y)
#define BOOK_HEIGHT  12
#define DOWN_HEIGHT  19
#define BOOK_WIDTH  100


#define LONG_UNIT_TIME        120
#define UNIT_TIME              40
#define FAST_UNIT_TIME          3
#define FASTEST_UNIT_TIME       2
#define ALMOST_FAST_UNIT_TIME  25
#define DOWNLOAD_X            (300 + STD_SCREEN_X)
#define DOWNLOAD_Y            (200 + STD_SCREEN_Y)
#define LAPTOP_WINDOW_X       DOWNLOAD_X + 12
#define LAPTOP_WINDOW_Y       DOWNLOAD_Y + 25
#define LAPTOP_BAR_Y          LAPTOP_WINDOW_Y + 2
#define LAPTOP_BAR_X          LAPTOP_WINDOW_X + 1
#define UNIT_WIDTH              4
#define DOWN_STRING_X         DOWNLOAD_X + 47
#define DOWN_STRING_Y         DOWNLOAD_Y +  5
#define LAPTOP_TITLE_ICONS_X  (113 + STD_SCREEN_X)
#define LAPTOP_TITLE_ICONS_Y  ( 27 + STD_SCREEN_Y)

// HD flicker times
#define HD_FLICKER_TIME 3000
#define FLICKER_TIME      50


#define NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS  18
#define LAPTOP_TITLE_BAR_WIDTH               500
#define LAPTOP_TITLE_BAR_HEIGHT               24

#define LAPTOP_TITLE_BAR_TOP_LEFT_X          (111 + STD_SCREEN_X)
#define LAPTOP_TITLE_BAR_TOP_LEFT_Y          ( 25 + STD_SCREEN_Y)
#define LAPTOP_TITLE_BAR_TOP_RIGHT_X         (610 + STD_SCREEN_X)

#define LAPTOP_TITLE_BAR_ICON_OFFSET_X         (5 + STD_SCREEN_X)
#define LAPTOP_TITLE_BAR_ICON_OFFSET_Y         (2 + STD_SCREEN_Y)
#define LAPTOP_TITLE_BAR_TEXT_OFFSET_X        (29 + STD_SCREEN_X)
#define LAPTOP_TITLE_BAR_TEXT_OFFSET_Y         (8 + STD_SCREEN_Y)

#define LAPTOP_PROGRAM_ICON_X      LAPTOP_TITLE_BAR_TOP_LEFT_X
#define LAPTOP_PROGRAM_ICON_Y      LAPTOP_TITLE_BAR_TOP_LEFT_Y
#define LAPTOP_PROGRAM_ICON_WIDTH  20
#define LAPTOP_PROGRAM_ICON_HEIGHT 20
#define DISPLAY_TIME_FOR_WEB_BOOKMARK_NOTIFY 2000

// the wait time for closing of laptop animation/delay
#define EXIT_LAPTOP_DELAY_TIME 100

static SGPVSurface* guiTitleBarSurface;
static BOOLEAN gfTitleBarSurfaceAlreadyActive = FALSE;

#define LAPTOP__NEW_FILE_ICON_X  (83 + STD_SCREEN_X)
#define LAPTOP__NEW_FILE_ICON_Y (412 + STD_SCREEN_Y)

#define LAPTOP__NEW_EMAIL_ICON_X ((83 - 16) + STD_SCREEN_X)
#define LAPTOP__NEW_EMAIL_ICON_Y LAPTOP__NEW_FILE_ICON_Y


// Mode values
LaptopMode        guiCurrentLaptopMode;
LaptopMode        guiPreviousLaptopMode;
// Used to prevent double free problems. Fixes Stracciatella issue #68:
LaptopMode        guiLastExitedLaptopMode = LAPTOP_MODE_NONE;
static LaptopMode guiCurrentWWWMode = LAPTOP_MODE_NONE;
INT32  giCurrentSubPage;


static INT32 iHighLightBookLine = -1;
BOOLEAN fFastLoadFlag = FALSE;
static BOOLEAN gfEnterLapTop=TRUE;
BOOLEAN gfShowBookmarks=FALSE;

// in progress of loading a page?
BOOLEAN fLoadPendingFlag=FALSE;

// mark buttons dirty?
static BOOLEAN fMarkButtonsDirtyFlag = TRUE;

// redraw afer rendering buttons?
BOOLEAN fReDrawPostButtonRender = FALSE;

// in laptop right now?
BOOLEAN fCurrentlyInLaptop = FALSE;

// exit due to a message box pop up?..don't really leave LAPTOP
BOOLEAN fExitDueToMessageBox = FALSE;

// exit laptop during a load?
static BOOLEAN fExitDuringLoad = FALSE;

// done loading?
BOOLEAN fDoneLoadPending = FALSE;

// going a subpage of a web page?..faster access time
BOOLEAN fConnectingToSubPage = FALSE;

// is this our first time in laptop?
static BOOLEAN fFirstTimeInLaptop = TRUE;

// redraw the book mark info panel .. for blitting on top of animations
BOOLEAN fReDrawBookMarkInfo = FALSE;

// show the 2 second info about bookmarks being accessed by clicking on web
BOOLEAN fShowBookmarkInfo = FALSE;

//GLOBAL FOR WHICH SCREEN TO EXIT TO FOR LAPTOP
static ScreenID guiExitScreen = MAP_SCREEN;

static BOOLEAN fNewWWW = TRUE;

//Used to store the site to go to after the 'rain delay' message
extern UINT32 guiRainLoop;


static INT32 giRainDelayInternetSite = -1;

namespace {
// Laptop screen graphic handle
cache_key_t const guiLAPTOP{ LAPTOPDIR "/laptop3.sti" };

// the laptop icons
cache_key_t const guiDOWNLOADTOP{ LAPTOPDIR "/downloadtop.sti" };
cache_key_t const guiDOWNLOADMID{ LAPTOPDIR "/downloadmid.sti" };
cache_key_t const guiDOWNLOADBOT{ LAPTOPDIR "/downloadbot.sti" };
cache_key_t const guiBOOKMARK{ LAPTOPDIR "/webpages.sti" };
cache_key_t const guiBOOKHIGH{ LAPTOPDIR "/hilite.sti" };
cache_key_t const guiGRAPHWINDOW{ LAPTOPDIR "/graphwindow.sti" };
cache_key_t const guiGRAPHBAR{ LAPTOPDIR "/graphsegment.sti" };
cache_key_t const guiLIGHTS{ LAPTOPDIR "/lights.sti" };
}
SGPVObject* guiLaptopBACKGROUND;
SGPVObject* guiTITLEBARICONS;
static SGPVSurface* guiDESKTOP;

// enter new laptop mode due to sliding bars
static BOOLEAN fEnteredNewLapTopDueToHandleSlidingBars = FALSE;


// whether or not we are initing the slide in title bar
static BOOLEAN fInitTitle = TRUE;

// tab handled
static BOOLEAN fTabHandled = FALSE;

static GUIButtonRef gLaptopButton[7];

// minimize button
static GUIButtonRef gLaptopMinButton;


static INT32 gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY + 1];

// process of mazimizing
static BOOLEAN fMaximizingProgram = FALSE;

// program we are maximizing
static INT8 bProgramBeingMaximized = -1;

// are we minimizing
static BOOLEAN fMinizingProgram = FALSE;


// process openned queue
static INT32 gLaptopProgramQueueList[6];


BOOLEAN fExitingLaptopFlag = FALSE;

// HD and power lights on
static BOOLEAN fPowerLightOn = TRUE;
static BOOLEAN fHardDriveLightOn = FALSE;

// HD flicker
static BOOLEAN fFlickerHD = FALSE;

// the screens limiting rect
static const SGPRect LaptopScreenRect = { LAPTOP_UL_X, (UINT16)(LAPTOP_UL_Y - 5), (UINT16)(LAPTOP_SCREEN_LR_X + 2), (UINT16)(LAPTOP_SCREEN_LR_Y + 5 + 19) };


// the sub pages vistsed or not status within the web browser
static bool gfWWWaitSubSitesVisitedFlags[LAPTOP_MODE_END - (LAPTOP_MODE_WWW + 1)];

// mouse regions
static MOUSE_REGION gLapTopScreenRegion;
static MOUSE_REGION gBookmarkMouseRegions[MAX_BOOKMARKS];
static MOUSE_REGION gLapTopProgramMinIcon;
static MOUSE_REGION gNewMailIconRegion;
static MOUSE_REGION gNewFileIconRegion;


//used for global variables that need to be saved
LaptopSaveInfoStruct LaptopSaveInfo;


BOOLEAN fReDrawScreenFlag=FALSE;
BOOLEAN fPausedReDrawScreenFlag=FALSE; //used in the handler functions to redraw the screen, after the current frame
void PrintBalance(void);


void PrintDate(void);
void PrintNumberOnTeam(void);

extern void ClearHistoryList(void);


void SetLaptopExitScreen(ScreenID const uiExitScreen)
{
	guiExitScreen = uiExitScreen;
}


void SetLaptopNewGameFlag(void)
{
	LaptopSaveInfo.gfNewGameLaptop = TRUE;
}


static void GetLaptopKeyboardInput(void)
{
	fTabHandled = FALSE;

	InputAtom InputEvent;
	while (DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS))
	{
		HandleKeyBoardShortCutsForLapTop(InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState);
	}
}


static void InitBookMarkList(void);


//This is called only once at game initialization.
void LaptopScreenInit(void)
{
	//Memset the whole structure, to make sure of no 'JUNK'
	LaptopSaveInfo = LaptopSaveInfoStruct{};

	LaptopSaveInfo.gfNewGameLaptop = TRUE;

	//reset the id of the last hired merc
	LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;

	//reset the flag that enables the 'just hired merc' popup
	LaptopSaveInfo.sLastHiredMerc.fHaveDisplayedPopUpInLaptop = FALSE;

	//Initialize all vars
	guiCurrentLaptopMode  = LAPTOP_MODE_EMAIL;
	guiPreviousLaptopMode = LAPTOP_MODE_NONE;
	guiCurrentWWWMode     = LAPTOP_MODE_NONE;

	gfShowBookmarks = FALSE;
	InitBookMarkList();
	GameInitAIM();
	GameInitAimSort();
	GameInitMercs();
	GameInitBobbyRGuns();
	GameInitBobbyRMailOrder();
	GameInitEmail();
	GameInitCharProfile();
	GameInitFiles();
	GameInitPersonnel();

	// init program states
	std::fill(std::begin(gLaptopProgramStates), std::end(gLaptopProgramStates), LAPTOP_PROGRAM_MINIMIZED);

	gfAtLeastOneMercWasHired = FALSE;

	//No longer inits the laptop screens, now InitLaptopAndLaptopScreens() does
}


void InitLaptopAndLaptopScreens(void)
{
	GameInitFinances();
	GameInitHistory();

	//Reset the flag so we can create a new IMP character
	LaptopSaveInfo.fIMPCompletedFlag = FALSE;

	//Reset the flag so that BOBBYR's isnt available at the begining of the game
	LaptopSaveInfo.fBobbyRSiteCanBeAccessed = FALSE;
}


//This is only called once at game shutdown.
void LaptopScreenShutdown(void)
{
	InsuranceContractEndGameShutDown();
	BobbyRayMailOrderEndGameShutDown();
	ShutDownEmailList();
	ClearHistoryList();
}


static void CreateDestroyMouseRegionForNewMailIcon(void);
static void CreateLapTopMouseRegions(void);
static void DrawDeskTopBackground(void);
static void EnterLaptopInitLaptopPages(void);
static void InitLaptopOpenQueue(void);
static void InitalizeSubSitesList(void);
static BOOLEAN IsItRaining(void);
static void LoadDesktopBackground(void);
static void RenderLapTopImage(void);


static void EnterLaptop(void)
{
	//Create, load, initialize data -- just entered the laptop.

	// we are re entering due to message box, leave NOW!
	if (fExitDueToMessageBox) return;

	//if the radar map mouse region is still active, disable it.
	if (gRadarRegion.uiFlags & MSYS_REGION_ENABLED)
	{
		gRadarRegion.Disable();
	}

	gfDontStartTransitionFromLaptop = FALSE;

	// ATE: Disable messages....
	DisableScrollMessages();

	// Stop any person from saying anything
	StopAnyCurrentlyTalkingSpeech();

	// Don't play music....
	SetMusicMode(MUSIC_LAPTOP);

	// Stop ambients...
	StopAmbients();

	//if its raining, start the rain showers
	if (IsItRaining())
	{
		//Enable the rain delay warning
		giRainDelayInternetSite = -1;

		//lower the volume
		guiRainLoop = PlayJA2Ambient(RAIN_1, LOWVOLUME, 0);
	}

	//pause the game because we dont want time to advance in the laptop
	PauseGame();

	// set the fact we are currently in laptop, for rendering purposes
	fCurrentlyInLaptop = TRUE;

	// reset redraw flag and redraw new mail
	fReDrawScreenFlag  = FALSE;
	fReDrawNewMailFlag = TRUE;

	// sub page
	giCurrentSubPage = 0;

	// background for panel
	guiLaptopBACKGROUND = AddVideoObjectFromFile(LAPTOPDIR "/taskbar.sti");

	// icons for title bars
	guiTITLEBARICONS = AddVideoObjectFromFile(LAPTOPDIR "/icons.sti");

	// load, blt and delete graphics
	guiEmailWarning = AddVideoObjectFromFile(LAPTOPDIR "/newmailwarning.sti");
	// load background
	LoadDesktopBackground();

	guiCurrentLaptopMode  = LAPTOP_MODE_NONE;
	guiPreviousLaptopMode = LAPTOP_MODE_NONE;
	guiCurrentWWWMode     = LAPTOP_MODE_NONE;
	CreateLapTopMouseRegions();
	RenderLapTopImage();

	// reset bookmarks flags
	fFirstTimeInLaptop = TRUE;

	// reset all bookmark visits
	std::fill(std::begin(LaptopSaveInfo.fVisitedBookmarkAlready), std::end(LaptopSaveInfo.fVisitedBookmarkAlready), 0);

	// init program states
	std::fill(std::begin(gLaptopProgramStates), std::end(gLaptopProgramStates), LAPTOP_PROGRAM_MINIMIZED);

	// turn the power on
	fPowerLightOn = TRUE;

	// we are not exiting laptop right now, we just got here
	fExitingLaptopFlag = FALSE;

	// reset program we are maximizing
	bProgramBeingMaximized = -1;

	// reset fact we are maximizing/ mining
	fMaximizingProgram = FALSE;
	fMinizingProgram = FALSE;


	// initialize open queue
	InitLaptopOpenQueue();


	gfShowBookmarks = FALSE;
	SetBookMark(AIM_BOOKMARK);

	DrawDeskTopBackground();

	// create region for new mail icon
	CreateDestroyMouseRegionForNewMailIcon();

	//DEF: Added to Init things in various laptop pages
	EnterLaptopInitLaptopPages();
	InitalizeSubSitesList();

	InvalidateScreen();
}


static void CreateDestoryBookMarkRegions(void);
static void CreateDestroyMinimizeButtonForCurrentMode(void);
static void DeleteBookmark(void);
static void DeleteDesktopBackground(void);
static void DeleteLapTopButtons(void);
static void DeleteLapTopMouseRegions(void);
static void DeleteLoadPending(void);
static void ExitLaptopMode(LaptopMode uiMode);


void ExitLaptop(void)
{
	// exit is called due to message box, leave
	if (fExitDueToMessageBox)
	{
		fExitDueToMessageBox = FALSE;
		return;
	}

	if (DidGameJustStart())
	{
		SetMusicMode(MUSIC_LAPTOP);
	}
	else
	{
		// Restore to old stuff...
		SetMusicMode(MUSIC_RESTORE);
	}

	// Start ambients...
	BuildDayAmbientSounds();

	//if its raining, start the rain showers
	if (IsItRaining())
	{
		//Raise the volume to where it was
		guiRainLoop = PlayJA2Ambient(RAIN_1, MIDVOLUME, 0);
	}

	// release cursor
	FreeMouseCursor();

	// set the fact we are currently not in laptop, for rendering purposes
	fCurrentlyInLaptop = FALSE;

	//Deallocate, save data -- leaving laptop.
	SetRenderFlags(RENDER_FLAG_FULL);

	ExitLaptopMode(fExitDuringLoad ? guiPreviousLaptopMode : guiCurrentLaptopMode);

	fExitDuringLoad  = FALSE;
	fLoadPendingFlag = FALSE;


	RemoveVObject(guiLAPTOP);
	DeleteVideoObject(guiLaptopBACKGROUND);
	RemoveVObject(guiTITLEBARLAPTOP);
	RemoveVObject(guiLIGHTS);
	DeleteVideoObject(guiTITLEBARICONS);
	DeleteVideoObject(guiEmailWarning);

	// destroy region for new mail icon
	CreateDestroyMouseRegionForNewMailIcon();

	// get rid of desktop
	DeleteDesktopBackground();

	if (MailToDelete != NULL)
	{
		MailToDelete = NULL;
		CreateDestroyDeleteNoticeMailButton();
	}
	if (fNewMailFlag)
	{
		fNewMailFlag = FALSE;
		CreateDestroyNewMailButton();
	}

	// get rid of minize button
	CreateDestroyMinimizeButtonForCurrentMode();

	//MSYS_SetCurrentCursor(CURSOR_NORMAL);
	gfEnterLapTop=TRUE;
	DeleteLapTopButtons();
	DeleteLapTopMouseRegions();
	gfShowBookmarks = FALSE;
	CreateDestoryBookMarkRegions();

	fNewWWW = TRUE;
	DeleteBookmark();
	DeleteLoadPending();
	fReDrawNewMailFlag = FALSE;

	//pause the game because we dont want time to advance in the laptop
	UnPauseGame();
}


static void RenderLapTopImage(void)
{
	if (fMaximizingProgram || fMinizingProgram) return;

	BltVideoObject(FRAME_BUFFER, guiLAPTOP,           0, LAPTOP_X,      LAPTOP_Y);
	BltVideoObject(FRAME_BUFFER, guiLaptopBACKGROUND, 1, LAPTOP_X + 25, LAPTOP_Y + 23);

	MarkButtonsDirty();
}


static void RenderLaptop(void)
{
	if (fMaximizingProgram || fMinizingProgram)
	{
		gfShowBookmarks = FALSE;
		return;
	}

	LaptopMode uiTempMode = LAPTOP_MODE_NONE;
	if (fLoadPendingFlag || fExitDuringLoad)
	{
		uiTempMode           = guiCurrentLaptopMode;
		guiCurrentLaptopMode = guiPreviousLaptopMode;
	}

	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_NONE:                     DrawDeskTopBackground();   break;

		case LAPTOP_MODE_AIM:                      RenderAIM();               break;
		case LAPTOP_MODE_AIM_MEMBERS:              RenderAIMMembers();        break;
		case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX: RenderAimFacialIndex();    break;
		case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES: RenderAimSort();           break;
		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:     RenderAimArchives();       break;
		case LAPTOP_MODE_AIM_POLICIES:             RenderAimPolicies();       break;
		case LAPTOP_MODE_AIM_LINKS:                RenderAimLinks();          break;
		case LAPTOP_MODE_AIM_HISTORY:              RenderAimHistory();        break;

		case LAPTOP_MODE_MERC:                     RenderMercs();             break;
		case LAPTOP_MODE_MERC_FILES:               RenderMercsFiles();        break;
		case LAPTOP_MODE_MERC_ACCOUNT:             RenderMercsAccount();      break;
		case LAPTOP_MODE_MERC_NO_ACCOUNT:          RenderMercsNoAccount();    break;

		case LAPTOP_MODE_BOBBY_R:                  RenderBobbyR();            break;
		case LAPTOP_MODE_BOBBY_R_GUNS:             RenderBobbyRGuns();        break;
		case LAPTOP_MODE_BOBBY_R_AMMO:             RenderBobbyRAmmo();        break;
		case LAPTOP_MODE_BOBBY_R_ARMOR:            RenderBobbyRArmour();      break;
		case LAPTOP_MODE_BOBBY_R_MISC:             RenderBobbyRMisc();        break;
		case LAPTOP_MODE_BOBBY_R_USED:             RenderBobbyRUsed();        break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:        RenderBobbyRMailOrder();   break;
		case LAPTOP_MODE_BOBBYR_SHIPMENTS:         RenderBobbyRShipments();   break;

		case LAPTOP_MODE_CHAR_PROFILE:             RenderCharProfile();       break;

		case LAPTOP_MODE_FLORIST:                  RenderFlorist();           break;
		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:   RenderFloristGallery();    break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:        RenderFloristOrderForm();  break;
		case LAPTOP_MODE_FLORIST_CARD_GALLERY:     RenderFloristCards();      break;

		case LAPTOP_MODE_INSURANCE:                RenderInsurance();         break;
		case LAPTOP_MODE_INSURANCE_INFO:           RenderInsuranceInfo();     break;
		case LAPTOP_MODE_INSURANCE_CONTRACT:       RenderInsuranceContract(); break;
		case LAPTOP_MODE_INSURANCE_COMMENTS:       RenderInsuranceComments(); break;

		case LAPTOP_MODE_FUNERAL:                  RenderFuneral();           break;

		case LAPTOP_MODE_FINANCES:                 RenderFinances();          break;
		case LAPTOP_MODE_PERSONNEL:                RenderPersonnel();         break;
		case LAPTOP_MODE_HISTORY:                  RenderHistory();           break;
		case LAPTOP_MODE_FILES:                    RenderFiles();             break;
		case LAPTOP_MODE_EMAIL:                    RenderEmail();             break;

		case LAPTOP_MODE_WWW:                      DrawDeskTopBackground();   break;

		case LAPTOP_MODE_BROKEN_LINK:              RenderBrokenLink();        break;
				default:
						break;
	}

	if (guiCurrentLaptopMode >= LAPTOP_MODE_WWW)
	{
		// render program bar for www program
		RenderWWWProgramTitleBar();
	}

	if (fLoadPendingFlag)
	{
		guiCurrentLaptopMode = uiTempMode;
		return;
	}

	DisplayProgramBoundingBox(FALSE);

	// mark the buttons dirty at this point
	MarkButtonsDirty();
}


static void InitTitleBarMaximizeGraphics(cache_key_t const uiBackgroundGraphic, const ST::string& str, const SGPVObject* uiIconGraphic, UINT16 usIconGraphicIndex);
static void SetSubSiteAsVisted(void);


static void EnterNewLaptopMode(void)
{
	static BOOLEAN fOldLoadFlag = FALSE;

	if (fExitingLaptopFlag) return;
	// cause flicker, as we are going to a new program/WEB page
	fFlickerHD = TRUE;

	// handle maximizing of programs
	UINT           prog;
	ST::string title;
	UINT16         gfx_idx;
	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_EMAIL:
			prog    = LAPTOP_PROGRAM_MAILER;
			title   = pLaptopTitles[0];
			gfx_idx = 0;
			break;

		case LAPTOP_MODE_FILES:
			prog    = LAPTOP_PROGRAM_FILES;
			title   = pLaptopTitles[1];
			gfx_idx = 2;
			break;

		case LAPTOP_MODE_PERSONNEL:
			prog    = LAPTOP_PROGRAM_PERSONNEL;
			title   = pLaptopTitles[2];
			gfx_idx = 3;
			break;

		case LAPTOP_MODE_FINANCES:
			prog    = LAPTOP_PROGRAM_FINANCES;
			title   = pLaptopTitles[3];
			gfx_idx = 5;
			break;

		case LAPTOP_MODE_HISTORY:
			prog    = LAPTOP_PROGRAM_HISTORY;
			title   = pLaptopTitles[4];
			gfx_idx = 4;
			break;

		case LAPTOP_MODE_NONE: goto do_nothing;

		default:
			prog    = LAPTOP_PROGRAM_WEB_BROWSER;
			title   = pWebTitle;
			gfx_idx = 1;
			break;
	}

	if (gLaptopProgramStates[prog] == LAPTOP_PROGRAM_MINIMIZED)
	{
		if (!fMaximizingProgram)
		{
			fInitTitle = TRUE;
			InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, title, guiTITLEBARICONS, gfx_idx);
			ExitLaptopMode(guiPreviousLaptopMode);
		}
		fMaximizingProgram         = TRUE;
		bProgramBeingMaximized     = prog;
		gLaptopProgramStates[prog] = LAPTOP_PROGRAM_OPEN;
		return;
	}

do_nothing:
	if (fMaximizingProgram || fMinizingProgram) return;

	if (fOldLoadFlag && !fLoadPendingFlag)
	{
		fOldLoadFlag = FALSE;
	}
	else if (fLoadPendingFlag && !fOldLoadFlag)
	{
		ExitLaptopMode(guiPreviousLaptopMode);
		fOldLoadFlag = TRUE;
		return;
	}
	else if (fOldLoadFlag && fLoadPendingFlag)
	{
		return;
	}
	else
	{
		// do not exit previous mode if coming from sliding bar handler
		if (!fEnteredNewLapTopDueToHandleSlidingBars)
		{
			ExitLaptopMode(guiPreviousLaptopMode);
		}
	}

	if (guiCurrentWWWMode == LAPTOP_MODE_NONE && guiCurrentLaptopMode >= LAPTOP_MODE_WWW)
	{
		RenderLapTopImage();
		guiCurrentLaptopMode = LAPTOP_MODE_WWW;
	}
	else if (guiCurrentLaptopMode > LAPTOP_MODE_WWW)
	{
		if (guiPreviousLaptopMode < LAPTOP_MODE_WWW)
		{
			guiCurrentLaptopMode = guiCurrentWWWMode;
		}
		else
		{
			guiCurrentWWWMode = guiCurrentLaptopMode;
			giCurrentSubPage  = 0;
		}
	}

	if (guiCurrentLaptopMode >= LAPTOP_MODE_WWW)
	{
		RenderWWWProgramTitleBar();
		if (guiPreviousLaptopMode >= LAPTOP_MODE_WWW) gfShowBookmarks = FALSE;
	}

	guiLastExitedLaptopMode = LAPTOP_MODE_NONE;

	//Initialize the new mode.
	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_AIM:                      EnterAIM();               break;
		case LAPTOP_MODE_AIM_MEMBERS:              EnterAIMMembers();        break;
		case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX: EnterAimFacialIndex();    break;
		case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES: EnterAimSort();           break;
		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:     EnterAimArchives();       break;
		case LAPTOP_MODE_AIM_POLICIES:             EnterAimPolicies();       break;
		case LAPTOP_MODE_AIM_LINKS:                EnterAimLinks();          break;
		case LAPTOP_MODE_AIM_HISTORY:              EnterAimHistory();        break;

		case LAPTOP_MODE_MERC:                     EnterMercs();             break;
		case LAPTOP_MODE_MERC_FILES:               EnterMercsFiles();        break;
		case LAPTOP_MODE_MERC_ACCOUNT:             EnterMercsAccount();      break;
		case LAPTOP_MODE_MERC_NO_ACCOUNT:          EnterMercsNoAccount();    break;

		case LAPTOP_MODE_BOBBY_R:                  EnterBobbyR();            break;
		case LAPTOP_MODE_BOBBY_R_GUNS:             EnterBobbyRGuns();        break;
		case LAPTOP_MODE_BOBBY_R_AMMO:             EnterBobbyRAmmo();        break;
		case LAPTOP_MODE_BOBBY_R_ARMOR:            EnterBobbyRArmour();      break;
		case LAPTOP_MODE_BOBBY_R_MISC:             EnterBobbyRMisc();        break;
		case LAPTOP_MODE_BOBBY_R_USED:             EnterBobbyRUsed();        break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:        EnterBobbyRMailOrder();   break;
		case LAPTOP_MODE_BOBBYR_SHIPMENTS:         EnterBobbyRShipments();   break;

		case LAPTOP_MODE_CHAR_PROFILE:             EnterCharProfile();       break;

		case LAPTOP_MODE_FLORIST:                  EnterFlorist();           break;
		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:   EnterFloristGallery();    break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:        EnterFloristOrderForm();  break;
		case LAPTOP_MODE_FLORIST_CARD_GALLERY:     EnterFloristCards();      break;

		case LAPTOP_MODE_INSURANCE:                EnterInsurance();         break;
		case LAPTOP_MODE_INSURANCE_INFO:           EnterInsuranceInfo();     break;
		case LAPTOP_MODE_INSURANCE_CONTRACT:       EnterInsuranceContract(); break;
		case LAPTOP_MODE_INSURANCE_COMMENTS:       EnterInsuranceComments(); break;

		case LAPTOP_MODE_FUNERAL:                  EnterFuneral();           break;

		case LAPTOP_MODE_FINANCES:                 EnterFinances();          break;
		case LAPTOP_MODE_PERSONNEL:                EnterPersonnel();         break;
		case LAPTOP_MODE_HISTORY:                  EnterHistory();           break;
		case LAPTOP_MODE_FILES:                    EnterFiles();             break;
		case LAPTOP_MODE_EMAIL:                    EnterEmail();             break;

		case LAPTOP_MODE_BROKEN_LINK:              EnterBrokenLink();        break;
				default:
						break;
	}

	// first time using webbrowser in this laptop session
	if (fFirstTimeInLaptop && guiCurrentLaptopMode >= LAPTOP_MODE_WWW)
	{
		gfShowBookmarks    = TRUE;
		fFirstTimeInLaptop = FALSE;
	}

	if (!fLoadPendingFlag)
	{
		CreateDestroyMinimizeButtonForCurrentMode();
		guiPreviousLaptopMode = guiCurrentLaptopMode;
		SetSubSiteAsVisted();
	}

	DisplayProgramBoundingBox(TRUE);
}


static void HandleLapTopHandles(void)
{
	if (fLoadPendingFlag) return;

	if (fMaximizingProgram || fMinizingProgram) return;

	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_AIM:                      HandleAIM();               break;
		case LAPTOP_MODE_AIM_MEMBERS:              HandleAIMMembers();        break;
		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:     HandleAimArchives();       break;
		case LAPTOP_MODE_AIM_POLICIES:             HandleAimPolicies();       break;

		case LAPTOP_MODE_MERC:                     HandleMercs();             break;
		case LAPTOP_MODE_MERC_ACCOUNT:             HandleMercsAccount();      break;

		case LAPTOP_MODE_BOBBY_R:                  HandleBobbyR();            break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:        HandleBobbyRMailOrder();   break;
		case LAPTOP_MODE_BOBBYR_SHIPMENTS:         HandleBobbyRShipments();   break;

		case LAPTOP_MODE_CHAR_PROFILE:             HandleCharProfile();       break;

		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:   HandleFloristGallery();    break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:        HandleFloristOrderForm();  break;

		case LAPTOP_MODE_INSURANCE_CONTRACT:       HandleInsuranceContract(); break;

		case LAPTOP_MODE_PERSONNEL:                HandlePersonnel();         break;
		case LAPTOP_MODE_FILES:                    HandleFiles();             break;
		case LAPTOP_MODE_EMAIL:                    HandleEmail();             break;
				default:
						break;
	}
}


static void CheckIfNewWWWW(void);
static void CheckMarkButtonsDirtyFlag(void);
static void CreateLaptopButtons(void);
static void DisplayBookMarks(void);
static void DisplayLoadPending(void);
static void DisplayTaskBarIcons(void);
static void DisplayWebBookMarkNotify(void);
static void FlickerHDLight(void);
static void HandleSlidingTitleBar(void);
static void HandleWWWSubSites(void);
static void LeaveLapTopScreen(void);
static void PostButtonRendering(void);
static void ShouldNewMailBeDisplayed(void);
static void ShowLights(void);
static void UpdateStatusOfDisplayingBookMarks(void);


ScreenID LaptopScreenHandle()
{
	//User just changed modes.  This is determined by the button callbacks
	//created in LaptopScreenInit()

	// just entered
	if (gfEnterLapTop)
	{
		EnterLaptop();
		CreateLaptopButtons();
		gfEnterLapTop = FALSE;
	}

	if (gfStartMapScreenToLaptopTransition)
	{ //Everything is set up to start the transition animation.
		SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
		//Step 1:  Build the laptop image into the save buffer.
		gfStartMapScreenToLaptopTransition = FALSE;
		RestoreBackgroundRects();
		RenderLapTopImage();
		RenderLaptop();
		RenderButtons();
		PrintDate();
		PrintBalance();
		PrintNumberOnTeam();
		ShowLights();

		//Step 2:  The mapscreen image is in the EXTRABUFFER, and laptop is in the SAVEBUFFER
		//         Start transitioning the screen.
		SGPBox const DstRect = { STD_SCREEN_X, STD_SCREEN_Y, MAP_SCREEN_WIDTH, MAP_SCREEN_HEIGHT };
		const UINT32 uiTimeRange = 1000;
		INT32 iPercentage     = 0;
		INT32 iRealPercentage = 0;
		const UINT32 uiStartTime = GetClock();
		BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER,   0, 0, NULL);
		BltVideoSurface(FRAME_BUFFER,  guiEXTRABUFFER, 0, 0, NULL);
		PlayJA2SampleFromFile(SOUNDSDIR "/laptop power up (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
		while (iRealPercentage < 100)
		{
			const UINT32 uiCurrTime = GetClock();
			iPercentage = (uiCurrTime-uiStartTime) * 100 / uiTimeRange;
			iPercentage = std::min(iPercentage, 100);

			iRealPercentage = iPercentage;

			//Factor the percentage so that it is modified by a gravity falling acceleration effect.
			const INT32 iFactor = (iPercentage - 50) * 2;
			if (iPercentage < 50)
			{
				iPercentage += iPercentage         * iFactor * 0.01 + 0.5;
			}
			else
			{
				iPercentage += (100 - iPercentage) * iFactor * 0.01 + 0.5;
			}

			INT32 iScalePercentage;
			if (iPercentage < 99)
			{
				iScalePercentage = 10000 / (100 - iPercentage);
			}
			else
			{
				iScalePercentage = 5333;
			}
			const UINT16 uWidth  = 12 * iScalePercentage / 100;
			const UINT16 uHeight =  9 * iScalePercentage / 100;
			const UINT16 uX      = 472 - (472 - 320) * iScalePercentage / 5333;
			const UINT16 uY      = 424 - (424 - 240) * iScalePercentage / 5333;

			SGPBox const SrcRect2 = { (UINT16)(STD_SCREEN_X + uX - uWidth / 2), (UINT16)(STD_SCREEN_Y + uY - uHeight / 2), uWidth, uHeight };

			BltStretchVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, &DstRect, &SrcRect2);
			InvalidateScreen();
			RefreshScreen();
		}
		fReDrawScreenFlag = TRUE;
	}

	//DO NOT MOVE THIS FUNCTION CALL!!!

	//This determines if the help screen should be active
	if (ShouldTheHelpScreenComeUp(HELP_SCREEN_LAPTOP, FALSE))
	{
		// handle the help screen
		HelpScreenHandler();
		return LAPTOP_SCREEN;
	}

	RestoreBackgroundRects();

	// lock cursor to screen
	RestrictMouseCursor(&LaptopScreenRect);

	// handle animated cursors
	HandleAnimatedCursors();
	// Deque all game events
	DequeAllGameEvents();

	// handle sub sites..like BR Guns, BR Ammo, Armour, Misc...for WW Wait..since they are not true sub pages
	// and are not individual sites
	HandleWWWSubSites();
	UpdateStatusOfDisplayingBookMarks();

	// check if we need to reset new WWW mode
	CheckIfNewWWWW();

	if (guiCurrentLaptopMode != guiPreviousLaptopMode)
	{
		if (guiCurrentLaptopMode <=LAPTOP_MODE_WWW) fLoadPendingFlag = FALSE;

		if (!fMaximizingProgram && !fMinizingProgram)
		{
			if (guiCurrentLaptopMode <= LAPTOP_MODE_WWW)
			{
				EnterNewLaptopMode();
				if (!fMaximizingProgram && !fMinizingProgram)
				{
					guiPreviousLaptopMode = guiCurrentLaptopMode;
				}
			}
			else
			{
				if (!fLoadPendingFlag)
				{
					EnterNewLaptopMode();
					guiPreviousLaptopMode = guiCurrentLaptopMode;
				}
			}
		}
	}
	if (fPausedReDrawScreenFlag)
	{
		fReDrawScreenFlag       = TRUE;
		fPausedReDrawScreenFlag = FALSE;
	}

	if (fReDrawScreenFlag)
	{
		RenderLapTopImage();
		RenderLaptop();
	}

	// are we about to leave laptop
	if (fExitingLaptopFlag)
	{
		if (fLoadPendingFlag)
		{
			fLoadPendingFlag = FALSE;
			fExitDuringLoad  = TRUE;
		}
		LeaveLapTopScreen();
	}

	if (!fExitingLaptopFlag)
	{
		// handle handles for laptop input stream
		HandleLapTopHandles();
	}

	// get keyboard input, handle it
	GetLaptopKeyboardInput();

	// check to see if new mail box needs to be displayed
	CreateDestroyNewMailButton();
	DisplayNewMailBox();

	// create various mouse regions that are global to laptop system
	CreateDestoryBookMarkRegions();

	// check to see if buttons marked dirty
	CheckMarkButtonsDirtyFlag();

	// check to see if new mail box needs to be displayed
	ShouldNewMailBeDisplayed();

	// check to see if new mail box needs to be displayed
	ReDrawNewMailBox();

	// look for unread email
	LookForUnread();
	//Handle keyboard shortcuts...

	if (!fLoadPendingFlag || fNewMailFlag)
	{
		// render buttons marked dirty
		RenderButtons();
	}

	// check to see if bookmarks need to be displayed
	if (gfShowBookmarks)
	{
		if (fExitingLaptopFlag)
		{
			gfShowBookmarks = FALSE;
		}
		else
		{
			DisplayBookMarks();
		}
	}

	// check to see if laod pending flag is set
	DisplayLoadPending();

	// check if we are showing message?
	DisplayWebBookMarkNotify();

	if (fReDrawPostButtonRender)
	{
		// rendering AFTER buttons and button text
		if (!fMaximizingProgram && !fMinizingProgram)
		{
			PostButtonRendering();
		}
	}

	PrintDate();
	PrintBalance();
	PrintNumberOnTeam();
	DisplayTaskBarIcons();

	// handle if we are maximizing a program from a minimized state or vice versa
	HandleSlidingTitleBar();

	// flicker HD light as nessacary
	FlickerHDLight();

	// display power and HD lights
	ShowLights();

	// invalidate screen if redrawn
	if (fReDrawScreenFlag)
	{
		InvalidateScreen();
		fReDrawScreenFlag = FALSE;
	}

	ExecuteVideoOverlays();

	SaveBackgroundRects();
	RenderFastHelp();

	// ex SAVEBUFFER queue
	ResetInterface();
	return (LAPTOP_SCREEN);
}


static void ExitLaptopMode(LaptopMode uiMode)
{
	if (guiLastExitedLaptopMode == uiMode) {
		return;
	}

	// Deallocate the previous mode that you were in.
	switch (uiMode)
	{
		case LAPTOP_MODE_AIM:                      ExitAIM();               break;
		case LAPTOP_MODE_AIM_MEMBERS:              ExitAIMMembers();        break;
		case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX: ExitAimFacialIndex();    break;
		case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES: ExitAimSort();           break;
		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:     ExitAimArchives();       break;
		case LAPTOP_MODE_AIM_POLICIES:             ExitAimPolicies();       break;
		case LAPTOP_MODE_AIM_LINKS:                ExitAimLinks();          break;
		case LAPTOP_MODE_AIM_HISTORY:              ExitAimHistory();        break;

		case LAPTOP_MODE_MERC:                     ExitMercs();             break;
		case LAPTOP_MODE_MERC_FILES:               ExitMercsFiles();        break;
		case LAPTOP_MODE_MERC_ACCOUNT:             ExitMercsAccount();      break;
		case LAPTOP_MODE_MERC_NO_ACCOUNT:          ExitMercsNoAccount();    break;

		case LAPTOP_MODE_BOBBY_R:                  ExitBobbyR();            break;
		case LAPTOP_MODE_BOBBY_R_GUNS:             ExitBobbyRGuns();        break;
		case LAPTOP_MODE_BOBBY_R_AMMO:             ExitBobbyRAmmo();        break;
		case LAPTOP_MODE_BOBBY_R_ARMOR:            ExitBobbyRArmour();      break;
		case LAPTOP_MODE_BOBBY_R_MISC:             ExitBobbyRMisc();        break;
		case LAPTOP_MODE_BOBBY_R_USED:             ExitBobbyRUsed();        break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:        ExitBobbyRMailOrder();   break;
		case LAPTOP_MODE_BOBBYR_SHIPMENTS:         ExitBobbyRShipments();   break;

		case LAPTOP_MODE_CHAR_PROFILE:             ExitCharProfile();       break;

		case LAPTOP_MODE_FLORIST:                  ExitFlorist();           break;
		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:   ExitFloristGallery();    break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:        ExitFloristOrderForm();  break;
		case LAPTOP_MODE_FLORIST_CARD_GALLERY:     ExitFloristCards();      break;

		case LAPTOP_MODE_INSURANCE:                ExitInsurance();         break;
		case LAPTOP_MODE_INSURANCE_INFO:           ExitInsuranceInfo();     break;
		case LAPTOP_MODE_INSURANCE_CONTRACT:       ExitInsuranceContract(); break;
		case LAPTOP_MODE_INSURANCE_COMMENTS:       ExitInsuranceComments(); break;

		case LAPTOP_MODE_FUNERAL:                  ExitFuneral();           break;

		case LAPTOP_MODE_FINANCES:                 ExitFinances();          break;
		case LAPTOP_MODE_PERSONNEL:                ExitPersonnel();         break;
		case LAPTOP_MODE_HISTORY:                  ExitHistory();           break;
		case LAPTOP_MODE_FILES:                    ExitFiles();             break;
		case LAPTOP_MODE_EMAIL:                    ExitEmail();             break;
		case LAPTOP_MODE_BROKEN_LINK:              ExitBrokenLink();        break;

		// nothing to do for other subwindows
		default: break;
	}

	if (uiMode != LAPTOP_MODE_NONE && uiMode < LAPTOP_MODE_WWW)
	{
		CreateDestroyMinimizeButtonForCurrentMode();
	}

	guiLastExitedLaptopMode = uiMode;
}


static void MakeButton(UINT idx, INT16 y, GUI_CALLBACK click, INT8 off_x, const ST::string& text, const ST::string& help_text)
{
	GUIButtonRef const btn = QuickCreateButtonImg(LAPTOPDIR "/buttonsforlaptop.sti", idx, idx + 8, (29 + STD_SCREEN_X), (y + STD_SCREEN_Y), MSYS_PRIORITY_HIGH, click);
	gLaptopButton[idx] = btn;
	btn->SetFastHelpText(help_text);
	btn->SpecifyTextOffsets(off_x, 11, TRUE);
	btn->SpecifyGeneralTextAttributes(text, FONT10ARIAL, 2, 0);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
}


static void BtnOnCallback(                GUI_BUTTON* btn, UINT32 reason);
static void EmailRegionButtonCallback(    GUI_BUTTON* btn, UINT32 reason);
static void FilesRegionButtonCallback(    GUI_BUTTON* btn, UINT32 reason);
static void FinancialRegionButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void HistoryRegionButtonCallback(  GUI_BUTTON* btn, UINT32 reason);
static void PersonnelRegionButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void WWWRegionButtonCallbackPrimary(GUI_BUTTON* btn, UINT32 reason);
static void WWWRegionButtonCallbackSecondary(GUI_BUTTON* btn, UINT32 reason);


static void CreateLaptopButtons(void)
{
	MakeButton(0,  66, EmailRegionButtonCallback,     30, pLaptopIcons[0], gzLaptopHelpText[LAPTOP_BN_HLP_TXT_VIEW_EMAIL]);
	MakeButton(1,  98, ButtonCallbackPrimarySecondary(WWWRegionButtonCallbackPrimary, WWWRegionButtonCallbackSecondary), 30, pLaptopIcons[1], gzLaptopHelpText[LAPTOP_BN_HLP_TXT_BROWSE_VARIOUS_WEB_SITES]);
	MakeButton(2, 130, FilesRegionButtonCallback,     30, pLaptopIcons[5], gzLaptopHelpText[LAPTOP_BN_HLP_TXT_VIEW_FILES_AND_EMAIL_ATTACHMENTS]);
	MakeButton(3, 194, PersonnelRegionButtonCallback, 30, pLaptopIcons[3], gzLaptopHelpText[LAPTOP_BN_HLP_TXT_VIEW_TEAM_INFO]);
	MakeButton(4, 162, HistoryRegionButtonCallback,   30, pLaptopIcons[4], gzLaptopHelpText[LAPTOP_BN_HLP_TXT_READ_LOG_OF_EVENTS]);
	MakeButton(5, 241, FinancialRegionButtonCallback, 30, pLaptopIcons[2], gzLaptopHelpText[LAPTOP_BN_HLP_TXT_VIEW_FINANCIAL_SUMMARY_AND_HISTORY]);
	MakeButton(6, 378, BtnOnCallback,                 25, pLaptopIcons[6], gzLaptopHelpText[LAPTOP_BN_HLP_TXT_CLOSE_LAPTOP]);
}


static void DeleteLapTopButtons(void)
{
	for (UINT32 i = 0; i < 7; ++i) RemoveButton(gLaptopButton[i]);
}


static BOOLEAN HandleExit(void);


static void BtnOnCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (HandleExit()) fExitingLaptopFlag = TRUE;
	}
}


static BOOLEAN ExitLaptopDone(void);


static void LeaveLapTopScreen(void)
{
	if (ExitLaptopDone())
	{
		SetLaptopExitScreen(MAP_SCREEN);

		if (gfAtLeastOneMercWasHired)
		{
			if (LaptopSaveInfo.gfNewGameLaptop)
			{
				LaptopSaveInfo.gfNewGameLaptop = FALSE;
				fExitingLaptopFlag = TRUE;
				gfDontStartTransitionFromLaptop = TRUE;
				return;
			}
		}
		else
		{
			gfDontStartTransitionFromLaptop = TRUE;
		}

		SetPendingNewScreen(guiExitScreen);

		if (!gfDontStartTransitionFromLaptop)
		{
			gfDontStartTransitionFromLaptop = TRUE;
			SetCurrentCursorFromDatabase(VIDEO_NO_CURSOR);
			//Step 1:  Build the laptop image into the save buffer.
			RestoreBackgroundRects();
			RenderLapTopImage();
			RenderLaptop();
			RenderButtons();
			PrintDate();
			PrintBalance();
			PrintNumberOnTeam();
			ShowLights();

			//Step 2:  The mapscreen image is in the EXTRABUFFER, and laptop is in the SAVEBUFFER
			//         Start transitioning the screen.
			SGPBox const SrcRect = { STD_SCREEN_X, STD_SCREEN_Y, MAP_SCREEN_WIDTH, MAP_SCREEN_HEIGHT };
			const UINT32 uiTimeRange = 1000;
			INT32 iPercentage     = 100;
			INT32 iRealPercentage = 100;
			const UINT32 uiStartTime = GetClock();
			BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
			PlayJA2SampleFromFile(SOUNDSDIR "/laptop power down (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
			while (iRealPercentage > 0)
			{
				BltVideoSurface(FRAME_BUFFER, guiEXTRABUFFER, 0, 0, NULL);

				const UINT32 uiCurrTime = GetClock();
				iPercentage = (uiCurrTime-uiStartTime) * 100 / uiTimeRange;
				iPercentage = std::min(iPercentage, 100);
				iPercentage = 100 - iPercentage;

				iRealPercentage = iPercentage;

				//Factor the percentage so that it is modified by a gravity falling acceleration effect.
				const INT32 iFactor = (iPercentage - 50) * 2;
				if (iPercentage < 50)
				{
					iPercentage += iPercentage       * iFactor * 0.01 + 0.5;
				}
				else
				{
					iPercentage += (100-iPercentage) * iFactor * 0.01 + 0.5;
				}

				//Scaled laptop
				INT32 iScalePercentage;
				if (iPercentage < 99)
				{
					iScalePercentage = 10000 / (100-iPercentage);
				}
				else
				{
					iScalePercentage = 5333;
				}
				const UINT16 uWidth  = 12 * iScalePercentage / 100;
				const UINT16 uHeight =  9 * iScalePercentage / 100;
				const UINT16 uX = 472 - (472 - 320) * iScalePercentage / 5333;
				const UINT16 uY = 424 - (424 - 240) * iScalePercentage / 5333;

				SGPBox const DstRect = { (UINT16)(STD_SCREEN_X + uX - uWidth / 2), (UINT16)(STD_SCREEN_Y + uY - uHeight / 2), uWidth, uHeight };

				BltStretchVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, &SrcRect, &DstRect);
				InvalidateScreen();
				RefreshScreen();
			}
		}
	}
}


static BOOLEAN HandleExit(void)
{
	// new game, send email
	if (LaptopSaveInfo.gfNewGameLaptop)
	{
		// Set an event to send this email (day 2 8:00-12:00)
		if (!LaptopSaveInfo.fIMPCompletedFlag && !LaptopSaveInfo.fSentImpWarningAlready)
		{
			AddFutureDayStrategicEvent(EVENT_HAVENT_MADE_IMP_CHARACTER_EMAIL, (8 + Random(4)) * 60, 0, 1);
			fExitingLaptopFlag = TRUE;
			return FALSE;
		}
	}

	return TRUE;
}

void HaventMadeImpMercEmailCallBack()
{
	//if the player STILL hasnt made an imp merc yet
	if (!LaptopSaveInfo.fIMPCompletedFlag && !LaptopSaveInfo.fSentImpWarningAlready)
	{
		LaptopSaveInfo.fSentImpWarningAlready = TRUE;
		AddEmail(IMP_EMAIL_AGAIN,IMP_EMAIL_AGAIN_LENGTH, 1, GetWorldTotalMin());
	}
}


static void CreateLapTopMouseRegions(void)
{
	// the entire laptop display region
	MSYS_DefineRegion(&gLapTopScreenRegion, LaptopScreenRect.iLeft, LaptopScreenRect.iTop, LaptopScreenRect.iRight, LaptopScreenRect.iBottom, MSYS_PRIORITY_NORMAL + 1, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);
}


static void DeleteLapTopMouseRegions(void)
{
	MSYS_RemoveRegion(&gLapTopScreenRegion);
}


static void UpdateListToReflectNewProgramOpened(INT32 iOpenedProgram);


static void FinancialRegionButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (gfShowBookmarks)
		{
			gfShowBookmarks = FALSE;
			fReDrawScreenFlag = TRUE;
		}
		guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;

		UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_FINANCES);
	}
}


static void PersonnelRegionButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
		if (gfShowBookmarks)
		{
			gfShowBookmarks = FALSE;
			fReDrawScreenFlag = TRUE;
		}
		gfShowBookmarks = FALSE;

		UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_PERSONNEL);
	}
}


static void EmailRegionButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		gfShowBookmarks = FALSE;
		UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_MAILER);
		fReDrawScreenFlag = TRUE;
	}
}


static void WWWRegionButtonCallbackPrimary(GUI_BUTTON *btn, UINT32 reason)
{
	// reset show bookmarks
	if (guiCurrentLaptopMode < LAPTOP_MODE_WWW)
	{
		gfShowBookmarks = FALSE;
		fShowBookmarkInfo = TRUE;
	}
	else
	{
		gfShowBookmarks = !gfShowBookmarks;
	}

	if (fNewWWW)
	{
		// no longer a new WWW mode
		fNewWWW = FALSE;

		// make sure program is maximized
		if (gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] == LAPTOP_PROGRAM_OPEN)
		{
			RenderLapTopImage();
			DrawDeskTopBackground();
		}
	}

	guiCurrentLaptopMode = (guiCurrentWWWMode == LAPTOP_MODE_NONE ? LAPTOP_MODE_WWW : guiCurrentWWWMode);

	UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_WEB_BROWSER);
	fReDrawScreenFlag = TRUE;
}

static void WWWRegionButtonCallbackSecondary(GUI_BUTTON *btn, UINT32 reason)
{
	guiCurrentLaptopMode = (guiCurrentWWWMode == LAPTOP_MODE_NONE ? LAPTOP_MODE_WWW : guiCurrentWWWMode);
	fReDrawScreenFlag = TRUE;
}


static void HistoryRegionButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (gfShowBookmarks)
		{
			gfShowBookmarks = FALSE;
		}
		guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;
		UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_HISTORY);
		gfShowBookmarks = FALSE;
		fReDrawScreenFlag = TRUE;
	}
}


static void FilesRegionButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (gfShowBookmarks)
		{
			gfShowBookmarks = FALSE;
		}
		guiCurrentLaptopMode = LAPTOP_MODE_FILES;
		UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_FILES);
		fReDrawScreenFlag = TRUE;
	}
}


static void InitBookMarkList(void)
{
	// sets bookmark list to -1
	std::fill(std::begin(LaptopSaveInfo.iBookMarkList), std::end(LaptopSaveInfo.iBookMarkList), -1);
}


void SetBookMark(INT32 iBookId)
{
	// find first empty spot, set to iBookId
	INT32 i;
	for (i = 0; LaptopSaveInfo.iBookMarkList[i] != -1; ++i)
	{
		// move through list until empty
		if (LaptopSaveInfo.iBookMarkList[i] == iBookId)
		{
			// found it, return
			return;
		}
	}
	LaptopSaveInfo.iBookMarkList[i] = iBookId;
}


static void DisplayBookMarks(void)
{
	// check if we are maximizing or minimizing.. if so, do not display
	if (fMaximizingProgram || fMinizingProgram) return;

	SetFont(BOOK_FONT);
	SetFontBackground(FONT_BLACK);
	SetFontShadow(NO_SHADOW);

	SetFontDestBuffer(FRAME_BUFFER, BOOK_X, BOOK_TOP_Y, BOOK_X + BOOK_WIDTH - 10, SCREEN_HEIGHT);

	// blt in book mark background
	INT32 const h  = BOOK_HEIGHT + 6;
	INT32 const sy = BOOK_TOP_Y + 6 + h;
	INT32       y  = sy;
	HCenterVCenterAlign const alignment{ BOOK_WIDTH - 3, h };
	for (INT32 i = 0;; ++i)
	{
		bool              const highlighted = iHighLightBookLine == i;
		auto * const vo{ highlighted ? guiBOOKHIGH : guiBOOKMARK };
		BltVideoObject(FRAME_BUFFER, vo, 0, BOOK_X, y);

		SetFontForeground(highlighted ? FONT_WHITE : FONT_BLACK);
		INT32          const idx = LaptopSaveInfo.iBookMarkList[i];
		MPrint(BOOK_X + 3, y + 2,
			pBookMarkStrings[idx != -1 ? idx : CANCEL_STRING], alignment);
		y += h;
		if (idx == -1) break;
	}

	SetFontDestBuffer(FRAME_BUFFER);
	SetFontShadow(DEFAULT_SHADOW);

	InvalidateRegion(BOOK_X, sy, BOOK_X + BOOK_WIDTH, y);
}


static void DeleteBookmark(void)
{
	RemoveVObject(guiBOOKHIGH);
	RemoveVObject(guiBOOKMARK);
	RemoveVObject(guiDOWNLOADTOP);
	RemoveVObject(guiDOWNLOADMID);
	RemoveVObject(guiDOWNLOADBOT);
}


static void BookmarkCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void BookmarkMvtCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateBookMarkMouseRegions(void)
{
	// creates regions based on number of entries
	for (INT32 i = 0;; ++i)
	{
		UINT16        const y = BOOK_TOP_Y + (i + 1) * (BOOK_HEIGHT + 6) + 6;
		MOUSE_REGION* const r = &gBookmarkMouseRegions[i];
		MSYS_DefineRegion(r, BOOK_X, y, BOOK_X + BOOK_WIDTH, y + BOOK_HEIGHT + 6, MSYS_PRIORITY_HIGHEST - 2, CURSOR_LAPTOP_SCREEN, BookmarkMvtCallBack, BookmarkCallBack);
		MSYS_SetRegionUserData(r, 0, i);
		INT32 const idx = LaptopSaveInfo.iBookMarkList[i];
		if (idx == -1) break; // just added region for cancel
		r->SetFastHelpText(gzLaptopHelpText[BOOKMARK_TEXT_ASSOCIATION_OF_INTERNATION_MERCENARIES + idx]);
	}
}


static void DeleteBookmarkRegions(void)
{
	INT32 i;
	// deletes bookmark regions
	for (i = 0; LaptopSaveInfo.iBookMarkList[i] != -1; ++i)
	{
		MSYS_RemoveRegion(&gBookmarkMouseRegions[i]);
	}

	// now one for the cancel
	MSYS_RemoveRegion(&gBookmarkMouseRegions[i]);
}


static void CreateDestoryBookMarkRegions(void)
{
	// checks to see if a bookmark needs to be created or destroyed
	static BOOLEAN fOldShowBookmarks = FALSE;

	if (gfShowBookmarks && !fOldShowBookmarks)
	{
		CreateBookMarkMouseRegions();
		fOldShowBookmarks = TRUE;
	}
	else if (!gfShowBookmarks && fOldShowBookmarks)
	{
		DeleteBookmarkRegions();
		fOldShowBookmarks = FALSE;
	}
}


static void BookmarkCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	// we are in process of loading
	if (fLoadPendingFlag) return;

	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		INT32 const i   = MSYS_GetRegionUserData(pRegion, 0);
		INT32 const idx = LaptopSaveInfo.iBookMarkList[i];
		if (idx != -1)
		{
			GoToWebPage(idx);
		}
		else
		{
			gfShowBookmarks   = FALSE;
			fReDrawScreenFlag = TRUE;
		}
	}
}


static void InternetRainDelayMessageBoxCallBack(MessageBoxReturnValue);


void GoToWebPage(INT32 iPageId)
{
	//if it is raining, popup a warning first saying connection time may be slow
	if (IsItRaining())
	{
		if (giRainDelayInternetSite == -1)
		{
			DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, pErrorStrings, LAPTOP_SCREEN, MSG_BOX_FLAG_OK, InternetRainDelayMessageBoxCallBack);
			giRainDelayInternetSite = iPageId;
			return;
		}
	}
	else
	{
		giRainDelayInternetSite = -1;
	}

	gfShowBookmarks   = FALSE;
	fReDrawScreenFlag = TRUE;

	switch (iPageId)
	{
		case AIM_BOOKMARK:
			guiCurrentWWWMode    = LAPTOP_MODE_AIM;
			guiCurrentLaptopMode = LAPTOP_MODE_AIM;
			break;

		case BOBBYR_BOOKMARK:
			guiCurrentWWWMode    = LAPTOP_MODE_BOBBY_R;
			guiCurrentLaptopMode = LAPTOP_MODE_BOBBY_R;
			break;

		case IMP_BOOKMARK:
			guiCurrentWWWMode    = LAPTOP_MODE_CHAR_PROFILE;
			guiCurrentLaptopMode = LAPTOP_MODE_CHAR_PROFILE;
			iCurrentImpPage = IMP_HOME_PAGE;
			break;

		case MERC_BOOKMARK:
			//if the mercs server has gone down, but hasnt come up yet
			if (LaptopSaveInfo.fMercSiteHasGoneDownYet &&
					LaptopSaveInfo.fFirstVisitSinceServerWentDown == FALSE)
			{
				guiCurrentWWWMode    = LAPTOP_MODE_BROKEN_LINK;
				guiCurrentLaptopMode = LAPTOP_MODE_BROKEN_LINK;
			}
			else
			{
				guiCurrentWWWMode    = LAPTOP_MODE_MERC;
				guiCurrentLaptopMode = LAPTOP_MODE_MERC;
			}
			break;

		case FUNERAL_BOOKMARK:
			guiCurrentWWWMode    = LAPTOP_MODE_FUNERAL;
			guiCurrentLaptopMode = LAPTOP_MODE_FUNERAL;
			break;

		case FLORIST_BOOKMARK:
			guiCurrentWWWMode    = LAPTOP_MODE_FLORIST;
			guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
			break;

		case INSURANCE_BOOKMARK:
			guiCurrentWWWMode    = LAPTOP_MODE_INSURANCE;
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
			break;

		default: return;
	}

	// do we have to have a World Wide Wait
	fFastLoadFlag = LaptopSaveInfo.fVisitedBookmarkAlready[iPageId];
	LaptopSaveInfo.fVisitedBookmarkAlready[iPageId] = TRUE;
	fLoadPendingFlag = TRUE;
}


static void BookmarkMvtCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		iHighLightBookLine = -1;
	}
	else if (iReason & MSYS_CALLBACK_REASON_MOVE)
	{
		iHighLightBookLine=MSYS_GetRegionUserData(pRegion, 0);
	}
}


static void DisplayLoadPending(void)
{
	// this function will display the load pending and return if the load is done
	static UINT32 uiBaseTime  = 0;
	static UINT32 uiTotalTime = 0;

	// if merc webpage, make it longer
	UINT32 uiUnitTime = UNIT_TIME;
	if (fFastLoadFlag)
	{
		uiUnitTime = fConnectingToSubPage ? FASTEST_UNIT_TIME : FAST_UNIT_TIME;
	}
	else if (fConnectingToSubPage)
	{
		uiUnitTime = ALMOST_FAST_UNIT_TIME;
	}
	else if (guiCurrentLaptopMode == LAPTOP_MODE_MERC && !LaptopSaveInfo.fMercSiteHasGoneDownYet)
	{
		// if we are connecting the MERC site, and the MERC site hasnt yet moved
		// to their new site, have the sloooww wait
		uiUnitTime = LONG_UNIT_TIME;
	}

	// increase delay if it's raining
	if (guiEnvWeather & WEATHER_FORECAST_THUNDERSHOWERS)
	{
		uiUnitTime += uiUnitTime * 0.8f;
	}
	else if (guiEnvWeather & WEATHER_FORECAST_SHOWERS)
	{
		uiUnitTime += uiUnitTime * 0.6f;
	}

	// Adjust loading time based on config var
	uiUnitTime *= gamepolicy(website_loading_time_scale);

	UINT32 uiLoadTime = uiUnitTime * 30;

	// we are now waiting on a web page to download, reset counter
	if (!fLoadPendingFlag)
	{
		fDoneLoadPending     = FALSE;
		fFastLoadFlag        = FALSE;
		fConnectingToSubPage = FALSE;
		uiBaseTime           = 0;
		uiTotalTime          = 0;
		return;
	}
	if (uiBaseTime == 0) uiBaseTime = GetJA2Clock();

	// if total time is exceeded, return
	if (uiTotalTime >= uiLoadTime)
	{
		// done loading, redraw screen
		fLoadPendingFlag        = FALSE;
		fFastLoadFlag           = FALSE;
		uiTotalTime             = 0;
		uiBaseTime              = 0;
		fDoneLoadPending        = TRUE;
		fConnectingToSubPage    = FALSE;
		fPausedReDrawScreenFlag = TRUE;
		return;
	}

	const UINT32 uiDifference = GetJA2Clock() - uiBaseTime;

	// difference has been long enough or we are redrawing the screen
	if (uiDifference > uiUnitTime)
	{
		// LONG ENOUGH TIME PASSED
		uiBaseTime   = GetJA2Clock();
		uiTotalTime += uiDifference;
	}

	// new mail, don't redraw
	if (fNewMailFlag) return;

	RenderButtons();

	// display top middle and bottom of box
	BltVideoObject(FRAME_BUFFER, guiDOWNLOADTOP,   0, DOWNLOAD_X,     DOWNLOAD_Y);
	BltVideoObject(FRAME_BUFFER, guiDOWNLOADMID,   0, DOWNLOAD_X,     DOWNLOAD_Y + DOWN_HEIGHT);
	BltVideoObject(FRAME_BUFFER, guiDOWNLOADBOT,   0, DOWNLOAD_X,     DOWNLOAD_Y + DOWN_HEIGHT * 2);
	BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, 1, DOWNLOAD_X + 4, DOWNLOAD_Y + 1);

	SetFontAttributes(DOWNLOAD_FONT, FONT_WHITE, NO_SHADOW);

	// reload or download?
	MPrint(STD_SCREEN_X + 328, DOWN_STRING_Y,
		pDownloadString[fFastLoadFlag ? 1 : 0], CenterAlign(446 - 328));

	BltVideoObject(FRAME_BUFFER, guiGRAPHWINDOW, 0, LAPTOP_WINDOW_X, LAPTOP_WINDOW_Y);

	// check to see if we are only updating screen, but not passed a new element in the load pending display

	// decide how many time units are to be displayed, based on amount of time passed
	for (UINT32 i = 0, uiTempTime = uiTotalTime; i <= 30 && uiTempTime > 0; ++i, uiTempTime -= uiUnitTime)
	{
		BltVideoObject(FRAME_BUFFER, guiGRAPHBAR, 0, LAPTOP_BAR_X + UNIT_WIDTH * i, LAPTOP_BAR_Y);
		if (uiUnitTime > uiTempTime) break; // prevent underflow
	}

	InvalidateRegion(DOWNLOAD_X, DOWNLOAD_Y, DOWNLOAD_X + 150, DOWNLOAD_Y + 100);

	// re draw screen and new mail warning box
	SetFontShadow(DEFAULT_SHADOW);

	MarkButtonsDirty();

	DisableMercSiteButton();
}


static void DeleteLoadPending(void)
{
	// this funtion will delete the load pending graphics
	// reuse bookmark
	RemoveVObject(guiGRAPHBAR);
	RemoveVObject(guiGRAPHWINDOW);
}


// This function is called every time the laptop is FIRST initialized, ie whenever the laptop is loaded.  It calls
// various init function in the laptop pages that need to be inited when the laptop is just loaded.
static void EnterLaptopInitLaptopPages(void)
{
	EnterInitAimMembers();
	EnterInitAimArchives();
	EnterInitAimPolicies();
	EnterInitAimHistory();
	EnterInitFloristGallery();
	EnterInitInsuranceInfo();
	EnterInitBobbyRayOrder();
	EnterInitMercSite();

	// init sub pages for WW Wait
	InitIMPSubPageList();
}


static void CheckMarkButtonsDirtyFlag(void)
{
	// this function checks the fMarkButtonsDirtyFlag, if true, mark buttons dirty
	if (fMarkButtonsDirtyFlag)
	{
		// flag set, mark buttons and reset
		MarkButtonsDirty();
		fMarkButtonsDirtyFlag = FALSE;
	}
}


static void PostButtonRendering(void)
{
	// this function is in place to allow for post button rendering
	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_AIM_MEMBERS: RenderAIMMembersTopLevel(); break;
				default:
						break;
	}
}


static void ShouldNewMailBeDisplayed(void)
{
	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_AIM_MEMBERS: DisableNewMailMessage(); break;
				default:
						break;
	}
}


static void CheckIfNewWWWW(void)
{
	// if no www mode, set new www flag..until new www mode that is not 0
	fNewWWW = (guiCurrentWWWMode == LAPTOP_MODE_NONE);
}


void HandleLapTopESCKey(void)
{
	// will handle esc key events, since handling depends on state of laptop
	if (fNewMailFlag)
	{
		// get rid of new mail warning box
		fNewMailFlag = FALSE;
		CreateDestroyNewMailButton();
	}
	else if (MailToDelete != NULL)
	{
		// get rid of delete mail box
		MailToDelete = NULL;
		CreateDestroyDeleteNoticeMailButton();
	}
	else if (gfShowBookmarks)
	{
		// get rid of bookmarks
		gfShowBookmarks = FALSE;
		RenderLapTopImage();
	}
	else
	{
		// leave
		fExitingLaptopFlag = TRUE;
		HandleExit();
		return;
	}

	// force redraw
	fReDrawScreenFlag = TRUE;
	RenderLaptop();
}


void HandleRightButtonUpEvent(void)
{
	// will handle the right button up event
	if (fNewMailFlag)
	{
		// get rid of new mail warning box
		fNewMailFlag = FALSE;
		CreateDestroyNewMailButton();
	}
	else if (MailToDelete != NULL)
	{
		// get rid of delete mail box
		MailToDelete = NULL;
		CreateDestroyDeleteNoticeMailButton();
	}
	else if (gfShowBookmarks)
	{
		// get rid of bookmarks
		gfShowBookmarks = FALSE;
		RenderLapTopImage();
	}
	else if (fDisplayMessageFlag)
	{
		fDisplayMessageFlag = FALSE;
		RenderLapTopImage();
	}
	else
	{
		fShowBookmarkInfo = FALSE;
		return;
	}

	// force redraw
	fReDrawScreenFlag = TRUE;
	RenderLaptop();
}


static void HandleLeftButtonUpEvent(void)
{
	// will handle the left button up event
	if (gfShowBookmarks)
	{
		// get rid of bookmarks
		gfShowBookmarks = FALSE;

		// force redraw
		fReDrawScreenFlag = TRUE;
		RenderLapTopImage();
		RenderLaptop();
	}
	else if (fShowBookmarkInfo)
	{
		fShowBookmarkInfo = FALSE;
	}
}

void LapTopScreenCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	HandleLeftButtonUpEvent();
}

void LapTopScreenCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	HandleRightButtonUpEvent();
}

MOUSE_CALLBACK LapTopScreenCallBack = MouseCallbackPrimarySecondary(LapTopScreenCallBackPrimary, LapTopScreenCallBackSecondary);

void DoLapTopMessageBox(MessageBoxStyleID ubStyle, const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags ubFlags, MSGBOX_CALLBACK ReturnCallback)
{
	SGPBox const centering_rect = { LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y, LAPTOP_SCREEN_WIDTH, LAPTOP_SCREEN_HEIGHT };
	DoLapTopSystemMessageBoxWithRect(ubStyle, str, uiExitScreen, ubFlags, ReturnCallback, &centering_rect);
}


void DoLapTopSystemMessageBoxWithRect(MessageBoxStyleID ubStyle, const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback, SGPBox const* centering_rect)
{
	// reset exit mode
	fExitDueToMessageBox = TRUE;

	// do message box and return
	DoMessageBox(ubStyle, str, uiExitScreen, usFlags, ReturnCallback, centering_rect);
}


void DoLapTopSystemMessageBox(const ST::string& str, ScreenID uiExitScreen, MessageBoxFlags usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	SGPBox const centering_rect = { 0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y };
	DoLapTopSystemMessageBoxWithRect(MSG_BOX_LAPTOP_DEFAULT, str, uiExitScreen, usFlags, ReturnCallback, &centering_rect);
}


//places a tileable pattern down
void WebPageTileBackground(const UINT8 ubNumX, const UINT8 ubNumY, const UINT16 usWidth, const UINT16 usHeight, const SGPVObject* const background)
{
	UINT16 uiPosY = LAPTOP_SCREEN_WEB_UL_Y;
	for (UINT16 y = 0; y < ubNumY; ++y)
	{
		UINT16 uiPosX = LAPTOP_SCREEN_UL_X;
		for (UINT16 x = 0; x < ubNumX; ++x)
		{
			BltVideoObject(FRAME_BUFFER, background, 0, uiPosX, uiPosY);
			uiPosX += usWidth;
		}
		uiPosY += usHeight;
	}
}


static void InitTitleBarMaximizeGraphics(cache_key_t const uiBackgroundGraphic, const ST::string& str, const SGPVObject* uiIconGraphic, UINT16 usIconGraphicIndex)
{
	// Create a background video surface to blt the title bar onto
	guiTitleBarSurface = AddVideoSurface(LAPTOP_TITLE_BAR_WIDTH + LAPTOP_TITLE_BAR_ICON_OFFSET_X,
						LAPTOP_TITLE_BAR_HEIGHT + LAPTOP_TITLE_BAR_ICON_OFFSET_Y, PIXEL_DEPTH);

	BltVideoObject(guiTitleBarSurface, uiBackgroundGraphic, 0, 0, 0);
	BltVideoObject(guiTitleBarSurface, uiIconGraphic, usIconGraphicIndex, LAPTOP_TITLE_BAR_ICON_OFFSET_X, LAPTOP_TITLE_BAR_ICON_OFFSET_Y);

	SetFontDestBuffer(guiTitleBarSurface);
	DrawTextToScreen(str, LAPTOP_TITLE_BAR_TEXT_OFFSET_X, LAPTOP_TITLE_BAR_TEXT_OFFSET_Y, 0, FONT14ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	SetFontDestBuffer(FRAME_BUFFER);
}


static BOOLEAN DisplayTitleBarMaximizeGraphic(BOOLEAN fForward, BOOLEAN fInit, UINT16 usTopLeftX, UINT16 usTopLeftY, UINT16 usTopRightX)
{
	static INT8   ubCount;
	static SGPBox LastRect;

	if (fInit)
	{
		if (gfTitleBarSurfaceAlreadyActive) return FALSE;
		gfTitleBarSurfaceAlreadyActive = TRUE;
		if (fForward)
		{
			ubCount = 1;
		}
		else
		{
			ubCount = NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS - 1;
		}
	}

	FLOAT dTemp;
	dTemp = (LAPTOP_TITLE_BAR_TOP_LEFT_X  - usTopLeftX)  / (FLOAT)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
	const INT16 sPosX       = usTopLeftX  + dTemp * ubCount;

	dTemp = (LAPTOP_TITLE_BAR_TOP_RIGHT_X - usTopRightX) / (FLOAT)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
	const INT16 sPosRightX  = usTopRightX + dTemp * ubCount;

	dTemp = (LAPTOP_TITLE_BAR_TOP_LEFT_Y  - usTopLeftY)  / (FLOAT)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
	const INT16 sPosY       = usTopLeftY  + dTemp * ubCount;

	const INT16 sPosBottomY = LAPTOP_TITLE_BAR_HEIGHT;

	SGPBox const SrcRect = { 0, 0, LAPTOP_TITLE_BAR_WIDTH, LAPTOP_TITLE_BAR_HEIGHT };

	//if its the last fram, bit the tittle bar to the final position
	SGPBox DestRect;
	if (ubCount == NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS)
	{
		DestRect.x = LAPTOP_TITLE_BAR_TOP_LEFT_X;
		DestRect.y = LAPTOP_TITLE_BAR_TOP_LEFT_Y;
		DestRect.w = LAPTOP_TITLE_BAR_TOP_RIGHT_X - LAPTOP_TITLE_BAR_TOP_LEFT_X;
		DestRect.h = sPosBottomY;
	}
	else
	{
		DestRect.x = sPosX;
		DestRect.y = sPosY;
		DestRect.w = sPosRightX - sPosX;
		DestRect.h = sPosBottomY;
	}

	if (fForward)
	{
		//Restore the old rect
		if (ubCount > 1)
		{
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, LastRect.x, LastRect.y, LastRect.w, LastRect.h);
		}

		//Save rectangle
		if (ubCount > 0)
		{
			BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.x, DestRect.y, DestRect.w, DestRect.h);
		}
	}
	else
	{
		//Restore the old rect
		if (ubCount < NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS - 1)
		{
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, LastRect.x, LastRect.y, LastRect.w, LastRect.h);
		}

		//Save rectangle
		if (ubCount < NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS)
		{
			BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.x, DestRect.y, DestRect.w, DestRect.h);
		}
	}

	BltStretchVideoSurface(FRAME_BUFFER, guiTitleBarSurface, &SrcRect, &DestRect);

	InvalidateRegion(DestRect.x, DestRect.y, DestRect.x + DestRect.w, DestRect.y + DestRect.h);
	InvalidateRegion(LastRect.x, LastRect.y, LastRect.x + LastRect.w, LastRect.y + LastRect.h);

	LastRect = DestRect;

	if (fForward)
	{
		if (ubCount == NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS)
		{
			gfTitleBarSurfaceAlreadyActive = FALSE;
			return TRUE;
		}
		else
		{
			ubCount++;
			return FALSE;
		}
	}
	else
	{
		if (ubCount == 0)
		{
			gfTitleBarSurfaceAlreadyActive = FALSE;
			return TRUE;
		}
		else
		{
			ubCount--;
			return FALSE;
		}
	}
}


static void RemoveTitleBarMaximizeGraphics(void)
{
	DeleteVideoSurface(guiTitleBarSurface);
}


static void HandleSlidingTitleBar(void)
{
	if (fExitingLaptopFlag) return;

	if (fMaximizingProgram)
	{
		UINT16 y;
		switch (bProgramBeingMaximized)
		{
			case LAPTOP_PROGRAM_MAILER:      y = STD_SCREEN_Y +  66; break;
			case LAPTOP_PROGRAM_FILES:       y = STD_SCREEN_Y + 120; break;
			case LAPTOP_PROGRAM_FINANCES:    y = STD_SCREEN_Y + 226; break;
			case LAPTOP_PROGRAM_PERSONNEL:   y = STD_SCREEN_Y + 194; break;
			case LAPTOP_PROGRAM_HISTORY:     y = STD_SCREEN_Y + 162; break;
			case LAPTOP_PROGRAM_WEB_BROWSER: y = STD_SCREEN_Y +  99; break;
			default: goto no_display_max;
		}

		fMaximizingProgram = !DisplayTitleBarMaximizeGraphic(TRUE, fInitTitle, STD_SCREEN_X + 29, y, STD_SCREEN_X + 29 + 20);
		if (!fMaximizingProgram)
		{
			RemoveTitleBarMaximizeGraphics();
			fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
			EnterNewLaptopMode();
			fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
			fPausedReDrawScreenFlag = TRUE;
		}

no_display_max:
		MarkButtonsDirty();
	}
	else if (fMinizingProgram)
	{
		// minimizing
		UINT16 y;
		switch (bProgramBeingMaximized)
		{
			case LAPTOP_PROGRAM_MAILER:      y = STD_SCREEN_Y +  66; break;
			case LAPTOP_PROGRAM_FILES:       y = STD_SCREEN_Y + 130; break; // XXX only difference to max case
			case LAPTOP_PROGRAM_FINANCES:    y = STD_SCREEN_Y + 226; break;
			case LAPTOP_PROGRAM_PERSONNEL:   y = STD_SCREEN_Y + 194; break;
			case LAPTOP_PROGRAM_HISTORY:     y = STD_SCREEN_Y + 162; break;
			case LAPTOP_PROGRAM_WEB_BROWSER: y = STD_SCREEN_Y +  99; break;
			default: goto no_display_min;
		}

		fMinizingProgram = !DisplayTitleBarMaximizeGraphic(FALSE, fInitTitle, STD_SCREEN_X + 29, y, STD_SCREEN_X + 29 + 20);
		if (!fMinizingProgram)
		{
			RemoveTitleBarMaximizeGraphics();
			EnterNewLaptopMode();
			fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
			fPausedReDrawScreenFlag = TRUE;
		}

no_display_min:;
	}
	else
	{
		return;
	}

	// reset init
	fInitTitle = FALSE;
}


static void ShowLights(void)
{
	// will show lights depending on state
	BltVideoObject(FRAME_BUFFER, guiLIGHTS, fPowerLightOn     ? 0 : 1, STD_SCREEN_X + 44, STD_SCREEN_Y + 466);
	BltVideoObject(FRAME_BUFFER, guiLIGHTS, fHardDriveLightOn ? 0 : 1, STD_SCREEN_X + 88, STD_SCREEN_Y + 466);
}


static void FlickerHDLight(void)
{
	static UINT32 uiBaseTime       = 0;
	static UINT32 uiTotalDifference = 0;

	if (fLoadPendingFlag) fFlickerHD = TRUE;
	if (!fFlickerHD) return;

	if (uiBaseTime == 0) uiBaseTime = GetJA2Clock();

	const UINT32 uiDifference = GetJA2Clock() - uiBaseTime;

	if (uiTotalDifference > HD_FLICKER_TIME && !fLoadPendingFlag)
	{
		uiBaseTime         = GetJA2Clock();
		fHardDriveLightOn = FALSE;
		uiBaseTime         = 0;
		uiTotalDifference  = 0;
		fFlickerHD        = FALSE;
		InvalidateRegion(88, 466, 102, 477);
		return;
	}

	if (uiDifference > FLICKER_TIME)
	{
		uiTotalDifference += uiDifference;

		if (fLoadPendingFlag) uiTotalDifference = 0;

		fHardDriveLightOn = (Random(2) == 0);
		InvalidateRegion(88, 466, 102, 477);
	}
}


static BOOLEAN ExitLaptopDone(void)
{
	// check if this is the first time, to reset counter
	static BOOLEAN fOldLeaveLaptopState = FALSE;
	static UINT32  uiBaseTime            = 0;

	if (!fOldLeaveLaptopState)
	{
		fOldLeaveLaptopState = TRUE;
		uiBaseTime = GetJA2Clock();
	}

	fPowerLightOn = FALSE;

	InvalidateRegion(44, 466, 58, 477);
	// get the current difference
	const UINT32 iDifference = GetJA2Clock() - uiBaseTime;

	// did we wait long enough?
	if (iDifference > EXIT_LAPTOP_DELAY_TIME)
	{
		uiBaseTime = 0;
		fOldLeaveLaptopState = FALSE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


static void CreateMinimizeButtonForCurrentMode(void);
static void CreateMinimizeRegionsForLaptopProgramIcons(void);
static void DestroyMinimizeButtonForCurrentMode(void);
static void DestroyMinimizeRegionsForLaptopProgramIcons(void);


static void CreateDestroyMinimizeButtonForCurrentMode(void)
{
	// will create the minimize button
	static BOOLEAN fAlreadyCreated = FALSE;
	// state of creation of minimize button
	BOOLEAN fCreateMinimizeButton = FALSE;
	// check to see if created, if so, do nothing

	// check current mode
	if (guiCurrentLaptopMode == LAPTOP_MODE_NONE && guiPreviousLaptopMode != LAPTOP_MODE_NONE)
	{
		fCreateMinimizeButton = FALSE;
	}
	else if (guiCurrentLaptopMode != LAPTOP_MODE_NONE)
	{
		fCreateMinimizeButton = TRUE;
	}
	else if (guiPreviousLaptopMode != LAPTOP_MODE_NONE)
	{
		fCreateMinimizeButton = FALSE;
	}

	// leaving laptop, get rid of the button
	if (fExitingLaptopFlag)
	{
		fCreateMinimizeButton = FALSE;
	}

	if (!fAlreadyCreated && fCreateMinimizeButton)
	{
		// not created, create
		fAlreadyCreated = TRUE;
		CreateMinimizeButtonForCurrentMode();
		CreateMinimizeRegionsForLaptopProgramIcons();
	}
	else if (fAlreadyCreated && !fCreateMinimizeButton)
	{
		// created and must be destroyed
		fAlreadyCreated = FALSE;
		DestroyMinimizeButtonForCurrentMode();
		DestroyMinimizeRegionsForLaptopProgramIcons();
	}
}


static void LaptopMinimizeProgramButtonCallback(GUI_BUTTON* btn, UINT32 reason);


static void CreateMinimizeButtonForCurrentMode(void)
{
	// create minimize button
	gLaptopMinButton = QuickCreateButtonImg(LAPTOPDIR "/x.sti", 0, 1, STD_SCREEN_X + 590, STD_SCREEN_Y + 30, MSYS_PRIORITY_HIGH, LaptopMinimizeProgramButtonCallback);
	gLaptopMinButton->SetCursor(CURSOR_LAPTOP_SCREEN);
}


static void DestroyMinimizeButtonForCurrentMode(void)
{
	// destroy minimize button
	RemoveButton(gLaptopMinButton);
}


static void SetCurrentToLastProgramOpened(void);


static void LaptopMinimizeProgramButtonCallback(GUI_BUTTON* btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT           prog;
		ST::string title;
		UINT16         gfx_idx;
		switch (guiCurrentLaptopMode)
		{
			case LAPTOP_MODE_EMAIL:
				prog    = LAPTOP_PROGRAM_MAILER;
				title   = pLaptopIcons[0];
				gfx_idx = 0;
				break;

			case LAPTOP_MODE_FILES:
				prog    = LAPTOP_PROGRAM_FILES;
				title   = pLaptopIcons[5];
				gfx_idx = 2;
				break;

			case LAPTOP_MODE_FINANCES:
				prog    = LAPTOP_PROGRAM_FINANCES;
				title   = pLaptopIcons[2];
				gfx_idx = 5;
				break;

			case LAPTOP_MODE_HISTORY:
				prog    = LAPTOP_PROGRAM_HISTORY;
				title   = pLaptopIcons[4];
				gfx_idx = 4;
				break;

			case LAPTOP_MODE_PERSONNEL:
				prog    = LAPTOP_PROGRAM_PERSONNEL;
				title   = pLaptopIcons[3];
				gfx_idx = 3;
				break;

			case LAPTOP_MODE_NONE: return; // nothing

			default:
				gfShowBookmarks = FALSE;
				prog    = LAPTOP_PROGRAM_WEB_BROWSER;
				title   = pLaptopIcons[7];
				gfx_idx = 1;
				break;
		}
		gLaptopProgramStates[prog] = LAPTOP_PROGRAM_MINIMIZED;
		InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, title, guiTITLEBARICONS, gfx_idx);
		SetCurrentToLastProgramOpened();
		fMinizingProgram = TRUE;
		fInitTitle = TRUE;
	}
}


static INT32 FindLastProgramStillOpen(void)
{
	INT32 iLowestValue = 6;
	INT32 iLowestValueProgram = 6;

	// returns ID of last program open and not minimized
	for (INT32 i = 0; i < 6; ++i)
	{
		if (gLaptopProgramStates[i] != LAPTOP_PROGRAM_MINIMIZED)
		{
			if (gLaptopProgramQueueList[i] < iLowestValue)
			{
				iLowestValue = gLaptopProgramQueueList[i];
				iLowestValueProgram = i;
			}
		}
	}

	return iLowestValueProgram;
}


static void UpdateListToReflectNewProgramOpened(INT32 iOpenedProgram)
{
	// will update queue of opened programs to show thier states
	// set iOpenedProgram to 1, and update others

	// increment everyone
	for (INT32 i = 0; i < 6; ++i)
	{
		gLaptopProgramQueueList[i]++;
	}

	gLaptopProgramQueueList[iOpenedProgram] = 1;
}


static void InitLaptopOpenQueue(void)
{
	// set evereyone to 1
	for (INT32 i = 0; i < 6; ++i)
	{
		gLaptopProgramQueueList[i] = 1;
	}
}


static void SetCurrentToLastProgramOpened(void)
{
	guiCurrentLaptopMode = LAPTOP_MODE_NONE;

	switch (FindLastProgramStillOpen())
	{
		case LAPTOP_PROGRAM_HISTORY:   guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;   break;
		case LAPTOP_PROGRAM_MAILER:    guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;     break;
		case LAPTOP_PROGRAM_PERSONNEL: guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL; break;
		case LAPTOP_PROGRAM_FINANCES:  guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;  break;
		case LAPTOP_PROGRAM_FILES:     guiCurrentLaptopMode = LAPTOP_MODE_FILES;     break;

		case LAPTOP_PROGRAM_WEB_BROWSER:
			// last www mode
			guiCurrentLaptopMode = (guiCurrentWWWMode == 0 ? LAPTOP_MODE_WWW : guiCurrentWWWMode);
			fShowBookmarkInfo = TRUE;
			break;
	}
}


void BlitTitleBarIcons(void)
{
	// will blit the icons for the title bar of the program we are in
	UINT32 Index;
	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_HISTORY:   Index = 4; break;
		case LAPTOP_MODE_EMAIL:     Index = 0; break;
		case LAPTOP_MODE_PERSONNEL: Index = 3; break;
		case LAPTOP_MODE_FINANCES:  Index = 5; break;
		case LAPTOP_MODE_FILES:     Index = 2; break;
		case LAPTOP_MODE_NONE:      return;           // do nothing
		default:                    Index = 1; break; // www pages
	}
	BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, Index, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
}


static void DrawDeskTopBackground(void)
{
	BltVideoSurface(FRAME_BUFFER, guiDESKTOP, LAPTOP_SCREEN_UL_X - 2, LAPTOP_SCREEN_UL_Y - 4, NULL);
}


static void LoadDesktopBackground(void)
{
	const char* const ImageFile = GetMLGFilename(MLG_DESKTOP);
	guiDESKTOP = AddVideoSurfaceFromFile(ImageFile);
}


static void DeleteDesktopBackground(void)
{
	DeleteVideoSurface(guiDESKTOP);
}


void PrintBalance(void)
{
	SetFontAttributes(FONT10ARIAL, FONT_BLACK, NO_SHADOW);

	ST::string pString = SPrintMoney(LaptopSaveInfo.iCurrentBalance);

	INT32 x = STD_SCREEN_X + 47;
	INT32 y = STD_SCREEN_Y + 257 + 15;
	if (gLaptopButton[5]->Clicked())
	{
		++x;
		++y;
	}
	MPrint(x, y, pString);

	SetFontShadow(DEFAULT_SHADOW);
}


void PrintNumberOnTeam(void)
{
	SetFontAttributes(FONT10ARIAL, FONT_BLACK, NO_SHADOW);

	const INT32 iCounter = NumberOfMercsOnPlayerTeam();

	UINT16 usPosX = STD_SCREEN_X + 47;
	UINT16 usPosY = STD_SCREEN_Y + 194 + 30;
	if (gLaptopButton[3]->Clicked())
	{
		++usPosX;
		++usPosY;
	}
	MPrint(usPosX, usPosY, ST::format("{} {}", pPersonnelString, iCounter));

	SetFontShadow(DEFAULT_SHADOW);
}


void PrintDate(void)
{
	SetFontAttributes(FONT10ARIAL, FONT_BLACK, NO_SHADOW);
	MPrint(STD_SCREEN_X + 30 + (70 - StringPixLength(WORLDTIMESTR, FONT10ARIAL)) / 2, (433 + STD_SCREEN_Y), WORLDTIMESTR);
	SetFontShadow(DEFAULT_SHADOW);
}


static void DisplayTaskBarIcons(void)
{
	// display the files icon, if there is any
	if (fNewFilesInFileViewer)
	{
		BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, 7, LAPTOP__NEW_FILE_ICON_X, LAPTOP__NEW_FILE_ICON_Y);
	}

	// display the email icon, if there is email
	if (fUnReadMailFlag)
	{
		BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, 6, LAPTOP__NEW_EMAIL_ICON_X, LAPTOP__NEW_EMAIL_ICON_Y);
	}
}


static void HandleAltTabKeyInLaptop(void);
static void HandleShiftAltTabKeyInLaptop(void);


void HandleKeyBoardShortCutsForLapTop(UINT16 usEvent, UINT32 usParam, UINT16 usKeyState)
{
	// will handle keyboard shortcuts for the laptop ... to be added to later
	if (fExitingLaptopFlag || fTabHandled) return;

	if (usEvent != KEY_DOWN) return;

	switch (usParam)
	{
		case SDLK_ESCAPE:
			// esc hit, check to see if boomark list is shown, if so, get rid of it, otherwise, leave
			HandleLapTopESCKey();
			break;

		case SDLK_TAB:
			if (usKeyState & CTRL_DOWN)
			{
				HandleShiftAltTabKeyInLaptop();
			}
			else
			{
				HandleAltTabKeyInLaptop();
			}
			fTabHandled = TRUE;
			break;

		case 'b':
			if (CHEATER_CHEAT_LEVEL())
			{
				if (usKeyState & ALT_DOWN)
				{
					LaptopSaveInfo.fBobbyRSiteCanBeAccessed = TRUE;
				}
				else if (usKeyState & CTRL_DOWN)
				{
					guiCurrentLaptopMode = LAPTOP_MODE_BROKEN_LINK;
				}
			}
			break;

		case 'x':
			if (usKeyState & ALT_DOWN) HandleShortCutExitState();
			break;

		case SDLK_h:
			ShouldTheHelpScreenComeUp(HELP_SCREEN_LAPTOP, TRUE);
			break;

		case '=':
		case '+':
			if (CHEATER_CHEAT_LEVEL())
			{
				// adding money
				AddTransactionToPlayersBook(ANONYMOUS_DEPOSIT, 0, GetWorldTotalMin(), 100000);
				MarkButtonsDirty();
			}
			break;

		case '-':
			if (CHEATER_CHEAT_LEVEL())
			{
				// subtracting money
				AddTransactionToPlayersBook(ANONYMOUS_DEPOSIT, 0, GetWorldTotalMin(), -10000);
				MarkButtonsDirty();
			}
			break;
	}
}


void RenderWWWProgramTitleBar(void)
{
	// will render the title bar for the www program
	BltVideoObject(FRAME_BUFFER, guiTITLEBARLAPTOP, 0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2);

	// now slapdown text
	SetFontAttributes(FONT14ARIAL, FONT_WHITE);

	// display title

	// no page loaded yet, do not handle yet

	if (guiCurrentLaptopMode == LAPTOP_MODE_WWW)
	{
		MPrint(STD_SCREEN_X + 140, STD_SCREEN_Y + 33, pWebTitle);
	}
	else
	{
		const INT32 iIndex = guiCurrentLaptopMode - LAPTOP_MODE_WWW-1;
		MPrint(STD_SCREEN_X + 140, STD_SCREEN_Y + 33, ST::format("{}  -  {}", pWebTitle, pWebPagesTitles[iIndex]));
	}

	BlitTitleBarIcons();
	DisplayProgramBoundingBox(FALSE);
}


static void LaptopProgramIconMinimizeCallback(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateMinimizeRegionsForLaptopProgramIcons(void)
{
	// will create the minizing region to lie over the icon for this particular laptop program
	const UINT16 x = LAPTOP_PROGRAM_ICON_X;
	const UINT16 y = LAPTOP_PROGRAM_ICON_Y;
	const UINT16 w = LAPTOP_PROGRAM_ICON_WIDTH;
	const UINT16 h = LAPTOP_PROGRAM_ICON_HEIGHT;
	MSYS_DefineRegion(&gLapTopProgramMinIcon, x, y, x + w, y + h, MSYS_PRIORITY_NORMAL + 1, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LaptopProgramIconMinimizeCallback);
}


static void DestroyMinimizeRegionsForLaptopProgramIcons(void)
{
	// will destroy the minizmize regions to be placed over the laptop icons that will be
	// displayed on the top of the laptop program bar
	MSYS_RemoveRegion(&gLapTopProgramMinIcon);
}


static void LaptopProgramIconMinimizeCallback(MOUSE_REGION* pRegion, UINT32 iReason)
{
	// callback handler for the minize region that is attatched to the laptop program icon
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		switch (guiCurrentLaptopMode)
		{
			case LAPTOP_MODE_EMAIL:
				gLaptopProgramStates[LAPTOP_PROGRAM_MAILER] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[0], guiTITLEBARICONS, 0);
				SetCurrentToLastProgramOpened();
				break;

			case LAPTOP_MODE_FILES:
				gLaptopProgramStates[LAPTOP_PROGRAM_FILES] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[5], guiTITLEBARICONS, 2);
				SetCurrentToLastProgramOpened();
				break;

			case LAPTOP_MODE_FINANCES:
				gLaptopProgramStates[LAPTOP_PROGRAM_FINANCES] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[2], guiTITLEBARICONS, 5);
				SetCurrentToLastProgramOpened();
				break;

			case LAPTOP_MODE_HISTORY:
				gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[4], guiTITLEBARICONS, 4);
				SetCurrentToLastProgramOpened();
				break;

			case LAPTOP_MODE_PERSONNEL:
				gLaptopProgramStates[LAPTOP_PROGRAM_PERSONNEL] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[3], guiTITLEBARICONS, 3);
				SetCurrentToLastProgramOpened();
				break;

			case LAPTOP_MODE_NONE: return; // nothing

			default:
				gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pWebTitle, guiTITLEBARICONS, 1);
				SetCurrentToLastProgramOpened();
				gfShowBookmarks = FALSE;
				fShowBookmarkInfo = FALSE;
				break;
		}
		fMinizingProgram = TRUE;
		fInitTitle = TRUE;
	}
}


void DisplayProgramBoundingBox(BOOLEAN fMarkButtons)
{
	// the border for the program
	BltVideoObject(FRAME_BUFFER, guiLaptopBACKGROUND, 1, STD_SCREEN_X + 25, STD_SCREEN_Y + 23);

	// no laptop mode, no border around the program
	if (guiCurrentLaptopMode != LAPTOP_MODE_NONE)
	{
		BltVideoObject(FRAME_BUFFER, guiLaptopBACKGROUND, 0, STD_SCREEN_X + 108, STD_SCREEN_Y + 23);
	}

	if (fMarkButtons || fLoadPendingFlag)
	{
		MarkButtonsDirty();
		RenderButtons();
	}

	PrintDate();
	PrintBalance();
	PrintNumberOnTeam();
	DisplayTaskBarIcons();
}


static void NewEmailIconCallback(MOUSE_REGION* pRegion, UINT32 iReason);
static void NewFileIconCallback(MOUSE_REGION* pRegion, UINT32 iReason);


static void CreateDestroyMouseRegionForNewMailIcon(void)
{
	static BOOLEAN fCreated = FALSE;

	// will toggle creation/destruction of the mouse regions used by the new mail icon
	if (!fCreated)
	{
		fCreated = TRUE;
		{
			const UINT16 x = LAPTOP__NEW_EMAIL_ICON_X;
			const UINT16 y = LAPTOP__NEW_EMAIL_ICON_Y;
			MSYS_DefineRegion(&gNewMailIconRegion, x, y + 5,  x + 16,  y + 16, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, NewEmailIconCallback);
			CreateFileAndNewEmailIconFastHelpText(LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_MAIL, fUnReadMailFlag == 0);
		}

		{
			const UINT16 x = LAPTOP__NEW_FILE_ICON_X;
			const UINT16 y = LAPTOP__NEW_FILE_ICON_Y;
			MSYS_DefineRegion(&gNewFileIconRegion, x, y + 5,  x + 16,  y + 16, MSYS_PRIORITY_HIGHEST - 3, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, NewFileIconCallback);
			CreateFileAndNewEmailIconFastHelpText(LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE, fNewFilesInFileViewer == 0);
		}
	}
	else
	{
		fCreated = FALSE;
		MSYS_RemoveRegion(&gNewMailIconRegion);
		MSYS_RemoveRegion(&gNewFileIconRegion);
	}
}


static void NewEmailIconCallback(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (fUnReadMailFlag)
		{
			fOpenMostRecentUnReadFlag = TRUE;
			guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		}
	}
}


static void NewFileIconCallback(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (fNewFilesInFileViewer)
		{
			fEnteredFileViewerFromNewFileIcon = TRUE;
			guiCurrentLaptopMode = LAPTOP_MODE_FILES;
		}
	}
}


static void HandleWWWSubSites(void)
{
	// check to see if WW Wait is needed for a sub site within the Web Browser
	if (guiCurrentLaptopMode == guiPreviousLaptopMode ||
			guiCurrentLaptopMode < LAPTOP_MODE_WWW ||
			fLoadPendingFlag ||
			fDoneLoadPending ||
			guiPreviousLaptopMode < LAPTOP_MODE_WWW)
	{
		// no go, leave
		return;
	}

	fLoadPendingFlag = TRUE;
	fConnectingToSubPage = TRUE;

	// fast or slow load?
	bool& visited = gfWWWaitSubSitesVisitedFlags[guiCurrentLaptopMode - (LAPTOP_MODE_WWW + 1)];
	if (visited) fFastLoadFlag = TRUE;
	visited = true; // Set fact we were here

	//Dont show the dlownload screen when switching between these pages
	if ((guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS              && guiPreviousLaptopMode == LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX) ||
			(guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX && guiPreviousLaptopMode == LAPTOP_MODE_AIM_MEMBERS))
	{
		fFastLoadFlag = FALSE;
		fLoadPendingFlag = FALSE;

		// set fact we were here
		gfWWWaitSubSitesVisitedFlags[LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX - (LAPTOP_MODE_WWW + 1)] = true;
		gfWWWaitSubSitesVisitedFlags[LAPTOP_MODE_AIM_MEMBERS              - (LAPTOP_MODE_WWW + 1)] = true;
	}
}


static void UpdateStatusOfDisplayingBookMarks(void)
{
	// this function will disable showing of bookmarks if in process of download or if we miniming web browser
	if (fLoadPendingFlag || guiCurrentLaptopMode < LAPTOP_MODE_WWW)
	{
		gfShowBookmarks = FALSE;
	}
}


static void InitalizeSubSitesList(void)
{
	// init all subsites list to not visited
	FOR_EACH(bool, i, gfWWWaitSubSitesVisitedFlags)
	{
		*i = false;
	}
}


static void SetSubSiteAsVisted(void)
{
	// sets a www sub site as visited
	// not at a web page yet?
	if (guiCurrentLaptopMode <= LAPTOP_MODE_WWW) return;

	gfWWWaitSubSitesVisitedFlags[guiCurrentLaptopMode - (LAPTOP_MODE_WWW + 1)] = true;
}


static void HandleShiftAltTabKeyInLaptop(void)
{
	// will handle the alt tab keying in laptop

	// move to next program
	if (fMaximizingProgram) return;

	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_FINANCES:  guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL; break;
		case LAPTOP_MODE_PERSONNEL: guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;   break;
		case LAPTOP_MODE_HISTORY:   guiCurrentLaptopMode = LAPTOP_MODE_FILES;     break;
		case LAPTOP_MODE_EMAIL:     guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;  break;
		case LAPTOP_MODE_FILES:     guiCurrentLaptopMode = LAPTOP_MODE_WWW;       break;
		case LAPTOP_MODE_NONE:      guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;  break;
		case LAPTOP_MODE_WWW:       guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;     break;
		default:                    guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;     break;
	}

	fPausedReDrawScreenFlag = TRUE;
}


static void HandleAltTabKeyInLaptop(void)
{
	// will handle the alt tab keying in laptop

	// move to next program
	if (fMaximizingProgram) return;

	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_FINANCES:  guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;     break;
		case LAPTOP_MODE_PERSONNEL: guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;  break;
		case LAPTOP_MODE_HISTORY:   guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL; break;
		case LAPTOP_MODE_EMAIL:     guiCurrentLaptopMode = LAPTOP_MODE_WWW;       break;
		case LAPTOP_MODE_FILES:     guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;   break;
		case LAPTOP_MODE_NONE:      guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;     break;
		default:                    guiCurrentLaptopMode = LAPTOP_MODE_FILES;     break;
	}

	fPausedReDrawScreenFlag = TRUE;
}


static void HandleWebBookMarkNotifyTimer(void);


// display the 2 second book mark instruction
static void DisplayWebBookMarkNotify(void)
{
	static BOOLEAN fOldShow = FALSE;

	// handle the timer for this thing
	HandleWebBookMarkNotifyTimer();

	// are we about to start showing box? or did we just stop?
	if ((!fOldShow || fReDrawBookMarkInfo) && fShowBookmarkInfo)
	{
		fOldShow = TRUE;
		fReDrawBookMarkInfo = FALSE;

		// show background objects
		BltVideoObject(FRAME_BUFFER, guiDOWNLOADTOP,   0,DOWNLOAD_X,     DOWNLOAD_Y);
		BltVideoObject(FRAME_BUFFER, guiDOWNLOADMID,   0,DOWNLOAD_X,     DOWNLOAD_Y +     DOWN_HEIGHT);
		BltVideoObject(FRAME_BUFFER, guiDOWNLOADBOT,   0,DOWNLOAD_X,     DOWNLOAD_Y + 2 * DOWN_HEIGHT);
		BltVideoObject(FRAME_BUFFER, guiTITLEBARICONS, 1,DOWNLOAD_X + 4, DOWNLOAD_Y + 1);

		SetFontAttributes(DOWNLOAD_FONT, FONT_WHITE, NO_SHADOW);

		// display download string
		MPrint(DOWN_STRING_X, DOWN_STRING_Y, pShowBookmarkString[0]);

		// now draw the message
		DisplayWrappedString(DOWN_STRING_X - 42, DOWN_STRING_Y + 20, BOOK_WIDTH + 45, 2, BOOK_FONT, FONT_BLACK, pShowBookmarkString[1], FONT_BLACK, CENTER_JUSTIFIED);

		// invalidate region
		InvalidateRegion(DOWNLOAD_X, DOWNLOAD_Y, DOWNLOAD_X + 150, DOWNLOAD_Y + 100);

		SetFontShadow(DEFAULT_SHADOW);
	}
	else if (fOldShow && !fShowBookmarkInfo)
	{
		fOldShow = FALSE;
		fPausedReDrawScreenFlag = TRUE;
	}
}


// handle timer for bookmark notify
static void HandleWebBookMarkNotifyTimer(void)
{
	static UINT32  uiBaseTime            = 0;
	static BOOLEAN fOldShowBookMarkInfo = FALSE;

	// check if maxing or mining?
	if (fMaximizingProgram || fMinizingProgram)
	{
		fOldShowBookMarkInfo |= fShowBookmarkInfo;
		fShowBookmarkInfo     = FALSE;
		return;
	}

	// if we were going to show this pop up, but were delayed, then do so now
	fShowBookmarkInfo |= fOldShowBookMarkInfo;

	// reset old flag
	fOldShowBookMarkInfo = FALSE;

	// if current mode is too low, then reset
	if (guiCurrentLaptopMode < LAPTOP_MODE_WWW)
	{
		fShowBookmarkInfo = FALSE;
	}

	// if showing bookmarks, don't show help
	if (gfShowBookmarks) fShowBookmarkInfo = FALSE;

	// check if flag false, is so, leave
	if (!fShowBookmarkInfo)
	{
		uiBaseTime = 0;
		return;
	}

	// check if this is the first time in here
	if (uiBaseTime == 0)
	{
		uiBaseTime = GetJA2Clock();
		return;
	}

	const UINT32 uiDifference = GetJA2Clock() - uiBaseTime;

	fReDrawBookMarkInfo = TRUE;

	if (uiDifference > DISPLAY_TIME_FOR_WEB_BOOKMARK_NOTIFY)
	{
		// waited long enough, stop showing
		uiBaseTime = 0;
		fShowBookmarkInfo = FALSE;
	}
}


void ClearOutTempLaptopFiles(void)
{
	// clear out all temp files from laptop
	GCM->tempFiles()->deleteFile(FILES_DATA_FILE);
	GCM->tempFiles()->deleteFile(FINANCES_DATA_FILE);
	GCM->tempFiles()->deleteFile(HISTORY_DATA_FILE);
}


static void InjectStoreInvetory(DataWriter& d, STORE_INVENTORY const& i)
{
	size_t start = d.getConsumed();
	INJ_U16( d, i.usItemIndex)
	INJ_U8(  d, i.ubQtyOnHand)
	INJ_U8(  d, i.ubQtyOnOrder)
	INJ_U8(  d, i.ubItemQuality)
	INJ_BOOL(d, i.fPreviouslyEligible)
	INJ_SKIP(d, 2)
	Assert(d.getConsumed() == start + 8);
}


static void ExtractStoreInvetory(DataReader& d, STORE_INVENTORY& i)
{
	size_t start = d.getConsumed();
	EXTR_U16( d, i.usItemIndex)
	EXTR_U8(  d, i.ubQtyOnHand)
	EXTR_U8(  d, i.ubQtyOnOrder)
	EXTR_U8(  d, i.ubItemQuality)
	EXTR_BOOL(d, i.fPreviouslyEligible)
	EXTR_SKIP(d, 2)
	Assert(d.getConsumed() == start + 8);
}


void SaveLaptopInfoToSavedGame(HWFILE const f)
{
	LaptopSaveInfoStruct const& l = LaptopSaveInfo;

	BYTE  data[7440];
	DataWriter d{data};
	INJ_BOOL( d, l.gfNewGameLaptop)
	INJ_BOOLA(d, l.fVisitedBookmarkAlready, lengthof(l.fVisitedBookmarkAlready))
	INJ_SKIP( d, 3)
	INJ_I32A( d, l.iBookMarkList, lengthof(l.iBookMarkList))
	INJ_I32(  d, l.iCurrentBalance)
	INJ_BOOL( d, l.fIMPCompletedFlag)
	INJ_BOOL( d, l.fSentImpWarningAlready)
	INJ_I16A( d, l.ubDeadCharactersList, lengthof(l.ubDeadCharactersList))
	INJ_I16A( d, l.ubLeftCharactersList, lengthof(l.ubLeftCharactersList))
	INJ_I16A( d, l.ubOtherCharactersList, lengthof(l.ubOtherCharactersList))
	INJ_U8(   d, l.gubPlayersMercAccountStatus)
	INJ_SKIP( d, 1)
	INJ_U32(  d, l.guiPlayersMercAccountNumber)
	INJ_U8(   d, l.gubLastMercIndex)
	INJ_SKIP( d, 1)
	FOR_EACH(STORE_INVENTORY const, i, l.BobbyRayInventory)
	{
		InjectStoreInvetory(d, *i);
	}
	FOR_EACH(STORE_INVENTORY const, i, l.BobbyRayUsedInventory)
	{
		InjectStoreInvetory(d, *i);
	}
	INJ_SKIP( d, 6)
	Assert(l.BobbyRayOrdersOnDeliveryArray.size() <= UINT8_MAX);
	INJ_U8(   d, static_cast<UINT8>(l.BobbyRayOrdersOnDeliveryArray.size()))
	INJ_U8(   d, l.usNumberOfBobbyRayOrderUsed)
	INJ_SKIP( d, 6)
	Assert(l.pLifeInsurancePayouts.size() <= UINT8_MAX);
	INJ_U8(   d, static_cast<UINT8>(l.pLifeInsurancePayouts.size()))
	Assert(l.ubNumberLifeInsurancePayoutUsed <= l.pLifeInsurancePayouts.size());
	INJ_U8(   d, l.ubNumberLifeInsurancePayoutUsed)
	INJ_BOOL( d, l.fBobbyRSiteCanBeAccessed)
	INJ_U8(   d, l.ubPlayerBeenToMercSiteStatus)
	INJ_BOOL( d, l.fFirstVisitSinceServerWentDown)
	INJ_BOOL( d, l.fNewMercsAvailableAtMercSite)
	INJ_BOOL( d, l.fSaidGenericOpeningInMercSite)
	INJ_BOOL( d, l.fSpeckSaidFloMarriedCousinQuote)
	INJ_BOOL( d, l.fHasAMercDiedAtMercSite)
	INJ_SKIP( d, 11)
	INJ_U16A( d, l.usInventoryListLength, lengthof(l.usInventoryListLength))
	INJ_I32(  d, l.iVoiceId)
	INJ_U8(   d, l.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction)
	INJ_BOOL( d, l.fMercSiteHasGoneDownYet)
	INJ_U8(   d, l.ubSpeckCanSayPlayersLostQuote)
	INJ_SKIP( d, 1)
	INJ_BOOL( d, l.sLastHiredMerc.fHaveDisplayedPopUpInLaptop)
	INJ_SKIP( d, 3)
	INJ_I32(  d, l.sLastHiredMerc.iIdOfMerc)
	INJ_U32(  d, l.sLastHiredMerc.uiArrivalTime)
	INJ_I32(  d, l.iCurrentHistoryPage)
	INJ_I32(  d, l.iCurrentFinancesPage)
	INJ_I32(  d, l.iCurrentEmailPage)
	INJ_U32(  d, l.uiSpeckQuoteFlags)
	INJ_U32(  d, l.uiFlowerOrderNumber)
	INJ_U32(  d, l.uiTotalMoneyPaidToSpeck)
	INJ_U8(   d, l.ubLastMercAvailableId)
	INJ_SKIP( d, 87)
	Assert(d.getConsumed() == lengthof(data));

	f->write(data, sizeof(data));

	if (l.usNumberOfBobbyRayOrderUsed != 0)
	{ // There is anything in the Bobby Ray Orders on delivery
		size_t const size = sizeof(*l.BobbyRayOrdersOnDeliveryArray.data()) * l.BobbyRayOrdersOnDeliveryArray.size();
		f->write(l.BobbyRayOrdersOnDeliveryArray.data(), size);
	}

	if (l.ubNumberLifeInsurancePayoutUsed != 0)
	{ // There are any insurance payouts in progress
		f->write(l.pLifeInsurancePayouts.data(), sizeof(LIFE_INSURANCE_PAYOUT) * l.pLifeInsurancePayouts.size());
	}
}


void LoadLaptopInfoFromSavedGame(HWFILE const f)
{
	LaptopSaveInfoStruct& l = LaptopSaveInfo;

	l.BobbyRayOrdersOnDeliveryArray.clear();

	l.pLifeInsurancePayouts.clear();

	BYTE data[7440];
	f->read(data, sizeof(data));

	DataReader d{data};
	EXTR_BOOL( d, l.gfNewGameLaptop)
	EXTR_BOOLA(d, l.fVisitedBookmarkAlready, lengthof(l.fVisitedBookmarkAlready))
	EXTR_SKIP( d, 3)
	EXTR_I32A( d, l.iBookMarkList, lengthof(l.iBookMarkList))
	EXTR_I32(  d, l.iCurrentBalance)
	EXTR_BOOL( d, l.fIMPCompletedFlag)
	EXTR_BOOL( d, l.fSentImpWarningAlready)
	EXTR_I16A( d, l.ubDeadCharactersList, lengthof(l.ubDeadCharactersList))
	EXTR_I16A( d, l.ubLeftCharactersList, lengthof(l.ubLeftCharactersList))
	EXTR_I16A( d, l.ubOtherCharactersList, lengthof(l.ubOtherCharactersList))
	EXTR_U8(   d, l.gubPlayersMercAccountStatus)
	EXTR_SKIP( d, 1)
	EXTR_U32(  d, l.guiPlayersMercAccountNumber)
	EXTR_U8(   d, l.gubLastMercIndex)
	EXTR_SKIP( d, 1)
	FOR_EACH(STORE_INVENTORY, i, l.BobbyRayInventory)
	{
		ExtractStoreInvetory(d, *i);
	}
	FOR_EACH(STORE_INVENTORY, i, l.BobbyRayUsedInventory)
	{
		ExtractStoreInvetory(d, *i);
	}
	EXTR_SKIP( d, 6)
	UINT8 BobbyRayOrdersOnDeliveryArraySize;
	EXTR_U8(   d, BobbyRayOrdersOnDeliveryArraySize)
	EXTR_U8(   d, l.usNumberOfBobbyRayOrderUsed)
	EXTR_SKIP( d, 6)
	UINT8 numLifeInsurancePayouts;
	EXTR_U8(   d, numLifeInsurancePayouts)
	EXTR_U8(   d, l.ubNumberLifeInsurancePayoutUsed)
	Assert(l.ubNumberLifeInsurancePayoutUsed <= numLifeInsurancePayouts);
	EXTR_BOOL( d, l.fBobbyRSiteCanBeAccessed)
	EXTR_U8(   d, l.ubPlayerBeenToMercSiteStatus)
	EXTR_BOOL( d, l.fFirstVisitSinceServerWentDown)
	EXTR_BOOL( d, l.fNewMercsAvailableAtMercSite)
	EXTR_BOOL( d, l.fSaidGenericOpeningInMercSite)
	EXTR_BOOL( d, l.fSpeckSaidFloMarriedCousinQuote)
	EXTR_BOOL( d, l.fHasAMercDiedAtMercSite)
	EXTR_SKIP( d, 11)
	EXTR_U16A( d, l.usInventoryListLength, lengthof(l.usInventoryListLength))
	EXTR_I32(  d, l.iVoiceId)
	EXTR_U8(   d, l.ubHaveBeenToBobbyRaysAtLeastOnceWhileUnderConstruction)
	EXTR_BOOL( d, l.fMercSiteHasGoneDownYet)
	EXTR_U8(   d, l.ubSpeckCanSayPlayersLostQuote)
	EXTR_SKIP( d, 1)
	EXTR_BOOL( d, l.sLastHiredMerc.fHaveDisplayedPopUpInLaptop)
	EXTR_SKIP( d, 3)
	EXTR_I32(  d, l.sLastHiredMerc.iIdOfMerc)
	EXTR_U32(  d, l.sLastHiredMerc.uiArrivalTime)
	EXTR_I32(  d, l.iCurrentHistoryPage)
	EXTR_I32(  d, l.iCurrentFinancesPage)
	EXTR_I32(  d, l.iCurrentEmailPage)
	EXTR_U32(  d, l.uiSpeckQuoteFlags)
	EXTR_U32(  d, l.uiFlowerOrderNumber)
	EXTR_U32(  d, l.uiTotalMoneyPaidToSpeck)
	EXTR_U8(   d, l.ubLastMercAvailableId)
	EXTR_SKIP( d, 87)
	Assert(d.getConsumed() == lengthof(data));

	// Handle old saves in M.E.R.C. module
	SyncLastMercFromSaveGame();

	if (l.usNumberOfBobbyRayOrderUsed != 0)
	{ // There is anything in the Bobby Ray Orders on Delivery
		l.BobbyRayOrdersOnDeliveryArray.resize(BobbyRayOrdersOnDeliveryArraySize);
		size_t const size = sizeof(*l.BobbyRayOrdersOnDeliveryArray.data()) * BobbyRayOrdersOnDeliveryArraySize;
		f->read(l.BobbyRayOrdersOnDeliveryArray.data(), size);
	}
	else
	{
		l.BobbyRayOrdersOnDeliveryArray.clear();
	}

	if (l.ubNumberLifeInsurancePayoutUsed != 0)
	{ // There are any Insurance Payouts in progress
		l.pLifeInsurancePayouts.assign(numLifeInsurancePayouts, LIFE_INSURANCE_PAYOUT{});
		f->read(l.pLifeInsurancePayouts.data(), sizeof(LIFE_INSURANCE_PAYOUT) * numLifeInsurancePayouts);
	}
	else
	{
		l.pLifeInsurancePayouts.clear();
	}
}

// Used to determine delay if its raining
static BOOLEAN IsItRaining(void)
{
	return
		guiEnvWeather & WEATHER_FORECAST_SHOWERS ||
		guiEnvWeather & WEATHER_FORECAST_THUNDERSHOWERS;
}


static void InternetRainDelayMessageBoxCallBack(MessageBoxReturnValue const bExitValue)
{
	GoToWebPage(giRainDelayInternetSite);

	//Set to -2 so we dont due the message for this occurence of laptop
	giRainDelayInternetSite = -2;
}


void CreateFileAndNewEmailIconFastHelpText(UINT32 uiHelpTextID, BOOLEAN fClearHelpText)
{
	MOUSE_REGION* pRegion;
	switch (uiHelpTextID)
	{
		case LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_MAIL: pRegion = &gNewMailIconRegion; break;
		case LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE: pRegion = &gNewFileIconRegion; break;

		default:
			SLOGA("CreateFileAndNewEmailIconFastHelpText: invalid HelpTextID");
			return;
	}

	ST::string help = (fClearHelpText ? ST::string() : gzLaptopHelpText[uiHelpTextID]);
	pRegion->SetFastHelpText(help);
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Laptop, asserts)
{
	EXPECT_EQ(sizeof(LIFE_INSURANCE_PAYOUT), 8u);
	EXPECT_EQ(sizeof(BobbyRayPurchaseStruct), 8u);
	EXPECT_EQ(sizeof(BobbyRayOrderStruct), 84u);
}

#endif
