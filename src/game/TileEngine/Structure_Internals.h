#ifndef __STRUCTURE_INTERNAL_H
#define __STRUCTURE_INTERNAL_H

//
// If you wish to use the structure database functions, include
// structure_extern.h, not structure.h!
//

#include "JA2Types.h"
#include "SGPFile.h"
#include <cstddef>
#include <vector>

// A few words about the overall structure scheme:
//
// Large structures are split into multiple sections,
// one for each tile.
//
// Each section is treated as a separate object,
// except that it does NOT record information about
// hit points, but instead stores a pointer to the
// base object (section).
//
// Each section has a line of sight profile.  These
// profiles are split into 5 in each horizontal direction
// and 4 vertically, forming 100 "cubes".  In real
// world terms, each section represents a volume
// with a height of 8 feet (and width and length
// of what?)
//
// It is important to note that the vertical
// position of each section is measured in individual
// cubes (rather than, as it were, groups of 4 vertical
// cubes)

#define PROFILE_X_SIZE 5
#define PROFILE_Y_SIZE 5
#define PROFILE_Z_SIZE 4

// these values should be compared for less than rather than less
// than or equal to
#define STRUCTURE_ON_GROUND 0
#define STRUCTURE_ON_ROOF PROFILE_Z_SIZE
#define STRUCTURE_ON_GROUND_MAX PROFILE_Z_SIZE
#define STRUCTURE_ON_ROOF_MAX PROFILE_Z_SIZE * 2

using PROFILE = const UINT8[PROFILE_X_SIZE][PROFILE_Y_SIZE];

constexpr UINT8 AtHeight[PROFILE_Z_SIZE] { 0x01, 0x02, 0x04, 0x08 };

// MAP_ELEMENT may get later:
// PROFILE *		CombinedLOSProfile;
// PROFILE *		CombinedProtectionProfile;
//
// LEVELNODE gets a pointer to a STRUCTURE or
// a union between its soldier pointer and a
// STRUCTURE pointer

// if (fFlags & STRUCTURE_BASE_TILE)
// then the structure is the "base" of the object
// and its hitpoint value is the one for the object
//
//    vv       generic flags for all structures
//    vvv      type flags
//

// how to handle explodable structures

enum StructureFlags : UINT32
{
	// NOT used in DB structures
	STRUCTURE_BASE_TILE     = 0x00000001,
	STRUCTURE_OPEN          = 0x00000002,
	STRUCTURE_OPENABLE      = 0x00000004,
	STRUCTURE_GARAGEDOOR    = 0x00000008,

	STRUCTURE_MOBILE        = 0x00000010,
	/* STRUCTURE_PASSABLE is set for each structure instance where the tile flag
	 * TILE_PASSABLE is set */
	STRUCTURE_PASSABLE      = 0x00000020,
	STRUCTURE_EXPLOSIVE     = 0x00000040,
	STRUCTURE_TRANSPARENT   = 0x00000080,

	STRUCTURE_GENERIC       = 0x00000100,
	STRUCTURE_TREE          = 0x00000200,
	STRUCTURE_FENCE         = 0x00000400,
	STRUCTURE_WIREFENCE     = 0x00000800,

	STRUCTURE_HASITEMONTOP  = 0x00001000, // ATE: struct has item on top of it
	STRUCTURE_SPECIAL       = 0x00002000,
	STRUCTURE_LIGHTSOURCE   = 0x00004000,
	STRUCTURE_VEHICLE       = 0x00008000,

	STRUCTURE_WALL          = 0x00010000,
	STRUCTURE_WALLNWINDOW   = 0x00020000,
	STRUCTURE_SLIDINGDOOR   = 0x00040000,
	STRUCTURE_DOOR          = 0x00080000,

	/* a "multi" structure (as opposed to multitiled) is composed of multiple
	 * graphics & structures */
	STRUCTURE_MULTI         = 0x00100000,
	STRUCTURE_CAVEWALL      = 0x00200000,
	STRUCTURE_DDOOR_LEFT    = 0x00400000,
	STRUCTURE_DDOOR_RIGHT   = 0x00800000,

