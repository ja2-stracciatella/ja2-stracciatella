#include "Laptop.h"
#include "BobbyRArmour.h"
#include "BobbyRGuns.h"
#include "BobbyR.h"
#include "Utilities.h"
#include "WCheck.h"
#include "Text.h"
#include "Button_System.h"
#include "Video.h"
#include "VSurface.h"


UINT32		guiArmourBackground;
UINT32		guiArmourGrid;


BOOLEAN EnterBobbyRArmour()
{
	// load the background graphic and add it
	guiArmourBackground = AddVideoObjectFromFile("LAPTOP/Armourbackground.sti");
	CHECKF(guiArmourBackground != NO_VOBJECT);

	// load the gunsgrid graphic and add it
	guiArmourGrid = AddVideoObjectFromFile("LAPTOP/Armourgrid.sti");
	CHECKF(guiArmourGrid != NO_VOBJECT);

	InitBobbyBrTitle();
	//Draw menu bar
	InitBobbyMenuBar( );

	SetFirstLastPagesForNew( IC_ARMOUR );
//	CalculateFirstAndLastIndexs();

	RenderBobbyRArmour( );

	return(TRUE);
}

void ExitBobbyRArmour()
{
	DeleteVideoObjectFromIndex(guiArmourBackground);
	DeleteVideoObjectFromIndex(guiArmourGrid);
	DeleteBobbyMenuBar();

	DeleteBobbyBrTitle();
	DeleteMouseRegionForBigImage();

	giCurrentSubPage = gusCurWeaponIndex;
	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_ARMOR;
}

void HandleBobbyRArmour()
{
}

void RenderBobbyRArmour()
{
	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES, BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiArmourBackground);

	//Display title at top of page
	DisplayBobbyRBrTitle();

	BltVideoObjectFromIndex(FRAME_BUFFER, guiArmourGrid, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

	DisplayItemInfo(IC_ARMOUR);

	UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}
