#include "Strategic_Movement.h"
#include "Animation_Data.h"
#include "Auto_Resolve.h"
#include "BloodCatSpawnsModel.h"
#include "Campaign.h"
#include "ContentManager.h"
#include "Dialogue_Control.h"
#include "Faces.h"
#include "FileMan.h"
#include "Font_Control.h"
#include "Game_Clock.h"
#include "Game_Events.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Inventory_Choosing.h"
#include "Items.h"
#include "JAScreens.h"
#include "LoadSaveData.h"
#include "Map_Information.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface.h"
#include "Map_Screen_Interface_Border.h"
#include "Map_Screen_Interface_Bottom.h"
#include "MapScreen.h"
#include "Meanwhile.h"
#include "MercProfile.h"
#include "Message.h"
#include "Music_Control.h"
#include "Overhead.h"
#include "Player_Command.h"
#include "PreBattle_Interface.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Soldier_Macros.h"
#include "Squads.h"
#include "Strategic.h"
#include "StrategicMap_Secrets.h"
#include "Strategic_AI.h"
#include "Strategic_Pathing.h"
#include "Tactical_Save.h"
#include "Text.h"
#include "Town_Militia.h"
#include "Video.h"
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <string_theory/format>
#include <string_theory/string>


// the delay for a group about to arrive
#define ABOUT_TO_ARRIVE_DELAY 5

GROUP *gpGroupList;

static GROUP* gpPendingSimultaneousGroup = NULL;

extern BOOLEAN gubNumAwareBattles;
extern INT8 SquadMovementGroups[ ];
extern INT8 gubVehicleMovementGroups[ ];

BOOLEAN gfDelayAutoResolveStart = FALSE;


static BOOLEAN gfRandomizingPatrolGroup = FALSE;

UINT8 gubNumGroupsArrivedSimultaneously = 0;

//Doesn't require text localization.  This is for debug strings only.
static const char* const gszTerrain[NUM_TRAVTERRAIN_TYPES] =
{
	"TOWN",
	"ROAD",
	"PLAINS",
	"SAND",
	"SPARSE",
	"DENSE",
	"SWAMP",
	"WATER",
	"HILLS",
	"GROUNDBARRIER",
	"NS_RIVER",
	"EW_RIVER",
	"EDGEOFWORLD"
};

BOOLEAN gfUndergroundTacticalTraversal = FALSE;

// remembers which player group is the Continue/Stop prompt about?  No need to save as long as you can't save while prompt ON
static GROUP* gpGroupPrompting = NULL;

static UINT32 uniqueIDMask[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };


static GROUP* gpInitPrebattleGroup = NULL;

// waiting for input from user
static BOOLEAN gfWaitingForInput = FALSE;


static UINT8 AddGroupToList(GROUP* pGroup);


//Player grouping functions
//.........................
//Creates a new player group, returning the unique ID of that group.  This is the first
//step before adding waypoints and members to the player group.
GROUP* CreateNewPlayerGroupDepartingFromSector(UINT8 const ubSectorX, UINT8 const ubSectorY)
{
	AssertMsg( ubSectorX >= 1 && ubSectorX <= 16, String( "CreateNewPlayerGroup with out of range sectorX value of %d", ubSectorX ) );
	AssertMsg( ubSectorY >= 1 && ubSectorY <= 16, String( "CreateNewPlayerGroup with out of range sectorY value of %d", ubSectorY ) );
	GROUP* const pNew = new GROUP{};
	pNew->pPlayerList = NULL;
	pNew->pWaypoints = NULL;
	pNew->ubSector.x = pNew->ubNext.x = ubSectorX;
	pNew->ubSector.y = pNew->ubNext.y = ubSectorY;
	pNew->ubOriginalSector = (UINT8)SECTOR( ubSectorX, ubSectorY );
	pNew->fPlayer = TRUE;
	pNew->ubMoveType = ONE_WAY;
	pNew->ubNextWaypointID = 0;
	pNew->ubTransportationMask = FOOT;
	pNew->fVehicle = FALSE;
	pNew->ubCreatedSectorID = pNew->ubOriginalSector;
	pNew->ubSectorIDOfLastReassignment = 255;

	AddGroupToList(pNew);
	return pNew;
}


GROUP* CreateNewVehicleGroupDepartingFromSector(UINT8 const ubSectorX, UINT8 const ubSectorY)
{
	AssertMsg( ubSectorX >= 1 && ubSectorX <= 16, String( "CreateNewVehicleGroup with out of range sectorX value of %d", ubSectorX ) );
	AssertMsg( ubSectorY >= 1 && ubSectorY <= 16, String( "CreateNewVehicleGroup with out of range sectorY value of %d", ubSectorY ) );
	GROUP* const pNew = new GROUP{};
	pNew->pWaypoints = NULL;
	pNew->ubSector.x = pNew->ubNext.x = ubSectorX;
	pNew->ubSector.y = pNew->ubNext.y = ubSectorY;
	pNew->ubOriginalSector = (UINT8)SECTOR( ubSectorX, ubSectorY );
	pNew->ubMoveType = ONE_WAY;
	pNew->ubNextWaypointID = 0;
	pNew->fVehicle = TRUE;
	pNew->fPlayer = TRUE;
	pNew->pPlayerList = NULL;
	pNew->ubCreatedSectorID = pNew->ubOriginalSector;
	pNew->ubSectorIDOfLastReassignment = 255;

	// get the type
	pNew->ubTransportationMask = CAR;

	AddGroupToList(pNew);
	return pNew;
}


void AddPlayerToGroup(GROUP& g, SOLDIERTYPE& s)
{
	AssertMsg(g.fPlayer, "Attempting AddPlayerToGroup() on an ENEMY group!");

	PLAYERGROUP* const p = new PLAYERGROUP{};
	p->pSoldier = &s;
	p->next     = 0;

	s.ubGroupID = g.ubGroupID;

	PLAYERGROUP* i = g.pPlayerList;
	if (!i)
	{
		g.pPlayerList = p;
		g.ubGroupSize = 1;
		g.ubPrev.x    = s.ubPrevSectorID % 16 + 1;
		g.ubPrev.y    = s.ubPrevSectorID / 16 + 1;
		g.ubSector.x   = s.sSectorX;
		g.ubSector.y   = s.sSectorY;
		g.ubSector.z   = s.bSectorZ;
	}
	else
	{
		for (; i->next; i = i->next)
		{
			AssertMsg(i->pSoldier->ubProfile != s.ubProfile, String("Attempting to add an already existing merc to group (ubProfile=%d).", s.ubProfile));
		}
		i->next = p;

		++g.ubGroupSize;
	}
}


static void CancelEmptyPersistentGroupMovement(GROUP&);


void RemovePlayerFromPGroup(GROUP& g, SOLDIERTYPE& s)
{
	AssertMsg(g.fPlayer, "Attempting RemovePlayerFromGroup() on an ENEMY group!");

	for (PLAYERGROUP** i = &g.pPlayerList; *i; i = &(*i)->next)
	{
		PLAYERGROUP* const p = *i;
		if (p->pSoldier != &s) continue;

		*i = p->next;
		delete p;

		s.ubPrevSectorID = g.ubPrev.AsByte();
		s.ubGroupID      = 0;

		if (--g.ubGroupSize == 0)
		{
			if (!g.fPersistant)
			{
				RemoveGroup(g);
			}
			else
			{
				CancelEmptyPersistentGroupMovement(g);
			}
		}
		break;
	}
}


void RemovePlayerFromGroup(SOLDIERTYPE& s)
{
	GROUP* const pGroup = GetGroup(s.ubGroupID);

	//KM : August 6, 1999 Patch fix
	//     Because the release build has no assertions, it was still possible for the group to be null,
	//     causing a crash.  Instead of crashing, it'll simply return false.
	if (!pGroup) return;
	//end

	AssertMsg(pGroup, String("Attempting to RemovePlayerFromGroup( %d, %d ) from non-existant group", s.ubGroupID, s.ubProfile));

	RemovePlayerFromPGroup(*pGroup, s);
}


static void SetLocationOfAllPlayerSoldiersInGroup(GROUP const&, const SGPSector& sSector);


BOOLEAN GroupReversingDirectionsBetweenSectors( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY, BOOLEAN fBuildingWaypoints )
{
	// if we're not between sectors, or we are but we're continuing in the same direction as before
	if ( !GroupBetweenSectorsAndSectorXYIsInDifferentDirection( pGroup, ubSectorX, ubSectorY ) )
	{
		// then there's no need to reverse directions
		return FALSE;
	}

	//The new direction is reversed, so we have to go back to the sector we just left.

	//Search for the arrival event, and kill it!
	DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );

	//Adjust the information in the group to reflect the new movement.
	pGroup->ubPrev = pGroup->ubNext;
	pGroup->ubNext = pGroup->ubSector;
	pGroup->ubSector = pGroup->ubPrev;

	if( pGroup->fPlayer )
	{
		// ARM: because we've changed the group's ubSectoryX and ubSectorY, we must now also go and change the sSectorX and
		// sSectorY of all the soldiers in this group so that they stay in synch.  Otherwise pathing and movement problems
		// will result since the group is in one place while the merc is in another...
		SetLocationOfAllPlayerSoldiersInGroup(*pGroup, SGPSector(pGroup->ubSector.x, pGroup->ubSector.y, 0));
	}


	// IMPORTANT: The traverse time doesn't change just because we reverse directions!  It takes the same time no matter
	// which direction you're going in!  This becomes critical in case the player reverse directions again before moving!

	// The time it takes to arrive there will be exactly the amount of time we have been moving away from it.
	pGroup->setArrivalTime(pGroup->uiTraverseTime - pGroup->uiArrivalTime + GetWorldTotalMin() * 2);

	// if they're not already there
	if( pGroup->uiArrivalTime > GetWorldTotalMin() )
	{
		//Post the replacement event to move back to the previous sector!
		AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );

		if( pGroup->fPlayer )
		{
			if( ( pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY ) > GetWorldTotalMin( ) )
			{
				// Post the about to arrive event
				AddStrategicEvent( EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID );
			}
		}
	}
	else
	{
		// IMPORTANT: this can't be called during RebuildWayPointsForGroupPath(), since it will clear the mercpath
		// prematurely by assuming the mercs are now at their final destination when only the first waypoint is in place!!!
		// To handle this situation, RebuildWayPointsForGroupPath() will issue it's own call after it's ready for it.
		if ( !fBuildingWaypoints )
		{
			// never really left.  Must set check for battle TRUE in order for HandleNonCombatGroupArrival() to run!
			GroupArrivedAtSector(*pGroup, TRUE, TRUE);
		}
	}


	return TRUE;
}



BOOLEAN GroupBetweenSectorsAndSectorXYIsInDifferentDirection( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY )
{
	INT32 currDX, currDY, newDX, newDY;
	UINT8 ubNumUnalignedAxes = 0;


	if( !pGroup->fBetweenSectors )
		return( FALSE );


	// Determine the direction the group is currently traveling in
	currDX = pGroup->ubNext.x - pGroup->ubSector.x;
	currDY = pGroup->ubNext.y - pGroup->ubSector.y;

	//Determine the direction the group would need to travel in to reach the given sector
	newDX = ubSectorX - pGroup->ubSector.x;
	newDY = ubSectorY - pGroup->ubSector.y;

	// clip the new dx/dy values to +/- 1
	if( newDX )
	{
		ubNumUnalignedAxes++;
		newDX /= ABS( newDX );
	}
	if( newDY )
	{
		ubNumUnalignedAxes++;
		newDY /= ABS( newDY );
	}

	// error checking
	if( ubNumUnalignedAxes > 1 )
	{
		AssertMsg( FALSE, String( "Checking a diagonal move for direction change, groupID %d. AM-0", pGroup->ubGroupID ) );
		return FALSE;
	}

	// Compare the dx/dy's.  If they're exactly the same, group is travelling in the same direction as before, so we're not
	// changing directions.
	// Note that 90-degree orthogonal changes are considered changing direction, as well as the full 180-degree reversal.
	// That's because the party must return to the previous sector in each of those cases, too.
	if( currDX == newDX && currDY == newDY )
		return( FALSE );


	// yes, we're between sectors, and we'd be changing direction to go to the given sector
	return( TRUE );
}


static void InitiateGroupMovementToNextSector(GROUP* pGroup);


/* Appends a waypoint to the end of the list. Waypoint MUST be on the same
 * horizontal or vertical level as the last waypoint added. */
BOOLEAN AddWaypointToPGroup(GROUP *pGroup, const SGPSector& ubSector)
{
	return AddWaypointToPGroup(pGroup, ubSector.x, ubSector.y);
}

BOOLEAN AddWaypointToPGroup(GROUP* const g, UINT8 const x, UINT8 const y) // Same, but overloaded
{
	AssertMsg(1 <= x && x <= 16, String("AddWaypointToPGroup with out of range sectorX value of %d", x));
	AssertMsg(1 <= y && y <= 16, String("AddWaypointToPGroup with out of range sectorY value of %d", y));

	if (!g) return FALSE;

	/* At this point, we have the group, and a valid coordinate. Now we must
	 * determine that this waypoint will be aligned exclusively to either the x or
	 * y axis of the last waypoint in the list. */
	UINT8     n_aligned_axes      = 0;
	bool      reversing_direction = FALSE;
	WAYPOINT* wp                  = g->pWaypoints;
	if (!wp)
	{
		if (GroupReversingDirectionsBetweenSectors(g, x, y, TRUE))
		{
			if (g->fPlayer)
			{ /* Because we reversed, we must add the new current sector back at the
				 * head of everyone's mercpath */
				AddSectorToFrontOfMercPathForAllSoldiersInGroup(g, g->ubSector);
			}

			/* Very special case that requiring specific coding. Check out the
			 * comments at the above function for more information. */
			reversing_direction = TRUE;
			// ARM:  Kris - new rulez.  Must still fall through and add a waypoint anyway!!!
		}
		else
		{ // No waypoints, so compare against the current location.
			if (g->ubSector.x == x) ++n_aligned_axes;
			if (g->ubSector.y == y) ++n_aligned_axes;
		}
	}
	else
	{	//we do have a waypoint list, so go to the last entry
		while (wp->next)
		{
			wp = wp->next;
		}
		// Now, we are pointing to the last waypoint in the list
		if (wp->x == x) ++n_aligned_axes;
		if (wp->y == y) ++n_aligned_axes;
	}

	if (!reversing_direction)
	{
		if (n_aligned_axes == 0)
		{
			AssertMsg(FALSE, String("Invalid DIAGONAL waypoint being added for groupID %d. AM-0", g->ubGroupID));
			return FALSE;
		}

		if (n_aligned_axes >= 2)
		{
			AssertMsg(FALSE, String("Invalid IDENTICAL waypoint being added for groupID %d. AM-0", g->ubGroupID));
			return FALSE;
		}

		// Has to be different in exactly 1 axis to be a valid new waypoint
		Assert(n_aligned_axes == 1);
	}

	WAYPOINT* const new_wp = new WAYPOINT{};
	new_wp->x    = x;
	new_wp->y    = y;
	new_wp->next = 0;

	if (wp)
	{ // Add the waypoint to the end of the list
		wp->next = new_wp;
	}
	else
	{ // We are adding the first waypoint.
		g->pWaypoints = new_wp;

		/* Important: The first waypoint added actually initiates the group's
		 * movement to the next sector. */
		/* Don't do this if we have reversed directions! In that case, the required
		 * work has already been done back there */
		if (!reversing_direction)
		{ /* We need to calculate the next sector the group is moving to and post an
			 * event for it. */
			InitiateGroupMovementToNextSector(g);
		}
	}

	if (g->fPlayer)
	{ // Also, nuke any previous "tactical traversal" information.
		CFOR_EACH_PLAYER_IN_GROUP(curr, g)
		{
			curr->pSoldier->ubStrategicInsertionCode = 0;
		}
	}

	return TRUE;
}


// NOTE: This does NOT expect a strategic sector ID
BOOLEAN AddWaypointIDToPGroup( GROUP *pGroup, UINT8 ubSectorID )
{
	UINT8 ubSectorX, ubSectorY;
	ubSectorX = SECTORX( ubSectorID );
	ubSectorY = SECTORY( ubSectorID );
	return AddWaypointToPGroup( pGroup, ubSectorX, ubSectorY );
}


