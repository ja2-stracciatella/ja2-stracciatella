#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Smooth.h"
#include "Editor_Undo.h"
#include "Random.h"


INT16 gbSmoothStruct[] =

{
	3, 2, 12, 27, 12, 0,
	5, 2, 15, 30, 39, 0,
	7, 2, 17, 32, 41, 0,
	11, 2, 14, 29, 14, 0,
	8, 2, 13, 28, 38, 0,
	15, 1, 19,  0, 43, 0,
	26, 1, 20,  0, 44, 0,
	12, 2, 18, 33, 42, 0,
	23, 1, 21,  0, 45, 0,
	18, 2, 16, 31, 40, 0,
	21, 1, 22,  0, 46, 0,
	14, 2, 11, 26, 11, 0,
	19, 1, 23,  0, 47, 0,
	16, 2, 24, 34, 48, 0,
	10, 2, 25, 35, 49, 0,
	-1
};

// This is a list of smoothed out water/shoreline textures
// Coding is as such:
// 1st - the corresponding bitvalue for the surrounding water tiles
// 2nd - the number of variations to randomize terrain
// 3rd - the first variation
// 4th - the second variation

INT16 gbSmoothWaterStruct[] =
// FIRST ENTRY		MAJIC NUMBER CALCULATED
// 2nd ENTRY		# OF VARIATIONS
// 3rd, 4rth ENTRY	TILE NUMBERS ( entry 1, entry 2 )
{
	1020, 1, 11, 0,
	1000, 1, 12, 0,
	510, 2, 13, 43,
	190, 1, 14, 0,
	894, 1, 15, 0,
	622, 1, 16, 0,
	1014, 2, 17, 41,
	944, 1, 18, 24,
	872, 1, 19, 0,
	992, 1, 20, 0,
	62, 1, 21, 0,
	190, 2, 22, 14,
	620, 1, 23, 0,
	944, 1, 24, 18,
	878, 2, 25, 32,
	434, 1, 28, 0,
	110, 1, 29,0,
	1010, 1, 30,0,
	876, 2, 31, 32,
	878, 2, 32, 31,
	1004, 2, 32, 31,
	1006, 2, 33, 34,
	1008, 2, 34, 33,
	1016, 2, 33, 34,
	126, 2, 35, 36,
	254, 2, 35, 26,
	638, 2, 36, 26,
	438, 2, 38, 27,
	446, 2, 37, 38,
	950, 2, 37, 27,
	864, 1, 39, 0,
	1040, 1, 40, 0,
	1014, 2, 41, 17,
	432, 1, 42, 0,
	510, 1, 43, 13,
	54, 1, 44, 0,
	108, 1, 45, 0,
	-1
};


static void SmoothWaterTerrain(int gridno, int origType, UINT16* piNewTile, BOOLEAN fForceSmooth);


