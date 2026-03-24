#pragma once

#include "JA2Types.h"
#include <array>
#include <memory_resource>
#include <vector>

/*
 * Support class to iterate over the gridnos in a square around a given
 * center gridno. This class handles the cases where the square crosses
 * any of the map edges itself; this means all gridnos are always valid
 * and do not need to be checked by the users of this class.
 */

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

// Clear the MAPELEMENT_REACHABLE flags for all gridnos of the square.
void ClearReachableFlags(GridSquare const& square);
