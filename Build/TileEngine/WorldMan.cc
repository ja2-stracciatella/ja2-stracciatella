#include <stdexcept>

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


static UINT32 guiLNCount[9];
static const wchar_t gzLevelString[][15] =
{
	L"",
	L"Land    %d",
	L"Object  %d",
	L"Struct  %d",
	L"Shadow  %d",
	L"Merc    %d",
	L"Roof    %d",
	L"Onroof  %d",
	L"Topmost %d",
};


// LEVEL NODE MANIPLULATION FUNCTIONS
static LEVELNODE* CreateLevelNode(void)
{
	LEVELNODE* const Node = MALLOCZ(LEVELNODE);
	Node->ubShadeLevel        = LightGetAmbient();
	Node->ubNaturalShadeLevel = LightGetAmbient();
	Node->pSoldier            = NULL;
	Node->pNext               = NULL;
	Node->sRelativeX          = 0;
	Node->sRelativeY          = 0;
	return Node;
}


void CountLevelNodes(void)
{
	for (UINT32 uiLoop2 = 0; uiLoop2 < 9; uiLoop2++)
	{
		guiLNCount[uiLoop2] = 0;
	}

	FOR_ALL_WORLD_TILES(pME)
	{
		// start at 1 to skip land head ptr; 0 stores total
		for (UINT32 uiLoop2 = 1; uiLoop2 < 9; uiLoop2++)
		{
			for (const LEVELNODE* pLN = pME->pLevelNodes[uiLoop2]; pLN != NULL; pLN = pLN->pNext)
			{
				guiLNCount[uiLoop2]++;
				guiLNCount[0]++;
			}
		}
	}
}


#define LINE_HEIGHT 20
void DebugLevelNodePage(void)
{
	SetFont(LARGEFONT1);
	gprintf(0, 0, L"DEBUG LEVELNODES PAGE 1 OF 1");

	for (UINT32 uiLoop = 1; uiLoop < 9; uiLoop++)
	{
		gprintf(0, LINE_HEIGHT * (uiLoop + 1), gzLevelString[uiLoop], guiLNCount[uiLoop]);
	}
	gprintf(0, LINE_HEIGHT * 12, L"%d land nodes in excess of world max (25600)", guiLNCount[1] - WORLD_MAX);
	gprintf(0, LINE_HEIGHT * 13, L"Total # levelnodes %d, %d bytes each", guiLNCount[0], sizeof(LEVELNODE));
	gprintf(0, LINE_HEIGHT * 14, L"Total memory for levelnodes %d", guiLNCount[0] * sizeof(LEVELNODE));
}


static BOOLEAN TypeExistsInLevel(const LEVELNODE* pStartNode, UINT32 fType, UINT16* pusIndex)
{
	// Look through all objects and Search for type
	for (; pStartNode != NULL; pStartNode = pStartNode->pNext)
	{
		if (pStartNode->usIndex != NO_TILE && pStartNode->usIndex < NUMBEROFTILES)
		{
			const UINT32 fTileType = GetTileType(pStartNode->usIndex);
			if (fTileType == fType)
			{
				if (pusIndex) *pusIndex = pStartNode->usIndex;
				return TRUE;
			}
		}
	}

	// Could not find it
	return FALSE;
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

			MemFree(pObject);

			//Add the index to the maps temp file so we can remove it after reloading the map
			AddRemoveObjectToMapTempFile(iMapIndex, usIndex);

			return TRUE;
		}

		pOldObject = pObject;
	}

	// Could not find it
	return FALSE;
}


UINT16 TypeRangeExistsInObjectLayer(UINT32 const iMapIndex, UINT32 const fStartType, UINT32 const fEndType)
{
	// Look through all objects and Search for type
	for (const LEVELNODE* pObject = gpWorldLevelData[iMapIndex].pObjectHead; pObject != NULL; pObject = pObject->pNext)
	{
		if (pObject->usIndex == NO_TILE || pObject->usIndex >= NUMBEROFTILES) continue;

		UINT32 const fTileType = GetTileType(pObject->usIndex);
		if (fTileType < fStartType || fEndType < fTileType) continue;

		return pObject->usIndex;
	}

	// Could not find it
	return NO_TILE;
}


