#include "Buffer.h"
#include "Directories.h"
#include "Font_Control.h"
#include "LoadSaveRottingCorpse.h"
#include "MapScreen.h"
#include "Soldier_Init_List.h"
#include "Types.h"
#include "MemMan.h"
#include "Message.h"
#include "Item_Types.h"
#include "Items.h"
#include "Handle_Items.h"
#include "StrategicMap.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"
#include "SaveLoadGame.h"
#include "WorldDef.h"
#include "Rotting_Corpses.h"
#include "Overhead.h"
#include "Keys.h"
#include "Soldier_Create.h"
#include "Soldier_Profile.h"
#include "Isometric_Utils.h"
#include "Soldier_Add.h"
#include "NPC.h"
#include "AI.h"
#include "Game_Clock.h"
#include "Animation_Control.h"
#include "Map_Information.h"
//#include "PathAI.h"
#include "SaveLoadMap.h"
#include "Debug.h"
#include "Random.h"
#include "Quests.h"
#include "Animated_ProgressBar.h"
#include "Text.h"
#include "Meanwhile.h"
#include "Enemy_Soldier_Save.h"
#include "SmokeEffects.h"
#include "LightEffects.h"
#include "PathAI.h"
#include "GameVersion.h"
#include "Strategic.h"
#include "Map_Screen_Interface_Map.h"
#include "Strategic_Status.h"
#include "Soldier_Macros.h"
#include "SGP.h"
#include "MessageBoxScreen.h"
#include "Queen_Command.h"
#include "Map_Screen_Interface_Map_Inventory.h"
#include "ScreenIDs.h"
#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "Logger.h"

#include <vector>

static BOOLEAN gfWasInMeanwhile = FALSE;


static void AddTempFileToSavedGame(HWFILE const f, UINT32 const flags, SectorFlags const type, INT16 const x, INT16 const y, INT8 const z)
{
	if (!(flags & type)) return;

	ST::string const map_name = GetMapTempFileName(type, x, y, z);
	SaveFilesToSavedGame(map_name.c_str(), f);
}


static void AddTempFilesToSavedGame(HWFILE const f, UINT32 const flags, INT16 const x, INT16 const y, INT8 const z)
{
	AddTempFileToSavedGame(f, flags, SF_ITEM_TEMP_FILE_EXISTS,              x, y, z);
	AddTempFileToSavedGame(f, flags, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS,    x, y, z);
	AddTempFileToSavedGame(f, flags, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, x, y, z);
	AddTempFileToSavedGame(f, flags, SF_DOOR_TABLE_TEMP_FILES_EXISTS,       x, y, z);
	AddTempFileToSavedGame(f, flags, SF_REVEALED_STATUS_TEMP_FILE_EXISTS,   x, y, z);
	AddTempFileToSavedGame(f, flags, SF_DOOR_STATUS_TEMP_FILE_EXISTS,       x, y, z);
	AddTempFileToSavedGame(f, flags, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS,   x, y, z);
	AddTempFileToSavedGame(f, flags, SF_CIV_PRESERVED_TEMP_FILE_EXISTS,     x, y, z);
	AddTempFileToSavedGame(f, flags, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS,     x, y, z);
	AddTempFileToSavedGame(f, flags, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS,  x, y, z);
}


// SaveMapTempFilesToSavedGameFile() Looks for and opens all Map Modification files.  It add each mod file to the save game file.
void SaveMapTempFilesToSavedGameFile(HWFILE const f)
{
	//Loop though all the array elements to see if there is a data file to be saved

	//First look through the above ground sectors
	for (INT16 y = 1; y <= 16; ++y)
	{
		for (INT16 x = 1; x <= 16; ++x)
		{
			UINT32 const flags = SectorInfo[SECTOR(x, y)].uiFlags;
			AddTempFilesToSavedGame(f, flags, x, y, 0);
		}
	}

	//then look throught all the underground sectors
	for (UNDERGROUND_SECTORINFO const* u = gpUndergroundSectorInfoHead; u; u = u->next)
	{
		INT16  const x     = u->ubSectorX;
		INT16  const y     = u->ubSectorY;
		INT8   const z     = u->ubSectorZ;
		UINT32 const flags = u->uiFlags;
		AddTempFilesToSavedGame(f, flags, x, y, z);
	}
}


static void RetrieveTempFileFromSavedGame(HWFILE const f, UINT32 const flags, SectorFlags const type, const SGPSector& sector)
{
	if (!(flags & type)) return;

	ST::string const map_name = GetMapTempFileName(type, sector);
	LoadFilesFromSavedGame(map_name.c_str(), f);
}


static void SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems(const SGPSector& sMap, bool check_consistency);


static void RetrieveTempFilesFromSavedGame(HWFILE const f, UINT32& flags, INT16 const x, INT16 const y, INT8 const z, UINT32 const savegame_version)
{
	SGPSector sector(x, y, z);
	RetrieveTempFileFromSavedGame(f, flags, SF_ITEM_TEMP_FILE_EXISTS,              sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS,    sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS, sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_DOOR_TABLE_TEMP_FILES_EXISTS,       sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_REVEALED_STATUS_TEMP_FILE_EXISTS,   sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_DOOR_STATUS_TEMP_FILE_EXISTS,       sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS,   sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_CIV_PRESERVED_TEMP_FILE_EXISTS,     sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS,     sector);
	RetrieveTempFileFromSavedGame(f, flags, SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS,  sector);

	if (flags & SF_ITEM_TEMP_FILE_EXISTS)
	{
		SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems(sector, savegame_version >= 86);
	}

	if (flags & SF_CIV_PRESERVED_TEMP_FILE_EXISTS && savegame_version < 78)
	{
		// Delete the file, because it is corrupted
		GCM->tempFiles()->deleteFile(GetMapTempFileName(SF_CIV_PRESERVED_TEMP_FILE_EXISTS, sector));
		flags &= ~SF_CIV_PRESERVED_TEMP_FILE_EXISTS;
	}
}


// Load all the temp files from the saved game file and write them into the temp
// directory
void LoadMapTempFilesFromSavedGameFile(HWFILE const f, UINT32 const savegame_version)
{
	// HACK FOR GABBY
	if (savegame_version < 81)
	{
		MERCPROFILESTRUCT& gabby = GetProfile(GABBY);
		if (gabby.bMercStatus != MERC_IS_DEAD)
		{
			// turn off alternate flags for the sectors he could be in
			// randomly place him in one of the two possible sectors
			SectorInfo[SECTOR(14, MAP_ROW_L)].uiFlags &= ~SF_USE_ALTERNATE_MAP;
			SectorInfo[SECTOR( 8, MAP_ROW_L)].uiFlags &= ~SF_USE_ALTERNATE_MAP;

			INT16 x;
			INT16 y;
			if (Random(2))
			{
				x = 11;
				y = MAP_ROW_H;
			}
			else
			{
				x = 4;
				y = MAP_ROW_I;
			}
			SectorInfo[SECTOR(x, y)].uiFlags |= SF_USE_ALTERNATE_MAP;
			gabby.sSectorX = x;
			gabby.sSectorY = y;
		}
	}

	//Loop though all the array elements to see if there is a data file to be loaded

	//First look through the above ground sectors
	UINT32 counter = 0;
	for (INT16 y = 1; y <= 16; ++y)
	{
		for (INT16 x = 1; x <= 16; ++x)
		{
			UINT32& flags = SectorInfo[SECTOR(x, y)].uiFlags;
			RetrieveTempFilesFromSavedGame(f, flags, x, y, 0, savegame_version);

			UINT32 const percentage = ++counter * 100 / 255;
			RenderProgressBar(0, percentage);
		}
	}

	//then look throught all the underground sectors
	for (UNDERGROUND_SECTORINFO* u = gpUndergroundSectorInfoHead; u; u = u->next)
	{
		INT16  const x     = u->ubSectorX;
		INT16  const y     = u->ubSectorY;
		INT8   const z     = u->ubSectorZ;
		UINT32&      flags = u->uiFlags;
		RetrieveTempFilesFromSavedGame(f, flags, x, y, z, savegame_version);
	}
}


void SaveWorldItemsToTempItemFile(const SGPSector& sMap, const std::vector<WORLDITEM>& items)
{
	{
		AutoSGPFile f(GCM->tempFiles()->openForWriting(GetMapTempFileName(SF_ITEM_TEMP_FILE_EXISTS, sMap), true));
		Assert(items.size() <= UINT32_MAX);
		UINT32 numItems = static_cast<UINT32>(items.size());
		f->writeArray(numItems, items.data());
		// Close the file before
		// SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems() reads it
	}

	SetSectorFlag(sMap, SF_ITEM_TEMP_FILE_EXISTS);
	SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems(sMap, false);
}


std::vector<WORLDITEM> LoadWorldItemsFromTempItemFile(const SGPSector& sMap)
{
	ST::string const filename = GetMapTempFileName(SF_ITEM_TEMP_FILE_EXISTS, sMap);

	std::vector<WORLDITEM> l_items;
	// If the file doesn't exists, it's no problem
	if (GCM->tempFiles()->exists(filename))
	{
		AutoSGPFile f(GCM->tempFiles()->openForReading(filename));

		UINT32 numItems = 0;
		f->read(&numItems, sizeof(UINT32));
		if (numItems != 0)
		{
			l_items.assign(numItems, WORLDITEM{});
			f->read(l_items.data(), numItems * sizeof(WORLDITEM));
		}
	}
	return l_items;
}


void AddItemsToUnLoadedSector(INT16 const sMapX, INT16 const sMapY, INT8 const bMapZ, INT16 const sGridNo, UINT32 const uiNumberOfItemsToAdd, OBJECTTYPE const* const pObject, UINT8 const ubLevel, UINT16 const usFlags, INT8 const bRenderZHeightAboveLevel, Visibility const bVisible)
{
	std::vector<WORLDITEM> wis = LoadWorldItemsFromTempItemFile(SGPSector(sMapX, sMapY, bMapZ));

	//loop through all the objects to add
	for (UINT32 uiLoop1 = 0; uiLoop1 < uiNumberOfItemsToAdd; ++uiLoop1)
	{
		// Loop through the array to see if there is a free spot to add an item to it
		size_t cnt;
		for (cnt = 0;; ++cnt)
		{
			if (cnt == wis.size())
			{
				//Error, there wasnt a free spot.  Reallocate memory for the array
				wis.push_back(WORLDITEM{});
				break;
			}
			if (!wis[cnt].fExists) break;
		}

		WORLDITEM& wi = wis[cnt];
		wi.fExists                  = TRUE;
		wi.sGridNo                  = sGridNo;
		wi.ubLevel                  = ubLevel;
		wi.usFlags                  = usFlags;
		wi.bVisible                 = bVisible;
		wi.bRenderZHeightAboveLevel = bRenderZHeightAboveLevel;
		wi.o                        = pObject[uiLoop1];

		if (sGridNo == NOWHERE && !(wi.usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT))
		{
			wi.usFlags |= WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT;
			// Display warning.....
			SLOGW(
				"Trying to add item ( %d: %s ) to invalid gridno in unloaded sector. Please Report.",
				wi.o.usItem, ItemNames[wi.o.usItem].c_str());
		}
	}

	SaveWorldItemsToTempItemFile(SGPSector(sMapX, sMapY, bMapZ), wis);
}


extern BOOLEAN gfInMeanwhile;


static void SaveNPCInformationToProfileStruct(void);
static void SetLastTimePlayerWasInSector(void);
static void SaveRottingCorpsesToTempCorpseFile(const SGPSector& sector);


void SaveCurrentSectorsInformationToTempItemFile()
{
	if (gfWasInMeanwhile)
	{
		// Don't save a temp file for the meanwhile scene map.
		gfWasInMeanwhile = FALSE;
		return;
	}

	bool should_be_in_meanwhile = false;
	if (AreInMeanwhile())
	{
		gfInMeanwhile          = FALSE;
		should_be_in_meanwhile = true;
	}

	// If we haven't been to tactical yet
	if (!gWorldSector.IsValid()) return;

	SaveBloodSmellAndRevealedStatesFromMapToTempFile();

	// handle all reachable before save
	HandleAllReachAbleItemsInTheSector(gWorldSector);

	SaveWorldItemsToTempItemFile(gWorldSector, gWorldItems);
	SaveRottingCorpsesToTempCorpseFile(gWorldSector);
	SaveDoorTableToDoorTableTempFile(gWorldSector);
	SaveRevealedStatusArrayToRevealedTempFile(gWorldSector);
	SaveDoorStatusArrayToDoorStatusTempFile(gWorldSector);
	NewWayOfSavingEnemyAndCivliansToTempFile(gWorldSector, TRUE, FALSE);  // Save the enemies
	NewWayOfSavingEnemyAndCivliansToTempFile(gWorldSector, FALSE, FALSE); // Save the civilian info
	SaveSmokeEffectsToMapTempFile(gWorldSector);
	SaveLightEffectsToMapTempFile(gWorldSector);

	// Save certain information from the NPC's soldier structure to the Merc structure
	SaveNPCInformationToProfileStruct();

	// Save the time the player was last in the sector
	SetLastTimePlayerWasInSector();

	if (should_be_in_meanwhile) gfInMeanwhile = TRUE;
}


