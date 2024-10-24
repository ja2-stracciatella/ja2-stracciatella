#include "WeaponModels.h"
#include "CalibreModel.h"
#include "Exceptions.h"
#include "ExplosiveCalibreModel.h"
#include "ItemModel.h"
#include "Logger.h"
#include "MagazineModel.h"
#include "Points.h"
#include "Sound_Control.h"
#include "Weapons.h"
#include <utility>

// exact gun types
// used as an index in WeaponType[] string array
enum
{
	NOT_GUN = 0,
	GUN_PISTOL,
	GUN_M_PISTOL,
	GUN_SMG,
	GUN_RIFLE,
	GUN_SN_RIFLE,
	GUN_AS_RIFLE,
	GUN_LMG,
	GUN_SHOTGUN
};

WeaponModel::WeaponModel(uint32_t itemClass, uint8_t weaponType, uint8_t cursor, uint16_t itemIndex, ST::string&& internalName_)
	:ItemModel(itemIndex, std::move(internalName_), itemClass, itemIndex, (ItemCursor)cursor),
	sound(NO_WEAPON_SOUND_STR),
	burstSound(NO_WEAPON_SOUND_STR),
	attachSilencer(false),
	attachSniperScope(false),
	attachLaserScope(false),
	attachBipod(false),
	attachDuckbill(false),
	attachUnderGLauncher(false),
	attachSpringAndBoltUpgrade(false),
	attachGunBarrelExtender(false),
	m_rateOfFire(0)
{
	ubWeaponType         = weaponType;
	ubWeaponClass        = NOGUNCLASS;
	calibre              = CalibreModel::getNoCalibreObject();
	ubReadyTime          = 0;
	ubShotsPer4Turns     = 0;
	ubShotsPerBurst      = 0;
	ubBurstPenalty       = 0;
	ubBulletSpeed        = 0;
	ubImpact             = 0;
	ubDeadliness         = 0;
	ubMagSize            = 0;
	usRange              = 0;
	usReloadDelay        = 0;
	ubAttackVolume       = 0;
	ubHitVolume          = 0;
	sReloadSound         = NO_WEAPON_SOUND;
	sLocknLoadSound      = NO_WEAPON_SOUND;
	usSmokeEffect        = NONE;
	explosiveCalibre     = nullptr;
}

void WeaponModel::serializeTo(JsonObject &obj) const
{
	obj.set("itemIndex",            itemIndex);
	obj.set("internalName",         internalName);
	obj.set("internalType",         internalType);
	obj.set("inventoryGraphics",    inventoryGraphics.serialize());
	obj.set("tileGraphic", tileGraphic.serialize());
	obj.set("ubWeight", getWeight());
	obj.set("ubPerPocket", getPerPocket());
	obj.set("usPrice", getPrice());
	obj.set("ubCoolness", getCoolness());
	obj.set("bReliability", getReliability());
	obj.set("bRepairEase", getRepairEase());
	obj.set("rateOfFire", m_rateOfFire);

	if(isInBigGunList())
	{
		obj.set("standardReplacement", standardReplacement);
	}
}

JsonValue WeaponModel::serialize() const {
	JsonObject obj;
	serializeTo(obj);
	return obj.toValue();
}

void addOptionalBool(JsonObject &obj, const char* key, bool val) {
	if (val) {
		obj.set(key, val);
	}
}

void WeaponModel::serializeAttachments(JsonObject &obj) const
{
	addOptionalBool(obj, "attachment_Silencer",                 attachSilencer);
	addOptionalBool(obj, "attachment_SniperScope",              attachSniperScope);
	addOptionalBool(obj, "attachment_LaserScope",               attachLaserScope);
	addOptionalBool(obj, "attachment_Bipod",                    attachBipod);
	addOptionalBool(obj, "attachment_Duckbill",                 attachDuckbill);
	addOptionalBool(obj, "attachment_UnderGLauncher",           attachUnderGLauncher);
	addOptionalBool(obj, "attachment_SpringAndBoltUpgrade",     attachSpringAndBoltUpgrade);
	addOptionalBool(obj, "attachment_GunBarrelExtender",        attachGunBarrelExtender);
}

ST::string readOptionalString(JsonObject &obj, const char* key, const ST::string &default_value) {
	ST::string sound = obj.getOptionalString(key);
	if (sound.empty()) {
		return default_value;
	}
	return sound;
}

