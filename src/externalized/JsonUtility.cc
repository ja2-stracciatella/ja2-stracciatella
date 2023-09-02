#include "JsonUtility.h"
#include "SGPFile.h"
#include "FileMan.h"

#include <array>

/** Write list of strings to file. */
bool JsonUtility::writeToFile(const ST::string &name, const std::vector<ST::string> &strings)
{
	JsonArray arr;
	for (auto& str : strings) {
		arr.push(str);
	}
	auto buf = arr.toValue().serialize() + "\n";
	try {
		AutoSGPFile file{FileMan::openForWriting(name)};

		file->write(buf.c_str(), buf.size());
	} catch (const std::runtime_error& ex) {
		SLOGE("JsonUtility::writeToFile: {}", ex.what());
		return false;
	}
	return true;
}

/** Parse json to a list of strings. */
bool JsonUtility::parseJsonToListStrings(const char* jsonData, std::vector<ST::string> &strings)
{
	auto val = JsonValue::deserialize(jsonData);

	return parseListStrings(val, strings);
}

/** Parse value as list of strings. */
bool JsonUtility::parseListStrings(const JsonValue& value, std::vector<ST::string> &strings)
{
	for (auto& i : value.toVec()) {
		strings.emplace_back(i.toString());
	}
	return true;
}

uint8_t JsonUtility::parseSectorID(const JsonValue& value)
{
	ST::string sectorString = value.toString();
	if (!SGPSector().IsValid(sectorString))
	{
		ST::string err = ST::format("{} is not a valid sector", sectorString);
		throw std::runtime_error(err.to_std_string());
	}
	return SGPSector::FromShortString(sectorString).AsByte();
}

std::vector<uint8_t> JsonUtility::parseSectorList(const JsonValue& json, const char* fieldName)
{
	auto r = json.toObject();
	std::vector<uint8_t> sectorIds;
	for (const auto& sector : r[fieldName].toVec())
	{
		sectorIds.emplace_back(JsonUtility::parseSectorID(sector.toString()));
	}
	return sectorIds;
}

std::array<uint8_t, NUM_DIF_LEVELS> JsonUtility::readIntArrayByDiff(const JsonValue& json, const char* fieldName)
{
	auto obj = json.toObject();
	std::array<uint8_t, NUM_DIF_LEVELS> vals = {};
	if (!obj.has(fieldName))
	{
		return vals;
	}
	auto arr = obj[fieldName].toVec();
	if (arr.size() != NUM_DIF_LEVELS)
	{
		ST::string err = ST::format("The number of values in {} is not same as NUM_DIF_LEVELS({})", fieldName, NUM_DIF_LEVELS);
		throw std::runtime_error(err.to_std_string());
	}
	for (unsigned int i = 0; i < NUM_DIF_LEVELS; i++)
	{
		vals[i] = static_cast<uint8_t>(arr[i].toUInt());
	}

	return vals;
}
