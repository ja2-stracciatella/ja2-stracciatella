#ifndef __GAME_INIT_H
#define __GAME_INIT_H


BOOLEAN InitNewGame(  BOOLEAN fReset );
BOOLEAN AnyMercsHired(void);

void InitStrategicLayer(void);
void ShutdownStrategicLayer(void);

void ReStartingGame(void);

void InitBloodCatSectors(void);


#endif
