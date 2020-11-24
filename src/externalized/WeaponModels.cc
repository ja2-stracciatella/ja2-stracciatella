#include "WeaponModels.h"
#include "CalibreModel.h"
#include "Items.h"
#include "JsonObject.h"
#include "Logger.h"
#include "MagazineModel.h"
#include "Points.h"
#include "Sound_Control.h"
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

WeaponModel::WeaponModel(uint32_t itemClass, uint8_t weaponType, uint8_t cursor, uint16_t itemIndex, ST::string internalName_, ST::string internalType_)
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
	internalType         = std::move(internalType_);
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
}

void WeaponModel::serializeTo(JsonObject &obj) const
{
	obj.AddMember("itemIndex",            itemIndex);
	obj.AddMember("internalName",         internalName);
	obj.AddMember("internalType",         internalType);

	obj.AddMember("ubGraphicType", getGraphicType());
	obj.AddMember("ubGraphicNum", getGraphicNum());
	obj.AddMember("ubWeight", getWeight());
	obj.AddMember("ubPerPocket", getPerPocket());
	obj.AddMember("usPrice", getPrice());
	obj.AddMember("ubCoolness", getCoolness());
	obj.AddMember("bReliability", getReliability());
	obj.AddMember("bRepairEase", getRepairEase());
	obj.AddMember("rateOfFire", m_rateOfFire);

	if(isInBigGunList())
	{
		obj.AddMember("standardReplacement", standardReplacement);
	}
}

void WeaponModel::serializeAttachments(JsonObject &obj) const
{
	obj.addOptionalBool("attachment_Silencer",                 attachSilencer);
	obj.addOptionalBool("attachment_SniperScope",              attachSniperScope);
	obj.addOptionalBool("attachment_LaserScope",               attachLaserScope);
	obj.addOptionalBool("attachment_Bipod",                    attachBipod);
	obj.addOptionalBool("attachment_Duckbill",                 attachDuckbill);
	obj.addOptionalBool("attachment_UnderGLauncher",           attachUnderGLauncher);
	obj.addOptionalBool("attachment_SpringAndBoltUpgrade",     attachSpringAndBoltUpgrade);
	obj.addOptionalBool("attachment_GunBarrelExtender",        attachGunBarrelExtender);
}

