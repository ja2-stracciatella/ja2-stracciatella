#ifndef SGPSTRINGS_H
#define SGPSTRINGS_H

#include <string_theory/format>
#include <string_theory/string>

#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <utility>

#include "PlatformStrings.h"

// poison deprecated functions
#if !defined(__MINGW32__) && defined(__GNUC__)
size_t strlcpy(char *dst, const char *src, size_t siz) __attribute__ ((deprecated("Don't use strlcpy, but regular ST::string assignment.")));
int snprintf(char* const s, size_t const n, const char* const fmt, ...) __attribute__ ((deprecated("Don't use snprintf, use ST::format instead.")));
#endif

#ifdef _WIN32
#ifndef __MINGW32__

#include <stdarg.h>


int WINsnprintf(char* s, size_t n, const char* fmt, ...);

#define snprintf  WINsnprintf

#endif
#endif

/// Converts `std::printf` formatting to `ST::format` formatting.
/// @see https://en.cppreference.com/w/cpp/io/c/fprintf
ST::string st_fmt_printf_to_format(const ST::string& fmt_printf);

/// Format a string with `std::printf` formatting.
template <typename ... T>
inline ST::string st_format_printf(ST::utf_validation_t validation, const ST::string& fmt_printf, T&& ... args)
{
	ST::string fmt = st_fmt_printf_to_format(fmt_printf);
	return ST::format(validation, fmt.c_str(), std::forward<T>(args) ...);
}

/// Format a string with `std::printf` formatting.
template <typename ... T>
inline ST::string st_format_printf(const ST::string& fmt_printf, T&& ... args)
{
	return st_format_printf(ST_DEFAULT_VALIDATION, fmt_printf, std::forward<T>(args) ...);
}

ST::string st_buffer_escape(const ST::char_buffer& buf);
ST::string st_buffer_escape(const ST::utf16_buffer& buf);
ST::string st_buffer_escape(const ST::utf32_buffer& buf);

/// Converts a buffer to a string.
template<typename T>
ST::string st_checked_buffer_to_string(ST::string& err_msg, const ST::buffer<T>& buf)
{
	err_msg = ST::null;
	try
	{
		return buf.c_str();
	}
	catch (const std::runtime_error& ex)
	{
		ST::string str = ST::string(buf.c_str(), ST_AUTO_SIZE, ST::substitute_invalid);
		err_msg = ST::format(ST::substitute_invalid, "{}: \"{}\" -> '{}'", ex.what(), st_buffer_escape(buf), str);
		return str;
	}
}

#endif
