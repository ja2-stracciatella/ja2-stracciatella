#include "Font.h"
#include "HImage.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "Types.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Input.h"
#include "English.h"
#include "Debug.h"
#include "VObject.h"
#include "VSurface.h"
#include "Video.h"
#include "VObject_Blitters.h"
#include "Line.h"
#include "Tactical_Placement_GUI.h"
#include "Overhead_Map.h"
#include "Interface.h"
#include "Font_Control.h"
#include "Overhead.h"
#include "Render_Dirty.h"
#include "SysUtil.h"
#include "PreBattle_Interface.h"
#include "Soldier_Profile.h"
#include "Map_Edgepoints.h"
#include "Strategic.h"
#include "StrategicMap.h"
#include "GameLoop.h"
#include "Message.h"
#include "Map_Information.h"
#include "Soldier_Add.h"
#include "Cursors.h"
#include "Cursor_Control.h"
#include "MessageBoxScreen.h"
#include "Assignments.h"
#include "Text.h"
#include "WordWrap.h"
#include "Game_Clock.h"
#include "MemMan.h"
#include "JAScreens.h"


typedef struct MERCPLACEMENT
{
	SOLDIERTYPE		*pSoldier;
	SGPVObject*   uiVObjectID;
	MOUSE_REGION	region;
	UINT8					ubStrategicInsertionCode;
	BOOLEAN				fPlaced;
}MERCPLACEMENT;

MERCPLACEMENT *gMercPlacement = NULL;

enum
{
	DONE_BUTTON,
	SPREAD_BUTTON,
	GROUP_BUTTON,
	CLEAR_BUTTON,
	NUM_TP_BUTTONS
};
GUIButtonRef iTPButtons[NUM_TP_BUTTONS];

UINT8	gubDefaultButton = CLEAR_BUTTON;
BOOLEAN gfTacticalPlacementGUIActive = FALSE;
BOOLEAN gfTacticalPlacementFirstTime = FALSE;
BOOLEAN gfEnterTacticalPlacementGUI = FALSE;
BOOLEAN gfKillTacticalGUI = FALSE;
static SGPVObject* giOverheadPanelImage;
static BUTTON_PICS* giOverheadButtonImages[NUM_TP_BUTTONS];
SGPVObject* giMercPanelImage = 0;
INT32 giPlacements = 0;
BOOLEAN gfTacticalPlacementGUIDirty = FALSE;
BOOLEAN gfValidLocationsChanged = FALSE;
SGPRect gTPClipRect = {0,0,0,0};
BOOLEAN gfValidCursor = FALSE;
BOOLEAN gfEveryonePlaced = FALSE;

UINT8	gubSelectedGroupID = 0;
UINT8	gubHilightedGroupID = 0;
UINT8 gubCursorGroupID = 0;
INT8	gbSelectedMercID = -1;
INT8	gbHilightedMercID = -1;
INT8  gbCursorMercID = -1;
SOLDIERTYPE *gpTacticalPlacementSelectedSoldier = NULL;
SOLDIERTYPE *gpTacticalPlacementHilightedSoldier = NULL;

BOOLEAN gfNorth, gfEast, gfSouth, gfWest;


#ifdef JA2BETAVERSION

BOOLEAN gfNorthValid, gfEastValid, gfSouthValid, gfWestValid;
BOOLEAN gfChangedEntrySide = FALSE;


static void FindValidInsertionCode(UINT8* pubStrategicInsertionCode)
{
	if( gMapInformation.sNorthGridNo == -1 &&
			gMapInformation.sEastGridNo == -1 &&
			gMapInformation.sSouthGridNo == -1 &&
			gMapInformation.sWestGridNo == -1 )
	{
		AssertMsg( 0, "Map has no entry points at all.  Can't generate edge points.  LC:1" );
	}
	if( gMapInformation.sNorthGridNo	!= -1 && !gps1stNorthEdgepointArray	||
			gMapInformation.sEastGridNo		!= -1 && !gps1stEastEdgepointArray		||
			gMapInformation.sSouthGridNo	!= -1 && !gps1stSouthEdgepointArray	||
			gMapInformation.sWestGridNo		!= -1 && !gps1stWestEdgepointArray		 )
	{
		InvalidateScreen();
		DrawTextToScreen(L"Map doesn't has entrypoints without corresponding edgepoints. LC:1", 30, 150, 600, FONT10ARIALBOLD, FONT_RED,    FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | MARK_DIRTY);
		DrawTextToScreen(L"GENERATING MAP EDGEPOINTS!  Please wait...",                         30, 160, 600, FONT10ARIALBOLD, FONT_YELLOW, FONT_MCOLOR_BLACK, LEFT_JUSTIFIED | MARK_DIRTY);

		RefreshScreen();
		GenerateMapEdgepoints();
		switch( *pubStrategicInsertionCode )
		{
			case INSERTION_CODE_NORTH:
				if( !gps1stNorthEdgepointArray )
					AssertMsg( 0, "Map Edgepoint generation failed.  KM : 0 -- send map" );
				break;
			case INSERTION_CODE_EAST:
				if( !gps1stEastEdgepointArray )
					AssertMsg( 0, "Map Edgepoint generation failed.  KM : 0 -- send map" );
				break;
			case INSERTION_CODE_SOUTH:
				if( !gps1stSouthEdgepointArray )
					AssertMsg( 0, "Map Edgepoint generation failed.  KM : 0 -- send map" );
				break;
			case INSERTION_CODE_WEST:
				if( !gps1stWestEdgepointArray )
					AssertMsg( 0, "Map Edgepoint generation failed.  KM : 0 -- send map" );
				break;
		}
		return;
	}
	if( gMapInformation.sNorthGridNo != -1 )
	{
		*pubStrategicInsertionCode = INSERTION_CODE_NORTH;
		gfChangedEntrySide = TRUE;
	}
	else if( gMapInformation.sEastGridNo != -1 )
	{
		*pubStrategicInsertionCode = INSERTION_CODE_EAST;
		gfChangedEntrySide = TRUE;
	}
	else if( gMapInformation.sSouthGridNo != -1 )
	{
		*pubStrategicInsertionCode = INSERTION_CODE_SOUTH;
		gfChangedEntrySide = TRUE;
	}
	else if( gMapInformation.sWestGridNo != -1 )
	{
		*pubStrategicInsertionCode = INSERTION_CODE_WEST;
		gfChangedEntrySide = TRUE;
	}
	else
	{
		AssertMsg( 0, "No maps edgepoints at all! KM, LC : 1" );
	}
}


