#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "Mercs_Files.h"
	#include "Mercs.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "WordWrap.h"
	#include "Cursors.h"
	#include "Soldier_Profile.h"
	#include "stdio.h"
	#include "Encrypted_File.h"
	#include "AIMMembers.h"
	#include "Game_Clock.h"
	#include "Soldier_Add.h"
	#include "Overhead.h"
	#include "Game_Clock.h"
	#include "Message.h"
	#include "Font.h"
	#include "LaptopSave.h"
	#include "Text.h"
	#include "Speck_Quotes.h"
#endif

#include "Cheats.h"



#define		MERCBIOFILE												"BINARYDATA\\MercBios.edt"

#define		MERC_BIO_FONT											FONT14ARIAL//FONT12ARIAL
#define		MERC_BIO_COLOR										FONT_MCOLOR_WHITE

#define		MERC_TITLE_FONT										FONT14ARIAL
#define		MERC_TITLE_COLOR									146

#define		MERC_NAME_FONT										FONT14ARIAL
#define		MERC_NAME_COLOR										FONT_MCOLOR_WHITE

#define		MERC_STATS_FONT										FONT12ARIAL
#define		MERC_STATIC_STATS_COLOR						146
#define		MERC_DYNAMIC_STATS_COLOR					FONT_MCOLOR_WHITE


#define		MERC_FILES_PORTRAIT_BOX_X					LAPTOP_SCREEN_UL_X + 16
#define		MERC_FILES_PORTRAIT_BOX_Y					LAPTOP_SCREEN_WEB_UL_Y + 17

#define		MERC_FACE_X												MERC_FILES_PORTRAIT_BOX_X + 2
#define		MERC_FACE_Y												MERC_FILES_PORTRAIT_BOX_Y + 2
#define		MERC_FACE_WIDTH										106
#define		MERC_FACE_HEIGHT									122

#define		MERC_FILES_STATS_BOX_X						LAPTOP_SCREEN_UL_X + 164
#define		MERC_FILES_STATS_BOX_Y						MERC_FILES_PORTRAIT_BOX_Y


#define		MERC_FILES_BIO_BOX_X							MERC_FILES_PORTRAIT_BOX_X
#define		MERC_FILES_BIO_BOX_Y							LAPTOP_SCREEN_WEB_UL_Y + 155

#define		MERC_FILES_PREV_BUTTON_X					128
#define		MERC_FILES_PREV_BUTTON_Y					380

#define		MERC_FILES_NEXT_BUTTON_X					490
#define		MERC_FILES_NEXT_BUTTON_Y					MERC_FILES_PREV_BUTTON_Y

#define		MERC_FILES_HIRE_BUTTON_X					260
#define		MERC_FILES_HIRE_BUTTON_Y					MERC_FILES_PREV_BUTTON_Y

#define		MERC_FILES_BACK_BUTTON_X					380
#define		MERC_FILES_BACK_BUTTON_Y					MERC_FILES_PREV_BUTTON_Y

#define		MERC_NAME_X												MERC_FILES_STATS_BOX_X + 50
#define		MERC_NAME_Y												MERC_FILES_STATS_BOX_Y + 10

#define		MERC_BIO_TEXT_X										MERC_FILES_BIO_BOX_X + 5
#define		MERC_BIO_TEXT_Y										MERC_FILES_BIO_BOX_Y + 10

#define		MERC_ADD_BIO_TITLE_X							MERC_BIO_TEXT_X
#define		MERC_ADD_BIO_TITLE_Y							MERC_BIO_TEXT_Y + 100

#define		MERC_ADD_BIO_TEXT_X								MERC_BIO_TEXT_X
#define		MERC_ADD_BIO_TEXT_Y								MERC_ADD_BIO_TITLE_Y + 20

#define		MERC_BIO_WIDTH										460 - 10

#define		MERC_BIO_INFO_TEXT_SIZE						5 * 80 * 2
#define		MERC_BIO_ADD_INFO_TEXT_SIZE				2 * 80 * 2
#define		MERC_BIO_SIZE											7 * 80 * 2

#define		MERC_STATS_FIRST_COL_X						MERC_NAME_X
#define		MERC_STATS_FIRST_NUM_COL_X				MERC_STATS_FIRST_COL_X + 90
#define		MERC_STATS_SECOND_COL_X						MERC_FILES_STATS_BOX_X + 170
#define		MERC_STATS_SECOND_NUM_COL_X				MERC_STATS_SECOND_COL_X + 115
#define		MERC_SPACE_BN_LINES								15

