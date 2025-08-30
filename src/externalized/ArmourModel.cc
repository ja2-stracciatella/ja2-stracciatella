#include "ArmourModel.h"
#include "Exceptions.h"
#include "GamePolicy.h"
#include "Item_Types.h"
#include "Weapons.h"
#include <cstdint>
#include <set>

static std::set<UINT16> const platesCanBeAttached = {
	FLAK_JACKET,
	FLAK_JACKET_18,
	FLAK_JACKET_Y,
	KEVLAR_VEST,
	KEVLAR_VEST_18,
	KEVLAR_VEST_Y,
	KEVLAR2_VEST,
	KEVLAR2_VEST_18,
	KEVLAR2_VEST_Y,
	SPECTRA_VEST,
	SPECTRA_VEST_18,
	SPECTRA_VEST_Y
};

uint8_t deserializeArmourClass(const ST::string& armourClass) {
	if (armourClass == "HELMET") return ARMOURCLASS_HELMET;
	if (armourClass == "VEST") return ARMOURCLASS_VEST;
	if (armourClass == "LEGGINGS") return ARMOURCLASS_LEGGINGS;
	if (armourClass == "PLATE") return ARMOURCLASS_PLATE;
	if (armourClass == "CREATURE") return ARMOURCLASS_MONST;
	throw DataError(ST::format("Unknown armour class '{}'", armourClass));
}

// additional possible attachments if the extra_attachments game policy is set
static std::set<UINT16> const g_helmets {STEEL_HELMET, KEVLAR_HELMET, KEVLAR_HELMET_18, KEVLAR_HELMET_Y, SPECTRA_HELMET, SPECTRA_HELMET_18, SPECTRA_HELMET_Y};
static std::set<UINT16> const g_leggings {KEVLAR_LEGGINGS, KEVLAR_LEGGINGS_18, KEVLAR_LEGGINGS_Y, SPECTRA_LEGGINGS, SPECTRA_LEGGINGS_18, SPECTRA_LEGGINGS_Y};
static std::map<UINT16, decltype(g_helmets) *> const g_attachments_mod
{
	{NIGHTGOGGLES, &g_helmets},
	{UVGOGGLES, &g_helmets},
	{SUNGOGGLES, &g_helmets},
	{ROBOT_REMOTE_CONTROL, &g_helmets},

	{BREAK_LIGHT, &g_leggings},
	{REGEN_BOOSTER, &g_leggings},
	{ADRENALINE_BOOSTER, &g_leggings}
};

ArmourModel::ArmourModel(
			uint16_t itemIndex,
			ST::string&& internalName,
			ST::string&& shortName,
			ST::string&& name,
			ST::string&& description,
			InventoryGraphicsModel&& inventoryGraphics,
			TilesetTileIndexModel&& tileGraphic,
			uint8_t weight,
			uint8_t perPocket,
			uint16_t price,
			uint8_t coolness,
			int8_t reliability,
			int8_t repairEase,
			uint32_t flags,
			uint8_t armourClass,
			uint8_t protection,
			uint8_t explosivesProtection,
			uint8_t degradePercentage,
			bool ignoreForMaxProtection
	) : ItemModel(itemIndex, std::move(internalName), std::move(shortName), std::move(name), std::move(description), IC_ARMOUR, 0, INVALIDCURS, std::move(inventoryGraphics), std::move(tileGraphic), weight, perPocket, price, coolness, reliability, repairEase, flags) {
	this->armourClass = armourClass;
	this->protection = protection;
	this->explosivesProtection = explosivesProtection;
	this->degradePercentage = degradePercentage;
	this->ignoreForMaxProtection = ignoreForMaxProtection;
}

ArmourModel* ArmourModel::deserialize(const JsonValue &json, const BinaryData& vanillaItemStrings, bool extraAttachmentsEnabled) {
	auto obj = json.toObject();
	ItemModel::InitData const initData{ obj, vanillaItemStrings };

	int itemIndex = obj.GetInt("itemIndex");
	ST::string internalName = obj.GetString("internalName");
	auto shortName = ItemModel::deserializeShortName(initData);
	auto name = ItemModel::deserializeName(initData);
	auto description = ItemModel::deserializeDescription(initData);
	auto flags = ItemModel::deserializeFlags(obj);
	auto inventoryGraphics = InventoryGraphicsModel::deserialize(obj["inventoryGraphics"]);
	auto tileGraphic = TilesetTileIndexModel::deserialize(obj["tileGraphic"]);
	auto armourClass = deserializeArmourClass(obj.GetString("armourClass"));
	auto protection = obj.GetUInt("protection");
	auto explosivesProtection = obj.getOptionalUInt("explosivesProtection", protection);
	auto degradePercentage = obj.GetUInt("degradePercentage");
	auto ignoreForMaxProtection = obj.getOptionalBool("ignoreForMaxProtection", false);

	return new ArmourModel(
		itemIndex,
		std::move(internalName),
		std::move(shortName),
		std::move(name),
		std::move(description),
		std::move(inventoryGraphics),
		std::move(tileGraphic),
		obj.GetUInt("ubWeight"),
		obj.GetUInt("ubPerPocket"),
		obj.GetUInt("usPrice"),
		obj.GetUInt("ubCoolness"),
		obj.GetInt("bReliability"),
		obj.GetInt("bRepairEase"),
		flags,
		armourClass,
		protection,
		explosivesProtection,
		degradePercentage,
		ignoreForMaxProtection
	);
}

uint8_t ArmourModel::getArmourClass() const {
	return armourClass;
}

uint8_t ArmourModel::getProtection() const {
	return protection;
}

uint8_t ArmourModel::getExplosivesProtection() const {
	return explosivesProtection;
}

uint8_t ArmourModel::getDegradePercentage() const {
	return degradePercentage;
}

bool ArmourModel::isIgnoredForMaxProtection() const {
	return ignoreForMaxProtection;
}

bool ArmourModel::canBeAttached(const GamePolicy* policy, const ItemModel* attachment) const {
	auto attachmentAsArmour = attachment->asArmour();
	if (attachmentAsArmour && attachmentAsArmour->getArmourClass() == ARMOURCLASS_PLATE) {
		return platesCanBeAttached.find(this->itemIndex) != platesCanBeAttached.end();
	}
	if (policy->extra_attachments)
	{
		auto const it = g_attachments_mod.find(attachment->getItemIndex());
		if (it != g_attachments_mod.end() && (*it->second).count(this->itemIndex) == 1) return true;
	}
	return false;
}
