#include <stdexcept>

#include "HImage.h"
#include "Structure.h"
#include "TileDef.h"
#include "VObject.h"
#include "WorldDef.h"
#include "Debug.h"
#include "WorldMan.h"
#include "Edit_Sys.h"
#include "PathAI.h"
#include "Tile_Surface.h"
#include "MemMan.h"


// GLobals
TILE_ELEMENT		gTileDatabase[ NUMBEROFTILES ];
static UINT16 gTileDatabaseSize;
UINT16					gusNumAnimatedTiles = 0;
UINT16					gusAnimatedTiles[ MAX_ANIMATED_TILES ];



UINT16						gTileTypeStartIndex[ NUMBEROFTILETYPES ];

// These values coorespond to TerrainTypeDefines order
UINT8		gTileTypeMovementCost[ NUM_TERRAIN_TYPES ] =
{
	TRAVELCOST_FLAT,						// NO_TERRAIN
	TRAVELCOST_FLAT,						// FLAT GROUND
	TRAVELCOST_FLATFLOOR,				// FLAT FLOOR
	TRAVELCOST_PAVEDROAD,				// PAVED ROAD
	TRAVELCOST_DIRTROAD,				// DIRT ROAD
	TRAVELCOST_FLAT,						// LOW_GRASS
	TRAVELCOST_FLAT,						// HIGH GRASS
	TRAVELCOST_TRAINTRACKS,			// TRAIN TRACKS
	TRAVELCOST_SHORE,						// LOW WATER
	TRAVELCOST_SHORE,						// MED WATER
	TRAVELCOST_SHORE,						// DEEP WATER
};


void CreateTileDatabase( )
{
	UINT32					cnt1, cnt2;
	UINT8						ubLoop;
	TILE_ELEMENT		TileElement;

	// Loop through all surfaces and tiles and build database
	for( cnt1 = 0; cnt1 < NUMBEROFTILETYPES; cnt1++ )
	{
		// Get number of regions
		const TILE_IMAGERY* const TileSurf = gTileSurfaceArray[cnt1];
		if ( TileSurf != NULL )
		{

			// Build start index list
	    gTileTypeStartIndex[ cnt1 ] = (UINT16)gTileDatabaseSize;

			UINT32 NumRegions = TileSurf->vo->SubregionCount();

			// Check for overflow
			if ( NumRegions > gNumTilesPerType[ cnt1 ] )
			{
				// Cutof
				NumRegions = gNumTilesPerType[ cnt1 ];
			}

			DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Type: %s Size: %d Index: %d", gTileSurfaceName[cnt1], gNumTilesPerType[ cnt1 ], gTileDatabaseSize ) );

			for( cnt2 = 0; cnt2 < NumRegions; cnt2++ )
			{

				memset( &TileElement, 0, sizeof( TileElement ) );
				TileElement.usRegionIndex = (UINT16)cnt2;
				TileElement.hTileSurface	= TileSurf->vo;
				TileElement.sBuddyNum			= -1;

				// Check for multi-z stuff
				if ( TileSurf->vo->ppZStripInfo != NULL )
				{
					// Only do this if we are within the # of video objects
					if (cnt2 < TileSurf->vo->SubregionCount())
					{
						if ( TileSurf->vo->ppZStripInfo[ cnt2 ] != NULL )
						{
							TileElement.uiFlags |= MULTI_Z_TILE;
						}
					}
				}

				// Structure database stuff!
				if (TileSurf->pStructureFileRef != NULL && TileSurf->pStructureFileRef->pubStructureData != NULL)
				{
					if (TileSurf->pStructureFileRef->pDBStructureRef[cnt2].pDBStructure != NULL)
					{
						TileElement.pDBStructureRef	= &(TileSurf->pStructureFileRef->pDBStructureRef[cnt2]);
					}
				}

				TileElement.fType							= (UINT16)TileSurf->fType;
				TileElement.ubTerrainID				= TileSurf->ubTerrainID;
				TileElement.usWallOrientation = NO_ORIENTATION;

				if (TileSurf->pAuxData != NULL)
				{
					if (TileSurf->pAuxData[cnt2].fFlags & AUX_FULL_TILE)
					{
						TileElement.ubFullTile = 1;
					}
					if (TileSurf->pAuxData[cnt2].fFlags & AUX_ANIMATED_TILE)
					{
						// Allocate Animated tile data
						AllocateAnimTileData( &TileElement, TileSurf->pAuxData[cnt2].ubNumberOfFrames );

						// Set values into tile element
						TileElement.pAnimData->bCurrentFrame = TileSurf->pAuxData[cnt2].ubCurrentFrame;
						for (ubLoop = 0; ubLoop < TileElement.pAnimData->ubNumFrames; ubLoop++)
						{
							TileElement.pAnimData->pusFrames[ ubLoop ] = gTileDatabaseSize - TileElement.pAnimData->bCurrentFrame + ubLoop;
						}

/*
						for (ubLoop = TileElement.pAnimData->bCurrentFrame; ubLoop < TileElement.pAnimData->ubNumFrames; ubLoop++)
						{
							TileElement.pAnimData->pusFrames[ ubLoop ] = gTileDatabaseSize - TileElement.pAnimData->bCurrentFrame + ubLoop;
						}
						for (ubLoop = 0; ubLoop < TileElement.pAnimData->bCurrentFrame; ubLoop++)
						{
							TileElement.pAnimData->pusFrames[ ubLoop ] = gTileDatabaseSize - TileElement.pAnimData->bCurrentFrame + ubLoop;
						}

*/

						// set into animation controller array
						gusAnimatedTiles[ gusNumAnimatedTiles ] = gTileDatabaseSize;
						gusNumAnimatedTiles++;

						Assert( gusNumAnimatedTiles <= MAX_ANIMATED_TILES );

						TileElement.uiFlags				 |= ANIMATED_TILE;
					}
					TileElement.usWallOrientation = TileSurf->pAuxData[cnt2].ubWallOrientation;
					if (TileSurf->pAuxData[cnt2].ubNumberOfTiles > 0)
					{
						TileElement.ubNumberOfTiles = TileSurf->pAuxData[cnt2].ubNumberOfTiles;
						TileElement.pTileLocData = TileSurf->pTileLocData + TileSurf->pAuxData[cnt2].usTileLocIndex;
					}
				}

				SetSpecificDatabaseValues( (UINT16)cnt1, gTileDatabaseSize, &TileElement, TileSurf->bRaisedObjectType );

				gTileDatabase[ gTileDatabaseSize ] = TileElement;
				gTileDatabaseSize++;
			}

			// Check if data matches what should be there
			if ( NumRegions < gNumTilesPerType[ cnt1 ] )
			{
				// Do underflows here
				for ( cnt2 = NumRegions; cnt2 < gNumTilesPerType[ cnt1 ]; cnt2++ )
				{
					memset( &TileElement, 0, sizeof( TileElement ) );
					TileElement.usRegionIndex = 0;
					TileElement.hTileSurface	= TileSurf->vo;
					TileElement.fType					= (UINT16)TileSurf->fType;
					TileElement.ubFullTile    = FALSE;
					TileElement.sOffsetHeight = 0;
					TileElement.ubFullTile		= 0;
          TileElement.uiFlags       |= UNDERFLOW_FILLER;

					gTileDatabase[ gTileDatabaseSize ] = TileElement;
					gTileDatabaseSize++;
				}

			}

		}
	}

	//Calculate mem usgae
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Sizes: %d vs %d", gTileDatabaseSize, NUMBEROFTILES ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Types: %d", NUMBEROFTILETYPES ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Item Mem:		%d", gTileDatabaseSize * sizeof( TILE_ELEMENT ) ) );
}


