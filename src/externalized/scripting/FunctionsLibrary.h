#pragma once

#include "Types.h"

struct OBJECTTYPE;
struct SECTORINFO;
struct UNDERGROUND_SECTORINFO;

/**
 * @defgroup cpp C++ functions
 * These are functions you can use from Lua to access or alter game states
 */

/** @name General */
///@{
/**
 * Loads the specified script file into Lua space. The file is loaded via the VFS sub-system.
 * This function can only be used during initialization.
 * @param scriptFileName the name to the lua script file, e.g. enums.lua
 * @ingroup cpp
 */
void JA2Require(std::string scriptFileName);
///@}

/** @name Map sectors (tactical) */
///@{
/**
 * Returns the ID of the current sector, the a format of A11 or B12-1(if underground)
 * @ingroup cpp
 */
std::string GetCurrentSector();

/**
 * Returns a pointer to the SECTOR_INFO of the specified sector. Does not work for underground sectors.
 * @param sectorID the short-string sector ID, e.g. "A10"
 * @throws std::runtime_error if sectorID is empty or invalid
 * @see GetUndergroundSectorInfo
 * @ingroup cpp
 */
SECTORINFO* GetSectorInfo(std::string const sectorID);

/**
 * Returns a pointer to the UNDERGROUND_SECTORINFO of the specific underground sector.
 * @param sectorID in the format of "XY-Z", e.g. A10-1
 * @see GetSectorInfo
 * @ingroup cpp
 */
UNDERGROUND_SECTORINFO* GetUndergroundSectorInfo(std::string const sectorID);
///@}

/** @name Items */
///@{
/**
 * Create a new object of the given item type and returns the pointer to it.
 * @param usItem the ID of the item type
 * @param ubStatus the status of the item, which should be a percentage value between 0 and 100
 * @return a pointer to the created object
 * @ingroup cpp
 */
OBJECTTYPE* CreateItem(const UINT16 usItem, const INT8 ubStatus);

/**
 * Creates a new money object of the specified amount
 * @param amt the amount of money
 * @return a pointer to the created object
 * @ingroup cpp
 */
OBJECTTYPE* CreateMoney(const UINT32 amt);

/**
 * Places the given object at the specified grid
 * @param sGridNo gridNo to a grid on the current tactical map
 * @param pObject pointer to an object created by CreateItem or CreateMoney
 * @param bVisibility determines if the object is known by the player. Must match one of the Visibility enum
 * @ingroup cpp
 */
void PlaceItem(const INT16 sGridNo, OBJECTTYPE* const pObject, const INT8 bVisibility);
///@}