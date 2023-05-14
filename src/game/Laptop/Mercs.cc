#include "Directories.h"
#include "HImage.h"
#include "Laptop.h"
#include "Local.h"
#include "Mercs.h"
#include "Timer_Control.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Overhead.h"
#include "Soldier_Add.h"
#include "EMail.h"
#include "Game_Clock.h"
#include "Faces.h"
#include "Dialogue_Control.h"
#include "MercTextBox.h"
#include "Merc_Hiring.h"
#include "Random.h"
#include "LaptopSave.h"
#include "GameSettings.h"
#include "Text.h"
#include "Speck_Quotes.h"
#include "Mercs_Account.h"
#include "Soldier_Profile.h"
#include "Game_Event_Hook.h"
#include "Quests.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Debug.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "MERCListingModel.h"
#include <climits>
#include <string_theory/format>
#include <string_theory/string>


#define MERC_TEXT_FONT			FONT12ARIAL
#define MERC_TEXT_COLOR			FONT_MCOLOR_WHITE

#define MERC_VIDEO_TITLE_FONT		FONT10ARIAL
#define MERC_VIDEO_TITLE_COLOR		FONT_MCOLOR_LTYELLOW

#define MERC_BACKGROUND_WIDTH		125
#define MERC_BACKGROUND_HEIGHT		100

#define MERC_TITLE_X			LAPTOP_SCREEN_UL_X + 135
#define MERC_TITLE_Y			LAPTOP_SCREEN_WEB_UL_Y + 20

#define MERC_PORTRAIT_X			LAPTOP_SCREEN_UL_X + 198
#define MERC_PORTRAIT_Y			LAPTOP_SCREEN_WEB_UL_Y + 96
#define MERC_PORTRAIT_TEXT_X		MERC_PORTRAIT_X
#define MERC_PORTRAIT_TEXT_Y		MERC_PORTRAIT_Y + 109
#define MERC_PORTRAIT_TEXT_WIDTH	115

#define MERC_ACCOUNT_BOX_X		LAPTOP_SCREEN_UL_X + 138
#define MERC_ACCOUNT_BOX_Y		LAPTOP_SCREEN_WEB_UL_Y + 251

#define MERC_ACCOUNT_BOX_TEXT_X		MERC_ACCOUNT_BOX_X
#define MERC_ACCOUNT_BOX_TEXT_Y		MERC_ACCOUNT_BOX_Y + 20
#define MERC_ACCOUNT_BOX_TEXT_WIDTH	110

#define MERC_ACCOUNT_ARROW_X		MERC_ACCOUNT_BOX_X + 125
#define MERC_ACCOUNT_ARROW_Y		MERC_ACCOUNT_BOX_Y + 18

#define MERC_ACCOUNT_BUTTON_X		MERC_ACCOUNT_BOX_X + 133
#define MERC_ACCOUNT_BUTTON_Y		MERC_ACCOUNT_BOX_Y + 8

#define MERC_FILE_BOX_X			MERC_ACCOUNT_BOX_X
#define MERC_FILE_BOX_Y			LAPTOP_SCREEN_WEB_UL_Y + 321

#define MERC_FILE_BOX_TEXT_X		MERC_FILE_BOX_X
#define MERC_FILE_BOX_TEXT_Y		MERC_FILE_BOX_Y + 20
#define MERC_FILE_BOX_TEXT_WIDTH	MERC_ACCOUNT_BOX_TEXT_WIDTH

#define MERC_FILE_ARROW_X		MERC_FILE_BOX_X + 125
#define MERC_FILE_ARROW_Y		MERC_FILE_BOX_Y + 18

#define MERC_FILE_BUTTON_X		MERC_ACCOUNT_BUTTON_X
#define MERC_FILE_BUTTON_Y		MERC_FILE_BOX_Y + 8



// Video Conference Defines
#define MERC_VIDEO_BACKGROUND_X		MERC_PORTRAIT_X
#define MERC_VIDEO_BACKGROUND_Y		MERC_PORTRAIT_Y
#define MERC_VIDEO_BACKGROUND_WIDTH	116
#define MERC_VIDEO_BACKGROUND_HEIGHT	108

#define MERC_VIDEO_FACE_X		(UINT16)(MERC_VIDEO_BACKGROUND_X + 10)
#define MERC_VIDEO_FACE_Y		(UINT16)(MERC_VIDEO_BACKGROUND_Y + 17)
#define MERC_VIDEO_FACE_WIDTH		96
#define MERC_VIDEO_FACE_HEIGHT		86
#define MERC_X_TO_CLOSE_VIDEO_X		MERC_VIDEO_BACKGROUND_X + 104
#define MERC_X_TO_CLOSE_VIDEO_Y		MERC_VIDEO_BACKGROUND_Y + 3
#define MERC_X_VIDEO_TITLE_X		MERC_VIDEO_BACKGROUND_X + 5
#define MERC_X_VIDEO_TITLE_Y		MERC_VIDEO_BACKGROUND_Y + 3

#define MERC_INTRO_TIME			1000
#define MERC_EXIT_TIME			500

#define MERC_TEXT_BOX_POS_Y		(STD_SCREEN_Y + 255)

#define SPECK_IDLE_CHAT_DELAY		10000

#define MERC_NUMBER_OF_RANDOM_QUOTES	14

enum
{
	MERC_ARRIVES_BUBBA,
	MERC_ARRIVES_LARRY,
	MERC_ARRIVES_NUMB,
	MERC_ARRIVES_COUGAR,

	NUM_MERC_ARRIVALS,
};


enum
{
	MERC_DISTORTION_NO_DISTORTION,
	MERC_DISTORTION_PIXELATE_UP,
	MERC_DISTORTION_PIXELATE_DOWN,
	MERC_DISRTORTION_DISTORT_IMAGE,
};


enum
{
	MERC_SITE_NEVER_VISITED,
	MERC_SITE_FIRST_VISIT,
	MERC_SITE_SECOND_VISIT,
	MERC_SITE_THIRD_OR_MORE_VISITS,
};


// Image Indetifiers

static SGPVObject* guiAccountBox;
static SGPVObject* guiArrow;
static SGPVObject* guiFilesBox;
static SGPVObject* guiMercSymbol;
static SGPVObject* guiSpecPortrait;
static SGPVObject* guiMercBackGround;
static SGPVSurface* guiMercVideoFaceBackground;
static SGPVObject* guiMercVideoPopupBackground;

UINT8			gubCurMercIndex;

static MercPopUpBox* g_merc_popup_box;

static ST::string gsSpeckDialogueTextPopUp;
static UINT16  gusSpeckDialogueX;
static UINT16  gusSpeckDialogueActualWidth;

static BOOLEAN gfInMercSite = FALSE; // this flag is set when inide of the merc site

//Merc Video Conferencing Mode
enum
{
	MERC_VIDEO_NO_VIDEO_MODE,
	MERC_VIDEO_INIT_VIDEO_MODE,
	MERC_VIDEO_VIDEO_MODE,
	MERC_VIDEO_EXIT_VIDEO_MODE,
};

static UINT8     gubCurrentMercVideoMode;
static BOOLEAN   gfMercVideoIsBeingDisplayed;
static FACETYPE* g_video_speck_face;
UINT16		gusMercVideoSpeckSpeech;

static BOOLEAN gfDisplaySpeckTextBox = FALSE;

static BOOLEAN gfJustEnteredMercSite = FALSE;
UINT8			gubArrivedFromMercSubSite=MERC_CAME_FROM_OTHER_PAGE;		//the merc is arriving from one of the merc sub pages
static BOOLEAN gfDoneIntroSpeech     = TRUE;

static BOOLEAN gfMercSiteScreenIsReDrawn = FALSE;

BOOLEAN		gfJustHiredAMercMerc=FALSE;

static BOOLEAN gfRedrawMercSite = FALSE;

static BOOLEAN gfFirstTimeIntoMERCSiteSinceEnteringLaptop = FALSE;

