#include "Local.h"
#include "SGP.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "Screens.h"
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
#include "Florist.h"
#include "Florist_Cards.h"
#include "Florist_Gallery.h"
#include "Florist_Order_Form.h"
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
#include "VObject_Blitters.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "Overhead.h"
#include "Environment.h"
#include "Music_Control.h"
#include "SaveLoadGame.h"
#include "LaptopSave.h"
#include "RenderWorld.h"
#include "Gameloop.h"
#include "English.h"
#include "Random.h"
#include "Merc_Hiring.h"
#include "Map_Screen_Interface.h"
#include "Ambient_Control.h"
#include "Sound_Control.h"
#include "Text.h"
#include "Message.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Cursor_Control.h"
#include "Quests.h"
#include "Multi_Language_Graphic_Utils.h"
#include "BrokenLink.h"
#include "BobbyRShipments.h"
#include "Dialogue_Control.h"
#include "HelpScreen.h"
#include "Cheats.h"
#include "Video.h"
#include "MemMan.h"
#include "Debug.h"
#include "Button_System.h"
#include "JAScreens.h"
#include "Stubs.h" // XXX

#ifdef JA2TESTVERSION
#	include "Arms_Dealer_Init.h"
#	include "Strategic_Status.h"
#endif


// laptop programs
enum{
	LAPTOP_PROGRAM_MAILER,
	LAPTOP_PROGRAM_WEB_BROWSER,
	LAPTOP_PROGRAM_FILES,
	LAPTOP_PROGRAM_PERSONNEL,
	LAPTOP_PROGRAM_FINANCES,
	LAPTOP_PROGRAM_HISTORY,

};

// laptop program states
enum{
	LAPTOP_PROGRAM_MINIMIZED,
	LAPTOP_PROGRAM_OPEN,
};
#define BOOK_FONT FONT10ARIAL
#define DOWNLOAD_FONT FONT12ARIAL


#define BOOK_X 111
#define BOOK_TOP_Y 79
#define BOOK_HEIGHT 12
#define DOWN_HEIGHT 19
#define BOOK_WIDTH 100


#define LONG_UNIT_TIME 120
#define UNIT_TIME 40
#define FAST_UNIT_TIME 3
#define FASTEST_UNIT_TIME 2
#define ALMOST_FAST_UNIT_TIME 25
#define DOWNLOAD_X 300
#define DOWNLOAD_Y 200
#define LAPTOP_WINDOW_X DOWNLOAD_X+12
#define LAPTOP_WINDOW_Y DOWNLOAD_Y+25
#define LAPTOP_BAR_Y LAPTOP_WINDOW_Y+2
#define LAPTOP_BAR_X LAPTOP_WINDOW_X+1
#define UNIT_WIDTH 4
#define DOWN_STRING_X DOWNLOAD_X+47
#define DOWN_STRING_Y DOWNLOAD_Y+ 5
#define LAPTOP_TITLE_ICONS_X 113
#define LAPTOP_TITLE_ICONS_Y 27

// HD flicker times
#define HD_FLICKER_TIME 3000
#define FLICKER_TIME 50


#define		NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS			18
#define		LAPTOP_TITLE_BAR_WIDTH										500
#define		LAPTOP_TITLE_BAR_HEIGHT										24

#define		LAPTOP_TITLE_BAR_TOP_LEFT_X							  111
#define		LAPTOP_TITLE_BAR_TOP_LEFT_Y								25
#define		LAPTOP_TITLE_BAR_TOP_RIGHT_X							610

#define		LAPTOP_TITLE_BAR_ICON_OFFSET_X						5
#define		LAPTOP_TITLE_BAR_ICON_OFFSET_Y						2
#define		LAPTOP_TITLE_BAR_TEXT_OFFSET_X						29//18
#define		LAPTOP_TITLE_BAR_TEXT_OFFSET_Y						8

#define  LAPTOP_PROGRAM_ICON_X       LAPTOP_TITLE_BAR_TOP_LEFT_X
#define  LAPTOP_PROGRAM_ICON_Y       LAPTOP_TITLE_BAR_TOP_LEFT_Y
#define  LAPTOP_PROGRAM_ICON_WIDTH   20
#define  LAPTOP_PROGRAM_ICON_HEIGHT  20
#define DISPLAY_TIME_FOR_WEB_BOOKMARK_NOTIFY 2000

// the wait time for closing of laptop animation/delay
#define EXIT_LAPTOP_DELAY_TIME 100

static UINT32  guiTitleBarSurface;
static BOOLEAN gfTitleBarSurfaceAlreadyActive = FALSE;

#define		LAPTOP__NEW_FILE_ICON_X			83
#define		LAPTOP__NEW_FILE_ICON_Y			412//(405+19)

#define		LAPTOP__NEW_EMAIL_ICON_X		( 83 - 16 )
#define		LAPTOP__NEW_EMAIL_ICON_Y		LAPTOP__NEW_FILE_ICON_Y





// Mode values
UINT32 guiCurrentLaptopMode;
UINT32 guiPreviousLaptopMode;
static UINT32 guiCurrentWWWMode = LAPTOP_MODE_NONE;
INT32	 giCurrentSubPage;

extern UINT32 guiVObjectSize;
extern UINT32 guiVSurfaceSize;


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

// show start button for ATM panel?
extern BOOLEAN fShowAtmPanelStartButton;


//TEMP!	Disables the loadpending delay when switching b/n www pages
static BOOLEAN gfTemporaryDisablingOfLoadPendingFlag = FALSE;


//GLOBAL FOR WHICH SCREEN TO EXIT TO FOR LAPTOP
UINT32		guiExitScreen = MAP_SCREEN;

// Laptop screen graphic handle
static UINT32 guiLAPTOP;

static BOOLEAN fNewWWW = TRUE;

//Used to store the site to go to after the 'rain delay' message
extern	UINT32										guiRainLoop;


static INT32 giRainDelayInternetSite = -1;


// the laptop icons
static UINT32 guiBOOKHIGH;
static UINT32 guiBOOKMARK;
static UINT32 guiGRAPHWINDOW;
static UINT32 guiGRAPHBAR;
UINT32 guiLaptopBACKGROUND;
static UINT32 guiDOWNLOADTOP;
static UINT32 guiDOWNLOADMID;
static UINT32 guiDOWNLOADBOT;
static UINT32 guiTITLEBARLAPTOP;
static UINT32 guiLIGHTS;
UINT32 guiTITLEBARICONS;
static UINT32 guiDESKTOP;

// enter new laptop mode due to sliding bars
static BOOLEAN fEnteredNewLapTopDueToHandleSlidingBars = FALSE;


// whether or not we are initing the slide in title bar
static BOOLEAN fInitTitle = TRUE;

// tab handled
static BOOLEAN fTabHandled = FALSE;

static INT32 gLaptopButton[7];
static INT32 gLaptopButtonImage[7];

// minimize button
static INT32 gLaptopMinButton[1];
static INT32 gLaptopMinButtonImage[1];


static INT32 gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY + 1];

// process of mazimizing
static BOOLEAN fMaximizingProgram = FALSE;

// program we are maximizing
static INT8 bProgramBeingMaximized = -1;

// are we minimizing
static BOOLEAN fMinizingProgram = FALSE;


// process openned queue
static INT32 gLaptopProgramQueueList[6];


// state of createion of minimize button
static BOOLEAN fCreateMinimizeButton = FALSE;

BOOLEAN fExitingLaptopFlag = FALSE;

// HD and power lights on
static BOOLEAN fPowerLightOn = TRUE;
static BOOLEAN fHardDriveLightOn = FALSE;

// HD flicker
static BOOLEAN fFlickerHD = FALSE;

// the screens limiting rect
static const SGPRect LaptopScreenRect = { LAPTOP_UL_X, LAPTOP_UL_Y - 5, LAPTOP_SCREEN_LR_X + 2, LAPTOP_SCREEN_LR_Y + 5 + 19};


// the sub pages vistsed or not status within the web browser
static BOOLEAN gfWWWaitSubSitesVisitedFlags[LAPTOP_MODE_FUNERAL - LAPTOP_MODE_WWW];

// mouse regions
static MOUSE_REGION gLapTopScreenRegion;
static MOUSE_REGION gBookmarkMouseRegions[MAX_BOOKMARKS];
static MOUSE_REGION gLapTopProgramMinIcon;
static MOUSE_REGION gNewMailIconRegion;
static MOUSE_REGION gNewFileIconRegion;


//used for global variables that need to be saved
LaptopSaveInfoStruct LaptopSaveInfo;


BOOLEAN fReDrawScreenFlag=FALSE;
BOOLEAN fPausedReDrawScreenFlag=FALSE;		//used in the handler functions to redraw the screen, after the current frame
void PrintBalance( void );


void PrintDate( void );
void PrintNumberOnTeam( void );

extern void ClearHistoryList( void );


//TEMP CHEAT
#ifdef JA2TESTVERSION
extern	void CheatToGetAll5Merc();
#endif
#if defined ( JA2TESTVERSION ) || defined ( JA2DEMO )
extern	void DemoHiringOfMercs( );
#endif



void	SetLaptopExitScreen( UINT32 uiExitScreen )
{
	guiExitScreen = uiExitScreen;
}

void	SetLaptopNewGameFlag( )
{
	LaptopSaveInfo.gfNewGameLaptop = TRUE;
}


static void GetLaptopKeyboardInput(void)
{
  InputAtom					InputEvent;
	POINT  MousePos;

	GetCursorPos(&MousePos);

	fTabHandled = FALSE;

  while (DequeueEvent(&InputEvent) == TRUE)
  {
		// HOOK INTO MOUSE HOOKS
		switch(InputEvent.usEvent)
	  {
			case LEFT_BUTTON_DOWN:
			case LEFT_BUTTON_UP:
			case RIGHT_BUTTON_DOWN:
			case RIGHT_BUTTON_UP:
			case RIGHT_BUTTON_REPEAT:
			case LEFT_BUTTON_REPEAT:
				MouseSystemHook(InputEvent.usEvent, MousePos.x, MousePos.y, _LeftButtonDown, _RightButtonDown);
				break;
	  }

		HandleKeyBoardShortCutsForLapTop( InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState );

	}
}


static void InitBookMarkList(void);


//This is called only once at game initialization.
UINT32 LaptopScreenInit()
{
	//Memset the whole structure, to make sure of no 'JUNK'
	memset( &LaptopSaveInfo, 0, sizeof( LaptopSaveInfoStruct ) );

	LaptopSaveInfo.gfNewGameLaptop = TRUE;

	InitializeNumDaysMercArrive();

	//reset the id of the last hired merc
	LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;

	//reset the flag that enables the 'just hired merc' popup
	LaptopSaveInfo.sLastHiredMerc.fHaveDisplayedPopUpInLaptop = FALSE;

	//Initialize all vars
	guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
	guiPreviousLaptopMode = LAPTOP_MODE_NONE;
	guiCurrentWWWMode = LAPTOP_MODE_NONE;


	gfShowBookmarks=FALSE;
	InitBookMarkList();
	GameInitAIM();
	GameInitAimSort();
	GameInitMercs();
	GameInitBobbyRGuns();
	GameInitBobbyRMailOrder();
	GameInitEmail();
	GameInitCharProfile();
	GameInitInsuranceContract();
	GameInitFiles();
	GameInitPersonnel();

	// init program states
	memset( &gLaptopProgramStates, LAPTOP_PROGRAM_MINIMIZED, sizeof( gLaptopProgramStates ) );

	gfAtLeastOneMercWasHired = FALSE;

	//No longer inits the laptop screens, now InitLaptopAndLaptopScreens() does

	return( 1 );
}


BOOLEAN InitLaptopAndLaptopScreens()
{

	GameInitFinances();
	GameInitHistory();

	//Reset the flag so we can create a new IMP character
	LaptopSaveInfo.fIMPCompletedFlag = FALSE;

	//Reset the flag so that BOBBYR's isnt available at the begining of the game
	LaptopSaveInfo.fBobbyRSiteCanBeAccessed = FALSE;

	return( TRUE );
}


//This is only called once at game shutdown.
UINT32 LaptopScreenShutdown()
{
	InsuranceContractEndGameShutDown();
	BobbyRayMailOrderEndGameShutDown();
	ShutDownEmailList();

	ClearHistoryList( );

  return TRUE;
}


static void CreateDestroyMouseRegionForNewMailIcon(void);
static BOOLEAN CreateLapTopMouseRegions(void);
static BOOLEAN DrawDeskTopBackground(void);
static void EnterLaptopInitLaptopPages(void);
static void InitLaptopOpenQueue(void);
static void InitalizeSubSitesList(void);
static BOOLEAN IsItRaining(void);
static BOOLEAN LoadBookmark(void);
static BOOLEAN LoadDesktopBackground(void);
static BOOLEAN LoadLoadPending(void);
static void RenderLapTopImage(void);


static INT32 EnterLaptop(void)
{
	//Create, load, initialize data -- just entered the laptop.

	// we are re entering due to message box, leave NOW!
	if( fExitDueToMessageBox  == TRUE )
	{

		return ( TRUE );
	}

	//if the radar map mouse region is still active, disable it.
	if( gRadarRegion.uiFlags & MSYS_REGION_ENABLED )
	{
		MSYS_DisableRegion( &gRadarRegion );
/*
		#ifdef JA2BETAVERSION
			DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, L"Mapscreen's radar region is still active, please tell Dave how you entered Laptop.", LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL );
		#endif
*/
	}

	gfDontStartTransitionFromLaptop = FALSE;

	//Since we are coming in from MapScreen, uncheck the flag
	guiTacticalInterfaceFlags &= ~INTERFACE_MAPSCREEN;

	// ATE: Disable messages....
	DisableScrollMessages( );

	// Stop any person from saying anything
	StopAnyCurrentlyTalkingSpeech( );

	// Don't play music....
	SetMusicMode( MUSIC_LAPTOP );

	// Stop ambients...
	StopAmbients( );

	//if its raining, start the rain showers
	if( IsItRaining() )
	{
		//Enable the rain delay warning
		giRainDelayInternetSite = -1;

		//lower the volume
		guiRainLoop	= PlayJA2Ambient( RAIN_1, LOWVOLUME, 0 );
	}

	//pause the game because we dont want time to advance in the laptop
	PauseGame();

	// set the fact we are currently in laptop, for rendering purposes
	fCurrentlyInLaptop = TRUE;



	// clear guiSAVEBUFFER
	//ColorFillVideoSurfaceArea(guiSAVEBUFFER,	0, 0, 640, 480, Get16BPPColor(FROMRGB(0, 0, 0)) );
  // disable characters panel buttons

  // reset redraw flag and redraw new mail
	fReDrawScreenFlag = FALSE;
  fReDrawNewMailFlag = TRUE;

	// sub page
	giCurrentSubPage = 0;

	// load the laptop graphic and add it
	guiLAPTOP = AddVideoObjectFromFile("LAPTOP/laptop3.sti");
	CHECKF(guiLAPTOP != NO_VOBJECT);

	// background for panel
	guiLaptopBACKGROUND = AddVideoObjectFromFile("LAPTOP/taskbar.sti");
	CHECKF(guiLaptopBACKGROUND != NO_VOBJECT);

	// background for panel
	guiTITLEBARLAPTOP = AddVideoObjectFromFile("LAPTOP/programtitlebar.sti");
	CHECKF(guiTITLEBARLAPTOP != NO_VOBJECT);

	// lights for power and HD
	guiLIGHTS = AddVideoObjectFromFile("LAPTOP/lights.sti");
	CHECKF(guiLIGHTS != NO_VOBJECT);

	// icons for title bars
	guiTITLEBARICONS = AddVideoObjectFromFile("LAPTOP/ICONS.sti");
	CHECKF(guiTITLEBARICONS != NO_VOBJECT);

	// load, blt and delete graphics
	guiEmailWarning = AddVideoObjectFromFile("LAPTOP/NewMailWarning.sti");
	CHECKF(guiEmailWarning != NO_VOBJECT);
	// load background
	LoadDesktopBackground( );


	guiCurrentLaptopMode = LAPTOP_MODE_NONE;
	//MSYS_SetCurrentCursor(CURSOR_NORMAL);

	guiCurrentLaptopMode = LAPTOP_MODE_NONE;
	guiPreviousLaptopMode = LAPTOP_MODE_NONE;
	guiCurrentWWWMode = LAPTOP_MODE_NONE;
  CreateLapTopMouseRegions();
  RenderLapTopImage();

	// reset bookmarks flags
	fFirstTimeInLaptop = TRUE;

	// reset all bookmark visits
	memset( &LaptopSaveInfo.fVisitedBookmarkAlready, 0, sizeof( LaptopSaveInfo.fVisitedBookmarkAlready ) );

	// init program states
	memset( &gLaptopProgramStates, LAPTOP_PROGRAM_MINIMIZED, sizeof( gLaptopProgramStates ) );

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
	InitLaptopOpenQueue( );


  gfShowBookmarks=FALSE;
  LoadBookmark( );
  SetBookMark(AIM_BOOKMARK);
	LoadLoadPending( );

	DrawDeskTopBackground( );

	// create region for new mail icon
	CreateDestroyMouseRegionForNewMailIcon( );

	//DEF: Added to Init things in various laptop pages
	EnterLaptopInitLaptopPages();
	InitalizeSubSitesList( );

	fShowAtmPanelStartButton = TRUE;

	InvalidateRegion(0,0,640,480);

	return( TRUE );
}


static void CreateDestoryBookMarkRegions(void);
static void CreateDestroyMinimizeButtonForCurrentMode(void);
static void DeleteBookmark(void);
static void DeleteDesktopBackground(void);
static void DeleteLapTopButtons(void);
static BOOLEAN DeleteLapTopMouseRegions(void);
static void DeleteLoadPending(void);
static UINT32 ExitLaptopMode(UINT32 uiMode);


void ExitLaptop()
{

  // exit is called due to message box, leave
	if( fExitDueToMessageBox )
	{
		fExitDueToMessageBox = FALSE;
		return;
	}

  if ( DidGameJustStart() )
  {
	  SetMusicMode( MUSIC_LAPTOP );
  }
  else
  {
	  // Restore to old stuff...
	  SetMusicMode( MUSIC_RESTORE);
  }

	// Start ambients...
	BuildDayAmbientSounds( );

	//if its raining, start the rain showers
	if( IsItRaining() )
	{
		//Raise the volume to where it was
		guiRainLoop	= PlayJA2Ambient( RAIN_1, MIDVOLUME, 0 );
	}

	// release cursor
	FreeMouseCursor( );

	// set the fact we are currently not in laptop, for rendering purposes
	fCurrentlyInLaptop = FALSE;


	//Deallocate, save data -- leaving laptop.
  SetRenderFlags( RENDER_FLAG_FULL );

	if( fExitDuringLoad == FALSE )
	{
		ExitLaptopMode(guiCurrentLaptopMode);
	}

	fExitDuringLoad = FALSE;
	fLoadPendingFlag = FALSE;


	DeleteVideoObjectFromIndex(guiLAPTOP);
  DeleteVideoObjectFromIndex(guiLaptopBACKGROUND);
	DeleteVideoObjectFromIndex( guiTITLEBARLAPTOP );
	DeleteVideoObjectFromIndex( guiLIGHTS );
  DeleteVideoObjectFromIndex( guiTITLEBARICONS );
	DeleteVideoObjectFromIndex( guiEmailWarning );

	// destroy region for new mail icon
	CreateDestroyMouseRegionForNewMailIcon( );

	// get rid of desktop
	DeleteDesktopBackground(  );

	if (MailToDelete != NULL)
	{
		MailToDelete = NULL;
	 CreateDestroyDeleteNoticeMailButton();
	}
	if(fNewMailFlag)
	{
	 fNewMailFlag=FALSE;
   CreateDestroyNewMailButton();
	}


	// get rid of minize button
	CreateDestroyMinimizeButtonForCurrentMode( );



	//MSYS_SetCurrentCursor(CURSOR_NORMAL);
	gfEnterLapTop=TRUE;
	DeleteLapTopButtons();
  DeleteLapTopMouseRegions();
	// restore tactical buttons
  //CreateCurrentTacticalPanelButtons();
	gfShowBookmarks=FALSE;
	CreateDestoryBookMarkRegions( );

  fNewWWW=TRUE;
	DeleteBookmark( );
	//DeleteBookmarkRegions();
  DeleteLoadPending( );
	fReDrawNewMailFlag = FALSE;

	//Since we are going to MapScreen, check the flag
	guiTacticalInterfaceFlags |= INTERFACE_MAPSCREEN;

	//pause the game because we dont want time to advance in the laptop
	UnPauseGame();

}


