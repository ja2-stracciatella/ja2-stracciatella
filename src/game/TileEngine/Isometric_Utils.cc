#include "Cursor_Control.h"
#include "Structure.h"
#include "WorldDef.h"
#include "RenderWorld.h"
#include "Isometric_Utils.h"
#include "Interface.h"
#include "math.h"
#include "WorldMan.h"
#include "Structure_Wrap.h"
#include "Overhead.h"
#include "Random.h"
#include "PathAI.h"
#include "UILayout.h"


// GLOBALS

// DIRECTION FACING    DIRECTION WE WANT TO GOTO
UINT8 const gPurpendicularDirection[NUM_WORLD_DIRECTIONS][NUM_WORLD_DIRECTIONS] =
{
	{ // NORTH
		WEST,		// EITHER
		NORTHWEST,
		NORTH,
		NORTHEAST,
		EAST,		// EITHER
		NORTHWEST,
		NORTH,
		NORTHEAST
	},

	{ // NORTH EAST
		NORTHWEST,
		NORTHWEST,	// EITHER
		SOUTH,
		NORTHEAST,
		EAST,
		SOUTHEAST,	// EITHER
		NORTH,
		NORTHEAST
	},

	{ // EAST
		EAST,
		SOUTHEAST,
		NORTH,		// EITHER
		NORTHEAST,
		EAST,
		SOUTHEAST,
		NORTH,		// EITHER
		NORTHEAST
	},

	{ // SOUTHEAST
		EAST,
		SOUTHEAST,
		SOUTH,
		SOUTHWEST,	// EITHER
		SOUTHWEST,
		SOUTHEAST,
		SOUTH,
		SOUTHWEST	// EITHER
	},

	{ // SOUTH
		WEST,		// EITHER
		SOUTHEAST,
		SOUTH,
		SOUTHWEST,
		EAST,		// EITHER
		SOUTHEAST,
		SOUTH,
		SOUTHWEST
	},

	{ // SOUTHWEST
		WEST,
		NORTHWEST,	// EITHER
		SOUTH,
		SOUTHWEST,
		WEST,
		SOUTHEAST,	// EITHER
		SOUTH,
		SOUTHWEST
	},

	{ // WEST
		WEST,
		NORTHWEST,
		NORTH,		// EITHER
		SOUTHWEST,
		WEST,
		NORTHWEST,
		SOUTH,		// EITHER
		SOUTHWEST
	},

	{ // NORTHWEST
		WEST,
		NORTHWEST,
		NORTH,
		SOUTHWEST,	// EITHER
		SOUTHWEST,
		NORTHWEST,
		NORTH,
		NORTHEAST	// EITHER
	}
};


void FromCellToScreenCoordinates( INT16 sCellX, INT16 sCellY, INT16 *psScreenX, INT16 *psScreenY )
{
	// cartesian to isometric
	*psScreenX = (sCellX - sCellY) * HALF_TILE_WIDTH;
	*psScreenY = (sCellX + sCellY) * HALF_TILE_HEIGHT;
}

void FromScreenToCellCoordinates( INT16 sScreenX, INT16 sScreenY, INT16 *psCellX, INT16 *psCellY )
{
	// isometric to cartesian
	*psCellX = floor((FLOAT(sScreenX) / HALF_TILE_WIDTH + FLOAT(sScreenY) / HALF_TILE_HEIGHT) / 2.0f);
	*psCellY = floor((FLOAT(sScreenY) / HALF_TILE_HEIGHT - FLOAT(sScreenX) / HALF_TILE_WIDTH) / 2.0f);
}

// These two functions take into account that our world is projected and attached
// to the screen (0,0) in a specific way, and we MUSt take that into account then
// determining screen coords

void FloatFromCellToScreenCoordinates( FLOAT dCellX, FLOAT dCellY, FLOAT *pdScreenX, FLOAT *pdScreenY )
{
	FLOAT dScreenX, dScreenY;

	dScreenX = ( 2 * dCellX ) - ( 2 * dCellY );
	dScreenY = dCellX + dCellY;

	*pdScreenX = dScreenX;
	*pdScreenY = dScreenY;
}


BOOLEAN GetMouseXY( INT16 *psMouseX, INT16 *psMouseY )
{
	INT16 sWorldX, sWorldY;

	if ( !GetMouseWorldCoords( &sWorldX, &sWorldY ) )
	{
		(*psMouseX) = 0;
		(*psMouseY) = 0;
		return( FALSE );
	}

	// Find start block
	(*psMouseX) = ( sWorldX / CELL_X_SIZE );
	(*psMouseY) = ( sWorldY / CELL_Y_SIZE );

	return( TRUE );
}


