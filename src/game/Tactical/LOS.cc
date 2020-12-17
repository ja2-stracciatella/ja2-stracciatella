#include <math.h>
#include "Font_Control.h"
#include "Handle_Items.h"
#include "Structure.h"
#include "TileDef.h"
#include "Timer_Control.h"
#include "WCheck.h"
#include "Isometric_Utils.h"
#include "Debug.h"
#include "LOS.h"
#include "Animation_Control.h"
#include "Random.h"
#include "Soldier_Control.h"
#include "Overhead.h"
#include "Weapons.h"
#include "OppList.h"
#include "Bullets.h"
#include "Items.h"
#include "Soldier_Profile.h"
#include "WorldMan.h"
#include "Rotting_Corpses.h"
#include "Keys.h"
#include "Message.h"
#include "Structure_Wrap.h"
#include "Campaign.h"
#include "Environment.h"
#include "PathAI.h"
#include "Soldier_Macros.h"
#include "StrategicMap.h"
#include "Quests.h"
#include "Interface.h"
#include "Points.h"
#include "Smell.h"
#include "Text.h"

#include "CalibreModel.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "WeaponModels.h"
#include "Logger.h"

#define STEPS_FOR_BULLET_MOVE_TRAILS				10
#define STEPS_FOR_BULLET_MOVE_SMALL_TRAILS			5
#define STEPS_FOR_BULLET_MOVE_FIRE_TRAILS			5

#define ALWAYS_CONSIDER_HIT					(STRUCTURE_WALLSTUFF | STRUCTURE_CAVEWALL | STRUCTURE_FENCE)


static const FIXEDPT gqStandardWallHeight         = INT32_TO_FIXEDPT(WALL_HEIGHT_UNITS);
static const FIXEDPT gqStandardWindowBottomHeight = INT32_TO_FIXEDPT(WINDOW_BOTTOM_HEIGHT_UNITS);
static const FIXEDPT gqStandardWindowTopHeight    = INT32_TO_FIXEDPT(WINDOW_TOP_HEIGHT_UNITS);


static const DOUBLE ddShotgunSpread[3][BUCKSHOT_SHOTS][2] =
{
	{
		// spread of about 2 degrees in all directions
		// Horiz, Vert
		{0.0, 0.0},
		{-0.012, 0.0},
		{+0.012, 0.0},
		{0.0, -0.012},
		{0.0, +0.012},
		{-0.008, -0.008},
		{-0.008, +0.008},
		{+0.008, -0.008},
		{+0.008, +0.008}
	},
	{
		// duckbill flattens the spread and makes it wider horizontally (5 degrees)
		// Horiz, Vert
		{0.0, 0.0},
		{-0.008, 0.0},
		{+0.008, 0.0},
		{-0.016, 0.0},
		{+0.016, 0.0},
		{-0.024, 0.0},
		{+0.024, 0.0},
		{-0.032, 0.0},
		{+0.032, 0.0},
	},
	{
		// flamethrower more spread out
		// Horiz, Vert
		{0.0, 0.0},
		{-0.120, 0.0},
		{+0.120, 0.0},
		{0.0, -0.120},
		{0.0, +0.120},
		{-0.080, -0.080},
		{-0.080, +0.080},
		{+0.080, -0.080},
		{+0.080, +0.080}
	},

};

static const UINT8 gubTreeSightReduction[ANIM_STAND + 1] =
{
	0,
	8, // prone
	0,
	7, // crouched
	0,
	0,
	6 // standing
};

#define MAX_DIST_FOR_LESS_THAN_MAX_CHANCE_TO_HIT_STRUCTURE	25

#define MAX_CHANCE_OF_HITTING_STRUCTURE			90

static const UINT32 guiStructureHitChance[MAX_DIST_FOR_LESS_THAN_MAX_CHANCE_TO_HIT_STRUCTURE + 1] =
{
	0, // 0 tiles
	0,
	0,
	2,
	4,
	7, // 5 tiles
	10,
	14,
	18,
	23,
	28, // 10 tiles
	34,
	40,
	47,
	54,
	60, // 15 tiles
	66,
	71,
	74,
	76,
	78, // 20 tiles
	80,
	82,
	84,
	86,
	88, // 25 tiles
};


#define PERCENT_BULLET_SLOWED_BY_RANGE				25

#define MIN_DIST_FOR_HIT_FRIENDS				30
#define MIN_DIST_FOR_HIT_FRIENDS_UNAIMED			15
#define MIN_CHANCE_TO_ACCIDENTALLY_HIT_SOMEONE			3

#define RADIANS_IN_CIRCLE					6.283
#define DEGREES_22_5						(RADIANS_IN_CIRCLE * 22.5 / 360)
#define DEGREES_45						(RADIANS_IN_CIRCLE * 45 / 360)
// note: these values are in RADIANS!!
// equal to 15 degrees
#define MAX_AIMING_SCREWUP					(RADIANS_IN_CIRCLE * 15 / 360)
// min aiming screwup is X degrees, gets divided by distance in tiles
#define MIN_AIMING_SCREWUP					(RADIANS_IN_CIRCLE * 22 / 360)
//#define MAX_AIMING_SCREWUP					0.2618
// equal to 10 degrees
//#define MAX_AIMING_SCREWUP_VERTIC				0.1745

#define SMELL_REDUCTION_FOR_NEARBY_OBSTACLE			80

#define STANDING_CUBES						3

// MoveBullet and ChanceToGetThrough use this array to maintain which
// of which structures in a tile might be hit by a bullet.

#define MAX_LOCAL_STRUCTURES					20

static STRUCTURE* gpLocalStructure[MAX_LOCAL_STRUCTURES];
static UINT32     guiLocalStructureCTH[MAX_LOCAL_STRUCTURES];
static UINT8      gubLocalStructureNumTimesHit[MAX_LOCAL_STRUCTURES];


#ifdef LOS_DEBUG
LOSResults gLOSTestResults = {0};
#endif


static FIXEDPT FloatToFixed(FLOAT dN)
{
	FIXEDPT qN;
	// verify that dN is within the range storable by FIXEDPT?

	// first get the whole part
	qN = (INT32) (dN * FIXEDPT_FRACTIONAL_RESOLUTION);

	//qN = INT32_TO_FIXEDPT( (INT32)dN );
	// now add the fractional part
	//qN += (INT32)(((dN - (INT32) dN)) * FIXEDPT_FRACTIONAL_RESOLUTION);

	return( qN );
}


static FLOAT FixedToFloat(FIXEDPT qN)
{
	return( ((FLOAT) qN)	/ FIXEDPT_FRACTIONAL_RESOLUTION );
}

//
// fixed-point arithmetic stuff ends here
//


static FLOAT Distance3D(FLOAT dDeltaX, FLOAT dDeltaY, FLOAT dDeltaZ)
{
	return ((FLOAT) sqrt( (DOUBLE)(dDeltaX * dDeltaX) +
			(DOUBLE)(dDeltaY * dDeltaY) + (DOUBLE)(dDeltaZ * dDeltaZ)));
}


static FLOAT Distance2D(FLOAT dDeltaX, FLOAT dDeltaY)
{
	return( (FLOAT) sqrt( (DOUBLE)(dDeltaX * dDeltaX) + (DOUBLE)(dDeltaY * dDeltaY) ));
}

enum LocationCode
{
	LOC_OTHER,
	LOC_0_4,
	LOC_3_4,
	LOC_4_0,
	LOC_4_3,
	LOC_4_4
};


