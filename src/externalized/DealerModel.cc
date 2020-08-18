#include "DealerModel.h"

#include "JsonObject.h"


DealerModel::DealerModel(UINT8 dealerID_, UINT8 mercID_, ST::string internalName_, ArmsDealerType dealerType_, 
	FLOAT buyingPrice_, FLOAT sellingPrice_, FLOAT repairSpeed_, FLOAT repairCost_, 
	INT32 initialCash_, std::bitset<ArmsDealerFlag::NUM_FLAGS> flags_)
	: dealerID(dealerID_), profileID(mercID_), internalName(internalName_), type(dealerType_), 
		buyingPrice(buyingPrice_), sellingPrice(sellingPrice_), repairSpeed(repairSpeed_), repairCost(repairCost_),
		initialCash(initialCash_), flags(flags_) {}

ST::string DealerModel::getInventoryDataFileName() const
{
	return ST::format("dealer-inventory-{}.json", internalName.to_lower());
}

BOOLEAN DealerModel::hasFlag(ArmsDealerFlag flag) const
{
	return flags.test(flag);
}

BOOLEAN DealerModel::doesRepairs() const
{
	return type == ARMS_DEALER_REPAIRS;
}

ArmsDealerType parseType(const std::string str)
{
	if (str == "BUYS_ONLY") return ArmsDealerType::ARMS_DEALER_BUYS_ONLY;
	if (str == "SELLS_ONLY") return ArmsDealerType::ARMS_DEALER_SELLS_ONLY;
	if (str == "BUYS_SELLS") return ArmsDealerType::ARMS_DEALER_BUYS_SELLS;
	if (str == "REPAIRS") return ArmsDealerType::ARMS_DEALER_REPAIRS;

	ST::string err = ST::format("Unrecognized dealer type: '{}'", str);
	throw std::runtime_error(err.to_std_string());
}

ArmsDealerFlag parseFlag(const std::string str)
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
	
	ST::string err = ST::format("Unrecognized dealer flag: '{}'", str);
	throw std::runtime_error(err.to_std_string());
}

const DealerModel* DealerModel::deserialize(rapidjson::Value& val, UINT8 dealerIndex)
{
	std::bitset<ArmsDealerFlag::NUM_FLAGS> flags;
	auto flagsArray = val["flags"].GetArray();
	for (auto& fl : flagsArray)
	{
		UINT8 flagIndex = static_cast<UINT8>(parseFlag(fl.GetString()));
		flags.set(flagIndex);
	}

	JsonObjectReader obj(val);
	return new DealerModel(
		dealerIndex,
		obj.GetUInt("profileID"),
		obj.GetString("internalName"),
		parseType(obj.GetString("type")),
		obj.getOptionalDouble("buyingPrice"),
		obj.getOptionalDouble("sellingPrice"),
		obj.getOptionalDouble("repairSpeed"),
		obj.getOptionalDouble("repairCost"),
		obj.GetInt("initialCash"),
		flags
	);
}

void DealerModel::validateData(std::vector<const DealerModel*> models)
{
	if (models.size() < NUM_ARMS_DEALERS)
	{
		// because of we still have hard-coded references, and also for save compatibility with vanilla
		ST::string err = ST::format("Number of Arms Dealers must be {}. Got {}", NUM_ARMS_DEALERS, models.size());
		throw std::runtime_error(err.to_std_string());
	}

	int i = -1;
	for (auto dealer: models)
	{
		if (dealer->dealerID != ++i)
		{
			ST::string err = ST::format("Inconsistent Dealer ID. Expected {} but got {}", i, dealer->dealerID);
			throw std::runtime_error(err.to_std_string());
		}

		if (dealer->type == ArmsDealerType::ARMS_DEALER_REPAIRS 
			&& (dealer->repairCost == 0.0 || dealer->repairSpeed == 0.0))
		{
			ST::string err = ST::format("Dealer #{} is a repairman, but repair cost or speed is not set", dealer->dealerID);
			throw std::runtime_error(err.to_std_string());
		}
		else if (dealer->type == ArmsDealerType::ARMS_DEALER_BUYS_ONLY 
			&& (dealer->buyingPrice == 0.0))
		{
			ST::string err = ST::format("Dealer is a buyer, but buying price is not set", dealer->dealerID);
			throw std::runtime_error(err.to_std_string());
		}
		else if (dealer->type == ArmsDealerType::ARMS_DEALER_SELLS_ONLY 
			&& (dealer->sellingPrice == 0.0))
		{
			ST::string err = ST::format("Dealer #{} is a seller, but selling price is not set", dealer->dealerID);
			throw std::runtime_error(err.to_std_string());
		}
		else if (dealer->type == ArmsDealerType::ARMS_DEALER_BUYS_ONLY 
			&& (dealer->buyingPrice == 0.0 || dealer->sellingPrice == 0.0))
		{
			ST::string err = ST::format("Dealer #{} is a trader, but buying price or selling price is not set", dealer->dealerID);
			throw std::runtime_error(err.to_std_string());
		}
	}
}

