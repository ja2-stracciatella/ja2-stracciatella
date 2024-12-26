#include "Directories.h"
#include "Font.h"
#include "Text.h"
#include "WordWrap.h"
#include "IMP_Text_System.h"
#include "CharProfile.h"
#include "Laptop.h"
#include "IMP_Personality_Quiz.h"
#include "IMP_Personality_Finish.h"
#include "IMP_Attribute_Selection.h"
#include "IMP_MainPage.h"
#include "Font_Control.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/string>


#define IMP_SEEK_AMOUNT (5 * 80)

#define IMP_LEFT_IDENT_TEXT_X  (STD_SCREEN_X + 116)
#define IMP_RIGHT_IDENT_TEXT_X (STD_SCREEN_X + 509)
#define IMP_IDENT_WIDTH         96


// the length of persona questions
static const INT32 iIMPQuestionLengths[25] =
{
	7, 5, 5, 6, 5, 6, 5, 5, 5, 5, 6, 9, 5, 5, 5, 5, 5, 5, 5, 5, 7, 10, 6, 5, 5,
};


#define QTN_FIRST_COLUMN_X   80
#define QTN_SECOND_COLUMN_X 320


static void LoadAndDisplayIMPText(INT16 sStartX, INT16 sStartY, INT16 sLineLength, INT16 sIMPTextRecordNumber, SGPFont const font, UINT8 ubColor, BOOLEAN fShadow, UINT32 uiFlags)
{
	// this procedure will load and display to the screen starting at postion X, Y relative to the start of the laptop screen
	// it will access record sIMPTextRecordNumber and go until all records following it but before the next IMP record are displayed in the specified font
	if (!fShadow)
	{
		// don't want shadow, remove it
		SetFontShadow(NO_SHADOW);
	}

	ST::string sString = GCM->loadEncryptedString(BINARYDATADIR "/imptext.edt", sIMPTextRecordNumber * IMP_SEEK_AMOUNT, IMP_SEEK_AMOUNT);
	DisplayWrappedString(sStartX, sStartY, sLineLength, 2, font, ubColor, sString, FONT_BLACK, uiFlags);

	// reset shadow
	SetFontShadow(DEFAULT_SHADOW);
}


static void PrintIMPPersonalityQuizQuestionAndAnswers(void);


