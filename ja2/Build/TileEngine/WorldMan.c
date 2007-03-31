#include "WorldDef.h"
#include <stdio.h>
#include <string.h>
#include "WCheck.h"
#include "stdlib.h"
#include "time.h"
#include "Debug.h"
#include "Smooth.h"
#include "WorldMan.h"
#include "Lighting.h"
#include "RenderWorld.h"
#include "Overhead.h"
#include "AI.h"
#include "Sound_Control.h"
#include "Animation_Control.h"
#include "Isometric_Utils.h"
#include "Font_Control.h"
#include "Message.h"
#include "Tile_Cache.h"
#include "Smoothing_Utils.h"
#include "SaveLoadMap.h"
#include "Random.h"
#include "Render_Fun.h"
#include "GameSettings.h"
#include "MemMan.h"


extern BOOLEAN	gfBasement;

UINT32 guiLNCount[9];
static CHAR16 gzLevelString[9][15] =
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


UINT32 guiLevelNodes = 0;


// LEVEL NODE MANIPLULATION FUNCTIONS
static BOOLEAN CreateLevelNode(LEVELNODE** ppNode)
{
	*ppNode = MemAlloc( sizeof( LEVELNODE ) );
	CHECKF( *ppNode != NULL );

	// Clear all values
	memset( *ppNode, 0, sizeof( LEVELNODE ) );

	// Set default values
	(*ppNode)->ubShadeLevel = LightGetAmbient();
	(*ppNode)->ubNaturalShadeLevel = LightGetAmbient();
	(*ppNode)->pSoldier			= NULL;
	(*ppNode)->pNext				= NULL;
	(*ppNode)->sRelativeX		= 0;
	(*ppNode)->sRelativeY		= 0;

	guiLevelNodes++;

	return( TRUE );
}

void CountLevelNodes( void )
{
	UINT32 uiLoop, uiLoop2;
	LEVELNODE * pLN;
	MAP_ELEMENT * pME;

	for (uiLoop2 = 0; uiLoop2 < 9; uiLoop2++)
	{
		guiLNCount[uiLoop2] = 0;
	}

	for (uiLoop = 0; uiLoop < WORLD_MAX; uiLoop++)
	{
		pME = &(gpWorldLevelData[uiLoop]);
		// start at 1 to skip land head ptr; 0 stores total
		for (uiLoop2 = 1; uiLoop2 < 9; uiLoop2++)
		{
			pLN = pME->pLevelNodes[uiLoop2];
			while( pLN != NULL )
			{
				guiLNCount[uiLoop2]++;
				guiLNCount[0]++;
				pLN = pLN->pNext;
			}
		}
	}
}

#define LINE_HEIGHT 20
void DebugLevelNodePage( void )
{
	UINT32 uiLoop;

	SetFont( LARGEFONT1 );
	gprintf( 0, 0, L"DEBUG LEVELNODES PAGE 1 OF 1" );

	for (uiLoop = 1; uiLoop < 9; uiLoop++)
	{
		gprintf( 0, LINE_HEIGHT * (uiLoop + 1), gzLevelString[uiLoop], guiLNCount[uiLoop] );
	}
	gprintf( 0, LINE_HEIGHT * 12, L"%d land nodes in excess of world max (25600)", guiLNCount[1] - WORLD_MAX);
	gprintf( 0, LINE_HEIGHT * 13, L"Total # levelnodes %d, %d bytes each", guiLNCount[0], sizeof( LEVELNODE ) );
	gprintf( 0, LINE_HEIGHT * 14, L"Total memory for levelnodes %d", guiLNCount[0] * sizeof( LEVELNODE ) );
}


static BOOLEAN TypeExistsInLevel(LEVELNODE* pStartNode, UINT32 fType, UINT16* pusIndex)
{
	UINT32				fTileType;

	// Look through all objects and Search for type
	while( pStartNode != NULL )
	{

		if ( pStartNode->usIndex != NO_TILE && pStartNode->usIndex < NUMBEROFTILES )
		{
			GetTileType( pStartNode->usIndex, &fTileType );

			if ( fTileType == fType )
			{
				*pusIndex = pStartNode->usIndex;
				return( TRUE );
			}

		}

		pStartNode = pStartNode->pNext;
	}

	// Could not find it, return FALSE
	return( FALSE );
}


// SHADE LEVEL MANIPULATION FOR NODES
static void SetLevelShadeLevel(LEVELNODE* pStartNode, UINT8 ubShadeLevel)
{
	// Look through all objects and Search for type
	while( pStartNode != NULL )
	{
		pStartNode->ubShadeLevel = ubShadeLevel;

		// Advance to next
		pStartNode = pStartNode->pNext;
	}
}


static void AdjustLevelShadeLevel(LEVELNODE* pStartNode, INT8 bShadeDiff)
{
	// Look through all objects and Search for type
	while( pStartNode != NULL )
	{
		pStartNode->ubShadeLevel += bShadeDiff;

		if ( pStartNode->ubShadeLevel > MAX_SHADE_LEVEL )
		{
			pStartNode->ubShadeLevel = MAX_SHADE_LEVEL;
		}

		if ( pStartNode->ubShadeLevel < MIN_SHADE_LEVEL )
		{
			pStartNode->ubShadeLevel = MIN_SHADE_LEVEL;
		}

		// Advance to next
		pStartNode = pStartNode->pNext;

	}
}


static void SetIndexLevelNodeFlags(LEVELNODE* pStartNode, UINT32 uiFlags, UINT16 usIndex)
{
	// Look through all objects and Search for type
	while( pStartNode != NULL )
	{
		if ( pStartNode->usIndex == usIndex )
		{
			pStartNode->uiFlags |= uiFlags;
			break;
		}

		// Advance to next
		pStartNode = pStartNode->pNext;
	}
}


static void RemoveIndexLevelNodeFlags(LEVELNODE* pStartNode, UINT32 uiFlags, UINT16 usIndex)
{
	// Look through all objects and Search for type
	while( pStartNode != NULL )
	{
		if ( pStartNode->usIndex == usIndex )
		{
			pStartNode->uiFlags &= (~uiFlags);
			break;
		}

		// Advance to next
		pStartNode = pStartNode->pNext;

	}

}




// First for object layer
// #################################################################

LEVELNODE *AddObjectToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE			*pObject				 = NULL;
	LEVELNODE			*pNextObject		 = NULL;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	// If we're at the head, set here
	if ( pObject == NULL )
	{
		CHECKF( CreateLevelNode( &pNextObject ) != FALSE );
		pNextObject->usIndex = usIndex;

		gpWorldLevelData[ iMapIndex ].pObjectHead = pNextObject;
	}
	else
	{
		while( pObject != NULL )
		{

			if ( pObject->pNext == NULL )
			{
				CHECKF( CreateLevelNode( &pNextObject ) != FALSE );
				pObject->pNext = pNextObject;

				pNextObject->pNext = NULL;
				pNextObject->usIndex = usIndex;

				break;
			}

			pObject = pObject->pNext;

		}

	}

	//CheckForAndAddTileCacheStructInfo( pNextObject, (INT16)iMapIndex, usIndex );

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_OBJECTS );
	return( pNextObject );

}


BOOLEAN AddObjectToHead( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE							 *pObject					= NULL;
	LEVELNODE							 *pNextObject		  = NULL;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	CHECKF( CreateLevelNode( &pNextObject ) != FALSE );

	pNextObject->pNext = pObject;
	pNextObject->usIndex = usIndex;

	// Set head
	gpWorldLevelData[ iMapIndex ].pObjectHead = pNextObject;

	//CheckForAndAddTileCacheStructInfo( pNextObject, (INT16)iMapIndex, usIndex );

	// If it's NOT the first head
	ResetSpecificLayerOptimizing( TILES_DYNAMIC_OBJECTS );

	//Add the object to the map temp file, if we have to
	AddObjectToMapTempFile( iMapIndex, usIndex );

	return( TRUE );

}


BOOLEAN RemoveObject( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pObject		 = NULL;
	LEVELNODE	*pOldObject = NULL;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	// Look through all objects and remove index if found

	while( pObject != NULL )
	{
		if ( pObject->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldObject == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pObjectHead = pObject->pNext;
			}
			else
			{
				pOldObject->pNext = pObject->pNext;
			}

			CheckForAndDeleteTileCacheStructInfo( pObject, usIndex );

			// Delete memory assosiated with item
			MemFree( pObject );
			guiLevelNodes--;

			//Add the index to the maps temp file so we can remove it after reloading the map
			AddRemoveObjectToMapTempFile( iMapIndex, usIndex );

			return( TRUE );
		}

		pOldObject = pObject;
		pObject = pObject->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN TypeRangeExistsInObjectLayer( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT16 *pusObjectIndex )
{
	LEVELNODE	*pObject		 = NULL;
	LEVELNODE	*pOldObject		 = NULL;
	UINT32			fTileType;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	// Look through all objects and Search for type

	while( pObject != NULL )
	{
		// Advance to next
		pOldObject = pObject;
		pObject = pObject->pNext;

		if ( pOldObject->usIndex != NO_TILE && pOldObject->usIndex < NUMBEROFTILES )
		{
			GetTileType( pOldObject->usIndex, &fTileType );

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				*pusObjectIndex = pOldObject->usIndex;
				return( TRUE );
			}

		}

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN TypeExistsInObjectLayer( UINT32 iMapIndex, UINT32 fType, UINT16 *pusObjectIndex )
{
	LEVELNODE			*pObject		 = NULL;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	return( TypeExistsInLevel( pObject, fType, pusObjectIndex ) );
}


static void SetAllObjectShadeLevels(UINT32 iMapIndex, UINT8 ubShadeLevel)
{
	LEVELNODE	*pObject		 = NULL;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	SetLevelShadeLevel( pObject, ubShadeLevel );
}


static void AdjustAllObjectShadeLevels(UINT32 iMapIndex, INT8 bShadeDiff)
{
	LEVELNODE	*pObject		 = NULL;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	AdjustLevelShadeLevel( pObject, bShadeDiff );
}


BOOLEAN RemoveAllObjectsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	LEVELNODE	*pObject		 = NULL;
	LEVELNODE	*pOldObject		 = NULL;
	UINT32				fTileType;
	BOOLEAN fRetVal = FALSE;

	pObject = gpWorldLevelData[ iMapIndex ].pObjectHead;

	// Look through all objects and Search for type

	while( pObject != NULL )
	{
		// Advance to next
		pOldObject = pObject;
		pObject = pObject->pNext;

		if ( pOldObject->usIndex != NO_TILE && pOldObject->usIndex < NUMBEROFTILES )
		{

			GetTileType( pOldObject->usIndex, &fTileType );

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				// Remove Item
				RemoveObject( iMapIndex, pOldObject->usIndex );
				fRetVal = TRUE;
			}

		}

	}
	return fRetVal;
}

// #######################################################
// Land Peice Layer
// #######################################################

