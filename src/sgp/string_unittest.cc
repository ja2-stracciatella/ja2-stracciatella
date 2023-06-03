#include "gtest/gtest.h"

#include <string_theory/string>


TEST(stringTest, English)
{
	const ST::string str = "test";
	EXPECT_EQ(str.size(), 4u);
	EXPECT_EQ(str[0], '\x74');
	EXPECT_EQ(str[1], '\x65');
	EXPECT_EQ(str[2], '\x73');
	EXPECT_EQ(str[3], '\x74');
	EXPECT_EQ(str[4], '\0');
}

TEST(stringTest, RussianWithUnicodeConsts)
{
	const ST::string str = u8"\u0442\u0435\u0441\u0442";
	EXPECT_EQ(str.size(), 8u);
	EXPECT_EQ(str[0], '\xD1');
	EXPECT_EQ(str[1], '\x82');
	EXPECT_EQ(str[2], '\xD0');
	EXPECT_EQ(str[3], '\xB5');
	EXPECT_EQ(str[4], '\xD1');
	EXPECT_EQ(str[5], '\x81');
	EXPECT_EQ(str[6], '\xD1');
	EXPECT_EQ(str[7], '\x82');
	EXPECT_EQ(str[8], '\0');
}

TEST(stringTest, Russian)
{
	const ST::string str = "тест";
	EXPECT_EQ(str.size(), 8u);
	EXPECT_EQ(str[0], '\xD1');
	EXPECT_EQ(str[1], '\x82');
	EXPECT_EQ(str[2], '\xD0');
	EXPECT_EQ(str[3], '\xB5');
	EXPECT_EQ(str[4], '\xD1');
	EXPECT_EQ(str[5], '\x81');
	EXPECT_EQ(str[6], '\xD1');
	EXPECT_EQ(str[7], '\x82');
	EXPECT_EQ(str[8], '\0');
}

TEST(stringTest, Chinese)
{
	const ST::string str = "测试";
	ASSERT_EQ(str.size(), 6u);
	EXPECT_EQ(str[0], '\xE6');
	EXPECT_EQ(str[1], '\xB5');
	EXPECT_EQ(str[2], '\x8B');
	EXPECT_EQ(str[3], '\xE8');
	EXPECT_EQ(str[4], '\xAF');
	EXPECT_EQ(str[5], '\x95');
	EXPECT_EQ(str[6], '\0');
}

TEST(stringTest, Greek)
{
	const ST::string str = "δοκιμή";
	ASSERT_EQ(str.size(), 12u);
	EXPECT_EQ(str[0], '\xCE');
	EXPECT_EQ(str[1], '\xB4');
	EXPECT_EQ(str[2], '\xCE');
	EXPECT_EQ(str[3], '\xBF');
	EXPECT_EQ(str[4], '\xCE');
	EXPECT_EQ(str[5], '\xBA');
	EXPECT_EQ(str[6], '\xCE');
	EXPECT_EQ(str[7], '\xB9');
	EXPECT_EQ(str[8], '\xCE');
	EXPECT_EQ(str[9], '\xBC');
	EXPECT_EQ(str[10], '\xCE');
	EXPECT_EQ(str[11], '\xAE');
	EXPECT_EQ(str[12], '\0');
}
