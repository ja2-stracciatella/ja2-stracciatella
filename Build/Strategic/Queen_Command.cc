#include <stdexcept>

#include "Creature_Spreading.h"
#include "Font_Control.h"
#include "LoadSaveUndergroundSectorInfo.h"
#include "Queen_Command.h"
#include "Strategic_Event_Handler.h"
#include "Overhead_Types.h"
#include "StrategicMap.h"
#include "Soldier_Init_List.h"
#include "Debug.h"
#include "Random.h"
#include "Strategic_Movement.h"
#include "Overhead.h"
#include "Strategic_Pathing.h"
#include "Strategic.h"
#include "Message.h"
#include "Map_Edgepoints.h"
#include "PreBattle_Interface.h"
#include "Strategic_Status.h"
#include "Squads.h"
#include "Assignments.h"
#include "Items.h"
#include "Tactical_Save.h"
#include "Soldier_Ani.h"
#include "Strategic_AI.h"
#include "Strategic_Town_Loyalty.h"
#include "Soldier_Profile.h"
#include "Quests.h"
#include "Auto_Resolve.h"
#include "Animation_Data.h"
#include "Game_Event_Hook.h"
#include "Game_Clock.h"
#include "RenderWorld.h"
#include "Dialogue_Control.h"
#include "Campaign_Init.h"
#include "Meanwhile.h"
#include "Soldier_Macros.h"
#include "Morale.h"
#include "MemMan.h"
#include "FileMan.h"
#include "Vehicles.h"

#ifdef JA2BETAVERSION
#	include "JAScreens.h"
#	include "ScreenIDs.h"
#	include "MessageBoxScreen.h"

	extern BOOLEAN gfClearCreatureQuest;
#endif

//The sector information required for the strategic AI.  Contains the number of enemy troops,
//as well as intentions, etc.
SECTORINFO SectorInfo[256];
UNDERGROUND_SECTORINFO *gpUndergroundSectorInfoHead = NULL;
extern UNDERGROUND_SECTORINFO* gpUndergroundSectorInfoTail;
BOOLEAN gfPendingEnemies = FALSE;

extern GARRISON_GROUP *gGarrisonGroup;

#ifdef JA2TESTVERSION
extern BOOLEAN gfOverrideSector;
#endif

INT16 gsInterrogationGridNo[3] = { 7756, 7757, 7758 };


static void ValidateEnemiesHaveWeapons(void)
{
#ifdef JA2BETAVERSION
	INT32 iNumInvalid = 0;
	CFOR_ALL_IN_TEAM(s, ENEMY_TEAM)
	{
		if (!s->bInSector) continue;
		if (!s->inv[HANDPOS].usItem) iNumInvalid++;
	}

	// do message box and return
	if (iNumInvalid)
	{
		wchar_t str[100];
		swprintf(str, lengthof(str), L"%d enemies have been added without any weapons!  KM:0.  Please note sector.", iNumInvalid);
		DoMessageBox(MSG_BOX_BASIC_STYLE, str, GAME_SCREEN, MSG_BOX_FLAG_OK, NULL, NULL);
	}
#endif
}

//Counts enemies and crepitus, but not bloodcats.
UINT8 NumHostilesInSector( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	UINT8 ubNumHostiles = 0;

	Assert( sSectorX >= 1 && sSectorX <= 16 );
	Assert( sSectorY >= 1 && sSectorY <= 16 );
	Assert( sSectorZ >= 0 && sSectorZ <= 3 );

	if( sSectorZ )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( sSectorX, sSectorY, (UINT8)sSectorZ );
		if( pSector )
		{
			ubNumHostiles = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumCreatures);
		}
	}
	else
	{
		SECTORINFO *pSector;

		//Count stationary hostiles
		pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
		ubNumHostiles = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumCreatures);

		//Count mobile enemies
		CFOR_ALL_ENEMY_GROUPS(pGroup)
		{
			if (!pGroup->fVehicle             &&
					pGroup->ubSectorX == sSectorX &&
					pGroup->ubSectorY == sSectorY)
			{
				ubNumHostiles += pGroup->ubGroupSize;
			}
		}
	}

	return ubNumHostiles;
}

UINT8 NumEnemiesInAnySector( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ )
{
	UINT8 ubNumEnemies = 0;

	Assert( sSectorX >= 1 && sSectorX <= 16 );
	Assert( sSectorY >= 1 && sSectorY <= 16 );
	Assert( sSectorZ >= 0 && sSectorZ <= 3 );

	if( sSectorZ )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( sSectorX, sSectorY, (UINT8)sSectorZ );
		if( pSector )
		{
			ubNumEnemies = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
		}
	}
	else
	{
		SECTORINFO *pSector;

		//Count stationary enemies
		pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
		ubNumEnemies = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);

		//Count mobile enemies
		CFOR_ALL_ENEMY_GROUPS(pGroup)
		{
			if (!pGroup->fVehicle             &&
					pGroup->ubSectorX == sSectorX &&
					pGroup->ubSectorY == sSectorY)
			{
				ubNumEnemies += pGroup->ubGroupSize;
			}
		}
	}

	return ubNumEnemies;
}

UINT8 NumEnemiesInSector( INT16 sSectorX, INT16 sSectorY )
{
	SECTORINFO *pSector;
	UINT8 ubNumTroops;
	Assert( sSectorX >= 1 && sSectorX <= 16 );
	Assert( sSectorY >= 1 && sSectorY <= 16 );
	pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
	ubNumTroops = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);

	CFOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (!pGroup->fVehicle             &&
				pGroup->ubSectorX == sSectorX &&
				pGroup->ubSectorY == sSectorY)
		{
			ubNumTroops += pGroup->ubGroupSize;
		}
	}
	return ubNumTroops;
}

UINT8 NumStationaryEnemiesInSector( INT16 sSectorX, INT16 sSectorY )
{
	SECTORINFO *pSector;
	Assert( sSectorX >= 1 && sSectorX <= 16 );
	Assert( sSectorY >= 1 && sSectorY <= 16 );
	pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];

	if( pSector->ubGarrisonID == NO_GARRISON )
	{ //If no garrison, no stationary.
		return( 0 );
	}

	// don't count roadblocks as stationary garrison, we want to see how many enemies are in them, not question marks
	if ( gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition == ROADBLOCK )
	{
		// pretend they're not stationary
		return( 0 );
	}

	return (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
}

UINT8 NumMobileEnemiesInSector( INT16 sSectorX, INT16 sSectorY )
{
	SECTORINFO *pSector;
	UINT8 ubNumTroops;
	Assert( sSectorX >= 1 && sSectorX <= 16 );
	Assert( sSectorY >= 1 && sSectorY <= 16 );

	ubNumTroops = 0;
	CFOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (!pGroup->fVehicle             &&
				pGroup->ubSectorX == sSectorX &&
				pGroup->ubSectorY == sSectorY)
		{
			ubNumTroops += pGroup->ubGroupSize;
		}
	}

	pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
	if( pSector->ubGarrisonID == ROADBLOCK )
	{ //consider these troops as mobile troops even though they are in a garrison
		ubNumTroops += (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
	}

	return ubNumTroops;
}


