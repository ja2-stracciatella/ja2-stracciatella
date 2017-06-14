#include "gtest/gtest.h"

#include "JsonUtility.h"

TEST(JsonUtilityTest, parseListOfStrings)
{
	{
		std::vector<std::string> strings;
		ASSERT_TRUE(JsonUtility::parseJsonToListStrings("[]", strings));
		ASSERT_EQ(strings.size(), 0);
	}

	{
		std::vector<std::string> strings;
		ASSERT_FALSE(JsonUtility::parseJsonToListStrings("foo", strings));
		ASSERT_EQ(strings.size(), 0);
	}

	{
		std::vector<std::string> strings;
		ASSERT_TRUE(JsonUtility::parseJsonToListStrings("[\"foo\"]", strings));
		ASSERT_EQ(strings.size(), 1);
		ASSERT_STREQ(strings[0].c_str(), "foo");
	}

	{
		std::vector<std::string> strings;
		ASSERT_TRUE(JsonUtility::parseJsonToListStrings("[\"foo\", \"bar\"]", strings));
		ASSERT_EQ(strings.size(), 2);
		ASSERT_STREQ(strings[0].c_str(), "foo");
		ASSERT_STREQ(strings[1].c_str(), "bar");
	}
}
