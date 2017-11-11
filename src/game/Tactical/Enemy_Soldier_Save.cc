#include <stdexcept>

#include "Enemy_Soldier_Save.h"
#include "LoadSaveSoldierCreate.h"
#include "StrategicMap.h"
#include "Overhead.h"
#include "Soldier_Add.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "Debug.h"
#include "Random.h"
#include "Items.h"
#include "Map_Information.h"
#include "Soldier_Profile.h"
#include "EditorMercs.h"
#include "Animation_Data.h"
#include "Tactical_Save.h"
#include "Campaign_Types.h"
#include "Game_Clock.h"
#include "Queen_Command.h"
#include "Scheduling.h"
#include "MemMan.h"
#include "FileMan.h"

#include "ContentManager.h"
#include "GameInstance.h"

BOOLEAN gfRestoringEnemySoldiersFromTempFile = FALSE;
BOOLEAN gfRestoringCiviliansFromTempFile = FALSE;


static void RemoveTempFile(INT16 const x, INT16 const y, INT8 const z, SectorFlags const file_flag)
{
	if (!GetSectorFlagStatus(x, y, z, file_flag)) return;

	// Delete any temp file that is here and toast the flag that says one exists.
	ReSetSectorFlag(x, y, z, file_flag);
	char filename[128];
	GetMapTempFileName(file_flag, filename, x, y, z);
	FileDelete(filename);
}