static void GetNumberOfMobileEnemiesInSector(INT16 sSectorX, INT16 sSectorY, UINT8* pubNumAdmins, UINT8* pubNumTroops, UINT8* pubNumElites)
{
	SECTORINFO *pSector;
	Assert( sSectorX >= 1 && sSectorX <= 16 );
	Assert( sSectorY >= 1 && sSectorY <= 16 );

	//Now count the number of mobile groups in the sector.
	*pubNumTroops = *pubNumElites = *pubNumAdmins = 0;
	CFOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (!pGroup->fVehicle             &&
				pGroup->ubSectorX == sSectorX &&
				pGroup->ubSectorY == sSectorY)
		{
			*pubNumTroops += pGroup->pEnemyGroup->ubNumTroops;
			*pubNumElites += pGroup->pEnemyGroup->ubNumElites;
			*pubNumAdmins += pGroup->pEnemyGroup->ubNumAdmins;
		}
	}

	pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];
	if( pSector->ubGarrisonID == ROADBLOCK )
	{ //consider these troops as mobile troops even though they are in a garrison
		*pubNumAdmins += pSector->ubNumAdmins;
		*pubNumTroops += pSector->ubNumTroops;
		*pubNumElites += pSector->ubNumElites;
	}

}


static void GetNumberOfStationaryEnemiesInSector(INT16 sSectorX, INT16 sSectorY, UINT8* pubNumAdmins, UINT8* pubNumTroops, UINT8* pubNumElites)
{
	SECTORINFO *pSector;
	Assert( sSectorX >= 1 && sSectorX <= 16 );
	Assert( sSectorY >= 1 && sSectorY <= 16 );
	pSector = &SectorInfo[ SECTOR( sSectorX, sSectorY ) ];

	//grab the number of each type in the stationary sector
	*pubNumAdmins = pSector->ubNumAdmins;
	*pubNumTroops = pSector->ubNumTroops;
	*pubNumElites = pSector->ubNumElites;
}

void GetNumberOfEnemiesInSector( INT16 sSectorX, INT16 sSectorY, UINT8 *pubNumAdmins, UINT8 *pubNumTroops, UINT8 *pubNumElites )
{
	UINT8 ubNumAdmins, ubNumTroops, ubNumElites;

	GetNumberOfStationaryEnemiesInSector( sSectorX, sSectorY, pubNumAdmins, pubNumTroops, pubNumElites );

	GetNumberOfMobileEnemiesInSector( sSectorX, sSectorY, &ubNumAdmins, &ubNumTroops, &ubNumElites );

	*pubNumAdmins += ubNumAdmins;
	*pubNumTroops += ubNumTroops;
	*pubNumElites += ubNumElites;
}


static BOOLEAN IsAnyOfTeamOKInSector(const INT8 team)
{
	CFOR_ALL_IN_TEAM(s, team)
	{
		if (s->bInSector && s->bLife >= OKLIFE) return TRUE;
	}
	return FALSE;
}


void EndTacticalBattleForEnemy()
{
	//Clear enemies in battle for all stationary groups in the sector.
	if( gbWorldSectorZ > 0 )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
		pSector->ubAdminsInBattle = 0;
		pSector->ubTroopsInBattle = 0;
		pSector->ubElitesInBattle = 0;
	}
	else if( !gbWorldSectorZ )
	{
		SECTORINFO *pSector;
		pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];
		//grab the number of each type in the stationary sector
		pSector->ubAdminsInBattle = 0;
		pSector->ubTroopsInBattle = 0;
		pSector->ubElitesInBattle = 0;
		pSector->ubNumCreatures = 0;
		pSector->ubCreaturesInBattle = 0;
	}
	else	// negative
		return;

	//Clear this value so that profiled enemies can be added into battles in the future.
	gfProfiledEnemyAdded = FALSE;

	//Clear enemies in battle for all mobile groups in the sector.
	CFOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (!pGroup->fVehicle                  &&
				pGroup->ubSectorX == gWorldSectorX &&
				pGroup->ubSectorY == gWorldSectorY)
		{
			pGroup->pEnemyGroup->ubTroopsInBattle = 0;
			pGroup->pEnemyGroup->ubElitesInBattle = 0;
			pGroup->pEnemyGroup->ubAdminsInBattle = 0;
		}
	}

	/* Check to see if any of our mercs have abandoned the militia during a
	 * battle.  This is cause for a rather severe loyalty blow. */
	if (IsAnyOfTeamOKInSector(MILITIA_TEAM) &&
			(IsAnyOfTeamOKInSector(ENEMY_TEAM) || IsAnyOfTeamOKInSector(CREATURE_TEAM)))
	{
		HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_ABANDON_MILITIA, gWorldSectorX, gWorldSectorY, 0);
	}
}


static UINT8 NumFreeEnemySlots(void)
{
	UINT8 ubNumFreeSlots = 0;
	INT32 i;
	//Count the number of free enemy slots.  It is possible to have multiple groups exceed the maximum.
	for( i = gTacticalStatus.Team[ ENEMY_TEAM ].bFirstID; i <= gTacticalStatus.Team[ ENEMY_TEAM].bLastID; i++ )
	{
		if (!GetMan(i).bActive) ++ubNumFreeSlots;
	}
	return ubNumFreeSlots;
}


static void PrepareEnemyForUndergroundBattle();


