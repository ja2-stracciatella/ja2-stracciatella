#include "Font.h"
#include "SGP.h"
#include "Gameloop.h"
#include "HImage.h"
#include "Soldier_Find.h"
#include "VObject.h"
#include "SysUtil.h"
#include "Overhead.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Interface.h"
#include "VSurface.h"
#include "WCheck.h"
#include "Input.h"
#include "Handle_UI.h"
#include "Animation_Control.h"
#include "Animation_Data.h"
#include "RenderWorld.h"
#include "Sys_Globals.h"
#include "Cursors.h"
#include "Radar_Screen.h"
#include "WorldMan.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "Utilities.h"
#include "Interface_Cursors.h"
#include "Lighting.h"
#include "Interface_Panels.h"
#include "PathAI.h"
#include "VObject_Blitters.h"
#include "Faces.h"
#include "Handle_UI_Plan.h"
#include "Interface_Control.h"
#include "Interface_Items.h"
#include "Interface_Dialogue.h"
#include "Interactive_Tiles.h"
#include "Game_Clock.h"
#include "Strategic_Exit_GUI.h"
#include "PopUpBox.h"
#include "Assignments.h"
#include "Spread_Burst.h"
#include "Squads.h"
#include "Line.h"
#include "Map_Screen_Interface.h"
#include "Civ_Quotes.h"
#include "Video.h"


#define				CLOCK_X								554
#define				CLOCK_Y								459
SGPRect				gOldClippingRect, gOldDirtyClippingRect;

UINT32		guiTacticalInterfaceFlags;

UINT16		gusUICurIntTileEffectIndex;
INT16			gsUICurIntTileEffectGridNo;
UINT8			gsUICurIntTileOldShade;

BOOLEAN		gfRerenderInterfaceFromHelpText = FALSE;

MOUSE_REGION		gLockPanelOverlayRegion;

extern void			RenderTownIDString( );
extern BOOLEAN	gfUIOverItemPool;
extern INT16		gfUIOverItemPoolGridNo;
extern BOOLEAN	gfInMovementMenu;
extern BOOLEAN	gfInItemPickupMenu;
extern BOOLEAN	gfInOpenDoorMenu;
extern UINT32		guiUIMessageTimeDelay;


BOOLEAN	gfPausedTacticalRenderInterfaceFlags = FALSE;
BOOLEAN	gfPausedTacticalRenderFlags					 = FALSE;

// which assignment menu can be shown?
extern void DetermineWhichAssignmentMenusCanBeShown( void );
extern void HandleAnyMercInSquadHasCompatibleStuff( UINT8 ubSquad, OBJECTTYPE *pObject, BOOLEAN fReset );


void SetTacticalInterfaceFlags( UINT32 uiFlags )
{
	guiTacticalInterfaceFlags = uiFlags;
}

void HandleTacticalPanelSwitch( )
{
	if ( gfSwitchPanel )
	{
		SetCurrentInterfacePanel( gbNewPanel );
		SetCurrentTacticalPanelCurrentMerc( gubNewPanelParam );
		gfSwitchPanel = FALSE;

		if ( (!guiTacticalInterfaceFlags & INTERFACE_NORENDERBUTTONS ) && !( guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE ) )
		{
			RenderButtons( );
		}
	}
}


static void HandlePausedTacticalRender(void);