void HandleAllReachAbleItemsInTheSector(const SGPSector& sector)
{ // Find out which items in the list are reachable
	if (gWorldItems.size() == 0) return;

	MAPCREATE_STRUCT const& m = gMapInformation;
	GridNo grid_no = m.sCenterGridNo;
	if (grid_no == -1) grid_no = m.sNorthGridNo;
	if (grid_no == -1) grid_no = m.sEastGridNo;
	if (grid_no == -1) grid_no = m.sSouthGridNo;
	if (grid_no == -1) grid_no = m.sWestGridNo;
	AssertMsg(grid_no != -1, String("Map %c%d_b%d does not have any entry points!", 'A' + sector.y - 1, sector.x, sector.z));
	if (grid_no == -1) return;

	GridNo       grid_no2 = NOWHERE;
	GridNo const isolated = gMapInformation.sIsolatedGridNo;
	if (isolated != -1)
	{
		FOR_EACH_IN_TEAM(s, OUR_TEAM)
		{
			if (s->bLife <= 0) continue;
			if (s->sSectorX != sector.x) continue;
			if (s->sSectorY != sector.y) continue;
			if (s->bSectorZ != sector.z) continue;
			if (!FindBestPath(s, isolated, s->bLevel, WALKING, NO_COPYROUTE, 0)) continue;
			grid_no2 = isolated;
			break;
		}
	}

	GlobalItemsReachableTest(grid_no, grid_no2);

	FOR_EACH_WORLD_ITEM(wi)
	{
		bool reachable;
		if (wi.o.bTrap > 0)
		{
			// If the item is trapped, then flag it as unreachable
			reachable = false;
		}
		else if (ItemTypeExistsAtLocation(wi.sGridNo, OWNERSHIP, wi.ubLevel, 0))
		{
			reachable = false;
		}
		else if (wi.o.usItem == CHALICE)
		{
			reachable = false;
		}
		else if (gpWorldLevelData[wi.sGridNo].uiFlags & MAPELEMENT_REACHABLE)
		{
			// The gridno itself is reachable, so the item is reachable
			reachable = true;
		}
		else if (wi.ubLevel > 0)
		{
			// Items on roofs are always reachable
			reachable = true;
		}
		else
		{
			// Check the 4 grids around the item. If any is reachable, then the item is reachable
			reachable = false;
			for (UINT8 dir = 0; dir != NUM_WORLD_DIRECTIONS; dir += 2)
			{
				GridNo const new_loc = NewGridNo(wi.sGridNo, DirectionInc(dir));
				if (new_loc == wi.sGridNo) continue;

				// then it's a valid gridno, so test it
				// requires non-wall movement cost from one location to the other!
				if (!(gpWorldLevelData[new_loc].uiFlags & MAPELEMENT_REACHABLE)) continue;

				UINT8 const movement_cost = gubWorldMovementCosts[wi.sGridNo][OppositeDirection(dir)][0];
				// If we find a door movement cost, if the door is open the gridno
				// should be accessible itself
				if (movement_cost == TRAVELCOST_DOOR) continue;
				if (movement_cost == TRAVELCOST_WALL) continue;

				reachable = true;
				break;
			}
		}

		if (reachable)
		{
			wi.usFlags |= WORLD_ITEM_REACHABLE;
		}
		else
		{
			wi.usFlags &= ~WORLD_ITEM_REACHABLE;
		}
	}
}


static UINT32 GetSectorFlags(const SGPSector& sector)
{
	if (sector.z == 0)
	{
		return SectorInfo[sector.AsByte()].uiFlags;
	}
	else
	{
		UNDERGROUND_SECTORINFO const* const u = FindUnderGroundSector(sector);
		return u ? u->uiFlags : 0;
	}
}


static void   LoadAndAddWorldItemsFromTempFile(const SGPSector& sector);
static UINT32 GetLastTimePlayerWasInSector(void);
static void   LoadRottingCorpsesFromTempCorpseFile(const SGPSector& sMap);


void LoadCurrentSectorsInformationFromTempItemsFile()
{
	UINT32 const flags = GetSectorFlags(gWorldSector);

	if (AreInMeanwhile())
	{
		// There will never be a temp file for the meanwhile scene, so return TRUE.
		// However, set a flag to not save it either!
		gfWasInMeanwhile = TRUE;

		// OK  - this is true except for interrogations - we need that item temp
		// file to be processed!
		if (GetMeanwhileID() == INTERROGATION)
		{
			//If there is a file, load in the Items array
			if (flags & SF_ITEM_TEMP_FILE_EXISTS)
			{
				LoadAndAddWorldItemsFromTempFile(gWorldSector);
			}

			gfWasInMeanwhile = FALSE;
		}
		return;
	}

	if (flags & SF_ITEM_TEMP_FILE_EXISTS)
	{
		LoadAndAddWorldItemsFromTempFile(gWorldSector);
	}

	if (flags & SF_ROTTING_CORPSE_TEMP_FILE_EXISTS)
	{
		LoadRottingCorpsesFromTempCorpseFile(gWorldSector);
	}

	if (flags & SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS)
	{
		LoadAllMapChangesFromMapTempFileAndApplyThem();
	}

	if (flags & SF_DOOR_TABLE_TEMP_FILES_EXISTS)
	{
		LoadDoorTableFromDoorTableTempFile();
	}

	if (flags & SF_REVEALED_STATUS_TEMP_FILE_EXISTS)
	{
		LoadRevealedStatusArrayFromRevealedTempFile();
	}

	if (flags & SF_DOOR_STATUS_TEMP_FILE_EXISTS)
	{
		LoadDoorStatusArrayFromDoorStatusTempFile();
	}

	// if the save is an older version, use the old way of loading it up
	if (guiSavedGameVersion < 57)
	{
		if (flags & SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS)
		{
			LoadEnemySoldiersFromTempFile();
		}
	}
	else
	{
		// use the new way of loading the enemy and civilian placements
		if (flags & SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS)
		{
			NewWayOfLoadingEnemySoldiersFromTempFile();
		}
		if (flags & SF_CIV_PRESERVED_TEMP_FILE_EXISTS)
		{
			NewWayOfLoadingCiviliansFromTempFile();
		}
	}

	if (flags & SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS)
	{
		LoadSmokeEffectsFromMapTempFile(gWorldSector);
	}

	if (flags & SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS)
	{
		LoadLightEffectsFromMapTempFile(gWorldSector);
	}

	// Init the world since we have modified the map
	InitLoadedWorld();

	guiTimeCurrentSectorWasLastLoaded = GetLastTimePlayerWasInSector();

	StripEnemyDetailedPlacementsIfSectorWasPlayerLiberated();
}


static void SetLastTimePlayerWasInSector(void)
{
	if (gWorldSector.z == 0)
	{
		SectorInfo[gWorldSector.AsByte()].uiTimeCurrentSectorWasLastLoaded = GetWorldTotalMin();
	}
	else if (gWorldSector.z > 0)
	{
		UNDERGROUND_SECTORINFO* const u = FindUnderGroundSector(gWorldSector);
		if (!u)
		{
			SLOGW("Failed to Set the 'uiTimeCurrentSectorWasLastLoaded' for an underground sector");
			return;
		}
		u->uiTimeCurrentSectorWasLastLoaded = GetWorldTotalMin();
	}
}


static UINT32 GetLastTimePlayerWasInSector(void)
{
	if (gWorldSector.z == 0)
	{
		return SectorInfo[gWorldSector.AsByte()].uiTimeCurrentSectorWasLastLoaded;
	}
	else if (gWorldSector.z > 0)
	{
		UNDERGROUND_SECTORINFO const* const u = FindUnderGroundSector(gWorldSector);
		if (!u)
		{
			SLOGW("Failed to Get the 'uiTimeCurrentSectorWasLastLoaded' from an underground sector");
			return 0;
		}
		return u->uiTimeCurrentSectorWasLastLoaded;
	}
	return 0;
}


static void LoadAndAddWorldItemsFromTempFile(const SGPSector& sMap)
{
	std::vector<WORLDITEM> items = LoadWorldItemsFromTempItemFile(sMap);

	// Have we already been to the sector?
	if (GetSectorFlagStatus(sMap, SF_ALREADY_LOADED))
	{
		// Completly replace the current sectors item table because all the items
		// SHOULD be in the temp file.
		TrashWorldItems();
	}

	if (items.size() == 0) return;

	// Add the items in the file to the current sector's item table
	for (WORLDITEM& wi : items)
	{
		if (!wi.fExists) continue;

		GridNo pos = wi.sGridNo;
		if (wi.usFlags & WORLD_ITEM_GRIDNO_NOT_SET_USE_ENTRY_POINT)
		{
			// The item has an invalid gridno, use the maps entry point
			pos = gMapInformation.sCenterGridNo;
		}
		else if (wi.usFlags & WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO)
		{
			// Find a gridno to place the item at
			GridNo new_pos = FindNearestAvailableGridNoForItem(pos, 5);
			if (new_pos == NOWHERE)
			{
				new_pos = FindNearestAvailableGridNoForItem(pos, 15);
			}
			if (new_pos != NOWHERE) pos = new_pos;
		}

		AddItemToPool(pos, &wi.o, static_cast<Visibility>(wi.bVisible), wi.ubLevel, wi.usFlags, wi.bRenderZHeightAboveLevel);
	}
}


void InitTacticalSave()
{
	GCM->tempFiles()->createDir(TACTICAL_SAVE_TEMPDIR);
	GCM->tempFiles()->eraseDir(TACTICAL_SAVE_TEMPDIR);
}


static void SaveRottingCorpsesToTempCorpseFile(const SGPSector& sector)
{
	AutoSGPFile f(GCM->tempFiles()->openForWriting(GetMapTempFileName(SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, sector), true));

	// Save the number of the rotting corpses
	UINT32 n_corpses = 0;
	CFOR_EACH_ROTTING_CORPSE(c) ++n_corpses;
	f->write(&n_corpses, sizeof(UINT32));

	// Loop through all the carcases in the array and save the active ones
	CFOR_EACH_ROTTING_CORPSE(c)
	{
		InjectRottingCorpseIntoFile(f, &c->def);
	}

	SetSectorFlag(sector, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS);
}


static void LoadRottingCorpsesFromTempCorpseFile(const SGPSector& sMap)
{
	RemoveCorpses();

	ST::string const map_name = GetMapTempFileName(SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, sMap);

	// If the file doesn't exist, it's no problem.
	if (!GCM->tempFiles()->exists(map_name)) return;

	AutoSGPFile f(GCM->tempFiles()->openForReading(map_name));

	// Load the number of Rotting corpses
	UINT32 n_corpses;
	f->read(&n_corpses, sizeof(UINT32));

	bool const maybe_dont_add = !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME) &&
					sMap.z == 0 && GetTownIdForSector(sMap.AsByte()) != BLANK_SECTOR; // In town?
	for (UINT32 n = n_corpses; n != 0; --n)
	{
		ROTTING_CORPSE_DEFINITION def;
		ExtractRottingCorpseFromFile(f, &def);

		// Check the flags to see if we have to find a gridno to place the rotting
		// corpses at
		if (def.usFlags & ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO)
		{
			def.sGridNo = FindNearestAvailableGridNoForCorpse(&def, 5);
			if (def.sGridNo == NOWHERE)
				def.sGridNo = FindNearestAvailableGridNoForCorpse(&def, 15);
			// ATE: Here we still could have a bad location, but send in NOWHERE to
			// corpse function anyway, 'cause it will iwth not drop it or use a map
			// edgepoint
		}
		else if (def.usFlags & ROTTING_CORPSE_USE_NORTH_ENTRY_POINT)
		{
			def.sGridNo = gMapInformation.sNorthGridNo;
		}
		else if (def.usFlags & ROTTING_CORPSE_USE_SOUTH_ENTRY_POINT)
		{
			def.sGridNo = gMapInformation.sSouthGridNo;
		}
		else if (def.usFlags & ROTTING_CORPSE_USE_EAST_ENTRY_POINT)
		{
			def.sGridNo = gMapInformation.sEastGridNo;
		}
		else if (def.usFlags & ROTTING_CORPSE_USE_WEST_ENTRY_POINT)
		{
			def.sGridNo = gMapInformation.sWestGridNo;
		}

		// Don't add a corpse on an invalid gridno
		if (def.sGridNo >= WORLD_MAX || def.sGridNo < 0) continue;

		// ATE: Don't place corpses if not loading a savegame, in town, indoors and
		// the corpse is too old
		if (maybe_dont_add &&
				FloorAtGridNo(def.sGridNo) && // Are we indoors?
				GetWorldTotalMin() - def.uiTimeOfDeath >= 30)
		{
			continue;
		}
		if (!AddRottingCorpse(&def))
		{
			SLOGD("Failed to add a corpse to GridNo # %d", def.sGridNo);
		}
	}

	// Check to see if we have to start decomposing the corpses
	HandleRottingCorpses();
}


void AddWorldItemsToUnLoadedSector(const INT16 sMapX, const INT16 sMapY, const INT8 bMapZ, const std::vector<WORLDITEM>& wis)
{
	for (const WORLDITEM& wi : wis)
	{
		if (!wi.fExists) continue;
		AddItemsToUnLoadedSector(sMapX, sMapY, bMapZ, wi.sGridNo, 1, &wi.o, wi.ubLevel, wi.usFlags, wi.bRenderZHeightAboveLevel, static_cast<Visibility>(wi.bVisible));
	}
}


static void SaveNPCInformationToProfileStruct(void)
{
	// Only do this on save now... on traversal this is handled in the strategic code
	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME)) return;

	FOR_EACH_MERC(i)
	{
		const SOLDIERTYPE* const s = *i;
		//if it is an active NPC
		if (s->ubProfile == NO_PROFILE || s->bTeam != CIV_TEAM) continue;

		MERCPROFILESTRUCT& p = GetProfile(s->ubProfile);

		p.ubQuoteActionID = s->ubQuoteActionID;
		p.ubQuoteRecord   = s->ubQuoteRecord;

		// if the merc is NOT added due to flag set, return
		if (p.ubMiscFlags2 & PROFILE_MISC_FLAG2_DONT_ADD_TO_SECTOR)       continue;
		if (p.ubMiscFlags3 & PROFILE_MISC_FLAG3_PERMANENT_INSERTION_CODE) continue;

		p.fUseProfileInsertionInfo = TRUE;
		p.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
		p.usStrategicInsertionData = s->sGridNo;
	}
}


