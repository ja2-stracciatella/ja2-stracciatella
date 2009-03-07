#ifndef _QUEST_DEBUG_SYSTEM_H_
#define _QUEST_DEBUG_SYSTEM_H_

#include "NPC.h"
#include "ScreenIDs.h"


extern	INT16				gsQdsEnteringGridNo;


void NpcRecordLoggingInit(UINT8 ubNpcID, UINT8 ubMercID, UINT8 ubQuoteNum, Approach);
void NpcRecordLogging(Approach, char const* pStringA, ...);

void     QuestDebugScreenInit(void);
ScreenID QuestDebugScreenHandle(void);

#endif
