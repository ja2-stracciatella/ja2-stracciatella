#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "Florist.h"
	#include "Florist_Gallery.h"
	#include "WCheck.h"
	#include "Utilities.h"
	#include "WordWrap.h"
	#include "Cursors.h"
	#include "stdio.h"
	#include "Encrypted_File.h"
	#include "Text.h"
	#include <string.h>
	#include "Button_System.h"
#endif




#define	FLOR_GALLERY_TITLE_FONT								FONT10ARIAL
#define	FLOR_GALLERY_TITLE_COLOR							FONT_MCOLOR_WHITE

#define	FLOR_GALLERY_FLOWER_TITLE_FONT				FONT14ARIAL
#define	FLOR_GALLERY_FLOWER_TITLE_COLOR				FONT_MCOLOR_WHITE

#define	FLOR_GALLERY_FLOWER_PRICE_FONT				FONT12ARIAL
#define	FLOR_GALLERY_FLOWER_PRICE_COLOR				FONT_MCOLOR_WHITE

#define	FLOR_GALLERY_FLOWER_DESC_FONT					FONT12ARIAL
#define	FLOR_GALLERY_FLOWER_DESC_COLOR				FONT_MCOLOR_WHITE

#define	FLOR_GALLERY_NUMBER_FLORAL_BUTTONS		3
#define	FLOR_GALLERY_NUMBER_FLORAL_IMAGES			10

#define	FLOR_GALLERY_FLOWER_DESC_TEXT_FONT		FONT12ARIAL
#define	FLOR_GALLERY_FLOWER_DESC_TEXT_COLOR		FONT_MCOLOR_WHITE

#define FLOR_GALLERY_BACK_BUTTON_X						LAPTOP_SCREEN_UL_X + 8
#define FLOR_GALLERY_BACK_BUTTON_Y						LAPTOP_SCREEN_WEB_UL_Y + 12

#define FLOR_GALLERY_NEXT_BUTTON_X						LAPTOP_SCREEN_UL_X + 420
#define FLOR_GALLERY_NEXT_BUTTON_Y						FLOR_GALLERY_BACK_BUTTON_Y

#define FLOR_GALLERY_FLOWER_BUTTON_X					LAPTOP_SCREEN_UL_X + 7
#define FLOR_GALLERY_FLOWER_BUTTON_Y					LAPTOP_SCREEN_WEB_UL_Y + 74

//#define FLOR_GALLERY_FLOWER_BUTTON_OFFSET_X		250

#define FLOR_GALLERY_FLOWER_BUTTON_OFFSET_Y		112

#define FLOR_GALLERY_TITLE_TEXT_X							LAPTOP_SCREEN_UL_X + 0
#define FLOR_GALLERY_TITLE_TEXT_Y							LAPTOP_SCREEN_WEB_UL_Y + 48
#define FLOR_GALLERY_TITLE_TEXT_WIDTH					LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X

#define FLOR_GALLERY_FLOWER_TITLE_X						FLOR_GALLERY_FLOWER_BUTTON_X + 88

#define FLOR_GALLERY_DESC_WIDTH								390

#define FLOR_GALLERY_FLOWER_TITLE_OFFSET_Y		9
#define FLOR_GALLERY_FLOWER_PRICE_OFFSET_Y		FLOR_GALLERY_FLOWER_TITLE_OFFSET_Y + 17
#define FLOR_GALLERY_FLOWER_DESC_OFFSET_Y			FLOR_GALLERY_FLOWER_PRICE_OFFSET_Y + 15


UINT32	guiFlowerImages[ 3 ];

UINT32	guiCurrentlySelectedFlower=0;

UINT8		gubCurFlowerIndex=0;
UINT8		gubCurNumberOfFlowers=0;
UINT8		gubPrevNumberOfFlowers=0;
BOOLEAN gfRedrawFloristGallery=FALSE;

BOOLEAN		FloristGallerySubPagesVisitedFlag[ 4 ];

//Floral buttons
INT32		guiGalleryButtonImage;
void		BtnGalleryFlowerButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiGalleryButton[ FLOR_GALLERY_NUMBER_FLORAL_BUTTONS ];

