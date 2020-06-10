#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
	#include "IMP Skill Trait.h"
	#include "_Ja25EnglishText.h"
#else
#endif



//*******************************************************************
//
// Local Defines
//
//*******************************************************************

#define			IMP_SKILL_TRAIT__TITLE_FONT				FONT14ARIAL

#define			IMP_SKILL_TRAIT__FONT							FONT12ARIAL
#define			IMP_SKILL_TRAIT__COLOR						FONT_MCOLOR_WHITE



enum
{
	IMP_SKILL_TRAIT__TITLE_TEXT=IMP_SKILL_TRAITS__NUMBER_SKILLS,
};



#define	IMP_SKILL_TRAIT__LEFT_COLUMN_START_X								( LAPTOP_SCREEN_UL_X + 15 )
#define	IMP_SKILL_TRAIT__LEFT_COLUMN_START_Y								( LAPTOP_SCREEN_WEB_UL_Y + 40 )

#define	IMP_SKILL_TRAIT__RIGHT_COLUMN_START_X								( IMP_SKILL_TRAIT__LEFT_COLUMN_START_X + 241 )
#define	IMP_SKILL_TRAIT__RIGHT_COLUMN_START_Y								IMP_SKILL_TRAIT__LEFT_COLUMN_START_Y

#define	IMP_SKILL_TRAIT__SPACE_BTN_BUTTONS									38

#define	IMP_SKILL_TRAIT__SKILL_TRAIT_TO_START_RIGHT_COL			6

#define	IMP_SKILL_TRAIT__NONE_BTN_LOC_X											( LAPTOP_SCREEN_UL_X + 136 )

#define	IMP_SKILL_TRAIT__TEXT_OFFSET_X											65
#define	IMP_SKILL_TRAIT__TEXT_OFFSET_Y											12

#define	IMP_SKILL_TRAIT__TITLE_X														LAPTOP_SCREEN_UL_X
#define	IMP_SKILL_TRAIT__TITLE_Y														53
#define	IMP_SKILL_TRAIT__TITLE_WIDTH												( LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X )

#define	IMP_SKILL_TRAIT__GREY_BOX_OFFSET_X									5
#define	IMP_SKILL_TRAIT__GREY_BOX_OFFSET_Y									7


//*******************************************************************
//
// Global Variables
//
//*******************************************************************

BOOLEAN gfIST_Redraw=FALSE;

BOOLEAN	gfSkillTraitQuestions[ IMP_SKILL_TRAITS__NUMBER_SKILLS ];

// these are the buttons for the questions
INT32 giIMPSkillTraitAnswerButton[ IMP_SKILL_TRAITS__NUMBER_SKILLS ];
INT32 giIMPSkillTraitAnswerButtonImage[ IMP_SKILL_TRAITS__NUMBER_SKILLS ];

// this is the Done  buttons
INT32 giIMPSkillTraitFinsihButton;
INT32 giIMPSkillTraitFinsihButtonImage;



//BOOLEAN	gfSkillTraitButtonChanged=FALSE;

#define	IST__NUM_SELECTABLE_TRAITS				2
INT8	gbLastSelectedTraits[ IST__NUM_SELECTABLE_TRAITS ];

//image handle
UINT32	guiIST_GreyGoldBox;

//*******************************************************************
//
// Function Prototypes
//
//*******************************************************************

void		BtnIMPSkillTraitAnswerCallback(GUI_BUTTON *btn,INT32 reason);
void		BtnIMPSkillTraitFinishCallback(GUI_BUTTON *btn,INT32 reason);
void		AddImpSkillTraitButtons();
void		HandleSkillTraitButtonStates( );
void		HandleIMPSkillTraitAnswers( UINT32 uiSkillPressed, BOOLEAN fResetAllButtons );
void		IMPSkillTraitDisplaySkills();
INT8		CountNumSkillStraitsSelected( BOOLEAN fIncludeNoneSkill );
BOOLEAN ShouldTraitBeSkipped( UINT32 uiTrait );
void		AddSelectedSkillsToSkillsList();
void		HandleLastSelectedTraits( INT8 bNewTrait );
BOOLEAN CameBackToSpecialtiesPageButNotFinished();
//ppp

