#include "Debug.h"
#include "JsonUtility.h"
#include "RustInterface.h"

#include "rapidjson/document.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"

#include <sstream>

/** Write list of strings to file. */
bool JsonUtility::writeToFile(const char *name, const std::vector<ST::string> &strings)
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
	if (!Fs_write(name, reinterpret_cast<const uint8_t*>(buf.c_str()), buf.size()))
	{
		RustPointer<char> err(getRustError());
		SLOGE("JsonUtility::writeToFile: %s", err.get());
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

