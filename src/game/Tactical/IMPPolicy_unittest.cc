#include "gtest/gtest.h"

#include "IMPPolicy.h"
#include "Soldier_Profile_Type.h"


static bool Check(const MERCPROFILESTRUCT& p, std::string attribute, uint8_t value)
{
	Condition a{ std::move(attribute) };
	Condition b{ value };
	return IMPItemCondition(a, b).Evaluate(p);
}

TEST(IMPPolicyTest, attributeConditionsReadTheRightStat)
{
	// every stat gets a different value, so reading the wrong one either
	// fails the check at its own value or passes it one above
	MERCPROFILESTRUCT p{};
	p.bLifeMax      = 99;
	p.bAgility      = 11;
	p.bDexterity    = 12;
	p.bStrength     = 13;
	p.bLeadership   = 14;
	p.bWisdom       = 15;
	p.bExpLevel     =  6;
	p.bMarksmanship = 17;
	p.bExplosive    = 18;
	p.bMechanical   = 19;
	p.bMedical      = 20;

	const std::pair<const char*, uint8_t> attributes[] = {
		{ "AGILITY",      11 },
		{ "DEXTERITY",    12 },
		{ "STRENGTH",     13 },
		{ "LEADERSHIP",   14 },
		{ "WISDOM",       15 },
		{ "EXPLEVEL",      6 },
		{ "MARKSMANSHIP", 17 },
		{ "EXPLOSIVES",   18 },
		{ "MECHANICAL",   19 },
		{ "MEDICAL",      20 },
	};
	static_assert(std::size(attributes) == NUM_ATTRIBUTES);

	for (auto const& [name, value] : attributes)
	{
		EXPECT_TRUE(Check(p, name, value)) << name;
		EXPECT_FALSE(Check(p, name, value + 1)) << name;
	}
}

TEST(IMPPolicyTest, attributeStatsDisplayOrder)
{
	MERCPROFILESTRUCT p{};
	p.bLifeMax      = 10;
	p.bAgility      = 11;
	p.bDexterity    = 12;
	p.bStrength     = 13;
	p.bLeadership   = 14;
	p.bWisdom       = 15;
	p.bExpLevel     = 16;
	p.bMarksmanship = 17;
	p.bMechanical   = 18;
	p.bExplosive    = 19;
	p.bMedical      = 20;

	// rows of the Personnel stats list, as in pPersonnelTeamStatsStrings
	for (int row = 0; row <= NUM_ATTRIBUTES; ++row)
	{
		EXPECT_EQ(Attribute(p, row, true), 10 + row) << row;
	}
}
