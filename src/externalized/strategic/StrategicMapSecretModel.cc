#include "StrategicMapSecretModel.h"
#include "JsonUtility.h"
#include "SamSiteModel.h"
#include <set>
#include <stdexcept>
#include <utility>

StrategicMapSecretModel::StrategicMapSecretModel(
	uint8_t sectorID_, bool isSAMSite_, ST::string secretMapIcon_, uint8_t secretLandType_, uint8_t foundLandType_
	) : sectorID(sectorID_), isSAMSite(isSAMSite_),
		secretMapIcon(std::move(secretMapIcon_)),
		secretLandType(secretLandType_ ), foundLandType(foundLandType_) {}

StrategicMapSecretModel* StrategicMapSecretModel::deserialize(const JsonValue& json, const TraversibilityMap &mapping)
{
	auto r = json.toObject();
	auto sector         = JsonUtility::parseSectorID(r["sector"]);
	auto landTypeString = r.GetString("secretLandType");
	auto secretLandType = mapping.at(landTypeString);

	landTypeString = r.GetString("foundLandType");
	auto foundLandType = mapping.at(landTypeString);

	return new StrategicMapSecretModel(
		sector,
		r.getOptionalBool("isSAMSite"),
		r.getOptionalString("secretMapIcon") ,
		secretLandType,
		foundLandType
	);
}

void StrategicMapSecretModel::validateData(const std::vector<const StrategicMapSecretModel *>& models, const std::vector<const SamSiteModel*>& samModels)
{
	std::set<uint8_t> samSiteLocations;
	for (auto s : samModels)
	{
		samSiteLocations.insert(s->sectorId);
	}

	int countNonSAMSites = 0;
	std::set<uint8_t> uniqueSectors;
	for (auto m : models)
	{
		// if it's a SAM site secret, ensure we have the corresponding SAM site
		if (m->isSAMSite)
		{
			if (samSiteLocations.find(m->sectorID) == samSiteLocations.end())
			{
				ST::string err = ST::format("No SAM site at sector {}", m->sectorID);
				throw std::runtime_error(err.to_std_string());
			}
		}
		else
		{
			countNonSAMSites++;
		}

		// must not have duplicated keys
		if (uniqueSectors.find(m->sectorID) != uniqueSectors.end())
		{
			ST::string err = ST::format("Sector {} has more than one secret", m->sectorID);
			throw std::runtime_error(err.to_std_string());
		}
		uniqueSectors.insert(m->sectorID);
	}

	if (countNonSAMSites > 2)
	{
		// Limitation for now, to maintain vanilla game compatibility.
		// If there is a need, we can use something like a bitset to provide more map secret slots in saves.
		SLOGW("There are more than 2 map secrets that are not SAM sites. Only the first 2 will be persisted in saves");
	}
}

uint8_t StrategicMapSecretModel::getLandType(bool isSecretFound) const
{
	return isSecretFound ? foundLandType : secretLandType;
}
