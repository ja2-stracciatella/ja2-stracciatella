#include "Font.h"
#include "Handle_Items.h"
#include "Isometric_Utils.h"
#include "Local.h"
#include "HImage.h"
#include "Soldier_Find.h"
#include "TileDat.h"
#include "Timer_Control.h"
#include "SysUtil.h"
#include "Overhead.h"
#include "MouseSystem.h"
#include "Button_System.h"
#include "Interface.h"
#include "VObject.h"
#include "VSurface.h"
#include "Input.h"
#include "Handle_UI.h"
#include "Animation_Data.h"
#include "RenderWorld.h"
#include "Cursors.h"
#include "Radar_Screen.h"
#include "WorldMan.h"
#include "Font_Control.h"
#include "Render_Dirty.h"
#include "Interface_Cursors.h"
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

		if (!(guiTacticalInterfaceFlags & INTERFACE_SHOPKEEP_INTERFACE))
		{
			RenderButtons( );
		}
	}
}


static void RenderPanel(void)
{
	switch (gsCurInterfacePanel)
	{
		case SM_PANEL:   RenderSMPanel(&fInterfacePanelDirty);  break;
		case TEAM_PANEL: RenderTEAMPanel(fInterfacePanelDirty); break;
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

	RenderPanel();

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
	RenderButtons();
	RenderPanel();
	HandleAutoFaces();
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
		const SOLDIERTYPE* const sel = GetSelectedMan();
		if (sel != NULL && sel->sGridNo != gsUIHandleShowMoveGridLocation)
		{
			UINT16 idx;
			if (gfUIHandleShowMoveGrid == 2)
			{
				idx = FIRSTPOINTERS4;
			}
			else if (sel->bStealthMode)
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
	INT16											 iLeft, iRight, iTop, iBottom;
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
	SGPVSurface::Lock l(FRAME_BUFFER);
	UINT16* const pDestBuf = l.Buffer<UINT16>();
	SetClippingRegionAndImageWidth(l.Pitch(), 0, 0, gsVIEWPORT_END_X, gsVIEWPORT_WINDOW_END_Y );

	usLineColor = Get16BPPColor( guiColors[ iFlashColor ] );

	if ( ( iRight - iLeft ) > 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iRight, iTop, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iLeft, iTop, iRight + 1, iTop + 1);
	}
	else if ( ( iRight - iLeft ) < 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iRight, iTop, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iRight, iTop, iLeft + 1, iTop + 1);
	}

	if ( ( iRight - iLeft ) > 0 )
	{
		LineDraw( TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iLeft, iBottom, iRight + 1, iBottom + 1);
	}
	else if ( ( iRight - iLeft ) < 0 )
	{
		LineDraw( TRUE, iLeft, iBottom, iRight, iBottom, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iRight, iBottom, iLeft + 1, iBottom + 1);
	}

	if ( ( iBottom - iTop ) > 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iLeft, iTop, iLeft + 1, iBottom);
	}
	else if ( ( iBottom - iTop ) < 0 )
	{
		LineDraw( TRUE, iLeft, iTop, iLeft, iBottom, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iLeft, iBottom, iLeft + 1, iTop);
	}

	if ( ( iBottom - iTop ) > 0 )
	{
		LineDraw( TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iRight, iTop, iRight + 1, iBottom);
	}
	else if ( ( iBottom - iTop ) < 0 )
	{
		LineDraw( TRUE, iRight, iTop, iRight, iBottom, usLineColor, pDestBuf );
		RegisterBackgroundRectSingleFilled(iRight, iBottom, iRight + 1, iTop);
	}
}


static void EndViewportOverlays(void);
static void StartViewportOverlays(void);


