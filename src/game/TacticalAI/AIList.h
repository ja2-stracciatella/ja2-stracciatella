#ifndef AILIST_H
#define AILIST_H

#include "game/JA2Types.h"


SOLDIERTYPE* RemoveFirstAIListEntry();
bool         BuildAIListForTeam(INT8 team);
bool         MoveToFrontOfAIList(SOLDIERTYPE*);

#endif
