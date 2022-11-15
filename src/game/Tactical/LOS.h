#ifndef LOS_H
#define LOS_H

#include "JA2Types.h"
#include "Item_Types.h"

//#define LOS_DEBUG


// fixed-point arithmetic definitions start here

typedef INT32 FIXEDPT;
// rem 1 signed bit at the top
#define FIXEDPT_WHOLE_BITS			11
#define FIXEDPT_FRACTIONAL_BITS		20
#define FIXEDPT_FRACTIONAL_RESOLUTION		1048576

#define INT32_TO_FIXEDPT( n )			( (n) << FIXEDPT_FRACTIONAL_BITS )
#define FIXEDPT_TO_INT32( n )			( (n) / FIXEDPT_FRACTIONAL_RESOLUTION )

#define FIXEDPT_TO_TILE_NUM( n )		(FIXEDPT_TO_INT32( (n) ) / CELL_X_SIZE)
#define FIXEDPT_TO_LOS_INDEX( n )		(CONVERT_WITHINTILE_TO_INDEX( FIXEDPT_TO_INT32( (n) ) % CELL_X_SIZE))

// fixed-point arithmetic definitions end here

#define	OK_CHANCE_TO_GET_THROUGH		10

enum CollisionEnums
{
	COLLISION_NONE,
	COLLISION_GROUND,
	COLLISION_MERC,
	COLLISION_WINDOW_SOUTHEAST,
	COLLISION_WINDOW_SOUTHWEST,
	COLLISION_WINDOW_NORTHEAST,
	COLLISION_WINDOW_NORTHWEST,
	COLLISION_WINDOW_NORTH,
	COLLISION_WALL_SOUTHEAST,
	COLLISION_WALL_SOUTHWEST,
	COLLISION_WALL_NORTHEAST,
	COLLISION_WALL_NORTHWEST,
	COLLISION_STRUCTURE,
	COLLISION_ROOF,
	COLLISION_INTERIOR_ROOF,
	COLLISION_STRUCTURE_Z,
	COLLISION_WATER
};


INT32 CheckForCollision(FLOAT dX, FLOAT dY, FLOAT dZ, FLOAT dDeltaX, FLOAT dDeltaY, FLOAT dDeltaZ, UINT16* pusStructureID, FLOAT* pdNormalX, FLOAT* pdNormalY, FLOAT* pdNormalZ);

INT8 FireBulletGivenTarget( SOLDIERTYPE * pFirer, FLOAT dEndX, FLOAT dEndY, FLOAT dEndZ, ItemId usHandItem, INT16 sHitBy, BOOLEAN fBuckshot, BOOLEAN fFake );

INT32 SoldierToSoldierLineOfSightTest(const SOLDIERTYPE* pStartSoldier, const SOLDIERTYPE* pEndSoldier, UINT8 ubTileSightLimit, INT8 bAware);
INT32 SoldierToLocationLineOfSightTest( SOLDIERTYPE * pStartSoldier, INT16 sGridNo, UINT8 ubSightLimit, INT8 bAware );
INT32 SoldierTo3DLocationLineOfSightTest(const SOLDIERTYPE* pStartSoldier, INT16 sGridNo, INT8 bLevel, INT8 bCubeLevel, UINT8 ubTileSightLimit, INT8 bAware);
INT32 SoldierToBodyPartLineOfSightTest( const SOLDIERTYPE * pStartSoldier, INT16 sGridNo, INT8 bLevel, UINT8 ubAimLocation, UINT8 ubTileSightLimit, INT8 bAware );
INT32 SoldierToVirtualSoldierLineOfSightTest(const SOLDIERTYPE* pStartSoldier, INT16 sGridNo, INT8 bLevel, INT8 bStance, UINT8 ubTileSightLimit, INT8 bAware);
UINT8 SoldierToSoldierBodyPartChanceToGetThrough(SOLDIERTYPE* pStartSoldier, const SOLDIERTYPE* pEndSoldier, UINT8 ubAimLocation);
UINT8 AISoldierToSoldierChanceToGetThrough(SOLDIERTYPE* pStartSoldier, const SOLDIERTYPE* pEndSoldier);
UINT8 AISoldierToLocationChanceToGetThrough( SOLDIERTYPE * pStartSoldier, INT16 sGridNo, INT8 bLevel, INT8 bCubeLevel );
UINT8 SoldierToLocationChanceToGetThrough(SOLDIERTYPE* pStartSoldier, INT16 sGridNo, INT8 bLevel, INT8 bCubeLevel, const SOLDIERTYPE* target);
INT16 SoldierToLocationWindowTest(const SOLDIERTYPE* pStartSoldier, INT16 sEndGridNo);
INT32 LocationToLocationLineOfSightTest( INT16 sStartGridNo, INT8 bStartLevel, INT16 sEndGridNo, INT8 bEndLevel, UINT8 ubTileSightLimit, INT8 bAware );

