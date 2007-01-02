#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "BobbyRUsed.h"
	#include "BobbyR.h"
	#include "BobbyRGuns.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "WordWrap.h"
	#include "Cursors.h"
	#include "Text.h"
	#include "LaptopSave.h"
	#include "Button_System.h"
	#include "Video.h"
	#include "VSurface.h"
#endif

UINT32		guiUsedBackground;
UINT32		guiUsedGrid;


void GameInitBobbyRUsed()
{

}

BOOLEAN EnterBobbyRUsed()
{
	// load the background graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP\\usedbackground.sti", &guiUsedBackground));

	// load the gunsgrid graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP\\usedgrid.sti", &guiUsedGrid));

	InitBobbyBrTitle();

	SetFirstLastPagesForUsed();
//	CalculateFirstAndLastIndexs();

	//Draw menu bar
	InitBobbyMenuBar( );

	RenderBobbyRUsed( );

	return(TRUE);
}

void ExitBobbyRUsed()
{
	DeleteVideoObjectFromIndex(guiUsedBackground);
	DeleteVideoObjectFromIndex(guiUsedGrid);
	DeleteBobbyMenuBar();
	DeleteBobbyBrTitle();
	DeleteMouseRegionForBigImage();

	giCurrentSubPage = gusCurWeaponIndex;
	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_USED;
}

void HandleBobbyRUsed()
{
}

void RenderBobbyRUsed()
{
	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES, BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiUsedBackground);

	//Display title at top of page
	DisplayBobbyRBrTitle();

	BltVideoObjectFromIndex(FRAME_BUFFER, guiUsedGrid, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

	DisplayItemInfo(BOBBYR_USED_ITEMS);

	UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}
