#include "Animation_Data.h"
#include "Environment.h"
#include "Font.h"
#include "Structure.h"
#include "WorldDef.h"
#include "WCheck.h"
#include "Debug.h"
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
	CHECKN(Node != NULL);

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

	for (UINT32 uiLoop = 0; uiLoop < WORLD_MAX; uiLoop++)
	{
		const MAP_ELEMENT* pME = &gpWorldLevelData[uiLoop];
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
				*pusIndex = pStartNode->usIndex;
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
	CHECKF(n != NULL);
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
	CHECKN(n != NULL);
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


BOOLEAN TypeRangeExistsInObjectLayer( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT16 *pusObjectIndex )
{
	// Look through all objects and Search for type
	for (const LEVELNODE* pObject = gpWorldLevelData[iMapIndex].pObjectHead; pObject != NULL; pObject = pObject->pNext)
	{
		if (pObject->usIndex != NO_TILE && pObject->usIndex < NUMBEROFTILES)
		{
			const UINT32 fTileType = GetTileType(pObject->usIndex);
			if (fTileType >= fStartType && fTileType <= fEndType)
			{
				*pusObjectIndex = pObject->usIndex;
				return TRUE;
			}
		}
	}

	// Could not find it
	return FALSE;
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

LEVELNODE* AddLandToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	CHECKF(n != NULL);
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


BOOLEAN AddLandToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	CHECKF(n != NULL);
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
	return TRUE;
}


static BOOLEAN AdjustForFullTile(UINT32 iMapIndex);
static void RemoveLandEx(UINT32 iMapIndex, UINT16 usIndex);


BOOLEAN RemoveLand(UINT32 iMapIndex, UINT16 usIndex)
{
	RemoveLandEx(iMapIndex, usIndex);
	AdjustForFullTile(iMapIndex);
	return FALSE;
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


BOOLEAN ReplaceLandIndex(UINT32 iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex)
{
	// Look through all Lands and remove index if found
	for (LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead; pLand != NULL; pLand = pLand->pNext)
	{
		if (pLand->usIndex == usOldIndex)
		{
			// OK, set new index value
			pLand->usIndex = usNewIndex;
			AdjustForFullTile(iMapIndex);
			return TRUE;
		}
	}

	// Could not find it
	return FALSE;
}


BOOLEAN TypeExistsInLandLayer(UINT32 iMapIndex, UINT32 fType, UINT16* pusLandIndex)
{
	const LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead;
	return TypeExistsInLevel(pLand, fType, pusLandIndex);
}


#ifdef JA2EDITOR

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

#endif


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


BOOLEAN DeleteAllLandLayers(UINT32 iMapIndex)
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

	return TRUE;
}


#ifdef JA2EDITOR

BOOLEAN InsertLandIndexAtLevel(const UINT32 iMapIndex, const UINT16 usIndex, const UINT8 ubLevel)
{
	// If we want to insert at head;
	if (ubLevel == 0)
	{
		AddLandToHead(iMapIndex, usIndex);
		return TRUE;
	}

	// Move to index before insertion
	LEVELNODE* pLand = gpWorldLevelData[iMapIndex].pLandHead;
	for (UINT8 level = 0;; ++level)
	{
		if (pLand == NULL) return FALSE;

		if (level == ubLevel - 1) break;

		pLand = pLand->pNext;
	}

	LEVELNODE* const n = CreateLevelNode();
	CHECKF(n != NULL);
	n->usIndex = usIndex;

	// Set links, according to position!
	n->pPrevNode = pLand;
	n->pNext     = pLand->pNext;
	pLand->pNext = n;

	// Check for tail
	if (n->pNext != NULL) n->pNext->pPrevNode = n;

	AdjustForFullTile(iMapIndex);

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_LAND);
	return TRUE;
}

#endif