void PrintImpText(void)
{
	INT16 sWidth = LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X + 1;

	// looks at current page and prints text needed
	switch (iCurrentImpPage)
	{
		case IMP_HOME_PAGE:
			// the imp homepage
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,     STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y +  43, sWidth,          IMP_HOME_1,  FONT14ARIAL, FONT_WHITE, true,  CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,     STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y +  60, sWidth,          IMP_HOME_2,  FONT10ARIAL, FONT_WHITE, true,  CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,     STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y + 208, sWidth,          IMP_HOME_3,  FONT14ARIAL, FONT_WHITE, true,  CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(IMP_LEFT_IDENT_TEXT_X,  STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y +  99, IMP_IDENT_WIDTH, IMP_HOME_7,  FONT10ARIAL, 142,        true,  CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(IMP_RIGHT_IDENT_TEXT_X, STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y +  99, IMP_IDENT_WIDTH, IMP_HOME_8,  FONT10ARIAL, 142,        true,  CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(STD_SCREEN_X + 258,     STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y + 362, 640,             IMP_HOME_5,  FONT14ARIAL, FONT_BLACK, false, 0);
			LoadAndDisplayIMPText(IMP_LEFT_IDENT_TEXT_X,  STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y + 188, IMP_IDENT_WIDTH, IMP_HOME_9,  FONT10ARIAL, 142,        true,  RIGHT_JUSTIFIED);
			LoadAndDisplayIMPText(IMP_RIGHT_IDENT_TEXT_X, STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y + 188, IMP_IDENT_WIDTH, IMP_HOME_10, FONT10ARIAL, 142,        true,  RIGHT_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,     STD_SCREEN_Y + LAPTOP_SCREEN_WEB_DELTA_Y + 402, sWidth,          IMP_HOME_6,  FONT12ARIAL, FONT_WHITE, true,  CENTER_JUSTIFIED);
			break;

		case IMP_ABOUT_US:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  17, LAPTOP_SCREEN_WEB_UL_Y + 137, 640      , IMP_ABOUT_US_1 , FONT12ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  25, LAPTOP_SCREEN_WEB_UL_Y + 154, 337 - 124, IMP_ABOUT_US_2,  FONT10ARIAL, 142,        true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  25, LAPTOP_SCREEN_WEB_UL_Y + 235, 337 - 124, IMP_ABOUT_US_3,  FONT10ARIAL, 142,        true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  17, LAPTOP_SCREEN_WEB_UL_Y + 260, 640      , IMP_ABOUT_US_10, FONT12ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  25, LAPTOP_SCREEN_WEB_UL_Y + 280, 337 - 124, IMP_ABOUT_US_4,  FONT10ARIAL, 142,        true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 267, LAPTOP_SCREEN_WEB_UL_Y + 137, 640      , IMP_ABOUT_US_11, FONT12ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 275, LAPTOP_SCREEN_WEB_UL_Y + 154, 337 - 129, IMP_ABOUT_US_5,  FONT10ARIAL, 142,        true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 267, LAPTOP_SCREEN_WEB_UL_Y + 227, 640      , IMP_ABOUT_US_8,  FONT12ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 275, LAPTOP_SCREEN_WEB_UL_Y + 247, 337 - 129, IMP_ABOUT_US_6,  FONT10ARIAL, 142,        true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 267, LAPTOP_SCREEN_WEB_UL_Y + 277, 640      , IMP_ABOUT_US_9,  FONT12ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 275, LAPTOP_SCREEN_WEB_UL_Y + 297, 337 - 129, IMP_ABOUT_US_7,  FONT10ARIAL, 142,        true, 0);
			break;

		case IMP_MAIN_PAGE:
		{
			// title
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 19, sWidth, IMP_MAIN_1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);

			// set up for IMP text for title box area
			UINT16 const x = LAPTOP_SCREEN_UL_X + 173;
			UINT16 const y = LAPTOP_SCREEN_WEB_UL_Y + 91;
			UINT16 const w = 329 - 173;
			switch (iCurrentProfileMode)
			{
				case 0: LoadAndDisplayIMPText(x, y, w, IMP_MAIN_2, FONT10ARIAL, 142, true, 0); break;
				case 1: DisplayWrappedString( x, y, w, 2, FONT10ARIAL, 142, pExtraIMPStrings[0], 0, 0); break;
				case 2: DisplayWrappedString( x, y, w, 2, FONT10ARIAL, 142, pExtraIMPStrings[1], 0, 0); break;
				case 3: DisplayWrappedString( x, y, w, 2, FONT10ARIAL, 142, pExtraIMPStrings[2], 0, 0); break;
				case 4: DisplayWrappedString( x, y, w, 2, FONT10ARIAL, 142, pExtraIMPStrings[3], 0, 0); break;
			}
			break;
		}

		case IMP_BEGIN:

			// title
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +   7, sWidth,    IMP_BEGIN_1,  FONT14ARIAL, FONT_WHITE, true,  CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 105, LAPTOP_SCREEN_WEB_UL_Y +  67, 390 - 105, IMP_BEGIN_2,  FONT10ARIAL, 142,        true,  CENTER_JUSTIFIED);

			// fullname
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  81, LAPTOP_SCREEN_WEB_UL_Y + 139, 640,       IMP_BEGIN_3,  FONT14ARIAL, FONT_BLACK, false, 0);

			// nick name
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  81, LAPTOP_SCREEN_WEB_UL_Y + 199, 640,       IMP_BEGIN_4,  FONT14ARIAL, FONT_BLACK, false, 0);

			// gender
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  81, LAPTOP_SCREEN_WEB_UL_Y + 259, 640,       IMP_BEGIN_6,  FONT14ARIAL, FONT_BLACK, false, 0);

			// male
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 240, LAPTOP_SCREEN_WEB_UL_Y + 259, 640,       IMP_BEGIN_10, FONT14ARIAL, FONT_BLACK, false, 0);

			// female
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 360, LAPTOP_SCREEN_WEB_UL_Y + 259, 640,       IMP_BEGIN_11, FONT14ARIAL, FONT_BLACK, false, 0);
			break;

		case IMP_PERSONALITY:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 130, LAPTOP_SCREEN_WEB_UL_Y +  60, 456 - 200, IMP_PERS_1, FONT12ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 130, LAPTOP_SCREEN_WEB_UL_Y + 130, 456 - 200, IMP_PERS_2, FONT12ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +   7, sWidth,    IMP_PERS_6, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			break;

		case IMP_PERSONALITY_QUIZ:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +   5, sWidth,    IMP_PERS_6,  FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 293, LAPTOP_SCREEN_WEB_UL_Y + 370, 456 - 200, IMP_PERS_11, FONT12ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 363, LAPTOP_SCREEN_WEB_UL_Y + 370, 456 - 200, IMP_PERS_12, FONT12ARIAL, FONT_WHITE, true, 0);

			// print the question and suitable answers
			PrintIMPPersonalityQuizQuestionAndAnswers();
			break;

		case IMP_PERSONALITY_FINISH:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_PERS_6, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			switch( bPersonalityEndState )
			{
				case 0: LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, 356 - 100, IMP_PERS_F1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED); break;
				case 1: LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, 356 - 100, IMP_PERS_F4, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED); break;
				case 2: LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, 356 - 100, IMP_PERS_F5, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED); break;
			}
			break;

		case IMP_ATTRIBUTE_ENTRANCE:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +   7, sWidth, IMP_ATTRIB_1 - 1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 110, LAPTOP_SCREEN_WEB_UL_Y +  50, 300,    IMP_ATTRIB_5,     FONT12ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 110, LAPTOP_SCREEN_WEB_UL_Y + 130, 300,    IMP_ATTRIB_6,     FONT12ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 110, LAPTOP_SCREEN_WEB_UL_Y + 200, 300,    IMP_ATTRIB_7,     FONT12ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			break;

		case IMP_ATTRIBUTE_PAGE:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y + 7, sWidth, IMP_ATTRIB_1 - 1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);

			// don't blit bonus if reviewing
			if (!fReviewStats)
			{
				LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 355, LAPTOP_SCREEN_WEB_UL_Y + 51, 640, IMP_ATTRIB_SA_2 - 1, FONT12ARIAL, FONT_WHITE, true, 0);
				LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X +  56, LAPTOP_SCREEN_WEB_UL_Y + 33, 240, IMP_ATTRIB_SA_15,    FONT10ARIAL, FONT_WHITE, true, 0);
			}
			else
			{
				LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 56, LAPTOP_SCREEN_WEB_UL_Y + 33, 240, IMP_ATTRIB_SA_18, FONT10ARIAL, FONT_WHITE, true, 0);
			}
			// stats
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 0 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_6  - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // health
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 1 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_8  - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // dex
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 2 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_7  - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // agili
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 3 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_9  - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // str
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 4 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_11 - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // wisdom
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 5 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_10 - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // lead
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 6 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_12 - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // marks
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 7 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_14 - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // med
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 8 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_15 - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // expl
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 60, LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + 9 * SKILL_SLIDE_HEIGHT, 100, IMP_ATTRIB_SA_13 - 1, FONT12ARIAL, FONT_WHITE, true, RIGHT_JUSTIFIED); // mech

			// should we display zero warning or nowmal ' come on herc..' text
			break;

		case IMP_ATTRIBUTE_FINISH:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +   7, sWidth,    IMP_ATTRIB_1 - 1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 125, LAPTOP_SCREEN_WEB_UL_Y + 100, 356 - 100, IMP_AF_2     - 1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			break;

		case IMP_PORTRAIT:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +  7, sWidth, IMP_POR_1 - 1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 135, LAPTOP_SCREEN_WEB_UL_Y + 68, 240,    IMP_POR_2 - 1, FONT10ARIAL, 142,        true, 0);
			break;

		case IMP_VOICE:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +  7, sWidth, IMP_VOC_1 - 1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 135, LAPTOP_SCREEN_WEB_UL_Y + 70, 240,    IMP_VOC_2 - 1, FONT10ARIAL, 142,        true, 0);
			break;

		case IMP_FINISH:
			//LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 160, LAPTOP_SCREEN_WEB_UL_Y +  7, 640, IMP_FIN_1 - 1, FONT14ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 150, LAPTOP_SCREEN_WEB_UL_Y + 55, 200, IMP_FIN_2 - 1, FONT12ARIAL, FONT_WHITE, true, 0);
			break;

		case IMP_CONFIRM:
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X,       LAPTOP_SCREEN_WEB_UL_Y +   7, sWidth, IMP_CON_1, FONT14ARIAL, FONT_WHITE, true, CENTER_JUSTIFIED);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 160, LAPTOP_SCREEN_WEB_UL_Y +  60, 200,    IMP_CON_2, FONT12ARIAL, FONT_WHITE, true, 0);
			LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 160, LAPTOP_SCREEN_WEB_UL_Y + 145, 200,    IMP_CON_3, FONT12ARIAL, FONT_WHITE, true, 0);
			break;
	}
}


