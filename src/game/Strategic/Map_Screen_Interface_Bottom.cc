#include "Directories.h"
#include "Font.h"
#include "Interface.h"
#include "Local.h"
#include "Map_Screen_Interface_Bottom.h"
#include "Map_Screen_Interface_Border.h"
#include "MessageBoxScreen.h"
#include "Timer_Control.h"
#include "Types.h"
#include "VObject.h"
#include "VSurface.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Message.h"
#include "MapScreen.h"
#include "StrategicMap.h"
#include "Font_Control.h"
#include "Radar_Screen.h"
#include "Game_Clock.h"
#include "SysUtil.h"
#include "Render_Dirty.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Map.h"
#include "Text.h"
#include "Overhead.h"
#include "PreBattle_Interface.h"
#include "Options_Screen.h"
#include "Cursor_Control.h"
#include "GameLoop.h"
#include "AI.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"
#include "Finances.h"
#include "LaptopSave.h"
#include "Interface_Items.h"
#include "WordWrap.h"
#include "Dialogue_Control.h"
#include "Meanwhile.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface_TownMine_Info.h"
#include "Merc_Contract.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Explosion_Control.h"
#include "Creature_Spreading.h"
#include "Soldier_Macros.h"
#include "GameSettings.h"
#include "SaveLoadScreen.h"
#include "Debug.h"
#include "JAScreens.h"
#include "ScreenIDs.h"
#include "UILayout.h"

#include <string_theory/string>


#define MAP_BOTTOM_X (STD_SCREEN_X + 0)
#define MAP_BOTTOM_Y (STD_SCREEN_Y + 359)

#define MESSAGE_BOX_X (STD_SCREEN_X +  17)
#define MESSAGE_BOX_Y (STD_SCREEN_Y + 377)
#define MESSAGE_BOX_W 301
#define MESSAGE_BOX_H  86

#define MESSAGE_SCROLL_AREA_START_X (STD_SCREEN_X + 330)
#define MESSAGE_SCROLL_AREA_WIDTH    15

#define MESSAGE_SCROLL_AREA_START_Y (STD_SCREEN_Y + 390)
#define MESSAGE_SCROLL_AREA_HEIGHT   59

#define SLIDER_HEIGHT		11
#define SLIDER_WIDTH		11

#define SLIDER_BAR_RANGE			( MESSAGE_SCROLL_AREA_HEIGHT - SLIDER_HEIGHT )



#define MESSAGE_BTN_SCROLL_TIME 100

// delay for paused flash
#define PAUSE_GAME_TIMER 500

#define MAP_BOTTOM_FONT_COLOR ( 32 * 4 - 9 )

// button enums
enum{
	MAP_SCROLL_MESSAGE_UP =0,
	MAP_SCROLL_MESSAGE_DOWN,
};

enum{
	MAP_TIME_COMPRESS_MORE = 0,
	MAP_TIME_COMPRESS_LESS,
};


BOOLEAN fMapScreenBottomDirty = TRUE;

static BOOLEAN fMapBottomDirtied = FALSE;

//Used to flag the transition animation from mapscreen to laptop.
BOOLEAN gfStartMapScreenToLaptopTransition = FALSE;

// leaving map screen
BOOLEAN fLeavingMapScreen = FALSE;

// don't start transition from laptop to tactical stuff
BOOLEAN gfDontStartTransitionFromLaptop = FALSE;

// exiting to laptop?
BOOLEAN fLapTop = FALSE;

static BOOLEAN gfOneFramePauseOnExit = FALSE;

// exit states
static ExitToWhere gbExitingMapScreenToWhere = MAP_EXIT_TO_INVALID;

static UINT8 gubFirstMapscreenMessageIndex = 0;

UINT32 guiCompressionStringBaseTime = 0;

// graphics
static SGPVObject* guiMAPBOTTOMPANEL;
static SGPVObject* guiSliderBar;

// buttons
GUIButtonRef        guiMapBottomExitButtons[3];
static GUIButtonRef guiMapBottomTimeButtons[2];
static GUIButtonRef guiMapMessageScrollButtons[2];

// mouse regions
static MOUSE_REGION gMapMessageScrollBarRegion;
static MOUSE_REGION gMapPauseRegion;

static MOUSE_REGION gTimeCompressionMask[3];


static void BtnLaptopCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnTacticalCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnOptionsFromMapScreenCallback(GUI_BUTTON *btn, INT32 reason);

static void BtnTimeCompressMoreMapScreenCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnTimeCompressLessMapScreenCallback(GUI_BUTTON *btn, INT32 reason);

static void BtnMessageDownMapScreenCallback(GUI_BUTTON *btn, INT32 reason);
static void BtnMessageUpMapScreenCallback(GUI_BUTTON *btn, INT32 reason);


static void LoadMessageSliderBar(void);


void HandleLoadOfMapBottomGraphics( void )
{
	// will load the graphics needed for the mapscreen interface bottom
	// will create buttons for interface bottom
	guiMAPBOTTOMPANEL = AddVideoObjectFromFile(INTERFACEDIR "/map_screen_bottom.sti");

	// load slider bar icon
	LoadMessageSliderBar( );
}


static void CreateButtonsForMapScreenInterfaceBottom(void);
static void CreateCompressModePause(void);
static void CreateMapScreenBottomMessageScrollBarRegion(void);


