#include "Font.h"
#include "HImage.h"
#include "Handle_Items.h"
#include "Interface_Items.h"
#include "Local.h"
#include "Structure.h"
#include "VObject.h"
#include "TileDef.h"
#include "VSurface.h"
#include "WorldDef.h"
#include "Isometric_Utils.h"
#include "RenderWorld.h"
#include "WorldDat.h"
#include "VObject_Blitters.h"
#include "Overhead_Map.h"
#include "Interface.h"
#include "Interface_Control.h"
#include "Overhead.h"
#include "Radar_Screen.h"
#include "Cursors.h"
#include "Sys_Globals.h"
#include "Render_Dirty.h"
#include "Soldier_Find.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Interface_Panels.h"
#include "English.h"
#include "Line.h"
#include "Map_Information.h"
#include "Tactical_Placement_GUI.h"
#include "World_Items.h"
#include "Environment.h"
#include "Faces.h"
#include "Squads.h"
#include "GameLoop.h"
#include "SysUtil.h"
#include "Tile_Surface.h"
#include "Button_System.h"
#include "Video.h"

#ifdef JA2DEMO
#	include "Message.h"
#	include "Text.h"
#endif

#ifdef JA2EDITOR
#include "Soldier_Init_List.h"
extern SOLDIERINITNODE *gpSelected;
#endif

// OK, these are values that are calculated in InitRenderParams( ) with normal view settings.
// These would be different if we change ANYTHING about the game worlkd map sizes...
#define	NORMAL_MAP_SCREEN_WIDTH			3160
#define	NORMAL_MAP_SCREEN_HEIGHT		1540
#define	NORMAL_MAP_SCREEN_X					1580
#define	NORMAL_MAP_SCREEN_BY				2400
#define	NORMAL_MAP_SCREEN_TY				860

#define FASTMAPROWCOLTOPOS( r, c )									( (r) * WORLD_COLS + (c) )


struct SMALL_TILE_SURF
{
	HVOBJECT vo;
};

struct SMALL_TILE_DB
{
	HVOBJECT	vo;
	UINT16		usSubIndex;
};


static SMALL_TILE_SURF gSmTileSurf[NUMBEROFTILETYPES];
static SMALL_TILE_DB   gSmTileDB[NUMBEROFTILES];
static TileSetID       gubSmTileNum                   = TILESET_INVALID;
static BOOLEAN         gfInOverheadMap = FALSE;
static MOUSE_REGION    OverheadRegion;
static MOUSE_REGION    OverheadBackgroundRegion;
static SGPVObject*     uiOVERMAP;
static SGPVObject*     uiPERSONS;
BOOLEAN                gfOverheadMapDirty             = FALSE;
extern BOOLEAN		gfRadarCurrentGuyFlash;
static INT16           gsStartRestrictedX;
static INT16           gsStartRestrictedY;
static INT16           gsOveritemPoolGridNo           = NOWHERE;


static void CopyOverheadDBShadetablesFromTileset(void);


void InitNewOverheadDB(TileSetID const ubTilesetID)
{
	if (gubSmTileNum == ubTilesetID) return;
	TrashOverheadMap();

	for (UINT32 i = 0; i < NUMBEROFTILETYPES; ++i)
	{
		const char* filename    = gTilesets[ubTilesetID].TileSurfaceFilenames[i];
		TileSetID   use_tileset = ubTilesetID;
		if (filename[0] == '\0')
		{
			// Try loading from default tileset
			filename    = gTilesets[GENERIC_1].TileSurfaceFilenames[i];
			use_tileset = GENERIC_1;
		}

		char adjusted_file[128];
		sprintf(adjusted_file, "TILESETS/%d/T/%s", use_tileset, filename);
		SGPVObject* vo;
		try
		{
			vo = AddVideoObjectFromFile(adjusted_file);
		}
		catch (...)
		{
			// Load one we know about
			vo = AddVideoObjectFromFile("TILESETS/0/T/grass.sti");
		}

		gSmTileSurf[i].vo = vo;
	}

	// Create database
	UINT32 dbSize = 0;
	for (UINT32 i = 0; i < NUMBEROFTILETYPES; ++i)
	{
		SGPVObject* const vo = gSmTileSurf[i].vo;

		// Get number of regions and check for overflow
		UINT32 const NumRegions = MIN(vo->SubregionCount(), gNumTilesPerType[i]);

		UINT32 k = 0;
		for (; k < NumRegions; ++k)
		{
			gSmTileDB[dbSize].vo         = vo;
			gSmTileDB[dbSize].usSubIndex = k;
			++dbSize;
		}

		// Handle underflow
		for (; k < gNumTilesPerType[i]; ++k)
		{
			gSmTileDB[dbSize].vo         = vo;
			gSmTileDB[dbSize].usSubIndex = 0;
			++dbSize;
		}
	}

	gsStartRestrictedX = 0;
	gsStartRestrictedY = 0;

	// Calculate Scale factors because of restricted map scroll regions
	if (gMapInformation.ubRestrictedScrollID != 0)
	{
		INT16 sX1;
		INT16 sY1;
		INT16 sX2;
		INT16 sY2;
		CalculateRestrictedMapCoords(NORTH, &sX1, &sY1, &sX2, &gsStartRestrictedY, SCREEN_WIDTH, 320);
		CalculateRestrictedMapCoords(WEST,  &sX1, &sY1, &gsStartRestrictedX, &sY2, SCREEN_WIDTH, 320);
	}

	// Copy over shade tables from main tileset
	CopyOverheadDBShadetablesFromTileset();
	gubSmTileNum = ubTilesetID;
}