static BOOLEAN ResolveHitOnWall(STRUCTURE* pStructure, INT32 iGridNo, INT8 bLOSIndexX, INT8 bLOSIndexY, DOUBLE ddHorizAngle)
{
	BOOLEAN fNorthSouth, fEastWest;
	BOOLEAN fTopLeft, fTopRight;
	INT8    bLocation = LOC_OTHER;

	switch ( bLOSIndexX )
	{
		case 0:
			if ( bLOSIndexY == 4 )
			{
				bLocation = LOC_0_4;
			}
			break;
		case 3:
			if ( bLOSIndexY == 4 )
			{
				bLocation = LOC_3_4;
			}
			break;
		case 4:
			switch( bLOSIndexY )
			{
				case 0:
					bLocation = LOC_4_0;
					break;
				case 3:
					bLocation = LOC_4_3;
					break;
				case 4:
					bLocation = LOC_4_4;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	if ( bLocation == LOC_OTHER )
	{
		// these spots always block
		return( TRUE );
	}

	// use cartesian angles for god's sakes -CJC
	ddHorizAngle = -ddHorizAngle;

	fNorthSouth = ((ddHorizAngle < (0) && ddHorizAngle > (-PI * 1 / 2)) ||
			(ddHorizAngle > (PI * 1 / 2) && ddHorizAngle < (PI)));
	fEastWest = ((ddHorizAngle > (0) && ddHorizAngle < (PI * 1 / 2)) ||
			(ddHorizAngle < (-PI * 1 / 2) && ddHorizAngle > (-PI)));

	fTopLeft = (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
			pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT);
	fTopRight = (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
			pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT);

	if ( fNorthSouth )
	{
		// Check N-S at west corner: 4,4 4,3 0,4
		if ( bLocation == LOC_4_3 || bLocation == LOC_4_4 )
		{
			// if wall orientation is top-right, then check S of this location
			// if wall orientation is top-left, then check E of this location
			// if no wall of same orientation there, let bullet through
			if ( fTopRight )
			{
				if (!WallOrClosedDoorExistsOfTopRightOrientation((INT16) (iGridNo + DirectionInc(SOUTH))) &&
					!WallOrClosedDoorExistsOfTopLeftOrientation((INT16) (iGridNo)) &&
					!OpenRightOrientedDoorWithDoorOnRightOfEdgeExists((INT16) (iGridNo + DirectionInc(SOUTH))))
				{
					return( FALSE );
				}

			}

		}
		else if ( bLocation == LOC_0_4 )
		{
			if ( fTopLeft )
			{
				if (!WallOrClosedDoorExistsOfTopLeftOrientation((INT16) (iGridNo + DirectionInc(WEST))) &&
					!WallOrClosedDoorExistsOfTopRightOrientation((INT16) (iGridNo + DirectionInc( SOUTHWEST))) &&
					!OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists((INT16) (iGridNo + DirectionInc( WEST))))
				{
					return( FALSE );
				}
			}

		}

		// Check N-S at east corner: 4,4 3,4 4,0
		if ( bLocation == LOC_4_4 || bLocation == LOC_3_4 )
		{
			if ( fTopLeft )
			{
				if (!WallOrClosedDoorExistsOfTopLeftOrientation((INT16) (iGridNo + DirectionInc(EAST))) &&
					!WallOrClosedDoorExistsOfTopRightOrientation((INT16) (iGridNo)) &&
					!OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists((INT16) (iGridNo + DirectionInc(EAST))))
				{
					return( FALSE );
				}
			}
		}
		else if ( bLocation == LOC_4_0 )
		{
			// if door is normal and OPEN and outside type then we let N-S pass
			if ( (pStructure->fFlags & STRUCTURE_DOOR) && (pStructure->fFlags & STRUCTURE_OPEN) )
			{
				if (pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT ||
					pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)
				{
					return( FALSE );
				}
			}
			else if ( fTopRight )
			{
				if (!WallOrClosedDoorExistsOfTopLeftOrientation((INT16) (iGridNo + DirectionInc(NORTHEAST))) &&
					!WallOrClosedDoorExistsOfTopRightOrientation((INT16) (iGridNo + DirectionInc( NORTH))) &&
					!OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists((INT16) (iGridNo + DirectionInc( NORTHEAST))))
				{
					return( FALSE );
				}
			}
		}
	}

	if ( fEastWest )
	{
		// Check E-W at north corner:   4,4   4,0   0,4
		if ( bLocation == LOC_4_4 )
		{
			if ( pStructure->ubWallOrientation == NO_ORIENTATION)
			{
				// very top north corner of building, and going (screenwise) west or east
				return( FALSE );
			}
		}
		else if ( bLocation == LOC_4_0 )
		{
			// maybe looking E-W at (screenwise) north corner of building
			// if wall orientation is top-right, then check N of this location
			// if no wall of same orientation there, let bullet through
			if ( fTopRight )
			{
				if (!WallOrClosedDoorExistsOfTopRightOrientation((INT16) (iGridNo + DirectionInc(NORTH))) &&
					!WallOrClosedDoorExistsOfTopLeftOrientation((INT16) (iGridNo + DirectionInc( NORTH))))
				{
					return( FALSE );
				}
			}
		}
		else if ( bLocation == LOC_0_4 )
		{
			// if normal door and OPEN and inside type then we let E-W pass
			if ( (pStructure->fFlags & STRUCTURE_DOOR) && (pStructure->fFlags & STRUCTURE_OPEN) )
			{
				if (pStructure->ubWallOrientation == INSIDE_TOP_LEFT ||
					pStructure->ubWallOrientation == INSIDE_TOP_RIGHT)
				{
					return( FALSE );
				}
			}

			// if wall orientation is top-left, then check W of this location
			// if no wall of same orientation there, let bullet through
			if ( fTopLeft )
			{
				if (!WallOrClosedDoorExistsOfTopLeftOrientation((INT16) (iGridNo + DirectionInc(WEST))) &&
					!WallOrClosedDoorExistsOfTopRightOrientation((INT16) (iGridNo + DirectionInc( WEST))))
				{
					return( FALSE );
				}
			}

		}

		// Check E-W at south corner:   4,4 3,4 4,3
		if ( bLocation == LOC_3_4 || bLocation == LOC_4_4 || bLocation == LOC_4_3 )
		{
			if ((bLocation == LOC_3_4 && fTopLeft) || (bLocation == LOC_4_3 && fTopRight) ||
				(bLocation == LOC_4_4))
			{
				if (!WallOrClosedDoorExistsOfTopLeftOrientation((INT16) (iGridNo + DirectionInc(EAST))) &&
					!WallOrClosedDoorExistsOfTopRightOrientation((INT16) (iGridNo + DirectionInc( SOUTH))) &&
					!OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists((INT16) (iGridNo + DirectionInc( EAST))) &&
					!OpenRightOrientedDoorWithDoorOnRightOfEdgeExists((INT16) (iGridNo + DirectionInc(SOUTH))))
				{
					return( FALSE );
				}
			}
		}

	}





	// currently handled:
	// E-W at north corner:  (4,4), (0,4), (4,0)
	// N-S at east corner: (4,4)
	// N-S at west corner: (4,4)

	// could add:
	// N-S at east corner: (3, 4), (4, 0)
	// N-S at west corner: (0, 4), (4, 3)
	// E-W at south corner: (4, 4), (3, 4), (4, 3) (completely new)

	/*
	// possibly shooting at corner in which case we should let it pass
	if ( bLOSIndexX == 0)
	{
		if ( bLOSIndexY == (PROFILE_Y_SIZE - 1))
		{
			// maybe looking E-W at (screenwise) north corner of building, or through open door
			if ( ( ddHorizAngle > (0) && ddHorizAngle < (PI * 1 / 2) ) || ( ddHorizAngle < (-PI * 1 / 2) && ddHorizAngle > ( -PI ) ) )
			{
				// if door is normal and OPEN and inside type then we let E-W pass
				if ( (pStructure->fFlags & STRUCTURE_DOOR) && (pStructure->fFlags & STRUCTURE_OPEN) )
				{
					if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == INSIDE_TOP_RIGHT )
					{
						fResolveHit = FALSE;
					}
				}

				// if wall orientation is top-left, then check W of this location
				// if no wall of same orientation there, let bullet through
				if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT )
				{
					if (!WallOrClosedDoorExistsOfTopLeftOrientation( (INT16) (iGridNo + DirectionInc( WEST )) ) &&
							!WallOrClosedDoorExistsOfTopRightOrientation( (INT16) (iGridNo + DirectionInc( WEST )) ) )
					{
						fResolveHit = FALSE;
					}
				}
			}
			else if ( ( ddHorizAngle < (0) && ddHorizAngle > ( -PI * 1 / 2) ) || ( ddHorizAngle > ( PI * 1 / 2 ) && ddHorizAngle < (PI) ) )
			{
				// maybe looking N-S at (screenwise) west corner of building

				// if wall orientation is top-left, then check W of this location
				// if no wall of same orientation there, let bullet through
				if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT )
				{
					if ( !WallOrClosedDoorExistsOfTopLeftOrientation( (INT16) (iGridNo + DirectionInc( WEST )) ) &&
						!WallOrClosedDoorExistsOfTopRightOrientation( (INT16) (iGridNo + DirectionInc( SOUTHWEST ) ) ) &&
						!OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists( (INT16) (iGridNo + DirectionInc( WEST )) ) )
					{
						fResolveHit = FALSE;
					}
				}

			}

		}
	}
	else if (bLOSIndexX == (PROFILE_X_SIZE - 1))
	{
		if (bLOSIndexY == 0)
		{
			// maybe looking E-W at (screenwise) north corner of building
			if ( ( ddHorizAngle > (0) && ddHorizAngle < (PI * 1 / 2) ) || ( ddHorizAngle < (-PI * 1 / 2) && ddHorizAngle > ( -PI ) ) )
			{
				// if wall orientation is top-right, then check N of this location
				// if no wall of same orientation there, let bullet through
				if ( pStructure->ubWallOrientation == INSIDE_TOP_RIGHT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT )
				{
					if (!WallOrClosedDoorExistsOfTopRightOrientation( (INT16) (iGridNo + DirectionInc( NORTH )) ) &&
							!WallOrClosedDoorExistsOfTopLeftOrientation( (INT16) (iGridNo + DirectionInc( NORTH )) ) )
					{
						fResolveHit = FALSE;
					}
				}
			}
			else
			{
				// if door is normal and OPEN and outside type then we let N-S pass
				if ( (pStructure->fFlags & STRUCTURE_DOOR) && (pStructure->fFlags & STRUCTURE_OPEN) )
				{
					if ( pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT )
					{
						fResolveHit = FALSE;
					}
				}
			}
		}
		else if ( bLOSIndexY == (PROFILE_Y_SIZE - 1) || bLOSIndexY == (PROFILE_Y_SIZE - 2) )
		{
			// maybe (SCREENWISE) west or east corner of building and looking N
			if ( ( ddHorizAngle < (0) && ddHorizAngle > ( -PI * 1 / 2) ) || ( ddHorizAngle > ( PI * 1 / 2 ) && ddHorizAngle < (PI) ) )
			{
				// if wall orientation is top-right, then check S of this location
				// if wall orientation is top-left, then check E of this location
				// if no wall of same orientation there, let bullet through
				if ( pStructure->ubWallOrientation == INSIDE_TOP_LEFT || pStructure->ubWallOrientation == OUTSIDE_TOP_LEFT )
				{
					if ( !WallOrClosedDoorExistsOfTopLeftOrientation( (INT16) (iGridNo + DirectionInc( EAST )) ) &&
						!WallOrClosedDoorExistsOfTopRightOrientation( (INT16) (iGridNo) ) &&
						!OpenLeftOrientedDoorWithDoorOnLeftOfEdgeExists( (INT16) (iGridNo + DirectionInc( EAST )) ) )
					{
						fResolveHit = FALSE;
					}
				}
				else if ( pStructure->ubWallOrientation == INSIDE_TOP_RIGHT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT )
				{
					if (!WallOrClosedDoorExistsOfTopRightOrientation( (INT16) (iGridNo + DirectionInc( SOUTH )) ) &&
							!WallOrClosedDoorExistsOfTopLeftOrientation( (INT16) (iGridNo) ) &&
							!OpenRightOrientedDoorWithDoorOnRightOfEdgeExists( (INT16) (iGridNo + DirectionInc( SOUTH )) ) )
					{
						fResolveHit = FALSE;
					}

				}
			}
			// the following only at 4,4
			else if ( bLOSIndexY == (PROFILE_Y_SIZE - 1) )
			{
				if ( pStructure->ubWallOrientation == NO_ORIENTATION)
				{
					// very top north corner of building, and going (screenwise) west or east
					fResolveHit = FALSE;
				}
			}
		}
	}*/

	return( TRUE );
}



// The line of sight code is now used to simulate smelling through the air (for monsters);
// It obeys the following rules:
// - ignores trees and vegetation
// - ignores people
// - should always start off with head height for both source and target, so that lying down makes no difference
// - stop at closed windows
// - stop for other obstacles
//
// Just for reference, normal sight obeys the following rules:
// - trees & vegetation reduce the maximum sighting distance
// - ignores people
// - starts at height relative to stance
// - ignores windows
// - stops at other obstacles
static INT32 LineOfSightTest(GridNo start_pos, FLOAT dStartZ, GridNo end_pos, FLOAT dEndZ, UINT8 ubTileSightLimit, UINT8 ubTreeSightReduction, INT8 bAware, INT8 bCamouflage, BOOLEAN fSmell, INT16* psWindowGridNo)
{
	// Parameters...
	// the X,Y,Z triplets should be obvious
	// TileSightLimit is the max # of tiles of distance visible
	// TreeSightReduction is the reduction in 10ths of tiles in max visibility for each LOS cube (5th of a tile) of
	// vegetation hit
	// Aware is whether the looker is aware of the target
	// Smell is whether this is a sight or a smell test

	// Now returns not a boolean but the adjusted (by cover) distance to the target, or 0 for unseen

	FIXEDPT qCurrX;
	FIXEDPT qCurrY;
	FIXEDPT qCurrZ;

	INT32 iGridNo;
	INT32 iCurrTileX;
	INT32 iCurrTileY;

	INT8  bLOSIndexX;
	INT8  bLOSIndexY;
	INT8  bOldLOSIndexX;
	INT8  bOldLOSIndexY;
	INT32 iOldCubesZ;

	INT32 iCurrCubesZ;

	FIXEDPT qLandHeight;
	INT32 iCurrAboveLevelZ;
	INT32 iCurrCubesAboveLevelZ;
	INT32 iStartCubesAboveLevelZ;
	INT32 iEndCubesAboveLevelZ;
	INT32 iStartCubesZ;
	INT32 iEndCubesZ;

	INT16 sDesiredLevel;


	INT32 iOldTileX;
	INT32 iOldTileY;

	FLOAT dDeltaX;
	FLOAT dDeltaY;
	FLOAT dDeltaZ;

	FIXEDPT qIncrX;
	FIXEDPT qIncrY;
	FIXEDPT qIncrZ;

	FLOAT dDistance;

	INT32 iDistance;
	INT32 iSightLimit = ubTileSightLimit * CELL_X_SIZE;
	INT32 iAdjSightLimit = iSightLimit;

	INT32 iLoop;

	MAP_ELEMENT *pMapElement;
	STRUCTURE *pStructure;
	STRUCTURE *pRoofStructure = NULL;

	BOOLEAN fCheckForRoof;
	FIXEDPT qLastZ;

	FIXEDPT qDistToTravelX;
	FIXEDPT qDistToTravelY;
	INT32 iStepsToTravelX;
	INT32 iStepsToTravelY;
	INT32 iStepsToTravel;
	BOOLEAN fResolveHit;
	DOUBLE ddHorizAngle;
	INT32 iStructureHeight;

	FIXEDPT qWallHeight;
	BOOLEAN fOpaque;
	INT8 bSmoke = 0;

	if ( gTacticalStatus.uiFlags & DISALLOW_SIGHT )
	{
		return( 0 );
	}

	if (iSightLimit == 0)
	{
		// blind!
		return( 0 );
	}

	if (!bAware && !fSmell)
	{
		// trees are x3 as good at reducing sight if looker is unaware
		// and increase that up to double for camouflage!
		ubTreeSightReduction = (ubTreeSightReduction * 3) * (100 + bCamouflage) / 100;
	}
	// verify start and end to make sure we'll always be inside the map

	// hack end location to the centre of the tile, because there was a problem
	// seeing a presumably off-centre merc...

	INT16 start_cell_x;
	INT16 start_cell_y;
	ConvertGridNoToCenterCellXY(start_pos, &start_cell_x, &start_cell_y);
	const float dStartX = start_cell_x;
	const float dStartY = start_cell_y;

	INT16 end_cell_x;
	INT16 end_cell_y;
	ConvertGridNoToCenterCellXY(end_pos, &end_cell_x, &end_cell_y);
	const float dEndX = end_cell_x;
	const float dEndY = end_cell_y;

	dDeltaX = dEndX - dStartX;
	dDeltaY = dEndY - dStartY;
	dDeltaZ = dEndZ - dStartZ;

	dDistance = Distance3D( dDeltaX, dDeltaY, CONVERT_HEIGHTUNITS_TO_DISTANCE( dDeltaZ ));
	iDistance = (INT32) dDistance;

	if ( iDistance == 0 )
	{
		return( FALSE );
	}

	if ( dDistance != (FLOAT) iDistance )
	{
		// add 1 step to account for fraction
		iDistance += 1;
	}

	if ( iDistance > iSightLimit)
	{
		// out of visual range
		return( 0 );
	}

	ddHorizAngle = atan2( dDeltaY, dDeltaX );

	#ifdef LOS_DEBUG
	gLOSTestResults = LOSResults{};
	gLOSTestResults.fLOSTestPerformed = TRUE;
	gLOSTestResults.iStartX = (INT32) dStartX;
	gLOSTestResults.iStartY = (INT32) dStartY;
	gLOSTestResults.iStartZ = (INT32) dStartZ;
	gLOSTestResults.iEndX = (INT32) dEndX;
	gLOSTestResults.iEndY = (INT32) dEndY;
	gLOSTestResults.iEndZ = (INT32) dEndZ;
	gLOSTestResults.iMaxDistance = (INT32) iSightLimit;
	gLOSTestResults.iDistance = (INT32) dDistance;
	#endif

	qIncrX = FloatToFixed( dDeltaX / (FLOAT)iDistance );
	qIncrY = FloatToFixed( dDeltaY / (FLOAT)iDistance );
	qIncrZ = FloatToFixed( dDeltaZ / (FLOAT)iDistance );

	fCheckForRoof = FALSE;

	// figure out starting and ending cubes
	iGridNo = GETWORLDINDEXFROMWORLDCOORDS( (INT32)dStartX, (INT32)dStartY );
	qCurrZ = FloatToFixed( dStartZ );
	qLandHeight = INT32_TO_FIXEDPT( CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ iGridNo ].sHeight ) );
	iCurrAboveLevelZ = FIXEDPT_TO_INT32( qCurrZ - qLandHeight );

	iStartCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrAboveLevelZ );
	iStartCubesAboveLevelZ = iStartCubesZ;
	if (iStartCubesAboveLevelZ >= STRUCTURE_ON_GROUND_MAX)
	{
		iStartCubesAboveLevelZ -= STRUCTURE_ON_ROOF;
	}

	// check to see if we need to check for roofs based on the starting gridno
	qWallHeight = gqStandardWallHeight + qLandHeight;
	if ( qCurrZ < qWallHeight )
	{
		// possibly going up through a roof on this level
		qCurrZ = FloatToFixed( dEndZ );

		if ( qCurrZ > qWallHeight )
		{
			fCheckForRoof = TRUE;
		}

	}
	else // >
	{
		// possibly going down through a roof on this level
		qCurrZ = FloatToFixed( dEndZ );

		if (qCurrZ < qWallHeight)
		{
			fCheckForRoof = TRUE;
		}
	}

	iGridNo = GETWORLDINDEXFROMWORLDCOORDS( (INT32)dEndX, (INT32)dEndY );
	qCurrZ = FloatToFixed( dEndZ );
	qLandHeight = INT32_TO_FIXEDPT( CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ iGridNo ].sHeight ) );
	iCurrAboveLevelZ = FIXEDPT_TO_INT32( qCurrZ - qLandHeight );
	iEndCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrAboveLevelZ );
	iEndCubesAboveLevelZ = iEndCubesZ;
	if (iEndCubesAboveLevelZ >= STRUCTURE_ON_GROUND_MAX)
	{
		iEndCubesAboveLevelZ -= STRUCTURE_ON_ROOF;
	}

	// check to see if we need to check for roofs based on the starting gridno
	qWallHeight = gqStandardWallHeight + qLandHeight;

	if ( qCurrZ < qWallHeight )
	{
		// possibly going down through a roof on this level
		qCurrZ = FloatToFixed( dStartZ );

		if ( qCurrZ > qWallHeight )
		{
			fCheckForRoof = TRUE;
		}

	}
	else // >
	{
		// possibly going up through a roof on this level
		qCurrZ = FloatToFixed( dStartZ );

		if (qCurrZ < qWallHeight)
		{
			fCheckForRoof = TRUE;
		}
	}

	// apply increments for first move

	// first move will be 1 step
	// plus a fractional part equal to half of the difference between the delta and
	// the increment times the distance

	qCurrX = FloatToFixed( dStartX ) + qIncrX + ( FloatToFixed( dDeltaX ) - qIncrX * iDistance ) / 2;
	qCurrY = FloatToFixed( dStartY ) + qIncrY + ( FloatToFixed( dDeltaY ) - qIncrY * iDistance ) / 2;
	qCurrZ = FloatToFixed( dStartZ ) + qIncrZ + ( FloatToFixed( dDeltaZ ) - qIncrZ * iDistance ) / 2;

	iCurrTileX = FIXEDPT_TO_TILE_NUM( qCurrX );
	iCurrTileY = FIXEDPT_TO_TILE_NUM( qCurrY );
	bLOSIndexX = FIXEDPT_TO_LOS_INDEX( qCurrX );
	bLOSIndexY = FIXEDPT_TO_LOS_INDEX( qCurrY );
	iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( qCurrZ ) );

	iLoop = 1;

	do
	{
		// check a particular tile

		// retrieve values from world for this particular tile
		iGridNo = iCurrTileX + iCurrTileY * WORLD_COLS;
		pMapElement = &(gpWorldLevelData[ iGridNo ]);
		qLandHeight = INT32_TO_FIXEDPT( CONVERT_PIXELS_TO_HEIGHTUNITS( pMapElement->sHeight ) );
		qWallHeight = gqStandardWallHeight + qLandHeight;

		if (fCheckForRoof)
		{
			pRoofStructure = FindStructure( (INT16) iGridNo, STRUCTURE_ROOF );

			if ( pRoofStructure )
			{

				qLastZ = qCurrZ - qIncrZ;

				// if just on going to next tile we cross boundary, then roof stops sight here!
				if ( (qLastZ > qWallHeight && qCurrZ <= qWallHeight) || (qLastZ < qWallHeight && qCurrZ >= qWallHeight))
				{
					// hit a roof
					return( 0 );
				}

			}

		}

		// record old tile location for loop purposes
		iOldTileX = iCurrTileX;
		iOldTileY = iCurrTileY;
		do
		{
			// check a particular location within the tile

			// check for collision with the ground
			iCurrAboveLevelZ = FIXEDPT_TO_INT32( qCurrZ - qLandHeight );
			if (iCurrAboveLevelZ < 0)
			{
				// ground is in the way!
				#ifdef LOS_DEBUG
					gLOSTestResults.iStoppedX = FIXEDPT_TO_INT32( qCurrX );
					gLOSTestResults.iStoppedY = FIXEDPT_TO_INT32( qCurrY );
					gLOSTestResults.iStoppedZ = FIXEDPT_TO_INT32( qCurrZ );
					// subtract one to compensate for rounding up when negative
					gLOSTestResults.iCurrCubesZ = 0;
				#endif
				return( 0 );
			}
			// check for the existence of structures
			pStructure = pMapElement->pStructureHead;
			if (pStructure == NULL)
			{	// no structures in this tile, AND THAT INCLUDES ROOFS! :-)

				// new system; figure out how many steps until we cross the next edge
				// and then fast forward that many steps.

				iOldTileX = iCurrTileX;
				iOldTileY = iCurrTileY;
				iOldCubesZ = iCurrCubesZ;

				if (qIncrX > 0)
				{
					qDistToTravelX = INT32_TO_FIXEDPT( CELL_X_SIZE ) - (qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE ));
					iStepsToTravelX = qDistToTravelX / qIncrX;
				}
				else if (qIncrX < 0)
				{
					qDistToTravelX = qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE );
					iStepsToTravelX = qDistToTravelX / -qIncrX;
				}
				else
				{
					// make sure we don't consider X a limit :-)
					iStepsToTravelX = 1000000;
				}

				if (qIncrY > 0)
				{
					qDistToTravelY = INT32_TO_FIXEDPT( CELL_Y_SIZE ) - (qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE ));
					iStepsToTravelY = qDistToTravelY / qIncrY;
				}
				else if (qIncrY < 0)
				{
					qDistToTravelY = qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE );
					iStepsToTravelY = qDistToTravelY / -qIncrY;
				}
				else
				{
					// make sure we don't consider Y a limit :-)
					iStepsToTravelY = 1000000;
				}

				iStepsToTravel = __min( iStepsToTravelX, iStepsToTravelY ) + 1;

				/*
				if (qIncrX > 0)
				{
					qDistToTravelX = INT32_TO_FIXEDPT( CELL_X_SIZE ) - (qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE ));
					iStepsToTravelX = qDistToTravelX / qIncrX;
				}
				else if (qIncrX < 0)
				{
					qDistToTravelX = qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE );
					iStepsToTravelX = qDistToTravelX / (-qIncrX);
				}
				else
				{
					// make sure we don't consider X a limit :-)
					iStepsToTravelX = 1000000;
				}

				if (qIncrY > 0)
				{
					qDistToTravelY = INT32_TO_FIXEDPT( CELL_Y_SIZE ) - (qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE ));
					iStepsToTravelY = qDistToTravelY / qIncrY;
				}
				else if (qIncrY < 0)
				{
					qDistToTravelY = qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE );
					iStepsToTravelY = qDistToTravelY / (-qIncrY);
				}
				else
				{
					// make sure we don't consider Y a limit :-)
					iStepsToTravelY = 1000000;
				}

				// add 1 to the # of steps to travel to go INTO the next tile
				iStepsToTravel = __min( iStepsToTravelX, iStepsToTravelY ) + 1;
				//iStepsToTravel = 1;*/

				qCurrX += qIncrX * iStepsToTravel;
				qCurrY += qIncrY * iStepsToTravel;
				qCurrZ += qIncrZ * iStepsToTravel;
				iLoop += iStepsToTravel;

				// check for ground collision
				if (qCurrZ < qLandHeight && iLoop < iDistance)
				{
					// ground is in the way!
					#ifdef LOS_DEBUG
						gLOSTestResults.iStoppedX = FIXEDPT_TO_INT32( qCurrX );
						gLOSTestResults.iStoppedY = FIXEDPT_TO_INT32( qCurrY );
						gLOSTestResults.iStoppedZ = FIXEDPT_TO_INT32( qCurrZ );
						// subtract one to compensate for rounding up when negative
						gLOSTestResults.iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrCubesAboveLevelZ ) - 1;
					#endif
					return( 0 );
				}

				// figure out the new tile location
				iCurrTileX = FIXEDPT_TO_TILE_NUM( qCurrX );
				iCurrTileY = FIXEDPT_TO_TILE_NUM( qCurrY );
				iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( qCurrZ ) );
				bLOSIndexX = FIXEDPT_TO_LOS_INDEX( qCurrX );
				bLOSIndexY = FIXEDPT_TO_LOS_INDEX( qCurrY );
			}
			else
			{	// there are structures in this tile

				iCurrCubesAboveLevelZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrAboveLevelZ );
				// figure out the LOS cube level of the current point

				if (iCurrCubesAboveLevelZ < STRUCTURE_ON_ROOF_MAX)
				{
					if (iCurrCubesAboveLevelZ < STRUCTURE_ON_GROUND_MAX)
					{
						// check objects on the ground
						sDesiredLevel = STRUCTURE_ON_GROUND;
					}
					else
					{
						// check objects on roofs
						sDesiredLevel = STRUCTURE_ON_ROOF;
						iCurrCubesAboveLevelZ -= STRUCTURE_ON_ROOF;
					}
					// check structures for collision
					while (pStructure != NULL)
					{
						// transparent structures should be skipped
						// normal roof structures should be skipped
						// here because their only bits are roof lips
						// and those should act as transparent
						fOpaque = (pStructure->fFlags & STRUCTURE_TRANSPARENT) == 0;
						if ( pStructure->fFlags & STRUCTURE_ROOF )
						{
							// roof lip; allow sighting if person on roof is near
							if ( (iLoop < 2 * CELL_X_SIZE || (iDistance - iLoop) < 2 * CELL_X_SIZE ) )
							{
								if ( iLoop <= CELL_X_SIZE + 1 || (iDistance - iLoop ) <= CELL_X_SIZE + 1 )
								{
									// right near edge, allow sighting at 3 tiles from roof edge if prone
									// less if standing, and we can tell that with iStartCubesZ and iEndCubesZ
									if ( iStartCubesZ < iEndCubesZ )
									{
										// looking up, so reduce for the target stance-height according to iEndCubesZ
										if ( iDistance >= (3 - iEndCubesAboveLevelZ) * CELL_X_SIZE )
										{
											fOpaque = FALSE;
										}
									}
									else
									{
										if ( iDistance >= (3 - iStartCubesAboveLevelZ) * CELL_X_SIZE )
										{
											fOpaque = FALSE;
										}
									}

								}
								else
								{
									if ( iDistance >= 12 * CELL_X_SIZE )
									{
										fOpaque = FALSE;
									}
								}
							}
						}

						if ( fOpaque )
						{
							if (pStructure->sCubeOffset == sDesiredLevel)
							{
								if (((*(pStructure->pShape))[bLOSIndexX][bLOSIndexY] & AtHeight[iCurrCubesAboveLevelZ]) > 0)
								{
									if (fSmell)
									{
										if (pStructure->fFlags & STRUCTURE_TREE)
										{
											// smell not stopped by vegetation
										}
										else if ((pStructure->fFlags & STRUCTURE_WALLNWINDOW) && (pStructure->fFlags & STRUCTURE_OPEN))
										{
											// open window, smell not stopped
										}
										else
										{
											if (pStructure->fFlags & STRUCTURE_WALLSTUFF)
											{
												// possibly at corner in which case we should let it pass
												fResolveHit = ResolveHitOnWall( pStructure, iGridNo, bLOSIndexX, bLOSIndexY, ddHorizAngle );
											}
											else
											{
												fResolveHit = TRUE;
											}
											if (fResolveHit)
											{
												// CJC, May 30:  smell reduced by obstacles but not
												// stopped if obstacle within 10 tiles
												iAdjSightLimit -= SMELL_REDUCTION_FOR_NEARBY_OBSTACLE;
												if (iLoop > 100 || iDistance > iAdjSightLimit)
												{
													// out of visual range
													#ifdef LOS_DEBUG
													gLOSTestResults.fOutOfRange = TRUE;
													gLOSTestResults.iCurrCubesZ = iCurrCubesZ;
													#endif
													return( 0 );
												}

												/*
												// smell-line stopped by obstacle!
												#ifdef LOS_DEBUG
													gLOSTestResults.iStoppedX = FIXEDPT_TO_INT32( qCurrX );
													gLOSTestResults.iStoppedY = FIXEDPT_TO_INT32( qCurrY );
													gLOSTestResults.iStoppedZ = FIXEDPT_TO_INT32( qCurrZ );
													gLOSTestResults.iCurrCubesZ = iCurrCubesAboveLevelZ;
												#endif
												return( 0 );*/
											}
										}
									}
									else
									{
										if (pStructure->fFlags & STRUCTURE_TREE)
										{
											// don't count trees close to the person
											if (iLoop > CLOSE_TO_FIRER)
											{
												if (iLoop > 100)
												{
													// at longer range increase the value of tree cover
													iAdjSightLimit -= (ubTreeSightReduction * iLoop) / 100;
												}
												else
												{
													// use standard value
													iAdjSightLimit -= ubTreeSightReduction;
												}
												#ifdef LOS_DEBUG
												gLOSTestResults.ubTreeSpotsHit++;
												gLOSTestResults.iMaxDistance = iSightLimit;
												#endif
												if (iDistance > iAdjSightLimit)
												{
													// out of visual range
													#ifdef LOS_DEBUG
													gLOSTestResults.fOutOfRange = TRUE;
													gLOSTestResults.iCurrCubesZ = iCurrCubesZ;
													#endif
													return( 0 );
												}
											}
										}
										else if ( (pStructure->fFlags & STRUCTURE_WALLNWINDOW) && !(pStructure->fFlags & STRUCTURE_SPECIAL) && qCurrZ >= (gqStandardWindowBottomHeight + qLandHeight) && qCurrZ <= (gqStandardWindowTopHeight + qLandHeight))
										{
											// do nothing; windows are transparent (except ones marked as special)
											if (psWindowGridNo != NULL)
											{
												// we're supposed to note the location of this window!
												// but if a location has already been set then there are two windows, in which case
												// we abort
												if (*psWindowGridNo == NOWHERE)
												{
													*psWindowGridNo = (INT16) iGridNo;
													return( iLoop );
												}
												else
												{
													//*psWindowGridNo = NOWHERE;
													//return( iLoop );
												}
											}
										}
										else
										{
											if (pStructure->fFlags & STRUCTURE_WALLSTUFF)
											{
												// possibly shooting at corner in which case we should let it pass
												fResolveHit = ResolveHitOnWall( pStructure, iGridNo, bLOSIndexX, bLOSIndexY, ddHorizAngle );
											}
											else
											{
												if (iCurrCubesAboveLevelZ < (STANDING_CUBES - 1) )
												{
													if ( (iLoop <= CLOSE_TO_FIRER) && (iCurrCubesAboveLevelZ <= iStartCubesAboveLevelZ) )
													{
														// if we are in the same vertical cube as the start,
														// and this is the height of the structure, then allow sight to go through
														// NB cubes are 0 based, heights 1 based
														iStructureHeight = StructureHeight( pStructure );
														fResolveHit = ( iCurrCubesAboveLevelZ != (iStructureHeight - 1) );
													}
													else if ( (iLoop >= (iDistance - CLOSE_TO_FIRER) ) && (iCurrCubesAboveLevelZ <= iEndCubesZ) && bAware )
													{
														// if we are in the same vertical cube as our destination,
														// and this is the height of the structure, and we are aware
														// then allow sight to go through
														// NB cubes are 0 based, heights 1 based
														iStructureHeight = StructureHeight( pStructure );
														fResolveHit = ( iCurrCubesAboveLevelZ != (iStructureHeight - 1) );
													}
													else
													{
														fResolveHit = TRUE;
													}
												}
												else
												{
													fResolveHit = TRUE;
												}
											}
											if (fResolveHit)
											{
												// hit the obstacle!
												#ifdef LOS_DEBUG
													gLOSTestResults.iStoppedX = FIXEDPT_TO_INT32( qCurrX );
													gLOSTestResults.iStoppedY = FIXEDPT_TO_INT32( qCurrY );
													gLOSTestResults.iStoppedZ = FIXEDPT_TO_INT32( qCurrZ );
													gLOSTestResults.iCurrCubesZ = iCurrCubesAboveLevelZ;
												#endif
												return( 0 );
											}
										}
									}
								}
							}
						}
						pStructure = pStructure->pNext;
					}
				}
				// got past all structures; go to next location within
				// tile, horizontally or vertically
				bOldLOSIndexX = bLOSIndexX;
				bOldLOSIndexY = bLOSIndexY;
				iOldCubesZ = iCurrCubesZ;
				do
				{
					qCurrX += qIncrX;
					qCurrY += qIncrY;
					if (pRoofStructure)
					{
						qLastZ = qCurrZ;
						qCurrZ += qIncrZ;
						if ( (qLastZ > qWallHeight && qCurrZ <= qWallHeight) || (qLastZ < qWallHeight && qCurrZ >= qWallHeight))
						{
							// hit a roof
							return( 0 );
						}
					}
					else
					{
						qCurrZ += qIncrZ;
					}

					iLoop++;
					bLOSIndexX = FIXEDPT_TO_LOS_INDEX( qCurrX );
					bLOSIndexY = FIXEDPT_TO_LOS_INDEX( qCurrY );
					iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( qCurrZ ) );
					// shouldn't need to check whether we are not at maximum range because
					// that will be caught below and this loop shouldn't go for more than a
					// couple of iterations.
				}
				while( (bLOSIndexX == bOldLOSIndexX) && (bLOSIndexY == bOldLOSIndexY) && (iCurrCubesZ == iOldCubesZ));
				iCurrTileX = FIXEDPT_TO_TILE_NUM( qCurrX );
				iCurrTileY = FIXEDPT_TO_TILE_NUM( qCurrY );
			}
		} while( (iCurrTileX == iOldTileX) && (iCurrTileY == iOldTileY) && (iLoop < iDistance));

		// leaving a tile, check to see if it had gas in it
		if ( pMapElement->ubExtFlags[0] & (MAPELEMENT_EXT_SMOKE | MAPELEMENT_EXT_TEARGAS | MAPELEMENT_EXT_MUSTARDGAS) )
		{
			if ( (pMapElement->ubExtFlags[0] & MAPELEMENT_EXT_SMOKE) && !fSmell )
			{
				bSmoke++;

				// we can only see 3 tiles in smoke
				// (2 if we're IN smoke)

				if ( bSmoke >= 3 )
				{
					iAdjSightLimit = 0;
				}
				// unpopular
				/*
				else
				{
					// losing 1/3rd results in chances to hit which are WAY too low when firing from out of
					// two tiles of smoke... changing this to a 1/6 penalty

					iAdjSightLimit -= iSightLimit / 6;
				}*/
			}
			else
			{
				// reduce by 2 tiles per tile of tear gas or mustard gas
				iAdjSightLimit -= 2 * CELL_X_SIZE;
			}

			if ( iAdjSightLimit <= 0 )
			{
				// can't see, period!
				return( 0 );
			}
		}

	} while( iLoop < iDistance );
	// unless the distance is integral, after the loop there will be a
	// fractional amount of distance remaining which is unchecked
	// but we shouldn't(?) need to check it because the target is there!
	#ifdef LOS_DEBUG
	gLOSTestResults.fLOSClear = TRUE;
	#endif
	// this somewhat complicated formula does the following:
	// it starts with the distance to the target
	// it adds the difference between the original and adjusted sight limit, = the amount of cover
	// it then scales the value based on the difference between the original sight limit and the
	//   very maximum possible in best lighting conditions
	return( (iDistance + (iSightLimit - iAdjSightLimit)) * (MaxDistanceVisible() * CELL_X_SIZE) / iSightLimit );
}


