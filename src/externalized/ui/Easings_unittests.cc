#include "gtest/gtest.h"

#include "Easings.h"

TEST(EasingsTest, easeInCubic)
{
	ASSERT_EQ(EaseInCubic(100, 101, 100), 0.0);
	ASSERT_EQ(EaseInCubic(1000, 1100, 1100), 1.0);

	ASSERT_EQ(EaseInCubic(1000, 1100, 1025), 0.0625);
	ASSERT_EQ(EaseInCubic(1000, 1100, 1075), 0.9375);
}
