#ifdef PRECOMPILEDHEADERS
	#include "TileEngine All.h"
#else
	#include "stdio.h"
	#include "SGP.h"
	#include "VObject.h"
	#include "TileDef.h"
	#include "WCheck.h"
	#include "Utilities.h"
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
	#include "Message.h"
	#include "Faces.h"
	#include "Squads.h"
	#include "Interactive_Tiles.h"
	#include "Gameloop.h"
	#include "SysUtil.h"
	#include "Tile_Surface.h"
	#include "Button_System.h"
	#include "Video.h"
	#include <stdlib.h>
	#include "Debug.h"
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



typedef struct
{
	HVOBJECT	vo;
	UINT32		fType;

} SMALL_TILE_SURF;

typedef struct
{
	HVOBJECT	vo;
	UINT16		usSubIndex;
	UINT32		fType;

} SMALL_TILE_DB;


SMALL_TILE_SURF		gSmTileSurf[ NUMBEROFTILETYPES ];
SMALL_TILE_DB			gSmTileDB[ NUMBEROFTILES ];
UINT8							gubSmTileNum   = 0;
BOOLEAN						gfSmTileLoaded = FALSE;
BOOLEAN						gfInOverheadMap = FALSE;
MOUSE_REGION			OverheadRegion;
MOUSE_REGION			OverheadBackgroundRegion;
UINT32						uiOVERMAP;
UINT32						uiPERSONS;
BOOLEAN						gfOverheadMapDirty = FALSE;
extern BOOLEAN		gfRadarCurrentGuyFlash;
INT16							gsStartRestrictedX, gsStartRestrictedY;
BOOLEAN						gfOverItemPool = FALSE;
INT16							gsOveritemPoolGridNo;


void HandleOverheadUI( );
static void ClickOverheadRegionCallback(MOUSE_REGION *reg,INT32 reason);
void MoveOverheadRegionCallback(MOUSE_REGION *reg,INT32 reason);
void DeleteOverheadDB( );
BOOLEAN GetOverheadMouseGridNoForFullSoldiersGridNo( INT16 *psGridNo );


extern BOOLEAN AnyItemsVisibleOnLevel( ITEM_POOL *pItemPool, INT8 bZLevel );
extern void HandleAnyMercInSquadHasCompatibleStuff( UINT8 ubSquad, OBJECTTYPE *pObject, BOOLEAN fReset );


//Isometric utilities (for overhead stuff only)
BOOLEAN GetOverheadMouseGridNo( INT16 *psGridNo );
void GetOverheadScreenXYFromGridNo( INT16 sGridNo, INT16 *psScreenX, INT16 *psScreenY );
void CopyOverheadDBShadetablesFromTileset( );

void RenderOverheadOverlays();

void InitNewOverheadDB( UINT8 ubTilesetID )
{
	UINT32					uiLoop;
	VOBJECT_DESC   VObjectDesc;
	HVOBJECT		   hVObject;
	CHAR8	cFileBPP[128];
	CHAR8	cAdjustedFile[ 128 ];
	UINT32					cnt1, cnt2;
	SMALL_TILE_SURF	s;
	UINT32					NumRegions;
	UINT32					dbSize = 0;


	for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++)
	{

		// Create video object

		// Adjust for BPP
		FilenameForBPP( gTilesets[ ubTilesetID ].TileSurfaceFilenames[ uiLoop ], cFileBPP);

		// Adjust for tileset position
		sprintf( cAdjustedFile, "TILESETS\\%d\\T\\%s", ubTilesetID, cFileBPP );

		VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
		strcpy( VObjectDesc.ImageFile, cAdjustedFile);
		hVObject = CreateVideoObject( &VObjectDesc );

		if ( hVObject == NULL )
		{
			// TRY loading from default directory
			FilenameForBPP( gTilesets[ GENERIC_1 ].TileSurfaceFilenames[ uiLoop ], cFileBPP);
			// Adjust for tileset position
			sprintf( cAdjustedFile, "TILESETS\\0\\T\\%s", cFileBPP );

			// LOAD ONE WE KNOW ABOUT!
			VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
			strcpy( VObjectDesc.ImageFile, cAdjustedFile );
			hVObject = CreateVideoObject( &VObjectDesc );

			if ( hVObject == NULL )
			{
				// LOAD ONE WE KNOW ABOUT!
				VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
				strcpy( VObjectDesc.ImageFile, "TILESETS\\0\\T\\grass.sti");
				hVObject = CreateVideoObject( &VObjectDesc );
			}
		}

		gSmTileSurf[ uiLoop ].vo				= hVObject;
		gSmTileSurf[ uiLoop ].fType		=	uiLoop;
	}

	// NOW LOOP THROUGH AND CREATE DATABASE
	for( cnt1 = 0; cnt1 < NUMBEROFTILETYPES; cnt1++ )
	{
		// Get number of regions
		s = gSmTileSurf[ cnt1 ];

		NumRegions = s.vo->usNumberOfObjects;

		// Check for overflow
		if ( NumRegions > gNumTilesPerType[ cnt1 ] )
		{
				// Cutof
				NumRegions = gNumTilesPerType[ cnt1 ];
		}

		for( cnt2 = 0; cnt2 < NumRegions; cnt2++ )
		{

			gSmTileDB[ dbSize ].vo						= s.vo;
			gSmTileDB[ dbSize ].usSubIndex		= (UINT16)cnt2;
			gSmTileDB[ dbSize ].fType					= cnt1;

			dbSize++;
		}

		// Check if data matches what should be there
		if ( NumRegions < gNumTilesPerType[ cnt1 ] )
		{
				// Do underflows here
				for ( cnt2 = NumRegions; cnt2 < gNumTilesPerType[ cnt1 ]; cnt2++ )
				{
					gSmTileDB[ dbSize ].vo						= s.vo;
					gSmTileDB[ dbSize ].usSubIndex		= 0;
					gSmTileDB[ dbSize ].fType					= cnt1;
					dbSize++;
				}

		}
	}

	gsStartRestrictedX = 0;
	gsStartRestrictedY = 0;

	// Calculate Scale factors because of restricted map scroll regions
	if ( gMapInformation.ubRestrictedScrollID != 0 )
	{
		INT16 sX1, sY1, sX2, sY2;

		CalculateRestrictedMapCoords( NORTH, &sX1, &sY1, &sX2, &gsStartRestrictedY, 640, 320 );
		CalculateRestrictedMapCoords( WEST,	 &sX1, &sY1, &gsStartRestrictedX, &sY2, 640, 320 );
	}

	// Copy over shade tables from main tileset
	CopyOverheadDBShadetablesFromTileset( );


}


