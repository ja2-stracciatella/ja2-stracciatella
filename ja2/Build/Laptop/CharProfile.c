#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "Cursors.h"
	#include "CharProfile.h"
	#include "IMP_AboutUs.h"
	#include "IMP_Attribute_Entrance.h"
	#include "IMP_Attribute_Finish.h"
	#include "IMP_MainPage.h"
	#include "IMP_HomePage.h"
	#include "IMPVideoObjects.h"
	#include "IMP_Text_System.h"
	#include "IMP_Finish.h"
	#include "IMP_Portraits.h"
	#include "IMP_Voices.h"
	#include "IMP_Personality_Entrance.h"
	#include "IMP_Attribute_Selection.h"
	#include "IMP_Personality_Quiz.h"
	#include "IMP_Begin_Screen.h"
	#include "IMP_Personality_Finish.h"
	#include "IMPVideoObjects.h"
	#include "IMP_Confirm.h"
	#include "MessageBoxScreen.h"
	#include "LaptopSave.h"
	#include <string.h>
	#include "Button_System.h"
	#include "Debug.h"
	#include "Font_Control.h"
#endif


//BOOLEAN fIMPCompletedFlag = FALSE;
BOOLEAN fReDrawCharProfile = FALSE;
BOOLEAN fButtonPendingFlag = FALSE;
BOOLEAN fAddCreatedCharToPlayersTeam = FALSE;
BOOLEAN fReEnterIMP = FALSE;

INT32 iCurrentImpPage = IMP_HOME_PAGE;
INT32 iPreviousImpPage = -1;


// attributes
INT32 iStrength = 55;
INT32 iDexterity = 55;
INT32 iAgility = 55;
INT32 iWisdom = 55;
INT32 iLeadership =55;
INT32 iHealth = 55;

// skills
INT32 iMarksmanship = 55;
INT32 iMedical = 55;
INT32 iExplosives = 55;
INT32 iMechanical = 55;

// gender
BOOLEAN fCharacterIsMale = TRUE;

// name and nick name
CHAR16 pFullName[ 32 ];
CHAR16 pNickName[ 32 ];

// skills
INT32 iSkillA = 0;
INT32 iSkillB = 0;

// personality
INT32 iPersonality = 0;

// attitude
INT32 iAttitude = 0;


// additives, but no preservatives
INT32 iAddStrength = 0;
INT32 iAddDexterity = 0;
INT32 iAddAgility= 0;
INT32 iAddWisdom= 0;
INT32 iAddHealth = 0;
INT32 iAddLeadership = 0;

INT32 iAddMarksmanship = 0;
INT32 iAddMedical = 0;
INT32 iAddExplosives = 0;
INT32 iAddMechanical = 0;


// IMP global buttons
INT32 giIMPButton[1];
INT32 giIMPButtonImage[1];

// visted subpages
BOOLEAN fVisitedIMPSubPages[ IMP_NUM_PAGES ];
extern INT32 iCurrentPortrait;
extern int iCurrentVoices;
extern	INT32 giMaxPersonalityQuizQuestion;
extern	BOOLEAN fStartOverFlag;

void ExitOldIMPMode( void );
void EnterNewIMPMode( void );
void LoadImpGraphics( void );
void RemoveImpGraphics( void );
void CreateIMPButtons( void );
void DestroyIMPButtons( void );
static void BtnIMPCancelCallback(GUI_BUTTON *btn, INT32 reason);
BOOLEAN HasTheCurrentIMPPageBeenVisited( void );
extern void SetAttributes( void );


void GameInitCharProfile()
{
	LaptopSaveInfo.iVoiceId = 0;
	iCurrentPortrait = 0;
	iCurrentVoices = 0;
	iPortraitNumber = 0;
}

void EnterCharProfile()
{
	// reset previous page
  iPreviousImpPage = -1;

	// grab the graphics
	LoadImpGraphics( );
}

