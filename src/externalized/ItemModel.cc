#include "ItemModel.h"
#include "Directories.h"
#include "TranslatableString.h"
#include <cstdint>
#include <memory>
#include <string_theory/st_string.h>
#include <utility>


namespace ItemStrings {
	constexpr const char* BINARY_STRING_FILE = BINARYDATADIR "/itemdesc.edt";
	constexpr uint32_t BINARY_SIZE_ITEM_NAME = 80;
	constexpr uint32_t BINARY_SIZE_SHORT_ITEM_NAME = 80;
	constexpr uint32_t BINARY_SIZE_ITEM_INFO = 240;
	constexpr uint32_t BINARY_SIZE_FULL_ITEM = BINARY_SIZE_ITEM_NAME + BINARY_SIZE_SHORT_ITEM_NAME + BINARY_SIZE_ITEM_INFO;

	constexpr const char* BINARY_STRING_BOBBYR_FILE = BINARYDATADIR "/braydesc.edt";
	constexpr uint32_t BINARY_SIZE_BOBBYR_ITEM_NAME = 80;
	constexpr uint32_t BINARY_SIZE_BOBBYR_ITEM_INFO = 320;
	constexpr uint32_t BINARY_SIZE_BOBBYR_FULL_ITEM = BINARY_SIZE_BOBBYR_ITEM_NAME + BINARY_SIZE_BOBBYR_ITEM_INFO;

	auto deserializeHelper(ItemModel::InitData const& initData,
		char const * file,
		char const * propertyName,
		uint32_t itemSize,
		uint32_t subOffset,
		uint32_t length)
	{
		auto itemIndex = initData.json.GetUInt("itemIndex");
		return TranslatableString::Utils::resolveOptionalProperty(
			initData.stringLoader,
			initData.json,
			propertyName,
			std::make_unique<TranslatableString::Binary>(file, itemIndex * itemSize + subOffset, length)
		);
	}
}

