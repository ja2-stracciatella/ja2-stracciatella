#include "Font_Control.h"
#include "Interface.h"
#include "Types.h"
#include "VSurface.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "WCheck.h"
#include "Utilities.h"
#include "Message.h"
#include "SysUtil.h"
#include "MapScreen.h"
#include "Render_Dirty.h"
#include "Map_Screen_Interface_Map.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Map_Screen_Helicopter.h"
#include "Text.h"
#include "Campaign_Types.h"
#include "Strategic_Town_Loyalty.h"
#include "StrategicMap.h"
#include "Assignments.h"
#include "Debug.h"
#include "Video.h"


#define MAP_BORDER_X 261
#define MAP_BORDER_Y 0

#define MAP_BORDER_CORNER_X 584
#define MAP_BORDER_CORNER_Y 279


// mouse levels
MOUSE_REGION LevelMouseRegions[ 4 ];

// graphics
UINT32 guiMapBorder;
//UINT32 guiMapBorderCorner;


// scroll direction
INT32 giScrollButtonState = -1;

// flags
BOOLEAN fShowTownFlag = FALSE;
BOOLEAN fShowMineFlag = FALSE;
BOOLEAN fShowTeamFlag = FALSE;
BOOLEAN fShowMilitia = FALSE;
BOOLEAN fShowAircraftFlag = FALSE;
BOOLEAN fShowItemsFlag = FALSE;

BOOLEAN fZoomFlag = FALSE;
//BOOLEAN fShowVehicleFlag = FALSE;

//BOOLEAN fMapScrollDueToPanelButton = FALSE;
//BOOLEAN fCursorIsOnMapScrollButtons = FALSE;
//BOOLEAN fDisabledMapBorder = FALSE;


// buttons & button images
INT32 giMapBorderButtons[ 6 ] = { -1, -1, -1, -1, -1, -1 };
INT32 giMapBorderButtonsImage[ 6 ] = { -1, -1, -1, -1, -1, -1 };

//UINT32 guiMapBorderScrollButtons[ 4 ] = { -1, -1, -1, -1 };

// raise/lower land buttons
//UINT32 guiMapBorderLandRaiseButtons[ 2 ] = { -1, -1 };
//UINT32 guiMapBorderLandRaiseButtonsImage[ 2 ];


//void MapScrollButtonMvtCheck( void );
//BOOLEAN ScrollButtonsDisplayingHelpMessage( void );
//void UpdateScrollButtonStatesWhileScrolling( void );

extern void CancelMapUIMessage( void );


//void BtnZoomCallback(GUI_BUTTON *btn,INT32 reason);


/*
void BtnScrollNorthMapScreenCallback( GUI_BUTTON *btn,INT32 reason );
void BtnScrollSouthMapScreenCallback( GUI_BUTTON *btn,INT32 reason );
void BtnScrollWestMapScreenCallback( GUI_BUTTON *btn,INT32 reason );
void BtnScrollEastMapScreenCallback( GUI_BUTTON *btn,INT32 reason );
void BtnLowerLevelBtnCallback(GUI_BUTTON *btn,INT32 reason);
void BtnRaiseLevelBtnCallback(GUI_BUTTON *btn,INT32 reason);
*/


BOOLEAN LoadMapBorderGraphics( void )
{
  // this procedure will load the graphics needed for the map border
	guiMapBorder = AddVideoObjectFromFile("INTERFACE/MBS.sti");
	CHECKF(guiMapBorder != NO_VOBJECT);

/* corner was removed along with the Zoom feature
	// will load map border corner
	guiMapBorderCorner = AddVideoObjectFromFile("INTERFACE/map_screen_cutout.sti");
	CHECKF(guiMapBorderCorner != NO_VOBJECT);

	fCursorIsOnMapScrollButtons = FALSE;
*/


	return ( TRUE );
}


void DeleteMapBorderGraphics( void )
{
	// procedure will delete graphics loaded for map border

	DeleteVideoObjectFromIndex( guiMapBorder );
//	DeleteVideoObjectFromIndex( guiMapBorderCorner );
}


static void DisplayCurrentLevelMarker(void);


void RenderMapBorder( void )
{
	// renders the actual border to the guiSAVEBUFFER
/*
	if( fDisabledMapBorder )
	{
		return;
	}
*/

	if( fShowMapInventoryPool )
	{
		// render background, then leave
		BlitInventoryPoolGraphic( );
		return;
	}

	BltVideoObjectFromIndex(guiSAVEBUFFER , guiMapBorder, 0, MAP_BORDER_X, MAP_BORDER_Y);

	// show the level marker
	DisplayCurrentLevelMarker( );
}


