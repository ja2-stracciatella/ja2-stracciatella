#ifndef _QUEST_DEBUG_SYSTEM_H_
#define _QUEST_DEBUG_SYSTEM_H_

#include "NPC.h"
#include "JA2Types.h"
#include "ScreenIDs.h"


extern	INT16				gsQdsEnteringGridNo;


void NpcRecordLoggingInit(ProfileID npc_id, ProfileID merc_id, UINT8 quote_id, Approach);
void NpcRecordLogging(Approach, char const* fmt, ...);

void     QuestDebugScreenInit(void);
ScreenID QuestDebugScreenHandle(void);

#endif
