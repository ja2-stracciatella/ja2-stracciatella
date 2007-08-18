#include "CharProfile.h"
#include "Font.h"
#include "IMP_Attribute_Selection.h"
#include "IMPVideoObjects.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Input.h"
#include "Isometric_Utils.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "IMP_Text_System.h"
#include "IMP_Compile_Character.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "ScreenIDs.h"
#include "Font_Control.h"


// width of the slider bar region
#define BAR_WIDTH 423 - 197

// width of the slider bar itself
#define SLIDER_BAR_WIDTH 37

// the sizeof one skill unit on the sliding bar in pixels
#define BASE_SKILL_PIXEL_UNIT_SIZE ( ( 423 - 230 ) )

enum{
	HEALTH_ATTRIBUTE,
	DEXTERITY_ATTRIBUTE,
	AGILITY_ATTRIBUTE,
	STRENGTH_ATTRIBUTE,
	WISDOM_ATTRIBUTE,
	LEADERSHIP_ATTRIBUTE,
	MARKSMANSHIP_SKILL,
	EXPLOSIVE_SKILL,
	MEDICAL_SKILL,
	MECHANICAL_SKILL,
};


enum
{
	SLIDER_ERROR,
	SLIDER_OK,
	SLIDER_OUT_OF_RANGE,
};

// the skills as they stand
INT32 iCurrentStrength    = 55;
INT32 iCurrentAgility     = 55;
INT32 iCurrentDexterity   = 55;
INT32 iCurrentHealth      = 55;
INT32 iCurrentLeaderShip  = 55;
INT32 iCurrentWisdom      = 55;
INT32 iCurrentMarkmanship = 55;
INT32 iCurrentMechanical  = 55;
INT32 iCurrentMedical     = 55;
INT32 iCurrentExplosives  = 55;

// which stat is message about stat at zero about
INT32 iCurrentStatAtZero = 0;

// total number of bonus points
INT32 iCurrentBonusPoints = 40;

// diplsay the 0 skill point warning..if skill set to 0, warn character
BOOLEAN fSkillAtZeroWarning = FALSE;

// is the sliding of the sliding bar active right now?
BOOLEAN fSlideIsActive = TRUE;

// first time in game
BOOLEAN fFirstIMPAttribTime = TRUE;

// review mode
BOOLEAN fReviewStats = FALSE;

// buttons
UINT32 giIMPAttributeSelectionButton[ 1 ];
UINT32 giIMPAttributeSelectionButtonImage[ 1 ];

// slider buttons
UINT32 giIMPAttributeSelectionSliderButton[ 20 ];
UINT32 giIMPAttributeSelectionSliderButtonImage[ 20 ];

// mouse regions
MOUSE_REGION pSliderRegions[ 10 ];


//The currently "anchored scroll bar"
MOUSE_REGION *gpCurrentScrollBox = NULL;
INT32					giCurrentlySelectedStat = -1;

// has any of the sliding bars moved?...for re-rendering puposes
BOOLEAN fHasAnySlidingBarMoved = FALSE;

INT32 uiBarToReRender = -1;

// are we actually coming back to edit, or are we restarting?
BOOLEAN fReturnStatus = FALSE;

void SetAttributes( void );
void DrawBonusPointsRemaining( void );
void SetGeneratedCharacterAttributes( void );


static void CreateAttributeSliderButtons(void);
static void CreateIMPAttributeSelectionButtons(void);
static void CreateSlideRegionMouseRegions(void);


void EnterIMPAttributeSelection( void )
{


	// set attributes and skills
	if( ( fReturnStatus == FALSE ) && ( fFirstIMPAttribTime == TRUE ) )
	{
		// re starting
    SetAttributes( );

		gpCurrentScrollBox = NULL;
		giCurrentlySelectedStat = -1;

	  // does character have PROBLEMS!!?!?!
		/*
	  if( DoesCharacterHaveAnAttitude() )
		{
      iCurrentBonusPoints+= 10;
		}
	  if( DoesCharacterHaveAPersoanlity( ) )
		{
		  iCurrentBonusPoints += 10;
		}
		*/
	}
  fReturnStatus = TRUE;
	fFirstIMPAttribTime = FALSE;

	// create done button
	CreateIMPAttributeSelectionButtons( );
	// create clider buttons
	CreateAttributeSliderButtons( );

	// the mouse regions
	CreateSlideRegionMouseRegions( );

	// render background
  RenderIMPAttributeSelection( );
}