LEVELNODE *AddLandToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pNextLand		 = NULL;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// If we're at the head, set here
	if ( pLand == NULL )
	{
		CHECKF( CreateLevelNode( &pNextLand ) != FALSE );
		pNextLand->usIndex = usIndex;

		gpWorldLevelData[ iMapIndex ].pLandHead = pNextLand;

	}
	else
	{
		while( pLand != NULL )
		{

			if ( pLand->pNext == NULL )
			{
				CHECKF( CreateLevelNode( &pNextLand ) != FALSE );
				pLand->pNext = pNextLand;

				pNextLand->pNext			= NULL;
				pNextLand->pPrevNode  = pLand;
				pNextLand->usIndex    = usIndex;

				break;
			}

			pLand = pLand->pNext;

		}

	}

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_LAND );
	return( pNextLand );

}


BOOLEAN AddLandToHead( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pNextLand		 = NULL;
  TILE_ELEMENT							 TileElem;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Allocate head
	CHECKF( CreateLevelNode( &pNextLand ) != FALSE );

	pNextLand->pNext			= pLand;
	pNextLand->pPrevNode  = NULL;
	pNextLand->usIndex		= usIndex;
	pNextLand->ubShadeLevel = LightGetAmbient();

	if ( usIndex < NUMBEROFTILES )
	{
		// Get tile element
		TileElem = gTileDatabase[ usIndex  ];

		// Check for full tile
		if ( TileElem.ubFullTile )
		{
			gpWorldLevelData[ iMapIndex ].pLandStart = pNextLand;
		}
	}

	// Set head
	gpWorldLevelData[ iMapIndex ].pLandHead = pNextLand;

	// If it's NOT the first head
	if ( pLand != NULL )
	{
		pLand->pPrevNode = pNextLand;
	}

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_LAND );
	return( TRUE );

}


static BOOLEAN AdjustForFullTile(UINT32 iMapIndex);
static BOOLEAN RemoveLandEx(UINT32 iMapIndex, UINT16 usIndex);


BOOLEAN RemoveLand( UINT32 iMapIndex, UINT16 usIndex )
{

	RemoveLandEx( iMapIndex, usIndex );

	AdjustForFullTile( iMapIndex );

	return( FALSE );

}


static BOOLEAN RemoveLandEx(UINT32 iMapIndex, UINT16 usIndex)
{
	LEVELNODE	*pLand		 = NULL;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all Lands and remove index if found

	while( pLand != NULL )
	{
		if ( pLand->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pLand->pPrevNode == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pLandHead = pLand->pNext;
			}
			else
			{
				pLand->pPrevNode->pNext = pLand->pNext;
			}

			// Check for tail
			if ( pLand->pNext == NULL )
			{
			}
			else
			{
				pLand->pNext->pPrevNode = pLand->pPrevNode;
			}

			// Delete memory assosiated with item
			MemFree( pLand );
			guiLevelNodes--;

			break;

		}
		pLand = pLand->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


static BOOLEAN AdjustForFullTile(UINT32 iMapIndex)
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pOldLand	 = NULL;
  TILE_ELEMENT		TileElem;
//	UINT32 iType;
//	UINT16 iNewIndex;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all Lands and remove index if found

	while( pLand != NULL )
	{
		if ( pLand->usIndex < NUMBEROFTILES )
		{
			// If this is a full tile, set new full tile
			TileElem = gTileDatabase[ pLand->usIndex ];

			// Check for full tile
			if ( TileElem.ubFullTile )
			{
				 gpWorldLevelData[ iMapIndex ].pLandStart = pLand;
				 return( TRUE );
			}
		}
		pOldLand = pLand;
		pLand = pLand->pNext;

	}

	// Could not find a full tile
	// Set to tail, and convert it to a full tile!
	// Add a land peice to tail from basic land
	{
		UINT16				NewIndex;
		LEVELNODE			*pNewNode;

		NewIndex = (UINT16)(Random( 10 ) );

		// Adjust for type
		NewIndex += gTileTypeStartIndex[ gCurrentBackground ];

		pNewNode = AddLandToTail( iMapIndex, NewIndex );

		gpWorldLevelData[ iMapIndex ].pLandStart = pNewNode;

	}

	return( FALSE );

}


BOOLEAN ReplaceLandIndex( UINT32 iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex )
{
	LEVELNODE	*pLand				= NULL;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all Lands and remove index if found

	while( pLand != NULL )
	{

		if ( pLand->usIndex == usOldIndex )
		{
			// OK, set new index value
			pLand->usIndex = usNewIndex;

			AdjustForFullTile( iMapIndex );

			return( TRUE );
		}

		// Advance
		pLand = pLand->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN TypeExistsInLandLayer( UINT32 iMapIndex, UINT32 fType, UINT16 *pusLandIndex )
{
	LEVELNODE	*pLand		 = NULL;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	return( TypeExistsInLevel( pLand, fType, pusLandIndex ) );

}


BOOLEAN TypeRangeExistsInLandLayer( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT16 *pusLandIndex )
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pOldLand		 = NULL;
	UINT32			fTileType;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all objects and Search for type

	while( pLand != NULL )
	{

		if ( pLand->usIndex != NO_TILE )
		{
			GetTileType( pLand->usIndex, &fTileType );

			// Advance to next
			pOldLand = pLand;
			pLand = pLand->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				*pusLandIndex = pOldLand->usIndex;
				return( TRUE );
			}

		}

	}

	// Could not find it, return FALSE

	return( FALSE );

}


static BOOLEAN TypeRangeExistsInLandHead(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT16* pusLandIndex)
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pOldLand		 = NULL;
	UINT32			fTileType;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all objects and Search for type

	if ( pLand->usIndex != NO_TILE )
	{
		GetTileType( pLand->usIndex, &fTileType );

		// Advance to next
		pOldLand = pLand;
		pLand = pLand->pNext;

		if ( fTileType >= fStartType && fTileType <= fEndType )
		{
			*pusLandIndex = pOldLand->usIndex;
			return( TRUE );
		}

	}

	// Could not find it, return FALSE

	return( FALSE );

}


static BOOLEAN TypeRangeExistsInStructLayer(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT16* pusStructIndex)
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct	 = NULL;
	UINT32		fTileType;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all objects and Search for type

	while( pStruct != NULL )
	{
		if ( pStruct->usIndex != NO_TILE )
		{
			GetTileType( pStruct->usIndex, &fTileType );

			// Advance to next
			pOldStruct = pStruct;
			pStruct = pStruct->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				*pusStructIndex = pOldStruct->usIndex;
				return( TRUE );
			}
		}
	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN RemoveAllLandsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pOldLand		 = NULL;
	UINT32			fTileType;
	BOOLEAN fRetVal = FALSE;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all objects and Search for type

	while( pLand != NULL )
	{

		if ( pLand->usIndex != NO_TILE )
		{
			GetTileType( pLand->usIndex, &fTileType );

			// Advance to next
			pOldLand = pLand;
			pLand = pLand->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				// Remove Item
				RemoveLand( iMapIndex, pOldLand->usIndex );
				fRetVal = TRUE;
			}

		}

	}
	return fRetVal;
}


static void SetAllLandShadeLevels(UINT32 iMapIndex, UINT8 ubShadeLevel)
{
	LEVELNODE	*pLand		 = NULL;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	SetLevelShadeLevel( pLand, ubShadeLevel );
}


static void AdjustAllLandShadeLevels(UINT32 iMapIndex, INT8 bShadeDiff)
{
	LEVELNODE	*pLand		 = NULL;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all objects and Search for type
	AdjustLevelShadeLevel( pLand, bShadeDiff );
}


BOOLEAN DeleteAllLandLayers( UINT32 iMapIndex )
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pOldLand		 = NULL;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Look through all objects and Search for type

	while( pLand != NULL )
	{

		// Advance to next
		pOldLand = pLand;
		pLand = pLand->pNext;

		// Remove Item
		RemoveLandEx( iMapIndex, pOldLand->usIndex );

	}

	// Set world data values
	gpWorldLevelData[ iMapIndex ].pLandHead = NULL;
	gpWorldLevelData[ iMapIndex ].pLandStart = NULL;

	return( TRUE );
}

BOOLEAN InsertLandIndexAtLevel( UINT32 iMapIndex, UINT16 usIndex, UINT8 ubLevel )
{
	LEVELNODE			*pLand		 = NULL;
	LEVELNODE			*pNextLand		 = NULL;
	UINT8					  level = 0;
	BOOLEAN					CanInsert = FALSE;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;


	// If we want to insert at head;
	if ( ubLevel == 0 )
	{
		 AddLandToHead( iMapIndex, usIndex );
		 return( TRUE );
	}


	// Allocate memory for new item
	CHECKF( CreateLevelNode( &pNextLand ) != FALSE );
	pNextLand->usIndex = usIndex;

	// Move to index before insertion
	while( pLand != NULL )
	{
		if ( level == (ubLevel-1) )
		{
			CanInsert = TRUE;
			break;
		}

		pLand = pLand->pNext;
		level++;

	}

	// Check if level has been macthed
	if ( !CanInsert )
	{
		return( FALSE );
	}

	// Set links, according to position!
	pNextLand->pPrevNode = pLand;
	pNextLand->pNext = pLand->pNext;
	pLand->pNext = pNextLand;

	// Check for tail
	if ( pNextLand->pNext == NULL )
	{
	}
	else
	{
		pNextLand->pNext->pPrevNode = pNextLand;
	}

	AdjustForFullTile( iMapIndex );

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_LAND );
	return( TRUE );

}

BOOLEAN RemoveHigherLandLevels( UINT32 iMapIndex, UINT32 fSrcType, UINT32 **puiHigherTypes, UINT8 *pubNumHigherTypes )
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pOldLand		 = NULL;
	UINT32			fTileType;
  UINT8					ubSrcLogHeight;

	*pubNumHigherTypes = 0;
	*puiHigherTypes = NULL;

	// Start at tail and up
	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	//GEt tail
	while( pLand != NULL )
	{
		pOldLand = pLand;
		pLand = pLand->pNext;
	}

	pLand = pOldLand;

	// Get src height
	GetTileTypeLogicalHeight( fSrcType, &ubSrcLogHeight );

	// Look through all objects and Search for height
	while( pLand != NULL )
	{

			GetTileType( pLand->usIndex, &fTileType );

			// Advance to next
			pOldLand = pLand;
			pLand = pLand->pPrevNode;

			if ( gTileTypeLogicalHeight[ fTileType ] > ubSrcLogHeight )
			{
				// Remove Item
				SetLandIndex( iMapIndex, pOldLand->usIndex, fTileType, TRUE );

				(*pubNumHigherTypes)++;

				*puiHigherTypes = MemRealloc( *puiHigherTypes, (*pubNumHigherTypes) * sizeof( UINT32 ) );

				(*puiHigherTypes)[ (*pubNumHigherTypes)-1 ] = fTileType;

			}

	}

	// Adjust full tile sets
	AdjustForFullTile( iMapIndex );

	return( TRUE );
}


