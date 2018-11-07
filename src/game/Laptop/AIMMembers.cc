#include "Cursors.h"
#include "Directories.h"
#include "EMail.h"
#include "Font.h"
#include "HImage.h"
#include "ItemModel.h"
#include "Laptop.h"
#include "AIMMembers.h"
#include "AIM.h"
#include "Local.h"
#include "MercPortrait.h"
#include "MessageBoxScreen.h"
#include "Timer_Control.h"
#include "SysUtil.h"
#include "Soldier_Profile.h"
#include "Soldier_Control.h"
#include "Interface_Items.h"
#include "WordWrap.h"
#include "Finances.h"
#include "VSurface.h"
#include "VObject.h"
#include "Game_Clock.h"
#include "Faces.h"
#include "Dialogue_Control.h"
#include "Text.h"
#include "History.h"
#include "Game_Event_Hook.h"
#include "MercTextBox.h"
#include "Render_Dirty.h"
#include "Soldier_Add.h"
#include "Merc_Hiring.h"
#include "Strategic.h"
#include "AIMFacialIndex.h"
#include "LaptopSave.h"
#include "GameSettings.h"
#include "Random.h"
#include "Strategic_Status.h"
#include "Merc_Contract.h"
#include "Assignments.h"
#include "StrategicMap.h"
#include "Sound_Control.h"
#include "Quests.h"
#include "Button_System.h"
#include "Video.h"
#include "SoundMan.h"
#include "ScreenIDs.h"
#include "Font_Control.h"

#include "game/GameRes.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"

#include <string_theory/format>
#include <string_theory/string>

#include <stdexcept>


#define MERCBIOSFILENAME			BINARYDATADIR "/aimbios.edt"



#define AIM_M_FONT_PREV_NEXT_CONTACT		FONT14ARIAL
#define AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_UP	FONT_MCOLOR_DKWHITE
#define AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_DOWN	138
#define AIM_M_FONT_STATIC_TEXT			FONT12ARIAL
#define AIM_M_COLOR_STATIC_TEXT			146//75
#define AIM_M_FONT_DYNAMIC_TEXT			FONT10ARIAL
#define AIM_M_COLOR_DYNAMIC_TEXT		FONT_MCOLOR_WHITE
#define AIM_M_WEAPON_TEXT_FONT			FONT10ARIAL
#define AIM_M_WEAPON_TEXT_COLOR			FONT_MCOLOR_WHITE
#define AIM_M_NUMBER_FONT			FONT12ARIAL
#define AIM_M_ACTIVE_MEMBER_TITLE_COLOR		AIM_GREEN
#define AIM_M_FEE_CONTRACT_COLOR		AIM_GREEN
#define AIM_M_VIDEO_TITLE_COLOR			AIM_FONT_GOLD
#define AIM_M_VIDEO_NAME_COLOR			FONT_MCOLOR_BLACK
#define AIM_M_VIDEO_NAME_SHADOWCOLOR		AIM_FONT_GOLD


#define AIM_M_VIDEO_CONTRACT_AMOUNT_FONT	FONT10ARIAL
#define AIM_M_VIDEO_CONTRACT_AMOUNT_COLOR	183

#define AIM_POPUP_BOX_FONT			FONT12ARIAL
#define AIM_POPUP_BOX_COLOR			FONT_MCOLOR_BLACK

#define HIGH_STAT_COLOR				FONT_MCOLOR_WHITE//FONT_MCOLOR_LTGREEN
#define MED_STAT_COLOR				FONT_MCOLOR_DKWHITE//FONT_MCOLOR_WHITE
#define LOW_STAT_COLOR				FONT_MCOLOR_DKWHITE//FONT_MCOLOR_DKGRAY

#define SIZE_MERC_BIO_INFO			400
#define SIZE_MERC_ADDITIONAL_INFO		160

#define MERC_ANNOYED_WONT_CONTACT_TIME_MINUTES	6 * 60
#define NUMBER_HATED_MERCS_ONTEAM		3


#define STATS_X					IMAGE_OFFSET_X + 121
#define STATS_Y					IMAGE_OFFSET_Y + 66//69

#define PRICE_X					IMAGE_OFFSET_X + 377
#define PRICE_Y					STATS_Y
#define PRICE_WIDTH				116

#define PORTRAIT_X				IMAGE_OFFSET_X + 8
#define PORTRAIT_Y				STATS_Y
#define PORTRAIT_WIDTH				110
#define PORTRAIT_HEIGHT				126

#define FACE_X					PORTRAIT_X + 2
#define FACE_Y					PORTRAIT_Y + 2
#define FACE_WIDTH				106
#define FACE_HEIGHT				122


#define WEAPONBOX_X				IMAGE_OFFSET_X + 6
#define WEAPONBOX_Y				IMAGE_OFFSET_Y + 296//299
#define WEAPONBOX_SIZE_X			61
#define WEAPONBOX_SIZE_Y			31
#define WEAPONBOX_NUMBER			8

#define SPACE_BN_LINES				15//13
#define STATS_FIRST_COL				STATS_X + 9
#define STATS_SECOND_COL			STATS_FIRST_COL + 129
#define STAT_NAME_WIDTH				87
#define STAT_VALUE_DX				102

#define HEALTH_Y				STATS_Y + 34
#define AGILITY_Y				HEALTH_Y	+ SPACE_BN_LINES
#define DEXTERITY_Y				AGILITY_Y + SPACE_BN_LINES
#define STRENGTH_Y				DEXTERITY_Y + SPACE_BN_LINES
#define LEADERSHIP_Y				STRENGTH_Y + SPACE_BN_LINES
#define WISDOM_Y				LEADERSHIP_Y + SPACE_BN_LINES

#define EXPLEVEL_Y				HEALTH_Y
#define MARKSMAN_Y				AGILITY_Y
#define MECHANAICAL_Y				DEXTERITY_Y
#define EXPLOSIVE_Y				STRENGTH_Y
#define MEDICAL_Y				LEADERSHIP_Y

#define NAME_X					STATS_FIRST_COL
#define NAME_Y					STATS_Y + 7

#define FEE_X					PRICE_X + 7
#define FEE_Y					NAME_Y
#define FEE_WIDTH				37  //33

#define AIM_CONTRACT_X				PRICE_X + 51
#define AIM_CONTRACT_Y				FEE_Y
#define AIM_CONTRACT_WIDTH			59

#define ONEDAY_X				AIM_CONTRACT_X
#define ONEWEEK_X				AIM_CONTRACT_X
#define TWOWEEK_X				AIM_CONTRACT_X

#define PREVIOUS_X				(STD_SCREEN_X + 224)
#define CONTACT_X				(STD_SCREEN_X + 331)
#define NEXT_X					(STD_SCREEN_X + 431)
#define BTN_BOX_Y				(STD_SCREEN_Y + 386 + LAPTOP_SCREEN_WEB_DELTA_Y - 4)

#define AIM_MERC_INFO_X				(STD_SCREEN_X + 124)
#define AIM_MERC_INFO_Y				(STD_SCREEN_Y + 223 + LAPTOP_SCREEN_WEB_DELTA_Y)

#define AIM_MERC_ADD_X				AIM_MERC_ADD_INFO_X
#define AIM_MERC_ADD_Y				(STD_SCREEN_Y + 269 + LAPTOP_SCREEN_WEB_DELTA_Y)

#define AIM_MERC_ADD_INFO_X			AIM_MERC_INFO_X
#define AIM_MERC_ADD_INFO_Y			AIM_MERC_ADD_Y + 15
#define AIM_MERC_INFO_WIDTH			470

#define AIM_MEDICAL_DEPOSIT_X			PRICE_X + 5
#define AIM_MEDICAL_DEPOSIT_Y			LEADERSHIP_Y
#define AIM_MEDICAL_DEPOSIT_WIDTH		PRICE_WIDTH - 6

#define AIM_MEMBER_ACTIVE_TEXT_X		IMAGE_OFFSET_X + 149
#define AIM_MEMBER_ACTIVE_TEXT_Y		AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y	- 1	// + 1
#define AIM_MEMBER_ACTIVE_TEXT_WIDTH		AIM_SYMBOL_WIDTH

#define AIM_MEMBER_OPTIONAL_GEAR_X		AIM_MERC_INFO_X
#define AIM_MEMBER_OPTIONAL_GEAR_Y		WEAPONBOX_Y - 13

#define AIM_MEMBER_WEAPON_NAME_Y		WEAPONBOX_Y + WEAPONBOX_SIZE_Y + 1
#define AIM_MEMBER_WEAPON_NAME_WIDTH		WEAPONBOX_SIZE_X - 2

//video Conferencing Info
#define AIM_MEMBER_VIDEO_CONF_TERMINAL_X	(STD_SCREEN_X + 125)
#define AIM_MEMBER_VIDEO_CONF_TERMINAL_Y	(STD_SCREEN_Y + 97 + LAPTOP_SCREEN_WEB_DELTA_Y)

#define AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH	368
#define AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT	21
#define AIM_MEMBER_VIDEO_TITLE_ITERATIONS	18
#define AIM_MEMBER_VIDEO_TITLE_START_Y		(STD_SCREEN_Y + 382 + LAPTOP_SCREEN_WEB_DELTA_Y)


#define AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_X	AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 6
#define AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_Y	AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + 130

#define AIM_MEMBER_VIDEO_CONF_XCLOSE_X		AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 348
#define AIM_MEMBER_VIDEO_CONF_XCLOSE_Y		AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + 3

#define AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y	20

#define AIM_MEMBER_BUY_CONTRACT_LENGTH_X	AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 113
#define AIM_MEMBER_BUY_CONTRACT_LENGTH_Y	AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 15

#define AIM_MEMBER_BUY_EQUIPMENT_GAP		23

#define AIM_MEMBER_BUY_EQUIPMENT_X		AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 235

#define AIM_MEMBER_AUTHORIZE_PAY_X		AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 113
#define AIM_MEMBER_AUTHORIZE_PAY_Y		AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 92
#define AIM_MEMBER_AUTHORIZE_PAY_GAP		122

#define AIM_MEMBER_VIDEO_FACE_X			AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 7 + 1
#define AIM_MEMBER_VIDEO_FACE_Y			AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 6 + 1

#define AIM_MEMBER_VIDEO_FACE_WIDTH		96
#define AIM_MEMBER_VIDEO_FACE_HEIGHT		86

#define AIM_MEMBER_VIDEO_NAME_X			AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 7
#define AIM_MEMBER_VIDEO_NAME_Y			AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + 5

#define AIM_CONTRACT_CHARGE_X			AIM_MEMBER_VIDEO_NAME_X
#define AIM_CONTRACT_CHARGE_Y			AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 98

#define AIM_CONTRACT_LENGTH_ONE_DAY		0
#define AIM_CONTRACT_LENGTH_ONE_WEEK		1
#define AIM_CONTRACT_LENGTH_TWO_WEEKS		2

#define AIM_SELECT_LIGHT_ON_X			105
#define AIM_SELECT_LIGHT_ON_Y			8

#define AIM_SELECT_LIGHT_OFF_X			105
#define AIM_SELECT_LIGHT_OFF_Y			7

#define AIM_CONTRACT_CHARGE_AMOUNNT_X		AIM_MEMBER_VIDEO_CONF_TERMINAL_X + 7//8
#define AIM_CONTRACT_CHARGE_AMOUNNT_Y		AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 111//114

#define AIM_POPUP_BOX_X				(STD_SCREEN_X + 260)
#define AIM_POPUP_BOX_Y				(STD_SCREEN_Y + 140 + LAPTOP_SCREEN_WEB_DELTA_Y)

#define AIM_POPUP_BOX_WIDTH			162
#define AIM_POPUP_BOX_STRING1_Y			6
#define AIM_POPUP_BOX_BUTTON_OFFSET_X		20
#define AIM_POPUP_BOX_BUTTON_OFFSET_Y		62
#define AIM_POPUP_BOX_SUCCESS			0
#define AIM_POPUP_BOX_FAILURE			1

#define AIM_MEMBER_HANG_UP_X			(STD_SCREEN_X + 290)
#define AIM_MEMBER_HANG_UP_Y			AIM_MEMBER_VIDEO_CONF_TERMINAL_Y + AIM_MEMBER_VIDEO_CONF_TITLE_BAR_HEIGHT_Y + 42

#define VC_CONTACT_STATIC_TIME			30
#define VC_CONTACT_FUZZY_LINE_TIME		100
#define VC_NUM_LINES_SNOW			6
#define VC_NUM_FUZZ_LINES			10
#define VC_NUM_STRAIGHT_LINES			9

#define VC_ANSWER_IMAGE_DELAY			100


#define QUOTE_FIRST_ATTITUDE_TIME		3000
#define QUOTE_ATTITUDE_TIME			10000

#define QUOTE_DELAY_SMALL_TALK			1
#define QUOTE_DELAY_IMPATIENT_TALK		2
#define QUOTE_DELAY_VERY_IMPATIENT_TALK		3
#define QUOTE_DELAY_HANGUP_TALK			4
#define QUOTE_DELAY_NO_ACTION			5
#define QUOTE_MERC_BUSY	6

#define TEXT_POPUP_WINDOW_Y			(STD_SCREEN_Y + 255 + LAPTOP_SCREEN_WEB_DELTA_Y)
#define TEXT_POPUP_STRING_SIZE			512

#define MINIMUM_TALKING_TIME_FOR_MERC		1500

#define AIM_TEXT_SPEECH_MODIFIER		80


// Enumerated types used for the Pop Up Box
enum PopUpAction
{
	AIM_POPUP_NOTHING,
	AIM_POPUP_DISPLAY,
	AIM_POPUP_DELETE
};

// Enumerated Types used for the different types of video distortion applied to the video face
enum
{
	VC_NO_STATIC,
	VC_FUZZY_LINE,
	VC_STRAIGHTLINE,
	VC_STATIC_IMAGE,
	VC_BW_SNOW,
	VC_PIXELATE,
	VC_TRANS_SNOW_IN, // fade from clear to snowy
	VC_TRANS_SNOW_OUT, // fade from snowy to clear
};


//Image Identifiers
static SGPVObject* guiStats;
static SGPVObject* guiPrice;
static SGPVObject* guiPortrait;
static SGPVObject* guiWeaponBox;
//UINT32  guiVideoFace;
//UINT32 guiContactButton;
static SGPVObject* guiVideoConfPopup;
static SGPVObject* guiVideoConfTerminal;
static SGPVObject* guiPopUpBox;
static SGPVSurface* guiVideoFaceBackground;
static SGPVObject* guiBWSnow;
static SGPVObject* guiFuzzLine;
static SGPVObject* guiStraightLine;
static SGPVObject* guiTransSnow;
static SGPVObject* guiVideoContractCharge;
static SGPVSurface* guiVideoTitleBar;