void ChangeNpcToDifferentSector(MERCPROFILESTRUCT& p, const SGPSector& sSector)
{
	SGPSector nextSector = sSector;
	if (p.ubMiscFlags2 & PROFILE_MISC_FLAG2_LEFT_COUNTRY)
	{
		// override location, this person is OUTTA here
		nextSector = SGPSector();
	}
	// Carmen no longer traverses out, he is temporarily removed instead

	p.sSectorX = nextSector.x;
	p.sSectorY = nextSector.y;
	p.bSectorZ = nextSector.z;
	p.fUseProfileInsertionInfo = FALSE;
}


void AddRottingCorpseToUnloadedSectorsRottingCorpseFile(const SGPSector& sMap, ROTTING_CORPSE_DEFINITION const* const corpse_def)
{
	AutoSGPFile f(GCM->tempFiles()->openForReadWrite(GetMapTempFileName(SF_ROTTING_CORPSE_TEMP_FILE_EXISTS, sMap)));

	UINT32 corpse_count;
	if (f->size() != 0)
	{
		f->read(&corpse_count, sizeof(corpse_count));
		f->seek(0, FILE_SEEK_FROM_START);
	}
	else
	{
		corpse_count = 0;
	}

	++corpse_count;
	f->write(&corpse_count, sizeof(corpse_count));

	f->seek(0, FILE_SEEK_FROM_END);
	InjectRottingCorpseIntoFile(f, corpse_def);

	SetSectorFlag(sMap, SF_ROTTING_CORPSE_TEMP_FILE_EXISTS);
}


void SetSectorFlag(const SGPSector& sector, SectorFlags const flag_to_set)
{
	static const SGPSector tixa(TIXA_SECTOR_X, TIXA_SECTOR_Y);
	static const SGPSector gunrange(GUN_RANGE_X, GUN_RANGE_Y, GUN_RANGE_Z);
	if (flag_to_set == SF_ALREADY_VISITED)
	{
		// Do certain things when particular sectors are visited
		if (sector == tixa)
		{
			// Tixa prison (not seen until Tixa visited)
			SectorInfo[SEC_J9].uiFacilitiesFlags |= SFCF_PRISON;
		}
		else if (sector == gunrange)
		{
			// Alma shooting range (not seen until sector visited)
			SectorInfo[SEC_H13].uiFacilitiesFlags |= SFCF_GUN_RANGE;
			SectorInfo[SEC_H14].uiFacilitiesFlags |= SFCF_GUN_RANGE;
			SectorInfo[SEC_I13].uiFacilitiesFlags |= SFCF_GUN_RANGE;
			SectorInfo[SEC_I14].uiFacilitiesFlags |= SFCF_GUN_RANGE;
		}

		// Increment daily counter of sectors visited
		if (!GetSectorFlagStatus(sector, SF_ALREADY_VISITED) &&
			++gStrategicStatus.ubNumNewSectorsVisitedToday == NEW_SECTORS_EQUAL_TO_ACTIVITY)
		{
			// Visited enough to count as an active day
			UpdateLastDayOfPlayerActivity(GetWorldDay());
		}
	}

	if (sector.z == 0)
	{
		SectorInfo[sector.AsByte()].uiFlags |= flag_to_set;
	}
	else
	{
		UNDERGROUND_SECTORINFO* const u = FindUnderGroundSector(sector);
		if (u) u->uiFlags |= flag_to_set;
	}
}

void ReSetSectorFlag(const SGPSector& sector, SectorFlags const flag_to_clear)
{
	if (sector.z == 0)
	{
		SectorInfo[sector.AsByte()].uiFlags &= ~flag_to_clear;
	}
	else
	{
		UNDERGROUND_SECTORINFO* const u = FindUnderGroundSector(sector);
		if (u) u->uiFlags &= ~flag_to_clear;
	}
}


BOOLEAN GetSectorFlagStatus(const SGPSector& sSector, SectorFlags const flag_to_check)
{
	return (GetSectorFlags(sSector) & flag_to_check) != 0;
}


void AddDeadSoldierToUnLoadedSector(const SGPSector& sMap, SOLDIERTYPE* const s, INT16 const grid_no, UINT32 const flags)
{
	// Setup the flags for the items and the rotting corpses
	UINT16 flags_for_world_items    = 0;
	UINT16 flags_for_rotting_corpse = 0;
	if (flags & ADD_DEAD_SOLDIER_USE_GRIDNO)
	{
		flags_for_world_items    = 0;
		flags_for_rotting_corpse = 0;
	}
	else if (flags & ADD_DEAD_SOLDIER_TO_SWEETSPOT)
	{
		flags_for_world_items    = WOLRD_ITEM_FIND_SWEETSPOT_FROM_GRIDNO | WORLD_ITEM_REACHABLE;
		flags_for_rotting_corpse = ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO;
	}
	else
	{
		SLOGA("Flag not is Switch statement");
	}

	//Create an array of objects from the mercs inventory

	// Loop through all the soldier's items and add them to the world item array
	if (!AM_A_ROBOT(s)) // If a robot, don't drop anything
	{
		FOR_EACH_SOLDIER_INV_SLOT(i, *s)
		{
			OBJECTTYPE& o = *i;
			if (o.usItem == NOTHING) continue;

			if (s->bTeam != OUR_TEAM)
			{
				// This percent of the time, they don't drop stuff they would've dropped
				// in tactical
				if (Random(100) < 75)
				{
					o.fFlags |= OBJECT_UNDROPPABLE;
					continue;
				}

				if (o.fFlags & OBJECT_UNDROPPABLE) continue;
			}

			ReduceAmmoDroppedByNonPlayerSoldiers(*s, o);
			AddItemsToUnLoadedSector(sMap.x, sMap.y, sMap.z, grid_no, 1, &o, s->bLevel, flags_for_world_items, 0, VISIBLE);
		}
	}

	DropKeysInKeyRing(*s, grid_no, s->bLevel, VISIBLE, false, 0, true);

	// Convert the soldier into a rotting corpse
	ROTTING_CORPSE_DEFINITION c;
	c = ROTTING_CORPSE_DEFINITION{};
	c.ubBodyType        = s->ubBodyType;
	c.sGridNo           = grid_no;
	c.sHeightAdjustment = s->sHeightAdjustment;
	c.bVisible          = TRUE;
	c.HeadPal           = s->HeadPal;
	c.VestPal           = s->VestPal;
	c.SkinPal           = s->SkinPal;
	c.PantsPal          = s->PantsPal;
	c.bDirection        = s->bDirection;
	c.uiTimeOfDeath     = GetWorldTotalMin();
	c.usFlags           = flags_for_rotting_corpse;

	static UINT32 const possible_death_anims[] =
	{
		GENERIC_HIT_DEATH,
		FALLBACK_HIT_DEATH,
		PRONE_HIT_DEATH,
		FLYBACK_HIT_DEATH
	};
	// If the dead body shot be the result of a JFK headshot, set it, else choose
	// a random death sequence
	UINT32 const death_anim = flags & ADD_DEAD_SOLDIER__USE_JFK_HEADSHOT_CORPSE ? JFK_HITDEATH :
					possible_death_anims[Random(lengthof(possible_death_anims))];
	c.ubType = gubAnimSurfaceCorpseID[s->ubBodyType][death_anim];

	AddRottingCorpseToUnloadedSectorsRottingCorpseFile(sMap, &c);
}


void SaveTempNpcQuoteArrayToSaveGameFile(HWFILE const f)
{
	// Write zero size marker for the obsolescent temporary NPC quote file.
	UINT32 const size = 0;
	f->write(&size, sizeof(size));
}


void LoadTempNpcQuoteArrayToSaveGameFile(HWFILE const f)
{
	// Skip obsolescent temporary NPC quote file.
	UINT32 size;
	f->read(&size, sizeof(size));
	f->seek(size, FILE_SEEK_FROM_CURRENT);
}


static void TempFileLoadErrorMessageReturnCallback(MessageBoxReturnValue const ubRetVal)
{
	requestGameExit();
}


//if you call this function, make sure you return TRUE (if applicable) to make the game
//think it succeeded the load.  This sets up the dialog for the game exit, after the hacker
//message appears.
void InitExitGameDialogBecauseFileHackDetected()
{
	DoMessageBox(MSG_BOX_BASIC_STYLE, pAntiHackerString, GAME_SCREEN, MSG_BOX_FLAG_OK, TempFileLoadErrorMessageReturnCallback, NULL);
}


static UINT8 const* GetRotationArray();


void NewJA2EncryptedFileRead(HWFILE const f, BYTE* const pDest, UINT32 const uiBytesToRead)
{
	f->read(pDest, uiBytesToRead);

	const UINT8* const pubRotationArray = GetRotationArray();
	UINT8              ubArrayIndex     = 0;
	UINT8              ubLastByte       = 0;
	for (UINT32 i = 0; i < uiBytesToRead; ++i)
	{
		UINT8 const ubLastByteForNextLoop = pDest[i];
		pDest[i] -= ubLastByte + pubRotationArray[ubArrayIndex];
		if (++ubArrayIndex >= NEW_ROTATION_ARRAY_SIZE) ubArrayIndex = 0;
		ubLastByte = ubLastByteForNextLoop;
	}
}


void NewJA2EncryptedFileWrite(HWFILE const hFile, BYTE const* const data, UINT32 const uiBytesToWrite)
{
	SGP::Buffer<UINT8> buf(uiBytesToWrite);
	const UINT8* const pubRotationArray = GetRotationArray();
	UINT8              ubArrayIndex     = 0;
	UINT8              last_byte        = 0;
	for (UINT32 i = 0; i < uiBytesToWrite; ++i)
	{
		buf[i] = data[i] + last_byte + pubRotationArray[ubArrayIndex];
		last_byte = buf[i];
		if (++ubArrayIndex >= NEW_ROTATION_ARRAY_SIZE) ubArrayIndex = 0;
	}

	hFile->write(buf, uiBytesToWrite);
}


#define ROTATION_ARRAY_SIZE 46
static const UINT8 ubRotationArray[46] = { 132, 235, 125, 99, 15, 220, 140, 89, 205, 132, 254, 144, 217, 78, 156, 58, 215, 76, 163, 187, 55, 49, 65, 48, 156, 140, 201, 68, 184, 13, 45, 69, 102, 185, 122, 225, 23, 250, 160, 220, 114, 240, 64, 175, 057, 233 };

void JA2EncryptedFileRead(HWFILE const f, BYTE* const pDest, UINT32 const uiBytesToRead)
{
	f->read(pDest, uiBytesToRead);

	UINT8 ubArrayIndex = 0;
	UINT8 ubLastByte   = 0;
	for (UINT32 i = 0; i < uiBytesToRead; ++i)
	{
		UINT8 const ubLastByteForNextLoop = pDest[i];
		pDest[i] -= ubLastByte + ubRotationArray[ubArrayIndex];
		if (++ubArrayIndex >= ROTATION_ARRAY_SIZE) ubArrayIndex = 0;
		ubLastByte = ubLastByteForNextLoop;
	}
}


void JA2EncryptedFileWrite(HWFILE const hFile, BYTE const* const data, UINT32 const uiBytesToWrite)
{
	SGP::Buffer<UINT8> buf(uiBytesToWrite);
	UINT8              ubArrayIndex = 0;
	UINT8              last_byte    = 0;
	for (UINT32 i = 0; i < uiBytesToWrite; ++i)
	{
		buf[i] += data[i] + last_byte + ubRotationArray[ubArrayIndex];
		last_byte = buf[i];
		if (++ubArrayIndex >= ROTATION_ARRAY_SIZE) ubArrayIndex = 0;
	}

	hFile->write(buf, uiBytesToWrite);
}

ST::string GetMapTempFileName(SectorFlags uiType, const SGPSector& sector)
{
	return GetMapTempFileName(uiType, sector.x, sector.y, sector.z);
}

ST::string GetMapTempFileName(SectorFlags const uiType, INT16 const sMapX, INT16 const sMapY, INT8 const bMapZ)
{
	// Convert the current sector location into a file name
	char zTempName[512];
	GetMapFileName(SGPSector(sMapX, sMapY, bMapZ), zTempName, FALSE);

	const char* prefix;
	switch (uiType)
	{
		case SF_ITEM_TEMP_FILE_EXISTS:              prefix = "i";  break;
		case SF_ROTTING_CORPSE_TEMP_FILE_EXISTS:    prefix = "r";  break;
		case SF_MAP_MODIFICATIONS_TEMP_FILE_EXISTS: prefix = "m";  break;
		case SF_DOOR_TABLE_TEMP_FILES_EXISTS:       prefix = "d";  break;
		case SF_REVEALED_STATUS_TEMP_FILE_EXISTS:   prefix = "v";  break;
		case SF_DOOR_STATUS_TEMP_FILE_EXISTS:       prefix = "ds"; break;
		case SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS:   prefix = "e";  break;
		case SF_SMOKE_EFFECTS_TEMP_FILE_EXISTS:     prefix = "sm"; break;
		case SF_LIGHTING_EFFECTS_TEMP_FILE_EXISTS:  prefix = "l";  break;
		case SF_CIV_PRESERVED_TEMP_FILE_EXISTS:     prefix = "c";  break;

		default: SLOGA("GetMapTempFileName: invalid Type"); return "";
	}
	return ST::format(TACTICAL_SAVE_TEMPDIR "/{}_{}", prefix, static_cast<char const*>(zTempName));
}


static UINT32 UpdateLoadedSectorsItemInventory(const SGPSector& sector, UINT32 n_items);


