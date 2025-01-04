#include "Directories.h"
#include "Item_Types.h"
#include "Laptop.h"
#include "BobbyRMisc.h"
#include "BobbyR.h"
#include "BobbyRGuns.h"
#include "Button_System.h"
#include "Object_Cache.h"
#include "Video.h"
#include "VSurface.h"


namespace {
cache_key_t const guiMiscBackground{ LAPTOPDIR "/miscbackground.sti" };
cache_key_t const guiMiscGrid{ LAPTOPDIR "/miscgrid.sti" };
}

void EnterBobbyRMisc()
{
	InitBobbyBrTitle();
	//Draw menu bar
	InitBobbyMenuBar( );

	SetFirstLastPagesForNew( IC_BOBBY_MISC );

	RenderBobbyRMisc( );
}


void ExitBobbyRMisc()
{
	RemoveVObject(guiMiscBackground);
	RemoveVObject(guiMiscGrid);
	DeleteBobbyBrTitle();
	DeleteMouseRegionForBigImage();
	DeleteBobbyMenuBar();

	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_MISC;
}


void RenderBobbyRMisc()
{
	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES,
		BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT,
		GetVObject(guiMiscBackground));

	//Display title at top of page
	DisplayBobbyRBrTitle();

	BltVideoObject(FRAME_BUFFER, guiMiscGrid, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

	DisplayItemInfo(IC_BOBBY_MISC);

	UpdateButtonText(guiCurrentLaptopMode);
	MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
	InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}