void LoadMapScreenInterfaceBottom(void)
{
	CreateButtonsForMapScreenInterfaceBottom();
	CreateMapScreenBottomMessageScrollBarRegion( );

	// create pause region
	CreateCompressModePause( );
}


static void DeleteMessageSliderBar(void);


void DeleteMapBottomGraphics( void )
{
	DeleteVideoObject(guiMAPBOTTOMPANEL);
	// delete slider bar icon
	DeleteMessageSliderBar( );
}


static void DeleteMapScreenBottomMessageScrollRegion(void);
static void DestroyButtonsForMapScreenInterfaceBottom();
static void RemoveCompressModePause(void);


void DeleteMapScreenInterfaceBottom( void )
{
	// will delete graphics loaded for the mapscreen interface bottom

	DestroyButtonsForMapScreenInterfaceBottom( );
	DeleteMapScreenBottomMessageScrollRegion( );

	// remove comrpess mode pause
	RemoveCompressModePause( );
}


static void DisplayCompressMode(void);
static void DisplayCurrentBalanceForMapBottom(void);
static void DisplayCurrentBalanceTitleForMapBottom(void);
static void DisplayProjectedDailyMineIncome(void);
static void DisplayScrollBarSlider(void);
static void DrawNameOfLoadedSector();
static void EnableDisableBottomButtonsAndRegions(void);
static void EnableDisableMessageScrollButtonsAndRegions(void);

// will render the map screen bottom interface
void RenderMapScreenInterfaceBottom( void )
{
	// render whole panel
	if (fMapScreenBottomDirty)
	{
		BltVideoObject(guiSAVEBUFFER, guiMAPBOTTOMPANEL, 0, MAP_BOTTOM_X, MAP_BOTTOM_Y);
		SGPSector sMap(sSelMap.x, sSelMap.y, iCurrentMapSectorZ);

		if (GetSectorFlagStatus(sMap, SF_ALREADY_VISITED))
		{
			LoadRadarScreenBitmap(GetMapFileName(sMap, TRUE));
		}
		else
		{
			ClearOutRadarMapImage();
		}

		fInterfacePanelDirty = DIRTYLEVEL2;

		// display title
		DisplayCurrentBalanceTitleForMapBottom( );

		// dirty buttons
		MarkButtonsDirty( );

		// invalidate region
		RestoreExternBackgroundRect(MAP_BOTTOM_X, MAP_BOTTOM_Y, SCREEN_WIDTH - MAP_BOTTOM_X, SCREEN_HEIGHT - MAP_BOTTOM_Y);

		// re render radar map
		RenderRadarScreen( );

		// reset dirty flag
		fMapScreenBottomDirty = FALSE;
		fMapBottomDirtied = TRUE;
	}

	DisplayCompressMode( );

	DisplayCurrentBalanceForMapBottom( );
	DisplayProjectedDailyMineIncome( );

	// draw the name of the loaded sector
	DrawNameOfLoadedSector( );

	// display slider on the scroll bar
	DisplayScrollBarSlider( );

	// display messages that can be scrolled through
	DisplayStringsInMapScreenMessageList( );

	EnableDisableMessageScrollButtonsAndRegions( );

	EnableDisableBottomButtonsAndRegions( );

	fMapBottomDirtied = FALSE;
}


static GUIButtonRef MakeExitButton(INT32 off, INT32 on, INT16 x, INT16 y, GUI_CALLBACK click, const ST::string& help)
{
	GUIButtonRef const btn = QuickCreateButtonImg(INTERFACEDIR "/map_border_buttons.sti", off, on, x, y, MSYS_PRIORITY_HIGHEST - 1, click);
	btn->SetFastHelpText(help);
	btn->SetCursor(MSYS_NO_CURSOR);
	return btn;
}


static GUIButtonRef MakeArrowButton(INT32 grayed, INT32 off, INT32 on, INT16 x, INT16 y, GUI_CALLBACK click, const ST::string& help)
{
	GUIButtonRef const btn = QuickCreateButtonImg(INTERFACEDIR "/map_screen_bottom_arrows.sti", grayed, off, -1, on, -1, x, y, MSYS_PRIORITY_HIGHEST - 2, click);
	btn->SetFastHelpText(help);
	btn->SetCursor(MSYS_NO_CURSOR);
	return btn;
}


static void CreateButtonsForMapScreenInterfaceBottom(void)
{
	guiMapBottomExitButtons[MAP_EXIT_TO_LAPTOP]   = MakeExitButton( 6, 15, STD_SCREEN_X + 456, STD_SCREEN_Y + 410, BtnLaptopCallback,               pMapScreenBottomFastHelp[0]);
	guiMapBottomExitButtons[MAP_EXIT_TO_TACTICAL] = MakeExitButton( 7, 16, STD_SCREEN_X + 496, STD_SCREEN_Y + 410, BtnTacticalCallback,             pMapScreenBottomFastHelp[1]);
	guiMapBottomExitButtons[MAP_EXIT_TO_OPTIONS]  = MakeExitButton(18, 19, STD_SCREEN_X + 458, STD_SCREEN_Y + 372, BtnOptionsFromMapScreenCallback, pMapScreenBottomFastHelp[2]);

	// time compression buttons
	guiMapBottomTimeButtons[MAP_TIME_COMPRESS_MORE] = MakeArrowButton(10, 1, 3, STD_SCREEN_X + 528, STD_SCREEN_Y + 456, BtnTimeCompressMoreMapScreenCallback, pMapScreenBottomFastHelp[3]);
	guiMapBottomTimeButtons[MAP_TIME_COMPRESS_LESS] = MakeArrowButton( 9, 0, 2, STD_SCREEN_X + 466, STD_SCREEN_Y + 456, BtnTimeCompressLessMapScreenCallback, pMapScreenBottomFastHelp[4]);

	// scroll buttons
	guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_UP]   = MakeArrowButton(11, 4, 6, STD_SCREEN_X + 331, STD_SCREEN_Y + 371, BtnMessageUpMapScreenCallback,   pMapScreenBottomFastHelp[5]);
	guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_DOWN] = MakeArrowButton(12, 5, 7, STD_SCREEN_X + 331, STD_SCREEN_Y + 452, BtnMessageDownMapScreenCallback, pMapScreenBottomFastHelp[6]);
}


