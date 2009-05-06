#include <stdexcept>

#include "LoadSaveData.h"
#include "Map_Screen_Interface_Bottom.h"
#include "MessageBoxScreen.h"
#include "Strategic_Movement.h"
#include "MemMan.h"
#include "Debug.h"
#include "Campaign_Types.h"
#include "Game_Event_Hook.h"
#include "Game_Clock.h"
#include "Soldier_Control.h"
#include "Queen_Command.h"
#include "Overhead.h"
#include "StrategicMap.h"
#include "JAScreens.h"
#include "Strategic_Pathing.h"
#include "Map_Screen_Interface_Map.h"
#include "PreBattle_Interface.h"
#include "Strategic.h"
#include "Assignments.h"
#include "Text.h"
#include "Font_Control.h"
#include "Message.h"
#include "MapScreen.h"
#include "Dialogue_Control.h"
#include "Soldier_Add.h"
#include "Game_Events.h"
#include "Vehicles.h"
#include "Map_Screen_Helicopter.h"
#include "Map_Screen_Interface.h"
#include "Squads.h"
#include "Random.h"
#include "Soldier_Macros.h"
#include "Map_Information.h"
#include "Tactical_Save.h"
#include "Player_Command.h"
#include "Strategic_AI.h"
#include "Town_Militia.h"
#include "Music_Control.h"
#include "Campaign.h"
#include "Isometric_Utils.h"
#include "Meanwhile.h"
#include "Inventory_Choosing.h"
#include "Map_Screen_Interface_Border.h"
#include "Auto_Resolve.h"
#include "GameSettings.h"
#include "Quests.h"
#include "Video.h"
#include "ScreenIDs.h"
#include "FileMan.h"
#include "Items.h"


// the delay for a group about to arrive
#define ABOUT_TO_ARRIVE_DELAY 5

GROUP *gpGroupList;

static GROUP* gpPendingSimultaneousGroup = NULL;

#ifdef JA2BETAVERSION
	extern BOOLEAN gfExitViewer;
static void ValidateGroups(GROUP const*);
#endif

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
	GROUP* const pNew = MALLOCZ(GROUP);
	pNew->pPlayerList = NULL;
	pNew->pWaypoints = NULL;
	pNew->ubSectorX = pNew->ubNextX = ubSectorX;
	pNew->ubSectorY = pNew->ubNextY = ubSectorY;
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
	GROUP* const pNew = MALLOCZ(GROUP);
	pNew->pWaypoints = NULL;
	pNew->ubSectorX = pNew->ubNextX = ubSectorX;
	pNew->ubSectorY = pNew->ubNextY = ubSectorY;
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


void AddPlayerToGroup(GROUP* const g, SOLDIERTYPE* const s)
{
	Assert(g);
	AssertMsg(g->fPlayer, "Attempting AddPlayerToGroup() on an ENEMY group!");

	PLAYERGROUP* const p = MALLOC(PLAYERGROUP);
	p->pSoldier = s;
	p->next     = NULL;

	s->ubGroupID = g->ubGroupID;

	PLAYERGROUP* i = g->pPlayerList;
	if (!i)
	{
		g->pPlayerList = p;
		g->ubGroupSize = 1;
		g->ubPrevX     = s->ubPrevSectorID % 16 + 1;
		g->ubPrevY     = s->ubPrevSectorID / 16 + 1;
		g->ubSectorX   = s->sSectorX;
		g->ubSectorY   = s->sSectorY;
		g->ubSectorZ   = s->bSectorZ;
	}
	else
	{
		for (; i->next; i = i->next)
		{
			AssertMsg(i->pSoldier->ubProfile != s->ubProfile, String("Attempting to add an already existing merc to group (ubProfile=%d).", s->ubProfile));
		}
		i->next = p;

		g->ubGroupSize++;
	}
}


static void CancelEmptyPersistentGroupMovement(GROUP* pGroup);


void RemovePlayerFromPGroup(GROUP* const g, SOLDIERTYPE* const s)
{
	AssertMsg(g->fPlayer, "Attempting RemovePlayerFromGroup() on an ENEMY group!");

	for (PLAYERGROUP** i = &g->pPlayerList; *i != NULL; i = &(*i)->next)
	{
		PLAYERGROUP* const p = *i;
		if (p->pSoldier != s) continue;

		*i = p->next;
		MemFree(p);

		s->ubPrevSectorID = SECTOR(g->ubPrevX, g->ubPrevY);
		s->ubGroupID      = 0;

		if (--g->ubGroupSize == 0)
		{
			if (!g->fPersistant)
			{
				RemovePGroup(g);
			}
			else
			{
				CancelEmptyPersistentGroupMovement(g);
			}
		}
		break;
	}
}


void RemovePlayerFromGroup(SOLDIERTYPE* const s)
{
	GROUP* const pGroup = GetGroup(s->ubGroupID);

	//KM : August 6, 1999 Patch fix
	//     Because the release build has no assertions, it was still possible for the group to be null,
	//     causing a crash.  Instead of crashing, it'll simply return false.
	if (!pGroup) return;
	//end

	AssertMsg(pGroup, String("Attempting to RemovePlayerFromGroup( %d, %d ) from non-existant group", s->ubGroupID, s->ubProfile));

	RemovePlayerFromPGroup(pGroup, s);
}


static void SetLocationOfAllPlayerSoldiersInGroup(GROUP const*, INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);


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
	pGroup->ubPrevX = pGroup->ubNextX;
	pGroup->ubPrevY = pGroup->ubNextY;
	pGroup->ubNextX = pGroup->ubSectorX;
	pGroup->ubNextY = pGroup->ubSectorY;
	pGroup->ubSectorX = pGroup->ubPrevX;
	pGroup->ubSectorY = pGroup->ubPrevY;

	if( pGroup->fPlayer )
	{
		// ARM: because we've changed the group's ubSectoryX and ubSectorY, we must now also go and change the sSectorX and
		// sSectorY of all the soldiers in this group so that they stay in synch.  Otherwise pathing and movement problems
		// will result since the group is in one place while the merc is in another...
		SetLocationOfAllPlayerSoldiersInGroup( pGroup, pGroup->ubSectorX, pGroup->ubSectorY, 0 );
	}


	// IMPORTANT: The traverse time doesn't change just because we reverse directions!  It takes the same time no matter
	// which direction you're going in!  This becomes critical in case the player reverse directions again before moving!

	// The time it takes to arrive there will be exactly the amount of time we have been moving away from it.
	SetGroupArrivalTime( pGroup, pGroup->uiTraverseTime - pGroup->uiArrivalTime + GetWorldTotalMin() * 2 );

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
			GroupArrivedAtSector(pGroup, TRUE, TRUE);
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
	currDX = pGroup->ubNextX - pGroup->ubSectorX;
	currDY = pGroup->ubNextY - pGroup->ubSectorY;

	//Determine the direction the group would need to travel in to reach the given sector
	newDX = ubSectorX - pGroup->ubSectorX;
	newDY = ubSectorY - pGroup->ubSectorY;

	// clip the new dx/dy values to +/- 1
	if( newDX )
	{
		ubNumUnalignedAxes++;
		newDX /= abs( newDX );
	}
	if( newDY )
	{
		ubNumUnalignedAxes++;
		newDY /= abs( newDY );
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
				AddSectorToFrontOfMercPathForAllSoldiersInGroup(g, g->ubSectorX, g->ubSectorY);
			}

			/* Very special case that requiring specific coding. Check out the
			 * comments at the above function for more information. */
			reversing_direction = TRUE;
			// ARM:  Kris - new rulez.  Must still fall through and add a waypoint anyway!!!
		}
		else
		{ // No waypoints, so compare against the current location.
			if (g->ubSectorX == x) ++n_aligned_axes;
			if (g->ubSectorY == y) ++n_aligned_axes;
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

	WAYPOINT* const new_wp = MALLOC(WAYPOINT);
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
		CFOR_ALL_PLAYERS_IN_GROUP(curr, g)
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
	AssertMsg( uiSector >= 0 && uiSector <= 255, String( "CreateNewEnemyGroup with out of range value of %d", uiSector ) );
	GROUP* const pNew = MALLOCZ(GROUP);
	pNew->pEnemyGroup = MALLOCZ(ENEMYGROUP);
	pNew->pWaypoints = NULL;
	pNew->ubSectorX = (UINT8)SECTORX( uiSector );
	pNew->ubSectorY = (UINT8)SECTORY( uiSector );
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

#ifdef JA2BETAVERSION
	{
		wchar_t str[512];
		if( PlayerMercsInSector( pNew->ubSectorX, pNew->ubSectorY, 0 ) || CountAllMilitiaInSector( pNew->ubSectorX, pNew->ubSectorY ) )
		{
			swprintf(str, lengthof(str), L"Attempting to send enemy troops from player occupied location.  "
										 L"Please ALT+TAB out of the game before doing anything else and send 'Strategic Decisions.txt' "
										 L"and this message.  You'll likely need to revert to a previous save.  If you can reproduce this "
										 L"with a save close to this event, that would really help me! -- KM:0" );
			DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
		}
		else if( pNew->ubGroupSize > 25 )
		{
			swprintf(str, lengthof(str), L"Strategic AI warning:  Creating an enemy group containing %d soldiers "
										 L"(%d admins, %d troops, %d elites) in sector %c%d.  This message is a temporary test message "
										 L"to evaluate a potential problems with very large enemy groups.",
										 pNew->ubGroupSize, ubNumAdmins, ubNumTroops, ubNumElites,
										 pNew->ubSectorY + 'A' - 1, pNew->ubSectorX );
			DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
		}
	}
#endif

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

		MemFree(g);
		return;
	}
	AssertMsg(0, "Trying to remove a strategic group that isn't in the list!");
}


GROUP* GetGroup( UINT8 ubGroupID )
{
	FOR_ALL_GROUPS(curr)
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
			ExecuteCharacterDialogue(s.ubProfile, QUOTE_ENEMY_PRESENCE, s.face, DIALOGUE_TACTICAL_UI, TRUE);

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
	UINT8				ubNumMercs = 0;

	if( fDisableMapInterfaceDueToBattle )
	{
		AssertMsg( 0, "fDisableMapInterfaceDueToBattle is set before attempting to bring up PBI.  Please send PRIOR save if possible and details on anything that just happened before this battle." );
		return;
	}

	// Pipe up with quote...
	AssertMsg( pPlayerDialogGroup, "Didn't get a player dialog group for prebattle interface." );

	AssertMsg(pPlayerDialogGroup->pPlayerList, String( "Player group %d doesn't have *any* players in it!  (Finding dialog group)", pPlayerDialogGroup->ubGroupID));

	SOLDIERTYPE* mercs_in_group[20];
	CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pPlayerDialogGroup)
	{
		SOLDIERTYPE* const pSoldier = pPlayer->pSoldier;

		if ( pSoldier->bLife >= OKLIFE && !( pSoldier->uiStatusFlags & SOLDIER_VEHICLE ) &&
					!AM_A_ROBOT( pSoldier ) && !AM_AN_EPC( pSoldier ) )
		{
			mercs_in_group[ubNumMercs++] = pSoldier;
		}
	}

	//Set music
	SetMusicMode( MUSIC_TACTICAL_ENEMYPRESENT );

	if( gfTacticalTraversal && pInitiatingBattleGroup == gpTacticalTraversalGroup ||
			pInitiatingBattleGroup && !pInitiatingBattleGroup->fPlayer &&
			pInitiatingBattleGroup->ubSectorX == gWorldSectorX &&
		  pInitiatingBattleGroup->ubSectorY == gWorldSectorY && !gbWorldSectorZ )
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
			LockPauseState(LOCK_PAUSE_11);

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
		LockPauseState(LOCK_PAUSE_12);

		// disable exit from mapscreen and what not until face done talking
		fDisableMapInterfaceDueToBattle = TRUE;
	}
	else
	{
		// ATE: What if we have unconscious guys, etc....
		// We MUST start combat, but donot play quote...
		InitPreBattleInterface( pInitiatingBattleGroup, TRUE );
	}
}


