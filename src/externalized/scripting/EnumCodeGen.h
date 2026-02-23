#pragma once

#ifndef NO_MAGICENUM_LIB
#include <magic_enum/magic_enum.hpp>
#ifdef MAGIC_ENUM_SUPPORTED
#ifdef MAGIC_ENUM_SUPPORTED_ALIASES
#define HAS_ENUMGEN_SUPPORT (1)
#endif
#endif
#endif


#include <stdexcept>
#include <ostream>
#include <string_theory/format>

/**
 * Lua code generation for C enums
 * @tparam E type of a named C enum that maps to integer values
 * @param os stream to print the generated Lua code to
 * @param zTypeName optional type name. Uses the C enum name if not provided
 */
template<typename E>
void PrintEnum(std::ostream& os, const ST::string& zTypeName = {})
{
#ifdef HAS_ENUMGEN_SUPPORT
	os << (zTypeName.empty() ? magic_enum::enum_type_name<E>() : zTypeName.to_std_string())
	   << " = {" << std::endl;

	constexpr auto& entries = magic_enum::enum_entries<E>();
	for (auto& pair : entries)
	{
		E value = pair.first;
		std::string name(pair.second);
		ST::string line = ST::format("\t{} = {},", name, value);
		os << line.to_std_string() << std::endl;
	}
	os << "}" << std::endl << std::endl;
#else
	throw std::runtime_error("For this to work, the build must have magic_enum with alias support");
#endif
}

/**
 * Same as PrintEnum but using a different namespace from magic_enum which specializes in enum-flags
 * @tparam E
 * @param os
 * @param zTypeName
 */
template<typename E>
void PrintEnumFlags(std::ostream& os, const ST::string& zTypeName = {})
{
#ifdef MAGIC_ENUM_SUPPORTED
#ifndef MAGIC_ENUM_SUPPORTED_ALIASES
	throw std::runtime_error("enum alias support is required")
#endif
	os << (zTypeName.empty() ? magic_enum::enum_type_name<E>() : zTypeName.to_std_string())
	   << " = {" << std::endl;

	constexpr auto& entries = magic_enum::enum_entries<E>();
	for (auto& pair : entries)
	{
		E value = pair.first;
		std::string name(pair.second);
		ST::string line = ST::format("\t{} = {},", name, value);
		os << line.to_std_string() << std::endl;
	}
	os << "}" << std::endl << std::endl;
#else
	throw std::runtime_error("magic_enum is not available");
#endif
}

/**
 * Prints a enum-class but using the underlying values.
 */
template<typename E, typename V>
void PrintEnumClass(std::ostream& os, const ST::string& zTypeName = {})
{
#ifdef HAS_ENUMGEN_SUPPORT
	os << (zTypeName.empty() ? magic_enum::enum_type_name<E>() : zTypeName.to_std_string())
	   << " = {" << std::endl;

	constexpr auto& entries = magic_enum::enum_entries<E>();
	for (auto& pair : entries)
	{
		E value = pair.first;
		std::string name(pair.second);
		ST::string line = ST::format("\t{} = {},", name, (V)value);
		os << line.to_std_string() << std::endl;
	}
	os << "}" << std::endl << std::endl;
#else
	throw std::runtime_error("For this to work, the build must have magic_enum with alias support");
#endif
}

void PrintAllJA2Enums(std::ostream& os);
