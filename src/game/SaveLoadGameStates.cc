#include "SaveLoadGameStates.h"
#include "Buffer.h"
#include "ContentManager.h"
#include "FileMan.h"
#include "GameInstance.h"
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
	hFile->write(&len, sizeof(UINT32));
	hFile->write(data.c_str(), len);
}

void LoadStatesFromSaveFile(HWFILE const hFile)
{
	UINT32 len;
	hFile->read(&len, sizeof(UINT32));

	ST::char_buffer buff{len, '\0'};
	hFile->read(buff.data(), len);

	std::stringstream ss(buff.c_str());
	g_gameStates.Deserialize(ss);
}

void AddModInfoToGameStates() {
	auto mods = GCM->getEnabledMods();
	std::vector<ST::string> modState;
	for (auto i = mods.begin(); i < mods.end(); i++) {
		modState.push_back(ST::format("{}!{}", (*i).first, (*i).second));
	}
	g_gameStates.SetVector("stracciatella:mods", modState);
}

void ResetGameStates()
{
	g_gameStates.Clear();
	AddModInfoToGameStates();
}

bool SavedGameStates::HasKey(const ST::string& key)
{
	return states.find(key) != states.end();
}

void SavedGameStates::Set(const ST::string& key, STORABLE_TYPE value)
{
	states[key] = std::move(value);
}

void SavedGameStates::Set(const ST::string& key, const char* s)
{
	Set(key, ST::string(s));
}

#define RETURN_PRIMITIVE(v) \
	if (v.IsString()) return ST::string(v.GetString()); \
	else if (v.IsBool()) return v.GetBool(); \
	else if (v.IsInt()) return v.GetInt(); \
	else if (v.IsFloat()) return v.GetFloat();

/**
 * Reads a JSON value into a std::variant without knowing the type beforehand.
 * @param v
 * @return
 */
static STORABLE_TYPE ReadFromJSON(const rapidjson::Value& v)
{
	auto readVal = [](const rapidjson::Value& v) -> PRIMITIVE_VALUE {
		RETURN_PRIMITIVE(v) throw std::runtime_error("not a supported type");
	};

	RETURN_PRIMITIVE(v)

	if (v.IsArray())
	{
		std::vector<PRIMITIVE_VALUE> vec;
		for (auto& el : v.GetArray())
		{
			vec.push_back(readVal(el));
		}
		return vec;
	}
	if (v.IsObject())
	{
		std::map<ST::string, PRIMITIVE_VALUE> map;
		for (auto& el : v.GetObject())
		{
			auto key = ST::string(el.name.GetString());
			map[key] = readVal(el.value);
		}
		return map;
	}

	throw std::logic_error("not de-serializable");
}

#undef RETURN_PRIMITIVE

#define WRITE_PRIMITIVE(v, w) \
	if      (auto *b = std::get_if<bool>(&v))       w.Bool(*b); \
	else if (auto *i = std::get_if<int32_t>(&v))    w.Int(*i); \
	else if (auto *s = std::get_if<ST::string>(&v)) w.String(s->c_str()); \
	else if (auto *f = std::get_if<float>(&v))      w.Double(*f);

static void WriteToJSON(const STORABLE_TYPE& v, rapidjson::Writer<rapidjson::OStreamWrapper>& w)
{
	auto writeJSON = [&w](const PRIMITIVE_VALUE& v) {
		WRITE_PRIMITIVE(v, w) else throw std::runtime_error("unsupported type");
	};

	WRITE_PRIMITIVE(v, w)
	else if (auto *vec = std::get_if<std::vector<PRIMITIVE_VALUE>>(&v)) {
		w.StartArray();
		for (auto& el : *vec) {
			writeJSON(el);
		}
		w.EndArray();
	}
	else if (auto *map = std::get_if<std::map<ST::string, PRIMITIVE_VALUE>>(&v)) {
		w.StartObject();
		for (auto& pair : *map) {
			writeJSON(pair.first);
			writeJSON(pair.second);
		}
		w.EndObject();
	}
	else throw std::logic_error("cannot serialize"); // this should never happen
}

#undef WRITE_PRIMITIVE

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
		STORABLE_TYPE v = ReadFromJSON(entry.value);
		states[key] = v;
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
		WriteToJSON(entry.second, writer);
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
