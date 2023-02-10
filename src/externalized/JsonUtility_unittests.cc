#include "gtest/gtest.h"

#include "JsonUtility.h"

TEST(JsonUtilityTest, parseListOfStrings)
{
	{
		std::vector<ST::string> strings;
		ASSERT_TRUE(JsonUtility::parseJsonToListStrings("[]", strings));
		ASSERT_EQ(strings.size(), 0u);
	}

	{
		std::vector<ST::string> strings;
		EXPECT_THROW(JsonUtility::parseJsonToListStrings("foo", strings), std::runtime_error);
		ASSERT_EQ(strings.size(), 0u);
	}

	{
		std::vector<ST::string> strings;
		ASSERT_TRUE(JsonUtility::parseJsonToListStrings("[\"foo\"]", strings));
		ASSERT_EQ(strings.size(), 1u);
		ASSERT_STREQ(strings[0].c_str(), "foo");
	}

	{
		std::vector<ST::string> strings;
		ASSERT_TRUE(JsonUtility::parseJsonToListStrings("[\"foo\", \"bar\"]", strings));
		ASSERT_EQ(strings.size(), 2u);
		ASSERT_STREQ(strings[0].c_str(), "foo");
		ASSERT_STREQ(strings[1].c_str(), "bar");
	}
}