// This is not persisted but lasts for the entire game session, and never get reset
static std::map<UINT8, UINT32> gNumberOfTimesQuoteSaid = {};


//
// Buttons
//

// The Account Box button
static void BtnAccountBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static BUTTON_PICS* guiAccountBoxButtonImage;
static GUIButtonRef guiAccountBoxButton;

//File Box
static void BtnFileBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiFileBoxButton;

// The 'X' to close the video conf window button
static void BtnXToCloseMercVideoButtonCallback(GUI_BUTTON *btn, UINT32 reason);
static GUIButtonRef guiXToCloseMercVideoButton;


//Mouse region for the subtitles region when the merc is talking
static MOUSE_REGION gMercSiteSubTitleMouseRegion;

static void MakeNewMercsAvailable(BOOLEAN fShouldNotifyPlayer);

void GameInitMercs()
{
	LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_NO_ACCOUNT;
	gubCurMercIndex = 0;

	MakeNewMercsAvailable(FALSE);

	gubCurrentMercVideoMode = MERC_VIDEO_NO_VIDEO_MODE;
	gfMercVideoIsBeingDisplayed = FALSE;

	gusMercVideoSpeckSpeech = 0;
}


static BOOLEAN GetSpeckConditionalOpening(BOOLEAN fJustEnteredScreen);
static void HandleSpeckIdleConversation(BOOLEAN fReset);
static BOOLEAN HandleSpeckTalking(BOOLEAN fReset);
static BOOLEAN ShouldSpeckSayAQuote(void);


void EnterMercs()
{
	SetBookMark( MERC_BOOKMARK );

	//Reset a static variable
	HandleSpeckTalking( TRUE );

	InitMercBackGround();

	// load the Account box graphic and add it
	guiAccountBox = AddVideoObjectFromFile(LAPTOPDIR "/accountbox.sti");

	// load the files Box graphic and add it
	guiFilesBox = AddVideoObjectFromFile(LAPTOPDIR "/filesbox.sti");

	// load the MercSymbol graphic and add it
	guiMercSymbol = AddVideoObjectFromFile(LAPTOPDIR "/mercsymbol.sti");

	// load the SpecPortrait graphic and add it
	guiSpecPortrait = AddVideoObjectFromFile(LAPTOPDIR "/specportrait.sti");

	// load the Arrow graphic and add it
	guiArrow = AddVideoObjectFromFile(LAPTOPDIR "/arrow.sti");

	// load the Merc video conf background graphic and add it
	guiMercVideoPopupBackground = AddVideoObjectFromFile(LAPTOPDIR "/speckcomwindow.sti");

	// Account Box button
	guiAccountBoxButtonImage = LoadButtonImage(LAPTOPDIR "/smallbuttons.sti", 0, 1);

	guiAccountBoxButton = QuickCreateButton(guiAccountBoxButtonImage, MERC_ACCOUNT_BUTTON_X, MERC_ACCOUNT_BUTTON_Y, MSYS_PRIORITY_HIGH, BtnAccountBoxButtonCallback);
	guiAccountBoxButton->SetCursor(CURSOR_LAPTOP_SCREEN);

	guiFileBoxButton = QuickCreateButton(guiAccountBoxButtonImage, MERC_FILE_BUTTON_X, MERC_FILE_BUTTON_Y, MSYS_PRIORITY_HIGH, BtnFileBoxButtonCallback);
	guiFileBoxButton->SetCursor(CURSOR_LAPTOP_SCREEN);
	guiFileBoxButton->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);

	//if the player doesnt have an account disable it
	if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_NO_ACCOUNT )
	{
		DisableButton( guiFileBoxButton );
	}


	//
	//	Video Conferencing stuff
	//

	// Create a background video surface to blt the face onto
	guiMercVideoFaceBackground = AddVideoSurface(MERC_VIDEO_FACE_WIDTH, MERC_VIDEO_FACE_HEIGHT, PIXEL_DEPTH);

	RenderMercs();

	//init the face

	gfJustEnteredMercSite = TRUE;

	//if NOT entering from a subsite
	if( gubArrivedFromMercSubSite == MERC_CAME_FROM_OTHER_PAGE )
	{
		//Set that we have been here before
		if( LaptopSaveInfo.ubPlayerBeenToMercSiteStatus == MERC_SITE_NEVER_VISITED )
			LaptopSaveInfo.ubPlayerBeenToMercSiteStatus = MERC_SITE_FIRST_VISIT;
		else if( LaptopSaveInfo.ubPlayerBeenToMercSiteStatus == MERC_SITE_FIRST_VISIT )
			LaptopSaveInfo.ubPlayerBeenToMercSiteStatus = MERC_SITE_SECOND_VISIT;
		else
			LaptopSaveInfo.ubPlayerBeenToMercSiteStatus = MERC_SITE_THIRD_OR_MORE_VISITS;

		//Reset the speech variable
		gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;
	}


	GetSpeckConditionalOpening( TRUE );
	//gubArrivedFromMercSubSite = MERC_CAME_FROM_OTHER_PAGE;

	//if Speck should start talking
	if( ShouldSpeckSayAQuote() )
	{
		gubCurrentMercVideoMode = MERC_VIDEO_INIT_VIDEO_MODE;
	}

	//Reset the some variables
	HandleSpeckIdleConversation( TRUE );

	//Since we are in the site, set the flag
	gfInMercSite = TRUE;
}


static void InitDestroyXToCloseVideoWindow(BOOLEAN fCreate);
static void RemoveSpeckPopupTextBox(void);
static void StopSpeckFromTalking(void);


void ExitMercs()
{
	StopSpeckFromTalking( );

	if( gfMercVideoIsBeingDisplayed )
	{
		gfMercVideoIsBeingDisplayed = FALSE;
		DeleteFace(g_video_speck_face);
		InitDestroyXToCloseVideoWindow( FALSE );
		gubCurrentMercVideoMode = MERC_VIDEO_NO_VIDEO_MODE;
	}

	DeleteVideoObject(guiAccountBox);
	DeleteVideoObject(guiFilesBox);
	DeleteVideoObject(guiMercSymbol);
	DeleteVideoObject(guiSpecPortrait);
	DeleteVideoObject(guiArrow);
	DeleteVideoObject(guiMercVideoPopupBackground);

	UnloadButtonImage( guiAccountBoxButtonImage );
	RemoveButton( guiFileBoxButton );
	RemoveButton( guiAccountBoxButton );

	RemoveMercBackGround();

	DeleteVideoSurface(guiMercVideoFaceBackground);

	/*
	//Set that we have been here before
	if( LaptopSaveInfo.ubPlayerBeenToMercSiteStatus == MERC_SITE_FIRST_VISIT )
		LaptopSaveInfo.ubPlayerBeenToMercSiteStatus = MERC_SITE_SECOND_VISIT;
	else
		LaptopSaveInfo.ubPlayerBeenToMercSiteStatus = MERC_SITE_THIRD_OR_MORE_VISITS;*/

	gfJustEnteredMercSite = TRUE;
	gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;

	//Remove the merc text box if one is available
	RemoveSpeckPopupTextBox();

	//Set up so next time we come in, we know we came from a differnt page
	gubArrivedFromMercSubSite = MERC_CAME_FROM_OTHER_PAGE;

	gfJustHiredAMercMerc = FALSE;

	//Since we are leaving the site, set the flag
	gfInMercSite = TRUE;

	//Empty the Queue cause Speck could still have a quote in waiting
	EmptyDialogueQueue( );
}


static void DrawMercVideoBackGround(void);
static void HandleTalkingSpeck(void);
static void InitMercVideoFace(void);


