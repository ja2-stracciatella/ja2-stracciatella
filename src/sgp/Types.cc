#include "Types.h"

#include "Campaign_Types.h"

#define MAX_SECTOR 16
#define MAX_Z 3
#define MAP_WORLD_X 18 // redefine to avoid needing to include StrategicMap.h

static UINT8 getXfromSectorID(UINT32 sectorId)
{
	return sectorId % MAX_SECTOR + 1;
}

static UINT8 getYfromSectorID(UINT32 sectorId)
{
	return sectorId / MAX_SECTOR + 1;
}

SGPSector::SGPSector(UINT32 s) noexcept
{
	x = getXfromSectorID(s);
	y = getYfromSectorID(s);
}

static UINT16 getXfromStrategicIndex(UINT16 idx)
{
	return idx % MAP_WORLD_X;
}

static UINT16 getYfromStrategicIndex(UINT16 idx)
{
	return idx / MAP_WORLD_X;
}

SGPSector SGPSector::FromStrategicIndex(UINT16 idx)
{
	return SGPSector(getXfromStrategicIndex(idx), getYfromStrategicIndex(idx));
}

SGPSector SGPSector::FromShortString(const ST::string coordinates, INT8 h)
{
	Assert(coordinates.size() > 1);
	UINT8 y = coordinates[0] - (coordinates[0] >= 'a' ? 'a' : 'A') + 1;
	UINT8 x = coordinates.substr(1).to_int();
	return SGPSector(x, y, h);
}

// takes offsets, not all coordinates like the normal constructor
SGPSector SGPSector::FromSectorID(UINT16 sectorId, UINT8 dx, UINT8 dy)
{
	UINT8 x = getXfromSectorID(sectorId) + dx;
	UINT8 y = getYfromSectorID(sectorId) + dy;
	return SGPSector(x, y);
}

SGPSector SGPSector::FromSectorID(UINT32 s, INT8 h)
{
	UINT8 x = getXfromSectorID(s);
	UINT8 y = getYfromSectorID(s);
	return SGPSector(x, y, h);
}

bool SGPSector::operator==(const SGPSector& sector) const noexcept
{
	return x == sector.x && y == sector.y && z == sector.z;
}

bool SGPSector::operator!=(const SGPSector& sector) const noexcept
{
	return !(*this == sector);
}

bool SGPSector::operator<(const SGPSector& rhs) const noexcept
{
	return x < rhs.x || y < rhs.y;
}

SGPSector& SGPSector::operator+=(const SGPSector& rhs) noexcept
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

SGPSector& SGPSector::operator-=(const SGPSector& rhs) noexcept
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

bool SGPSector::IsValid() const noexcept
{
	return (x >= 1 && x <= MAX_SECTOR) && (y >= 1 && y <= MAX_SECTOR) && (z >= 0 && z <= MAX_Z);
}

// NOTE: adapt tests if MAX_SECTOR ever changes
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

// convert sector coordinates (1-16,1-16) to 0-255 sector ID
UINT8 SGPSector::AsByte() const
{
	Assert(1 <= x && x <= MAX_SECTOR && 1 <= y && y <= MAX_SECTOR);
	return (y - 1) * MAX_SECTOR + x - 1;
}

UINT16 SGPSector::AsStrategicIndex() const
{
	return x + y * MAP_WORLD_X;
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
