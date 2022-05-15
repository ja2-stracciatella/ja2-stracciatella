#include "Debug_Pages.h"
#include "Animation_Data.h"
#include "Environment.h"
#include "Font.h"
#include "Structure.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "Smooth.h"
#include "WorldMan.h"
#include "Lighting.h"
#include "RenderWorld.h"
#include "Overhead.h"
#include "AI.h"
#include "Animation_Control.h"
#include "Isometric_Utils.h"
#include "Font_Control.h"
#include "Message.h"
#include "Tile_Cache.h"
#include "SaveLoadMap.h"
#include "Random.h"
#include "Render_Fun.h"
#include "GameSettings.h"
#include "MemMan.h"

#include <string_theory/format>
#include <string_theory/string>
#include <numeric>
#include <stdexcept>


// LEVEL NODE MANIPLULATION FUNCTIONS
static LEVELNODE* CreateLevelNode(void)
{
	LEVELNODE* const Node = new LEVELNODE{};
	Node->ubShadeLevel        = LightGetAmbient();
	Node->ubNaturalShadeLevel = LightGetAmbient();
	Node->pSoldier            = NULL;
	Node->pNext               = NULL;
	Node->sRelativeX          = 0;
	Node->sRelativeY          = 0;
	return Node;
}


static void CountLevelNodes(int (&guiLNCount)[9])
{
	FOR_EACH_WORLD_TILE(pME)
	{
		// start at 1 to skip land head ptr; 0 stores total
		for (UINT32 uiLoop2 = 1; uiLoop2 < 9; uiLoop2++)
		{
			for (const LEVELNODE* pLN = pME->pLevelNodes[uiLoop2]; pLN != NULL; pLN = pLN->pNext)
			{
				guiLNCount[uiLoop2]++;
			}
		}
	}
	guiLNCount[0] = std::accumulate(guiLNCount + 1, guiLNCount + 9, 0);
}


void DebugLevelNodePage(void)
{
	int guiLNCount[9] {};
	const ST::string levelString[]
	{
		ST_LITERAL("Land"),
		ST_LITERAL("Object"),
		ST_LITERAL("Struct"),
		ST_LITERAL("Shadow"),
		ST_LITERAL("Merc"),
		ST_LITERAL("Roof"),
		ST_LITERAL("Onroof"),
		ST_LITERAL("Topmost")
	};

	MPageHeader("DEBUG LEVELNODES PAGE 1 OF 1");
	INT32 y = DEBUG_PAGE_START_Y;
	INT32 h = DEBUG_PAGE_LINE_HEIGHT;
	CountLevelNodes(guiLNCount);

	for (UINT32 uiLoop = 1; uiLoop < 9; uiLoop++)
	{
		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, levelString[uiLoop - 1], guiLNCount[uiLoop]);
	}
	MPrint(DEBUG_PAGE_FIRST_COLUMN, y += h, ST::format("{} land nodes in excess of world max (25600)", guiLNCount[1] - WORLD_MAX));
	MPrint(DEBUG_PAGE_FIRST_COLUMN, y += h, ST::format("Total # levelnodes {}, {} bytes each", guiLNCount[0], sizeof(LEVELNODE)));
	MPrint(DEBUG_PAGE_FIRST_COLUMN, y += h, ST::format("Total memory for levelnodes {}", guiLNCount[0] * sizeof(LEVELNODE)));
}


static LEVELNODE* FindTypeInLayer(LEVELNODE* const start_node, UINT32 const type)
{
	// Look through all objects and Search for type
	for (LEVELNODE* i = start_node; i; i = i->pNext)
	{
		UINT16 const idx = i->usIndex;
		if (idx == NO_TILE || idx >= NUMBEROFTILES) continue;
		if (GetTileType(idx) != type) continue;
		return i;
	}
	return 0;
}


// First for object layer
// #################################################################

LEVELNODE* AddObjectToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	// Append node to list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pObjectHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_OBJECTS);
	return n;
}


LEVELNODE* AddObjectToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	LEVELNODE** const head = &gpWorldLevelData[iMapIndex].pObjectHead;
	n->pNext = *head;
	*head    = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_OBJECTS);
	AddObjectToMapTempFile(iMapIndex, usIndex);
	return n;
}


BOOLEAN RemoveObject(UINT32 iMapIndex, UINT16 usIndex)
{
	// Look through all objects and remove index if found
	LEVELNODE* pOldObject = NULL;
	for (LEVELNODE* pObject = gpWorldLevelData[iMapIndex].pObjectHead; pObject != NULL; pObject = pObject->pNext)
	{
		if (pObject->usIndex == usIndex)
		{
			// OK, set links
			// Check for head or tail
			if (pOldObject == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pObjectHead = pObject->pNext;
			}
			else
			{
				pOldObject->pNext = pObject->pNext;
			}

			CheckForAndDeleteTileCacheStructInfo(pObject, usIndex);

			delete pObject;

			//Add the index to the maps temp file so we can remove it after reloading the map
			AddRemoveObjectToMapTempFile(iMapIndex, usIndex);

			return TRUE;
		}

		pOldObject = pObject;
	}

	// Could not find it
	return FALSE;
}


LEVELNODE* TypeRangeExistsInObjectLayer(UINT32 const iMapIndex, UINT32 const fStartType, UINT32 const fEndType)
{
	// Look through all objects and Search for type
	for (LEVELNODE* pObject = gpWorldLevelData[iMapIndex].pObjectHead; pObject != NULL; pObject = pObject->pNext)
	{
		if (pObject->usIndex == NO_TILE || pObject->usIndex >= NUMBEROFTILES) continue;

		UINT32 const fTileType = GetTileType(pObject->usIndex);
		if (fTileType < fStartType || fEndType < fTileType) continue;

		return pObject;
	}

	// Could not find it
	return 0;
}


LEVELNODE* FindTypeInObjectLayer(UINT32 const map_idx, UINT32 const type)
{
	return FindTypeInLayer(gpWorldLevelData[map_idx].pObjectHead, type);
}


BOOLEAN RemoveAllObjectsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	BOOLEAN fRetVal = FALSE;

	// Look through all objects and Search for type
	for (LEVELNODE* pObject = gpWorldLevelData[iMapIndex].pObjectHead; pObject != NULL;)
	{
		LEVELNODE* Next = pObject->pNext;

		if (pObject->usIndex != NO_TILE && pObject->usIndex < NUMBEROFTILES)
		{
			const UINT32 fTileType = GetTileType(pObject->usIndex);
			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				RemoveObject(iMapIndex, pObject->usIndex);
				fRetVal = TRUE;
			}
		}

		pObject = Next;
	}
	return fRetVal;
}


// #######################################################
// Land Piece Layer
// #######################################################

