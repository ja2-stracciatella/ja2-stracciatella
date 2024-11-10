#pragma once

#include "ContentManager.h"
#include "Json.h"

#include <utility>
#include <vector>


// Definition of a Loading Screen
struct LoadingScreen
{
	LoadingScreen(uint8_t index_, ST::string&& internalName_, ST::string&& filename_)
		: index(index_), internalName(std::move(internalName_)), filename(std::move(filename_)) {}

	uint8_t index;
	ST::string internalName;
	ST::string filename;
};

// An entry in the sector to loading-screen mapping
struct LoadingScreenMapping
{
	uint8_t sectorId;
	uint8_t sectorLevel;
	uint8_t loadScreenNight;
	uint8_t loadScreenDay;
};

// Model class providing access to the available loading screens
class LoadingScreenModel
{
public:
	LoadingScreenModel(std::vector<LoadingScreen>&& screensList, std::vector<LoadingScreenMapping>&& screensMapping);

	// returns NULL if the given sector is not mapped
	const LoadingScreen* getScreenForSector(uint8_t sectorId, uint8_t sectorLevel, bool isNight) const;

	// throws out_of_range if index is out of bounds
	const LoadingScreen* getByIndex(uint8_t index) const;

	void validateData(ContentManager* cm) const;

	static LoadingScreenModel* deserialize(const JsonValue& screensList, const JsonValue& screensMapping);

protected:
	// list of available loading screens
	std::vector<LoadingScreen> screensList;

	// mapping of sector to loading screens
	std::vector<LoadingScreenMapping> screensMapping;
};

