#include "MagazineModel.h"

#include "AmmoTypeModel.h"
#include "CalibreModel.h"
#include "ItemModel.h"
#include <utility>

MagazineModel::MagazineModel(uint16_t itemIndex_,
				ST::string internalName_,
				ST::string shortName_,
				ST::string name_,
				ST::string description_,
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
	this->shortName = shortName_;
	this->name = name_;
	this->description = description_;
}


JsonValue MagazineModel::serialize() const
{
	JsonObject obj;
	obj.set("itemIndex",            itemIndex);
	obj.set("internalName",         internalName);
	obj.set("calibre",              calibre->internalName);
	obj.set("capacity",             capacity);
	obj.set("ammoType",             ammoType->internalName);

	obj.set("inventoryGraphics",      inventoryGraphics.serialize());
	obj.set("tileGraphic",      tileGraphic.serialize());
	obj.set("ubWeight",             getWeight());
	obj.set("ubPerPocket",          getPerPocket());
	obj.set("usPrice",              getPrice());
	obj.set("ubCoolness",           getCoolness());

	if(isInBigGunList())
	{
		obj.set("standardReplacement", standardReplacement);
	}

	serializeFlags(obj);

	if(dontUseAsDefaultMagazine)
	{
		obj.set("dontUseAsDefaultMagazine", dontUseAsDefaultMagazine);
	}

	return obj.toValue();
}

MagazineModel* MagazineModel::deserialize(
	const JsonValue &json,
	const std::map<ST::string, const CalibreModel*> &calibreMap,
	const std::map<ST::string, const AmmoTypeModel*> &ammoTypeMap,
	const BinaryData& vanillaItemStrings)
{
	auto obj = json.toObject();
	ItemModel::InitData const initData{ obj, vanillaItemStrings };
	int itemIndex                 = obj.GetInt("itemIndex");
	ST::string internalName       = obj.GetString("internalName");
	const CalibreModel *calibre   = getCalibre(obj.GetString("calibre"), calibreMap);
	uint32_t itemClass            = (calibre->index != CalibreModel::NOAMMO) ? IC_AMMO : IC_NONE;
	uint16_t capacity             = obj.GetInt("capacity");
	const AmmoTypeModel *ammoType = getAmmoType(obj.GetString("ammoType"), ammoTypeMap);
	bool dontUseAsDefaultMagazine = obj.getOptionalBool("dontUseAsDefaultMagazine");
	auto shortName = ItemModel::deserializeShortName(initData);
	auto name = ItemModel::deserializeName(initData);
	auto description = ItemModel::deserializeDescription(initData);
	MagazineModel *mag = new MagazineModel(
		itemIndex,
		internalName,
		shortName,
		name,
		description,
		itemClass,
		calibre,
		capacity,
		ammoType,
		dontUseAsDefaultMagazine
	);

	mag->fFlags = ItemModel::deserializeFlags(obj);

	const auto inventoryGraphics = InventoryGraphicsModel::deserialize(obj["inventoryGraphics"]);
	const auto tileGraphic = TilesetTileIndexModel::deserialize(obj["tileGraphic"]);

	mag->inventoryGraphics  = inventoryGraphics;
	mag->tileGraphic = tileGraphic;
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
