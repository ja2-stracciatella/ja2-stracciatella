#pragma once

/* XXX */
#include "ItemModel.h"
#include "ContentMusic.h"

#include "Facts.h"
#include "ItemSystem.h"
#include "DirFs.h"

#include <string_theory/string>

#include <map>
#include <stdint.h>
#include <vector>


class ArmyCompositionModel;
class BloodCatPlacementsModel;
class BloodCatSpawnsModel;
class CacheSectorsModel;
class CreatureLairModel;
class DealerInventory;
class DealerModel;
class FactParamsModel;
class GamePolicy;
class GarrisonGroupModel;
class IMPPolicy;
class LoadingScreenModel;
class MercProfile;
class MercProfileInfo;
class MERCListingModel;
class MineModel;
class MovementCostsModel;
class NpcActionParamsModel;
class NpcPlacementModel;
class PatrolGroupModel;
class SamSiteModel;
class SamSiteAirControlModel;
class ShippingDestinationModel;
class StrategicAIPolicy;
class StrategicMapSecretModel;
class SGPFile;
class TownModel;
class UndergroundSectorModel;
class VehicleModel;
struct AmmoTypeModel;
struct CalibreModel;
struct LoadingScreen;
struct MagazineModel;
struct RPCSmallFaceModel;
struct WeaponModel;
struct ARMY_COMPOSITION;
struct PATROL_GROUP;
struct GARRISON_GROUP;

class ContentManager : public ItemSystem
{
public:
	virtual ~ContentManager() {};

	virtual void logConfiguration() const = 0;

	/** Get map file path. */
	virtual ST::string getMapPath(const ST::string& mapName) const = 0;

	/** Get radar map resource name. */
	virtual ST::string getRadarMapResourceName(const ST::string &mapName) const = 0;

	/** Get tileset resource name. */
	virtual ST::string getTilesetResourceName(int number, const ST::string& fileName) const = 0;

	/** Get tileset db resource name. */
	virtual ST::string getTilesetDBResName() const = 0;

	/** Get all available tilecache. */
	virtual std::vector<ST::string> getAllTilecache() const = 0;

	/** Open map for reading. */
	virtual SGPFile* openMapForReading(const ST::string& mapName) const = 0;

	/* Open a game resource file for reading. */
	virtual SGPFile* openGameResForReading(const ST::string& filename) const = 0;

	/* Checks if a game resource exists. */
	virtual bool doesGameResExists(const ST::string& filename) const = 0;

	/** User private file (e.g. settings) */
	virtual DirFs* userPrivateFiles() const = 0;

	/** Temp files */
	virtual DirFs* tempFiles() const = 0;

	/** Get folder for saved games. */
	virtual ST::string getSavedGamesFolder() const = 0;

	/** Load encrypted string from game resource file. */
	virtual ST::string loadEncryptedString(const ST::string& fileName, uint32_t seek_chars, uint32_t read_chars) const = 0;

	virtual ST::string loadEncryptedString(SGPFile* File, uint32_t seek_chars, uint32_t read_chars) const = 0;

	/** Load dialogue quote from file. */
	virtual ST::string* loadDialogQuoteFromFile(const ST::string& filename, int quote_number) = 0;

	/** Get weapons with the give index. */
	virtual const WeaponModel* getWeapon(uint16_t index) = 0;
	virtual const WeaponModel* getWeaponByName(const ST::string &internalName) = 0;

	virtual const MagazineModel* getMagazineByName(const ST::string &internalName) = 0;
	virtual const MagazineModel* getMagazineByItemIndex(uint16_t itemIndex) = 0;
	virtual const std::vector<const MagazineModel*>& getMagazines() const = 0;

	virtual const CalibreModel* getCalibre(uint8_t index) = 0;
	virtual const ST::string* getCalibreName(uint8_t index) const = 0;
	virtual const ST::string* getCalibreNameForBobbyRay(uint8_t index) const = 0;

	virtual const AmmoTypeModel* getAmmoType(uint8_t index) = 0;

	virtual const ItemModel* getItem(uint16_t index) const = 0;
	virtual const std::map<uint16_t, uint16_t> getMapItemReplacements() const = 0;

	virtual const std::vector<std::vector<const WeaponModel*> > & getNormalGunChoice() const = 0;
	virtual const std::vector<std::vector<const WeaponModel*> > & getExtendedGunChoice() const = 0;
	virtual const std::vector<GARRISON_GROUP>& getGarrisonGroups() const = 0;
	virtual const std::vector<PATROL_GROUP>& getPatrolGroups() const = 0;
	virtual const std::vector<ARMY_COMPOSITION>& getArmyCompositions() const = 0;

	virtual const DealerModel* getDealer(uint8_t dealerID) const = 0;
	virtual const std::vector<const DealerModel*> getDealers() const = 0;