static void HandleOtherGroupsArrivingSimultaneously(UINT8 ubSectorX, UINT8 ubSectorY, UINT8 ubSectorZ);
static void NotifyPlayerOfBloodcatBattle(UINT8 ubSectorX, UINT8 ubSectorY);
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

	if( gfWorldLoaded )
	{ //look for people arriving in the currently loaded sector.  This handles reinforcements.
		const GROUP* const curr = FindPlayerMovementGroupInSector(gWorldSectorX, gWorldSectorY);
		if( !gbWorldSectorZ && PlayerMercsInSector( (UINT8)gWorldSectorX, (UINT8)gWorldSectorY, gbWorldSectorZ ) &&
				pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY &&
				curr )
		{ //Reinforcements have arrived!
			#ifdef JA2BETAVERSION
			if( guiCurrentScreen == AIVIEWER_SCREEN )
			{
				gfExitViewer = TRUE;
			}
			#endif
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

	HandleOtherGroupsArrivingSimultaneously( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ );

	FOR_ALL_PLAYER_GROUPS(curr)
	{
		if (curr->ubGroupSize)
		{
			if( !curr->fBetweenSectors )
			{
				if( curr->ubSectorX == pGroup->ubSectorX && curr->ubSectorY == pGroup->ubSectorY && !curr->ubSectorZ )
				{
					if( !GroupHasInTransitDeadOrPOWMercs( curr ) &&
							(!IsGroupTheHelicopterGroup( curr ) || !fHelicopterIsAirBorne) &&
							(!curr->fVehicle || DoesVehicleGroupHaveAnyPassengers(curr)))
					{
						//Now, a player group is in this sector.  Determine if the group contains any mercs that can fight.
						//Vehicles, EPCs and the robot doesn't count.  Mercs below OKLIFE do.
						CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, curr)
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
							pPlayerDialogGroup = curr;
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

		if( NumEnemiesInSector( pGroup->ubSectorX, pGroup->ubSectorY ) )
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
		if( CountAllMilitiaInSector( pGroup->ubSectorX, pGroup->ubSectorY ) )
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
		#ifdef JA2BETAVERSION
			ValidateGroups( pGroup );
		#endif

		if( gubNumGroupsArrivedSimultaneously )
		{ //Because this is a battle case, clear all the group flags
			FOR_ALL_GROUPS(curr)
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
			NotifyPlayerOfBloodcatBattle( pGroup->ubSectorX, pGroup->ubSectorY );
			return TRUE;
		}

		if( !fCombatAbleMerc )
		{ //Prepare for instant autoresolve.
			gfDelayAutoResolveStart = TRUE;
			gfUsePersistantPBI = TRUE;
			if( fMilitiaPresent )
			{
				NotifyPlayerOfInvasionByEnemyForces( pGroup->ubSectorX, pGroup->ubSectorY, 0, TriggerPrebattleInterface );
			}
			else
			{
				wchar_t str[ 256 ];
				wchar_t pSectorStr[ 128 ];
				GetSectorIDString( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ , pSectorStr, lengthof(pSectorStr), TRUE );
				swprintf( str, lengthof(str), gpStrategicString[ STR_DIALOG_ENEMIES_ATTACK_UNCONCIOUSMERCS ], pSectorStr );
				DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, TriggerPrebattleInterface );
			}
		}

		#ifdef JA2BETAVERSION
			if( guiCurrentScreen == AIVIEWER_SCREEN )
				gfExitViewer = TRUE;
		#endif

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
			InitPreBattleInterface(init_prebattle_group_, TRUE);
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


static void DeployGroupToSector(GROUP* pGroup)
{
	Assert( pGroup );
	if( pGroup->fPlayer )
	{
		//Update the sector positions of the players...
		return;
	}
	//Assuming enemy code from here on...
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
	if( pGroup->ubSectorX == wp->x && pGroup->ubSectorY == wp->y )
	{ //We have reached the next waypoint, so now determine what the next waypoint is.
		switch( pGroup->ubMoveType )
		{
			case ONE_WAY:
				if( !wp->next )
				{ //No more waypoints, so we've reached the destination.
					DeployGroupToSector( pGroup );
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


static void AwardExperienceForTravelling(GROUP* pGroup)
{
	// based on how long movement took, mercs gain a bit of life experience for travelling
	SOLDIERTYPE	*	pSoldier;
	UINT32				uiPoints;
	UINT32				uiCarriedPercent;

	if ( !pGroup || !pGroup->fPlayer )
	{
		return;
	}

	CFOR_ALL_PLAYERS_IN_GROUP(pPlayerGroup, pGroup)
	{
		pSoldier = pPlayerGroup->pSoldier;
		if( pSoldier  && !AM_A_ROBOT( pSoldier ) &&
				!AM_AN_EPC( pSoldier ) && !(pSoldier->uiStatusFlags & SOLDIER_VEHICLE) )
		{
			if ( pSoldier->bLifeMax < 100 )
			{
				// award exp...
				// amount was originally based on getting 100-bLifeMax points for 12 hours of travel (720)
				// but changed to flat rate since StatChange makes roll vs 100-lifemax as well!
				uiPoints = pGroup->uiTraverseTime / (450 / 100 - pSoldier->bLifeMax );
				if ( uiPoints > 0 )
				{
					StatChange( pSoldier, HEALTHAMT, (UINT8) uiPoints, FALSE );
				}
			}

			if ( pSoldier->bStrength < 100 )
			{
				uiCarriedPercent = CalculateCarriedWeight( pSoldier );
				if ( uiCarriedPercent > 50 )
				{
					uiPoints = pGroup->uiTraverseTime / (450 / (100 - pSoldier->bStrength ) );
					StatChange( pSoldier, STRAMT, ( UINT16 ) ( uiPoints * ( uiCarriedPercent - 50) / 100 ), FALSE );
				}
			}
		}
	}
}


static void AddCorpsesToBloodcatLair(INT16 sSectorX, INT16 sSectorY)
{
  ROTTING_CORPSE_DEFINITION		Corpse;
	memset( &Corpse, 0, sizeof( ROTTING_CORPSE_DEFINITION ) );

	// Setup some values!
	Corpse.ubBodyType							= REGMALE;
	Corpse.sHeightAdjustment			= 0;
	Corpse.bVisible								=	TRUE;

	SET_PALETTEREP_ID ( Corpse.HeadPal,		"BROWNHEAD" );
	SET_PALETTEREP_ID ( Corpse.VestPal,		"YELLOWVEST" );
	SET_PALETTEREP_ID ( Corpse.SkinPal,		"PINKSKIN" );
	SET_PALETTEREP_ID ( Corpse.PantsPal,  "GREENPANTS" );


	Corpse.bDirection	= (INT8)Random(8);

	// Set time of death
  // Make sure they will be rotting!
	Corpse.uiTimeOfDeath = GetWorldTotalMin( ) - ( 2 * NUM_SEC_IN_DAY / 60 );
	// Set type
	Corpse.ubType	= (UINT8)SMERC_JFK;
	Corpse.usFlags = ROTTING_CORPSE_FIND_SWEETSPOT_FROM_GRIDNO;

  // 1st gridno
	Corpse.sGridNo								= 14319;
	//Add the rotting corpse info to the sectors unloaded rotting corpse file
	AddRottingCorpseToUnloadedSectorsRottingCorpseFile( sSectorX, sSectorY, 0, &Corpse);

  // 2nd gridno
	Corpse.sGridNo								= 9835;
	//Add the rotting corpse info to the sectors unloaded rotting corpse file
	AddRottingCorpseToUnloadedSectorsRottingCorpseFile( sSectorX, sSectorY, 0, &Corpse);


  // 3rd gridno
	Corpse.sGridNo								= 11262;
	//Add the rotting corpse info to the sectors unloaded rotting corpse file
	AddRottingCorpseToUnloadedSectorsRottingCorpseFile( sSectorX, sSectorY, 0, &Corpse);
}


static void HandleNonCombatGroupArrival(GROUP* pGroup, BOOLEAN fMainGroup, BOOLEAN fNeverLeft);
static void ReportVehicleOutOfGas(const VEHICLETYPE*, UINT8 ubSectorX, UINT8 ubSectorY);
static BOOLEAN SpendVehicleFuel(SOLDIERTYPE* pSoldier, INT16 sFuelSpent);
static INT16 VehicleFuelRemaining(SOLDIERTYPE* pSoldier);


//ARRIVALCALLBACK
//...............
//This is called whenever any group arrives in the next sector (player or enemy)
//This function will first check to see if a battle should start, or if they
//aren't at the final destination, they will move to the next sector.
void GroupArrivedAtSector(GROUP* const pGroup, BOOLEAN const fCheckForBattle, BOOLEAN const fNeverLeft)
{
	UINT8 ubInsertionDirection, ubStrategicInsertionCode;
	BOOLEAN fExceptionQueue = FALSE;
	BOOLEAN fFirstTimeInSector = FALSE;
	BOOLEAN fGroupDestroyed = FALSE;

	// reset
	gfWaitingForInput = FALSE;

	if( pGroup->fPlayer )
	{
		//Set the fact we have visited the  sector
		const PLAYERGROUP* curr = pGroup->pPlayerList;
		if( curr )
		{
			if( curr->pSoldier->bAssignment < ON_DUTY )
			{
				ResetDeadSquadMemberList( curr->pSoldier->bAssignment );
			}
		}

		if( pGroup->fVehicle )
		{
			VEHICLETYPE const* const v = GetVehicleFromMvtGroup(pGroup);
			if (VEHICLE2ID(v) != iHelicopterVehicleId &&
					pGroup->pPlayerList == NULL)
			{
				// nobody here, better just get out now
				// with vehicles, arriving empty is probably ok, since passengers might have been killed but vehicle lived.
				return;
			}
		}
		else
		{
			if( pGroup->pPlayerList == NULL )
			{
				// nobody here, better just get out now
				AssertMsg(0, String("Player group %d arrived in sector empty.  KM 0", pGroup->ubGroupID));
				return;
			}
		}
	}
	//Check for exception cases which
	if( gTacticalStatus.bBoxingState != NOT_BOXING )
	{
		if( !pGroup->fPlayer && pGroup->ubNextX == 5 && pGroup->ubNextY == 4 && pGroup->ubSectorZ == 0 )
		{
			fExceptionQueue = TRUE;
		}
	}
	//First check if the group arriving is going to queue another battle.
	//NOTE:  We can't have more than one battle ongoing at a time.
	if( fExceptionQueue || fCheckForBattle && gTacticalStatus.fEnemyInSector &&
			FindPlayerMovementGroupInSector(gWorldSectorX, gWorldSectorY) &&
		  (pGroup->ubNextX != gWorldSectorX || pGroup->ubNextY != gWorldSectorY || gbWorldSectorZ > 0 ) ||
			AreInMeanwhile() ||
			//KM : Aug 11, 1999 -- Patch fix:  Added additional checks to prevent a 2nd battle in the case
			//     where the player is involved in a potential battle with bloodcats/civilians
			fCheckForBattle && HostileCiviliansPresent() ||
			fCheckForBattle && HostileBloodcatsPresent()
		)
	{
		//QUEUE BATTLE!
		//Delay arrival by a random value ranging from 3-5 minutes, so it doesn't get the player
		//too suspicious after it happens to him a few times, which, by the way, is a rare occurrence.
		if( AreInMeanwhile() )
		{
			pGroup->uiArrivalTime ++; //tack on only 1 minute if we are in a meanwhile scene.  This effectively
			                          //prevents any battle from occurring while inside a meanwhile scene.
		}
		else
		{
			pGroup->uiArrivalTime += Random(3) + 3;
		}


		if( !AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID ) )
			AssertMsg( 0, "Failed to add movement event." );

		if( pGroup->fPlayer )
		{
			if( pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin( ) )
			{
				AddStrategicEvent( EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID );
			}
		}

		return;
	}


	//Update the position of the group
	pGroup->ubPrevX = pGroup->ubSectorX;
	pGroup->ubPrevY = pGroup->ubSectorY;
	pGroup->ubSectorX = pGroup->ubNextX;
	pGroup->ubSectorY = pGroup->ubNextY;
	pGroup->ubNextX = 0;
	pGroup->ubNextY = 0;


	if( pGroup->fPlayer )
	{
		// award life 'experience' for travelling, based on travel time!
		if ( !pGroup->fVehicle )
		{
			// gotta be walking to get tougher
			AwardExperienceForTravelling( pGroup );
		}
		else if( !IsGroupTheHelicopterGroup( pGroup ) )
		{
			VEHICLETYPE const* const v  = GetVehicleFromMvtGroup(pGroup);
			SOLDIERTYPE*       const vs = GetSoldierStructureForVehicle(v);

			SpendVehicleFuel(vs, pGroup->uiTraverseTime * 6);

			if (!VehicleFuelRemaining(vs))
			{
				ReportVehicleOutOfGas(v, pGroup->ubSectorX, pGroup->ubSectorY);
				//Nuke the group's path, so they don't continue moving.
				ClearMercPathsAndWaypointsForAllInGroup( pGroup );
			}
		}
	}

	pGroup->uiTraverseTime = 0;
	SetGroupArrivalTime( pGroup, 0 );
	pGroup->fBetweenSectors = FALSE;

	fMapPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;


	// if a player group
	if( pGroup->fPlayer )
	{
		// if this is the last sector along player group's movement path (no more waypoints)
		if ( GroupAtFinalDestination( pGroup ) )
		{
			// clear their strategic movement (mercpaths and waypoints)
			ClearMercPathsAndWaypointsForAllInGroup( pGroup );
		}

		// if on surface
		if( pGroup->ubSectorZ == 0 )
		{
			// check for discovering secret locations
			INT8 bTownId = GetTownIdForSector( pGroup->ubSectorX, pGroup->ubSectorY );

			if( bTownId == TIXA )
				SetTixaAsFound();
			else if( bTownId == ORTA )
				SetOrtaAsFound();
			else if( IsThisSectorASAMSector( pGroup->ubSectorX, pGroup->ubSectorY, 0 ) )
				SetSAMSiteAsFound( GetSAMIdFromSector( pGroup->ubSectorX, pGroup->ubSectorY, 0 ) );
		}


		if( pGroup->ubSectorX < pGroup->ubPrevX )
		{
			ubInsertionDirection = SOUTHWEST;
			ubStrategicInsertionCode = INSERTION_CODE_EAST;
		}
		else if( pGroup->ubSectorX > pGroup->ubPrevX )
		{
			ubInsertionDirection = NORTHEAST;
			ubStrategicInsertionCode = INSERTION_CODE_WEST;
		}
		else if( pGroup->ubSectorY < pGroup->ubPrevY )
		{
			ubInsertionDirection = NORTHWEST;
			ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
		}
		else if( pGroup->ubSectorY > pGroup->ubPrevY )
		{
			ubInsertionDirection = SOUTHEAST;
			ubStrategicInsertionCode = INSERTION_CODE_NORTH;
		}
		else
		{
			Assert(0);
			return;
		}

		if (!pGroup->fVehicle)
		{
			// non-vehicle player group
			CFOR_ALL_PLAYERS_IN_GROUP(curr, pGroup)
			{
				curr->pSoldier->fBetweenSectors = FALSE;
				curr->pSoldier->sSectorX = pGroup->ubSectorX;
				curr->pSoldier->sSectorY = pGroup->ubSectorY;
				curr->pSoldier->bSectorZ = pGroup->ubSectorZ;
				curr->pSoldier->ubPrevSectorID = (UINT8)SECTOR( pGroup->ubPrevX, pGroup->ubPrevY );
				curr->pSoldier->ubInsertionDirection = ubInsertionDirection;

				// don't override if a tactical traversal
				if( curr->pSoldier->ubStrategicInsertionCode != INSERTION_CODE_PRIMARY_EDGEINDEX &&
						curr->pSoldier->ubStrategicInsertionCode != INSERTION_CODE_SECONDARY_EDGEINDEX )
				{
					curr->pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;
				}

				if( curr->pSoldier->pMercPath )
				{
					// remove head from their mapscreen path list
					curr->pSoldier->pMercPath = RemoveHeadFromStrategicPath( curr->pSoldier->pMercPath );
				}

				// ATE: Alrighty, check if this sector is currently loaded, if so,
				// add them to the tactical engine!
				if ( pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY && pGroup->ubSectorZ == gbWorldSectorZ )
				{
					UpdateMercInSector( curr->pSoldier, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
				}
			}

			// if there's anybody in the group
			if( pGroup->pPlayerList )
			{
				// don't print any messages when arriving underground (there's no delay involved) or if we never left (cancel)
				if ( GroupAtFinalDestination( pGroup ) && ( pGroup->ubSectorZ == 0 ) && !fNeverLeft )
				{
					// if assigned to a squad
					if( pGroup->pPlayerList->pSoldier->bAssignment < ON_DUTY )
					{
						// squad
						ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pMessageStrings[ MSG_ARRIVE ], pAssignmentStrings[ pGroup->pPlayerList->pSoldier->bAssignment ], pMapVertIndex[ pGroup->pPlayerList->pSoldier->sSectorY ], pMapHortIndex[ pGroup->pPlayerList->pSoldier->sSectorX ]);
					}
					else
					{
						// a loner
						ScreenMsg( FONT_MCOLOR_DKRED, MSG_INTERFACE, pMessageStrings[ MSG_ARRIVE ], pGroup->pPlayerList->pSoldier->name, pMapVertIndex[ pGroup->pPlayerList->pSoldier->sSectorY  ], pMapHortIndex[ pGroup->pPlayerList->pSoldier->sSectorX  ] );
					}
				}
			}
		}
		else	// vehicle player group
		{
			VEHICLETYPE* const v = GetVehicleFromMvtGroup(pGroup);
			if (v->pMercPath  )
			{
				// remove head from vehicle's mapscreen path list
				v->pMercPath = RemoveHeadFromStrategicPath(v->pMercPath);
			}

			// update vehicle position
			SetVehicleSectorValues(v, pGroup->ubSectorX, pGroup->ubSectorY);
			v->fBetweenSectors = FALSE;

			// update passengers position
			UpdatePositionOfMercsInVehicle(v);

			if (VEHICLE2ID(v) != iHelicopterVehicleId)
			{
				SOLDIERTYPE* const pSoldier = GetSoldierStructureForVehicle(v);

				pSoldier->fBetweenSectors = FALSE;
				pSoldier->sSectorX = pGroup->ubSectorX;
				pSoldier->sSectorY = pGroup->ubSectorY;
				pSoldier->bSectorZ = pGroup->ubSectorZ;
				pSoldier->ubInsertionDirection = ubInsertionDirection;

				// ATE: Removed, may 21 - sufficient to use insertion direction...
				//pSoldier->bDesiredDirection = ubInsertionDirection;

				pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;

				// if this sector is currently loaded
				if ( pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY && pGroup->ubSectorZ == gbWorldSectorZ )
				{
					// add vehicle to the tactical engine!
					UpdateMercInSector( pSoldier, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
				}

				// set directions of insertion
				CFOR_ALL_PLAYERS_IN_GROUP(curr, pGroup)
				{
					curr->pSoldier->fBetweenSectors = FALSE;
					curr->pSoldier->sSectorX = pGroup->ubSectorX;
					curr->pSoldier->sSectorY = pGroup->ubSectorY;
					curr->pSoldier->bSectorZ = pGroup->ubSectorZ;
					curr->pSoldier->ubInsertionDirection = ubInsertionDirection;

					// ATE: Removed, may 21 - sufficient to use insertion direction...
					// curr->pSoldier->bDesiredDirection = ubInsertionDirection;

					curr->pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;

					// if this sector is currently loaded
					if ( pGroup->ubSectorX == gWorldSectorX && pGroup->ubSectorY == gWorldSectorY && pGroup->ubSectorZ == gbWorldSectorZ )
					{
						// add passenger to the tactical engine!
						UpdateMercInSector( curr->pSoldier, gWorldSectorX, gWorldSectorY, gbWorldSectorZ );
					}
				}
			}
			else
			{
				if (HandleHeliEnteringSector(v->sSectorX, v->sSectorY))
				{
					// helicopter destroyed
					fGroupDestroyed = TRUE;
				}
			}


			if ( !fGroupDestroyed )
			{
				// don't print any messages when arriving underground, there's no delay involved
				if ( GroupAtFinalDestination( pGroup ) && ( pGroup->ubSectorZ == 0 ) && !fNeverLeft )
				{
					ScreenMsg(FONT_MCOLOR_DKRED, MSG_INTERFACE, pMessageStrings[MSG_ARRIVE], pVehicleStrings[v->ubVehicleType], pMapVertIndex[pGroup->ubSectorY], pMapHortIndex[pGroup->ubSectorX]);
				}
			}
		}


		if ( !fGroupDestroyed )
		{
			// check if sector had been visited previously
			fFirstTimeInSector = !GetSectorFlagStatus( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ, SF_ALREADY_VISITED );

			// on foot, or in a vehicle other than the chopper
			if ( !pGroup->fVehicle || !IsGroupTheHelicopterGroup( pGroup ) )
			{

        // ATE: Add a few corpse to the bloodcat lair...
        if ( SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) == SEC_I16 && fFirstTimeInSector )
        {
          AddCorpsesToBloodcatLair( pGroup->ubSectorX, pGroup->ubSectorY );
        }

				// mark the sector as visited already
				SetSectorFlag( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ, SF_ALREADY_VISITED );
			}
		}

		// update character info
		fTeamPanelDirty = TRUE;
		fCharacterInfoPanelDirty = TRUE;
	}

	if ( !fGroupDestroyed )
	{
		//Determine if a battle should start.
		//if a battle does start, or get's delayed, then we will keep the group in memory including
		//all waypoints, until after the battle is resolved.  At that point, we will continue the processing.
		if( fCheckForBattle && !CheckConditionsForBattle( pGroup ) && !gfWaitingForInput )
		{
			HandleNonCombatGroupArrival( pGroup, TRUE, fNeverLeft );

			if( gubNumGroupsArrivedSimultaneously )
			{
				FOR_ALL_GROUPS_SAFE(g)
				{
					if (gubNumGroupsArrivedSimultaneously == 0) break;

					if (g->uiFlags & GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY)
					{
						gubNumGroupsArrivedSimultaneously--;
						HandleNonCombatGroupArrival(g, FALSE, FALSE);
					}
				}
			}
		}
		else
		{ //Handle cases for pre battle conditions
			pGroup->uiFlags = 0;
			if( gubNumAwareBattles )
			{ //When the AI is looking for the players, and a battle is initiated, then
				//decrement the value, otherwise the queen will continue searching to infinity.
				gubNumAwareBattles--;
			}
		}
	}
	gfWaitingForInput = FALSE;
}


static void HandleNonCombatGroupArrival(GROUP* pGroup, BOOLEAN fMainGroup, BOOLEAN fNeverLeft)
{
	// if any mercs are actually in the group

	if( StrategicAILookForAdjacentGroups( pGroup ) )
	{ //The routine actually just deleted the enemy group (player's don't get deleted), so we are done!
		return;
	}

	if( pGroup->fPlayer )
	{
		//The group will always exist after the AI was processed.

		//Determine if the group should rest, change routes, or continue moving.
		// if on foot, or in a vehicle other than the helicopter
		if( !pGroup->fVehicle || !IsGroupTheHelicopterGroup( pGroup ) )
		{
			// take control of sector
			SetThisSectorAsPlayerControlled( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ, FALSE );
		}

		// if this is the last sector along their movement path (no more waypoints)
		if ( GroupAtFinalDestination( pGroup ) )
		{
			// if currently selected sector has nobody in it
			if ( PlayerMercsInSector( ( UINT8 ) sSelMapX, ( UINT8 ) sSelMapY, ( UINT8 ) iCurrentMapSectorZ ) == 0 )
			{
				// make this sector strategically selected
				ChangeSelectedMapSector( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ );
			}

			// if on foot or in a vehicle other than the helicopter (Skyrider speaks for heli movement)
			if ( !pGroup->fVehicle || !IsGroupTheHelicopterGroup( pGroup ) )
			{
				StopTimeCompression();

				// if traversing tactically, or we never left (just canceling), don't do this
				if( !gfTacticalTraversal && !fNeverLeft )
				{
					RandomMercInGroupSaysQuote( pGroup, QUOTE_MERC_REACHED_DESTINATION );
				}
			}
		}
		// look for NPCs to stop for, anyone is too tired to keep going, if all OK rebuild waypoints & continue movement
		// NOTE: Only the main group (first group arriving) will stop for NPCs, it's just too much hassle to stop them all
		PlayerGroupArrivedSafelyInSector( pGroup, fMainGroup );
	}
	else
	{
		if( !pGroup->fDebugGroup )
		{
			CalculateNextMoveIntention( pGroup );
		}
		else
		{
			RemovePGroup( pGroup );
		}
	}
	//Clear the non-persistant flags.
	pGroup->uiFlags = 0;
}



//Because a battle is about to start, we need to go through the event list and look for other
//groups that may arrive at the same time -- enemies or players, and blindly add them to the sector
//without checking for battle conditions, as it has already determined that a new battle is about to
//start.
static void HandleOtherGroupsArrivingSimultaneously(UINT8 ubSectorX, UINT8 ubSectorY, UINT8 ubSectorZ)
{
	STRATEGICEVENT *pEvent;
	UINT32 uiCurrTimeStamp;
	GROUP *pGroup;
	uiCurrTimeStamp = GetWorldTotalSeconds();
	pEvent = gpEventList;
	gubNumGroupsArrivedSimultaneously = 0;
	while( pEvent && pEvent->uiTimeStamp <= uiCurrTimeStamp )
	{
		if( pEvent->ubCallbackID == EVENT_GROUP_ARRIVAL && !(pEvent->ubFlags & SEF_DELETION_PENDING) )
		{
			pGroup = GetGroup( (UINT8)pEvent->uiParam );
			Assert( pGroup );
			if( pGroup->ubNextX == ubSectorX && pGroup->ubNextY == ubSectorY && pGroup->ubSectorZ == ubSectorZ )
			{
				if( pGroup->fBetweenSectors )
				{
					GroupArrivedAtSector(pGroup, FALSE, FALSE );
					pGroup->uiFlags |= GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY;
					gubNumGroupsArrivedSimultaneously++;
					DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );
					pEvent = gpEventList;
					continue;
				}
			}
		}
		pEvent = pEvent->next;
	}
}


static void DelayEnemyGroupsIfPathsCross(GROUP* pPlayerGroup);


//The user has just approved to plan a simultaneous arrival.  So we will syncronize all of the involved
//groups so that they arrive at the same time (which is the time the final group would arrive).
static void PrepareGroupsForSimultaneousArrival(void)
{
	UINT32 uiLatestArrivalTime = 0;

	FOR_ALL_PLAYER_GROUPS(pGroup)
	{ //For all of the groups that haven't arrived yet, determine which one is going to take the longest.
		if( pGroup != gpPendingSimultaneousGroup
				&& pGroup->fBetweenSectors
				&& pGroup->ubNextX == gpPendingSimultaneousGroup->ubSectorX
				&& pGroup->ubNextY == gpPendingSimultaneousGroup->ubSectorY &&
				!IsGroupTheHelicopterGroup( pGroup ) )
		{
			uiLatestArrivalTime = MAX( pGroup->uiArrivalTime, uiLatestArrivalTime );
			pGroup->uiFlags |= GROUPFLAG_SIMULTANEOUSARRIVAL_APPROVED | GROUPFLAG_MARKER;
		}
	}
	//Now, go through the list again, and reset their arrival event to the latest arrival time.
	FOR_ALL_GROUPS(pGroup)
	{
		if( pGroup->uiFlags & GROUPFLAG_MARKER )
		{
			DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );

			// NOTE: This can cause the arrival time to be > GetWorldTotalMin() + TraverseTime, so keep that in mind
			// if you have any code that uses these 3 values to figure out how far along its route a group is!
			SetGroupArrivalTime( pGroup, uiLatestArrivalTime );
			AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );

			if( pGroup->fPlayer )
			{
				if( pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin( ) )
				{
					AddStrategicEvent( EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID );
				}
			}

			DelayEnemyGroupsIfPathsCross( pGroup );

			pGroup->uiFlags &= ~GROUPFLAG_MARKER;
		}
	}
	//We still have the first group that has arrived.  Because they are set up to be in the destination
	//sector, we will "warp" them back to the last sector, and also setup a new arrival time for them.
	GROUP* const pGroup = gpPendingSimultaneousGroup;
	pGroup->ubNextX = pGroup->ubSectorX;
	pGroup->ubNextY = pGroup->ubSectorY;
	pGroup->ubSectorX = pGroup->ubPrevX;
	pGroup->ubSectorY = pGroup->ubPrevY;
	SetGroupArrivalTime( pGroup, uiLatestArrivalTime );
	pGroup->fBetweenSectors = TRUE;

	if( pGroup->fVehicle )
	{
		VEHICLETYPE* const v = GetVehicleFromMvtGroup(pGroup);
		v->fBetweenSectors = TRUE;

		if (VEHICLE2ID(v) != iHelicopterVehicleId)
		{
			SOLDIERTYPE* const vs = GetSoldierStructureForVehicle(v);
			vs->fBetweenSectors = TRUE;
		}
	}

	AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );

	if( pGroup->fPlayer )
	{
		if( pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin( ) )
		{
			AddStrategicEvent( EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID );
		}
	}
	DelayEnemyGroupsIfPathsCross( pGroup );
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
	if (IsGroupTheHelicopterGroup(first_group)) return FALSE;

	/* Count the number of groups that are scheduled to arrive in the same sector
	 * and are currently adjacent to the sector in question. */
	UINT8 n_nearby_groups = 0;
	FOR_ALL_PLAYER_GROUPS(g)
	{
		if (g == first_group)                                   continue;
		if (!g->fBetweenSectors)                                continue;
		if (g->ubNextX != first_group->ubSectorX)               continue;
		if (g->ubNextY != first_group->ubSectorY)               continue;
		if (g->uiFlags & GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED) continue;
		if (IsGroupTheHelicopterGroup(g))                       continue;
		g->uiFlags |= GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED;
		++n_nearby_groups;
	}

	if (n_nearby_groups == 0) return FALSE;

	// Postpone the battle until the user answers the dialog.
	InterruptTime();
	PauseGame();
	LockPauseState(LOCK_PAUSE_13);
	gpPendingSimultaneousGroup = first_group;

	wchar_t const* const pStr =
		n_nearby_groups == 1 ? gpStrategicString[STR_DETECTED_SINGULAR] :
		gpStrategicString[STR_DETECTED_PLURAL];
	wchar_t const* const enemy_type =
		gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE ? gpStrategicString[STR_PB_BLOODCATS] :
		gpStrategicString[STR_PB_ENEMIES];
	/* header, sector, singular/plural str, confirmation string.
	 * Ex:  Enemies have been detected in sector J9 and another squad is about to
	 *      arrive.  Do you wish to coordinate a simultaneous arrival? */
	wchar_t str[255];
	size_t const n = swprintf(str, lengthof(str), pStr, enemy_type, 'A' + first_group->ubSectorY - 1, first_group->ubSectorX);
	swprintf(str + n, lengthof(str) - n, L" %ls", gpStrategicString[STR_COORDINATE]);
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


static void DelayEnemyGroupsIfPathsCross(GROUP* pPlayerGroup)
{
	FOR_ALL_ENEMY_GROUPS(pGroup)
	{
		// Check to see if this group will arrive in next sector before the player group.
		if( pGroup->uiArrivalTime < pPlayerGroup->uiArrivalTime )
		{ //check to see if enemy group will cross paths with player group.
			if( pGroup->ubNextX == pPlayerGroup->ubSectorX &&
					pGroup->ubNextY == pPlayerGroup->ubSectorY &&
					pGroup->ubSectorX == pPlayerGroup->ubNextX &&
					pGroup->ubSectorY == pPlayerGroup->ubNextY )
			{ //Okay, the enemy group will cross paths with the player, so find and delete the arrival event
				//and repost it in the future (like a minute or so after the player arrives)
				DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );

				// NOTE: This can cause the arrival time to be > GetWorldTotalMin() + TraverseTime, so keep that in mind
				// if you have any code that uses these 3 values to figure out how far along its route a group is!
				SetGroupArrivalTime( pGroup, pPlayerGroup->uiArrivalTime + 1 + Random( 10 ) );
				if( !AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID ) )
					AssertMsg( 0, "Failed to add movement event." );
			}
		}
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
	dx = wp->x - pGroup->ubSectorX;
	dy = wp->y - pGroup->ubSectorY;
	if( dx && dy )
	{ //Can't move diagonally!
		AssertMsg( 0, String("Attempting to move to waypoint in a diagonal direction from sector %d,%d to sector %d,%d",
			pGroup->ubSectorX, pGroup->ubSectorY, wp->x, wp->y ) );
	}
	AssertMsg(dx != 0 || dy != 0, String("Attempting to move to waypoint %d, %d that you are already at!", wp->x, wp->y));
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
		Assert( 0 );
		return;
	}
	//All conditions for moving to the next waypoint are now good.
	pGroup->ubNextX = (UINT8)( dx + pGroup->ubSectorX );
	pGroup->ubNextY = (UINT8)( dy + pGroup->ubSectorY );
	//Calc time to get to next waypoint...
	ubSector = (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY );
	if( !pGroup->ubSectorZ )
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

	if( pGroup->uiTraverseTime == 0xffffffff )
	{
		AssertMsg( 0, String("Group %d (%s) attempting illegal move from %c%d to %c%d (%s).",
				pGroup->ubGroupID, ( pGroup->fPlayer ) ? "Player" : "AI",
				pGroup->ubSectorY+'A', pGroup->ubSectorX, pGroup->ubNextY+'A', pGroup->ubNextX,
				gszTerrain[SectorInfo[ubSector].ubTraversability[ubDirection]] ) );
	}

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
		SetGroupArrivalTime( pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime );
	}
	// NOTE: if the group is already between sectors, DON'T MESS WITH ITS ARRIVAL TIME!  THAT'S NOT OUR JOB HERE!!!


	// special override for AI patrol initialization only
	if( gfRandomizingPatrolGroup )
	{ //We're initializing the patrol group, so randomize the enemy groups to have extremely quick and varying
		//arrival times so that their initial positions aren't easily determined.
		pGroup->uiTraverseTime = 1 + Random( pGroup->uiTraverseTime - 1 );
		SetGroupArrivalTime( pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime );
	}


	if (pGroup->fVehicle)
	{
		// vehicle, set fact it is between sectors too
		VEHICLETYPE* const v = GetVehicleFromMvtGroup(pGroup);
		v->fBetweenSectors = TRUE;

		if (VEHICLE2ID(v) != iHelicopterVehicleId)
		{
			SOLDIERTYPE* const vs = GetSoldierStructureForVehicle(v);
			vs->fBetweenSectors = TRUE;
			RemoveSoldierFromTacticalSector(vs);
		}
	}

	//Post the event!
	if( !AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID ) )
		AssertMsg( 0, "Failed to add movement event." );

	//For the case of player groups, we need to update the information of the soldiers.
	if( pGroup->fPlayer )
	{
		if( pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin( ) )
		{
			AddStrategicEvent( EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID );
		}

		CFOR_ALL_PLAYERS_IN_GROUP(curr, pGroup)
		{
			curr->pSoldier->fBetweenSectors = TRUE;

			// OK, Remove the guy from tactical engine!
			RemoveSoldierFromTacticalSector(curr->pSoldier);
		}
		CheckAndHandleUnloadingOfCurrentWorld();

		//If an enemy group will be crossing paths with the player group, delay the enemy group's arrival time so that
		//the player will always encounter that group.
		if( !pGroup->ubSectorZ )
		{
			DelayEnemyGroupsIfPathsCross( pGroup );
		}
	}
}


void RemoveGroupWaypoints(GROUP* const pGroup)
{
	WAYPOINT* wp;
	//if there aren't any waypoints to delete, then return.
	if( !pGroup->pWaypoints )
		return;
	//remove all of the waypoints.
	while( pGroup->pWaypoints )
	{
		wp = pGroup->pWaypoints;
		pGroup->pWaypoints = pGroup->pWaypoints->next;
		MemFree( wp );
	}
	pGroup->ubNextWaypointID = 0;
	pGroup->pWaypoints = NULL;
}


// set this groups previous sector values
static void SetGroupPrevSectors(GROUP* const g, UINT8 const ubX, UINT8 const ubY)
{
	g->ubPrevX = ubX;
	g->ubPrevY = ubY;
}


void RemoveGroup( UINT8 ubGroupID )
{
	GROUP *pGroup;
	pGroup = GetGroup( ubGroupID );

	Assert( pGroup );
	RemovePGroup( pGroup );
}


static BOOLEAN gfRemovingAllGroups = FALSE;


void RemovePGroup(GROUP* const g)
{
	if (g->fPersistant && !gfRemovingAllGroups)
	{
		CancelEmptyPersistentGroupMovement(g);
		return;
		DoScreenIndependantMessageBox(L"Strategic Info Warning:  Attempting to delete a persistant group.", MSG_BOX_FLAG_OK, NULL);
	}

	RemoveGroupWaypoints(g);

	// Remove the arrival event if applicable.
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g->ubGroupID);

	// Determine what type of group we have (because it requires different methods)
	if (g->fPlayer)
	{
		while (g->pPlayerList)
		{
			PLAYERGROUP* const pPlayer = g->pPlayerList;
			g->pPlayerList = g->pPlayerList->next;
			MemFree(pPlayer);
		}
	}
	else
	{
		RemoveGroupFromStrategicAILists(g->ubGroupID);
		MemFree(g->pEnemyGroup);
	}

	RemoveGroupFromList(g);
}