static void FreeAnimTileData(TILE_ELEMENT* pTileElem);


void DeallocateTileDatabase( )
{
	INT32 cnt;

	for( cnt = 0; cnt < NUMBEROFTILES; cnt++ )
	{
		// Check if an existing set of animated tiles are in place, remove if found
		if ( gTileDatabase[ cnt ].pAnimData != NULL )
		{
			FreeAnimTileData( &gTileDatabase[ cnt ] );
		}
	}

	gTileDatabaseSize = 0;
	gusNumAnimatedTiles = 0;
}


#ifdef JA2EDITOR

void SetLandIndex(INT32 const iMapIndex, UINT16 const usIndex, UINT32 const uiNewType)
{
	UINT16	usTempIndex;
	UINT8		ubLastHighLevel=0;

	if ( AnyHeigherLand( iMapIndex, uiNewType, &ubLastHighLevel ) )
	{
			// Check if type exists and get it's index if so
			if ( TypeExistsInLandLayer( iMapIndex, uiNewType, &usTempIndex ) )
			{
				// Replace with new index
				ReplaceLandIndex(iMapIndex, usTempIndex, usIndex);
			}
			else
			{
				InsertLandIndexAtLevel(iMapIndex, usIndex, ubLastHighLevel + 1);
			}
	}
	else
	{

		// Check if type exists and get it's index if so
		if ( TypeExistsInLandLayer( iMapIndex, uiNewType, &usTempIndex ) )
		{
			// Replace with new index
			ReplaceLandIndex(iMapIndex, usTempIndex, usIndex);
		}
		else
		{
			// Otherwise, add to head
			AddLandToHead(iMapIndex, usIndex);
		}
	}
}

#endif


BOOLEAN GetTypeLandLevel( UINT32 iMapIndex, UINT32 uiNewType, UINT8 *pubLevel )
{
	UINT8					level = 0;
	LEVELNODE			*pLand;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	while( pLand != NULL )
	{

		if ( pLand->usIndex != NO_TILE )
		{
			const UINT32 fTileType = GetTileType(pLand->usIndex);
			if ( fTileType == uiNewType )
			{
				*pubLevel = level;
				return( TRUE );
			}

		}

		level++;
		pLand = pLand->pNext;
	}

	return( FALSE );
}