/*
void RenderMapBorderCorner( void )
{
	// renders map border corner to the FRAME_BUFFER
	if( fDisabledMapBorder )
	{
		return;
	}

	if( fShowMapInventoryPool )
	{
		return;
	}

	BltVideoObjectFromIndex(FRAME_BUFFER, guiMapBorderCorner, 0, MAP_BORDER_CORNER_X, MAP_BORDER_CORNER_Y);

	InvalidateRegion( MAP_BORDER_CORNER_X, MAP_BORDER_CORNER_Y, 635, 315);
}
*/


void RenderMapBorderEtaPopUp( void )
{
	// renders map border corner to the FRAME_BUFFER
/*
	if( fDisabledMapBorder )
	{
		return;
	}
*/

	if( fShowMapInventoryPool )
	{
		return;
	}

	if( fPlotForHelicopter == TRUE )
	{
		DisplayDistancesForHelicopter( );
		return;
	}

	BltVideoObjectFromIndex(FRAME_BUFFER, guiMapBorderEtaPopUp, 0, MAP_BORDER_X + 215, 291);

	InvalidateRegion( MAP_BORDER_X + 215, 291, MAP_BORDER_X + 215 + 100 , 310);
}


static void MakeButton(UINT idx, UINT gfx, INT16 x, GUI_CALLBACK click, const wchar_t* help)
{
	INT32 img = LoadButtonImage("INTERFACE/map_border_buttons.sti", -1, gfx, -1, gfx + 9, -1);
	giMapBorderButtonsImage[idx] = img;
	INT32 btn = QuickCreateButtonNoMove(img, x, 323, MSYS_PRIORITY_HIGH, click);
	giMapBorderButtons[idx] = btn;
	SetButtonFastHelpText(btn, help);
	SetButtonCursor(btn, MSYS_NO_CURSOR);
}


#if 0
static void MakeButtonScroll(UINT idx, INT32 gray, INT32 normal, INT16 x, INT16 y, GUI_CALLBACK click, const wchar_t* help)
{
	INT32 btn = QuickCreateButtonImg("INTERFACE/map_screen_bottom_arrows.sti", gray, normal, -1, normal + 2, -1, x, y, MSYS_PRIORITY_HIGH, click);
	guiMapBorderScrollButtons[idx] = btn;
	SetButtonFastHelpText(btn, help);
}
#endif


static void BtnAircraftCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnItemCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnMilitiaCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnMineCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnTeamCallback(GUI_BUTTON* btn, INT32 reason);
static void BtnTownCallback(GUI_BUTTON* btn, INT32 reason);
static void InitializeMapBorderButtonStates(void);


BOOLEAN CreateButtonsForMapBorder( void )
{
	// will create the buttons needed for the map screen border region
#if 0
	MakeButtonScroll(ZOOM_MAP_SCROLL_UP,    11, 4, 602, 303, BtnScrollNorthMapScreenCallback, pMapScreenBorderButtonHelpText[6]);
	MakeButtonScroll(ZOOM_MAP_SCROLL_DWN,   12, 5, 602, 338, BtnScrollSouthMapScreenCallback, pMapScreenBorderButtonHelpText[7]);
	MakeButtonScroll(ZOOM_MAP_SCROLL_LEFT,   9, 0, 584, 322, BtnScrollWestMapScreenCallback,  pMapScreenBorderButtonHelpText[9]);
	MakeButtonScroll(ZOOM_MAP_SCROLL_RIGHT, 10, 1, 619, 322, BtnScrollEastMapScreenCallback,  pMapScreenBorderButtonHelpText[8]);
#endif

  MakeButton(MAP_BORDER_TOWN_BTN,     5, 299, BtnTownCallback,     pMapScreenBorderButtonHelpText[0]); // towns
  MakeButton(MAP_BORDER_MINE_BTN,     4, 342, BtnMineCallback,     pMapScreenBorderButtonHelpText[1]); // mines
  MakeButton(MAP_BORDER_TEAMS_BTN,    3, 385, BtnTeamCallback,     pMapScreenBorderButtonHelpText[2]); // people
  MakeButton(MAP_BORDER_MILITIA_BTN,  8, 428, BtnMilitiaCallback,  pMapScreenBorderButtonHelpText[5]); // militia
  MakeButton(MAP_BORDER_AIRSPACE_BTN, 2, 471, BtnAircraftCallback, pMapScreenBorderButtonHelpText[3]); // airspace
  MakeButton(MAP_BORDER_ITEM_BTN,     1, 514, BtnItemCallback,     pMapScreenBorderButtonHelpText[4]); // items

	// raise and lower view level

	// raise
	/*
	guiMapBorderLandRaiseButtonsImage[MAP_BORDER_RAISE_LEVEL] = LoadButtonImage("INTERFACE/map_screen_bottom_arrows.sti", 11, 4, -1, 6, -1);
	guiMapBorderLandRaiseButtons[MAP_BORDER_RAISE_LEVEL] = QuickCreateButtonNoMove(guiMapBorderLandRaiseButtonsImage[MAP_BORDER_RAISE_LEVEL], MAP_BORDER_X + 264, 322, MSYS_PRIORITY_HIGH, BtnRaiseLevelBtnCallback);

	// lower
	guiMapBorderLandRaiseButtonsImage[MAP_BORDER_LOWER_LEVEL] = LoadButtonImage("INTERFACE/map_screen_bottom_arrows.sti", 12, 5, -1, 7, -1);
	guiMapBorderLandRaiseButtons[MAP_BORDER_LOWER_LEVEL] = QuickCreateButtonNoMove(guiMapBorderLandRaiseButtonsImage[MAP_BORDER_LOWER_LEVEL], MAP_BORDER_X + 264, 340, MSYS_PRIORITY_HIGH, BtnLowerLevelBtnCallback);

*/

	//SetButtonFastHelpText( guiMapBorderLandRaiseButtons[ 0 ], pMapScreenBorderButtonHelpText[ 10 ] );
	//SetButtonFastHelpText( guiMapBorderLandRaiseButtons[ 1 ], pMapScreenBorderButtonHelpText[ 11 ] );

//	SetButtonCursor(guiMapBorderLandRaiseButtons[ 0 ], MSYS_NO_CURSOR );
//	SetButtonCursor(guiMapBorderLandRaiseButtons[ 1 ], MSYS_NO_CURSOR );

	InitializeMapBorderButtonStates( );

	return( TRUE );
}