void HandleMercs()
{
	if( gfRedrawMercSite )
	{
		RenderMercs();
		gfRedrawMercSite = FALSE;
		gfMercSiteScreenIsReDrawn = TRUE;
	}

	//if Speck has something to say, say it
	if( gusMercVideoSpeckSpeech != MERC_VIDEO_SPECK_SPEECH_NOT_TALKING )// && !gfDoneIntroSpeech )
	{
		//if the face isnt active, make it so
		if( !gfMercVideoIsBeingDisplayed )
		{
			// Blt the video window background
			DrawMercVideoBackGround();

			InitDestroyXToCloseVideoWindow( TRUE );

			InitMercVideoFace();
			gubCurrentMercVideoMode = MERC_VIDEO_INIT_VIDEO_MODE;

			//gfMercSiteScreenIsReDrawn = TRUE;
		}
	}

	//if the page is redrawn, and we are in video conferencing, redraw the VC backgrund graphic
	if( gfMercVideoIsBeingDisplayed && gfMercSiteScreenIsReDrawn )
	{
		// Blt the video window background
		DrawMercVideoBackGround();

		gfMercSiteScreenIsReDrawn = FALSE;
	}


	//if Specks should be video conferencing...
	if( gubCurrentMercVideoMode != MERC_VIDEO_NO_VIDEO_MODE )
	{
		HandleTalkingSpeck();
	}

	//Reset the some variables
	HandleSpeckIdleConversation( FALSE );

	if (!fCurrentlyInLaptop)
	{
		//if we are exiting the laptop screen, shut up the speck
		StopSpeckFromTalking( );
	}
}

void RenderMercs()
{
	DrawMecBackGround();

	BltVideoObject(FRAME_BUFFER, guiMercSymbol,   0, MERC_TITLE_X,       MERC_TITLE_Y);
	BltVideoObject(FRAME_BUFFER, guiSpecPortrait, 0, MERC_PORTRAIT_X,    MERC_PORTRAIT_Y);
	BltVideoObject(FRAME_BUFFER, guiAccountBox,   0, MERC_ACCOUNT_BOX_X, MERC_ACCOUNT_BOX_Y);
	BltVideoObject(FRAME_BUFFER, guiFilesBox,     0, MERC_FILE_BOX_X,    MERC_FILE_BOX_Y);

	//Text on the Speck Portrait
	DisplayWrappedString(MERC_PORTRAIT_TEXT_X, MERC_PORTRAIT_TEXT_Y, MERC_PORTRAIT_TEXT_WIDTH, 2, MERC_TEXT_FONT, MERC_TEXT_COLOR, MercHomePageText[MERC_SPECK_OWNER], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Text on the Account Box
	if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_NO_ACCOUNT )
		DisplayWrappedString(MERC_ACCOUNT_BOX_TEXT_X, MERC_ACCOUNT_BOX_TEXT_Y, MERC_ACCOUNT_BOX_TEXT_WIDTH, 2, MERC_TEXT_FONT, MERC_TEXT_COLOR, MercHomePageText[MERC_OPEN_ACCOUNT], FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	else
		DisplayWrappedString(MERC_ACCOUNT_BOX_TEXT_X, MERC_ACCOUNT_BOX_TEXT_Y, MERC_ACCOUNT_BOX_TEXT_WIDTH, 2, MERC_TEXT_FONT, MERC_TEXT_COLOR, MercHomePageText[MERC_VIEW_ACCOUNT], FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	//Text on the Files Box
	DisplayWrappedString(MERC_FILE_BOX_TEXT_X, MERC_FILE_BOX_TEXT_Y, MERC_FILE_BOX_TEXT_WIDTH, 2, MERC_TEXT_FONT, MERC_TEXT_COLOR, MercHomePageText[MERC_VIEW_FILES], FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	//If the Specks popup dioalogue box is active, display it.
	if (g_merc_popup_box)
	{
		guiAccountBoxButton->Draw();
		guiAccountBoxButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
		RenderMercPopUpBox(g_merc_popup_box, gusSpeckDialogueX, MERC_TEXT_BOX_POS_Y, FRAME_BUFFER);
	}

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );

	//if the page is redrawn, and we are in video conferencing, redraw the VC backgrund graphic
	gfMercSiteScreenIsReDrawn = TRUE;

	guiAccountBoxButton->uiFlags &= ~BUTTON_FORCE_UNDIRTY;

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


void InitMercBackGround()
{
	// load the Merc background graphic and add it
	guiMercBackGround = AddVideoObjectFromFile(LAPTOPDIR "/mercbackground.sti");
}


void DrawMecBackGround()
{
	WebPageTileBackground(4, 4, MERC_BACKGROUND_WIDTH, MERC_BACKGROUND_HEIGHT, guiMercBackGround);
}


void RemoveMercBackGround()
{
	DeleteVideoObject(guiMercBackGround);
}


static void BtnAccountBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_NO_ACCOUNT)
			guiCurrentLaptopMode = LAPTOP_MODE_MERC_NO_ACCOUNT;
		else
			guiCurrentLaptopMode = LAPTOP_MODE_MERC_ACCOUNT;

		if (g_merc_popup_box)
		{
			guiAccountBoxButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
			RenderMercPopUpBox(g_merc_popup_box, gusSpeckDialogueX, MERC_TEXT_BOX_POS_Y, FRAME_BUFFER);
		}
	}
}


static void BtnFileBoxButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_MERC_FILES;
	}
}


static void ScheduleNewMercsToBeAvailable();
static BOOLEAN ShouldTheMercSiteServerGoDown(void);


void DailyUpdateOfMercSite()
{
	//if its the first day, leave
	if (GetWorldDay() == 1)
		return;

	INT32 iNumDays = 0;

	//loop through all of the hired mercs from M.E.R.C.
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		const ProfileID pid = GetProfileIDFromMERCListing(m);
		if (!IsMercOnTeam(pid)) continue;
		MERCPROFILESTRUCT& p = GetProfile(pid);

		//if the merc is dead, dont advance the contract length
		if (!IsMercDead(p)) p.iMercMercContractLength += 1;

		//Get the longest time
		if (p.iMercMercContractLength > iNumDays) iNumDays = p.iMercMercContractLength;
	}

	//if the players hasnt paid for a while, get email him to tell him to pay
	//iTotalContractLength
	if( iNumDays > MERC_NUM_DAYS_TILL_ACCOUNT_INVALID )
	{
		if( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_INVALID )
		{
			LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_INVALID;
			AddEmail( MERC_INVALID, MERC_INVALID_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());
		}
	}
	else if( iNumDays > MERC_NUM_DAYS_TILL_ACCOUNT_SUSPENDED )
	{
		if( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_SUSPENDED )
		{
			LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_SUSPENDED;
			AddEmail( MERC_WARNING, MERC_WARNING_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());

			// Have speck complain next time player come to site
			LaptopSaveInfo.uiSpeckQuoteFlags |= SPECK_QUOTE__SENT_EMAIL_ABOUT_LACK_OF_PAYMENT;
		}
	}
	else if( iNumDays > MERC_NUM_DAYS_TILL_FIRST_WARNING)
	{
		if( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_VALID_FIRST_WARNING )
		{
			LaptopSaveInfo.gubPlayersMercAccountStatus = MERC_ACCOUNT_VALID_FIRST_WARNING;
			AddEmail( MERC_FIRST_WARNING, MERC_FIRST_WARNING_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());

			// Have speck complain next time player come to site
			LaptopSaveInfo.uiSpeckQuoteFlags |= SPECK_QUOTE__SENT_EMAIL_ABOUT_LACK_OF_PAYMENT;
		}
	}


	//Check and act if any new Merc Mercs should become available
	ScheduleNewMercsToBeAvailable();

	// If the merc site has never gone down, the number of MERC payment days is above 'X',
	// and the players account status is ok ( cant have the merc site going down when the player owes him money, player may lose account that way )
	if( ShouldTheMercSiteServerGoDown() )
	{
		UINT32	uiTimeInMinutes=0;


		//Set the fact the site has gone down
		LaptopSaveInfo.fMercSiteHasGoneDownYet = TRUE;

		//Get the site up the next day at 6:00 pm
		uiTimeInMinutes = GetMidnightOfFutureDayInMinutes( 1 ) + 18 * 60;

		//Add an event that will get the site back up and running
		AddStrategicEvent( EVENT_MERC_SITE_BACK_ONLINE, uiTimeInMinutes, 0 );
	}
}


