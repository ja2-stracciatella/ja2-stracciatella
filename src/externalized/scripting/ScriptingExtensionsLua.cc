#include "ScriptingExtensions.h"

#define SOL_CHECK_ARGUMENTS 1
#define SOL_PRINT_ERRORS 1
#define SOL_ALL_SAFETIES_ON 1
#include <limits>
#include <sol/sol.hpp>  // this needs to be included first
#include "STStringHandler.h"

#include "Arms_Dealer_Init.h"
#include "Campaign_Types.h"
#include "ContentManager.h"
#include "FileMan.h"
#include "FunctionsLibrary.h"
#include "Game_Events.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "LaptopSave.h"
#include "Logger.h"
#include "Overhead.h"
#include "Quests.h"
#include "Soldier_Profile.h"
#include "Soldier_Profile_Type.h"
#include "StrategicMap.h"
#include "Strategic_Movement.h"
#include "Structure.h"
#include "Types.h"
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

static bool isLuaInitialized = false;
static bool isLuaDisabled = false;
static sol::state lua;

// an increment counter used to generate unique keys for listeners
static unsigned int counter;

extern ScreenID guiCurrentScreen;

static void RegisterUserTypes();
static void RegisterGlobals();
static void RegisterLogger();
static void RegisterListener(std::string observable, std::string luaFunctionName);
static void UnregisterListener(std::string observable, std::string key);

int StracciatellaLoadFileRequire(lua_State* L)
{
	ST::string path = sol::stack::get<std::string>(L);
	SLOGD("Loading LUA script file: {}", path);

	try {
		AutoSGPFile file(GCM->openGameResForReading(SCRIPTS_DIR "/" + path));
		std::string script = file->readStringToEnd().to_std_string();

		luaL_loadbuffer(L, script.data(), script.size(), ST::format("@{}", path).c_str());
		return 1;
	} catch (const std::runtime_error& ex) {
		auto errorMessage = ST::format("Error loading lua required file `{}`: {}", path, ex.what());
		sol::stack::push(L, errorMessage.c_str());
		return 1;
	}
}

void RunEntryPoint()
{
	auto luaName = ENTRYPOINT_SCRIPT;
	auto fileName = SCRIPTS_DIR "/" ENTRYPOINT_SCRIPT;
	
	SLOGD("Loading LUA script file: {}", luaName);
	AutoSGPFile f{GCM->openGameResForReading(fileName)};
	std::string scriptbody = f->readStringToEnd().to_std_string();
	auto result = lua.safe_script(scriptbody, ST::format("@{}", luaName).to_std_string());
	if (!result.valid())
	{
		sol::error err = result;
		SLOGE("Lua script had an error. Scripting engine is now DISABLED. The error was:");
		SLOGE(err.what());
		isLuaDisabled = true;
	}
}

void InitScriptingEngine()
{
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
			sol::lib::package,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table
		);

		lua.clear_package_loaders();
		lua.add_package_loader(StracciatellaLoadFileRequire);

		RegisterUserTypes();
		RegisterGlobals();
		RegisterLogger();

		RunEntryPoint();

		isLuaInitialized = true;
	} 
	catch (const std::exception &ex)
	{
		SLOGE("Lua script engine has failed to initialize:\n {}", ex.what());
		ST::string err = "The game cannot be started due to an error in the mod scripts. Check the logs for more details.";
		std::throw_with_nested(std::runtime_error(err.to_std_string()));
	}
}

