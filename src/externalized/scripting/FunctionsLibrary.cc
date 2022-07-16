#include "FunctionsLibrary.h"
#include "Arms_Dealer_Init.h"
#include "Campaign_Types.h"
#include "Dialogue_Control.h"
#include "Game_Event_Hook.h"
#include "Handle_Items.h"
#include "Item_Types.h"
#include "Items.h"
#include "JAScreens.h"
#include "Laptop.h"
#include "MessageBoxScreen.h"
#include "Overhead.h"
#include "Queen_Command.h"
#include "SaveLoadGameStates.h"
#include "Soldier_Profile.h"
#include "StrategicMap.h"
#include "Exit_Grids.h"
#include "WorldMan.h"
#include "SaveLoadMap.h"
#include "RenderWorld.h"
#include <stdexcept>
#include <string>
#include <string_theory/format>
#include <string_theory/string>

std::string GetCurrentSector()
{
	ST::string str = gWorldSector.AsShortString();
	if (gWorldSector.z > 0)
	{
		str = gWorldSector.AsLongString();
	}
	return str.to_std_string();
}

std::tuple<int, int, int> GetCurrentSectorLoc()
{
	return std::make_tuple(gWorldSector.x, gWorldSector.y, gWorldSector.z);
}

static UINT8 ToSectorID(const std::string sectorID)
{
	if (!SGPSector().IsValid(sectorID.c_str()))
	{
		ST::string err = ST::format("The given sectorID ('{}') is invalid", sectorID);
		throw std::runtime_error(err.to_std_string());
	}
	SGPSector sector = SGPSector::FromShortString(sectorID);
	return sector.AsByte();
}

SECTORINFO* GetSectorInfo(const std::string sectorID)
{
	UINT8 ubSector = ToSectorID(sectorID);
	return &(SectorInfo[ubSector]);
}

UNDERGROUND_SECTORINFO* GetUndergroundSectorInfo(const std::string sectorID)
{
	auto pos = sectorID.find('-');
	if (pos == std::string::npos) throw std::runtime_error("Invalid underground sectorID format");

	std::string stSector = sectorID.substr(0, pos);
	UINT8 ubSectorZ = std::stoi(sectorID.substr(pos + 1));
	if (!SGPSector().IsValid(stSector.c_str()) || ubSectorZ == 0 || ubSectorZ > 3)
	{
		throw std::runtime_error("Invalid underground sectorID");
	}

	const SGPSector& ubSector = SGPSector::FromShortString(stSector, ubSectorZ);
	return FindUnderGroundSector(ubSector);
}

StrategicMapElement* GetStrategicMapElement(const std::string sectorID)
{
	SGPSector sector = SGPSector::FromShortString(sectorID);
	UINT8 index = sector.AsStrategicIndex();
	return &(StrategicMap[index]);
}

OBJECTTYPE* CreateItem(const UINT16 usItem, const INT8 bStatus)
{
	OBJECTTYPE* o = new OBJECTTYPE{};
	CreateItem(usItem, bStatus, o);
	return o;
}

OBJECTTYPE* CreateMoney(const UINT32 amt)
{
	auto o = new OBJECTTYPE{};
	CreateMoney(amt, o);
	return o;
}

void PlaceItem(const INT16 sGridNo, OBJECTTYPE* const pObject, const INT8 ubVisibility)
{
	AddItemToPool(sGridNo, pObject, static_cast<Visibility>(ubVisibility), 0, 0, 0);
}

MERCPROFILESTRUCT* GetMercProfile(const UINT8 ubProfileID)
{
	return &(GetProfile(ubProfileID));
}

