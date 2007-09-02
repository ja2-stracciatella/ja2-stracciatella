#include "Font.h"
#include "IMP_MainPage.h"
#include "CharProfile.h"
#include "IMPVideoObjects.h"
#include "Text.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Debug.h"
#include "Render_Dirty.h"
#include "Cursors.h"
#include "Laptop.h"
#include "Finances.h"
#include "IMP_Attribute_Selection.h"
#include "IMP_Finish.h"
#include "MouseSystem.h"
#include "Merc_Hiring.h"
#include "LaptopSave.h"
#include "IMP_Compile_Character.h"
#include "IMP_Portraits.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "ScreenIDs.h"
#include "Font_Control.h"


#define MAIN_PAGE_BUTTON_TEXT_WIDTH 95

// main page buttons
INT32 giIMPMainPageButton[6];
INT32 giIMPMainPageButtonImage[6];

extern INT32 iCurrentVoices;

// mouse regions for not entablable warning
MOUSE_REGION pIMPMainPageMouseRegions[ 4 ];

UINT32 guiCHARACTERPORTRAITFORMAINPAGE;

void BtnIMPMainPagePortraitCallback(GUI_BUTTON *btn, INT32 reason);
void BtnIMPMainPageVoiceCallback(GUI_BUTTON *btn, INT32 reason);


// this is the current state of profiling the player is in.
/*
  0 - Beginning
	1 - Personnality
	2 - Attributes and Skills
	3 - Portrait
	4 - Voice
	5 - Done
	*/
INT32 iCurrentProfileMode = 0;


static void CreateIMPMainPageButtons(void);
static void CreateMouseRegionsForIMPMainPageBasedOnCharGenStatus(void);
static BOOLEAN LoadCharacterPortraitForMainPage(void);
static void UpDateIMPMainPageButtons(void);


void EnterIMPMainPage( void )
{

	// turn off review mode
	fReviewStats = FALSE;


  // create buttons
	CreateIMPMainPageButtons( );

	// load portrait for face button, if applicable
	LoadCharacterPortraitForMainPage( );

	// create button masks for this screen
	CreateMouseRegionsForIMPMainPageBasedOnCharGenStatus( );

	// alter states
	UpDateIMPMainPageButtons( );

	// entry into IMP about us page
	RenderIMPMainPage( );
}


static void DeleteIMPMainPageButtons(void);
static void DestoryMouseRegionsForIMPMainPageBasedOnCharGenStatus(void);


void ExitIMPMainPage( void )
{
  // exit from IMP About us page

	// delete Buttons
	DeleteIMPMainPageButtons( );
	DestoryMouseRegionsForIMPMainPageBasedOnCharGenStatus( );
}


void RenderIMPMainPage( void )
{
  // rneders the IMP about us page

	// the background
	RenderProfileBackGround( );

	// the IMP symbol
	//RenderIMPSymbol( 106, 1 );
  // indent
	RenderMainIndentFrame( 164, 74 );
}


static BOOLEAN CheckIfFinishedCharacterGeneration(void);


void HandleIMPMainPage( void )
{
  // handles the IMP about main page

	if ( CheckIfFinishedCharacterGeneration( ) )
	{
    iCurrentImpPage = IMP_FINISH;
	}
}


static void BtnIMPMainPageAttributesCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnIMPMainPageBackCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnIMPMainPageBeginCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnIMPMainPagePersonalityCallback(GUI_BUTTON* btn, INT32 reason);


