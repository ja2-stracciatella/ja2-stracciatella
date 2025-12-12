#pragma once

#include "Json.h"
#include "TranslatableString.h"
#include "Types.h"

class ShippingDestinationModel
{
public:
	ShippingDestinationModel(uint8_t locationId_, ST::string&& internalName, ST::string&& name,
		uint16_t chargeRateOverNight_, uint16_t chargeRate2Days_, uint16_t chargeRateStandard_,
		uint8_t flowersNextDayDeliveryCost_, uint8_t flowersWhenItGetsThereCost_,
		bool canDeliver_, bool isPrimary_,
		uint8_t deliverySectorId_, uint8_t deliverySectorZ_, int16_t deliverySectorGridNo_,
		int16_t emailOffset_, int16_t emailLength_);

	uint8_t getDeliverySector() const;

	static ShippingDestinationModel* deserialize(const JsonValue& json, TranslatableString::Loader& stringLoader);
	static void validateData(std::vector<const ShippingDestinationModel*> destinations);

	const uint8_t locationId;
	const ST::string internalName;
	const ST::string name;

	const uint16_t chargeRateOverNight;
	const uint16_t chargeRate2Days;
	const uint16_t chargeRateStandard;

	const uint8_t flowersNextDayDeliveryCost;
	const uint8_t flowersWhenItGetsThereCost;

	const bool canDeliver;
	const bool isPrimary;

	const SGPSector deliverySector;
	const int16_t deliverySectorGridNo;
	const int32_t emailOffset;
	const int32_t emailLength;
private:
	static const ST::string NAME_TRANSLATION_PREFIX;
};
