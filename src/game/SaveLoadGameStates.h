#pragma once

#include "SGPFile.h"
#include "Types.h"
#include <iterator>
#include <map>
#include <utility>
#include <variant>
#include <string_theory/string>

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

uint32_t SaveStatesToSaveGameFile(SGPFile &);
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
	template<typename T> decltype(auto) Get(const ST::string& key) const
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
	auto GetVector(const ST::string& key) const
	{
		auto && stored{ std::get<std::vector<PRIMITIVE_VALUE>>(states.at(key)) };
		std::vector<T> vec;
		for (auto& v : stored)
		{
			vec.push_back(std::get<T>(v));
		}
		return vec;
	}

	// This template must be disabled for vector<bool> otherwise the compiler
	// will choose it as being more specific for some value types of vec, such
	// as rvalues.
	template<typename T, std::enable_if_t<!std::is_same_v<bool, T>, bool> = true>
	void SetVector(const ST::string& key, std::vector<T> && vec)
	{
		Set(key, STORABLE_TYPE{
			std::vector<PRIMITIVE_VALUE> {
				std::move_iterator{ vec.begin() },
				std::move_iterator{ vec.end() }
			}
		});
	}

	// Overload for the problematic vector<bool> to avoid a problem
	// with Apple Clang or the STL implementation it uses.
	void SetVector(ST::string const& key, std::vector<bool> const& bvec)
	{
		std::vector<PRIMITIVE_VALUE> temp;
		for (auto && val : bvec)
		{
			temp.push_back(bool{val});
		}
		Set(key, STORABLE_TYPE{ std::move(temp) });
	}

	/**
	 * @tparam K
	 * @tparam V
	 * @param key
	 * @return a copy of the map
	 */
	template<typename V>
	auto GetMap(const ST::string& key) const
	{
		auto && stored{ std::get<std::map<ST::string, PRIMITIVE_VALUE>>(states.at(key)) };
		std::map<ST::string, V> map;
		for (auto& pair : stored)
		{
			map[pair.first] = std::get<V>(pair.second);
		}
		return map;
	}

	template<typename T>
	void SetMap(const ST::string& key, std::map<ST::string, T> && map)
	{
		Set(key, STORABLE_TYPE{
			std::map<ST::string, PRIMITIVE_VALUE> {
				std::move_iterator{ map.begin() },
				std::move_iterator{ map.end() }
			}
		});
	}

	void Deserialize(const ST::string&);
	ST::string Serialize() const;

	// Clears all states
	void Clear();

	// Returns the entire table of states. Usually you want to use Get<T> instead of this.
	StateTable const& GetAll() const noexcept;
private:
	StateTable states;
};

std::vector<std::pair<ST::string, ST::string>> GetModInfoFromGameStates(const SavedGameStates &states);
void AddModInfoToGameStates(SavedGameStates &states);
