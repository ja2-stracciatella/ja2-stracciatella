#include "MagazineModel.h"

#include "AmmoTypeModel.h"
#include "CalibreModel.h"
#include "JsonObject.h"

MagazineModel::MagazineModel(uint16_t index_,
                             uint16_t itemIndex_,
                             const char* internalName_,
                             const CalibreModel *calibre_,
                             uint16_t capacity_,
                             const AmmoTypeModel *ammoType_
  )
  :index(index_), itemIndex(itemIndex_), internalName(internalName_),
   calibre(calibre_), capacity(capacity_), ammoType(ammoType_)
{
}

void MagazineModel::serializeTo(JsonObject &obj) const
{
  obj.AddMember("index",                index);
  obj.AddMember("itemIndex",            itemIndex);
  obj.AddMember("internalName",         internalName.c_str());
  obj.AddMember("calibre",              calibre->internalName);
  obj.AddMember("capacity",             capacity);
  obj.AddMember("ammoType",             ammoType->internalName);
}

MagazineModel* MagazineModel::deserialize(
  JsonObjectReader &obj,
  const std::map<std::string, const CalibreModel*> &calibreMap,
  const std::map<std::string, const AmmoTypeModel*> &ammoTypeMap)
{
  int index                     = obj.GetInt("index");
  int itemIndex                 = obj.GetInt("itemIndex");
  const char *internalName      = obj.GetString("internalName");
  const CalibreModel *calibre   = getCalibre(obj.GetString("calibre"), calibreMap);
  uint16_t capacity             = obj.GetInt("capacity");
  const AmmoTypeModel *ammoType = getAmmoType(obj.GetString("ammoType"), ammoTypeMap);
  return new MagazineModel(index, itemIndex, internalName, calibre, capacity, ammoType);
}
