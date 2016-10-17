#include <sgp/VObject.h>
#include "CharProfile.h"
#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "IMP_Begin_Screen.h"
#include "IMP_MainPage.h"
#include "IMP_HomePage.h"
#include "IMPVideoObjects.h"
#include "Local.h"
#include "Timer_Control.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Finish.h"
#include "Text_Input.h"
#include "MessageBoxScreen.h"
#include "Soldier_Profile_Type.h"
#include "IMP_Portraits.h"
#include "IMP_Attribute_Selection.h"
#include "English.h"
#include "Line.h"
#include "Merc_Hiring.h"
#include "Game_Clock.h"
#include "Text.h"
#include "LaptopSave.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "ScreenIDs.h"
#include "Font_Control.h"
#include "UILayout.h"

#if defined JA2BETAVERSION && defined _DEBUG
#	include "Campaign_Types.h"
#	include "Soldier_Profile.h"
#	include "Strategic.h"
#endif


#define FULL_NAME_INPUT_X LAPTOP_SCREEN_UL_X + 196
#define FULL_NAME_INPUT_Y LAPTOP_SCREEN_UL_Y + 153
#define FULL_NAME_INPUT_WIDTH 229
#define NICK_NAME_INPUT_X FULL_NAME_INPUT_X
#define NICK_NAME_INPUT_Y LAPTOP_SCREEN_UL_Y + 213
#define NICK_NAME_INPUT_WIDTH 110
#define NAMES_INPUT_HEIGHT 23
#define FULL_NAME_CURSOR_Y LAPTOP_SCREEN_WEB_UL_Y + 138
#define NICK_NAME_CURSOR_Y LAPTOP_SCREEN_WEB_UL_Y + 195
#define MALE_BOX_X 2 + 192 + LAPTOP_SCREEN_UL_X
#define MALE_BOX_Y 254 + LAPTOP_SCREEN_WEB_UL_Y
#define MALE_BOX_WIDTH 24 - 2
#define MALE_BOX_HEIGHT 24 - 2
#define FEMALE_BOX_X  2 + 302 + LAPTOP_SCREEN_UL_X


// genders
enum {
	IMP_FEMALE = 0,
	IMP_MALE,
};

// TextEnterMode .. whether user is entering full name or nick name, or gender selection
enum {
	OTHER_INPUT,
	MALE_GENDER,
	FEMALE_GENDER
};

// beginning character stats
wchar_t pFullNameString[NAME_LENGTH];
wchar_t pNickNameString[NICKNAME_LENGTH];


// non gender
INT8 bGenderFlag = -1;

// IMP begin page buttons
static BUTTON_PICS* giIMPBeginScreenButtonImage[1];
GUIButtonRef giIMPBeginScreenButton[1];

// currently focused input
UINT8 ubFocus = 0;

static MOUSE_REGION gIMPBeginScreenMouseRegions[4];

static void CreateIMPBeginScreenButtons(void);
static void CreateIMPBeginScreenMouseRegions(void);

static void InvalidateCheckboxes() {
	InvalidateRegion(MALE_BOX_X, MALE_BOX_Y,  MALE_BOX_X + MALE_BOX_WIDTH + 1, MALE_BOX_Y + MALE_BOX_HEIGHT + 1);
	InvalidateRegion(FEMALE_BOX_X, MALE_BOX_Y,  FEMALE_BOX_X + MALE_BOX_WIDTH + 1, MALE_BOX_Y + MALE_BOX_HEIGHT + 1);

}

static void MaleCheckboxCallback(UINT8 ubID, BOOLEAN fEntering)
{
	ubFocus = fEntering ? MALE_GENDER : OTHER_INPUT;
	InvalidateCheckboxes();
}

static void FemaleCheckboxCallback(UINT8 ubID, BOOLEAN fEntering)
{
	ubFocus = fEntering ? FEMALE_GENDER : OTHER_INPUT;
	InvalidateCheckboxes();
}

