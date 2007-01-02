#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "AIMLinks.h"
	#include "AIM.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "WordWrap.h"
	#include "Text.h"
	#include "Multi_Language_Graphic_Utils.h"
	#include "Button_System.h"
	#include "Video.h"
	#include "VSurface.h"
	#include "Font_Control.h"
#endif

#define		AIM_LINK_TITLE_FONT						FONT14ARIAL
#define		AIM_LINK_TITLE_COLOR					AIM_GREEN

#define   AIM_LINK_FONT									FONT12ARIAL
#define   AIM_LINK_COLOR								AIM_FONT_GOLD

#define		AIM_LINK_NUM_LINKS						3

#define		AIM_LINK_LINK_OFFSET_Y				94//90

#define		AIM_LINK_LINK_WIDTH						420
#define		AIM_LINK_LINK_HEIGHT					70

#define		AIM_LINK_BOBBY_LINK_X					LAPTOP_SCREEN_UL_X + 40
#define		AIM_LINK_BOBBY_LINK_Y					LAPTOP_SCREEN_WEB_UL_Y + 91

#define		AIM_LINK_FUNERAL_LINK_X				AIM_LINK_BOBBY_LINK_X
#define		AIM_LINK_FUNERAL_LINK_Y				AIM_LINK_BOBBY_LINK_Y + AIM_LINK_LINK_OFFSET_Y

#define		AIM_LINK_INSURANCE_LINK_X			AIM_LINK_BOBBY_LINK_X
#define		AIM_LINK_INSURANCE_LINK_Y			AIM_LINK_FUNERAL_LINK_Y + AIM_LINK_LINK_OFFSET_Y

#define		AIM_LINK_TITLE_X							IMAGE_OFFSET_X + 149
#define		AIM_LINK_TITLE_Y							AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y + 10
#define		AIM_LINK_TITLE_WIDTH					AIM_SYMBOL_WIDTH

#define		AIM_LINK_LINK_TEXT_1_Y				AIM_LINK_BOBBY_LINK_Y + 71
#define		AIM_LINK_LINK_TEXT_2_Y				AIM_LINK_FUNERAL_LINK_Y + 36
#define		AIM_LINK_LINK_TEXT_3_Y				AIM_LINK_INSURANCE_LINK_Y + 45






UINT32		guiBobbyLink;
UINT32		guiFuneralLink;
UINT32		guiInsuranceLink;
UINT8			gubLinkPages[]={
							BOBBYR_BOOKMARK,
							FUNERAL_BOOKMARK,
							INSURANCE_BOOKMARK};

//Clicking on guys Face
MOUSE_REGION    gSelectedLinkRegion[ AIM_LINK_NUM_LINKS ] ;
void SelectLinkRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason );


void GameInitAimLinks()
{

}

BOOLEAN EnterAimLinks()
{
	SGPFILENAME ImageFile;
	UINT16					usPosY;
	INT16						i;

	InitAimDefaults();
	InitAimMenuBar();

	// load the Bobby link graphic and add it
	GetMLGFilename(ImageFile, MLG_BOBBYRAYLINK);
	CHECKF(AddVideoObjectFromFile(ImageFile, &guiBobbyLink));

	// load the Funeral graphic and add it
	GetMLGFilename(ImageFile, MLG_MORTUARYLINK);
	CHECKF(AddVideoObjectFromFile(ImageFile, &guiFuneralLink));

	// load the Insurance graphic and add it
	GetMLGFilename(ImageFile, MLG_INSURANCELINK);
	CHECKF(AddVideoObjectFromFile(ImageFile, &guiInsuranceLink));

	usPosY = AIM_LINK_BOBBY_LINK_Y;
	for(i=0; i<AIM_LINK_NUM_LINKS; i++)
	{
		MSYS_DefineRegion( &gSelectedLinkRegion[i], AIM_LINK_BOBBY_LINK_X, usPosY , AIM_LINK_BOBBY_LINK_X + AIM_LINK_LINK_WIDTH, (UINT16)(usPosY + AIM_LINK_LINK_HEIGHT), MSYS_PRIORITY_HIGH,
								 CURSOR_WWW, MSYS_NO_CALLBACK, SelectLinkRegionCallBack );
		MSYS_AddRegion(&gSelectedLinkRegion[i]);
		MSYS_SetRegionUserData( &gSelectedLinkRegion[i], 0, gubLinkPages[i]);
		usPosY += AIM_LINK_LINK_OFFSET_Y;
	}


	RenderAimLinks();
	return(TRUE);
}

void ExitAimLinks()
{
	INT16						i;

	RemoveAimDefaults();

	DeleteVideoObjectFromIndex(guiBobbyLink);
	DeleteVideoObjectFromIndex(guiFuneralLink);
	DeleteVideoObjectFromIndex(guiInsuranceLink);

	for(i=0; i<AIM_LINK_NUM_LINKS; i++)
		MSYS_RemoveRegion( &gSelectedLinkRegion[i]);

	ExitAimMenuBar();

}

void HandleAimLinks()
{

}

void RenderAimLinks()
{
	DrawAimDefaults();
	DisableAimButton();

	BltVideoObjectFromIndex(FRAME_BUFFER, guiBobbyLink, 0, AIM_LINK_BOBBY_LINK_X, AIM_LINK_BOBBY_LINK_Y);

	BltVideoObjectFromIndex(FRAME_BUFFER, guiFuneralLink, 0, AIM_LINK_FUNERAL_LINK_X, AIM_LINK_FUNERAL_LINK_Y);
//	DrawTextToScreen(AimLinkText[AIM_LINK_FUNERAL], AIM_LINK_BOBBY_LINK_X, AIM_LINK_LINK_TEXT_2_Y, AIM_LINK_LINK_WIDTH, AIM_LINK_FONT, AIM_LINK_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

	BltVideoObjectFromIndex(FRAME_BUFFER, guiInsuranceLink, 0, AIM_LINK_INSURANCE_LINK_X, AIM_LINK_INSURANCE_LINK_Y);
//	DrawTextToScreen(AimLinkText[AIM_LINK_LISTENING], AIM_LINK_BOBBY_LINK_X, AIM_LINK_LINK_TEXT_3_Y, AIM_LINK_LINK_WIDTH, AIM_LINK_FONT, AIM_LINK_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

	//Draw Link Title
	DrawTextToScreen(AimLinkText[AIM_LINK_TITLE], AIM_LINK_TITLE_X, AIM_LINK_TITLE_Y, AIM_LINK_TITLE_WIDTH, AIM_LINK_TITLE_FONT, AIM_LINK_TITLE_COLOR, FONT_MCOLOR_BLACK, FALSE, CENTER_JUSTIFIED);

  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}

void SelectLinkRegionCallBack(MOUSE_REGION * pRegion, INT32 iReason )
{
	if (iReason & MSYS_CALLBACK_REASON_INIT)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
	}
	else if(iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		UINT32	gNextLaptopPage;

		gNextLaptopPage = MSYS_GetRegionUserData( pRegion, 0 );

		GoToWebPage( gNextLaptopPage );
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
	}
}