BOOLEAN RemoveHigherLandLevels(UINT32 iMapIndex, UINT32 fSrcType, UINT32** puiHigherTypes, UINT8* pubNumHigherTypes)
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
	return TRUE;
}


static BOOLEAN AddNodeToWorld(const UINT32 iMapIndex, const UINT16 usIndex, const INT8 level, LEVELNODE* const n)
{
	if (usIndex >= NUMBEROFTILES) return TRUE;

	const DB_STRUCTURE_REF* const sr = gTileDatabase[usIndex].pDBStructureRef;
	if (sr == NULL) return TRUE;

	if (AddStructureToWorld(iMapIndex, level, sr, n)) return TRUE;

	MemFree(n);
	return FALSE;
}


// Struct layer
// #################################################################

static LEVELNODE* AddStructToTailCommon(UINT32 iMapIndex, UINT16 usIndex, BOOLEAN fAddStructDBInfo);


LEVELNODE* AddStructToTail(UINT32 iMapIndex, UINT16 usIndex)
{
	return AddStructToTailCommon(iMapIndex, usIndex, TRUE);
}


LEVELNODE* ForceStructToTail(UINT32 iMapIndex, UINT16 usIndex)
{
	return AddStructToTailCommon(iMapIndex, usIndex, FALSE);
}


static LEVELNODE* AddStructToTailCommon(const UINT32 iMapIndex, const UINT16 usIndex, const BOOLEAN fAddStructDBInfo)
{
	LEVELNODE* const n = CreateLevelNode();
	CHECKN(n != NULL);

	if (fAddStructDBInfo && !AddNodeToWorld(iMapIndex, usIndex, 0, n)) return NULL;

	n->usIndex = usIndex;

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


BOOLEAN AddStructToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	CHECKF(n != NULL);

	if (!AddNodeToWorld(iMapIndex, usIndex, 0, n)) return FALSE;

	n->usIndex = usIndex;

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
	return TRUE;
}


static BOOLEAN InsertStructIndex(const UINT32 iMapIndex, const UINT16 usIndex, const UINT8 ubLevel)
{
	// If we want to insert at head
	if (ubLevel == 0)
	{
		return AddStructToHead(iMapIndex, usIndex);
	}

	// Move to index before insertion
	LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead;
	for (UINT8 level = 0;; ++level)
	{
		if (pStruct == NULL) return FALSE;

		if (level == ubLevel - 1) break;

		pStruct = pStruct->pNext;
	}

	LEVELNODE* const n = CreateLevelNode();
	CHECKF(n != NULL);

	n->usIndex = usIndex;

	if (!AddNodeToWorld(iMapIndex, usIndex, 0, n)) return FALSE;

	// Set links, according to position!
	n->pNext       = pStruct->pNext;
	pStruct->pNext = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
	return TRUE;
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


BOOLEAN ForceRemoveStructFromTail(UINT32 iMapIndex)
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
			return TRUE;
		}

		pPrevStruct = pStruct;
	}

	return TRUE;
}


static BOOLEAN InternalRemoveStruct(UINT32 MapIndex, LEVELNODE* Pred, LEVELNODE* Removee, UINT16 Index)
{
	if (Pred == NULL)
	{
		gpWorldLevelData[MapIndex].pStructHead = Removee->pNext;
	}
	else
	{
		Pred->pNext = Removee->pNext;
	}

	// Delete memory assosiated with item
	DeleteStructureFromWorld(Removee->pStructureData);

	//If we have to, make sure to remove this node when we reload the map from a saved game
	RemoveStructFromMapTempFile(MapIndex, Index);

	RemoveShadowBuddy(MapIndex, Index);
	MemFree(Removee);

	return TRUE;
}


BOOLEAN RemoveStruct(UINT32 iMapIndex, UINT16 usIndex)
{
	LEVELNODE* pOldStruct = NULL;

	// Look through all structs and remove index if found
	for (LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead; pStruct != NULL; pStruct = pStruct->pNext)
	{
		if (pStruct->usIndex == usIndex)
		{
			return InternalRemoveStruct(iMapIndex, pOldStruct, pStruct, usIndex);
		}

		pOldStruct = pStruct;
	}

	return FALSE;
}


