#include "Directories.h"
#include "Laptop.h"
#include "BobbyRUsed.h"
#include "BobbyR.h"
#include "BobbyRGuns.h"
#include "VObject.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"


static SGPVObject* guiUsedBackground;
static SGPVObject* guiUsedGrid;


void EnterBobbyRUsed()
{
	// load the background graphic and add it
	guiUsedBackground = AddVideoObjectFromFile(LAPTOPDIR "/usedbackground.sti");

	// load the gunsgrid graphic and add it
	guiUsedGrid = AddVideoObjectFromFile(LAPTOPDIR "/usedgrid.sti");

	InitBobbyBrTitle();

	SetFirstLastPagesForUsed();

	//Draw menu bar
	InitBobbyMenuBar( );

	RenderBobbyRUsed( );
}


void ExitBobbyRUsed()
{
	delete guiUsedBackground;
	delete guiUsedGrid;
	DeleteBobbyMenuBar();
	DeleteBobbyBrTitle();
	DeleteMouseRegionForBigImage();

	giCurrentSubPage = gusCurWeaponIndex;
	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_USED;
}


void RenderBobbyRUsed()
{
	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES, BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiUsedBackground);

	//Display title at top of page
	DisplayBobbyRBrTitle();

	BltVideoObject(FRAME_BUFFER, guiUsedGrid, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

	DisplayItemInfo(BOBBYR_USED_ITEMS);

	UpdateButtonText(guiCurrentLaptopMode);
	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}
