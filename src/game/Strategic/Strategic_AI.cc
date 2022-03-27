#include "Strategic_AI.h"
#include "ArmyCompositionModel.h"
#include "CacheSectorsModel.h"
#include "Campaign.h"
#include "Campaign_Init.h"
#include "Campaign_Types.h"
#include "ContentManager.h"
#include "Debug.h"
#include "Facts.h"
#include "FileMan.h"
#include "Game_Clock.h"
#include "Game_Event_Hook.h"
#include "Game_Init.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Interface_Dialogue.h"
#include "Logger.h"
#include "Map_Information.h"
#include "MapScreen.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "Player_Command.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Random.h"
#include "SAM_Sites.h"
#include "Scheduling.h"
#include "SGPFile.h"
#include "Soldier_Control.h"
#include "Soldier_Profile.h"
#include "Soldier_Tile.h"
#include "Strategic_Movement.h"
#include "Strategic_Movement_Costs.h"
#include "Strategic_Pathing.h"
#include "StrategicAIPolicy.h"
#include "StrategicMap.h"
#include "Town_Militia.h"
#include <vector>

#define SAI_VERSION		29

/*
STRATEGIC AI -- UNDERLYING PHILOSOPHY
The most fundamental part of the strategic AI which takes from reality and gives to gameplay is the manner
the queen attempts to take her towns back.  Finances and owning mines are the most important way
to win the game.  As the player takes more mines over, the queen will focus more on quality and defense.  In
the beginning of the game, she will focus more on offense than mid-game or end-game.

REALITY
The queen owns the entire country, and the player starts the game with a small lump of cash, enough to hire
some mercenaries for about a week.  In that week, the queen may not notice what is going on, and the player
would believably take over one of the towns before she could feasibly react.  As soon as her military was
aware of the situation, she would likely proceed to send 300-400 troops to annihilate the opposition, and the
game would be over relatively quickly.  If the player was a prodigy, and managed to hold the town against such
a major assault, he would probably lose in the long run being forced into a defensive position and running out
of money quickly while the queen could continue to pump out the troops.  On the other hand, if the player
somehow managed to take over most of the mines, he would be able to casually walk over the queen eventually
just from the sheer income allowing him to purchase several of the best mercs.  That would have the effect of
making the game impossibly difficult in the beginning of the game, and a joke at the end (this is very much
like Master Of Orion II on the more difficult settings )

GAMEPLAY
Because we want the game to be like a normal game and make it fun, we need to make the game easy in the
beginning and harder at the end.  In order to accomplish this, I feel that pure income shouldn't be the factor
for the queen, because she would likely crucify a would-be leader in his early days.  So, in the beginning of
the game, the forces would already be situated with the majority of forces being the administrators in the towns,
and army troops and elites in the more important sectors.  Restricting the queen's offensive
abilities using a distance penalty would mean that the furthest sectors from the queen's palace would be
much easier to defend because she would only be allowed to send x number of troops.  As you get closer to the
queen, she would be allowed to send larger forces to attack those towns in question.  Also, to further
increase the games difficulty as the campaign progresses in the player's favor, we could also increase the
quality of the queen's troops based purely on the peek progress percentage.  This is calculated using a formula
that determines how well the player is doing by combining loyalty of towns owned, income generated, etc.  So,
in the beginning of the game, the quality is at the worst, but once you capture your first mines/towns, it
permanently  increase the queen's quality rating, effectively bumping up the stakes.  By the time you capture
four or five mines, the queen is going to focus more (but not completely) on quality defense as she prepares
for your final onslaught.  This quality rating will augment the experience level, equipment rating, and/or
attribute ratings of the queen's troops.  I would maintain a table of these enhancements based on the current
quality rating hooking into the difficulty all along.

//EXPLANATION OF THE WEIGHT SYSTEM:
The strategic AI has two types of groups:  garrisons and patrol groups.  Each of these groups contain
information of it's needs, mainly desired population.  If the current population is greater than the
desired population, and the group will get a negative weight assigned to it, which means that it is willing
to give up troops to areas that need them more.  On the other hand, if a group has less than the desired population,
then the weight will be positive, meaning they are requesting reinforcements.

The weight generated will range between -100 and +100.  The calculated weight is modified by the priority
of the group.  If the priority of the group is high, they
*/

BOOLEAN gfAutoAIAware = FALSE;

//Saved vars
BOOLEAN      gfExtraElites = 0;			//Set when queen compositions are augmented with bonus elites.
INT32        giForcePercentage = 0;		//Modifies the starting group sizes relative by percentage
INT32        giArmyAlertness = 0;		//The chance the group will spot an adjacent player/militia
INT32        giArmyAlertnessDecay = 0;		//How much the spotting chance decreases when spot check succeeds
UINT8        gubNumAwareBattles = 0;		//When non-zero, this means the queen is very aware and searching for players.  Every time
						//there is an enemy initiated battle, this counter decrements until zero.  Until that point,
						//all adjacent sector checks automatically succeed.
BOOLEAN gfQueenAIAwake = FALSE;			//This flag turns on/off the strategic decisions.  If it's off, no reinforcements
						//or assaults will happen.
						//@@@Alex, this flag is ONLY set by the first meanwhile scene which calls an action.  If this
						//action isn't called, the AI will never turn on.  It is completely dependant on this action.  It can
						//be toggled at will in the AIViewer for testing purposes.
INT32        giReinforcementPool = 0;		//How many troops the queen has in reserve in noman's land.  These guys are spawned as needed in P3.
INT32        giReinforcementPoints = 0;		//the entire army's capacity to provide reinforcements.
INT32        giRequestPoints = 0;		//the entire army's need for reinforcements.
UINT8        gubSAIVersion = SAI_VERSION;	//Used for adding new features to be saved.
UINT8        gubQueenPriorityPhase = 0;		//Defines how far into defence the queen is -- abstractly related to defcon index ranging from 0-10.
																			//10 is the most defensive
//Used for authorizing the use of the first battle meanwhile scene AFTER the battle is complete.  This is the case used when
//the player attacks a town, and is set once militia are sent to investigate.
BOOLEAN gfFirstBattleMeanwhileScenePending = FALSE;

//After the first battle meanwhile scene is finished, this flag is set, and the queen orders patrol groups to immediately fortify all towns.
BOOLEAN gfMassFortificationOrdered = FALSE;

UINT8   gubMinEnemyGroupSize     = 0;
UINT8   gubHoursGracePeriod      = 0;
UINT16  gusPlayerBattleVictories = 0;
BOOLEAN gfUseAlternateQueenPosition = FALSE;

//padding for generic globals
#define SAI_PADDING_BYTES		97
INT8		gbPadding[SAI_PADDING_BYTES]; // XXX HACK000B
//patrol group info plus padding
std::vector<PATROL_GROUP> gPatrolGroup;
//army composition info plus padding
std::vector<ARMY_COMPOSITION> gArmyComp;
//garrison info plus padding
std::vector<GARRISON_GROUP> gGarrisonGroup;

extern UINT8 gubNumGroupsArrivedSimultaneously;

//This refers to the number of force points that are *saved* for the AI to use.  This is basically an array of each
//group.  When the queen wants to send forces to attack a town that is defended, the initial number of forces that
//she would send would be considered too weak.  So, instead, she will send that force to the sector's adjacent sector,
//and stage, while
UINT8 *gubGarrisonReinforcementsDenied = NULL;
UINT8 *gubPatrolReinforcementsDenied = NULL;

//Unsaved vars
BOOLEAN gfDisplayStrategicAILogs = FALSE;

extern INT16 sWorldSectorLocationOfFirstBattle;
static const SGPSector meduna(3, 16);

//returns the number of reinforcements permitted to be sent.  Will increased if the denied counter is non-zero.
static INT32 GarrisonReinforcementsRequested(INT32 iGarrisonID, UINT8* pubExtraReinforcements)
{
	INT32 iReinforcementsRequested;
	INT32 iExistingForces;
	SECTORINFO *pSector;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iExistingForces = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iReinforcementsRequested = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation - iExistingForces;

	//Record how many of the reinforcements are additionally provided due to being denied in the past.  This will grow
	//until it is finally excepted or an absolute max is made.
	*pubExtraReinforcements = (UINT8)(gubGarrisonReinforcementsDenied[ iGarrisonID ] / (6 - gGameOptions.ubDifficultyLevel));
	//Make sure the number of extra reinforcements don't bump the force size past the max of MAX_STRATEGIC_TEAM_SIZE.
	*pubExtraReinforcements = (UINT8)MIN( (INT32)*pubExtraReinforcements, MIN( (INT32)(*pubExtraReinforcements), MAX_STRATEGIC_TEAM_SIZE - iReinforcementsRequested ) );

	iReinforcementsRequested = MIN( MAX_STRATEGIC_TEAM_SIZE, iReinforcementsRequested );

	return iReinforcementsRequested;
}


static INT32 PatrolReinforcementsRequested(PATROL_GROUP const* const pg)
{
	GROUP* const g = GetGroup(pg->ubGroupID);
	INT32 size = pg->bSize;
	if (g) size -= g->ubGroupSize;
	return size;
}


static INT32 ReinforcementsAvailable(INT32 iGarrisonID)
{
	SECTORINFO *pSector;
	INT32 iReinforcementsAvailable;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iReinforcementsAvailable = pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins;
	iReinforcementsAvailable -= gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;

	switch( gGarrisonGroup[ iGarrisonID ].ubComposition )
	{
		case LEVEL1_DEFENCE:
		case LEVEL2_DEFENCE:
		case LEVEL3_DEFENCE:
		case ALMA_DEFENCE:
		case ALMA_MINE:
			//Legal spawning locations
			break;
		default:
			//No other sector permitted to send surplus troops
			return 0;
	}

	return iReinforcementsAvailable;
}


static BOOLEAN PlayerForceTooStrong(UINT8 ubSectorID, UINT16 usOffensePoints, UINT16* pusDefencePoints)
{
	SECTORINFO *pSector;
	SGPSector sSector(ubSectorID);
	pSector = &SectorInfo[ ubSectorID ];

	*pusDefencePoints = pSector->ubNumberOfCivsAtLevel[ GREEN_MILITIA ]		* 1 +
											pSector->ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] * 2 +
											pSector->ubNumberOfCivsAtLevel[ ELITE_MILITIA ]		* 3 +
											PlayerMercsInSector(sSector) * 5;
	if( *pusDefencePoints > usOffensePoints )
	{
		return TRUE;
	}
	return FALSE;
}


static void SendReinforcementsForGarrison(INT32 iDstGarrisonID, UINT16 usDefencePoints, GROUP** pOptionalGroup);


static void RequestAttackOnSector(UINT8 ubSectorID, UINT16 usDefencePoints)
{
	UINT32 i;
	for( i = 0; i < gGarrisonGroup.size(); i++ )
	{
		if( gGarrisonGroup[ i ].ubSectorID == ubSectorID && !gGarrisonGroup[ i ].ubPendingGroupID )
		{
			SLOGD("An attack has been requested in sector %c%d.",
					SECTORY( ubSectorID ) + 'A' - 1, SECTORX( ubSectorID ) );
			SendReinforcementsForGarrison( i, usDefencePoints, NULL );
			return;
		}
	}
}


static bool AdjacentSectorIsImportantAndUndefended(UINT8 const sector_id)
{
	switch (StrategicMap[SECTOR_INFO_TO_STRATEGIC_INDEX(sector_id)].bNameId)
	{
		case OMERTA:
		case SAN_MONA:
		case ESTONI:
			// These towns aren't important.
			return false;
	}
	SECTORINFO const& si = SectorInfo[sector_id];
	return
		si.ubNumTroops == 0                                 &&
		si.ubNumElites == 0                                 &&
		si.ubNumAdmins == 0                                 &&
		si.ubTraversability[THROUGH_STRATEGIC_MOVE] == TOWN &&
		!PlayerSectorDefended(sector_id);
}


static void ReassignAIGroup(GROUP** pGroup);


