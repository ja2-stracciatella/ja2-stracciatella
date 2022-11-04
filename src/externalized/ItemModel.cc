#include "ItemModel.h"

#include <utility>

#include "JsonObject.h"
#include "MagazineModel.h"
#include "WeaponModels.h"


ItemModel::ItemModel(ItemId itemIndex,
			ST::string internalName,
			uint32_t usItemClass,
			uint8_t classIndex,
			ItemCursor cursor) :
	inventoryGraphics(InventoryGraphicsModel(GraphicModel("interface/mdguns.sti", 0), GraphicModel("bigitems/gun00.sti", 0))),
	tileGraphic(TilesetTileIndexModel(TileTypeDefines::GUNS, 1))
{
	this->itemIndex             = itemIndex;
	this->internalName          = std::move(internalName);
	this->usItemClass           = usItemClass;
	this->ubClassIndex          = classIndex;
	this->ubCursor              = cursor;
	this->ubWeight              = 0;
	this->ubPerPocket           = 0;
	this->usPrice               = 0;
	this->ubCoolness            = 0;
	this->bReliability          = 0;
	this->bRepairEase           = 0;
	this->fFlags                = 0;
}

ItemModel::ItemModel(ItemId   itemIndex,
			ST::string internalName,
			ST::string shortName,
			ST::string name,
			ST::string description,
			uint32_t   usItemClass,
			uint8_t    ubClassIndex,
			ItemCursor ubCursor,
			InventoryGraphicsModel inventoryGraphics_,
			TilesetTileIndexModel tileGraphic_,
			uint8_t    ubWeight,
			uint8_t    ubPerPocket,
			uint16_t   usPrice,
			uint8_t    ubCoolness,
			int8_t     bReliability,
			int8_t     bRepairEase,
			uint16_t   fFlags) : inventoryGraphics(inventoryGraphics_), tileGraphic(tileGraphic_)
{
	this->itemIndex             = itemIndex;
	this->internalName          = internalName;
	this->shortName             = shortName;
	this->name                  = name;
	this->description           = description;
	this->usItemClass           = usItemClass;
	this->ubClassIndex          = ubClassIndex;
	this->ubCursor              = ubCursor;
	this->ubWeight              = ubWeight;
	this->ubPerPocket           = ubPerPocket;
	this->usPrice               = usPrice;
	this->ubCoolness            = ubCoolness;
	this->bReliability          = bReliability;
	this->bRepairEase           = bRepairEase;
	this->fFlags                = fFlags;
}

ItemModel::~ItemModel() = default;

const ST::string& ItemModel::getInternalName() const   { return internalName;          }

const ST::string& ItemModel::getShortName() const      { return shortName; }
const ST::string& ItemModel::getName() const           { return name; }
const ST::string& ItemModel::getDescription() const     { return description; }

ItemId          ItemModel::getItemIndex() const        { return itemIndex;             }
uint32_t        ItemModel::getItemClass() const        { return usItemClass;           }
uint8_t         ItemModel::getClassIndex() const       { return ubClassIndex;          }
ItemCursor      ItemModel::getCursor() const           { return ubCursor;              }
uint8_t         ItemModel::getWeight() const           { return ubWeight;              }
uint8_t         ItemModel::getPerPocket() const        { return ubPerPocket;           }
uint16_t        ItemModel::getPrice() const            { return usPrice;               }
uint8_t         ItemModel::getCoolness() const         { return ubCoolness;            }
int8_t          ItemModel::getReliability() const      { return bReliability;          }
int8_t          ItemModel::getRepairEase() const       { return bRepairEase;           }
uint16_t        ItemModel::getFlags() const            { return fFlags;                }
const GraphicModel& ItemModel::getInventoryGraphicSmall() const { return inventoryGraphics.small; }
const GraphicModel& ItemModel::getInventoryGraphicBig() const { return inventoryGraphics.big; }
const TilesetTileIndexModel& ItemModel::getTileGraphic() const { return tileGraphic; }