static UINT8 gbCurrentSoldier = 0;
UINT8        gbCurrentIndex = 0;

AIMVideoMode        gubVideoConferencingMode         = AIM_VIDEO_NOT_DISPLAYED_MODE;
static AIMVideoMode gubVideoConferencingPreviousMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
static BOOLEAN      gfJustSwitchedVideoConferenceMode;

static BOOLEAN gfMercIsTalking=FALSE;
static BOOLEAN gfVideoFaceActive=FALSE;

static PopUpAction gubPopUpBoxAction = AIM_POPUP_NOTHING;
static BOOLEAN     gfRedrawScreen    = FALSE;
static UINT8       gubContractLength;
static BOOLEAN     gfBuyEquipment;
static INT32       giContractAmount  = 0;
static FACETYPE*   giMercFaceIndex;
static ST::string  gsTalkingMercText;
static UINT32      guiTimeThatMercStartedTalking;
static UINT32      guiLastHandleMercTime;
static BOOLEAN     gfFirstTimeInContactScreen;

static UINT8   gubCurrentCount;
static UINT8   gubCurrentStaticMode;
static UINT32  guiMercAttitudeTime;         // retains the amount of time the user is in a screen, if over a certain time, the merc gets miffed
static UINT8   gubMercAttitudeLevel;        // retains the current level the merc is  P.O.'ed at the caller.
static BOOLEAN gfHangUpMerc;                // if we have to cancel the video conferencing after the merc is finsihed talking
static BOOLEAN gfIsShutUpMouseRegionActive;
static BOOLEAN gfIsAnsweringMachineActive;
static BOOLEAN gfStopMercFromTalking;

static UINT16 usAimMercSpeechDuration=0;

static BOOLEAN gfIsNewMailFlagSet = FALSE;

extern		BOOLEAN fExitDueToMessageBox;

static BOOLEAN gfWaitingForMercToStopTalkingOrUserToClick=FALSE;

static BOOLEAN gfAimMemberDisplayFaceHelpText = FALSE;


static BOOLEAN gfAimMemberCanMercSayOpeningQuote = TRUE;


static BUTTON_PICS* guiPreviousContactNextButtonImage;
static GUIButtonRef giPreviousButton;
static GUIButtonRef giContactButton;
static GUIButtonRef giNextButton;

//Video conference buttons
static BUTTON_PICS* guiVideoConferenceButtonImage[3];

//Contract Length Button
static GUIButtonRef giContractLengthButton[3];

//BuyEquipment Button
static GUIButtonRef giBuyEquipmentButton[2];

//Authorize Payment Button
static GUIButtonRef giAuthorizeButton[2];

//Hang up Button
static GUIButtonRef giHangUpButton;

// PopupBox button
static GUIButtonRef guiPopUpOkButton;
static BUTTON_PICS* guiPopUpImage;

//Leave Message merc Button
static GUIButtonRef giAnsweringMachineButton[2];

//X to Close the video conference Button
static BUTTON_PICS* giXToCloseVideoConfButtonImage;
static GUIButtonRef giXToCloseVideoConfButton;


//Mouse Regions
//Clicking on guys Face
static MOUSE_REGION    gSelectedFaceRegion;

//Clicking To shut merc up
static MOUSE_REGION    gSelectedShutUpMercRegion;


void EnterInitAimMembers()
{
	gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
	gubVideoConferencingPreviousMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
	gfVideoFaceActive = FALSE;
	//fShouldMercTalk = FALSE;
	gubPopUpBoxAction = AIM_POPUP_NOTHING;
	gfRedrawScreen = FALSE;
	giContractAmount = 0;
	giMercFaceIndex = NULL;
	guiLastHandleMercTime = GetJA2Clock();
	gubCurrentCount = 0;
	gfFirstTimeInContactScreen = TRUE;

	//reset the variable so a pop up can be displyed this time in laptop
	LaptopSaveInfo.sLastHiredMerc.fHaveDisplayedPopUpInLaptop = FALSE;

	//reset the id of the last merc
	LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = -1;
}


static GUIButtonRef MakeButton(const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	GUIButtonRef const btn = CreateIconAndTextButton
	(
		guiPreviousContactNextButtonImage, text, AIM_M_FONT_PREV_NEXT_CONTACT,
		AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_UP,   DEFAULT_SHADOW,
		AIM_M_FONT_PREV_NEXT_CONTACT_COLOR_DOWN, DEFAULT_SHADOW,
		x, BTN_BOX_Y, MSYS_PRIORITY_HIGH, click
	);
	btn->SetCursor(CURSOR_WWW);
	return btn;
}


static void BtnContactButtonCallback( GUI_BUTTON* btn, UINT32 reason);
static void BtnNextButtonCallback(    GUI_BUTTON* btn, UINT32 reason);
static void BtnPreviousButtonCallback(GUI_BUTTON* btn, UINT32 reason);
static void InitDeleteVideoConferencePopUp(void);
static void InitVideoFace(UINT8 ubMercID);
static void SelectFaceMovementRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectFaceRegionCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectFaceRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectShutUpMercRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterAIMMembers()
{
	// Create a background video surface to blt the face onto
	guiVideoFaceBackground = AddVideoSurface(AIM_MEMBER_VIDEO_FACE_WIDTH, AIM_MEMBER_VIDEO_FACE_HEIGHT, PIXEL_DEPTH);

	// load the stats graphic and add it
	guiStats = AddVideoObjectFromFile(LAPTOPDIR "/stats.sti");

	// load the Price graphic and add it
	guiPrice = AddVideoObjectFromFile(LAPTOPDIR "/price.sti");

	// load the Portait graphic and add it
	guiPortrait = AddVideoObjectFromFile(LAPTOPDIR "/portrait.sti");

	// load the WeaponBox graphic and add it
	guiWeaponBox = AddVideoObjectFromFile(LAPTOPDIR "/weaponbox.sti");

	// load the videoconf Popup graphic and add it
	guiVideoConfPopup = AddVideoObjectFromFile(LAPTOPDIR "/videoconfpopup.sti");

	// load the video conf terminal graphic and add it
	guiVideoConfTerminal = AddVideoObjectFromFile(LAPTOPDIR "/videoconfterminal.sti");

	// load the background snow for the video conf terminal
	guiBWSnow = AddVideoObjectFromFile(LAPTOPDIR "/bwsnow.sti");

	// load the fuzzy line for the video conf terminal
	guiFuzzLine = AddVideoObjectFromFile(LAPTOPDIR "/fuzzline.sti");

	// load the line distortion for the video conf terminal
	guiStraightLine = AddVideoObjectFromFile(LAPTOPDIR "/lineinterference.sti");

	// load the translucent snow for the video conf terminal
	guiTransSnow = AddVideoObjectFromFile(LAPTOPDIR "/transsnow.sti");

	// load the translucent snow for the video conf terminal
	guiVideoContractCharge = AddVideoObjectFromFile(LAPTOPDIR "/videocontractcharge.sti");


	//** Mouse Regions **
	MSYS_DefineRegion(&gSelectedFaceRegion, PORTRAIT_X, PORTRAIT_Y,
				PORTRAIT_X + PORTRAIT_WIDTH , PORTRAIT_Y + PORTRAIT_HEIGHT, MSYS_PRIORITY_HIGH,
				CURSOR_WWW, SelectFaceMovementRegionCallBack, MouseCallbackPrimarySecondary(SelectFaceRegionCallBackPrimary, SelectFaceRegionCallBackSecondary));

	// if user clicks in the area, the merc will shut up!
	MSYS_DefineRegion(&gSelectedShutUpMercRegion, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y,
				LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH-1,
				CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, SelectShutUpMercRegionCallBack);
	//have it disbled at first
	gSelectedShutUpMercRegion.Disable();


	//Button Regions
	giXToCloseVideoConfButtonImage = LoadButtonImage(LAPTOPDIR "/x_button.sti", 0, 1);


	guiPreviousContactNextButtonImage = LoadButtonImage(LAPTOPDIR "/bottombuttons2.sti", 0, 1);

	giPreviousButton = MakeButton(CharacterInfo[AIM_MEMBER_PREVIOUS], PREVIOUS_X, BtnPreviousButtonCallback);
	giContactButton  = MakeButton(CharacterInfo[AIM_MEMBER_CONTACT],  CONTACT_X,  BtnContactButtonCallback);
	giNextButton     = MakeButton(CharacterInfo[AIM_MEMBER_NEXT],     NEXT_X,     BtnNextButtonCallback);

	gbCurrentSoldier = AimMercArray[gbCurrentIndex];

	gfStopMercFromTalking = FALSE;
	gubVideoConferencingMode = static_cast<AIMVideoMode>(giCurrentSubPage);
	gubVideoConferencingPreviousMode = AIM_VIDEO_NOT_DISPLAYED_MODE;

	// if we are re-entering but the video conference should still be up
	if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE)
	{
		//if we need to re initialize the talking face
		if( gubVideoConferencingMode !=  AIM_VIDEO_FIRST_CONTACT_MERC_MODE)
			InitVideoFace(gbCurrentSoldier);

		InitDeleteVideoConferencePopUp();
	}




	InitAimMenuBar();
	InitAimDefaults();

	//LoadTextMercPopupImages( BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER);

	RenderAIMMembers();
	gfIsNewMailFlagSet = FALSE;
	gfAimMemberCanMercSayOpeningQuote = TRUE;
}


static BOOLEAN DeleteAimPopUpBox();


void ExitAIMMembers()
{
	RemoveAimDefaults();

	//if we are exiting and the transfer of funds popup is enable, make sure we dont come back to it
	giCurrentSubPage = gubPopUpBoxAction != AIM_POPUP_NOTHING ?
		AIM_VIDEO_NOT_DISPLAYED_MODE : gubVideoConferencingMode;

	gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
	InitDeleteVideoConferencePopUp( );

	DeleteVideoSurface(guiVideoFaceBackground);

	delete guiStats;
	delete guiPrice;
	delete guiPortrait;
	delete guiWeaponBox;
	delete guiVideoConfPopup;
	delete guiVideoConfTerminal;
	delete guiBWSnow;
	delete guiFuzzLine;
	delete guiStraightLine;
	delete guiTransSnow;
	delete guiVideoContractCharge;

	UnloadButtonImage( guiPreviousContactNextButtonImage );
	UnloadButtonImage( giXToCloseVideoConfButtonImage );

	RemoveButton( giPreviousButton );
	RemoveButton( giContactButton );
	RemoveButton( giNextButton );

	MSYS_RemoveRegion( &gSelectedFaceRegion);
	MSYS_RemoveRegion( &gSelectedShutUpMercRegion);

	ExitAimMenuBar();

	DeleteAimPopUpBox();
}


static void DelayMercSpeech(UINT8 ubMercID, UINT16 usQuoteNum, UINT16 usDelay, BOOLEAN fNewQuote, BOOLEAN fReset);
static BOOLEAN DisplayTalkingMercFaceForVideoPopUp(const FACETYPE*);
static void HandleCurrentVideoConfMode(void);
static void HandleMercAttitude(void);
static void HandleVideoDistortion(void);
static void StopMercTalking(void);

void HandleAIMMembers()
{
	//determine if the merc has a quote that is waiting to be said
	DelayMercSpeech( 0, 0, 0, FALSE, FALSE );

	if( gfHangUpMerc && !gfMercIsTalking)
	{
		if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE &&
				gubPopUpBoxAction        != AIM_POPUP_DISPLAY)
		{
			gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
		}
		gfHangUpMerc = FALSE;
	}

	if( gfStopMercFromTalking )
	{
		StopMercTalking();
		gfStopMercFromTalking = FALSE;
		/*
		//if we were waiting for the merc to stop talking
		if( gfWaitingForMercToStopTalkingOrUserToClick )
		{
			gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
			gfWaitingForMercToStopTalkingOrUserToClick = FALSE;
		}*/
	}

	// If we have to change video conference modes, change to new mode
	if( gubVideoConferencingMode != gubVideoConferencingPreviousMode && gubPopUpBoxAction != AIM_POPUP_DISPLAY )
	{
		InitDeleteVideoConferencePopUp( );

		//if we are exiting to display a popup box, dont rerender the display
		if( !fExitDueToMessageBox )
			gfRedrawScreen = TRUE;
	}


	// If we have to get rid of the popup box
	if( gubPopUpBoxAction == AIM_POPUP_DELETE )
	{
		DeleteAimPopUpBox();

		//if we are exiting to display a popup box, dont rerender the display
		if( !fExitDueToMessageBox )
			gfRedrawScreen = TRUE;
	}

	// Handle the current video conference screen
	HandleCurrentVideoConfMode();

	// if the face is active, display the talking face
	if( gfVideoFaceActive )
	{
		gfMercIsTalking = DisplayTalkingMercFaceForVideoPopUp( giMercFaceIndex );

		//put the noise lines on the screen
		if( !gfIsAnsweringMachineActive )
			HandleVideoDistortion();

		//to handle when/if the merc is getting po'ed (waiting for player to do something)
		if( !gfMercIsTalking )
			HandleMercAttitude();
	}

	//if we have to rerender the popup, set the flag to render the PostButtonRender function in laptop.c
	if( gubPopUpBoxAction == AIM_POPUP_DISPLAY )
	{
		fReDrawPostButtonRender = TRUE;
	}

	// Gets set in the InitDeleteVideoConferencePopUp() function
	gfJustSwitchedVideoConferenceMode = FALSE;

	if( gfRedrawScreen )
	{
		RenderAIMMembers();
		gfRedrawScreen = FALSE;
	}
	MarkButtonsDirty( );
}


static BOOLEAN DisplayAimPopUpBox();


void RenderAIMMembersTopLevel()
{
	DisplayAimPopUpBox();
}


static void DisplayAimMemberClickOnFaceHelpText(void);
static void DisplayMercStats(MERCPROFILESTRUCT const&);
static void DisplayMercsFace(void);
static void DisplayMercsInventory(MERCPROFILESTRUCT const&);
static void DisplayVideoConferencingDisplay(MERCPROFILESTRUCT const&);
static void UpdateMercInfo(void);