BOOLEAN CalculateSoldierZPos(const SOLDIERTYPE* pSoldier, UINT8 ubPosType, FLOAT* pdZPos);


#define HEIGHT_UNITS				256
#define HEIGHT_UNITS_PER_INDEX			(HEIGHT_UNITS / PROFILE_Z_SIZE)
#define MAX_STRUCTURE_HEIGHT			50
// 5.12 == HEIGHT_UNITS / MAX_STRUCTURE_HEIGHT
#define CONVERT_PIXELS_TO_HEIGHTUNITS( n )	((n) * HEIGHT_UNITS / MAX_STRUCTURE_HEIGHT)
#define CONVERT_PIXELS_TO_INDEX( n )		((n) * HEIGHT_UNITS / MAX_STRUCTURE_HEIGHT / HEIGHT_UNITS_PER_INDEX)
#define CONVERT_HEIGHTUNITS_TO_INDEX( n )	((n) / HEIGHT_UNITS_PER_INDEX)
#define CONVERT_HEIGHTUNITS_TO_DISTANCE( n )	((n) / (HEIGHT_UNITS / CELL_X_SIZE) )
#define CONVERT_HEIGHTUNITS_TO_PIXELS( n )	((n) * MAX_STRUCTURE_HEIGHT / HEIGHT_UNITS )
#define CONVERT_WITHINTILE_TO_INDEX( n )	((n) >> 1)
#define CONVERT_INDEX_TO_WITHINTILE( n )	((n) << 1)
#define CONVERT_INDEX_TO_PIXELS( n )		((n) * MAX_STRUCTURE_HEIGHT * HEIGHT_UNITS_PER_INDEX / HEIGHT_UNITS)


enum {
	LOS_POS,
	FIRING_POS,
	TARGET_POS,
	HEAD_TARGET_POS,
	TORSO_TARGET_POS,
	LEGS_TARGET_POS,
	HEIGHT
};

// 191 is 6' (structures of height 3)
// 127 is 4' (structures of height 2)
//  63 is 2' (structures of height 1)


#define STANDING_HEIGHT			191.0f
#define STANDING_LOS_POS			175.0f
#define STANDING_FIRING_POS			175.0f
#define STANDING_HEAD_TARGET_POS		175.0f
#define STANDING_HEAD_BOTTOM_POS		159.0f
#define STANDING_TORSO_TARGET_POS		127.0f
#define STANDING_TORSO_BOTTOM_POS		95.0f
#define STANDING_LEGS_TARGET_POS		47.0f
#define STANDING_TARGET_POS			STANDING_HEAD_TARGET_POS

#define CROUCHED_HEIGHT			130.0f
#define CROUCHED_LOS_POS			111.0f
#define CROUCHED_FIRING_POS			111.0f

#define CROUCHED_HEAD_TARGET_POS		111.0f
#define CROUCHED_HEAD_BOTTOM_POS		95.0f
#define CROUCHED_TORSO_TARGET_POS		71.0f
#define CROUCHED_TORSO_BOTTOM_POS		47.0f
#define CROUCHED_LEGS_TARGET_POS		31.0f
#define CROUCHED_TARGET_POS			CROUCHED_HEAD_TARGET_POS

#define PRONE_HEIGHT				63.0f
#define PRONE_LOS_POS				31.0f
#define PRONE_FIRING_POS			31.0f
#define PRONE_TORSO_TARGET_POS			31.0f
#define PRONE_HEAD_TARGET_POS			31.0f
#define PRONE_LEGS_TARGET_POS			31.0f
#define PRONE_TARGET_POS			PRONE_HEAD_TARGET_POS

#define WALL_HEIGHT_UNITS			HEIGHT_UNITS
#define WINDOW_BOTTOM_HEIGHT_UNITS		87
#define WINDOW_TOP_HEIGHT_UNITS		220

#define CLOSE_TO_FIRER				25
#define VERY_CLOSE_TO_FIRER			21

#ifdef LOS_DEBUG
struct LOSResults
{
	BOOLEAN fLOSTestPerformed;
	BOOLEAN fLOSClear;
	BOOLEAN fOutOfRange;
	INT32   iDistance;
	INT32   iMaxDistance;
	UINT8   ubTreeSpotsHit;
	INT32   iStartX;
	INT32   iStartY;
	INT32   iStartZ;
	INT32   iEndX;
	INT32   iEndY;
	INT32   iEndZ;
	INT32   iStoppedX;
	INT32   iStoppedY;
	INT32   iStoppedZ;
	INT32   iCurrCubesZ;
	UINT8   ubChanceToGetThrough;
};

extern LOSResults gLOSTestResults;

#endif

void MoveBullet(BULLET* b);

#endif