static BOOLEAN SetLowerLandLevels(UINT32 iMapIndex, UINT32 fSrcType, UINT16 usIndex)
{
	LEVELNODE	*pLand		 = NULL;
	LEVELNODE	*pOldLand		 = NULL;
	UINT32			fTileType;
  UINT8					ubSrcLogHeight;
	UINT16			NewTile;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	// Get src height
	GetTileTypeLogicalHeight( fSrcType, &ubSrcLogHeight );

	// Look through all objects and Search for height
	while( pLand != NULL )
	{

			GetTileType( pLand->usIndex, &fTileType );

			// Advance to next
			pOldLand = pLand;
			pLand = pLand->pNext;

			if ( gTileTypeLogicalHeight[ fTileType ] < ubSrcLogHeight )
			{
				// Set item
				GetTileIndexFromTypeSubIndex( fTileType, usIndex, &NewTile );

				// Set as normal
				SetLandIndex( iMapIndex, NewTile, fTileType, FALSE );

			}

	}


	// Adjust full tile sets
	AdjustForFullTile( iMapIndex );

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_LAND );
	return( TRUE );

}


// Struct layer
// #################################################################

LEVELNODE *AddStructToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	return(AddStructToTailCommon( iMapIndex, usIndex, TRUE ));
}

LEVELNODE *ForceStructToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	return(AddStructToTailCommon( iMapIndex, usIndex, FALSE ));
}

LEVELNODE *AddStructToTailCommon( UINT32 iMapIndex, UINT16 usIndex, BOOLEAN fAddStructDBInfo )
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pTailStruct = NULL;
	LEVELNODE	*pNextStruct = NULL;
	DB_STRUCTURE *				pDBStructure;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Do we have an empty list?
	if ( pStruct == NULL )
	{
		CHECKF( CreateLevelNode( &pNextStruct ) != FALSE );

		if ( fAddStructDBInfo )
		{
			if ( usIndex < NUMBEROFTILES )
			{
				if (gTileDatabase[usIndex].pDBStructureRef != NULL)
				{
					if (AddStructureToWorld( (INT16) iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef, pNextStruct ) == FALSE)
					{
						MemFree( pNextStruct );
						guiLevelNodes--;
						return( NULL );
					}
				}
				else
				{
	//				 pNextStruct->pStructureData = NULL;
				}
			}
		}

		pNextStruct->usIndex = usIndex;

		pNextStruct->pNext = NULL;

		gpWorldLevelData[ iMapIndex ].pStructHead = pNextStruct;
	}
	else
	{
		// MOVE TO TAIL
		while ( pStruct != NULL )
		{
			pTailStruct = pStruct;
			pStruct = pStruct->pNext;
		}

		CHECKN( CreateLevelNode( &pNextStruct ) != FALSE );

		if ( fAddStructDBInfo )
		{
			if ( usIndex < NUMBEROFTILES )
			{

				if (gTileDatabase[usIndex].pDBStructureRef != NULL)
				{
					if (AddStructureToWorld( (INT16) iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef, pNextStruct ) == FALSE)
					{
						MemFree( pNextStruct );
						guiLevelNodes--;
						return( NULL );
					}
					else
					{
	//					pNextStruct->pStructureData = NULL;
					}
				}
			}
		}
		pNextStruct->usIndex = usIndex;

		pNextStruct->pNext = NULL;
		pTailStruct->pNext = pNextStruct;
	}

	// Check flags for tiledat and set a shadow if we have a buddy
	if ( usIndex < NUMBEROFTILES )
	{
		if ( !GridNoIndoors( iMapIndex ) && gTileDatabase[ usIndex ].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[ usIndex ].sBuddyNum != -1 )
		{
			AddShadowToHead( iMapIndex, gTileDatabase[ usIndex ].sBuddyNum );
			gpWorldLevelData[ iMapIndex ].pShadowHead->uiFlags |= LEVELNODE_BUDDYSHADOW;
		}

		//Check for special flag to stop burn-through on same-tile structs...
		if ( gTileDatabase[ usIndex ].pDBStructureRef != NULL )
		{
			pDBStructure = gTileDatabase[ usIndex ].pDBStructureRef->pDBStructure;

			// Default to off....
			gpWorldLevelData[ iMapIndex ].ubExtFlags[0] &= (~MAPELEMENT_EXT_NOBURN_STRUCT);

			// If we are NOT a wall and NOT multi-tiles, set mapelement flag...
			if ( !FindStructure( (INT16)iMapIndex, STRUCTURE_WALLSTUFF ) && pDBStructure->ubNumberOfTiles == 1 )
			{
				// Set flag...
				gpWorldLevelData[ iMapIndex ].ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
			}
		}
	}

	//Add the structure the maps temp file
	AddStructToMapTempFile( iMapIndex, usIndex );

	//CheckForAndAddTileCacheStructInfo( pNextStruct, (INT16)iMapIndex, usIndex );

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_STRUCTURES );

	return( pNextStruct );
}


static void SetWorldFlagsFromNewNode(UINT16 sGridNo, UINT16 usIndex);


BOOLEAN AddStructToHead( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE							 *pStruct		 = NULL;
	LEVELNODE							 *pNextStruct		 = NULL;
	DB_STRUCTURE *				pDBStructure;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	CHECKF( CreateLevelNode( &pNextStruct ) != FALSE );

	if ( usIndex < NUMBEROFTILES )
	{
		if (gTileDatabase[usIndex].pDBStructureRef != NULL)
		{
			if (AddStructureToWorld( (INT16) iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef, pNextStruct ) == FALSE)
			{
				MemFree( pNextStruct );
				guiLevelNodes--;
				return( FALSE );
			}
		}
	}

	pNextStruct->pNext = pStruct;
	pNextStruct->usIndex = usIndex;

	// Set head
	gpWorldLevelData[ iMapIndex ].pStructHead = pNextStruct;

	SetWorldFlagsFromNewNode( (UINT16)iMapIndex, pNextStruct->usIndex );

	if ( usIndex < NUMBEROFTILES )
	{
		// Check flags for tiledat and set a shadow if we have a buddy
		if ( !GridNoIndoors( iMapIndex ) && gTileDatabase[ usIndex ].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[ usIndex ].sBuddyNum != -1 )
		{
			AddShadowToHead( iMapIndex, gTileDatabase[ usIndex ].sBuddyNum );
			gpWorldLevelData[ iMapIndex ].pShadowHead->uiFlags |= LEVELNODE_BUDDYSHADOW;
		}

		//Check for special flag to stop burn-through on same-tile structs...
		if ( gTileDatabase[ usIndex ].pDBStructureRef != NULL )
		{
			pDBStructure = gTileDatabase[ usIndex ].pDBStructureRef->pDBStructure;

			// Default to off....
			gpWorldLevelData[ iMapIndex ].ubExtFlags[0] &= (~MAPELEMENT_EXT_NOBURN_STRUCT);

			// If we are NOT a wall and NOT multi-tiles, set mapelement flag...
			if ( !!FindStructure( (INT16)iMapIndex, STRUCTURE_WALLSTUFF ) && pDBStructure->ubNumberOfTiles == 1 )
			{
				// Set flag...
				gpWorldLevelData[ iMapIndex ].ubExtFlags[0] |= MAPELEMENT_EXT_NOBURN_STRUCT;
			}
		}

	}

	//Add the structure the maps temp file
	AddStructToMapTempFile( iMapIndex, usIndex );

	//CheckForAndAddTileCacheStructInfo( pNextStruct, (INT16)iMapIndex, usIndex );

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_STRUCTURES );
	return( TRUE );

}


static BOOLEAN InsertStructIndex(UINT32 iMapIndex, UINT16 usIndex, UINT8 ubLevel)
{
	LEVELNODE		*pStruct		 = NULL;
	LEVELNODE		*pNextStruct		 = NULL;
	UINT8					  level = 0;
	BOOLEAN					CanInsert = FALSE;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// If we want to insert at head;
	if ( ubLevel == 0 )
	{
		 return( AddStructToHead( iMapIndex, usIndex ) );
	}


	// Allocate memory for new item
	CHECKF( CreateLevelNode( &pNextStruct ) != FALSE );

	pNextStruct->usIndex = usIndex;

	// Move to index before insertion
	while( pStruct != NULL )
	{
		if ( level == (ubLevel-1) )
		{
			CanInsert = TRUE;
			break;
		}

		pStruct = pStruct->pNext;
		level++;

	}

	// Check if level has been macthed
	if ( !CanInsert )
	{
		MemFree( pNextStruct );
		guiLevelNodes--;
		return( FALSE );
	}

	if ( usIndex < NUMBEROFTILES )
	{
		if (gTileDatabase[usIndex].pDBStructureRef != NULL)
		{
			if (AddStructureToWorld( (INT16) iMapIndex, 0, gTileDatabase[usIndex].pDBStructureRef, pNextStruct ) == FALSE)
			{
				MemFree( pNextStruct );
				guiLevelNodes--;
				return( FALSE );
			}
		}
	}

	// Set links, according to position!
	pNextStruct->pNext = pStruct->pNext;
	pStruct->pNext = pNextStruct;

	//CheckForAndAddTileCacheStructInfo( pNextStruct, (INT16)iMapIndex, usIndex );

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_STRUCTURES );
	return( TRUE );
}


static BOOLEAN RemoveStructFromTailCommon(UINT32 iMapIndex, BOOLEAN fRemoveStructDBInfo);


static BOOLEAN RemoveStructFromTail(UINT32 iMapIndex)
{
	return(RemoveStructFromTailCommon( iMapIndex, TRUE ));
}

BOOLEAN ForceRemoveStructFromTail( UINT32 iMapIndex )
{
	return(RemoveStructFromTailCommon( iMapIndex, FALSE ));
}


static BOOLEAN RemoveShadow(UINT32 iMapIndex, UINT16 usIndex);