static const ITEM_POOL* GetClosestItemPool(INT16 sSweetGridNo, UINT8 ubRadius, INT8 bLevel)
{
	const ITEM_POOL* ClosestItemPool = NULL;
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.

	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	uiLowestRange = 999999;

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				// Go on sweet stop
				const ITEM_POOL* pItemPool = GetItemPool(sGridNo, bLevel);
				if (pItemPool != NULL)
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

					if ( uiRange < uiLowestRange )
					{
						ClosestItemPool = pItemPool;
						uiLowestRange = uiRange;
					}
				}
			}
		}
	}

	return ClosestItemPool;
}


static SOLDIERTYPE* GetClosestMercInOverheadMap(INT16 sSweetGridNo, UINT8 ubRadius)
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;

	//create dummy soldier, and use the pathing to determine which nearby slots are
	//reachable.

	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	uiLowestRange = 999999;

	SOLDIERTYPE* res = NULL;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( sSweetGridNo + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			sGridNo = sSweetGridNo + ( WORLD_COLS * cnt1 ) + cnt2;
			if( sGridNo >=0 && sGridNo < WORLD_MAX && sGridNo >= leftmost && sGridNo < ( leftmost + WORLD_COLS ) )
			{
				// Go on sweet stop
        if ( gpWorldLevelData[ sGridNo ].pMercHead != NULL && gpWorldLevelData[ sGridNo ].pMercHead->pSoldier->bVisible != -1 )
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

					if ( uiRange < uiLowestRange )
					{
						res = gpWorldLevelData[sGridNo].pMercHead->pSoldier;
						uiLowestRange = uiRange;
					}
				}
			}
		}
	}

	return res;
}


static void DisplayMercNameInOverhead(const SOLDIERTYPE* const pSoldier)
{
	INT16		sWorldScreenX, sX;
	INT16		sWorldScreenY, sY;

	// Get Screen position of guy.....
	GetAbsoluteScreenXYFromMapPos(GETWORLDINDEXFROMWORLDCOORDS(pSoldier->sY, pSoldier->sX), &sWorldScreenX, &sWorldScreenY);

	sWorldScreenX = gsStartRestrictedX + ( sWorldScreenX / 5 ) + 5;
	sWorldScreenY = gsStartRestrictedY + ( sWorldScreenY / 5 ) + ( pSoldier->sHeightAdjustment / 5 ) + (gpWorldLevelData[ pSoldier->sGridNo ].sHeight/5) - 8;

	sWorldScreenY += ( gsRenderHeight / 5 );

	// Display name
	SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);

	// Center here....
	FindFontCenterCoordinates(sWorldScreenX, sWorldScreenY, 1, 1, pSoldier->name, TINYFONT1, &sX, &sY);

	// OK, selected guy is here...
	GDirtyPrint(sX, sY, pSoldier->name);
}


static GridNo GetOverheadMouseGridNoForFullSoldiersGridNo(void);
static void   HandleOverheadUI(void);
static void   RenderOverheadOverlays(void);


