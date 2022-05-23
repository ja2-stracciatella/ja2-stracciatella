#include "Cursors.h"
#include "Directories.h"
#include "HImage.h"
#include "Laptop.h"
#include "AIMFacialIndex.h"
#include "MercPortrait.h"
#include "Merc_Hiring.h"
#include "Soldier_Control.h"
#include "VObject.h"
#include "WordWrap.h"
#include "AIM.h"
#include "Soldier_Profile.h"
#include "Text.h"
#include "AIMSort.h"
#include "Assignments.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"

#include <string_theory/string>


extern UINT8			gbCurrentIndex;


static SGPVObject* guiMugShotBorder;
static SGPVObject* guiAimFiFace[MAX_NUMBER_MERCS];



#define AIM_FI_NUM_MUHSHOTS_X		8
#define AIM_FI_NUM_MUHSHOTS_Y		5

#define AIM_FI_PORTRAIT_WIDTH		52
#define AIM_FI_PORTRAIT_HEIGHT		48

#define AIM_FI_FIRST_MUGSHOT_X		IMAGE_OFFSET_X + 6
#define AIM_FI_FIRST_MUGSHOT_Y		IMAGE_OFFSET_Y + 69//67//70 //68 //65
#define AIM_FI_MUGSHOT_GAP_X		10
#define AIM_FI_MUGSHOT_GAP_Y		13
#define AIM_FI_FACE_OFFSET		2

#define AIM_FI_NNAME_OFFSET_X		2
#define AIM_FI_NNAME_OFFSET_Y		AIM_FI_PORTRAIT_HEIGHT+1
#define AIM_FI_NNAME_WIDTH		AIM_FI_PORTRAIT_WIDTH+4

#define AIM_FI_MEMBER_TEXT_X		IMAGE_OFFSET_X + 155
#define AIM_FI_MEMBER_TEXT_Y		AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y + 1
#define AIM_FI_MEMBER_TEXT_WIDTH	190

#define AIM_FI_AWAY_TEXT_OFFSET_X	3
#define AIM_FI_AWAY_TEXT_OFFSET_Y	23//3//36
#define AIM_FI_AWAY_TEXT_OFFSET_WIDTH	48


//Mouse Regions

//Face regions
static MOUSE_REGION gMercFaceMouseRegions[MAX_NUMBER_MERCS];

//Screen region, used to right click to go back to previous page
static MOUSE_REGION gScreenMouseRegions;


static void SelectMercFaceMoveRegionCallBack(MOUSE_REGION* pRegion, UINT32 reason);
static void SelectMercFaceRegionCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectMercFaceRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectScreenRegionCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason);
static void SelectScreenRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterAimFacialIndex()
{
	UINT8	i;
	UINT16		usPosX, usPosY, x,y;

	// load the Portait graphic and add it
	guiMugShotBorder = AddVideoObjectFromFile(LAPTOPDIR "/mugshotborder3.sti");

	usPosX = AIM_FI_FIRST_MUGSHOT_X;
	usPosY = AIM_FI_FIRST_MUGSHOT_Y;
	i=0;
	for(y=0; y<AIM_FI_NUM_MUHSHOTS_Y; y++)
	{
		for(x=0; x<AIM_FI_NUM_MUHSHOTS_X; x++)
		{

			MSYS_DefineRegion(&gMercFaceMouseRegions[ i ], usPosX, usPosY,
						(INT16)(usPosX + AIM_FI_PORTRAIT_WIDTH),
						(INT16)(usPosY + AIM_FI_PORTRAIT_HEIGHT),
						MSYS_PRIORITY_HIGH,
						CURSOR_WWW, SelectMercFaceMoveRegionCallBack,
						MouseCallbackPrimarySecondary<MOUSE_REGION>(SelectMercFaceRegionCallBackPrimary, SelectMercFaceRegionCallBackSecondary));
			MSYS_SetRegionUserData( &gMercFaceMouseRegions[ i ], 0, i);

			guiAimFiFace[i] = LoadSmallPortrait(GetProfile(AimMercArray[i]));

			usPosX += AIM_FI_PORTRAIT_WIDTH + AIM_FI_MUGSHOT_GAP_X;
			i++;
		}
		usPosX = AIM_FI_FIRST_MUGSHOT_X;
		usPosY += AIM_FI_PORTRAIT_HEIGHT + AIM_FI_MUGSHOT_GAP_Y;
	}

	MSYS_DefineRegion(&gScreenMouseRegions, LAPTOP_SCREEN_UL_X, LAPTOP_SCREEN_WEB_UL_Y,
				LAPTOP_SCREEN_LR_X, LAPTOP_SCREEN_WEB_LR_Y, MSYS_PRIORITY_HIGH-1,
				CURSOR_LAPTOP_SCREEN, MSYS_NO_CALLBACK, MouseCallbackPrimarySecondary<MOUSE_REGION>(MSYS_NO_CALLBACK, SelectScreenRegionCallBackSecondary));

	InitAimMenuBar();
	InitAimDefaults();

	RenderAimFacialIndex();
}