BOOLEAN GetMouseWorldCoords( INT16 *psMouseX, INT16 *psMouseY )
{
	INT16 sOffsetX, sOffsetY;
	INT16 sTempPosX_W, sTempPosY_W;
	INT16 sStartPointX_W, sStartPointY_W;

	// Convert mouse screen coords into offset from center
	if ( ! ( gViewportRegion.uiFlags & MSYS_MOUSE_IN_AREA ) )
	{
		*psMouseX = 0;
		*psMouseY = 0;
		return( FALSE );
	}

	auto const cursorPosition{ GetCursorPos() };
	sOffsetX = cursorPosition.iX - ( g_ui.m_tacticalMapCenterX ); // + gsRenderWorldOffsetX;
	sOffsetY = cursorPosition.iY - ( g_ui.m_tacticalMapCenterY ) + 10;// + gsRenderWorldOffsetY;

	// OK, Let's offset by a value if our interfac level is changed!
	if ( gsInterfaceLevel != 0 )
	{
		//sOffsetY -= 50;
	}


	FromScreenToCellCoordinates( sOffsetX, sOffsetY, &sTempPosX_W, &sTempPosY_W );

	// World start point is Render center plus this distance
	sStartPointX_W = gsRenderCenterX + sTempPosX_W;
	sStartPointY_W = gsRenderCenterY + sTempPosY_W;


	// check if we are out of bounds..
	if (sStartPointX_W < 0 || sStartPointX_W >= WORLD_COORD_COLS || sStartPointY_W < 0 || sStartPointY_W >= WORLD_COORD_ROWS)
	{
		*psMouseX = 0;
		*psMouseY = 0;
		return( FALSE );
	}

	// Determine Start block and render offsets
	// Find start block
	// Add adjustment for render origin as well
	(*psMouseX) = sStartPointX_W;
	(*psMouseY) = sStartPointY_W;

	return( TRUE );
}

void GetAbsoluteScreenXYFromMapPos(const GridNo pos, INT16* const psWorldScreenX, INT16* const psWorldScreenY)
{
	INT16 sScreenCenterX, sScreenCenterY;

	INT16 sWorldCellX;
	INT16 sWorldCellY;
	ConvertGridNoToCellXY(pos, &sWorldCellX, &sWorldCellY);

	// Find the diustance from render center to true world center
	const INT16 sDistToCenterX = sWorldCellX - gCenterWorldX;
	const INT16 sDistToCenterY = sWorldCellY - gCenterWorldY;


	// From render center in world coords, convert to render center in "screen" coords

	// ATE: We should call the fowllowing function but I'm putting it here verbatim for speed
	//FromCellToScreenCoordinates( sDistToCenterX , sDistToCenterY, &sScreenCenterX, &sScreenCenterY );
	sScreenCenterX = ( 2 * sDistToCenterX ) - ( 2 * sDistToCenterY );
	sScreenCenterY = sDistToCenterX + sDistToCenterY;

	// Subtract screen center
	*psWorldScreenX = sScreenCenterX + gsCX - gsLeftX;
	*psWorldScreenY = sScreenCenterY + gsCY - gsTopY;

}


GridNo GetMapPosFromAbsoluteScreenXY(const INT16 sWorldScreenX, const INT16 sWorldScreenY)
{
	INT16 sWorldCenterX, sWorldCenterY;
	INT16 sDistToCenterY, sDistToCenterX;

	// Subtract screen center
	sDistToCenterX = sWorldScreenX - gsCX + gsLeftX;
	sDistToCenterY = sWorldScreenY - gsCY + gsTopY;

	// From render center in world coords, convert to render center in "screen" coords

	// ATE: We should call the fowllowing function but I'm putting it here verbatim for speed
	//FromCellToScreenCoordinates( sDistToCenterX , sDistToCenterY, &sScreenCenterX, &sScreenCenterY );
	sWorldCenterX = ( ( sDistToCenterX + ( 2 * sDistToCenterY ) ) / 4 );
	sWorldCenterY = ( ( 2 * sDistToCenterY ) - sDistToCenterX ) / 4;

	// Goto center again
	sWorldCenterX += gCenterWorldX;
	sWorldCenterY += gCenterWorldY;

	return GETWORLDINDEXFROMWORLDCOORDS(sWorldCenterY, sWorldCenterX);
}


