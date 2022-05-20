#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Laptop.h"
#include "Insurance.h"
#include "Insurance_Contract.h"
#include "Local.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Line.h"
#include "Insurance_Text.h"
#include "Text.h"
#include "Multi_Language_Graphic_Utils.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"
#include "UILayout.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <string_theory/string>


#define INSURANCE_TEXT_SINGLE_FILE		BINARYDATADIR "/insurancesingle.edt"
#define INSURANCE_TEXT_MULTI_FILE		BINARYDATADIR "/insurancemulti.edt"


#define INSURANCE_BACKGROUND_WIDTH		125
#define INSURANCE_BACKGROUND_HEIGHT		100

#define INSURANCE_BIG_TITLE_X			95 + LAPTOP_SCREEN_UL_X
#define INSURANCE_BIG_TITLE_Y			4 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_RED_BAR_X			LAPTOP_SCREEN_UL_X
#define INSURANCE_RED_BAR_Y			LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_TOP_RED_BAR_X			LAPTOP_SCREEN_UL_X + 66
#define INSURANCE_TOP_RED_BAR_Y			109 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_TOP_RED_BAR_Y1		31 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BOTTOM_RED_BAR_Y		345 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BOTTOM_LINK_RED_BAR_X		77 + LAPTOP_SCREEN_UL_X
#define INSURANCE_BOTTOM_LINK_RED_BAR_Y		392 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH	107
#define INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET	148
#define INSURANCE_BOTTOM_LINK_RED_BAR_X_2	INSURANCE_BOTTOM_LINK_RED_BAR_X + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET
#define INSURANCE_BOTTOM_LINK_RED_BAR_X_3	INSURANCE_BOTTOM_LINK_RED_BAR_X_2 + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET

#define INSURANCE_LINK_TEXT_WIDTH		INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH

#define INSURANCE_LINK_TEXT_1_X			INSURANCE_BOTTOM_LINK_RED_BAR_X
#define INSURANCE_LINK_TEXT_1_Y			INSURANCE_BOTTOM_LINK_RED_BAR_Y - 36

#define INSURANCE_LINK_TEXT_2_X			INSURANCE_LINK_TEXT_1_X + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET
#define INSURANCE_LINK_TEXT_2_Y			INSURANCE_LINK_TEXT_1_Y

#define INSURANCE_LINK_TEXT_3_X			INSURANCE_LINK_TEXT_2_X + INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET
#define INSURANCE_LINK_TEXT_3_Y			INSURANCE_LINK_TEXT_1_Y

#define INSURANCE_SUBTITLE_X			INSURANCE_BOTTOM_LINK_RED_BAR_X + 15
#define INSURANCE_SUBTITLE_Y			150 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BULLET_TEXT_1_Y		188 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BULLET_TEXT_2_Y		215 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BULLET_TEXT_3_Y		242 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_BOTTOM_SLOGAN_X		INSURANCE_SUBTITLE_X
#define INSURANCE_BOTTOM_SLOGAN_Y		285 + LAPTOP_SCREEN_WEB_UL_Y
#define INSURANCE_BOTTOM_SLOGAN_WIDTH		370

#define INSURANCE_SMALL_TITLE_X			64 + LAPTOP_SCREEN_UL_X
#define INSURANCE_SMALL_TITLE_Y			5 + LAPTOP_SCREEN_WEB_UL_Y

#define INSURANCE_SMALL_TITLE_WIDTH		434 - 170
#define INSURANCE_SMALL_TITLE_HEIGHT		40 - 10


static SGPVObject* guiInsuranceBackGround;
static SGPVObject* guiInsuranceTitleImage;
static SGPVObject* guiInsuranceSmallTitleImage;
static SGPVObject* guiInsuranceRedBarImage;
static SGPVObject* guiInsuranceBigRedLineImage;
static SGPVObject* guiInsuranceBulletImage;


//link to the varios pages
static MOUSE_REGION gSelectedInsuranceLinkRegion[3];

//link to the home page by clicking on the small title
static MOUSE_REGION gSelectedInsuranceTitleLinkRegion;


