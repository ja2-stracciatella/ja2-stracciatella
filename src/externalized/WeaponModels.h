#pragma once

#include "ItemModel.h"
#include "ItemStrings.h"
#include "Sound_Control.h"

#include <string_theory/string>

#include <map>
#include <stdint.h>

enum SoundID;
class ExplosiveCalibreModel;
struct CalibreModel;
struct MagazineModel;

#define NO_WEAPON_SOUND ((SoundID)-1)
#define NO_WEAPON_SOUND_STR ("")

#define WEAPON_TYPE_NOWEAPON ("NOWEAPON")
#define WEAPON_TYPE_PUNCH ("PUNCH")
#define WEAPON_TYPE_THROWN ("THROWN")

struct WeaponModel : ItemModel
{
	WeaponModel(uint32_t itemClass,
			uint8_t weaponType,
			uint8_t cursor,
			uint16_t itemIndex,
			ST::string & internalName);

	virtual JsonValue serialize() const;
	virtual void serializeTo(JsonObject &obj) const;

	static WeaponModel* deserialize(const JsonValue &json,
	const std::map<ST::string, const CalibreModel*> &calibreMap,
	const std::vector<const ExplosiveCalibreModel*> &explosiveCalibres,
	const BinaryData& vanillaItemStrings);

	virtual const WeaponModel* asWeapon() const   { return this; }

	bool shootsExplosiveCalibre() const;
	bool matches(const CalibreModel *calibre) const;
	bool matches(const MagazineModel *mag) const;
	bool matches(const ExplosiveCalibreModel *explosiveCalibre) const;
	bool isSameMagCapacity(const MagazineModel *mag) const;

	/** Check if the given attachment can be attached to the item. */
	virtual bool canBeAttached(uint16_t attachment) const;

	/** Get standard replacement gun name. */
	virtual const ST::string & getStandardReplacement() const;

	int getRateOfFire() const;

	ST::string sound;
	ST::string burstSound;
	ST::string silencedSound;
	ST::string silencedBurstSound;
	ST::string standardReplacement;
	bool attachSilencer;
	bool attachSniperScope;
	bool attachLaserScope;
	bool attachBipod;
	bool attachDuckbill;
	bool attachUnderGLauncher;
	bool attachSpringAndBoltUpgrade;
	bool attachGunBarrelExtender;
	int m_rateOfFire;

	ST::string internalType;
	UINT8    ubWeaponClass;    // handgun/shotgun/rifle/knife
	UINT8    ubWeaponType;     // exact type (for display purposes)
	const CalibreModel *calibre;  // type of ammunition needed
	const ExplosiveCalibreModel *explosiveCalibre;  // type of ammunition needed
	UINT8    ubReadyTime;      // APs to ready/unready weapon
	UINT8    ubShotsPer4Turns; // maximum (mechanical) firing rate
	UINT8    ubShotsPerBurst;
	UINT8    ubBurstPenalty;   // % penalty per shot after first
	UINT8    ubBulletSpeed;    // bullet's travelling speed
	UINT8    ubImpact;         // weapon's max damage impact (size & speed)
	UINT8    ubDeadliness;     // comparative ratings of guns
	UINT8    ubMagSize;
	UINT16   usRange;
	UINT16   usReloadDelay;
	UINT8    ubAttackVolume;
	UINT8    ubHitVolume;
	SoundID  sReloadSound;
	SoundID  sLocknLoadSound;
	UINT16   usSmokeEffect;    // item index of the smoke effect on ammo miss

protected:
	void serializeAttachments(JsonObject &obj) const;
};

struct NoWeapon : WeaponModel
{
	NoWeapon(uint16_t indexIndex, ST::string & internalName);

	NoWeapon(uint16_t itemIndex, ST::string & internalName, uint32_t itemClass, uint8_t cursor);

	virtual JsonValue serialize() const;
};

struct Pistol : WeaponModel
{
	Pistol(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string SilencedSound);

	JsonValue serialize() const override;
};



struct MPistol : WeaponModel
{
	MPistol(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t ShotsPerBurst,
		uint8_t BurstPenalty,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string BurstSound,
		ST::string SilencedSound,
		ST::string SilencedBurstSound);

	JsonValue serialize() const override;
};



struct SMG : WeaponModel
{
	SMG(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t ShotsPerBurst,
		uint8_t BurstPenalty,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string BurstSound,
		ST::string SilencedSound,
		ST::string SilencedBurstSound);

	JsonValue serialize() const override;
};


struct SniperRifle : WeaponModel
{
	SniperRifle(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string SilencedSound);

	JsonValue serialize() const override;
};


struct Rifle : WeaponModel
{
	Rifle(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string SilencedSound);

	JsonValue serialize() const override;
};


struct AssaultRifle : WeaponModel
{
	AssaultRifle(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t ShotsPerBurst,
		uint8_t BurstPenalty,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string BurstSound,
		ST::string SilencedSound,
		ST::string SilencedBurstSound);

	JsonValue serialize() const override;
};


struct Shotgun : WeaponModel
{
	Shotgun(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t ShotsPerBurst,
		uint8_t BurstPenalty,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string BurstSound,
		ST::string SilencedSound,
		ST::string SilencedBurstSound);

	JsonValue serialize() const override;
};


struct LMG : WeaponModel
{
	LMG(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t BulletSpeed,
		uint8_t Impact,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t ShotsPerBurst,
		uint8_t BurstPenalty,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ST::string BurstSound,
		ST::string SilencedSound,
		ST::string SilencedBurstSound);

	JsonValue serialize() const override;
};


struct Blade : WeaponModel
{
	Blade(uint16_t indexIndex,
		ST::string & internalName,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		ST::string Sound);

	JsonValue serialize() const override;
};


struct ThrowingBlade : WeaponModel
{
	ThrowingBlade(uint16_t indexIndex,
		ST::string & internalName,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		ST::string Sound);

	JsonValue serialize() const override;
};


struct PunchWeapon : WeaponModel
{
	PunchWeapon(uint16_t indexIndex,
		ST::string & internalName,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint8_t AttackVolume,
		ST::string Sound);

	JsonValue serialize() const override;
};


struct Launcher : WeaponModel
{
	Launcher(uint16_t indexIndex,
		ST::string & internalName,
		const ExplosiveCalibreModel* explosiveCalibre,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound);

	JsonValue serialize() const override;
};


struct LAW : WeaponModel
{
	LAW(uint16_t indexIndex,
		ST::string & internalName,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound);

	JsonValue serialize() const override;
};


struct Cannon : WeaponModel
{
	Cannon(uint16_t indexIndex,
		ST::string & internalName,
		const ExplosiveCalibreModel* explosiveCalibre,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound);

	JsonValue serialize() const override;
};


struct MonsterSpit : WeaponModel
{
	MonsterSpit(uint16_t indexIndex,
		ST::string & internalName,
		const CalibreModel *calibre,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		uint16_t smokeEffect);

	JsonValue serialize() const override;
};
