#ifndef __GAME_INIT_H
#define __GAME_INIT_H


BOOLEAN InitNewGame(  BOOLEAN fReset );
BOOLEAN AnyMercsHired( );

void InitStrategicLayer();
void ShutdownStrategicLayer();

void ReStartingGame();

void InitBloodCatSectors();


#endif