const ExplosiveCalibreModel* readExplosiveCalibreWithFallbacks(const ST::string& internalName, JsonObject &obj, const std::vector<const ExplosiveCalibreModel*> &explosiveCalibres, std::map<ST::string, ST::string> fallback) {
	auto calibreStr = obj.getOptionalString("calibre");

	if (calibreStr == "") {
		SLOGW("Missing property `calibre` for launcher weapon '{}'. This is deprectated, using vanilla default.", internalName);

		auto fallbackIt = fallback.find(internalName);
		if (fallbackIt == fallback.end()) {
			throw DataError(ST::format("Could not determine fallback explosive calibre for launcher weapon '{}'", internalName));
		}

		calibreStr = (*fallbackIt).second;
	}

	auto calibreIt = std::find_if(explosiveCalibres.begin(), explosiveCalibres.end(), [&calibreStr](const ExplosiveCalibreModel* item) -> bool {
		return item->getName() == calibreStr;
	});
	if (calibreIt == explosiveCalibres.end()) {
		throw DataError(ST::format("Could not find explosive calibre '{}' for launcher weapon '{}'", calibreStr, internalName));
	}

	return *calibreIt;
}

WeaponModel* WeaponModel::deserialize(const JsonValue &json,
					const std::map<ST::string, const CalibreModel*> &calibreMap,
					const std::vector<const ExplosiveCalibreModel*> &explosiveCalibres,
					const VanillaItemStrings& vanillaItemStrings)
{
	auto obj = json.toObject();
	ItemModel::InitData const initData{ obj, vanillaItemStrings };
	WeaponModel *wep = NULL;
	int itemIndex = obj.GetInt("itemIndex");
	ST::string internalName = obj.GetString("internalName");
	ST::string internalType = obj.GetString("internalType");

	if (internalType == WEAPON_TYPE_NOWEAPON)
	{
		wep = new NoWeapon(itemIndex, std::move(internalName));
	}
	else if (internalType == WEAPON_TYPE_PUNCH)
	{
		wep = new NoWeapon(itemIndex, std::move(internalName), IC_PUNCH, PUNCHCURS);
		internalType = WEAPON_TYPE_NOWEAPON;
	}
	else if (internalType == WEAPON_TYPE_THROWN)
	{
		wep = new NoWeapon(itemIndex, std::move(internalName), IC_THROWN, TOSSCURS);
		internalType = WEAPON_TYPE_NOWEAPON;
	}
	else if (internalType == "PISTOL")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);

		wep = new Pistol(itemIndex, std::move(internalName),
					calibre,
					BulletSpeed,
					Impact,
					ReadyTime,
					ShotsPer4Turns,
					Deadliness,
					MagSize,
					Range,
					AttackVolume,
					HitVolume,
					sound,
					silencedSound);
	}
	else if (internalType == "M_PISTOL")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
		uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		// uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto burstSound = readOptionalString(obj, "burstSound", calibre->burstSound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);
		auto silencedBurstSound = readOptionalString(obj, "silencedBurstSound", calibre->silencedBurstSound);

		wep = new MPistol(itemIndex, std::move(internalName),
					calibre,
					BulletSpeed,
					Impact,
					ReadyTime,
					ShotsPer4Turns,
					ShotsPerBurst,
					BurstPenalty,
					Deadliness,
					MagSize,
					Range,
					AttackVolume,
					HitVolume,
					sound,
					burstSound,
					silencedSound,
					silencedBurstSound);
	}
	else if (internalType == "SMG")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
		uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		// uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto burstSound = readOptionalString(obj, "burstSound", calibre->burstSound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);
		auto silencedBurstSound = readOptionalString(obj, "silencedBurstSound", calibre->silencedBurstSound);

		wep = new SMG(itemIndex, std::move(internalName),
				calibre,
				BulletSpeed,
				Impact,
				ReadyTime,
				ShotsPer4Turns,
				ShotsPerBurst,
				BurstPenalty,
				Deadliness,
				MagSize,
				Range,
				AttackVolume,
				HitVolume,
				sound,
				burstSound,
				silencedSound,
				silencedBurstSound);
	}
	else if (internalType == "SN_RIFLE")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		// uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
		// uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		// uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);

		wep = new SniperRifle(itemIndex, std::move(internalName),
					calibre,
					BulletSpeed,
					Impact,
					ReadyTime,
					ShotsPer4Turns,
					Deadliness,
					MagSize,
					Range,
					AttackVolume,
					HitVolume,
					sound,
					silencedSound);
	}
	else if(internalType == "RIFLE")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		// uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
		// uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		// uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);

		wep = new Rifle(itemIndex, std::move(internalName),
				calibre,
				BulletSpeed,
				Impact,
				ReadyTime,
				ShotsPer4Turns,
				Deadliness,
				MagSize,
				Range,
				AttackVolume,
				HitVolume,
				sound,
				silencedSound);
	}
	else if (internalType == "ASRIFLE")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
		uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		// uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto burstSound = readOptionalString(obj, "burstSound", calibre->burstSound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);
		auto silencedBurstSound = readOptionalString(obj, "silencedBurstSound", calibre->silencedBurstSound);

		wep = new AssaultRifle(itemIndex, std::move(internalName),
					calibre,
					BulletSpeed,
					Impact,
					ReadyTime,
					ShotsPer4Turns,
					ShotsPerBurst,
					BurstPenalty,
					Deadliness,
					MagSize,
					Range,
					AttackVolume,
					HitVolume,
					sound,
					burstSound,
					silencedSound,
					silencedBurstSound);
	}
	else if (internalType == "SHOTGUN")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
		uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		// uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto burstSound = readOptionalString(obj, "burstSound", calibre->burstSound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);
		auto silencedBurstSound = readOptionalString(obj, "silencedBurstSound", calibre->silencedBurstSound);

		wep = new Shotgun(itemIndex, std::move(internalName),
					calibre,
					BulletSpeed,
					Impact,
					ReadyTime,
					ShotsPer4Turns,
					ShotsPerBurst,
					BurstPenalty,
					Deadliness,
					MagSize,
					Range,
					AttackVolume,
					HitVolume,
					sound,
					burstSound,
					silencedSound,
					silencedBurstSound);
	}
	else if (internalType == "LMG")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
		uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		// uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		auto sound = readOptionalString(obj, "sound", calibre->sound);
		auto burstSound = readOptionalString(obj, "burstSound", calibre->burstSound);
		auto silencedSound = readOptionalString(obj, "silencedSound", calibre->silencedSound);
		auto silencedBurstSound = readOptionalString(obj, "silencedBurstSound", calibre->silencedBurstSound);
		wep = new LMG(itemIndex, std::move(internalName),
				calibre,
				BulletSpeed,
				Impact,
				ReadyTime,
				ShotsPer4Turns,
				ShotsPerBurst,
				BurstPenalty,
				Deadliness,
				MagSize,
				Range,
				AttackVolume,
				HitVolume,
				sound,
				burstSound,
				silencedSound,
				silencedBurstSound);
	}
	else if (internalType == "BLADE")
	{
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		ST::string Sound       = obj.getOptionalString("sound");
		wep = new Blade(itemIndex, std::move(internalName),
				Impact,
				ShotsPer4Turns,
				Deadliness,
				Range,
				AttackVolume,
				Sound);
	}
	else if (internalType == "THROWINGBLADE")
	{
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		ST::string Sound         = obj.getOptionalString("sound");
		wep = new ThrowingBlade(itemIndex, std::move(internalName),
					Impact,
					ShotsPer4Turns,
					Deadliness,
					Range,
					AttackVolume,
					Sound);
	}
	else if (internalType == "PUNCHWEAPON")
	{
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		ST::string Sound         = obj.getOptionalString("sound");
		wep = new PunchWeapon(itemIndex, std::move(internalName),
					Impact,
					ShotsPer4Turns,
					Deadliness,
					AttackVolume,
					Sound);
	}
	else if (internalType == "LAUNCHER")
	{
		auto calibre = readExplosiveCalibreWithFallbacks(internalName, obj, explosiveCalibres, {
			{"GLAUNCHER", "40MM_GRENADE"},
			{"UNDER_GLAUNCHER", "40MM_GRENADE"},
			{"MORTAR", "MORTAR_SHELL"}
		});
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		ST::string Sound         = obj.getOptionalString("sound");
		wep = new Launcher(itemIndex, std::move(internalName),
					calibre,
					BulletSpeed,
					ReadyTime,
					ShotsPer4Turns,
					Deadliness,
					Range,
					AttackVolume,
					HitVolume,
					Sound);
	}
	else if (internalType == "LAW")
	{
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		ST::string Sound       = obj.getOptionalString("sound");
		wep = new LAW(itemIndex, std::move(internalName),
				BulletSpeed,
				ReadyTime,
				ShotsPer4Turns,
				Deadliness,
				Range,
				AttackVolume,
				HitVolume,
				Sound);
	}
	else if (internalType == "CANNON")
	{
		auto calibre = readExplosiveCalibreWithFallbacks(internalName, obj, explosiveCalibres, {
			{"TANK_CANNON", "TANK_SHELL"},
		});
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		ST::string Sound       = obj.getOptionalString("sound");
		wep = new Cannon(itemIndex, std::move(internalName),
					calibre,
					BulletSpeed,
					ReadyTime,
					ShotsPer4Turns,
					Deadliness,
					Range,
					AttackVolume,
					HitVolume,
					Sound);
	}
	else if (internalType == "MONSTSPIT")
	{
		const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint8_t  MagSize         = obj.GetInt("ubMagSize");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		ST::string Sound         = obj.getOptionalString("sound");
		uint16_t smokeEffect     = obj.GetInt("usSmokeEffect");
		wep = new MonsterSpit(itemIndex, std::move(internalName),
					calibre,
					Impact,
					ShotsPer4Turns,
					Deadliness,
					MagSize,
					Range,
					AttackVolume,
					HitVolume,
					Sound,
					smokeEffect);
	}

	if(!wep)
	{
		SLOGE("Weapon type '{}' is not found", internalType);
		return wep;
	}

	wep->shortName    = ItemModel::deserializeShortName(initData);
	wep->name         = ItemModel::deserializeName(initData);
	wep->description  = ItemModel::deserializeDescription(initData);
	wep->internalType = std::move(internalType);

	const auto inventoryGraphics = InventoryGraphicsModel::deserialize(obj["inventoryGraphics"]);
	const auto tileGraphic = TilesetTileIndexModel::deserialize(obj["tileGraphic"]);

	wep->inventoryGraphics  = inventoryGraphics;
	wep->tileGraphic = tileGraphic;

	wep->ubWeight         = obj.GetInt("ubWeight");
	wep->ubPerPocket      = obj.GetInt("ubPerPocket");
	wep->usPrice          = obj.GetInt("usPrice");
	wep->ubCoolness       = obj.GetInt("ubCoolness");
	wep->bReliability     = obj.GetInt("bReliability");
	wep->bRepairEase      = obj.GetInt("bRepairEase");
	wep->m_rateOfFire     = obj.GetInt("rateOfFire");

	wep->attachSilencer               = obj.getOptionalBool("attachment_Silencer");
	wep->attachSniperScope            = obj.getOptionalBool("attachment_SniperScope");
	wep->attachLaserScope             = obj.getOptionalBool("attachment_LaserScope");
	wep->attachBipod                  = obj.getOptionalBool("attachment_Bipod");
	wep->attachDuckbill               = obj.getOptionalBool("attachment_Duckbill");
	wep->attachUnderGLauncher         = obj.getOptionalBool("attachment_UnderGLauncher");
	wep->attachSpringAndBoltUpgrade   = obj.getOptionalBool("attachment_SpringAndBoltUpgrade");
	wep->attachGunBarrelExtender      = obj.getOptionalBool("attachment_GunBarrelExtender");

	wep->fFlags |= ItemModel::deserializeFlags(obj);

	ST::string replacement = obj.getOptionalString("standardReplacement");
	if (!replacement.empty())
	{
		wep->standardReplacement = replacement;
	}

	return wep;
}

