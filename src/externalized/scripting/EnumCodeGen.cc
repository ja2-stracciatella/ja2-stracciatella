#include "EnumCodeGen.h"

#ifdef MAGIC_ENUM_SUPPORTED
#include "Animation_Data.h"
#include "Game_Event_Hook.h"
#include "Soldier_Profile.h"
#include "Soldier_Control.h"
#include "Handle_Items.h"
#include <iostream>


static void PrintHeader(std::ostream& os)
{
	os << "--[[" << std::endl
	   << " This file was auto-generated. Do not manually edit." << std::endl
	   << "--]]" << std::endl << std::endl;
}

void PrintAllJA2Enums(std::ostream& os)
{
	PrintHeader(os);

	PrintEnum<NPCIDs>(os);
	PrintEnum<SoldierClass>(os);
	PrintEnum<SoldierBodyType>(os);
	PrintEnum<StrategicEventKind>(os);
	PrintEnum<Visibility>(os);
}
#else
void PrintAllJA2Enums(std::ostream& os)
{
	throw std::runtime_error("magic_enum is not available");
}
#endif