BOOLEAN CalculateSoldierZPos(const SOLDIERTYPE* pSoldier, UINT8 ubPosType, FLOAT* pdZPos)
{
	UINT8 ubHeight;

	if ( pSoldier->ubBodyType == CROW )
	{
		// Crow always as prone...
		ubHeight = ANIM_PRONE;
	}
	else if (pSoldier->bOverTerrainType == DEEP_WATER)
	{
		// treat as prone
		ubHeight = ANIM_PRONE;
	}
	else if ( pSoldier->bOverTerrainType == LOW_WATER || pSoldier->bOverTerrainType == MED_WATER )
	{
		// treat as crouched
		ubHeight = ANIM_CROUCH;
	}
	else
	{
		if ( CREATURE_OR_BLOODCAT( pSoldier ) || pSoldier->ubBodyType == COW )
		{
			// this if statement is to avoid the 'creature weak spot' target
			// spot for creatures
			if ( ubPosType == HEAD_TARGET_POS || ubPosType == LEGS_TARGET_POS )
			{
				// override!
				ubPosType = TORSO_TARGET_POS;
			}
		}
		else if ( TANK( pSoldier ) )
		{
			// high up!
			ubPosType = HEAD_TARGET_POS;
		}

		ubHeight = gAnimControl[ pSoldier->usAnimState ].ubEndHeight;
	}

	switch( ubPosType )
	{
		case LOS_POS:
			switch ( ubHeight )
			{
				case ANIM_STAND:
					*pdZPos = STANDING_LOS_POS;
					break;
				case ANIM_CROUCH:
					*pdZPos = CROUCHED_LOS_POS;
					break;
				case ANIM_PRONE:
					*pdZPos = PRONE_LOS_POS;
					break;
				default:
					return( FALSE );
			}
			break;
		case FIRING_POS:
			switch ( ubHeight )
			{
				case ANIM_STAND:
					*pdZPos = STANDING_FIRING_POS;
					break;
				case ANIM_CROUCH:
					*pdZPos = CROUCHED_FIRING_POS;
					break;
				case ANIM_PRONE:
					*pdZPos = PRONE_FIRING_POS;
					break;
				default:
					return( FALSE );
			}
			break;
		case TARGET_POS:
			switch ( ubHeight )
			{
				case ANIM_STAND:
					*pdZPos = STANDING_TARGET_POS;
					break;
				case ANIM_CROUCH:
					*pdZPos = CROUCHED_TARGET_POS;
					break;
				case ANIM_PRONE:
					*pdZPos = PRONE_TARGET_POS;
					break;
				default:
					return( FALSE );
			}
			break;
		case HEAD_TARGET_POS:
			switch ( ubHeight )
			{
				case ANIM_STAND:
					*pdZPos = STANDING_HEAD_TARGET_POS;
					break;
				case ANIM_CROUCH:
					*pdZPos = CROUCHED_HEAD_TARGET_POS;
					break;
				case ANIM_PRONE:
					*pdZPos = PRONE_HEAD_TARGET_POS;
					break;
				default:
					return( FALSE );
			}
			break;
		case TORSO_TARGET_POS:
			switch ( ubHeight )
			{
				case ANIM_STAND:
					*pdZPos = STANDING_TORSO_TARGET_POS;
					break;
				case ANIM_CROUCH:
					*pdZPos = CROUCHED_TORSO_TARGET_POS;
					break;
				case ANIM_PRONE:
					*pdZPos = PRONE_TORSO_TARGET_POS;
					break;
				default:
					return( FALSE );
			}
			break;
		case LEGS_TARGET_POS:
			switch ( ubHeight )
			{
				case ANIM_STAND:
					*pdZPos = STANDING_LEGS_TARGET_POS;
					break;
				case ANIM_CROUCH:
					*pdZPos = CROUCHED_LEGS_TARGET_POS;
					break;
				case ANIM_PRONE:
					*pdZPos = PRONE_LEGS_TARGET_POS;
					break;
				default:
					return( FALSE );
			}
			break;
		case HEIGHT:
			switch ( ubHeight )
			{
				case ANIM_STAND:
					*pdZPos = STANDING_HEIGHT;
					break;
				case ANIM_CROUCH:
					*pdZPos = CROUCHED_HEIGHT;
					break;
				case ANIM_PRONE:
					*pdZPos = PRONE_HEIGHT;
					break;
				default:
					return( FALSE );
			}
			break;
	}
	if ( pSoldier->ubBodyType == HATKIDCIV || pSoldier->ubBodyType == KIDCIV )
	{
		// reduce value for kids who are 2/3 the height of regular people
		*pdZPos = (*pdZPos * 2) / 3;
	}
	else if ( pSoldier->ubBodyType == ROBOTNOWEAPON || pSoldier->ubBodyType == LARVAE_MONSTER || pSoldier->ubBodyType == INFANT_MONSTER || pSoldier->ubBodyType == BLOODCAT )
	{
		// robot is 1/3 the height of regular people
		*pdZPos = *pdZPos / 3;
	}
	else if ( TANK ( pSoldier ) )
	{
		*pdZPos = (*pdZPos * 4) / 3;
	}

	if (pSoldier->bLevel > 0)
	{
		// on a roof
		*pdZPos += WALL_HEIGHT_UNITS;
	}

	// IF this is a plane, strafe!
	// ATE: Don;t panic - this is temp - to be changed to a status flag....
	if ( pSoldier->ubID == MAX_NUM_SOLDIERS )
	{
		*pdZPos = ( WALL_HEIGHT_UNITS * 2 ) - 1;
	}

	*pdZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[pSoldier->sGridNo].sHeight );
	return( TRUE );
}


INT32 SoldierToSoldierLineOfSightTest(const SOLDIERTYPE* const pStartSoldier, const SOLDIERTYPE* const pEndSoldier, UINT8 ubTileSightLimit, const INT8 bAware)
{
	FLOAT dStartZPos, dEndZPos;
	BOOLEAN fOk;
	BOOLEAN fSmell;
	INT8 bEffectiveCamo;
	UINT8 ubTreeReduction;

	// TO ADD: if target is camouflaged and in cover, reduce sight distance by 30%
	// TO ADD: if in tear gas, reduce sight limit to 2 tiles
	CHECKF( pStartSoldier );
	CHECKF( pEndSoldier );
	fOk = CalculateSoldierZPos( pStartSoldier, LOS_POS, &dStartZPos );
	CHECKF( fOk );

	if ( gWorldSectorX == 5 && gWorldSectorY == MAP_ROW_N )
	{
		// in the bloodcat arena sector, skip sight between army & bloodcats
		if ( pStartSoldier->bTeam == ENEMY_TEAM && pEndSoldier->bTeam == CREATURE_TEAM )
		{
			return( 0 );
		}
		if ( pStartSoldier->bTeam == CREATURE_TEAM && pEndSoldier->bTeam == ENEMY_TEAM )
		{
			return( 0 );
		}
	}


	if (pStartSoldier->uiStatusFlags & SOLDIER_MONSTER)
	{
		// monsters use smell instead of sight!
		dEndZPos = STANDING_LOS_POS; // should avoid low rocks etc
		if (pEndSoldier->bLevel > 0)
		{
			// on a roof
			dEndZPos += WALL_HEIGHT_UNITS;
		}
		dEndZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[pEndSoldier->sGridNo].sHeight );
		fSmell = TRUE;
	}
	else
	{
		fOk = CalculateSoldierZPos( pEndSoldier, LOS_POS, &dEndZPos );
		CHECKF( fOk );
		fSmell = FALSE;
	}

	if ( TANK( pStartSoldier ) )
	{
		INT16 sDistance;

		sDistance = PythSpacesAway( pStartSoldier->sGridNo, pEndSoldier->sGridNo );

		if ( sDistance <= 8 )
		{
			// blind spot?
			if ( dEndZPos <= PRONE_LOS_POS )
			{
				return( FALSE );
			}
			else if ( sDistance <= 4 && dEndZPos <= CROUCHED_LOS_POS )
			{
				return( FALSE );
			}
		}
	}

	if ( pEndSoldier->bCamo && !bAware )
	{

		INT32 iTemp;

		// reduce effects of camo of 5% per tile moved last turn
		if ( pEndSoldier->ubBodyType == BLOODCAT )
		{
			bEffectiveCamo = 100 - pEndSoldier->bTilesMoved * 5;
		}
		else
		{
			bEffectiveCamo = pEndSoldier->bCamo * (100 - pEndSoldier->bTilesMoved * 5) / 100;
		}
		bEffectiveCamo = __max( bEffectiveCamo, 0 );

		if ( gAnimControl[ pEndSoldier->usAnimState ].ubEndHeight < ANIM_STAND )
		{
			// reduce visibility by up to a third for camouflage!
			switch( pEndSoldier->bOverTerrainType )
			{
				case FLAT_GROUND:
				case LOW_GRASS:
				case HIGH_GRASS:
					iTemp = ubTileSightLimit;
					iTemp -= iTemp * (bEffectiveCamo / 3) / 100;
					ubTileSightLimit = (UINT8) iTemp;
					break;
				default:
					break;
			}
		}
	}
	else
	{
		bEffectiveCamo = 0;
	}

	if ( TANK( pEndSoldier ) )
	{
		ubTreeReduction = 0;
	}
	else
	{
		ubTreeReduction = gubTreeSightReduction[ gAnimControl[pEndSoldier->usAnimState].ubEndHeight ];
	}

	return LineOfSightTest(pStartSoldier->sGridNo, dStartZPos, pEndSoldier->sGridNo, dEndZPos, ubTileSightLimit, ubTreeReduction, bAware, bEffectiveCamo, fSmell, NULL);
}

INT16 SoldierToLocationWindowTest(const SOLDIERTYPE* pStartSoldier, INT16 sEndGridNo)
{
	// figure out if there is a SINGLE window between the looker and target
	FLOAT dStartZPos, dEndZPos;

	CHECKF( pStartSoldier );
	dStartZPos = FixedToFloat( ((gqStandardWindowTopHeight + gqStandardWindowBottomHeight) / 2) );
	if (pStartSoldier->bLevel > 0)
	{
		// on a roof
		dStartZPos += WALL_HEIGHT_UNITS;
	}
	dStartZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[pStartSoldier->sGridNo].sHeight );
	dEndZPos = dStartZPos;

	// We don't want to consider distance limits here so pass in tile sight limit of 255
	// and consider trees as little as possible
	INT16 sWindowGridNo = NOWHERE;
	LineOfSightTest(pStartSoldier->sGridNo, dStartZPos, sEndGridNo, dEndZPos, 255, 0, TRUE, 0, FALSE, &sWindowGridNo);
	if (sWindowGridNo == pStartSoldier->sGridNo) sWindowGridNo = NOWHERE; // XXX TODO0012
	return( sWindowGridNo );
}


INT32 SoldierTo3DLocationLineOfSightTest(const SOLDIERTYPE* pStartSoldier, INT16 sGridNo, INT8 bLevel, INT8 bCubeLevel, UINT8 ubTileSightLimit, INT8 bAware)
{
	FLOAT dStartZPos, dEndZPos;
	BOOLEAN fOk;

	CHECKF( pStartSoldier );

	fOk = CalculateSoldierZPos( pStartSoldier, LOS_POS, &dStartZPos );
	CHECKF( fOk );

	if (bCubeLevel > 0)
	{
		dEndZPos = ((FLOAT) (bCubeLevel + bLevel * PROFILE_Z_SIZE ) - 0.5f) * HEIGHT_UNITS_PER_INDEX;
		dEndZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ sGridNo ].sHeight );
	}
	else
	{
		const SOLDIERTYPE* const tgt = WhoIsThere2(sGridNo, bLevel);
		if (tgt != NULL)
		{
			// there's a merc there; do a soldier-to-soldier test
			return SoldierToSoldierLineOfSightTest(pStartSoldier, tgt, ubTileSightLimit, bAware);
		}
		// else... assume standing height
		dEndZPos = STANDING_LOS_POS + bLevel * HEIGHT_UNITS;
		// add in ground height
		dEndZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ sGridNo ].sHeight );
	}

	return LineOfSightTest(pStartSoldier->sGridNo, dStartZPos, sGridNo, dEndZPos, ubTileSightLimit, gubTreeSightReduction[ANIM_STAND], bAware, 0, FALSE, NULL);
}

INT32 SoldierToBodyPartLineOfSightTest( const SOLDIERTYPE * pStartSoldier, INT16 sGridNo, INT8 bLevel, UINT8 ubAimLocation, UINT8 ubTileSightLimit, INT8 bAware )
{
	FLOAT dStartZPos, dEndZPos;
	BOOLEAN fOk;
	UINT8 ubPosType;

	const SOLDIERTYPE* const pEndSoldier = WhoIsThere2(sGridNo, bLevel);
	// CJC August 13, 2002: for this routine to work there MUST be a target at the location specified
	if (pEndSoldier == NULL) return 0;

	CHECKF( pStartSoldier );

	fOk = CalculateSoldierZPos( pStartSoldier, LOS_POS, &dStartZPos );
	CHECKF( fOk );

	switch( ubAimLocation )
	{
		case AIM_SHOT_HEAD:
			ubPosType = HEAD_TARGET_POS;
			break;
		case AIM_SHOT_TORSO:
			ubPosType = TORSO_TARGET_POS;
			break;
		case AIM_SHOT_LEGS:
			ubPosType = LEGS_TARGET_POS;
			break;
		default:
			ubPosType = TARGET_POS;
			break;
	}

	fOk = CalculateSoldierZPos( pEndSoldier, ubPosType, &dEndZPos );
	if (!fOk)
	{
		return( FALSE );
	}

	return LineOfSightTest(pStartSoldier->sGridNo, dStartZPos, sGridNo, dEndZPos, ubTileSightLimit, gubTreeSightReduction[ANIM_STAND], bAware, 0, FALSE, NULL);
}


INT32 SoldierToVirtualSoldierLineOfSightTest(const SOLDIERTYPE* pStartSoldier, INT16 sGridNo, INT8 bLevel, INT8 bStance, UINT8 ubTileSightLimit, INT8 bAware)
{
	FLOAT dStartZPos, dEndZPos;
	BOOLEAN fOk;

	CHECKF( pStartSoldier );

	fOk = CalculateSoldierZPos( pStartSoldier, LOS_POS, &dStartZPos );
	CHECKF( fOk );

	// manually calculate destination Z position.
	switch( bStance )
	{
		case ANIM_STAND:
			dEndZPos = STANDING_LOS_POS;
			break;
		case ANIM_CROUCH:
			dEndZPos = CROUCHED_LOS_POS;
			break;
		case ANIM_PRONE:
			dEndZPos = PRONE_LOS_POS;
			break;
		default:
			return( FALSE );
	}
	dEndZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[sGridNo].sHeight );
	if (bLevel > 0)
	{
		// on a roof
		dEndZPos += WALL_HEIGHT_UNITS;
	}

	return LineOfSightTest(pStartSoldier->sGridNo, dStartZPos, sGridNo, dEndZPos, ubTileSightLimit, gubTreeSightReduction[ANIM_STAND], bAware, 0, FALSE, NULL);
}

INT32 SoldierToLocationLineOfSightTest( SOLDIERTYPE * pStartSoldier, INT16 sGridNo, UINT8 ubTileSightLimit, INT8 bAware )
{
	return( SoldierTo3DLocationLineOfSightTest( pStartSoldier, sGridNo, 0, 0, ubTileSightLimit, bAware ) );
}

INT32 LocationToLocationLineOfSightTest( INT16 sStartGridNo, INT8 bStartLevel, INT16 sEndGridNo, INT8 bEndLevel, UINT8 ubTileSightLimit, INT8 bAware )
{
	FLOAT dStartZPos, dEndZPos;

	const SOLDIERTYPE* const start_soldier = WhoIsThere2(sStartGridNo, bStartLevel);
	if (start_soldier != NULL)
	{
		return SoldierTo3DLocationLineOfSightTest(start_soldier, sEndGridNo, bEndLevel, 0, ubTileSightLimit, bAware);
	}

	// else... assume standing heights
	dStartZPos = STANDING_LOS_POS + bStartLevel * HEIGHT_UNITS;
	// add in ground height
	dStartZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ sStartGridNo ].sHeight );

	dEndZPos = STANDING_LOS_POS + bEndLevel * HEIGHT_UNITS;
	// add in ground height
	dEndZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ sEndGridNo ].sHeight );

	return LineOfSightTest(sStartGridNo, dStartZPos, sEndGridNo, dEndZPos, ubTileSightLimit, gubTreeSightReduction[ANIM_STAND], bAware, 0, FALSE, NULL);
}

/*
INT32 BulletImpactReducedByRange( INT32 iImpact, INT32 iDistanceTravelled, INT32 iRange )
{
	// for now, don't reduce, because did weird stuff to AI!
	return( iImpact );

	// only start reducing impact at distances greater than one range
	//return( __max( 1, iImpact * ( 100 - ( PERCENT_BULLET_SLOWED_BY_RANGE * iDistanceTravelled ) / iRange ) / 100 ) );

}*/


