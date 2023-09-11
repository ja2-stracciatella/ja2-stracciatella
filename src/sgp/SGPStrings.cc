#include "SGPStrings.h"

#include <string_theory/format>
#include <string_theory/string>

ST::string st_fmt_printf_to_format(const ST::string& fmt_printf)
{
	ST::utf32_buffer codepoints = fmt_printf.to_utf32();
	ST::string fmt;
	ST::string err;
	ST::string param;
	enum class Parse {
		Literal,
		ParamFlags,
		ParamWidth,
		ParamPrecision,
		ParamPrecisionValue,
		ParamLengthModifier,
		ParamSpecifier,
	} state = Parse::Literal;
	for (char32_t c : codepoints)
	{
		if (state == Parse::Literal)
		{
			if (c == U'%')
			{
				param = "{"; // param start
				state = Parse::ParamFlags;
				continue;
			}
			if (c == U'{')
			{
				fmt += "{{"; // literal '{'
				continue;
			}
			fmt += c; // literal
			continue;
		}
		if (state == Parse::ParamFlags)
		{
			if (c == U'-')
			{
				param += '<';
				continue;
			}
			if (c == U'+' || c == U'0' || c == U'#')
			{
				param += c;
				continue;
			}
			if (c == U' ')
			{
				err = ST::format("param flag U+{04X} '{c}' is not supported", c, c);
				break;
			}
			state = Parse::ParamWidth; // with the same codepoint
		}
		if (state == Parse::ParamWidth)
		{
			if (c >= U'0' && c <= U'9')
			{
				param += c;
				continue;
			}
			if (c == U'*')
			{
				err = ST::format("param width U+{04X} '{c}' is not supported", c, c);
				break;
			}
			state = Parse::ParamPrecision; // with the same codepoint
		}
		if (state == Parse::ParamPrecision)
		{
			if (c == U'.')
			{
				param += c;
				state = Parse::ParamPrecisionValue;
				continue;
			}
			state = Parse::ParamLengthModifier; // with the same codepoint
		}
		if (state == Parse::ParamPrecisionValue)
		{
			if (c >= U'0' && c <= U'9')
			{
				param += c;
				continue;
			}
			state = Parse::ParamLengthModifier; // with the same codepoint
		}
		if (state == Parse::ParamLengthModifier)
		{
			if (c == U'h' || c == U'l' || c == U'j' || c == U'z' || c == U't' || c == U'L')
			{
				// ignore
				continue;
			}
			state = Parse::ParamSpecifier; // with the same codepoint
		}
		if (state == Parse::ParamSpecifier)
		{
			if (c == U'%' && param == "{")
			{
				fmt += c; // literal '%'
				state = Parse::Literal;
				continue;
			}
			if (c == U'c' || c == U'o' || c == U'x' || c == U'X' || c == U'e' || c == U'E')
			{
				param += c;
				param += '}';
				fmt += param;
				state = Parse::Literal;
				continue;
			}
			if (c == U'f' || c == U'F')
			{
				param += "f}";
				fmt += param;
				state = Parse::Literal;
				continue;
			}
			if (c == U'd' || c == U'i' || c == U'u')
			{
				param += "d}";
				fmt += param;
				state = Parse::Literal;
				continue;
			}
			if (c == U's')
			{
				param += "}";
				fmt += param;
				state = Parse::Literal;
				continue;
			}
			if (c == U'a' || c == U'A' || c == U'g' || c == U'G' || c == U'n' || c == U'p')
			{
				err = ST::format("param specifier U+{04X} '{c}' is not supported", c, c);
				break;
			}
		}
		err = ST::format("unexpected codepoint U+{04X} '{c}'", c, c);
		break;
	}
	if (err.empty() && state != Parse::Literal)
	{
		err = ST::format("format param is incomplete '{}'", param);
	}
	if (!err.empty())
	{
		ST::string what = ST::format("{}: '{}' -> '{}'", err, fmt_printf, fmt);
		throw ST::bad_format(what.c_str());
	}
	return fmt;
}


ST::string st_buffer_escape(const ST::char_buffer& buf)
{
	ST::string escaped;
	for (char c : buf)
	{
		escaped += ST::format("\\x{02X}", c);
	}
	return escaped;
}


ST::string st_buffer_escape(const ST::utf16_buffer& buf)
{
	ST::string escaped;
	for (char16_t c : buf)
	{
		escaped += ST::format("\\u{04X}", c);
	}
	return escaped;
}


ST::string st_buffer_escape(const ST::utf32_buffer& buf)
{
	ST::string escaped;
	for (char32_t c : buf)
	{
		escaped += ST::format("\\U{08X}", c);
	}
	return escaped;
}