static void SelectInsuranceRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterInsurance()
{
	UINT16 usPosX, i;

	SetBookMark( INSURANCE_BOOKMARK );

	InitInsuranceDefaults();

	// load the Insurance title graphic and add it
	const char* const ImageFile = GetMLGFilename(MLG_INSURANCETITLE);
	guiInsuranceTitleImage = AddVideoObjectFromFile(ImageFile);

	// load the red bar on the side of the page and add it
	guiInsuranceBulletImage = AddVideoObjectFromFile(LAPTOPDIR "/bullet.sti");

	usPosX = INSURANCE_BOTTOM_LINK_RED_BAR_X;
	for(i=0; i<3; i++)
	{
		MSYS_DefineRegion(&gSelectedInsuranceLinkRegion[i], usPosX, INSURANCE_BOTTOM_LINK_RED_BAR_Y-37,
					(UINT16)(usPosX + INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH),
					INSURANCE_BOTTOM_LINK_RED_BAR_Y+2, MSYS_PRIORITY_HIGH,
					CURSOR_WWW, MSYS_NO_CALLBACK, SelectInsuranceRegionCallBack);
		MSYS_SetRegionUserData( &gSelectedInsuranceLinkRegion[i], 0, i );

		usPosX += INSURANCE_BOTTOM_LINK_RED_BAR_OFFSET;
	}

	RenderInsurance();

	// reset the current merc index on the insurance contract page
	gusCurrentInsuranceMercIndex = 0;
}


void ExitInsurance()
{
	RemoveInsuranceDefaults();

	DeleteVideoObject(guiInsuranceTitleImage);
	DeleteVideoObject(guiInsuranceBulletImage);

	FOR_EACH(MOUSE_REGION, i, gSelectedInsuranceLinkRegion) MSYS_RemoveRegion(&*i);
}