void InitImpBeginScreeenTextInputBoxes() {
	InitTextInputMode();

	SetTextInputFont(FONT14ARIAL);
	Set16BPPTextFieldColor( Get16BPPColor(FROMRGB( 0, 0, 0) ) );
	SetTextInputRegularColors( FONT_LTGREEN, FONT_BLACK );
	SetTextInputHilitedColors( FONT_BLACK, FONT_LTGREEN, FONT_LTGREEN  );
	SetCursorColor( Get16BPPColor(FROMRGB(0, 255, 0) ) );

	AddTextInputField(
			FULL_NAME_INPUT_X,
			FULL_NAME_INPUT_Y,
			FULL_NAME_INPUT_WIDTH,
			NAMES_INPUT_HEIGHT,
			MSYS_PRIORITY_HIGH + 2,
			pFullName,
			NAME_LENGTH,
			INPUTTYPE_FULL_TEXT
	);

	AddTextInputField(
			NICK_NAME_INPUT_X,
			NICK_NAME_INPUT_Y,
			NICK_NAME_INPUT_WIDTH,
			NAMES_INPUT_HEIGHT,
			MSYS_PRIORITY_HIGH + 2,
			pNickName,
			NICKNAME_LENGTH,
			INPUTTYPE_FULL_TEXT
	);

	AddUserInputField(MaleCheckboxCallback);
	AddUserInputField(FemaleCheckboxCallback);
}

void EnterIMPBeginScreen( void )
{
	InitImpBeginScreeenTextInputBoxes();

	bGenderFlag = iCurrentProfileMode != 0 ? fCharacterIsMale : -1;

	ubFocus = OTHER_INPUT;

	// render the screen on entry
  RenderIMPBeginScreen( );

  if( !fFinishedCharGeneration )
	{
		fFirstIMPAttribTime = TRUE;
	}

	// create mouse regions
	CreateIMPBeginScreenMouseRegions( );

	// create buttons needed for begin screen
	CreateIMPBeginScreenButtons( );
}


static void Print8CharacterOnlyString(void);
static void RenderGender(void);


void RenderIMPBeginScreen( void )
{
  // the background
	RenderProfileBackGround( );

	// fourth button image 3X
	RenderButton4Image( 64, 118 );
	RenderButton4Image( 64, 178 );
	RenderButton4Image( 64, 238 );

	// the begin screen indents
  RenderBeginIndent( 105, 58);

	// full name indent
	RenderNameIndent( 194, 132);

	// nick name
  RenderNickNameIndent( 194, 192);

	// render warning string
	Print8CharacterOnlyString();

	RenderGender();
	RenderAllTextFields();
}


static void DestroyIMPBeginScreenMouseRegions();
static void RemoveIMPBeginScreenButtons(void);


void ExitIMPBeginScreen( void )
{
	// remove buttons
  RemoveIMPBeginScreenButtons( );

  // remove mouse regions
	DestroyIMPBeginScreenMouseRegions( );

	KillTextInputMode();

	wcscpy( pFullName, pFullNameString );

	// is nick name too long?..shorten
	if( wcslen( pNickNameString ) > 8 )
	{
		// null out char 9
		pNickNameString[ 8 ] = 0;
	}


	wcscpy( pNickName, pNickNameString );

	// set gender
	fCharacterIsMale = bGenderFlag;
}


static void DisplayFemaleGlowCursor(void);
static void DisplayMaleGlowCursor(void);
static void GetPlayerKeyBoardInputForIMPBeginScreen(void);


void HandleIMPBeginScreen( void )
{

	GetPlayerKeyBoardInputForIMPBeginScreen( );

	RenderGender();

	// render the cursor
	switch (ubFocus)
	{
		case MALE_GENDER:   DisplayMaleGlowCursor();       break;
		case FEMALE_GENDER: DisplayFemaleGlowCursor();     break;
		default: break;
	}

	RenderAllTextFields();
}


static void BtnIMPBeginScreenDoneCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateIMPBeginScreenButtons(void)
{
  // this procedure will create the buttons needed for the IMP BeginScreen

	// ths done button
	giIMPBeginScreenButtonImage[0] = LoadButtonImage(LAPTOPDIR "/button_2.sti", 0, 1);
	giIMPBeginScreenButton[0] = CreateIconAndTextButton( giIMPBeginScreenButtonImage[ 0 ], pImpButtonText[ 6 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 LAPTOP_SCREEN_UL_X + 134, LAPTOP_SCREEN_WEB_UL_Y + 314, MSYS_PRIORITY_HIGH,
														 BtnIMPBeginScreenDoneCallback);

	giIMPBeginScreenButton[0]->SetCursor(CURSOR_WWW);
}


static void RemoveIMPBeginScreenButtons(void)
{
  // this procedure will destroy the already created buttosn for the IMP BeginScreen

  // the done button
  RemoveButton(giIMPBeginScreenButton[0] );
  UnloadButtonImage(giIMPBeginScreenButtonImage[0] );
}


static BOOLEAN CheckCharacterInputForEgg(void);
static void CopyFirstNameIntoNickName(void);


static void BtnIMPBeginScreenDoneCallback(GUI_BUTTON *btn, INT32 reason)
{
	// easter egg check
	BOOLEAN fEggOnYouFace = FALSE;

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		if (fFinishedCharGeneration)
		{
			// simply reviewing name and gender, exit to finish page
			iCurrentImpPage = IMP_FINISH;
			fButtonPendingFlag = TRUE;
			return;
		}
		else
		{
			if (CheckCharacterInputForEgg())
			{
				fEggOnYouFace = TRUE;
			}
		}

		// back to mainpage

		// check to see if a name has been selected, if not, do not allow player to proceed with more char generation
		if (pFullNameString[0] != L'\0' && pFullNameString[0] != L' ' && bGenderFlag != -1)
		{
			// valid full name, check to see if nick name
			if (pNickNameString[0] == '\0' || pNickNameString[0] == L' ')
			{
				// no nick name
				// copy first name to nick name
				CopyFirstNameIntoNickName();
			}
			// ok, now set back to main page, and set the fact we have completed part 1
			if (iCurrentProfileMode < 1 && bGenderFlag != -1)
			{
				iCurrentProfileMode = 1;
			}
			else if (bGenderFlag == -1)
			{
				iCurrentProfileMode = 0;
			}

			// no easter egg?...then proceed along
			if (!fEggOnYouFace)
			{
				iCurrentImpPage = IMP_MAIN_PAGE;
				fButtonPendingFlag = TRUE;
			}
		}
		else
		{
			// invalid name, reset current mode
			DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[2], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, NULL);
			iCurrentProfileMode = 0;
		}
	}
}

static void GetPlayerKeyBoardInputForIMPBeginScreen(void)
{
	InputAtom					InputEvent;

	// handle input events
  while( DequeueEvent(&InputEvent) )
  {
    if(	!HandleTextInput( &InputEvent ) && (InputEvent.usEvent == KEY_DOWN || InputEvent.usEvent == KEY_REPEAT) )
		{
		  switch( InputEvent.usParam )
			{
				case SDLK_RETURN:
				case SDLK_SPACE:
					if (ubFocus != OTHER_INPUT) {
						bGenderFlag = ubFocus == MALE_GENDER ? IMP_MALE : IMP_FEMALE;
					}
					SetActiveField(0);
					break;

		    case SDLK_ESCAPE:
					HandleLapTopESCKey();
					break;

				default:
					break;
			}
		}
  }
}

static UINT16 CurrentGlowColour(void)
{
	static UINT32 uiBaseTime    = 0;
	static UINT32 iCurrentState = 0;
	static BOOLEAN fIncrement   = TRUE;

	if (uiBaseTime == 0) uiBaseTime = GetJA2Clock();

	// get difference
	UINT32 uiDeltaTime = GetJA2Clock() - uiBaseTime;

	// if difference is long enough, rotate colors
	if (uiDeltaTime > MIN_GLOW_DELTA)
	{
		if (iCurrentState == 10) fIncrement = FALSE; // start rotating downward
		if (iCurrentState ==  0) fIncrement = TRUE;  // rotate colors upward
		iCurrentState = iCurrentState + (fIncrement ? 1 : -1);
		// reset basetime to current clock
		uiBaseTime = GetJA2Clock();
	}

	return Get16BPPColor(GlowColorsList[iCurrentState]);
}