UINT32 GetNumberOfVisibleWorldItemsFromSectorStructureForSector(const SGPSector& sMap)
{
	UINT32 n_items;
	if (sMap.z == 0)
	{
		n_items = SectorInfo[sMap.AsByte()].uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer;
	}
	else
	{
		//find the underground sector
		UNDERGROUND_SECTORINFO const* const u = FindUnderGroundSector(sMap);
		n_items = u ? u->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer : 0;
	}

	// If the requested sector is currently loaded
	if (gfWorldLoaded && sMap == gWorldSector)
	{
		// Since items might have been added, update
		n_items = UpdateLoadedSectorsItemInventory(sMap, n_items);
	}

	return n_items;
}


void SetNumberOfVisibleWorldItemsInSectorStructureForSector(const SGPSector& sector, UINT32 const n_items)
{
	if (sector.z == 0)
	{
		SectorInfo[sector.AsByte()].uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer = n_items;
	}
	else
	{
		UNDERGROUND_SECTORINFO* const u = FindUnderGroundSector(sector);
		if (u) u->uiNumberOfWorldItemsInTempFileThatCanBeSeenByPlayer = n_items;
	}
}


static void SynchronizeItemTempFileVisbleItemsToSectorInfoVisbleItems(const SGPSector& sMap, bool const check_consistency)
{
	std::vector<WORLDITEM> pTotalSectorList = LoadWorldItemsFromTempItemFile(sMap);

	UINT32 uiItemCount = 0;
	if (pTotalSectorList.size() > 0)
	{
		for (const WORLDITEM& wi : pTotalSectorList)
		{
			if (!IsMapScreenWorldItemVisibleInMapInventory(wi)) continue;
			uiItemCount += wi.o.ubNumberOfObjects;
		}
		pTotalSectorList.clear();
	}

	if (check_consistency)
	{
		const UINT32 uiReported = GetNumberOfVisibleWorldItemsFromSectorStructureForSector(sMap);
		if (uiItemCount != uiReported)
		{
			SLOGW("SynchronizeItemTempFile() Reported %d, should be %d", uiReported, uiItemCount);
		}
	}
	SetNumberOfVisibleWorldItemsInSectorStructureForSector(sMap, uiItemCount);
}


static UINT32 UpdateLoadedSectorsItemInventory(const SGPSector& sector, UINT32 const n_items)
{
	UINT32 n = 0;
	CFOR_EACH_WORLD_ITEM(wi)
	{
		if (!IsMapScreenWorldItemVisibleInMapInventory(wi)) continue;
		n += wi.o.ubNumberOfObjects;
	}

	// Update the value in the sector info struct, if the item count is different
	if (n != n_items)
	{
		SetNumberOfVisibleWorldItemsInSectorStructureForSector(sector, n);
	}

	return n;
}