BOOLEAN AddWaypointStrategicIDToPGroup( GROUP *pGroup, UINT32 uiSectorID )
{
	UINT8 ubSectorX, ubSectorY;
	ubSectorX = ( UINT8 ) GET_X_FROM_STRATEGIC_INDEX( uiSectorID );
	ubSectorY = ( UINT8 ) GET_Y_FROM_STRATEGIC_INDEX( uiSectorID );
	return AddWaypointToPGroup( pGroup, ubSectorX, ubSectorY );
}


//Enemy grouping functions -- private use by the strategic AI.
//............................................................
GROUP* CreateNewEnemyGroupDepartingFromSector( UINT32 uiSector, UINT8 ubNumAdmins, UINT8 ubNumTroops, UINT8 ubNumElites )
{
	AssertMsg( uiSector <= 255, String( "CreateNewEnemyGroup with out of range value of %d", uiSector ) );
	GROUP* const pNew = new GROUP{};
	pNew->pEnemyGroup = new ENEMYGROUP{};
	pNew->pWaypoints = NULL;
	pNew->ubSector = SGPSector(uiSector);
	pNew->ubOriginalSector = (UINT8)uiSector;
	pNew->fPlayer = FALSE;
	pNew->ubMoveType = CIRCULAR;
	pNew->ubNextWaypointID = 0;
	pNew->pEnemyGroup->ubNumAdmins = ubNumAdmins;
	pNew->pEnemyGroup->ubNumTroops = ubNumTroops;
	pNew->pEnemyGroup->ubNumElites = ubNumElites;
	pNew->ubGroupSize = (UINT8)(ubNumTroops + ubNumElites);
	pNew->ubTransportationMask = FOOT;
	pNew->fVehicle = FALSE;
	pNew->ubCreatedSectorID = pNew->ubOriginalSector;
	pNew->ubSectorIDOfLastReassignment = 255;

	AddGroupToList(pNew);
	return pNew;
}

//INTERNAL LIST MANIPULATION FUNCTIONS

//When adding any new group to the list, this is what must be done:
//1)  Find the first unused ID (unique)
//2)  Assign that ID to the new group
//3)  Insert the group at the end of the list.
static UINT8 AddGroupToList(GROUP* const g)
{
	// First, find a unique ID
	for (UINT8 id = 0; ++id;)
	{
		const UINT32 index = id / 32;
		const UINT32 bit   = id % 32;
		const UINT32 mask  = 1 << bit;
		if (uniqueIDMask[index] & mask) continue;

		// Found a free id
		g->ubGroupID         = id;
		uniqueIDMask[index] |= mask;

		// Append group to list
		GROUP** i = &gpGroupList;
		while (*i != NULL) i = &(*i)->next;
		*i = g;

		return id;
	}
	throw std::runtime_error("Out of group IDs");
}


/* Destroys the waypoint list, detaches group from list, then deallocated the
 * memory for the group */
static void RemoveGroupFromList(GROUP* const g)
{
	for (GROUP** i = &gpGroupList; *i != NULL; i = &(*i)->next)
	{
		if (*i != g) continue;

		// Found the group, so now remove it.
		*i = g->next;

		// Clear the unique group ID
		const UINT32 index = g->ubGroupID / 32;
		const UINT32 bit   = g->ubGroupID % 32;
		const UINT32 mask  = 1 << bit;
		Assert(uniqueIDMask[index] & mask);
		uniqueIDMask[index] &= ~mask;

		delete g;
		return;
	}
	SLOGA("Trying to remove a strategic group that isn't in the list!");
}


GROUP* GetGroup( UINT8 ubGroupID )
{
	FOR_EACH_GROUP(curr)
	{
		if( curr->ubGroupID == ubGroupID )
			return curr;
	}
	return NULL;
}


class CharacterDialogueEventBeginPrebattleInterface : public CharacterDialogueEvent
{
	public:
		CharacterDialogueEventBeginPrebattleInterface(SOLDIERTYPE& soldier, GROUP* const initiating_battle_group) :
			CharacterDialogueEvent(soldier),
			initiating_battle_group_(initiating_battle_group)
		{}

		bool Execute()
		{
			if (!MayExecute()) return true;

			SOLDIERTYPE const& s = soldier_;
			ExecuteCharacterDialogue(s.ubProfile, QUOTE_ENEMY_PRESENCE, s.face, DIALOGUE_TACTICAL_UI, TRUE, false);

			// Setup face with data!
			FACETYPE& f = *gpCurrentTalkingFace;
			f.uiFlags                   |= FACE_TRIGGER_PREBATTLE_INT;
			f.u.initiating_battle.group  = initiating_battle_group_;

			return false;
		}

	private:
		GROUP* const initiating_battle_group_;
};


static void HandleImportantPBIQuote(SOLDIERTYPE& s, GROUP* const initiating_battle_group)
{
	// Wake merc up for THIS quote
	bool const asleep = s.fMercAsleep;
	if (asleep) MakeCharacterDialogueEventSleep(s, false);
	DialogueEvent::Add(new CharacterDialogueEventBeginPrebattleInterface(s, initiating_battle_group));
	if (asleep) MakeCharacterDialogueEventSleep(s, true);
}


//If this is called, we are setting the game up to bring up the prebattle interface.  Before doing so,
//one of the involved mercs will pipe up.  When he is finished, we automatically go into the mapscreen,
//regardless of the mode we are in.
static void PrepareForPreBattleInterface(GROUP* pPlayerDialogGroup, GROUP* pInitiatingBattleGroup)
{
	// ATE; Changed alogrithm here...
	// We first loop through the group and save ubID's ov valid guys to talk....
	// ( Can't if sleeping, unconscious, and EPC, etc....
	UINT8 ubNumMercs = 0;

	if( fDisableMapInterfaceDueToBattle )
	{
		SLOGA("fDisableMapInterfaceDueToBattle is set before attempting to bring up PBI.\
			Please send PRIOR save if possible and details on anything that just happened before this battle." );
		return;
	}

	// Pipe up with quote...
	AssertMsg( pPlayerDialogGroup, "Didn't get a player dialog group for prebattle interface." );

	AssertMsg(pPlayerDialogGroup->pPlayerList, String( "Player group %d doesn't have *any* players in it!  (Finding dialog group)", pPlayerDialogGroup->ubGroupID));

	SOLDIERTYPE* mercs_in_group[20];
	CFOR_EACH_PLAYER_IN_GROUP(pPlayer, pPlayerDialogGroup)
	{
		SOLDIERTYPE* const pSoldier = pPlayer->pSoldier;

		if (pSoldier->bLife >= OKLIFE && !IsMechanical(*pSoldier) && !AM_AN_EPC(pSoldier))
		{
			mercs_in_group[ubNumMercs++] = pSoldier;
		}
	}

	//Set music
	SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

	if( (gfTacticalTraversal && pInitiatingBattleGroup == gpTacticalTraversalGroup) ||
		(pInitiatingBattleGroup && !pInitiatingBattleGroup->fPlayer &&
		pInitiatingBattleGroup->ubSector == gWorldSector && !gWorldSector.z))
	{	// At least say quote....
		if ( ubNumMercs > 0 )
		{
			if( pPlayerDialogGroup->uiFlags & GROUPFLAG_JUST_RETREATED_FROM_BATTLE )
			{
				gfCantRetreatInPBI = TRUE;
			}

			SOLDIERTYPE* const chosen = mercs_in_group[Random(ubNumMercs)];
			gpTacticalTraversalChosenSoldier = chosen;

			if( !gfTacticalTraversal )
			{
				HandleImportantPBIQuote(*chosen, pInitiatingBattleGroup);
			}

			InterruptTime();
			PauseGame();
			LockPauseState(LOCK_PAUSE_PREBATTLE_CURRENT_SQUAD);

			if( !gfTacticalTraversal )
				fDisableMapInterfaceDueToBattle = TRUE;
		}
		return;
	}


	// Randomly pick a valid merc from the list we have created!
	if ( ubNumMercs > 0 )
	{
		if( pPlayerDialogGroup->uiFlags & GROUPFLAG_JUST_RETREATED_FROM_BATTLE )
		{
			gfCantRetreatInPBI = TRUE;
		}

		SOLDIERTYPE* const chosen = mercs_in_group[Random(ubNumMercs)];
		HandleImportantPBIQuote(*chosen, pInitiatingBattleGroup);
		InterruptTime();
		PauseGame();
		LockPauseState(LOCK_PAUSE_PREBATTLE);

		// disable exit from mapscreen and what not until face done talking
		fDisableMapInterfaceDueToBattle = TRUE;
	}
	else
	{
		// ATE: What if we have unconscious guys, etc....
		// We MUST start combat, but donot play quote...
		InitPreBattleInterface(pInitiatingBattleGroup, true);
	}
}


static void HandleOtherGroupsArrivingSimultaneously(UINT8 x, UINT8 y, UINT8 z);
static void NotifyPlayerOfBloodcatBattle(const SGPSector& ubSector);
static BOOLEAN TestForBloodcatAmbush(GROUP const*);
static void TriggerPrebattleInterface(MessageBoxReturnValue);
static BOOLEAN PossibleToCoordinateSimultaneousGroupArrivals(GROUP* pFirstGroup);


static BOOLEAN CheckConditionsForBattle(GROUP* pGroup)
{
	GROUP *pPlayerDialogGroup = NULL;
	SOLDIERTYPE *pSoldier;
	BOOLEAN fBattlePending = FALSE;
	BOOLEAN fAliveMerc = FALSE;
	BOOLEAN fMilitiaPresent = FALSE;
	BOOLEAN fCombatAbleMerc = FALSE;
	BOOLEAN fBloodCatAmbush = FALSE;
	SGPSector gSector = pGroup->ubSector;
	SGPSector gSector0(gSector.x, gSector.y, 0);

	if( gfWorldLoaded )
	{ //look for people arriving in the currently loaded sector.  This handles reinforcements.
		const GROUP* const curr = FindPlayerMovementGroupInSector(gWorldSector.x, gWorldSector.y);
		if (gWorldSector == gSector0 && curr && PlayerMercsInSector(gWorldSector))
		{ //Reinforcements have arrived!
			if( gTacticalStatus.fEnemyInSector )
			{
				HandleArrivalOfReinforcements( pGroup );
				return( TRUE );
			}
		}
	}

	if( !DidGameJustStart() )
	{
		gubEnemyEncounterCode = NO_ENCOUNTER_CODE;
	}

	HandleOtherGroupsArrivingSimultaneously(gSector.x, gSector.y, gSector.z);

	FOR_EACH_PLAYER_GROUP(i)
	{
		GROUP& g = *i;
		if (g.ubGroupSize)
		{
			if (!g.fBetweenSectors)
			{
				SGPSector sSector(g.ubSector.x, g.ubSector.y);
				if (sSector == gSector0)
				{
					if (!GroupHasInTransitDeadOrPOWMercs(g) &&
						(!IsGroupTheHelicopterGroup(g) || !fHelicopterIsAirBorne) &&
						(!g.fVehicle || DoesVehicleGroupHaveAnyPassengers(g)))
					{
						//Now, a player group is in this sector.  Determine if the group contains any mercs that can fight.
						//Vehicles, EPCs and the robot doesn't count.  Mercs below OKLIFE do.
						CFOR_EACH_PLAYER_IN_GROUP(pPlayer, &g)
						{
							pSoldier = pPlayer->pSoldier;
							if( !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) )
							{
								if( !AM_A_ROBOT( pSoldier ) &&
									!AM_AN_EPC( pSoldier ) &&
									pSoldier->bLife >= OKLIFE )
								{
									fCombatAbleMerc = TRUE;
								}
								if( pSoldier->bLife > 0 )
								{
									fAliveMerc = TRUE;
								}
							}
						}
						if( !pPlayerDialogGroup && fCombatAbleMerc )
						{
							pPlayerDialogGroup = &g;
						}
						if( fCombatAbleMerc )
						{
							break;
						}
					}
				}
			}
		}
	}

	if( pGroup->fPlayer )
	{
		pPlayerDialogGroup = pGroup;

		if (NumEnemiesInSector(gSector))
		{
			fBattlePending = TRUE;
		}

		if( pGroup->uiFlags & GROUPFLAG_HIGH_POTENTIAL_FOR_AMBUSH && fBattlePending )
		{ //This group has just arrived in a new sector from an adjacent sector that he retreated from
			//If this battle is an encounter type battle, then there is a 90% chance that the battle will
			//become an ambush scenario.
			gfHighPotentialForAmbush = TRUE;
		}

		//If there are bloodcats in this sector, then it internally checks and handles it
		if( TestForBloodcatAmbush( pGroup ) )
		{
			fBloodCatAmbush = TRUE;
			fBattlePending = TRUE;
		}

		if( fBattlePending && (!fBloodCatAmbush || gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE) )
		{
			if( PossibleToCoordinateSimultaneousGroupArrivals( pGroup ) )
			{
				return FALSE;
			}
		}
	}
	else
	{
		if (CountAllMilitiaInSector(gSector))
		{
			fMilitiaPresent = TRUE;
			fBattlePending = TRUE;
		}
		if( fAliveMerc )
		{
			fBattlePending = TRUE;
		}
	}

	if( !fAliveMerc && !fMilitiaPresent )
	{ //empty vehicle, everyone dead, don't care.  Enemies don't care.
		return FALSE;
	}

	if( fBattlePending )
	{	//A battle is pending, but the player's could be all unconcious or dead.
		//Go through every group until we find at least one concious merc.  The looping will determine
		//if there are any live mercs and/or concious ones.  If there are no concious mercs, but alive ones,
		//then we will go straight to autoresolve, where the enemy will likely annihilate them or capture them.
		//If there are no alive mercs, then there is nothing anybody can do.  The enemy will completely ignore
		//this, and continue on.

		if( gubNumGroupsArrivedSimultaneously )
		{ //Because this is a battle case, clear all the group flags
			FOR_EACH_GROUP(curr)
			{
				if (gubNumGroupsArrivedSimultaneously == 0) break;
				if( curr->uiFlags & GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY )
				{
					curr->uiFlags &= ~GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY;
					gubNumGroupsArrivedSimultaneously--;
				}
			}
		}

		gpInitPrebattleGroup = pGroup;

		if( gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE || gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE )
		{
			NotifyPlayerOfBloodcatBattle(gSector);
			return TRUE;
		}

		if( !fCombatAbleMerc )
		{ //Prepare for instant autoresolve.
			gfDelayAutoResolveStart = TRUE;
			gfUsePersistantPBI = TRUE;
			if( fMilitiaPresent )
			{
				NotifyPlayerOfInvasionByEnemyForces(gSector, TriggerPrebattleInterface);
			}
			else
			{
				ST::string pSectorStr = GetSectorIDString(gSector, TRUE);
				ST::string str = st_format_printf(gpStrategicString[ STR_DIALOG_ENEMIES_ATTACK_UNCONCIOUSMERCS ], pSectorStr);
				DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, TriggerPrebattleInterface );
			}
		}

		if( pPlayerDialogGroup )
		{
			PrepareForPreBattleInterface( pPlayerDialogGroup, pGroup );
		}
		return TRUE;
	}
	return FALSE;
}


class DialogueEventTriggerPrebattleInterface : public DialogueEvent
{
	public:
		DialogueEventTriggerPrebattleInterface(GROUP* const init_prebattle_group) :
			init_prebattle_group_(init_prebattle_group)
		{}

		bool Execute()
		{
			UnLockPauseState();
			InitPreBattleInterface(init_prebattle_group_, true);
			return false;
		}

	private:
		GROUP* const init_prebattle_group_;
};


static void TriggerPrebattleInterface(MessageBoxReturnValue const ubResult)
{
	StopTimeCompression();
	DialogueEvent::Add(new DialogueEventTriggerPrebattleInterface(gpInitPrebattleGroup));
	gpInitPrebattleGroup = NULL;
}


