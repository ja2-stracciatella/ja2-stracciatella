#include "EnumCodeGen.h"
#include <iostream>

#ifdef MAGIC_ENUM_SUPPORTED
#include "Animation_Data.h"
#include "Campaign_Types.h"
#include "Game_Events.h"
#include "Game_Event_Hook.h"
#include "GameSettings.h"
#include "Handle_Items.h"
#include "Item_Types.h"
#include "Overhead_Types.h"
#include "Soldier_Profile.h"
#include "Soldier_Control.h"
#include "TileDef.h"
#include "Quests.h"
#include "Sound_Control.h"


namespace magic_enum::customize
{
	template<> struct enum_range<ITEMDEFINE>	{ static constexpr int min = 0;	static constexpr int max = 400; };
	template<> struct enum_range<SectorIndex>	{ static constexpr int min = 0;	static constexpr int max = 256; };
	template<> struct enum_range<NPCIDs>	{ static constexpr int min = 0;	static constexpr int max = 200; };
	template<> struct enum_range<Quests>	{ static constexpr int min = 0;	static constexpr int max = 500; };
	template<> struct enum_range<Fact>	{ static constexpr int min = -1;	static constexpr int max = 400; };
	template<> struct enum_range<TileTypeDefines>	{ static constexpr int min = 0;	static constexpr int max = 200; };
	template<> struct enum_range<SoundID>	{ static constexpr int min = -1;	static constexpr int max = 300; };
}


static void PrintHeader(std::ostream& os)
{
	os << "--[[" << std::endl
	   << " This file was auto-generated. Do not manually edit." << std::endl
	   << "--]]" << std::endl << std::endl;
}

void PrintAllJA2Enums(std::ostream& os)
{
	PrintHeader(os);

	PrintEnumFlags<SectorFlags>(os, "SectorFlag");
	PrintEnum<SectorIndex>(os);

	PrintEnum<Team>(os);
	PrintEnum<CivilianGroup>(os);
	PrintEnum<SoldierClass>(os);
	PrintEnum<MilitiaLevel>(os);

	PrintEnum<InvSlotPos>(os);
	PrintEnum<ITEMDEFINE>(os, "Item");

	PrintEnum<Quests>(os, "Quest");
	PrintEnum<Fact>(os);

	PrintEnum<DifficultyLevel>(os);
	PrintEnum<GameSaveMode>(os);

	PrintEnum<StrategicEventKind>(os);
	PrintEnum<StrategicEventFrequency>(os);

	PrintEnum<NPCIDs>(os, "MercProfile");
	PrintEnum<SkillTrait>(os);
	PrintEnum<PersonalityTrait>(os);
	PrintEnum<Attitudes>(os, "Attitude");

	PrintEnum<SoldierBodyType>(os);
	PrintEnum<TileTypeDefines>(os, "TileType");
	PrintEnum<Visibility>(os);
	PrintEnum<BattleSound>(os);
	PrintEnum<SoundID>(os);
	PrintEnum<VehicleMovementType>(os);

	PrintEnum<MessageBoxFlags>(os);
	PrintEnum<MessageBoxReturnValue>(os);
}
#else
void PrintAllJA2Enums(std::ostream& os)
{
	throw std::runtime_error("magic_enum is not available");
}
#endif