static UINT8 const g_encryption_array[][NEW_ROTATION_ARRAY_SIZE] =
{
	{
		11,129,18,136,163,80,128,
		53,174,146,188,240,208,162,
		127,192,251,6,52,128,119,
		123,234,131,60,66,171,237,
		89,192,17,37,139,20,185,
		48,218,176,116,87,91,156,
		166,224,215,100,237,71,157
	},

	{
		99,93,179,25,113,109,38,
		135,144,248,17,108,178,84,
		210,254,54,240,209,92,35,
		121,166,153,203,239,27,134,
		231,193,97,73,188,59,87,
		146,111,103,48,161,25,196,
		84,201,179,208,220,189,113
	},

	{
		217,122,77,162,22,25,203,
		190,115,43,235,154,27,227,
		44,125,34,58,157,17,83,
		26,63,221,159,135,149,85,
		26,32,201,22,253,189,250,
		113,185,171,252,214,115,41,
		65,49,3,174,27,220,206
	},

	{
		180,29,63,146,101,192,164,
		128,7,28,111,172,125,203,
		63,77,184,220,27,184,98,
		224,64,156,178,229,167,105,
		169,147,215,103,168,25,40,
		114,156,45,253,198,179,207,
		35,235,225,20,205,34,14
	},

	{
		144,113,83,76,208,239,2,
		236,97,66,38,195,185,52,
		121,123,196,66,34,78,50,
		66,57,247,79,216,38,63,
		155,107,62,234,218,142,175,
		125,209,159,31,135,99,143,
		250,243,90,127,249,32,121
	},

	{
		76,189,22,40,143,9,197,
		91,66,142,145,3,138,202,
		4,170,214,219,20,140,212,
		214,183,197,98,142,57,235,
		16,71,13,186,77,72,147,
		14,206,204,152,226,86,66,
		219,148,100,89,148,22,243
	},

	{
		206,52,86,217,117,60,134,
		215,114,154,137,4,37,66,
		48,147,170,113,137,175,130,
		112,246,87,69,40,64,249,
		227,97,91,216,102,11,40,
		195,173,91,205,85,32,40,
		248,63,194,73,172,181,184
	},

	{
		95,234,168,147,217,213,204,
		132,140,158,22,201,182,222,
		191,104,206,165,108,154,226,
		236,135,245,247,166,112,180,
		49,43,146,43,23,40,249,
		206,228,226,135,211,33,241,
		74,214,104,47,44,132,187
	},

	{
		26,121,193,40,90,98,61,
		42,168,13,248,146,203,80,
		166,90,223,172,107,45,168,
		161,239,10,34,239,59,209,
		4,231,81,23,120,85,12,
		12,182,35,39,229,187,83,
		243,142,218,94,179,241,90
	},

	{
		142,9,212,220,121,125,115,
		191,40,144,216,56,6,76,
		46,90,51,206,241,201,77,
		202,90,67,73,116,203,199,
		115,197,174,133,56,174,132,
		91,185,8,143,5,221,216,
		232,211,225,219,86,5,16
	},

	{
		195,200,131,175,42,189,78,
		186,174,122,45,142,55,106,
		248,10,204,112,38,82,5,
		154,209,193,124,244,5,29,
		79,237,97,175,16,8,250,
		93,44,79,143,63,64,84,
		205,244,74,6,242,42,58
	},

	{
		82,107,124,208,214,175,77,
		77,123,32,162,220,87,3,
		156,62,171,114,178,161,88,
		102,51,47,152,147,54,41,
		105,91,137,53,107,125,183,
		174,142,207,195,133,127,164,
		165,161,2,37,44,65,60
	},

	{
		79,86,68,37,213,189,232,
		15,143,41,230,136,191,222,
		103,133,217,205,204,23,10,
		45,197,140,55,160,166,99,
		169,119,117,198,35,51,178,
		15,139,203,66,8,252,236,
		125,90,88,169,222,244,35
	},

	{
		206,62,201,224,92,24,183,
		166,85,215,246,29,100,157,
		224,164,126,118,116,219,18,
		120,92,60,230,206,224,4,
		51,19,146,37,19,255,11,
		123,233,236,157,232,207,13,
		66,177,158,229,169,180,180
	},

	{
		254,188,196,16,50,108,215,
		198,166,37,72,138,227,191,
		212,60,245,70,171,57,30,
		97,27,98,4,211,66,152,
		225,198,91,154,166,86,234,
		10,124,31,130,130,182,56,
		85,134,152,104,78,51,237
	},

	{
		236,233,113,237,175,131,167,
		99,63,81,142,139,103,199,
		62,171,185,154,14,203,229,
		21,89,153,52,2,198,224,
		253,212,40,155,123,88,134,
		5,188,144,130,185,34,11,
		131,71,64,82,241,56,101
	},

	{
		201,241,254,183,146,209,209,
		56,204,180,211,122,211,146,
		154,117,190,49,116,68,216,
		29,125,35,65,69,224,144,
		123,84,91,223,20,83,90,
		22,36,186,200,111,220,126,
		240,96,223,53,112,101,142
	},

	{
		243,182,138,24,233,13,236,
		111,101,139,30,233,72,14,
		184,190,152,139,8,27,43,
		209,11,1,28,32,184,180,
		23,136,221,224,23,172,92,
		54,164,174,55,183,64,226,
		55,110,252,49,136,184,237
	},

	{
		168,31,242,128,40,66,176,
		76,175,24,54,137,56,123,
		117,172,160,14,64,79,145,
		189,229,248,232,177,1,228,
		140,236,147,75,46,114,76,
		98,236,88,11,153,87,150,
		113,38,187,224,182,142,66
	},

	{
		122,108,52,202,69,109,124,
		11,236,192,188,209,141,143,
		107,2,161,242,191,43,149,
		213,80,56,214,152,203,56,
		184,181,95,171,8,24,230,
		90,155,103,126,27,205,246,
		138,151,25,142,214,114,214
	},

	{
		213,206,212,75,128,138,215,
		3,9,165,183,182,42,136,
		95,160,201,65,37,150,152,
		120,86,194,18,154,95,72,
		32,216,51,191,18,14,196,
		210,34,39,38,205,70,33,
		49,149,117,58,69,184,47
	},

	{
		103,250,19,106,176,94,193,
		170,129,145,215,119,136,99,
		150,230,68,189,140,121,205,
		122,113,250,249,128,25,160,
		111,175,78,31,194,145,244,
		241,50,63,74,129,28,126,
		145,114,195,134,27,192,129
	},

	{
		105,226,237,237,149,175,252,
		47,105,12,92,232,88,227,
		150,6,135,43,231,207,108,
		96,125,141,94,67,149,47,
		140,149,47,26,91,70,142,
		191,156,210,71,9,152,66,
		2,50,96,134,186,44,99
	},

	{
		170,175,37,193,148,175,69,
		82,195,14,57,200,212,237,
		173,12,48,17,66,33,172,
		253,26,162,149,146,154,80,
		201,219,119,146,111,85,150,
		130,251,78,244,97,242,176,
		24,248,51,221,36,223,33
	},

	{
		213,62,25,152,108,57,4,
		234,182,62,27,201,109,115,
		108,127,138,80,90,52,185,
		155,125,184,249,199,67,143,
		9,170,32,163,226,66,228,
		119,8,136,33,43,190,251,
		25,44,15,242,48,231,170
	},

	{
		69,91,100,123,125,126,3,
		16,15,53,81,162,184,255,
		71,231,166,75,180,107,111,
		70,76,164,128,12,27,134,
		15,3,189,185,48,3,58,
		65,38,37,200,190,124,195,
		106,2,12,153,166,58,138
	},

	{
		123,65,90,159,168,71,9,
		59,29,75,192,44,243,30,
		96,151,51,235,228,107,25,
		10,122,99,77,82,172,172,
		238,98,218,213,14,151,193,
		17,99,33,53,103,222,249,
		211,243,248,47,243,195,56
	},

	{
		56,101,106,235,205,98,227,
		149,9,73,232,235,35,111,
		57,2,186,181,5,123,226,
		25,5,73,234,254,89,240,
		29,109,202,196,100,234,133,
		21,35,57,225,202,22,171,
		86,96,154,71,43,190,51
	},

	{
		113,4,220,161,78,222,190,
		105,166,150,161,29,113,135,
		244,63,197,107,244,34,109,
		12,130,139,204,212,9,130,
		89,234,19,21,88,222,93,
		119,22,135,8,75,90,48,
		186,108,73,157,75,30,108
	},

	{
		17,212,231,18,216,185,255,
		206,139,136,155,69,129,54,
		249,247,173,184,136,88,86,
		87,234,205,238,209,80,233,
		120,180,224,128,177,228,157,
		229,131,214,148,251,230,103,
		174,80,190,201,4,2,208
	},

	{
		146,254,166,111,69,45,224,
		229,101,23,235,77,130,235,
		201,210,60,113,78,20,166,
		239,198,159,25,140,106,57,
		210,220,85,242,120,14,58,
		186,211,127,152,111,219,20,
		147,21,114,47,54,65,221
	},

	{
		155,85,87,94,68,199,179,
		247,16,43,80,193,182,242,
		2,222,241,243,234,109,211,
		36,48,131,165,204,150,166,
		31,224,17,175,90,178,226,
		156,73,193,106,70,25,207,
		45,110,76,9,57,33,236
	},

	{
		92,120,224,167,215,16,147,
		48,40,91,171,186,7,158,
		131,243,194,20,101,62,158,
		82,46,144,65,190,23,223,
		233,24,132,74,164,166,105,
		255,50,68,41,33,225,183,
		114,136,18,144,18,185,137
	},

	{
		17,38,234,111,64,195,105,
		13,188,100,59,18,103,53,
		94,165,126,168,103,216,69,
		76,238,220,198,89,149,145,
		208,249,2,180,199,181,161,
		111,85,255,154,106,166,101,
		176,38,55,72,130,223,209
	},

	{
		167,241,154,215,233,21,164,
		109,34,121,61,150,197,43,
		152,236,80,57,1,247,197,
		38,56,196,145,26,241,225,
		184,26,125,57,215,61,108,
		5,217,25,106,86,248,82,
		244,93,228,157,51,255,155
	},

	{
		17,171,212,222,22,253,38,
		199,218,104,6,108,146,77,
		24,18,109,109,32,217,136,
		11,245,112,196,192,233,176,
		199,40,69,165,19,35,228,
		253,123,233,162,79,218,165,
		141,47,92,202,150,152,45
	},

	{
		225,196,143,248,9,135,132,
		255,208,73,103,33,197,201,
		226,174,65,101,190,110,210,
		217,216,142,238,169,181,244,
		21,167,20,177,57,211,138,
		7,141,14,153,186,125,207,
		52,40,255,101,49,219,7
	},

	{
		221,161,17,57,52,123,239,
		174,209,155,220,246,5,150,
		183,9,120,65,88,110,87,
		121,50,70,26,37,101,116,
		179,214,27,159,28,19,139,
		66,223,181,127,121,199,102,
		162,222,47,153,130,15,38
	},

	{
		17,153,36,244,88,45,209,
		13,52,102,102,28,128,115,
		92,56,99,16,189,213,111,
		73,11,146,130,213,230,111,
		254,221,49,151,65,127,210,
		67,208,249,241,212,65,196,
		197,202,67,119,183,143,207
	},

	{
		215,34,213,248,221,72,132,
		67,21,208,217,15,31,125,
		206,1,200,69,102,231,110,
		158,231,21,29,36,182,156,
		61,82,201,119,35,142,102,
		8,89,59,54,229,1,5,
		70,123,34,41,23,101,57
	},

	{
		237,73,8,231,245,255,96,
		23,192,156,114,102,247,212,
		103,60,202,216,189,92,25,
		135,142,70,147,87,94,74,
		147,3,108,158,231,159,5,
		78,65,36,186,99,128,2,
		18,25,152,230,144,115,68
	},

	{
		67,160,17,62,161,74,151,
		54,117,56,133,66,108,253,
		205,246,229,26,15,240,160,
		164,184,238,53,49,8,79,
		128,206,34,19,134,160,42,
		46,244,13,175,78,210,242,
		70,225,118,232,164,133,95
	},

	{
		234,31,155,10,134,212,66,
		76,236,106,93,102,226,169,
		245,73,20,225,147,59,178,
		82,227,106,224,136,73,140,
		225,8,166,24,129,254,2,
		60,177,128,86,36,201,254,
		46,168,120,115,83,251,187
	},

	{
		250,68,115,1,74,167,252,
		185,210,119,96,136,228,20,
		226,83,178,12,92,93,137,
		176,99,183,16,82,157,33,
		240,58,214,190,190,59,230,
		162,107,243,19,160,47,144,
		237,98,48,50,92,59,202
	},

	{
		200,215,171,200,126,118,10,
		108,131,226,88,154,98,215,
		172,254,156,39,46,143,200,
		255,29,50,226,118,222,231,
		11,231,124,91,113,98,162,
		28,148,147,126,109,164,97,
		127,240,42,199,123,23,180
	},

	{
		12,31,155,207,220,35,247,
		228,132,83,16,83,176,193,
		3,101,248,53,107,111,184,
		58,242,191,69,225,126,151,
		54,140,231,245,181,216,122,
		88,139,10,38,53,235,254,
		183,99,170,32,231,44,224
	},

	{
		58,165,53,170,203,99,6,
		7,176,212,91,123,166,220,
		253,142,168,30,92,181,147,
		230,151,199,9,241,63,145,
		18,185,118,166,114,193,176,
		248,40,177,209,100,8,139,
		202,181,248,231,144,70,195
	},

	{
		130,125,204,233,239,251,121,
		125,221,146,233,122,172,27,
		30,135,191,89,246,172,84,
		164,101,232,68,230,205,26,
		131,152,212,69,170,4,10,
		157,124,120,171,85,158,175,
		253,165,128,27,100,7,165
	},

	{
		73,196,31,75,40,86,63,
		169,84,115,12,176,43,123,
		51,162,223,112,169,80,73,
		82,110,99,145,226,70,121,
		158,222,108,131,72,70,172,
		32,48,207,219,106,99,30,
		51,249,130,195,120,123,171
	},

	{
		200,109,185,213,166,141,95,
		219,114,120,103,244,57,23,
		111,193,41,233,225,180,176,
		121,66,10,13,97,114,5,
		125,233,83,167,164,179,138,
		170,44,99,209,215,82,191,
		160,149,104,78,138,201,227
	},

	{
		213,42,215,30,187,239,231,
		72,204,179,60,193,67,75,
		188,94,173,32,126,173,132,
		254,121,236,7,2,33,168,
		68,249,31,180,211,23,228,
		135,44,63,68,159,20,222,
		221,240,43,142,123,23,244
	},

	{
		94,48,61,112,51,92,118,
		161,242,95,128,16,237,38,
		159,179,81,76,164,62,254,
		14,169,65,160,197,179,44,
		81,30,183,138,178,196,54,
		141,197,193,234,76,97,64,
		186,179,158,235,115,169,11
	},

	{
		168,114,74,10,218,199,193,
		41,245,250,186,205,131,243,
		215,15,105,59,42,17,150,
		13,40,194,220,108,22,72,
		37,197,83,249,187,57,148,
		64,234,33,109,51,156,254,
		53,72,55,68,174,240,117
	},

	{
		55,124,124,107,105,194,22,
		114,242,8,53,46,244,181,
		96,111,133,70,140,37,165,
		131,1,12,65,220,11,103,
		233,72,92,101,91,163,116,
		125,83,15,224,107,242,66,
		178,12,37,63,4,213,48
	},

	{
		115,221,24,12,172,92,193,
		137,224,105,238,239,70,221,
		224,35,164,110,174,229,104,
		50,86,88,100,25,108,68,
		195,55,41,87,21,47,249,
		212,245,152,96,232,85,190,
		77,39,177,136,180,82,111
	},

	{
		231,138,6,145,159,235,88,
		248,198,164,9,130,209,202,
		96,14,128,170,71,217,142,
		180,183,53,110,173,41,252,
		249,214,242,217,100,66,95,
		181,71,11,111,109,35,175,
		240,32,51,69,251,41,220
	},

	{
		177,114,53,161,177,151,101,
		226,222,87,255,85,102,86,
		161,114,178,99,106,206,188,
		98,29,157,53,65,169,67,
		74,31,24,83,60,48,80,
		186,116,113,179,144,90,220,
		252,71,167,25,110,167,94
	},

	{
		81,168,102,49,61,70,172,
		127,7,148,115,179,10,117,
		253,35,30,218,76,63,116,
		210,241,65,169,157,4,9,
		29,205,160,111,41,213,193,
		190,86,19,207,133,25,190,
		187,131,66,196,253,227,163
	},

	{
		236,81,128,26,96,137,92,
		120,243,71,33,141,55,2,
		185,185,187,33,230,72,146,
		143,226,21,110,155,226,150,
		111,124,165,242,4,76,201,
		34,223,227,29,202,119,86,
		172,219,8,121,183,148,142
	},

	{
		73,81,249,189,120,220,96,
		79,28,37,30,199,170,56,
		1,10,70,215,151,190,146,
		156,233,11,251,146,195,134,
		188,140,23,233,137,204,150,
		116,36,97,40,119,204,194,
		55,9,193,18,143,76,209
	},

	{
		143,200,80,50,154,231,247,
		140,225,72,226,136,2,99,
		209,182,82,146,128,49,3,
		98,101,196,95,136,85,3,
		81,37,131,9,251,79,209,
		101,128,157,238,229,205,178,
		49,161,141,101,175,48,2
	},

	{
		171,145,70,2,204,252,166,
		194,2,182,182,189,17,205,
		141,80,12,104,215,96,174,
		11,106,181,150,113,213,71,
		209,89,219,35,229,63,18,
		64,237,140,153,224,215,31,
		48,198,108,180,139,233,39
	},

	{
		31,60,240,131,203,6,32,
		206,137,249,244,36,8,39,
		38,142,11,77,76,18,200,
		76,210,138,106,198,130,22,
		37,178,76,18,233,77,149,
		77,28,47,186,126,72,234,
		254,158,97,10,162,58,93
	},

	{
		181,91,80,208,248,218,224,
		90,219,176,181,120,63,35,
		132,167,206,85,6,76,223,
		90,15,100,142,121,79,111,
		255,88,196,134,19,72,133,
		40,95,98,232,153,217,182,
		186,109,241,66,128,26,140
	},

	{
		225,112,22,157,253,52,123,
		209,31,7,165,18,48,83,
		190,53,154,87,188,199,251,
		248,185,48,164,93,44,22,
		165,77,239,88,148,21,155,
		115,203,231,20,170,83,100,
		222,204,57,83,39,241,126
	},

	{
		51,228,1,96,176,54,89,
		49,157,66,125,113,124,187,
		15,110,70,101,246,16,108,
		156,20,32,200,96,70,170,
		120,198,42,199,79,93,100,
		115,246,113,213,187,66,169,
		129,241,230,251,145,23,29
	},

	{
		101,67,67,78,108,22,58,
		97,212,30,230,190,142,195,
		243,169,75,210,61,248,180,
		47,75,218,191,5,154,172,
		19,204,224,123,244,251,1,
		94,192,187,249,33,79,120,
		115,109,88,173,40,7,61
	},

	{
		178,239,46,52,245,141,192,
		77,114,7,54,78,179,195,
		12,213,79,16,184,192,27,
		100,195,74,5,1,15,226,
		223,50,245,39,244,230,85,
		8,196,217,18,128,187,251,
		52,225,65,96,62,49,239
	},

	{
		94,190,122,81,36,204,171,
		249,104,120,214,89,133,203,
		103,19,163,117,53,59,48,
		182,240,91,101,160,234,121,
		1,78,151,26,3,28,228,
		130,138,137,188,21,167,198,
		197,34,63,165,195,11,206
	},

	{
		220,164,4,55,94,159,83,
		86,186,109,101,150,60,4,
		146,58,195,221,221,148,11,
		50,155,232,95,39,71,37,
		106,104,102,59,34,41,206,
		61,129,242,45,11,12,85,
		27,184,210,214,165,106,238
	},

	{
		44,151,227,47,39,93,122,
		237,91,225,164,198,235,53,
		226,36,124,86,57,94,185,
		251,231,101,120,178,168,162,
		218,38,34,33,126,235,196,
		151,30,175,212,103,170,17,
		208,48,75,16,231,10,66
	},

	{
		90,201,253,44,89,120,222,
		145,82,80,6,63,114,22,
		37,106,33,40,177,4,112,
		152,9,70,250,103,229,235,
		22,147,65,5,164,55,48,
		233,103,112,216,69,34,63,
		31,159,108,38,14,111,132
	},

	{
		170,177,43,7,140,86,71,
		207,205,235,155,239,88,129,
		53,6,111,139,249,208,172,
		252,34,88,175,213,251,129,
		86,54,242,107,142,144,79,
		96,161,29,171,196,16,17,
		143,205,207,201,47,88,10
	},

	{
		112,203,199,48,59,153,46,
		130,221,189,166,213,63,67,
		50,190,21,148,242,238,113,
		94,28,228,234,100,103,187,
		95,53,124,69,6,55,6,
		39,204,11,231,64,176,183,
		86,248,21,152,62,205,200
	},

	{
		133,189,180,176,46,27,167,
		213,33,253,7,209,112,92,
		8,60,122,206,229,50,184,
		207,25,142,34,149,64,234,
		16,252,247,255,22,70,90,
		126,212,16,93,248,50,89,
		194,248,244,230,224,203,62
	},

	{
		234,231,52,206,57,230,130,
		158,240,79,58,175,226,31,
		213,160,154,113,199,241,84,
		178,66,108,201,41,240,188,
		24,9,50,145,172,208,226,
		100,127,42,93,55,123,245,
		197,102,110,206,48,234,81
	},

	{
		176,193,13,70,253,101,15,
		39,156,34,168,239,92,185,
		235,239,109,151,115,6,12,
		184,92,210,40,152,12,36,
		57,183,30,136,247,232,152,
		77,59,225,190,208,165,59,
		133,118,100,111,45,20,177
	},

	{
		242,224,56,94,247,232,26,
		5,5,92,145,57,226,205,
		229,79,2,235,214,16,252,
		135,176,247,51,4,190,78,
		45,155,6,184,220,121,136,
		151,66,29,253,210,166,255,
		75,82,234,246,152,230,26
	},

	{
		71,92,199,196,224,54,108,
		71,191,224,240,145,231,54,
		198,135,241,115,219,139,162,
		178,253,95,131,237,175,212,
		251,85,23,132,211,92,89,
		14,166,88,5,254,58,254,
		204,5,41,160,72,105,230
	},

	{
		122,240,140,138,243,30,90,
		95,32,36,108,168,91,94,
		223,161,93,96,118,32,153,
		235,41,222,65,8,181,221,
		196,207,70,218,41,74,198,
		79,6,240,9,240,142,41,
		69,48,113,102,15,168,239
	},

	{
		33,100,248,1,115,220,254,
		216,113,156,238,175,27,36,
		61,20,255,131,209,181,91,
		40,126,238,227,186,46,228,
		253,64,201,169,138,5,71,
		30,20,119,212,255,127,121,
		170,240,13,84,96,212,210
	},

	{
		213,91,87,107,184,54,252,
		30,240,165,139,29,230,205,
		42,49,78,136,10,113,160,
		132,246,82,211,200,164,178,
		112,128,15,43,150,157,197,
		88,7,48,126,75,210,2,
		196,228,191,100,25,80,254
	},

	{
		161,100,85,71,23,7,176,
		250,104,108,62,88,195,110,
		27,102,177,102,127,163,105,
		65,133,21,24,117,155,208,
		39,37,206,154,225,85,176,
		161,128,129,8,6,18,114,
		12,53,53,165,90,177,26
	},

	{
		36,101,106,199,26,144,50,
		33,13,177,101,149,6,56,
		112,251,141,157,46,152,191,
		112,214,180,104,160,119,202,
		112,75,167,190,116,185,126,
		27,229,214,110,194,128,191,
		19,24,123,193,148,15,69
	},

	{
		94,179,36,34,168,18,167,
		136,168,214,94,144,245,66,
		241,83,82,74,25,138,106,
		213,7,78,133,181,254,75,
		73,230,160,60,93,65,204,
		19,24,42,169,150,246,86,
		182,211,11,68,108,69,44
	},

	{
		127,97,243,9,60,97,125,
		234,114,130,69,83,11,157,
		186,44,46,37,4,205,85,
		226,218,134,60,127,11,227,
		55,80,85,109,26,120,25,
		19,60,173,155,255,182,196,
		64,12,112,91,145,131,23
	},

	{
		166,108,127,147,227,40,125,
		27,76,18,129,108,39,24,
		100,92,205,245,147,59,47,
		190,251,54,204,28,72,76,
		151,24,71,228,181,206,140,
		172,161,216,38,201,154,203,
		141,252,226,169,135,208,183
	},

	{
		143,63,98,145,133,62,237,
		139,191,134,154,119,47,128,
		196,154,179,252,48,125,206,
		245,106,207,37,87,203,218,
		175,115,73,116,243,111,167,
		231,22,81,34,216,26,234,
		36,136,62,97,46,137,172
	},

	{
		125,118,61,116,135,101,54,
		15,215,129,210,84,238,226,
		43,56,40,197,174,30,226,
		95,117,34,65,115,192,53,
		67,222,155,83,103,157,18,
		188,158,107,3,133,177,144,
		242,76,231,253,233,120,19
	},

	{
		65,199,211,202,123,9,216,
		255,223,125,141,168,222,211,
		71,184,143,146,32,40,117,
		82,140,45,50,203,212,115,
		70,166,219,132,100,127,245,
		11,146,38,66,68,135,130,
		71,59,123,29,51,25,249
	},

	{
		190,161,56,194,42,46,58,
		104,89,177,209,173,64,141,
		117,68,136,202,201,9,67,
		112,209,78,255,233,204,77,
		72,199,213,85,252,204,10,
		221,117,125,115,252,250,46,
		101,111,125,194,139,186,255
	},

	{
		31,86,139,193,121,21,227,
		136,133,87,49,18,190,230,
		224,29,177,1,149,9,151,
		24,129,123,31,191,15,12,
		186,16,109,103,75,43,16,
		233,124,37,188,28,178,253,
		78,27,215,234,165,87,234
	},

	{
		170,4,161,157,18,78,11,
		223,148,86,130,184,244,199,
		171,43,229,209,235,160,189,
		23,30,145,9,19,238,221,
		41,113,74,156,184,137,226,
		186,241,135,67,86,193,87,
		95,11,215,251,31,39,80
	},

	{
		85,223,21,67,248,93,201,
		234,55,255,161,143,221,252,
		110,10,206,90,36,240,21,
		39,237,78,109,10,6,248,
		7,125,44,88,104,198,17,
		195,47,15,140,61,184,167,
		251,182,72,224,147,159,196
	},

	{
		123,19,239,168,30,154,104,
		60,207,7,69,172,234,176,
		23,31,137,211,242,79,208,
		145,194,222,40,205,220,218,
		202,197,167,163,162,203,119,
		188,250,215,100,172,230,115,
		79,119,241,177,204,213,247
	},

	{
		164,47,183,166,25,101,55,
		173,255,232,132,251,73,117,
		235,22,152,184,146,245,171,
		192,242,110,184,81,188,135,
		183,224,50,201,210,124,141,
		254,235,54,153,73,133,105,
		251,27,228,131,103,255,205
	},

	{
		141,53,177,160,37,136,109,
		76,46,150,5,207,226,32,
		160,180,136,191,175,88,111,
		36,245,86,149,192,56,237,
		119,37,117,196,117,115,54,
		203,93,49,208,170,113,233,
		46,143,140,211,5,170,166
	},

	{
		2,245,56,116,61,150,114,
		76,28,201,61,29,126,115,
		226,153,233,246,200,95,246,
		112,88,77,97,48,22,186,
		203,102,27,213,211,119,235,
		204,147,244,229,21,29,175,
		52,103,21,222,146,203,247
	},

	{
		203,187,75,44,10,228,157,
		113,156,9,120,202,29,117,
		184,122,64,90,44,31,194,
		191,30,155,50,190,115,208,
		130,69,132,201,246,81,100,
		108,67,226,228,85,112,150,
		243,222,107,49,158,2,199
	},

	{
		243,107,29,155,25,62,54,
		254,77,132,226,20,121,227,
		78,33,189,27,153,172,240,
		78,17,13,117,133,92,119,
		205,123,66,233,177,141,20,
		2,21,244,85,47,218,7,
		194,47,175,180,65,92,188
	},

	{
		3,223,186,179,171,112,64,
		226,172,223,83,208,235,215,
		235,20,187,212,96,218,104,
		41,66,233,141,50,89,115,
		102,35,34,147,219,175,203,
		216,121,7,60,166,18,77,
		246,202,53,41,124,33,72
	},

	{
		115,8,155,105,168,30,132,
		213,57,253,89,18,62,83,
		13,138,197,40,21,218,47,
		156,13,133,209,66,82,67,
		99,176,107,172,80,52,146,
		108,148,19,61,193,41,114,
		250,121,164,57,125,29,35
	},

	{
		252,37,152,15,100,236,254,
		226,253,7,172,123,6,10,
		139,17,7,254,146,114,216,
		45,95,154,241,252,4,47,
		134,193,201,19,97,207,39,
		144,254,165,157,60,218,144,
		173,48,97,157,173,165,139
	},

	{
		43,160,148,141,246,225,109,
		162,9,196,127,84,62,238,
		41,82,224,161,103,65,28,
		169,227,113,157,43,7,17,
		78,6,67,179,102,7,16,
		186,197,164,203,143,211,114,
		255,34,222,17,63,184,47
	},

	{
		202,17,101,190,148,37,65,
		227,178,78,200,202,174,105,
		90,13,220,198,78,74,44,
		37,172,23,111,225,219,23,
		223,119,96,149,12,206,14,
		131,243,148,179,37,120,26,
		168,77,232,223,65,125,224
	},

	{
		95,160,190,37,50,215,151,
		196,31,209,227,48,62,1,
		249,123,173,73,232,137,16,
		25,39,88,73,51,149,31,
		178,19,34,120,74,112,230,
		97,243,213,31,34,121,189,
		200,255,68,57,75,208,129
	},

	{
		135,244,60,186,169,184,217,
		128,88,234,225,59,98,108,
		180,189,228,11,222,179,81,
		129,225,122,192,210,117,133,
		142,124,112,228,166,55,29,
		3,133,178,40,47,107,31,
		237,22,170,212,75,186,142
	},

	{
		254,191,1,222,153,4,115,
		100,149,222,68,112,139,102,
		61,231,195,98,156,181,242,
		24,137,44,136,34,43,76,
		219,155,8,100,28,81,181,
		252,75,50,126,154,46,177,
		11,53,126,99,180,248,57
	},

	{
		109,132,42,96,40,141,189,
		8,138,168,147,13,172,54,
		196,69,7,241,14,128,187,
		124,187,63,140,172,27,97,
		195,63,235,191,196,254,187,
		234,123,1,174,6,88,6,
		177,247,45,156,120,226,125
	},

	{
		30,95,46,155,25,137,245,
		52,68,72,74,137,152,60,
		3,26,152,42,137,223,113,
		244,114,229,1,192,53,115,
		90,174,232,119,234,38,77,
		114,195,46,6,151,52,203,
		85,96,69,205,58,68,39
	},

	{
		30,59,201,53,236,197,180,
		160,196,103,128,88,254,209,
		225,82,124,227,135,183,59,
		172,203,39,154,168,155,144,
		235,35,68,198,41,8,87,
		105,92,255,110,51,167,55,
		248,69,103,157,208,146,105
	},

	{
		249,205,245,168,172,15,16,
		148,112,53,141,74,5,64,
		218,194,253,75,243,113,168,
		228,80,59,85,44,153,81,
		216,126,55,215,165,220,72,
		217,83,199,41,128,227,208,
		24,3,173,137,18,75,53
	},

	{
		21,130,191,209,19,213,58,
		123,18,147,139,147,81,14,
		156,255,245,64,150,115,131,
		244,12,61,185,211,155,114,
		151,96,11,203,200,138,71,
		17,26,234,71,106,73,60,
		76,106,210,50,115,45,2
	},

	{
		55,17,107,248,62,204,196,
		224,45,253,50,10,2,51,
		233,34,73,107,137,68,253,
		235,235,24,99,52,125,184,
		3,3,88,146,80,226,29,
		5,25,66,196,226,87,125,
		83,169,124,218,190,135,197
	},

	{
		250,224,3,197,156,209,110,
		159,75,119,221,42,212,180,
		223,115,13,246,173,221,211,
		148,3,78,214,195,102,155,
		5,128,5,204,42,72,240,
		65,177,242,226,81,255,139,
		70,150,95,124,203,83,248
	},

	{
		234,33,49,205,144,43,212,
		44,249,86,116,150,112,80,
		244,150,120,207,182,110,50,
		179,160,41,114,31,130,253,
		243,221,106,120,118,181,252,
		103,30,238,119,10,242,187,
		99,99,210,197,153,71,176
	},

	{
		137,180,252,121,200,124,8,
		111,186,110,245,102,71,247,
		195,157,232,115,191,169,136,
		138,98,54,253,14,34,248,
		106,226,167,185,48,19,112,
		183,175,155,66,76,150,34,
		114,38,225,8,126,236,96
	},

	{
		16,220,169,218,40,146,208,
		171,96,114,57,235,189,141,
		227,252,238,194,231,160,128,
		231,91,85,175,137,143,46,
		106,176,119,234,149,173,154,
		114,52,93,90,126,142,222,
		243,157,223,56,7,82,175
	},

	{
		214,11,122,112,113,118,195,
		111,55,4,85,186,203,217,
		125,14,7,2,128,91,236,
		239,85,23,213,142,125,198,
		74,130,186,4,118,41,195,
		123,188,1,212,1,94,239,
		40,89,169,57,55,203,169
	},

	{
		107,224,17,213,57,56,188,
		177,120,49,183,211,64,230,
		226,84,84,171,122,18,226,
		165,77,205,198,31,112,139,
		65,93,107,58,110,22,144,
		19,97,87,140,177,42,4,
		192,72,174,177,138,11,166
	},

	{
		9,11,33,144,120,134,56,
		4,91,241,26,37,93,204,
		71,167,75,221,9,34,10,
		219,253,35,235,183,134,199,
		28,130,28,63,91,151,1,
		3,187,36,59,41,166,150,
		104,162,205,29,72,186,83
	},

	{
		196,130,182,208,71,167,231,
		133,59,210,10,82,151,243,
		72,199,77,48,187,1,229,
		90,194,112,224,238,252,108,
		206,154,86,168,215,178,231,
		84,179,166,10,22,174,219,
		127,163,226,226,243,31,38
	},

	{
		151,198,6,214,2,99,210,
		26,142,255,31,154,133,47,
		111,116,53,2,99,76,220,
		32,138,65,181,182,139,192,
		37,121,215,223,133,181,173,
		70,135,166,142,83,146,1,
		243,133,54,3,113,189,13
	},

	{
		218,245,135,17,201,119,232,
		222,199,170,217,219,28,16,
		3,178,162,3,204,38,47,
		12,104,170,218,33,215,196,
		149,34,158,166,210,45,34,
		243,172,26,99,60,84,204,
		28,18,35,143,222,62,46
	},

	{
		60,75,3,168,92,248,239,
		242,4,39,47,181,156,203,
		212,206,79,31,30,121,87,
		53,27,131,225,189,185,224,
		197,139,173,133,179,233,43,
		197,57,111,229,53,35,75,
		91,56,162,191,210,60,204
	},

	{
		204,16,230,187,172,49,5,
		6,62,173,174,199,231,242,
		88,238,27,145,67,3,252,
		116,22,44,104,24,248,161,
		191,68,19,63,190,51,179,
		124,223,155,19,121,99,175,
		236,86,157,100,225,151,149
	},

	{
		20,225,193,156,236,144,244,
		233,27,222,169,213,53,207,
		99,209,213,167,118,171,224,
		107,166,60,107,5,215,26,
		193,227,130,90,118,110,40,
		15,9,41,122,128,4,213,
		119,214,25,121,36,43,50
	},

	{
		145,47,181,236,88,31,32,
		115,104,90,150,49,168,172,
		179,101,188,142,221,234,236,
		228,41,88,211,109,94,201,
		158,144,56,104,73,210,109,
		23,168,157,173,64,144,150,
		18,68,3,56,48,116,165
	},

	{
		244,90,27,112,128,36,134,
		214,150,207,139,84,223,171,
		128,173,54,7,27,180,4,
		201,54,253,233,84,240,76,
		115,170,33,14,5,159,140,
		205,195,253,229,225,165,86,
		11,58,114,131,107,165,215
	},

	{
		127,30,93,91,165,158,58,
		91,236,151,103,207,65,207,
		224,16,142,150,170,76,137,
		179,3,245,230,90,117,207,
		4,1,32,217,158,175,10,
		214,182,171,214,154,51,253,
		189,234,95,204,17,14,207
	},

	{
		251,51,223,223,24,80,138,
		60,244,179,168,186,1,21,
		12,239,194,171,206,186,121,
		108,254,72,86,66,135,179,
		75,154,160,214,228,28,109,
		100,31,230,13,217,190,45,
		212,123,22,131,225,202,182
	},

	{
		185,198,186,9,155,133,18,
		53,111,146,55,105,127,17,
		220,228,159,10,193,193,233,
		209,13,3,157,84,98,206,
		113,120,76,80,52,103,3,
		69,15,214,66,155,70,31,
		44,43,203,79,226,242,132
	},

	{
		243,234,219,137,211,230,117,
		77,78,213,164,239,148,89,
		188,164,131,43,255,119,66,
		78,239,81,106,25,124,145,
		243,179,114,20,144,27,54,
		248,181,69,49,9,19,129,
		246,21,163,160,145,26,21
	},

	{
		19,244,140,188,119,3,162,
		214,207,50,237,66,223,44,
		37,110,211,126,117,193,202,
		185,39,26,89,15,255,186,
		152,204,45,61,223,196,18,
		230,196,12,213,241,104,9,
		2,33,192,82,18,67,223
	},

	{
		74,68,234,227,249,134,5,
		155,29,216,149,124,210,253,
		70,1,251,206,7,6,169,
		11,110,69,164,249,34,121,
		124,192,237,83,24,179,204,
		195,70,140,154,203,57,204,
		154,84,113,52,162,44,11
	},

	{
		149,12,210,227,237,40,13,
		145,9,125,242,172,155,114,
		134,79,24,170,101,90,40,
		201,183,100,21,213,235,222,
		1,235,97,78,63,140,139,
		41,175,36,176,69,106,21,
		222,78,151,1,31,62,206
	},

	{
		111,142,87,207,172,114,135,
		240,251,218,183,28,227,230,
		7,172,200,86,82,11,141,
		106,27,97,114,183,48,49,
		236,5,27,61,172,200,203,
		128,129,90,113,165,107,124,
		2,196,116,74,95,198,166
	},

	{
		36,157,67,183,185,88,56,
		196,189,140,108,182,108,4,
		207,158,104,168,192,176,19,
		219,132,39,248,42,196,176,
		100,106,126,180,172,179,32,
		32,102,40,67,229,250,6,
		212,3,207,255,251,39,137
	},

	{
		75,159,202,137,103,226,221,
		61,6,107,208,82,34,206,
		43,111,163,245,105,131,160,
		221,86,66,164,127,159,241,
		252,63,209,15,117,177,134,
		241,155,33,226,253,211,145,
		55,122,105,182,231,179,227
	},

	{
		157,96,103,188,105,64,44,
		218,9,130,220,208,31,209,
		165,84,23,196,202,232,165,
		52,185,56,150,110,141,11,
		65,114,137,84,121,247,180,
		97,83,114,27,129,147,201,
		227,59,40,2,192,121,117
	},

	{
		141,213,168,224,119,181,65,
		98,40,127,183,126,248,200,
		61,116,77,83,91,13,104,
		56,217,205,187,161,226,238,
		229,156,224,248,17,35,26,
		72,247,255,100,102,62,145,
		12,135,83,17,77,255,163
	},

	{
		114,95,19,65,117,142,233,
		198,248,84,19,166,59,238,
		91,165,4,102,92,171,109,
		125,153,177,72,137,125,255,
		201,156,23,103,141,9,230,
		198,139,174,164,127,20,8,
		55,25,105,110,215,204,24
	},

	{
		158,164,46,157,212,125,174,
		116,154,138,38,34,169,58,
		43,99,220,22,105,253,182,
		66,163,101,91,9,182,186,
		147,53,45,66,185,174,198,
		244,21,25,133,42,145,223,
		147,19,91,117,172,252,72
	},

	{
		49,66,21,133,143,27,168,
		148,62,162,138,247,194,151,
		175,153,19,96,160,84,252,
		176,202,168,181,193,91,4,
		91,206,171,158,213,18,227,
		101,224,241,223,225,148,168,
		252,160,86,4,213,6,111
	},

	{
		144,151,17,65,208,251,3,
		77,204,130,87,4,157,7,
		28,165,66,66,8,17,95,
		85,91,208,59,252,247,77,
		146,111,174,109,148,149,48,
		134,177,171,170,239,125,216,
		120,18,77,240,230,76,226
	},

	{
		210,134,132,192,156,253,190,
		117,63,210,141,138,131,45,
		185,81,35,254,244,69,17,
		145,239,66,118,235,177,58,
		145,10,125,173,254,99,41,
		155,144,176,54,26,63,107,
		135,92,92,2,13,83,139
	},

	{
		51,60,163,170,147,164,49,
		58,161,146,230,89,121,242,
		4,248,134,113,158,82,65,
		18,148,65,101,47,159,144,
		148,39,206,229,233,148,16,
		64,113,112,11,203,242,240,
		255,1,19,113,237,186,66
	},

	{
		89,159,78,103,56,246,78,
		204,4,21,252,53,204,162,
		14,168,189,244,222,214,188,
		53,154,156,141,90,137,154,
		195,28,5,79,102,155,54,
		192,149,251,61,20,11,162,
		196,30,206,82,172,93,1
	},

	{
		226,222,85,249,190,223,200,
		178,240,60,187,187,232,97,
		207,164,185,5,211,32,8,
		168,23,210,90,85,110,5,
		12,44,92,46,148,220,104,
		161,95,153,5,51,231,168,
		13,54,84,34,77,166,72
	},

	{
		252,15,213,37,242,26,114,
		115,99,46,77,163,196,100,
		157,235,193,113,53,117,144,
		72,105,138,167,8,22,7,
		97,184,138,186,169,200,185,
		7,73,199,135,77,234,79,
		143,149,114,153,47,242,186
	},

	{
		187,60,9,83,243,54,78,
		90,20,70,81,255,107,243,
		177,221,63,217,7,159,51,
		56,113,50,168,185,8,252,
		138,74,218,63,120,74,198,
		59,206,5,205,40,123,185,
		46,167,40,14,241,178,153
	},

	{
		75,41,175,215,50,141,196,
		250,196,198,238,44,224,253,
		14,195,247,8,102,7,200,
		205,196,115,107,61,202,22,
		142,105,139,229,44,24,255,
		154,171,123,119,239,174,72,
		160,219,106,222,45,158,228
	},

	{
		201,188,54,248,57,37,25,
		96,199,162,200,176,46,20,
		27,160,39,217,196,100,58,
		103,23,127,168,47,95,229,
		39,234,244,187,179,238,89,
		154,37,140,111,160,190,49,
		56,56,126,62,22,213,80
	},

	{
		81,12,160,241,248,231,70,
		171,127,226,220,168,223,151,
		45,22,115,217,54,204,131,
		100,66,186,63,198,114,191,
		69,158,2,56,67,137,48,
		242,216,196,25,192,64,253,
		95,93,232,65,242,229,139
	},

	{
		134,221,148,217,202,95,252,
		95,61,51,127,170,99,97,
		40,82,194,103,179,250,244,
		25,250,229,172,5,102,45,
		149,205,194,61,150,45,7,
		167,96,27,110,234,204,213,
		117,58,248,57,20,234,161
	},

	{
		38,213,157,169,107,23,175,
		84,238,15,28,30,134,243,
		88,168,69,218,79,201,159,
		159,4,16,64,125,5,223,
		214,149,64,121,210,33,68,
		249,64,123,162,195,195,200,
		107,77,238,103,118,198,207
	},

	{
		232,120,145,34,201,147,8,
		220,158,104,126,144,240,77,
		8,89,132,187,230,206,52,
		139,46,181,45,26,125,223,
		181,244,93,1,55,20,46,
		220,205,75,29,161,7,5,
		34,193,17,215,109,50,25
	},

	{
		25,89,86,245,1,51,123,
		253,111,240,58,28,252,69,
		144,241,90,250,19,53,165,
		34,9,11,197,1,207,136,
		105,56,90,29,184,34,29,
		30,96,214,85,38,248,211,
		231,131,125,190,194,106,204
	},

	{
		61,15,48,227,80,24,43,
		221,58,41,146,86,89,88,
		250,64,248,115,177,207,134,
		12,182,142,54,217,120,46,
		111,96,32,51,32,37,151,
		15,72,90,11,200,212,66,
		17,187,46,58,64,154,125
	},

	{
		176,94,60,25,239,233,78,
		19,10,51,143,104,187,179,
		159,185,176,236,250,20,228,
		122,71,189,152,144,122,121,
		149,165,253,58,50,118,92,
		202,216,34,158,78,119,147,
		232,32,175,242,105,5,20
	},

	{
		88,62,37,83,109,101,204,
		176,66,65,101,138,12,229,
		157,97,249,172,65,38,232,
		47,177,45,30,73,118,158,
		209,49,230,186,172,61,84,
		202,3,116,192,24,3,129,
		135,189,122,24,1,172,139
	},

	{
		115,137,193,238,244,237,60,
		4,136,178,113,108,224,44,
		23,96,32,227,245,129,17,
		62,100,83,120,217,93,33,
		161,164,138,122,190,26,26,
		17,48,159,188,27,71,132,
		155,5,167,136,166,149,216
	},

	{
		124,10,86,29,212,50,96,
		40,191,32,87,212,177,122,
		184,100,207,41,78,103,73,
		208,226,235,2,23,9,255,
		153,233,21,34,48,194,23,
		194,249,39,252,94,6,68,
		157,81,56,5,229,1,239
	},

	{
		109,209,104,83,161,130,167,
		172,101,12,168,226,109,80,
		124,120,101,130,117,14,239,
		162,172,222,143,156,249,47,
		182,69,250,40,239,237,75,
		18,96,198,112,106,145,201,
		171,208,196,95,49,54,187
	},

	{
		125,238,86,66,116,112,229,
		80,35,251,120,41,196,128,
		141,64,28,109,190,69,41,
		7,139,44,39,89,183,137,
		4,83,178,29,23,51,255,
		218,62,204,31,93,41,202,
		220,250,247,133,158,120,253
	},

	{
		117,124,147,199,242,198,81,
		46,74,212,97,166,187,160,
		98,132,139,36,127,115,172,
		244,19,206,38,12,210,29,
		201,63,54,94,83,86,145,
		105,132,61,162,21,95,76,
		244,88,13,24,242,35,139
	},

	{
		191,252,45,196,59,89,93,
		15,158,95,25,209,189,162,
		46,60,61,146,124,209,115,
		74,54,193,42,248,209,175,
		155,184,122,14,184,40,48,
		143,46,158,66,212,21,89,
		120,234,207,110,136,175,12
	},

	{
		239,25,187,91,249,22,224,
		99,40,115,213,19,41,56,
		53,221,222,229,82,112,215,
		23,12,215,126,112,44,146,
		209,173,116,133,9,253,233,
		75,235,96,117,211,69,72,
		120,209,63,49,107,230,5
	},

	{
		98,24,14,131,155,143,55,
		150,221,114,139,140,10,153,
		84,73,144,203,3,226,232,
		129,64,28,254,91,143,128,
		99,100,112,138,96,179,122,
		168,183,133,108,113,69,98,
		167,230,45,116,11,32,225
	},

	{
		153,169,41,171,77,85,127,
		241,6,111,247,245,26,2,
		97,66,194,143,211,123,90,
		150,228,211,108,60,176,209,
		165,35,7,167,82,207,143,
		205,104,166,75,33,202,249,
		58,54,206,10,136,19,166
	},

	{
		146,29,30,194,190,208,94,
		195,8,67,217,18,255,127,
		64,188,106,114,153,172,177,
		98,54,195,32,99,153,14,
		221,5,133,140,46,33,255,
		187,212,29,98,102,47,125,
		80,232,235,19,180,106,219
	},

	{
		177,131,58,218,175,130,210,
		59,25,190,170,189,227,245,
		104,118,7,168,136,178,184,
		4,27,64,199,101,160,24,
		83,177,178,232,185,40,122,
		109,38,253,160,14,133,106,
		190,206,58,102,244,229,124
	},

	{
		201,183,24,153,17,111,47,
		19,116,248,160,215,143,180,
		195,122,74,29,158,193,73,
		159,193,93,140,172,31,38,
		129,181,96,183,56,29,172,
		191,252,183,91,214,254,247,
		135,66,76,87,1,112,214
	},

	{
		28,138,132,173,9,230,220,
		57,32,166,202,153,246,49,
		119,246,212,1,50,230,19,
		135,20,173,235,182,61,143,
		76,162,155,224,182,54,123,
		29,223,119,200,175,10,66,
		87,171,204,12,1,44,68
	},

	{
		142,1,145,102,227,81,84,
		108,1,120,168,9,14,135,
		165,118,141,183,47,235,212,
		109,85,167,185,255,89,18,
		12,36,41,24,181,66,107,
		96,5,160,235,87,33,173,
		140,207,20,70,135,44,102
	},

	{
		85,38,191,199,157,183,122,
		186,2,236,106,181,53,100,
		69,189,22,137,117,230,153,
		150,211,184,157,220,165,234,
		113,19,238,122,120,233,181,
		130,182,140,76,219,162,90,
		66,242,19,125,8,215,162
	},

	{
		167,227,240,92,158,80,82,
		18,222,92,51,44,252,34,
		112,74,12,237,158,156,179,
		48,154,47,61,204,24,36,
		62,228,29,23,43,176,239,
		144,117,131,97,219,129,138,
		243,31,33,108,211,255,196
	},

	{
		83,29,187,49,130,95,252,
		214,6,223,87,149,168,140,
		172,33,170,24,103,180,107,
		63,212,92,143,155,126,129,
		190,25,211,244,151,103,7,
		51,227,119,104,18,177,110,
		33,155,104,15,159,62,103
	},

	{
		9,151,31,80,46,220,174,
		147,4,3,76,83,157,9,
		39,52,79,252,16,63,25,
		215,131,90,79,128,143,128,
		117,57,224,68,198,98,158,
		191,90,222,94,135,106,238,
		212,168,164,233,138,106,14
	},

	{
		159,70,200,99,183,101,85,
		117,251,218,149,119,199,12,
		102,190,144,68,145,212,202,
		250,32,118,13,149,136,91,
		203,247,68,69,124,122,248,
		92,125,154,189,60,160,12,
		148,182,162,108,187,198,113
	},

	{
		75,226,151,153,186,223,56,
		174,53,4,212,8,241,147,
		253,166,123,148,241,166,172,
		118,103,6,219,178,187,41,
		89,250,28,220,20,27,134,
		77,125,14,40,175,125,114,
		15,2,160,240,169,186,63
	},

	{
		165,251,182,208,45,34,74,
		41,106,244,193,205,160,100,
		51,41,232,106,237,201,184,
		35,112,98,118,238,38,211,
		191,33,221,226,126,55,61,
		224,82,255,19,252,131,188,
		220,111,242,172,93,61,67
	},

	{
		246,142,205,184,114,219,1,
		246,63,229,6,41,167,187,
		173,185,75,205,96,37,252,
		142,135,8,82,240,138,254,
		107,202,42,104,198,71,98,
		173,197,100,156,37,180,190,
		88,30,229,135,12,58,198
	},

	{
		7,121,166,123,114,32,78,
		223,125,21,155,124,116,240,
		222,91,57,5,254,51,4,
		161,122,236,49,146,88,235,
		253,149,42,31,140,254,163,
		71,253,70,242,115,10,171,
		101,38,217,187,117,31,141
	},

	{
		220,220,58,61,163,182,106,
		201,38,91,53,99,7,253,
		179,12,108,175,148,246,162,
		217,7,36,146,193,22,3,
		220,68,101,117,184,62,195,
		25,94,226,155,28,43,110,
		161,132,70,110,201,58,228
	},

	{
		39,253,170,4,47,122,100,
		182,223,98,128,205,167,103,
		42,16,227,30,43,181,80,
		212,194,100,164,123,181,97,
		126,145,213,51,44,135,240,
		100,105,151,106,174,180,134,
		106,49,72,73,237,2,84
	},

	{
		1,140,181,150,80,96,57,
		214,115,209,143,122,31,162,
		201,171,155,38,225,68,12,
		219,180,253,105,97,208,19,
		20,8,84,223,139,223,146,
		150,53,161,187,167,163,61,
		45,242,115,110,195,89,15
	},

	{
		50,197,196,115,105,176,64,
		87,141,157,64,185,202,118,
		158,70,79,168,121,141,57,
		163,128,141,228,192,195,115,
		15,227,176,28,130,126,54,
		75,200,45,202,7,158,179,
		77,23,142,127,110,31,141
	},

	{
		123,175,80,224,82,146,27,
		61,247,16,236,96,150,244,
		102,13,165,47,253,185,96,
		178,149,204,82,2,235,182,
		47,249,110,211,181,241,87,
		93,208,215,155,65,168,65,
		152,71,236,50,249,80,249
	},

	{
		139,219,5,39,213,136,215,
		228,108,228,169,234,173,243,
		229,45,65,105,121,208,18,
		202,118,209,11,19,178,162,
		59,74,82,99,111,28,119,
		6,217,203,9,252,227,146,
		217,194,195,213,12,221,229
	},

	{
		49,9,169,202,201,231,152,
		102,226,150,26,173,50,161,
		241,73,224,232,42,44,182,
		48,85,6,112,192,109,58,
		164,25,233,113,68,229,93,
		83,32,42,74,152,119,240,
		95,234,245,83,222,203,49
	},

	{
		58,104,2,6,164,206,186,
		224,222,73,218,87,103,158,
		186,30,242,149,198,193,89,
		94,43,38,197,36,33,64,
		7,136,243,253,80,61,90,
		223,72,116,47,46,190,94,
		50,77,217,111,227,35,30
	},

	{
		161,5,6,26,113,239,46,
		35,195,65,36,225,119,8,
		31,27,206,249,207,129,119,
		218,138,239,90,154,78,217,
		247,85,161,87,123,185,175,
		152,74,2,181,30,66,9,
		30,147,91,147,86,146,232
	},

	{
		111,204,65,49,124,16,12,
		149,51,137,89,252,190,203,
		155,194,84,231,136,213,197,
		202,212,73,251,17,80,12,
		106,75,180,1,15,52,1,
		36,27,194,180,13,226,151,
		210,106,27,190,237,194,16
	},

	{
		63,38,43,226,100,106,44,
		100,72,214,138,170,142,137,
		51,246,203,190,250,5,139,
		146,105,90,117,73,83,168,
		127,89,106,238,176,54,135,
		79,97,13,229,87,119,110,
		253,184,151,32,70,14,191
	},

	{
		3,134,209,61,87,248,16,
		140,69,243,241,249,240,83,
		184,156,4,81,67,134,241,
		25,176,185,228,181,65,200,
		143,165,255,165,222,193,94,
		146,188,178,68,171,218,177,
		198,3,208,189,1,204,100
	},

	{
		22,66,132,87,17,19,147,
		142,42,56,105,247,102,68,
		124,234,205,209,46,130,54,
		254,10,162,22,63,87,26,
		148,32,114,179,139,23,239,
		128,191,233,238,76,251,218,
		169,126,70,24,167,70,244
	},

	{
		156,153,114,200,16,71,190,
		214,189,154,249,102,57,40,
		212,144,211,72,187,194,106,
		32,131,241,173,161,83,169,
		128,245,153,168,115,72,94,
		90,15,107,218,206,232,88,
		162,109,52,236,54,42,212
	},

	{
		242,93,89,143,226,33,252,
		126,83,174,156,75,156,21,
		131,85,147,30,130,90,228,
		40,2,178,7,79,138,196,
		44,201,134,186,63,196,113,
		28,89,156,53,96,166,205,
		213,15,57,112,70,138,190
	},

	{
		148,127,27,140,97,157,84,
		2,67,71,99,182,13,250,
		113,100,84,156,156,16,242,
		216,176,107,248,176,40,208,
		235,220,159,243,169,56,65,
		159,199,244,77,142,201,210,
		81,198,4,7,43,126,122
	},

	{
		23,101,250,171,81,20,89,
		128,51,189,208,81,88,108,
		69,78,59,131,199,38,130,
		102,215,54,7,153,251,58,
		80,98,150,172,193,144,115,
		212,46,119,19,184,10,188,
		14,237,241,16,173,125,191
	},

	{
		97,19,206,235,176,242,227,
		101,12,97,196,163,218,152,
		104,40,3,63,3,39,15,
		31,4,138,217,199,156,12,
		239,16,55,165,35,249,155,
		252,132,220,53,22,68,104,
		63,246,158,93,50,126,104
	},

	{
		136,86,29,88,238,198,176,
		148,161,188,49,187,54,131,
		114,216,90,238,169,131,120,
		150,234,162,232,205,105,255,
		161,150,158,52,226,116,23,
		23,218,114,33,50,169,53,
		18,105,210,117,86,89,125
	},

	{
		178,13,149,148,121,91,82,
		38,29,64,153,12,44,11,
		161,74,139,68,88,35,124,
		42,191,86,119,137,56,127,
		38,40,69,17,89,107,234,
		226,90,137,67,200,96,232,
		79,43,143,239,180,33,81
	},

	{
		254,112,239,101,191,19,131,
		123,153,174,191,181,101,3,
		46,221,48,253,2,188,62,
		48,225,85,249,74,118,101,
		32,58,51,4,196,252,82,
		178,127,201,119,193,107,155,
		252,77,54,194,83,192,240
	},

	{
		141,117,43,233,154,235,63,
		50,253,75,91,22,195,180,
		105,62,225,65,245,173,73,
		222,82,82,21,138,31,105,
		125,103,13,104,162,169,56,
		165,203,198,233,117,38,240,
		39,243,174,64,209,60,76
	},

	{
		159,196,56,192,208,188,198,
		122,162,99,117,240,187,85,
		122,142,242,127,51,213,189,
		170,210,194,148,133,133,29,
		217,17,181,176,52,189,178,
		36,200,26,107,114,103,92,
		121,166,6,179,52,234,142
	},

	{
		164,250,146,128,92,129,33,
		77,59,62,251,236,133,203,
		203,174,137,91,50,172,10,
		10,235,227,82,61,235,101,
		200,199,150,159,83,13,162,
		42,100,220,210,25,21,56,
		254,76,23,196,161,169,214
	},

	{
		22,112,203,180,148,28,10,
		150,46,218,99,193,146,186,
		52,61,36,113,59,173,108,
		178,223,233,55,161,52,50,
		17,128,164,93,112,43,216,
		202,37,131,180,130,100,82,
		237,77,23,199,63,30,158
	},

	{
		47,156,173,221,226,110,149,
		164,39,71,206,74,250,7,
		218,158,122,242,131,252,231,
		165,119,158,248,92,168,3,
		119,157,186,136,214,160,95,
		7,206,124,253,149,155,145,
		105,46,239,35,23,126,58
	},

	{
		159,159,134,88,135,222,68,
		64,110,148,41,201,20,219,
		144,41,1,72,230,41,167,
		16,194,210,133,8,165,130,
		47,225,148,17,203,103,84,
		43,236,41,98,94,76,91,
		133,8,79,85,16,175,221
	},

	{
		115,161,236,118,192,144,104,
		255,59,191,32,78,18,14,
		108,203,171,32,254,80,206,
		178,84,132,206,60,193,226,
		4,8,188,193,118,133,91,
		87,13,9,191,67,23,181,
		124,134,134,191,130,95,195
	},

	{
		202,187,20,83,145,231,14,
		74,153,208,76,200,34,106,
		157,107,9,255,160,250,224,
		5,251,57,230,149,200,39,
		39,79,170,143,95,141,130,
		62,7,170,74,174,17,85,
		238,144,174,152,202,247,202
	},

	{
		173,144,151,112,43,60,201,
		219,175,238,226,103,153,58,
		119,15,237,206,33,193,243,
		253,102,63,183,24,77,166,
		242,11,72,143,127,79,246,
		188,238,128,118,67,87,183,
		175,79,239,6,108,52,153
	},

	{
		19,148,26,226,91,100,230,
		169,250,250,171,233,136,16,
		69,214,232,55,111,210,171,
		116,246,96,204,142,82,166,
		104,230,80,151,80,59,13,
		94,247,35,244,34,181,156,
		5,183,210,139,3,95,85
	},

	{
		125,219,35,9,131,243,232,
		189,133,142,219,41,228,14,
		55,114,188,66,24,210,163,
		124,99,75,61,165,94,228,
		127,6,12,114,246,253,243,
		77,231,101,85,4,164,200,
		117,117,240,63,15,76,203
	},

	{
		9,134,85,112,185,245,169,
		239,37,166,32,55,179,23,
		7,216,118,32,145,113,152,
		81,254,2,110,33,225,26,
		221,253,151,32,87,233,78,
		155,18,169,163,207,196,34,
		77,135,154,3,192,96,146
	},

	{
		75,91,117,7,210,246,189,
		146,84,107,248,11,41,245,
		123,204,73,243,191,15,151,
		245,1,109,44,157,204,112,
		153,55,210,68,226,149,209,
		80,22,29,254,243,76,121,
		172,17,1,242,167,176,116
	},

	{
		230,254,102,33,70,173,175,
		228,171,233,19,239,231,124,
		38,24,147,237,60,159,138,
		243,245,16,215,222,97,60,
		1,116,241,4,46,3,242,
		237,32,9,197,54,119,77,
		137,95,57,179,123,232,16
	},

	{
		183,187,60,61,212,166,108,
		187,190,151,215,122,188,39,
		86,57,254,185,238,247,79,
		142,209,107,232,255,138,122,
		212,48,23,47,163,228,255,
		32,224,187,246,195,5,116,
		28,160,96,142,147,222,171
	},

	{
		229,93,42,194,149,50,89,
		58,204,91,225,69,212,47,
		124,178,247,73,3,94,144,
		13,244,241,225,151,159,73,
		16,171,96,76,202,185,250,
		174,10,152,121,68,111,6,
		140,114,166,133,234,104,94
	},

	{
		186,157,156,42,213,188,78,
		242,32,136,103,58,86,99,
		171,229,203,151,101,196,15,
		109,148,140,4,130,208,135,
		52,214,212,167,66,145,243,
		54,61,240,104,223,212,87,
		181,68,19,247,26,217,165
	},

	{
		20,68,71,101,64,63,241,
		25,177,222,2,32,2,140,
		138,20,101,194,10,249,205,
		25,11,29,236,53,13,41,
		149,115,115,159,108,172,186,
		56,97,223,149,79,240,33,
		74,244,153,175,230,173,139
	},

	{
		112,160,137,249,193,254,166,
		20,237,27,169,28,25,166,
		89,133,11,238,230,171,153,
		149,249,253,37,58,38,48,
		226,198,210,91,107,161,50,
		129,73,109,104,128,240,157,
		106,209,245,142,188,44,209
	},

	{
		94,155,95,27,57,178,126,
		126,85,239,193,85,88,143,
		21,44,88,255,219,33,48,
		160,202,89,107,12,103,123,
		111,249,20,123,41,72,38,
		150,235,92,3,204,197,84,
		176,187,243,59,226,49,184
	},

	{
		202,228,64,73,214,118,70,
		4,246,243,163,181,137,217,
		140,24,167,19,201,69,35,
		1,179,95,32,201,86,26,
		111,210,1,98,46,226,39,
		226,217,237,225,141,89,245,
		240,244,44,2,143,156,58
	},

	{
		157,246,3,119,213,223,157,
		231,63,233,113,19,222,44,
		151,145,130,222,78,190,130,
		93,215,44,86,109,232,234,
		249,24,206,73,167,235,11,
		22,201,8,161,6,173,196,
		202,81,229,72,71,33,216
	},

	{
		245,6,229,112,241,89,47,
		99,172,118,22,20,193,113,
		230,122,251,107,255,10,232,
		55,158,24,188,30,159,178,
		246,6,211,101,134,171,245,
		216,223,48,231,212,198,167,
		71,142,46,234,237,200,4
	}
};


static UINT8 const* GetRotationArray()
{
	Assert(guiJA2EncryptionSet < lengthof(g_encryption_array));
	return g_encryption_array[guiJA2EncryptionSet];
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(TacticalSave, asserts)
{
	EXPECT_EQ(lengthof(g_encryption_array), static_cast<size_t>(BASE_NUMBER_OF_ROTATION_ARRAYS * 12));
}

#endif