bool ItemModel::isAmmo() const           { return usItemClass == IC_AMMO; }
bool ItemModel::isArmour() const         { return usItemClass == IC_ARMOUR; }
bool ItemModel::isBlade() const          { return usItemClass == IC_BLADE; }
bool ItemModel::isBomb() const           { return usItemClass == IC_BOMB; }
bool ItemModel::isExplosive() const      { return usItemClass & (IC_GRENADE | IC_BOMB); }
bool ItemModel::isFace() const           { return usItemClass == IC_FACE; }
bool ItemModel::isGrenade() const        { return usItemClass == IC_GRENADE; }
bool ItemModel::isGun() const            { return usItemClass == IC_GUN; }
bool ItemModel::isKey() const            { return usItemClass == IC_KEY; }
bool ItemModel::isKit() const            { return usItemClass == IC_KIT; }
bool ItemModel::isLauncher() const       { return usItemClass == IC_LAUNCHER; }
bool ItemModel::isMedkit() const         { return usItemClass == IC_MEDKIT; }
bool ItemModel::isMisc() const           { return usItemClass == IC_MISC; }
bool ItemModel::isMoney() const          { return usItemClass == IC_MONEY; }
bool ItemModel::isPunch() const          { return usItemClass == IC_PUNCH; }
bool ItemModel::isTentacles() const      { return usItemClass == IC_TENTACLES; }
bool ItemModel::isThrowingKnife() const  { return usItemClass == IC_THROWING_KNIFE; }
bool ItemModel::isThrown() const         { return usItemClass == IC_THROWN; }
bool ItemModel::isWeapon() const         { return usItemClass & (IC_GUN | IC_BLADE | IC_THROWING_KNIFE | IC_LAUNCHER | IC_TENTACLES); }

bool ItemModel::isTwoHanded() const      { return fFlags & ITEM_TWO_HANDED;             }
bool ItemModel::isInBigGunList() const   { return fFlags & ITEM_BIGGUNLIST;             }

void ItemModel::serializeFlags(JsonObject &obj) const
{
	if(getFlags() & ITEM_DAMAGEABLE)          { obj.AddMember("bDamageable", true);     }
	if(getFlags() & ITEM_REPAIRABLE)          { obj.AddMember("bRepairable", true);     }
	if(getFlags() & ITEM_WATER_DAMAGES)       { obj.AddMember("bWaterDamages", true);   }
	if(getFlags() & ITEM_METAL)               { obj.AddMember("bMetal", true);          }
	if(getFlags() & ITEM_SINKS)               { obj.AddMember("bSinks", true);          }
	if(getFlags() & ITEM_SHOW_STATUS)         { obj.AddMember("bShowStatus", true);     }
	if(getFlags() & ITEM_HIDDEN_ADDON)        { obj.AddMember("bHiddenAddon", true);    }
	if(getFlags() & ITEM_TWO_HANDED)          { obj.AddMember("bTwoHanded", true);      }
	if(getFlags() & ITEM_NOT_BUYABLE)         { obj.AddMember("bNotBuyable", true);     }
	if(getFlags() & ITEM_ATTACHMENT)          { obj.AddMember("bAttachment", true);     }
	if(getFlags() & ITEM_BIGGUNLIST)          { obj.AddMember("bBigGunList", true);     }
	if(getFlags() & ITEM_NOT_EDITOR)          { obj.AddMember("bNotEditor", true);      }
	if(getFlags() & ITEM_DEFAULT_UNDROPPABLE) { obj.AddMember("bDefaultUndroppable", true); }
	if(getFlags() & ITEM_UNAERODYNAMIC)       { obj.AddMember("bUnaerodynamic", true);  }
	if(getFlags() & ITEM_ELECTRONIC)          { obj.AddMember("bElectronic", true);     }
	if(getFlags() & ITEM_INSEPARABLE)         { obj.AddMember("bInseparable", true);    }
}

uint32_t ItemModel::deserializeFlags(JsonObjectReader &obj) const
{
	uint32_t flags = 0;
	if(obj.getOptionalBool("bDamageable"))        { flags |= ITEM_DAMAGEABLE;             }
	if(obj.getOptionalBool("bRepairable"))        { flags |= ITEM_REPAIRABLE;             }
	if(obj.getOptionalBool("bWaterDamages"))      { flags |= ITEM_WATER_DAMAGES;          }
	if(obj.getOptionalBool("bMetal"))             { flags |= ITEM_METAL;                  }
	if(obj.getOptionalBool("bSinks"))             { flags |= ITEM_SINKS;                  }
	if(obj.getOptionalBool("bShowStatus"))        { flags |= ITEM_SHOW_STATUS;            }
	if(obj.getOptionalBool("bHiddenAddon"))       { flags |= ITEM_HIDDEN_ADDON;           }
	if(obj.getOptionalBool("bTwoHanded"))         { flags |= ITEM_TWO_HANDED;             }
	if(obj.getOptionalBool("bNotBuyable"))        { flags |= ITEM_NOT_BUYABLE;            }
	if(obj.getOptionalBool("bAttachment"))        { flags |= ITEM_ATTACHMENT;             }
	if(obj.getOptionalBool("bBigGunList"))        { flags |= ITEM_BIGGUNLIST;             }
	if(obj.getOptionalBool("bNotEditor"))         { flags |= ITEM_NOT_EDITOR;             }
	if(obj.getOptionalBool("bDefaultUndroppable")){ flags |= ITEM_DEFAULT_UNDROPPABLE;    }
	if(obj.getOptionalBool("bUnaerodynamic"))     { flags |= ITEM_UNAERODYNAMIC;          }
	if(obj.getOptionalBool("bElectronic"))        { flags |= ITEM_ELECTRONIC;             }
	if(obj.getOptionalBool("bInseparable"))       { flags |= ITEM_INSEPARABLE;            }
	return flags;
}

