#ifndef QUEST_DEBUG_H
#define QUEST_DEBUG_H


//Type of quote
enum
{
	QD_NPC_MSG,
	QD_QUEST_MSG,
};


// Output priority level for the npc and quest debug messages
enum
{
	QD_LEVEL_1,
	QD_LEVEL_2,
	QD_LEVEL_3,
	QD_LEVEL_4,
	QD_LEVEL_5,
};


void ToggleQuestDebugModes( UINT8 ubType );
void QuestDebugFileMsg( UINT8 ubQuoteType, UINT8 ubPriority, STR pStringA, ...);


#endif