void RenderAIMMembers()
{
	UINT16		x, uiPosX;

	DrawAimDefaults();

	BltVideoObject(FRAME_BUFFER, guiStats, 0, STATS_X, STATS_Y);
	BltVideoObject(FRAME_BUFFER, guiPrice, 0, PRICE_X, PRICE_Y);

	uiPosX = WEAPONBOX_X;
	for(x=0; x<WEAPONBOX_NUMBER; x++)
	{
		BltVideoObject(FRAME_BUFFER, guiWeaponBox, 0, uiPosX, WEAPONBOX_Y);
		uiPosX += WEAPONBOX_SIZE_X;
	}

	UpdateMercInfo();

	//Draw fee & contract
	DrawTextToScreen(CharacterInfo[AIM_MEMBER_FEE],      FEE_X,          FEE_Y,          0,                  AIM_M_FONT_PREV_NEXT_CONTACT, AIM_M_FEE_CONTRACT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawTextToScreen(CharacterInfo[AIM_MEMBER_CONTRACT], AIM_CONTRACT_X, AIM_CONTRACT_Y, AIM_CONTRACT_WIDTH, AIM_M_FONT_PREV_NEXT_CONTACT, AIM_M_FEE_CONTRACT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	//Draw pay period (day, week, 2 week)
	DrawTextToScreen(CharacterInfo[AIM_MEMBER_1_DAY],   ONEDAY_X,  EXPLEVEL_Y,    AIM_CONTRACT_WIDTH, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	DrawTextToScreen(CharacterInfo[AIM_MEMBER_1_WEEK],  ONEWEEK_X, MARKSMAN_Y,    AIM_CONTRACT_WIDTH, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
	DrawTextToScreen(CharacterInfo[AIM_MEMBER_2_WEEKS], TWOWEEK_X, MECHANAICAL_Y, AIM_CONTRACT_WIDTH, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);

	//Display AIM Member text
	DrawTextToScreen(CharacterInfo[AIM_MEMBER_ACTIVE_MEMBERS], AIM_MEMBER_ACTIVE_TEXT_X, AIM_MEMBER_ACTIVE_TEXT_Y, AIM_MEMBER_ACTIVE_TEXT_WIDTH, AIM_MAINTITLE_FONT, AIM_M_ACTIVE_MEMBER_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display Option Gear Cost text
	DrawTextToScreen(CharacterInfo[AIM_MEMBER_OPTIONAL_GEAR], AIM_MEMBER_OPTIONAL_GEAR_X, AIM_MEMBER_OPTIONAL_GEAR_Y, 0, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	MERCPROFILESTRUCT const& p = GetProfile(gbCurrentSoldier);
	uiPosX = AIM_MEMBER_OPTIONAL_GEAR_X + StringPixLength( CharacterInfo[AIM_MEMBER_OPTIONAL_GEAR], AIM_M_FONT_STATIC_TEXT) + 5;
	DrawTextToScreen(SPrintMoney(p.usOptionalGearCost), uiPosX, AIM_MEMBER_OPTIONAL_GEAR_Y, 0, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	DisableAimButton();

	DisplayMercsInventory(p);


	DisplayMercsFace();

	DisplayMercStats(p);

	if( gubVideoConferencingMode)
	{
		DisplayVideoConferencingDisplay(p);
	}
	else
	{
		gubMercAttitudeLevel=0;
		gfIsAnsweringMachineActive = FALSE;
	}

	//DisplayAimPopUpBox();

	//check to see if the merc is dead if so disable the contact button
	EnableButton(giContactButton, !IsMercDead(p));

	//if we are to renbder the 'click face' text
	if(	gfAimMemberDisplayFaceHelpText )
	{
		DisplayAimMemberClickOnFaceHelpText();
	}




	RenderWWWProgramTitleBar( );
	DisplayProgramBoundingBox( TRUE );
	fReDrawScreenFlag = TRUE;
}


void DrawNumeralsToScreen(INT32 iNumber, INT8 bWidth, UINT16 usLocX, UINT16 usLocY, SGPFont const font, UINT32 ubColor)
{
	ST::string sStr = ST::format("{}", iNumber);
	DrawTextToScreen(sStr, usLocX, usLocY, bWidth, font, ubColor, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
}


static void DrawMoneyToScreen(INT32 iNumber, INT8 bWidth, UINT16 usLocX, UINT16 usLocY, SGPFont const font, UINT32 ubColor)
{
	DrawTextToScreen(SPrintMoney(iNumber), usLocX, usLocY, bWidth, font, ubColor, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
}

static void SelectFaceRegionCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	//if the merc is not dead, video conference with the merc
	if (!IsMercDead(GetProfile(gbCurrentSoldier)))
	{
		gubVideoConferencingMode = AIM_VIDEO_POPUP_MODE;
		gfFirstTimeInContactScreen = TRUE;
	}
}

static void SelectFaceRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_FACIAL_INDEX;
}


static void SelectFaceMovementRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if( iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		gfAimMemberDisplayFaceHelpText = FALSE;
		gfRedrawScreen = TRUE;
	}
	else if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		gfAimMemberDisplayFaceHelpText = TRUE;
		gfRedrawScreen = TRUE;
	}
}


static void LoadMercBioInfo(UINT8 ubIndex, ST::string& pInfoString, ST::string& pAddInfo);


static void UpdateMercInfo(void)
{
	//Display the salaries
	DrawMoneyToScreen(gMercProfiles[gbCurrentSoldier].sSalary, FEE_WIDTH, FEE_X, HEALTH_Y, AIM_M_NUMBER_FONT, AIM_M_COLOR_DYNAMIC_TEXT	);
	DrawMoneyToScreen(gMercProfiles[gbCurrentSoldier].uiWeeklySalary, FEE_WIDTH, FEE_X, AGILITY_Y, AIM_M_NUMBER_FONT, AIM_M_COLOR_DYNAMIC_TEXT	);
	DrawMoneyToScreen(gMercProfiles[gbCurrentSoldier].uiBiWeeklySalary, FEE_WIDTH, FEE_X, DEXTERITY_Y, AIM_M_NUMBER_FONT, AIM_M_COLOR_DYNAMIC_TEXT	);

	//if medical deposit is required
	if( gMercProfiles[gbCurrentSoldier].bMedicalDeposit )
	{
		ST::string zTemp;
		ST::string sMedicalString;

		zTemp = SPrintMoney(gMercProfiles[gbCurrentSoldier].sMedicalDepositAmount);
		sMedicalString = ST::format("{} {}", zTemp, CharacterInfo[AIM_MEMBER_MEDICAL_DEPOSIT_REQ]);

		// If the string will be displayed in more then 2 lines, recenter the string
		if (DisplayWrappedString(0, 0, AIM_MEDICAL_DEPOSIT_WIDTH, 2, AIM_FONT12ARIAL, AIM_M_COLOR_DYNAMIC_TEXT, sMedicalString, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT) / GetFontHeight(AIM_FONT12ARIAL) > 2)
		{
			DisplayWrappedString(AIM_MEDICAL_DEPOSIT_X, AIM_MEDICAL_DEPOSIT_Y - GetFontHeight(AIM_FONT12ARIAL), AIM_MEDICAL_DEPOSIT_WIDTH, 2, AIM_FONT12ARIAL, AIM_M_COLOR_DYNAMIC_TEXT, sMedicalString, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
		}
		else
			DisplayWrappedString(AIM_MEDICAL_DEPOSIT_X, AIM_MEDICAL_DEPOSIT_Y, AIM_MEDICAL_DEPOSIT_WIDTH, 2, AIM_FONT12ARIAL, AIM_M_COLOR_DYNAMIC_TEXT, sMedicalString, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}

	ST::string MercInfoString;
	ST::string AdditionalInfoString;
	LoadMercBioInfo( gbCurrentSoldier, MercInfoString, AdditionalInfoString);
	if( MercInfoString[0] != 0)
	{
		DisplayWrappedString(AIM_MERC_INFO_X, AIM_MERC_INFO_Y, AIM_MERC_INFO_WIDTH, 2, AIM_M_FONT_DYNAMIC_TEXT, AIM_FONT_MCOLOR_WHITE, MercInfoString, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
	if( AdditionalInfoString[0] != 0)
	{
		DrawTextToScreen(CharacterInfo[AIM_MEMBER_ADDTNL_INFO], AIM_MERC_ADD_X, AIM_MERC_ADD_Y, 0, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		DisplayWrappedString(AIM_MERC_ADD_INFO_X, AIM_MERC_ADD_INFO_Y, AIM_MERC_INFO_WIDTH, 2, AIM_M_FONT_DYNAMIC_TEXT, AIM_FONT_MCOLOR_WHITE, AdditionalInfoString, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
}


static void LoadMercBioInfo(UINT8 ubIndex, ST::string& pInfoString, ST::string& pAddInfo)
{
	UINT32 uiStartSeekAmount = (SIZE_MERC_BIO_INFO + SIZE_MERC_ADDITIONAL_INFO) * ubIndex;
	pInfoString = GCM->loadEncryptedString(MERCBIOSFILENAME, uiStartSeekAmount, SIZE_MERC_BIO_INFO);
	pAddInfo = GCM->loadEncryptedString(MERCBIOSFILENAME, uiStartSeekAmount + SIZE_MERC_BIO_INFO, SIZE_MERC_ADDITIONAL_INFO);
}


static void DisplayMercsInventory(MERCPROFILESTRUCT const& p)
{
	// If the merc's inventory has already been purchased, don't display the inventory
	if (p.ubMiscFlags & PROFILE_MISC_FLAG_ALREADY_USED_ITEMS) return;

	INT16       x       = WEAPONBOX_X + 3; // + 3 (1 to take care of the shadow, +2 to get past the weapon box border)
	INT16 const y       = WEAPONBOX_Y;
	UINT8       n_items = 0;
	for (UINT8 i = 0; i < NUM_INV_SLOTS; ++i)
	{
		UINT16 const usItem = p.inv[i];
		if (usItem == NOTHING) continue;

		const ItemModel * item = GCM->getItem(usItem);
		auto graphic = GetSmallInventoryGraphicForItem(item);
		auto item_vo  = graphic.first;
		auto index = graphic.second;
		ETRLEObject const& e        = item_vo->SubregionProperties(index);
		INT16       const  sCenX    = x + std::abs(WEAPONBOX_SIZE_X - 3 - e.usWidth)  / 2 - e.sOffsetX;
		INT16       const  sCenY    = y + std::abs(WEAPONBOX_SIZE_Y     - e.usHeight) / 2 - e.sOffsetY;

		if (gamepolicy(f_draw_item_shadow))
		{
			// Blt the shadow of the item
			BltVideoObjectOutlineShadow(FRAME_BUFFER, item_vo, index, sCenX - 2, sCenY + 2);
		}

		// Blt the item
		BltVideoObjectOutline(      FRAME_BUFFER, item_vo, index, sCenX,     sCenY, SGP_TRANSPARENT);

		// If there are more then 1 piece of equipment in the current slot, display
		// how many there are
		if (p.bInvNumber[i] > 1)
		{
			ST::string buf = ST::format("x{}", p.bInvNumber[i]);
			DrawTextToScreen(buf, x - 1, y + 20, AIM_MEMBER_WEAPON_NAME_WIDTH, AIM_M_FONT_DYNAMIC_TEXT, AIM_M_WEAPON_TEXT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
		}

		// If this will only be a single line, center it in the box
		auto& item_name = GCM->getItem(usItem)->getShortName();
		UINT16  const        tx        = x - 1;
		UINT16  const        ty        = AIM_MEMBER_WEAPON_NAME_Y;
		UINT16  const        tw        = AIM_MEMBER_WEAPON_NAME_WIDTH;
		SGPFont const        tf        = AIM_M_WEAPON_TEXT_FONT;
		UINT32  const        tc        = AIM_M_WEAPON_TEXT_COLOR;
		UINT32  const        tb        = FONT_MCOLOR_BLACK;
		if (DisplayWrappedString(tx, ty, tw, 2, tf, tc, item_name, tb, CENTER_JUSTIFIED | DONT_DISPLAY_TEXT) / GetFontHeight(tf) == 1)
		{
			DisplayWrappedString(tx, ty + GetFontHeight(tf) / 2, tw, 2, tf, tc, item_name, tb, CENTER_JUSTIFIED);
		}
		else
		{
			DisplayWrappedString(tx, ty, tw, 2, tf, tc, item_name, tb, CENTER_JUSTIFIED);
		}

		if (++n_items >= WEAPONBOX_NUMBER) break;
		x += WEAPONBOX_SIZE_X;
	}
}


static void BtnPreviousButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		DeleteAimPopUpBox();

		gbCurrentIndex =
			(gbCurrentIndex > 0 ? gbCurrentIndex - 1 : MAX_NUMBER_MERCS - 1);

		gfRedrawScreen = TRUE;
		gbCurrentSoldier = AimMercArray[gbCurrentIndex];
		gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
	}
}


static void BtnContactButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		// if we are not already in the video conferemce mode, go in to it
		if (gubVideoConferencingMode == AIM_VIDEO_NOT_DISPLAYED_MODE)
		{
			gubVideoConferencingMode = AIM_VIDEO_POPUP_MODE;
			//gubVideoConferencingMode = AIM_VIDEO_INIT_MODE;
			gfFirstTimeInContactScreen = TRUE;
		}

		DeleteAimPopUpBox();
	}
}


static void BtnNextButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		DeleteAimPopUpBox();

		gbCurrentIndex =
			(gbCurrentIndex < MAX_NUMBER_MERCS - 1 ? gbCurrentIndex + 1 : 0);

		gbCurrentSoldier = AimMercArray[gbCurrentIndex];
		gfRedrawScreen = TRUE;
		gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
	}
}


static void DisplayMercsFace(void)
try
{
	// see if the merc is currently hired
	ProfileID          const  id = gbCurrentSoldier;
	SOLDIERTYPE const* const  s  = FindSoldierByProfileIDOnPlayerTeam(id);
	MERCPROFILESTRUCT  const& p  = GetProfile(id);

	// Portrait Frame
	BltVideoObject(FRAME_BUFFER, guiPortrait, 0, PORTRAIT_X, PORTRAIT_Y);

	// load the face graphic
	AutoSGPVObject face(LoadBigPortrait(p));

	BOOLEAN                  shaded;
	ST::string text;
	if (IsMercDead(p))
	{
		// the merc is dead, so shade the face red
		face->pShades[0] = RGBA(255, 55, 55, 0x7f); // TESTME: maxrd2 - Create16BPPPaletteShaded(face->Palette(), DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);
		face->CurrentShade(0);
		shaded = FALSE;
		text   = AimPopUpText[AIM_MEMBER_DEAD];
	}
	else if (p.bMercStatus == MERC_FIRED_AS_A_POW || (s && s->bAssignment == ASSIGNMENT_POW))
	{
		// the merc is currently a POW or, the merc was fired as a pow
		shaded = TRUE;
		text   = pPOWStrings[0];
	}
	else if (s != NULL)
	{
		// the merc has already been hired
		shaded = TRUE;
		text   = MercInfo[MERC_FILES_ALREADY_HIRED];
	}
	else if (!IsMercHireable(p))
	{
		// the merc has a text file and the merc is not away
		shaded = TRUE;
		text   = AimPopUpText[AIM_MEMBER_ON_ASSIGNMENT];
	}
	else
	{
		shaded = FALSE;
		text.clear();
	}

	BltVideoObject(FRAME_BUFFER, face.get(), 0, FACE_X, FACE_Y);

	if (shaded)
	{
		FRAME_BUFFER->ShadowRect(FACE_X, FACE_Y, FACE_X + FACE_WIDTH, FACE_Y + FACE_HEIGHT);
	}

	if (!text.empty())
	{
		DrawTextToScreen(text, FACE_X + 1, FACE_Y + 107, FACE_WIDTH, FONT14ARIAL, 145, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}
}
catch (...) { /* XXX ignore */ }


static void DisplayDots(UINT16 usNameX, UINT16 usNameY, UINT16 usStatX, const ST::string& pString);


static void DrawStatColored(const UINT16 x, const UINT16 y, const ST::string stat, const INT32 val, const UINT32 color)
{
	DrawTextToScreen(stat, x, y, 0, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DisplayDots(x, y, x + STAT_NAME_WIDTH, stat);
	DrawNumeralsToScreen(val, 3, x + STAT_VALUE_DX, y, AIM_M_NUMBER_FONT, color);
}


static void DrawStat(UINT16 x, UINT16 y, const ST::string& stat, INT32 val)
{
	const UINT32 color = (val >= 80 ? HIGH_STAT_COLOR : (val >= 50 ? MED_STAT_COLOR : LOW_STAT_COLOR));
	DrawStatColored(x, y, stat, val, color);
}


static void DisplayMercStats(MERCPROFILESTRUCT const& p)
{
	//Name
	DrawTextToScreen(p.zName, NAME_X, NAME_Y, 0, FONT14ARIAL, AIM_M_COLOR_DYNAMIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//First column in stats box.  Health, Agility, dexterity, strength, leadership, wisdom
	const UINT16 x1 = STATS_FIRST_COL;
	DrawStat(x1, HEALTH_Y,     str_stat_health,     p.bLife      );
	DrawStat(x1, AGILITY_Y,    str_stat_agility,    p.bAgility   );
	DrawStat(x1, DEXTERITY_Y,  str_stat_dexterity,  p.bDexterity );
	DrawStat(x1, STRENGTH_Y,   str_stat_strength,   p.bStrength  );
	DrawStat(x1, LEADERSHIP_Y, str_stat_leadership, p.bLeadership);
	DrawStat(x1, WISDOM_Y,     str_stat_wisdom,     p.bWisdom    );

	//Second column in stats box.  Exp.Level, Markmanship, mechanical, explosive, medical
	const UINT16 x2 = STATS_SECOND_COL;
	DrawStatColored(x2, EXPLEVEL_Y,    str_stat_exp_level,    p.bExpLevel, FONT_MCOLOR_WHITE);
	DrawStat(        x2, MARKSMAN_Y,    str_stat_marksmanship, p.bMarksmanship);
	DrawStat(        x2, MECHANAICAL_Y, str_stat_mechanical,   p.bMechanical  );
	DrawStat(        x2, EXPLOSIVE_Y,   str_stat_explosive,    p.bExplosive   );
	DrawStat(        x2, MEDICAL_Y,     str_stat_medical,      p.bMedical     );
}


//displays the dots between the stats and the stat name
static void DisplayDots(UINT16 usNameX, UINT16 usNameY, UINT16 usStatX, const ST::string& pString)
{
	UINT16 usStringLength = StringPixLength(pString, AIM_M_FONT_STATIC_TEXT);
	INT16  i;
	UINT16 usPosX;

	usPosX = usStatX;
	for(i=usNameX + usStringLength; i <= usPosX; usPosX-=7)
	{
		DrawTextToScreen(".", usPosX, usNameY, 0, AIM_M_FONT_STATIC_TEXT, AIM_M_COLOR_STATIC_TEXT, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
}


static void DisplayMercChargeAmount(void);
static void DisplaySelectLights();


static void BtnContractLengthButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		DisplaySelectLights();
	}
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubContractLength = btn->GetUserData();
		DisplaySelectLights();
		guiMercAttitudeTime = GetJA2Clock();
		DisplayMercChargeAmount();
	}
}


static void BtnBuyEquipmentButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		DisplaySelectLights();
	}
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gfBuyEquipment = btn->GetUserData();
		DisplaySelectLights();
		DisplayMercChargeAmount();
		guiMercAttitudeTime = GetJA2Clock();
	}
}


static INT8    AimMemberHireMerc(void);
static BOOLEAN CanMercBeHired(void);
static void    EnableDisableCurrentVideoConferenceButtons(BOOLEAN fEnable);
static void    CreateAimPopUpBox(const ST::string& sString1, const ST::string& sString2, UINT16 usPosX, UINT16 usPosY, UINT8 ubData);


//Transfer funds button callback
static void BtnAuthorizeButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT8	ubRetValue = btn->GetUserData();

		gfStopMercFromTalking = TRUE;
		gubMercAttitudeLevel = QUOTE_DELAY_NO_ACTION;

		//If we try to hire the merc
		if (ubRetValue == 0)
		{
			StopMercTalking();

			//can the merc be hired?  (does he like/not like people on the team
			//if( CanMercBeHired() )
			{
				//Was the merc hired
				if (AimMemberHireMerc())
				{
					// if merc was hired
					CreateAimPopUpBox(AimPopUpText[AIM_MEMBER_FUNDS_TRANSFER_SUCCESFUL], {}, AIM_POPUP_BOX_X, AIM_POPUP_BOX_Y, AIM_POPUP_BOX_SUCCESS);
					DelayMercSpeech( gbCurrentSoldier, QUOTE_CONTRACT_ACCEPTANCE, 750, TRUE, FALSE );

					//Disable the buttons behind the message box
					EnableDisableCurrentVideoConferenceButtons( TRUE );

					giBuyEquipmentButton[0]->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
					giBuyEquipmentButton[1]->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
				}
			}
#if 0 // XXX was commented out
			else
			{
				//else the merc doesnt like a player on the team, hang up when the merc is done complaining

				//reset ( in case merc was going to say something
				DelayMercSpeech( 0, 0, 0, FALSE, TRUE );

				gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
			}
#endif
		}
		else
		{
			// else we cancel
			gubVideoConferencingMode = AIM_VIDEO_FIRST_CONTACT_MERC_MODE;
		}
	}
}


static INT8 AimMemberHireMerc(void)
{
	if (LaptopSaveInfo.iCurrentBalance < giContractAmount)
	{
		// Wasn't hired because of lack of funds
		CreateAimPopUpBox(AimPopUpText[AIM_MEMBER_FUNDS_TRANSFER_FAILED], AimPopUpText[AIM_MEMBER_NOT_ENOUGH_FUNDS], AIM_POPUP_BOX_X, AIM_POPUP_BOX_Y, AIM_POPUP_BOX_FAILURE);

		// Disable the buttons behind the message box
		EnableDisableCurrentVideoConferenceButtons(TRUE);
		giBuyEquipmentButton[0]->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
		giBuyEquipmentButton[1]->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);

		DelayMercSpeech(gbCurrentSoldier, QUOTE_REFUSAL_TO_JOIN_LACK_OF_FUNDS, 750, TRUE, FALSE);
		return FALSE;
	}

	ProfileID const pid = AimMercArray[gbCurrentIndex];

	MERC_HIRE_STRUCT h;
	h = MERC_HIRE_STRUCT{};
	h.ubProfileID               = pid;
	h.sSector                   = g_merc_arrive_sector;
	h.fUseLandingZoneForArrival = TRUE;
	h.ubInsertionCode           = INSERTION_CODE_ARRIVING_GAME;
	h.fCopyProfileItemsOver     = gfBuyEquipment;
	h.uiTimeTillMercArrives     = GetMercArrivalTimeOfDay();
	h.bWhatKindOfMerc           = MERC_TYPE__AIM_MERC;

	INT8  contract_type;
	INT16 contract_length;
	switch (gubContractLength)
	{
		case AIM_CONTRACT_LENGTH_ONE_DAY:
			contract_type   = CONTRACT_EXTEND_1_DAY;
			contract_length =  1;
			break;

		case AIM_CONTRACT_LENGTH_ONE_WEEK:
			contract_type   = CONTRACT_EXTEND_1_WEEK;
			contract_length =  7;
			break;

		case AIM_CONTRACT_LENGTH_TWO_WEEKS:
			contract_type   = CONTRACT_EXTEND_2_WEEK;
			contract_length = 14;
			break;

		default: abort();
	}
	h.iTotalContractLength = contract_length;

	INT8 const ret = HireMerc(h);
	if (ret == MERC_HIRE_OK)
	{
		// Set the type of contract the merc is on
		SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(pid);
		if (!s) return FALSE;
		s->bTypeOfLastContract = contract_type;

		MERCPROFILESTRUCT& p = GetProfile(pid);
		if (gfBuyEquipment) p.ubMiscFlags |= PROFILE_MISC_FLAG_ALREADY_USED_ITEMS;

		// Add an entry in the finacial page for the hiring of the merc
		AddTransactionToPlayersBook(HIRED_MERC, pid, GetWorldTotalMin(), -giContractAmount + p.sMedicalDepositAmount);

		if (p.bMedicalDeposit)
		{ // Add an entry in the finacial page for the medical deposit
			AddTransactionToPlayersBook(MEDICAL_DEPOSIT, pid, GetWorldTotalMin(), -p.sMedicalDepositAmount);
		}

		// Add an entry in the history page for the hiring of the merc
		AddHistoryToPlayersLog(HISTORY_HIRED_MERC_FROM_AIM, pid, GetWorldTotalMin(), SGPSector(-1, -1));
		return TRUE;
	}
	else if (ret == MERC_HIRE_OVER_20_MERCS_HIRED)
	{
		// Display a warning saying you can't hire more then 20 mercs
		DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, AimPopUpText[AIM_MEMBER_ALREADY_HAVE_20_MERCS], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
	}
	return FALSE;
}


static void DisplayMercsVideoFace(void);


static void DisplayVideoConferencingDisplay(MERCPROFILESTRUCT const& p)
{
	ST::string sMercName;

	if (gubVideoConferencingMode == AIM_VIDEO_NOT_DISPLAYED_MODE) return;
	if (gubVideoConferencingMode == AIM_VIDEO_POPUP_MODE)         return;

	DisplayMercsVideoFace();

	//Title & Name
	if( gubVideoConferencingMode == AIM_VIDEO_INIT_MODE)
	{
		sMercName = VideoConfercingText[AIM_MEMBER_CONNECTING];
		DrawTextToScreen(sMercName, AIM_MEMBER_VIDEO_NAME_X, AIM_MEMBER_VIDEO_NAME_Y, 0, FONT12ARIAL, AIM_M_VIDEO_TITLE_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}
	else
	{
		sMercName = ST::format("{} {}", VideoConfercingText[AIM_MEMBER_VIDEO_CONF_WITH], p.zName);
		DrawTextToScreen(sMercName, AIM_MEMBER_VIDEO_NAME_X, AIM_MEMBER_VIDEO_NAME_Y, 0, FONT12ARIAL, AIM_M_VIDEO_TITLE_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	//Display Contract charge text
	if( gubVideoConferencingMode == AIM_VIDEO_HIRE_MERC_MODE )
	{
		// Display the contract charge
		SetFontShadow(AIM_M_VIDEO_NAME_SHADOWCOLOR);
		DrawTextToScreen(VideoConfercingText[AIM_MEMBER_CONTRACT_CHARGE], AIM_CONTRACT_CHARGE_X, AIM_CONTRACT_CHARGE_Y, 0, FONT12ARIAL, AIM_M_VIDEO_NAME_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		SetFontShadow(DEFAULT_SHADOW);
	}

	DisplayMercChargeAmount();

	//if( gfMercIsTalking && !gfIsAnsweringMachineActive)
	if( gfMercIsTalking && gGameSettings.fOptions[ TOPTION_SUBTITLES ] )
	{
		UINT16 usActualWidth;
		UINT16 usActualHeight;
		UINT16 usPosX;

		AutoMercPopUpBox const aim_members_box(PrepareMercPopupBox(0, BASIC_MERC_POPUP_BACKGROUND, BASIC_MERC_POPUP_BORDER, gsTalkingMercText, 300, 0, 0, 0, &usActualWidth, &usActualHeight));
		usPosX = (STD_SCREEN_X + LAPTOP_SCREEN_LR_X - usActualWidth ) / 2 ;
		RenderMercPopUpBox(aim_members_box, usPosX, TEXT_POPUP_WINDOW_Y, FRAME_BUFFER);
	}

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void DisplayMercsVideoFace(void)
{
	ETRLEObject const& e = guiVideoConfTerminal->SubregionProperties(0);
	INT32       const  x = AIM_MEMBER_VIDEO_CONF_TERMINAL_X;
	INT32       const  y = AIM_MEMBER_VIDEO_CONF_TERMINAL_Y;
	INT32       const  w = e.usWidth;
	INT32       const  h = e.usHeight;

	// Draw a drop shadow
	FRAME_BUFFER->ShadowRect(x + 3, y + h, x + w,     y + h + 3); // Horizontal
	FRAME_BUFFER->ShadowRect(x + w, y + 3, x + w + 3, y + h);     // Vertical

	BltVideoObject(FRAME_BUFFER, guiVideoConfTerminal, 0, x, y);

	//Display the Select light on the merc
	if(gubVideoConferencingMode == AIM_VIDEO_HIRE_MERC_MODE)
		DisplaySelectLights();
}


static void DrawButtonSelection(GUI_BUTTON const* const btn, bool const selected)
{
	INT32 x = btn->X();
	INT32 y = btn->Y();
	if (btn->Clicked())
	{
		x += AIM_SELECT_LIGHT_ON_X;
		y += AIM_SELECT_LIGHT_ON_Y;
	}
	else
	{
		x += AIM_SELECT_LIGHT_OFF_X;
		y += AIM_SELECT_LIGHT_OFF_Y;
	}
	UINT32 const fill_color = selected ? RGB(0, 255, 0) : RGB(0, 0, 0);
	ColorFillVideoSurfaceArea(FRAME_BUFFER, x, y, x + 8, y + 8, fill_color);
}


static void DisplaySelectLights()
{
	{ // First draw the select light for the contract length buttons.
		size_t selected = gubContractLength;
		FOR_EACH(GUIButtonRef, i, giContractLengthButton)
		{
			DrawButtonSelection(*i, selected-- == 0);
		}
	}

	{ // Draw the select light for the buy equipment buttons.
		size_t selected = gfBuyEquipment;
		FOR_EACH(GUIButtonRef, i, giBuyEquipmentButton)
		{
			DrawButtonSelection(*i, selected-- == 0);
		}
	}

	InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y);
}


static void DisplayMercChargeAmount(void)
{
	if (gubVideoConferencingMode != AIM_VIDEO_HIRE_MERC_MODE) return;

	// Display the 'black hole' for the contract charge  in the video conference terminal
	BltVideoObject(FRAME_BUFFER, guiVideoContractCharge, 0, AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_X, AIM_MEMBER_VIDEO_CONF_CONTRACT_IMAGE_Y);

	MERCPROFILESTRUCT const& p = GetProfile(gbCurrentSoldier);

	if (FindSoldierByProfileIDOnPlayerTeam(gbCurrentSoldier) == NULL)
	{
		//the contract charge amount
		INT32 amount;

		// Get the salary rate
		switch (gubContractLength)
		{
			case AIM_CONTRACT_LENGTH_ONE_DAY:   amount = p.sSalary;          break;
			case AIM_CONTRACT_LENGTH_ONE_WEEK:  amount = p.uiWeeklySalary;   break;
			case AIM_CONTRACT_LENGTH_TWO_WEEKS: amount = p.uiBiWeeklySalary; break;
			default:                            amount = 0;                  break;
		}

		// If there is a medical deposit, add it in
		if (p.bMedicalDeposit) amount += p.sMedicalDepositAmount;

		// If hired with the equipment, add it in aswell
		if (gfBuyEquipment) amount += p.usOptionalGearCost;

		giContractAmount = amount;
	}

	ST::string wDollarTemp = SPrintMoney(giContractAmount);

	ST::string wTemp;
	if (p.bMedicalDeposit)
	{
		wTemp = ST::format("{} {}", wDollarTemp, VideoConfercingText[AIM_MEMBER_WITH_MEDICAL]);
	}
	else
	{
		wTemp = wDollarTemp;
	}
	DrawTextToScreen(wTemp, AIM_CONTRACT_CHARGE_AMOUNNT_X + 1, AIM_CONTRACT_CHARGE_AMOUNNT_Y + 3, 0, AIM_M_VIDEO_CONTRACT_AMOUNT_FONT, AIM_M_VIDEO_CONTRACT_AMOUNT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
}


static UINT16  usPopUpBoxPosX;
static UINT16  usPopUpBoxPosY;
static ST::string sPopUpString1;
static ST::string sPopUpString2;
static BOOLEAN fPopUpBoxActive = FALSE;


static void BtnPopUpOkButtonCallback(GUI_BUTTON* btn, UINT32 reason);


static void CreateAimPopUpBox(const ST::string& sString1, const ST::string& sString2, UINT16 usPosX, UINT16 usPosY, UINT8 ubData)
{
	if (fPopUpBoxActive) throw std::logic_error("AIM popup box already active");

	//Disable the 'X' to close the pop upi video
	DisableButton(giXToCloseVideoConfButton);

	sPopUpString1 = sString1;
	sPopUpString2 = sString2;

	usPopUpBoxPosX = usPosX;
	usPopUpBoxPosY = usPosY;

	// load the popup box graphic
	guiPopUpBox = AddVideoObjectFromFile(LAPTOPDIR "/videoconfpopup.sti");

	BltVideoObject(FRAME_BUFFER, guiPopUpBox, 0, usPosX, usPosY);

	//Create the popup boxes button
	guiPopUpImage = LoadButtonImage(LAPTOPDIR "/videoconfbuttons.sti", 2, 3);
	UINT32 const color  = AIM_POPUP_BOX_COLOR;
	UINT32 const shadow = AIM_M_VIDEO_NAME_SHADOWCOLOR;
	INT16 const x       = usPosX + AIM_POPUP_BOX_BUTTON_OFFSET_X;
	INT16 const y       = usPosY + AIM_POPUP_BOX_BUTTON_OFFSET_Y;
	guiPopUpOkButton = CreateIconAndTextButton(guiPopUpImage, VideoConfercingText[AIM_MEMBER_OK], FONT14ARIAL, color, shadow, color, shadow, x, y, MSYS_PRIORITY_HIGH + 5, BtnPopUpOkButtonCallback);
	guiPopUpOkButton->SetCursor(CURSOR_LAPTOP_SCREEN);
	guiPopUpOkButton->SetUserData(ubData);

	fPopUpBoxActive   = TRUE;
	gubPopUpBoxAction = AIM_POPUP_DISPLAY;

	if (gubVideoConferencingPreviousMode == AIM_VIDEO_HIRE_MERC_MODE)
	{
		// Enable the current video conference buttons
		EnableDisableCurrentVideoConferenceButtons(FALSE);
	}

	// Create a new flag for the PostButtonRendering function
	fReDrawPostButtonRender = TRUE;
}


static BOOLEAN DisplayAimPopUpBox()
{
	if (gubPopUpBoxAction != AIM_POPUP_DISPLAY) return FALSE;

	UINT16 y = usPopUpBoxPosY;
	BltVideoObject(FRAME_BUFFER, guiPopUpBox, 0, usPopUpBoxPosX, y);

	SetFontShadow(AIM_M_VIDEO_NAME_SHADOWCOLOR);

	y += AIM_POPUP_BOX_STRING1_Y;
	if (sPopUpString1[0] != '\0') y += DisplayWrappedString(usPopUpBoxPosX, y,     AIM_POPUP_BOX_WIDTH, 2, AIM_POPUP_BOX_FONT, AIM_POPUP_BOX_COLOR, sPopUpString1, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	if (sPopUpString2[0] != '\0')      DisplayWrappedString(usPopUpBoxPosX, y + 4, AIM_POPUP_BOX_WIDTH, 2, AIM_POPUP_BOX_FONT, AIM_POPUP_BOX_COLOR, sPopUpString2, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	SetFontShadow(DEFAULT_SHADOW);
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
	return TRUE;
}


static BOOLEAN DeleteAimPopUpBox()
{
	if (!fPopUpBoxActive) return FALSE;

	//Disable the 'X' to close the pop up video
	EnableButton(giXToCloseVideoConfButton);

	UnloadButtonImage(guiPopUpImage);
	RemoveButton(guiPopUpOkButton);
	delete guiPopUpBox;

	fPopUpBoxActive   = FALSE;
	gubPopUpBoxAction = AIM_POPUP_NOTHING;

	switch (gubVideoConferencingPreviousMode)
	{
		case AIM_VIDEO_HIRE_MERC_MODE:              EnableDisableCurrentVideoConferenceButtons(FALSE); break;
		case AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE: EnableButton(giAnsweringMachineButton[1]);         break;
		default:
			break;
	}

	return TRUE;
}


static void WaitForMercToFinishTalkingOrUserToClick(void);


static void BtnPopUpOkButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	static BOOLEAN fInCallback = TRUE;

	if (fInCallback)
	{
		if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
		{
			UINT8 const ubCurPageNum = btn->GetUserData();

			fInCallback = FALSE;

			//gfStopMercFromTalking = TRUE;

			gubPopUpBoxAction = AIM_POPUP_DELETE;

			if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE)
			{
				if (ubCurPageNum == AIM_POPUP_BOX_SUCCESS)
				{
					gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
					WaitForMercToFinishTalkingOrUserToClick();
				}
					//				gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
				else
					gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
			}

			fInCallback = TRUE;
		}
	}
}


// we first contact merc.  We either go to hire him or cancel the call
static void BtnFirstContactButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT8	const ubRetValue = btn->GetUserData();

		//gfStopMercFromTalking = TRUE;
		StopMercTalking();

		gfAimMemberCanMercSayOpeningQuote = FALSE;

		if (ubRetValue == 0)
		{
			if (CanMercBeHired())
			{
				gubVideoConferencingMode = AIM_VIDEO_HIRE_MERC_MODE;
			}
		}
		else
		{
			gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
		}
	}
}


static void BtnAnsweringMachineButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT8	const ubRetValue = btn->GetUserData();

		if (ubRetValue == 0)
		{
			//Set a flag indicating that the merc has a message
			gMercProfiles[gbCurrentSoldier].ubMiscFlags3 |= PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM;
			WaitForMercToFinishTalkingOrUserToClick();

			//Display a message box displaying a messsage that the message was recorded
			//DoLapTopMessageBox(10, AimPopUpText[AIM_MEMBER_MESSAGE_RECORDED], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
			CreateAimPopUpBox(" ", AimPopUpText[AIM_MEMBER_MESSAGE_RECORDED], AIM_POPUP_BOX_X, AIM_POPUP_BOX_Y, AIM_POPUP_BOX_SUCCESS);


			giAnsweringMachineButton[1]->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
			DisableButton(giAnsweringMachineButton[1]);
			DisableButton(giAnsweringMachineButton[0]);
		}
		else
		{
			gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
			//gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
		}
	}
}


static void BtnHangUpButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		//gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
		gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
	}
}


// InitVideoFace() is called once to initialize things
static void InitVideoFace(UINT8 ubMercID)
{
	//Create the facial index
	FACETYPE& f = InitFace(ubMercID, 0, 0);
	giMercFaceIndex = &f;

	SetAutoFaceActive(guiVideoFaceBackground, FACE_AUTO_RESTORE_BUFFER, f, 0, 0);

	RenderAutoFace(f);

	gubCurrentStaticMode = VC_NO_STATIC;

	gfVideoFaceActive = TRUE;

	guiMercAttitudeTime = GetJA2Clock();
}


// InitVideoFaceTalking() is called to start a merc speaking a particular message
static BOOLEAN InitVideoFaceTalking(UINT8 ubMercID, UINT16 usQuoteNum)
{
	//Starts the merc talking
	CharacterDialogue(ubMercID, usQuoteNum, giMercFaceIndex, DIALOGUE_CONTACTPAGE_UI, FALSE);

	//Enables it so if a player clicks, he will shutup the merc
	gSelectedShutUpMercRegion.Enable();

	gfIsShutUpMouseRegionActive = TRUE;
	gfMercIsTalking = TRUE;
	guiTimeThatMercStartedTalking	= GetJA2Clock();
	return(TRUE);
}


static BOOLEAN DisplayTalkingMercFaceForVideoPopUp(const FACETYPE* const face)
{
	static BOOLEAN fWasTheMercTalking=FALSE;
	BOOLEAN		fIsTheMercTalking;

	//If the answering machine graphics is up, dont handle the faces
	if (gfIsAnsweringMachineActive) giMercFaceIndex->fInvalidAnim = TRUE;

	HandleDialogue();
	HandleAutoFaces( );
	HandleTalkingAutoFaces( );

	//If the answering machine is up, dont display the face
	//if( !gfIsAnsweringMachineActive )
	{
		//Test
		SGPBox const SrcRect = { 0, 0, 48, 43 };

		SGPBox const DestRect =
		{
			(UINT16)(AIM_MEMBER_VIDEO_FACE_X),
			(UINT16)(AIM_MEMBER_VIDEO_FACE_Y),
			AIM_MEMBER_VIDEO_FACE_WIDTH,
			AIM_MEMBER_VIDEO_FACE_HEIGHT
		};

		//Blt the face surface to the video background surface
		BltStretchVideoSurface(FRAME_BUFFER, guiVideoFaceBackground, &SrcRect, &DestRect);

		//if the merc is not at home and the players is leaving a message, shade the players face
		if( gfIsAnsweringMachineActive )
			FRAME_BUFFER->ShadowRect(DestRect.x, DestRect.y, DestRect.x + DestRect.w - 1, DestRect.y + DestRect.h - 1);


		//If the answering machine graphics is up, place a message on the screen
		if( gfIsAnsweringMachineActive )
		{
			//display a message over the mercs face
			DisplayWrappedString(AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y + 20, AIM_MEMBER_VIDEO_FACE_WIDTH, 2, FONT14ARIAL, 145, AimPopUpText[AIM_MEMBER_PRERECORDED_MESSAGE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
		}


		InvalidateRegion(AIM_MEMBER_VIDEO_FACE_X,AIM_MEMBER_VIDEO_FACE_Y, AIM_MEMBER_VIDEO_FACE_X+AIM_MEMBER_VIDEO_FACE_WIDTH,AIM_MEMBER_VIDEO_FACE_Y+AIM_MEMBER_VIDEO_FACE_HEIGHT);
	}

	fIsTheMercTalking = face->fTalking;

	//if the merc is talking, reset their attitude time
	if( fIsTheMercTalking )
	{
		//def added 3/18/99
		guiMercAttitudeTime = GetJA2Clock();
	}

	//if the text the merc is saying is really short, extend the time that it is on the screen
	if( ( GetJA2Clock() - guiTimeThatMercStartedTalking ) > usAimMercSpeechDuration )
	{
		//if the merc just stopped talking
		if(fWasTheMercTalking && !fIsTheMercTalking )
		{
			fWasTheMercTalking = FALSE;

			gfRedrawScreen = TRUE;
			guiMercAttitudeTime = GetJA2Clock();

			StopMercTalking();
		}
	}
	else if( fIsTheMercTalking )
	{
		fWasTheMercTalking = fIsTheMercTalking;
	}

	return(fIsTheMercTalking);
}


void DisplayTextForMercFaceVideoPopUp(const ST::string& str)
{
	gsTalkingMercText = ST::format("\"{}\"", str);

	//Set the minimum time for the dialogue text to be present
	usAimMercSpeechDuration = static_cast<UINT16>(gsTalkingMercText.to_utf32().size() * AIM_TEXT_SPEECH_MODIFIER);

	if( usAimMercSpeechDuration < MINIMUM_TALKING_TIME_FOR_MERC )
		usAimMercSpeechDuration = MINIMUM_TALKING_TIME_FOR_MERC;


	gfRedrawScreen = TRUE;
}


static void SelectShutUpMercRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & (MSYS_CALLBACK_REASON_ANY_BUTTON_UP))
	{
		gfStopMercFromTalking = TRUE;
	}
}


static AIMVideoMode WillMercAcceptCall(void)
{
	// If merc has hung up on the player twice within a period of time
	// (MERC_ANNOYED_WONT_CONTACT_TIME_MINUTES), the merc cant ber hired
	MERCPROFILESTRUCT const& p = GetProfile(gbCurrentSoldier);
	if (p.bMercStatus == MERC_ANNOYED_WONT_CONTACT) return AIM_VIDEO_MERC_UNAVAILABLE_MODE;

	// If the merc is at home, or if the merc is only slightly annoyed at the
	// player, he will greet the player
	if (IsMercHireable(p)) return AIM_VIDEO_FIRST_CONTACT_MERC_MODE;

	return AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE;
}


static BOOLEAN CanMercBeHired(void)
{
	StopMercTalking();

	ProfileID         const  pid = gbCurrentSoldier;
	MERCPROFILESTRUCT const& p   = GetProfile(pid);

	// if the merc recently came back with poor morale, and hasn't gotten over it yet
	if (p.ubDaysOfMoraleHangover > 0)
	{
		// then he refuses with a lame excuse.  Buddy or no buddy.
		WaitForMercToFinishTalkingOrUserToClick();
		InitVideoFaceTalking(pid, QUOTE_LAME_REFUSAL);
		return FALSE;
	}

	INT const buddy = GetFirstBuddyOnTeam(p);

	// loop through the list of people the merc hates
	UINT16 join_quote = QUOTE_NONE;
	for (UINT8 i = 0; i < NUMBER_HATED_MERCS_ONTEAM; ++i)
	{
		//see if someone the merc hates is on the team
		INT8 const bMercID = p.bHated[i];
		if (bMercID < 0) continue;

		if (!IsMercOnTeamAndInOmertaAlreadyAndAlive(bMercID)) continue;

		//if the merc hates someone on the team, see if a buddy is on the team
		//if a buddy is on the team, the merc will join
		switch (buddy)
		{
			UINT16 quote;
			case 0: quote = QUOTE_JOINING_CAUSE_BUDDY_1_ON_TEAM;               goto join_buddy;
			case 1: quote = QUOTE_JOINING_CAUSE_BUDDY_2_ON_TEAM;               goto join_buddy;
			case 2: quote = QUOTE_JOINING_CAUSE_LEARNED_TO_LIKE_BUDDY_ON_TEAM; goto join_buddy;
join_buddy:
				InitVideoFaceTalking(pid, quote);
				return TRUE;
		}

		// the merc doesnt like anybody on the team
		UINT16 quote;
		switch (i)
		{
			case 0:
				if (p.bHatedTime[i] >= 24)
				{
					join_quote = QUOTE_PERSONALITY_BIAS_WITH_MERC_1;
					continue;
				}
				quote = QUOTE_HATE_MERC_1_ON_TEAM;
				break;

			case 1:
				if (p.bHatedTime[i] >= 24)
				{
					join_quote = QUOTE_PERSONALITY_BIAS_WITH_MERC_2;
					continue;
				}
				quote = QUOTE_HATE_MERC_2_ON_TEAM;
				break;

			default:
				quote = QUOTE_LEARNED_TO_HATE_MERC_ON_TEAM;
				break;
		}
		WaitForMercToFinishTalkingOrUserToClick();
		InitVideoFaceTalking(pid, quote);
		return FALSE;
	}

	if (buddy >= 0) return TRUE;

	// Check the players Death rate
	if (MercThinksDeathRateTooHigh(p))
	{
		WaitForMercToFinishTalkingOrUserToClick();
		InitVideoFaceTalking(pid, QUOTE_DEATH_RATE_REFUSAL);
		return FALSE;
	}

	// Check the players Reputation
	if (MercThinksBadReputationTooHigh(p))
	{
		WaitForMercToFinishTalkingOrUserToClick();
		InitVideoFaceTalking(pid, QUOTE_REPUTATION_REFUSAL);
		return FALSE;
	}

	if (join_quote != QUOTE_NONE) InitVideoFaceTalking(pid, join_quote);
	return TRUE;
}


static BOOLEAN DisplaySnowBackground(void)
{
	UINT32		uiCurrentTime = 0;
	UINT8	ubCount;

	uiCurrentTime = GetJA2Clock();

	if(gubCurrentCount < VC_NUM_LINES_SNOW)
	{
		ubCount = gubCurrentCount;
	}
	else if( gubCurrentCount < VC_NUM_LINES_SNOW*2 )
	{
		ubCount = gubCurrentCount - VC_NUM_LINES_SNOW;
	}
	else
	{
		gfFirstTimeInContactScreen = FALSE;
		gubCurrentCount = 0;
		ubCount = 0;

		if( gubVideoConferencingMode == AIM_VIDEO_FIRST_CONTACT_MERC_MODE && gfAimMemberCanMercSayOpeningQuote )
			InitVideoFaceTalking(gbCurrentSoldier, QUOTE_GREETING);

		return(TRUE);
	}

	// if it is time to update the snow image
	if( (uiCurrentTime - guiLastHandleMercTime) > VC_CONTACT_STATIC_TIME)
	{
		gubCurrentCount++;
		guiLastHandleMercTime = uiCurrentTime;
	}
	BltVideoObject(FRAME_BUFFER, guiBWSnow, ubCount,AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y);

	InvalidateRegion(AIM_MEMBER_VIDEO_FACE_X,AIM_MEMBER_VIDEO_FACE_Y, AIM_MEMBER_VIDEO_FACE_X+AIM_MEMBER_VIDEO_FACE_WIDTH,AIM_MEMBER_VIDEO_FACE_Y+AIM_MEMBER_VIDEO_FACE_HEIGHT);

	return(FALSE);
}


static BOOLEAN DisplayBlackBackground(UINT8 ubMaxNumOfLoops)
{
	UINT32		uiCurrentTime = 0;

	uiCurrentTime = GetJA2Clock();

	if (gubCurrentCount >= ubMaxNumOfLoops)
	{
		gubCurrentCount = 0;
		return(TRUE);
	}

	// if it is time to update the snow image
	if( (uiCurrentTime - guiLastHandleMercTime) > VC_CONTACT_STATIC_TIME)
	{
		gubCurrentCount++;
		guiLastHandleMercTime = uiCurrentTime;
	}
	// Blit color to screen
	ColorFillVideoSurfaceArea( FRAME_BUFFER, AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y, AIM_MEMBER_VIDEO_FACE_X+AIM_MEMBER_VIDEO_FACE_WIDTH,	AIM_MEMBER_VIDEO_FACE_Y+AIM_MEMBER_VIDEO_FACE_HEIGHT, RGB(0, 0, 0) );
	InvalidateRegion(AIM_MEMBER_VIDEO_FACE_X,AIM_MEMBER_VIDEO_FACE_Y, AIM_MEMBER_VIDEO_FACE_X+AIM_MEMBER_VIDEO_FACE_WIDTH,AIM_MEMBER_VIDEO_FACE_Y+AIM_MEMBER_VIDEO_FACE_HEIGHT);

	return(FALSE);
}


static UINT8 DisplayDistortionLine(UINT8 ubMode, const SGPVObject* image, UINT8 ubMaxImages);
static UINT8 DisplayPixelatedImage(UINT8 ubMaxImages);
static UINT8 DisplayTransparentSnow(UINT8 ubMode, const SGPVObject* image, UINT8 ubMaxImages, BOOLEAN bForward);


static void HandleVideoDistortion(void)
{
	static UINT32	uiStaticNoiseSound = NO_SAMPLE;
	UINT8		ubOldMode = gubCurrentStaticMode;

	// if we are just entering the contact page, display a snowy background
	if( gfFirstTimeInContactScreen && !gfIsAnsweringMachineActive)
	{
		DisplaySnowBackground();

		//if it is time to start playing another sound
		if( uiStaticNoiseSound == NO_SAMPLE )
		{
			uiStaticNoiseSound = PlayJA2SampleFromFile(LAPTOPDIR "/static4.wav", LOWVOLUME, 1, MIDDLEPAN);
		}
	}
	else
	{
		switch( gubCurrentStaticMode )
		{
			case VC_NO_STATIC:
			{
				static UINT32 uiCurTime=0;
				UINT8	ubNum;

				//if the sound is playing, stop it
				if( uiStaticNoiseSound != NO_SAMPLE )
				{
					SoundStop( uiStaticNoiseSound );
					uiStaticNoiseSound = NO_SAMPLE;
				}

				//DECIDE WHICH ONE TO BLIT NEXT
				if( (GetJA2Clock() - uiCurTime) > 2500)
				{
					ubNum = (UINT8)Random( 200 );//125;

					if( ubNum < 15)
						gubCurrentStaticMode = VC_FUZZY_LINE;

					else if( ubNum < 25)
						gubCurrentStaticMode = VC_STRAIGHTLINE;

					else if( ubNum < 35)
						gubCurrentStaticMode = VC_BW_SNOW;

					else if( ubNum < 40)
						gubCurrentStaticMode = VC_PIXELATE;

					else if( ubNum < 80)
						gubCurrentStaticMode = VC_TRANS_SNOW_OUT;

					else if( ubNum < 100)
						gubCurrentStaticMode = VC_TRANS_SNOW_IN;

					uiCurTime = GetJA2Clock();
				}
			}
				break;

			case VC_FUZZY_LINE:
				gubCurrentStaticMode = DisplayDistortionLine(VC_FUZZY_LINE, guiFuzzLine, VC_NUM_FUZZ_LINES);

				//if it is time to start playing another sound
				if( uiStaticNoiseSound == NO_SAMPLE )
				{
					uiStaticNoiseSound = PlayJA2SampleFromFile(LAPTOPDIR "/static1.wav", LOWVOLUME, 1, MIDDLEPAN);
				}
				break;

			case VC_STRAIGHTLINE:
				gubCurrentStaticMode = DisplayDistortionLine(VC_STRAIGHTLINE, guiStraightLine, VC_NUM_STRAIGHT_LINES);

				//if it is time to start playing another sound
				if( uiStaticNoiseSound == NO_SAMPLE )
				{
					uiStaticNoiseSound = PlayJA2SampleFromFile(LAPTOPDIR "/static5.wav", LOWVOLUME, 1, MIDDLEPAN);
				}
				break;

			case VC_BW_SNOW:
				gubCurrentStaticMode = DisplayDistortionLine(VC_BW_SNOW, guiBWSnow, 5);

				//if it is time to start playing another sound
				if( uiStaticNoiseSound == NO_SAMPLE )
				{
					uiStaticNoiseSound = PlayJA2SampleFromFile(LAPTOPDIR "/static6.wav", LOWVOLUME, 1, MIDDLEPAN);
				}
				break;

			case VC_PIXELATE:
				gubCurrentStaticMode = DisplayPixelatedImage(4);

				//if it is time to start playing another sound
				if( uiStaticNoiseSound == NO_SAMPLE )
				{
					uiStaticNoiseSound = PlayJA2SampleFromFile(LAPTOPDIR "/static3.wav", LOWVOLUME, 1, MIDDLEPAN);
				}
				break;

			case VC_TRANS_SNOW_OUT:
				gubCurrentStaticMode = DisplayTransparentSnow(VC_TRANS_SNOW_OUT, guiTransSnow, 7, FALSE);

				//if it is time to start playing another sound
				if( uiStaticNoiseSound == NO_SAMPLE )
				{
					uiStaticNoiseSound = PlayJA2SampleFromFile(LAPTOPDIR "/static5.wav", LOWVOLUME, 1, MIDDLEPAN);
				}
				break;

			case VC_TRANS_SNOW_IN:
				gubCurrentStaticMode = DisplayTransparentSnow(VC_TRANS_SNOW_IN, guiTransSnow, 7, TRUE);

				//if it is time to start playing another sound
				if( uiStaticNoiseSound == NO_SAMPLE )
				{
					uiStaticNoiseSound = PlayJA2SampleFromFile(LAPTOPDIR "/static4.wav", LOWVOLUME, 1, MIDDLEPAN);
				}
				break;
		}

		if( ubOldMode != gubCurrentStaticMode )
		{
			uiStaticNoiseSound = NO_SAMPLE;
		}
	}
}


//returns true when done. else false
static UINT8 DisplayTransparentSnow(const UINT8 ubMode, const SGPVObject* const image, const UINT8 ubMaxImages, const BOOLEAN bForward)
{
	static INT8   bCount= 0;
	UINT32        uiCurrentTime = 0;
	static UINT32 uiLastTime=0;

	uiCurrentTime = GetJA2Clock();

	if( (uiCurrentTime - uiLastTime) > 100)
	{
		if( bForward )
		{
			if( bCount > ubMaxImages-1 )
				bCount = 0;
			else
				bCount++;
		}
		else
		{
			if( bCount <= 0 )
				bCount = ubMaxImages-1;
			else
				bCount--;
		}
		uiLastTime = uiCurrentTime;
	}

	if( bCount >= ubMaxImages)
		bCount = ubMaxImages - 1;

	BltVideoObject(FRAME_BUFFER, image, bCount, AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y);

	if( bForward )
	{
		if( bCount == ubMaxImages-1)
		{
			bCount = 0;
			return(VC_BW_SNOW);
		}
		else
			return(ubMode);
	}
	else
	{
		if( bCount == 0)
		{
			bCount = 0;
			return(VC_NO_STATIC);
		}
		else
			return(ubMode);
	}
}


//returns true when done. else false
static UINT8 DisplayDistortionLine(const UINT8 ubMode, const SGPVObject* const image, const UINT8 ubMaxImages)
{
	static UINT8	ubCount=255;
	UINT32		uiCurrentTime = 0;
	static UINT32	uiLastTime=0;

	uiCurrentTime = GetJA2Clock();

	if( (uiCurrentTime - uiLastTime) > VC_CONTACT_FUZZY_LINE_TIME)
	{
		if( ubCount >= ubMaxImages-1 )
			ubCount = 0;
		else
			ubCount++;
		uiLastTime = uiCurrentTime;
	}

	if( ubCount >= ubMaxImages)
		ubCount = ubMaxImages - 1;

	BltVideoObject(FRAME_BUFFER, image, ubCount,AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y);

	if( ubCount == ubMaxImages-1)
	{
		ubCount = 0;
		if( ubMode == VC_BW_SNOW)
			return(VC_TRANS_SNOW_OUT);
		else
			return(VC_NO_STATIC);
	}
	else
		return(ubMode);
}


static UINT8 DisplayPixelatedImage(UINT8 ubMaxImages)
{
	static UINT8	ubCount=255;
	UINT32		uiCurrentTime = 0;
	static UINT32	uiLastTime=0;

	uiCurrentTime = GetJA2Clock();

	if( (uiCurrentTime - uiLastTime) > VC_CONTACT_FUZZY_LINE_TIME)
	{
		if( ubCount >= ubMaxImages-1 )
			ubCount = 0;
		else
			ubCount++;
		uiLastTime = uiCurrentTime;
	}

	FRAME_BUFFER->ShadowRect(AIM_MEMBER_VIDEO_FACE_X, AIM_MEMBER_VIDEO_FACE_Y, AIM_MEMBER_VIDEO_FACE_X + AIM_MEMBER_VIDEO_FACE_WIDTH - 1, AIM_MEMBER_VIDEO_FACE_Y + AIM_MEMBER_VIDEO_FACE_HEIGHT - 1);

	if( ubCount == ubMaxImages-1)
	{
		ubCount = 0;
		return(VC_NO_STATIC);
	}
	else
		return(VC_PIXELATE);
}


static void HandleMercAttitude(void)
{
	UINT32 uiCurrentTime = 0;

	uiCurrentTime = GetJA2Clock();

	if( ( gubMercAttitudeLevel <= 1 && ( ( uiCurrentTime -  guiMercAttitudeTime ) > QUOTE_FIRST_ATTITUDE_TIME ) ) ||
			( ( uiCurrentTime -  guiMercAttitudeTime ) > QUOTE_ATTITUDE_TIME ) )
	{

		if( gubMercAttitudeLevel == QUOTE_DELAY_SMALL_TALK)
		{
			InitVideoFaceTalking(gbCurrentSoldier, QUOTE_SMALL_TALK);
		}
		else if( gubMercAttitudeLevel == QUOTE_DELAY_IMPATIENT_TALK)
		{
			InitVideoFaceTalking(gbCurrentSoldier, QUOTE_IMPATIENT_QUOTE);
		}
		else if( gubMercAttitudeLevel == QUOTE_DELAY_VERY_IMPATIENT_TALK)
		{
			InitVideoFaceTalking(gbCurrentSoldier, QUOTE_PRECEDENT_TO_REPEATING_ONESELF);
			InitVideoFaceTalking(gbCurrentSoldier, QUOTE_IMPATIENT_QUOTE);
		}
		else if( gubMercAttitudeLevel == QUOTE_DELAY_HANGUP_TALK)
		{
			UINT32	uiResetTime;
			InitVideoFaceTalking(gbCurrentSoldier, QUOTE_COMMENT_BEFORE_HANG_UP);

			//if the merc is going to hang up disable the buttons, so user cant press any buttons
			//EnableDisableCurrentVideoConferenceButtons( FALSE);
			if( gubVideoConferencingPreviousMode == AIM_VIDEO_HIRE_MERC_MODE )
			{
				// Enable the current video conference buttons
				EnableDisableCurrentVideoConferenceButtons(FALSE);
			}


			//increments the merc 'annoyance' at the player
			if( gMercProfiles[ gbCurrentSoldier ].bMercStatus == 0 )
				gMercProfiles[ gbCurrentSoldier ].bMercStatus = MERC_ANNOYED_BUT_CAN_STILL_CONTACT;
			else if( gMercProfiles[ gbCurrentSoldier ].bMercStatus == MERC_ANNOYED_BUT_CAN_STILL_CONTACT )
				gMercProfiles[ gbCurrentSoldier ].bMercStatus = MERC_ANNOYED_WONT_CONTACT;

			// add an event so we can reset the 'annoyance factor'
			uiResetTime =  ( Random( 600 ) );
			uiResetTime += GetWorldTotalMin() + MERC_ANNOYED_WONT_CONTACT_TIME_MINUTES;
			AddStrategicEvent( EVENT_AIM_RESET_MERC_ANNOYANCE, uiResetTime, gbCurrentSoldier );

			gfHangUpMerc = TRUE;
		}

		if( gubMercAttitudeLevel == QUOTE_MERC_BUSY )
		{
			InitVideoFaceTalking(gbCurrentSoldier, QUOTE_LAME_REFUSAL);
			gfHangUpMerc = TRUE;
		}
		else if( gubMercAttitudeLevel != QUOTE_DELAY_NO_ACTION )
			gubMercAttitudeLevel++;

		guiMercAttitudeTime = GetJA2Clock();
	}
}


static void StopMercTalking(void)
{
	if( gfIsShutUpMouseRegionActive )
	{
		gSelectedShutUpMercRegion.Disable();

		ShutupaYoFace( giMercFaceIndex );
		gfMercIsTalking = FALSE;
		guiMercAttitudeTime = GetJA2Clock();
		gfIsShutUpMouseRegionActive = FALSE;
		gfRedrawScreen = TRUE;
	}
}


static void BtnXToCloseVideoConfButtonCallback(GUI_BUTTON *btn, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gubVideoConferencingMode = AIM_VIDEO_POPDOWN_MODE;
		//gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;
	}
}


static GUIButtonRef MakeButtonVideo(BUTTON_PICS* img, const ST::string& text, INT16 x, INT16 y, GUI_CALLBACK click)
{
	const INT16 txt_col    = AIM_M_VIDEO_NAME_COLOR;
	const INT16 shadow_col = AIM_M_VIDEO_NAME_SHADOWCOLOR;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, FONT12ARIAL, txt_col, shadow_col, txt_col, shadow_col, x, y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	return btn;
}


static void DeleteVideoConfPopUp(void);


static void InitDeleteVideoConferencePopUp(void)
{
	//remove the face help text
	gfAimMemberDisplayFaceHelpText = FALSE;

	//Gets reset to FALSE in the HandleCurrentVideoConfMode() function
	gfJustSwitchedVideoConferenceMode = TRUE;

	//remove old mode
	DeleteVideoConfPopUp();

	// reset (in case merc was going to say something)
	DelayMercSpeech(0, 0, 0, FALSE, TRUE);

	// if the video conferencing is currently displayed, put the 'x' to close it
	// in the top right corner and disable the ability to click on the BIG face to
	// go to different screen
	if (gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE &&
			gubVideoConferencingMode != AIM_VIDEO_POPUP_MODE         &&
			!giXToCloseVideoConfButton)
	{
		giXToCloseVideoConfButton = QuickCreateButton(giXToCloseVideoConfButtonImage, AIM_MEMBER_VIDEO_CONF_XCLOSE_X, AIM_MEMBER_VIDEO_CONF_XCLOSE_Y, MSYS_PRIORITY_HIGH, BtnXToCloseVideoConfButtonCallback);
		giXToCloseVideoConfButton->SetCursor(CURSOR_LAPTOP_SCREEN);
		giXToCloseVideoConfButton->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
		gSelectedFaceRegion.Disable();
	}

	switch (gubVideoConferencingMode)
	{
		case AIM_VIDEO_NOT_DISPLAYED_MODE:
		{
			// The video conference is not displayed
			gubVideoConferencingPreviousMode = gubVideoConferencingMode;
			gfRedrawScreen = TRUE;

			if (gfVideoFaceActive)
			{
				StopMercTalking();
				//Get rid of the talking face
				DeleteFace(giMercFaceIndex);
			}

			//if the ansering machine is currently on, turn it off
			gfIsAnsweringMachineActive = FALSE;

			gfVideoFaceActive = FALSE;

			if (giXToCloseVideoConfButton)
			{
				RemoveButton(giXToCloseVideoConfButton);
			}

			gSelectedShutUpMercRegion.Disable();

			//Enable the ability to click on the BIG face to go to different screen
			gSelectedFaceRegion.Enable();

			if (gubVideoConferencingPreviousMode == AIM_VIDEO_HIRE_MERC_MODE)
			{
				// Enable the current video conference buttons
				EnableDisableCurrentVideoConferenceButtons(FALSE);
			}

			fNewMailFlag       = gfIsNewMailFlagSet;
			gfIsNewMailFlagSet = FALSE;
			break;
		}

		case AIM_VIDEO_POPUP_MODE:
		{
			gubVideoConferencingPreviousMode = gubVideoConferencingMode;

			if (gfJustSwitchedVideoConferenceMode)
			{
				// load the answering machine graphic and add it

				// Create a background video surface to blt the face onto
				guiVideoTitleBar = AddVideoSurface(AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH, AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT, PIXEL_DEPTH);
				BltVideoObjectOnce(guiVideoTitleBar, LAPTOPDIR "/videotitlebar.sti", 0, 0, 0);

				gfAimMemberCanMercSayOpeningQuote = TRUE;
			}
			break;
		}

		case AIM_VIDEO_INIT_MODE:
		{
			// The opening animation of the vc (fuzzy screen, then goes to black)
			gubVideoConferencingPreviousMode = gubVideoConferencingMode;
			gubMercAttitudeLevel  = 0;
			gubContractLength     = AIM_CONTRACT_LENGTH_ONE_WEEK;
			gfBuyEquipment        = GetProfile(gbCurrentSoldier).usOptionalGearCost != 0;
			gfMercIsTalking       = FALSE;
			gfVideoFaceActive     = FALSE;
			guiLastHandleMercTime = 0;
			gfHangUpMerc          = FALSE;
			break;
		}

		case AIM_VIDEO_FIRST_CONTACT_MERC_MODE:
		{
			// The screen in which you first contact the merc, you have the option to
			// hang up or goto hire merc screen
			// if the last screen was the init screen, then we need to initialize the
			// video face
			if (gubVideoConferencingPreviousMode == AIM_VIDEO_INIT_MODE ||
					gubVideoConferencingPreviousMode == AIM_VIDEO_NOT_DISPLAYED_MODE)
			{
				//Put the merc face up on the screen
				InitVideoFace(gbCurrentSoldier);
			}

			gubVideoConferencingPreviousMode = gubVideoConferencingMode;

			// Hang up button
			UINT16 usPosX = AIM_MEMBER_AUTHORIZE_PAY_X;
			BUTTON_PICS* const img = LoadButtonImage(LAPTOPDIR "/videoconfbuttons.sti", 2, 3);
			guiVideoConferenceButtonImage[2] = img;
			for (UINT8 i = 0; i < 2; ++i)
			{
				giAuthorizeButton[i] = MakeButtonVideo(img, VideoConfercingText[AIM_MEMBER_HIRE + i], usPosX, AIM_MEMBER_HANG_UP_Y, BtnFirstContactButtonCallback);
				giAuthorizeButton[i]->SetUserData(i);
				usPosX += AIM_MEMBER_AUTHORIZE_PAY_GAP;
			}

			if (gfWaitingForMercToStopTalkingOrUserToClick)
			{
				DisableButton(giAuthorizeButton[0]);
				gfWaitingForMercToStopTalkingOrUserToClick = FALSE;
			}
			break;
		}

		case AIM_VIDEO_HIRE_MERC_MODE:
		{
			// The screen in which you set the contract length, and the ability to buy
			// equipment.
			gubVideoConferencingPreviousMode = gubVideoConferencingMode;

			// Contract Length button
			BUTTON_PICS* const img0 = LoadButtonImage(LAPTOPDIR "/videoconfbuttons.sti", 0, 1);
			guiVideoConferenceButtonImage[0] = img0;
			{ UINT16 usPosY = AIM_MEMBER_BUY_CONTRACT_LENGTH_Y;
				for (UINT8 i = 0; i < 3; ++i)
				{
					GUIButtonRef const btn = MakeButtonVideo(img0, VideoConfercingText[AIM_MEMBER_ONE_DAY + i], AIM_MEMBER_BUY_CONTRACT_LENGTH_X, usPosY, BtnContractLengthButtonCallback);
					giContractLengthButton[i] = btn;
					btn->SpecifyTextJustification(GUI_BUTTON::TEXT_LEFT);
					btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
					btn->SetUserData(i);
					usPosY += AIM_MEMBER_BUY_EQUIPMENT_GAP;
				}
			}

			// BuyEquipment button
			{ UINT16 usPosY = AIM_MEMBER_BUY_CONTRACT_LENGTH_Y;
				for (UINT8 i = 0; i < 2; ++i)
				{
					GUIButtonRef const btn = MakeButtonVideo(img0, VideoConfercingText[AIM_MEMBER_NO_EQUIPMENT + i], AIM_MEMBER_BUY_EQUIPMENT_X, usPosY, BtnBuyEquipmentButtonCallback);
					giBuyEquipmentButton[i] = btn;
					btn->SpecifyTextJustification(GUI_BUTTON::TEXT_LEFT);
					btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);
					btn->SetUserData(i);
					usPosY += AIM_MEMBER_BUY_EQUIPMENT_GAP;
				}
			}
			if (GetProfile(gbCurrentSoldier).usOptionalGearCost == 0)
			{
				DisableButton(giBuyEquipmentButton[1]);
			}

			// Authorize button
			UINT16 usPosX = AIM_MEMBER_AUTHORIZE_PAY_X;
			BUTTON_PICS* const img1 = LoadButtonImage(LAPTOPDIR "/videoconfbuttons.sti", 2, 3);
			guiVideoConferenceButtonImage[1] = img1;
			for (UINT8 i = 0; i < 2; ++i)
			{
				GUIButtonRef const btn = MakeButtonVideo(img1, VideoConfercingText[AIM_MEMBER_TRANSFER_FUNDS + i], usPosX, AIM_MEMBER_AUTHORIZE_PAY_Y, BtnAuthorizeButtonCallback);
				giAuthorizeButton[i] = btn;
				btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_NONE);
				btn->SetUserData(i);
				usPosX += AIM_MEMBER_AUTHORIZE_PAY_GAP;
			}

			DelayMercSpeech(gbCurrentSoldier, QUOTE_LENGTH_OF_CONTRACT, 750, TRUE, FALSE);
			break;
		}

		case AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE:
		{
			// The merc is not home and the player gets the answering machine
			gubVideoConferencingPreviousMode = gubVideoConferencingMode;

			gfIsAnsweringMachineActive = TRUE;

			// Leave msg button
			UINT16 usPosX = AIM_MEMBER_AUTHORIZE_PAY_X;
			BUTTON_PICS* const img = LoadButtonImage(LAPTOPDIR "/videoconfbuttons.sti", 2, 3);
			guiVideoConferenceButtonImage[2] = img;

			giAnsweringMachineButton[0] = MakeButtonVideo(img, VideoConfercingText[AIM_MEMBER_LEAVE_MESSAGE], usPosX, AIM_MEMBER_HANG_UP_Y, BtnAnsweringMachineButtonCallback);
			giAnsweringMachineButton[0]->SetUserData(0);

			//if the user has already left a message, disable the button
			if (GetProfile(gbCurrentSoldier).ubMiscFlags3 & PROFILE_MISC_FLAG3_PLAYER_LEFT_MSG_FOR_MERC_AT_AIM)
			{
				DisableButton(giAnsweringMachineButton[0]);
			}

			usPosX += AIM_MEMBER_AUTHORIZE_PAY_GAP;

			giAnsweringMachineButton[1] = MakeButtonVideo(img, VideoConfercingText[AIM_MEMBER_HANG_UP], usPosX, AIM_MEMBER_HANG_UP_Y, BtnAnsweringMachineButtonCallback);
			giAnsweringMachineButton[1]->SetUserData(1);

			//The face must be inited even though the face wont appear.  It is so the voice is played
			InitVideoFace(gbCurrentSoldier);

			//Make sure the merc doesnt ramble away to the player
			gubMercAttitudeLevel = QUOTE_DELAY_NO_ACTION;

			gubCurrentStaticMode = VC_NO_STATIC;
			break;
		}

		case AIM_VIDEO_MERC_UNAVAILABLE_MODE:
		{
			// The merc is home but for some reason doesnt want to work for player
			gubVideoConferencingPreviousMode = gubVideoConferencingMode;

			// The hangup button
			BUTTON_PICS* const img = LoadButtonImage(LAPTOPDIR "/videoconfbuttons.sti", 2, 3);
			guiVideoConferenceButtonImage[2] = img;
			giHangUpButton = MakeButtonVideo(img, VideoConfercingText[AIM_MEMBER_HANG_UP], AIM_MEMBER_HANG_UP_X, AIM_MEMBER_HANG_UP_Y, BtnHangUpButtonCallback);
			giHangUpButton->SetUserData(1);

			//set the flag saying specifying that merc is busy
			gubMercAttitudeLevel = QUOTE_MERC_BUSY;

			InitVideoFace(gbCurrentSoldier);
			break;
		}

		case AIM_VIDEO_POPDOWN_MODE:
		{
			if (gubPopUpBoxAction == AIM_POPUP_DISPLAY) return;

			gubVideoConferencingPreviousMode = gubVideoConferencingMode;

			gfIsAnsweringMachineActive = FALSE;

			// load the Video conference background graphic and add it

			// Create a background video surface to blt the face onto
			guiVideoTitleBar = AddVideoSurface(AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH, AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT, PIXEL_DEPTH);
			BltVideoObjectOnce(guiVideoTitleBar, LAPTOPDIR "/videotitlebar.sti", 0, 0, 0);
			break;
		}
	}

	//reset the time in which the merc will get annoyed
	guiMercAttitudeTime = GetJA2Clock();
}


