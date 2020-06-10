#ifndef __WEAPONS_H
#define __WEAPONS_H

#include "Item_Types.h"
#include "JA2Types.h"
#include "Sound_Control.h"

struct CalibreModel;


#define MAXCHANCETOHIT					(gamepolicy(chance_to_hit_maximum))
#define MINCHANCETOHIT					(gamepolicy(chance_to_hit_minimum))

#define BAD_DODGE_POSITION_PENALTY			20

#define GUN_BARREL_RANGE_BONUS				100

// Special deaths can only occur within a limited distance to the target
#define MAX_DISTANCE_FOR_MESSY_DEATH			7
// If you do a lot of damage with a close-range shot, instant kill
#define MIN_DAMAGE_FOR_INSTANT_KILL			55
// If you happen to kill someone with a close-range shot doing a lot of damage to the head, head explosion
#define MIN_DAMAGE_FOR_HEAD_EXPLOSION			45
// If you happen to kill someone with a close-range shot doing a lot of damage to the chest, chest explosion
// This value is lower than head because of the damage bonus for shooting the head
#define MIN_DAMAGE_FOR_BLOWN_AWAY			30
// If you happen to hit someone in the legs for enough damage, REGARDLESS of distance, person falls down
// Leg damage is halved for these purposes
#define MIN_DAMAGE_FOR_AUTO_FALL_OVER			20

// short range at which being prone provides to hit penalty when shooting standing people
#define MIN_PRONE_RANGE				50

// can't miss at this range?
#define POINT_BLANK_RANGE				16

#define BODY_IMPACT_ABSORPTION				20

#define BUCKSHOT_SHOTS					9

#define MIN_MORTAR_RANGE				150 // minimum range of a mortar

// WEAPON CLASSES
enum
{
	NOGUNCLASS,
	HANDGUNCLASS,
	SMGCLASS,
	RIFLECLASS,
	MGCLASS,
	SHOTGUNCLASS,
	KNIFECLASS,
	MONSTERCLASS,
	NUM_WEAPON_CLASSES
};

// ARMOUR CLASSES
enum
{
	ARMOURCLASS_HELMET,
	ARMOURCLASS_VEST,
	ARMOURCLASS_LEGGINGS,
	ARMOURCLASS_PLATE,
	ARMOURCLASS_MONST,
	ARMOURCLASS_VEHICLE
};

enum
{
	AMMO_REGULAR = 0,
	AMMO_HP,
	AMMO_AP,
	AMMO_SUPER_AP,
	AMMO_BUCKSHOT,
	AMMO_FLECHETTE,
	AMMO_GRENADE,
	AMMO_MONSTER,
	AMMO_KNIFE,
	AMMO_HE,
	AMMO_HEAT,
	AMMO_SLEEP_DART,
	AMMO_FLAME,
};

enum
{
	EXPLOSV_NORMAL,
	EXPLOSV_STUN,
	EXPLOSV_TEARGAS,
	EXPLOSV_MUSTGAS,
	EXPLOSV_FLARE,
	EXPLOSV_NOISE,
	EXPLOSV_SMOKE,
	EXPLOSV_CREATUREGAS,
};

#define AMMO_DAMAGE_ADJUSTMENT_BUCKSHOT( x )	(x / 4)
#define NUM_BUCKSHOT_PELLETS			9

// hollow point bullets do lots of damage to people
#define AMMO_DAMAGE_ADJUSTMENT_HP( x )		( (x * 17) / 10 )
// but they SUCK at penetrating armour
#define AMMO_ARMOUR_ADJUSTMENT_HP( x )		( (x * 3) / 2 )
// armour piercing bullets are good at penetrating armour
#define AMMO_ARMOUR_ADJUSTMENT_AP( x )		((x * 3) / 4)
// "super" AP bullets are great at penetrating armour
#define AMMO_ARMOUR_ADJUSTMENT_SAP( x )	(x / 2)

// high explosive damage value (PRIOR to armour subtraction)
#define AMMO_DAMAGE_ADJUSTMENT_HE( x )		((x * 4) / 3)

// but they SUCK at penetrating armour
#define AMMO_STRUCTURE_ADJUSTMENT_HP( x )	(x * 2)
// armour piercing bullets are good at penetrating structure
#define AMMO_STRUCTURE_ADJUSTMENT_AP( x )	( (x * 3 ) / 4)
// "super" AP bullets are great at penetrating structures
#define AMMO_STRUCTURE_ADJUSTMENT_SAP( x )	( x / 2 )

// one quarter of punching damage is "real" rather than breath damage
#define PUNCH_REAL_DAMAGE_PORTION		4

#define AIM_BONUS_SAME_TARGET			10 // chance-to-hit bonus (in %)
#define AIM_BONUS_PER_AP			10 // chance-to-hit bonus (in %) for aim
#define AIM_BONUS_CROUCHING			10
#define AIM_BONUS_PRONE				20
#define AIM_BONUS_TWO_HANDED_PISTOL		5
#define AIM_BONUS_FIRING_DOWN			15
#define AIM_PENALTY_ONE_HANDED_PISTOL		5
#define AIM_PENALTY_DUAL_PISTOLS		20
#define AIM_PENALTY_SMG				5
#define AIM_PENALTY_GASSED			50
#define AIM_PENALTY_GETTINGAID			20
#define AIM_PENALTY_PER_SHOCK			5      // 5% penalty per point of shock
#define AIM_BONUS_TARGET_HATED			20
#define AIM_BONUS_PSYCHO			15
#define AIM_PENALTY_TARGET_BUDDY		20
#define AIM_PENALTY_BURSTING			10
#define AIM_PENALTY_GENTLEMAN			15
#define AIM_PENALTY_TARGET_CROUCHED		20
#define AIM_PENALTY_TARGET_PRONE		40
#define AIM_PENALTY_BLIND			80
#define AIM_PENALTY_FIRING_UP			25

