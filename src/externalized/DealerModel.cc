#include "DealerModel.h"
#include "Exceptions.h"


DealerModel::DealerModel(ArmsDealerID dealerID_, UINT8 mercID_, ArmsDealerType dealerType_,
	float buyingPrice_, float sellingPrice_, float repairSpeed_, float repairCost_,
	INT32 initialCash_, std::bitset<ArmsDealerFlag::NUM_FLAGS> flags_)
	: dealerID(dealerID_), profileID(mercID_), type(dealerType_),
		buyingPrice(buyingPrice_), sellingPrice(sellingPrice_), repairSpeed(repairSpeed_), repairCost(repairCost_),
		initialCash(initialCash_), flags(flags_) {}

ST::string DealerModel::getInventoryDataFileName(const MercSystem* mercSystem) const
{
	auto mercProfile = mercSystem->getMercProfileInfo(profileID);
	return ST::format("dealer-inventory-{}.json", mercProfile->internalName.to_lower());
}

BOOLEAN DealerModel::hasFlag(ArmsDealerFlag flag) const
{
	return flags.test(flag);
}

BOOLEAN DealerModel::doesRepairs() const
{
	return type == ARMS_DEALER_REPAIRS;
}

ArmsDealerType parseType(const ST::string& str)
{
	if (str == "BUYS_ONLY") return ArmsDealerType::ARMS_DEALER_BUYS_ONLY;
	if (str == "SELLS_ONLY") return ArmsDealerType::ARMS_DEALER_SELLS_ONLY;
	if (str == "BUYS_SELLS") return ArmsDealerType::ARMS_DEALER_BUYS_SELLS;
	if (str == "REPAIRS") return ArmsDealerType::ARMS_DEALER_REPAIRS;

	throw DataError(ST::format("Unrecognized dealer type: '{}'", str));
}

ArmsDealerFlag parseFlag(const ST::string& str)
{
	if (str == "ONLY_USED_ITEMS") return ArmsDealerFlag::ONLY_USED_ITEMS;
	if (str == "SOME_USED_ITEMS") return ArmsDealerFlag::SOME_USED_ITEMS;
	if (str == "GIVES_CHANGE") return ArmsDealerFlag::GIVES_CHANGE;
	if (str == "ACCEPTS_GIFTS") return ArmsDealerFlag::ACCEPTS_GIFTS;
	if (str == "HAS_NO_INVENTORY") return ArmsDealerFlag::HAS_NO_INVENTORY;
	if (str == "DELAYS_REPAIR") return ArmsDealerFlag::DELAYS_REPAIR;
	if (str == "REPAIRS_ELECTRONICS") return ArmsDealerFlag::REPAIRS_ELECTRONICS;
	if (str == "SELLS_ALCOHOL") return ArmsDealerFlag::SELLS_ALCOHOL;
	if (str == "SELLS_FUEL") return ArmsDealerFlag::SELLS_FUEL;
	if (str == "BUYS_EVERYTHING") return ArmsDealerFlag::BUYS_EVERYTHING;

	throw DataError(ST::format("Unrecognized dealer flag: '{}'", str));
}

const DealerModel* DealerModel::deserialize(const JsonValue& json, const MercSystem* mercSystem, UINT8 dealerIndex)
{
	auto obj = json.toObject();
	std::bitset<ArmsDealerFlag::NUM_FLAGS> flags;
	auto flagsArray = obj["flags"].toVec();
	for (auto& fl : flagsArray)
	{
		UINT8 flagIndex = static_cast<UINT8>(parseFlag(fl.toString()));
		flags.set(flagIndex);
	}

	ST::string profile = obj.GetString("profile");
	auto mercProfile = mercSystem->getMercProfileInfoByName(profile);
	if (mercProfile == NULL) {
		throw DataError(ST::format("dealer `{}` does not refer to a valid profile.", profile));
	}
	auto profileId = mercProfile->profileID;

	return new DealerModel(
		static_cast<ArmsDealerID>(dealerIndex),
		profileId,
		parseType(obj.GetString("type")),
		obj.getOptionalDouble("buyingPrice"),
		obj.getOptionalDouble("sellingPrice"),
		obj.getOptionalDouble("repairSpeed"),
		obj.getOptionalDouble("repairCost"),
		obj.GetInt("initialCash"),
		flags
	);
}

void DealerModel::validateData(std::vector<const DealerModel*> models, const MercSystem* mercSystem)
{
	if (models.size() < NUM_ARMS_DEALERS)
	{
		// because of we still have hard-coded references, and also for save compatibility with vanilla
		throw DataError(ST::format("Number of Arms Dealers must be {}. Got {}", NUM_ARMS_DEALERS, models.size()));
	}

	int i = -1;
	for (auto dealer: models)
	{
		auto mercProfile = mercSystem->getMercProfileInfo(dealer->profileID);
		auto profile = mercProfile->internalName;

		if (dealer->dealerID != ++i)
		{
			ST::string err = ST::format("Inconsistent dealer ID for dealer `{}`. Expected {} but got {}", profile, i, dealer->dealerID);
			throw DataError(err.to_std_string());
		}

		if (dealer->type == ArmsDealerType::ARMS_DEALER_REPAIRS
			&& (dealer->repairCost == 0.0 || dealer->repairSpeed == 0.0))
		{
			throw DataError(ST::format("Dealer `{}` is a repairman, but repair cost or speed is not set", profile));
		}
		else if (dealer->type == ArmsDealerType::ARMS_DEALER_BUYS_ONLY
			&& (dealer->buyingPrice == 0.0))
		{
			throw DataError(ST::format("Dealer `{}` is a buyer, but buying price is not set", profile));
		}
		else if (dealer->type == ArmsDealerType::ARMS_DEALER_SELLS_ONLY
			&& (dealer->sellingPrice == 0.0))
		{
			throw DataError(ST::format("Dealer `{}` is a seller, but selling price is not set", profile));
		}
		else if (dealer->type == ArmsDealerType::ARMS_DEALER_BUYS_ONLY
			&& (dealer->buyingPrice == 0.0 || dealer->sellingPrice == 0.0))
		{
			throw DataError(ST::format("Dealer `{}` is a trader, but buying price or selling price is not set", profile));
		}
	}
}