void HandleOverheadMap(void)
{
	gfInOverheadMap      = TRUE;
	gsOveritemPoolGridNo = NOWHERE;

	InitNewOverheadDB(giCurrentTilesetID);

	RestoreBackgroundRects();

	RenderOverheadMap(0, WORLD_COLS / 2, 0, 0, SCREEN_WIDTH, 320, FALSE);

	HandleTalkingAutoFaces();

	if (!gfEditMode)
	{
		if (gfTacticalPlacementGUIActive)
		{
			TacticalPlacementHandle();
			if (!gfTacticalPlacementGUIActive) return;
		}
		else
		{
			HandleOverheadUI();

			if (!gfInOverheadMap) return;
			RenderTacticalInterface();
			RenderRadarScreen();
			RenderClock();
			RenderTownIDString();

    	HandleAutoFaces();
		}
	}

	if (!gfEditMode && !gfTacticalPlacementGUIActive)
	{
		HandleAnyMercInSquadHasCompatibleStuff(NULL);

		INT16 usMapPos;
		usMapPos = GetOverheadMouseGridNo();
		if (usMapPos != NOWHERE)
		{
			const ITEM_POOL* pItemPool;

			// ATE: Find the closest item pool within 5 tiles....
			pItemPool = GetClosestItemPool(usMapPos, 1, 0);
			if (pItemPool != NULL)
			{
				const STRUCTURE* const structure = FindStructure(usMapPos, STRUCTURE_HASITEMONTOP | STRUCTURE_OPENABLE);
				INT8             const bZLevel   = GetZLevelOfItemPoolGivenStructure(usMapPos, 0, structure);
				if (AnyItemsVisibleOnLevel(pItemPool, bZLevel))
				{
					DrawItemPoolList(pItemPool, bZLevel, gusMouseXPos, gusMouseYPos);
					gsOveritemPoolGridNo = GetWorldItem(pItemPool->iItemIndex)->sGridNo;
				}
			}

			pItemPool = GetClosestItemPool(usMapPos, 1, 1);
			if (pItemPool != NULL)
			{
				const INT8 bZLevel = 0;
				if (AnyItemsVisibleOnLevel(pItemPool, bZLevel))
				{
					DrawItemPoolList(pItemPool, bZLevel, gusMouseXPos, gusMouseYPos - 5);
					gsOveritemPoolGridNo = GetWorldItem(pItemPool->iItemIndex)->sGridNo;
				}
			}
    }

		gSelectedGuy = NULL;
		usMapPos = GetOverheadMouseGridNoForFullSoldiersGridNo();
		if (usMapPos != NOWHERE)
    {
			SOLDIERTYPE* const s = GetClosestMercInOverheadMap(usMapPos, 1);
			if (s != NULL)
			{
				if (s->bTeam == gbPlayerNum) gSelectedGuy = s;
				DisplayMercNameInOverhead(s);
      }
		}
	}

	RenderOverheadOverlays();
	if (!gfEditMode && !gfTacticalPlacementGUIActive)
	{
		const SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel != NULL) DisplayMercNameInOverhead(sel);
	}

	RenderButtons();
	SaveBackgroundRects();
	RenderButtonsFastHelp();
	ExecuteBaseDirtyRectQueue();
	EndFrameBufferRender();
	fInterfacePanelDirty = FALSE;
}


BOOLEAN InOverheadMap( )
{
	return( gfInOverheadMap );
}


static void ClickOverheadRegionCallback(MOUSE_REGION* reg, INT32 reason);


void GoIntoOverheadMap( )
{
#ifdef JA2DEMO
	if (gfCaves)
	{
		ScreenMsg(FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, str_overhead_map_disabled);
		return;
	}
#endif

	gfInOverheadMap = TRUE;

	MSYS_DefineRegion(&OverheadBackgroundRegion, 0, 0, SCREEN_WIDTH, 360, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK);

	MSYS_DefineRegion(&OverheadRegion, 0, 0, gsVIEWPORT_END_X, 320, MSYS_PRIORITY_HIGH, CURSOR_NORMAL, MSYS_NO_CALLBACK, ClickOverheadRegionCallback);

	// LOAD CLOSE ANIM
	uiOVERMAP = AddVideoObjectFromFile("INTERFACE/MAP_BORD.sti");

	// LOAD PERSONS
	uiPERSONS = AddVideoObjectFromFile("INTERFACE/PERSONS.sti");

	// Add shades to persons....
	SGPVObject*            const vo  = uiPERSONS;
	SGPPaletteEntry const* const pal = vo->Palette();
	vo->pShades[0] = Create16BPPPaletteShaded(pal, 256, 256, 256, FALSE);
	vo->pShades[1] = Create16BPPPaletteShaded(pal, 310, 310, 310, FALSE);
	vo->pShades[2] = Create16BPPPaletteShaded(pal,   0,   0,   0, FALSE);

	gfOverheadMapDirty = TRUE;

	if( !gfEditMode )
	{
		// Make sure we are in team panel mode...
		gfSwitchPanel = TRUE;
		gbNewPanel = TEAM_PANEL;
		gNewPanelSoldier = GetSelectedMan();
		fInterfacePanelDirty = DIRTYLEVEL2;

		// Disable tactical buttons......
		if( !gfEnterTacticalPlacementGUI )
		{
			// Handle switch of panel....
			HandleTacticalPanelSwitch( );
			DisableTacticalTeamPanelButtons( TRUE );
		}

		EmptyBackgroundRects( );
	}

}