static BOOLEAN HasLarryRelapsed(void);

/**
 * Returns the ProfileID of this profile listing.
 *
 * This method handles LARRY logic (LARRY_NORMAL becoming LARRY_DRUNK once
 * relapsed), and should always be used instead of directly accessing the
 * field profileID
 */
ProfileID GetProfileIDFromMERCListing(const MERCListingModel* listing)
{
	ProfileID ubMercID = listing->profileID;
	if (ubMercID == LARRY_NORMAL && HasLarryRelapsed())
	{
		return LARRY_DRUNK;
	}
	return ubMercID;
};

//Gets the actual merc id from the array
ProfileID GetProfileIDFromMERCListingIndex(UINT8 ubMercIndex)
{
	auto mercsListing = GCM->getMERCListings();
	return GetProfileIDFromMERCListing(mercsListing.at(ubMercIndex));
}

static void InitMercVideoFace(void)
{
	// Allocates space, and loads the sti for SPECK
	FACETYPE& f = InitFace(SPECK, 0, 0);
	g_video_speck_face = &f;

	// Sets up the eyes blinking and the mouth moving
	SetAutoFaceActive(guiMercVideoFaceBackground, FACE_AUTO_RESTORE_BUFFER, f, 0, 0);

	//Renders the face to the background
	RenderAutoFace(f);

	//enables the global flag indicating the the video is being displayed
	gfMercVideoIsBeingDisplayed = TRUE;

}


static BOOLEAN StartSpeckTalking(UINT16 usQuoteNum)
{
	if( usQuoteNum == MERC_VIDEO_SPECK_SPEECH_NOT_TALKING || usQuoteNum == MERC_VIDEO_SPECK_HAS_TO_TALK_BUT_QUOTE_NOT_CHOSEN_YET )
		return( FALSE );

	//Reset the time for when speck starts to do the random quotes
	HandleSpeckIdleConversation( TRUE );

	//Start Speck talking
	CharacterDialogue(SPECK, usQuoteNum, g_video_speck_face, DIALOGUE_SPECK_CONTACT_PAGE_UI, FALSE);

	gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;

	return(TRUE);
}


// Performs the frame by frame update
static BOOLEAN HandleSpeckTalking(BOOLEAN fReset)
{
	static BOOLEAN fWasTheMercTalking=FALSE;
	BOOLEAN        fIsTheMercTalking;

	if( fReset )
	{
		fWasTheMercTalking = FALSE;
		return( TRUE );
	}

	HandleDialogue();
	HandleAutoFaces( );
	HandleTalkingAutoFaces( );

	//Blt the face surface to the video background surface
	SGPBox const SrcRect  = { 0, 0, 48, 43 };
	SGPBox const DestRect = { MERC_VIDEO_FACE_X, MERC_VIDEO_FACE_Y, MERC_VIDEO_FACE_WIDTH, MERC_VIDEO_FACE_HEIGHT };
	BltStretchVideoSurface(FRAME_BUFFER, guiMercVideoFaceBackground, &SrcRect, &DestRect);

	//HandleCurrentMercDistortion();

	InvalidateRegion(MERC_VIDEO_BACKGROUND_X, MERC_VIDEO_BACKGROUND_Y, (MERC_VIDEO_BACKGROUND_X + MERC_VIDEO_BACKGROUND_WIDTH), (MERC_VIDEO_BACKGROUND_Y + MERC_VIDEO_BACKGROUND_HEIGHT) );

	//find out if the merc just stopped talking
	fIsTheMercTalking = g_video_speck_face->fTalking;

	//if the merc just stopped talking
	if(fWasTheMercTalking && !fIsTheMercTalking)
	{
		fWasTheMercTalking = FALSE;

		if( DialogueQueueIsEmpty( ) )
		{
			RemoveSpeckPopupTextBox();

			gfDisplaySpeckTextBox = FALSE;

			gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;

			//Reset the time for when speck starts to do the random quotes
			HandleSpeckIdleConversation( TRUE );
		}
		else
			fIsTheMercTalking = TRUE;
	}

	fWasTheMercTalking = fIsTheMercTalking;

	return(fIsTheMercTalking);
}

static void InitDestroyXToCloseVideoWindow(BOOLEAN fCreate)
{
	static BOOLEAN fButtonCreated=FALSE;

	//if we are asked to create the buttons and the button isnt already created
	if( fCreate && !fButtonCreated )
	{
		guiXToCloseMercVideoButton = QuickCreateButtonImg(LAPTOPDIR "/closebutton.sti", 0, 1, MERC_X_TO_CLOSE_VIDEO_X, MERC_X_TO_CLOSE_VIDEO_Y, MSYS_PRIORITY_HIGH, BtnXToCloseMercVideoButtonCallback);
		guiXToCloseMercVideoButton->SetCursor(CURSOR_LAPTOP_SCREEN);

		fButtonCreated = TRUE;
	}

	//if we are asked to destroy the buttons and the buttons are created
	if( !fCreate && fButtonCreated )
	{
		RemoveButton( guiXToCloseMercVideoButton );
		fButtonCreated = FALSE;
	}
}


static void BtnXToCloseMercVideoButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//Stop speck from talking
		//ShutupaYoFace(g_video_speck_face);
		StopSpeckFromTalking();

		//make sure we are done the intro speech
		gfDoneIntroSpeech = TRUE;

		//remove the video conf mode
		gubCurrentMercVideoMode = MERC_VIDEO_EXIT_VIDEO_MODE;

		gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;
	}
}


