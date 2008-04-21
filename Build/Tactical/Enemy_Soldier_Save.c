#include "LoadSaveSoldierCreate.h"
#include "Types.h"
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


extern void AddPlacementToWorld( SOLDIERINITNODE *pNode );

BOOLEAN gfRestoringEnemySoldiersFromTempFile = FALSE;
BOOLEAN gfRestoringCiviliansFromTempFile = FALSE;


static void RemoveTempFile(const INT16 x, const INT16 y, const INT8 z, const UINT32 file_flag)
{
	if (!GetSectorFlagStatus(x, y, z, file_flag)) return;

	// Delete any temp file that is here and toast the flag that says one exists.
	ReSetSectorFlag(x, y, z, file_flag);
	char filename[128];
	GetMapTempFileName(file_flag, filename, x, y, z);
	FileDelete(filename);
}


static void RemoveEnemySoldierTempFile(const INT16 x, const INT16 y, const INT8 z)
{
	RemoveTempFile(x, y, z, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS);
}


static void RemoveCivilianTempFile(const INT16 x, const INT16 y, const INT8 z)
{
	RemoveTempFile(x, y, z, SF_CIV_PRESERVED_TEMP_FILE_EXISTS);
}


static UINT16 CalcSoldierCreateCheckSum(const SOLDIERCREATE_STRUCT* const s)
{
	return
		s->bLife            *  7 +
		s->bLifeMax         *  8 -
		s->bAgility         *  2 +
		s->bDexterity       *  1 +
		s->bExpLevel        *  5 -
		s->bMarksmanship    *  9 +
		s->bMedical         * 10 +
		s->bMechanical      *  3 +
		s->bExplosive       *  4 +
		s->bLeadership      *  5 +
		s->bStrength        *  7 +
		s->bWisdom          * 11 +
		s->bMorale          *  7 +
		s->bAIMorale        *  3 -
		s->bBodyType        *  7 +
		4                   *  6 +
		s->sSectorX         *  7 -
		s->ubSoldierClass   *  4 +
		s->bTeam            *  7 +
		s->bDirection       *  5 +
		s->fOnRoof          * 17 +
		s->sInsertionGridNo *  1 +
		3;
}