struct ARMOURTYPE
{
	UINT8 ubArmourClass;
	UINT8 ubProtection;
	UINT8 ubDegradePercent;
};

struct EXPLOSIVETYPE
{
	UINT8 ubType; // type of explosive
	UINT8 ubDamage; // damage value
	UINT8 ubStunDamage; // stun amount / 100
	UINT8 ubRadius; // radius of effect
	UINT8 ubVolume; // sound radius of explosion
	UINT8 ubVolatility; // maximum chance of accidental explosion
	UINT8 ubAnimationID; // Animation enum to use
};

//GLOBALS

extern ARMOURTYPE    const Armour[];
extern EXPLOSIVETYPE const Explosive[];

INT8 EffectiveArmour(const OBJECTTYPE* pObj);
extern INT8 ArmourVersusExplosivesPercent( SOLDIERTYPE * pSoldier );
extern BOOLEAN FireWeapon( SOLDIERTYPE *pSoldier , INT16 sTargetGridNo );
void WeaponHit(SOLDIERTYPE* target, UINT16 usWeaponIndex, INT16 sDamage, INT16 sBreathLoss, UINT16 usDirection, INT16 sXPos, INT16 sYPos, INT16 sZPos, INT16 sRange, SOLDIERTYPE* attacker, UINT8 ubSpecial, UINT8 ubHitLocation);
void StructureHit(BULLET* b, INT16 sXPos, INT16 sYPos, INT16 sZPos, UINT16 usStructureID, INT32 iImpact, BOOLEAN fStopped);
extern void WindowHit( INT16 sGridNo, UINT16 usStructureID, BOOLEAN fBlowWindowSouth, BOOLEAN fLargeForce );
extern INT32 BulletImpact( SOLDIERTYPE *pFirer, SOLDIERTYPE * pTarget, UINT8 ubHitLocation, INT32 iImpact, INT16 sHitBy, UINT8 * pubSpecial );
BOOLEAN InRange(const SOLDIERTYPE* pSoldier, INT16 sGridNo);
void ShotMiss(const BULLET* b);
extern UINT32 CalcChanceToHitGun(SOLDIERTYPE *pSoldier, UINT16 sGridNo, UINT8 ubAimTime, UINT8 ubAimPos, BOOLEAN fModify);
extern UINT32 AICalcChanceToHitGun(SOLDIERTYPE *pSoldier, UINT16 sGridNo, UINT8 ubAimTime, UINT8 ubAimPos );
extern UINT32 CalcChanceToPunch(SOLDIERTYPE *pAttacker, SOLDIERTYPE * pDefender, UINT8 ubAimTime);
extern UINT32 CalcChanceToStab(SOLDIERTYPE * pAttacker,SOLDIERTYPE *pDefender, UINT8 ubAimTime);
void ReloadWeapon(SOLDIERTYPE*, UINT8 inv_pos);
bool IsGunBurstCapable(SOLDIERTYPE const*, UINT8 inv_pos);
extern INT32 CalcBodyImpactReduction( UINT8 ubAmmoType, UINT8 ubHitLocation );
INT32 TotalArmourProtection(SOLDIERTYPE&, UINT8 ubHitLocation, INT32 iImpact, UINT8 ubAmmoType);
INT8 ArmourPercent(const SOLDIERTYPE* pSoldier);

extern void GetTargetWorldPositions( SOLDIERTYPE *pSoldier, INT16 sTargetGridNo, FLOAT *pdXPos, FLOAT *pdYPos, FLOAT *pdZPos );

extern BOOLEAN	OKFireWeapon( SOLDIERTYPE *pSoldier );
extern BOOLEAN CheckForGunJam( SOLDIERTYPE * pSoldier );

INT32 CalcMaxTossRange(const SOLDIERTYPE* pSoldier, UINT16 usItem, BOOLEAN fArmed);
extern UINT32 CalcThrownChanceToHit(SOLDIERTYPE *pSoldier, INT16 sGridNo, UINT8 ubAimTime, UINT8 ubAimPos );

extern void ChangeWeaponMode( SOLDIERTYPE * pSoldier );

void UseHandToHand(SOLDIERTYPE* pSoldier, INT16 sTargetGridNo, BOOLEAN fStealing);

void DishoutQueenSwipeDamage( SOLDIERTYPE *pQueenSoldier );

INT32 HTHImpact(const SOLDIERTYPE* pSoldier, const SOLDIERTYPE* pTarget, INT32 iHitBy, BOOLEAN fBladeAttack);

UINT16 GunRange(OBJECTTYPE const&);

extern BOOLEAN gfNextFireJam;
extern BOOLEAN gfNextShotKills;
extern BOOLEAN gfReportHitChances;

#endif
