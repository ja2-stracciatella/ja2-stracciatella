#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "AutoObj.h"
#include "JA2Types.h"
#include "Structure_Internals.h"
#include "Overhead_Types.h"
#include "Sound_Control.h"

#define NOTHING_BLOCKING			0
#define BLOCKING_REDUCE_RANGE			1
#define BLOCKING_NEXT_TILE			10
#define BLOCKING_TOPLEFT_WINDOW			30
#define BLOCKING_TOPRIGHT_WINDOW		40
#define BLOCKING_TOPLEFT_DOOR			50
#define BLOCKING_TOPRIGHT_DOOR			60
#define FULL_BLOCKING				70
#define BLOCKING_TOPLEFT_OPEN_WINDOW		90
#define BLOCKING_TOPRIGHT_OPEN_WINDOW		100

enum StructureDamageResult
{
    STRUCTURE_NOT_DAMAGED, // structure not damaged
    STRUCTURE_DESTROYED,   // structure to be deleted
    STRUCTURE_DAMAGED      // structure to be replaced with damaged graphics
};

// ATE: Increased to allow corpses to not collide with soldiers
// 200 == MAX_ROTTING_CORPSES, see Rotting_Corpses.h
constexpr UINT16 INVALID_STRUCTURE_ID = TOTAL_SOLDIERS + 200;
constexpr UINT16 IGNORE_PEOPLE_STRUCTURE_ID = INVALID_STRUCTURE_ID + 1;

enum StructureDamageReason
{
	STRUCTURE_DAMAGE_EXPLOSION = 1,
	STRUCTURE_DAMAGE_GUNFIRE   = 2
};


// functions at the structure database level
STRUCTURE_FILE_REF* LoadStructureFile(const char* szFileName);
void FreeAllStructureFiles( void );
void FreeStructureFile(STRUCTURE_FILE_REF*);

//
// functions at the structure instance level
//
BOOLEAN OkayToAddStructureToWorld(INT16 sBaseGridNo, INT8 bLevel, const DB_STRUCTURE_REF* pDBStructureRef, INT16 sExclusionID);
BOOLEAN InternalOkayToAddStructureToWorld(INT16 sBaseGridNo, INT8 bLevel, const DB_STRUCTURE_REF* pDBStructureRef, INT16 sExclusionID, BOOLEAN fIgnorePeople);

STRUCTURE* AddStructureToWorld(INT16 base_grid_no, INT8 level, DB_STRUCTURE_REF const*, LEVELNODE*);
BOOLEAN DeleteStructureFromWorld( STRUCTURE * pStructure );

//
// functions to find a structure in a location
//

// Finds a structure that matches any of the given flags
STRUCTURE* FindStructure(INT16 sGridNo, StructureFlags);

STRUCTURE* FindNextStructure(const STRUCTURE* s, StructureFlags);
STRUCTURE* FindStructureByID(INT16 sGridNo, UINT16 structure_id);

#define FOR_EACH_STRUCTURE(iter, grid_no, flags) \
	for (STRUCTURE* iter = FindStructure((grid_no), (flags)); iter; iter = FindNextStructure(iter, (flags)))

// Finds the base structure for any structure
STRUCTURE* FindBaseStructure(STRUCTURE* s);

//
// functions related to interactive tiles
//
STRUCTURE* SwapStructureForPartner(STRUCTURE*);
STRUCTURE* SwapStructureForPartnerAndStoreChangeInMap(STRUCTURE*);
//
// functions useful for AI that return info about heights
//
INT8 StructureHeight( STRUCTURE * pStructure );
INT8 StructureBottomLevel( STRUCTURE * pStructure );
INT8 GetTallestStructureHeight( INT16 sGridNo, BOOLEAN fOnRoof );
INT8 GetStructureTargetHeight( INT16 sGridNo, BOOLEAN fOnRoof );

BOOLEAN StructureDensity( STRUCTURE * pStructure, UINT8 * pubLevel0, UINT8 * pubLevel1, UINT8 * pubLevel2, UINT8 * pubLevel3 );

BOOLEAN FindAndSwapStructure( INT16 sGridNo );
//
// functions to work with the editor undo code
//

void DebugStructurePage1( void );

void AddZStripInfoToVObject(HVOBJECT, STRUCTURE_FILE_REF const*, BOOLEAN fFromAnimation, INT16 sSTIStartIndex);

// FUNCTIONS FOR DETERMINING STUFF THAT BLOCKS VIEW FOR TILE_bASED LOS
INT8 GetBlockingStructureInfo( INT16 sGridNo, INT8 bDir, INT8 bNextDir, INT8 bLevel, INT8 *pStructHeight, STRUCTURE ** ppTallestStructure, BOOLEAN fWallsBlock );

StructureDamageResult DamageStructure(STRUCTURE*, UINT8 damage, StructureDamageReason, GridNo, INT16 x, INT16 y, SOLDIERTYPE* owner);

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

STRUCTURE* FindStructureBySavedInfo(GridNo, UINT8 type, UINT8 wall_orientation, INT8 level);
UINT8				StructureFlagToType( UINT32 uiFlag );

SoundID GetStructureOpenSound(STRUCTURE const*, bool closing);

extern const UINT8 gubMaterialArmour[];

typedef SGP::AutoObj<STRUCTURE_FILE_REF, FreeStructureFile> AutoStructureFileRef;

#endif
