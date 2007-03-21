#include "Laptop.h"
#include "AIMArchives.h"
#include "AIM.h"
#include "WordWrap.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Encrypted_File.h"
#include "Text.h"
#include <string.h>
#include "Button_System.h"
#include "Debug.h"
#include "VSurface.h"
#include "Video.h"
#include "Font_Control.h"


#define		AIM_ALUMNI_NAME_FILE		"BINARYDATA/AlumName.edt"
#define		AIM_ALUMNI_FILE					"BINARYDATA/Alumni.edt"


#define		AIM_ALUMNI_TITLE_FONT					FONT14ARIAL
#define		AIM_ALUMNI_TITLE_COLOR				AIM_GREEN

#define		AIM_ALUMNI_POPUP_FONT					FONT10ARIAL
#define		AIM_ALUMNI_POPUP_COLOR				FONT_MCOLOR_WHITE

#define		AIM_ALUMNI_POPUP_NAME_FONT		FONT12ARIAL
#define		AIM_ALUMNI_POPUP_NAME_COLOR		FONT_MCOLOR_WHITE

#define		AIM_ALUMNI_NAME_FONT					FONT12ARIAL
#define		AIM_ALUMNI_NAME_COLOR					FONT_MCOLOR_WHITE
#define		AIM_ALUMNI_PAGE_FONT					FONT14ARIAL
#define		AIM_ALUMNI_PAGE_COLOR_UP			FONT_MCOLOR_DKWHITE
#define		AIM_ALUMNI_PAGE_COLOR_DOWN		138

#define		AIM_ALUMNI_NAME_LINESIZE			80 * 2
#define		AIM_ALUMNI_ALUMNI_LINESIZE		7 * 80 * 2


#define		AIM_ALUMNI_NUM_FACE_COLS			5
#define		AIM_ALUMNI_NUM_FACE_ROWS			4
#define		MAX_NUMBER_OLD_MERCS_ON_PAGE	AIM_ALUMNI_NUM_FACE_ROWS * AIM_ALUMNI_NUM_FACE_COLS

#define		AIM_ALUMNI_START_GRID_X				LAPTOP_SCREEN_UL_X + 37
#define		AIM_ALUMNI_START_GRID_Y				LAPTOP_SCREEN_WEB_UL_Y + 68

#define		AIM_ALUMNI_GRID_OFFSET_X				90
#define		AIM_ALUMNI_GRID_OFFSET_Y				72

#define		AIM_ALUMNI_ALUMNI_FRAME_WIDTH		66
#define		AIM_ALUMNI_ALUMNI_FRAME_HEIGHT	64

#define		AIM_ALUMNI_ALUMNI_FACE_WIDTH		56
#define		AIM_ALUMNI_ALUMNI_FACE_HEIGHT		50

#define		AIM_ALUMNI_NAME_OFFSET_X				5
#define		AIM_ALUMNI_NAME_OFFSET_Y				55
#define		AIM_ALUMNI_NAME_WIDTH						AIM_ALUMNI_ALUMNI_FRAME_WIDTH - AIM_ALUMNI_NAME_OFFSET_X * 2

#define		AIM_ALUMNI_PAGE1_X							LAPTOP_SCREEN_UL_X + 100
#define		AIM_ALUMNI_PAGE1_Y							LAPTOP_SCREEN_WEB_UL_Y + 357
#define		AIM_ALUMNI_PAGE_GAP							BOTTOM_BUTTON_START_WIDTH + 25

#define		AIM_ALUMNI_PAGE_END_X						AIM_ALUMNI_PAGE1_X + (BOTTOM_BUTTON_START_WIDTH + BOTTOM_BUTTON_START_WIDTH) * 3
#define		AIM_ALUMNI_PAGE_END_Y						AIM_ALUMNI_PAGE1_Y + BOTTOM_BUTTON_START_HEIGHT

#define		AIM_ALUMNI_TITLE_X							IMAGE_OFFSET_X + 149
#define		AIM_ALUMNI_TITLE_Y							AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y // + 2
#define		AIM_ALUMNI_TITLE_WIDTH					AIM_SYMBOL_WIDTH