static void DestroyButtonsForMapScreenInterfaceBottom()
{
	FOR_EACH(GUIButtonRef, i, guiMapBottomExitButtons)    RemoveButton(*i);
	FOR_EACH(GUIButtonRef, i, guiMapBottomTimeButtons)    RemoveButton(*i);
	FOR_EACH(GUIButtonRef, i, guiMapMessageScrollButtons) RemoveButton(*i);
	fMapScreenBottomDirty = TRUE;
}


static void BtnLaptopCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		RequestTriggerExitFromMapscreen(MAP_EXIT_TO_LAPTOP);
	}
}


static void BtnTacticalCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		RequestTriggerExitFromMapscreen(MAP_EXIT_TO_TACTICAL);
	}
}


static void BtnOptionsFromMapScreenCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fMapScreenBottomDirty = TRUE;
		RequestTriggerExitFromMapscreen(MAP_EXIT_TO_OPTIONS);
	}
}


static void DrawNameOfLoadedSector()
{
	SetFontDestBuffer(FRAME_BUFFER);
	SGPFont const font = COMPFONT;
	SetFontAttributes(font, 183);

	ST::string buf = GetSectorIDString(SGPSector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ), TRUE);
	buf = ReduceStringLength(buf, 80, font);

	INT16 x;
	INT16 y;
	FindFontCenterCoordinates(STD_SCREEN_X + 548, STD_SCREEN_Y + 426, 80, 16, buf, font, &x, &y);
	MPrint(x, y, buf);
}


static void CompressModeClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( iReason & ( MSYS_CALLBACK_REASON_RBUTTON_UP | MSYS_CALLBACK_REASON_LBUTTON_UP ) )
	{
		if (CommonTimeCompressionChecks()) return;

		RequestToggleTimeCompression();
	}
}


static void BtnTimeCompressMoreMapScreenCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		if (CommonTimeCompressionChecks()) return;
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fMapScreenBottomDirty = TRUE;
		RequestIncreaseInTimeCompression();
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		CommonTimeCompressionChecks();
	}
}


static void BtnTimeCompressLessMapScreenCallback(GUI_BUTTON *btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		if (CommonTimeCompressionChecks()) return;
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		fMapScreenBottomDirty = TRUE;
		RequestDecreaseInTimeCompression();
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		CommonTimeCompressionChecks();
	}
}


static void BtnMessageDownMapScreenCallback(GUI_BUTTON *btn, INT32 reason)
{
	static UINT32 uiLastRepeatScrollTime = 0;

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		uiLastRepeatScrollTime = 0;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		MapScreenMsgScrollDown(1);
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
		if (GetJA2Clock() - uiLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME)
		{
			MapScreenMsgScrollDown(1);
			uiLastRepeatScrollTime = GetJA2Clock();
		}
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		uiLastRepeatScrollTime = 0;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM);
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_REPEAT)
	{
		if (GetJA2Clock() - uiLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME)
		{
			MapScreenMsgScrollDown(MAX_MESSAGES_ON_MAP_BOTTOM);
			uiLastRepeatScrollTime = GetJA2Clock();
		}
	}
}


static void BtnMessageUpMapScreenCallback(GUI_BUTTON *btn, INT32 reason)
{
	static UINT32 uiLastRepeatScrollTime = 0;

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		uiLastRepeatScrollTime = 0;
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		MapScreenMsgScrollUp(1);
	}
	else if (reason & MSYS_CALLBACK_REASON_LBUTTON_REPEAT)
	{
		if (GetJA2Clock() - uiLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME)
		{
			MapScreenMsgScrollUp(1);
			uiLastRepeatScrollTime = GetJA2Clock();
		}
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		uiLastRepeatScrollTime = 0;
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_UP)
	{
		// redraw region
		if (btn->HasFastHelp()) fMapScreenBottomDirty = TRUE;
		MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);
	}
	else if (reason & MSYS_CALLBACK_REASON_RBUTTON_REPEAT)
	{
		if (GetJA2Clock() - uiLastRepeatScrollTime >= MESSAGE_BTN_SCROLL_TIME)
		{
			MapScreenMsgScrollUp(MAX_MESSAGES_ON_MAP_BOTTOM);
			uiLastRepeatScrollTime = GetJA2Clock();
		}
	}
}


