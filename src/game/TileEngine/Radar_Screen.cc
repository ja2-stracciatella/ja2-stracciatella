#include "Directories.h"
#include "Font.h"
#include "HImage.h"
#include "Local.h"
#include "MapScreen.h"
#include "Radar_Screen.h"
#include "Line.h"
#include "RenderWorld.h"
#include "Isometric_Utils.h"
#include "Interface.h"
#include "Overhead.h"
#include "Soldier_Control.h"
#include "Timer_Control.h"
#include "SysUtil.h"
#include "Render_Dirty.h"
#include "Overhead_Map.h"
#include "Squads.h"
#include "MouseSystem.h"
#include "Text.h"
#include "Font_Control.h"
#include "VObject.h"
#include "Interface_Control.h"
#include "Game_Clock.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "Environment.h"
#include "Meanwhile.h"
#include "StrategicMap.h"
#include "Animation_Data.h"
#include "JAScreens.h"
#include "Video.h"
#include "VSurface.h"
#include "Button_System.h"
#include "ScreenIDs.h"
#include "UILayout.h"

#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <cmath>

extern INT32 iCurrentMapSectorZ;

// the squad list font
#define SQUAD_FONT COMPFONT

#define SQUAD_REGION_HEIGHT 2 * RADAR_WINDOW_HEIGHT
#define SQUAD_WINDOW_TM_Y RADAR_WINDOW_TM_Y + GetFontHeight( SQUAD_FONT )

// subtractor for squad list from size of radar view region height
#define SUBTRACTOR_FOR_SQUAD_LIST 0


static SGPVObject* gusRadarImage;
BOOLEAN   fRenderRadarScreen = TRUE;
static INT16       sSelectedSquadLine = -1;

BOOLEAN		gfRadarCurrentGuyFlash = FALSE;


static MOUSE_REGION gRadarRegionSquadList[NUMBER_OF_SQUADS];


static void RadarRegionButtonCallback(MOUSE_REGION* pRegion, INT32 iReason);
static void RadarRegionMoveCallback(MOUSE_REGION* pRegion, INT32 iReason);


void InitRadarScreen()
{
	// Add region for radar
	UINT16        const x = RADAR_WINDOW_X;
	UINT16        const y = RADAR_WINDOW_TM_Y;
	UINT16        const w = RADAR_WINDOW_WIDTH;
	UINT16        const h = RADAR_WINDOW_HEIGHT;
	MOUSE_REGION* const r = &gRadarRegion;
	MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, 0, RadarRegionMoveCallback, RadarRegionButtonCallback);
	r->Disable();
}


void LoadRadarScreenBitmap(const ST::string& filename)
{
	ClearOutRadarMapImage();

	// Grab the Map image
	ST::string image_filename(GCM->getRadarMapResourceName(FileMan::replaceExtension(FileMan::getFileName(filename), "sti")));

	SGPVObject* const radar = AddVideoObjectFromFile(image_filename.c_str());
	gusRadarImage = radar;

	// ATE: Add a shade table!
	const SGPPaletteEntry* const pal = radar->Palette();
	radar->pShades[0] = Create16BPPPaletteShaded(pal, 255, 255, 255, FALSE);
	radar->pShades[1] = Create16BPPPaletteShaded(pal, 100, 100, 100, FALSE);

	// Dirty interface
	fInterfacePanelDirty = DIRTYLEVEL1;
}


void ClearOutRadarMapImage( void )
{
	// If we have loaded, remove old one
	if (gusRadarImage)
	{
		DeleteVideoObject(gusRadarImage);
		gusRadarImage = 0;
	}
}


void MoveRadarScreen( )
{
	// check if we are allowed to do anything?
	if (!fRenderRadarScreen) return;

	gRadarRegion.RegionTopLeftX     = RADAR_WINDOW_X;
	gRadarRegion.RegionTopLeftY     = RADAR_WINDOW_TM_Y;
	gRadarRegion.RegionBottomRightX = RADAR_WINDOW_X + RADAR_WINDOW_WIDTH;
	gRadarRegion.RegionBottomRightY = RADAR_WINDOW_TM_Y + RADAR_WINDOW_HEIGHT;

}