static void RenderLapTopImage(void)
{
	if( ( fMaximizingProgram == TRUE ) ||( fMinizingProgram == TRUE) )
	{
		return;
	}

	BltVideoObjectFromIndex(FRAME_BUFFER, guiLAPTOP, 0, LAPTOP_X, LAPTOP_Y);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiLaptopBACKGROUND, 1, 25, 23);

	MarkButtonsDirty( );
}


static void RenderLaptop(void)
{
	UINT32 uiTempMode = 0;

	if( ( fMaximizingProgram == TRUE ) ||( fMinizingProgram == TRUE) )
	{
		gfShowBookmarks = FALSE;
		return;
	}

	if(fLoadPendingFlag)
	{
		uiTempMode = guiCurrentLaptopMode;
		guiCurrentLaptopMode = guiPreviousLaptopMode;
	}

	switch( guiCurrentLaptopMode )
	{
		case( LAPTOP_MODE_NONE ):
		  DrawDeskTopBackground( );
		break;
		case LAPTOP_MODE_AIM:
		  RenderAIM();
			break;
		case LAPTOP_MODE_AIM_MEMBERS:
			RenderAIMMembers();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
			RenderAimFacialIndex();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
			RenderAimSort();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
			RenderAimArchives();
			break;
		case LAPTOP_MODE_AIM_POLICIES:
			RenderAimPolicies();
			break;
		case LAPTOP_MODE_AIM_LINKS:
			RenderAimLinks();
			break;
		case LAPTOP_MODE_AIM_HISTORY:
			RenderAimHistory();
			break;
		case LAPTOP_MODE_MERC:
			RenderMercs();
			break;
		case LAPTOP_MODE_MERC_FILES:
			RenderMercsFiles();
			break;
		case LAPTOP_MODE_MERC_ACCOUNT:
			RenderMercsAccount();
			break;
		case LAPTOP_MODE_MERC_NO_ACCOUNT:
			RenderMercsNoAccount();
			break;

		case LAPTOP_MODE_BOBBY_R:
			RenderBobbyR();
			break;

		case LAPTOP_MODE_BOBBY_R_GUNS:
			RenderBobbyRGuns();
			break;
		case LAPTOP_MODE_BOBBY_R_AMMO:
			RenderBobbyRAmmo();
			break;
		case LAPTOP_MODE_BOBBY_R_ARMOR:
			RenderBobbyRArmour();
			break;
		case LAPTOP_MODE_BOBBY_R_MISC:
			RenderBobbyRMisc();
			break;
		case LAPTOP_MODE_BOBBY_R_USED:
			RenderBobbyRUsed();
			break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:
			RenderBobbyRMailOrder();
			break;
		case LAPTOP_MODE_CHAR_PROFILE:
			RenderCharProfile();
			break;
		case LAPTOP_MODE_FLORIST:
			RenderFlorist();
			break;
		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
			RenderFloristGallery();
			break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:
			RenderFloristOrderForm();
			break;
		case LAPTOP_MODE_FLORIST_CARD_GALLERY:
			RenderFloristCards();
			break;

		case LAPTOP_MODE_INSURANCE:
			RenderInsurance();
			break;

		case LAPTOP_MODE_INSURANCE_INFO:
			RenderInsuranceInfo();
			break;

		case LAPTOP_MODE_INSURANCE_CONTRACT:
			RenderInsuranceContract();
			break;

		case LAPTOP_MODE_INSURANCE_COMMENTS:
			RenderInsuranceComments();
			break;

		case LAPTOP_MODE_FUNERAL:
			RenderFuneral();
			break;
		case LAPTOP_MODE_FINANCES:
			RenderFinances();
			break;
		case LAPTOP_MODE_PERSONNEL:
			RenderPersonnel();
			break;
		case LAPTOP_MODE_HISTORY:
			RenderHistory();
			break;
		case LAPTOP_MODE_FILES:
			RenderFiles();
			break;
		case LAPTOP_MODE_EMAIL:
			RenderEmail();
			break;
		case( LAPTOP_MODE_WWW ):
			DrawDeskTopBackground( );
			RenderWWWProgramTitleBar( );
		break;
		case( LAPTOP_MODE_BROKEN_LINK ):
			RenderBrokenLink();
		break;

		case LAPTOP_MODE_BOBBYR_SHIPMENTS:
			RenderBobbyRShipments();
			break;


	}



	if( guiCurrentLaptopMode >= LAPTOP_MODE_WWW )
	{
		// render program bar for www program
		RenderWWWProgramTitleBar( );
	}



	if(fLoadPendingFlag)
	{
		guiCurrentLaptopMode  =  uiTempMode;
		return;
	}

	DisplayProgramBoundingBox( FALSE );

	// mark the buttons dirty at this point
  MarkButtonsDirty( );
}


static BOOLEAN InitTitleBarMaximizeGraphics(UINT32 uiBackgroundGraphic, const wchar_t* pTitle, UINT32 uiIconGraphic, UINT16 usIconGraphicIndex);
static void SetSubSiteAsVisted(void);


static void EnterNewLaptopMode(void)
{
	static BOOLEAN fOldLoadFlag=FALSE;


	if( fExitingLaptopFlag )
	{
		return;
	}
	// cause flicker, as we are going to a new program/WEB page
	fFlickerHD = TRUE;

	// handle maximizing of programs
  switch( guiCurrentLaptopMode )
	{
		case ( LAPTOP_MODE_EMAIL ):
		 if( gLaptopProgramStates[ LAPTOP_PROGRAM_MAILER ] == LAPTOP_PROGRAM_MINIMIZED )
		 {
			 // minized, maximized
			 if(  fMaximizingProgram == FALSE )
			 {
			   fInitTitle = TRUE;
			   InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[ 0 ], guiTITLEBARICONS, 0 );
			   ExitLaptopMode(guiPreviousLaptopMode);

			 }
			 fMaximizingProgram = TRUE;
			 bProgramBeingMaximized = LAPTOP_PROGRAM_MAILER;
			 gLaptopProgramStates[ LAPTOP_PROGRAM_MAILER ] = LAPTOP_PROGRAM_OPEN;

       return;
		 }
		break;
		case ( LAPTOP_MODE_FILES ):
		 if( gLaptopProgramStates[ LAPTOP_PROGRAM_FILES ] == LAPTOP_PROGRAM_MINIMIZED )
		 {
			 // minized, maximized
			 if(  fMaximizingProgram == FALSE )
			 {
			   fInitTitle = TRUE;
			   InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[ 1 ], guiTITLEBARICONS, 2 );
			   ExitLaptopMode(guiPreviousLaptopMode);

			 }

			 // minized, maximized
			 fMaximizingProgram = TRUE;
			 bProgramBeingMaximized = LAPTOP_PROGRAM_FILES;
			 gLaptopProgramStates[ LAPTOP_PROGRAM_FILES ] = LAPTOP_PROGRAM_OPEN;
			 return;
		 }
		break;
		case ( LAPTOP_MODE_PERSONNEL ):
		 if( gLaptopProgramStates[ LAPTOP_PROGRAM_PERSONNEL ] == LAPTOP_PROGRAM_MINIMIZED )
		 {

			 // minized, maximized
			 if(  fMaximizingProgram == FALSE )
			 {
			   fInitTitle = TRUE;
			   InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[ 2 ], guiTITLEBARICONS, 3 );
			   ExitLaptopMode(guiPreviousLaptopMode);

			 }

			 // minized, maximized
			 fMaximizingProgram = TRUE;
			 bProgramBeingMaximized = LAPTOP_PROGRAM_PERSONNEL;
			 gLaptopProgramStates[ LAPTOP_PROGRAM_PERSONNEL ] = LAPTOP_PROGRAM_OPEN;
			 return;
		 }
		break;
		case ( LAPTOP_MODE_FINANCES ):
		 if( gLaptopProgramStates[ LAPTOP_PROGRAM_FINANCES ] == LAPTOP_PROGRAM_MINIMIZED )
		 {

			 // minized, maximized
			 if(  fMaximizingProgram == FALSE )
			 {
			   fInitTitle = TRUE;
			   InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[ 3 ], guiTITLEBARICONS, 5 );
			   ExitLaptopMode(guiPreviousLaptopMode);


			 }

			 // minized, maximized
			 fMaximizingProgram = TRUE;
			 bProgramBeingMaximized = LAPTOP_PROGRAM_FINANCES;
			 gLaptopProgramStates[ LAPTOP_PROGRAM_FINANCES ] = LAPTOP_PROGRAM_OPEN;
			 return;
		 }
		break;
		case ( LAPTOP_MODE_HISTORY ):
		 if( gLaptopProgramStates[ LAPTOP_PROGRAM_HISTORY ] == LAPTOP_PROGRAM_MINIMIZED )
		 {
			 // minized, maximized
			 if(  fMaximizingProgram == FALSE )
			 {
			   fInitTitle = TRUE;
			   InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopTitles[ 4 ], guiTITLEBARICONS, 4 );
			   ExitLaptopMode(guiPreviousLaptopMode);


			 }
			 // minized, maximized
			 fMaximizingProgram = TRUE;
			 bProgramBeingMaximized = LAPTOP_PROGRAM_HISTORY;
			 gLaptopProgramStates[ LAPTOP_PROGRAM_HISTORY ] = LAPTOP_PROGRAM_OPEN;
			 return;
		 }
		break;
		case( LAPTOP_MODE_NONE ):
		  // do nothing
		break;
		default:
		 if( gLaptopProgramStates[ LAPTOP_PROGRAM_WEB_BROWSER ] == LAPTOP_PROGRAM_MINIMIZED )
		 {
			 // minized, maximized
			 if(  fMaximizingProgram == FALSE )
			 {
			   fInitTitle = TRUE;
			   InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pWebTitle[ 0 ], guiTITLEBARICONS, 1 );
			   ExitLaptopMode(guiPreviousLaptopMode);
			 }
			 // minized, maximized
			 fMaximizingProgram = TRUE;
			 bProgramBeingMaximized = LAPTOP_PROGRAM_WEB_BROWSER;
			 gLaptopProgramStates[ LAPTOP_PROGRAM_WEB_BROWSER ] = LAPTOP_PROGRAM_OPEN;
			 return;
		 }
		break;

	}

	if( ( fMaximizingProgram == TRUE ) ||( fMinizingProgram == TRUE) )
	{
		return;
	}

	if((fOldLoadFlag)&&(!fLoadPendingFlag))
	{
	  fOldLoadFlag=FALSE;
	}
  else if((fLoadPendingFlag)&&(!fOldLoadFlag))
  {
    ExitLaptopMode(guiPreviousLaptopMode);
		fOldLoadFlag=TRUE;
		return;
	}
	else if((fOldLoadFlag)&&(fLoadPendingFlag))
	{
		return;
	}
	else
	{
		// do not exit previous mode if coming from sliding bar handler
		if( ( fEnteredNewLapTopDueToHandleSlidingBars == FALSE ) )
		{
      ExitLaptopMode(guiPreviousLaptopMode);
		}
	}



	if((guiCurrentWWWMode==LAPTOP_MODE_NONE)&&(guiCurrentLaptopMode >=LAPTOP_MODE_WWW))
	{
   RenderLapTopImage();
	 guiCurrentLaptopMode =LAPTOP_MODE_WWW;
	}
	else
	{
	 if( guiCurrentLaptopMode > LAPTOP_MODE_WWW)
	 {
	 	 if(guiPreviousLaptopMode < LAPTOP_MODE_WWW )
			 guiCurrentLaptopMode = guiCurrentWWWMode;
		 else
		 {
			  guiCurrentWWWMode = guiCurrentLaptopMode;
			  giCurrentSubPage =0;
		 }
	  }
	}

	if( guiCurrentLaptopMode >= LAPTOP_MODE_WWW )
	{
		RenderWWWProgramTitleBar( );
	}



	if( ( guiCurrentLaptopMode >= LAPTOP_MODE_WWW ) &&( guiPreviousLaptopMode >= LAPTOP_MODE_WWW ) )
	{
	  gfShowBookmarks = FALSE;
	}



	//Initialize the new mode.
	switch( guiCurrentLaptopMode )
	{

		case LAPTOP_MODE_AIM:
			EnterAIM();
			break;
		case LAPTOP_MODE_AIM_MEMBERS:
			EnterAIMMembers();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
			EnterAimFacialIndex();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
			EnterAimSort();
			break;

		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
			EnterAimArchives();
			break;
		case LAPTOP_MODE_AIM_POLICIES:
			EnterAimPolicies();
			break;
		case LAPTOP_MODE_AIM_LINKS:
			EnterAimLinks();
			break;
		case LAPTOP_MODE_AIM_HISTORY:
			EnterAimHistory();
			break;

		case LAPTOP_MODE_MERC:
			EnterMercs();
			break;
		case LAPTOP_MODE_MERC_FILES:
			EnterMercsFiles();
			break;
		case LAPTOP_MODE_MERC_ACCOUNT:
			EnterMercsAccount();
			break;
		case LAPTOP_MODE_MERC_NO_ACCOUNT:
			EnterMercsNoAccount();
			break;

		case LAPTOP_MODE_BOBBY_R:
			EnterBobbyR();
			break;
		case LAPTOP_MODE_BOBBY_R_GUNS:
			EnterBobbyRGuns();
			break;
		case LAPTOP_MODE_BOBBY_R_AMMO:
			EnterBobbyRAmmo();
			break;
		case LAPTOP_MODE_BOBBY_R_ARMOR:
			EnterBobbyRArmour();
			break;
		case LAPTOP_MODE_BOBBY_R_MISC:
			EnterBobbyRMisc();
			break;
		case LAPTOP_MODE_BOBBY_R_USED:
			EnterBobbyRUsed();
			break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:
			EnterBobbyRMailOrder();
			break;
		case LAPTOP_MODE_CHAR_PROFILE:
			EnterCharProfile();
			break;

		case LAPTOP_MODE_FLORIST:
			EnterFlorist();
			break;
		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
			EnterFloristGallery();
			break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:
			EnterFloristOrderForm();
			break;
		case LAPTOP_MODE_FLORIST_CARD_GALLERY:
			EnterFloristCards();
			break;

		case LAPTOP_MODE_INSURANCE:
			EnterInsurance();
			break;
		case LAPTOP_MODE_INSURANCE_INFO:
			EnterInsuranceInfo();
			break;
		case LAPTOP_MODE_INSURANCE_CONTRACT:
			EnterInsuranceContract();
			break;
		case LAPTOP_MODE_INSURANCE_COMMENTS:
			EnterInsuranceComments();
			break;

		case LAPTOP_MODE_FUNERAL:
			EnterFuneral();
			break;
		case LAPTOP_MODE_FINANCES:
			EnterFinances();
			break;
		case LAPTOP_MODE_PERSONNEL:
			EnterPersonnel();
			break;
		case LAPTOP_MODE_HISTORY:
			EnterHistory();
			break;
		case LAPTOP_MODE_FILES:
			EnterFiles();
			break;
		case LAPTOP_MODE_EMAIL:
			EnterEmail();
			break;
		case LAPTOP_MODE_BROKEN_LINK:
			EnterBrokenLink();
		break;
		case LAPTOP_MODE_BOBBYR_SHIPMENTS:
			EnterBobbyRShipments();
			break;

	}

	// first time using webbrowser in this laptop session
	if( ( fFirstTimeInLaptop == TRUE ) && ( guiCurrentLaptopMode >=LAPTOP_MODE_WWW ) )
	{
		// show bookmarks
		gfShowBookmarks = TRUE;

		// reset flag
		fFirstTimeInLaptop = FALSE;
	}

  if( ( !fLoadPendingFlag) )
	{
	  CreateDestroyMinimizeButtonForCurrentMode( );
		guiPreviousLaptopMode = guiCurrentLaptopMode;
		SetSubSiteAsVisted( );
	}

	DisplayProgramBoundingBox( TRUE );
}


static void HandleLapTopHandles(void)
{
	if(fLoadPendingFlag)
		return;

	if( ( fMaximizingProgram == TRUE ) || ( fMinizingProgram == TRUE ) )
	{
		return;
	}


 	switch( guiCurrentLaptopMode )
	{

		case LAPTOP_MODE_AIM:

			HandleAIM();
			break;
		case LAPTOP_MODE_AIM_MEMBERS:
			HandleAIMMembers();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
			HandleAimFacialIndex();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
			HandleAimSort();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
			HandleAimArchives();
			break;
		case LAPTOP_MODE_AIM_POLICIES:
			HandleAimPolicies();
			break;
		case LAPTOP_MODE_AIM_LINKS:
			HandleAimLinks();
			break;
		case LAPTOP_MODE_AIM_HISTORY:
			HandleAimHistory();
			break;

		case LAPTOP_MODE_MERC:
			HandleMercs();
			break;
		case LAPTOP_MODE_MERC_FILES:
			HandleMercsFiles();
			break;
		case LAPTOP_MODE_MERC_ACCOUNT:
			HandleMercsAccount();
			break;
		case LAPTOP_MODE_MERC_NO_ACCOUNT:
			HandleMercsNoAccount();
			break;


		case LAPTOP_MODE_BOBBY_R:
			HandleBobbyR();
			break;
		case LAPTOP_MODE_BOBBY_R_GUNS:
			HandleBobbyRGuns();
			break;
		case LAPTOP_MODE_BOBBY_R_AMMO:
			HandleBobbyRAmmo();
			break;
		case LAPTOP_MODE_BOBBY_R_ARMOR:
			HandleBobbyRArmour();
			break;
		case LAPTOP_MODE_BOBBY_R_MISC:
			HandleBobbyRMisc();
			break;
		case LAPTOP_MODE_BOBBY_R_USED:
			HandleBobbyRUsed();
			break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:
			HandleBobbyRMailOrder();
			break;


		case LAPTOP_MODE_CHAR_PROFILE:
			HandleCharProfile();
			break;
		case LAPTOP_MODE_FLORIST:
			HandleFlorist();
			break;
		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
			HandleFloristGallery();
			break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:
			HandleFloristOrderForm();
			break;
		case LAPTOP_MODE_FLORIST_CARD_GALLERY:
			HandleFloristCards();
			break;

		case LAPTOP_MODE_INSURANCE:
			HandleInsurance();
			break;

		case LAPTOP_MODE_INSURANCE_INFO:
			HandleInsuranceInfo();
			break;

		case LAPTOP_MODE_INSURANCE_CONTRACT:
			HandleInsuranceContract();
			break;
		case LAPTOP_MODE_INSURANCE_COMMENTS:
			HandleInsuranceComments();
			break;

		case LAPTOP_MODE_FUNERAL:
			HandleFuneral();
			break;
		case LAPTOP_MODE_FINANCES:
			HandleFinances();
			break;
		case LAPTOP_MODE_PERSONNEL:
			HandlePersonnel();
			break;
		case LAPTOP_MODE_HISTORY:
			HandleHistory();
			break;
		case LAPTOP_MODE_FILES:
			HandleFiles();
			break;
		case LAPTOP_MODE_EMAIL:
			HandleEmail();
			break;

		case LAPTOP_MODE_BROKEN_LINK:
			HandleBrokenLink();
			break;

		case LAPTOP_MODE_BOBBYR_SHIPMENTS:
			HandleBobbyRShipments();
			break;
	}

}