static BOOLEAN BulletHitMerc(BULLET* pBullet, STRUCTURE* pStructure, BOOLEAN fIntended)
{
	SOLDIERTYPE* const pFirer = pBullet->pFirer;
	INT32 iImpact, iDamage;
	FLOAT dZPosRelToMerc;
	UINT8 ubHitLocation = AIM_SHOT_RANDOM;
	UINT8 ubAttackDirection;
	UINT8 ubAmmoType;
	UINT32 uiChanceThrough;
	UINT8 ubSpecial = FIRE_WEAPON_NO_SPECIAL;
	INT16 sHitBy;
	BOOLEAN fStopped = TRUE;
	INT8 bSlot;
	INT8 bHeadSlot = NO_SLOT;
	OBJECTTYPE Object;
	INT16 sNewGridNo;
	BOOLEAN fCanSpewBlood = FALSE;
	INT8 bSpewBloodLevel;

	// structure IDs for mercs match their merc IDs
	SOLDIERTYPE& tgt = GetMan(pStructure->usStructureID);

	if (pBullet->usFlags & BULLET_FLAG_KNIFE)
	{
		// Place knife on guy....

		// See if they have room ( and make sure it's not in hand pos?
		bSlot = FindEmptySlotWithin(&tgt, BIGPOCK1POS, SMALLPOCK8POS );
		if (bSlot == NO_SLOT)
		{
			// Add item
			CreateItem(BLOODY_THROWING_KNIFE, (INT8) pBullet->ubItemStatus, &Object);

			AddItemToPool(tgt.sGridNo, &Object, INVISIBLE, tgt.bLevel, 0, 0);

			// Make team look for items
			NotifySoldiersToLookforItems( );
		}
		else
		{
			CreateItem(BLOODY_THROWING_KNIFE, (INT8) pBullet->ubItemStatus, &tgt.inv[bSlot]);
		}

		ubAmmoType = AMMO_KNIFE;
	}
	else
	{
		ubAmmoType = pFirer->inv[pFirer->ubAttackingHand].ubGunAmmoType;
	}

	// at least partly compensate for "near miss" increases for this guy, after all, the bullet
	// actually hit him!
	// take this out for now at least... no longer certain that he was awarded a suppression pt
	// when the bullet got near him
	//--tgt.ubSuppressionPoints;

	if (tgt.uiStatusFlags & SOLDIER_VEHICLE || (tgt.ubBodyType == COW || tgt.ubBodyType == CROW || tgt.ubBodyType == BLOODCAT))
	{
		//ubHitLocation = pStructure->ubVehicleHitLocation;
		ubHitLocation = AIM_SHOT_TORSO;
	}
	else
	{
		// Determine where the person was hit...

		if (CREATURE_OR_BLOODCAT(&tgt))
		{
			ubHitLocation = AIM_SHOT_TORSO;

			// adult monster types have a weak spot
			if (ADULTFEMALEMONSTER <= tgt.ubBodyType && tgt.ubBodyType <= YAM_MONSTER)
			{
				ubAttackDirection = (UINT8)GetDirectionToGridNoFromGridNo( pBullet->pFirer->sGridNo, tgt.sGridNo);
				if (ubAttackDirection == tgt.bDirection ||
					ubAttackDirection == OneCCDirection(tgt.bDirection) ||
					ubAttackDirection == OneCDirection(tgt.bDirection))
				{
					// may hit weak spot!
					if (0) // check fact
					{
						uiChanceThrough = 30;
					}
					else
					{
						uiChanceThrough = 1;
					}

					if (PreRandom( 100 ) < uiChanceThrough)
					{
						ubHitLocation = AIM_SHOT_GLAND;
					}
				}
			}
		}

		if (ubHitLocation == AIM_SHOT_RANDOM) // i.e. if not set yet
		{
			if (tgt.bOverTerrainType == DEEP_WATER)
			{
				// automatic head hit!
				ubHitLocation = AIM_SHOT_HEAD;
			}
			else
			{
				switch (gAnimControl[tgt.usAnimState].ubEndHeight)
				{
					case ANIM_STAND:
						dZPosRelToMerc = FixedToFloat( pBullet->qCurrZ ) - CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[pBullet->sGridNo].sHeight );
						if ( dZPosRelToMerc > HEIGHT_UNITS )
						{
							dZPosRelToMerc -= HEIGHT_UNITS;
						}
						if (dZPosRelToMerc > STANDING_HEAD_BOTTOM_POS)
						{
							ubHitLocation = AIM_SHOT_HEAD;
						}
						else if (dZPosRelToMerc < STANDING_TORSO_BOTTOM_POS )
						{
							ubHitLocation = AIM_SHOT_LEGS;
						}
						else
						{
							ubHitLocation = AIM_SHOT_TORSO;
						}
						break;
					case ANIM_CROUCH:
						dZPosRelToMerc = FixedToFloat( pBullet->qCurrZ ) - CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[pBullet->sGridNo].sHeight );
						if ( dZPosRelToMerc > HEIGHT_UNITS )
						{
							dZPosRelToMerc -= HEIGHT_UNITS;
						}
						if (dZPosRelToMerc > CROUCHED_HEAD_BOTTOM_POS)
						{
							ubHitLocation = AIM_SHOT_HEAD;
						}
						else if ( dZPosRelToMerc < CROUCHED_TORSO_BOTTOM_POS )
						{
							// prevent targets in water from being hit in legs
							ubHitLocation = AIM_SHOT_LEGS;
						}
						else
						{
							ubHitLocation = AIM_SHOT_TORSO;
						}
						break;
					case ANIM_PRONE:
						ubHitLocation = AIM_SHOT_TORSO;
						break;

				}
			}
		}

		if (ubAmmoType == AMMO_MONSTER && (ubHitLocation == AIM_SHOT_HEAD) && !(tgt.uiStatusFlags & SOLDIER_MONSTER))
		{
			UINT8 ubOppositeDirection;

			ubAttackDirection   = (UINT8)GetDirectionToGridNoFromGridNo(pBullet->pFirer->sGridNo, tgt.sGridNo);
			ubOppositeDirection = OppositeDirection(ubAttackDirection);

			if (ubOppositeDirection != tgt.bDirection &&
				ubAttackDirection != OneCCDirection(tgt.bDirection) &&
				ubAttackDirection != OneCDirection(tgt.bDirection))
			{
				// lucky bastard was facing away!
			}
			else if ((tgt.inv[HEAD1POS].usItem == NIGHTGOGGLES || tgt.inv[HEAD1POS].usItem == SUNGOGGLES ||
				tgt.inv[HEAD1POS].usItem == GASMASK) && static_cast<INT8>(PreRandom(100)) < tgt.inv[HEAD1POS].bStatus[0])
			{
				// lucky bastard was wearing protective stuff
				bHeadSlot = HEAD1POS;
			}
			else if ((tgt.inv[HEAD2POS].usItem == NIGHTGOGGLES || tgt.inv[HEAD2POS].usItem == SUNGOGGLES ||
				tgt.inv[HEAD2POS].usItem == GASMASK) && static_cast<INT8>(PreRandom(100)) < tgt.inv[HEAD2POS].bStatus[0])
			{
				// lucky bastard was wearing protective stuff
				bHeadSlot = HEAD2POS;
			}
			else
			{
				// splat!!
				ubSpecial = FIRE_WEAPON_BLINDED_BY_SPIT_SPECIAL;
			}

		}

	}

	// Determine damage, checking guy's armour, etc
	INT16 const sRange = GetRangeInCellCoordsFromGridNoDiff(pFirer->sGridNo, tgt.sGridNo);
	if ( gTacticalStatus.uiFlags & GODMODE  && !(pFirer->uiStatusFlags & SOLDIER_PC))
	{
		// in god mode, and firer is computer controlled
		iImpact = 0;
		iDamage = 0;
	}
	else if (fIntended)
	{
		if (pFirer->bOppList[tgt.ubID] == SEEN_CURRENTLY)
		{
			sHitBy = pBullet->sHitBy;
		}
		else
		{
			// hard to aim at something far away being reported by someone else!
			sHitBy = pBullet->sHitBy / 2;
		}
		// hit the intended target which was in our LOS
		// reduce due to range
		iImpact = pBullet->iImpact; //BulletImpactReducedByRange( pBullet->iImpact, pBullet->iLoop, pBullet->iRange );
		iImpact -= pBullet->iImpactReduction;
		if (iImpact < 0)
		{
			// shouldn't happen but
			iImpact = 0;
		}
		iDamage = BulletImpact(pFirer, &tgt, ubHitLocation, iImpact, sHitBy, &ubSpecial);
		// handle hit here...
		if( pFirer->bTeam == 0 )
		{
			// issue #1140 : sync usShotsFired with usShotsHit and so that merc statistic <= 100%
			if (pFirer->target->bLife > 0) gMercProfiles[ pFirer->ubProfile ].usShotsHit++;
		}

		// intentionally shot
		tgt.fIntendedTarget = TRUE;

		if (pBullet->usFlags & BULLET_FLAG_BUCKSHOT && &tgt == pFirer->target)
		{
			++tgt.bNumPelletsHitBy;
		}
	}
	else
	{
		// if an accidental target was hit, don't give a bonus for good aim!
		sHitBy = 0;
		iImpact = pBullet->iImpact;
		//iImpact = BulletImpactReducedByRange( pBullet->iImpact, pBullet->iLoop, pBullet->iRange );
		iImpact -= pBullet->iImpactReduction;
		if (iImpact < 0)
		{
			// shouldn't happen but
			iImpact = 0;
		}
		iDamage = BulletImpact(pFirer, &tgt, ubHitLocation, iImpact, sHitBy, &ubSpecial);

		// accidentally shot
		tgt.fIntendedTarget = FALSE;
	}

	if ( ubAmmoType == AMMO_MONSTER )
	{
		if (bHeadSlot != NO_SLOT)
		{
			tgt.inv[bHeadSlot].bStatus[0] -= (INT8)(iImpact / 2 + Random(iImpact / 2));
			if (tgt.inv[bHeadSlot].bStatus[0] <= USABLE)
			{
				if (tgt.inv[bHeadSlot].bStatus[0] <= 0)
				{
					DeleteObj(&tgt.inv[bHeadSlot]);
					DirtyMercPanelInterface(&tgt, DIRTYLEVEL2);
				}
				// say curse?
			}
		}
	}
	else if (ubHitLocation == AIM_SHOT_HEAD)
	{
		// bullet to the head may damage any head item
		bHeadSlot = HEAD1POS + (INT8) Random( 2 );
		if (tgt.inv[bHeadSlot].usItem != NOTHING)
		{
			tgt.inv[bHeadSlot].bStatus[0] -= (INT8)(Random(iImpact / 2));
			if (tgt.inv[bHeadSlot].bStatus[0] < 0)
			{
				// just break it...
				tgt.inv[bHeadSlot].bStatus[0] = 1;
			}
		}
	}

	// check to see if the guy is a friendly?..if so, up the number of times wounded
	if (tgt.bTeam == OUR_TEAM)
	{
		++gMercProfiles[tgt.ubProfile].usTimesWounded;
	}

	// check to see if someone was accidentally hit when no target was specified by the player
	if (pFirer->bTeam == OUR_TEAM && pFirer->target == NULL && tgt.bNeutral)
	{
		if (tgt.ubCivilianGroup == KINGPIN_CIV_GROUP || tgt.ubCivilianGroup == HICKS_CIV_GROUP)
		{
			// hicks and kingpin are touchy!
			pFirer->target = &tgt;
		}
		else if ( Random( 100 ) < 60 )
		{
			// get touchy
			pFirer->target = &tgt;
		}

	}

	if ( (pFirer->bDoBurst) && (ubSpecial == FIRE_WEAPON_NO_SPECIAL) )
	{
		// the animation required by the bullet hit (head explosion etc) overrides the
		// hit-by-a-burst animation
		ubSpecial = FIRE_WEAPON_BURST_SPECIAL;
	}

	// breath loss is based on original impact of bullet
	INT16  const breath_loss   = (iImpact * BP_GET_WOUNDED * (tgt.bBreathMax * 100 - tgt.sBreathRed)) / 10000;
	UINT16 const hit_direction = GetDirectionFromGridNo(pFirer->sGridNo, &tgt);

	// now check to see if the bullet goes THROUGH this person! (not vehicles)
	if (!(tgt.uiStatusFlags & SOLDIER_VEHICLE) &&
		(ubAmmoType == AMMO_REGULAR || ubAmmoType == AMMO_AP || ubAmmoType == AMMO_SUPER_AP) &&
		!EXPLOSIVE_GUN(pFirer->usAttackingWeapon))
	{
		// if we do more damage than expected, then the bullet will be more likely
		// to be lodged in the body

		// if we do less than expected, then the bullet has been slowed and less
		// likely to continue

		// higher chance for bigger guns, because they'll go through the back armour

		// reduce impact to match damage, if damage wasn't more than the impact
		// due to good aim, etc.
		if (iDamage < iImpact)
		{
			iImpact = iDamage;
		}
		uiChanceThrough = (UINT8) __max( 0, ( iImpact - 20 ) );
		if (PreRandom( 100 ) < uiChanceThrough )
		{
			// bullet MAY go through
			// adjust for bullet going through person
			iImpact -= CalcBodyImpactReduction( ubAmmoType, ubHitLocation );
			// adjust for other side of armour!
			iImpact -= TotalArmourProtection(tgt, ubHitLocation, iImpact, ubAmmoType);
			if (iImpact > 0)
			{
				pBullet->iImpact = (INT8) iImpact;
				// bullet was NOT stopped
				fStopped = FALSE;
			}
		}
	}

	if (fStopped)
	{
		RemoveBullet(pBullet);
	}
	else
	{
		// ATE: I'm in enemy territory again, evil CC's world :)
		// This looks like the place I should add code to spew blood on the ground
		// The algorithm I'm going to use is given the current gridno of bullet,
		// get a new gridno based on direction it was moving.  Check to see if we're not
		// going through walls, etc by testing for a path, unless on the roof, in which case it would always
		// be legal, but the bLevel May change...
		sNewGridNo = NewGridNo(pBullet->sGridNo, DirectionInc(OppositeDirection(hit_direction)));

		bSpewBloodLevel = tgt.bLevel;
		fCanSpewBlood   = TRUE;

		// If on anything other than bLevel of 0, we can pretty much freely spew blood
		if ( bSpewBloodLevel == 0 )
		{
			if (gubWorldMovementCosts[sNewGridNo][OppositeDirection(hit_direction)][0] >= TRAVELCOST_BLOCKED)
			{
				fCanSpewBlood = FALSE;
			}
		}
		else
		{
			// If a roof does not exist here, make level = 0
			if ( !IsRoofPresentAtGridno( sNewGridNo ) )
			{
				bSpewBloodLevel = 0;
			}
		}

		if ( fCanSpewBlood )
		{
			// Drop blood dude!
			InternalDropBlood(sNewGridNo, bSpewBloodLevel, HUMAN, MAXBLOODQUANTITY, 1);
		}
	}

	if (gTacticalStatus.ubCurrentTeam != OUR_TEAM && tgt.bTeam == OUR_TEAM)
	{
		// someone has been hit so no close-call quotes
		gTacticalStatus.fSomeoneHit = TRUE;
	}

	// handle hit!
	WeaponHit(&tgt, pFirer->usAttackingWeapon, iDamage, breath_loss, hit_direction, tgt.dXPos, tgt.dYPos, 20, sRange, pFirer, ubSpecial, ubHitLocation);
	return( fStopped );
}


static void BulletHitStructure(BULLET* pBullet, UINT16 usStructureID, INT32 iImpact, BOOLEAN fStopped)
{
	const FIXEDPT qCurrX = pBullet->qCurrX;
	const FIXEDPT qCurrY = pBullet->qCurrY;
	const FIXEDPT qCurrZ = pBullet->qCurrZ;
	INT16 sXPos = FIXEDPT_TO_INT32(qCurrX + FloatToFixed(0.5f)); // + 0.5);
	INT16 sYPos = FIXEDPT_TO_INT32(qCurrY + FloatToFixed(0.5f)); // (dCurrY + 0.5);
	INT16 sZPos = CONVERT_HEIGHTUNITS_TO_PIXELS((INT16)FIXEDPT_TO_INT32(qCurrZ + FloatToFixed(0.5f)));// dCurrZ + 0.5) );
	StructureHit(pBullet, sXPos, sYPos, sZPos, usStructureID, iImpact, fStopped);
}


static void BulletHitWindow(BULLET* pBullet, INT16 sGridNo, UINT16 usStructureID, BOOLEAN fBlowWindowSouth)
{
	WindowHit( sGridNo, usStructureID, fBlowWindowSouth, FALSE );
}


static UINT32 ChanceOfBulletHittingStructure(INT32 iDistance, INT32 iDistanceToTarget, INT16 sHitBy)
{
	INT32 iCloseToCoverPenalty;

	if (iDistance / CELL_X_SIZE > MAX_DIST_FOR_LESS_THAN_MAX_CHANCE_TO_HIT_STRUCTURE)
	{
		return( MAX_CHANCE_OF_HITTING_STRUCTURE );
	}
	else
	{
		iCloseToCoverPenalty = iDistance / 5 - (iDistanceToTarget - iDistance);
		if (iCloseToCoverPenalty < 0)
		{
			iCloseToCoverPenalty = 0;
		}
		if (sHitBy < 0)
		{
			// add 20% to distance so that misses hit nearer obstacles a bit more
			iDistance += iDistance / 5;
		}
		if ( ( (iDistance + iCloseToCoverPenalty) / CELL_X_SIZE ) > MAX_DIST_FOR_LESS_THAN_MAX_CHANCE_TO_HIT_STRUCTURE )
		{
			return( MAX_CHANCE_OF_HITTING_STRUCTURE );
		}
		else
		{
			return( guiStructureHitChance[ (iDistance + iCloseToCoverPenalty) / CELL_X_SIZE ] );
		}
	}
}


static INT32 StructureResistanceIncreasedByRange(INT32 iImpactReduction, INT32 iGunRange, INT32 iDistance)
{
	return( iImpactReduction * ( 100 + PERCENT_BULLET_SLOWED_BY_RANGE * (iDistance - iGunRange) / iGunRange ) / 100 );
	/*
	if ( iDistance > iGunRange )
	{
		return( iImpactReduction * ( 100 + PERCENT_BULLET_SLOWED_BY_RANGE * (iDistance - iGunRange) / iGunRange ) / 100 );
	}
	else
	{
		return( iImpactReduction );
	}*/
}


static INT32 HandleBulletStructureInteraction(BULLET* pBullet, STRUCTURE* pStructure, BOOLEAN* pfHit)
{
	DOOR  *pDoor;
	INT16 sLockDamage;

	// returns remaining impact amount


	INT32 iCurrImpact;
	INT32 iImpactReduction;

	*pfHit = FALSE;

	if (pBullet->usFlags & BULLET_FLAG_KNIFE || pBullet->usFlags & BULLET_FLAG_MISSILE || pBullet->usFlags & BULLET_FLAG_TANK_CANNON || pBullet->usFlags & BULLET_FLAG_FLAME )
	{
		// stops!
		*pfHit = TRUE;
		return( 0 );
	}
	else if ( pBullet->usFlags & BULLET_FLAG_SMALL_MISSILE )
	{
		// stops if using HE ammo
		if (pBullet->pFirer->inv[ pBullet->pFirer->ubAttackingHand ].ubGunAmmoType == AMMO_HE)
		{
			*pfHit = TRUE;
			return( 0 );
		}
	}

	// ATE: Alrighty, check for shooting door locks...
	// First check this is a type of struct that can handle locks...
	if ( pStructure->fFlags & ( STRUCTURE_DOOR | STRUCTURE_OPENABLE ) && PythSpacesAway( (INT16) pBullet->sTargetGridNo, pStructure->sGridNo ) <= 2 )
	{
		// lookup lock table to see if we have a lock,
		// and then remove lock if enough damage done....
		pDoor = FindDoorInfoAtGridNo( pBullet->sGridNo );

		// Does it have a lock?
		if ( pDoor && LockTable[ pDoor->ubLockID ].ubPickDifficulty < 50 && LockTable[ pDoor->ubLockID ].ubSmashDifficulty < 70 )
		{
			// Yup.....

			// Chance that it hit the lock....
			if ( PreRandom( 2 ) == 0 )
			{
				// Adjust damage-- CC adjust this based on gun type, etc.....
				//sLockDamage = (INT16)( 35 + Random( 35 ) );
				sLockDamage = (INT16) (pBullet->iImpact - pBullet->iImpactReduction);
				sLockDamage += (INT16) PreRandom( sLockDamage );

				ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ LOCK_HAS_BEEN_HIT ] );

				pDoor->bLockDamage+= sLockDamage;

				// Check if it has been shot!
				if ( pDoor->bLockDamage > LockTable[ pDoor->ubLockID ].ubSmashDifficulty )
				{
					// Display message!
					ScreenMsg( FONT_MCOLOR_LTYELLOW, MSG_INTERFACE, TacticalStr[ LOCK_HAS_BEEN_DESTROYED ] );

					// succeeded! door can never be locked again, so remove from door list...
					RemoveDoorInfoFromTable( pDoor->sGridNo );

					// MARKSMANSHIP GAIN (marksPts): Opened/Damaged a door
					StatChange(*pBullet->pFirer, MARKAMT, 10, FROM_SUCCESS);
				}
			}
		}
	}

	// okay, this seems pretty weird, so here's the comment to explain it:
	// iImpactReduction is the reduction in impact due to the structure
	// pBullet->iImpactReduction is the accumulated reduction in impact
	//   for all bullets encountered thus far
	// iCurrImpact is the original impact value of the bullet reduced due to
	//   range.  To avoid problems involving multiple multiplication
	//   ( (1 - X) * (1 - Y) != (1 - X - Y) ! ), this is calculated from
	//   scratch at each collision with an obstacle
	//   reduction due to range is 25% per "max range"
	if (PreRandom( 100 ) < pStructure->pDBStructureRef->pDBStructure->ubDensity)
	{
		iCurrImpact = pBullet->iImpact;
		//iCurrImpact = BulletImpactReducedByRange( pBullet->iImpact, pBullet->iLoop, pBullet->iRange );
		iImpactReduction = gubMaterialArmour[ pStructure->pDBStructureRef->pDBStructure->ubArmour ];
		iImpactReduction = StructureResistanceIncreasedByRange( iImpactReduction, pBullet->iRange, pBullet->iLoop );

		switch (pBullet->pFirer->inv[ pBullet->pFirer->ubAttackingHand ].ubGunAmmoType)
		{
			case AMMO_HP:
				iImpactReduction = AMMO_STRUCTURE_ADJUSTMENT_HP( iImpactReduction );
				break;
			case AMMO_AP:
			case AMMO_HEAT:
				iImpactReduction = AMMO_STRUCTURE_ADJUSTMENT_AP( iImpactReduction );
				break;
			case AMMO_SUPER_AP:
				iImpactReduction = AMMO_STRUCTURE_ADJUSTMENT_SAP( iImpactReduction );
				break;
			default:
				break;
		}

		pBullet->iImpactReduction += iImpactReduction;

		// really weak stuff like grass should never *stop* a bullet, maybe slow it though
		if ( pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_LIGHT_VEGETATION )
		{
			// just return a +ve value to indicate the bullet wasn't stopped
			*pfHit = FALSE;
			return( 1 );
		}

		*pfHit = TRUE;
		return( iCurrImpact - pBullet->iImpactReduction );
	}
	else
	{
		// just return a +ve value to indicate the bullet wasn't stopped
		*pfHit = FALSE;
		return( 1 );
	}
}