static void CheckForValidMapEdge(UINT8* pubStrategicInsertionCode)
{
	switch( *pubStrategicInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			if( !gps1stNorthEdgepointArray )
				FindValidInsertionCode( pubStrategicInsertionCode );
			break;
		case INSERTION_CODE_EAST:
			if( !gps1stEastEdgepointArray )
				FindValidInsertionCode( pubStrategicInsertionCode );
			break;
		case INSERTION_CODE_SOUTH:
			if( !gps1stSouthEdgepointArray )
				FindValidInsertionCode( pubStrategicInsertionCode );
			break;
		case INSERTION_CODE_WEST:
			if( !gps1stWestEdgepointArray )
				FindValidInsertionCode( pubStrategicInsertionCode );
			break;
	}
}

#endif


static void MakeButton(UINT idx, INT16 y, GUI_CALLBACK click, const wchar_t* text, const wchar_t* help)
{
	GUIButtonRef const btn = QuickCreateButton(giOverheadButtonImages[idx], 11, y, MSYS_PRIORITY_HIGH, click);
	iTPButtons[idx] = btn;
	btn->SpecifyGeneralTextAttributes(text, BLOCKFONT, FONT_BEIGE, 141);
	btn->SetFastHelpText(help);
	btn->SpecifyHilitedTextColors(FONT_WHITE, FONT_NEARBLACK);
}


static void ClearPlacementsCallback(GUI_BUTTON* btn, INT32 reason);
static void DoneOverheadPlacementClickCallback(GUI_BUTTON* btn, INT32 reason);
static void GroupPlacementsCallback(GUI_BUTTON* btn, INT32 reason);
static void MercClickCallback(MOUSE_REGION* reg, INT32 reason);
static void MercMoveCallback(MOUSE_REGION* reg, INT32 reason);
static void PlaceMercs(void);
static void SetCursorMerc(INT8 bPlacementID);
static void SpreadPlacementsCallback(GUI_BUTTON* btn, INT32 reason);


