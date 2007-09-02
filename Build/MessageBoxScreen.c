#include "Font.h"
#include "Local.h"
#include "ScreenIDs.h"
#include "Timer_Control.h"
#include "Sys_Globals.h"
#include "Fade_Screen.h"
#include "SysUtil.h"
#include "MercTextBox.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Cursors.h"
#include "MessageBoxScreen.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Map_Screen_Interface.h"
#include "RenderWorld.h"
#include "Gameloop.h"
#include "English.h"
#include "GameSettings.h"
#include "Interface_Control.h"
#include "Cursor_Control.h"
#include "Laptop.h"
#include "Text.h"
#include "MapScreen.h"
#include "Overhead_Map.h"
#include "TileDef.h"
#include "Button_System.h"
#include "JAScreens.h"
#include "Video.h"

#ifdef JA2BETAVERSION
#	include "Debug.h"
#endif


#define		MSGBOX_DEFAULT_WIDTH							300

#define		MSGBOX_BUTTON_WIDTH								61
#define		MSGBOX_BUTTON_HEIGHT							20
#define		MSGBOX_BUTTON_X_SEP								15

#define		MSGBOX_SMALL_BUTTON_WIDTH					31
#define		MSGBOX_SMALL_BUTTON_X_SEP					8

// old mouse x and y positions
SGPPoint pOldMousePosition;
SGPRect MessageBoxRestrictedCursorRegion;

// if the cursor was locked to a region
BOOLEAN fCursorLockedToArea = FALSE;
BOOLEAN	gfInMsgBox = FALSE;

extern BOOLEAN fInMapMode;
extern BOOLEAN gfOverheadMapDirty;


SGPRect gOldCursorLimitRectangle;


MESSAGE_BOX_STRUCT	gMsgBox;
BOOLEAN							gfNewMessageBox = FALSE;
BOOLEAN							gfStartedFromGameScreen = FALSE;
BOOLEAN							gfStartedFromMapScreen = FALSE;
BOOLEAN							fRestoreBackgroundForMessageBox = FALSE;
BOOLEAN							gfDontOverRideSaveBuffer = TRUE;	//this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save buffer
extern void HandleTacticalUILoseCursorFromOtherScreen( );
extern STR16 pUpdatePanelButtons[];

CHAR16		gzUserDefinedButton1[ 128 ];
CHAR16		gzUserDefinedButton2[ 128 ];


static void ContractMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static UINT16 GetMSgBoxButtonWidth(INT32 iButtonImage);
static void LieMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void MsgBoxClickCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void NOMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void NumberedMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void OKMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);
static void YESMsgBoxCallback(GUI_BUTTON* btn, INT32 reason);


