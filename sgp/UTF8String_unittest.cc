#include "gtest/gtest.h"

#include "UTF8String.h"

#define ARR_SIZE(x) (sizeof(x)/sizeof(x[0]))

// English word "test"
static uint8_t   u8a_testEnglish[] = {0x74, 0x65, 0x73, 0x74, 0};
static uint16_t u16a_testEnglish[] = {0x0074, 0x0065, 0x0073, 0x0074, 0};
static uint32_t u32a_testEnglish[] = {0x00000074, 0x00000065, 0x00000073, 0x00000074, 0};
static std::vector<uint16_t>u16v_testEnglish(u16a_testEnglish, u16a_testEnglish + ARR_SIZE(u16a_testEnglish) - 1);
static std::vector<uint32_t>u32v_testEnglish(u32a_testEnglish, u32a_testEnglish + ARR_SIZE(u32a_testEnglish) - 1);

// Russian word "test" (тест)
static uint8_t   u8a_testRussian[] = {0xd1, 0x82, 0xd0, 0xb5, 0xd1, 0x81, 0xd1, 0x82, 0};
static uint16_t u16a_testRussian[] = {0x0442, 0x0435, 0x0441, 0x0442, 0};
static uint32_t u32a_testRussian[] = {0x00000442, 0x00000435, 0x00000441, 0x00000442, 0};
static std::vector<uint16_t>u16v_testRussian(u16a_testRussian, u16a_testRussian + ARR_SIZE(u16a_testRussian) - 1);
static std::vector<uint32_t>u32v_testRussian(u32a_testRussian, u32a_testRussian + ARR_SIZE(u32a_testRussian) - 1);

// Chinese (simplified) word "test" (测试)
static uint8_t   u8a_testChinese[] = {0xe6, 0xb5, 0x8b, 0xe8, 0xaf, 0x95, 0};
static uint16_t u16a_testChinese[] = {0x6d4b, 0x8bd5, 0};
static uint32_t u32a_testChinese[] = {0x00006d4b, 0x00008bd5, 0};
static std::vector<uint16_t>u16v_testChinese(u16a_testChinese, u16a_testChinese + ARR_SIZE(u16a_testChinese) - 1);
static std::vector<uint32_t>u32v_testChinese(u32a_testChinese, u32a_testChinese + ARR_SIZE(u32a_testChinese) - 1);

// Greek word "test" (δοκιμή)
static uint8_t   u8a_testGreek[] = {0xce, 0xb4, 0xce, 0xbf, 0xce, 0xba, 0xce, 0xb9, 0xce, 0xbc, 0xce, 0xae, 0};
static uint16_t u16a_testGreek[] = {0x03b4, 0x03bf, 0x03ba, 0x03b9, 0x03bc, 0x03ae, 0};
static uint32_t u32a_testGreek[] = {0x000003b4, 0x000003bf, 0x000003ba, 0x000003b9, 0x000003bc, 0x000003ae, 0};
static std::vector<uint16_t>u16v_testGreek(u16a_testGreek, u16a_testGreek + ARR_SIZE(u16a_testGreek) - 1);
static std::vector<uint32_t>u32v_testGreek(u32a_testGreek, u32a_testGreek + ARR_SIZE(u32a_testGreek) - 1);


TEST(UTF8StringTest, Basics)
{
  UTF8String empty("");
  EXPECT_EQ(empty.getNumCharacters(), 0);
  EXPECT_EQ(empty.getNumBytes(), 0);
#ifdef WCHAR_SUPPORT
  EXPECT_STREQ(empty.getWCHAR(), L"");
#endif

  UTF8String foo("foo");
  EXPECT_EQ(foo.getNumCharacters(), 3);
  EXPECT_EQ(foo.getNumBytes(), 3);
#ifdef WCHAR_SUPPORT
  EXPECT_STREQ(foo.getWCHAR(), L"foo");
#endif

  UTF8String testEnglish1( u8a_testEnglish);
  UTF8String testEnglish2(u16a_testEnglish);
  UTF8String testEnglish3(u32a_testEnglish);
  EXPECT_EQ(testEnglish1.getNumCharacters(), 4);
  EXPECT_EQ(testEnglish1.getNumBytes(),      4);
  EXPECT_EQ(testEnglish2.getNumCharacters(), 4);
  EXPECT_EQ(testEnglish2.getNumBytes(),      4);
  EXPECT_EQ(testEnglish3.getNumCharacters(), 4);
  EXPECT_EQ(testEnglish3.getNumBytes(),      4);
  EXPECT_EQ(testEnglish1.getUTF16(), u16v_testEnglish);
  EXPECT_EQ(testEnglish1.getUTF32(), u32v_testEnglish);
#ifdef WCHAR_SUPPORT
  EXPECT_STREQ(testEnglish1.getWCHAR(), L"test");
  EXPECT_STREQ(testEnglish2.getWCHAR(), L"test");
  EXPECT_STREQ(testEnglish3.getWCHAR(), L"test");
#endif
}


