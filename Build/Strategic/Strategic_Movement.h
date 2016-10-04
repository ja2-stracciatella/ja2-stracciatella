#ifndef __STRATEGIC_MOVEMENT_H
#define __STRATEGIC_MOVEMENT_H

#include "JA2Types.h"


enum //enemy intentions,
{
	NO_INTENTIONS,			//enemy intentions are undefined.
	PURSUIT,						//enemy group has spotted a player group and is pursuing them.  If they lose the player group, they
											//will get reassigned.
	STAGING,						//enemy is prepare to assault a town sector, but doesn't have enough troops.
	PATROL,							//enemy is moving around determining safe areas.
	REINFORCEMENTS,			//enemy group has intentions to fortify position at final destination.
	ASSAULT,						//enemy is ready to fight anything they encounter.
	NUM_ENEMY_INTENTIONS
};

enum //move types
{
	ONE_WAY,						//from first waypoint to last, deleting each waypoint as they are reached.
	CIRCULAR,						//from first to last, recycling forever.
	ENDTOEND_FORWARDS,	//from first to last -- when reaching last, change to backwards.
	ENDTOEND_BACKWARDS	//from last to first -- when reaching first, change to forwards.
};

enum
{
	NORTH_STRATEGIC_MOVE,
	EAST_STRATEGIC_MOVE,
	SOUTH_STRATEGIC_MOVE,
	WEST_STRATEGIC_MOVE,
	THROUGH_STRATEGIC_MOVE
};

//This structure contains all of the information about a group moving in the strategic
//layer.  This includes all troops, equipment, and waypoints, and location.
//NOTE:  This is used for groups that are initiating a movement to another sector.
struct WAYPOINT
{
	UINT8 x;											//sector x position of waypoint
	UINT8 y;											//sector y position of waypoint
	WAYPOINT* next; //next waypoint in list
};


struct PLAYERGROUP
{
	SOLDIERTYPE *pSoldier;				//direct access to the soldier pointer
	PLAYERGROUP* next;			//next player in list
};


struct ENEMYGROUP
{
	UINT8 ubNumTroops;						//number of regular troops in the group
	UINT8 ubNumElites;						//number of elite troops in the group
	UINT8 ubNumAdmins;						//number of administrators in the group
	UINT8 ubPendingReinforcements;//This group is waiting for reinforcements before attacking or attempting to fortify newly aquired sector.
	UINT8 ubAdminsInBattle;				//number of administrators in currently in battle.
	UINT8 ubIntention;						//the type of group this is:  patrol, assault, spies, etc.
	UINT8 ubTroopsInBattle;				//number of soldiers currently in battle.
	UINT8 ubElitesInBattle;				//number of elite soldiers currently in battle.
};


//NOTE:  ALL FLAGS ARE CLEARED WHENEVER A GROUP ARRIVES IN A SECTOR, OR ITS WAYPOINTS ARE
//       DELETED!!!
#define GROUPFLAG_SIMULTANEOUSARRIVAL_APPROVED	0x00000001
#define GROUPFLAG_SIMULTANEOUSARRIVAL_CHECKED		0x00000002
//I use this flag when traversing through a list to determine which groups meet whatever conditions,
//then add this marker flag.  The second time I traverse the list, I simply check for this flag,
//apply my modifications to the group, and remove the flag.  If you decide to use it, make sure the
//flag is cleared.
#define GROUPFLAG_MARKER												0x00000004
//Set whenever a group retreats from battle.  If the group arrives in the next sector and enemies are there
//retreat will not be an option.
#define GROUPFLAG_JUST_RETREATED_FROM_BATTLE		0x00000008
#define GROUPFLAG_HIGH_POTENTIAL_FOR_AMBUSH			0x00000010
#define GROUPFLAG_GROUP_ARRIVED_SIMULTANEOUSLY	0x00000020