void InitTacticalPlacementGUI()
{
	gfTacticalPlacementGUIActive = TRUE;
	gfTacticalPlacementGUIDirty = TRUE;
	gfValidLocationsChanged = TRUE;
	gfTacticalPlacementFirstTime = TRUE;
	gfNorth = gfEast = gfSouth = gfWest = FALSE;
	#ifdef JA2BETAVERSION
		gfNorthValid = gfEastValid = gfSouthValid = gfWestValid = FALSE;
		gfChangedEntrySide = FALSE;
	#endif

	//Enter overhead map
	GoIntoOverheadMap();

	//Load the images
	giOverheadPanelImage = AddVideoObjectFromFile("Interface/OverheadInterface.sti");
	giMercPanelImage     = AddVideoObjectFromFile("Interface/panels.sti");

	giOverheadButtonImages[ DONE_BUTTON ] = LoadButtonImage( "Interface/OverheadUIButtons.sti", -1, 0, -1, 1, -1 );
	AssertMsg(giOverheadButtonImages[DONE_BUTTON] != NULL, "Failed to load Interface/OverheadUIButtons.sti");
	giOverheadButtonImages[ SPREAD_BUTTON ]		= UseLoadedButtonImage( giOverheadButtonImages[ DONE_BUTTON ], -1, 0, -1, 1, -1 );
	giOverheadButtonImages[ GROUP_BUTTON ]		= UseLoadedButtonImage( giOverheadButtonImages[ DONE_BUTTON ], -1, 0, -1, 1, -1 );
	giOverheadButtonImages[ CLEAR_BUTTON ]		= UseLoadedButtonImage( giOverheadButtonImages[ DONE_BUTTON ], -1, 0, -1, 1, -1 );

	//Create the buttons which provide automatic placements.
	MakeButton(CLEAR_BUTTON,  332, ClearPlacementsCallback,            gpStrategicString[STR_TP_CLEAR],  gpStrategicString[STR_TP_CLEARHELP]);
	MakeButton(SPREAD_BUTTON, 367, SpreadPlacementsCallback,           gpStrategicString[STR_TP_SPREAD], gpStrategicString[STR_TP_SPREADHELP]);
	MakeButton(GROUP_BUTTON,  402, GroupPlacementsCallback,            gpStrategicString[STR_TP_GROUP],  gpStrategicString[STR_TP_GROUPHELP]);
	MakeButton(DONE_BUTTON,   437, DoneOverheadPlacementClickCallback, gpStrategicString[STR_TP_DONE],   gpStrategicString[STR_TP_DONEHELP]);
	iTPButtons[DONE_BUTTON]->AllowDisabledFastHelp();

	//First pass:  Count the number of mercs that are going to be placed by the player.
	//             This determines the size of the array we will allocate.
	giPlacements = 0;
	CFOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (!s->fBetweenSectors &&
				s->sSectorX == gpBattleGroup->ubSectorX &&
				s->sSectorY == gpBattleGroup->ubSectorY	&&
				!(s->uiStatusFlags & SOLDIER_VEHICLE) && // ATE Ignore vehicles
				s->bAssignment != ASSIGNMENT_POW &&
				s->bAssignment != IN_TRANSIT &&
				!s->bSectorZ)
		{
			giPlacements++;
		}
	}
	//Allocate the array based on how many mercs there are.
	gMercPlacement = MALLOCNZ(MERCPLACEMENT, giPlacements);
	//Second pass:  Assign the mercs to their respective slots.
	giPlacements = 0;
	FOR_ALL_IN_TEAM(s, OUR_TEAM)
	{
		if (s->bLife != 0 &&
				!s->fBetweenSectors &&
				s->sSectorX == gpBattleGroup->ubSectorX &&
				s->sSectorY == gpBattleGroup->ubSectorY	&&
				s->bAssignment != ASSIGNMENT_POW &&
				s->bAssignment != IN_TRANSIT &&
				!(s->uiStatusFlags & SOLDIER_VEHICLE) && // ATE Ignore vehicles
				!s->bSectorZ)
		{

			// ATE: If we are in a vehicle - remove ourselves from it!
			//if (s->uiStatusFlags & (SOLDIER_DRIVER | SOLDIER_PASSENGER))
			//{
			//	RemoveSoldierFromVehicle(s, s->bVehicleID);
			//}

			if (s->ubStrategicInsertionCode == INSERTION_CODE_PRIMARY_EDGEINDEX ||
					s->ubStrategicInsertionCode == INSERTION_CODE_SECONDARY_EDGEINDEX)
			{
				s->ubStrategicInsertionCode = (UINT8)s->usStrategicInsertionData;
			}
			gMercPlacement[giPlacements].pSoldier = s;
			gMercPlacement[giPlacements].ubStrategicInsertionCode = s->ubStrategicInsertionCode;
			gMercPlacement[ giPlacements ].fPlaced = FALSE;
			#ifdef JA2BETAVERSION
				CheckForValidMapEdge(&s->ubStrategicInsertionCode);
			#endif
			switch (s->ubStrategicInsertionCode)
			{
				case INSERTION_CODE_NORTH:
					gfNorth = TRUE;
					break;
				case INSERTION_CODE_EAST:
					gfEast	= TRUE;
					break;
				case INSERTION_CODE_SOUTH:
					gfSouth = TRUE;
					break;
				case INSERTION_CODE_WEST:
					gfWest	= TRUE;
					break;
			}
			giPlacements++;
		}
	}
	//add all the faces now
	for (INT32 i = 0; i < giPlacements; ++i)
	{
		//Load the faces
		SGPFILENAME ImageFile;
		sprintf(ImageFile, "Faces/65Face/%02d.sti", gMercProfiles[gMercPlacement[i].pSoldier->ubProfile].ubFaceIndex);
		try
		{
			gMercPlacement[i].uiVObjectID = AddVideoObjectFromFile(ImageFile);
		}
		catch (...)
		{
			gMercPlacement[i].uiVObjectID = AddVideoObjectFromFile("Faces/65Face/speck.sti");
		}
		const INT32 xp = 91 + i / 2 * 54;
		const INT32 yp = (i % 2 ? 412 : 361);
		MSYS_DefineRegion( &gMercPlacement[ i ].region, (UINT16)xp, (UINT16)yp, (UINT16)(xp + 54), (UINT16)(yp + 62), MSYS_PRIORITY_HIGH, 0, MercMoveCallback, MercClickCallback );
	}

	PlaceMercs();

	if( gubDefaultButton == GROUP_BUTTON )
	{
		iTPButtons[GROUP_BUTTON]->uiFlags |= BUTTON_CLICKED_ON;
		for (INT32 i = 0; i < giPlacements; ++i)
		{ //go from the currently selected soldier to the end
			if( !gMercPlacement[ i ].fPlaced )
			{ //Found an unplaced merc.  Select him.
				gbSelectedMercID = (INT8)i;
				if( gubDefaultButton == GROUP_BUTTON )
					gubSelectedGroupID = gMercPlacement[ i ].pSoldier->ubGroupID;
				gfTacticalPlacementGUIDirty = TRUE;
				SetCursorMerc( (INT8)i );
				gpTacticalPlacementSelectedSoldier = gMercPlacement[ i ].pSoldier;
				break;
			}
		}
	}
}


