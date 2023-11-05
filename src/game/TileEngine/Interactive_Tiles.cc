#include "Animation_Data.h"
#include "Cursor_Control.h"
#include "Cursors.h"
#include "HImage.h"
#include "Isometric_Utils.h"
#include "TileDef.h"
#include "VObject.h"
#include "RenderWorld.h"
#include "Interface.h"
#include "Sound_Control.h"
#include "WorldDef.h"
#include "Interactive_Tiles.h"
#include "WorldMan.h"
#include "Structure.h"
#include "Animation_Control.h"
#include "Overhead.h"
#include "Structure_Wrap.h"
#include "Tile_Animation.h"
#include "Tile_Cache.h"
#include "Handle_Doors.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "Dialogue_Control.h"
#include "English.h"
#include "Handle_Items.h"
#include "Handle_UI.h"
#include "NPC.h"
#include "Explosion_Control.h"
#include "Text.h"
#include "GameSettings.h"
#include "Environment.h"
#include "Debug.h"
#include "UILayout.h"

#include "Soldier.h"
#include "GameInstance.h"
#include "ContentManager.h"
#include "ShippingDestinationModel.h"

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


void StartInteractiveObject(GridNo const gridno, STRUCTURE const& structure, SOLDIERTYPE& s, UINT8 const direction)
{
	// ATE: Patch fix: Don't allow if alreay in animation
	if (s.usAnimState == OPEN_STRUCT)               return;
	if (s.usAnimState == OPEN_STRUCT_CROUCHED)      return;
	if (s.usAnimState == BEGIN_OPENSTRUCT)          return;
	if (s.usAnimState == BEGIN_OPENSTRUCT_CROUCHED) return;

	// Add soldier event for opening door/struct
	Soldier{&s}.setPendingAction(structure.fFlags & STRUCTURE_ANYDOOR ? MERC_OPENDOOR : MERC_OPENSTRUCT);
	s.uiPendingActionData1     = structure.usStructureID;
	s.sPendingActionData2      = gridno;
	s.bPendingActionData3      = direction;
}


bool SoldierHandleInteractiveObject(SOLDIERTYPE& s)
{
	GridNo     const gridno       = s.sPendingActionData2;
	UINT16     const structure_id = (UINT16)s.uiPendingActionData1;
	STRUCTURE* const structure    = FindStructureByID(gridno, structure_id);
	if (!structure) return false;
	return HandleOpenableStruct(&s, gridno, structure);
}