LEVELNODE* AddLandToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	// Append node to list
	LEVELNODE*  prev   = NULL;
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pLandHead;
	while (*anchor != NULL)
	{
		prev   = *anchor;
		anchor = &prev->pNext;
	}
	*anchor      = n;
	n->pPrevNode = prev;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
	return n;
}


void AddLandToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex		= usIndex;

	LEVELNODE** const head = &gpWorldLevelData[iMapIndex].pLandHead;
	if (*head != NULL) (*head)->pPrevNode = n;
	n->pNext     = *head;
	n->pPrevNode = NULL;
	*head = n;

	if (usIndex < NUMBEROFTILES && gTileDatabase[usIndex].ubFullTile)
	{
		gpWorldLevelData[iMapIndex].pLandStart = n;
	}

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
}


static BOOLEAN AdjustForFullTile(UINT32 iMapIndex);
static void RemoveLandEx(UINT32 iMapIndex, UINT16 usIndex);


void RemoveLand(UINT32 const map_idx, UINT16 const idx)
{
	RemoveLandEx(map_idx, idx);
	AdjustForFullTile(map_idx);
}


static void RemoveLandEx(UINT32 iMapIndex, UINT16 usIndex)
{
	// Look through all Lands and remove index if found
	for (LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead; pLand != NULL; pLand = pLand->pNext)
	{
		if (pLand->usIndex == usIndex)
		{
			// Check for head
			if (pLand->pPrevNode == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pLandHead = pLand->pNext;
			}
			else
			{
				pLand->pPrevNode->pNext = pLand->pNext;
			}

			// Check for tail
			if (pLand->pNext != NULL)
			{
				pLand->pNext->pPrevNode = pLand->pPrevNode;
			}

			delete pLand;
			break;
		}
	}
}


static BOOLEAN AdjustForFullTile(UINT32 iMapIndex)
{
	for (LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead; pLand != NULL; pLand = pLand->pNext)
	{
		if (pLand->usIndex < NUMBEROFTILES)
		{
			// If this is a full tile, set new full tile
			if (gTileDatabase[pLand->usIndex].ubFullTile)
			{
				gpWorldLevelData[iMapIndex].pLandStart = pLand;
				return TRUE;
			}
		}
	}

	// Could not find a full tile
	// Set to tail, and convert it to a full tile!
	// Add a land piece to tail from basic land

	UINT16 NewIndex = Random(10);

	// Adjust for type
	NewIndex += gTileTypeStartIndex[gCurrentBackground];

	LEVELNODE* pNewNode = AddLandToTail(iMapIndex, NewIndex);
	gpWorldLevelData[iMapIndex].pLandStart = pNewNode;

	return FALSE;
}


void ReplaceLandIndex(UINT32 const iMapIndex, UINT16 const usOldIndex, UINT16 const usNewIndex)
{
	// Look through all Lands and remove index if found
	for (LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead; pLand != NULL; pLand = pLand->pNext)
	{
		if (pLand->usIndex == usOldIndex)
		{
			// OK, set new index value
			pLand->usIndex = usNewIndex;
			AdjustForFullTile(iMapIndex);
			return;
		}
	}

	// Could not find it
	throw std::logic_error("Tried to replace non-existent land index");
}


LEVELNODE* FindTypeInLandLayer(UINT32 const map_idx, UINT32 const type)
{
	return FindTypeInLayer(gpWorldLevelData[map_idx].pLandHead, type);
}


BOOLEAN TypeRangeExistsInLandLayer(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType)
{
	// Look through all objects and Search for type
	for (const LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead; pLand != NULL; )
	{
		if (pLand->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pLand->usIndex);

			pLand = pLand->pNext; // XXX TODO0009 if pLand->usIndex == NO_TILE this is an endless loop

			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				return TRUE;
			}
		}
	}

	// Could not find it
	return FALSE;
}


BOOLEAN RemoveAllLandsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	const LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead;
	BOOLEAN fRetVal = FALSE;

	// Look through all objects and Search for type
	while (pLand != NULL)
	{
		if (pLand->usIndex != NO_TILE)
		{
			const LEVELNODE* Next = pLand->pNext;

			const UINT32 fTileType = GetTileType(pLand->usIndex);
			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				// Remove Item
				RemoveLand(iMapIndex, pLand->usIndex);
				fRetVal = TRUE;
			}

			pLand = Next; // XXX TODO0009 if pLand->usIndex == NO_TILE this is an endless loop
		}
	}
	return fRetVal;
}


void DeleteAllLandLayers(UINT32 iMapIndex)
{
	const LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead;

	while (pLand != NULL)
	{
		const LEVELNODE* pOldLand = pLand;
		pLand = pLand->pNext;
		RemoveLandEx(iMapIndex, pOldLand->usIndex);
	}

	// Set world data values
	gpWorldLevelData[iMapIndex].pLandHead = NULL;
	gpWorldLevelData[iMapIndex].pLandStart = NULL;
}


void InsertLandIndexAtLevel(const UINT32 iMapIndex, const UINT16 usIndex, const UINT8 ubLevel)
{
	// If we want to insert at head;
	if (ubLevel == 0)
	{
		AddLandToHead(iMapIndex, usIndex);
		return;
	}

	// Move to index before insertion
	LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead;
	for (UINT8 level = 0;; ++level)
	{
		if (!pLand) throw std::logic_error("Tried to insert land index at invalid level");

		if (level == ubLevel - 1) break;

		pLand = pLand->pNext;
	}

	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	// Set links, according to position!
	n->pPrevNode = pLand;
	n->pNext     = pLand->pNext;
	pLand->pNext = n;

	// Check for tail
	if (n->pNext != NULL) n->pNext->pPrevNode = n;

	AdjustForFullTile(iMapIndex);

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
}


void RemoveHigherLandLevels(UINT32 const map_idx, UINT32 const src_type, std::vector<UINT32>& out_higher_types)
{
	out_higher_types.clear();

	// Get tail
	LEVELNODE* tail = 0;
	for (LEVELNODE* i = gpWorldLevelData[map_idx].pLandHead; i; i = i->pNext)
	{
		tail = i;
	}

	UINT8 const src_log_height = GetTileTypeLogicalHeight(src_type);
	for (LEVELNODE* i = tail; i;)
	{
		LEVELNODE const& l = *i;
		i = i->pPrevNode;

		UINT32 const tile_type = GetTileType(l.usIndex);
		if (GetTileTypeLogicalHeight(tile_type) <= src_log_height) continue;

		RemoveLand(map_idx, l.usIndex);

		out_higher_types.push_back(tile_type);
	}

	AdjustForFullTile(map_idx);
}