static void DisplayGenderGlowCursor(INT32 x)
{
	// this procdure will draw the activation string cursor on the screen at position cursorx cursory
	SGPVSurface::Lock l(FRAME_BUFFER);
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  RectangleDraw(TRUE, x, MALE_BOX_Y, x + MALE_BOX_WIDTH, MALE_BOX_Y + MALE_BOX_HEIGHT, CurrentGlowColour(), l.Buffer<UINT16>());
  InvalidateRegion(x, MALE_BOX_Y,  x + MALE_BOX_WIDTH + 1, MALE_BOX_Y + MALE_BOX_HEIGHT + 1);
}

static void DisplayMaleGlowCursor(void)
{
	DisplayGenderGlowCursor(MALE_BOX_X);
}


static void DisplayFemaleGlowCursor(void)
{
	DisplayGenderGlowCursor(FEMALE_BOX_X);
}


static void CopyFirstNameIntoNickName(void)
{
	// this procedure will copy the characters first name in to the nickname for the character
	UINT32 iCounter=0;
  while( ( pFullNameString[ iCounter ] != L' ' ) && ( iCounter < NICKNAME_LENGTH) && ( pFullNameString[ iCounter ] != 0 ) )
	{
		// copy charcters into nick name
		pNickNameString[ iCounter ] = pFullNameString[ iCounter ];
		iCounter++;
	}
}

static void MvtOnFemaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void MvtOnMaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectFemaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void SelectMaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);

static void CreateIMPBeginScreenMouseRegions(void)
{
	// IMP_MALE gender area
  MSYS_DefineRegion(&gIMPBeginScreenMouseRegions[ 2 ] , MALE_BOX_X, MALE_BOX_Y,   MALE_BOX_X + MALE_BOX_WIDTH, MALE_BOX_Y + MALE_BOX_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW,
		MvtOnMaleRegionCallBack, SelectMaleRegionCallBack);

	// IMP_FEMALE gender region
	MSYS_DefineRegion(&gIMPBeginScreenMouseRegions[ 3 ] , FEMALE_BOX_X, MALE_BOX_Y,   FEMALE_BOX_X + MALE_BOX_WIDTH, MALE_BOX_Y + MALE_BOX_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW,
		MvtOnFemaleRegionCallBack, SelectFemaleRegionCallBack);
}


static void DestroyIMPBeginScreenMouseRegions()
{
	FOR_EACH(MOUSE_REGION, i, gIMPBeginScreenMouseRegions) MSYS_RemoveRegion(&*i);
}

static void SelectMaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set mode to nick name type in
		bGenderFlag = IMP_MALE;
		InvalidateCheckboxes();
	}
}


static void SelectFemaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// set mode to nick name type in
		bGenderFlag = IMP_FEMALE;
		InvalidateCheckboxes();
	}
}


static void MvtOnFemaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
    SetActiveField(3);
		InvalidateCheckboxes();
	}
}


static void MvtOnMaleRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE)
	{
		SetActiveField(2);
		InvalidateCheckboxes();
	}
}


static void RenderGender(void)
{
  // this procedure will render the gender of the character int he appropriate box
	RenderGenderIndent(192, 252);
	RenderGenderIndent(302, 252);

	INT32 x;
	switch (bGenderFlag)
	{
		case IMP_FEMALE: x = FEMALE_BOX_X + 9; break;
		case IMP_MALE:   x = MALE_BOX_X   + 9; break;
		default:         return; // none selected yet
	}
	SetFontBackground(FONT_BLACK);
	SetFontAttributes(FONT14ARIAL, 184);
	MPrint(x, MALE_BOX_Y + 6, L"X");
}


static void Print8CharacterOnlyString(void)
{
	SetFontAttributes(FONT12ARIAL, FONT_BLACK, NO_SHADOW);
	MPrint(STD_SCREEN_X + 430, STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y + 228, pIMPBeginScreenStrings);
	SetFontShadow(DEFAULT_SHADOW);
}


