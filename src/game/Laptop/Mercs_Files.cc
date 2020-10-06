#include "Mercs_Files.h"
#include "AIMMembers.h"
#include "Assignments.h"
#include "Button_System.h"
#include "Campaign_Types.h"
#include "Cheats.h"
#include "ContentManager.h"
#include "Cursors.h"
#include "Directories.h"
#include "English.h"
#include "Facts.h"
#include "Font.h"
#include "Font_Control.h"
#include "GameInstance.h"
#include "GameRes.h"
#include "HImage.h"
#include "Input.h"
#include "JA2Types.h"
#include "Laptop.h"
#include "LaptopSave.h"
#include "Merc_Hiring.h"
#include "MercPortrait.h"
#include "Mercs.h"
#include "Quests.h"
#include "ScreenIDs.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "Soldier_Profile_Type.h"
#include "Speck_Quotes.h"
#include "Text.h"
#include "Types.h"
#include "UILayout.h"
#include "Video.h"
#include "VObject.h"
#include "VSurface.h"
#include "WordWrap.h"
#include <string_theory/format>
#include <string_theory/string>
struct BUTTON_PICS;


#define MERCBIOFILE			BINARYDATADIR "/mercbios.edt"

#define MERC_BIO_FONT			FONT14ARIAL//FONT12ARIAL
#define MERC_BIO_COLOR			FONT_MCOLOR_WHITE

#define MERC_TITLE_FONT			FONT14ARIAL
#define MERC_TITLE_COLOR		146

#define MERC_NAME_FONT			FONT14ARIAL
#define MERC_NAME_COLOR			FONT_MCOLOR_WHITE

#define MERC_STATS_FONT			FONT12ARIAL
#define MERC_STATIC_STATS_COLOR		146
#define MERC_DYNAMIC_STATS_COLOR	FONT_MCOLOR_WHITE


#define MERC_FILES_PORTRAIT_BOX_X	LAPTOP_SCREEN_UL_X + 16
#define MERC_FILES_PORTRAIT_BOX_Y	LAPTOP_SCREEN_WEB_UL_Y + 17

#define MERC_FACE_X			MERC_FILES_PORTRAIT_BOX_X + 2
#define MERC_FACE_Y			MERC_FILES_PORTRAIT_BOX_Y + 2
#define MERC_FACE_WIDTH			106
#define MERC_FACE_HEIGHT		122

#define MERC_FILES_STATS_BOX_X		LAPTOP_SCREEN_UL_X + 164
#define MERC_FILES_STATS_BOX_Y		MERC_FILES_PORTRAIT_BOX_Y


#define MERC_FILES_BIO_BOX_X		MERC_FILES_PORTRAIT_BOX_X
#define MERC_FILES_BIO_BOX_Y		LAPTOP_SCREEN_WEB_UL_Y + 155

#define MERC_FILES_PREV_BUTTON_X	(STD_SCREEN_X + 128)
#define MERC_FILES_NEXT_BUTTON_X	(STD_SCREEN_X + 490)
#define MERC_FILES_HIRE_BUTTON_X	(STD_SCREEN_X + 260)
#define MERC_FILES_BACK_BUTTON_X	(STD_SCREEN_X + 380)
#define MERC_FILES_BUTTON_Y		(STD_SCREEN_Y + 380)

#define MERC_NAME_X			MERC_FILES_STATS_BOX_X + 50
#define MERC_NAME_Y			MERC_FILES_STATS_BOX_Y + 10

#define MERC_BIO_TEXT_X			MERC_FILES_BIO_BOX_X + 5
#define MERC_BIO_TEXT_Y			MERC_FILES_BIO_BOX_Y + 10

#define MERC_ADD_BIO_TITLE_X		MERC_BIO_TEXT_X
#define MERC_ADD_BIO_TITLE_Y		MERC_BIO_TEXT_Y + 100

#define MERC_ADD_BIO_TEXT_X		MERC_BIO_TEXT_X
#define MERC_ADD_BIO_TEXT_Y		MERC_ADD_BIO_TITLE_Y + 20

#define MERC_BIO_WIDTH			460 - 10