void RenderTacticalInterface( )
{

	// handle paused render of tactical
	HandlePausedTacticalRender( );

	if ( !(guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		HandleFlashingItems( );

    HandleMultiPurposeLocator( );

	}

	// Handle degrading new items...
	DegradeNewlyAddedItems( );

	switch( gsCurInterfacePanel )
	{
		case SM_PANEL:
			RenderSMPanel( &fInterfacePanelDirty );
			break;

		case TEAM_PANEL:
			RenderTEAMPanel( fInterfacePanelDirty );
			break;
	}

	// Handle faces
	if( !(guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE ) )
		HandleAutoFaces( );

}


/* handle paused render of tactical panel, if flag set, OR it in with tactical
 * render flags then reset */
static void HandlePausedTacticalRender(void)
{

	// for a one frame paused render of tactical
	if( gfPausedTacticalRenderFlags )
	{
		gRenderFlags|=gfPausedTacticalRenderFlags;
		gfPausedTacticalRenderFlags = FALSE;
	}

	if ( gfPausedTacticalRenderInterfaceFlags )
	{
		fInterfacePanelDirty = gfPausedTacticalRenderInterfaceFlags;
		gfPausedTacticalRenderInterfaceFlags = FALSE;
	}
}


void RenderTacticalInterfaceWhileScrolling( )
{
	RenderButtons( );

	switch( gsCurInterfacePanel )
	{
		case SM_PANEL:
			RenderSMPanel( &fInterfacePanelDirty );
			break;

		case TEAM_PANEL:
			RenderTEAMPanel( fInterfacePanelDirty );
			break;
	}

	// Handle faces
	HandleAutoFaces( );

}

void SetUpInterface( )
{
	LEVELNODE									 *pIntTile;

	if ( ( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		return;
	}

	DrawUICursor( );

	SetupPhysicsTrajectoryUI( );


	if ( giUIMessageOverlay != -1 )
	{
		if ( ( GetJA2Clock( ) - guiUIMessageTime ) > guiUIMessageTimeDelay )
		{
			EndUIMessage( );
		}
	}

	if ( gCurrentUIMode == OPENDOOR_MENU_MODE )
	{
		HandleOpenDoorMenu( );
	}

	HandleTalkingMenu( );

	if ( gCurrentUIMode == EXITSECTORMENU_MODE )
	{
		HandleSectorExitMenu( );
	}

	// FOR THE MOST PART - SHUTDOWN INTERFACE WHEN IT'S THE ENEMY'S TURN
	if ( gTacticalStatus.ubCurrentTeam != gbPlayerNum )
	{
		return;
	}

	HandleInterfaceBackgrounds( );


	if ( gfUIHandleSelection == NONSELECTED_GUY_SELECTION )
	{
		if ( gsSelectedLevel > 0 )
		{
			AddRoofToHead( gsSelectedGridNo, GOODRING1 );
			gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

		}
		else
		{
			AddObjectToHead( gsSelectedGridNo, GOODRING1 );
			gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
		}

	}

	if ( gfUIHandleSelection == SELECTED_GUY_SELECTION )
	{
		if ( gsSelectedLevel > 0 )
		{
			//AddRoofToHead( gsSelectedGridNo, SELRING1 );
			AddRoofToHead( gsSelectedGridNo, FIRSTPOINTERS2 );
			gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

		}
		else
		{
			//AddObjectToHead( gsSelectedGridNo, SELRING1 );
			AddObjectToHead( gsSelectedGridNo, FIRSTPOINTERS2 );
			gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
		}

	}


	if ( gfUIHandleSelection == ENEMY_GUY_SELECTION )
	{
		if ( gsSelectedLevel > 0 )
		{
			AddRoofToHead( gsSelectedGridNo, FIRSTPOINTERS2 );
			gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			gpWorldLevelData[gsSelectedGridNo].pRoofHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;

		}
		else
		{
			AddObjectToHead( gsSelectedGridNo, FIRSTPOINTERS2 );
			gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
			gpWorldLevelData[gsSelectedGridNo].pObjectHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
		}

	}

	if ( gfUIHandleShowMoveGrid )
	{
		if ( gusSelectedSoldier != NOBODY )
		{
			if ( MercPtrs[ gusSelectedSoldier ]->sGridNo != gsUIHandleShowMoveGridLocation )
			{
				if ( gfUIHandleShowMoveGrid == 2 )
				{
					AddTopmostToHead( gsUIHandleShowMoveGridLocation, GetSnapCursorIndex( FIRSTPOINTERS4 ) );
					gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
					gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
				}
				else
				{
					if ( MercPtrs[ gusSelectedSoldier ]->bStealthMode )
					{
						AddTopmostToHead( gsUIHandleShowMoveGridLocation, GetSnapCursorIndex( FIRSTPOINTERS9 ) );
						gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					}
					else
					{
						AddTopmostToHead( gsUIHandleShowMoveGridLocation, GetSnapCursorIndex( FIRSTPOINTERS2 ) );
						gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubShadeLevel=DEFAULT_SHADE_LEVEL;
						gpWorldLevelData[gsUIHandleShowMoveGridLocation].pTopmostHead->ubNaturalShadeLevel=DEFAULT_SHADE_LEVEL;
					}
				}
			}
		}
	}

	// Check if we are over an interactive tile...
	if ( gfUIShowCurIntTile )
	{
		pIntTile = GetCurInteractiveTileGridNo( &gsUICurIntTileEffectGridNo );

		if ( pIntTile != NULL )
		{
			gusUICurIntTileEffectIndex = pIntTile->usIndex;

			// Shade green
			gsUICurIntTileOldShade = pIntTile->ubShadeLevel;
			pIntTile->ubShadeLevel = 0;
			pIntTile->uiFlags |= LEVELNODE_DYNAMIC;
		}
	}
}


void ResetInterface( )
{
	LEVELNODE *pNode;

	if ( ( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		return;
	}

	// find out if we need to show any menus
	DetermineWhichAssignmentMenusCanBeShown();
	CreateDestroyAssignmentPopUpBoxes( );

	HideUICursor( );

	ResetPhysicsTrajectoryUI( );

	if ( gfUIHandleSelection )
	{
		if ( gsSelectedLevel > 0 )
		{
			RemoveRoof( gsSelectedGridNo, GOODRING1 );
			RemoveRoof( gsSelectedGridNo, FIRSTPOINTERS2 );
		}
		else
		{
			RemoveObject( gsSelectedGridNo, FIRSTPOINTERS2 );
			RemoveObject( gsSelectedGridNo, GOODRING1 );

		}
	}

	if ( gfUIHandleShowMoveGrid )
	{
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS4	);
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS9	);
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS2 );
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS13 );
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS15 );
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS19 );
		RemoveTopmost( gsUIHandleShowMoveGridLocation, FIRSTPOINTERS20 );
	}

	if ( fInterfacePanelDirty )
	{
		fInterfacePanelDirty = FALSE;
	}


	// Reset int tile cursor stuff
	if ( gfUIShowCurIntTile )
	{
		if ( gsUICurIntTileEffectGridNo != NOWHERE )
		{
			//Find our tile!
			pNode = gpWorldLevelData[ gsUICurIntTileEffectGridNo].pStructHead;

			while( pNode != NULL )
			{
				if ( pNode->usIndex == gusUICurIntTileEffectIndex )
				{
					pNode->ubShadeLevel = gsUICurIntTileOldShade;
					pNode->uiFlags &= (~LEVELNODE_DYNAMIC);
					break;
				}

				pNode = pNode->pNext;
			}
		}
	}

}


