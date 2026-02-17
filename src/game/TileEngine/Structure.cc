#include "HImage.h"
#include "LoadSaveData.h"
#include "SGPFile.h"
#include "Soldier_Control.h"
#include "Structure_Internals.h"
#include "Types.h"
#include "VObject.h"
#include "WCheck.h"
#include "Debug.h"
#include "Structure.h"
#include "TileDef.h"
#include "WorldDef.h"
#include "WorldMan.h"
#include "Interface.h"
#include "Isometric_Utils.h"
#include "Font.h"
#include "Debug_Pages.h"
#include "Smell.h"
#include "SaveLoadMap.h"
#include "Sys_Globals.h" //for access to gfEditMode flag
//Kris:
#include "Editor_Undo.h" //for access to AddToUndoList( iMapIndex )

#include "Explosion_Control.h"
#include "Buildings.h"
#include "Random.h"
#include "Tile_Animation.h"
#include "GameMode.h"

#include "ContentManager.h"
#include "GameInstance.h"

#include <array>
#include <climits>
#include <map>
#include <memory>
#include <stdexcept>
#include <string_theory/format>
#include <string_theory/string>



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

constexpr UINT16 FIRST_AVAILABLE_STRUCTURE_ID = INVALID_STRUCTURE_ID + 2;

static UINT16 gusNextAvailableStructureID = FIRST_AVAILABLE_STRUCTURE_ID;