//Next Previous buttons
INT32		guiFloralGalleryButtonImage;
void		BtnFloralGalleryNextButtonCallback(GUI_BUTTON *btn,INT32 reason);
void		BtnFloralGalleryBackButtonCallback(GUI_BUTTON *btn,INT32 reason);
UINT32	guiFloralGalleryButton[2];


BOOLEAN InitFlowerButtons();
void DeleteFlowerButtons();
BOOLEAN DisplayFloralDescriptions();
void ChangingFloristGallerySubPage( UINT8 ubSubPageNumber );


void GameInitFloristGallery()
{

}

void EnterInitFloristGallery()
{
	memset( &FloristGallerySubPagesVisitedFlag, 0, 4);
}


BOOLEAN EnterFloristGallery()
{
	InitFloristDefaults();

	//the next previous buttons
	guiFloralGalleryButtonImage	= LoadButtonImage("LAPTOP\\FloristButtons.sti", -1,0,-1,1,-1 );

	guiFloralGalleryButton[0] = CreateIconAndTextButton( guiFloralGalleryButtonImage, sFloristGalleryText[FLORIST_GALLERY_PREV], FLORIST_BUTTON_TEXT_FONT,
													 FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
													 FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
													 TEXT_CJUSTIFIED,
													 FLOR_GALLERY_BACK_BUTTON_X, FLOR_GALLERY_BACK_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnFloralGalleryBackButtonCallback);
	SetButtonCursor(guiFloralGalleryButton[0], CURSOR_WWW );

	guiFloralGalleryButton[1] = CreateIconAndTextButton( guiFloralGalleryButtonImage, sFloristGalleryText[FLORIST_GALLERY_NEXT], FLORIST_BUTTON_TEXT_FONT,
													 FLORIST_BUTTON_TEXT_UP_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
													 FLORIST_BUTTON_TEXT_DOWN_COLOR, FLORIST_BUTTON_TEXT_SHADOW_COLOR,
													 TEXT_CJUSTIFIED,
													 FLOR_GALLERY_NEXT_BUTTON_X, FLOR_GALLERY_NEXT_BUTTON_Y, BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
													 DEFAULT_MOVE_CALLBACK, BtnFloralGalleryNextButtonCallback);
	SetButtonCursor(guiFloralGalleryButton[1], CURSOR_WWW );

	RenderFloristGallery();

	InitFlowerButtons();

	return(TRUE);
}

void ExitFloristGallery()
{
	UINT16 i;

	RemoveFloristDefaults();

	for(i=0; i<2; i++)
		RemoveButton( guiFloralGalleryButton[i] );

	UnloadButtonImage( guiFloralGalleryButtonImage  );

	DeleteFlowerButtons();

}

void HandleFloristGallery()
{
	if( gfRedrawFloristGallery )
	{
		gfRedrawFloristGallery=FALSE;

		//
		DeleteFlowerButtons();
		InitFlowerButtons();

		fPausedReDrawScreenFlag = TRUE;
	}

}

void RenderFloristGallery()
{
	DisplayFloristDefaults();

	DrawTextToScreen(sFloristGalleryText[FLORIST_GALLERY_CLICK_TO_ORDER], FLOR_GALLERY_TITLE_TEXT_X, FLOR_GALLERY_TITLE_TEXT_Y, FLOR_GALLERY_TITLE_TEXT_WIDTH, FLOR_GALLERY_TITLE_FONT, FLOR_GALLERY_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED	);
	DrawTextToScreen(sFloristGalleryText[FLORIST_GALLERY_ADDIFTIONAL_FEE], FLOR_GALLERY_TITLE_TEXT_X, FLOR_GALLERY_TITLE_TEXT_Y+11, FLOR_GALLERY_TITLE_TEXT_WIDTH, FLOR_GALLERY_TITLE_FONT, FLOR_GALLERY_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED );

	DisplayFloralDescriptions();

  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


void BtnFloralGalleryNextButtonCallback(GUI_BUTTON *btn,INT32 reason)
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


			if( (gubCurFlowerIndex + 3 ) <= FLOR_GALLERY_NUMBER_FLORAL_IMAGES )
				gubCurFlowerIndex += 3;

			ChangingFloristGallerySubPage( gubCurFlowerIndex );

			InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);

			gfRedrawFloristGallery = TRUE;
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}