static void RenderTacticalPlacementGUI(void)
{
	INT32 i, xp, yp, width;
	INT32 iStartY;
	SOLDIERTYPE *pSoldier;
	UINT16 usHatchColor;
	wchar_t str[ 128 ];
	UINT8 ubColor;
	if( gfTacticalPlacementFirstTime )
	{
		gfTacticalPlacementFirstTime = FALSE;
		DisableScrollMessages();
	}
	//Check to make sure that if we have a hilighted merc (not selected) and the mouse has moved out
	//of it's region, then we will clear the hilighted ID, and refresh the display.
	if( !gfTacticalPlacementGUIDirty && gbHilightedMercID != -1 )
	{
		xp = 91 + (gbHilightedMercID / 2) * 54;
		yp = (gbHilightedMercID % 2) ? 412 : 361;
		if( gusMouseXPos < xp || gusMouseXPos > xp + 54 || gusMouseYPos < yp || gusMouseYPos > yp + 62 )
		{
			gbHilightedMercID = -1;
			gubHilightedGroupID = 0;
			SetCursorMerc( gbSelectedMercID );
			gpTacticalPlacementHilightedSoldier = NULL;
		}
	}
	//If the display is dirty render the entire panel.
	if( gfTacticalPlacementGUIDirty )
	{
		BltVideoObject(FRAME_BUFFER, giOverheadPanelImage, 0, 0, 320);
		InvalidateRegion(0, 0, 320, SCREEN_HEIGHT);
		gfTacticalPlacementGUIDirty = FALSE;
		MarkButtonsDirty();
		for( i = 0; i < giPlacements; i++ )
		{ //Render the mercs
			pSoldier = gMercPlacement[ i ].pSoldier;
			xp = 95 + (i / 2) * 54;
			yp = (i % 2) ? 422 : 371;
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+36, yp+2, xp+44,	yp+30, 0 );
			BltVideoObject(FRAME_BUFFER, giMercPanelImage,              0, xp,     yp);
			BltVideoObject(FRAME_BUFFER, gMercPlacement[i].uiVObjectID, 0, xp + 2, yp + 2);
			//HEALTH BAR
			if( !pSoldier->bLife )
				continue;
			//yellow one for bleeding
			iStartY = yp + 29 - 27*pSoldier->bLifeMax/100;
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+36, iStartY, xp+37, yp+29, Get16BPPColor( FROMRGB( 107, 107, 57 ) ) );
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+37, iStartY, xp+38, yp+29, Get16BPPColor( FROMRGB( 222, 181, 115 ) ) );
			//pink one for bandaged.
			iStartY += 27*pSoldier->bBleeding/100;
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+36, iStartY, xp+37, yp+29, Get16BPPColor( FROMRGB( 156, 57, 57 ) ) );
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+37, iStartY, xp+38, yp+29, Get16BPPColor( FROMRGB( 222, 132, 132 ) ) );
			//red one for actual health
			iStartY = yp + 29 - 27*pSoldier->bLife/100;
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+36, iStartY, xp+37, yp+29, Get16BPPColor( FROMRGB( 107, 8, 8 ) ) );
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+37, iStartY, xp+38, yp+29, Get16BPPColor( FROMRGB( 206, 0, 0 ) ) );
			//BREATH BAR
			iStartY = yp + 29 - 27*pSoldier->bBreathMax/100;
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+39, iStartY, xp+40, yp+29, Get16BPPColor( FROMRGB( 8, 8, 132 ) ) );
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+40, iStartY, xp+41, yp+29, Get16BPPColor( FROMRGB( 8, 8, 107 ) ) );
			//MORALE BAR
			iStartY = yp + 29 - 27*pSoldier->bMorale/100;
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+42, iStartY, xp+43, yp+29, Get16BPPColor( FROMRGB( 8, 156, 8 ) ) );
			ColorFillVideoSurfaceArea( FRAME_BUFFER, xp+43, iStartY, xp+44, yp+29, Get16BPPColor( FROMRGB( 8, 107, 8 ) ) );
		}
		SetFontAttributes(BLOCKFONT, FONT_BEIGE);
		GetSectorIDString( gubPBSectorX, gubPBSectorY, gubPBSectorZ, str, lengthof(str), TRUE );
		mprintf( 120, 335, L"%ls %ls -- %ls...", gpStrategicString[ STR_TP_SECTOR ], str, gpStrategicString[ STR_TP_CHOOSEENTRYPOSITIONS ] );

		//Shade out the part of the tactical map that isn't considered placable.
		BlitBufferToBuffer(FRAME_BUFFER, guiSAVEBUFFER, 0, 320, SCREEN_WIDTH, 160);
	}
	if( gfValidLocationsChanged )
	{
		if( DayTime() )
		{ //6AM to 9PM is black
			usHatchColor = 0; //Black
		}
		else
		{ //9PM to 6AM is gray (black is too dark to distinguish)
			usHatchColor = Get16BPPColor( FROMRGB( 63, 31, 31 ) );
		}
		gfValidLocationsChanged--;
		BlitBufferToBuffer( guiSAVEBUFFER, FRAME_BUFFER, 4, 4, 636, 320 );
		InvalidateRegion( 4, 4, 636, 320 );
		if( gbCursorMercID == -1 )
		{
			gTPClipRect.iLeft		= gfWest	? 30	: 4;
			gTPClipRect.iTop			= gfNorth ? 30	: 4;
			gTPClipRect.iRight		= gfEast	? 610 : 636;
			gTPClipRect.iBottom	= gfSouth ? 290 : 320;
		}
		else
		{
			gTPClipRect.iLeft		= 4;
			gTPClipRect.iTop			= 4;
			gTPClipRect.iRight		= 636;
			gTPClipRect.iBottom	= 320;
			switch( gMercPlacement[ gbCursorMercID ].ubStrategicInsertionCode )
			{
				case INSERTION_CODE_NORTH:	gTPClipRect.iTop			= 30;			break;
				case INSERTION_CODE_EAST:		gTPClipRect.iRight		= 610;		break;
				case INSERTION_CODE_SOUTH:	gTPClipRect.iBottom	= 290;		break;
				case INSERTION_CODE_WEST:		gTPClipRect.iLeft		= 30;			break;
			}
		}
		SGPVSurface::Lock l(FRAME_BUFFER);
		UINT8* const pDestBuf         = l.Buffer<UINT8>();
		UINT32 const uiDestPitchBYTES = l.Pitch();
		Blt16BPPBufferLooseHatchRectWithColor( (UINT16*)pDestBuf, uiDestPitchBYTES, &gTPClipRect, usHatchColor );
		SetClippingRegionAndImageWidth(uiDestPitchBYTES, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		RectangleDraw( TRUE, gTPClipRect.iLeft, gTPClipRect.iTop, gTPClipRect.iRight, gTPClipRect.iBottom, usHatchColor, pDestBuf );
	}
	for( i = 0; i < giPlacements; i++ )
	{ //Render the merc's names
		pSoldier = gMercPlacement[ i ].pSoldier;
		xp = 95 + (i / 2) * 54;
		yp = (i % 2) ? 422 : 371;
		//NAME
		if( gubDefaultButton == GROUP_BUTTON && gMercPlacement[ i ].pSoldier->ubGroupID == gubSelectedGroupID ||
			  gubDefaultButton != GROUP_BUTTON && i == gbSelectedMercID )
		{
			ubColor = FONT_YELLOW;
		}
		else if( gubDefaultButton == GROUP_BUTTON && gMercPlacement[ i ].pSoldier->ubGroupID == gubHilightedGroupID ||
						 gubDefaultButton != GROUP_BUTTON && i == gbHilightedMercID )
		{
			ubColor = FONT_WHITE;
		}
		else
		{
			ubColor = FONT_GRAY3;
		}
		SetFontAttributes(FONT10ARIALBOLD, ubColor);
		//Render the question mark over the face if the merc hasn't yet been placed.
		if( gMercPlacement[ i ].fPlaced )
		{
			RegisterBackgroundRect(BGND_FLAG_SINGLE, xp + 16, yp + 14, xp + 24, yp + 22);
		}
		else
		{
			MPrint(xp + 16, yp + 14, L"?");
			InvalidateRegion( xp + 16, yp + 14, xp + 24, yp + 22 );
		}
		SetFont( BLOCKFONT );
		width = StringPixLength( pSoldier->name, BLOCKFONT );
		xp = xp + ( 48 - width ) / 2;
		yp = yp + 33;
		MPrint(xp, yp, pSoldier->name);
		InvalidateRegion( xp, yp, xp + width, yp + width );
	}

}


