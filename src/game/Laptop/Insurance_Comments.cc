#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "Insurance_Text.h"
#include "Insurance.h"
#include "Insurance_Comments.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"

#include <string_theory/string>


#define INS_CMNT_TITLE_Y		52 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CMNT_FIRST_BULLET_X		82 + LAPTOP_SCREEN_UL_X
#define INS_CMNT_FIRST_BULLET_Y		75 + LAPTOP_SCREEN_WEB_UL_Y

#define INS_CMNT_REDLINE_WIDTH		384

#define INS_CMNT_COMMENT_OFFSET_Y	20

#define INS_CMNT_NEXT_COMMENT_OFFSET_Y	65

#define INS_CMNT_COMMENT_TEXT_WIDTH	364

#define INS_CMNT_LINK_Y			357 + LAPTOP_SCREEN_WEB_UL_Y
#define INS_CMNT_LINK_WIDTH		90
#define INS_CMNT_LINK_HEIGHT		35
#define INS_CMNT_LINK_OFFSET_X		166


static SGPVObject* guiInsCmntBulletImage;


//link to the varios pages
static MOUSE_REGION gSelectedInsuranceCommentLinkRegion[3];


static void SelectInsuranceCommentLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterInsuranceComments()
{
	UINT8 i;
	UINT16 usPosX;

	InitInsuranceDefaults();

	// load the Insurance bullet graphic and add it
	guiInsCmntBulletImage = AddVideoObjectFromFile(LAPTOPDIR "/bullet.sti");

	usPosX = INS_CMNT_FIRST_BULLET_X-6;
	for( i=0; i<3; i++)
	{
		MSYS_DefineRegion(&gSelectedInsuranceCommentLinkRegion[i], usPosX, INS_CMNT_LINK_Y-1,
					(UINT16)(usPosX + INS_CMNT_LINK_WIDTH), INS_CMNT_LINK_Y+INS_CMNT_LINK_HEIGHT+1,
					MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
					SelectInsuranceCommentLinkRegionCallBack);
		MSYS_SetRegionUserData( &gSelectedInsuranceCommentLinkRegion[i], 0, i );

		usPosX += INS_CMNT_LINK_OFFSET_X;
	}

	RenderInsuranceComments();
}


void ExitInsuranceComments()
{
	RemoveInsuranceDefaults();
	DeleteVideoObject(guiInsCmntBulletImage);

	FOR_EACH(MOUSE_REGION, i, gSelectedInsuranceCommentLinkRegion) MSYS_RemoveRegion(&*i);
}


static void DisplayComment(UINT8 ubCommentorsName, UINT8 ubComment, UINT16 usPosY);