static BOOLEAN DisplayMercVideoIntro(UINT16 usTimeTillFinish)
{
	UINT32	uiCurTime = GetJA2Clock();
	static UINT32	uiLastTime=0;

	//init variable
	if( uiLastTime == 0 )
		uiLastTime = uiCurTime;


	ColorFillVideoSurfaceArea( FRAME_BUFFER, MERC_VIDEO_FACE_X, MERC_VIDEO_FACE_Y, MERC_VIDEO_FACE_X+MERC_VIDEO_FACE_WIDTH,	MERC_VIDEO_FACE_Y+MERC_VIDEO_FACE_HEIGHT, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

	//if the intro is done
	if( (uiCurTime - uiLastTime) > usTimeTillFinish )
	{
		uiLastTime = 0;
		return(TRUE);
	}
	else
		return(FALSE);
}


static BOOLEAN ShouldSpeckStartTalkingDueToActionOnSubPage(void);


static void HandleTalkingSpeck(void)
{
	BOOLEAN fIsSpeckTalking = TRUE;

	switch( gubCurrentMercVideoMode )
	{
		//Init the video conferencing
		case MERC_VIDEO_INIT_VIDEO_MODE:
			//perform some opening animation.  When its done start Speck talking

			//if the intro is finished
			if( DisplayMercVideoIntro( MERC_INTRO_TIME ) )
			{
				//NULL out the string
				gsSpeckDialogueTextPopUp.clear();

				//Start speck talking
				if( !StartSpeckTalking( gusMercVideoSpeckSpeech ) )
				{
					gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;
				}

				gubCurrentMercVideoMode = MERC_VIDEO_VIDEO_MODE;
			}
			break;

		//Display his talking and blinking face
		case MERC_VIDEO_VIDEO_MODE:

			//Make sure the accounts button does not overwrite the dialog text
			//guiAccountBoxButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
			//def:

			if( (gfJustEnteredMercSite && gubArrivedFromMercSubSite != MERC_CAME_FROM_OTHER_PAGE) || gfFirstTimeIntoMERCSiteSinceEnteringLaptop )
			{
				gfFirstTimeIntoMERCSiteSinceEnteringLaptop = FALSE;
				GetSpeckConditionalOpening( FALSE );
				gfJustEnteredMercSite = FALSE;
			}
			else
			{
				fIsSpeckTalking = HandleSpeckTalking( FALSE );

				if( !fIsSpeckTalking )
					fIsSpeckTalking = GetSpeckConditionalOpening( FALSE );

				//if speck didnt start talking, see if he just hired someone
				if( !fIsSpeckTalking )
				{
					fIsSpeckTalking = ShouldSpeckStartTalkingDueToActionOnSubPage();
				}
			}

			if( !fIsSpeckTalking )
				gubCurrentMercVideoMode = MERC_VIDEO_EXIT_VIDEO_MODE;

			if( gfDisplaySpeckTextBox && gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
			{
				if( !gfInMercSite )
				{
					StopSpeckFromTalking( );
					return;
				}


				if (!gsSpeckDialogueTextPopUp.empty())
				{
					//guiAccountBoxButton->Draw();
					//guiAccountBoxButton->uiFlags |= BUTTON_FORCE_UNDIRTY;

					if (g_merc_popup_box)
					{
						guiAccountBoxButton->Draw();
						guiAccountBoxButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
						RenderMercPopUpBox(g_merc_popup_box, gusSpeckDialogueX, MERC_TEXT_BOX_POS_Y, FRAME_BUFFER);
					}
				}
			}

			break;

		// shut down the video conferencing
		case MERC_VIDEO_EXIT_VIDEO_MODE:

			//if the exit animation is finished, exit the video conf window
			if( DisplayMercVideoIntro( MERC_EXIT_TIME ) )
			{
				StopSpeckFromTalking( );

				//Delete the face
				DeleteFace(g_video_speck_face);
				InitDestroyXToCloseVideoWindow( FALSE );

				gfRedrawMercSite = TRUE;
				gfMercVideoIsBeingDisplayed = FALSE;

				//Remove the merc popup
				RemoveSpeckPopupTextBox();

				//maybe display ending animation
				gubCurrentMercVideoMode = MERC_VIDEO_NO_VIDEO_MODE;
			}
			else
			{
				//else we are done the exit animation.  The area is not being invalidated anymore
				InvalidateRegion( MERC_VIDEO_FACE_X, MERC_VIDEO_FACE_Y, MERC_VIDEO_FACE_X+MERC_VIDEO_FACE_WIDTH,	MERC_VIDEO_FACE_Y+MERC_VIDEO_FACE_HEIGHT );
			}
			break;
		}
}


static void MercSiteSubTitleRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void DisplayTextForSpeckVideoPopUp(const ST::string& str)
{
	UINT16	usActualHeight;

	//If the user has selected no subtitles
	if( !gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
		return;

	//add the "" around the speech.
	gsSpeckDialogueTextPopUp = ST::format("\"{}\"", str);

	gfDisplaySpeckTextBox = TRUE;

	//Set this so the popup box doesnt render in RenderMercs()
	MercPopUpBox* const old_merc_popup_box = g_merc_popup_box;
	g_merc_popup_box = 0;

	//Render the screen to get rid of any old text popup boxes
	RenderMercs();

	g_merc_popup_box = old_merc_popup_box;

	if( gfMercVideoIsBeingDisplayed && gfMercSiteScreenIsReDrawn )
	{
		DrawMercVideoBackGround();
	}

	//Create the popup box
	g_merc_popup_box = PrepareMercPopupBox(g_merc_popup_box, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, gsSpeckDialogueTextPopUp, 300, 0, 0, 0, &gusSpeckDialogueActualWidth, &usActualHeight);

	gusSpeckDialogueX = ( LAPTOP_SCREEN_LR_X - gusSpeckDialogueActualWidth - LAPTOP_SCREEN_UL_X ) / 2 + LAPTOP_SCREEN_UL_X;

	//Render the pop box
	RenderMercPopUpBox(g_merc_popup_box, gusSpeckDialogueX, MERC_TEXT_BOX_POS_Y, FRAME_BUFFER);

	//check to make sure the region is not already initialized
	if( !( gMercSiteSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS ) )
	{
		MSYS_DefineRegion(&gMercSiteSubTitleMouseRegion, gusSpeckDialogueX, MERC_TEXT_BOX_POS_Y,
					(INT16)(gusSpeckDialogueX + gusSpeckDialogueActualWidth),
					(INT16)(MERC_TEXT_BOX_POS_Y + usActualHeight), MSYS_PRIORITY_HIGH,
					CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, MercSiteSubTitleRegionCallBack);
	}
}

static BOOLEAN AreAnyOfTheNewMercsAvailable(void);
static UINT32 CalcMercDaysServed(void);
static BOOLEAN CanMercBeAvailableYet(const MERCListingModel* mercToCheck);
static UINT8 CountNumberOfMercMercsHired(void);
static UINT8 CountNumberOfMercMercsWhoAreDead(void);
static BOOLEAN IsAnyMercMercsDead(void);
static BOOLEAN IsAnyMercMercsHired(void);
static void MakeBiffAwayForCoupleOfDays(void);


static BOOLEAN IsSpeckTryingToRecruit()
{
	auto listings = GCM->getMERCListings();
	if (LaptopSaveInfo.gubLastMercIndex >= (listings.size() - 1))
	{
		// we have got all mercs already
		return FALSE;
	}

	auto nextAvailable = listings[LaptopSaveInfo.gubLastMercIndex + 1];
	return CanMercBeAvailableYet(nextAvailable);
}

static BOOLEAN GetSpeckConditionalOpening(BOOLEAN fJustEnteredScreen)
{
	static UINT16	usQuoteToSay=MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;
	BOOLEAN	fCanSayLackOfPaymentQuote = TRUE;

	//If we just entered the screen, reset some variables
	if( fJustEnteredScreen )
	{
		gfDoneIntroSpeech = FALSE;
		usQuoteToSay = 0;
		return( FALSE );
	}

	//if we are done the intro speech, or arrived from a sub page, get out of the function
	if( gfDoneIntroSpeech || gubArrivedFromMercSubSite != MERC_CAME_FROM_OTHER_PAGE )
	{
		return( FALSE );
	}

	gfDoneIntroSpeech = TRUE;

	//set the opening quote based on if the player has been here before
	if( LaptopSaveInfo.ubPlayerBeenToMercSiteStatus == MERC_SITE_FIRST_VISIT && usQuoteToSay <= 8 )
	{
		StartSpeckTalking( usQuoteToSay );
		usQuoteToSay++;
		if( usQuoteToSay <= 8 )
			gfDoneIntroSpeech = FALSE;
	}

	//if its the players second visit
	else if( LaptopSaveInfo.ubPlayerBeenToMercSiteStatus == MERC_SITE_SECOND_VISIT )
	{
		StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_1_TOUGH_START );
	}

	// We have been here at least 2 times before, Check which quote we should use
	else
	{
		BOOLEAN fCanUseIdleTag = FALSE;

		//if the player has not hired any MERC mercs before
		// CJC Dec 1 2002: fixing this, so near-bankrupt msg will play
		if( !IsAnyMercMercsHired( ) && CalcMercDaysServed() == 0)
		{
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_2_BUSINESS_BAD );
		}

		//else if it is the first visit since the server went down
		else if( LaptopSaveInfo.fFirstVisitSinceServerWentDown == TRUE )
		{
			LaptopSaveInfo.fFirstVisitSinceServerWentDown = 2;
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_9_FIRST_VISIT_SINCE_SERVER_WENT_DOWN );
			fCanUseIdleTag = TRUE;
		}
		/*
		//else if new mercs are available
		else if( LaptopSaveInfo.fNewMercsAvailableAtMercSite )
		{
			LaptopSaveInfo.fNewMercsAvailableAtMercSite = FALSE;

			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_11_NEW_MERCS_AVAILABLE );
		}*/
		//else if lots of MERC mercs are DEAD, and speck can say the quote ( dont want him to continously say it )
		else if( CountNumberOfMercMercsWhoAreDead() >= 2 && LaptopSaveInfo.ubSpeckCanSayPlayersLostQuote )
		{
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_12_PLAYERS_LOST_MERCS );

			//Set it so speck Wont say the quote again till someone else dies
			LaptopSaveInfo.ubSpeckCanSayPlayersLostQuote = 0;
		}

		//else if player owes lots of money
		else if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_SUSPENDED )
		{
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_5_PLAYER_OWES_SPECK_ACCOUNT_SUSPENDED );

			fCanSayLackOfPaymentQuote = FALSE;
		}

		//else if the player owes speck a large sum of money, have speck say so
		else if( CalculateHowMuchPlayerOwesSpeck() > 5000 )
		{
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_6_PLAYER_OWES_SPECK_ALMOST_BANKRUPT_1 );
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_6_PLAYER_OWES_SPECK_ALMOST_BANKRUPT_2 );

			fCanSayLackOfPaymentQuote = FALSE;
		}

		else
		{
			UINT8	ubRandom = ( UINT8 ) Random( 100 );

			//if business is good
			if( ubRandom < 40 && AreAnyOfTheNewMercsAvailable() && CountNumberOfMercMercsHired() > 1 )
			{
				StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_3_BUSINESS_GOOD );
				fCanUseIdleTag = TRUE;
			}

			//or if still trying to recruit ( the last recruit hasnt arrived and the player has paid for some of his mercs )
			else if( ubRandom < 80 && IsSpeckTryingToRecruit())
			{
				StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_4_TRYING_TO_RECRUIT );
				fCanUseIdleTag = TRUE;
			}

			//else use the generic opening
			else
			{
				StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_10_GENERIC_OPENING );
				fCanUseIdleTag = TRUE;

				//if the  merc hasnt said the line before
				if( !LaptopSaveInfo.fSaidGenericOpeningInMercSite )
				{
					LaptopSaveInfo.fSaidGenericOpeningInMercSite = TRUE;
					StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_10_TAG_FOR_20 );
				}
			}
		}

		if( fCanUseIdleTag )
		{
			if (Chance(50))
			{
				// Ensure these quotes are contiguous
				static_assert(SPECK_QUOTE_ALTERNATE_OPENING_TAG_ON_AFTER_OTHER_TAGS_1 ==
				              SPECK_QUOTE_ALTERNATE_OPENING_TAG_ON_AFTER_OTHER_TAGS_4 - 3);
				StartSpeckTalking(SPECK_QUOTE_ALTERNATE_OPENING_TAG_ON_AFTER_OTHER_TAGS_1 + Random(4));
			}
		}
	}


	//If Speck has sent an email to the player, and the player still hasnt paid, has speck complain about it.
	// CJC Dec 1 2002 ACTUALLY HOOKED IN THAT CHECK
	if (fCanSayLackOfPaymentQuote)
	{
		if( LaptopSaveInfo.uiSpeckQuoteFlags & SPECK_QUOTE__SENT_EMAIL_ABOUT_LACK_OF_PAYMENT )
		{
			LaptopSaveInfo.uiSpeckQuoteFlags &= ~SPECK_QUOTE__SENT_EMAIL_ABOUT_LACK_OF_PAYMENT;

			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_TAG_PLAYER_OWES_MONEY );
		}
	}

	//if new mercs are available
	if( LaptopSaveInfo.fNewMercsAvailableAtMercSite )
	{
		LaptopSaveInfo.fNewMercsAvailableAtMercSite = FALSE;

		StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_11_NEW_MERCS_AVAILABLE );
	}

	//if any mercs are dead
	if( IsAnyMercMercsDead() )
	{
		//if no merc has died before
		if( !LaptopSaveInfo.fHasAMercDiedAtMercSite )
		{
			LaptopSaveInfo.fHasAMercDiedAtMercSite = TRUE;
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_TAG_FIRST_MERC_DIES );
		}

		//loop through all the mercs and see if any are dead and the quote is not said
		for (const MERCListingModel* listing : GCM->getMERCListings())
		{
			ProfileID profileID = GetProfileIDFromMERCListing(listing);
			MERCPROFILESTRUCT& p = GetProfile(profileID);
			if (!IsMercDead(p)) continue;

			//if the quote has not been said
			if (p.ubMiscFlags3 & PROFILE_MISC_FLAG3_MERC_MERC_IS_DEAD_AND_QUOTE_SAID) continue;
			//set the flag
			p.ubMiscFlags3 |= PROFILE_MISC_FLAG3_MERC_MERC_IS_DEAD_AND_QUOTE_SAID;

			if (profileID == FLO) // special handling for FLO
			{
				//if biff is dead
				if (IsMercDead(GetProfile(BIFF)))
					StartSpeckTalking(SPECK_QUOTE_ALTERNATE_OPENING_TAG_FLO_IS_DEAD_BIFF_IS_DEAD);
				else
				{
					StartSpeckTalking(SPECK_QUOTE_ALTERNATE_OPENING_TAG_FLO_IS_DEAD_BIFF_ALIVE);
					MakeBiffAwayForCoupleOfDays();
				}
				continue;
			}

			const std::vector<SpeckQuote> quotes = listing->getQuotesByType(SpeckQuoteType::MERC_DEAD);
			if (!quotes.empty()) // other mercs
			{
				StartSpeckTalking(quotes[0]->quoteID);
			}
		}
	}


	//if flo has married the cousin
	if( gubFact[ FACT_PC_MARRYING_DARYL_IS_FLO ] )
	{
		//if speck hasnt said the quote before, and Biff is NOT dead
		if (!LaptopSaveInfo.fSpeckSaidFloMarriedCousinQuote && !IsMercDead(GetProfile(BIFF)))
		{
			StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_TAG_FLO_MARRIED_A_COUSIN_BIFF_IS_ALIVE );
			LaptopSaveInfo.fSpeckSaidFloMarriedCousinQuote = TRUE;

			MakeBiffAwayForCoupleOfDays();
		}
	}


	//if larry has relapsed
	if( HasLarryRelapsed() && !( LaptopSaveInfo.uiSpeckQuoteFlags & SPECK_QUOTE__ALREADY_TOLD_PLAYER_THAT_LARRY_RELAPSED ) )
	{
		LaptopSaveInfo.uiSpeckQuoteFlags |= SPECK_QUOTE__ALREADY_TOLD_PLAYER_THAT_LARRY_RELAPSED;

		StartSpeckTalking( SPECK_QUOTE_ALTERNATE_OPENING_TAG_LARRY_RELAPSED );
	}

	return( TRUE );
}


