#ifndef AILIST_H
#define AILIST_H

#include "JA2Types.h"


SOLDIERTYPE* RemoveFirstAIListEntry();
bool         BuildAIListForTeam(INT8 team);
BOOLEAN MoveToFrontOfAIList(SOLDIERTYPE* s);

#endif
