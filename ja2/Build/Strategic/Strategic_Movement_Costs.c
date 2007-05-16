#include "Campaign_Types.h"
#include "Queen_Command.h"
#include "Strategic_Movement.h"

#ifdef JA2TESTVERSION
#	include "Debug.h"
#	include "Stubs.h"
#endif


static void InitStrategicRowA(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_A1 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS;

	pSector = &SectorInfo[ SEC_A2 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_A3 ];
	pSector->ubTravelRating = 9;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_A4 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_A5 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_A6 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_A7 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_A8 ];
	pSector->ubTravelRating = 14;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_A9 ];
	pSector->ubTravelRating = 70;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_A10 ];
	pSector->ubTravelRating = 70;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_A11 ];
	pSector->ubTravelRating = 18;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_A12 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_A13 ];
	pSector->ubTravelRating = 14;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_A14 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_A15 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_A16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowB(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_B1 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS;

	pSector = &SectorInfo[ SEC_B2 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_B3 ];
	pSector->ubTravelRating = 6;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_B4 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_B5 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_B6 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_B7 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_B8 ];
	pSector->ubTravelRating = 20;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_B9 ];
	pSector->ubTravelRating = 70;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_B10 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_B11 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_B12 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND_ROAD;

	pSector = &SectorInfo[ SEC_B13 ];
	pSector->ubTravelRating = 85;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_B14 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = WATER;

	pSector = &SectorInfo[ SEC_B15 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_B16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;
}


static void InitStrategicRowC(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_C1 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS;

	pSector = &SectorInfo[ SEC_C2 ];
	pSector->ubTravelRating = 40;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS_ROAD;

	pSector = &SectorInfo[ SEC_C3 ];
	pSector->ubTravelRating = 40;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_C4 ];
	pSector->ubTravelRating = 20;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_C5 ];
	pSector->ubTravelRating = 80;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_C6 ];
	pSector->ubTravelRating = 75;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_C7 ];
	pSector->ubTravelRating = 45;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_C8 ];
	pSector->ubTravelRating = 48;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_C9 ];
	pSector->ubTravelRating = 80;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_C10 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_C11 ];
	pSector->ubTravelRating = 3;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_C12 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_C13 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_C14 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = WATER;

	pSector = &SectorInfo[ SEC_C15 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_C16 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;
}


static void InitStrategicRowD(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_D1 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_D2 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;//TROPICS_SAMSITE;

	pSector = &SectorInfo[ SEC_D3 ];
	pSector->ubTravelRating = 40;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_D4 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_D5 ];
	pSector->ubTravelRating = 49;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_D6 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_D7 ];
	pSector->ubTravelRating = 45;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE_ROAD;

	pSector = &SectorInfo[ SEC_D8 ];
	pSector->ubTravelRating = 16;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_D9 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_D10 ];
	pSector->ubTravelRating = 11;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_D11 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_D12 ];
	pSector->ubTravelRating = 11;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_D13 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_D14 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = WATER;

	pSector = &SectorInfo[ SEC_D15 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_D16 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;
}