void RemoveAllGroups()
{
	gfRemovingAllGroups = TRUE;
	while( gpGroupList )
	{
		RemovePGroup( gpGroupList );
	}
	gfRemovingAllGroups = FALSE;
}


void SetGroupSectorValue(INT16 const sSectorX, INT16 const sSectorY, INT16 const sSectorZ, GROUP* const g)
{
	RemoveGroupWaypoints(g);

	// set sector x and y to passed values
	g->ubSectorX = g->ubNextX = sSectorX;
	g->ubSectorY = g->ubNextY = sSectorY;
	g->ubSectorZ =              sSectorZ;
	g->fBetweenSectors = FALSE;

	// set next sectors same as current
	g->ubOriginalSector = SECTOR(g->ubSectorX, g->ubSectorY);
	DeleteStrategicEvent(EVENT_GROUP_ARRIVAL, g->ubGroupID);

	// set all of the mercs in the group so that they are in the new sector too.
	CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, g)
	{
		pPlayer->pSoldier->sSectorX = sSectorX;
		pPlayer->pSoldier->sSectorY = sSectorY;
		pPlayer->pSoldier->bSectorZ = (UINT8)sSectorZ;
		pPlayer->pSoldier->fBetweenSectors = FALSE;
	}

	CheckAndHandleUnloadingOfCurrentWorld();
}