// UTILITY FUNTIONS

INT32 OutOfBounds(INT16 sGridno, INT16 sProposedGridno)
{
	INT16 sMod,sPropMod;

	// get modulas of our origin
	sMod = sGridno % MAXCOL;

	if (sMod != 0)  		// if we're not on leftmost grid
		if (sMod != RIGHTMOSTGRID)	// if we're not on rightmost grid
			if (sGridno < LASTROWSTART)	// if we're above bottom row
				if (sGridno > MAXCOL)	// if we're below top row
				// Everything's OK - we're not on the edge of the map
					return(FALSE);


	// if we've got this far, there's a potential problem - check it out!

	if (sProposedGridno < 0)
		return(TRUE);

	sPropMod = sProposedGridno % MAXCOL;

	if (sMod == 0 && sPropMod == RIGHTMOSTGRID)
		return(TRUE);
	else if (sMod == RIGHTMOSTGRID && sPropMod == 0)
		return TRUE;
	else
		return (sGridno >= LASTROWSTART && sProposedGridno >= GRIDSIZE);
}



INT16 NewGridNo(INT16 sGridno, INT16 sDirInc)
{
	INT16 sProposedGridno = sGridno + sDirInc;

	// now check for out-of-bounds
	if (OutOfBounds(sGridno,sProposedGridno))
		// return ORIGINAL gridno to user
		sProposedGridno = sGridno;

	return(sProposedGridno);
}


INT16 DirectionInc(UINT8 ubDirection)
{
	if (ubDirection > 7)
	{
		//direction = random(8);	// replace garbage with random direction
		ubDirection = 1;
	}
	return(DirIncrementer[ubDirection]);
}


void CellXYToScreenXY(INT16 const sCellX, INT16 const sCellY, INT16* const sScreenX, INT16* const sScreenY)
{
	INT16 sDeltaCellX, sDeltaCellY;
	INT16 sDeltaScreenX, sDeltaScreenY;

	sDeltaCellX=sCellX-gsRenderCenterX;
	sDeltaCellY=sCellY-gsRenderCenterY;

	FromCellToScreenCoordinates(sDeltaCellX, sDeltaCellY, &sDeltaScreenX, &sDeltaScreenY);

	*sScreenX=((g_ui.m_tacticalMapCenterX)+sDeltaScreenX);
	*sScreenY=((g_ui.m_tacticalMapCenterY)+sDeltaScreenY);
}


void ConvertGridNoToXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos )
{
	*sYPos = sGridNo / WORLD_COLS;
	*sXPos = ( sGridNo - ( *sYPos * WORLD_COLS ) );
}

void ConvertGridNoToCellXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos )
{
	*sYPos = ( sGridNo / WORLD_COLS );
	*sXPos = sGridNo - ( *sYPos * WORLD_COLS );

	*sYPos = ( *sYPos * CELL_Y_SIZE );
	*sXPos = ( *sXPos * CELL_X_SIZE );
}


void ConvertGridNoToCenterCellXY(const INT16 gridno, INT16* const x, INT16* const y)
{
	*x = gridno % WORLD_COLS * CELL_X_SIZE + CELL_X_SIZE / 2;
	*y = gridno / WORLD_COLS * CELL_Y_SIZE + CELL_Y_SIZE / 2;
}


INT32 GetRangeFromGridNoDiff( INT16 sGridNo1, INT16 sGridNo2 )
{
	INT32 uiDist;
	INT16 sXPos, sYPos, sXPos2, sYPos2;

	// Convert our grid-not into an XY
	ConvertGridNoToXY( sGridNo1, &sXPos, &sYPos );

	// Convert our grid-not into an XY
	ConvertGridNoToXY( sGridNo2, &sXPos2, &sYPos2 );

	uiDist = (INT16) std::hypot(sXPos2 - sXPos, sYPos2 - sYPos);

	return( uiDist );
}

INT32 GetRangeInCellCoordsFromGridNoDiff( INT16 sGridNo1, INT16 sGridNo2 )
{
	INT16 sXPos, sYPos, sXPos2, sYPos2;

	// Convert our grid-not into an XY
	ConvertGridNoToXY( sGridNo1, &sXPos, &sYPos );

	// Convert our grid-not into an XY
	ConvertGridNoToXY( sGridNo2, &sXPos2, &sYPos2 );

	return (INT32) (std::hypot(sXPos2 - sXPos, sYPos2 - sYPos) * CELL_X_SIZE);
}