static BOOLEAN CheckCharacterInputForEgg(void)
{
#if defined JA2BETAVERSION && defined _DEBUG
	if (wcscmp(pFullNameString, L"retraC kraM") == 0 && wcscmp(pNickNameString, L"BdyCnt"))
	{
		wcscpy(pFullNameString, L"Mark Carter");
		wcscpy(pNickNameString, L"BodyCount");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 99;
		iAgility                = 99;
		iStrength               = 99;
		iDexterity              = 99;
		iWisdom                 = 35;
		iLeadership             = 35;
		iMarksmanship           = 99;
		iMechanical             =  0;
		iExplosives             = 99;
		iMedical                =  0;
		iSkillA                 = AMBIDEXT;
		iSkillB                 = AUTO_WEAPS;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_ASSHOLE;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 2;
	}
	else if (wcscmp(pFullNameString, L"hcnerF evaD") == 0 && wcscmp(pNickNameString, L"Feral") == 0)
	{
		wcscpy(pFullNameString, L"Dave French");
		wcscpy(pNickNameString, L"Feral");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 90;
		iAgility                = 95;
		iStrength               = 90;
		iDexterity              = 95;
		iWisdom                 = 60;
		iLeadership             = 60;
		iMarksmanship           = 89;
		iMechanical             = 40;
		iExplosives             = 25;
		iMedical                = 60;
		iSkillA                 = MARTIALARTS;
		iSkillB                 = AUTO_WEAPS;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_FRIENDLY;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 1;
	}
	else if (wcscmp(pFullNameString, L"Marnes") == 0 && wcscmp(pNickNameString, L"Marnes") == 0)
	{
		wcscpy(pFullNameString, L"Kris Marnes");
		wcscpy(pNickNameString, L"Marnes");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 100;
		iAgility                = 100;
		iStrength               = 100;
		iDexterity              = 100;
		iWisdom                 = 100;
		iLeadership             = 100;
		iMarksmanship           = 100;
		iMechanical             = 100;
		iExplosives             = 100;
		iMedical                = 100;
		iSkillA                 = MARTIALARTS;
		iSkillB                 = AUTO_WEAPS;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_FRIENDLY;
		LaptopSaveInfo.iVoiceId = 2;
		iPortraitNumber         = 1;
	}
	else if (wcscmp(pFullNameString, L"neslO namroN") == 0 && wcscmp(pNickNameString, L"N.R.G") == 0)
	{
		wcscpy(pFullNameString, L"Norman Olsen");
		wcscpy(pNickNameString, L"N.R.G");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 99;
		iAgility                = 99;
		iStrength               = 99;
		iDexterity              = 90;
		iWisdom                 = 70;
		iLeadership             = 35;
		iMarksmanship           = 70;
		iMechanical             = 65;
		iExplosives             = 99;
		iMedical                = 75;
		iSkillA                 = STEALTHY;
		iSkillB                 = MARTIALARTS;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_AGGRESSIVE;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 4;
	}
	else if (wcscmp(pFullNameString, L"snommE werdnA") == 0 && wcscmp(pNickNameString, L"Bubba") == 0)
	{
		wcscpy(pFullNameString, L"Andrew Emmons");
		wcscpy(pNickNameString, L"Bubba");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 97;
		iAgility                = 98;
		iStrength               = 80;
		iDexterity              = 80;
		iWisdom                 = 99;
		iLeadership             = 99;
		iMarksmanship           = 99;
		iMechanical             =  0;
		iExplosives             =  0;
		iMedical                = 99;
		iSkillA                 = AUTO_WEAPS;
		iSkillB                 = AMBIDEXT;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_ARROGANT;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 2;
	}
	else if (wcscmp(pFullNameString, L"nalehW yeoJ") == 0 && wcscmp(pNickNameString, L"Joeker") == 0)
	{
		wcscpy(pFullNameString, L"Joey Whelan");
		wcscpy(pNickNameString, L"Joeker");
		bGenderFlag             =  0;
		iHealth                 = 99;
		iAgility                = 99;
		iStrength               = 99;
		iDexterity              = 99;
		iWisdom                 = 70;
		iLeadership             = 80;
		iMarksmanship           = 99;
		iMechanical             = 35;
		iExplosives             = 99;
		iMedical                = 35;
		iSkillA                 = AUTO_WEAPS;
		iSkillB                 = MARTIALARTS;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_AGGRESSIVE;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 4;
	}
	else if (wcscmp(pFullNameString, L"gnehC cirE") == 0 && wcscmp(pNickNameString, L"BlakAddr") == 0)
	{
		wcscpy(pFullNameString, L"Eric Cheng");
		wcscpy(pNickNameString, L"BlakAddr");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 99;
		iAgility                = 99;
		iStrength               = 99;
		iDexterity              = 99;
		iWisdom                 = 99;
		iLeadership             = 70;
		iMarksmanship           = 99;
		iMechanical             = 50;
		iExplosives             = 99;
		iMedical                =  0;
		iSkillA                 = AUTO_WEAPS;
		iSkillB                 = MARTIALARTS;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_LONER;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 3;
	}
	else if (wcscmp(pFullNameString, L"Karters Killer Kru") == 0 && wcscmp(pNickNameString, L"Bitchin") == 0)
	{
		wcscpy(pFullNameString, L"Mark Carter");
		wcscpy(pNickNameString, L"BodyCount");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 99;
		iAgility                = 99;
		iStrength               = 99;
		iDexterity              = 99;
		iWisdom                 = 35;
		iLeadership             = 35;
		iMarksmanship           = 99;
		iMechanical             =  0;
		iExplosives             = 99;
		iMedical                =  0;
		iSkillA                 = AMBIDEXT;
		iSkillB                 = AUTO_WEAPS;
		iPersonality            = PSYCHO;
		iAttitude               = ATT_ASSHOLE;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 2;

		MERC_HIRE_STRUCT HireMercStruct;
		HireMercStruct.sSectorX                  = SECTORX(g_merc_arrive_sector);
		HireMercStruct.sSectorY                  = SECTORY(g_merc_arrive_sector);
		HireMercStruct.bSectorZ                  = 0;
		HireMercStruct.fUseLandingZoneForArrival = TRUE;
		HireMercStruct.ubInsertionCode           = INSERTION_CODE_ARRIVING_GAME;
		HireMercStruct.fCopyProfileItemsOver     = TRUE;
		HireMercStruct.iTotalContractLength      = 365;
		HireMercStruct.uiTimeTillMercArrives     = GetMercArrivalTimeOfDay();

		HireMercStruct.ubProfileID = REAPER;
		HireMerc(HireMercStruct);

		HireMercStruct.ubProfileID = SHADOW;
		HireMerc(HireMercStruct);

		HireMercStruct.ubProfileID = RAVEN;
		HireMerc(HireMercStruct);

		HireMercStruct.ubProfileID = LYNX;
		HireMerc(HireMercStruct);

		HireMercStruct.ubProfileID = CLIFF;
		HireMerc(HireMercStruct);
	}
	else if (wcscmp(pFullNameString, L"dleifmaC sirhC") == 0 && wcscmp(pNickNameString, L"SSR") == 0)
	{
		wcscpy(pFullNameString, L"James Bolivar DiGriz");
		wcscpy(pNickNameString, L"DiGriz");
		bGenderFlag             = IMP_MALE;
		iHealth                 = 99;
		iAgility                = 80;
		iStrength               = 80;
		iDexterity              = 99;
		iWisdom                 = 70;
		iLeadership             = 70;
		iMarksmanship           = 99;
		iMechanical             = 99;
		iExplosives             = 99;
		iMedical                = 60;
		iSkillA                 = ELECTRONICS;
		iSkillB                 = LOCKPICKING;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_LONER;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 3;
	}
	else if (wcscmp(pFullNameString, L"Test Female") == 0 && wcscmp(pNickNameString, L"Test") == 0)
	{
		wcscpy(pFullNameString, L"Test Female");
		wcscpy(pNickNameString, L"Test");
		bGenderFlag             = IMP_FEMALE;
		iHealth                 = 55;
		iAgility                = 55;
		iStrength               = 55;
		iDexterity              = 55;
		iWisdom                 = 55;
		iLeadership             = 55;
		iMarksmanship           = 55;
		iMechanical             = 55;
		iExplosives             = 55;
		iMedical                = 55;
		iSkillA                 = NO_SKILLTRAIT;
		iSkillB                 = NO_SKILLTRAIT;
		iPersonality            = NO_PERSONALITYTRAIT;
		iAttitude               = ATT_LONER;
		LaptopSaveInfo.iVoiceId = 1;
		iPortraitNumber         = 5;
	}
	else
	{
		return FALSE;
	}

	iCurrentImpPage = IMP_FINISH;
	return TRUE;
#else
	return FALSE;
#endif
}