static void AdjustWorldCenterFromRadarCoords(INT16 sRadarX, INT16 sRadarY);


static void RadarRegionMoveCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT16 sRadarX, sRadarY;

	// check if we are allowed to do anything?
	if (!fRenderRadarScreen) return;

	if (iReason == MSYS_CALLBACK_REASON_MOVE )
	{
		if ( pRegion->ButtonState & MSYS_LEFT_BUTTON )
		{
			// Use relative coordinates to set center of viewport
			sRadarX = pRegion->RelativeXPos - ( RADAR_WINDOW_WIDTH / 2 );
			sRadarY = pRegion->RelativeYPos - ( RADAR_WINDOW_HEIGHT / 2 );

			AdjustWorldCenterFromRadarCoords( sRadarX, sRadarY );

			SetRenderFlags(RENDER_FLAG_FULL);

		}
	}
}


static void RadarRegionButtonCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT16 sRadarX, sRadarY;

	// check if we are allowed to do anything?
	if (!fRenderRadarScreen) return;

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_DWN)
	{
		if ( !InOverheadMap( ) )
		{
			// Use relative coordinates to set center of viewport
			sRadarX = pRegion->RelativeXPos - ( RADAR_WINDOW_WIDTH / 2 );
			sRadarY = pRegion->RelativeYPos - ( RADAR_WINDOW_HEIGHT / 2 );

			AdjustWorldCenterFromRadarCoords( sRadarX, sRadarY );
		}
		else
		{
			KillOverheadMap();
		}
	}
	else if (iReason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		if ( !InOverheadMap( ) )
		{
			GoIntoOverheadMap( );
		}
		else
		{
			KillOverheadMap();
		}
	}
}


static void CreateDestroyMouseRegionsForSquadList(void);
static void RenderSquadList(void);


