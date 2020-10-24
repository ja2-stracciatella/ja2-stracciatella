#include "SaveLoadGameStates.h"
#include "Buffer.h"
#include "FileMan.h"
#include <cmath>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <string_theory/string_stream>
#include <utility>


SavedGameStates g_gameStates;

void SaveStatesToSaveGameFile(HWFILE const hFile)
{
	std::stringstream ss;
	g_gameStates.Serialize(ss);

	std::string data = ss.str();
	UINT32      len  = data.length();
	FileWrite(hFile, &len, sizeof(UINT32));
	FileWrite(hFile, data.c_str(), len);
}

void LoadStatesFromSaveFile(HWFILE const hFile)
{
	UINT32 len;
	FileRead(hFile, &len, sizeof(UINT32));

	ST::char_buffer buff{len, '\0'};
	FileRead(hFile, buff.data(), len);

	std::stringstream ss(buff.c_str());
	g_gameStates.Deserialize(ss);
}

void ClearGameStates()
{
	g_gameStates.Clear();
}

bool SavedGameStates::HasKey(const ST::string& key)
{
	return states.find(key) != states.end();
}

void SavedGameStates::Set(const ST::string& key, VARIANT value)
{
	states[key] = std::move(value);
}

void SavedGameStates::Set(const ST::string& key, const char* s)
{
	Set(key, ST::string(s));
}

void SavedGameStates::Deserialize(std::stringstream& ss)
{
	rapidjson::IStreamWrapper is(ss);
	rapidjson::Document doc;
	doc.ParseStream(is);
	if (doc.HasParseError())
	{
		ST::string err = ST::format("Failed to parse JSON from saved game states : {}",
			rapidjson::GetParseError_En(doc.GetParseError()));
		throw std::runtime_error(err.to_std_string());
	}

	states.clear();
	for (const auto& entry : doc.GetObject())
	{
		ST::string key = entry.name.GetString();
		auto& v = entry.value;
		if      (v.IsString())  Set(key, ST::string(v.GetString()));
		else if (v.IsBool())    Set(key, v.GetBool());
		else if (v.IsInt())     Set(key, v.GetInt());
		else if (v.IsFloat())   Set(key, v.GetFloat());
		else                    throw std::runtime_error("unsupported type");
	}
}

void SavedGameStates::Serialize(std::stringstream& ss)
{
	rapidjson::OStreamWrapper os(ss);
	rapidjson::Writer<rapidjson::OStreamWrapper> writer(os);
	writer.StartObject();
	for (const auto& entry : states)
	{
		writer.String(entry.first.c_str());
		auto v = entry.second;
		if      (auto *i = std::get_if<int32_t>(&v))    writer.Int(*i);
		else if (auto *s = std::get_if<ST::string>(&v)) writer.String(s->c_str());
		else if (auto *f = std::get_if<float>(&v))      writer.Double(*f);
		else if (auto *b = std::get_if<bool>(&v))       writer.Bool(*b);
		else    throw std::runtime_error("unsupported type");
	}
	writer.EndObject();
}

void SavedGameStates::Clear()
{
	states.clear();
}

StateTable SavedGameStates::GetAll()
{
	return states;
}