void RenderInsurance()
{
	ST::string sText;

	DisplayInsuranceDefaults();

	SetFontShadow( INS_FONT_SHADOW );

	BltVideoObject(FRAME_BUFFER, guiInsuranceTitleImage, 0, INSURANCE_BIG_TITLE_X, INSURANCE_BIG_TITLE_Y);

	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_WERE_LISTENING);
	DrawTextToScreen(sText, LAPTOP_SCREEN_UL_X, INSURANCE_TOP_RED_BAR_Y - 35, LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	//Display the subtitle slogan
	sText = GetInsuranceText(INS_SNGL_LIFE_INSURANCE_SPECIALISTS);
	DrawTextToScreen(sText, INSURANCE_SUBTITLE_X, INSURANCE_SUBTITLE_Y, 0, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the bulleted text 1
	BltVideoObject(FRAME_BUFFER, guiInsuranceBulletImage, 0, INSURANCE_SUBTITLE_X, INSURANCE_BULLET_TEXT_1_Y);
	sText = GetInsuranceText(INS_MLTI_EMPLOY_HIGH_RISK);
	DrawTextToScreen(sText, INSURANCE_SUBTITLE_X + INSURANCE_BULLET_TEXT_OFFSET_X, INSURANCE_BULLET_TEXT_1_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the bulleted text 2
	BltVideoObject(FRAME_BUFFER, guiInsuranceBulletImage, 0, INSURANCE_SUBTITLE_X, INSURANCE_BULLET_TEXT_2_Y);
	sText = GetInsuranceText(INS_MLTI_HIGH_FATALITY_RATE);
	DrawTextToScreen(sText, INSURANCE_SUBTITLE_X + INSURANCE_BULLET_TEXT_OFFSET_X, INSURANCE_BULLET_TEXT_2_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the bulleted text 3
	BltVideoObject(FRAME_BUFFER, guiInsuranceBulletImage, 0, INSURANCE_SUBTITLE_X, INSURANCE_BULLET_TEXT_3_Y);
	sText = GetInsuranceText(INS_MLTI_DRAIN_SALARY);
	DrawTextToScreen(sText, INSURANCE_SUBTITLE_X + INSURANCE_BULLET_TEXT_OFFSET_X, INSURANCE_BULLET_TEXT_3_Y, 0, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the bottom slogan
	sText = GetInsuranceText(INS_MLTI_IF_ANSWERED_YES);
	DrawTextToScreen(sText, INSURANCE_BOTTOM_SLOGAN_X, INSURANCE_BOTTOM_SLOGAN_Y, INSURANCE_BOTTOM_SLOGAN_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Display the red bar under the link at the bottom.  and the text
	DisplaySmallRedLineWithShadow( INSURANCE_BOTTOM_LINK_RED_BAR_X, INSURANCE_BOTTOM_LINK_RED_BAR_Y, INSURANCE_BOTTOM_LINK_RED_BAR_X+INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH, INSURANCE_BOTTOM_LINK_RED_BAR_Y);

	sText = GetInsuranceText(INS_SNGL_COMMENTSFROM_CLIENTS);
	DisplayWrappedString(INSURANCE_LINK_TEXT_1_X, INSURANCE_LINK_TEXT_1_Y, INSURANCE_LINK_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Display the red bar under the link at the bottom
	DisplaySmallRedLineWithShadow( INSURANCE_BOTTOM_LINK_RED_BAR_X_2, INSURANCE_BOTTOM_LINK_RED_BAR_Y, INSURANCE_BOTTOM_LINK_RED_BAR_X_2+INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH, INSURANCE_BOTTOM_LINK_RED_BAR_Y);

	sText = GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK);
	DisplayWrappedString(INSURANCE_LINK_TEXT_2_X, INSURANCE_LINK_TEXT_2_Y+7, INSURANCE_LINK_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Display the red bar under the link at the bottom
	DisplaySmallRedLineWithShadow( INSURANCE_BOTTOM_LINK_RED_BAR_X_3, INSURANCE_BOTTOM_LINK_RED_BAR_Y, INSURANCE_BOTTOM_LINK_RED_BAR_X_3+INSURANCE_BOTTOM_LINK_RED_BAR_WIDTH, INSURANCE_BOTTOM_LINK_RED_BAR_Y);

	sText = GetInsuranceText(INS_SNGL_TO_ENTER_REVIEW);
	DisplayWrappedString(INSURANCE_LINK_TEXT_3_X, INSURANCE_LINK_TEXT_3_Y+7, INSURANCE_LINK_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	SetFontShadow(DEFAULT_SHADOW);

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectInsuranceTitleLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void InitInsuranceDefaults()
{
	// load the Flower Account Box graphic and add it
	guiInsuranceBackGround = AddVideoObjectFromFile(LAPTOPDIR "/backgroundtile.sti");

	// load the red bar on the side of the page and add it
	guiInsuranceRedBarImage = AddVideoObjectFromFile(LAPTOPDIR "/lefttile.sti");

	// load the red bar on the side of the page and add it
	guiInsuranceBigRedLineImage = AddVideoObjectFromFile(LAPTOPDIR "/largebar.sti");

	//if it is not the first page, display the small title
	if( guiCurrentLaptopMode != LAPTOP_MODE_INSURANCE )
	{
		// load the small title for the every page other then the first page
		const char* const ImageFile = GetMLGFilename(MLG_SMALLTITLE);
		guiInsuranceSmallTitleImage = AddVideoObjectFromFile(ImageFile);

		//create the link to the home page on the small titles
		MSYS_DefineRegion(&gSelectedInsuranceTitleLinkRegion, INSURANCE_SMALL_TITLE_X+85,
					INSURANCE_SMALL_TITLE_Y,
					(UINT16)(INSURANCE_SMALL_TITLE_X + INSURANCE_SMALL_TITLE_WIDTH),
					(UINT16)(INSURANCE_SMALL_TITLE_Y + INSURANCE_SMALL_TITLE_HEIGHT),
					MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
					SelectInsuranceTitleLinkRegionCallBack);
	}
}


void DisplayInsuranceDefaults()
{
	UINT8  i;
	UINT16 usPosY;

	WebPageTileBackground(4, 4, INSURANCE_BACKGROUND_WIDTH, INSURANCE_BACKGROUND_HEIGHT, guiInsuranceBackGround);

	usPosY = INSURANCE_RED_BAR_Y;

	for(i=0; i<4; i++)
	{
		BltVideoObject(FRAME_BUFFER, guiInsuranceRedBarImage, 0, INSURANCE_RED_BAR_X, usPosY);
		usPosY += INSURANCE_BACKGROUND_HEIGHT;
	}

	//display the top red bar
	switch( guiCurrentLaptopMode )
	{
		case LAPTOP_MODE_INSURANCE:
			usPosY = INSURANCE_TOP_RED_BAR_Y;
			BltVideoObject(FRAME_BUFFER, guiInsuranceBigRedLineImage, 0, INSURANCE_TOP_RED_BAR_X, usPosY);
			break;

		case LAPTOP_MODE_INSURANCE_INFO:
		case LAPTOP_MODE_INSURANCE_CONTRACT:
			usPosY = INSURANCE_TOP_RED_BAR_Y1;
			break;
		default:
			break;
	}

	BltVideoObject(FRAME_BUFFER, guiInsuranceBigRedLineImage, 0, INSURANCE_TOP_RED_BAR_X, INSURANCE_BOTTOM_RED_BAR_Y);

	//if it is not the first page, display the small title
	if( guiCurrentLaptopMode != LAPTOP_MODE_INSURANCE )
	{
		BltVideoObject(FRAME_BUFFER, guiInsuranceSmallTitleImage, 0, INSURANCE_SMALL_TITLE_X, INSURANCE_SMALL_TITLE_Y);
	}
}

void RemoveInsuranceDefaults()
{
	DeleteVideoObject(guiInsuranceBackGround);
	DeleteVideoObject(guiInsuranceRedBarImage);
	DeleteVideoObject(guiInsuranceBigRedLineImage);

	//if it is not the first page, display the small title
	if( guiPreviousLaptopMode != LAPTOP_MODE_INSURANCE )
	{
		DeleteVideoObject(guiInsuranceSmallTitleImage);
		MSYS_RemoveRegion( &gSelectedInsuranceTitleLinkRegion );
	}
}


void DisplaySmallRedLineWithShadow( UINT16 usStartX, UINT16 usStartY, UINT16 EndX, UINT16 EndY)
{
	SGPVSurface::Lock l(FRAME_BUFFER);

	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	UINT16* const pDestBuf = l.Buffer<UINT16>();

	// draw the red line
	LineDraw(FALSE, usStartX, usStartY, EndX, EndY, Get16BPPColor( FROMRGB( 255, 0, 0 ) ), pDestBuf);

	// draw the black shadow line
	LineDraw(FALSE, usStartX+1, usStartY+1, EndX+1, EndY+1, Get16BPPColor( FROMRGB( 0, 0, 0 ) ), pDestBuf);
}


ST::string GetInsuranceText(const UINT8 ubNumber)
{
	UINT32	uiStartLoc=0;

	if( ubNumber < INS_MULTI_LINE_BEGINS )
	{
		//Get and display the card saying
		uiStartLoc = INSURANCE_TEXT_SINGLE_LINE_SIZE * ubNumber;
		return GCM->loadEncryptedString(INSURANCE_TEXT_SINGLE_FILE, uiStartLoc, INSURANCE_TEXT_SINGLE_LINE_SIZE);
	}
	else
	{
		//Get and display the card saying
		uiStartLoc = INSURANCE_TEXT_MULTI_LINE_SIZE * ( ubNumber - INS_MULTI_LINE_BEGINS - 1 );
		return GCM->loadEncryptedString(INSURANCE_TEXT_MULTI_FILE, uiStartLoc, INSURANCE_TEXT_MULTI_LINE_SIZE);
	}
}


static void SelectInsuranceRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_POINTER_UP)
	{
		UINT32 uiInsuranceLink = MSYS_GetRegionUserData( pRegion, 0 );

		if( uiInsuranceLink == 0 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_COMMENTS;
		else if( uiInsuranceLink == 1 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_INFO;
		else if( uiInsuranceLink == 2 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_CONTRACT;
	}
}


static void SelectInsuranceTitleLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_POINTER_UP)
	{
		guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
	}
}
