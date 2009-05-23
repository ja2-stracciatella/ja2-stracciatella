#include <stdexcept>

#include "Buffer.h"
#include "HImage.h"
#include "Soldier_Control.h"
#include "Types.h"
#include "VObject.h"
#include "WCheck.h"
#include "Debug.h"
#include "FileMan.h"
#include "MemMan.h"
#include "Structure.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "Font.h"
#include "Font_Control.h"
#include "LOS.h"
#include "Smell.h"
#include "SaveLoadMap.h"
#include "StrategicMap.h"
#include "Sys_Globals.h"	//for access to gfEditMode flag
//Kris:
#ifdef JA2EDITOR
#	include "Editor_Undo.h" //for access to AddToUndoList( iMapIndex )
#endif
#include "Explosion_Control.h"
#include "Buildings.h"
#include "Random.h"
#include "Tile_Animation.h"


#ifdef COUNT_PATHS
	extern UINT32 guiSuccessfulPathChecks;
	extern UINT32 guiTotalPathChecks;
	extern UINT32 guiFailedPathChecks;
	extern UINT32 guiUnsuccessfulPathChecks;
#endif

/*
 * NB:  STRUCTURE_SPECIAL
 *
 * Means different things depending on the context.
 *
 * WALLNWINDOW SPECIAL - opaque to sight
 * MULTI SPECIAL - second level (damaged) MULTI structure, should only be deleted if
 *    starting with the deletion of a MULTI SPECIAL structure
 */

UINT8 AtHeight[PROFILE_Z_SIZE] = { 0x01, 0x02, 0x04, 0x08 };

#define FIRST_AVAILABLE_STRUCTURE_ID (INVALID_STRUCTURE_ID + 2)

static UINT16 gusNextAvailableStructureID = FIRST_AVAILABLE_STRUCTURE_ID;

static STRUCTURE_FILE_REF* gpStructureFileRefs;


static SoundID const guiMaterialHitSound[NUM_MATERIAL_TYPES] =
{
	NO_SOUND,
	S_WOOD_IMPACT1,
	S_WOOD_IMPACT2,
	S_WOOD_IMPACT3,
	S_VEG_IMPACT1,
	NO_SOUND,
	S_PORCELAIN_IMPACT1,
	NO_SOUND,
	NO_SOUND,
	NO_SOUND,

	NO_SOUND,
	S_STONE_IMPACT1,
	S_STONE_IMPACT1,
	S_STONE_IMPACT1,
	S_STONE_IMPACT1,
	S_RUBBER_IMPACT1,
	NO_SOUND,
	NO_SOUND,
	NO_SOUND,
	NO_SOUND,

	NO_SOUND,
	S_METAL_IMPACT1,
	S_METAL_IMPACT2,
	S_METAL_IMPACT3,
	S_STONE_IMPACT1,
	S_METAL_IMPACT3,
};


/*
index  1-10, organics
index 11-20, rocks and concretes
index 21-30, metals

index 1, dry timber
index 2, furniture wood
index 3, tree wood
index 11, stone masonry
index 12, non-reinforced concrete
index 13, reinforced concrete
index 14, rock
index 21, light metal (furniture)
index 22, heavy metal (doors etc)
index 23, really heavy metal
index 24, indestructable stone
index 25, indestructable metal
*/
const UINT8 gubMaterialArmour[] =
{ // note: must increase; r.c. should block *AP* 7.62mm rounds
	 0,		// nothing
	25,		// dry timber; wood wall +1/2
	20,		// furniture wood (thin!) or plywood wall +1/2
	30,		// wood (live); 1.5x timber
	 3,		// light vegetation
	10,		// upholstered furniture
	47,		// porcelain
	10,		// cactus, hay, bamboo
		0,
		0,
		0,
	55,		// stone masonry; 3x timber
	63,		// non-reinforced concrete; 4x timber???
  70,		// reinforced concrete; 6x timber
  85,		// rock? - number invented
	 9,		// rubber - tires
	40,		// sand
		1,	// cloth
	40,		// sandbag
		0,
		0,
	37,		// light metal (furniture; NB thin!)
	57,		// thicker metal (dumpster)
  85,		// heavy metal (vault doors) - block everything
	// note that vehicle armour will probably end up in here
	127,	// rock indestructable
	127,	// indestructable
	57,		// like 22 but with screen windows
};


// Function operating on a structure tile
static UINT8 FilledTilePositions(DB_STRUCTURE_TILE const* const t)
{
	// Loop through all parts of a structure and add up the number of filled spots
	UINT8 filled = 0;
	for (INT8 x = 0; x != PROFILE_X_SIZE; ++x)
	{
		for (INT8 y = 0; y != PROFILE_Y_SIZE; ++y)
		{
			UINT8 const shape_value = t->Shape[x][y];
			for (INT8 z = 0; z != PROFILE_Z_SIZE; ++z)
			{
				if (shape_value & AtHeight[z]) ++filled;
			}
		}
	}
	return filled;
}

//
// Structure database functions
//
namespace
{
	/* Free all of the memory associated with a file reference, including the file
	 * reference structure itself */
	void FreeStructureFileRef(STRUCTURE_FILE_REF* const f)
	{
		if (DB_STRUCTURE_REF* const sr = f->pDBStructureRef)
		{
			DB_STRUCTURE_REF const* const end = sr + f->usNumberOfStructures;
			for (DB_STRUCTURE_REF* i = sr; i != end; ++i)
			{
				if (i->ppTile) MemFree(i->ppTile);
			}
			MemFree(sr);
		}
		if (f->pubStructureData) MemFree(f->pubStructureData);
		if (f->pAuxData)
		{
			MemFree(f->pAuxData);
			if (f->pTileLocData) MemFree(f->pTileLocData);
		}
		MemFree(f);
	}
}


void FreeAllStructureFiles()
{ // Free all of the structure database
	STRUCTURE_FILE_REF* next;
	for (STRUCTURE_FILE_REF* i = gpStructureFileRefs; i; i = next)
	{
		next = i->pNext;
		FreeStructureFileRef(i);
	}
}


void FreeStructureFile(STRUCTURE_FILE_REF* const sfr)
{
	CHECKV(sfr);

	STRUCTURE_FILE_REF* const next = sfr->pNext;
	STRUCTURE_FILE_REF* const prev = sfr->pPrev;
	Assert((prev == NULL) == (gpStructureFileRefs == sfr));
	*(prev != NULL ? &prev->pNext : &gpStructureFileRefs) = next;
	if (next) next->pPrev = prev;

	FreeStructureFileRef(sfr);
}


/* IMPORTANT THING TO REMEMBER
 * Although the number of structures and images about which information may be
 * stored in a file, the two are stored very differently.
 * The structure data stored amounts to a sparse array, with no data saved for
 * any structures that are not defined.
 * For image information, however, an array is stored with every entry filled
 * regardless of whether there is non-zero data defined for that graphic! */
struct STRUCTURE_FILE_HEADER
{
	char szId[4];
	union
	{
		UINT16 usNumberOfStructures;
		UINT16 usNumberOfImages;
	};
	UINT16 usNumberOfStructuresStored;
	UINT16 usStructureDataSize;
	UINT8  fFlags;
	UINT8  bUnused[3]; // XXX HACK000B
	UINT16 usNumberOfImageTileLocsStored;
};
CASSERT(sizeof(STRUCTURE_FILE_HEADER) == 16)

// "J2SD" = Jagged 2 Structure Data
#define STRUCTURE_FILE_ID "J2SD"
#define STRUCTURE_FILE_ID_LEN 4


// Loads a structure file's data as a honking chunk o' memory
static void LoadStructureData(const char* const filename, STRUCTURE_FILE_REF* const sfr, UINT32* const structure_data_size)
{
	AutoSGPFile f(FileOpen(filename, FILE_ACCESS_READ));

	STRUCTURE_FILE_HEADER header;
	FileRead(f, &header, sizeof(STRUCTURE_FILE_HEADER));
	if (strncmp(header.szId, STRUCTURE_FILE_ID, STRUCTURE_FILE_ID_LEN) != 0 ||
			header.usNumberOfStructures == 0)
	{
		throw std::runtime_error("Failed to load structure file, because header is invalid");
	}

	SGP::Buffer<AuxObjectData> aux_data;
	SGP::Buffer<RelTileLoc>    tile_loc_data;
	sfr->usNumberOfStructures = header.usNumberOfStructures;
	if (header.fFlags & STRUCTURE_FILE_CONTAINS_AUXIMAGEDATA)
	{
		const UINT16 n_images = header.usNumberOfImages;
		aux_data.Allocate(n_images);
		FileRead(f, aux_data, sizeof(*aux_data) * n_images);

		const UINT16 n_tile_locs = header.usNumberOfImageTileLocsStored;
		if (n_tile_locs > 0)
		{
			tile_loc_data.Allocate(n_tile_locs);
			FileRead(f, tile_loc_data, sizeof(*tile_loc_data) * n_tile_locs);
		}
	}

	SGP::Buffer<UINT8> structure_data;
	if (header.fFlags & STRUCTURE_FILE_CONTAINS_STRUCTUREDATA)
	{
		sfr->usNumberOfStructuresStored = header.usNumberOfStructuresStored;
		const UINT16 data_size = header.usStructureDataSize;
		structure_data.Allocate(data_size);
		FileRead(f, structure_data, data_size);

		*structure_data_size = data_size;
	}

	sfr->pAuxData         = aux_data.Release();
	sfr->pTileLocData     = tile_loc_data.Release();
	sfr->pubStructureData = structure_data.Release();
}