static void HandleOverheadUI(void)
{
	InputAtom a;
	while (DequeueEvent(&a))
	{
		if (a.usEvent == KEY_DOWN)
		{
			switch (a.usParam)
			{
				case SDLK_ESCAPE:
				case SDLK_INSERT:
					KillOverheadMap();
					break;

				case 'x':
					if (a.usKeyState & ALT_DOWN)
					{
						HandleShortCutExitState();
					}
					break;
			}
		}
	}
}


void KillOverheadMap()
{
	gfInOverheadMap = FALSE;
	SetRenderFlags( RENDER_FLAG_FULL );
	RenderWorld( );

	MSYS_RemoveRegion(&OverheadRegion );
	MSYS_RemoveRegion(&OverheadBackgroundRegion );

	DeleteVideoObject(uiOVERMAP);
	DeleteVideoObject(uiPERSONS);

	HandleTacticalPanelSwitch( );
	DisableTacticalTeamPanelButtons( FALSE );

}


static INT16 GetOffsetLandHeight(INT32 sGridNo)
{
	INT16 sTileHeight;

	sTileHeight = gpWorldLevelData[ sGridNo ].sHeight;

	return( sTileHeight );
}


static INT16 GetModifiedOffsetLandHeight(INT32 sGridNo)
{
	INT16 sTileHeight;
	INT16 sModifiedTileHeight;

	sTileHeight = gpWorldLevelData[ sGridNo ].sHeight;

	sModifiedTileHeight = ( ( ( sTileHeight / 80 ) - 1 ) * 80 );

	if ( sModifiedTileHeight < 0 )
	{
		sModifiedTileHeight = 0;
	}

	return( sModifiedTileHeight );
}