//This will get called after a battle is auto-resolved or automatically after arriving
//at the next sector during a move and the area is clear.
void CalculateNextMoveIntention( GROUP *pGroup )
{
	INT32 i;
	WAYPOINT *wp;

	Assert( pGroup );

	//TEMP:  Ignore resting...

	//Should be surely an enemy group that has just made a new decision to go elsewhere!
	if( pGroup->fBetweenSectors )
	{
		return;
	}

	if( !pGroup->pWaypoints )
	{
		return;
	}

	//Determine if we are at a waypoint.
	i = pGroup->ubNextWaypointID;
	wp = pGroup->pWaypoints;
	while( i-- )
	{ //Traverse through the waypoint list to the next waypoint ID
		Assert( wp );
		wp = wp->next;
	}
	Assert( wp );

	//We have the next waypoint, now check if we are actually there.
	if (pGroup->ubSector.x == wp->x && pGroup->ubSector.y == wp->y)
	{ //We have reached the next waypoint, so now determine what the next waypoint is.
		switch( pGroup->ubMoveType )
		{
			case ONE_WAY:
				if( !wp->next )
				{ //No more waypoints, so we've reached the destination.
					return;
				}
				//Advance destination to next waypoint ID
				pGroup->ubNextWaypointID++;
				break;
			case CIRCULAR:
				wp = wp->next;
				if( !wp )
				{	//reached the end of the patrol route.  Set to the first waypoint in list, indefinately.
					//NOTE:  If the last waypoint isn't exclusively aligned to the x or y axis of the first
					//			 waypoint, there will be an assertion failure inside the waypoint movement code.
					pGroup->ubNextWaypointID = 0;
				}
				else
					pGroup->ubNextWaypointID++;
				break;
			case ENDTOEND_FORWARDS:
				wp = wp->next;
				if( !wp )
				{
					AssertMsg( pGroup->ubNextWaypointID, "EndToEnd patrol group needs more than one waypoint!" );
					pGroup->ubNextWaypointID--;
					pGroup->ubMoveType = ENDTOEND_BACKWARDS;
				}
				else
					pGroup->ubNextWaypointID++;
				break;
			case ENDTOEND_BACKWARDS:
				if( !pGroup->ubNextWaypointID )
				{
					pGroup->ubNextWaypointID++;
					pGroup->ubMoveType = ENDTOEND_FORWARDS;
				}
				else
					pGroup->ubNextWaypointID--;
				break;
		}
	}
	InitiateGroupMovementToNextSector( pGroup );
}


/* Based on how long movement took, mercs gain a bit of life experience for
 * travelling */
static void AwardExperienceForTravelling(GROUP& g)
{
	UINT32 const traverse_time = g.uiTraverseTime;
	CFOR_EACH_PLAYER_IN_GROUP(i, &g)
	{
		if (!i->pSoldier)    continue;
		SOLDIERTYPE& s = *i->pSoldier;
		if (IsMechanical(s)) continue;
		if (AM_AN_EPC(&s))   continue;

		if (s.bLifeMax < 100)
		{
			/* Amount was originally based on getting 100 - bLifeMax points for 12
			 * hours of travel (720), but changed to flat rate since StatChange makes
			 * roll vs 100-lifemax as well */
			UINT32 const points = traverse_time / (450 / (100 - s.bLifeMax));
			if (points > 0) StatChange(s, HEALTHAMT, (UINT8)points, FROM_SUCCESS);
		}

		if (s.bStrength < 100)
		{
			UINT32 const carried_percent = CalculateCarriedWeight(&s);
			if (carried_percent > 50)
			{
				UINT32 const points = traverse_time / (450 / (100 - s.bStrength));
				StatChange(s, STRAMT, points * (carried_percent - 50) / 100, FROM_SUCCESS);
			}
		}
	}
}


static void AddCorpsesToBloodcatLair(const SGPSector& sSector)
{
	ROTTING_CORPSE_DEFINITION		Corpse;
	Corpse = ROTTING_CORPSE_DEFINITION{};

	// Setup some values!
	Corpse.ubBodyType        = REGMALE;
	Corpse.sHeightAdjustment = 0;
	Corpse.bVisible          = TRUE;

	Corpse.HeadPal  = "BROWNHEAD";
	Corpse.VestPal  = "YELLOWVEST";
	Corpse.SkinPal  = "PINKSKIN";
	Corpse.PantsPal = "GREENPANTS";


	Corpse.bDirection = (INT8)Random(8);

	// Set time of death
	// Make sure they will be rotting!
	Corpse.uiTimeOfDeath = GetWorldTotalMin( ) - ( 2 * NUM_SEC_IN_DAY / 60 );
	// Set type
	Corpse.ubType  = (UINT8)SMERC_JFK;
	Corpse.usFlags = ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO;

	// 1st gridno
	Corpse.sGridNo 	= 14319;
	//Add the rotting corpse info to the sectors unloaded rotting corpse file
	AddRottingCorpseToUnloadedSectorsRottingCorpseFile(sSector, &Corpse);

	// 2nd gridno
	Corpse.sGridNo = 9835;
	//Add the rotting corpse info to the sectors unloaded rotting corpse file
	AddRottingCorpseToUnloadedSectorsRottingCorpseFile(sSector, &Corpse);


	// 3rd gridno
	Corpse.sGridNo = 11262;
	//Add the rotting corpse info to the sectors unloaded rotting corpse file
	AddRottingCorpseToUnloadedSectorsRottingCorpseFile(sSector, &Corpse);
}


static void HandleNonCombatGroupArrival(GROUP&, bool main_group, bool never_left);
static void ReportVehicleOutOfGas(VEHICLETYPE const&, const SGPSector& sMap);
static void SpendVehicleFuel(SOLDIERTYPE&, INT16 fuel_spent);
static INT16 VehicleFuelRemaining(SOLDIERTYPE const&);


void GroupArrivedAtSector(GROUP& g, BOOLEAN const check_for_battle, BOOLEAN const never_left)
{
	gfWaitingForInput = FALSE;

	if (g.fPlayer)
	{
		// Set the fact we have visited the sector
		if (PLAYERGROUP const* curr = g.pPlayerList)
		{
			INT8 const assignment = curr->pSoldier->bAssignment;
			if (assignment < ON_DUTY) ResetDeadSquadMemberList(assignment);
		}

		if (g.fVehicle)
		{
			VEHICLETYPE const& v = GetVehicleFromMvtGroup(g);
			if (!IsHelicopter(v) && !g.pPlayerList)
			{ /* Nobody here, better just get out now. With vehicles, arriving empty
				* is probably ok, since passengers might have been killed but vehicle
				* lived. */
				return;
			}
		}
		else
		{
			if (!g.pPlayerList)
			{ // Nobody here, better just get out now
				SLOGA("Player group %d arrived in sector empty.", g.ubGroupID);
				return;
			}
		}
	}

	SGPSector cSector(g.ubNext.x, g.ubNext.y, g.ubSector.z);
	static const SGPSector sanMona(5, 4);

	// Check for exception cases which
	bool const exception_queue =
		gTacticalStatus.bBoxingState != NOT_BOXING &&
		!g.fPlayer                                 &&
		cSector == sanMona;

	/* First check if the group arriving is going to queue another battle.
	 * NOTE: We can't have more than one battle ongoing at a time. */
	if (exception_queue ||
		(check_for_battle && gTacticalStatus.fEnemyInSector && FindPlayerMovementGroupInSector(gWorldSector.x, gWorldSector.y) && (cSector != gWorldSector || gWorldSector.z > 0)) ||
		AreInMeanwhile() ||
		/* KM: Aug 11, 1999 -- Patch fix: Added additional checks to prevent a 2nd
			* battle in the case where the player is involved in a potential battle
			* with bloodcats/civilians */
		(check_for_battle && HostileCiviliansPresent()) ||
		(check_for_battle && HostileBloodcatsPresent()))
	{ /* Queue battle! Delay arrival by a random value ranging from 3-5 minutes,
		* so it doesn't get the player too suspicious after it happens to him a few
		* times, which, by the way, is a rare occurrence. */
		if (AreInMeanwhile())
		{ /* Tack on only 1 minute if we are in a meanwhile scene. This effectively
			* prevents any battle from occurring while inside a meanwhile scene. */
			++g.uiArrivalTime;
		}
		else
		{
			g.uiArrivalTime += Random(3) + 3;
		}

		if (!AddStrategicEvent(EVENT_GROUP_ARRIVAL, g.uiArrivalTime, g.ubGroupID))
			SLOGA("Failed to add movement event.");

		if (g.fPlayer && g.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin())
		{
			AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE, g.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, g.ubGroupID);
		}
		return;
	}

	// Update the position of the group
	g.ubPrev  = g.ubSector;
	g.ubSector = cSector;
	g.ubNext.x   = 0;
	g.ubNext.y   = 0;

	if (g.fPlayer)
	{
		// Award life 'experience' for traveling, based on travel time.
		if (!g.fVehicle)
		{ // Gotta be walking to get tougher
			AwardExperienceForTravelling(g);
		}
		else if (!IsGroupTheHelicopterGroup(g))
		{
			VEHICLETYPE const& v  = GetVehicleFromMvtGroup(g);
			SOLDIERTYPE&       vs = GetSoldierStructureForVehicle(v);

			SpendVehicleFuel(vs, g.uiTraverseTime * 6);

			if (VehicleFuelRemaining(vs) == 0)
			{
				ReportVehicleOutOfGas(v, cSector);
				// Nuke the group's path, so they don't continue moving.
				ClearMercPathsAndWaypointsForAllInGroup(g);
			}
		}
	}

	g.uiTraverseTime      = 0;
	g.setArrivalTime(0);
	g.fBetweenSectors     = FALSE;
	fMapPanelDirty        = TRUE;
	fMapScreenBottomDirty = TRUE;

	bool group_destroyed = false;
	if (g.fPlayer)
	{
		// If this is the last sector along player group's movement path (no more waypoints)
		if (GroupAtFinalDestination(&g))
		{ // Clear their strategic movement (mercpaths and waypoints)
			ClearMercPathsAndWaypointsForAllInGroup(g);
		}

		// If on surface
		if (cSector.z == 0)
		{
			// check for discovering secret locations
			if (GetMapSecretBySectorID(cSector.AsByte()))
			{
				SetSectorSecretAsFound(cSector.AsByte());
			}
		}

		UINT8 insertion_direction;
		UINT8 strategic_insertion_code;
		if (cSector.x < g.ubPrev.x)
		{
			insertion_direction      = SOUTHWEST;
			strategic_insertion_code = INSERTION_CODE_EAST;
		}
		else if (cSector.x > g.ubPrev.x)
		{
			insertion_direction      = NORTHEAST;
			strategic_insertion_code = INSERTION_CODE_WEST;
		}
		else if (cSector.y < g.ubPrev.y)
		{
			insertion_direction      = NORTHWEST;
			strategic_insertion_code = INSERTION_CODE_SOUTH;
		}
		else if (cSector.y > g.ubPrev.y)
		{
			insertion_direction      = SOUTHEAST;
			strategic_insertion_code = INSERTION_CODE_NORTH;
		}
		else
		{
			SLOGA("GroupArrivedAtSector: group arrives in sector where it already has been");
			return;
		}

		bool    const  here = cSector == gWorldSector;
		ST::string who;
		if (!g.fVehicle)
		{
			// non-vehicle player group
			CFOR_EACH_PLAYER_IN_GROUP(i, &g)
			{
				SOLDIERTYPE& s = *i->pSoldier;
				s.fBetweenSectors      = FALSE;
				s.sSectorX             = cSector.x;
				s.sSectorY             = cSector.y;
				s.bSectorZ             = cSector.z;
				s.ubPrevSectorID       = g.ubPrev.AsByte();
				s.ubInsertionDirection = insertion_direction;

				// don't override if a tactical traversal
				if (s.ubStrategicInsertionCode != INSERTION_CODE_PRIMARY_EDGEINDEX &&
						s.ubStrategicInsertionCode != INSERTION_CODE_SECONDARY_EDGEINDEX)
				{
					s.ubStrategicInsertionCode = strategic_insertion_code;
				}

				// Remove head from their mapscreen path list
				if (s.pMercPath) s.pMercPath = RemoveHeadFromStrategicPath(s.pMercPath);

				/* ATE: Check if this sector is currently loaded, if so, add them to the
				 * tactical engine */
				if (here) UpdateMercInSector(s, gWorldSector);
			}

			// If there's anybody in the group
			if (g.pPlayerList)
			{
				SOLDIERTYPE const& s = *g.pPlayerList->pSoldier;
				who =
					s.bAssignment >= ON_DUTY ? s.name : // A loner
					pAssignmentStrings[s.bAssignment];  // Squad
			}
		}
		else
		{ // Vehicle player group
			VEHICLETYPE& v = GetVehicleFromMvtGroup(g);
			// Remove head from vehicle's mapscreen path list
			if (v.pMercPath) v.pMercPath = RemoveHeadFromStrategicPath(v.pMercPath);

			// Update vehicle position
			SetVehicleSectorValues(v, cSector);
			v.fBetweenSectors = FALSE;

			if (!IsHelicopter(v))
			{
				SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);
				vs.fBetweenSectors          = FALSE;
				vs.sSectorX                 = cSector.x;
				vs.sSectorY                 = cSector.y;
				vs.bSectorZ                 = cSector.z;
				vs.ubInsertionDirection     = insertion_direction;
				vs.ubStrategicInsertionCode = strategic_insertion_code;

				// If this sector is currently loaded, add vehicle to the tactical engine
				if (here) UpdateMercInSector(vs, gWorldSector);

				// Set directions of insertion
				CFOR_EACH_PLAYER_IN_GROUP(i, &g)
				{
					SOLDIERTYPE& s = *i->pSoldier;
					s.fBetweenSectors = FALSE;
					s.sSectorX = cSector.x;
					s.sSectorY = cSector.y;
					s.bSectorZ = cSector.z;
					s.ubInsertionDirection = insertion_direction;
					s.ubStrategicInsertionCode = strategic_insertion_code;

					// If this sector is currently loaded, add passenger to the tactical engine
					if (here) UpdateMercInSector(s, gWorldSector);
				}
			}
			else
			{
				if (HandleHeliEnteringSector(v.sSector))
				{ // Helicopter destroyed
					group_destroyed = true;
				}
			}

			if (!group_destroyed) who = pVehicleStrings[v.ubVehicleType];
		}

		if (!who.empty())
		{ /* Don't print any messages when arriving underground (there's no delay
			 * involved) or if we never left (cancel) */
			if (GroupAtFinalDestination(&g) && cSector.z == 0 && !never_left)
			{
				ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, st_format_printf(pMessageStrings[MSG_ARRIVE], who, pMapVertIndex[cSector.y], pMapHortIndex[cSector.x]));
			}
		}

		if (!group_destroyed)
		{
			// On foot, or in a vehicle other than the chopper
			if (!g.fVehicle || !IsGroupTheHelicopterGroup(g))
			{
				// ATE: Add a few corpse to the bloodcat lair
				auto spawns = GCM->getBloodCatSpawnsOfSector(cSector.AsByte());
				if ( spawns != NULL && spawns->isLair &&
					!GetSectorFlagStatus(cSector, SF_ALREADY_VISITED))
				{
					AddCorpsesToBloodcatLair(cSector);
				}

				// Mark the sector as visited already
				SetSectorFlag(cSector, SF_ALREADY_VISITED);
			}
		}

		// Update character info
		fTeamPanelDirty          = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}

	if (!group_destroyed)
	{
		/* Determine if a battle should start. If a battle does start, or get's
		 * delayed, then we will keep the group in memory including all waypoints,
		 * until after the battle is resolved.  At that point, we will continue the
		 * processing. */
		if (check_for_battle && !CheckConditionsForBattle(&g) && !gfWaitingForInput)
		{
			HandleNonCombatGroupArrival(g, true, never_left);

			if (gubNumGroupsArrivedSimultaneously != 0)
			{
				FOR_EACH_GROUP_SAFE(i)
				{
					GROUP& g = *i;
					if (!(g.uiFlags & GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY)) continue;
					--gubNumGroupsArrivedSimultaneously;
					HandleNonCombatGroupArrival(g, false, false);
					if (gubNumGroupsArrivedSimultaneously == 0) break;
				}
			}
		}
		else
		{ // Handle cases for pre-battle conditions
			g.uiFlags = 0;
			if (gubNumAwareBattles != 0)
			{ /* When the AI is looking for the players, and a battle is initiated,
				* then decrement the value, otherwise the queen will continue searching
				* to infinity. */
				--gubNumAwareBattles;
			}
		}
	}
	gfWaitingForInput = FALSE;
}