void BtnFloralGalleryBackButtonCallback(GUI_BUTTON *btn,INT32 reason)
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


			if( gubCurFlowerIndex != 0 )
			{
				if( gubCurFlowerIndex >=3 )
					gubCurFlowerIndex -= 3;
				else
					gubCurFlowerIndex = 0;

				ChangingFloristGallerySubPage( gubCurFlowerIndex );
			}
			else
			{
				guiCurrentLaptopMode = LAPTOP_MODE_FLORIST;
			}

			gfRedrawFloristGallery = TRUE;

			InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}



void BtnGalleryFlowerButtonCallback(GUI_BUTTON *btn,INT32 reason)
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

			guiCurrentlySelectedFlower = (UINT8) MSYS_GetBtnUserData( btn, 0 );
			guiCurrentLaptopMode = LAPTOP_MODE_FLORIST_ORDERFORM;

			gfShowBookmarks = FALSE;

			InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
		}
	}
	if(reason & MSYS_CALLBACK_REASON_LOST_MOUSE)
	{
		btn->uiFlags &= (~BUTTON_CLICKED_ON );
		InvalidateRegion(btn->Area.RegionTopLeftX, btn->Area.RegionTopLeftY, btn->Area.RegionBottomRightX, btn->Area.RegionBottomRightY);
	}
}


BOOLEAN InitFlowerButtons()
{
	UINT16 i,j, count;
	UINT16 usPosY;
	char		sTemp[40];
  VOBJECT_DESC    VObjectDesc;


	if( (FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex) >= 3 )
		gubCurNumberOfFlowers = 3;
	else
		gubCurNumberOfFlowers = FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex;

	gubPrevNumberOfFlowers = gubCurNumberOfFlowers;

	//the 10 pictures of the flowers
	count = gubCurFlowerIndex;
	for(i=0; i<gubCurNumberOfFlowers; i++)
	{
		// load the handbullet graphic and add it
		sprintf( sTemp, "LAPTOP\\Flower_%d.sti", count);
		VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
		FilenameForBPP(sTemp, VObjectDesc.ImageFile);
		CHECKF(AddVideoObject(&VObjectDesc, &guiFlowerImages[i]));
		count++;
	}

	//the buttons with the flower pictures on them
	usPosY = FLOR_GALLERY_FLOWER_BUTTON_Y;
//	usPosX = FLOR_GALLERY_FLOWER_BUTTON_X;
	count = gubCurFlowerIndex;
	guiGalleryButtonImage	= LoadButtonImage("LAPTOP\\GalleryButtons.sti", -1,0,-1,1,-1 );
	for(j=0; j<gubCurNumberOfFlowers; j++)
	{
		guiGalleryButton[j] = QuickCreateButton( guiGalleryButtonImage, FLOR_GALLERY_FLOWER_BUTTON_X, usPosY,
																	BUTTON_TOGGLE, MSYS_PRIORITY_HIGH,
																	DEFAULT_MOVE_CALLBACK, (GUI_CALLBACK)BtnGalleryFlowerButtonCallback);
		SetButtonCursor( guiGalleryButton[j], CURSOR_WWW);
		MSYS_SetBtnUserData( guiGalleryButton[j], 0, count);

		SpecifyButtonIcon( guiGalleryButton[j], guiFlowerImages[ j ], 0, 5, 5, FALSE );
		usPosY += FLOR_GALLERY_FLOWER_BUTTON_OFFSET_Y;
		count ++;
	}

	//if its the first page, display the 'back' text  in place of the 'prev' text on the top left button
	if( gubCurFlowerIndex == 0 )
		SpecifyButtonText( guiFloralGalleryButton[0], sFloristGalleryText[FLORIST_GALLERY_HOME] );
	else
		SpecifyButtonText( guiFloralGalleryButton[0], sFloristGalleryText[FLORIST_GALLERY_PREV] );

	//if it is the last page disable the next button
	if( gubCurFlowerIndex == FLOR_GALLERY_NUMBER_FLORAL_IMAGES-1 )
		DisableButton( guiFloralGalleryButton[1] );
	else
		EnableButton( guiFloralGalleryButton[1] );


	return(TRUE);
}


void DeleteFlowerButtons()
{
	UINT16 i;

	for(i=0; i<gubPrevNumberOfFlowers; i++)
	{
		DeleteVideoObjectFromIndex( guiFlowerImages[i] );
	}

	UnloadButtonImage( guiGalleryButtonImage );

	for(i=0; i<gubPrevNumberOfFlowers; i++)
	{
		RemoveButton( guiGalleryButton[i] );
	}

}


