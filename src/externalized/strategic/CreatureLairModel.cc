#include "CreatureLairModel.h"

#include "Campaign_Types.h"
#include "Creature_Spreading.h"
#include "JsonUtility.h"
#include "Random.h"
#include <set>


CreatureLairModel::CreatureLairModel(const uint8_t lairId_, const uint8_t associatedMineId_, const uint8_t entranceSector_, const uint8_t entranceSectorLevel_, const std::vector<CreatureLairSector> lairSectors_, const std::vector<CreatureAttackSector> attackSectors_, const uint8_t warpExitSector_, const uint16_t warpExitGridNo_)
	: lairId(lairId_), associatedMineId(associatedMineId_), 
	entranceSector(entranceSector_), entranceSectorLevel(entranceSectorLevel_), 
	lairSectors(lairSectors_), attackSectors(attackSectors_),
	warpExitSector(warpExitSector_), warpExitGridNo(warpExitGridNo_) {}

uint8_t creatureHabitatFromString(std::string habitat)
{
	if (habitat == "QUEEN_LAIR") return QUEEN_LAIR;
	if (habitat == "LAIR") return LAIR;
	if (habitat == "LAIR_ENTRANCE") return LAIR_ENTRANCE;
	if (habitat == "INNER_MINE") return INNER_MINE;
	if (habitat == "OUTER_MINE") return OUTER_MINE;
	if (habitat == "FEEDING_GROUNDS") return FEEDING_GROUNDS;
	if (habitat == "MINE_EXIT") return MINE_EXIT;

	ST::string err = ST::format("Unrecognized creature habitat: '{}'", habitat);
	throw std::runtime_error(err.to_std_string());
}

InsertionCode insertionCodeFromString(std::string code)
{
	if (code == "NORTH") return INSERTION_CODE_NORTH;
	if (code == "SOUTH") return INSERTION_CODE_SOUTH;
	if (code == "EAST") return INSERTION_CODE_EAST;
	if (code == "WEST") return INSERTION_CODE_WEST;
	if (code == "GRIDNO") return INSERTION_CODE_GRIDNO;
	if (code == "ARRIVING_GAME") return INSERTION_CODE_ARRIVING_GAME;
	if (code == "CHOPPER") return INSERTION_CODE_CHOPPER;
	if (code == "PRIMARY_EDGEINDEX") return INSERTION_CODE_PRIMARY_EDGEINDEX;
	if (code == "SECONDARY_EDGEINDEX") return INSERTION_CODE_SECONDARY_EDGEINDEX;
	if (code == "CENTER") return INSERTION_CODE_CENTER;

	ST::string err = ST::format("Unrecognized insertion code: '{}'", code);
	throw std::runtime_error(err.to_std_string());
}

std::vector<CreatureLairSector> readLairSectors(const rapidjson::Value& json)
{
	std::vector<CreatureLairSector> sectors;
	for (auto& el : json.GetArray())
	{
		auto arr = el.GetArray();
		auto sectorString = arr[0].GetString();

		CreatureLairSector sec = {};
		sec.sectorId = JsonUtility::parseSectorID(sectorString);
		sec.sectorLevel = arr[1].GetUint();
		sec.habitatType = creatureHabitatFromString(arr[2].GetString());

		sectors.push_back(sec);
	}

	if (sectors.size() == 0)
	{
		ST::string err = ST::format("Lair has no lair sectors");
		throw std::runtime_error(err.to_std_string());
	}

	return sectors;
}

std::vector<CreatureAttackSector> readAttackSectors(const rapidjson::Value& json)
{
	std::vector<CreatureAttackSector> attacks;
	for (auto& el : json.GetArray())
	{
		JsonObjectReader obj(el);
		auto sectorString = obj.GetString("sector");

		CreatureAttackSector sectorAttack = {};
		sectorAttack.chance = obj.GetUInt("chance");
		sectorAttack.insertionCode = insertionCodeFromString(obj.GetString("insertionCode"));
		sectorAttack.insertionGridNo = obj.getOptionalInt("insertionGridNo");
		sectorAttack.sectorId = JsonUtility::parseSectorID(sectorString);
		attacks.push_back(sectorAttack);
	}

	if (attacks.size() == 0)
	{
		ST::string err = ST::format("Lair has no town attack sectors");
		throw std::runtime_error(err.to_std_string());
	}

	return attacks;
}

bool CreatureLairModel::isSectorInLair(uint8_t sectorX, uint8_t sectorY, uint8_t sectorZ) const
{
	uint8_t sectorId = SECTOR(sectorX, sectorY);
	for (auto sec : lairSectors)
	{
		if (sec.sectorId == sectorId && sec.sectorLevel == sectorZ)
		{
			return true;
		}
	}
	return false;
}