void DeleteMapBorderButtons( void )
{
	UINT8 ubCnt;

/*
	RemoveButton( guiMapBorderScrollButtons[ 0 ]);
	RemoveButton( guiMapBorderScrollButtons[ 1 ]);
	RemoveButton( guiMapBorderScrollButtons[ 2 ]);
	RemoveButton( guiMapBorderScrollButtons[ 3 ]);
*/

	RemoveButton( giMapBorderButtons[ 0 ]);
	RemoveButton( giMapBorderButtons[ 1 ]);
	RemoveButton( giMapBorderButtons[ 2 ]);
	RemoveButton( giMapBorderButtons[ 3 ]);
	RemoveButton( giMapBorderButtons[ 4 ]);
	RemoveButton( giMapBorderButtons[ 5 ]);

	//RemoveButton( guiMapBorderLandRaiseButtons[ 0 ]);
	//RemoveButton( guiMapBorderLandRaiseButtons[ 1 ]);

	// images

	UnloadButtonImage( giMapBorderButtonsImage[ 0 ] );
	UnloadButtonImage( giMapBorderButtonsImage[ 1 ] );
	UnloadButtonImage( giMapBorderButtonsImage[ 2 ] );
	UnloadButtonImage( giMapBorderButtonsImage[ 3 ] );
	UnloadButtonImage( giMapBorderButtonsImage[ 4 ] );
	UnloadButtonImage( giMapBorderButtonsImage[ 5 ] );

	//UnloadButtonImage( guiMapBorderLandRaiseButtonsImage[ 0 ] );
	//UnloadButtonImage( guiMapBorderLandRaiseButtonsImage[ 1 ] );


	for ( ubCnt = 0; ubCnt < 6; ubCnt++ )
	{
		giMapBorderButtons[ ubCnt ] = -1;
		giMapBorderButtonsImage[ ubCnt ] = -1;
	}
}


// callbacks

/*
void BtnLowerLevelBtnCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		// are help messages being displayed?..redraw
		if( ScrollButtonsDisplayingHelpMessage( ) )
		{
			fMapPanelDirty = TRUE;
		}

		MarkButtonsDirty( );
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
		// go down one level
		GoDownOneLevelInMap( );
	}
}


void BtnRaiseLevelBtnCallback(GUI_BUTTON *btn,INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		// are help messages being displayed?..redraw
		if( ScrollButtonsDisplayingHelpMessage( ) )
		{
			fMapPanelDirty = TRUE;
		}


		MarkButtonsDirty( );
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
		// go up one level
		GoUpOneLevelInMap( );
	}
}
*/


static void CommonBtnCallbackBtnDownChecks(void);


static void BtnMilitiaCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
		ToggleShowMilitiaMode( );
	}
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
	}
}


static void BtnTeamCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
		ToggleShowTeamsMode();
	}
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
	}
}


static void BtnTownCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
		ToggleShowTownsMode();
	}
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
	}
}