static BOOLEAN IsAnyMercMercsHired(void)
{
	//loop through all of the hired mercs from M.E.R.C.
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		ProfileID ubMercID = GetProfileIDFromMERCListing(m);
		if( IsMercOnTeam( ubMercID ) )
		{
			return( TRUE );
		}
	}

	return( FALSE );
}


static BOOLEAN IsAnyMercMercsDead(void)
{
	//loop through all of the hired mercs from M.E.R.C.
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		ProfileID mercID = GetProfileIDFromMERCListing(m);
		if (GetProfile(mercID).bMercStatus == MERC_IS_DEAD)
			return( TRUE );
	}

	return( FALSE );
}


static UINT8 CountNumberOfMercMercsHired(void)
{
	UINT8	ubCount=0;

	//loop through all of the hired mercs from M.E.R.C
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		ProfileID ubMercID = GetProfileIDFromMERCListing(m);
		if( IsMercOnTeam( ubMercID ) )
		{
			ubCount++;
		}
	}

	return( ubCount );
}


static UINT8 CountNumberOfMercMercsWhoAreDead(void)
{
	UINT8	ubCount=0;

	//loop through all of the hired mercs from M.E.R.C.
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		ProfileID ubMercID = GetProfileIDFromMERCListing(m);
		if (GetProfile(ubMercID).bMercStatus == MERC_IS_DEAD)
		{
			ubCount++;
		}
	}

	return( ubCount );
}


//Mouse Call back for the pop up text box
static void MercSiteSubTitleRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_ANY_BUTTON_UP)
	{
		StopSpeckFromTalking( );
	}
}