void ExitCharProfile()
{
  // get rid of graphics
	RemoveImpGraphics( );

	// clean up past mode
	ExitOldIMPMode( );
}

void HandleCharProfile()
{
	if( fReDrawCharProfile )
  {
		// re draw
    RenderCharProfile( );
		fReDrawCharProfile = FALSE;

	}

	// button pending, but not changing mode, still need a rernder, but under different circumstances
	if( ( fButtonPendingFlag == TRUE )&&( iCurrentImpPage == iPreviousImpPage ) )
	{
		RenderCharProfile( );
	}

	// page has changed, handle the fact..get rid of old page, load up new, and re render
	if( ( iCurrentImpPage != iPreviousImpPage ) )
	{
		if( fDoneLoadPending == FALSE )
		{
			//make sure we are not hosing memory
			Assert( iCurrentImpPage <= IMP_NUM_PAGES );


			fFastLoadFlag = HasTheCurrentIMPPageBeenVisited( );
			fVisitedIMPSubPages[ iCurrentImpPage ] = TRUE;
			fConnectingToSubPage = TRUE;

			if( iPreviousImpPage != -1 )
			{
				fLoadPendingFlag = TRUE;
				MarkButtonsDirty();
				return;
			}
			else
			{
				fDoneLoadPending = TRUE;
			}


		}

		fVisitedIMPSubPages[ iCurrentImpPage ] = TRUE;

		if( fButtonPendingFlag == TRUE )
		{
      // render screen
      RenderCharProfile( );
		  return;
		}

		// exity old mode
    ExitOldIMPMode( );

		// set previous page
		iPreviousImpPage = iCurrentImpPage;

		// enter new
    EnterNewIMPMode( );


		// render screen
    RenderCharProfile( );

		 // render title bar



	}

	// handle
  switch( iCurrentImpPage )
	 {
		 case( IMP_HOME_PAGE ):
			 HandleImpHomePage( );
		 break;
		 case( IMP_BEGIN ):
		   HandleIMPBeginScreen( );
		 break;
		 case( IMP_PERSONALITY ):
			 HandleIMPPersonalityEntrance( );
		 break;
		 case( IMP_PERSONALITY_QUIZ ):
			 HandleIMPPersonalityQuiz( );
		 break;
		 case( IMP_PERSONALITY_FINISH ):
		   HandleIMPPersonalityFinish( );
		 break;
		 case( IMP_ATTRIBUTE_ENTRANCE ):
			 HandleIMPAttributeEntrance( );
		 break;
		 case( IMP_ATTRIBUTE_PAGE ):
       HandleIMPAttributeSelection( );
		 break;
		 case( IMP_ATTRIBUTE_FINISH ):
       HandleIMPAttributeFinish( );
		 break;
		 case( IMP_PORTRAIT ):
       HandleIMPPortraits( );
		 break;
		 case( IMP_VOICE ):
       HandleIMPVoices( );
		 break;
		 case( IMP_FINISH ):
       HandleIMPFinish( );
		 break;
		 case( IMP_ABOUT_US ):
       HandleIMPAboutUs( );
		 break;
		 case( IMP_MAIN_PAGE ):
			 HandleIMPMainPage( );
		 break;
		 case( IMP_CONFIRM ):
		   HandleIMPConfirm( );
		 break;
	 }
}