static void EnableDisableMessageScrollButtonsAndRegions(void)
{
	UINT8 ubNumMessages;

	ubNumMessages = GetRangeOfMapScreenMessages();

	// if no scrolling required, or already showing the topmost message
	if( ( ubNumMessages <= MAX_MESSAGES_ON_MAP_BOTTOM ) || ( gubFirstMapscreenMessageIndex == 0 ) )
	{
		DisableButton( guiMapMessageScrollButtons[ MAP_SCROLL_MESSAGE_UP ] );
		guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_UP]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
	else
	{
		EnableButton( guiMapMessageScrollButtons[ MAP_SCROLL_MESSAGE_UP ] );
	}

	// if no scrolling required, or already showing the last message
	if( ( ubNumMessages <= MAX_MESSAGES_ON_MAP_BOTTOM ) ||
			( ( gubFirstMapscreenMessageIndex + MAX_MESSAGES_ON_MAP_BOTTOM ) >= ubNumMessages ) )
	{
		DisableButton( guiMapMessageScrollButtons[ MAP_SCROLL_MESSAGE_DOWN ] );
		guiMapMessageScrollButtons[MAP_SCROLL_MESSAGE_DOWN]->uiFlags &= ~BUTTON_CLICKED_ON;
	}
	else
	{
		EnableButton( guiMapMessageScrollButtons[ MAP_SCROLL_MESSAGE_DOWN ] );
	}

	if( ubNumMessages <= MAX_MESSAGES_ON_MAP_BOTTOM )
	{
		gMapMessageScrollBarRegion.Disable();
	}
	else
	{
		gMapMessageScrollBarRegion.Enable();
	}
}


static void DisplayCompressMode(void)
{
	INT16 sX, sY;
	static UINT8 usColor = FONT_LTGREEN;

	// get compress speed
	ST::string Time;
	if( giTimeCompressMode != NOT_USING_TIME_COMPRESSION )
	{
		Time = sTimeStrings[IsTimeBeingCompressed() ? giTimeCompressMode : 0];
	}

	RestoreExternBackgroundRect( STD_SCREEN_X + 489, STD_SCREEN_Y + 457, 522 - 489, 467 - 454 );
	SetFontDestBuffer(FRAME_BUFFER);

	if( GetJA2Clock() - guiCompressionStringBaseTime >= PAUSE_GAME_TIMER )
	{
		if( usColor == FONT_LTGREEN )
		{
			usColor = FONT_WHITE;
		}
		else
		{
			usColor = FONT_LTGREEN;
		}

		guiCompressionStringBaseTime = GetJA2Clock();
	}

	if (giTimeCompressMode != 0 && !GamePaused())
	{
		usColor = FONT_LTGREEN;
	}

	SetFontAttributes(COMPFONT, usColor);
	FindFontCenterCoordinates(STD_SCREEN_X + 489, STD_SCREEN_Y + 457, 522 - 489, 467 - 454, Time, COMPFONT, &sX, &sY);
	MPrint(sX, sY, Time);
}


static void CreateCompressModePause(void)
{
	MSYS_DefineRegion( &gMapPauseRegion, STD_SCREEN_X + 487, STD_SCREEN_Y + 456, STD_SCREEN_X + 522, STD_SCREEN_Y + 467, MSYS_PRIORITY_HIGH,
							MSYS_NO_CURSOR, MSYS_NO_CALLBACK, CompressModeClickCallback );
	gMapPauseRegion.SetFastHelpText(pMapScreenBottomFastHelp[7]);
}


static void RemoveCompressModePause(void)
{
	MSYS_RemoveRegion( &gMapPauseRegion );
}


static void LoadMessageSliderBar(void)
{
	// this function will load the message slider bar
	guiSliderBar = AddVideoObjectFromFile(INTERFACEDIR "/map_screen_bottom_arrows.sti");
}


static void DeleteMessageSliderBar(void)
{
	// this function will delete message slider bar
	DeleteVideoObject(guiSliderBar);
}


static void MapScreenMessageBoxCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		MapScreenMsgScrollUp(3);
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		MapScreenMsgScrollDown(3);
	}
}


static MOUSE_REGION MapMessageBoxRegion;


static void MapScreenMessageScrollBarCallBack(MOUSE_REGION* pRegion, INT32 iReason);


static void CreateMapScreenBottomMessageScrollBarRegion(void)
{
	const INT8 prio = MSYS_PRIORITY_NORMAL;
	{
		const UINT16 x = MESSAGE_SCROLL_AREA_START_X;
		const UINT16 y = MESSAGE_SCROLL_AREA_START_Y;
		const UINT16 w = MESSAGE_SCROLL_AREA_WIDTH;
		const UINT16 h = MESSAGE_SCROLL_AREA_HEIGHT;
		MSYS_DefineRegion(&gMapMessageScrollBarRegion, x, y, x + w, y + h, prio, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MapScreenMessageScrollBarCallBack);
	}
	{
		const UINT16 x = MESSAGE_BOX_X;
		const UINT16 y = MESSAGE_BOX_Y;
		const UINT16 w = MESSAGE_BOX_W;
		const UINT16 h = MESSAGE_BOX_H;
		MSYS_DefineRegion(&MapMessageBoxRegion, x, y, x + w, y + h, prio, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, MapScreenMessageBoxCallBack);
	}
}


static void DeleteMapScreenBottomMessageScrollRegion(void)
{
	MSYS_RemoveRegion( &gMapMessageScrollBarRegion );
	MSYS_RemoveRegion(&MapMessageBoxRegion);
}


static void MapScreenMessageScrollBarCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	UINT8	ubDesiredSliderOffset;
	UINT8 ubDesiredMessageIndex;
	UINT8 ubNumMessages;

	if ( iReason & ( MSYS_CALLBACK_REASON_LBUTTON_DWN | MSYS_CALLBACK_REASON_LBUTTON_REPEAT ) )
	{
		// how many messages are there?
		ubNumMessages = GetRangeOfMapScreenMessages();

		// region is supposed to be disabled if there aren't enough messages to scroll.  Formulas assume this
		if ( ubNumMessages > MAX_MESSAGES_ON_MAP_BOTTOM )
		{
			const UINT8 ubMouseYOffset = pRegion->RelativeYPos;

			// if clicking in the top 5 pixels of the slider bar
			if ( ubMouseYOffset < ( SLIDER_HEIGHT / 2 ) )
			{
				// scroll all the way to the top
				ubDesiredMessageIndex = 0;
			}
			// if clicking in the bottom 6 pixels of the slider bar
			else if ( ubMouseYOffset >= ( MESSAGE_SCROLL_AREA_HEIGHT - ( SLIDER_HEIGHT / 2 ) ) )
			{
				// scroll all the way to the bottom
				ubDesiredMessageIndex = ubNumMessages - MAX_MESSAGES_ON_MAP_BOTTOM;
			}
			else
			{
				// somewhere in between
				ubDesiredSliderOffset = ubMouseYOffset - ( SLIDER_HEIGHT / 2 );

				Assert( ubDesiredSliderOffset <= SLIDER_BAR_RANGE );

				// calculate what the index should be to place the slider at this offset (round fractions of .5+ up)
				ubDesiredMessageIndex = ( ( ubDesiredSliderOffset * ( ubNumMessages - MAX_MESSAGES_ON_MAP_BOTTOM ) ) + ( SLIDER_BAR_RANGE / 2 ) ) / SLIDER_BAR_RANGE;
			}

			// if it's a change
			if ( ubDesiredMessageIndex != gubFirstMapscreenMessageIndex )
			{
				ChangeCurrentMapscreenMessageIndex( ubDesiredMessageIndex );
			}
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_UP)
	{
		MapScreenMsgScrollUp(3);
	}
	else if (iReason & MSYS_CALLBACK_REASON_WHEEL_DOWN)
	{
		MapScreenMsgScrollDown(3);
	}
}


static void DisplayScrollBarSlider(void)
{
	// will display the scroll bar icon
	UINT8 ubNumMessages;
	UINT8 ubSliderOffset;

	ubNumMessages = GetRangeOfMapScreenMessages();

	// only show the slider if there are more messages than will fit on screen
	if ( ubNumMessages > MAX_MESSAGES_ON_MAP_BOTTOM )
	{
		// calculate where slider should be positioned
		ubSliderOffset = ( SLIDER_BAR_RANGE * gubFirstMapscreenMessageIndex ) / ( ubNumMessages - MAX_MESSAGES_ON_MAP_BOTTOM );

		BltVideoObject(FRAME_BUFFER, guiSliderBar, 8, MESSAGE_SCROLL_AREA_START_X + 2, MESSAGE_SCROLL_AREA_START_Y + ubSliderOffset);
	}
}


static void EnableDisableTimeCompressButtons(void);


static void EnableDisableBottomButtonsAndRegions(void)
{
	// this enables and disables the buttons MAP_EXIT_TO_LAPTOP, MAP_EXIT_TO_TACTICAL, and MAP_EXIT_TO_OPTIONS
	for (ExitToWhere iExitButtonIndex = MAP_EXIT_TO_LAPTOP; iExitButtonIndex <= MAP_EXIT_TO_OPTIONS; ++iExitButtonIndex)
	{
		EnableButton(guiMapBottomExitButtons[iExitButtonIndex], AllowedToExitFromMapscreenTo(iExitButtonIndex));
	}

	// enable/disable time compress buttons and region masks
	EnableDisableTimeCompressButtons( );
	CreateDestroyMouseRegionMasksForTimeCompressionButtons( );


	// Enable/Disable map inventory panel buttons

	// if in merc inventory panel
	if( fShowInventoryFlag )
	{
		// and an item is in the cursor
		EnableButton(giMapInvDoneButton, !fMapInventoryItem && !InKeyRingPopup() && !InItemStackPopup());

		if( fShowDescriptionFlag )
		{
			ForceButtonUnDirty( giMapInvDoneButton );
		}
	}
}


static void EnableDisableTimeCompressButtons(void)
{
	if (!AllowedToTimeCompress())
	{
		DisableButton( guiMapBottomTimeButtons[ MAP_TIME_COMPRESS_MORE ] );
		DisableButton( guiMapBottomTimeButtons[ MAP_TIME_COMPRESS_LESS ] );
	}
	else
	{
		// disable LESS if time compression is at minimum or OFF
		EnableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_LESS], IsTimeCompressionOn() && giTimeCompressMode != TIME_COMPRESS_X0);

		// disable MORE if we're not paused and time compression is at maximum
		// only disable MORE if we're not paused and time compression is at maximum
		EnableButton(guiMapBottomTimeButtons[MAP_TIME_COMPRESS_MORE], !IsTimeCompressionOn() || giTimeCompressMode != TIME_COMPRESS_60MINS);
	}
}


