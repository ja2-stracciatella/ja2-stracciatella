#include "CharProfile.h"
#include "Font.h"
#include "IMP_Personality_Quiz.h"
#include "IMPVideoObjects.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Compile_Character.h"
#include "Soldier_Profile_Type.h"
#include "IMP_Text_System.h"
#include "Input.h"
#include "Button_System.h"
#include "Stubs.h" // XXX
#include "Font_Control.h"


static UINT32 giIMPPersonalityQuizButton[2];
static UINT32 giIMPPersonalityQuizButtonImage[2];


// these are the buttons for the current question
static INT32 giIMPPersonalityQuizAnswerButton[8];
static INT32 giIMPPersonalityQuizAnswerButtonImage[8];

static INT32 giPreviousQuestionButton;
static INT32 giNextQuestionButton;

static INT32 giPreviousQuestionButtonImage;
static INT32 giNextQuestionButtonImage;

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
  memset( &iQuizAnswerList, -1, sizeof( INT32 ) * MAX_NUMBER_OF_IMP_QUESTIONS );

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


static void BtnIMPPersonalityQuizAnswerConfirmCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizStartOverCallback(GUI_BUTTON *btn, INT32 reason);
static void PreviousQuestionButtonCallback(GUI_BUTTON *btn, INT32 iReason);
static void NextQuestionButtonCallback(GUI_BUTTON *btn, INT32 iReason);


static void CreateIMPPersonalityQuizButtons(void)
{
  // this function will create the buttons needed for the IMP personality quiz Page





	// ths Done button
  giIMPPersonalityQuizButtonImage[0]=  LoadButtonImage( "LAPTOP/button_7.sti" ,-1,0,-1,1,-1 );

/* giIMPPersonalityQuizButton[0] = QuickCreateButton( giIMPPersonalityQuizButtonImage[0], LAPTOP_SCREEN_UL_X +  ( 197 ), LAPTOP_SCREEN_WEB_UL_Y + ( 310 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPPersonalityQuizAnswerConfirmCallback);
*/
	giIMPPersonalityQuizButton[0] = CreateIconAndTextButton( giIMPPersonalityQuizButtonImage[0], pImpButtonText[ 8 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 197 ), LAPTOP_SCREEN_WEB_UL_Y + ( 302 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, BtnIMPPersonalityQuizAnswerConfirmCallback);

  // start over
	giIMPPersonalityQuizButtonImage[ 1 ]=  LoadButtonImage( "LAPTOP/button_5.sti" ,-1,0,-1,1,-1 );

	/* giIMPPersonalityQuizButton[ 1 ] = QuickCreateButton( giIMPPersonalityQuizButtonImage[1], LAPTOP_SCREEN_UL_X +  ( BTN_FIRST_COLUMN_X ), LAPTOP_SCREEN_WEB_UL_Y + ( 310 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPPersonalityQuizStartOverCallback);
*/

	giIMPPersonalityQuizButton[ 1 ] = CreateIconAndTextButton( giIMPPersonalityQuizButtonImage[ 1 ], pImpButtonText[ 7 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( BTN_FIRST_COLUMN_X ), LAPTOP_SCREEN_WEB_UL_Y + ( 302 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, BtnIMPPersonalityQuizStartOverCallback);

	giPreviousQuestionButtonImage = LoadButtonImage( "LAPTOP/button_3.sti" ,-1,0,-1,1,-1 );
	giPreviousQuestionButton = CreateIconAndTextButton( giPreviousQuestionButtonImage, pImpButtonText[ 12 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 197 ), LAPTOP_SCREEN_WEB_UL_Y + ( 361 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, PreviousQuestionButtonCallback);

	giNextQuestionButtonImage = LoadButtonImage( "LAPTOP/button_3.sti" ,-1,0,-1,1,-1 );
	giNextQuestionButton = CreateIconAndTextButton( giNextQuestionButtonImage, pImpButtonText[ 13 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 417 ), LAPTOP_SCREEN_WEB_UL_Y + ( 361 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, NextQuestionButtonCallback);


	SpecifyButtonTextSubOffsets( giNextQuestionButton, 0, -1, FALSE );
	SpecifyButtonTextSubOffsets( giPreviousQuestionButton, 0, -1, FALSE );

	DisableButton( giPreviousQuestionButton );
	DisableButton( giNextQuestionButton );

  SetButtonCursor( giIMPPersonalityQuizButton[0], CURSOR_WWW );
	SetButtonCursor( giIMPPersonalityQuizButton[1], CURSOR_WWW );
	SetButtonCursor( giPreviousQuestionButton, CURSOR_WWW );
	SetButtonCursor( giNextQuestionButton, CURSOR_WWW );
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


static void BtnIMPPersonalityQuizAnswer0Callback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizAnswer1Callback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizAnswer2Callback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizAnswer3Callback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizAnswer4Callback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizAnswer5Callback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizAnswer6Callback(GUI_BUTTON *btn, INT32 reason);
static void BtnIMPPersonalityQuizAnswer7Callback(GUI_BUTTON *btn, INT32 reason);


static void AddIMPPersonalityQuizAnswerButtons(INT32 iNumberOfButtons)
{
	static const GUI_CALLBACK Callback[] =
	{
		BtnIMPPersonalityQuizAnswer0Callback,
		BtnIMPPersonalityQuizAnswer1Callback,
		BtnIMPPersonalityQuizAnswer2Callback,
		BtnIMPPersonalityQuizAnswer3Callback,
		BtnIMPPersonalityQuizAnswer4Callback,
		BtnIMPPersonalityQuizAnswer5Callback,
		BtnIMPPersonalityQuizAnswer6Callback,
		BtnIMPPersonalityQuizAnswer7Callback
	};

  // will add iNumberofbuttons to the answer button list
	for (UINT32 i = 0; i < iNumberOfButtons; i++)
	{
		INT32 XLoc = LAPTOP_SCREEN_UL_X + (i < 4 ? BTN_FIRST_COLUMN_X : BTN_SECOND_COLUMN_X);
		INT32 YLoc = LAPTOP_SCREEN_WEB_UL_Y + 97 + i % 4 * 50;
		INT32 Image = LoadButtonImage("LAPTOP/button_6.sti", -1, 0, -1, 1, -1);
		giIMPPersonalityQuizAnswerButtonImage[i] = Image;
		Assert(i < lengthof(Callback));
		INT32 Button = QuickCreateButton(Image, XLoc, YLoc, BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 3, MSYS_NO_CALLBACK, Callback[i]);
		giIMPPersonalityQuizAnswerButton[i] = Button;
		SpecifyButtonUpTextColors(Button, FONT_WHITE, FONT_BLACK);
		SpecifyButtonDownTextColors(Button, FONT_WHITE, FONT_BLACK);
		SpecifyButtonTextOffsets(Button, +23, +12, TRUE);
		SpecifyButtonFont(Button, FONT12ARIAL);
		wchar_t sString[32];
		swprintf(sString, lengthof(sString), L"%d", i + 1);
		SpecifyButtonText(Button, sString);
		SetButtonCursor(Button, CURSOR_WWW);
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
		 giIMPPersonalityQuizAnswerButton[ iCounter ] = -1;
	}
}


static void CheckStateOfTheConfirmButton(void);


static void QuizAnswerCallback(INT32 Answer, GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		ResetQuizAnswerButtons();
		btn->uiFlags |= BUTTON_CLICKED_ON;
		CheckStateOfTheConfirmButton();
		iCurrentAnswer = Answer;
		PrintImpText();
		PrintQuizQuestionNumber();
		fReDrawCharProfile = TRUE;
	}
}


static void BtnIMPPersonalityQuizAnswer0Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(0, btn, reason);
}


static void BtnIMPPersonalityQuizAnswer1Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(1, btn, reason);
}


static void BtnIMPPersonalityQuizAnswer2Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(2, btn, reason);
}


static void BtnIMPPersonalityQuizAnswer3Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(3, btn, reason);
}


static void BtnIMPPersonalityQuizAnswer4Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(4, btn, reason);
}


