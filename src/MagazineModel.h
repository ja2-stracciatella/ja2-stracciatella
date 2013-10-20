#pragma once

#include <map>
#include <stdint.h>
#include <string>

#include "ItemModel.h"

class JsonObject;
class JsonObjectReader;
struct AmmoTypeModel;
struct CalibreModel;

struct MagazineModel : ItemModel
{
  MagazineModel(uint16_t itemIndex,
                const char* internalName,
                const CalibreModel *calibre,
                uint16_t capacity,
                const AmmoTypeModel *ammoType
    );

  virtual const MagazineModel* asAmmo() const   { return this; }

  virtual void serializeTo(JsonObject &obj) const;

  static MagazineModel* deserialize(JsonObjectReader &obj,
                                    const std::map<std::string, const CalibreModel*> &calibreMap,
                                    const std::map<std::string, const AmmoTypeModel*> &ammoTypeMap);


  /** Get standard replacement ammo name. */
  virtual const std::string & getStandardReplacement() const;

  std::string standardReplacement;
  const CalibreModel *calibre;
  const uint16_t capacity;
  const AmmoTypeModel *ammoType;
};