struct GROUP
{
	BOOLEAN fDebugGroup;					//for testing purposes -- handled differently in certain cases.
	BOOLEAN fPlayer;							//set if this is a player controlled group.
	BOOLEAN fVehicle;							//vehicle controlled group?
	BOOLEAN fPersistant;					//This flag when set prevents the group from being automatically deleted when it becomes empty.
	UINT8 ubGroupID;							//the unique ID of the group (used for hooking into events and SOLDIERTYPE)
	UINT8 ubGroupSize;						//total number of individuals in the group.
	UINT8 ubSectorX, ubSectorY;		//last/curr sector occupied
	UINT8 ubSectorZ;
	UINT8 ubNextX, ubNextY;				//next sector destination
	UINT8 ubPrevX, ubPrevY;				//prev sector occupied (could be same as ubSectorX/Y)
	UINT8 ubOriginalSector;				//sector where group was created.
	BOOLEAN fBetweenSectors;			//set only if a group is between sector.
	UINT8 ubMoveType;							//determines the type of movement (ONE_WAY, CIRCULAR, ENDTOEND, etc.)
	UINT8 ubNextWaypointID;				//the ID of the next waypoint
	UINT32 uiArrivalTime;					//the arrival time in world minutes that the group will arrive at the next sector.
	UINT32 uiTraverseTime;				//the total traversal time from the previous sector to the next sector.
	WAYPOINT *pWaypoints;					//a list of all of the waypoints in the groups movement.
	UINT8 ubTransportationMask;		//the mask combining all of the groups transportation methods.
	UINT32 uiFlags;								//various conditions that apply to the group
	UINT8 ubCreatedSectorID;			//used for debugging strategic AI for keeping track of the sector ID a group was created in.
	UINT8 ubSectorIDOfLastReassignment;	//used for debuggin strategic AI.  Records location of any reassignments.

	union
	{
		PLAYERGROUP *pPlayerList;		//list of players in the group
		ENEMYGROUP *pEnemyGroup;		//a structure containing general enemy info
	};
	GROUP* next;						//next group
};


extern GROUP *gpGroupList;


#define BASE_FOR_EACH_GROUP(type, iter) \
	for (type iter = gpGroupList; iter != NULL; iter = iter->next)
#define FOR_EACH_GROUP(iter)  BASE_FOR_EACH_GROUP(      GROUP*, iter)
#define CFOR_EACH_GROUP(iter) BASE_FOR_EACH_GROUP(const GROUP*, iter)

#define BASE_FOR_EACH_ENEMY_GROUP(type, iter) \
	BASE_FOR_EACH_GROUP(type, iter)                  \
		if (iter->fPlayer) continue; else
#define FOR_EACH_ENEMY_GROUP(iter)  BASE_FOR_EACH_ENEMY_GROUP(      GROUP*, iter)
#define CFOR_EACH_ENEMY_GROUP(iter) BASE_FOR_EACH_ENEMY_GROUP(const GROUP*, iter)

#define BASE_FOR_EACH_PLAYER_GROUP(type, iter) \
	BASE_FOR_EACH_GROUP(type, iter)              \
		if (!iter->fPlayer) continue; else
#define FOR_EACH_PLAYER_GROUP(iter)  BASE_FOR_EACH_PLAYER_GROUP(      GROUP*, iter)
#define CFOR_EACH_PLAYER_GROUP(iter) BASE_FOR_EACH_PLAYER_GROUP(const GROUP*, iter)

