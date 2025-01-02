
#include "CharProfile.h"
#include "Cursors.h"
#include "Directories.h"
#include "Font.h"
#include "IMP_Personality_Quiz.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "Laptop.h"
#include "IMP_Compile_Character.h"
#include "Soldier_Profile_Type.h"
#include "IMP_Text_System.h"
#include "Input.h"
#include "Button_System.h"
#include "Font_Control.h"

#include <string_theory/format>
#include <string_theory/string>

#include <algorithm>

static BUTTON_PICS* giIMPPersonalityQuizButtonImage[2];
static GUIButtonRef giIMPPersonalityQuizButton[2];


// these are the buttons for the current question
static BUTTON_PICS* giIMPPersonalityQuizAnswerButtonImage[8];
static GUIButtonRef giIMPPersonalityQuizAnswerButton[8];

static GUIButtonRef giPreviousQuestionButton;
static GUIButtonRef giNextQuestionButton;

static BUTTON_PICS* giPreviousQuestionButtonImage;
static BUTTON_PICS* giNextQuestionButtonImage;

// this the currently highlighted answer
INT32 iCurrentAnswer = -1;

// the current quiz question
INT32 giCurrentPersonalityQuizQuestion = 0;
static INT32 giPreviousPersonalityQuizQuestion = -1;
INT32 giMaxPersonalityQuizQuestion = 0;

// start over flag
BOOLEAN fStartOverFlag = FALSE;


#define BTN_FIRST_COLUMN_X 15
#define BTN_SECOND_COLUMN_X 256

#define INDENT_OFFSET 55

// number of IMP questions
#define MAX_NUMBER_OF_IMP_QUESTIONS 16

// answer list
INT32 iQuizAnswerList[MAX_NUMBER_OF_IMP_QUESTIONS];
// current number of buttons being shown
INT32 iNumberOfPersonaButtons = 0;


static void CreateIMPPersonalityQuizAnswerButtons(void);
static void CreateIMPPersonalityQuizButtons(void);
static void PrintQuizQuestionNumber(void);
static void ResetQuizAnswerButtons(void);


void EnterIMPPersonalityQuiz( void )
{

	// void answers out the quiz
	std::fill_n(iQuizAnswerList, MAX_NUMBER_OF_IMP_QUESTIONS, -1);

	// if we are entering for first time, reset
	if( giCurrentPersonalityQuizQuestion == MAX_NUMBER_OF_IMP_QUESTIONS )
	{
		giCurrentPersonalityQuizQuestion = 0;
	}
	// reset previous
	giPreviousPersonalityQuizQuestion = -1;

	// reset skills, attributes and personality
	ResetSkillsAttributesAndPersonality( );

	// create/destroy buttons for  questions, if needed
	CreateIMPPersonalityQuizAnswerButtons( );

	// now reset them
	ResetQuizAnswerButtons( );

	// create other buttons
	CreateIMPPersonalityQuizButtons( );
}

void RenderIMPPersonalityQuiz( void )
{
	// the background
	RenderProfileBackGround( );

	// highlight answer
	PrintImpText( );

	// indent for current and last page numbers
	//RenderAttrib2IndentFrame(BTN_FIRST_COLUMN_X + 2, 365 );

	// the current and last question numbers
	PrintQuizQuestionNumber( );
}


static void DestroyIMPersonalityQuizButtons(void);
static void DestroyPersonalityQuizButtons(void);


void ExitIMPPersonalityQuiz( void )
{
	// set previous to current, we want it's buttons gone!
	giPreviousPersonalityQuizQuestion = giCurrentPersonalityQuizQuestion;


	// destroy regular quiz buttons: the done and start over buttons
	DestroyIMPersonalityQuizButtons( );

	// destroy the buttons used for answers
	DestroyPersonalityQuizButtons( );

	if( fStartOverFlag )
	{
		fStartOverFlag = FALSE;
		giCurrentPersonalityQuizQuestion = 0;
	}
}


static void HandleIMPQuizKeyBoard(void);