static void CreateFileStructureArrays(STRUCTURE_FILE_REF* const pFileRef, UINT32 uiDataSize)
{ /* Based on a file chunk, creates all the dynamic arrays for the structure
	 * definitions contained within */
	UINT8*                  pCurrent        = pFileRef->pubStructureData;
	DB_STRUCTURE_REF* const pDBStructureRef = MALLOCNZ(DB_STRUCTURE_REF, pFileRef->usNumberOfStructures);
	pFileRef->pDBStructureRef = pDBStructureRef;
	for (UINT16 usLoop = 0; usLoop < pFileRef->usNumberOfStructuresStored; ++usLoop)
	{
		if (uiDataSize < sizeof(DB_STRUCTURE))
		{	// gone past end of file block?!
			// freeing of memory will occur outside of the function
			throw std::runtime_error("Failed to create structure arrays, because input data is too short");
		}
		DB_STRUCTURE* const dbs = (DB_STRUCTURE*)pCurrent;
		pCurrent   += sizeof(DB_STRUCTURE);
		uiDataSize -= sizeof(DB_STRUCTURE);

		DB_STRUCTURE_TILE** const tiles       = MALLOCN(DB_STRUCTURE_TILE*, dbs->ubNumberOfTiles);
		UINT16              const usIndex     = dbs->usStructureNumber;
		pDBStructureRef[usIndex].pDBStructure = dbs;
		pDBStructureRef[usIndex].ppTile       = tiles;

		// Set things up to calculate hit points
		UINT32 uiHitPoints = 0;
		for (UINT16 usTileLoop = 0; usTileLoop < dbs->ubNumberOfTiles; ++usTileLoop)
		{
			if (uiDataSize < sizeof(DB_STRUCTURE_TILE))
			{	// gone past end of file block?!
				// freeing of memory will occur outside of the function
				throw std::runtime_error("Failed to create structure arrays, because input data is too short");
			}
			DB_STRUCTURE_TILE* const tile = (DB_STRUCTURE_TILE*)pCurrent;
			pCurrent   += sizeof(DB_STRUCTURE_TILE);
			uiDataSize -= sizeof(DB_STRUCTURE_TILE);

			tiles[usTileLoop] = tile;
			// set the single-value relative position between this tile and the base tile
			tile->sPosRelToBase = tile->bXPosRelToBase + tile->bYPosRelToBase * WORLD_COLS;
			uiHitPoints += FilledTilePositions(tile);
		}
		// scale hit points down to something reasonable...
		uiHitPoints = uiHitPoints * 100 / 255;
		dbs->ubHitPoints = (UINT8)uiHitPoints;
	}
}


STRUCTURE_FILE_REF* LoadStructureFile(const char* szFileName)
{ // NB should be passed in expected number of structures so we can check equality
	UINT32								uiDataSize = 0;

	SGP::AutoObj<STRUCTURE_FILE_REF, FreeStructureFileRef> sfr(MALLOCZ(STRUCTURE_FILE_REF));
	LoadStructureData(szFileName, sfr, &uiDataSize);
	if (sfr->pubStructureData) CreateFileStructureArrays(sfr, uiDataSize);
	// Add the file reference to the master list, at the head for convenience
	if (gpStructureFileRefs != NULL)
	{
		gpStructureFileRefs->pPrev = sfr;
	}
	sfr->pNext = gpStructureFileRefs;
	gpStructureFileRefs = sfr;
	return sfr.Release();
}


//
// Structure creation functions
//


static STRUCTURE* CreateStructureFromDB(DB_STRUCTURE_REF const* const pDBStructureRef, UINT8 const ubTileNum)
{ // Creates a STRUCTURE struct for one tile of a structure
	DB_STRUCTURE const* const pDBStructure = pDBStructureRef->pDBStructure;
	DB_STRUCTURE_TILE*  const pTile        = pDBStructureRef->ppTile[ubTileNum];

	STRUCTURE* const pStructure = MALLOCZ(STRUCTURE);

	pStructure->fFlags          = pDBStructure->fFlags;
	pStructure->pShape          = &pTile->Shape;
	pStructure->pDBStructureRef = pDBStructureRef;
	if (pTile->sPosRelToBase == 0)
	{	// base tile
		pStructure->fFlags      |= STRUCTURE_BASE_TILE;
		pStructure->ubHitPoints  = pDBStructure->ubHitPoints;
	}
	if (pDBStructure->ubWallOrientation != NO_ORIENTATION)
	{
		/* for multi-tile walls, which are only the special corner pieces, the
		 * non-base tile gets no orientation value because this copy will be
		 * skipped */
		if (!(pStructure->fFlags & STRUCTURE_WALL) ||
				pStructure->fFlags & STRUCTURE_BASE_TILE)
		{
			pStructure->ubWallOrientation = pDBStructure->ubWallOrientation;
		}
	}
	pStructure->ubVehicleHitLocation = pTile->ubVehicleHitLocation;
	return pStructure;
}


static BOOLEAN OkayToAddStructureToTile(INT16 const sBaseGridNo, INT16 const sCubeOffset, DB_STRUCTURE_REF const* const pDBStructureRef, UINT8 ubTileIndex, INT16 const sExclusionID, BOOLEAN const fIgnorePeople)
{ // Verifies whether a structure is blocked from being added to the map at a particular point
	DB_STRUCTURE_TILE const* const* const ppTile = pDBStructureRef->ppTile;
	INT16 const sGridNo = sBaseGridNo + ppTile[ubTileIndex]->sPosRelToBase;
	if (sGridNo < 0 || WORLD_MAX < sGridNo) return FALSE;

	if (gpWorldLevelData[sBaseGridNo].sHeight != gpWorldLevelData[sGridNo].sHeight)
	{
		// uneven terrain, one portion on top of cliff and another not! can't add!
		return FALSE;
	}

	DB_STRUCTURE const* const pDBStructure = pDBStructureRef->pDBStructure;
	for (STRUCTURE const* pExistingStructure = gpWorldLevelData[sGridNo].pStructureHead; pExistingStructure != NULL; pExistingStructure = pExistingStructure->pNext)
	{
		if (sCubeOffset != pExistingStructure->sCubeOffset) continue;

		// CJC:
		// If adding a mobile structure, allow addition if existing structure is passable
		if (pDBStructure->fFlags & STRUCTURE_MOBILE && pExistingStructure->fFlags & STRUCTURE_PASSABLE)
		{
			continue;
		}

		if (pDBStructure->fFlags & STRUCTURE_OBSTACLE)
		{
			// CJC: NB these next two if states are probably COMPLETELY OBSOLETE but I'm leaving
			// them in there for now (no harm done)

			// ATE:
			// ignore this one if it has the same ID num as exclusion
			if (sExclusionID != INVALID_STRUCTURE_ID &&
					sExclusionID == pExistingStructure->usStructureID)
			{
				continue;
			}

			// If we are a person, skip!
			if (fIgnorePeople && pExistingStructure->usStructureID < TOTAL_SOLDIERS)
			{
				continue;
			}

			// two obstacle structures aren't allowed in the same tile at the same height
			// ATE: There is more sophisticated logic for mobiles, so postpone this check if mobile....
			if (pExistingStructure->fFlags & STRUCTURE_OBSTACLE && !(pDBStructure->fFlags & STRUCTURE_MOBILE))
			{
				if (pExistingStructure->fFlags & STRUCTURE_PASSABLE && !(pExistingStructure->fFlags & STRUCTURE_MOBILE))
				{
					// no mobiles, existing structure is passable
				}
				else
				{
					return FALSE;
				}
			}
			else if (pDBStructure->ubNumberOfTiles > 1 && pExistingStructure->fFlags & STRUCTURE_WALLSTUFF)
			{
				// if not an open door...
				if (!(pExistingStructure->fFlags & STRUCTURE_ANYDOOR) ||
						!(pExistingStructure->fFlags & STRUCTURE_OPEN))
				{
					// we could be trying to place a multi-tile obstacle on top of a wall; we shouldn't
					// allow this if the structure is going to be on both sides of the wall
					for (INT8 bLoop = 1; bLoop < 4; ++bLoop)
					{
						INT16 sOtherGridNo;
						switch (pExistingStructure->ubWallOrientation)
						{
							case OUTSIDE_TOP_LEFT:
							case INSIDE_TOP_LEFT:
								sOtherGridNo = NewGridNo(sGridNo, DirectionInc((INT8)(bLoop + 2)));
								break;

							case OUTSIDE_TOP_RIGHT:
							case INSIDE_TOP_RIGHT:
								sOtherGridNo = NewGridNo(sGridNo, DirectionInc(bLoop));
								break;

							default:
								// @%?@#%?@%
								sOtherGridNo = NewGridNo(sGridNo, DirectionInc(SOUTHEAST));
								break;
						}
						for (INT8 bLoop2 = 0; bLoop2 < pDBStructure->ubNumberOfTiles; ++bLoop2)
						{
							if (sBaseGridNo + ppTile[bLoop2]->sPosRelToBase != sOtherGridNo) continue;

							// obstacle will straddle wall!
							return FALSE;
						}
					}
				}
			}
		}
		else if (pDBStructure->fFlags & STRUCTURE_WALLSTUFF)
		{
			// two walls with the same alignment aren't allowed in the same tile
			if (pExistingStructure->fFlags & STRUCTURE_WALLSTUFF &&
					pExistingStructure->ubWallOrientation == pDBStructure->ubWallOrientation)
			{
				return FALSE;
			}
			else if (!(pExistingStructure->fFlags & (STRUCTURE_CORPSE | STRUCTURE_PERSON)))
			{
				// it's possible we're trying to insert this wall on top of a multitile obstacle
				for (INT8 bLoop = 1; bLoop < 4; ++bLoop)
				{
					INT16 sOtherGridNo;
					switch (pDBStructure->ubWallOrientation)
					{
						case OUTSIDE_TOP_LEFT:
						case INSIDE_TOP_LEFT:
							sOtherGridNo = NewGridNo(sGridNo, DirectionInc((INT8)(bLoop + 2)));
							break;

						case OUTSIDE_TOP_RIGHT:
						case INSIDE_TOP_RIGHT:
							sOtherGridNo = NewGridNo(sGridNo, DirectionInc(bLoop));
							break;

						default:
							// @%?@#%?@%
							sOtherGridNo = NewGridNo(sGridNo, DirectionInc(SOUTHEAST));
							break;
					}
					for (ubTileIndex = 0; ubTileIndex < pDBStructure->ubNumberOfTiles; ++ubTileIndex)
					{
						STRUCTURE const* const pOtherExistingStructure = FindStructureByID(sOtherGridNo, pExistingStructure->usStructureID);
						if (pOtherExistingStructure) return FALSE;
					}
				}
			}
		}

		if (pDBStructure->fFlags & STRUCTURE_MOBILE)
		{
			// ATE:
			// ignore this one if it has the same ID num as exclusion
			if (sExclusionID != INVALID_STRUCTURE_ID)
			{
				if (pExistingStructure->usStructureID == sExclusionID) continue;
			}

			// If we are a person, skip!
			if (fIgnorePeople && pExistingStructure->usStructureID < TOTAL_SOLDIERS)
			{
				continue;
			}

			// ATE: Added check here - UNLESS the part we are trying to add is PASSABLE!
			if (pExistingStructure->fFlags & STRUCTURE_MOBILE &&
					!(pExistingStructure->fFlags & STRUCTURE_PASSABLE) &&
					!(ppTile[ubTileIndex]->fFlags & TILE_PASSABLE))
			{
				// don't allow 2 people in the same tile
				return FALSE;
			}

			// ATE: Another rule: allow PASSABLE *IF* the PASSABLE is *NOT* MOBILE!
			if (!(pExistingStructure->fFlags & STRUCTURE_MOBILE) &&
					pExistingStructure->fFlags & STRUCTURE_PASSABLE)
			{
				continue;
			}

			// ATE: Added here - UNLESS this part is PASSABLE....
			// two obstacle structures aren't allowed in the same tile at the same height
			if (pExistingStructure->fFlags & STRUCTURE_OBSTACLE &&
					!(ppTile[ubTileIndex]->fFlags & TILE_PASSABLE))
			{
				return FALSE;
			}
		}

		if (pDBStructure->fFlags & STRUCTURE_OPENABLE &&
				pExistingStructure->fFlags & STRUCTURE_OPENABLE)
		{
			/* Don't allow two openable structures in the same tile or things will
			 * screw up on an interface level */
			return FALSE;
		}
	}

	return TRUE;
}


