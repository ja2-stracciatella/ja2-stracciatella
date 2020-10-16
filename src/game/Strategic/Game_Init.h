#ifndef __GAME_INIT_H
#define __GAME_INIT_H
#include "Types.h"

void InitNewGame();
BOOLEAN AnyMercsHired(void);

void InitStrategicLayer(void);
void ShutdownStrategicLayer(void);

void ReStartingGame(void);

void InitBloodCatSectors(void);


#endif