void DeleteOverheadDB( )
{
	INT32 cnt;

	for( cnt = 0; cnt < NUMBEROFTILETYPES; cnt++ )
	{
		DeleteVideoObject( gSmTileSurf[ cnt ].vo );
	}

}


BOOLEAN GetClosestItemPool( INT16 sSweetGridNo, ITEM_POOL **ppReturnedItemPool, UINT8 ubRadius, INT8 bLevel )
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;
	ITEM_POOL	*pItemPool;

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
				if ( GetItemPool( sGridNo, &pItemPool, bLevel ) )
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

					if ( uiRange < uiLowestRange )
					{
						(*ppReturnedItemPool) = pItemPool;
						uiLowestRange = uiRange;
						fFound = TRUE;
					}
				}
			}
		}
	}

	return( fFound );
}


BOOLEAN GetClosestMercInOverheadMap( INT16 sSweetGridNo, SOLDIERTYPE **ppReturnedSoldier, UINT8 ubRadius )
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT16		sGridNo;
	INT32		uiRange, uiLowestRange = 999999;
	INT32					leftmost;
	BOOLEAN	fFound = FALSE;

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
        if ( gpWorldLevelData[ sGridNo ].pMercHead != NULL && gpWorldLevelData[ sGridNo ].pMercHead->pSoldier->bVisible != -1 )
				{
					uiRange = GetRangeInCellCoordsFromGridNoDiff( sSweetGridNo, sGridNo );

					if ( uiRange < uiLowestRange )
					{
						(*ppReturnedSoldier) = gpWorldLevelData[ sGridNo ].pMercHead->pSoldier;
						uiLowestRange = uiRange;
						fFound = TRUE;
					}
				}
			}
		}
	}

	return( fFound );
}


void DisplayMercNameInOverhead( SOLDIERTYPE *pSoldier )
{
	INT16		sWorldScreenX, sX;
	INT16		sWorldScreenY, sY;

	// Get Screen position of guy.....
	GetWorldXYAbsoluteScreenXY( ( pSoldier->sX / CELL_X_SIZE ), ( pSoldier->sY / CELL_Y_SIZE ), &sWorldScreenX, &sWorldScreenY );

	sWorldScreenX = gsStartRestrictedX + ( sWorldScreenX / 5 ) + 5;
	sWorldScreenY = gsStartRestrictedY + ( sWorldScreenY / 5 ) + ( pSoldier->sHeightAdjustment / 5 ) + (gpWorldLevelData[ pSoldier->sGridNo ].sHeight/5) - 8;

	sWorldScreenY += ( gsRenderHeight / 5 );

	// Display name
	SetFont( TINYFONT1 );
	SetFontBackground( FONT_MCOLOR_BLACK );
	SetFontForeground( FONT_MCOLOR_WHITE );

	// Center here....
	FindFontCenterCoordinates( sWorldScreenX, sWorldScreenY, (INT16)( 1 ), 1, pSoldier->name, TINYFONT1, &sX, &sY );

	// OK, selected guy is here...
	gprintfdirty( sX, sY, pSoldier->name );
	mprintf( sX, sY, pSoldier->name );
}