BOOLEAN InternalOkayToAddStructureToWorld(INT16 const sBaseGridNo, INT8 const bLevel, DB_STRUCTURE_REF const* const pDBStructureRef, INT16 const sExclusionID, BOOLEAN const fIgnorePeople)
{
	CHECKF(pDBStructureRef);
	CHECKF(pDBStructureRef->pDBStructure);
	UINT8 const n_tiles = pDBStructureRef->pDBStructure->ubNumberOfTiles;
	CHECKF(n_tiles > 0);
	DB_STRUCTURE_TILE const* const* const tiles = pDBStructureRef->ppTile;
	CHECKF(tiles);

	for (UINT8 i = 0; i < n_tiles; ++i)
	{
		INT16 cube_offset;
		if (!(tiles[i]->fFlags & TILE_ON_ROOF))
		{
			cube_offset = bLevel * PROFILE_Z_SIZE;
		}
		else if (bLevel == 0)
		{
			cube_offset = PROFILE_Z_SIZE;
		}
		else
		{
			return FALSE;
		}

		if (!OkayToAddStructureToTile(sBaseGridNo, cube_offset, pDBStructureRef, i, sExclusionID, fIgnorePeople))
		{
			return FALSE;
		}
	}
	return TRUE;
}


BOOLEAN OkayToAddStructureToWorld(const INT16 sBaseGridNo, const INT8 bLevel, const DB_STRUCTURE_REF* const pDBStructureRef, const INT16 sExclusionID)
{
	return InternalOkayToAddStructureToWorld(sBaseGridNo, bLevel, pDBStructureRef, sExclusionID, sExclusionID == IGNORE_PEOPLE_STRUCTURE_ID);
}


static void AddStructureToTile(MAP_ELEMENT* const pMapElement, STRUCTURE* const pStructure, UINT16 const usStructureID)
{ // adds a STRUCTURE to a MAP_ELEMENT (adds part of a structure to a location on the map)
	STRUCTURE *		pStructureTail;

	pStructureTail = pMapElement->pStructureTail;
	if (pStructureTail == NULL)
	{ // set the head and tail to the new structure
		pMapElement->pStructureHead = pStructure;
	}
	else
	{ // add to the end of the list
		pStructure->pPrev = pStructureTail;
		pStructureTail->pNext = pStructure;
	}
	pMapElement->pStructureTail = pStructure;
	pStructure->usStructureID = usStructureID;
	if (pStructure->fFlags & STRUCTURE_OPENABLE)
	{
		pMapElement->uiFlags |= MAPELEMENT_INTERACTIVETILE;
	}
}


static void DeleteStructureFromTile(MAP_ELEMENT* pMapElement, STRUCTURE* pStructure);


static STRUCTURE* InternalAddStructureToWorld(INT16 const sBaseGridNo, INT8 const bLevel, DB_STRUCTURE_REF const* const pDBStructureRef, LEVELNODE* const pLevelNode)
try
{ // Adds a complete structure to the world at a location plus all other locations covered by the structure
	CHECKN(pDBStructureRef);
	CHECKN(pLevelNode);

	DB_STRUCTURE const* const pDBStructure = pDBStructureRef->pDBStructure;
	CHECKN(pDBStructure);

	DB_STRUCTURE_TILE const* const* const ppTile = pDBStructureRef->ppTile;
	CHECKN(ppTile);

	CHECKN(pDBStructure->ubNumberOfTiles > 0);

	// first check to see if the structure will be blocked
	if (!OkayToAddStructureToWorld(sBaseGridNo, bLevel, pDBStructureRef, INVALID_STRUCTURE_ID))
	{
		return 0;
	}

	/* We go through a definition stage here and a later stage of adding
	 * everything to the world so that we don't have to untangle things if we run
	 * out of memory.  First we create an array of pointers to point to all of the
	 * STRUCTURE elements created in the first stage.  This array gets given to
	 * the base tile so there is an easy way to remove an entire object from the
	 * world quickly */
	SGP::Buffer<STRUCTURE*> structures(pDBStructure->ubNumberOfTiles);

	for (UINT8 i = BASE_TILE; i < pDBStructure->ubNumberOfTiles; ++i)
	{ // for each tile, create the appropriate STRUCTURE struct
		STRUCTURE* s;
		try
		{
			s = CreateStructureFromDB(pDBStructureRef, i);
			structures[i] = s;
		}
		catch (...)
		{
			// Free allocated memory and abort!
			for (UINT8 k = 0; k < i; ++k)
			{
				MemFree(structures[k]);
			}
			return 0;
		}
		DB_STRUCTURE_TILE const* const t = ppTile[i];
		s->sGridNo = sBaseGridNo + t->sPosRelToBase;
		if (i != BASE_TILE)
		{
#if defined JA2EDITOR
			/* Kris:
			 * Added this undo code if in the editor.
			 * It is important to save tiles effected by multitiles.  If the
			 * structure placement fails below, it doesn't matter, because it won't
			 * hurt the undo code. */
			if (gfEditMode) AddToUndoList(s->sGridNo);
#endif
			s->sBaseGridNo = sBaseGridNo;
		}
		s->sCubeOffset =
			(t->fFlags & TILE_ON_ROOF ? bLevel + 1 : bLevel) * PROFILE_Z_SIZE;
		if (t->fFlags & TILE_PASSABLE) s->fFlags |= STRUCTURE_PASSABLE;
		if (pLevelNode->uiFlags & LEVELNODE_SOLDIER)
		{
			// should now be unncessary
			s->fFlags |= STRUCTURE_PERSON;
			s->fFlags &= ~STRUCTURE_BLOCKSMOVES;
		}
		else if (pLevelNode->uiFlags & LEVELNODE_ROTTINGCORPSE || pDBStructure->fFlags & STRUCTURE_CORPSE)
		{
			s->fFlags |= STRUCTURE_CORPSE;
			// attempted check to screen this out for queen creature or vehicle
			if (pDBStructure->ubNumberOfTiles < 10)
			{
				s->fFlags |= STRUCTURE_PASSABLE;
				s->fFlags &= ~STRUCTURE_BLOCKSMOVES;
			}
			else
			{
				// make sure not transparent
				s->fFlags &= ~STRUCTURE_TRANSPARENT;
			}
		}
	}

	UINT16 usStructureID;
	if (pLevelNode->uiFlags & LEVELNODE_SOLDIER)
	{
		// use the merc's ID as the structure ID for his/her structure
		usStructureID = pLevelNode->pSoldier->ubID;
	}
	else if (pLevelNode->uiFlags & LEVELNODE_ROTTINGCORPSE)
	{
    // ATE: Offset IDs so they don't collide with soldiers
		usStructureID = (UINT16)(TOTAL_SOLDIERS + pLevelNode->pAniTile->v.user.uiData);
	}
	else
	{
		gusNextAvailableStructureID++;
		if (gusNextAvailableStructureID == 0)
		{
			// skip past the #s for soldiers' structures and the invalid structure #
			gusNextAvailableStructureID = FIRST_AVAILABLE_STRUCTURE_ID;
		}
		usStructureID = gusNextAvailableStructureID;
	}
	// now add all these to the world!
	INT16 sBaseTileHeight = -1;
	for (UINT8 i = BASE_TILE; i < pDBStructure->ubNumberOfTiles; ++i)
	{
		STRUCTURE*   const s  = structures[i];
		MAP_ELEMENT* const me = &gpWorldLevelData[s->sGridNo];
		if (i == BASE_TILE)
		{
			sBaseTileHeight = me->sHeight;
		}
		else if (me->sHeight != sBaseTileHeight)
		{
			// not level ground! abort!
			for (UINT8 k = BASE_TILE; k < i; ++k)
			{
				STRUCTURE* const s = structures[k];
				DeleteStructureFromTile(&gpWorldLevelData[s->sGridNo], s);
			}
			return 0;
		}
		AddStructureToTile(me, s, usStructureID);
	}

	STRUCTURE* const base = structures[BASE_TILE];
	pLevelNode->pStructureData = base;
	return base;
}
catch (...) { return 0; }


