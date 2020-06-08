#pragma once

#include "JsonObject.h"
#include "Strategic.h"
#include "UndergroundSectorModel.h"

#include <array>
#include <rapidjson/document.h>
#include <string>
#include <vector>

struct CreatureAttackSector
{
        uint8_t sectorId;
        uint8_t chance;
        InsertionCode insertionCode;
        int16_t insertionGridNo;
};

struct CreatureLairSector
{
        uint8_t sectorId;
        uint8_t sectorLevel;
        uint8_t habitatType;
};

class CreatureLairModel
{
public:
        CreatureLairModel(const uint8_t lairId_, const uint8_t associatedMineId_, 
                const uint8_t entranceSector_, const uint8_t entranceSectorLevel_,
                const std::vector<CreatureLairSector> lairSectors_,
                const std::vector<CreatureAttackSector> attackSectors_,
                const uint8_t warpExitSector_, const uint16_t warpExitGridNo_);
        
        const uint8_t lairId;
        const uint8_t associatedMineId;

        // underground sector where the lair entrance is at
        const uint8_t entranceSector;

        // sector Z of the lair entrance
        const uint8_t entranceSectorLevel;

        // underground sectors making up the lair, the list always starts from the innermost sector, i.e. the creature queen location
        const std::vector<CreatureLairSector> lairSectors;

        // town sectors that may be attacked by creatures
        const std::vector<CreatureAttackSector> attackSectors;

        // destination sector of the "travel to surface" warp
        const uint8_t warpExitSector;

        // destination gridNo of the "travel to surface" warp
        const uint16_t warpExitGridNo;

        // returns if the given sector is part of the lair
        bool isSectorInLair(uint8_t sectorX, uint8_t sectorY, uint8_t sectorZ) const;

        // randomly choose a town sector to attack, returns the placement details of the attack
        const CreatureAttackSector* chooseTownSectorToAttack() const;

        // returns the placeent details of an attack to the specific sector
        const CreatureAttackSector* getTownAttackDetails(uint8_t sectorId) const;

        static CreatureLairModel* deserialize(const rapidjson::Value& json);
        static void validateData(const std::vector<const CreatureLairModel*>& lairs, const std::vector<const UndergroundSectorModel*>& ugSectors, uint8_t numMines);
};
