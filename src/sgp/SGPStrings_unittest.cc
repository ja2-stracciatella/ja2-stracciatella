#include "SGPStrings.h"

#include <gtest/gtest.h>


TEST(st_fmt_printf_to_format, translations)
{
	ASSERT_EQ(st_fmt_printf_to_format("x{x"), "x{{x");
	ASSERT_EQ(st_fmt_printf_to_format("x%%x"), "x%x");
	ASSERT_EQ(st_fmt_printf_to_format("x%cx"), "x{c}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%sx"), "x{}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%lsx"), "x{}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%hsx"), "x{}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%xx"), "x{x}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%Xx"), "x{X}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%dx"), "x{d}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%ux"), "x{d}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%03dx"), "x{03d}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%ox"), "x{o}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%ex"), "x{e}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%Ex"), "x{E}x");
	ASSERT_EQ(st_fmt_printf_to_format("x%fx"), "x{f}x");
}