static void ValidateGroup(GROUP* pGroup)
{
	if (!pGroup->ubSector.IsValid())
	{
		if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
		{
			STLOGE("Internal error (invalid enemy group #{} location at {}, destination {}).  Please send PRIOR save file and Debug Log.",
				pGroup->ubGroupID, pGroup->ubSector.AsShortString(), pGroup->ubNext.AsShortString());
			RemoveGroupFromStrategicAILists(*pGroup);
			RemoveGroup(*pGroup);
			return;
		}
	}
	if (!pGroup->ubNext.IsValid())
	{
		if( !pGroup->fPlayer && pGroup->pEnemyGroup->ubIntention != STAGING
			&& pGroup->pEnemyGroup->ubIntention != REINFORCEMENTS )
		{
			SLOGE( "Internal error (floating group).  Please send PRIOR save file and Debug Log." );
			if( gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
			{
				RemoveGroupFromStrategicAILists(*pGroup);
				ReassignAIGroup( &pGroup );
				return;
			}
		}
	}
	if( pGroup->pEnemyGroup->ubNumAdmins + pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites != pGroup->ubGroupSize ||
			pGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE )
		{
			SLOGE( "Internal error (bad group populations).  Please send PRIOR save file and Debug Log." );
		}
}


static void ValidateLargeGroup(GROUP* pGroup)
{
		if( pGroup->ubGroupSize > 25 )
		{
			STLOGW("warning:  Enemy group containing {} soldiers\n\
				({} admins, {} troops, {} elites) in sector {}.  This message is a temporary test message\n\
				to evaluate a potential problems with very large enemy groups.",
				pGroup->ubGroupSize, pGroup->pEnemyGroup->ubNumAdmins, pGroup->pEnemyGroup->ubNumTroops, pGroup->pEnemyGroup->ubNumElites,
				pGroup->ubSector.AsShortString());
		}
}


void InitStrategicAI()
{
	gfExtraElites                      = FALSE;
	gubNumAwareBattles                 = 0;
	gfQueenAIAwake                     = FALSE;
	giReinforcementPoints              = 0;
	giRequestPoints                    = 0;
	gubSAIVersion                      = SAI_VERSION;
	gubQueenPriorityPhase              = 0;
	gfFirstBattleMeanwhileScenePending = FALSE;
	gfMassFortificationOrdered         = FALSE;
	gusPlayerBattleVictories           = 0;
	gfUseAlternateQueenPosition        = FALSE;

	// 475 is 7:55am in minutes since midnight, the time the game starts on day 1
	UINT32      evaluate_time = 475;
	UINT8 const difficulty    = gGameOptions.ubDifficultyLevel;

	giReinforcementPool   = saipolicy_by_diff(queens_pool_of_troops);
	giForcePercentage     = saipolicy_by_diff(initial_garrison_percentages);
	giArmyAlertness       = saipolicy_by_diff(enemy_starting_alert_level);
	giArmyAlertnessDecay  = saipolicy_by_diff(enemy_starting_alert_decay);
	gubMinEnemyGroupSize  = saipolicy_by_diff(min_enemy_group_size);
	gubHoursGracePeriod   = saipolicy_by_diff(grace_period_in_hours);
	evaluate_time        += saipolicy_by_diff(time_evaluate_in_minutes) + Random(saipolicy_by_diff(time_evaluate_variance));

	AddStrategicEvent(EVENT_EVALUATE_QUEEN_SITUATION, evaluate_time, 0);

	//Initialize the sectorinfo structure so all sectors don't point to a garrisonID.
	FOR_EACH(SECTORINFO, i, SectorInfo)
	{
		i->ubGarrisonID = NO_GARRISON;
	}

	/* Copy over the original army composition as it does get modified during the
	 * campaign. This bulletproofs starting the game over again. */
	gArmyComp = GCM->getArmyCompositions();

	// Eliminate more perimeter defenses on the easier levels.
	switch (difficulty)
	{
		case DIF_LEVEL_EASY:
			gArmyComp[LEVEL2_DEFENCE].bDesiredPopulation = 0;
			gArmyComp[LEVEL2_DEFENCE].bStartPopulation   = 0;
			/* FALLTHROUGH */
		case DIF_LEVEL_MEDIUM:
			gArmyComp[LEVEL3_DEFENCE].bDesiredPopulation = 0;
			gArmyComp[LEVEL3_DEFENCE].bStartPopulation   = 0;
			break;
	}

	// Initialize the patrol group definitions
	gPatrolGroup = GCM->getPatrolGroups();
	gubPatrolReinforcementsDenied = new UINT8[gPatrolGroup.size()]{};

	// Initialize the garrison group definitions
	auto origGarrisonGroups = GCM->getGarrisonGroups();
	size_t uiGarrisonArraySize = origGarrisonGroups.size();
	gGarrisonGroup = origGarrisonGroups;

	gubGarrisonReinforcementsDenied = new UINT8[uiGarrisonArraySize]{};

	// Modify initial force sizes?
	INT32 const force_percentage = giForcePercentage;
	if (force_percentage != 100)
	{ /* The initial force sizes are being modified, so go through each of the
		 * army compositions and adjust them accordingly. */
		for (size_t i = 0; i != gArmyComp.size(); ++i)
		{
			ARMY_COMPOSITION& a = gArmyComp[i];
			if (i != QUEEN_DEFENCE)
			{
				a.bDesiredPopulation = MIN(MAX_STRATEGIC_TEAM_SIZE, a.bDesiredPopulation * force_percentage / 100);
				if (a.bStartPopulation != MAX_STRATEGIC_TEAM_SIZE)
				{ /* If the value is MAX_STRATEGIC_TEAM_SIZE, then that means the
					 * particular sector is a spawning location. Don't modify the value if
					 * it is MAX_STRATEGIC_TEAM_SIZE. Everything else is game. */
					a.bStartPopulation = MIN(MAX_STRATEGIC_TEAM_SIZE, a.bStartPopulation * force_percentage / 100);
				}
			}
			else
			{
				a.bDesiredPopulation = MIN(32, a.bDesiredPopulation * force_percentage / 100);
				a.bStartPopulation   = a.bDesiredPopulation;
			}
		}
		for (size_t i = 0; i != gPatrolGroup.size(); ++i)
		{ // Force modified range within 1 - MAX_STRATEGIC_TEAM_SIZE.
			INT8& size = gPatrolGroup[i].bSize;
			size = MAX(gubMinEnemyGroupSize, MIN(MAX_STRATEGIC_TEAM_SIZE, size * force_percentage / 100));
		}
	}

	/* Initialize the garrisons based on the initial sizes (all variances are plus
	 * or minus 1). */
	for (UINT32 i = 0; i != uiGarrisonArraySize; ++i)
	{
		GARRISON_GROUP& gg = gGarrisonGroup[i];
		SECTORINFO&     si = SectorInfo[gg.ubSectorID];
		si.ubGarrisonID = i;
		ARMY_COMPOSITION const& ac = gArmyComp[gg.ubComposition];
		INT32       start_pop    = ac.bStartPopulation;
		INT32 const desired_pop  = ac.bDesiredPopulation;
		INT32 const iPriority    = ac.bPriority;
		INT32 const elite_chance = ac.bElitePercentage;
		INT32 const troop_chance = ac.bTroopPercentage + elite_chance;
		INT32 const admin_chance = ac.bAdminPercentage;

		switch (gg.ubComposition)
		{
			case ROADBLOCK:
				si.uiFlags |= SF_ENEMY_AMBUSH_LOCATION;
				start_pop   = Chance(20) ? ac.bDesiredPopulation : 0;
				break;

			case SANMONA_SMALL:
				start_pop = 0; // Not appropriate until Kingpin is killed.
				break;
		}

		if (start_pop != 0)
		{
			if (gg.ubSectorID != SEC_P3)
			{
				// if population is less than maximum
				if (start_pop != MAX_STRATEGIC_TEAM_SIZE)
				{
					// then vary it a bit (+/- 25%)
					start_pop = start_pop * (100 + Random(51) - 25) / 100;
				}

				start_pop = MAX(gubMinEnemyGroupSize, MIN(MAX_STRATEGIC_TEAM_SIZE, start_pop));
			}

			if (admin_chance != 0)
			{
				si.ubNumAdmins = admin_chance * start_pop / 100;
			}
			else for (INT32 cnt = start_pop; cnt != 0; --cnt)
			{ // For each soldier randomly determine the type.
				INT32 const roll = Random(100);
				if (roll < elite_chance)
				{
					++si.ubNumElites;
				}
				else if (roll < troop_chance)
				{
					++si.ubNumTroops;
				}
			}

			switch (gg.ubComposition)
			{
				case CAMBRIA_DEFENCE:
				case CAMBRIA_MINE:
				case ALMA_MINE:
				case GRUMM_MINE:
					// Fill up extra start slots with troops
					start_pop      -= si.ubNumAdmins;
					si.ubNumTroops  = start_pop;
					break;

				case DRASSEN_AIRPORT:
				case DRASSEN_DEFENCE:
				case DRASSEN_MINE:
					si.ubNumAdmins = MAX(5, si.ubNumAdmins);
					break;

				case TIXA_PRISON:
					si.ubNumAdmins = MAX(8, si.ubNumAdmins);
					break;
			}
		}

		if (admin_chance != 0 && si.ubNumAdmins < gubMinEnemyGroupSize)
		{
			si.ubNumAdmins = gubMinEnemyGroupSize;
		}

		/* Calculate weight (range is -20 to +20 before multiplier). The multiplier
		 * of 3 brings it to a range of -96 to +96 which is close enough to a
		 * plus/minus 100%. The resultant percentage is then converted based on the
		 * priority. */
		INT32 weight = (desired_pop - start_pop) * 3;
		if (weight > 0)
		{ // Modify it by its priority.
			// Generates a value between 2 and 100
			weight = weight * iPriority / 96;
			weight = MAX(weight, 2);
			giRequestPoints += weight;
		}
		else if (weight < 0)
		{ // Modify it by its reverse priority
			// Generates a value between -2 and -100
			weight = weight * (100 - iPriority) / 96;
			weight = MIN(weight, -2);
			giReinforcementPoints -= weight;
		}
		gg.bWeight = weight;

		/* Post an event which allows them to check adjacent sectors periodically.
		 * Spread them out so that they process at different times. */
		AddPeriodStrategicEventWithOffset(EVENT_CHECK_ENEMY_CONTROLLED_SECTOR, 140 - 20 * difficulty + Random(4), 475 + i, gg.ubSectorID);
	}

	// Initialize each of the patrol groups
	for (size_t i = 0; i != gPatrolGroup.size(); ++i)
	{
		PATROL_GROUP& pg = gPatrolGroup[i];
		UINT8 n_troops = pg.bSize + Random(3) - 1;
		n_troops = MAX(gubMinEnemyGroupSize, MIN(MAX_STRATEGIC_TEAM_SIZE, n_troops));
		/* Note on adding patrol groups: The patrol group can't actually start on
		 * the first waypoint, so we set it to the second way point for
		 * initialization, and then add the waypoints from 0 up */
		GROUP&      g  = *CreateNewEnemyGroupDepartingFromSector(pg.ubSectorID[1], 0, n_troops, 0);
		ENEMYGROUP& eg = *g.pEnemyGroup;

		if (i == 3 || i == 4)
		{ /* Special case: Two patrol groups are administrator groups -- rest are
			 * troops */
			eg.ubNumAdmins = eg.ubNumTroops;
			eg.ubNumTroops = 0;
		}
		pg.ubGroupID   = g.ubGroupID;
		eg.ubIntention = PATROL;
		g.ubMoveType   = ENDTOEND_FORWARDS;
		FOR_EACH(UINT8 const, i, pg.ubSectorID)
		{
			if (*i == 0) break;
			AddWaypointIDToPGroup(&g, *i);
		}
		RandomizePatrolGroupLocation(&g);
		ValidateGroup(&g);
	}

	/* Choose one cache map out of five possible maps. Select the sector randomly,
	 * set up the flags to use the alternate map, then place 8-12 regular troops
	 * there (no AI though). Changing MAX_STRATEGIC_TEAM_SIZE may require changes
	 * to to the defending force here. */
	auto model = GCM->getCacheSectors();
	INT16 sSectorID = model->pickSector();
	if (sSectorID >= 0)
	{
		SECTORINFO &si = SectorInfo[sSectorID];
		si.uiFlags |= SF_USE_ALTERNATE_MAP;
		si.ubNumTroops = model->getNumTroops(difficulty);
		STLOGD("Weapon cache is at {}", SECTOR_SHORT_STRING(sSectorID));
	}
}


void KillStrategicAI()
{
	gPatrolGroup.clear();
	gGarrisonGroup.clear();

	if( gubPatrolReinforcementsDenied )
	{
		delete[] gubPatrolReinforcementsDenied;
		gubPatrolReinforcementsDenied = NULL;
	}
	if( gubGarrisonReinforcementsDenied )
	{
		delete[] gubGarrisonReinforcementsDenied;
		gubGarrisonReinforcementsDenied = NULL;
	}
	DeleteAllStrategicEventsOfType( EVENT_EVALUATE_QUEEN_SITUATION );
}

BOOLEAN OkayForEnemyToMoveThroughSector( UINT8 ubSectorID )
{
	SECTORINFO *pSector;
	pSector = &SectorInfo[ ubSectorID ];
	if( pSector->uiTimeLastPlayerLiberated && pSector->uiTimeLastPlayerLiberated + (gubHoursGracePeriod * 3600) > GetWorldTotalSeconds() )
	{
		return FALSE;
	}
	return TRUE;
}


static BOOLEAN EnemyPermittedToAttackSector(GROUP** pGroup, UINT8 ubSectorID)
{
	SECTORINFO *pSector;
	BOOLEAN fPermittedToAttack = TRUE;

	pSector = &SectorInfo[ ubSectorID ];
	fPermittedToAttack = OkayForEnemyToMoveThroughSector( ubSectorID );
	if( pGroup && *pGroup && pSector->ubGarrisonID != NO_GARRISON )
	{
		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{
			GROUP *pPendingGroup;
			pPendingGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
			if( pPendingGroup == *pGroup )
			{
				if( fPermittedToAttack )
				{
					if( GroupAtFinalDestination( *pGroup ) )
					{ //High priority reinforcements have arrived.  This overrides most other situations.
						return TRUE;
					}
				}
				else
				{ //Reassign the group
					ReassignAIGroup( pGroup );
				}
			}
		}
	}
	if( !fPermittedToAttack )
	{
		return FALSE;
	}
	//If Hill-billies are alive, then enemy won't attack the sector.
	switch( ubSectorID )
	{
		case SEC_F10:
			//Hill-billy farm -- not until hill billies are dead.
			if (CheckFact(FACT_HILLBILLIES_KILLED, FALSE))
				return FALSE;
			break;
		case SEC_A9:
		case SEC_A10:
			//Omerta -- not until Day 2 at 7:45AM.
			if( GetWorldTotalMin() < 3345 )
				return FALSE;
			break;
		case SEC_B13:
		case SEC_C13:
		case SEC_D13:
			//Drassen -- not until Day 3 at 6:30AM.
			if( GetWorldTotalMin() < 4710 )
				return FALSE;
			break;
		case SEC_C5:
		case SEC_C6:
		case SEC_D5:
			//San Mona -- not until Kingpin is dead.
			return CheckFact(FACT_KINGPIN_DEAD, 0);

		case SEC_G1:
			if( PlayerSectorDefended( SEC_G2 ) && (PlayerSectorDefended( SEC_H1 ) || PlayerSectorDefended( SEC_H2 )) )
			{
				return FALSE;
			}
			break;
		case SEC_H2:
			if( PlayerSectorDefended( SEC_H2 ) && (PlayerSectorDefended( SEC_G1 ) || PlayerSectorDefended( SEC_G2 )) )
			{
				return FALSE;
			}
			break;
	}
	return TRUE;
}


enum SAIMOVECODE
{
	DIRECT,
	EVASIVE,
	STAGE
};
static void MoveSAIGroupToSector(GROUP**, UINT8 sector, SAIMOVECODE, UINT8 intention);


static BOOLEAN HandlePlayerGroupNoticedByPatrolGroup(const GROUP* const pPlayerGroup, GROUP* pEnemyGroup)
{
	UINT16 usDefencePoints;
	UINT16 usOffensePoints;

	UINT8 const ubSectorID = pPlayerGroup->ubSector.AsByte();
	usOffensePoints = pEnemyGroup->pEnemyGroup->ubNumAdmins * 2 +
										pEnemyGroup->pEnemyGroup->ubNumTroops * 4 +
										pEnemyGroup->pEnemyGroup->ubNumElites * 6;

	const UINT8 playerSector = pPlayerGroup->ubNext.x
			? pPlayerGroup->ubNext.AsByte()
			: ubSectorID;

	if(PlayerForceTooStrong(ubSectorID, usOffensePoints, &usDefencePoints)
	   || SectorInfo[playerSector].ubGarrisonID != NO_GARRISON)
	{
		RequestAttackOnSector(ubSectorID, usDefencePoints);
		return FALSE;
	}
	//For now, automatically attack.
	if (pPlayerGroup->ubNext.x)
	{
		MoveSAIGroupToSector( &pEnemyGroup, playerSector, DIRECT, PURSUIT );

		STLOGD("Enemy group at {} detected player group at {} and is moving to intercept them at {}.",
			pEnemyGroup->ubSector.AsShortString(), pPlayerGroup->ubSector.AsShortString(),
			pPlayerGroup->ubNext.AsShortString());
	}
	else
	{
		MoveSAIGroupToSector( &pEnemyGroup, playerSector, DIRECT, PURSUIT );

		STLOGD("Enemy group at {} detected player group at {} and is moving to intercept them at {}.",
					pEnemyGroup->ubSector.AsShortString(),
					pPlayerGroup->ubSector.AsShortString(),
					pPlayerGroup->ubSector.AsShortString());
	}
	return TRUE;
}


static void ConvertGroupTroopsToComposition(GROUP* pGroup, INT32 iCompositionID);
static void RemoveSoldiersFromGarrisonBasedOnComposition(INT32 iGarrisonID, UINT8 ubSize);


static void HandlePlayerGroupNoticedByGarrison(const GROUP* const pPlayerGroup, const UINT8 ubSectorID)
{
	SECTORINFO *pSector;
	GROUP *pGroup;
	INT32 iReinforcementsApproved;
	UINT16 usOffensePoints, usDefencePoints;
	UINT8 ubEnemies;
	pSector = &SectorInfo[ ubSectorID ];
	//First check to see if the player is at his final destination.
	if( !GroupAtFinalDestination( pPlayerGroup ) )
	{
		return;
	}
	usOffensePoints = pSector->ubNumAdmins * 2 +
										pSector->ubNumTroops * 4 +
										pSector->ubNumElites * 6;
	if( PlayerForceTooStrong( ubSectorID, usOffensePoints, &usDefencePoints ) )
	{
		RequestAttackOnSector( ubSectorID, usDefencePoints );
		return;
	}

	if( pSector->ubGarrisonID != NO_GARRISON )
	{
		//Decide whether or not they will attack them with some of the troops.
		ubEnemies = (UINT8)(pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites);
		iReinforcementsApproved = (ubEnemies - gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bDesiredPopulation / 2);
		if( iReinforcementsApproved*2 > pPlayerGroup->ubGroupSize*3 && iReinforcementsApproved > gubMinEnemyGroupSize )
		{ //Then enemy's available outnumber the player by at least 3:2, so attack them.
			pGroup = CreateNewEnemyGroupDepartingFromSector( ubSectorID, 0, (UINT8)iReinforcementsApproved, 0 );

			ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition );

			MoveSAIGroupToSector(&pGroup, pPlayerGroup->ubSector.AsByte(), DIRECT, REINFORCEMENTS);

			RemoveSoldiersFromGarrisonBasedOnComposition( pSector->ubGarrisonID, pGroup->ubGroupSize );

			if( pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins > MAX_STRATEGIC_TEAM_SIZE )
			{
				SLOGE("Sector %c%d now has %d enemies (max %d).",
						gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID / 16 + 'A' , gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID % 16,
						pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins, MAX_STRATEGIC_TEAM_SIZE );
			}

			STLOGD("Enemy garrison at {}{} detected stopped player group at {} and is sending {} troops to attack.",
					gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID / 16 + 'A' , gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID % 16,
					pPlayerGroup->ubSector.AsShortString(),
					pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumTroops );
		}
	}
}


static BOOLEAN HandleMilitiaNoticedByPatrolGroup(UINT8 ubSectorID, GROUP* pEnemyGroup)
{
	//For now, automatically attack.
	UINT16 usDefencePoints;

	SGPSector sSector((ubSectorID % 16) + 1, (ubSectorID / 16) + 1);
	UINT16 usOffensePoints = pEnemyGroup->pEnemyGroup->ubNumAdmins * 2 +
										pEnemyGroup->pEnemyGroup->ubNumTroops * 4 +
										pEnemyGroup->pEnemyGroup->ubNumElites * 6;
	if( PlayerForceTooStrong( ubSectorID, usOffensePoints, &usDefencePoints ) )
	{
		RequestAttackOnSector( ubSectorID, usDefencePoints );
		return FALSE;
	}

	MoveSAIGroupToSector(&pEnemyGroup, sSector.AsByte(), DIRECT, REINFORCEMENTS);
	STLOGD("Enemy group at {} detected militia at {}{} and is moving to attack them.",
			pEnemyGroup->ubSector.AsShortString(), sSector.AsShortString());
	return FALSE;
}


static BOOLEAN AttemptToNoticeEmptySectorSucceeds(void)
{
	if( gubNumAwareBattles || gfAutoAIAware )
	{ //The queen is in high-alert and is searching for players.  All adjacent checks will automatically succeed.
		return TRUE;
	}
	if( DayTime() )
	{ //Day time chances are normal
		if( Chance( giArmyAlertness ) )
		{
			giArmyAlertness -= giArmyAlertnessDecay;
			//Minimum alertness should always be at least 0.
			giArmyAlertness = MAX( 0, giArmyAlertness );
			return TRUE;
		}
		giArmyAlertness++;
		return FALSE;
	}
	//Night time chances are one third of normal.
	if( Chance( giArmyAlertness/3 ) )
	{
		giArmyAlertness -= giArmyAlertnessDecay;
		//Minimum alertness should always be at least 0.
		giArmyAlertness = MAX( 0, giArmyAlertness );
		return TRUE;
	}
	if( Chance( 33 ) )
	{
		giArmyAlertness++;
	}
	return FALSE;
}


//Calling the function assumes that a player group is found to be adjacent to an enemy group.
//This uses the alertness rating to emulate the chance that the group will notice.  If it does
//notice, then the alertness drops accordingly to simulate a period of time where the enemy would
//not notice as much.  If it fails, the alertness gradually increases until it succeeds.
static BOOLEAN AttemptToNoticeAdjacentGroupSucceeds(void)
{
	if( gubNumAwareBattles || gfAutoAIAware )
	{ //The queen is in high-alert and is searching for players.  All adjacent checks will automatically succeed.
		return TRUE;
	}
	if( DayTime() )
	{ //Day time chances are normal
		if( Chance( giArmyAlertness ) )
		{
			giArmyAlertness -= giArmyAlertnessDecay;
			//Minimum alertness should always be at least 0.
			giArmyAlertness = MAX( 0, giArmyAlertness );
			return TRUE;
		}
		giArmyAlertness++;
		return FALSE;
	}
	//Night time chances are one third of normal.
	if( Chance( giArmyAlertness/3 ) )
	{
		giArmyAlertness -= giArmyAlertnessDecay;
		//Minimum alertness should always be at least 0.
		giArmyAlertness = MAX( 0, giArmyAlertness );
		return TRUE;
	}
	if( Chance( 33 ) )
	{
		giArmyAlertness++;
	}
	return FALSE;
}


static BOOLEAN HandleEmptySectorNoticedByPatrolGroup(GROUP* pGroup, UINT8 ubEmptySectorID)
{
	SGPSector sSector((ubEmptySectorID % 16) + 1, (ubEmptySectorID / 16) + 1);
	UINT8 ubGarrisonID = SectorInfo[ubEmptySectorID].ubGarrisonID;
	if( ubGarrisonID != NO_GARRISON )
	{
		if( gGarrisonGroup[ ubGarrisonID ].ubPendingGroupID )
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	//Clear the patrol group's previous orders.
	RemoveGroupFromStrategicAILists(*pGroup);

	gGarrisonGroup[ ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
	MoveSAIGroupToSector(&pGroup, sSector.AsByte(), DIRECT, REINFORCEMENTS);
	STLOGD("Enemy group at {} detected undefended sector at {}{} and is moving to retake it.",
			pGroup->ubSector.AsShortString(), sSector.AsShortString());
	return TRUE;
}


static void HandleEmptySectorNoticedByGarrison(UINT8 ubGarrisonSectorID, UINT8 ubEmptySectorID)
{
	SECTORINFO *pSector;
	GROUP *pGroup;
	UINT8 ubAvailableTroops;
	UINT8 ubSrcGarrisonID = 255, ubDstGarrisonID = 255;

	//Make sure that the destination sector doesn't already have a pending group.
	pSector = &SectorInfo[ ubEmptySectorID ];

	ubSrcGarrisonID = SectorInfo[ ubGarrisonSectorID ].ubGarrisonID;
	ubDstGarrisonID = SectorInfo[ ubEmptySectorID ].ubGarrisonID;

	if( ubSrcGarrisonID == NO_GARRISON || ubDstGarrisonID == NO_GARRISON )
	{ //Bad logic
		return;
	}

	if( gGarrisonGroup[ ubDstGarrisonID ].ubPendingGroupID )
	{ //A group is already on-route, so don't send anybody from here.
		return;
	}

	//An opportunity has arisen, where the enemy has noticed an important sector that is undefended.
	pSector = &SectorInfo[ ubGarrisonSectorID ];
	ubAvailableTroops = pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins;

	if( ubAvailableTroops >= gubMinEnemyGroupSize * 2 )
	{ //split group into two groups, and move one of the groups to the next sector.
		pGroup = CreateNewEnemyGroupDepartingFromSector( ubGarrisonSectorID, 0, (UINT8)(ubAvailableTroops / 2), 0 );
		ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ ubDstGarrisonID ].ubComposition );
		RemoveSoldiersFromGarrisonBasedOnComposition( ubSrcGarrisonID, pGroup->ubGroupSize );
		gGarrisonGroup[ ubDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
		MoveSAIGroupToSector( &pGroup, ubEmptySectorID, DIRECT, REINFORCEMENTS );
	}
}


static BOOLEAN ReinforcementsApproved(INT32 iGarrisonID, UINT16* pusDefencePoints)
{
	SECTORINFO *pSector;
	UINT16 usOffensePoints;
	SGPSector sSector(gGarrisonGroup[iGarrisonID].ubSectorID);
	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];

	*pusDefencePoints = pSector->ubNumberOfCivsAtLevel[ GREEN_MILITIA ]		* 1 +
										pSector->ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] * 2 +
										pSector->ubNumberOfCivsAtLevel[ ELITE_MILITIA ]		* 3 +
										PlayerMercsInSector(sSector) * 4;
	usOffensePoints = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bAdminPercentage * 2 +
										gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bTroopPercentage * 3 +
										gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bElitePercentage * 4 +
										gubGarrisonReinforcementsDenied[ iGarrisonID ];
	usOffensePoints = usOffensePoints * gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation / 100;

	if( usOffensePoints > *pusDefencePoints )
	{
		return TRUE;
	}
	//Before returning false, determine if reinforcements have been denied repeatedly.  If so, then
	//we might send an augmented force to take it back.
	if( gubGarrisonReinforcementsDenied[ iGarrisonID ] + usOffensePoints > *pusDefencePoints )
	{
		STLOGD("Sector {} will now recieve an {} extra troops due to multiple denials for reinforcements in the past for strong player presence.",
				sSector.AsShortString(), gubGarrisonReinforcementsDenied[iGarrisonID] / 3);
		return TRUE;
	}
	//Reinforcements will have to wait.  For now, increase the reinforcements denied.  The amount increase is 20 percent
	//of the garrison's priority.
	gubGarrisonReinforcementsDenied[ iGarrisonID ] += (UINT8)(gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bPriority / 2);

	return FALSE;
}