void PrepareEnemyForSectorBattle()
{
	gfPendingEnemies = FALSE;

	if (gbWorldSectorZ > 0)
	{
		PrepareEnemyForUndergroundBattle();
		return;
	}

	GROUP* const bg = gpBattleGroup;
	if (bg && !bg->fPlayer)
	{ /* The enemy has instigated the battle which means they are the ones
		 * entering the conflict. The player was actually in the sector first, and
		 * the enemy doesn't use reinforced placements */
		HandleArrivalOfReinforcements(bg);
		/* It is possible that other enemy groups have also arrived. Add them in the
		 * same manner. */
		FOR_ALL_ENEMY_GROUPS(g)
		{
			if (g == bg)                               continue;
			if (g->fVehicle)                           continue;
			if (g->ubSectorX != bg->ubSectorX)         continue;
			if (g->ubSectorY != bg->ubSectorY)         continue;
			if (g->pEnemyGroup->ubAdminsInBattle != 0) continue;
			if (g->pEnemyGroup->ubTroopsInBattle != 0) continue;
			if (g->pEnemyGroup->ubElitesInBattle != 0) continue;
			HandleArrivalOfReinforcements(g);
		}
		ValidateEnemiesHaveWeapons();
		return;
	}

	INT16 const x = gWorldSectorX;
	INT16 const y = gWorldSectorY;

	if (gbWorldSectorZ == 0 && NumEnemiesInSector(x, y) > 32)
	{
		gfPendingEnemies = TRUE;
	}

	UINT8       total_admins;
	UINT8       total_troops;
	UINT8       total_elites;
	SECTORINFO& sector = SectorInfo[SECTOR(x, y)];
	if (sector.uiFlags & SF_USE_MAP_SETTINGS)
	{ // Count the number of enemy placements in a map and use those
		total_admins = 0;
		total_troops = 0;
		total_elites = 0;
		CFOR_ALL_SOLDIERINITNODES(curr)
		{
			BASIC_SOLDIERCREATE_STRUCT const& bp = *curr->pBasicPlacement;
			if (bp.bTeam != ENEMY_TEAM) continue;
			switch (bp.ubSoldierClass)
			{
				case SOLDIER_CLASS_ADMINISTRATOR: ++total_admins; break;
				case SOLDIER_CLASS_ARMY:          ++total_troops; break;
				case SOLDIER_CLASS_ELITE:         ++total_elites; break;
			}
		}
		sector.ubNumAdmins      = total_admins;
		sector.ubNumTroops      = total_troops;
		sector.ubNumElites      = total_elites;
		sector.ubAdminsInBattle = 0;
		sector.ubTroopsInBattle = 0;
		sector.ubElitesInBattle = 0;
	}
	else
	{
		total_admins = sector.ubNumAdmins - sector.ubAdminsInBattle;
		total_troops = sector.ubNumTroops - sector.ubTroopsInBattle;
		total_elites = sector.ubNumElites - sector.ubElitesInBattle;
	}

	UINT8 const n_stationary_enemies = total_admins + total_troops + total_elites;
	if (n_stationary_enemies > 32)
	{
#ifdef JA2BETAVERSION
		ScreenMsg(FONT_RED, MSG_ERROR, L"The total stationary enemy forces in sector %c%d is %d. (max %d)", y + 'A' - 1, x, total_admins + total_troops + total_elites, 32);
#endif
		total_admins = MIN(total_admins, 32);
		total_troops = MIN(total_troops, 32 - total_admins);
		total_elites = MIN(total_elites, 32 - total_admins + total_troops);
	}

	sector.ubAdminsInBattle += total_admins;
	sector.ubTroopsInBattle += total_troops;
	sector.ubElitesInBattle += total_elites;

#ifdef JA2TESTVERSION
	if (gfOverrideSector)
	{ // If there are no troops in the current groups, then we're done.
		if (total_admins == 0 && total_troops == 0 && total_elites == 0) return;
		AddSoldierInitListEnemyDefenceSoldiers(total_admins, total_troops, total_elites);
		ValidateEnemiesHaveWeapons();
		return;
	}
#endif

	// Search for movement groups that happen to be in the sector.
	INT16 n_slots = NumFreeEnemySlots();
	//Test:  All slots should be free at this point!
	if (n_slots != gTacticalStatus.Team[ENEMY_TEAM].bLastID - gTacticalStatus.Team[ENEMY_TEAM].bFirstID + 1)
	{
#ifdef JA2BETAVERSION
		ScreenMsg(FONT_RED, MSG_ERROR, L"All enemy slots should be free at this point.  Only %d of %d are available.\nTrying to add %d admins, %d troops, and %d elites.",
				n_slots, gTacticalStatus.Team[ENEMY_TEAM].bLastID - gTacticalStatus.Team[ENEMY_TEAM].bFirstID + 1 ,
				total_admins, total_troops, total_elites);
#endif
	}
	/* Subtract the total number of stationary enemies from the available slots,
	 * as stationary forces take precendence in combat. The mobile forces that
	 * could also be in the same sector are considered later if all the slots fill
	 * up. */
	n_slots -= total_admins + total_troops + total_elites;
	/* Now, process all of the groups and search for both enemy and player groups
	 * in the sector. For enemy groups, we fill up the slots until we have none
	 * left or all of the groups have been processed. */
	FOR_ALL_GROUPS(g)
	{
		if (n_slots == 0) break;

		if (g->fVehicle)         continue;
		if (g->ubSectorX   != x) continue;
		if (g->ubSectorY   != y) continue;
		if (gbWorldSectorZ != 0) continue;

		if (!g->fPlayer)
		{ // Process enemy group in sector
			ENEMYGROUP& eg = *g->pEnemyGroup;
			if (n_slots > 0)
			{
				UINT8 n_admins = eg.ubNumAdmins - eg.ubAdminsInBattle;
				n_slots -= n_admins;
				if (n_slots < 0)
				{ // Adjust the value to zero
					n_admins        += n_slots;
					n_slots          = 0;
					gfPendingEnemies = TRUE;
				}
				eg.ubAdminsInBattle += n_admins;
				total_admins        += n_admins;
			}
			if (n_slots > 0)
			{ // Add regular army forces
				UINT8 n_troops = eg.ubNumTroops - eg.ubTroopsInBattle;
				n_slots -= n_troops;
				if (n_slots < 0)
				{ // Adjust the value to zero
					n_troops        += n_slots;
					n_slots          = 0;
					gfPendingEnemies = TRUE;
				}
				eg.ubTroopsInBattle += n_troops;
				total_troops        += n_troops;
			}
			if (n_slots > 0)
			{ // Add elite troops
				UINT8 n_elites = eg.ubNumElites - eg.ubElitesInBattle;
				n_slots -= n_elites;
				if (n_slots < 0)
				{ // Adjust the value to zero
					n_elites        += n_slots;
					n_slots          = 0;
					gfPendingEnemies = TRUE;
				}
				eg.ubElitesInBattle += n_elites;
				total_elites        += n_elites;
			}
			// NOTE: No provisions for profile troop leader or retreat groups yet
		}
		else if (!g->fBetweenSectors)
		{ /* TEMP: The player path needs to get destroyed, otherwise, it'll be
			 * impossible to move the group after the battle is resolved. */
			// XXX TODO001F This does not work, when n_slots drops to 0 before all player groups are handled.

			// No one in the group any more continue loop
			if (!g->pPlayerList) continue;

			RemoveGroupWaypoints(*g);
		}
	}

	// If there are no troops in the current groups, then we're done.
	if (total_admins == 0 && total_troops == 0 && total_elites == 0) return;

	AddSoldierInitListEnemyDefenceSoldiers(total_admins, total_troops, total_elites);

	/* Now, we have to go through all of the enemies in the new map and assign
	 * their respective groups if in a mobile group, but only for the ones that
	 * were assigned from the */
	n_slots = 32 - n_stationary_enemies;
	CFOR_ALL_ENEMY_GROUPS(g)
	{
		if (n_slots == 0) break;

		if (g->fVehicle) continue;
		if (g->ubSectorX   != x) continue;
		if (g->ubSectorY   != y) continue;
		if (gbWorldSectorZ != 0) continue;

		INT32 n        = g->ubGroupSize;
		UINT8 n_admins = g->pEnemyGroup->ubAdminsInBattle;
		UINT8 n_troops = g->pEnemyGroup->ubTroopsInBattle;
		UINT8 n_elites = g->pEnemyGroup->ubElitesInBattle;
		FOR_ALL_IN_TEAM(s, ENEMY_TEAM)
		{
			if (n == 0 || n_slots == 0) break;
			if (s->ubGroupID != 0) continue;

			switch (s->ubSoldierClass)
			{
				case SOLDIER_CLASS_ADMINISTRATOR:
					if (n_admins == 0) continue;
					--n_admins;
					break;

				case SOLDIER_CLASS_ARMY:
					if (n_troops == 0) continue;
					--n_troops;
					break;

				case SOLDIER_CLASS_ELITE:
					if (n_elites == 0) continue;
					--n_elites;
					break;

				default: continue;
			}

			--n;
			--n_slots;
			s->ubGroupID = g->ubGroupID;
		}
		AssertMsg(n == 0 || n_slots == 0, "Failed to assign battle counters for enemies properly. Please send save. KM:0.");
	}

	ValidateEnemiesHaveWeapons();
}