UINT32	guiColors[ 12 ] =
{
	FROMRGB( 198, 163, 0 ),
	FROMRGB( 185, 150, 0 ),
	FROMRGB( 172, 136, 0 ),
	FROMRGB( 159, 123, 0 ),
	FROMRGB( 146, 110, 0 ),
	FROMRGB( 133, 96, 0 ),
	FROMRGB( 120, 83, 0 ),
	FROMRGB( 133, 96, 0 ),
	FROMRGB( 146, 110, 0 ),
	FROMRGB( 159, 123, 0 ),
	FROMRGB( 172, 136, 0 ),
	FROMRGB( 185, 150, 0 )
};


static void RenderRubberBanding(void)
{
	UINT16										 usLineColor;
	UINT32										 uiDestPitchBYTES;
	UINT8											 *pDestBuf;
	INT16											 iLeft, iRight, iTop, iBottom;
	INT32											 iBack = -1;
	static INT32							 iFlashColor = 0;
	static INT32							 uiTimeOfLastUpdate = 0;

	if ( !gRubberBandActive )
		return;

	iLeft		= (INT16)gRubberBandRect.iLeft;
	iRight	= (INT16)gRubberBandRect.iRight;
	iTop		= (INT16)gRubberBandRect.iTop;
	iBottom	= (INT16)gRubberBandRect.iBottom;

	if ( iLeft == iRight && iTop == iBottom )
	{
		return;
	}

	if ( ( GetJA2Clock( ) - uiTimeOfLastUpdate ) > 60 )
	{
		uiTimeOfLastUpdate = GetJA2Clock( );
		iFlashColor++;

		if ( iFlashColor == 12 )
		{
			iFlashColor = 0;
		}
	}

	// Draw rectangle.....
	pDestBuf = LockVideoSurface( FRAME_BUFFER, &uiDestPitchBYTES );
	SetClippingRegionAndImageWidth( uiDestPitchBYTES, 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y );

	usLineColor = Get16BPPColor( guiColors[ iFlashColor ] );

	if ( ( iRight - iLeft ) > 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iRight, iTop, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iLeft, iTop, (INT16)( iRight + 1 ), (INT16)( iTop + 1 ) );
	}
	else if ( ( iRight - iLeft ) < 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iRight, iTop, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iRight, iTop, (INT16)( iLeft + 1 ), (INT16)( iTop + 1 ) );
	}

	if ( iBack != -1 )
	{
		SetBackgroundRectFilled( iBack );
	}


	iBack = -1;

	if ( ( iRight - iLeft ) > 0 )
	{
		LineDraw( TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iLeft, iBottom, (INT16)( iRight +1 ), (INT16)( iBottom + 1 ) );
	}
	else if ( ( iRight - iLeft ) < 0 )
	{
		LineDraw( TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iRight, iBottom, (INT16)( iLeft + 1 ), (INT16)( iBottom + 1 ) );
	}

	if ( iBack != -1 )
	{
		SetBackgroundRectFilled( iBack );
	}



	iBack = -1;

	if ( ( iBottom - iTop ) > 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iLeft, iTop, (INT16)( iLeft +1 ), iBottom );
	}
	else if ( ( iBottom - iTop ) < 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iLeft, iBottom, (INT16)( iLeft + 1 ), iTop );
	}

	if ( iBack != -1 )
	{
		SetBackgroundRectFilled( iBack );
	}


	iBack = -1;

	if ( ( iBottom - iTop ) > 0 )
	{
		LineDraw( TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iRight, iTop, (INT16)( iRight +1 ), iBottom );
	}
	else if ( ( iBottom - iTop ) < 0 )
	{
		LineDraw( TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect( BGND_FLAG_SINGLE, NULL, iRight, iBottom, (INT16)( iRight + 1 ), iTop );
	}

	if ( iBack != -1 )
	{
		SetBackgroundRectFilled( iBack );
	}



	UnLockVideoSurface( FRAME_BUFFER );
}