void RenderIMPAttributeSelection( void )
{
   // the background
	RenderProfileBackGround( );


	// attribute frame
	RenderAttributeFrame( 51, 87 );

	// render attribute boxes
	RenderAttributeBoxes( );

	RenderAttrib1IndentFrame(51, 30 );

	if( fReviewStats != TRUE )
	{
	  RenderAttrib2IndentFrame(350, 42 );
	}

	// reset rerender flag
	fHasAnySlidingBarMoved = FALSE;

	// print text for screen
	PrintImpText( );

	// amt of bonus pts
	DrawBonusPointsRemaining( );
}


static void DestroyAttributeSliderButtons(void);
static void DestroyIMPAttributeSelectionButtons(void);
static void DestroySlideRegionMouseRegions(void);


void ExitIMPAttributeSelection( void )
{
  // get rid of slider buttons
	DestroyAttributeSliderButtons( );

	// the mouse regions
	DestroySlideRegionMouseRegions( );

	// get rid of done buttons
  DestroyIMPAttributeSelectionButtons( );

	fReturnStatus = FALSE;
}


static UINT8 DecrementStat(INT32 iStatToDecrement);
static INT32 GetCurrentAttributeValue(INT32 iAttribute);
static UINT8 IncrementStat(INT32 iStatToIncrement);
static void ProcessAttributes(void);
static void StatAtZeroBoxCallBack(UINT8 bExitValue);


void HandleIMPAttributeSelection( void )
{

	// review mode, do not allow changes
  if( fReviewStats )
	{
		return;
	}

	// set the currently selectd slider bar
	if( gfLeftButtonState && gpCurrentScrollBox != NULL )
	{
		//if theuser is holding down the mouse cursor to left of the start of the slider bars
		if( gusMouseXPos < ( SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X ) )
		{
			DecrementStat( giCurrentlySelectedStat );
		}

		//else if the user is holding down the mouse button to the right of the scroll bars
		else if( gusMouseXPos > ( LAPTOP_SCREEN_UL_X + SKILL_SLIDE_START_X + BAR_WIDTH  ) )
		{
			IncrementStat( giCurrentlySelectedStat );
		}
		else
		{
			INT32		iCurrentAttributeValue;
			INT32		sNewX = gusMouseXPos;
			INT32		iNewValue;
			INT32		iCounter;


			// get old stat value
		  iCurrentAttributeValue = GetCurrentAttributeValue( giCurrentlySelectedStat );
			sNewX = sNewX - ( SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X );
      iNewValue = ( sNewX * 50 ) / BASE_SKILL_PIXEL_UNIT_SIZE + 35;

			// chenged, move mouse region if change large enough
      if( iCurrentAttributeValue != iNewValue )
			{
         // update screen
         fHasAnySlidingBarMoved = TRUE;
			}

      // change is enough
      if ( iNewValue - iCurrentAttributeValue > 0)
			{
				// positive, increment stat
				for (iCounter = iNewValue - iCurrentAttributeValue; iCounter > 0; iCounter--)
				{
					IncrementStat( giCurrentlySelectedStat );
				}
			}
			else
			{
				// negative, decrement stat
				for (iCounter = iCurrentAttributeValue - iNewValue; iCounter > 0; iCounter--)
				{
					DecrementStat( giCurrentlySelectedStat );
				}
			}
		}

		RenderIMPAttributeSelection( );
	}
	else
	{
		gpCurrentScrollBox = NULL;
		giCurrentlySelectedStat = -1;
	}


	// prcoess current state of attributes
	ProcessAttributes( );

	// has any bar moved?
	if( fHasAnySlidingBarMoved )
	{
		// render
		if( uiBarToReRender == -1 )
		{
			RenderIMPAttributeSelection( );
		}
		else
		{

			RenderAttributeFrameForIndex( 51, 87, uiBarToReRender );
			/*
			// print text for screen
			PrintImpText( );

			// amt of bonus pts
			DrawBonusPointsRemaining( );

			RenderAttributeFrame( 51, 87 );

			// render attribute boxes
			RenderAttributeBoxes( );

			PrintImpText( );

			InvalidateRegion( LAPTOP_SCREEN_UL_X + 51, LAPTOP_SCREEN_WEB_UL_Y + 87, LAPTOP_SCREEN_UL_X + 51 + 400, LAPTOP_SCREEN_WEB_UL_Y + 87 + 220 );
*/
			uiBarToReRender = -1;
			MarkButtonsDirty();
		}

		fHasAnySlidingBarMoved = FALSE;

	}
	if ( fSkillAtZeroWarning == TRUE )
	{
		DoLapTopMessageBox( MSG_BOX_IMP_STYLE, pSkillAtZeroWarning[ 0 ], LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO, StatAtZeroBoxCallBack);
		fSkillAtZeroWarning = FALSE;

	}
}