void RenderRadarScreen()
{
	// create / destroy squad list regions as nessacary
	CreateDestroyMouseRegionsForSquadList();

	// check if we are allowed to do anything?
	if (!fRenderRadarScreen)
	{
		RenderSquadList();
		return;
	}

	// in a meanwhile, don't render any map
	if (AreInMeanwhile()) ClearOutRadarMapImage();

	if (fInterfacePanelDirty == DIRTYLEVEL2 && gusRadarImage)
	{
		// If night time and on surface, darken the radarmap.
		size_t const shade =
			NightTime() &&
			(
				(guiCurrentScreen == MAP_SCREEN  && iCurrentMapSectorZ == 0) ||
				(guiCurrentScreen == GAME_SCREEN && gWorldSector.z     == 0)
			) ? 1 : 0;
		gusRadarImage->CurrentShade(shade);
		BltVideoObject(guiSAVEBUFFER, gusRadarImage, 0, RADAR_WINDOW_X, RADAR_WINDOW_TM_Y);
	}

	// First delete what's there
	RestoreExternBackgroundRect(RADAR_WINDOW_X, RADAR_WINDOW_TM_Y, RADAR_WINDOW_WIDTH + 1, RADAR_WINDOW_HEIGHT + 1);

	{
		SGPVSurface::Lock l(FRAME_BUFFER);

		SetClippingRegionAndImageWidth(l.Pitch(), RADAR_WINDOW_X, RADAR_WINDOW_TM_Y, RADAR_WINDOW_WIDTH, RADAR_WINDOW_HEIGHT);
		UINT16* const pDestBuf = l.Buffer<UINT16>();

		// Cycle fFlash variable
		if (COUNTERDONE(RADAR_MAP_BLINK))
		{
			RESETCOUNTER(RADAR_MAP_BLINK);
			gfRadarCurrentGuyFlash = !gfRadarCurrentGuyFlash;
		}

		if (!fInMapMode)
		{
			RectangleDraw(TRUE,
				RADAR_WINDOW_X + std::max(double(0), round((gsTopLeftWorldX - SCROLL_LEFT_PADDING) * gdScaleX)),
				RADAR_WINDOW_TM_Y + std::max(double(0), round((gsTopLeftWorldY - SCROLL_TOP_PADDING) * gdScaleY)),
				RADAR_WINDOW_X + MIN(round((gsBottomRightWorldX - SCROLL_RIGHT_PADDING - SCROLL_LEFT_PADDING) * gdScaleX - 1.0), double(RADAR_WINDOW_WIDTH - 1)),
				RADAR_WINDOW_TM_Y + MIN(round((gsBottomRightWorldY - SCROLL_BOTTOM_PADDING - SCROLL_TOP_PADDING) * gdScaleY - 1.0), double(RADAR_WINDOW_HEIGHT - 1)),
				Get16BPPColor(FROMRGB(0, 255, 0)), pDestBuf);

			// Re-render radar
			FOR_EACH_MERC(i)
			{
				SOLDIERTYPE const* const s = *i;

				// Don't place guys in radar until visible!
				if (s->bVisible == -1 &&
						!(gTacticalStatus.uiFlags & SHOW_ALL_MERCS) &&
						!(s->ubMiscSoldierFlags & SOLDIER_MISC_XRAYED))
				{
					continue;
				}

				if (s->uiStatusFlags & SOLDIER_DEAD)       continue;
				if (s->ubBodyType == CROW)                 continue;
				if (!GridNoOnVisibleWorldTile(s->sGridNo)) continue;

				// Get fullscreen coordinate for guy's position
				INT16 sXSoldScreen;
				INT16 sYSoldScreen;
				GetAbsoluteScreenXYFromMapPos(s->sGridNo, &sXSoldScreen, &sYSoldScreen);

				// Get radar x and y postion and add starting relative to interface
				const INT16 x = floor(DOUBLE(sXSoldScreen) * gdScaleX) + RADAR_WINDOW_X;
				const INT16 y = floor(DOUBLE(sYSoldScreen) * gdScaleY) + RADAR_WINDOW_TM_Y;

				UINT32 const line_colour =
					/* flash selected merc */
					s == GetSelectedMan() && gfRadarCurrentGuyFlash                 ? 0                      :
					/* on roof */
					s->bTeam == OUR_TEAM && s->bLevel > 0                        ? FROMRGB(150, 150,   0) :
					/* unconscious enemy */
					s->bTeam != OUR_TEAM && s->bLife < OKLIFE                    ? FROMRGB(128, 128, 128) :
					/* hostile civilian */
					s->bTeam == CIV_TEAM && !s->bNeutral && s->bSide != OUR_TEAM ? FROMRGB(255,   0,   0) :
					gTacticalStatus.Team[s->bTeam].RadarColor;

				RectangleDraw(TRUE, x, y, x + 1, y + 1, Get16BPPColor(line_colour), pDestBuf);
			}
		}
		else if (fShowMapInventoryPool)
		{
			if (iCurrentlyHighLightedItem != -1)
			{
				INT32     const  item_idx = iCurrentInventoryPoolPage * MAP_INVENTORY_POOL_SLOT_COUNT + iCurrentlyHighLightedItem;
				WORLDITEM const& wi       = pInventoryPoolList[item_idx];
				if (wi.o.ubNumberOfObjects != 0 && wi.sGridNo != 0)
				{
					INT16	sXSoldScreen;
					INT16 sYSoldScreen;
					GetAbsoluteScreenXYFromMapPos(wi.sGridNo, &sXSoldScreen, &sYSoldScreen);

					// Get radar x and y postion and add starting relative to interface
					INT16  const x = sXSoldScreen * gdScaleX + RADAR_WINDOW_X;
					INT16  const y = sYSoldScreen * gdScaleY + RADAR_WINDOW_TM_Y;

					UINT16 const line_colour = fFlashHighLightInventoryItemOnradarMap ?
						Get16BPPColor(FROMRGB(  0, 255,   0)) :
						Get16BPPColor(FROMRGB(255, 255, 255));

					RectangleDraw(TRUE, x, y, x + 1, y + 1, line_colour, pDestBuf);
				}
			}
			InvalidateRegion(RADAR_WINDOW_X, RADAR_WINDOW_TM_Y, RADAR_WINDOW_X + RADAR_WINDOW_WIDTH, RADAR_WINDOW_TM_Y + RADAR_WINDOW_HEIGHT);
		}
	}
}