//OLD SAVE METHOD:  This is the old way of loading the enemies and civilians
BOOLEAN LoadEnemySoldiersFromTempFile()
{
	SOLDIERCREATE_STRUCT tempDetailedPlacement;
	INT32 i;
	INT32 slots = 0;
	UINT32 uiTimeStamp;
	HWFILE hfile;
	INT16 sSectorX, sSectorY;
	CHAR8		zMapName[ 128 ];
	#ifdef JA2TESTVERSION
		CHAR8		zReason[256];
	#endif
	INT8 bSectorZ;
	UINT8 ubSectorID;
	UINT8 ubNumElites = 0, ubNumTroops = 0, ubNumAdmins = 0, ubNumCreatures = 0;
	UINT8 ubStrategicElites, ubStrategicTroops, ubStrategicAdmins, ubStrategicCreatures;

	gfRestoringEnemySoldiersFromTempFile = TRUE;

	//STEP ONE:  Set up the temp file to read from.

	GetMapTempFileName( SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, zMapName, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//Open the file for reading
	hfile = FileOpen(zMapName, FILE_ACCESS_READ);
	if( hfile == 0 )
	{	//Error opening map modification file
		return FALSE;
	}

	//STEP TWO:  determine whether or not we should use this data.
	//because it is the demo, it is automatically used.

	if (!FileRead(hfile, &sSectorY, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading sSectorY.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorY != sSectorY )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- sSectorY mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	LoadSoldierInitListLinks( hfile );

	//STEP THREE:  read the data

	if (!FileRead(hfile, &sSectorX, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading sSectorX.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorX != sSectorX )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- sSectorX mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &slots, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading slots.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &uiTimeStamp, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading uiTimeStamp.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &bSectorZ, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading bSectorZ.  KM");
#endif
		goto FAIL_LOAD;
	}

	if( GetWorldTotalMin() > uiTimeStamp + 300 )
	{ //the file has aged.  Use the regular method for adding soldiers.
		FileClose( hfile );
		RemoveEnemySoldierTempFile( sSectorX, sSectorY, bSectorZ );
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		return TRUE;
	}

	if( gbWorldSectorZ != bSectorZ )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- bSectorZ mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if( !slots )
	{ //no need to restore the enemy's to the map.  This means we are restoring a saved game.
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		FileClose( hfile );
		return TRUE;
	}
	if( slots < 0 || slots >= 64 )
	{ //bad IO!
		#ifdef JA2TESTVERSION
			sprintf( zReason, "EnemySoldier -- illegal slot value of %d.  KM", slots );
		#endif
		goto FAIL_LOAD;
	}

	//For all the enemy slots (enemy/creature), clear the fPriorityExistance flag.  We will use these flags
	//to determine which slots have been modified as we load the data into the map pristine soldier init list.
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( curr->pBasicPlacement->fPriorityExistance )
		{
			if( curr->pBasicPlacement->bTeam == ENEMY_TEAM || curr->pBasicPlacement->bTeam == CREATURE_TEAM || curr->pBasicPlacement->bTeam == CIV_TEAM )
			{
				curr->pBasicPlacement->fPriorityExistance = FALSE;
			}
		}
	}

	//get the number of enemies in this sector.
	if( bSectorZ )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( sSectorX, sSectorY, bSectorZ );
		if( !pSector )
		{
		#ifdef JA2TESTVERSION
			sprintf( zReason, "EnemySoldier -- Couldn't find underground sector info for (%d,%d,%d)  KM", sSectorX, sSectorY, bSectorZ );
		#endif
			goto FAIL_LOAD;
		}
		ubStrategicElites		 = pSector->ubNumElites;
		ubStrategicTroops		 = pSector->ubNumTroops;
		ubStrategicAdmins		 = pSector->ubNumAdmins;
		ubStrategicCreatures = pSector->ubNumCreatures;
	}
	else
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
		ubStrategicCreatures = pSector->ubNumCreatures;
		GetNumberOfEnemiesInSector( sSectorX, sSectorY, &ubStrategicAdmins, &ubStrategicTroops, &ubStrategicElites );
	}

	for( i = 0; i < slots; i++ )
	{
		if (!ExtractSoldierCreateFromFile(hfile, &tempDetailedPlacement))
		{
			#ifdef JA2TESTVERSION
				sprintf( zReason, "EnemySoldier -- EOF while reading tempDetailedPlacment %d.  KM", i );
			#endif
			goto FAIL_LOAD;
		}
		FOR_ALL_SOLDIERINITNODES(curr)
		{
			if( !curr->pBasicPlacement->fPriorityExistance )
			{
				if( !curr->pDetailedPlacement || curr->pDetailedPlacement && curr->pDetailedPlacement->ubProfile == NO_PROFILE )
				{
					if( curr->pBasicPlacement->bTeam == tempDetailedPlacement.bTeam )
					{
						curr->pBasicPlacement->fPriorityExistance = TRUE;
						if( !curr->pDetailedPlacement )
						{ //need to upgrade the placement to detailed placement
							curr->pDetailedPlacement = MALLOC(SOLDIERCREATE_STRUCT);
						}
						//now replace the map pristine placement info with the temp map file version..
						*curr->pDetailedPlacement = tempDetailedPlacement;

						curr->pBasicPlacement->fPriorityExistance	=	TRUE;
						curr->pBasicPlacement->bDirection					= curr->pDetailedPlacement->bDirection;
						curr->pBasicPlacement->bOrders						= curr->pDetailedPlacement->bOrders;
						curr->pBasicPlacement->bAttitude					= curr->pDetailedPlacement->bAttitude;
						curr->pBasicPlacement->bBodyType					= curr->pDetailedPlacement->bBodyType;
						curr->pBasicPlacement->fOnRoof						= curr->pDetailedPlacement->fOnRoof;
						curr->pBasicPlacement->ubSoldierClass			= curr->pDetailedPlacement->ubSoldierClass;
						curr->pBasicPlacement->ubCivilianGroup		= curr->pDetailedPlacement->ubCivilianGroup;
						curr->pBasicPlacement->fHasKeys						= curr->pDetailedPlacement->fHasKeys;
						curr->pBasicPlacement->usStartingGridNo		= curr->pDetailedPlacement->sInsertionGridNo;

						curr->pBasicPlacement->bPatrolCnt			= curr->pDetailedPlacement->bPatrolCnt;
						memcpy( curr->pBasicPlacement->sPatrolGrid, curr->pDetailedPlacement->sPatrolGrid,
							sizeof( INT16 ) * curr->pBasicPlacement->bPatrolCnt );

						UINT16 usCheckSum;
						if (!FileRead(hfile, &usCheckSum, 2))
						{
							#ifdef JA2TESTVERSION
								sprintf( zReason, "EnemySoldier -- EOF while reading usCheckSum %d.  KM", i );
							#endif
							goto FAIL_LOAD;
						}
						//verify the checksum equation (anti-hack) -- see save
						const UINT16 usFileCheckSum = CalcSoldierCreateCheckSum(curr->pDetailedPlacement);
						if( usCheckSum != usFileCheckSum )
						{	//Hacker has modified the stats on the enemy placements.
							#ifdef JA2TESTVERSION
								sprintf( zReason, "EnemySoldier -- checksum for placement %d failed.  KM", i );
							#endif
							goto FAIL_LOAD;
						}

						if( curr->pBasicPlacement->bTeam == CIV_TEAM )
						{
							AddPlacementToWorld( curr );
							break;
						}
						else
						{
							//Add preserved placements as long as they don't exceed the actual population.
							switch( curr->pBasicPlacement->ubSoldierClass )
							{
								case SOLDIER_CLASS_ELITE:
									ubNumElites++;
									if( ubNumElites < ubStrategicElites )
									{
										AddPlacementToWorld( curr );
									}
									break;
								case SOLDIER_CLASS_ARMY:
									ubNumTroops++;
									if( ubNumTroops < ubStrategicTroops )
									{
										AddPlacementToWorld( curr );
									}
									break;
								case SOLDIER_CLASS_ADMINISTRATOR:
									ubNumAdmins++;
									if( ubNumAdmins < ubStrategicAdmins )
									{
										AddPlacementToWorld( curr );
									}
									break;
								case SOLDIER_CLASS_CREATURE:
									ubNumCreatures++;
									if( ubNumCreatures < ubStrategicCreatures )
									{
										AddPlacementToWorld( curr );
									}
									break;
							}
							break;
						}
					}
				}
			}
		}
	}

	if (!FileRead(hfile, &ubSectorID, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading ubSectorID.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( ubSectorID != SECTOR( sSectorX, sSectorY ) )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- ubSectorID mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	//now add any extra enemies that have arrived since the temp file was made.
	if( ubStrategicTroops > ubNumTroops || ubStrategicElites > ubNumElites || ubStrategicAdmins > ubNumAdmins )
	{
		ubStrategicTroops = ( ubStrategicTroops > ubNumTroops ) ? ubStrategicTroops - ubNumTroops : 0;
		ubStrategicElites = ( ubStrategicElites > ubNumElites ) ? ubStrategicElites - ubNumElites : 0;
		ubStrategicAdmins = ( ubStrategicAdmins > ubNumAdmins ) ? ubStrategicAdmins - ubNumAdmins : 0;
		AddSoldierInitListEnemyDefenceSoldiers( ubStrategicAdmins, ubStrategicTroops, ubStrategicElites );
	}
	if( ubStrategicCreatures > ubNumCreatures )
	{
		ubStrategicCreatures;  //not sure if this wil ever happen.  If so, needs to be handled.
	}

	//successful
	FileClose( hfile );
	return TRUE;

	FAIL_LOAD:
		//The temp file load failed either because of IO problems related to hacking/logic, or
		//various checks failed for hacker validation.  If we reach this point, the "error: exit game"
		//dialog would appear in a non-testversion.
		FileClose( hfile );
		#ifdef JA2TESTVERSION
			AssertMsg( 0, zReason );
		#endif
		return FALSE;
}


static BOOLEAN CountNumberOfElitesRegularsAdminsAndCreaturesFromEnemySoldiersTempFile(UINT8* pubNumElites, UINT8* pubNumRegulars, UINT8* pubNumAdmins, UINT8* pubNumCreatures);


BOOLEAN NewWayOfLoadingEnemySoldiersFromTempFile()
{
	SOLDIERCREATE_STRUCT tempDetailedPlacement;
	INT32 i;
	INT32 slots = 0;
	UINT32 uiTimeStamp;
	HWFILE hfile;
	INT16 sSectorX, sSectorY;
	CHAR8		zMapName[ 128 ];
	#ifdef JA2TESTVERSION
		CHAR8		zReason[256];
	#endif
	INT8 bSectorZ;
	UINT8 ubSectorID;
	UINT8 ubNumElites = 0, ubNumTroops = 0, ubNumAdmins = 0, ubNumCreatures = 0;
	UINT8 ubStrategicElites, ubStrategicTroops, ubStrategicAdmins, ubStrategicCreatures;

	gfRestoringEnemySoldiersFromTempFile = TRUE;

	//STEP ONE:  Set up the temp file to read from.

	GetMapTempFileName( SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, zMapName, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//Count the number of enemies ( elites, regulars, admins and creatures ) that are in the temp file.

	if( gbWorldSectorZ )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		if( !pSector )
		{
		#ifdef JA2TESTVERSION
			sprintf( zReason, "EnemySoldier -- Couldn't find underground sector info for (%d,%d,%d)  KM", gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		#endif

			goto FAIL_LOAD;

		}
	}
	else
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];

		ubNumElites = pSector->ubNumElites;
		ubNumTroops = pSector->ubNumTroops;
		ubNumAdmins = pSector->ubNumAdmins;
		ubNumCreatures = pSector->ubNumCreatures;
	}

	if( !( gTacticalStatus.uiFlags & LOADING_SAVED_GAME ) )
	{
		// Get the number of enemies form the temp file
		CountNumberOfElitesRegularsAdminsAndCreaturesFromEnemySoldiersTempFile( &ubStrategicElites, &ubStrategicTroops, &ubStrategicAdmins, &ubStrategicCreatures );

		//If any of the counts differ from what is in memory
		if( ubStrategicElites != ubNumElites || ubStrategicTroops != ubNumTroops || ubStrategicAdmins != ubNumAdmins || ubStrategicCreatures != ubNumCreatures )
		{
			//remove the file
			RemoveEnemySoldierTempFile( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
			return( TRUE );
		}
	}

	//reset
	ubNumElites = ubNumTroops = ubNumAdmins = ubNumCreatures = 0;


	//Open the file for reading
	hfile = FileOpen(zMapName, FILE_ACCESS_READ);
	if( hfile == 0 )
	{	//Error opening map modification file
		return FALSE;
	}

	//STEP TWO:  determine whether or not we should use this data.
	//because it is the demo, it is automatically used.

	if (!FileRead(hfile, &sSectorY, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading sSectorY.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorY != sSectorY )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- sSectorY mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

//	LoadSoldierInitListLinks( hfile );
	NewWayOfLoadingEnemySoldierInitListLinks( hfile );

	//STEP THREE:  read the data

	if (!FileRead(hfile, &sSectorX, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading sSectorX.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorX != sSectorX )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- sSectorX mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead( hfile, &slots, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading slots.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &uiTimeStamp, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading uiTimeStamp.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &bSectorZ, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading bSectorZ.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gbWorldSectorZ != bSectorZ )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- bSectorZ mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if( GetWorldTotalMin() > uiTimeStamp + 300 )
	{ //the file has aged.  Use the regular method for adding soldiers.
		FileClose( hfile );
		RemoveEnemySoldierTempFile( sSectorX, sSectorY, bSectorZ );
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		return TRUE;
	}

	if( !slots )
	{ //no need to restore the enemy's to the map.  This means we are restoring a saved game.
		gfRestoringEnemySoldiersFromTempFile = FALSE;
		FileClose( hfile );
		return TRUE;
	}


	if( slots < 0 || slots >= 64 )
	{ //bad IO!
		#ifdef JA2TESTVERSION
			sprintf( zReason, "EnemySoldier -- illegal slot value of %d.  KM", slots );
		#endif
		goto FAIL_LOAD;
	}

	//For all the enemy slots (enemy/creature), clear the fPriorityExistance flag.  We will use these flags
	//to determine which slots have been modified as we load the data into the map pristine soldier init list.
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( curr->pBasicPlacement->fPriorityExistance )
		{
			if( curr->pBasicPlacement->bTeam == ENEMY_TEAM || curr->pBasicPlacement->bTeam == CREATURE_TEAM )
			{
				curr->pBasicPlacement->fPriorityExistance = FALSE;
			}
		}
	}

	//get the number of enemies in this sector.
	if( bSectorZ )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( sSectorX, sSectorY, bSectorZ );
		if( !pSector )
		{
		#ifdef JA2TESTVERSION
			sprintf( zReason, "EnemySoldier -- Couldn't find underground sector info for (%d,%d,%d)  KM", sSectorX, sSectorY, bSectorZ );
		#endif
			goto FAIL_LOAD;
		}
		ubStrategicElites		 = pSector->ubNumElites;
		ubStrategicTroops		 = pSector->ubNumTroops;
		ubStrategicAdmins		 = pSector->ubNumAdmins;
		ubStrategicCreatures = pSector->ubNumCreatures;
	}
	else
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
		ubStrategicCreatures = pSector->ubNumCreatures;
		GetNumberOfEnemiesInSector( sSectorX, sSectorY, &ubStrategicAdmins, &ubStrategicTroops, &ubStrategicElites );
	}

	for( i = 0; i < slots; i++ )
	{
		if (!ExtractSoldierCreateFromFile(hfile, &tempDetailedPlacement))
		{
			#ifdef JA2TESTVERSION
				sprintf( zReason, "EnemySoldier -- EOF while reading tempDetailedPlacment %d.  KM", i );
			#endif
			goto FAIL_LOAD;
		}
		FOR_ALL_SOLDIERINITNODES(curr)
		{
			if( !curr->pBasicPlacement->fPriorityExistance )
			{
				if( curr->pBasicPlacement->bTeam == tempDetailedPlacement.bTeam )
				{
					curr->pBasicPlacement->fPriorityExistance = TRUE;
					if( !curr->pDetailedPlacement )
					{ //need to upgrade the placement to detailed placement
						curr->pDetailedPlacement = MALLOC(SOLDIERCREATE_STRUCT);
					}
					//now replace the map pristine placement info with the temp map file version..
					*curr->pDetailedPlacement = tempDetailedPlacement;

					curr->pBasicPlacement->fPriorityExistance	=	TRUE;
					curr->pBasicPlacement->bDirection					= curr->pDetailedPlacement->bDirection;
					curr->pBasicPlacement->bOrders						= curr->pDetailedPlacement->bOrders;
					curr->pBasicPlacement->bAttitude					= curr->pDetailedPlacement->bAttitude;
					curr->pBasicPlacement->bBodyType					= curr->pDetailedPlacement->bBodyType;
					curr->pBasicPlacement->fOnRoof						= curr->pDetailedPlacement->fOnRoof;
					curr->pBasicPlacement->ubSoldierClass			= curr->pDetailedPlacement->ubSoldierClass;
					curr->pBasicPlacement->ubCivilianGroup		= curr->pDetailedPlacement->ubCivilianGroup;
					curr->pBasicPlacement->fHasKeys						= curr->pDetailedPlacement->fHasKeys;
					curr->pBasicPlacement->usStartingGridNo		= curr->pDetailedPlacement->sInsertionGridNo;

					curr->pBasicPlacement->bPatrolCnt			= curr->pDetailedPlacement->bPatrolCnt;
					memcpy( curr->pBasicPlacement->sPatrolGrid, curr->pDetailedPlacement->sPatrolGrid,
						sizeof( INT16 ) * curr->pBasicPlacement->bPatrolCnt );

					UINT16 usCheckSum;
					if (!FileRead(hfile, &usCheckSum, 2))
					{
						#ifdef JA2TESTVERSION
							sprintf( zReason, "EnemySoldier -- EOF while reading usCheckSum %d.  KM", i );
						#endif
						goto FAIL_LOAD;
					}
					//verify the checksum equation (anti-hack) -- see save
					const UINT16 usFileCheckSum = CalcSoldierCreateCheckSum(curr->pDetailedPlacement);
					if( usCheckSum != usFileCheckSum )
					{	//Hacker has modified the stats on the enemy placements.
						#ifdef JA2TESTVERSION
							sprintf( zReason, "EnemySoldier -- checksum for placement %d failed.  KM", i );
						#endif
						goto FAIL_LOAD;
					}


					//Add preserved placements as long as they don't exceed the actual population.
					switch( curr->pBasicPlacement->ubSoldierClass )
					{
						case SOLDIER_CLASS_ELITE:
							ubNumElites++;
							if( ubNumElites <= ubStrategicElites )
							{
//def:								AddPlacementToWorld( curr );
							}
							break;
						case SOLDIER_CLASS_ARMY:
							ubNumTroops++;
							if( ubNumTroops <= ubStrategicTroops )
							{
//def:								AddPlacementToWorld( curr );
							}
							break;
						case SOLDIER_CLASS_ADMINISTRATOR:
							ubNumAdmins++;
							if( ubNumAdmins <= ubStrategicAdmins )
							{
//def:								AddPlacementToWorld( curr );
							}
							break;
						case SOLDIER_CLASS_CREATURE:
							ubNumCreatures++;
							if( ubNumCreatures <= ubStrategicCreatures )
							{
//def:								AddPlacementToWorld( curr );
							}
							break;
					}
					break;
				}
			}
		}
	}

	if (!FileRead(hfile, &ubSectorID, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- EOF while reading ubSectorID.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( ubSectorID != SECTOR( sSectorX, sSectorY ) )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "EnemySoldier -- ubSectorID mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	//now add any extra enemies that have arrived since the temp file was made.
	if( ubStrategicTroops > ubNumTroops || ubStrategicElites > ubNumElites || ubStrategicAdmins > ubNumAdmins )
	{
		ubStrategicTroops = ( ubStrategicTroops > ubNumTroops ) ? ubStrategicTroops - ubNumTroops : 0;
		ubStrategicElites = ( ubStrategicElites > ubNumElites ) ? ubStrategicElites - ubNumElites : 0;
		ubStrategicAdmins = ( ubStrategicAdmins > ubNumAdmins ) ? ubStrategicAdmins - ubNumAdmins : 0;
		AddSoldierInitListEnemyDefenceSoldiers( ubStrategicAdmins, ubStrategicTroops, ubStrategicElites );
	}
	if( ubStrategicCreatures > ubNumCreatures )
	{
		ubStrategicCreatures;  //not sure if this wil ever happen.  If so, needs to be handled.
	}



	//set the number of enemies in the sector
	if( bSectorZ )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		if( !pSector )
		{
		#ifdef JA2TESTVERSION
			sprintf( zReason, "EnemySoldier -- Couldn't find underground sector info for (%d,%d,%d)  KM", sSectorX, sSectorY, bSectorZ );
		#endif
			goto FAIL_LOAD;

/*
			pSector->ubElitesInBattle = ubStrategicElites;
			pSector->ubTroopsInBattle = ubStrategicTroops;
			pSector->ubAdminsInBattle = ubStrategicAdmins;
			pSector->ubCreaturesInBattle = ubStrategicCreatures;
*/
		}
	}
	else
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];
/*
		pSector->ubElitesInBattle = ubStrategicElites;
		pSector->ubTroopsInBattle = ubStrategicTroops;
		pSector->ubAdminsInBattle = ubStrategicAdmins;
		pSector->ubCreaturesInBattle = ubStrategicCreatures;
*/
	}

	//if in battle, what about the ubNumInBAttle


	//successful
	FileClose( hfile );
	return TRUE;

	FAIL_LOAD:
		//The temp file load failed either because of IO problems related to hacking/logic, or
		//various checks failed for hacker validation.  If we reach this point, the "error: exit game"
		//dialog would appear in a non-testversion.
		FileClose( hfile );
		#ifdef JA2TESTVERSION
			AssertMsg( 0, zReason );
		#endif
		return FALSE;
}




