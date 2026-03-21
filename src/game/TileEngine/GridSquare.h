#pragma once

#include "JA2Types.h"
#include <array>
#include <memory_resource>
#include <vector>


class GridSquare
{
	std::array<GridNo, 32 * 32> buffer;
	std::pmr::monotonic_buffer_resource mbr{ buffer.data(), sizeof(buffer) };
    std::pmr::polymorphic_allocator<GridNo> pa{ &mbr };

public:
	std::pmr::vector<GridNo> c{ pa };

	auto begin() const noexcept { return c.begin(); }
	auto end()   const noexcept { return c.end(); }

	GridSquare(GridNo center, int apothem);
};

void ClearReachableFlags(GridSquare const& square);