bool IsPointInScreenRect(INT16 const x, INT16 const y, SGPRect const& r)
{
	return r.iLeft <= x && x <= r.iRight && r.iTop <= y && y <= r.iBottom;
}


BOOLEAN IsPointInScreenRectWithRelative( INT16 sXPos, INT16 sYPos, SGPRect *pRect, INT16 *sXRel, INT16 *sYRel )
{
	if ( (sXPos >= pRect->iLeft) && (sXPos <= pRect->iRight) && (sYPos >= pRect->iTop) && (sYPos <= pRect->iBottom) )
	{
		(*sXRel) = pRect->iLeft - sXPos;
		(*sYRel) = sYPos - (INT16)pRect->iTop;

		return( TRUE );
	}
	else
	{
		return( FALSE );
	}
}


INT16 PythSpacesAway(INT16 sOrigin, INT16 sDest)
{
	INT16 const sRows = (sOrigin / MAXCOL) - (sDest / MAXCOL);
	INT16 const sCols = (sOrigin % MAXROW) - (sDest % MAXROW);

	return static_cast<INT16>(std::hypot(sRows, sCols));
}


INT16 SpacesAway(INT16 sOrigin, INT16 sDest)
{
	INT16 sRows,sCols;

	sRows = std::abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
	sCols = std::abs((sOrigin % MAXROW) - (sDest % MAXROW));

	return( std::max(sRows, sCols ));
}

INT16 CardinalSpacesAway(INT16 sOrigin, INT16 sDest)
// distance away, ignoring diagonals!
{
	INT16 sRows,sCols;

	sRows = std::abs((sOrigin / MAXCOL) - (sDest / MAXCOL));
	sCols = std::abs((sOrigin % MAXROW) - (sDest % MAXROW));

	return( (INT16)( sRows + sCols ) );
}


static UINT8 FindNumTurnsBetweenDirs(const UINT8 sDir1, const UINT8 sDir2)
{
	const UINT8 steps = sDir1 > sDir2 ? sDir1 - sDir2 : sDir2 - sDir1;
	return steps <= 4 ? steps : 8 - steps;
}


bool FindHigherLevel(SOLDIERTYPE const* const s, UINT8* const out_direction)
{
	if (s->bLevel > 0) return false;

	GridNo const grid_no = s->sGridNo;
	// If there is a roof over our heads, this is an ivalid
	if (FindStructure(grid_no, STRUCTURE_ROOF)) return false;

	bool       found         = false;
	UINT8      min_turns     = 100;
	UINT8      min_direction = 0;
	UINT8 const starting_dir = s->bDirection;
	for (UINT8 cnt = 0; cnt != 8; cnt += 2)
	{
		GridNo const new_grid_no = NewGridNo(grid_no, DirectionInc(cnt));
		if (!NewOKDestination(s, new_grid_no, TRUE, 1)) continue;

		// Check if this tile has a higher level
		if (!IsHeigherLevel(new_grid_no)) continue;

		// FInd how many turns we should go to get here
		UINT8 const n_turns = FindNumTurnsBetweenDirs(cnt, starting_dir);
		if (min_turns <= n_turns) continue;

		found         = true;
		min_turns     = n_turns;
		min_direction = cnt;
	}

	if (!found) return false;

	if (out_direction) *out_direction = min_direction;
	return true;
}


bool FindLowerLevel(SOLDIERTYPE const* const s, UINT8* const out_direction)
{
	if (s->bLevel == 0) return false;

	bool         found         = false;
	UINT8        min_turns     = 100;
	UINT8        min_direction = 0;
	GridNo const grid_no       = s->sGridNo;
	UINT8  const starting_dir  = s->bDirection;
	for (UINT8 dir = 0; dir != 8; dir += 2)
	{
		GridNo const new_grid_no = NewGridNo(grid_no, DirectionInc(dir));
		if (!NewOKDestination(s, new_grid_no, TRUE, 0)) continue;
		// Make sure there is NOT a roof here
		if (FindStructure(new_grid_no, STRUCTURE_ROOF)) continue;

		// Find how many turns we should go to get here
		UINT8 const n_turns = FindNumTurnsBetweenDirs(dir, starting_dir);
		if (min_turns <= n_turns) continue;

		found         = true;
		min_turns     = n_turns;
		min_direction = dir;
	}

	if (!found) return false;

	if (out_direction) *out_direction = min_direction;
	return true;
}