void HandleIMPPersonalityQuiz( void )
{
	// create/destroy buttons for  questions, if needed
	CreateIMPPersonalityQuizAnswerButtons( );

	// handle keyboard input
	HandleIMPQuizKeyBoard( );

	if( iCurrentAnswer == -1)
	{
		DisableButton(	giIMPPersonalityQuizButton[0] );
	}
}


static GUIButtonRef MakeButton(BUTTON_PICS* const img, const ST::string& text, const INT16 x, const INT16 y, GUI_CALLBACK click)
{
	const INT16 text_col   = FONT_WHITE;
	const INT16 shadow_col = DEFAULT_SHADOW;
	GUIButtonRef const btn = CreateIconAndTextButton(img, text, FONT12ARIAL,
		text_col, shadow_col, text_col, shadow_col, x, y,
		MSYS_PRIORITY_HIGH, std::move(click));
	btn->SetCursor(CURSOR_WWW);
	return btn;
}


static void BtnIMPPersonalityQuizAnswerConfirmCallback(GUI_BUTTON *btn, UINT32 reason);
static void BtnIMPPersonalityQuizStartOverCallback(GUI_BUTTON *btn, UINT32 reason);
static void PreviousQuestionButtonCallback(GUI_BUTTON *btn, UINT32 iReason);
static void NextQuestionButtonCallback(GUI_BUTTON *btn, UINT32 iReason);


static void CreateIMPPersonalityQuizButtons(void)
{
	// this function will create the buttons needed for the IMP personality quiz Page
	const INT16 dx = LAPTOP_SCREEN_UL_X;
	const INT16 dy = LAPTOP_SCREEN_WEB_UL_Y;

	// ths Done button
	giIMPPersonalityQuizButtonImage[0] = LoadButtonImage(LAPTOPDIR "/button_7.sti", 0, 1);
	giIMPPersonalityQuizButton[0] = MakeButton(giIMPPersonalityQuizButtonImage[0], pImpButtonText[8], dx + 197, dy + 302, BtnIMPPersonalityQuizAnswerConfirmCallback);

	// start over
	giIMPPersonalityQuizButtonImage[1] = LoadButtonImage(LAPTOPDIR "/button_5.sti", 0, 1);
	giIMPPersonalityQuizButton[1] = MakeButton(giIMPPersonalityQuizButtonImage[1], pImpButtonText[7], dx + BTN_FIRST_COLUMN_X, dy + 302, BtnIMPPersonalityQuizStartOverCallback);

	giPreviousQuestionButtonImage = LoadButtonImage(LAPTOPDIR "/button_3.sti", 0, 1);
	giPreviousQuestionButton = MakeButton(giPreviousQuestionButtonImage, pImpButtonText[12], dx + 197, dy + 361, PreviousQuestionButtonCallback);

	giNextQuestionButtonImage = LoadButtonImage(LAPTOPDIR "/button_3.sti", 0, 1);
	giNextQuestionButton = MakeButton(giNextQuestionButtonImage, pImpButtonText[13], dx + 417, dy + 361, NextQuestionButtonCallback);

	giNextQuestionButton->SpecifyTextSubOffsets(    0, -1, FALSE);
	giPreviousQuestionButton->SpecifyTextSubOffsets(0, -1, FALSE);

	DisableButton( giPreviousQuestionButton );
	DisableButton( giNextQuestionButton );
}


static void DestroyIMPersonalityQuizButtons(void)
{
	// this function will destroy the buttons needed for the IMP personality quiz page

	// the done button
	RemoveButton(giIMPPersonalityQuizButton[ 0 ] );
	UnloadButtonImage(giIMPPersonalityQuizButtonImage[ 0 ] );

	// the start over button
	RemoveButton(giIMPPersonalityQuizButton[ 1 ] );
	UnloadButtonImage(giIMPPersonalityQuizButtonImage[ 1 ] );

	// previosu button
	RemoveButton( giPreviousQuestionButton );
	UnloadButtonImage( giPreviousQuestionButtonImage );

	// next button
	RemoveButton( giNextQuestionButton );
	UnloadButtonImage( giNextQuestionButtonImage );
}


static void AddIMPPersonalityQuizAnswerButtons(INT32 iNumberOfButtons);
static void ToggleQuestionNumberButtonOn(INT32 iAnswerNumber);


