#pragma once

#include <map>
#include <stdint.h>
#include <string>

struct AmmoTypeModel;
struct CalibreModel;
class JsonObject;
class JsonObjectReader;

struct MagazineModel
{
  MagazineModel(uint16_t index,
                uint16_t itemIndex,
                const char* internalName,
                const CalibreModel *calibre,
                uint16_t capacity,
                const AmmoTypeModel *ammoType);

  virtual void serializeTo(JsonObject &obj) const;

  static MagazineModel* deserialize(JsonObjectReader &obj,
                                    const std::map<std::string, const CalibreModel*> &calibreMap,
                                    const std::map<std::string, const AmmoTypeModel*> &ammoTypeMap);

  const uint16_t index;
  const uint16_t itemIndex;
  const std::string internalName;
  const CalibreModel *calibre;
  const uint16_t capacity;
  const AmmoTypeModel *ammoType;
};