//*******************************************************************
//
// Functions
//
//*******************************************************************



void EnterIMPSkillTrait( void )
{
//	UINT32 uiCnt;
  VOBJECT_DESC    VObjectDesc;

	//add the skill trait buttons
	AddImpSkillTraitButtons();


	// load the stats graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\SkillTraitSmallGreyIdent.sti", VObjectDesc.ImageFile);
	if( !AddVideoObject(&VObjectDesc, &guiIST_GreyGoldBox ) )
	{
		Assert( 0 );
		return;
	}

  giIMPSkillTraitFinsihButtonImage =  LoadButtonImage( "LAPTOP\\button_5.sti" ,-1,0,-1,1,-1 );
  giIMPSkillTraitFinsihButton = CreateIconAndTextButton( giIMPSkillTraitFinsihButtonImage, pImpButtonText[ 11 ], FONT12ARIAL, 
																 FONT_WHITE, DEFAULT_SHADOW, 
																 FONT_WHITE, DEFAULT_SHADOW, 
																 TEXT_CJUSTIFIED, 
																 LAPTOP_SCREEN_UL_X +  ( 350 ), LAPTOP_SCREEN_WEB_UL_Y + ( 340 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 			 BtnGenericMouseMoveButtonCallback, (GUI_CALLBACK)BtnIMPSkillTraitFinishCallback );
	SetButtonCursor( giIMPSkillTraitFinsihButton, CURSOR_WWW);

	//if we are not DONE and are just reviewing
	if( iCurrentProfileMode != 5 )
	{
		//Have the NONE trait initially selected
		gfSkillTraitQuestions[ IMP_SKILL_TRAITS__NONE ] = TRUE;
	}

	HandleSkillTraitButtonStates( );

	//reset last selecterd trait
	memset( gbLastSelectedTraits, -1, IST__NUM_SELECTABLE_TRAITS );
}


void RenderIMPSkillTrait( void )
{
	//render the metal background graphic
	RenderProfileBackGround();

	IMPSkillTraitDisplaySkills();
}


void ExitIMPSkillTrait( void )
{
	INT32 iCnt;

	DeleteVideoObjectFromIndex( guiIST_GreyGoldBox );

	//remove the skill buttons
	for(iCnt = 0; iCnt < IMP_SKILL_TRAITS__NUMBER_SKILLS; iCnt++)
	{
		//if there is a button allocated
		if( giIMPSkillTraitAnswerButton[iCnt] != -1 )
		{
			RemoveButton(giIMPSkillTraitAnswerButton[ iCnt ] );
			UnloadButtonImage(giIMPSkillTraitAnswerButtonImage[ iCnt ] );
		}
	}

	RemoveButton( giIMPSkillTraitFinsihButton );
	UnloadButtonImage( giIMPSkillTraitFinsihButtonImage );
}


void HandleIMPSkillTrait( void )
{
	if( gfIST_Redraw )
	{
		RenderIMPSkillTrait( );
		gfIST_Redraw = FALSE;
	}

  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}

void AddImpSkillTraitButtons()
{
	INT32 iCnt = 0;
	UINT16 usPosX, usPosY;

	usPosX = IMP_SKILL_TRAIT__LEFT_COLUMN_START_X;
	usPosY = IMP_SKILL_TRAIT__LEFT_COLUMN_START_Y;

	for(iCnt = 0; iCnt < IMP_SKILL_TRAITS__NUMBER_SKILLS; iCnt++)
	{
		//reset
		giIMPSkillTraitAnswerButton[iCnt] = -1;

		//if we are not DONE and are just reviewing
		if( iCurrentProfileMode != 5 )
		{
			gfSkillTraitQuestions[ iCnt ] = FALSE;
		}

		//if the merc is a FEMALE, skip this skill cause there isnt any fenmal martial artists
		if( ShouldTraitBeSkipped( iCnt ) )
		{
			usPosY += IMP_SKILL_TRAIT__SPACE_BTN_BUTTONS;

			//if its the none button
			if( iCnt == IMP_SKILL_TRAITS__NONE-1 )
			{
				usPosX = IMP_SKILL_TRAIT__NONE_BTN_LOC_X;
			}
			continue;
		}

		if( iCnt == 0 )
			giIMPSkillTraitAnswerButtonImage[ iCnt ] =  LoadButtonImage( "LAPTOP\\button_6.sti", -1,0,-1,1,-1 );
		else
			giIMPSkillTraitAnswerButtonImage[ iCnt ] =  UseLoadedButtonImage( giIMPSkillTraitAnswerButtonImage[ 0 ], -1,0,-1,1,-1 );

		giIMPSkillTraitAnswerButton[iCnt] = QuickCreateButton( giIMPSkillTraitAnswerButtonImage[ iCnt ], usPosX, usPosY,
									BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 3,
									MSYS_NO_CALLBACK, (GUI_CALLBACK)BtnIMPSkillTraitAnswerCallback);

		//Set the button data
		MSYS_SetBtnUserData( giIMPSkillTraitAnswerButton[iCnt], 0, iCnt );
		SetButtonCursor( giIMPSkillTraitAnswerButton[iCnt], CURSOR_WWW);

		//Get rid of playing the button sound, it will be handled here
		ButtonList[ giIMPSkillTraitAnswerButton[ iCnt ] ]->ubSoundSchemeID = 0;

		//Determine the next x location
		if( iCnt < IMP_SKILL_TRAIT__SKILL_TRAIT_TO_START_RIGHT_COL )
			usPosX = IMP_SKILL_TRAIT__LEFT_COLUMN_START_X;
		else
			usPosX = IMP_SKILL_TRAIT__RIGHT_COLUMN_START_X;

		//Determine the next Y location
		if( iCnt == IMP_SKILL_TRAIT__SKILL_TRAIT_TO_START_RIGHT_COL )
			usPosY = IMP_SKILL_TRAIT__RIGHT_COLUMN_START_Y;
		else
			usPosY += IMP_SKILL_TRAIT__SPACE_BTN_BUTTONS;

		//if its the none button
		if( iCnt == IMP_SKILL_TRAITS__NONE-1 )
		{
			usPosX = IMP_SKILL_TRAIT__NONE_BTN_LOC_X;
		}
	}
}


void BtnIMPSkillTraitAnswerCallback(GUI_BUTTON *btn,INT32 reason)
{
	// btn callback for IMP personality quiz answer button
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
//		btn->uiFlags|=(BUTTON_CLICKED_ON);

		INT32 iSkillTrait =  MSYS_GetBtnUserData( btn, 0 );

		HandleIMPSkillTraitAnswers( iSkillTrait, FALSE );
	}
}
void HandleIMPSkillTraitAnswers( UINT32 uiSkillPressed, BOOLEAN fResetAllButtons )
{
	UINT32 uiCnt;

	//if we are DONE and are just reviewing
	if( iCurrentProfileMode == 5 )
	{
		return;
	}

	//if we are to reset all the buttons
	if( fResetAllButtons )
	{
		//loop through all the skill and reset them
		for( uiCnt=0; uiCnt<IMP_SKILL_TRAITS__NUMBER_SKILLS; uiCnt++ )
		{
			gfSkillTraitQuestions[ uiCnt ] = FALSE;
		}
		return;
	}

	//make sure its a valid skill
	if( uiSkillPressed > IMP_SKILL_TRAITS__NUMBER_SKILLS )
	{
		Assert( 0 );
		return;
	}


	//if its allready set
	if( gfSkillTraitQuestions[ uiSkillPressed ] )
	{
		//dont need to do anything
		return;
	}

	HandleLastSelectedTraits( (INT8)uiSkillPressed );

/*	
	//if there are already 2 skills selected, and we are trying to PRESS a new button ( skill isnt currently set )
	if( CountNumSkillStraitsSelected() == 2 && 
			!gfSkillTraitQuestions[ uiSkillPressed ] &&
			uiSkillPressed != IMP_SKILL_TRAITS__NONE )
	{
		//if we have to, unselect an existing trait

		//play the diosabld sound
//		PlayButtonSound( giIMPSkillTraitAnswerButton[ uiSkillPressed ], BUTTON_SOUND_DISABLED_CLICK );

		//dont select a new one
//		return;
	}
*/

	//Set the skill
//	gfSkillTraitQuestions[ uiSkillPressed ] = !gfSkillTraitQuestions[ uiSkillPressed ];
	gfSkillTraitQuestions[ uiSkillPressed ] = TRUE;

	//if the NONE trait was selected, clear the rest of the buttons
	if( uiSkillPressed == IMP_SKILL_TRAITS__NONE )
	{
		//reset all the traits
		HandleIMPSkillTraitAnswers( 0, TRUE );

		gfSkillTraitQuestions[ IMP_SKILL_TRAITS__NONE ] = TRUE;
	}


	//make sure the none skill is NOT selected if we select anything else
	if( uiSkillPressed != IMP_SKILL_TRAITS__NONE )
	{
		gfSkillTraitQuestions[ IMP_SKILL_TRAITS__NONE ] = FALSE;
	}

	//Play the button sound
	if( gfSkillTraitQuestions[ uiSkillPressed ] )
	{
		PlayButtonSound( giIMPSkillTraitAnswerButton[ uiSkillPressed ], BUTTON_SOUND_CLICKED_ON );
	}
	else
	{
		PlayButtonSound( giIMPSkillTraitAnswerButton[ uiSkillPressed ], BUTTON_SOUND_CLICKED_OFF );
	}

	//update buttons
	HandleSkillTraitButtonStates( );

	//redraw the screen
	gfIST_Redraw = TRUE;
}