BOOLEAN RemoveStructFromLevelNode(UINT32 iMapIndex, LEVELNODE* pNode)
{
	LEVELNODE* pOldStruct = NULL;

	// Look through all structs and remove index if found
	for (LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead; pStruct != NULL; pStruct = pStruct->pNext)
	{
		if (pStruct == pNode)
		{
			return InternalRemoveStruct(iMapIndex, pOldStruct, pStruct, pNode->usIndex);
		}

		pOldStruct = pStruct;
	}

	return FALSE;
}


BOOLEAN RemoveAllStructsOfTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType)
{
	BOOLEAN fRetVal = FALSE;

	// Look through all structs and Search for type
	for (const LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead; pStruct != NULL;)
	{
		if (pStruct->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pStruct->usIndex);

			// Advance to next
			const LEVELNODE* pOldStruct = pStruct;
			pStruct = pStruct->pNext;

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
BOOLEAN AddWallToStructLayer(INT32 iMapIndex, UINT16 usIndex, BOOLEAN fReplace)
{
	LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead;
	BOOLEAN				fInsertFound = FALSE;
	BOOLEAN				fRoofFound = FALSE;
	UINT8					ubRoofLevel=0;
	UINT8					ubLevel = 0;

	// Get orientation of piece we want to add
	UINT16 usWallOrientation = GetWallOrientation(usIndex);

	// Look through all objects and Search for orientation
	while (pStruct != NULL)
	{
		UINT16 usCheckWallOrient = GetWallOrientation(pStruct->usIndex);
		//OLD CASE
		//if ( usCheckWallOrient > usWallOrientation )
		//Kris:
		//New case -- If placing a new wall which is at right angles to the current wall, then
		//we insert it.
		if (usCheckWallOrient > usWallOrientation)
		{
			if ((usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) &&
					(usCheckWallOrient == INSIDE_TOP_LEFT  || usCheckWallOrient == OUTSIDE_TOP_LEFT) ||
					(usWallOrientation == INSIDE_TOP_LEFT  || usWallOrientation == OUTSIDE_TOP_LEFT) &&
					(usCheckWallOrient == INSIDE_TOP_RIGHT || usCheckWallOrient == OUTSIDE_TOP_RIGHT))
			{
				fInsertFound = TRUE;
			}
		}

		const UINT32 uiCheckType = GetTileType(pStruct->usIndex);

//		if (uiCheckType >= FIRSTFLOOR && uiCheckType <= LASTFLOOR)
		if (uiCheckType >= FIRSTROOF && uiCheckType <= LASTROOF)
		{
			fRoofFound = TRUE;
			ubRoofLevel = ubLevel;
		}

		//OLD CHECK
		// Check if it's the same orientation
		//if ( usCheckWallOrient == usWallOrientation )
		//Kris:
		//New check -- we want to check for walls being parallel to each other.  If so, then
		//we we want to replace it.  This is because of an existing problem with say, INSIDE_TOP_LEFT
		//and OUTSIDE_TOP_LEFT walls coexisting.
		if ((usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) &&
			  (usCheckWallOrient == INSIDE_TOP_RIGHT || usCheckWallOrient == OUTSIDE_TOP_RIGHT) ||
				(usWallOrientation == INSIDE_TOP_LEFT  || usWallOrientation == OUTSIDE_TOP_LEFT) &&
			  (usCheckWallOrient == INSIDE_TOP_LEFT  || usCheckWallOrient == OUTSIDE_TOP_LEFT))
		{
			// Same, if replace, replace here
			if (fReplace)
			{
				return ReplaceStructIndex(iMapIndex, pStruct->usIndex, usIndex);
			}
			else
			{
				return FALSE;
			}
		}

		// Advance to next
		pStruct = pStruct->pNext;

		ubLevel++;
	}

	// Check if we found an insert position, otherwise set to head
	if (fInsertFound)
	{
		// Insert struct at head
		AddStructToHead(iMapIndex, usIndex);
	}
	else
	{
		// Make sure it's ALWAYS after the roof ( if any )
		if (fRoofFound)
		{
			InsertStructIndex(iMapIndex, usIndex, ubRoofLevel);
		}
		else
		{
			AddStructToTail(iMapIndex, usIndex);
		}
	}

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_STRUCTURES);
	// Could not find it, return FALSE
	return TRUE; // XXX code and comment disagree
}


BOOLEAN TypeExistsInStructLayer(UINT32 iMapIndex, UINT32 fType, UINT16* pusStructIndex)
{
	const LEVELNODE* pStruct = gpWorldLevelData[iMapIndex].pStructHead;
	return TypeExistsInLevel(pStruct, fType, pusStructIndex);
}


BOOLEAN HideStructOfGivenType(UINT32 iMapIndex, UINT32 fType, BOOLEAN fHide)
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
	return TRUE;
}