void HandleOverheadMap( )
{
	static BOOLEAN fFirst = TRUE;
	SOLDIERTYPE *pSoldier;

	if ( fFirst )
	{
		fFirst = FALSE;
	}

	gfInOverheadMap = TRUE;
	gfOverItemPool	= FALSE;


	// Check tileset numbers
	if ( gubSmTileNum != giCurrentTilesetID )
	{
		// If loaded, unload!
		if ( gfSmTileLoaded )
		{
			//Unload
			DeleteOverheadDB( );

			// Force load
			gfSmTileLoaded = FALSE;
		}
	}

	gubSmTileNum = (UINT8)giCurrentTilesetID;


	if ( gfSmTileLoaded == FALSE )
	{
		// LOAD LAND
		InitNewOverheadDB( gubSmTileNum );
		gfSmTileLoaded = TRUE;
	}

	// restore background rects
	RestoreBackgroundRects( );

	// RENDER!!!!!!!!
	RenderOverheadMap( 0, (WORLD_COLS/2), 0, 0, 640, 320, FALSE );

	HandleTalkingAutoFaces( );

	if( !gfEditMode )
	{
		// CHECK FOR UI
		if( gfTacticalPlacementGUIActive )
		{
			TacticalPlacementHandle();
			if( !gfTacticalPlacementGUIActive )
			{
				return;
			}
		}
		else
		{
			HandleOverheadUI();

			if ( !gfInOverheadMap )
			{
				return;
			}
			RenderTacticalInterface( );
			RenderRadarScreen( );
			RenderClock( CLOCK_X, CLOCK_Y );
			RenderTownIDString( );

    	HandleAutoFaces( );
		}
	}

	if( !gfEditMode && !gfTacticalPlacementGUIActive )
	{
		INT16 usMapPos;
		ITEM_POOL	*pItemPool;

		gfUIHandleSelectionAboveGuy			= FALSE;

		HandleAnyMercInSquadHasCompatibleStuff( (INT8) CurrentSquad( ), NULL, TRUE );

		if ( GetOverheadMouseGridNo( &usMapPos ) )
		{
			// ATE: Find the closest item pool within 5 tiles....
			if ( GetClosestItemPool( usMapPos, &pItemPool, 1, 0 ) )
			{
				STRUCTURE					*pStructure = NULL;
				INT16							sIntTileGridNo;
				INT8							bZLevel = 0;
				INT16							sActionGridNo = usMapPos;

				// Get interactive tile...
				if ( ConditionalGetCurInteractiveTileGridNoAndStructure( &sIntTileGridNo , &pStructure, FALSE ) )
				{
					sActionGridNo = sIntTileGridNo;
				}

				bZLevel = GetZLevelOfItemPoolGivenStructure( sActionGridNo, 0, pStructure );

				if ( AnyItemsVisibleOnLevel( pItemPool, bZLevel ) )
				{
					DrawItemPoolList( pItemPool, usMapPos	, ITEMLIST_DISPLAY, bZLevel, gusMouseXPos, gusMouseYPos );

					gfOverItemPool = TRUE;
					gsOveritemPoolGridNo = pItemPool->sGridNo;
				}
			}

			if ( GetClosestItemPool( usMapPos, &pItemPool, 1, 1 ) )
			{
				INT8							bZLevel = 0;
				INT16							sActionGridNo = usMapPos;

				if ( AnyItemsVisibleOnLevel( pItemPool, bZLevel ) )
				{
					DrawItemPoolList( pItemPool, usMapPos	, ITEMLIST_DISPLAY, bZLevel, gusMouseXPos, (UINT16)( gusMouseYPos - 5 ) );

					gfOverItemPool = TRUE;
					gsOveritemPoolGridNo = pItemPool->sGridNo;
				}
			}

    }

    if ( GetOverheadMouseGridNoForFullSoldiersGridNo( &usMapPos ) )
    {
			if ( GetClosestMercInOverheadMap( usMapPos, &pSoldier, 1 ) )
			{
        if ( pSoldier->bTeam == gbPlayerNum )
        {
    		  gfUIHandleSelectionAboveGuy			= TRUE;
          gsSelectedGuy                   = pSoldier->ubID;
        }

        DisplayMercNameInOverhead( pSoldier );
      }
		}
	}


	RenderOverheadOverlays();
	if( !gfEditMode && !gfTacticalPlacementGUIActive && gusSelectedSoldier != NOBODY )
	{
		pSoldier = MercPtrs[ gusSelectedSoldier ];

    DisplayMercNameInOverhead( pSoldier );
	}

	RenderButtons( );
	StartFrameBufferRender( );

	// save background rects
	SaveBackgroundRects( );

	RenderButtonsFastHelp();

	ExecuteBaseDirtyRectQueue( );
	EndFrameBufferRender( );

	fInterfacePanelDirty = FALSE;

}

BOOLEAN InOverheadMap( )
{
	return( gfInOverheadMap );
}

void GoIntoOverheadMap( )
{
  VOBJECT_DESC    VObjectDesc;
	HVOBJECT				hVObject;

#ifdef JA2DEMO

	if ( gfCaves )
	{
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, pMessageStrings[ MSG_OVERHEAD_MAP_DISABLED ] );
		return;
	}