INT32 DoMessageBox(UINT8 ubStyle, const wchar_t* zString, UINT32 uiExitScreen, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, const SGPRect* pCenteringRect)
{
	UINT16	usTextBoxWidth;
	UINT16	usTextBoxHeight;
	SGPRect	aRect;
	INT16	sButtonX, sButtonY, sBlankSpace;
	UINT8	ubMercBoxBackground = BASIC_MERC_POPUP_BACKGROUND, ubMercBoxBorder = BASIC_MERC_POPUP_BORDER;
	UINT8	ubFontColor, ubFontShadowColor;
	UINT16	usCursor;
	INT32 iId = -1;

	GetMousePos( &pOldMousePosition );

	//this variable can be unset if ur in a non gamescreen and DONT want the msg box to use the save buffer
	gfDontOverRideSaveBuffer = TRUE;

	SetCurrentCursorFromDatabase( CURSOR_NORMAL );

	if( gMsgBox.BackRegion.uiFlags & MSYS_REGION_EXISTS )
	{
		return( 0 );
	}

	// Based on style....
	switch( ubStyle )
	{
		//default
		case 	MSG_BOX_BASIC_STYLE:

			ubMercBoxBackground = DIALOG_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder			= DIALOG_MERC_POPUP_BORDER;

			// Add button images
			gMsgBox.iButtonImages			= LoadButtonImage( "INTERFACE/popupbuttons.sti", -1,0,-1,1,-1 );
			ubFontColor	= FONT_MCOLOR_WHITE;
			ubFontShadowColor = DEFAULT_SHADOW;
			usCursor = CURSOR_NORMAL;

			break;

		case MSG_BOX_RED_ON_WHITE:
			ubMercBoxBackground = WHITE_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder			= RED_MERC_POPUP_BORDER;

			// Add button images
			gMsgBox.iButtonImages			= LoadButtonImage( "INTERFACE/msgboxRedButtons.sti", -1,0,-1,1,-1 );

			ubFontColor	= 2;
			ubFontShadowColor = NO_SHADOW;
			usCursor = CURSOR_LAPTOP_SCREEN;
			break;

		case MSG_BOX_BLUE_ON_GREY:
			ubMercBoxBackground = GREY_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder			= BLUE_MERC_POPUP_BORDER;

			// Add button images
			gMsgBox.iButtonImages			= LoadButtonImage( "INTERFACE/msgboxGreyButtons.sti", -1,0,-1,1,-1 );

			ubFontColor	= 2;
			ubFontShadowColor = FONT_MCOLOR_WHITE;
			usCursor = CURSOR_LAPTOP_SCREEN;
			break;
	  case MSG_BOX_IMP_STYLE:
			ubMercBoxBackground = IMP_POPUP_BACKGROUND;
			ubMercBoxBorder			= DIALOG_MERC_POPUP_BORDER;

			// Add button images
			gMsgBox.iButtonImages			= LoadButtonImage( "INTERFACE/msgboxGreyButtons.sti", -1,0,-1,1,-1 );

			ubFontColor	= 2;
			ubFontShadowColor = FONT_MCOLOR_WHITE;
			usCursor = CURSOR_LAPTOP_SCREEN;
			break;
		case MSG_BOX_BASIC_SMALL_BUTTONS:

			ubMercBoxBackground = DIALOG_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder			= DIALOG_MERC_POPUP_BORDER;

			// Add button images
			gMsgBox.iButtonImages			= LoadButtonImage( "INTERFACE/popupbuttons.sti", -1,2,-1,3,-1 );
			ubFontColor	= FONT_MCOLOR_WHITE;
			ubFontShadowColor = DEFAULT_SHADOW;
			usCursor = CURSOR_NORMAL;

			break;

		case MSG_BOX_LAPTOP_DEFAULT:
			ubMercBoxBackground = LAPTOP_POPUP_BACKGROUND;
			ubMercBoxBorder			= LAPTOP_POP_BORDER;

			// Add button images
			gMsgBox.iButtonImages			= LoadButtonImage( "INTERFACE/popupbuttons.sti", -1,0,-1,1,-1 );
			ubFontColor	= FONT_MCOLOR_WHITE;
			ubFontShadowColor = DEFAULT_SHADOW;
			usCursor = CURSOR_LAPTOP_SCREEN;
			break;

		default:
			ubMercBoxBackground = BASIC_MERC_POPUP_BACKGROUND;
			ubMercBoxBorder			= BASIC_MERC_POPUP_BORDER;

			// Add button images
			gMsgBox.iButtonImages			= LoadButtonImage( "INTERFACE/msgboxbuttons.sti", -1,0,-1,1,-1 );
			ubFontColor	= FONT_MCOLOR_WHITE;
			ubFontShadowColor = DEFAULT_SHADOW;
			usCursor = CURSOR_NORMAL;
			break;
	}


	if (pCenteringRect != NULL)
	{
		aRect.iTop = 	pCenteringRect->iTop;
		aRect.iLeft = 	pCenteringRect->iLeft;
		aRect.iBottom = 	pCenteringRect->iBottom;
		aRect.iRight = 	pCenteringRect->iRight;
	}
	else
	{
		// Use default!
		aRect.iLeft   = 0;
		aRect.iTop    = 0;
		aRect.iRight  = SCREEN_WIDTH;
		aRect.iBottom = SCREEN_HEIGHT;
	}

	// Set some values!
	gMsgBox.usFlags				= usFlags;
	gMsgBox.uiExitScreen	= uiExitScreen;
	gMsgBox.ExitCallback  = ReturnCallback;
	gMsgBox.fRenderBox		= TRUE;
	gMsgBox.bHandled			= 0;

	// Init message box
	gMsgBox.iBoxId = PrepareMercPopupBox( iId, ubMercBoxBackground, ubMercBoxBorder, zString, MSGBOX_DEFAULT_WIDTH, 40, 10, 30, &usTextBoxWidth, &usTextBoxHeight );

	if( gMsgBox.iBoxId == -1 )
	{
		#ifdef JA2BETAVERSION
			AssertMsg( 0, "Failed in DoMessageBox().  Probable reason is because the string was too large to fit in max message box size." );
		#endif
		return 0;
	}

	// Save height,width
	gMsgBox.usWidth = usTextBoxWidth;
	gMsgBox.usHeight = usTextBoxHeight;

	// Determine position ( centered in rect )
	gMsgBox.sX = (INT16)( ( ( ( aRect.iRight	- aRect.iLeft ) - usTextBoxWidth ) / 2 ) + aRect.iLeft );
	gMsgBox.sY = (INT16)( ( ( ( aRect.iBottom - aRect.iTop ) - usTextBoxHeight ) / 2 ) + aRect.iTop );

	if ( guiCurrentScreen == GAME_SCREEN )
	{
		gfStartedFromGameScreen = TRUE;
	}

	if ( (fInMapMode == TRUE ) )
	{
		gfStartedFromMapScreen = TRUE;
		fMapPanelDirty = TRUE;
	}


	// Set pending screen
	SetPendingNewScreen( MSG_BOX_SCREEN);

	// Init save buffer
	gMsgBox.uiSaveBuffer = AddVideoSurface(usTextBoxWidth, usTextBoxHeight, PIXEL_DEPTH);
	if (gMsgBox.uiSaveBuffer == NO_VSURFACE) return -1;

  //Save what we have under here...
	const SGPRect r = { gMsgBox.sX, gMsgBox.sY, gMsgBox.sX + usTextBoxWidth, gMsgBox.sY + usTextBoxHeight };
	BltVideoSurface(gMsgBox.uiSaveBuffer, FRAME_BUFFER, 0, 0, &r);

	// Create top-level mouse region
	MSYS_DefineRegion(&gMsgBox.BackRegion, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, MSYS_PRIORITY_HIGHEST, usCursor, MSYS_NO_CALLBACK, MsgBoxClickCallback);

	if( gGameSettings.fOptions[ TOPTION_DONT_MOVE_MOUSE ] == FALSE )
	{
		if( usFlags & MSG_BOX_FLAG_OK )
		{
			SimulateMouseMovement( ( gMsgBox.sX + ( usTextBoxWidth / 2 ) + 27 ), ( gMsgBox.sY + ( usTextBoxHeight - 10 ) ) );
		}
		else
		{
			SimulateMouseMovement( gMsgBox.sX + usTextBoxWidth / 2 , gMsgBox.sY + usTextBoxHeight - 4 );
		}
	}

	// findout if cursor locked, if so, store old params and store, restore when done
	if( IsCursorRestricted() )
	{
		fCursorLockedToArea = TRUE;
		GetRestrictedClipCursor( &MessageBoxRestrictedCursorRegion );
		FreeMouseCursor( );
	}

	// Create four numbered buttons
	if ( usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS )
	{
		// This is exclusive of any other buttons... no ok, no cancel, no nothing

		sBlankSpace = usTextBoxWidth - MSGBOX_SMALL_BUTTON_WIDTH * 4 - MSGBOX_SMALL_BUTTON_X_SEP * 3;
		sButtonX = sBlankSpace / 2;
		sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

		gMsgBox.uiButton[0] = CreateIconAndTextButton( gMsgBox.iButtonImages, L"1", FONT12ARIAL,
														 ubFontColor, ubFontShadowColor,
														 ubFontColor, ubFontShadowColor,
														 TEXT_CJUSTIFIED,
														 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
														 NumberedMsgBoxCallback );
		MSYS_SetBtnUserData(gMsgBox.uiButton[0], 1);
		SetButtonCursor(gMsgBox.uiButton[0], usCursor);

		sButtonX += MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
		gMsgBox.uiButton[1] = CreateIconAndTextButton( gMsgBox.iButtonImages, L"2", FONT12ARIAL,
														 ubFontColor, ubFontShadowColor,
														 ubFontColor, ubFontShadowColor,
														 TEXT_CJUSTIFIED,
														 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
														 NumberedMsgBoxCallback );
		MSYS_SetBtnUserData(gMsgBox.uiButton[1], 2);
		SetButtonCursor(gMsgBox.uiButton[1], usCursor);

		sButtonX += MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
		gMsgBox.uiButton[2] = CreateIconAndTextButton( gMsgBox.iButtonImages, L"3", FONT12ARIAL,
														 ubFontColor, ubFontShadowColor,
														 ubFontColor, ubFontShadowColor,
														 TEXT_CJUSTIFIED,
														 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
														 NumberedMsgBoxCallback );
		MSYS_SetBtnUserData(gMsgBox.uiButton[2], 3);
		SetButtonCursor(gMsgBox.uiButton[2], usCursor);

		sButtonX += MSGBOX_SMALL_BUTTON_WIDTH + MSGBOX_SMALL_BUTTON_X_SEP;
		gMsgBox.uiButton[3] = CreateIconAndTextButton( gMsgBox.iButtonImages, L"4", FONT12ARIAL,
														 ubFontColor, ubFontShadowColor,
														 ubFontColor, ubFontShadowColor,
														 TEXT_CJUSTIFIED,
														 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
														 NumberedMsgBoxCallback );
		MSYS_SetBtnUserData(gMsgBox.uiButton[3], 4);
		SetButtonCursor(gMsgBox.uiButton[3], usCursor);
		ForceButtonUnDirty( gMsgBox.uiButton[3] );
		ForceButtonUnDirty( gMsgBox.uiButton[2] );
		ForceButtonUnDirty( gMsgBox.uiButton[1] );
		ForceButtonUnDirty( gMsgBox.uiButton[0] );

	}
	else
	{

		// Create text button
		if ( usFlags & MSG_BOX_FLAG_OK )
		{


//			sButtonX = ( usTextBoxWidth - MSGBOX_BUTTON_WIDTH ) / 2;
			sButtonX = ( usTextBoxWidth - GetMSgBoxButtonWidth( gMsgBox.iButtonImages ) ) / 2;

			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiOKButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_OK ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 OKMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiOKButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiOKButton );
		}



		// Create text button
		if ( usFlags & MSG_BOX_FLAG_CANCEL )
		{
			sButtonX = ( usTextBoxWidth - GetMSgBoxButtonWidth( gMsgBox.iButtonImages ) ) / 2;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiOKButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_CANCEL ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 OKMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiOKButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiOKButton );

		}

		if ( usFlags & MSG_BOX_FLAG_YESNO )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 2;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_YES ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 YESMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);

			ForceButtonUnDirty( gMsgBox.uiYESButton );

			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_NO ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 NOMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );

		}

		if ( usFlags & MSG_BOX_FLAG_CONTINUESTOP )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 2;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pUpdatePanelButtons[ 0 ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 YESMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);

			ForceButtonUnDirty( gMsgBox.uiYESButton );

			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pUpdatePanelButtons[ 1 ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 NOMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );

		}

		if ( usFlags & MSG_BOX_FLAG_OKCONTRACT )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 2;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_OK ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 OKMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);

			ForceButtonUnDirty( gMsgBox.uiYESButton );

			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_REHIRE ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 ContractMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );

		}

		if ( usFlags & MSG_BOX_FLAG_YESNOCONTRACT )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 3;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_YES ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 YESMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiYESButton );


			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_NO ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 NOMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );

			gMsgBox.uiOKButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_REHIRE ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + 2 * ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 ContractMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiOKButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiOKButton );

		}


		if ( usFlags & MSG_BOX_FLAG_GENERICCONTRACT )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 3;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, gzUserDefinedButton1, FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 YESMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiYESButton );


			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, gzUserDefinedButton2, FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 NOMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );

			gMsgBox.uiOKButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_REHIRE ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + 2 * ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 ContractMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiOKButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiOKButton );

		}

		if ( usFlags & MSG_BOX_FLAG_GENERIC )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 2;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, gzUserDefinedButton1, FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 YESMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiYESButton );


			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, gzUserDefinedButton2, FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 NOMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );
		}

		if ( usFlags & MSG_BOX_FLAG_YESNOLIE )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 3;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_YES ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 YESMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiYESButton );


			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_NO ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 NOMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );

			gMsgBox.uiOKButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_LIE ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + 2 * ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 LieMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiOKButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiOKButton );

		}

		if ( usFlags & MSG_BOX_FLAG_OKSKIP )
		{
			sButtonX = ( usTextBoxWidth - ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ) / 2;
			sButtonY = usTextBoxHeight - MSGBOX_BUTTON_HEIGHT - 10;

			gMsgBox.uiYESButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_OK ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 YESMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiYESButton, usCursor);

			ForceButtonUnDirty( gMsgBox.uiYESButton );

			gMsgBox.uiNOButton = CreateIconAndTextButton( gMsgBox.iButtonImages, pMessageStrings[ MSG_SKIP ], FONT12ARIAL,
															 ubFontColor, ubFontShadowColor,
															 ubFontColor, ubFontShadowColor,
															 TEXT_CJUSTIFIED,
															 (INT16)(gMsgBox.sX + sButtonX + ( MSGBOX_BUTTON_WIDTH + MSGBOX_BUTTON_X_SEP ) ), (INT16)(gMsgBox.sY + sButtonY ), BUTTON_TOGGLE ,MSYS_PRIORITY_HIGHEST,
															 NOMsgBoxCallback );
			SetButtonCursor(gMsgBox.uiNOButton, usCursor);
			ForceButtonUnDirty( gMsgBox.uiNOButton );
		}

	}

	InterruptTime();
	PauseGame();
	LockPauseState( 1 );
	// Pause timers as well....
	PauseTime( TRUE );

  // Save mouse restriction region...
  GetRestrictedClipCursor( &gOldCursorLimitRectangle );
  FreeMouseCursor( );

	gfNewMessageBox = TRUE;

	gfInMsgBox			= TRUE;

	return( iId );
}


