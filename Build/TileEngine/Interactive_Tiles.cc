#include "Animation_Data.h"
#include "Cursors.h"
#include "Font_Control.h"
#include "HImage.h"
#include "Isometric_Utils.h"
#include "TileDef.h"
#include "VObject.h"
#include "SysUtil.h"
#include "RenderWorld.h"
#include "Interface.h"
#include "Sound_Control.h"
#include "WorldDef.h"
#include "Interactive_Tiles.h"
#include "Interface_Cursors.h"
#include "WorldMan.h"
#include "Structure.h"
#include "Animation_Control.h"
#include "Points.h"
#include "Overhead.h"
#include "Structure_Wrap.h"
#include "Tile_Animation.h"
#include "Tile_Cache.h"
#include "Handle_Doors.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "Dialogue_Control.h"
#include "Random.h"
#include "English.h"
#include "Handle_Items.h"
#include "Message.h"
#include "Handle_UI.h"
#include "NPC.h"
#include "Explosion_Control.h"
#include "Text.h"
#include "GameSettings.h"
#include "Environment.h"
#include "Debug.h"


#define MAX_INTTILE_STACK 10


struct CUR_INTERACTIVE_TILE
{
	INT16            sGridNo;
	INT16            sTileIndex;
	INT16            sHeighestScreenY;
	BOOLEAN          fFound;
	LEVELNODE const* pFoundNode;
	INT16            sFoundGridNo;
	UINT16           usStructureID;
	BOOLEAN          fStructure;
};


struct INTERACTIVE_TILE_STACK_TYPE
{
	INT8                 bNum;
	CUR_INTERACTIVE_TILE bTiles[MAX_INTTILE_STACK];
	INT8                 bCur;
};


static INTERACTIVE_TILE_STACK_TYPE gCurIntTileStack;
static BOOLEAN                     gfCycleIntTile = FALSE;


static CUR_INTERACTIVE_TILE gCurIntTile;
static BOOLEAN              gfOverIntTile = FALSE;

// Values to determine if we should check or not
static INT16  gsINTOldRenderCenterX = 0;
static INT16  gsINTOldRenderCenterY = 0;
static UINT16 gusINTOldMousePosX    = 0;
static UINT16 gusINTOldMousePosY    = 0;


BOOLEAN StartInteractiveObject( INT16 sGridNo, UINT16 usStructureID, SOLDIERTYPE *pSoldier, UINT8 ubDirection )
{
	STRUCTURE * pStructure;

  // ATE: Patch fix: Don't allow if alreay in animation
  if ( pSoldier->usAnimState == OPEN_STRUCT || pSoldier->usAnimState == OPEN_STRUCT_CROUCHED ||
       pSoldier->usAnimState == BEGIN_OPENSTRUCT || pSoldier->usAnimState == BEGIN_OPENSTRUCT_CROUCHED )
  {
    return( FALSE );
  }

	pStructure = FindStructureByID( sGridNo, usStructureID );
	if (pStructure == NULL)
	{
		return( FALSE );
	}
	if (pStructure->fFlags & STRUCTURE_ANYDOOR)
	{
		// Add soldier event for opening door....
		pSoldier->ubPendingAction = MERC_OPENDOOR;
		pSoldier->uiPendingActionData1 = usStructureID;
		pSoldier->sPendingActionData2  = sGridNo;
		pSoldier->bPendingActionData3  = ubDirection;
		pSoldier->ubPendingActionAnimCount = 0;


	}
	else
	{
		// Add soldier event for opening door....
		pSoldier->ubPendingAction = MERC_OPENSTRUCT;
		pSoldier->uiPendingActionData1 = usStructureID;
		pSoldier->sPendingActionData2  = sGridNo;
		pSoldier->bPendingActionData3  = ubDirection;
		pSoldier->ubPendingActionAnimCount = 0;

	}

	return( TRUE );
}