static void RemoveSpeckPopupTextBox(void)
{
	if (!g_merc_popup_box) return;

	if( gMercSiteSubTitleMouseRegion.uiFlags & MSYS_REGION_EXISTS )
		MSYS_RemoveRegion( &gMercSiteSubTitleMouseRegion );

	RemoveMercPopupBox(g_merc_popup_box);
	g_merc_popup_box = 0;

	//redraw the screen
	gfRedrawMercSite = TRUE;
}


static BOOLEAN IsMercMercAvailable(UINT8 ubMercID);


static void HandlePlayerHiringMerc(const MERCListingModel* hired)
{
	gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;

	auto quotes = hired->getQuotesByType(SpeckQuoteType::CROSS_SELL);
	for (SpeckQuote const& quote : quotes)
	{
		if (IsMercMercAvailable(quote->relatedMercID))
		{
			StartSpeckTalking(quote->quoteID);
		}
	}

	gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;
}

// Checks if the merc is available for hire right now
static BOOLEAN IsMercMercAvailable(UINT8 ubMercID)
{
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		if (m->index > LaptopSaveInfo.gubLastMercIndex)
		{
			// Not available in M.E.R.C. yet
			break;
		}

		//if this is the merc
		if (GetProfileIDFromMERCListing(m) == ubMercID)
		{
			//if the merc is available, and Not dead
			if (IsMercHireable(GetProfile(ubMercID)))
				return( TRUE );
		}
	}

	return( FALSE );
}


static BOOLEAN ShouldSpeckStartTalkingDueToActionOnSubPage(void)
{
	//if the merc came from the hire screen
	if( gfJustHiredAMercMerc )
	{
		auto listing = GCM->getMERCListings();
		HandlePlayerHiringMerc(listing[gubCurMercIndex]);

		//get speck to say the thank you
		StartSpeckTalking(CoinToss()
			? SPECK_QUOTE_GENERIC_THANKS_FOR_HIRING_MERCS_1
			: SPECK_QUOTE_GENERIC_THANKS_FOR_HIRING_MERCS_2);

		gfJustHiredAMercMerc = FALSE;
		//gfDoneIntroSpeech = TRUE;

		return( TRUE );
	}

	return( FALSE );
}


static BOOLEAN ShouldSpeckSayAQuote(void)
{
	//if we are entering from anywhere except a sub page, and we should say the opening quote
	if( gfJustEnteredMercSite && gubArrivedFromMercSubSite == MERC_CAME_FROM_OTHER_PAGE )
	{
		//if the merc has something to say
		if( gusMercVideoSpeckSpeech != MERC_VIDEO_SPECK_SPEECH_NOT_TALKING )
			return( FALSE );
	}


	//if the player just hired a merc
	if( gfJustHiredAMercMerc )
	{
		gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_HAS_TO_TALK_BUT_QUOTE_NOT_CHOSEN_YET;
		return( TRUE );

		/*
		//if the merc has something to say
		if( gusMercVideoSpeckSpeech != MERC_VIDEO_SPECK_SPEECH_NOT_TALKING )
			return( TRUE );
		else
		{
			gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_HAS_TO_TALK_BUT_QUOTE_NOT_CHOSEN_YET;
			return( TRUE );
		}*/
	}

	//If it is the first time into the merc site
	if( gfFirstTimeIntoMERCSiteSinceEnteringLaptop )
	{
		//gfFirstTimeIntoMERCSiteSinceEnteringLaptop = FALSE;
		gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_HAS_TO_TALK_BUT_QUOTE_NOT_CHOSEN_YET;
		return( TRUE );
	}

	/*
	//if we are entering from anywhere except a sub page
	if( gubArrivedFromMercSubSite == MERC_CAME_FROM_OTHER_PAGE )
	{
		GetSpeckConditionalOpening( FALSE );
		return( TRUE );
	}*/
	return( FALSE );
}


static INT16 GetRandomQuoteThatHasBeenSaidTheLeast(void);
static void IncreaseMercRandomQuoteValue(UINT8 ubQuoteID, UINT8 ubValue);


static void HandleSpeckIdleConversation(BOOLEAN fReset)
{
	static UINT32	uiLastTime=0;
	UINT32	uiCurTime = GetJA2Clock();
	INT16		sLeastSaidQuote;

	//if we should reset the variables
	if( fReset )
	{
		uiLastTime = GetJA2Clock();
		return;
	}



	if( ( uiCurTime - uiLastTime ) > SPECK_IDLE_CHAT_DELAY )
	{

		//if Speck is not talking
		if( !gfMercVideoIsBeingDisplayed )
		{
			sLeastSaidQuote = GetRandomQuoteThatHasBeenSaidTheLeast( );

			if( sLeastSaidQuote != -1 )
				gusMercVideoSpeckSpeech = (UINT8)sLeastSaidQuote;

			//Say the aim slander quotes the least
			if( sLeastSaidQuote >= 47 && sLeastSaidQuote <= 57 )
			{
				IncreaseMercRandomQuoteValue( (UINT8)sLeastSaidQuote, 1 );
			}
			else if( sLeastSaidQuote != -1 )
				IncreaseMercRandomQuoteValue( (UINT8)sLeastSaidQuote, 3 );
		}

		uiLastTime = GetJA2Clock();
	}
}


static std::vector<UINT8> GetAvailableRandomQuotes();

static INT16 GetRandomQuoteThatHasBeenSaidTheLeast()
{
	// random as in random chatter, there has never been any random selection in this fnuction
	UINT32 sLeastSaidSoFar = UINT_MAX;
	std::vector<UINT8> quotes = GetAvailableRandomQuotes();

	for (UINT32 quoteID : quotes)
	{
		if (sLeastSaidSoFar == UINT_MAX ||
			gNumberOfTimesQuoteSaid[quoteID] < gNumberOfTimesQuoteSaid[sLeastSaidSoFar])
		{	// if this quote has been said less times then the last one, or this is the first one we check
			sLeastSaidSoFar = quoteID;
		}
	}

	if (sLeastSaidSoFar == UINT_MAX)
		return -1;
	else
		return sLeastSaidSoFar;
}


static void IncreaseMercRandomQuoteValue(UINT8 ubQuoteID, UINT8 ubValue)
{
	gNumberOfTimesQuoteSaid[ubQuoteID] += ubValue;
}


static void StopSpeckFromTalking(void)
{
	if (g_video_speck_face == NULL) return;

	//Stop speck from talking
	ShutupaYoFace(g_video_speck_face);

	RemoveSpeckPopupTextBox();

	gusMercVideoSpeckSpeech = MERC_VIDEO_SPECK_SPEECH_NOT_TALKING;
}


static BOOLEAN HasLarryRelapsed(void)
{
	return( CheckFact( FACT_LARRY_CHANGED, 0 ) );
}


//Gets Called on each enter into laptop.
void EnterInitMercSite()
{
	gfFirstTimeIntoMERCSiteSinceEnteringLaptop = TRUE;
	gubCurMercIndex = 0;
}


static BOOLEAN ShouldTheMercSiteServerGoDown(void)
{
	UINT32	uiDay = GetWorldDay();

	// If the merc site has never gone down, the first new merc has shown ( which shows the player is using the site ),
	// and the players account status is ok ( cant have the merc site going down when the player owes him money, player may lose account that way )
	if( !LaptopSaveInfo.fMercSiteHasGoneDownYet  && LaptopSaveInfo.ubLastMercAvailableId >= 1 && LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_VALID )
	{
		if( Random( 100 ) < ( uiDay * 2 + 10 ) )
		{
			return( TRUE );
		}
		else
		{
			return( FALSE );
		}
	}

	return( FALSE );
}


void GetMercSiteBackOnline()
{
	//Add an email telling the user the site is back up
	AddEmail( MERC_NEW_SITE_ADDRESS, MERC_NEW_SITE_ADDRESS_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin() );

	//Set a flag indicating that the server just went up ( so speck can make a comment when the player next visits the site )
	LaptopSaveInfo.fFirstVisitSinceServerWentDown = TRUE;
}