bool WeaponModel::shootsExplosiveCalibre() const {
	return this->explosiveCalibre;
}

bool WeaponModel::matches(const CalibreModel *calibre) const
{
	return this->calibre->index == calibre->index;
}

bool WeaponModel::matches(const MagazineModel *mag) const
{
	return (this->calibre->index == mag->calibre->index)
		&& (ubMagSize == mag->capacity);
}

bool WeaponModel::matches(const ExplosiveCalibreModel *calibre) const
{
	if (!this->shootsExplosiveCalibre()) {
		return false;
	}
	return (this->explosiveCalibre->getID() == calibre->getID());
}

bool WeaponModel::isSameMagCapacity(const MagazineModel *mag) const
{
	return ubMagSize == mag->capacity;
}

/** Check if the given attachment can be attached to the item. */
bool WeaponModel::canBeAttached(uint16_t attachment) const
{
	return (attachSilencer && (attachment == SILENCER))
		|| (attachSniperScope && (attachment == SNIPERSCOPE))
		|| (attachLaserScope && (attachment == LASERSCOPE))
		|| (attachBipod && (attachment == BIPOD))
		|| (attachDuckbill && (attachment == DUCKBILL))
		|| (attachUnderGLauncher && (attachment == UNDER_GLAUNCHER))
		|| (attachSpringAndBoltUpgrade && (attachment == SPRING_AND_BOLT_UPGRADE))
		|| (attachGunBarrelExtender && (attachment == GUN_BARREL_EXTENDER));
}

