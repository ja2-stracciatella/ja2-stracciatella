#include "ScriptingExtensions.h"

#define SOL_CHECK_ARGUMENTS 1
#define SOL_PRINT_ERRORS 1
#include <sol/sol.hpp>  // this needs to be included first

#include "Campaign_Types.h"
#include "ContentManager.h"
#include "FileMan.h"
#include "FunctionsLibrary.h"
#include "GameInstance.h"
#include "Logger.h"
#include "Overhead.h"
#include "Quests.h"
#include "StrategicMap.h"
#include <set>
#include <string>
#include <string_theory/format>
#include <string_theory/string>

#define SCRIPTS_DIR "scripts"
#define ENTRYPOINT_SCRIPT "main.lua"

static std::set<std::string> loadedScripts;
static bool isLuaInitialized = false;
static bool hasLuaPanicked = false;
static sol::state lua;

static void RegisterUserTypes();
static void RegisterGlobals();
static void RegisterLogger();

void JA2Require(std::string scriptFileName)
{
	if (isLuaInitialized)
	{
		throw std::runtime_error("JA2Require is not allowed after initialization");
	}

	if (loadedScripts.find(scriptFileName) != loadedScripts.end())
	{
		SLOGW(ST::format("Script file '{}' has already been loaded", scriptFileName));
		return;
	}
	
	SLOGD(ST::format("Loading LUA script file: {}", scriptFileName));
	std::string scriptbody = FileMan::fileReadText(
		AutoSGPFile(GCM->openGameResForReading(SCRIPTS_DIR "/" + scriptFileName))
	).to_std_string();
	lua.script(scriptbody);
}

void InitScriptingEngine()
{
	loadedScripts.clear();
	isLuaInitialized = false;
	hasLuaPanicked = false;

	if (!GCM->doesGameResExists(SCRIPTS_DIR "/" ENTRYPOINT_SCRIPT))
	{
		return;
	}

	try
	{
		SLOGD("Initializing Lua/Sol2 scripting engine");

		lua = sol::state();
		lua.open_libraries(
			sol::lib::base,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table
		);

		RegisterUserTypes();
		RegisterGlobals();
		RegisterLogger();

		JA2Require(ENTRYPOINT_SCRIPT);

		isLuaInitialized = true;
	} 
	catch (std::exception ex)
	{
		SLOGE(ST::format("Lua script engine has failed to initialize:\n {}", ex.what()));
		ST::string err = "The game cannot be started due to an error in the mod scripts. Check the logs for more details.";
		std::throw_with_nested(std::runtime_error(err.to_std_string()));
	}
}

static void RegisterUserTypes()
{
	lua.new_usertype<SECTORINFO>("SECTORINFO",
		"ubNumAdmins", &SECTORINFO::ubNumAdmins,
		"ubNumTroops", &SECTORINFO::ubNumTroops,
		"ubNumElites", &SECTORINFO::ubNumElites,
		"uiFlags", &SECTORINFO::uiFlags
		);

	lua.new_usertype<UNDERGROUND_SECTORINFO>("UNDERGROUND_SECTORINFO",
		"ubNumAdmins", &UNDERGROUND_SECTORINFO::ubNumAdmins,
		"ubNumTroops", &UNDERGROUND_SECTORINFO::ubNumTroops,
		"ubNumElites", &UNDERGROUND_SECTORINFO::ubNumElites,
		"uiFlags", &UNDERGROUND_SECTORINFO::uiFlags
		);

	lua.new_usertype<OBJECTTYPE>("OBJECTTYPE",
		"usItem", &OBJECTTYPE::usItem,
		"bTrap", &OBJECTTYPE::bTrap
		);

	lua.new_simple_usertype<StrategicMapElement>("StrategicMapElement",
		"bNameId", &StrategicMapElement::bNameId,
		"fEnemyControlled", &StrategicMapElement::fEnemyControlled,
		"fEnemyAirControlled", &StrategicMapElement::fEnemyAirControlled
		);

	lua.new_usertype<TacticalStatusType>("TacticalStatusType",
		"fEnemyInSector", &TacticalStatusType::fEnemyInSector,
		"fDidGameJustStart", &TacticalStatusType::fDidGameJustStart
		);
}

static void RegisterGlobals()
{
	lua["gTacticalStatus"] = &gTacticalStatus;
	lua["gubQuest"] = &gubQuest;
	lua["gubFact"] = &gubFact;

	lua.set_function("JA2Require", JA2Require);

	lua.set_function("GetCurrentSector", GetCurrentSector);
	lua.set_function("GetSectorInfo", GetSectorInfo);
	lua.set_function("GetUndergroundSectorInfo", GetUndergroundSectorInfo);
	
	lua.set_function("CreateItem", CreateItem);
	lua.set_function("CreateMoney", CreateMoney);
	lua.set_function("PlaceItem", PlaceItem);
}

static void RegisterLogger()
{
	sol::table log = lua["log"].get_or_create<sol::table>();
	log["debug"] = [](std::string msg) { SLOGD(msg); };
	log["info"]  = [](std::string msg) { SLOGI(msg); };
	log["warn"]  = [](std::string msg) { SLOGW(msg); };
	log["error"] = [](std::string msg) { SLOGE(msg); };

	// overrides the default print()
	lua.set_function("print", [](std::string msg) { SLOGI(msg); });
}

static void InvokeFunction(ST::string functionName)
{
	if (!isLuaInitialized)
	{
		SLOGD("Lua scripting is not available");
		return;
	}
	if (hasLuaPanicked)
	{
		SLOGE("Scripting engine has been disabled due to a previous error"); 
		return;
	}

	sol::protected_function func = lua[functionName.to_std_string()];
	if (!func.valid())
	{
		SLOGD(ST::format("Function {} is not defined", functionName));
		return;
	}
	
	auto result = func.call();
	if (!result.valid())
	{
		sol::error err = result;
		SLOGE(ST::format("Lua script had an error. Scripting engine is now DISABLED. The error was:\n{}", err.what()));
		hasLuaPanicked = true;
	}
}

void BeforePrepareSector()
{
	InvokeFunction("BeforePrepareSector");
}