static void DrawMercVideoBackGround(void)
{
	BltVideoObject(FRAME_BUFFER, guiMercVideoPopupBackground, 0, MERC_VIDEO_BACKGROUND_X, MERC_VIDEO_BACKGROUND_Y);

	//put the title on the window
	DrawTextToScreen(MercHomePageText[MERC_SPECK_COM], MERC_X_VIDEO_TITLE_X, MERC_X_VIDEO_TITLE_Y, 0, MERC_VIDEO_TITLE_FONT, MERC_VIDEO_TITLE_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	InvalidateRegion(MERC_VIDEO_BACKGROUND_X, MERC_VIDEO_BACKGROUND_Y, (MERC_VIDEO_BACKGROUND_X + MERC_VIDEO_BACKGROUND_WIDTH), (MERC_VIDEO_BACKGROUND_Y + MERC_VIDEO_BACKGROUND_HEIGHT) );
}

void DisableMercSiteButton()
{
	if (!g_merc_popup_box) return;
	guiAccountBoxButton->uiFlags |= BUTTON_FORCE_UNDIRTY;
}

static std::vector<UINT8> GetAvailableRandomQuotes()
{
	std::vector<UINT8> quotes = {};
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		if (!IsMercMercAvailable(GetProfileIDFromMERCListing(m))) continue;
		for (auto const& q : m->getQuotesByType(SpeckQuoteType::ADVERTISE))
		{
			quotes.push_back(q->quoteID);
		}
	}

	// A.I.M. slanders are always available
	quotes.push_back(SPECK_QUOTE_PLAYER_NOT_DOING_ANYTHING_AIM_SLANDER_1);
	quotes.push_back(SPECK_QUOTE_PLAYER_NOT_DOING_ANYTHING_AIM_SLANDER_2);
	quotes.push_back(SPECK_QUOTE_PLAYER_NOT_DOING_ANYTHING_AIM_SLANDER_3);
	quotes.push_back(SPECK_QUOTE_PLAYER_NOT_DOING_ANYTHING_AIM_SLANDER_4);

	return quotes;
}


static void MakeBiffAwayForCoupleOfDays(void)
{
	gMercProfiles[ BIFF ].uiDayBecomesAvailable = Random( 2 ) + 2;
}


// used in determining Speck's welcome quote
static BOOLEAN AreAnyOfTheNewMercsAvailable(void)
{
	if( LaptopSaveInfo.fNewMercsAvailableAtMercSite )
		return( FALSE );

	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		if (m->isAvailableAtStart()) continue;

		ProfileID ubMercID = GetProfileIDFromMERCListing(m);
		if( IsMercMercAvailable( ubMercID ) )
			return( TRUE );
	}

	return( FALSE );
}

// Check if any new merc is about to become available
static void ScheduleNewMercsToBeAvailable(void)
{
	BOOLEAN fNewMercAreAvailable = FALSE;
	for (const MERCListingModel* m : GCM->getMERCListings())
	{
		if (CanMercBeAvailableYet(m)) fNewMercAreAvailable = TRUE;
	}

	//if there is a new merc available
	if( fNewMercAreAvailable )
	{
		//Set up an event to add the merc in x days
		AddStrategicEvent( EVENT_MERC_SITE_NEW_MERC_AVAILABLE, GetMidnightOfFutureDayInMinutes( 1 ) + 420 + Random( 3 * 60 ), 0 );
	}
}

// Checks if the merc is not yet available for hire, but the conditions for availablilty is already met
static BOOLEAN CanMercBeAvailableYet(const MERCListingModel* merc)
{
	//if the merc is already available
	if (merc->index <= LaptopSaveInfo.gubLastMercIndex)
	{
		return( FALSE );
	}

	//if the merc is already hired
	ProfileID profileID = GetProfileIDFromMERCListing(merc);
	if (!IsMercHireable(GetProfile(profileID)))
		return( FALSE );

	//if player has paid enough money for the merc to be available, and the it is after the current day
	if (merc->minTotalSpending <= LaptopSaveInfo.uiTotalMoneyPaidToSpeck
		&& merc->minDays <= GetWorldDay())
	{
		return( TRUE );
	}

	return( FALSE );
}

// update the value of ubLastMercAvailableId for save game compatability
static void SetLastMercArrival(const MERCListingModel* merc)
{
	switch (merc->profileID)
	{
	case BUBBA:        LaptopSaveInfo.ubLastMercAvailableId = MERC_ARRIVES_BUBBA; break;
	case COUGAR:       LaptopSaveInfo.ubLastMercAvailableId = MERC_ARRIVES_COUGAR; break;
	case NUMB:         LaptopSaveInfo.ubLastMercAvailableId = MERC_ARRIVES_NUMB; break;
	case LARRY_NORMAL: LaptopSaveInfo.ubLastMercAvailableId = MERC_ARRIVES_LARRY; break;
	}
}

// sync last available M.E.R.C. listing from ubLastMercAvailableId for vanilla save compatability
void SyncLastMercFromSaveGame()
{
	// This allows us to load saves from old versions and vanilla
	//
	// In the old implementation, the M.E.R.C. list has 2 LARRY profiles (NORMAL and DRUNK), so
	// the index is off by 1 for mercs after LARRY
	//
	// But this only accounts version upgrade. If you are downgrading, you may need to wait for
	// a few days before the last merc becomes available again.
	int iLastMercID = -1;
	switch (LaptopSaveInfo.ubLastMercAvailableId)
	{
	case MERC_ARRIVES_NUMB:   iLastMercID = NUMB; break;
	case MERC_ARRIVES_COUGAR: iLastMercID = COUGAR; break;
	}

	if (iLastMercID <= 0) return;

	for (const MERCListingModel* merc : GCM->getMERCListings())
	{
		// check for exactly the case that we are off by 1 due to LARRY
		if (merc->profileID == (UINT8)iLastMercID &&
			(LaptopSaveInfo.gubLastMercIndex - merc->index) == 1)
		{
			// adjust gubLastMercIndex to match the new listing
			LaptopSaveInfo.gubLastMercIndex -= 1;
			break;
		}
	}
}

// make all can-be-available mercs available immediately
static void MakeNewMercsAvailable(BOOLEAN fShouldNotifyPlayer)
{
	BOOLEAN fNewMercAvailable = FALSE;
	for (const MERCListingModel* merc : GCM->getMERCListings())
	{
		if (CanMercBeAvailableYet(merc) && LaptopSaveInfo.gubLastMercIndex < merc->index)
		{
			LaptopSaveInfo.gubLastMercIndex = merc->index;
			SetLastMercArrival(merc);
			fNewMercAvailable = TRUE;
		}
	}

	if (fNewMercAvailable && fShouldNotifyPlayer)
	{
		AddEmail(NEW_MERCS_AT_MERC, NEW_MERCS_AT_MERC_LENGTH, SPECK_FROM_MERC, GetWorldTotalMin());

		//new mercs are available
		LaptopSaveInfo.fNewMercsAvailableAtMercSite = TRUE;
	}
}

void NewMercsAvailableAtMercSiteCallBack()
{
	MakeNewMercsAvailable(TRUE);
}

//used for older saves
void CalcAproximateAmountPaidToSpeck()
{
	//loop through all the mercs and tally up the amount speck should have been paid
	for (const MERCListingModel* merc : GCM->getMERCListings())
	{
		//get the id
		ProfileID ubMercID = GetProfileIDFromMERCListing(merc);
		//increment the amount
		LaptopSaveInfo.uiTotalMoneyPaidToSpeck += gMercProfiles[ ubMercID ].uiTotalCostToDate;
	}
}


// CJC Dec 1 2002: calculate whether any MERC characters have been used at all
static UINT32 CalcMercDaysServed(void)
{
	UINT32 uiDaysServed = 0;
	for (const MERCListingModel* merc : GCM->getMERCListings())
	{
		//get the id
		ProfileID ubMercID = GetProfileIDFromMERCListing(merc);
		uiDaysServed += gMercProfiles[ ubMercID ].usTotalDaysServed;

	}
	return( uiDaysServed );
}