static void CreateIMPMainPageButtons(void)
{

  // this function will create the buttons needed for th IMP about us page
  CHAR16 sString[ 128 ];


	// the back button button
  giIMPMainPageButtonImage[0]=  LoadButtonImage( "LAPTOP/button_3.sti" ,-1,0,-1,1,-1 );


	/* giIMPMainPageButton[0] = QuickCreateButton( giIMPMainPageButtonImage[0], LAPTOP_SCREEN_UL_X + 10 , LAPTOP_SCREEN_WEB_UL_Y + ( 360 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPMainPageBackCallback);

		*/
		 giIMPMainPageButton[0] = CreateIconAndTextButton( giIMPMainPageButtonImage[0], pImpButtonText[ 19 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X + 15 , LAPTOP_SCREEN_WEB_UL_Y + ( 360 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnIMPMainPageBackCallback);


		 SpecifyButtonTextSubOffsets( giIMPMainPageButton[0], 0, -1, FALSE );

	// the begin profiling button
	giIMPMainPageButtonImage[1]=  LoadButtonImage( "LAPTOP/button_2.sti" ,-1,0,-1,1,-1 );
	/*giIMPMainPageButton[1] = QuickCreateButton( giIMPMainPageButtonImage[1], LAPTOP_SCREEN_UL_X + 136 , LAPTOP_SCREEN_WEB_UL_Y + ( 174 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPMainPageBeginCallback);
*/
	if(( iCurrentProfileMode ==0 )||( iCurrentProfileMode > 2) )
	{
	  giIMPMainPageButton[1] = CreateIconAndTextButton( giIMPMainPageButtonImage[ 1 ], pImpButtonText[ 1 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X + 136 , LAPTOP_SCREEN_WEB_UL_Y + ( 174 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnIMPMainPageBeginCallback);
  }
	else
	{
    giIMPMainPageButton[1] = CreateIconAndTextButton( giIMPMainPageButtonImage[ 1 ], pImpButtonText[ 22 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														LAPTOP_SCREEN_UL_X + 136 , LAPTOP_SCREEN_WEB_UL_Y + ( 174 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnIMPMainPageBeginCallback);
	}
	// the personality button
	giIMPMainPageButtonImage[2]=  LoadButtonImage( "LAPTOP/button_8.sti" ,-1,0,-1,1,-1 );

	/*
	giIMPMainPageButton[2] = QuickCreateButton( giIMPMainPageButtonImage[2], LAPTOP_SCREEN_UL_X + 13 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPMainPagePersonalityCallback);
*/
  giIMPMainPageButton[2] = CreateIconAndTextButton( giIMPMainPageButtonImage[ 2 ], pImpButtonText[ 2 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														LAPTOP_SCREEN_UL_X + 13 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnIMPMainPagePersonalityCallback);


	// the attribs button
	giIMPMainPageButtonImage[3]=  LoadButtonImage( "LAPTOP/button_8.sti" ,-1,0,-1,1,-1 );

	/*
	giIMPMainPageButton[3] = QuickCreateButton( giIMPMainPageButtonImage[3], LAPTOP_SCREEN_UL_X + 133 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPMainPageAttributesCallback);
*/
  giIMPMainPageButton[3] = CreateIconAndTextButton( giIMPMainPageButtonImage[ 3 ], pImpButtonText[ 3 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X + 133 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnIMPMainPageAttributesCallback);


  // the portrait button
	giIMPMainPageButtonImage[4]=  LoadButtonImage( "LAPTOP/button_8.sti" ,-1,0,-1,1,-1 );

	/*giIMPMainPageButton[4] = QuickCreateButton( giIMPMainPageButtonImage[4], LAPTOP_SCREEN_UL_X + 253 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPMainPagePortraitCallback);
  */

  giIMPMainPageButton[4] = CreateIconAndTextButton( giIMPMainPageButtonImage[ 4 ], pImpButtonText[ 4 ], FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 LAPTOP_SCREEN_UL_X + 253 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnIMPMainPagePortraitCallback);



	// the voice button
	giIMPMainPageButtonImage[5]=  LoadButtonImage( "LAPTOP/button_8.sti" ,-1,0,-1,1,-1 );
	/*giIMPMainPageButton[5] = QuickCreateButton( giIMPMainPageButtonImage[5], LAPTOP_SCREEN_UL_X + 373 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),
										BUTTON_TOGGLE, MSYS_PRIORITY_HIGHEST - 1,
										BtnGenericMouseMoveButtonCallback, BtnIMPMainPageVoiceCallback);
  */

	if( iCurrentProfileMode == 5 )
	{
		swprintf( sString, lengthof(sString), pImpButtonText[ 5 ], iCurrentVoices + 1 );
	}
	else
	{
		swprintf( sString, lengthof(sString), pImpButtonText[ 25 ] );
	}
	giIMPMainPageButton[5] = CreateIconAndTextButton( giIMPMainPageButtonImage[ 5 ], sString, FONT12ARIAL,
														 FONT_WHITE, DEFAULT_SHADOW,
														 FONT_WHITE, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														  LAPTOP_SCREEN_UL_X + 373 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ), BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 BtnIMPMainPageVoiceCallback);





  SetButtonCursor(giIMPMainPageButton[ 0 ], CURSOR_WWW);
	SetButtonCursor(giIMPMainPageButton[ 1 ], CURSOR_WWW);
	SetButtonCursor(giIMPMainPageButton[ 2 ], CURSOR_WWW);
	SetButtonCursor(giIMPMainPageButton[ 3 ], CURSOR_WWW);
	SetButtonCursor(giIMPMainPageButton[ 4 ], CURSOR_WWW);
	SetButtonCursor(giIMPMainPageButton[ 5 ], CURSOR_WWW);

	SpecifyButtonTextOffsets( giIMPMainPageButton[ 2 ], 10, 40, TRUE );
	SpecifyButtonTextOffsets( giIMPMainPageButton[ 3 ], 10, 40, TRUE );
	SpecifyButtonTextOffsets( giIMPMainPageButton[ 4 ], 10, 40, TRUE );
	SpecifyButtonTextOffsets( giIMPMainPageButton[ 5 ], 10, 40, TRUE );


	SpecifyButtonTextWrappedWidth( giIMPMainPageButton[ 2 ], MAIN_PAGE_BUTTON_TEXT_WIDTH);
	SpecifyButtonTextWrappedWidth( giIMPMainPageButton[ 3 ], MAIN_PAGE_BUTTON_TEXT_WIDTH);
	SpecifyButtonTextWrappedWidth( giIMPMainPageButton[ 4 ], MAIN_PAGE_BUTTON_TEXT_WIDTH);
	SpecifyButtonTextWrappedWidth( giIMPMainPageButton[ 5 ], MAIN_PAGE_BUTTON_TEXT_WIDTH);
}


static void DeleteIMPMainPageButtons(void)
{
  // this function destroys the buttons needed for the IMP about Us Page

  // the back  button
  RemoveButton(giIMPMainPageButton[0] );
  UnloadButtonImage(giIMPMainPageButtonImage[0] );

	// begin profiling button
	RemoveButton(giIMPMainPageButton[1] );
  UnloadButtonImage(giIMPMainPageButtonImage[1] );

	// begin personna button
	RemoveButton(giIMPMainPageButton[2] );
  UnloadButtonImage(giIMPMainPageButtonImage[2] );

	// begin attribs button
	RemoveButton(giIMPMainPageButton[3] );
  UnloadButtonImage(giIMPMainPageButtonImage[3] );

	// begin portrait button
	RemoveButton(giIMPMainPageButton[4] );
  UnloadButtonImage(giIMPMainPageButtonImage[4] );

	// begin voice button
	RemoveButton(giIMPMainPageButton[5] );
  UnloadButtonImage(giIMPMainPageButtonImage[5] );
}


static void BtnIMPMainPageBackCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for IMP Homepage About US button

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_HOME_PAGE;
		fButtonPendingFlag = TRUE;
		iCurrentProfileMode = 0;
		fFinishedCharGeneration = FALSE;
		ResetCharacterStats();
	}
}


static void BeginMessageBoxCallBack(UINT8 bExitValue);


static void BtnIMPMainPageBeginCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for Main Page Begin Profiling

	// too far along to change gender

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// are we going to change name, or do we have to start over from scratch
		if (iCurrentProfileMode > 2)
		{
			// too far along, restart
			DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[1], LAPTOP_SCREEN, MSG_BOX_FLAG_YESNO, BeginMessageBoxCallBack);
		}
		else
		{
			if (LaptopSaveInfo.iCurrentBalance < COST_OF_PROFILE)
			{
				DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[3], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, BeginMessageBoxCallBack);
			}
			else if (NumberOfMercsOnPlayerTeam() >= 18)
			{
				DoLapTopMessageBox(MSG_BOX_IMP_STYLE, pImpPopUpStrings[5], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, BeginMessageBoxCallBack);
			}
			else
			{
				// change name
				iCurrentImpPage = IMP_BEGIN;
				fButtonPendingFlag = TRUE;
			}
		}
	}
}


static void BtnIMPMainPagePersonalityCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for Main Page Begin Profiling

  // if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode != 1)
	{
		btn->uiFlags &= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_PERSONALITY;
		fButtonPendingFlag = TRUE;
	}
}


static void BtnIMPMainPageAttributesCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for Main Page Begin Profiling

  // if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode < 2)
	{
		btn->uiFlags &= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_ATTRIBUTE_ENTRANCE;
		fButtonPendingFlag = TRUE;
	}
}


void BtnIMPMainPagePortraitCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for Main Page Begin Profiling

  // if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode != 3 && iCurrentProfileMode != 4 && iCurrentProfileMode > 5)
	{
		btn->uiFlags&= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_PORTRAIT;
		fButtonPendingFlag = TRUE;
	}
}


void BtnIMPMainPageVoiceCallback(GUI_BUTTON *btn, INT32 reason)
{
	// btn callback for Main Page Begin Profiling

  // if not this far in char generation, don't alot ANY action
	if (iCurrentProfileMode != 4 && iCurrentProfileMode > 5)
	{
		btn->uiFlags&= ~BUTTON_CLICKED_ON;
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		iCurrentImpPage = IMP_VOICE;
		fButtonPendingFlag = TRUE;
	}
}


static void NextProfilingMode(void)
{
  // this function will change to mode the player is in for profiling

	// if less than done
  if(iCurrentProfileMode < 5)
    iCurrentProfileMode++;
}


static BOOLEAN CheckIfFinishedCharacterGeneration(void)
{
  // this function checks to see if character is done character generation

	// are we done character generation
	if( iCurrentProfileMode == 5 )
	{
		// yes
    return ( TRUE );
	}
	else
	{
		// no
		return ( FALSE );
	}
}