// OLD SAVE METHOD:  This is the old way of loading the enemies and civilians
void LoadEnemySoldiersFromTempFile()
{
	gfRestoringEnemySoldiersFromTempFile = TRUE;

	INT16 const x = gWorldSectorX;
	INT16 const y = gWorldSectorY;
	INT8  const z = gbWorldSectorZ;

	// STEP ONE: Set up the temp file to read from.
	char map_name[128];
	GetMapTempFileName(SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, map_name, x, y, z);
	AutoSGPFile f(GCM->openGameResForReading(map_name));

	// STEP TWO: Determine whether or not we should use this data.  Because it
	// is the demo, it is automatically used.

	INT16 saved_y;
	FileRead(f, &saved_y, 2);
	if (y != saved_y)
	{
		throw std::runtime_error("Sector Y mismatch");
	}

	LoadSoldierInitListLinks(f);

	// STEP THREE: Read the data

	INT16 saved_x;
	FileRead(f, &saved_x, 2);
	if (x != saved_x)
	{
		throw std::runtime_error("Sector X mismatch");
	}

	INT32 saved_slots;
	FileRead(f, &saved_slots, 4);
	INT32 const slots = saved_slots;

	UINT32 timestamp;
	FileRead(f, &timestamp, 4);

	INT8 saved_z;
	FileRead(f, &saved_z, 1);
	if (z != saved_z)
	{
		throw std::runtime_error("Sector Z mismatch");
	}

	if (GetWorldTotalMin() > timestamp + 300)
	{
		// The file has aged.  Use the regular method for adding soldiers.
		f.Deallocate(); // Close the file before deleting it
		RemoveTempFile(x, y, z, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS);
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		return;
	}

	if (slots == 0)
	{
		// No need to restore the enemy's to the map.  This means we are restoring
		// a saved game.
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		return;
	}

	if (slots < 0 || 64 <= slots)
	{
		//bad IO!
		throw std::runtime_error("Invalid slot count");
	}

	// For all the enemy (enemy/creature) and civilian slots, clear the
	// fPriorityExistance flag.  We will use these flags to determine which
	// slots have been modified as we load the data into the map pristine
	// soldier init list.
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		BASIC_SOLDIERCREATE_STRUCT* const bp = curr->pBasicPlacement;
		if (!bp->fPriorityExistance)
			continue;
		if (bp->bTeam != ENEMY_TEAM && bp->bTeam != CREATURE_TEAM && bp->bTeam != CIV_TEAM)
			continue;
		bp->fPriorityExistance = FALSE;
	}

	//get the number of enemies in this sector.
	UINT8 ubStrategicElites;
	UINT8 ubStrategicTroops;
	UINT8 ubStrategicAdmins;
	UINT8 ubStrategicCreatures;
	if (z != 0)
	{
		UNDERGROUND_SECTORINFO const* const pSector = FindUnderGroundSector(x, y, z);
		if (!pSector)
		{
			throw std::runtime_error("Missing underground sector info");
		}
		ubStrategicElites = pSector->ubNumElites;
		ubStrategicTroops = pSector->ubNumTroops;
		ubStrategicAdmins = pSector->ubNumAdmins;
		ubStrategicCreatures = pSector->ubNumCreatures;
	}
	else
	{
		SECTORINFO const* const pSector = &SectorInfo[SECTOR(x, y)];
		ubStrategicCreatures = pSector->ubNumCreatures;
		GetNumberOfEnemiesInSector(x, y, &ubStrategicAdmins, &ubStrategicTroops, &ubStrategicElites);
	}

	UINT8 ubNumElites    = 0;
	UINT8 ubNumTroops    = 0;
	UINT8 ubNumAdmins    = 0;
	UINT8 ubNumCreatures = 0;
	for (INT32 i = 0; i < slots; ++i)
	{
		SOLDIERCREATE_STRUCT tempDetailedPlacement;
		UINT16 saved_checksum;
		ExtractSoldierCreateFromFileWithChecksumAndGuess(f, &tempDetailedPlacement, &saved_checksum);
		FOR_EACH_SOLDIERINITNODE(curr)
		{
			BASIC_SOLDIERCREATE_STRUCT* const bp = curr->pBasicPlacement;
			if (bp->fPriorityExistance)
				continue;
			if (bp->bTeam != tempDetailedPlacement.bTeam)
				continue;

			SOLDIERCREATE_STRUCT* dp = curr->pDetailedPlacement;
			if (dp && dp->ubProfile != NO_PROFILE)
				continue;

			bp->fPriorityExistance = TRUE;

			if (!dp)
			{
				// Need to upgrade the placement to detailed placement
				dp = MALLOC(SOLDIERCREATE_STRUCT);
				curr->pDetailedPlacement = dp;
			}
			// Now replace the map pristine placement info with the temp map file
			// version.
			*dp = tempDetailedPlacement;

			bp->fPriorityExistance = TRUE;
			bp->bDirection         = dp->bDirection;
			bp->bOrders            = dp->bOrders;
			bp->bAttitude          = dp->bAttitude;
			bp->bBodyType          = dp->bBodyType;
			bp->fOnRoof            = dp->fOnRoof;
			bp->ubSoldierClass     = dp->ubSoldierClass;
			bp->ubCivilianGroup    = dp->ubCivilianGroup;
			bp->fHasKeys           = dp->fHasKeys;
			bp->usStartingGridNo   = dp->sInsertionGridNo;
			bp->bPatrolCnt         = dp->bPatrolCnt;
			memcpy(bp->sPatrolGrid, dp->sPatrolGrid, sizeof(INT16) * bp->bPatrolCnt);

			// Verify the checksum equation (anti-hack) -- see save
			UINT16 const checksum = CalcSoldierCreateCheckSum(dp);
			if (saved_checksum != checksum)
			{
				// Hacker has modified the stats on the enemy placements.
				throw std::runtime_error("Invalid checksum for placement");
			}

			if (bp->bTeam != CIV_TEAM)
			{
				switch (bp->ubSoldierClass)
				{
					// Add preserved placements as long as they don't exceed the actual
					// population.
					case SOLDIER_CLASS_ELITE:
						if (++ubNumElites >= ubStrategicElites)
							goto no_add;
						break;
					case SOLDIER_CLASS_ARMY:
						if (++ubNumTroops >= ubStrategicTroops)
							goto no_add;
						break;
					case SOLDIER_CLASS_ADMINISTRATOR:
						if (++ubNumAdmins >= ubStrategicAdmins)
							goto no_add;
						break;
					case SOLDIER_CLASS_CREATURE:
						if (++ubNumCreatures >= ubStrategicCreatures)
							goto no_add;
						break;
					default:
						goto no_add;
				}
			}
			AddPlacementToWorld(curr);
no_add:
			break;
		}
	}

	UINT8 saved_sector_id;
	FileRead(f, &saved_sector_id, 1);
	if (saved_sector_id != SECTOR(x, y))
	{
		throw std::runtime_error("Sector ID mismatch");
	}

	// Now add any extra enemies that have arrived since the temp file was made.
	if (ubStrategicTroops > ubNumTroops ||
		ubStrategicElites > ubNumElites ||
		ubStrategicAdmins > ubNumAdmins)
	{
		ubStrategicTroops = ubStrategicTroops > ubNumTroops ? ubStrategicTroops - ubNumTroops : 0;
		ubStrategicElites = ubStrategicElites > ubNumElites ? ubStrategicElites - ubNumElites : 0;
		ubStrategicAdmins = ubStrategicAdmins > ubNumAdmins ? ubStrategicAdmins - ubNumAdmins : 0;
		AddSoldierInitListEnemyDefenceSoldiers(ubStrategicAdmins, ubStrategicTroops, ubStrategicElites);
	}
}