BOOLEAN AddStructureToWorld(const INT16 sBaseGridNo, const INT8 bLevel, const DB_STRUCTURE_REF* const pDBStructureRef, LEVELNODE* const pLevelN)
{
	return InternalAddStructureToWorld(sBaseGridNo, bLevel, pDBStructureRef, pLevelN) != NULL;
}

//
// Structure deletion functions
//


static void DeleteStructureFromTile(MAP_ELEMENT* const me, STRUCTURE* const s)
{ // removes a STRUCTURE element at a particular location from the world
	// put location pointer in tile
	STRUCTURE* const next = s->pNext;
	STRUCTURE* const prev = s->pPrev;
	Assert((prev == NULL) == (me->pStructureHead == s));
	Assert((next == NULL) == (me->pStructureTail == s));
	*(prev != NULL ? &prev->pNext : &me->pStructureHead) = next;
	*(next != NULL ? &next->pPrev : &me->pStructureTail) = prev;

	// only one allowed in a tile, so we are safe to do this
	if (s->fFlags & STRUCTURE_OPENABLE) me->uiFlags &= ~MAPELEMENT_INTERACTIVETILE;

	MemFree(s);
}


BOOLEAN DeleteStructureFromWorld( STRUCTURE * pStructure )
{ // removes all of the STRUCTURE elements for a structure from the world
	STRUCTURE *						pBaseStructure;
	DB_STRUCTURE_TILE	**	ppTile;
	STRUCTURE *						pCurrent;
	UINT8									ubLoop, ubLoop2;
	UINT8									ubNumberOfTiles;
	INT16									sBaseGridNo, sGridNo;
	UINT16								usStructureID;
	BOOLEAN								fRecompileMPs;
	BOOLEAN								fRecompileExtraRadius; // for doors... yuck
	INT16									sCheckGridNo;

	CHECKF( pStructure );

	pBaseStructure = FindBaseStructure( pStructure );
	CHECKF( pBaseStructure );

	usStructureID = pBaseStructure->usStructureID;
	fRecompileMPs = ( ( gsRecompileAreaLeft != 0 ) && ! ( ( pBaseStructure->fFlags & STRUCTURE_MOBILE ) != 0 ) );
	if (fRecompileMPs)
	{
		fRecompileExtraRadius = ( ( pBaseStructure->fFlags & STRUCTURE_WALLSTUFF ) != 0 );
	}
	else
	{
		fRecompileExtraRadius = FALSE;
	}

	ppTile = pBaseStructure->pDBStructureRef->ppTile;
	sBaseGridNo = pBaseStructure->sGridNo;
	ubNumberOfTiles = pBaseStructure->pDBStructureRef->pDBStructure->ubNumberOfTiles;
	// Free all the tiles
	for (ubLoop = BASE_TILE; ubLoop < ubNumberOfTiles; ubLoop++)
	{
		sGridNo = sBaseGridNo + ppTile[ubLoop]->sPosRelToBase;
		// there might be two structures in this tile, one on each level, but we just want to
		// delete one on each pass
		pCurrent = FindStructureByID( sGridNo, usStructureID );
		if (pCurrent)
		{
			DeleteStructureFromTile( &gpWorldLevelData[sGridNo], pCurrent );
		}

		if ( !gfEditMode && ( fRecompileMPs ) )
		{
			if ( fRecompileMPs )
			{
				AddTileToRecompileArea( sGridNo );
				if ( fRecompileExtraRadius )
				{
					// add adjacent tiles too
					for (ubLoop2 = 0; ubLoop2 < NUM_WORLD_DIRECTIONS; ubLoop2++)
					{
						sCheckGridNo = NewGridNo( sGridNo, DirectionInc( ubLoop2 ) );
						if (sCheckGridNo != sGridNo)
						{
							AddTileToRecompileArea( sCheckGridNo );
						}
					}
				}
			}
		}
	}
	return( TRUE );
}


static STRUCTURE* InternalSwapStructureForPartner(INT16 sGridNo, STRUCTURE* pStructure, BOOLEAN fFlipSwitches, BOOLEAN fStoreInMap)
try
{ // switch structure
	LEVELNODE *				pLevelNode;
	LEVELNODE *				pShadowNode;
	STRUCTURE *				pBaseStructure;
	STRUCTURE *				pNewBaseStructure;
	BOOLEAN						fDoor;

	INT8							bDelta;
	UINT8							ubHitPoints;
	INT16							sCubeOffset;

	if (pStructure == NULL)
	{
		return( NULL );
	}
	pBaseStructure = FindBaseStructure( pStructure );
	CHECKF( pBaseStructure );
	if ((pBaseStructure->pDBStructureRef->pDBStructure)->bPartnerDelta == NO_PARTNER_STRUCTURE)
	{
		return( NULL );
	}
	fDoor = ((pBaseStructure->fFlags & STRUCTURE_ANYDOOR) > 0);
	pLevelNode = FindLevelNodeBasedOnStructure( pBaseStructure->sGridNo, pBaseStructure );
	pShadowNode = FindShadow( pBaseStructure->sGridNo, pLevelNode->usIndex );

	// record values
	bDelta = pBaseStructure->pDBStructureRef->pDBStructure->bPartnerDelta;
	const DB_STRUCTURE_REF* const pPartnerDBStructure = pBaseStructure->pDBStructureRef + bDelta;
	sGridNo = pBaseStructure->sGridNo;
	ubHitPoints = pBaseStructure->ubHitPoints;
	sCubeOffset = pBaseStructure->sCubeOffset;
	// delete the old structure and add the new one
	if (!DeleteStructureFromWorld(pBaseStructure))
	{
		return( NULL );
	}
	pNewBaseStructure = InternalAddStructureToWorld( sGridNo, (INT8) (sCubeOffset / PROFILE_Z_SIZE), pPartnerDBStructure, pLevelNode );
	if (pNewBaseStructure == NULL)
	{
		return( NULL );
	}
	// set values in the new structure
	pNewBaseStructure->ubHitPoints = ubHitPoints;
	if (!fDoor)
	{ // swap the graphics

		// store removal of previous if necessary
		if ( fStoreInMap )
		{
			ApplyMapChangesToMapTempFile( TRUE );
			RemoveStructFromMapTempFile( sGridNo, pLevelNode->usIndex );
		}

		pLevelNode->usIndex += bDelta;

		// store removal of new one if necessary
		if ( fStoreInMap )
		{
			AddStructToMapTempFile( sGridNo, pLevelNode->usIndex );
			ApplyMapChangesToMapTempFile( FALSE );
		}

		if (pShadowNode != NULL)
		{
			pShadowNode->usIndex += bDelta;
		}
	}

	//if ( (pNewBaseStructure->fFlags & STRUCTURE_SWITCH) && (pNewBaseStructure->fFlags & STRUCTURE_OPEN) )
	if ( 0 /*fFlipSwitches*/ )
	{
		if ( pNewBaseStructure->fFlags & STRUCTURE_SWITCH )
		{
			// just turned a switch on!
			ActivateSwitchInGridNo(NULL, sGridNo);
		}
	}
	return( pNewBaseStructure );
}
catch (...) { return 0; }


STRUCTURE * SwapStructureForPartner( INT16 sGridNo, STRUCTURE * pStructure )
{
	return( InternalSwapStructureForPartner( sGridNo, pStructure, TRUE, FALSE ) );
}

STRUCTURE * SwapStructureForPartnerWithoutTriggeringSwitches( INT16 sGridNo, STRUCTURE * pStructure )
{
	return( InternalSwapStructureForPartner( sGridNo, pStructure, FALSE, FALSE ) );
}

STRUCTURE * SwapStructureForPartnerAndStoreChangeInMap( INT16 sGridNo, STRUCTURE * pStructure )
{
	return( InternalSwapStructureForPartner( sGridNo, pStructure, TRUE, TRUE ) );
}


STRUCTURE* FindStructure(const INT16 sGridNo, const UINT32 flags)
{
	Assert(flags != 0);
	for (STRUCTURE* i = gpWorldLevelData[sGridNo].pStructureHead;; i = i->pNext)
	{
		if (i == NULL || i->fFlags & flags) return i;
	}
}