BOOLEAN CalcInteractiveObjectAPs(INT16 sGridNo, const STRUCTURE* pStructure, INT16* psAPCost, INT16* psBPCost)
{
	if (pStructure == NULL)
	{
		return( FALSE );
	}
	if (pStructure->fFlags & STRUCTURE_ANYDOOR)
	{
		// For doors, if open, we can safely add APs for closing
		// If closed, we do not know what to do yet...
		//if ( pStructure->fFlags & STRUCTURE_OPEN )
		//{
			*psAPCost = AP_OPEN_DOOR;
			*psBPCost = AP_OPEN_DOOR;
		//}
		//else
		//{
		//	*psAPCost = 0;
		//	*psBPCost = 0;
		//}
	}
	else
	{
		*psAPCost = AP_OPEN_DOOR;
		*psBPCost = AP_OPEN_DOOR;
	}

	return( TRUE );
}


BOOLEAN InteractWithInteractiveObject( SOLDIERTYPE *pSoldier, STRUCTURE *pStructure, UINT8 ubDirection )
{
	BOOLEAN	fDoor = FALSE;

	if (pStructure == NULL)
	{
		return( FALSE );
	}

	if (pStructure->fFlags & STRUCTURE_ANYDOOR)
	{
		fDoor = TRUE;
	}

	InteractWithOpenableStruct( pSoldier, pStructure, ubDirection, fDoor );

	return( TRUE );
}


BOOLEAN SoldierHandleInteractiveObject( SOLDIERTYPE *pSoldier )
{
	STRUCTURE			*pStructure;
	UINT16				usStructureID;
	INT16					sGridNo;


	sGridNo					= pSoldier->sPendingActionData2;
	usStructureID		= (UINT16)pSoldier->uiPendingActionData1;

	// HANDLE SOLDIER ACTIONS
	pStructure = FindStructureByID( sGridNo, usStructureID );
	if (pStructure == NULL)
	{
		//DEBUG MSG!
		return( FALSE );
	}

  return( HandleOpenableStruct( pSoldier, sGridNo, pStructure ) );
}

