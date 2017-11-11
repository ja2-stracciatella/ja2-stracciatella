#include "Soldier_Control.h"
#include "Militia_Control.h"
#include "Town_Militia.h"
#include "Soldier_Init_List.h"
#include "Campaign_Types.h"
#include "Overhead.h"
#include "StrategicMap.h"
#include "PreBattle_Interface.h"


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
	FOR_EACH_IN_TEAM(i, MILITIA_TEAM) TacticalRemoveSoldier(*i);
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
	if( gbWorldSectorZ > 0 )
		return;

	// Do we have a loaded sector?
	if ( gWorldSectorX ==0 && gWorldSectorY == 0 )
		return;

	pSector = &SectorInfo[ SECTOR( gWorldSectorX, gWorldSectorY ) ];
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

		UINT8 const militia_rank = SoldierClassToMilitiaRank(s.ubSoldierClass);
		UINT8 const promotions   = CheckOneMilitiaForPromotion(gWorldSectorX, gWorldSectorY, militia_rank, s.ubMilitiaKills);
		if (promotions != 0)
		{
			if (promotions == 2)
			{
				++gbGreenToElitePromotions;
				++gbMilitiaPromotions;
			}
			else if (s.ubSoldierClass == SOLDIER_CLASS_GREEN_MILITIA)
			{
				++gbGreenToRegPromotions;
				++gbMilitiaPromotions;
			}
			else if (s.ubSoldierClass == SOLDIER_CLASS_REG_MILITIA)
			{
				++gbRegToElitePromotions;
				++gbMilitiaPromotions;
			}
		}

		s.ubMilitiaKills = 0;
	}
}