/** Check if the given attachment can be attached to the item. */
bool ItemModel::canBeAttached(ItemId attachment) const
{
	return false;
}

void ItemModel::serializeTo(JsonObject &obj) const
{
    obj.AddMember("itemIndex", itemIndex.inner());
    obj.AddMember("internalName", internalName);
    obj.AddMember("usItemClass", (uint32_t)getItemClass());
    obj.AddMember("ubClassIndex", getClassIndex());
    obj.AddMember("ubCursor",  getCursor());
    obj.AddMember("inventoryGraphics", inventoryGraphics.serialize(obj.getAllocator()).getValue());
	obj.AddMember("tileGraphic", tileGraphic.serialize(obj.getAllocator()).getValue());
    obj.AddMember("ubWeight", getWeight());
    obj.AddMember("ubPerPocket", getPerPocket());
    obj.AddMember("usPrice", getPrice());
    obj.AddMember("ubCoolness", getCoolness());
    obj.AddMember("bReliability", getReliability());
    obj.AddMember("bRepairEase", getRepairEase());

    serializeFlags(obj);
}

ST::string ItemModel::deserializeShortName(JsonObjectReader &obj, const VanillaItemStrings& vanillaItemStrings) {
	uint16_t itemIndex = obj.GetUInt("itemIndex");
	ST::string shortName = vanillaItemStrings.getShortName(itemIndex);
	if (obj.getOptionalString("shortName")) {
		shortName = obj.getOptionalString("shortName");
	}
	return shortName;
}

ST::string ItemModel::deserializeName(JsonObjectReader &obj, const VanillaItemStrings& vanillaItemStrings) {
	uint16_t itemIndex = obj.GetUInt("itemIndex");
	ST::string name = vanillaItemStrings.getName(itemIndex);
	if (obj.getOptionalString("name")) {
		name = obj.getOptionalString("name");
	}
	return name;
}

ST::string ItemModel::deserializeDescription(JsonObjectReader &obj, const VanillaItemStrings& vanillaItemStrings) {
	uint16_t itemIndex = obj.GetUInt("itemIndex");
	ST::string description = vanillaItemStrings.getDescription(itemIndex);
	if (obj.getOptionalString("description")) {
		description = obj.getOptionalString("description");
	}
	return description;
}

const ItemModel* ItemModel::deserialize(JsonObjectReader &obj, const VanillaItemStrings& vanillaItemStrings)
{
	auto itemIndex = ItemId(obj.GetUInt("itemIndex"));
	ST::string internalName = obj.GetString("internalName");
	const rapidjson::Value& igSource = obj.GetValue("inventoryGraphics");
	JsonObjectReader igGreader(igSource);
	auto inventoryGraphics = InventoryGraphicsModel::deserialize(igGreader);

	const rapidjson::Value& tgSource = obj.GetValue("tileGraphic");
	JsonObjectReader tgReader(tgSource);
	auto tileGraphic = TilesetTileIndexModel::deserialize(tgReader);

	auto shortName = ItemModel::deserializeShortName(obj, vanillaItemStrings);
	auto name = ItemModel::deserializeName(obj, vanillaItemStrings);
	auto description = ItemModel::deserializeDescription(obj, vanillaItemStrings);

	auto* item = new ItemModel(
		itemIndex,
		internalName,
		shortName,
		name,
		description,
		obj.GetUInt("usItemClass"),
		obj.GetUInt("ubClassIndex"),
		(ItemCursor)obj.GetUInt("ubCursor"),
		inventoryGraphics,
		tileGraphic,
		obj.GetUInt("ubWeight"),
		obj.GetUInt("ubPerPocket"),
		obj.GetUInt("usPrice"),
		obj.GetUInt("ubCoolness"),
		obj.GetInt("bReliability"),
		obj.GetInt("bRepairEase"),
		0
	);
	item->fFlags = item->deserializeFlags(obj);
	return item;
}
