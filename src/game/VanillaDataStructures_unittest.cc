#include "gtest/gtest.h"

#include "VanillaDataStructures.h"

TEST(VanillaDataStructuresTest, structSizes)
{
	EXPECT_EQ(sizeof(VDS::SAVED_GAME_HEADER), 432u);
}