void HandleStructChangeFromGridNo(SOLDIERTYPE* const s, GridNo const grid_no)
{
	STRUCTURE* const structure = FindStructure(grid_no, STRUCTURE_OPENABLE);
	if (!structure)
	{
		SLOGW("Told to handle struct that does not exist at {}.", grid_no);
		return;
	}

	// Do sound...
	bool const closing = structure->fFlags & STRUCTURE_OPEN;
	PlayLocationJA2Sample(grid_no, GetStructureOpenSound(structure, closing), HIGHVOLUME, 1);

	// ATE: Don't handle switches!
	if (!(structure->fFlags & STRUCTURE_SWITCH))
	{
		bool did_missing_quote = false;
		if (s->bTeam == OUR_TEAM)
		{
			auto primaryDest = GCM->getPrimaryShippingDestination();
			if (grid_no        == primaryDest->deliverySectorGridNo  &&
			    gWorldSector == primaryDest->deliverySector &&
					CheckFact(FACT_PABLOS_STOLE_FROM_LATEST_SHIPMENT, 0) &&
					!CheckFact(FACT_PLAYER_FOUND_ITEMS_MISSING, 0))
			{
				SayQuoteFromNearbyMercInSector(grid_no, 3, QUOTE_STUFF_MISSING_DRASSEN);
				did_missing_quote = true;
			}
		}
		else if (s->bTeam == CIV_TEAM)
		{
			if (s->ubProfile != NO_PROFILE)
			{
				TriggerNPCWithGivenApproach(s->ubProfile, APPROACH_DONE_OPEN_STRUCTURE);
			}
		}

		ITEM_POOL* const item_pool = GetItemPool(grid_no, s->bLevel);
		if (item_pool)
		{
			// Update visiblity
			if (!closing)
			{
				bool do_humm     = true;
				bool do_locators = true;

				if (s->bTeam != OUR_TEAM)
				{
					do_humm     = false;
					do_locators = false;
				}

				// Look for ownership here
				if (GetWorldItem(item_pool->iItemIndex).o.usItem == OWNERSHIP)
				{
					do_humm = false;
					MakeCharacterDialogueEventDoBattleSound(*s, BATTLE_SOUND_NOTHING, 500);
				}

				// If now open, set visible
				SetItemsVisibilityOn(grid_no, s->bLevel, ANY_VISIBILITY_VALUE, do_locators);

				// ATE: Check now many things in pool
				if (!did_missing_quote)
				{
					if (item_pool->pNext && item_pool->pNext->pNext)
					{
						MakeCharacterDialogueEventDoBattleSound(*s, BATTLE_SOUND_COOL1, 500);
					}
				else if (do_humm)
				{
					MakeCharacterDialogueEventDoBattleSound(*s, BATTLE_SOUND_HUMM, 500);
				}
				}
			}
			else
			{
				SetItemsVisibilityHidden(grid_no, s->bLevel);
			}
		}
		else
		{
			if (!closing)
			{
				MakeCharacterDialogueEventDoBattleSound(*s, BATTLE_SOUND_NOTHING, 500);
			}
		}
	}

	STRUCTURE* const new_structure = SwapStructureForPartner(structure);
	if (new_structure)
	{
		RecompileLocalMovementCosts(grid_no);
		SetRenderFlags(RENDER_FLAG_FULL);
		if (new_structure->fFlags & STRUCTURE_SWITCH)
		{ // Just turned a switch on!
			ActivateSwitchInGridNo(s, grid_no);
		}
	}
}


UICursorID GetInteractiveTileCursor(UICursorID const old_cursor, BOOLEAN const confirm)
{
	GridNo                 grid_no;
	STRUCTURE*             structure;
	LEVELNODE const* const int_node = GetCurInteractiveTileGridNoAndStructure(&grid_no, &structure);
	if (!int_node || !structure) return old_cursor;

	if (structure->fFlags & STRUCTURE_ANYDOOR)
	{
		SetDoorString(grid_no);
	}
	else if (structure->fFlags & STRUCTURE_SWITCH)
	{
		SetIntTileLocationText(gzLateLocalizedString[STR_LATE_25]);
	}
	return confirm ? OKHANDCURSOR_UICURSOR : NORMALHANDCURSOR_UICURSOR;
}


void SetActionModeDoorCursorText()
{
	// If we are over a merc, don't
	if (gUIFullTarget) return;

	GridNo     grid_no;
	STRUCTURE* structure;
	LEVELNODE const* const int_node = GetCurInteractiveTileGridNoAndStructure(&grid_no, &structure);
	if (!int_node || !structure)                  return;
	if (!(structure->fFlags & STRUCTURE_ANYDOOR)) return;
	SetDoorString(grid_no);
}


