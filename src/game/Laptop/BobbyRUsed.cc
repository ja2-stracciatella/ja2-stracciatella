#include "Directories.h"
#include "Laptop.h"
#include "BobbyRUsed.h"
#include "BobbyR.h"
#include "BobbyRGuns.h"
#include "Button_System.h"
#include "Object_Cache.h"
#include "Video.h"
#include "VSurface.h"

namespace {
cache_key_t const guiUsedBackground{ LAPTOPDIR "/usedbackground.sti" };
cache_key_t const guiUsedGrid{ LAPTOPDIR "/usedgrid.sti" };
}

void EnterBobbyRUsed()
{
	InitBobbyBrTitle();

	SetFirstLastPagesForUsed();

	//Draw menu bar
	InitBobbyMenuBar( );

	RenderBobbyRUsed( );
}


void ExitBobbyRUsed()
{
	RemoveVObject(guiUsedBackground);
	RemoveVObject(guiUsedGrid);
	DeleteBobbyMenuBar();
	DeleteBobbyBrTitle();
	DeleteMouseRegionForBigImage();

	giCurrentSubPage = gusCurWeaponIndex;
	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_USED;
}


void RenderBobbyRUsed()
{
	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES,
		BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT,
		GetVObject(guiUsedBackground));

	//Display title at top of page
	DisplayBobbyRBrTitle();

	BltVideoObject(FRAME_BUFFER, guiUsedGrid, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

	DisplayItemInfo(BOBBYR_USED_ITEMS);

	UpdateButtonText(guiCurrentLaptopMode);
	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}
