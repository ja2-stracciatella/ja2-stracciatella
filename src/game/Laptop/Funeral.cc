#include "Directories.h"
#include "Font.h"
#include "Laptop.h"
#include "Funeral.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Cursors.h"
#include "Text.h"
#include "GameRes.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"


#define FUNERAL_SENTENCE_FONT		FONT12ARIAL
#define FUNERAL_SENTENCE_COLOR		2//FONT_MCOLOR_WHITE
#define FUNERAL_SENTENCE_SHADOW_COLOR	FONT_MCOLOR_WHITE//FONT_MCOLOR_DKWHITE

#define FUNERAL_SMALL_FONT		FONT10ARIAL

#define FUNERAL_TITLE_FONT		FONT14ARIAL
#define FUNERAL_TITLE_COLOR		FONT_MCOLOR_WHITE
#define FUNERAL_TITLE_SHADOW_COLOR	FONT_MCOLOR_DKWHITE

#define FUNERAL_RIP_SHADOW_COLOR	FONT_MCOLOR_DKWHITE


#define FUNERAL_MCGILICUTTYS_SIGN_X	LAPTOP_SCREEN_UL_X + 92
#define FUNERAL_MCGILICUTTYS_SIGN_Y	LAPTOP_SCREEN_WEB_UL_Y + 0

#define FUNERAL_MORTUARY_SIGN_X		LAPTOP_SCREEN_UL_X + 58
#define FUNERAL_MORTUARY_SIGN_Y		LAPTOP_SCREEN_WEB_UL_Y + 43

#define FUNERAL_LEFT_COLUMN_X		LAPTOP_SCREEN_UL_X + 0
#define FUNERAL_LEFT_COLUMN_Y		LAPTOP_SCREEN_WEB_UL_Y + 43

#define FUNERAL_RIGHT_COLUMN_X		LAPTOP_SCREEN_UL_X + 442
#define FUNERAL_RIGHT_COLUMN_Y		LAPTOP_SCREEN_WEB_UL_Y + 43

#define FUNERAL_LINK_1_X		LAPTOP_SCREEN_UL_X + 37
#define FUNERAL_LINK_1_Y		LAPTOP_SCREEN_WEB_UL_Y + 329
#define FUNERAL_LINK_1_WIDTH		85
#define FUNERAL_LINK_1_HEIGHT		60

#define FUNERAL_LINK_OFFSET_X		85
#define FUNERAL_NUMBER_OF_LINKS		5

#define FUNERAL_LINK_TEXT_OFFSET_X	4
#define FUNERAL_LINK_TEXT_OFFSET_Y	17
#define FUNERAL_LINK_TEXT_WIDTH		76


#define FUNERAL_MARBLE_WIDTH		125
#define FUNERAL_MARBLE_HEIGHT		100

#define FUNERAL_SENTENCE_WIDTH		380

#define FUNERAL_SENTENCE_1_X		LAPTOP_SCREEN_UL_X + 60
#define FUNERAL_SENTENCE_1_Y		LAPTOP_SCREEN_WEB_UL_Y + 164

#define FUNERAL_SENTENCE_2_X		FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_2_Y		LAPTOP_SCREEN_WEB_UL_Y + 198

#define FUNERAL_SENTENCE_3_X		FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_3_Y		LAPTOP_SCREEN_WEB_UL_Y + 227

#define FUNERAL_SENTENCE_4_X		FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_4_Y		LAPTOP_SCREEN_WEB_UL_Y + 261

#define FUNERAL_SENTENCE_5_X		FUNERAL_SENTENCE_1_X
#define FUNERAL_SENTENCE_5_Y		LAPTOP_SCREEN_WEB_UL_Y + 303


#define FUNERAL_CLOSED_RIP_SIGN_X	LAPTOP_SCREEN_UL_X + 72
#define FUNERAL_CLOSED_RIP_SIGN_Y	LAPTOP_SCREEN_WEB_UL_Y + 151
#define FUNERAL_CLOSED_WIDTH		364
#define FUNERAL_CLOSED_HEIGHT		204

#define FUNERAL_RIP_SENTENCE_WIDTH	260

#define FUNERAL_RIP_SENTENCE_1_X	FUNERAL_CLOSED_RIP_SIGN_X + 55
#define FUNERAL_RIP_SENTENCE_1_Y	FUNERAL_CLOSED_RIP_SIGN_Y + 98

#define FUNERAL_RIP_SENTENCE_2_X	FUNERAL_RIP_SENTENCE_1_X
#define FUNERAL_RIP_SENTENCE_2_Y	FUNERAL_CLOSED_RIP_SIGN_Y + 162


//Image Identifiers
static SGPVObject* guiClosedSign;
static SGPVObject* guiLeftColumn;
static SGPVObject* guiLinkCarving;
static SGPVObject* guiMarbleBackground;
static SGPVObject* guiMcGillicuttys;
static SGPVObject* guiMortuary;
static SGPVObject* guiRightColumn;