static void CreateIMPPersonalityQuizAnswerButtons(void)
{
	// this function will create the buttons for the personality quiz answer selections

	if( IMP_PERSONALITY_QUIZ != iCurrentImpPage )
	{
		// not valid pagre, get out
		return;
	}


	if( giCurrentPersonalityQuizQuestion == giPreviousPersonalityQuizQuestion )
	{
		// mode has not changed, return;
		return;
	}

	// destroy old screens buttons
	DestroyPersonalityQuizButtons( );

	// re-render screen
	RenderProfileBackGround( );

	switch( giCurrentPersonalityQuizQuestion )
	{
		case -1: break; // do nothing
		case  0: iNumberOfPersonaButtons = 6; break;
		case  3: iNumberOfPersonaButtons = 5; break;
		case  5: iNumberOfPersonaButtons = 5; break;
		case 10: iNumberOfPersonaButtons = 5; break;
		case 11: iNumberOfPersonaButtons = 8; break;
		default: iNumberOfPersonaButtons = 4; break;
	}

	AddIMPPersonalityQuizAnswerButtons( iNumberOfPersonaButtons );

	ToggleQuestionNumberButtonOn( iQuizAnswerList[ giCurrentPersonalityQuizQuestion ] );

	// re render text
	PrintImpText( );

	// the current and last question numbers
	PrintQuizQuestionNumber( );

	// title bar
	RenderWWWProgramTitleBar( );
}


static void DestroyIMPPersonalityQuizAnswerButtons(INT32 iNumberOfButtons);


static void DestroyPersonalityQuizButtons(void)
{

	// this function will destroy the buttons used in the previous personality question
	// destroy old buttons
	UINT32 ButtonCount;
	switch( giPreviousPersonalityQuizQuestion  )
	{
		case -1: return; // do nothing
		case  0: ButtonCount = 6; break;
		case  3: ButtonCount = 5; break;
		case  5: ButtonCount = 5; break;
		case 10: ButtonCount = 5; break;
		case 11: ButtonCount = 8; break;
		default: ButtonCount = 4; break;
	}
	DestroyIMPPersonalityQuizAnswerButtons(ButtonCount);
}


static void BtnQuizAnswerCallback(GUI_BUTTON*, UINT32 reason);


static void AddIMPPersonalityQuizAnswerButtons(INT32 iNumberOfButtons)
{
	// will add iNumberofbuttons to the answer button list
	for (INT32 i = 0; i < iNumberOfButtons; i++)
	{
		INT32 XLoc = LAPTOP_SCREEN_UL_X + (i < 4 ? BTN_FIRST_COLUMN_X : BTN_SECOND_COLUMN_X);
		INT32 YLoc = LAPTOP_SCREEN_WEB_UL_Y + 97 + i % 4 * 50;
		BUTTON_PICS* const Image = LoadButtonImage(LAPTOPDIR "/button_6.sti", 0, 1);
		giIMPPersonalityQuizAnswerButtonImage[i] = Image;
		GUIButtonRef const Button = QuickCreateButtonNoMove(Image, XLoc, YLoc, MSYS_PRIORITY_HIGHEST - 3, BtnQuizAnswerCallback);
		giIMPPersonalityQuizAnswerButton[i] = Button;
		Button->SetUserData(i);
		Button->SpecifyTextOffsets(23, 12, TRUE);
		ST::string sString = ST::format("{}", i + 1);
		Button->SpecifyGeneralTextAttributes(sString, FONT12ARIAL, FONT_WHITE, FONT_BLACK);
		Button->SetCursor(CURSOR_WWW);
	}

	// previous is current
	giPreviousPersonalityQuizQuestion = giCurrentPersonalityQuizQuestion;
}


static void DestroyIMPPersonalityQuizAnswerButtons(INT32 iNumberOfButtons)
{
	INT32 iCounter = 0;
	for(iCounter = 0; iCounter < iNumberOfButtons; iCounter++)
	{
		RemoveButton(giIMPPersonalityQuizAnswerButton[ iCounter ] );
		UnloadButtonImage(giIMPPersonalityQuizAnswerButtonImage[ iCounter ] );
	}
}