static void ProcessAttributes(void)
{

	// this function goes through and confirms thet state of attributes, ie not allowing attributes to
	// drop below 35 or skills to go below 0...and if skill is 34 set to 0

	// check any attribute below 35

	// strength
	if( iCurrentStrength <= 35 )
	{
		iCurrentStrength = 35;
		// disable button too


	}

  // dex
	if( iCurrentDexterity <= 35 )
	{
		iCurrentDexterity = 35;
		// disable button too


	}

   // agility
	if( iCurrentAgility <= 35 )
	{
		iCurrentAgility = 35;
		// disable button too


	}

	 // wisdom
	if( iCurrentWisdom <= 35 )
	{
		iCurrentWisdom = 35;
		// disable button too


	}

   // leadership
	if( iCurrentLeaderShip <= 35 )
	{
		iCurrentLeaderShip = 35;
		// disable button too


	}

	 // health
	if( iCurrentHealth <= 35 )
	{
		iCurrentHealth = 35;
		// disable button too

	}




	// now check for above 85
  // strength
	if( iCurrentStrength >= 85 )
	{
		iCurrentStrength = 85;
		// disable button too


	}

  // dex
	if( iCurrentDexterity >= 85 )
	{
		iCurrentDexterity = 85;
		// disable button too


	}

   // agility
	if( iCurrentAgility >= 85 )
	{
		iCurrentAgility = 85;
		// disable button too


	}

	 // wisdom
	if( iCurrentWisdom >= 85 )
	{
		iCurrentWisdom = 85;
		// disable button too


	}

   // leadership
	if( iCurrentLeaderShip >= 85 )
	{
		iCurrentLeaderShip = 85;
		// disable button too


	}

	 // health
	if( iCurrentHealth >= 85 )
	{
		iCurrentHealth = 85;
		// disable button too

	}
}


static BOOLEAN DoWeHaveThisManyBonusPoints(INT32 iBonusPoints);


static UINT8 IncrementStat(INT32 iStatToIncrement)
{
  // this function is responsable for incrementing a stat

	// review mode, do not allow changes
  if( fReviewStats )
	{
		return( SLIDER_ERROR );
	}

	// make sure we have enough bonus points
	if( iCurrentBonusPoints < 1 )
	{
		// nope...GO GET SOME BONUS POINTS, IDIOT!
		return( SLIDER_ERROR );
	}

	// check to make sure stat isn't maxed out already
	switch( iStatToIncrement )
	{
		case( STRENGTH_ATTRIBUTE ):
		  if( iCurrentStrength > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				iCurrentStrength++;
			  iCurrentBonusPoints--;
			}
		break;
    case( DEXTERITY_ATTRIBUTE ):
		  if( iCurrentDexterity > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				iCurrentDexterity++;
			  iCurrentBonusPoints--;
			}
		break;
		case( AGILITY_ATTRIBUTE ):
		  if( iCurrentAgility > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				iCurrentAgility++;
			  iCurrentBonusPoints--;
			}
		break;
		case( LEADERSHIP_ATTRIBUTE ):
		  if( iCurrentLeaderShip > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				iCurrentLeaderShip++;
			  iCurrentBonusPoints--;
			}
		break;
		case( WISDOM_ATTRIBUTE ):
		  if( iCurrentWisdom > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				iCurrentWisdom++;
			  iCurrentBonusPoints--;
			}
		break;
		case( HEALTH_ATTRIBUTE ):
		  if( iCurrentHealth > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				iCurrentHealth++;
			  iCurrentBonusPoints--;
			}
		break;
		case( MARKSMANSHIP_SKILL ):
		  if( iCurrentMarkmanship > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				if( iCurrentMarkmanship == 0)
				{
					if( DoWeHaveThisManyBonusPoints( 15 ) == TRUE )
					{
					  iCurrentMarkmanship+=35;
					  iCurrentBonusPoints-=15;
						fSkillAtZeroWarning = FALSE;
					}
					else
					{
						return( SLIDER_OK );
					}
				}
				else
				{
				  iCurrentMarkmanship++;
			    iCurrentBonusPoints--;
				}
			}
		break;
		case( MECHANICAL_SKILL ):
		  if( iCurrentMechanical > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				if( iCurrentMechanical == 0)
				{
					if( DoWeHaveThisManyBonusPoints( 15 ) == TRUE )
					{
					  iCurrentMechanical+=35;
					  iCurrentBonusPoints-=15;
						fSkillAtZeroWarning = FALSE;
					}
					else
					{
						return( SLIDER_OK );
					}
				}
				else
				{
				  iCurrentMechanical++;
			    iCurrentBonusPoints--;
				}
			}
		break;
		case( MEDICAL_SKILL ):
		  if( iCurrentMedical > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
			if( iCurrentMedical == 0)
				{
					if( DoWeHaveThisManyBonusPoints( 15 ) == TRUE )
					{
					  iCurrentMedical+=35;
					  iCurrentBonusPoints-=15;
						fSkillAtZeroWarning = FALSE;
					}
					else
					{
						return( SLIDER_OK );
					}
				}
				else
				{
				  iCurrentMedical++;
			    iCurrentBonusPoints--;
				}
			}
		break;
		case( EXPLOSIVE_SKILL ):
		  if( iCurrentExplosives > 84 )
			{
				// too high, leave
				return( SLIDER_OUT_OF_RANGE );
      }
		  else
			{
				if( iCurrentExplosives == 0)
				{
					if( DoWeHaveThisManyBonusPoints( 15 ) == TRUE )
					{
					  iCurrentExplosives+=35;
					  iCurrentBonusPoints-=15;
						fSkillAtZeroWarning = FALSE;
					}
					else
					{
						return( SLIDER_OK );
					}
				}
				else
				{
				  iCurrentExplosives++;
			    iCurrentBonusPoints--;
				}
			}
		break;

	}

	return( SLIDER_OK );
}


