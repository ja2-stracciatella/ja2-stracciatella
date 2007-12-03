#include "Font.h"
#include "Isometric_Utils.h"
#include "Local.h"
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


SGPRect				gOldClippingRect, gOldDirtyClippingRect;

UINT32		guiTacticalInterfaceFlags;

UINT16		gusUICurIntTileEffectIndex;
INT16			gsUICurIntTileEffectGridNo;
UINT8			gsUICurIntTileOldShade;

BOOLEAN		gfRerenderInterfaceFromHelpText = FALSE;


BOOLEAN	gfPausedTacticalRenderInterfaceFlags = FALSE;
BOOLEAN	gfPausedTacticalRenderFlags					 = FALSE;


void SetTacticalInterfaceFlags( UINT32 uiFlags )
{
	guiTacticalInterfaceFlags = uiFlags;
}

void HandleTacticalPanelSwitch( )
{
	if ( gfSwitchPanel )
	{
		SetCurrentInterfacePanel( gbNewPanel );
		SetCurrentTacticalPanelCurrentMerc(gNewPanelSoldier);
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
		SetRenderFlags(gfPausedTacticalRenderFlags);
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

	if (g_ui_message_overlay != NULL)
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

	switch (gfUIHandleSelection)
	{
		UINT16 idx;

		case NONSELECTED_GUY_SELECTION: idx = GOODRING1;      goto add_node;
		//case SELECTED_GUY_SELECTION:    idx = SELRING1;       goto add_node;
		case SELECTED_GUY_SELECTION:    idx = FIRSTPOINTERS2; goto add_node;
		case ENEMY_GUY_SELECTION:       idx = FIRSTPOINTERS2; goto add_node;
		default:                        break;

add_node:;
			LEVELNODE* n;
			if (gsSelectedLevel > 0)
			{
				n = AddRoofToHead(gsSelectedGridNo, idx);
			}
			else
			{
				n = AddObjectToHead(gsSelectedGridNo, idx);
			}
			n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
			n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
			break;
	}

	if ( gfUIHandleShowMoveGrid )
	{
		if ( gusSelectedSoldier != NOBODY )
		{
			const SOLDIERTYPE* const s = GetSelectedMan();
			if (s->sGridNo != gsUIHandleShowMoveGridLocation)
			{
				UINT16 idx;
				if (gfUIHandleShowMoveGrid == 2)
				{
					idx = FIRSTPOINTERS4;
				}
				else if (s->bStealthMode)
				{
					idx = FIRSTPOINTERS9;
				}
				else
				{
					idx = FIRSTPOINTERS2;
				}
				LEVELNODE* const n = AddTopmostToHead(gsUIHandleShowMoveGridLocation, GetSnapCursorIndex(idx));
				n->ubShadeLevel        = DEFAULT_SHADE_LEVEL;
				n->ubNaturalShadeLevel = DEFAULT_SHADE_LEVEL;
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
	INT32											 iBack = NO_BGND_RECT;
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
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iLeft, iTop, iRight + 1, iTop + 1);
	}
	else if ( ( iRight - iLeft ) < 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iRight, iTop, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iRight, iTop, iLeft + 1, iTop + 1);
	}

	if (iBack != NO_BGND_RECT)
	{
		SetBackgroundRectFilled( iBack );
	}


	iBack = NO_BGND_RECT;

	if ( ( iRight - iLeft ) > 0 )
	{
		LineDraw( TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iLeft, iBottom, iRight + 1, iBottom + 1);
	}
	else if ( ( iRight - iLeft ) < 0 )
	{
		LineDraw( TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iRight, iBottom, iLeft + 1, iBottom + 1);
	}

	if (iBack != NO_BGND_RECT)
	{
		SetBackgroundRectFilled( iBack );
	}



	iBack = NO_BGND_RECT;

	if ( ( iBottom - iTop ) > 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iLeft, iTop, iLeft + 1, iBottom);
	}
	else if ( ( iBottom - iTop ) < 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iLeft, iBottom, iLeft + 1, iTop);
	}

	if (iBack != NO_BGND_RECT)
	{
		SetBackgroundRectFilled( iBack );
	}


	iBack = NO_BGND_RECT;

	if ( ( iBottom - iTop ) > 0 )
	{
		LineDraw( TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iRight, iTop, iRight + 1, iBottom);
	}
	else if ( ( iBottom - iTop ) < 0 )
	{
		LineDraw( TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf );
		iBack = RegisterBackgroundRect(BGND_FLAG_SINGLE, iRight, iBottom, iRight + 1, iTop);
	}

	if (iBack != NO_BGND_RECT)
	{
		SetBackgroundRectFilled( iBack );
	}



	UnLockVideoSurface( FRAME_BUFFER );
}