static void EliminateSurplusTroopsForGarrison(GROUP* pGroup, SECTORINFO* pSector);
static void RecalculateGarrisonWeight(INT32 iGarrisonID);
static void RecalculatePatrolWeight(PATROL_GROUP&);


//if the group has arrived in a sector, and doesn't have any particular orders, then
//send him back where they came from.
//RETURNS TRUE if the group is deleted or told to move somewhere else.
//This is important as the calling function will need
//to abort processing of the group for obvious reasons.
static BOOLEAN EvaluateGroupSituation(GROUP* pGroup)
{
	SECTORINFO *pSector;
	GROUP *pPatrolGroup;
	UINT32 i;

	if( !gfQueenAIAwake )
	{
		return FALSE;
	}
	Assert( !pGroup->fPlayer );
	if( pGroup->pEnemyGroup->ubIntention == PURSUIT )
	{ //Lost the player group that he was going to attack.  Return to original position.
		ReassignAIGroup( &pGroup );
		return TRUE;
	}
	else if( pGroup->pEnemyGroup->ubIntention == REINFORCEMENTS )
	{ //The group has arrived at the location where he is supposed to reinforce.
		//Step 1 -- Check for matching garrison location
		SGPSector sec = pGroup->ubSector;
		for( i = 0; i < gGarrisonGroup.size(); i++ )
		{
			if (gGarrisonGroup[i].ubSectorID == sec.AsByte() &&
					gGarrisonGroup[ i ].ubPendingGroupID == pGroup->ubGroupID )
			{
				pSector = &SectorInfo[sec.AsByte()];

				if( gGarrisonGroup[ i ].ubSectorID != SEC_P3 )
				{
					EliminateSurplusTroopsForGarrison( pGroup, pSector );
					pSector->ubNumAdmins = (UINT8)(pSector->ubNumAdmins + pGroup->pEnemyGroup->ubNumAdmins);
					pSector->ubNumTroops = (UINT8)(pSector->ubNumTroops + pGroup->pEnemyGroup->ubNumTroops);
					pSector->ubNumElites = (UINT8)(pSector->ubNumElites + pGroup->pEnemyGroup->ubNumElites);

					SLOGD(ST::format("{} reinforcements have arrived to garrison sector {}",
							pGroup->pEnemyGroup->ubNumAdmins + pGroup->pEnemyGroup->ubNumTroops +
							pGroup->pEnemyGroup->ubNumElites, sec.AsShortString()));
					if (IsThisSectorASAMSector(sec))
					{
						StrategicMap[sec.AsStrategicIndex()].bSAMCondition = 100;
						UpdateSAMDoneRepair(sec);
					}
				}
				else
				{ //The group was sent back to the queen's palace (probably because they couldn't be reassigned
					//anywhere else, but it is possible that the queen's sector is requesting the reinforcements.  In
					//any case, if the queen's sector is less than full strength, fill it up first, then
					//simply add the rest to the global pool.
					if( pSector->ubNumElites < MAX_STRATEGIC_TEAM_SIZE )
					{
						if( pSector->ubNumElites + pGroup->ubGroupSize >= MAX_STRATEGIC_TEAM_SIZE )
						{ //Fill up the queen's guards, then apply the rest to the reinforcement pool
							giReinforcementPool += MAX_STRATEGIC_TEAM_SIZE - pSector->ubNumElites;
							pSector->ubNumElites = MAX_STRATEGIC_TEAM_SIZE;
							SLOGD("%d reinforcements have arrived to garrison queen's sector.  The excess troops will be relocated to the reinforcement pool.",
									pGroup->ubGroupSize );
						}
						else
						{ //Add all the troops to the queen's guard.
							pSector->ubNumElites += pGroup->ubGroupSize;
							SLOGD(ST::format("{} reinforcements have arrived to garrison queen's sector ({}).",
									pGroup->ubGroupSize, sec.AsShortString()));
						}
					}
					else
					{ //Add all the troops to the reinforcement pool as the queen's guard is at full strength.
						giReinforcementPool += pGroup->ubGroupSize;
						SLOGD(ST::format("{} reinforcements have arrived at queen's sector ({}) and have been added to the reinforcement pool.",
								pGroup->ubGroupSize, sec.AsShortString()));
					}
				}

				SetThisSectorAsEnemyControlled(sec);
				RemoveGroup(*pGroup);
				RecalculateGarrisonWeight( i );

				return TRUE;
			}
		}
		//Step 2 -- Check for Patrol groups matching waypoint index.
		for( i = 0; i < gPatrolGroup.size(); i++ )
		{
			if( gPatrolGroup[i].ubSectorID[1] == sec.AsByte() &&
					gPatrolGroup[ i ].ubPendingGroupID == pGroup->ubGroupID )
			{
				gPatrolGroup[ i ].ubPendingGroupID = 0;
				if( gPatrolGroup[ i ].ubGroupID && gPatrolGroup[ i ].ubGroupID != pGroup->ubGroupID )
				{ //cheat, and warp our reinforcements to them!
					pPatrolGroup = GetGroup( gPatrolGroup[ i ].ubGroupID );
					pPatrolGroup->pEnemyGroup->ubNumTroops += pGroup->pEnemyGroup->ubNumTroops;
					pPatrolGroup->pEnemyGroup->ubNumElites += pGroup->pEnemyGroup->ubNumElites;
					pPatrolGroup->pEnemyGroup->ubNumAdmins += pGroup->pEnemyGroup->ubNumAdmins;
					pPatrolGroup->ubGroupSize += (UINT8)(pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins );
					STLOGD("{} reinforcements have joined patrol group at sector {} (new size: {})",
							pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
							pPatrolGroup->ubSector.AsShortString(), pPatrolGroup->ubGroupSize);
					if( pPatrolGroup->ubGroupSize > MAX_STRATEGIC_TEAM_SIZE )
					{
						UINT8 ubCut;
						SLOGE(ST::format("Patrol group #{} in {} received too many reinforcements from group #{} that was created in {}{}.  Size truncated from {} to {}.\n\
							Please send Strategic Decisions.txt and PRIOR save.",
							pPatrolGroup->ubGroupID, sec.AsShortString(),
							pGroup->ubGroupID, SECTORY( pGroup->ubCreatedSectorID ) + 'A' - 1, SECTORX( pGroup->ubCreatedSectorID ),
							pPatrolGroup->ubGroupSize, MAX_STRATEGIC_TEAM_SIZE));
						//truncate the group size.
						ubCut = pPatrolGroup->ubGroupSize - MAX_STRATEGIC_TEAM_SIZE;
						while( ubCut-- )
						{
							if( pGroup->pEnemyGroup->ubNumAdmins )
							{
								pGroup->pEnemyGroup->ubNumAdmins--;
								pPatrolGroup->pEnemyGroup->ubNumAdmins--;
							}
							else if( pGroup->pEnemyGroup->ubNumTroops )
							{
								pGroup->pEnemyGroup->ubNumTroops--;
								pPatrolGroup->pEnemyGroup->ubNumTroops--;
							}
							else if( pGroup->pEnemyGroup->ubNumElites )
							{
								pGroup->pEnemyGroup->ubNumElites--;
								pPatrolGroup->pEnemyGroup->ubNumElites--;
							}
						}
						pPatrolGroup->ubGroupSize = MAX_STRATEGIC_TEAM_SIZE;
						Assert( pPatrolGroup->pEnemyGroup->ubNumAdmins +
										pPatrolGroup->pEnemyGroup->ubNumTroops +
										pPatrolGroup->pEnemyGroup->ubNumElites == MAX_STRATEGIC_TEAM_SIZE );
					}
					RemoveGroup(*pGroup);
					RecalculatePatrolWeight(gPatrolGroup[i]);
					ValidateLargeGroup( pPatrolGroup );
				}
				else
				{ //the reinforcements have become the new patrol group (even if same group)
					gPatrolGroup[ i ].ubGroupID = pGroup->ubGroupID;
					pGroup->pEnemyGroup->ubIntention = PATROL;
					pGroup->ubMoveType = ENDTOEND_FORWARDS;
					RemoveGroupWaypoints(*pGroup);
					AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 0 ] );
					AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 1 ] );
					if( gPatrolGroup[ i ].ubSectorID[ 2 ] )
					{ //Add optional waypoints if included.
						AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 2 ] );
						if( gPatrolGroup[ i ].ubSectorID[ 3 ] )
							AddWaypointIDToPGroup( pGroup, gPatrolGroup[ i ].ubSectorID[ 3 ] );
					}
					SLOGD(ST::format("{} soldiers have arrived to patrol area near sector {}",
							pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins, sec.AsShortString()));
					RecalculatePatrolWeight(gPatrolGroup[i]);
				}
				return TRUE;
			}
		}
	}
	else
	{	//This is a floating group at his final destination...
		if( pGroup->pEnemyGroup->ubIntention != STAGING && pGroup->pEnemyGroup->ubIntention != REINFORCEMENTS )
		{
			ReassignAIGroup( &pGroup );
			return TRUE;
		}
	}
	return FALSE;
}


static bool EnemyNoticesPlayerArrival(GROUP const& pg, UINT8 const x, UINT8 const y)
{
	SGPSector sMap(x, y);
	GROUP* const eg = FindEnemyMovementGroupInSector(sMap);
	if (eg && AttemptToNoticeAdjacentGroupSucceeds())
	{
		HandlePlayerGroupNoticedByPatrolGroup(&pg, eg);
		return true;
	}

	SECTORINFO const& s         = SectorInfo[sMap.AsByte()];
	UINT8      const  n_enemies = s.ubNumAdmins + s.ubNumTroops + s.ubNumElites;
	if (n_enemies && s.ubGarrisonID != NO_GARRISON && AttemptToNoticeAdjacentGroupSucceeds())
	{
		HandlePlayerGroupNoticedByGarrison(&pg, sMap.AsByte());
		return true;
	}

	return false;
}


static void SendGroupToPool(GROUP** pGroup);


//returns TRUE if the group was deleted.
BOOLEAN StrategicAILookForAdjacentGroups( GROUP *pGroup )
{
	UINT8 ubSectorID;
	if( !gfQueenAIAwake )
	{ //The queen isn't aware the player's presence yet, so she is oblivious to any situations.

		if( !pGroup->fPlayer )
		{
			//Exception case!
			//In the beginning of the game, a group is sent to A9 after the first battle.  If you leave A9, when they arrive,
			//they will stay there indefinately because the AI isn't awake.  What we do, is if this is a group in A9, then
			//send them home.
			if( GroupAtFinalDestination( pGroup ) )
			{
				//Wake up the queen now, if she hasn't woken up already.
				WakeUpQueen();
				static const SGPSector omerta(9, 1);
				if (pGroup->ubSector == omerta || pGroup->ubSector == meduna)
				{
					SendGroupToPool( &pGroup );
					if( !pGroup )
					{ //Group was transferred to the pool
						return TRUE;
					}
				}
			}
		}

		if( !gfQueenAIAwake )
		{
			return FALSE;
		}
	}
	if( !pGroup->fPlayer )
	{	//The enemy group has arrived at a new sector and now controls it.
		//Look in each of the four directions, and the alertness rating will
		//determine the chance to detect any players that may exist in that sector.
		GROUP* pEnemyGroup = pGroup;
		if( GroupAtFinalDestination( pEnemyGroup ) )
		{
			return EvaluateGroupSituation( pEnemyGroup );
		}
		ubSectorID = pEnemyGroup->ubSector.AsByte();
		if (pEnemyGroup && pEnemyGroup->ubSector.y > 1 && EnemyPermittedToAttackSector(&pEnemyGroup, (UINT8)(ubSectorID - 16)))
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSector.x, pEnemyGroup->ubSector.y - 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if (CountAllMilitiaInSector(SGPSector(pEnemyGroup->ubSector.x, pEnemyGroup->ubSector.y - 1)) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup(SECTOR(pEnemyGroup->ubSector.x, pEnemyGroup->ubSector.y - 1), pEnemyGroup);
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID-16) );
			}
		}
		if (pEnemyGroup && pEnemyGroup->ubSector.x > 1 && EnemyPermittedToAttackSector(&pEnemyGroup, (UINT8)(ubSectorID - 1)))
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSector.x - 1, pEnemyGroup->ubSector.y);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if (CountAllMilitiaInSector(SGPSector(pEnemyGroup->ubSector.x - 1, pEnemyGroup->ubSector.y)) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup(SECTOR(pEnemyGroup->ubSector.x - 1, pEnemyGroup->ubSector.y), pEnemyGroup);
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID-1) );
			}
		}
		if( pEnemyGroup && pEnemyGroup->ubSector.y < 16 && EnemyPermittedToAttackSector( &pEnemyGroup, (UINT8)(ubSectorID + 16) ) )
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSector.x, pEnemyGroup->ubSector.y + 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if (CountAllMilitiaInSector(SGPSector(pEnemyGroup->ubSector.x, pEnemyGroup->ubSector.y + 1)) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup((UINT8)SECTOR(pEnemyGroup->ubSector.x, pEnemyGroup->ubSector.y + 1), pEnemyGroup);
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID+16) );
			}
		}
		if( pEnemyGroup && pEnemyGroup->ubSector.x < 16 && EnemyPermittedToAttackSector( &pEnemyGroup, (UINT8)(ubSectorID + 1) ) )
		{
			GROUP* const pPlayerGroup = FindPlayerMovementGroupInSector(pEnemyGroup->ubSector.x + 1, pEnemyGroup->ubSector.y);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandlePlayerGroupNoticedByPatrolGroup( pPlayerGroup, pEnemyGroup );
			}
			else if (CountAllMilitiaInSector(SGPSector(pEnemyGroup->ubSector.x + 1, pEnemyGroup->ubSector.y)) &&
							AttemptToNoticeAdjacentGroupSucceeds() )
			{
				return HandleMilitiaNoticedByPatrolGroup(SECTOR(pEnemyGroup->ubSector.x + 1, pEnemyGroup->ubSector.y), pEnemyGroup);
			}
			else if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				return HandleEmptySectorNoticedByPatrolGroup( pEnemyGroup, (UINT8)(ubSectorID+1) );
			}
		}
		if( !pEnemyGroup )
		{ //group deleted.
			return TRUE;
		}
	}
	else
	{ /* The player group has arrived at a new sector and now controls it.  Look
		 * in each of the four directions, and the enemy alertness rating will
		 * determine if the enemy notices that the player is here.  Additionally,
		 * there are also stationary enemy groups that may also notice the player's
		 * new presence.
		 * NOTE: Always returns false because it is the player group that we are
		 *       handling.  We don't mess with the player group here! */
		GROUP const& pg = *pGroup;
		if (pg.ubSector.z != 0) return FALSE;
		UINT8 const x = pg.ubSector.x;
		UINT8 const y = pg.ubSector.y;
		if (!EnemyPermittedToAttackSector(0, pg.ubSector.AsByte())) return FALSE;
		if (y >  1 && EnemyNoticesPlayerArrival(pg, x,     y - 1)) return FALSE;
		if (x < 16 && EnemyNoticesPlayerArrival(pg, x + 1, y))     return FALSE;
		if (y < 16 && EnemyNoticesPlayerArrival(pg, x,     y + 1)) return FALSE;
		if (x >  1 && EnemyNoticesPlayerArrival(pg, x - 1, y))     return FALSE;
	}
	return FALSE;
}


//This is called periodically for each enemy occupied sector containing garrisons.
void CheckEnemyControlledSector( UINT8 ubSectorID )
{
	SECTORINFO *pSector;
	if( !gfQueenAIAwake )
	{
		return;
	}
	//First, determine if the sector is still owned by the enemy.
	pSector = &SectorInfo[ ubSectorID ];
	if( pSector->ubGarrisonID != NO_GARRISON )
	{
		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{ //Look for a staging group.
			GROUP *pGroup;
			pGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
			if( pGroup )
			{ //We have a staging group
				if( GroupAtFinalDestination( pGroup ) )
				{
					if( pGroup->pEnemyGroup->ubPendingReinforcements )
					{
						if( pGroup->pEnemyGroup->ubPendingReinforcements > 4 )
						{
							UINT8 ubNum = (UINT8)(3 + Random( 3 ));
							pGroup->pEnemyGroup->ubNumTroops += ubNum;
							pGroup->ubGroupSize += ubNum;
							pGroup->pEnemyGroup->ubPendingReinforcements -= ubNum;
							RecalculateGroupWeight(*pGroup);
							ValidateLargeGroup( pGroup );
						}
						else
						{
							pGroup->pEnemyGroup->ubNumTroops += pGroup->pEnemyGroup->ubPendingReinforcements;
							pGroup->ubGroupSize += pGroup->pEnemyGroup->ubPendingReinforcements;
							pGroup->pEnemyGroup->ubPendingReinforcements = 0;
							ValidateLargeGroup( pGroup );
						}
					}
					else if (pGroup->ubSector.AsByte() != gGarrisonGroup[pSector->ubGarrisonID].ubSectorID)
					{
						MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ pSector->ubGarrisonID ].ubSectorID, DIRECT, pGroup->pEnemyGroup->ubIntention );
					}
				}
				//else the group is on route to stage hopefully...
			}
		}
	}

	if( pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites )
	{
		//The sector is still controlled, so look around to see if there are any players nearby.
		SGPSector sSector(ubSectorID);
		if (sSector.y > 1 && EnemyPermittedToAttackSector(NULL, (UINT8)(ubSectorID - 16)))
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(bSectorX, sSector.y - 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID-16) );
				return;
			}
		}
		if( sSector.x < 16 && EnemyPermittedToAttackSector( NULL, (UINT8)(ubSectorID + 1) ) )
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(sSector.x + 1, sSector.y);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID+1) );
				return;
			}
		}
		if( sSector.y < 16 && EnemyPermittedToAttackSector( NULL, (UINT8)(ubSectorID + 16) ) )
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(sSector.x, sSector.y + 1);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID+16) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID+16) );
				return;
			}
		}
		if( sSector.x > 1 && EnemyPermittedToAttackSector( NULL, (UINT8)(ubSectorID - 1) ) )
		{
			/*
			pPlayerGroup = FindPlayerMovementGroupInSector(sSector.x - 1, sSector.y);
			if( pPlayerGroup && AttemptToNoticeAdjacentGroupSucceeds() )
			{
				HandlePlayerGroupNoticedByGarrison( pPlayerGroup, ubSectorID );
				return;
			}
			else
			*/
			if( AdjacentSectorIsImportantAndUndefended( (UINT8)(ubSectorID-1) ) && AttemptToNoticeEmptySectorSucceeds() )
			{
				HandleEmptySectorNoticedByGarrison( ubSectorID, (UINT8)(ubSectorID-1) );
				return;
			}
		}
	}
}