ExtraGameStatesTable GetGameStates(const std::string key)
{
	auto stateKey = ST::format("scripts:{}", key);
	if (!g_gameStates.HasKey(stateKey))
	{
		return ExtraGameStatesTable{};
	}

	auto storedStates = g_gameStates.Get<std::map<ST::string, PRIMITIVE_VALUE>>(stateKey);
	ExtraGameStatesTable table{};
	for (auto& s : storedStates)
	{
		auto k = s.first.to_std_string();
		auto v = s.second;
		if (auto *b = std::get_if<bool>(&v)) table[k] = *b;
		else if (auto *i = std::get_if<int32_t>(&v)) table[k] = *i;
		else if (auto *str = std::get_if<ST::string>(&v)) table[k] = str->to_std_string();
		else if (auto *f = std::get_if<float>(&v)) table[k] = *f;
	}
	return table;
}

void PutGameStates(const std::string key, ExtraGameStatesTable const states)
{
	std::map<ST::string, PRIMITIVE_VALUE> storables{};
	for (auto &pair: states)
	{
		auto k = pair.first;
		auto v = pair.second;
		if (auto *b = std::get_if<bool>(&v)) storables[k] = *b;
		else if (auto *i = std::get_if<int32_t>(&v)) storables[k] = *i;
		else if (auto *s = std::get_if<std::string>(&v)) storables[k] = ST::string(*s);
		else if (auto *f = std::get_if<float>(&v)) storables[k] = *f;
	}
	g_gameStates.Set(ST::format("scripts:{}", key), storables);
}

void CenterAtGridNo(const INT16 sGridNo, const bool fForce)
{
	InternalLocateGridNo(sGridNo, fForce);
}

void StrategicNPCDialogue(UINT8 const ubProfileID, UINT16 const usQuoteNum)
{
	CharacterDialogue(ubProfileID, usQuoteNum, GetExternalNPCFace(ubProfileID), DIALOGUE_EXTERNAL_NPC_UI, FALSE, true);
}

BOOLEAN DoMercBattleSound_(SOLDIERTYPE* const s, UINT8 const battle_snd_id)
{
	return DoMercBattleSound(s, (BattleSound)battle_snd_id);
}

std::vector<SOLDIERTYPE*> ListSoldiersFromTeam(UINT8 const ubTeamID)
{
	std::vector<SOLDIERTYPE *> soldiers;
	FOR_EACH_IN_TEAM(s, ubTeamID)
	soldiers.push_back(s);
	return soldiers;
}

void AddEveryDayStrategicEvent_(UINT8 const ubCallbackID, UINT32 const uiStartMin, UINT32 const uiParam)
{
	BOOLEAN result = AddEveryDayStrategicEvent((StrategicEventKind)ubCallbackID, uiStartMin, uiParam);
	if (!result)
	{
		SLOGW("Failed to add daily strategic event {}", ubCallbackID);
	}
}

GROUP *CreateNewEnemyGroupDepartingSector(std::string const sectorID, UINT8 const ubNumAdmins, UINT8 const ubNumTroops, UINT8 const ubNumElites)
{
	UINT32 uiSector = ToSectorID(sectorID);
	return CreateNewEnemyGroupDepartingFromSector(uiSector, ubNumAdmins, ubNumTroops, ubNumElites);
}

void AddStrategicEvent_(UINT8 const ubCallbackID, UINT32 const uiMinStampSeconds, UINT32 const uiParams)
{
	BOOLEAN result = AddStrategicEventUsingSeconds((StrategicEventKind)ubCallbackID, uiMinStampSeconds, uiParams);
	if (!result)
	{
		SLOGW("Failed to add one time strategic event {}", ubCallbackID);
	}
}

void StartQuest(UINT8, const SGPSector &);
void StartQuest_(UINT8 const ubQuestID, const std::string sectorID)
{
	if (!sectorID.empty())
	{
		SGPSector s = SGPSector::FromShortString(sectorID);
		StartQuest(ubQuestID, s);
	}
	else
	{
		SGPSector s = SGPSector{-1, -1};
		StartQuest(ubQuestID, s);
	}
}