TEST(UTF8StringTest, Russian)
{
  UTF8String testRussian1( u8a_testRussian);
  UTF8String testRussian2(u16a_testRussian);
  UTF8String testRussian3(u32a_testRussian);
  EXPECT_EQ(testRussian1.getNumCharacters(), 4);
  EXPECT_EQ(testRussian1.getNumBytes(),      8);
  EXPECT_EQ(testRussian2.getNumCharacters(), 4);
  EXPECT_EQ(testRussian2.getNumBytes(),      8);
  EXPECT_EQ(testRussian3.getNumCharacters(), 4);
  EXPECT_EQ(testRussian3.getNumBytes(),      8);
  EXPECT_EQ(testRussian1.getUTF16(), u16v_testRussian);
  EXPECT_EQ(testRussian1.getUTF32(), u32v_testRussian);
#ifdef WCHAR_SUPPORT
  EXPECT_STREQ(testRussian1.getWCHAR(), L"тест");
  EXPECT_STREQ(testRussian2.getWCHAR(), L"тест");
  EXPECT_STREQ(testRussian3.getWCHAR(), L"тест");
#endif
}


TEST(UTF8StringTest, Chinese)
{
  UTF8String testChinese1( u8a_testChinese);
  UTF8String testChinese2(u16a_testChinese);
  UTF8String testChinese3(u32a_testChinese);
  EXPECT_EQ(testChinese1.getNumCharacters(), 2);
  EXPECT_EQ(testChinese1.getNumBytes(),      6);
  EXPECT_EQ(testChinese2.getNumCharacters(), 2);
  EXPECT_EQ(testChinese2.getNumBytes(),      6);
  EXPECT_EQ(testChinese3.getNumCharacters(), 2);
  EXPECT_EQ(testChinese3.getNumBytes(),      6);
  EXPECT_EQ(testChinese1.getUTF16(), u16v_testChinese);
  EXPECT_EQ(testChinese1.getUTF32(), u32v_testChinese);
#ifdef WCHAR_SUPPORT
  EXPECT_STREQ(testChinese1.getWCHAR(), L"测试");
  EXPECT_STREQ(testChinese2.getWCHAR(), L"测试");
  EXPECT_STREQ(testChinese3.getWCHAR(), L"测试");
#endif
}


TEST(UTF8StringTest, Greek)
{
  UTF8String testGreek1( u8a_testGreek);
  UTF8String testGreek2(u16a_testGreek);
  UTF8String testGreek3(u32a_testGreek);
  EXPECT_EQ(testGreek1.getNumCharacters(), 6);
  EXPECT_EQ(testGreek1.getNumBytes(),      12);
  EXPECT_EQ(testGreek2.getNumCharacters(), 6);
  EXPECT_EQ(testGreek2.getNumBytes(),      12);
  EXPECT_EQ(testGreek3.getNumCharacters(), 6);
  EXPECT_EQ(testGreek3.getNumBytes(),      12);
  EXPECT_EQ(testGreek1.getUTF16(), u16v_testGreek);
  EXPECT_EQ(testGreek1.getUTF32(), u32v_testGreek);
#ifdef WCHAR_SUPPORT
  EXPECT_STREQ(testGreek1.getWCHAR(), L"δοκιμή");
  EXPECT_STREQ(testGreek2.getWCHAR(), L"δοκιμή");
  EXPECT_STREQ(testGreek3.getWCHAR(), L"δοκιμή");
#endif
}


TEST(UTF8StringTest, GarbageInExceptionOut)
{
  const uint8_t  garbage8 [] = {0x3e, 0x3e, 0x59, 0xfb, 0x2d, 0x68, 0xf7, 0xfb,  0x77, 0x15, 0x1c, 0x22, 0xc9, 0xc6, 0x77, 0xb8};
  const uint16_t garbage16[] = {0xDCCC, 0x59fb, 0x2d68, 0xf7fb,  0x7715, 0x1c22, 0xc9c6, 0x77b8};
  const uint32_t garbage32[] = {0xDCCC59fb, 0x2d68f7fb,  0x77151c22, 0xc9c677b8};
  EXPECT_THROW({UTF8String test8 (garbage8 );}, InvalidEncodingException);
  EXPECT_THROW({UTF8String test16(garbage16);}, InvalidEncodingException);
  EXPECT_THROW({UTF8String test32(garbage32);}, InvalidEncodingException);
}