void RemoveGroupFromStrategicAILists(GROUP const& g)
{
	UINT8 const group_id = g.ubGroupID;
	for (size_t i = 0; i < gPatrolGroup.size(); ++i)
	{
		PATROL_GROUP& pg = gPatrolGroup[i];
		if (pg.ubGroupID == group_id)
		{ // Patrol group was destroyed
			pg.ubGroupID = 0;
			RecalculatePatrolWeight(pg);
			return;
		}
		if (pg.ubPendingGroupID == group_id)
		{ // Group never arrived to reinforce
			pg.ubPendingGroupID = 0;
			return;
		}
	}
	for (size_t i = 0; i < gGarrisonGroup.size(); ++i)
	{
		GARRISON_GROUP& gg = gGarrisonGroup[i];
		if (gg.ubPendingGroupID == group_id)
		{ // Group never arrived to reinforce
			gg.ubPendingGroupID = 0;
			return;
		}
	}
}


/* Recalculates a group's weight based on any changes.
 * @@@Alex, this is possibly missing in some areas. It is hard to ensure it is
 * everywhere with all the changes I've made. I'm sure you could probably find
 * some missing calls. */
static void RecalculatePatrolWeight(PATROL_GROUP& p)
{
	// First, remove the previous weight from the applicable field
	INT32 const prev_weight = p.bWeight;
	if (prev_weight > 0) giRequestPoints -= prev_weight;

	INT32 need_population;
	if (p.ubGroupID != 0)
	{
		need_population = p.bSize - GetGroup(p.ubGroupID)->ubGroupSize;
		if (need_population < 0)
		{
			p.bWeight = 0;
			return;
		}
	}
	else
	{
		need_population = p.bSize;
	}
	INT32 weight = need_population * 3 * p.bPriority / 96;
	weight = MIN(weight, 2);
	p.bWeight        = weight;
	giRequestPoints += weight;
}


//Recalculates a group's weight based on any changes.
//@@@Alex, this is possibly missing in some areas.  It is hard to ensure it is
//everywhere with all the changes I've made.  I'm sure you could probably find some missing calls.
static void RecalculateGarrisonWeight(INT32 iGarrisonID)
{
	SECTORINFO *pSector;
	INT32 iWeight, iPrevWeight;
	INT32 iDesiredPop, iCurrentPop, iPriority;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iDesiredPop = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;
	iCurrentPop = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iPriority = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bPriority;

	//First, remove the previous weight from the applicable field.
	iPrevWeight = gGarrisonGroup[ iGarrisonID ].bWeight;
	if( iPrevWeight > 0 )
		giRequestPoints -= iPrevWeight;
	else if( iPrevWeight < 0 )
		giReinforcementPoints += iPrevWeight;

	//Calculate weight (range is -20 to +20 before multiplier).
	//The multiplier of 3 brings it to a range of -96 to +96 which is
	//close enough to a plus/minus 100%.  The resultant percentage is then
	//converted based on the priority.
	iWeight = (iDesiredPop - iCurrentPop) * 3;
	if( iWeight > 0 )
	{ //modify it by it's priority.
		//generates a value between 2 and 100
		iWeight = iWeight * iPriority / 96;
		iWeight = MAX( iWeight, 2 );
		giRequestPoints += iWeight;
	}
	else if( iWeight < 0 )
	{ //modify it by it's reverse priority
		//generates a value between -2 and -100
		iWeight = iWeight * (100-iPriority) / 96;
		iWeight = MIN( iWeight, -2 );
		giReinforcementPoints -= (INT8)iWeight;
	}

	gGarrisonGroup[ iGarrisonID ].bWeight = (INT8)iWeight;
}

void RecalculateSectorWeight( UINT8 ubSectorID )
{
	for( size_t i = 0; i < gGarrisonGroup.size(); i++ )
	{
		if( gGarrisonGroup[ i ].ubSectorID == ubSectorID )
		{
			RecalculateGarrisonWeight( i );
			return;
		}
	}
}


static void TagSAIGroupWithGracePeriod(GROUP const&);


void RecalculateGroupWeight(GROUP const& g)
{
	for (size_t i = 0; i != gPatrolGroup.size(); ++i)
	{
		PATROL_GROUP& p = gPatrolGroup[i];
		if (p.ubGroupID != g.ubGroupID) continue;
		if (g.ubGroupSize == 0)
		{
			TagSAIGroupWithGracePeriod(g);
			p.ubGroupID = 0;
		}
		RecalculatePatrolWeight(p);
		return;
	}
}


static BOOLEAN GarrisonCanProvideMinimumReinforcements(INT32 iGarrisonID);


static INT32 ChooseSuitableGarrisonToProvideReinforcements(INT32 iDstGarrisonID, INT32 iReinforcementsRequested)
{
	UINT32 uiSrcGarrisonID, uiBestGarrisonID = NO_GARRISON;
	INT32 iReinforcementsAvailable, iWeight, iRandom;
	INT8 bBestWeight;
	UINT8 ubSectorID;
	size_t uiGarrisonArraySize = gGarrisonGroup.size();

	//Check to see if we could send reinforcements from Alma.  Only Drassen/Cambria get preferred
	//service from Alma, due to it's proximity and Alma's purpose as a forward military base.
	ubSectorID = gGarrisonGroup[ iDstGarrisonID ].ubSectorID;
	switch( ubSectorID )
	{
		case SEC_B13: case SEC_C13: case SEC_D13: case SEC_D15:								//Drassen + nearby SAM site
		case SEC_F8:  case SEC_F9:  case SEC_G8:  case SEC_G9:  case SEC_H8:	//Cambria
			//reinforcements will be primarily sent from Alma whenever possible.
			size_t i;
			//find which the first sector that contains Alma soldiers.
			for( i = 0; i < gGarrisonGroup.size(); i++ )
			{
				if( gGarrisonGroup[ i ].ubComposition == ALMA_DEFENCE )
					break;
			}
			uiSrcGarrisonID = i;
			//which of these 4 Alma garrisons have the most reinforcements available?  It is
			//possible that none of these garrisons can provide any reinforcements.
			bBestWeight = 0;
			for( i = uiSrcGarrisonID; i < uiSrcGarrisonID + 4; i++ )
			{
				RecalculateGarrisonWeight( i );
				if( bBestWeight > gGarrisonGroup[ i ].bWeight && GarrisonCanProvideMinimumReinforcements( i ) )
				{
					bBestWeight = gGarrisonGroup[ i ].bWeight;
					uiBestGarrisonID = i;
				}
			}
			//If we can provide reinforcements from Alma, then make sure that it can provide at least 67% of
			//the requested reinforcements.
			if( bBestWeight < 0 )
			{
				iReinforcementsAvailable = ReinforcementsAvailable( uiBestGarrisonID );
				if( iReinforcementsAvailable * 100 >= iReinforcementsRequested * 67 )
				{ //This is the approved group to provide the reinforcements.
					return uiBestGarrisonID;
				}
			}
			break;
	}

	//The Alma case either wasn't applicable or failed to have the right reinforcements.  Do a general weighted search.
	iRandom = Random( giReinforcementPoints );
	for( uiSrcGarrisonID = 0; uiSrcGarrisonID < uiGarrisonArraySize; uiSrcGarrisonID++ )
	{ //go through the garrisons
		RecalculateGarrisonWeight( uiSrcGarrisonID );
		iWeight = -gGarrisonGroup[ uiSrcGarrisonID ].bWeight;
		if( iWeight > 0 )
		{ //if group is able to provide reinforcements.
			if( iRandom < iWeight && GarrisonCanProvideMinimumReinforcements( uiSrcGarrisonID ) )
			{
				iReinforcementsAvailable = ReinforcementsAvailable( uiSrcGarrisonID );
				if( iReinforcementsAvailable * 100 >= iReinforcementsRequested * 67 )
				{ //This is the approved group to provide the reinforcements.
					return uiSrcGarrisonID;
				}
			}
			iRandom -= iWeight;
		}
	}

	//So far we have failed on all accounts.  Now, simply process all the garrisons, and return the first garrison that can
	//provide the reinforcements.
	for( uiSrcGarrisonID = 0; uiSrcGarrisonID < uiGarrisonArraySize; uiSrcGarrisonID++ )
	{ //go through the garrisons
		RecalculateGarrisonWeight( uiSrcGarrisonID );
		iWeight = -gGarrisonGroup[ uiSrcGarrisonID ].bWeight;
		if( iWeight > 0 && GarrisonCanProvideMinimumReinforcements( uiSrcGarrisonID ) )
		{ //if group is able to provide reinforcements.
			iReinforcementsAvailable = ReinforcementsAvailable( uiSrcGarrisonID );
			if( iReinforcementsAvailable * 100 >= iReinforcementsRequested * 67 )
			{ //This is the approved group to provide the reinforcements.
				return uiSrcGarrisonID;
			}
		}
	}

	//Well, if we get this far, the queen must be low on troops.  Send whatever we can.
	iRandom = Random( giReinforcementPoints );
	for( uiSrcGarrisonID = 0; uiSrcGarrisonID < uiGarrisonArraySize; uiSrcGarrisonID++ )
	{ //go through the garrisons
		RecalculateGarrisonWeight( uiSrcGarrisonID );
		iWeight = -gGarrisonGroup[ uiSrcGarrisonID ].bWeight;
		if( iWeight > 0 && GarrisonCanProvideMinimumReinforcements( uiSrcGarrisonID ) )
		{ //if group is able to provide reinforcements.
			if( iRandom < iWeight )
			{
				iReinforcementsAvailable = ReinforcementsAvailable( uiSrcGarrisonID );
				return uiSrcGarrisonID;
			}
			iRandom -= iWeight;
		}
	}

	//Failed completely.
	return -1;
}


static void SendReinforcementsForGarrison(INT32 iDstGarrisonID, UINT16 usDefencePoints, GROUP** pOptionalGroup)
{
	INT32 iChance, iRandom, iSrcGarrisonID;
	INT32 iMaxReinforcementsAllowed, iReinforcementsAvailable, iReinforcementsRequested, iReinforcementsApproved;
	GROUP *pGroup;
	UINT8 ubNumExtraReinforcements;
	UINT8 ubGroupSize;
	BOOLEAN fLimitMaxTroopsAllowable = FALSE;

	//Determine how many units the garrison needs.
	iReinforcementsRequested = GarrisonReinforcementsRequested( iDstGarrisonID, &ubNumExtraReinforcements );

	//The maximum number of reinforcements can't be offsetted past a certain point based on the
	//priority of the garrison.
	iMaxReinforcementsAllowed = //from 1 to 3 times the desired size of the normal force.
		gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bDesiredPopulation +
		gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bDesiredPopulation *
		gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bPriority / 50;

	if( iReinforcementsRequested + ubNumExtraReinforcements > iMaxReinforcementsAllowed )
	{ //adjust the extra reinforcements so that it doesn't exceed the maximum allowed.
		fLimitMaxTroopsAllowable = TRUE;
		ubNumExtraReinforcements = (UINT8)(iMaxReinforcementsAllowed - iReinforcementsRequested);
	}

	iReinforcementsRequested += ubNumExtraReinforcements;

	if( iReinforcementsRequested <= 0 )
	{
		return;
	}

	UINT8 ubDstSectorX = (UINT8)SECTORX( gGarrisonGroup[ iDstGarrisonID ].ubSectorID );
	UINT8 ubDstSectorY = (UINT8)SECTORY( gGarrisonGroup[ iDstGarrisonID ].ubSectorID );
	SGPSector dstSector(ubDstSectorX, ubDstSectorY);

	if( pOptionalGroup && *pOptionalGroup )
	{ //This group will provide the reinforcements
		pGroup = *pOptionalGroup;

		STLOGD("{} troops have been reassigned from {} to garrison sector {}",
				pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
				pGroup->ubSector.AsShortString(),
				dstSector.AsShortString());

		gGarrisonGroup[ iDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
		ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ iDstGarrisonID ].ubComposition );
		MoveSAIGroupToSector( pOptionalGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, REINFORCEMENTS );
		return;
	}
	iRandom = Random( giReinforcementPoints + giReinforcementPool );
	if( iRandom < giReinforcementPool )
	{ //use the pool and send the requested amount from SECTOR P3 (queen's palace)
		QUEEN_POOL:

		//KM : Sep 9, 1999
		//If the player owns sector P3, any troops that spawned there were causing serious problems, seeing battle checks
		//were not performed!
		if( !StrategicMap[ CALCULATE_STRATEGIC_INDEX( 3, 16 ) ].fEnemyControlled )
		{ //Queen can no longer send reinforcements from the palace if she doesn't control it!
			return;
		}


		if( !giReinforcementPool )
		{
			return;
		}
		iReinforcementsApproved = MIN( iReinforcementsRequested, giReinforcementPool );

		if( iReinforcementsApproved * 3 < usDefencePoints )
		{ //The enemy force that would be sent would likely be decimated by the player forces.
			gubGarrisonReinforcementsDenied[ iDstGarrisonID ] += (UINT8)(gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bPriority / 2);
			return;
		}
		else
		{
			//The force is strong enough to be able to take the sector.
			gubGarrisonReinforcementsDenied[ iDstGarrisonID ] = 0;
		}

		//The chance she will send them is related with the strength difference between the
		//player's force and the queen's.
		if( ubNumExtraReinforcements && fLimitMaxTroopsAllowable && iReinforcementsApproved == iMaxReinforcementsAllowed )
		{
			iChance = (iReinforcementsApproved + ubNumExtraReinforcements) * 100 / usDefencePoints;
			if( !Chance( iChance ) )
			{
				return;
			}
		}


		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, (UINT8)iReinforcementsApproved, 0 );
		ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ iDstGarrisonID ].ubComposition );
		pGroup->ubOriginalSector = dstSector.AsByte();
		giReinforcementPool -= iReinforcementsApproved;
		pGroup->ubMoveType = ONE_WAY;
		gGarrisonGroup[ iDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;

		ubGroupSize = (UINT8)(pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins);

		if( ubNumExtraReinforcements )
		{
			SLOGD(ST::format("{} troops have been sent from palace to stage assault near sector {}",
				  ubGroupSize, dstSector.AsShortString()));
			MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, STAGING );
		}
		else
		{
			SLOGD(ST::format("{} troops have been sent from palace to garrison sector {}",
					ubGroupSize, dstSector.AsShortString()));
			MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, REINFORCEMENTS );
		}
		return;
	}
	else
	{
		iSrcGarrisonID = ChooseSuitableGarrisonToProvideReinforcements( iDstGarrisonID, iReinforcementsRequested );
		if( iSrcGarrisonID == -1 )
		{
			goto QUEEN_POOL;
		}

		UINT8 ubSrcSectorX = (gGarrisonGroup[ iSrcGarrisonID ].ubSectorID % 16) + 1;
		UINT8 ubSrcSectorY = (gGarrisonGroup[ iSrcGarrisonID ].ubSectorID / 16) + 1;
		SGPSector srcSector(ubSrcSectorX, ubSrcSectorY);
		if (srcSector != gWorldSector)
		{ //The reinforcements aren't coming from the currently loaded sector!
			iReinforcementsAvailable = ReinforcementsAvailable( iSrcGarrisonID );
			if( iReinforcementsAvailable <= 0)
			{
				SLOGE( "Attempting to send reinforcements from a garrison that doesn't have any! -- KM:0 (with prior saved game and Debug Log)" );
				return;
			}
			//Send the lowest of the two:  number requested or number available

			iReinforcementsApproved = MIN( iReinforcementsRequested, iReinforcementsAvailable );
			if( iReinforcementsApproved > iMaxReinforcementsAllowed - ubNumExtraReinforcements )
			{ //The force isn't strong enough, but the queen isn't willing to apply extra resources
				iReinforcementsApproved = iMaxReinforcementsAllowed - ubNumExtraReinforcements;
			}
			else if( (iReinforcementsApproved + ubNumExtraReinforcements) * 3 < usDefencePoints )
			{ //The enemy force that would be sent would likely be decimated by the player forces.
				gubGarrisonReinforcementsDenied[ iDstGarrisonID ] += (UINT8)(gArmyComp[ gGarrisonGroup[ iDstGarrisonID ].ubComposition ].bPriority / 2);
				return;
			}
			else
			{
				//The force is strong enough to be able to take the sector.
				gubGarrisonReinforcementsDenied[ iDstGarrisonID ] = 0;
			}

			//The chance she will send them is related with the strength difference between the
			//player's force and the queen's.
			if( iReinforcementsApproved + ubNumExtraReinforcements == iMaxReinforcementsAllowed && usDefencePoints )
			{
				iChance = (iReinforcementsApproved + ubNumExtraReinforcements) * 100 / usDefencePoints;
				if( !Chance( iChance ) )
				{
					return;
				}
			}

			pGroup = CreateNewEnemyGroupDepartingFromSector( gGarrisonGroup[ iSrcGarrisonID ].ubSectorID, 0, (UINT8)iReinforcementsApproved, 0 );
			ConvertGroupTroopsToComposition( pGroup, gGarrisonGroup[ iDstGarrisonID ].ubComposition );
			RemoveSoldiersFromGarrisonBasedOnComposition( iSrcGarrisonID, pGroup->ubGroupSize );
			pGroup->ubOriginalSector = dstSector.AsByte();
			pGroup->ubMoveType = ONE_WAY;
			gGarrisonGroup[ iDstGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
			ubGroupSize = (UINT8)( pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins );

			if( ubNumExtraReinforcements )
			{
				pGroup->pEnemyGroup->ubPendingReinforcements = ubNumExtraReinforcements;
				STLOGD("{} troops have been sent from sector {} to stage assault near sector {}",
						ubGroupSize, pGroup->ubSector.AsShortString(), dstSector.AsShortString());

				MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, STAGING );
			}
			else
			{
				STLOGD("{} troops have been sent from sector {} to garrison sector {}",
						ubGroupSize, pGroup->ubSector.AsShortString(), dstSector.AsShortString());

				MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iDstGarrisonID ].ubSectorID, STAGE, REINFORCEMENTS );
			}
			SLOGD(ST::format("{} troops have been sent from garrison sector {} to patrol area near sector {}",
					ubGroupSize, srcSector.AsShortString(), dstSector.AsShortString()));

			return;
		}
	}
}