static void BtnMineCallback(GUI_BUTTON* btn, INT32 reason)
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
		ToggleShowMinesMode();
	}
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
	}
}


static void BtnAircraftCallback(GUI_BUTTON* btn, INT32 reason)
{
  if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
  {
		CommonBtnCallbackBtnDownChecks();

		ToggleAirspaceMode();
  }
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
	}
}


static void BtnItemCallback(GUI_BUTTON* btn, INT32 reason)
{
  if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
  {
		CommonBtnCallbackBtnDownChecks();

		ToggleItemsFilter();
  }
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
	}
}


/*
void BtnZoomCallback(GUI_BUTTON *btn,INT32 reason)
{
	UINT16 sTempXOff=0;
	UINT16 sTempYOff=0;


	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();

		btn->uiFlags ^= BUTTON_CLICKED_ON;
		fZoomFlag = (btn->uiFlags & BUTTON_CLICKED_ON) != 0;
		if (fZoomFlag)
		{
		 if( sSelMapX > 14 )
		 {
			 iZoomX = ( ( sSelMapX + 2 ) / 2 ) * ( MAP_GRID_X * 2 );
		 }
		 else
		 {
			 iZoomX=sSelMapX/2*MAP_GRID_X*2;
		 }

		 if( sOldSelMapY > 14 )
		 {
			 iZoomY = ( ( sSelMapY + 2 ) / 2 ) * ( MAP_GRID_Y * 2 );
		 }
		 else
		 {
			 iZoomY=sSelMapY/2*MAP_GRID_Y*2;
		 }

		}

	 	fMapPanelDirty=TRUE;
	}
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN )
	{
		CommonBtnCallbackBtnDownChecks();
	}
}
*/


static void MapBorderButtonOff(UINT8 ubBorderButtonIndex);
static void MapBorderButtonOn(UINT8 ubBorderButtonIndex);


void ToggleShowTownsMode( void )
{
	if( fShowTownFlag == TRUE )
	{
		fShowTownFlag = FALSE;
		MapBorderButtonOff( MAP_BORDER_TOWN_BTN );
	}
	else
	{
		fShowTownFlag = TRUE;
		MapBorderButtonOn( MAP_BORDER_TOWN_BTN );

		if( fShowMineFlag == TRUE )
		{
			fShowMineFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_MINE_BTN );
		}

		if( fShowAircraftFlag == TRUE )
		{
			fShowAircraftFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );
		}

		if( fShowItemsFlag == TRUE )
		{
			fShowItemsFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_ITEM_BTN );
		}
	}

	fMapPanelDirty = TRUE;
}


void ToggleShowMinesMode( void )
{
	if( fShowMineFlag == TRUE )
	{
		fShowMineFlag = FALSE;
		MapBorderButtonOff( MAP_BORDER_MINE_BTN );
	}
	else
	{
		fShowMineFlag = TRUE;
		MapBorderButtonOn( MAP_BORDER_MINE_BTN );

		if( fShowTownFlag == TRUE )
		{
			fShowTownFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_TOWN_BTN );
		}

		if( fShowAircraftFlag == TRUE )
		{
			fShowAircraftFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );
		}

		if( fShowItemsFlag == TRUE )
		{
			fShowItemsFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_ITEM_BTN );
		}
	}

	fMapPanelDirty = TRUE;
}


static BOOLEAN DoesPlayerHaveAnyMilitia(void);


void ToggleShowMilitiaMode( void )
{
	if( fShowMilitia == TRUE )
	{
		fShowMilitia = FALSE;
		MapBorderButtonOff( MAP_BORDER_MILITIA_BTN );
	}
	else
	{
		// toggle militia ON
		fShowMilitia = TRUE;
		MapBorderButtonOn( MAP_BORDER_MILITIA_BTN );

		// if Team is ON, turn it OFF
		if( fShowTeamFlag == TRUE )
		{
			fShowTeamFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_TEAMS_BTN );
		}

/*
		// if Airspace is ON, turn it OFF
		if( fShowAircraftFlag == TRUE )
		{
			fShowAircraftFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );
		}
*/

		if ( fShowItemsFlag == TRUE )
		{
			fShowItemsFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_ITEM_BTN );
		}


		// check if player has any militia
		if ( DoesPlayerHaveAnyMilitia( ) == FALSE )
		{
			const wchar_t *pwString = NULL;

			// no - so put up a message explaining how it works

			// if he's already training some
			if( IsAnyOneOnPlayersTeamOnThisAssignment( TRAIN_TOWN ) )
			{
				// say they'll show up when training is completed
				pwString = pMapErrorString[ 28 ];
			}
			else
			{
				// say you need to train them first
				pwString = zMarksMapScreenText[ 1 ];
			}

			BeginMapUIMessage(0, pwString);
		}
	}

	fMapPanelDirty = TRUE;
}


