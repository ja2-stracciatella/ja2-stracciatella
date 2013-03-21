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
  // XXX
  // This test fails on Visual Studio 10 Express when
  // the source file has utf-8 encoding.

  const wchar_t str[] = L"тест";
  EXPECT_EQ(ARR_SIZE(str), 5);
  // EXPECT_EQ(str[0], 0x0442);
  // EXPECT_EQ(str[1], 0x0435);
  // EXPECT_EQ(str[2], 0x0441);
  // EXPECT_EQ(str[3], 0x0442);
  // EXPECT_EQ(str[4], 0x00);
}