static void CheckIfNewWWWW(void);
static void CheckMarkButtonsDirtyFlag(void);
static UINT32 CreateLaptopButtons(void);
static void DisplayBookMarks(void);
static BOOLEAN DisplayLoadPending(void);
static void DisplayTaskBarIcons(void);
static void DisplayWebBookMarkNotify(void);
static void FlickerHDLight(void);
static void HandleSlidingTitleBar(void);
static void HandleWWWSubSites(void);
static void PostButtonRendering(void);
static void ShouldNewMailBeDisplayed(void);
static void ShowLights(void);
static void UpdateStatusOfDisplayingBookMarks(void);


UINT32 LaptopScreenHandle()
{
	//User just changed modes.  This is determined by the button callbacks
	//created in LaptopScreenInit()

	// just entered
	if(gfEnterLapTop)
	{
	 EnterLaptop();
	 CreateLaptopButtons();
   gfEnterLapTop=FALSE;
	}

	if( gfStartMapScreenToLaptopTransition )
	{ //Everything is set up to start the transition animation.
		SGPRect SrcRect1, SrcRect2, DstRect;
		INT32 iPercentage, iScalePercentage, iFactor;
		UINT32 uiStartTime, uiTimeRange, uiCurrTime;
		INT32 iX, iY, iWidth, iHeight;

		INT32 iRealPercentage;

		SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
		//Step 1:  Build the laptop image into the save buffer.
		gfStartMapScreenToLaptopTransition = FALSE;
		RestoreBackgroundRects();
		RenderLapTopImage();
		RenderLaptop();
		RenderButtons();
		PrintDate( );
		PrintBalance( );
		PrintNumberOnTeam( );
		ShowLights();

		//Step 2:  The mapscreen image is in the EXTRABUFFER, and laptop is in the SAVEBUFFER
		//         Start transitioning the screen.
		DstRect.iLeft = 0;
		DstRect.iTop = 0;
		DstRect.iRight = 640;
		DstRect.iBottom = 480;
		uiTimeRange = 1000;
		iPercentage = iRealPercentage = 0;
		uiStartTime = GetJA2Clock();
		BlitBufferToBuffer( FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 640, 480 );
		BlitBufferToBuffer( guiEXTRABUFFER, FRAME_BUFFER, 0, 0, 640, 480 );
		PlayJA2SampleFromFile("SOUNDS/Laptop power up (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
		while( iRealPercentage < 100  )
		{
			uiCurrTime = GetJA2Clock();
			iPercentage = (uiCurrTime-uiStartTime) * 100 / uiTimeRange;
			iPercentage = min( iPercentage, 100 );

			iRealPercentage = iPercentage;

			//Factor the percentage so that it is modified by a gravity falling acceleration effect.
			iFactor = (iPercentage - 50) * 2;
			if( iPercentage < 50 )
				iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
			else
				iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.5);

			//Mapscreen source rect
			SrcRect1.iLeft = 464 * iPercentage / 100;
			SrcRect1.iRight = 640 - 163 * iPercentage / 100;
			SrcRect1.iTop = 417 * iPercentage / 100;
			SrcRect1.iBottom = 480 - 55 * iPercentage / 100;
			//Laptop source rect
			if( iPercentage < 99 )
				iScalePercentage = 10000 / (100-iPercentage);
			else
				iScalePercentage = 5333;
			iWidth = 12 * iScalePercentage / 100;
			iHeight = 9 * iScalePercentage / 100;
			iX = 472 - (472-320) * iScalePercentage / 5333;
			iY = 424 - (424-240) * iScalePercentage / 5333;

			SrcRect2.iLeft = iX - iWidth / 2;
			SrcRect2.iRight = SrcRect2.iLeft + iWidth;
			SrcRect2.iTop = iY - iHeight / 2;
			SrcRect2.iBottom = SrcRect2.iTop + iHeight;
			//SrcRect2.iLeft = 464 - 464 * iScalePercentage / 100;
			//SrcRect2.iRight = 477 + 163 * iScalePercentage / 100;
			//SrcRect2.iTop = 417 - 417 * iScalePercentage / 100;
			//SrcRect2.iBottom = 425 + 55 * iScalePercentage / 100;

			//BltStretchVideoSurface(FRAME_BUFFER, guiEXTRABUFFER, &SrcRect1, &DstRect);

			//SetFont( FONT10ARIAL );
			//SetFontForeground( FONT_YELLOW );
			//SetFontShadow( FONT_NEARBLACK );
			//mprintf( 10, 10, L"%d -> %d", iRealPercentage, iPercentage );
			//pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
			//SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );
			//RectangleDraw( TRUE, SrcRect1.iLeft, SrcRect1.iTop, SrcRect1.iRight, SrcRect1.iBottom, Get16BPPColor( FROMRGB( 255, 100, 0 ) ), pDestBuf );
			//RectangleDraw( TRUE, SrcRect2.iLeft, SrcRect2.iTop, SrcRect2.iRight, SrcRect2.iBottom, Get16BPPColor( FROMRGB( 100, 255, 0 ) ), pDestBuf );
			//UnLockVideoSurface( FRAME_BUFFER );

			BltStretchVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, &DstRect, &SrcRect2);
			InvalidateScreen();
			RefreshScreen();
		}
		fReDrawScreenFlag = TRUE;
	}


	//DO NOT MOVE THIS FUNCTION CALL!!!

	//This determines if the help screen should be active
	if( ShouldTheHelpScreenComeUp( HELP_SCREEN_LAPTOP, FALSE ) )
	{
		// handle the help screen
		HelpScreenHandler();
		return( LAPTOP_SCREEN );
	}

	RestoreBackgroundRects();

	// lock cursor to screen
	RestrictMouseCursor( &LaptopScreenRect );



	// handle animated cursors
	HandleAnimatedCursors( );
	// Deque all game events
	DequeAllGameEvents( TRUE );

	// handle sub sites..like BR Guns, BR Ammo, Armour, Misc...for WW Wait..since they are not true sub pages
	// and are not individual sites
	HandleWWWSubSites( );
	UpdateStatusOfDisplayingBookMarks( );

	// check if we need to reset new WWW mode
	CheckIfNewWWWW( );

	if( guiCurrentLaptopMode != guiPreviousLaptopMode )
	{
		if( guiCurrentLaptopMode <=LAPTOP_MODE_WWW )
		{
			fLoadPendingFlag=FALSE;
		}

		if( ( fMaximizingProgram == FALSE ) && ( fMinizingProgram == FALSE ) )
		{
			if( guiCurrentLaptopMode <= LAPTOP_MODE_WWW )
			{
				EnterNewLaptopMode();
				if( ( fMaximizingProgram == FALSE ) && ( fMinizingProgram == FALSE ) )
				{
					guiPreviousLaptopMode = guiCurrentLaptopMode;
				}
			}
	    else
			{
				 if(!fLoadPendingFlag)
				 {
					EnterNewLaptopMode();
					guiPreviousLaptopMode = guiCurrentLaptopMode;
				 }
			}
		}
  }
	if( fPausedReDrawScreenFlag )
	{
		fReDrawScreenFlag = TRUE;
		fPausedReDrawScreenFlag = FALSE;
	}

  if( fReDrawScreenFlag )
	{
   RenderLapTopImage();
	 RenderLaptop();
	}


	// are we about to leave laptop
	if( fExitingLaptopFlag )
	{
		if( fLoadPendingFlag == TRUE )
		{
		  fLoadPendingFlag = FALSE;
		  fExitDuringLoad = TRUE;
		}
		LeaveLapTopScreen( );
	}

	if( fExitingLaptopFlag == FALSE )
	{
	  // handle handles for laptop input stream
	  HandleLapTopHandles();
	}

	// get keyboard input, handle it
	GetLaptopKeyboardInput();





	// check to see if new mail box needs to be displayed
	DisplayNewMailBox( );
  CreateDestroyNewMailButton( );

	// create various mouse regions that are global to laptop system
  CreateDestoryBookMarkRegions( );

	// check to see if buttons marked dirty
  CheckMarkButtonsDirtyFlag( );

  // check to see if new mail box needs to be displayed
	ShouldNewMailBeDisplayed();

	// check to see if new mail box needs to be displayed
	ReDrawNewMailBox( );


	// look for unread email
	LookForUnread();
	//Handle keyboard shortcuts...

	// mouse regions
  //RenderButtons();
	//RenderButtonsFastHelp( );



	if( ( fLoadPendingFlag == FALSE )||( fNewMailFlag  ) )
	{
	  // render buttons marked dirty
	  RenderButtons( );

    // render fast help 'quick created' buttons
//		RenderFastHelp( );
//	  RenderButtonsFastHelp( );
  }

	// check to see if bookmarks need to be displayed
	if(gfShowBookmarks)
	{
		if( fExitingLaptopFlag )
			gfShowBookmarks = FALSE;
		else
			DisplayBookMarks();
	}

	// check to see if laod pending flag is set
	DisplayLoadPending( );

	// check if we are showing message?
	DisplayWebBookMarkNotify( );

	if (fReDrawPostButtonRender)
	{
	  // rendering AFTER buttons and button text
	 	if( ( fMaximizingProgram == FALSE ) && ( fMinizingProgram == FALSE ) )
		{
		  PostButtonRendering( );
		}
  }
	//PrintBalance( );

	PrintDate( );

	PrintBalance( );

	PrintNumberOnTeam( );
	DisplayTaskBarIcons();

	// handle if we are maximizing a program from a minimized state or vice versa
	HandleSlidingTitleBar( );


	// flicker HD light as nessacary
	FlickerHDLight( );

	// display power and HD lights
	ShowLights( );


	// render frame rate
	DisplayFrameRate( );

	// invalidate screen if redrawn
	if( fReDrawScreenFlag == TRUE )
	{
		InvalidateRegion(0,0,640,480);
    fReDrawScreenFlag = FALSE;
	}

	ExecuteVideoOverlays();

	SaveBackgroundRects( );
//	RenderButtonsFastHelp();
	RenderFastHelp( );

  // ex SAVEBUFFER queue
	ExecuteBaseDirtyRectQueue();
	ResetInterface();
	EndFrameBufferRender( );
	return (LAPTOP_SCREEN);
}


static UINT32 ExitLaptopMode(UINT32 uiMode)
{
		//Deallocate the previous mode that you were in.

	switch( uiMode )
	{
		case LAPTOP_MODE_AIM:
			ExitAIM();
			break;
		case LAPTOP_MODE_AIM_MEMBERS:
			ExitAIMMembers();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX:
			ExitAimFacialIndex();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES:
			ExitAimSort();
			break;
		case LAPTOP_MODE_AIM_MEMBERS_ARCHIVES:
			ExitAimArchives();
			break;
		case LAPTOP_MODE_AIM_POLICIES:
			ExitAimPolicies();
			break;
		case LAPTOP_MODE_AIM_LINKS:
			ExitAimLinks();
			break;
		case LAPTOP_MODE_AIM_HISTORY:
			ExitAimHistory();
			break;

		case LAPTOP_MODE_MERC:
			ExitMercs();
			break;
		case LAPTOP_MODE_MERC_FILES:
			ExitMercsFiles();
			break;
		case LAPTOP_MODE_MERC_ACCOUNT:
			ExitMercsAccount();
			break;
		case LAPTOP_MODE_MERC_NO_ACCOUNT:
			ExitMercsNoAccount();
			break;


		case LAPTOP_MODE_BOBBY_R:
			ExitBobbyR();
			break;
		case LAPTOP_MODE_BOBBY_R_GUNS:
			ExitBobbyRGuns();
			break;
		case LAPTOP_MODE_BOBBY_R_AMMO:
			ExitBobbyRAmmo();
			break;
		case LAPTOP_MODE_BOBBY_R_ARMOR:
			ExitBobbyRArmour();
			break;
		case LAPTOP_MODE_BOBBY_R_MISC:
			ExitBobbyRMisc();
			break;
		case LAPTOP_MODE_BOBBY_R_USED:
			ExitBobbyRUsed();
			break;
		case LAPTOP_MODE_BOBBY_R_MAILORDER:
			ExitBobbyRMailOrder();
			break;


		case LAPTOP_MODE_CHAR_PROFILE:
			ExitCharProfile();
			break;
		case LAPTOP_MODE_FLORIST:
			ExitFlorist();
			break;
		case LAPTOP_MODE_FLORIST_FLOWER_GALLERY:
			ExitFloristGallery();
			break;
		case LAPTOP_MODE_FLORIST_ORDERFORM:
			ExitFloristOrderForm();
			break;
		case LAPTOP_MODE_FLORIST_CARD_GALLERY:
			ExitFloristCards();
			break;

		case LAPTOP_MODE_INSURANCE:
			ExitInsurance();
			break;

		case LAPTOP_MODE_INSURANCE_INFO:
			ExitInsuranceInfo();
			break;

		case LAPTOP_MODE_INSURANCE_CONTRACT:
			ExitInsuranceContract();
			break;
		case LAPTOP_MODE_INSURANCE_COMMENTS:
			ExitInsuranceComments();
			break;

		case LAPTOP_MODE_FUNERAL:
			ExitFuneral();
			break;
		case LAPTOP_MODE_FINANCES:
			ExitFinances();
			break;
		case LAPTOP_MODE_PERSONNEL:
			ExitPersonnel();
			break;
		case LAPTOP_MODE_HISTORY:
			ExitHistory();
			break;
		case LAPTOP_MODE_FILES:
			ExitFiles();
			break;
  	case LAPTOP_MODE_EMAIL:
			ExitEmail();
			break;
		case LAPTOP_MODE_BROKEN_LINK:
			ExitBrokenLink();
			break;

		case LAPTOP_MODE_BOBBYR_SHIPMENTS:
			ExitBobbyRShipments();
			break;
	}

	if( ( uiMode != LAPTOP_MODE_NONE )&&( uiMode < LAPTOP_MODE_WWW ) )
	{
	  CreateDestroyMinimizeButtonForCurrentMode( );
	}
	return(TRUE);
}


static void BtnOnCallback(GUI_BUTTON* btn, INT32 reason);
static void CreateLaptopButtonHelpText(INT32 iButtonIndex, UINT32 uiButtonHelpTextID);
static void EmailRegionButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void FilesRegionButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void FinancialRegionButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void HistoryRegionButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void PersonnelRegionButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void WWWRegionButtonCallback(GUI_BUTTON* btn, INT32 reason);