void ToggleShowTeamsMode( void )
{
	if( fShowTeamFlag == TRUE )
	{
		// turn show teams OFF
		fShowTeamFlag = FALSE;
		MapBorderButtonOff( MAP_BORDER_TEAMS_BTN );

		// dirty regions
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
	else
	{	// turn show teams ON
		TurnOnShowTeamsMode();
	}
}


void ToggleAirspaceMode( void )
{
	if( fShowAircraftFlag == TRUE )
	{
		// turn airspace OFF
		fShowAircraftFlag = FALSE;
		MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );

		if( fPlotForHelicopter == TRUE )
		{
			AbortMovementPlottingMode( );
		}

		// dirty regions
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
	else
	{	// turn airspace ON
		TurnOnAirSpaceMode();
	}
}


static void TurnOnItemFilterMode(void);


void ToggleItemsFilter( void )
{
	if( fShowItemsFlag == TRUE )
	{
		// turn items OFF
		fShowItemsFlag = FALSE;
		MapBorderButtonOff( MAP_BORDER_ITEM_BTN );

		// dirty regions
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
	else
	{
		// turn items ON
		TurnOnItemFilterMode();
	}
}


/*
void BtnScrollNorthMapScreenCallback( GUI_BUTTON *btn,INT32 reason )
{
	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{

		// not zoomed in?...don't push down
	  if( fZoomFlag == FALSE )
		{
			return;
		}

		// are help messages being displayed?..redraw
		if( ScrollButtonsDisplayingHelpMessage( ) )
		{
			fMapPanelDirty = TRUE;
		}
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
		giScrollButtonState = NORTH_DIR;
		fMapScrollDueToPanelButton = TRUE;
	}
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT )
	{
		giScrollButtonState = NORTH_DIR;
	}
}

void BtnScrollSouthMapScreenCallback( GUI_BUTTON *btn,INT32 reason )
{

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{

		// not zoomed in?...don't push down
	  if( fZoomFlag == FALSE )
		{
			return;
		}

		// are help messages being displayed?..redraw
		 if( ScrollButtonsDisplayingHelpMessage( ) )
		 {
			 fMapPanelDirty = TRUE;
		 }
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
		giScrollButtonState = SOUTH_DIR;
		fMapScrollDueToPanelButton = TRUE;
	}
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT )
	{
		 giScrollButtonState = SOUTH_DIR;
	}
}

void BtnScrollEastMapScreenCallback( GUI_BUTTON *btn,INT32 reason )
{

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
	  // not zoomed in?...don't push down
	  if( fZoomFlag == FALSE )
		{
			return;
		}

		// are help messages being displayed?..redraw
		if( ScrollButtonsDisplayingHelpMessage( ) )
		{
			fMapPanelDirty = TRUE;
		}
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
		giScrollButtonState = EAST_DIR;
		fMapScrollDueToPanelButton = TRUE;
	}
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT )
	{
		 giScrollButtonState = EAST_DIR;
	}
}

void BtnScrollWestMapScreenCallback( GUI_BUTTON *btn,INT32 reason )
{

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{

	  // not zoomed in?...don't push down
	  if( fZoomFlag == FALSE )
		{
			return;
		}

		// are help messages being displayed?..redraw
		if( ScrollButtonsDisplayingHelpMessage( ) )
		{
			fMapPanelDirty = TRUE;
		}
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
  {
		giScrollButtonState = WEST_DIR;
		fMapScrollDueToPanelButton = TRUE;
	}
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT )
	{
		 giScrollButtonState = WEST_DIR;
	}
}


void MapScrollButtonMvtCheck( void  )
{
	// run through each button's mouse region, if mouse cursor there...don't show map white sector highlight
	fCursorIsOnMapScrollButtons = FALSE;

	if( ButtonList[ guiMapBorderScrollButtons[ 0 ] ] ->Area.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		fCursorIsOnMapScrollButtons = TRUE;
	}

	if( ButtonList[ guiMapBorderScrollButtons[ 1 ] ] ->Area.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		fCursorIsOnMapScrollButtons = TRUE;
	}

	if( ButtonList[ guiMapBorderScrollButtons[ 2 ] ] ->Area.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		fCursorIsOnMapScrollButtons = TRUE;
	}

	if( ButtonList[ guiMapBorderScrollButtons[ 3 ] ] ->Area.uiFlags & MSYS_MOUSE_IN_AREA )
	{
		fCursorIsOnMapScrollButtons = TRUE;
	}
}
*/


/*
void HandleMapScrollButtonStates( void )
{
	// will enable/disable map scroll buttons based on zoom mode

	if( fDisabledMapBorder || fShowMapInventoryPool )
	{
		return;
	}

	// if underground, don't want zoom in
	if( iCurrentMapSectorZ )
	{
		if( fZoomFlag == TRUE )
		{
			fZoomFlag = FALSE;
			fMapPanelDirty = TRUE;
		}

		MapBorderButtonOff( MAP_BORDER_ZOOM_BTN );
		DisableButton( giMapBorderButtons[ MAP_BORDER_ZOOM_BTN ]);
	}
	else
	{
		EnableButton( giMapBorderButtons[ MAP_BORDER_ZOOM_BTN ]);
	}

	if( fZoomFlag )
	{
		EnableButton( guiMapBorderScrollButtons[ 0 ]);
	  EnableButton( guiMapBorderScrollButtons[ 1 ]);
	  EnableButton( guiMapBorderScrollButtons[ 2 ]);
	  EnableButton( guiMapBorderScrollButtons[ 3 ]);

		UpdateScrollButtonStatesWhileScrolling(  );

	}
	else
	{

		DisableButton( guiMapBorderScrollButtons[ 0 ]);
	  DisableButton( guiMapBorderScrollButtons[ 1 ]);
	  DisableButton( guiMapBorderScrollButtons[ 2 ]);
	  DisableButton( guiMapBorderScrollButtons[ 3 ]);

	}

	// check mvt too
	MapScrollButtonMvtCheck( );
}
*/


/*
BOOLEAN ScrollButtonsDisplayingHelpMessage( void )
{
	// return if any help messages are being displayed for the scroll buttons

	if( ( ButtonList[ guiMapBorderScrollButtons[ 0 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT )||
	( ButtonList[ guiMapBorderScrollButtons[ 1 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT )||
	( ButtonList[ guiMapBorderScrollButtons[ 2 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT )||
	( ButtonList[ guiMapBorderScrollButtons[ 3 ] ]->Area.uiFlags & MSYS_HAS_BACKRECT ) )
	{
		return( TRUE );
	}

	return( FALSE );
}
*/


static void DisplayCurrentLevelMarker(void)
{
	// display the current level marker on the map border
/*
	if( fDisabledMapBorder )
	{
		return;
	}
*/

	// it's actually a white rectangle, not a green arrow!
	BltVideoObjectFromIndex(guiSAVEBUFFER, guiLEVELMARKER, 0, MAP_LEVEL_MARKER_X + 1, MAP_LEVEL_MARKER_Y + MAP_LEVEL_MARKER_DELTA * iCurrentMapSectorZ);
}


static void LevelMarkerBtnCallback(MOUSE_REGION* pRegion, INT32 iReason);


void CreateMouseRegionsForLevelMarkers( void )
{
	INT16 sCounter = 0;
	CHAR16 sString[ 64 ];

	for( sCounter = 0; sCounter  < 4 ; sCounter++ )
	{
		MSYS_DefineRegion(&LevelMouseRegions[ sCounter ], MAP_LEVEL_MARKER_X, ( INT16 )( MAP_LEVEL_MARKER_Y + ( MAP_LEVEL_MARKER_DELTA * sCounter ) ),  MAP_LEVEL_MARKER_X + MAP_LEVEL_MARKER_WIDTH, ( INT16 )( MAP_LEVEL_MARKER_Y + ( MAP_LEVEL_MARKER_DELTA * ( sCounter + 1 ) ) ), MSYS_PRIORITY_HIGH, MSYS_NO_CURSOR,
			MSYS_NO_CALLBACK, LevelMarkerBtnCallback );

		MSYS_SetRegionUserData(&LevelMouseRegions[sCounter],0,sCounter);

		swprintf( sString, lengthof(sString), L"%ls %d", zMarksMapScreenText[ 0 ], sCounter + 1 );
		SetRegionFastHelpText( &LevelMouseRegions[ sCounter ], sString );
  }


}


void DeleteMouseRegionsForLevelMarkers( void )
{
	INT16 sCounter = 0;


	for( sCounter = 0; sCounter < 4; sCounter++ )
	{
		MSYS_RemoveRegion(  &LevelMouseRegions[ sCounter ] );
	}
}


static void LevelMarkerBtnCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for assignment screen mask region
	INT32 iCounter = 0;

	iCounter = MSYS_GetRegionUserData( pRegion, 0 );

	if( ( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP ) )
	{
		JumpToLevel( iCounter );
	}
}


/*
void DisableMapBorderRegion( void )
{
	// will shutdown map border region

	if( fDisabledMapBorder )
	{
		// checked, failed
		return;
	}

	// get rid of graphics and mouse regions
	DeleteMapBorderGraphics( );


	fDisabledMapBorder = TRUE;
}

void EnableMapBorderRegion( void )
{
	// will re-enable mapborder region

	if( fDisabledMapBorder == FALSE )
	{
		// checked, failed
		return;
	}

	// re load graphics and buttons
	LoadMapBorderGraphics( );

	fDisabledMapBorder = FALSE;

}
*/


void TurnOnShowTeamsMode( void )
{
	// if mode already on, leave, else set and redraw

	if( fShowTeamFlag == FALSE )
	{
		fShowTeamFlag = TRUE;
		MapBorderButtonOn( MAP_BORDER_TEAMS_BTN );

		if( fShowMilitia == TRUE )
		{
			fShowMilitia = FALSE;
			MapBorderButtonOff( MAP_BORDER_MILITIA_BTN );
		}

/*
		if( fShowAircraftFlag == TRUE )
		{
			fShowAircraftFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );
		}
*/

		if( fShowItemsFlag == TRUE )
		{
			fShowItemsFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_ITEM_BTN );
		}

		// dirty regions
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
}



void TurnOnAirSpaceMode( void )
{
	// if mode already on, leave, else set and redraw

	if( fShowAircraftFlag == FALSE )
	{
		fShowAircraftFlag = TRUE;
		MapBorderButtonOn( MAP_BORDER_AIRSPACE_BTN );


		// Turn off towns & mines (mostly because town/mine names overlap SAM site names)
		if( fShowTownFlag == TRUE )
		{
			fShowTownFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_TOWN_BTN );
		}

		if( fShowMineFlag == TRUE )
		{
			fShowMineFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_MINE_BTN );
		}

/*
		// Turn off teams and militia
		if( fShowTeamFlag == TRUE )
		{
			fShowTeamFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_TEAMS_BTN );
		}

		if( fShowMilitia == TRUE )
		{
			fShowMilitia = FALSE;
			MapBorderButtonOff( MAP_BORDER_MILITIA_BTN );
		}
*/

		// Turn off items
		if( fShowItemsFlag == TRUE )
		{
			fShowItemsFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_ITEM_BTN );
		}

		if ( bSelectedDestChar != -1 )
		{
			AbortMovementPlottingMode( );
		}


		// if showing underground
		if ( iCurrentMapSectorZ != 0 )
		{
			// switch to the surface
			JumpToLevel( 0 );
		}

		// dirty regions
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
}