static void HandleNonCombatGroupArrival(GROUP& g, bool const main_group, bool const never_left)
{
	if (StrategicAILookForAdjacentGroups(&g))
	{ /* The routine actually just deleted the enemy group (player's don't get
		* deleted), so we are done! */
		return;
	}

	if (g.fPlayer)
	{ // The group will always exist after the AI was processed.

		bool const is_heli_group = g.fVehicle && IsGroupTheHelicopterGroup(g);
		if (!is_heli_group)
		{ // Take control of sector
			SetThisSectorAsPlayerControlled(g.ubSector, FALSE);
		}

		// If this is the last sector along their movement path (no more waypoints)
		if (GroupAtFinalDestination(&g))
		{
			// If currently selected sector has nobody in it
			if (PlayerMercsInSector(sSelMap.x, sSelMap.y, iCurrentMapSectorZ) == 0)
			{ // Make this sector strategically selected
				ChangeSelectedMapSector(g.ubSector);
			}

			if (!is_heli_group) // Else Skyrider speaks for heli movement
			{
				StopTimeCompression();

				// If traversing tactically, or we never left (just canceling), don't do this
				if (!gfTacticalTraversal && !never_left)
				{
					RandomMercInGroupSaysQuote(g, QUOTE_MERC_REACHED_DESTINATION);
				}
			}
		}
		/* Look for NPCs to stop for, anyone is too tired to keep going, if all OK
		 * rebuild waypoints & continue movement
		 * NOTE: Only the main group (first group arriving) will stop for NPCs, it's
		 * just too much hassle to stop them all */
		PlayerGroupArrivedSafelyInSector(g, main_group);
	}
	else
	{
		if (!g.fDebugGroup)
		{
			CalculateNextMoveIntention(&g);
		}
		else
		{
			RemoveGroup(g);
		}
	}

	g.uiFlags = 0; // Clear the non-persistant flags
}


/* Because a battle is about to start, we need to go through the event list and
 * look for other groups that may arrive at the same time -- enemies or players,
 * and blindly add them to the sector without checking for battle conditions, as
 * it has already determined that a new battle is about to start. */
static void HandleOtherGroupsArrivingSimultaneously(UINT8 const x, UINT8 const y, UINT8 const z)
{
	UINT32 const now = GetWorldTotalSeconds();
	gubNumGroupsArrivedSimultaneously = 0;
restart:
	for (STRATEGICEVENT* i = gpEventList; i && i->uiTimeStamp <= now; i = i->next)
	{
		if (i->ubCallbackID != EVENT_GROUP_ARRIVAL) continue;
		if (i->ubFlags & SEF_DELETION_PENDING)      continue;

		GROUP& g = *GetGroup((UINT8)i->uiParam);
		if (g.ubNext.x != x || g.ubNext.x != y || g.ubSector.z != z) continue;
		if (!g.fBetweenSectors) continue;

		GroupArrivedAtSector(g, FALSE, FALSE);
		g.uiFlags |= GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY;
		++gubNumGroupsArrivedSimultaneously;
		DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);
		goto restart;
	}
}


static void DelayEnemyGroupsIfPathsCross(GROUP& player_group);


/* The user has just approved to plan a simultaneous arrival. So we will
 * syncronize all of the involved groups so that they arrive at the same time
 * (which is the time the final group would arrive). */
static void PrepareGroupsForSimultaneousArrival()
{
	GROUP& first_group = *gpPendingSimultaneousGroup;

	/* For all of the groups that haven't arrived yet, determine which one is
	 * going to take the longest. */
	UINT32 latest_arrival_time = 0;
	FOR_EACH_PLAYER_GROUP(i)
	{
		GROUP& g = *i;
		if (&g == &first_group)                 continue;
		if (!g.fBetweenSectors)                 continue;
		if (g.ubNext != first_group.ubSector) continue;
		if (IsGroupTheHelicopterGroup(g))       continue;
		latest_arrival_time = MAX(g.uiArrivalTime, latest_arrival_time);
		g.uiFlags |= GROUPFLAG_SIMULTANEOUSARRIVAL_APPROVED | GROUPFLAG_MARKER;
	}

	/* Now, go through the list again, and reset their arrival event to the latest
	 * arrival time. */
	FOR_EACH_GROUP(i)
	{
		GROUP& g = *i;
		if (!(g.uiFlags & GROUPFLAG_MARKER)) continue;

		DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);

		/* NOTE: This can cause the arrival time to be > GetWorldTotalMin() +
		 * TraverseTime, so keep that in mind if you have any code that uses these 3
		 * values to figure out how far along its route a group is! */
		g.setArrivalTime(latest_arrival_time);
		AddStrategicEvent(EVENT_GROUP_ARRIVAL, g.uiArrivalTime, g.ubGroupID);

		if (g.fPlayer && g.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin())
		{
			AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE, g.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, g.ubGroupID);
		}

		DelayEnemyGroupsIfPathsCross(g);
		g.uiFlags &= ~GROUPFLAG_MARKER;
	}

	/* We still have the first group that has arrived. Because they are set up to
	 * be in the destination sector, we will "warp" them back to the last sector,
	 * and also setup a new arrival time for them. */
	first_group.ubNext         = first_group.ubSector;
	first_group.ubSector       = first_group.ubPrev;
	first_group.setArrivalTime(latest_arrival_time);
	first_group.fBetweenSectors = TRUE;

	if (first_group.fVehicle)
	{
		VEHICLETYPE& v = GetVehicleFromMvtGroup(first_group);
		v.fBetweenSectors = TRUE;

		if (!IsHelicopter(v))
		{
			SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);
			vs.fBetweenSectors = TRUE;
		}
	}

	AddStrategicEvent(EVENT_GROUP_ARRIVAL, first_group.uiArrivalTime, first_group.ubGroupID);

	if (first_group.fPlayer && first_group.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin())
	{
		AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE, first_group.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, first_group.ubGroupID);
	}
	DelayEnemyGroupsIfPathsCross(first_group);
}


static void PlanSimultaneousGroupArrivalCallback(MessageBoxReturnValue);


/* See if there are other groups OTW.  If so, and if we haven't asked the user
 * yet to plan a simultaneous attack, do so now, and readjust the groups
 * accordingly.  If it is possible to do so, then we will set up the gui, and
 * postpone the prebattle interface. */
static BOOLEAN PossibleToCoordinateSimultaneousGroupArrivals(GROUP* const first_group)
{
	// If the user has already been asked, then don't ask the question again!
	if (first_group->uiFlags & (GROUPFLAG_SIMULTANEOUSARRIVAL_APPROVED | GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED)) return FALSE;
	if (IsGroupTheHelicopterGroup(*first_group)) return FALSE;

	/* Count the number of groups that are scheduled to arrive in the same sector
	 * and are currently adjacent to the sector in question. */
	UINT8 n_nearby_groups = 0;
	FOR_EACH_PLAYER_GROUP(i)
	{
		GROUP& g = *i;
		if (&g == first_group)                                 continue;
		if (!g.fBetweenSectors)                                continue;
		if (g.ubNext != first_group->ubSector)                 continue;
		if (g.uiFlags & GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED) continue;
		if (IsGroupTheHelicopterGroup(g))                      continue;
		g.uiFlags |= GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED;
		++n_nearby_groups;
	}

	if (n_nearby_groups == 0) return FALSE;

	// Postpone the battle until the user answers the dialog.
	InterruptTime();
	PauseGame();
	LockPauseState(LOCK_PAUSE_SIMULTANEOUS_ARRIVAL);
	gpPendingSimultaneousGroup = first_group;

	ST::string pStr =
		n_nearby_groups == 1 ? gpStrategicString[STR_DETECTED_SINGULAR] :
		gpStrategicString[STR_DETECTED_PLURAL];
	ST::string enemy_type =
		gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE ? gpStrategicString[STR_PB_BLOODCATS] :
		gpStrategicString[STR_PB_ENEMIES];
	/* header, sector, singular/plural str, confirmation string.
	 * Ex:  Enemies have been detected in sector J9 and another squad is about to
	 *      arrive.  Do you wish to coordinate a simultaneous arrival? */
	ST::string str = st_format_printf(pStr, enemy_type, first_group->ubSector.AsShortString());
	str += ST::format(" {}", gpStrategicString[STR_COORDINATE]);
	DoMapMessageBox(MSG_BOX_BASIC_STYLE, str, guiCurrentScreen, MSG_BOX_FLAG_YESNO, PlanSimultaneousGroupArrivalCallback);
	gfWaitingForInput = TRUE;
	return TRUE;
}


static void PlanSimultaneousGroupArrivalCallback(MessageBoxReturnValue const bMessageValue)
{
	if( bMessageValue == MSG_BOX_RETURN_YES )
	{
		PrepareGroupsForSimultaneousArrival();
	}
	else
	{
		PrepareForPreBattleInterface( gpPendingSimultaneousGroup, gpPendingSimultaneousGroup );
	}
	UnLockPauseState();
	UnPauseGame();
}


static void DelayEnemyGroupsIfPathsCross(GROUP& player_group)
{
	FOR_EACH_ENEMY_GROUP(i)
	{
		GROUP& g = *i;
		// Check to see if this group will arrive in next sector before the player group.
		if (g.uiArrivalTime >= player_group.uiArrivalTime) continue;
		// Check to see if enemy group will cross paths with player group.
		if (g.ubNext   != player_group.ubSector) continue;
		if (g.ubSector != player_group.ubNext)   continue;

		/* The enemy group will cross paths with the player, so find and delete the
		 * arrival event and repost it in the future (like a minute or so after the
		 * player arrives) */
		DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);

		/* NOTE: This can cause the arrival time to be > GetWorldTotalMin() +
		 * TraverseTime, so keep that in mind if you have any code that uses these 3
		 * values to figure out how far along its route a group is! */
		g.setArrivalTime(player_group.uiArrivalTime + 1 + Random(10));
		if (!AddStrategicEvent(EVENT_GROUP_ARRIVAL, g.uiArrivalTime, g.ubGroupID))
			SLOGA("Failed to add movement event.");
	}
}


//Calculates and posts an event to move the group to the next sector.
static void InitiateGroupMovementToNextSector(GROUP* pGroup)
{
	INT32 dx, dy;
	INT32 i;
	UINT8 ubDirection;
	UINT8 ubSector;
	WAYPOINT *wp;
	UINT32 uiSleepMinutes = 0;

	Assert( pGroup );
	i = pGroup->ubNextWaypointID;
	wp = pGroup->pWaypoints;
	while( i-- )
	{ //Traverse through the waypoint list to the next waypoint ID
		Assert( wp );
		wp = wp->next;
	}
	Assert( wp );
	//We now have the correct waypoint.
	//Analyse the group and determine which direction it will move from the current sector.
	dx = wp->x - pGroup->ubSector.x;
	dy = wp->y - pGroup->ubSector.y;
	if( dx && dy )
	{ //Can't move diagonally!
		SLOGA("Attempting to move to waypoint in a diagonal direction from sector %d,%d to sector %d,%d",
			pGroup->ubSector.x, pGroup->ubSector.y, wp->x, wp->y );
	}
	//Clip dx/dy value so that the move is for only one sector.
	if( dx >= 1 )
	{
		ubDirection = EAST_STRATEGIC_MOVE;
		dx = 1;
	}
	else if( dy >= 1 )
	{
		ubDirection = SOUTH_STRATEGIC_MOVE;
		dy = 1;
	}
	else if( dx <= -1 )
	{
		ubDirection = WEST_STRATEGIC_MOVE;
		dx = -1;
	}
	else if( dy <= -1 )
	{
		ubDirection = NORTH_STRATEGIC_MOVE;
		dy = -1;
	}
	else
	{
		SLOGA("InitiateGroupMovementToNextSector: Attempting to move to waypoint %d, %d that you are already at!", wp->x, wp->y);
		return;
	}
	//All conditions for moving to the next waypoint are now good.
	pGroup->ubNext.x = (UINT8)( dx + pGroup->ubSector.x );
	pGroup->ubNext.y = (UINT8)( dy + pGroup->ubSector.y );
	//Calc time to get to next waypoint...
	ubSector = pGroup->ubSector.AsByte();
	if (!pGroup->ubSector.z)
	{
		BOOLEAN fCalcRegularTime = TRUE;
		if( !pGroup->fPlayer )
		{ //Determine if the enemy group is "sleeping".  If so, then simply delay their arrival time by the amount of time
			//they are going to be sleeping for.
			if( GetWorldHour() >= 21 || GetWorldHour() <= 4 )
			{ //It is definitely night time.
				if( Chance( 67 ) )
				{ //2 in 3 chance of going to sleep.
					pGroup->uiTraverseTime = GetSectorMvtTimeForGroup( ubSector, ubDirection, pGroup );
					uiSleepMinutes = 360 + Random( 121 ); //6-8 hours sleep
					fCalcRegularTime = FALSE;
				}
			}
		}
		if( fCalcRegularTime )
		{
			pGroup->uiTraverseTime = GetSectorMvtTimeForGroup( ubSector, ubDirection, pGroup );
		}
	}
	else
	{
		pGroup->uiTraverseTime = 1;
	}

	AssertMsg(pGroup->uiTraverseTime != TRAVERSE_TIME_IMPOSSIBLE, String("Group %d (%s) attempting illegal move from %c%d to %c%d (%s).",
			pGroup->ubGroupID, ( pGroup->fPlayer ) ? "Player" : "AI",
			pGroup->ubSector.y+'A', pGroup->ubSector.x, pGroup->ubNext.y + 'A', pGroup->ubNext.x,
			gszTerrain[SectorInfo[ubSector].ubTraversability[ubDirection]] ) );

	// add sleep, if any
	pGroup->uiTraverseTime += uiSleepMinutes;

	if( gfTacticalTraversal && gpTacticalTraversalGroup == pGroup )
	{
		if( gfUndergroundTacticalTraversal )
		{	//underground movement between sectors takes 1 minute.
			pGroup->uiTraverseTime = 1;
		}
		else
		{ //strategic movement between town sectors takes 5 minutes.
			pGroup->uiTraverseTime = 5;
		}
	}

	// if group isn't already between sectors
	if ( !pGroup->fBetweenSectors )
	{
		// put group between sectors
		pGroup->fBetweenSectors	= TRUE;
		// and set it's arrival time
		pGroup->setArrivalTime(GetWorldTotalMin() + pGroup->uiTraverseTime);
	}
	// NOTE: if the group is already between sectors, DON'T MESS WITH ITS ARRIVAL TIME!  THAT'S NOT OUR JOB HERE!!!


	// special override for AI patrol initialization only
	if( gfRandomizingPatrolGroup )
	{ //We're initializing the patrol group, so randomize the enemy groups to have extremely quick and varying
		//arrival times so that their initial positions aren't easily determined.
		pGroup->uiTraverseTime = 1 + Random( pGroup->uiTraverseTime - 1 );
		pGroup->setArrivalTime(GetWorldTotalMin() + pGroup->uiTraverseTime);
	}


	if (pGroup->fVehicle)
	{
		// vehicle, set fact it is between sectors too
		VEHICLETYPE& v = GetVehicleFromMvtGroup(*pGroup);
		v.fBetweenSectors = TRUE;

		if (!IsHelicopter(v))
		{
			SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);
			vs.fBetweenSectors = TRUE;
			RemoveSoldierFromTacticalSector(vs);
		}
	}

	//Post the event!
	if( !AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID ) )
		SLOGA("Failed to add movement event.");

	//For the case of player groups, we need to update the information of the soldiers.
	if( pGroup->fPlayer )
	{
		if( pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin( ) )
		{
			AddStrategicEvent( EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID );
		}

		CFOR_EACH_PLAYER_IN_GROUP(curr, pGroup)
		{
			SOLDIERTYPE& s = *curr->pSoldier;
			s.fBetweenSectors = TRUE;
			RemoveSoldierFromTacticalSector(s);
		}
		CheckAndHandleUnloadingOfCurrentWorld();

		//If an enemy group will be crossing paths with the player group, delay the enemy group's arrival time so that
		//the player will always encounter that group.
		if (!pGroup->ubSector.z)
		{
			DelayEnemyGroupsIfPathsCross(*pGroup);
		}
	}
}