void ExitAimFacialIndex()
{
	RemoveAimDefaults();

	DeleteVideoObject(guiMugShotBorder);

	FOR_EACH(SGPVObject*,  i, guiAimFiFace)          DeleteVideoObject(*i);
	FOR_EACH(MOUSE_REGION, i, gMercFaceMouseRegions) MSYS_RemoveRegion(&*i);
	ExitAimMenuBar();

	MSYS_RemoveRegion(&gScreenMouseRegions);
}


static void DrawMercsFaceToScreen(UINT8 ubMercID, UINT16 usPosX, UINT16 usPosY, UINT8 ubImage);


void RenderAimFacialIndex()
{
	UINT16		usPosX, usPosY, x,y;
	ST::string sString;
	UINT8			i;

	DrawAimDefaults();

	//Display the 'A.I.M. Members Sorted Ascending By Price' type string
	if( gubCurrentListMode == AIM_ASCEND )
		sString = st_format_printf(AimFiText[ AIM_FI_AIM_MEMBERS_SORTED_ASCENDING ], AimFiText[gubCurrentSortMode]);
	else
		sString = st_format_printf(AimFiText[ AIM_FI_AIM_MEMBERS_SORTED_DESCENDING ], AimFiText[gubCurrentSortMode]);

	DrawTextToScreen(sString, AIM_FI_MEMBER_TEXT_X, AIM_FI_MEMBER_TEXT_Y, AIM_FI_MEMBER_TEXT_WIDTH, AIM_MAINTITLE_FONT, AIM_MAINTITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);


	//Draw the mug shot border and face
	usPosX = AIM_FI_FIRST_MUGSHOT_X;
	usPosY = AIM_FI_FIRST_MUGSHOT_Y;

	i=0;
	for(y=0; y<AIM_FI_NUM_MUHSHOTS_Y; y++)
	{
		for(x=0; x<AIM_FI_NUM_MUHSHOTS_X; x++)
		{
			DrawMercsFaceToScreen(i, usPosX, usPosY, 1);
			DrawTextToScreen(gMercProfiles[AimMercArray[i]].zNickname, usPosX - AIM_FI_NNAME_OFFSET_X, usPosY + AIM_FI_NNAME_OFFSET_Y, AIM_FI_NNAME_WIDTH, AIM_FONT12ARIAL, AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

			usPosX += AIM_FI_PORTRAIT_WIDTH + AIM_FI_MUGSHOT_GAP_X;
			i++;
		}
		usPosX = AIM_FI_FIRST_MUGSHOT_X;
		usPosY += AIM_FI_PORTRAIT_HEIGHT + AIM_FI_MUGSHOT_GAP_Y;
	}

	DisableAimButton();

	//display the 'left and right click' onscreen help msg
	DrawTextToScreen(AimFiText[AIM_FI_LEFT_CLICK], AIM_FI_LEFT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y,                                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_TITLE_FONT, AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(AimFiText[AIM_FI_TO_SELECT],  AIM_FI_LEFT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y + AIM_FI_CLICK_DESC_TEXT_Y_OFFSET, AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_FONT,       AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	DrawTextToScreen(AimFiText[AIM_FI_RIGHT_CLICK],        AIM_FI_RIGHT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y,                                   AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_TITLE_FONT, AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	DrawTextToScreen(AimFiText[AIM_FI_TO_ENTER_SORT_PAGE], AIM_FI_RIGHT_CLICK_TEXT_X, AIM_FI_LEFT_CLICK_TEXT_Y + AIM_FI_CLICK_DESC_TEXT_Y_OFFSET, AIM_FI_CLICK_TEXT_WIDTH, AIM_FI_HELP_FONT,       AIM_FONT_MCOLOR_WHITE, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	MarkButtonsDirty( );

	RenderWWWProgramTitleBar( );

	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectMercFaceRegionCallBackPrimary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS;
	gbCurrentIndex = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );
}

static void SelectMercFaceRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES;
}


static void SelectScreenRegionCallBackSecondary(MOUSE_REGION* pRegion, UINT32 iReason)
{
	guiCurrentLaptopMode = LAPTOP_MODE_AIM_MEMBERS_SORTED_FILES;
}


static void SelectMercFaceMoveRegionCallBack(MOUSE_REGION* pRegion, UINT32 reason)
{
	UINT8	ubMercNum;
	UINT16 usPosX, usPosY;

	ubMercNum = (UINT8) MSYS_GetRegionUserData( pRegion, 0 );

	usPosY = ubMercNum / AIM_FI_NUM_MUHSHOTS_X;
	usPosY = AIM_FI_FIRST_MUGSHOT_Y + (AIM_FI_PORTRAIT_HEIGHT + AIM_FI_MUGSHOT_GAP_Y) * usPosY;

	usPosX = ubMercNum % AIM_FI_NUM_MUHSHOTS_X;
	usPosX = AIM_FI_FIRST_MUGSHOT_X + (AIM_FI_PORTRAIT_WIDTH + AIM_FI_MUGSHOT_GAP_X) * usPosX;

	//fReDrawNewMailFlag = TRUE;

	if( reason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		DrawMercsFaceToScreen(ubMercNum, usPosX, usPosY, 1);
		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
	else if( reason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		DrawMercsFaceToScreen(ubMercNum, usPosX, usPosY, 0);
		InvalidateRegion(pRegion->RegionTopLeftX, pRegion->RegionTopLeftY, pRegion->RegionBottomRightX, pRegion->RegionBottomRightY);
	}
}


static void DrawMercsFaceToScreen(const UINT8 ubMercID, const UINT16 usPosX, const UINT16 usPosY, const UINT8 ubImage)
{
	const ProfileID          id = AimMercArray[ubMercID];
	const SOLDIERTYPE* const s  = FindSoldierByProfileIDOnPlayerTeam(id);

	//Blt the portrait background
	BltVideoObject(FRAME_BUFFER, guiMugShotBorder, ubImage, usPosX, usPosY);

	SGPVObject* const face = guiAimFiFace[ubMercID];

	BOOLEAN                  shaded;
	ST::string text;
	MERCPROFILESTRUCT const& p = GetProfile(id);
	if (IsMercDead(p))
	{
		// the merc is dead, so shade the face red
		face->pShades[0] = Create16BPPPaletteShaded(face->Palette(), DEAD_MERC_COLOR_RED, DEAD_MERC_COLOR_GREEN, DEAD_MERC_COLOR_BLUE, TRUE);
		face->CurrentShade(0);
		shaded = FALSE;
		text   = AimFiText[AIM_FI_DEAD];
	}
	else if (p.bMercStatus == MERC_FIRED_AS_A_POW || (s && s->bAssignment == ASSIGNMENT_POW))
	{
		// the merc is currently a POW or, the merc was fired as a pow
		shaded = TRUE;
		text   = pPOWStrings[0];
	}
	else if (s != NULL)
	{
		// the merc is on our team
		shaded = TRUE;
		text   = MercInfo[MERC_FILES_ALREADY_HIRED];
	}
	else if (!IsMercHireable(p))
	{
		// the merc is away, shadow his/her face and blit 'away' over top
		shaded = TRUE;
		text   = AimFiText[AIM_FI_DEAD + 1];
	}
	else
	{
		shaded = FALSE;
		text   = ST::null;
	}

	BltVideoObject(FRAME_BUFFER, face, 0, usPosX + AIM_FI_FACE_OFFSET, usPosY + AIM_FI_FACE_OFFSET);

	if (shaded)
	{
		FRAME_BUFFER->ShadowRect(usPosX + AIM_FI_FACE_OFFSET, usPosY + AIM_FI_FACE_OFFSET, usPosX + 48 + AIM_FI_FACE_OFFSET, usPosY + 43 + AIM_FI_FACE_OFFSET);
	}

	if (text != NULL)
	{
		DrawTextToScreen(text, usPosX + AIM_FI_AWAY_TEXT_OFFSET_X, usPosY + AIM_FI_AWAY_TEXT_OFFSET_Y, AIM_FI_AWAY_TEXT_OFFSET_WIDTH, FONT10ARIAL, 145, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);
	}
}