static void MsgBoxClickCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
}


static void OKMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_OK;
	}
}


static void YESMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_YES;
	}
}


static void NOMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_NO;
	}
}


static void ContractMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_CONTRACT;
	}
}


static void LieMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSG_BOX_RETURN_LIE;
	}
}


static void NumberedMsgBoxCallback(GUI_BUTTON* btn, INT32 reason)
{
	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		gMsgBox.bHandled = MSYS_GetBtnUserData(btn);
	}
}


static UINT32 ExitMsgBox(INT8 ubExitCode)
{
	SGPPoint pPosition;

	// Delete popup!
	RemoveMercPopupBoxFromIndex( gMsgBox.iBoxId );
	gMsgBox.iBoxId = -1;

	//Delete buttons!
	if ( gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS )
	{
		RemoveButton( gMsgBox.uiButton[0] );
		RemoveButton( gMsgBox.uiButton[1] );
		RemoveButton( gMsgBox.uiButton[2] );
		RemoveButton( gMsgBox.uiButton[3] );
	}
	else
	{
		if ( gMsgBox.usFlags & MSG_BOX_FLAG_OK )
		{
			RemoveButton( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNO )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_OKCONTRACT )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNOCONTRACT )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
			RemoveButton( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_GENERICCONTRACT )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
			RemoveButton( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_GENERIC )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNOLIE )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
			RemoveButton( gMsgBox.uiOKButton );
		}

		if( gMsgBox.usFlags & MSG_BOX_FLAG_CONTINUESTOP )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_OKSKIP )
		{
			RemoveButton( gMsgBox.uiYESButton );
			RemoveButton( gMsgBox.uiNOButton );
		}

	}

	// Delete button images
	UnloadButtonImage( gMsgBox.iButtonImages );

	// Unpause game....
	UnLockPauseState();
	UnPauseGame();
	// UnPause timers as well....
	PauseTime( FALSE );

  // Restore mouse restriction region...
  RestrictMouseCursor( &gOldCursorLimitRectangle );


	gfInMsgBox = FALSE;

	// Call done callback!
	if ( gMsgBox.ExitCallback != NULL )
	{
		(*(gMsgBox.ExitCallback))( ubExitCode );
	}


	//if ur in a non gamescreen and DONT want the msg box to use the save buffer, unset gfDontOverRideSaveBuffer in ur callback
	if( ( ( gMsgBox.uiExitScreen != GAME_SCREEN ) || ( fRestoreBackgroundForMessageBox == TRUE ) ) && gfDontOverRideSaveBuffer )
	{
		// restore what we have under here...
		BltVideoSurface(FRAME_BUFFER, gMsgBox.uiSaveBuffer, gMsgBox.sX, gMsgBox.sY, NULL);
		InvalidateRegion( gMsgBox.sX, gMsgBox.sY, (INT16)( gMsgBox.sX + gMsgBox.usWidth ), (INT16)( gMsgBox.sY + gMsgBox.usHeight ) );
	}

	fRestoreBackgroundForMessageBox = FALSE;
	gfDontOverRideSaveBuffer = TRUE;

	if( fCursorLockedToArea == TRUE )
	{
		GetMousePos( &pPosition );

		if( ( pPosition.iX > MessageBoxRestrictedCursorRegion.iRight ) || ( pPosition.iX > MessageBoxRestrictedCursorRegion.iLeft ) && ( pPosition.iY < MessageBoxRestrictedCursorRegion.iTop ) && ( pPosition.iY > MessageBoxRestrictedCursorRegion.iBottom ) )
		{
			SimulateMouseMovement( pOldMousePosition.iX , pOldMousePosition.iY );
		}

		fCursorLockedToArea = FALSE;
		RestrictMouseCursor( &MessageBoxRestrictedCursorRegion );
	}

	// Remove region
	MSYS_RemoveRegion(&(gMsgBox.BackRegion) );

	// Remove save buffer!
	DeleteVideoSurfaceFromIndex( gMsgBox.uiSaveBuffer );


	switch( gMsgBox.uiExitScreen )
	{
		case GAME_SCREEN:

      if ( InOverheadMap( ) )
      {
        gfOverheadMapDirty = TRUE;
      }
      else
      {
			  SetRenderFlags( RENDER_FLAG_FULL );
      }
			break;
		case MAP_SCREEN:
			fMapPanelDirty = TRUE;
			break;
	}

  if ( gfFadeInitialized )
  {
    SetPendingNewScreen(FADE_SCREEN);
    return( FADE_SCREEN );
  }

	return( gMsgBox.uiExitScreen );
}