static void EndViewportOverlays(void);
static void StartViewportOverlays(void);


void RenderTopmostTacticalInterface( )
{
	UINT32											 cnt;
	static UINT32				uiBogTarget = 0;
	INT16			sX, sY;
	INT16			sOffsetX, sOffsetY, sTempY_S, sTempX_S;
	UINT16						usMapPos;
	ITEM_POOL					*pItemPool;


	if ( gfRerenderInterfaceFromHelpText == TRUE )
	{
		fInterfacePanelDirty = DIRTYLEVEL2;

		switch( gsCurInterfacePanel )
		{
			case SM_PANEL:
				RenderSMPanel( &fInterfacePanelDirty );
				break;

			case TEAM_PANEL:
				RenderTEAMPanel( fInterfacePanelDirty );
				break;
		}
		gfRerenderInterfaceFromHelpText = FALSE;
	}

	if ( ( guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN ) )
	{
		if ( !(guiTacticalInterfaceFlags & INTERFACE_NORENDERBUTTONS ) )
		{
			// If we want to rederaw whole screen, dirty all buttons!
			if ( fInterfacePanelDirty == DIRTYLEVEL2 )
			{
				MarkButtonsDirty( );
			}


			RenderButtons( );
		}

		return;
	}

	if ( InItemStackPopup( ) )
	{
		if ( fInterfacePanelDirty == DIRTYLEVEL2 )
		{
			RenderItemStackPopup( TRUE );
		}
		else
		{
			RenderItemStackPopup( FALSE );
		}
	}

	if( ( InKeyRingPopup() ) && ( !InItemDescriptionBox( ) ) )
	{
		RenderKeyRingPopup( ( BOOLEAN )( fInterfacePanelDirty == DIRTYLEVEL2 ) );
	}

	if ( gfInMovementMenu )
	{
		RenderMovementMenu( );
	}



	// if IN PLAN MODE AND WE HAVE TARGETS, draw black targets!
	if ( InUIPlanMode( ) )
	{
		// Zero out any planned soldiers
		for( cnt = MAX_NUM_SOLDIERS; cnt < TOTAL_SOLDIERS; cnt++ )
		{
			if ( MercPtrs[ cnt ]->bActive )
			{
				if ( MercPtrs[ cnt ]->sPlannedTargetX != -1 )
				{
					// Blit bogus target
					if ( uiBogTarget == 0 )
					{
						//Loadup cursor!
						uiBogTarget = AddVideoObjectFromFile("CURSORS/targblak.sti");
					}

					if ( GridNoOnScreen( (INT16)MAPROWCOLTOPOS( ( MercPtrs[ cnt ]->sPlannedTargetY/CELL_Y_SIZE), ( MercPtrs[ cnt ]->sPlannedTargetX / CELL_X_SIZE ) ) ) )
					{
						// GET SCREEN COORDINATES
						sOffsetX = (MercPtrs[ cnt ]->sPlannedTargetX - gsRenderCenterX);
						sOffsetY = (MercPtrs[ cnt ]->sPlannedTargetY - gsRenderCenterY);

						FromCellToScreenCoordinates( sOffsetX, sOffsetY, &sTempX_S, &sTempY_S );

						sX = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + sTempX_S;
						sY = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + sTempY_S;

						// Adjust for offset position on screen
						sX -= gsRenderWorldOffsetX;
						sY -= gsRenderWorldOffsetY;

						sX -= 10;
						sY -= 10;

						BltVideoObjectFromIndex(  FRAME_BUFFER, uiBogTarget, 0, sX, sY);
						InvalidateRegion( sX, sY, sX + 20, sY + 20 );
					}
				}
			}
		}
	}



	if ( gfUIInDeadlock )
	{
		SetFont( LARGEFONT1 );
		SetFontBackground( FONT_MCOLOR_BLACK );
		SetFontForeground( FONT_MCOLOR_WHITE );
		gprintfdirty( 0, 300, L"OPPONENT %d DEADLOCKED - 'Q' TO DEBUG, <ALT><ENTER> END OPP TURN", gUIDeadlockedSoldier  );
		mprintf( 0, 300, L"OPPONENT %d DEADLOCKED - 'Q' TO DEBUG, <ALT><ENTER> END OPP TURN", gUIDeadlockedSoldier );

	}


	// Syncronize for upcoming soldier counters
	SYNCTIMECOUNTER( );


	// Setup system for video overlay ( text and blitting ) Sets clipping rects, etc
	StartViewportOverlays( );

	RenderTopmostFlashingItems( );

  RenderTopmostMultiPurposeLocator( );

	RenderAccumulatedBurstLocations( );

	// Loop through all mercs and make go
	for ( cnt = 0; cnt < guiNumMercSlots; cnt++ )
	{
		const SOLDIERTYPE* pSoldier = MercSlots[cnt];

		if (pSoldier != NULL )
		{
			if ( pSoldier->ubID == gsSelectedGuy && gfUIHandleSelectionAboveGuy )
			{

			}
			else
			{
				DrawSelectedUIAboveGuy((UINT16)pSoldier->ubID);
			}

			if ( pSoldier->fDisplayDamage )
			{
				// Display damage

				// Use world coordinates!
				INT16 sMercScreenX, sMercScreenY, sOffsetX, sOffsetY, sDamageX, sDamageY;

				if ( pSoldier->sGridNo != NOWHERE && pSoldier->bVisible != -1 )
				{
					GetSoldierScreenPos( pSoldier, &sMercScreenX, &sMercScreenY );
					GetSoldierAnimOffsets( pSoldier, &sOffsetX, &sOffsetY );

          if ( pSoldier->ubBodyType == QUEENMONSTER )
          {
		        sDamageX = sMercScreenX + pSoldier->sDamageX - pSoldier->sBoundingBoxOffsetX;
		        sDamageY = sMercScreenY + pSoldier->sDamageY - pSoldier->sBoundingBoxOffsetY;

		        sDamageX += 25;
		        sDamageY += 10;
          }
          else
          {
					  sDamageX = pSoldier->sDamageX + (INT16)(sMercScreenX + ( 2 * 30 / 3 )  );
					  sDamageY = pSoldier->sDamageY + (INT16)(sMercScreenY - 5 );

					  sDamageX -= sOffsetX;
					  sDamageY -= sOffsetY;

			      if ( sDamageY < gsVIEWPORT_WINDOW_START_Y )
			      {
				      sDamageY = ( sMercScreenY - sOffsetY );
            }
          }

          SetFont( TINYFONT1 );
					SetFontBackground( FONT_MCOLOR_BLACK );
					SetFontForeground( FONT_MCOLOR_WHITE );

					gprintfdirty( sDamageX, sDamageY, L"-%d", pSoldier->sDamage );
					mprintf( sDamageX, sDamageY, L"-%d", pSoldier->sDamage );
				}
			}
		}

	}

	if ( gusSelectedSoldier != NOBODY )
	{
		DrawSelectedUIAboveGuy( gusSelectedSoldier );
	}

	if ( gfUIHandleSelectionAboveGuy && gsSelectedGuy != NOBODY )
	{
		DrawSelectedUIAboveGuy((UINT16)gsSelectedGuy );
	}

	// FOR THE MOST PART, DISABLE INTERFACE STUFF WHEN IT'S ENEMY'S TURN
	if ( gTacticalStatus.ubCurrentTeam == gbPlayerNum )
	{
		RenderArrows( );
	}

	RenderAimCubeUI( );

	EndViewportOverlays( );

	RenderRubberBanding( );

	if ( !gfInItemPickupMenu && gpItemPointer == NULL )
	{
		HandleAnyMercInSquadHasCompatibleStuff( (INT8) CurrentSquad( ), NULL, TRUE );
	}

	// CHECK IF OUR CURSOR IS OVER AN INV POOL
	if( GetMouseMapPos( &usMapPos) )
	{
		if ( gfUIOverItemPool )
		{
			const SOLDIERTYPE* pSoldier = GetSoldier(gusSelectedSoldier);
			if (pSoldier != NULL)
			{
				// Check if we are over an item pool
				if ( GetItemPool( gfUIOverItemPoolGridNo, &pItemPool, pSoldier->bLevel ) )
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

					bZLevel = GetZLevelOfItemPoolGivenStructure( sActionGridNo, pSoldier->bLevel, pStructure );

					if ( AnyItemsVisibleOnLevel( pItemPool, bZLevel ) )
					{
						DrawItemPoolList( pItemPool, gfUIOverItemPoolGridNo	, ITEMLIST_DISPLAY, bZLevel, gusMouseXPos, gusMouseYPos );

						// ATE: If over items, remove locator....
						RemoveFlashItemSlot( pItemPool );

					}
				}
        else
        {
          INT8 bCheckLevel;

          // ATE: Allow to see list if a different level....
          if ( pSoldier->bLevel == 0 )
          {
            bCheckLevel = 1;
          }
          else
          {
            bCheckLevel = 0;
          }

				  // Check if we are over an item pool
				  if ( GetItemPool( gfUIOverItemPoolGridNo, &pItemPool, bCheckLevel ) )
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

					  bZLevel = GetZLevelOfItemPoolGivenStructure( sActionGridNo, bCheckLevel, pStructure );

					  if ( AnyItemsVisibleOnLevel( pItemPool, bZLevel ) )
					  {
						  DrawItemPoolList( pItemPool, gfUIOverItemPoolGridNo	, ITEMLIST_DISPLAY, bZLevel, gusMouseXPos, gusMouseYPos );

						  // ATE: If over items, remove locator....
						  RemoveFlashItemSlot( pItemPool );

					  }
				  }
        }
			}
		}
	}

	// Check if we should render item selection window
	if ( gCurrentUIMode == GETTINGITEM_MODE )
	{
		SetItemPickupMenuDirty( DIRTYLEVEL2 );
		// Handle item pickup will return true if it's been closed
		RenderItemPickupMenu( );
	}

	// Check if we should render item selection window
		if ( gCurrentUIMode == OPENDOOR_MENU_MODE )
	{
			RenderOpenDoorMenu( );
	}

	if ( gfInTalkPanel )
	{
		SetTalkingMenuDirty( DIRTYLEVEL2 );
		// Handle item pickup will return true if it's been closed
		RenderTalkingMenu( );
	}

	if ( gfInSectorExitMenu )
	{
		RenderSectorExitMenu( );
	}

	if( fRenderRadarScreen == TRUE )
	{
	  // Render clock
	  RenderClock( CLOCK_X, CLOCK_Y );
	  RenderTownIDString( );
		CreateMouseRegionForPauseOfClock( CLOCK_REGION_START_X, CLOCK_REGION_START_Y );
	}
	else
	{
		RemoveMouseRegionForPauseOfClock( );
	}


	if ( !(guiTacticalInterfaceFlags & INTERFACE_NORENDERBUTTONS ) )
	{
		// If we want to rederaw whole screen, dirty all buttons!
		if ( fInterfacePanelDirty == DIRTYLEVEL2 )
		{
			MarkButtonsDirty( );
		}


		RenderButtons( );
		RenderPausedGameBox( );
	}

		// mark all pop ups as dirty
	MarkAllBoxesAsAltered( );

	HandleShowingOfTacticalInterfaceFastHelpText( );
	HandleShadingOfLinesForAssignmentMenus( );
	DetermineBoxPositions( );
	DisplayBoxes( FRAME_BUFFER );
}