#define MERC_BIO_INFO_TEXT_SIZE		5 * 80
#define MERC_BIO_ADD_INFO_TEXT_SIZE	2 * 80
#define MERC_BIO_SIZE			7 * 80

#define MERC_STATS_FIRST_COL_X		MERC_NAME_X
#define MERC_STATS_FIRST_NUM_COL_X	MERC_STATS_FIRST_COL_X + 90
#define MERC_STATS_SECOND_COL_X		MERC_FILES_STATS_BOX_X + 170
#define MERC_STATS_SECOND_NUM_COL_X	MERC_STATS_SECOND_COL_X + 115
#define MERC_SPACE_BN_LINES		15

#define MERC_HEALTH_Y			MERC_FILES_STATS_BOX_Y + 30

#define MERC_PORTRAIT_TEXT_OFFSET_Y	110


static SGPVObject* guiPortraitBox;
static SGPVObject* guiStatsBox;
static SGPVObject* guiBioBox;

//
// Buttons
//

// The Prev button
static void BtnMercPrevButtonCallback(GUI_BUTTON *btn, INT32 reason);
static BUTTON_PICS* guiButtonImage;
GUIButtonRef guiPrevButton;

// The Next button
static void BtnMercNextButtonCallback(GUI_BUTTON *btn, INT32 reason);
GUIButtonRef guiNextButton;

// The Hire button
static void BtnMercHireButtonCallback(GUI_BUTTON *btn, INT32 reason);
GUIButtonRef guiHireButton;

// The Back button
static void BtnMercFilesBackButtonCallback(GUI_BUTTON *btn, INT32 reason);
GUIButtonRef guiMercBackButton;


static GUIButtonRef MakeButton(const ST::string& text, INT16 x, GUI_CALLBACK click)
{
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(guiButtonImage, text, FONT12ARIAL, MERC_BUTTON_UP_COLOR, shadow_col, MERC_BUTTON_DOWN_COLOR, shadow_col, x, MERC_FILES_BUTTON_Y, MSYS_PRIORITY_HIGH, click);
	btn->SetCursor(CURSOR_LAPTOP_SCREEN);
	btn->SpecifyDisabledStyle(GUI_BUTTON::DISABLED_STYLE_SHADED);
	return btn;
}


void EnterMercsFiles()
{
	InitMercBackGround();

	// load the stats box graphic and add it
	const char* const ImageFile = GetMLGFilename(MLG_STATSBOX);
	guiStatsBox = AddVideoObjectFromFile(ImageFile);

	// load the Portrait box graphic and add it
	guiPortraitBox = AddVideoObjectFromFile(LAPTOPDIR "/portraitbox.sti");

	// load the bio box graphic and add it
	guiBioBox = AddVideoObjectFromFile(LAPTOPDIR "/biobox.sti");

	guiButtonImage    = LoadButtonImage(LAPTOPDIR "/bigbuttons.sti", 0, 1);
	guiPrevButton     = MakeButton(MercInfo[MERC_FILES_PREVIOUS], MERC_FILES_PREV_BUTTON_X, BtnMercPrevButtonCallback);
	guiNextButton     = MakeButton(MercInfo[MERC_FILES_NEXT],     MERC_FILES_NEXT_BUTTON_X, BtnMercNextButtonCallback);
	guiHireButton     = MakeButton(MercInfo[MERC_FILES_HIRE],     MERC_FILES_HIRE_BUTTON_X, BtnMercHireButtonCallback);
	guiMercBackButton = MakeButton(MercInfo[MERC_FILES_HOME],     MERC_FILES_BACK_BUTTON_X, BtnMercFilesBackButtonCallback);

	//RenderMercsFiles();
}


void ExitMercsFiles()
{
	DeleteVideoObject(guiPortraitBox);
	DeleteVideoObject(guiStatsBox);
	DeleteVideoObject(guiBioBox);

	UnloadButtonImage( guiButtonImage );
	RemoveButton( guiPrevButton );
	RemoveButton( guiNextButton );
	RemoveButton( guiHireButton );
	RemoveButton( guiMercBackButton );

	RemoveMercBackGround();
}


static void DisplayMercFace(ProfileID);
static void DisplayMercsStats(MERCPROFILESTRUCT const&);
static void EnableDisableMercFilesNextPreviousButton(void);
static void LoadAndDisplayMercBio(UINT8 ubMercID);