static void AdjustWorldCenterFromRadarCoords(INT16 sRadarX, INT16 sRadarY)
{
	const INT16 SCROLL_X_STEP = WORLD_TILE_X;
	const INT16 SCROLL_Y_STEP = WORLD_TILE_Y * 2;

	INT16 sScreenX, sScreenY;
	INT16	sTempX_W, sTempY_W;
	INT16 sNewCenterWorldX, sNewCenterWorldY;
	INT16 sNumXSteps, sNumYSteps;

	// Use radar scale values to get screen values, then convert ot map values, rounding to nearest middle tile
	sScreenX = (INT16) ( sRadarX / gdScaleX );
	sScreenY = (INT16) ( sRadarY / gdScaleY );

	// Adjust to viewport start!
	sScreenX -= ( g_ui.m_tacticalMapCenterX );
	sScreenY -= ( g_ui.m_tacticalMapCenterY );

	//Make sure these coordinates are multiples of scroll steps
	sNumXSteps = sScreenX  / SCROLL_X_STEP;
	sNumYSteps = sScreenY / SCROLL_Y_STEP;

	sScreenX = ( sNumXSteps * SCROLL_X_STEP );
	sScreenY = ( sNumYSteps * SCROLL_Y_STEP );

	// Adjust back
	sScreenX += ( g_ui.m_tacticalMapCenterX );
	sScreenY += ( g_ui.m_tacticalMapCenterY );

	// Subtract world center
	//sScreenX += gsCX;
	//sScreenY += gsCY;

	// Convert these into world coordinates
	FromScreenToCellCoordinates( sScreenX, sScreenY, &sTempX_W, &sTempY_W );

	// Adjust these to world center
	sNewCenterWorldX = (INT16)(gCenterWorldX + sTempX_W);
	sNewCenterWorldY = (INT16)(gCenterWorldY + sTempY_W);

	SetRenderCenter( sNewCenterWorldX, sNewCenterWorldY );
}


void ToggleRadarScreenRender( void )
{
	fRenderRadarScreen = ! fRenderRadarScreen;
}


static void TacticalSquadListBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason);
static void TacticalSquadListMvtCallback(MOUSE_REGION* pRegion, INT32 iReason);