static void CountNumberOfElitesRegularsAdminsAndCreaturesFromEnemySoldiersTempFile(UINT8* n_elites, UINT8* n_regulars, UINT8* n_admins, UINT8* n_creatures);


void NewWayOfLoadingEnemySoldiersFromTempFile()
{
	UINT8 ubStrategicElites;
	UINT8 ubStrategicTroops;
	UINT8 ubStrategicAdmins;
	UINT8 ubStrategicCreatures;

	gfRestoringEnemySoldiersFromTempFile = TRUE;

	INT16 const x = gWorldSectorX;
	INT16 const y = gWorldSectorY;
	INT8  const z = gbWorldSectorZ;

	// Count the number of enemies (elites, regulars, admins and creatures) that
	// are in the temp file.
	UNDERGROUND_SECTORINFO const* underground_info = NULL;
	UINT8                         ubNumElites      = 0;
	UINT8                         ubNumTroops      = 0;
	UINT8                         ubNumAdmins      = 0;
	UINT8                         ubNumCreatures   = 0;
	if (z != 0)
	{
		underground_info = FindUnderGroundSector(x, y, z);
		if (!underground_info)
		{
			throw std::runtime_error("Missing underground sector info");
		}
	}
	else
	{
		SECTORINFO const* const sector_info = &SectorInfo[SECTOR(x, y)];
		ubNumElites    = sector_info->ubNumElites;
		ubNumTroops    = sector_info->ubNumTroops;
		ubNumAdmins    = sector_info->ubNumAdmins;
		ubNumCreatures = sector_info->ubNumCreatures;
	}

	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		// Get the number of enemies form the temp file
		CountNumberOfElitesRegularsAdminsAndCreaturesFromEnemySoldiersTempFile(&ubStrategicElites, &ubStrategicTroops, &ubStrategicAdmins, &ubStrategicCreatures);
		// If any of the counts differ from what is in memory
		if (ubStrategicElites != ubNumElites ||
			ubStrategicTroops != ubNumTroops ||
			ubStrategicAdmins != ubNumAdmins ||
			ubStrategicCreatures != ubNumCreatures)
		{
			RemoveTempFile(x, y, z, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS);
			return;
		}
	}

	// reset
	ubNumElites    = 0;
	ubNumTroops    = 0;
	ubNumAdmins    = 0;
	ubNumCreatures = 0;

	// STEP ONE:  Set up the temp file to read from.
	char map_name[128];
	GetMapTempFileName(SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, map_name, x, y, z);
	AutoSGPFile f(GCM->openGameResForReading(map_name));

	// STEP TWO:  Determine whether or not we should use this data.  Because it
	// is the demo, it is automatically used.

	INT16 saved_y;
	FileRead(f, &saved_y, 2);
	if (y != saved_y)
	{
		throw std::runtime_error("Sector Y mismatch");
	}

	NewWayOfLoadingEnemySoldierInitListLinks(f);

	// STEP THREE:  read the data

	INT16 saved_x;
	FileRead(f, &saved_x, 2);
	if (x != saved_x)
	{
		throw std::runtime_error("Sector X mismatch");
	}

	INT32 saved_slots;
	FileRead(f, &saved_slots, 4);
	INT32 const slots = saved_slots;

	UINT32 timestamp;
	FileRead(f, &timestamp, 4);

	INT8 saved_z;
	FileRead(f, &saved_z, 1);
	if (z != saved_z)
	{
		throw std::runtime_error("Sector Z mismatch");
	}

	if (GetWorldTotalMin() > timestamp + 300)
	{
		// The file has aged.  Use the regular method for adding soldiers.
		f.Deallocate(); // Close the file before deleting it
		RemoveTempFile(x, y, z, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS);
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		return;
	}

	if (slots == 0)
	{
		// no need to restore the enemy's to the map.  This means we are restoring
		// a saved game.
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		return;
	}

	if (slots < 0 || 64 <= slots)
	{
		//bad IO!
		throw std::runtime_error("Invalid slot count");
	}

	// For all the enemy slots (enemy/creature), clear the fPriorityExistance
	// flag.  We will use these flags to determine which slots have been
	// modified as we load the data into the map pristine soldier init list.
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		BASIC_SOLDIERCREATE_STRUCT* const bp = curr->pBasicPlacement;
		if (!bp->fPriorityExistance)
			continue;
		if (bp->bTeam != ENEMY_TEAM && bp->bTeam != CREATURE_TEAM)
			continue;
		bp->fPriorityExistance = FALSE;
	}

	// Get the number of enemies in this sector.
	if (z != 0)
	{
		ubStrategicElites = underground_info->ubNumElites;
		ubStrategicTroops = underground_info->ubNumTroops;
		ubStrategicAdmins = underground_info->ubNumAdmins;
		ubStrategicCreatures = underground_info->ubNumCreatures;
	}
	else
	{
		SECTORINFO const* const sector_info = &SectorInfo[SECTOR(x, y)];
		ubStrategicCreatures = sector_info->ubNumCreatures;
		GetNumberOfEnemiesInSector(x, y, &ubStrategicAdmins, &ubStrategicTroops, &ubStrategicElites);
	}

	for (INT32 i = 0; i != slots; ++i)
	{
		UINT16 saved_checksum;
		SOLDIERCREATE_STRUCT tempDetailedPlacement;
		ExtractSoldierCreateFromFileWithChecksumAndGuess(f, &tempDetailedPlacement, &saved_checksum);
		FOR_EACH_SOLDIERINITNODE(curr)
		{
			BASIC_SOLDIERCREATE_STRUCT* const bp = curr->pBasicPlacement;
			if (bp->fPriorityExistance)
				continue;
			if (bp->bTeam != tempDetailedPlacement.bTeam)
				continue;

			bp->fPriorityExistance = TRUE;
			SOLDIERCREATE_STRUCT* dp = curr->pDetailedPlacement;
			if (!dp)
			{
				// Need to upgrade the placement to detailed placement
				dp = MALLOC(SOLDIERCREATE_STRUCT);
				curr->pDetailedPlacement = dp;
			}
			// Now replace the map pristine placement info with the temp map file
			// version.
			*dp = tempDetailedPlacement;

			bp->fPriorityExistance  =  TRUE;
			bp->bDirection          = dp->bDirection;
			bp->bOrders             = dp->bOrders;
			bp->bAttitude           = dp->bAttitude;
			bp->bBodyType           = dp->bBodyType;
			bp->fOnRoof             = dp->fOnRoof;
			bp->ubSoldierClass      = dp->ubSoldierClass;
			bp->ubCivilianGroup     = dp->ubCivilianGroup;
			bp->fHasKeys            = dp->fHasKeys;
			bp->usStartingGridNo    = dp->sInsertionGridNo;
			bp->bPatrolCnt          = dp->bPatrolCnt;
			memcpy(bp->sPatrolGrid, dp->sPatrolGrid, sizeof(INT16) * bp->bPatrolCnt);

			// verify the checksum equation (anti-hack) -- see save
			UINT16 const checksum = CalcSoldierCreateCheckSum(dp);
			if (saved_checksum != checksum)
			{
				// Hacker has modified the stats on the enemy placements.
				throw std::runtime_error("Invalid checksum for placement");
			}

			// Add preserved placements as long as they don't exceed the actual
			// population.
			switch (bp->ubSoldierClass)
			{
				case SOLDIER_CLASS_ELITE:
					++ubNumElites;
					break;
				case SOLDIER_CLASS_ARMY:
					++ubNumTroops;
					break;
				case SOLDIER_CLASS_ADMINISTRATOR:
					++ubNumAdmins;
					break;
				case SOLDIER_CLASS_CREATURE:
					++ubNumCreatures;
					break;
			}
			break;
		}
	}

	UINT8 saved_sector_id;
	FileRead(f, &saved_sector_id, 1);
	if (saved_sector_id != SECTOR(x, y))
	{
		throw std::runtime_error("Sector ID mismatch");
	}

	//now add any extra enemies that have arrived since the temp file was made.
	if (ubStrategicTroops > ubNumTroops ||
		ubStrategicElites > ubNumElites ||
		ubStrategicAdmins > ubNumAdmins)
	{
		ubStrategicTroops = ubStrategicTroops > ubNumTroops ? ubStrategicTroops - ubNumTroops : 0;
		ubStrategicElites = ubStrategicElites > ubNumElites ? ubStrategicElites - ubNumElites : 0;
		ubStrategicAdmins = ubStrategicAdmins > ubNumAdmins ? ubStrategicAdmins - ubNumAdmins : 0;
		AddSoldierInitListEnemyDefenceSoldiers(ubStrategicAdmins, ubStrategicTroops, ubStrategicElites);
	}
}


