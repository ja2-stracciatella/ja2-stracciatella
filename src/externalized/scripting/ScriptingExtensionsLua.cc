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
#include <stdexcept>
#include <string>
#include <string_theory/format>
#include <string_theory/string>
#include <game/TileEngine/Explosion_Control.h>

#define SCRIPTS_DIR "scripts"
#define ENTRYPOINT_SCRIPT "main.lua"

static std::set<std::string> loadedScripts;
static bool isLuaInitialized = false;
static bool isLuaDisabled = false;
static sol::state lua;

static void RegisterUserTypes();
static void RegisterGlobals();
static void RegisterLogger();
static void RegisterListener(const std::string& observable, const std::string& key, const std::string& luaFunctionName);
static void UnregisterListener(const std::string& observable, const std::string& key);



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
	lua.script(scriptbody, ST::format("@{}", scriptFileName).to_std_string());
}

void InitScriptingEngine()
{
	loadedScripts.clear();
	isLuaInitialized = false;
	isLuaDisabled = false;

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
	catch (const std::exception &ex)
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

	lua.set_function("GetCurrentSector", GetCurrentSector);
	lua.set_function("GetSectorInfo", GetSectorInfo);
	lua.set_function("GetUndergroundSectorInfo", GetUndergroundSectorInfo);
	
	lua.set_function("CreateItem", CreateItem);
	lua.set_function("CreateMoney", CreateMoney);
	lua.set_function("PlaceItem", PlaceItem);

	lua.set_function("JA2Require", JA2Require);
	lua.set_function("require",  []() { throw std::logic_error("require is not allowed. Use JA2Require instead"); });
	lua.set_function("dofile",   []() { throw std::logic_error("dofile is not allowed. Use JA2Require instead"); });
	lua.set_function("loadfile", []() { throw std::logic_error("loadfile is not allowed. Use JA2Require instead"); });

	lua.set_function("RegisterListener", RegisterListener);
	lua.set_function("UnregisterListener", UnregisterListener);
}

static void LogLuaMessage(LogLevel level, std::string msg) {
		lua_Debug info;
		// Stack position 0 is the c function we are in
		// Stack position 1 is the calling lua script
		lua_getstack(lua, 1, &info);
		lua_getinfo(lua, "S", &info);
		LogMessage(false, level, info.short_src, msg);
}

static void RegisterLogger()
{
	sol::table log = lua["log"].get_or_create<sol::table>();
	log["debug"] = [](std::string msg) { LogLuaMessage(LogLevel::Debug, msg); };
	log["info"]  = [](std::string msg) { LogLuaMessage(LogLevel::Info, msg); };
	log["warn"]  = [](std::string msg) { LogLuaMessage(LogLevel::Warn, msg); };
	log["error"] = [](std::string msg) { LogLuaMessage(LogLevel::Error, msg); };

	// overrides the default print()
	lua.set_function("print", [](std::string msg) { LogLuaMessage(LogLevel::Info, msg); });
}

static void InvokeFunction(ST::string functionName)
{
	if (!isLuaInitialized)
	{
		SLOGD("Lua scripting is not available");
		return;
	}
	if (isLuaDisabled)
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
		isLuaDisabled = true;
	}
}

// Creates a typed std::function out of a Lua function
template<typename ...A>
static std::function<void(A...)> wrap(std::string luaFunc)
{
	if (luaFunc.empty()) return {};

	sol::protected_function func = lua[luaFunc];
	if (!func.valid())
	{
		ST::string err = ST::format("There is no function '{}' in Lua", luaFunc);
		throw std::logic_error(err.to_std_string());
	}
	return [func](A... args) {
		func(args...);
	};
}

/**
 * Registers a callback listener with an Observable, to receive notifications in Lua scripts.
 * @param observable string the name of an instance of Observable
 * @param key string a unique key identifying the callback listener
 * @param luaFunc string name to the callback handling lua function
 */
static void RegisterListener(const std::string& observable, const std::string& key, const std::string& luaFunc)
{
	if (isLuaInitialized)
	{
		throw std::runtime_error("RegisterListener is not allowed after initialization");
	}

	if      (observable == "OnStructureDamage")			OnStructureDamaged.addListener(key, wrap<INT16, INT16, INT16, INT16, UINT8>(luaFunc));
	else if (observable == "OnAirspaceControlUpdated")		OnAirspaceControlUpdated.addListener(key, wrap<>(luaFunc));
	else if (observable == "BeforePrepareSector")			BeforePrepareSector.addListener(key, wrap<>(luaFunc));
	else {
		ST::string err = ST::format("There is no observable named '{}'", observable);
		throw std::logic_error(err.to_std_string());
	}
}

/**
 * Unregisters a listener from the Observable
 * @param observable
 * @param key
 */
void UnregisterListener(const std::string& observable, const std::string& key)
{
	RegisterListener(observable, key, "");
}