void SmoothTerrain(int gridno, int origType, UINT16 *piNewTile, BOOLEAN fForceSmooth )
{
	int temp = 0;
	UINT32 cnt;
	BOOLEAN fFound;
	UINT32	uiTempIndex;
	UINT16	land=0;
	BOOLEAN	fSameTile;

	INT16		*pSmoothStruct;
	// Check to see if the orginal texture was water, is so, smooth square/hex with
	// SmoothWaterTerrain rather than the proceeding method
	if( origType==REGWATERTEXTURE)
	{
		SmoothWaterTerrain( gridno, origType, piNewTile, TRUE);
		return;
	}
	pSmoothStruct = gbSmoothStruct;

	// Get land index value for given level and adjust according to type
	LEVELNODE const* const existing_land = FindTypeInLandLayer(gridno, origType);
	if (!existing_land)
	{
		*piNewTile = NO_TILE;
		return;
	}

	UINT16 const usOldIndex = GetTypeSubIndexFromTileIndex(origType, existing_land->usIndex);

	// Check if we're dealing with a 'full' tile ( first ten )
	// If so, do not smooth
	// Onlt do if we are not forcing the smooth
	if ( !fForceSmooth )
	{
		if ( usOldIndex >= 1 && usOldIndex <= 10 )
		{
			*piNewTile = NO_TILE;
			return;
		}
	}

	// is land height one tile above not the same type?
	if ( (gridno- WORLD_COLS ) >= 0 )
	{
		if (!FindTypeInLandLayer(gridno - WORLD_COLS, origType))
		{
			// no it's not
			temp+=3;
		}
	}
	// is land height one tile to the right not the same type?
	// (make sure there IS a tile to the right, i.e. check for border)
	if ((gridno+1)% WORLD_COLS !=0)
	{
		if (!FindTypeInLandLayer(gridno + 1, origType))
		{
			// no it's not
			temp+=5;
		}
	}

	// is land height one tile down not the same type?
	if ( (gridno + WORLD_COLS ) < ( WORLD_COLS * WORLD_ROWS ) )
	{
		if (!FindTypeInLandLayer(gridno + WORLD_COLS, origType))
		{
			// no it's not
			temp+=7;
		}
	}

	// is land height one tile to left not the same type?
	if (gridno % WORLD_COLS!=0)
	{
		if (!FindTypeInLandLayer(gridno - 1, origType))
		{
			// no it's not
			temp+=11;
		}
	}

	// Now, at this point the height (of "temp") will tell us
	// which texture piece to use

	// Loop through smooth struct
	cnt = 0;
	fFound = FALSE;

	while( pSmoothStruct[ cnt ] != -1 )
	{
		if ( pSmoothStruct[ cnt ] == temp )
		{
			fSameTile = FALSE;
			// If water is it's top type
			do
			{
				// CHeck if it's the same tile
				if ( pSmoothStruct[ cnt + 2 ] == usOldIndex )
				{
						fSameTile = TRUE;
				}
				if ( pSmoothStruct[ cnt + 3 ] )
				{
					if ( pSmoothStruct[ cnt + 3 ] == usOldIndex  )
					{
						fSameTile = TRUE;
					}
				}
				if ( fSameTile )
				{
					*piNewTile = NO_TILE;
					return;
				}
				uiTempIndex = Random(pSmoothStruct[ cnt + 1 ]);
				land = pSmoothStruct[ cnt + 2 + uiTempIndex ];
				fFound = TRUE;
			} while( FALSE );
			break;
		}
		cnt += 6;
	}
	if ( !fFound )
	{
		// Check for existance of same tile
		if ( usOldIndex >= 1 && usOldIndex <= 10 )
		{
			*piNewTile = NO_TILE;
			return;
		}
		// this is a "full" tile, so randomize between the
		// five available tiles
		land = Random(10) + 1;
	}
	*piNewTile = GetTileIndexFromTypeSubIndex(origType, land);
}


void SmoothAllTerrainWorld( void )
{
	int	cnt;
	UINT16	NewTile;
	UINT32	uiCheckType;
	// Smooth out entire world surrounding tiles
	for ( cnt = 0; cnt < WORLD_MAX; cnt++ )
	{
		for ( uiCheckType = FIRSTTEXTURE; uiCheckType <= SEVENTHTEXTURE; uiCheckType++ )
		{
			if (FindTypeInLandLayer(cnt, uiCheckType))
			{
				SmoothTerrain( cnt, uiCheckType, &NewTile, TRUE );

				if ( NewTile != NO_TILE )
				{
					// Change tile
					SetLandIndex(cnt, NewTile, uiCheckType);
				}
			}
		}
	}
}