#define		AIM_POPUP_WIDTH									309
#define		AIM_POPUP_TEXT_WIDTH						296
#define		AIM_POPUP_SECTION_HEIGHT				9

#define		AIM_POPUP_X											LAPTOP_SCREEN_UL_X + (500-AIM_POPUP_WIDTH)/2
#define		AIM_POPUP_Y											120 + LAPTOP_SCREEN_WEB_DELTA_Y

#define		AIM_POPUP_SHADOW_GAP						4

#define		AIM_POPUP_TEXT_X								AIM_POPUP_X

#define		AIM_ALUMNI_FACE_PANEL_X					AIM_POPUP_X + 6
#define		AIM_ALUMNI_FACE_PANEL_Y					AIM_POPUP_Y + 6
#define		AIM_ALUMNI_FACE_PANEL_WIDTH			58
#define		AIM_ALUMNI_FACE_PANEL_HEIGHT		52

#define		AIM_ALUMNI_POPUP_NAME_X					AIM_ALUMNI_FACE_PANEL_X + AIM_ALUMNI_FACE_PANEL_WIDTH + 10
#define		AIM_ALUMNI_POPUP_NAME_Y					AIM_ALUMNI_FACE_PANEL_Y + 20

#define		AIM_ALUMNI_POPUP_DESC_X					AIM_POPUP_X + 8
#define		AIM_ALUMNI_POPUP_DESC_Y					AIM_ALUMNI_FACE_PANEL_Y + AIM_ALUMNI_FACE_PANEL_HEIGHT + 5

#define		AIM_ALUMNI_DONE_X								AIM_POPUP_X + AIM_POPUP_WIDTH - AIM_ALUMNI_DONE_WIDTH - 7
#define		AIM_ALUMNI_DONE_WIDTH						36
#define		AIM_ALUMNI_DONE_HEIGHT					16

#define		AIM_ALUMNI_NAME_SIZE						80 * 2
#define		AIM_ALUMNI_DECRIPTION_SIZE			80 * 7 * 2
#define		AIM_ALUMNI_FILE_RECORD_SIZE			80 * 8 * 2
//#define		AIM_ALUMNI_FILE_RECORD_SIZE			80 * 7 * 2
#define		AIM_ALUMNI_FULL_NAME_SIZE				80 * 2

UINT32		guiAlumniFrame;
UINT32		guiOldAim;
UINT32		guiAlumniPopUp;
UINT32		guiPopUpPic;
UINT32		guiDoneButton;

UINT8			gubPageNum;
BOOLEAN		gfExitingAimArchives;
UINT8			gubDrawOldMerc;
UINT8			gfDrawPopUpBox=FALSE;
BOOLEAN		gfDestroyPopUpBox;
BOOLEAN		gfFaceMouseRegionsActive;
//BOOLEAN		gfDestroyDoneRegion;
BOOLEAN		gfReDrawScreen=FALSE;

BOOLEAN		AimArchivesSubPagesVisitedFlag[3] = {0,0,0};

//Face regions
MOUSE_REGION		gMercAlumniFaceMouseRegions[ MAX_NUMBER_OLD_MERCS_ON_PAGE ];

//Done region
MOUSE_REGION		gDoneRegion;

//Previous Button
UINT32	guiAlumniPageButton[3];
INT32		guiAlumniPageButtonImage;


void GameInitAimArchives()
{

}

void EnterInitAimArchives()
{
	gfDrawPopUpBox=FALSE;
	gfDestroyPopUpBox = FALSE;

	memset( &AimArchivesSubPagesVisitedFlag, 0, 3);
	AimArchivesSubPagesVisitedFlag[0] = TRUE;
}


static void BtnAlumniPageButtonCallback(GUI_BUTTON* btn, INT32 reason);
static void DisableAimArchiveButton(void);
static void InitAlumniFaceRegions(void);