static void SendReinforcementsForPatrol(INT32 iPatrolID, GROUP** pOptionalGroup)
{
	GROUP *pGroup;
	INT32 iRandom, iWeight;
	UINT32 uiSrcGarrisonID;
	INT32 iReinforcementsAvailable, iReinforcementsRequested, iReinforcementsApproved;

	PATROL_GROUP* const pg = &gPatrolGroup[iPatrolID];

	//Determine how many units the patrol group needs.
	iReinforcementsRequested = PatrolReinforcementsRequested(pg);

	if( iReinforcementsRequested <= 0)
		return;

	SGPSector dstSector((pg->ubSectorID[1] % 16) + 1, (pg->ubSectorID[1] / 16) + 1);

	if( pOptionalGroup && *pOptionalGroup )
	{ //This group will provide the reinforcements
		pGroup = *pOptionalGroup;
		pg->ubPendingGroupID = pGroup->ubGroupID;

		STLOGD("{} troops have been reassigned from {c}{} to reinforce patrol group covering sector {}",
				pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
				pGroup->ubSector.AsShortString(), dstSector.AsShortString());

		MoveSAIGroupToSector(pOptionalGroup, pg->ubSectorID[1], EVASIVE, REINFORCEMENTS);
		return;
	}
	iRandom = Random( giReinforcementPoints + giReinforcementPool );
	if( iRandom < giReinforcementPool )
	{ //use the pool and send the requested amount from SECTOR P3 (queen's palace)
		iReinforcementsApproved = MIN( iReinforcementsRequested, giReinforcementPool );
		if( !iReinforcementsApproved )
		{
			AssertMsg(FALSE, "Trying to create empty reinforcements group!");
			return;
		}
		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, (UINT8)iReinforcementsApproved, 0 );
		pGroup->ubOriginalSector = dstSector.AsByte();
		giReinforcementPool -= iReinforcementsApproved;
		pg->ubPendingGroupID = pGroup->ubGroupID;

		SLOGD(ST::format("{} troops have been sent from palace to patrol area near sector {}",
				pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins, dstSector.AsShortString()));

		MoveSAIGroupToSector(&pGroup, pg->ubSectorID[1], EVASIVE, REINFORCEMENTS);
		return;
	}
	else
	{
		iRandom -= giReinforcementPool;
		for( uiSrcGarrisonID = 0; uiSrcGarrisonID < gGarrisonGroup.size(); uiSrcGarrisonID++ )
		{ //go through the garrisons
			RecalculateGarrisonWeight( uiSrcGarrisonID );
			iWeight = -gGarrisonGroup[ uiSrcGarrisonID ].bWeight;
			if( iWeight > 0 )
			{ //if group is able to provide reinforcements.
				if( iRandom < iWeight )
				{ //This is the group that gets the reinforcements!
					UINT8 ubSrcSectorX = (UINT8) SECTORX(gGarrisonGroup[uiSrcGarrisonID].ubSectorID);
					UINT8 ubSrcSectorY = (UINT8) SECTORY(gGarrisonGroup[uiSrcGarrisonID].ubSectorID);
					SGPSector srcSector(ubSrcSectorX, ubSrcSectorY);
					if (srcSector != gWorldSector)
					{ //The reinforcements aren't coming from the currently loaded sector!
						iReinforcementsAvailable = ReinforcementsAvailable( uiSrcGarrisonID );
						//Send the lowest of the two:  number requested or number available
						iReinforcementsApproved = MIN( iReinforcementsRequested, iReinforcementsAvailable );
						pGroup = CreateNewEnemyGroupDepartingFromSector( gGarrisonGroup[ uiSrcGarrisonID ].ubSectorID, 0, (UINT8)iReinforcementsApproved, 0 );
						pGroup->ubOriginalSector = dstSector.AsByte();
						pg->ubPendingGroupID = pGroup->ubGroupID;

						RemoveSoldiersFromGarrisonBasedOnComposition( uiSrcGarrisonID, pGroup->ubGroupSize );

						SLOGD(ST::format("{} troops have been sent from garrison sector {} to patrol area near sector {}",
								pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins,
								srcSector.AsShortString(), dstSector.AsShortString()));

						MoveSAIGroupToSector(&pGroup, pg->ubSectorID[1], EVASIVE, REINFORCEMENTS);
						return;
					}
				}
				iRandom -= iWeight;
			}
		}
	}
}


static void EvolveQueenPriorityPhase(BOOLEAN fForceChange);
static BOOLEAN GarrisonRequestingMinimumReinforcements(INT32 iGarrisonID);
static BOOLEAN PatrolRequestingMinimumReinforcements(INT32 iPatrolID);
static void UpgradeAdminsToTroops();


//Periodically does a general poll and check on each of the groups and garrisons, determines
//reinforcements, new patrol groups, planned assaults, etc.
void EvaluateQueenSituation()
{
	INT32 iRandom, iWeight;
	UINT32 uiOffset;
	UINT16 usDefencePoints;
	INT32 iSumOfAllWeights = 0;

	// figure out how long it shall be before we call this again

	// The more work to do there is (request points the queen's army is asking for), the more often she will make decisions
	// This can increase the decision intervals by up to 500 extra minutes (> 8 hrs)
	uiOffset = MAX( 100 - giRequestPoints, 0);
	uiOffset = uiOffset + Random( uiOffset * 4 );
	uiOffset += saipolicy_by_diff(time_evaluate_in_minutes) + Random(saipolicy_by_diff(time_evaluate_variance));

	if( !giReinforcementPool )
	{ //Queen has run out of reinforcements.  Simulate recruiting and training new troops
		uiOffset *= 10;
		giReinforcementPool += 30;
		AddStrategicEvent( EVENT_EVALUATE_QUEEN_SITUATION, GetWorldTotalMin() + uiOffset, 0 );
		return;
	}

	//Re-post the event
	AddStrategicEvent( EVENT_EVALUATE_QUEEN_SITUATION, GetWorldTotalMin() + uiOffset, 0 );

	// if the queen hasn't been alerted to player's presence yet
	if( !gfQueenAIAwake )
	{ //no decisions can be made yet.
		return;
	}

	// Adjust queen's disposition based on player's progress
	EvolveQueenPriorityPhase( FALSE );

	// Gradually promote any remaining admins into troops
	UpgradeAdminsToTroops();

	if( ( giRequestPoints <= 0 ) || ( ( giReinforcementPoints <= 0 ) && ( giReinforcementPool <= 0 ) ) )
	{ //we either have no reinforcements or request for reinforcements.
		return;
	}

	//now randomly choose who gets the reinforcements.
	// giRequestPoints is the combined sum of all the individual weights of all garrisons and patrols requesting reinforcements
	iRandom = Random( giRequestPoints );

	//go through garrisons first
	for (size_t i = 0; i < gGarrisonGroup.size(); i++)
	{
		RecalculateGarrisonWeight( i );
		iWeight = gGarrisonGroup[ i ].bWeight;
		if( iWeight > 0 )
		{	//if group is requesting reinforcements.

			iSumOfAllWeights += iWeight;	// debug only!

			if( iRandom < iWeight && !gGarrisonGroup[ i ].ubPendingGroupID &&
					EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) &&
					GarrisonRequestingMinimumReinforcements( i ) )
			{ //This is the group that gets the reinforcements!
				if( ReinforcementsApproved( i, &usDefencePoints ) )
				{
					SendReinforcementsForGarrison( i, usDefencePoints, NULL );
				}
				else
				{
					SLOGD("Reinforcements were denied to go to %c%d because player forces too strong.",
							SECTORY( gGarrisonGroup[ i ].ubSectorID ) + 'A' - 1, SECTORX( gGarrisonGroup[ i ].ubSectorID ) );
				}
				return;
			}
			iRandom -= iWeight;
		}
	}

	//go through the patrol groups
	for (size_t i = 0; i < gPatrolGroup.size(); i++)
	{
		RecalculatePatrolWeight(gPatrolGroup[i]);
		iWeight = gPatrolGroup[ i ].bWeight;
		if( iWeight > 0 )
		{
			iSumOfAllWeights += iWeight;	// debug only!

			if( iRandom < iWeight && !gPatrolGroup[ i ].ubPendingGroupID && PatrolRequestingMinimumReinforcements( i ) )
			{ //This is the group that gets the reinforcements!
				SendReinforcementsForPatrol( i, NULL );
				return;
			}
			iRandom -= iWeight;
		}
	}
}


void SaveStrategicAI(HWFILE const hFile)
{
	GARRISON_GROUP gEmptyGarrisonGroup;
	PATROL_GROUP gEmptyPatrolGroup;
	ARMY_COMPOSITION gEmptyArmyComp;
	INT32 iPatrolArraySize = static_cast<INT32>(gPatrolGroup.size());
	INT32 iGarrisonArraySize = static_cast<INT32>(gGarrisonGroup.size());
	size_t i;

	gEmptyPatrolGroup = PATROL_GROUP{};
	gEmptyArmyComp = ARMY_COMPOSITION{};
	gEmptyGarrisonGroup = GARRISON_GROUP{};

	hFile->seek(3, FILE_SEEK_FROM_CURRENT);
	hFile->write(&gfExtraElites,                      1);
	hFile->write(&iGarrisonArraySize,                 4);
	hFile->write(&iPatrolArraySize,                   4);
	hFile->write(&giReinforcementPool,                4);
	hFile->write(&giForcePercentage,                  4);
	hFile->write(&giArmyAlertness,                    4);
	hFile->write(&giArmyAlertnessDecay,               4);
	hFile->write(&gfQueenAIAwake,                     1);
	hFile->write(&giReinforcementPoints,              4);
	hFile->write(&giRequestPoints,                    4);
	hFile->write(&gubNumAwareBattles,                 1);
	hFile->write(&gubSAIVersion,                      1);
	hFile->write(&gubQueenPriorityPhase,              1);
	hFile->write(&gfFirstBattleMeanwhileScenePending, 1);
	hFile->write(&gfMassFortificationOrdered,         1);
	hFile->write(&gubMinEnemyGroupSize,               1);
	hFile->write(&gubHoursGracePeriod,                1);
	hFile->write(&gusPlayerBattleVictories,           2);
	hFile->write(&gfUseAlternateQueenPosition,        1);
	hFile->write(gbPadding,           SAI_PADDING_BYTES);
	//Save the army composition (which does get modified)
	hFile->write(gArmyComp.data(), gArmyComp.size() * sizeof(ARMY_COMPOSITION));
	i = SAVED_ARMY_COMPOSITIONS - gArmyComp.size();
	while( i-- )
	{
		hFile->write(&gEmptyArmyComp, sizeof(ARMY_COMPOSITION));
	}
	//Save the patrol group definitions
	if (!gPatrolGroup.empty()) hFile->write(gPatrolGroup.data(), gPatrolGroup.size() * sizeof(PATROL_GROUP));
	i = SAVED_PATROL_GROUPS - gPatrolGroup.size();
	while( i-- )
	{
		hFile->write(&gEmptyPatrolGroup, sizeof(PATROL_GROUP));
	}
	//Save the garrison information!
	if (!gGarrisonGroup.empty()) hFile->write(gGarrisonGroup.data(), gGarrisonGroup.size() * sizeof(GARRISON_GROUP));
	i = SAVED_GARRISON_GROUPS - gGarrisonGroup.size();
	while( i-- )
	{
		hFile->write(&gEmptyGarrisonGroup, sizeof(GARRISON_GROUP));
	}

	hFile->write(gubPatrolReinforcementsDenied, gPatrolGroup.size());

	hFile->write(gubGarrisonReinforcementsDenied, gGarrisonGroup.size());
}


static void ReinitializeUnvisitedGarrisons(void);


void LoadStrategicAI(HWFILE const hFile)
{
	size_t i;
	UINT8 ubSAIVersion;
	INT32 iPatrolArraySize, iGarrisonArraySize;

	hFile->seek(3, FILE_SEEK_FROM_CURRENT);
	hFile->read(&gfExtraElites,                      1);
	hFile->read(&iGarrisonArraySize,                 4);
	hFile->read(&iPatrolArraySize,                   4);
	hFile->read(&giReinforcementPool,                4);
	hFile->read(&giForcePercentage,                  4);
	hFile->read(&giArmyAlertness,                    4);
	hFile->read(&giArmyAlertnessDecay,               4);
	hFile->read(&gfQueenAIAwake,                     1);
	hFile->read(&giReinforcementPoints,              4);
	hFile->read(&giRequestPoints,                    4);
	hFile->read(&gubNumAwareBattles,                 1);
	hFile->read(&ubSAIVersion,                       1);
	hFile->read(&gubQueenPriorityPhase,              1);
	hFile->read(&gfFirstBattleMeanwhileScenePending, 1);
	hFile->read(&gfMassFortificationOrdered,         1);
	hFile->read(&gubMinEnemyGroupSize,               1);
	hFile->read(&gubHoursGracePeriod,                1);
	hFile->read(&gusPlayerBattleVictories,           2);
	hFile->read(&gfUseAlternateQueenPosition,        1);
	hFile->read(gbPadding,           SAI_PADDING_BYTES);
	//Restore the army composition
	gArmyComp.clear();
	gArmyComp.assign(SAVED_ARMY_COMPOSITIONS, ARMY_COMPOSITION{});
	hFile->read(gArmyComp.data(), SAVED_ARMY_COMPOSITIONS * sizeof(ARMY_COMPOSITION)); // read everything first, will discard what we don't need when we have also the Garrison Groups

	//Restore the patrol group definitions
	if (GCM->getPatrolGroups().size() != (size_t)iPatrolArraySize)
	{
		STLOGW("Number of Patrol Groups in save ({}) is different from definition ({}). Save might not work properly.", iPatrolArraySize, GCM->getPatrolGroups().size());
	}
	auto buffPG = new PATROL_GROUP[SAVED_PATROL_GROUPS]{};
	hFile->read(buffPG, SAVED_PATROL_GROUPS * sizeof(PATROL_GROUP));
	gPatrolGroup = std::vector<PATROL_GROUP>(buffPG, buffPG + iPatrolArraySize);
	delete[] buffPG;

	gubSAIVersion = SAI_VERSION;
	//Load the garrison information!
	if (GCM->getGarrisonGroups().size() != (size_t)iGarrisonArraySize)
	{
		STLOGW("Number of Garrison Groups in save ({}) is different from definition ({}). Save might not work properly.", iGarrisonArraySize, GCM->getGarrisonGroups().size());
	}
	auto buffGG = new GARRISON_GROUP[SAVED_GARRISON_GROUPS]{};
	hFile->read(buffGG, SAVED_GARRISON_GROUPS * sizeof(GARRISON_GROUP));
	gGarrisonGroup = std::vector<GARRISON_GROUP>(buffGG, buffGG + iGarrisonArraySize);
	delete[] buffGG;

	// resize gArmyComp, ensuring all army compositions referenced by Garrison Groups exist
	size_t numArmyCompositions = NUM_ARMY_COMPOSITIONS;
	for (auto gGroup : gGarrisonGroup)
	{
		numArmyCompositions = std::max<size_t>(numArmyCompositions, gGroup.ubComposition + 1);
	}
	gArmyComp.resize(numArmyCompositions);
	if (gArmyComp.size() != GCM->getArmyCompositions().size())
	{
		STLOGW("Number of Army Compositions in save ({}) is different from definition ({}). Save might not work properly.", gArmyComp.size(), GCM->getArmyCompositions().size());
	}
	ArmyCompositionModel::validateLoadedData(gArmyComp);

	//Load the list of reinforcement patrol points.
	if( gubPatrolReinforcementsDenied )
	{
		delete[] gubPatrolReinforcementsDenied;
		gubPatrolReinforcementsDenied = NULL;
	}
	gubPatrolReinforcementsDenied = new UINT8[iPatrolArraySize]{};
	hFile->read(gubPatrolReinforcementsDenied, iPatrolArraySize);

	//Load the list of reinforcement garrison points.
	if( gubGarrisonReinforcementsDenied )
	{
		delete[] gubGarrisonReinforcementsDenied;
		gubGarrisonReinforcementsDenied = NULL;
	}
	gubGarrisonReinforcementsDenied = new UINT8[iGarrisonArraySize]{};
	hFile->read(gubGarrisonReinforcementsDenied, iGarrisonArraySize);

	if( ubSAIVersion < 6 )
	{ //Reinitialize the costs since they have changed.

		//Recreate the compositions
		gArmyComp = GCM->getArmyCompositions();
		EvolveQueenPriorityPhase( TRUE );

		//Recreate the patrol desired sizes
		auto origPatrolGroup = GCM->getPatrolGroups();
		for( i = 0; i < gPatrolGroup.size(); i++ )
		{
			gPatrolGroup[ i ].bSize = origPatrolGroup[ i ].bSize;
		}
	}
	if( ubSAIVersion < 7 )
	{
		BuildUndergroundSectorInfoList();
	}
	if( ubSAIVersion < 8 )
	{
		ReinitializeUnvisitedGarrisons();
	}
	if( ubSAIVersion < 10 )
	{
		for( i = 0; i < gPatrolGroup.size(); i++ )
		{
			if( gPatrolGroup[ i ].bSize >= 16 )
			{
				gPatrolGroup[ i ].bSize = 10;
			}
		}
		FOR_EACH_ENEMY_GROUP(pGroup)
		{
			if (pGroup->ubGroupSize >= 16)
			{ //accident in patrol groups being too large
				UINT8	ubGetRidOfXTroops = pGroup->ubGroupSize - 10;
				if (gWorldSector.z || pGroup->ubSector != gWorldSector)
				{ //don't modify groups in the currently loaded sector.
					if( pGroup->pEnemyGroup->ubNumTroops >= ubGetRidOfXTroops )
					{
						pGroup->pEnemyGroup->ubNumTroops -= ubGetRidOfXTroops;
						pGroup->ubGroupSize -= ubGetRidOfXTroops;
					}
				}
			}
		}
	}
	if( ubSAIVersion < 13 )
	{
		for( i = 0; i < 255; i++ )
		{
			SectorInfo[ i ].bBloodCatPlacements = 0;
			SectorInfo[ i ].bBloodCats = -1;
		}
		InitBloodCatSectors();
		//This info is used to clean up the two new codes inserted into the
		//middle of the enumeration for battle codes.
		if( gubEnemyEncounterCode > CREATURE_ATTACK_CODE )
		{
			gubEnemyEncounterCode += 2;
		}
		if( gubExplicitEnemyEncounterCode > CREATURE_ATTACK_CODE )
		{
			gubExplicitEnemyEncounterCode += 2;
		}

	}
	if( ubSAIVersion < 14 )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector(SGPSector(4, 11, 1));
		if( pSector->ubNumTroops + pSector->ubNumElites > 20 )
		{
			pSector->ubNumTroops -= 2;
		}
		pSector = FindUnderGroundSector(SGPSector(3, 15, 1));
		if( pSector->ubNumTroops + pSector->ubNumElites > 20 )
		{
			pSector->ubNumTroops -= 2;
		}
	}
	if( ubSAIVersion < 16 )
	{
		UNDERGROUND_SECTORINFO *pSector;
		pSector = FindUnderGroundSector(SGPSector(3, 15, 1));
		if( pSector )
		{
			pSector->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
		}
		pSector = FindUnderGroundSector(SGPSector(3, 16, 1));
		if( pSector )
		{
			pSector->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
		}
	}
	if( ubSAIVersion < 17 )
	{ //Patch all groups that have this flag set
		gubNumGroupsArrivedSimultaneously = 0;
		{
			FOR_EACH_GROUP(pGroup)
			{
				if( pGroup->uiFlags & GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY )
				{
					pGroup->uiFlags &= ~GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY;
				}
			}
		}
	}
	if( ubSAIVersion < 18 )
	{ //adjust down the number of bloodcats based on difficulty in the two special bloodcat levels
		switch( gGameOptions.ubDifficultyLevel )
		{
			case DIF_LEVEL_EASY: //50%
				SectorInfo[ SEC_I16	].bBloodCatPlacements = 14;
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 13;
				SectorInfo[ SEC_I16	].bBloodCats = 14;
				SectorInfo[ SEC_N5	].bBloodCats = 13;
				break;
			case DIF_LEVEL_MEDIUM: //75%
				SectorInfo[ SEC_I16	].bBloodCatPlacements = 19;
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 18;
				SectorInfo[ SEC_I16	].bBloodCats = 19;
				SectorInfo[ SEC_N5	].bBloodCats = 18;
				break;
			case DIF_LEVEL_HARD: //100%
				SectorInfo[ SEC_I16	].bBloodCatPlacements = 26;
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 25;
				SectorInfo[ SEC_I16	].bBloodCats = 26;
				SectorInfo[ SEC_N5	].bBloodCats = 25;
				break;
		}
	}
	if( ubSAIVersion < 19 )
	{
		//Clear the garrison in C5
		gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_C5 ].ubGarrisonID ].ubComposition ].bPriority = 0;
		gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_C5 ].ubGarrisonID ].ubComposition ].bDesiredPopulation = 0;
	}
	if( ubSAIVersion < 20 )
	{
		gArmyComp[ QUEEN_DEFENCE ].bDesiredPopulation = 32;
		SectorInfo[ SEC_P3 ].ubNumElites = 32;
	}
	if( ubSAIVersion < 21 )
	{
		FOR_EACH_GROUP(pGroup) pGroup->uiFlags = 0;
	}
	if( ubSAIVersion < 22 )
	{ //adjust down the number of bloodcats based on difficulty in the two special bloodcat levels
		switch( gGameOptions.ubDifficultyLevel )
		{
			case DIF_LEVEL_EASY: //50%
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 8;
				SectorInfo[ SEC_N5	].bBloodCats = 10;
				break;
			case DIF_LEVEL_MEDIUM: //75%
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 8;
				SectorInfo[ SEC_N5	].bBloodCats = 10;
				break;
			case DIF_LEVEL_HARD: //100%
				SectorInfo[ SEC_N5	].bBloodCatPlacements = 8;
				SectorInfo[ SEC_N5	].bBloodCats = 10;
				break;
		}
	}
	if( ubSAIVersion < 23 )
	{
		if (gWorldSector.x != 3 || gWorldSector.y != 16 || !gWorldSector.z)
		{
			SectorInfo[ SEC_P3 ].ubNumElites = 32;
		}
	}
	if( ubSAIVersion < 24 )
	{
		//If the queen has escaped to the basement, do not use the profile insertion info
		//when we finally go down there, otherwise she will end up in the wrong spot, possibly inside
		//the walls.
		if( !gubFact[ FACT_QUEEN_DEAD ] && gMercProfiles[ QUEEN ].bSectorZ == 1 )
				{
		if (gWorldSector.z != 1 || gWorldSector.x != 16 || gWorldSector.y != 3)
		{ //We aren't in the basement sector
			gMercProfiles[ QUEEN ].fUseProfileInsertionInfo = FALSE;
		}
		else
		{ //We are in the basement sector, relocate queen to proper position.
			FOR_EACH_IN_TEAM(i, CIV_TEAM)
			{
				SOLDIERTYPE& s = *i;
				if (s.ubProfile != QUEEN) continue;
				// Found queen, relocate her to 16866
				BumpAnyExistingMerc(16866);
				TeleportSoldier(s, 16866, true);
				break;
			}
		}
				}
	}
	if( ubSAIVersion < 25 )
	{
		if( gubFact[ FACT_SKYRIDER_CLOSE_TO_CHOPPER ] )
		{
			gMercProfiles[ SKYRIDER ].fUseProfileInsertionInfo = FALSE;
		}
	}

	//KM : July 21, 1999 patch fix
	if( ubSAIVersion < 26 )
	{
		INT32 i;
		for( i = 0; i < 255; i++ )
		{
			if( SectorInfo[ i ].ubNumberOfCivsAtLevel[ GREEN_MILITIA ] +
					SectorInfo[ i ].ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] +
					SectorInfo[ i ].ubNumberOfCivsAtLevel[ ELITE_MILITIA ] > 20 )
			{
				SectorInfo[ i ].ubNumberOfCivsAtLevel[ GREEN_MILITIA ] = 0;
				SectorInfo[ i ].ubNumberOfCivsAtLevel[ REGULAR_MILITIA ] = 20;
				SectorInfo[ i ].ubNumberOfCivsAtLevel[ ELITE_MILITIA ] = 0;
			}
		}
	}

	//KM : August 4, 1999 patch fix
	//     This addresses the problem of not having any soldiers in sector N7 when playing the game under easy difficulty.
	//     If captured and interrogated, the player would find no soldiers defending the sector.  This changes the composition
	//     so that it will always be there, and adds the soldiers accordingly if the sector isn't loaded when the update is made.
	if( ubSAIVersion < 27 )
	{
		if( gGameOptions.ubDifficultyLevel == DIF_LEVEL_EASY )
		{
			if (gWorldSector.x != 7 || gWorldSector.y != 14 || gWorldSector.z)
			{
				INT32 cnt, iRandom;
				INT32 iEliteChance, iTroopChance, iAdminChance;
				INT32 iStartPop;
				SECTORINFO *pSector = NULL;

				//Change the garrison composition to LEVEL1_DEFENCE from LEVEL2_DEFENCE
				pSector = &SectorInfo[ SEC_N7 ];
				gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition = LEVEL1_DEFENCE;

				iStartPop = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bStartPopulation;
				iEliteChance = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bElitePercentage;
				iTroopChance = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bTroopPercentage + iEliteChance;
				iAdminChance = gArmyComp[ gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bAdminPercentage;

				if( iStartPop )
				{
					// if population is less than maximum
					if( iStartPop != MAX_STRATEGIC_TEAM_SIZE )
					{
						// then vary it a bit (+/- 25%)
						iStartPop = iStartPop * ( 100 + ( Random ( 51 ) - 25 ) ) / 100;
					}

					iStartPop = MAX( gubMinEnemyGroupSize, MIN( MAX_STRATEGIC_TEAM_SIZE, iStartPop ) );
					cnt = iStartPop;

					if( iAdminChance )
					{
						pSector->ubNumAdmins = iAdminChance * iStartPop / 100;
					}
					else while( cnt-- )
					{ //for each person, randomly determine the types of each soldier.
						{
							iRandom = Random( 100 );
							if( iRandom < iEliteChance )
							{
								pSector->ubNumElites++;
							}
							else if( iRandom < iTroopChance )
							{
								pSector->ubNumTroops++;
							}
						}
					}
				}
			}
		}
	}

	if( ubSAIVersion < 28 )
	{
		if (!StrategicMap[meduna.AsStrategicIndex()].fEnemyControlled)
		{ //Eliminate all enemy groups in this sector, because the player owns the sector, and it is not
			//possible for them to spawn there!
			FOR_EACH_GROUP_SAFE(i)
			{
				GROUP& g = *i;
				if (g.fPlayer)         continue;
				if (g.ubSector != meduna) continue;
				if (g.ubPrev.IsValid()) continue;
				RemoveGroupFromStrategicAILists(g);
				RemoveGroup(g);
			}
		}
	}
	if( ubSAIVersion < 29 )
	{
		InitStrategicMovementCosts();
	}

	//KM : Aug 11, 1999 -- Patch fix:  Blindly update the airspace control.  There is a bug somewhere
	//     that is failing to keep this information up to date, and I failed to find it.  At least this
	//     will patch saves.
	UpdateAirspaceControl( );

	EvolveQueenPriorityPhase( TRUE );

	//Count and correct the floating groups
	FOR_EACH_GROUP_SAFE(pGroup)
	{
		if( !pGroup->fPlayer )
		{
			if( !pGroup->fBetweenSectors )
			{
				if (gWorldSector.z || pGroup->ubSector != gWorldSector)
				{
					RepollSAIGroup( pGroup );
					ValidateGroup( pGroup );
				}
			}
		}
	}
	SLOGD("Restoring saved game at Day %02d, %02d:%02d ", GetWorldDay(), GetWorldHour(), GetWorldMinutesInDay()%60 );

	//Update the version number to the most current.
	gubSAIVersion = SAI_VERSION;
}