void RenderTopmostTacticalInterface()
{
	static SGPVObject* uiBogTarget = 0;

	if (gfRerenderInterfaceFromHelpText)
	{
		fInterfacePanelDirty = DIRTYLEVEL2;
		RenderPanel();
		gfRerenderInterfaceFromHelpText = FALSE;
	}

	if (guiTacticalInterfaceFlags & INTERFACE_MAPSCREEN)
	{
		// If we want to rederaw whole screen, dirty all buttons!
		if (fInterfacePanelDirty == DIRTYLEVEL2) MarkButtonsDirty();
		RenderButtons();
		return;
	}

	if (InItemStackPopup())
	{
		RenderItemStackPopup(fInterfacePanelDirty == DIRTYLEVEL2);
	}

	if (InKeyRingPopup() && !InItemDescriptionBox())
	{
		RenderKeyRingPopup(fInterfacePanelDirty == DIRTYLEVEL2);
	}

	if (gfInMovementMenu)
	{
		RenderMovementMenu();
	}

	// if IN PLAN MODE AND WE HAVE TARGETS, draw black targets!
	if (InUIPlanMode())
	{
		// Zero out any planned soldiers
		CFOR_ALL_PLANNING_SOLDIERS(s)
		{
			if (s->sPlannedTargetX == -1) continue;

			if (!GridNoOnScreen(MAPROWCOLTOPOS(s->sPlannedTargetY / CELL_Y_SIZE, s->sPlannedTargetX / CELL_X_SIZE))) continue;

			// GET SCREEN COORDINATES
			INT16 const sOffsetX = (s->sPlannedTargetX - gsRenderCenterX);
			INT16 const sOffsetY = (s->sPlannedTargetY - gsRenderCenterY);

			INT16 sTempX_S;
			INT16 sTempY_S;
			FromCellToScreenCoordinates(sOffsetX, sOffsetY, &sTempX_S, &sTempY_S);

			INT16 sX = (gsVIEWPORT_END_X - gsVIEWPORT_START_X) / 2 + sTempX_S;
			INT16 sY = (gsVIEWPORT_END_Y - gsVIEWPORT_START_Y) / 2 + sTempY_S;

			// Adjust for offset position on screen
			sX -= gsRenderWorldOffsetX;
			sY -= gsRenderWorldOffsetY;

			sX -= 10;
			sY -= 10;

			// Blit bogus target
			if (!uiBogTarget)
			{
				uiBogTarget = AddVideoObjectFromFile("CURSORS/targblak.sti");
			}

			BltVideoObject(FRAME_BUFFER, uiBogTarget, 0, sX, sY);
			InvalidateRegion(sX, sY, sX + 20, sY + 20);
		}
	}

#ifdef JA2TESTVERSION
	if (gUIDeadlockedSoldier != NOBODY)
	{
		SetFontAttributes(LARGEFONT1, FONT_MCOLOR_WHITE);
		GDirtyPrintF(0, 300, L"OPPONENT %d DEADLOCKED - 'Q' TO DEBUG, <ALT><ENTER> END OPP TURN", gUIDeadlockedSoldier);
	}
#endif

	// Syncronize for upcoming soldier counters
	SYNCTIMECOUNTER();

	// Setup system for video overlay (text and blitting) Sets clipping rects, etc
	StartViewportOverlays();

	RenderTopmostFlashingItems();
  RenderTopmostMultiPurposeLocator();
	RenderAccumulatedBurstLocations();

	FOR_ALL_MERCS(i)
	{
		SOLDIERTYPE* const s = *i;
		DrawSelectedUIAboveGuy(s);

		if (!s->fDisplayDamage)    continue;
		if (s->sGridNo == NOWHERE) continue;
		if (s->bVisible == -1)     continue;

		INT16 sMercScreenX;
		INT16 sMercScreenY;
		GetSoldierTRUEScreenPos(s, &sMercScreenX, &sMercScreenY);

		INT16 x = sMercScreenX + s->sDamageX;
		INT16 y = sMercScreenY + s->sDamageY;
		if (s->ubBodyType == QUEENMONSTER)
		{
			x += 25;
			y += 10;
		}
		else
		{
			x += 2 * 30 / 3;
			y += -5;

			if (y < gsVIEWPORT_WINDOW_START_Y)
			{
				y = sMercScreenY - s->sBoundingBoxOffsetY;
			}
		}

		SetFontAttributes(TINYFONT1, FONT_MCOLOR_WHITE);
		GDirtyPrintF(x, y, L"-%d", s->sDamage);
	}

	// FOR THE MOST PART, DISABLE INTERFACE STUFF WHEN IT'S ENEMY'S TURN
	if (gTacticalStatus.ubCurrentTeam == gbPlayerNum)
	{
		RenderArrows();
	}

	EndViewportOverlays();
	RenderRubberBanding();

	if (!gfInItemPickupMenu && !gpItemPointer)
	{
		HandleAnyMercInSquadHasCompatibleStuff(NULL);
	}

	// CHECK IF OUR CURSOR IS OVER AN INV POOL
	GridNo       const usMapPos = GetMouseMapPos();
	SOLDIERTYPE* const sel      = GetSelectedMan();
	if (usMapPos != NOWHERE && gfUIOverItemPoolGridNo != NOWHERE && sel)
	{
		// Check if we are over an item pool
		INT8             level     = sel->bLevel;
		ITEM_POOL const* item_pool = GetItemPool(gfUIOverItemPoolGridNo, level);
		if (!item_pool)
		{
			// ATE: Allow to see list if a different level....
			level     = (level == 0 ? 1 : 0);
			item_pool = GetItemPool(gfUIOverItemPoolGridNo, level);
		}

		if (item_pool)
		{
			STRUCTURE* pStructure;
			INT16      sIntTileGridNo;
			INT16 const sActionGridNo =
				ConditionalGetCurInteractiveTileGridNoAndStructure(&sIntTileGridNo, &pStructure, FALSE) ?
					sIntTileGridNo : usMapPos;

			INT8 const bZLevel = GetZLevelOfItemPoolGivenStructure(sActionGridNo, level, pStructure);
			if (AnyItemsVisibleOnLevel(item_pool, bZLevel))
			{
				DrawItemPoolList(item_pool, bZLevel, gusMouseXPos, gusMouseYPos);
				// ATE: If over items, remove locator....
				RemoveFlashItemSlot(item_pool);
			}
		}
	}

	switch (gCurrentUIMode)
	{
		case GETTINGITEM_MODE:
			SetItemPickupMenuDirty(DIRTYLEVEL2);
			RenderItemPickupMenu();
			break;

		case OPENDOOR_MENU_MODE:
			RenderOpenDoorMenu();
			break;
	}

	if (gfInTalkPanel) RenderTalkingMenu();

	if (gfInSectorExitMenu) RenderSectorExitMenu();

	if (fRenderRadarScreen)
	{
	  RenderClock();
	  RenderTownIDString();
		CreateMouseRegionForPauseOfClock();
	}
	else
	{
		RemoveMouseRegionForPauseOfClock();
	}

	// If we want to rederaw whole screen, dirty all buttons!
	if (fInterfacePanelDirty == DIRTYLEVEL2) MarkButtonsDirty();

	RenderButtons();
	RenderPausedGameBox();

	MarkAllBoxesAsAltered();

	HandleShowingOfTacticalInterfaceFastHelpText();
	HandleShadingOfLinesForAssignmentMenus();
	DetermineBoxPositions();
	DisplayBoxes(FRAME_BUFFER);
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
