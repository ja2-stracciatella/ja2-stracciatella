#ifndef __POINTS_H_
#define __POINTS_H_

#include "Item_Types.h"
#include "JA2Types.h"


#define AP_MINIMUM			10 // no merc can have less for his turn
#define AP_MAXIMUM			25 // no merc can have more for his turn
#define AP_MONSTER_MAXIMUM		40 // no monster can have more for his turn
#define AP_VEHICLE_MAXIMUM		50 // no merc can have more for his turn
#define AP_INCREASE			10 // optional across-the-board AP boost
#define MAX_AP_CARRIED			5 // APs carried from turn-to-turn

// monster AP bonuses, expressed in 10ths (12 = 120% normal)
#define AP_YOUNG_MONST_FACTOR		15
#define AP_ADULT_MONST_FACTOR		12
#define AP_MONST_FRENZY_FACTOR		13

// AP penalty for a phobia situation (again, in 10ths)
#define AP_CLAUSTROPHOBE		9
#define AP_AFRAID_OF_INSECTS		8

#define AP_EXCHANGE_PLACES		5

// Action Point values
#define AP_REVERSE_MODIFIER		1
#define AP_STEALTH_MODIFIER		2

#define AP_STEAL_ITEM			10 // APs to steal item....

#define AP_TAKE_BLOOD			10

#define AP_TALK			6

#define AP_MOVEMENT_FLAT		3 // div by 2 for run, +2, for crawl, -1 for swat
#define AP_MOVEMENT_GRASS		4
#define AP_MOVEMENT_BUSH		5
#define AP_MOVEMENT_RUBBLE		6
#define AP_MOVEMENT_SHORE		7 // shallow wade

#define AP_MOVEMENT_LAKE		9 // deep wade -> slowest
#define AP_MOVEMENT_OCEAN		8 // swimming is faster than deep wade

#define AP_CHANGE_FACING		1 // turning to face any other direction
#define AP_CHANGE_TARGET		1 // aiming at a new target

#define AP_CATCH_ITEM			5 // turn to catch item
#define AP_TOSS_ITEM			8 // toss item from inv

#define AP_REFUEL_VEHICLE		10
/*
#define AP_MOVE_ITEM_FREE		0 // same place, pocket->pocket
#define AP_MOVE_ITEM_FAST		2 // hand, holster, ground only
#define AP_MOVE_ITEM_AVG		4 // everything else!
#define AP_MOVE_ITEM_SLOW		6 // vests, protective gear
*/
#define AP_MOVE_ITEM_FAST		4 // hand, holster, ground only
#define AP_MOVE_ITEM_SLOW		6 // vests, protective gear

#define AP_RADIO			5
#define AP_CROUCH			2
#define AP_PRONE			2

#define AP_LOOK_STANDING		1
#define AP_LOOK_CROUCHED		2
#define AP_LOOK_PRONE			2

#define AP_READY_KNIFE			0
#define AP_READY_PISTOL		1
#define AP_READY_RIFLE			2
#define AP_READY_SAW			0
// JA2Gold: reduced dual AP cost from 3 to 1
//#define AP_READY_DUAL		3
#define AP_READY_DUAL			1


#define AP_MIN_AIM_ATTACK		0 // minimum permitted extra aiming
#define AP_MAX_AIM_ATTACK		4 // maximum permitted extra aiming

#define AP_BURST			5

#define AP_DROP_BOMB			3

#define AP_RELOAD_GUN			5 // loading new clip/magazine

#define AP_START_FIRST_AID		5 // get the stuff out of medic kit
#define AP_PER_HP_FIRST_AID		1 // for each point healed
#define AP_STOP_FIRST_AID		3 // put everything away again

#define AP_START_REPAIR		5 // get the stuff out of repair kit

#define AP_GET_HIT			2 // struck by bullet, knife, explosion
#define AP_GET_WOUNDED_DIVISOR		4 // 1 AP lost for every 'divisor' dmg
#define AP_FALL_DOWN			4 // falling down (explosion, exhaustion)
#define AP_GET_THROWN			2 // get thrown back (by explosion)