static UINT32 CreateLaptopButtons(void)
{
 // the program buttons

 gLaptopButtonImage[0]=  LoadButtonImage( "LAPTOP/buttonsforlaptop.sti" ,-1,0,-1,8,-1 );
 gLaptopButton[0] = QuickCreateButton( gLaptopButtonImage[0], 29, 66,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, EmailRegionButtonCallback);
	CreateLaptopButtonHelpText( gLaptopButton[0], LAPTOP_BN_HLP_TXT_VIEW_EMAIL );

 SpecifyButtonText(  gLaptopButton[0], pLaptopIcons[ 0 ] );
 SpecifyButtonFont( gLaptopButton[0], FONT10ARIAL );
 SpecifyButtonTextOffsets( gLaptopButton[0], 30, 11, TRUE );
 SpecifyButtonDownTextColors( gLaptopButton[0], 2, 0 );
 SpecifyButtonUpTextColors( gLaptopButton[0], 2, 0 );

 gLaptopButtonImage[1]=  LoadButtonImage( "LAPTOP/buttonsforlaptop.sti" ,-1,1,-1,9,-1 );
 gLaptopButton[1] = QuickCreateButton( gLaptopButtonImage[1], 29, 98,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, WWWRegionButtonCallback);
	CreateLaptopButtonHelpText( gLaptopButton[1], LAPTOP_BN_HLP_TXT_BROWSE_VARIOUS_WEB_SITES );

 SpecifyButtonText(  gLaptopButton[1], pLaptopIcons[1 ] );
 SpecifyButtonFont( gLaptopButton[1], FONT10ARIAL );
 SpecifyButtonTextOffsets( gLaptopButton[1], 30, 11, TRUE );
 SpecifyButtonUpTextColors( gLaptopButton[1], 2, 0 );
 SpecifyButtonDownTextColors( gLaptopButton[1], 2, 0 );

 gLaptopButtonImage[2]=  LoadButtonImage( "LAPTOP/buttonsforlaptop.sti" ,-1,2,-1,10,-1 );
 gLaptopButton[2] = QuickCreateButton( gLaptopButtonImage[2], 29, 130,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, FilesRegionButtonCallback);
	CreateLaptopButtonHelpText( gLaptopButton[2], LAPTOP_BN_HLP_TXT_VIEW_FILES_AND_EMAIL_ATTACHMENTS );

 SpecifyButtonText(  gLaptopButton[2], pLaptopIcons[ 5 ] );
 SpecifyButtonFont( gLaptopButton[2], FONT10ARIAL );
 SpecifyButtonTextOffsets( gLaptopButton[2], 30, 11, TRUE );
 SpecifyButtonUpTextColors( gLaptopButton[2], 2, 0 );
 SpecifyButtonDownTextColors( gLaptopButton[2], 2, 0 );


 gLaptopButtonImage[3]=  LoadButtonImage( "LAPTOP/buttonsforlaptop.sti" ,-1,3,-1,11,-1 );
 gLaptopButton[3] = QuickCreateButton( gLaptopButtonImage[3], 29, 194,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, PersonnelRegionButtonCallback);
	CreateLaptopButtonHelpText( gLaptopButton[3], LAPTOP_BN_HLP_TXT_VIEW_TEAM_INFO );

 SpecifyButtonText(  gLaptopButton[3], pLaptopIcons[ 3] );
 SpecifyButtonFont( gLaptopButton[3], FONT10ARIAL );
 SpecifyButtonTextOffsets( gLaptopButton[3], 30, 11, TRUE );
 SpecifyButtonUpTextColors( gLaptopButton[3], 2, 0 );
 SpecifyButtonDownTextColors( gLaptopButton[3], 2, 0 );


 gLaptopButtonImage[4]=  LoadButtonImage( "LAPTOP/buttonsforlaptop.sti" ,-1,4,-1,12,-1 );
 gLaptopButton[4] = QuickCreateButton( gLaptopButtonImage[4], 29, 162,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, HistoryRegionButtonCallback);
	CreateLaptopButtonHelpText( gLaptopButton[4], LAPTOP_BN_HLP_TXT_READ_LOG_OF_EVENTS );

 SpecifyButtonText(  gLaptopButton[4], pLaptopIcons[ 4 ] );
 SpecifyButtonFont( gLaptopButton[4], FONT10ARIAL );
 SpecifyButtonTextOffsets( gLaptopButton[4], 30, 11, TRUE );
 SpecifyButtonUpTextColors( gLaptopButton[4], 2, 0 );
 SpecifyButtonDownTextColors( gLaptopButton[4], 2, 0 );


 gLaptopButtonImage[5]=  LoadButtonImage( "LAPTOP/buttonsforlaptop.sti" ,-1,5,-1,13,-1 );
 gLaptopButton[5] = QuickCreateButton( gLaptopButtonImage[5], 29, 226 + 15,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, FinancialRegionButtonCallback);
	CreateLaptopButtonHelpText( gLaptopButton[5], LAPTOP_BN_HLP_TXT_VIEW_FINANCIAL_SUMMARY_AND_HISTORY );

 SpecifyButtonText(  gLaptopButton[5], pLaptopIcons[ 2 ] );
 SpecifyButtonFont( gLaptopButton[5], FONT10ARIAL );
 SpecifyButtonTextOffsets( gLaptopButton[5], 30, 11, TRUE );
 SpecifyButtonUpTextColors( gLaptopButton[5], 2, 0 );
 SpecifyButtonDownTextColors( gLaptopButton[5], 2, 0 );


 gLaptopButtonImage[6]=  LoadButtonImage( "LAPTOP/buttonsforlaptop.sti" ,-1,6,-1,14,-1 );
 gLaptopButton[6] = QuickCreateButton( gLaptopButtonImage[6], 29, 371 + 7, //DEF: was 19
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, BtnOnCallback);
	CreateLaptopButtonHelpText( gLaptopButton[6], LAPTOP_BN_HLP_TXT_CLOSE_LAPTOP );

 SpecifyButtonText(  gLaptopButton[6], pLaptopIcons[ 6 ] );
 SpecifyButtonFont( gLaptopButton[6], FONT10ARIAL );
 SpecifyButtonTextOffsets( gLaptopButton[6], 25, 11, TRUE );
 SpecifyButtonUpTextColors( gLaptopButton[6], 2, 0 );
 SpecifyButtonDownTextColors( gLaptopButton[6], 2, 0 );


 // define the cursor
	SetButtonCursor(gLaptopButton[0], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(gLaptopButton[1], CURSOR_LAPTOP_SCREEN);
  SetButtonCursor(gLaptopButton[2], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(gLaptopButton[3], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(gLaptopButton[4], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(gLaptopButton[5], CURSOR_LAPTOP_SCREEN);
	SetButtonCursor(gLaptopButton[6], CURSOR_LAPTOP_SCREEN);



 return (TRUE);
}


static void DeleteLapTopButtons(void)
{
	for (UINT32 cnt = 0; cnt < 7; cnt++)
	{
		RemoveButton( gLaptopButton[ cnt ] );
		UnloadButtonImage( gLaptopButtonImage[ cnt ] );
	}
}


static BOOLEAN HandleExit(void);


static void BtnOnCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (HandleExit())
		{
			fExitingLaptopFlag = TRUE;
		}
	}
}


static BOOLEAN ExitLaptopDone(void);


BOOLEAN LeaveLapTopScreen( void )
{

	if( ExitLaptopDone( ) )
	{

	  // exit screen is set
		// set new screen
		//if( ( LaptopSaveInfo.gfNewGameLaptop != TRUE ) || !( AnyMercsHired() ) )
	//	{
			SetLaptopExitScreen( MAP_SCREEN );
		//}
		//if( ( LaptopSaveInfo.gfNewGameLaptop )&&( AnyMercsHired() ) )
		//{
		//	SetLaptopExitScreen( GAME_SCREEN );
	//	}

		if( gfAtLeastOneMercWasHired == TRUE )
		{
			if( LaptopSaveInfo.gfNewGameLaptop )
			{
				LaptopSaveInfo.gfNewGameLaptop = FALSE;
				fExitingLaptopFlag = TRUE;
				/*guiExitScreen = GAME_SCREEN; */
				InitNewGame( FALSE );
				gfDontStartTransitionFromLaptop = TRUE;
				/*InitHelicopterEntranceByMercs();*/
				return( TRUE );
			}
		}
		else
		{
			gfDontStartTransitionFromLaptop = TRUE;
		}

	  SetPendingNewScreen(guiExitScreen);

		if( !gfDontStartTransitionFromLaptop )
		{
			SGPRect SrcRect1, SrcRect2, DstRect;
			INT32 iPercentage, iScalePercentage, iFactor;
			UINT32 uiStartTime, uiTimeRange, uiCurrTime;
			INT32 iX, iY, iWidth, iHeight;
			INT32 iRealPercentage;

			gfDontStartTransitionFromLaptop = TRUE;
			SetCurrentCursorFromDatabase( VIDEO_NO_CURSOR );
			//Step 1:  Build the laptop image into the save buffer.
			RestoreBackgroundRects();
			RenderLapTopImage();
			RenderLaptop();
			RenderButtons();
			PrintDate( );
			PrintBalance( );
			PrintNumberOnTeam( );
			ShowLights();

			//Step 2:  The mapscreen image is in the EXTRABUFFER, and laptop is in the SAVEBUFFER
			//         Start transitioning the screen.
			DstRect.iLeft = 0;
			DstRect.iTop = 0;
			DstRect.iRight = 640;
			DstRect.iBottom = 480;
			uiTimeRange = 1000;
			iPercentage = iRealPercentage = 100;
			uiStartTime = GetJA2Clock();
			BlitBufferToBuffer( FRAME_BUFFER, guiSAVEBUFFER, 0, 0, 640, 480 );
			PlayJA2SampleFromFile("SOUNDS/Laptop power down (8-11).wav", HIGHVOLUME, 1, MIDDLEPAN);
			while( iRealPercentage > 0  )
			{
				BlitBufferToBuffer( guiEXTRABUFFER, FRAME_BUFFER, 0, 0, 640, 480 );

				uiCurrTime = GetJA2Clock();
				iPercentage = (uiCurrTime-uiStartTime) * 100 / uiTimeRange;
				iPercentage = min( iPercentage, 100 );
				iPercentage = 100 - iPercentage;

				iRealPercentage = iPercentage;

				//Factor the percentage so that it is modified by a gravity falling acceleration effect.
				iFactor = (iPercentage - 50) * 2;
				if( iPercentage < 50 )
					iPercentage = (UINT32)(iPercentage + iPercentage * iFactor * 0.01 + 0.5);
				else
					iPercentage = (UINT32)(iPercentage + (100-iPercentage) * iFactor * 0.01 + 0.5);

				//Mapscreen source rect
				SrcRect1.iLeft = 464 * iPercentage / 100;
				SrcRect1.iRight = 640 - 163 * iPercentage / 100;
				SrcRect1.iTop = 417 * iPercentage / 100;
				SrcRect1.iBottom = 480 - 55 * iPercentage / 100;
				//Laptop source rect
				if( iPercentage < 99 )
					iScalePercentage = 10000 / (100-iPercentage);
				else
					iScalePercentage = 5333;
				iWidth = 12 * iScalePercentage / 100;
				iHeight = 9 * iScalePercentage / 100;
				iX = 472 - (472-320) * iScalePercentage / 5333;
				iY = 424 - (424-240) * iScalePercentage / 5333;

				SrcRect2.iLeft = iX - iWidth / 2;
				SrcRect2.iRight = SrcRect2.iLeft + iWidth;
				SrcRect2.iTop = iY - iHeight / 2;
				SrcRect2.iBottom = SrcRect2.iTop + iHeight;
				//SrcRect2.iLeft = 464 - 464 * iScalePercentage / 100;
				//SrcRect2.iRight = 477 + 163 * iScalePercentage / 100;
				//SrcRect2.iTop = 417 - 417 * iScalePercentage / 100;
				//SrcRect2.iBottom = 425 + 55 * iScalePercentage / 100;

				//BltStretchVideoSurface(FRAME_BUFFER, guiEXTRABUFFER, &SrcRect1, &DstRect);

				//SetFont( FONT10ARIAL );
				//SetFontForeground( FONT_YELLOW );
				//SetFontShadow( FONT_NEARBLACK );
				//mprintf( 10, 10, L"%d -> %d", iRealPercentage, iPercentage );
				//pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
				//SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, 640, 480 );
				//RectangleDraw( TRUE, SrcRect1.iLeft, SrcRect1.iTop, SrcRect1.iRight, SrcRect1.iBottom, Get16BPPColor( FROMRGB( 255, 100, 0 ) ), pDestBuf );
				//RectangleDraw( TRUE, SrcRect2.iLeft, SrcRect2.iTop, SrcRect2.iRight, SrcRect2.iBottom, Get16BPPColor( FROMRGB( 100, 255, 0 ) ), pDestBuf );
				//UnLockVideoSurface( FRAME_BUFFER );

				BltStretchVideoSurface(FRAME_BUFFER, guiSAVEBUFFER, &DstRect, &SrcRect2);
				InvalidateScreen();
				RefreshScreen();
			}
		}
	}
	return( TRUE );

}


static BOOLEAN HandleExit(void)
{
//	static BOOLEAN fSentImpWarningAlready = FALSE;


	// remind player about IMP
	if ( LaptopSaveInfo.gfNewGameLaptop != 0 )
	{
		if ( !AnyMercsHired( ) )
		{
			//AddEmail(0,1, GAME_HELP, GetWorldTotalMin( ) );
			//fExitingLaptopFlag = FALSE;
			//return( FALSE );
		}
	}

	// new game, send email
	if ( LaptopSaveInfo.gfNewGameLaptop != 0 )
	{
		// Set an event to send this email ( day 2 8:00-12:00 )
		if( ( LaptopSaveInfo.fIMPCompletedFlag == FALSE ) && ( LaptopSaveInfo.fSentImpWarningAlready == FALSE ) )
		{
			AddFutureDayStrategicEvent( EVENT_HAVENT_MADE_IMP_CHARACTER_EMAIL, (8 + Random(4) ) * 60, 0, 1 );

/*
 Moved to an event that gets triggered the next day: HaventMadeImpMercEmailCallBack()

			LaptopSaveInfo.fSentImpWarningAlready = TRUE;
			AddEmail(IMP_EMAIL_AGAIN,IMP_EMAIL_AGAIN_LENGTH,1, GetWorldTotalMin( ) );
*/
			fExitingLaptopFlag = TRUE;

		  return( FALSE );
		}
	}


	return ( TRUE );

}

void HaventMadeImpMercEmailCallBack()
{
	//if the player STILL hasnt made an imp merc yet
	if( ( LaptopSaveInfo.fIMPCompletedFlag == FALSE ) && ( LaptopSaveInfo.fSentImpWarningAlready == FALSE ) )
	{
		LaptopSaveInfo.fSentImpWarningAlready = TRUE;
		AddEmail(IMP_EMAIL_AGAIN,IMP_EMAIL_AGAIN_LENGTH,1, GetWorldTotalMin( ) );
	}
}


static BOOLEAN CreateLapTopMouseRegions(void)
{
 // define regions


 // the entire laptop display region
	MSYS_DefineRegion(&gLapTopScreenRegion, LaptopScreenRect.iLeft, LaptopScreenRect.iTop, LaptopScreenRect.iRight, LaptopScreenRect.iBottom, MSYS_PRIORITY_NORMAL + 1, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LapTopScreenCallBack);
 return (TRUE);
}


static BOOLEAN DeleteLapTopMouseRegions(void)
{

 MSYS_RemoveRegion( &gLapTopScreenRegion);

 return (TRUE);
}


static void UpdateListToReflectNewProgramOpened(INT32 iOpenedProgram);


static void FinancialRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
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


static void PersonnelRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
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


static void EmailRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		gfShowBookmarks = FALSE;
		UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_MAILER);
		fReDrawScreenFlag = TRUE;
	}
}


static void WWWRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
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

		if (guiCurrentWWWMode != LAPTOP_MODE_NONE)
			guiCurrentLaptopMode = guiCurrentWWWMode;
		else
			guiCurrentLaptopMode = LAPTOP_MODE_WWW;

		UpdateListToReflectNewProgramOpened(LAPTOP_PROGRAM_WEB_BROWSER);
		fReDrawScreenFlag = TRUE;
	}
  else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		if (guiCurrentWWWMode != LAPTOP_MODE_NONE)
			guiCurrentLaptopMode = guiCurrentWWWMode;
		else
			guiCurrentLaptopMode = LAPTOP_MODE_WWW;

		fReDrawScreenFlag = TRUE;
	}
}


static void HistoryRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
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


static void FilesRegionButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
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
	memset( LaptopSaveInfo.iBookMarkList, -1, sizeof( LaptopSaveInfo.iBookMarkList ) );
}

void SetBookMark(INT32 iBookId)
{
	// find first empty spot, set to iBookId
  INT32 iCounter=0;
  if(iBookId!=-2)
	{
	 while(LaptopSaveInfo.iBookMarkList[iCounter]!=-1)
	 {
		 // move trhough list until empty
		 if( LaptopSaveInfo.iBookMarkList[iCounter]==iBookId)
		 {
			 // found it, return
			 return;
		 }
		 iCounter++;
	 }
	 LaptopSaveInfo.iBookMarkList[iCounter]=iBookId;
	}
}


static BOOLEAN LoadBookmark(void)
{
		// grab download bars too
	guiDOWNLOADTOP = AddVideoObjectFromFile("LAPTOP/downloadtop.sti");
	CHECKF(guiDOWNLOADTOP != NO_VOBJECT);
	guiDOWNLOADMID = AddVideoObjectFromFile("LAPTOP/downloadmid.sti");
	CHECKF(guiDOWNLOADMID != NO_VOBJECT);
	guiDOWNLOADBOT = AddVideoObjectFromFile("LAPTOP/downloadbot.sti");
	CHECKF(guiDOWNLOADBOT != NO_VOBJECT);
	guiBOOKMARK = AddVideoObjectFromFile("LAPTOP/webpages.sti");
	CHECKF(guiBOOKMARK != NO_VOBJECT);
	guiBOOKHIGH = AddVideoObjectFromFile("LAPTOP/hilite.sti");
	CHECKF(guiBOOKHIGH != NO_VOBJECT);
	return( TRUE );
}


static void DisplayBookMarks(void)
{
	// will look at bookmarklist and set accordingly
	INT32 iCounter=1;
   // laptop icons
	INT16 sX, sY;


	// check if we are maximizing or minimizing.. if so, do not display
	if( ( fMaximizingProgram == TRUE ) || ( fMinizingProgram == TRUE ) )
	{
		return;
	}


	// font stuff
	SetFont(BOOK_FONT);
	SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

	// set buffer
	SetFontDestBuffer(FRAME_BUFFER ,BOOK_X, BOOK_TOP_Y,BOOK_X+BOOK_WIDTH-10,480, FALSE);


	// blt in book mark background
	while( LaptopSaveInfo.iBookMarkList[iCounter-1]!=-1)
	{

   if(iHighLightBookLine==iCounter-1)
	 {
			BltVideoObjectFromIndex(FRAME_BUFFER, guiBOOKHIGH, 0, BOOK_X, BOOK_TOP_Y + iCounter * (BOOK_HEIGHT + 6) + 6);
	 }
	 else
	 {
			BltVideoObjectFromIndex(FRAME_BUFFER, guiBOOKMARK, 0, BOOK_X, BOOK_TOP_Y+ iCounter * (BOOK_HEIGHT + 6) + 6);
	 }

	 if( iHighLightBookLine==iCounter-1 )
	 {
	   // blit in text
     SetFontForeground(FONT_WHITE);
     SetFontBackground(FONT_BLACK);
	 }
	 else
	 {
     // blit in text
     SetFontForeground(FONT_BLACK);
     SetFontBackground(FONT_BLACK);
	 }

	 FindFontCenterCoordinates(BOOK_X + 3, (UINT16)(BOOK_TOP_Y+2+(iCounter*( BOOK_HEIGHT + 6 ) ) + 6),BOOK_WIDTH - 3,BOOK_HEIGHT+6, pBookMarkStrings[ LaptopSaveInfo.iBookMarkList[iCounter-1] ] , BOOK_FONT, &sX, &sY );

	 mprintf(sX, sY,pBookMarkStrings[ LaptopSaveInfo.iBookMarkList[iCounter-1] ] );
	 	 iCounter++;
	}

	// blit one more


	 if(iHighLightBookLine==iCounter-1)
	 {
			BltVideoObjectFromIndex(FRAME_BUFFER, guiBOOKHIGH, 0, BOOK_X, BOOK_TOP_Y + iCounter * (BOOK_HEIGHT + 6) + 6);
	 }
	 else
	 {
			BltVideoObjectFromIndex(FRAME_BUFFER, guiBOOKMARK, 0, BOOK_X, BOOK_TOP_Y + iCounter * (BOOK_HEIGHT + 6) + 6);
	 }

	 if( iHighLightBookLine==iCounter-1 )
	 {
	   // blit in text
     SetFontForeground(FONT_WHITE);
     SetFontBackground(FONT_BLACK);
	 }
	 else
	 {
     // blit in text
     SetFontForeground(FONT_BLACK);
     SetFontBackground(FONT_BLACK);
	 }
	 FindFontCenterCoordinates(BOOK_X + 3, (UINT16)(BOOK_TOP_Y+2+(iCounter*( BOOK_HEIGHT + 6 ) ) + 6),BOOK_WIDTH - 3,BOOK_HEIGHT+6,pBookMarkStrings[ CANCEL_STRING ] , BOOK_FONT, &sX, &sY );
	 mprintf(sX, sY,pBookMarkStrings[CANCEL_STRING] );
	 	 iCounter++;

	 SetFontDestBuffer(FRAME_BUFFER,0,0,640,480, FALSE);

	InvalidateRegion(BOOK_X, BOOK_TOP_Y + iCounter * BOOK_HEIGHT + 12, BOOK_X + BOOK_WIDTH, BOOK_TOP_Y + (iCounter + 1) * BOOK_HEIGHT + 16);
	SetFontShadow(DEFAULT_SHADOW);


	InvalidateRegion(BOOK_X, BOOK_TOP_Y, BOOK_X+BOOK_WIDTH, BOOK_TOP_Y+(iCounter + 6 )*BOOK_HEIGHT+16);
	return;
}


static void DeleteBookmark(void)
{
	DeleteVideoObjectFromIndex(guiBOOKHIGH);
  DeleteVideoObjectFromIndex(guiBOOKMARK);

	DeleteVideoObjectFromIndex(guiDOWNLOADTOP);
	DeleteVideoObjectFromIndex(guiDOWNLOADMID);
	DeleteVideoObjectFromIndex(guiDOWNLOADBOT);
}


static void BookmarkCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void BookmarkMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void CreateBookMarkHelpText(MOUSE_REGION* pRegion, UINT32 uiBookMarkID);


