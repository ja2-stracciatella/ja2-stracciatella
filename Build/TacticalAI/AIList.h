#include "Types.h"

typedef struct AILIST
{
	UINT8							ubID;
	INT8							bPriority;
	UINT8							ubUnused;
	struct AILIST *		pNext;
} AILIST;

#define MAX_AI_PRIORITY 100

extern UINT8 RemoveFirstAIListEntry( void );
extern BOOLEAN BuildAIListForTeam( INT8 bTeam );
extern BOOLEAN MoveToFrontOfAIList( UINT8 ubID );
