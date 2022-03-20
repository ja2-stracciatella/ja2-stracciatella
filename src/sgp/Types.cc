#include "Types.h"

#include "Campaign_Types.h"

SGPSector::SGPSector(UINT32 s) noexcept
{
	x = SECTORX(s);
	y = SECTORY(s);
}

bool SGPSector::operator==(const SGPSector& sector) const noexcept
{
	return x == sector.x && y == sector.y && z == sector.z;
}

bool SGPSector::operator!=(const SGPSector& sector) const noexcept
{
	return !(*this == sector);
}

bool SGPSector::IsValid() const noexcept
{
	return (x >= 1 && x <= 16) && (y >= 1 && y <= 16) && (z >= 0 && z <= 3);
}

UINT8 SGPSector::AsByte() const
{
	return SECTOR(x, y);
}

UINT16 SGPSector::AsStrategicIndex() const
{
	return x + y * MAP_WORLD_X; // same as CALCULATE_STRATEGIC_INDEX
}

ST::string AsShortString() const
{
	// return a string id like J11
	return ST::format("{c}{}", 'A' - 1 + y, x);
}

ST::string AsLongString() const
{
	// return a string id like J11_B0
	return ST::format("{c}{}_b{}", y + 'A' - 1, x, z)
}