// Shadow layer
// #################################################################

BOOLEAN AddShadowToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = CreateLevelNode();
	CHECKF(n != NULL);
	n->usIndex = usIndex;

	// Append node to list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pShadowHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	ResetSpecificLayerOptimizing(TILES_DYNAMIC_SHADOWS);
	return TRUE;
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
	CHECKN(n != NULL);
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


static BOOLEAN AddMercStructureInfo(INT16 sGridNo, SOLDIERTYPE* pSoldier);


LEVELNODE* AddMercToHead(const UINT32 iMapIndex, SOLDIERTYPE* const pSoldier, const BOOLEAN fAddStructInfo)
{
	LEVELNODE* pMerc = gpWorldLevelData[iMapIndex].pMercHead;

	LEVELNODE* pNextMerc = CreateLevelNode();
	CHECKN(pNextMerc != NULL);
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


static BOOLEAN AddMercStructureInfo(INT16 sGridNo, SOLDIERTYPE* pSoldier)
{
	UINT16 usAnimSurface = GetSoldierAnimationSurface(pSoldier, pSoldier->usAnimState);
	AddMercStructureInfoFromAnimSurface(sGridNo, pSoldier, usAnimSurface, pSoldier->usAnimState);
	return TRUE;
}


BOOLEAN AddMercStructureInfoFromAnimSurface(INT16 sGridNo, SOLDIERTYPE *pSoldier, UINT16 usAnimSurface, UINT16 usAnimState)
{
	pSoldier->uiStatusFlags &= ~SOLDIER_MULTITILE;

	if (pSoldier->pLevelNode == NULL ||
			usAnimSurface == INVALID_ANIMATION_SURFACE)
	{
		return FALSE;
	}

	// Remove existing structs
	DeleteStructureFromWorld(pSoldier->pLevelNode->pStructureData);
	pSoldier->pLevelNode->pStructureData = NULL;

	// Now check if we are multi-tiled!
	const STRUCTURE_FILE_REF* const pStructureFileRef = GetAnimationStructureRef(pSoldier, usAnimSurface, usAnimState);
	if (pStructureFileRef != NULL)
	{
		BOOLEAN fReturn;
		if (pSoldier->ubBodyType == QUEENMONSTER)
		{
			// Queen uses onely one direction....
			fReturn = AddStructureToWorld(sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[0], pSoldier->pLevelNode);
		}
		else
		{
			fReturn = AddStructureToWorld(sGridNo, pSoldier->bLevel, &pStructureFileRef->pDBStructureRef[OneCDirection(pSoldier->bDirection)], pSoldier->pLevelNode);
		}

		if (!fReturn)
		{
			// Debug msg
			ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"FAILED: add struct info for merc %d (%ls), at %d direction %d", pSoldier->ubID, pSoldier->name, sGridNo, pSoldier->bDirection);

			if (pStructureFileRef->pDBStructureRef[OneCDirection(pSoldier->bDirection)].pDBStructure->ubNumberOfTiles > 1)
			{
				// If we have more than one tile
				pSoldier->uiStatusFlags |= SOLDIER_MULTITILE;
			}

			return FALSE;
		}
		else
		{
			// Turn on if we are multi-tiled
			if (pSoldier->pLevelNode->pStructureData->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1)
			{
				// If we have more than one tile
				pSoldier->uiStatusFlags |= SOLDIER_MULTITILE;
			}
		}
	}

	return TRUE;
}