// This keeps track of all currently loaded .jsd files.
static std::map<STRUCTURE_FILE_REF *, ST::string> gpStructureFileRefs;


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
UINT8 const gubMaterialArmour[] =
{ // note: must increase; r.c. should block *AP* 7.62mm rounds
	0, // nothing
	25, // dry timber; wood wall +1/2
	20, // furniture wood (thin!) or plywood wall +1/2
	30, // wood (live); 1.5x timber
	3, // light vegetation
	10, // upholstered furniture
	47, // porcelain
	10, // cactus, hay, bamboo
	0,
	0,
	0,
	55, // stone masonry; 3x timber
	63, // non-reinforced concrete; 4x timber???
	70, // reinforced concrete; 6x timber
	85, // rock? - number invented
	9, // rubber - tires
	40, // sand
	1, // cloth
	40, // sandbag
	0,
	0,
	37, // light metal (furniture; NB thin!)
	57, // thicker metal (dumpster)
	85, // heavy metal (vault doors) - block everything
	// note that vehicle armour will probably end up in here
	127, // rock indestructable
	127, // indestructable
	57, // like 22 but with screen windows
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
STRUCTURE_FILE_REF::~STRUCTURE_FILE_REF()
{
	/* Free all of the memory associated with a file reference, including the file
	 * reference structure itself */
	for (auto const& dbs : pDBStructureRef)
	{
		delete[] dbs.ppTile;
	}

	try
	{
		if (gpStructureFileRefs.erase(this) != 1)
		{
			SLOGE("Deleting a STRUCTURE_FILE_REF that is not tracked.");
		}
	}
	catch (...)
	{
		// Ignore any exceptions thrown by erase or SLOGE.
	}
}


void FreeAllStructureFiles()
{ // Free all of the structure database

	while (!gpStructureFileRefs.empty())
	{
		delete gpStructureFileRefs.begin()->first;
	}
}


/* IMPORTANT THING TO REMEMBER
 * Although the number of structures and images about which information may be
 * stored in a file, the two are stored very differently.
 * The structure data stored amounts to a sparse array, with no data saved for
 * any structures that are not defined.
 * For image information, however, an array is stored with every entry filled
 * regardless of whether there is non-zero data defined for that graphic! */

// "J2SD" = Jagged 2 Structure Data
#define STRUCTURE_FILE_ID "J2SD"
#define STRUCTURE_FILE_ID_LEN 4


// Loads a structure file's data as a honking chunk o' memory
STRUCTURE_FILE_REF::STRUCTURE_FILE_REF(SGPFile & jsdFile)
{
	constexpr UINT8 STRUCTURE_FILE_CONTAINS_AUXIMAGEDATA  = 0x01;
	constexpr UINT8 STRUCTURE_FILE_CONTAINS_STRUCTUREDATA = 0x02;

	BYTE data[16];
	jsdFile.read(data, sizeof(data));

	char   id[4];
	UINT16 n_structures;
	UINT16 n_structures_stored;
	UINT16 data_size;
	UINT8  flags;
	UINT16 n_tile_locs_stored;

	DataReader d{data};
	EXTR_STR(d, id, lengthof(id))
	EXTR_U16(d, n_structures);
	EXTR_U16( d, n_structures_stored)
	EXTR_U16( d, data_size)
	EXTR_U8(  d, flags)
	EXTR_SKIP(d, 3)
	EXTR_U16( d, n_tile_locs_stored)
	Assert(d.getConsumed() == lengthof(data));

	if (strncmp(id, STRUCTURE_FILE_ID, STRUCTURE_FILE_ID_LEN) != 0 ||
			n_structures == 0)
	{
		throw std::runtime_error("Failed to load structure file, because header is invalid");
	}

	usNumberOfStructures = n_structures;
	if (flags & STRUCTURE_FILE_CONTAINS_AUXIMAGEDATA)
	{
		pAuxData.resize(n_structures);
		jsdFile.read(pAuxData.data(), sizeof(AuxObjectData) * n_structures);

		if (n_tile_locs_stored > 0)
		{
			pTileLocData.resize(n_tile_locs_stored);
			jsdFile.read(pTileLocData.data(), sizeof(RelTileLoc) * n_tile_locs_stored);
		}
	}

	if (flags & STRUCTURE_FILE_CONTAINS_STRUCTUREDATA)
	{
		usNumberOfStructuresStored = n_structures_stored;
		pubStructureData.resize(data_size);
		jsdFile.read(pubStructureData.data(), data_size);
	}
}

void NormalizeStructureTiles(DB_STRUCTURE_TILE** pTiles, UINT8 ubNumTiles)
{
	/**
	 * In #1107, it was discovered that some of the Copter structures do not
	 * have a base tile. RemoveStruct will not work without a base tile, as
	 * it tries to find a non-existent base structure.
	 *
	 * This function attempts to correct such issues with JSD data, by
	 * ensuring there is a tile with (0, 0) position relative to base.
	 */
	int minDistFromBase = INT_MAX;
	for (UINT8 i = 0; i < ubNumTiles; i++)
	{
		DB_STRUCTURE_TILE* tile = pTiles[i];
		if (abs(minDistFromBase) > abs(tile->sPosRelToBase))
		{
			minDistFromBase = tile->sPosRelToBase;
		}
	}

	if (minDistFromBase == 0)
	{
		// Data is fine. Nothing to do.
		return;
	}

	SLOGD("Adjusting tiles relative positions by {}", -minDistFromBase);
	int xDist = minDistFromBase % WORLD_COLS;
	int yDist = minDistFromBase / WORLD_COLS;
	for (UINT8 i = 0; i < ubNumTiles; i++)
	{
		DB_STRUCTURE_TILE* tile = pTiles[i];
		tile->sPosRelToBase -= minDistFromBase;
		tile->bXPosRelToBase -= xDist;
		tile->bYPosRelToBase -= yDist;
	}
}

static void CreateFileStructureArrays(STRUCTURE_FILE_REF & pFileRef)
{ /* Based on a file chunk, creates all the dynamic arrays for the structure
	 * definitions contained within */
	auto * pCurrent{ pFileRef.pubStructureData.data() };
	auto & pDBStructureRef{ pFileRef.pDBStructureRef };
	pDBStructureRef.resize(pFileRef.usNumberOfStructures);
	size_t uiDataSize{ pFileRef.pubStructureData.size() };

	for (UINT16 usLoop = 0; usLoop < pFileRef.usNumberOfStructuresStored; ++usLoop)
	{
		if (uiDataSize < sizeof(DB_STRUCTURE))
		{	// gone past end of file block?!
			// freeing of memory will occur outside of the function
			throw std::runtime_error("Failed to create structure arrays, because input data is too short");
		}
		DB_STRUCTURE* const dbs = (DB_STRUCTURE*)pCurrent;
		pCurrent   += sizeof(DB_STRUCTURE);
		uiDataSize -= sizeof(DB_STRUCTURE);

		DB_STRUCTURE_TILE** const tiles       = new DB_STRUCTURE_TILE*[dbs->ubNumberOfTiles]{};
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

		NormalizeStructureTiles(tiles, dbs->ubNumberOfTiles);

		// scale hit points down to something reasonable...
		uiHitPoints = uiHitPoints * 100 / 255;
		dbs->ubHitPoints = (UINT8)uiHitPoints;
	}
}


STRUCTURE_FILE_REF* LoadStructureFile(ST::string const& filename)
{ // NB should be passed in expected number of structures so we can check equality
	std::unique_ptr<SGPFile> jsdFile{ GCM->openGameResForReading(filename) };
	auto sfr{ std::make_unique<STRUCTURE_FILE_REF>(*jsdFile) };
	if (!sfr->pubStructureData.empty())
	{
		CreateFileStructureArrays(*sfr);
	}
	// Add the file reference to the master list.
	gpStructureFileRefs.emplace(sfr.get(), filename);
	return sfr.release();
}


//
// Structure creation functions
//


static STRUCTURE* CreateStructureFromDB(DB_STRUCTURE_REF const* const pDBStructureRef, UINT8 const ubTileNum)
{ // Creates a STRUCTURE struct for one tile of a structure
	DB_STRUCTURE const* const pDBStructure = pDBStructureRef->pDBStructure;
	DB_STRUCTURE_TILE*  const pTile        = pDBStructureRef->ppTile[ubTileNum];

	STRUCTURE* const pStructure = new STRUCTURE{};

	pStructure->fFlags          = pDBStructure->fFlags;
	pStructure->pShape          = &pTile->Shape;
	pStructure->pDBStructureRef = pDBStructureRef;
	if (pTile->sPosRelToBase != 0 && ubTileNum == 0)
	{
		SLOGW("Possible bad structure {}", pDBStructureRef->pDBStructure->usStructureNumber);
	}
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
	return pStructure;
}


static BOOLEAN OkayToAddStructureToTile(INT16 const sBaseGridNo, INT16 const sCubeOffset, DB_STRUCTURE_REF const* const pDBStructureRef, UINT8 ubTileIndex, INT16 const sExclusionID, BOOLEAN const fIgnorePeople)
{ // Verifies whether a structure is blocked from being added to the map at a particular point
	DB_STRUCTURE_TILE const* const* const ppTile = pDBStructureRef->ppTile;
	INT16 const sGridNo = sBaseGridNo + ppTile[ubTileIndex]->sPosRelToBase;
	if (sGridNo < 0 || WORLD_MAX <= sGridNo) return FALSE;

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
								sOtherGridNo = NewGridNo(sGridNo, DirectionInc( bLoop + 2 ));
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
							sOtherGridNo = NewGridNo(sGridNo, DirectionInc( bLoop + 2 ));
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
						if (pOtherExistingStructure && !(pOtherExistingStructure->fFlags & STRUCTURE_WALL)) return FALSE;
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


static void AddStructureToTile(MAP_ELEMENT* const me, STRUCTURE* const s, UINT16 const structure_id)
{ // Add a STRUCTURE to a MAP_ELEMENT (Add part of a structure to a location on the map)
	STRUCTURE* const tail = me->pStructureTail;
	s->usStructureID = structure_id;
	s->pPrev         = tail;
	*(tail ? &tail->pNext : &me->pStructureHead) = s;
	me->pStructureTail = s;
	if (s->fFlags & STRUCTURE_OPENABLE) me->uiFlags |= MAPELEMENT_INTERACTIVETILE;
}


static void DeleteStructureFromTile(MAP_ELEMENT* pMapElement, STRUCTURE* pStructure);


STRUCTURE* AddStructureToWorld(INT16 const sBaseGridNo, INT8 const bLevel, DB_STRUCTURE_REF const* const pDBStructureRef, LEVELNODE* const pLevelNode)
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

	// Reserve enough space for the theoretical maximum of tiles one
	// DB_STRUCTURE can have.
	std::array<STRUCTURE *, 255> structures;

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
				delete structures[k];
			}
			return 0;
		}
		DB_STRUCTURE_TILE const* const t = ppTile[i];
		s->sGridNo = sBaseGridNo + t->sPosRelToBase;
		if (i != BASE_TILE)
		{
			if(GameMode::getInstance()->isEditorMode())
			{
				/* Kris:
				* Added this undo code if in the editor.
				* It is important to save tiles effected by multitiles.  If the
				* structure placement fails below, it doesn't matter, because it won't
				* hurt the undo code. */
				if (gfEditMode) AddToUndoList(s->sGridNo);
			}
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
				DeleteStructureFromTile(me, structures[k]);
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

	delete s;
}