void HandleStructChangeFromGridNo( SOLDIERTYPE *pSoldier, INT16 sGridNo )
{
	STRUCTURE			*pStructure, *pNewStructure;
  BOOLEAN       fDidMissingQuote = FALSE;

	pStructure = FindStructure( sGridNo, STRUCTURE_OPENABLE );

	if ( pStructure == NULL )
	{
#ifdef JA2TESTVERSION
		ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_TESTVERSION, L"ERROR: Told to handle struct that does not exist at %d.", sGridNo );
#endif
		return;
	}

	// Do sound...
	const BOOLEAN closing = (pStructure->fFlags & STRUCTURE_OPEN) != 0;
	PlayLocationJA2Sample(sGridNo, GetStructureOpenSound(pStructure, closing), HIGHVOLUME, 1);

	// ATE: Don't handle switches!
	if ( !( pStructure->fFlags & STRUCTURE_SWITCH ) )
	{
		if ( pSoldier->bTeam == gbPlayerNum )
		{
			if ( sGridNo == BOBBYR_SHIPPING_DEST_GRIDNO && gWorldSectorX == BOBBYR_SHIPPING_DEST_SECTOR_X && gWorldSectorY == BOBBYR_SHIPPING_DEST_SECTOR_Y && gbWorldSectorZ == BOBBYR_SHIPPING_DEST_SECTOR_Z && CheckFact( FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT, 0 ) && !(CheckFact( FACT_PLAYER_FOUND_ITEMS_MISSING, 0) ) )
			{
				SayQuoteFromNearbyMercInSector( BOBBYR_SHIPPING_DEST_GRIDNO, 3, QUOTE_STUFF_MISSING_DRASSEN );
        fDidMissingQuote = TRUE;
			}
		}
		else if ( pSoldier->bTeam == CIV_TEAM )
		{
			if ( pSoldier->ubProfile != NO_PROFILE )
			{
				TriggerNPCWithGivenApproach(pSoldier->ubProfile, APPROACH_DONE_OPEN_STRUCTURE);
			}
		}


		// LOOK for item pool here...
		ITEM_POOL* pItemPool = GetItemPool((INT16)sGridNo, pSoldier->bLevel);
		if (pItemPool != NULL)
		{
			// Update visiblity....
			if ( !( pStructure->fFlags & STRUCTURE_OPEN ) )
			{
				BOOLEAN fDoHumm			= TRUE;
				BOOLEAN	fDoLocators = TRUE;

				if ( pSoldier->bTeam != gbPlayerNum )
				{
					fDoHumm			= FALSE;
					fDoLocators = FALSE;
				}

				// Look for ownership here....
				if (GetWorldItem(pItemPool->iItemIndex)->o.usItem == OWNERSHIP)
				{
					fDoHumm			= FALSE;
					MakeCharacterDialogueEventDoBattleSound(*pSoldier, BATTLE_SOUND_NOTHING, 500);
				}

				// If now open, set visible...
				SetItemsVisibilityOn(sGridNo, pSoldier->bLevel, ANY_VISIBILITY_VALUE, fDoLocators);

				// Display quote!
				//TacticalCharacterDialogue( pSoldier, (UINT16)( QUOTE_SPOTTED_SOMETHING_ONE + Random( 2 ) ) );

				// ATE: Check now many things in pool.....
        if ( !fDidMissingQuote )
        {
				  if ( pItemPool->pNext != NULL )
				  {
					  if ( pItemPool->pNext->pNext != NULL )
					  {
						  fDoHumm = FALSE;
							MakeCharacterDialogueEventDoBattleSound(*pSoldier, BATTLE_SOUND_COOL1, 500);
					  }
				  }

				  if ( fDoHumm )
				  {
						MakeCharacterDialogueEventDoBattleSound(*pSoldier, BATTLE_SOUND_HUMM, 500);
				  }
        }
			}
			else
			{
				SetItemsVisibilityHidden(sGridNo, pSoldier->bLevel);
			}
		}
		else
		{
			if ( !( pStructure->fFlags & STRUCTURE_OPEN ) )
			{
				MakeCharacterDialogueEventDoBattleSound(*pSoldier, BATTLE_SOUND_NOTHING, 500);
			}
		}
	}

	// Deduct points!
	//INT16 sAPCost = 0;
	//INT16 sBPCost = 0;
	// CalcInteractiveObjectAPs( sGridNo, pStructure, &sAPCost, &sBPCost );
	// DeductPoints( pSoldier, sAPCost, sBPCost );



	pNewStructure = SwapStructureForPartner( sGridNo, pStructure );
	if ( pNewStructure != NULL)
	{
		RecompileLocalMovementCosts( sGridNo );
		SetRenderFlags( RENDER_FLAG_FULL );
		if ( pNewStructure->fFlags & STRUCTURE_SWITCH )
		{
			// just turned a switch on!
			ActivateSwitchInGridNo(pSoldier, sGridNo);
		}
	}
}



UINT32 GetInteractiveTileCursor( UINT32 uiOldCursor, BOOLEAN fConfirm )
{
	LEVELNODE	 *pIntNode;
	STRUCTURE	 *pStructure;
	INT16			 sGridNo;

	// OK, first see if we have an in tile...
	pIntNode = GetCurInteractiveTileGridNoAndStructure( &sGridNo, &pStructure );

	if ( pIntNode != NULL && pStructure != NULL )
	{
		if( pStructure->fFlags & STRUCTURE_ANYDOOR )
		{
			SetDoorString( sGridNo );

			if ( fConfirm )
			{
				return( OKHANDCURSOR_UICURSOR );
			}
			else
			{
				return( NORMALHANDCURSOR_UICURSOR );
			}

		}
		else
		{
		  if( pStructure->fFlags & STRUCTURE_SWITCH )
		  {
			  SetIntTileLocationText(gzLateLocalizedString[25]);
      }


			if ( fConfirm )
			{
				return( OKHANDCURSOR_UICURSOR );
			}
			else
			{
				return( NORMALHANDCURSOR_UICURSOR );
			}
		}

	}

	return( uiOldCursor );
}