BOOLEAN TypeExistsInObjectLayer(UINT32 iMapIndex, UINT32 fType, UINT16* pusObjectIndex)
{
	const LEVELNODE* pObject = gpWorldLevelData[iMapIndex].pObjectHead;
	return TypeExistsInLevel(pObject, fType, pusObjectIndex);
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

#ifdef JA2EDITOR

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

#endif


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


#ifdef JA2EDITOR

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

			MemFree(pLand);
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


BOOLEAN TypeExistsInLandLayer(UINT32 iMapIndex, UINT32 fType, UINT16* pusLandIndex)
{
	const LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead;
	return TypeExistsInLevel(pLand, fType, pusLandIndex);
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


void RemoveHigherLandLevels(UINT32 const iMapIndex, UINT32 const fSrcType, UINT32** const puiHigherTypes, UINT8* const pubNumHigherTypes)
{
	LEVELNODE* pOldLand = NULL;

	*pubNumHigherTypes = 0;
	*puiHigherTypes = NULL;

	// Start at tail and up
	LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead;

	// Get tail
	while (pLand != NULL)
	{
		pOldLand = pLand;
		pLand = pLand->pNext;
	}

	pLand = pOldLand;

	// Get src height
	UINT8 ubSrcLogHeight = GetTileTypeLogicalHeight(fSrcType);

	// Look through all objects and Search for height
	while (pLand != NULL)
	{
		const UINT32 fTileType = GetTileType(pLand->usIndex);

		// Advance to next
		pOldLand = pLand;
		pLand = pLand->pPrevNode;

		if (gTileTypeLogicalHeight[fTileType] > ubSrcLogHeight)
		{
			RemoveLand(iMapIndex, pOldLand->usIndex);

			++*pubNumHigherTypes;
			*puiHigherTypes = REALLOC(*puiHigherTypes, UINT32, *pubNumHigherTypes);
			(*puiHigherTypes)[*pubNumHigherTypes - 1] = fTileType;
		}
	}

	AdjustForFullTile(iMapIndex);
}

#endif


static LEVELNODE* AddNodeToWorld(UINT32 const iMapIndex, UINT16 const usIndex, INT8 const level)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;

	if (usIndex >= NUMBEROFTILES) return n;

	const DB_STRUCTURE_REF* const sr = gTileDatabase[usIndex].pDBStructureRef;
	if (!sr) return n;

	if (AddStructureToWorld(iMapIndex, level, sr, n)) return n;

	MemFree(n);
	throw FailedToAddNode();
}


// Struct layer
// #################################################################

static LEVELNODE* AddStructToTailCommon(UINT32 const iMapIndex, UINT16 const usIndex, LEVELNODE* const n)
{
	// Append node to list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pStructHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	if (usIndex < NUMBEROFTILES)
	{
		// Check flags for tiledat and set a shadow if we have a buddy
		if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[usIndex].sBuddyNum != -1)
		{
			LEVELNODE* const n = AddShadowToHead(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
			n->uiFlags |= LEVELNODE_BUDDYSHADOW;
		}

		// Check for special flag to stop burn-through on same-tile structs...
		const DB_STRUCTURE_REF* const sr = gTileDatabase[usIndex].pDBStructureRef;
		if (sr != NULL)
		{
			// If we are NOT a wall and NOT multi-tiles, set mapelement flag...
			if (!FindStructure(iMapIndex, STRUCTURE_WALLSTUFF) && sr->pDBStructure->ubNumberOfTiles == 1) // XXX TODO0015
			{
				gpWorldLevelData[iMapIndex].ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
			}
			else
			{
				gpWorldLevelData[iMapIndex].ubExtFlags[0] &= ~MAPELEMENT_EXT_NOBURN_STRUCT;
			}
		}
	}

	AddStructToMapTempFile(iMapIndex, usIndex);

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
	return n;
}


LEVELNODE* AddStructToTail(UINT32 iMapIndex, UINT16 usIndex)
{
	LEVELNODE* const n = AddNodeToWorld(iMapIndex, usIndex, 0);
	return AddStructToTailCommon(iMapIndex, usIndex, n);
}


LEVELNODE* ForceStructToTail(UINT32 iMapIndex, UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	n->usIndex = usIndex;
	return AddStructToTailCommon(iMapIndex, usIndex, n);
}


void AddStructToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddNodeToWorld(iMapIndex, usIndex, 0);

	// Prepend node to list
	LEVELNODE** const head = &gpWorldLevelData[iMapIndex].pStructHead;
	n->pNext = *head;
	*head = n;

	if (usIndex < NUMBEROFTILES)
	{
		// Check flags for tiledat and set a shadow if we have a buddy
		if (!GridNoIndoors(iMapIndex) && gTileDatabase[usIndex].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[usIndex].sBuddyNum != -1)
		{
			LEVELNODE* const n = AddShadowToHead(iMapIndex, gTileDatabase[usIndex].sBuddyNum);
			n->uiFlags |= LEVELNODE_BUDDYSHADOW;
		}

		//Check for special flag to stop burn-through on same-tile structs...
		const DB_STRUCTURE_REF* const sr = gTileDatabase[usIndex].pDBStructureRef;
		if (sr != NULL)
		{
			// If we are NOT a wall and NOT multi-tiles, set mapelement flag...
			if (FindStructure(iMapIndex, STRUCTURE_WALLSTUFF) != NULL && sr->pDBStructure->ubNumberOfTiles == 1) // XXX TODO0015
			{
				gpWorldLevelData[iMapIndex].ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
			}
			else
			{
				gpWorldLevelData[iMapIndex].ubExtFlags[0] &= ~MAPELEMENT_EXT_NOBURN_STRUCT;
			}
		}
	}

	AddStructToMapTempFile(iMapIndex, usIndex);

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


#ifdef JA2EDITOR

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

			MemFree(pStruct);

			RemoveShadowBuddy(iMapIndex, usIndex);
			return;
		}

		pPrevStruct = pStruct;
	}
}