INT8	CountNumSkillStraitsSelected( BOOLEAN fIncludeNoneSkill )
{
	UINT32	uiCnt;
	INT8	iNumberSkills=0;
	UINT32	uiUpperLimit;

	if( fIncludeNoneSkill )
	{
		uiUpperLimit = IMP_SKILL_TRAITS__NUMBER_SKILLS;
	}
	else
	{
		uiUpperLimit = IMP_SKILL_TRAITS__NONE;
	}

	for( uiCnt=0; uiCnt < uiUpperLimit; uiCnt++ )
	{
		//if the skill is selected ( ie depressed )
		if( gfSkillTraitQuestions[ uiCnt ] )
		{
			iNumberSkills++;
		}
	}

	return( iNumberSkills );
}


void HandleSkillTraitButtonStates( )
{
	UINT32 uiCnt;

	for( uiCnt=0; uiCnt<IMP_SKILL_TRAITS__NUMBER_SKILLS; uiCnt++ )
	{
			//if the merc is a FEMALE, skip this skill cause there isnt any fenmal martial artists
		if( ShouldTraitBeSkipped( uiCnt ) )
		{
			continue;
		}

		//if the skill is selected ( ie depressed )
		if( gfSkillTraitQuestions[ uiCnt ] )
		{
			ButtonList[ giIMPSkillTraitAnswerButton[ uiCnt ] ]->uiFlags |= BUTTON_CLICKED_ON;
		}
		else
		{
			ButtonList[ giIMPSkillTraitAnswerButton[ uiCnt ] ]->uiFlags &= ~BUTTON_CLICKED_ON;
		}
	}
}