STRUCTURE* FindNextStructure(const STRUCTURE* const s, const UINT32 flags)
{
	Assert(flags != 0);
	CHECKN(s);
	for (STRUCTURE* i = s->pNext;; i = i->pNext)
	{
		if (i == NULL || i->fFlags & flags) return i;
	}
}


STRUCTURE* FindStructureByID(const INT16 sGridNo, const UINT16 structure_id)
{
	for (STRUCTURE* i = gpWorldLevelData[sGridNo].pStructureHead;; i = i->pNext)
	{
		if (i == NULL || i->usStructureID == structure_id) return i;
	}
}


STRUCTURE* FindBaseStructure(STRUCTURE* const s)
{
	CHECKN(s);
	if (s->fFlags & STRUCTURE_BASE_TILE) return s;
	return FindStructureByID(s->sBaseGridNo, s->usStructureID);
}


INT8 StructureHeight( STRUCTURE * pStructure )
{ // return the height of an object from 1-4
	UINT8				ubLoopX, ubLoopY;
	PROFILE *		pShape;
	UINT8				ubShapeValue;
	INT8				bLoopZ;
	INT8				bGreatestHeight = -1;

	if (pStructure == NULL || pStructure->pShape == NULL)
	{
		return( 0 );
	}

	if (pStructure->ubStructureHeight != 0)
	{
		return( pStructure->ubStructureHeight );
	}

	pShape = pStructure->pShape;

	// loop horizontally on the X and Y planes
	for (ubLoopX = 0; ubLoopX < PROFILE_X_SIZE; ubLoopX++)
	{
		for (ubLoopY = 0; ubLoopY < PROFILE_Y_SIZE; ubLoopY++)
		{
			ubShapeValue = (*pShape)[ubLoopX][ubLoopY];
			// loop DOWN vertically so that we find the tallest point first
			// and don't need to check any below it
			for (bLoopZ = PROFILE_Z_SIZE - 1; bLoopZ > bGreatestHeight; bLoopZ--)
			{
				if (ubShapeValue & AtHeight[bLoopZ])
				{
					bGreatestHeight = bLoopZ;
					if (bGreatestHeight == PROFILE_Z_SIZE - 1)
					{
						// store height
						pStructure->ubStructureHeight = bGreatestHeight + 1;
						return( bGreatestHeight + 1);
					}
					break;
				}
			}
		}
	}
	// store height
	pStructure->ubStructureHeight = bGreatestHeight + 1;
	return( bGreatestHeight + 1);
}

INT8 GetTallestStructureHeight( INT16 sGridNo, BOOLEAN fOnRoof )
{
	STRUCTURE *		pCurrent;
	INT8					iHeight;
	INT8					iTallest = 0;
	INT16					sDesiredHeight;

	if (fOnRoof)
	{
		sDesiredHeight = STRUCTURE_ON_ROOF;
	}
	else
	{
		sDesiredHeight = STRUCTURE_ON_GROUND;
	}
	pCurrent = gpWorldLevelData[sGridNo].pStructureHead;
	while (pCurrent != NULL)
	{
		if (pCurrent->sCubeOffset == sDesiredHeight)
		{
			iHeight = StructureHeight( pCurrent );
			if (iHeight > iTallest)
			{
				iTallest = iHeight;
			}
		}
		pCurrent = pCurrent->pNext;
	}
	return( iTallest );
}


INT8 GetStructureTargetHeight( INT16 sGridNo, BOOLEAN fOnRoof )
{
	STRUCTURE *		pCurrent;
	INT8					iHeight;
	INT8					iTallest = 0;
	INT16					sDesiredHeight;

	if (fOnRoof)
	{
		sDesiredHeight = STRUCTURE_ON_ROOF;
	}
	else
	{
		sDesiredHeight = STRUCTURE_ON_GROUND;
	}

	// prioritize openable structures and doors
	pCurrent = FindStructure( sGridNo, (STRUCTURE_DOOR | STRUCTURE_OPENABLE ) );
	if ( pCurrent )
	{
		// use this structure
		if ( pCurrent->fFlags & STRUCTURE_DOOR )
		{
			iTallest = 3; // don't aim at the very top of the door
		}
		else
		{
			iTallest = StructureHeight( pCurrent );
		}
	}
	else
	{
		pCurrent = gpWorldLevelData[sGridNo].pStructureHead;
		while (pCurrent != NULL)
		{
			if (pCurrent->sCubeOffset == sDesiredHeight)
			{
				iHeight = StructureHeight( pCurrent );

				if (iHeight > iTallest)
				{
					iTallest = iHeight;
				}
			}
			pCurrent = pCurrent->pNext;
		}
	}
	return( iTallest );
}


INT8 StructureBottomLevel( STRUCTURE * pStructure )
{ // return the bottom level of an object, from 1-4
	UINT8				ubLoopX, ubLoopY;
	PROFILE *		pShape;
	UINT8				ubShapeValue;
	INT8				bLoopZ;
	INT8				bLowestHeight = PROFILE_Z_SIZE;

	if (pStructure == NULL || pStructure->pShape == NULL)
	{
		return( 0 );
	}
	pShape = pStructure->pShape;

	// loop horizontally on the X and Y planes
	for (ubLoopX = 0; ubLoopX < PROFILE_X_SIZE; ubLoopX++)
	{
		for (ubLoopY = 0; ubLoopY < PROFILE_Y_SIZE; ubLoopY++)
		{
			ubShapeValue = (*pShape)[ubLoopX][ubLoopY];
			// loop DOWN vertically so that we find the tallest point first
			// and don't need to check any below it
			for (bLoopZ = 0; bLoopZ < bLowestHeight; bLoopZ++)
			{
				if (ubShapeValue & AtHeight[bLoopZ])
				{
					bLowestHeight = bLoopZ;
					if (bLowestHeight == 0)
					{
						return( 1 );
					}
					break;
				}
			}
		}
	}
	return( bLowestHeight + 1);
}


BOOLEAN StructureDensity( STRUCTURE * pStructure, UINT8 * pubLevel0, UINT8 * pubLevel1, UINT8 * pubLevel2, UINT8 * pubLevel3 )
{
	UINT8				ubLoopX, ubLoopY;
	UINT8				ubShapeValue;
	PROFILE *		pShape;

	CHECKF( pStructure );
	CHECKF( pubLevel0 );
	CHECKF( pubLevel1 );
	CHECKF( pubLevel2 );
	CHECKF( pubLevel3 );
	*pubLevel0 = 0;
	*pubLevel1 = 0;
	*pubLevel2 = 0;
	*pubLevel3 = 0;

	pShape = pStructure->pShape;

	for (ubLoopX = 0; ubLoopX < PROFILE_X_SIZE; ubLoopX++)
	{
		for (ubLoopY = 0; ubLoopY < PROFILE_Y_SIZE; ubLoopY++)
		{
			ubShapeValue = (*pShape)[ubLoopX][ubLoopY];
			if (ubShapeValue & AtHeight[0])
			{
				(*pubLevel0)++;
			}
			if (ubShapeValue & AtHeight[1])
			{
				(*pubLevel1)++;
			}
			if (ubShapeValue & AtHeight[2])
			{
				(*pubLevel2)++;
			}
			if (ubShapeValue & AtHeight[3])
			{
				(*pubLevel3)++;
			}

		}
	}
	// convert values to percentages!
	*pubLevel0 *= 4;
	*pubLevel1 *= 4;
	*pubLevel2 *= 4;
	*pubLevel3 *= 4;
	return( TRUE );
}