void RenderCharProfile()
{
	// button is waiting to go up?...do nothing,

	 if( fButtonPendingFlag )
	 {
		 fPausedReDrawScreenFlag = TRUE;
		 fButtonPendingFlag = FALSE;
		 return;
	 }

	 switch( iCurrentImpPage )
	 {
		 case( IMP_HOME_PAGE ):
			 RenderImpHomePage( );
		 break;
		 case( IMP_BEGIN ):
		   RenderIMPBeginScreen( );
		 break;
		 case( IMP_PERSONALITY ):
			 RenderIMPPersonalityEntrance( );
		 break;
		 case( IMP_PERSONALITY_QUIZ ):
			 RenderIMPPersonalityQuiz( );
		 break;
		 case( IMP_PERSONALITY_FINISH ):
		   RenderIMPPersonalityFinish( );
		 break;
		 case( IMP_ATTRIBUTE_ENTRANCE ):
			 RenderIMPAttributeEntrance( );
		 break;
		 case( IMP_ATTRIBUTE_PAGE ):
       RenderIMPAttributeSelection( );
		 break;
		 case( IMP_ATTRIBUTE_FINISH ):
       RenderIMPAttributeFinish( );
		 break;
		 case( IMP_PORTRAIT ):
       RenderIMPPortraits( );
		 break;
		 case( IMP_VOICE ):
       RenderIMPVoices( );
		 break;
		 case( IMP_FINISH ):
       RenderIMPFinish( );
		 break;
		 case( IMP_ABOUT_US ):
       RenderIMPAboutUs( );
		 break;
		 case( IMP_MAIN_PAGE ):
		   RenderIMPMainPage( );
		 break;
		 case( IMP_CONFIRM ):
		   RenderIMPConfirm( );
		 break;
	 }

	 // render title bar
	 //RenderWWWProgramTitleBar( );

   // render the text
	 PrintImpText( );

	 RenderWWWProgramTitleBar( );

	 DisplayProgramBoundingBox( TRUE );

	 //InvalidateRegion( 0, 0, 640, 480 );
}


void ExitOldIMPMode( void )
{
  // exit old mode

	if( iPreviousImpPage == -1 )
	{
		// don't both, leave
	  return;

	}
	// remove old mode
	 switch( iPreviousImpPage )
	 {
		 case( IMP_HOME_PAGE ):
			 ExitImpHomePage( );
		 break;
		 case( IMP_BEGIN ):
       DestroyIMPButtons( );
		   ExitIMPBeginScreen( );
		 break;
		 case( IMP_FINISH ):
       DestroyIMPButtons( );
		   ExitIMPFinish( );
		 break;
		 case( IMP_PERSONALITY ):
			 DestroyIMPButtons( );
			 ExitIMPPersonalityEntrance( );
		 break;
		 case( IMP_PERSONALITY_QUIZ ):
			 DestroyIMPButtons( );
			 ExitIMPPersonalityQuiz( );
		 break;
		 case( IMP_PERSONALITY_FINISH ):
			 DestroyIMPButtons( );
		   ExitIMPPersonalityFinish( );
		 break;
		 case( IMP_ATTRIBUTE_ENTRANCE ):
			 DestroyIMPButtons( );
			 ExitIMPAttributeEntrance( );
		 break;
		 case( IMP_ATTRIBUTE_PAGE ):
			 DestroyIMPButtons( );
       ExitIMPAttributeSelection( );
		 break;
		 case( IMP_ATTRIBUTE_FINISH ):
			 DestroyIMPButtons( );
       ExitIMPAttributeFinish( );
		 break;
		 case( IMP_PORTRAIT ):
			 DestroyIMPButtons( );
       ExitIMPPortraits( );
		 break;
		 case( IMP_VOICE ):
			 DestroyIMPButtons( );
       ExitIMPVoices( );
		 break;
		 case( IMP_ABOUT_US ):
       ExitIMPAboutUs( );
		 break;
		 case( IMP_MAIN_PAGE ):
		   ExitIMPMainPage( );
		 break;
		  case( IMP_CONFIRM ):
		   ExitIMPConfirm( );
		 break;
	 }
}



