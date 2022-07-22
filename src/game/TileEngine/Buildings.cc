#include "Structure.h"
#include "TileDef.h"
#include "Types.h"
#include "Buildings.h"
#include "Isometric_Utils.h"
#include "PathAI.h"
#include "AI.h"
#include "Structure_Wrap.h"
#include "Random.h"
#include "Overhead.h"
#include "Render_Fun.h"
#include "StrategicMap.h"
#include "Sys_Globals.h"
#include "WorldMan.h"
#include "Logger.h"

#include <algorithm>

#define ROOF_LOCATION_CHANCE 8

UINT8						gubBuildingInfo[ WORLD_MAX ];
BUILDING				gBuildings[ MAX_BUILDINGS ];
UINT8						gubNumberOfBuildings;


static BUILDING* CreateNewBuilding(UINT8* pubBuilding)
{
	if (gubNumberOfBuildings + 1 >= MAX_BUILDINGS)
	{
		return( NULL );
	}
	// increment # of buildings
	gubNumberOfBuildings++;
	// clear entry
	gBuildings[ gubNumberOfBuildings ].ubNumClimbSpots = 0;
	*pubBuilding = gubNumberOfBuildings;
	// return pointer (have to subtract 1 since we just added 1
	return( &(gBuildings[ gubNumberOfBuildings ]) );
}


