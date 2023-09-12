#include "Militia_Control.h"
#include "Campaign_Types.h"
#include "Overhead.h"
#include "Overhead_Types.h"
#include "Soldier_Control.h"
#include "Soldier_Create.h"
#include "Soldier_Init_List.h"
#include "StrategicMap.h"
#include "Town_Militia.h"
#include <stdexcept>


BOOLEAN gfStrategicMilitiaChangesMade = FALSE;


static void RemoveMilitiaFromTactical(void);


void ResetMilitia()
{
	if( gfStrategicMilitiaChangesMade || gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
	{
		gfStrategicMilitiaChangesMade = FALSE;
		RemoveMilitiaFromTactical();
		PrepareMilitiaForTactical();
	}
}


static void RemoveMilitiaFromTactical(void)
{
	TrashAllSoldiers(MILITIA_TEAM);
	FOR_EACH_SOLDIERINITNODE(curr)
	{
		if( curr->pBasicPlacement->bTeam == MILITIA_TEAM )
		{
			curr->pSoldier = NULL;
		}
	}
}

void PrepareMilitiaForTactical()
{
	SECTORINFO *pSector;
	//INT32 i;
	UINT8 ubGreen, ubRegs, ubElites;
	if (gWorldSector.z > 0)
		return;

	// Do we have a loaded sector?
	if (gWorldSector.x == 0 && gWorldSector.y == 0)
		return;

	pSector = &SectorInfo[gWorldSector.AsByte()];
	ubGreen = pSector->ubNumberOfCivsAtLevel[ GREEN_MILITIA ];
	ubRegs = pSector->ubNumberOfCivsAtLevel[ REGULAR_MILITIA ];
	ubElites = pSector->ubNumberOfCivsAtLevel[ ELITE_MILITIA ];
	AddSoldierInitListMilitia( ubGreen, ubRegs, ubElites );
}


void HandleMilitiaPromotions()
{
	gbGreenToElitePromotions = 0;
	gbGreenToRegPromotions   = 0;
	gbRegToElitePromotions   = 0;
	gbMilitiaPromotions      = 0;

	FOR_EACH_IN_TEAM(i, MILITIA_TEAM)
	{
		SOLDIERTYPE& s = *i;
		if (!s.bInSector)          continue;
		if (s.bLife <= 0)          continue;
		if (s.ubMilitiaKills == 0) continue;

		// Is this a regular militia or a "rebel" (the editor kind, not the
		// A10 basement kind? Only regular milita can be promoted.
		auto const rank = SoldierClassToMilitiaRank(s.ubSoldierClass);
		if (!rank) continue;
		auto militia_rank = *rank;

		UINT8 const promotions   = CheckOneMilitiaForPromotion(gWorldSector, militia_rank, s.ubMilitiaKills);
		if (promotions != 0)
		{
			if (militia_rank == ELITE_MILITIA) {
				// MOD: too good already, increase xp level or other stats?
				continue;
			}
			++gbMilitiaPromotions;
			if (promotions == 2)
			{
				++gbGreenToElitePromotions;
			}
			else if (militia_rank == GREEN_MILITIA)
			{
				++gbGreenToRegPromotions;
			}
			else if (militia_rank == REGULAR_MILITIA)
			{
				++gbRegToElitePromotions;
			}

			s.ubMilitiaKills = 0;
		}
	}
}