#define AP_GET_UP			5 // getting up again
#define AP_ROLL_OVER			2 // flipping from back to stomach

#define AP_OPEN_DOOR			3 // whether successful, or not (locked)
#define AP_PICKLOCK			10 // should really be several turns
#define AP_EXAMINE_DOOR		5 // time to examine door
#define AP_BOOT_DOOR			8 // time to boot door
#define AP_USE_CROWBAR			10 // time to crowbar door
#define AP_UNLOCK_DOOR			6 // time to unlock door
#define AP_LOCK_DOOR			6 // time to lock door
#define AP_EXPLODE_DOOR		10 // time to set explode charge on door
#define AP_UNTRAP_DOOR			10 // time to untrap door

#define AP_USEWIRECUTTERS		10 // Time to use wirecutters

#define AP_CLIMBROOF			10 // APs to climb roof
#define AP_CLIMBOFFROOF		6 // APs to climb off roof
#define AP_JUMPFENCE			6 // time to jump over a fence


#define AP_OPEN_SAFE			8 // time to use combination


#define AP_USE_REMOTE			2
#define AP_PULL_TRIGGER		2 // operate nearby panic trigger

#define AP_FORCE_LID_OPEN		10
#define AP_SEARCH_CONTAINER		5 // boxes, crates, safe, etc.

#define AP_READ_NOTE			10 // reading a note's contents in inv.

#define AP_SNAKE_BATTLE		10 // when first attacked
#define AP_KILL_SNAKE			7 // when snake battle's been won

#define AP_USE_SURV_CAM		5

#define AP_PICKUP_ITEM			3
#define AP_GIVE_ITEM			1

#define AP_BURY_MINE			10
#define AP_DISARM_MINE			10

#define AP_DRINK			5
#define AP_CAMOFLAGE			10

#define AP_TAKE_PHOTOGRAPH		5
#define AP_MERGE			8

#define AP_OTHER_COST			99

#define AP_START_RUN_COST		1

#define AP_ATTACH_CAN			5

#define AP_JUMP_OVER			6

// special Breath Point related constants

#define BP_RATIO_RED_PTS_TO_NORMAL	100

#define BP_RUN_ENERGYCOSTFACTOR	3 // Richard thinks running is 3rd most strenous over time... tough, Mark didn't. CJC increased it again
#define BP_WALK_ENERGYCOSTFACTOR	1 // walking subtracts flat terrain breath value
#define BP_SWAT_ENERGYCOSTFACTOR	2 // Richard thinks swatmove is 2nd most strenous over time... tough, Mark didn't
#define BP_CRAWL_ENERGYCOSTFACTOR	4 // Richard thinks crawling is the MOST strenuous over time

#define BP_RADIO			0 // no breath cost
#define BP_USE_DETONATOR		0 // no breath cost

#define BP_REVERSE_MODIFIER		0 // no change, a bit more challenging
#define BP_STEALTH_MODIFIER		-20 // slow & cautious, not too strenuous
#define BP_MINING_MODIFIER		-30 // pretty relaxing, overall


// end-of-turn Breath Point gain/usage rates
#define BP_PER_AP_NO_EFFORT		-200 // gain breath!
#define BP_PER_AP_MIN_EFFORT		-100 // gain breath!
#define BP_PER_AP_LT_EFFORT		-50 // gain breath!
#define BP_PER_AP_MOD_EFFORT		25
#define BP_PER_AP_HVY_EFFORT		50
#define BP_PER_AP_MAX_EFFORT		100


// Breath Point values
#define BP_MOVEMENT_FLAT		5
#define BP_MOVEMENT_GRASS		10
#define BP_MOVEMENT_BUSH		20
#define BP_MOVEMENT_RUBBLE		35
#define BP_MOVEMENT_SHORE		50 // shallow wade
#define BP_MOVEMENT_LAKE		75 // deep wade
#define BP_MOVEMENT_OCEAN		100 // swimming

#define BP_CHANGE_FACING		10 // turning to face another direction

#define BP_CROUCH			10
#define BP_PRONE			10

#define BP_CLIMBROOF			500 // BPs to climb roof
#define BP_CLIMBOFFROOF		250 // BPs to climb off roof
#define BP_JUMPFENCE			200 // BPs to jump fence