static void CheckStateOfTheConfirmButton(void);


static void BtnQuizAnswerCallback(GUI_BUTTON* const btn, UINT32 const reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_DWN)
	{
		ResetQuizAnswerButtons();
		btn->uiFlags |= BUTTON_CLICKED_ON;
		CheckStateOfTheConfirmButton();
		iCurrentAnswer = btn->GetUserData();
		PrintImpText();
		PrintQuizQuestionNumber();
		fReDrawCharProfile = TRUE;
	}
}


static void CheckAndUpdateNextPreviousIMPQuestionButtonStates(void);
static void CompileQuestionsInStatsAndWhatNot(void);


static void BtnIMPPersonalityQuizAnswerConfirmCallback(GUI_BUTTON *, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		if (iCurrentAnswer != -1)
		{
			ResetQuizAnswerButtons( );
			iQuizAnswerList[giCurrentPersonalityQuizQuestion] = iCurrentAnswer;
			iCurrentAnswer = -1;

			// next question, JOHNNY!
			if (giCurrentPersonalityQuizQuestion == giMaxPersonalityQuizQuestion)
			{
				giMaxPersonalityQuizQuestion++;
			}

			giCurrentPersonalityQuizQuestion++;
			CheckAndUpdateNextPreviousIMPQuestionButtonStates();

			// OPPS!, done..time to finish up
			if (giCurrentPersonalityQuizQuestion > 15)
			{
				iCurrentImpPage = IMP_PERSONALITY_FINISH;
				CompileQuestionsInStatsAndWhatNot();
			}
		}
	}
}


static void BtnIMPPersonalityQuizStartOverCallback(GUI_BUTTON *, UINT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		giPreviousPersonalityQuizQuestion = giCurrentPersonalityQuizQuestion;
		giMaxPersonalityQuizQuestion = 0;
		fStartOverFlag = TRUE;

		iCurrentImpPage = IMP_PERSONALITY;
		fButtonPendingFlag = TRUE;
		iCurrentAnswer = -1;
	}
}