static void RegisterUserTypes()
{
	lua.new_usertype<SGPSector>("SGPSector",
		"x", &SGPSector::x,
		"y", &SGPSector::y,
		"z", &SGPSector::z
		);

	lua.new_usertype<SECTORINFO>("SECTORINFO",
		"ubNumAdmins", &SECTORINFO::ubNumAdmins,
		"ubNumTroops", &SECTORINFO::ubNumTroops,
		"ubNumElites", &SECTORINFO::ubNumElites,

		"bBloodCats", &SECTORINFO::bBloodCats,
		"bBloodCatPlacements", &SECTORINFO::bBloodCatPlacements,

		"uiFlags", &SECTORINFO::uiFlags,
		"ubGarrisonID", &SECTORINFO::ubGarrisonID,

		"fSurfaceWasEverPlayerControlled", &SECTORINFO::fSurfaceWasEverPlayerControlled
		);

	lua.new_usertype<UNDERGROUND_SECTORINFO>("UNDERGROUND_SECTORINFO",
		"ubNumAdmins", &UNDERGROUND_SECTORINFO::ubNumAdmins,
		"ubNumTroops", &UNDERGROUND_SECTORINFO::ubNumTroops,
		"ubNumElites", &UNDERGROUND_SECTORINFO::ubNumElites,
		"uiFlags", &UNDERGROUND_SECTORINFO::uiFlags
		);

	lua.new_usertype<OBJECTTYPE>("OBJECTTYPE",
		"usItem", &OBJECTTYPE::usItem,
		"bTrap", &OBJECTTYPE::bTrap,
		"bActionValue", &OBJECTTYPE::bActionValue
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
		"fDidGameJustStart", &TacticalStatusType::fDidGameJustStart,
		"uiFlags", &TacticalStatusType::uiFlags,
		"fEnemyFlags", &TacticalStatusType::fEnemyFlags
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

	lua.new_usertype<SGPSector>("SGPSector",
		"x", &SGPSector::x,
		"y", &SGPSector::y,
		"z", &SGPSector::z
		);

	lua.new_usertype<SOLDIERTYPE>("SOLDIERTYPE",
		"ubID", &SOLDIERTYPE::ubID,
		"ubProfile", &SOLDIERTYPE::ubProfile,
		"name", &SOLDIERTYPE::name,

		"ubWhatKindOfMercAmI", &SOLDIERTYPE::ubWhatKindOfMercAmI,
		"bActive", &SOLDIERTYPE::bActive,
		"bVisible", &SOLDIERTYPE::bVisible,
		"ubBodyType", &SOLDIERTYPE::ubBodyType,
		"ubSoldierClass", &SOLDIERTYPE::ubSoldierClass,
		"uiAnimSubFlags", &SOLDIERTYPE::uiAnimSubFlags,
		"bTeam", &SOLDIERTYPE::bTeam,
		"ubCivilianGroup", &SOLDIERTYPE::ubCivilianGroup,
		"bNeutral", &SOLDIERTYPE::bNeutral,

		"bLifeMax", &SOLDIERTYPE::bLifeMax,
		"bLife", &SOLDIERTYPE::bLife,
		"bBreath", &SOLDIERTYPE::bBreath,
		"bBreathMax", &SOLDIERTYPE::bBreathMax,
		"sBreathRed", &SOLDIERTYPE::sBreathRed,
		"bActionPoints", &SOLDIERTYPE::bActionPoints,
		"bCamo", &SOLDIERTYPE::bCamo,

		"bCollapsed", &SOLDIERTYPE::bCollapsed,
		"fMercAsleep", &SOLDIERTYPE::fMercAsleep,
		"fMercCollapsedFlag", &SOLDIERTYPE::fMercCollapsedFlag,

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

		"ubBattleSoundID", &SOLDIERTYPE::ubBattleSoundID,

		"sSector", &SOLDIERTYPE::sSector,
		"fBetweenSectors", &SOLDIERTYPE::fBetweenSectors,

		"sGridNo", &SOLDIERTYPE::sGridNo,
		"bLevel", &SOLDIERTYPE::bLevel,
		"sInitialGridNo", &SOLDIERTYPE::sInitialGridNo,

		"usAnimState", &SOLDIERTYPE::usAnimState,

		"ubStrategicInsertionCode", &SOLDIERTYPE::ubStrategicInsertionCode,
		"usStrategicInsertionData", &SOLDIERTYPE::usStrategicInsertionData,
		"ubInsertionDirection", &SOLDIERTYPE::ubInsertionDirection,

		"iTotalContractLength", &SOLDIERTYPE::iTotalContractLength,
		"iEndofContractTime", &SOLDIERTYPE::iEndofContractTime
	);

	lua.new_usertype<MERCPROFILESTRUCT>("MERCPROFILESTRUCT",
		"zNickname", &MERCPROFILESTRUCT::zNickname,
		"zName", &MERCPROFILESTRUCT::zName,

		"ubBodyType", &MERCPROFILESTRUCT::ubBodyType,
		"ubFaceIndex", &MERCPROFILESTRUCT::ubFaceIndex,
		"usEyesX", &MERCPROFILESTRUCT::usEyesX,
		"usEyesY", &MERCPROFILESTRUCT::usEyesY,
		"usMouthX", &MERCPROFILESTRUCT::usMouthX,
		"usMouthY", &MERCPROFILESTRUCT::usMouthY,

		"ubNeedForSleep", &MERCPROFILESTRUCT::ubNeedForSleep,

		"bSkillTrait", &MERCPROFILESTRUCT::bSkillTrait,
		"bSkillTrait2", &MERCPROFILESTRUCT::bSkillTrait2,

		"bAgility", &MERCPROFILESTRUCT::bAgility,
		"bDexterity", &MERCPROFILESTRUCT::bDexterity,
		"bExplosive", &MERCPROFILESTRUCT::bExplosive,
		"bLeadership", &MERCPROFILESTRUCT::bLeadership,
		"bMarksmanship", &MERCPROFILESTRUCT::bMarksmanship,
		"bMechanical", &MERCPROFILESTRUCT::bMechanical,
		"bMedical", &MERCPROFILESTRUCT::bMedical,
		"bStrength", &MERCPROFILESTRUCT::bStrength,
		"bWisdom", &MERCPROFILESTRUCT::bWisdom,

		"bTown", &MERCPROFILESTRUCT::bTown,
		"sSector", &MERCPROFILESTRUCT::sSector,
		"sGridNo", &MERCPROFILESTRUCT::sGridNo,
		"ubLastDateSpokenTo", &MERCPROFILESTRUCT::ubLastDateSpokenTo,

		"sSector", &MERCPROFILESTRUCT::sSector,
		"bSectorZ", &MERCPROFILESTRUCT::bSectorZ,
		"sGridNo", &MERCPROFILESTRUCT::sGridNo,

		"ubStrategicInsertionCode", &MERCPROFILESTRUCT::ubStrategicInsertionCode,
		"usStrategicInsertionData", &MERCPROFILESTRUCT::usStrategicInsertionData,
		"fUseProfileInsertionInfo", &MERCPROFILESTRUCT::fUseProfileInsertionInfo,

		"bMercStatus", &MERCPROFILESTRUCT::bMercStatus,
		"uiDayBecomesAvailable", &MERCPROFILESTRUCT::uiDayBecomesAvailable,
		"iMercMercContractLength", &MERCPROFILESTRUCT::iMercMercContractLength,
		"sSalary", &MERCPROFILESTRUCT::sSalary,
		"uiWeeklySalary", &MERCPROFILESTRUCT::uiWeeklySalary,
		"uiBiWeeklySalary", &MERCPROFILESTRUCT::uiBiWeeklySalary,
		"bMedicalDeposit", &MERCPROFILESTRUCT::bMedicalDeposit,
		"sMedicalDepositAmount", &MERCPROFILESTRUCT::sMedicalDepositAmount,
		"usOptionalGearCost", &MERCPROFILESTRUCT::usOptionalGearCost,

		"iBalance", &MERCPROFILESTRUCT::iBalance,
		"ubLastDateSpokenTo", &MERCPROFILESTRUCT::ubLastDateSpokenTo,

		"bSkillTrait", &MERCPROFILESTRUCT::bSkillTrait,
		"bSkillTrait2", &MERCPROFILESTRUCT::bSkillTrait2
		);

	lua.new_usertype<GROUP>("GROUP",
		"ubGroupID", &GROUP::ubGroupID,
		"ubNext", &GROUP::ubNext,
		"uiTraverseTime", &GROUP::uiTraverseTime,
		"ubMoveType", &GROUP::ubMoveType,
		"setArrivalTime", &GROUP::setArrivalTime
		);

	lua.new_usertype<LaptopSaveInfoStruct>("LaptopSaveInfoStruct",
		"iCurrentBalance", &LaptopSaveInfoStruct::iCurrentBalance,

		"gubPlayersMercAccountStatus", &LaptopSaveInfoStruct::gubPlayersMercAccountStatus,
		"ubPlayerBeenToMercSiteStatus", &LaptopSaveInfoStruct::ubPlayerBeenToMercSiteStatus
		);

	lua.new_usertype<DEALER_ITEM_HEADER>("DEALER_ITEM_HEADER",
		"ubTotalItems", &DEALER_ITEM_HEADER::ubTotalItems,
		"ubPerfectItems", &DEALER_ITEM_HEADER::ubPerfectItems,
		"ubStrayAmmo", &DEALER_ITEM_HEADER::ubStrayAmmo,
		"uiOrderArrivalTime", &DEALER_ITEM_HEADER::uiOrderArrivalTime,
		"ubQtyOnOrder", &DEALER_ITEM_HEADER::ubQtyOnOrder,
		"fPreviouslyEligible", &DEALER_ITEM_HEADER::fPreviouslyEligible
		);

	lua.new_usertype<BOOLEAN_S>("BOOLEAN_S",
		"val", &BOOLEAN_S::val
		);
	lua.new_usertype<UINT8_S>("UINT8_S",
		"val", &UINT8_S::val
		);
	lua.new_usertype<UINT32_S>("UINT32_S",
		"val", &UINT32_S::val
	)	;
}

static void RegisterGlobals()
{
	lua["gTacticalStatus"] = &gTacticalStatus;
	lua["gubQuest"] = &gubQuest;
	lua["gubFact"] = &gubFact;
	lua["gGameOptions"] = &gGameOptions;
	lua["guiCurrentScreen"] = &guiCurrentScreen;
	lua["LaptopSaveInfo"] = &LaptopSaveInfo;

	lua.set_function("GetCurrentSector", GetCurrentSector);
	lua.set_function("GetCurrentSectorLoc", GetCurrentSectorLoc);
	lua.set_function("GetSectorInfo", GetSectorInfo);
	lua.set_function("GetUndergroundSectorInfo", GetUndergroundSectorInfo);
	lua.set_function("GetStrategicMapElement", GetStrategicMapElement);
	lua.set_function("SetThisSectorAsPlayerControlled", SetThisSectorAsPlayerControlled);

	lua.set_function("CreateItem", CreateItem);
	lua.set_function("CreateMoney", CreateMoney);
	lua.set_function("PlaceItem", PlaceItem);
	lua.set_function("RemoveAllUnburiedItems", RemoveAllUnburiedItems);

	lua.set_function("DoBasicMessageBox", DoBasicMessageBox);
	lua.set_function("ExecuteTacticalTextBox", ExecuteTacticalTextBox_);

	lua.set_function("GetMercProfile", GetMercProfile);
	lua.set_function("CenterAtGridNo", CenterAtGridNo);
	lua.set_function("PythSpacesAway", PythSpacesAway);

	lua.set_function("ListSoldiersFromTeam", ListSoldiersFromTeam);
	lua.set_function("FindSoldierByProfileID", FindSoldierByProfileID);

	lua.set_function("ChangeSoldierState", ChangeSoldierState);
	lua.set_function("RemoveObjectFromSoldierProfile", RemoveObjectFromSoldierProfile);

	lua.set_function("TriggerNPCRecord", TriggerNPCRecord);
	lua.set_function("StrategicNPCDialogue", StrategicNPCDialogue);
	lua.set_function("TacticalCharacterDialogue", TacticalCharacterDialogue);
	lua.set_function("DeleteTalkingMenu", DeleteTalkingMenu);
	lua.set_function("PlayJA2SampleFromFile", PlayJA2SampleFromFile);
	lua.set_function("DoMercBattleSound", DoMercBattleSound_);

	lua.set_function("IgniteExplosion", IgniteExplosion);
	lua.set_function("SetOffBombsByFrequency", SetOffBombsByFrequency);

	lua.set_function("ChangeSoldierStance", ChangeSoldierStance);
	lua.set_function("ChangeSoldierState", ChangeSoldierState);

	lua.set_function("CreateNewEnemyGroupDepartingSector", CreateNewEnemyGroupDepartingSector);

	lua.set_function("GetWorldTotalMin", GetWorldTotalMin);
	lua.set_function("GetWorldTotalSeconds", GetWorldTotalSeconds);
	lua.set_function("GetWorldDay", GetWorldDay);
	lua.set_function("AddEveryDayStrategicEvent", AddEveryDayStrategicEvent_);
	lua.set_function("AddStrategicEvent", AddStrategicEvent_);

	lua.set_function("StartQuest", StartQuest_);
	lua.set_function("EndQuest", EndQuest_);
	lua.set_function("SetFactTrue", SetFactTrue);
	lua.set_function("SetFactFalse", SetFactFalse);
	lua.set_function("CheckFact", CheckFact);

	lua.set_function("AddEmailMessage", AddEmailMessage);
	lua.set_function("SetBookMark", SetBookMark);
	lua.set_function("AddTransactionToPlayersBook", AddTransactionToPlayersBook);
	lua.set_function("AddHistoryToPlayersLog", AddHistoryToPlayersLog);

	lua.set_function("TriggerNPCRecord", TriggerNPCRecord);
	lua.set_function("StrategicNPCDialogue", StrategicNPCDialogue);

	lua.set_function("GetGameStates", GetGameStates);
	lua.set_function("PutGameStates", PutGameStates);

	lua.set_function("DailyCheckOnItemQuantities", DailyCheckOnItemQuantities);
	lua.set_function("GuaranteeAtLeastXItemsOfIndex", GuaranteeAtLeastXItemsOfIndex_);
	lua.set_function("RemoveRandomItemFromDealerInventory", RemoveRandomItemFromDealerInventory);
	lua.set_function("GetDealerInventory", GetDealerInventory);
	lua.set_function("StartShopKeeperTalking", StartShopKeeperTalking);
	lua.set_function("EnterShopKeeperInterfaceScreen", EnterShopKeeperInterfaceScreen);

	lua.set_function("dofile",   []() { throw std::logic_error("dofile is not allowed. Use require instead"); });
	lua.set_function("loadfile", []() { throw std::logic_error("loadfile is not allowed. Use require instead"); });

	lua.set_function("___noop", []() {});
	lua.set_function("RegisterListener", RegisterListener);
	lua.set_function("UnregisterListener", UnregisterListener);
}

static void LogLuaMessage(LogLevel const level, const std::string& msg) {
	lua_Debug info;
	// Stack position 0 is the c function we are in
	// Stack position 1 is the calling lua script
	lua_getstack(lua, 1, &info);
	lua_getinfo(lua, "S", &info);
	Logger_log(level, msg.c_str(), info.short_src);
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
static void InvokeFunction(const ST::string& functionName, A... args)
{
	if (isLuaDisabled)
	{
		SLOGE("Scripting engine has been disabled due to a previous error"); 
		return;
	}

	sol::protected_function func = lua[functionName.to_std_string()];
	if (!func.valid())
	{
		SLOGE("Lua script had an error. Scripting engine is now DISABLED. The error was:");
		SLOGE("Function {} is not defined", functionName);
		isLuaDisabled = true;
		return;
	}
	
	auto result = func.call(args...);
	if (!result.valid())
	{
		sol::error err = result;
		SLOGE("Lua script had an error. Scripting engine is now DISABLED. The error was:");
		SLOGE(err.what());
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

static void _RegisterListener(const std::string& observable, const std::string& luaFunc, const ST::string& key)
{
	if (isLuaInitialized)
	{
		throw std::runtime_error("RegisterListener is not allowed after initialization");
	}

	if      (observable == "OnStructureDamaged")         OnStructureDamaged.addListener(key, wrap<INT16, INT16, INT8, INT16, STRUCTURE*, UINT8, BOOLEAN>(luaFunc));
	else if (observable == "BeforeStructureDamaged")     BeforeStructureDamaged.addListener(key, wrap<INT16, INT16, INT8, INT16, STRUCTURE*, UINT32, BOOLEAN_S*>(luaFunc));
	else if (observable == "OnItemAction")               OnItemAction.addListener(key, wrap<INT16, OBJECTTYPE*, BOOLEAN_S*>(luaFunc));
	else if (observable == "OnAirspaceControlUpdated")   OnAirspaceControlUpdated.addListener(key, wrap<>(luaFunc));
	else if (observable == "BeforePrepareSector")        BeforePrepareSector.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnEnterSector")              OnEnterSector.addListener(key, wrap<INT16,INT16,INT8>(luaFunc));
	else if (observable == "OnSoldierCreated")           OnSoldierCreated.addListener(key, wrap<SOLDIERTYPE*>(luaFunc));
	else if (observable == "OnSoldierDeath")             OnSoldierDeath.addListener(key, wrap<const SOLDIERTYPE*>(luaFunc));
	else if (observable == "OnSoldierGotItem")           OnSoldierGotItem.addListener(key, wrap<SOLDIERTYPE*, OBJECTTYPE*, INT16, INT8>(luaFunc));
	else if (observable == "OnInitNewCampaign")          OnInitNewCampaign.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnMercHired")                OnMercHired.addListener(key, wrap<SOLDIERTYPE*>(luaFunc));
	else if (observable == "OnEnterTacticalScreen")      OnEnterTacticalScreen.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnHandleStrategicScreen")    OnHandleStrategicScreen.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnStrategicEvent")           OnStrategicEvent.addListener(key, wrap<STRATEGICEVENT*, BOOLEAN_S*>(luaFunc));
	else if (observable == "OnCalcPlayerProgress")       OnCalcPlayerProgress.addListener(key, wrap<UINT8_S*>(luaFunc));
	else if (observable == "OnTimeCompressDisallowed")   OnTimeCompressDisallowed.addListener(key, wrap<BOOLEAN_S*>(luaFunc));
	else if (observable == "OnCheckQuests")              OnCheckQuests.addListener(key, wrap<UINT32, BOOLEAN_S*>(luaFunc));
	else if (observable == "OnQuestEnded")               OnQuestEnded.addListener(key, wrap<UINT8, INT16, INT16, BOOLEAN>(luaFunc));
	else if (observable == "OnNPCDoAction")              OnNPCDoAction.addListener(key, wrap<UINT8, UINT16, UINT8, BOOLEAN_S*>(luaFunc));
	else if (observable == "OnAddEmail")                 OnAddEmail.addListener(key, wrap<INT32, INT32, INT32, UINT8, BOOLEAN, INT32, UINT32, BOOLEAN_S*>(luaFunc));
	else if (observable == "BeforeGameSaved")            BeforeGameSaved.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnGameLoaded")               OnGameLoaded.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnDealerInventoryUpdated")   OnDealerInventoryUpdated.addListener(key, wrap<>(luaFunc));
	else if (observable == "OnItemTransacted")           OnItemTransacted.addListener(key, wrap<INT8, UINT16, BOOLEAN>(luaFunc));
	else if (observable == "OnItemPriced")               OnItemPriced.addListener(key, wrap<INT8, UINT16, BOOLEAN, UINT32_S*>(luaFunc));
	else if (observable == "OnMercHired")                OnMercHired.addListener(key, wrap<SOLDIERTYPE*>(luaFunc));
	else if (observable == "OnRPCRecruited")             OnRPCRecruited.addListener(key, wrap<SOLDIERTYPE*>(luaFunc));
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
static void RegisterListener(const std::string observable, const std::string luaFunc)
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
static void UnregisterListener(const std::string observable, const std::string key)
{
	_RegisterListener(observable, "___noop", key);
}