void EnterNewIMPMode( void )
{
  // enter new mode

	 switch( iCurrentImpPage )
	 {
		 case( IMP_HOME_PAGE ):
			 EnterImpHomePage( );
		 break;
		 case( IMP_BEGIN ):
			 CreateIMPButtons( );
		   EnterIMPBeginScreen( );
		 break;
		 case( IMP_FINISH ):
			 CreateIMPButtons( );
		   EnterIMPFinish( );
		 break;
		 case( IMP_PERSONALITY ):
			 CreateIMPButtons( );
			 EnterIMPPersonalityEntrance( );
		 break;
		 case( IMP_PERSONALITY_QUIZ ):
			 CreateIMPButtons( );
			 EnterIMPPersonalityQuiz( );
		 break;
		 case( IMP_PERSONALITY_FINISH ):
			 CreateIMPButtons( );
		   EnterIMPPersonalityFinish( );
		 break;
		 case( IMP_ATTRIBUTE_ENTRANCE ):
			 CreateIMPButtons( );
			 EnterIMPAttributeEntrance( );
		 break;
		 case( IMP_ATTRIBUTE_PAGE ):
       CreateIMPButtons( );
       EnterIMPAttributeSelection( );
		 break;
		 case( IMP_ATTRIBUTE_FINISH ):
			 CreateIMPButtons( );
       EnterIMPAttributeFinish( );
		 break;
		 case( IMP_PORTRAIT ):
			 CreateIMPButtons( );
       EnterIMPPortraits( );
		 break;
		 case( IMP_VOICE ):
			 CreateIMPButtons( );
       EnterIMPVoices( );
		 break;
		 case( IMP_ABOUT_US ):
       EnterIMPAboutUs( );
		 break;
		 case( IMP_MAIN_PAGE ):
		   EnterIMPMainPage( );
		 break;
     case( IMP_CONFIRM ):
		   EnterIMPConfirm( );
		 break;
	 }
}


void ResetCharacterStats( void )
{
  // attributes
  iStrength = 55;
  iDexterity = 55;
  iAgility = 55;
  iWisdom = 55;
  iLeadership =55;
  iHealth = 55;

  // skills
  iMarksmanship = 55;
  iMedical = 55;
  iExplosives = 55;
  iMechanical = 55;

	// skills
  iSkillA = 0;
  iSkillB = 0;

  // personality
  iPersonality = 0;

  // attitude
  iAttitude = 0;

	// names
	memset( &pFullName, 0 , sizeof( pFullName) );
	memset( &pNickName, 0 , sizeof( pNickName) );


}



void LoadImpGraphics( void )
{
	// load all graphics needed for IMP

	LoadProfileBackGround( );
	LoadIMPSymbol( );
	LoadBeginIndent( );
	LoadActivationIndent( );
	LoadFrontPageIndent( );
	LoadAnalyse( );
	LoadAttributeGraph( );
	LoadAttributeGraphBar( );

	LoadFullNameIndent( );
  LoadNameIndent( );
	LoadGenderIndent( );
	LoadNickNameIndent( );

	//LoadSmallFrame( );

	LoadSmallSilhouette( );
	LoadLargeSilhouette( );

	LoadAttributeFrame( );
	LoadSliderBar( );

	LoadButton2Image( );
	LoadButton4Image( );
  LoadButton1Image( );

	LoadPortraitFrame( );
	LoadMainIndentFrame( );

	LoadQtnLongIndentFrame( );
	LoadQtnShortIndentFrame( );
	LoadQtnLongIndentHighFrame( );
	LoadQtnShortIndentHighFrame( );
  LoadQtnShort2IndentFrame( );
	LoadQtnShort2IndentHighFrame( );

	LoadQtnIndentFrame( );
  LoadAttrib1IndentFrame( );
	LoadAttrib2IndentFrame( );
  LoadAvgMercIndentFrame( );
	LoadAboutUsIndentFrame( );
}