static BOOLEAN RemoveStructFromTailCommon(UINT32 iMapIndex, BOOLEAN fRemoveStructDBInfo)
{
	LEVELNODE	*pStruct		  = NULL;
	LEVELNODE	*pPrevStruct	= NULL;
	UINT16		usIndex;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// GOTO TAIL
	while( pStruct != NULL )
	{
		// AT THE TAIL
		if ( pStruct->pNext == NULL )
		{
			if ( pPrevStruct != NULL )
			{
				pPrevStruct->pNext = pStruct->pNext;
			}
			else
				gpWorldLevelData[ iMapIndex ].pStructHead = pPrevStruct;

			if ( fRemoveStructDBInfo )
			{
				//Check for special flag to stop burn-through on same-tile structs...
				if ( pStruct->pStructureData != NULL )
				{
					// If we are NOT a wall and NOT multi-tiles, set mapelement flag...
					//if ( !( pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF ) && pStruct->pStructureData->pDBStructureRef->pDBStructure->ubNumberOfTiles == 1 )
					//{
						// UNSet flag...
					//	gpWorldLevelData[ iMapIndex ].ubExtFlags[0] &= ( ~MAPELEMENT_EXT_NOBURN_STRUCT );
					//}
				}

				DeleteStructureFromWorld( pStruct->pStructureData );
			}

			usIndex = pStruct->usIndex;

			//If we have to, make sure to remove this node when we reload the map from a saved game
			RemoveStructFromMapTempFile( iMapIndex, usIndex );


			MemFree( pStruct );
			guiLevelNodes--;

			if ( usIndex < NUMBEROFTILES )
			{
				// Check flags for tiledat and set a shadow if we have a buddy
				if ( !GridNoIndoors( iMapIndex ) && gTileDatabase[ usIndex ].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[ usIndex ].sBuddyNum != -1 )
				{
					RemoveShadow( iMapIndex, gTileDatabase[ usIndex ].sBuddyNum );
				}

			}
			return( TRUE );
		}

		pPrevStruct = pStruct;
		pStruct = pStruct->pNext;
	}

	return( TRUE );
}


static void RemoveWorldFlagsFromNewNode(UINT16 sGridNo, UINT16 usIndex);


BOOLEAN RemoveStruct( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct  = NULL;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all structs and remove index if found

	while( pStruct != NULL )
	{
		if ( pStruct->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldStruct == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pStructHead = pStruct->pNext;
			}
			else
			{
				pOldStruct->pNext = pStruct->pNext;
			}

			//Check for special flag to stop burn-through on same-tile structs...
			if ( pStruct->pStructureData != NULL )
			{
				// If we are NOT a wall and NOT multi-tiles, set mapelement flag...
				//if ( !( pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF ) && pStruct->pStructureData->pDBStructureRef->pDBStructure->ubNumberOfTiles == 1 )
				//{
					// UNSet flag...
				//	gpWorldLevelData[ iMapIndex ].ubExtFlags[0] &= ( ~MAPELEMENT_EXT_NOBURN_STRUCT );
				//}
			}

			// Delete memory assosiated with item
			DeleteStructureFromWorld( pStruct->pStructureData );

			//If we have to, make sure to remove this node when we reload the map from a saved game
			RemoveStructFromMapTempFile( iMapIndex, usIndex );

			if ( usIndex < NUMBEROFTILES )
			{
				// Check flags for tiledat and set a shadow if we have a buddy
				if ( !GridNoIndoors( iMapIndex ) && gTileDatabase[ usIndex ].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[ usIndex ].sBuddyNum != -1 )
				{
					RemoveShadow( iMapIndex, gTileDatabase[ usIndex ].sBuddyNum );
				}
			}
			MemFree( pStruct );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldStruct = pStruct;
		pStruct = pStruct->pNext;

	}

	// Could not find it, return FALSE
	RemoveWorldFlagsFromNewNode( (UINT16)iMapIndex, usIndex );

	return( FALSE );

}


BOOLEAN RemoveStructFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode )
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct  = NULL;
	UINT16		usIndex;

	usIndex = pNode->usIndex;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all structs and remove index if found

	while( pStruct != NULL )
	{
		if ( pStruct == pNode )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldStruct == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pStructHead = pStruct->pNext;
			}
			else
			{
				pOldStruct->pNext = pStruct->pNext;
			}

			// Delete memory assosiated with item
			DeleteStructureFromWorld( pStruct->pStructureData );

			//If we have to, make sure to remove this node when we reload the map from a saved game
			RemoveStructFromMapTempFile( iMapIndex, usIndex );

			if ( pNode->usIndex < NUMBEROFTILES )
			{
				// Check flags for tiledat and set a shadow if we have a buddy
				if ( !GridNoIndoors( iMapIndex ) && gTileDatabase[ usIndex ].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[ usIndex ].sBuddyNum != -1 )
				{
					RemoveShadow( iMapIndex, gTileDatabase[ usIndex ].sBuddyNum );
				}
			}
			MemFree( pStruct );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldStruct = pStruct;
		pStruct = pStruct->pNext;

	}

	// Could not find it, return FALSE
	RemoveWorldFlagsFromNewNode( (UINT16)iMapIndex, usIndex );

	return( FALSE );

}


BOOLEAN RemoveAllStructsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct		 = NULL;
	UINT32				fTileType;
	UINT16				usIndex;
	BOOLEAN fRetVal = FALSE;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all structs and Search for type

	while( pStruct != NULL )
	{

		if ( pStruct->usIndex != NO_TILE )
		{

			GetTileType( pStruct->usIndex, &fTileType );

			// Advance to next
			pOldStruct = pStruct;
			pStruct = pStruct->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				usIndex = pOldStruct->usIndex;

				// Remove Item
				if ( usIndex < NUMBEROFTILES )
				{
					RemoveStruct( iMapIndex, pOldStruct->usIndex );
					fRetVal = TRUE;
					if ( !GridNoIndoors( iMapIndex ) && gTileDatabase[ usIndex ].uiFlags & HAS_SHADOW_BUDDY && gTileDatabase[ usIndex ].sBuddyNum != -1 )
					{
						RemoveShadow( iMapIndex, gTileDatabase[ usIndex ].sBuddyNum );
					}
				}
			}

		}

	}
	return fRetVal;
}

//Kris:  This was a serious problem.  When saving the map and then reloading it, the structure
//  information was invalid if you changed the types, etc.  This is the bulletproof way.
BOOLEAN ReplaceStructIndex( UINT32 iMapIndex, UINT16 usOldIndex, UINT16 usNewIndex )
{
	RemoveStruct( iMapIndex, usOldIndex );
	AddWallToStructLayer( iMapIndex, usNewIndex, FALSE );
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
BOOLEAN AddWallToStructLayer( INT32 iMapIndex, UINT16 usIndex, BOOLEAN fReplace )
{
	LEVELNODE	*pStruct = NULL;
	UINT16				usCheckWallOrient;
	UINT16				usWallOrientation;
	BOOLEAN				fInsertFound = FALSE;
	BOOLEAN				fRoofFound = FALSE;
	UINT8					ubRoofLevel=0;
	UINT32				uiCheckType;
	UINT8					ubLevel = 0;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;


	// Get orientation of peice we want to add
	GetWallOrientation( usIndex, &usWallOrientation );

	// Look through all objects and Search for orientation
	while( pStruct != NULL )
	{

		GetWallOrientation( pStruct->usIndex, &usCheckWallOrient );
		//OLD CASE
		//if ( usCheckWallOrient > usWallOrientation )
		//Kris:
		//New case -- If placing a new wall which is at right angles to the current wall, then
		//we insert it.
		if( usCheckWallOrient > usWallOrientation )
		{
			if( (usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) &&
				  (usCheckWallOrient == INSIDE_TOP_LEFT || usCheckWallOrient == OUTSIDE_TOP_LEFT ) ||
					(usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT) &&
					(usCheckWallOrient == INSIDE_TOP_RIGHT || usCheckWallOrient == OUTSIDE_TOP_RIGHT ) )
			{
				fInsertFound = TRUE;
			}
		}

		GetTileType( pStruct->usIndex, &uiCheckType );

//		if ( uiCheckType >= FIRSTFLOOR && uiCheckType <= LASTFLOOR )
		if ( uiCheckType >= FIRSTROOF && uiCheckType <= LASTROOF )
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
		if( (usWallOrientation == INSIDE_TOP_RIGHT || usWallOrientation == OUTSIDE_TOP_RIGHT) &&
			  (usCheckWallOrient == INSIDE_TOP_RIGHT || usCheckWallOrient == OUTSIDE_TOP_RIGHT ) ||
				(usWallOrientation == INSIDE_TOP_LEFT || usWallOrientation == OUTSIDE_TOP_LEFT) &&
			  (usCheckWallOrient == INSIDE_TOP_LEFT || usCheckWallOrient == OUTSIDE_TOP_LEFT ) )
		{
			 // Same, if replace, replace here
			 if ( fReplace )
			 {
					return( ReplaceStructIndex( iMapIndex, pStruct->usIndex, usIndex ) );
			 }
			 else
			 {
					return( FALSE );
			 }
		}

		// Advance to next
		pStruct = pStruct->pNext;

		ubLevel++;

	}

	// Check if we found an insert position, otherwise set to head
	if ( fInsertFound )
	{
		 // Insert struct at head
		 AddStructToHead( iMapIndex, usIndex );
	}
	else
	{
		 // Make sure it's ALWAYS after the roof ( if any )
		 if ( fRoofFound )
		 {
				InsertStructIndex( iMapIndex, usIndex, ubRoofLevel );
		 }
		 else
		 {
				AddStructToTail( iMapIndex, usIndex );
		 }
	}

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_STRUCTURES );
	// Could not find it, return FALSE
	return( TRUE );

}

BOOLEAN TypeExistsInStructLayer( UINT32 iMapIndex, UINT32 fType, UINT16 *pusStructIndex )
{
	LEVELNODE			*pStruct		 = NULL;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	return( TypeExistsInLevel( pStruct, fType, pusStructIndex ) );
}


static void SetAllStructShadeLevels(UINT32 iMapIndex, UINT8 ubShadeLevel)
{
	LEVELNODE	*pStruct		 = NULL;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	SetLevelShadeLevel( pStruct, ubShadeLevel );
}


static void AdjustAllStructShadeLevels(UINT32 iMapIndex, INT8 bShadeDiff)
{
	LEVELNODE	*pStruct		 = NULL;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	AdjustLevelShadeLevel( pStruct, bShadeDiff );
}


static void SetStructIndexFlagsFromTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT32 uiFlags)
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct		 = NULL;
	UINT32				fTileType;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all structs and Search for type

	while( pStruct != NULL )
	{

		if ( pStruct->usIndex != NO_TILE )
		{

			GetTileType( pStruct->usIndex, &fTileType );

			// Advance to next
			pOldStruct = pStruct;
			pStruct = pStruct->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				pOldStruct->uiFlags |= uiFlags;
			}

		}

	}

}

BOOLEAN HideStructOfGivenType( UINT32 iMapIndex, UINT32 fType, BOOLEAN fHide )
{
	if ( fHide )
	{
		SetRoofIndexFlagsFromTypeRange( iMapIndex, fType, fType, LEVELNODE_HIDDEN  );
	}
	else
	{
		// ONLY UNHIDE IF NOT REAVEALED ALREADY
		if ( !(gpWorldLevelData[ iMapIndex ].uiFlags & MAPELEMENT_REVEALED ) )
		{
			RemoveRoofIndexFlagsFromTypeRange( iMapIndex, fType, fType, LEVELNODE_HIDDEN  );
		}
	}
	return( TRUE );
}