void RemoveGroupWaypoints(GROUP& g)
{
	// If there aren't any waypoints to delete, then return
	if (!g.pWaypoints) return;

	// Remove all of the waypoints
	for (WAYPOINT* i = g.pWaypoints; i;)
	{
		WAYPOINT* const del = i;
		i = i->next;
		delete del;
	}

	g.ubNextWaypointID = 0;
	g.pWaypoints       = 0;
}

static BOOLEAN gfRemovingAllGroups = FALSE;


void RemoveGroup(GROUP& g)
{
	if (g.fPersistant && !gfRemovingAllGroups)
	{
		CancelEmptyPersistentGroupMovement(g);
		return;
		DoScreenIndependantMessageBox("Strategic Info Warning:  Attempting to delete a persistant group.", MSG_BOX_FLAG_OK, NULL);
	}

	RemoveGroupWaypoints(g);

	// Remove the arrival event if applicable.
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);

	// Determine what type of group we have (because it requires different methods)
	if (g.fPlayer)
	{
		while (g.pPlayerList)
		{
			PLAYERGROUP* const pPlayer = g.pPlayerList;
			g.pPlayerList = g.pPlayerList->next;
			delete pPlayer;
		}
	}
	else
	{
		RemoveGroupFromStrategicAILists(g);
		delete g.pEnemyGroup;
	}

	RemoveGroupFromList(&g);

	/* safety check: if this group is the BattleGroup, invalid the pointer */
	if(gpBattleGroup == &g)
	{
		gpBattleGroup = 0;
	}
}


void RemoveAllGroups()
{
	gfRemovingAllGroups = TRUE;
	while( gpGroupList )
	{
		RemoveGroup(*gpGroupList);
	}
	gfRemovingAllGroups = FALSE;
}

void SetGroupSectorValue(const SGPSector& sector, GROUP& g)
{
	SetGroupSectorValue(sector.x, sector.y, sector.z, g);
}

void SetGroupSectorValue(INT16 const x, INT16 const y, INT16 const z, GROUP& g)
{
	RemoveGroupWaypoints(g);

	// Set sector x and y to passed values
	g.ubSector.x       = x;
	g.ubSector.y       = y;
	g.ubNext.x         = x;
	g.ubNext.y         = y;
	g.ubSector.z       = z;
	g.fBetweenSectors = FALSE;

	// Set next sectors same as current
	g.ubOriginalSector = SECTOR(x, y);
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);

	// Set all of the mercs in the group so that they are in the new sector, too.
	CFOR_EACH_PLAYER_IN_GROUP(i, &g)
	{
		SOLDIERTYPE& s = *i->pSoldier;
		s.sSectorX        = x;
		s.sSectorY        = y;
		s.bSectorZ        = z;
		s.fBetweenSectors = FALSE;
	}

	CheckAndHandleUnloadingOfCurrentWorld();
}


void SetEnemyGroupSector(GROUP& g, UINT8 const sector_id)
{
	// Make sure it is valid
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);

	if (!gfRandomizingPatrolGroup) RemoveGroupWaypoints(g);

	g.ubSector = g.ubNext = SGPSector(sector_id);
	g.ubSector.z = 0;
	g.fBetweenSectors = FALSE;
}


// Set groups next sector x,y value, used ONLY for teleporting groups
static void SetGroupNextSectorValue(const SGPSector& sector, GROUP& g)
{
	RemoveGroupWaypoints(g);
	// Set sector x and y to passed values
	g.ubNext.x         = sector.x;
	g.ubNext.y         = sector.y;
	g.fBetweenSectors = FALSE;
	// Set next sectors same as current
	g.ubOriginalSector = g.ubSector.AsByte();
}


INT32 CalculateTravelTimeOfGroup(GROUP const* const pGroup)
{
	INT32 iDelta;
	UINT32 uiEtaTime = 0;
	WAYPOINT *pNode = NULL;
	WAYPOINT pCurrent, pDest;

	// check if valid group
	if( pGroup == NULL )
	{
		// return current time
		return( uiEtaTime );
	}

	// set up next node
	pNode = pGroup-> pWaypoints;

	// now get the delta in current sector and next sector
	iDelta = ( INT32 )(pGroup->ubSector.AsByte() - pGroup->ubNext.AsByte());

	if( iDelta == 0 )
	{
		// not going anywhere...return current time
		return( uiEtaTime );
	}


	// if already on the road
	if ( pGroup->fBetweenSectors )
	{
		// to get travel time to the first sector, use the arrival time, this way it accounts for delays due to simul. arrival
		if ( pGroup->uiArrivalTime >= GetWorldTotalMin( ) )
		{
			uiEtaTime += ( pGroup->uiArrivalTime - GetWorldTotalMin( ) );
		}

		// first waypoint is NEXT sector
		pCurrent.x = pGroup->ubNext.x;
		pCurrent.y = pGroup->ubNext.y;
	}
	else
	{
		// first waypoint is CURRENT sector
		pCurrent.x = pGroup->ubSector.x;
		pCurrent.y = pGroup->ubSector.y;
	}

	while( pNode )
	{
		pDest.x = pNode->x;
		pDest.y = pNode->y;

		// update eta time by the path between these 2 waypts
		uiEtaTime += FindTravelTimeBetweenWaypoints( &pCurrent, &pDest, pGroup );

		pCurrent.x = pNode->x;
		pCurrent.y = pNode->y;

		// next waypt
		pNode = pNode->next;
	}

	return( uiEtaTime );
}


INT32 FindTravelTimeBetweenWaypoints(WAYPOINT const* const pSource, WAYPOINT const* const pDest,  GROUP const * const pGroup)
{
	UINT8 ubStart=0, ubEnd = 0;
	INT32 iDelta = 0;
	INT32 iCurrentCostInTime = 0;
	UINT8 ubCurrentSector = 0;
	UINT8 ubDirection;
	INT32 iThisCostInTime;


	// find travel time between waypoints
	if( !pSource || !pDest )
	{
		// no change
		return( iCurrentCostInTime );
	}

	// get start and end setor values
	ubStart = SECTOR( pSource->x, pSource->y );
	ubEnd   = SECTOR( pDest->x,   pDest->y );

	// are we in fact moving?
	if( ubStart == ubEnd )
	{
		// no
		return( iCurrentCostInTime );
	}

	iDelta = ( INT32 )( ubEnd - ubStart );

	// which direction are we moving?
	if( iDelta > 0 )
	{
		if( iDelta % ( SOUTH_MOVE - 2 ) == 0 )
		{
			iDelta = ( SOUTH_MOVE - 2 ) ;
			ubDirection = SOUTH_STRATEGIC_MOVE;
		}
		else
		{
			iDelta = EAST_MOVE;
			ubDirection = EAST_STRATEGIC_MOVE;
		}
	}
	else
	{
		if( iDelta % ( NORTH_MOVE + 2 ) == 0 )
		{
			iDelta = ( NORTH_MOVE + 2 );
			ubDirection = NORTH_STRATEGIC_MOVE;
		}
		else
		{
			iDelta = WEST_MOVE;
			ubDirection = WEST_STRATEGIC_MOVE;
		}
	}

	for( ubCurrentSector = ubStart; ubCurrentSector != ubEnd; ubCurrentSector += ( INT8 ) iDelta )
	{
		// find diff between current and next
		iThisCostInTime = GetSectorMvtTimeForGroup( ubCurrentSector, ubDirection, pGroup );

		AssertMsg(iThisCostInTime != static_cast<INT32>(TRAVERSE_TIME_IMPOSSIBLE), String("Group %d (%s) attempting illegal move from sector %d, dir %d (%s).",
					pGroup->ubGroupID, ( pGroup->fPlayer ) ? "Player" : "AI",
					ubCurrentSector, ubDirection,
					gszTerrain[SectorInfo[ubCurrentSector].ubTraversability[ubDirection]] ) );

		// accumulate it
		iCurrentCostInTime += iThisCostInTime;
	}

	return( iCurrentCostInTime );
}


#define FOOT_TRAVEL_TIME    89
#define CAR_TRAVEL_TIME     30
#define TRUCK_TRAVEL_TIME   32
#define TRACKED_TRAVEL_TIME 46
#define AIR_TRAVEL_TIME     10


// Changes: direction contains the strategic move value, not the delta value.
INT32 GetSectorMvtTimeForGroup(UINT8 const ubSector, UINT8 const direction, GROUP const* const g)
{
	/* Determine the group's method(s) of transportation.  If more than one, we
	 * will always use the highest time. */
	UINT8 const transport_mask     = g->ubTransportationMask;
	UINT8 const traverse_type      = SectorInfo[ubSector].ubTraversability[direction];
	INT32       best_traverse_time = 1000000;

	if (traverse_type == EDGEOFWORLD) return TRAVERSE_TIME_IMPOSSIBLE;

	/* ARM: Made air-only travel take its normal time per sector even through
	 * towns.  Because Skyrider charges by the sector, not by flying time, it's
	 * annoying when his default route detours through a town to save time, but
	 * costs extra money. This isn't exactly unrealistic, since the chopper
	 * shouldn't be faster flying over a town anyway. Not that other kinds of
	 * travel should be either - but the towns represents a kind of warping of our
	 * space-time scale as it is. */
	if (traverse_type == TOWN && transport_mask != AIR)
		return 5; // Very fast, and vehicle types don't matter.

	if (transport_mask & FOOT)
	{
		UINT8 traverse_mod;
		switch (traverse_type)
		{
			case ROAD:     traverse_mod = 100; break;
			case PLAINS:   traverse_mod =  85; break;
			case SAND:     traverse_mod =  50; break;
			case SPARSE:   traverse_mod =  70; break;
			case DENSE:    traverse_mod =  60; break;
			case SWAMP:    traverse_mod =  35; break;
			case WATER:    traverse_mod =  25; break;
			case HILLS:    traverse_mod =  50; break;
			case NS_RIVER: traverse_mod =  25; break;
			case EW_RIVER: traverse_mod =  25; break;
			default:       return TRAVERSE_TIME_IMPOSSIBLE;
		}
		INT32 const traverse_time = FOOT_TRAVEL_TIME * 100 / traverse_mod;
		if (best_traverse_time > traverse_time)
			best_traverse_time = traverse_time;

		if (g->fPlayer)
		{
			INT32 highest_encumbrance = 100;
			CFOR_EACH_PLAYER_IN_GROUP(curr, g)
			{
				SOLDIERTYPE const* const s = curr->pSoldier;
				if (s->bAssignment == VEHICLE) continue;
				/* Soldier is on foot and travelling.  Factor encumbrance into movement
				 * rate. */
				INT32 const encumbrance = CalculateCarriedWeight(s);
				if (highest_encumbrance < encumbrance)
				{
					highest_encumbrance = encumbrance;
				}
			}
			best_traverse_time = best_traverse_time * highest_encumbrance / 100;
		}
	}

	if (transport_mask & CAR)
	{
		UINT8 traverse_mod;
		switch (traverse_type)
		{
			case ROAD: traverse_mod = 100; break;
			default:   return TRAVERSE_TIME_IMPOSSIBLE;
		}
		INT32 const traverse_time = CAR_TRAVEL_TIME * 100 / traverse_mod;
		if (best_traverse_time > traverse_time)
			best_traverse_time = traverse_time;
	}

	if (transport_mask & TRUCK)
	{
		UINT8 traverse_mod;
		switch (traverse_type)
		{
			case ROAD:   traverse_mod = 100; break;
			case PLAINS: traverse_mod =  75; break;
			case SPARSE: traverse_mod =  60; break;
			case HILLS:  traverse_mod =  50; break;
			default:     return TRAVERSE_TIME_IMPOSSIBLE;
		}
		INT32 const traverse_time = TRUCK_TRAVEL_TIME * 100 / traverse_mod;
		if (best_traverse_time > traverse_time)
			best_traverse_time = traverse_time;
	}

	if (transport_mask & TRACKED)
	{
		UINT8 traverse_mod;
		switch (traverse_type)
		{
			case ROAD:     traverse_mod = 100; break;
			case PLAINS:   traverse_mod = 100; break;
			case SAND:     traverse_mod =  70; break;
			case SPARSE:   traverse_mod =  60; break;
			case HILLS:    traverse_mod =  60; break;
			case NS_RIVER: traverse_mod =  20; break;
			case EW_RIVER: traverse_mod =  20; break;
			case WATER:    traverse_mod =  10; break;
			default:       return TRAVERSE_TIME_IMPOSSIBLE;
		}
		INT32 const traverse_time = TRACKED_TRAVEL_TIME * 100 / traverse_mod;
		if (best_traverse_time > traverse_time)
			best_traverse_time = traverse_time;
	}

	if (transport_mask & AIR)
	{
		INT32 const traverse_time = AIR_TRAVEL_TIME;
		if (best_traverse_time > traverse_time)
			best_traverse_time = traverse_time;
	}

	return best_traverse_time;
}

UINT8 PlayerMercsInSector(const SGPSector& sector)
{
	return PlayerMercsInSector(sector.x, sector.y, sector.z);
}

// Counts the number of live mercs in any given sector.
UINT8 PlayerMercsInSector(UINT8 const x, UINT8 const y, UINT8 const z)
{
	UINT8 n_mercs = 0;
	CFOR_EACH_PLAYER_GROUP(g)
	{
		if (g->fBetweenSectors) continue;
		if (g->ubSector.x != x || g->ubSector.y != y || g->ubSector.z != z) continue;
		/* We have a group, make sure that it isn't a group containing only dead
		 * members. */
		CFOR_EACH_PLAYER_IN_GROUP(p, g)
		{
			SOLDIERTYPE const* const s = p->pSoldier;
			if (s->bLife == 0)                      continue;
			// Robots count as mercs here, because they can fight, but vehicles don't
			if (s->uiStatusFlags & SOLDIER_VEHICLE) continue;
			n_mercs++;
		}
	}
	return n_mercs;
}

UINT8 PlayerGroupsInSector(const SGPSector& sector)
{
	UINT8 n_groups = 0;
	CFOR_EACH_PLAYER_GROUP(g)
	{
		if (g->fBetweenSectors) continue;
		if (g->ubSector != sector) continue;
		/* We have a group, make sure that it isn't a group containing only dead
		 * members. */
		CFOR_EACH_PLAYER_IN_GROUP(p, g)
		{
			if (p->pSoldier->bLife == 0) continue;
			++n_groups;
			break;
		}
	}
	return n_groups;
}


// is the player group with this id in motion?
bool PlayerIDGroupInMotion(UINT8 const id)
{
	GROUP* const g = GetGroup(id);
	return g && PlayerGroupInMotion(g);
}


// is the player group in motion?
BOOLEAN PlayerGroupInMotion(GROUP const* const pGroup)
{
	return( pGroup -> fBetweenSectors );
}


/* Add this group to the current battle fray!
 * NOTE: For enemies, only MAX_STRATEGIC_TEAM_SIZE at a time can be in a battle,
 * so if it ever gets past that, god help the player, but we'll have to insert
 * them as those slots free up. */