void SmoothTerrainRadius( UINT32 iMapIndex, UINT32 uiCheckType, UINT8 ubRadius, BOOLEAN fForceSmooth )
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2;
	INT32  iNewIndex;
	UINT16 NewTile;
	INT32  leftmost;
	// Don't bother to smooth floors, they don't need them
	if ( uiCheckType >= FIRSTFLOOR && uiCheckType <= LASTFLOOR )
		return;
	// Determine start end end indicies and num rows
	sTop	= ubRadius;
	sBottom = -ubRadius;
	sLeft   = -ubRadius;
	sRight  = ubRadius;
	for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
	{
		leftmost = ( ( iMapIndex + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;
		for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
		{
			iNewIndex = iMapIndex + ( WORLD_COLS * cnt1 ) + cnt2;
			if ( iNewIndex >=0 && iNewIndex < WORLD_MAX &&
				iNewIndex >= leftmost && iNewIndex < ( leftmost + WORLD_COLS ) )
			{
				if (FindTypeInLandLayer(iNewIndex, uiCheckType))
				{
					SmoothTerrain(  iNewIndex, uiCheckType, &NewTile, fForceSmooth );

					if ( NewTile != NO_TILE )
					{
						// Change tile
						AddToUndoList( iNewIndex );
						SetLandIndex(iNewIndex, NewTile, uiCheckType);
					}
				}
			}
		}
	}
}

void SmoothAllTerrainTypeRadius( UINT32 iMapIndex, UINT8 ubRadius, BOOLEAN fForceSmooth )
{
	INT16  sTop, sBottom;
	INT16  sLeft, sRight;
	INT16  cnt1, cnt2, cnt3;
	INT32  iNewIndex;
	UINT16 NewTile;
	INT32  leftmost;
	// Determine start end end indicies and num rows
	sTop	= ubRadius;
	sBottom = -ubRadius;
	sLeft   = -ubRadius;
	sRight  = ubRadius;
	for( cnt3 = FIRSTTEXTURE; cnt3 <= SEVENTHTEXTURE; cnt3++ )
	{
		for( cnt1 = sBottom; cnt1 <= sTop; cnt1++ )
		{
			leftmost = ( ( iMapIndex + ( WORLD_COLS * cnt1 ) )/ WORLD_COLS ) * WORLD_COLS;
			for( cnt2 = sLeft; cnt2 <= sRight; cnt2++ )
			{
				iNewIndex = iMapIndex + ( WORLD_COLS * cnt1 ) + cnt2;
				if ( iNewIndex >=0 && iNewIndex < WORLD_MAX &&
					iNewIndex >= leftmost && iNewIndex < ( leftmost + WORLD_COLS ) )
				{
					if (FindTypeInLandLayer(iNewIndex, cnt3))
					{
						SmoothTerrain(  iNewIndex, cnt3, &NewTile, fForceSmooth );
						if ( NewTile != NO_TILE )
						{
							// Change tile
							SetLandIndex(iNewIndex, NewTile, cnt3);
						}
					}
				}
			}
		}
	}
}


static void SmoothWaterTerrain(int gridno, int origType, UINT16* piNewTile, BOOLEAN fForceSmooth)
{
	// This procedure will calculate the approriate smooth texture for a water texture
	// based on the surrounding water textures. This is done via masking bits within
	// a temp variable, then searching for the right texture and inserting it
	int temp = 0;
	UINT32 cnt;
	BOOLEAN fFound;
	UINT32	uiTempIndex;
	UINT16	land=0;
	BOOLEAN	fSameTile;
	INT16	*pSmoothStruct;

	pSmoothStruct = gbSmoothWaterStruct;
	// Get land index value for given level and adjust according to type
	LEVELNODE const* const existing_land = FindTypeInLandLayer(gridno, origType);
	if (!existing_land)
	{
		*piNewTile = NO_TILE;
		return;
	}

	UINT16 const usOldIndex = GetTypeSubIndexFromTileIndex(origType, existing_land->usIndex);

	// Check if we're dealing with a 'full' tile ( first ten )
	// If so, do not smooth
	// Onlt do if we are not forcing the smooth
	if ( !fForceSmooth )
	{
		if ( usOldIndex >= 1 && usOldIndex <= 10 )
		{
			*piNewTile = NO_TILE;
			return;
		}
	}
	// Mask approriate bits in temp for the lookup in the SmoothWaterStruct list
	if ( (gridno- WORLD_COLS ) >= 0 )
	{
		if (TypeRangeExistsInLandLayer(gridno - WORLD_COLS, origType, origType))
		{
			// no it's not
			temp|=4;
		}
	}
	// is land height one tile to the right not the same type?
	// (make sure there IS a tile to the right, i.e. check for border)
	if ((gridno+1)% WORLD_COLS !=0)
	{
		if (TypeRangeExistsInLandLayer(gridno + 1, origType, origType))
		{
			// no it's not
			temp|=64;
		}
	}
	// is land height one tile down not the same type?
	if ( (gridno + WORLD_COLS ) < ( WORLD_COLS * WORLD_ROWS ) )
	{
		if (TypeRangeExistsInLandLayer(gridno + WORLD_COLS, origType, origType))
		{
			// no it's not
			temp|=256;
		}
	}
	// is land height one tile to left not the same type?
	if (gridno % WORLD_COLS!=0)
	{
		if (TypeRangeExistsInLandLayer(gridno - 1, origType, origType))
		{
			// no it's not
			temp|=16;
		}
	}
	if ((gridno+1)% WORLD_COLS !=0)
	{

		if ( (gridno- WORLD_COLS ) >= 0 )
		{
			if (TypeRangeExistsInLandLayer(gridno - WORLD_COLS + 1, origType, origType))
			{
				// no it's not
				temp|=8;
			}
		}
	}
	if (gridno % WORLD_COLS!=0)
	{
		if ( (gridno- WORLD_COLS ) >= 0 )
		{
			if (TypeRangeExistsInLandLayer(gridno - WORLD_COLS - 1, origType, origType))
			{
				// no it's not
				temp|=2;
			}
		}
	}
	if ((gridno+1)% WORLD_COLS !=0)
	{
		if ( (gridno + WORLD_COLS ) < ( WORLD_COLS * WORLD_ROWS ) )
		{
			if (TypeRangeExistsInLandLayer(gridno + WORLD_COLS + 1, origType, origType))
			{
				// no it's not
				temp|=512;
			}
		}
	}
	if (gridno % WORLD_COLS!=0)
	{
		if ( (gridno + WORLD_COLS ) < ( WORLD_COLS * WORLD_ROWS ) )
		{
			if (TypeRangeExistsInLandLayer(gridno + WORLD_COLS - 1, origType, origType))
			{
				// no it's not
				temp|=128;
			}
		}
	}
	if (TypeRangeExistsInLandLayer(gridno, origType, origType))
	{
		// no it's not
		temp|=32;
	}
	// Loop through smooth struct
	cnt = 0;
	fFound = FALSE;
	// Speed up of this while loop  using double/quick search will result
	// in an incorrect solution due to multiple instances of bitvalues in the list of
	// smoothed water textures
	while( pSmoothStruct[ cnt ] != -1 )
	{
		if ( pSmoothStruct[ cnt ] == temp )
		{
			fSameTile = FALSE;
			do
			{
				// CHeck if it's the same tile
				if ( pSmoothStruct[ cnt + 2 ] == usOldIndex )
				{
					fSameTile = TRUE;
				}
				if ( pSmoothStruct[ cnt + 3 ] )
				{
					if ( pSmoothStruct[ cnt + 3 ] == usOldIndex  )
					{
						fSameTile = TRUE;
					}
				}
				if ( fSameTile )
				{
					*piNewTile = NO_TILE;
					return;
				}
				uiTempIndex = Random(pSmoothStruct[ cnt + 1 ]);
				land = pSmoothStruct[ cnt + 2 + uiTempIndex ];
				fFound = TRUE;
			} while( FALSE );
			break;
		}
		cnt += 4;
	}
	if ( !fFound )
	{
			// Check for existance of same tile
			if ( usOldIndex >= 1 && usOldIndex <= 10 )
			{
				*piNewTile = NO_TILE;
				return;
			}
			land = Random(10) + 1;
	}
	*piNewTile = GetTileIndexFromTypeSubIndex(origType, land);
}