static void RemoveStructIndexFlagsFromTypeRange(UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT32 uiFlags)
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct		 = NULL;
	UINT32				fTileType;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all structs and Search for type

	while( pStruct != NULL )
	{

		if ( pStruct->usIndex != NO_TILE )
		{

			GetTileType( pStruct->usIndex, &fTileType );

			// Advance to next
			pOldStruct = pStruct;
			pStruct = pStruct->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				pOldStruct->uiFlags &= ( ~uiFlags );
			}

		}

	}

}


// Shadow layer
// #################################################################

BOOLEAN AddShadowToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pShadow		 = NULL;
	LEVELNODE	*pNextShadow		 = NULL;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	// If we're at the head, set here
	if ( pShadow == NULL )
	{
		CHECKF( CreateLevelNode( &pShadow ) != FALSE );
		pShadow->usIndex = usIndex;

		gpWorldLevelData[ iMapIndex ].pShadowHead = pShadow;

	}
	else
	{
		while( pShadow != NULL )
		{

			if ( pShadow->pNext == NULL )
			{
				CHECKF( CreateLevelNode( &pNextShadow ) != FALSE );
				pShadow->pNext = pNextShadow;
				pNextShadow->pNext = NULL;
				pNextShadow->usIndex = usIndex;
				break;
			}

			pShadow = pShadow->pNext;

		}

	}

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_SHADOWS );
	return( TRUE );

}

//Kris:  identical shadows can exist in the same gridno, though it makes no sense
//		because it actually renders the shadows darker than the others.  This is an
//	  undesirable effect with walls and buildings so I added this function to make
//		sure there isn't already a shadow before placing it.
void AddExclusiveShadow( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pShadow;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;
	while( pShadow )
	{
		if( pShadow->usIndex == usIndex )
			return;
		pShadow = pShadow->pNext;
	}
	AddShadowToHead( iMapIndex, usIndex );
}

BOOLEAN AddShadowToHead( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pShadow;
	LEVELNODE	*pNextShadow = NULL;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	// Allocate head
	CHECKF( CreateLevelNode( &pNextShadow ) != FALSE );
	pNextShadow->pNext = pShadow;
	pNextShadow->usIndex = usIndex;

	// Set head
	gpWorldLevelData[ iMapIndex ].pShadowHead = pNextShadow;

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_SHADOWS );
	return( TRUE );
}


static BOOLEAN RemoveShadow(UINT32 iMapIndex, UINT16 usIndex)
{
	LEVELNODE	*pShadow		 = NULL;
	LEVELNODE	*pOldShadow  = NULL;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	// Look through all shadows and remove index if found

	while( pShadow != NULL )
	{
		if ( pShadow->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldShadow == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pShadowHead = pShadow->pNext;
			}
			else
			{
				pOldShadow->pNext = pShadow->pNext;
			}

			// Delete memory assosiated with item
			MemFree( pShadow );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldShadow = pShadow;
		pShadow = pShadow->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN RemoveShadowFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode )
{
	LEVELNODE	*pShadow		 = NULL;
	LEVELNODE	*pOldShadow  = NULL;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	// Look through all shadows and remove index if found

	while( pShadow != NULL )
	{
		if ( pShadow == pNode )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldShadow == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pShadowHead = pShadow->pNext;
			}
			else
			{
				pOldShadow->pNext = pShadow->pNext;
			}

			// Delete memory assosiated with item
			MemFree( pShadow );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldShadow = pShadow;
		pShadow = pShadow->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


static BOOLEAN RemoveStructShadowPartner(UINT32 iMapIndex, UINT16 usIndex)
{
	LEVELNODE	*pShadow		 = NULL;
	LEVELNODE	*pOldShadow  = NULL;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	// Look through all shadows and remove index if found

	while( pShadow != NULL )
	{
		if ( pShadow->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldShadow == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pShadowHead = pShadow->pNext;
			}
			else
			{
				pOldShadow->pNext = pShadow->pNext;
			}

			// Delete memory assosiated with item
			MemFree( pShadow );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldShadow = pShadow;
		pShadow = pShadow->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN RemoveAllShadowsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	LEVELNODE	*pShadow		 = NULL;
	LEVELNODE	*pOldShadow		 = NULL;
	UINT32				fTileType;
	BOOLEAN fRetVal = FALSE;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	// Look through all shadows and Search for type

	while( pShadow != NULL )
	{

		if ( pShadow->usIndex != NO_TILE )
		{

			GetTileType( pShadow->usIndex, &fTileType );

			// Advance to next
			pOldShadow = pShadow;
			pShadow = pShadow->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				// Remove Item
				RemoveShadow( iMapIndex, pOldShadow->usIndex );
				fRetVal = TRUE;
			}

		}

	}
	return fRetVal;
}

BOOLEAN RemoveAllShadows( UINT32 iMapIndex )
{
	LEVELNODE	*pShadow		 = NULL;
	LEVELNODE	*pOldShadow		 = NULL;
	BOOLEAN fRetVal = FALSE;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	// Look through all shadows and Search for type

	while( pShadow != NULL )
	{

		if ( pShadow->usIndex != NO_TILE )
		{

			// Advance to next
			pOldShadow = pShadow;
			pShadow = pShadow->pNext;

			// Remove Item
			RemoveShadow( iMapIndex, pOldShadow->usIndex );
			fRetVal = TRUE;
		}

	}
	return fRetVal;
}

BOOLEAN TypeExistsInShadowLayer( UINT32 iMapIndex, UINT32 fType, UINT16 *pusShadowIndex )
{
	LEVELNODE			*pShadow		 = NULL;

	pShadow = gpWorldLevelData[ iMapIndex ].pShadowHead;

	return( TypeExistsInLevel( pShadow, fType, pusShadowIndex ) );
}


// Merc layer
// #################################################################


static BOOLEAN AddMercStructureInfo(INT16 sGridNo, SOLDIERTYPE* pSoldier);


BOOLEAN AddMercToHead( UINT32 iMapIndex, SOLDIERTYPE *pSoldier, BOOLEAN fAddStructInfo )
{
	LEVELNODE							 *pMerc		 = NULL;
	LEVELNODE							 *pNextMerc		 = NULL;

	pMerc = gpWorldLevelData[ iMapIndex ].pMercHead;

	// Allocate head
	CHECKF( CreateLevelNode( &pNextMerc ) != FALSE );
	pNextMerc->pNext = pMerc;
	pNextMerc->pSoldier				= pSoldier;
	pNextMerc->uiFlags				|= LEVELNODE_SOLDIER;

	// Add structure info if we want
	if ( fAddStructInfo )
	{
		// Set soldier's levelnode
		pSoldier->pLevelNode	= pNextMerc;

		AddMercStructureInfo( (UINT16)iMapIndex, pSoldier );
	}

	// Set head
	gpWorldLevelData[ iMapIndex ].pMercHead = pNextMerc;


	ResetSpecificLayerOptimizing( TILES_DYNAMIC_MERCS | TILES_DYNAMIC_STRUCT_MERCS | TILES_DYNAMIC_HIGHMERCS );
	return( TRUE );

}


static BOOLEAN AddMercStructureInfo(INT16 sGridNo, SOLDIERTYPE* pSoldier)
{
	UINT16 usAnimSurface;

	// Get surface data
 	usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );

	AddMercStructureInfoFromAnimSurface( sGridNo, pSoldier, usAnimSurface, pSoldier->usAnimState );

	return( TRUE );
}


BOOLEAN AddMercStructureInfoFromAnimSurface( INT16 sGridNo, SOLDIERTYPE *pSoldier, UINT16 usAnimSurface, UINT16 usAnimState )
{
	STRUCTURE_FILE_REF		*pStructureFileRef;
	BOOLEAN								fReturn;

	// Turn off multi tile flag...
	pSoldier->uiStatusFlags &= ( ~SOLDIER_MULTITILE );

	if ( pSoldier->pLevelNode == NULL )
	{
		return( FALSE );
	}

	if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
	{
		return( FALSE );
	}

	// Remove existing structs
	DeleteStructureFromWorld( pSoldier->pLevelNode->pStructureData );
	pSoldier->pLevelNode->pStructureData = NULL;

	pStructureFileRef = GetAnimationStructureRef( pSoldier->ubID, usAnimSurface, usAnimState );

	// Now check if we are multi-tiled!
	if ( pStructureFileRef != NULL )
	{
		if ( pSoldier->ubBodyType == QUEENMONSTER )
		{
			// Queen uses onely one direction....
			fReturn = AddStructureToWorld( sGridNo, pSoldier->bLevel, &( pStructureFileRef->pDBStructureRef[ 0 ] ), pSoldier->pLevelNode );
		}
		else
		{
			fReturn = AddStructureToWorld( sGridNo, pSoldier->bLevel, &( pStructureFileRef->pDBStructureRef[ gOneCDirection[ pSoldier->bDirection ] ] ), pSoldier->pLevelNode );
		}
/*
		if ( fReturn == FALSE )
		{
			// try to add default
			ScreenMsg( MSG_FONT_YELLOW, MSG_DEBUG, L"FAILED: add struct info for merc: %d, at %d direction %d, trying default instead", pSoldier->ubID, sGridNo, pSoldier->bDirection );

			pStructureFileRef = GetDefaultStructureRef( pSoldier->ubID );
			if ( pStructureFileRef )
			{
				fReturn = AddStructureToWorld( sGridNo, pSoldier->bLevel, &( pStructureFileRef->pDBStructureRef[ gOneCDirection[ pSoldier->bDirection ] ] ), pSoldier->pLevelNode );
			}
		}
		*/

		if ( fReturn == FALSE )
		{

			// Debug msg
			ScreenMsg(MSG_FONT_RED, MSG_DEBUG, L"FAILED: add struct info for merc %d (%S), at %d direction %d", pSoldier->ubID, pSoldier->name, sGridNo, pSoldier->bDirection);

			if ( pStructureFileRef->pDBStructureRef[ gOneCDirection[ pSoldier->bDirection ] ].pDBStructure->ubNumberOfTiles > 1 )
			{
				// If we have more than one tile
				pSoldier->uiStatusFlags |= SOLDIER_MULTITILE_Z;
			}


			return( FALSE );
		}
		else
		{
			// Turn on if we are multi-tiled
			if ( pSoldier->pLevelNode->pStructureData->pDBStructureRef->pDBStructure->ubNumberOfTiles > 1 )
			{
				// If we have more than one tile
				pSoldier->uiStatusFlags |= SOLDIER_MULTITILE_Z;
			}
			else
			{
				//pSoldier->uiStatusFlags |= SOLDIER_MULTITILE_NZ;
			}
		}
	}

	return( TRUE );
}

BOOLEAN OKToAddMercToWorld( SOLDIERTYPE *pSoldier, INT8 bDirection )
{
	UINT16 usAnimSurface;
	STRUCTURE_FILE_REF			*pStructFileRef;
	UINT16 usOKToAddStructID;

  //if ( pSoldier->uiStatusFlags & SOLDIER_MULTITILE )
	{
		// Get surface data
 		usAnimSurface = GetSoldierAnimationSurface( pSoldier, pSoldier->usAnimState );
		if ( usAnimSurface == INVALID_ANIMATION_SURFACE )
		{
			return( FALSE );
		}

		pStructFileRef = GetAnimationStructureRef( pSoldier->ubID, usAnimSurface, pSoldier->usAnimState );

		// Now check if we have multi-tile info!
		if ( pStructFileRef != NULL )
		{
			//Try adding struct to this location, if we can it's good!
			if ( pSoldier->pLevelNode && pSoldier->pLevelNode->pStructureData != NULL )
			{
				usOKToAddStructID = pSoldier->pLevelNode->pStructureData->usStructureID;
			}
			else
			{
				usOKToAddStructID = INVALID_STRUCTURE_ID;
			}

			if ( !OkayToAddStructureToWorld( pSoldier->sGridNo, pSoldier->bLevel, &(pStructFileRef->pDBStructureRef[gOneCDirection[ bDirection ]]), usOKToAddStructID ))
			{
				return( FALSE );
			}
		}
	}

	return( TRUE );
}


BOOLEAN UpdateMercStructureInfo( SOLDIERTYPE *pSoldier )
{
	// Remove strucute info!
	if ( pSoldier->pLevelNode == NULL )
	{
		return( FALSE );
	}

	//DeleteStructureFromWorld( pSoldier->pLevelNode->pStructureData );

	// Add new one!
	return( AddMercStructureInfo( pSoldier->sGridNo, pSoldier ) );
}


BOOLEAN RemoveMerc( UINT32 iMapIndex, SOLDIERTYPE *pSoldier, BOOLEAN fPlaceHolder )
{
	LEVELNODE	*pMerc		 = NULL;
	LEVELNODE	*pOldMerc	 = NULL;
	BOOLEAN		fMercFound;

	if ( iMapIndex == NOWHERE )
	{
		return( FALSE );
	}

	pMerc = gpWorldLevelData[ iMapIndex ].pMercHead;

	// Look through all mercs and remove index if found

	while( pMerc != NULL )
	{
		fMercFound = FALSE;

		if ( pMerc->pSoldier == pSoldier )
		{
			// If it's a placeholder, check!
			if ( fPlaceHolder )
			{
				if ( ( pMerc->uiFlags & LEVELNODE_MERCPLACEHOLDER ) )
				{
					fMercFound = TRUE;
				}
			}
			else
			{
				if ( !( pMerc->uiFlags & LEVELNODE_MERCPLACEHOLDER ) )
				{
					fMercFound = TRUE;
				}
			}

			if ( fMercFound )
			{
				// OK, set links
				// Check for head or tail
				if ( pOldMerc == NULL )
				{
					// It's the head
					gpWorldLevelData[ iMapIndex ].pMercHead = pMerc->pNext;
				}
				else
				{
					pOldMerc->pNext = pMerc->pNext;
				}

				if ( !fPlaceHolder )
				{
					// Set level node to NULL
					pSoldier->pLevelNode = NULL;

					// Remove strucute info!
					DeleteStructureFromWorld( pMerc->pStructureData );
				}

				// Delete memory assosiated with item
				MemFree( pMerc );
				guiLevelNodes--;

				return( TRUE );
			}
		}

		pOldMerc = pMerc;
		pMerc = pMerc->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}

// Roof layer
// #################################################################

LEVELNODE *AddRoofToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE			*pRoof				 = NULL;
	LEVELNODE			*pNextRoof		 = NULL;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// If we're at the head, set here
	if ( pRoof == NULL )
	{
		CHECKF( CreateLevelNode( &pRoof ) != FALSE );

		if ( usIndex < NUMBEROFTILES )
		{
			if (gTileDatabase[usIndex].pDBStructureRef != NULL)
			{
				if (AddStructureToWorld( (INT16) iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef, pRoof ) == FALSE)
				{
					MemFree( pRoof );
					guiLevelNodes--;
					return( FALSE );
				}
			}
		}
		pRoof->usIndex = usIndex;

		gpWorldLevelData[ iMapIndex ].pRoofHead = pRoof;

		pNextRoof = pRoof;
	}
	else
	{
		while( pRoof != NULL )
		{

			if ( pRoof->pNext == NULL )
			{
				CHECKF( CreateLevelNode( &pNextRoof ) != FALSE );

				if ( usIndex < NUMBEROFTILES )
				{
					if (gTileDatabase[usIndex].pDBStructureRef != NULL)
					{
						if (AddStructureToWorld( (INT16) iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef, pNextRoof ) == FALSE)
						{
							MemFree( pNextRoof );
							guiLevelNodes--;
							return( FALSE );
						}
					}
				}
				pRoof->pNext = pNextRoof;

				pNextRoof->pNext = NULL;
				pNextRoof->usIndex = usIndex;

				break;
			}

			pRoof = pRoof->pNext;

		}

	}


	ResetSpecificLayerOptimizing( TILES_DYNAMIC_ROOF );

	return( pNextRoof );

}


BOOLEAN AddRoofToHead( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE							 *pRoof					= NULL;
	LEVELNODE							 *pNextRoof		  = NULL;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	CHECKF( CreateLevelNode( &pNextRoof ) != FALSE );

	if ( usIndex < NUMBEROFTILES )
	{
		if (gTileDatabase[usIndex].pDBStructureRef != NULL)
		{
			if (AddStructureToWorld( (INT16) iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef, pNextRoof ) == FALSE)
			{
				MemFree( pNextRoof );
				guiLevelNodes--;
				return( FALSE );
			}
		}
	}

	pNextRoof->pNext = pRoof;
	pNextRoof->usIndex = usIndex;


	// Set head
	gpWorldLevelData[ iMapIndex ].pRoofHead = pNextRoof;

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_ROOF );
	return( TRUE );

}