void RenderOverheadMap( INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS, BOOLEAN fFromMapUtility )
{
	INT8				bXOddFlag = 0;
	INT16				sModifiedHeight = 0;
	INT16				sAnchorPosX_M, sAnchorPosY_M;
	INT16				sAnchorPosX_S, sAnchorPosY_S;
	INT16				sTempPosX_M, sTempPosY_M;
	INT16				sTempPosX_S, sTempPosY_S;
	BOOLEAN			fEndRenderRow = FALSE, fEndRenderCol = FALSE;
	UINT32			usTileIndex;
	INT16				sX, sY;
	LEVELNODE		*pNode;
	SMALL_TILE_DB	*pTile;
	INT16				sHeight;
	INT16				sX1, sX2, sY1, sY2;

	if (!gfOverheadMapDirty) return;
	// Black out.......
	ColorFillVideoSurfaceArea( FRAME_BUFFER, sStartPointX_S, sStartPointY_S, sEndXS,	sEndYS, 0 );

	InvalidateScreen( );
	gfOverheadMapDirty = FALSE;

	// Begin Render Loop
	sAnchorPosX_M = sStartPointX_M;
	sAnchorPosY_M = sStartPointY_M;
	sAnchorPosX_S = sStartPointX_S;
	sAnchorPosY_S = sStartPointY_S;

	{ SGPVSurface::Lock l(FRAME_BUFFER);
		UINT16* const pDestBuf         = l.Buffer<UINT16>();
		UINT32  const uiDestPitchBYTES = l.Pitch();

		do
		{

			fEndRenderRow = FALSE;
			sTempPosX_M = sAnchorPosX_M;
			sTempPosY_M = sAnchorPosY_M;
			sTempPosX_S = sAnchorPosX_S;
			sTempPosY_S = sAnchorPosY_S;

			if(bXOddFlag > 0)
				sTempPosX_S += 4;


			do
			{

				usTileIndex=FASTMAPROWCOLTOPOS( sTempPosY_M, sTempPosX_M );

				if ( usTileIndex < GRIDSIZE )
				{
					sHeight=( GetOffsetLandHeight(usTileIndex) /5);

					pNode = gpWorldLevelData[ usTileIndex ].pLandStart;
					while( pNode != NULL )
					{

						pTile = &( gSmTileDB[ pNode->usIndex ] );

						sX = sTempPosX_S;
						sY = sTempPosY_S - sHeight + ( gsRenderHeight / 5 );

						pTile->vo->CurrentShade(pNode->ubShadeLevel);

						Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex);

						pNode = pNode->pPrevNode;
					}

				}

				sTempPosX_S += 8;
				sTempPosX_M ++;
				sTempPosY_M --;

				if ( sTempPosX_S >= sEndXS )
				{
					fEndRenderRow = TRUE;
				}

			} while( !fEndRenderRow );

			if ( bXOddFlag > 0 )
			{
				sAnchorPosY_M ++;
			}
			else
			{
				sAnchorPosX_M ++;
			}


			bXOddFlag = !bXOddFlag;
			sAnchorPosY_S += 2;

			if ( sAnchorPosY_S >= sEndYS )
			{
				fEndRenderCol = TRUE;
			}

		}
		while( !fEndRenderCol );



		// Begin Render Loop
		sAnchorPosX_M = sStartPointX_M;
		sAnchorPosY_M = sStartPointY_M;
		sAnchorPosX_S = sStartPointX_S;
		sAnchorPosY_S = sStartPointY_S;
		bXOddFlag = 0;
		fEndRenderRow = FALSE;
		fEndRenderCol = FALSE;

		do
		{

			fEndRenderRow = FALSE;
			sTempPosX_M = sAnchorPosX_M;
			sTempPosY_M = sAnchorPosY_M;
			sTempPosX_S = sAnchorPosX_S;
			sTempPosY_S = sAnchorPosY_S;

			if(bXOddFlag > 0)
				sTempPosX_S += 4;


			do
			{

				usTileIndex=FASTMAPROWCOLTOPOS( sTempPosY_M, sTempPosX_M );

				if ( usTileIndex < GRIDSIZE )
				{
					sHeight=( GetOffsetLandHeight(usTileIndex) /5);
					sModifiedHeight = ( GetModifiedOffsetLandHeight( usTileIndex ) / 5 );

					pNode = gpWorldLevelData[ usTileIndex ].pObjectHead;
					while( pNode != NULL )
					{

						if ( pNode->usIndex < NUMBEROFTILES )
						{
							// Don't render itempools!
							if ( !( pNode->uiFlags & LEVELNODE_ITEM ) )
							{
								pTile = &( gSmTileDB[ pNode->usIndex ] );

								sX = sTempPosX_S;
								sY = sTempPosY_S;

								if( gTileDatabase[ pNode->usIndex ].uiFlags & IGNORE_WORLD_HEIGHT )
								{
									sY -= sModifiedHeight;
								}
								else
								{
									sY -= sHeight;
								}

								sY += ( gsRenderHeight / 5 );

								pTile->vo->CurrentShade(pNode->ubShadeLevel);

								// RENDER!
								Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex);
							}
						}

						pNode = pNode->pNext;
					}


					pNode = gpWorldLevelData[ usTileIndex ].pShadowHead;
					while( pNode != NULL )
					{
						if ( pNode->usIndex < NUMBEROFTILES )
						{
							pTile = &( gSmTileDB[ pNode->usIndex ] );
							sX = sTempPosX_S;
							sY = sTempPosY_S - sHeight;

							sY += ( gsRenderHeight / 5 );

							pTile->vo->CurrentShade(pNode->ubShadeLevel);

							// RENDER!
							Blt8BPPDataTo16BPPBufferShadow(pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex);
						}

						pNode = pNode->pNext;
					}

					pNode = gpWorldLevelData[ usTileIndex ].pStructHead;

					while( pNode != NULL )
					{
						if ( pNode->usIndex < NUMBEROFTILES )
						{
							// Don't render itempools!
							if ( !( pNode->uiFlags & LEVELNODE_ITEM ) )
							{
								pTile = &( gSmTileDB[ pNode->usIndex ] );

								sX = sTempPosX_S;
								sY = sTempPosY_S - (gTileDatabase[ pNode->usIndex ].sOffsetHeight/5);

								if( gTileDatabase[ pNode->usIndex ].uiFlags & IGNORE_WORLD_HEIGHT )
								{
									sY -= sModifiedHeight;
								}
								else
								{
									sY -= sHeight;
								}

								sY += ( gsRenderHeight / 5 );

								pTile->vo->CurrentShade(pNode->ubShadeLevel);

								// RENDER!
								Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex);
							}
						}

						pNode = pNode->pNext;
					}
				}

				sTempPosX_S += 8;
				sTempPosX_M ++;
				sTempPosY_M --;

				if ( sTempPosX_S >= sEndXS )
				{
					fEndRenderRow = TRUE;
				}

			} while( !fEndRenderRow );

			if ( bXOddFlag > 0 )
			{
				sAnchorPosY_M ++;
			}
			else
			{
				sAnchorPosX_M ++;
			}


			bXOddFlag = !bXOddFlag;
			sAnchorPosY_S += 2;

			if ( sAnchorPosY_S >= sEndYS )
			{
				fEndRenderCol = TRUE;
			}

		}
		while( !fEndRenderCol );

		//if ( !fFromMapUtility && !gfEditMode )
		{

			// ROOF RENDR LOOP
			// Begin Render Loop
			sAnchorPosX_M = sStartPointX_M;
			sAnchorPosY_M = sStartPointY_M;
			sAnchorPosX_S = sStartPointX_S;
			sAnchorPosY_S = sStartPointY_S;
			bXOddFlag = 0;
			fEndRenderRow = FALSE;
			fEndRenderCol = FALSE;

			do
			{

				fEndRenderRow = FALSE;
				sTempPosX_M = sAnchorPosX_M;
				sTempPosY_M = sAnchorPosY_M;
				sTempPosX_S = sAnchorPosX_S;
				sTempPosY_S = sAnchorPosY_S;

				if(bXOddFlag > 0)
					sTempPosX_S += 4;

				do
				{
					usTileIndex=FASTMAPROWCOLTOPOS( sTempPosY_M, sTempPosX_M );

					if ( usTileIndex < GRIDSIZE )
					{
						sHeight=( GetOffsetLandHeight(usTileIndex) /5);

						pNode = gpWorldLevelData[ usTileIndex ].pRoofHead;
						while( pNode != NULL )
						{
							if ( pNode->usIndex < NUMBEROFTILES )
							{
								if ( !( pNode->uiFlags & LEVELNODE_HIDDEN ) )
								{
									pTile = &( gSmTileDB[ pNode->usIndex ] );

									sX = sTempPosX_S;
									sY = sTempPosY_S - (gTileDatabase[ pNode->usIndex ].sOffsetHeight/5) -sHeight;

									sY -= ( WALL_HEIGHT/5 );

									sY += ( gsRenderHeight / 5 );

									pTile->vo->CurrentShade(pNode->ubShadeLevel);

									// RENDER!
									Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex);
								}
							}
							pNode = pNode->pNext;
						}
					}

					sTempPosX_S += 8;
					sTempPosX_M ++;
					sTempPosY_M --;

					if ( sTempPosX_S >= sEndXS )
					{
						fEndRenderRow = TRUE;
					}

				} while( !fEndRenderRow );

				if ( bXOddFlag > 0 )
				{
					sAnchorPosY_M ++;
				}
				else
				{
					sAnchorPosX_M ++;
				}


				bXOddFlag = !bXOddFlag;
				sAnchorPosY_S += 2;

				if ( sAnchorPosY_S >= sEndYS )
				{
					fEndRenderCol = TRUE;
				}

			}
			while( !fEndRenderCol );
		}
	}

	// OK, blacken out edges of smaller maps...
	if ( gMapInformation.ubRestrictedScrollID != 0 )
	{
		CalculateRestrictedMapCoords( NORTH, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS );
		ColorFillVideoSurfaceArea( FRAME_BUFFER, sX1, sY1, sX2, sY2, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

		CalculateRestrictedMapCoords( WEST, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS );
		ColorFillVideoSurfaceArea( FRAME_BUFFER, sX1, sY1, sX2, sY2, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

		CalculateRestrictedMapCoords( SOUTH, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS );
		ColorFillVideoSurfaceArea( FRAME_BUFFER, sX1, sY1, sX2, sY2, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

		CalculateRestrictedMapCoords( EAST, &sX1, &sY1, &sX2, &sY2, sEndXS, sEndYS );
		ColorFillVideoSurfaceArea( FRAME_BUFFER, sX1, sY1, sX2, sY2, Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

	}

	if ( !fFromMapUtility )
	{
		// Render border!
		BltVideoObject(FRAME_BUFFER, uiOVERMAP, 0, 0, 0);
	}

	// Update the save buffer
	BltVideoSurface(guiSAVEBUFFER, FRAME_BUFFER, 0, 0, NULL);
}


static void GetOverheadScreenXYFromGridNo(INT16 sGridNo, INT16* psScreenX, INT16* psScreenY);


static void RenderOverheadOverlays(void)
{
	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf         = l.Buffer<UINT16>();
	UINT32  const uiDestPitchBYTES = l.Pitch();

	// Soldier overlay
	SGPVObject*        const marker = uiPERSONS;
	SOLDIERTYPE const* const sel    = gfTacticalPlacementGUIActive || !gfRadarCurrentGuyFlash ? 0 : GetSelectedMan();
	UINT16             const end    = gfTacticalPlacementGUIActive ? gTacticalStatus.Team[OUR_TEAM].bLastID : MAX_NUM_SOLDIERS;
	for (UINT32 i = 0; i < end; ++i)
	{
		const SOLDIERTYPE* const s = GetMan(i);
		if (!s->bActive || !s->bInSector) continue;

		if (!gfTacticalPlacementGUIActive && s->bLastRenderVisibleValue == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_MERCS))
		{
			continue;
		}

		if (s->sGridNo == NOWHERE) continue;

		//Soldier is here.  Calculate his screen position based on his current gridno.
		INT16 sX;
		INT16 sY;
		GetOverheadScreenXYFromGridNo(s->sGridNo, &sX, &sY);
		//Now, draw his "doll"

		//adjust for position.
		sX += 2;
		sY -= 5;

		sY -= GetOffsetLandHeight(s->sGridNo) / 5;
		sY -= s->sHeightAdjustment / 5; // Adjust for height
		sY += gsRenderHeight / 5;

		UINT32 const shade =
			s == sel             ? 2 :
			s->sHeightAdjustment ? 1 : // On roof
			0;
		marker->CurrentShade(shade);

#ifdef JA2EDITOR
		if (gfEditMode && gpSelected && gpSelected->pSoldier == s)
		{ //editor:  show the selected edited merc as the yellow one.
			Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, marker, sX, sY, 0);
		}
		else
#endif
		{
			UINT16 const region =
				!gfTacticalPlacementGUIActive                                ? s->bTeam :
				s->uiStatusFlags & SOLDIER_VEHICLE                           ? 9        :
				s == gpTacticalPlacementSelectedSoldier                      ? 7        :
				s == gpTacticalPlacementHilightedSoldier && s->uiStatusFlags ? 8        :
				s->bTeam;
			Blt8BPPDataTo16BPPBufferTransparent(pDestBuf, uiDestPitchBYTES, marker, sX, sY, region);
			ETRLEObject const& e = *marker->SubregionProperties(region);
			RegisterBackgroundRect(BGND_FLAG_SINGLE, sX + e.sOffsetX, sY + e.sOffsetY, e.usWidth, e.usHeight);
		}
	}

	// Items overlay
	if (!gfTacticalPlacementGUIActive)
	{
		CFOR_ALL_WORLD_ITEMS(wi)
		{
			if (wi->bVisible != VISIBLE && !(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS))
			{
				continue;
			}

			INT16 sX;
			INT16 sY;
			GetOverheadScreenXYFromGridNo(wi->sGridNo, &sX, &sY);

			//adjust for position.
			sY += 6;
			sY -= GetOffsetLandHeight(wi->sGridNo) / 5;
			sY += gsRenderHeight / 5;

			UINT32 col;
			if (gsOveritemPoolGridNo == wi->sGridNo)
			{
				col = FROMRGB(255, 0, 0);
			}
			else if (gfRadarCurrentGuyFlash)
			{
				col = FROMRGB(0, 0, 0);
			}
			else switch (wi->bVisible)
			{
				case HIDDEN_ITEM:      col = FROMRGB(  0,   0, 255); break;
				case BURIED:           col = FROMRGB(255,   0,   0); break;
				case HIDDEN_IN_OBJECT: col = FROMRGB(  0,   0, 255); break;
				case INVISIBLE:        col = FROMRGB(  0, 255,   0); break;
				case VISIBLE:          col = FROMRGB(255, 255, 255); break;
				default:               abort();
			}
			PixelDraw(FALSE, sX, sY, Get16BPPColor(col), pDestBuf);
			InvalidateRegion(sX, sY, sX + 1, sY + 1);
		}
	}
}


static void ClickOverheadRegionCallback(MOUSE_REGION* reg, INT32 reason)
{
	INT16  sWorldScreenX, sWorldScreenY;

	if( gfTacticalPlacementGUIActive )
	{
		HandleTacticalPlacementClicksInOverheadMap(reason);
		return;
	}

	if (reason & MSYS_CALLBACK_REASON_LBUTTON_UP)
	{
		sWorldScreenX = ( gusMouseXPos - gsStartRestrictedX ) * 5;
		sWorldScreenY = ( gusMouseYPos - gsStartRestrictedY ) * 5;

		// Get new proposed center location.
		const GridNo pos = GetMapPosFromAbsoluteScreenXY(sWorldScreenX, sWorldScreenY);
		INT16 cell_x;
		INT16 cell_y;
		ConvertGridNoToCenterCellXY(pos, &cell_x, &cell_y);

		SetRenderCenter(cell_x, cell_y);

		KillOverheadMap();
	}
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		KillOverheadMap();
	}
}