static void GetLevelNodeScreenRect(LEVELNODE const& n, SGPRect& rect, INT16 const x, INT16 const y, GridNo const gridno)
{
	// Get 'TRUE' merc position
	INT16 sTempX_S;
	INT16 sTempY_S;
	INT16 const offset_x = x - gsRenderCenterX;
	INT16 const offset_y = y - gsRenderCenterY;
	FromCellToScreenCoordinates(offset_x, offset_y, &sTempX_S, &sTempY_S);

	ETRLEObject const* pTrav;
	if (n.uiFlags & LEVELNODE_CACHEDANITILE)
	{
		ANITILE const& a = *n.pAniTile;
		pTrav = &gpTileCache[a.sCachedTileID].pImagery->vo->SubregionProperties(a.sCurrentFrame);
	}
	else
	{
		TILE_ELEMENT const* te = &gTileDatabase[n.usIndex];
		// Adjust for current frames and animations
		if (te->uiFlags & ANIMATED_TILE)
		{
			TILE_ANIMATION_DATA const& a = *te->pAnimData;
			te = &gTileDatabase[a.pusFrames[a.bCurrentFrame]];
		}
		else if (n.uiFlags & LEVELNODE_ANIMATION && n.sCurrentFrame != -1)
		{
			te = &gTileDatabase[te->pAnimData->pusFrames[n.sCurrentFrame]];
		}
		pTrav = &te->hTileSurface->SubregionProperties(te->usRegionIndex);
	}

	INT16 sScreenX = (g_ui.m_tacticalMapCenterX) + (INT16)sTempX_S;
	INT16 sScreenY = (g_ui.m_tacticalMapCenterY) + (INT16)sTempY_S;

	// Adjust for offset position on screen
	sScreenY -= gpWorldLevelData[gridno].sHeight;

	// Adjust based on interface level
	if (gsInterfaceLevel > 0)
	{
		sScreenY += ROOF_LEVEL_HEIGHT;
	}

	// Adjust for render height
	sScreenY += gsRenderHeight;

	// Add to start position of dest buffer
	sScreenX += pTrav->sOffsetX - WORLD_TILE_X / 2;
	sScreenY += pTrav->sOffsetY - WORLD_TILE_Y / 2;

	// Adjust y offset!
	sScreenY += WORLD_TILE_Y / 2;

	rect.iLeft   = sScreenX;
	rect.iTop    = sScreenY;
	rect.iRight  = sScreenX + pTrav->usWidth;
	rect.iBottom = sScreenY + pTrav->usHeight;
}