static void TurnOnItemFilterMode(void)
{
	// if mode already on, leave, else set and redraw

	if( fShowItemsFlag == FALSE )
	{
		fShowItemsFlag = TRUE;
		MapBorderButtonOn( MAP_BORDER_ITEM_BTN );


		// Turn off towns, mines, teams, militia & airspace if any are on
		if( fShowTownFlag == TRUE )
		{
			fShowTownFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_TOWN_BTN );
		}

		if( fShowMineFlag == TRUE )
		{
			fShowMineFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_MINE_BTN );
		}

		if( fShowTeamFlag == TRUE )
		{
			fShowTeamFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_TEAMS_BTN );
		}

		if( fShowMilitia == TRUE )
		{
			fShowMilitia = FALSE;
			MapBorderButtonOff( MAP_BORDER_MILITIA_BTN );
		}

		if( fShowAircraftFlag == TRUE )
		{
			fShowAircraftFlag = FALSE;
			MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );
		}

		if( ( bSelectedDestChar != -1 ) || ( fPlotForHelicopter == TRUE ) )
		{
			AbortMovementPlottingMode( );
		}

		// dirty regions
		fMapPanelDirty = TRUE;
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}
}


/*
void UpdateLevelButtonStates( void )
{

	if( iCurrentMapSectorZ == 0 )
	{
		DisableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_RAISE_LEVEL ] );
	}
	else
	{
		EnableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_RAISE_LEVEL ] );
	}

	if( iCurrentMapSectorZ == 3 )
	{
		DisableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_LOWER_LEVEL ] );
	}
	else
	{
		EnableButton( guiMapBorderLandRaiseButtons[ MAP_BORDER_LOWER_LEVEL ] );
	}
}
*/