static void GetOverheadScreenXYFromGridNo(INT16 sGridNo, INT16* psScreenX, INT16* psScreenY)
{
	GetAbsoluteScreenXYFromMapPos(sGridNo, psScreenX, psScreenY);
	*psScreenX /= 5;
	*psScreenY /= 5;

	*psScreenX += gsStartRestrictedX + 5;
	*psScreenY += gsStartRestrictedY + 5;

	//Subtract the height....
  //*psScreenY -= gpWorldLevelData[ sGridNo ].sHeight / 5;
}


static GridNo InternalGetOverheadMouseGridNo(const INT dy)
{
	if (!(OverheadRegion.uiFlags & MSYS_MOUSE_IN_AREA)) return NOWHERE;

	// ATE: Adjust alogrithm values a tad to reflect map positioning
	INT16 const sWorldScreenX = (gusMouseXPos - gsStartRestrictedX -  5) * 5;
	INT16       sWorldScreenY = (gusMouseYPos - gsStartRestrictedY + dy) * 5;

	// Get new proposed center location.
	const GridNo grid_no = GetMapPosFromAbsoluteScreenXY(sWorldScreenX, sWorldScreenY);

	// Adjust for height.....
	sWorldScreenY = sWorldScreenY + gpWorldLevelData[grid_no].sHeight;

	return GetMapPosFromAbsoluteScreenXY(sWorldScreenX, sWorldScreenY);
}


