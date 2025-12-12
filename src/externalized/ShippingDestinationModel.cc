#include "ShippingDestinationModel.h"
#include "TranslatableString.h"
#include <cstdint>
#include <memory>

const ST::string ShippingDestinationModel::NAME_TRANSLATION_PREFIX = "strings/shipping-destinations";

ShippingDestinationModel::ShippingDestinationModel(uint8_t locationId_, ST::string&& internalName_, ST::string&& name_,
	uint16_t chargeRateOverNight_, uint16_t chargeRate2Days_, uint16_t chargeRateStandard_,
	uint8_t flowersNextDayDeliveryCost_, uint8_t flowersWhenItGetsThereCost_,
	bool canDeliver_, bool isPrimary_,
	uint8_t deliverySectorId_, uint8_t deliverySectorZ_, int16_t deliverySectorGridNo_,
	int16_t emailOffset_, int16_t emailLength_):
		locationId(locationId_), internalName(internalName_), name(name_),
		chargeRateOverNight(chargeRateOverNight_), chargeRate2Days(chargeRate2Days_), chargeRateStandard(chargeRateStandard_),
		flowersNextDayDeliveryCost(flowersNextDayDeliveryCost_), flowersWhenItGetsThereCost(flowersWhenItGetsThereCost_),
		canDeliver(canDeliver_), isPrimary(isPrimary_),
		deliverySector(SGPSector::FromSectorID(deliverySectorId_, deliverySectorZ_)),
		deliverySectorGridNo(deliverySectorGridNo_), emailOffset(emailOffset_), emailLength(emailLength_) {}

uint8_t ShippingDestinationModel::getDeliverySector() const
{
	return deliverySector.AsByte();
}

ShippingDestinationModel* ShippingDestinationModel::deserialize(const JsonValue& json, TranslatableString::Loader& stringLoader)
{
	auto obj = json.toObject();
	uint8_t destSectorId = 0, destSectorZ = 0;
	int16_t destGridNo = 0;
	bool isPrimary = false;
	bool canDeliver = obj.getOptionalBool("canDeliver");
	if (canDeliver) {
		destSectorId = SGPSector::FromShortString(obj.GetString("deliverySector")).AsByte();
		destSectorZ = obj.GetInt("deliverySectorZ");
		destGridNo = obj.GetInt("deliverySectorGridNo");
		isPrimary = obj.getOptionalBool("isPrimary");
	}
	auto locationId = static_cast<uint8_t>(obj.GetInt("locationId"));
	std::unique_ptr<TranslatableString::String> translatableName = std::make_unique<TranslatableString::Json>(NAME_TRANSLATION_PREFIX, locationId);
	if (obj.has("name")) {
		translatableName = TranslatableString::String::parse(obj.GetValue("name"));
	}

	return new ShippingDestinationModel(
		locationId, obj.GetString("internalName"), translatableName->resolve(stringLoader),
		obj.GetInt("chargeRateOverNight"), obj.GetInt("chargeRate2Days"), obj.GetInt("chargeRateStandard"),
		obj.getOptionalInt("flowersNextDayDeliveryCost"), obj.getOptionalInt("flowersWhenItGetsThereCost"),
		canDeliver, isPrimary,
		destSectorId, destSectorZ, destGridNo,
		obj.getOptionalInt("emailOffset"), obj.getOptionalInt("emailLength")
	);
}

void ShippingDestinationModel::validateData(std::vector<const ShippingDestinationModel*> destinations)
{
	int numPrimaryDestinations = 0;
	for (size_t i = 0; i < destinations.size(); i++)
	{
		auto dest = destinations[i];
		if (dest->locationId != i)
		{
			ST::string err = ST::format("Wrong locationId at position {}. Got {}.", i, dest->locationId);
			throw std::runtime_error(err.to_std_string());
		}
		if (dest->isPrimary) numPrimaryDestinations++;
	}

	if (numPrimaryDestinations != 1)
	{
		ST::string err = ST::format("There must be exactly 1 primary Shipping Destination. Got {}.", numPrimaryDestinations);
		throw std::runtime_error(err.to_std_string());
	}
}