static LEVELNODE* AddNodeToWorld(UINT32 const iMapIndex, UINT16 const usIndex, INT8 const level)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	if (usIndex >= NUMBEROFTILES) return n;

	const DB_STRUCTURE_REF* const sr = gTileDatabase[usIndex].pDBStructureRef;
	if (!sr) return n;

	if (AddStructureToWorld(iMapIndex, level, sr, n)) return n;

	delete n;
	throw FailedToAddNode();
}


// Struct layer
// #################################################################

static LEVELNODE* AddStructToTailCommon(UINT32 const map_idx, UINT16 const idx, LEVELNODE* const n)
{
	MAP_ELEMENT& me = gpWorldLevelData[map_idx];
	// Append node to list
	LEVELNODE** anchor = &me.pStructHead;
	while (*anchor) anchor = &(*anchor)->pNext;
	*anchor = n;

	if (idx < NUMBEROFTILES)
	{
		TILE_ELEMENT const& te = gTileDatabase[idx];
		// Check flags for tiledat and set a shadow if we have a buddy
		if (!GridNoIndoors(map_idx) && te.uiFlags & HAS_SHADOW_BUDDY && te.sBuddyNum != -1)
		{
			LEVELNODE* const n = AddShadowToHead(map_idx, te.sBuddyNum);
			n->uiFlags |= LEVELNODE_BUDDYSHADOW;
		}

		// Check for special flag to stop burn-through on same-tile structs
		if (DB_STRUCTURE_REF const* const sr = te.pDBStructureRef)
		{
			// If we are NOT a wall and NOT multi-tiles, set mapelement flag
			if (!FindStructure(map_idx, STRUCTURE_WALLSTUFF) && sr->pDBStructure->ubNumberOfTiles == 1) // XXX TODO0015
			{
				me.ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
			}
			else
			{
				me.ubExtFlags[0] &= ~MAPELEMENT_EXT_NOBURN_STRUCT;
			}
		}
	}

	AddStructToMapTempFile(map_idx, idx);

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
	return n;
}


LEVELNODE* AddStructToTail(UINT32 const map_idx, UINT16 const idx)
{
	LEVELNODE* const n = AddNodeToWorld(map_idx, idx, 0);
	return AddStructToTailCommon(map_idx, idx, n);
}


LEVELNODE* ForceStructToTail(UINT32 const map_idx, UINT16 const idx)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = idx;
	return AddStructToTailCommon(map_idx, idx, n);
}


void AddStructToHead(UINT32 const map_idx, UINT16 const idx)
{
	LEVELNODE* const n = AddNodeToWorld(map_idx, idx, 0);

	MAP_ELEMENT& me = gpWorldLevelData[map_idx];
	// Prepend node to list
	LEVELNODE** const head = &me.pStructHead;
	n->pNext = *head;
	*head = n;

	if (idx < NUMBEROFTILES)
	{
		TILE_ELEMENT const& te = gTileDatabase[idx];
		// Check flags for tiledat and set a shadow if we have a buddy
		if (!GridNoIndoors(map_idx) && te.uiFlags & HAS_SHADOW_BUDDY && te.sBuddyNum != -1)
		{
			LEVELNODE* const n = AddShadowToHead(map_idx, te.sBuddyNum);
			n->uiFlags |= LEVELNODE_BUDDYSHADOW;
		}

		// Check for special flag to stop burn-through on same-tile structs
		if (DB_STRUCTURE_REF const* const sr = te.pDBStructureRef)
		{
			// If we are NOT a wall and NOT multi-tiles, set mapelement flag
			if (FindStructure(map_idx, STRUCTURE_WALLSTUFF) && sr->pDBStructure->ubNumberOfTiles == 1) // XXX TODO0015
			{
				me.ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
			}
			else
			{
				me.ubExtFlags[0] &= ~MAPELEMENT_EXT_NOBURN_STRUCT;
			}
		}
	}

	AddStructToMapTempFile(map_idx, idx);

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
}


static void InsertStructIndex(const UINT32 iMapIndex, const UINT16 usIndex, const UINT8 ubLevel)
{
	// If we want to insert at head
	if (ubLevel == 0)
	{
		AddStructToHead(iMapIndex, usIndex);
		return;
	}

	// Move to index before insertion
	LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead;
	for (UINT8 level = 0;; ++level)
	{
		if (!pStruct) throw std::logic_error("Tried to insert struct at invalid level");

		if (level == ubLevel - 1) break;

		pStruct = pStruct->pNext;
	}

	LEVELNODE* const n = AddNodeToWorld(iMapIndex, usIndex, 0);

	// Set links, according to position!
	n->pNext       = pStruct->pNext;
	pStruct->pNext = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
}


static BOOLEAN RemoveShadow(UINT32 iMapIndex, UINT16 usIndex);


static void RemoveShadowBuddy(UINT32 iMapIndex, UINT16 usIndex)
{
	if (usIndex >= NUMBEROFTILES) return;
	if (GridNoIndoors(iMapIndex)) return;

	const TILE_ELEMENT* const te = &gTileDatabase[usIndex];
	if (!(te->uiFlags & HAS_SHADOW_BUDDY)) return;
	if (te->sBuddyNum == -1)               return;

	RemoveShadow(iMapIndex, te->sBuddyNum);
}


void ForceRemoveStructFromTail(UINT32 const iMapIndex)
{
	LEVELNODE* pPrevStruct	= NULL;

	// GOTO TAIL
	for (LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead; pStruct != NULL; pStruct = pStruct->pNext)
	{
		// AT THE TAIL
		if (pStruct->pNext == NULL)
		{
			if (pPrevStruct != NULL)
			{
				pPrevStruct->pNext = pStruct->pNext;
			}
			else
			{
				gpWorldLevelData[iMapIndex].pStructHead = pPrevStruct;
			}

			UINT16 usIndex = pStruct->usIndex;

			// XXX TODO000A It rather seems like a memory leak not to DeleteStructureFromWorld() here. See InternalRemoveStruct()

			//If we have to, make sure to remove this node when we reload the map from a saved game
			RemoveStructFromMapTempFile(iMapIndex, usIndex);

			delete pStruct;

			RemoveShadowBuddy(iMapIndex, usIndex);
			return;
		}

		pPrevStruct = pStruct;
	}
}


static void InternalRemoveStruct(UINT32 const map_idx, LEVELNODE** const anchor)
{
	LEVELNODE* const removee = *anchor;
	*anchor = removee->pNext;

	// Delete memory assosiated with item
	DeleteStructureFromWorld(removee->pStructureData);

	UINT16 const idx = removee->usIndex;

	// If we have to, make sure to remove this node when we reload the map from a saved game
	RemoveStructFromMapTempFile(map_idx, idx);

	RemoveShadowBuddy(map_idx, idx);
	delete removee;
}