static void ResetQuizAnswerButtons(void)
{
	// how many buttons to reset?
	INT32 iCounter;
	switch (giCurrentPersonalityQuizQuestion)
	{
		case -1: return; // do nothing
		case  0: iCounter = 6; break;
		case  3: iCounter = 5; break;
		case  5: iCounter = 5; break;
		case 10: iCounter = 5; break;
		case 11: iCounter = 8; break;
		default: iCounter = 4; break;
	}

	// now run through and reset the buttons
	for (INT32 i = 0; i < iCounter; i++)
	{
		giIMPPersonalityQuizAnswerButton[i]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


static void CompileQuestionsInStatsAndWhatNot(void)
{
	switch (iQuizAnswerList[0])
	{
		case 0: AddSkillToSkillList(fCharacterIsMale ? MARTIALARTS : AMBIDEXT); break;
		case 1: AddAnAttitudeToAttitudeList(ATT_LONER);                         break;
		case 2: AddSkillToSkillList(HANDTOHAND);                                break;
		case 3: AddSkillToSkillList(LOCKPICKING);                               break;
		case 4: AddSkillToSkillList(THROWING);                                  break;
		case 5: AddAnAttitudeToAttitudeList(ATT_OPTIMIST);                      break;
	}

	switch (iQuizAnswerList[1])
	{
		case 0: AddSkillToSkillList(TEACHING);             break;
		case 1: AddSkillToSkillList(STEALTHY);             break;
		case 2: AddAPersonalityToPersonalityList(PSYCHO);  break;
		case 3: AddAnAttitudeToAttitudeList(ATT_FRIENDLY); break;
	}

	switch (iQuizAnswerList[2])
	{
		case 0: AddSkillToSkillList(LOCKPICKING);          break;
		case 1: AddAnAttitudeToAttitudeList(ATT_ARROGANT); break;
		case 2: AddSkillToSkillList(STEALTHY);             break;
		case 3: AddAnAttitudeToAttitudeList(ATT_NORMAL);   break;
	}

	switch (iQuizAnswerList[3])
	{
		case 0: AddSkillToSkillList(AUTO_WEAPS);           break;
		case 1: AddAnAttitudeToAttitudeList(ATT_FRIENDLY); break;
		case 2: AddAnAttitudeToAttitudeList(ATT_NORMAL);   break;
		case 3: AddAnAttitudeToAttitudeList(ATT_ASSHOLE);  break;
		case 4: AddAnAttitudeToAttitudeList(ATT_LONER);    break;
	}

	switch (iQuizAnswerList[4])
	{
		case 0: AddAnAttitudeToAttitudeList(ATT_COWARD);     break;
		case 1: break; // none
		case 2: AddAnAttitudeToAttitudeList(ATT_AGGRESSIVE); break;
		case 3: break; // none
	}

	switch (iQuizAnswerList[5])
	{
		case 0: AddAnAttitudeToAttitudeList(ATT_COWARD);          break;
		case 1: AddSkillToSkillList(NIGHTOPS);                    break;
		case 2: AddAPersonalityToPersonalityList(CLAUSTROPHOBIC); break;
		case 3: break; // none
		case 4: break; // none
	}

	switch (iQuizAnswerList[6])
	{
		case 0: AddSkillToSkillList(ELECTRONICS); break;
		case 1: AddSkillToSkillList(KNIFING);     break;
		case 2: AddSkillToSkillList(NIGHTOPS);    break;
		case 3: break; // none
	}

	switch (iQuizAnswerList[7])
	{
		case 0: AddSkillToSkillList(AMBIDEXT);             break;
		case 1: break; // none
		case 2: AddAnAttitudeToAttitudeList(ATT_OPTIMIST); break;
		case 3: AddAPersonalityToPersonalityList(PSYCHO);  break;
	}

	switch (iQuizAnswerList[8])
	{
		case 0: AddAPersonalityToPersonalityList(FORGETFUL); break;
		case 1: break; // none
		case 2: AddAnAttitudeToAttitudeList(ATT_PESSIMIST);  break;
		case 3: AddAPersonalityToPersonalityList(NERVOUS);   break;
	}

	switch (iQuizAnswerList[9])
	{
		case 0: break; // none
		case 1: AddAnAttitudeToAttitudeList(ATT_PESSIMIST); break;
		case 2: AddAnAttitudeToAttitudeList(ATT_ASSHOLE);   break;
		case 3: AddAPersonalityToPersonalityList(NERVOUS);  break;
	}

	switch (iQuizAnswerList[10])
	{
		case 0: break; // none
		case 1: AddSkillToSkillList(TEACHING);               break;
		case 2: AddAnAttitudeToAttitudeList(ATT_AGGRESSIVE); break;
		case 3: AddAnAttitudeToAttitudeList(ATT_NORMAL);     break;
		case 4: break; // none
	}

	switch (iQuizAnswerList[11])
	{
		case 0: AddSkillToSkillList(fCharacterIsMale ? MARTIALARTS : AMBIDEXT); break;
		case 1: AddSkillToSkillList(KNIFING);                                   break;
		case 2: break; // none
		case 3: AddSkillToSkillList(AUTO_WEAPS);                                break;
		case 4: AddSkillToSkillList(HANDTOHAND);                                break;
		case 5: AddSkillToSkillList(ELECTRONICS);                               break;
		case 6: break; // asshole...not adding another ATT_ASSHOLE though as 2 are enough
		case 7: break; // none
	}

	switch (iQuizAnswerList[12])
	{
		case 0: AddAPersonalityToPersonalityList(FORGETFUL);       break;
		case 1: AddAnAttitudeToAttitudeList(ATT_NORMAL);           break;
		case 2: AddAnAttitudeToAttitudeList(ATT_NORMAL);           break;
		case 3: AddAPersonalityToPersonalityList(HEAT_INTOLERANT); break;
	}

	switch (iQuizAnswerList[13])
	{
		case 0: AddAPersonalityToPersonalityList(CLAUSTROPHOBIC);  break;
		case 1: AddAnAttitudeToAttitudeList(ATT_NORMAL);           break;
		case 2: AddAPersonalityToPersonalityList(HEAT_INTOLERANT); break;
		case 3: break; // none
	}

	switch (iQuizAnswerList[14])
	{
		case 0: AddSkillToSkillList(THROWING);             break;
		case 1: AddSkillToSkillList(AMBIDEXT);             break;
		case 2: AddAnAttitudeToAttitudeList(ATT_ARROGANT); break;
		case 3: break; // none
	}

	switch (iQuizAnswerList[15])
	{
		case 0: break; // none !
		case 1: break; // none !
		case 2: break; // none !
		case 3: break; // none !
	}
}


void BltAnswerIndents( INT32 iNumberOfIndents )
{
	INT32 iCounter = 0;


	// the question indent
	RenderQtnIndentFrame( 15, 20 );


	// the answers

	for( iCounter = 0; iCounter < iNumberOfIndents; iCounter++)
	{
		switch( iCounter )
		{
			case( 0 ):
				if( iNumberOfIndents < 5 )
				{
					RenderQtnLongIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 93);

					if( iCurrentAnswer == iCounter )
					{
						RenderQtnLongIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 93 );
					}
				}
				else
				{
					RenderQtnShortIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 93);

					if( iCurrentAnswer == iCounter )
					{
						RenderQtnShortIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 93 );
					}
				}
				break;
			case( 1 ):
				if( iNumberOfIndents < 5 )
				{
					RenderQtnLongIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 143);

					if( iCurrentAnswer == iCounter )
					{
						RenderQtnLongIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 143 );
					}
				}
				else
				{
					RenderQtnShortIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 143);

					if( iCurrentAnswer == iCounter )
					{
						RenderQtnShortIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 143 );
					}
				}
				break;
			case( 2 ):
				if( iNumberOfIndents < 5 )
				{
					RenderQtnLongIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 193);

					if( iCurrentAnswer == iCounter )
					{
					RenderQtnLongIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 193 );
					}
				}
				else
				{
					RenderQtnShortIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 193);

					if( iCurrentAnswer == iCounter )
					{
					RenderQtnShortIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 193 );
					}
				}
				break;
			case( 3 ):
				// is this question # 6 ..if so, need longer answer box
				if( giCurrentPersonalityQuizQuestion == 5 )
				{
					// render longer frame
					RenderQtnShort2IndentFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 243 );

					// is this answer currently selected?
					if( iCurrentAnswer == iCounter )
					{
						// need to highlight
						RenderQtnShort2IndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 243 );
					}
					// done
					break;
				}

				if( iNumberOfIndents < 5 )
				{
					RenderQtnLongIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 243);

					if( iCurrentAnswer == iCounter )
					{
						RenderQtnLongIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 243 );
					}
				}
				else
				{
					RenderQtnShortIndentFrame(BTN_FIRST_COLUMN_X + INDENT_OFFSET, 243);

					if( iCurrentAnswer == iCounter )
					{
						RenderQtnShortIndentHighFrame( BTN_FIRST_COLUMN_X + INDENT_OFFSET, 243 );
					}
				}
				break;
			case( 4 ):
				//is this question # 14 or 21?..if so, need longer answer box
				if( ( giCurrentPersonalityQuizQuestion == 10)||( giCurrentPersonalityQuizQuestion == 5 ) )
				{
					// render longer frame
					RenderQtnShort2IndentFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 93 );

					// is this answer currently selected?
					if( iCurrentAnswer == iCounter )
					{
						// need to highlight
						RenderQtnShort2IndentHighFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 93 );
					}
					// done
					break;
				}

				RenderQtnShortIndentFrame(BTN_SECOND_COLUMN_X + INDENT_OFFSET, 93);

				if( iCurrentAnswer == iCounter )
				{
					RenderQtnShortIndentHighFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 93 );
				}
				break;
			case( 5 ):
				// special case?..longer frame needed if so
				if(  giCurrentPersonalityQuizQuestion == 19 )
				{
					// render longer frame
					RenderQtnShort2IndentFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 143 );

					// is this answer currently selected?
					if( iCurrentAnswer == iCounter )
					{
						// need to highlight
						RenderQtnShort2IndentHighFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 143 );
					}
					// done
					break;
				}
				RenderQtnShortIndentFrame(BTN_SECOND_COLUMN_X + INDENT_OFFSET, 143);
				if( iCurrentAnswer == iCounter )
				{
					RenderQtnShortIndentHighFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 143 );
				}
				break;
			case( 6 ):
				RenderQtnShortIndentFrame(BTN_SECOND_COLUMN_X + INDENT_OFFSET, 193);
				if( iCurrentAnswer == iCounter )
				{
					RenderQtnShortIndentHighFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 193 );
				}
				break;
			case( 7 ):
				RenderQtnShortIndentFrame(BTN_SECOND_COLUMN_X + INDENT_OFFSET, 243);
				if( iCurrentAnswer == iCounter )
				{
					RenderQtnShortIndentHighFrame( BTN_SECOND_COLUMN_X + INDENT_OFFSET, 243 );
				}
				break;
			case( 8 ):
				break;
		}
	}
}