#endif

	gfInOverheadMap = TRUE;

	MSYS_DefineRegion( &OverheadBackgroundRegion, 0, 0 ,640, 360, MSYS_PRIORITY_HIGH,
						 CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );
	//Add region
	MSYS_AddRegion( &OverheadBackgroundRegion );

	MSYS_DefineRegion( &OverheadRegion, 0, 0 ,gsVIEWPORT_END_X, 320, MSYS_PRIORITY_HIGH,
						 CURSOR_NORMAL, MoveOverheadRegionCallback, ClickOverheadRegionCallback );
	// Add region
	MSYS_AddRegion( &OverheadRegion );

	// LOAD CLOSE ANIM
	VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("INTERFACE\\MAP_BORD.sti", VObjectDesc.ImageFile);
	if( !AddVideoObject( &VObjectDesc, &uiOVERMAP ) )
		AssertMsg(0, "Missing INTERFACE\\MAP_BORD.sti" );


	// LOAD PERSONS
	VObjectDesc.fCreateFlags = VOBJECT_CREATE_FROMFILE;
	FilenameForBPP("INTERFACE\\PERSONS.sti", VObjectDesc.ImageFile);
	if( !AddVideoObject( &VObjectDesc, &uiPERSONS ) )
		AssertMsg(0, "Missing INTERFACE\\PERSONS.sti" );

	// Add shades to persons....
	GetVideoObject( &hVObject, uiPERSONS );
	hVObject->pShades[ 0 ]		  = Create16BPPPaletteShaded( hVObject->pPaletteEntry, 256, 256, 256, FALSE );
	hVObject->pShades[ 1 ]		  = Create16BPPPaletteShaded( hVObject->pPaletteEntry, 310, 310, 310, FALSE );
	hVObject->pShades[ 2 ]		  = Create16BPPPaletteShaded( hVObject->pPaletteEntry, 0, 0, 0, FALSE );

	gfOverheadMapDirty = TRUE;

	if( !gfEditMode )
	{
		// Make sure we are in team panel mode...
		gfSwitchPanel = TRUE;
		gbNewPanel = TEAM_PANEL;
		gubNewPanelParam = (UINT8)gusSelectedSoldier;
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

void HandleOverheadUI( )
{
  InputAtom					InputEvent;
	INT16							sMousePos=0;
	UINT8							ubID;

	// CHECK FOR MOUSE OVER REGIONS...
	if ( GetOverheadMouseGridNo( &sMousePos ) )
	{
		// Look quickly for a soldier....
		ubID =  QuickFindSoldier( sMousePos );

		if ( ubID != NOBODY )
		{
			// OK, selected guy is here...
			//gprintfdirty( gusMouseXPos, gusMouseYPos, MercPtrs[ ubID ]->name );
			//mprintf( gusMouseXPos, gusMouseYPos, MercPtrs[ ubID ]->name );

		}

	}

  while (DequeueEvent(&InputEvent) == TRUE)
  {
    if( ( InputEvent.usEvent == KEY_DOWN ) )
    {
			switch( InputEvent.usParam )
			{
				case( ESC ):
				case( INSERT ):

					KillOverheadMap();
				break;

				case( 'x' ):
					if( ( InputEvent.usKeyState & ALT_DOWN ) )
					{
						HandleShortCutExitState( );
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

	DeleteVideoObjectFromIndex( uiOVERMAP );
	DeleteVideoObjectFromIndex( uiPERSONS );

	HandleTacticalPanelSwitch( );
	DisableTacticalTeamPanelButtons( FALSE );

}


INT16 GetOffsetLandHeight( INT32 sGridNo )
{
	INT16 sTileHeight;

	sTileHeight = gpWorldLevelData[ sGridNo ].sHeight;

	return( sTileHeight );
}

INT16 GetModifiedOffsetLandHeight( INT32 sGridNo )
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
	UINT32			uiDestPitchBYTES;
	UINT8				*pDestBuf;
	LEVELNODE		*pNode;
	SMALL_TILE_DB	*pTile;
	INT16				sHeight;
	HVOBJECT hVObject;
	INT16				sX1, sX2, sY1, sY2;

	// Get video object for persons...
	if ( !fFromMapUtility )
	{
		GetVideoObject( &hVObject, uiPERSONS );
	}

	if ( gfOverheadMapDirty )
	{
		// Black out.......
		ColorFillVideoSurfaceArea( FRAME_BUFFER, sStartPointX_S, sStartPointY_S, sEndXS,	sEndYS, 0 );

		InvalidateScreen( );
		gfOverheadMapDirty = FALSE;

		// Begin Render Loop
		sAnchorPosX_M = sStartPointX_M;
		sAnchorPosY_M = sStartPointY_M;
		sAnchorPosX_S = sStartPointX_S;
		sAnchorPosY_S = sStartPointY_S;


		// Zero out area!
		//ColorFillVideoSurfaceArea( FRAME_BUFFER, 0, 0, (INT16)(640), (INT16)(gsVIEWPORT_WINDOW_END_Y), Get16BPPColor( FROMRGB( 0, 0, 0 ) ) );

		pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );

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

						pTile->vo->pShadeCurrent= gSmTileSurf[ pTile->fType ].vo->pShades[pNode->ubShadeLevel];

						// RENDER!
						//BltVideoObjectFromIndex(  FRAME_BUFFER, SGR1, gSmallTileDatabase[ gpWorldLevelData[ usTileIndex ].pLandHead->usIndex ], sX, sY, VO_BLT_SRCTRANSPARENCY);
						//BltVideoObjectFromIndex(  FRAME_BUFFER, SGR1, 0, sX, sY, VO_BLT_SRCTRANSPARENCY);
						Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex );

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

								pTile->vo->pShadeCurrent= gSmTileSurf[ pTile->fType ].vo->pShades[pNode->ubShadeLevel];

								// RENDER!
								Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex );
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

							pTile->vo->pShadeCurrent= gSmTileSurf[ pTile->fType ].vo->pShades[pNode->ubShadeLevel];

							// RENDER!
							Blt8BPPDataTo16BPPBufferShadow((UINT16*)pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex );
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

								pTile->vo->pShadeCurrent= gSmTileSurf[ pTile->fType ].vo->pShades[pNode->ubShadeLevel];

								// RENDER!
								Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex );
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

								  pTile->vo->pShadeCurrent= gSmTileSurf[ pTile->fType ].vo->pShades[pNode->ubShadeLevel];

								  // RENDER!
								  Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, pTile->vo, sX, sY, pTile->usSubIndex );
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


		UnLockVideoSurface( FRAME_BUFFER );

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
			BltVideoObjectFromIndex( FRAME_BUFFER, uiOVERMAP, 0, 0, 0, VO_BLT_SRCTRANSPARENCY);
		}

    // Update the save buffer
    {
	    UINT32 uiDestPitchBYTES, uiSrcPitchBYTES;
	    UINT8	 *pDestBuf, *pSrcBuf;
	    UINT16 usWidth, usHeight;
	    UINT8  ubBitDepth;

	    // Update saved buffer - do for the viewport size ony!
	    GetCurrentVideoSettings( &usWidth, &usHeight, &ubBitDepth );

	    pSrcBuf = LockVideoSurface(guiRENDERBUFFER, &uiSrcPitchBYTES);
	    pDestBuf = LockVideoSurface(guiSAVEBUFFER, &uiDestPitchBYTES);

	    if(gbPixelDepth==16)
	    {
		    // BLIT HERE
		    Blt16BPPTo16BPP((UINT16 *)pDestBuf, uiDestPitchBYTES,
					    (UINT16 *)pSrcBuf, uiSrcPitchBYTES,
					    0, 0, 0, 0, usWidth, usHeight );
	    }

	    UnLockVideoSurface(guiRENDERBUFFER);
	    UnLockVideoSurface(guiSAVEBUFFER);

    }
	}
}

