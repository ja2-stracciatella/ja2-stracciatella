#include "SamSiteModel.h"

#include "Campaign_Types.h"
#include "StrategicMap.h"
#include "WorldDef.h"

#include <algorithm>


const int8_t SAM_GRAPHIC_INDEX_NE_SW = 3;  // SAM computer terminal graphic for the "/" orientation
const int8_t SAM_GRAPHIC_INDEX_NW_SE = 4;  // SAM computer terminal graphic for the "\" orientation

SamSiteModel::SamSiteModel(uint8_t sectorId_, std::array<GridNo, 2> gridNos_)
	: sectorId(sectorId_), gridNos(gridNos_) 
{
	Assert(gridNos[0] > gridNos[1]);
	graphicIndex = (gridNos[0] - gridNos[1] == WORLD_COLS) ? SAM_GRAPHIC_INDEX_NE_SW : SAM_GRAPHIC_INDEX_NW_SE;
}

const bool SamSiteModel::doesSamExistHere(INT16 const x, INT16 const y, GridNo const gridNo) const
{
	return x == SECTORX(sectorId) 
		&& y == SECTORY(sectorId) 
		&& std::find(gridNos.begin(), gridNos.end(), gridNo) != gridNos.end()
	;
}

SamSiteModel* SamSiteModel::deserialize(const rapidjson::Value& obj)
{
	const char* sector = obj["sector"].GetString();
	if (!IS_VALID_SECTOR_SHORT_STRING(sector))
	{
		throw std::runtime_error("SAM site sector is not a valid string");
	}
	uint8_t sectorId = SECTOR_FROM_SECTOR_SHORT_STRING(sector);

	auto g = obj["gridNos"].GetArray();
	if (g.Size() != 2)
	{
		throw std::runtime_error("SAM site gridNos must be an array of 2 integers");
	}
	std::array<GridNo, 2> gridNoList = {
		static_cast<GridNo>(g[0].GetInt()),
		static_cast<GridNo>(g[1].GetInt()),
	};

	// sort descending, so gridNos[0] is always the lower tile
	std::sort(gridNoList.begin(), gridNoList.end(), std::greater<int>());

	auto diff = gridNoList[0] - gridNoList[1];
	if (diff != 1 && diff != WORLD_COLS)
	{
		throw std::runtime_error("SAM site gridNos must be 2 adjacent tiles");
	}

	return new SamSiteModel(sectorId, gridNoList);
}

void SamSiteModel::validateData(const std::vector<const SamSiteModel*>& models)
{
	if (models.size() != NUMBER_OF_SAMS)
	{
		// Game saves, Skyrider and Meanwhile dialogues all assume 4 SAM sites
		SLOGE(ST::format("There must be exactly {} SAM sites defined", NUMBER_OF_SAMS));
		throw std::runtime_error("Unexpected number of SAM sites");
	}
}