static INT32 CTGTHandleBulletStructureInteraction(BULLET* pBullet, STRUCTURE* pStructure)
{
	// returns reduction in impact for summing in CTGT

	//INT32 iCurrImpact;
	INT32 iImpactReduction;

	if (pBullet->usFlags & BULLET_FLAG_KNIFE || pBullet->usFlags & BULLET_FLAG_MISSILE || pBullet->usFlags & BULLET_FLAG_FLAME || pBullet->usFlags & BULLET_FLAG_TANK_CANNON )
	{
		// knife/rocket stops when it hits anything, and people block completely
		return( pBullet->iImpact );
	}
	else if ( pBullet->usFlags & BULLET_FLAG_SMALL_MISSILE )
	{
		// stops if using HE ammo
		if (pBullet->pFirer->inv[ pBullet->pFirer->ubAttackingHand ].ubGunAmmoType == AMMO_HE)
		{
			return( pBullet->iImpact );
		}
	}

	// okay, this seems pretty weird, so here's the comment to explain it:
	// iImpactReduction is the reduction in impact due to the structure
	// pBullet->iImpactReduction is the accumulated reduction in impact
	//   for all bullets encountered thus far
	// iCurrImpact is the original impact value of the bullet reduced due to
	//   range.  To avoid problems involving multiple multiplication
	//   ( (1 - X) * (1 - Y) != (1 - X - Y) ! ), this is calculated from
	//   scratch at each collision with an obstacle
	//   reduction due to range is 25% per "max range"
	//iCurrImpact = BulletImpactReducedByRange( pBullet->iImpact, pBullet->iLoop, pBullet->iRange );
	//iCurrImpact = pBullet->iImpact;
	// multiply impact reduction by 100 to retain fractions for a bit...
	iImpactReduction = gubMaterialArmour[ pStructure->pDBStructureRef->pDBStructure->ubArmour ] * pStructure->pDBStructureRef->pDBStructure->ubDensity / 100;
	iImpactReduction = StructureResistanceIncreasedByRange( iImpactReduction, pBullet->iRange, pBullet->iLoop );
	switch (pBullet->pFirer->inv[ pBullet->pFirer->ubAttackingHand ].ubGunAmmoType)
	{
		case AMMO_HP:
			iImpactReduction = AMMO_STRUCTURE_ADJUSTMENT_HP( iImpactReduction );
			break;
		case AMMO_AP:
			iImpactReduction = AMMO_STRUCTURE_ADJUSTMENT_AP( iImpactReduction );
			break;
		case AMMO_SUPER_AP:
			iImpactReduction = AMMO_STRUCTURE_ADJUSTMENT_SAP( iImpactReduction );
			break;
		default:
			break;
	}
	return( iImpactReduction );
}


static UINT8 CalcChanceToGetThrough(BULLET* pBullet)
{
	FIXEDPT qLandHeight;
	INT32 iCurrAboveLevelZ;
	INT32 iCurrCubesAboveLevelZ;
	INT16 sDesiredLevel;

	INT32 iOldTileX;
	INT32 iOldTileY;
	INT32 iOldCubesZ;

	MAP_ELEMENT * pMapElement;
	STRUCTURE * pStructure;
	STRUCTURE * pRoofStructure = NULL;

	FIXEDPT qLastZ;

	BOOLEAN fIntended;
	INT8 bOldLOSIndexX;
	INT8 bOldLOSIndexY;

	INT32 iChanceToGetThrough = 100;

	FIXEDPT qDistToTravelX;
	FIXEDPT qDistToTravelY;
	INT32 iStepsToTravelX;
	INT32 iStepsToTravelY;
	INT32 iStepsToTravel;
	INT32 iNumLocalStructures;
	INT32 iStructureLoop;
	UINT32 uiChanceOfHit;
	INT32 iGridNo;
	INT32 iTotalStructureImpact;
	BOOLEAN fResolveHit;

	FIXEDPT qWallHeight;
	FIXEDPT qWindowBottomHeight;
	FIXEDPT qWindowTopHeight;

	SLOGD("Starting CalcChanceToGetThrough" );

	do
	{
		// check a particular tile
		// retrieve values from world for this particular tile
		iGridNo = pBullet->iCurrTileX + pBullet->iCurrTileY * WORLD_COLS;
		STLOGD("CTGT now at {}", iGridNo);
		pMapElement = &(gpWorldLevelData[ iGridNo ] );
		qLandHeight = INT32_TO_FIXEDPT( CONVERT_PIXELS_TO_HEIGHTUNITS( pMapElement->sHeight ) );
		qWallHeight = gqStandardWallHeight + qLandHeight;
		qWindowBottomHeight = gqStandardWindowBottomHeight + qLandHeight;
		qWindowTopHeight = gqStandardWindowTopHeight + qLandHeight;

		// Assemble list of structures we might hit!
		iNumLocalStructures = 0;
		pStructure = pMapElement->pStructureHead;
		// calculate chance of hitting each structure
		uiChanceOfHit = ChanceOfBulletHittingStructure( pBullet->iLoop, pBullet->iDistanceLimit, 0 );

		// reset roof structure pointer each tile
		pRoofStructure = NULL;

		if (iGridNo == (INT32) pBullet->sTargetGridNo)
		{
			fIntended = TRUE;
			// if in the same tile as our destination, we WANT to hit the structure!
			uiChanceOfHit = 100;
		}
		else
		{
			fIntended = FALSE;
		}

		iCurrAboveLevelZ = FIXEDPT_TO_INT32( pBullet->qCurrZ - qLandHeight );
		if (iCurrAboveLevelZ < 0)
		{
			// ground is in the way!
			return( 0 );
		}
		iCurrCubesAboveLevelZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrAboveLevelZ );

		while( pStructure )
		{
			if (pStructure->fFlags & ALWAYS_CONSIDER_HIT)
			{
				// ALWAYS add walls
				gpLocalStructure[iNumLocalStructures] = pStructure;
				// fence is special
				//(iCurrCubesAboveLevelZ <= iStartCubesAboveLevelZ)
				if ( pStructure->fFlags & STRUCTURE_ANYFENCE )
				{
					if (pStructure->pDBStructureRef->pDBStructure->ubDensity < 100)
					{
						guiLocalStructureCTH[iNumLocalStructures] = uiChanceOfHit;
					}
					else if ((pBullet->iLoop <= CLOSE_TO_FIRER) && (iCurrCubesAboveLevelZ <= pBullet->bStartCubesAboveLevelZ) &&
						(pBullet->bEndCubesAboveLevelZ >= iCurrCubesAboveLevelZ) &&
						iCurrCubesAboveLevelZ == (StructureHeight(pStructure) - 1))
					{
						guiLocalStructureCTH[iNumLocalStructures] = uiChanceOfHit;
					}
					else if ((pBullet->iDistanceLimit - pBullet->iLoop <= CLOSE_TO_FIRER) &&
						(iCurrCubesAboveLevelZ <= pBullet->bEndCubesAboveLevelZ) &&
						iCurrCubesAboveLevelZ == (StructureHeight( pStructure ) - 1))
					{
						guiLocalStructureCTH[iNumLocalStructures] = uiChanceOfHit;
					}
					else
					{
						guiLocalStructureCTH[iNumLocalStructures] = 100;
					}
				}
				else
				{
					guiLocalStructureCTH[iNumLocalStructures] = 100;
				}
				gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
				iNumLocalStructures++;
			}
			else if (pStructure->fFlags & STRUCTURE_ROOF)
			{
				// only consider roofs if the flag is set; don't add them to the array since they
				// are a special case
				if (pBullet->fCheckForRoof)
				{
					pRoofStructure = pStructure;

					if ( pRoofStructure )
					{
						qLastZ = pBullet->qCurrZ - pBullet->qIncrZ;

						// if just on going to next tile we cross boundary, then roof stops bullet here!
						if ((qLastZ > qWallHeight && pBullet->qCurrZ <= qWallHeight) ||
							(qLastZ < qWallHeight && pBullet->qCurrZ >= qWallHeight))
						{
							// hit a roof
							return( 0 );
						}
					}

				}
			}
			else if (pStructure->fFlags & STRUCTURE_PERSON)
			{
				SOLDIERTYPE const& person = GetMan(pStructure->usStructureID);
				if (&person != pBullet->pFirer && &person != pBullet->target)
				{
					// ignore intended target since we will get closure upon reaching the center
					// of the destination tile

					// ignore intervening target if not visible; PCs are always visible so AI will never skip them on that
					// basis
					if (!fIntended && person.bVisible == TRUE)
					{
						// in actually moving the bullet, we consider only count friends as targets if the bullet is unaimed
						// (buckshot), if they are the intended target, or beyond the range of automatic friendly fire hits
						// OR a 1 in 30 chance occurs
						if (gAnimControl[person.usAnimState].ubEndHeight == ANIM_STAND &&
							((pBullet->fAimed && pBullet->iLoop > MIN_DIST_FOR_HIT_FRIENDS) ||
							(!pBullet->fAimed && pBullet->iLoop > MIN_DIST_FOR_HIT_FRIENDS_UNAIMED)))
						{
							// could hit this person!
							gpLocalStructure[iNumLocalStructures] = pStructure;
							// CJC commented this out because of tank trying to shoot through another tank
							//guiLocalStructureCTH[iNumLocalStructures] = uiChanceOfHit;
							guiLocalStructureCTH[iNumLocalStructures] = 100;
							gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
							iNumLocalStructures++;
						}
						else
						{
							// minimal chance of hitting this person
							gpLocalStructure[iNumLocalStructures] = pStructure;
							guiLocalStructureCTH[iNumLocalStructures] = MIN_CHANCE_TO_ACCIDENTALLY_HIT_SOMEONE;
							gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
							iNumLocalStructures++;
						}
					}
				}
			}
			else if ( pStructure->fFlags & STRUCTURE_CORPSE )
			{
				if ( iGridNo == (INT32) pBullet->sTargetGridNo || (pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles >= 10) )
				{
					// could hit this corpse!
					// but we should ignore the corpse if there is someone standing there
					if ( FindStructure( (INT16) iGridNo, STRUCTURE_PERSON ) == NULL )
					{
						gpLocalStructure[iNumLocalStructures] = pStructure;
						iNumLocalStructures++;
					}
				}
			}
			else
			{
				if ( pBullet->iLoop > CLOSE_TO_FIRER && !fIntended )
				{
					// could hit it

					gpLocalStructure[iNumLocalStructures] = pStructure;
					guiLocalStructureCTH[iNumLocalStructures] = uiChanceOfHit;
					gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
					iNumLocalStructures++;
				}
			}
			pStructure = pStructure->pNext;
		}

		// record old tile location for loop purposes
		iOldTileX = pBullet->iCurrTileX;
		iOldTileY = pBullet->iCurrTileY;

		do
		{
			// check a particular location within the tile

			// check for collision with the ground
			iCurrAboveLevelZ = FIXEDPT_TO_INT32( pBullet->qCurrZ - qLandHeight );
			if (iCurrAboveLevelZ < 0)
			{
				// ground is in the way!
				return( 0 );
			}
			// check for the existence of structures
			pStructure = pMapElement->pStructureHead;
			if (pStructure == NULL)
			{	// no structures in this tile, and THAT INCLUDES ROOFS AND PEOPLE! :-)
				// new system; figure out how many steps until we cross the next edge
				// and then fast forward that many steps.

				iOldTileX = pBullet->iCurrTileX;
				iOldTileY = pBullet->iCurrTileY;
				iOldCubesZ = pBullet->iCurrCubesZ;

				if (pBullet->qIncrX > 0)
				{
					qDistToTravelX = INT32_TO_FIXEDPT( CELL_X_SIZE ) - (pBullet->qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE ));
					iStepsToTravelX = qDistToTravelX / pBullet->qIncrX;
				}
				else if (pBullet->qIncrX < 0)
				{
					qDistToTravelX = pBullet->qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE );
					iStepsToTravelX = qDistToTravelX / (-pBullet->qIncrX);
				}
				else
				{
					// make sure we don't consider X a limit :-)
					iStepsToTravelX = 1000000;
				}

				if (pBullet->qIncrY > 0)
				{
					qDistToTravelY = INT32_TO_FIXEDPT( CELL_Y_SIZE ) - (pBullet->qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE ));
					iStepsToTravelY = qDistToTravelY / pBullet->qIncrY;
				}
				else if (pBullet->qIncrY < 0)
				{
					qDistToTravelY = pBullet->qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE );
					iStepsToTravelY = qDistToTravelY / (-pBullet->qIncrY);
				}
				else
				{
					// make sure we don't consider Y a limit :-)
					iStepsToTravelY = 1000000;
				}

				// add 1 to the # of steps to travel to go INTO the next tile
				iStepsToTravel = __min( iStepsToTravelX, iStepsToTravelY ) + 1;

				pBullet->qCurrX += pBullet->qIncrX * iStepsToTravel;
				pBullet->qCurrY += pBullet->qIncrY * iStepsToTravel;
				pBullet->qCurrZ += pBullet->qIncrZ * iStepsToTravel;
				pBullet->iLoop += iStepsToTravel;

				// check for ground collision
				if (pBullet->qCurrZ < qLandHeight && pBullet->iLoop < pBullet->iDistanceLimit)
				{
					// ground is in the way!
					return( 0 );
				}

				// figure out the new tile location
				pBullet->iCurrTileX = FIXEDPT_TO_TILE_NUM( pBullet->qCurrX );
				pBullet->iCurrTileY = FIXEDPT_TO_TILE_NUM( pBullet->qCurrY );
				pBullet->iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrZ ) );
				pBullet->bLOSIndexX = FIXEDPT_TO_LOS_INDEX( pBullet->qCurrX );
				pBullet->bLOSIndexY = FIXEDPT_TO_LOS_INDEX( pBullet->qCurrY );

				SLOGD(ST::format("CTGT at {} {} after traversing empty tile",
					pBullet->bLOSIndexX, pBullet->bLOSIndexY));
			}
			else
			{
				// there are structures in this tile

				iCurrCubesAboveLevelZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrAboveLevelZ );

				// figure out the LOS cube level of the current point
				if (iCurrCubesAboveLevelZ < STRUCTURE_ON_ROOF_MAX)
				{
					if (iCurrCubesAboveLevelZ < STRUCTURE_ON_GROUND_MAX)
					{
						// check objects on the ground
						sDesiredLevel = STRUCTURE_ON_GROUND;
					}
					else
					{
						// check objects on roofs
						sDesiredLevel = STRUCTURE_ON_ROOF;
						iCurrCubesAboveLevelZ -= STRUCTURE_ON_ROOF;
					}
					// check structures for collision
					for ( iStructureLoop = 0; iStructureLoop < iNumLocalStructures; iStructureLoop++)
					{
						pStructure = gpLocalStructure[iStructureLoop];
						if (pStructure && pStructure->sCubeOffset == sDesiredLevel)
						{
							if (((*(pStructure->pShape))[pBullet->bLOSIndexX][pBullet->bLOSIndexY] & AtHeight[iCurrCubesAboveLevelZ]) > 0)
							{
								if (pStructure->fFlags & STRUCTURE_PERSON)
								{
									// hit someone?
									if (fIntended)
									{	// gotcha! ... return chance to get through
										iChanceToGetThrough = iChanceToGetThrough * (pBullet->iImpact - pBullet->iImpactReduction) / pBullet->iImpact;
										return( (UINT8) iChanceToGetThrough );
									}
									else
									{
										gubLocalStructureNumTimesHit[iStructureLoop]++;
									}
								}
								else if (pStructure->fFlags & STRUCTURE_WALLNWINDOW && pBullet->qCurrZ >= qWindowBottomHeight && pBullet->qCurrZ <= qWindowTopHeight)
								{
									fResolveHit = ResolveHitOnWall( pStructure, iGridNo, pBullet->bLOSIndexX, pBullet->bLOSIndexY, pBullet->ddHorizAngle );

									if (fResolveHit)
									{
										// the bullet would keep on going!  unless we're considering a knife...
										if (pBullet->usFlags & BULLET_FLAG_KNIFE)
										{
											gubLocalStructureNumTimesHit[iStructureLoop]++;
										}
									}
								}
								else if (pBullet->iLoop > CLOSE_TO_FIRER ||
									(pStructure->fFlags & ALWAYS_CONSIDER_HIT))
								{
									if (pStructure->fFlags & STRUCTURE_WALLSTUFF)
									{
										// possibly shooting at corner in which case we should let it pass
										fResolveHit = ResolveHitOnWall( pStructure, iGridNo, pBullet->bLOSIndexX, pBullet->bLOSIndexY, pBullet->ddHorizAngle );
									}
									else
									{
										fResolveHit = TRUE;
									}
									if (fResolveHit)
									{
										gubLocalStructureNumTimesHit[iStructureLoop]++;
									}
								}
							}
						}
					}
				}

				// got past everything; go to next LOS location within
				// tile, horizontally or vertically
				bOldLOSIndexX = pBullet->bLOSIndexX;
				bOldLOSIndexY = pBullet->bLOSIndexY;
				iOldCubesZ = pBullet->iCurrCubesZ;
				do
				{
					pBullet->qCurrX += pBullet->qIncrX;
					pBullet->qCurrY += pBullet->qIncrY;
					if (pRoofStructure)
					{
						qLastZ = pBullet->qCurrZ;
						pBullet->qCurrZ += pBullet->qIncrZ;
						if ( (qLastZ > qWallHeight && pBullet->qCurrZ < qWallHeight) || (qLastZ < qWallHeight && pBullet->qCurrZ > qWallHeight))
						{
							// hit roof!
							//pBullet->iImpactReduction += CTGTHandleBulletStructureInteraction( pBullet, pRoofStructure );
							//if (pBullet->iImpactReduction >= pBullet->iImpact)
							{
								return( 0 );
							}

						}
					}
					else
					{
						pBullet->qCurrZ += pBullet->qIncrZ;
					}
					pBullet->iLoop++;
					pBullet->bLOSIndexX = CONVERT_WITHINTILE_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrX ) % CELL_X_SIZE );
					pBullet->bLOSIndexY = CONVERT_WITHINTILE_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrY ) % CELL_Y_SIZE );
					pBullet->iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrZ ) );
				}
				while( (pBullet->bLOSIndexX == bOldLOSIndexX) && (pBullet->bLOSIndexY == bOldLOSIndexY) && (pBullet->iCurrCubesZ == iOldCubesZ));

				SLOGD(ST::format("CTGT at {} {} {} after moving in nonempty tile from {} {} {}",
					pBullet->bLOSIndexX, pBullet->bLOSIndexY, pBullet->iCurrCubesZ,
					bOldLOSIndexX, bOldLOSIndexY, iOldCubesZ));
				pBullet->iCurrTileX = FIXEDPT_TO_INT32( pBullet->qCurrX ) / CELL_X_SIZE;
				pBullet->iCurrTileY = FIXEDPT_TO_INT32( pBullet->qCurrY ) / CELL_Y_SIZE;
			}
		} while( (pBullet->iLoop < pBullet->iDistanceLimit) && (pBullet->iCurrTileX == iOldTileX) && (pBullet->iCurrTileY == iOldTileY));

		if ( pBullet->iCurrTileX < 0 || pBullet->iCurrTileX >= WORLD_COLS || pBullet->iCurrTileY < 0 || pBullet->iCurrTileY >= WORLD_ROWS )
		{
			return( 0 );
		}

		pBullet->sGridNo = MAPROWCOLTOPOS( pBullet->iCurrTileY , pBullet->iCurrTileX );

		if (pBullet->iLoop > pBullet->iRange * 2)
		{
			// beyond max effective range, bullet starts to drop!
			// since we're doing an increment based on distance, not time, the
			// decrement is scaled down depending on how fast the bullet is (effective range)
			pBullet->qIncrZ -= INT32_TO_FIXEDPT( 100 ) / (pBullet->iRange * 2);
		}

		// end of the tile...
		if (iNumLocalStructures > 0)
		{
			for ( iStructureLoop = 0; iStructureLoop < iNumLocalStructures; iStructureLoop++)
			{
				// Calculate the total impact based on the number of points in the structure that were hit
				if (gubLocalStructureNumTimesHit[iStructureLoop] > 0)
				{
					iTotalStructureImpact = CTGTHandleBulletStructureInteraction( pBullet, gpLocalStructure[iStructureLoop] ) * gubLocalStructureNumTimesHit[iStructureLoop];

					// reduce the impact reduction of a structure tile to that of the bullet, since it can't do MORE than stop it.
					iTotalStructureImpact = __min( iTotalStructureImpact, pBullet->iImpact );

					// add to "impact reduction" based on strength of structure weighted by probability of hitting it
					pBullet->iImpactReduction += (iTotalStructureImpact * guiLocalStructureCTH[iStructureLoop]) / 100;
				}
			}
			if (pBullet->iImpactReduction >= pBullet->iImpact)
			{
				return( 0 );
			}
		}
	} while( pBullet->iLoop < pBullet->iDistanceLimit );
	// unless the distance is integral, after the loop there will be a
	// fractional amount of distance remaining which is unchecked
	// but we shouldn't(?) need to check it because the target is there!

	// try simple chance to get through, ignoring range effects
	iChanceToGetThrough = iChanceToGetThrough * (pBullet->iImpact - pBullet->iImpactReduction) / pBullet->iImpact;

	if (iChanceToGetThrough < 0)
	{
		iChanceToGetThrough = 0;
	}
	return( (UINT8) iChanceToGetThrough );
}


static INT8 ChanceToGetThrough(SOLDIERTYPE* pFirer, GridNo end_pos, FLOAT dEndZ);


