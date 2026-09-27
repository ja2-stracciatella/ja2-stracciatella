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
	MERCPROFILESTRUCT p{};
	p.bMechanical = 60;
	p.bExplosive  = 10;

	EXPECT_TRUE(Check(p, "MECHANICAL", 50));
	EXPECT_FALSE(Check(p, "EXPLOSIVES", 50));

	p.bMechanical = 10;
	p.bExplosive  = 60;

	EXPECT_FALSE(Check(p, "MECHANICAL", 50));
	EXPECT_TRUE(Check(p, "EXPLOSIVES", 50));
}