/*
void UpdateScrollButtonStatesWhileScrolling( void )
{
	// too far west, disable
	if ( iZoomY == NORTH_ZOOM_BOUND )
	{
		ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_UP ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
		DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_UP ] );
	}
	else if(iZoomY == SOUTH_ZOOM_BOUND )
	{
		ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_DWN ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
		DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_DWN ] );
	}

	// too far west, disable
	if ( iZoomX == WEST_ZOOM_BOUND )
	{
		ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_LEFT ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
		DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_LEFT ] );
	}
	else if(iZoomX == EAST_ZOOM_BOUND )
	{
		ButtonList[ guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_RIGHT ] ]->uiFlags&=~(BUTTON_CLICKED_ON);
		DisableButton( guiMapBorderScrollButtons[ ZOOM_MAP_SCROLL_RIGHT ] );
	}

}
*/


// set button states to match map flags
static void InitializeMapBorderButtonStates(void)
{
	if( fShowItemsFlag )
	{
		MapBorderButtonOn( MAP_BORDER_ITEM_BTN );
	}
	else
	{
		MapBorderButtonOff( MAP_BORDER_ITEM_BTN );
	}

	if( fShowTownFlag )
	{
		MapBorderButtonOn( MAP_BORDER_TOWN_BTN );
	}
	else
	{
		MapBorderButtonOff( MAP_BORDER_TOWN_BTN );
	}

	if( fShowMineFlag )
	{
		MapBorderButtonOn( MAP_BORDER_MINE_BTN );
	}
	else
	{
		MapBorderButtonOff( MAP_BORDER_MINE_BTN );
	}

	if( fShowTeamFlag )
	{
		MapBorderButtonOn( MAP_BORDER_TEAMS_BTN );
	}
	else
	{
		MapBorderButtonOff( MAP_BORDER_TEAMS_BTN );
	}

	if( fShowAircraftFlag )
	{
		MapBorderButtonOn( MAP_BORDER_AIRSPACE_BTN );
	}
	else
	{
		MapBorderButtonOff( MAP_BORDER_AIRSPACE_BTN );
	}

	if( fShowMilitia )
	{
		MapBorderButtonOn( MAP_BORDER_MILITIA_BTN );
	}
	else
	{
		MapBorderButtonOff( MAP_BORDER_MILITIA_BTN );
	}
}