void SetActionModeDoorCursorText( )
{
	LEVELNODE	 *pIntNode;
	STRUCTURE	 *pStructure;
	INT16			 sGridNo;

  // If we are over a merc, don't
	if (gUIFullTarget != NULL) return;

	// OK, first see if we have an in tile...
	pIntNode = GetCurInteractiveTileGridNoAndStructure( &sGridNo, &pStructure );

	if ( pIntNode != NULL && pStructure != NULL )
	{
		if( pStructure->fFlags & STRUCTURE_ANYDOOR )
		{
			SetDoorString( sGridNo );
		}
	}
}


static void GetLevelNodeScreenRect(LEVELNODE const* const pNode, SGPRect* const pRect, INT16 const sXPos, INT16 const sYPos, INT16 const sGridNo)
{
		INT16 sScreenX, sScreenY;
		INT16 sOffsetX, sOffsetY;
		INT16 sTempX_S, sTempY_S;
		UINT32 usHeight, usWidth;
		TILE_ELEMENT *TileElem;

		// Get 'TRUE' merc position
		sOffsetX = sXPos - gsRenderCenterX;
		sOffsetY = sYPos - gsRenderCenterY;

		FromCellToScreenCoordinates( sOffsetX, sOffsetY, &sTempX_S, &sTempY_S );

		ETRLEObject const* pTrav;
		if ( pNode->uiFlags & LEVELNODE_CACHEDANITILE )
		{
			pTrav = gpTileCache[pNode->pAniTile->sCachedTileID].pImagery->vo->SubregionProperties(pNode->pAniTile->sCurrentFrame);
		}
		else
		{
			TileElem = &(gTileDatabase[pNode->usIndex]);

			//Adjust for current frames and animations....
			if ( TileElem->uiFlags & ANIMATED_TILE)
			{
				Assert( TileElem->pAnimData != NULL );
				TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[TileElem->pAnimData->bCurrentFrame]];
			}
			else if( ( pNode->uiFlags & LEVELNODE_ANIMATION ) )
			{
				if ( pNode->sCurrentFrame != -1  )
				{
					Assert( TileElem->pAnimData != NULL );
					TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[pNode->sCurrentFrame]];
				}
			}

			pTrav = TileElem->hTileSurface->SubregionProperties(TileElem->usRegionIndex);
		}

		sScreenX = ( ( gsVIEWPORT_END_X - gsVIEWPORT_START_X ) /2 ) + (INT16)sTempX_S;
		sScreenY = ( ( gsVIEWPORT_END_Y - gsVIEWPORT_START_Y ) /2 ) + (INT16)sTempY_S;

		// Adjust for offset position on screen
		sScreenX -= gsRenderWorldOffsetX;
		sScreenY -= gsRenderWorldOffsetY;
		sScreenY -=	gpWorldLevelData[ sGridNo ].sHeight;

		// Adjust based on interface level
		if ( gsInterfaceLevel > 0 )
		{
			sScreenY += ROOF_LEVEL_HEIGHT;
		}

		// Adjust for render height
		sScreenY += gsRenderHeight;



		usHeight				= (UINT32)pTrav->usHeight;
		usWidth					= (UINT32)pTrav->usWidth;

		// Add to start position of dest buffer
		sScreenX += ( pTrav->sOffsetX - ( WORLD_TILE_X/2 ) );
		sScreenY += ( pTrav->sOffsetY - ( WORLD_TILE_Y/2 ) );

		// Adjust y offset!
		sScreenY += ( WORLD_TILE_Y/2);

		pRect->iLeft		= sScreenX;
		pRect->iTop			= sScreenY;
		pRect->iBottom	= sScreenY + usHeight;
		pRect->iRight		= sScreenX + usWidth;
}