UINT16 GetSubIndexFromTileIndex(const UINT16 usTileIndex)
{
	const UINT32 uiType = GetTileType(usTileIndex);
	return usTileIndex - gTileTypeStartIndex[uiType] + 1;
}


#ifdef JA2EDITOR

UINT16 GetTypeSubIndexFromTileIndex(UINT32 const uiCheckType, UINT16 const usIndex)
{
	// Tile database is zero-based, Type indecies are 1-based!
	if (uiCheckType >= NUMBEROFTILETYPES)
	{
		throw std::logic_error("Tried to get sub-index from invalid tile type");
	}
	return usIndex - gTileTypeStartIndex[uiCheckType] + 1;
}

#endif


UINT16 GetTileIndexFromTypeSubIndex(UINT32 uiCheckType, UINT16 usSubIndex)
{
	Assert(uiCheckType < NUMBEROFTILETYPES);
	// Tile database is zero-based, Type indices are 1-based!
	return usSubIndex + gTileTypeStartIndex[uiCheckType] - 1;
}


// Database access functions
UINT32 GetTileType(const UINT16 usIndex)
{
	Assert(usIndex < lengthof(gTileDatabase));
	return gTileDatabase[usIndex].fType;
}


UINT32 GetTileFlags(const UINT16 usIndex)
{
	Assert(usIndex < lengthof(gTileDatabase));
	return gTileDatabase[usIndex].uiFlags;
}


UINT8 GetTileTypeLogicalHeight(UINT32 fType)
{
	return gTileTypeLogicalHeight[fType];
}


BOOLEAN AnyHeigherLand( UINT32 iMapIndex, UINT32 uiSrcType, UINT8 *pubLastLevel )
{
	LEVELNODE		*pLand		 = NULL;
	UINT8					level = 0;
	UINT8					ubSrcTypeLevel=0;
	BOOLEAN				fFound = FALSE;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	UINT8 ubSrcLogHeight = GetTileTypeLogicalHeight(uiSrcType);

	// Check that src type is not head
	if ( GetTypeLandLevel( iMapIndex, uiSrcType, &ubSrcTypeLevel ) )
	{
		 if ( ubSrcTypeLevel == LANDHEAD )
		 {
				return( FALSE );
		 }
	}

	// Look through all objects and Search for type

	while( pLand != NULL )
	{
		// Get type and height
		const UINT32 fTileType = GetTileType(pLand->usIndex);
		if ( gTileTypeLogicalHeight[ fTileType ] > ubSrcLogHeight )
		{
			*pubLastLevel = level;
			fFound = TRUE;
		}

		// Advance to next
		pLand = pLand->pNext;

		level++;

	}

	// Could not find it, return FALSE
	return( fFound );
}


static BOOLEAN AnyLowerLand(UINT32 iMapIndex, UINT32 uiSrcType, UINT8* pubLastLevel)
{
	LEVELNODE		*pLand		 = NULL;
	UINT8					level = 0;
	UINT8					ubSrcTypeLevel;
  TILE_ELEMENT		TileElem;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	UINT8 ubSrcLogHeight = GetTileTypeLogicalHeight(uiSrcType);

	GetTypeLandLevel( iMapIndex, uiSrcType, &ubSrcTypeLevel );

	// Look through all objects and Search for type
	while( pLand != NULL )
	{
		// Get type and height
		const UINT32 fTileType = GetTileType(pLand->usIndex);

		if ( gTileTypeLogicalHeight[ fTileType ] < ubSrcLogHeight )
		{
			*pubLastLevel = level;
			return( TRUE );
		}

		// Get tile element
		TileElem = gTileDatabase[ pLand->usIndex ];

		// Get full tile flag
		if ( TileElem.ubFullTile && fTileType != uiSrcType )
		{
			return( FALSE );
		}


		// Advance to next
		pLand = pLand->pNext;

		level++;

	}

	// Could not find it, return FALSE
	return( FALSE );

}


UINT16 GetWallOrientation(UINT16 usIndex)
{
	Assert(usIndex != NO_TILE);
	return gTileDatabase[usIndex].usWallOrientation;
}


void AllocateAnimTileData(TILE_ELEMENT* const pTileElem, UINT8 const ubNumFrames)
{
	pTileElem->pAnimData            = MALLOC(TILE_ANIMATION_DATA);
	pTileElem->pAnimData->pusFrames = MALLOCN(UINT16, ubNumFrames);

	// Set # if frames!
	pTileElem->pAnimData->ubNumFrames = ubNumFrames;
}


static void FreeAnimTileData(TILE_ELEMENT* pTileElem)
{
	if ( pTileElem->pAnimData != NULL )
	{
		// Free frames list
		MemFree( pTileElem->pAnimData->pusFrames );

		// Free frames
		MemFree( pTileElem->pAnimData );
	}
}
