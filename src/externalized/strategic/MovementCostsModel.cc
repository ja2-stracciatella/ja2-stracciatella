#include "MovementCostsModel.h"
#include "Debug.h"
#include "JsonObject.h"

#include <string_theory/string>

#include <map>


// helper functions
void readTraversibiltyIntoVector(const rapidjson::Value& jsonArray, IntIntVector& vec, std::map<ST::string, uint8_t>& mapping, size_t expectedRows, size_t expectedCols);
void readIntIntoVector(const rapidjson::Value& jsonArray, IntIntVector& vec, size_t expectedRows, size_t expectedCols);


MovementCostsModel::MovementCostsModel(IntIntVector traverseWE_, IntIntVector traverseNS_, IntIntVector traverseThrough_, IntIntVector travelRatings_)
	:traverseWE(traverseWE_), traverseNS(traverseNS_), traverseThrough(traverseThrough_), travelRatings(travelRatings_) {}

const uint8_t MovementCostsModel::getTraversibilityWestEast(uint8_t x, uint8_t y) const
{
	Assert(y < 16 && x <= 16);
	return traverseWE[y][x];
}

const uint8_t MovementCostsModel::getTraversibilityNorthSouth(uint8_t x, uint8_t y) const
{
	Assert(y <= 16 && x < 16);
	return traverseNS[y][x];
}

const uint8_t MovementCostsModel::getTraversibilityThrough(uint8_t x, uint8_t y) const
{
	Assert(y < 16 && x < 16);
	return traverseThrough[y][x];
}

const uint8_t MovementCostsModel::getTravelRating(uint8_t x, uint8_t y) const
{
	Assert(y < 16 && x < 16);
	return travelRatings[y][x];
}

MovementCostsModel* MovementCostsModel::deserialize(const rapidjson::Document& root)
{
	Assert(root.HasMember("mapping") && root["mapping"].IsObject());
	std::map<ST::string, uint8_t> mapToEnum; // maps from traversibility short name to enum
	for (auto& iter : root["mapping"].GetObject())
	{
		mapToEnum.insert(std::make_pair(ST::string(iter.name.GetString()), iter.value.GetInt()));
	}

	Assert(root.HasMember("traverseWE") && root["traverseWE"].IsArray());
	IntIntVector traverseWE_;
	readTraversibiltyIntoVector(root["traverseWE"], traverseWE_, mapToEnum, 16, 17);

	Assert(root.HasMember("traverseNS") && root["traverseNS"].IsArray());
	IntIntVector traverseNS_;
	readTraversibiltyIntoVector(root["traverseNS"], traverseNS_, mapToEnum, 17, 16);

	Assert(root.HasMember("traverseThrough") && root["traverseThrough"].IsArray());
	IntIntVector traverseThrough_;
	readTraversibiltyIntoVector(root["traverseThrough"], traverseThrough_, mapToEnum, 16, 16);

	Assert(root.HasMember("travelRatings") && root["travelRatings"].IsArray());
	IntIntVector travelRatings_;
	readIntIntoVector(root["travelRatings"], travelRatings_, 16, 16);

	return new MovementCostsModel(
		traverseWE_,
		traverseNS_,
		traverseThrough_,
		travelRatings_
	);
}

void readTraversibiltyIntoVector(const rapidjson::Value& jsonArray, IntIntVector& vec, std::map<ST::string, uint8_t>& mapping, size_t expectedRows, size_t expectedCols)
{
	for (auto& r : jsonArray.GetArray())
	{
		auto row = std::vector<uint8_t>();
		for (auto& c : r.GetArray())
		{
			row.push_back(mapping.at(ST::string(c.GetString())));
		}
		
		if (row.size() != expectedCols) {
			SLOGE("num of columns is different from expected");
			throw std::runtime_error("num of columns is different from expected");
		}
		vec.push_back(row);
	}
	if (vec.size() != expectedRows) {
		SLOGE("num of rows is different from expected");
		throw std::runtime_error("num of rows is different from expected");
	}
}

void readIntIntoVector(const rapidjson::Value& jsonArray, IntIntVector& vec, size_t expectedRows, size_t expectedCols)
{
	for (auto& r : jsonArray.GetArray())
	{
		auto row = std::vector<uint8_t>();
		for (auto& c : r.GetArray())
		{
			row.push_back(c.GetInt());
		}

		if (row.size() != expectedCols) {
			SLOGE("num of columns is different from expected");
			throw std::runtime_error("num of columns is different from expected");
		}
		vec.push_back(row);
	}
	if (vec.size() != expectedRows) {
		SLOGE("num of rows is different from expected");
		throw std::runtime_error("num of row is different from expected");
	}
}