#endif


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
	MemFree(removee);
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
			if ((wall_orientation  == INSIDE_TOP_RIGHT || wall_orientation  == OUTSIDE_TOP_RIGHT) &&
					(check_wall_orient == INSIDE_TOP_LEFT  || check_wall_orient == OUTSIDE_TOP_LEFT) ||
					(wall_orientation  == INSIDE_TOP_LEFT  || wall_orientation  == OUTSIDE_TOP_LEFT) &&
					(check_wall_orient == INSIDE_TOP_RIGHT || check_wall_orient == OUTSIDE_TOP_RIGHT))
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
		if ((wall_orientation  == INSIDE_TOP_RIGHT || wall_orientation  == OUTSIDE_TOP_RIGHT) &&
			  (check_wall_orient == INSIDE_TOP_RIGHT || check_wall_orient == OUTSIDE_TOP_RIGHT) ||
				(wall_orientation  == INSIDE_TOP_LEFT  || wall_orientation  == OUTSIDE_TOP_LEFT) &&
			  (check_wall_orient == INSIDE_TOP_LEFT  || check_wall_orient == OUTSIDE_TOP_LEFT))
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
//	  undesirable effect with walls and buildings so I added this function to make
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

			MemFree(pShadow);
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

			MemFree(pShadow);
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