#define		MERC_HEALTH_Y											MERC_FILES_STATS_BOX_Y + 30

#define		MERC_PORTRAIT_TEXT_OFFSET_Y				110


UINT32	guiPortraitBox;
UINT32	guiStatsBox;
UINT32	guiBioBox;
UINT32	guiMercFace;

//
// Buttons
//

// The Prev button
void BtnMercPrevButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiPrevButton;
INT32		guiButtonImage;

// The Next button
void BtnMercNextButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiNextButton;

// The Hire button
void BtnMercHireButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiHireButton;

// The Back button
void BtnMercFilesBackButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiMercBackButton;


//****************************
//
//  Function Prototypes
//
//****************************

BOOLEAN DisplayMercFace( UINT8 ubMercID);
void LoadAndDisplayMercBio( UINT8 ubMercID );
void DisplayMercsStats( UINT8 ubMercID );
BOOLEAN MercFilesHireMerc(UINT8 ubMercID);
void EnableDisableMercFilesNextPreviousButton( );





void GameInitMercsFiles()
{

}

BOOLEAN EnterMercsFiles()
{
  VOBJECT_DESC    VObjectDesc;

	InitMercBackGround();

	// load the stats box graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	GetMLGFilename( VObjectDesc.ImageFile, MLG_STATSBOX );
	CHECKF(AddVideoObject(&VObjectDesc, &guiStatsBox));

	// load the Portrait box graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\PortraitBox.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiPortraitBox));

	// load the bio box graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\BioBox.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiBioBox));


	// Prev Box button
	guiButtonImage  = LoadButtonImage("LAPTOP\\BigButtons.sti", -1,0,-1,1,-1 );

	guiPrevButton = CreateIconAndTextButton( guiButtonImage, MercInfo[MERC_FILES_PREVIOUS],
													 FONT12ARIAL,
													 MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW,
													 MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 MERC_FILES_PREV_BUTTON_X, MERC_FILES_PREV_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnMercPrevButtonCallback);

	SetButtonCursor(guiPrevButton, CURSOR_LAPTOP_SCREEN);
	SpecifyDisabledButtonStyle( guiPrevButton, DISABLED_STYLE_SHADED);

	//Next Button
	guiNextButton = CreateIconAndTextButton( guiButtonImage, MercInfo[MERC_FILES_NEXT],
													 FONT12ARIAL,
													 MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW,
													 MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 MERC_FILES_NEXT_BUTTON_X, MERC_FILES_NEXT_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnMercNextButtonCallback);

	SetButtonCursor(guiNextButton, CURSOR_LAPTOP_SCREEN);
	SpecifyDisabledButtonStyle( guiNextButton, DISABLED_STYLE_SHADED);

	//Hire button
	guiHireButton = CreateIconAndTextButton( guiButtonImage, MercInfo[MERC_FILES_HIRE],
													 FONT12ARIAL,
													 MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW,
													 MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 MERC_FILES_HIRE_BUTTON_X, MERC_FILES_HIRE_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnMercHireButtonCallback);
	SetButtonCursor(guiHireButton, CURSOR_LAPTOP_SCREEN);
	SpecifyDisabledButtonStyle( guiHireButton, DISABLED_STYLE_SHADED);

	//Back button
	guiMercBackButton = CreateIconAndTextButton( guiButtonImage, MercInfo[MERC_FILES_HOME],
													 FONT12ARIAL,
													 MERC_BUTTON_UP_COLOR, DEFAULT_SHADOW,
													 MERC_BUTTON_DOWN_COLOR, DEFAULT_SHADOW,
													 TEXT_CJUSTIFIED,
													 MERC_FILES_BACK_BUTTON_X, MERC_FILES_BACK_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnMercFilesBackButtonCallback);
	SetButtonCursor(guiMercBackButton, CURSOR_LAPTOP_SCREEN);
	SpecifyDisabledButtonStyle( guiMercBackButton, DISABLED_STYLE_SHADED);

//	RenderMercsFiles();
	return( TRUE );
}

void ExitMercsFiles()
{
	DeleteVideoObjectFromIndex(guiPortraitBox);
	DeleteVideoObjectFromIndex(guiStatsBox);
	DeleteVideoObjectFromIndex(guiBioBox);

	UnloadButtonImage( guiButtonImage );
	RemoveButton( guiPrevButton );
	RemoveButton( guiNextButton );
	RemoveButton( guiHireButton );
	RemoveButton( guiMercBackButton );

	RemoveMercBackGround();
}

