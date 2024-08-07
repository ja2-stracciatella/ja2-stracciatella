#pragma once

#include "Types.h"
#include <map>
#include <string_theory/string>
#include <utility>
#include <variant>

class DataReader;
class DataWriter;
class SavedGameStates;

/**
 * Variant on the primitive types compatible with JSON
 */
typedef std::variant<bool, int32_t, float, ST::string> PRIMITIVE_VALUE;

/**
 * Variant on all the data types we support storing in save games
 */
typedef std::variant<bool, int32_t, float, ST::string, std::vector<PRIMITIVE_VALUE>, std::map<ST::string, PRIMITIVE_VALUE>> STORABLE_TYPE;

typedef std::map<ST::string, STORABLE_TYPE> StateTable;

/**
 * Provides a method to save and load key-value pairs to a save game.
 * State keys and type should still be pre-determined in code or
 * scripts for it to be usable.
 *
 * This is a Stracciatella-only feature and appends additional data to
 * the end of the save file. The extra data are ignored if the game
 * is loaded in the original base game.
 */
extern SavedGameStates g_gameStates;

uint32_t SaveStatesSize();
void SaveStatesToSaveGameFile(HWFILE);
void LoadStatesFromSaveFile(HWFILE, SavedGameStates &states);
void ResetGameStates();

class SavedGameStates
{
public:
	bool HasKey(const ST::string& key) const;

	/**
	 * Gets state value of the specified type, by key
	 * @tparam T supported types are: int32_t, float, bool and ST::string
	 * @param key
	 * @return
	 * @throws std::out_of_range if the key was never set; throws std::bad_variant_access if the data type is different from what was set.
	 */
	template<typename T> T Get(const ST::string& key) const
	{
		return std::get<T>(states.at(key));
	}

	void Set(const ST::string& key, STORABLE_TYPE value);
	void Set(const ST::string& key, const char* value);

	/**
	 * @tparam T one of the primitive values compatible with JSON
	 * @param key
	 * @return a copy of vector
	 */
	template<typename T>
	std::vector<T> GetVector(const ST::string& key)
	{
		auto stored = std::get<std::vector<PRIMITIVE_VALUE>>(states.at(key));
		std::vector<T> vec;
		for (auto& v : stored)
		{
			vec.push_back(std::get<T>(v));
		}
		return vec;
	}

	template<typename T>
	void SetVector(const ST::string& key, std::vector<T> && vec)
	{
		std::vector<PRIMITIVE_VALUE> stored;
		for (auto const& v : vec)
		{
			PRIMITIVE_VALUE var{std::in_place_type<T>, v};
			stored.push_back(var);
		}
		Set(key, STORABLE_TYPE{stored});
	}

	/**
	 * @tparam K
	 * @tparam V
	 * @param key
	 * @return a copy of the map
	 */
	template<typename V>
	std::map<ST::string, V> GetMap(const ST::string& key)
	{
		auto stored = std::get<std::map<ST::string, PRIMITIVE_VALUE>>(states.at(key));
		std::map<ST::string, V> map;
		for (auto& pair : stored)
		{
			auto k = pair.first;
			map[k] = std::get<V>(pair.second);
		}
		return map;
	}

	template<typename T>
	void SetMap(const ST::string& key, std::map<ST::string, T> vec)
	{
		std::map<ST::string, PRIMITIVE_VALUE> stored;
		stored.insert(vec.begin(), vec.end());
		Set(key, STORABLE_TYPE{stored});
	}

	void Deserialize(const ST::string&);
	ST::string Serialize();

	// Clears all states
	void Clear();

	// Returns the entire table of states. Usually you want to use Get<T> instead of this.
	StateTable GetAll();
private:
	StateTable states;
};

std::vector<std::pair<ST::string, ST::string>> GetModInfoFromGameStates(const SavedGameStates &states);
void AddModInfoToGameStates(SavedGameStates &states);