//As the player's progress changes in the game, the queen will adjust her priorities accordingly.
//Basically, increasing priorities and numbers for sectors she owns, and lowering them.
//@@@Alex, this is tweakable.  My philosophies could be incorrect.  It might be better if instead of lowering
//priorities and numbers for towns the queen has lost, to instead lower the priority but increase the numbers so
//she would send larger attack forces.  This is questionable.
static void EvolveQueenPriorityPhase(BOOLEAN fForceChange)
{
	INT32 index, num, iFactor;
	INT32 iChange, iNew, iNumSoldiers, iNumPromotions;
	SECTORINFO *pSector;
	std::vector<UINT8> ubOwned;
	std::vector<UINT8> ubTotal;
	UINT8 ubNewPhase;
	ubNewPhase = CurrentPlayerProgressPercentage() / 10;
	auto origArmyComp = GCM->getArmyCompositions();

	if( !fForceChange && ubNewPhase == gubQueenPriorityPhase )
	{
		return;
	}

	if( gubQueenPriorityPhase > ubNewPhase )
	{
		SLOGD("The queen's defence priority has decreased from %d0%% to %d0%%.", gubQueenPriorityPhase, ubNewPhase );
	}
	else if( gubQueenPriorityPhase < ubNewPhase )
	{
		SLOGD("The queen's defence priority has increased from %d0%% to %d0%%.", gubQueenPriorityPhase, ubNewPhase );
	}
	else
	{
		SLOGD("The queen's defence priority is the same (%d0%%), but has been forced to update.", gubQueenPriorityPhase );
	}

	gubQueenPriorityPhase = ubNewPhase;

	//The phase value refers to the deviation percentage she will apply to original garrison values.
	//All sector values are evaluated to see how many of those sectors are enemy controlled.  If they
	//are controlled by her, the desired number will be increased as well as the priority.  On the other
	//hand, if she doesn't own those sectors, the values will be decreased instead.  All values are based off of
	//the originals.
	std::fill_n(std::back_inserter(ubOwned), gArmyComp.size(), 0);
	std::fill_n(std::back_inserter(ubTotal), gArmyComp.size(), 0);

	//Record the values required to calculate the percentage of each composition type that the queen controls.
	for( size_t i = 0; i < gGarrisonGroup.size(); i++ )
	{
		index = gGarrisonGroup[ i ].ubComposition;
		if( StrategicMap[ SECTOR_INFO_TO_STRATEGIC_INDEX( gGarrisonGroup[ i ].ubSectorID ) ].fEnemyControlled )
		{
			ubOwned[ index ]++;
		}
		ubTotal[ index ]++;
	}

	//Go through the *majority* of compositions and modify the priority/desired values.
	for( size_t i = 0; i < gArmyComp.size(); i++ )
	{
		switch( i )
		{
			case QUEEN_DEFENCE:
			case MEDUNA_DEFENCE:
			case MEDUNA_SAMSITE:
			case LEVEL1_DEFENCE:
			case LEVEL2_DEFENCE:
			case LEVEL3_DEFENCE:
			case OMERTA_WELCOME_WAGON:
			case ROADBLOCK:
			//case SANMONA_SMALL:
				//don't consider these compositions
				continue;
		}
		//If the queen owns them ALL, then she gets the maximum defensive bonus.  If she owns NONE,
		//then she gets a maximum defensive penalty.  Everything else lies in the middle.  The legal
		//range is +-50.
		if( ubTotal[ i ] )
		{
			iFactor = (ubOwned[ i ] * 100 / ubTotal[ i ]) - 50;
		}
		else
		{
			iFactor = -50;
		}
		iFactor = iFactor * gubQueenPriorityPhase / 10;

		//modify priority by + or - 25% of original
		if( gArmyComp[ i ].bPriority )
		{
			num = origArmyComp[ i ].bPriority+ iFactor / 2;
			num = MIN( MAX( 0, num ), 100 );
			gArmyComp[ i ].bPriority = (INT8)num;
		}

		//modify desired population by + or - 50% of original population
		num = origArmyComp[ i ].bDesiredPopulation * (100 + iFactor) / 100;
		num = MIN( MAX( 6, num ), MAX_STRATEGIC_TEAM_SIZE );
		gArmyComp[ i ].bDesiredPopulation = (INT8)num;

		//if gfExtraElites is set, then augment the composition sizes
		if( gfExtraElites && iFactor >= 15 && gArmyComp[ i ].bElitePercentage )
		{
			iChange = gGameOptions.ubDifficultyLevel * 5;

			//increase elite % (max 100)
			iNew = gArmyComp[ i ].bElitePercentage + iChange;
			iNew = (INT8)MIN( 100, iNew );
			gArmyComp[ i ].bElitePercentage = (INT8)iNew;

			//decrease troop % (min 0)
			iNew = gArmyComp[ i ].bTroopPercentage - iChange;
			iNew = (INT8)MAX( 0, iNew );
			gArmyComp[ i ].bTroopPercentage = (INT8)iNew;
		}
	}
	if( gfExtraElites )
	{
		//Turn off the flag so that this doesn't happen everytime this function is called!
		gfExtraElites = FALSE;

		for( UINT32 i = 0; i < gGarrisonGroup.size(); i++ )
		{
			//if we are dealing with extra elites, then augment elite compositions (but only if they exist in the sector).
			//If the queen still owns the town by more than 65% (iFactor >= 15), then upgrade troops to elites in those sectors.
			index = gGarrisonGroup[ i ].ubComposition;
			switch( index )
			{
				case QUEEN_DEFENCE:
				case MEDUNA_DEFENCE:
				case MEDUNA_SAMSITE:
				case LEVEL1_DEFENCE:
				case LEVEL2_DEFENCE:
				case LEVEL3_DEFENCE:
				case OMERTA_WELCOME_WAGON:
				case ROADBLOCK:
				//case SANMONA_SMALL:
					//don't consider these compositions
					continue;
			}
			pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];
			if( ubTotal[ index ] )
			{
				iFactor = (ubOwned[ index ] * 100 / ubTotal[ index ]) - 50;
			}
			else
			{
				iFactor = -50;
			}
			if( iFactor >= 15 )
			{ //Make the actual elites in sector match the new garrison percentage
				SGPSector garrisonSector(gGarrisonGroup[i].ubSectorID);
				if (!gfWorldLoaded || gWorldSector != garrisonSector)
				{ //Also make sure the sector isn't currently loaded!
					iNumSoldiers = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
					iNumPromotions = gArmyComp[ index ].bElitePercentage * iNumSoldiers / 100 - pSector->ubNumElites;

					if( iNumPromotions > 0 )
					{
						while( iNumPromotions-- )
						{
							if( pSector->ubNumAdmins )
							{
								pSector->ubNumAdmins--;
							}
							else if( pSector->ubNumTroops )
							{
								pSector->ubNumTroops--;
							}
							else
							{
								SLOGA("EvolveQueenPriorityPhase: more promotions than soldiers");
							}
							pSector->ubNumElites++;
						}
						Assert( iNumSoldiers == pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites );
					}
				}
			}
		}
	}
	//Recalculate all of the weights.
	for( size_t i = 0; i < gGarrisonGroup.size(); i++ )
	{
		RecalculateGarrisonWeight( i );
	}
}


static void RequestHighPriorityGarrisonReinforcements(INT32 iGarrisonID, UINT8 ubSoldiersRequested);


void ExecuteStrategicAIAction( UINT16 usActionCode, INT16 sSectorX, INT16 sSectorY )
{
	GROUP *pGroup, *pPendingGroup = NULL;
	SECTORINFO *pSector;
	UINT8 ubSectorID;
	UINT8 ubNumSoldiers;
	SGPSector sSector;
	switch( usActionCode )
	{
		case STRATEGIC_AI_ACTION_WAKE_QUEEN:
			WakeUpQueen();
			break;

		case STRATEGIC_AI_ACTION_QUEEN_DEAD:
			gfQueenAIAwake = FALSE;
			break;

		case STRATEGIC_AI_ACTION_KINGPIN_DEAD:
			//Immediate send a small garrison to C5 (to discourage access to Tony the dealer)
			/*
			for( i = 0; i < gGarrisonGroup.size(); i++ )
			{
				if( gGarrisonGroup[ i ].ubComposition == SANMONA_SMALL )
				{
					//Setup the composition so from now on the queen will consider this an important sector
					//to hold.
					gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bPriority = 65;
					gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bTroopPercentage = 100;
					gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bDesiredPopulation = 5;
					RequestHighPriorityGarrisonReinforcements( i, (UINT8)(2 + Random( 4 )) ); //send 2-5 soldiers now.
					break;
				}
			}
			*/
			break;
		case NPC_ACTION_SEND_SOLDIERS_TO_DRASSEN:
			//Send 6, 9, or 12 troops (based on difficulty) one of the Drassen sectors.  If nobody is there when they arrive,
			//those troops will get reassigned.

			if( Chance( 50 ) )
			{
				ubSectorID = SEC_D13;
			}
			else if( Chance( 60 ) )
			{
				ubSectorID = SEC_B13;
			}
			else
			{
				ubSectorID = SEC_C13;
			}
			ubNumSoldiers = (UINT8)(3 + gGameOptions.ubDifficultyLevel * 3);
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubNumSoldiers, 0 );

			if( !gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID )
			{
				pGroup->pEnemyGroup->ubIntention = STAGE;
				gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
			}
			else
			{ //this should never happen (but if it did, then this is the best way to deal with it).
				pGroup->pEnemyGroup->ubIntention = PURSUIT;
			}
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );

			MoveSAIGroupToSector( &pGroup, ubSectorID, EVASIVE, pGroup->pEnemyGroup->ubIntention );

			break;
		case NPC_ACTION_SEND_SOLDIERS_TO_BATTLE_LOCATION:

			//Send 4, 8, or 12 troops (based on difficulty) to the location of the first battle.  If nobody is there when they arrive,
			//those troops will get reassigned.
			ubSectorID = (UINT8)STRATEGIC_INDEX_TO_SECTOR_INFO( sWorldSectorLocationOfFirstBattle );
			pSector = &SectorInfo[ ubSectorID ];
			sSector = SGPSector(ubSectorID);
			ubNumSoldiers = (UINT8)(gGameOptions.ubDifficultyLevel * 4);
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubNumSoldiers, 0 );
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );

			//Determine if the battle location actually has a garrison assignment.  If so, and the following
			//checks succeed, the enemies will be sent to attack and reinforce that sector.  Otherwise, the
			//enemies will simply check it out, then leave.
			if( pSector->ubGarrisonID != NO_GARRISON )
			{ //sector has a garrison
				if (!NumEnemiesInSector(sSector))
				{ //no enemies are here
					if( gArmyComp[ !gGarrisonGroup[ pSector->ubGarrisonID ].ubComposition ].bPriority )
					{ //the garrison is important
						if( !gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
						{ //the garrison doesn't have reinforcements already on route.
							gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
							MoveSAIGroupToSector( &pGroup, ubSectorID, STAGE, REINFORCEMENTS );
							break;
						}
					}
				}
			}
			else
			{
				MoveSAIGroupToSector( &pGroup, ubSectorID, EVASIVE, PURSUIT );
			}

			break;
		case NPC_ACTION_SEND_SOLDIERS_TO_OMERTA:
			ubNumSoldiers = (UINT8)(gGameOptions.ubDifficultyLevel * 6); //6, 12, or 18 based on difficulty.
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubNumSoldiers, (UINT8)(ubNumSoldiers/7) ); //add 1 elite to normal, and 2 for hard
			ubNumSoldiers = (UINT8)(ubNumSoldiers + ubNumSoldiers / 7);
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );
			if (PlayerMercsInSector(SGPSector(9, 1, 1)) && !PlayerMercsInSector(SGPSector(10, 1, 1)) && !PlayerMercsInSector(SGPSector(10, 1, 2)))
			{ //send to A9 (if mercs in A9, but not in A10 or A10 basement)
				ubSectorID = SEC_A9;
			}
			else
			{ //send to A10
				ubSectorID = SEC_A10;
			}

			MoveSAIGroupToSector( &pGroup, ubSectorID, EVASIVE, PURSUIT );

			ValidateGroup( pGroup );
			break;
		case NPC_ACTION_SEND_TROOPS_TO_SAM:
			ubSectorID = (UINT8)SECTOR( sSectorX, sSectorY );
			ubNumSoldiers = (UINT8)( 3 + gGameOptions.ubDifficultyLevel + HighestPlayerProgressPercentage() / 15 );
			giReinforcementPool -= ubNumSoldiers;
			giReinforcementPool = MAX( giReinforcementPool, 0 );
			pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, 0, ubNumSoldiers );
			MoveSAIGroupToSector( &pGroup, ubSectorID, STAGE, REINFORCEMENTS );

			if( gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID )
			{	//Clear the pending group's assignment.
				pPendingGroup = GetGroup( gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID );
				Assert( pPendingGroup );
				RemoveGroupFromStrategicAILists(*pPendingGroup);
			}
			//Assign the elite squad to attack the SAM site
			pGroup->pEnemyGroup->ubIntention = REINFORCEMENTS;
			gGarrisonGroup[ SectorInfo[ ubSectorID ].ubGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;

			if( pPendingGroup )
			{ //Reassign the pending group
				ReassignAIGroup( &pPendingGroup );
			}

			break;
		case NPC_ACTION_ADD_MORE_ELITES:
			gfExtraElites = TRUE;
			EvolveQueenPriorityPhase( TRUE );
			break;
		case NPC_ACTION_GIVE_KNOWLEDGE_OF_ALL_MERCS:
			//temporarily make the queen's forces more aware (high alert)
			gubNumAwareBattles = saipolicy_by_diff(num_aware_battles);
			break;
		default:
			SLOGD("QueenAI failed to handle action code %d.", usActionCode );
			break;
	}
}