	STRUCTURE_NORMAL_ROOF   = 0x01000000,
	STRUCTURE_SLANTED_ROOF  = 0x02000000,
	STRUCTURE_TALL_ROOF     = 0x04000000,
	STRUCTURE_SWITCH        = 0x08000000,

	STRUCTURE_ON_LEFT_WALL  = 0x10000000,
	STRUCTURE_ON_RIGHT_WALL = 0x20000000,
	STRUCTURE_CORPSE        = 0x40000000,
	STRUCTURE_PERSON        = 0x80000000,

// Combination flags
	STRUCTURE_ANYFENCE      = 0x00000C00,
	STRUCTURE_ANYDOOR       = 0x00CC0008,
	STRUCTURE_OBSTACLE      = 0x00008F00,
	STRUCTURE_WALLSTUFF     = 0x00CF0008,
	STRUCTURE_BLOCKSMOVES   = 0x00208F00,
	STRUCTURE_TYPE_DEFINED  = 0x8FEF8F08,
	STRUCTURE_ROOF          = 0x07000000
};
ENUM_BITSET(StructureFlags)

#define TILE_ON_ROOF						0x01
#define TILE_PASSABLE						0x02

struct DB_STRUCTURE_TILE
{
	INT16			sPosRelToBase;  // "single-axis"
	INT8			bXPosRelToBase;
	INT8			bYPosRelToBase;
	PROFILE		Shape;					// 25 bytes
	UINT8			fFlags;
	UINT8			ubVehicleHitLocation;
	BYTE			bUnused[1]; // XXX HACK000B
}; // 32 bytes

#define BASE_TILE 0

#define NO_PARTNER_STRUCTURE 0

struct DB_STRUCTURE
{
	UINT8								ubArmour;
	UINT8								ubHitPoints;
	UINT8								ubDensity;
	UINT8								ubNumberOfTiles;
	StructureFlags					    fFlags;
	UINT16							usStructureNumber;
	UINT8								ubWallOrientation;
	INT8								bDestructionPartner; // >0 = debris number (bDP - 1), <0 = partner graphic
	INT8								bPartnerDelta; // opened/closed version, etc... 0 for unused
	INT8								bZTileOffsetX;
	INT8								bZTileOffsetY;
	BYTE								bUnused[1]; // XXX HACK000B
}; // 16 bytes

struct DB_STRUCTURE_REF
{
	DB_STRUCTURE * 												pDBStructure;
	DB_STRUCTURE_TILE **									ppTile; // dynamic array
};

struct STRUCTURE
{
	STRUCTURE*                    pPrev;
	STRUCTURE*                    pNext;
	const DB_STRUCTURE_REF*       pDBStructureRef;
	PROFILE*					  pShape;
	StructureFlags				  fFlags; // need to have something to indicate base tile/not
	GridNo						  sGridNo;
	UINT16						  usStructureID;
	UINT8						  ubHitPoints;
	UINT8						  ubLockStrength;
	GridNo						  sBaseGridNo;
	INT16						  sCubeOffset;// height of bottom of object in profile "cubes"
	UINT8						  ubWallOrientation;
	UINT8						  ubStructureHeight; // if 0, then unset; otherwise stores height of structure when last calculated
};

struct STRUCTURE_FILE_REF
{
	std::vector<AuxObjectData>    pAuxData;
	std::vector<RelTileLoc>       pTileLocData;
	std::vector<std::byte>        pubStructureData;
	std::vector<DB_STRUCTURE_REF> pDBStructureRef;
	UINT16                        usNumberOfStructures{ 0 };
	UINT16                        usNumberOfStructuresStored{ 0 };

	STRUCTURE_FILE_REF(SGPFile & jsdFile);
	~STRUCTURE_FILE_REF();
};

#endif