void HandleArrivalOfReinforcements(GROUP const* const g)
{
	if (g->fPlayer)
	{ /* We don't have to worry about filling up the player slots, because it is
		 * impossible to have more player's in the game than the number of slots
		 * available for the player. */

		/* First, determine which entrypoint to use, based on the travel direction
		 * of the group */
		UINT8 const x = g->ubSector.x;
		UINT8 const y = g->ubSector.y;
		SGPSector insertion = g->ubSector;
		insertion.z = 0;
		InsertionCode const strategic_insertion_code =
			x < g->ubPrev.x ? INSERTION_CODE_EAST  :
			x > g->ubPrev.x ? INSERTION_CODE_WEST  :
			y < g->ubPrev.y ? INSERTION_CODE_SOUTH :
			y > g->ubPrev.y ? INSERTION_CODE_NORTH :
			throw std::logic_error("reinforcements come from same sector");

		bool first = true;
		CFOR_EACH_PLAYER_IN_GROUP(p, g)
		{
			SOLDIERTYPE& s = *p->pSoldier;
			s.ubStrategicInsertionCode = strategic_insertion_code;
			UpdateMercInSector(s, insertion);

			// Do arrives quote
			if (first) TacticalCharacterDialogue(&s, QUOTE_MERC_REACHED_DESTINATION);
			first = false;
		}
		ScreenMsg(FONT_YELLOW, MSG_INTERFACE, g_langRes->Message[STR_PLAYER_REINFORCEMENTS]);
	}
	else
	{
		gfPendingEnemies = TRUE;
		ResetMortarsOnTeamCount();
		AddPossiblePendingEnemiesToBattle();
	}
}


BOOLEAN PlayersBetweenTheseSectors(INT16 const sec_src, INT16 const sec_dst, INT32* const n_enter, INT32* const n_exit, BOOLEAN* const about_to_arrive_enter)
{
	*n_enter               = 0;
	*n_exit                = 0;
	*about_to_arrive_enter = FALSE;

	GROUP const* const bg         = gpBattleGroup;
	INT16        const sec_battle = bg ? bg->ubSector.AsByte() : -1;

	/* Get number of characters entering/existing between these two sectors.
	 * Special conditions during pre-battle interface to return where this
	 * function is used to show potential retreating directions instead! */

	CFOR_EACH_PLAYER_GROUP(i)
	{
		GROUP const& g = *i;
		bool  const is_heli_group = IsGroupTheHelicopterGroup(g);

		/* If this group is aboard the helicopter and we're showing the airspace
		 * layer, don't count any mercs aboard the chopper, because the chopper icon
		 * itself serves the function of showing the location/size of this group. */
		if (is_heli_group && fShowAircraftFlag) continue;

		/* If only showing retreat paths, ignore groups not in the battle sector.
		 * If NOT showing retreat paths, ignore groups not between sectors. */
		if (gfDisplayPotentialRetreatPaths ? sec_battle != sec_src : !g.fBetweenSectors) continue;

		UINT8 n_mercs = g.ubGroupSize;
		if (n_mercs == 0) // Skip empty persistent groups
		{
			Assert(g.fPersistant);
			continue;
		}

		INT16 const sec_prev = g.ubPrev.IsValid() ? g.ubPrev.AsByte() : -1;
		INT16 const sec_cur  = g.ubSector.AsByte();
		INT16 const sec_next = g.ubNext.IsValid() ? g.ubNext.AsByte() : -1;

		bool const may_retreat_from_battle =
			sec_battle == sec_src && sec_cur == sec_src && sec_prev == sec_dst;

		bool const retreating_from_battle =
			sec_battle == sec_dst && sec_cur == sec_dst && sec_prev == sec_src;

		if (may_retreat_from_battle || (sec_cur == sec_src && sec_next == sec_dst))
		{
			// If it's a valid vehicle, but not the helicopter (which can fly empty)
			if (g.fVehicle && !is_heli_group)
			{ // subtract 1, we don't wanna count the vehicle itself for purposes of showing a number on the map
				n_mercs--;
			}

			*n_enter += n_mercs;

			if (may_retreat_from_battle ||
					g.uiArrivalTime - GetWorldTotalMin() <= ABOUT_TO_ARRIVE_DELAY)
			{
				*about_to_arrive_enter = TRUE;
			}
		}
		else if (retreating_from_battle || (sec_cur == sec_dst && sec_next == sec_src))
		{
			// If it's a valid vehicle, but not the helicopter (which can fly empty)
			if (g.fVehicle && !is_heli_group)
			{ // subtract 1, we don't wanna count the vehicle itself for purposes of showing a number on the map
				n_mercs--;
			}

			*n_exit += n_mercs;
		}
	}

	// if there was actually anyone leaving this sector and entering next
	return *n_enter > 0;
}


void MoveAllGroupsInCurrentSectorToSector(const SGPSector& sector)
{
	FOR_EACH_PLAYER_GROUP(g)
	{
		if (g->ubSector != gWorldSector) continue;
		if (g->fBetweenSectors)             continue;

		// This player group is in the currently loaded sector
		g->ubSector = sector;
		CFOR_EACH_PLAYER_IN_GROUP(p, g)
		{
			p->pSoldier->sSectorX        = sector.x;
			p->pSoldier->sSectorY        = sector.y;
			p->pSoldier->bSectorZ        = sector.z;
			p->pSoldier->fBetweenSectors = FALSE;
		}
	}
	CheckAndHandleUnloadingOfCurrentWorld();
}


static void SaveEnemyGroupStruct(HWFILE, GROUP const&);
static void SavePlayerGroupList(HWFILE, GROUP const*);
static void SaveWayPointList(HWFILE, GROUP const*);


void SaveStrategicMovementGroupsToSaveGameFile(HWFILE const f)
{
	// Save the number of movement groups to the saved game file
	UINT32 uiNumberOfGroups = 0;
	CFOR_EACH_GROUP(g) ++uiNumberOfGroups;
	f->write(&uiNumberOfGroups, sizeof(UINT32));

	CFOR_EACH_GROUP(g)
	{
		BYTE data[84];
		DataWriter d{data};
		INJ_BOOL(d, g->fDebugGroup)
		INJ_BOOL(d, g->fPlayer)
		INJ_BOOL(d, g->fVehicle)
		INJ_BOOL(d, g->fPersistant)
		INJ_U8(d, g->ubGroupID)
		INJ_U8(d, g->ubGroupSize)
		INJ_U8(d, g->ubSector.x)
		INJ_U8(d, g->ubSector.y)
		INJ_U8(d, g->ubSector.z)
		INJ_U8(d, g->ubNext.x)
		INJ_U8(d, g->ubNext.y)
		INJ_U8(d, g->ubPrev.x)
		INJ_U8(d, g->ubPrev.y)
		INJ_U8(d, g->ubOriginalSector)
		INJ_BOOL(d, g->fBetweenSectors)
		INJ_U8(d, g->ubMoveType)
		INJ_U8(d, g->ubNextWaypointID)
		INJ_SKIP(d, 3)
		INJ_U32(d, g->uiArrivalTime)
		INJ_U32(d, g->uiTraverseTime)
		INJ_SKIP(d, 8)
		INJ_U8(d, g->ubTransportationMask)
		INJ_SKIP(d, 3)
		INJ_U32(d, g->uiFlags)
		INJ_U8(d, g->ubCreatedSectorID)
		INJ_U8(d, g->ubSectorIDOfLastReassignment)
		INJ_SKIP(d, 38)
		Assert(d.getConsumed() == lengthof(data));

		f->write(data, sizeof(data));

		// Save the linked list, for the current type of group
		if (g->fPlayer)
		{
			if (g->ubGroupSize) SavePlayerGroupList(f, g);
		}
		else
		{
			SaveEnemyGroupStruct(f, *g);
		}

		SaveWayPointList(f, g);
	}

	// Save the unique id mask
	f->write(uniqueIDMask, sizeof(uniqueIDMask));
}


static void LoadEnemyGroupStructFromSavedGame(HWFILE, GROUP&);
static void LoadPlayerGroupList(HWFILE, GROUP*);
static void LoadWayPointList(HWFILE, GROUP*);


void LoadStrategicMovementGroupsFromSavedGameFile(HWFILE const f)
{
	Assert(gpGroupList == NULL);

	// Load the number of nodes in the list
	UINT32 uiNumberOfGroups;
	f->read(&uiNumberOfGroups, sizeof(UINT32));

	//loop through all the nodes and add them to the LL
	GROUP** anchor = &gpGroupList;
	for (UINT32 i = uiNumberOfGroups; i != 0; --i)
	{
		GROUP* const g = new GROUP{};

		BYTE data[84];
		f->read(data, sizeof(data));

		DataReader d{data};
		EXTR_BOOL(d, g->fDebugGroup)
		EXTR_BOOL(d, g->fPlayer)
		EXTR_BOOL(d, g->fVehicle)
		EXTR_BOOL(d, g->fPersistant)
		EXTR_U8(d, g->ubGroupID)
		EXTR_U8(d, g->ubGroupSize)
		EXTR_U8(d, g->ubSector.x)
		EXTR_U8(d, g->ubSector.y)
		EXTR_U8(d, g->ubSector.z)
		EXTR_U8(d, g->ubNext.x)
		EXTR_U8(d, g->ubNext.y)
		EXTR_U8(d, g->ubPrev.x)
		EXTR_U8(d, g->ubPrev.y)
		EXTR_U8(d, g->ubOriginalSector)
		EXTR_BOOL(d, g->fBetweenSectors)
		EXTR_U8(d, g->ubMoveType)
		EXTR_U8(d, g->ubNextWaypointID)
		EXTR_SKIP(d, 3)
		EXTR_U32(d, g->uiArrivalTime)
		EXTR_U32(d, g->uiTraverseTime)
		EXTR_SKIP(d, 8)
		EXTR_U8(d, g->ubTransportationMask)
		EXTR_SKIP(d, 3)
		EXTR_U32(d, g->uiFlags)
		EXTR_U8(d, g->ubCreatedSectorID)
		EXTR_U8(d, g->ubSectorIDOfLastReassignment)
		EXTR_SKIP(d, 38)
		Assert(d.getConsumed() == lengthof(data));

		if (g->fPlayer)
		{
			// If there is a player list, add it
			if (g->ubGroupSize) LoadPlayerGroupList(f, g);
		}
		else // Else it's an enemy group
		{
			LoadEnemyGroupStructFromSavedGame(f, *g);
		}

		LoadWayPointList(f, g);

		// Add the node to the list
		*anchor = g;
		anchor  = &g->next;
	}

	//@@@ TEMP!
	//Rebuild the uniqueIDMask as a very old bug broke the uniqueID assignments in extremely rare cases.
	std::fill(std::begin(uniqueIDMask), std::end(uniqueIDMask), 0);
	CFOR_EACH_GROUP(g)
	{
		const UINT32 index = g->ubGroupID / 32;
		const UINT32 bit   = g->ubGroupID % 32;
		const UINT32 mask  = 1 << bit;
		uniqueIDMask[index] += mask;
	}

	// Skip over saved unique id mask
	f->seek(32, FILE_SEEK_FROM_CURRENT);
}


// Saves the Player's group list to the saved game file
static void SavePlayerGroupList(HWFILE const f, GROUP const* const g)
{
	// Save the number of nodes in the list
	UINT32 uiNumberOfNodesInList = 0;
	CFOR_EACH_PLAYER_IN_GROUP(p, g) ++uiNumberOfNodesInList;
	f->write(&uiNumberOfNodesInList, sizeof(UINT32));

	// Loop through and save only the players profile id
	CFOR_EACH_PLAYER_IN_GROUP(p, g)
	{
		// Save the ubProfile ID for this node
		const UINT32 uiProfileID = p->pSoldier->ubProfile;
		f->write(&uiProfileID, sizeof(UINT32));
	}
}


//Loads the LL for the playerlist from the savegame file
static void LoadPlayerGroupList(HWFILE const f, GROUP* const g)
{
	// Load the number of nodes in the player list
	UINT32 node_count;
	f->read(&node_count, sizeof(UINT32));

	PLAYERGROUP** anchor = &g->pPlayerList;
	for (UINT32 i = node_count; i != 0; --i)
	{
		PLAYERGROUP* const pg = new PLAYERGROUP{};

		UINT32 profile_id;
		f->read(&profile_id, sizeof(UINT32));

		SOLDIERTYPE* const s = FindSoldierByProfileIDOnPlayerTeam(profile_id);
		//Should never happen
		//Assert(s != NULL);
		pg->pSoldier = s;
		pg->next     = NULL;

		*anchor = pg;
		anchor  = &pg->next;
	}
}


// Saves the enemy group struct to the saved game file
static void SaveEnemyGroupStruct(HWFILE const f, GROUP const& g)
{
	BYTE              data[29];
	DataWriter d{data};
	ENEMYGROUP const& eg = *g.pEnemyGroup;
	INJ_U8(  d, eg.ubNumTroops)
	INJ_U8(  d, eg.ubNumElites)
	INJ_U8(  d, eg.ubNumAdmins)
	INJ_SKIP(d, 1)
	INJ_U8(  d, eg.ubPendingReinforcements)
	INJ_U8(  d, eg.ubAdminsInBattle)
	INJ_U8(  d, eg.ubIntention)
	INJ_U8(  d, eg.ubTroopsInBattle)
	INJ_U8(  d, eg.ubElitesInBattle)
	INJ_SKIP(d, 20)
	Assert(d.getConsumed() == lengthof(data));

	f->write(data, sizeof(data));
}


// Loads the enemy group struct from the saved game file
static void LoadEnemyGroupStructFromSavedGame(HWFILE const f, GROUP& g)
{
	BYTE data[29];
	f->read(data, sizeof(data));

	ENEMYGROUP* const eg = new ENEMYGROUP{};
	DataReader d{data};
	EXTR_U8(  d, eg->ubNumTroops)
	EXTR_U8(  d, eg->ubNumElites)
	EXTR_U8(  d, eg->ubNumAdmins)
	EXTR_SKIP(d, 1)
	EXTR_U8(  d, eg->ubPendingReinforcements)
	EXTR_U8(  d, eg->ubAdminsInBattle)
	EXTR_U8(  d, eg->ubIntention)
	EXTR_U8(  d, eg->ubTroopsInBattle)
	EXTR_U8(  d, eg->ubElitesInBattle)
	EXTR_SKIP(d, 20)
	Assert(d.getConsumed() == lengthof(data));

	g.pEnemyGroup = eg;
}


static void SaveWayPointList(HWFILE const f, GROUP const* const g)
{
	// Save the number of waypoints
	UINT32 uiNumberOfWayPoints = 0;
	for (const WAYPOINT* w = g->pWaypoints; w != NULL; w = w->next)
	{
		++uiNumberOfWayPoints;
	}
	f->write(&uiNumberOfWayPoints, sizeof(UINT32));

	for (const WAYPOINT* w = g->pWaypoints; w != NULL; w = w->next)
	{
		BYTE  data[8];
		DataWriter d{data};
		INJ_U8(  d, w->x)
		INJ_U8(  d, w->y)
		INJ_SKIP(d, 6)
		Assert(d.getConsumed() == lengthof(data));

		f->write(data, sizeof(data));
	}
}


static void LoadWayPointList(HWFILE const f, GROUP* const g)
{
	// Load the number of waypoints
	UINT32 uiNumberOfWayPoints;
	f->read(&uiNumberOfWayPoints, sizeof(UINT32));

	WAYPOINT** anchor = &g->pWaypoints;
	for (UINT32 i = uiNumberOfWayPoints; i != 0; --i)
	{
		WAYPOINT* const w = new WAYPOINT{};

		BYTE data[8];
		f->read(data, sizeof(data));

		DataReader d{data};
		EXTR_U8(  d, w->x)
		EXTR_U8(  d, w->y)
		EXTR_SKIP(d, 6)
		Assert(d.getConsumed() == lengthof(data));

		// Add the node to the list
		*anchor = w;
		anchor  = &w->next;
	}
	*anchor = NULL;
}