BOOLEAN DamageStructure(STRUCTURE* const pStructure, UINT8 ubDamage, const UINT8 ubReason, const INT16 sGridNo, const INT16 sX, const INT16 sY, SOLDIERTYPE* const owner)
{	// do damage to a structure; returns TRUE if the structure should be removed

	STRUCTURE			*pBase;
	UINT8					ubArmour;
	//LEVELNODE			*pNode;

	CHECKF( pStructure );
	if (pStructure->fFlags & STRUCTURE_PERSON || pStructure->fFlags & STRUCTURE_CORPSE)
	{
		// don't hurt this structure, it's used for hit detection only!
		return( FALSE );
	}

	if ( (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_INDESTRUCTABLE_METAL) || (pStructure->pDBStructureRef->pDBStructure->ubArmour == MATERIAL_INDESTRUCTABLE_STONE) )
	{
		return( FALSE );
	}

	// Account for armour!
	if (ubReason == STRUCTURE_DAMAGE_EXPLOSION)
	{
		if ( pStructure->fFlags & STRUCTURE_EXPLOSIVE )
		{
			ubArmour = gubMaterialArmour[ pStructure->pDBStructureRef->pDBStructure->ubArmour ] / 3;
		}
		else
		{
			ubArmour = gubMaterialArmour[ pStructure->pDBStructureRef->pDBStructure->ubArmour ] / 2;
		}

		if (ubArmour > ubDamage)
		{
			// didn't even scratch the paint
			return( FALSE );
		}
		else
		{
			// did some damage to the structure
			ubDamage -= ubArmour;
		}
	}
	else
	{
		ubDamage = 0;
	}

	// OK, Let's check our reason
	if ( ubReason == STRUCTURE_DAMAGE_GUNFIRE )
	{
		// If here, we have penetrated, check flags
		// Are we an explodable structure?
		if ( (pStructure->fFlags & STRUCTURE_EXPLOSIVE) && Random( 2 ) )
		{
			// Remove struct!
			pBase = FindBaseStructure( pStructure );

			// ATE: Set hit points to zero....
			pBase->ubHitPoints = 0;

			// Get LEVELNODE for struct and remove!
			// pNode = FindLevelNodeBasedOnStructure( pBase->sGridNo, pBase );


			//Set a flag indicating that the following changes are to go the the maps temp file
			// ApplyMapChangesToMapTempFile( TRUE );
			// Remove!
			// RemoveStructFromLevelNode( pBase->sGridNo, pNode );
			// ApplyMapChangesToMapTempFile( FALSE );

			// Generate an explosion here!
			IgniteExplosionXY(owner, sX, sY, 0, sGridNo, STRUCTURE_IGNITE, 0);

			// ATE: Return false here, as we are dealing with deleting the graphic here...
			return( FALSE );
		}

		// Make hit sound....
    if ( pStructure->fFlags & STRUCTURE_CAVEWALL )
    {
			PlayLocationJA2Sample(sGridNo, S_VEG_IMPACT1, HIGHVOLUME, 1);
    }
    else
    {
			SoundID const snd = guiMaterialHitSound[pStructure->pDBStructureRef->pDBStructure->ubArmour];
			if (snd != NO_SOUND) PlayLocationJA2Sample(sGridNo, snd, HIGHVOLUME, 1);
    }
		// Don't update damage HPs....
		return( TRUE );
	}

	// OK, LOOK FOR A SAM SITE, UPDATE....
	UpdateAndDamageSAMIfFound( gWorldSectorX, gWorldSectorY, gbWorldSectorZ, sGridNo, ubDamage );

	// find the base so we can reduce the hit points!
	pBase = FindBaseStructure( pStructure );
	CHECKF( pBase );
	if (pBase->ubHitPoints <= ubDamage)
	{
		// boom! structure destroyed!
		return( TRUE );
	}
	else
	{
		pBase->ubHitPoints -= ubDamage;

		//Since the structure is being damaged, set the map element that a structure is damaged
		gpWorldLevelData[ sGridNo ].uiFlags |= MAPELEMENT_STRUCTURE_DAMAGED;

		// We are a little damaged....
		return( 2 );
	}
}

#define LINE_HEIGHT 20
void DebugStructurePage1( void )
{
	STRUCTURE *		pStructure;
	STRUCTURE *		pBase;
	//LEVELNODE *		pLand;
	INT8					bStructures;

	static const wchar_t* const WallOrientationString[] =
	{
		L"None",
		L"Inside left",
		L"Inside right",
		L"Outside left",
		L"Outside right"
	};

	SetFont( LARGEFONT1 );
	gprintf( 0, 0, L"DEBUG STRUCTURES PAGE 1 OF 1" );

	const GridNo sGridNo = GetMouseMapPos();
	if (sGridNo == NOWHERE) return;

	const INT16 sDesiredLevel = (gsInterfaceLevel == I_GROUND_LEVEL ? STRUCTURE_ON_GROUND : STRUCTURE_ON_ROOF);

	gprintf( 320, 0, L"Building %d", gubBuildingInfo[ sGridNo ] );
	/*
	pLand = gpWorldLevelData[sGridNo].pLandHead;
	gprintf( 320, 0, L"Fake light %d", pLand->ubFakeShadeLevel );
	gprintf( 320, LINE_HEIGHT, L"Real light: ground %d roof %d", LightTrueLevel( sGridNo, 0 ), LightTrueLevel( sGridNo, 1 ) );
	*/

	pStructure = gpWorldLevelData[sGridNo].pStructureHead;
	while (pStructure != NULL)
	{
		if (pStructure->sCubeOffset == sDesiredLevel)
		{
			break;
		}
		pStructure = pStructure->pNext;
	}

	if (pStructure != NULL)
	{
		if (pStructure->fFlags & STRUCTURE_GENERIC)
		{
			gprintf( 0, LINE_HEIGHT * 1, L"Generic structure %x #%d", pStructure->fFlags, pStructure->pDBStructureRef->pDBStructure->usStructureNumber );
		}
		else if (pStructure->fFlags & STRUCTURE_TREE)
		{
			gprintf( 0, LINE_HEIGHT * 1, L"Tree");
		}
		else if (pStructure->fFlags & STRUCTURE_WALL)
		{
			gprintf(0, LINE_HEIGHT * 1, L"Wall with orientation %ls", WallOrientationString[pStructure->ubWallOrientation]);
		}
		else if (pStructure->fFlags & STRUCTURE_WALLNWINDOW)
		{
			gprintf( 0, LINE_HEIGHT * 1, L"Wall with window" );
		}
		else if (pStructure->fFlags & STRUCTURE_VEHICLE)
		{
			gprintf( 0, LINE_HEIGHT * 1, L"Vehicle %d", pStructure->pDBStructureRef->pDBStructure->usStructureNumber );
		}
		else if (pStructure->fFlags & STRUCTURE_NORMAL_ROOF)
		{
			gprintf( 0, LINE_HEIGHT * 1, L"Roof" );
		}
		else if (pStructure->fFlags & STRUCTURE_SLANTED_ROOF)
		{
			gprintf( 0, LINE_HEIGHT * 1, L"Slanted roof" );
		}
		else if (pStructure->fFlags & STRUCTURE_DOOR)
		{
			gprintf(0, LINE_HEIGHT * 1, L"Door with orientation %ls", WallOrientationString[pStructure->ubWallOrientation]);
		}
		else if (pStructure->fFlags & STRUCTURE_SLIDINGDOOR)
		{
			gprintf(0, LINE_HEIGHT * 1, L"%ls sliding door with orientation %ls",
				(pStructure->fFlags & STRUCTURE_OPEN) ? L"Open" : L"Closed",
				WallOrientationString[pStructure->ubWallOrientation] );
		}
		else if (pStructure->fFlags & STRUCTURE_DDOOR_LEFT)
		{
			gprintf(0, LINE_HEIGHT * 1, L"DDoorLft with orientation %ls", WallOrientationString[pStructure->ubWallOrientation]);
		}
		else if (pStructure->fFlags & STRUCTURE_DDOOR_RIGHT)
		{
			gprintf(0, LINE_HEIGHT * 1, L"DDoorRt with orientation %ls", WallOrientationString[pStructure->ubWallOrientation]);
		}
		else
		{
			gprintf( 0, LINE_HEIGHT * 1, L"UNKNOWN STRUCTURE! (%x)", pStructure->fFlags );
		}
		const INT8 bHeight = StructureHeight(pStructure);
		pBase = FindBaseStructure( pStructure );
		gprintf( 0, LINE_HEIGHT * 2, L"Structure height %d, cube offset %d, armour %d, HP %d", bHeight, pStructure->sCubeOffset, gubMaterialArmour[pStructure->pDBStructureRef->pDBStructure->ubArmour], pBase->ubHitPoints );
		UINT8 bDens0;
		UINT8 bDens1;
		UINT8 bDens2;
		UINT8 bDens3;
		if (StructureDensity(pStructure, &bDens0, &bDens1, &bDens2, &bDens3))
		{
			gprintf( 0, LINE_HEIGHT * 3, L"Structure fill %d%%/%d%%/%d%%/%d%% density %d", bDens0, bDens1, bDens2, bDens3,
				pStructure->pDBStructureRef->pDBStructure->ubDensity );
		}

		#ifndef LOS_DEBUG
		gprintf( 0, LINE_HEIGHT * 4, L"Structure ID %d", pStructure->usStructureID );
		#endif

		pStructure = gpWorldLevelData[sGridNo].pStructureHead;
		for ( bStructures = 0; pStructure != NULL; pStructure = pStructure->pNext)
		{
			bStructures++;
		}
		gprintf( 0, LINE_HEIGHT * 12, L"Number of structures = %d", bStructures );
	}
	#ifdef LOS_DEBUG
		if (gLOSTestResults.fLOSTestPerformed)
		{
			gprintf( 0, LINE_HEIGHT * 4, L"LOS from (%7d,%7d,%7d)", gLOSTestResults.iStartX, gLOSTestResults.iStartY, gLOSTestResults.iStartZ);
			gprintf( 0, LINE_HEIGHT * 5, L"to (%7d,%7d,%7d)", gLOSTestResults.iEndX, gLOSTestResults.iEndY, gLOSTestResults.iEndZ);
			if (gLOSTestResults.fOutOfRange)
			{
				gprintf( 0, LINE_HEIGHT * 6, L"is out of range" );
			}
			else if (gLOSTestResults.fLOSClear)
			{
				gprintf( 0, LINE_HEIGHT * 6, L"is clear!" );
			}
			else
			{
				gprintf( 0, LINE_HEIGHT * 6, L"is blocked at (%7d,%7d,%7d)!", gLOSTestResults.iStoppedX, gLOSTestResults.iStoppedY, gLOSTestResults.iStoppedZ );
				gprintf( 0, LINE_HEIGHT * 10, L"Blocked at cube level %d", gLOSTestResults.iCurrCubesZ );
			}
			gprintf( 0, LINE_HEIGHT * 7, L"Passed through %d tree bits!", gLOSTestResults.ubTreeSpotsHit );
			gprintf( 0, LINE_HEIGHT * 8, L"Maximum range was %7d", gLOSTestResults.iMaxDistance );
			gprintf( 0, LINE_HEIGHT * 9, L"actual range was %7d", gLOSTestResults.iDistance );
			if (gLOSTestResults.ubChanceToGetThrough <= 100)
			{
				gprintf( 0, LINE_HEIGHT * 11, L"Chance to get through was %d", gLOSTestResults.ubChanceToGetThrough );
			}
		}
	#endif
	gprintf( 0, LINE_HEIGHT * 13, L"N %d NE %d E %d SE %d",
		gubWorldMovementCosts[ sGridNo ][ NORTH ][ gsInterfaceLevel ],
		gubWorldMovementCosts[ sGridNo ][ NORTHEAST ][ gsInterfaceLevel ],
		gubWorldMovementCosts[ sGridNo ][ EAST ][ gsInterfaceLevel ],
		gubWorldMovementCosts[ sGridNo ][ SOUTHEAST ][ gsInterfaceLevel ] );
	gprintf( 0, LINE_HEIGHT * 14, L"S %d SW %d W %d NW %d",
		gubWorldMovementCosts[ sGridNo ][ SOUTH ][ gsInterfaceLevel ],
		gubWorldMovementCosts[ sGridNo ][ SOUTHWEST ][ gsInterfaceLevel ],
		gubWorldMovementCosts[ sGridNo ][ WEST ][ gsInterfaceLevel ],
		gubWorldMovementCosts[ sGridNo ][ NORTHWEST ][ gsInterfaceLevel ] );
	gprintf( 0, LINE_HEIGHT * 15, L"Ground smell %d strength %d",
		SMELL_TYPE( gpWorldLevelData[ sGridNo ].ubSmellInfo ),
		SMELL_STRENGTH( gpWorldLevelData[ sGridNo ].ubSmellInfo ) );

	#ifdef COUNT_PATHS
	if (guiTotalPathChecks > 0)
	{
		gprintf( 0, LINE_HEIGHT * 16,
			L"Total %ld, %%succ %3ld | %%failed %3ld | %%unsucc %3ld",
			guiTotalPathChecks,
			100 * guiSuccessfulPathChecks / guiTotalPathChecks,
			100 * guiFailedPathChecks / guiTotalPathChecks,
			100 * guiUnsuccessfulPathChecks / guiTotalPathChecks );

	}
	#else
	gprintf( 0, LINE_HEIGHT * 16,
		L"Adj soldiers %d", gpWorldLevelData[sGridNo].ubAdjacentSoldierCnt );
	#endif
}


