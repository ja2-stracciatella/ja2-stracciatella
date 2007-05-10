#ifndef __FOV_H
#define __FOV_H

void RevealRoofsAndItems(SOLDIERTYPE *pSoldier, UINT32 itemsToo, BOOLEAN fShowLocators, UINT8 ubLevel, BOOLEAN fForce );

void ClearSlantRoofs( void );
void AddSlantRoofFOVSlot( INT16 sGridNo );
void ExamineSlantRoofFOVSlots( );

#endif
