#ifndef __FOV_H
#define __FOV_H

#include "overhead.h"

void RevealRoofsAndItems(SOLDIERTYPE *pSoldier, UINT32 itemsToo, BOOLEAN fShowLocators, UINT8 ubLevel, BOOLEAN fForce );

INT32 GetFreeSlantRoof( void );
void RecountSlantRoofs( void );
void ClearSlantRoofs( void );
BOOLEAN FindSlantRoofSlot( INT16 sGridNo );
void AddSlantRoofFOVSlot( INT16 sGridNo );
void ExamineSlantRoofFOVSlots( );









#endif
