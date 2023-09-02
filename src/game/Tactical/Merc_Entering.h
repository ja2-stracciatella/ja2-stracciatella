#ifndef _MERC_ENTRING_H
#define _MERC_ENTRING_H

#include "JA2Types.h"

void ResetHeliSeats(void);
void AddMercToHeli(SOLDIERTYPE* s);

void StartHelicopterRun( INT16 sGridNoSweetSpot );


void HandleHeliDrop(void);


extern BOOLEAN gfIngagedInDrop;

#endif