static void EnsureDoneButtonStatus(void)
{
	INT32 i;
	//static BOOLEAN fInside = FALSE;
	//BOOLEAN fChanged = FALSE;
	for( i = 0; i < giPlacements; i++ )
	{
		if( !gMercPlacement[ i ].fPlaced )
		{
			if (iTPButtons[DONE_BUTTON]->uiFlags & BUTTON_ENABLED)
			{
				DisableButton( iTPButtons[ DONE_BUTTON ] );
				iTPButtons[DONE_BUTTON]->SetFastHelpText(gpStrategicString[STR_TP_DISABLED_DONEHELP]);
			}
			return;
		}
	}
	if (!(iTPButtons[DONE_BUTTON]->uiFlags & BUTTON_ENABLED))
	{ //only enable it when it is disabled, otherwise the button will stay down!
		EnableButton( iTPButtons[ DONE_BUTTON ] );
		iTPButtons[DONE_BUTTON]->SetFastHelpText(gpStrategicString[STR_TP_DONEHELP]);
	}
}


static void KillTacticalPlacementGUI(void);


void TacticalPlacementHandle()
{
  InputAtom InputEvent;

	EnsureDoneButtonStatus();

	RenderTacticalPlacementGUI();

	if( gfRightButtonState )
	{
		gbSelectedMercID = -1;
		gubSelectedGroupID = 0;
		gpTacticalPlacementSelectedSoldier = NULL;
	}

  while( DequeueEvent( &InputEvent ) )
  {
		if( InputEvent.usEvent == KEY_DOWN )
		{
			switch( InputEvent.usParam )
			{
#ifdef JA2TESTVERSION
				case SDLK_ESCAPE: KillTacticalPlacementGUI(); break;
#endif

				case SDLK_RETURN:
					if (iTPButtons[DONE_BUTTON]->uiFlags & BUTTON_ENABLED)
					{
						KillTacticalPlacementGUI();
					}
					break;
				case 'c':
					ClearPlacementsCallback(iTPButtons[CLEAR_BUTTON], MSYS_CALLBACK_REASON_LBUTTON_UP);
					break;
				case 'g':
					GroupPlacementsCallback(iTPButtons[GROUP_BUTTON], MSYS_CALLBACK_REASON_LBUTTON_UP);
					break;
				case 's':
					SpreadPlacementsCallback(iTPButtons[SPREAD_BUTTON], MSYS_CALLBACK_REASON_LBUTTON_UP);
					break;
				case 'x':
					if( InputEvent.usKeyState & ALT_DOWN )
					{
						HandleShortCutExitState();
					}
					break;
			}
		}
	}
	gfValidCursor = FALSE;
	if( gbSelectedMercID != -1 && gusMouseYPos < 320 )
	{
		switch( gMercPlacement[ gbCursorMercID ].ubStrategicInsertionCode )
		{
			case INSERTION_CODE_NORTH:
				if( gusMouseYPos <= 40 )
					gfValidCursor = TRUE;
				break;
			case INSERTION_CODE_EAST:
				if( gusMouseXPos >= 600 )
					gfValidCursor = TRUE;
				break;
			case INSERTION_CODE_SOUTH:
				if( gusMouseYPos >= 280 )
					gfValidCursor = TRUE;
				break;
			case INSERTION_CODE_WEST:
				if( gusMouseXPos <= 40 )
					gfValidCursor = TRUE;
				break;
		}
		if( gubDefaultButton == GROUP_BUTTON )
		{
			if( gfValidCursor )
			{
				SetCurrentCursorFromDatabase( CURSOR_PLACEGROUP );
			}
			else
			{
				SetCurrentCursorFromDatabase( CURSOR_DPLACEGROUP );
			}
		}
		else
		{
			if( gfValidCursor )
			{
				SetCurrentCursorFromDatabase( CURSOR_PLACEMERC );
			}
			else
			{
				SetCurrentCursorFromDatabase( CURSOR_DPLACEMERC );
			}
		}
	}
	else
	{
		SetCurrentCursorFromDatabase( CURSOR_NORMAL );
	}
	if( gfKillTacticalGUI == 1 )
	{
		KillTacticalPlacementGUI();
	}
	else if( gfKillTacticalGUI == 2 )
	{
		gfKillTacticalGUI = 1;
	}
}