void RenderMercsFiles()
{
	DrawMecBackGround();

	BltVideoObject(FRAME_BUFFER, guiPortraitBox, 0, MERC_FILES_PORTRAIT_BOX_X, MERC_FILES_PORTRAIT_BOX_Y);
	BltVideoObject(FRAME_BUFFER, guiStatsBox,    0, MERC_FILES_STATS_BOX_X,    MERC_FILES_STATS_BOX_Y);
	BltVideoObject(FRAME_BUFFER, guiBioBox,      0, MERC_FILES_BIO_BOX_X + 1,  MERC_FILES_BIO_BOX_Y);

	ProfileID         const  pid = GetProfileIDFromMERCListingIndex(gubCurMercIndex);
	MERCPROFILESTRUCT const& p   = GetProfile(pid);

	//Display the mercs face
	DisplayMercFace(pid);

	//Display Mercs Name
	DrawTextToScreen(p.zName, MERC_NAME_X, MERC_NAME_Y, 0, MERC_NAME_FONT, MERC_NAME_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Load and display the mercs bio
	LoadAndDisplayMercBio((UINT8)(pid - BIFF));

	//Display the mercs statistic
	DisplayMercsStats(p);

	bool const enable =
		!IsMercDead(p) &&
		(
			LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_VALID     ||
			LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_SUSPENDED ||
			LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_VALID_FIRST_WARNING
		);
	EnableButton(guiHireButton, enable);

	//Enable or disable the buttons
	EnableDisableMercFilesNextPreviousButton();

	MarkButtonsDirty();
	RenderWWWProgramTitleBar();
	InvalidateRegion(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y, LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y);
}


static void BtnMercPrevButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (gubCurMercIndex > 0) gubCurMercIndex--;
		fReDrawScreenFlag = TRUE;
		EnableDisableMercFilesNextPreviousButton();
	}
}


static void BtnMercNextButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (gubCurMercIndex <= LaptopSaveInfo.gubLastMercIndex - 1) gubCurMercIndex++;
		fReDrawScreenFlag = TRUE;
		EnableDisableMercFilesNextPreviousButton( );
	}
}


static BOOLEAN MercFilesHireMerc(UINT8 ubMercID);


static void BtnMercHireButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//if the players accont is suspended, go back to the main screen and have Speck inform the players
		if (LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_SUSPENDED)
		{
			guiCurrentLaptopMode = LAPTOP_MODE_MERC;
			gusMercVideoSpeckSpeech = SPECK_QUOTE_ALTERNATE_OPENING_5_PLAYER_OWES_SPECK_ACCOUNT_SUSPENDED;
			gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;
		}
		else if (MercFilesHireMerc(GetProfileIDFromMERCListingIndex(gubCurMercIndex)))
		{
			// else try to hire the merc
			guiCurrentLaptopMode = LAPTOP_MODE_MERC;
			gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;

			gfJustHiredAMercMerc = TRUE;
			DisplayPopUpBoxExplainingMercArrivalLocationAndTime();
		}
	}
}


