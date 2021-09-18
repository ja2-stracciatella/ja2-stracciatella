#pragma once

#include "Types.h"
#include "Observable.h"
#include <variant>

typedef std::map<std::string, std::variant<std::string, int32_t, float, bool>> ExtraGameStatesTable;

/*! \file FunctionsLibrary.h */

/*! \struct OBJECTTYPE
    \brief Representation of an inventory item in the game world */
struct OBJECTTYPE;
/*! \struct SECTORINFO
    \brief Status information of a sector */
struct SECTORINFO;
/*! \struct UNDERGROUND_SECTORINFO
    \brief Status information of an underground sector, similar to SECTORINFO but holds slightly different data  */
struct UNDERGROUND_SECTORINFO;
/*! \struct SOLDIERTYPE
    \brief Information and attributes of a soldier in game */
struct SOLDIERTYPE;
/*! \struct STRUCTURE
    \brief An structure element on the tactical map */
struct STRUCTURE;

/**
 * @defgroup observables Observables
 * @brief Register listeners on these observables to receive callbacks when somemthing happens in game.
 * @see RegisterListener
 */

 /**
  * Callback when a soldier is created and before the soldier is assigned to teams and placed on map.
  * @param pSoldier pointer to the soldier being created
  * @ingroup observables
  */
extern Observable<SOLDIERTYPE*> OnSoldierCreated;

/**
 * Callback after loading map and before setting it up (e.g. placing soldiers)
 * @ingroup observables
 */
extern Observable<> BeforePrepareSector;

/**
 * Callback right before the structure damange is inflicted.
 * @param sSectorX
 * @param sSectorY
 * @param bSectorZ
 * @param sGridNo location on the tactical map affected by the exploson
 * @param pStructure pointer to the structure to be damaged
 * @param uiDist no of grids from the source of explosion
 * @param fSkipDamage damage processing will be skipped if it is set to TRUE
 * @ingroup observables
 */
extern Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT32, BOOLEAN_S*> BeforeStructureDamaged;

/**
  * Callback just after a structure has just been damaged by explosives
  * @param sSectorX
  * @param sSectorY
  * @param bSectorZ
  * @param sGridNo location on map affected by the exploson
  * @param pStructure pointer to the structure being damaged by explosion
  * @param ubDamage damage amount just dealt
  * @param fIsDestroyed whether or not the structure has been destroyed
  * @ingroup observables
  */
extern Observable<INT16, INT16, INT8, INT16, STRUCTURE*, UINT8, BOOLEAN> OnStructureDamaged;

/**
 * @defgroup funclib-general General 
 * @brief Functions to compose mod modules
 */

/**
 * When the game about to be saved. This is the place to persist mod game states.
 */
extern Observable<> BeforeGameSaved;

/**
 * Right after a game is loaded. This is the place to restore game states from a saved game.
 */
extern Observable<> OnGameLoaded;

/** @defgroup funclib-sectors Map sectors
 *  @brief Access and alter sectors' stratgic-level data
 */

/**
 * Returns the ID of the current sector, the a format of A11 or B12-1(if underground)
 * @ingroup funclib-sectors
 */
std::string GetCurrentSector();

/**
 * Returns a pointer to the SECTOR_INFO of the specified sector. Does not work for underground sectors.
 * @param sectorID the short-string sector ID, e.g. "A10"
 * @throws std::runtime_error if sectorID is empty or invalid
 * @see GetUndergroundSectorInfo
 * @ingroup funclib-sectors
 */
SECTORINFO* GetSectorInfo(std::string const sectorID);

/**
 * Returns a pointer to the UNDERGROUND_SECTORINFO of the specific underground sector.
 * @param sectorID in the format of "XY-Z", e.g. A10-1
 * @see GetSectorInfo
 * @ingroup funclib-sectors
 */
UNDERGROUND_SECTORINFO* GetUndergroundSectorInfo(std::string const sectorID);

/** @defgroup funclib-items Items and objects
 *  @brief Functions to handle items, objects and inventories
 */

/**
 * Create a new object of the given item type and returns the pointer to it.
 * @param usItem the ID of the item type
 * @param ubStatus the status of the item, which should be a percentage value between 0 and 100
 * @return a pointer to the created object
 * @ingroup funclib-items
 */
OBJECTTYPE* CreateItem(const UINT16 usItem, const INT8 ubStatus);

/**
 * Creates a new money object of the specified amount
 * @param amt the amount of money
 * @return a pointer to the created object
 * @ingroup funclib-items
 */
OBJECTTYPE* CreateMoney(const UINT32 amt);

/**
 * Places the given object at the specified grid
 * @param sGridNo gridNo to a grid on the current tactical map
 * @param pObject pointer to an object created by CreateItem or CreateMoney
 * @param bVisibility determines if the object is known by the player. Must match one of the Visibility enum
 * @ingroup funclib-items
 */
void PlaceItem(const INT16 sGridNo, OBJECTTYPE* const pObject, const INT8 bVisibility);

/**
 * Retrieves a key-value mapping from saved game states.
 * @param key provide a unique key so it will not clash with other mods
 * @return
 */
ExtraGameStatesTable GetGameStates(std::string key);

/**
 * Copies the key-value mapping into the game states. Game states are
 * persisted in game saves, and can be retrieved with GetGameStates.
 * @param key provide a unique key so it will not clash with other mods
 * @param states a map of primitive types (string, numeric or boolean)
 */
void PutGameStates(std::string key, ExtraGameStatesTable states);
