#include "WeaponModels.h"

#include "Build/Tactical/Points.h"
#include "Build/Utils/Sound_Control.h"

#include "CalibreModel.h"
#include "JsonObject.h"
#include "MagazineModel.h"

#include "slog/slog.h"
#define TAG "Weapons"


WeaponModel::WeaponModel(uint16_t index, const char* internalName, const char* internalType)
  :sound(NO_WEAPON_SOUND_STR),
   burstSound(NO_WEAPON_SOUND_STR)
{
  this->index = index;
  strncpy(this->internalName, internalName, sizeof(this->internalName));
  strncpy(this->internalType, internalType, sizeof(this->internalType));
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
}

WeaponModel::WeaponModel(uint16_t index,
                         const char* internalName, const char* internalType,
                         uint8_t  WeaponClass,
                         const CalibreModel *calibre,
                         uint8_t  ReadyTime,
                         uint8_t  ShotsPer4Turns,
                         uint8_t  ShotsPerBurst,
                         uint8_t  BurstPenalty,
                         uint8_t  BulletSpeed,
                         uint8_t  Impact,
                         uint8_t  Deadliness,
                         uint8_t  MagSize,
                         uint16_t Range,
                         uint16_t ReloadDelay,
                         uint8_t  AttackVolume,
                         uint8_t  HitVolume,
                         const char * Sound_,
                         const char * BurstSound_,
                         SoundID  ReloadSound,
                         SoundID  LocknLoadSound)
  :sound(Sound_),
   burstSound(BurstSound_)
{
  this->index = index;
  strncpy(this->internalName, internalName, sizeof(this->internalName));
  strncpy(this->internalType, internalType, sizeof(this->internalType));
  ubWeaponClass        = WeaponClass;
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
  usReloadDelay        = ReloadDelay;
  ubAttackVolume       = AttackVolume;
  ubHitVolume          = HitVolume;
  sReloadSound         = ReloadSound;
  sLocknLoadSound      = LocknLoadSound;
}

void WeaponModel::serializeTo(JsonObject &obj) const
{
  obj.AddMember("index",                index);
  obj.AddMember("internalName",         internalName);
  obj.AddMember("internalType",         internalType);
}