void RemoveStruct(UINT32 const map_idx, UINT16 const idx)
{
	// Look through all structs and remove index if found
	for (LEVELNODE** anchor = &gpWorldLevelData[map_idx].pStructHead;; anchor = &(*anchor)->pNext)
	{
		LEVELNODE* const i = *anchor;
		if (!i) return; // XXX exception?
		if (i->usIndex != idx) continue;
		InternalRemoveStruct(map_idx, anchor);
		return;
	}
}


void RemoveStructFromLevelNode(UINT32 const map_idx, LEVELNODE* const n)
{
	// Look through all structs and remove index if found
	for (LEVELNODE** anchor = &gpWorldLevelData[map_idx].pStructHead;; anchor = &(*anchor)->pNext)
	{
		LEVELNODE* const i = *anchor;
		if (!i) return; // XXX exception?
		if (i != n) continue;
		InternalRemoveStruct(map_idx, anchor);
		return;
	}
}


BOOLEAN RemoveAllStructsOfTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType)
{
	BOOLEAN fRetVal = FALSE;

	// Look through all structs and Search for type
	for (const LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead; pStruct != NULL;)
	{
		if (pStruct->uiFlags & LEVELNODE_CACHEDANITILE)
		{
			pStruct = pStruct->pNext;
			continue;
		}

		if (pStruct->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pStruct->usIndex);

			// Advance to next
			const LEVELNODE* pOldStruct = pStruct;
			pStruct = pStruct->pNext; // XXX TODO0009 if pStruct->usIndex == NO_TILE this is an endless loop

			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				UINT16 usIndex = pOldStruct->usIndex;
				if (usIndex < NUMBEROFTILES)
				{
					RemoveStruct(iMapIndex, usIndex);
					fRetVal = TRUE;
					RemoveShadowBuddy(iMapIndex, usIndex);
				}
			}
		}
	}
	return fRetVal;
}


//Kris:  This was a serious problem.  When saving the map and then reloading it, the structure
//  information was invalid if you changed the types, etc.  This is the bulletproof way.
BOOLEAN ReplaceStructIndex(UINT32 iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex)
{
	RemoveStruct(iMapIndex, usOldIndex);
	AddWallToStructLayer(iMapIndex, usNewIndex, FALSE);
	return TRUE;
//	LEVELNODE	*pStruct				= NULL;
//	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;
// Look through all Structs and remove index if found
//	while( pStruct != NULL )
//	{
//		if ( pStruct->usIndex == usOldIndex )
//		{
//			// OK, set new index value
//			pStruct->usIndex = usNewIndex;
//			AdjustForFullTile( iMapIndex );
//			return( TRUE );
//		}
//		// Advance
//		pStruct = pStruct->pNext;
//	}
//	// Could not find it, return FALSE
//	return( FALSE );
}


// When adding, put in order such that it's drawn before any walls of a
// lesser orientation value
bool AddWallToStructLayer(INT32 const map_idx, UINT16 const idx, bool const replace)
{
	// Get orientation of piece we want to add
	UINT16 const wall_orientation = GetWallOrientation(idx);

	// Look through all objects and Search for orientation
	bool  insert_found = false;
	bool  roof_found   = false;
	UINT8 roof_level   = 0;
	UINT8 level        = 0;
	for (LEVELNODE* i = gpWorldLevelData[map_idx].pStructHead; i; ++level, i = i->pNext)
	{
		if (i->uiFlags & LEVELNODE_CACHEDANITILE) continue;
		UINT16 const check_wall_orient = GetWallOrientation(i->usIndex);

		/* Kris: If placing a new wall which is at right angles to the current wall,
		 * then we insert it. */
		if (check_wall_orient > wall_orientation)
		{
			if (((wall_orientation  == INSIDE_TOP_RIGHT || wall_orientation  == OUTSIDE_TOP_RIGHT) &&
					(check_wall_orient == INSIDE_TOP_LEFT  || check_wall_orient == OUTSIDE_TOP_LEFT)) ||
					((wall_orientation  == INSIDE_TOP_LEFT  || wall_orientation  == OUTSIDE_TOP_LEFT) &&
					(check_wall_orient == INSIDE_TOP_RIGHT || check_wall_orient == OUTSIDE_TOP_RIGHT)))
			{
				insert_found = true;
			}
		}

		UINT32 const check_type = GetTileType(i->usIndex);
		if (FIRSTROOF <= check_type && check_type <= LASTROOF)
		{
			roof_found = true;
			roof_level = level;
		}

		/* Kris: We want to check for walls being parallel to each other.  If so,
		 * then we we want to replace it.  This is because of an existing problem
		 * with say, INSIDE_TOP_LEFT and OUTSIDE_TOP_LEFT walls coexisting. */
		if (((wall_orientation  == INSIDE_TOP_RIGHT || wall_orientation  == OUTSIDE_TOP_RIGHT) &&
			(check_wall_orient == INSIDE_TOP_RIGHT || check_wall_orient == OUTSIDE_TOP_RIGHT)) ||
			((wall_orientation  == INSIDE_TOP_LEFT  || wall_orientation  == OUTSIDE_TOP_LEFT) &&
			(check_wall_orient == INSIDE_TOP_LEFT  || check_wall_orient == OUTSIDE_TOP_LEFT)))
		{
			// Same, if replace, replace here
			return replace ? ReplaceStructIndex(map_idx, i->usIndex, idx) : false;
		}
	}

	// Check if we found an insert position, otherwise set to head
	if (insert_found)
	{
		AddStructToHead(map_idx, idx);
	}
	else if (roof_found) // Make sure it's ALWAYS after the roof (if any)
	{
		InsertStructIndex(map_idx, idx, roof_level);
	}
	else
	{
		AddStructToTail(map_idx, idx);
	}

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
	return true;
}


static bool IndexExistsInLayer(LEVELNODE const* n, UINT16 const tile_index)
{
	for (; n; n = n->pNext)
	{
		if (n->usIndex == tile_index) return true;
	}
	return false;
}


BOOLEAN IndexExistsInStructLayer(GridNo const grid_no, UINT16 const tile_index)
{
	return IndexExistsInLayer(gpWorldLevelData[grid_no].pStructHead, tile_index);
}


void HideStructOfGivenType(UINT32 const iMapIndex, UINT32 const fType, BOOLEAN const fHide)
{
	if (fHide)
	{
		SetRoofIndexFlagsFromTypeRange(iMapIndex, fType, fType, LEVELNODE_HIDDEN);
	}
	else
	{
		// ONLY UNHIDE IF NOT REAVEALED ALREADY
		if (!(gpWorldLevelData[iMapIndex].uiFlags & MAPELEMENT_REVEALED))
		{
			RemoveRoofIndexFlagsFromTypeRange(iMapIndex, fType, fType, LEVELNODE_HIDDEN);
		}
	}
}