LEVELNODE* AddMercToHead(const UINT32 iMapIndex, SOLDIERTYPE* const pSoldier, const BOOLEAN fAddStructInfo)
{
	LEVELNODE* pMerc = gpWorldLevelData[iMapIndex].pMercHead;

	LEVELNODE* pNextMerc = CreateLevelNode();
	pNextMerc->pNext = pMerc;
	pNextMerc->pSoldier = pSoldier;
	pNextMerc->uiFlags |= LEVELNODE_SOLDIER;

	// Add structure info if we want
	if (fAddStructInfo)
	{
		// Set soldier's levelnode
		pSoldier->pLevelNode = pNextMerc;
		AddMercStructureInfo(iMapIndex, pSoldier);
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
		ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"FAILED: add struct info for merc %d (%ls), at %d direction %d", s->ubID, s->name, sGridNo, s->bDirection);
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


BOOLEAN RemoveMerc(UINT32 iMapIndex, SOLDIERTYPE* pSoldier, BOOLEAN fPlaceHolder)
{
	if (iMapIndex == NOWHERE)
	{
		return FALSE;
	}

	LEVELNODE* pOldMerc = NULL;
	for (LEVELNODE* pMerc = gpWorldLevelData[iMapIndex].pMercHead; pMerc != NULL; pMerc = pMerc->pNext)
	{
		if (pMerc->pSoldier == pSoldier)
		{
			// If it's a placeholder, check!
			BOOLEAN fMercFound = FALSE;
			if (fPlaceHolder)
			{
				if (pMerc->uiFlags & LEVELNODE_MERCPLACEHOLDER)
				{
					fMercFound = TRUE;
				}
			}
			else
			{
				if (!(pMerc->uiFlags & LEVELNODE_MERCPLACEHOLDER))
				{
					fMercFound = TRUE;
				}
			}

			if (fMercFound)
			{
				// OK, set links
				// Check for head or tail
				if (pOldMerc == NULL)
				{
					// It's the head
					gpWorldLevelData[iMapIndex].pMercHead = pMerc->pNext;
				}
				else
				{
					pOldMerc->pNext = pMerc->pNext;
				}

				if (!fPlaceHolder)
				{
					// Set level node to NULL
					pSoldier->pLevelNode = NULL;

					// Remove strucute info!
					DeleteStructureFromWorld(pMerc->pStructureData);
				}

				MemFree(pMerc);
				return TRUE;
			}
		}

		pOldMerc = pMerc;
	}

	// Could not find it
	return FALSE;
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
			MemFree(pRoof);
			return TRUE;
		}

		pOldRoof = pRoof;
	}

	// Could not find it
	return FALSE;
}


BOOLEAN TypeExistsInRoofLayer( UINT32 iMapIndex, UINT32 fType, UINT16 *pusRoofIndex )
{
	const LEVELNODE* pRoof = gpWorldLevelData[iMapIndex].pRoofHead;
	return TypeExistsInLevel(pRoof, fType, pusRoofIndex);
}


BOOLEAN TypeRangeExistsInRoofLayer(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT16* pusRoofIndex)
{
	// Look through all objects and Search for type
	for (const LEVELNODE* pRoof = gpWorldLevelData[iMapIndex].pRoofHead; pRoof != NULL;)
	{
		if (pRoof->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pRoof->usIndex);

			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				*pusRoofIndex = pRoof->usIndex;
				return TRUE;
			}
			pRoof = pRoof->pNext; // XXX TODO0009 if pRoof->usIndex == NO_TILE this is an endless loop
		}
	}

	// Could not find it
	return FALSE;
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


void RemoveRoofIndexFlagsFromTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT32 uiFlags)
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


void SetRoofIndexFlagsFromTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT32 uiFlags)
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

			MemFree(pOnRoof);
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

			MemFree(pOnRoof);
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

			MemFree(pTopmost);
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

			MemFree(pTopmost);
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


BOOLEAN TypeExistsInTopmostLayer(UINT32 iMapIndex, UINT32 fType, UINT16* pusTopmostIndex)
{
	const LEVELNODE* pTopmost = gpWorldLevelData[iMapIndex].pTopmostHead;
	return TypeExistsInLevel(pTopmost, fType, pusTopmostIndex);
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

		SOLDIERTYPE* const tgt = GetMan(structure->usStructureID);
		// person must either have their pSoldier->sGridNo here or be non-passable
		if (structure->fFlags & STRUCTURE_PASSABLE && tgt->sGridNo != gridno) continue;

		if ((level == 0 && structure->sCubeOffset == 0) ||
				(level >  0 && structure->sCubeOffset >  0))
		{
			// found a person, on the right level!
			// structure ID and merc ID are identical for merc structures
			return tgt;
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


void SetStructAframeFlags(UINT32 iMapIndex, UINT32 uiFlags)
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


LEVELNODE* FindLevelNodeBasedOnStructure(INT16 const grid_no, STRUCTURE* const s)
{
	MAP_ELEMENT const& me = gpWorldLevelData[grid_no];

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
	FOR_ALL_WORLD_TILES(i)
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
	FOR_ALL_WORLD_TILES(i)
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


void SetWallLevelnodeFlags( UINT16 sGridNo, UINT32 uiFlags )
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


void RemoveWallLevelnodeFlags(UINT16 sGridNo, UINT32 uiFlags)
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