BOOLEAN RemoveRoof( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pRoof		 = NULL;
	LEVELNODE	*pOldRoof	 = NULL;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// Look through all Roofs and remove index if found

	while( pRoof != NULL )
	{
		if ( pRoof->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldRoof == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pRoofHead = pRoof->pNext;
			}
			else
			{
				pOldRoof->pNext = pRoof->pNext;
			}
			// Delete memory assosiated with item
			DeleteStructureFromWorld( pRoof->pStructureData );
			MemFree( pRoof );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldRoof = pRoof;
		pRoof = pRoof->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN TypeExistsInRoofLayer( UINT32 iMapIndex, UINT32 fType, UINT16 *pusRoofIndex )
{
	LEVELNODE			*pRoof		 = NULL;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	return( TypeExistsInLevel( pRoof, fType, pusRoofIndex ) );
}


BOOLEAN TypeRangeExistsInRoofLayer( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT16 *pusRoofIndex )
{
	LEVELNODE	*pRoof		 = NULL;
	LEVELNODE	*pOldRoof	 = NULL;
	UINT32		fTileType;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// Look through all objects and Search for type

	while( pRoof != NULL )
	{
		if ( pRoof->usIndex != NO_TILE )
		{
			GetTileType( pRoof->usIndex, &fTileType );

			// Advance to next
			pOldRoof = pRoof;
			pRoof = pRoof->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				*pusRoofIndex = pOldRoof->usIndex;
				return( TRUE );
			}
		}
	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN IndexExistsInRoofLayer( INT16 sGridNo, UINT16 usIndex )
{
	LEVELNODE	*pRoof		 = NULL;

	pRoof = gpWorldLevelData[ sGridNo ].pRoofHead;

	// Look through all objects and Search for type

	while( pRoof != NULL )
	{
		if ( pRoof->usIndex == usIndex )
		{
			return( TRUE );
		}

		pRoof = pRoof->pNext;
	}

	// Could not find it, return FALSE
	return( FALSE );
}


static void SetAllRoofShadeLevels(UINT32 iMapIndex, UINT8 ubShadeLevel)
{
	LEVELNODE	*pRoof		 = NULL;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	SetLevelShadeLevel( pRoof, ubShadeLevel );
}


static void AdjustAllRoofShadeLevels(UINT32 iMapIndex, INT8 bShadeDiff)
{
	LEVELNODE	*pRoof		 = NULL;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	AdjustLevelShadeLevel( pRoof, bShadeDiff );
}


BOOLEAN RemoveAllRoofsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	LEVELNODE	*pRoof		 = NULL;
	LEVELNODE	*pOldRoof		 = NULL;
	UINT32				fTileType;
	BOOLEAN fRetVal = FALSE;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// Look through all Roofs and Search for type

	while( pRoof != NULL )
	{

		if ( pRoof->usIndex != NO_TILE )
		{

			GetTileType( pRoof->usIndex, &fTileType );

			// Advance to next
			pOldRoof = pRoof;
			pRoof = pRoof->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				// Remove Item
				RemoveRoof( iMapIndex, pOldRoof->usIndex );
				fRetVal = TRUE;
			}

		}

	}

	// Could not find it, return FALSE

	return fRetVal;

}

void RemoveRoofIndexFlagsFromTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT32 uiFlags  )
{
	LEVELNODE	*pRoof		 = NULL;
	LEVELNODE	*pOldRoof		 = NULL;
	UINT32				fTileType;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// Look through all Roofs and Search for type

	while( pRoof != NULL )
	{

		if ( pRoof->usIndex != NO_TILE )
		{

			GetTileType( pRoof->usIndex, &fTileType );

			// Advance to next
			pOldRoof = pRoof;
			pRoof = pRoof->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				pOldRoof->uiFlags &= ( ~uiFlags );
			}

		}

	}

}

void SetRoofIndexFlagsFromTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType, UINT32 uiFlags  )
{
	LEVELNODE	*pRoof		 = NULL;
	LEVELNODE	*pOldRoof		 = NULL;
	UINT32				fTileType;

	pRoof = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// Look through all Roofs and Search for type

	while( pRoof != NULL )
	{

		if ( pRoof->usIndex != NO_TILE )
		{

			GetTileType( pRoof->usIndex, &fTileType );

			// Advance to next
			pOldRoof = pRoof;
			pRoof = pRoof->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				pOldRoof->uiFlags |= uiFlags;
			}

		}

	}

}


// OnRoof layer
// #################################################################