static bool RefineLogicOnStruct(GridNo, LEVELNODE const&);
static BOOLEAN RefinePointCollisionOnStruct(INT16 sTestX, INT16 sTestY, INT16 sSrcX, INT16 sSrcY, LEVELNODE const&);


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
	SGPPoint cursorPosition = {};
	GetCursorPos(cursorPosition);

	for (LEVELNODE const* n = gpWorldLevelData[sGridNo].pStructHead; n; n = n->pNext)
	{
		SGPRect aRect;
		GetLevelNodeScreenRect(*n, aRect, sXMapPos, sYMapPos, sGridNo);

		// Make sure we are always on guy if we are on same gridno
		if (!IsPointInScreenRect(cursorPosition.iX, cursorPosition.iY, aRect)) continue;

		if (!RefinePointCollisionOnStruct(cursorPosition.iX, cursorPosition.iY, aRect.iLeft, aRect.iTop, *n)) continue;

		if (!RefineLogicOnStruct(sGridNo, *n)) continue;

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


void EndCurInteractiveTileCheck()
{
	if (gCurIntTile.fFound)
	{ // We are over this cycled node or levelnode
		CUR_INTERACTIVE_TILE const& cur_int_tile =
			gfCycleIntTile ? gCurIntTileStack.bTiles[gCurIntTileStack.bCur] :
			gCurIntTile;

		gCurIntTile.sGridNo    = cur_int_tile.sFoundGridNo;
		gCurIntTile.sTileIndex = cur_int_tile.pFoundNode->usIndex;

		if (cur_int_tile.pFoundNode->pStructureData)
		{
			gCurIntTile.usStructureID = cur_int_tile.pFoundNode->pStructureData->usStructureID;
			gCurIntTile.fStructure    = TRUE;
		}
		else
		{
			gCurIntTile.fStructure = FALSE;
		}

		gfOverIntTile = TRUE;
	}
	else
	{ // If we are in cycle mode, end it
		gfCycleIntTile = FALSE;
	}
}


static bool RefineLogicOnStruct(INT16 gridno, LEVELNODE const& n)
{
	if (n.uiFlags & LEVELNODE_CACHEDANITILE) return false;

	// See if we are on an interactable tile!
	// Try and get struct data from levelnode pointer
	if (!n.pStructureData) return false; // If no data, quit
	STRUCTURE const& structure = *n.pStructureData;

	if (!(structure.fFlags & (STRUCTURE_OPENABLE | STRUCTURE_HASITEMONTOP))) return false;

	SOLDIERTYPE const* const sel = GetSelectedMan();
	if (sel && sel->ubBodyType == ROBOTNOWEAPON) return false;

	if (structure.fFlags & STRUCTURE_ANYDOOR)
	{ // A door, we need a different definition of being visible than other structs
		if (!IsDoorVisibleAtGridNo(gridno)) return false;

		// For a OPENED door, addition requirements are: need to be in 'HAND CURSOR' mode
		if (structure.fFlags & STRUCTURE_OPEN &&
			gCurrentUIMode != HANDCURSOR_MODE &&
			gCurrentUIMode != ACTION_MODE)
		{
			return false;
		}

		if (!gGameSettings.fOptions[TOPTION_SNAP_CURSOR_TO_DOOR] &&
			gCurrentUIMode != HANDCURSOR_MODE)
		{
			return false;
		}

		return true;
	}
	else if (structure.fFlags & STRUCTURE_SWITCH)
	{ // A switch, reject in another direction
		// Find a new gridno based on switch's orientation
		switch (structure.pDBStructureRef->pDBStructure->ubWallOrientation)
		{
			case OUTSIDE_TOP_LEFT:
			case INSIDE_TOP_LEFT:
				// Move south
				gridno = NewGridNo(gridno, DirectionInc(SOUTH));
				break;

			case OUTSIDE_TOP_RIGHT:
			case INSIDE_TOP_RIGHT:
				// Move east
				gridno = NewGridNo(gridno, DirectionInc(EAST));
				break;

			default: return true; // XXX exception?
		}
	}

	// If we are hidden by a roof, reject it!
	if (!gfBasement && IsRoofVisible(gridno) && !(gTacticalStatus.uiFlags & SHOW_ALL_ITEMS))
	{
		return false;
	}

	return true;
}


static BOOLEAN RefinePointCollisionOnStruct(INT16 const test_x, INT16 const test_y, INT16 const src_x, INT16 const src_y, LEVELNODE const& n)
{
	HVOBJECT vo;
	UINT16   idx;
	if (n.uiFlags & LEVELNODE_CACHEDANITILE)
	{
		ANITILE const& a = *n.pAniTile;
		vo  = gpTileCache[a.sCachedTileID].pImagery->vo;
		idx = a.sCurrentFrame;
	}
	else
	{
		TILE_ELEMENT const* te = &gTileDatabase[n.usIndex];
		// Adjust for current frames and animations
		if (te->uiFlags & ANIMATED_TILE)
		{
			TILE_ANIMATION_DATA const& a = *te->pAnimData;
			te = &gTileDatabase[a.pusFrames[a.bCurrentFrame]];
		}
		else if (n.uiFlags & LEVELNODE_ANIMATION && n.sCurrentFrame != -1)
		{
			te = &gTileDatabase[te->pAnimData->pusFrames[n.sCurrentFrame]];
		}
		vo  = te->hTileSurface;
		idx = te->usRegionIndex;
	}
	return CheckVideoObjectScreenCoordinateInData(vo, idx, test_x - src_x, test_y - src_y);
}


// This function will check the video object at SrcX and SrcY for the lack of transparency
// will return true if data found, else false
BOOLEAN CheckVideoObjectScreenCoordinateInData(HVOBJECT srcObj, UINT16 srcIndex, INT32 testX, INT32 testY)
{
	Assert(srcObj != nullptr);

	const ETRLEObject &etrle = srcObj->SubregionProperties(srcIndex);

	const UINT32 *buf = reinterpret_cast<const UINT32 *>(srcObj->PixData(etrle));
	return buf[testY * etrle.usHeight + testX] != 0;
}


BOOLEAN ShouldCheckForMouseDetections( )
{
	BOOLEAN fOK = FALSE;

	if (gsINTOldRenderCenterX != gsRenderCenterX || gsINTOldRenderCenterY != gsRenderCenterY ||
		gusINTOldMousePosX != gusMouseXPos || gusINTOldMousePosY != gusMouseYPos)
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
