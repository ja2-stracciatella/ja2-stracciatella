#ifndef __ENEMY_SOLDIER_SAVE_H
#define __ENEMY_SOLDIER_SAVE_H

#include "FileMan.h"

//This funciton load both the enemies AND the civilians, now only used to load old saves
BOOLEAN LoadEnemySoldiersFromTempFile();

//The new way of loading in enemies from the temp file.  ONLY the ENEMIES are stored in the e_* temp file
BOOLEAN NewWayOfLoadingEnemySoldiersFromTempFile();

//The new way of loading in CIVILIANS from the temp file.  ONLY the CIVILIEANS are stored in the c_* temp file
BOOLEAN NewWayOfLoadingCiviliansFromTempFile();


BOOLEAN NewWayOfSavingEnemyAndCivliansToTempFile( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, BOOLEAN fEnemy, BOOLEAN fValidateOnly  );




BOOLEAN SaveEnemySoldiersToTempFile( INT16 sSectorX, INT16 sSectorY, INT8 bSectorZ, UINT8	ubFirstIdTeam, UINT8 ubLastIdTeam, BOOLEAN fAppendToFile );

extern BOOLEAN gfRestoringEnemySoldiersFromTempFile;

#endif