void EnableDisAbleMapScreenOptionsButton( BOOLEAN fEnable )
{
	EnableButton(guiMapBottomExitButtons[MAP_EXIT_TO_OPTIONS], fEnable);
}


BOOLEAN AllowedToTimeCompress( void )
{
	// if already leaving, disallow any other attempts to exit
	if ( fLeavingMapScreen )
	{
		return( FALSE );
	}

	// if already going someplace
	if (gbExitingMapScreenToWhere != MAP_EXIT_TO_INVALID) return FALSE;

	// if we're locked into paused time compression by some event that enforces that
	if ( PauseStateLocked() )
	{
		return( FALSE );
	}

	// meanwhile coming up
	if ( gfMeanwhileTryingToStart )
	{
		return( FALSE );
	}

	// someone has something to say
	if ( !DialogueQueueIsEmpty() )
	{
		return( FALSE );
	}

	// moving / confirming movement
	if( ( bSelectedDestChar != -1 ) || fPlotForHelicopter || gfInConfirmMapMoveMode || fShowMapScreenMovementList )
	{
		return( FALSE );
	}

	if (fShowAssignmentMenu || fShowTrainingMenu || fShowAttributeMenu || fShowSquadMenu || fShowContractMenu)
	{
		return( FALSE );
	}

	if( fShowUpdateBox || fShowTownInfo || ( sSelectedMilitiaTown != 0 ) )
	{
		return( FALSE );
	}

	// renewing contracts
	if ( gfContractRenewalSquenceOn )
	{
		return( FALSE );
	}

	// disabled due to battle?
	if( ( fDisableMapInterfaceDueToBattle ) || ( fDisableDueToBattleRoster ) )
	{
		return( FALSE );
	}

	// if holding an inventory item
	if ( fMapInventoryItem )
	{
		return( FALSE );
	}

	// show the inventory pool?
	if( fShowMapInventoryPool )
	{
		// prevent time compress (items get stolen over time, etc.)
		return( FALSE );
	}

	// no mercs have ever been hired
	if (!gfAtLeastOneMercWasHired) return FALSE;


	// no usable mercs on team!
	if ( !AnyUsableRealMercenariesOnTeam() )
	{
		return( FALSE );
	}

		// must wait till bombs go off
	if ( ActiveTimedBombExists() )
	{
		return( FALSE );
	}

	// hostile sector / in battle
	if( (gTacticalStatus.uiFlags & INCOMBAT ) || ( gTacticalStatus.fEnemyInSector ) )
	{
		return( FALSE );
	}

	if( PlayerGroupIsInACreatureInfestedMine() )
	{
		return FALSE;
	}

	// bloodcat ambush?
	if (gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE && HostileBloodcatsPresent())
	{
		return FALSE;
	}

	return( TRUE );
}


static void DisplayCurrentBalanceTitleForMapBottom(void)
{
	ST::string sString;
	INT16 sFontX, sFontY;

	SetFontDestBuffer(guiSAVEBUFFER);
	SetFontAttributes(COMPFONT, MAP_BOTTOM_FONT_COLOR);

	sString = pMapScreenBottomText;
	FindFontCenterCoordinates(STD_SCREEN_X + 359, STD_SCREEN_Y + 387 - 14,  437 - 359, 10, sString, COMPFONT, &sFontX, &sFontY);
	MPrint(sFontX, sFontY, sString);

	sString = zMarksMapScreenText[2];
	FindFontCenterCoordinates(STD_SCREEN_X + 359, STD_SCREEN_Y + 433 - 14,  437 - 359, 10, sString, COMPFONT, &sFontX, &sFontY);
	MPrint(sFontX, sFontY, sString);

	SetFontDestBuffer(FRAME_BUFFER);
}


static void DisplayCurrentBalanceForMapBottom(void)
{
	// show the current balance for the player on the map panel bottom
	INT16 sFontX, sFontY;

	SetFontDestBuffer(FRAME_BUFFER);
	SetFontAttributes(COMPFONT, 183);
	ST::string sString = SPrintMoney(LaptopSaveInfo.iCurrentBalance);
	FindFontCenterCoordinates(STD_SCREEN_X + 359, STD_SCREEN_Y + 387 + 2,  437 - 359, 10, sString, COMPFONT, &sFontX, &sFontY);
	MPrint(sFontX, sFontY, sString);
}


static void CompressMaskClickCallback(MOUSE_REGION* pRegion, INT32 iReason);


void CreateDestroyMouseRegionMasksForTimeCompressionButtons()
{
	static bool created = false;

	// Disable buttons, if not allowed to compress time.
	bool const disabled = fInMapMode && !AllowedToTimeCompress();
	if (disabled && !created)
	{
		// Mask over compress more, compress less and paus game buttons.
		MSYS_DefineRegion(&gTimeCompressionMask[0], STD_SCREEN_X + 528, STD_SCREEN_Y + 457, 528 + 13, 457 + 14, MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, CompressMaskClickCallback);
		MSYS_DefineRegion(&gTimeCompressionMask[1], STD_SCREEN_X + 466, STD_SCREEN_Y + 457, 466 + 13, 457 + 14, MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, CompressMaskClickCallback);
		MSYS_DefineRegion(&gTimeCompressionMask[2], STD_SCREEN_X + 487, STD_SCREEN_Y + 457, 487 + 35, 457 + 11, MSYS_PRIORITY_HIGHEST - 1, MSYS_NO_CURSOR, MSYS_NO_CALLBACK, CompressMaskClickCallback);
		created = true;
	}
	else if (!disabled && created)
	{
		FOR_EACH(MOUSE_REGION, i, gTimeCompressionMask) MSYS_RemoveRegion(&*i);
		created = false;
	}
}


