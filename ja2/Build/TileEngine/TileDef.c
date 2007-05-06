#include "TileDef.h"
#include "WorldDef.h"
#include "WCheck.h"
#include "Debug.h"
#include "WorldMan.h"
#include "Gameloop.h"
#include "Edit_Sys.h"
#include "PathAI.h"
#include "Tile_Surface.h"
#include "Tactical_Save.h"
#include "MemMan.h"


// GLobals
TILE_ELEMENT		gTileDatabase[ NUMBEROFTILES ];
UINT16					gTileDatabaseSize;
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
	UINT32					NumRegions;
	PTILE_IMAGERY   TileSurf;
	TILE_ELEMENT		TileElement;

	// Loop through all surfaces and tiles and build database
	for( cnt1 = 0; cnt1 < NUMBEROFTILETYPES; cnt1++ )
	{
		// Get number of regions
		TileSurf = gTileSurfaceArray[ cnt1 ];

		if ( TileSurf != NULL )
		{

			// Build start index list
	    gTileTypeStartIndex[ cnt1 ] = (UINT16)gTileDatabaseSize;

			NumRegions = TileSurf->vo->usNumberOfObjects;

			// Check for overflow
			if ( NumRegions > gNumTilesPerType[ cnt1 ] )
			{
				#ifdef JA2EDITOR
					// Display warning
					gfWarning = (UINT8)cnt1;
				#endif

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
					if ( cnt2 < TileSurf->vo->usNumberOfObjects )
					{
						if ( TileSurf->vo->ppZStripInfo[ cnt2 ] != NULL )
						{
							TileElement.uiFlags |= MULTI_Z_TILE;
						}
					}
					else
					{
						//Ate test to see if problems is gone
						int i = 0;
					}
				}

				// Structure database stuff!
				if (TileSurf->pStructureFileRef != NULL && TileSurf->pStructureFileRef->pubStructureData != NULL)
				{
					if (TileSurf->pStructureFileRef->pDBStructureRef[cnt2].pDBStructure != NULL)
					{
						TileElement.pDBStructureRef	= &(TileSurf->pStructureFileRef->pDBStructureRef[cnt2]);

						if ( TileElement.pDBStructureRef->pDBStructure->fFlags & STRUCTURE_HIDDEN )
						{
							// ATE: These are ignored!
							//TileElement.uiFlags |= HIDDEN_TILE;
						}
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
				#ifdef JA2EDITOR
					// Display warning here
					gfWarning = (UINT8)cnt1;
				#endif

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
	gSurfaceMemUsage = guiMemTotal - gSurfaceMemUsage;
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Sizes: %d vs %d", gTileDatabaseSize, NUMBEROFTILES ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Types: %d", NUMBEROFTILETYPES ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Item Mem:		%d", gTileDatabaseSize * sizeof( TILE_ELEMENT ) ) );
	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, String("Database Item Total Mem:		%d", gSurfaceMemUsage ) );

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
	gSurfaceMemUsage = 0;
	gusNumAnimatedTiles = 0;

}


BOOLEAN GetLandHeadType( INT32 iMapIndex, UINT32 *puiType )
{
 UINT16					usIndex;

 Assert( puiType != NULL );

 CHECKF( gpWorldLevelData[ iMapIndex ].pLandHead != NULL );

 usIndex = gpWorldLevelData[ iMapIndex ].pLandHead->usIndex;

 GetTileType( usIndex, puiType );

 return( TRUE );

}

BOOLEAN SetLandIndex( INT32 iMapIndex, UINT16 usIndex, UINT32 uiNewType, BOOLEAN fDelete )
{
	UINT16	usTempIndex;
	UINT8		ubLastHighLevel=0;

	if ( fDelete )
	{
		RemoveLand( iMapIndex, usIndex );
		return( TRUE );
	}

	if ( AnyHeigherLand( iMapIndex, uiNewType, &ubLastHighLevel ) )
	{
			// Check if type exists and get it's index if so
			if ( TypeExistsInLandLayer( iMapIndex, uiNewType, &usTempIndex ) )
			{
				// Replace with new index
				return( ReplaceLandIndex( iMapIndex, usTempIndex, usIndex ) );
			}
			else
			{
				return( InsertLandIndexAtLevel( iMapIndex, usIndex, (UINT8)(ubLastHighLevel+1) ) );
			}

	}
	else
	{

		// Check if type exists and get it's index if so
		if ( TypeExistsInLandLayer( iMapIndex, uiNewType, &usTempIndex ) )
		{
			// Replace with new index
			return( ReplaceLandIndex( iMapIndex, usTempIndex, usIndex ) );
		}
		else
		{
			// Otherwise, add to head
			return( AddLandToHead( iMapIndex, usIndex ) );
		}
	}

}


static BOOLEAN SetLandIndexWithRadius(INT32 iMapIndex, UINT16 usIndex, UINT32 uiNewType, UINT8 ubRadius, BOOLEAN fReplace)
{
	UINT16				usTempIndex;
	INT16					sTop, sBottom;
	INT16					sLeft, sRight;
	INT16					cnt1, cnt2;
	INT32				  iNewIndex;
	BOOLEAN				fDoPaste = FALSE;
	INT32					leftmost;
	//BOOLEAN				fNewCommand;

	// Determine start end end indicies and num rows
	sTop		= ubRadius;
	sBottom = -ubRadius;
	sLeft   = - ubRadius;
	sRight  = ubRadius;

	//fNewCommand = fFirstDrawMode; //NEW_UNDO_COMMAND;

	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{

		leftmost = ( ( iMapIndex + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;

		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			iNewIndex = iMapIndex + ( WORLD_COLS * cnt1 ) + cnt2;

			if ( iNewIndex >=0 && iNewIndex < WORLD_MAX &&
				   iNewIndex >= leftmost && iNewIndex < ( leftmost + WORLD_COLS ) )
			{

				if ( fReplace )
				{
					fDoPaste = TRUE;
				}
				else
				{
					if ( !TypeExistsInLandLayer( iNewIndex, uiNewType, &usTempIndex ) )
					{
						fDoPaste = TRUE;
					}
				}

				if ( fDoPaste && ((uiNewType >= FIRSTFLOOR && uiNewType <= LASTFLOOR) ||
												 ((uiNewType < FIRSTFLOOR || uiNewType > LASTFLOOR) &&
						!TypeRangeExistsInLandLayer(iNewIndex, FIRSTFLOOR, LASTFLOOR))))
				{
					SetLandIndex( iNewIndex, usIndex, uiNewType, FALSE );
				}
			}
		}
	}

	return( TRUE );
}


BOOLEAN GetTypeLandLevel( UINT32 iMapIndex, UINT32 uiNewType, UINT8 *pubLevel )
{
	UINT8					level = 0;
	LEVELNODE			*pLand;
	UINT32				fTileType=0;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	while( pLand != NULL )
	{

		if ( pLand->usIndex != NO_TILE )
		{
			GetTileType( pLand->usIndex, &fTileType );

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


static UINT8 GetLandLevelDepth(UINT32 iMapIndex)
{
	UINT8					level = 0;
	LEVELNODE		*pLand;

	pLand = gpWorldLevelData[ iMapIndex ].pLandHead;

	while( pLand != NULL )
	{
		level++;
		pLand = pLand->pNext;
	}

	return( level );

}

BOOLEAN GetSubIndexFromTileIndex( UINT16 usTileIndex, UINT16 *pusSubIndex )
{
	UINT32 uiType=0;
	if( GetTileType( usTileIndex, &uiType ) )
	{
		*pusSubIndex = usTileIndex - gTileTypeStartIndex[ uiType ] + 1;
		return TRUE;
	}
	return FALSE;
}

BOOLEAN GetTypeSubIndexFromTileIndex( UINT32 uiCheckType, UINT16 usIndex, UINT16 *pusSubIndex )
{

	// Tile database is zero-based, Type indecies are 1-based!

	CHECKF ( uiCheckType < NUMBEROFTILETYPES );

	*pusSubIndex = usIndex - gTileTypeStartIndex[ uiCheckType ] + 1;

	return( TRUE );
}

BOOLEAN GetTypeSubIndexFromTileIndexChar( UINT32 uiCheckType, UINT16 usIndex, UINT8 *pubSubIndex )
{

	// Tile database is zero-based, Type indecies are 1-based!

	CHECKF ( uiCheckType < NUMBEROFTILETYPES );

	*pubSubIndex = (UINT8)(usIndex - gTileTypeStartIndex[ uiCheckType ] + 1);

	return( TRUE );
}

BOOLEAN	GetTileIndexFromTypeSubIndex( UINT32 uiCheckType, UINT16 usSubIndex, UINT16 *pusTileIndex )
{
	// Tile database is zero-based, Type indecies are 1-based!

	CHECKF ( uiCheckType < NUMBEROFTILETYPES );

	*pusTileIndex = usSubIndex + gTileTypeStartIndex[ uiCheckType ] - 1;

	return( TRUE );

}


static BOOLEAN MoveLandIndexToTop(UINT32 iMapIndex, UINT16 usIndex)
{

	// Remove, then add again
	RemoveLand( iMapIndex, usIndex );

	AddLandToHead( iMapIndex, usIndex );

	return( TRUE );
}


// Database access functions
BOOLEAN	GetTileType( UINT16 usIndex, UINT32 *puiType )
{
  TILE_ELEMENT		TileElem;

	CHECKF( usIndex != NO_TILE );

	// Get tile element
	TileElem = gTileDatabase[ usIndex ];

	*puiType = TileElem.fType;

	return( TRUE );
}

BOOLEAN	GetTileFlags( UINT16 usIndex, UINT32 *puiFlags )
{
  TILE_ELEMENT		TileElem;

	CHECKF( usIndex != NO_TILE );
	CHECKF( usIndex < NUMBEROFTILES );

	// Get tile element
	TileElem = gTileDatabase[ usIndex ];

	*puiFlags = TileElem.uiFlags;

	return( TRUE );
}


UINT8 GetTileTypeLogicalHeight(UINT32 fType)
{
	return gTileTypeLogicalHeight[fType];
}


static BOOLEAN LandTypeHeigher(UINT32 uiDestType, UINT32 uiSrcType)
{
	// Get logical height of type at head and type we wish to paste
	UINT8 ubDestLogHeight = GetTileTypeLogicalHeight(uiDestType);
	UINT8 ubSrcLogHeight  = GetTileTypeLogicalHeight(uiSrcType);

	return( ubDestLogHeight > ubSrcLogHeight );
}


BOOLEAN AnyHeigherLand( UINT32 iMapIndex, UINT32 uiSrcType, UINT8 *pubLastLevel )
{
	LEVELNODE		*pLand		 = NULL;
	UINT32				fTileType=0;
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
		GetTileType( pLand->usIndex, &fTileType );

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
	UINT32				fTileType=0;
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
		GetTileType( pLand->usIndex, &fTileType );

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


BOOLEAN GetWallOrientation( UINT16 usIndex, UINT16 *pusWallOrientation )
{
  TILE_ELEMENT		TileElem;

	CHECKF( usIndex != NO_TILE );

	// Get tile element
	TileElem = gTileDatabase[ usIndex ];

	*pusWallOrientation = TileElem.usWallOrientation;

	return( TRUE );
}


static BOOLEAN ContainsWallOrientation(INT32 iMapIndex, UINT32 uiType, UINT16 usWallOrientation, UINT8* pubLevel)
{
	LEVELNODE	*pStruct = NULL;
	UINT8					level = 0;
	UINT16				usCheckWallOrient=0;

	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;

	// Look through all objects and Search for type

	while( pStruct != NULL )
	{

		GetWallOrientation( pStruct->usIndex, &usCheckWallOrient );

		if ( usCheckWallOrient == usWallOrientation )
		{
				*pubLevel = level;
				return( TRUE );
		}

		// Advance to next
		pStruct = pStruct->pNext;

		level++;

	}

	// Could not find it, return FALSE
	return( FALSE );
}


//Kris:  This function returns two special types if there are two walls
//sharing the same tile.  This case only happens with the exterior and
//interior bottom corners.  Otherwise, it returns the orientation of the
//first wall encountered -- not that there should be duplicate walls...
static UINT8 CalculateWallOrientationsAtGridNo(INT32 iMapIndex)
{
	UINT16 usCheckWallOrientation=0;
	LEVELNODE *pStruct = NULL;
	UINT8 ubFinalWallOrientation = NO_ORIENTATION;
	pStruct = gpWorldLevelData[ iMapIndex ].pStructHead;
	//Traverse all of the pStructs
	while( pStruct != NULL )
	{
		GetWallOrientation( pStruct->usIndex, &usCheckWallOrientation );
		if( ubFinalWallOrientation == NO_ORIENTATION )
		{	//Get the first valid orientation.
			ubFinalWallOrientation = (UINT8)usCheckWallOrientation;
		}
		else switch( ubFinalWallOrientation )
		{	//If the first valid orientation has the key counterpart orientation,
			//return the special corner orientations.
			case INSIDE_TOP_LEFT:
				if( usCheckWallOrientation == INSIDE_TOP_RIGHT )
					return INSIDE_BOTTOM_CORNER;
				break;
			case INSIDE_TOP_RIGHT:
				if( usCheckWallOrientation == INSIDE_TOP_LEFT )
					return INSIDE_BOTTOM_CORNER;
				break;
			case OUTSIDE_TOP_LEFT:
				if( usCheckWallOrientation == OUTSIDE_TOP_RIGHT )
					return OUTSIDE_BOTTOM_CORNER;
				break;
			case OUTSIDE_TOP_RIGHT:
				if( usCheckWallOrientation == OUTSIDE_TOP_LEFT )
					return OUTSIDE_BOTTOM_CORNER;
				break;
		}
		// Advance to next
		pStruct = pStruct->pNext;
	}
	// Only one wall, so return it's orienation.
	return ubFinalWallOrientation;
}


BOOLEAN AllocateAnimTileData( TILE_ELEMENT *pTileElem, UINT8 ubNumFrames )
{
	pTileElem->pAnimData = MemAlloc( sizeof( TILE_ANIMATION_DATA ) );

	CHECKF( pTileElem->pAnimData != NULL );

	pTileElem->pAnimData->pusFrames = MemAlloc( sizeof( UINT16 ) * ubNumFrames );

	CHECKF( pTileElem->pAnimData->pusFrames != NULL );

	// Set # if frames!
	pTileElem->pAnimData->ubNumFrames = ubNumFrames;

	return( TRUE );
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