static UINT8 DecrementStat(INT32 iStatToDecrement)
{

	// review mode, do not allow changes
  if( fReviewStats )
	{
		return( SLIDER_ERROR );
	}

  // decrement a stat
	// check to make sure stat isn't maxed out already
	switch( iStatToDecrement )
	{
		case( STRENGTH_ATTRIBUTE ):
			if( iCurrentStrength > 35 )
			{
				// ok to decrement
				iCurrentStrength--;
        iCurrentBonusPoints++;
			}
			else
			{
				return( SLIDER_OUT_OF_RANGE );
			}
		break;
    case( DEXTERITY_ATTRIBUTE ):
			if( iCurrentDexterity > 35 )
			{
				// ok to decrement
				iCurrentDexterity--;
        iCurrentBonusPoints++;
			}
			else
			{
				return( SLIDER_OUT_OF_RANGE );
			}
		break;
		case( AGILITY_ATTRIBUTE ):
			if( iCurrentAgility > 35 )
			{
				// ok to decrement
				iCurrentAgility--;
        iCurrentBonusPoints++;
			}
			else
			{
				return( SLIDER_OUT_OF_RANGE );
			}
		break;
    case( WISDOM_ATTRIBUTE ):
			if( iCurrentWisdom > 35 )
			{
				// ok to decrement
				iCurrentWisdom--;
        iCurrentBonusPoints++;
			}
			else
			{
				return( SLIDER_OUT_OF_RANGE );
			}
		break;
		case( LEADERSHIP_ATTRIBUTE ):
			if( iCurrentLeaderShip > 35 )
			{
				// ok to decrement
				iCurrentLeaderShip--;
        iCurrentBonusPoints++;
			}
			else
			{
				return( SLIDER_OUT_OF_RANGE );
			}
		break;
		case( HEALTH_ATTRIBUTE ):
			if( iCurrentHealth > 35 )
			{
				// ok to decrement
				iCurrentHealth--;
        iCurrentBonusPoints++;
			}
			else
			{
				return( SLIDER_OUT_OF_RANGE );
			}
		break;
    case( MARKSMANSHIP_SKILL ):
			if( iCurrentMarkmanship > 35 )
			{
				// ok to decrement
				iCurrentMarkmanship--;
        iCurrentBonusPoints++;
			}
			else if( iCurrentMarkmanship == 35)
			{
        	// ok to decrement
				iCurrentMarkmanship-=35;
        iCurrentBonusPoints+=15;
				fSkillAtZeroWarning = TRUE;
			}
		break;
		case( MEDICAL_SKILL ):
			if( iCurrentMedical > 35 )
			{
				// ok to decrement
				iCurrentMedical--;
        iCurrentBonusPoints++;
			}
			else if( iCurrentMedical == 35)
			{
        	// ok to decrement
				iCurrentMedical-=35;
        iCurrentBonusPoints+=15;
				fSkillAtZeroWarning = TRUE;
			}
		break;
		case( MECHANICAL_SKILL ):
			if( iCurrentMechanical > 35 )
			{
				// ok to decrement
				iCurrentMechanical--;
        iCurrentBonusPoints++;
			}
			else if( iCurrentMechanical == 35)
			{
        	// ok to decrement
				iCurrentMechanical-=35;
        iCurrentBonusPoints+=15;
				fSkillAtZeroWarning = TRUE;
			}
		break;
		case( EXPLOSIVE_SKILL ):
			if( iCurrentExplosives > 35 )
			{
				// ok to decrement
				iCurrentExplosives--;
        iCurrentBonusPoints++;
			}
			else if( iCurrentExplosives == 35)
			{
        	// ok to decrement
				iCurrentExplosives-=35;
        iCurrentBonusPoints+=15;
				fSkillAtZeroWarning = TRUE;
			}
		break;


	}

	if( fSkillAtZeroWarning == TRUE )
	{
		// current stat at zero
		iCurrentStatAtZero = iStatToDecrement;
	}

	return( SLIDER_OK );
}