BOOLEAN NewWayOfLoadingCiviliansFromTempFile()
{
	SOLDIERCREATE_STRUCT tempDetailedPlacement;
	INT32 i;
	INT32 slots = 0;
	UINT32 uiTimeStamp;
	UINT32 uiTimeSinceLastLoaded;
	HWFILE hfile;
	INT16 sSectorX, sSectorY;
	CHAR8		zMapName[ 128 ];
	#ifdef JA2TESTVERSION
		CHAR8		zReason[256];
	#endif
	INT8 bSectorZ;
	UINT8 ubSectorID;
//	UINT8 ubStrategicElites, ubStrategicTroops, ubStrategicAdmins, ubStrategicCreatures;

	gfRestoringCiviliansFromTempFile = TRUE;

	//STEP ONE:  Set up the temp file to read from.

	GetMapTempFileName( SF_CIV_PRESERVED_TEMP_FILE_EXISTS, zMapName, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//Open the file for reading
	hfile = FileOpen(zMapName, FILE_ACCESS_READ);
	if( hfile == 0 )
	{	//Error opening map modification file
		return FALSE;
	}

	//STEP TWO:  determine whether or not we should use this data.
	//because it is the demo, it is automatically used.

	if (!FileRead(hfile, &sSectorY, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- EOF while reading sSectorY.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorY != sSectorY )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- sSectorY mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	//LoadSoldierInitListLinks( hfile );
	NewWayOfLoadingCivilianInitListLinks( hfile );


	//STEP THREE:  read the data
	if (!FileRead(hfile, &sSectorX, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- EOF while reading sSectorX.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorX != sSectorX )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- sSectorX mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &slots, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- EOF while reading slots.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &uiTimeStamp, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- EOF while reading uiTimeStamp.  KM");
#endif
		goto FAIL_LOAD;
	}

	uiTimeSinceLastLoaded = GetWorldTotalMin() - uiTimeStamp;

	if (!FileRead(hfile, &bSectorZ, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- EOF while reading bSectorZ.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gbWorldSectorZ != bSectorZ )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- bSectorZ mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if( !slots )
	{
		//no need to restore the enemy's to the map.  This means we are restoring a saved game.
		gfRestoringCiviliansFromTempFile = FALSE;
		FileClose( hfile );
		return TRUE;
	}
	if( slots < 0 || slots >= 64 )
	{
		//bad IO!
		#ifdef JA2TESTVERSION
			sprintf( zReason, "Civilian -- illegal slot value of %d.  KM", slots );
		#endif
		goto FAIL_LOAD;
	}

	//For all the enemy slots (enemy/creature), clear the fPriorityExistance flag.  We will use these flags
	//to determine which slots have been modified as we load the data into the map pristine soldier init list.
	CFOR_ALL_SOLDIERINITNODES(curr)
	{
		if( curr->pBasicPlacement->fPriorityExistance )
		{
			if( curr->pBasicPlacement->bTeam == CIV_TEAM )
			{
				curr->pBasicPlacement->fPriorityExistance = FALSE;
			}
		}
	}

	for( i = 0; i < slots; i++ )
	{
		if (!ExtractSoldierCreateFromFile(hfile, &tempDetailedPlacement))
		{
			#ifdef JA2TESTVERSION
				sprintf( zReason, "Civilian -- EOF while reading tempDetailedPlacment %d.  KM", i );
			#endif
			goto FAIL_LOAD;
		}
		FOR_ALL_SOLDIERINITNODES(curr)
		{
			if( !curr->pBasicPlacement->fPriorityExistance )
			{
				if( !curr->pDetailedPlacement || curr->pDetailedPlacement && curr->pDetailedPlacement->ubProfile == NO_PROFILE )
				{
					if( curr->pBasicPlacement->bTeam == tempDetailedPlacement.bTeam )
					{
						curr->pBasicPlacement->fPriorityExistance = TRUE;

						if( !curr->pDetailedPlacement )
						{
							//need to upgrade the placement to detailed placement
							curr->pDetailedPlacement = MALLOC(SOLDIERCREATE_STRUCT);
						}
						//now replace the map pristine placement info with the temp map file version..
						*curr->pDetailedPlacement = tempDetailedPlacement;

						curr->pBasicPlacement->fPriorityExistance	=	TRUE;
						curr->pBasicPlacement->bDirection					= curr->pDetailedPlacement->bDirection;
						curr->pBasicPlacement->bOrders						= curr->pDetailedPlacement->bOrders;
						curr->pBasicPlacement->bAttitude					= curr->pDetailedPlacement->bAttitude;
						curr->pBasicPlacement->bBodyType					= curr->pDetailedPlacement->bBodyType;
						curr->pBasicPlacement->fOnRoof						= curr->pDetailedPlacement->fOnRoof;
						curr->pBasicPlacement->ubSoldierClass			= curr->pDetailedPlacement->ubSoldierClass;
						curr->pBasicPlacement->ubCivilianGroup		= curr->pDetailedPlacement->ubCivilianGroup;
						curr->pBasicPlacement->fHasKeys						= curr->pDetailedPlacement->fHasKeys;
						curr->pBasicPlacement->usStartingGridNo		= curr->pDetailedPlacement->sInsertionGridNo;

						curr->pBasicPlacement->bPatrolCnt			= curr->pDetailedPlacement->bPatrolCnt;
						memcpy( curr->pBasicPlacement->sPatrolGrid, curr->pDetailedPlacement->sPatrolGrid,
							sizeof( INT16 ) * curr->pBasicPlacement->bPatrolCnt );

						UINT16 usCheckSum;
						if (!FileRead(hfile, &usCheckSum, 2))
						{
							#ifdef JA2TESTVERSION
								sprintf( zReason, "Civilian -- EOF while reading usCheckSum %d.  KM", i );
							#endif
							goto FAIL_LOAD;
						}
						//verify the checksum equation (anti-hack) -- see save
						const UINT16 usFileCheckSum = CalcSoldierCreateCheckSum(curr->pDetailedPlacement);
						if( usCheckSum != usFileCheckSum )
						{
							//Hacker has modified the stats on the enemy placements.
							#ifdef JA2TESTVERSION
								sprintf( zReason, "Civilian -- checksum for placement %d failed.  KM", i );
							#endif
							goto FAIL_LOAD;
						}

						if( curr->pDetailedPlacement->bLife < curr->pDetailedPlacement->bLifeMax )
						{ //Add 4 life for every hour that passes.
							INT32 iNewLife;
							iNewLife = curr->pDetailedPlacement->bLife + uiTimeSinceLastLoaded / 15;
							iNewLife = min( curr->pDetailedPlacement->bLifeMax, iNewLife );
							curr->pDetailedPlacement->bLife = (INT8)iNewLife;
						}

						if( curr->pBasicPlacement->bTeam == CIV_TEAM )
						{
//def:				AddPlacementToWorld( curr );
							break;
						}

					}
				}
			}
		}
	}

	// now remove any non-priority placement which matches the conditions!
	FOR_ALL_SOLDIERINITNODES_SAFE(curr)
	{
		if( !curr->pBasicPlacement->fPriorityExistance )
		{
			if( !curr->pDetailedPlacement || curr->pDetailedPlacement && curr->pDetailedPlacement->ubProfile == NO_PROFILE )
			{
				if( curr->pBasicPlacement->bTeam == tempDetailedPlacement.bTeam )
				{
					RemoveSoldierNodeFromInitList( curr );
				}
			}
		}
	}

	if (!FileRead(hfile, &ubSectorID, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- EOF while reading ubSectorID.  KM");
#endif
		goto FAIL_LOAD;
	}

	/*
	if( ubSectorID != SECTOR( sSectorX, sSectorY ) )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Civilian -- ubSectorID mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}
	*/

	//successful
	FileClose( hfile );
	return TRUE;

	FAIL_LOAD:
		//The temp file load failed either because of IO problems related to hacking/logic, or
		//various checks failed for hacker validation.  If we reach this point, the "error: exit game"
		//dialog would appear in a non-testversion.
		FileClose( hfile );
		#ifdef JA2TESTVERSION
			AssertMsg( 0, zReason );
		#endif
		return FALSE;
}


//If we are saving a game and we are in the sector, we will need to preserve the links between the
//soldiers and the soldier init list.  Otherwise, the temp file will be deleted.
BOOLEAN NewWayOfSavingEnemyAndCivliansToTempFile( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, BOOLEAN fEnemy, BOOLEAN fValidateOnly )
{
	INT32 i;
	INT32 slots = 0;
	UINT32 uiTimeStamp;
	CHAR8		zMapName[ 128 ];
	UINT8 ubSectorID;

	UINT8	ubStartID=0;
	UINT8	ubEndID = 0;



	//if we are saving the enemy info to the enemy temp file
	if( fEnemy )
	{
		ubStartID = ENEMY_TEAM;
		ubEndID = CREATURE_TEAM;
	}

	//else its the civilian team
	else
	{
		ubStartID = CIV_TEAM;
		ubEndID = CIV_TEAM;
	}



	//STEP ONE:  Prep the soldiers for saving...

	//modify the map's soldier init list to reflect the changes to the member's still alive...
	for( i = gTacticalStatus.Team[ ubStartID ].bFirstID; i <= gTacticalStatus.Team[ ubEndID ].bLastID; i++ )
	{
		const SOLDIERTYPE* const pSoldier = GetMan(i);

		//make sure the person is active, alive, in the sector, and is not a profiled person
		if( pSoldier->bActive /*&& pSoldier->bInSector*/ && pSoldier->bLife && pSoldier->ubProfile == NO_PROFILE )
		{ //soldier is valid, so find the matching soldier init list entry for modification.
			SOLDIERINITNODE* const curr = FindSoldierInitNodeBySoldier(pSoldier);
			if (curr && pSoldier->ubProfile == NO_PROFILE)
			{ //found a match.

				if( !fValidateOnly )
				{
					if( !(gTacticalStatus.uiFlags & LOADING_SAVED_GAME) )
					{
						if( !curr->pDetailedPlacement )
						{ //need to upgrade the placement to detailed placement
							curr->pBasicPlacement->fDetailedPlacement = TRUE;
							curr->pDetailedPlacement = MALLOCZ(SOLDIERCREATE_STRUCT);
						}

						//Copy over the data of the soldier.
						curr->pDetailedPlacement->ubProfile							= NO_PROFILE;
						curr->pDetailedPlacement->bLife									= pSoldier->bLife;
						curr->pDetailedPlacement->bLifeMax  						= pSoldier->bLifeMax;
						curr->pDetailedPlacement->bAgility							= pSoldier->bAgility;
						curr->pDetailedPlacement->bDexterity						= pSoldier->bDexterity;
						curr->pDetailedPlacement->bExpLevel							= pSoldier->bExpLevel;
						curr->pDetailedPlacement->bMarksmanship					= pSoldier->bMarksmanship;
						curr->pDetailedPlacement->bMedical							= pSoldier->bMedical;
						curr->pDetailedPlacement->bMechanical						= pSoldier->bMechanical;
						curr->pDetailedPlacement->bExplosive						= pSoldier->bExplosive;
						curr->pDetailedPlacement->bLeadership						= pSoldier->bLeadership;
						curr->pDetailedPlacement->bStrength							= pSoldier->bStrength;
						curr->pDetailedPlacement->bWisdom								= pSoldier->bWisdom;
						curr->pDetailedPlacement->bAttitude							= pSoldier->bAttitude;
						curr->pDetailedPlacement->bOrders								= pSoldier->bOrders;
						curr->pDetailedPlacement->bMorale								= pSoldier->bMorale;
						curr->pDetailedPlacement->bAIMorale							= pSoldier->bAIMorale;
						curr->pDetailedPlacement->bBodyType							= pSoldier->ubBodyType;
						curr->pDetailedPlacement->ubCivilianGroup				= pSoldier->ubCivilianGroup;
						curr->pDetailedPlacement->ubScheduleID					= pSoldier->ubScheduleID;
						curr->pDetailedPlacement->fHasKeys							= pSoldier->bHasKeys;
						curr->pDetailedPlacement->sSectorX							= pSoldier->sSectorX;
						curr->pDetailedPlacement->sSectorY							= pSoldier->sSectorY;
						curr->pDetailedPlacement->bSectorZ							= pSoldier->bSectorZ;
						curr->pDetailedPlacement->ubSoldierClass				= pSoldier->ubSoldierClass;
						curr->pDetailedPlacement->bTeam									= pSoldier->bTeam;
						curr->pDetailedPlacement->bDirection						= pSoldier->bDirection;

						//we don't want the player to think that all the enemies start in the exact position when we
						//left the map, so randomize the start locations either current position or original position.
						if( PreRandom( 2 ) )
						{ //use current position
							curr->pDetailedPlacement->fOnRoof								= pSoldier->bLevel;
							curr->pDetailedPlacement->sInsertionGridNo			= pSoldier->sGridNo;
						}
						else
						{ //use original position
							curr->pDetailedPlacement->fOnRoof								= curr->pBasicPlacement->fOnRoof;
							curr->pDetailedPlacement->sInsertionGridNo			= curr->pBasicPlacement->usStartingGridNo;
						}

						wcslcpy(curr->pDetailedPlacement->name, pSoldier->name, lengthof(curr->pDetailedPlacement->name));

						//Copy patrol points
						curr->pDetailedPlacement->bPatrolCnt						= pSoldier->bPatrolCnt;
						memcpy( curr->pDetailedPlacement->sPatrolGrid, pSoldier->usPatrolGrid, sizeof( INT16 ) * MAXPATROLGRIDS );

						//copy colors for soldier based on the body type.
						strcpy(curr->pDetailedPlacement->HeadPal,  pSoldier->HeadPal);
						strcpy(curr->pDetailedPlacement->VestPal,  pSoldier->VestPal);
						strcpy(curr->pDetailedPlacement->SkinPal,  pSoldier->SkinPal);
						strcpy(curr->pDetailedPlacement->PantsPal, pSoldier->PantsPal);

						//copy soldier's inventory
						memcpy( curr->pDetailedPlacement->Inv, pSoldier->inv, sizeof( OBJECTTYPE ) * NUM_INV_SLOTS );
					}
				}

				//DONE, now increment the counter, so we know how many there are.
				slots++;
			}
		}
	}

	if( !slots )
	{
		if( fEnemy )
		{
			//No need to save anything, so return successfully
			RemoveEnemySoldierTempFile( sSectorX, sSectorY, bSectorZ );
			return TRUE;
		}
		else
		{
			//No need to save anything, so return successfully
			RemoveCivilianTempFile( sSectorX, sSectorY, bSectorZ );
			return( TRUE );
		}

	}

	if( fValidateOnly )
	{
		return( TRUE );
	}

	//STEP TWO:  Set up the temp file to write to.

	if( fEnemy )
	{
		GetMapTempFileName( SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ );
	}
	else
	{
		GetMapTempFileName( SF_CIV_PRESERVED_TEMP_FILE_EXISTS, zMapName, sSectorX, sSectorY, bSectorZ );
	}

	const HWFILE hfile = FileOpen(zMapName, FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS);
	if( hfile == 0 )
	{	//Error opening map modification file
		return FALSE;
	}

	if (!FileWrite(hfile, &sSectorY, 2)) goto FAIL_SAVE;

	//STEP THREE:  Save the data

	//this works for both civs and enemies
	SaveSoldierInitListLinks( hfile );

	if (!FileWrite(hfile, &sSectorX, 2)) goto FAIL_SAVE;

	//This check may appear confusing.  It is intended to abort if the player is saving the game.  It is only
	//supposed to preserve the links to the placement list, so when we finally do leave the level with enemies remaining,
	//we will need the links that are only added when the map is loaded, and are normally lost when restoring a save.
	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
	{
		slots = 0;
	}

	if (!FileWrite(hfile, &slots, 4)) goto FAIL_SAVE;

	uiTimeStamp = GetWorldTotalMin();
	if (!FileWrite(hfile, &uiTimeStamp, 4)) goto FAIL_SAVE;

	if (!FileWrite(hfile, &bSectorZ, 1)) goto FAIL_SAVE;

	if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
	{
		//if we are saving the game, we don't need to preserve the soldier information, just
		//preserve the links to the placement list.
		slots = 0;
		FileClose( hfile );

		if( fEnemy )
		{
			SetSectorFlag( sSectorX, sSectorY, bSectorZ, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS );
		}
		else
		{
			SetSectorFlag( sSectorX, sSectorY, bSectorZ, SF_CIV_PRESERVED_TEMP_FILE_EXISTS );
		}
		return TRUE;
	}

	for( i = gTacticalStatus.Team[ ubStartID ].bFirstID; i <= gTacticalStatus.Team[ ubEndID ].bLastID; i++ )
	{
		const SOLDIERTYPE* const pSoldier = GetMan(i);
		// CJC: note that bInSector is not required; the civ could be offmap!
		if( pSoldier->bActive /*&& pSoldier->bInSector*/ && pSoldier->bLife )
		{
			//soldier is valid, so find the matching soldier init list entry for modification.
			const SOLDIERINITNODE* const curr = FindSoldierInitNodeBySoldier(pSoldier);
			if (curr && pSoldier->ubProfile == NO_PROFILE)
			{
				//found a match.
				if (!InjectSoldierCreateIntoFile(hfile, curr->pDetailedPlacement)) goto FAIL_SAVE;
				//insert a checksum equation (anti-hack)
				const UINT16 usCheckSum = CalcSoldierCreateCheckSum(curr->pDetailedPlacement);
				if (!FileWrite(hfile, &usCheckSum, 2)) goto FAIL_SAVE;
			}
		}
	}

	ubSectorID = SECTOR( sSectorX, sSectorY );
	if (!FileWrite(hfile, &ubSectorID, 1)) goto FAIL_SAVE;

	FileClose( hfile );

	if( fEnemy )
	{
		SetSectorFlag( sSectorX, sSectorY, bSectorZ, SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS );
	}
	else
	{
		SetSectorFlag( sSectorX, sSectorY, bSectorZ, SF_CIV_PRESERVED_TEMP_FILE_EXISTS );
	}

	return TRUE;

	FAIL_SAVE:
		FileClose( hfile );
		return FALSE;
}


static BOOLEAN CountNumberOfElitesRegularsAdminsAndCreaturesFromEnemySoldiersTempFile(UINT8* pubNumElites, UINT8* pubNumRegulars, UINT8* pubNumAdmins, UINT8* pubNumCreatures)
{
//	SOLDIERINITNODE *curr;
	SOLDIERCREATE_STRUCT tempDetailedPlacement;
	INT32 i;
	INT32 slots = 0;
	UINT32 uiTimeStamp;
	HWFILE hfile;
	INT16 sSectorX, sSectorY;
	UINT16 usCheckSum;
	CHAR8		zMapName[ 128 ];
	#ifdef JA2TESTVERSION
		CHAR8		zReason[256];
	#endif
	INT8 bSectorZ;
	UINT8 ubSectorID;
//	UINT8 ubNumElites = 0, ubNumTroops = 0, ubNumAdmins = 0, ubNumCreatures = 0;
//	UINT8 ubStrategicElites, ubStrategicTroops, ubStrategicAdmins, ubStrategicCreatures;



	//make sure the variables are initialized
	*pubNumElites = 0;
	*pubNumRegulars = 0;
	*pubNumAdmins = 0;
	*pubNumCreatures = 0;

	//STEP ONE:  Set up the temp file to read from.

	GetMapTempFileName( SF_ENEMY_PRESERVED_TEMP_FILE_EXISTS, zMapName, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );

	//Open the file for reading
	hfile = FileOpen(zMapName, FILE_ACCESS_READ);
	if( hfile == 0 )
	{	//Error opening map modification file
		return FALSE;
	}

	//STEP TWO:  determine whether or not we should use this data.
	//because it is the demo, it is automatically used.

	if (!FileRead(hfile, &sSectorY, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- EOF while reading sSectorY.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorY != sSectorY )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- sSectorY mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

//	LoadSoldierInitListLinks( hfile );
	NewWayOfLoadingEnemySoldierInitListLinks(hfile);

	//STEP THREE:  read the data

	if (!FileRead(hfile, &sSectorX, 2))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- EOF while reading sSectorX.  KM");
#endif
		goto FAIL_LOAD;
	}
	if( gWorldSectorX != sSectorX )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- sSectorX mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &slots, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- EOF while reading slots.  KM");
#endif
		goto FAIL_LOAD;
	}

	if (!FileRead(hfile, &uiTimeStamp, 4))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- EOF while reading uiTimeStamp.  KM");
#endif
		goto FAIL_LOAD;
	}


	if (!FileRead(hfile, &bSectorZ, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- EOF while reading bSectorZ.  KM");
#endif
		goto FAIL_LOAD;
	}

	if( gbWorldSectorZ != bSectorZ )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- bSectorZ mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	if( !slots )
	{
		//no need to restore the enemy's to the map.  This means we are restoring a saved game.
		FileClose( hfile );
		return TRUE;
	}


	if( slots < 0 || slots >= 64 )
	{ //bad IO!
		#ifdef JA2TESTVERSION
			sprintf( zReason, "Check EnemySoldier -- illegal slot value of %d.  KM", slots );
		#endif
		goto FAIL_LOAD;
	}

/*
	//get the number of enemies in this sector.
	if( bSectorZ )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( sSectorX, sSectorY, bSectorZ );
		if( !pSector )
		{
		#ifdef JA2TESTVERSION
			sprintf( zReason, "EnemySoldier -- Couldn't find underground sector info for (%d,%d,%d)  KM", sSectorX, sSectorY, bSectorZ );
		#endif
			goto FAIL_LOAD;
		}
		ubStrategicElites		 = pSector->ubNumElites;
		ubStrategicTroops		 = pSector->ubNumTroops;
		ubStrategicAdmins		 = pSector->ubNumAdmins;
		ubStrategicCreatures = pSector->ubNumCreatures;
	}
	else
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
		ubStrategicCreatures = pSector->ubNumCreatures;
		GetNumberOfEnemiesInSector( sSectorX, sSectorY, &ubStrategicAdmins, &ubStrategicTroops, &ubStrategicElites );
	}
*/

	for( i = 0; i < slots; i++ )
	{
		if (!ExtractSoldierCreateFromFile(hfile, &tempDetailedPlacement))
		{
			#ifdef JA2TESTVERSION
				sprintf( zReason, "Check EnemySoldier -- EOF while reading tempDetailedPlacment %d.  KM", i );
			#endif
			goto FAIL_LOAD;
		}

		//increment the current type of soldier
		switch( tempDetailedPlacement.ubSoldierClass )
		{
			case SOLDIER_CLASS_ELITE:
				(*pubNumElites)++;
				break;
			case SOLDIER_CLASS_ARMY:
				(*pubNumRegulars)++;
				break;
			case SOLDIER_CLASS_ADMINISTRATOR:
				(*pubNumAdmins)++;
				break;
			case SOLDIER_CLASS_CREATURE:
				(*pubNumCreatures)++;
				break;
		}


		if (!FileRead(hfile, &usCheckSum, 2))
		{
			#ifdef JA2TESTVERSION
				sprintf( zReason, "Check EnemySoldier -- EOF while reading usCheckSum %d.  KM", i );
			#endif
			goto FAIL_LOAD;
		}
/*
		while( curr )
		{
			if( !curr->pBasicPlacement->fPriorityExistance )
			{
				if( curr->pBasicPlacement->bTeam == tempDetailedPlacement.bTeam )
				{
					curr->pBasicPlacement->fPriorityExistance = TRUE;
					if( !curr->pDetailedPlacement )
					{ //need to upgrade the placement to detailed placement
						curr->pDetailedPlacement = MALLOC(SOLDIERCREATE_STRUCT);
					}
					//now replace the map pristine placement info with the temp map file version..
					*curr->pDetailedPlacement = tempDetailedPlacement;

					curr->pBasicPlacement->fPriorityExistance	=	TRUE;
					curr->pBasicPlacement->bDirection					= curr->pDetailedPlacement->bDirection;
					curr->pBasicPlacement->bOrders						= curr->pDetailedPlacement->bOrders;
					curr->pBasicPlacement->bAttitude					= curr->pDetailedPlacement->bAttitude;
					curr->pBasicPlacement->bBodyType					= curr->pDetailedPlacement->bBodyType;
					curr->pBasicPlacement->fOnRoof						= curr->pDetailedPlacement->fOnRoof;
					curr->pBasicPlacement->ubSoldierClass			= curr->pDetailedPlacement->ubSoldierClass;
					curr->pBasicPlacement->ubCivilianGroup		= curr->pDetailedPlacement->ubCivilianGroup;
					curr->pBasicPlacement->fHasKeys						= curr->pDetailedPlacement->fHasKeys;
					curr->pBasicPlacement->usStartingGridNo		= curr->pDetailedPlacement->sInsertionGridNo;

					curr->pBasicPlacement->bPatrolCnt			= curr->pDetailedPlacement->bPatrolCnt;
					memcpy( curr->pBasicPlacement->sPatrolGrid, curr->pDetailedPlacement->sPatrolGrid,
						sizeof( INT16 ) * curr->pBasicPlacement->bPatrolCnt );

					if (!FileRead(hfile, &usCheckSum, 2))
					{
						#ifdef JA2TESTVERSION
							sprintf( zReason, "EnemySoldier -- EOF while reading usCheckSum %d.  KM", i );
						#endif
						goto FAIL_LOAD;
					}
					//verify the checksum equation (anti-hack) -- see save
					const UINT16 usFileCheckSum = CalcSoldierCreateCheckSum(curr->pDetailedPlacement);
					if( usCheckSum != usFileCheckSum )
					{	//Hacker has modified the stats on the enemy placements.
						#ifdef JA2TESTVERSION
							sprintf( zReason, "EnemySoldier -- checksum for placement %d failed.  KM", i );
						#endif
						goto FAIL_LOAD;
					}

					//Add preserved placements as long as they don't exceed the actual population.
					switch( curr->pBasicPlacement->ubSoldierClass )
					{
						case SOLDIER_CLASS_ELITE:
							ubNumElites++;
							if( ubNumElites <= ubStrategicElites )
							{
								AddPlacementToWorld( curr );
							}
							break;
						case SOLDIER_CLASS_ARMY:
							ubNumTroops++;
							if( ubNumTroops <= ubStrategicTroops )
							{
								AddPlacementToWorld( curr );
							}
							break;
						case SOLDIER_CLASS_ADMINISTRATOR:
							ubNumAdmins++;
							if( ubNumAdmins <= ubStrategicAdmins )
							{
								AddPlacementToWorld( curr );
							}
							break;
						case SOLDIER_CLASS_CREATURE:
							ubNumCreatures++;
							if( ubNumCreatures <= ubStrategicCreatures )
							{
								AddPlacementToWorld( curr );
							}
							break;
					}
					break;
				}
			}
			curr = curr->next;
		}
*/
	}

	if (!FileRead(hfile, &ubSectorID, 1))
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- EOF while reading ubSectorID.  KM");
#endif
		goto FAIL_LOAD;
	}

	if( ubSectorID != SECTOR( sSectorX, sSectorY ) )
	{
#ifdef JA2TESTVERSION
		strcpy(zReason, "Check EnemySoldier -- ubSectorID mismatch.  KM");
#endif
		goto FAIL_LOAD;
	}

	//successful
	FileClose( hfile );
	return TRUE;

	FAIL_LOAD:
		//The temp file load failed either because of IO problems related to hacking/logic, or
		//various checks failed for hacker validation.  If we reach this point, the "error: exit game"
		//dialog would appear in a non-testversion.
		FileClose( hfile );
		#ifdef JA2TESTVERSION
			AssertMsg( 0, zReason );
		#endif
		return FALSE;
}
