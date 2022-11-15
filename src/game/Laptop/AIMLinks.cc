#include "Cursors.h"
#include "Laptop.h"
#include "AIMLinks.h"
#include "AIM.h"
#include "VObject.h"
#include "WordWrap.h"
#include "Text.h"
#include "GameRes.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"
#include "Font_Control.h"


#define AIM_LINK_TITLE_FONT		FONT14ARIAL
#define AIM_LINK_TITLE_COLOR		AIM_GREEN

#define AIM_LINK_NUM_LINKS		3

#define AIM_LINK_LINK_OFFSET_X		94
#define AIM_LINK_LINK_OFFSET_Y		94//90

#define AIM_LINK_LINK_WIDTH		420
#define AIM_LINK_LINK_HEIGHT		70

#define AIM_LINK_BOBBY_LINK_X		LAPTOP_SCREEN_UL_X + 40
#define AIM_LINK_BOBBY_LINK_Y		LAPTOP_SCREEN_WEB_UL_Y + 91

#define AIM_LINK_FUNERAL_LINK_X		AIM_LINK_BOBBY_LINK_X
#define AIM_LINK_FUNERAL_LINK_Y		AIM_LINK_BOBBY_LINK_Y + AIM_LINK_LINK_OFFSET_Y

#define AIM_LINK_INSURANCE_LINK_X	AIM_LINK_BOBBY_LINK_X
#define AIM_LINK_INSURANCE_LINK_Y	AIM_LINK_FUNERAL_LINK_Y + AIM_LINK_LINK_OFFSET_Y

#define AIM_LINK_TITLE_X		IMAGE_OFFSET_X + 149
#define AIM_LINK_TITLE_Y		AIM_SYMBOL_Y + AIM_SYMBOL_SIZE_Y + 10
#define AIM_LINK_TITLE_WIDTH		AIM_SYMBOL_WIDTH


static SGPVObject* guiBobbyLink;
static SGPVObject* guiFuneralLink;
static SGPVObject* guiInsuranceLink;
static UINT8 const gubLinkPages[] = { BOBBYR_BOOKMARK, FUNERAL_BOOKMARK, INSURANCE_BOOKMARK };

//Clicking on guys Face
static MOUSE_REGION gSelectedLinkRegion[AIM_LINK_NUM_LINKS];


static void SelectLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason);


void EnterAimLinks()
{
	InitAimDefaults();
	InitAimMenuBar();

	// Load the Bobby link graphic.
	guiBobbyLink = AddVideoObjectFromFile(GetMLGFilename(MLG_BOBBYRAYLINK));
	// Load the Funeral graphic.
	guiFuneralLink = AddVideoObjectFromFile(GetMLGFilename(MLG_MORTUARYLINK));
	// Load the Insurance graphic.
	guiInsuranceLink = AddVideoObjectFromFile(GetMLGFilename(MLG_INSURANCELINK));

	UINT16 const  x    = STD_SCREEN_X + AIM_LINK_LINK_OFFSET_X;
	UINT16        y    = AIM_LINK_BOBBY_LINK_Y;
	UINT8  const* page = gubLinkPages;
	FOR_EACHX(MOUSE_REGION, i, gSelectedLinkRegion, y += AIM_LINK_LINK_OFFSET_Y)
	{
		MOUSE_REGION& r = *i;
		MSYS_DefineRegion(&r, x, y, x + AIM_LINK_LINK_WIDTH, y + AIM_LINK_LINK_HEIGHT, MSYS_PRIORITY_HIGH, CURSOR_WWW, MSYS_NO_CALLBACK, SelectLinkRegionCallBack);
		MSYS_SetRegionUserData(&r, 0, *page++);
	}

	RenderAimLinks();
}


void ExitAimLinks()
{
	RemoveAimDefaults();

	DeleteVideoObject(guiBobbyLink);
	DeleteVideoObject(guiFuneralLink);
	DeleteVideoObject(guiInsuranceLink);

	FOR_EACH(MOUSE_REGION, i, gSelectedLinkRegion) MSYS_RemoveRegion(&*i);

	ExitAimMenuBar();
}


void RenderAimLinks()
{
	DrawAimDefaults();
	DisableAimButton();

	BltVideoObject(FRAME_BUFFER, guiBobbyLink,     0, AIM_LINK_BOBBY_LINK_X,     AIM_LINK_BOBBY_LINK_Y);
	BltVideoObject(FRAME_BUFFER, guiFuneralLink,   0, AIM_LINK_FUNERAL_LINK_X,   AIM_LINK_FUNERAL_LINK_Y);
	BltVideoObject(FRAME_BUFFER, guiInsuranceLink, 0, AIM_LINK_INSURANCE_LINK_X, AIM_LINK_INSURANCE_LINK_Y);

	//Draw Link Title
	DrawTextToScreen(AimLinkText, AIM_LINK_TITLE_X, AIM_LINK_TITLE_Y, AIM_LINK_TITLE_WIDTH, AIM_LINK_TITLE_FONT, AIM_LINK_TITLE_COLOR, FONT_MCOLOR_BLACK, CENTER_JUSTIFIED);

	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}


static void SelectLinkRegionCallBack(MOUSE_REGION* pRegion, UINT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_POINTER_UP)
	{
		UINT32	gNextLaptopPage;

		gNextLaptopPage = MSYS_GetRegionUserData( pRegion, 0 );

		GoToWebPage( gNextLaptopPage );
	}
}