void NewWayOfLoadingCiviliansFromTempFile()
{
	gfRestoringCiviliansFromTempFile = TRUE;

	INT16 const x = gWorldSectorX;
	INT16 const y = gWorldSectorY;
	INT8  const z = gbWorldSectorZ;

	// STEP ONE: Set up the temp file to read from.
	char map_name[128];
	GetMapTempFileName(SF_CIV_PRESERVED_TEMP_FILE_EXISTS, map_name, x, y, z);
	AutoSGPFile f(GCM->openGameResForReading(map_name));

	// STEP TWO:  Determine whether or not we should use this data.  Because it
	// is the demo, it is automatically used.

	INT16 saved_y;
	FileRead(f, &saved_y, 2);
	if (y != saved_y)
	{
		throw std::runtime_error("Sector Y mismatch");
	}

	NewWayOfLoadingCivilianInitListLinks(f);

	// STEP THREE:  read the data

	INT16 saved_x;
	FileRead(f, &saved_x, 2);
	if (x != saved_x)
	{
		throw std::runtime_error("Sector X mismatch");
	}

	INT32 saved_slots;
	FileRead(f, &saved_slots, 4);
	INT32 const slots = saved_slots;

	UINT32 timestamp;
	FileRead(f, &timestamp, 4);
	UINT32 const time_since_last_loaded = GetWorldTotalMin() - timestamp;

	INT8 saved_z;
	FileRead(f, &saved_z, 1);
	if (z != saved_z)
	{
		throw std::runtime_error("Sector Z mismatch");
	}

	if (slots == 0)
	{
		// No need to restore the civilians to the map.  This means we are
		// restoring a saved game.
		gfRestoringCiviliansFromTempFile = FALSE;
		return;
	}

	if (slots < 0 || 64 <= slots)
	{ //bad IO!
		throw std::runtime_error("Invalid slot count");
	}

	// For all the civilian slots, clear the fPriorityExistance flag.  We will
	// use these flags to determine which slots have been modified as we load
	// the data into the map pristine soldier init list.
	CFOR_EACH_SOLDIERINITNODE(curr)
	{
		BASIC_SOLDIERCREATE_STRUCT* const bp = curr->pBasicPlacement;
		if (!bp->fPriorityExistance) continue;
		if (bp->bTeam != CIV_TEAM)   continue;
		bp->fPriorityExistance = FALSE;
	}

	SOLDIERCREATE_STRUCT tempDetailedPlacement;
	for (INT32 i = 0; i != slots; ++i)
	{
		UINT16 saved_checksum;
		ExtractSoldierCreateFromFileWithChecksumAndGuess(f, &tempDetailedPlacement, &saved_checksum);
		FOR_EACH_SOLDIERINITNODE(curr)
		{
			BASIC_SOLDIERCREATE_STRUCT* const bp = curr->pBasicPlacement;
			if (bp->fPriorityExistance)
				continue;
			if (bp->bTeam != tempDetailedPlacement.bTeam)
				continue;

			SOLDIERCREATE_STRUCT* dp = curr->pDetailedPlacement;
			if (dp && dp->ubProfile != NO_PROFILE)
				continue;

			bp->fPriorityExistance = TRUE;

			if (!dp)
			{
				// Need to upgrade the placement to detailed placement
				dp = MALLOC(SOLDIERCREATE_STRUCT);
				curr->pDetailedPlacement = dp;
			}
			// Now replace the map pristine placement info with the temp map file
			// version.
			*dp = tempDetailedPlacement;

			bp->fPriorityExistance = TRUE;
			bp->bDirection         = dp->bDirection;
			bp->bOrders            = dp->bOrders;
			bp->bAttitude          = dp->bAttitude;
			bp->bBodyType          = dp->bBodyType;
			bp->fOnRoof            = dp->fOnRoof;
			bp->ubSoldierClass     = dp->ubSoldierClass;
			bp->ubCivilianGroup    = dp->ubCivilianGroup;
			bp->fHasKeys           = dp->fHasKeys;
			bp->usStartingGridNo   = dp->sInsertionGridNo;
			bp->bPatrolCnt         = dp->bPatrolCnt;
			memcpy(bp->sPatrolGrid, dp->sPatrolGrid, sizeof(INT16) * bp->bPatrolCnt);

			// Verify the checksum equation (anti-hack) -- see save
			UINT16 const checksum = CalcSoldierCreateCheckSum(curr->pDetailedPlacement);
			if (saved_checksum != checksum)
			{
				// Hacker has modified the stats on the civilian placements.
				throw std::runtime_error("Invalid checksum for placement");
			}

			if (dp->bLife < dp->bLifeMax)
			{
				// Add 4 life for every hour that passes.
				INT32 const new_life = MIN(dp->bLife + time_since_last_loaded / 15, dp->bLifeMax);
				dp->bLife = (INT8)new_life;
			}

			if (bp->bTeam == CIV_TEAM)
				break;
		}
	}

	// now remove any non-priority placement which matches the conditions!
	FOR_EACH_SOLDIERINITNODE_SAFE(curr)
	{
		BASIC_SOLDIERCREATE_STRUCT const* const bp = curr->pBasicPlacement;
		if (bp->fPriorityExistance)
			continue;
		if (bp->bTeam != tempDetailedPlacement.bTeam)
			continue;
		SOLDIERCREATE_STRUCT       const* const dp = curr->pDetailedPlacement;
		if (dp && dp->ubProfile != NO_PROFILE)
			continue;
		RemoveSoldierNodeFromInitList(curr);
	}

	UINT8 saved_sector_id;
	FileRead(f, &saved_sector_id, 1);
#if 0 // XXX was commented out
	if (saved_sector_id != SECTOR(sSectorX, sSectorY))
	{
		throw std::runtime_error("Sector ID mismatch");
	}
#endif
}


