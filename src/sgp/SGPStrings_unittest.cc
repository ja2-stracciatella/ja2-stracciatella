#include "SGPStrings.h"

#include <gtest/gtest.h>

TEST(CopyTrimmedString, no_trim)
{
	wchar_t dst[16];
	wchar_t const input[] = L"Don't trim this";
	wchar_t const expected[] = L"Don't trim this";

	CopyTrimmedString(dst, 16, input);
	EXPECT_EQ(wcslen(dst), wcslen(expected));
	EXPECT_EQ(wcscmp(dst, expected), 0);
}

TEST(CopyTrimmedString, trim_left)
{
	wchar_t dst[16];
	wchar_t const input[] = L"   Left Trim Täst";
	wchar_t const expected[] = L"Left Trim Täst";

	CopyTrimmedString(dst, 16, input);
	EXPECT_EQ(wcslen(dst), wcslen(expected));
	EXPECT_EQ(wcscmp(dst, expected), 0);
}

TEST(CopyTrimmedString, trim_right)
{
	wchar_t dst[16];
	wchar_t const input[] = L"trimright!!!   ";
	wchar_t const expected[] = L"trimright!!!";

	CopyTrimmedString(dst, 16, input);
	EXPECT_EQ(wcslen(dst), wcslen(expected));
	EXPECT_EQ(wcscmp(dst, expected), 0);
}

TEST(CopyTrimmedString, trim_both)
{
	wchar_t dst[16];
	wchar_t const input[] = L"   left and right ";
	wchar_t const expected[] = L"left and right";

	CopyTrimmedString(dst, 16, input);
	EXPECT_EQ(wcslen(dst), wcslen(expected));
	EXPECT_EQ(wcscmp(dst, expected), 0);
}

TEST(CopyTrimmedString, max_length)
{
	wchar_t dst[16];
	wchar_t const input[] = L"   123456789012345 ";
	wchar_t const expected[] = L"123456789012345";

	CopyTrimmedString(dst, 16, input);
	EXPECT_EQ(wcslen(dst), wcslen(expected));
	EXPECT_EQ(wcscmp(dst, expected), 0);
}