BOOLEAN DisplayFloralDescriptions()
{
	wchar_t		sTemp[ 640 ];
	UINT32	uiStartLoc=0, i;
	UINT16	usPosY, usPrice;

	if( (FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex) >= 3 )
		gubCurNumberOfFlowers = 3;
	else
		gubCurNumberOfFlowers = FLOR_GALLERY_NUMBER_FLORAL_IMAGES - gubCurFlowerIndex;

	usPosY = FLOR_GALLERY_FLOWER_BUTTON_Y;
	for(i=0; i<gubCurNumberOfFlowers; i++)
	{
		//Display Flower title
		uiStartLoc = FLOR_GALLERY_TEXT_TOTAL_SIZE * (i + gubCurFlowerIndex);
		LoadEncryptedDataFromFile(FLOR_GALLERY_TEXT_FILE, sTemp, uiStartLoc, FLOR_GALLERY_TEXT_TITLE_SIZE);
		DrawTextToScreen(sTemp, FLOR_GALLERY_FLOWER_TITLE_X, (UINT16)(usPosY+FLOR_GALLERY_FLOWER_TITLE_OFFSET_Y), 0, FLOR_GALLERY_FLOWER_TITLE_FONT, FLOR_GALLERY_FLOWER_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);

		//Display Flower Price
		uiStartLoc = FLOR_GALLERY_TEXT_TOTAL_SIZE * (i + gubCurFlowerIndex) + FLOR_GALLERY_TEXT_TITLE_SIZE;
		LoadEncryptedDataFromFile(FLOR_GALLERY_TEXT_FILE, sTemp, uiStartLoc, FLOR_GALLERY_TEXT_PRICE_SIZE);
		swscanf( sTemp, L"%hu", &usPrice);
		swprintf( sTemp, lengthof(sTemp), L"$%d.00 %s", usPrice, pMessageStrings[ MSG_USDOLLAR_ABBREVIATION ] );
		DrawTextToScreen(sTemp, FLOR_GALLERY_FLOWER_TITLE_X, (UINT16)(usPosY+FLOR_GALLERY_FLOWER_PRICE_OFFSET_Y), 0, FLOR_GALLERY_FLOWER_PRICE_FONT, FLOR_GALLERY_FLOWER_PRICE_COLOR, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED	);

		//Display Flower Desc
		uiStartLoc = FLOR_GALLERY_TEXT_TOTAL_SIZE * (i + gubCurFlowerIndex) + FLOR_GALLERY_TEXT_TITLE_SIZE + FLOR_GALLERY_TEXT_PRICE_SIZE;
		LoadEncryptedDataFromFile(FLOR_GALLERY_TEXT_FILE, sTemp, uiStartLoc, FLOR_GALLERY_TEXT_DESC_SIZE);
		DisplayWrappedString(FLOR_GALLERY_FLOWER_TITLE_X, (UINT16)(usPosY+FLOR_GALLERY_FLOWER_DESC_OFFSET_Y), FLOR_GALLERY_DESC_WIDTH, 2, FLOR_GALLERY_FLOWER_DESC_FONT, FLOR_GALLERY_FLOWER_DESC_COLOR,  sTemp, FONT_MCOLOR_BLACK, FALSE, LEFT_JUSTIFIED);

		usPosY += FLOR_GALLERY_FLOWER_BUTTON_OFFSET_Y;
	}

	return(TRUE);
}


void ChangingFloristGallerySubPage( UINT8 ubSubPageNumber )
{
	fLoadPendingFlag = TRUE;

	//there are 3 flowers per page
	if( ubSubPageNumber == FLOR_GALLERY_NUMBER_FLORAL_IMAGES )
		ubSubPageNumber = 4;
	else
		ubSubPageNumber = ubSubPageNumber / 3;

	if( FloristGallerySubPagesVisitedFlag[ ubSubPageNumber ] == FALSE )
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = FALSE;

		FloristGallerySubPagesVisitedFlag[ ubSubPageNumber ] = TRUE;
	}
	else
	{
		fConnectingToSubPage = TRUE;
		fFastLoadFlag = TRUE;
	}
}