static void PrintQuizQuestionNumber(void)
{
	// this function will print the number of the current question and the numebr of questions

	SetFontAttributes(FONT12ARIAL, FONT_WHITE);

	// print current question number
	MPrint(LAPTOP_SCREEN_UL_X + 345, LAPTOP_SCREEN_WEB_UL_Y + 370, ST::format("{}", giCurrentPersonalityQuizQuestion + 1));

	// total number of questions
	MPrint(LAPTOP_SCREEN_UL_X + 383, LAPTOP_SCREEN_WEB_UL_Y + 370, "16");
}


static void CheckStateOfTheConfirmButton(void)
{
	// will check the state of the confirm button, should it be enabled or disabled?
	if( iCurrentAnswer == -1 )
	{
		// was disabled, enable
		EnableButton( giIMPPersonalityQuizButton[0] );
	}
}


static void MoveAheadAQuestion(void);
static void MoveBackAQuestion(void);


static void HandleIMPQuizKeyBoard(void)
{
	InputAtom InputEvent;

	while (DequeueSpecificEvent(&InputEvent, KEYBOARD_EVENTS))
	{
		HandleKeyBoardShortCutsForLapTop(InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState);
	}
}


static void CheckAndUpdateNextPreviousIMPQuestionButtonStates(void)
{
	EnableButton(giNextQuestionButton,     giCurrentPersonalityQuizQuestion < giMaxPersonalityQuizQuestion);
	EnableButton(giPreviousQuestionButton, giCurrentPersonalityQuizQuestion != 0);
}