static UINT8 SoldierToSoldierChanceToGetThrough(SOLDIERTYPE* const pStartSoldier, const SOLDIERTYPE* const pEndSoldier)
{
	FLOAT dEndZPos;
	BOOLEAN fOk;

	if (pStartSoldier == pEndSoldier)
	{
		return( 0 );
	}
	CHECKF( pStartSoldier );
	CHECKF( pEndSoldier );
	fOk = CalculateSoldierZPos( pEndSoldier, TARGET_POS, &dEndZPos );
	if (!fOk)
	{
		return( FALSE );
	}

	// set startsoldier's target ID ... need an ID stored in case this
	// is the AI calculating cover to a location where he might not be any more
	pStartSoldier->CTGTTarget = pEndSoldier;
	return ChanceToGetThrough(pStartSoldier, pEndSoldier->sGridNo, dEndZPos);
}


UINT8 SoldierToSoldierBodyPartChanceToGetThrough(SOLDIERTYPE* const pStartSoldier, const SOLDIERTYPE* const pEndSoldier, const UINT8 ubAimLocation)
{
	// does like StS-CTGT but with a particular body part in mind
	FLOAT dEndZPos;
	BOOLEAN fOk;
	UINT8 ubPosType;

	if (pStartSoldier == pEndSoldier)
	{
		return( 0 );
	}
	CHECKF( pStartSoldier );
	CHECKF( pEndSoldier );
	switch( ubAimLocation )
	{
		case AIM_SHOT_HEAD:
			ubPosType = HEAD_TARGET_POS;
			break;
		case AIM_SHOT_TORSO:
			ubPosType = TORSO_TARGET_POS;
			break;
		case AIM_SHOT_LEGS:
			ubPosType = LEGS_TARGET_POS;
			break;
		default:
			ubPosType = TARGET_POS;
			break;
	}

	fOk = CalculateSoldierZPos( pEndSoldier, ubPosType, &dEndZPos );
	if (!fOk)
	{
		return( FALSE );
	}

	// set startsoldier's target ID ... need an ID stored in case this
	// is the AI calculating cover to a location where he might not be any more
	pStartSoldier->CTGTTarget = pEndSoldier;
	return ChanceToGetThrough(pStartSoldier, pEndSoldier->sGridNo, dEndZPos);
}


UINT8 SoldierToLocationChanceToGetThrough(SOLDIERTYPE* const pStartSoldier, const INT16 sGridNo, const INT8 bLevel, const INT8 bCubeLevel, const SOLDIERTYPE* const target)
{
	FLOAT dEndZPos;

	if (pStartSoldier->sGridNo == sGridNo)
	{
		return( 0 );
	}
	CHECKF( pStartSoldier );

	const SOLDIERTYPE* const pEndSoldier = WhoIsThere2(sGridNo, bLevel);
	if (pEndSoldier != NULL)
	{
		return( SoldierToSoldierChanceToGetThrough( pStartSoldier, pEndSoldier ) );
	}
	else
	{
		if (bCubeLevel)
		{
			// fire at the centre of the cube specified
			dEndZPos = ( ( (FLOAT) (bCubeLevel + bLevel * PROFILE_Z_SIZE) ) - 0.5f) * HEIGHT_UNITS_PER_INDEX;
		}
		else
		{
			INT8 const bStructHeight = GetStructureTargetHeight(sGridNo, bLevel == 1);
			if (bStructHeight > 0)
			{
				// fire at the centre of the cube of the tallest structure
				dEndZPos = ((FLOAT) (bStructHeight + bLevel * PROFILE_Z_SIZE) - 0.5f) * HEIGHT_UNITS_PER_INDEX;
			}
			else
			{
				// fire at 1 unit above the level of the ground
				dEndZPos = (FLOAT) ((bLevel * PROFILE_Z_SIZE) * HEIGHT_UNITS_PER_INDEX + 1);
			}
		}

		dEndZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[sGridNo].sHeight );

		// set startsoldier's target ID ... need an ID stored in case this
		// is the AI calculating cover to a location where he might not be any more
		pStartSoldier->CTGTTarget = target;
		return ChanceToGetThrough(pStartSoldier, sGridNo, dEndZPos);
	}
}


UINT8 AISoldierToSoldierChanceToGetThrough(SOLDIERTYPE* const pStartSoldier, const SOLDIERTYPE* const pEndSoldier)
{
	// Like a standard CTGT algorithm BUT fakes the start soldier at standing height
	FLOAT dEndZPos;
	BOOLEAN fOk;
	UINT8 ubChance;
	UINT16 usTrueState;

	if (pStartSoldier == pEndSoldier)
	{
		return( 0 );
	}
	CHECKF( pStartSoldier );
	CHECKF( pEndSoldier );
	fOk = CalculateSoldierZPos( pEndSoldier, TARGET_POS, &dEndZPos );
	if (!fOk)
	{
		return( FALSE );
	}
	usTrueState = pStartSoldier->usAnimState;
	pStartSoldier->usAnimState = STANDING;

	// set startsoldier's target ID ... need an ID stored in case this
	// is the AI calculating cover to a location where he might not be any more
	pStartSoldier->CTGTTarget = NULL;

	ubChance = ChanceToGetThrough(pStartSoldier, pEndSoldier->sGridNo, dEndZPos);
	pStartSoldier->usAnimState = usTrueState;
	return( ubChance );
}

UINT8 AISoldierToLocationChanceToGetThrough( SOLDIERTYPE * pStartSoldier, INT16 sGridNo, INT8 bLevel, INT8 bCubeLevel )
{
	FLOAT dEndZPos;

	UINT16 usTrueState;
	UINT8 ubChance;

	if (pStartSoldier->sGridNo == sGridNo)
	{
		return( 0 );
	}
	CHECKF( pStartSoldier );

	const SOLDIERTYPE* const pEndSoldier = WhoIsThere2(sGridNo, bLevel);
	if (pEndSoldier != NULL)
	{
		return( AISoldierToSoldierChanceToGetThrough( pStartSoldier, pEndSoldier ) );
	}
	else
	{
		if (bCubeLevel)
		{
			// fire at the centre of the cube specified
			dEndZPos = ( (FLOAT) (bCubeLevel + bLevel * PROFILE_Z_SIZE) - 0.5f) * HEIGHT_UNITS_PER_INDEX;
		}
		else
		{
			INT8 const bStructHeight = GetStructureTargetHeight(sGridNo, bLevel == 1);
			if (bStructHeight > 0)
			{
				// fire at the centre of the cube of the tallest structure
				dEndZPos = ((FLOAT) (bStructHeight + bLevel * PROFILE_Z_SIZE) - 0.5f) * HEIGHT_UNITS_PER_INDEX;
			}
			else
			{
				// fire at 1 unit above the level of the ground
				dEndZPos = (FLOAT) ((bLevel * PROFILE_Z_SIZE) * HEIGHT_UNITS_PER_INDEX + 1);
			}
		}

		dEndZPos += CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[sGridNo].sHeight );

		// set startsoldier's target ID ... need an ID stored in case this
		// is the AI calculating cover to a location where he might not be any more
		pStartSoldier->CTGTTarget = NULL;

		usTrueState = pStartSoldier->usAnimState;
		pStartSoldier->usAnimState = STANDING;

		ubChance = ChanceToGetThrough(pStartSoldier, sGridNo, dEndZPos);

		pStartSoldier->usAnimState = usTrueState;

		return( ubChance );
	}
}


static void CalculateFiringIncrements(DOUBLE ddHorizAngle, DOUBLE ddVerticAngle, DOUBLE dd2DDistance, BULLET* pBullet, DOUBLE* pddNewHorizAngle, DOUBLE* pddNewVerticAngle)
{
	INT32 iMissedBy = - pBullet->sHitBy;
	DOUBLE ddVerticPercentOfMiss;
	DOUBLE ddAbsVerticAngle;
	DOUBLE ddScrewupAdjustmentLimit;
	UINT32 uiChanceOfMissAbove;
	DOUBLE ddMinimumMiss;
	DOUBLE ddMaximumMiss;
	DOUBLE ddAmountOfMiss;

	if (iMissedBy > 0)
	{
		ddVerticPercentOfMiss = PreRandom( 50 );

		ddAbsVerticAngle = ddVerticAngle;
		if (ddAbsVerticAngle < 0)
		{
			ddAbsVerticAngle *= -1.0;
		}

		// chance of shooting over target is 60 for horizontal shots, up to 80% for shots at 22.5 degrees,
		// and then down again to 50% for shots at 45+%.
		if (ddAbsVerticAngle < DEGREES_22_5)
		{
			uiChanceOfMissAbove = 60 + (INT32) (20 * (ddAbsVerticAngle) / DEGREES_22_5);
		}
		else if (ddAbsVerticAngle < DEGREES_45)
		{
			uiChanceOfMissAbove = 80 - (INT32) (30.0 * (ddAbsVerticAngle - DEGREES_22_5) / DEGREES_22_5);
		}
		else
		{
			uiChanceOfMissAbove = 50;
		}
		// figure out change in horizontal and vertical angle due to shooter screwup
		// the more the screwup, the greater the angle;
		// for the look of things, the further away, reduce the angle a bit.
		ddScrewupAdjustmentLimit = (dd2DDistance / CELL_X_SIZE) / 200;
		if (ddScrewupAdjustmentLimit > MAX_AIMING_SCREWUP / 2)
		{
			ddScrewupAdjustmentLimit = MAX_AIMING_SCREWUP / 2;
		}
		ddMaximumMiss = MAX_AIMING_SCREWUP - ddScrewupAdjustmentLimit;

		// Want to make sure that not too many misses actually hit the target after all
		// to miss a target at 1 tile is about 30 degrees off, at 5 tiles, 6 degrees off
		// at 15 tiles, 2 degrees off.  Thus 30 degrees divided by the # of tiles distance.
		ddMinimumMiss = MIN_AIMING_SCREWUP / (dd2DDistance / CELL_X_SIZE );

		if (ddMinimumMiss > ddMaximumMiss)
		{
			ddMinimumMiss = ddMaximumMiss;
		}

		ddAmountOfMiss = ( (ddMaximumMiss - ddMinimumMiss) * ((DOUBLE) iMissedBy)) / 100.0 + ddMinimumMiss;

		// miss to the left or right
		if (PreRandom( 2 ) )
		{
			ddHorizAngle += ddAmountOfMiss * (100.0 - ddVerticPercentOfMiss) / 100.0;
		}
		else
		{
			ddHorizAngle -= ddAmountOfMiss * (100.0 - ddVerticPercentOfMiss) / 100.0;
		}

		// miss up or down
		if (PreRandom( 100 ) < uiChanceOfMissAbove)
		{
			ddVerticAngle += ddAmountOfMiss * ddVerticPercentOfMiss / 100.0;
		}
		else
		{
			ddVerticAngle -= ddAmountOfMiss * ddVerticPercentOfMiss / 100.0;
		}
	}

	*pddNewHorizAngle = ddHorizAngle;
	*pddNewVerticAngle = ddVerticAngle;

	pBullet->qIncrX = FloatToFixed( (FLOAT) cos( ddHorizAngle ) );
	pBullet->qIncrY = FloatToFixed( (FLOAT) sin( ddHorizAngle ) );

	// this is the same as multiplying the X and Y increments by the projection of the line in
	// 3-space onto the horizontal plane, without reducing the X/Y increments and thus slowing
	// the LOS code
	pBullet->qIncrZ = FloatToFixed( (FLOAT) ( sin( ddVerticAngle ) / sin( (PI/2) - ddVerticAngle ) * 2.56 ) );
}


static INT8 FireBullet(BULLET* pBullet, BOOLEAN fFake)
{
	pBullet->iCurrTileX = FIXEDPT_TO_INT32( pBullet->qCurrX ) / CELL_X_SIZE;
	pBullet->iCurrTileY = FIXEDPT_TO_INT32( pBullet->qCurrY ) / CELL_Y_SIZE;
	pBullet->bLOSIndexX = CONVERT_WITHINTILE_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrX ) % CELL_X_SIZE );
	pBullet->bLOSIndexY = CONVERT_WITHINTILE_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrY ) % CELL_Y_SIZE );
	pBullet->iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrZ ) );
	pBullet->iLoop = 1;
	pBullet->iImpactReduction = 0;
	pBullet->sGridNo = MAPROWCOLTOPOS( pBullet->iCurrTileY, pBullet->iCurrTileX );
	SOLDIERTYPE* const pFirer = pBullet->pFirer;
	if (fFake)
	{
		pBullet->target = pFirer->CTGTTarget;
		return( CalcChanceToGetThrough( pBullet ) );
	}
	else
	{
		pBullet->target = pFirer->target;
		//if ( gGameSettings.fOptions[ TOPTION_HIDE_BULLETS ] )
		//{
		//	pBullet->uiLastUpdate = 0;
		//	pBullet->ubTilesPerUpdate	= 4;
		//}
		//else
		//{
			pBullet->uiLastUpdate = 0;
			pBullet->ubTilesPerUpdate	= 1;
		//}

		// increment shots fired if shooter has a merc profile
		if( ( pFirer->ubProfile != NO_PROFILE ) && ( pFirer->bTeam == 0 ) )
		{
			if (pFirer->target)
			{
				// Do not count shots fired at nothing in particular because it skews hit %. It would always counts as a miss
				if (pFirer->target->bLife > 0) gMercProfiles[pFirer->ubProfile].usShotsFired++;
			}
		}

		if ( GCM->getItem(pFirer->usAttackingWeapon)->getItemClass() == IC_THROWING_KNIFE )
		{
			pBullet->usClockTicksPerUpdate = 30;
		}
		else
		{
			pBullet->usClockTicksPerUpdate = GCM->getWeapon( pFirer->usAttackingWeapon )->ubBulletSpeed / 10;
		}

		HandleBulletSpecialFlags(pBullet);

		MoveBullet(pBullet);

		return( TRUE );
	}
}


INT8 FireBulletGivenTarget(SOLDIERTYPE* const pFirer, const FLOAT dEndX, const FLOAT dEndY, const FLOAT dEndZ, const UINT16 usHandItem, INT16 sHitBy, const BOOLEAN fBuckshot, const BOOLEAN fFake)
{
	// fFake indicates that we should set things up for a call to ChanceToGetThrough
	FLOAT dStartZ;

	FLOAT d2DDistance;
	FLOAT dDeltaX;
	FLOAT dDeltaY;
	FLOAT dDeltaZ;

	FLOAT dStartX;
	FLOAT dStartY;

	DOUBLE ddOrigHorizAngle;
	DOUBLE ddOrigVerticAngle;
	DOUBLE ddHorizAngle;
	DOUBLE ddVerticAngle;
	DOUBLE ddAdjustedHorizAngle;
	DOUBLE ddAdjustedVerticAngle;
	DOUBLE ddDummyHorizAngle;
	DOUBLE ddDummyVerticAngle;

	INT32 iDistance;

	UINT8 ubLoop;
	UINT8 ubShots;
	UINT8 ubImpact;
	INT8 bCTGT;
	UINT8 ubSpreadIndex = 0;
	UINT16 usBulletFlags = 0;

	CalculateSoldierZPos( pFirer, FIRING_POS, &dStartZ );

	dStartX = (FLOAT) CenterX( pFirer->sGridNo );
	dStartY = (FLOAT) CenterY( pFirer->sGridNo );

	dDeltaX = dEndX - dStartX;
	dDeltaY = dEndY - dStartY;
	dDeltaZ = dEndZ - dStartZ;

	d2DDistance = Distance2D( dDeltaX, dDeltaY );
	iDistance = (INT32) d2DDistance;

	if ( d2DDistance != iDistance )
	{
		iDistance += 1;
		d2DDistance = (FLOAT) ( iDistance);
	}

	ddOrigHorizAngle = atan2( dDeltaY, dDeltaX );
	ddOrigVerticAngle = atan2( dDeltaZ, (d2DDistance * 2.56f) );

	ubShots = 1;

	// Check if we have spit as a weapon!
	if (GCM->getWeapon( usHandItem )->calibre->monsterWeapon)
	{
		usBulletFlags |= BULLET_FLAG_CREATURE_SPIT;
	}
	else if (GCM->getItem(usHandItem)->getItemClass() == IC_THROWING_KNIFE)
	{
		usBulletFlags |= BULLET_FLAG_KNIFE;
		if (GCM->getItem(usHandItem)->getItemIndex() == BLOODY_THROWING_KNIFE)
			usBulletFlags |= BULLET_FLAG_BLOODY;
	}
	else if (usHandItem == ROCKET_LAUNCHER)
	{
		usBulletFlags |= BULLET_FLAG_MISSILE;
	}
	else if (usHandItem == TANK_CANNON)
	{
		usBulletFlags |= BULLET_FLAG_TANK_CANNON;
	}
	else if (usHandItem == ROCKET_RIFLE || usHandItem == AUTO_ROCKET_RIFLE)
	{
		usBulletFlags |= BULLET_FLAG_SMALL_MISSILE;
	}
	else if (usHandItem == FLAMETHROWER)
	{
		usBulletFlags |= BULLET_FLAG_FLAME;
		ubSpreadIndex = 2;
	}

	ubImpact = GCM->getWeapon(usHandItem)->ubImpact;
	//if (!fFake)
	{
		if (fBuckshot)
		{
			// shotgun pellets fire 9 bullets doing 1/4 damage each
			if (!fFake)
			{
				ubShots = BUCKSHOT_SHOTS;
				// but you can't really aim the damn things very well!
				if (sHitBy > 0)
				{
					sHitBy = sHitBy / 2;
				}
				if (FindAttachment( &(pFirer->inv[pFirer->ubAttackingHand]), DUCKBILL ) != NO_SLOT)
				{
					ubSpreadIndex = 1;
				}
				if (pFirer->target != NULL) pFirer->target->bNumPelletsHitBy = 0;
				usBulletFlags |= BULLET_FLAG_BUCKSHOT;
			}
			ubImpact = AMMO_DAMAGE_ADJUSTMENT_BUCKSHOT( ubImpact );
		}
	}

	// GET BULLET
	for (ubLoop = 0; ubLoop < ubShots; ubLoop++)
	{
		BULLET* const pBullet = CreateBullet(pFirer, fFake, usBulletFlags);
		if (pBullet == NULL)
		{
			SLOGW("Failed to create bullet");
			return FALSE;
		}
		pBullet->sHitBy	= sHitBy;

		if (dStartZ < WALL_HEIGHT_UNITS)
		{
			if (dEndZ > WALL_HEIGHT_UNITS)
			{
				pBullet->fCheckForRoof = TRUE;
			}
			else
			{
				pBullet->fCheckForRoof = FALSE;
			}
		}
		else // dStartZ >= WALL_HEIGHT_UNITS; presumably >
		{
			if (dEndZ < WALL_HEIGHT_UNITS)
			{
				pBullet->fCheckForRoof = TRUE;
			}
			else
			{
				pBullet->fCheckForRoof = FALSE;
			}
		}

		if ( ubLoop == 0 )
		{
			ddHorizAngle = ddOrigHorizAngle;
			ddVerticAngle = ddOrigVerticAngle;

			// first bullet, roll to hit...
			if ( sHitBy >= 0 )
			{
				// calculate by hand (well, without angles) to match LOS
				pBullet->qIncrX = FloatToFixed( dDeltaX / (FLOAT)iDistance );
				pBullet->qIncrY = FloatToFixed( dDeltaY / (FLOAT)iDistance );
				pBullet->qIncrZ = FloatToFixed( dDeltaZ / (FLOAT)iDistance );
				ddAdjustedHorizAngle = ddHorizAngle;
				ddAdjustedVerticAngle = ddVerticAngle;
			}
			else
			{
				CalculateFiringIncrements( ddHorizAngle, ddVerticAngle, d2DDistance, pBullet, &ddAdjustedHorizAngle, &ddAdjustedVerticAngle );
			}
		}
		else
		{
			// temporarily set bullet's sHitBy value to 0 to get unadjusted angles
			pBullet->sHitBy = 0;

			ddHorizAngle = ddAdjustedHorizAngle + ddShotgunSpread[ubSpreadIndex][ubLoop][0];
			ddVerticAngle = ddAdjustedVerticAngle + ddShotgunSpread[ubSpreadIndex][ubLoop][1];

			CalculateFiringIncrements( ddHorizAngle, ddVerticAngle, d2DDistance, pBullet, &ddDummyHorizAngle, &ddDummyVerticAngle );
			pBullet->sHitBy = sHitBy;
		}


		pBullet->ddHorizAngle = ddHorizAngle;

		if (ubLoop == 0 && pFirer->bDoBurst < 2)
		{
			pBullet->fAimed = TRUE;
		}
		else
		{
			// buckshot pellets after the first can hit friendlies even at close range
			pBullet->fAimed = FALSE;
		}

		if ( pBullet->usFlags & BULLET_FLAG_KNIFE )
		{
			pBullet->ubItemStatus = pFirer->inv[pFirer->ubAttackingHand].bStatus[0];
		}

		// apply increments for first move

		pBullet->qCurrX = FloatToFixed( dStartX ) + pBullet->qIncrX;
		pBullet->qCurrY = FloatToFixed( dStartY ) + pBullet->qIncrY;
		pBullet->qCurrZ = FloatToFixed( dStartZ ) + pBullet->qIncrZ;

		// NB we can only apply correction for leftovers if the bullet is going to hit
		// because otherwise the increments are not right for the calculations!
		if ( pBullet->sHitBy >= 0 )
		{
			pBullet->qCurrX += ( FloatToFixed( dDeltaX ) - pBullet->qIncrX * iDistance ) / 2;
			pBullet->qCurrY += ( FloatToFixed( dDeltaY ) - pBullet->qIncrY * iDistance ) / 2;
			pBullet->qCurrZ += ( FloatToFixed( dDeltaZ ) - pBullet->qIncrZ * iDistance ) / 2;
		}

		pBullet->iImpact = ubImpact;

		pBullet->iRange = GunRange(pFirer->inv[pFirer->ubAttackingHand]);
		pBullet->sTargetGridNo = ((INT32)dEndX) / CELL_X_SIZE + ((INT32)dEndY) / CELL_Y_SIZE * WORLD_COLS;

		pBullet->bStartCubesAboveLevelZ = (INT8) CONVERT_HEIGHTUNITS_TO_INDEX( (INT32)dStartZ - CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ pFirer->sGridNo ].sHeight ) );
		pBullet->bEndCubesAboveLevelZ = (INT8) CONVERT_HEIGHTUNITS_TO_INDEX( (INT32)dEndZ - CONVERT_PIXELS_TO_HEIGHTUNITS( gpWorldLevelData[ pBullet->sTargetGridNo ].sHeight ) );

		// this distance limit only applies in a "hard" sense to fake bullets for chance-to-get-through,
		// but is used for determining structure hits by the regular code
		pBullet->iDistanceLimit = iDistance;
		if (fFake)
		{
			bCTGT = FireBullet(pBullet, TRUE);
			RemoveBullet(pBullet);
			return( bCTGT );
		}
		else
		{
			FireBullet(pBullet, FALSE);
		}
	}

	return( TRUE );
}