static void CreateBookMarkMouseRegions(void)
{
	INT32 iCounter=0;
	// creates regions based on number of entries
  while( LaptopSaveInfo.iBookMarkList[iCounter] != -1 )
	{
	  MSYS_DefineRegion(&gBookmarkMouseRegions[iCounter],(INT16)BOOK_X, (UINT16)(BOOK_TOP_Y+((iCounter+1)*(BOOK_HEIGHT+6))+6), BOOK_X+BOOK_WIDTH ,(INT16)(BOOK_TOP_Y+((iCounter+2)*(BOOK_HEIGHT+6))+6),
			MSYS_PRIORITY_HIGHEST - 2,CURSOR_LAPTOP_SCREEN, BookmarkMvtCallBack, BookmarkCallBack );
		MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter],0,iCounter);
		MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter],1,0);

		//Create the regions help text
		CreateBookMarkHelpText( &gBookmarkMouseRegions[iCounter], LaptopSaveInfo.iBookMarkList[iCounter] );

	  iCounter++;
	}
	// now add one more
	// for the cancel button
   MSYS_DefineRegion(&gBookmarkMouseRegions[iCounter],(INT16)BOOK_X, (UINT16)(BOOK_TOP_Y+((iCounter+1)*(BOOK_HEIGHT+6))+6), BOOK_X+BOOK_WIDTH ,(INT16)(BOOK_TOP_Y+((iCounter+2)*(BOOK_HEIGHT+6))+6),
			MSYS_PRIORITY_HIGHEST - 2,CURSOR_LAPTOP_SCREEN, BookmarkMvtCallBack, BookmarkCallBack );
	 MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter],0,iCounter);
   MSYS_SetRegionUserData(&gBookmarkMouseRegions[iCounter],1,CANCEL_STRING );
}


static void DeleteBookmarkRegions(void)
{
	INT32 iCounter=0;
	//deletes bookmark regions
  while( LaptopSaveInfo.iBookMarkList[iCounter] != -1 )
	{
    MSYS_RemoveRegion(&gBookmarkMouseRegions[iCounter]);
	  iCounter++;
	}


	// now one for the cancel
	MSYS_RemoveRegion(&gBookmarkMouseRegions[iCounter]);


}


static void CreateDestoryBookMarkRegions(void)
{
	// checks to see if a bookmark needs to be created or destroyed
	static BOOLEAN fOldShowBookmarks=FALSE;

	if((gfShowBookmarks)&&(!fOldShowBookmarks))
	{
		// create regions
    CreateBookMarkMouseRegions( );
	  fOldShowBookmarks=TRUE;
	}
	else if((!gfShowBookmarks)&&(fOldShowBookmarks))
	{
    // destroy bookmarks
		DeleteBookmarkRegions( );
    fOldShowBookmarks=FALSE;
	}


}


static void BookmarkCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
 INT32 iCount;

 // we are in process of loading
 if( fLoadPendingFlag == TRUE )
 {
	 return;
 }

 if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
 {
	 iCount=MSYS_GetRegionUserData(pRegion, 0);
	 if( MSYS_GetRegionUserData(pRegion, 1) == CANCEL_STRING )
	 {
		 gfShowBookmarks = FALSE;
		 fReDrawScreenFlag = TRUE;
	 }
   if( LaptopSaveInfo.iBookMarkList[iCount] != -1 )
	 {
		 GoToWebPage( LaptopSaveInfo.iBookMarkList[iCount] );
	 }
	 else
	 {
		 return;
	 }
 }
 else if(iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
 {
   iCount=MSYS_GetRegionUserData(pRegion, 0);

 }
}


static void InternetRainDelayMessageBoxCallBack(UINT8 bExitValue);


void GoToWebPage(INT32 iPageId )
{

	//if it is raining, popup a warning first saying connection time may be slow
	if( IsItRaining() )
	{
		if( giRainDelayInternetSite == -1 )
		{
			DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, pErrorStrings[0], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, InternetRainDelayMessageBoxCallBack);
			giRainDelayInternetSite = iPageId;
			return;
		}
	}
	else
		giRainDelayInternetSite = -1;

	switch(iPageId)
	{
		case AIM_BOOKMARK:
			guiCurrentWWWMode=LAPTOP_MODE_AIM;
		  guiCurrentLaptopMode=LAPTOP_MODE_AIM;

			// do we have to have a World Wide Wait
			if( LaptopSaveInfo.fVisitedBookmarkAlready[ AIM_BOOKMARK ] == FALSE )
			{
        // reset flag and set load pending flag
				LaptopSaveInfo.fVisitedBookmarkAlready[ AIM_BOOKMARK ] = TRUE;
        fLoadPendingFlag = TRUE;
			}
			else
			{
				// fast reload
        fLoadPendingFlag = TRUE;
				fFastLoadFlag =  TRUE;
			}
		break;
		case BOBBYR_BOOKMARK:
			guiCurrentWWWMode=LAPTOP_MODE_BOBBY_R;
		  guiCurrentLaptopMode=LAPTOP_MODE_BOBBY_R;

				// do we have to have a World Wide Wait
			if( LaptopSaveInfo.fVisitedBookmarkAlready[ BOBBYR_BOOKMARK ] == FALSE )
			{
        // reset flag and set load pending flag
				LaptopSaveInfo.fVisitedBookmarkAlready[ BOBBYR_BOOKMARK ] = TRUE;
        fLoadPendingFlag = TRUE;
			}
			else
			{
				// fast reload
        fLoadPendingFlag = TRUE;
				fFastLoadFlag =  TRUE;
			}
		break;
		case( IMP_BOOKMARK ):
		  guiCurrentWWWMode=LAPTOP_MODE_CHAR_PROFILE;
		  guiCurrentLaptopMode=LAPTOP_MODE_CHAR_PROFILE;

			// do we have to have a World Wide Wait
			if( LaptopSaveInfo.fVisitedBookmarkAlready[ IMP_BOOKMARK ] == FALSE )
			{
        // reset flag and set load pending flag
				LaptopSaveInfo.fVisitedBookmarkAlready[ IMP_BOOKMARK ] = TRUE;
        fLoadPendingFlag = TRUE;
			}
			else
			{
				// fast reload
        fLoadPendingFlag = TRUE;
				fFastLoadFlag =  TRUE;
			}
			iCurrentImpPage = IMP_HOME_PAGE;
		break;
		case( MERC_BOOKMARK ):

			//if the mercs server has gone down, but hasnt come up yet
			if( LaptopSaveInfo.fMercSiteHasGoneDownYet == TRUE && LaptopSaveInfo.fFirstVisitSinceServerWentDown == FALSE )
			{
				guiCurrentWWWMode = LAPTOP_MODE_BROKEN_LINK;
				guiCurrentLaptopMode = LAPTOP_MODE_BROKEN_LINK;
			}
			else
			{
				guiCurrentWWWMode = LAPTOP_MODE_MERC;
				guiCurrentLaptopMode = LAPTOP_MODE_MERC;
			}


			// do we have to have a World Wide Wait
			if( LaptopSaveInfo.fVisitedBookmarkAlready[ MERC_BOOKMARK ] == FALSE )
			{
        // reset flag and set load pending flag
				LaptopSaveInfo.fVisitedBookmarkAlready[ MERC_BOOKMARK ] = TRUE;
        fLoadPendingFlag = TRUE;
			}
			else
			{
				// fast reload
        fLoadPendingFlag = TRUE;
				fFastLoadFlag =  TRUE;
			}
		break;
		case( FUNERAL_BOOKMARK ):
		  guiCurrentWWWMode=LAPTOP_MODE_FUNERAL;
		  guiCurrentLaptopMode=LAPTOP_MODE_FUNERAL;

			// do we have to have a World Wide Wait
			if( LaptopSaveInfo.fVisitedBookmarkAlready[ FUNERAL_BOOKMARK ] == FALSE )
			{
        // reset flag and set load pending flag
				LaptopSaveInfo.fVisitedBookmarkAlready[ FUNERAL_BOOKMARK ] = TRUE;
        fLoadPendingFlag = TRUE;
			}
			else
			{
				// fast reload
        fLoadPendingFlag = TRUE;
				fFastLoadFlag =  TRUE;
			}
		break;
		case( FLORIST_BOOKMARK ):
		  guiCurrentWWWMode=LAPTOP_MODE_FLORIST;
		  guiCurrentLaptopMode=LAPTOP_MODE_FLORIST;

			// do we have to have a World Wide Wait
			if( LaptopSaveInfo.fVisitedBookmarkAlready[ FLORIST_BOOKMARK ] == FALSE )
			{
        // reset flag and set load pending flag
				LaptopSaveInfo.fVisitedBookmarkAlready[ FLORIST_BOOKMARK ] = TRUE;
        fLoadPendingFlag = TRUE;
			}
			else
			{
				// fast reload
        fLoadPendingFlag = TRUE;
				fFastLoadFlag =  TRUE;
			}
		break;

		case( INSURANCE_BOOKMARK ):
		  guiCurrentWWWMode=LAPTOP_MODE_INSURANCE;
		  guiCurrentLaptopMode=LAPTOP_MODE_INSURANCE;

			// do we have to have a World Wide Wait
			if( LaptopSaveInfo.fVisitedBookmarkAlready[ INSURANCE_BOOKMARK ] == FALSE )
			{
        // reset flag and set load pending flag
				LaptopSaveInfo.fVisitedBookmarkAlready[ INSURANCE_BOOKMARK ] = TRUE;
        fLoadPendingFlag = TRUE;
			}
			else
			{
				// fast reload
        fLoadPendingFlag = TRUE;
				fFastLoadFlag =  TRUE;
			}
		break;

	}

  gfShowBookmarks=FALSE;
	fReDrawScreenFlag=TRUE;
}


static void BookmarkMvtCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason == MSYS_CALLBACK_REASON_MOVE)
	{
	 iHighLightBookLine=MSYS_GetRegionUserData(pRegion, 0);
	}
  if (iReason == MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
    iHighLightBookLine=-1;
	}
}


static BOOLEAN LoadLoadPending(void)
{
	// function will load the load pending graphics
	// reuse bookmark
	// load graph window and bar
	guiGRAPHWINDOW = AddVideoObjectFromFile("LAPTOP/graphwindow.sti");
	CHECKF(guiGRAPHWINDOW != NO_VOBJECT);
	guiGRAPHBAR = AddVideoObjectFromFile("LAPTOP/graphsegment.sti");
	CHECKF(guiGRAPHBAR != NO_VOBJECT);
	return (TRUE);
}


static INT32 WWaitDelayIncreasedIfRaining(INT32 iUnitTime);


static BOOLEAN DisplayLoadPending(void)
{
	// this function will display the load pending and return if the load is done
  static INT32 iBaseTime=0;
	static INT32 iTotalTime=0;
	INT32 iTempTime=0;
	INT32 iCounter=0;
	INT32 iDifference=0;
  INT32 iLoadTime;
	INT32 iUnitTime;
	INT16 sXPosition = 0, sYPosition = 0;

	// if merc webpage, make it longer
		//TEMP disables the loadpending
	if( gfTemporaryDisablingOfLoadPendingFlag )
	{
		iLoadTime=1;
		iUnitTime=1;
	}
	else
	{
		if( ( fFastLoadFlag == TRUE )&&( fConnectingToSubPage == TRUE ) )
		{
			iUnitTime=FASTEST_UNIT_TIME;
		}
		else if( fFastLoadFlag == TRUE )
		{
			iUnitTime=FAST_UNIT_TIME;
		}
		else if( fConnectingToSubPage == TRUE )
		{
			iUnitTime=ALMOST_FAST_UNIT_TIME;
		}

		//if we are connecting the MERC site, and the MERC site hasnt yet moved to their new site, have the sloooww wait
		else if(guiCurrentLaptopMode == LAPTOP_MODE_MERC && !LaptopSaveInfo.fMercSiteHasGoneDownYet )
		{
			iUnitTime=LONG_UNIT_TIME;
		}
		else
		{
			iUnitTime=UNIT_TIME;
		}

		iUnitTime += WWaitDelayIncreasedIfRaining( iUnitTime );

		iLoadTime = iUnitTime * 30;
	}


	// we are now waiting on a web page to download, reset counter
	if(!fLoadPendingFlag)
	{
	 fDoneLoadPending = FALSE;
	 fFastLoadFlag = FALSE;
	 fConnectingToSubPage = FALSE;
   iBaseTime=0;
   iTotalTime=0;
	 return (FALSE);
	}
	// if total time is exceeded, return (TRUE)
	if(iBaseTime==0)
	{
	 iBaseTime=GetJA2Clock();
	}



	if(iTotalTime >= iLoadTime)
	{
		// done loading, redraw screen
		fLoadPendingFlag  = FALSE;
		fFastLoadFlag = FALSE;
		iTotalTime=0;
		iBaseTime=0;
		fDoneLoadPending = TRUE;
		fConnectingToSubPage = FALSE;
		fPausedReDrawScreenFlag = TRUE;

		return(TRUE);
	}


	iDifference=GetJA2Clock()-iBaseTime;



	// difference has been long enough or we are redrawing the screen
	if( (iDifference) > iUnitTime)
	{
		// LONG ENOUGH TIME PASSED
		iCounter=0;
		iBaseTime=GetJA2Clock();
		iTotalTime+=iDifference;
    iTempTime=iTotalTime;

	}

	// new mail, don't redraw
	if( fNewMailFlag == TRUE )
	{
		return( FALSE );
	}

	RenderButtons( );

//	RenderFastHelp( );
//	RenderButtonsFastHelp( );


	// display top middle and bottom of box
	BltVideoObjectFromIndex(FRAME_BUFFER, guiDOWNLOADTOP,   0, DOWNLOAD_X,     DOWNLOAD_Y);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiDOWNLOADMID,   0, DOWNLOAD_X,     DOWNLOAD_Y + DOWN_HEIGHT);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiDOWNLOADBOT,   0, DOWNLOAD_X,     DOWNLOAD_Y + 2 * DOWN_HEIGHT);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiTITLEBARICONS, 1, DOWNLOAD_X + 4, DOWNLOAD_Y + 1);


  // font stuff
	SetFont(DOWNLOAD_FONT);
	SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);
  SetFontShadow(NO_SHADOW);

	// reload or download?
	if( fFastLoadFlag == TRUE )
  {
		FindFontCenterCoordinates( 328, 0, 446 - 328, 0, pDownloadString[1], DOWNLOAD_FONT, &sXPosition, &sYPosition );

    // display download string
		mprintf( sXPosition, DOWN_STRING_Y,pDownloadString[1]);
	}
	else
	{
		FindFontCenterCoordinates( 328, 0, 446 - 328, 0, pDownloadString[ 0 ], DOWNLOAD_FONT, &sXPosition, &sYPosition );

		// display download string
		mprintf( sXPosition, DOWN_STRING_Y,pDownloadString[0]);
	}

	BltVideoObjectFromIndex(FRAME_BUFFER, guiGRAPHWINDOW, 0, LAPTOP_WINDOW_X, LAPTOP_WINDOW_Y);

	// check to see if we are only updating screen, but not passed a new element in the load pending display


  iTempTime = iTotalTime;
	// decide how many time units are to be displayed, based on amount of time passed
	while(iTempTime >0)
	{
		BltVideoObjectFromIndex(FRAME_BUFFER, guiGRAPHBAR, 0, LAPTOP_BAR_X + UNIT_WIDTH * iCounter, LAPTOP_BAR_Y);
		iTempTime-=iUnitTime;
		iCounter++;

		// have we gone too far?
		if( iCounter > 30 )
		{
			iTempTime = 0;
		}
	}

	InvalidateRegion(DOWNLOAD_X, DOWNLOAD_Y, DOWNLOAD_X + 150, DOWNLOAD_Y + 100);

	// re draw screen and new mail warning box
  SetFontShadow(DEFAULT_SHADOW);

  MarkButtonsDirty( );

	DisableMercSiteButton();

	return (FALSE);
}


static void DeleteLoadPending(void)
{
	// this funtion will delete the load pending graphics
	// reuse bookmark
  DeleteVideoObjectFromIndex(guiGRAPHBAR);
  DeleteVideoObjectFromIndex(guiGRAPHWINDOW);
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
	InitIMPSubPageList( );
}


static void CheckMarkButtonsDirtyFlag(void)
{
  // this function checks the fMarkButtonsDirtyFlag, if true, mark buttons dirty
	if( fMarkButtonsDirtyFlag )
	{
    // flag set, mark buttons and reset
		MarkButtonsDirty( );
		fMarkButtonsDirtyFlag = FALSE;
	}
}


static void PostButtonRendering(void)
{
  // this function is in place to allow for post button rendering

	switch( guiCurrentLaptopMode )
  {
	  case LAPTOP_MODE_AIM:
//	    RenderCharProfilePostButton( );
		break;

	  case 	LAPTOP_MODE_AIM_MEMBERS:
			RenderAIMMembersTopLevel();
		break;
	}
}


static void ShouldNewMailBeDisplayed(void)
{
	switch (guiCurrentLaptopMode)
	{
		case LAPTOP_MODE_AIM_MEMBERS: DisableNewMailMessage(); break;
	}
}


static void CheckIfNewWWWW(void)
{
  // if no www mode, set new www flag..until new www mode that is not 0

	if( guiCurrentWWWMode == LAPTOP_MODE_NONE )
	{
    fNewWWW = TRUE;
	}
	else
	{
    fNewWWW = FALSE;
	}
}


static void HandleLapTopESCKey(void)
{

	// will handle esc key events, since handling depends on state of laptop


	if( fNewMailFlag )
	{
		// get rid of new mail warning box
	  fNewMailFlag=FALSE;
    CreateDestroyNewMailButton();

		// force redraw
	  fReDrawScreenFlag = TRUE;
	  RenderLaptop( );
	}
	else if (MailToDelete != NULL)
	{
    // get rid of delete mail box
		MailToDelete = NULL;
	  CreateDestroyDeleteNoticeMailButton();

		// force redraw
	  fReDrawScreenFlag = TRUE;
	  RenderLaptop( );
	}
	else if( gfShowBookmarks )
	{
		// get rid of bookmarks
		gfShowBookmarks = FALSE;

		// force redraw
	  fReDrawScreenFlag = TRUE;
		RenderLapTopImage( );
	  RenderLaptop( );
	}
	else
	{
    // leave
		fExitingLaptopFlag = TRUE;
		HandleExit( );
	}
}


void HandleRightButtonUpEvent( void )
{

	// will handle the right button up event
	if( fNewMailFlag )
	{
		// get rid of new mail warning box
	  fNewMailFlag=FALSE;
    CreateDestroyNewMailButton();

		// force redraw
	  fReDrawScreenFlag = TRUE;
	  RenderLaptop( );
	}
	else if (MailToDelete != NULL)
	{
    // get rid of delete mail box
		MailToDelete = NULL;
	  CreateDestroyDeleteNoticeMailButton();

		// force redraw
	  fReDrawScreenFlag = TRUE;
	  RenderLaptop( );
	}
	else if( gfShowBookmarks )
	{
		// get rid of bookmarks
		gfShowBookmarks = FALSE;

		// force redraw
	  fReDrawScreenFlag = TRUE;
		RenderLapTopImage( );
	  RenderLaptop( );
	}
	else if( fDisplayMessageFlag )
	{
		fDisplayMessageFlag = FALSE;

			// force redraw
	  fReDrawScreenFlag = TRUE;
		RenderLapTopImage( );
	  RenderLaptop( );

	}
	else if( fShowBookmarkInfo )
	{
		fShowBookmarkInfo = FALSE;
	}

}


static void HandleLeftButtonUpEvent(void)
{

	// will handle the left button up event

	if( gfShowBookmarks )
	{
		// get rid of bookmarks
		gfShowBookmarks = FALSE;

		// force redraw
	  fReDrawScreenFlag = TRUE;
		RenderLapTopImage( );
	  RenderLaptop( );
	}
	else if( fShowBookmarkInfo )
	{
		fShowBookmarkInfo = FALSE;
	}
}

void LapTopScreenCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		HandleLeftButtonUpEvent( );
	}
  if(iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
  {
    HandleRightButtonUpEvent( );
  }
}