INT8 QuickestDirection(UINT8 const origin, UINT8 const dest, UINT8 maxDistance)
{
	if (origin == dest) return 0;
	if (origin > dest) return origin - dest >= maxDistance ? 1 : -1;
	return dest - origin > maxDistance ? -1 : 1;
}


// Returns the (center ) cell coordinates in X
INT16 CenterX( INT16 sGridNo )
{
	INT16 sXPos;

	sXPos = sGridNo % WORLD_COLS;

	return( ( sXPos * CELL_X_SIZE ) + ( CELL_X_SIZE / 2 ) );
}


// Returns the (center ) cell coordinates in Y
INT16 CenterY( INT16 sGridNo )
{
	INT16 sYPos;

	sYPos = sGridNo / WORLD_COLS;

	return( ( sYPos * CELL_Y_SIZE ) + ( CELL_Y_SIZE / 2 ) );
}


BOOLEAN GridNoOnVisibleWorldTile( INT16 sGridNo )
{
	INT16 sWorldX;
	INT16 sWorldY;
	GetAbsoluteScreenXYFromMapPos(sGridNo, &sWorldX, &sWorldY);

	if (sWorldX > 0 && sWorldX < (gsRightX - gsLeftX - 20) &&
		sWorldY > 20 && sWorldY < (gsBottomY - gsTopY - 20))
	{
		return( TRUE );
	}

	return( FALSE );
}


BOOLEAN GridNoOnEdgeOfMap( INT16 sGridNo, INT8 * pbDirection )
{
	INT8 bDir;

	// check NE, SE, SW, NW because of tilt of isometric display

	for (bDir = NORTHEAST; bDir < NUM_WORLD_DIRECTIONS; bDir += 2 )
	{
		if (gubWorldMovementCosts[ (sGridNo + DirectionInc( bDir ) ) ][ bDir ][ 0 ] == TRAVELCOST_OFF_MAP)
		//if ( !GridNoOnVisibleWorldTile( (INT16) (sGridNo + DirectionInc( bDir ) ) ) )
		{
			*pbDirection = bDir;
			return( TRUE );
		}
	}
	return( FALSE );
}

BOOLEAN IsFacingClimableWindow( SOLDIERTYPE const* const pSoldier )
{
	GridNo sNewGridNo, sOtherSideOfWindow;

	GridNo const sGridNo = pSoldier->sGridNo;
	INT8 const bStartingDir=pSoldier->bDirection;

	// WANNE: No need to check on SOUTH and EAST tile, because it is the tile that has the fence we are standing on!
	if (bStartingDir == NORTH || bStartingDir == WEST)
	{
		// IF there is a fence in this gridno, return false!
		if ( IsJumpableWindowPresentAtGridNo( sGridNo, bStartingDir ) )
		{
			return( FALSE );
		}

		sNewGridNo = NewGridNo( sGridNo, (UINT16)DirectionInc( (UINT8)bStartingDir ) );
		sOtherSideOfWindow = sNewGridNo;
	}
	else
	{
		// current tile we are standing is the fence tile
		sNewGridNo = sGridNo;
		sOtherSideOfWindow = NewGridNo( sNewGridNo, (UINT16)DirectionInc( (UINT8)bStartingDir ) );
	}

	// ATE: Check if there is somebody waiting here.....
	if (! NewOKDestination( pSoldier, sOtherSideOfWindow, TRUE, 0 ) ) return false;

	// Check if we have a window here
	if (!IsJumpableWindowPresentAtGridNo( sNewGridNo , bStartingDir) ) return false;

	return true;
}