static void PrepareEnemyForUndergroundBattle()
{
	// This is the sector we are going to be fighting in.
	UNDERGROUND_SECTORINFO* const u = FindUnderGroundSector(gWorldSectorX, gWorldSectorY, gbWorldSectorZ);
	Assert(u);
	if (!u) return;

	if (u->ubNumAdmins == 0 && u->ubNumTroops == 0 && u->ubNumElites == 0) return;

	UINT8 const ubTotalAdmins = u->ubNumAdmins - u->ubAdminsInBattle;
	UINT8 const ubTotalTroops = u->ubNumTroops - u->ubTroopsInBattle;
	UINT8 const ubTotalElites = u->ubNumElites - u->ubElitesInBattle;
	u->ubAdminsInBattle += ubTotalAdmins;
	u->ubTroopsInBattle += ubTotalTroops;
	u->ubElitesInBattle += ubTotalElites;
	AddSoldierInitListEnemyDefenceSoldiers(u->ubNumAdmins, u->ubNumTroops, u->ubNumElites);
	ValidateEnemiesHaveWeapons();
}


//The queen AI layer must process the event by subtracting forces, etc.
void ProcessQueenCmdImplicationsOfDeath(const SOLDIERTYPE* const pSoldier)
{
	EvaluateDeathEffectsToSoldierInitList(*pSoldier);

	switch( pSoldier->ubProfile )
	{
		case MIKE:
		case IGGY:
			if( pSoldier->ubProfile == IGGY && !gubFact[ FACT_IGGY_AVAILABLE_TO_ARMY ] )
			{ //Iggy is on our team!
				break;
			}
			if( !pSoldier->bSectorZ )
			{
				SECTORINFO* pSector = &SectorInfo[SECTOR(pSoldier->sSectorX, pSoldier->sSectorY)];
				if( pSector->ubNumElites )
				{
					pSector->ubNumElites--;
				}
				if( pSector->ubElitesInBattle )
				{
					pSector->ubElitesInBattle--;
				}
			}
			else
			{
				UNDERGROUND_SECTORINFO *pUnderground;
				pUnderground = FindUnderGroundSector( (UINT8)pSoldier->sSectorX, (UINT8)pSoldier->sSectorY, (UINT8)pSoldier->bSectorZ );
				Assert( pUnderground );
				if( pUnderground->ubNumElites )
				{
					pUnderground->ubNumElites--;
				}
				if( pUnderground->ubElitesInBattle )
				{
					pUnderground->ubElitesInBattle--;
				}
			}
			break;
	}

	if( pSoldier->bNeutral || pSoldier->bTeam != ENEMY_TEAM && pSoldier->bTeam != CREATURE_TEAM )
		return;
	//we are recording an enemy death
	if( pSoldier->ubGroupID )
	{ //The enemy was in a mobile group
		GROUP *pGroup;
		pGroup = GetGroup( pSoldier->ubGroupID );
		if( !pGroup )
		{
			#ifdef JA2BETAVERSION
				wchar_t str[256];
				swprintf(str, lengthof(str), L"Enemy soldier killed with ubGroupID of %d, and the group doesn't exist!", pSoldier->ubGroupID);
				DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
			#endif
			return;
		}
		if( pGroup->fPlayer )
		{
			#ifdef JA2BETAVERSION
				wchar_t str[256];
				swprintf(str, lengthof(str), L"Attempting to process player group thinking it's an enemy group in ProcessQueenCmdImplicationsOfDeath()", pSoldier->ubGroupID);
				DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
			#endif
			return;
		}
		switch( pSoldier->ubSoldierClass )
		{
			case SOLDIER_CLASS_ELITE:
				#ifdef JA2BETAVERSION
					if( !pGroup->pEnemyGroup->ubNumElites )
					{
						wchar_t str[128];
						swprintf(str, lengthof(str), L"Enemy elite killed with ubGroupID of %d, but the group doesn't contain elites!", pGroup->ubGroupID);
						DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
						break;
					}
					if( guiCurrentScreen == GAME_SCREEN )
					{
						if( pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE && pGroup->pEnemyGroup->ubNumElites != pGroup->pEnemyGroup->ubElitesInBattle && !gfPendingEnemies ||
								pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE || pGroup->pEnemyGroup->ubNumElites > 50 || pGroup->pEnemyGroup->ubElitesInBattle > 50 )
						{
							DoScreenIndependantMessageBox( L"Group elite counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
						}
					}
				#endif
				if( pGroup->pEnemyGroup->ubNumElites )
				{
					pGroup->pEnemyGroup->ubNumElites--;
				}
				if( pGroup->pEnemyGroup->ubElitesInBattle )
				{
					pGroup->pEnemyGroup->ubElitesInBattle--;
				}
				break;
			case SOLDIER_CLASS_ARMY:
				#ifdef JA2BETAVERSION
					if( !pGroup->pEnemyGroup->ubNumTroops )
					{
						wchar_t str[128];
						swprintf(str, lengthof(str), L"Enemy troop killed with ubGroupID of %d, but the group doesn't contain elites!", pGroup->ubGroupID);
						DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
						break;
					}
					if( guiCurrentScreen == GAME_SCREEN )
					{
						if( pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE && pGroup->pEnemyGroup->ubNumTroops != pGroup->pEnemyGroup->ubTroopsInBattle && !gfPendingEnemies ||
								pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE || pGroup->pEnemyGroup->ubNumTroops > 50 || pGroup->pEnemyGroup->ubTroopsInBattle > 50 )
						{
							DoScreenIndependantMessageBox( L"Group troop counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
						}
					}
				#endif
				if( pGroup->pEnemyGroup->ubNumTroops )
				{
					pGroup->pEnemyGroup->ubNumTroops--;
				}
				if( pGroup->pEnemyGroup->ubTroopsInBattle )
				{
					pGroup->pEnemyGroup->ubTroopsInBattle--;
				}
				break;
			case SOLDIER_CLASS_ADMINISTRATOR:
				#ifdef JA2BETAVERSION
					if( !pGroup->pEnemyGroup->ubNumAdmins )
					{
						wchar_t str[128];
						swprintf(str, lengthof(str), L"Enemy administrator killed with ubGroupID of %d, but the group doesn't contain elites!", pGroup->ubGroupID);
						DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
						break;
					}
					if( guiCurrentScreen == GAME_SCREEN )
					{
						if( pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE && pGroup->pEnemyGroup->ubNumAdmins != pGroup->pEnemyGroup->ubAdminsInBattle && !gfPendingEnemies ||
						pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE || pGroup->pEnemyGroup->ubNumAdmins > 50 || pGroup->pEnemyGroup->ubAdminsInBattle > 50 )
						{
							DoScreenIndependantMessageBox( L"Group admin counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
						}
					}
				#endif
				if( pGroup->pEnemyGroup->ubNumAdmins )
				{
					pGroup->pEnemyGroup->ubNumAdmins--;
				}
				if( pGroup->pEnemyGroup->ubAdminsInBattle )
				{
					pGroup->pEnemyGroup->ubAdminsInBattle--;
				}
				break;
		}
		if( pGroup->ubGroupSize )
			pGroup->ubGroupSize--;
		RecalculateGroupWeight( pGroup );
		if( !pGroup->ubGroupSize )
		{
			RemovePGroup(*pGroup);
		}
	}
	else
	{ //The enemy was in a stationary defence group
		if( !gbWorldSectorZ || IsAutoResolveActive() )
		{ //ground level (SECTORINFO)
			SECTORINFO *pSector;
			#ifdef JA2BETAVERSION
			UINT32 ubTotalEnemies;
			#endif

			if( !IsAutoResolveActive() )
			{
				pSector = &SectorInfo[ SECTOR( pSoldier->sSectorX, pSoldier->sSectorY ) ];
			}
			else
			{
				pSector = &SectorInfo[ GetAutoResolveSectorID() ];
			}

			#ifdef JA2BETAVERSION
				ubTotalEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
			#endif

			switch( pSoldier->ubSoldierClass )
			{
				case SOLDIER_CLASS_ADMINISTRATOR:
					#ifdef JA2BETAVERSION
						if( guiCurrentScreen == GAME_SCREEN )
						{
							if( ubTotalEnemies <= 32 && pSector->ubNumAdmins != pSector->ubAdminsInBattle ||
								!pSector->ubNumAdmins || !pSector->ubAdminsInBattle ||
								pSector->ubNumAdmins > 100 || pSector->ubAdminsInBattle > 32 )
							{
								DoScreenIndependantMessageBox( L"Sector admin counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
							}
						}
					#endif
					if( pSector->ubNumAdmins )
					{
						pSector->ubNumAdmins--;
					}
					if( pSector->ubAdminsInBattle )
					{
						pSector->ubAdminsInBattle--;
					}
					break;
				case SOLDIER_CLASS_ARMY:
					#ifdef JA2BETAVERSION
						if( guiCurrentScreen == GAME_SCREEN )
						{
							if( ubTotalEnemies <= 32 && pSector->ubNumTroops != pSector->ubTroopsInBattle ||
									!pSector->ubNumTroops || !pSector->ubTroopsInBattle ||
									pSector->ubNumTroops > 100 || pSector->ubTroopsInBattle > 32 )
							{
								DoScreenIndependantMessageBox( L"Sector troop counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
							}
						}
					#endif
					if( pSector->ubNumTroops )
					{
						pSector->ubNumTroops--;
					}
					if( pSector->ubTroopsInBattle )
					{
						pSector->ubTroopsInBattle--;
					}
					break;
				case SOLDIER_CLASS_ELITE:
					#ifdef JA2BETAVERSION
						if( guiCurrentScreen == GAME_SCREEN )
						{
							if( ubTotalEnemies <= 32 && pSector->ubNumElites != pSector->ubElitesInBattle ||
									!pSector->ubNumElites || !pSector->ubElitesInBattle ||
									pSector->ubNumElites > 100 || pSector->ubElitesInBattle > 32 )
							{
								DoScreenIndependantMessageBox( L"Sector elite counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
							}
						}
					#endif
					if( pSector->ubNumElites )
					{
						pSector->ubNumElites--;
					}
					if( pSector->ubElitesInBattle )
					{
						pSector->ubElitesInBattle--;
					}
					break;
				case SOLDIER_CLASS_CREATURE:
					if( pSoldier->ubBodyType != BLOODCAT )
					{
						#ifdef JA2BETAVERSION
							if( guiCurrentScreen == GAME_SCREEN )
							{
								if( ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumCreatures != pSector->ubCreaturesInBattle ||
										!pSector->ubNumCreatures || !pSector->ubCreaturesInBattle ||
										pSector->ubNumCreatures > 50 || pSector->ubCreaturesInBattle > 50 )
								{
									DoScreenIndependantMessageBox( L"Sector creature counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
								}
							}
						#endif
						if( pSector->ubNumCreatures )
						{
							pSector->ubNumCreatures--;
						}
						if( pSector->ubCreaturesInBattle )
						{
							pSector->ubCreaturesInBattle--;
						}
					}
					else
					{
						if( pSector->bBloodCats )
						{
							pSector->bBloodCats--;
						}
					}

					break;
			}
			RecalculateSectorWeight( (UINT8)SECTOR( pSoldier->sSectorX, pSoldier->sSectorY ) );
		}
		else
		{ //basement level (UNDERGROUND_SECTORINFO)
			UNDERGROUND_SECTORINFO *pSector = FindUnderGroundSector( gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
			#ifdef JA2BETAVERSION
			UINT32 ubTotalEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
			#endif
			if( pSector )
			{
				switch( pSoldier->ubSoldierClass )
				{
					case SOLDIER_CLASS_ADMINISTRATOR:
						#ifdef JA2BETAVERSION
						if( ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumAdmins != pSector->ubAdminsInBattle ||
								!pSector->ubNumAdmins || !pSector->ubAdminsInBattle ||
								pSector->ubNumAdmins > 100 || pSector->ubAdminsInBattle > MAX_STRATEGIC_TEAM_SIZE )
						{
							DoScreenIndependantMessageBox( L"Underground sector admin counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
						}
						#endif
						if( pSector->ubNumAdmins )
						{
							pSector->ubNumAdmins--;
						}
						if( pSector->ubAdminsInBattle )
						{
							pSector->ubAdminsInBattle--;
						}
						break;
					case SOLDIER_CLASS_ARMY:
						#ifdef JA2BETAVERSION
						if( ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumTroops != pSector->ubTroopsInBattle ||
								!pSector->ubNumTroops || !pSector->ubTroopsInBattle ||
								pSector->ubNumTroops > 100 || pSector->ubTroopsInBattle > MAX_STRATEGIC_TEAM_SIZE )
						{
							DoScreenIndependantMessageBox( L"Underground sector troop counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
						}
						#endif
						if( pSector->ubNumTroops )
						{
							pSector->ubNumTroops--;
						}
						if( pSector->ubTroopsInBattle )
						{
							pSector->ubTroopsInBattle--;
						}
						break;
					case SOLDIER_CLASS_ELITE:
						#ifdef JA2BETAVERSION
						if( ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumElites != pSector->ubElitesInBattle ||
								!pSector->ubNumElites || !pSector->ubElitesInBattle ||
								pSector->ubNumElites > 100 || pSector->ubElitesInBattle > MAX_STRATEGIC_TEAM_SIZE )
						{
							DoScreenIndependantMessageBox( L"Underground sector elite counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
						}
						#endif
						if( pSector->ubNumElites )
						{
							pSector->ubNumElites--;
						}
						if( pSector->ubElitesInBattle )
						{
							pSector->ubElitesInBattle--;
						}
						break;
					case SOLDIER_CLASS_CREATURE:
						#ifdef JA2BETAVERSION
						if( ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumCreatures != pSector->ubCreaturesInBattle ||
								!pSector->ubNumCreatures || !pSector->ubCreaturesInBattle ||
								pSector->ubNumCreatures > 50 || pSector->ubCreaturesInBattle > 50 )
						{
							DoScreenIndependantMessageBox( L"Underground sector creature counters are bad.  What were the last 2-3 things to die, and how?  Save game and send to KM with info!!!", MSG_BOX_FLAG_OK, NULL );
						}
						#endif
						if( pSector->ubNumCreatures )
						{
							pSector->ubNumCreatures--;
						}
						if( pSector->ubCreaturesInBattle )
						{
							pSector->ubCreaturesInBattle--;
						}

						if( !pSector->ubNumCreatures && gWorldSectorX != 9 && gWorldSectorY != 10 )
						{ //If the player has successfully killed all creatures in ANY underground sector except J9
							//then cancel any pending creature town attack.
							DeleteAllStrategicEventsOfType( EVENT_CREATURE_ATTACK );
						}

						// a monster has died.  Post an event to immediately check whether a mine has been cleared.
						AddStrategicEventUsingSeconds( EVENT_CHECK_IF_MINE_CLEARED, GetWorldTotalSeconds() + 15, 0);

						if( pSoldier->ubBodyType == QUEENMONSTER )
						{
							//Need to call this, as the queen is really big, and killing her leaves a bunch
							//of bad tiles in behind her.  Calling this function cleans it up.
							InvalidateWorldRedundency();
							//Now that the queen is dead, turn off the creature quest.
							EndCreatureQuest();
							EndQuest( QUEST_CREATURES, gWorldSectorX, gWorldSectorY );
						}
						break;
				}
			}
		}
	}
}


static void AddEnemiesToBattle(GROUP* pGroup, UINT8 ubStrategicInsertionCode, UINT8 ubNumAdmins, UINT8 ubNumTroops, UINT8 ubNumElites, BOOLEAN fMagicallyAppeared);


//Rarely, there will be more enemies than supported by the engine.  In this case, these
//soldier's are waiting for a slot to be free so that they can enter the battle.  This
//essentially allows for an infinite number of troops, though only 32 at a time can fight.
//This is also called whenever an enemy group's reinforcements arrive because the code is
//identical, though it is highly likely that they will all be successfully added on the first call.
void AddPossiblePendingEnemiesToBattle()
{
	UINT8 ubSlots, ubNumAvailable;
	UINT8 ubNumElites, ubNumTroops, ubNumAdmins;
  if( !gfPendingEnemies )
	{ //Optimization.  No point in checking if we know that there aren't any more enemies that can
		//be added to this battle.  This changes whenever a new enemy group arrives at the scene.
		return;
	}
	ubSlots = NumFreeEnemySlots();
	if( !ubSlots )
	{ //no available slots to add enemies to.  Try again later...
		return;
	}

	FOR_ALL_ENEMY_GROUPS(pGroup)
	{
		if (ubSlots == 0) break;

		if (!pGroup->fVehicle                  &&
				pGroup->ubSectorX == gWorldSectorX &&
				pGroup->ubSectorY == gWorldSectorY &&
				!gbWorldSectorZ)
		{ //This enemy group is currently in the sector.
			ubNumElites = ubNumTroops = ubNumAdmins = 0;
			ubNumAvailable = pGroup->ubGroupSize - pGroup->pEnemyGroup->ubElitesInBattle - pGroup->pEnemyGroup->ubTroopsInBattle - pGroup->pEnemyGroup->ubAdminsInBattle;
			while( ubNumAvailable && ubSlots )
			{ //This group has enemies waiting for a chance to enter the battle.
				if( pGroup->pEnemyGroup->ubTroopsInBattle < pGroup->pEnemyGroup->ubNumTroops )
				{ //Add a regular troop.
					pGroup->pEnemyGroup->ubTroopsInBattle++;
					ubNumAvailable--;
					ubSlots--;
					ubNumTroops++;
				}
				else if( pGroup->pEnemyGroup->ubElitesInBattle < pGroup->pEnemyGroup->ubNumElites )
				{ //Add an elite troop
					pGroup->pEnemyGroup->ubElitesInBattle++;
					ubNumAvailable--;
					ubSlots--;
					ubNumElites++;
				}
				else if( pGroup->pEnemyGroup->ubAdminsInBattle < pGroup->pEnemyGroup->ubNumAdmins )
				{ //Add an elite troop
					pGroup->pEnemyGroup->ubAdminsInBattle++;
					ubNumAvailable--;
					ubSlots--;
					ubNumAdmins++;
				}
				else
				{
					AssertMsg( 0, "AddPossiblePendingEnemiesToBattle():  Logic Error -- by Kris" );
				}
			}
			if( ubNumAdmins || ubNumTroops || ubNumElites )
			{ //This group has contributed forces, then add them now, because different
				//groups appear on different sides of the map.
				UINT8 ubStrategicInsertionCode=0;
				//First, determine which entrypoint to use, based on the travel direction of the group.
				if( pGroup->ubPrevX && pGroup->ubPrevY )
				{
					if( pGroup->ubSectorX < pGroup->ubPrevX )
						ubStrategicInsertionCode = INSERTION_CODE_EAST;
					else if( pGroup->ubSectorX > pGroup->ubPrevX )
						ubStrategicInsertionCode = INSERTION_CODE_WEST;
					else if( pGroup->ubSectorY < pGroup->ubPrevY )
						ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
					else if( pGroup->ubSectorY > pGroup->ubPrevY )
						ubStrategicInsertionCode = INSERTION_CODE_NORTH;
				}
				else if( pGroup->ubNextX && pGroup->ubNextY )
				{
					if( pGroup->ubSectorX < pGroup->ubNextX )
						ubStrategicInsertionCode = INSERTION_CODE_EAST;
					else if( pGroup->ubSectorX > pGroup->ubNextX )
						ubStrategicInsertionCode = INSERTION_CODE_WEST;
					else if( pGroup->ubSectorY < pGroup->ubNextY )
						ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
					else if( pGroup->ubSectorY > pGroup->ubNextY )
						ubStrategicInsertionCode = INSERTION_CODE_NORTH;
				}
				//Add the number of each type of troop and place them in the appropriate positions
				AddEnemiesToBattle( pGroup, ubStrategicInsertionCode, ubNumAdmins, ubNumTroops, ubNumElites, FALSE );
			}
		}
	}
	if( ubSlots )
	{ //After going through the process, we have finished with some free slots and no more enemies to add.
    //So, we can turn off the flag, as this check is no longer needed.
		gfPendingEnemies = FALSE;
	}
}


static void NotifyPlayersOfNewEnemies(void)
{
	INT32 iSoldiers;
	INT32 iChosenSoldier;
	BOOLEAN fIgnoreBreath = FALSE;

	iSoldiers = 0;
	CFOR_ALL_IN_TEAM(s, OUR_TEAM)
	{ //find a merc that is aware.
		if (s->bInSector && s->bLife >= OKLIFE && s->bBreath >= OKBREATH)
		{
			iSoldiers++;
		}
	}
	if( !iSoldiers )
	{ // look for an out of breath merc.
		fIgnoreBreath = TRUE;

		CFOR_ALL_IN_TEAM(s, OUR_TEAM)
		{ //find a merc that is aware.
			if (s->bInSector && s->bLife >= OKLIFE)
			{
				iSoldiers++;
			}
		}
	}
	if( iSoldiers )
	{
		iChosenSoldier = Random( iSoldiers );
		FOR_ALL_IN_TEAM(s, OUR_TEAM)
		{ //find a merc that is aware.
			if (s->bInSector &&
					s->bLife >= OKLIFE &&
					(s->bBreath >= OKBREATH || fIgnoreBreath))
			{
				if( !iChosenSoldier )
				{
					// ATE: This is to allow special handling of initial heli drop
					if ( !DidGameJustStart() )
					{
						TacticalCharacterDialogue(s, QUOTE_ENEMY_PRESENCE);
					}
					return;
				}
				iChosenSoldier--;
			}
		}
	}
	else
	{ //There is either nobody here or our mercs can't talk

	}
}


static void AddEnemiesToBattle(GROUP* const g, UINT8 const strategic_insertion_code, UINT8 n_admins, UINT8 n_troops, UINT8 n_elites, BOOLEAN const magically_appeared)
{
#ifdef JA2TESTVERSION
	ScreenMsg(FONT_RED, MSG_INTERFACE, L"Enemy reinforcements have arrived! (%d admins, %d troops, %d elite)", n_admins, n_troops, n_elites);
#endif

	UINT8 desired_direction;
	switch (strategic_insertion_code)
	{
		case INSERTION_CODE_NORTH: desired_direction = SOUTHEAST; break;
		case INSERTION_CODE_EAST:  desired_direction = SOUTHWEST; break;
		case INSERTION_CODE_SOUTH: desired_direction = NORTHWEST; break;
		case INSERTION_CODE_WEST:  desired_direction = NORTHEAST; break;
		default: throw std::logic_error("Illegal direction passed to AddEnemiesToBattle()");
	}

	if (magically_appeared)
	{ // Update the strategic counters
		if (gbWorldSectorZ == 0)
		{
			SECTORINFO& sector = SectorInfo[SECTOR(gWorldSectorX, gWorldSectorY)];
			sector.ubNumAdmins      += n_admins;
			sector.ubAdminsInBattle += n_admins;
			sector.ubNumTroops      += n_troops;
			sector.ubTroopsInBattle += n_troops;
			sector.ubNumElites      += n_elites;
			sector.ubElitesInBattle += n_elites;
		}
		else if (UNDERGROUND_SECTORINFO* const sector = FindUnderGroundSector(gWorldSectorX, gWorldSectorY, gbWorldSectorZ))
		{
			sector->ubNumAdmins      += n_admins;
			sector->ubAdminsInBattle += n_admins;
			sector->ubNumTroops      += n_troops;
			sector->ubTroopsInBattle += n_troops;
			sector->ubNumElites      += n_elites;
			sector->ubElitesInBattle += n_elites;
		}
		// Because the enemies magically appeared, have one of our soldiers say something
		NotifyPlayersOfNewEnemies();
	}

	UINT8            n_total   = n_admins + n_troops + n_elites;
	UINT8            curr_slot = 0;
	MAPEDGEPOINTINFO edgepoint_info;
	ChooseMapEdgepoints(&edgepoint_info, strategic_insertion_code, n_total);
	while (n_total != 0)
	{
		UINT32       const roll = Random(n_total--);
		SoldierClass const sc   =
			roll < n_elites           ? --n_elites, SOLDIER_CLASS_ELITE :
			roll < n_elites + n_total ? --n_total,  SOLDIER_CLASS_ARMY  :
			(--n_admins, SOLDIER_CLASS_ADMINISTRATOR);

		SOLDIERTYPE* const s = TacticalCreateEnemySoldier(sc);
		if (g) s->ubGroupID = g->ubGroupID;
		s->ubInsertionDirection = desired_direction;
		// Setup the position
		if (curr_slot < edgepoint_info.ubNumPoints)
		{ // Use an edgepoint
			s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			s->usStrategicInsertionData = edgepoint_info.sGridNo[curr_slot++];
		}
		else
		{ // No edgepoints left, so put him at the entrypoint
			s->ubStrategicInsertionCode = strategic_insertion_code;
		}
		UpdateMercInSector(*s, gWorldSectorX, gWorldSectorY, 0);
	}
}


void SaveUnderGroundSectorInfoToSaveGame(HWFILE const f)
{
	// Save the number of nodes
	UINT32 n_records = 0;
	for (UNDERGROUND_SECTORINFO const* i = gpUndergroundSectorInfoHead; i; i = i->next)
	{
		++n_records;
	}
	FileWrite(f, &n_records, sizeof(UINT32));

	// Save the nodes
	for (UNDERGROUND_SECTORINFO const* i = gpUndergroundSectorInfoHead; i; i = i->next)
	{
		InjectUndergroundSectorInfoIntoFile(f, i);
	}
}


void LoadUnderGroundSectorInfoFromSavedGame(HWFILE const f)
{
	TrashUndergroundSectorInfo();

	// Read the number of nodes stored
	UINT32 n_records;
	FileRead(f, &n_records, sizeof(UINT32));

#ifdef JA2BETAVERSION
	if (n_records == 0)
	{
		BuildUndergroundSectorInfoList();
		gfClearCreatureQuest = TRUE;
		return;
	}
#endif

	UNDERGROUND_SECTORINFO** anchor = &gpUndergroundSectorInfoHead;
	for (UINT32 n = n_records; n != 0; --n)
	{
		UNDERGROUND_SECTORINFO* const u = MALLOC(UNDERGROUND_SECTORINFO);
		ExtractUndergroundSectorInfoFromFile(f, u);

		gpUndergroundSectorInfoTail = u;
		*anchor = u;
		anchor  = &u->next;
	}
}


UNDERGROUND_SECTORINFO* FindUnderGroundSector(INT16 const x, INT16 const y, UINT8 const z)
{
	UNDERGROUND_SECTORINFO* i = gpUndergroundSectorInfoHead;
	for (; i; i = i->next)
	{
		if (i->ubSectorX != x) continue;
		if (i->ubSectorY != y) continue;
		if (i->ubSectorZ != z) continue;
		break;
	}
	return i;
}


void BeginCaptureSquence( )
{
	if( !( gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE ) || !( gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE ) )
	{
		gStrategicStatus.ubNumCapturedForRescue = 0;
	}
}

void EndCaptureSequence( )
{

	// Set flag...
	if( !( gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE ) || !(gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE) )
	{
		// CJC Dec 1 2002: fixing multiple captures:
		//gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE;

		if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTNOTSTARTED )
		{
			// CJC Dec 1 2002: fixing multiple captures:
			gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE;
			StartQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY );
		}
		// CJC Dec 1 2002: fixing multiple captures:
		//else if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE )
		else if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE && gubQuest[ QUEST_INTERROGATION ] == QUESTNOTSTARTED )
		{
			StartQuest( QUEST_INTERROGATION, gWorldSectorX, gWorldSectorY );
			// CJC Dec 1 2002: fixing multiple captures:
			gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE;

			ScheduleMeanwhileEvent(7, 14, 0, INTERROGATION, QUEEN, 10);
		}
		// CJC Dec 1 2002: fixing multiple captures
		else
		{
			// !?!? set both flags
			gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE;
			gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE;
		}
	}

}


static void CaptureSoldier(SOLDIERTYPE* const s, INT16 const x, INT16 const y, GridNo const soldier_pos, GridNo const item_pos)
{
	s->sSectorX                 = x;
	s->sSectorY                 = y;
	s->bSectorZ                 = 0;
	s->bLevel                   = 0; // put him on the floor
	s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
	s->usStrategicInsertionData = soldier_pos;

	// Drop all items
	FOR_ALL_SOLDIER_INV_SLOTS(i, *s)
	{
		OBJECTTYPE& o = *i;
		if (o.usItem == NOTHING) continue;

		AddItemsToUnLoadedSector(x, y, 0, item_pos, 1, &o, 0, 0, 0, VISIBILITY_0);
		DeleteObj(&o);
	}
}


void EnemyCapturesPlayerSoldier( SOLDIERTYPE *pSoldier )
{
  BOOLEAN       fMadeCorpse;
  INT32         iNumEnemiesInSector;

	static INT16 sAlmaCaptureGridNos[] = { 9208, 9688, 9215 };
	static INT16 sAlmaCaptureItemsGridNo[] = { 12246, 12406, 13046 };

	static INT16 sInterrogationItemGridNo[] = { 12089, 12089, 12089 };


  // ATE: Check first if ! in player captured sequence already
	// CJC Dec 1 2002: fixing multiple captures
	if ( ( gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_RESCUE ) && (gStrategicStatus.uiFlags & STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE) )
  {
    return;
  }

  // ATE: If maximum prisoners captured, return!
  if ( gStrategicStatus.ubNumCapturedForRescue > 3 )
  {
    return;
  }


  // If this is an EPC , just kill them...
  if ( AM_AN_EPC( pSoldier ) )
  {
	  pSoldier->bLife = 0;
    HandleSoldierDeath( pSoldier, &fMadeCorpse );
    return;
  }

  if ( pSoldier->uiStatusFlags & SOLDIER_VEHICLE )
  {
    return;
  }

  // ATE: Patch fix If in a vehicle, remove from vehicle...
  TakeSoldierOutOfVehicle( pSoldier );


  // Are there anemies in ALMA? ( I13 )
  iNumEnemiesInSector = NumEnemiesInSector( 13, 9 );

  // IF there are no enemies, and we need to do alma, skip!
  if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTNOTSTARTED && iNumEnemiesInSector == 0 )
  {
	  InternalStartQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY, FALSE );
	  InternalEndQuest( QUEST_HELD_IN_ALMA, gWorldSectorX, gWorldSectorY, FALSE );
  }

	HandleMoraleEvent( pSoldier, MORALE_MERC_CAPTURED, pSoldier->sSectorX, pSoldier->sSectorY, pSoldier->bSectorZ );

	// Change to POW....
	//-add him to a POW assignment/group
	if( ( pSoldier->bAssignment != ASSIGNMENT_POW )  )
	{
		SetTimeOfAssignmentChangeForMerc( pSoldier );
	}

	ChangeSoldiersAssignment( pSoldier, ASSIGNMENT_POW );
	// ATE: Make them neutral!
	if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTNOTSTARTED )
	{
		pSoldier->bNeutral = TRUE;
	}

	RemoveCharacterFromSquads( pSoldier );

	// Is this the first one..?
	if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTNOTSTARTED )
	{
		UINT8& idx = gStrategicStatus.ubNumCapturedForRescue;
		// Teleport him to NE Alma sector (not Tixa as originally planned)
		CaptureSoldier(pSoldier, 13, 9, sAlmaCaptureGridNos[idx], sAlmaCaptureItemsGridNo[idx]);
		++idx;
	}
	else if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE )
	{
		// Teleport him to N7
		UINT8& idx = gStrategicStatus.ubNumCapturedForRescue;
		CaptureSoldier(pSoldier, 7, 14, gsInterrogationGridNo[idx], sInterrogationItemGridNo[idx]);
		++idx;
	}

	//Bandaging him would prevent him from dying (due to low HP)
	pSoldier->bBleeding = 0;

	// wake him up
	if ( pSoldier->fMercAsleep )
	{
		PutMercInAwakeState( pSoldier );
		pSoldier->fForcedToStayAwake = FALSE;
	}

	//Set his life to 50% + or - 10 HP.
	pSoldier->bLife = pSoldier->bLifeMax / 2;
	if ( pSoldier->bLife <= 35 )
	{
		pSoldier->bLife = 35;
	}
	else if ( pSoldier->bLife >= 45 )
	{
		pSoldier->bLife += (INT8)(10 - Random( 21 ) );
	}

	// make him quite exhausted when found
	pSoldier->bBreath = pSoldier->bBreathMax = 50;
	pSoldier->sBreathRed = 0;
	pSoldier->fMercCollapsedFlag = FALSE;
}


BOOLEAN PlayerSectorDefended( UINT8 ubSectorID )
{
	SECTORINFO *pSector;
	pSector = &SectorInfo[ ubSectorID ];
	if( pSector->ubNumberOfCivsAtLevel[ GREEN_MILITIA ] +
			pSector->ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] +
			pSector->ubNumberOfCivsAtLevel[ ELITE_MILITIA ] )
	{ //militia in sector
		return TRUE;
	}
	// Player in sector?
	return FindPlayerMovementGroupInSector(SECTORX(ubSectorID), SECTORY(ubSectorID)) != NULL;
}


static BOOLEAN AnyNonNeutralOfTeamInSector(INT8 team)
{
	CFOR_ALL_IN_TEAM(s, team)
	{
		if (s->bInSector && s->bLife != 0 && !s->bNeutral)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//Assumes gTacticalStatus.fEnemyInSector
BOOLEAN OnlyHostileCivsInSector()
{
	//Look for any hostile civs.
	if (!AnyNonNeutralOfTeamInSector(CIV_TEAM)) return FALSE;
	//Look for anybody else hostile.  If found, return FALSE immediately.
	if (AnyNonNeutralOfTeamInSector(ENEMY_TEAM))    return FALSE;
	if (AnyNonNeutralOfTeamInSector(CREATURE_TEAM)) return FALSE;
	if (AnyNonNeutralOfTeamInSector(MILITIA_TEAM))  return FALSE;
	//We only have hostile civilians, don't allow time compression.
	return TRUE;
}
