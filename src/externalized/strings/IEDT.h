#pragma once

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string_theory/string>

struct IEDT
{
	virtual ST::string at(unsigned row, unsigned column = 0) const = 0;
	virtual ~IEDT() = default;

	using uptr = std::unique_ptr<IEDT const>;
	using column_list = std::initializer_list<std::uint16_t>;
};