WeaponModel* WeaponModel::deserialize(JsonObjectReader &obj,
					const std::map<ST::string, const CalibreModel*> &calibreMap)
{
	WeaponModel *wep = NULL;
	int itemIndex = obj.GetInt("itemIndex");
	ST::string internalName = obj.GetString("internalName");
	ST::string internalType = obj.GetString("internalType");

	if (internalType == WEAPON_TYPE_NOWEAPON)
	{
		wep = new NoWeapon(itemIndex, internalName);
	}
	else if (internalType == WEAPON_TYPE_PUNCH)
	{
		wep = new NoWeapon(itemIndex, internalName, IC_PUNCH, PUNCHCURS);
	}
	else if (internalType == WEAPON_TYPE_THROWN)
	{
		wep = new NoWeapon(itemIndex, internalName, IC_THROWN, TOSSCURS);
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
		ST::string Sound       = obj.GetString("Sound");
		wep = new Pistol(itemIndex, internalName,
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
					Sound);
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
		ST::string Sound         = obj.GetString("Sound");
		ST::string BurstSound    = obj.GetString("BurstSound");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		wep = new MPistol(itemIndex, internalName,
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
					Sound,
					BurstSound);
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
		ST::string Sound         = obj.GetString("Sound");
		ST::string BurstSound    = obj.GetString("BurstSound");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		wep = new SMG(itemIndex, internalName,
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
				Sound,
				BurstSound);
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
		ST::string Sound       = obj.GetString("Sound");
		// ST::string BurstSound  = obj.GetString("BurstSound");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		wep = new SniperRifle(itemIndex, internalName,
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
					Sound);
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
		ST::string Sound       = obj.GetString("Sound");
		// ST::string BurstSound  = obj.GetString("BurstSound");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		wep = new Rifle(itemIndex, internalName,
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
				Sound);
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
		ST::string Sound       = obj.GetString("Sound");
		ST::string BurstSound  = obj.GetString("BurstSound");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		wep = new AssaultRifle(itemIndex, internalName,
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
					Sound,
					BurstSound);
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
		ST::string Sound         = obj.GetString("Sound");
		ST::string BurstSound    = obj.GetString("BurstSound");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		wep = new Shotgun(itemIndex, internalName,
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
					Sound,
					BurstSound);
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
		ST::string Sound       = obj.GetString("Sound");
		ST::string BurstSound  = obj.GetString("BurstSound");
		// SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
		// SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
		wep = new LMG(itemIndex, internalName,
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
				Sound,
				BurstSound);
	}
	else if (internalType == "BLADE")
	{
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  Impact          = obj.GetInt("ubImpact");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		ST::string Sound       = obj.GetString("Sound");
		wep = new Blade(itemIndex, internalName,
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
		ST::string Sound         = obj.GetString("Sound");
		wep = new ThrowingBlade(itemIndex, internalName,
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
		ST::string Sound         = obj.GetString("Sound");
		wep = new PunchWeapon(itemIndex, internalName,
					Impact,
					ShotsPer4Turns,
					Deadliness,
					AttackVolume,
					Sound);
	}
	else if (internalType == "LAUNCHER")
	{
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		ST::string Sound         = obj.GetString("Sound");
		wep = new Launcher(itemIndex, internalName,
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
		ST::string Sound       = obj.GetString("Sound");
		wep = new LAW(itemIndex, internalName,
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
		uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
		uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
		uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
		uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
		uint16_t Range           = obj.GetInt("usRange");
		uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
		uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
		ST::string Sound       = obj.GetString("Sound");
		wep = new Cannon(itemIndex, internalName,
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
		ST::string Sound         = obj.GetString("Sound");
		uint16_t smokeEffect     = obj.GetInt("usSmokeEffect");
		wep = new MonsterSpit(itemIndex, internalName,
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
		SLOGE("Weapon type '%s' is not found", internalType);
		return wep;
	}

	wep->ubGraphicType    = obj.GetInt("ubGraphicType");
	wep->ubGraphicNum     = obj.GetInt("ubGraphicNum");
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

	wep->fFlags |= wep->deserializeFlags(obj);

	ST::string replacement = obj.getOptionalString("standardReplacement");
	if (!replacement.empty())
	{
		wep->standardReplacement = replacement;
	}

	return wep;
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

bool WeaponModel::isSameMagCapacity(const MagazineModel *mag) const
{
	return ubMagSize == mag->capacity;
}

bool WeaponModel::hasSound() const
{
	return sound.compare(NO_WEAPON_SOUND_STR) != 0;
}

bool WeaponModel::hasBurstSound() const
{
	return burstSound.compare(NO_WEAPON_SOUND_STR) != 0;
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

NoWeapon::NoWeapon(uint16_t itemIndex, const ST::string& internalName)
	:NoWeapon(itemIndex, internalName, IC_NONE, INVALIDCURS)
{
}

NoWeapon::NoWeapon(uint16_t itemIndex, const ST::string& internalName, uint32_t itemClass, uint8_t cursor)
	: WeaponModel(itemClass, NOT_GUN, cursor, itemIndex, internalName, WEAPON_TYPE_NOWEAPON)
{
}

void NoWeapon::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("usRange", usRange);
	serializeAttachments(obj);
	serializeFlags(obj);
}


Pistol::Pistol(uint16_t itemIndex, ST::string internalName,
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
		ST::string Sound)
	:WeaponModel(IC_GUN, GUN_PISTOL, TARGETCURS, itemIndex, internalName, "PISTOL")
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
	this->sound          = Sound;
	sReloadSound         = S_RELOAD_PISTOL;
	sLocknLoadSound      = S_LNL_PISTOL;
}

void Pistol::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

MPistol::MPistol(uint16_t itemIndex, ST::string internalName,
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
			ST::string BurstSound)
	:WeaponModel(IC_GUN, GUN_M_PISTOL, TARGETCURS, itemIndex, internalName, "M_PISTOL")
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
	this->sound          = Sound;
	this->burstSound     = BurstSound;
	sReloadSound         = S_RELOAD_PISTOL;
	sLocknLoadSound      = S_LNL_PISTOL;
}

void MPistol::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubShotsPerBurst",      ubShotsPerBurst);
	obj.AddMember("ubBurstPenalty",       ubBurstPenalty);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	obj.AddMember("BurstSound",           burstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

SMG::SMG(uint16_t itemIndex, ST::string internalName,
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
		ST::string BurstSound)
	:WeaponModel(IC_GUN, GUN_SMG, TARGETCURS, itemIndex, internalName, "SMG")
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
	this->sound          = Sound;
	this->burstSound     = BurstSound;
	sReloadSound         = S_RELOAD_SMG;
	sLocknLoadSound      = S_LNL_SMG;
}

void SMG::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubShotsPerBurst",      ubShotsPerBurst);
	obj.AddMember("ubBurstPenalty",       ubBurstPenalty);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	obj.AddMember("BurstSound",           burstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

SniperRifle::SniperRifle(uint16_t itemIndex, ST::string internalName,
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
				ST::string Sound)
	:WeaponModel(IC_GUN, GUN_SN_RIFLE, TARGETCURS, itemIndex, internalName, "SN_RIFLE")
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
	this->sound          = Sound;
	sReloadSound         = S_RELOAD_RIFLE;
	sLocknLoadSound      = S_LNL_RIFLE;
}

void SniperRifle::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

Rifle::Rifle(uint16_t itemIndex, ST::string internalName,
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
		ST::string Sound)
	:WeaponModel(IC_GUN, GUN_RIFLE, TARGETCURS, itemIndex, internalName, "RIFLE")
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
	this->sound          = Sound;
	sReloadSound         = S_RELOAD_RIFLE;
	sLocknLoadSound      = S_LNL_RIFLE;
}

void Rifle::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

AssaultRifle::AssaultRifle(uint16_t itemIndex, ST::string internalName,
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
				ST::string BurstSound)
	:WeaponModel(IC_GUN, GUN_AS_RIFLE, TARGETCURS, itemIndex, internalName, "ASRIFLE")
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
	this->sound          = Sound;
	this->burstSound     = BurstSound;
	sReloadSound         = S_RELOAD_RIFLE;
	sLocknLoadSound      = S_LNL_RIFLE;
}

void AssaultRifle::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubShotsPerBurst",      ubShotsPerBurst);
	obj.AddMember("ubBurstPenalty",       ubBurstPenalty);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	obj.AddMember("BurstSound",           burstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

Shotgun::Shotgun(uint16_t itemIndex, ST::string internalName,
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
			ST::string BurstSound)
	:WeaponModel(IC_GUN, GUN_SHOTGUN, TARGETCURS, itemIndex, internalName, "SHOTGUN")
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
	this->sound          = Sound;
	this->burstSound     = BurstSound;
	sReloadSound         = S_RELOAD_SHOTGUN;
	sLocknLoadSound      = S_LNL_SHOTGUN;
}

void Shotgun::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubShotsPerBurst",      ubShotsPerBurst);
	obj.AddMember("ubBurstPenalty",       ubBurstPenalty);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	obj.AddMember("BurstSound",           burstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

LMG::LMG(uint16_t itemIndex, ST::string internalName,
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
		ST::string BurstSound)
	:WeaponModel(IC_GUN, GUN_LMG, TARGETCURS, itemIndex, internalName, "LMG")
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
	this->sound          = Sound;
	this->burstSound     = BurstSound;
	sReloadSound         = S_RELOAD_LMG;
	sLocknLoadSound      = S_LNL_LMG;
}

void LMG::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubShotsPerBurst",      ubShotsPerBurst);
	obj.AddMember("ubBurstPenalty",       ubBurstPenalty);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	obj.AddMember("BurstSound",           burstSound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

Blade::Blade(uint16_t itemIndex, ST::string internalName,
		uint8_t Impact,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		ST::string Sound)
	:WeaponModel(IC_BLADE, NOT_GUN, KNIFECURS, itemIndex, internalName, "BLADE")
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

void Blade::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

ThrowingBlade::ThrowingBlade(uint16_t itemIndex, ST::string internalName,
				uint8_t Impact,
				uint8_t ShotsPer4Turns,
				uint8_t Deadliness,
				uint16_t Range,
				uint8_t AttackVolume,
				ST::string Sound)
	:WeaponModel(IC_THROWING_KNIFE, NOT_GUN, TARGETCURS, itemIndex, internalName, "THROWINGBLADE")
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

void ThrowingBlade::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

PunchWeapon::PunchWeapon(uint16_t itemIndex, ST::string internalName,
				uint8_t Impact,
				uint8_t ShotsPer4Turns,
				uint8_t Deadliness,
				uint8_t AttackVolume,
				ST::string Sound)
	:WeaponModel(IC_PUNCH, NOT_GUN, PUNCHCURS, itemIndex, internalName, "PUNCHWEAPON")
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


void PunchWeapon::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

Launcher::Launcher(uint16_t itemIndex, ST::string internalName,
			uint8_t BulletSpeed,
			uint8_t ReadyTime,
			uint8_t ShotsPer4Turns,
			uint8_t Deadliness,
			uint16_t Range,
			uint8_t AttackVolume,
			uint8_t HitVolume,
			ST::string Sound)
	:WeaponModel(IC_LAUNCHER, NOT_GUN, TRAJECTORYCURS, itemIndex, internalName, "LAUNCHER")
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
}

void Launcher::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

LAW::LAW(uint16_t itemIndex, ST::string internalName,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound)
	:WeaponModel(IC_GUN, NOT_GUN, TARGETCURS, itemIndex, internalName, "LAW")
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

void LAW::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

Cannon::Cannon(uint16_t itemIndex, ST::string internalName,
		uint8_t BulletSpeed,
		uint8_t ReadyTime,
		uint8_t ShotsPer4Turns,
		uint8_t Deadliness,
		uint16_t Range,
		uint8_t AttackVolume,
		uint8_t HitVolume,
		ST::string Sound)
	:WeaponModel(IC_GUN, NOT_GUN, TARGETCURS, itemIndex, internalName, "CANNON")
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

void Cannon::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("ubReadyTime",          ubReadyTime);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubBulletSpeed",        ubBulletSpeed);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	serializeAttachments(obj);
	serializeFlags(obj);
}

MonsterSpit::MonsterSpit(uint16_t itemIndex, ST::string internalName,
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
	:WeaponModel(IC_GUN, NOT_GUN, TARGETCURS, itemIndex, internalName, "MONSTSPIT")
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

void MonsterSpit::serializeTo(JsonObject &obj) const
{
	WeaponModel::serializeTo(obj);
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("ubShotsPer4Turns",     ubShotsPer4Turns);
	obj.AddMember("ubImpact",             ubImpact);
	obj.AddMember("ubDeadliness",         ubDeadliness);
	obj.AddMember("ubMagSize",            ubMagSize);
	obj.AddMember("usRange",              usRange);
	obj.AddMember("ubAttackVolume",       ubAttackVolume);
	obj.AddMember("ubHitVolume",          ubHitVolume);
	obj.AddMember("Sound",                sound);
	obj.AddMember("ubSmokeEffect",        usSmokeEffect);
	serializeAttachments(obj);
	serializeFlags(obj);
}