void EndQuest(UINT8, const SGPSector &);
void EndQuest_(UINT8 const ubQuestID, const std::string sectorID)
{
	if (!sectorID.empty())
	{
		SGPSector s = SGPSector::FromShortString(sectorID);
		EndQuest(ubQuestID, s);
	}
	else
	{
		SGPSector s = SGPSector{-1, -1};
		EndQuest(ubQuestID, s);
	}
}

void GuaranteeAtLeastXItemsOfIndex(ArmsDealerID, UINT16, UINT8);
void GuaranteeAtLeastXItemsOfIndex_(INT8 const bDealerID, UINT16 const usItemIndex, UINT8 const ubNumItems)
{
	GuaranteeAtLeastXItemsOfIndex((ArmsDealerID)bDealerID, usItemIndex, ubNumItems);
}

void RemoveRandomItemFromArmsDealerInventory(ArmsDealerID, UINT16, UINT8);
void RemoveRandomItemFromDealerInventory(INT8 bDealerID, UINT16 usItemIndex, UINT8 ubHowMany)
{
	RemoveRandomItemFromArmsDealerInventory((ArmsDealerID)bDealerID, usItemIndex, ubHowMany);
}

std::vector<DEALER_ITEM_HEADER*> GetDealerInventory(UINT8 ubDealerID)
{
	std::vector<DEALER_ITEM_HEADER*> items{};
	for (DEALER_ITEM_HEADER& i : gArmsDealersInventory[ubDealerID])
	{
		items.push_back(&i);
	}
	return items;
}

void DoScreenIndependantMessageBox(const ST::string &, MessageBoxFlags, MSGBOX_CALLBACK);
void DoBasicMessageBox(const ST::string text)
{
	DoScreenIndependantMessageBox(text, MSG_BOX_FLAG_OK, NULL);
}

void ExecuteTacticalTextBox(INT16 sLeftPosition, INT16 sTopPosition, const ST::string &pString);
void ExecuteTacticalTextBox_(INT16 sLeftPosition, INT16 sTopPosition, ST::string pString)
{
	ExecuteTacticalTextBox(sLeftPosition, sTopPosition, pString);
}

extern LaptopMode guiCurrentWWWMode;
extern LaptopMode guiCurrentLaptopMode;

void SetLaptopModes(UINT32 uiLaptopMode, UINT32 uiWWWMode)
{
	guiCurrentLaptopMode = static_cast<LaptopMode>(uiLaptopMode);
	guiCurrentWWWMode = static_cast<LaptopMode>(uiWWWMode);
}

void AddExitGridToWorld(INT32 iMapIndex, EXITGRID *pExitGrid);
void AddExitGridToWorld_(INT32 iMapIndex, UINT16 gridNo, UINT8 destX, UINT8 destY, UINT8 destZ)
{
	SGPSector s{destX, destY, static_cast<INT8>(destZ)};
	EXITGRID exitGrid {gridNo, s};
	AddExitGridToWorld(iMapIndex, &exitGrid);
}

UINT16 GetTileIndexFromTypeSubIndex(UINT32, UINT16);
void AddStructToHead(UINT32, UINT16);
void RemoveStruct(UINT32, UINT16);
void RemoveStructFromMap(UINT16 gridNo, UINT32 tileType, UINT16 tileSubIndex)
{
	ApplyMapChangesToMapTempFile app;
	UINT16 tileIndex = GetTileIndexFromTypeSubIndex(tileType, tileSubIndex);
	RemoveStruct(gridNo, tileIndex);
//	gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
//	SetRenderFlags(RENDER_FLAG_FULL);
}

void AddStructToMap(UINT16 gridNo, UINT32 tileType, UINT16 tileSubIndex)
{
	ApplyMapChangesToMapTempFile app;
	UINT16 tileIndex = GetTileIndexFromTypeSubIndex(tileType, tileSubIndex);
	AddStructToHead(gridNo, tileIndex);
//	gTacticalStatus.uiFlags |= NOHIDE_REDUNDENCY;
//	SetRenderFlags(RENDER_FLAG_FULL);
}