WeaponModel* WeaponModel::deserialize(JsonObjectReader &obj,
                                      const std::map<std::string, const CalibreModel*> &calibreMap)
{
  int index = obj.GetInt("index");
  const char *internalName = obj.GetString("internalName");
  const char *internalType = obj.GetString("internalType");

  if(!strcmp(internalType, "NOWEAPON"))
  {
    uint16_t Range           = obj.GetInt("usRange");
    return new NoWeapon(index, internalName, Range);
  }
  else if(!strcmp(internalType, "PISTOL"))
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
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new Pistol(index, internalName,
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
  else if(!strcmp(internalType, "M_PISTOL"))
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
    const char * Sound       = obj.GetString("Sound");
    const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new MPistol(index, internalName,
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
  else if(!strcmp(internalType, "SMG"))
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
    const char * Sound       = obj.GetString("Sound");
    const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new SMG(index, internalName,
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
  else if(!strcmp(internalType, "SN_RIFLE"))
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
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new SniperRifle(index, internalName,
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
  else if(!strcmp(internalType, "RIFLE"))
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
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new Rifle(index, internalName,
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
  else if(!strcmp(internalType, "ASRIFLE"))
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
    const char * Sound       = obj.GetString("Sound");
    const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new AssaultRifle(index, internalName,
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
  else if(!strcmp(internalType, "SHOTGUN"))
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
    const char * Sound       = obj.GetString("Sound");
    const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new Shotgun(index, internalName,
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
  else if(!strcmp(internalType, "LMG"))
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
    const char * Sound       = obj.GetString("Sound");
    const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new LMG(index, internalName,
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
  else if(!strcmp(internalType, "BLADE"))
  {
    // const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
    // uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
    uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
    // uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
    // uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
    // uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
    uint8_t  Impact          = obj.GetInt("ubImpact");
    uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
    // uint8_t  MagSize         = obj.GetInt("ubMagSize");
    uint16_t Range           = obj.GetInt("usRange");
    // uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
    uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
    // uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new Blade(index, internalName,
                     Impact,
                     ShotsPer4Turns,
                     Deadliness,
                     Range,
                     AttackVolume,
                     Sound);
  }
  else if(!strcmp(internalType, "THROWINGBLADE"))
  {
    // const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
    // uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
    uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
    // uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
    // uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
    // uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
    uint8_t  Impact          = obj.GetInt("ubImpact");
    uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
    // uint8_t  MagSize         = obj.GetInt("ubMagSize");
    uint16_t Range           = obj.GetInt("usRange");
    // uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
    uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
    // uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new ThrowingBlade(index, internalName,
                             Impact,
                             ShotsPer4Turns,
                             Deadliness,
                             Range,
                             AttackVolume,
                             Sound);
  }
  else if(!strcmp(internalType, "PUNCHWEAPON"))
  {
    // const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
    // uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
    uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
    // uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
    // uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
    // uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
    uint8_t  Impact          = obj.GetInt("ubImpact");
    uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
    // uint8_t  MagSize         = obj.GetInt("ubMagSize");
    // uint16_t Range           = obj.GetInt("usRange");
    // uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
    uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
    // uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new PunchWeapon(index, internalName,
                           Impact,
                           ShotsPer4Turns,
                           Deadliness,
                           AttackVolume,
                           Sound);
  }
  else if(!strcmp(internalType, "LAUNCHER"))
  {
    // const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
    uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
    uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
    // uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
    // uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
    uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
    // uint8_t  Impact          = obj.GetInt("ubImpact");
    uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
    // uint8_t  MagSize         = obj.GetInt("ubMagSize");
    uint16_t Range           = obj.GetInt("usRange");
    // uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
    uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
    uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new Launcher(index, internalName,
                        BulletSpeed,
                        ReadyTime,
                        ShotsPer4Turns,
                        Deadliness,
                        Range,
                        AttackVolume,
                        HitVolume,
                        Sound);
  }
  else if(!strcmp(internalType, "LAW"))
  {
    // const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
    uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
    uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
    // uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
    // uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
    uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
    // uint8_t  Impact          = obj.GetInt("ubImpact");
    uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
    // uint8_t  MagSize         = obj.GetInt("ubMagSize");
    uint16_t Range           = obj.GetInt("usRange");
    // uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
    uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
    uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new LAW(index, internalName,
                   BulletSpeed,
                   ReadyTime,
                   ShotsPer4Turns,
                   Deadliness,
                   Range,
                   AttackVolume,
                   HitVolume,
                   Sound);
  }
  else if(!strcmp(internalType, "CANNON"))
  {
    // const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
    uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
    uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
    // uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
    // uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
    uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
    // uint8_t  Impact          = obj.GetInt("ubImpact");
    uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
    // uint8_t  MagSize         = obj.GetInt("ubMagSize");
    uint16_t Range           = obj.GetInt("usRange");
    // uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
    uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
    uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new Cannon(index, internalName,
                      BulletSpeed,
                      ReadyTime,
                      ShotsPer4Turns,
                      Deadliness,
                      Range,
                      AttackVolume,
                      HitVolume,
                      Sound);
  }
  else if(!strcmp(internalType, "MONSTSPIT"))
  {
    const CalibreModel *calibre = getCalibre(obj.GetString("calibre"), calibreMap);
    // uint8_t  ReadyTime       = obj.GetInt("ubReadyTime");
    uint8_t  ShotsPer4Turns  = obj.GetInt("ubShotsPer4Turns");
    // uint8_t  ShotsPerBurst   = obj.GetInt("ubShotsPerBurst");
    // uint8_t  BurstPenalty    = obj.GetInt("ubBurstPenalty");
    // uint8_t  BulletSpeed     = obj.GetInt("ubBulletSpeed");
    uint8_t  Impact          = obj.GetInt("ubImpact");
    uint8_t  Deadliness      = obj.GetInt("ubDeadliness");
    uint8_t  MagSize         = obj.GetInt("ubMagSize");
    uint16_t Range           = obj.GetInt("usRange");
    // uint16_t ReloadDelay     = obj.GetInt("usReloadDelay");
    uint8_t  AttackVolume    = obj.GetInt("ubAttackVolume");
    uint8_t  HitVolume       = obj.GetInt("ubHitVolume");
    const char * Sound       = obj.GetString("Sound");
    // const char * BurstSound  = obj.GetString("BurstSound");
    // SoundID  ReloadSound     = (SoundID) obj.GetInt("sReloadSound");
    // SoundID  LocknLoadSound  = (SoundID) obj.GetInt("sLocknLoadSound");
    return new MonsterSpit(index, internalName,
                           calibre,
                           Impact,
                           ShotsPer4Turns,
                           Deadliness,
                           MagSize,
                           Range,
                           AttackVolume,
                           HitVolume,
                           Sound);
  }

  SLOGE(TAG, "Weapon type '%s' is not found", internalType);
  return NULL;
}


bool WeaponModel::matches(const CalibreModel *calibre) const
{
  return this->calibre->index == calibre->index;
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

////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////// 

NoWeapon::NoWeapon(uint16_t index, const char * internalName, uint16_t Range)
  :WeaponModel(index, internalName, "NOWEAPON")
{
}

void NoWeapon::serializeTo(JsonObject &obj) const
{
  WeaponModel::serializeTo(obj);
  obj.AddMember("usRange", usRange);
}


Pistol::Pistol(uint16_t index, const char * internalName,
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
               const char * Sound)
  :WeaponModel(index, internalName, "PISTOL")
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
}

MPistol::MPistol(uint16_t index, const char * internalName,
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
                 const char * Sound,
                 const char * BurstSound)
  :WeaponModel(index, internalName, "M_PISTOL")
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
}

SMG::SMG(uint16_t index, const char * internalName,
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
         const char * Sound,
         const char * BurstSound)
  :WeaponModel(index, internalName, "SMG")
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
}

SniperRifle::SniperRifle(uint16_t index, const char * internalName,
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
                         const char * Sound)
  :WeaponModel(index, internalName, "SN_RIFLE")
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
}

Rifle::Rifle(uint16_t index, const char * internalName,
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
             const char * Sound)
  :WeaponModel(index, internalName, "RIFLE")
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
}

AssaultRifle::AssaultRifle(uint16_t index, const char * internalName,
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
                           const char * Sound,
                           const char * BurstSound)
  :WeaponModel(index, internalName, "ASRIFLE")
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
}