BOOLEAN DeleteStructureFromWorld(STRUCTURE* const structure)
{ // removes all of the STRUCTURE elements for a structure from the world
	CHECKF(structure);

	STRUCTURE* const base = FindBaseStructure(structure);
	CHECKF(base);

	UINT16              const structure_id           = base->usStructureID;
	bool                const recompile_mps          = gsRecompileAreaLeft != 0 && !(base->fFlags & STRUCTURE_MOBILE);
	bool                const recompile_extra_radius = recompile_mps && base->fFlags & STRUCTURE_WALLSTUFF; // For doors, yuck
	GridNo              const base_grid_no           = base->sGridNo;
	DB_STRUCTURE_TILE** const tile                   = base->pDBStructureRef->ppTile;
	DB_STRUCTURE_TILE** const end_tile               = tile + base->pDBStructureRef->pDBStructure->ubNumberOfTiles;
	// Free all the tiles
	for (DB_STRUCTURE_TILE* const* i = tile; i != end_tile; ++i)
	{
		GridNo const grid_no = base_grid_no + (*i)->sPosRelToBase;
		/* There might be two structures in this tile, one on each level, but we
		 * just want to delete one on each pass */
		if (STRUCTURE* const current = FindStructureByID(grid_no, structure_id))
		{
			DeleteStructureFromTile(&gpWorldLevelData[grid_no], current);
		}

		if (gfEditMode || !recompile_mps) continue;

		AddTileToRecompileArea(grid_no);

		if (!recompile_extra_radius) continue;

		// Add adjacent tiles too
		for (UINT8 k = 0; k != NUM_WORLD_DIRECTIONS; ++k)
		{
			GridNo const check_grid_no = NewGridNo(grid_no, DirectionInc(k));
			if (check_grid_no == grid_no) continue;
			AddTileToRecompileArea(check_grid_no);
		}
	}
	return TRUE;
}