GridNo GetOverheadMouseGridNo(void)
{
	return InternalGetOverheadMouseGridNo(-8);
}


static GridNo GetOverheadMouseGridNoForFullSoldiersGridNo(void)
{
	return InternalGetOverheadMouseGridNo(0);
}


void CalculateRestrictedMapCoords( INT8 bDirection, INT16 *psX1, INT16 *psY1, INT16 *psX2, INT16 *psY2, INT16 sEndXS, INT16 sEndYS )
{
	switch( bDirection )
	{
		case NORTH:

			*psX1 = 0;
			*psX2 = sEndXS;
			*psY1 = 0;
			*psY2 = ( abs( NORMAL_MAP_SCREEN_TY - gsTLY ) / 5 );
			break;

		case WEST:

			*psX1 = 0;
			*psX2 = ( abs( -NORMAL_MAP_SCREEN_X - gsTLX ) / 5 );
			*psY1 = 0;
			*psY2 = sEndYS;
			break;

		case SOUTH:

			*psX1 = 0;
			*psX2 = sEndXS;
			*psY1 = ( NORMAL_MAP_SCREEN_HEIGHT - abs( NORMAL_MAP_SCREEN_BY - gsBLY ) ) / 5;
			*psY2 = sEndYS;
			break;

		case EAST:

			*psX1 = ( NORMAL_MAP_SCREEN_WIDTH - abs( NORMAL_MAP_SCREEN_X - gsTRX ) ) / 5;
			*psX2 = sEndXS;
			*psY1 = 0;
			*psY2 = sEndYS;
			break;

	}
}


static void CopyOverheadDBShadetablesFromTileset(void)
{
	// Loop through tileset
	for (size_t i = 0; i < NUMBEROFTILETYPES; ++i)
	{
		gSmTileSurf[i].vo->ShareShadetables(gTileSurfaceArray[i]->vo);
	}
}


void TrashOverheadMap(void)
{
	if (gubSmTileNum == TILESET_INVALID) return;
	gubSmTileNum = TILESET_INVALID;

	for (SMALL_TILE_SURF* i = gSmTileSurf; i != endof(gSmTileSurf); ++i)
	{
		DeleteVideoObject(i->vo);
	}
}
