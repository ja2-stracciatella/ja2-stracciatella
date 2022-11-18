#include "JA2Types.h"
#include "JsonObject.h"

#include <array>

class SamSiteModel
{
public:
	SamSiteModel(uint8_t sectorId_, std::array<GridNo, 2> gridNos_);
	bool doesSamExistHere(const SGPSector& sector, GridNo const gridno) const;

	static SamSiteModel* deserialize(const rapidjson::Value& obj);
	static void validateData(const std::vector<const SamSiteModel*>& models);

	uint8_t sectorId;

	// 2 gridNo of the SAM computer terminal. Always sorted in descending order, so the first element is the "anchor point".
	std::array<GridNo, 2> gridNos;

	//Use 3 if / orientation, 4 if \ orientation
	int8_t graphicIndex;
};