LEVELNODE *AddOnRoofToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE			*pOnRoof				 = NULL;
	LEVELNODE			*pNextOnRoof		 = NULL;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	// If we're at the head, set here
	if ( pOnRoof == NULL )
	{
		CHECKF( CreateLevelNode( &pOnRoof ) != FALSE );

		if ( usIndex < NUMBEROFTILES )
		{
			if (gTileDatabase[usIndex].pDBStructureRef != NULL)
			{
				if (AddStructureToWorld( (INT16) iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef, pOnRoof ) == FALSE)
				{
					MemFree( pOnRoof );
					guiLevelNodes--;
					return( FALSE );
				}
			}
		}
		pOnRoof->usIndex = usIndex;

		gpWorldLevelData[ iMapIndex ].pOnRoofHead = pOnRoof;

		ResetSpecificLayerOptimizing( TILES_DYNAMIC_ONROOF );
		return( pOnRoof );

	}
	else
	{
		while( pOnRoof != NULL )
		{

			if ( pOnRoof->pNext == NULL )
			{
				CHECKF( CreateLevelNode( &pNextOnRoof ) != FALSE );

				if ( usIndex < NUMBEROFTILES )
				{
					if (gTileDatabase[usIndex].pDBStructureRef != NULL)
					{
						if (AddStructureToWorld( (INT16) iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef, pNextOnRoof ) == FALSE)
						{
							MemFree( pNextOnRoof );
							guiLevelNodes--;
							return( NULL );
						}
					}
				}

				pOnRoof->pNext = pNextOnRoof;

				pNextOnRoof->pNext = NULL;
				pNextOnRoof->usIndex = usIndex;
				break;
			}

			pOnRoof = pOnRoof->pNext;

		}

	}

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_ONROOF );
	return( pNextOnRoof );

}


BOOLEAN AddOnRoofToHead( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE							 *pOnRoof					= NULL;
	LEVELNODE							 *pNextOnRoof		  = NULL;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	CHECKF( CreateLevelNode( &pNextOnRoof ) != FALSE );
	if ( usIndex < NUMBEROFTILES )
	{
		if (gTileDatabase[usIndex].pDBStructureRef != NULL)
		{
			if (AddStructureToWorld( (INT16) iMapIndex, 1, gTileDatabase[usIndex].pDBStructureRef, pNextOnRoof ) == FALSE)
			{
				MemFree( pNextOnRoof );
				guiLevelNodes--;
				return( FALSE );
			}
		}
	}

	pNextOnRoof->pNext = pOnRoof;
	pNextOnRoof->usIndex = usIndex;


	// Set head
	gpWorldLevelData[ iMapIndex ].pOnRoofHead = pNextOnRoof;

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_ONROOF );
	return( TRUE );

}


BOOLEAN RemoveOnRoof( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pOnRoof		 = NULL;
	LEVELNODE	*pOldOnRoof	 = NULL;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	// Look through all OnRoofs and remove index if found

	while( pOnRoof != NULL )
	{
		if ( pOnRoof->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldOnRoof == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pOnRoofHead = pOnRoof->pNext;
			}
			else
			{
				pOldOnRoof->pNext = pOnRoof->pNext;
			}

			// REMOVE ONROOF!
			MemFree( pOnRoof );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldOnRoof = pOnRoof;
		pOnRoof = pOnRoof->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN RemoveOnRoofFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode )
{
	LEVELNODE	*pOnRoof		 = NULL;
	LEVELNODE	*pOldOnRoof	 = NULL;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	// Look through all OnRoofs and remove index if found

	while( pOnRoof != NULL )
	{
		if ( pOnRoof == pNode )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldOnRoof == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pOnRoofHead = pOnRoof->pNext;
			}
			else
			{
				pOldOnRoof->pNext = pOnRoof->pNext;
			}

			// REMOVE ONROOF!
			MemFree( pOnRoof );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldOnRoof = pOnRoof;
		pOnRoof = pOnRoof->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


static BOOLEAN TypeExistsInOnRoofLayer(UINT32 iMapIndex, UINT32 fType, UINT16* pusOnRoofIndex)
{
	LEVELNODE			*pOnRoof		 = NULL;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	return( TypeExistsInLevel( pOnRoof, fType, pusOnRoofIndex ) );
}


static void SetAllOnRoofShadeLevels(UINT32 iMapIndex, UINT8 ubShadeLevel)
{
	LEVELNODE	*pOnRoof		 = NULL;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	SetLevelShadeLevel( pOnRoof, ubShadeLevel );
}


static void AdjustAllOnRoofShadeLevels(UINT32 iMapIndex, INT8 bShadeDiff)
{
	LEVELNODE	*pOnRoof		 = NULL;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	AdjustLevelShadeLevel( pOnRoof, bShadeDiff );
}


BOOLEAN RemoveAllOnRoofsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	LEVELNODE	*pOnRoof		 = NULL;
	LEVELNODE	*pOldOnRoof		 = NULL;
	UINT32				fTileType;
	BOOLEAN fRetVal = FALSE;

	pOnRoof = gpWorldLevelData[ iMapIndex ].pOnRoofHead;

	// Look through all OnRoofs and Search for type

	while( pOnRoof != NULL )
	{

		if ( pOnRoof->usIndex != NO_TILE )
		{

			GetTileType( pOnRoof->usIndex, &fTileType );

			// Advance to next
			pOldOnRoof = pOnRoof;
			pOnRoof = pOnRoof->pNext;

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				// Remove Item
				RemoveOnRoof( iMapIndex, pOldOnRoof->usIndex );
				fRetVal = TRUE;
			}

		}

	}
	return fRetVal;
}


// Topmost layer
// #################################################################

LEVELNODE  *AddTopmostToTail( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pTopmost		 = NULL;
	LEVELNODE	*pNextTopmost		 = NULL;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	// If we're at the head, set here
	if ( pTopmost == NULL )
	{
		CHECKN( CreateLevelNode( &pNextTopmost ) != FALSE );
		pNextTopmost->usIndex = usIndex;

		gpWorldLevelData[ iMapIndex ].pTopmostHead = pNextTopmost;

	}
	else
	{
		while( pTopmost != NULL )
		{

			if ( pTopmost->pNext == NULL )
			{
				CHECKN( CreateLevelNode( &pNextTopmost ) != FALSE );
				pTopmost->pNext = pNextTopmost;
				pNextTopmost->pNext = NULL;
				pNextTopmost->usIndex = usIndex;

				break;
			}

			pTopmost = pTopmost->pNext;

		}

	}

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_TOPMOST );
	return( pNextTopmost );

}

BOOLEAN AddUIElem( UINT32 iMapIndex, UINT16 usIndex, INT8	sRelativeX, INT8 sRelativeY, LEVELNODE **ppNewNode )
{
	LEVELNODE							 *pTopmost		 = NULL;

	pTopmost = AddTopmostToTail( iMapIndex, usIndex );

	CHECKF( pTopmost != NULL );

	// Set flags
	pTopmost->uiFlags		|= LEVELNODE_USERELPOS;
	pTopmost->sRelativeX = sRelativeX;
	pTopmost->sRelativeY = sRelativeY;

	if ( ppNewNode != NULL )
	{
		*ppNewNode = pTopmost;
	}

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_TOPMOST );
	return( TRUE );
}


static void RemoveUIElem(UINT32 iMapIndex, UINT16 usIndex)
{
	RemoveTopmost( iMapIndex, usIndex );
}


BOOLEAN AddTopmostToHead( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE							 *pTopmost		 = NULL;
	LEVELNODE							 *pNextTopmost		 = NULL;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	// Allocate head
	CHECKF( CreateLevelNode( &pNextTopmost ) != FALSE );
	pNextTopmost->pNext = pTopmost;
	pNextTopmost->usIndex = usIndex;

	// Set head
	gpWorldLevelData[ iMapIndex ].pTopmostHead = pNextTopmost;

	ResetSpecificLayerOptimizing( TILES_DYNAMIC_TOPMOST );
	return( TRUE );

}