void IMPSkillTraitDisplaySkills()
{
	UINT32 uiCnt;
	UINT16 usPosX, usPosY;
	UINT16 usBoxPosX, usBoxPosY;
  HVOBJECT	hImageHandle;

	//Display the title
	DrawTextToScreen( gzIMPSkillTraitsText[ IMP_SKILL_TRAIT__TITLE_TEXT ], IMP_SKILL_TRAIT__TITLE_X, IMP_SKILL_TRAIT__TITLE_Y, IMP_SKILL_TRAIT__TITLE_WIDTH, IMP_SKILL_TRAIT__TITLE_FONT, IMP_SKILL_TRAIT__COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

	// Stats
	GetVideoObject(&hImageHandle, guiIST_GreyGoldBox );

	usPosX = IMP_SKILL_TRAIT__LEFT_COLUMN_START_X + IMP_SKILL_TRAIT__TEXT_OFFSET_X;
	usPosY = IMP_SKILL_TRAIT__LEFT_COLUMN_START_Y + IMP_SKILL_TRAIT__TEXT_OFFSET_Y;

	for( uiCnt=0; uiCnt<IMP_SKILL_TRAITS__NUMBER_SKILLS; uiCnt++ )
	{
		usBoxPosX = usPosX - IMP_SKILL_TRAIT__GREY_BOX_OFFSET_X;
		usBoxPosY = usPosY - IMP_SKILL_TRAIT__GREY_BOX_OFFSET_Y;

		//if the merc is a FEMALE, skip this skill cause there isnt any fenmal martial artists
		if( ShouldTraitBeSkipped( uiCnt ) )
		{
			//if its the none button
			if( uiCnt == IMP_SKILL_TRAITS__NONE-1 )
			{
				usPosX = IMP_SKILL_TRAIT__NONE_BTN_LOC_X + IMP_SKILL_TRAIT__TEXT_OFFSET_X;
				usPosY += IMP_SKILL_TRAIT__SPACE_BTN_BUTTONS;
			}
			continue;
		}

		//if the trait is selected
		if( gfSkillTraitQuestions[ uiCnt ] )
		{
			//Display the gold background box
			BltVideoObject(FRAME_BUFFER, hImageHandle, 1, usBoxPosX, usBoxPosY, VO_BLT_SRCTRANSPARENCY,NULL);
		}
		else
		{
			//Display the grey background box
			BltVideoObject(FRAME_BUFFER, hImageHandle, 0,usBoxPosX, usBoxPosY, VO_BLT_SRCTRANSPARENCY,NULL);
		}

		//draw the text to the screenx
		DrawTextToScreen( gzIMPSkillTraitsText[ uiCnt ], usPosX, usPosY, 0, IMP_SKILL_TRAIT__FONT, IMP_SKILL_TRAIT__COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );

		//Determine the next x location
		if( uiCnt < IMP_SKILL_TRAIT__SKILL_TRAIT_TO_START_RIGHT_COL )
			usPosX = IMP_SKILL_TRAIT__LEFT_COLUMN_START_X + IMP_SKILL_TRAIT__TEXT_OFFSET_X;
		else
			usPosX = IMP_SKILL_TRAIT__RIGHT_COLUMN_START_X + IMP_SKILL_TRAIT__TEXT_OFFSET_X;

		//Determine the next Y location
		if( uiCnt == IMP_SKILL_TRAIT__SKILL_TRAIT_TO_START_RIGHT_COL )
			usPosY = IMP_SKILL_TRAIT__RIGHT_COLUMN_START_Y + IMP_SKILL_TRAIT__TEXT_OFFSET_Y;
		else
			usPosY += IMP_SKILL_TRAIT__SPACE_BTN_BUTTONS;

		//if its the none button
		if( uiCnt == IMP_SKILL_TRAITS__NONE-1 )
		{
			usPosX = IMP_SKILL_TRAIT__NONE_BTN_LOC_X + IMP_SKILL_TRAIT__TEXT_OFFSET_X;
		}
	}
}


void BtnIMPSkillTraitFinishCallback(GUI_BUTTON *btn,INT32 reason)
{
	// btn callback for IMP personality quiz answer button
	if (!(btn->uiFlags & BUTTON_ENABLED))
		return;

	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags|=(BUTTON_CLICKED_ON);

		//if we are just reviewing the page
		if( iCurrentProfileMode == 5 )
		{
			//go back tot he done screen
			iCurrentImpPage = IMP_FINISH;
		}
		else
		{
			iCurrentImpPage = IMP_MAIN_PAGE;

			if( CameBackToSpecialtiesPageButNotFinished() )
			{
			}
			else
			{
				//We are finished on this page
				iCurrentProfileMode = 2;
			}
		}
	}
}


