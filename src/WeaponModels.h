#pragma once

#include <stdint.h>

#include <map>
#include <string>

#include "Build/Tactical/Weapons.h"

class JsonObject;
class JsonObjectReader;
enum SoundID;
struct CalibreModel;
struct MagazineModel;

#define NO_WEAPON_SOUND ((SoundID)-1)
#define NO_WEAPON_SOUND_STR ("")

struct WeaponModel : WEAPONTYPE
{
  WeaponModel(uint16_t index, const char* internalName, const char* internalType);

  WeaponModel(uint16_t index, const char* internalName, const char* internalType,
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
              const char * Sound,
              const char * BurstSound,
              SoundID  ReloadSound,
              SoundID  LocknLoadSound);

  virtual void serializeTo(JsonObject &obj) const;

  static WeaponModel* deserialize(JsonObjectReader &obj,
                                  const std::map<std::string, const CalibreModel*> &calibreMap);

  bool matches(const CalibreModel *calibre) const;
  bool isSameMagCapacity(const MagazineModel *mag) const;

  bool hasSound() const;
  bool hasBurstSound() const;

  uint16_t index;
  std::string sound;
  std::string burstSound;
};

struct NoWeapon : WeaponModel
{
  NoWeapon(uint16_t index, const char * internalName, uint16_t Range);

  virtual void serializeTo(JsonObject &obj) const;
};

struct Pistol : WeaponModel
{
  Pistol(uint16_t index, const char * internalName,
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
         const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};



struct MPistol : WeaponModel
{
  MPistol(uint16_t index, const char * internalName,
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
          const char * BurstSound);

  virtual void serializeTo(JsonObject &obj) const;
};



struct SMG : WeaponModel
{
  SMG(uint16_t index, const char * internalName,
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
      const char * BurstSound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct SniperRifle : WeaponModel
{
  SniperRifle(uint16_t index, const char * internalName,
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
              const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct Rifle : WeaponModel
{
  Rifle(uint16_t index, const char * internalName,
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
        const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct AssaultRifle : WeaponModel
{
  AssaultRifle(uint16_t index, const char * internalName,
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
               const char * BurstSound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct Shotgun : WeaponModel
{
  Shotgun(uint16_t index, const char * internalName,
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
          const char * BurstSound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct LMG : WeaponModel
{
  LMG(uint16_t index, const char * internalName,
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
      const char * BurstSound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct Blade : WeaponModel
{
  Blade(uint16_t index, const char * internalName,
        uint8_t Impact,
        uint8_t ShotsPer4Turns,
        uint8_t Deadliness,
        uint16_t Range,
        uint8_t AttackVolume,
        const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct ThrowingBlade : WeaponModel
{
  ThrowingBlade(uint16_t index, const char * internalName,
                uint8_t Impact,
                uint8_t ShotsPer4Turns,
                uint8_t Deadliness,
                uint16_t Range,
                uint8_t AttackVolume,
                const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct PunchWeapon : WeaponModel
{
  PunchWeapon(uint16_t index, const char * internalName,
              uint8_t Impact,
              uint8_t ShotsPer4Turns,
              uint8_t Deadliness,
              uint8_t AttackVolume,
              const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct Launcher : WeaponModel
{
  Launcher(uint16_t index, const char * internalName,
           uint8_t BulletSpeed,
           uint8_t ReadyTime,
           uint8_t ShotsPer4Turns,
           uint8_t Deadliness,
           uint16_t Range,
           uint8_t AttackVolume,
           uint8_t HitVolume,
           const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct LAW : WeaponModel
{
  LAW(uint16_t index, const char * internalName,
      uint8_t BulletSpeed,
      uint8_t ReadyTime,
      uint8_t ShotsPer4Turns,
      uint8_t Deadliness,
      uint16_t Range,
      uint8_t AttackVolume,
      uint8_t HitVolume,
      const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct Cannon : WeaponModel
{
  Cannon(uint16_t index, const char * internalName,
         uint8_t BulletSpeed,
         uint8_t ReadyTime,
         uint8_t ShotsPer4Turns,
         uint8_t Deadliness,
         uint16_t Range,
         uint8_t AttackVolume,
         uint8_t HitVolume,
         const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};


struct MonsterSpit : WeaponModel
{
  MonsterSpit(uint16_t index, const char * internalName,
              const CalibreModel *calibre,
              uint8_t Impact,
              uint8_t ShotsPer4Turns,
              uint8_t Deadliness,
              uint8_t MagSize,
              uint16_t Range,
              uint8_t AttackVolume,
              uint8_t HitVolume,
              const char * Sound);

  virtual void serializeTo(JsonObject &obj) const;
};