static void PickUpMercPiece(INT32 iPlacement);


static void KillTacticalPlacementGUI(void)
{
	INT32 i;

	gbHilightedMercID = -1;
	gbSelectedMercID = -1;
	gubSelectedGroupID = 0;
	gubHilightedGroupID = 0;
	gbCursorMercID = -1;
	gpTacticalPlacementHilightedSoldier = NULL;
	gpTacticalPlacementSelectedSoldier = NULL;

	//Destroy the tactical placement gui.
	gfEnterTacticalPlacementGUI = FALSE;
	gfTacticalPlacementGUIActive = FALSE;
	gfKillTacticalGUI = FALSE;
	//Delete video objects
	DeleteVideoObject(giOverheadPanelImage);
	DeleteVideoObject(giMercPanelImage);
	//Delete buttons
	for( i = 0; i < NUM_TP_BUTTONS; i++ )
	{
		UnloadButtonImage( giOverheadButtonImages[ i ] );
		RemoveButton( iTPButtons[ i ] );
	}
	//Delete faces and regions
	for( i = 0; i < giPlacements; i++ )
	{
		DeleteVideoObject(gMercPlacement[i].uiVObjectID);
		MSYS_RemoveRegion( &gMercPlacement[ i ].region );
	}

	if( gsCurInterfacePanel < 0 || gsCurInterfacePanel >= NUM_UI_PANELS )
		gsCurInterfacePanel = TEAM_PANEL;

	SetCurrentInterfacePanel(gsCurInterfacePanel);

	//Leave the overhead map.
	KillOverheadMap();
	//Recreate the tactical panel.
	gRadarRegion.Enable();
	SetCurrentInterfacePanel( TEAM_PANEL );
	//Initialize the rest of the map (AI, enemies, civs, etc.)

	for( i = 0; i < giPlacements; i++ )
	{
		PickUpMercPiece( i );
	}

	PrepareLoadedSector();
	EnableScrollMessages();

	#ifdef JA2BETAVERSION
	if( gfChangedEntrySide )
	{
		ScreenMsg( FONT_RED, MSG_ERROR, L"Substituted different entry side due to invalid entry points or map edgepoints.  KM, LC : 1" );
	}
	#endif
}


static void PutDownMercPiece(INT32 iPlacement);


static void ChooseRandomEdgepoints(void)
{
	INT32 i;
	for( i = 0; i < giPlacements; i++ )
	{
		if ( !( gMercPlacement[ i ].pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) )
		{
			gMercPlacement[ i ].pSoldier->usStrategicInsertionData = ChooseMapEdgepoint( gMercPlacement[ i ].ubStrategicInsertionCode );
			if( gMercPlacement[ i ].pSoldier->usStrategicInsertionData != NOWHERE )
			{
				gMercPlacement[ i ].pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			}
			else
			{
				#if 0 /* XXX unsigned < 0 ? */
				if( gMercPlacement[ i ].pSoldier->usStrategicInsertionData < 0 || gMercPlacement[ i ].pSoldier->usStrategicInsertionData > WORLD_MAX )
				#else
				if (gMercPlacement[ i ].pSoldier->usStrategicInsertionData > WORLD_MAX)
				#endif
				{
					i = i;
				}
				gMercPlacement[ i ].pSoldier->ubStrategicInsertionCode = gMercPlacement[ i ].ubStrategicInsertionCode;
			}
		}

		PutDownMercPiece( i );
	}
	gfEveryonePlaced = TRUE;
}


static void PlaceMercs(void)
{
	INT32 i;
	switch( gubDefaultButton )
	{
		case SPREAD_BUTTON: //Place mercs randomly along their side using map edgepoints.
			ChooseRandomEdgepoints();
			break;
		case CLEAR_BUTTON:
			for( i = 0; i < giPlacements; i++ )
			{
				PickUpMercPiece( i );
			}
			gubSelectedGroupID = 0;
			gbSelectedMercID = 0;
			SetCursorMerc( 0 );
			gfEveryonePlaced = FALSE;
			break;
		default:
			return;
	}
	gfTacticalPlacementGUIDirty = TRUE;
}


static void DoneOverheadPlacementClickCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gfKillTacticalGUI = 2;
	}
}


static void SpreadPlacementsCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		gubDefaultButton = SPREAD_BUTTON;
		iTPButtons[GROUP_BUTTON]->uiFlags &= ~BUTTON_CLICKED_ON;
		iTPButtons[GROUP_BUTTON]->uiFlags |= BUTTON_DIRTY;
		PlaceMercs();
		gubSelectedGroupID = 0;
		gbSelectedMercID = -1;
		SetCursorMerc( -1 );
	}
}


