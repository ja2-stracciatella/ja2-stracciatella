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
#include "Campaign_Init.h"
#include "Meanwhile.h"
#include "Soldier_Macros.h"
#include "Morale.h"
#include "MemMan.h"
#include "FileMan.h"
#include "Vehicles.h"
#include "Logger.h"
#include "JAScreens.h"

//The sector information required for the strategic AI.  Contains the number of enemy troops,
//as well as intentions, etc.
SECTORINFO SectorInfo[256];
UNDERGROUND_SECTORINFO *gpUndergroundSectorInfoHead = NULL;
extern UNDERGROUND_SECTORINFO* gpUndergroundSectorInfoTail;
BOOLEAN gfPendingEnemies = FALSE;

extern std::vector<GARRISON_GROUP> gGarrisonGroup;

INT16 gsInterrogationGridNo[3] = { 7756, 7757, 7758 };

//Counts enemies and crepitus, but not bloodcats.
UINT8 NumHostilesInSector(const SGPSector& sSector)
{
	Assert(sSector.IsValid());
	UINT8 ubNumHostiles = 0;

	if (sSector.z)
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector(sSector);
		if( pSector )
		{
			ubNumHostiles = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumCreatures);
		}
	}
	else
	{
		SECTORINFO *pSector;

		//Count stationary hostiles
		pSector = &SectorInfo[sSector.AsByte()];
		ubNumHostiles = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumCreatures);

		//Count mobile enemies
		CFOR_EACH_ENEMY_GROUP(pGroup)
		{
			if (!pGroup->fVehicle && pGroup->ubSector == sSector)
			{
				ubNumHostiles += pGroup->ubGroupSize;
			}
		}
	}

	return ubNumHostiles;
}

UINT8 NumEnemiesInAnySector(const SGPSector& sSector)
{
	Assert(sSector.IsValid());
	UINT8 ubNumEnemies = 0;

	if (sSector.z)
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector(sSector);
		if( pSector )
		{
			ubNumEnemies = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
		}
	}
	else
	{
		SECTORINFO *pSector;

		//Count stationary enemies
		pSector = &SectorInfo[sSector.AsByte()];
		ubNumEnemies = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);

		//Count mobile enemies
		CFOR_EACH_ENEMY_GROUP(pGroup)
		{
			if (!pGroup->fVehicle && pGroup->ubSector == sSector)
			{
				ubNumEnemies += pGroup->ubGroupSize;
			}
		}
	}

	return ubNumEnemies;
}

UINT8 NumEnemiesInSector(const SGPSector& sSector)
{
	Assert(sSector.IsValid());

	SECTORINFO *pSector = &SectorInfo[sSector.AsByte()];
	UINT8 ubNumTroops = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);

	CFOR_EACH_ENEMY_GROUP(pGroup)
	{
		if (!pGroup->fVehicle && pGroup->ubSector == sSector)
		{
			ubNumTroops += pGroup->ubGroupSize;
		}
	}
	return ubNumTroops;
}

