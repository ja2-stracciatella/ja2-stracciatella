#include "FunctionsLibrary.h"
#include "Arms_Dealer_Init.h"
#include "Campaign_Types.h"
#include "Handle_Items.h"
#include "Item_Types.h"
#include "Items.h"
#include "JAScreens.h"
#include "MessageBoxScreen.h"
#include "Queen_Command.h"
#include "SaveLoadGameStates.h"
#include "Soldier_Profile.h"
#include "StrategicMap.h"
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

SECTORINFO* GetSectorInfo(const std::string sectorID)
{
	if (!SGPSector().IsValid(sectorID.c_str()))
	{
		ST::string err = ST::format("The given sectorID ('{}') is invalid", sectorID);
		throw std::runtime_error(err.to_std_string());
	}
	UINT8 ubSector = SGPSector::FromShortString(sectorID).AsByte();
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

OBJECTTYPE* CreateItem(const UINT16 usItem, const INT8 bStatus)
{
	OBJECTTYPE* o = new OBJECTTYPE{};
	CreateItem(ItemId(usItem), bStatus, o);
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

void GuaranteeAtLeastXItemsOfIndex(ArmsDealerID, ItemId, UINT8);
void GuaranteeAtLeastXItemsOfIndex_(INT8 const bDealerID, UINT16 const usItemIndex, UINT8 const ubNumItems)
{
	GuaranteeAtLeastXItemsOfIndex((ArmsDealerID)bDealerID, ItemId(usItemIndex), ubNumItems);
}

void RemoveRandomItemFromArmsDealerInventory(ArmsDealerID, ItemId, UINT8);
void RemoveRandomItemFromDealerInventory(INT8 bDealerID, UINT16 usItemIndex, UINT8 ubHowMany)
{
	RemoveRandomItemFromArmsDealerInventory((ArmsDealerID)bDealerID, ItemId(usItemIndex), ubHowMany);
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