static INT8 ChanceToGetThrough(SOLDIERTYPE* const pFirer, const GridNo end_pos, const FLOAT dEndZ)
{
	UINT16  weapon = pFirer->usAttackingWeapon;
	BOOLEAN buck_shot;
	if (GCM->getItem(weapon)->getItemClass() == IC_GUN ||
		GCM->getItem(weapon)->getItemClass() == IC_THROWING_KNIFE)
	{
		// if shotgun, shotgun would have to be in main hand
		buck_shot = pFirer->inv[HANDPOS].usItem        == weapon &&
				pFirer->inv[HANDPOS].ubGunAmmoType == AMMO_BUCKSHOT;
	}
	else
	{
		// fake it
		weapon    = GLOCK_17;
		buck_shot = FALSE;
	}

	INT16 end_x;
	INT16 end_y;
	ConvertGridNoToCenterCellXY(end_pos, &end_x, &end_y);
	return FireBulletGivenTarget(pFirer, end_x, end_y, dEndZ, weapon, 0, buck_shot, TRUE);
}


void MoveBullet(BULLET* const pBullet)
{
	FIXEDPT qLandHeight;
	INT32 iCurrAboveLevelZ;
	INT32 iCurrCubesAboveLevelZ;
	INT16 sDesiredLevel;

	INT32 iOldTileX;
	INT32 iOldTileY;
	INT32 iOldCubesZ;

	MAP_ELEMENT * pMapElement;
	STRUCTURE * pStructure;
	STRUCTURE * pRoofStructure = NULL;

	FIXEDPT qLastZ;

	BOOLEAN fIntended;
	BOOLEAN fStopped;
	INT8 bOldLOSIndexX;
	INT8 bOldLOSIndexY;

	UINT32 uiTileInc = 0;
	UINT32 uiTime;

	INT8 bDir;
	INT32 iGridNo, iAdjGridNo;

	INT32 iRemainingImpact;

	FIXEDPT qDistToTravelX;
	FIXEDPT qDistToTravelY;
	INT32 iStepsToTravelX;
	INT32 iStepsToTravelY;
	INT32 iStepsToTravel;

	INT32 iNumLocalStructures;
	INT32 iStructureLoop;
	UINT32 uiChanceOfHit;

	BOOLEAN fResolveHit;

	INT32 i;
	BOOLEAN fGoingOver = FALSE;
	BOOLEAN fHitStructure;

	FIXEDPT qWallHeight;
	FIXEDPT qWindowBottomHeight;
	FIXEDPT qWindowTopHeight;

	// CHECK MIN TIME ELAPSED
	uiTime = GetJA2Clock( );

	if ( ( uiTime - pBullet->uiLastUpdate ) < pBullet->usClockTicksPerUpdate )
	{
		return;
	}

	pBullet->uiLastUpdate = uiTime;

	do
	{
		// check a particular tile
		// retrieve values from world for this particular tile
		iGridNo = pBullet->iCurrTileX + pBullet->iCurrTileY * WORLD_COLS;
		if (!GridNoOnVisibleWorldTile( (INT16) iGridNo ) || (pBullet->iCurrCubesZ > PROFILE_Z_SIZE * 2 && FIXEDPT_TO_INT32( pBullet->qIncrZ ) > 0 ) )
		{
			// bullet outside of world!
			// NB remove bullet only flags a bullet for deletion; we still have access to the
			// information in the structure
			RemoveBullet(pBullet);
			ShotMiss(pBullet);
			return;
		}

		pMapElement = &(gpWorldLevelData[ iGridNo ]);
		qLandHeight = INT32_TO_FIXEDPT( CONVERT_PIXELS_TO_HEIGHTUNITS( pMapElement->sHeight ) );
		qWallHeight = gqStandardWallHeight + qLandHeight;
		qWindowBottomHeight = gqStandardWindowBottomHeight + qLandHeight;
		qWindowTopHeight = gqStandardWindowTopHeight + qLandHeight;


		// calculate which level bullet is on for suppression and close call purposes
		// figure out the LOS cube level of the current point
		iCurrCubesAboveLevelZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrZ - qLandHeight) );
		// figure out the level
		if (iCurrCubesAboveLevelZ < STRUCTURE_ON_GROUND_MAX)
		{
			// check objects on the ground
			sDesiredLevel = 0;
		}
		else
		{
			// check objects on roofs
			sDesiredLevel = 1;
		}

		// assemble list of structures we might hit!
		iNumLocalStructures = 0;
		pStructure = pMapElement->pStructureHead;
		// calculate chance of hitting each structure
		uiChanceOfHit = ChanceOfBulletHittingStructure( pBullet->iLoop, pBullet->iDistanceLimit, pBullet->sHitBy );
		if (iGridNo == (INT32) pBullet->sTargetGridNo)
		{
			fIntended = TRUE;
			// if in the same tile as our destination, we WANT to hit the structure!
			if ( fIntended )
			{
				uiChanceOfHit = 100;
			}
		}
		else
		{
			fIntended = FALSE;
		}

		while( pStructure )
		{
			if (pStructure->fFlags & ALWAYS_CONSIDER_HIT)
			{
				// ALWAYS add walls
				// fence is special
				if ( pStructure->fFlags & STRUCTURE_ANYFENCE )
				{
					// If the density of the fence is less than 100%, or this is the top of the fence, then roll the dice
					// NB cubes are 0 based, heights 1 based
					if ( pStructure->pDBStructureRef->pDBStructure->ubDensity < 100 )
					{
						// requires roll
						if ( PreRandom( 100 ) < uiChanceOfHit )
						{
							gpLocalStructure[iNumLocalStructures] = pStructure;
							gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
							iNumLocalStructures++;
						}
					}
					else if ( (pBullet->iLoop <= CLOSE_TO_FIRER) && (iCurrCubesAboveLevelZ <= pBullet->bStartCubesAboveLevelZ) && (pBullet->bEndCubesAboveLevelZ >= iCurrCubesAboveLevelZ) && iCurrCubesAboveLevelZ == (StructureHeight( pStructure ) - 1) )
					{
						// near firer and at top of structure and at same level as bullet's start
						// requires roll
						if ( PreRandom( 100 ) < uiChanceOfHit )
						{
							gpLocalStructure[iNumLocalStructures] = pStructure;
							gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
							iNumLocalStructures++;
						}
					}
					else if ( (pBullet->iDistanceLimit - pBullet->iLoop <= CLOSE_TO_FIRER) && (iCurrCubesAboveLevelZ <= pBullet->bEndCubesAboveLevelZ) && iCurrCubesAboveLevelZ == (StructureHeight( pStructure ) - 1) )
					{
						// near target and at top of structure and at same level as bullet's end
						// requires roll
						if ( PreRandom( 100 ) < uiChanceOfHit )
						{
							gpLocalStructure[iNumLocalStructures] = pStructure;
							gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
							iNumLocalStructures++;
						}
					}
					else
					{
						// always add
						gpLocalStructure[iNumLocalStructures] = pStructure;
						gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
						iNumLocalStructures++;
					}

					/*
					if ( !( (pStructure->pDBStructureRef->pDBStructure->ubDensity < 100 || iCurrCubesAboveLevelZ == (StructureHeight( pStructure ) - 1) )	) && (PreRandom( 100 ) >= uiChanceOfHit) )
					{
						gpLocalStructure[iNumLocalStructures] = pStructure;
						gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
						iNumLocalStructures++;
					}*/
				}
				else
				{
					gpLocalStructure[iNumLocalStructures] = pStructure;
					gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
					iNumLocalStructures++;
				}
			}
			else if (pStructure->fFlags & STRUCTURE_ROOF)
			{
				// only consider roofs if the flag is set; don't add them to the array since they
				// are a special case
				if (pBullet->fCheckForRoof)
				{
					pRoofStructure = pStructure;

					qLastZ = pBullet->qCurrZ - pBullet->qIncrZ;

					// if just on going to next tile we cross boundary, then roof stops bullet here!
					if ((qLastZ > qWallHeight && pBullet->qCurrZ <= qWallHeight) ||
						(qLastZ < qWallHeight && pBullet->qCurrZ >= qWallHeight))
					{
						// hit a roof
						StopBullet(pBullet);
						BulletHitStructure(pBullet, 0, 0, TRUE);
						return;
					}

				}
			}
			else if (pStructure->fFlags & STRUCTURE_PERSON)
			{
				SOLDIERTYPE& tgt = GetMan(pStructure->usStructureID);
				if (&tgt != pBullet->pFirer)
				{
					// in actually moving the bullet, we consider only count friends as targets if the bullet is unaimed
					// (buckshot), if they are the intended target, or beyond the range of automatic friendly fire hits
					// OR a 1 in 30 chance occurs


					// ignore *intervening* target if not visible; PCs are always visible so AI will never skip them on that
					// basis
					if (fIntended)
					{
						// could hit this person!
						gpLocalStructure[iNumLocalStructures] = pStructure;
						iNumLocalStructures++;
					}
					else if ( pBullet->pFirer->uiStatusFlags & SOLDIER_MONSTER )
					{
						// monsters firing will always accidentally hit people but never accidentally hit each other.
						if (!(tgt.uiStatusFlags & SOLDIER_MONSTER))
						{
							gpLocalStructure[iNumLocalStructures] = pStructure;
							iNumLocalStructures++;
						}
					}
					else if (tgt.bVisible == TRUE &&
						gAnimControl[tgt.usAnimState].ubEndHeight == ANIM_STAND &&
						((pBullet->fAimed && pBullet->iLoop > MIN_DIST_FOR_HIT_FRIENDS) ||
						(!pBullet->fAimed && pBullet->iLoop > MIN_DIST_FOR_HIT_FRIENDS_UNAIMED) ||
						PreRandom( 100 ) < MIN_CHANCE_TO_ACCIDENTALLY_HIT_SOMEONE))
					{
						// could hit this person!
						gpLocalStructure[iNumLocalStructures] = pStructure;
						iNumLocalStructures++;
					}

					// this might be a close call
					if (tgt.bTeam == OUR_TEAM && pBullet->pFirer->bTeam != OUR_TEAM && sDesiredLevel == tgt.bLevel)
					{
						tgt.fCloseCall = TRUE;
					}

					if (IS_MERC_BODY_TYPE(&tgt))
					{
						// apply suppression, regardless of friendly or enemy
						// except if friendly, not within a few tiles of shooter
						if (tgt.bSide != pBullet->pFirer->bSide || pBullet->iLoop > MIN_DIST_FOR_HIT_FRIENDS)
						{
							// buckshot has only a 1 in 2 chance of applying a suppression point
							if ( !(pBullet->usFlags & BULLET_FLAG_BUCKSHOT) || Random( 2 ) )
							{
								// bullet goes whizzing by this guy!
								switch (gAnimControl[tgt.usAnimState].ubEndHeight)
								{
									case ANIM_PRONE:
										// two 1/4 chances of avoiding suppression pt - one below
										if (PreRandom( 4 ) == 0)
										{
											break;
										}
										// else fall through
									case ANIM_CROUCH:
										// 1/4 chance of avoiding suppression pt
										if (PreRandom( 4 ) == 0)
										{
											break;
										}
										// else fall through
									default:
										tgt.ubSuppressionPoints++;
										tgt.suppressor = pBullet->pFirer;
										break;
								}
							}
						}
					}
				}

			}
			else if ( pStructure->fFlags & STRUCTURE_CORPSE )
			{
				if (iGridNo == (INT32) pBullet->sTargetGridNo ||
					(pStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles >= 10))
				{
					// could hit this corpse!
					// but ignore if someone is here
					if ( FindStructure( (INT16) iGridNo, STRUCTURE_PERSON ) == NULL )
					{
						gpLocalStructure[iNumLocalStructures] = pStructure;
						iNumLocalStructures++;
					}
				}
			}
			else
			{
				if (pBullet->iLoop > CLOSE_TO_FIRER || ( fIntended ) )
				{
					// calculate chance of hitting structure
					if (PreRandom( 100 ) < uiChanceOfHit)
					{
						// could hit it
						gpLocalStructure[iNumLocalStructures] = pStructure;
						gubLocalStructureNumTimesHit[iNumLocalStructures] = 0;
						iNumLocalStructures++;
					}
				}
			}
			pStructure = pStructure->pNext;
		}

		// check to see if any soldiers are nearby; those soldiers
		// have their near-miss value incremented
		if (pMapElement->ubAdjacentSoldierCnt > 0)
		{
			// cube level now calculated above!
			// figure out the LOS cube level of the current point
			//iCurrCubesAboveLevelZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrZ - qLandHeight) );
			// figure out what level to affect...
			if (iCurrCubesAboveLevelZ < STRUCTURE_ON_ROOF_MAX)
			{
				/*
				if (iCurrCubesAboveLevelZ < STRUCTURE_ON_GROUND_MAX)
				{
					// check objects on the ground
					sDesiredLevel = 0;
				}
				else
				{
					// check objects on roofs
					sDesiredLevel = 1;
				}*/

				for( bDir = 0; bDir < NUM_WORLD_DIRECTIONS; bDir++)
				{
					iAdjGridNo = iGridNo + DirIncrementer[bDir];

					if ( gubWorldMovementCosts[ iAdjGridNo ][ bDir ][ sDesiredLevel ] < TRAVELCOST_BLOCKED)
					{
						SOLDIERTYPE* const pTarget = WhoIsThere2(iAdjGridNo, sDesiredLevel);
						if (pTarget != NULL)
						{
							if ( IS_MERC_BODY_TYPE( pTarget ) && pBullet->pFirer->bSide != pTarget->bSide )
							{
								if ( !(pBullet->usFlags & BULLET_FLAG_BUCKSHOT) || Random( 2 ) )
								{
									// bullet goes whizzing by this guy!
									switch ( gAnimControl[ pTarget->usAnimState ].ubEndHeight )
									{
										case ANIM_PRONE:
											// two 1/4 chances of avoiding suppression pt - one below
											if (PreRandom( 4 ) == 0)
											{
												break;
											}
											// else fall through
										case ANIM_CROUCH:
											// 1/4 chance of avoiding suppression pt
											if (PreRandom( 4 ) == 0)
											{
												break;
											}
											// else fall through
										default:
											pTarget->ubSuppressionPoints++;
											pTarget->suppressor = pBullet->pFirer;
											break;
									}
								}

								/*
								// this could be a close call
								if ( pTarget->bTeam == OUR_TEAM && pBullet->pFirer->bTeam != OUR_TEAM )
								{
									pTarget->fCloseCall = TRUE;
								}*/

							}
						}
					}
				}
			}
		}

		// record old tile location for loop purposes
		iOldTileX = pBullet->iCurrTileX;
		iOldTileY = pBullet->iCurrTileY;

		do
		{
			// check a particular location within the tile

			// check for collision with the ground
			iCurrAboveLevelZ = FIXEDPT_TO_INT32( pBullet->qCurrZ - qLandHeight );
			if (iCurrAboveLevelZ < 0)
			{
				// ground is in the way!
				StopBullet(pBullet);
				BulletHitStructure(pBullet, INVALID_STRUCTURE_ID, 0, TRUE);
				return;
			}
			// check for the existence of structures
			if (iNumLocalStructures == 0 && !pRoofStructure)
			{
				// no structures in this tile, AND THAT INCLUDES ROOFS! :-)
				// new system; figure out how many steps until we cross the next edge
				// and then fast forward that many steps.

				iOldTileX = pBullet->iCurrTileX;
				iOldTileY = pBullet->iCurrTileY;
				iOldCubesZ = pBullet->iCurrCubesZ;

				if (pBullet->qIncrX > 0)
				{
					qDistToTravelX = INT32_TO_FIXEDPT( CELL_X_SIZE ) - (pBullet->qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE ));
					iStepsToTravelX = qDistToTravelX / pBullet->qIncrX;
				}
				else if (pBullet->qIncrX < 0)
				{
					qDistToTravelX = pBullet->qCurrX % INT32_TO_FIXEDPT( CELL_X_SIZE );
					iStepsToTravelX = qDistToTravelX / (-pBullet->qIncrX);
				}
				else
				{
					// make sure we don't consider X a limit :-)
					iStepsToTravelX = 1000000;
				}

				if (pBullet->qIncrY > 0)
				{
					qDistToTravelY = INT32_TO_FIXEDPT( CELL_Y_SIZE ) - (pBullet->qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE ));
					iStepsToTravelY = qDistToTravelY / pBullet->qIncrY;
				}
				else if (pBullet->qIncrY < 0)
				{
					qDistToTravelY = pBullet->qCurrY % INT32_TO_FIXEDPT( CELL_Y_SIZE );
					iStepsToTravelY = qDistToTravelY / (-pBullet->qIncrY);
				}
				else
				{
					// make sure we don't consider Y a limit :-)
					iStepsToTravelY = 1000000;
				}

				// add 1 to the # of steps to travel to go INTO the next tile
				iStepsToTravel = __min( iStepsToTravelX, iStepsToTravelY ) + 1;

				// special coding (compared with other versions above) to deal with
				// bullets hitting the ground
				if (pBullet->qCurrZ + pBullet->qIncrZ * iStepsToTravel < qLandHeight)
				{
					iStepsToTravel = __min( iStepsToTravel, ABS( (pBullet->qCurrZ - qLandHeight) / pBullet->qIncrZ ) );
					pBullet->qCurrX += pBullet->qIncrX * iStepsToTravel;
					pBullet->qCurrY += pBullet->qIncrY * iStepsToTravel;
					pBullet->qCurrZ += pBullet->qIncrZ * iStepsToTravel;

					StopBullet(pBullet);
					BulletHitStructure(pBullet, INVALID_STRUCTURE_ID, 0, TRUE);
					return;
				}

				if (pBullet->usFlags & (BULLET_FLAG_MISSILE | BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_TANK_CANNON |
							BULLET_FLAG_FLAME | BULLET_FLAG_CREATURE_SPIT))
				{
					INT8 bStepsPerMove = STEPS_FOR_BULLET_MOVE_TRAILS;

					if ( pBullet->usFlags & ( BULLET_FLAG_SMALL_MISSILE ) )
					{
						bStepsPerMove = STEPS_FOR_BULLET_MOVE_SMALL_TRAILS;
					}
					else if ( pBullet->usFlags & ( BULLET_FLAG_FLAME ) )
					{
						bStepsPerMove = STEPS_FOR_BULLET_MOVE_FIRE_TRAILS;
					}

					for ( i = 0; i < iStepsToTravel; i++ )
					{
						if ( ( ( pBullet->iLoop + i ) % bStepsPerMove ) == 0 )
						{
							fGoingOver = TRUE;
							break;
						}
					}

					if ( fGoingOver )
					{
						FIXEDPT	qCurrX, qCurrY, qCurrZ;

						qCurrX = pBullet->qCurrX + pBullet->qIncrX * i;
						qCurrY = pBullet->qCurrY + pBullet->qIncrY * i;
						qCurrZ = pBullet->qCurrZ + pBullet->qIncrZ * i;

						AddMissileTrail( pBullet, qCurrX, qCurrY, qCurrZ );
					}
				}

				pBullet->qCurrX += pBullet->qIncrX * iStepsToTravel;
				pBullet->qCurrY += pBullet->qIncrY * iStepsToTravel;
				pBullet->qCurrZ += pBullet->qIncrZ * iStepsToTravel;
				pBullet->iLoop += iStepsToTravel;


				// figure out the new tile location
				pBullet->iCurrTileX = FIXEDPT_TO_TILE_NUM( pBullet->qCurrX );
				pBullet->iCurrTileY = FIXEDPT_TO_TILE_NUM( pBullet->qCurrY );
				pBullet->iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrZ ) );
				pBullet->bLOSIndexX = FIXEDPT_TO_LOS_INDEX( pBullet->qCurrX );
				pBullet->bLOSIndexY = FIXEDPT_TO_LOS_INDEX( pBullet->qCurrY );
			}
			else
			{
				// there are structures in this tile
				iCurrCubesAboveLevelZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrAboveLevelZ );
				// figure out the LOS cube level of the current point

				if (iCurrCubesAboveLevelZ < STRUCTURE_ON_ROOF_MAX)
				{
					if (iCurrCubesAboveLevelZ < STRUCTURE_ON_GROUND_MAX)
					{
						// check objects on the ground
						sDesiredLevel = STRUCTURE_ON_GROUND;
					}
					else
					{
						// check objects on roofs
						sDesiredLevel = STRUCTURE_ON_ROOF;
						iCurrCubesAboveLevelZ -= STRUCTURE_ON_ROOF;
					}
					// check structures for collision
					for ( iStructureLoop = 0; iStructureLoop < iNumLocalStructures; iStructureLoop++)
					{
						pStructure = gpLocalStructure[iStructureLoop];
						if (pStructure && pStructure->sCubeOffset == sDesiredLevel)
						{
							if (((*(pStructure->pShape))[pBullet->bLOSIndexX][pBullet->bLOSIndexY] & AtHeight[iCurrCubesAboveLevelZ]) > 0)
							{
								if (pStructure->fFlags & STRUCTURE_PERSON)
								{
									// hit someone!
									fStopped = BulletHitMerc( pBullet, pStructure, fIntended );
									if (fStopped)
									{
										// remove bullet function now called from within BulletHitMerc, so just quit
										return;
									}
									else
									{
										// set pointer to null so that we don't consider hitting this person again
										gpLocalStructure[iStructureLoop] = NULL;
									}
								}
								else if (pStructure->fFlags & STRUCTURE_WALLNWINDOW && pBullet->qCurrZ >= qWindowBottomHeight && pBullet->qCurrZ <= qWindowTopHeight)
								{
									fResolveHit = ResolveHitOnWall( pStructure, iGridNo, pBullet->bLOSIndexX, pBullet->bLOSIndexY, pBullet->ddHorizAngle );

									if (fResolveHit)
									{

										if (pBullet->usFlags & BULLET_FLAG_KNIFE)
										{
											// knives do get stopped by windows!

											iRemainingImpact = HandleBulletStructureInteraction( pBullet, pStructure, &fHitStructure );
											if ( iRemainingImpact <= 0 )
											{
												// check angle of knife and place on ground appropriately
												OBJECTTYPE Object;
												INT32 iKnifeGridNo;

												// Add item
												if ((pBullet->usFlags & BULLET_FLAG_BLOODY))
													CreateItem(BLOODY_THROWING_KNIFE, (INT8) pBullet->ubItemStatus, &Object);
												else
													CreateItem(THROWING_KNIFE, (INT8) pBullet->ubItemStatus, &Object);

												// by default knife at same tile as window
												iKnifeGridNo = (INT16) iGridNo;

												if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT || pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)
												{
													if ( pBullet->qIncrX > 0)
													{
														// heading east so place knife on west, in same tile
													}
													else
													{
														// place to east of window
														iKnifeGridNo += 1;
													}
												}
												else
												{
													if (pBullet->qIncrY > 0)
													{
														// heading south so place wall to north, in same tile of window
													}
													else
													{
														iKnifeGridNo += WORLD_ROWS;
													}
												}

												if ( sDesiredLevel == STRUCTURE_ON_GROUND )
												{
													AddItemToPool(iKnifeGridNo, &Object, INVISIBLE, 0, 0, 0);
												}
												else
												{
													AddItemToPool(iKnifeGridNo, &Object, INVISIBLE, 0, 1, 0);
												}

												// Make team look for items
												NotifySoldiersToLookforItems( );

												// bullet must end here!
												StopBullet(pBullet);
												BulletHitStructure(pBullet, pStructure->usStructureID, 1, TRUE);
												return;
											}
										}
										else
										{
											BOOLEAN blow_south;
											if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
													pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)
											{
												blow_south = pBullet->qIncrX > 0;
											}
											else
											{
												blow_south = pBullet->qIncrY > 0;
											}
											BulletHitWindow(pBullet, pBullet->iCurrTileX + pBullet->iCurrTileY * WORLD_COLS, pStructure->usStructureID, blow_south);
											LocateBullet(pBullet);
											// have to remove this window from future hit considerations so the deleted structure data can't be referenced!
											gpLocalStructure[iStructureLoop] = NULL;
											// but the bullet keeps on going!!!
										}

									}
								}
								else if ( pBullet->iLoop > CLOSE_TO_FIRER || (pStructure->fFlags & ALWAYS_CONSIDER_HIT) || (pBullet->iLoop > CLOSE_TO_FIRER) || (fIntended) )
								{
									if (pStructure->fFlags & STRUCTURE_WALLSTUFF)
									{
										// possibly shooting at corner in which case we should let it pass
										fResolveHit = ResolveHitOnWall( pStructure, iGridNo, pBullet->bLOSIndexX, pBullet->bLOSIndexY, pBullet->ddHorizAngle );
									}
									else
									{
										fResolveHit = TRUE;
									}

									if (fResolveHit)
									{

										iRemainingImpact = HandleBulletStructureInteraction( pBullet, pStructure, &fHitStructure );
										if (fHitStructure)
										{
											// ATE: NOT if we are a special bullet like a LAW trail...
											if (pStructure->fFlags & STRUCTURE_CORPSE && !( pBullet->usFlags & ( BULLET_FLAG_MISSILE | BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_TANK_CANNON | BULLET_FLAG_FLAME | BULLET_FLAG_CREATURE_SPIT ) ) )
											{
												// ATE: In enemy territory here... ;)
												// Now that we have hit a corpse, make the bugger twich!
												RemoveBullet(pBullet);

												CorpseHit( (INT16)pBullet->sGridNo, pStructure->usStructureID );
												SLOGD("Reducing attacker busy count..., CORPSE HIT");

												FreeUpAttacker(pBullet->pFirer);
												return;
											}
											else if ( iRemainingImpact <= 0 )
											{
												StopBullet(pBullet);
												BulletHitStructure(pBullet, pStructure->usStructureID, 1, TRUE);
												return;
											}
											else if (fHitStructure && (gubLocalStructureNumTimesHit[iStructureLoop] == 0) )
											{
												// play animation to indicate structure being hit
												BulletHitStructure(pBullet, pStructure->usStructureID, 1, FALSE);
												gubLocalStructureNumTimesHit[iStructureLoop] = 1;
											}
										}
									}
								}
							}
						}
					}
				}
				// got past everything; go to next LOS location within
				// tile, horizontally or vertically
				bOldLOSIndexX = pBullet->bLOSIndexX;
				bOldLOSIndexY = pBullet->bLOSIndexY;
				iOldCubesZ = pBullet->iCurrCubesZ;
				do
				{
					pBullet->qCurrX += pBullet->qIncrX;
					pBullet->qCurrY += pBullet->qIncrY;
					if (pRoofStructure)
					{
						qLastZ = pBullet->qCurrZ;
						pBullet->qCurrZ += pBullet->qIncrZ;
						if ( (qLastZ > qWallHeight && pBullet->qCurrZ <= qWallHeight) || (qLastZ < qWallHeight && pBullet->qCurrZ >= qWallHeight))
						{
							// generate roof-hitting event
							// always stop with roofs

							if ( 1 /*HandleBulletStructureInteraction( pBullet, pRoofStructure, &fHitStructure ) <= 0 */)
							{
								StopBullet(pBullet);
								BulletHitStructure(pBullet, 0, 0, TRUE);
								return;
							}
							/*
							else
							{
								// ATE: Found this: Should we be calling this because if we do, it will
								// delete a bullet that was not supposed to be deleted....
								//BulletHitStructure(pBullet, 0, 0);
							}*/
						}
					}
					else
					{
						pBullet->qCurrZ += pBullet->qIncrZ;
					}
					pBullet->bLOSIndexX = FIXEDPT_TO_LOS_INDEX( pBullet->qCurrX );
					pBullet->bLOSIndexY = FIXEDPT_TO_LOS_INDEX( pBullet->qCurrY );
					pBullet->iCurrCubesZ = CONVERT_HEIGHTUNITS_TO_INDEX( FIXEDPT_TO_INT32( pBullet->qCurrZ ) );
					pBullet->iLoop++;

					if (pBullet->usFlags & (BULLET_FLAG_MISSILE | BULLET_FLAG_SMALL_MISSILE | BULLET_FLAG_TANK_CANNON |
								BULLET_FLAG_FLAME | BULLET_FLAG_CREATURE_SPIT))
					{
						INT8 bStepsPerMove = STEPS_FOR_BULLET_MOVE_TRAILS;

						if ( pBullet->usFlags & ( BULLET_FLAG_SMALL_MISSILE ) )
						{
							bStepsPerMove = STEPS_FOR_BULLET_MOVE_SMALL_TRAILS;
						}
						else if ( pBullet->usFlags & ( BULLET_FLAG_FLAME ) )
						{
							bStepsPerMove = STEPS_FOR_BULLET_MOVE_FIRE_TRAILS;
						}

						if ( pBullet->iLoop % bStepsPerMove == 0 )
						{
							// add smoke trail
							AddMissileTrail( pBullet, pBullet->qCurrX, pBullet->qCurrY, pBullet->qCurrZ );
						}
					}

				}
				while( (pBullet->bLOSIndexX == bOldLOSIndexX) && (pBullet->bLOSIndexY == bOldLOSIndexY) && (pBullet->iCurrCubesZ == iOldCubesZ));
				pBullet->iCurrTileX = FIXEDPT_TO_INT32( pBullet->qCurrX ) / CELL_X_SIZE;
				pBullet->iCurrTileY = FIXEDPT_TO_INT32( pBullet->qCurrY ) / CELL_Y_SIZE;
			}
		} while( (pBullet->iCurrTileX == iOldTileX) && (pBullet->iCurrTileY == iOldTileY));

		if ( !GridNoOnVisibleWorldTile( (INT16) (pBullet->iCurrTileX + pBullet->iCurrTileY * WORLD_COLS) ) || (pBullet->iCurrCubesZ > PROFILE_Z_SIZE * 2 && FIXEDPT_TO_INT32( pBullet->qIncrZ ) > 0 ) )
		{
			// bullet outside of world!
			RemoveBullet(pBullet);
			ShotMiss(pBullet);
			return;
		}

		pBullet->sGridNo = MAPROWCOLTOPOS( pBullet->iCurrTileY , pBullet->iCurrTileX );
		uiTileInc++;

		if ( (pBullet->iLoop > pBullet->iRange * 2) )
		{
			// beyond max effective range, bullet starts to drop!
			// since we're doing an increment based on distance, not time, the
			// decrement is scaled down depending on how fast the bullet is (effective range)
			pBullet->qIncrZ -= INT32_TO_FIXEDPT( 100 ) / (pBullet->iRange * 2);
		}
		else if ( (pBullet->usFlags & BULLET_FLAG_FLAME) && ( pBullet->iLoop > pBullet->iRange ) )
		{
			pBullet->qIncrZ -= INT32_TO_FIXEDPT( 100 ) / (pBullet->iRange * 2);
		}

		// check to see if bullet is close to target
		if (pBullet->pFirer->target != NULL &&
				!(pBullet->pFirer->uiStatusFlags & SOLDIER_ATTACK_NOTICED) &&
				PythSpacesAway(pBullet->sGridNo, pBullet->sTargetGridNo) <= 3)
		{
			pBullet->pFirer->uiStatusFlags |= SOLDIER_ATTACK_NOTICED;
		}
	} while( uiTileInc < pBullet->ubTilesPerUpdate );
	// unless the distance is integral, after the loop there will be a
	// fractional amount of distance remaining which is unchecked
	// but we shouldn't(?) need to check it because the target is there!

}