BOOLEAN FindFenceJumpDirection(SOLDIERTYPE const* const pSoldier, UINT8* const out_direction)
{
	GridNo  sNewGridNo, sOtherSideOfFence;
	BOOLEAN fFound = FALSE;
	UINT8   bMinNumTurns = 100;
	UINT8   bMinDirection = 0;

	GridNo const sGridNo = pSoldier->sGridNo;
	// IF there is a fence in this gridno, return false!
	if ( IsJumpableFencePresentAtGridno( sGridNo ) )
	{
		return( FALSE );
	}

	// LOOP THROUGH ALL 8 DIRECTIONS
	UINT8 const bStartingDir = pSoldier->bDirection;
	for ( UINT8 cnt = 0; cnt < 8; cnt += 2 )
	{
		// go out *2* tiles
		sNewGridNo = NewGridNo( sGridNo, DirectionInc( cnt ) );
		sOtherSideOfFence = NewGridNo( sNewGridNo, DirectionInc( cnt ) );

		if ( NewOKDestination( pSoldier, sOtherSideOfFence, TRUE, 0 ) )
		{
			// ATE: Check if there is somebody waiting here.....


			// Check if we have a fence here
			if ( IsJumpableFencePresentAtGridno( sNewGridNo ) )
			{
				fFound = TRUE;

				// FInd how many turns we should go to get here
				UINT8 bNumTurns = FindNumTurnsBetweenDirs( cnt, bStartingDir );

				if ( bNumTurns < bMinNumTurns )
				{
					bMinNumTurns = bNumTurns;
					bMinDirection = cnt;
				}
			}
		}
	}

	if ( fFound )
	{
		if (out_direction) *out_direction = bMinDirection;
		return( TRUE );
	}

	return( FALSE );
}

//Simply chooses a random gridno within valid boundaries (for dropping things in unloaded sectors)
INT16 RandomGridNo()
{
	INT32 iMapXPos, iMapYPos, iMapIndex;
	do
	{
		iMapXPos = Random( WORLD_COLS );
		iMapYPos = Random( WORLD_ROWS );
		iMapIndex = iMapYPos * WORLD_COLS + iMapXPos;
	} while( !GridNoOnVisibleWorldTile( (INT16)iMapIndex ) );
	return (INT16)iMapIndex;
}

#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(Isometric_Utils, FindNumTurnsBetweenDirs)
{
	// Same direction for both arguments must always return 0
	for (UINT8 a = 0; a < 8; ++a)
		EXPECT_EQ(FindNumTurnsBetweenDirs(a, a), 0);

	// Order of arguments must not matter
	for (UINT8 a = 0; a < 8; ++a)
		EXPECT_EQ(FindNumTurnsBetweenDirs(3, a), FindNumTurnsBetweenDirs(a, 3));

	// Expected results for this loop: 1, 2, 3, 4
	for (UINT8 a = 1; a <= 4; ++a)
		EXPECT_EQ(FindNumTurnsBetweenDirs(0, a), a);

	// Expected results for this loop: 3, 2, 1
	for (UINT8 a = 5; a <= 7; ++a)
		EXPECT_EQ(FindNumTurnsBetweenDirs(0, a), 8 - a);
}

TEST(Isometric_Utils, QuickestDirection)
{
	EXPECT_EQ(QuickestDirection(1,3), 1);
	EXPECT_EQ(QuickestDirection(0,6), -1);
	EXPECT_EQ(QuickestDirection(5,0), 1);

	for (UINT a = 0; a <= 7; ++a)
		EXPECT_EQ(QuickestDirection(a,a), 0);

	// For opposite directions we want to go clockwise (1)
	for (UINT a = 0; a <= 3; ++a)
		EXPECT_EQ(QuickestDirection(a,a+4), 1);

	for (UINT a = 1; a <= 7; ++a)
		if (a == 4) ; else EXPECT_EQ(QuickestDirection(0,a), -QuickestDirection(a,0));
}

// Verify that QuickestDirection works with a MaxDistance of 16,
// equivalent to the removed function ExtQuickestDirection
TEST(Isometric_Utils, ExtQuickestDirection)
{
	auto ExtQuickestDirection = [](UINT8 a, UINT8 b)
	{
		return QuickestDirection(a, b, 16);
	};

	EXPECT_EQ(ExtQuickestDirection(1,3), 1);
	EXPECT_EQ(ExtQuickestDirection(0,6), 1);
	EXPECT_EQ(ExtQuickestDirection(0,25), -1);
	EXPECT_EQ(ExtQuickestDirection(5,0), -1);
	EXPECT_EQ(ExtQuickestDirection(24,0), 1);

	for (UINT a = 0; a <= 31; ++a)
		EXPECT_EQ(ExtQuickestDirection(a,a), 0);

	// For opposite directions we want to go clockwise (1)
	for (UINT a = 0; a <= 15; ++a)
		EXPECT_EQ(ExtQuickestDirection(a,a+16), 1);

	for (UINT a = 1; a <= 31; ++a)
		if (a == 16) ; else EXPECT_EQ(ExtQuickestDirection(0,a), -ExtQuickestDirection(a,0));
}

#endif