void SetEnemyGroupSector( GROUP *pGroup, UINT8 ubSectorID )
{
	// make sure it is valid
	Assert( pGroup );
	DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );

	if (!gfRandomizingPatrolGroup) RemoveGroupWaypoints(pGroup);

	// set sector x and y to passed values
	pGroup->ubSectorX = pGroup->ubNextX = (UINT8)SECTORX( ubSectorID );
	pGroup->ubSectorY = pGroup->ubNextY = (UINT8)SECTORY( ubSectorID );
	pGroup->ubSectorZ = 0;
	pGroup->fBetweenSectors = FALSE;
}


// set groups next sector x,y value..used ONLY for teleporting groups
static void SetGroupNextSectorValue(INT16 const sSectorX, INT16 const sSectorY, GROUP* const g)
{
	RemoveGroupWaypoints(g);

	// set sector x and y to passed values
	g->ubNextX         = sSectorX;
	g->ubNextY         = sSectorY;
	g->fBetweenSectors = FALSE;

	// set next sectors same as current
	g->ubOriginalSector = SECTOR(g->ubSectorX, g->ubSectorY);
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
	iDelta = ( INT32 )( SECTOR( pGroup->ubSectorX, pGroup->ubSectorY ) - SECTOR( pGroup->ubNextX, pGroup->ubNextY ) );

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
		pCurrent.x = pGroup->ubNextX;
		pCurrent.y = pGroup->ubNextY;
	}
	else
	{
		// first waypoint is CURRENT sector
		pCurrent.x = pGroup->ubSectorX;
		pCurrent.y = pGroup->ubSectorY;
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
	INT32	iThisCostInTime;


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

		if( iThisCostInTime == 0xffffffff )
		{
			AssertMsg( 0, String("Group %d (%s) attempting illegal move from sector %d, dir %d (%s).",
					pGroup->ubGroupID, ( pGroup->fPlayer ) ? "Player" : "AI",
					ubCurrentSector, ubDirection,
					gszTerrain[SectorInfo[ubCurrentSector].ubTraversability[ubDirection]] ) );
		}

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

	if (traverse_type == EDGEOFWORLD) return 0xFFFFFFFF; // Can't travel here!

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
			default:       return 0xFFFFFFFF; // Group can't traverse here.
		}
		INT32 const traverse_time = FOOT_TRAVEL_TIME * 100 / traverse_mod;
		if (best_traverse_time > traverse_time)
			best_traverse_time = traverse_time;

		if (g->fPlayer)
		{
			INT32 highest_encumbrance = 100;
			CFOR_ALL_PLAYERS_IN_GROUP(curr, g)
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
			default:   return 0xFFFFFFFF; // Group can't traverse here.
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
			default:     return 0xFFFFFFFF; // Group can't traverse here.
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
			default:       return 0xFFFFFFFF; // Group can't traverse here.
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


// Counts the number of live mercs in any given sector.
UINT8 PlayerMercsInSector(UINT8 const x, UINT8 const y, UINT8 const z)
{
	UINT8 n_mercs = 0;
	CFOR_ALL_PLAYER_GROUPS(g)
	{
		if (g->fBetweenSectors) continue;
		if (g->ubSectorX != x || g->ubSectorY != y || g->ubSectorZ != z) continue;
		/* We have a group, make sure that it isn't a group containing only dead
		 * members. */
		CFOR_ALL_PLAYERS_IN_GROUP(p, g)
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


UINT8 PlayerGroupsInSector(UINT8 const x, UINT8 const y, UINT8 const z)
{
	UINT8 n_groups = 0;
	CFOR_ALL_PLAYER_GROUPS(g)
	{
		if (g->fBetweenSectors) continue;
		if (g->ubSectorX != x || g->ubSectorY != y || g->ubSectorZ != z) continue;
		/* We have a group, make sure that it isn't a group containing only dead
		 * members. */
		CFOR_ALL_PLAYERS_IN_GROUP(p, g)
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


//Add this group to the current battle fray!
//NOTE:  For enemies, only MAX_STRATEGIC_TEAM_SIZE at a time can be in a battle, so
//if it ever gets past that, god help the player, but we'll have to insert them
//as those slots free up.
void HandleArrivalOfReinforcements(GROUP const* const pGroup)
{
	SOLDIERTYPE *pSoldier;
	INT32	cnt;

	if( pGroup->fPlayer )
	{ //We don't have to worry about filling up the player slots, because it is impossible
		//to have more player's in the game then the number of slots available for the player.
		UINT8 ubStrategicInsertionCode;
		//First, determine which entrypoint to use, based on the travel direction of the group.
		if( pGroup->ubSectorX < pGroup->ubPrevX )
			ubStrategicInsertionCode = INSERTION_CODE_EAST;
		else if( pGroup->ubSectorX > pGroup->ubPrevX )
			ubStrategicInsertionCode = INSERTION_CODE_WEST;
		else if( pGroup->ubSectorY < pGroup->ubPrevY )
			ubStrategicInsertionCode = INSERTION_CODE_SOUTH;
		else if( pGroup->ubSectorY > pGroup->ubPrevY )
			ubStrategicInsertionCode = INSERTION_CODE_NORTH;
		else
		{
			Assert( 0 );
			return;
		}

		cnt = 0;
		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
		{
			pSoldier = pPlayer->pSoldier;
			Assert( pSoldier );
			pSoldier->ubStrategicInsertionCode = ubStrategicInsertionCode;
			UpdateMercInSector( pSoldier, pGroup->ubSectorX, pGroup->ubSectorY, 0 );

			// DO arrives quote....
			if ( cnt == 0 )
			{
				TacticalCharacterDialogue( pSoldier, QUOTE_MERC_REACHED_DESTINATION );
			}
			cnt++;
		}
		ScreenMsg( FONT_YELLOW, MSG_INTERFACE, Message[ STR_PLAYER_REINFORCEMENTS ] );

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
	INT16        const sec_battle = bg ? SECTOR(bg->ubSectorX, bg->ubSectorY) : -1;

	/* Get number of characters entering/existing between these two sectors.
	 * Special conditions during pre-battle interface to return where this
	 * function is used to show potential retreating directions instead! */

	CFOR_ALL_PLAYER_GROUPS(curr)
	{
		bool const is_heli_group = IsGroupTheHelicopterGroup(curr);

		/* If this group is aboard the helicopter and we're showing the airspace
		 * layer, don't count any mercs aboard the chopper, because the chopper icon
		 * itself serves the function of showing the location/size of this group. */
		if (is_heli_group && fShowAircraftFlag) continue;

		/* If only showing retreat paths, ignore groups not in the battle sector.
		 * If NOT showing retreat paths, ignore groups not between sectors. */
		if (gfDisplayPotentialRetreatPaths ? sec_battle != sec_src : !curr->fBetweenSectors) continue;

		UINT8 n_mercs = curr->ubGroupSize;
		if (n_mercs == 0) // Skip empty persistent groups
		{
			Assert(curr->fPersistant);
			continue;
		}

		INT16 const sec_prev = SECTOR(curr->ubPrevX,   curr->ubPrevY);
		INT16 const sec_cur  = SECTOR(curr->ubSectorX, curr->ubSectorY);
		INT16 const sec_next = SECTOR(curr->ubNextX,   curr->ubNextY);

		bool const may_retreat_from_battle =
			sec_battle == sec_src && sec_cur == sec_src && sec_prev == sec_dst;

		bool const retreating_from_battle =
			sec_battle == sec_dst && sec_cur == sec_dst && sec_prev == sec_src;

		if (may_retreat_from_battle || (sec_cur == sec_src && sec_next == sec_dst))
		{
			// If it's a valid vehicle, but not the helicopter (which can fly empty)
			if (curr->fVehicle && !is_heli_group)
			{ // subtract 1, we don't wanna count the vehicle itself for purposes of showing a number on the map
				n_mercs--;
			}

			*n_enter += n_mercs;

			if (may_retreat_from_battle ||
					curr->uiArrivalTime - GetWorldTotalMin() <= ABOUT_TO_ARRIVE_DELAY)
			{
				*about_to_arrive_enter = TRUE;
			}
		}
		else if (retreating_from_battle || (sec_cur == sec_dst && sec_next == sec_src))
		{
			// If it's a valid vehicle, but not the helicopter (which can fly empty)
			if (curr->fVehicle && !is_heli_group)
			{ // subtract 1, we don't wanna count the vehicle itself for purposes of showing a number on the map
				n_mercs--;
			}

			*n_exit += n_mercs;
		}
	}

	// if there was actually anyone leaving this sector and entering next
	return *n_enter > 0;
}


void MoveAllGroupsInCurrentSectorToSector(UINT8 const x, UINT8 const y, UINT8 const z)
{
	FOR_ALL_PLAYER_GROUPS(g)
	{
		if (g->ubSectorX != gWorldSectorX)  continue;
		if (g->ubSectorY != gWorldSectorY)  continue;
		if (g->ubSectorZ != gbWorldSectorZ) continue;
		if (g->fBetweenSectors)             continue;

		// This player group is in the currently loaded sector
		g->ubSectorX = x;
		g->ubSectorY = y;
		g->ubSectorZ = z;
		CFOR_ALL_PLAYERS_IN_GROUP(p, g)
		{
			p->pSoldier->sSectorX        = x;
			p->pSoldier->sSectorY        = y;
			p->pSoldier->bSectorZ        = z;
			p->pSoldier->fBetweenSectors = FALSE;
		}
	}
	CheckAndHandleUnloadingOfCurrentWorld();
}


static void SaveEnemyGroupStruct(HWFILE, GROUP const*);
static void SavePlayerGroupList(HWFILE, GROUP const*);
static void SaveWayPointList(HWFILE, GROUP const*);


void SaveStrategicMovementGroupsToSaveGameFile(HWFILE const f)
{
	// Save the number of movement groups to the saved game file
	UINT32 uiNumberOfGroups = 0;
	CFOR_ALL_GROUPS(g) ++uiNumberOfGroups;
	FileWrite(f, &uiNumberOfGroups, sizeof(UINT32));

	CFOR_ALL_GROUPS(g)
	{
		BYTE data[84];
		BYTE* d = data;
		INJ_BOOL(d, g->fDebugGroup)
		INJ_BOOL(d, g->fPlayer)
		INJ_BOOL(d, g->fVehicle)
		INJ_BOOL(d, g->fPersistant)
		INJ_U8(d, g->ubGroupID)
		INJ_U8(d, g->ubGroupSize)
		INJ_U8(d, g->ubSectorX)
		INJ_U8(d, g->ubSectorY)
		INJ_U8(d, g->ubSectorZ)
		INJ_U8(d, g->ubNextX)
		INJ_U8(d, g->ubNextY)
		INJ_U8(d, g->ubPrevX)
		INJ_U8(d, g->ubPrevY)
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
		Assert(d == endof(data));

		FileWrite(f, data, sizeof(data));

		// Save the linked list, for the current type of group
		if (g->fPlayer)
		{
			if (g->ubGroupSize) SavePlayerGroupList(f, g);
		}
		else
		{
			Assert(g->pEnemyGroup);
			SaveEnemyGroupStruct(f, g);
		}

		SaveWayPointList(f, g);
	}

	// Save the unique id mask
	FileWrite(f, uniqueIDMask, sizeof(uniqueIDMask));
}


static void LoadEnemyGroupStructFromSavedGame(HWFILE, GROUP*);
static void LoadPlayerGroupList(HWFILE, GROUP*);
static void LoadWayPointList(HWFILE, GROUP*);


void LoadStrategicMovementGroupsFromSavedGameFile(HWFILE const f)
{
	Assert(gpGroupList == NULL);

	// Load the number of nodes in the list
	UINT32 uiNumberOfGroups;
	FileRead(f, &uiNumberOfGroups, sizeof(UINT32));

	//loop through all the nodes and add them to the LL
	GROUP** anchor = &gpGroupList;
	for (UINT32 i = uiNumberOfGroups; i != 0; --i)
	{
		GROUP* const g = MALLOCZ(GROUP);

		BYTE data[84];
		FileRead(f, data, sizeof(data));

		BYTE const* d = data;
		EXTR_BOOL(d, g->fDebugGroup)
		EXTR_BOOL(d, g->fPlayer)
		EXTR_BOOL(d, g->fVehicle)
		EXTR_BOOL(d, g->fPersistant)
		EXTR_U8(d, g->ubGroupID)
		EXTR_U8(d, g->ubGroupSize)
		EXTR_U8(d, g->ubSectorX)
		EXTR_U8(d, g->ubSectorY)
		EXTR_U8(d, g->ubSectorZ)
		EXTR_U8(d, g->ubNextX)
		EXTR_U8(d, g->ubNextY)
		EXTR_U8(d, g->ubPrevX)
		EXTR_U8(d, g->ubPrevY)
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
		Assert(d == endof(data));

		if (g->fPlayer)
		{
			// If there is a player list, add it
			if (g->ubGroupSize) LoadPlayerGroupList(f, g);
		}
		else // Else it's an enemy group
		{
			LoadEnemyGroupStructFromSavedGame(f, g);
		}

		LoadWayPointList(f, g);

		// Add the node to the list
		*anchor = g;
		anchor  = &g->next;
	}

	//@@@ TEMP!
	//Rebuild the uniqueIDMask as a very old bug broke the uniqueID assignments in extremely rare cases.
	memset(uniqueIDMask, 0, sizeof(uniqueIDMask));
	CFOR_ALL_GROUPS(g)
	{
		const UINT32 index = g->ubGroupID / 32;
		const UINT32 bit   = g->ubGroupID % 32;
		const UINT32 mask  = 1 << bit;
		uniqueIDMask[index] += mask;
	}

	// Skip over saved unique id mask
	FileSeek(f, 32, FILE_SEEK_FROM_CURRENT);
}


// Saves the Player's group list to the saved game file
static void SavePlayerGroupList(HWFILE const f, GROUP const* const g)
{
	// Save the number of nodes in the list
	UINT32 uiNumberOfNodesInList = 0;
	CFOR_ALL_PLAYERS_IN_GROUP(p, g) ++uiNumberOfNodesInList;
	FileWrite(f, &uiNumberOfNodesInList, sizeof(UINT32));

	// Loop through and save only the players profile id
	CFOR_ALL_PLAYERS_IN_GROUP(p, g)
	{
		// Save the ubProfile ID for this node
		const UINT32 uiProfileID = p->pSoldier->ubProfile;
		FileWrite(f, &uiProfileID, sizeof(UINT32));
	}
}


//Loads the LL for the playerlist from the savegame file
static void LoadPlayerGroupList(HWFILE const f, GROUP* const g)
{
	// Load the number of nodes in the player list
	UINT32 node_count;
	FileRead(f, &node_count, sizeof(UINT32));

	PLAYERGROUP** anchor = &g->pPlayerList;
	for (UINT32 i = node_count; i != 0; --i)
	{
		PLAYERGROUP* const pg = MALLOC(PLAYERGROUP);

		UINT32 profile_id;
		FileRead(f, &profile_id, sizeof(UINT32));

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
static void SaveEnemyGroupStruct(HWFILE const f, GROUP const* const g)
{
	FileWrite(f, g->pEnemyGroup, sizeof(ENEMYGROUP));
}


// Loads the enemy group struct from the saved game file
static void LoadEnemyGroupStructFromSavedGame(HWFILE const f, GROUP* const g)
{
	ENEMYGROUP* const eg = MALLOCZ(ENEMYGROUP);
	FileRead(f, eg, sizeof(ENEMYGROUP));
	g->pEnemyGroup = eg;
}


static void SaveWayPointList(HWFILE const f, GROUP const* const g)
{
	// Save the number of waypoints
	UINT32 uiNumberOfWayPoints = 0;
	for (const WAYPOINT* w = g->pWaypoints; w != NULL; w = w->next)
	{
		++uiNumberOfWayPoints;
	}
	FileWrite(f, &uiNumberOfWayPoints, sizeof(UINT32));

	for (const WAYPOINT* w = g->pWaypoints; w != NULL; w = w->next)
	{
		BYTE  data[8];
		BYTE* d = data;
		INJ_U8(  d, w->x)
		INJ_U8(  d, w->y)
		INJ_SKIP(d, 6)
		Assert(d == endof(data));

		FileWrite(f, data, sizeof(data));
	}
}


static void LoadWayPointList(HWFILE const f, GROUP* const g)
{
	// Load the number of waypoints
	UINT32 uiNumberOfWayPoints;
	FileRead(f, &uiNumberOfWayPoints, sizeof(UINT32));

	WAYPOINT** anchor = &g->pWaypoints;
	for (UINT32 i = uiNumberOfWayPoints; i != 0; --i)
	{
		WAYPOINT* const w = MALLOCZ(WAYPOINT);

		BYTE data[8];
		FileRead(f, data, sizeof(data));

		BYTE const* d = data;
		EXTR_U8(  d, w->x)
		EXTR_U8(  d, w->y)
		EXTR_SKIP(d, 6)
		Assert(d == endof(data));

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

	uiSectorID = SECTOR( pGroup->ubSectorX, pGroup->ubSectorY );
	pSector = &SectorInfo[ uiSectorID ];

	if( pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrevX = pGroup->ubSectorX;
		pGroup->ubPrevY = pGroup->ubSectorY - 1;
	}
	else if( pSector->ubTraversability[ EAST_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ EAST_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrevX = pGroup->ubSectorX + 1;
		pGroup->ubPrevY = pGroup->ubSectorY;
	}
	else if( pSector->ubTraversability[ WEST_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ WEST_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrevX = pGroup->ubSectorX - 1;
		pGroup->ubPrevY = pGroup->ubSectorY;
	}
	else if( pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ] != GROUNDBARRIER &&
			pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ] != EDGEOFWORLD )
	{
		pGroup->ubPrevX = pGroup->ubSectorX;
		pGroup->ubPrevY = pGroup->ubSectorY + 1;
	}
	else
	{
		AssertMsg( 0, String("Player group cannot retreat from sector %c%d ", pGroup->ubSectorY+'A'-1, pGroup->ubSectorX ) );
		return;
	}
	if( pGroup->fPlayer )
	{ //update the previous sector for the mercs
		CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
		{
			pPlayer->pSoldier->ubPrevSectorID = (UINT8)SECTOR( pGroup->ubPrevX, pGroup->ubPrevY );
		}
	}
}

//Called when all checks have been made for the group (if possible to retreat, etc.)  This function
//blindly determines where to move the group.
void RetreatGroupToPreviousSector( GROUP *pGroup )
{
	UINT8 ubSector, ubDirection = 255;
	Assert( pGroup );
	AssertMsg( !pGroup->fBetweenSectors, "Can't retreat a group when between sectors!" );

	if( pGroup->ubPrevX != 16 || pGroup->ubPrevY != 16 )
	{ //Group has a previous sector
		pGroup->ubNextX = pGroup->ubPrevX;
		pGroup->ubNextY = pGroup->ubPrevY;

		//Determine the correct direction.
		const INT32 dx = pGroup->ubNextX - pGroup->ubSectorX;
		const INT32 dy = pGroup->ubNextY - pGroup->ubSectorY;
		if( dy == -1 && !dx )
			ubDirection = NORTH_STRATEGIC_MOVE;
		else if( dx == 1 && !dy )
			ubDirection = EAST_STRATEGIC_MOVE;
		else if( dy == 1 && !dx )
			ubDirection = SOUTH_STRATEGIC_MOVE;
		else if( dx == -1 && !dy )
			ubDirection = WEST_STRATEGIC_MOVE;
		else
		{

			AssertMsg( 0, String("Player group attempting illegal retreat from %c%d to %c%d.",
				pGroup->ubSectorY+'A'-1, pGroup->ubSectorX, pGroup->ubNextY+'A'-1, pGroup->ubNextX ) );
		}
	}
	else
	{ //Group doesn't have a previous sector.  Create one, then recurse
		CalculateGroupRetreatSector( pGroup );
		RetreatGroupToPreviousSector( pGroup );
	}

	//Calc time to get to next waypoint...
	ubSector = (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY );
	pGroup->uiTraverseTime = GetSectorMvtTimeForGroup( ubSector, ubDirection, pGroup );
	if( pGroup->uiTraverseTime == 0xffffffff )
	{
		AssertMsg( 0, String("Group %d (%s) attempting illegal move from %c%d to %c%d (%s).",
				pGroup->ubGroupID, ( pGroup->fPlayer ) ? "Player" : "AI",
				pGroup->ubSectorY+'A', pGroup->ubSectorX, pGroup->ubNextY+'A', pGroup->ubNextX,
				gszTerrain[SectorInfo[ubSector].ubTraversability[ubDirection]] ) );
	}

	if( !pGroup->uiTraverseTime )
	{ //Because we are in the strategic layer, don't make the arrival instantaneous (towns).
		pGroup->uiTraverseTime = 5;
	}

	SetGroupArrivalTime( pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime );
	pGroup->fBetweenSectors = TRUE;
	pGroup->uiFlags |= GROUPFLAG_JUST_RETREATED_FROM_BATTLE;

	if (pGroup->fVehicle)
	{
		// vehicle, set fact it is between sectors too
		VEHICLETYPE* const v = GetVehicleFromMvtGroup(pGroup);
		v->fBetweenSectors = TRUE;
	}

	//Post the event!
	if( !AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID ) )
		AssertMsg( 0, "Failed to add movement event." );

	//For the case of player groups, we need to update the information of the soldiers.
	if( pGroup->fPlayer )
	{
		if( pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY > GetWorldTotalMin( ) )
		{
			AddStrategicEvent( EVENT_GROUP_ABOUT_TO_ARRIVE, pGroup->uiArrivalTime - ABOUT_TO_ARRIVE_DELAY, pGroup->ubGroupID );
		}

		CFOR_ALL_PLAYERS_IN_GROUP(curr, pGroup)
		{
			curr->pSoldier->fBetweenSectors = TRUE;

			// OK, Remove the guy from tactical engine!
			RemoveSoldierFromTacticalSector(curr->pSoldier);
		}
	}
}


GROUP* FindEnemyMovementGroupInSector(const UINT8 ubSectorX, const UINT8 ubSectorY)
{
	FOR_ALL_ENEMY_GROUPS(g)
	{
		if (g->ubSectorX == ubSectorX &&
				g->ubSectorY == ubSectorY &&
				g->ubSectorZ == 0)
		{
			return g;
		}
	}
	return NULL;
}


GROUP* FindPlayerMovementGroupInSector(const UINT8 x, const UINT8 y)
{
	FOR_ALL_PLAYER_GROUPS(g)
	{
		// NOTE: These checks must always match the INVOLVED group checks in PBI!!!
		if (g->ubGroupSize != 0                 &&
				!g->fBetweenSectors                 &&
				g->ubSectorX   == x                 &&
				g->ubSectorY   == y                 &&
				g->ubSectorZ   == 0                 &&
				!GroupHasInTransitDeadOrPOWMercs(g) &&
				(!IsGroupTheHelicopterGroup(g) || !fHelicopterIsAirBorne))
		{
			return g;
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
	if( ( pGroup->ubSectorX == wp->x ) && ( pGroup->ubSectorY == wp->y ) )
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


//The sector supplied resets ALL enemy groups in the sector specified.  See comments in
//ResetMovementForEnemyGroup() for more details on what the resetting does.
void ResetMovementForEnemyGroupsInLocation( UINT8 ubSectorX, UINT8 ubSectorY )
{
	INT16 sSectorX, sSectorY, sSectorZ;

	GetCurrentBattleSectorXYZ( &sSectorX, &sSectorY, &sSectorZ );
	FOR_ALL_GROUPS_SAFE(pGroup)
	{
		if( !pGroup->fPlayer )
		{
			if( pGroup->ubSectorX == sSectorX && pGroup->ubSectorY == sSectorY )
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
	if( !pGroup->fBetweenSectors || !pGroup->ubNextX || !pGroup->ubNextY )
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
	SetGroupArrivalTime( pGroup, GetWorldTotalMin() + pGroup->uiTraverseTime );

	//Add a new event
	AddStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->uiArrivalTime, pGroup->ubGroupID );
}


void UpdatePersistantGroupsFromOldSave( UINT32 uiSavedGameVersion )
{
	INT32		cnt;
	BOOLEAN	fDoChange = FALSE;

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
		FOR_ALL_GROUPS_SAFE(g)
		{
			if (g->ubGroupSize == 0 && !g->fPersistant) RemovePGroup(g);
		}
	}
}

//Determines if any particular group WILL be moving through a given sector given it's current
//position in the route and the pGroup->ubMoveType must be ONE_WAY.  If the group is currently
//IN the sector, or just left the sector, it will return FALSE.
BOOLEAN GroupWillMoveThroughSector( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY )
{
	WAYPOINT *wp;
	INT32 i, dx, dy;
	UINT8 ubOrigX, ubOrigY;

	Assert( pGroup );
	AssertMsg( pGroup->ubMoveType == ONE_WAY, String( "GroupWillMoveThroughSector() -- Attempting to test group with an invalid move type.  ubGroupID: %d, ubMoveType: %d, sector: %c%d -- KM:0",
						 pGroup->ubGroupID, pGroup->ubMoveType, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX ) );

	//Preserve the original sector values, as we will be temporarily modifying the group's ubSectorX/Y values
	//as we traverse the waypoints.
	ubOrigX = pGroup->ubSectorX;
	ubOrigY = pGroup->ubSectorY;

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
		while( pGroup->ubSectorX != wp->x || pGroup->ubSectorY != wp->y )
		{
			//We now have the correct waypoint.
			//Analyse the group and determine which direction it will move from the current sector.
			dx = wp->x - pGroup->ubSectorX;
			dy = wp->y - pGroup->ubSectorY;
			if( dx && dy )
			{ //Can't move diagonally!
				AssertMsg( 0, String( "GroupWillMoveThroughSector() -- Attempting to process waypoint in a diagonal direction from sector %c%d to sector %c%d for group at sector %c%d -- KM:0",
					pGroup->ubSectorY + 'A', pGroup->ubSectorX, wp->y + 'A' - 1, wp->x, ubOrigY + 'A' - 1, ubOrigX ) );
				pGroup->ubSectorX = ubOrigX;
				pGroup->ubSectorY = ubOrigY;
				return TRUE;
			}
			if( !dx && !dy ) //Can't move to position currently at!
			{
				AssertMsg( 0, String( "GroupWillMoveThroughSector() -- Attempting to process same waypoint at %c%d for group at %c%d -- KM:0",
					wp->y + 'A' - 1, wp->x, ubOrigY + 'A' - 1, ubOrigX ) );
				pGroup->ubSectorX = ubOrigX;
				pGroup->ubSectorY = ubOrigY;
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
			else
			{
				Assert( 0 );
				pGroup->ubSectorX = ubOrigX;
				pGroup->ubSectorY = ubOrigY;
				return TRUE;
			}
			//Advance the sector value
			pGroup->ubSectorX = (UINT8)( dx + pGroup->ubSectorX );
			pGroup->ubSectorY = (UINT8)( dy + pGroup->ubSectorY );
			//Check to see if it the sector we are checking to see if this group will be moving through.
			if( pGroup->ubSectorX == ubSectorX && pGroup->ubSectorY == ubSectorY )
			{
				pGroup->ubSectorX = ubOrigX;
				pGroup->ubSectorY = ubOrigY;
				return TRUE;
			}
		}
		//Advance to the next waypoint.
		wp = wp->next;
	}
	pGroup->ubSectorX = ubOrigX;
	pGroup->ubSectorY = ubOrigY;
	return FALSE;
}


BOOLEAN VehicleHasFuel(const SOLDIERTYPE* const pSoldier)
{
	Assert( pSoldier->uiStatusFlags & SOLDIER_VEHICLE );
	if( pSoldier->sBreathRed )
	{
		return TRUE;
	}
	return FALSE;
}


static INT16 VehicleFuelRemaining(SOLDIERTYPE* pSoldier)
{
	Assert( pSoldier->uiStatusFlags & SOLDIER_VEHICLE );
	return pSoldier->sBreathRed;
}


static BOOLEAN SpendVehicleFuel(SOLDIERTYPE* pSoldier, INT16 sFuelSpent)
{
	Assert( pSoldier->uiStatusFlags & SOLDIER_VEHICLE );
	pSoldier->sBreathRed -= sFuelSpent;
	pSoldier->sBreathRed = (INT16)MAX( 0, pSoldier->sBreathRed );
	pSoldier->bBreath = (INT8)((pSoldier->sBreathRed+99) / 100);
	return( FALSE );
}


void AddFuelToVehicle(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pVehicle)
{
	OBJECTTYPE *pItem;
	INT16 sFuelNeeded, sFuelAvailable, sFuelAdded;
	pItem = &pSoldier->inv[ HANDPOS ];
	if( pItem->usItem != GAS_CAN )
	{
#ifdef JA2BETAVERSION
		wchar_t str[100];
		swprintf(str, lengthof(str), L"%ls is supposed to have gas can in hand.  ATE:0", pSoldier->name);
		DoScreenIndependantMessageBox(str, MSG_BOX_FLAG_OK, NULL);
#endif
		return;
	}
	//Soldier has gas can, so now add gas to vehicle while removing gas from the gas can.
	//A gas can with 100 status translate to 50% of a fillup.
	if( pVehicle->sBreathRed == 10000 )
	{ //Message for vehicle full?
		return;
	}
	if( pItem->bStatus )
	{ //Fill 'er up.
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
}


static void ReportVehicleOutOfGas(const VEHICLETYPE* const v, const UINT8 ubSectorX, const UINT8 ubSectorY)
{
	wchar_t str[255];
	//Report that the vehicle that just arrived is out of gas.
	swprintf(str, lengthof(str), gzLateLocalizedString[5], pVehicleStrings[v->ubVehicleType], ubSectorY + 'A' - 1, ubSectorX);
	DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
}


static void SetLocationOfAllPlayerSoldiersInGroup(GROUP const* const pGroup, INT16 const sSectorX, INT16 const sSectorY, INT8 const bSectorZ)
{
	SOLDIERTYPE *pSoldier = NULL;

	CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
	{
		pSoldier = pPlayer->pSoldier;

		if ( pSoldier != NULL )
		{
			pSoldier->sSectorX = sSectorX;
			pSoldier->sSectorY = sSectorY;
			pSoldier->bSectorZ = bSectorZ;
		}
	}

	// if it's a vehicle
	if ( pGroup->fVehicle )
	{
		VEHICLETYPE* const v = GetVehicleFromMvtGroup(pGroup);
		v->sSectorX = sSectorX;
		v->sSectorY = sSectorY;
		v->sSectorZ = bSectorZ;

		// if it ain't the chopper
		if (VEHICLE2ID(v) != iHelicopterVehicleId)
		{
			pSoldier = GetSoldierStructureForVehicle(v);
			// these are apparently unnecessary, since vehicles are part of the pPlayerList in a vehicle group.  Oh well.
			pSoldier->sSectorX = sSectorX;
			pSoldier->sSectorY = sSectorY;
			pSoldier->bSectorZ = bSectorZ;
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

	SetEnemyGroupSector( pGroup, ubSectorID );
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

	if( pGroup->ubSectorZ )
	{ //no ambushes underground (no bloodcats either)
		return FALSE;
	}

	ubSectorID = (UINT8)SECTOR( pGroup->ubSectorX, pGroup->ubSectorY );
	pSector = &SectorInfo[ ubSectorID ];

	ubChance = 5 * gGameOptions.ubDifficultyLevel;

	iHoursElapsed = (GetWorldTotalMin() - pSector->uiTimeCurrentSectorWasLastLoaded) / 60;
	if( ubSectorID == SEC_N5 || ubSectorID == SEC_I16 )
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
			bNumMercMaxCats = (INT8)(PlayerMercsInSector( pGroup->ubSectorX, pGroup->ubSectorY, pGroup->ubSectorZ ) * 2);

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
	else if( ubSectorID != SEC_I16 )
	{
		if( !gfAutoAmbush && PreChance( 95 ) )
		{ //already ambushed here.  But 5% chance of getting ambushed again!
			fAlreadyAmbushed = TRUE;
		}
	}

	if( !fAlreadyAmbushed && ubSectorID != SEC_N5 && pSector->bBloodCats > 0 &&
			!pGroup->fVehicle && !NumEnemiesInSector( pGroup->ubSectorX, pGroup->ubSectorY ) )
	{
		if( ubSectorID != SEC_I16 || !gubFact[ FACT_PLAYER_KNOWS_ABOUT_BLOODCAT_LAIR ] )
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


static void NotifyPlayerOfBloodcatBattle(UINT8 ubSectorX, UINT8 ubSectorY)
{
	wchar_t str[ 256 ];
	wchar_t zTempString[ 128 ];
	if( gubEnemyEncounterCode == BLOODCAT_AMBUSH_CODE )
	{
		GetSectorIDString( ubSectorX, ubSectorY, 0, zTempString, lengthof(zTempString), TRUE );
		swprintf( str, lengthof(str), pMapErrorString[ 12 ], zTempString );
	}
	else if( gubEnemyEncounterCode == ENTERING_BLOODCAT_LAIR_CODE )
	{
		wcscpy( str, pMapErrorString[ 13 ] );
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


void PlaceGroupInSector(GROUP* const g, INT16 const sPrevX, INT16 const sPrevY, INT16 const sNextX, INT16 const sNextY, INT8 const bZ, BOOLEAN const fCheckForBattle)
{
	ClearMercPathsAndWaypointsForAllInGroup(g);

	// change where they are and where they're going
	SetGroupPrevSectors(g, sPrevX, sPrevY);
	SetGroupSectorValue(sPrevX, sPrevY, bZ, g);
	SetGroupNextSectorValue(sNextX, sNextY, g);

	// call arrive event
	GroupArrivedAtSector(g, fCheckForBattle, FALSE);
}



// ARM: centralized it so we can do a comprehensive Assert on it.  Causing problems with helicopter group!
void SetGroupArrivalTime( GROUP *pGroup, UINT32 uiArrivalTime )
{
	// PLEASE CENTRALIZE ALL CHANGES TO THE ARRIVAL TIMES OF GROUPS THROUGH HERE, ESPECIALLY THE HELICOPTER GROUP!!!

	// if this group is the helicopter group, we have to make sure that its arrival time is never greater than the sum
	// of the current time and its traverse time, 'cause those 3 values are used to plot its map position!  Because of this
	// the chopper groups must NEVER be delayed for any reason - it gets excluded from simultaneous arrival logic

	// Also note that non-chopper groups can currently be delayed such that this assetion would fail - enemy groups by
	// DelayEnemyGroupsIfPathsCross(), and player groups via PrepareGroupsForSimultaneousArrival().  So we skip the assert.

	if ( IsGroupTheHelicopterGroup( pGroup ) )
	{
		// make sure it's valid (NOTE: the correct traverse time must be set first!)
		if ( uiArrivalTime > ( GetWorldTotalMin() + pGroup->uiTraverseTime ) )
		{
			AssertMsg( FALSE, String( "SetGroupArrivalTime: Setting invalid arrival time %d for group %d, WorldTime = %d, TraverseTime = %d", uiArrivalTime, pGroup->ubGroupID, GetWorldTotalMin(), pGroup->uiTraverseTime ) );

			// fix it if assertions are disabled
			uiArrivalTime = GetWorldTotalMin() + pGroup->uiTraverseTime;
		}
	}

	pGroup->uiArrivalTime = uiArrivalTime;
}


// non-persistent groups should be simply removed instead!
static void CancelEmptyPersistentGroupMovement(GROUP* pGroup)
{
	Assert( pGroup );
	Assert( pGroup->ubGroupSize == 0 );
	Assert( pGroup->fPersistant );


	// don't do this for vehicle groups - the chopper can keep flying empty,
	// while other vehicles still exist and teleport to nearest sector instead
	if ( pGroup->fVehicle )
	{
		return;
	}

	// prevent it from arriving empty
	DeleteStrategicEvent( EVENT_GROUP_ARRIVAL, pGroup->ubGroupID );

	RemoveGroupWaypoints(pGroup);

	pGroup->uiTraverseTime = 0;
	SetGroupArrivalTime( pGroup, 0 );
	pGroup->fBetweenSectors = FALSE;

	pGroup->ubPrevX = 0;
	pGroup->ubPrevY = 0;
	pGroup->ubSectorX = 0;
	pGroup->ubSectorY = 0;
	pGroup->ubNextX = 0;
	pGroup->ubNextY = 0;
}


static BOOLEAN HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(GROUP* pGroup);


// look for NPCs to stop for, anyone is too tired to keep going, if all OK rebuild waypoints & continue movement
void PlayerGroupArrivedSafelyInSector( GROUP *pGroup, BOOLEAN fCheckForNPCs )
{
	BOOLEAN fPlayerPrompted = FALSE;


	Assert( pGroup );
	Assert( pGroup->fPlayer );


	// if we haven't already checked for NPCs, and the group isn't empty
	if (fCheckForNPCs && HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(pGroup))
	{
		// wait for player to answer/confirm prompt before doing anything else
		fPlayerPrompted = TRUE;
	}

	// if we're not prompting the player
	if ( !fPlayerPrompted )
	{
		// and we're not at the end of our road
		if ( !GroupAtFinalDestination( pGroup ) )
		{
			if ( AnyMercInGroupCantContinueMoving( pGroup ) )
			{
				// stop: clear their strategic movement (mercpaths and waypoints)
				ClearMercPathsAndWaypointsForAllInGroup( pGroup );

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
				RebuildWayPointsForGroupPath(GetGroupMercPathPtr(pGroup), pGroup);
			}
		}
	}
}


static void HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback(MessageBoxReturnValue);
static BOOLEAN WildernessSectorWithAllProfiledNPCsNotSpokenWith(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ);


static BOOLEAN HandlePlayerGroupEnteringSectorToCheckForNPCsOfNote(GROUP* pGroup)
{
	INT16 sSectorX = 0, sSectorY = 0;
	INT8 bSectorZ = 0;
	CHAR16 sString[ 128 ];
	CHAR16 wSectorName[ 128 ];
	INT16 sStrategicSector;


	Assert( pGroup );
	Assert( pGroup->fPlayer );

	// nobody in the group (perfectly legal with the chopper)
	if ( pGroup->pPlayerList == NULL )
	{
		return( FALSE );
	}

	// chopper doesn't stop for NPCs
	if ( IsGroupTheHelicopterGroup( pGroup ) )
	{
		return( FALSE );
	}

	// if we're already in the middle of a prompt (possible with simultaneously group arrivals!), don't try to prompt again
	if ( gpGroupPrompting != NULL )
	{
		return( FALSE );
	}


	// get the sector values
	sSectorX = pGroup->ubSectorX;
	sSectorY = pGroup->ubSectorY;
	bSectorZ = pGroup->ubSectorZ;


	// don't do this for underground sectors
	if ( bSectorZ != 0 )
	{
		return( FALSE );
	}

	// get the strategic sector value
	sStrategicSector = sSectorX + MAP_WORLD_X * sSectorY;

	// skip towns/pseudo-towns (anything that shows up on the map as being special)
	if( StrategicMap[ sStrategicSector ].bNameId != BLANK_SECTOR )
	{
		return( FALSE );
	}

	// skip SAM-sites
	if ( IsThisSectorASAMSector( sSectorX, sSectorY, bSectorZ ) )
	{
		return( FALSE );
	}

	// check for profiled NPCs in sector
	if (!WildernessSectorWithAllProfiledNPCsNotSpokenWith(sSectorX, sSectorY, bSectorZ))
	{
		return( FALSE );
	}


	// store the group ptr for use by the callback function
	gpGroupPrompting = pGroup;

	// build string for squad
	GetSectorIDString( sSectorX, sSectorY, bSectorZ, wSectorName, lengthof(wSectorName), FALSE );
	swprintf(sString, lengthof(sString), pLandMarkInSectorString, pGroup->pPlayerList->pSoldier->bAssignment + 1, wSectorName);

	if ( GroupAtFinalDestination( pGroup ) )
	{
		// do an OK message box
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_OK, HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback );
	}
	else
	{
		// do a CONTINUE/STOP message box
		DoScreenIndependantMessageBox( sString, MSG_BOX_FLAG_CONTINUESTOP, HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback );
	}

	// wait, we're prompting the player
	return( TRUE );
}


static BOOLEAN WildernessSectorWithAllProfiledNPCsNotSpokenWith(INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ)
{
	UINT8									ubProfile;
	BOOLEAN fFoundSomebody = FALSE;


	for ( ubProfile = FIRST_RPC; ubProfile < NUM_PROFILES; ubProfile++ )
	{
		MERCPROFILESTRUCT const& p = GetProfile(ubProfile);

		// skip stiffs
		if (p.bMercStatus == MERC_IS_DEAD) continue;
		if (p.bLife <= 0)                  continue;

 		// skip vehicles
		if ( ubProfile >= PROF_HUMMER && ubProfile <= PROF_HELICOPTER )
		{
			continue;
		}

		// in this sector?
		if (p.sSectorX == sSectorX && p.sSectorY == sSectorY && p.bSectorZ == bSectorZ)
		{
			// if we haven't talked to him yet, and he's not currently recruired/escorted by player (!)
			if (p.ubLastDateSpokenTo == 0 &&
					!(p.ubMiscFlags & (PROFILE_MISC_FLAG_RECRUITED | PROFILE_MISC_FLAG_EPCACTIVE)))
			{
				// then this is a guy we need to stop for...
				fFoundSomebody = TRUE;
			}
			else
			{
				// already spoke to this guy, don't prompt about this sector again, regardless of status of other NPCs here
				// (although Hamous wanders around, he never shares the same wilderness sector as other important NPCs)
				return( FALSE );
			}
		}
	}


	return( fFoundSomebody );
}


static void HandlePlayerGroupEnteringSectorToCheckForNPCsOfNoteCallback(MessageBoxReturnValue const ubExitValue)
{
	Assert( gpGroupPrompting );

	if ( (ubExitValue == MSG_BOX_RETURN_YES) ||
			 (ubExitValue == MSG_BOX_RETURN_OK) )
	{
		// NPCs now checked, continue moving if appropriate
		PlayerGroupArrivedSafelyInSector( gpGroupPrompting, FALSE );
	}
	else if( ubExitValue == MSG_BOX_RETURN_NO )
	{
		// stop here

		// clear their strategic movement (mercpaths and waypoints)
		ClearMercPathsAndWaypointsForAllInGroup( gpGroupPrompting );

//		// if currently selected sector has nobody in it
//		if ( PlayerMercsInSector( ( UINT8 ) sSelMapX, ( UINT8 ) sSelMapY, ( UINT8 ) iCurrentMapSectorZ ) == 0 )
		// New: ALWAYS make this sector strategically selected, even if there were mercs in the previously selected one
		{
			ChangeSelectedMapSector( gpGroupPrompting->ubSectorX, gpGroupPrompting->ubSectorY, gpGroupPrompting->ubSectorZ );
		}

		StopTimeCompression();
	}

	gpGroupPrompting = NULL;

	fMapPanelDirty = TRUE;
	fMapScreenBottomDirty = TRUE;
}


BOOLEAN DoesPlayerExistInPGroup(GROUP const* const g, SOLDIERTYPE const* const pSoldier)
{
	CFOR_ALL_PLAYERS_IN_GROUP(curr, g)
	{ //definately more than one node

		if( curr->pSoldier == pSoldier )
		{
			return TRUE;
		}
	}

	// !curr
	return FALSE;
}


BOOLEAN GroupHasInTransitDeadOrPOWMercs(const GROUP* const pGroup)
{
	CFOR_ALL_PLAYERS_IN_GROUP(pPlayer, pGroup)
	{
		if ( pPlayer->pSoldier )
		{
			if( ( pPlayer->pSoldier->bAssignment == IN_TRANSIT ) ||
					( pPlayer->pSoldier->bAssignment == ASSIGNMENT_POW ) ||
					( pPlayer->pSoldier->bAssignment == ASSIGNMENT_DEAD ) )
			{
				// yup!
				return( TRUE );
			}
		}
	}

	// nope
	return( FALSE );
}


#ifdef JA2BETAVERSION

static void ValidateGroups(GROUP const* const pGroup)
{
	//Do error checking, and report group
	ValidatePlayersAreInOneGroupOnly();
	if( !pGroup->fPlayer && !pGroup->ubGroupSize )
	{
		//report error
		wchar_t str[512];
		if( pGroup->ubSectorIDOfLastReassignment == 255 )
		{
			swprintf(str, lengthof(str), L"Enemy group found with 0 troops in it.  This is illegal and group will be deleted."
										 L"  Group %d in sector %c%d originated from sector %c%d.",
										 pGroup->ubGroupID, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
										 SECTORY( pGroup->ubCreatedSectorID ) + 'A' - 1, SECTORX( pGroup->ubCreatedSectorID ) );
		}
		else
		{
			swprintf(str, lengthof(str), L"Enemy group found with 0 troops in it.  This is illegal and group will be deleted."
										 L"  Group %d in sector %c%d originated from sector %c%d and last reassignment location was %c%d.",
										 pGroup->ubGroupID, pGroup->ubSectorY + 'A' - 1, pGroup->ubSectorX,
										 SECTORY( pGroup->ubCreatedSectorID ) + 'A' - 1, SECTORX( pGroup->ubCreatedSectorID ),
										 SECTORY( pGroup->ubSectorIDOfLastReassignment ) + 'A' - 1, SECTORX( pGroup->ubSectorIDOfLastReassignment ) );
		}
		//correct error

		DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
	}
}
#endif