// create destroy squad list regions as needed
static void CreateDestroyMouseRegionsForSquadList(void)
{
	// will check the state of renderradarscreen flag and decide if we need to create mouse regions for
	static BOOLEAN fCreated = FALSE;

	if (!fRenderRadarScreen && !fCreated)
	{
		BltVideoObjectOnce(guiSAVEBUFFER, INTERFACEDIR "/squadpanel.sti", 0, INTERFACE_START_X + 538, gsVIEWPORT_END_Y);
		RestoreExternBackgroundRect(INTERFACE_START_X + 538, gsVIEWPORT_END_Y, 102, 120);

		// create regions
		INT16 const w = RADAR_WINDOW_WIDTH / 2 - 1;
		INT16 const h = (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / (NUMBER_OF_SQUADS / 2);
		for (UINT i = 0; i < NUMBER_OF_SQUADS; ++i)
		{
			// run through list of squads and place appropriatly
			INT16 x = RADAR_WINDOW_X;
			INT16 y = SQUAD_WINDOW_TM_Y;
			if (i < NUMBER_OF_SQUADS / 2)
			{
				// left half of list
				y += i * h;
			}
			else
			{
				// right half of list
				x += RADAR_WINDOW_WIDTH / 2;
				y += (i - NUMBER_OF_SQUADS / 2) * h;
			}

			MOUSE_REGION* const r = &gRadarRegionSquadList[i];
			MSYS_DefineRegion(r, x, y, x + w, y + h, MSYS_PRIORITY_HIGHEST, 0, TacticalSquadListMvtCallback, TacticalSquadListBtnCallBack);
			MSYS_SetRegionUserData(r, 0, i);
		}

		sSelectedSquadLine = -1;

		fCreated = TRUE;
	}
	else if (fRenderRadarScreen && fCreated)
	{
		// destroy regions
		for (UINT i = 0; i < NUMBER_OF_SQUADS; ++i)
		{
			MSYS_RemoveRegion(&gRadarRegionSquadList[i]);
		}

		if (guiCurrentScreen == GAME_SCREEN)
		{
			fInterfacePanelDirty = DIRTYLEVEL2;
			MarkButtonsDirty();
			RenderTacticalInterface();
			RenderButtons();
			RenderPausedGameBox();
		}

		fCreated = FALSE;
	}
}


// show list of squads
static void RenderSquadList(void)
{
	INT16 const dx = RADAR_WINDOW_X;
	INT16 const dy = RADAR_WINDOW_TM_Y;

	RestoreExternBackgroundRect(dx, dy, RADAR_WINDOW_WIDTH, SQUAD_REGION_HEIGHT);
	ColorFillVideoSurfaceArea(FRAME_BUFFER, dx, dy, dx + RADAR_WINDOW_WIDTH, dy + SQUAD_REGION_HEIGHT, Get16BPPColor(FROMRGB(0, 0, 0)));

	SetFont(SQUAD_FONT);
	SetFontBackground(FONT_BLACK);

	for (INT16 i = 0; i < NUMBER_OF_SQUADS; ++i)
	{
		const UINT8 colour =
			sSelectedSquadLine == i         ? FONT_WHITE   : // highlight line?
			!IsSquadOnCurrentTacticalMap(i) ? FONT_BLACK   :
			CurrentSquad() == i             ? FONT_LTGREEN :
			                                  FONT_DKGREEN;
		SetFontForeground(colour);

		INT16 sX;
		INT16 sY;
		INT16       x = dx;
		INT16       y = SQUAD_WINDOW_TM_Y;
		INT16 const w = RADAR_WINDOW_WIDTH / 2 - 1;
		INT16 const h = 2 * (SQUAD_REGION_HEIGHT - SUBTRACTOR_FOR_SQUAD_LIST) / NUMBER_OF_SQUADS;
		if (i < NUMBER_OF_SQUADS / 2)
		{
			x += 2;
			y += i * h;
		}
		else
		{
			x += RADAR_WINDOW_WIDTH / 2 - 2;
			y += (i - NUMBER_OF_SQUADS / 2) * h;
		}
		FindFontCenterCoordinates(x, y, w, h, pSquadMenuStrings[i], SQUAD_FONT, &sX, &sY);
		MPrint(x, sY, pSquadMenuStrings[i]);
	}
}


static void TacticalSquadListMvtCallback(MOUSE_REGION* pRegion, INT32 iReason)
{
	INT32 iValue = -1;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_GAIN_MOUSE )
	{
		if (IsSquadOnCurrentTacticalMap(iValue))
		{
			sSelectedSquadLine = ( INT16 )iValue;
		}
	}
	if (iReason & MSYS_CALLBACK_REASON_LOST_MOUSE )
	{
		sSelectedSquadLine = -1;
	}
}


static void TacticalSquadListBtnCallBack(MOUSE_REGION* pRegion, INT32 iReason)
{
	// btn callback handler for team list info region
	INT32 iValue = 0;

	iValue = MSYS_GetRegionUserData( pRegion, 0 );

	if (iReason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		// find out if this squad is valid and on this map..if so, set as selected
		if (IsSquadOnCurrentTacticalMap(iValue))
		{
			// ok, squad is here, set as selected
			SetCurrentSquad( iValue, FALSE );

			// stop showing
			fRenderRadarScreen = TRUE;
		}
	}
}
