#include "MagazineModel.h"

#include "AmmoTypeModel.h"
#include "CalibreModel.h"
#include "JsonObject.h"

MagazineModel::MagazineModel(uint16_t itemIndex_,
				const char* internalName_,
				const CalibreModel *calibre_,
				uint16_t capacity_,
				const AmmoTypeModel *ammoType_,
				bool dontUseAsDefaultMagazine_
)
	:ItemModel(itemIndex_, internalName_, IC_AMMO, 0, INVALIDCURS),
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
	const std::map<std::string, const CalibreModel*> &calibreMap,
	const std::map<std::string, const AmmoTypeModel*> &ammoTypeMap)
{
	int itemIndex                 = obj.GetInt("itemIndex");
	const char *internalName      = obj.GetString("internalName");
	const CalibreModel *calibre   = getCalibre(obj.GetString("calibre"), calibreMap);
	uint16_t capacity             = obj.GetInt("capacity");
	const AmmoTypeModel *ammoType = getAmmoType(obj.GetString("ammoType"), ammoTypeMap);
	bool dontUseAsDefaultMagazine = obj.getOptionalBool("dontUseAsDefaultMagazine");
	MagazineModel *mag = new MagazineModel(itemIndex, internalName, calibre, capacity, ammoType,
						dontUseAsDefaultMagazine);

	mag->fFlags = mag->deserializeFlags(obj);

	mag->ubGraphicType    = obj.GetInt("ubGraphicType");
	mag->ubGraphicNum     = obj.GetInt("ubGraphicNum");
	mag->ubWeight         = obj.GetInt("ubWeight");
	mag->ubPerPocket      = obj.GetInt("ubPerPocket");
	mag->usPrice          = obj.GetInt("usPrice");
	mag->ubCoolness       = obj.GetInt("ubCoolness");

	const char *replacement = obj.getOptionalString("standardReplacement");
	if(replacement)
	{
		mag->standardReplacement = replacement;
	}

	return mag;
}


const std::string & MagazineModel::getStandardReplacement() const
{
	return standardReplacement;
}