static void DeleteVideoConfPopUp(void)
{
	// reset (in case merc was going to say something)
	DelayMercSpeech(0, 0, 0, FALSE, TRUE);

	switch (gubVideoConferencingPreviousMode)
	{
		// The video conference is not displayed
		case AIM_VIDEO_NOT_DISPLAYED_MODE:
			break;

		case AIM_VIDEO_POPUP_MODE:
			DeleteVideoSurface(guiVideoTitleBar);
			break;

		// The opening animation of the vc (fuzzy screen, then goes to black)
		case AIM_VIDEO_INIT_MODE:
			break;

		// The screen in which you first contact the merc, you have the option to
		// hang up or goto hire merc screen
		case AIM_VIDEO_FIRST_CONTACT_MERC_MODE:
			UnloadButtonImage(guiVideoConferenceButtonImage[2]);
			for (UINT16 i = 0; i < 2; ++i) RemoveButton(giAuthorizeButton[i]);
			break;

		// The screen in which you set the contract length, and the ability to buy
		// equipment.
		case AIM_VIDEO_HIRE_MERC_MODE:
			for (UINT16 i = 0; i < 2; ++i) UnloadButtonImage(guiVideoConferenceButtonImage[i]);
			for (UINT16 i = 0; i < 3; ++i) RemoveButton(giContractLengthButton[i]);
			for (UINT16 i = 0; i < 2; ++i) RemoveButton(giBuyEquipmentButton[i]);
			for (UINT16 i = 0; i < 2; ++i) RemoveButton(giAuthorizeButton[i]);
			break;

		// The merc is not home and the player gets the answering machine
		case AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE:
			UnloadButtonImage(guiVideoConferenceButtonImage[2]);
			for (UINT16 i = 0; i < 2; ++i) RemoveButton(giAnsweringMachineButton[i]);
			break;

		// The merc is home but doesnt want to work for player
		case AIM_VIDEO_MERC_UNAVAILABLE_MODE:
			RemoveButton(giHangUpButton);
			UnloadButtonImage(guiVideoConferenceButtonImage[2]);
			break;

		case AIM_VIDEO_POPDOWN_MODE:
			if (gubPopUpBoxAction == AIM_POPUP_DISPLAY) return;
			gfWaitingForMercToStopTalkingOrUserToClick = FALSE;
			DeleteVideoSurface(guiVideoTitleBar);
			break;
	}
}