BOOLEAN CameBackToSpecialtiesPageButNotFinished()
{
	//if we are in a page that comes after this one
	if( iCurrentProfileMode == 2 )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}





INT8	DoesPlayerHaveExtraAttibutePointsToDistributeBasedOnSkillSelection()
{
	INT8		bExtraPoints=10;
	INT8		bNumSkills=0;

	//Count the number of skills selected
	bNumSkills = CountNumSkillStraitsSelected( FALSE );

	//if there NONE selected
	if( bNumSkills == 0 )
	{
		bExtraPoints = 10;
	}

	//if there is 1 trait selected
	else if( bNumSkills == 1 )
	{
		//if the trait is not the ones you cant be expert in )
		if( gfSkillTraitQuestions[ IMP_SKILL_TRAITS__ELECTRONICS ] ||
				gfSkillTraitQuestions[ IMP_SKILL_TRAITS__AMBIDEXTROUS  ] ||
				gfSkillTraitQuestions[ IMP_SKILL_TRAITS__CAMO ] )
		{
			bExtraPoints = 5;
		}
		else
		{
			bExtraPoints = 0;
		}
	}

	//else both traits selected
	else
	{
		bExtraPoints = 0;
	}

	return( bExtraPoints );
}



BOOLEAN ShouldTraitBeSkipped( UINT32 uiTrait )
{
	if( uiTrait == IMP_SKILL_TRAITS__MARTIAL_ARTS && !fCharacterIsMale )
		return( TRUE );
	else
		return( FALSE );
}


