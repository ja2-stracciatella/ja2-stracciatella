#pragma once

// XXX
#include "game/Tactical/Weapons.h"

#include "ItemModel.h"
#include "ItemStrings.h"

#include <string_theory/string>

#include <map>
#include <stdint.h>

class JsonObject;
class JsonObjectReader;
enum SoundID;
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
			ItemId itemIndex,
			ST::string internalName,
			ST::string shortName,
			ST::string name,
			ST::string description,
			ST::string internalType);

	virtual void serializeTo(JsonObject &obj) const;

	static WeaponModel* deserialize(JsonObjectReader &obj,
	const std::map<ST::string, const CalibreModel*> &calibreMap,
	const VanillaItemStrings& vanillaItemStrings);

	virtual const WeaponModel* asWeapon() const   { return this; }

	bool matches(const CalibreModel *calibre) const;
	bool matches(const MagazineModel *mag) const;
	bool isSameMagCapacity(const MagazineModel *mag) const;

	/** Check if the given attachment can be attached to the item. */
	virtual bool canBeAttached(ItemId attachment) const;

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
	ItemId   usSmokeEffect;    // item index of the smoke effect on ammo miss

protected:
	void serializeAttachments(JsonObject &obj) const;
};

struct NoWeapon : WeaponModel
{
	NoWeapon(ItemId indexIndex, const ST::string& internalName);

	NoWeapon(ItemId itemIndex, const ST::string& internalName, uint32_t itemClass, uint8_t cursor);

	virtual void serializeTo(JsonObject &obj) const;
};

struct Pistol : WeaponModel
{
	Pistol(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};



struct MPistol : WeaponModel
{
	MPistol(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};



struct SMG : WeaponModel
{
	SMG(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};


struct SniperRifle : WeaponModel
{
	SniperRifle(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};


struct Rifle : WeaponModel
{
	Rifle(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};


struct AssaultRifle : WeaponModel
{
	AssaultRifle(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};


struct Shotgun : WeaponModel
{
	Shotgun(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};


struct LMG : WeaponModel
{
	LMG(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
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

	void serializeTo(JsonObject &obj) const override;
};


struct Blade : WeaponModel
{
	Blade(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		ST::string Sound);

	void serializeTo(JsonObject &obj) const override;
};


struct ThrowingBlade : WeaponModel
{
	ThrowingBlade(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		ST::string Sound);

	void serializeTo(JsonObject &obj) const override;
};


struct PunchWeapon : WeaponModel
{
	PunchWeapon(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint8_t AttackVolume,
		ST::string Sound);

	void serializeTo(JsonObject &obj) const override;
};


struct Launcher : WeaponModel
{
	Launcher(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound);

	void serializeTo(JsonObject &obj) const override;
};


struct LAW : WeaponModel
{
	LAW(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound);

	void serializeTo(JsonObject &obj) const override;
};


struct Cannon : WeaponModel
{
	Cannon(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound);

	void serializeTo(JsonObject &obj) const override;
};


struct MonsterSpit : WeaponModel
{
	MonsterSpit(ItemId indexIndex,
		ST::string internalName,
		ST::string shortName,
		ST::string name,
		ST::string description,
		const CalibreModel *calibre,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint8_t MagSize,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound,
		ItemId smokeEffect);

	void serializeTo(JsonObject &obj) const override;
};