const CreatureAttackSector* CreatureLairModel::chooseTownSectorToAttack() const
{
	unsigned int totalChance = 0;
	for (auto sec : attackSectors)
	{
		totalChance += sec.chance;
	}

	int random = Random(totalChance);
	for (auto& sec : attackSectors)
	{
		random -= sec.chance;
		if (random < 0)
		{
			return &sec;
		}
	}

	SLOGA("Some coding error with the random selection logic. We should never reach here");
	return NULL;
}

const CreatureAttackSector* CreatureLairModel::getTownAttackDetails(uint8_t sectorId) const
{
	for (auto& sec : attackSectors)
	{
		if (sec.sectorId == sectorId) 
		{
			return &sec;
		}
	}
	return NULL;
}

CreatureLairModel* CreatureLairModel::deserialize(const rapidjson::Value& json)
{
	JsonObjectReader obj(json);

	auto entrance = json["entranceSector"].GetArray();
	auto sectorString = entrance[0].GetString();
	uint8_t entranceSector = JsonUtility::parseSectorID(sectorString);
	uint8_t entranceSectorLevel = entrance[1].GetUint();

	auto warpExit = JsonObjectReader(json["warpExit"]);
	sectorString = warpExit.GetString("sector");
	uint8_t warpExitSector = JsonUtility::parseSectorID(sectorString);
	uint16_t warpExitGridNo = warpExit.GetUInt("gridNo");

	return new CreatureLairModel(
		obj.GetUInt("lairId"),
		obj.GetUInt("associatedMineId"),
		entranceSector, entranceSectorLevel,
		readLairSectors(json["sectors"]),
		readAttackSectors(json["attackSectors"]),
		warpExitSector, warpExitGridNo
	);
}

void CreatureLairModel::validateData(const std::vector<const CreatureLairModel*>& lairs, const std::vector<const UndergroundSectorModel*>& ugSectors, const uint8_t numMines)
{
	std::set<uint8_t> distinctLairIds;
	for (auto lair : lairs)
	{
		// checkd for valid lairId, 0 is reserved
		if (lair->lairId < 1)
		{
			ST::string err = ST::format("lairID {} is invalid. Must be greater than 0");
			throw std::runtime_error(err.to_std_string());
		}

		// make sure we do not define the same lairId more than once
		if (distinctLairIds.find(lair->lairId) != distinctLairIds.end())
		{
			ST::string err = ST::format("lairID {} is already defined before");
			throw std::runtime_error(err.to_std_string());
		}
		distinctLairIds.insert(lair->lairId);

		// if the mineId is valid
		if (lair->associatedMineId > numMines)
		{
			ST::string err = ST::format("Invalid mineId {}", lair->associatedMineId);
			throw std::runtime_error(err.to_std_string());
		}
		
		// The first lair sector in list should be QUEEN_LAIR
		if (lair->lairSectors.size() < 1 || lair->lairSectors[0].habitatType != QUEEN_LAIR)
		{
			SLOGW(ST::format("The list of lair sectors should be non-empty and begin with the QUEEN_LAIR. Lair ID: {}", lair->lairId));
		}

		// all lair sectors should be adjacent and defined as underground sector
		int8_t x = 0, y = 0, z = -1;
		for (auto sec : lair->lairSectors)
		{
			if (z != -1)
			{
				int distance = abs(SECTORX(sec.sectorId) - x) + abs(SECTORY(sec.sectorId) - y) + abs(sec.sectorLevel - z);
				if (distance != 1)
				{
					SLOGW(ST::format("The current lair sector ({},{}) is not adjacent to the previous. This may indicate data issues", sec.sectorId, sec.sectorLevel));
				}
			}
			x = SECTORX(sec.sectorId);
			y = SECTORY(sec.sectorId);
			z = sec.sectorLevel;
		}

		// all underground sectors must also be defined with UndergroundSectorModel
		for (auto sec : lair->lairSectors)
		{
			if (sec.sectorLevel == 0)
			{
				SLOGW(ST::format("Lair sector ({}) is not in the underground. This may cause problems.", sec.sectorId));
				continue;
			}
			bool isDefined = false;
			for (auto ug : ugSectors)
			{
				if (ug->sectorId == sec.sectorId && ug->sectorZ == sec.sectorLevel)
				{
					isDefined = true;
					break;
				}
			}
			if (!isDefined)
			{
				ST::string err = ST::format("Underground lair sector ({},{}) is not defined as an underground sector. Make sure the data is consistent with strategic-map-underground-sectors.json.", sec.sectorId, sec.sectorLevel);
				throw std::runtime_error(err.to_std_string());
			}
		}
	}
}