static void UpDateIMPMainPageButtons(void)
{
	// update mainpage button states
  INT32 iCount = 0;

	// disable all
  for( iCount = 2; iCount < 6; iCount++)
	{
		DisableButton( giIMPMainPageButton[ iCount ] );
	}

	for( iCount = 0; iCount < 4; iCount++ )
	{
		MSYS_DisableRegion( &pIMPMainPageMouseRegions[ iCount ]);
	}
	// enable
  switch(  iCurrentProfileMode )
	{
		case 0:
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 0 ]);
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 1 ]);
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 2 ]);
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 3 ]);
		break;
		case( 1 ):
		 EnableButton( giIMPMainPageButton[2] );
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 1 ]);
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 2 ]);
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 3 ]);
		break;
		case( 2 ):
		  EnableButton( giIMPMainPageButton[3] );
			MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 0 ]);
		  MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 2 ]);
		  MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 3 ]);
		break;
		case( 3 ):
			EnableButton( giIMPMainPageButton[3] );
		  EnableButton( giIMPMainPageButton[4] );
			MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 0 ]);
		  //MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 1 ]);
		  MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 3 ]);

		break;
		case( 4 ):
		 //MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 1 ]);
		 MSYS_EnableRegion( &pIMPMainPageMouseRegions[ 0 ]);
		 EnableButton( giIMPMainPageButton[3] );
		 EnableButton( giIMPMainPageButton[4] );
	   EnableButton( giIMPMainPageButton[5] );
		break;
	}
}