// If we are saving a game and we are in the sector, we will need to preserve
// the links between the soldiers and the soldier init list.  Otherwise, the
// temp file will be deleted.
void NewWayOfSavingEnemyAndCivliansToTempFile(INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ, BOOLEAN const fEnemy, BOOLEAN const fValidateOnly)
{
	//if we are saving the enemy info to the enemy temp file
	UINT8       first_team;
	UINT8       last_team;
	SectorFlags file_flag;
	if (fEnemy)
	{
		first_team = ENEMY_TEAM;
		last_team  = CREATURE_TEAM;
		file_flag  = SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS;
	}
	else
	{
		// It's the civilian team
		first_team = CIV_TEAM;
		last_team  = CIV_TEAM;
		file_flag  = SF_CIV_PRESERVED_TEMP_FILE_EXISTS;
	}

	UINT8 const first = gTacticalStatus.Team[first_team].bFirstID;
	UINT8 const last  = gTacticalStatus.Team[last_team ].bLastID;

	// STEP ONE:  Prep the soldiers for saving

	// Modify the map's soldier init list to reflect the changes to the member's
	// still alive
	INT32 slots = 0;
	for (INT32 i = first; i <= last; ++i)
	{
		SOLDIERTYPE const& s = GetMan(i);

		// Make sure the person is active, alive, and is not a profiled person
		if (!s.bActive || s.bLife == 0 || s.ubProfile != NO_PROFILE)
			continue;
		// Soldier is valid, so find the matching soldier init list entry for modification.
		SOLDIERINITNODE* const curr = FindSoldierInitNodeBySoldier(s);
		if (!curr)
			continue;

		// Increment the counter, so we know how many there are.
		slots++;

		if (fValidateOnly)
			continue;
		if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME)
			continue;

		SOLDIERCREATE_STRUCT* dp = curr->pDetailedPlacement;
		if (!dp)
		{
			//need to upgrade the placement to detailed placement
			dp                                        = MALLOCZ(SOLDIERCREATE_STRUCT);
			curr->pDetailedPlacement                  = dp;
			curr->pBasicPlacement->fDetailedPlacement = TRUE;
		}

		//Copy over the data of the soldier.
		dp->ubProfile       = NO_PROFILE;
		dp->bLife           = s.bLife;
		dp->bLifeMax        = s.bLifeMax;
		dp->bAgility        = s.bAgility;
		dp->bDexterity      = s.bDexterity;
		dp->bExpLevel       = s.bExpLevel;
		dp->bMarksmanship   = s.bMarksmanship;
		dp->bMedical        = s.bMedical;
		dp->bMechanical     = s.bMechanical;
		dp->bExplosive      = s.bExplosive;
		dp->bLeadership     = s.bLeadership;
		dp->bStrength       = s.bStrength;
		dp->bWisdom         = s.bWisdom;
		dp->bAttitude       = s.bAttitude;
		dp->bOrders         = s.bOrders;
		dp->bMorale         = s.bMorale;
		dp->bAIMorale       = s.bAIMorale;
		dp->bBodyType       = s.ubBodyType;
		dp->ubCivilianGroup = s.ubCivilianGroup;
		dp->ubScheduleID    = s.ubScheduleID;
		dp->fHasKeys        = s.bHasKeys;
		dp->sSectorX        = s.sSectorX;
		dp->sSectorY        = s.sSectorY;
		dp->bSectorZ        = s.bSectorZ;
		dp->ubSoldierClass  = s.ubSoldierClass;
		dp->bTeam           = s.bTeam;
		dp->bDirection      = s.bDirection;

		// We don't want the player to think that all the enemies start in the exact
		// position when we left the map, so randomize the start locations either
		// current position or original position.
		if (PreRandom(2))
		{
			// Use current position
			dp->fOnRoof          = s.bLevel;
			dp->sInsertionGridNo = s.sGridNo;
		}
		else
		{
			// Use original position
			dp->fOnRoof          = curr->pBasicPlacement->fOnRoof;
			dp->sInsertionGridNo = curr->pBasicPlacement->usStartingGridNo;
		}

		wcslcpy(dp->name, s.name, lengthof(dp->name));

		// Copy patrol points
		dp->bPatrolCnt = s.bPatrolCnt;
		memcpy(dp->sPatrolGrid, s.usPatrolGrid, sizeof(dp->sPatrolGrid));

		// Copy colors for soldier based on the body type.
		strcpy(dp->HeadPal,  s.HeadPal);
		strcpy(dp->VestPal,  s.VestPal);
		strcpy(dp->SkinPal,  s.SkinPal);
		strcpy(dp->PantsPal, s.PantsPal);

		// Copy soldier's inventory
		memcpy(dp->Inv, s.inv, sizeof(dp->Inv));
	}

	if (slots == 0)
	{
		// No need to save anything, so return successfully
		RemoveTempFile(sSectorX, sSectorY, bSectorZ, file_flag);
		return;
	}

	if (fValidateOnly) return;

	// STEP TWO:  Set up the temp file to write to.

	char map_name[128];
	GetMapTempFileName(file_flag, map_name, sSectorX, sSectorY, bSectorZ);
	AutoSGPFile f(FileMan::openForWriting(map_name));

	FileWrite(f, &sSectorY, 2);

	// STEP THREE:  Save the data

	// This works for both civs and enemies
	SaveSoldierInitListLinks(f);

	FileWrite(f, &sSectorX, 2);

	// This check may appear confusing.  It is intended to abort if the player is
	// saving the game.  It is only supposed to preserve the links to the
	// placement list, so when we finally do leave the level with enemies
	// remaining, we will need the links that are only added when the map is
	// loaded, and are normally lost when restoring a save.
	if (gTacticalStatus.uiFlags & LOADING_SAVED_GAME)
	{
		slots = 0;
	}

	FileWrite(f, &slots, 4);

	UINT32 const timestamp = GetWorldTotalMin();
	FileWrite(f, &timestamp, 4);

	FileWrite(f, &bSectorZ, 1);

	// If we are saving the game, we don't need to preserve the soldier
	// information, just preserve the links to the placement list.
	if (!(gTacticalStatus.uiFlags & LOADING_SAVED_GAME))
	{
		for (INT32 i = first; i <= last; ++i)
		{
			SOLDIERTYPE const& s = GetMan(i);
			// CJC: note that bInSector is not required; the civ could be offmap!
			if (!s.bActive || s.bLife == 0 || s.ubProfile != NO_PROFILE)
				continue;

			// Soldier is valid, so find the matching soldier init list entry for modification.
			SOLDIERINITNODE const* const curr = FindSoldierInitNodeBySoldier(s);
			if (!curr)
				continue;

			SOLDIERCREATE_STRUCT const* const dp = curr->pDetailedPlacement;
			InjectSoldierCreateIntoFile(f, dp);
			// Insert a checksum equation (anti-hack)
			UINT16 const checksum = CalcSoldierCreateCheckSum(dp);
			FileWrite(f, &checksum, 2);
		}

		UINT8 const sector_id = SECTOR(sSectorX, sSectorY);
		FileWrite(f, &sector_id, 1);
	}

	SetSectorFlag(sSectorX, sSectorY, bSectorZ, file_flag);
}