BOOLEAN EnterAimArchives()
{
	UINT16	usPosX, i;


	gfExitingAimArchives = FALSE;
//	gubDrawOldMerc = 255;
	gfDrawPopUpBox=FALSE;
	gfDestroyPopUpBox=FALSE;

	InitAimDefaults();
	InitAimMenuBar();

	gubPageNum = (UINT8)giCurrentSubPage;

	// load the Alumni Frame and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP/AlumniFrame.sti", &guiAlumniFrame));

	// load the 1st set of faces and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP/Old_Aim.sti", &guiOldAim));

	// load the PopupPic graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP/PopupPicFrame.sti", &guiPopUpPic));

		// load the AlumniPopUp graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP/AlumniPopUp.sti", &guiAlumniPopUp));

		// load the Done Button graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP/DoneButton.sti", &guiDoneButton));

	InitAlumniFaceRegions();

	//Load graphic for buttons
	guiAlumniPageButtonImage =  LoadButtonImage("LAPTOP/BottomButtons2.sti", -1,0,-1,1,-1 );

	usPosX = AIM_ALUMNI_PAGE1_X;
	for(i=0; i<3; i++)
	{
		guiAlumniPageButton[i] = CreateIconAndTextButton( guiAlumniPageButtonImage, AimAlumniText[i], AIM_ALUMNI_PAGE_FONT,
														 AIM_ALUMNI_PAGE_COLOR_UP, DEFAULT_SHADOW,
														 AIM_ALUMNI_PAGE_COLOR_DOWN, DEFAULT_SHADOW,
														 TEXT_CJUSTIFIED,
														 usPosX, AIM_ALUMNI_PAGE1_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
														 DEFAULT_MOVE_CALLBACK, BtnAlumniPageButtonCallback);
		SetButtonCursor(guiAlumniPageButton[i], CURSOR_WWW);
		MSYS_SetBtnUserData( guiAlumniPageButton[i], 0, i);

		usPosX += AIM_ALUMNI_PAGE_GAP;
	}

	DisableAimArchiveButton();
	RenderAimArchives();
	return(TRUE);
}


static void CreateDestroyDoneMouseRegion(UINT16 usPosY);
static void RemoveAimAlumniFaceRegion(void);


void ExitAimArchives()
{
	UINT16 i;

	gfExitingAimArchives = TRUE;

	DeleteVideoObjectFromIndex(guiAlumniFrame);
	DeleteVideoObjectFromIndex(guiOldAim);
	DeleteVideoObjectFromIndex(guiAlumniPopUp);
	DeleteVideoObjectFromIndex(guiPopUpPic);
	DeleteVideoObjectFromIndex(guiDoneButton);


	RemoveAimAlumniFaceRegion();

	UnloadButtonImage( guiAlumniPageButtonImage );
	for(i=0; i<3; i++)
 		RemoveButton( guiAlumniPageButton[i] );

	RemoveAimDefaults();
	ExitAimMenuBar();
	giCurrentSubPage = gubPageNum;

	CreateDestroyDoneMouseRegion(0);
	gfDestroyPopUpBox = FALSE;
	gfDrawPopUpBox = FALSE;
}

void HandleAimArchives()
{
	if( gfReDrawScreen )
	{
//		RenderAimArchives();
		fPausedReDrawScreenFlag = TRUE;

		gfReDrawScreen = FALSE;
	}
	if( gfDestroyPopUpBox )
	{
		gfDestroyPopUpBox = FALSE;

		CreateDestroyDoneMouseRegion(0);
		InitAlumniFaceRegions();
		gfDestroyPopUpBox = FALSE;
	}
}


static void DisplayAlumniOldMercPopUp(void);