static BUILDING* GenerateBuilding(INT16 sDesiredSpot)
{
	UINT32	uiLoop;
	INT16		sTempGridNo, sNextTempGridNo, sVeryTemporaryGridNo;
	INT16		sStartGridNo, sCurrGridNo, sPrevGridNo = NOWHERE, sRightGridNo;
	INT8		bDirection, bTempDirection;
	BOOLEAN	fFoundDir, fFoundWall;
	UINT32	uiChanceIn = ROOF_LOCATION_CHANCE; // chance of a location being considered
	INT16		sWallGridNo;
	INT8		bDesiredOrientation;
	INT8		bSkipSpots = 0;
	SOLDIERTYPE 	FakeSoldier;
	BUILDING *		pBuilding;
	UINT8					ubBuildingID = 0;

	pBuilding = CreateNewBuilding( &ubBuildingID );
	if (!pBuilding)
	{
		return( NULL );
	}

	// set up fake soldier for location testing
	FakeSoldier = SOLDIERTYPE{};
	FakeSoldier.sGridNo = sDesiredSpot;
	FakeSoldier.bLevel = 1;
	FakeSoldier.bTeam = 1;

#ifdef ROOF_DEBUG
	std::fill_n(gsCoverValue, WORLD_MAX, 0x7F7F);
#endif

	// Set reachable
	RoofReachableTest( sDesiredSpot, ubBuildingID );

	// From sGridNo, search until we find a spot that isn't part of the building
	bDirection = NORTHWEST;
	sTempGridNo = sDesiredSpot;
	// using diagonal directions to hopefully prevent picking a
	// spot that
	while( (gpWorldLevelData[ sTempGridNo ].uiFlags & MAPELEMENT_REACHABLE ) )
	{
		sNextTempGridNo = NewGridNo( sTempGridNo, DirectionInc( bDirection ) );
		if ( sTempGridNo == sNextTempGridNo )
		{
			// hit edge of map!??!
			return( NULL );
		}
		else
		{
			sTempGridNo = sNextTempGridNo;
		}
	}

	// we've got our spot
	sStartGridNo = sTempGridNo;

	sCurrGridNo = sStartGridNo;
	sVeryTemporaryGridNo = NewGridNo( sCurrGridNo, DirectionInc( EAST ) );
	if ( gpWorldLevelData[ sVeryTemporaryGridNo ].uiFlags & MAPELEMENT_REACHABLE )
	{
		// go north first
		bDirection = NORTH;
	}
	else
	{
		// go that way (east)
		bDirection = EAST;
	}

	gpWorldLevelData[ sStartGridNo ].ubExtFlags[0] |= MAPELEMENT_EXT_ROOFCODE_VISITED;

	while( 1 )
	{

		// if point to (2 clockwise) is not part of building and is not visited,
		// or is starting point, turn!
		sRightGridNo = NewGridNo(sCurrGridNo, DirectionInc(TwoCDirection(bDirection)));
		sTempGridNo = sRightGridNo;
		if ( ( ( !(gpWorldLevelData[ sTempGridNo ].uiFlags & MAPELEMENT_REACHABLE) && !(gpWorldLevelData[ sTempGridNo ].ubExtFlags[0] & MAPELEMENT_EXT_ROOFCODE_VISITED) ) || (sTempGridNo == sStartGridNo) ) && (sCurrGridNo != sStartGridNo) )
		{
			bDirection = TwoCDirection(bDirection);
			// try in that direction
			continue;
		}

		// if spot ahead is part of building, turn
		sTempGridNo = NewGridNo( sCurrGridNo, DirectionInc( bDirection ) );
		if ( gpWorldLevelData[ sTempGridNo ].uiFlags & MAPELEMENT_REACHABLE )
		{
			// first search for a spot that is neither part of the building or visited

			// we KNOW that the spot in the original direction is blocked, so only loop 3 times
			bTempDirection = TwoCDirection(bDirection);
			fFoundDir = FALSE;
			for ( uiLoop = 0; uiLoop < 3; uiLoop++ )
			{
				sTempGridNo = NewGridNo( sCurrGridNo, DirectionInc( bTempDirection ) );
				if ( !(gpWorldLevelData[ sTempGridNo ].uiFlags & MAPELEMENT_REACHABLE) && !(gpWorldLevelData[ sTempGridNo ].ubExtFlags[0] & MAPELEMENT_EXT_ROOFCODE_VISITED) )
				{
					// this is the way to go!
					fFoundDir = TRUE;
					break;
				}
				bTempDirection = TwoCDirection(bTempDirection);
			}
			if (!fFoundDir)
			{
				// now search for a spot that is just not part of the building
				bTempDirection = TwoCDirection(bDirection);
				fFoundDir = FALSE;
				for ( uiLoop = 0; uiLoop < 3; uiLoop++ )
				{
					sTempGridNo = NewGridNo( sCurrGridNo, DirectionInc( bTempDirection ) );
					if ( !(gpWorldLevelData[ sTempGridNo ].uiFlags & MAPELEMENT_REACHABLE) )
					{
						// this is the way to go!
						fFoundDir = TRUE;
						break;
					}
					bTempDirection = TwoCDirection(bTempDirection);
				}
				if (!fFoundDir)
				{
					// WTF is going on?
					return( NULL );
				}
			}
			bDirection = bTempDirection;
			// try in that direction
			continue;
		}

		// move ahead
		sPrevGridNo = sCurrGridNo;
		sCurrGridNo = sTempGridNo;
		sRightGridNo = NewGridNo( sCurrGridNo, DirectionInc(TwoCDirection(bDirection)));

#ifdef ROOF_DEBUG
		if (gsCoverValue[sCurrGridNo] == 0x7F7F)
		{
			gsCoverValue[sCurrGridNo] = 1;
		}
		else if (gsCoverValue[sCurrGridNo] >= 0)
		{
			gsCoverValue[sCurrGridNo]++;
		}

		SLOGD("Roof code visits {}", sCurrGridNo);
#endif
		if (sCurrGridNo == sPrevGridNo)
		{
			// not progressing, we are just repeating the same gridNo
			SLOGW("Dead loop detected in GenerateBuilding. This may indicate a problem with the current map. Probably reached edge of map. (starting GridNo:{})", sStartGridNo);
			break;
		}

		if (sCurrGridNo == sStartGridNo)
		{
			// done
			break;
		}

		if ( !(gpWorldLevelData[ sCurrGridNo ].ubExtFlags[0] & MAPELEMENT_EXT_ROOFCODE_VISITED) )
		{
			gpWorldLevelData[ sCurrGridNo ].ubExtFlags[0] |= MAPELEMENT_EXT_ROOFCODE_VISITED;

			// consider this location as possible climb gridno
			// there must be a regular wall adjacent to this for us to consider it a
			// climb gridno

			// if the direction is east or north, the wall would be in our gridno;
			// if south or west, the wall would be in the gridno two clockwise
			fFoundWall = FALSE;

			switch( bDirection )
			{
				case NORTH:
					sWallGridNo = sCurrGridNo;
					bDesiredOrientation = OUTSIDE_TOP_RIGHT;
					break;
				case EAST:
					sWallGridNo = sCurrGridNo;
					bDesiredOrientation = OUTSIDE_TOP_LEFT;
					break;
				case SOUTH:
					sWallGridNo = (INT16)(sCurrGridNo + DirectionInc(TwoCDirection(bDirection)));
					bDesiredOrientation = OUTSIDE_TOP_RIGHT;
					break;
				case WEST:
					sWallGridNo = (INT16)(sCurrGridNo + DirectionInc(TwoCDirection(bDirection)));
					bDesiredOrientation = OUTSIDE_TOP_LEFT;
					break;
				default:
					// what the heck?
					return( NULL );
			}

			if (bDesiredOrientation == OUTSIDE_TOP_LEFT)
			{
				if (WallExistsOfTopLeftOrientation( sWallGridNo ))
				{
					fFoundWall = TRUE;
				}
			}
			else
			{
				if (WallExistsOfTopRightOrientation( sWallGridNo ))
				{
					fFoundWall = TRUE;
				}
			}

			if (fFoundWall)
			{
				if (bSkipSpots > 0)
				{
					bSkipSpots--;
				}
				else if ( Random( uiChanceIn ) == 0 )
				{
					// don't consider people as obstacles
					if ( NewOKDestination( &FakeSoldier, sCurrGridNo, FALSE, 0 ) )
					{
						pBuilding->sUpClimbSpots[ pBuilding->ubNumClimbSpots ] = sCurrGridNo;
						pBuilding->sDownClimbSpots[ pBuilding->ubNumClimbSpots ] = sRightGridNo;
						pBuilding->ubNumClimbSpots++;

						if ( pBuilding->ubNumClimbSpots == MAX_CLIMBSPOTS_PER_BUILDING)
						{
							// gotta stop!
							return( pBuilding );
						}

						// if location is added as a spot, reset uiChanceIn
						uiChanceIn = ROOF_LOCATION_CHANCE;
#ifdef ROOF_DEBUG
						gsCoverValue[sCurrGridNo] = 99;
#endif
						// skip the next spot
						bSkipSpots = 1;
					}
					else
					{
						// if location is not added, 100% chance of handling next location
						// and the next until we can add one
						uiChanceIn = 1;

					}
				}
				else
				{
					// didn't pick this location, so increase chance that next location
					// will be considered
					if (uiChanceIn > 2)
					{
						uiChanceIn--;
					}
				}

			}
			else
			{
				// can't select this spot
				if ( (sPrevGridNo != NOWHERE) && (pBuilding->ubNumClimbSpots > 0) )
				{
					if ( pBuilding->sDownClimbSpots[ pBuilding->ubNumClimbSpots - 1 ] == sCurrGridNo )
					{
						// unselect previous spot
						pBuilding->ubNumClimbSpots--;
						// overwrote a selected spot so go into automatic selection for later
						uiChanceIn = 1;
#ifdef ROOF_DEBUG
						// reset marker
						gsCoverValue[sPrevGridNo] = 1;
#endif
					}
				}

				// skip the next gridno
				bSkipSpots = 1;
			}

		}

	}

	// at end could prune # of locations if there are too many

/*
#ifdef ROOF_DEBUG
	SetRenderFlags( RENDER_FLAG_FULL );
	RenderWorld();
	RenderCoverDebug( );
	InvalidateScreen( );
	RefreshScreen();
#endif
*/

	return( pBuilding );
}

