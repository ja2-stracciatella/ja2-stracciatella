#include "gtest/gtest.h"

#define ARR_SIZE(x) (sizeof(x)/sizeof(x[0]))

TEST(wcharTest, English)
{
	const wchar_t str[] = L"test";
	EXPECT_EQ(ARR_SIZE(str), 5);
	EXPECT_EQ(str[0], 0x74);
	EXPECT_EQ(str[1], 0x65);
	EXPECT_EQ(str[2], 0x73);
	EXPECT_EQ(str[3], 0x74);
	EXPECT_EQ(str[4], 0x00);
}

TEST(wcharTest, RussianWithUnicodeConsts)
{
	const wchar_t str[] = L"\u0442\u0435\u0441\u0442";
	EXPECT_EQ(ARR_SIZE(str), 5);
	EXPECT_EQ(str[0], 0x0442);
	EXPECT_EQ(str[1], 0x0435);
	EXPECT_EQ(str[2], 0x0441);
	EXPECT_EQ(str[3], 0x0442);
	EXPECT_EQ(str[4], 0x00);
}

TEST(wcharTest, Russian)
{
	// This test will fail on Visual Studio 10 Express, if the file doesn't
	// have byte-order mark.

	const wchar_t str[] = L"тест";
	ASSERT_EQ(ARR_SIZE(str), 5) << "Russian string check fails.  If Visual Studio, make sure the source file has the byte-order mark";
	EXPECT_EQ(str[0], 0x0442);
	EXPECT_EQ(str[1], 0x0435);
	EXPECT_EQ(str[2], 0x0441);
	EXPECT_EQ(str[3], 0x0442);
	EXPECT_EQ(str[4], 0x00);
}

TEST(wcharTest, Chinese)
{
	// This test will fail on Visual Studio 10 Express, if the file doesn't
	// have byte-order mark.

	const wchar_t str[] = L"测试";
	ASSERT_EQ(ARR_SIZE(str), 3);
	EXPECT_EQ(str[0], 0x6d4b);
	EXPECT_EQ(str[1], 0x8bd5);
	EXPECT_EQ(str[2], 0x00);
}

TEST(wcharTest, Greek)
{
	// This test will fail on Visual Studio 10 Express, if the file doesn't
	// have byte-order mark.

	const wchar_t str[] = L"δοκιμή";
	ASSERT_EQ(ARR_SIZE(str), 7);
	EXPECT_EQ(str[0], 0x03b4);
	EXPECT_EQ(str[1], 0x03bf);
	EXPECT_EQ(str[2], 0x03ba);
	EXPECT_EQ(str[3], 0x03b9);
	EXPECT_EQ(str[4], 0x03bc);
	EXPECT_EQ(str[5], 0x03ae);
	EXPECT_EQ(str[6], 0x00);
}