static BOOLEAN DoWeHaveThisManyBonusPoints(INT32 iBonusPoints)
{

	// returns if player has at least this many bonus points
	if( iCurrentBonusPoints >= iBonusPoints )
	{
		// yep, return true
		return ( TRUE );
	}
	else
	{
		// nope, return false
		return ( FALSE );
	}
}


static void BtnIMPAttributeFinishCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateIMPAttributeSelectionButtons(void)
{

	// the finished button
  giIMPAttributeSelectionButtonImage[0]=  LoadButtonImage( "LAPTOP/button_2.sti" ,-1,0,-1,1,-1 );
/*	giIMPAttributeSelectionButton[0] = QuickCreateButton( giIMPAttributeSelectionButtonImage[0], LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPAttributeFinishCallback );
  */
	giIMPAttributeSelectionButton[0] = CreateIconAndTextButton( giIMPAttributeSelectionButtonImage[0], pImpButtonText[ 11 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X +  ( 136 ), LAPTOP_SCREEN_WEB_UL_Y + ( 314 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 	BtnGenericMouseMoveButtonCallback, BtnIMPAttributeFinishCallback);



	SetButtonCursor(giIMPAttributeSelectionButton[0], CURSOR_WWW);
}


static void DestroyIMPAttributeSelectionButtons(void)
{
	// this function will destroy the buttons needed for the IMP attrib enter page

	// the begin  button
  RemoveButton(giIMPAttributeSelectionButton[ 0 ] );
  UnloadButtonImage(giIMPAttributeSelectionButtonImage[ 0 ] );
}


static void BtnIMPAttributeFinishCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		//are we done diting, or just reviewing the stats?
		if (fReviewStats)
		{
			iCurrentImpPage = IMP_FINISH;
		}
		else
		{
			iCurrentImpPage = IMP_ATTRIBUTE_FINISH;
		}
		fButtonPendingFlag = TRUE;
	}
}


void RenderAttributeBoxes(void)
{
	// this function will render the boxes in the sliding attribute bar, based on position
	SetFont(FONT10ARIAL);
	SetFontShadow(NO_SHADOW);
	SetFontForeground(FONT_WHITE);
	SetFontBackground(FONT_BLACK);

	// run through and render each slider bar
	for (INT32 i = HEALTH_ATTRIBUTE; i <= MECHANICAL_SKILL; ++i)
	{
		INT32 val;
		switch (i)
		{
			case HEALTH_ATTRIBUTE:     val = iCurrentHealth;      break;
			case DEXTERITY_ATTRIBUTE:  val = iCurrentDexterity;   break;
			case AGILITY_ATTRIBUTE:    val = iCurrentAgility;     break;
			case STRENGTH_ATTRIBUTE:   val = iCurrentStrength;    break;
			case WISDOM_ATTRIBUTE:     val = iCurrentWisdom;      break;
			case LEADERSHIP_ATTRIBUTE: val = iCurrentLeaderShip;  break;
			case MARKSMANSHIP_SKILL:   val = iCurrentMarkmanship; break;
			case EXPLOSIVE_SKILL:      val = iCurrentExplosives;  break;
			case MEDICAL_SKILL:        val = iCurrentMedical;     break;
			case MECHANICAL_SKILL:     val = iCurrentMechanical;  break;
		}

		// Compensate for zeroed skills: x pos is at least 0
		INT16 sX = max(0, val - 35) * BASE_SKILL_PIXEL_UNIT_SIZE / 50;
		INT16 sY = SKILL_SLIDE_START_Y + SKILL_SLIDE_HEIGHT * i;

		sX += SKILL_SLIDE_START_X;
		RenderSliderBar(sX, sY);

		sX += LAPTOP_SCREEN_UL_X;
		sY += LAPTOP_SCREEN_WEB_UL_Y;
		mprintf(sX + 13, sY + 3, L"%d", val);
	}

	SetFontShadow(DEFAULT_SHADOW);
}


static void BtnIMPAttributeSliderLeftCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnIMPAttributeSliderRightCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateAttributeSliderButtons(void)
{
	// this function will create the buttons for the attribute slider
  // the finished button
  INT32 iCounter =0;

	giIMPAttributeSelectionSliderButtonImage[ 0 ]=  LoadButtonImage( "LAPTOP/AttributeArrows.sti" ,-1,0,-1,1,-1 );
  giIMPAttributeSelectionSliderButtonImage[ 1 ]=  LoadButtonImage( "LAPTOP/AttributeArrows.sti" ,-1,3,-1,4,-1 );

	for(iCounter = 0; iCounter < 20; iCounter+=2 )
	{
	  // left button - decrement stat
	 giIMPAttributeSelectionSliderButton[ iCounter ] = QuickCreateButton( giIMPAttributeSelectionSliderButtonImage[ 0 ], LAPTOP_SCREEN_UL_X +  ( 163 ), ( INT16 ) ( LAPTOP_SCREEN_WEB_UL_Y + ( 99 + iCounter / 2 * 20 ) ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPAttributeSliderLeftCallback );

    // right button - increment stat
	 giIMPAttributeSelectionSliderButton[ iCounter + 1 ] = QuickCreateButton( giIMPAttributeSelectionSliderButtonImage[ 1 ], LAPTOP_SCREEN_UL_X +  ( 419 ), ( INT16 ) ( LAPTOP_SCREEN_WEB_UL_Y + ( 99 + iCounter / 2 * 20 ) ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPAttributeSliderRightCallback );

		SetButtonCursor(giIMPAttributeSelectionSliderButton[iCounter], CURSOR_WWW);
		SetButtonCursor(giIMPAttributeSelectionSliderButton[iCounter + 1 ], CURSOR_WWW);
		// set user data
		MSYS_SetBtnUserData(giIMPAttributeSelectionSliderButton[iCounter], iCounter / 2);
		MSYS_SetBtnUserData(giIMPAttributeSelectionSliderButton[iCounter + 1], iCounter / 2);
	}

	MarkButtonsDirty( );
}


static void DestroyAttributeSliderButtons(void)
{
  // this function will destroy the buttons used for attribute manipulation
  INT32 iCounter =0;

  // get rid of image
	UnloadButtonImage(giIMPAttributeSelectionSliderButtonImage[ 0 ] );
	UnloadButtonImage(giIMPAttributeSelectionSliderButtonImage[ 1 ] );

	for(iCounter = 0; iCounter < 20; iCounter++ )
	{
		// get rid of button
     RemoveButton(giIMPAttributeSelectionSliderButton[ iCounter ] );

	}
}


static void BtnIMPAttributeSliderLeftCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN ||
			reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
		INT32 iValue = MSYS_GetBtnUserData(btn);

		DecrementStat(iValue);
		fHasAnySlidingBarMoved = TRUE;
		uiBarToReRender = iValue;
	}
}


static void BtnIMPAttributeSliderRightCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN ||
			reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
  	INT32 iValue = MSYS_GetBtnUserData(btn);

		IncrementStat(iValue);
		fHasAnySlidingBarMoved = TRUE;
		uiBarToReRender = iValue;
	}
}


static void SliderRegionButtonCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateSlideRegionMouseRegions(void)
{
	// this function will create that mouse regions on the sliding area, that, if the player clicks on, the bar will automatically jump to
  INT32 iCounter = 0;

	for( iCounter = 0; iCounter < 10; iCounter++ )
	{
		 // define the region
		 MSYS_DefineRegion( &pSliderRegions[ iCounter ], ( INT16 ) ( SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X), ( INT16 ) ( LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + iCounter * SKILL_SLIDE_HEIGHT ) , ( INT16 ) ( LAPTOP_SCREEN_UL_X + SKILL_SLIDE_START_X + BAR_WIDTH ), ( INT16 ) ( LAPTOP_SCREEN_WEB_UL_Y + SKILL_SLIDE_START_Y + iCounter * SKILL_SLIDE_HEIGHT + 15 ),
			MSYS_PRIORITY_HIGH + 2, CURSOR_WWW, MSYS_NO_CALLBACK, SliderRegionButtonCallback);

		 // define user data
		 	MSYS_SetRegionUserData(&pSliderRegions[iCounter],0,iCounter);
	}
}


static void DestroySlideRegionMouseRegions(void)
{
  // this function will destroy the regions user for the slider ' jumping'
	INT32 iCounter=0;

	// delete the regions
	for( iCounter=0; iCounter < 10; iCounter++ )
	{
	 MSYS_RemoveRegion( &pSliderRegions[ iCounter ] );
	}
}