void CalculateGroupRetreatSector( GROUP *pGroup )
{
	SECTORINFO *pSector;
	UINT32 uiSectorID;

	uiSectorID = pGroup->ubSector.AsByte();
	pSector = &SectorInfo[ uiSectorID ];

	if( pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrev.x = pGroup->ubSector.x;
		pGroup->ubPrev.y = pGroup->ubSector.y - 1;
	}
	else if( pSector->ubTraversability[ EAST_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ EAST_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrev.x = pGroup->ubSector.x + 1;
		pGroup->ubPrev.y = pGroup->ubSector.y;
	}
	else if( pSector->ubTraversability[ WEST_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ WEST_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrev.x = pGroup->ubSector.x - 1;
		pGroup->ubPrev.y = pGroup->ubSector.y;
	}
	else if( pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrev.x = pGroup->ubSector.x;
		pGroup->ubPrev.y = pGroup->ubSector.y + 1;
	}
	else
	{
		STLOGA("Player group cannot retreat from sector {} ", pGroup->ubSector.AsShortString());
		return;
	}
	if( pGroup->fPlayer )
	{ //update the previous sector for the mercs
		CFOR_EACH_PLAYER_IN_GROUP(pPlayer, pGroup)
		{
			pPlayer->pSoldier->ubPrevSectorID = pGroup->ubPrev.AsByte();
		}
	}
}

//Called when all checks have been made for the group (if possible to retreat, etc.)  This function
//blindly determines where to move the group.
void RetreatGroupToPreviousSector(GROUP& g)
{
	AssertMsg(!g.fBetweenSectors, "Can't retreat a group when between sectors!");

	UINT8 direction = 255;
	if (g.ubPrev.x != 16 || g.ubPrev.y != 16)
	{ // Group has a previous sector
		g.ubNext = g.ubPrev;

		// Determine the correct direction
		INT32 const dx = g.ubNext.x - g.ubSector.x;
		INT32 const dy = g.ubNext.y - g.ubSector.y;
		if      (dx ==  0 && dy == -1) direction = NORTH_STRATEGIC_MOVE;
		else if (dx ==  1 && dy ==  0) direction = EAST_STRATEGIC_MOVE;
		else if (dx ==  0 && dy ==  1) direction = SOUTH_STRATEGIC_MOVE;
		else if (dx == -1 && dy ==  0) direction = WEST_STRATEGIC_MOVE;
		else
		{
			throw std::runtime_error(ST::format("Player group attempting illegal retreat from {} to {}.", g.ubSector.AsShortString(), g.ubNext.AsShortString()).to_std_string());
		}
	}
	else
	{ // Group doesn't have a previous sector. Create one, then recurse
		CalculateGroupRetreatSector(&g);
		RetreatGroupToPreviousSector(g);
		// XXX direction is invalid, causes out-of-bounds access below
	}

	// Calc time to get to next waypoint
	UINT8 const sector = g.ubSector.AsByte();
	g.uiTraverseTime = GetSectorMvtTimeForGroup(sector, direction, &g);
	AssertMsg(g.uiTraverseTime != TRAVERSE_TIME_IMPOSSIBLE, String("Group %d (%s) attempting illegal move from %c%d to %c%d (%s).", g.ubGroupID, g.fPlayer ? "Player" : "AI", g.ubSector.y + 'A', g.ubSector.x, g.ubNext.y + 'A', g.ubNext.x, gszTerrain[SectorInfo[sector].ubTraversability[direction]]));

	// Because we are in the strategic layer, don't make the arrival instantaneous (towns)
	if (g.uiTraverseTime == 0) g.uiTraverseTime = 5;

	g.setArrivalTime(GetWorldTotalMin() + g.uiTraverseTime);
	g.fBetweenSectors = TRUE;
	g.uiFlags        |= GROUPFLAG_JUST_RETREATED_FROM_BATTLE;

	if (g.fVehicle)
	{ // Vehicle, set fact it is between sectors too
		VEHICLETYPE& v = GetVehicleFromMvtGroup(g);
		v.fBetweenSectors = TRUE;
	}

	if (!AddStrategicEvent(EVENT_GROUP_ARRIVAL, g.uiArrivalTime, g.ubGroupID))
		SLOGA("Failed to add movement event.");

	// For the case of player groups, we need to update the information of the soldiers.
	if (g.fPlayer)
	{
		if (g.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin())
		{
			AddStrategicEvent(EVENT_GROUP_ABOUT_TO_ARRIVE, g.uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, g.ubGroupID);
		}

		CFOR_EACH_PLAYER_IN_GROUP(i, &g)
		{
			SOLDIERTYPE& s = *i->pSoldier;
			s.fBetweenSectors = TRUE;
			RemoveSoldierFromTacticalSector(s);
		}
	}
}


GROUP* FindEnemyMovementGroupInSector(const UINT8 ubSectorX, const UINT8 ubSectorY)
{
	FOR_EACH_ENEMY_GROUP(g)
	{
		if (g->ubSector.x == ubSectorX &&
				g->ubSector.y == ubSectorY &&
				g->ubSector.z == 0)
		{
			return g;
		}
	}
	return NULL;
}


GROUP* FindPlayerMovementGroupInSector(const UINT8 x, const UINT8 y)
{
	FOR_EACH_PLAYER_GROUP(i)
	{
		GROUP& g = *i;
		// NOTE: These checks must always match the INVOLVED group checks in PBI!!!
		if (g.ubGroupSize != 0 &&
			!g.fBetweenSectors &&
			g.ubSector.x   == x &&
			g.ubSector.y   == y &&
			g.ubSector.z   == 0 &&
			!GroupHasInTransitDeadOrPOWMercs(g) &&
			(!IsGroupTheHelicopterGroup(g) || !fHelicopterIsAirBorne))
		{
			return &g;
		}
	}
	return NULL;
}


BOOLEAN GroupAtFinalDestination(const GROUP* const pGroup)
{
	WAYPOINT *wp;

	if( pGroup->ubMoveType != ONE_WAY )
		return FALSE; //Group will continue to patrol, hence never stops.

	//Determine if we are at the final waypoint.
	wp = GetFinalWaypoint( pGroup );

	if( !wp )
	{ //no waypoints, so the group is at it's destination.  This happens when
		//an enemy group is created in the destination sector (which is legal for
		//staging groups which always stop adjacent to their real sector destination)
		return TRUE;
	}

	// if we're there
	if (pGroup->ubSector.x == wp->x && pGroup->ubSector.y == wp->y)
	{
		return TRUE;
	}

	return FALSE;
}


WAYPOINT* GetFinalWaypoint(const GROUP* const pGroup)
{
	WAYPOINT *wp;

	Assert( pGroup );

	//Make sure they're on a one way route, otherwise this request is illegal
	Assert( pGroup->ubMoveType == ONE_WAY );

	wp = pGroup->pWaypoints;
	if( wp )
	{
		while( wp->next )
		{
			wp = wp->next;
		}
	}

	return( wp );
}


static void ResetMovementForEnemyGroup(GROUP* pGroup);

// See comments in ResetMovementForEnemyGroup() for more details on what the resetting does
void ResetMovementForEnemyGroupsInLocation()
{
	SGPSector sSector;
	GetCurrentBattleSectorXYZ(sSector);

	FOR_EACH_GROUP_SAFE(pGroup)
	{
		if( !pGroup->fPlayer )
		{
			if (pGroup->ubSector == sSector)
			{
				ResetMovementForEnemyGroup( pGroup );
			}
		}
	}
}


//This function is used to reset the location of the enemy group if they are
//currently between sectors.  If they were 50% of the way from sector A10 to A11,
//then after this function is called, then that group would be 0% of the way from
//sector A10 to A11.  In no way does this function effect the strategic path for
//the group.
static void ResetMovementForEnemyGroup(GROUP* pGroup)
{
	//Validate that the group is an enemy group and that it is moving.
	if( pGroup->fPlayer )
	{
		return;
	}
	if (!pGroup->fBetweenSectors || !pGroup->ubNext.x || !pGroup->ubNext.y)
	{ //Reset the group's assignment by moving it to the group's original sector as it's pending group.
		RepollSAIGroup( pGroup );
		return;
	}

	//Cancel the event that is posted.
	DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );

	//Calculate the new arrival time (all data pertaining to movement should be valid)
	if( pGroup->uiTraverseTime > 400 )
	{ //The group was likely sleeping which makes for extremely long arrival times.  Shorten it
		//arbitrarily.  Doesn't really matter if this isn't accurate.
		pGroup->uiTraverseTime = 90;
	}
	pGroup->setArrivalTime(GetWorldTotalMin() + pGroup->uiTraverseTime);

	//Add a new event
	AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );
}


void UpdatePersistantGroupsFromOldSave( UINT32 uiSavedGameVersion )
{
	INT32   cnt;
	BOOLEAN fDoChange = FALSE;

	// ATE: If saved game is < 61, we need to do something better!
	if( uiSavedGameVersion < 61 )
	{
		for( cnt = 0; cnt < 55; cnt++ )
		{
			// create mvt groups
			GROUP* const pGroup = GetGroup(cnt);
			if( pGroup != NULL && pGroup->fPlayer )
			{
				pGroup->fPersistant = TRUE;
			}
		}

		fDoChange = TRUE;
	}
	else if( uiSavedGameVersion < 63 )
	{
		for( cnt = 0; cnt <  NUMBER_OF_SQUADS; cnt++ )
		{
			// create mvt groups
			GROUP* const pGroup = GetGroup(SquadMovementGroups[cnt]);
			if ( pGroup != NULL )
			{
				pGroup->fPersistant = TRUE;
			}
		}

		for( cnt = 0; cnt <  MAX_VEHICLES; cnt++ )
		{
			GROUP* const pGroup = GetGroup(gubVehicleMovementGroups[cnt]);
			if ( pGroup != NULL )
			{
				pGroup->fPersistant = TRUE;
			}
		}

		fDoChange = TRUE;
	}

	if ( fDoChange )
	{
		//Remove all empty groups
		FOR_EACH_GROUP_SAFE(i)
		{
			GROUP& g = *i;
			if (g.ubGroupSize == 0 && !g.fPersistant) RemoveGroup(g);
		}
	}
}

//Determines if any particular group WILL be moving through a given sector given it's current
//position in the route and the pGroup->ubMoveType must be ONE_WAY.  If the group is currently
//IN the sector, or just left the sector, it will return FALSE.
BOOLEAN GroupWillMoveThroughSector(GROUP *pGroup, const SGPSector& sSector)
{
	WAYPOINT *wp;
	INT32 i, dx, dy;
	UINT8 ubOrigX, ubOrigY;

	Assert( pGroup );
	AssertMsg( pGroup->ubMoveType == ONE_WAY, String( "GroupWillMoveThroughSector() -- Attempting to test group with an invalid move type.  ubGroupID: %d, ubMoveType: %d, sector: %c%d -- KM:0",
			pGroup->ubGroupID, pGroup->ubMoveType, pGroup->ubSector.y + 'A' - 1, pGroup->ubSector.x ) );

	//Preserve the original sector values, as we will be temporarily modifying the group's ubSectorX/Y values
	//as we traverse the waypoints.
	ubOrigX = pGroup->ubSector.x;
	ubOrigY = pGroup->ubSector.y;

	i = pGroup->ubNextWaypointID;
	wp = pGroup->pWaypoints;

	if( !wp )
	{ //This is a floating group!?
		return FALSE;
	}
	while( i-- )
	{ //Traverse through the waypoint list to the next waypoint ID
		Assert( wp );
		wp = wp->next;
	}
	Assert( wp );


	while( wp )
	{
		while (pGroup->ubSector.x != wp->x || pGroup->ubSector.y != wp->y)
		{
			//We now have the correct waypoint.
			//Analyse the group and determine which direction it will move from the current sector.
			dx = wp->x - pGroup->ubSector.x;
			dy = wp->y - pGroup->ubSector.y;
			if( dx && dy )
			{ //Can't move diagonally!
				SLOGA("GroupWillMoveThroughSector() -- Attempting to process waypoint in a diagonal direction from sector %c%d to sector %c%d for group at sector %c%d",
					pGroup->ubSector.y + 'A', pGroup->ubSector.x, wp->y + 'A' - 1, wp->x, ubOrigY + 'A' - 1, ubOrigX);
				pGroup->ubSector.x = ubOrigX;
				pGroup->ubSector.y = ubOrigY;
				return TRUE;
			}
			if( !dx && !dy ) //Can't move to position currently at!
			{
				SLOGA("GroupWillMoveThroughSector() -- Attempting to process same waypoint at %c%d for group at %c%d",
					wp->y + 'A' - 1, wp->x, ubOrigY + 'A' - 1, ubOrigX);
				pGroup->ubSector.x = ubOrigX;
				pGroup->ubSector.y = ubOrigY;
				return TRUE;
			}
			//Clip dx/dy value so that the move is for only one sector.
			if( dx >= 1 )
			{
				dx = 1;
			}
			else if( dy >= 1 )
			{
				dy = 1;
			}
			else if( dx <= -1 )
			{
				dx = -1;
			}
			else if( dy <= -1 )
			{
				dy = -1;
			}
			//Advance the sector value
			pGroup->ubSector.x = (UINT8)( dx + pGroup->ubSector.x );
			pGroup->ubSector.y = (UINT8)( dy + pGroup->ubSector.y );
			//Check to see if it the sector we are checking to see if this group will be moving through.
			if (pGroup->ubSector == sSector)
			{
				pGroup->ubSector.x = ubOrigX;
				pGroup->ubSector.y = ubOrigY;
				return TRUE;
			}
		}
		//Advance to the next waypoint.
		wp = wp->next;
	}
	pGroup->ubSector.x = ubOrigX;
	pGroup->ubSector.y = ubOrigY;
	return FALSE;
}

static INT16 VehicleFuelRemaining(SOLDIERTYPE const& vs)
{
	Assert(vs.uiStatusFlags & SOLDIER_VEHICLE);
	return vs.sBreathRed;
}


static void SpendVehicleFuel(SOLDIERTYPE& vs, INT16 const fuel_spent)
{
	Assert(vs.uiStatusFlags & SOLDIER_VEHICLE);
	vs.sBreathRed  = MAX(0, vs.sBreathRed - fuel_spent);
	vs.bBreath     = (vs.sBreathRed + 99) / 100;
}


void AddFuelToVehicle(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pVehicle)
{
	OBJECTTYPE *pItem;
	INT16 sFuelNeeded, sFuelAvailable, sFuelAdded;
	pItem = &pSoldier->inv[ HANDPOS ];
	if( pItem->usItem != GAS_CAN )
	{
		return;
	}
	//Soldier has gas can, so now add gas to vehicle while removing gas from the gas can.
	//A gas can with 100 status translate to 50% of a fillup.
	if( pVehicle->sBreathRed == 10000 )
	{ //Message for vehicle full?
		return;
	}
	// pItem->bStatus is always present, no need to check for it
	//Fill 'er up.
	sFuelNeeded = 10000 - pVehicle->sBreathRed;
	sFuelAvailable = pItem->bStatus[0] * 50;
	sFuelAdded = MIN( sFuelNeeded, sFuelAvailable );
	//Add to vehicle
	pVehicle->sBreathRed += sFuelAdded;
	pVehicle->bBreath = (INT8)(pVehicle->sBreathRed / 100);
	//Subtract from item
	pItem->bStatus[0] = (INT8)(pItem->bStatus[0] - sFuelAdded / 50);
	if( !pItem->bStatus[0] )
	{ //Gas can is empty, so toast the item.
		DeleteObj( pItem );
	}
}


static void ReportVehicleOutOfGas(VEHICLETYPE const& v, const SGPSector& sMap)
{
	// Report that the vehicle that just arrived is out of gas
	ST::string str = st_format_printf(gzLateLocalizedString[STR_LATE_05], pVehicleStrings[v.ubVehicleType], sMap.AsShortString());
	DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, 0);
}


static void SetLocationOfAllPlayerSoldiersInGroup(GROUP const& g, const SGPSector& sSector)
{
	CFOR_EACH_PLAYER_IN_GROUP(i, &g)
	{
		if (!i->pSoldier) continue;
		SOLDIERTYPE& s = *i->pSoldier;
		s.sSectorX = sSector.x;
		s.sSectorY = sSector.y;
		s.bSectorZ = sSector.z;
	}

	if (g.fVehicle)
	{
		VEHICLETYPE& v = GetVehicleFromMvtGroup(g);
		v.sSector = sSector;

		if (!IsHelicopter(v))
		{
			SOLDIERTYPE& vs = GetSoldierStructureForVehicle(v);
			/* These are apparently unnecessary, since vehicles are part of the
				* pPlayerList in a vehicle group. Oh well. */
			vs.sSectorX = sSector.x;
			vs.sSectorY = sSector.y;
			vs.bSectorZ = sSector.z;
		}
	}
}