Shotgun::Shotgun(uint16_t index, const char * internalName,
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
                 const char * Sound,
                 const char * BurstSound)
  :WeaponModel(index, internalName, "SHOTGUN")
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
}

LMG::LMG(uint16_t index, const char * internalName,
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
         const char * Sound,
         const char * BurstSound)
  :WeaponModel(index, internalName, "LMG")
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
}

Blade::Blade(uint16_t index, const char * internalName,
             uint8_t Impact,
             uint8_t ShotsPer4Turns,
             uint8_t Deadliness,
             uint16_t Range,
             uint8_t AttackVolume,
             const char * Sound)
  :WeaponModel(index, internalName, "BLADE")
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
}

ThrowingBlade::ThrowingBlade(uint16_t index, const char * internalName,
                             uint8_t Impact,
                             uint8_t ShotsPer4Turns,
                             uint8_t Deadliness,
                             uint16_t Range,
                             uint8_t AttackVolume,
                             const char * Sound)
  :WeaponModel(index, internalName, "THROWINGBLADE")
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
}

PunchWeapon::PunchWeapon(uint16_t index, const char * internalName,
                         uint8_t Impact,
                         uint8_t ShotsPer4Turns,
                         uint8_t Deadliness,
                         uint8_t AttackVolume,
                         const char * Sound)
  :WeaponModel(index, internalName, "PUNCHWEAPON")
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
}

Launcher::Launcher(uint16_t index, const char * internalName,
                   uint8_t BulletSpeed,
                   uint8_t ReadyTime,
                   uint8_t ShotsPer4Turns,
                   uint8_t Deadliness,
                   uint16_t Range,
                   uint8_t AttackVolume,
                   uint8_t HitVolume,
                   const char * Sound)
  :WeaponModel(index, internalName, "LAUNCHER")
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
}

LAW::LAW(uint16_t index, const char * internalName,
         uint8_t BulletSpeed,
         uint8_t ReadyTime,
         uint8_t ShotsPer4Turns,
         uint8_t Deadliness,
         uint16_t Range,
         uint8_t AttackVolume,
         uint8_t HitVolume,
         const char * Sound)
  :WeaponModel(index, internalName, "LAW")
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
}

Cannon::Cannon(uint16_t index, const char * internalName,
               uint8_t BulletSpeed,
               uint8_t ReadyTime,
               uint8_t ShotsPer4Turns,
               uint8_t Deadliness,
               uint16_t Range,
               uint8_t AttackVolume,
               uint8_t HitVolume,
               const char * Sound)
  :WeaponModel(index, internalName, "CANNON")
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
}

MonsterSpit::MonsterSpit(uint16_t index, const char * internalName,
                         const CalibreModel *calibre,
                         uint8_t Impact,
                         uint8_t ShotsPer4Turns,
                         uint8_t Deadliness,
                         uint8_t MagSize,
                         uint16_t Range,
                         uint8_t AttackVolume,
                         uint8_t HitVolume,
                         const char * Sound)
  :WeaponModel(index, internalName, "MONSTSPIT")
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
}