static STRUCTURE* InternalSwapStructureForPartner(STRUCTURE* const s, bool const store_in_map)
try
{
	if (!s) return 0;

	STRUCTURE* const base = FindBaseStructure(s);
	CHECKN(base);

	INT8 const delta = base->pDBStructureRef->pDBStructure->bPartnerDelta;
	if (delta == NO_PARTNER_STRUCTURE) return 0;

	// Record values, base is deleted
	GridNo                  const grid_no     = base->sGridNo;
	LEVELNODE*              const node        = FindLevelNodeBasedOnStructure(base);
	LEVELNODE*              const shadow      = FindShadow(grid_no, node->usIndex);
	bool                    const is_door     = base->fFlags & STRUCTURE_ANYDOOR;
	DB_STRUCTURE_REF const* const partner     = base->pDBStructureRef + delta;
	UINT8                   const hit_points  = base->ubHitPoints;
	INT16                   const cube_offset = base->sCubeOffset;

	// Delete the old structure and add the new one
	if (!DeleteStructureFromWorld(base)) return 0;

	STRUCTURE* const new_base = AddStructureToWorld(grid_no, (INT8)(cube_offset / PROFILE_Z_SIZE), partner, node);
	if (!new_base) return 0;

	// Set values in the new structure
	new_base->ubHitPoints = hit_points;

	if (!is_door)
	{ // Swap the graphics
		if (store_in_map) // Store removal of previous if necessary
		{
			ApplyMapChangesToMapTempFile app;
			RemoveStructFromMapTempFile(grid_no, node->usIndex);
			node->usIndex += delta;
			AddStructToMapTempFile(grid_no, node->usIndex);
		}
		else
		{
			node->usIndex += delta;
		}

		if (shadow) shadow->usIndex += delta;
	}

	return new_base;
}
catch (...) { return 0; }