	virtual const DealerInventory* getDealerInventory(int dealerId) const = 0;
	virtual const DealerInventory* getBobbyRayNewInventory() const = 0;
	virtual const DealerInventory* getBobbyRayUsedInventory() const = 0;
	virtual const std::vector<const ShippingDestinationModel*>& getShippingDestinations() const = 0;
	virtual const ShippingDestinationModel* getShippingDestination(uint8_t locationId) const = 0;
	virtual const ShippingDestinationModel* getPrimaryShippingDestination() const = 0;
	virtual const ST::string* getShippingDestinationName(uint8_t index) const = 0;

	virtual const ST::string* getMusicForMode(MusicMode mode) const = 0;

	virtual const GamePolicy* getGamePolicy() const = 0;
	virtual const IMPPolicy* getIMPPolicy() const = 0;
	virtual const StrategicAIPolicy* getStrategicAIPolicy() const = 0;

	virtual const std::vector<const BloodCatPlacementsModel*> & getBloodCatPlacements() const = 0;
	virtual const std::vector<const BloodCatSpawnsModel*> & getBloodCatSpawns() const = 0;
	virtual const BloodCatSpawnsModel* getBloodCatSpawnsOfSector(uint8_t sectorId) const = 0;
	virtual const std::vector<const CreatureLairModel*>& getCreatureLairs() const = 0;
	virtual const CreatureLairModel* getCreatureLair(uint8_t lairId) const = 0;
	virtual const CreatureLairModel* getCreatureLairByMineId(uint8_t mineId) const = 0;
	virtual const MineModel* getMineForSector(uint8_t sectorX, uint8_t sectorY, uint8_t sectorZ) const = 0;
	virtual const MineModel* getMine(uint8_t mineId) const = 0;
	virtual const std::vector<const MineModel*>& getMines() const = 0;
	virtual const std::vector<const SamSiteModel*>& getSamSites() const = 0;
	virtual const int8_t findSamIDBySector(uint8_t sectorId) const = 0;
	virtual const SamSiteModel* findSamSiteBySector(uint8_t sectorId) const = 0;
	virtual const std::vector<const StrategicMapSecretModel*>& getMapSecrets() const = 0;

	/* returns the index of the controlling SAM site , or -1 if sector is not covered */
	virtual const int8_t getControllingSamSite(uint8_t sectorId) const = 0;

	virtual const TownModel* getTown(int8_t townId) const = 0;
	virtual const std::map<int8_t, const TownModel*>& getTowns() const = 0;
	virtual const ST::string getTownName(uint8_t townId) const = 0;
	virtual const ST::string getTownLocative(uint8_t townId) const = 0;
	virtual const std::vector <const UndergroundSectorModel*> & getUndergroundSectors() const = 0;
	virtual const CacheSectorsModel* getCacheSectors() const = 0;
	virtual const MovementCostsModel* getMovementCosts() const = 0;
	/* Returns land type index for special sectors. Return -1 if no special land type is defined */
	virtual int16_t getSectorLandType(uint8_t sectorID, uint8_t sectorLevel) const = 0;
	virtual const std::map<uint8_t, const NpcPlacementModel*>& listNpcPlacements() const = 0;
	virtual const NpcPlacementModel* getNpcPlacement(uint8_t profileId) const = 0;
	
	/* Params for the given NPC_ACTION if found, or return an empty instance */
	virtual const NpcActionParamsModel* getNpcActionParams(uint16_t actionCode) const = 0;

	/* Params for the given NPC_ACTION if found, or return an empty instance */
	virtual const FactParamsModel* getFactParams(Fact fact) const = 0;

	/* Returns the full list of profile listings on M.E.R.C. */
	virtual const std::vector<const MERCListingModel*>& getMERCListings() const = 0;

	//returns the full list of character profiles
	virtual const std::vector<const MercProfile*>& listMercProfiles() const = 0;

	/* Gets eyes and mouths offsets for the RPC small portraits. Returns null if none defined. */
	virtual const RPCSmallFaceModel* getRPCSmallFaceOffsets(uint8_t profileID) const = 0;

	/* Gets all vehicle types */
	virtual const VehicleModel* getVehicle(uint8_t vehicleID) const = 0;

	/* Gets loading screen for the sector. Returns NULL if the sector does not have an associated loading screen */
	virtual const LoadingScreen* getLoadingScreenForSector(uint8_t sectorId, uint8_t sectorLevel, bool isNight) const = 0;

	/* Gets a loading screen by ID. Never returns NULL, but throws out_of_range if index is invalid */
	virtual const LoadingScreen* getLoadingScreen(uint8_t index) const = 0;

	virtual const ST::string* getNewString(size_t stringId) const = 0;

	virtual const ST::string& getLandTypeString(size_t index) const = 0;

	virtual const std::vector<UINT16>* getTranslationTable() const = 0;
};