ItemModel::ItemModel(uint16_t itemIndex,
			ST::string&& internalName,
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

ItemModel::ItemModel(uint16_t   itemIndex,
			ST::string&& internalName,
			ST::string&& shortName,
			ST::string&& name,
			ST::string&& description,
			ST::string&& bobbyRaysName,
			ST::string&& bobbyRaysDescription,
			uint32_t   usItemClass,
			uint8_t    ubClassIndex,
			ItemCursor ubCursor,
			InventoryGraphicsModel&& inventoryGraphics_,
			TilesetTileIndexModel&& tileGraphic_,
			uint8_t    ubWeight,
			uint8_t    ubPerPocket,
			uint16_t   usPrice,
			uint8_t    ubCoolness,
			int8_t     bReliability,
			int8_t     bRepairEase,
			uint16_t   fFlags) : inventoryGraphics(std::move(inventoryGraphics_)), tileGraphic(tileGraphic_)
{
	this->itemIndex             = itemIndex;
	this->internalName          = std::move(internalName);
	this->shortName             = std::move(shortName);
	this->name                  = std::move(name);
	this->description           = std::move(description);
	this->bobbyRaysName         = std::move(bobbyRaysName);
	this->bobbyRaysDescription  = std::move(bobbyRaysDescription);
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

const ST::string& ItemModel::getInternalName() const   { return internalName;          }

const ST::string& ItemModel::getShortName() const      { return shortName; }
const ST::string& ItemModel::getName() const           { return name; }
const ST::string& ItemModel::getDescription() const     { return description; }
const ST::string& ItemModel::getBobbyRaysName() const           { return bobbyRaysName; }
const ST::string& ItemModel::getBobbyRaysDescription() const     { return bobbyRaysDescription; }

uint16_t        ItemModel::getItemIndex() const        { return itemIndex;             }
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
	if(getFlags() & ITEM_DAMAGEABLE)          { obj.set("bDamageable", true);     }
	if(getFlags() & ITEM_REPAIRABLE)          { obj.set("bRepairable", true);     }
	if(getFlags() & ITEM_WATER_DAMAGES)       { obj.set("bWaterDamages", true);   }
	if(getFlags() & ITEM_METAL)               { obj.set("bMetal", true);          }
	if(getFlags() & ITEM_SINKS)               { obj.set("bSinks", true);          }
	if(getFlags() & ITEM_SHOW_STATUS)         { obj.set("bShowStatus", true);     }
	if(getFlags() & ITEM_HIDDEN_ADDON)        { obj.set("bHiddenAddon", true);    }
	if(getFlags() & ITEM_TWO_HANDED)          { obj.set("bTwoHanded", true);      }
	if(getFlags() & ITEM_NOT_BUYABLE)         { obj.set("bNotBuyable", true);     }
	if(getFlags() & ITEM_ATTACHMENT)          { obj.set("bAttachment", true);     }
	if(getFlags() & ITEM_BIGGUNLIST)          { obj.set("bBigGunList", true);     }
	if(getFlags() & ITEM_NOT_EDITOR)          { obj.set("bNotEditor", true);      }
	if(getFlags() & ITEM_DEFAULT_UNDROPPABLE) { obj.set("bDefaultUndroppable", true); }
	if(getFlags() & ITEM_UNAERODYNAMIC)       { obj.set("bUnaerodynamic", true);  }
	if(getFlags() & ITEM_ELECTRONIC)          { obj.set("bElectronic", true);     }
	if(getFlags() & ITEM_INSEPARABLE)         { obj.set("bInseparable", true);    }
}

uint16_t ItemModel::deserializeFlags(const JsonObject &obj)
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
bool ItemModel::canBeAttached(const GamePolicy* policy, const ItemModel* attachment) const
{
	return false;
}

JsonValue ItemModel::serialize() const
{
	JsonObject obj;
    obj.set("itemIndex", itemIndex);
    obj.set("internalName", internalName);
    obj.set("usItemClass", (uint32_t)getItemClass());
    obj.set("ubClassIndex", getClassIndex());
    obj.set("ubCursor",  getCursor());
    obj.set("inventoryGraphics", inventoryGraphics.serialize());
	obj.set("tileGraphic", tileGraphic.serialize());
    obj.set("ubWeight", getWeight());
    obj.set("ubPerPocket", getPerPocket());
    obj.set("usPrice", getPrice());
    obj.set("ubCoolness", getCoolness());
    obj.set("bReliability", getReliability());
    obj.set("bRepairEase", getRepairEase());

    serializeFlags(obj);

	return obj.toValue();
}

ST::string ItemModel::deserializeShortName(InitData const& initData)
{
	using namespace ItemStrings;
	return deserializeHelper(initData, BINARY_STRING_FILE, "shortName", BINARY_SIZE_FULL_ITEM, 0, BINARY_SIZE_SHORT_ITEM_NAME);
}

ST::string ItemModel::deserializeName(InitData const& initData)
{
	using namespace ItemStrings;
	return deserializeHelper(initData, BINARY_STRING_FILE, "name", BINARY_SIZE_FULL_ITEM, BINARY_SIZE_SHORT_ITEM_NAME, BINARY_SIZE_ITEM_NAME);
}

ST::string ItemModel::deserializeDescription(InitData const& initData)
{
	using namespace ItemStrings;
	return deserializeHelper(initData, BINARY_STRING_FILE, "description", BINARY_SIZE_FULL_ITEM, BINARY_SIZE_SHORT_ITEM_NAME + BINARY_SIZE_ITEM_NAME, BINARY_SIZE_ITEM_INFO);
}

ST::string ItemModel::deserializeBobbyRaysName(InitData const& initData)
{
	using namespace ItemStrings;
	return deserializeHelper(initData, BINARY_STRING_BOBBYR_FILE, "bobbyRaysName", BINARY_SIZE_BOBBYR_FULL_ITEM, 0, BINARY_SIZE_BOBBYR_ITEM_NAME);
}

ST::string ItemModel::deserializeBobbyRaysDescription(InitData const& initData)
{
	using namespace ItemStrings;
	return deserializeHelper(initData, BINARY_STRING_BOBBYR_FILE, "bobbyRaysDescription", BINARY_SIZE_BOBBYR_FULL_ITEM, BINARY_SIZE_BOBBYR_ITEM_NAME, BINARY_SIZE_BOBBYR_ITEM_INFO);
}

const ItemModel* ItemModel::deserialize(const JsonValue &json, TranslatableString::Loader& stringLoader)
{
	auto obj = json.toObject();
	InitData const initData{ obj, stringLoader };
	uint16_t itemIndex = obj.GetUInt("itemIndex");
	ST::string internalName = obj.GetString("internalName");
	auto inventoryGraphics = InventoryGraphicsModel::deserialize(obj["inventoryGraphics"]);
	auto tileGraphic = TilesetTileIndexModel::deserialize(obj["tileGraphic"]);

	auto shortName = ItemModel::deserializeShortName(initData);
	auto name = ItemModel::deserializeName(initData);
	auto description = ItemModel::deserializeDescription(initData);
	auto bobbyRaysName = ItemModel::deserializeName(initData);
	auto bobbyRaysDescription = ItemModel::deserializeDescription(initData);
	auto flags = ItemModel::deserializeFlags(obj);

	return new ItemModel(
		itemIndex,
		std::move(internalName),
		std::move(shortName),
		std::move(name),
		std::move(description),
		std::move(bobbyRaysName),
		std::move(bobbyRaysDescription),
		obj.GetUInt("usItemClass"),
		obj.GetUInt("ubClassIndex"),
		(ItemCursor)obj.GetUInt("ubCursor"),
		std::move(inventoryGraphics),
		std::move(tileGraphic),
		obj.GetUInt("ubWeight"),
		obj.GetUInt("ubPerPocket"),
		obj.GetUInt("usPrice"),
		obj.GetUInt("ubCoolness"),
		obj.GetInt("bReliability"),
		obj.GetInt("bRepairEase"),
		flags
	);
}