STRUCTURE* SwapStructureForPartner(STRUCTURE* const s)
{
	return InternalSwapStructureForPartner(s, false);
}


STRUCTURE* SwapStructureForPartnerAndStoreChangeInMap(STRUCTURE* const s)
{
	return InternalSwapStructureForPartner(s, true);
}


STRUCTURE* FindStructure(INT16 const sGridNo, StructureFlags const flags)
{
	for (STRUCTURE* i = gpWorldLevelData[sGridNo].pStructureHead;; i = i->pNext)
	{
		if (i == NULL || i->fFlags & flags) return i;
	}
}


STRUCTURE* FindNextStructure(STRUCTURE const* const s, StructureFlags const flags)
{
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

StructureDamageResult DamageStructure(STRUCTURE* const s, UINT8 damage, StructureDamageReason const reason, GridNo const grid_no, INT16 const x, INT16 const y, SOLDIERTYPE* const owner)
{	// Do damage to a structure; returns TRUE if the structure should be removed
	Assert(s);

	if (s->fFlags & (STRUCTURE_PERSON | STRUCTURE_CORPSE))
	{ // Don't hurt this structure, it's used for hit detection only
		return STRUCTURE_NOT_DAMAGED;
	}

	UINT8 const armour_kind = s->pDBStructureRef->pDBStructure->ubArmour;
	if (armour_kind == MATERIAL_INDESTRUCTABLE_METAL) return STRUCTURE_NOT_DAMAGED;
	if (armour_kind == MATERIAL_INDESTRUCTABLE_STONE) return STRUCTURE_NOT_DAMAGED;

	if (reason == STRUCTURE_DAMAGE_EXPLOSION)
	{
		// Account for armour!
		UINT8 const base_armour = gubMaterialArmour[armour_kind];
		UINT8 const armour      = s->fFlags & STRUCTURE_EXPLOSIVE ? base_armour / 3 : base_armour / 2;
		if (damage < armour) return STRUCTURE_NOT_DAMAGED; // Didn't even scratch the paint
		// Did some damage to the structure
		damage -= armour;
	}
	else if (reason == STRUCTURE_DAMAGE_GUNFIRE)
	{
		// If here, we have penetrated, check flags
		// Are we an explodable structure?
		if (s->fFlags & STRUCTURE_EXPLOSIVE && Random(2))
		{ // Remove struct
			// ATE: Set hit points to zero
			STRUCTURE* const base = FindBaseStructure(s);
			base->ubHitPoints = 0;

			IgniteExplosionXY(owner, x, y, 0, grid_no, STRUCTURE_IGNITE, 0);

			// ATE: Return negative here, as we are dealing with deleting the graphic here
			return STRUCTURE_NOT_DAMAGED;
		}

		// Make hit sound
		SoundID const snd =
			s->fFlags & STRUCTURE_CAVEWALL ? S_VEG_IMPACT1 :
			guiMaterialHitSound[armour_kind];
		if (snd != NO_SOUND) PlayLocationJA2Sample(grid_no, snd, HIGHVOLUME, 1);

		// Don't update damage HPs
		return STRUCTURE_DESTROYED;
	}
	else
	{
		damage = 0;
	}

	// Find the base so we can reduce the hit points!
	STRUCTURE* const base = FindBaseStructure(s);
	if (!base) return STRUCTURE_NOT_DAMAGED;

	if (base->ubHitPoints <= damage) return STRUCTURE_DESTROYED; // boom! structure destroyed!
	base->ubHitPoints -= damage;

	/* Since the structure is being damaged, set the map element that a structure
	 * is damaged */
	gpWorldLevelData[grid_no].uiFlags |= MAPELEMENT_STRUCTURE_DAMAGED;

	// We are a little damaged
	return STRUCTURE_DAMAGED;
}


void DebugStructurePage1()
{
	static const ST::string WallOrientationString[] =
	{
		"None",
		"Inside left",
		"Inside right",
		"Outside left",
		"Outside right"
	};

	GridNo const grid_no = guiCurrentCursorGridNo;
	if (grid_no == NOWHERE) {
		MPageHeader("DEBUG STRUCTURES PAGE ONE");
		return;
	} else {
		MPageHeader(ST::format("DEBUG STRUCTURES PAGE ONE, GRIDNO {}", grid_no));
	}

	INT32 const h = DEBUG_PAGE_LINE_HEIGHT;
	INT32 y = DEBUG_PAGE_START_Y;

	MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y+=h, "Building:", gubBuildingInfo[grid_no]);


	bool might_have_structures = GridNoOnVisibleWorldTile(grid_no);
	INT8 n_structures = 0;
	if (might_have_structures) {
		for (STRUCTURE* i = gpWorldLevelData[grid_no].pStructureHead; i; i = i->pNext)
		{
			++n_structures;
		}
	}
	MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Number of structures:", n_structures);

	if (!might_have_structures) return;

	MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "Movement Costs:");
	MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("N {} NE {} E {} SE {} S {} SW {} W {} NW {}",
		gubWorldMovementCosts[grid_no][NORTH    ][gsInterfaceLevel],
		gubWorldMovementCosts[grid_no][NORTHEAST][gsInterfaceLevel],
		gubWorldMovementCosts[grid_no][EAST     ][gsInterfaceLevel],
		gubWorldMovementCosts[grid_no][SOUTHEAST][gsInterfaceLevel],
		gubWorldMovementCosts[grid_no][SOUTH    ][gsInterfaceLevel],
		gubWorldMovementCosts[grid_no][SOUTHWEST][gsInterfaceLevel],
		gubWorldMovementCosts[grid_no][WEST     ][gsInterfaceLevel],
		gubWorldMovementCosts[grid_no][NORTHWEST][gsInterfaceLevel]));
	MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "Ground smell:");
	MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{} of strength {}",
		SMELL_TYPE(gpWorldLevelData[grid_no].ubSmellInfo),
		SMELL_STRENGTH(gpWorldLevelData[grid_no].ubSmellInfo)));

	INT16 const desired_level = gsInterfaceLevel == I_GROUND_LEVEL ? STRUCTURE_ON_GROUND : STRUCTURE_ON_ROOF;
	for (STRUCTURE* s = gpWorldLevelData[grid_no].pStructureHead; s; s = s->pNext)
	{
		if (s->sCubeOffset != desired_level) continue;

		y += h;

		MPrintStat(DEBUG_PAGE_FIRST_COLUMN, y += h, "Structure ID:", s->usStructureID);
		MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "Type:");
		if (s->fFlags & STRUCTURE_GENERIC)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Generic structure no {}", s->pDBStructureRef->pDBStructure->usStructureNumber));
		}
		else if (s->fFlags & STRUCTURE_TREE)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Tree");
		}
		else if (s->fFlags & STRUCTURE_FENCE)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Fence with orientation {}", WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_WIREFENCE)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Wirefence with orientation {}", WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_WALL)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Wall with orientation {}", WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_WALLNWINDOW)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Wall with window with orientation {}", WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_VEHICLE)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Vehicle {}", s->pDBStructureRef->pDBStructure->usStructureNumber));
		}
		else if (s->fFlags & STRUCTURE_NORMAL_ROOF)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Roof");
		}
		else if (s->fFlags & STRUCTURE_SLANTED_ROOF)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Slanted roof");
		}
		else if (s->fFlags & STRUCTURE_TALL_ROOF)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Tall roof");
		}
		else if (s->fFlags & STRUCTURE_SWITCH)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Switch");
		}
		else if (s->fFlags & STRUCTURE_CORPSE)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Corpse");
		}
		else if (s->fFlags & STRUCTURE_PERSON)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Person");
		}
		else if (s->fFlags & STRUCTURE_CAVEWALL)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Cave wall");
		}
		else if (s->fFlags & STRUCTURE_DOOR)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Door with orientation {}", WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_SLIDINGDOOR)
		{
			ST::string state = s->fFlags & STRUCTURE_OPEN ? "Open" : "Closed";
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{} sliding door with orientation {}", state, WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_GARAGEDOOR)
		{
			ST::string state = s->fFlags & STRUCTURE_OPEN ? "Open" : "Closed";
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("{} garage door with orientation {}", state, WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_DDOOR_LEFT)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("DDoorLft with orientation {}", WallOrientationString[s->ubWallOrientation]));
		}
		else if (s->fFlags & STRUCTURE_DDOOR_RIGHT)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("DDoorRt with orientation {}", WallOrientationString[s->ubWallOrientation]));
		}
		else
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, "Unknown Structure");
		}

		MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "Flags:");
		ST::string flagString;
		if (s->fFlags & STRUCTURE_MOBILE) {
			flagString += "MOB ";
		}
		if (s->fFlags & STRUCTURE_PASSABLE) {
			flagString += "PAS ";
		}
		if (s->fFlags & STRUCTURE_EXPLOSIVE) {
			flagString += "EXP ";
		}
		if (s->fFlags & STRUCTURE_TRANSPARENT) {
			flagString += "TRA ";
		}
		if (s->fFlags & STRUCTURE_HASITEMONTOP) {
			flagString += "HIT ";
		}
		if (s->fFlags & STRUCTURE_SPECIAL) {
			flagString += "SPE ";
		}
		if (s->fFlags & STRUCTURE_LIGHTSOURCE) {
			flagString += "LIG ";
		}
		MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, flagString);

		INT8             const height = StructureHeight(s);
		STRUCTURE const* const base   = FindBaseStructure(s);
		UINT8            const armour = gubMaterialArmour[s->pDBStructureRef->pDBStructure->ubArmour];
		MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "Structure info:");
		MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format("Structure height {}, cube offset {}, armour {}, HP {}}", height, s->sCubeOffset, armour, base->ubHitPoints));

		UINT8 dens0;
		UINT8 dens1;
		UINT8 dens2;
		UINT8 dens3;
		if (StructureDensity(s, &dens0, &dens1, &dens2, &dens3))
		{
			MHeader(DEBUG_PAGE_FIRST_COLUMN, y += h, "Structure fill:");
			MPrint(DEBUG_PAGE_FIRST_COLUMN+DEBUG_PAGE_LABEL_WIDTH, y, ST::format(" {}%/{}%/{}%/{}% density {}", dens0, dens1, dens2, dens3, s->pDBStructureRef->pDBStructure->ubDensity));
		}
	}

