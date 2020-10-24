#pragma once

#include "Types.h"
#include <map>
#include <string_theory/string>
#include <variant>

class DataReader;
class DataWriter;
class SavedGameStates;

typedef std::variant<int32_t, float, bool, ST::string> VARIANT;
typedef std::map<ST::string, VARIANT> StateTable;

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

void SaveStatesToSaveGameFile(HWFILE);
void LoadStatesFromSaveFile(HWFILE);
void ClearGameStates();

class SavedGameStates
{
public:
	/**
	 * Gets state value of the specified type, by key
	 * @tparam T supported types are: int32_t, float, bool and ST::string
	 * @param key
	 * @return
	 * @throws std::out_of_range if the key was never set; throws std::bad_variant_access if the data type is different from what was set.
	 */
	template<typename T> T Get(const ST::string& key)
	{
		return std::get<T>(states.at(key));
	}

	bool HasKey(const ST::string& key);

	void Set(const ST::string& key, VARIANT value);
	void Set(const ST::string& key, const char* value);

	void Deserialize(std::stringstream&);
	void Serialize(std::stringstream&);

	// Clears all states
	void Clear();

	// Returns the entire table of states. Usually you want to use Get<T> instead of this.
	StateTable GetAll();
private:
	StateTable states;
};