static void CompressMaskClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	if( iReason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		TellPlayerWhyHeCantCompressTime( );
	}
}


static void DisplayProjectedDailyMineIncome(void)
{
	INT32 iRate = 0;
	static INT32 iOldRate = -1;
	INT16 sFontX, sFontY;

	// grab the rate from the financial system
	iRate = GetProjectedTotalDailyIncome( );

	if( iRate != iOldRate )
	{
		iOldRate = iRate;
		fMapScreenBottomDirty = TRUE;

		// if screen was not dirtied, leave
		if (!fMapBottomDirtied) return;
	}

	SetFontDestBuffer(FRAME_BUFFER);
	SetFontAttributes(COMPFONT, 183);
	ST::string sString = SPrintMoney(iRate);
	FindFontCenterCoordinates(STD_SCREEN_X + 359, STD_SCREEN_Y + 433 + 2,  437 - 359, 10, sString, COMPFONT, &sFontX, &sFontY);
	MPrint(sFontX, sFontY, sString);
}


BOOLEAN CommonTimeCompressionChecks( void )
{
	if (bSelectedDestChar != -1 || fPlotForHelicopter)
	{
		// abort plotting movement
		AbortMovementPlottingMode( );
		return( TRUE );
	}

	return( FALSE );
}


bool AnyUsableRealMercenariesOnTeam()
{
	/* Check whether there is a merc on team, who is not a vehicle, robot, POW or
		* EPC. */
	CFOR_EACH_IN_TEAM(i, OUR_TEAM)
	{
		SOLDIERTYPE const& s = *i;
		if (s.bLife <= 0)                            continue;
		if (IsMechanical(s))                         continue;
		if (s.bAssignment == ASSIGNMENT_POW)         continue;
		if (s.bAssignment == ASSIGNMENT_DEAD)        continue;
		if (s.ubWhatKindOfMercAmI == MERC_TYPE__EPC) continue;
		return true;
	}
	return false;
}



void RequestTriggerExitFromMapscreen(ExitToWhere const bExitToWhere)
{
	Assert( ( bExitToWhere >= MAP_EXIT_TO_LAPTOP ) && ( bExitToWhere <= MAP_EXIT_TO_SAVE ) );

	// if allowed to do so
	if ( AllowedToExitFromMapscreenTo( bExitToWhere ) )
	{
		//if the screen to exit to is the SAVE screen
		if( bExitToWhere == MAP_EXIT_TO_SAVE )
		{
			//if the game CAN NOT be saved
			if( !CanGameBeSaved() )
			{
				//Display a message saying the player cant save now
				DoMapMessageBox( MSG_BOX_BASIC_STYLE, zNewTacticalMessages[ TCTL_MSG__IRON_MAN_CANT_SAVE_NOW ], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL );
				return;
			}
			else if ( gGameOptions.ubGameSaveMode == DIF_DEAD_IS_DEAD )
			{
				//Display DiD message saying the player cant save now
				DoMapMessageBox( MSG_BOX_BASIC_STYLE, zNewTacticalMessages[ TCTL_MSG__DEAD_IS_DEAD_CANT_SAVE_NOW ], MAP_SCREEN, MSG_BOX_FLAG_OK, NULL );
				return;
			}
		}

		// permit it, and get the ball rolling
		gbExitingMapScreenToWhere = bExitToWhere;

		// delay until mapscreen has had a chance to render at least one full frame
		gfOneFramePauseOnExit = TRUE;
	}
}


BOOLEAN AllowedToExitFromMapscreenTo(ExitToWhere const bExitToWhere)
{
	Assert( ( bExitToWhere >= MAP_EXIT_TO_LAPTOP ) && ( bExitToWhere <= MAP_EXIT_TO_SAVE ) );

	// if already leaving, disallow any other attempts to exit
	if ( fLeavingMapScreen )
	{
		return( FALSE );
	}

	// if already going someplace else
	if (gbExitingMapScreenToWhere != MAP_EXIT_TO_INVALID &&
			gbExitingMapScreenToWhere != bExitToWhere)
	{
		return( FALSE );
	}

	// someone has something to say
	if ( !DialogueQueueIsEmpty() )
	{
		return( FALSE );
	}

	// meanwhile coming up
	if ( gfMeanwhileTryingToStart )
	{
		return( FALSE );
	}

	// if we're locked into paused time compression by some event that enforces that
	if ( PauseStateLocked() )
	{
		return( FALSE );
	}

	// if holding an inventory item
	if (fMapInventoryItem) return FALSE;

	if( fShowUpdateBox || fShowTownInfo || ( sSelectedMilitiaTown != 0 ) )
	{
		return( FALSE );
	}

	// renewing contracts
	if( gfContractRenewalSquenceOn )
	{
		return( FALSE );
	}

	// battle about to occur?
	if( ( fDisableDueToBattleRoster ) || ( fDisableMapInterfaceDueToBattle ) )
	{
		return( FALSE );
	}

	// the following tests apply to going tactical screen only
	if ( bExitToWhere == MAP_EXIT_TO_TACTICAL )
	{
		// if in battle or bloodcat ambush, the ONLY sector we can go tactical in is the one that's loaded
		SGPSector sector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ);
		BOOLEAN fBattleGoingOn = gTacticalStatus.uiFlags & INCOMBAT || gTacticalStatus.fEnemyInSector || (gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE && HostileBloodcatsPresent());
		BOOLEAN fCurrentSectorSelected = sector == gWorldSector;
		if (fBattleGoingOn && !fCurrentSectorSelected)
		{
			return( FALSE );
		}

		// must have some mercs there
		if (!CanGoToTacticalInSector(sector))
		{
			return( FALSE );
		}
	}

	//if we are map screen sector inventory
	if( fShowMapInventoryPool )
	{
		//dont allow it
		return( FALSE );
	}

	// OK to go there, passed all the checks
	return( TRUE );
}