// Shadow layer
// #################################################################

void AddShadowToTail(UINT32 const iMapIndex, UINT16 const usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	// Append node to list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pShadowHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_SHADOWS);
}


//Kris:  identical shadows can exist in the same gridno, though it makes no sense
//		because it actually renders the shadows darker than the others.  This is an
//		undesirable effect with walls and buildings so I added this function to make
//		sure there isn't already a shadow before placing it.
void AddExclusiveShadow(UINT32 iMapIndex, UINT16 usIndex)
{
	for (LEVELNODE* pShadow = gpWorldLevelData[iMapIndex].pShadowHead; pShadow; pShadow = pShadow->pNext)
	{
		if (pShadow->usIndex == usIndex)
			return;
	}
	AddShadowToHead(iMapIndex, usIndex);
}


LEVELNODE* AddShadowToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	// Prepend node to list
	LEVELNODE** const head = &gpWorldLevelData[iMapIndex].pShadowHead;
	n->pNext = *head;
	*head = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_SHADOWS);
	return n;
}


static BOOLEAN RemoveShadow(UINT32 iMapIndex, UINT16 usIndex)
{
	// Look through all shadows and remove index if found
	LEVELNODE* pOldShadow = NULL;
	for (LEVELNODE* pShadow = gpWorldLevelData[iMapIndex].pShadowHead; pShadow != NULL; pShadow = pShadow->pNext)
	{
		if (pShadow->usIndex == usIndex)
		{
			// OK, set links
			// Check for head
			if (pOldShadow == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pShadowHead = pShadow->pNext;
			}
			else
			{
				pOldShadow->pNext = pShadow->pNext;
			}

			delete pShadow;
			return TRUE;
		}

		pOldShadow = pShadow;
	}

	// Could not find it
	return FALSE;
}


BOOLEAN RemoveShadowFromLevelNode(UINT32 iMapIndex, LEVELNODE* pNode)
{
	LEVELNODE* pOldShadow = NULL;
	for (LEVELNODE* pShadow = gpWorldLevelData[iMapIndex].pShadowHead; pShadow != NULL; pShadow = pShadow->pNext)
	{
		if (pShadow == pNode)
		{
			// OK, set links
			// Check for head
			if (pOldShadow == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pShadowHead = pShadow->pNext;
			}
			else
			{
				pOldShadow->pNext = pShadow->pNext;
			}

			delete pShadow;
			return TRUE;
		}

		pOldShadow = pShadow;
	}

	// Could not find it
	return FALSE;
}


BOOLEAN RemoveAllShadowsOfTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType)
{
	BOOLEAN fRetVal = FALSE;

	// Look through all shadows and Search for type
	for (const LEVELNODE* pShadow = gpWorldLevelData[iMapIndex].pShadowHead; pShadow != NULL;)
	{
		if (pShadow->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pShadow->usIndex);

			// Advance to next
			const LEVELNODE* pOldShadow = pShadow;
			pShadow = pShadow->pNext;

			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				RemoveShadow(iMapIndex, pOldShadow->usIndex);
				fRetVal = TRUE;
			}
		}
	}
	return fRetVal;
}


BOOLEAN RemoveAllShadows( UINT32 iMapIndex )
{
	BOOLEAN fRetVal = FALSE;

	for (LEVELNODE* pShadow = gpWorldLevelData[iMapIndex].pShadowHead; pShadow != NULL;)
	{
		if (pShadow->usIndex != NO_TILE)
		{
			// Advance to next
			const LEVELNODE* pOldShadow = pShadow;
			pShadow = pShadow->pNext;

			RemoveShadow(iMapIndex, pOldShadow->usIndex);
			fRetVal = TRUE;
		}
	}
	return fRetVal;
}


// Merc layer
// #################################################################


static void AddMercStructureInfo(INT16 sGridNo, SOLDIERTYPE* pSoldier);


LEVELNODE* AddMercToHead(UINT32 const iMapIndex, SOLDIERTYPE& s, BOOLEAN const fAddStructInfo)
{
	LEVELNODE* pMerc = gpWorldLevelData[iMapIndex].pMercHead;

	LEVELNODE* pNextMerc = CreateLevelNode();
	pNextMerc->pNext = pMerc;
	pNextMerc->pSoldier = &s;
	pNextMerc->uiFlags |= LEVELNODE_SOLDIER;

	// Add structure info if we want
	if (fAddStructInfo)
	{
		// Set soldier's levelnode
		s.pLevelNode = pNextMerc;
		AddMercStructureInfo(iMapIndex, &s);
	}

	gpWorldLevelData[iMapIndex].pMercHead = pNextMerc;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_MERCS | TILES_DYNAMIC_STRUCT_MERCS | TILES_DYNAMIC_HIGHMERCS);
	return pNextMerc;
}


static void AddMercStructureInfo(INT16 sGridNo, SOLDIERTYPE* pSoldier)
{
	UINT16 const usAnimSurface = GetSoldierAnimationSurface(pSoldier);
	AddMercStructureInfoFromAnimSurface(sGridNo, pSoldier, usAnimSurface, pSoldier->usAnimState);
}


BOOLEAN AddMercStructureInfoFromAnimSurface(const INT16 sGridNo, SOLDIERTYPE* const s, const UINT16 usAnimSurface, const UINT16 usAnimState)
{
	s->uiStatusFlags &= ~SOLDIER_MULTITILE;

	LEVELNODE* const n = s->pLevelNode;
	if (n == NULL || usAnimSurface == INVALID_ANIMATION_SURFACE) return FALSE;

	// Remove existing structs
	DeleteStructureFromWorld(n->pStructureData);
	n->pStructureData = NULL;

	const STRUCTURE_FILE_REF* const sfr = GetAnimationStructureRef(s, usAnimSurface, usAnimState);
	if (sfr == NULL) return TRUE; // XXX why TRUE?

	const DB_STRUCTURE_REF* const sr =
		s->ubBodyType == QUEENMONSTER ? // Queen uses only one direction
			&sfr->pDBStructureRef[0] :
			&sfr->pDBStructureRef[OneCDirection(s->bDirection)];

	bool const success = AddStructureToWorld(sGridNo, s->bLevel, sr, n);
	if (!success)
	{
		SLOGD("add struct info for merc {} ({}), at {} direction {} failed",
					s->ubID, s->name, sGridNo, s->bDirection);
	}

	// Turn on if we are multi-tiled
	if (sr->pDBStructure->ubNumberOfTiles > 1) s->uiStatusFlags |= SOLDIER_MULTITILE;

	return success;
}