static BOOLEAN RefineLogicOnStruct(INT16 sGridNo, LEVELNODE const* pNode);
static BOOLEAN RefinePointCollisionOnStruct(INT16 sGridNo, INT16 sTestX, INT16 sTestY, INT16 sSrcX, INT16 sSrcY, LEVELNODE const* pNode);


void LogMouseOverInteractiveTile(INT16 const sGridNo)
{
	// OK, for now, don't allow any interactive tiles on higher interface level!
	if (gsInterfaceLevel > 0) return;

	// Also, don't allow for mercs who are on upper level
	SOLDIERTYPE const* const sel = GetSelectedMan();
	if (sel && sel->bLevel == 1) return;

	// Get World XY From gridno
	INT16 sXMapPos;
	INT16 sYMapPos;
	ConvertGridNoToCellXY(sGridNo, &sXMapPos, &sYMapPos);

	// Set mouse stuff
	INT16 const sScreenX = gusMouseXPos;
	INT16 const sScreenY = gusMouseYPos;

	for (LEVELNODE const* n = gpWorldLevelData[sGridNo].pStructHead; n; n = n->pNext)
	{
		SGPRect aRect;
		GetLevelNodeScreenRect(n, &aRect, sXMapPos, sYMapPos, sGridNo);

		// Make sure we are always on guy if we are on same gridno
		if (!IsPointInScreenRect(sScreenX, sScreenY, &aRect)) continue;

		if (!RefinePointCollisionOnStruct(sGridNo, sScreenX, sScreenY, aRect.iLeft, aRect.iBottom, n)) continue;

		if (!RefineLogicOnStruct(sGridNo, n)) continue;

		gCurIntTile.fFound = TRUE;

		if (gfCycleIntTile) continue;

		// Accumulate them!
		gCurIntTileStack.bTiles[gCurIntTileStack.bNum].pFoundNode   = n;
		gCurIntTileStack.bTiles[gCurIntTileStack.bNum].sFoundGridNo = sGridNo;
		gCurIntTileStack.bNum++;

		// Determine if it's the best one
		if (aRect.iBottom <= gCurIntTile.sHeighestScreenY) continue;

		gCurIntTile.sHeighestScreenY = aRect.iBottom;

		gCurIntTile.pFoundNode   = n;
		gCurIntTile.sFoundGridNo = sGridNo;

		// Set stack current one
		gCurIntTileStack.bCur = gCurIntTileStack.bNum - 1;
	}
}


static LEVELNODE* InternalGetCurInteractiveTile(const BOOLEAN fRejectItemsOnTop)
{
	if (_KeyDown(SHIFT)) return NULL;
	if (!gfOverIntTile)  return NULL;

	LEVELNODE* n = gpWorldLevelData[gCurIntTile.sGridNo].pStructHead;
	for (; n != NULL; n = n->pNext)
	{
		if (n->usIndex != gCurIntTile.sTileIndex) continue;
		if (fRejectItemsOnTop && gCurIntTile.fStructure)
		{
			// get strucuture here...
			STRUCTURE* const s = FindStructureByID(gCurIntTile.sGridNo, gCurIntTile.usStructureID);
			if (s == NULL || s->fFlags & STRUCTURE_HASITEMONTOP) return NULL;
		}
		break;
	}
	return n;
}


LEVELNODE* GetCurInteractiveTile(void)
{
	return InternalGetCurInteractiveTile(TRUE);
}


LEVELNODE* GetCurInteractiveTileGridNo(INT16* const psGridNo)
{
	LEVELNODE* const n = GetCurInteractiveTile();
	*psGridNo = (n != NULL ? gCurIntTile.sGridNo : NOWHERE);
	return n;
}