static BOOLEAN DisplayMovingTitleBar(BOOLEAN fForward);


static void HandleCurrentVideoConfMode(void)
{
	switch (gubVideoConferencingMode)
	{
		// The video conference is not displayed
		case AIM_VIDEO_NOT_DISPLAYED_MODE:
			gfWaitingForMercToStopTalkingOrUserToClick = FALSE;
			break;

		case AIM_VIDEO_POPUP_MODE:
			if (DisplayMovingTitleBar(TRUE))
			{
				gubVideoConferencingMode = AIM_VIDEO_INIT_MODE;
			}
			break;

		// The opening animation of the vc (fuzzy screen, then goes to black)
		case AIM_VIDEO_INIT_MODE:
		{
			static UINT8 ubCurMode = 0;

			BOOLEAN	done;
			if (ubCurMode == 0)
			{
				done = DisplayBlackBackground(10);
				if (done) ubCurMode = 1;
			}
			else
			{
				done = DisplaySnowBackground();
			}

			if (done && ubCurMode != 0)
			{
				ubCurMode = 0;
				gubVideoConferencingMode = WillMercAcceptCall();
			}
			break;
		}

		// The screen in which you first contact the merc, you have the option to
		// hang up or goto hire merc screen
		case AIM_VIDEO_FIRST_CONTACT_MERC_MODE:
			break;

		// The screen in which you set the contract length, and the ability to buy
		// equipment.
		case AIM_VIDEO_HIRE_MERC_MODE:
			break;

		// The merc is not home and the player gets the answering machine
		case AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE:
			if (gfJustSwitchedVideoConferenceMode)
			{
				InitVideoFaceTalking(gbCurrentSoldier, QUOTE_ANSWERING_MACHINE_MSG);
			}
			break;

		// The merc is home but doesnt want to work for player
		case AIM_VIDEO_MERC_UNAVAILABLE_MODE:
			break;

		case AIM_VIDEO_POPDOWN_MODE:
			if (DisplayMovingTitleBar(FALSE))
			{
				gubVideoConferencingMode = AIM_VIDEO_NOT_DISPLAYED_MODE;

				//display the popup telling the user when the just hired merc is going to land
				DisplayPopUpBoxExplainingMercArrivalLocationAndTime();

				//render the screen immediately to get rid of the pop down stuff
				InitDeleteVideoConferencePopUp();
				RenderAIMMembers();
				gfVideoFaceActive = FALSE;
			}
			break;
	}
}


