#ifndef __ISOMETRIC_UTILSH
#define __ISOMETRIC_UTILSH

#include "WorldDef.h"

//DEFINES
#define MAXCOL					WORLD_COLS
#define MAXROW					WORLD_ROWS
#define GRIDSIZE        (MAXCOL * MAXROW)
#define RIGHTMOSTGRID   (MAXCOL - 1)
#define LASTROWSTART    (GRIDSIZE - MAXCOL)
#define NOWHERE         (GRIDSIZE + 1)
#define	NO_MAP_POS			NOWHERE
#define MAPWIDTH			(WORLD_COLS)
#define MAPHEIGHT			(WORLD_ROWS)
#define MAPLENGTH			(MAPHEIGHT*MAPWIDTH)


#define	ADJUST_Y_FOR_HEIGHT( pos, y )				( y -= gpWorldLevelData[ pos ].sHeight )


UINT8 gOppositeDirection[ NUM_WORLD_DIRECTIONS ];
UINT8 gTwoCCDirection[ NUM_WORLD_DIRECTIONS ];
UINT8 gTwoCDirection[ NUM_WORLD_DIRECTIONS ];
UINT8 gOneCDirection[ NUM_WORLD_DIRECTIONS ];
UINT8 gOneCCDirection[ NUM_WORLD_DIRECTIONS ];

UINT8 gPurpendicularDirection[ NUM_WORLD_DIRECTIONS ][ NUM_WORLD_DIRECTIONS ];

// Macros


//                                                |Check for map bounds------------------------------------------|   |Invalid-|   |Valid-------------------|
#define MAPROWCOLTOPOS( r, c )									( ( (r < 0) || (r >= WORLD_ROWS) || (c < 0) || (c >= WORLD_COLS) ) ? ( 0xffff ) : ( (r) * WORLD_COLS + (c) ) )

#define GETWORLDINDEXFROMWORLDCOORDS( r, c )		( (INT16) ( r / CELL_X_SIZE ) ) * WORLD_COLS + ( (INT16) ( c / CELL_Y_SIZE ) )

void ConvertGridNoToXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos );
void ConvertGridNoToCellXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos );
void ConvertGridNoToCenterCellXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos );


// GRID NO MANIPULATION FUNCTIONS
INT16 NewGridNo(INT16 sGridno, INT16 sDirInc);
INT16 DirectionInc(INT16 sDirection);
INT32 OutOfBounds(INT16 sGridno, INT16 sProposedGridno);


// Functions
BOOLEAN GetMouseCell( INT32 *piMouseMapPos );
BOOLEAN GetMouseXY( INT16 *psMouseX, INT16 *psMouseY );
BOOLEAN GetMouseWorldCoords( INT16 *psMouseX, INT16 *psMouseY );
BOOLEAN GetMouseMapPos( INT16	*psMapPos );
BOOLEAN GetMouseWorldCoordsInCenter( INT16 *psMouseX, INT16 *psMouseY );
BOOLEAN GetMouseXYWithRemainder( INT16 *psMouseX, INT16 *psMouseY, INT16 *psCellX, INT16 *psCellY );



void GetScreenXYWorldCoords( INT16 sScreenX, INT16 sScreenY, INT16 *pWorldX, INT16 *psWorldY );
void GetScreenXYWorldCell( INT16 sScreenX, INT16 sScreenY, INT16 *psWorldCellX, INT16 *psWorldCellY );
void GetScreenXYGridNo( INT16 sScreenX, INT16 sScreenY, INT16	*psMapPos );
void GetWorldXYAbsoluteScreenXY( INT32 sWorldCellX, INT32 sWorldCellY, INT16 *psWorldScreenX, INT16 *psWorldScreenY );
void GetFromAbsoluteScreenXYWorldXY( INT32 *psWorldCellX, INT32* psWorldCellY, INT16 sWorldScreenX, INT16 sWorldScreenY );


void FromCellToScreenCoordinates( INT16 sCellX, INT16 sCellY, INT16 *psScreenX, INT16 *psScreenY );
void FromScreenToCellCoordinates( INT16 sScreenX, INT16 sScreenY, INT16 *psCellX, INT16 *psCellY );

// Higher resolution convertion functions
void FloatFromCellToScreenCoordinates( FLOAT dCellX, FLOAT dCellY, FLOAT *pdScreenX, FLOAT *pdScreenY );
void FloatFromScreenToCellCoordinates( FLOAT dScreenX, FLOAT dScreenY, FLOAT *pdCellX, FLOAT *pdCellY );

BOOLEAN GridNoOnVisibleWorldTile( INT16 sGridNo );
BOOLEAN GridNoOnVisibleWorldTileGivenYLimits( INT16 sGridNo );
BOOLEAN GridNoOnEdgeOfMap( INT16 sGridNo, INT8 * pbDirection );

BOOLEAN ConvertMapPosToWorldTileCenter( UINT16 usMapPos, INT16 *psXPos, INT16 *psYPos );

BOOLEAN CellXYToScreenXY(INT16 sCellX, INT16 sCellY, INT16 *sScreenX, INT16 *sScreenY);

INT32 GetRangeFromGridNoDiff( INT16 sGridNo1, INT16 sGridNo2 );
INT32 GetRangeInCellCoordsFromGridNoDiff( INT16 sGridNo1, INT16 sGridNo2 );

BOOLEAN IsPointInScreenRect( INT16 sXPos, INT16 sYPos, SGPRect *pRect );
BOOLEAN IsPointInScreenRectWithRelative( INT16 sXPos, INT16 sYPos, SGPRect *pRect, INT16 *sXRel, INT16 *sRelY );


INT16 PythSpacesAway(INT16 sOrigin, INT16 sDest);
INT16 SpacesAway(INT16 sOrigin, INT16 sDest);
INT16 CardinalSpacesAway(INT16 sOrigin, INT16 sDest);
INT8 FindNumTurnsBetweenDirs( INT8 sDir1, INT8 sDir2 );
BOOLEAN FindHeigherLevel( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bStartingDir, INT8 *pbDirection );
BOOLEAN FindLowerLevel( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bStartingDir, INT8 *pbDirection );

INT16 QuickestDirection(INT16 origin, INT16 dest);
INT16 ExtQuickestDirection(INT16 origin, INT16 dest);


// Returns the (center ) cell coordinates in X
INT16 CenterX( INT16 sGridno );

// Returns the (center ) cell coordinates in Y
INT16 CenterY( INT16 sGridno );

INT16 MapX( INT16 sGridNo );
INT16 MapY( INT16 sGridNo );
BOOLEAN FindFenceJumpDirection( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bStartingDir, INT8 *pbDirection );

//Simply chooses a random gridno within valid boundaries (for dropping things in unloaded sectors)
INT16 RandomGridNo();

extern UINT32 guiForceRefreshMousePositionCalculation;


#endif