void RenderAimArchives()
{
	UINT16		usPosX, usPosY,x,y,i=0;
	UINT8			ubNumRows=0;
	UINT32			uiStartLoc=0;
	wchar_t			sText[400];


	DrawAimDefaults();
	DisableAimButton();

	//Draw Link Title
	DrawTextToScreen(AimAlumniText[AIM_ALUMNI_ALUMNI], AIM_ALUMNI_TITLE_X, AIM_ALUMNI_TITLE_Y, AIM_ALUMNI_TITLE_WIDTH, AIM_ALUMNI_TITLE_FONT, AIM_ALUMNI_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

	//Draw the mug shot border and face
	HVOBJECT hFrameHandle = GetVideoObject(guiAlumniFrame);
	HVOBJECT hFaceHandle  = GetVideoObject(guiOldAim);

	switch(gubPageNum)
	{
		case 0:
			ubNumRows = AIM_ALUMNI_NUM_FACE_ROWS;
			i=0;
			break;
		case 1:
			ubNumRows = AIM_ALUMNI_NUM_FACE_ROWS;
			i=20;
			break;
		case 2:
			ubNumRows = 2;
			i=40;
			break;
		default:
			Assert(0);
			break;
	}

	usPosX = AIM_ALUMNI_START_GRID_X;
	usPosY = AIM_ALUMNI_START_GRID_Y;
	for(y=0; y<ubNumRows; y++)
	{
		for(x=0; x<AIM_ALUMNI_NUM_FACE_COLS; x++)
		{
			//Blt face to screen
			BltVideoObject(FRAME_BUFFER, hFaceHandle, i,usPosX+4, usPosY+4);

			//Blt the alumni frame background
			BltVideoObject(FRAME_BUFFER, hFrameHandle, 0,usPosX, usPosY);

			//Display the merc's name
			uiStartLoc = AIM_ALUMNI_NAME_LINESIZE * i;
			LoadEncryptedDataFromFile(AIM_ALUMNI_NAME_FILE, sText, uiStartLoc, AIM_ALUMNI_NAME_SIZE );
			DrawTextToScreen(sText, (UINT16)(usPosX + AIM_ALUMNI_NAME_OFFSET_X), (UINT16)(usPosY + AIM_ALUMNI_NAME_OFFSET_Y), AIM_ALUMNI_NAME_WIDTH, AIM_ALUMNI_NAME_FONT, AIM_ALUMNI_NAME_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);

			usPosX += AIM_ALUMNI_GRID_OFFSET_X;
			i++;
		}
		usPosX = AIM_ALUMNI_START_GRID_X;
		usPosY += AIM_ALUMNI_GRID_OFFSET_Y;
	}

	//the 3rd page now has an additional row with 1 merc on it, so add a new row
	if( gubPageNum == 2 )
	{
		//Blt face to screen
		BltVideoObject(FRAME_BUFFER, hFaceHandle, i,usPosX+4, usPosY+4);

		//Blt the alumni frame background
		BltVideoObject(FRAME_BUFFER, hFrameHandle, 0,usPosX, usPosY);

		//Display the merc's name
		uiStartLoc = AIM_ALUMNI_NAME_LINESIZE * i;
		LoadEncryptedDataFromFile(AIM_ALUMNI_NAME_FILE, sText, uiStartLoc, AIM_ALUMNI_NAME_SIZE );
		DrawTextToScreen(sText, (UINT16)(usPosX + AIM_ALUMNI_NAME_OFFSET_X), (UINT16)(usPosY + AIM_ALUMNI_NAME_OFFSET_Y), AIM_ALUMNI_NAME_WIDTH, AIM_ALUMNI_NAME_FONT, AIM_ALUMNI_NAME_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);

		usPosX += AIM_ALUMNI_GRID_OFFSET_X;
	}


	usPosX = AIM_ALUMNI_PAGE1_X;

	if( gfDrawPopUpBox )
	{
		DisplayAlumniOldMercPopUp();
		RemoveAimAlumniFaceRegion();
	}


  MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectAlumniFaceRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gfDrawPopUpBox = TRUE;
		gfReDrawScreen = TRUE;

		gubDrawOldMerc = (UINT8)MSYS_GetRegionUserData( pRegion, 0 );
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	}
}


static void ChangingAimArchiveSubPage(UINT8 ubSubPageNumber);
static void ResetAimArchiveButtons(void);


