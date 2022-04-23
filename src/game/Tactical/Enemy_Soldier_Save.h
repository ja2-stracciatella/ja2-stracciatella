#ifndef __ENEMY_SOLDIER_SAVE_H
#define __ENEMY_SOLDIER_SAVE_H

#include "Types.h"


//This funciton load both the enemies AND the civilians, now only used to load old saves
void LoadEnemySoldiersFromTempFile();

//The new way of loading in enemies from the temp file.  ONLY the ENEMIES are stored in the e_* temp file
void NewWayOfLoadingEnemySoldiersFromTempFile();

//The new way of loading in CIVILIANS from the temp file.  ONLY the CIVILIEANS are stored in the c_* temp file
void NewWayOfLoadingCiviliansFromTempFile();


void NewWayOfSavingEnemyAndCivliansToTempFile(const SGPSector& sSector, BOOLEAN fEnemy, BOOLEAN fValidateOnly);


extern BOOLEAN gfRestoringEnemySoldiersFromTempFile;

#endif