static void SliderRegionButtonCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
  INT32 iCurrentAttributeValue = 0;
  INT32 iNewAttributeValue = 0;
  INT32 iAttributeDelta = 0;
  INT32 iCounter =0;
  INT16 sX =0;
	static INT16 sOldX = -1;
	static INT32 iAttribute = -1;
	INT32 iNewValue = 0;
  INT16 sNewX = -1;


	//if we already have an anchored slider bar
	if( gpCurrentScrollBox != pRegion && gpCurrentScrollBox != NULL )
		return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
  {
		if( fSlideIsActive == FALSE)
		{
			// not active leave
      return;
		}


	  // check to see if we have moved
	  if( MSYS_GetRegionUserData(pRegion, 0) != iAttribute )
		{
			// different regions
			iAttribute = MSYS_GetRegionUserData(pRegion, 0);
		  sOldX = -1;
      return;
		}

		uiBarToReRender = iAttribute;

		giCurrentlySelectedStat = iAttribute;
		gpCurrentScrollBox = pRegion;

    // get new attribute value x
		sNewX = pRegion->MouseXPos;

		// sOldX has been reset, set to sNewX
		if( sOldX == -1)
		{
			sOldX = sNewX;
		  return;
		}
		// check against old x
		if( sNewX != sOldX )
		{

			// get old stat value
		  iCurrentAttributeValue = GetCurrentAttributeValue( iAttribute );
			sNewX = sNewX - ( SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X );
      iNewValue = ( sNewX * 50 ) / BASE_SKILL_PIXEL_UNIT_SIZE + 35;

			// chenged, move mouse region if change large enough
      if( iCurrentAttributeValue != iNewValue )
			{
         // update screen
         fHasAnySlidingBarMoved = TRUE;
			}

      // change is enough
      if ( iNewValue - iCurrentAttributeValue > 0)
			{
				// positive, increment stat
				for (iCounter = iNewValue - iCurrentAttributeValue; iCounter > 0; iCounter--)
				{
					IncrementStat( iAttribute );
				}
			}
			else
			{
				// negative, decrement stat
				for (iCounter = iCurrentAttributeValue - iNewValue; iCounter > 0; iCounter--)
				{
					DecrementStat( iAttribute );
				}
			}

			sOldX = sNewX;
		}
	}
  else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {

		if( fSlideIsActive )
		{
      // reset slide is active flag
		  fSlideIsActive = FALSE;
			return;
		}


		// get mouse XY

		sX = pRegion -> MouseXPos;

    // which region are we in?

		// get attribute
		iAttribute = MSYS_GetRegionUserData(pRegion, 0);
    uiBarToReRender = iAttribute;

		// get value of attribute
		iCurrentAttributeValue = GetCurrentAttributeValue( iAttribute );

		// set the new attribute value based on position of mouse click
    iNewAttributeValue = ( ( sX - SKILL_SLIDE_START_X ) * 50 ) / BASE_SKILL_PIXEL_UNIT_SIZE;

		// too high, reset to 85
		if( iNewAttributeValue > 85 )
    {
			iNewAttributeValue = 85;
		}

		// get the delta
		iAttributeDelta = iCurrentAttributeValue - iNewAttributeValue;

		// set Counter
		iCounter = iAttributeDelta;

		// check if increment or decrement
		if( iAttributeDelta > 0)
		{
			// decrement
			for( iCounter = 0; iCounter < iAttributeDelta; iCounter++ )
			{
        DecrementStat( iAttribute );
			}
		}
		else
		{
			// increment attribute
      for( iCounter = iAttributeDelta; iCounter < 0; iCounter++ )
			{
				if( iCurrentAttributeValue == 0)
				{
					iCounter = 0;
				}
        IncrementStat( iAttribute );
			}
		}

		// update screen
    fHasAnySlidingBarMoved = TRUE;
  }
	else if( iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{

		// get mouse positions
		sX = pRegion -> MouseXPos;

		// get attribute
		iAttribute = MSYS_GetRegionUserData(pRegion, 0);
    uiBarToReRender = iAttribute;

		// get value of attribute
		iCurrentAttributeValue = GetCurrentAttributeValue( iAttribute );

		// get the boxes bounding x
		sNewX =  ( ( iCurrentAttributeValue - 35 ) * BASE_SKILL_PIXEL_UNIT_SIZE )/ 50 + SKILL_SLIDE_START_X + LAPTOP_SCREEN_UL_X;

		// the sNewX is below 0, reset to zero
		if( sNewX < 0 )
		{
			sNewX = 0;
		}

		if( ( sX > sNewX )&&( sX < sNewX + SLIDER_BAR_WIDTH) )
		{
			// we are within the slide bar, set fact we want to drag and draw
       fSlideIsActive = TRUE;
		}
		else
		{
			// otherwise want to jump to position
			fSlideIsActive =FALSE;
		}
	}
}