static void InitStrategicRowE(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_E1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_E2 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS;

	pSector = &SectorInfo[ SEC_E3 ];
	pSector->ubTravelRating = 45;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_E4 ];
	pSector->ubTravelRating = 11;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_E5 ];
	pSector->ubTravelRating = 9;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_E6 ];
	pSector->ubTravelRating = 11;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_E7 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND_ROAD;

	pSector = &SectorInfo[ SEC_E8 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_E9 ];
	pSector->ubTravelRating = 56;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND_ROAD;

	pSector = &SectorInfo[ SEC_E10 ];
	pSector->ubTravelRating = 11;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_E11 ];
	pSector->ubTravelRating = 9;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_E12 ];
	pSector->ubTravelRating = 35;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_E13 ];
	pSector->ubTravelRating = 45;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = WATER;

	pSector = &SectorInfo[ SEC_E14 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_E15 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE_ROAD;

	pSector = &SectorInfo[ SEC_E16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowF(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_F1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_F2 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= NS_RIVER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL;

	pSector = &SectorInfo[ SEC_F3 ];
	pSector->ubTravelRating = 40;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= NS_RIVER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_F4 ];
	pSector->ubTravelRating = 9;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_F5 ];
	pSector->ubTravelRating = 6;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_F6 ];
	pSector->ubTravelRating = 9;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_F7 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS_ROAD;

	pSector = &SectorInfo[ SEC_F8 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_F9 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_F10 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_F11 ];
	pSector->ubTravelRating = 9;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_F12 ];
	pSector->ubTravelRating = 40;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_F13 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_F14 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_F15 ];
	pSector->ubTravelRating = 3;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE_ROAD;

	pSector = &SectorInfo[ SEC_F16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowG(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_G1 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_G2 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_G3 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_G4 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS_ROAD;

	pSector = &SectorInfo[ SEC_G5 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS_ROAD;

	pSector = &SectorInfo[ SEC_G6 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS_ROAD;

	pSector = &SectorInfo[ SEC_G7 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND_ROAD;

	pSector = &SectorInfo[ SEC_G8 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_G9 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_G10 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND_ROAD;

	pSector = &SectorInfo[ SEC_G11 ];
	pSector->ubTravelRating = 25;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND_ROAD;

	pSector = &SectorInfo[ SEC_G12 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_G13 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_G14 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_G15 ];
	pSector->ubTravelRating = 16;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_G16 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;
}


static void InitStrategicRowH(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_H1 ];
	pSector->ubTravelRating = 67;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_H2 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_H3 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_H4 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_H5 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS;

	pSector = &SectorInfo[ SEC_H6 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = HILLS_ROAD;

	pSector = &SectorInfo[ SEC_H7 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= HILLS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_H8 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_H9 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_H10 ];
	pSector->ubTravelRating = 3;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_H11 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_H12 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_H13 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_H14 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_H15 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_H16 ];
	pSector->ubTravelRating = 3;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;
}


static void InitStrategicRowI(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_I1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_I2 ];
	pSector->ubTravelRating = 3;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_I3 ];
	pSector->ubTravelRating = 45;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_I4 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_I5 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_I6 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_I7 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_I8 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_I9 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_I10 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_I11 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_I12 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_I13 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_I14 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_I15 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_I16 ];
	pSector->ubTravelRating = 2;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;
}


static void InitStrategicRowJ(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_J1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_J2 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_J3 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_J4 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_J5 ];
	pSector->ubTravelRating = 3;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_J6 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_J7 ];
	pSector->ubTravelRating = 6;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_J8 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_J9 ];
	pSector->ubTravelRating = 80;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_J10 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_J11 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_J12 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_J13 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_J14 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SPARSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND_ROAD;

	pSector = &SectorInfo[ SEC_J15 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_J16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowK(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_K1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_K2 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_K3 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_K4 ];
	pSector->ubTravelRating = 45;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_K5 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_K6 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_K7 ];
	pSector->ubTravelRating = 60;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND_ROAD;

	pSector = &SectorInfo[ SEC_K8 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND_ROAD;

	pSector = &SectorInfo[ SEC_K9 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND_ROAD;

	pSector = &SectorInfo[ SEC_K10 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_K11 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_K12 ];
	pSector->ubTravelRating = 70;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_K13 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_K14 ];
	pSector->ubTravelRating = 50;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_K15 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_K16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowL(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_L1 ];
	pSector->ubTravelRating = 4;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL;

	pSector = &SectorInfo[ SEC_L2 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_L3 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_L4 ];
	pSector->ubTravelRating = 15;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_L5 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= DENSE;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_L6 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS_ROAD;

	pSector = &SectorInfo[ SEC_L7 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_L8 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SAND;

	pSector = &SectorInfo[ SEC_L9 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_L10 ];
	pSector->ubTravelRating = 9;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE_ROAD;

	pSector = &SectorInfo[ SEC_L11 ];
	pSector->ubTravelRating = 17;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_L12 ];
	pSector->ubTravelRating = 55;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_L13 ];
	pSector->ubTravelRating = 18;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL;

	pSector = &SectorInfo[ SEC_L14 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_L15 ];
	pSector->ubTravelRating = 3;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE;

	pSector = &SectorInfo[ SEC_L16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowM(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_M1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_M2 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_M3 ];
	pSector->ubTravelRating = 70;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SWAMP;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP_ROAD;

	pSector = &SectorInfo[ SEC_M4 ];
	pSector->ubTravelRating = 38;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = FARMLAND;

	pSector = &SectorInfo[ SEC_M5 ];
	pSector->ubTravelRating = 70;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE_ROAD;

	pSector = &SectorInfo[ SEC_M6 ];
	pSector->ubTravelRating = 65;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = DENSE_ROAD;

	pSector = &SectorInfo[ SEC_M7 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_M8 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= SAND;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_M9 ];
	pSector->ubTravelRating = 8;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = PLAINS;

	pSector = &SectorInfo[ SEC_M10 ];
	pSector->ubTravelRating = 7;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS_ROAD;

	pSector = &SectorInfo[ SEC_M11 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_M12 ];
	pSector->ubTravelRating = 12;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_M13 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SPARSE;

	pSector = &SectorInfo[ SEC_M14 ];
	pSector->ubTravelRating = 2;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = SWAMP;

	pSector = &SectorInfo[ SEC_M15 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_M16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowN(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_N1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_N2 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_N3 ];
	pSector->ubTravelRating = 80;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_N4 ];
	pSector->ubTravelRating = 80;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_N5 ];
	pSector->ubTravelRating = 80;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_N6 ];
	pSector->ubTravelRating = 40;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS_ROAD;

	pSector = &SectorInfo[ SEC_N7 ];
	pSector->ubTravelRating = 20;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_N8 ];
	pSector->ubTravelRating = 10;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = COASTAL_ROAD;

	pSector = &SectorInfo[ SEC_N9 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS_ROAD;

	pSector = &SectorInfo[ SEC_N10 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= ROAD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS_ROAD;

	pSector = &SectorInfo[ SEC_N11 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_N12 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_N13 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_N14 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_N15 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_N16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowO(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_O1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O2 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O3 ];
	pSector->ubTravelRating = 90;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_O4 ];
	pSector->ubTravelRating = 90;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_O5 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O6 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O7 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O8 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS;

	pSector = &SectorInfo[ SEC_O9 ];
	pSector->ubTravelRating = 5;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= PLAINS;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TROPICS;

	pSector = &SectorInfo[ SEC_O10 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O11 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O12 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O13 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_O14 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_O15 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_O16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}


static void InitStrategicRowP(void)
{
	SECTORINFO *pSector;

	pSector = &SectorInfo[ SEC_P1 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;
//	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = WATER; //keep as water so we can teleport to demo maps.

	pSector = &SectorInfo[ SEC_P2 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P3 ];
	pSector->ubTravelRating = 100;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= TOWN;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = TOWN;

	pSector = &SectorInfo[ SEC_P4 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P5 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P6 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P7 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P8 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P9 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P10 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P11 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P12 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P13 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= GROUNDBARRIER;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = GROUNDBARRIER;

	pSector = &SectorInfo[ SEC_P14 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_P15 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;

	pSector = &SectorInfo[ SEC_P16 ];
	pSector->ubTravelRating = 0;
	pSector->ubTraversability[ NORTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ EAST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ SOUTH_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ WEST_STRATEGIC_MOVE ]		= EDGEOFWORLD;
	pSector->ubTraversability[ THROUGH_STRATEGIC_MOVE ] = EDGEOFWORLD;
}

void InitStrategicMovementCosts()
{
	InitStrategicRowA();
	InitStrategicRowB();
	InitStrategicRowC();
	InitStrategicRowD();
	InitStrategicRowE();
	InitStrategicRowF();
	InitStrategicRowG();
	InitStrategicRowH();
	InitStrategicRowI();
	InitStrategicRowJ();
	InitStrategicRowK();
	InitStrategicRowL();
	InitStrategicRowM();
	InitStrategicRowN();
	InitStrategicRowO();
	InitStrategicRowP();
	#ifdef JA2TESTVERSION
	{ //Simply make sure all shared borders between sectors match.
		INT32 x,y;
		OutputDebugString( "STRATEGIC MOVE COSTS:  Testing validity of data (okay if no following debug msgs)...\n");
		for( y = 1; y <= 16; y++ ) for( x = 1; x <= 16; x++ )
		{
			if( y > 1 )
			{
				if( SectorInfo[ SECTOR(x,y) ].ubTraversability[ NORTH_STRATEGIC_MOVE ] !=
					  SectorInfo[ SECTOR(x,y-1) ].ubTraversability[ SOUTH_STRATEGIC_MOVE ] )
				{	//north
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  North mismatch for sector %c%d\n", y+'A'-1, x ) );
				}
				if( y == 16 && SectorInfo[ SECTOR(x,y) ].ubTraversability[ SOUTH_STRATEGIC_MOVE ] != EDGEOFWORLD )
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  South should be EDGEOFWORLD for sector %c%d\n", y+'A'-1, x ) );
			}
			else
			{
				if( SectorInfo[ SECTOR(x,y) ].ubTraversability[ SOUTH_STRATEGIC_MOVE ] !=
				    SectorInfo[ SECTOR(x,y+1) ].ubTraversability[ NORTH_STRATEGIC_MOVE ] )
				{ //south
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  South mismatch for sector %c%d\n", y+'A'-1, x ) );
				}
				if( SectorInfo[ SECTOR(x,y) ].ubTraversability[ NORTH_STRATEGIC_MOVE ] != EDGEOFWORLD )
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  North should be EDGEOFWORLD for sector %c%d\n", y+'A'-1, x ) );
			}
			if( x < 16 )
			{
				if( SectorInfo[ SECTOR(x,y) ].ubTraversability[ EAST_STRATEGIC_MOVE ] !=
					  SectorInfo[ SECTOR(x+1,y) ].ubTraversability[ WEST_STRATEGIC_MOVE ] )
				{ //east
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  East mismatch for sector %c%d\n", y+'A'-1, x ) );
				}
				if( x == 1 && SectorInfo[ SECTOR(x,y) ].ubTraversability[ WEST_STRATEGIC_MOVE ] != EDGEOFWORLD )
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  West should be EDGEOFWORLD for sector %c%d\n", y+'A'-1, x ) );
			}
			else
			{
				if( SectorInfo[ SECTOR(x,y) ].ubTraversability[ WEST_STRATEGIC_MOVE ] !=
				    SectorInfo[ SECTOR(x-1,y) ].ubTraversability[ EAST_STRATEGIC_MOVE ] )
				{ //west
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  West mismatch for sector %c%d\n", y+'A'-1, x ) );
				}
				if( SectorInfo[ SECTOR(x,y) ].ubTraversability[ EAST_STRATEGIC_MOVE ] != EDGEOFWORLD )
					OutputDebugString( String( "STRATEGIC MOVE COSTS:  East should be EDGEOFWORLD for sector %c%d\n", y+'A'-1, x ) );
			}
		}
	}
	#endif
}

UINT8 GetTraversability( INT16 sStartSector, INT16 sEndSector )
{
	UINT8 ubDirection = 0;
	INT16 sDifference = 0;

	// given start and end sectors
	sDifference = sEndSector - sStartSector;


	if( sDifference == -1 )
	{
		ubDirection = WEST_STRATEGIC_MOVE;
	}
	else if( sDifference == 1 )
	{
		ubDirection = EAST_STRATEGIC_MOVE;
	}
	else if( sDifference == 16 )
	{
		ubDirection = SOUTH_STRATEGIC_MOVE;
	}
	else
	{
		ubDirection = NORTH_STRATEGIC_MOVE;
	}


	return( SectorInfo[ sStartSector ].ubTraversability[ ubDirection ] );
}

BOOLEAN SectorIsImpassable( INT16 sSector )
{
	// returns true if the sector is impassable in all directions
	return( SectorInfo[ sSector ].ubTraversability[ THROUGH_STRATEGIC_MOVE ] == GROUNDBARRIER ||
		SectorInfo[ sSector ].ubTraversability[ THROUGH_STRATEGIC_MOVE ] == EDGEOFWORLD );
}