static void StartViewportOverlays(void)
{
	// Set Clipping Rect to be the viewscreen
	// Save old one
	gOldClippingRect = ClippingRect;

	// Save old dirty clipping rect
	gOldDirtyClippingRect = ClippingRect;

	// Set bottom clipping value for blitter clipping rect
	ClippingRect.iLeft = INTERFACE_START_X;
	ClippingRect.iTop = gsVIEWPORT_WINDOW_START_Y;
	ClippingRect.iRight = 640;
	ClippingRect.iBottom = gsVIEWPORT_WINDOW_END_Y;

	// Set values for dirty rect clipping rect
	gDirtyClipRect.iLeft = INTERFACE_START_X;
	gDirtyClipRect.iTop = gsVIEWPORT_WINDOW_START_Y;
	gDirtyClipRect.iRight = 640;
	gDirtyClipRect.iBottom = gsVIEWPORT_WINDOW_END_Y;

	SaveFontSettings( );
	SetFontDestBuffer( FRAME_BUFFER, 0, gsVIEWPORT_WINDOW_START_Y, 640, gsVIEWPORT_WINDOW_END_Y, FALSE );

}


static void EndViewportOverlays(void)
{
	// Reset clipping rect
	ClippingRect = gOldClippingRect;
	gDirtyClipRect = gOldDirtyClippingRect;
	RestoreFontSettings( );

}


