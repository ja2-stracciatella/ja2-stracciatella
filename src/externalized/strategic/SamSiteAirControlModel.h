#pragma once

#include "JA2Types.h"

#include <array>
#include <rapidjson/document.h>

#define NUM_SECTORS 256

class SamSiteAirControlModel
{
public:
	SamSiteAirControlModel(std::array<int8_t, NUM_SECTORS>& controlTable_);

	// returns the ID of the SAM site covering the sector, or -1 if not covered by any
	int8_t getControllingSamSiteID(uint16_t sectorId) const;

	static SamSiteAirControlModel* deserialize(const rapidjson::Value& obj);
	static void validateData(const SamSiteAirControlModel* model, int numSamSites);

protected:
	// the controlling SAM of each sector; values are the ID+1 of the controlling SAM, or 0 if not covered by any.
	std::array<int8_t, NUM_SECTORS> airControlTable;
};