void RenderInsuranceComments()
{
	//HVOBJECT hPixHandle;
	ST::string sText;
	UINT16  usPosX, usPosY;

	SetFontShadow( INS_FONT_SHADOW );

	DisplayInsuranceDefaults();

	//Display the title slogan
	sText = GetInsuranceText(INS_SNGL_COMMENTSFROM_CLIENTS);
	DrawTextToScreen(sText, LAPTOP_SCREEN_UL_X, INS_CMNT_TITLE_Y, LAPTOP_SCREEN_LR_X - LAPTOP_SCREEN_UL_X, INS_FONT_BIG, INS_FONT_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	usPosY = INS_CMNT_FIRST_BULLET_Y;

	//Display the commnet from Gus
	DisplayComment( INS_SNGL_GUS_TARBALLS, INS_MLTI_GUS_SPEECH, usPosY );

	usPosY += INS_CMNT_NEXT_COMMENT_OFFSET_Y;

	//Display the commnet from ali hussean
	DisplayComment( INS_SNGL_ALI_HUSSEAN, INS_MLTI_ALI_HUSSEAN_SPEECH, usPosY );

	usPosY += INS_CMNT_NEXT_COMMENT_OFFSET_Y;


	//Display the commnet from Lance allot
	DisplayComment( INS_SNGL_LANCE_ALLOT, INS_MLTI_LANCE_ALLOT_SPEECH, usPosY );

	usPosY += INS_CMNT_NEXT_COMMENT_OFFSET_Y;

	//Display the commnet from Fred Cousteau
	DisplayComment( INS_SNGL_FRED_COUSTEAU, INS_MLTI_FRED_COUSTEAU_SPEECH, usPosY );


	//Display the link text
	usPosX = INS_CMNT_FIRST_BULLET_X-6;

	//Display the first link text
	DisplayWrappedString(usPosX, INS_CMNT_LINK_Y + 13, INS_CMNT_LINK_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, pMessageStrings[MSG_HOMEPAGE], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	//Display the red bar under the link at the bottom
	DisplaySmallRedLineWithShadow( usPosX, INS_CMNT_LINK_Y+INS_CMNT_LINK_HEIGHT, (UINT16)(usPosX+INS_CMNT_LINK_WIDTH), INS_CMNT_LINK_Y+INS_CMNT_LINK_HEIGHT);
	usPosX += INS_CMNT_LINK_OFFSET_X;


	//Display the third link text
	sText = GetInsuranceText(INS_SNGL_HOW_DOES_INS_WORK);
	DisplayWrappedString(usPosX, INS_CMNT_LINK_Y + 6, INS_CMNT_LINK_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	//Display the red bar under the link at the bottom
	DisplaySmallRedLineWithShadow( usPosX, INS_CMNT_LINK_Y+INS_CMNT_LINK_HEIGHT, (UINT16)(usPosX+INS_CMNT_LINK_WIDTH), INS_CMNT_LINK_Y+INS_CMNT_LINK_HEIGHT);
	usPosX += INS_CMNT_LINK_OFFSET_X;

	//Display the fourth link text
	sText = GetInsuranceText(INS_SNGL_TO_ENTER_REVIEW);
	DisplayWrappedString(usPosX, INS_CMNT_LINK_Y - 1, INS_CMNT_LINK_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	//Display the red bar under the link at the bottom
	DisplaySmallRedLineWithShadow( usPosX, INS_CMNT_LINK_Y+INS_CMNT_LINK_HEIGHT, (UINT16)(usPosX+INS_CMNT_LINK_WIDTH), INS_CMNT_LINK_Y+INS_CMNT_LINK_HEIGHT);

	SetFontShadow(DEFAULT_SHADOW);
	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectInsuranceCommentLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_POINTER_UP)
	{
		UINT32 uiInsuranceLink = MSYS_GetRegionUserData( pRegion, 0 );

		if( uiInsuranceLink == 0 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE;
		else if( uiInsuranceLink == 1 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_INFO;
		else if( uiInsuranceLink == 2 )
			guiCurrentLaptopMode = LAPTOP_MODE_INSURANCE_CONTRACT;
	}
}


static void DisplayComment(UINT8 ubCommentorsName, UINT8 ubComment, UINT16 usPosY)
{
	ST::string sText;
	UINT16  sNumPixels=0;

	BltVideoObject(FRAME_BUFFER, guiInsCmntBulletImage, 0, INS_CMNT_FIRST_BULLET_X, usPosY);

	//Display the commenters comment
	sText = GetInsuranceText(ubComment);			//+INS_CMNT_COMMENT_OFFSET_Y
	sNumPixels = DisplayWrappedString(INS_CMNT_FIRST_BULLET_X + INSURANCE_BULLET_TEXT_OFFSET_X, usPosY, INS_CMNT_COMMENT_TEXT_WIDTH, 2, INS_FONT_MED, INS_FONT_COLOR, sText, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED);

	//Display the red bar under the link at the bottom
	DisplaySmallRedLineWithShadow( INS_CMNT_FIRST_BULLET_X+INSURANCE_BULLET_TEXT_OFFSET_X, (UINT16)(usPosY+sNumPixels), INS_CMNT_FIRST_BULLET_X+INS_CMNT_REDLINE_WIDTH, (UINT16)(usPosY+sNumPixels));

	sNumPixels += 4;

	//Display the commenters name
	sText = GetInsuranceText(ubCommentorsName);
	DrawTextToScreen(sText, INS_CMNT_FIRST_BULLET_X + INSURANCE_BULLET_TEXT_OFFSET_X, usPosY + sNumPixels, INS_CMNT_REDLINE_WIDTH, INS_FONT_MED, INS_FONT_COLOR, FONT_MCOLOR_BLACK, RIGHT_JUSTIFIED);
}
