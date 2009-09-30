#include "Types.h"
#include "Campaign_Init.h"
#include "Quests.h"
#include "Random.h"
#include "Campaign_Types.h"
#include "Queen_Command.h"
#include "Overhead.h"
#include "Strategic_Mines.h"
#include "Strategic_Movement.h"
#include "Strategic_Movement_Costs.h"
#include "Strategic_Status.h"
#include "GameSettings.h"
#include "Creature_Spreading.h"
#include "Strategic_AI.h"
#include "Tactical_Save.h"
#include "MemMan.h"


UNDERGROUND_SECTORINFO* gpUndergroundSectorInfoTail = NULL;


static UNDERGROUND_SECTORINFO* NewUndergroundNode(UINT8 const x, UINT8 const y, UINT8 const z)
{
	UNDERGROUND_SECTORINFO* const u = MALLOCZ(UNDERGROUND_SECTORINFO);
	u->ubSectorX = x;
	u->ubSectorY = y;
	u->ubSectorZ = z;

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


static void InitMiningLocations()
{
	FOR_EACH(MINE_LOCATION_TYPE const, i, gMineLocation)
	{
		MINE_LOCATION_TYPE const& l = *i;
		SectorInfo[SECTOR(l.sSectorX, l.sSectorY)].uiFlags |= SF_MINING_SITE;
	}
}


void TrashUndergroundSectorInfo()
{
	UNDERGROUND_SECTORINFO *curr;
	while( gpUndergroundSectorInfoHead )
	{
		curr = gpUndergroundSectorInfoHead;
		gpUndergroundSectorInfoHead = gpUndergroundSectorInfoHead->next;
		MemFree( curr );
	}
	gpUndergroundSectorInfoHead = NULL;
	gpUndergroundSectorInfoTail = NULL;
}

//Defines the sectors that can be occupied by enemies, creatures, etc.  It also
//contains the network of cave connections critical for strategic creature spreading, as we can't
//know how the levels connect without loading the maps.  This is completely hardcoded, and any
//changes to the maps, require changes accordingly.
void BuildUndergroundSectorInfoList()
{
	UNDERGROUND_SECTORINFO *curr;

	TrashUndergroundSectorInfo();

	//Miguel's basement.  Nothing here.
	curr = NewUndergroundNode( 10, 1, 1 );

	//Chitzena mine.  Nothing here.
	curr = NewUndergroundNode( 2, 2, 1 );

	//San mona mine.  Nothing here.
	curr = NewUndergroundNode( 4, 4, 1 );
	curr = NewUndergroundNode( 5, 4, 1 );

	//J9
	curr = NewUndergroundNode( 9, 10, 1 );
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			curr->ubNumTroops = 8;
			break;
		case DIF_LEVEL_MEDIUM:
			curr->ubNumTroops = 11;
			break;
		case DIF_LEVEL_HARD:
			curr->ubNumTroops = 15;
			break;
	}
	//J9 feeding zone
	curr = NewUndergroundNode( 9, 10, 2 );
	curr->ubNumCreatures = (UINT8)(2 + gGameOptions.ubDifficultyLevel*2 + Random( 2 ));

	//K4
	curr = NewUndergroundNode( 4, 11, 1 );
	curr->ubNumTroops = (UINT8)(6 + gGameOptions.ubDifficultyLevel*2 + Random( 3 ));
	curr->ubNumElites = (UINT8)(4 + gGameOptions.ubDifficultyLevel + Random( 2 ));

	//O3
	curr = NewUndergroundNode( 3, 15, 1 );
	curr->ubNumTroops = (UINT8)(6 + gGameOptions.ubDifficultyLevel*2 + Random( 3 ));
	curr->ubNumElites = (UINT8)(4 + gGameOptions.ubDifficultyLevel + Random( 2 ));
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;

	//P3
	curr = NewUndergroundNode( 3, 16, 1 );
	switch( gGameOptions.ubDifficultyLevel )
	{
		case DIF_LEVEL_EASY:
			curr->ubNumElites = (UINT8)(8 + Random( 3 ));
			break;
		case DIF_LEVEL_MEDIUM:
			curr->ubNumElites = (UINT8)(10 + Random( 5 ));
			break;
		case DIF_LEVEL_HARD:
			curr->ubNumElites = (UINT8)(14 + Random( 6 ));
			break;
	}
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;

	//Do all of the mandatory underground mine sectors

	//Drassen's mine
	//D13_B1
	curr = NewUndergroundNode( 13, 4, 1 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
	//E13_B1
	curr = NewUndergroundNode( 13, 5, 1 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
	//E13_B2
	curr = NewUndergroundNode( 13, 5, 2 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
	//F13_B2
	curr = NewUndergroundNode( 13, 6, 2 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR | SOUTH_ADJACENT_SECTOR;
	//G13_B2
	curr = NewUndergroundNode( 13, 7, 2 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
	//G13_B3
	curr = NewUndergroundNode( 13, 7, 3 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
	//F13_B3
	curr = NewUndergroundNode( 13, 6, 3 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;

	//Cambria's mine
	//H8_B1
	curr = NewUndergroundNode( 8, 8, 1 );
	curr->ubAdjacentSectors |= EAST_ADJACENT_SECTOR;
	//H9_B1
	curr = NewUndergroundNode( 9, 8, 1 );
	curr->ubAdjacentSectors |= WEST_ADJACENT_SECTOR;
	//H9_B2
	curr = NewUndergroundNode( 9, 8, 2 );
	curr->ubAdjacentSectors |= WEST_ADJACENT_SECTOR;
	//H8_B2
	curr = NewUndergroundNode( 8, 8, 2 );
	curr->ubAdjacentSectors |= EAST_ADJACENT_SECTOR;
	//H8_B3
	curr = NewUndergroundNode( 8, 8, 3 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
	//I8_B3
	curr = NewUndergroundNode( 8, 9, 3 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR | SOUTH_ADJACENT_SECTOR;
	//J8_B3
	curr = NewUndergroundNode( 8, 10, 3 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;

	//Alma's mine
	//I14_B1
	curr = NewUndergroundNode( 14, 9, 1 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
	//J14_B1
	curr = NewUndergroundNode( 14, 10, 1 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
	//J14_B2
	curr = NewUndergroundNode( 14, 10, 2 );
	curr->ubAdjacentSectors |= WEST_ADJACENT_SECTOR;
	//J13_B2
	curr = NewUndergroundNode( 13, 10, 2 );
	curr->ubAdjacentSectors |= EAST_ADJACENT_SECTOR;
	//J13_B3
	curr = NewUndergroundNode( 13, 10, 3 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
	//K13_B3
	curr = NewUndergroundNode( 13, 11, 3 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;

	//Grumm's mine
	//H3_B1
	curr = NewUndergroundNode( 3, 8, 1 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;
	//I3_B1
	curr = NewUndergroundNode( 3, 9, 1 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
	//I3_B2
	curr = NewUndergroundNode( 3, 9, 2 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
	//H3_B2
	curr = NewUndergroundNode( 3, 8, 2 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR | EAST_ADJACENT_SECTOR;
	//H4_B2
	curr = NewUndergroundNode( 4, 8, 2 );
	curr->ubAdjacentSectors |= WEST_ADJACENT_SECTOR;
	curr->uiFlags |= SF_PENDING_ALTERNATE_MAP;
	//H4_B3
	curr = NewUndergroundNode( 4, 8, 3 );
	curr->ubAdjacentSectors |= NORTH_ADJACENT_SECTOR;
	//G4_B3
	curr = NewUndergroundNode( 4, 7, 3 );
	curr->ubAdjacentSectors |= SOUTH_ADJACENT_SECTOR;

#ifdef JA2DEMO
	curr = NewUndergroundNode(1, 16, 1);
	switch (gGameOptions.ubDifficultyLevel)
	{
		case DIF_LEVEL_EASY:   curr->ubNumTroops =  8; break;
		case DIF_LEVEL_MEDIUM: curr->ubNumTroops = 11; break;
		case DIF_LEVEL_HARD:   curr->ubNumTroops = 15; break;
	}
	curr = NewUndergroundNode(1, 16, 2);
	curr->ubNumCreatures = 2 + gGameOptions.ubDifficultyLevel * 2 + Random(2);
#endif
}

//This is the function that is called only once, when the player begins a new game.  This will calculate
//starting numbers of the queen's army in various parts of the map, which will vary from campaign to campaign.
//This is also highly effected by the game's difficulty setting.
void InitNewCampaign()
{
	//First clear all the sector information of all enemy existance.  Conveniently, the
	//ubGroupType is also cleared, which is perceived to be an empty group.
	memset( &SectorInfo, 0, sizeof( SECTORINFO ) * 256 );
	InitStrategicMovementCosts();
	RemoveAllGroups();

	InitMiningLocations();
	InitKnowFacilitiesFlags( );

	BuildUndergroundSectorInfoList();

	// Allow overhead view of start sector on game onset.
	SetSectorFlag(SECTORX(START_SECTOR), SECTORY(START_SECTOR), 0, SF_ALREADY_VISITED);

	//Generates the initial forces in a new campaign.  The idea is to randomize numbers and sectors
	//so that no two games are the same.
	InitStrategicAI();

	InitStrategicStatus();

}
