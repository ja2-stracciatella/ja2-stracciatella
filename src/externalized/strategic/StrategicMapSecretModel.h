#pragma once

#include "Json.h"
#include "TraversibilityMapping.h"

#include <vector>

class SamSiteModel;

class StrategicMapSecretModel
{
public:
	StrategicMapSecretModel(uint8_t sectorID_, bool isSAMSite_, ST::string secretMapIcon_, uint8_t secretLandType_, uint8_t foundLandType_);
	static StrategicMapSecretModel* deserialize(const JsonValue& json, const TraversibilityMap& mapping);
	static void validateData(const std::vector<const StrategicMapSecretModel*>& models, const std::vector<const SamSiteModel*>& samModels);

	uint8_t getLandType(bool isSecretFound) const;

	const uint8_t sectorID;
	const bool isSAMSite;
	const ST::string secretMapIcon;

protected:
	const uint8_t secretLandType;
	const uint8_t foundLandType;
};