static void MoveAheadAQuestion(void)
{

	// move ahead a question in the personality question list
	if( giCurrentPersonalityQuizQuestion < giMaxPersonalityQuizQuestion )
	{
		giCurrentPersonalityQuizQuestion++;

		iCurrentAnswer = -1;
		CheckStateOfTheConfirmButton( );

		iCurrentAnswer = iQuizAnswerList[ giCurrentPersonalityQuizQuestion ];
	}

	CheckAndUpdateNextPreviousIMPQuestionButtonStates( );
}


static void MoveBackAQuestion(void)
{
	if( giCurrentPersonalityQuizQuestion > 0 )
	{
		giCurrentPersonalityQuizQuestion--;

		iCurrentAnswer = -1;
		CheckStateOfTheConfirmButton( );

		iCurrentAnswer = iQuizAnswerList[ giCurrentPersonalityQuizQuestion ];
	}

	EnableButton( giNextQuestionButton );

	CheckAndUpdateNextPreviousIMPQuestionButtonStates( );
}


static void ToggleQuestionNumberButtonOn(INT32 iAnswerNumber)
{
	if( ( giCurrentPersonalityQuizQuestion <= giMaxPersonalityQuizQuestion ) && ( iAnswerNumber != -1 ) )
	{
		// reset buttons
		ResetQuizAnswerButtons( );

		// toggle this button on
		giIMPPersonalityQuizAnswerButton[iAnswerNumber]->uiFlags |= BUTTON_CLICKED_ON;
		iCurrentAnswer = iAnswerNumber;
	}
}


static void PreviousQuestionButtonCallback(GUI_BUTTON *btn, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		MoveBackAQuestion( );
	}
}


static void NextQuestionButtonCallback(GUI_BUTTON *btn, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		MoveAheadAQuestion( );
	}
}
