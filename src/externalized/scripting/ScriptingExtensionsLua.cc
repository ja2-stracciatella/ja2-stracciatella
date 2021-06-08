#include "ScriptingExtensions.h"

#define SOL_CHECK_ARGUMENTS 1
#define SOL_PRINT_ERRORS 1
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>  // this needs to be included first
#include "STStringHandler.h"

#include "Campaign_Types.h"
#include "ContentManager.h"
#include "FileMan.h"
#include "FunctionsLibrary.h"
#include "Game_Events.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "Logger.h"
#include "Overhead.h"
#include "Quests.h"
#include "StrategicMap.h"
#include "Structure.h"
#include <set>
#include <stdexcept>
#include <string>
#include <string_theory/format>
#include <string_theory/string>

#define SCRIPTS_DIR "scripts"
#define ENTRYPOINT_SCRIPT "main.lua"

/*! \struct GAME_OPTIONS
    \brief Options which the current game was started with */
struct GAME_OPTIONS;
/*! \struct TacticalStatusType
    \brief Status information of the game
    \details Accessible via the gTacticalStatusType global variable
    */
struct TacticalStatusType;

static std::set<std::string> loadedScripts;
static bool isLuaInitialized = false;
static bool isLuaDisabled = false;
static sol::state lua;

// an increment counter used to generate unique keys for listeners
static unsigned int counter;

static void RegisterUserTypes();
static void RegisterGlobals();
static void RegisterLogger();
static void RegisterListener(std::string observable, std::string luaFunctionName);
static void UnregisterListener(std::string observable, std::string key);

void JA2Require(std::string scriptFileName)
{
	if (isLuaInitialized)
	{
		throw std::runtime_error("JA2Require is not allowed after initialization");
	}

	if (loadedScripts.find(scriptFileName) != loadedScripts.end())
	{
		STLOGW("Script file '{}' has already been loaded", scriptFileName);
		return;
	}
	
	STLOGD("Loading LUA script file: {}", scriptFileName);
	std::string scriptbody = FileReadAsText(
		AutoSGPFile(GCM->openGameResForReading(SCRIPTS_DIR "/" + scriptFileName))
	).to_std_string();
	lua.script(scriptbody, ST::format("@{}", scriptFileName).to_std_string());
}

void InitScriptingEngine()
{
	loadedScripts.clear();
	isLuaInitialized = false;
	isLuaDisabled = false;
	counter = 0;

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
		STLOGE("Lua script engine has failed to initialize:\n {}", ex.what());
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

	lua.new_usertype<STRUCTURE>("STRUCTURE",
		"sGridNo", &STRUCTURE::sGridNo,
		"uiFlags", &STRUCTURE::fFlags
		);

	lua.new_usertype<StrategicMapElement>("StrategicMapElement",
		"bNameId", &StrategicMapElement::bNameId,
		"fEnemyControlled", &StrategicMapElement::fEnemyControlled,
		"fEnemyAirControlled", &StrategicMapElement::fEnemyAirControlled
		);

	lua.new_usertype<TacticalStatusType>("TacticalStatusType",
		"fEnemyInSector", &TacticalStatusType::fEnemyInSector,
		"fDidGameJustStart", &TacticalStatusType::fDidGameJustStart
		);

	lua.new_usertype<STRATEGICEVENT>("STRATEGICEVENT",
		"uiTimeStamp", &STRATEGICEVENT::uiTimeStamp,
		"uiParam", &STRATEGICEVENT::uiParam,
		"uiTimeOffset", &STRATEGICEVENT::uiTimeOffset,
		"ubEventFrequency", &STRATEGICEVENT::ubEventType,
		"ubEventKind", &STRATEGICEVENT::ubCallbackID
		);

	lua.new_usertype<GAME_OPTIONS>("GAME_OPTIONS",
		"fGunNut", &GAME_OPTIONS::fGunNut,
		"fSciFi", &GAME_OPTIONS::fSciFi,
		"ubDifficultyLevel", &GAME_OPTIONS::ubDifficultyLevel,
		"fTurnTimeLimit", &GAME_OPTIONS::fTurnTimeLimit,
		"ubGameSaveMode", &GAME_OPTIONS::ubGameSaveMode
		);
	
	lua.new_usertype<SOLDIERTYPE>("SOLDIERTYPE",
		"ubID", &SOLDIERTYPE::ubID,
		"ubProfile", &SOLDIERTYPE::ubProfile,
		"ubBodyType", &SOLDIERTYPE::ubBodyType,
		"ubSoldierClass", &SOLDIERTYPE::ubSoldierClass,
		"bTeam", &SOLDIERTYPE::bTeam,
		"ubCivilianGroup", &SOLDIERTYPE::ubCivilianGroup,
		"bNeutral", &SOLDIERTYPE::bNeutral,

		"bLifeMax", &SOLDIERTYPE::bLifeMax,
		"bLife", &SOLDIERTYPE::bLife,
		"bBreath", &SOLDIERTYPE::bBreath,
		"bBreathMax", &SOLDIERTYPE::bBreathMax,
		"bCamo", &SOLDIERTYPE::bCamo,

		"bAgility", &SOLDIERTYPE::bAgility,
		"bDexterity", &SOLDIERTYPE::bDexterity,
		"bExplosive", &SOLDIERTYPE::bExplosive,
		"bLeadership", &SOLDIERTYPE::bLeadership,
		"bMarksmanship", &SOLDIERTYPE::bMarksmanship,
		"bMechanical", &SOLDIERTYPE::bMechanical,
		"bMedical", &SOLDIERTYPE::bMedical,
		"bStrength", &SOLDIERTYPE::bStrength,
		"bWisdom", &SOLDIERTYPE::bWisdom,

		"bExpLevel", &SOLDIERTYPE::bExpLevel,
		"ubSkillTrait1", &SOLDIERTYPE::ubSkillTrait1,
		"ubSkillTrait2", &SOLDIERTYPE::ubSkillTrait2,

		"HeadPal", &SOLDIERTYPE::HeadPal,
		"PantsPal", &SOLDIERTYPE::PantsPal,
		"VestPal", &SOLDIERTYPE::VestPal,
		"SkinPal", &SOLDIERTYPE::SkinPal,

		"ubBattleSoundID", &SOLDIERTYPE::ubBattleSoundID
		);

	lua.new_usertype<BOOLEAN_S>("BOOLEAN_S",
		"val", &BOOLEAN_S::val
		);
	lua.new_usertype<UINT8_S>("UINT8_S",
		"val", &UINT8_S::val
		);
}

