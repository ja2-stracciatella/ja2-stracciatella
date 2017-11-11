#ifndef __ISOMETRIC_UTILSH
#define __ISOMETRIC_UTILSH

#include "Overhead_Types.h"
#include "WorldDef.h"


#define MAXCOL					WORLD_COLS
#define MAXROW					WORLD_ROWS
#define GRIDSIZE        (MAXCOL * MAXROW)
#define RIGHTMOSTGRID   (MAXCOL - 1)
#define LASTROWSTART    (GRIDSIZE - MAXCOL)
#define NOWHERE         (GRIDSIZE + 1)
#define MAPWIDTH			(WORLD_COLS)
#define MAPHEIGHT			(WORLD_ROWS)
#define MAPLENGTH			(MAPHEIGHT*MAPWIDTH)


static inline UINT8 OppositeDirection(UINT dir) { return (dir + 4) % NUM_WORLD_DIRECTIONS; }
static inline UINT8 TwoCCDirection(UINT dir)    { return (dir + 6) % NUM_WORLD_DIRECTIONS; }
static inline UINT8 TwoCDirection(UINT dir)     { return (dir + 2) % NUM_WORLD_DIRECTIONS; }
static inline UINT8 OneCCDirection(UINT dir)    { return (dir + 7) % NUM_WORLD_DIRECTIONS; }
static inline UINT8 OneCDirection(UINT dir)     { return (dir + 1) % NUM_WORLD_DIRECTIONS; }

extern const UINT8 gPurpendicularDirection[NUM_WORLD_DIRECTIONS][NUM_WORLD_DIRECTIONS];

// Macros


//                                                |Check for map bounds------------------------------------------|   |Invalid-|   |Valid-------------------|
#define MAPROWCOLTOPOS( r, c )									( ( (r < 0) || (r >= WORLD_ROWS) || (c < 0) || (c >= WORLD_COLS) ) ? ( 0xffff ) : ( (r) * WORLD_COLS + (c) ) )

#define GETWORLDINDEXFROMWORLDCOORDS( r, c )		( (INT16) ( r / CELL_X_SIZE ) ) * WORLD_COLS + ( (INT16) ( c / CELL_Y_SIZE ) )

void ConvertGridNoToXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos );
void ConvertGridNoToCellXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos );
void ConvertGridNoToCenterCellXY( INT16 sGridNo, INT16 *sXPos, INT16 *sYPos );


// GRID NO MANIPULATION FUNCTIONS
INT16 NewGridNo(INT16 sGridno, INT16 sDirInc);
INT16 DirectionInc(UINT8 sDirection);
INT32 OutOfBounds(INT16 sGridno, INT16 sProposedGridno);


BOOLEAN GetMouseXY( INT16 *psMouseX, INT16 *psMouseY );
BOOLEAN GetMouseWorldCoords( INT16 *psMouseX, INT16 *psMouseY );

/* Returns the GridNo of the tile the mouse cursor is currently over or NOWHERE
	* if the cursor is not over any tile. */
GridNo GetMouseMapPos(void);


void   GetAbsoluteScreenXYFromMapPos(GridNo pos, INT16* psWorldScreenX, INT16* psWorldScreenY);
GridNo GetMapPosFromAbsoluteScreenXY(INT16 sWorldScreenX, INT16 sWorldScreenY);


void FromCellToScreenCoordinates( INT16 sCellX, INT16 sCellY, INT16 *psScreenX, INT16 *psScreenY );
void FromScreenToCellCoordinates( INT16 sScreenX, INT16 sScreenY, INT16 *psCellX, INT16 *psCellY );

// Higher resolution convertion functions
void FloatFromCellToScreenCoordinates( FLOAT dCellX, FLOAT dCellY, FLOAT *pdScreenX, FLOAT *pdScreenY );

BOOLEAN GridNoOnVisibleWorldTile( INT16 sGridNo );
BOOLEAN GridNoOnEdgeOfMap( INT16 sGridNo, INT8 * pbDirection );

void CellXYToScreenXY(INT16 sCellX, INT16 sCellY, INT16* sScreenX, INT16* sScreenY);

INT32 GetRangeFromGridNoDiff( INT16 sGridNo1, INT16 sGridNo2 );
INT32 GetRangeInCellCoordsFromGridNoDiff( INT16 sGridNo1, INT16 sGridNo2 );

bool IsPointInScreenRect(INT16 x, INT16 y, SGPRect const&);
BOOLEAN IsPointInScreenRectWithRelative( INT16 sXPos, INT16 sYPos, SGPRect *pRect, INT16 *sXRel, INT16 *sRelY );


INT16 PythSpacesAway(INT16 sOrigin, INT16 sDest);
INT16 SpacesAway(INT16 sOrigin, INT16 sDest);
INT16 CardinalSpacesAway(INT16 sOrigin, INT16 sDest);
bool FindHigherLevel(SOLDIERTYPE const*, UINT8* out_direction = 0);
bool FindLowerLevel(SOLDIERTYPE const*, UINT8* out_direction = 0);

INT16 QuickestDirection(INT16 origin, INT16 dest);
INT16 ExtQuickestDirection(INT16 origin, INT16 dest);


// Returns the (center ) cell coordinates in X
INT16 CenterX( INT16 sGridno );

// Returns the (center ) cell coordinates in Y
INT16 CenterY( INT16 sGridno );

BOOLEAN FindFenceJumpDirection(SOLDIERTYPE const*, UINT8* out_direction = 0);
BOOLEAN IsFacingClimableWindow(SOLDIERTYPE const*);

//Simply chooses a random gridno within valid boundaries (for dropping things in unloaded sectors)
INT16 RandomGridNo(void);

extern UINT32 guiForceRefreshMousePositionCalculation;

extern const INT16 DirIncrementer[8];

#endif