BOOLEAN OKToAddMercToWorld( SOLDIERTYPE *pSoldier, INT8 bDirection )
{
  //if (pSoldier->uiStatusFlags & SOLDIER_MULTITILE)
	{
		// Get surface data
 		UINT16 usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );
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

	return AddMercStructureInfo(pSoldier->sGridNo, pSoldier);
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
	LEVELNODE* const n = CreateLevelNode();
	CHECKN(n != NULL);

	if (!AddNodeToWorld(iMapIndex, usIndex, 1, n)) return NULL;

	n->usIndex = usIndex;
	ResetSpecificLayerOptimizing(TILES_DYNAMIC_ROOF);
	return n;
}


LEVELNODE* AddRoofToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddRoof(iMapIndex, usIndex);
	if (n == NULL) return NULL;

	// Append node to list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pRoofHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	return n;
}


LEVELNODE* AddRoofToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddRoof(iMapIndex, usIndex);
	if (n == NULL) return NULL;

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


BOOLEAN IndexExistsInRoofLayer(INT16 sGridNo, UINT16 usIndex)
{
	for (const LEVELNODE* pRoof = gpWorldLevelData[sGridNo].pRoofHead; pRoof != NULL; pRoof = pRoof->pNext)
	{
		if (pRoof->usIndex == usIndex)
		{
			return TRUE;
		}
	}

	// Could not find it
	return FALSE;
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
	LEVELNODE* const n = CreateLevelNode();
	CHECKN(n != NULL);

	if (!AddNodeToWorld(iMapIndex, usIndex, 1, n)) return NULL;

	n->usIndex = usIndex;
	ResetSpecificLayerOptimizing(TILES_DYNAMIC_ONROOF);
	return n;
}


LEVELNODE* AddOnRoofToTail(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddOnRoof(iMapIndex, usIndex);
	if (n == NULL) return NULL;

	// Append the node to the list
	LEVELNODE** anchor = &gpWorldLevelData[iMapIndex].pOnRoofHead;
	while (*anchor != NULL) anchor = &(*anchor)->pNext;
	*anchor = n;

	return n;
}