static void BtnAlumniPageButtonCallback(GUI_BUTTON *btn, INT32 reason)
{
	UINT8	ubRetValue = (UINT8)MSYS_GetBtnUserData(btn, 0);

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		RemoveAimAlumniFaceRegion();
		ChangingAimArchiveSubPage(ubRetValue);
		gubPageNum = ubRetValue;
		gfReDrawScreen = TRUE;
		gfDestroyPopUpBox = TRUE;
		ResetAimArchiveButtons();
		DisableAimArchiveButton();
		gfDrawPopUpBox = FALSE;
	}
}


static void ResetAimArchiveButtons(void)
{
	int i=0;

	for(i=0; i<3; i++)
	{
		ButtonList[ guiAlumniPageButton[i] ]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
}


static void DisableAimArchiveButton(void)
{
	if( gfExitingAimArchives == TRUE)
		return;

	if( (gubPageNum == 0 ) )
	{
		ButtonList[ guiAlumniPageButton[ 0 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
	}
	else if( gubPageNum == 1 )
	{
		ButtonList[ guiAlumniPageButton[ 1 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
	}
	else if( gubPageNum == 2 )
	{
		ButtonList[ guiAlumniPageButton[ 2 ] ]->uiFlags |= (BUTTON_CLICKED_ON );
	}
}


static void DisplayAlumniOldMercPopUp(void)
{
	UINT8			i,ubNumLines=11; //17
	UINT16		usPosY, usTextPosY;
	UINT8			ubFontHeight, ubNumDescLines;
//	WRAPPED_STRING *pFirstWrappedString, *pTempWrappedString;
	wchar_t	sName[AIM_ALUMNI_NAME_SIZE];
	wchar_t	sDesc[AIM_ALUMNI_DECRIPTION_SIZE];
	UINT32		uiStartLoc;
	UINT16	usStringPixLength;

	HVOBJECT hAlumniPopUpHandle = GetVideoObject(guiAlumniPopUp);

	ubFontHeight = (UINT8)GetFontHeight(AIM_ALUMNI_POPUP_FONT);

	//Load the description
	uiStartLoc = AIM_ALUMNI_FILE_RECORD_SIZE * gubDrawOldMerc + AIM_ALUMNI_FULL_NAME_SIZE;
	LoadEncryptedDataFromFile(AIM_ALUMNI_FILE, sDesc, uiStartLoc, AIM_ALUMNI_DECRIPTION_SIZE);

	usStringPixLength = StringPixLength( sDesc, AIM_ALUMNI_POPUP_FONT);
	ubNumDescLines = (UINT8) (usStringPixLength / AIM_POPUP_TEXT_WIDTH);

	ubNumLines += ubNumDescLines;

	usTextPosY = AIM_POPUP_Y + 5;
	usPosY = AIM_POPUP_Y;

	//draw top line of the popup background
	ShadowVideoSurfaceRect( FRAME_BUFFER, AIM_POPUP_X+AIM_POPUP_SHADOW_GAP, usPosY+AIM_POPUP_SHADOW_GAP, AIM_POPUP_X + AIM_POPUP_WIDTH+AIM_POPUP_SHADOW_GAP, usPosY + AIM_POPUP_SECTION_HEIGHT+AIM_POPUP_SHADOW_GAP-1);
	BltVideoObject(FRAME_BUFFER, hAlumniPopUpHandle, 0,AIM_POPUP_X, usPosY);
	//draw mid section of the popup background
	usPosY += AIM_POPUP_SECTION_HEIGHT;
	for(i=0; i<ubNumLines; i++)
	{
		ShadowVideoSurfaceRect( FRAME_BUFFER, AIM_POPUP_X+AIM_POPUP_SHADOW_GAP, usPosY+AIM_POPUP_SHADOW_GAP, AIM_POPUP_X + AIM_POPUP_WIDTH+AIM_POPUP_SHADOW_GAP, usPosY + AIM_POPUP_SECTION_HEIGHT+AIM_POPUP_SHADOW_GAP-1);
		BltVideoObject(FRAME_BUFFER, hAlumniPopUpHandle, 1,AIM_POPUP_X, usPosY);
		usPosY += AIM_POPUP_SECTION_HEIGHT;
	}
	//draw the bottom line and done button
	ShadowVideoSurfaceRect( FRAME_BUFFER, AIM_POPUP_X+AIM_POPUP_SHADOW_GAP, usPosY+AIM_POPUP_SHADOW_GAP, AIM_POPUP_X + AIM_POPUP_WIDTH+AIM_POPUP_SHADOW_GAP, usPosY + AIM_POPUP_SECTION_HEIGHT+AIM_POPUP_SHADOW_GAP-1);
	BltVideoObject(FRAME_BUFFER, hAlumniPopUpHandle, 2,AIM_POPUP_X, usPosY);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiDoneButton, 0, AIM_ALUMNI_DONE_X, usPosY - AIM_ALUMNI_DONE_HEIGHT);
	DrawTextToScreen(AimAlumniText[AIM_ALUMNI_DONE], (UINT16)(AIM_ALUMNI_DONE_X+1), (UINT16)(usPosY-AIM_ALUMNI_DONE_HEIGHT+3), AIM_ALUMNI_DONE_WIDTH, AIM_ALUMNI_POPUP_NAME_FONT, AIM_ALUMNI_POPUP_NAME_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

	CreateDestroyDoneMouseRegion(usPosY);

	///blt face panale and the mecs fce
	BltVideoObjectFromIndex(FRAME_BUFFER, guiPopUpPic, 0,AIM_ALUMNI_FACE_PANEL_X, AIM_ALUMNI_FACE_PANEL_Y);
	BltVideoObjectFromIndex(FRAME_BUFFER, guiOldAim, gubDrawOldMerc, AIM_ALUMNI_FACE_PANEL_X + 1, AIM_ALUMNI_FACE_PANEL_Y + 1);

	//Load and display the name
//	uiStartLoc = AIM_ALUMNI_NAME_SIZE * gubDrawOldMerc;
//	LoadEncryptedDataFromFile(AIM_ALUMNI_NAME_FILE, sName, uiStartLoc, AIM_ALUMNI_NAME_SIZE);
	uiStartLoc = AIM_ALUMNI_FILE_RECORD_SIZE * gubDrawOldMerc;
	LoadEncryptedDataFromFile( AIM_ALUMNI_FILE, sName, uiStartLoc, AIM_ALUMNI_FULL_NAME_SIZE );

	DrawTextToScreen(sName, AIM_ALUMNI_POPUP_NAME_X, AIM_ALUMNI_POPUP_NAME_Y, 0, AIM_ALUMNI_POPUP_NAME_FONT, AIM_ALUMNI_POPUP_NAME_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	//Display the description
	DisplayWrappedString(AIM_ALUMNI_POPUP_DESC_X, AIM_ALUMNI_POPUP_DESC_Y, AIM_POPUP_TEXT_WIDTH, 2, AIM_ALUMNI_POPUP_FONT, AIM_ALUMNI_POPUP_COLOR, sDesc, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void DestroyPopUpBox(void)
{
	gfDestroyPopUpBox = FALSE;
	RenderAimArchives();
}


static void InitAlumniFaceRegions(void)
{
	UINT16	usPosX, usPosY,i,x,y, usNumRows;

	if(gfFaceMouseRegionsActive)
		return;

	if( gubPageNum == 2 )
		usNumRows = 2;
	else
		usNumRows = AIM_ALUMNI_NUM_FACE_ROWS;

	usPosX = AIM_ALUMNI_START_GRID_X;
	usPosY = AIM_ALUMNI_START_GRID_Y;
	i=0;
	for(y=0; y<usNumRows; y++)
	{
		for(x=0; x<AIM_ALUMNI_NUM_FACE_COLS; x++)
		{

			MSYS_DefineRegion( &gMercAlumniFaceMouseRegions[ i ], usPosX, usPosY, (INT16)(usPosX + AIM_ALUMNI_ALUMNI_FACE_WIDTH), (INT16)(usPosY + AIM_ALUMNI_ALUMNI_FACE_HEIGHT), MSYS_PRIORITY_HIGH,
								 CURSOR_WWW, MSYS_NO_CALLBACK, SelectAlumniFaceRegionCallBack);
			// Add region
			MSYS_AddRegion( &gMercAlumniFaceMouseRegions[ i ] );
			MSYS_SetRegionUserData( &gMercAlumniFaceMouseRegions[ i ], 0, i+(20*gubPageNum));

			usPosX += AIM_ALUMNI_GRID_OFFSET_X;
			i++;
		}
		usPosX = AIM_ALUMNI_START_GRID_X;
		usPosY += AIM_ALUMNI_GRID_OFFSET_Y;
	}

	//the 3rd page now has an additional row with 1 merc on it, so add a new row
	if( gubPageNum == 2 )
	{
			MSYS_DefineRegion( &gMercAlumniFaceMouseRegions[ i ], usPosX, usPosY, (INT16)(usPosX + AIM_ALUMNI_ALUMNI_FACE_WIDTH), (INT16)(usPosY + AIM_ALUMNI_ALUMNI_FACE_HEIGHT), MSYS_PRIORITY_HIGH,
								 CURSOR_WWW, MSYS_NO_CALLBACK, SelectAlumniFaceRegionCallBack);
			// Add region
			MSYS_AddRegion( &gMercAlumniFaceMouseRegions[ i ] );
			MSYS_SetRegionUserData( &gMercAlumniFaceMouseRegions[ i ], 0, i+(20*gubPageNum));
	}



	gfFaceMouseRegionsActive = TRUE;
}


static void RemoveAimAlumniFaceRegion(void)
{
	UINT16 i;
	UINT16 usNumber=0;

	if(!gfFaceMouseRegionsActive)
		return;

	switch(gubPageNum)
	{
		case 0:
			usNumber = AIM_ALUMNI_NUM_FACE_ROWS * AIM_ALUMNI_NUM_FACE_COLS;
			break;
		case 1:
			usNumber = AIM_ALUMNI_NUM_FACE_ROWS * AIM_ALUMNI_NUM_FACE_COLS;
			break;
		case 2:
			usNumber = 2 * AIM_ALUMNI_NUM_FACE_COLS + 1;

		default:
			break;
	}

	for(i=0; i<usNumber; i++)
	{
	  MSYS_RemoveRegion( &gMercAlumniFaceMouseRegions[ i ]);
	}
	gfFaceMouseRegionsActive = FALSE;
}


static void SelectAlumniDoneRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateDestroyDoneMouseRegion(UINT16 usPosY)
{
	static BOOLEAN DoneRegionCreated=FALSE;

	if( ( !DoneRegionCreated ) && ( usPosY != 0) )
	{
		usPosY -= AIM_ALUMNI_DONE_HEIGHT;
		MSYS_DefineRegion( &gDoneRegion, AIM_ALUMNI_DONE_X-2, usPosY, (AIM_ALUMNI_DONE_X-2 + AIM_ALUMNI_DONE_WIDTH), (INT16)(usPosY + AIM_ALUMNI_DONE_HEIGHT), MSYS_PRIORITY_HIGH,
							 CURSOR_WWW, MSYS_NO_CALLBACK, SelectAlumniDoneRegionCallBack);
		// Add region
		MSYS_AddRegion( &gDoneRegion );
		DoneRegionCreated = TRUE;
	}

	if( DoneRegionCreated && usPosY == 0)
	{
		MSYS_RemoveRegion( &gDoneRegion );
		DoneRegionCreated = FALSE;
//		gfDestroyDoneRegion = FALSE;
	}
}


static void SelectAlumniDoneRegionCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gfDestroyPopUpBox = TRUE;
		gfDrawPopUpBox = FALSE;
		gfReDrawScreen = TRUE;
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	}
}


static void ChangingAimArchiveSubPage(UINT8 ubSubPageNumber)
{
	fLoadPendingFlag = TRUE;

	if( AimArchivesSubPagesVisitedFlag[ ubSubPageNumber ] == FALSE )
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = FALSE;

		AimArchivesSubPagesVisitedFlag[ ubSubPageNumber ] = TRUE;
	}
	else
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = TRUE;
	}
}
