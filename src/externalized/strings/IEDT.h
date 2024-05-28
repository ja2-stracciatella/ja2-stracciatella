#pragma once

#include <cstdint>
#ifndef _MSC_VER
#include <initializer_list>
#else
#include <vector>
#endif
#include <memory>
#include <string_theory/string>

struct IEDT
{
	virtual ST::string at(unsigned row, unsigned column = 0) const = 0;
	virtual ~IEDT() = default;

	using uptr = std::unique_ptr<IEDT const>;
#ifndef _MSC_VER
	using column_list = std::initializer_list<std::uint16_t>;
#else
	// Microsoft's STL implementation of initializer_list seems broken,
	// we have to use vector as a workaround.
	using column_list = std::vector<std::uint16_t>
#endif
};
