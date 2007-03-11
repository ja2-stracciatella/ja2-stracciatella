#include "Soldier_Control.h"
#include "Militia_Control.h"
#include "Town_Militia.h"
#include "Soldier_Init_List.h"
#include "Campaign_Types.h"
#include "Overhead.h"
#include "StrategicMap.h"
#include "PreBattle_Interface.h"


BOOLEAN gfStrategicMilitiaChangesMade = FALSE;

void ResetMilitia()
{
	if( gfStrategicMilitiaChangesMade || gTacticalStatus.uiFlags & LOADING_SAVED_GAME )
	{
		gfStrategicMilitiaChangesMade = FALSE;
		RemoveMilitiaFromTactical();
		PrepareMilitiaForTactical();
	}
}

void RemoveMilitiaFromTactical()
{
	SOLDIERINITNODE *curr;
	INT32 i;
	for( i = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID; i <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; i++ )
	{
		if( MercPtrs[ i ]->bActive )
		{
			TacticalRemoveSoldier( MercPtrs[ i ]->ubID );
		}
	}
	curr = gSoldierInitHead;
	while( curr )
	{
		if( curr->pBasicPlacement->bTeam == MILITIA_TEAM )
		{
			curr->pSoldier = NULL;
		}
		curr = curr->next;
	}
}

void PrepareMilitiaForTactical()
{
	SECTORINFO *pSector;
//	INT32 i;
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
	/*
	for( i = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID; i <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; i++ )
	{
		if( MercPtrs[ i ]->bInSector )
		{
			MercPtrs[ i ]->bAttitude = AGGRESSIVE;
		}
	}
	*/
}

void HandleMilitiaPromotions( void )
{
	UINT8						cnt;
	UINT8						ubMilitiaRank;
	SOLDIERTYPE *		pTeamSoldier;
	UINT8						ubPromotions;

	gbGreenToElitePromotions = 0;
	gbGreenToRegPromotions = 0;
	gbRegToElitePromotions = 0;
	gbMilitiaPromotions = 0;

	cnt = gTacticalStatus.Team[ MILITIA_TEAM ].bFirstID;

	for ( pTeamSoldier = MercPtrs[ cnt ]; cnt <= gTacticalStatus.Team[ MILITIA_TEAM ].bLastID; cnt++, pTeamSoldier++)
	{
		if ( pTeamSoldier->bActive && pTeamSoldier->bInSector && pTeamSoldier->bLife > 0 )
		{
			if ( pTeamSoldier->ubMilitiaKills > 0 )
			{
				ubMilitiaRank = SoldierClassToMilitiaRank( pTeamSoldier->ubSoldierClass );
				ubPromotions = CheckOneMilitiaForPromotion( gWorldSectorX, gWorldSectorY, ubMilitiaRank, pTeamSoldier->ubMilitiaKills );
				if( ubPromotions )
				{
					if( ubPromotions == 2 )
					{
						gbGreenToElitePromotions++;
						gbMilitiaPromotions++;
					}
					else if( pTeamSoldier->ubSoldierClass == SOLDIER_CLASS_GREEN_MILITIA )
					{
						gbGreenToRegPromotions++;
						gbMilitiaPromotions++;
					}
					else if( pTeamSoldier->ubSoldierClass == SOLDIER_CLASS_REG_MILITIA )
					{
						gbRegToElitePromotions++;
						gbMilitiaPromotions++;
					}
				}

				pTeamSoldier->ubMilitiaKills = 0;
			}
		}
	}
	if( gbMilitiaPromotions )
	{
    // ATE: Problems here with bringing up message box...

		// UINT16 str[ 512 ];
		// BuildMilitiaPromotionsString( str );
		// DoScreenIndependantMessageBox( str, MSG_BOX_FLAG_OK, NULL );
	}
}