BOOLEAN DoLapTopMessageBox( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT8 ubFlags, MSGBOX_CALLBACK ReturnCallback )
{
  SGPRect pCenteringRect= {LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y, LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_LR_Y };


	// reset exit mode
	fExitDueToMessageBox = TRUE;

	// do message box and return
	INT32 iLaptopMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen, ubFlags | MSG_BOX_FLAG_USE_CENTERING_RECT, ReturnCallback, &pCenteringRect);

	// send back return state
	return( ( iLaptopMessageBox != -1 ) );
}


BOOLEAN DoLapTopSystemMessageBoxWithRect( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, SGPRect *pCenteringRect )
{
	// reset exit mode
	fExitDueToMessageBox = TRUE;

	// do message box and return
	INT32 iLaptopMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen, usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT, ReturnCallback, pCenteringRect);

	// send back return state
	return( ( iLaptopMessageBox != -1 ) );
}

BOOLEAN DoLapTopSystemMessageBox( UINT8 ubStyle, const wchar_t *zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback )
{
  SGPRect CenteringRect= {0, 0, 640, INV_INTERFACE_START_Y };
	// reset exit mode
	fExitDueToMessageBox = TRUE;

	// do message box and return
	INT32 iLaptopMessageBox = DoMessageBox(ubStyle, zString, uiExitScreen, usFlags | MSG_BOX_FLAG_USE_CENTERING_RECT, ReturnCallback, &CenteringRect);

	// send back return state
	return( ( iLaptopMessageBox != -1 ) );
}



//places a tileable pattern down
BOOLEAN WebPageTileBackground(UINT8 ubNumX, UINT8 ubNumY, UINT16 usWidth, UINT16 usHeight, UINT32 uiBackgroundIdentifier)
{
	UINT16	x,y, uiPosX, uiPosY;

	HVOBJECT hBackGroundHandle = GetVideoObject(uiBackgroundIdentifier);

	uiPosY = LAPTOP_SCREEN_WEB_UL_Y;
	for(y=0; y<ubNumY; y++)
	{
		uiPosX = LAPTOP_SCREEN_UL_X;
		for(x=0; x<ubNumX; x++)
		{
		  BltVideoObject(FRAME_BUFFER, hBackGroundHandle, 0,uiPosX, uiPosY);
			uiPosX += usWidth;
		}
		uiPosY += usHeight;
	}
	return(TRUE);
}


static BOOLEAN InitTitleBarMaximizeGraphics(UINT32 uiBackgroundGraphic, const wchar_t* pTitle, UINT32 uiIconGraphic, UINT16 usIconGraphicIndex)
{
	Assert( uiBackgroundGraphic );

	// Create a background video surface to blt the title bar onto
	guiTitleBarSurface = AddVideoSurface(LAPTOP_TITLE_BAR_WIDTH, LAPTOP_TITLE_BAR_HEIGHT, PIXEL_DEPTH);
	CHECKF(guiTitleBarSurface != NO_VSURFACE);

	BltVideoObjectFromIndex(guiTitleBarSurface, uiBackgroundGraphic, 0, 0, 0);
	BltVideoObjectFromIndex(guiTitleBarSurface, uiIconGraphic, usIconGraphicIndex, LAPTOP_TITLE_BAR_ICON_OFFSET_X, LAPTOP_TITLE_BAR_ICON_OFFSET_Y);

	SetFontDestBuffer(guiTitleBarSurface, 0, 0, LAPTOP_TITLE_BAR_WIDTH, LAPTOP_TITLE_BAR_HEIGHT, FALSE);
	DrawTextToScreen(pTitle, LAPTOP_TITLE_BAR_TEXT_OFFSET_X, LAPTOP_TITLE_BAR_TEXT_OFFSET_Y, 0, FONT14ARIAL, FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);
	SetFontDestBuffer( FRAME_BUFFER, 0, 0, 640, 480, FALSE);

	return( TRUE );
}


static BOOLEAN DisplayTitleBarMaximizeGraphic(BOOLEAN fForward, BOOLEAN fInit, UINT16 usTopLeftX, UINT16 usTopLeftY, UINT16 usTopRightX)
{
	static 	INT8			ubCount;
	INT16		sPosX, sPosY, sPosRightX, sPosBottomY, sWidth, sHeight;
	SGPRect		SrcRect;
	SGPRect		DestRect;
	static SGPRect		LastRect;
	FLOAT dTemp;


	if( fInit )
	{
		if( gfTitleBarSurfaceAlreadyActive )
					return(FALSE);

		gfTitleBarSurfaceAlreadyActive=TRUE;
		if( fForward )
		{
			ubCount = 1;
		}
		else
		{
			ubCount = NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS - 1;
		}
	}

		dTemp = (LAPTOP_TITLE_BAR_TOP_LEFT_X - usTopLeftX) / (FLOAT)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
		sPosX = (INT16)(usTopLeftX + dTemp * ubCount);

		dTemp = (LAPTOP_TITLE_BAR_TOP_RIGHT_X - usTopRightX) / (FLOAT)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
		sPosRightX = (INT16)(usTopRightX + dTemp * ubCount);

		dTemp = ( LAPTOP_TITLE_BAR_TOP_LEFT_Y - usTopLeftY ) / (FLOAT)NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS;
		sPosY =  (INT16)(usTopLeftY + dTemp * ubCount);


	sPosBottomY = LAPTOP_TITLE_BAR_HEIGHT;


	SrcRect.iLeft = 0;
	SrcRect.iTop = 0;
	SrcRect.iRight = LAPTOP_TITLE_BAR_WIDTH;
	SrcRect.iBottom = LAPTOP_TITLE_BAR_HEIGHT;

	//if its the last fram, bit the tittle bar to the final position
	if( ubCount == NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS)
	{
		DestRect.iLeft = LAPTOP_TITLE_BAR_TOP_LEFT_X;
		DestRect.iTop = LAPTOP_TITLE_BAR_TOP_LEFT_Y;
		DestRect.iRight = LAPTOP_TITLE_BAR_TOP_RIGHT_X;
		DestRect.iBottom = DestRect.iTop + sPosBottomY;
	}
	else
	{
		DestRect.iLeft = sPosX;
		DestRect.iTop = sPosY;
		DestRect.iRight = sPosRightX;
		DestRect.iBottom = DestRect.iTop + sPosBottomY;
	}



	if( fForward )
	{
		//Restore the old rect
		if( ubCount > 1 )
		{
			sWidth = (UINT16)(LastRect.iRight - LastRect.iLeft);
			sHeight = (UINT16)(LastRect.iBottom - LastRect.iTop);
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, LastRect.iLeft, LastRect.iTop, sWidth, sHeight);
		}

		//Save rectangle
		if( ubCount > 0 )
		{
			sWidth = (UINT16)(DestRect.iRight - DestRect.iLeft);
			sHeight = (UINT16)(DestRect.iBottom - DestRect.iTop);
			BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.iLeft, DestRect.iTop, sWidth, sHeight);
		}
	}
	else
	{
		//Restore the old rect
		if( ubCount < NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS - 1 )
		{
			sWidth = (UINT16)(LastRect.iRight - LastRect.iLeft);
			sHeight = (UINT16)(LastRect.iBottom - LastRect.iTop);
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, LastRect.iLeft, LastRect.iTop, sWidth, sHeight);
		}

		//Save rectangle
		if( ubCount < NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS )
		{
			sWidth = (UINT16)(DestRect.iRight - DestRect.iLeft);
			sHeight = (UINT16)(DestRect.iBottom - DestRect.iTop);
			BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.iLeft, DestRect.iTop, sWidth, sHeight);
		}
	}

	BltStretchVideoSurface(FRAME_BUFFER, guiTitleBarSurface, &SrcRect, &DestRect);

	InvalidateRegion(DestRect.iLeft,DestRect.iTop, DestRect.iRight, DestRect.iBottom);
	InvalidateRegion(LastRect.iLeft,LastRect.iTop, LastRect.iRight, LastRect.iBottom);

	LastRect = DestRect;

	if( fForward )
	{
		if( ubCount == NUMBER_OF_LAPTOP_TITLEBAR_ITERATIONS)
		{
			gfTitleBarSurfaceAlreadyActive=FALSE;
			return(TRUE);
		}
		else
		{
			ubCount++;
			return(FALSE);
		}
	}
	else
	{
		if( ubCount == 0)
		{
			gfTitleBarSurfaceAlreadyActive=FALSE;
			return(TRUE);
		}
		else
		{
			ubCount--;
			return(FALSE);
		}
	}

	return(TRUE);
}


static void RemoveTitleBarMaximizeGraphics(void)
{
	DeleteVideoSurfaceFromIndex( guiTitleBarSurface );
}


static void HandleSlidingTitleBar(void)
{
	if( ( fMaximizingProgram == FALSE ) && ( fMinizingProgram == FALSE ) )
	{
		return;
	}

	if( fExitingLaptopFlag )
	{
		return;
	}

	if( fMaximizingProgram )
	{
		switch( bProgramBeingMaximized )
		{
			case( LAPTOP_PROGRAM_MAILER ):
			 fMaximizingProgram = !DisplayTitleBarMaximizeGraphic( TRUE, fInitTitle, 29, 66, 29 + 20 );
			 if( fMaximizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_FILES ):
			 fMaximizingProgram = !DisplayTitleBarMaximizeGraphic( TRUE, fInitTitle, 29, 120, 29 + 20 );
			 if( fMaximizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_FINANCES ):
			 fMaximizingProgram = !DisplayTitleBarMaximizeGraphic( TRUE, fInitTitle, 29, 226, 29 + 20 );
			 if( fMaximizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_PERSONNEL ):
			 fMaximizingProgram = !DisplayTitleBarMaximizeGraphic( TRUE, fInitTitle, 29,  194, 29 + 20 );
			 if( fMaximizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_HISTORY ):
			 fMaximizingProgram = !DisplayTitleBarMaximizeGraphic( TRUE, fInitTitle, 29,  162, 29 + 20 );
			 if( fMaximizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			  case( LAPTOP_PROGRAM_WEB_BROWSER ):
			 fMaximizingProgram = !DisplayTitleBarMaximizeGraphic( TRUE, fInitTitle, 29,  99, 29 + 20 );
			 if( fMaximizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 fEnteredNewLapTopDueToHandleSlidingBars = TRUE;
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
		}

		MarkButtonsDirty( );
	}
	else
	{
		// minimizing
		switch( bProgramBeingMaximized )
		{
			case( LAPTOP_PROGRAM_MAILER ):
			 fMinizingProgram = !DisplayTitleBarMaximizeGraphic( FALSE, fInitTitle, 29, 66, 29 + 20 );
			 if( fMinizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_FILES ):
			 fMinizingProgram = !DisplayTitleBarMaximizeGraphic( FALSE, fInitTitle, 29, 130, 29 + 20 );
			 if( fMinizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_FINANCES ):
			 fMinizingProgram = !DisplayTitleBarMaximizeGraphic( FALSE, fInitTitle, 29, 226, 29 + 20 );
			 if( fMinizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_PERSONNEL ):
			 fMinizingProgram = !DisplayTitleBarMaximizeGraphic( FALSE, fInitTitle, 29,  194, 29 + 20 );
			 if( fMinizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			 case( LAPTOP_PROGRAM_HISTORY ):
			 fMinizingProgram = !DisplayTitleBarMaximizeGraphic( FALSE, fInitTitle, 29,  162 , 29 + 20 );
			 if( fMinizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
			  case( LAPTOP_PROGRAM_WEB_BROWSER ):
			 fMinizingProgram = !DisplayTitleBarMaximizeGraphic( FALSE, fInitTitle, 29,  99 , 29 + 20 );
			 if( fMinizingProgram  == FALSE )
			 {
				 RemoveTitleBarMaximizeGraphics();
				 EnterNewLaptopMode();
				 fEnteredNewLapTopDueToHandleSlidingBars = FALSE;
				 fPausedReDrawScreenFlag = TRUE;
			 }
			 break;
		}
	}


	// reset init
	fInitTitle = FALSE;
}


static void ShowLights(void)
{
	// will show lights depending on state
	BltVideoObjectFromIndex(FRAME_BUFFER, guiLIGHTS, fPowerLightOn     ? 0 : 1, 44, 466);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiLIGHTS, fHardDriveLightOn ? 0 : 1, 88, 466);
}


static void FlickerHDLight(void)
{
	static INT32 iBaseTime = 0;
	static INT32 iTotalDifference = 0;
	INT32 iDifference = 0;

	if( fLoadPendingFlag == TRUE )
	{
		fFlickerHD = TRUE;
	}

	if( fFlickerHD == FALSE )
	{
		return;
	}


	if( iBaseTime == 0 )
	{
		iBaseTime = GetJA2Clock( );
	}

	iDifference = GetJA2Clock( ) - iBaseTime;

	if( ( iTotalDifference > HD_FLICKER_TIME ) && ( fLoadPendingFlag == FALSE ) )
	{
		iBaseTime = GetJA2Clock( );
		fHardDriveLightOn = FALSE;
		iBaseTime = 0;
		iTotalDifference = 0;
		fFlickerHD = FALSE;
		InvalidateRegion(88, 466, 102, 477 );
		return;
	}

	if( iDifference > FLICKER_TIME )
	{
		iTotalDifference += iDifference;

		if( fLoadPendingFlag == TRUE )
		{
			iTotalDifference = 0;
		}

		if( ( Random( 2 ) ) == 0 )
		{
		  fHardDriveLightOn = TRUE;
		}
	  else
		{
		  fHardDriveLightOn = FALSE;
		}
		InvalidateRegion(88, 466, 102, 477 );
	}
}


static BOOLEAN ExitLaptopDone(void)
{
	// check if this is the first time, to reset counter

	static BOOLEAN fOldLeaveLaptopState = FALSE;
	static INT32 iBaseTime = 0;
	INT32 iDifference = 0;

	if( fOldLeaveLaptopState == FALSE )
	{
		fOldLeaveLaptopState = TRUE;
		iBaseTime = GetJA2Clock( );
	}

	fPowerLightOn = FALSE;


  InvalidateRegion(44, 466, 58, 477 );
	// get the current difference
	iDifference = GetJA2Clock( ) - iBaseTime;


	// did we wait long enough?
	if( iDifference > EXIT_LAPTOP_DELAY_TIME )
	{
		iBaseTime = 0;
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
	// check to see if created, if so, do nothing


	// check current mode
	if( ( guiCurrentLaptopMode == LAPTOP_MODE_NONE )&&( guiPreviousLaptopMode != LAPTOP_MODE_NONE ) )
	{
		fCreateMinimizeButton = FALSE;
	}
	else if( ( guiCurrentLaptopMode != LAPTOP_MODE_NONE ) )
	{
		fCreateMinimizeButton = TRUE;
	}
	else if( ( guiPreviousLaptopMode != LAPTOP_MODE_NONE ) )
	{
		fCreateMinimizeButton = FALSE;
	}

	// leaving laptop, get rid of the button
	if( fExitingLaptopFlag == TRUE )
	{
		fCreateMinimizeButton = FALSE;
	}


	if( ( fAlreadyCreated == FALSE ) && ( fCreateMinimizeButton == TRUE ) )
	{
		// not created, create
		fAlreadyCreated = TRUE;
		CreateMinimizeButtonForCurrentMode( );
		CreateMinimizeRegionsForLaptopProgramIcons( );
	}
	else if(  ( fAlreadyCreated == TRUE ) && ( fCreateMinimizeButton == FALSE )  )
	{
		// created and must be destroyed
		fAlreadyCreated = FALSE;
		DestroyMinimizeButtonForCurrentMode( );
		DestroyMinimizeRegionsForLaptopProgramIcons( );

	}
	else
	{
		// do nothing
	}
}


static void LaptopMinimizeProgramButtonCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateMinimizeButtonForCurrentMode(void)
{
	// create minimize button
  gLaptopMinButtonImage[0]=  LoadButtonImage( "LAPTOP/x.sti" ,-1,0,-1,1,-1 );
  gLaptopMinButton[0] = QuickCreateButton( gLaptopMinButtonImage[0], 590, 30,
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
										BtnGenericMouseMoveButtonCallback, LaptopMinimizeProgramButtonCallback);

	SetButtonCursor(gLaptopMinButton[0], CURSOR_LAPTOP_SCREEN);
}


static void DestroyMinimizeButtonForCurrentMode(void)
{
	// destroy minimize button
	RemoveButton( gLaptopMinButton[ 0 ] );
	UnloadButtonImage( gLaptopMinButtonImage[ 0 ] );

}


static void SetCurrentToLastProgramOpened(void);


static void LaptopMinimizeProgramButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
		switch (guiCurrentLaptopMode)
		{
			case LAPTOP_MODE_EMAIL:
				gLaptopProgramStates[LAPTOP_PROGRAM_MAILER] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[0], guiTITLEBARICONS, 0);
				break;

			case LAPTOP_MODE_FILES:
				gLaptopProgramStates[LAPTOP_PROGRAM_FILES] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[5], guiTITLEBARICONS, 2);
				break;

			case LAPTOP_MODE_FINANCES:
				gLaptopProgramStates[LAPTOP_PROGRAM_FINANCES] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[2], guiTITLEBARICONS, 5);
				break;

			case LAPTOP_MODE_HISTORY:
				gLaptopProgramStates[LAPTOP_PROGRAM_HISTORY] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[4], guiTITLEBARICONS, 4);
				break;

			case LAPTOP_MODE_PERSONNEL:
				gLaptopProgramStates[LAPTOP_PROGRAM_PERSONNEL] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[3], guiTITLEBARICONS, 3);
				break;

			case LAPTOP_MODE_NONE:
				// nothing
				return;

			default:
				gLaptopProgramStates[LAPTOP_PROGRAM_WEB_BROWSER] = LAPTOP_PROGRAM_MINIMIZED;
				InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[7], guiTITLEBARICONS, 1);
				gfShowBookmarks = FALSE;
				break;
		}
		SetCurrentToLastProgramOpened();
		fMinizingProgram = TRUE;
		fInitTitle = TRUE;
	}
}


static INT32 FindLastProgramStillOpen(void)
{
	INT32 iLowestValue = 6;
	INT32 iLowestValueProgram = 6;
	INT32 iCounter =0;

	// returns ID of last program open and not minimized
	for( iCounter = 0; iCounter < 6; iCounter++ )
	{
		if( gLaptopProgramStates[ iCounter ] != LAPTOP_PROGRAM_MINIMIZED )
		{
			if( gLaptopProgramQueueList[ iCounter ] < iLowestValue )
			{
				iLowestValue = gLaptopProgramQueueList[ iCounter ];
				iLowestValueProgram = iCounter;
			}
		}
	}

	return ( iLowestValueProgram );
}


static void UpdateListToReflectNewProgramOpened(INT32 iOpenedProgram)
{
	INT32 iCounter = 0;

	// will update queue of opened programs to show thier states
	// set iOpenedProgram to 1, and update others

	// increment everyone
	for( iCounter = 0; iCounter < 6; iCounter++ )
	{
			gLaptopProgramQueueList[ iCounter ]++;
	}

	gLaptopProgramQueueList[ iOpenedProgram ] = 1;
}


static void InitLaptopOpenQueue(void)
{

	INT32 iCounter = 0;

	// set evereyone to 1
	for( iCounter = 0; iCounter < 6; iCounter++ )
	{
			gLaptopProgramQueueList[ iCounter ] = 1;
	}
}


