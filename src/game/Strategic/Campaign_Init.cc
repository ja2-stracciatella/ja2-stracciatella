#include "Campaign_Init.h"

#include "Campaign_Types.h"
#include "ContentManager.h"
#include "Creature_Spreading.h"
#include "GameInstance.h"
#include "GameSettings.h"
#include "MemMan.h"
#include "Overhead.h"
#include "Queen_Command.h"
#include "Quests.h"
#include "Random.h"
#include "Strategic_AI.h"
#include "Strategic_Mines.h"
#include "Strategic_Movement.h"
#include "Strategic_Movement_Costs.h"
#include "Strategic_Status.h"
#include "Tactical_Save.h"
#include "Types.h"
#include "UndergroundSectorModel.h"

#include <algorithm>

UNDERGROUND_SECTORINFO* gpUndergroundSectorInfoTail = NULL;


static UNDERGROUND_SECTORINFO* AddUndergroundNode(const UndergroundSectorModel* ugInfo, UINT8 ubDifLevel)
{
	UNDERGROUND_SECTORINFO* const u = ugInfo->createUndergroundSectorInfo(ubDifLevel);
	UNDERGROUND_SECTORINFO*& tail = gpUndergroundSectorInfoTail;
	*(tail ? &tail->next : &gpUndergroundSectorInfoHead) = u;
	tail = u;

	return u;
}


// setup which know facilities are in which cities
static void InitKnowFacilitiesFlags(void)
{
	SECTORINFO *pSector;

	// Cambria hospital
	pSector = &SectorInfo[SEC_G8];
	pSector->uiFacilitiesFlags |= SFCF_HOSPITAL;
	pSector = &SectorInfo[SEC_F8];
	pSector->uiFacilitiesFlags |= SFCF_HOSPITAL;
	pSector = &SectorInfo[SEC_G9];
	pSector->uiFacilitiesFlags |= SFCF_HOSPITAL;
	pSector = &SectorInfo[SEC_F9];
	pSector->uiFacilitiesFlags |= SFCF_HOSPITAL;

	// Drassen airport
	pSector = &SectorInfo[SEC_B13];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;
	pSector = &SectorInfo[SEC_C13];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;
	pSector = &SectorInfo[SEC_D13];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;

	// Meduna airport & military complex
	pSector = &SectorInfo[SEC_N3];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;
	pSector = &SectorInfo[SEC_N4];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;
	pSector = &SectorInfo[SEC_N5];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;
	pSector = &SectorInfo[SEC_O3];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;
	pSector = &SectorInfo[SEC_O4];
	pSector->uiFacilitiesFlags |= SFCF_AIRPORT;
}


void TrashUndergroundSectorInfo()
{
	UNDERGROUND_SECTORINFO *curr;
	while( gpUndergroundSectorInfoHead )
	{
		curr = gpUndergroundSectorInfoHead;
		gpUndergroundSectorInfoHead = gpUndergroundSectorInfoHead->next;
		delete curr;
	}
	gpUndergroundSectorInfoHead = NULL;
	gpUndergroundSectorInfoTail = NULL;
}

//Defines the sectors that can be occupied by enemies, creatures, etc.  It also
//contains the network of cave connections critical for strategic creature spreading, as we can't
//know how the levels connect without loading the maps.
void BuildUndergroundSectorInfoList()
{
	TrashUndergroundSectorInfo();

	for (auto ugInfo : GCM->getUndergroundSectors())
	{
		AddUndergroundNode(ugInfo, gGameOptions.ubDifficultyLevel);
	}
}

//This is the function that is called only once, when the player begins a new game.  This will calculate
//starting numbers of the queen's army in various parts of the map, which will vary from campaign to campaign.
//This is also highly effected by the game's difficulty setting.
void InitNewCampaign()
{
	//First clear all the sector information of all enemy existance.  Conveniently, the
	//ubGroupType is also cleared, which is perceived to be an empty group.
	std::fill_n(SectorInfo, 256, SECTORINFO{});
	InitStrategicMovementCosts();
	RemoveAllGroups();

	InitKnowFacilitiesFlags( );

	BuildUndergroundSectorInfoList();

	// Allow overhead view of start sector on game onset.
	SetSectorFlag(SECTORX(START_SECTOR), SECTORY(START_SECTOR), 0, SF_ALREADY_VISITED);

	//Generates the initial forces in a new campaign.  The idea is to randomize numbers and sectors
	//so that no two games are the same.
	InitStrategicAI();

	InitStrategicStatus();

}
