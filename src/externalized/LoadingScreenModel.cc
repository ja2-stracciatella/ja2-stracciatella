#include "LoadingScreenModel.h"

#include "Campaign_Types.h"
#include "Directories.h"
#include "Loading_Screen.h"

#include <map>
#include <stdexcept>

const std::vector<LoadingScreen> PREDEFINED_SCREENS = {
	LoadingScreen(LOADINGSCREEN_NOTHING,     "NOTHING",     "/ls_heli.sti"),
	LoadingScreen(LOADINGSCREEN_DAYGENERIC,  "DAYGENERIC",  "/ls_daygeneric.sti"),
	LoadingScreen(LOADINGSCREEN_DAYTOWN1,    "DAYTOWN1",    "/ls_daytown1.sti"),
	LoadingScreen(LOADINGSCREEN_DAYTOWN2,    "DAYTOWN2",    "/ls_daytown2.sti"),
	LoadingScreen(LOADINGSCREEN_DAYWILD,     "DAYWILD",     "/ls_daywild.sti"),
	LoadingScreen(LOADINGSCREEN_DAYTROPICAL, "DAYTROPICAL", "/ls_daytropical.sti"),
	LoadingScreen(LOADINGSCREEN_DAYFOREST,   "DAYFOREST",   "/ls_dayforest.sti"),
	LoadingScreen(LOADINGSCREEN_DAYDESERT,   "DAYDESERT",   "/ls_daydesert.sti"),
	LoadingScreen(LOADINGSCREEN_DAYPALACE,   "DAYPALACE",   "/ls_daypalace.sti"),

	LoadingScreen(LOADINGSCREEN_NIGHTGENERIC,  "NIGHTGENERIC",  "/ls_nightgeneric.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTWILD,     "NIGHTWILD",     "/ls_nightwild.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTTOWN1,    "NIGHTTOWN1",    "/ls_nighttown1.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTTOWN2,    "NIGHTTOWN2",    "/ls_nighttown2.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTFOREST,   "NIGHTFOREST",   "/ls_nightforest.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTTROPICAL, "NIGHTTROPICAL", "/ls_nighttropical.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTDESERT,   "NIGHTDESERT",   "/ls_nightdesert.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTPALACE,   "NIGHTPALACE",   "/ls_nightpalace.sti"),

	LoadingScreen(LOADINGSCREEN_HELI,     "HELI",     "/ls_heli.sti"),
	LoadingScreen(LOADINGSCREEN_BASEMENT, "BASEMENT", "/ls_basement.sti"),
	LoadingScreen(LOADINGSCREEN_MINE,     "MINE",     "/ls_mine.sti"),
	LoadingScreen(LOADINGSCREEN_CAVE,     "CAVE",     "/ls_cave.sti"),

	LoadingScreen(LOADINGSCREEN_DAYPINE,       "DAYPINE",       "/ls_daypine.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTPINE,     "NIGHTPINE",     "/ls_nightpine.sti"),
	LoadingScreen(LOADINGSCREEN_DAYMILITARY,   "DAYMILITARY",   "/ls_daymilitary.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTMILITARY, "NIGHTMILITARY", "/ls_nightmilitary.sti"),
	LoadingScreen(LOADINGSCREEN_DAYSAM,        "DAYSAM",        "/ls_daysam.sti"),
	LoadingScreen(LOADINGSCREEN_NIGHTSAM,      "NIGHTSAM",      "/ls_nightsam.sti")
};

LoadingScreenModel::LoadingScreenModel(std::vector<LoadingScreen> screensList_, std::vector<LoadingScreenMapping> screensMapping_)
	: screensList(screensList_), screensMapping(screensMapping_) {}

const LoadingScreen* LoadingScreenModel::getScreenForSector(uint8_t sectorId, uint8_t sectorLevel, bool isNight) const
{
	for (auto const& screen : screensMapping)
	{
		if (screen.sectorId == sectorId && screen.sectorLevel == sectorLevel)
		{
			uint8_t screenId = isNight ? screen.loadScreenNight : screen.loadScreenDay;
			return getByIndex(screenId);
		}
	}
	return NULL;
}

const LoadingScreen* LoadingScreenModel::getByIndex(uint8_t index) const
{
	return &(screensList.at(index));
}

void LoadingScreenModel::validateData(ContentManager* cm) const
{
	for (size_t i = 0; i < screensList.size(); i++)
	{
		if (cm->doesGameResExists(screensList[i].filename))
		{
			SLOGW("Load Screen image '{}' cannot be opened", screensList[i].filename);
		}
		if (screensList[i].index != i)
		{
			ST::string err = ST::format("Load Screen with index {} does not match its position ({}) in list", screensList[i].index, i);
			throw std::logic_error(err.to_std_string());
		}
	}
}

LoadingScreenModel* LoadingScreenModel::deserialize(const JsonValue& screensList, const JsonValue& screensMapping)
{
	std::vector<LoadingScreen> screens = PREDEFINED_SCREENS;
	size_t index = screens.size();
	for (auto& item : screensList.toVec())
	{
		auto r = item.toObject();
		screens.emplace_back(static_cast<uint8_t>(index++), r.GetString("internalName"), r.GetString("filename"));
	}

	std::map<ST::string, uint8_t> namesMapping;
	for (index = 0; index < screens.size(); index++)
	{
		ST::string const& name = screens[index].internalName;
		namesMapping[name] = static_cast<uint8_t>(index);
	}


	std::vector<LoadingScreenMapping> mappings;
	for (auto& item : screensMapping.toVec())
	{
		auto r = item.toObject();
		uint8_t sectorId = SGPSector::FromShortString(r.GetString("sector")).AsByte();
		mappings.push_back(LoadingScreenMapping{
			sectorId,
			(UINT8)r.getOptionalInt("sectorLevel", 0),
			namesMapping.at(r.GetString("night")),
			namesMapping.at(r.GetString("day"))
			});
	}

	return new LoadingScreenModel(screens, mappings);
}


