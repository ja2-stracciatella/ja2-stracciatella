#include "SaveLoadGameStates.h"
#include "ContentManager.h"
#include "GameInstance.h"
#include "Json.h"
#include "SGPFile.h"

#include <cmath>
#include <string_theory/string_stream>
#include <utility>


SavedGameStates g_gameStates;

uint32_t SaveStatesToSaveGameFile(SGPFile & hFile)
{
	std::string data = g_gameStates.Serialize().to_std_string();
	uint32_t const len{ static_cast<uint32_t>(data.length()) };
	hFile.write(&len, sizeof(len));
	hFile.write(data.data(), len);
	return len;
}

void LoadStatesFromSaveFile(HWFILE const hFile, SavedGameStates &states)
{
	UINT32 len;
	hFile->read(&len, sizeof(UINT32));

	ST::char_buffer buff{len, '\0'};
	hFile->read(buff.data(), len);

	states.Deserialize(buff);
}

const ST::string MODS_KEY = "stracciatella:mods";

void ResetGameStates()
{
	g_gameStates.Clear();
	AddModInfoToGameStates(g_gameStates);
}

bool SavedGameStates::HasKey(const ST::string& key) const
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
	if (v.isString()) return ST::string(v.toString()); \
	else if (v.isBool()) return v.toBool(); \
	else if (v.isInt()) return v.toInt(); \
	else if (v.isDouble()) return static_cast<float>(v.toDouble());

/**
 * Reads a JSON value into a std::variant without knowing the type beforehand.
 * @param v
 * @return
 */
static STORABLE_TYPE DeserializeStorableType(const JsonValue& v)
{
	auto readVal = [](const JsonValue& v) -> PRIMITIVE_VALUE {
		RETURN_PRIMITIVE(v) throw std::runtime_error("not a supported type");
	};

	RETURN_PRIMITIVE(v)

	if (v.isVec()) {
		std::vector<PRIMITIVE_VALUE> vec;
		for (auto& el : v.toVec())
		{
			vec.push_back(readVal(el));
		}
		return vec;
	}
	if (v.isObject()) {
		auto obj = v.toObject();
		std::map<ST::string, PRIMITIVE_VALUE> map;
		for (auto& key : obj.keys())
		{
			map[key] = readVal(obj[key.c_str()]);
		}
		return map;
	}

	throw std::logic_error("not de-serializable");
}

#undef RETURN_PRIMITIVE

#define RETURN_PRIMITIVE(v) \
	if      (auto *b = std::get_if<bool>(&v))       return JsonValue(*b); \
	else if (auto *i = std::get_if<int32_t>(&v))    return JsonValue(*i); \
	else if (auto *s = std::get_if<ST::string>(&v)) return JsonValue(*s); \
	else if (auto *f = std::get_if<float>(&v))      return JsonValue(static_cast<double>(*f));

static JsonValue SerializeStorableType(const STORABLE_TYPE& v)
{
	auto serializePrimitive = [](const PRIMITIVE_VALUE& v) {
		RETURN_PRIMITIVE(v) else throw std::runtime_error("unsupported type");
	};

	RETURN_PRIMITIVE(v)
	else if (auto *vec = std::get_if<std::vector<PRIMITIVE_VALUE>>(&v)) {
		JsonArray w;
		for (auto& el : *vec) {
			w.push(serializePrimitive(el));
		}
		return w.toValue();
	}
	else if (auto *map = std::get_if<std::map<ST::string, PRIMITIVE_VALUE>>(&v)) {
		JsonObject obj;
		for (auto& pair : *map) {
			obj.set(pair.first.c_str(), serializePrimitive(pair.second));
		}
		return obj.toValue();
	}
	else throw std::logic_error("cannot serialize"); // this should never happen
}

#undef WRITE_PRIMITIVE

void SavedGameStates::Deserialize(const ST::string& s)
{
	auto json = JsonValue::deserialize(s);
	this->Clear();
	auto obj = json.toObject();
	for (const auto& key : obj.keys())
	{
		auto val = obj[key.c_str()];
		states[key] = DeserializeStorableType(val);
	}
}

ST::string SavedGameStates::Serialize() const
{
	JsonObject obj;
	for (const auto& entry : states)
	{
		obj.set(entry.first.c_str(), SerializeStorableType(entry.second));
	}
	return obj.toValue().serialize();
}

void SavedGameStates::Clear()
{
	states.clear();
}

StateTable const& SavedGameStates::GetAll() const noexcept
{
	return states;
}

void AddModInfoToGameStates(SavedGameStates &states) {
	auto mods = GCM->getEnabledMods();

	JsonArray arr;
	for (auto& i : mods) {
		JsonObject obj;
		obj.set("name", i.first);
		obj.set("version", i.second);

		arr.push(obj.toValue());
	}

	states.Set(MODS_KEY, arr.toValue().serialize());
}

std::vector<std::pair<ST::string, ST::string>> GetModInfoFromGameStates(const SavedGameStates &states) {
	std::vector<std::pair<ST::string, ST::string>> mods;
	if (!states.HasKey(MODS_KEY)) {
		auto errorMsg = ST::format("Failed to parse JSON from saved game states for mods: missing `{}` key", MODS_KEY);
		throw std::runtime_error(errorMsg.c_str());
	}
	auto && str{ states.Get<ST::string>(MODS_KEY) };
	auto json = JsonValue::deserialize(str);

	for (auto& modJson : json.toVec()) {
		auto obj = modJson.toObject();
		mods.emplace_back(
			obj.GetString("name"),
			obj.GetString("version")
		);
	}

	return mods;
}