UINT32	MessageBoxScreenInit( )
{
	return( TRUE );
}


UINT32	MessageBoxScreenHandle( )
{
	InputAtom  InputEvent;

	if ( gfNewMessageBox )
	{
		// If in game screen....
		if ( ( gfStartedFromGameScreen )||( gfStartedFromMapScreen ) )
		{
			if( gfStartedFromGameScreen )
			{
				HandleTacticalUILoseCursorFromOtherScreen( );
			}
			else
			{
				HandleMAPUILoseCursorFromOtherScreen( );
			}

			gfStartedFromGameScreen = FALSE;
			gfStartedFromMapScreen = FALSE;
		}

		gfNewMessageBox = FALSE;

		return( MSG_BOX_SCREEN );
	}



	UnmarkButtonsDirty( );

	// Render the box!
	if ( gMsgBox.fRenderBox )
	{
		if ( gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS )
		{
			MarkAButtonDirty( gMsgBox.uiButton[0] );
			MarkAButtonDirty( gMsgBox.uiButton[1] );
			MarkAButtonDirty( gMsgBox.uiButton[2] );
			MarkAButtonDirty( gMsgBox.uiButton[3] );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_OK )
		{
			MarkAButtonDirty( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_CANCEL )
		{
			MarkAButtonDirty( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNO )
		{
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_OKCONTRACT )
		{
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNOCONTRACT )
		{
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
			MarkAButtonDirty( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_GENERICCONTRACT )
		{
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
			MarkAButtonDirty( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_GENERIC )
		{
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
		}

		if( gMsgBox.usFlags & MSG_BOX_FLAG_CONTINUESTOP )
		{
			// Exit messagebox
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNOLIE )
		{
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
			MarkAButtonDirty( gMsgBox.uiOKButton );
		}

		if ( gMsgBox.usFlags & MSG_BOX_FLAG_OKSKIP )
		{
			MarkAButtonDirty( gMsgBox.uiYESButton );
			MarkAButtonDirty( gMsgBox.uiNOButton );
		}


		RenderMercPopUpBoxFromIndex( gMsgBox.iBoxId, gMsgBox.sX, gMsgBox.sY,  FRAME_BUFFER );
		//gMsgBox.fRenderBox = FALSE;
		// ATE: Render each frame...
	}

	// Render buttons
	RenderButtons( );

	EndFrameBufferRender( );

	// carter, need key shortcuts for clearing up message boxes
	// Check for esc
	while (DequeueEvent(&InputEvent) == TRUE)
  {
      if( InputEvent.usEvent == KEY_UP )
			{
				if (InputEvent.usParam == SDLK_ESCAPE || InputEvent.usParam == 'n')
				{
          if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNO )
          {
					  // Exit messagebox
					  gMsgBox.bHandled = MSG_BOX_RETURN_NO;
          }
				}

				if (InputEvent.usParam == SDLK_RETURN)
				{
					if ( gMsgBox.usFlags & MSG_BOX_FLAG_YESNO )
					{
						// Exit messagebox
						gMsgBox.bHandled = MSG_BOX_RETURN_YES;
					}
					else if( gMsgBox.usFlags & MSG_BOX_FLAG_OK )
					{
						// Exit messagebox
						gMsgBox.bHandled = MSG_BOX_RETURN_OK;
					}
					else if( gMsgBox.usFlags & MSG_BOX_FLAG_CONTINUESTOP )
					{
						// Exit messagebox
						gMsgBox.bHandled = MSG_BOX_RETURN_OK;
					}
				}
				if( InputEvent.usParam == 'o' )
				{
					if( gMsgBox.usFlags & MSG_BOX_FLAG_OK )
					{
						// Exit messagebox
						gMsgBox.bHandled = MSG_BOX_RETURN_OK;
					}
				}
				if( InputEvent.usParam == 'y' )
				{
					if( gMsgBox.usFlags & MSG_BOX_FLAG_YESNO )
					{
						// Exit messagebox
						gMsgBox.bHandled = MSG_BOX_RETURN_YES;
					}
				}
				if( InputEvent.usParam == '1' )
				{
					if ( gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS )
					{
						// Exit messagebox
						gMsgBox.bHandled = 1;
					}
				}
				if( InputEvent.usParam == '2' )
				{
					if ( gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS )
					{
						// Exit messagebox
						gMsgBox.bHandled = 1;
					}
				}
				if( InputEvent.usParam == '3' )
				{
					if ( gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS )
					{
						// Exit messagebox
						gMsgBox.bHandled = 1;
					}
				}
				if( InputEvent.usParam == '4' )
				{
					if ( gMsgBox.usFlags & MSG_BOX_FLAG_FOUR_NUMBERED_BUTTONS )
					{
						// Exit messagebox
						gMsgBox.bHandled = 1;
					}
				}

			}
	}

  if ( gMsgBox.bHandled )
	{
		SetRenderFlags( RENDER_FLAG_FULL );
		return( ExitMsgBox( gMsgBox.bHandled ) );
	}

	return( MSG_BOX_SCREEN );
}

UINT32	MessageBoxScreenShutdown(  )
{
	return( FALSE );
}


static void DoScreenIndependantMessageBoxWithRect(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, SGPRect* pCenteringRect);


// a basic box that don't care what screen we came from
void DoScreenIndependantMessageBox(const wchar_t *zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback )
{
	SGPRect CenteringRect = {0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y };
	DoScreenIndependantMessageBoxWithRect( zString, usFlags, ReturnCallback, &CenteringRect );
}


// a basic box that don't care what screen we came from
static void DoUpperScreenIndependantMessageBox(wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback)
{
	SGPRect CenteringRect = {0, 0, SCREEN_WIDTH, INV_INTERFACE_START_Y / 2 };
	DoScreenIndependantMessageBoxWithRect( zString, usFlags, ReturnCallback, &CenteringRect );
}

// a basic box that don't care what screen we came from
void DoLowerScreenIndependantMessageBox( wchar_t *zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback )
{
	SGPRect CenteringRect = {0, INV_INTERFACE_START_Y / 2, SCREEN_WIDTH, INV_INTERFACE_START_Y };
	DoScreenIndependantMessageBoxWithRect( zString, usFlags, ReturnCallback, &CenteringRect );
}


static void DoScreenIndependantMessageBoxWithRect(const wchar_t* zString, UINT16 usFlags, MSGBOX_CALLBACK ReturnCallback, SGPRect* pCenteringRect)
{

	/// which screen are we in?

	// Map Screen (excluding AI Viewer)
#ifdef JA2BETAVERSION
	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) && ( guiCurrentScreen != AIVIEWER_SCREEN ) )
#else
	if ( (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
#endif
	{

		// auto resolve is a special case
		if( guiCurrentScreen == AUTORESOLVE_SCREEN )
		{
			DoMessageBox( MSG_BOX_BASIC_STYLE, zString, AUTORESOLVE_SCREEN, usFlags, ReturnCallback, pCenteringRect );
		}
		else
		{
			// set up for mapscreen
			DoMapMessageBoxWithRect( MSG_BOX_BASIC_STYLE, zString, MAP_SCREEN, usFlags, ReturnCallback, pCenteringRect );
		}
	}

	//Laptop
	else if( guiCurrentScreen == LAPTOP_SCREEN )
	{
		// set up for laptop
		DoLapTopSystemMessageBoxWithRect( MSG_BOX_LAPTOP_DEFAULT, zString, LAPTOP_SCREEN, usFlags, ReturnCallback, pCenteringRect );
	}

	//Save Load Screen
	else if( guiCurrentScreen == SAVE_LOAD_SCREEN )
	{
		DoSaveLoadMessageBoxWithRect( MSG_BOX_BASIC_STYLE, zString, SAVE_LOAD_SCREEN, usFlags, ReturnCallback, pCenteringRect );
	}

	//Options Screen
	else if( guiCurrentScreen == OPTIONS_SCREEN )
	{
		DoOptionsMessageBoxWithRect( MSG_BOX_BASIC_STYLE, zString, OPTIONS_SCREEN, usFlags, ReturnCallback, pCenteringRect );
	}

	// Tactical
	else if( guiCurrentScreen == GAME_SCREEN )
	{
		DoMessageBox(  MSG_BOX_BASIC_STYLE, zString,  guiCurrentScreen, usFlags,  ReturnCallback,  pCenteringRect );
	}
}


static UINT16 GetMSgBoxButtonWidth(INT32 iButtonImage)
{
	return( GetWidthOfButtonPic( (UINT16)iButtonImage, ButtonPictures[iButtonImage].OnNormal ) );
}