static void SetCurrentToLastProgramOpened(void)
{
	guiCurrentLaptopMode = LAPTOP_MODE_NONE;

	switch( FindLastProgramStillOpen( ) )
	{
		case(LAPTOP_PROGRAM_HISTORY ):
				guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;
		break;
		case( LAPTOP_PROGRAM_MAILER ):
       guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		break;
		case( LAPTOP_PROGRAM_PERSONNEL ):
			guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
		break;
		case( LAPTOP_PROGRAM_FINANCES ):
			guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
		break;
		case( LAPTOP_PROGRAM_FILES ):
			guiCurrentLaptopMode = LAPTOP_MODE_FILES;
		break;
		case( LAPTOP_PROGRAM_WEB_BROWSER ):
		// last www mode
			if( guiCurrentWWWMode != 0 )
			{
			  guiCurrentLaptopMode = guiCurrentWWWMode;
			}
			else
			{
				guiCurrentLaptopMode = LAPTOP_MODE_WWW;
			}
			//gfShowBookmarks = TRUE;
			fShowBookmarkInfo = TRUE;
		break;
	}



}


void BlitTitleBarIcons( void )
{
	// will blit the icons for the title bar of the program we are in
	UINT32 Index;
	switch( guiCurrentLaptopMode )
	{
		case LAPTOP_MODE_HISTORY:   Index = 4; break;
		case LAPTOP_MODE_EMAIL:     Index = 0; break;
		case LAPTOP_MODE_PERSONNEL: Index = 3; break;
		case LAPTOP_MODE_FINANCES:  Index = 5; break;
		case LAPTOP_MODE_FILES:     Index = 2; break;
		case LAPTOP_MODE_NONE:      return; // do nothing
		default:                    Index = 1; break; // www pages
	}
	BltVideoObjectFromIndex(FRAME_BUFFER, guiTITLEBARICONS, Index, LAPTOP_TITLE_ICONS_X, LAPTOP_TITLE_ICONS_Y);
}


static BOOLEAN DrawDeskTopBackground(void)
{
  UINT32 uiDestPitchBYTES;
	UINT32 uiSrcPitchBYTES;
  UINT16  *pDestBuf;
	UINT8  *pSrcBuf;
	SGPRect clip;

	// set clipping region
	 clip.iLeft=0 ;
   clip.iRight=506;
	 clip.iTop=0;
	 clip.iBottom=408 + 19;
	// get surfaces
	pDestBuf = (UINT16*)LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES);
	HVSURFACE hSrcVSurface = GetVideoSurface(guiDESKTOP);
  CHECKF(hSrcVSurface != NULL);
	pSrcBuf = LockVideoSurface( guiDESKTOP, &uiSrcPitchBYTES);


	// blit .pcx for the background onto desktop
	Blt8BPPDataSubTo16BPPBuffer( pDestBuf,  uiDestPitchBYTES, hSrcVSurface, pSrcBuf,uiSrcPitchBYTES, LAPTOP_SCREEN_UL_X - 2, LAPTOP_SCREEN_UL_Y - 3, &clip);


	// release surfaces
	UnLockVideoSurface( guiDESKTOP );
  UnLockVideoSurface( FRAME_BUFFER );

	return ( TRUE );
}


static BOOLEAN LoadDesktopBackground(void)
{
	SGPFILENAME ImageFile;
	GetMLGFilename(ImageFile, MLG_DESKTOP);
	guiDESKTOP = AddVideoSurfaceFromFile(ImageFile);
	CHECKF(guiDESKTOP != NO_VSURFACE);
	return( TRUE );
}


static void DeleteDesktopBackground(void)
{
	// delete desktop

	DeleteVideoSurfaceFromIndex(guiDESKTOP);
}


void PrintBalance( void )
{
  CHAR16 pString[ 32 ];
//	UINT16 usX, usY;

	SetFont( FONT10ARIAL );
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );
	SetFontShadow( NO_SHADOW );

	SPrintMoney(pString, LaptopSaveInfo.iCurrentBalance);

	if( ButtonList[ gLaptopButton[ 5 ] ]->uiFlags & BUTTON_CLICKED_ON )
	{
//		gprintfdirty(47 +1, 257 +15 + 1,pString);
		mprintf(47 + 1, 257 + 15 + 1,pString);
	}
	else
	{
//		gprintfdirty(47, 257 +15 ,pString);
		mprintf(47, 257 + 15,pString);
	}


	SetFontShadow( DEFAULT_SHADOW );
}


void PrintNumberOnTeam( void )
{
  CHAR16 pString[ 32 ];
	SOLDIERTYPE *pSoldier, *pTeamSoldier;
  INT32 cnt=0;
	INT32 iCounter=0;
	UINT16 usPosX, usPosY, usFontHeight, usStrLength;


	SetFont( FONT10ARIAL );
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );
	SetFontShadow( NO_SHADOW );

	// grab number on team
	pSoldier = MercPtrs[0];

	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ pSoldier->bTeam ].bLastID; cnt++, pTeamSoldier++)
	{
		pTeamSoldier = MercPtrs[ cnt ];

		if( ( pTeamSoldier->bActive)&&( !( pTeamSoldier->uiStatusFlags & SOLDIER_VEHICLE ) ) )
		{
			  iCounter++;
		}
	}


	swprintf( pString, lengthof(pString), L"%ls %d",pPersonnelString[ 0 ], iCounter );

	usFontHeight = GetFontHeight( FONT10ARIAL );
	usStrLength = StringPixLength( pString, FONT10ARIAL );

	if( ButtonList[ gLaptopButton[ 3 ] ]->uiFlags & BUTTON_CLICKED_ON )
	{
		usPosX = 47 + 1;
		usPosY = 194 + 30 + 1;
//		gprintfdirty(47 + 1, 194 +30 +1  ,pString);
//		mprintf(47 + 1, 194 + 30 + 1,pString);
	}
	else
	{
		usPosX = 47;
		usPosY = 194 + 30;
//		gprintfdirty(47, 194 +30 ,pString);
//		mprintf(47, 194 + 30,pString);
	}

//	RestoreExternBackgroundRect( usPosX, usPosY, usStrLength, usFontHeight );
//	gprintfdirty( usPosX, usPosY, pString);
	mprintf( usPosX, usPosY, pString);


	SetFontShadow( DEFAULT_SHADOW );
}

void PrintDate( void )
{
	SetFont( FONT10ARIAL );
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );

	SetFontShadow( NO_SHADOW );

	mprintf( 30 + (70 - StringPixLength( WORLDTIMESTR, FONT10ARIAL ))/2, 433, WORLDTIMESTR );

	SetFontShadow( DEFAULT_SHADOW );

//	RenderClock( 35, 414 );

/*
	def: removed 3/8/99.
 Now use the render clock function used every where else

	CHAR16 pString[ 32 ];
//	UINT16 usX, usY;

	SetFont( FONT10ARIAL );
	SetFontForeground( FONT_BLACK );
	SetFontBackground( FONT_BLACK );

	SetFontShadow( NO_SHADOW );

	swprintf(pString, L"%ls %d", pMessageStrings[MSG_DAY], GetWorldDay());

//	gprintfdirty(35, 413 + 19,pString);
	mprintf(35, 413 + 19,pString);

	SetFontShadow( DEFAULT_SHADOW );
*/
}


static void DisplayTaskBarIcons(void)
{
//	UINT16 usPosX;


//	usPosX = 83;

	HVOBJECT hPixHandle = GetVideoObject(guiTITLEBARICONS);

	if( fNewFilesInFileViewer )
	{
		//display the files icon, if there is any
	  BltVideoObject(FRAME_BUFFER, hPixHandle, 7,LAPTOP__NEW_FILE_ICON_X, LAPTOP__NEW_FILE_ICON_Y);
	}

	//display the email icon, if there is email
	if( fUnReadMailFlag )
	{
//		usPosX -= 16;
		BltVideoObject(FRAME_BUFFER, hPixHandle, 6, LAPTOP__NEW_EMAIL_ICON_X, LAPTOP__NEW_EMAIL_ICON_Y);
	}

}


static void HandleAltTabKeyInLaptop(void);
static void HandleShiftAltTabKeyInLaptop(void);


void HandleKeyBoardShortCutsForLapTop( UINT16 usEvent, UINT32 usParam, UINT16 usKeyState )
{

	// will handle keyboard shortcuts for the laptop ... to be added to later

	if( ( fExitingLaptopFlag == TRUE )||( fTabHandled ) )
	{
		return;
	}

	if (usEvent == KEY_DOWN && usParam == SDLK_ESCAPE)
	{
		// esc hit, check to see if boomark list is shown, if so, get rid of it, otherwise, leave
    HandleLapTopESCKey( );
	}
	else if (usEvent == KEY_DOWN && usParam == SDLK_TAB)
	{
		if(usKeyState & CTRL_DOWN )
		{
			HandleShiftAltTabKeyInLaptop( );
		}
		else
		{
			HandleAltTabKeyInLaptop( );
		}

		fTabHandled = TRUE;
	}

#ifdef JA2TESTVERSION

	else if ((usEvent == KEY_DOWN )&& ( usParam == 'm' ) )
	{
		if( ( usKeyState & ALT_DOWN ) )
		{
				CheatToGetAll5Merc();
		}
	}
#endif

	else if( (usEvent == KEY_DOWN )&& ( usParam == 'b' ) )
	{
		if( CHEATER_CHEAT_LEVEL( ) )
		{
			if( ( usKeyState & ALT_DOWN ) )
				LaptopSaveInfo.fBobbyRSiteCanBeAccessed = TRUE;
			else if( usKeyState & CTRL_DOWN )
			{
				guiCurrentLaptopMode = LAPTOP_MODE_BROKEN_LINK;
			}
		}
	}

	else if ((usEvent == KEY_DOWN )&& ( usParam == 'x' ) )
	{
		if( ( usKeyState & ALT_DOWN ) )
		{
      HandleShortCutExitState( );
		}
		   //LeaveLapTopScreen( );
	}
#ifdef JA2TESTVERSION
	else if ((usEvent == KEY_DOWN )&& ( usParam == 'q' ))
	{
			//if we dont currently have mercs on the team, hire some
			if( NumberOfMercsOnPlayerTeam() == 0 )
			{
				UINT8 ubRand = (UINT8) Random( 2 ) + 2;
				TempHiringOfMercs( ubRand, FALSE );
			//	QuickStartGame( );
			}
			MarkButtonsDirty( );
			fExitingLaptopFlag = TRUE;
	}
	else if ((usEvent == KEY_DOWN )&& ( usParam == 's' ))
	{
			if( ( usKeyState & ALT_DOWN ) )
			{
				SetBookMark( AIM_BOOKMARK );
				SetBookMark( BOBBYR_BOOKMARK );
				SetBookMark( IMP_BOOKMARK );
				SetBookMark( MERC_BOOKMARK );
				SetBookMark( FUNERAL_BOOKMARK );
				SetBookMark( FLORIST_BOOKMARK );
				SetBookMark( INSURANCE_BOOKMARK );
			}
	}

	//help screen stuff
	else
#endif
	if (usEvent == KEY_DOWN && usParam == SDLK_h)
	{
		ShouldTheHelpScreenComeUp( HELP_SCREEN_LAPTOP, TRUE );
	}


#if defined ( JA2TESTVERSION ) || defined ( JA2DEMO )
	else if ((usEvent == KEY_DOWN )&& ( usParam == 'w' ))
	{
		DemoHiringOfMercs( );
	}
#endif

#ifdef JA2BETAVERSION
	//adding all emails
	else if ((usEvent == KEY_DOWN )&& ( usParam == 'e' ))
	{
		if( CHEATER_CHEAT_LEVEL( ) )
		{
			if( ( usKeyState & ALT_DOWN ) )
			{
				AddAllEmails();
			}
		}
	}
#endif

	//adding money
	else if ((usEvent == KEY_DOWN )&& ( usParam == '=' ))
	{
		if( CHEATER_CHEAT_LEVEL( ) )
		{
			AddTransactionToPlayersBook( ANONYMOUS_DEPOSIT, 0, GetWorldTotalMin(), 100000 );
			MarkButtonsDirty( );
		}
	}

	//subtracting money
	else if( (usEvent == KEY_DOWN ) && ( usParam == '-' ) )
	{
		if( CHEATER_CHEAT_LEVEL( ) )
		{
			AddTransactionToPlayersBook( ANONYMOUS_DEPOSIT, 0, GetWorldTotalMin(), -10000 );
			MarkButtonsDirty( );
		}
	}


#ifdef JA2TESTVERSION
	else if ((usEvent == KEY_DOWN )&& ( usParam == 'd' ))
	{
		if( gfTemporaryDisablingOfLoadPendingFlag )
			gfTemporaryDisablingOfLoadPendingFlag = FALSE;
		else
			gfTemporaryDisablingOfLoadPendingFlag = TRUE;
	}
	else if ((usEvent == KEY_DOWN ) && ( usParam == '+' ))
	{
		if( usKeyState & ALT_DOWN )
		{
			gStrategicStatus.ubHighestProgress += 10;
			if (gStrategicStatus.ubHighestProgress > 100)
				gStrategicStatus.ubHighestProgress = 100;

			InitAllArmsDealers();
			InitBobbyRayInventory();
		}
	}
	else if ((usEvent == KEY_DOWN ) && ( usParam == '-' ))
	{
		if( usKeyState & ALT_DOWN )
		{
			if (gStrategicStatus.ubHighestProgress >= 10)
				gStrategicStatus.ubHighestProgress -= 10;
			else
				gStrategicStatus.ubHighestProgress = 0;

			InitAllArmsDealers();
			InitBobbyRayInventory();
		}
	}
	else if ((usEvent == KEY_DOWN ) && ( usParam == '*' ))
	{
		if( usKeyState & ALT_DOWN )
		{
			DeleteAllStrategicEventsOfType( EVENT_EVALUATE_QUEEN_SITUATION );
			AdvanceToNextDay();
		}
	}
	else if( usEvent == KEY_DOWN && usParam == 'v' && usKeyState & CTRL_DOWN )
	{
		#ifdef SGP_VIDEO_DEBUGGING
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"VObjects:  %d", guiVObjectSize );
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"VSurfaces:  %d", guiVSurfaceSize );
			ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, L"SGPVideoDump.txt updated..." );
			PerformVideoInfoDumpIntoFile( "SGPVideoDump.txt", TRUE );
		#endif
	}
	else
	{
		if ((usEvent == KEY_DOWN )&& ( usParam == '1' ))
		{
			TempHiringOfMercs( 1, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '2' ))
		{
			TempHiringOfMercs( 2, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '3' ))
		{
			TempHiringOfMercs( 3, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '4' ))
		{
			TempHiringOfMercs( 4, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '5' ))
		{
			TempHiringOfMercs( 5, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '6' ))
		{
			TempHiringOfMercs( 6, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '7' ))
		{
			TempHiringOfMercs( 7, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '8' ))
		{
			TempHiringOfMercs( 8, FALSE);
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '9' ))
		{
			TempHiringOfMercs( 9, FALSE );
		}

		if ((usEvent == KEY_DOWN )&& ( usParam == '0' ))
		{
			TempHiringOfMercs( 10, FALSE );
		}
	}
#endif
}


BOOLEAN RenderWWWProgramTitleBar( void )
{
	// will render the title bar for the www program
  UINT32 uiTITLEFORWWW;
	INT32 iIndex = 0;
	CHAR16 sString[256];

	 // title bar - load
	uiTITLEFORWWW = AddVideoObjectFromFile("LAPTOP/programtitlebar.sti");
	CHECKF(uiTITLEFORWWW != NO_VOBJECT);

	BltVideoObjectFromIndex(FRAME_BUFFER, uiTITLEFORWWW, 0, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_UL_Y - 2);

	// now delete
	DeleteVideoObjectFromIndex( uiTITLEFORWWW );

	// now slapdown text
	SetFont( FONT14ARIAL );
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );

  // display title

	// no page loaded yet, do not handle yet


	if( guiCurrentLaptopMode == LAPTOP_MODE_WWW )
	{
	  mprintf(140 ,33 ,pWebTitle[0]);
	}

	else
	{
		iIndex = guiCurrentLaptopMode - LAPTOP_MODE_WWW-1;

		swprintf( sString, lengthof(sString), L"%ls  -  %ls", pWebTitle[0], pWebPagesTitles[ iIndex ] );
		mprintf(140 ,33 ,sString);
	}




	BlitTitleBarIcons( );

	DisplayProgramBoundingBox( FALSE );

	//InvalidateRegion( 0, 0, 640, 480 );
	return( TRUE );
}


static void LaptopProgramIconMinimizeCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateMinimizeRegionsForLaptopProgramIcons(void)
{
	// will create the minizing region to lie over the icon for this particular laptop program

	 MSYS_DefineRegion( &gLapTopProgramMinIcon,LAPTOP_PROGRAM_ICON_X, LAPTOP_PROGRAM_ICON_Y ,LAPTOP_PROGRAM_ICON_X + LAPTOP_PROGRAM_ICON_WIDTH ,LAPTOP_PROGRAM_ICON_Y + LAPTOP_PROGRAM_ICON_HEIGHT, MSYS_PRIORITY_NORMAL+1,
							CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, LaptopProgramIconMinimizeCallback );
}


static void DestroyMinimizeRegionsForLaptopProgramIcons(void)
{
	// will destroy the minizmize regions to be placed over the laptop icons that will be
	// displayed on the top of the laptop program bar

	MSYS_RemoveRegion( &gLapTopProgramMinIcon);
}


static void LaptopProgramIconMinimizeCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// callback handler for the minize region that is attatched to the laptop program icon
	if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
			switch( guiCurrentLaptopMode )
			{
			  case( LAPTOP_MODE_EMAIL ):
					gLaptopProgramStates[ LAPTOP_PROGRAM_MAILER ] = LAPTOP_PROGRAM_MINIMIZED;
				  InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[0], guiTITLEBARICONS, 0 );
				  SetCurrentToLastProgramOpened( );
					fMinizingProgram = TRUE;
					fInitTitle = TRUE;
				break;
				case( LAPTOP_MODE_FILES ):
					gLaptopProgramStates[ LAPTOP_PROGRAM_FILES ] = LAPTOP_PROGRAM_MINIMIZED;
				  InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[5], guiTITLEBARICONS, 2 );
				  SetCurrentToLastProgramOpened( );
					fMinizingProgram = TRUE;
					fInitTitle = TRUE;
				break;
				case( LAPTOP_MODE_FINANCES ):
					gLaptopProgramStates[ LAPTOP_PROGRAM_FINANCES ] = LAPTOP_PROGRAM_MINIMIZED;
				  InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[2], guiTITLEBARICONS, 5 );
					SetCurrentToLastProgramOpened( );
					fMinizingProgram = TRUE;
					fInitTitle = TRUE;
				break;
				case( LAPTOP_MODE_HISTORY ):
					gLaptopProgramStates[ LAPTOP_PROGRAM_HISTORY ] = LAPTOP_PROGRAM_MINIMIZED;
				  InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[4], guiTITLEBARICONS, 4 );
					SetCurrentToLastProgramOpened( );
					fMinizingProgram = TRUE;
					fInitTitle = TRUE;
				break;
				case( LAPTOP_MODE_PERSONNEL ):
					gLaptopProgramStates[ LAPTOP_PROGRAM_PERSONNEL ] = LAPTOP_PROGRAM_MINIMIZED;
				  InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pLaptopIcons[3], guiTITLEBARICONS, 3 );
					SetCurrentToLastProgramOpened( );
					fMinizingProgram = TRUE;
					fInitTitle = TRUE;
				break;
				case( LAPTOP_MODE_NONE ):
				 // nothing
				break;
				default:
					gLaptopProgramStates[ LAPTOP_PROGRAM_WEB_BROWSER ] = LAPTOP_PROGRAM_MINIMIZED;
				  InitTitleBarMaximizeGraphics(guiTITLEBARLAPTOP, pWebTitle[ 0 ], guiTITLEBARICONS, 1 );
					SetCurrentToLastProgramOpened( );
					gfShowBookmarks = FALSE;
					fShowBookmarkInfo = FALSE;
					fMinizingProgram = TRUE;
					fInitTitle = TRUE;
				break;
			}
	}
}