static void OffSetQuestionForFemaleSpecificQuestions(INT32* iCurrentOffset);


static void PrintIMPPersonalityQuizQuestionAndAnswers(void)
{
	if (giCurrentPersonalityQuizQuestion < 0)
	{
		return;
	}

	INT32 iOffset = 0;

	// how far into text is the question?
	for (INT32 iCounter = 0; iCounter < giCurrentPersonalityQuizQuestion; iCounter++)
	{
		// incrment until question is found
		iOffset += iIMPQuestionLengths[iCounter];
	}

	// handle any female specifc questions
	if (!fCharacterIsMale)
	{
		OffSetQuestionForFemaleSpecificQuestions(&iOffset);
	}

	// how many answers are there?
	INT32 n = iIMPQuestionLengths[giCurrentPersonalityQuizQuestion];

	// question is at IMP_QUESTION_1 + iOffset and there are n - 1 answers afterwards
	BltAnswerIndents(n - 1);
	LoadAndDisplayIMPText(LAPTOP_SCREEN_UL_X + 20, LAPTOP_SCREEN_WEB_UL_Y + 30, 460, IMP_QUESTION_1 + iOffset, FONT10ARIAL, FONT_WHITE, true, LEFT_JUSTIFIED);

	// use extra wide columns for 4 or less answers
	INT16 w = (n - 1 <= 4 ? 390 : 160);

	// answers
	for (INT32 i = 0; i < n - 1; ++i)
	{
		INT16 x = LAPTOP_SCREEN_UL_X + (i < 4 ? QTN_FIRST_COLUMN_X : QTN_SECOND_COLUMN_X);
		INT16 y = LAPTOP_SCREEN_WEB_UL_Y + 100 + 50 * (i % 4);
		LoadAndDisplayIMPText(x, y, w, IMP_QUESTION_1 + iOffset + 1 + i, FONT10ARIAL, 142, true, LEFT_JUSTIFIED);
	}
}


static void OffSetQuestionForFemaleSpecificQuestions(INT32* iCurrentOffset)
{
	// find the extra
	INT32 iExtraOffSet = 0;
	switch (giCurrentPersonalityQuizQuestion)
	{
		case 13: iExtraOffSet += iIMPQuestionLengths[9]; /* FALLTHROUGH */
		case  9: iExtraOffSet += iIMPQuestionLengths[8]; /* FALLTHROUGH */
		case  8: iExtraOffSet += iIMPQuestionLengths[3]; /* FALLTHROUGH */
		case  3: iExtraOffSet += iIMPQuestionLengths[0]; /* FALLTHROUGH */
		case  0: break;

		default: return;
	}

	*iCurrentOffset = IMP_CON_3 - IMP_QUESTION_1 + 3 + iExtraOffSet;
}