void RandomizePatrolGroupLocation( GROUP *pGroup )
{	//Make sure this is an enemy patrol group
	WAYPOINT *wp;
	UINT8 ubMaxWaypointID = 0;
	UINT8 ubTotalWaypoints;
	UINT8 ubChosen;
	UINT8 ubSectorID;

	//return; //disabled for now

	Assert( !pGroup->fPlayer );
	Assert( pGroup->ubMoveType == ENDTOEND_FORWARDS );
	Assert( pGroup->pEnemyGroup->ubIntention == PATROL );

	//Search for the event, and kill it (if it exists)!
	DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );

	//count the group's waypoints
	wp = pGroup->pWaypoints;
	while( wp )
	{
		if( wp->next )
		{
			ubMaxWaypointID++;
		}
		wp = wp->next;
	}
	//double it (they go back and forth) -- it's using zero based indices, so you have to add one to get the number of actual
	//waypoints in one direction.
	ubTotalWaypoints = (UINT8)((ubMaxWaypointID) * 2);

	//pick the waypoint they start at
	ubChosen = (UINT8)Random( ubTotalWaypoints );

	if( ubChosen >= ubMaxWaypointID )
	{ //They chose a waypoint going in the reverse direction, so translate it
		//to an actual waypointID and switch directions.
		pGroup->ubMoveType = ENDTOEND_BACKWARDS;
		pGroup->ubNextWaypointID = ubChosen - ubMaxWaypointID;
		ubChosen = pGroup->ubNextWaypointID + 1;
	}
	else
	{
		pGroup->ubMoveType = ENDTOEND_FORWARDS;
		pGroup->ubNextWaypointID = ubChosen + 1;
	}

	//Traverse through the waypoint list again, to extract the location they are at.
	wp = pGroup->pWaypoints;
	while( wp && ubChosen )
	{
		ubChosen--;
		wp = wp->next;
	}

	//logic error if this fails.  We should have a null value for ubChosen
	Assert( !ubChosen );
	Assert( wp );

	//Move the group to the location of this chosen waypoint.
	ubSectorID = (UINT8)SECTOR( wp->x, wp->y );

	//Set up this global var to randomize the arrival time of the group from
	//1 minute to actual traverse time between the sectors.
	gfRandomizingPatrolGroup = TRUE;

	SetEnemyGroupSector(*pGroup, ubSectorID);
	InitiateGroupMovementToNextSector( pGroup );

	//Immediately turn off the flag once finished.
	gfRandomizingPatrolGroup = FALSE;

}


//Whenever a player group arrives in a sector, and if bloodcats exist in the sector,
//roll the dice to see if this will become an ambush random encounter.
static BOOLEAN TestForBloodcatAmbush(GROUP const* const pGroup)
{
	SECTORINFO *pSector;
	INT32 iHoursElapsed;
	UINT8 ubSectorID;
	UINT8 ubChance;
	INT8 bDifficultyMaxCats;
	INT8 bProgressMaxCats;
	INT8 bNumMercMaxCats;
	BOOLEAN fAlreadyAmbushed = FALSE;

	SGPSector gSector = pGroup->ubSector;
	if (gSector.z)
	{ //no ambushes underground (no bloodcats either)
		return FALSE;
	}

	ubSectorID = gSector.AsByte();
	pSector = &SectorInfo[ ubSectorID ];

	ubChance = 5 * gGameOptions.ubDifficultyLevel;

	bool bIsLair = false, bIsArena = false;
	auto spawns = GCM->getBloodCatSpawnsOfSector( ubSectorID );
	if (spawns != NULL) {
		bIsLair = spawns->isLair;   // SEC_I16
		bIsArena = spawns->isArena; // SEC_N5
	}

	iHoursElapsed = (GetWorldTotalMin() - pSector->uiTimeCurrentSectorWasLastLoaded) / 60;
	if( bIsLair || bIsArena )
	{ //These are special maps -- we use all placements.
		if( pSector->bBloodCats == -1 )
		{
			pSector->bBloodCats = pSector->bBloodCatPlacements;
		}
		else if( pSector->bBloodCats > 0 && pSector->bBloodCats < pSector->bBloodCatPlacements )
		{ //Slowly have them recuperate if we haven't been here for a long time.  The population will
			//come back up to the maximum if left long enough.
			INT32 iBloodCatDiff;
			iBloodCatDiff = pSector->bBloodCatPlacements - pSector->bBloodCats;
			pSector->bBloodCats += (INT8)MIN( iHoursElapsed / 18, iBloodCatDiff );
		}
		//Once 0, the bloodcats will never recupe.
	}
	else if( pSector->bBloodCats == -1 )
	{ //If we haven't been ambushed by bloodcats yet...
		if( gfAutoAmbush || PreChance( ubChance ) )
		{
			//randomly choose from 5-8, 7-10, 9-12 bloodcats based on easy, normal, and hard, respectively
			bDifficultyMaxCats = (INT8)( Random( 4 ) + gGameOptions.ubDifficultyLevel*2 + 3 );

			//maximum of 3 bloodcats or 1 for every 6%, 5%, 4% progress based on easy, normal, and hard, respectively
			bProgressMaxCats = (INT8)MAX( CurrentPlayerProgressPercentage() / (7 - gGameOptions.ubDifficultyLevel), 3 );

			//make sure bloodcats don't outnumber mercs by a factor greater than 2
			bNumMercMaxCats = (INT8) (PlayerMercsInSector(gSector.x, gSector.y, gSector.z) * 2);

			//choose the lowest number of cats calculated by difficulty and progress.
			pSector->bBloodCats = (INT8)MIN( bDifficultyMaxCats, bProgressMaxCats );

			if( gGameOptions.ubDifficultyLevel != DIF_LEVEL_HARD )
			{ //if not hard difficulty, ensure cats never outnumber mercs by a factor of 2 (min 3 bloodcats)
				pSector->bBloodCats = (INT8)MIN( pSector->bBloodCats, bNumMercMaxCats );
				pSector->bBloodCats = (INT8)MAX( pSector->bBloodCats, 3 );
			}

			//ensure that there aren't more bloodcats than placements
			pSector->bBloodCats = (INT8)MIN( pSector->bBloodCats, pSector->bBloodCatPlacements );
		}
	}
	else if( !bIsLair )
	{
		if( !gfAutoAmbush && PreChance( 95 ) )
		{ //already ambushed here.  But 5% chance of getting ambushed again!
			fAlreadyAmbushed = TRUE;
		}
	}

	if( !fAlreadyAmbushed && !bIsArena && pSector->bBloodCats > 0 &&
			!pGroup->fVehicle && !NumEnemiesInSector(gSector))
	{
		if( !bIsLair || !gubFact[ FACT_PLAYER_KNOWS_ABOUT_BLOODCAT_LAIR ] )
		{
			gubEnemyEncounterCode = BLOODCAT_AMBUSH_CODE;
		}
		else
		{
			gubEnemyEncounterCode = ENTERING_BLOODCAT_LAIR_CODE;
		}
		return TRUE;
	}
	else
	{
		gubEnemyEncounterCode = NO_ENCOUNTER_CODE;
		return FALSE;
	}
}


static void NotifyPlayerOfBloodcatBattle(const SGPSector& ubSector)
{
	ST::string str;
	ST::string zTempString;
	if( gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE )
	{
		zTempString = GetSectorIDString(ubSector, TRUE);
		str = st_format_printf(pMapErrorString[ 12 ], zTempString);
	}
	else if( gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE )
	{
		str = pMapErrorString[ 13 ];
	}

	if( guiCurrentScreen == MAP_SCREEN )
	{	//Force render mapscreen (need to update the position of the group before the dialog appears.
		fMapPanelDirty = TRUE;
		MapScreenHandle();
		InvalidateScreen();
		RefreshScreen();
	}

	gfUsePersistantPBI = TRUE;
	DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, TriggerPrebattleInterface );
}


void PlaceGroupInSector(GROUP& g, const SGPSector& prev, const SGPSector& next, bool check_for_battle)
{
	ClearMercPathsAndWaypointsForAllInGroup(g);
	// Change where they are and where they're going
	g.ubPrev = prev;
	SetGroupSectorValue(prev.x, prev.y, next.z, g); // only one user cares about Z this way
	SetGroupNextSectorValue(next, g);
	// Call arrive event
	GroupArrivedAtSector(g, check_for_battle, FALSE);
}

void GROUP::setArrivalTime(UINT32 arrival_time)
{
	/* Please centralize all changes to the arrival times of groups through here,
	 * especially the helicopter group! */

	/* If this group is the helicopter group, we have to make sure that its
	 * arrival time is never greater than the sum of the current time and its
	 * traverse time, because those 3 values are used to plot its map position!
	 * Because of this the chopper groups must NEVER be delayed for any reason -
	 * it gets excluded from simultaneous arrival logic */

	/* Also note that non-chopper groups can currently be delayed such that this
	 * assetion would fail - enemy groups by DelayEnemyGroupsIfPathsCross(), and
	 * player groups via PrepareGroupsForSimultaneousArrival().  So we skip the
	 * assert. */

	if (IsGroupTheHelicopterGroup(*this))
	{
		// Make sure it's valid (NOTE: the correct traverse time must be set first!)
		UINT32 const now = GetWorldTotalMin();
		if (arrival_time > now + this->uiTraverseTime)
		{
			AssertMsg(FALSE, String( "setArrivalTime: Setting invalid arrival time %d for group %d, WorldTime = %d, TraverseTime = %d", arrival_time, this->ubGroupID, now, this->uiTraverseTime));
			// Fix it if assertions are disabled
			arrival_time = now + this->uiTraverseTime;
		}
	}

	this->uiArrivalTime = arrival_time;
}


// Non-persistent groups should be simply removed instead
static void CancelEmptyPersistentGroupMovement(GROUP& g)
{
	Assert(g.ubGroupSize == 0);
	Assert(g.fPersistant);

	/* Don't do this for vehicle groups - the chopper can keep flying empty, while
	 * other vehicles still exist and teleport to nearest sector instead */
	if (g.fVehicle) return;

	// Prevent it from arriving empty
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g.ubGroupID);

	RemoveGroupWaypoints(g);
	g.uiTraverseTime  = 0;
	g.setArrivalTime(0);
	g.fBetweenSectors = FALSE;
	g.ubPrev.x        = 0;
	g.ubPrev.y        = 0;
	g.ubSector.x       = 0;
	g.ubSector.y       = 0;
	g.ubNext.x        = 0;
	g.ubNext.y        = 0;
}


static bool HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(GROUP&);


// look for NPCs to stop for, anyone is too tired to keep going, if all OK rebuild waypoints & continue movement
void PlayerGroupArrivedSafelyInSector(GROUP& g, BOOLEAN const fCheckForNPCs)
{
	BOOLEAN fPlayerPrompted = FALSE;

	Assert(g.fPlayer);

	// if we haven't already checked for NPCs, and the group isn't empty
	if (fCheckForNPCs && HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(g))
	{
		// wait for player to answer/confirm prompt before doing anything else
		fPlayerPrompted = TRUE;
	}

	// if we're not prompting the player
	if ( !fPlayerPrompted )
	{
		// and we're not at the end of our road
		if (!GroupAtFinalDestination(&g))
		{
			if (AnyMercInGroupCantContinueMoving(g))
			{
				// stop: clear their strategic movement (mercpaths and waypoints)
				ClearMercPathsAndWaypointsForAllInGroup(g);

				// NOTE: Of course, it would be better if they continued onwards once everyone was ready to go again, in which
				// case we'd want to preserve the plotted path, but since the player can mess with the squads, etc.
				// in the mean-time, that just seemed to risky to try to support.  They could get into a fight and be too
				// injured to move, etc.  Basically, we'd have run a complete CanCharacterMoveInStrategic(0 check on all of them.
				// It's a wish list task for AM...

				// stop time so player can react if group was already on the move and suddenly halts
				StopTimeCompression();
			}
			else
			{
				// continue onwards: rebuild way points, initiate movement
				RebuildWayPointsForGroupPath(GetGroupMercPathPtr(g), g);
			}
		}
	}
}


static void HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback(MessageBoxReturnValue);


static bool HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(GROUP& g)
{
	Assert(g.fPlayer);

	// Nobody in the group (perfectly legal with the chopper).
	if (!g.pPlayerList) return false;

	// Chopper doesn't stop for NPCs.
	if (IsGroupTheHelicopterGroup(g)) return false;

	/* If we're already in the middle of a prompt (possible with simultaneously
	 * group arrivals!), don't try to prompt again. */
	if (gpGroupPrompting) return false;

	SGPSector sector = g.ubSector;

	// Check for profiled NPCs in sector.
	if (!WildernessSectorWithAllProfiledNPCsNotSpokenWith(sector)) return false;

	// Store the group pointer for use by the callback function.
	gpGroupPrompting = &g;

	// Build string for squad.
	ST::string sector_name = GetSectorIDString(sector, FALSE);
	ST::string msg = st_format_printf(pLandMarkInSectorString, g.pPlayerList->pSoldier->bAssignment + 1, sector_name);

	MessageBoxFlags const flags =
		GroupAtFinalDestination(&g) ? MSG_BOX_FLAG_OK :
		MSG_BOX_FLAG_CONTINUESTOP;
	DoScreenIndependantMessageBox(msg, flags, HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback);
	// wait, we're prompting the player
	return true;
}


bool WildernessSectorWithAllProfiledNPCsNotSpokenWith(const SGPSector& sector)
{
	bool found_somebody = false;

	// Don't do this for underground sectors.
	if (sector.z != 0) return false;

	// Skip towns/pseudo-towns (anything that shows up on the map as being special).
	if (StrategicMap[sector.AsStrategicIndex()].bNameId != BLANK_SECTOR) return false;

	// Skip SAM sites.
	if (IsThisSectorASAMSector(sector)) return false;

	for (const MercProfile* profile : GCM->listMercProfiles())
	{
		// Skip player mercs
		if (profile->isPlayerMerc())       continue;

		MERCPROFILESTRUCT const& p = profile->getStruct();

		// Skip dead.
		if (p.bMercStatus == MERC_IS_DEAD) continue;
		if (p.bLife <= 0)                  continue;
 		// Skip vehicles.
		if (profile->isVehicle()) continue;

		// In this sector?
		if (p.sSectorX != sector.x || p.sSectorY != sector.y || p.bSectorZ != sector.z) continue;

		if (p.ubLastDateSpokenTo != 0 ||
				p.ubMiscFlags & (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE))
		{ /* Already spoke to this guy, don't prompt about this sector again,
			 * regardless of status of other NPCs here. Although Hamous wanders
			 * around, he never shares the same wilderness sector as other important
			 * NPCs. */
			return false;
		}
		/* We haven't talked to him yet and he's not currently recruired/escorted by
		 * player. This is a guy we need to stop for. */
		found_somebody = true;
	}
	return found_somebody;
}


static void HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback(MessageBoxReturnValue const exit_value)
{
	Assert(gpGroupPrompting);
	GROUP& g = *gpGroupPrompting;
	gpGroupPrompting = 0;
	switch (exit_value)
	{
		case MSG_BOX_RETURN_YES:
		case MSG_BOX_RETURN_OK:
			// NPCs now checked, continue moving if appropriate
			PlayerGroupArrivedSafelyInSector(g, FALSE);
			break;

		case MSG_BOX_RETURN_NO:
			// Stop here
			ClearMercPathsAndWaypointsForAllInGroup(g);
			ChangeSelectedMapSector(g.ubSector);
			StopTimeCompression();
			break;
				default:
						break;
	}
	fMapPanelDirty        = TRUE;
	fMapScreenBottomDirty = TRUE;
}


bool DoesPlayerExistInPGroup(GROUP const& g, SOLDIERTYPE const& s)
{
	CFOR_EACH_PLAYER_IN_GROUP(curr, &g)
	{
		if (curr->pSoldier == &s) return true;
	}
	return false;
}


bool GroupHasInTransitDeadOrPOWMercs(GROUP const& g)
{
	CFOR_EACH_PLAYER_IN_GROUP(i, &g)
	{
		if (!i->pSoldier) continue;
		switch (i->pSoldier->bAssignment)
		{
			case IN_TRANSIT:
			case ASSIGNMENT_POW:
			case ASSIGNMENT_DEAD:
				return true;
		}
	}
	return false;
}

