#include "Soldier_Control.h"
#include "Militia_Control.h"
#include "Town_Militia.h"
#include "Soldier_Init_List.h"
#include "Campaign_Types.h"
#include "Overhead.h"
#include "StrategicMap.h"
#include "PreBattle_Interface.h"

#include "ContentManager.h"
#include "GameInstance.h"
#include "policy/GamePolicy.h"


BOOLEAN gfStrategicMilitiaChangesMade = FALSE;


static void RemoveMilitiaFromTactical(void);


void ResetMilitia()
{
	if( gfStrategicMilitiaChangesMade || gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
	{
		if gamepolicy(militia_use_sector_inventory) TeamDropAll(MILITIA_TEAM, OUR_TEAM);
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
	MilitiaEquipFromLoadedSectorInventory(true);
}


void HandleMilitiaPromotions()
{
	gbGreenToElitePromotions = 0;
	gbGreenToRegPromotions   = 0;
	gbRegToElitePromotions   = 0;
	gbMilitiaPromotions      = 0;

	FOR_EACH_SOLDIER(i)
	{
		SOLDIERTYPE& s = *i;
		if (s.bTeam != MILITIA_TEAM && s.ubWhatKindOfMercAmI != MERC_TYPE__MILITA) continue;
		if (!s.bInSector)          continue;
		if (s.bLife <= 0)          continue;
		if (s.ubMilitiaKills == 0) continue;

		UINT8 militia_rank = SoldierClassToMilitiaRank(s.ubSoldierClass);
		UINT8 const promotions   = CheckOneMilitiaForPromotion(gWorldSectorX, gWorldSectorY, militia_rank, s.ubMilitiaKills);
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