LEVELNODE* ConditionalGetCurInteractiveTileGridNoAndStructure(INT16* const psGridNo, STRUCTURE** const ppStructure, const BOOLEAN fRejectOnTopItems)
{
	GridNo     g = NOWHERE;
	STRUCTURE* s = NULL;
	LEVELNODE* n = InternalGetCurInteractiveTile(fRejectOnTopItems);
	if (n != NULL)
	{
		g = gCurIntTile.sGridNo;
		if (gCurIntTile.fStructure)
		{
			s = FindStructureByID(g, gCurIntTile.usStructureID);
			if (s == NULL) n = NULL;
		}
	}
	*ppStructure = s;
	*psGridNo    = g;
	return n;
}


LEVELNODE* GetCurInteractiveTileGridNoAndStructure(INT16* const psGridNo, STRUCTURE** const ppStructure)
{
	return ConditionalGetCurInteractiveTileGridNoAndStructure(psGridNo, ppStructure, TRUE);
}


void BeginCurInteractiveTileCheck(void)
{
	gfOverIntTile = FALSE;

	// OK, release our stack, stuff could be different!
	gfCycleIntTile = FALSE;

	// Reset some highest values
	gCurIntTile.sHeighestScreenY = 0;
	gCurIntTile.fFound           = FALSE;

	// Reset stack values
	gCurIntTileStack.bNum = 0;

}

void EndCurInteractiveTileCheck( )
{
	CUR_INTERACTIVE_TILE		*pCurIntTile;

	if ( gCurIntTile.fFound )
	{
		// Set our currently cycled guy.....
		if ( gfCycleIntTile )
		{
			// OK, we're over this cycled node
			pCurIntTile = &( gCurIntTileStack.bTiles[ gCurIntTileStack.bCur ] );
		}
		else
		{
			// OK, we're over this levelnode,
			pCurIntTile = &gCurIntTile;
		}

		gCurIntTile.sGridNo				= pCurIntTile->sFoundGridNo;
		gCurIntTile.sTileIndex    = pCurIntTile->pFoundNode->usIndex;

		if ( pCurIntTile->pFoundNode->pStructureData != NULL )
		{
			gCurIntTile.usStructureID			= pCurIntTile->pFoundNode->pStructureData->usStructureID;
			gCurIntTile.fStructure				= TRUE;
		}
		else
		{
			gCurIntTile.fStructure				= FALSE;
		}


		gfOverIntTile							= TRUE;

	}
	else
	{
		// If we are in cycle mode, end it
		if ( gfCycleIntTile )
		{
			gfCycleIntTile = FALSE;
		}
	}
}