//Clicking on Funeral link
static MOUSE_REGION gSelectedFuneralLinkRegion[FUNERAL_NUMBER_OF_LINKS];

//Clicking on rip sign to make it disappear
static MOUSE_REGION gSelectedRipSignRegion;


static void SelectFuneralLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectRipSignRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterFuneral()
{
	UINT16 usPosX, i;

	// load the Closed graphic and add it
	guiClosedSign = AddVideoObjectFromFile(MLG_CLOSED);

	// load the Left column graphic and add it
	guiLeftColumn = AddVideoObjectFromFile(LAPTOPDIR "/leftcolumn.sti");

	// load the Link carving graphic and add it
	guiLinkCarving = AddVideoObjectFromFile(LAPTOPDIR "/linkcarving.sti");

	// load the Marble graphic and add it
	guiMarbleBackground = AddVideoObjectFromFile(LAPTOPDIR "/marble.sti");

	// load the McGillicuttys sign graphic and add it
	guiMcGillicuttys = AddVideoObjectFromFile(MLG_MCGILLICUTTYS);

	// load the Mortuary  graphic and add it
	guiMortuary = AddVideoObjectFromFile(MLG_MORTUARY);

	// load the right column graphic and add it
	guiRightColumn = AddVideoObjectFromFile(LAPTOPDIR "/rightcolumn.sti");


	usPosX = FUNERAL_LINK_1_X;
	for(i=0; i<FUNERAL_NUMBER_OF_LINKS; i++)
	{
		//Mouse region for the bottom links

		MSYS_DefineRegion(&gSelectedFuneralLinkRegion[i], usPosX, FUNERAL_LINK_1_Y,
					(UINT16)(usPosX + FUNERAL_LINK_1_WIDTH),
					(UINT16)(FUNERAL_LINK_1_Y + FUNERAL_LINK_1_HEIGHT),
					MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK,
					SelectFuneralLinkRegionCallBack );
		MSYS_SetRegionUserData( &gSelectedFuneralLinkRegion[i], 0, i );

		usPosX += FUNERAL_LINK_OFFSET_X;
	}

	MSYS_DefineRegion(&gSelectedRipSignRegion, FUNERAL_CLOSED_RIP_SIGN_X, FUNERAL_CLOSED_RIP_SIGN_Y,
				(UINT16)(FUNERAL_CLOSED_RIP_SIGN_X + FUNERAL_CLOSED_WIDTH),
				(UINT16)(FUNERAL_CLOSED_RIP_SIGN_Y + FUNERAL_CLOSED_HEIGHT),
				MSYS_PRIORITY_HIGH+1, CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK,
				SelectRipSignRegionCallBack );
	gSelectedRipSignRegion.Disable();

	SetBookMark( FUNERAL_BOOKMARK );
}


void ExitFuneral()
{
	DeleteVideoObject(guiClosedSign);
	DeleteVideoObject(guiLeftColumn);
	DeleteVideoObject(guiLinkCarving);
	DeleteVideoObject(guiMarbleBackground);
	DeleteVideoObject(guiMcGillicuttys);
	DeleteVideoObject(guiMortuary);
	DeleteVideoObject(guiRightColumn);

	FOR_EACH(MOUSE_REGION, i, gSelectedFuneralLinkRegion) MSYS_RemoveRegion(&*i);
	MSYS_RemoveRegion(&gSelectedRipSignRegion);
}