#ifdef LOS_DEBUG
	LOSResults const& los = gLOSTestResults;
	if (los.fLOSTestPerformed)
	{
		MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("LOS from ({7d},{7d},{7d})", los.iStartX, los.iStartY, los.iStartZ));
		MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("to ({7d},{7d},{7d})", los.iEndX, los.iEndY, los.iEndZ));
		if (los.fOutOfRange)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, "is out of range");
		}
		else if (los.fLOSClear)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, "is clear!");
		}
		else
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("is blocked at ({7d},{7d},{7d})!", los.iStoppedX, los.iStoppedY, los.iStoppedZ));
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("Blocked at cube level {}", los.iCurrCubesZ));
		}
		MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("Passed through {} tree bits!", los.ubTreeSpotsHit));
		MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("Maximum range was {7d}", los.iMaxDistance));
		MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("actual range was {7d}", los.iDistance));
		if (los.ubChanceToGetThrough <= 100)
		{
			MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h, ST::format("Chance to get through was {}", los.ubChanceToGetThrough));
		}
	}
#endif

#ifdef COUNT_PATHS
	if (guiTotalPathChecks > 0)
	{
		MPrint(DEBUG_PAGE_FIRST_COLUMN, y+=h,
			ST::format("Total {}, %succ {3d} | %failed {3d} | %unsucc {3d}",
			guiTotalPathChecks,
			100 * guiSuccessfulPathChecks   / guiTotalPathChecks,
			100 * guiFailedPathChecks       / guiTotalPathChecks,
			100 * guiUnsuccessfulPathChecks / guiTotalPathChecks));
	}
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
	auto & zinfo = hVObject->ppZStripInfo;
	zinfo = std::make_unique<std::unique_ptr<ZStripInfo> []>(zcount);

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
	{
		// Defualt to true
		BOOLEAN fCopyIntoVo = TRUE;

		// Increment struct index....
		if (uiLoop == (UINT32)sNext)
		{
			sNext = uiLoop + sSTIStep;
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
				if (!(pDBStructure->fFlags & STRUCTURE_ANYDOOR) || pDBStructure->fFlags & STRUCTURE_GARAGEDOOR)
				{
					Assert(uiDestVoIndex < zcount);
					auto & pCurr = zinfo[uiDestVoIndex];
					pCurr = std::make_unique<ZStripInfo>();

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
					Assert(pCurr->ubNumberOfZChanges <= std::size(pCurr->pbZChange));
					auto end = std::fill_n(pCurr->pbZChange, ubNumIncreasing, 1);
					     end = std::fill_n(end, ubNumStable, 0);
					     end = std::fill_n(end, ubNumDecreasing, -1);
					std::fill(end, std::end(pCurr->pbZChange), 0);

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
}


INT8 GetBlockingStructureInfo( INT16 sGridNo, INT8 bDir, INT8 bNextDir, INT8 bLevel, INT8 *pStructHeight, STRUCTURE ** ppTallestStructure, BOOLEAN fWallsBlock )
{
	STRUCTURE* pStructure = NULL; // XXX HACK000E
	STRUCTURE* pCurrent;
	INT16      sDesiredLevel;
	BOOLEAN    fOKStructOnLevel = FALSE;
	BOOLEAN    fMinimumBlockingFound = FALSE;

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

			if ( pCurrent->fFlags & STRUCTURE_WALL )
			{
				if (!fWallsBlock)
				{
					// OK! This will be handled by movement costs......!
					fOKStructOnLevel = FALSE;
				}
				else
				{
					// Check how a wall is oriented relative to the direction the test ray is coming from
					const uint8_t orientation{ pCurrent->ubWallOrientation };
					if ((bDir >= NORTH && bDir <= SOUTH) && (orientation == INSIDE_TOP_RIGHT || orientation == OUTSIDE_TOP_RIGHT))
					{
						fOKStructOnLevel = FALSE;
					}
					else if ((bDir >= EAST && bDir <= WEST) && (orientation == INSIDE_TOP_LEFT || orientation == OUTSIDE_TOP_LEFT))
					{
						fOKStructOnLevel = FALSE;
					}
					else
					{
						fOKStructOnLevel = TRUE;
						break;
					}
				}
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
	UINT8  ubLoop;
	UINT32 uiBit = STRUCTURE_GENERIC;

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


STRUCTURE* FindStructureBySavedInfo(INT16 const grid_no, UINT8 const type, UINT8 const wall_orientation, INT8 const level)
{
	for (STRUCTURE* i = gpWorldLevelData[grid_no].pStructureHead; i; i = i->pNext)
	{
		if (!(i->fFlags & 1U << type))                continue;
		if (i->ubWallOrientation != wall_orientation) continue;
		if ((i->sCubeOffset == 0) != (level == 0))    continue;
		return i;
	}
	return 0;
}


SoundID GetStructureOpenSound(STRUCTURE const* const s, bool const closing)
{
	SoundID sound_id;
	switch (s->pDBStructureRef->pDBStructure->ubArmour)
	{
		case MATERIAL_LIGHT_METAL:
		case MATERIAL_THICKER_METAL: sound_id = OPEN_LOCKER;           break;
		case MATERIAL_WOOD_WALL:
		case MATERIAL_PLYWOOD_WALL:
		case MATERIAL_FURNITURE:     sound_id = OPEN_WOODEN_BOX;       break;
		default:                     sound_id = OPEN_DEFAULT_OPENABLE; break;
	}

	if (closing) sound_id = static_cast<SoundID>(sound_id + 1);

	return sound_id;
}