static UINT8 RedirectEnemyGroupsMovingThroughSector(const SGPSector& sSector);


void StrategicHandleQueenLosingControlOfSector(const SGPSector& sSector)
{
	SECTORINFO *pSector;
	UINT8 ubSectorID;
	if (sSector.z)
	{ //The queen doesn't care about anything happening under the ground.
		return;
	}

	if (StrategicMap[sSector.AsStrategicIndex()].fEnemyControlled)
	{ //If the sector doesn't belong to the player, then we shouldn't be calling this function!
		SLOGE( "StrategicHandleQueenLosingControlOfSector() was called for a sector that is internally considered to be enemy controlled." );
		return;
	}

	ubSectorID = sSector.AsByte();
	pSector = &SectorInfo[ ubSectorID ];

	//Keep track of victories and wake up the queen after x number of battles.
	gusPlayerBattleVictories++;
	if( gusPlayerBattleVictories == 5 - gGameOptions.ubDifficultyLevel )
	{ //4 victories for easy, 3 for normal, 2 for hard
		WakeUpQueen();
	}

	if( pSector->ubGarrisonID == NO_GARRISON )
	{ //Queen doesn't care if the sector lost wasn't a garrison sector.
		return;
	}
	else
	{ //check to see if there are any pending reinforcements.  If so, then cancel their orders and have them
		//reassigned, so the player doesn't get pestered.  This is a feature that *dumbs* down the AI, and is done
		//for the sake of gameplay.  We don't want the game to be tedious.
		if( !pSector->uiTimeLastPlayerLiberated )
		{
			pSector->uiTimeLastPlayerLiberated = GetWorldTotalSeconds();
		}
		else
		{ //convert hours to seconds and subtract up to half of it randomly "seconds - (hours*3600 / 2)"
			pSector->uiTimeLastPlayerLiberated = GetWorldTotalSeconds() - Random( gubHoursGracePeriod * 1800 );
		}
		if( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID )
		{
			GROUP *pGroup;
			pGroup = GetGroup( gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID );
			if( pGroup )
			{
				ReassignAIGroup( &pGroup );
			}
			gGarrisonGroup[ pSector->ubGarrisonID ].ubPendingGroupID = 0;
		}
	}

	//If there are any enemy groups that will be moving through this sector due, they will have to repath which
	//will cause them to avoid the sector.  Returns the number of redirected groups.
	RedirectEnemyGroupsMovingThroughSector(sSector);
}


static UINT8 SectorDistance(const SGPSector& sSector, UINT8 ubSectorID2)
{
	UINT8 ubSectorX1, ubSectorX2, ubSectorY1, ubSectorY2;
	UINT8 ubDist;
	ubSectorX1 = sSector.x;
	ubSectorX2 = (UINT8)SECTORX( ubSectorID2 );
	ubSectorY1 = sSector.y;
	ubSectorY2 = (UINT8)SECTORY( ubSectorID2 );

	ubDist = (UINT8)( ABS( ubSectorX1 - ubSectorX2 ) + ABS( ubSectorY1 - ubSectorY2 ) );

	return ubDist;
}


static void RequestHighPriorityGarrisonReinforcements(size_t iGarrisonID, UINT8 ubSoldiersRequested)
{
	size_t i, uiBestIndex;
	GROUP *pGroup;
	UINT8 ubBestDist, ubDist;
	UINT8 ubDstSectorX, ubDstSectorY;
	//AssertMsg( gPatrolGroup.size() == GCM->getPatrolGroups().size(), "Strategic AI -- Patrol group definition mismatch." );
	ubBestDist = 255;
	uiBestIndex = -1;
	for( i = 0; i < gPatrolGroup.size(); i++ )
	{
		if( gPatrolGroup[ i ].ubGroupID )
		{
			pGroup = GetGroup( gPatrolGroup[ i ].ubGroupID );
			if( pGroup && pGroup->ubGroupSize >= ubSoldiersRequested )
			{
				ubDist = SectorDistance(pGroup->ubSector.AsByte(), gGarrisonGroup[iGarrisonID].ubSectorID);
				if( ubDist < ubBestDist )
				{
					ubBestDist = ubDist;
					uiBestIndex = i;
				}
			}
		}
	}
	ubDstSectorX = (UINT8)SECTORX( gGarrisonGroup[ iGarrisonID ].ubSectorID );
	ubDstSectorY = (UINT8)SECTORY( gGarrisonGroup[ iGarrisonID ].ubSectorID );
	if( uiBestIndex != (size_t)-1 )
	{ //Send the group to the garrison
		pGroup = GetGroup( gPatrolGroup[ uiBestIndex ].ubGroupID );
		if( pGroup->ubGroupSize > ubSoldiersRequested && pGroup->ubGroupSize - ubSoldiersRequested >= gubMinEnemyGroupSize )
		{ //Split the group, and send to location
			GROUP *pNewGroup;
			pNewGroup = CreateNewEnemyGroupDepartingFromSector(pGroup->ubSector.AsByte(), 0, 0, 0);
			//Transfer the troops from group to new group
			if( pGroup->pEnemyGroup->ubNumTroops >= ubSoldiersRequested )
			{ //All of them are troops, so do it in one shot.
				pGroup->pEnemyGroup->ubNumTroops -= ubSoldiersRequested;
				pGroup->ubGroupSize -= ubSoldiersRequested;
				pNewGroup->pEnemyGroup->ubNumTroops = ubSoldiersRequested;
				pNewGroup->ubGroupSize += ubSoldiersRequested;
				ValidateLargeGroup( pGroup );
				ValidateLargeGroup( pNewGroup );
			}
			else while( ubSoldiersRequested )
			{ //There aren't enough troops, so transfer other types when we run out of troops, prioritizing admins, then elites.
				if( pGroup->pEnemyGroup->ubNumTroops )
				{
					pGroup->pEnemyGroup->ubNumTroops--;
					pGroup->ubGroupSize--;
					pNewGroup->pEnemyGroup->ubNumTroops++;
					pNewGroup->ubGroupSize++;
					ubSoldiersRequested--;
					ValidateLargeGroup( pGroup );
					ValidateLargeGroup( pNewGroup );
				}
				else if( pGroup->pEnemyGroup->ubNumAdmins )
				{
					pGroup->pEnemyGroup->ubNumAdmins--;
					pGroup->ubGroupSize--;
					pNewGroup->pEnemyGroup->ubNumAdmins++;
					pNewGroup->ubGroupSize++;
					ubSoldiersRequested--;
					ValidateLargeGroup( pGroup );
					ValidateLargeGroup( pNewGroup );
				}
				else if( pGroup->pEnemyGroup->ubNumElites )
				{
					pGroup->pEnemyGroup->ubNumElites--;
					pGroup->ubGroupSize--;
					pNewGroup->pEnemyGroup->ubNumElites++;
					pNewGroup->ubGroupSize++;
					ubSoldiersRequested--;
					ValidateLargeGroup( pGroup );
					ValidateLargeGroup( pNewGroup );
				}
				else
				{
					SLOGA("Strategic AI group transfer error." );
					return;
				}
			}
			pNewGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
			gGarrisonGroup[ iGarrisonID ].ubPendingGroupID = pNewGroup->ubGroupID;
			RecalculatePatrolWeight(gPatrolGroup[uiBestIndex]);

			MoveSAIGroupToSector( &pNewGroup, gGarrisonGroup[ iGarrisonID ].ubSectorID, EVASIVE, REINFORCEMENTS );
		}
		else
		{ //Send the whole group and kill it's patrol assignment.
			gPatrolGroup[ uiBestIndex ].ubGroupID = 0;
			gGarrisonGroup[ iGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
			pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
			RecalculatePatrolWeight(gPatrolGroup[uiBestIndex]);
			//The ONLY case where the group is told to move somewhere else when they could be BETWEEN sectors.  The movegroup functions
			//don't work if this is the case.  Teleporting them to their previous sector is the best and easiest way to deal with this.
			SetEnemyGroupSector(*pGroup, pGroup->ubSector.AsByte());

			MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iGarrisonID ].ubSectorID, EVASIVE, REINFORCEMENTS );
			ValidateGroup( pGroup );
		}
	}
	else
	{ //There are no groups that have enough troops.  Send a new force from the palace instead.
		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_P3, 0, ubSoldiersRequested, 0 );
		pGroup->ubMoveType = ONE_WAY;
		pGroup->pEnemyGroup->ubIntention = REINFORCEMENTS;
		gGarrisonGroup[ iGarrisonID ].ubPendingGroupID = pGroup->ubGroupID;
		pGroup->ubOriginalSector = (UINT8)SECTOR( ubDstSectorX, ubDstSectorY );
		giReinforcementPool -= (INT32)ubSoldiersRequested;

		MoveSAIGroupToSector( &pGroup, gGarrisonGroup[ iGarrisonID ].ubSectorID, EVASIVE, REINFORCEMENTS );
		ValidateGroup( pGroup );
	}
}


static void MassFortifyTowns(void);


void WakeUpQueen()
{
	gfQueenAIAwake = TRUE;
	if( !gfMassFortificationOrdered )
	{
		gfMassFortificationOrdered = TRUE;
		MassFortifyTowns();
	}
}


//Simply orders all garrisons to take troops from the patrol groups and send the closest troops from them.  Any garrison,
//whom there request isn't fulfilled (due to lack of troops), will recieve their reinforcements from the queen (P3).
static void MassFortifyTowns(void)
{
	size_t i;
	SECTORINFO *pSector;
	GROUP *pGroup;
	UINT8 ubNumTroops, ubDesiredTroops;
	for( i = 0; i < gGarrisonGroup.size(); i++ )
	{
		pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];
		ubNumTroops = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
		ubDesiredTroops = (UINT8)gArmyComp[ gGarrisonGroup[ i ].ubComposition ].bDesiredPopulation;
		if( ubNumTroops < ubDesiredTroops  )
		{
			if( !gGarrisonGroup[ i ].ubPendingGroupID &&
					gGarrisonGroup[ i ].ubComposition != ROADBLOCK &&
					EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) )
			{
				RequestHighPriorityGarrisonReinforcements( i, (UINT8)(ubDesiredTroops - ubNumTroops) );
			}
		}
	}
	//Convert the garrison sitting in Omerta (if alive), and reassign them
	pSector = &SectorInfo[ SEC_A9 ];
	if( pSector->ubNumTroops )
	{
		pGroup = CreateNewEnemyGroupDepartingFromSector( SEC_A9, 0, pSector->ubNumTroops, 0 );
		pSector->ubNumTroops = 0;
		pGroup->pEnemyGroup->ubIntention = PATROL;
		pGroup->ubMoveType = ONE_WAY;
		ReassignAIGroup( &pGroup );
		ValidateGroup( pGroup );
		RecalculateSectorWeight( SEC_A9 );
	}
}

void StrategicHandleMineThatRanOut( UINT8 ubSectorID )
{
	switch( ubSectorID )
	{
		case SEC_B2:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_A2 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_B2 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
		case SEC_D13:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_B13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_C13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_D13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
		case SEC_H8:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_F8 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_F9 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_G8 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_G9 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_H8 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
		case SEC_I14:
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_H13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_H14 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_I13 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			gArmyComp[ gGarrisonGroup[ SectorInfo[ SEC_I14 ].ubGarrisonID ].ubComposition ].bPriority /= 4;
			break;
	}
}