static void BeginMessageBoxCallBack(UINT8 bExitValue)
{
	// yes, so start over, else stay here and do nothing for now
  if( bExitValue == MSG_BOX_RETURN_YES )
	{
		iCurrentImpPage = IMP_BEGIN;
    iCurrentProfileMode = 0;
	}

	else if( bExitValue == MSG_BOX_RETURN_OK )
	{
		// if ok, then we are coming from financial warning, allow continue
	}
}


static void IMPMainPageNotSelectableBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateMouseRegionsForIMPMainPageBasedOnCharGenStatus(void)
{
  // this procedure will create masks for the char generation main page


	// mask for personality page button
	MSYS_DefineRegion( &pIMPMainPageMouseRegions[ 0 ],LAPTOP_SCREEN_UL_X + 13 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),  LAPTOP_SCREEN_UL_X + 13 + 115 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ) + 93 , MSYS_PRIORITY_HIGH+5,
						CURSOR_WWW, MSYS_NO_CALLBACK, IMPMainPageNotSelectableBtnCallback );

	// mask for attrib page button
	MSYS_DefineRegion( &pIMPMainPageMouseRegions[ 1 ],LAPTOP_SCREEN_UL_X + 133 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),  LAPTOP_SCREEN_UL_X + 133 + 115 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ) + 93 , MSYS_PRIORITY_HIGH+5,
						CURSOR_WWW, MSYS_NO_CALLBACK, IMPMainPageNotSelectableBtnCallback );



	// mask for portrait page button
	MSYS_DefineRegion( &pIMPMainPageMouseRegions[ 2 ],LAPTOP_SCREEN_UL_X + 253 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),  LAPTOP_SCREEN_UL_X + 253 + 115 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ) + 93 , MSYS_PRIORITY_HIGH+5,
						CURSOR_WWW, MSYS_NO_CALLBACK, IMPMainPageNotSelectableBtnCallback );




	// mask for voice page button
	MSYS_DefineRegion( &pIMPMainPageMouseRegions[ 3 ],LAPTOP_SCREEN_UL_X + 373 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ),  LAPTOP_SCREEN_UL_X + 373  + 115 , LAPTOP_SCREEN_WEB_UL_Y + ( 245 ) + 93 , MSYS_PRIORITY_HIGH+5,
						CURSOR_WWW, MSYS_NO_CALLBACK, IMPMainPageNotSelectableBtnCallback );
}


static void DestoryMouseRegionsForIMPMainPageBasedOnCharGenStatus(void)
{
	// will destroy button masks for the char gen pages


	 MSYS_RemoveRegion( &pIMPMainPageMouseRegions[ 0 ]);

	 MSYS_RemoveRegion( &pIMPMainPageMouseRegions[ 1 ]);

	 MSYS_RemoveRegion( &pIMPMainPageMouseRegions[ 2 ]);

	 MSYS_RemoveRegion( &pIMPMainPageMouseRegions[ 3 ]);
}


static void IMPMainPageNotSelectableBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
  if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
  {
   	DoLapTopMessageBox( MSG_BOX_IMP_STYLE, pImpPopUpStrings[ 4 ], LAPTOP_SCREEN, MSG_BOX_FLAG_OK, BeginMessageBoxCallBack);
  }
}


static BOOLEAN LoadCharacterPortraitForMainPage(void)
{
  // this function will load the character's portrait, to be used on portrait button
	if( iCurrentProfileMode >= 4 )
	{
		// load it
		guiCHARACTERPORTRAITFORMAINPAGE = AddVideoObjectFromFile(pPlayerSelectedFaceFileNames[iPortraitNumber]);
		CHECKF(guiCHARACTERPORTRAITFORMAINPAGE != NO_VOBJECT);

	// now specify
	SpecifyButtonIcon(  giIMPMainPageButton[4], guiCHARACTERPORTRAITFORMAINPAGE, 0,
													 33, 23, FALSE );
	}

	return( TRUE );
}