static INT32 GetCurrentAttributeValue(INT32 iAttribute)
{
	// this function will get the value of the attribute that was passed to this fucntion via iAttribute
  INT32 iValue =0;

  switch( iAttribute )
	{
		case ( STRENGTH_ATTRIBUTE ):
			iValue = iCurrentStrength;
		break;
		case ( DEXTERITY_ATTRIBUTE ):
			iValue = iCurrentDexterity;
		break;
    case ( AGILITY_ATTRIBUTE ):
			iValue = iCurrentAgility;
		break;
    case ( HEALTH_ATTRIBUTE ):
			iValue = iCurrentHealth;
		break;
		case ( WISDOM_ATTRIBUTE ):
			iValue = iCurrentWisdom;
		break;
		case ( LEADERSHIP_ATTRIBUTE ):
			iValue = iCurrentLeaderShip;
		break;
		case ( MARKSMANSHIP_SKILL ):
			iValue = iCurrentMarkmanship;
		break;
		case ( MEDICAL_SKILL ):
			iValue = iCurrentMedical;
		break;
		case ( MECHANICAL_SKILL ):
			iValue = iCurrentMechanical;
		break;
		case ( EXPLOSIVE_SKILL ):
			iValue = iCurrentExplosives;
		break;


	}

	return iValue;
}


void SetAttributes( void )
{
/*
  // set attributes and skills based on what is in charprofile.c

	// attributes
	  iCurrentStrength = iStrength + iAddStrength;
		iCurrentDexterity = iDexterity + iAddDexterity;
		iCurrentHealth = iHealth + iAddHealth;
		iCurrentLeaderShip = iLeadership + iAddLeadership;
		iCurrentWisdom = iWisdom + iAddWisdom;
		iCurrentAgility = iAgility + iAddAgility;

	// skills
    iCurrentMarkmanship = iMarksmanship + iAddMarksmanship;
		iCurrentMechanical = iMechanical + iAddMechanical;
		iCurrentMedical = iMedical + iAddMedical;
		iCurrentExplosives = iExplosives + iAddExplosives;

		// reset bonus pts
    iCurrentBonusPoints = 40;
*/

	iCurrentStrength = 55;
	iCurrentDexterity = 55;
	iCurrentHealth = 55;
	iCurrentLeaderShip = 55;
	iCurrentWisdom = 55;
	iCurrentAgility = 55;

	// skills
	iCurrentMarkmanship = 55;
	iCurrentMechanical = 55;
	iCurrentMedical = 55;
	iCurrentExplosives = 55;


	// reset bonus pts
	iCurrentBonusPoints = 40;

	ResetIncrementCharacterAttributes( );
}


void DrawBonusPointsRemaining( void )
{
  // draws the amount of points remaining player has

	// just reviewing, don't blit stats
	if( fReviewStats == TRUE )
	{
    return;
	}

	// set font color
	SetFontForeground( FONT_WHITE );
  SetFontBackground( FONT_BLACK );
  SetFont( FONT12ARIAL );
	mprintf(LAPTOP_SCREEN_UL_X + 425, LAPTOP_SCREEN_WEB_UL_Y + 51, L"%d", iCurrentBonusPoints);

	InvalidateRegion( LAPTOP_SCREEN_UL_X + 425, LAPTOP_SCREEN_WEB_UL_Y + 51, LAPTOP_SCREEN_UL_X + 475, LAPTOP_SCREEN_WEB_UL_Y + 71 );
}


void SetGeneratedCharacterAttributes( void )
{
  // copies over the attributes of the player generated character
    iStrength = iCurrentStrength  ;
		iDexterity = iCurrentDexterity;
		iHealth = iCurrentHealth;
		iLeadership = iCurrentLeaderShip;
		iWisdom = iCurrentWisdom;
		iAgility = iCurrentAgility;

	// skills
    iMarksmanship = iCurrentMarkmanship;
		iMechanical = iCurrentMechanical;
		iMedical = iCurrentMedical;
		iExplosives = iCurrentExplosives;
}


static void StatAtZeroBoxCallBack(UINT8 bExitValue)
{
	// yes, so start over, else stay here and do nothing for now
  if( bExitValue == MSG_BOX_RETURN_YES )
	{
		MarkButtonsDirty();
	}
	else if( bExitValue == MSG_BOX_RETURN_NO )
	{
		IncrementStat( iCurrentStatAtZero );
		fHasAnySlidingBarMoved = TRUE;
		MarkButtonsDirty();
	}
}
