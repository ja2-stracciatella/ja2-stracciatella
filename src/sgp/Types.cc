#include "Types.h"

#include "Campaign_Types.h"

SGPSector::SGPSector(UINT32 s) noexcept
{
	x = SECTORX(s);
	y = SECTORY(s);
}

SGPSector::SGPSector(UINT32 s, INT8 h, const ST::string /*hack*/) noexcept
{
	x = SECTORX(s);
	y = SECTORY(s);
	z = h;
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

bool SGPSector::IsValid(ST::string shortString) const noexcept
{
	size_t len = shortString.size();
	if (len < 2 || len > 3) return false;

	char y = shortString[0], x = shortString[1];
	if (y < 'A' || y > 'P' || x < '1' || x > '9') return false;

	if (len == 3)
	{
		char x2 = shortString[2];
		if (x != '1' || x2 < '0' || x2 > '6') return false;
	}

	return true;
}

UINT8 SGPSector::AsByte() const
{
	return SECTOR(x, y);
}

UINT16 SGPSector::AsStrategicIndex() const
{
	return x + y * 18; // 18 == MAP_WORLD_X
}

ST::string SGPSector::AsShortString() const
{
	// return a string id like J11
	return ST::format("{c}{}", 'A' - 1 + y, x);
}

ST::string SGPSector::AsLongString() const
{
	// return a string id like J11_B0
	return ST::format("{c}{}_b{}", y + 'A' - 1, x, z);
}