static void EnableDisableCurrentVideoConferenceButtons(BOOLEAN const fEnable)
{
	static BOOLEAN fCreated = FALSE;

	if (fCreated == fEnable) return;
	fCreated = fEnable;

	//HB: typo missing "!"
	// Enable/disable buttons behind the acknowlegde button
	for (INT8 i = 0; i != 3; ++i) EnableButton(giContractLengthButton[i], !fEnable);
	for (INT8 i = 0; i != 2; ++i) EnableButton(giBuyEquipmentButton[i],   !fEnable);
	for (INT8 i = 0; i != 2; ++i) EnableButton(giAuthorizeButton[i],      !fEnable);
}


void ResetMercAnnoyanceAtPlayer(ProfileID ubMercID)
{
	if (ubMercID == LARRY_NORMAL)
	{
		if (CheckFact(FACT_LARRY_CHANGED, 0)) ubMercID = LARRY_DRUNK;
	}
	else if (ubMercID == LARRY_DRUNK)
	{
		if (!CheckFact(FACT_LARRY_CHANGED, 0)) ubMercID = LARRY_NORMAL;
	}
	INT8& status = GetProfile(ubMercID).bMercStatus;
	if (status == MERC_ANNOYED_WONT_CONTACT ||
			status == MERC_ANNOYED_BUT_CAN_STILL_CONTACT)
	{
		status = 0;
	}
}