static BOOLEAN RefineLogicOnStruct(INT16 const sGridNo, LEVELNODE const* const pNode)
{
	STRUCTURE		 *pStructure;

	if ( pNode->uiFlags & LEVELNODE_CACHEDANITILE )
	{
		return ( FALSE );
	}

	// See if we are on an interactable tile!
	// Try and get struct data from levelnode pointer
	pStructure = pNode->pStructureData;

	// If no data, quit
	if ( pStructure == NULL )
	{
		return( FALSE );
	}

	if ( !( pStructure->fFlags & ( STRUCTURE_OPENABLE | STRUCTURE_HASITEMONTOP ) ) )
	{
		return( FALSE );
	}

	const SOLDIERTYPE* const sel = GetSelectedMan();
	if (sel != NULL && sel->ubBodyType == ROBOTNOWEAPON) return FALSE;

	// If we are a door, we need a different definition of being visible than other structs
	if ( pStructure->fFlags & STRUCTURE_ANYDOOR )
	{
		if ( !IsDoorVisibleAtGridNo( sGridNo ) )
		{
			return( FALSE );
		}

		// OK, For a OPENED door, addition requirements are: need to be in 'HAND CURSOR' mode...
		if ( pStructure->fFlags & STRUCTURE_OPEN )
		{
			//Are we in hand cursor mode?
			if ( gCurrentUIMode != HANDCURSOR_MODE && gCurrentUIMode != ACTION_MODE )
			{
				return( FALSE );
			}
		}

		// If this option is on...
		if ( !gGameSettings.fOptions[ TOPTION_SNAP_CURSOR_TO_DOOR ] )
		{
			if ( gCurrentUIMode != HANDCURSOR_MODE )
			{
				return( FALSE );
			}
		}
	}
	else
	{
		// IF we are a switch, reject in another direction...
		if ( pStructure->fFlags & STRUCTURE_SWITCH )
		{
			// Find a new gridno based on switch's orientation...
			INT16 sNewGridNo = NOWHERE;

			switch( pStructure->pDBStructureRef->pDBStructure->ubWallOrientation )
			{
				case OUTSIDE_TOP_LEFT:
				case INSIDE_TOP_LEFT:

					// Move south...
					sNewGridNo = NewGridNo( sGridNo, DirectionInc( SOUTH ) );
					break;

				case OUTSIDE_TOP_RIGHT:
				case INSIDE_TOP_RIGHT:

					// Move east...
					sNewGridNo = NewGridNo( sGridNo, DirectionInc( EAST ) );
					break;

			}

			if ( sNewGridNo != NOWHERE )
			{
				// If we are hidden by a roof, reject it!
				if ( !gfBasement && IsRoofVisible2( sNewGridNo ) && !( gTacticalStatus.uiFlags&SHOW_ALL_ITEMS ) )
				{
					return( FALSE );
				}
			}
		}
		else
		{
			// If we are hidden by a roof, reject it!
			if ( !gfBasement && IsRoofVisible( sGridNo ) && !( gTacticalStatus.uiFlags&SHOW_ALL_ITEMS ) )
			{
				return( FALSE );
			}
		}
	}

	return( TRUE );
}


static BOOLEAN RefinePointCollisionOnStruct(INT16 const sGridNo, INT16 const sTestX, INT16 const sTestY, INT16 const sSrcX, INT16 const sSrcY, LEVELNODE const* const pNode)
{
	TILE_ELEMENT *TileElem;

	if ( pNode->uiFlags & LEVELNODE_CACHEDANITILE )
	{
		//Check it!
		return ( CheckVideoObjectScreenCoordinateInData( gpTileCache[ pNode->pAniTile->sCachedTileID ].pImagery->vo, pNode->pAniTile->sCurrentFrame, (INT32)( sTestX - sSrcX  ), (INT32)( -1 * ( sTestY - sSrcY  ) ) ) );

	}
	else
	{
		TileElem = &( gTileDatabase[pNode->usIndex] );

		//Adjust for current frames and animations....
		if ( TileElem->uiFlags & ANIMATED_TILE)
		{
				Assert( TileElem->pAnimData != NULL );
				TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[TileElem->pAnimData->bCurrentFrame]];
		}
		else if( ( pNode->uiFlags & LEVELNODE_ANIMATION ) )
		{
			if ( pNode->sCurrentFrame != -1  )
			{
				Assert( TileElem->pAnimData != NULL );
				TileElem = &gTileDatabase[TileElem->pAnimData->pusFrames[pNode->sCurrentFrame]];
			}
		}

		//Check it!
		return ( CheckVideoObjectScreenCoordinateInData( TileElem->hTileSurface, TileElem->usRegionIndex, (INT32)( sTestX - sSrcX  ), (INT32)( -1 * ( sTestY - sSrcY  ) ) ) );
	}
}


