#include "MineModel.h"

#include "JsonUtility.h"
#include "Strategic_Mines.h"
#include <utility>

MineModel::MineModel(const uint8_t mineId_, const uint8_t entranceSector_, const uint8_t associatedTownId_,
	const uint8_t mineType_, const uint16_t minimumMineProduction_, const bool headMinerAssigned_,
	const bool noDepletion_, const bool delayDepletion_,
	std::vector<std::array<uint8_t, 2>> mineSectors_, const int16_t faceDisplayYOffset_) :
		mineId(mineId_), entranceSector(entranceSector_), associatedTownId(associatedTownId_),
		mineType(mineType_), minimumMineProduction(minimumMineProduction_), headMinerAssigned(headMinerAssigned_),
		noDepletion(noDepletion_), delayDepletion(delayDepletion_),
		mineSectors(std::move(mineSectors_)), faceDisplayYOffset(faceDisplayYOffset_) {}

bool MineModel::isAbandoned() const
{
	return minimumMineProduction == 0;
}

uint8_t mineTypeFromString(const ST::string& mineType)
{
	if (mineType == "GOLD_MINE") return GOLD_MINE;
	if (mineType == "SILVER_MINE") return SILVER_MINE;

	ST::string err = ST::format("Unrecognized mine type: '{}'", mineType);
	throw std::runtime_error(err.to_std_string());
}

std::vector<std::array<uint8_t, 2>> readMineSectors(const JsonValue& sectorsJson)
{
	std::vector<std::array<uint8_t, 2>> sectors;
	for (auto& s: sectorsJson.toVec())
	{
		auto sector = s.toVec();
		if (sector.size() != 2) throw std::runtime_error("Elements in mineSector must be arrays of 2");

		auto& sectorString = sector[0];
		auto sectorLevel = sector[1].toUInt();

		sectors.push_back(std::array<uint8_t, 2> {
			JsonUtility::parseSectorID(sectorString),
			static_cast<uint8_t>(sectorLevel)
		});
	}
	return sectors;
}

MineModel* MineModel::deserialize(uint8_t index, const JsonValue& json)
{
	auto obj = json.toObject();
	auto entrance = JsonUtility::parseSectorID(obj["entranceSector"]);
	auto mineSectors = readMineSectors(obj["mineSectors"]);

	return new MineModel(
		index,
		entrance,
		obj.GetUInt("associatedTownId"),
		mineTypeFromString(obj.GetString("mineType")),
		obj.GetUInt("minimumMineProduction"),
		obj.getOptionalBool("headMinerAssigned"),
		obj.getOptionalBool("noDepletion"),
		obj.getOptionalBool("delayDepletion"),
		mineSectors,
		obj.getOptionalInt("faceDisplayYOffset")
	);
}

void MineModel::validateData(std::vector<const MineModel*>& models)
{
	if (models.size() != MAX_NUMBER_OF_MINES)
	{
		SLOGW("There are {} mines defined. Expected {} mines. This breaks vanilla save compatability", models.size(), MAX_NUMBER_OF_MINES);
	}
	if (models.size() < MAX_NUMBER_OF_MINES)
	{
		// Mine-related quests use hard-coded enums and may run into problems.
		SLOGW("There are fewer than {} mines defined.", MAX_NUMBER_OF_MINES);
	}
}