static void RegisterGlobals()
{
	lua["gTacticalStatus"] = &gTacticalStatus;
	lua["gubQuest"] = &gubQuest;
	lua["gubFact"] = &gubFact;
	lua["gGameOptions"] = &gGameOptions;

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

	lua.set_function("___noop", []() {});
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

/**
 * Invokes a Lua function by name
 */
template<typename ...A>
static void InvokeFunction(ST::string functionName, A... args)
{
	if (isLuaDisabled)
	{
		SLOGE("Scripting engine has been disabled due to a previous error"); 
		return;
	}

	sol::protected_function func = lua[functionName.to_std_string()];
	if (!func.valid())
	{
		STLOGE("Function {} is not defined", functionName);
		isLuaDisabled = true;
		return;
	}
	
	auto result = func.call(args...);
	if (!result.valid())
	{
		sol::error err = result;
		STLOGE("Lua script had an error. Scripting engine is now DISABLED. The error was:\n{}", err.what());
		isLuaDisabled = true;
	}
}

// Creates a typed std::function out of a Lua function
template<typename ...A>
static std::function<void(A...)> wrap(std::string luaFunc)
{
	return [luaFunc](A... args) {
		InvokeFunction(luaFunc, args...);
	};
}

static void _RegisterListener(std::string observable, std::string luaFunc, ST::string key)
{
	if (isLuaInitialized)
	{
		throw std::runtime_error("RegisterListener is not allowed after initialization");
	}

	if      (observable == "OnStructureDamaged")         OnStructureDamaged.addListener(key, wrap<INT16, INT16, INT8, INT16, STRUCTURE*, UINT8, BOOLEAN>(luaFunc));
	else if (observable == "BeforeStructureDamaged")     BeforeStructureDamaged.addListener(key, wrap<INT16, INT16, INT8, INT16, STRUCTURE*, UINT32, BOOLEAN_S*>(luaFunc));
	else if (observable == "OnAirspaceControlUpdated")   OnAirspaceControlUpdated.addListener(key, wrap<>(luaFunc));
	else if (observable == "BeforePrepareSector")        BeforePrepareSector.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnSoldierCreated")           OnSoldierCreated.addListener(key, wrap<SOLDIERTYPE*>(luaFunc));
	else {
		ST::string err = ST::format("There is no observable named '{}'", observable);
		throw std::logic_error(err.to_std_string());
	}
}

/**
 * Registers a callback listener with an Observable, to receive notifications in Lua scripts.
 * This function can only be used during initialization.
 * @param observable the name of an Observable
 * @param luaFunc name of the function handling callback
 * @ingroup funclib-general
 */
static void RegisterListener(std::string observable, std::string luaFunc)
{
	ST::string key = ST::format("mod:{03d}", counter++);
	_RegisterListener(observable, luaFunc, key);
}

/**
 * Unregisters a listener from the Observable.
 * This function can only be used during initialization.
 * @param observable
 * @param key
 * @ingroup funclib-general
 */
static void UnregisterListener(std::string observable, std::string key)
{
	_RegisterListener(observable, "___noop", key);
}
