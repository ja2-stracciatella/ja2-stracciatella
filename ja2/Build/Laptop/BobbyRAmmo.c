#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "Laptop.h"
	#include "BobbyRAmmo.h"
	#include "BobbyRGuns.h"
	#include "BobbyR.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "WordWrap.h"
	#include "Cursors.h"
	#include "Interface_Items.h"
	#include "Encrypted_File.h"
	#include "Text.h"
	#include "Button_System.h"
	#include "Video.h"
	#include "VSurface.h"
#endif

UINT32		guiAmmoBackground;
UINT32		guiAmmoGrid;

BOOLEAN DisplayAmmoInfo();


void GameInitBobbyRAmmo()
{
}

BOOLEAN EnterBobbyRAmmo()
{
	// load the background graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP\\ammobackground.sti", &guiAmmoBackground));

	// load the gunsgrid graphic and add it
	CHECKF(AddVideoObjectFromFile("LAPTOP\\ammogrid.sti", &guiAmmoGrid));

	InitBobbyBrTitle();


	SetFirstLastPagesForNew( IC_AMMO );
//	CalculateFirstAndLastIndexs();

	//Draw menu bar
	InitBobbyMenuBar( );

	RenderBobbyRAmmo( );

	return(TRUE);
}

void ExitBobbyRAmmo()
{
	DeleteVideoObjectFromIndex(guiAmmoBackground);
	DeleteVideoObjectFromIndex(guiAmmoGrid);
	DeleteBobbyMenuBar();

	DeleteBobbyBrTitle();
	DeleteMouseRegionForBigImage();

	giCurrentSubPage = gusCurWeaponIndex;
	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_AMMO;
}

void HandleBobbyRAmmo()
{
}

void RenderBobbyRAmmo()
{
  HVOBJECT hPixHandle;

	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES, BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiAmmoBackground);

	//Display title at top of page
	DisplayBobbyRBrTitle();

	// GunForm
	GetVideoObject(&hPixHandle, guiAmmoGrid);
  BltVideoObject(FRAME_BUFFER, hPixHandle, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y);

	DisplayItemInfo(IC_AMMO);

	UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}