void AddSelectedSkillsToSkillsList()
{
	UINT32	uiCnt;

	//loop through all the buttons and reset them
	for( uiCnt=0; uiCnt<IMP_SKILL_TRAITS__NONE; uiCnt++ )
	{
		//if the trait is selected
		if( gfSkillTraitQuestions[ uiCnt ] )
		{
			//switch on the trait, and add it to the list
			switch( uiCnt )
			{
				case IMP_SKILL_TRAITS__LOCKPICK:
					AddSkillToSkillList( LOCKPICKING );
					break;

				case IMP_SKILL_TRAITS__HAND_2_HAND:
					AddSkillToSkillList( HANDTOHAND );
					break;

				case IMP_SKILL_TRAITS__ELECTRONICS:
					AddSkillToSkillList( ELECTRONICS );
					break;
				
				case IMP_SKILL_TRAITS__NIGHT_OPS:
					AddSkillToSkillList( NIGHTOPS );
					break;
				
				case IMP_SKILL_TRAITS__THROWING:
					AddSkillToSkillList( THROWING );
					break;
				
				case IMP_SKILL_TRAITS__TEACHING:
					AddSkillToSkillList( TEACHING );
					break;
				
				case IMP_SKILL_TRAITS__HEAVY_WEAPONS:
					AddSkillToSkillList( HEAVY_WEAPS );
					break;
				
				case IMP_SKILL_TRAITS__AUTO_WEAPONS:
					AddSkillToSkillList( AUTO_WEAPS );
					break;
				
				case IMP_SKILL_TRAITS__STEALTHY:
					AddSkillToSkillList( STEALTHY );
					break;
				
				case IMP_SKILL_TRAITS__AMBIDEXTROUS:
					AddSkillToSkillList( AMBIDEXT );
					break;
				
				case IMP_SKILL_TRAITS__KNIFING:
					AddSkillToSkillList( KNIFING );
					break;
				
				case IMP_SKILL_TRAITS__ROOFTOP_SNIPING:
					AddSkillToSkillList( ONROOF );
					break;
				
				case IMP_SKILL_TRAITS__CAMO:
					AddSkillToSkillList( CAMOUFLAGED );
					break;
				
				case IMP_SKILL_TRAITS__MARTIAL_ARTS:
					AddSkillToSkillList( MARTIALARTS );
					break;
			}
		}
	}
}


void HandleLastSelectedTraits( INT8 bNewTrait )
{
	INT8	bTemp=-1;
	//if there are none selected
	if( gbLastSelectedTraits[ 0 ] == -1 )
	{
		gbLastSelectedTraits[ 0 ] = bNewTrait;
	}

	//if the second one is not selected
	else if( gbLastSelectedTraits[ 1 ] == -1 )
	{
		gbLastSelectedTraits[ 1 ] = gbLastSelectedTraits[ 0 ];
		gbLastSelectedTraits[ 0 ] = bNewTrait;
	}

	else
	{
		//unselect old trait
		gfSkillTraitQuestions[ gbLastSelectedTraits[ 1 ] ] = FALSE;

		gbLastSelectedTraits[ 1 ] = gbLastSelectedTraits[ 0 ];
		gbLastSelectedTraits[ 0 ] = bNewTrait;
	}
}