void DisableNewMailMessage(void)
{
	if (fNewMailFlag && gubVideoConferencingMode != AIM_VIDEO_NOT_DISPLAYED_MODE)
	{
		gfIsNewMailFlagSet = TRUE;
		fNewMailFlag = FALSE;
		gfRedrawScreen = TRUE;
	}
}


static INT32 Interpolate(INT32 start, INT32 end, INT32 step)
{
	return start + (end - start) * step / AIM_MEMBER_VIDEO_TITLE_ITERATIONS;
}


static BOOLEAN DisplayMovingTitleBar(BOOLEAN fForward)
{
	static UINT8  ubCount;
	static SGPBox LastRect;

	if (gfJustSwitchedVideoConferenceMode)
	{
		ubCount = (fForward ? 1 : AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 1);
	}

	UINT16 const usPosX      = STD_SCREEN_X + Interpolate(331, 125, ubCount);
	UINT16 const usPosRightX = STD_SCREEN_X + Interpolate(405, 490, ubCount);
	UINT16 const usPosY      = Interpolate(AIM_MEMBER_VIDEO_TITLE_START_Y, STD_SCREEN_Y + 96, ubCount);
	SGPBox const SrcRect     = { 0,      0,      AIM_MEMBER_VIDEO_TITLE_BAR_WIDTH, AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT };
	SGPBox const DestRect    = { usPosX, usPosY, (UINT16)(usPosRightX - usPosX),             AIM_MEMBER_VIDEO_TITLE_BAR_HEIGHT };

	if (fForward)
	{
		//Restore the old rect
		if (ubCount > 2)
		{
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, LastRect.x, LastRect.y, LastRect.w, LastRect.h);
		}

		//Save rectangle
		if (ubCount > 1)
		{
			BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.x, DestRect.y, DestRect.w, DestRect.h);
		}
	}
	else
	{
		//Restore the old rect
		if (ubCount < AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 2)
		{
			BlitBufferToBuffer(guiSAVEBUFFER, FRAME_BUFFER, LastRect.x, LastRect.y, LastRect.w, LastRect.h);
		}

		//Save rectangle
		if (ubCount < AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 1)
		{
			BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, DestRect.x, DestRect.y, DestRect.w, DestRect.h);
		}
	}

	BltStretchVideoSurface(FRAME_BUFFER, guiVideoTitleBar, &SrcRect, &DestRect);

	InvalidateRegion(DestRect.x, DestRect.y, DestRect.x + DestRect.w, DestRect.y + DestRect.h);
	InvalidateRegion(LastRect.x, LastRect.y, LastRect.x + LastRect.w, LastRect.y + LastRect.h);

	LastRect = DestRect;

	if (fForward)
	{
		ubCount++;
		return ubCount == AIM_MEMBER_VIDEO_TITLE_ITERATIONS - 1;
	}
	else
	{
		ubCount--;
		return ubCount == 0;
	}
}

