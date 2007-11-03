#ifndef AILIST_H
#define AILIST_H

#include "Types.h"

extern UINT8 RemoveFirstAIListEntry( void );
extern BOOLEAN BuildAIListForTeam( INT8 bTeam );
extern BOOLEAN MoveToFrontOfAIList( UINT8 ubID );

#endif