LEVELNODE* AddOnRoofToHead(const UINT32 iMapIndex, const UINT16 usIndex)
{
	LEVELNODE* const n = AddOnRoof(iMapIndex, usIndex);
	CHECKN(n != NULL);

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
		if (pOnRoof->usIndex != NO_TILE)
		{
			const UINT32 fTileType = GetTileType(pOnRoof->usIndex);

			// Advance to next
			const LEVELNODE* pOldOnRoof = pOnRoof;
			pOnRoof = pOnRoof->pNext;

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
	CHECKN(n != NULL);
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
	CHECKN(pTopmost != NULL);

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
	CHECKN(n != NULL);
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


SOLDIERTYPE* WhoIsThere2(INT16 sGridNo, INT8 bLevel)
{
	if (!GridNoOnVisibleWorldTile(sGridNo)) return NULL;

	for (const STRUCTURE* pStructure = gpWorldLevelData[sGridNo].pStructureHead; pStructure != NULL; pStructure = pStructure->pNext)
	{
		if (!(pStructure->fFlags & STRUCTURE_PERSON)) continue;

		SOLDIERTYPE* const tgt = GetMan(pStructure->usStructureID);
		// person must either have their pSoldier->sGridNo here or be non-passable
		if (!(pStructure->fFlags & STRUCTURE_PASSABLE) || tgt->sGridNo == sGridNo)
		{
			if (bLevel == 0 && pStructure->sCubeOffset == 0 ||
					(bLevel > 0 && pStructure->sCubeOffset > 0))
			{
				// found a person, on the right level!
				// structure ID and merc ID are identical for merc structures
				return tgt;
			}
		}
	}

	return NULL;
}


UINT8	GetTerrainType(INT16 sGridNo)
{
	return gpWorldLevelData[sGridNo].ubTerrainID;
/*
	LEVELNODE	*pNode;


	// Check if we have anything in object layer which has a terrain modifier
	pNode = gpWorldLevelData[ sGridNo ].pObjectHead;

	if ( pNode != NULL )
	{
		if ( gTileDatabase[ pNode->usIndex ].ubTerrainID != NO_TERRAIN )
		{
			return( gTileDatabase[ pNode->usIndex ].ubTerrainID );
		}
	}

	// Now try terrain!
	pNode = gpWorldLevelData[ sGridNo ].pLandHead;

	return( gTileDatabase[ pNode->usIndex ].ubTerrainID );
*/
}


BOOLEAN Water(INT16 sGridNo)
{
  if (sGridNo == NOWHERE)
  {
    return FALSE;
  }

	const MAP_ELEMENT* pMapElement = &gpWorldLevelData[sGridNo];
	return
		pMapElement->ubTerrainID == LOW_WATER ||
		pMapElement->ubTerrainID == MED_WATER ||
		pMapElement->ubTerrainID == DEEP_WATER;
}


BOOLEAN DeepWater( INT16 sGridNo )
{
	const MAP_ELEMENT* pMapElement = &gpWorldLevelData[sGridNo];
	return pMapElement->ubTerrainID == DEEP_WATER;
}


BOOLEAN WaterTooDeepForAttacks(INT16 sGridNo)
{
	return DeepWater(sGridNo);
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


LEVELNODE* FindLevelNodeBasedOnStructure(INT16 sGridNo, STRUCTURE* pStructure)
{
	//ATE: First look on the struct layer.....
	for (LEVELNODE* pLevelNode = gpWorldLevelData[sGridNo].pStructHead; pLevelNode != NULL; pLevelNode = pLevelNode->pNext)
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return pLevelNode;
		}
	}

	// Next the roof layer....
	for (LEVELNODE* pLevelNode = gpWorldLevelData[sGridNo].pRoofHead; pLevelNode != NULL; pLevelNode = pLevelNode->pNext)
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return pLevelNode;
		}
	}

	// Then the object layer....
	for (LEVELNODE* pLevelNode = gpWorldLevelData[sGridNo].pObjectHead; pLevelNode != NULL; pLevelNode = pLevelNode->pNext)
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return pLevelNode;
		}
	}

	// Finally the onroof layer....
	for (LEVELNODE* pLevelNode = gpWorldLevelData[sGridNo].pOnRoofHead; pLevelNode != NULL; pLevelNode = pLevelNode->pNext)
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return pLevelNode;
		}
	}

	AssertMsg(0, "FindLevelNodeBasedOnStruct failed.");
	return NULL;
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
	for (UINT32 cnt = 0; cnt < WORLD_MAX; cnt++)
	{
		for (LEVELNODE* pNode = gpWorldLevelData[cnt].pStructHead; pNode != NULL; pNode = pNode->pNext)
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
	for (UINT32 cnt = 0; cnt < WORLD_MAX; cnt++)
	{
		for (LEVELNODE* pNode = gpWorldLevelData[cnt].pStructHead; pNode != NULL; pNode = pNode->pNext)
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