/*
#define BP_MOVE_ITEM_FREE		0 // same place, pocket->pocket
#define BP_MOVE_ITEM_FAST		0 // hand, holster, ground only
#define BP_MOVE_ITEM_AVG		0 // everything else!
#define BP_MOVE_ITEM_SLOW		20 // vests, protective gear
*/
#define BP_MOVE_ITEM_FAST		0 // hand, holster, ground only
#define BP_MOVE_ITEM_SLOW		20 // vests, protective gear

#define BP_READY_KNIFE			0 // raise/lower knife
#define BP_READY_PISTOL		10 // raise/lower pistol
#define BP_READY_RIFLE			20 // raise/lower rifle
#define BP_READY_SAW			0 // raise/lower saw

#define BP_STEAL_ITEM			50 // BPs steal item

#define BP_PER_AP_AIMING		5 // breath cost while aiming
#define BP_RELOAD_GUN			20 // loading new clip/magazine

#define BP_THROW_ITEM			50 // throw grenades, fire-bombs, etc.

#define BP_START_FIRST_AID		0 // get the stuff out of medic kit
#define BP_PER_HP_FIRST_AID		-25 // gain breath for each point healed
#define BP_STOP_FIRST_AID		0 // put everything away again

#define BP_GET_HIT			200 // struck by bullet, knife, explosion
#define BP_GET_WOUNDED			50 // per pt of GUNFIRE/EXPLOSION impact
#define BP_FALL_DOWN			250 // falling down (explosion, exhaustion)
#define BP_GET_UP			50 // getting up again
#define BP_ROLL_OVER			20 // flipping from back to stomach

#define BP_OPEN_DOOR			30 // whether successful, or not (locked)
#define BP_PICKLOCK			-250 // gain breath, not very tiring...
#define BP_EXAMINE_DOOR		-250 // gain breath, not very tiring...
#define BP_BOOT_DOOR			200 // BP to boot door
#define BP_USE_CROWBAR			350 // BP to crowbar door
#define BP_UNLOCK_DOOR			50 // BP to unlock door
#define BP_EXPLODE_DOOR		-250 // BP to set explode charge on door
#define BP_UNTRAP_DOOR			150 // BP to untrap
#define BP_LOCK_DOOR			50 // BP to untrap

#define BP_USEWIRECUTTERS		200 // BP to use wirecutters


#define BP_PULL_TRIGGER		0 // for operating panic triggers

#define BP_FORCE_LID_OPEN		50 // per point of strength required
#define BP_SEARCH_CONTAINER		0 // get some breath back (was -50)

#define BP_OPEN_SAFE			-50
#define BP_READ_NOTE			-250 // reading a note's contents in inv.

#define BP_SNAKE_BATTLE		500 // when first attacked
#define BP_KILL_SNAKE			350 // when snake battle's been won

#define BP_USE_SURV_CAM		-100

#define BP_BURY_MINE			250 // involves digging & filling again
#define BP_DISARM_MINE			0 // 1/2 digging, 1/2 light effort

#define BP_FIRE_HANDGUN		25 // preatty easy, little recoil
#define BP_FIRE_RIFLE			50 // heavier, nasty recoil
#define BP_FIRE_SHOTGUN		100 // quite tiring, must be pumped up

#define BP_STAB_KNIFE			200

#define BP_TAKE_PHOTOGRAPH		0
#define BP_MERGE			50

#define BP_FALLFROMROOF		1000

#define BP_JUMP_OVER			250



#define DEFAULT_APS			20
#define DEFAULT_AIMSKILL		80

UINT8 BaseAPsToShootOrStab(INT8 bAPs, INT8 bAimSkill, OBJECTTYPE const&);

INT16 TerrainActionPoints(const SOLDIERTYPE* s, INT16 sGridno, INT8 bDir, INT8 bLevel);
INT16 ActionPointCost(const SOLDIERTYPE* s, INT16 sGridNo, INT8 bDir, UINT16 usMovementMode);
INT16 EstimateActionPointCost( SOLDIERTYPE *pSoldier, INT16 sGridNo, INT8 bDir, UINT16 usMovementMode, INT8 bPathIndex, INT8 bPathLength );