static void BtnIMPPersonalityQuizAnswer5Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(5, btn, reason);
}

static void BtnIMPPersonalityQuizAnswer6Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(6, btn, reason);
}


static void BtnIMPPersonalityQuizAnswer7Callback(GUI_BUTTON *btn, INT32 reason)
{
	QuizAnswerCallback(7, btn, reason);
}


static void CheckAndUpdateNextPreviousIMPQuestionButtonStates(void);
static void CompileQuestionsInStatsAndWhatNot(void);


static void BtnIMPPersonalityQuizAnswerConfirmCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
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


static void BtnIMPPersonalityQuizStartOverCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
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
		ButtonList[giIMPPersonalityQuizAnswerButton[i]]->uiFlags &= ~BUTTON_CLICKED_ON;
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
		// XXX TODO0006 the effects seems odd. answer 3 is even more aggressive than answer 2
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
		case 1: // none // XXX TODO0006 fallthrough? code and comment disagree
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
		case 6: break; // asshole // XXX TODO0006 code and commend disagree
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
		// XXX TODO0006 cases 2 and 3 probably interchanged
		case 3: break; // none
		case 2: AddAnAttitudeToAttitudeList(ATT_ARROGANT); break;
	}

	switch (iQuizAnswerList[15])
	{
		// XXX TODO0006 this question has no effect
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
				if( ( giCurrentPersonalityQuizQuestion == 5) )
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
				 if( ( giCurrentPersonalityQuizQuestion == 19) )
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

	CHAR16 sString[ 10 ];


	// setup font
	SetFont( FONT12ARIAL);
	SetFontForeground( FONT_WHITE );
	SetFontBackground( FONT_BLACK );

	// get current question number into a string
	swprintf(sString, lengthof(sString), L"%d", giCurrentPersonalityQuizQuestion + 1);

	// print current question number
  mprintf( LAPTOP_SCREEN_UL_X + 345 , LAPTOP_SCREEN_WEB_UL_Y + 370 ,sString);

  // total number of questions
  mprintf( LAPTOP_SCREEN_UL_X + 383 , LAPTOP_SCREEN_WEB_UL_Y + 370 , L"16");
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
	InputAtom					InputEvent;
	POINT  MousePos;
	BOOLEAN fSkipFrame = FALSE;

	GetCursorPos(&MousePos);

  while( ( DequeueEvent(&InputEvent) == TRUE )  )
  {
	if( fSkipFrame == FALSE )
	{
		// HOOK INTO MOUSE HOOKS


		/*
		if( (InputEvent.usEvent == KEY_DOWN ) && ( InputEvent.usParam >= '1' ) && ( InputEvent.usParam <= '9') )
		{
			if( ( UINT16 )( iNumberOfPersonaButtons ) >= InputEvent.usParam - '0' )
			{
				// reset buttons
				ResetQuizAnswerButtons( );

				// ok, check to see if button was disabled, if so, re enable
				CheckStateOfTheConfirmButton( );

				// toggle this button on
				ButtonList[ giIMPPersonalityQuizAnswerButton[ InputEvent.usParam - '1' ] ]->uiFlags |= (BUTTON_CLICKED_ON);

				iCurrentAnswer = InputEvent.usParam - '1';

				PrintImpText( );

				// the current and last question numbers
				PrintQuizQuestionNumber( );

				fReDrawCharProfile = TRUE;
				fSkipFrame = TRUE;
			}
		}
		else if (iCurrentAnswer != -1 && InputEvent.usEvent == KEY_DOWN && InputEvent.usParam == SDLK_RETURN)
		{
			// reset all the buttons
			ResetQuizAnswerButtons( );

			// copy the answer into the list
			iQuizAnswerList[ giCurrentPersonalityQuizQuestion ] = iCurrentAnswer;

			// reset answer for next question
			iCurrentAnswer = -1;

			// next question, JOHNNY!
			giCurrentPersonalityQuizQuestion++;
			giMaxPersonalityQuizQuestion++;


			// OPPS!, done..time to finish up
			if( giCurrentPersonalityQuizQuestion > 15)
			{
				iCurrentImpPage = IMP_PERSONALITY_FINISH;
				// process
				CompileQuestionsInStatsAndWhatNot( );
			}

			fSkipFrame = TRUE;
		}
		else if( ( InputEvent.usEvent == KEY_DOWN ) && ( InputEvent.usParam == '=' ) )
		{
			MoveAheadAQuestion( );
			fSkipFrame = TRUE;
		}
		else if( ( InputEvent.usEvent == KEY_DOWN ) && ( InputEvent.usParam == '-' ) )
		{
			MoveBackAQuestion( );
			fSkipFrame = TRUE;
		}
		else
		{

*/			switch(InputEvent.usEvent)
			{
				case LEFT_BUTTON_DOWN:
				case LEFT_BUTTON_UP:
				case RIGHT_BUTTON_DOWN:
				case RIGHT_BUTTON_UP:
				case RIGHT_BUTTON_REPEAT:
				case LEFT_BUTTON_REPEAT:
					MouseSystemHook(InputEvent.usEvent, MousePos.x, MousePos.y, _LeftButtonDown, _RightButtonDown);
					break;

				default:
					HandleKeyBoardShortCutsForLapTop( InputEvent.usEvent, InputEvent.usParam, InputEvent.usKeyState );
					break;
//			}
		}
		}
	}
}