static BOOLEAN DoesPlayerHaveAnyMilitia(void)
{
	INT16 sX, sY;

	// run through list of towns that might have militia..if any return TRUE..else return FALSE
	for( sX = 1; sX < MAP_WORLD_X - 1; sX++ )
	{
		for( sY = 1; sY < MAP_WORLD_Y - 1; sY++ )
		{
			if( ( SectorInfo[ SECTOR( sX, sY )].ubNumberOfCivsAtLevel[ GREEN_MILITIA ] +  SectorInfo[ SECTOR( sX, sY )].ubNumberOfCivsAtLevel[ REGULAR_MILITIA ]
					+ SectorInfo[ SECTOR( sX, sY )].ubNumberOfCivsAtLevel[ ELITE_MILITIA ] ) > 0 )
			{
				// found at least one
				return( TRUE );
			}
		}
	}

	// no one found
	return( FALSE );
}


static void CommonBtnCallbackBtnDownChecks(void)
{
	if( IsMapScreenHelpTextUp() )
	{
		// stop mapscreen text
		StopMapScreenHelpText( );
	}

	// any click cancels MAP UI messages, unless we're in confirm map move mode
	if( ( giUIMessageOverlay != -1 ) && !gfInConfirmMapMoveMode )
	{
		CancelMapUIMessage( );
	}
}



void InitMapScreenFlags( void )
{
	fShowTownFlag = TRUE;
	fShowMineFlag = FALSE;

	fShowTeamFlag = TRUE;
	fShowMilitia = FALSE;

	fShowAircraftFlag = FALSE;
	fShowItemsFlag = FALSE;
}


static void MapBorderButtonOff(UINT8 ubBorderButtonIndex)
{
	Assert( ubBorderButtonIndex < 6 );

	if( fShowMapInventoryPool )
	{
		return;
	}

	// if button doesn't exist, return
	if ( giMapBorderButtons[ ubBorderButtonIndex ] == -1 )
	{
		return;
	}

	Assert( giMapBorderButtons[ ubBorderButtonIndex ] < MAX_BUTTONS );

	ButtonList[ giMapBorderButtons[ ubBorderButtonIndex ] ]->uiFlags &= ~(BUTTON_CLICKED_ON);
}


static void MapBorderButtonOn(UINT8 ubBorderButtonIndex)
{
	Assert( ubBorderButtonIndex < 6 );

	if( fShowMapInventoryPool )
	{
		return;
	}

	// if button doesn't exist, return
	if ( giMapBorderButtons[ ubBorderButtonIndex ] == -1 )
	{
		return;
	}

	Assert( giMapBorderButtons[ ubBorderButtonIndex ] < MAX_BUTTONS );

	ButtonList[ giMapBorderButtons[ ubBorderButtonIndex ] ]->uiFlags |= BUTTON_CLICKED_ON;
}