/** Get standard replacement gun name. */
const ST::string & WeaponModel::getStandardReplacement() const
{
	return standardReplacement;
}

int WeaponModel::getRateOfFire() const
{
	return m_rateOfFire;
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

NoWeapon::NoWeapon(uint16_t itemIndex, ST::string&& internalName)
	:NoWeapon(itemIndex, std::move(internalName), IC_NONE, INVALIDCURS)
{
}

NoWeapon::NoWeapon(uint16_t itemIndex, ST::string&& internalName, uint32_t itemClass, uint8_t cursor)
	: WeaponModel(itemClass, NOT_GUN, cursor, itemIndex, std::move(internalName))
{
}

JsonValue NoWeapon::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("usRange", usRange);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}


Pistol::Pistol(uint16_t itemIndex,
		ST::string&& internalName,
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
		ST::string sound,
		ST::string silencedSound)
	: WeaponModel(IC_GUN, GUN_PISTOL, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = HANDGUNCLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->silencedSound  = silencedSound;
	sReloadSound         = S_RELOAD_PISTOL;
	sLocknLoadSound      = S_LNL_PISTOL;
}

JsonValue Pistol::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("silencedSound",        silencedSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

MPistol::MPistol(uint16_t itemIndex,
			ST::string&& internalName,
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
			ST::string sound,
			ST::string burstSound,
			ST::string silencedSound,
			ST::string silencedBurstSound)
	:WeaponModel(IC_GUN, GUN_M_PISTOL, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = HANDGUNCLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubShotsPerBurst      = ShotsPerBurst;
	ubBurstPenalty       = BurstPenalty;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->burstSound     = burstSound;
	this->silencedSound  = silencedSound;
	this->silencedBurstSound = silencedBurstSound;
	sReloadSound         = S_RELOAD_PISTOL;
	sLocknLoadSound      = S_LNL_PISTOL;
}

JsonValue MPistol::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubShotsPerBurst",      ubShotsPerBurst);
	obj.set("ubBurstPenalty",       ubBurstPenalty);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("burstSound",           burstSound);
	obj.set("silencedSound",        silencedSound);
	obj.set("silencedBurstSound",   silencedBurstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

SMG::SMG(uint16_t itemIndex, ST::string&& internalName,
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
		ST::string sound,
		ST::string burstSound,
		ST::string silencedSound,
		ST::string silencedBurstSound)
	:WeaponModel(IC_GUN, GUN_SMG, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = SMGCLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubShotsPerBurst      = ShotsPerBurst;
	ubBurstPenalty       = BurstPenalty;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->burstSound     = burstSound;
	this->silencedSound  = silencedSound;
	this->silencedBurstSound = silencedBurstSound;
	sReloadSound         = S_RELOAD_SMG;
	sLocknLoadSound      = S_LNL_SMG;
}

JsonValue SMG::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubShotsPerBurst",      ubShotsPerBurst);
	obj.set("ubBurstPenalty",       ubBurstPenalty);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("burstSound",           burstSound);
	obj.set("silencedSound",        silencedSound);
	obj.set("silencedBurstSound",   silencedBurstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

SniperRifle::SniperRifle(uint16_t itemIndex, ST::string&& internalName,
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
				ST::string sound,
				ST::string silencedSound)
	:WeaponModel(IC_GUN, GUN_SN_RIFLE, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = RIFLECLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->silencedSound  = silencedSound;
	sReloadSound         = S_RELOAD_RIFLE;
	sLocknLoadSound      = S_LNL_RIFLE;
}

JsonValue SniperRifle::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("silencedSound",        silencedSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

Rifle::Rifle(uint16_t itemIndex, ST::string&& internalName,
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
		ST::string sound,
		ST::string silencedSound)
	:WeaponModel(IC_GUN, GUN_RIFLE, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = RIFLECLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->silencedSound  = silencedSound;
	sReloadSound         = S_RELOAD_RIFLE;
	sLocknLoadSound      = S_LNL_RIFLE;
}

JsonValue Rifle::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("silencedSound",        silencedSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

AssaultRifle::AssaultRifle(uint16_t itemIndex, ST::string&& internalName,
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
				ST::string sound,
				ST::string burstSound,
				ST::string silencedSound,
				ST::string silencedBurstSound)
	:WeaponModel(IC_GUN, GUN_AS_RIFLE, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = RIFLECLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubShotsPerBurst      = ShotsPerBurst;
	ubBurstPenalty       = BurstPenalty;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->burstSound     = burstSound;
	this->silencedSound  = silencedSound;
	this->silencedBurstSound = silencedBurstSound;
	sReloadSound         = S_RELOAD_RIFLE;
	sLocknLoadSound      = S_LNL_RIFLE;
}

JsonValue AssaultRifle::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubShotsPerBurst",      ubShotsPerBurst);
	obj.set("ubBurstPenalty",       ubBurstPenalty);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("burstSound",           burstSound);
	obj.set("silencedSound",        silencedSound);
	obj.set("silencedBurstSound",   silencedBurstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

Shotgun::Shotgun(uint16_t itemIndex, ST::string&& internalName,
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
			ST::string sound,
			ST::string burstSound,
			ST::string silencedSound,
			ST::string silencedBurstSound)
	:WeaponModel(IC_GUN, GUN_SHOTGUN, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = SHOTGUNCLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubShotsPerBurst      = ShotsPerBurst;
	ubBurstPenalty       = BurstPenalty;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->burstSound     = burstSound;
	this->silencedSound  = silencedSound;
	this->silencedBurstSound = silencedBurstSound;
	sReloadSound         = S_RELOAD_SHOTGUN;
	sLocknLoadSound      = S_LNL_SHOTGUN;
}

JsonValue Shotgun::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubShotsPerBurst",      ubShotsPerBurst);
	obj.set("ubBurstPenalty",       ubBurstPenalty);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("burstSound",           burstSound);
	obj.set("silencedSound",        silencedSound);
	obj.set("silencedBurstSound",   silencedBurstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

LMG::LMG(uint16_t itemIndex, ST::string&& internalName,
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
		ST::string sound,
		ST::string burstSound,
		ST::string silencedSound,
		ST::string silencedBurstSound)
	:WeaponModel(IC_GUN, GUN_LMG, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = MGCLASS;
	this->calibre        = calibre;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubShotsPerBurst      = ShotsPerBurst;
	ubBurstPenalty       = BurstPenalty;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = sound;
	this->burstSound     = burstSound;
	this->silencedSound  = silencedSound;
	this->silencedBurstSound = silencedBurstSound;
	sReloadSound         = S_RELOAD_LMG;
	sLocknLoadSound      = S_LNL_LMG;
}

JsonValue LMG::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubShotsPerBurst",      ubShotsPerBurst);
	obj.set("ubBurstPenalty",       ubBurstPenalty);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("burstSound",           burstSound);
	obj.set("silencedSound",        silencedSound);
	obj.set("silencedBurstSound",   silencedBurstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

Blade::Blade(uint16_t itemIndex, ST::string&& internalName,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		ST::string Sound)
	:WeaponModel(IC_BLADE, NOT_GUN, KNIFECURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = KNIFECLASS;
	ubReadyTime          = AP_READY_KNIFE;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	this->sound          = Sound;
}

JsonValue Blade::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

ThrowingBlade::ThrowingBlade(uint16_t itemIndex, ST::string&& internalName,
				uint8_t Impact,
				uint8_t ShotsPer4Turns,
				uint8_t Deadliness,
				uint16_t Range,
				uint8_t AttackVolume,
				ST::string Sound)
	:WeaponModel(IC_THROWING_KNIFE, NOT_GUN, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = KNIFECLASS;
	ubReadyTime          = AP_READY_KNIFE;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	this->sound          = Sound;
}

JsonValue ThrowingBlade::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

PunchWeapon::PunchWeapon(uint16_t itemIndex, ST::string&& internalName,
				uint8_t Impact,
				uint8_t ShotsPer4Turns,
				uint8_t Deadliness,
				uint8_t AttackVolume,
				ST::string Sound)
	:WeaponModel(IC_PUNCH, NOT_GUN, PUNCHCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = KNIFECLASS;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	usRange              = 10;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	this->sound          = Sound;
}


JsonValue PunchWeapon::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

Launcher::Launcher(uint16_t itemIndex, ST::string&& internalName,
			const ExplosiveCalibreModel* explosiveCalibre,
			uint8_t BulletSpeed,
			uint8_t ReadyTime,
			uint8_t ShotsPer4Turns,
			uint8_t Deadliness,
			uint16_t Range,
			uint8_t AttackVolume,
			uint8_t HitVolume,
			ST::string Sound)
	:WeaponModel(IC_LAUNCHER, NOT_GUN, TRAJECTORYCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = RIFLECLASS;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = 1;
	ubDeadliness         = Deadliness;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = Sound;
	this->explosiveCalibre = explosiveCalibre;
}

JsonValue Launcher::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

LAW::LAW(uint16_t itemIndex, ST::string&& internalName,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound)
	:WeaponModel(IC_GUN, NOT_GUN, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = RIFLECLASS;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = 80;
	ubDeadliness         = Deadliness;
	ubMagSize            = 1;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = Sound;
}

JsonValue LAW::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

Cannon::Cannon(uint16_t itemIndex, ST::string&& internalName,
		const ExplosiveCalibreModel* explosiveCalibre,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound)
	:WeaponModel(IC_GUN, NOT_GUN, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = RIFLECLASS;
	ubReadyTime          = ReadyTime;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubBulletSpeed        = BulletSpeed;
	ubImpact             = 80;
	ubDeadliness         = Deadliness;
	ubMagSize            = 1;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = Sound;
	this->explosiveCalibre = explosiveCalibre;
}

JsonValue Cannon::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("ubReadyTime",          ubReadyTime);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubBulletSpeed",        ubBulletSpeed);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}

MonsterSpit::MonsterSpit(uint16_t itemIndex, ST::string&& internalName,
				const CalibreModel *calibre,
				uint8_t Impact,
				uint8_t ShotsPer4Turns,
				uint8_t Deadliness,
				uint8_t MagSize,
				uint16_t Range,
				uint8_t AttackVolume,
				uint8_t HitVolume,
				ST::string Sound,
				uint16_t smokeEffect)
	:WeaponModel(IC_GUN, NOT_GUN, TARGETCURS, itemIndex, std::move(internalName))
{
	ubWeaponClass        = MONSTERCLASS;
	this->calibre        = calibre;
	ubReadyTime          = AP_READY_KNIFE;
	ubShotsPer4Turns     = ShotsPer4Turns;
	ubBulletSpeed        = 250;
	ubImpact             = Impact;
	ubDeadliness         = Deadliness;
	ubMagSize            = MagSize;
	usRange              = Range;
	usReloadDelay        = 200;
	ubAttackVolume       = AttackVolume;
	ubHitVolume          = HitVolume;
	this->sound          = Sound;
	usSmokeEffect        = smokeEffect;
}

JsonValue MonsterSpit::serialize() const
{
	JsonObject obj;
	WeaponModel::serializeTo(obj);
	obj.set("calibre",              calibre->internalName);
	obj.set("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.set("ubImpact",             ubImpact);
	obj.set("ubDeadliness",         ubDeadliness);
	obj.set("ubMagSize",            ubMagSize);
	obj.set("usRange",              usRange);
	obj.set("ubAttackVolume",       ubAttackVolume);
	obj.set("ubHitVolume",          ubHitVolume);
	obj.set("sound",                sound);
	obj.set("ubSmokeEffect",        usSmokeEffect);
	serializeAttachments(obj);
	serializeFlags(obj);
	return obj.toValue();
}
