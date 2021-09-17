#include "JsonUtility.h"
#include "Campaign_Types.h"
#include "Debug.h"
#include "SGPFile.h"
#include "FileMan.h"

#include "rapidjson/document.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include <array>
#include <sstream>

/** Write list of strings to file. */
bool JsonUtility::writeToFile(const ST::string &name, const std::vector<ST::string> &strings)
{
	std::stringstream ss;
	rapidjson::OStreamWrapper os(ss);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(os);
	writer.StartArray();
	for(auto it = strings.begin(); it != strings.end(); ++it)
	{
		writer.String(it->c_str());
	}
	writer.EndArray();
	ss << std::endl;

	ST::string buf = ss.str();
	try {
		AutoSGPFile file{FileMan::openForWriting(name)};

		file->write(buf.c_str(), buf.size());
	} catch (const std::runtime_error& ex) {
		SLOGE("JsonUtility::writeToFile: %s", ex.what());
		return false;
	}
	return true;
}

/** Parse json to a list of strings. */
bool JsonUtility::parseJsonToListStrings(const char* jsonData, std::vector<ST::string> &strings)
{
	rapidjson::Document document;

	if (document.Parse<rapidjson::kParseCommentsFlag>(jsonData).HasParseError())
	{
		return false;
	}

	return parseListStrings(document, strings);
}

/** Parse value as list of strings. */
bool JsonUtility::parseListStrings(const rapidjson::Value &value, std::vector<ST::string> &strings)
{
	if(value.IsArray()) {
		for (rapidjson::SizeType i = 0; i < value.Size(); i++)
		{
			strings.push_back(value[i].GetString());
		}
		return true;
	}
	return false;
}

uint8_t JsonUtility::parseSectorID(const ST::string& sectorString)
{
	if (!IS_VALID_SECTOR_SHORT_STRING(sectorString))
	{
		ST::string err = ST::format("{} is not a valid sector", sectorString);
		throw std::runtime_error(err.to_std_string());
	}
	return SECTOR_FROM_SECTOR_SHORT_STRING(sectorString.c_str());
}

uint8_t JsonUtility::parseSectorID(const rapidjson::Value& json, const char* fieldName)
{
	if (!json.HasMember(fieldName) || !json[fieldName].IsString())
	{
		ST::string err = ST::format("expecting string value in field '{}'", fieldName);
		throw std::runtime_error(err.to_std_string());
	}
	return JsonUtility::parseSectorID(json.GetString());
}

std::vector<uint8_t> JsonUtility::parseSectorList(const rapidjson::Value& json, const char* fieldName)
{
	if (!json.HasMember(fieldName) || !json[fieldName].IsArray())
	{
		ST::string err = ST::format("field '{}' is not an array", fieldName);
		throw std::runtime_error(err.to_std_string());
	}
	std::vector<uint8_t> sectorIds;
	for (const auto& sector : json[fieldName].GetArray())
	{
		if (!sector.IsString())
		{
			throw std::runtime_error("sector list must contain only strings");
		}
		sectorIds.push_back(JsonUtility::parseSectorID(sector.GetString()));
	}
	return sectorIds;
}

std::array<uint8_t, NUM_DIF_LEVELS> JsonUtility::readIntArrayByDiff(const rapidjson::Value& obj, const char* fieldName)
{
	std::array<uint8_t, NUM_DIF_LEVELS> vals = {};
	if (!obj.HasMember(fieldName))
	{
		return vals;
	}
	const auto& arr = obj[fieldName].GetArray();
	if (arr.Size() != NUM_DIF_LEVELS)
	{
		ST::string err = ST::format("The number of values in {} is not same as NUM_DIF_LEVELS({})", fieldName, NUM_DIF_LEVELS);
		throw std::runtime_error(err.to_std_string());
	}
	for (unsigned int i = 0; i < NUM_DIF_LEVELS; i++)
	{
		vals[i] = static_cast<uint8_t>(arr[i].GetUint());
	}

	return vals;
}