BOOLEAN OKToAddMercToWorld( SOLDIERTYPE *pSoldier, INT8 bDirection )
{
	//if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE)
	{
		// Get surface data
		UINT16 const usAnimSurface = GetSoldierAnimationSurface(pSoldier);
		if (usAnimSurface == INVALID_ANIMATION_SURFACE)
		{
			return FALSE;
		}

		// Now check if we have multi-tile info!
		const STRUCTURE_FILE_REF* const pStructFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface, pSoldier->usAnimState);
		if (pStructFileRef != NULL)
		{
			//Try adding struct to this location, if we can it's good!
			UINT16 usOKToAddStructID;
			if (pSoldier->pLevelNode && pSoldier->pLevelNode->pStructureData != NULL)
			{
				usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
			}
			else
			{
				usOKToAddStructID = INVALID_STRUCTURE_ID;
			}

			if (!OkayToAddStructureToWorld(pSoldier->sGridNo, pSoldier->bLevel, &pStructFileRef->pDBStructureRef[OneCDirection(bDirection)], usOKToAddStructID))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}


BOOLEAN UpdateMercStructureInfo(SOLDIERTYPE *pSoldier)
{
	if (pSoldier->pLevelNode == NULL)
	{
		return FALSE;
	}

	AddMercStructureInfo(pSoldier->sGridNo, pSoldier);
	return TRUE;
}


void RemoveMerc(UINT32 const map_idx, SOLDIERTYPE& s, bool const placeholder)
{
	if (map_idx == NOWHERE) return; // XXX exception?

	for (LEVELNODE** anchor = &gpWorldLevelData[map_idx].pMercHead;; anchor = &(*anchor)->pNext)
	{
		LEVELNODE* const merc = *anchor;
		if (!merc) break;

		if (merc->pSoldier != &s) continue;
		if (placeholder ^ ((merc->uiFlags & LEVELNODE_MERCPLACEHOLDER) != 0)) continue;

		*anchor = merc->pNext;

		if (!placeholder)
		{
			s.pLevelNode = 0;
			DeleteStructureFromWorld(merc->pStructureData);
		}

		delete merc;
		break;
	}
	// XXX exception?
}


// Roof layer
// #################################################################

static LEVELNODE* AddRoof(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddNodeToWorld(iMapIndex, usIndex, 1);
	ResetSpecificLayerOptimizing(TILES_DYNAMIC_ROOF);
	return n;
}


LEVELNODE* AddRoofToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddRoof(iMapIndex, usIndex);

	// Append node to list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pRoofHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	return n;
}


LEVELNODE* AddRoofToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddRoof(iMapIndex, usIndex);

	// Prepend node to list
	LEVELNODE** const head = &gpWorldLevelData[iMapIndex].pRoofHead;
	n->pNext = *head;
	*head = n;

	return n;
}


BOOLEAN RemoveRoof(UINT32 iMapIndex, UINT16 usIndex)
{
	// Look through all Roofs and remove index if found
	LEVELNODE* pOldRoof = NULL;
	for (LEVELNODE* pRoof = gpWorldLevelData[iMapIndex].pRoofHead; pRoof != NULL; pRoof = pRoof->pNext)
	{
		if (pRoof->usIndex == usIndex)
		{
			// OK, set links
			// Check for head
			if (pOldRoof == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pRoofHead = pRoof->pNext;
			}
			else
			{
				pOldRoof->pNext = pRoof->pNext;
			}

			DeleteStructureFromWorld(pRoof->pStructureData);
			delete pRoof;
			return TRUE;
		}

		pOldRoof = pRoof;
	}

	// Could not find it
	return FALSE;
}


LEVELNODE* FindTypeInRoofLayer(UINT32 const map_idx, UINT32 const type)
{
	return FindTypeInLayer(gpWorldLevelData[map_idx].pRoofHead, type);
}


LEVELNODE* TypeRangeExistsInRoofLayer(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType)
{
	// Look through all objects and Search for type
	for (LEVELNODE* pRoof = gpWorldLevelData[iMapIndex].pRoofHead; pRoof;)
	{
		if (pRoof->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pRoof->usIndex);
			if (fStartType <= fTileType && fTileType <= fEndType)
			{
				return pRoof;
			}
			pRoof = pRoof->pNext; // XXX TODO0009 if pRoof->usIndex == NO_TILE this is an endless loop
		}
	}

	// Could not find it
	return 0;
}


BOOLEAN IndexExistsInRoofLayer(INT16 const sGridNo, UINT16 const usIndex)
{
	return IndexExistsInLayer(gpWorldLevelData[sGridNo].pRoofHead, usIndex);
}


BOOLEAN RemoveAllRoofsOfTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType)
{
	BOOLEAN fRetVal = FALSE;

	// Look through all Roofs and Search for type
	for (const LEVELNODE* pRoof = gpWorldLevelData[iMapIndex].pRoofHead; pRoof != NULL;)
	{
		if (pRoof->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pRoof->usIndex);

			// Advance to next
			const LEVELNODE* pOldRoof = pRoof;
			pRoof = pRoof->pNext; // XXX TODO0009 if pRoof->usIndex == NO_TILE this is an endless loop

			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				RemoveRoof(iMapIndex, pOldRoof->usIndex);
				fRetVal = TRUE;
			}
		}
	}

	// Could not find it
	return fRetVal;
}


void RemoveRoofIndexFlagsFromTypeRange(UINT32 const iMapIndex, UINT32 const fStartType, UINT32 const fEndType, LevelnodeFlags const uiFlags)
{
	// Look through all Roofs and Search for type
	for (LEVELNODE* pRoof = gpWorldLevelData[iMapIndex].pRoofHead; pRoof != NULL;)
	{
		if (pRoof->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pRoof->usIndex);
			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				pRoof->uiFlags &= ~uiFlags;
			}
			pRoof = pRoof->pNext; // XXX TODO0009 if pRoof->usIndex == NO_TILE this is an endless loop
		}
	}
}


void SetRoofIndexFlagsFromTypeRange(UINT32 const iMapIndex, UINT32 const fStartType, UINT32 const fEndType, LevelnodeFlags const uiFlags)
{
	// Look through all Roofs and Search for type
	for (LEVELNODE* pRoof = gpWorldLevelData[iMapIndex].pRoofHead; pRoof != NULL;)
	{
		if (pRoof->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pRoof->usIndex);
			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				pRoof->uiFlags |= uiFlags;
			}
			pRoof = pRoof->pNext; // XXX TODO0009 if pRoof->usIndex == NO_TILE this is an endless loop
		}
	}
}


// OnRoof layer
// #################################################################

static LEVELNODE* AddOnRoof(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddNodeToWorld(iMapIndex, usIndex, 1);
	ResetSpecificLayerOptimizing(TILES_DYNAMIC_ONROOF);
	return n;
}