#define FOR_EACH_GROUP_SAFE(iter)                                                    \
	for (GROUP* iter = gpGroupList, * iter##__next; iter != NULL; iter = iter##__next) \
		if (iter##__next = iter->next, FALSE) {} else                                    \


#define CFOR_EACH_PLAYER_IN_GROUP(iter, group) \
	for (PLAYERGROUP const* iter = (Assert((group)->fPlayer), (group)->pPlayerList); iter; iter = iter->next)


//General utility functions
void RemoveAllGroups(void);
GROUP* GetGroup( UINT8 ubGroupID );

/* Remove a group from the list. This removes all of the waypoints as well as
 * the members of the group. Calling this function doesn't position them in a
 * sector. It is up to you to do that. The event system will automatically
 * handle their updating as they arrive in sectors. */
void RemoveGroup(GROUP&);

/* Clear a group's waypoints. This is necessary when sending new orders such as
 * different routes. */
void RemoveGroupWaypoints(GROUP&);

//Player grouping functions

/* Create a new player group.  This is the first step before adding waypoints
 * and members to the player group. */
GROUP* CreateNewPlayerGroupDepartingFromSector(UINT8 ubSectorX, UINT8 ubSectorY);

//Allows you to add or remove players from the group.
void AddPlayerToGroup(GROUP&, SOLDIERTYPE&);

void RemovePlayerFromGroup(SOLDIERTYPE&);
void RemovePlayerFromPGroup(GROUP&, SOLDIERTYPE&);

// create a vehicle group, it is by itself,
GROUP* CreateNewVehicleGroupDepartingFromSector(UINT8 ubSectorX, UINT8 ubSectorY);


//Appends a waypoint to the end of the list.  Waypoint MUST be on the
//same horizontal xor vertical level as the last waypoint added.
BOOLEAN AddWaypointToPGroup( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY );
//Same, but uses a plain sectorID (0-255)
BOOLEAN AddWaypointIDToPGroup( GROUP *pGroup, UINT8 ubSectorID );
//Same, but uses a strategic sectorID
BOOLEAN AddWaypointStrategicIDToPGroup( GROUP *pGroup, UINT32 uiSectorID );

//Enemy grouping functions -- private use by the strategic AI.
//............................................................
GROUP* CreateNewEnemyGroupDepartingFromSector( UINT32 uiSector, UINT8 ubNumAdmins, UINT8 ubNumTroops, UINT8 ubNumElites );

/* Arrival callback -- None of these functions should be called directly.
 * This is called whenever any group arrives in the next sector (player or
 * enemy). This function will first check to see if a battle should start, or if
 * they aren't at the final destination, they will move to the next sector. */
void GroupArrivedAtSector(GROUP&, BOOLEAN check_for_battle, BOOLEAN never_left);
void CalculateNextMoveIntention( GROUP *pGroup );


// Set current sector of the group, used for player controlled mercs
void SetGroupSectorValue(INT16 x, INT16 y, INT16 z, GROUP&);

void SetEnemyGroupSector(GROUP&, UINT8 sector_id);


// calculate the eta time in world total mins of this group
INT32 CalculateTravelTimeOfGroup(GROUP const*);

static UINT32 const TRAVERSE_TIME_IMPOSSIBLE = 0xFFFFFFFF;

// Get travel time for this group
INT32 GetSectorMvtTimeForGroup(UINT8 ubSector, UINT8 ubDirection, GROUP const*);

UINT8 PlayerMercsInSector( UINT8 ubSectorX, UINT8 ubSectorY, UINT8 ubSectorZ );
UINT8 PlayerGroupsInSector( UINT8 ubSectorX, UINT8 ubSectorY, UINT8 ubSectorZ );

// Is this player group in motion?
BOOLEAN PlayerGroupInMotion(GROUP const*);

// Is the player greoup with this id in motion
bool PlayerIDGroupInMotion(UINT8 id);

// get number of mercs between sectors
BOOLEAN PlayersBetweenTheseSectors( INT16 sSource, INT16 sDest, INT32 *iCountEnter, INT32 *iCountExit, BOOLEAN *fAboutToArriveEnter );

void MoveAllGroupsInCurrentSectorToSector( UINT8 ubSectorX, UINT8 ubSectorY, UINT8 ubSectorZ );

//Save the strategic movemnet Group paths to the saved game file
void SaveStrategicMovementGroupsToSaveGameFile(HWFILE);

//Load the strategic movement Group paths from the saved game file
void LoadStrategicMovementGroupsFromSavedGameFile(HWFILE);

void HandleArrivalOfReinforcements(GROUP const*);

//Called when all checks have been made for the group (if possible to retreat, etc.)  This function
//blindly determines where to move the group.
void RetreatGroupToPreviousSector(GROUP&);

GROUP* FindEnemyMovementGroupInSector(UINT8 x, UINT8 y);
GROUP* FindPlayerMovementGroupInSector(UINT8 x, UINT8 y);

BOOLEAN GroupAtFinalDestination(const GROUP*);

// find the travel time between waypts for this group
INT32 FindTravelTimeBetweenWaypoints(WAYPOINT const* pSource, WAYPOINT const* pDest,  GROUP const*);

BOOLEAN GroupReversingDirectionsBetweenSectors( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY, BOOLEAN fBuildingWaypoints );
BOOLEAN GroupBetweenSectorsAndSectorXYIsInDifferentDirection( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY );

WAYPOINT* GetFinalWaypoint(const GROUP*);

void ResetMovementForEnemyGroupsInLocation( UINT8 ubSectorX, UINT8 ubSectorY );

//Determines if any particular group WILL be moving through a given sector given it's current
//position in the route and TREATS the pGroup->ubMoveType as ONE_WAY EVEN IF IT ISN'T.  If the
//group is currently IN the sector, or just left the sector, it will return FALSE.
BOOLEAN GroupWillMoveThroughSector( GROUP *pGroup, UINT8 ubSectorX, UINT8 ubSectorY );

void RandomizePatrolGroupLocation( GROUP *pGroup );

void PlaceGroupInSector(GROUP&, INT16 prev_x, INT16 prev_y, INT16 next_x, INT16 next_y, INT8 z, bool check_for_battle);

void SetGroupArrivalTime(GROUP&, UINT32 arrival_time);

void PlayerGroupArrivedSafelyInSector(GROUP&, BOOLEAN fCheckForNPCs);

bool DoesPlayerExistInPGroup(GROUP const&, SOLDIERTYPE const&);

bool GroupHasInTransitDeadOrPOWMercs(GROUP const&);

void AddFuelToVehicle(SOLDIERTYPE* pSoldier, SOLDIERTYPE* pVehicle);

void CalculateGroupRetreatSector(GROUP*);

void UpdatePersistantGroupsFromOldSave(UINT32 uiSavedGameVersion);

extern BOOLEAN gfUndergroundTacticalTraversal;

#endif