void HandleMercsFiles()
{

}

void RenderMercsFiles()
{
  HVOBJECT hPixHandle;

	DrawMecBackGround();

	// Portrait Box
	GetVideoObject(&hPixHandle, guiPortraitBox);
	BltVideoObject(FRAME_BUFFER, hPixHandle, 0,MERC_FILES_PORTRAIT_BOX_X, MERC_FILES_PORTRAIT_BOX_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	// Stats Box
	GetVideoObject(&hPixHandle, guiStatsBox);
	BltVideoObject(FRAME_BUFFER, hPixHandle, 0,MERC_FILES_STATS_BOX_X, MERC_FILES_STATS_BOX_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	// bio box
	GetVideoObject(&hPixHandle, guiBioBox);
	BltVideoObject(FRAME_BUFFER, hPixHandle, 0,MERC_FILES_BIO_BOX_X+1, MERC_FILES_BIO_BOX_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	//Display the mercs face
	DisplayMercFace( GetMercIDFromMERCArray( gubCurMercIndex ) );

	//Display Mercs Name
	DrawTextToScreen(gMercProfiles[ GetMercIDFromMERCArray( gubCurMercIndex ) ].zName, MERC_NAME_X, MERC_NAME_Y, 0, MERC_NAME_FONT, MERC_NAME_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	//Load and display the mercs bio
	LoadAndDisplayMercBio( (UINT8)(GetMercIDFromMERCArray( gubCurMercIndex ) - BIFF ) );

	//Display the mercs statistic
	DisplayMercsStats( GetMercIDFromMERCArray( gubCurMercIndex ) );

	//check to see if the merc is dead if so disable the contact button
	if( IsMercDead( GetMercIDFromMERCArray( gubCurMercIndex ) ) )
		DisableButton( guiHireButton );
	else if( ( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_VALID ) && ( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_SUSPENDED ) && ( LaptopSaveInfo.gubPlayersMercAccountStatus != MERC_ACCOUNT_VALID_FIRST_WARNING ) )
	{
		//if the players account is suspended, disable the button
		DisableButton( guiHireButton );
	}
	else
		EnableButton( guiHireButton );

	//Enable or disable the buttons
	EnableDisableMercFilesNextPreviousButton( );

  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}




void BtnMercPrevButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );

			if( gubCurMercIndex > 0 )
				gubCurMercIndex--;

			//Since there are 2 larry roachburns
			if( gubCurMercIndex == MERC_LARRY_ROACHBURN)
				gubCurMercIndex--;

			fReDrawScreenFlag = TRUE;

			//Enable or disable the buttons
			EnableDisableMercFilesNextPreviousButton( );

			InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}


void BtnMercNextButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );


			if( gubCurMercIndex <= LaptopSaveInfo.gubLastMercIndex-1 )
				gubCurMercIndex++;

			//Since there are 2 larry roachburns
			if( gubCurMercIndex == MERC_LARRY_ROACHBURN)
				gubCurMercIndex++;

			fReDrawScreenFlag = TRUE;

			//Enable or disable the buttons
			EnableDisableMercFilesNextPreviousButton( );

			InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}


void BtnMercHireButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );

			//if the players accont is suspended, go back to the main screen and have Speck inform the players
			if( LaptopSaveInfo.gubPlayersMercAccountStatus == MERC_ACCOUNT_SUSPENDED )
			{
				guiCurrentLaptopMode = LAPTOP_MODE_MERC;
				gusMercVideoSpeckSpeech = SPECK_QUOTE_ALTERNATE_OPENING_5_PLAYER_OWES_SPECK_ACCOUNT_SUSPENDED;
				gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;

			}

			//else try to hire the merc
			else if( MercFilesHireMerc( GetMercIDFromMERCArray( gubCurMercIndex ) ) )
			{
				guiCurrentLaptopMode = LAPTOP_MODE_MERC;
				gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;

				//start the merc talking
//				HandlePlayerHiringMerc( GetMercIDFromMERCArray( gubCurMercIndex ) );

				//We just hired a merc
				gfJustHiredAMercMerc = TRUE;

				//Display a popup msg box telling the user when and where the merc will arrive
				DisplayPopUpBoxExplainingMercArrivalLocationAndTime( GetMercIDFromMERCArray( gubCurMercIndex ) );
			}

			InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}