void RenderOverheadOverlays()
{
	UINT32			uiDestPitchBYTES;
	WORLDITEM		*pWorldItem;
	UINT32				i;
	SOLDIERTYPE	*pSoldier;
	HVOBJECT		hVObject;
	INT16				sX, sY;
	UINT16			end;
	UINT16			usLineColor=0;
	UINT8				*pDestBuf;
	UINT8				ubPassengers = 0;

	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	GetVideoObject( &hVObject, uiPERSONS );


	//SOLDIER OVERLAY
	if( gfTacticalPlacementGUIActive )
	{ //loop through only the player soldiers
		end = gTacticalStatus.Team[ OUR_TEAM ].bLastID;
	}
	else
	{ //loop through all soldiers.
		end = MAX_NUM_SOLDIERS;
	}
	for( i = 0; i < end; i++ )
	{
		//First, check to see if the soldier exists and is in the sector.
		pSoldier = MercPtrs[ i ];
		if( !pSoldier->bActive || !pSoldier->bInSector )
			continue;
		//Soldier is here.  Calculate his screen position based on his current gridno.
		GetOverheadScreenXYFromGridNo( pSoldier->sGridNo, &sX, &sY );
		//Now, draw his "doll"

		//adjust for position.
		sX += 2;
		sY -= 5;
		//sScreenY -= 7;	//height of doll

		if( !gfTacticalPlacementGUIActive && pSoldier->bLastRenderVisibleValue == -1 && !(gTacticalStatus.uiFlags&SHOW_ALL_MERCS) )
		{
			continue;
		}

		if ( pSoldier->sGridNo == NOWHERE )
		{
			continue;
		}

		sY -= ( GetOffsetLandHeight( pSoldier->sGridNo ) /5);

		// Adjust for height...
		sY -= ( pSoldier->sHeightAdjustment / 5 );

		sY += ( gsRenderHeight / 5 );

		// Adjust shade a bit...
		SetObjectShade( hVObject, 0 );

		// If on roof....
		if ( pSoldier->sHeightAdjustment )
		{
			SetObjectShade( hVObject, 1 );
		}

		if ( pSoldier->ubID == gusSelectedSoldier )
		{
			if( gfRadarCurrentGuyFlash && !gfTacticalPlacementGUIActive )
			{
				SetObjectShade( hVObject, 2 );
			}
		}
		#ifdef JA2EDITOR
		if( gfEditMode && gpSelected && gpSelected->pSoldier == pSoldier )
		{ //editor:  show the selected edited merc as the yellow one.
			Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, 0 );
		}
		else
		#endif
		if( !gfTacticalPlacementGUIActive )
		{ //normal
			Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, pSoldier->bTeam );
			RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sX, sY, (INT16)(sX + 3), (INT16)(sY + 9));
		}
		else if( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
		{ //vehicle
			Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, 9 );
			RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (INT16)(sX-6), (INT16)(sY), (INT16)(sX + 9), (INT16)(sY + 10));
		}
		//else if( pSoldier->uiStatusFlags & (SOLDIER_PASSENGER | SOLDIER_DRIVER) )
		//{// //don't draw person, because they are inside the vehicle.
		//	ubPassengers++;
		//}
		else if( gpTacticalPlacementSelectedSoldier == pSoldier )
		{ //tactical placement selected merc
			Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, 7 );
			RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (INT16)(sX-2), (INT16)(sY-2), (INT16)(sX + 5), (INT16)(sY + 11));
		}
		else if( gpTacticalPlacementHilightedSoldier == pSoldier && pSoldier->uiStatusFlags )
		{ //tactical placement hilighted merc
			Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, 8 );
			RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (INT16)(sX-2), (INT16)(sY-2), (INT16)(sX + 5), (INT16)(sY + 11));
		}
		else
		{ //normal
			Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, pSoldier->bTeam );
			RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, sX, sY, (INT16)(sX + 3), (INT16)(sY + 9));
		}
		if( ubPassengers )
		{
			SetFont( SMALLCOMPFONT );
			SetFontForeground( FONT_WHITE );
			gprintfdirty( (INT16)(sX - 3), sY, L"%d", ubPassengers );
			mprintf_buffer( pDestBuf, uiDestPitchBYTES, SMALLCOMPFONT, sX - 3,  sY , L"%d", ubPassengers );
		}
	}

	//ITEMS OVERLAY
	if( !gfTacticalPlacementGUIActive )
	{
		for( i = 0 ; i < guiNumWorldItems; i++  )
		{
			pWorldItem = &gWorldItems[ i ];
			if( !pWorldItem || !pWorldItem->fExists || pWorldItem->bVisible != VISIBLE && !(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS) )
			{
				continue;
			}

			GetOverheadScreenXYFromGridNo( pWorldItem->sGridNo, &sX, &sY );

			//adjust for position.
			//sX += 2;
			sY += 6;
			sY -= ( GetOffsetLandHeight( pWorldItem->sGridNo ) /5);

			sY += ( gsRenderHeight / 5 );


			if ( gfRadarCurrentGuyFlash )
			{
				usLineColor = Get16BPPColor( FROMRGB( 0, 0, 0 ) );
			}
			else switch( pWorldItem->bVisible )
			{
				case HIDDEN_ITEM:				usLineColor = Get16BPPColor( FROMRGB(   0,   0, 255 ) );	break;
				case BURIED:						usLineColor = Get16BPPColor( FROMRGB( 255,   0,   0 ) );	break;
				case HIDDEN_IN_OBJECT:	usLineColor = Get16BPPColor( FROMRGB(   0,   0, 255 ) );	break;
				case INVISIBLE:					usLineColor = Get16BPPColor( FROMRGB(   0, 255,   0 ) );  break;
				case VISIBLE:						usLineColor = Get16BPPColor( FROMRGB( 255, 255, 255 ) );	break;
			}

			if ( gfOverItemPool && gsOveritemPoolGridNo == pWorldItem->sGridNo )
			{
				usLineColor = Get16BPPColor( FROMRGB( 255,   0,   0 ) );
			}

			PixelDraw( FALSE, sX, sY, usLineColor, pDestBuf );

			InvalidateRegion( sX, sY, (INT16)( sX + 1 ), (INT16)( sY + 1 ) );

		}
	}

	UnLockVideoSurface( FRAME_BUFFER );
}