// This function will check the video object at SrcX and SrcY for the lack of transparency
// will return true if data found, else false
BOOLEAN CheckVideoObjectScreenCoordinateInData(HVOBJECT hSrcVObject, UINT16 usIndex, INT32 iTestX, INT32 iTestY)
{
	UINT32 uiOffset;
	UINT32 usHeight, usWidth;
	BOOLEAN	fDataFound = FALSE;
	INT32	 iTestPos, iStartPos;

	// Assertions
	Assert( hSrcVObject != NULL );

	// Get Offsets from Index into structure
	ETRLEObject const* const pTrav = hSrcVObject->SubregionProperties(usIndex);
	usHeight				= (UINT32)pTrav->usHeight;
	usWidth					= (UINT32)pTrav->usWidth;
	uiOffset				= pTrav->uiDataOffset;


	// Calculate test position we are looking for!
	// Calculate from 0, 0 at top left!
	iTestPos	= ( ( usHeight - iTestY ) * usWidth ) + iTestX;
	iStartPos	= 0;

	UINT8 const* SrcPtr = hSrcVObject->PixData(pTrav);

#if 1 // XXX TODO
	do
	{
		for (;;)
		{
			UINT8 PxCount = *SrcPtr++;
			if (PxCount == 0) break;
			if (PxCount & 0x80)
			{
				PxCount &= 0x7F;
			}
			else
			{
				if (iStartPos < iTestPos && iTestPos <= iStartPos + PxCount) return TRUE;
				SrcPtr += PxCount;
			}
			iStartPos += PxCount;
		}
		if (iStartPos >= iTestPos) break;
	}
	while (--usHeight > 0);
#else
	__asm {

		mov		esi, SrcPtr
		mov		edi, iStartPos
		xor		eax, eax
		xor		ebx, ebx
		xor		ecx, ecx

BlitDispatch:

		mov		cl, [esi]
		inc		esi
		or		cl, cl
		js		BlitTransparent
		jz		BlitDoneLine

//BlitNonTransLoop:

		clc
		rcr		cl, 1
		jnc		BlitNTL2

		inc		esi

		// Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1


BlitNTL2:
		clc
		rcr		cl, 1
		jnc		BlitNTL3

		add		esi, 2

		// Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

		// Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1


BlitNTL3:

		or		cl, cl
		jz		BlitDispatch

		xor		ebx, ebx

BlitNTL4:

		add		esi, 4

		// Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

		// Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

		// Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

		// Check
		cmp		edi, iTestPos
		je		BlitFound
		add		edi, 1

		dec		cl
		jnz		BlitNTL4

		jmp		BlitDispatch

BlitTransparent:

		and		ecx, 07fH
//		shl		ecx, 1
		add		edi, ecx
		jmp		BlitDispatch


BlitDoneLine:

// Here check if we have passed!
		cmp		edi, iTestPos
		jge		BlitDone

		dec		usHeight
		jz		BlitDone
		jmp		BlitDispatch


BlitFound:

		mov		fDataFound, 1

BlitDone:
	}
#endif

	return(fDataFound);

}


BOOLEAN ShouldCheckForMouseDetections( )
{
	BOOLEAN fOK = FALSE;

	if ( gsINTOldRenderCenterX != gsRenderCenterX || gsINTOldRenderCenterY != gsRenderCenterY ||
			 gusINTOldMousePosX	!= gusMouseXPos	|| gusINTOldMousePosY	!= gusMouseYPos	)
	{
		fOK = TRUE;
	}

	// Set old values
	gsINTOldRenderCenterX = gsRenderCenterX;
	gsINTOldRenderCenterY = gsRenderCenterY;

	gusINTOldMousePosX		= gusMouseXPos;
	gusINTOldMousePosY		= gusMouseYPos;

	return( fOK );
}


void CycleIntTileFindStack( UINT16 usMapPos )
{
	gfCycleIntTile = TRUE;

	// Cycle around!
	gCurIntTileStack.bCur++;

	//PLot new movement
	gfPlotNewMovement = TRUE;

	if ( gCurIntTileStack.bCur == gCurIntTileStack.bNum )
	{
		gCurIntTileStack.bCur = 0;
	}
}