static void GroupPlacementsCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		if( gubDefaultButton == GROUP_BUTTON )
		{
			btn->uiFlags &= ~BUTTON_CLICKED_ON;
			btn->uiFlags |= BUTTON_DIRTY;
			gubDefaultButton = CLEAR_BUTTON;
			gubSelectedGroupID = 0;
		}
		else
		{
			btn->uiFlags |= BUTTON_CLICKED_ON | BUTTON_DIRTY;
			gubDefaultButton = GROUP_BUTTON;
			gbSelectedMercID = 0;
			SetCursorMerc( gbSelectedMercID );
			gubSelectedGroupID = gMercPlacement[ gbSelectedMercID ].pSoldier->ubGroupID;
		}
	}
}


static void ClearPlacementsCallback(GUI_BUTTON* btn, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		iTPButtons[GROUP_BUTTON]->uiFlags &= ~BUTTON_CLICKED_ON;
		iTPButtons[GROUP_BUTTON]->uiFlags |= BUTTON_DIRTY;
		gubDefaultButton = CLEAR_BUTTON;
		PlaceMercs();
	}
}


static void MercMoveCallback(MOUSE_REGION* reg, INT32 reason)
{
	if( reg->uiFlags & MSYS_MOUSE_IN_AREA )
	{
		INT8 i;
		for( i = 0; i < giPlacements; i++ )
		{
			if( &gMercPlacement[ i ].region == reg )
			{
				if( gbHilightedMercID != i )
				{
					gbHilightedMercID = i;
					if( gubDefaultButton == GROUP_BUTTON )
						gubHilightedGroupID = gMercPlacement[ i ].pSoldier->ubGroupID;
					SetCursorMerc( i );
					gpTacticalPlacementHilightedSoldier = gMercPlacement[ i ].pSoldier;
				}
				return;
			}
		}
	}
}


static void MercClickCallback(MOUSE_REGION* reg, INT32 reason)
{
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		INT8 i;
		for( i = 0; i < giPlacements; i++ )
		{
			if( &gMercPlacement[ i ].region == reg )
			{
				if( gbSelectedMercID != i )
				{
					gbSelectedMercID = i;
					gpTacticalPlacementSelectedSoldier = gMercPlacement[ i ].pSoldier;
					if( gubDefaultButton == GROUP_BUTTON )
					{
						gubSelectedGroupID = gpTacticalPlacementSelectedSoldier->ubGroupID;
					}
				}
				return;
			}
		}
	}
}


static void SelectNextUnplacedUnit(void)
{
	INT32 i;
	if( gbSelectedMercID == -1 )
		return;
	for( i = gbSelectedMercID; i < giPlacements; i++ )
	{ //go from the currently selected soldier to the end
		if( !gMercPlacement[ i ].fPlaced )
		{ //Found an unplaced merc.  Select him.
			gbSelectedMercID = (INT8)i;
			if( gubDefaultButton == GROUP_BUTTON )
				gubSelectedGroupID = gMercPlacement[ i ].pSoldier->ubGroupID;
			gfTacticalPlacementGUIDirty = TRUE;
			SetCursorMerc( (INT8)i );
			gpTacticalPlacementSelectedSoldier = gMercPlacement[ i ].pSoldier;
			return;
		}
	}
	for( i = 0; i < gbSelectedMercID; i++ )
	{ //go from the beginning to the currently selected soldier
		if( !gMercPlacement[ i ].fPlaced )
		{ //Found an unplaced merc.  Select him.
			gbSelectedMercID = (INT8)i;
			if( gubDefaultButton == GROUP_BUTTON )
				gubSelectedGroupID = gMercPlacement[ i ].pSoldier->ubGroupID;
			gfTacticalPlacementGUIDirty = TRUE;
			SetCursorMerc( (INT8)i );
			gpTacticalPlacementSelectedSoldier = gMercPlacement[ i ].pSoldier;
			return;
		}
	}
	//checked the whole array, and everybody has been placed.  Select nobody.
	if( !gfEveryonePlaced )
	{
		gfEveryonePlaced = TRUE;
		SetCursorMerc( -1 );
		gbSelectedMercID = -1;
		gubSelectedGroupID = 0;
		gfTacticalPlacementGUIDirty = TRUE;
		gfValidLocationsChanged = TRUE;
		gpTacticalPlacementSelectedSoldier = gMercPlacement[ i ].pSoldier;
	}
}


static void DialogRemoved(MessageBoxReturnValue);