BUILDING * FindBuilding( INT16 sGridNo )
{
	UINT8					ubBuildingID;

	if (sGridNo <= 0 || sGridNo >= WORLD_MAX)
	{
		return( NULL );
	}

	// id 0 indicates no building
	ubBuildingID = gubBuildingInfo[ sGridNo ];
	if ( ubBuildingID == NO_BUILDING )
	{
		return( NULL );
		/*
		// need extra checks to see if is valid spot...
		// must have valid room information and be a flat-roofed
		// building
		if (GetRoom(sGridNo) != NO_ROOM && FindStructure(sGridNo, STRUCTURE_NORMAL_ROOF) != NULL)
		{
			return( GenerateBuilding( sGridNo ) );
		}
		else
		{
			return( NULL );
		}
		*/
	}
	else if ( ubBuildingID > gubNumberOfBuildings ) // huh?
	{
		return( NULL );
	}

	return( &(gBuildings[ ubBuildingID ]) );
}


void GenerateBuildings( void )
{
	// init building structures and variables
	std::fill_n(gubBuildingInfo, WORLD_MAX, 0);
	std::fill_n(gBuildings, MAX_BUILDINGS, BUILDING{});
	gubNumberOfBuildings = 0;

	if (gWorldSector.z > 0 || gfEditMode)
	{
		return;
	}

	// reset ALL reachable flags
	// do once before we start building generation for
	// whole map
	FOR_EACH_WORLD_TILE(i)
	{
		i->uiFlags       &= ~MAPELEMENT_REACHABLE;
		i->ubExtFlags[0] &= ~MAPELEMENT_EXT_ROOFCODE_VISITED;
	}

	// search through world
	// for each location in a room try to find building info

	for (GridNo loop = 0; loop < WORLD_MAX; ++loop)
	{
		if ( (gubWorldRoomInfo[ loop ] != NO_ROOM) && (gubBuildingInfo[ loop ] == NO_BUILDING) && (FindStructure( loop, STRUCTURE_NORMAL_ROOF ) != NULL) )
		{
			GenerateBuilding( loop );
		}
	}
}