static void DisplayMercFace(const ProfileID pid)
try
{
	BltVideoObject(FRAME_BUFFER, guiPortraitBox, 0, MERC_FILES_PORTRAIT_BOX_X, MERC_FILES_PORTRAIT_BOX_Y);

	MERCPROFILESTRUCT const&       p = GetProfile(pid);
	SOLDIERTYPE       const* const s = FindSoldierByProfileIDOnPlayerTeam(pid);

	// Load the face graphic
	AutoSGPVObject face(LoadBigPortrait(p));

	BOOLEAN        shaded;
	ST::string text;
	if (IsMercDead(p))
	{
		// The merc is dead, shade the face red and put text over top saying the merc is dead
		face->pShades[0] = Create16BPPPaletteShaded(face->Palette(), DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);
		face->CurrentShade(0);
		shaded = FALSE;
		text   = MercInfo[MERC_FILES_MERC_IS_DEAD];
	}
	else if (pid == FLO && gubFact[FACT_PC_MARRYING_DARYL_IS_FLO])
	{
		shaded = TRUE;
		text   = pPersonnelDepartedStateStrings[2];
	}
	else if (p.bMercStatus == MERC_FIRED_AS_A_POW || (s && s->bAssignment == ASSIGNMENT_POW))
	{
		// The merc is currently a POW or the merc was fired as a pow
		shaded = TRUE;
		text   = pPOWStrings[0];
	}
	else if (p.bMercStatus == MERC_HIRED_BUT_NOT_ARRIVED_YET || p.bMercStatus > 0)
	{
		// The merc is hired already
		shaded = TRUE;
		text   = MercInfo[MERC_FILES_ALREADY_HIRED];
	}
	else if (!IsMercHireable(p))
	{
		// The merc is away on another assignemnt, say the merc is unavailable
		shaded = TRUE;
		text   = MercInfo[MERC_FILES_MERC_UNAVAILABLE];
	}
	else
	{
		shaded = FALSE;
		text   = ST::null;
	}

	BltVideoObject(FRAME_BUFFER, face.get(), 0, MERC_FACE_X, MERC_FACE_Y);

	if (shaded)
	{
		FRAME_BUFFER->ShadowRect(MERC_FACE_X, MERC_FACE_Y, MERC_FACE_X + MERC_FACE_WIDTH, MERC_FACE_Y + MERC_FACE_HEIGHT);
	}

	if (text != NULL)
	{
		DisplayWrappedString(MERC_FACE_X, MERC_FACE_Y + MERC_PORTRAIT_TEXT_OFFSET_Y, MERC_FACE_WIDTH, 2, FONT14ARIAL, 145, text, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}
}
catch (...) { /* XXX ignore */ }


static void LoadAndDisplayMercBio(UINT8 ubMercID)
{
	UINT32 uiStartLoc;

	{
		//load and display the merc bio
		uiStartLoc = MERC_BIO_SIZE * ubMercID;
		ST::string sText = GCM->loadEncryptedString(MERCBIOFILE, uiStartLoc, MERC_BIO_INFO_TEXT_SIZE);
		DisplayWrappedString(MERC_BIO_TEXT_X, MERC_BIO_TEXT_Y, MERC_BIO_WIDTH, 2, MERC_BIO_FONT, MERC_BIO_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	}

	{
		//load and display the merc's additioanl info (if any)
		uiStartLoc += MERC_BIO_INFO_TEXT_SIZE;
		ST::string sText = GCM->loadEncryptedString(MERCBIOFILE, uiStartLoc, MERC_BIO_ADD_INFO_TEXT_SIZE);
		if( sText[0] != 0 )
		{
			DrawTextToScreen(MercInfo[MERC_FILES_ADDITIONAL_INFO], MERC_ADD_BIO_TITLE_X, MERC_ADD_BIO_TITLE_Y, 0, MERC_TITLE_FONT, MERC_TITLE_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
			DisplayWrappedString(MERC_ADD_BIO_TEXT_X, MERC_ADD_BIO_TEXT_Y, MERC_BIO_WIDTH, 2, MERC_BIO_FONT, MERC_BIO_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
		}
	}
}


static void DrawStat(UINT16 x, UINT16 y, const ST::string& stat, UINT16 x_val, INT32 val)
{
	DrawTextToScreen(stat, x, y, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(val, 3, x_val, y, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
}


static void DisplayMercsStats(MERCPROFILESTRUCT const& p)
{
	const UINT16 x1     = MERC_STATS_FIRST_COL_X;
	const UINT16 x1_val = MERC_STATS_FIRST_NUM_COL_X;
	UINT16       y1     = MERC_HEALTH_Y;
	const UINT16 dy     = MERC_SPACE_BN_LINES;
	DrawStat(x1, y1,       str_stat_health,     x1_val, p.bLife);
	DrawStat(x1, y1 += dy, str_stat_agility,    x1_val, p.bAgility);
	DrawStat(x1, y1 += dy, str_stat_dexterity,  x1_val, p.bDexterity);
	DrawStat(x1, y1 += dy, str_stat_strength,   x1_val, p.bStrength);
	DrawStat(x1, y1 += dy, str_stat_leadership, x1_val, p.bLeadership);
	DrawStat(x1, y1 += dy, str_stat_wisdom,     x1_val, p.bWisdom);

	const UINT16 x2     = MERC_STATS_SECOND_COL_X;
	const UINT16 x2_val = MERC_STATS_SECOND_NUM_COL_X;
	UINT16       y2     = MERC_HEALTH_Y;
	DrawStat(x2, y2,       str_stat_exp_level,    x2_val, p.bExpLevel);
	DrawStat(x2, y2 += dy, str_stat_marksmanship, x2_val, p.bMarksmanship);
	DrawStat(x2, y2 += dy, str_stat_mechanical,   x2_val, p.bMechanical);
	DrawStat(x2, y2 += dy, str_stat_explosive,    x2_val, p.bExplosive);
	DrawStat(x2, y2 += dy, str_stat_medical,      x2_val, p.bMedical);

	//Daily Salary
	y2 += dy;
	ST::string salary = MercInfo[MERC_FILES_SALARY];
	DrawTextToScreen(salary, MERC_STATS_SECOND_COL_X, y2, 0, MERC_NAME_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	const UINT16 x = MERC_STATS_SECOND_COL_X + StringPixLength(salary, MERC_NAME_FONT) + 1;
	ST::string sString = ST::format("{} {}", p.sSalary, MercInfo[MERC_FILES_PER_DAY]);
	DrawTextToScreen(sString, x, y2, 95, MERC_NAME_FONT, MERC_DYNAMIC_STATS_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
}


static BOOLEAN MercFilesHireMerc(UINT8 ubMercID)
{
	MERC_HIRE_STRUCT HireMercStruct;
	INT8	bReturnCode;

	HireMercStruct = MERC_HIRE_STRUCT{};
	MERCPROFILESTRUCT& p = GetProfile(ubMercID);

	//if the ALT key is down
	if (_KeyDown(ALT) && CHEATER_CHEAT_LEVEL())
	{
		//set the merc to be hireable
		p.bMercStatus           = MERC_OK;
		p.uiDayBecomesAvailable = 0;
	}

	//if the merc is away, dont hire
	if (!IsMercHireable(p))
	{
		if (p.bMercStatus != MERC_IS_DEAD)
		{
			guiCurrentLaptopMode = LAPTOP_MODE_MERC;
			gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_TRIES_TO_HIRE_ALREADY_HIRED_MERC;
			gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;
		}

		return(FALSE);
	}

	HireMercStruct.ubProfileID = ubMercID;
	HireMercStruct.bWhatKindOfMerc = MERC_TYPE__MERC;


	//HireMercStruct.fCopyProfileItemsOver = gfBuyEquipment;

	HireMercStruct.fCopyProfileItemsOver = TRUE;

	HireMercStruct.iTotalContractLength = 1;

	//Specify where the merc is to appear
	HireMercStruct.sSectorX                  = SECTORX(g_merc_arrive_sector);
	HireMercStruct.sSectorY                  = SECTORY(g_merc_arrive_sector);
	HireMercStruct.fUseLandingZoneForArrival = TRUE;

	HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay( );// + ubMercID


	//Set the time and ID of the last hired merc will arrive
	//LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = HireMercStruct.ubProfileID;
	//LaptopSaveInfo.sLastHiredMerc.uiArrivalTime = HireMercStruct.uiTimeTillMercArrives;


	bReturnCode = HireMerc(HireMercStruct);
	//already have 20 mercs on the team
	if( bReturnCode == MERC_HIRE_OVER_20_MERCS_HIRED )
	{
		DoLapTopMessageBox( MSG_BOX_LAPTOP_DEFAULT, MercInfo[ MERC_FILES_HIRE_TO_MANY_PEOPLE_WARNING ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
		return(FALSE);
	}
	else if( bReturnCode == MERC_HIRE_FAILED )
	{
		//function failed
		return(FALSE);
	}
	else
	{
		//if we succesfully hired the merc
		return(TRUE);
	}
}


static void BtnMercFilesBackButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_MERC;
		gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;
	}
}


static void EnableDisableMercFilesNextPreviousButton(void)
{
	EnableButton(guiNextButton, gubCurMercIndex <= LaptopSaveInfo.gubLastMercIndex - 1);
	EnableButton(guiPrevButton, gubCurMercIndex > 0);
}