void HandleTacticalPlacementClicksInOverheadMap(INT32 reason)
{
	INT32 i;
	BOOLEAN fInvalidArea = FALSE;
	if( reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{ //if we have a selected merc, move him to the new closest map edgepoint of his side.
		if( gfValidCursor )
		{
			if( gbSelectedMercID != -1 )
			{
				const GridNo sGridNo = GetOverheadMouseGridNo();
				if (sGridNo != NOWHERE)
				{ //we have clicked within a valid part of the map.
					BeginMapEdgepointSearch();

					if( gubDefaultButton == GROUP_BUTTON )
					{ //We are placing a whole group.
						for( i = 0; i < giPlacements; i++ )
						{ //Find locations of each member of the group, but don't place them yet.  If
							//one of the mercs can't be placed, then we won't place any, and tell the user
							//the problem.  If everything's okay, we will place them all.
							if( gMercPlacement[ i ].pSoldier->ubGroupID == gubSelectedGroupID )
							{
								gMercPlacement[ i ].pSoldier->usStrategicInsertionData = SearchForClosestPrimaryMapEdgepoint( sGridNo, gMercPlacement[ i ].ubStrategicInsertionCode );
								if( gMercPlacement[ i ].pSoldier->usStrategicInsertionData == NOWHERE )
								{
									fInvalidArea = TRUE;
									break;
								}
							}
						}
						if( !fInvalidArea )
						{ //One or more of the mercs in the group didn't get gridno assignments, so we
							//report an error.
							for( i = 0; i < giPlacements; i++ )
							{
								gMercPlacement[ i ].pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
								if( gMercPlacement[ i ].pSoldier->ubGroupID == gubSelectedGroupID )
								{
									PutDownMercPiece( i );
								}
							}
						}
					}
					else
					{ //This is a single merc placement.  If valid, then place him, else report error.
						gMercPlacement[ gbSelectedMercID ].pSoldier->usStrategicInsertionData = SearchForClosestPrimaryMapEdgepoint( sGridNo, gMercPlacement[ gbSelectedMercID ].ubStrategicInsertionCode );
						if( gMercPlacement[ gbSelectedMercID ].pSoldier->usStrategicInsertionData != NOWHERE )
						{
							gMercPlacement[ gbSelectedMercID ].pSoldier->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
							PutDownMercPiece( gbSelectedMercID );
						}
						else
						{
							fInvalidArea = TRUE;
						}

						//gbSelectedMercID++;
						//if( gbSelectedMercID == giPlacements )
						//	gbSelectedMercID = 0;
						//gpTacticalPlacementSelectedSoldier = gMercPlacement[ gbSelectedMercID ].pSoldier;
						gfTacticalPlacementGUIDirty = TRUE;
						//SetCursorMerc( gbSelectedMercID );
					}
					EndMapEdgepointSearch();

					if( fInvalidArea )
					{ //Report error due to invalid placement.
						SGPBox const CenterRect = { 220, 120, 200, 80 };
						DoMessageBox(MSG_BOX_BASIC_STYLE, gpStrategicString[STR_TP_INACCESSIBLE_MESSAGE], guiCurrentScreen, MSG_BOX_FLAG_OK, DialogRemoved, &CenterRect);
					}
					else
					{ //Placement successful, so select the next unplaced unit (single or group).
						SelectNextUnplacedUnit();
					}
				}
			}
		}
		else
		{ //not a valid cursor location...
			if( gbCursorMercID != - 1 )
			{
				SGPBox const CenterRect = { 220, 120, 200, 80 };
				DoMessageBox(MSG_BOX_BASIC_STYLE, gpStrategicString[STR_TP_INVALID_MESSAGE], guiCurrentScreen, MSG_BOX_FLAG_OK, DialogRemoved, &CenterRect);
			}
		}
	}
}


static void SetCursorMerc(INT8 bPlacementID)
{
	if( gbCursorMercID != bPlacementID )
	{
		if( gbCursorMercID == -1 || bPlacementID == -1 ||
			  gMercPlacement[ gbCursorMercID ].ubStrategicInsertionCode != gMercPlacement[ bPlacementID ].ubStrategicInsertionCode )
			gfValidLocationsChanged = TRUE;
		gbCursorMercID = bPlacementID;
	}
}


static void PutDownMercPiece(INT32 iPlacement)
{
	SOLDIERTYPE *pSoldier;
	pSoldier = gMercPlacement[ iPlacement ].pSoldier;
	switch( pSoldier->ubStrategicInsertionCode )
	{
		case INSERTION_CODE_NORTH:
			pSoldier->sInsertionGridNo = gMapInformation.sNorthGridNo;
			break;
		case INSERTION_CODE_SOUTH:
			pSoldier->sInsertionGridNo = gMapInformation.sSouthGridNo;
			break;
		case INSERTION_CODE_EAST:
			pSoldier->sInsertionGridNo = gMapInformation.sEastGridNo;
			break;
		case INSERTION_CODE_WEST:
			pSoldier->sInsertionGridNo = gMapInformation.sWestGridNo;
			break;
		case INSERTION_CODE_GRIDNO:
			pSoldier->sInsertionGridNo = pSoldier->usStrategicInsertionData;
			break;
		default:
			Assert( 0 );
			break;
	}
	if( gMercPlacement[ iPlacement ].fPlaced )
		PickUpMercPiece( iPlacement );
	const INT16 sGridNo = FindGridNoFromSweetSpot(pSoldier, pSoldier->sInsertionGridNo, 4);
	if( sGridNo != NOWHERE )
	{
		EVENT_SetSoldierPositionNoCenter(pSoldier, sGridNo, SSP_NONE);
		const UINT8 ubDirection = GetDirectionToGridNoFromGridNo(sGridNo, CENTER_GRIDNO);
		EVENT_SetSoldierDirection( pSoldier, ubDirection );
		pSoldier->ubInsertionDirection = pSoldier->bDirection;
		gMercPlacement[ iPlacement ].fPlaced = TRUE;
		gMercPlacement[ iPlacement ].pSoldier->bInSector = TRUE;
	}
}


static void PickUpMercPiece(INT32 iPlacement)
{
	RemoveSoldierFromGridNo( gMercPlacement[ iPlacement ].pSoldier );
	gMercPlacement[ iPlacement ].fPlaced = FALSE;
	gMercPlacement[ iPlacement ].pSoldier->bInSector = FALSE;
}


static void DialogRemoved(MessageBoxReturnValue const ubResult)
{
	gfTacticalPlacementGUIDirty = TRUE;
	gfValidLocationsChanged = TRUE;
}