/*//Render the soldiers and items on top of the pristine overhead map.
void RenderOverheadOverlays( INT16 sStartPointX_M, INT16 sStartPointY_M, INT16 sStartPointX_S, INT16 sStartPointY_S, INT16 sEndXS, INT16 sEndYS )
{
	INT8				bXOddFlag = 0;
	INT16				sAnchorPosX_M, sAnchorPosY_M;
	INT16				sAnchorPosX_S, sAnchorPosY_S;
	INT16				sTempPosX_M, sTempPosY_M;
	INT16				sTempPosX_S, sTempPosY_S;
	BOOLEAN			fEndRenderRow = FALSE, fEndRenderCol = FALSE;
	UINT32			usTileIndex;
	INT16				sX, sY;
	UINT32			uiDestPitchBYTES;
	UINT8				*pDestBuf;
	LEVELNODE		*pNode;
	UINT16			usLineColor;
	INT16				sHeight;
	SOLDIERTYPE	*pSoldier;
	HVOBJECT hVObject;
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	// Begin Render Loop
	sAnchorPosX_M = sStartPointX_M;
	sAnchorPosY_M = sStartPointY_M;
	sAnchorPosX_S = sStartPointX_S;
	sAnchorPosY_S = sStartPointY_S;
	bXOddFlag = 0;
	fEndRenderRow = FALSE;
	fEndRenderCol = FALSE;

	GetVideoObject( &hVObject, uiPERSONS );
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
				sHeight=(gpWorldLevelData[usTileIndex].sHeight/5);

				pNode = gpWorldLevelData[ usTileIndex ].pStructHead;
				while( pNode != NULL )
				{
					// Render itempools!
					if ( ( pNode->uiFlags & LEVELNODE_ITEM ) )
					{
						sX = sTempPosX_S;
						sY = sTempPosY_S - sHeight;
						// RENDER!
						if ( pNode->pItemPool->bVisible == -1 && !(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS)  )
						{

						}
						else
						{
							if ( gfRadarCurrentGuyFlash )
							{
								usLineColor = Get16BPPColor( FROMRGB( 0, 0, 0 ) );
							}
							else
							{
								usLineColor = Get16BPPColor( FROMRGB( 255, 255, 255 ) );
							}
							RectangleDraw( TRUE, sX, sY, sX + 1, sY + 1, usLineColor, pDestBuf );

							InvalidateRegion( sX, sY, (INT16)( sX + 2 ), (INT16)( sY + 2 ) );

						}
						break;
					}

					pNode = pNode->pNext;
				}


				pNode = gpWorldLevelData[ usTileIndex ].pMercHead;
				while( pNode != NULL )
				{
						pSoldier = pNode->pSoldier;

						sX = sTempPosX_S;
						sY = sTempPosY_S - sHeight - 8; // 8 height of doll guy

						// RENDER!
						if ( pSoldier->bLastRenderVisibleValue == -1 && !(gTacticalStatus.uiFlags&SHOW_ALL_MERCS)  )
						{

						}
						else
						{
							// Adjust for height...
							sY -= ( pSoldier->sHeightAdjustment / 5 );

							// Adjust shade a bit...
							SetObjectShade( hVObject, 0 );

							// If on roof....
							if ( pSoldier->sHeightAdjustment )
							{
								SetObjectShade( hVObject, 1 );
							}

							if ( pSoldier->ubID == gusSelectedSoldier )
							{
								if( gfRadarCurrentGuyFlash && !gfTacticalPlacementGUIActive )
								{
									SetObjectShade( hVObject, 2 );
								}
							}
							#ifdef JA2EDITOR
							if( gfEditMode && gpSelected && gpSelected->pSoldier == pSoldier )
							{ //editor:  show the selected edited merc as the yellow one.
								Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, 0 );
							}
							else
							#endif
							if( gfTacticalPlacementGUIActive && gpTacticalPlacementSelectedSoldier == pSoldier )
							{ //tactical placement selected merc
								Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, 7 );
							}
							else if( gfTacticalPlacementGUIActive && gpTacticalPlacementHilightedSoldier == pSoldier )
							{ //tactical placement selected merc
								Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, 8 );
							}
							else
							{ //normal
								Blt8BPPDataTo16BPPBufferTransparent((UINT16*)pDestBuf, uiDestPitchBYTES, hVObject, sX, sY, pSoldier->bTeam );
							}
							RegisterBackgroundRect(BGND_FLAG_SINGLE, NULL, (INT16)(sX-2), (INT16)(sY-2), (INT16)(sX + 5), (INT16)(sY + 11));
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
	UnLockVideoSurface( FRAME_BUFFER );
}
*/