INT16 FindClosestClimbPoint( INT16 sStartGridNo, INT16 sDesiredGridNo, BOOLEAN fClimbUp )
{
	BUILDING *	pBuilding;
	UINT8				ubNumClimbSpots;
	INT16 *			psClimbSpots;
	UINT8				ubLoop;
	INT16				sDistance, sClosestDistance = 1000, sClosestSpot= NOWHERE;

	pBuilding = FindBuilding( sDesiredGridNo );
	if (!pBuilding)
	{
		return( NOWHERE );
	}

	ubNumClimbSpots = pBuilding->ubNumClimbSpots;

	if (fClimbUp)
	{
		psClimbSpots = pBuilding->sUpClimbSpots;
	}
	else
	{
		psClimbSpots = pBuilding->sDownClimbSpots;
	}

	for ( ubLoop = 0; ubLoop < ubNumClimbSpots; ubLoop++ )
	{
		if (WhoIsThere2(pBuilding->sUpClimbSpots[ubLoop],   0) == NULL &&
				WhoIsThere2(pBuilding->sDownClimbSpots[ubLoop], 1) == NULL)
		{
			sDistance = PythSpacesAway( sStartGridNo, psClimbSpots[ ubLoop ] );
			if (sDistance < sClosestDistance )
			{
				sClosestDistance = sDistance;
				sClosestSpot = psClimbSpots[ ubLoop ];
			}
		}
	}

	return( sClosestSpot );
}

BOOLEAN SameBuilding( INT16 sGridNo1, INT16 sGridNo2 )
{
	if ( gubBuildingInfo[ sGridNo1 ] == NO_BUILDING )
	{
		return( FALSE );
	}
	if ( gubBuildingInfo[ sGridNo2 ] == NO_BUILDING )
	{
		return( FALSE );
	}
	return gubBuildingInfo[sGridNo1] == gubBuildingInfo[sGridNo2];
}
