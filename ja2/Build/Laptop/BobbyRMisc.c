#ifdef PRECOMPILEDHEADERS
	#include "Laptop All.h"
#else
	#include "laptop.h"
	#include "BobbyRMisc.h"
	#include "BobbyR.h"
	#include "BobbyRGuns.h"
	#include "Utilities.h"
	#include "WCheck.h"
	#include "WordWrap.h"
	#include "Cursors.h"
	#include "Text.h"
#endif


UINT32		guiMiscBackground;
UINT32		guiMiscGrid;



void GameInitBobbyRMisc()
{

}

BOOLEAN EnterBobbyRMisc()
{
  VOBJECT_DESC    VObjectDesc;

	// load the background graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\miscbackground.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiMiscBackground));

	// load the gunsgrid graphic and add it
	VObjectDesc.fCreateFlags=VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("LAPTOP\\miscgrid.sti", VObjectDesc.ImageFile);
	CHECKF(AddVideoObject(&VObjectDesc, &guiMiscGrid));

	InitBobbyBrTitle();
	//Draw menu bar
	InitBobbyMenuBar( );

	SetFirstLastPagesForNew( IC_BOBBY_MISC );
//	CalculateFirstAndLastIndexs();

	RenderBobbyRMisc( );

	return(TRUE);
}

void ExitBobbyRMisc()
{
	DeleteVideoObjectFromIndex(guiMiscBackground);
	DeleteVideoObjectFromIndex(guiMiscGrid);
	DeleteBobbyBrTitle();
	DeleteMouseRegionForBigImage();
	DeleteBobbyMenuBar();

	guiLastBobbyRayPage = LAPTOP_MODE_BOBBY_R_MISC;
}

void HandleBobbyRMisc()
{
}

void RenderBobbyRMisc()
{
  HVOBJECT hPixHandle;

	WebPageTileBackground(BOBBYR_NUM_HORIZONTAL_TILES, BOBBYR_NUM_VERTICAL_TILES, BOBBYR_BACKGROUND_WIDTH, BOBBYR_BACKGROUND_HEIGHT, guiMiscBackground);

	//Display title at top of page
	DisplayBobbyRBrTitle();

	// GunForm
	GetVideoObject(&hPixHandle, guiMiscGrid);
  BltVideoObject(FRAME_BUFFER, hPixHandle, 0, BOBBYR_GRIDLOC_X, BOBBYR_GRIDLOC_Y, VO_BLT_SRCTRANSPARENCY,NULL);

	DisplayItemInfo(IC_BOBBY_MISC);

	UpdateButtonText(guiCurrentLaptopMode);
  MarkButtonsDirty( );
	RenderWWWProgramTitleBar( );
  InvalidateRegion(LAPTOP_SCREEN_UL_X,LAPTOP_SCREEN_WEB_UL_Y,LAPTOP_SCREEN_LR_X,LAPTOP_SCREEN_WEB_LR_Y);
}