void HandleExitsFromMapScreen( void )
{
	// if going somewhere
	if (gbExitingMapScreenToWhere == MAP_EXIT_TO_INVALID) return;

	// delay all exits by one frame...
	if (gfOneFramePauseOnExit)
	{
		gfOneFramePauseOnExit = FALSE;
		return;
	}

	// make sure it's still legal to do this!
	if ( AllowedToExitFromMapscreenTo( gbExitingMapScreenToWhere ) )
	{
		// see where we're trying to go
		switch ( gbExitingMapScreenToWhere )
		{
			case MAP_EXIT_TO_LAPTOP:
				fLapTop = TRUE;
				SetPendingNewScreen(LAPTOP_SCREEN);

				BltVideoSurface(guiEXTRABUFFER, FRAME_BUFFER, 0, 0, NULL);
				gfStartMapScreenToLaptopTransition = TRUE;
				break;

			case MAP_EXIT_TO_TACTICAL:
				SetCurrentWorldSector(SGPSector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ));
				break;

			case MAP_EXIT_TO_OPTIONS:
				guiPreviousOptionScreen = guiCurrentScreen;
				SetPendingNewScreen( OPTIONS_SCREEN );
				break;

			case MAP_EXIT_TO_SAVE:
			case MAP_EXIT_TO_LOAD:
				gfCameDirectlyFromGame = TRUE;
				guiPreviousOptionScreen = guiCurrentScreen;
				SetPendingNewScreen( SAVE_LOAD_SCREEN );
				break;

			default:
				// invalid exit type
				Assert( FALSE );
		}

		// time compression during mapscreen exit doesn't seem to cause any problems, but turn it off as early as we can
		StopTimeCompression();

		// now leaving mapscreen
		fLeavingMapScreen = TRUE;
	}

	// cancel exit, either we're on our way, or we're not allowed to go
	gbExitingMapScreenToWhere = MAP_EXIT_TO_INVALID;
}



void MapScreenMsgScrollDown( UINT8 ubLinesDown )
{
	UINT8 ubNumMessages;

	ubNumMessages = GetRangeOfMapScreenMessages();

	// check if we can go that far, only go as far as we can
	if ( ( gubFirstMapscreenMessageIndex + MAX_MESSAGES_ON_MAP_BOTTOM + ubLinesDown ) > ubNumMessages )
	{
		ubLinesDown = ubNumMessages - gubFirstMapscreenMessageIndex - std::min(int(ubNumMessages), MAX_MESSAGES_ON_MAP_BOTTOM);
	}

	if ( ubLinesDown > 0 )
	{
		ChangeCurrentMapscreenMessageIndex( ( UINT8 ) ( gubFirstMapscreenMessageIndex + ubLinesDown ) );
	}
}


void MapScreenMsgScrollUp( UINT8 ubLinesUp )
{
	// check if we can go that far, only go as far as we can
	if ( gubFirstMapscreenMessageIndex < ubLinesUp )
	{
		ubLinesUp = gubFirstMapscreenMessageIndex;
	}

	if ( ubLinesUp > 0 )
	{
		ChangeCurrentMapscreenMessageIndex( ( UINT8 ) ( gubFirstMapscreenMessageIndex - ubLinesUp ) );
	}
}



void MoveToEndOfMapScreenMessageList( void )
{
	UINT8 ubDesiredMessageIndex;
	UINT8 ubNumMessages;

	ubNumMessages = GetRangeOfMapScreenMessages();

	ubDesiredMessageIndex = ubNumMessages - std::min(int(ubNumMessages), MAX_MESSAGES_ON_MAP_BOTTOM);
	ChangeCurrentMapscreenMessageIndex( ubDesiredMessageIndex );
}



void ChangeCurrentMapscreenMessageIndex( UINT8 ubNewMessageIndex )
{
	Assert(ubNewMessageIndex + MAX_MESSAGES_ON_MAP_BOTTOM <= std::max(MAX_MESSAGES_ON_MAP_BOTTOM, int(GetRangeOfMapScreenMessages())));

	gubFirstMapscreenMessageIndex = ubNewMessageIndex;
	gubCurrentMapMessageString = ( gubStartOfMapScreenMessageList + gubFirstMapscreenMessageIndex ) % 256;

	// refresh screen
	fMapScreenBottomDirty = TRUE;
}
