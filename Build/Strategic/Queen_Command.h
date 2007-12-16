#ifndef __QUEEN_COMMAND_H
#define __QUEEN_COMMAND_H

#include "Types.h"
#include "Campaign_Types.h"
#include "Soldier_Control.h"
#include "Strategic_Movement.h"


extern BOOLEAN gfPendingEnemies;


//Counts enemies and crepitus, but not bloodcats.
UINT8 NumHostilesInSector( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ );

UINT8 NumEnemiesInAnySector( INT16 sSectorX, INT16 sSectorY, INT16 sSectorZ );

UINT8 NumEnemiesInSector( INT16 sSectorX, INT16 sSectorY );
UINT8 NumStationaryEnemiesInSector( INT16 sSectorX, INT16 sSectorY );
UINT8 NumMobileEnemiesInSector( INT16 sSectorX, INT16 sSectorY );
void GetNumberOfEnemiesInSector( INT16 sSectorX, INT16 sSectorY, UINT8 *pubNumAdmins, UINT8 *pubNumTroops, UINT8 *pubNumElites );

//Called when entering a sector so the campaign AI can automatically insert the
//correct number of troops of each type based on the current number in the sector
//in global focus (gWorldSectorX/Y)
BOOLEAN PrepareEnemyForSectorBattle(void);

void AddPossiblePendingEnemiesToBattle(void);
void EndTacticalBattleForEnemy(void);

void ProcessQueenCmdImplicationsOfDeath( SOLDIERTYPE *pSoldier );

BOOLEAN SaveUnderGroundSectorInfoToSaveGame( HWFILE hFile );
BOOLEAN LoadUnderGroundSectorInfoFromSavedGame( HWFILE hFile );

//Finds and returns the specified underground structure ( DONT MODIFY IT ).  Else returns NULL
UNDERGROUND_SECTORINFO* FindUnderGroundSector( INT16 sMapX, INT16 sMapY, UINT8 bMapZ );

void EnemyCapturesPlayerSoldier( SOLDIERTYPE *pSoldier );
void BeginCaptureSquence(void);
void EndCaptureSequence(void);

BOOLEAN PlayerSectorDefended( UINT8 ubSectorID );

BOOLEAN OnlyHostileCivsInSector(void);

extern INT16 gsInterrogationGridNo[3];

#endif