static void DelayMercSpeech(UINT8 ubMercID, UINT16 usQuoteNum, UINT16 usDelay, BOOLEAN fNewQuote, BOOLEAN fReset)
{
	static UINT32  uiLastTime=0;
	UINT32         uiCurTime;
	static UINT16  usCurQuoteNum;
	static UINT16  usCurDelay;
	static BOOLEAN fQuoteWaiting = FALSE; //a quote is waiting to be said
	static UINT8   ubCurMercID;
	static BOOLEAN fHangUpAfter=FALSE;

	uiCurTime = GetJA2Clock();

	if( fReset )
		fQuoteWaiting = FALSE;

	if( fNewQuote )
	{
		//set up the counters
		uiLastTime = uiCurTime;

		ubCurMercID = ubMercID;
		usCurQuoteNum = usQuoteNum;
		usCurDelay = usDelay;

		if( gfHangUpMerc )
		{
			gfHangUpMerc = FALSE;
			fHangUpAfter = TRUE;
		}


		fQuoteWaiting = TRUE;
	}


	if( fQuoteWaiting )
	{
		if( ( uiCurTime - uiLastTime ) > usCurDelay )
		{
			InitVideoFaceTalking( ubCurMercID, usCurQuoteNum );
			fQuoteWaiting = FALSE;

			if( fHangUpAfter )
			{
				gfHangUpMerc = TRUE;
				fHangUpAfter = FALSE;
			}
		}
	}
}

static void WaitForMercToFinishTalkingOrUserToClick(void)
{
	//if the region is not active
	if( !gfIsShutUpMouseRegionActive )
	{
		//Enables it so if a player clicks, he will shutup the merc
		gSelectedShutUpMercRegion.Enable();
		gfIsShutUpMouseRegionActive = TRUE;
	}

	if( gfIsAnsweringMachineActive )
		gubVideoConferencingMode = AIM_VIDEO_MERC_ANSWERING_MACHINE_MODE;
	else
		gubVideoConferencingMode = AIM_VIDEO_FIRST_CONTACT_MERC_MODE;

	gfWaitingForMercToStopTalkingOrUserToClick = TRUE;
	gfHangUpMerc = TRUE;
	gfStopMercFromTalking = FALSE;
}

static void DisplayPopUpBoxExplainingMercArrivalLocationAndTimeCallBack(MessageBoxReturnValue);


void DisplayPopUpBoxExplainingMercArrivalLocationAndTime()
{
	LAST_HIRED_MERC_STRUCT& h = LaptopSaveInfo.sLastHiredMerc;

	// if the id of the merc is invalid, dont display the pop up
	if (h.iIdOfMerc == -1) return;

	// If the pop up has already been displayed, do not display it again for this occurence of laptop
	if (h.fHaveDisplayedPopUpInLaptop) return;

	// Calculate the approximate hour the mercs will arrive at
	ST::string time_string;
	UINT32 const hour = h.uiArrivalTime % 1440 / 60;
	time_string = ST::format("{02d}:00", hour);

	ST::string sector_string = GetSectorIDString(g_merc_arrive_sector, FALSE);

	ST::string msg;
	ST::string nickname = GetProfile(h.iIdOfMerc).zNickname;
	UINT32         const day      = h.uiArrivalTime / 1440;

	// German version has a different argument order
	if(isGermanVersion())
	{
		msg = st_format_printf(pMessageStrings[MSG_JUST_HIRED_MERC_ARRIVAL_LOCATION_POPUP], nickname, day, time_string, sector_string);
	}
	else
	{
		msg = st_format_printf(pMessageStrings[MSG_JUST_HIRED_MERC_ARRIVAL_LOCATION_POPUP], nickname, sector_string, day, time_string);
	}

	DoLapTopMessageBox(MSG_BOX_LAPTOP_DEFAULT, msg, LAPTOP_SCREEN, MSG_BOX_FLAG_OK, DisplayPopUpBoxExplainingMercArrivalLocationAndTimeCallBack);

	// Reset the id of the last merc
	h.iIdOfMerc = -1;
	// Set the fact that the pop up has been displayed this time in laptop
	h.fHaveDisplayedPopUpInLaptop = TRUE;
}


static void DisplayPopUpBoxExplainingMercArrivalLocationAndTimeCallBack(MessageBoxReturnValue const bExitValue)
{
	//unset the flag so the msgbox WONT dislay its save buffer
	gfDontOverRideSaveBuffer = FALSE;

	if( guiCurrentLaptopMode == LAPTOP_MODE_AIM_MEMBERS )
	{
		//render the screen
		gfRedrawScreen = TRUE;
		RenderAIMMembers();
	}
}


static void DisplayAimMemberClickOnFaceHelpText(void)
{
	//display the 'left and right click' onscreen help msg
	DrawTextToScreen(AimMemberText[0], AIM_FI_LEFT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y,                                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_TITLE_FONT, AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(AimMemberText[1], AIM_FI_LEFT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y + AIM_FI_CLICK_DESC_TEXT_Y_OFFSET, AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_FONT,       AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	DrawTextToScreen(AimMemberText[2], AIM_FI_RIGHT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y,                                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_TITLE_FONT, AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(AimMemberText[3], AIM_FI_RIGHT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y + AIM_FI_CLICK_DESC_TEXT_Y_OFFSET, AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_FONT,       AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
}