BOOLEAN DisplayMercFace( UINT8 ubMercID)
{
  HVOBJECT hFaceHandle;
  HVOBJECT hPortraitHandle;
	STR							sFaceLoc = "FACES\\BIGFACES\\";
	char						sTemp[100];
	MERCPROFILESTRUCT	*pMerc;
  VOBJECT_DESC    VObjectDesc;
	SOLDIERTYPE			*pSoldier=NULL;

	// Portrait Frame
	GetVideoObject(&hPortraitHandle, guiPortraitBox);
  BltVideoObject(FRAME_BUFFER, hPortraitHandle, 0,MERC_FILES_PORTRAIT_BOX_X, MERC_FILES_PORTRAIT_BOX_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	pMerc = &gMercProfiles[ ubMercID ];

	//See if the merc is currently hired
	pSoldier = FindSoldierByProfileID( ubMercID, TRUE );

	// load the Face graphic and add it
  sprintf(sTemp, "%s%02d.sti", sFaceLoc, ubMercID);
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP(sTemp, VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiMercFace));

	//Blt face to screen
	GetVideoObject(&hFaceHandle, guiMercFace);
  BltVideoObject(FRAME_BUFFER, hFaceHandle, 0,MERC_FACE_X, MERC_FACE_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	//if the merc is dead, shadow the face red and put text over top saying the merc is dead
	if( IsMercDead( ubMercID ) )
	{
		//shade the face red, (to signif that he is dead)
		hFaceHandle->pShades[ 0 ]		= Create16BPPPaletteShaded( hFaceHandle->pPaletteEntry, DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE );

		//get the face object
		GetVideoObject(&hFaceHandle, guiMercFace);

		//set the red pallete to the face
		SetObjectHandleShade( guiMercFace, 0 );

		//Blt face to screen
	  BltVideoObject(FRAME_BUFFER, hFaceHandle, 0,MERC_FACE_X, MERC_FACE_Y, VO_BLT_SRCTRANSPARENCY,NULL);

		DisplayWrappedString(MERC_FACE_X, MERC_FACE_Y+MERC_PORTRAIT_TEXT_OFFSET_Y, MERC_FACE_WIDTH, 2, FONT14ARIAL, 145, MercInfo[MERC_FILES_MERC_IS_DEAD], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
	}

	else if( ubMercID == FLO && gubFact[ FACT_PC_MARRYING_DARYL_IS_FLO ] )
	{
		ShadowVideoSurfaceRect( FRAME_BUFFER, MERC_FACE_X, MERC_FACE_Y, MERC_FACE_X + MERC_FACE_WIDTH, MERC_FACE_Y + MERC_FACE_HEIGHT);
		DisplayWrappedString( MERC_FACE_X, MERC_FACE_Y+MERC_PORTRAIT_TEXT_OFFSET_Y, MERC_FACE_WIDTH, 2, FONT14ARIAL, 145, pPersonnelDepartedStateStrings[3], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );
	}

	//else if the merc is currently a POW or, the merc was fired as a pow
	else if( pMerc->bMercStatus == MERC_FIRED_AS_A_POW || ( pSoldier &&  pSoldier->bAssignment == ASSIGNMENT_POW ) )
	{
		ShadowVideoSurfaceRect( FRAME_BUFFER, MERC_FACE_X, MERC_FACE_Y, MERC_FACE_X + MERC_FACE_WIDTH, MERC_FACE_Y + MERC_FACE_HEIGHT);
		DisplayWrappedString(MERC_FACE_X, MERC_FACE_Y+MERC_PORTRAIT_TEXT_OFFSET_Y, MERC_FACE_WIDTH, 2, FONT14ARIAL, 145, pPOWStrings[0], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
	}

	//if the merc is hired already, say it
	else if( !IsMercHireable( ubMercID ) && pMerc->bMercStatus == MERC_HIRED_BUT_NOT_ARRIVED_YET || pMerc->bMercStatus > 0 )
	{
		ShadowVideoSurfaceRect( FRAME_BUFFER, MERC_FACE_X, MERC_FACE_Y, MERC_FACE_X + MERC_FACE_WIDTH, MERC_FACE_Y + MERC_FACE_HEIGHT);
		DisplayWrappedString(MERC_FACE_X, MERC_FACE_Y+MERC_PORTRAIT_TEXT_OFFSET_Y, MERC_FACE_WIDTH, 2, FONT14ARIAL, 145, MercInfo[MERC_FILES_ALREADY_HIRED], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
	}

	//if the merc is away on another assignemnt, say the merc is unavailable
	else if( !IsMercHireable( ubMercID ) )
	{
		ShadowVideoSurfaceRect( FRAME_BUFFER, MERC_FACE_X, MERC_FACE_Y, MERC_FACE_X + MERC_FACE_WIDTH, MERC_FACE_Y + MERC_FACE_HEIGHT);
		DisplayWrappedString(MERC_FACE_X, MERC_FACE_Y+MERC_PORTRAIT_TEXT_OFFSET_Y, MERC_FACE_WIDTH, 2, FONT14ARIAL, 145, MercInfo[MERC_FILES_MERC_UNAVAILABLE], FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);
	}

	DeleteVideoObjectFromIndex(guiMercFace);

	return( TRUE );
}


void LoadAndDisplayMercBio( UINT8 ubMercID )
{
	wchar_t	sText[400];
	UINT32	uiStartLoc = 0;

	//load and display the merc bio
	uiStartLoc = MERC_BIO_SIZE * ubMercID;
	LoadEncryptedDataFromFile(MERCBIOFILE, sText, uiStartLoc, MERC_BIO_INFO_TEXT_SIZE);
	DisplayWrappedString(MERC_BIO_TEXT_X, MERC_BIO_TEXT_Y, MERC_BIO_WIDTH, 2, MERC_BIO_FONT, MERC_BIO_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	//load and display the merc's additioanl info (if any)
	uiStartLoc = MERC_BIO_SIZE * ubMercID + MERC_BIO_INFO_TEXT_SIZE;
	LoadEncryptedDataFromFile(MERCBIOFILE, sText, uiStartLoc, MERC_BIO_ADD_INFO_TEXT_SIZE);
	if( sText[0] != 0 )
	{
		DrawTextToScreen( MercInfo[ MERC_FILES_ADDITIONAL_INFO ], MERC_ADD_BIO_TITLE_X, MERC_ADD_BIO_TITLE_Y, 0, MERC_TITLE_FONT, MERC_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
		DisplayWrappedString( MERC_ADD_BIO_TEXT_X, MERC_ADD_BIO_TEXT_Y, MERC_BIO_WIDTH, 2, MERC_BIO_FONT, MERC_BIO_COLOR, sText, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED );
	}
}


void DisplayMercsStats( UINT8 ubMercID )
{
	UINT16 usPosY, usPosX;
	wchar_t sString[128];

	usPosY = MERC_HEALTH_Y;

	//Health
	DrawTextToScreen(MercInfo[MERC_FILES_HEALTH], MERC_STATS_FIRST_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bLife, 3, MERC_STATS_FIRST_NUM_COL_X, MERC_HEALTH_Y, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Agility
	DrawTextToScreen(MercInfo[MERC_FILES_AGILITY], MERC_STATS_FIRST_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bAgility, 3, MERC_STATS_FIRST_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Dexterity
	DrawTextToScreen(MercInfo[MERC_FILES_DEXTERITY], MERC_STATS_FIRST_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bDexterity, 3, MERC_STATS_FIRST_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Strenght
	DrawTextToScreen(MercInfo[MERC_FILES_STRENGTH], MERC_STATS_FIRST_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bStrength, 3, MERC_STATS_FIRST_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Leadership
	DrawTextToScreen(MercInfo[MERC_FILES_LEADERSHIP], MERC_STATS_FIRST_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bLeadership, 3, MERC_STATS_FIRST_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Wisdom
	DrawTextToScreen(MercInfo[MERC_FILES_WISDOM], MERC_STATS_FIRST_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bWisdom, 3, MERC_STATS_FIRST_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);

	usPosY = MERC_HEALTH_Y;

	//Experience Level
	DrawTextToScreen(MercInfo[MERC_FILES_EXPLEVEL], MERC_STATS_SECOND_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bExpLevel, 3, MERC_STATS_SECOND_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Marksmanship
	DrawTextToScreen(MercInfo[MERC_FILES_MARKSMANSHIP], MERC_STATS_SECOND_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bMarksmanship, 3, MERC_STATS_SECOND_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Mechanical
	DrawTextToScreen(MercInfo[MERC_FILES_MECHANICAL], MERC_STATS_SECOND_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bMechanical, 3, MERC_STATS_SECOND_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Explosive
	DrawTextToScreen(MercInfo[MERC_FILES_EXPLOSIVE], MERC_STATS_SECOND_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bExplosive, 3, MERC_STATS_SECOND_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Medical
	DrawTextToScreen(MercInfo[MERC_FILES_MEDICAL], MERC_STATS_SECOND_COL_X, usPosY, 0, MERC_STATS_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);
	DrawNumeralsToScreen(gMercProfiles[ ubMercID ].bMedical, 3, MERC_STATS_SECOND_NUM_COL_X, usPosY, MERC_STATS_FONT, MERC_DYNAMIC_STATS_COLOR);
	usPosY += MERC_SPACE_BN_LINES;

	//Daily Salary
	DrawTextToScreen( MercInfo[MERC_FILES_SALARY], MERC_STATS_SECOND_COL_X, usPosY, 0, MERC_NAME_FONT, MERC_STATIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	usPosX = MERC_STATS_SECOND_COL_X + StringPixLength(MercInfo[MERC_FILES_SALARY], MERC_NAME_FONT) + 1;
	swprintf(sString, L"%d %s", gMercProfiles[ ubMercID ].sSalary, MercInfo[MERC_FILES_PER_DAY]);
	DrawTextToScreen( sString, usPosX, usPosY, 95, MERC_NAME_FONT, MERC_DYNAMIC_STATS_COLOR, FONT_MCOLOR_BLACK, FALSE, RIGHT_JUSTIFIED);
}



BOOLEAN MercFilesHireMerc(UINT8 ubMercID)
{
	MERC_HIRE_STRUCT HireMercStruct;
	INT8	bReturnCode;

	memset(&HireMercStruct, 0, sizeof(MERC_HIRE_STRUCT));

	//if the ALT key is down
	if( gfKeyState[ ALT ] && CHEATER_CHEAT_LEVEL( ) )
	{
		//set the merc to be hireable
		gMercProfiles[ ubMercID ].bMercStatus = MERC_OK;
		gMercProfiles[ ubMercID ].uiDayBecomesAvailable = 0;
	}

	//if the merc is away, dont hire
	if( !IsMercHireable( ubMercID ) )
	{
		if( gMercProfiles[ ubMercID ].bMercStatus != MERC_IS_DEAD )
		{
			guiCurrentLaptopMode = LAPTOP_MODE_MERC;
			gusMercVideoSpeckSpeech = SPECK_QUOTE_PLAYER_TRIES_TO_HIRE_ALREADY_HIRED_MERC;
			gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;
		}

		return(FALSE);
	}

	HireMercStruct.ubProfileID = ubMercID;

//
//	HireMercStruct.fCopyProfileItemsOver = gfBuyEquipment;
//
	HireMercStruct.fCopyProfileItemsOver = TRUE;

	HireMercStruct.iTotalContractLength = 1;

	//Specify where the merc is to appear
	HireMercStruct.sSectorX = gsMercArriveSectorX;//13;
	HireMercStruct.sSectorY = gsMercArriveSectorY;
	HireMercStruct.fUseLandingZoneForArrival = TRUE;

	HireMercStruct.uiTimeTillMercArrives = GetMercArrivalTimeOfDay( );// + ubMercID


	//Set the time and ID of the last hired merc will arrive
//	LaptopSaveInfo.sLastHiredMerc.iIdOfMerc = HireMercStruct.ubProfileID;
//	LaptopSaveInfo.sLastHiredMerc.uiArrivalTime = HireMercStruct.uiTimeTillMercArrives;


	bReturnCode = HireMerc( &HireMercStruct );
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



void BtnMercFilesBackButtonCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		btn->uiFlags |= BUTTON_CLICKED_ON;
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if (btn->uiFlags & BUTTON_CLICKED_ON)
		{
			btn->uiFlags &= (~BUTTON_CLICKED_ON );

			guiCurrentLaptopMode = LAPTOP_MODE_MERC;
			gubArrivedFromMercSubSite = MERC_CAME_FROM_HIRE_PAGE;

			InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}

void EnableDisableMercFilesNextPreviousButton( )
{
	if( gubCurMercIndex <= LaptopSaveInfo.gubLastMercIndex-1 )
		EnableButton( guiNextButton );
	else
		DisableButton( guiNextButton );

	if( gubCurMercIndex > 0 )
		EnableButton( guiPrevButton );
	else
		DisableButton( guiPrevButton );
}