BOOLEAN EnoughPoints(const SOLDIERTYPE* pSoldier, INT16 sAPCost, INT16 sBPCost, BOOLEAN fDisplayMsg);
void DeductPoints( SOLDIERTYPE *pSoldier, INT16 sAPCost, INT16 sBPCost );
void UnusedAPsToBreath(SOLDIERTYPE *pSold);
INT16 TerrainBreathPoints(SOLDIERTYPE * pSoldier, INT16 sGridno,INT8 bDir, UINT16 usMovementMode);
UINT8 MinAPsToAttack(SOLDIERTYPE *, GridNo, bool add_turning_cost);
INT8  MinPtsToMove(const SOLDIERTYPE* pSoldier);
INT8 MinAPsToStartMovement(const SOLDIERTYPE* pSoldier, UINT16 usMovementMode);
INT8 PtsToMoveDirection(const SOLDIERTYPE* pSoldier, UINT8 bDirection);
UINT8 MinAPsToShootOrStab(SOLDIERTYPE&, GridNo, bool add_turning_cost);
BOOLEAN EnoughAmmo( SOLDIERTYPE *pSoldier, BOOLEAN fDisplay, INT8 bInvPos );
void DeductAmmo( SOLDIERTYPE *pSoldier, INT8 bInvPos );


UINT16 GetAPsToPickupItem( SOLDIERTYPE *pSoldier, UINT16 usMapPos );
UINT8 CalcTotalAPsToAttack(SOLDIERTYPE *, GridNo, bool add_turning_cost, INT8 bAimTime);
UINT8 CalcAPsToBurst(INT8 bBaseActionPoints, OBJECTTYPE const&);
UINT16 GetAPsToChangeStance(const SOLDIERTYPE* pSoldier, INT8 bDesiredHeight);

UINT16 GetAPsToLook(const SOLDIERTYPE* pSoldier);
UINT16 GetAPsToGiveItem( SOLDIERTYPE *pSoldier, UINT16 usMapPos );

BOOLEAN CheckForMercContMove( SOLDIERTYPE *pSoldier );

INT16 GetAPsToReadyWeapon(const SOLDIERTYPE* pSoldier, UINT16 usAnimState);

INT8 GetAPsToClimbRoof( SOLDIERTYPE *pSoldier, BOOLEAN fClimbDown );

INT8 GetAPsToJumpFence( const SOLDIERTYPE *pSoldier );

INT8 GetAPsToCutFence( SOLDIERTYPE *pSoldier );
INT8 GetAPsToBeginFirstAid( SOLDIERTYPE *pSoldier );
INT8 GetAPsToBeginRepair( SOLDIERTYPE *pSoldier );
INT8 GetAPsToRefuelVehicle( SOLDIERTYPE *pSoldier );

INT16 MinAPsToThrow(SOLDIERTYPE const&, GridNo, bool add_turning_cost);

UINT16 GetAPsToDropBomb( SOLDIERTYPE *pSoldier );
UINT16 GetTotalAPsToDropBomb( SOLDIERTYPE *pSoldier, INT16 sGridNo );
UINT16 GetAPsToUseRemote( SOLDIERTYPE *pSoldier );

INT8 GetAPsToStealItem( SOLDIERTYPE *pSoldier, INT16 usMapPos );

INT8 GetAPsToUseJar( SOLDIERTYPE *pSoldier, INT16 usMapPos );

INT8 GetAPsToJumpOver(const SOLDIERTYPE* pSoldier);

void GetAPChargeForShootOrStabWRTGunRaises(SOLDIERTYPE const*, GridNo, UINT8 ubAddTurningCost, BOOLEAN* charge_turning, BOOLEAN* charge_raise);

UINT16 GetAPsToReloadRobot(SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pRobot);
INT8 GetAPsToReloadGunWithAmmo( OBJECTTYPE * pGun, OBJECTTYPE * pAmmo );
INT8 GetAPsToAutoReload( SOLDIERTYPE * pSoldier );


#endif
