#include "SamSiteAirControlModel.h"

#include <stdexcept>


SamSiteAirControlModel::SamSiteAirControlModel(std::array<int8_t, 256>& controlTable_)
	:airControlTable(controlTable_) {}

int8_t SamSiteAirControlModel::getControllingSamSiteID(uint16_t const sectorId) const
{
	if (sectorId > NUM_SECTORS)
	{
		ST::string err = ST::format("sectorId given ({})is not valid", sectorId);
		throw std::invalid_argument(err.to_std_string());
	}
	return airControlTable[sectorId] - 1;
}

SamSiteAirControlModel* SamSiteAirControlModel::deserialize(const rapidjson::Value& obj)
{
	auto table = obj.GetArray();
	if (table.Size() != 16)
	{
		throw std::runtime_error("The SAM control table must have exactly 16 rows");
	}

	int i = 0;
	std::array<int8_t, NUM_SECTORS> controlTable{};
	for (auto& el: table)
	{
		auto row = el.GetArray();
		if (row.Size() != 16)
		{
			throw std::runtime_error("Every row in the SAM control table must have exactly 16 values");
		}
		for (auto& col : row)
		{
			controlTable[i++] = static_cast<int8_t>(col.GetInt());
		}
	}

	return new SamSiteAirControlModel(controlTable);
}

void SamSiteAirControlModel::validateData(const SamSiteAirControlModel* model, int numSamSites)
{
	for (auto samID : model->airControlTable)
	{
		if (samID > numSamSites)
		{
			ST::string err = ST::format("Value '{}' in the SAM air control table does not refer to a valid SAM site.", samID);
			throw std::runtime_error(err.to_std_string());
		}
	}
}