LEVELNODE* AddOnRoofToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddOnRoof(iMapIndex, usIndex);

	// Append the node to the list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pOnRoofHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	return n;
}


LEVELNODE* AddOnRoofToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddOnRoof(iMapIndex, usIndex);

	// Prepend the node to the list
	LEVELNODE** const head = &gpWorldLevelData[iMapIndex].pOnRoofHead;
	n->pNext = *head;
	*head = n;

	return n;
}


BOOLEAN RemoveOnRoof(UINT32 iMapIndex, UINT16 usIndex)
{
	LEVELNODE* pOldOnRoof = NULL;

	// Look through all OnRoofs and remove index if found
	for (LEVELNODE* pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead; pOnRoof != NULL; pOnRoof = pOnRoof->pNext)
	{
		if (pOnRoof->usIndex == usIndex)
		{
			// OK, set links
			// Check for head
			if (pOldOnRoof == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pOnRoofHead = pOnRoof->pNext;
			}
			else
			{
				pOldOnRoof->pNext = pOnRoof->pNext;
			}

			delete pOnRoof;
			return TRUE;
		}

		pOldOnRoof = pOnRoof;
	}

	// Could not find it
	return FALSE;
}


BOOLEAN RemoveOnRoofFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode )
{
	LEVELNODE* pOldOnRoof = NULL;

	for (LEVELNODE* pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead; pOnRoof != NULL; pOnRoof = pOnRoof->pNext)
	{
		if (pOnRoof == pNode)
		{
			// OK, set links
			// Check for head
			if (pOldOnRoof == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pOnRoofHead = pOnRoof->pNext;
			}
			else
			{
				pOldOnRoof->pNext = pOnRoof->pNext;
			}

			delete pOnRoof;
			return TRUE;
		}

		pOldOnRoof = pOnRoof;
	}

	// Could not find it
	return FALSE;
}


BOOLEAN RemoveAllOnRoofsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	BOOLEAN fRetVal = FALSE;

	// Look through all OnRoofs and Search for type
	for (const LEVELNODE* pOnRoof = gpWorldLevelData[iMapIndex].pOnRoofHead; pOnRoof != NULL;)
	{
		if (pOnRoof->uiFlags & LEVELNODE_CACHEDANITILE)
		{
			pOnRoof = pOnRoof->pNext;
			continue;
		}

		if (pOnRoof->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pOnRoof->usIndex);

			// Advance to next
			const LEVELNODE* pOldOnRoof = pOnRoof;
			pOnRoof = pOnRoof->pNext; // XXX TODO0009 if pOnRoof->usIndex == NO_TILE this is an endless loop

			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				RemoveOnRoof(iMapIndex, pOldOnRoof->usIndex);
				fRetVal = TRUE;
			}
		}
	}
	return fRetVal;
}


// Topmost layer
// #################################################################

LEVELNODE* AddTopmostToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	// Append node to list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pTopmostHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_TOPMOST);
	return n;
}


LEVELNODE* AddUIElem(UINT32 iMapIndex, UINT16 usIndex, INT8 sRelativeX, INT8 sRelativeY)
{
	LEVELNODE* pTopmost = AddTopmostToTail(iMapIndex, usIndex);

	// Set flags
	pTopmost->uiFlags		|= LEVELNODE_USERELPOS;
	pTopmost->sRelativeX = sRelativeX;
	pTopmost->sRelativeY = sRelativeY;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_TOPMOST);
	return pTopmost;
}


LEVELNODE* AddTopmostToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	// Prepend node to list
	LEVELNODE** const head = &gpWorldLevelData[iMapIndex].pTopmostHead;
	n->pNext = *head;
	*head = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_TOPMOST);
	return n;
}


BOOLEAN RemoveTopmost(UINT32 iMapIndex, UINT16 usIndex)
{
	// Look through all topmosts and remove index if found
	LEVELNODE* pOldTopmost = NULL;
	for (LEVELNODE* pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead; pTopmost != NULL; pTopmost = pTopmost->pNext)
	{
		if (pTopmost->usIndex == usIndex)
		{
			// OK, set links
			// Check for head
			if (pOldTopmost == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pTopmostHead = pTopmost->pNext;
			}
			else
			{
				pOldTopmost->pNext = pTopmost->pNext;
			}

			delete pTopmost;
			return TRUE;
		}

		pOldTopmost = pTopmost;
	}

	// Could not find it
	return FALSE;
}


BOOLEAN RemoveTopmostFromLevelNode(UINT32 iMapIndex, LEVELNODE* pNode)
{
	// Look through all topmosts and remove index if found
	LEVELNODE* pOldTopmost = NULL;
	for (LEVELNODE* pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead; pTopmost != NULL; pTopmost = pTopmost->pNext)
	{
		if (pTopmost == pNode)
		{
			// OK, set links
			// Check for head or tail
			if (pOldTopmost == NULL)
			{
				// It's the head
				gpWorldLevelData[iMapIndex].pTopmostHead = pTopmost->pNext;
			}
			else
			{
				pOldTopmost->pNext = pTopmost->pNext;
			}

			delete pTopmost;
			return TRUE;
		}

		pOldTopmost = pTopmost;
	}

	// Could not find it
	return FALSE;
}


BOOLEAN RemoveAllTopmostsOfTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType)
{
	BOOLEAN fRetVal = FALSE;

	// Look through all topmosts and Search for type
	for (const LEVELNODE* pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead; pTopmost != NULL;)
	{
		const LEVELNODE* pOldTopmost = pTopmost;
		pTopmost = pTopmost->pNext;

		if (pOldTopmost->usIndex != NO_TILE && pOldTopmost->usIndex < NUMBEROFTILES)
		{
			const UINT32 fTileType = GetTileType(pOldTopmost->usIndex);
			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				// Remove Item
				RemoveTopmost(iMapIndex, pOldTopmost->usIndex);
				fRetVal = TRUE;
			}
		}
	}
	return fRetVal;
}


LEVELNODE* FindTypeInTopmostLayer(UINT32 const map_idx, UINT32 const type)
{
	return FindTypeInLayer(gpWorldLevelData[map_idx].pTopmostHead, type);
}


BOOLEAN IsHeigherLevel(INT16 sGridNo)
{
	const STRUCTURE* pStructure = FindStructure(sGridNo, STRUCTURE_NORMAL_ROOF);
	return pStructure != NULL;
}


BOOLEAN IsRoofVisible(INT16 sMapPos)
{
	if (gfBasement) return TRUE;

	const STRUCTURE* pStructure = FindStructure(sMapPos, STRUCTURE_ROOF);
	return
		pStructure != NULL &&
		!(gpWorldLevelData[sMapPos].uiFlags & MAPELEMENT_REVEALED);
}