INT32 CheckForCollision(FLOAT dX, FLOAT dY, FLOAT dZ, FLOAT dDeltaX, FLOAT dDeltaY, FLOAT dDeltaZ, UINT16* pusStructureID, FLOAT* pdNormalX, FLOAT* pdNormalY, FLOAT* pdNormalZ)
{
	INT32 iLandHeight;
	INT32 iCurrAboveLevelZ;
	INT32 iCurrCubesAboveLevelZ;
	INT16 sDesiredLevel;

	MAP_ELEMENT * pMapElement;
	STRUCTURE * pStructure, *pTempStructure;

	SOLDIERTYPE * pTarget;
	//FLOAT dTargetX;
	//FLOAT dTargetY;
	FLOAT dTargetZMin;
	FLOAT dTargetZMax;

	//INT8 iImpactReduction;

	INT16 sX, sY;

	FLOAT dOldZUnits, dZUnits;

	INT8 bLOSIndexX, bLOSIndexY;


	sX = (INT16)( dX / CELL_X_SIZE );
	sY = (INT16)( dY / CELL_Y_SIZE );

	// Check if gridno is in bounds....
	if ( !GridNoOnVisibleWorldTile( (INT16) (sX + sY * WORLD_COLS) ) )
	{
		//return( COLLISION_NONE );
	}

	if ( sX < 0 || sX > WORLD_COLS || sY < 0 || sY > WORLD_COLS )
	{
		//return( COLLISION_NONE );
	}

	// check a particular tile
	// retrieve values from world for this particular tile
	pMapElement = &(gpWorldLevelData[ sX + sY * WORLD_COLS] );
	iLandHeight = CONVERT_PIXELS_TO_HEIGHTUNITS( pMapElement->sHeight );

	// Calculate old height and new hieght in pixels
	dOldZUnits = (dZ - dDeltaZ );
	dZUnits = dZ;

	//BOOLEAN fRoofPresent = FALSE;
	//if (pBullet->fCheckForRoof)
	//{
	//	if (pMapElement->pRoofHead != NULL)
	//	{
	//		fRoofPresent = TRUE;
	//	}
	//	else
	//	{
	//		fRoofPresent = FALSE;
	//	}
	//}

	//if (pMapElement->pMercHead != NULL && pBullet->iLoop != 1)
	if (pMapElement->pMercHead != NULL )
	{
		// a merc! that isn't us :-)
		pTarget = pMapElement->pMercHead->pSoldier;
		//dTargetX = pTarget->dXPos;
		//dTargetY = pTarget->dYPos;
		dTargetZMin = 0.0f;
		CalculateSoldierZPos( pTarget, HEIGHT, &dTargetZMax );
		if (pTarget->bLevel > 0)
		{
			// on roof
			dTargetZMin += WALL_HEIGHT_UNITS;
		}
	}
	else
	{
		pTarget = NULL;
	}

	// record old tile location for loop purposes

	// check for collision with the ground
	iCurrAboveLevelZ = (INT32) dZ - iLandHeight;
	if (iCurrAboveLevelZ < 0)
	{
		// ground is in the way!
		if ( pMapElement->ubTerrainID == DEEP_WATER || pMapElement->ubTerrainID == LOW_WATER || pMapElement->ubTerrainID == MED_WATER )
		{
			return ( COLLISION_WATER );
		}
		else
		{
			return ( COLLISION_GROUND );
		}
	}
	// check for the existence of structures
	pStructure = pMapElement->pStructureHead;
	if (pStructure == NULL)
	{	// no structures in this tile

		// we can go as far as we like vertically (so long as we don't hit
		// the ground), but want to stop when we get to the next tile or
		// the end of the LOS path

		// move 1 unit along the bullet path
		//if (fRoofPresent)
		//{
		//	dLastZ = pBullet->dCurrZ;
		//	(pBullet->dCurrZ) += pBullet->dIncrZ;
		//	if ( (dLastZ > WALL_HEIGHT && pBullet->dCurrZ < WALL_HEIGHT) || (dLastZ < WALL_HEIGHT && pBullet->dCurrZ > WALL_HEIGHT))
		//	{
		//		// generate roof-hitting event
		//		BulletHitStructure(pBullet);
		//		RemoveBullet(pBullet);
		//		return;
		//	}
		//}
		//else
		//{
		//	(pBullet->dCurrZ) += pBullet->dIncrZ;
		//}

		// check for ground collision
		if ( dZ < iLandHeight)
		{
			// ground is in the way!
			if (pMapElement->ubTerrainID == DEEP_WATER || pMapElement->ubTerrainID == LOW_WATER ||
				pMapElement->ubTerrainID == MED_WATER )
			{
				return ( COLLISION_WATER );
			}
			else
			{
				return ( COLLISION_GROUND );
			}
		}

		if ( gfCaves || gfBasement )
		{
			if ( dOldZUnits > HEIGHT_UNITS && dZUnits  < HEIGHT_UNITS )
			{
				return( COLLISION_ROOF );
			}
			if ( dOldZUnits < HEIGHT_UNITS && dZUnits  > HEIGHT_UNITS )
			{
				return( COLLISION_INTERIOR_ROOF );
			}
		}

		// check to see if we hit someone
		//if (pTarget && Distance2D( dX - dTargetX, dY - dTargetY ) < HIT_DISTANCE )
		//{
		//	// well, we're in the right area; it's possible that
		//	// we're firing over or under them though
		//	if ( dZ < dTargetZMax && dZ > dTargetZMin)
		//	{
		//		return( COLLISION_MERC );
		//	}
		//}

	}
	else
	{
		// there are structures in this tile
		iCurrCubesAboveLevelZ = CONVERT_HEIGHTUNITS_TO_INDEX( iCurrAboveLevelZ );
		// figure out the LOS cube level of the current point

		// CALCULAT LOS INDEX
		bLOSIndexX = CONVERT_WITHINTILE_TO_INDEX( ((INT32)dX) % CELL_X_SIZE );
		bLOSIndexY = CONVERT_WITHINTILE_TO_INDEX( ((INT32)dY) % CELL_Y_SIZE );

		if (iCurrCubesAboveLevelZ < STRUCTURE_ON_ROOF_MAX)
		{
			if (iCurrCubesAboveLevelZ < STRUCTURE_ON_GROUND_MAX)
			{
				// check objects on the ground
				sDesiredLevel = STRUCTURE_ON_GROUND;
			}
			else
			{
				// check objects on roofs
				sDesiredLevel = STRUCTURE_ON_ROOF;
				iCurrCubesAboveLevelZ -= STRUCTURE_ON_ROOF;
			}

			// check structures for collision
			while (pStructure != NULL)
			{

				if (pStructure->fFlags & STRUCTURE_ROOF || gfCaves || gfBasement )
				{
					if ( dOldZUnits > HEIGHT_UNITS && dZUnits  < HEIGHT_UNITS )
					{
						return( COLLISION_ROOF );
					}
					if ( dOldZUnits < HEIGHT_UNITS && dZUnits  > HEIGHT_UNITS )
					{
						return( COLLISION_INTERIOR_ROOF );
					}
				}

				if (pStructure->sCubeOffset == sDesiredLevel)
				{

					if (((*(pStructure->pShape))[bLOSIndexX][bLOSIndexY] & AtHeight[iCurrCubesAboveLevelZ]) > 0)
					{
						*pusStructureID = pStructure->usStructureID;

						if (pStructure->fFlags & STRUCTURE_WALLNWINDOW && dZ >= WINDOW_BOTTOM_HEIGHT_UNITS &&
							dZ <= WINDOW_TOP_HEIGHT_UNITS)
						{
							if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
								pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)
							{

								if (dDeltaX > 0)
								{
									return( COLLISION_WINDOW_SOUTHWEST );
								}
								else
								{
									return( COLLISION_WINDOW_NORTHWEST );
								}
							}
							else
							{
								if ( dDeltaY > 0)
								{
									return( COLLISION_WINDOW_SOUTHEAST );
								}
								else
								{
									return( COLLISION_WINDOW_NORTHEAST );
								}
							}
						}

						if (pStructure->fFlags & STRUCTURE_WALLSTUFF )
						{
							*pdNormalX = 0;
							*pdNormalY = 0;
							*pdNormalZ = 0;

							if (pStructure->ubWallOrientation == INSIDE_TOP_RIGHT ||
								pStructure->ubWallOrientation == OUTSIDE_TOP_RIGHT)
							{

								if (dDeltaX > 0)
								{
									*pdNormalX = -1;
									return( COLLISION_WALL_SOUTHEAST );
								}
								else
								{
									*pdNormalX = 1;
									return( COLLISION_WALL_NORTHEAST );
								}
							}
							else
							{
								if ( dDeltaY > 0)
								{
									*pdNormalY = -1;
									return( COLLISION_WALL_SOUTHWEST );
								}
								else
								{
									*pdNormalY = 1;
									return( COLLISION_WALL_NORTHWEST );
								}
							}

						}
						else
						{
							// Determine if we are on top of this struct
							// If we are a tree, not dense enough to stay!
							if (!(pStructure->fFlags & STRUCTURE_TREE) && !(pStructure->fFlags & STRUCTURE_CORPSE))
							{
								if ( iCurrCubesAboveLevelZ < PROFILE_Z_SIZE-1 )
								{
									if (!((*(pStructure->pShape))[bLOSIndexX][bLOSIndexY] & AtHeight[ iCurrCubesAboveLevelZ + 1 ]))
									{
										if ( ( pStructure->fFlags & STRUCTURE_ROOF ) )
										{
											return( COLLISION_ROOF );
										}
										else
										{
											return( COLLISION_STRUCTURE_Z );
										}
									}
								}
								else
								{
									// Search next level ( if we are ground )
									if ( sDesiredLevel == STRUCTURE_ON_GROUND )
									{
										pTempStructure = pMapElement->pStructureHead;

										// LOOK at ALL structs on roof
										while ( pTempStructure != NULL )
										{
											if (pTempStructure->sCubeOffset == STRUCTURE_ON_ROOF )
											{
												if ( !((*(pTempStructure->pShape))[bLOSIndexX][bLOSIndexY] & AtHeight[ 0 ]) )
												{
													return( COLLISION_STRUCTURE_Z );
												}

											}

											pTempStructure = pTempStructure->pNext;
										}
									}
									else
									{
										// We are very high!
										return( COLLISION_STRUCTURE_Z );
									}
								}
							}

							// Check armour rating.....
							// ATE; not if small vegitation....
							if ( pStructure->pDBStructureRef->pDBStructure->ubArmour != MATERIAL_LIGHT_VEGETATION )
							{
								if ( !(pStructure->fFlags & STRUCTURE_CORPSE ) )
								{
									return( COLLISION_STRUCTURE );
								}
							}
						}
					}
				}
				pStructure = pStructure->pNext;
			}

		}

		// check to see if we hit someone
		//if (pTarget && Distance2D( dX - dTargetX, dY - dTargetY ) < HIT_DISTANCE )
		//{
		//	// well, we're in the right area; it's possible that
		//	// we're firing over or under them though
		//	if ( dZ < dTargetZMax && dZ > dTargetZMin)
		//	{
		//		return( COLLISION_MERC );
		//	}
		//}

	}

	return( COLLISION_NONE );
}