static void ClickOverheadRegionCallback(MOUSE_REGION *reg,INT32 reason)
{
	UINT32 uiCellX, uiCellY;
	INT16  sWorldScreenX, sWorldScreenY;

	if( gfTacticalPlacementGUIActive )
	{
		HandleTacticalPlacementClicksInOverheadMap(reason);
		return;
	}

	if (!(reg->uiFlags & BUTTON_ENABLED))
		return;

	if(reason & MSYS_CALLBACK_REASON_LBUTTON_DWN )
	{
		reg->uiFlags |= BUTTON_CLICKED_ON;
	}
	else if(reason & MSYS_CALLBACK_REASON_LBUTTON_UP )
	{
		reg->uiFlags &= (~BUTTON_CLICKED_ON );
		sWorldScreenX = ( gusMouseXPos - gsStartRestrictedX ) * 5;
		sWorldScreenY = ( gusMouseYPos - gsStartRestrictedY ) * 5;

		// Get new proposed center location.
		GetFromAbsoluteScreenXYWorldXY( &uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY );

		SetRenderCenter( (INT16)uiCellX, (INT16)uiCellY );

		KillOverheadMap();

	}
	else if(reason & MSYS_CALLBACK_REASON_RBUTTON_DWN)
	{
		KillOverheadMap();
	}
}


