#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "Structure Internals.h"
#include "tiledef.h"
#include "overhead types.h"

#define NOTHING_BLOCKING							0
#define BLOCKING_REDUCE_RANGE					1
#define BLOCKING_NEXT_TILE						10
#define	BLOCKING_TOPLEFT_WINDOW				30
#define	BLOCKING_TOPRIGHT_WINDOW			40
#define	BLOCKING_TOPLEFT_DOOR					50
#define	BLOCKING_TOPRIGHT_DOOR				60
#define FULL_BLOCKING									70
#define BLOCKING_TOPLEFT_OPEN_WINDOW  90
#define BLOCKING_TOPRIGHT_OPEN_WINDOW 100


// ATE: Increased to allow corpses to not collide with soldiers
// 100 == MAX_CORPSES
#define	INVALID_STRUCTURE_ID					( TOTAL_SOLDIERS + 100 )
#define IGNORE_PEOPLE_STRUCTURE_ID		(TOTAL_SOLDIERS+101)

#define STRUCTURE_DAMAGE_EXPLOSION		1
#define STRUCTURE_DAMAGE_GUNFIRE			2


// functions at the structure database level
//
STRUCTURE_FILE_REF * LoadStructureFile( STR szFileName );
void FreeAllStructureFiles( void );
BOOLEAN FreeStructureFile( STRUCTURE_FILE_REF * pStructureFile );

//
// functions at the structure instance level
//
BOOLEAN OkayToAddStructureToWorld( INT16 sBaseGridNo, INT8 bLevel, DB_STRUCTURE_REF * pDBStructureRef, INT16 sExclusionID );

// for the PTR argument of AddStructureToWorld, pass in a LEVELNODE * please!
BOOLEAN AddStructureToWorld( INT16 sBaseGridNo, INT8 bLevel, DB_STRUCTURE_REF * pDBStructureRef, PTR pLevelN );
BOOLEAN DeleteStructureFromWorld( STRUCTURE * pStructure );

//
// functions to find a structure in a location
//
STRUCTURE * FindStructure( INT16 sGridNo, UINT32 fFlags );
STRUCTURE * FindNextStructure( STRUCTURE * pStructure, UINT32 fFlags );
STRUCTURE * FindStructureByID( INT16 sGridNo, UINT16 usStructureID );
STRUCTURE * FindBaseStructure( STRUCTURE * pStructure );
STRUCTURE * FindNonBaseStructure( INT16 sGridNo, STRUCTURE * pStructure );

//
// functions related to interactive tiles
//
STRUCTURE * SwapStructureForPartner( INT16 sGridNo, STRUCTURE * pStructure );
STRUCTURE * SwapStructureForPartnerWithoutTriggeringSwitches( INT16 sGridNo, STRUCTURE * pStructure );
STRUCTURE * SwapStructureForPartnerAndStoreChangeInMap( INT16 sGridNo, STRUCTURE * pStructure );
//
// functions useful for AI that return info about heights
//
INT8 StructureHeight( STRUCTURE * pStructure );
INT8 StructureBottomLevel( STRUCTURE * pStructure );
INT8 GetTallestStructureHeight( INT16 sGridNo, BOOLEAN fOnRoof );
INT8 GetStructureTargetHeight( INT16 sGridNo, BOOLEAN fOnRoof );

BOOLEAN StructureDensity( STRUCTURE * pStructure, UINT8 * pubLevel0, UINT8 * pubLevel1, UINT8 * pubLevel2, UINT8 * pubLevel3 );

BOOLEAN FindAndSwapStructure( INT16 sGridNo );
INT16 GetBaseTile( STRUCTURE * pStructure );
//
// functions to work with the editor undo code
//
void DeleteStructureFromTile( PTR pMapElement, STRUCTURE * pStructure );

void DebugStructurePage1( void );

BOOLEAN AddZStripInfoToVObject( HVOBJECT hVObject, STRUCTURE_FILE_REF * pStructureFileRef, BOOLEAN fFromAnimation, INT16 sSTIStartIndex );

// FUNCTIONS FOR DETERMINING STUFF THAT BLOCKS VIEW FOR TILE_bASED LOS
INT8 GetBlockingStructureInfo( INT16 sGridNo, INT8 bDir, INT8 bNextDir, INT8 bLevel, INT8 *pStructHeight, STRUCTURE ** ppTallestStructure, BOOLEAN fWallsBlock );

BOOLEAN DamageStructure( STRUCTURE * pStructure, UINT8 ubDamage, UINT8 ubReason, INT16 sGridNo, INT16 sX, INT16 sY, UINT8 ubOwner );

// Material armour type enumeration
enum
{
	MATERIAL_NOTHING,
	MATERIAL_WOOD_WALL,
	MATERIAL_PLYWOOD_WALL,
	MATERIAL_LIVE_WOOD,
	MATERIAL_LIGHT_VEGETATION,
	MATERIAL_FURNITURE,
	MATERIAL_PORCELAIN,
	MATERIAL_CACTUS,
	MATERIAL_NOTUSED1,
	MATERIAL_NOTUSED2,

	MATERIAL_NOTUSED3,
	MATERIAL_STONE,
	MATERIAL_CONCRETE1,
	MATERIAL_CONCRETE2,
	MATERIAL_ROCK,
	MATERIAL_RUBBER,
	MATERIAL_SAND,
	MATERIAL_CLOTH,
	MATERIAL_SANDBAG,
	MATERIAL_NOTUSED5,

	MATERIAL_NOTUSED6,
	MATERIAL_LIGHT_METAL,
	MATERIAL_THICKER_METAL,
	MATERIAL_HEAVY_METAL,
	MATERIAL_INDESTRUCTABLE_STONE,
	MATERIAL_INDESTRUCTABLE_METAL,
	MATERIAL_THICKER_METAL_WITH_SCREEN_WINDOWS,
	NUM_MATERIAL_TYPES
};

extern INT32 guiMaterialHitSound[ NUM_MATERIAL_TYPES ];

STRUCTURE		*FindStructureBySavedInfo( INT16 sGridNo, UINT8 ubType, UINT8 ubWallOrientation, INT8 bLevel );
UINT32			StructureTypeToFlag( UINT8 ubType );
UINT8				StructureFlagToType( UINT32 uiFlag );

UINT32 GetStructureOpenSound( STRUCTURE *pStructure, BOOLEAN fClose );

#endif