BOOLEAN RemoveTopmost( UINT32 iMapIndex, UINT16 usIndex )
{
	LEVELNODE	*pTopmost		 = NULL;
	LEVELNODE *pOldTopmost		 = NULL;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	// Look through all topmosts and remove index if found

	while( pTopmost != NULL )
	{
		if ( pTopmost->usIndex == usIndex )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldTopmost == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pTopmostHead = pTopmost->pNext;
			}
			else
			{
				pOldTopmost->pNext = pTopmost->pNext;
			}

			// Delete memory assosiated with item
			MemFree( pTopmost );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldTopmost = pTopmost;
		pTopmost = pTopmost->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN RemoveTopmostFromLevelNode( UINT32 iMapIndex, LEVELNODE *pNode )
{
	LEVELNODE	*pTopmost		 = NULL;
	LEVELNODE *pOldTopmost		 = NULL;
	UINT16		usIndex;

	usIndex = pNode->usIndex;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	// Look through all topmosts and remove index if found

	while( pTopmost != NULL )
	{
		if ( pTopmost == pNode )
		{
			// OK, set links
			// Check for head or tail
			if ( pOldTopmost == NULL )
			{
				// It's the head
				gpWorldLevelData[ iMapIndex ].pTopmostHead = pTopmost->pNext;
			}
			else
			{
				pOldTopmost->pNext = pTopmost->pNext;
			}

			// Delete memory assosiated with item
			MemFree( pTopmost );
			guiLevelNodes--;

			return( TRUE );
		}

		pOldTopmost = pTopmost;
		pTopmost = pTopmost->pNext;

	}

	// Could not find it, return FALSE

	return( FALSE );

}


BOOLEAN RemoveAllTopmostsOfTypeRange( UINT32 iMapIndex, UINT32 fStartType, UINT32 fEndType )
{
	LEVELNODE	*pTopmost		 = NULL;
	LEVELNODE	*pOldTopmost		 = NULL;
	UINT32				fTileType;
	BOOLEAN fRetVal = FALSE;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	// Look through all topmosts and Search for type

	while( pTopmost != NULL )
	{
		// Advance to next
		pOldTopmost = pTopmost;
		pTopmost = pTopmost->pNext;

		if ( pOldTopmost->usIndex != NO_TILE && pOldTopmost->usIndex < NUMBEROFTILES )
		{
			GetTileType( pOldTopmost->usIndex, &fTileType );

			if ( fTileType >= fStartType && fTileType <= fEndType )
			{
				// Remove Item
				RemoveTopmost( iMapIndex, pOldTopmost->usIndex );
				fRetVal = TRUE;
			}

		}

	}
  return fRetVal;
}

BOOLEAN TypeExistsInTopmostLayer( UINT32 iMapIndex, UINT32 fType, UINT16 *pusTopmostIndex )
{
	LEVELNODE			*pTopmost		 = NULL;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	return( TypeExistsInLevel( pTopmost, fType, pusTopmostIndex ) );
}


static void SetTopmostFlags(UINT32 iMapIndex, UINT32 uiFlags, UINT16 usIndex)
{
	LEVELNODE			*pTopmost		 = NULL;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	SetIndexLevelNodeFlags( pTopmost, uiFlags, usIndex );
}


static void RemoveTopmostFlags(UINT32 iMapIndex, UINT32 uiFlags, UINT16 usIndex)
{
	LEVELNODE			*pTopmost		 = NULL;

	pTopmost = gpWorldLevelData[ iMapIndex ].pTopmostHead;

	RemoveIndexLevelNodeFlags( pTopmost, uiFlags, usIndex );
}


static BOOLEAN SetMapElementShadeLevel(UINT32 uiMapIndex, UINT8 ubShadeLevel)
{
	SetAllLandShadeLevels( uiMapIndex, ubShadeLevel );
	SetAllObjectShadeLevels( uiMapIndex, ubShadeLevel );
	SetAllStructShadeLevels( uiMapIndex, ubShadeLevel );

	return( TRUE );
}


BOOLEAN IsHeigherLevel( INT16 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_NORMAL_ROOF );

	if ( pStructure != NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}

BOOLEAN IsLowerLevel( INT16 sGridNo )
{
	STRUCTURE * pStructure;

	pStructure = FindStructure( sGridNo, STRUCTURE_NORMAL_ROOF );

	if ( pStructure == NULL )
	{
		return( TRUE );
	}

	return( FALSE );
}

BOOLEAN IsRoofVisible( INT16 sMapPos )
{
	STRUCTURE * pStructure;

	if ( !gfBasement )
	{
		pStructure = FindStructure( sMapPos, STRUCTURE_ROOF );

		if ( pStructure != NULL )
		{
			if ( !( gpWorldLevelData[ sMapPos ].uiFlags & MAPELEMENT_REVEALED ) )
			{
				return( TRUE );
			}
		}
	}
	else
	{
		//if ( InARoom( sMapPos, &ubRoom ) )
		{
			//if ( !( gpWorldLevelData[ sMapPos ].uiFlags & MAPELEMENT_REVEALED ) )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}

BOOLEAN IsRoofVisible2( INT16 sMapPos )
{
	STRUCTURE * pStructure;

	if ( !gfBasement )
	{
		pStructure = FindStructure( sMapPos, STRUCTURE_ROOF );

		if ( pStructure != NULL )
		{
			if ( !( gpWorldLevelData[ sMapPos ].uiFlags & MAPELEMENT_REVEALED ) )
			{
				return( TRUE );
			}
		}
	}
	else
	{
		//if ( InARoom( sMapPos, &ubRoom ) )
		{
			if ( !( gpWorldLevelData[ sMapPos ].uiFlags & MAPELEMENT_REVEALED ) )
			{
				return( TRUE );
			}
		}
	}

	return( FALSE );
}


UINT8 WhoIsThere2( INT16 sGridNo, INT8 bLevel )
{
	STRUCTURE * pStructure;

  if ( !GridNoOnVisibleWorldTile( sGridNo ) )
  {
    return( NOBODY );
  }


	if ( gpWorldLevelData[ sGridNo ].pStructureHead != NULL )
	{

		pStructure = gpWorldLevelData[ sGridNo ].pStructureHead;

		while ( pStructure )
		{
			// person must either have their pSoldier->sGridNo here or be non-passable
			if ( (pStructure->fFlags & STRUCTURE_PERSON) && ( !(pStructure->fFlags & STRUCTURE_PASSABLE) || MercPtrs[ pStructure->usStructureID ]->sGridNo == sGridNo) )
			{
				if ( ( bLevel == 0 && pStructure->sCubeOffset == 0 ) || (bLevel > 0 && pStructure->sCubeOffset > 0) )
				{
					// found a person, on the right level!
					// structure ID and merc ID are identical for merc structures
					return( (UINT8) pStructure->usStructureID );
				}
			}
			pStructure = pStructure->pNext;
		}

	}

	return( (UINT8)NOBODY );
}

UINT8	GetTerrainType( INT16 sGridNo )
{
	return( gpWorldLevelData[sGridNo].ubTerrainID );
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

BOOLEAN Water( INT16 sGridNo )
{
	MAP_ELEMENT *			pMapElement;

  if ( sGridNo == NOWHERE )
  {
    return( FALSE );
  }

	pMapElement = &(gpWorldLevelData[sGridNo]);
	if ( pMapElement->ubTerrainID == LOW_WATER || pMapElement->ubTerrainID == MED_WATER || pMapElement->ubTerrainID == DEEP_WATER )
	{
		// check for a bridge!  otherwise...
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

BOOLEAN DeepWater( INT16 sGridNo )
{
	MAP_ELEMENT *			pMapElement;

	pMapElement = &(gpWorldLevelData[sGridNo]);
	if (pMapElement->ubTerrainID == DEEP_WATER)
	{
		// check for a bridge!  otherwise...
		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}

BOOLEAN WaterTooDeepForAttacks( INT16 sGridNo )
{
	return( DeepWater( sGridNo ) );
}

void SetStructAframeFlags( UINT32 iMapIndex, UINT32 uiFlags  )
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct		 = NULL;
	UINT32				uiTileFlags;

	pStruct = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// Look through all Roofs and Search for type
	while( pStruct != NULL )
	{

		if ( pStruct->usIndex != NO_TILE )
		{

			GetTileFlags( pStruct->usIndex, &uiTileFlags );

			// Advance to next
			pOldStruct = pStruct;
			pStruct = pStruct->pNext;

			if ( uiTileFlags & AFRAME_TILE )
			{
				pOldStruct->uiFlags |= uiFlags;
			}

		}

	}

}


static void RemoveStructAframeFlags(UINT32 iMapIndex, UINT32 uiFlags)
{
	LEVELNODE	*pStruct		 = NULL;
	LEVELNODE	*pOldStruct		 = NULL;
	UINT32				uiTileFlags;

	pStruct = gpWorldLevelData[ iMapIndex ].pRoofHead;

	// Look through all Roofs and Search for type
	while( pStruct != NULL )
	{

		if ( pStruct->usIndex != NO_TILE )
		{

			GetTileFlags( pStruct->usIndex, &uiTileFlags );

			// Advance to next
			pOldStruct = pStruct;
			pStruct = pStruct->pNext;

			if ( uiTileFlags & AFRAME_TILE )
			{
				pOldStruct->uiFlags &= ( ~uiFlags );
			}

		}

	}

}

LEVELNODE * FindLevelNodeBasedOnStructure( INT16 sGridNo, STRUCTURE * pStructure )
{
	LEVELNODE *				pLevelNode;

	//ATE: First look on the struct layer.....
	pLevelNode = gpWorldLevelData[sGridNo].pStructHead;
	while( pLevelNode != NULL )
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return( pLevelNode );
		}
		pLevelNode = pLevelNode->pNext;
	}

	// Next the roof layer....
	pLevelNode = gpWorldLevelData[sGridNo].pRoofHead;
	while( pLevelNode != NULL )
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return( pLevelNode );
		}
		pLevelNode = pLevelNode->pNext;
	}

	// Then the object layer....
	pLevelNode = gpWorldLevelData[sGridNo].pObjectHead;
	while( pLevelNode != NULL )
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return( pLevelNode );
		}
		pLevelNode = pLevelNode->pNext;
	}

	// Finally the onroof layer....
	pLevelNode = gpWorldLevelData[sGridNo].pOnRoofHead;
	while( pLevelNode != NULL )
	{
		if (pLevelNode->pStructureData == pStructure)
		{
			return( pLevelNode );
		}
		pLevelNode = pLevelNode->pNext;
	}

	// Assert here if it cannot be found....
	AssertMsg( 0, "FindLevelNodeBasedOnStruct failed.");


	return( NULL );
}

LEVELNODE * FindShadow( INT16 sGridNo, UINT16 usStructIndex )
{
	LEVELNODE *				pLevelNode;
	UINT16						usShadowIndex;

	if (usStructIndex < FIRSTOSTRUCT1 || usStructIndex >= FIRSTSHADOW1)
	{
		return( NULL );
	}

	usShadowIndex = usStructIndex - FIRSTOSTRUCT1 + FIRSTSHADOW1;

	pLevelNode = gpWorldLevelData[sGridNo].pShadowHead;
	while( pLevelNode != NULL )
	{
		if (pLevelNode->usIndex == usShadowIndex)
		{
			break;
		}
		pLevelNode = pLevelNode->pNext;
	}
	return( pLevelNode );

}


void WorldHideTrees( )
{
LEVELNODE *pNode;
UINT32	fTileFlags;
UINT32 cnt;

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		pNode=gpWorldLevelData[ cnt ].pStructHead;
		while(pNode!=NULL)
		{
			GetTileFlags( pNode->usIndex, &fTileFlags );

			if ( fTileFlags & FULL3D_TILE )
			{
				if ( !( pNode->uiFlags & LEVELNODE_REVEALTREES ) )
				{
					pNode->uiFlags |= ( LEVELNODE_REVEALTREES  );
				}
			}
			pNode=pNode->pNext;
		}
	}

	SetRenderFlags(RENDER_FLAG_FULL);
}


void WorldShowTrees( )
{
LEVELNODE *pNode;
UINT32	fTileFlags;
UINT32 cnt;

	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		pNode=gpWorldLevelData[cnt].pStructHead;
		while(pNode!=NULL)
		{
			GetTileFlags( pNode->usIndex, &fTileFlags );

			if ( fTileFlags & FULL3D_TILE )
			{

				if ( ( pNode->uiFlags & LEVELNODE_REVEALTREES ) )
				{
					pNode->uiFlags  &=(~( LEVELNODE_REVEALTREES ) );
				}
			}
			pNode=pNode->pNext;
		}
	}

	SetRenderFlags(RENDER_FLAG_FULL );
}


static void SetWorldFlagsFromNewNode(UINT16 sGridNo, UINT16 usIndex)
{
}


static void RemoveWorldFlagsFromNewNode(UINT16 sGridNo, UINT16 usIndex)
{
}


void SetWallLevelnodeFlags( UINT16 sGridNo, UINT32 uiFlags )
{
	LEVELNODE	*pStruct		 = NULL;

	pStruct = gpWorldLevelData[ sGridNo ].pStructHead;

	// Look through all objects and Search for type

	while( pStruct != NULL )
	{

		if ( pStruct->pStructureData != NULL )
		{
			// See if we are a wall!
			if ( pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF )
			{
				pStruct->uiFlags |= uiFlags;
			}
		}
		// Advance to next
		pStruct = pStruct->pNext;

	}
}

void RemoveWallLevelnodeFlags( UINT16 sGridNo, UINT32 uiFlags )
{
	LEVELNODE	*pStruct		 = NULL;

	pStruct = gpWorldLevelData[ sGridNo ].pStructHead;

	// Look through all objects and Search for type

	while( pStruct != NULL )
	{
		if ( pStruct->pStructureData != NULL )
		{
			// See if we are a wall!
			if ( pStruct->pStructureData->fFlags & STRUCTURE_WALLSTUFF )
			{
				pStruct->uiFlags &= (~uiFlags);
			}
		}
		// Advance to next
		pStruct = pStruct->pNext;

	}
}



void SetTreeTopStateForMap()
{
	if ( !gGameSettings.fOptions[ TOPTION_TOGGLE_TREE_TOPS ] )
	{
		WorldHideTrees( );
		gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
	}
	else
	{
		WorldShowTrees( );
		gTacticalStatus.uiFlags &= (~NOHIDE_REDUNDENCY);
	}

	// FOR THE NEXT RENDER LOOP, RE-EVALUATE REDUNDENT TILES
	InvalidateWorldRedundency( );
}