void RemoveImpGraphics( void )
{
	// remove all graphics needed for IMP

  RemoveProfileBackGround( );
	DeleteIMPSymbol( );
	DeleteBeginIndent( );
	DeleteActivationIndent( );
	DeleteFrontPageIndent( );
	DeleteAnalyse( );
	DeleteAttributeGraph( );
	DeleteAttributeBarGraph( );

	DeleteFullNameIndent( );
  DeleteNameIndent( );
	DeleteGenderIndent( );
	DeleteNickNameIndent( );

	//DeleteSmallFrame( );

	DeleteSmallSilhouette( );
	DeleteLargeSilhouette( );

	DeleteAttributeFrame( );
	DeleteSliderBar( );

	DeleteButton2Image( );
	DeleteButton4Image( );
  DeleteButton1Image( );

	DeletePortraitFrame( );
	DeleteMainIndentFrame( );

	DeleteQtnLongIndentFrame( );
	DeleteQtnShortIndentFrame( );
	DeleteQtnLongIndentHighFrame( );
	DeleteQtnShortIndentHighFrame( );
  DeleteQtnShort2IndentFrame( );
	DeleteQtnShort2IndentHighFrame( );

	DeleteQtnIndentFrame( );
  DeleteAttrib1IndentFrame( );
	DeleteAttrib2IndentFrame( );
  DeleteAvgMercIndentFrame( );
	DeleteAboutUsIndentFrame( );
}

void CreateIMPButtons( void )
{
  // create all the buttons global to the IMP system

  giIMPButtonImage[ 0 ] = LoadButtonImage( "LAPTOP\\button_3.sti" ,-1,0,-1,1,-1 );


	// cancel
	giIMPButton[0] = CreateIconAndTextButton( giIMPButtonImage[0], pImpButtonText[ 19 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X + 15, LAPTOP_SCREEN_WEB_UL_Y + ( 360 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPCancelCallback);

	SpecifyButtonTextSubOffsets( giIMPButton[0], 0, -1, FALSE );

	// set up generic www cursor
	SetButtonCursor(giIMPButton[ 0 ], CURSOR_WWW);
}

void DestroyIMPButtons( void )
{
  // destroy the buttons we created
	RemoveButton(giIMPButton[0] );
  UnloadButtonImage(giIMPButtonImage[0] );
}


static void BtnIMPCancelCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// back to the main page, otherwise, back to home page
		switch (iCurrentImpPage)
		{
			case IMP_MAIN_PAGE:
				iCurrentImpPage = IMP_HOME_PAGE;
				fButtonPendingFlag = TRUE;
				iCurrentProfileMode = 0;
				fFinishedCharGeneration = FALSE;
				ResetCharacterStats( );
				break;

			case IMP_FINISH:
				iCurrentImpPage = IMP_MAIN_PAGE;
				iCurrentProfileMode = 4;
				fFinishedCharGeneration = FALSE;
				fButtonPendingFlag = TRUE;
				//iCurrentProfileMode = 0;
				//fFinishedCharGeneration = FALSE;
				//ResetCharacterStats( );
				break;

			case IMP_PERSONALITY_QUIZ:
			case IMP_PERSONALITY_FINISH:
				giMaxPersonalityQuizQuestion = 0;
				fStartOverFlag = TRUE;
				iCurrentAnswer = -1;
				iCurrentImpPage = IMP_PERSONALITY;
				fButtonPendingFlag = TRUE;
				break;

			case IMP_ATTRIBUTE_PAGE:
				SetAttributes();
				/* FALLTHROUGH */

			default:
				iCurrentImpPage = IMP_MAIN_PAGE;
				iCurrentAnswer = -1;
				break;
		}
	}
}


void InitIMPSubPageList( void )
{
	INT32 iCounter = 0;

	for(iCounter = 0; iCounter < IMP_CONFIRM; iCounter++ )
	{
		fVisitedIMPSubPages[ iCounter ] = FALSE ;
	}
}

BOOLEAN HasTheCurrentIMPPageBeenVisited( void )
{
	// returns if we have vsisted the current IMP PageAlready

	//make sure we are not hosing memory
	Assert( iCurrentImpPage <= IMP_NUM_PAGES );

	return ( fVisitedIMPSubPages[ iCurrentImpPage ]);
}