void DisplayProgramBoundingBox( BOOLEAN fMarkButtons )
{
		// the border fot eh program
	BltVideoObjectFromIndex(FRAME_BUFFER, guiLaptopBACKGROUND, 1, 25, 23);

// no laptop mode, no border around the program
	if( guiCurrentLaptopMode != LAPTOP_MODE_NONE )
	{
		BltVideoObjectFromIndex(FRAME_BUFFER, guiLaptopBACKGROUND, 0, 108, 23);
 	}

	if( fMarkButtons || fLoadPendingFlag )
	{
	  MarkButtonsDirty( );
	  RenderButtons( );
	}


	PrintDate( );

	PrintBalance( );

	PrintNumberOnTeam( );

	// new files or email?
	DisplayTaskBarIcons( );

	//InvalidateRegion( 0,0, 640, 480 );
}


static void NewEmailIconCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void NewFileIconCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyMouseRegionForNewMailIcon(void)
{
	static BOOLEAN fCreated = FALSE;

	//. will toggle creation/destruction of the mouse regions used by the new mail icon

	if( fCreated == FALSE )
	{
		fCreated = TRUE;
		MSYS_DefineRegion(&gNewMailIconRegion, LAPTOP__NEW_EMAIL_ICON_X, LAPTOP__NEW_EMAIL_ICON_Y + 5,  LAPTOP__NEW_EMAIL_ICON_X+16,  LAPTOP__NEW_EMAIL_ICON_Y + 16,
		    MSYS_PRIORITY_HIGHEST - 3 , CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, NewEmailIconCallback);
		CreateFileAndNewEmailIconFastHelpText( LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_MAIL, (BOOLEAN)(fUnReadMailFlag == 0 ) );

		MSYS_DefineRegion(&gNewFileIconRegion, LAPTOP__NEW_FILE_ICON_X, LAPTOP__NEW_FILE_ICON_Y + 5,  LAPTOP__NEW_FILE_ICON_X + 16,  LAPTOP__NEW_FILE_ICON_Y + 16,
		    MSYS_PRIORITY_HIGHEST - 3 , CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, NewFileIconCallback);
		CreateFileAndNewEmailIconFastHelpText( LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE, (BOOLEAN)( fNewFilesInFileViewer == 0 ) );
	}
	else
	{
		fCreated = FALSE;
		MSYS_RemoveRegion( &gNewMailIconRegion );
		MSYS_RemoveRegion( &gNewFileIconRegion );
	}
}


static void NewEmailIconCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
		if( fUnReadMailFlag )
		{
			fOpenMostRecentUnReadFlag = TRUE;
			guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		}
	}

}


static void NewFileIconCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
		if( fNewFilesInFileViewer )
		{
			fEnteredFileViewerFromNewFileIcon = TRUE;
			guiCurrentLaptopMode = LAPTOP_MODE_FILES;
		}
	}

}


static void HandleWWWSubSites(void)
{
	// check to see if WW Wait is needed for a sub site within the Web Browser

	if( (guiCurrentLaptopMode == guiPreviousLaptopMode )||( guiCurrentLaptopMode < LAPTOP_MODE_WWW) ||( fLoadPendingFlag == TRUE ) ||( fDoneLoadPending == TRUE ) || ( guiPreviousLaptopMode < LAPTOP_MODE_WWW ) )
	{
		// no go, leave
		return;
	}

	fLoadPendingFlag = TRUE;
	fConnectingToSubPage = TRUE;

	// fast or slow load?
	if( gfWWWaitSubSitesVisitedFlags[ guiCurrentLaptopMode - ( LAPTOP_MODE_WWW + 1 ) ] == TRUE )
	{
		fFastLoadFlag = TRUE;
	}

	// set fact we were here
	gfWWWaitSubSitesVisitedFlags[ guiCurrentLaptopMode - ( LAPTOP_MODE_WWW + 1 ) ] = TRUE;

	//Dont show the dlownload screen when switching between these pages
	if( ( guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS ) && ( guiPreviousLaptopMode == LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX ) ||
			( guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX ) && ( guiPreviousLaptopMode == LAPTOP_MODE_AIM_MEMBERS ) )
	{
		fFastLoadFlag = FALSE;
		fLoadPendingFlag = FALSE;

		// set fact we were here
		gfWWWaitSubSitesVisitedFlags[ LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX - ( LAPTOP_MODE_WWW + 1 ) ] = TRUE;
		gfWWWaitSubSitesVisitedFlags[ LAPTOP_MODE_AIM_MEMBERS - ( LAPTOP_MODE_WWW + 1 ) ] = TRUE;
	}
}


static void UpdateStatusOfDisplayingBookMarks(void)
{
	// this function will disable showing of bookmarks if in process of download or if we miniming web browser
	if( ( fLoadPendingFlag == TRUE ) || ( guiCurrentLaptopMode < LAPTOP_MODE_WWW )  )
	{
		gfShowBookmarks = FALSE;
	}
}


static void InitalizeSubSitesList(void)
{
	INT32 iCounter = 0;

	// init all subsites list to not visited
	for (iCounter = LAPTOP_MODE_WWW + 1; iCounter <= LAPTOP_MODE_FUNERAL; iCounter++)
	{
			gfWWWaitSubSitesVisitedFlags[ iCounter - ( LAPTOP_MODE_WWW + 1 ) ] = FALSE;
	}
}


static void SetSubSiteAsVisted(void)
{
	// sets a www sub site as visited
	if( guiCurrentLaptopMode <= LAPTOP_MODE_WWW )
	{
		// not at a web page yet
	}
	else
	{
		gfWWWaitSubSitesVisitedFlags[ guiCurrentLaptopMode - ( LAPTOP_MODE_WWW + 1 ) ] = TRUE;
	}

}


static void HandleShiftAltTabKeyInLaptop(void)
{
	// will handle the alt tab keying in laptop

	// move to next program
	if( fMaximizingProgram == TRUE )
	{
		return;
	}

	switch( guiCurrentLaptopMode )
	{
		case( LAPTOP_MODE_FINANCES ):
			guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
		break;
		case( LAPTOP_MODE_PERSONNEL ):
			guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;
		break;
		case( LAPTOP_MODE_HISTORY ):
			guiCurrentLaptopMode = LAPTOP_MODE_FILES;
		break;
		case( LAPTOP_MODE_EMAIL ):
			guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
		break;
		case( LAPTOP_MODE_FILES ):
			guiCurrentLaptopMode = LAPTOP_MODE_WWW;
		break;
		case( LAPTOP_MODE_NONE ):
			guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
		break;
		case( LAPTOP_MODE_WWW ):
			guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		break;
		default:
			guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		break;
	}

	fPausedReDrawScreenFlag = TRUE;
}


static void HandleAltTabKeyInLaptop(void)
{
	// will handle the alt tab keying in laptop

	// move to next program
	// move to next program
	if( fMaximizingProgram == TRUE )
	{
		return;
	}

	switch( guiCurrentLaptopMode )
	{
		case( LAPTOP_MODE_FINANCES ):
			guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		break;
		case( LAPTOP_MODE_PERSONNEL ):
			guiCurrentLaptopMode = LAPTOP_MODE_FINANCES;
		break;

		case( LAPTOP_MODE_HISTORY ):
			guiCurrentLaptopMode = LAPTOP_MODE_PERSONNEL;
		break;
		case( LAPTOP_MODE_EMAIL ):
			guiCurrentLaptopMode = LAPTOP_MODE_WWW;
		break;
		case( LAPTOP_MODE_FILES ):
			guiCurrentLaptopMode = LAPTOP_MODE_HISTORY;
		break;
		case( LAPTOP_MODE_NONE ):
			guiCurrentLaptopMode = LAPTOP_MODE_EMAIL;
		break;
		default:
			guiCurrentLaptopMode = LAPTOP_MODE_FILES;
		break;
	}

	fPausedReDrawScreenFlag = TRUE;
}


static void HandleWebBookMarkNotifyTimer(void);


// display the 2 second book mark instruction
static void DisplayWebBookMarkNotify(void)
{
	static BOOLEAN fOldShow = FALSE;

	// handle the timer for this thing
	HandleWebBookMarkNotifyTimer( );

	// are we about to start showing box? or did we just stop?
	if( ( ( fOldShow == FALSE )||( fReDrawBookMarkInfo ) ) && ( fShowBookmarkInfo == TRUE ) )
	{
		fOldShow = TRUE;
		fReDrawBookMarkInfo = FALSE;

		// show background objects
		BltVideoObjectFromIndex(FRAME_BUFFER, guiDOWNLOADTOP,   0,DOWNLOAD_X,     DOWNLOAD_Y);
		BltVideoObjectFromIndex(FRAME_BUFFER, guiDOWNLOADMID,   0,DOWNLOAD_X,     DOWNLOAD_Y +     DOWN_HEIGHT);
		BltVideoObjectFromIndex(FRAME_BUFFER, guiDOWNLOADBOT,   0,DOWNLOAD_X,     DOWNLOAD_Y + 2 * DOWN_HEIGHT);
		BltVideoObjectFromIndex(FRAME_BUFFER, guiTITLEBARICONS, 1,DOWNLOAD_X + 4, DOWNLOAD_Y + 1);

			// font stuff
		SetFont(DOWNLOAD_FONT);
		SetFontForeground(FONT_WHITE);
		SetFontBackground(FONT_BLACK);
		SetFontShadow(NO_SHADOW);


		// display download string
		mprintf( DOWN_STRING_X, DOWN_STRING_Y, pShowBookmarkString[ 0 ]);

		SetFont(BOOK_FONT);
		SetFontForeground(FONT_BLACK);
		SetFontBackground(FONT_BLACK);
		SetFontShadow(NO_SHADOW);

		// now draw the message
		DisplayWrappedString(( INT16 )( DOWN_STRING_X - 42 ),(UINT16)(DOWN_STRING_Y + 20), BOOK_WIDTH + 45, 2,BOOK_FONT,FONT_BLACK, pShowBookmarkString[ 1 ],FONT_BLACK,FALSE,CENTER_JUSTIFIED);

		// invalidate region
		InvalidateRegion(DOWNLOAD_X, DOWNLOAD_Y, DOWNLOAD_X + 150, DOWNLOAD_Y + 100);

	}
	else if( ( fOldShow == TRUE ) && ( fShowBookmarkInfo == FALSE ) )
	{
		//MSYS_RemoveRegion( &gLapTopScreenRegion );
		fOldShow = FALSE;
		fPausedReDrawScreenFlag = TRUE;
	}

	SetFontShadow(DEFAULT_SHADOW);
}


// handle timer for bookmark notify
static void HandleWebBookMarkNotifyTimer(void)
{
	static INT32 iBaseTime = 0;
	INT32 iDifference = 0;
	static BOOLEAN fOldShowBookMarkInfo = FALSE;


	// check if maxing or mining?
	if( ( fMaximizingProgram == TRUE ) || ( fMinizingProgram == TRUE ) )
	{
		fOldShowBookMarkInfo |= fShowBookmarkInfo;
		fShowBookmarkInfo = FALSE;
		return;
	}

		// if we were going to show this pop up, but were delayed, then do so now
	fShowBookmarkInfo |= fOldShowBookMarkInfo;

	// reset old flag
	fOldShowBookMarkInfo = FALSE;

	// if current mode is too low, then reset
	if( guiCurrentLaptopMode < LAPTOP_MODE_WWW )
	{
		fShowBookmarkInfo = FALSE;
	}

	// if showing bookmarks, don't show help
	if( gfShowBookmarks == TRUE )
	{
		fShowBookmarkInfo = FALSE;
	}

	// check if flag false, is so, leave
	if( fShowBookmarkInfo == FALSE )
	{
		iBaseTime = 0;
		return;
	}




	// check if this is the first time in here
	if( iBaseTime == 0 )
	{
		iBaseTime = GetJA2Clock( );
		return;
	}

	iDifference = GetJA2Clock( ) - iBaseTime;

	fReDrawBookMarkInfo = TRUE;

	if( iDifference > DISPLAY_TIME_FOR_WEB_BOOKMARK_NOTIFY )
	{
		// waited long enough, stop showing
		iBaseTime = 0;
		fShowBookmarkInfo = FALSE;
	}
}


void ClearOutTempLaptopFiles( void )
{
	// clear out all temp files from laptop

	// file file
	if (  (FileExists( "files.dat" ) == TRUE ) )
	{
		 FileClearAttributes( "files.dat" );
		 FileDelete( "files.dat" );
	}

	// finances
	if (  (FileExists( "finances.dat" ) == TRUE ) )
	{
		 FileClearAttributes( "finances.dat" );
		 FileDelete( "finances.dat" );
	}

	// email
	if (  (FileExists( "email.dat" ) == TRUE ) )
	{
		 FileClearAttributes( "email.dat" );
		 FileDelete( "email.dat" );
	}

	// history
	if (  (FileExists( "history.dat" ) == TRUE ) )
	{
		 FileClearAttributes( "history.dat" );
		 FileDelete( "history.dat" );
	}

}




BOOLEAN SaveLaptopInfoToSavedGame( HWFILE hFile )
{
	UINT32	uiSize;

	// Save The laptop information
	if (!FileWrite(hFile, &LaptopSaveInfo, sizeof(LaptopSaveInfoStruct))) return FALSE;

	//If there is anything in the Bobby Ray Orders on Delivery
	if( LaptopSaveInfo.usNumberOfBobbyRayOrderUsed )
	{
		//Allocate memory for the information
		uiSize = sizeof( BobbyRayOrderStruct ) * LaptopSaveInfo.usNumberOfBobbyRayOrderItems;

		// Load The laptop information
		if (!FileWrite(hFile, LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray, uiSize)) return FALSE;
	}


	//If there is any Insurance Payouts in progress
	if( LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed )
	{
		//Allocate memory for the information
		uiSize = sizeof( LIFE_INSURANCE_PAYOUT ) * LaptopSaveInfo.ubNumberLifeInsurancePayouts;

		// Load The laptop information
		if (!FileWrite(hFile, LaptopSaveInfo.pLifeInsurancePayouts, uiSize)) return FALSE;
	}

	return( TRUE );
}




BOOLEAN LoadLaptopInfoFromSavedGame( HWFILE hFile )
{
	UINT32	uiSize;


	//if there is memory allocated for the BobbyR orders
	if( LaptopSaveInfo.usNumberOfBobbyRayOrderItems )
	{
//		if( !LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray )
//			Assert( 0 );	//Should never happen

		//Free the memory
		if( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray )
			MemFree( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray );
		LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = NULL;
	}

	//if there is memory allocated for life insurance payouts
	if( LaptopSaveInfo.ubNumberLifeInsurancePayouts )
	{
		Assert(LaptopSaveInfo.pLifeInsurancePayouts != NULL); //Should never happen

		//Free the memory
		MemFree( LaptopSaveInfo.pLifeInsurancePayouts );
		LaptopSaveInfo.pLifeInsurancePayouts = NULL;
	}


	// Load The laptop information
	if (!FileRead(hFile, &LaptopSaveInfo, sizeof(LaptopSaveInfoStruct))) return FALSE;


	//If there is anything in the Bobby Ray Orders on Delivery
	if( LaptopSaveInfo.usNumberOfBobbyRayOrderUsed )
	{
		//Allocate memory for the information
		uiSize = sizeof( BobbyRayOrderStruct ) * LaptopSaveInfo.usNumberOfBobbyRayOrderItems;

		LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = MemAlloc( uiSize );
		Assert( LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray );

		// Load The laptop information
		if (!FileRead(hFile, LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray, uiSize)) return FALSE;
	}
	else
	{
		LaptopSaveInfo.usNumberOfBobbyRayOrderItems = 0;
		LaptopSaveInfo.BobbyRayOrdersOnDeliveryArray = NULL;
	}


	//If there is any Insurance Payouts in progress
	if( LaptopSaveInfo.ubNumberLifeInsurancePayoutUsed )
	{
		//Allocate memory for the information
		uiSize = sizeof( LIFE_INSURANCE_PAYOUT ) * LaptopSaveInfo.ubNumberLifeInsurancePayouts;

		LaptopSaveInfo.pLifeInsurancePayouts = MemAlloc( uiSize );
		Assert( LaptopSaveInfo.pLifeInsurancePayouts );

		// Load The laptop information
		if (!FileRead(hFile, LaptopSaveInfo.pLifeInsurancePayouts, uiSize)) return FALSE;
	}
	else
	{
		LaptopSaveInfo.ubNumberLifeInsurancePayouts = 0;
		LaptopSaveInfo.pLifeInsurancePayouts = NULL;
	}


	return( TRUE );
}


static INT32 WWaitDelayIncreasedIfRaining(INT32 iUnitTime)
{
	INT32	iRetVal = 0;

	if( guiEnvWeather	& WEATHER_FORECAST_THUNDERSHOWERS )
	{
		iRetVal = (INT32)( iUnitTime * (FLOAT) 0.80 );
	}
	else if( guiEnvWeather & WEATHER_FORECAST_SHOWERS )
	{
		iRetVal = (INT32)( iUnitTime * (FLOAT) 0.6 );
	}

	return( iRetVal );
}


// Used to determine delay if its raining
static BOOLEAN IsItRaining(void)
{
	if( guiEnvWeather & WEATHER_FORECAST_SHOWERS || guiEnvWeather & WEATHER_FORECAST_THUNDERSHOWERS )
		return( TRUE );
	else
		return( FALSE );
}


static void InternetRainDelayMessageBoxCallBack(UINT8 bExitValue)
{
	GoToWebPage(	giRainDelayInternetSite );

	//Set to -2 so we dont due the message for this occurence of laptop
	giRainDelayInternetSite = -2;
}


static void CreateBookMarkHelpText(MOUSE_REGION* pRegion, UINT32 uiBookMarkID)
{
	SetRegionFastHelpText( pRegion, gzLaptopHelpText[ BOOKMARK_TEXT_ASSOCIATION_OF_INTERNATION_MERCENARIES + uiBookMarkID ] );
}

void CreateFileAndNewEmailIconFastHelpText( UINT32 uiHelpTextID, BOOLEAN fClearHelpText )
{
	MOUSE_REGION *pRegion;

	switch( uiHelpTextID )
	{
		case LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_MAIL:
			pRegion = &gNewMailIconRegion;
			break;

		case LAPTOP_BN_HLP_TXT_YOU_HAVE_NEW_FILE:
			pRegion = &gNewFileIconRegion;
			break;

		default:
			Assert(0);
			return;
	}

	if( fClearHelpText )
		SetRegionFastHelpText( pRegion, L"" );
	else
		SetRegionFastHelpText( pRegion, gzLaptopHelpText[ uiHelpTextID ] );

	//fUnReadMailFlag
	//fNewFilesInFileViewer
}


static void CreateLaptopButtonHelpText(INT32 iButtonIndex, UINT32 uiButtonHelpTextID)
{
	SetButtonFastHelpText( iButtonIndex, gzLaptopHelpText[ uiButtonHelpTextID ] );
}