static void CheckAndUpdateNextPreviousIMPQuestionButtonStates(void)
{
	if( giCurrentPersonalityQuizQuestion >= giMaxPersonalityQuizQuestion )
	{
		DisableButton( giNextQuestionButton );
	}
	else
	{
		EnableButton( giNextQuestionButton );
	}

	if( giCurrentPersonalityQuizQuestion == 0 )
	{
		DisableButton( giPreviousQuestionButton );
	}
	else
	{
		EnableButton( giPreviousQuestionButton );
	}
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

/*
	EnableButton( giPreviousQuestionButton );

	if( giCurrentPersonalityQuizQuestion >= giMaxPersonalityQuizQuestion )
	{
		DisableButton( giNextQuestionButton );
		iCurrentAnswer = -1;
	}
	else
	{
		EnableButton( giNextQuestionButton );
	}
*/
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
/*
	if( giCurrentPersonalityQuizQuestion == 0 )
	{
		DisableButton( giPreviousQuestionButton );
	}
	else
	{
		EnableButton( giPreviousQuestionButton );
	}
*/
}


static void ToggleQuestionNumberButtonOn(INT32 iAnswerNumber)
{
	if( ( giCurrentPersonalityQuizQuestion <= giMaxPersonalityQuizQuestion ) && ( iAnswerNumber != -1 ) )
	{
		// reset buttons
		ResetQuizAnswerButtons( );

		// toggle this button on
		ButtonList[ giIMPPersonalityQuizAnswerButton[ iAnswerNumber ] ]->uiFlags |= (BUTTON_CLICKED_ON);
		iCurrentAnswer = iAnswerNumber;
	}
}


static void PreviousQuestionButtonCallback(GUI_BUTTON *btn, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		MoveBackAQuestion( );
	}
}


static void NextQuestionButtonCallback(GUI_BUTTON *btn, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		MoveAheadAQuestion( );
	}
}