void AddZStripInfoToVObject(HVOBJECT const hVObject, STRUCTURE_FILE_REF const* const pStructureFileRef, BOOLEAN const fFromAnimation, INT16 sSTIStartIndex)
{
	if (pStructureFileRef->usNumberOfStructuresStored == 0) return;

	BOOLEAN             fFound       = FALSE;
	const DB_STRUCTURE* pDBStructure = NULL;
	for (UINT32 uiLoop = 0; uiLoop < pStructureFileRef->usNumberOfStructures; ++uiLoop)
	{
		const DB_STRUCTURE_REF* const pDBStructureRef = &pStructureFileRef->pDBStructureRef[uiLoop];
		pDBStructure = pDBStructureRef->pDBStructure;
		//if (pDBStructure != NULL && pDBStructure->ubNumberOfTiles > 1 && !(pDBStructure->fFlags & STRUCTURE_WALLSTUFF) )
		if (pDBStructure != NULL && pDBStructure->ubNumberOfTiles > 1)
		{
			for (UINT8 ubLoop2 = 1; ubLoop2 < pDBStructure->ubNumberOfTiles; ++ubLoop2)
			{
				if (pDBStructureRef->ppTile[ubLoop2]->sPosRelToBase != 0)
				{
					// spans multiple tiles! (could be two levels high in one tile)
					fFound = TRUE;
					break;
				}
			}
		}
	}

	// ATE: Make all corpses use z-strip info..
	if (pDBStructure != NULL && pDBStructure->fFlags & STRUCTURE_CORPSE)
	{
		fFound = TRUE;
	}

	// if no multi-tile images in this vobject, that's okay... return!
	if (!fFound) return;

	UINT         const zcount = hVObject->SubregionCount();
	ZStripInfo** const zinfo  = MALLOCNZ(ZStripInfo*, zcount);

	INT16 sSTIStep;
	if (fFromAnimation)
	{
		// Determine step index for STI
		if (sSTIStartIndex == -1)
		{
			// one-direction only for this anim structure
			sSTIStep = zcount;
			sSTIStartIndex = 0;
		}
		else
		{
			sSTIStep = zcount / pStructureFileRef->usNumberOfStructures;
		}
	}
	else
	{
		sSTIStep = 1;
	}

	INT16   sLeftHalfWidth;
	INT16   sRightHalfWidth;
	INT16   sStructIndex    = 0;
	INT16   sNext           = sSTIStartIndex + sSTIStep;
	BOOLEAN fFirstTime      = TRUE;
	for (UINT32 uiLoop = sSTIStartIndex; uiLoop < zcount; ++uiLoop)
	try
	{
		// Defualt to true
		BOOLEAN fCopyIntoVo = TRUE;

		// Increment struct index....
		if (uiLoop == (UINT32)sNext)
		{
			sNext					= uiLoop + sSTIStep;
			sStructIndex++;
		}
		else
		{
			if (fFirstTime)
			{
				fFirstTime = FALSE;
			}
			else
			{
				fCopyIntoVo = FALSE;
			}
		}

		const UINT32 uiDestVoIndex = (fFromAnimation ? sStructIndex : uiLoop);

		if (fCopyIntoVo && sStructIndex < pStructureFileRef->usNumberOfStructures)
		{
			const DB_STRUCTURE* const pDBStructure = pStructureFileRef->pDBStructureRef[sStructIndex].pDBStructure;
			if (pDBStructure != NULL && (pDBStructure->ubNumberOfTiles > 1 || pDBStructure->fFlags & STRUCTURE_CORPSE))
			//if (pDBStructure != NULL && pDBStructure->ubNumberOfTiles > 1 )
			{
				// ATE: We allow SLIDING DOORS of 2 tile sizes...
				if (!(pDBStructure->fFlags & STRUCTURE_ANYDOOR) || pDBStructure->fFlags & STRUCTURE_SLIDINGDOOR)
				{
					ZStripInfo* const pCurr = MALLOC(ZStripInfo);
					Assert(uiDestVoIndex < zcount);
					zinfo[uiDestVoIndex] = pCurr;

					UINT8 ubNumIncreasing = 0;
					UINT8 ubNumStable     = 0;
					UINT8 ubNumDecreasing = 0;

					// time to do our calculations!
					ETRLEObject const& e        = hVObject->SubregionProperties(uiLoop);
					INT16              sOffsetX = e.sOffsetX;
					INT16              sOffsetY = e.sOffsetY;
					UINT16      const  usWidth  = e.usWidth;
					if (pDBStructure->fFlags & (STRUCTURE_MOBILE | STRUCTURE_CORPSE))
					{
						// adjust for the difference between the animation and structure base tile

						//if (pDBStructure->fFlags & (STRUCTURE_MOBILE ) )
						{
							sOffsetX += WORLD_TILE_X / 2;
							sOffsetY += WORLD_TILE_Y / 2;
						}
						// adjust for the tile offset
						sOffsetX = sOffsetX - pDBStructure->bZTileOffsetX * (WORLD_TILE_X / 2) + pDBStructure->bZTileOffsetY * (WORLD_TILE_X / 2);
						sOffsetY = sOffsetY - pDBStructure->bZTileOffsetY * (WORLD_TILE_Y / 2);
					}

					// figure out how much of the image is on each side of
					// the bottom corner of the base tile
					if (sOffsetX <= 0)
					{
						// note that the adjustments here by (WORLD_TILE_X / 2) are to account for the X difference
						// between the blit position and the bottom corner of the base tile
						sRightHalfWidth = usWidth + sOffsetX - (WORLD_TILE_X / 2);

						if (sRightHalfWidth >= 0)
						{
							// Case 1: negative image offset, image straddles bottom corner

							// negative of a negative is positive
							sLeftHalfWidth = -sOffsetX + (WORLD_TILE_X / 2);
						}
						else
						{
							// Case 2: negative image offset, image all on left side

							// bump up the LeftHalfWidth to the right edge of the last tile-half,
							// so we can calculate the size of the leftmost portion accurately
							// NB subtracting a negative to add the absolute value
							sLeftHalfWidth = usWidth - (sRightHalfWidth % (WORLD_TILE_X / 2));
							sRightHalfWidth = 0;
						}
					}
					else if (sOffsetX < (WORLD_TILE_X / 2))
					{
						sLeftHalfWidth = (WORLD_TILE_X / 2) - sOffsetX;
						sRightHalfWidth = usWidth - sLeftHalfWidth;
						if (sRightHalfWidth <= 0)
						{
							// Case 3: positive offset < 20, image all on left side
							// should never happen because these images are multi-tile!
							sRightHalfWidth = 0;
							// fake the left width to one half-tile
							sLeftHalfWidth = (WORLD_TILE_X / 2);
						}
						else
						{
							// Case 4: positive offset < 20, image straddles bottom corner

							// all okay?
						}
					}
					else
					{
						// Case 5: positive offset, image all on right side
						// should never happen either
						sLeftHalfWidth = 0;
						sRightHalfWidth = usWidth;
					}

					if (sLeftHalfWidth > 0)
					{
						ubNumIncreasing = sLeftHalfWidth / (WORLD_TILE_X / 2);
					}
					if (sRightHalfWidth > 0)
					{
						ubNumStable = 1;
						if (sRightHalfWidth > (WORLD_TILE_X / 2))
						{
							ubNumDecreasing = sRightHalfWidth / (WORLD_TILE_X / 2);
						}
					}
					if (sLeftHalfWidth > 0)
					{
						pCurr->ubFirstZStripWidth = sLeftHalfWidth % (WORLD_TILE_X / 2);
						if (pCurr->ubFirstZStripWidth == 0)
						{
							ubNumIncreasing--;
							pCurr->ubFirstZStripWidth = (WORLD_TILE_X / 2);
						}
					}
					else // right side only; offset is at least 20 (= WORLD_TILE_X / 2)
					{
						if (sOffsetX > WORLD_TILE_X)
						{
							pCurr->ubFirstZStripWidth = (WORLD_TILE_X / 2) - (sOffsetX - WORLD_TILE_X) % (WORLD_TILE_X / 2);
						}
						else
						{
							pCurr->ubFirstZStripWidth = WORLD_TILE_X - sOffsetX;
						}
						if (pCurr->ubFirstZStripWidth == 0)
						{
							ubNumDecreasing--;
							pCurr->ubFirstZStripWidth = (WORLD_TILE_X / 2);
						}
					}

					// now create the array!
					pCurr->ubNumberOfZChanges = ubNumIncreasing + ubNumStable + ubNumDecreasing;
					pCurr->pbZChange = MALLOCN(INT8, pCurr->ubNumberOfZChanges);

					UINT8 ubLoop2;
					for (ubLoop2 = 0; ubLoop2 < ubNumIncreasing; ubLoop2++)
					{
						pCurr->pbZChange[ubLoop2] = 1;
					}
					for (; ubLoop2 < ubNumIncreasing + ubNumStable; ubLoop2++)
					{
						pCurr->pbZChange[ubLoop2] = 0;
					}
					for (; ubLoop2 < pCurr->ubNumberOfZChanges; ubLoop2++)
					{
						pCurr->pbZChange[ubLoop2] = -1;
					}
					if (ubNumIncreasing > 0)
					{
						pCurr->bInitialZChange = -ubNumIncreasing;
					}
					else if (ubNumStable > 0)
					{
						pCurr->bInitialZChange = 0;
					}
					else
					{
						pCurr->bInitialZChange = -ubNumDecreasing;
					}
				}
			}
		}
	}
	catch (...)
	{
		for (UINT ubLoop2 = 0; ubLoop2 < uiLoop; ++ubLoop2)
		{
			if (zinfo[ubLoop2] != NULL)
			{
				MemFree(zinfo[uiLoop]);
			}
		}
		MemFree(zinfo);
		throw;
	}

	hVObject->ppZStripInfo = zinfo;
}