static BOOLEAN GarrisonCanProvideMinimumReinforcements(INT32 iGarrisonID)
{
	INT32 iAvailable;
	INT32 iDesired;
	SECTORINFO *pSector;

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];

	iAvailable = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iDesired = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;

	if( iAvailable - iDesired >= gubMinEnemyGroupSize )
	{
		//Do a more expensive check first to determine if there is a player presence here (combat in progress)
		//If so, do not provide reinforcements from here.
		SGPSector ubSector(gGarrisonGroup[iGarrisonID].ubSectorID);
		if (PlayerMercsInSector(ubSector) || CountAllMilitiaInSector(ubSector))
		{
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


static BOOLEAN GarrisonRequestingMinimumReinforcements(INT32 iGarrisonID)
{
	INT32 iAvailable;
	INT32 iDesired;
	SECTORINFO *pSector;

	if( gGarrisonGroup[ iGarrisonID ].ubPendingGroupID )
	{
		return FALSE;
	}

	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	iAvailable = pSector->ubNumAdmins + pSector->ubNumTroops + pSector->ubNumElites;
	iDesired = gArmyComp[ gGarrisonGroup[ iGarrisonID ].ubComposition ].bDesiredPopulation;

	if( iDesired - iAvailable >= gubMinEnemyGroupSize )
	{
		return TRUE;
	}
	return FALSE;
}


static BOOLEAN PermittedToFillPatrolGroup(INT32 iPatrolID);


static BOOLEAN PatrolRequestingMinimumReinforcements(INT32 iPatrolID)
{
	GROUP *pGroup;

	if( gPatrolGroup[ iPatrolID ].ubPendingGroupID )
	{
		return FALSE;
	}
	if( !PermittedToFillPatrolGroup( iPatrolID ) )
	{ //if the group was defeated, it won't be considered for reinforcements again for several days
		return FALSE;
	}
	pGroup = GetGroup( gPatrolGroup[ iPatrolID ].ubGroupID );
	if( pGroup )
	{
		if( gPatrolGroup[ iPatrolID ].bSize - pGroup->ubGroupSize >= gubMinEnemyGroupSize )
		{
			return TRUE;
		}
	}
	else if (saipolicy(refill_defeated_patrol_groups))
	{ // we want to refill totally defeated patrols too
		return TRUE;
	}
	return FALSE;
}


static void EliminateSurplusTroopsForGarrison(GROUP* pGroup, SECTORINFO* pSector)
{
	INT32 iTotal;
	iTotal = pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites + pGroup->pEnemyGroup->ubNumAdmins +
			pSector->ubNumTroops + pSector->ubNumElites + pSector->ubNumAdmins;
	if( iTotal <= MAX_STRATEGIC_TEAM_SIZE )
	{
		return;
	}
	iTotal -= MAX_STRATEGIC_TEAM_SIZE;
	while( iTotal )
	{
		if( pGroup->pEnemyGroup->ubNumAdmins )
		{
			if( pGroup->pEnemyGroup->ubNumAdmins < iTotal )
			{
				iTotal -= pGroup->pEnemyGroup->ubNumAdmins;
				pGroup->pEnemyGroup->ubNumAdmins = 0;
			}
			else
			{
				pGroup->pEnemyGroup->ubNumAdmins -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pSector->ubNumAdmins )
		{
			if( pSector->ubNumAdmins < iTotal )
			{
				iTotal -= pSector->ubNumAdmins;
				pSector->ubNumAdmins = 0;
			}
			else
			{
				pSector->ubNumAdmins -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pGroup->pEnemyGroup->ubNumTroops )
		{
			if( pGroup->pEnemyGroup->ubNumTroops < iTotal )
			{
				iTotal -= pGroup->pEnemyGroup->ubNumTroops;
				pGroup->pEnemyGroup->ubNumTroops = 0;
			}
			else
			{
				pGroup->pEnemyGroup->ubNumTroops -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pSector->ubNumTroops )
		{
			if( pSector->ubNumTroops < iTotal )
			{
				iTotal -= pSector->ubNumTroops;
				pSector->ubNumTroops = 0;
			}
			else
			{
				pSector->ubNumTroops -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pGroup->pEnemyGroup->ubNumElites )
		{
			if( pGroup->pEnemyGroup->ubNumElites < iTotal )
			{
				iTotal -= pGroup->pEnemyGroup->ubNumElites;
				pGroup->pEnemyGroup->ubNumElites = 0;
			}
			else
			{
				pGroup->pEnemyGroup->ubNumElites -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
		else if( pSector->ubNumElites )
		{
			if( pSector->ubNumElites < iTotal )
			{
				iTotal -= pSector->ubNumElites;
				pSector->ubNumElites = 0;
			}
			else
			{
				pSector->ubNumElites -= (UINT8)iTotal;
				iTotal = 0;
			}
		}
	}
}


/* Once Queen is awake, she'll gradually begin replacing admins with regular
 * troops. This is mainly to keep player from fighting many more admins once
 * they are no longer any challenge for him. Eventually all admins will vanish
 * off map. */
static void UpgradeAdminsToTroops()
{
	/* On normal, AI evaluates approximately every 10 hrs. There are about
	 * 130 administrators seeded on the map. Some of these will be killed by the
	 * player. */

	INT32 const min_priority = 100 - 10 * HighestPlayerProgressPercentage();

	// Check all garrisons for administrators.
	UINT                  const cur = GetWorldSector();
	//GARRISON_GROUP const* const end = gGarrisonGroup + gGarrisonGroup.size();
	for (auto i = gGarrisonGroup.begin(); i != gGarrisonGroup.end(); ++i)
	{
		GARRISON_GROUP const& g = *i;

		// Skip sector if it's currently loaded, we'll never upgrade guys in those.
		if (cur == g.ubSectorID) continue;

		SECTORINFO& sector = SectorInfo[g.ubSectorID];
		if (sector.ubNumAdmins == 0) continue; // No admins in garrison.

		INT8 const priority = gArmyComp[g.ubComposition].bPriority;

		/* Highest priority sectors are upgraded first. Each 1% of progress lowers
		 * the priority threshold required to start triggering upgrades by 10%. */
		if (priority <= min_priority) continue;

		for (UINT8 n_to_check = sector.ubNumAdmins; n_to_check != 0; --n_to_check)
		{
			/* Chance to upgrade at each check is random and is dependent on the
			 * garrison's priority. */
			if (!Chance(priority)) continue;
			--sector.ubNumAdmins;
			++sector.ubNumTroops;
		}
	}

	// Check all moving enemy groups for administrators.
	FOR_EACH_ENEMY_GROUP(i)
	{
		GROUP const& g = *i;
		if (g.ubGroupSize == 0) continue;
		if (g.fVehicle)         continue;

		// Skip sector if it's currently loaded, we'll never upgrade guys in those.
		if (g.ubSector == gWorldSector) continue;

		Assert(g.pEnemyGroup);
		ENEMYGROUP& eg = *g.pEnemyGroup;
		if (eg.ubNumAdmins == 0) continue; // No admins in group.

		INT8 priority;
		if (eg.ubIntention == PATROL)
		{ // Use that patrol's priority.
			size_t const patrol_id = FindPatrolGroupIndexForGroupID(g.ubGroupID);
			Assert(patrol_id != (size_t)-1);
			priority = gPatrolGroup[patrol_id].bPriority;
		}
		else
		{ // Use a default priority.
			priority = 50;
		}

		/* Highest priority groups are upgraded first. Each 1% of progress lowers
		 * the priority threshold required to start triggering upgrades by 10%. */
		if (priority <= min_priority) continue;

		for (UINT8 n_to_check = eg.ubNumAdmins; n_to_check != 0; --n_to_check)
		{
			/* Chance to upgrade at each check is random and is dependent on the
			 * group's priority. */
			if (!Chance(priority)) continue;
			--eg.ubNumAdmins;
			++eg.ubNumTroops;
		}
	}
}


size_t FindPatrolGroupIndexForGroupID( UINT8 ubGroupID )
{
	for( size_t sPatrolIndex = 0; sPatrolIndex < gPatrolGroup.size(); sPatrolIndex++ )
	{
		if ( gPatrolGroup[ sPatrolIndex ].ubGroupID == ubGroupID )
		{
			// found it
			return( sPatrolIndex );
		}
	}

	// not there!
	return( -1 );
}


size_t FindPatrolGroupIndexForGroupIDPending( UINT8 ubGroupID )
{
	for( size_t sPatrolIndex = 0; sPatrolIndex < gPatrolGroup.size(); sPatrolIndex++ )
	{
		if ( gPatrolGroup[ sPatrolIndex ].ubPendingGroupID == ubGroupID )
		{
			// found it
			return( sPatrolIndex );
		}
	}

	// not there!
	return( -1 );
}


size_t FindGarrisonIndexForGroupIDPending( UINT8 ubGroupID )
{
	for( size_t sGarrisonIndex = 0; sGarrisonIndex < gGarrisonGroup.size(); sGarrisonIndex++ )
	{
		if ( gGarrisonGroup[ sGarrisonIndex ].ubPendingGroupID == ubGroupID )
		{
			// found it
			return( sGarrisonIndex );
		}
	}

	// not there!
	return( -1 );
}


static void TransferGroupToPool(GROUP** pGroup)
{
	giReinforcementPool += (*pGroup)->ubGroupSize;
	RemoveGroup(**pGroup);
	*pGroup = NULL;
}


//NOTE:  Make sure you call SetEnemyGroupSector() first if the group is between sectors!!  See example in ReassignAIGroup()...
static void SendGroupToPool(GROUP** pGroup)
{
	if ((*pGroup)->ubSector == meduna)
	{
		TransferGroupToPool( pGroup );
	}
	else
	{
		(*pGroup)->ubSectorIDOfLastReassignment = (*pGroup)->ubSector.AsByte();
		MoveSAIGroupToSector( pGroup, SEC_P3, EVASIVE, REINFORCEMENTS );
	}
}


static void ReassignAIGroup(GROUP** pGroup)
{
	INT32 iWeight, iRandom;
	UINT16 usDefencePoints;
	size_t uiReloopLastIndex = -1;
	UINT8 ubSectorID;

	ubSectorID = (*pGroup)->ubSector.AsByte();

	(*pGroup)->ubSectorIDOfLastReassignment = ubSectorID;

	RemoveGroupFromStrategicAILists(**pGroup);

	//First thing to do, is teleport the group to be AT the sector he is currently moving from.  Otherwise, the
	//strategic pathing can break if the group is between sectors upon reassignment.
	SetEnemyGroupSector(**pGroup, ubSectorID);

	if( giRequestPoints <= 0  )
	{ //we have no request for reinforcements, so send the group to Meduna for reassignment in the pool.
		SendGroupToPool( pGroup );
		return;
	}

	//now randomly choose who gets the reinforcements.
	// giRequestPoints is the combined sum of all the individual weights of all garrisons and patrols requesting reinforcements
	iRandom = Random( giRequestPoints );

	//go through garrisons first and begin considering where the random value dictates.  If that garrison doesn't require
	//reinforcements, it'll continue on considering all subsequent garrisons till the end of the array.  If it fails at that
	//point, it'll restart the loop at zero, and consider all garrisons to the index that was first considered by the random value.
	for (size_t i = 0; i < gGarrisonGroup.size(); i++)
	{
		RecalculateGarrisonWeight( i );
		iWeight = gGarrisonGroup[ i ].bWeight;
		if( iWeight > 0 )
		{	//if group is requesting reinforcements.
			if( iRandom < iWeight )
			{
				if( !gGarrisonGroup[ i ].ubPendingGroupID &&
						EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) &&
						GarrisonRequestingMinimumReinforcements( i ) )
				{ //This is the group that gets the reinforcements!
					if( ReinforcementsApproved( i, &usDefencePoints ) )
					{
						SendReinforcementsForGarrison( i, usDefencePoints, pGroup );
						return;
					}
				}
				if( uiReloopLastIndex == (size_t)-1 )
				{ //go to the next garrison and clear the iRandom value so it attempts to use all subsequent groups.
					uiReloopLastIndex = i - 1;
					iRandom = 0;
				}
			}
			//Decrease the iRandom value until it hits 0.  When that happens, all garrisons will get considered until
			//we either have a match or process all of the garrisons.
			iRandom -= iWeight;
		}
	}
	if( uiReloopLastIndex >= 0 )
	{ //Process the loop again to the point where the original random slot started considering, and consider
		//all of the garrisons.  If this fails, all patrol groups will be considered next.
		for (size_t i = 0; i <= uiReloopLastIndex; i++)
		{
			RecalculateGarrisonWeight( i );
			iWeight = gGarrisonGroup[ i ].bWeight;
			if( iWeight > 0 )
			{	//if group is requesting reinforcements.
				if( !gGarrisonGroup[ i ].ubPendingGroupID &&
						EnemyPermittedToAttackSector( NULL, gGarrisonGroup[ i ].ubSectorID ) &&
						GarrisonRequestingMinimumReinforcements( i ) )
				{ //This is the group that gets the reinforcements!
					if( ReinforcementsApproved( i, &usDefencePoints ) )
					{
						SendReinforcementsForGarrison( i, usDefencePoints, pGroup );
						return;
					}
				}
			}
		}
	}
	if( uiReloopLastIndex == (size_t)-1 )
	{
		//go through the patrol groups
		for (size_t i = 0; i < gPatrolGroup.size(); i++)
		{
			RecalculatePatrolWeight(gPatrolGroup[i]);
			iWeight = gPatrolGroup[ i ].bWeight;
			if( iWeight > 0 )
			{
				if( iRandom < iWeight )
				{
					if( !gPatrolGroup[ i ].ubPendingGroupID && PatrolRequestingMinimumReinforcements( i ) )
					{ //This is the group that gets the reinforcements!
						SendReinforcementsForPatrol( i, pGroup );
						return;
					}
				}
				if( uiReloopLastIndex == (size_t)-1 )
				{
					uiReloopLastIndex = i - 1;
					iRandom = 0;
				}
				iRandom -= iWeight;
			}
		}
	}
	else
	{
		uiReloopLastIndex = gPatrolGroup.size() - 1;
	}

	for (size_t i = 0; i <= uiReloopLastIndex; i++)
	{
		RecalculatePatrolWeight(gPatrolGroup[i]);
		iWeight = gPatrolGroup[ i ].bWeight;
		if( iWeight > 0 )
		{
			if( !gPatrolGroup[ i ].ubPendingGroupID && PatrolRequestingMinimumReinforcements( i ) )
			{ //This is the group that gets the reinforcements!
				SendReinforcementsForPatrol( i, pGroup );
				return;
			}
		}
	}
	TransferGroupToPool( pGroup );
}


/* When an enemy AI group is eliminated by the player, apply a grace period in
 * which the group isn't allowed to be filled for several days. */
static void TagSAIGroupWithGracePeriod(GROUP const& g)
{
	size_t const patrol_id = FindPatrolGroupIndexForGroupID(g.ubGroupID);
	if (patrol_id == (size_t)-1) return;

	UINT32 grace_period = saipolicy_by_diff(patrol_grace_period_in_days);
	gPatrolGroup[patrol_id].bFillPermittedAfterDayMod100 = (GetWorldDay() + grace_period) % 100;
}


static BOOLEAN PermittedToFillPatrolGroup(INT32 iPatrolID)
{
	INT32 iDay;
	INT32 iDayAllowed;
	iDay = GetWorldDay();
	iDayAllowed = gPatrolGroup[ iPatrolID ].bFillPermittedAfterDayMod100 + (iDay / 100) * 100;
	return iDay >= iDayAllowed;
}

void RepollSAIGroup( GROUP *pGroup )
{
	UINT32 i;
	Assert( !pGroup->fPlayer );
	if( GroupAtFinalDestination( pGroup ) )
	{
		EvaluateGroupSituation( pGroup );
		return;
	}
	for( i = 0; i < gPatrolGroup.size(); i++ )
	{
		if( gPatrolGroup[ i ].ubGroupID == pGroup->ubGroupID )
		{
			RecalculatePatrolWeight(gPatrolGroup[i]); //in case there are any dead enemies
			CalculateNextMoveIntention( pGroup );
			return;
		}
	}
	for( i = 0; i < gGarrisonGroup.size(); i++ )
	{
		//KM : August 6, 1999 Patch fix
		//     Ack, wasn't checking for the matching group to garrison
		if( gGarrisonGroup[ i ].ubPendingGroupID == pGroup->ubGroupID )
		//end
		{
			RecalculateGarrisonWeight( i ); //in case there are any dead enemies
			CalculateNextMoveIntention( pGroup );
			return;
		}
	}
}


static void CalcNumTroopsBasedOnComposition(UINT8* pubNumTroops, UINT8* pubNumElites, UINT8 ubTotal, INT32 iCompositionID)
{
	*pubNumTroops = gArmyComp[ iCompositionID ].bTroopPercentage * ubTotal / 100;
	*pubNumElites = gArmyComp[ iCompositionID ].bElitePercentage * ubTotal / 100;

	//Due to low roundoff, it is highly possible that we will be short one soldier.
	while( *pubNumTroops + *pubNumElites < ubTotal )
	{
		if( Chance( gArmyComp[ iCompositionID ].bTroopPercentage ) )
		{
			(*pubNumTroops)++;
		}
		else
		{
			(*pubNumElites)++;
		}
	}
	Assert( *pubNumTroops + *pubNumElites == ubTotal );
}


static void ConvertGroupTroopsToComposition(GROUP* pGroup, INT32 iCompositionID)
{
	Assert( pGroup );
	Assert( !pGroup->fPlayer );
	CalcNumTroopsBasedOnComposition( &pGroup->pEnemyGroup->ubNumTroops, &pGroup->pEnemyGroup->ubNumElites, pGroup->ubGroupSize, iCompositionID );
	pGroup->pEnemyGroup->ubNumAdmins = 0;
	pGroup->ubGroupSize = pGroup->pEnemyGroup->ubNumTroops + pGroup->pEnemyGroup->ubNumElites;
	ValidateLargeGroup( pGroup );
}


static void RemoveSoldiersFromGarrisonBasedOnComposition(INT32 iGarrisonID, UINT8 ubSize)
{
	SECTORINFO *pSector;
	INT32 iCompositionID;
	UINT8 ubNumTroops, ubNumElites;

	iCompositionID = gGarrisonGroup[ iGarrisonID ].ubComposition;

	CalcNumTroopsBasedOnComposition( &ubNumTroops, &ubNumElites, ubSize, iCompositionID );
	pSector = &SectorInfo[ gGarrisonGroup[ iGarrisonID ].ubSectorID ];
	//if there are administrators in this sector, remove them first.

	while( ubSize && pSector->ubNumAdmins )
	{
		pSector->ubNumAdmins--;
		ubSize--;
		if( ubNumTroops )
		{
			ubNumTroops--;
		}
		else
		{
			ubNumElites--;
		}
	}
	//No administrators are left.

	//Eliminate the troops
	while( ubNumTroops )
	{
		if( pSector->ubNumTroops )
		{
			pSector->ubNumTroops--;
		}
		else if( pSector->ubNumElites )
		{
			pSector->ubNumElites--;
		}
		else
		{
			SLOGA("RemoveSoldiersFromGarrisonBasedOnComposition: trying to eliminate more troops than present");
		}
		ubNumTroops--;
	}

	//Eliminate the elites
	while( ubNumElites )
	{
		if( pSector->ubNumElites )
		{
			pSector->ubNumElites--;
		}
		else if( pSector->ubNumTroops )
		{
			pSector->ubNumTroops--;
		}
		else
		{
			SLOGA("RemoveSoldiersFromGarrisonBasedOnComposition: trying to eliminate more elites than present");
		}
		ubNumElites--;
	}

	RecalculateGarrisonWeight( iGarrisonID );
}


static void MoveSAIGroupToSector(GROUP** const pGroup, UINT8 const sector, SAIMOVECODE const move_code, UINT8 const intention)
{
	SGPSector sSector(sector);
	GROUP& g = **pGroup;

	if (g.fBetweenSectors) SetEnemyGroupSector(g, g.ubSector.AsByte());

	g.pEnemyGroup->ubIntention = intention;
	g.ubMoveType               = ONE_WAY;

	/* Make sure that the group isn't moving into a garrison sector. These sectors
	 * should be using ASSAULT intentions! */
	Assert(intention != PURSUIT || SectorInfo[sector].ubGarrisonID == NO_GARRISON);

	/* If the destination sector is the current location. Instead of causing code
	 * logic problems, simply process them as if they just arrived. */
	if (g.ubSector == sSector && EvaluateGroupSituation(&g))
	{ // The group was deleted.
		*pGroup = 0;
		return;
	}

	switch (move_code)
	{
		case STAGE:   MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectorsAndStopOneSectorBeforeEnd(g, g.ubSector, sSector); break;
		case EVASIVE: MoveGroupFromSectorToSectorButAvoidPlayerInfluencedSectors(                         g, g.ubSector, sSector); break;
		case DIRECT:  MoveGroupFromSectorToSector(                                                        g, g.ubSector, sSector); break;
	}
	/* Make sure that the group is moving. If this fails, then the pathing may
	 * have failed for some reason. */
	ValidateGroup(&g);
}


//If there are any enemy groups that will be moving through this sector due, they will have to repath which
//will cause them to avoid the sector.  Returns the number of redirected groups.
static UINT8 RedirectEnemyGroupsMovingThroughSector(const SGPSector& sSector)
{
	UINT8 ubNumGroupsRedirected = 0;
	WAYPOINT *pWaypoint;
	UINT8 ubDestSectorID;
	FOR_EACH_ENEMY_GROUP(pGroup)
	{
		if (pGroup->ubMoveType == ONE_WAY)
		{ //check the waypoint list
			if (GroupWillMoveThroughSector(pGroup, sSector))
			{
				//extract the group's destination.
				pWaypoint = GetFinalWaypoint( pGroup );
				Assert( pWaypoint );
				ubDestSectorID = pWaypoint->sSector.AsByte();
				SetEnemyGroupSector(*pGroup, pGroup->ubSector.AsByte());
				MoveSAIGroupToSector( &pGroup, ubDestSectorID, EVASIVE, pGroup->pEnemyGroup->ubIntention );
				ubNumGroupsRedirected++;
			}
		}
	}
	if( ubNumGroupsRedirected )
	{
		STLOGD("Test message for new feature: {} enemy groups were redirected away from moving through sector {}.  Please don't report unless this number is greater than 5.",
			ubNumGroupsRedirected, sSector.AsShortString());
	}
	return ubNumGroupsRedirected;
}


//when the SAI compositions change, it is necessary to call this function upon version load,
//to reflect the changes of the compositions to the sector that haven't been visited yet.
static void ReinitializeUnvisitedGarrisons(void)
{
	SECTORINFO *pSector;
	ARMY_COMPOSITION *pArmyComp;
	GROUP *pGroup;
	UINT32 i, cnt, uiEliteChance, uiAdminChance;

	//Recreate the compositions
	gArmyComp = GCM->getArmyCompositions();
	EvolveQueenPriorityPhase( TRUE );

	//Go through each unvisited sector and recreate the garrison forces based on
	//the desired population.
	for( i = 0; i < gGarrisonGroup.size(); i++ )
	{
		if( gGarrisonGroup[ i ].ubComposition >= LEVEL1_DEFENCE && gGarrisonGroup[ i ].ubComposition <= LEVEL3_DEFENCE )
		{ //These 3 compositions make up the perimeter around Meduna.  The existance of these are based on the
			//difficulty level, and we don't want to reset these anyways, due to the fact that many of the reinforcements
			//come from these sectors, and it could potentially add upwards of 150 extra troops which would seriously
			//unbalance the difficulty.
			continue;
		}
		pSector = &SectorInfo[ gGarrisonGroup[ i ].ubSectorID ];
		pArmyComp = &gArmyComp[ gGarrisonGroup[ i ].ubComposition ];
		if( !(pSector->uiFlags & SF_ALREADY_VISITED) )
		{
			pSector->ubNumAdmins = 0;
			pSector->ubNumTroops = 0;
			pSector->ubNumElites = 0;
			if( gfQueenAIAwake )
			{
				cnt = pArmyComp->bDesiredPopulation;
			}
			else
			{
				cnt = pArmyComp->bStartPopulation;
			}

			if( gGarrisonGroup[ i ].ubPendingGroupID )
			{ //if the garrison has reinforcements on route, then subtract the number of
				//reinforcements from the value we reset the size of the garrison.  This is to
				//prevent overfilling the group.
				pGroup = GetGroup( gGarrisonGroup[ i ].ubPendingGroupID );
				if( pGroup )
				{
					cnt -= pGroup->ubGroupSize;
					cnt = MAX( cnt, 0 );
				}
			}

			uiEliteChance = pArmyComp->bElitePercentage;
			uiAdminChance = pArmyComp->bAdminPercentage;
			if( uiAdminChance && !gfQueenAIAwake && cnt )
			{
				pSector->ubNumAdmins = uiAdminChance * cnt / 100;
			}
			else while( cnt-- )
			{ //for each person, randomly determine the types of each soldier.
				if( Chance( uiEliteChance ) )
				{
					pSector->ubNumElites++;
				}
				else
				{
					pSector->ubNumTroops++;
				}
			}
		}
	}
}


#ifdef WITH_UNITTESTS
#include "gtest/gtest.h"

TEST(StrategicAI, asserts)
{
	EXPECT_EQ(sizeof(ARMY_COMPOSITION), 20u);
	EXPECT_EQ(sizeof(PATROL_GROUP), 20u);
	EXPECT_EQ(sizeof(GARRISON_GROUP), 14u);
}

#endif
