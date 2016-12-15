#ifndef __PITS_H
#define __PITS_H

#include "game/JA2Types.h"


void Add3X3Pit( INT32 iMapIndex );
void Add5X5Pit( INT32 iMapIndex );
void Remove3X3Pit( INT32 iMapIndex );
void Remove5X5Pit( INT32 iMapIndex );

void SearchForOtherMembersWithinPitRadiusAndMakeThemFall( INT16 sGridNo, INT16 sRadius );

void AddAllPits(void);
void RemoveAllPits(void);

extern BOOLEAN gfShowPits;

void HandleFallIntoPitFromAnimation(SOLDIERTYPE&);

#endif
