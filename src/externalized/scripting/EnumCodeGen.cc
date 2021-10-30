#include "EnumCodeGen.h"

#ifdef MAGIC_ENUM_SUPPORTED
#include "Animation_Data.h"
#include "Campaign_Types.h"
#include "Game_Event_Hook.h"
#include "GameSettings.h"
#include "Handle_Items.h"
#include "Item_Types.h"
#include "Overhead_Types.h"
#include "Soldier_Profile.h"
#include "Soldier_Control.h"
#include <iostream>


namespace magic_enum::customize
{
	template<> struct enum_range<ITEMDEFINE>	{ static constexpr int min = 0;	static constexpr int max = 400; };
	template<> struct enum_range<SectorIndex>	{ static constexpr int min = 0;	static constexpr int max = 256; };
	template<> struct enum_range<NPCIDs>	{ static constexpr int min = 0;	static constexpr int max = 200; };
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

	PrintEnum<ITEMDEFINE>(os);
	PrintEnum<Visibility>(os);
	PrintEnum<SkillTrait>(os);
	PrintEnum<NPCIDs>(os);
}
#else
void PrintAllJA2Enums(std::ostream& os)
{
	throw std::runtime_error("magic_enum is not available");
}
#endif