INT8 GetBlockingStructureInfo( INT16 sGridNo, INT8 bDir, INT8 bNextDir, INT8 bLevel, INT8 *pStructHeight, STRUCTURE ** ppTallestStructure, BOOLEAN fWallsBlock )
{
	STRUCTURE* pStructure = NULL; // XXX HACK000E
	STRUCTURE* pCurrent;
	INT16				sDesiredLevel;
	BOOLEAN			fOKStructOnLevel = FALSE;
	BOOLEAN			fMinimumBlockingFound = FALSE;

	if ( bLevel == 0)
	{
		sDesiredLevel = STRUCTURE_ON_GROUND;
	}
	else
	{
		sDesiredLevel = STRUCTURE_ON_ROOF;
	}

	pCurrent =  gpWorldLevelData[sGridNo].pStructureHead;

	// If no struct, return
	if ( pCurrent == NULL )
	{
    (*pStructHeight) = StructureHeight( pCurrent );
		(*ppTallestStructure) = NULL;
		return( NOTHING_BLOCKING );
	}

	while (pCurrent != NULL)
	{
		// Check level!
		if (pCurrent->sCubeOffset == sDesiredLevel )
		{
			fOKStructOnLevel = TRUE;
      pStructure       = pCurrent;

			// Turn off if we are on upper level!
		  if ( pCurrent->fFlags & STRUCTURE_ROOF && bLevel == 1 )
			{
				fOKStructOnLevel = FALSE;
			}

			// Don't stop FOV for people
			if ( pCurrent->fFlags & ( STRUCTURE_CORPSE | STRUCTURE_PERSON ) )
			{
				fOKStructOnLevel = FALSE;
			}


			if ( pCurrent->fFlags & ( STRUCTURE_TREE | STRUCTURE_ANYFENCE ) )
			{
				fMinimumBlockingFound = TRUE;
			}

			// Default, if we are a wall, set full blocking
			if ( ( pCurrent->fFlags & STRUCTURE_WALL ) && !fWallsBlock )
			{
				// Return full blocking!
				// OK! This will be handled by movement costs......!
				fOKStructOnLevel = FALSE;
			}

			// CHECK FOR WINDOW
			if ( pCurrent->fFlags & STRUCTURE_WALLNWINDOW )
			{
				switch( pCurrent->ubWallOrientation )
				{
					case OUTSIDE_TOP_LEFT:
					case INSIDE_TOP_LEFT:

            (*pStructHeight) = StructureHeight( pCurrent );
						(*ppTallestStructure) = pCurrent;

            if ( pCurrent->fFlags & STRUCTURE_OPEN )
            {
						  return( BLOCKING_TOPLEFT_OPEN_WINDOW );
            }
            else
            {
						  return( BLOCKING_TOPLEFT_WINDOW );
            }

					case OUTSIDE_TOP_RIGHT:
					case INSIDE_TOP_RIGHT:

            (*pStructHeight) = StructureHeight( pCurrent );
						(*ppTallestStructure) = pCurrent;

            if ( pCurrent->fFlags & STRUCTURE_OPEN )
            {
						  return( BLOCKING_TOPRIGHT_OPEN_WINDOW );
            }
            else
            {
						  return( BLOCKING_TOPRIGHT_WINDOW );
            }
				}
			}

			// Check for door
			if ( pCurrent->fFlags & STRUCTURE_ANYDOOR )
			{
				// If we are not opem, we are full blocking!
				if ( !(pCurrent->fFlags & STRUCTURE_OPEN ) )
				{
          (*pStructHeight) = StructureHeight( pCurrent );
					(*ppTallestStructure) = pCurrent;
					return( FULL_BLOCKING );
				}
				else
				{
					switch( pCurrent->ubWallOrientation )
					{
						case OUTSIDE_TOP_LEFT:
						case INSIDE_TOP_LEFT:

              (*pStructHeight) = StructureHeight( pCurrent );
							(*ppTallestStructure) = pCurrent;
							return( BLOCKING_TOPLEFT_DOOR );

						case OUTSIDE_TOP_RIGHT:
						case INSIDE_TOP_RIGHT:

              (*pStructHeight) = StructureHeight( pCurrent );
							(*ppTallestStructure) = pCurrent;
							return( BLOCKING_TOPRIGHT_DOOR );
					}
				}
			}
		}
		pCurrent = pCurrent->pNext;
	}

	// OK, here, we default to we've seen a struct, reveal just this one
	if ( fOKStructOnLevel )
	{
		if ( fMinimumBlockingFound )
		{
      (*pStructHeight) = StructureHeight( pStructure );
			(*ppTallestStructure) = pStructure;
			return( BLOCKING_REDUCE_RANGE );
		}
		else
		{
      (*pStructHeight) = StructureHeight( pStructure );
			(*ppTallestStructure) = pStructure;
			return( BLOCKING_NEXT_TILE );
		}
	}
	else
	{
    (*pStructHeight) = 0;
		(*ppTallestStructure) = NULL;
		return( NOTHING_BLOCKING );
	}
}




UINT8 StructureFlagToType( UINT32 uiFlag )
{
	UINT8		ubLoop;
	UINT32		uiBit = STRUCTURE_GENERIC;

	for ( ubLoop = 8; ubLoop < 32; ubLoop++ )
	{
		if ( (uiFlag & uiBit) != 0 )
		{
			return( ubLoop );
		}
		uiBit = uiBit << 1;
	}
	return( 0 );
}


static UINT32 StructureTypeToFlag(UINT8 ubType)
{
	UINT32		uiFlag = 0x1;

	uiFlag = uiFlag << ubType;
	return( uiFlag );
}

STRUCTURE * FindStructureBySavedInfo( INT16 sGridNo, UINT8 ubType, UINT8 ubWallOrientation, INT8 bLevel )
{
	STRUCTURE *	pCurrent;
	UINT32		uiTypeFlag;

	uiTypeFlag = StructureTypeToFlag( ubType );

	pCurrent =  gpWorldLevelData[sGridNo].pStructureHead;
	while (pCurrent != NULL)
	{
		if (pCurrent->fFlags & uiTypeFlag && pCurrent->ubWallOrientation == ubWallOrientation &&
			( (bLevel == 0 && pCurrent->sCubeOffset == 0) || (bLevel > 0 && pCurrent->sCubeOffset > 0) ) )
		{
			return( pCurrent );
		}
		pCurrent = pCurrent->pNext;
	}
	return( NULL );
}


SoundID GetStructureOpenSound(STRUCTURE* const pStructure, BOOLEAN const fClose)
{
  SoundID uiSoundID;
  switch( pStructure->pDBStructureRef->pDBStructure->ubArmour )
  {
    case MATERIAL_LIGHT_METAL:
    case MATERIAL_THICKER_METAL:

      uiSoundID = OPEN_LOCKER;
      break;

    case MATERIAL_WOOD_WALL:
    case MATERIAL_PLYWOOD_WALL:
    case MATERIAL_FURNITURE:

      uiSoundID = OPEN_WOODEN_BOX;
      break;

    default:
      uiSoundID = OPEN_DEFAULT_OPENABLE;
  }

  if ( fClose )
  {
    uiSoundID = static_cast<SoundID>(uiSoundID + 1);
  }

  return( uiSoundID );
}