void MoveOverheadRegionCallback(MOUSE_REGION *reg,INT32 reason)
{
}



void GetOverheadScreenXYFromGridNo( INT16 sGridNo, INT16 *psScreenX, INT16 *psScreenY )
{
	GetWorldXYAbsoluteScreenXY( (INT16)(CenterX( sGridNo ) / CELL_X_SIZE ), (INT16)( CenterY( sGridNo ) / CELL_Y_SIZE ), psScreenX, psScreenY );
	*psScreenX /= 5;
	*psScreenY /= 5;

	*psScreenX += 5;
	*psScreenY += 5;

	//Subtract the height....
  //*psScreenY -= gpWorldLevelData[ sGridNo ].sHeight / 5;
}

BOOLEAN GetOverheadMouseGridNo( INT16 *psGridNo )
{
	UINT32 uiCellX, uiCellY;
	INT16  sWorldScreenX, sWorldScreenY;

	if ( ( OverheadRegion.uiFlags & MSYS_MOUSE_IN_AREA ) )
	{

		// ATE: Adjust alogrithm values a tad to reflect map positioning
		sWorldScreenX = gsStartRestrictedX + ( gusMouseXPos - 5 ) * 5;
		sWorldScreenY = gsStartRestrictedY + ( gusMouseYPos - 8 ) * 5;

		// Get new proposed center location.
		GetFromAbsoluteScreenXYWorldXY( &uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY );

		// Get gridNo
		(*psGridNo ) = (INT16)MAPROWCOLTOPOS( ( uiCellY / CELL_Y_SIZE ), ( uiCellX / CELL_X_SIZE ) );

		// Adjust for height.....
		sWorldScreenY =sWorldScreenY + gpWorldLevelData[ (*psGridNo) ].sHeight;

		GetFromAbsoluteScreenXYWorldXY( &uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY );

		// Get gridNo
		(*psGridNo ) = (INT16)MAPROWCOLTOPOS( ( uiCellY / CELL_Y_SIZE ), ( uiCellX / CELL_X_SIZE ) );


		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


BOOLEAN GetOverheadMouseGridNoForFullSoldiersGridNo( INT16 *psGridNo )
{
	UINT32 uiCellX, uiCellY;
	INT16  sWorldScreenX, sWorldScreenY;

	if ( ( OverheadRegion.uiFlags & MSYS_MOUSE_IN_AREA ) )
	{

		// ATE: Adjust alogrithm values a tad to reflect map positioning
		sWorldScreenX = gsStartRestrictedX + ( gusMouseXPos - 5 ) * 5;
		sWorldScreenY = gsStartRestrictedY + ( gusMouseYPos ) * 5;

		// Get new proposed center location.
		GetFromAbsoluteScreenXYWorldXY( &uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY );

		// Get gridNo
		(*psGridNo ) = (INT16)MAPROWCOLTOPOS( ( uiCellY / CELL_Y_SIZE ), ( uiCellX / CELL_X_SIZE ) );

		// Adjust for height.....
		sWorldScreenY =sWorldScreenY + gpWorldLevelData[ (*psGridNo) ].sHeight;

		GetFromAbsoluteScreenXYWorldXY( &uiCellX, &uiCellY, sWorldScreenX, sWorldScreenY );

		// Get gridNo
		(*psGridNo ) = (INT16)MAPROWCOLTOPOS( ( uiCellY / CELL_Y_SIZE ), ( uiCellX / CELL_X_SIZE ) );


		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
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


void CalculateRestrictedScaleFactors( INT16 *pScaleX, INT16 *pScaleY )
{

}


void CopyOverheadDBShadetablesFromTileset( )
{
	UINT32					uiLoop, uiLoop2;
	PTILE_IMAGERY   pTileSurf;


	// Loop through tileset
	for (uiLoop = 0; uiLoop < NUMBEROFTILETYPES; uiLoop++)
	{
		pTileSurf = ( gTileSurfaceArray[ uiLoop ] );

		gSmTileSurf[ uiLoop ].vo->fFlags |= VOBJECT_FLAG_SHADETABLE_SHARED;

		for (uiLoop2 = 0; uiLoop2 < HVOBJECT_SHADE_TABLES; uiLoop2++)
		{
			gSmTileSurf[ uiLoop ].vo->pShades[ uiLoop2 ] = pTileSurf->vo->pShades[ uiLoop2 ];
		}
	}

}

void TrashOverheadMap( )
{
	// If loaded, unload!
	if ( gfSmTileLoaded )
	{
		//Unload
		DeleteOverheadDB( );

		// Force load
		gfSmTileLoaded = FALSE;
	}
}
