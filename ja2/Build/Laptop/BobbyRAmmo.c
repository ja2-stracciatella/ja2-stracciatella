#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "laptop.h"
	#include "BobbyRAmmo.h"
	#include "BobbyRGuns.h"
	#include "BobbyR.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "WordWrap.h"
	#include "Cursors.h"
	#include "interface items.h"
	#include "Encrypted File.h"
	#include "text.h"
#endif

UINT32		guiAmmoBackground;
UINT32		guiAmmoGrid;

BOOLEAN DisplayAmmoInfo();


void GameInitBobbyRAmmo()
{
}

BOOLEAN EnterBobbyRAmmo()
{
  VOBJECT_DESC    VObjectDesc;

	// load the background graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\ammobackground.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiAmmoBackground));

	// load the gunsgrid graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\ammogrid.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiAmmoGrid));

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
  BltVideoObject(FRAME_BUFFER, hPixHandle, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	DisplayItemInfo(IC_AMMO);

	UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}