static void EndViewportOverlays(void);
static void StartViewportOverlays(void);


void RenderTopmostTacticalInterface( )
{
	static UINT32				uiBogTarget = 0;
	INT16			sX, sY;
	INT16			sOffsetX, sOffsetY, sTempY_S, sTempX_S;
	UINT16						usMapPos;

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
		CFOR_ALL_PLANNING_SOLDIERS(s)
		{
			if (s->sPlannedTargetX != -1)
			{
				// Blit bogus target
				if (uiBogTarget == 0)
				{
					//Loadup cursor!
					uiBogTarget = AddVideoObjectFromFile("CURSORS/targblak.sti");
				}

				if (GridNoOnScreen((INT16)MAPROWCOLTOPOS(s->sPlannedTargetY / CELL_Y_SIZE, s->sPlannedTargetX / CELL_X_SIZE)))
				{
					// GET SCREEN COORDINATES
					sOffsetX = (s->sPlannedTargetX - gsRenderCenterX);
					sOffsetY = (s->sPlannedTargetY - gsRenderCenterY);

					FromCellToScreenCoordinates(sOffsetX, sOffsetY, &sTempX_S, &sTempY_S);

					sX = (gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2 + sTempX_S;
					sY = (gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2 + sTempY_S;

					// Adjust for offset position on screen
					sX -= gsRenderWorldOffsetX;
					sY -= gsRenderWorldOffsetY;

					sX -= 10;
					sY -= 10;

					BltVideoObjectFromIndex(FRAME_BUFFER, uiBogTarget, 0, sX, sY);
					InvalidateRegion(sX, sY, sX + 20, sY + 20);
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
	for (UINT32 cnt = 0; cnt < guiNumMercSlots; ++cnt)
	{
		SOLDIERTYPE* const pSoldier = MercSlots[cnt];
		if (pSoldier != NULL )
		{
			if (pSoldier != gSelectedGuy) DrawSelectedUIAboveGuy(pSoldier);

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

	if (gusSelectedSoldier != NOBODY) DrawSelectedUIAboveGuy(GetSelectedMan());
	if (gSelectedGuy       != NULL)   DrawSelectedUIAboveGuy(gSelectedGuy);

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
				const ITEM_POOL* pItemPool = GetItemPool(gfUIOverItemPoolGridNo, pSoldier->bLevel);
				if (pItemPool != NULL)
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
						DrawItemPoolList(pItemPool, gfUIOverItemPoolGridNo, bZLevel, gusMouseXPos, gusMouseYPos);

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
					const ITEM_POOL* pItemPool = GetItemPool(gfUIOverItemPoolGridNo, bCheckLevel);
				  if (pItemPool != NULL)
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
							DrawItemPoolList(pItemPool, gfUIOverItemPoolGridNo, bZLevel, gusMouseXPos, gusMouseYPos);

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
	  RenderClock();
	  RenderTownIDString( );
		CreateMouseRegionForPauseOfClock();
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
	ClippingRect.iLeft   = INTERFACE_START_X;
	ClippingRect.iTop    = gsVIEWPORT_WINDOW_START_Y;
	ClippingRect.iRight  = SCREEN_WIDTH;
	ClippingRect.iBottom = gsVIEWPORT_WINDOW_END_Y;

	// Set values for dirty rect clipping rect
	gDirtyClipRect.iLeft   = INTERFACE_START_X;
	gDirtyClipRect.iTop    = gsVIEWPORT_WINDOW_START_Y;
	gDirtyClipRect.iRight  = SCREEN_WIDTH;
	gDirtyClipRect.iBottom = gsVIEWPORT_WINDOW_END_Y;

	SaveFontSettings( );
	SetFontDestBuffer(FRAME_BUFFER, 0, gsVIEWPORT_WINDOW_START_Y, SCREEN_WIDTH, gsVIEWPORT_WINDOW_END_Y);
}


static void EndViewportOverlays(void)
{
	// Reset clipping rect
	ClippingRect = gOldClippingRect;
	gDirtyClipRect = gOldDirtyClippingRect;
	RestoreFontSettings( );

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

	if (g_ui_message_overlay != NULL)
	{
		RemoveVideoOverlay(g_ui_message_overlay);
		g_ui_message_overlay = NULL;
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
