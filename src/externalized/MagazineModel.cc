#include "MagazineModel.h"

#include "AmmoTypeModel.h"
#include "CalibreModel.h"
#include "JsonObject.h"
#include <utility>

MagazineModel::MagazineModel(uint16_t itemIndex_,
				ST::string internalName_,
				uint32_t itemClass_,
				const CalibreModel *calibre_,
				uint16_t capacity_,
				const AmmoTypeModel *ammoType_,
				bool dontUseAsDefaultMagazine_
)
	:ItemModel(itemIndex_, std::move(internalName_), itemClass_, 0, INVALIDCURS),
	calibre(calibre_), capacity(capacity_), ammoType(ammoType_),
	dontUseAsDefaultMagazine(dontUseAsDefaultMagazine_)
{
}

#include "ContentManager.h"
#include "GameInstance.h"
#include "Items.h"

void MagazineModel::serializeTo(JsonObject &obj) const
{
	obj.AddMember("itemIndex",            itemIndex);
	obj.AddMember("internalName",         internalName.c_str());
	obj.AddMember("calibre",              calibre->internalName);
	obj.AddMember("capacity",             capacity);
	obj.AddMember("ammoType",             ammoType->internalName);

	obj.AddMember("ubGraphicType",        getGraphicType());
	obj.AddMember("ubGraphicNum",         getGraphicNum());
	obj.AddMember("ubWeight",             getWeight());
	obj.AddMember("ubPerPocket",          getPerPocket());
	obj.AddMember("usPrice",              getPrice());
	obj.AddMember("ubCoolness",           getCoolness());

	if(isInBigGunList())
	{
		obj.AddMember("standardReplacement", standardReplacement);
	}

	serializeFlags(obj);

	if(dontUseAsDefaultMagazine)
	{
		obj.AddMember("dontUseAsDefaultMagazine", dontUseAsDefaultMagazine);
	}
}

MagazineModel* MagazineModel::deserialize(
	JsonObjectReader &obj,
	const std::map<ST::string, const CalibreModel*> &calibreMap,
	const std::map<ST::string, const AmmoTypeModel*> &ammoTypeMap)
{
	int itemIndex                 = obj.GetInt("itemIndex");
	ST::string internalName       = obj.GetString("internalName");
	const CalibreModel *calibre   = getCalibre(obj.GetString("calibre"), calibreMap);
	uint32_t itemClass            = (calibre->index != NOAMMO) ? IC_AMMO : IC_NONE;
	uint16_t capacity             = obj.GetInt("capacity");
	const AmmoTypeModel *ammoType = getAmmoType(obj.GetString("ammoType"), ammoTypeMap);
	bool dontUseAsDefaultMagazine = obj.getOptionalBool("dontUseAsDefaultMagazine");
	MagazineModel *mag = new MagazineModel(itemIndex, internalName, itemClass, calibre, capacity, ammoType,
						dontUseAsDefaultMagazine);

	mag->fFlags = mag->deserializeFlags(obj);

	mag->ubGraphicType    = obj.GetInt("ubGraphicType");
	mag->ubGraphicNum     = obj.GetInt("ubGraphicNum");
	mag->ubWeight         = obj.GetInt("ubWeight");
	mag->ubPerPocket      = obj.GetInt("ubPerPocket");
	mag->usPrice          = obj.GetInt("usPrice");
	mag->ubCoolness       = obj.GetInt("ubCoolness");

	ST::string replacement = obj.getOptionalString("standardReplacement");
	if (!replacement.empty())
	{
		mag->standardReplacement = replacement;
	}

	return mag;
}


const ST::string & MagazineModel::getStandardReplacement() const
{
	return standardReplacement;
}