UINT8 NumStationaryEnemiesInSector(const SGPSector& sSector)
{
	Assert(sSector.IsValid());

	SECTORINFO* pSector = &SectorInfo[sSector.AsByte()];
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

UINT8 NumMobileEnemiesInSector(const SGPSector& sSector)
{
	Assert(sSector.IsValid());
	SECTORINFO *pSector;
	UINT8 ubNumTroops;

	ubNumTroops = 0;
	CFOR_EACH_ENEMY_GROUP(pGroup)
	{
		if (!pGroup->fVehicle && pGroup->ubSector == sSector)
		{
			ubNumTroops += pGroup->ubGroupSize;
		}
	}

	pSector = &SectorInfo[sSector.AsByte()];
	if( pSector->ubGarrisonID == ROADBLOCK )
	{ //consider these troops as mobile troops even though they are in a garrison
		ubNumTroops += (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
	}

	return ubNumTroops;
}


static void GetNumberOfMobileEnemiesInSector(const SGPSector& sSector, UINT8* pubNumAdmins, UINT8* pubNumTroops, UINT8* pubNumElites)
{
	Assert(sSector.IsValid());
	SECTORINFO *pSector;

	//Now count the number of mobile groups in the sector.
	*pubNumTroops = *pubNumElites = *pubNumAdmins = 0;
	CFOR_EACH_ENEMY_GROUP(pGroup)
	{
		if (!pGroup->fVehicle && pGroup->ubSector == sSector)
		{
			*pubNumTroops += pGroup->pEnemyGroup->ubNumTroops;
			*pubNumElites += pGroup->pEnemyGroup->ubNumElites;
			*pubNumAdmins += pGroup->pEnemyGroup->ubNumAdmins;
		}
	}

	pSector = &SectorInfo[sSector.AsByte()];
	if( pSector->ubGarrisonID == ROADBLOCK )
	{ //consider these troops as mobile troops even though they are in a garrison
		*pubNumAdmins += pSector->ubNumAdmins;
		*pubNumTroops += pSector->ubNumTroops;
		*pubNumElites += pSector->ubNumElites;
	}

}


static void GetNumberOfStationaryEnemiesInSector(const SGPSector& sSector, UINT8* pubNumAdmins, UINT8* pubNumTroops, UINT8* pubNumElites)
{
	Assert(sSector.IsValid());
	SECTORINFO* pSector = &SectorInfo[sSector.AsByte()];

	//grab the number of each type in the stationary sector
	*pubNumAdmins = pSector->ubNumAdmins;
	*pubNumTroops = pSector->ubNumTroops;
	*pubNumElites = pSector->ubNumElites;
}

void GetNumberOfEnemiesInSector(const SGPSector& sSector, UINT8 *pubNumAdmins, UINT8 *pubNumTroops, UINT8 *pubNumElites )
{
	UINT8 ubNumAdmins, ubNumTroops, ubNumElites;

	GetNumberOfStationaryEnemiesInSector(sSector, pubNumAdmins, pubNumTroops, pubNumElites);
	GetNumberOfMobileEnemiesInSector(sSector, &ubNumAdmins, &ubNumTroops, &ubNumElites);

	*pubNumAdmins += ubNumAdmins;
	*pubNumTroops += ubNumTroops;
	*pubNumElites += ubNumElites;
}

static bool IsAnyOfTeamOKInSector(INT8 const team)
{
	CFOR_EACH_IN_TEAM(i, team)
	{
		SOLDIERTYPE const& s = *i;
		if (s.bInSector && s.bLife >= OKLIFE) return true;
	}
	return false;
}


void EndTacticalBattleForEnemy()
{
	// Clear enemies in battle for all stationary groups in the sector
	if (gWorldSector.z == 0)
	{
		SECTORINFO& sector = SectorInfo[gWorldSector.AsByte()];
		sector.ubAdminsInBattle    = 0;
		sector.ubTroopsInBattle    = 0;
		sector.ubElitesInBattle    = 0;
		sector.ubNumCreatures      = 0;
		sector.ubCreaturesInBattle = 0;
	}
	else if (gWorldSector.z > 0)
	{
		UNDERGROUND_SECTORINFO& sector = *FindUnderGroundSector(gWorldSector);
		sector.ubAdminsInBattle = 0;
		sector.ubTroopsInBattle = 0;
		sector.ubElitesInBattle = 0;
	}
	else
	{ // Negative
		return; // XXX exception?
	}

	/* Clear this value so that profiled enemies can be added into battles in the
	 * future */
	gfProfiledEnemyAdded = FALSE;

	// Clear enemies in battle for all mobile groups in the sector
	CFOR_EACH_ENEMY_GROUP(i)
	{
		GROUP const& g = *i;
		if (g.fVehicle)       continue;
		if (g.ubSector != gWorldSector) continue;

		ENEMYGROUP& eg = *g.pEnemyGroup;
		eg.ubTroopsInBattle = 0;
		eg.ubElitesInBattle = 0;
		eg.ubAdminsInBattle = 0;
	}

	/* Check to see if any of our mercs have abandoned the militia during a
	 * battle. This is cause for a rather severe loyalty blow. */
	if (IsAnyOfTeamOKInSector(MILITIA_TEAM) &&
			(IsAnyOfTeamOKInSector(ENEMY_TEAM) || IsAnyOfTeamOKInSector(CREATURE_TEAM)))
	{
		HandleGlobalLoyaltyEvent(GLOBAL_LOYALTY_ABANDON_MILITIA, gWorldSector);
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

	if (gWorldSector.z > 0)
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
		FOR_EACH_ENEMY_GROUP(g)
		{
			if (g == bg)                               continue;
			if (g->fVehicle)                           continue;
			if (g->ubSector != bg->ubSector)           continue;
			if (g->pEnemyGroup->ubAdminsInBattle != 0) continue;
			if (g->pEnemyGroup->ubTroopsInBattle != 0) continue;
			if (g->pEnemyGroup->ubElitesInBattle != 0) continue;
			HandleArrivalOfReinforcements(g);
		}
		return;
	}

	if (gWorldSector.z == 0 && NumEnemiesInSector(gWorldSector) > 32)
	{
		gfPendingEnemies = TRUE;
	}

	int total_admins;
	int total_troops;
	int total_elites;
	SECTORINFO& sector = SectorInfo[gWorldSector.AsByte()];
	if (sector.uiFlags & SF_USE_MAP_SETTINGS)
	{ // Count the number of enemy placements in a map and use those
		total_admins = 0;
		total_troops = 0;
		total_elites = 0;
		CFOR_EACH_SOLDIERINITNODE(curr)
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
		STLOGE("The total stationary enemy forces in sector {} is {}. (max {})",
			  gWorldSector.AsShortString(), total_admins + total_troops + total_elites, 32);
		total_admins = std::min(total_admins, 32);
		total_troops = std::min(total_troops, 32 - total_admins);
		total_elites = std::min(total_elites, 32 - total_admins + total_troops);
	}

	sector.ubAdminsInBattle += total_admins;
	sector.ubTroopsInBattle += total_troops;
	sector.ubElitesInBattle += total_elites;

	// Search for movement groups that happen to be in the sector.
	INT16 n_slots = NumFreeEnemySlots();
	//Test:  All slots should be free at this point!
	if (n_slots != gTacticalStatus.Team[ENEMY_TEAM].bLastID - gTacticalStatus.Team[ENEMY_TEAM].bFirstID + 1)
	{
		SLOGE("All enemy slots should be free at this point.  Only %d of %d are available.\n\
			Trying to add %d admins, %d troops, and %d elites.",
			n_slots, gTacticalStatus.Team[ENEMY_TEAM].bLastID - gTacticalStatus.Team[ENEMY_TEAM].bFirstID + 1,
			total_admins, total_troops, total_elites);
	}
	/* Subtract the total number of stationary enemies from the available slots,
	 * as stationary forces take precendence in combat. The mobile forces that
	 * could also be in the same sector are considered later if all the slots fill
	 * up. */
	n_slots -= total_admins + total_troops + total_elites;
	/* Now, process all of the groups and search for both enemy and player groups
	 * in the sector. For enemy groups, we fill up the slots until we have none
	 * left or all of the groups have been processed. */
	FOR_EACH_GROUP(g)
	{
		if (n_slots == 0) break;

		if (g->fVehicle)         continue;
		if (gWorldSector.z != 0) continue;
		if (g->ubSector != gWorldSector) continue;

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
	CFOR_EACH_ENEMY_GROUP(g)
	{
		if (n_slots == 0) break;

		if (g->fVehicle) continue;
		if (gWorldSector.z != 0) continue;
		if (g->ubSector != gWorldSector) continue;

		INT32 n        = g->ubGroupSize;
		UINT8 n_admins = g->pEnemyGroup->ubAdminsInBattle;
		UINT8 n_troops = g->pEnemyGroup->ubTroopsInBattle;
		UINT8 n_elites = g->pEnemyGroup->ubElitesInBattle;
		FOR_EACH_IN_TEAM(s, ENEMY_TEAM)
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

}


static void PrepareEnemyForUndergroundBattle()
{
	// This is the sector we are going to be fighting in.
	UNDERGROUND_SECTORINFO* const u = FindUnderGroundSector(gWorldSector);
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
			if (!pSoldier->sSector.z)
			{
				SECTORINFO* pSector = &SectorInfo[pSoldier->sSector.AsByte()];
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
				pUnderground = FindUnderGroundSector(pSoldier->sSector);
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

	if( pSoldier->bNeutral || (pSoldier->bTeam != ENEMY_TEAM && pSoldier->bTeam != CREATURE_TEAM) )
		return;
	//we are recording an enemy death
	if( pSoldier->ubGroupID )
	{ //The enemy was in a mobile group
		GROUP *pGroup;
		pGroup = GetGroup( pSoldier->ubGroupID );
		if( !pGroup )
		{
			SLOGW("Enemy soldier killed with ubGroupID of {}, and the group doesn't exist!", pSoldier->ubGroupID);
			return;
		}
		if( pGroup->fPlayer )
		{
			SLOGW("Attempting to process player group thinking it's an enemy group (#{}) in ProcessQueenCmdImplicationsOfDeath()", pSoldier->ubGroupID);
			return;
		}
		switch( pSoldier->ubSoldierClass )
		{
			case SOLDIER_CLASS_ELITE:
				if( !pGroup->pEnemyGroup->ubNumElites )
				{
					SLOGW("Enemy elite killed with ubGroupID of {}, but the group doesn't contain elites!", pGroup->ubGroupID);
					break;
				}
				if( guiCurrentScreen == GAME_SCREEN )
				{
					if( (pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE && pGroup->pEnemyGroup->ubNumElites != pGroup->pEnemyGroup->ubElitesInBattle && !gfPendingEnemies) ||
						pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE || pGroup->pEnemyGroup->ubNumElites > 50 || pGroup->pEnemyGroup->ubElitesInBattle > 50 )
					{
						SLOGW("Group elite counters are bad. What were the last 2-3 things you did, and how?\n\
							Pleas save the game and file a bug.");
					}
				}
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
				if( !pGroup->pEnemyGroup->ubNumTroops )
				{
					SLOGW("Enemy troop killed with ubGroupID of {}, but the group doesn't contain troops!", pGroup->ubGroupID);
					break;
				}
				if( guiCurrentScreen == GAME_SCREEN )
				{
					if( (pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE && pGroup->pEnemyGroup->ubNumTroops != pGroup->pEnemyGroup->ubTroopsInBattle && !gfPendingEnemies) ||
						pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE || pGroup->pEnemyGroup->ubNumTroops > 50 || pGroup->pEnemyGroup->ubTroopsInBattle > 50 )
					{
						SLOGW("Group troop counters are bad. What were the last 2-3 things you did, and how?\n\
							Please save the game and file a bug.");
					}
				}
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
				if( !pGroup->pEnemyGroup->ubNumAdmins )
				{
					SLOGW("Enemy administrator killed with ubGroupID of {}, but the group doesn't contain administrators!", pGroup->ubGroupID);
				}
				if( guiCurrentScreen == GAME_SCREEN )
				{
					if( (pGroup->ubGroupSize <= MAX_STRATEGIC_TEAM_SIZE && pGroup->pEnemyGroup->ubNumAdmins != pGroup->pEnemyGroup->ubAdminsInBattle && !gfPendingEnemies) ||
					pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE || pGroup->pEnemyGroup->ubNumAdmins > 50 || pGroup->pEnemyGroup->ubAdminsInBattle > 50 )
					{
						SLOGW("Group admin counters are bad. What were the last 2-3 things you did, and how?\n\
							Please save the game and file a bug.");
					}
				}
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
		RecalculateGroupWeight(*pGroup);
		if( !pGroup->ubGroupSize )
		{
			RemoveGroup(*pGroup);
		}
	}
	else
	{ //The enemy was in a stationary defence group
		if (!gWorldSector.z || IsAutoResolveActive())
		{ //ground level (SECTORINFO)
			SECTORINFO *pSector;
			UINT32 ubTotalEnemies;

			if( !IsAutoResolveActive() )
			{
				pSector = &SectorInfo[pSoldier->sSector.AsByte()];
			}
			else
			{
				pSector = &SectorInfo[ GetAutoResolveSectorID() ];
			}

			ubTotalEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;

			switch( pSoldier->ubSoldierClass )
			{
				case SOLDIER_CLASS_ADMINISTRATOR:
					if( guiCurrentScreen == GAME_SCREEN )
					{
						if( (ubTotalEnemies <= 32 && pSector->ubNumAdmins != pSector->ubAdminsInBattle) ||
							!pSector->ubNumAdmins || !pSector->ubAdminsInBattle ||
							pSector->ubNumAdmins > 100 || pSector->ubAdminsInBattle > 32 )
						{
							SLOGW("Sector admin counters are bad. What were the last 2-3 things you did, and how?\n\
								Please save the game and file a bug.");
						}
					}
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
					if( guiCurrentScreen == GAME_SCREEN )
					{
						if( (ubTotalEnemies <= 32 && pSector->ubNumTroops != pSector->ubTroopsInBattle) ||
							!pSector->ubNumTroops || !pSector->ubTroopsInBattle ||
							pSector->ubNumTroops > 100 || pSector->ubTroopsInBattle > 32 )
						{
							SLOGW("Sector troop counters are bad. What were the last 2-3 things you did, and how?\n\
								Please save the game and file a bug.");
						}
					}
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
					if( guiCurrentScreen == GAME_SCREEN )
					{
						if( (ubTotalEnemies <= 32 && pSector->ubNumElites != pSector->ubElitesInBattle) ||
							!pSector->ubNumElites || !pSector->ubElitesInBattle ||
							pSector->ubNumElites > 100 || pSector->ubElitesInBattle > 32 )
						{
							SLOGW("Sector elite counters are bad. What were the last 2-3 things you did, and how?\n\
								Please save the game and file a bug");
						}
					}
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
						if( guiCurrentScreen == GAME_SCREEN )
						{
							if( (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumCreatures != pSector->ubCreaturesInBattle) ||
								!pSector->ubNumCreatures || !pSector->ubCreaturesInBattle ||
								pSector->ubNumCreatures > 50 || pSector->ubCreaturesInBattle > 50 )
							{
								SLOGW("Sector creature counters are bad. What were the last 2-3 things you did, and how?\n\
									Please save game and file a bug.");
							}
						}
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
			RecalculateSectorWeight(pSoldier->sSector.AsByte());
		}
		else
		{ //basement level (UNDERGROUND_SECTORINFO)
			UNDERGROUND_SECTORINFO* pSector = FindUnderGroundSector(gWorldSector);
			UINT32 ubTotalEnemies = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
			if( pSector )
			{
				switch( pSoldier->ubSoldierClass )
				{
					case SOLDIER_CLASS_ADMINISTRATOR:
						if( (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumAdmins != pSector->ubAdminsInBattle) ||
							!pSector->ubNumAdmins || !pSector->ubAdminsInBattle ||
							pSector->ubNumAdmins > 100 || pSector->ubAdminsInBattle > MAX_STRATEGIC_TEAM_SIZE )
						{
							SLOGW("Underground sector admin counters are bad. What were the last 2-3 things you did, and how?\n\
								Please save game and file a bug.");
						}
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
						if( (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumTroops != pSector->ubTroopsInBattle) ||
							!pSector->ubNumTroops || !pSector->ubTroopsInBattle ||
							pSector->ubNumTroops > 100 || pSector->ubTroopsInBattle > MAX_STRATEGIC_TEAM_SIZE )
						{
							SLOGW("Underground sector troop counters are bad. What were the last 2-3 things you did, and how?\n\
								Please save game and file a bug.");
						}
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
						if( (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumElites != pSector->ubElitesInBattle) ||
							!pSector->ubNumElites || !pSector->ubElitesInBattle ||
							pSector->ubNumElites > 100 || pSector->ubElitesInBattle > MAX_STRATEGIC_TEAM_SIZE )
						{
							SLOGW("Underground sector elite counters are bad. What were the last 2-3 things you did, and how?\n\
								Please save game and file a bug.");
						}
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
						if( (ubTotalEnemies <= MAX_STRATEGIC_TEAM_SIZE && pSector->ubNumCreatures != pSector->ubCreaturesInBattle) ||
							!pSector->ubNumCreatures || !pSector->ubCreaturesInBattle ||
							pSector->ubNumCreatures > 50 || pSector->ubCreaturesInBattle > 50 )
						{
							SLOGW("Underground sector creature counters are bad. What were the last 2-3 things you did, and how?\n\
								Please save game and file a bug.");
						}
						if( pSector->ubNumCreatures )
						{
							pSector->ubNumCreatures--;
						}
						if( pSector->ubCreaturesInBattle )
						{
							pSector->ubCreaturesInBattle--;
						}

						const SGPSector sectorJ9(9, 10);
						if (!pSector->ubNumCreatures && gWorldSector != sectorJ9)
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
							EndQuest(QUEST_CREATURES, gWorldSector);
						}
						break;
				}
			}
		}
	}
}


static void AddEnemiesToBattle(GROUP const&, UINT8 strategic_insertion_code, UINT8 n_admins, UINT8 n_troops, UINT8 n_elites);


/* Rarely, there will be more enemies than supported by the engine. In this
 * case, these soldier's are waiting for a slot to be free so that they can
 * enter the battle. This essentially allows for an infinite number of troops,
 * though only 32 at a time can fight. This is also called whenever an enemy
 * group's reinforcements arrive because the code is identical, though it is
 * highly likely that they will all be successfully added on the first call. */
void AddPossiblePendingEnemiesToBattle()
{
	if (!gfPendingEnemies)
	{ /* Optimization: No point in checking if we know that there aren't any more
		 * enemies that can be added to this battle. This changes whenever a new
		 * enemy group arrives at the scene. */
		return;
	}

	UINT8 n_slots = NumFreeEnemySlots();
	CFOR_EACH_ENEMY_GROUP(i)
	{
		if (n_slots == 0) break;

		GROUP const& g = *i;
		if (g.fVehicle)                   continue;
		if (gWorldSector.z != 0)          continue;
		if (g.ubSector != gWorldSector) continue;

		// This enemy group is currently in the sector.
		ENEMYGROUP& eg          = *g.pEnemyGroup;
		UINT8       n_elites    = 0;
		UINT8       n_troops    = 0;
		UINT8       n_admins    = 0;
		UINT8       n_available = g.ubGroupSize - eg.ubElitesInBattle - eg.ubTroopsInBattle - eg.ubAdminsInBattle;
		while (n_available != 0 && n_slots != 0)
		{ // This group has enemies waiting for a chance to enter the battle.
			if (eg.ubTroopsInBattle < eg.ubNumTroops)
			{ // Add a regular troop.
				++eg.ubTroopsInBattle;
				++n_troops;
			}
			else if (eg.ubElitesInBattle < eg.ubNumElites)
			{ // Add an elite troop.
				++eg.ubElitesInBattle;
				++n_elites;
			}
			else if (eg.ubAdminsInBattle < eg.ubNumAdmins)
			{ // Add an admin troop.
				++eg.ubAdminsInBattle;
				++n_admins;
			}
			else
			{
				throw std::logic_error("AddPossiblePendingEnemiesToBattle(): Logic Error");
			}
			--n_available;
			--n_slots;
		}

		if (n_admins != 0 || n_troops != 0 || n_elites != 0)
		{ /* This group has contributed forces, then add them now, because different
			 * groups appear on different sides of the map. */
			UINT8 strategic_insertion_code = 0;
			//First, determine which entrypoint to use, based on the travel direction of the group.
			if (g.ubPrev.IsValid())
			{
				strategic_insertion_code =
					g.ubSector.x < g.ubPrev.x ? INSERTION_CODE_EAST  :
					g.ubSector.x > g.ubPrev.x ? INSERTION_CODE_WEST  :
					g.ubSector.y < g.ubPrev.y ? INSERTION_CODE_SOUTH :
					g.ubSector.y > g.ubPrev.y ? INSERTION_CODE_NORTH :
					0; // XXX exception?
			}
			else if (g.ubNext.IsValid())
			{
				strategic_insertion_code =
					g.ubSector.x < g.ubNext.x ? INSERTION_CODE_EAST  :
					g.ubSector.x > g.ubNext.x ? INSERTION_CODE_WEST  :
					g.ubSector.y < g.ubNext.y ? INSERTION_CODE_SOUTH :
					g.ubSector.y > g.ubNext.y ? INSERTION_CODE_NORTH :
					0; // XXX exception?
			} // XXX else exception?
			/* Add the number of each type of troop and place them in the appropriate
			 * positions */
			AddEnemiesToBattle(g, strategic_insertion_code, n_admins, n_troops, n_elites);
		}
	}

	if (n_slots != 0)
	{ /* After going through the process, we have finished with some free slots
		 * and no more enemies to add. So, we can turn off the flag, as this check
		 * is no longer needed. */
		gfPendingEnemies = FALSE;
	}
}


static void AddEnemiesToBattle(GROUP const& g, UINT8 const strategic_insertion_code, UINT8 n_admins, UINT8 n_troops, UINT8 n_elites)
{
	SLOGD("Enemy reinforcements have arrived! ({} admins, {} troops, {} elite)",
				n_admins, n_troops, n_elites);

	UINT8 desired_direction;
	switch (strategic_insertion_code)
	{
		case INSERTION_CODE_NORTH: desired_direction = SOUTHEAST; break;
		case INSERTION_CODE_EAST:  desired_direction = SOUTHWEST; break;
		case INSERTION_CODE_SOUTH: desired_direction = NORTHWEST; break;
		case INSERTION_CODE_WEST:  desired_direction = NORTHEAST; break;
		default: throw std::logic_error("Invalid direction passed to AddEnemiesToBattle()");
	}

	UINT8            n_total   = n_admins + n_troops + n_elites;
	UINT8            curr_slot = 0;
	MAPEDGEPOINTINFO edgepoint_info;
	ChooseMapEdgepoints(&edgepoint_info, strategic_insertion_code, n_total);
	while (n_total != 0)
	{
		UINT32       const roll = Random(n_total--);
		SoldierClass const sc   =
			roll < n_elites            ? --n_elites, SOLDIER_CLASS_ELITE :
			roll < n_elites + n_troops ? --n_troops, SOLDIER_CLASS_ARMY  :
			(--n_admins, SOLDIER_CLASS_ADMINISTRATOR);

		SOLDIERTYPE& s = *TacticalCreateEnemySoldier(sc);
		s.ubGroupID = g.ubGroupID;
		s.ubInsertionDirection = desired_direction;
		// Setup the position
		if (curr_slot < edgepoint_info.ubNumPoints)
		{ // Use an edgepoint
			s.ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
			s.usStrategicInsertionData = edgepoint_info.sGridNo[curr_slot++];
		}
		else
		{ // No edgepoints left, so put him at the entrypoint
			s.ubStrategicInsertionCode = strategic_insertion_code;
		}
		UpdateMercInSector(s, gWorldSector);
	}
	Assert(n_admins == 0);
	Assert(n_troops == 0);
	Assert(n_elites == 0);
}


void SaveUnderGroundSectorInfoToSaveGame(HWFILE const f)
{
	// Save the number of nodes
	UINT32 n_records = 0;
	for (UNDERGROUND_SECTORINFO const* i = gpUndergroundSectorInfoHead; i; i = i->next)
	{
		++n_records;
	}
	f->write(&n_records, sizeof(UINT32));

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
	f->read(&n_records, sizeof(UINT32));

	UNDERGROUND_SECTORINFO** anchor = &gpUndergroundSectorInfoHead;
	for (UINT32 n = n_records; n != 0; --n)
	{
		UNDERGROUND_SECTORINFO* const u = new UNDERGROUND_SECTORINFO{};
		ExtractUndergroundSectorInfoFromFile(f, u);

		gpUndergroundSectorInfoTail = u;
		*anchor = u;
		anchor  = &u->next;
	}
}


UNDERGROUND_SECTORINFO* FindUnderGroundSector(const SGPSector& sector)
{
	UNDERGROUND_SECTORINFO* i = gpUndergroundSectorInfoHead;
	for (; i; i = i->next)
	{
		if (i->ubSector != sector) continue;
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
			StartQuest(QUEST_HELD_IN_ALMA, gWorldSector);
		}
		// CJC Dec 1 2002: fixing multiple captures:
		//else if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE )
		else if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE && gubQuest[ QUEST_INTERROGATION ] == QUESTNOTSTARTED )
		{
			StartQuest(QUEST_INTERROGATION, gWorldSector);
			// CJC Dec 1 2002: fixing multiple captures:
			gStrategicStatus.uiFlags |= STRATEGIC_PLAYER_CAPTURED_FOR_ESCAPE;

			ScheduleMeanwhileEvent(SGPSector(7, 14), 0, INTERROGATION, QUEEN, 10);
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


static void CaptureSoldier(SOLDIERTYPE* const s, const SGPSector& sMap, GridNo const soldier_pos, GridNo const item_pos)
{
	s->sSector                  = sMap;
	s->bLevel                   = 0; // put him on the floor
	s->ubStrategicInsertionCode = INSERTION_CODE_GRIDNO;
	s->usStrategicInsertionData = soldier_pos;

	// Drop all items
	FOR_EACH_SOLDIER_INV_SLOT(i, *s)
	{
		OBJECTTYPE& o = *i;
		if (o.usItem == NOTHING) continue;

		AddItemsToUnLoadedSector(sMap, item_pos, 1, &o, 0, 0, 0, VISIBILITY_0);
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
	iNumEnemiesInSector = NumEnemiesInSector(SGPSector(13, 9));

	// IF there are no enemies, and we need to do alma, skip!
	if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTNOTSTARTED && iNumEnemiesInSector == 0 )
	{
		InternalStartQuest(QUEST_HELD_IN_ALMA, gWorldSector, FALSE);
		InternalEndQuest(QUEST_HELD_IN_ALMA, gWorldSector, FALSE);
	}

	HandleMoraleEvent(pSoldier, MORALE_MERC_CAPTURED, pSoldier->sSector);

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
		CaptureSoldier(pSoldier, SGPSector(13, 9), sAlmaCaptureGridNos[idx], sAlmaCaptureItemsGridNo[idx]);
		++idx;
	}
	else if ( gubQuest[ QUEST_HELD_IN_ALMA ] == QUESTDONE )
	{
		// Teleport him to N7
		UINT8& idx = gStrategicStatus.ubNumCapturedForRescue;
		CaptureSoldier(pSoldier, SGPSector(7, 14), gsInterrogationGridNo[idx], sInterrogationItemGridNo[idx]);
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
	return FindPlayerMovementGroupInSector(SGPSector(ubSectorID)) != nullptr;
}


static BOOLEAN AnyNonNeutralOfTeamInSector(INT8 team)
{
	CFOR_EACH_IN_TEAM(s, team)
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