static void LockTacticalInterface(void)
{
	// OK, check and see if we are not locked, if so
	// 1) create a mouse region over the entrie interface panel
	// 2) set flag for use in tactical to indicate we are locked
	if ( !(guiTacticalInterfaceFlags & INTERFACE_LOCKEDLEVEL1 ) )
	{
		MSYS_DefineRegion( &gLockPanelOverlayRegion, 0, gsVIEWPORT_WINDOW_END_Y ,640, 480, MSYS_PRIORITY_HIGHEST,
							 CURSOR_NORMAL, MSYS_NO_CALLBACK, MSYS_NO_CALLBACK );

		guiTacticalInterfaceFlags |= INTERFACE_LOCKEDLEVEL1;
	}
}


static void UnLockTacticalInterface(void)
{
	if ( (guiTacticalInterfaceFlags & INTERFACE_LOCKEDLEVEL1 ) )
	{
		// Remove region
		MSYS_RemoveRegion( &gLockPanelOverlayRegion);

		guiTacticalInterfaceFlags &= (~INTERFACE_LOCKEDLEVEL1);
	}

}

void EraseInterfaceMenus( BOOLEAN fIgnoreUIUnLock )
{
	// ATE: If we are currently talking, setup this flag so that the
	// automatic handler in handledialogue doesn't adjust the UI setting
	if ( ( gTacticalStatus.uiFlags & ENGAGED_IN_CONV ) && fIgnoreUIUnLock )
	{
		gTacticalStatus.uiFlags |= IGNORE_ENGAGED_IN_CONV_UI_UNLOCK;
	}

	// Remove item pointer if one active
	CancelItemPointer( );

  ShutDownQuoteBoxIfActive( );
	PopDownMovementMenu( );
	PopDownOpenDoorMenu( );
	DeleteTalkingMenu( );
}


BOOLEAN AreWeInAUIMenu( )
{
	if ( gfInMovementMenu || gfInOpenDoorMenu || gfInItemPickupMenu || gfInSectorExitMenu || gfInTalkPanel )
	{
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


void ResetInterfaceAndUI( )
{
	// Erase menus
	EraseInterfaceMenus( FALSE );

	EraseRenderArrows( );

	EndRubberBanding( );

	//ResetMultiSelection( );

	if ( giUIMessageOverlay != -1  )
	{
		RemoveVideoOverlay( giUIMessageOverlay );
		giUIMessageOverlay = -1;
	}

	// Set UI back to movement...
	guiPendingOverrideEvent = M_ON_TERRAIN;
	HandleTacticalUI( );
}

BOOLEAN InterfaceOKForMeanwhilePopup()
{
	if ( gfSwitchPanel )
	{
		return( FALSE );
	}

	return( TRUE );
}