void RenderFuneral()
{
	UINT16 i, usPosX, usStringHeight;

	WebPageTileBackground(4, 4, FUNERAL_MARBLE_WIDTH, FUNERAL_MARBLE_HEIGHT, guiMarbleBackground);

	BltVideoObject(FRAME_BUFFER, guiLeftColumn,    0, FUNERAL_LEFT_COLUMN_X,       FUNERAL_LEFT_COLUMN_Y);
	BltVideoObject(FRAME_BUFFER, guiMcGillicuttys, 0, FUNERAL_MCGILICUTTYS_SIGN_X, FUNERAL_MCGILICUTTYS_SIGN_Y);
	BltVideoObject(FRAME_BUFFER, guiMortuary,      0, FUNERAL_MORTUARY_SIGN_X,     FUNERAL_MORTUARY_SIGN_Y);
	BltVideoObject(FRAME_BUFFER, guiRightColumn,   0, FUNERAL_RIGHT_COLUMN_X,      FUNERAL_RIGHT_COLUMN_Y);

		// LinkCarving
	usPosX = FUNERAL_LINK_1_X;
	for(i=0; i<FUNERAL_NUMBER_OF_LINKS; i++)
	{
		BltVideoObject(FRAME_BUFFER, guiLinkCarving, 0,usPosX, FUNERAL_LINK_1_Y);

		//Calculate the height of the string, as it needs to be vertically centered.
		usStringHeight = IanWrappedStringHeight(FUNERAL_LINK_TEXT_WIDTH, 2, FUNERAL_SENTENCE_FONT, sFuneralString[i + FUNERAL_SEND_FLOWERS]);
		DisplayWrappedString(usPosX + FUNERAL_LINK_TEXT_OFFSET_X, FUNERAL_LINK_1_Y + (FUNERAL_LINK_1_HEIGHT - usStringHeight) / 2, FUNERAL_LINK_TEXT_WIDTH, 2, FUNERAL_SENTENCE_FONT, FUNERAL_TITLE_COLOR, sFuneralString[i + FUNERAL_SEND_FLOWERS], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

		usPosX += FUNERAL_LINK_OFFSET_X;
	}

	//display all the sentences

	// sentence 1
	DisplayWrappedString(FUNERAL_SENTENCE_1_X, FUNERAL_SENTENCE_1_Y, FUNERAL_SENTENCE_WIDTH, 2, FUNERAL_TITLE_FONT, FUNERAL_TITLE_COLOR, sFuneralString[FUNERAL_INTRO_1], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	SetFontShadow( FUNERAL_SENTENCE_SHADOW_COLOR );

	// sentence 2
	DisplayWrappedString(FUNERAL_SENTENCE_2_X, FUNERAL_SENTENCE_2_Y, FUNERAL_SENTENCE_WIDTH, 2, FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR, sFuneralString[FUNERAL_INTRO_2], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	// sentence 3
	DisplayWrappedString(FUNERAL_SENTENCE_3_X, FUNERAL_SENTENCE_3_Y, FUNERAL_SENTENCE_WIDTH, 2, FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR, sFuneralString[FUNERAL_INTRO_3], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	// sentence 4
	DisplayWrappedString(FUNERAL_SENTENCE_4_X, FUNERAL_SENTENCE_4_Y, FUNERAL_SENTENCE_WIDTH, 2, FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR, sFuneralString[FUNERAL_INTRO_4], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	// sentence 5
	DisplayWrappedString(FUNERAL_SENTENCE_5_X, FUNERAL_SENTENCE_5_Y, FUNERAL_SENTENCE_WIDTH, 2, FUNERAL_SENTENCE_FONT, FUNERAL_SENTENCE_COLOR, sFuneralString[FUNERAL_INTRO_5], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	SetFontShadow(DEFAULT_SHADOW);


	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void DisplayFuneralRipTombStone(void)
{
	// rip tombstone
	BltVideoObjectOutlineShadow(FRAME_BUFFER, guiClosedSign, 0, FUNERAL_CLOSED_RIP_SIGN_X + g_ui.m_stdScreenScale * 5, FUNERAL_CLOSED_RIP_SIGN_Y + g_ui.m_stdScreenScale * 5);
	BltVideoObject(             FRAME_BUFFER, guiClosedSign, 0, FUNERAL_CLOSED_RIP_SIGN_X,     FUNERAL_CLOSED_RIP_SIGN_Y);

	SetFontShadow( FUNERAL_RIP_SHADOW_COLOR );

	// sentence 10
	DisplayWrappedString(FUNERAL_RIP_SENTENCE_1_X, FUNERAL_RIP_SENTENCE_1_Y, FUNERAL_RIP_SENTENCE_WIDTH, 2, FUNERAL_SMALL_FONT, FUNERAL_SENTENCE_COLOR, sFuneralString[FUNERAL_OUR_CONDOLENCES], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	// sentence 11
	DisplayWrappedString(FUNERAL_RIP_SENTENCE_2_X, FUNERAL_RIP_SENTENCE_2_Y, FUNERAL_RIP_SENTENCE_WIDTH, 2, FUNERAL_SMALL_FONT, FUNERAL_SENTENCE_COLOR, sFuneralString[FUNERAL_OUR_SYMPATHIES], FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	SetFontShadow(DEFAULT_SHADOW);

	InvalidateRegion(FUNERAL_CLOSED_RIP_SIGN_X,FUNERAL_CLOSED_RIP_SIGN_Y,  FUNERAL_CLOSED_RIP_SIGN_X+FUNERAL_CLOSED_WIDTH+5, FUNERAL_CLOSED_RIP_SIGN_Y+FUNERAL_CLOSED_HEIGHT+5);

	//enable the region to make the sign disappear
	gSelectedRipSignRegion.Enable();
}


static void SelectFuneralLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT32	uiUserData;

		uiUserData = MSYS_GetRegionUserData( pRegion, 0 );

		if( uiUserData == 0 )
			GoToWebPage( FLORIST_BOOKMARK );
		else
		{
			RenderFuneral();
			DisplayFuneralRipTombStone();
		}

	}
}


static void SelectRipSignRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		gSelectedRipSignRegion.Disable();
		fPausedReDrawScreenFlag = TRUE;
	}
}