BOOLEAN IsRoofVisible2(INT16 sMapPos)
{
	if (!gfBasement)
	{
		const STRUCTURE* pStructure = FindStructure(sMapPos, STRUCTURE_ROOF);
		if (pStructure == NULL) return FALSE;
	}

	return !(gpWorldLevelData[sMapPos].uiFlags & MAPELEMENT_REVEALED);
}


SOLDIERTYPE* WhoIsThere2(INT16 const gridno, INT8 const level)
{
	if (!GridNoOnVisibleWorldTile(gridno)) return NULL;

	for (STRUCTURE const* structure = gpWorldLevelData[gridno].pStructureHead; structure; structure = structure->pNext)
	{
		if (!(structure->fFlags & STRUCTURE_PERSON)) continue;

		SOLDIERTYPE& tgt = GetMan(structure->usStructureID);
		// person must either have their pSoldier->sGridNo here or be non-passable
		if (structure->fFlags & STRUCTURE_PASSABLE && tgt.sGridNo != gridno) continue;

		if ((level == 0 && structure->sCubeOffset == 0) ||
				(level >  0 && structure->sCubeOffset >  0))
		{
			// found a person, on the right level!
			// structure ID and merc ID are identical for merc structures
			return &tgt;
		}
	}

	return NULL;
}


UINT8	GetTerrainType(GridNo const grid_no)
{
	return gpWorldLevelData[grid_no].ubTerrainID;
}


bool Water(GridNo const grid_no)
{
	if (grid_no == NOWHERE) return false;

	UINT8 const terrain = GetTerrainType(grid_no);
	return
		terrain == LOW_WATER ||
		terrain == MED_WATER ||
		terrain == DEEP_WATER;
}


bool DeepWater(GridNo const grid_no)
{
	return GetTerrainType(grid_no) == DEEP_WATER;
}


bool WaterTooDeepForAttacks(GridNo const grid_no)
{
	return DeepWater(grid_no);
}


void SetStructAframeFlags(UINT32 const iMapIndex, LevelnodeFlags const uiFlags)
{
	// Look through all Roofs and Search for type
	for (LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pRoofHead; pStruct != NULL;)
	{
		if ( pStruct->usIndex != NO_TILE )
		{
			if (GetTileFlags(pStruct->usIndex) & AFRAME_TILE)
			{
				pStruct->uiFlags |= uiFlags;
			}
			pStruct = pStruct->pNext; // XXX TODO0009 if pStruct->usIndex == NO_TILE this is an endless loop
		}
	}
}


LEVELNODE* FindLevelNodeBasedOnStructure(STRUCTURE const* const s)
{
	Assert(s->fFlags & STRUCTURE_BASE_TILE);
	MAP_ELEMENT const& me = gpWorldLevelData[s->sGridNo];

	// ATE: First look on the struct layer
	for (LEVELNODE* i = me.pStructHead; i; i = i->pNext)
	{
		if (i->pStructureData == s) return i;
	}

	// Next the roof layer
	for (LEVELNODE* i = me.pRoofHead; i; i = i->pNext)
	{
		if (i->pStructureData == s) return i;
	}

	// Then the object layer
	for (LEVELNODE* i = me.pObjectHead; i; i = i->pNext)
	{
		if (i->pStructureData == s) return i;
	}

	// Finally the onroof layer
	for (LEVELNODE* i = me.pOnRoofHead; i; i = i->pNext)
	{
		if (i->pStructureData == s) return i;
	}

	throw std::logic_error("FindLevelNodeBasedOnStruct failed");
}


LEVELNODE* FindShadow(INT16 sGridNo, UINT16 usStructIndex)
{
	if (usStructIndex < FIRSTOSTRUCT1 || usStructIndex >= FIRSTSHADOW1)
	{
		return NULL;
	}

	UINT16 usShadowIndex = usStructIndex - FIRSTOSTRUCT1 + FIRSTSHADOW1;
	LEVELNODE* pLevelNode;
	for (pLevelNode = gpWorldLevelData[sGridNo].pShadowHead; pLevelNode != NULL; pLevelNode = pLevelNode->pNext)
	{
		if (pLevelNode->usIndex == usShadowIndex)
		{
			break;
		}
	}
	return pLevelNode;
}


void WorldHideTrees(void)
{
	FOR_EACH_WORLD_TILE(i)
	{
		for (LEVELNODE* pNode = i->pStructHead; pNode != NULL; pNode = pNode->pNext)
		{
			if (pNode->uiFlags & LEVELNODE_ANIMATION) continue;
			if (GetTileFlags(pNode->usIndex) & FULL3D_TILE)
			{
				pNode->uiFlags |= LEVELNODE_REVEALTREES;
			}
		}
	}

	SetRenderFlags(RENDER_FLAG_FULL);
}


void WorldShowTrees(void)
{
	FOR_EACH_WORLD_TILE(i)
	{
		for (LEVELNODE* pNode = i->pStructHead; pNode != NULL; pNode = pNode->pNext)
		{
			if (pNode->uiFlags & LEVELNODE_ANIMATION) continue;
			if (GetTileFlags(pNode->usIndex) & FULL3D_TILE)
			{
				pNode->uiFlags &= ~LEVELNODE_REVEALTREES;
			}
		}
	}

	SetRenderFlags(RENDER_FLAG_FULL);
}


void SetWallLevelnodeFlags(UINT16 const sGridNo, LevelnodeFlags const uiFlags)
{
	for (LEVELNODE* pStruct = gpWorldLevelData[sGridNo].pStructHead; pStruct != NULL; pStruct = pStruct->pNext)
	{
		if (pStruct->pStructureData != NULL &&
				pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF) // See if we are a wall!
		{
			pStruct->uiFlags |= uiFlags;
		}
	}
}


void RemoveWallLevelnodeFlags(UINT16 const sGridNo, LevelnodeFlags const uiFlags)
{
	for (LEVELNODE* pStruct = gpWorldLevelData[sGridNo].pStructHead; pStruct != NULL; pStruct = pStruct->pNext)
	{
		if (pStruct->pStructureData != NULL &&
				pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF) // See if we are a wall!
		{
			pStruct->uiFlags &= ~uiFlags;
		}
	}
}


void SetTreeTopStateForMap(void)
{
	if (!gGameSettings.fOptions[TOPTION_TOGGLE_TREE_TOPS])
	{
		WorldHideTrees();
		gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
	}
	else
	{
		WorldShowTrees();
		gTacticalStatus.uiFlags &= ~NOHIDE_REDUNDENCY;
	}

	// FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
	InvalidateWorldRedundency();
}