static void CountNumberOfElitesRegularsAdminsAndCreaturesFromEnemySoldiersTempFile(UINT8* const n_elites, UINT8* const n_regulars, UINT8* const n_admins, UINT8* const n_creatures)
{
	// Make sure the variables are initialized
	*n_elites    = 0;
	*n_regulars  = 0;
	*n_admins    = 0;
	*n_creatures = 0;

	INT16 const x = gWorldSectorX;
	INT16 const y = gWorldSectorY;
	INT8  const z = gbWorldSectorZ;

	// STEP ONE: Set up the temp file to read from.
	char map_name[128];
	GetMapTempFileName(SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, map_name, x, y, z);
	AutoSGPFile f(GCM->openGameResForReading(map_name));

	// STEP TWO: Determine whether or not we should use this data.  Because it
	// is the demo, it is automatically used.

	INT16 saved_y;
	FileRead(f, &saved_y, 2);
	if (y != saved_y)
	{
		throw std::runtime_error("Sector Y mismatch");
	}

	NewWayOfLoadingEnemySoldierInitListLinks(f);

	// STEP THREE: Read the data

	INT16 saved_x;
	FileRead(f, &saved_x, 2);
	if (x != saved_x)
	{
		throw std::runtime_error("Sector X mismatch");
	}

	INT32 saved_slots = 0;
	FileRead(f, &saved_slots, 4);
	INT32 slots = saved_slots;

	// Skip timestamp
	FileSeek(f, 4, FILE_SEEK_FROM_CURRENT);

	INT8 saved_z;
	FileRead(f, &saved_z, 1);
	if (z != saved_z)
	{
		throw std::runtime_error("Sector Z mismatch");
	}

	if (slots == 0)
	{
		// No need to restore the enemy's to the map.  This means we are restoring
		// a saved game.
		return;
	}

	if (slots < 0 || 64 <= slots)
	{
		//bad IO!
		throw std::runtime_error("Invalid slot count");
	}

	for (INT32 i = 0; i != slots; ++i)
	{
		UINT16 saved_checksum;
		SOLDIERCREATE_STRUCT tempDetailedPlacement;
		ExtractSoldierCreateFromFileWithChecksumAndGuess(f, &tempDetailedPlacement, &saved_checksum);
		// Increment the current type of soldier
		switch (tempDetailedPlacement.ubSoldierClass)
		{
			case SOLDIER_CLASS_ELITE:
				++*n_elites;
				break;
			case SOLDIER_CLASS_ARMY:
				++*n_regulars;
				break;
			case SOLDIER_CLASS_ADMINISTRATOR:
				++*n_admins;
				break;
			case SOLDIER_CLASS_CREATURE:
				++*n_creatures;
				break;
		}
	}

	UINT8 saved_sector_id;
	FileRead(f, &saved_sector_id, 1);
	if (saved_sector_id != SECTOR(x, y))
	{
		throw std::runtime_error("Sector ID mismatch");
	}
}
