#pragma once

#include "ContentManager.h"
#include "Json.h"
#include "Types.h"
#include <memory>

constexpr UINT8 QUEST_NOT_STARTED_NUM = 100;
constexpr UINT8 QUEST_DONE_NUM = 200;
constexpr UINT8 NO_QUOTE = 255;
constexpr UINT8 IRRELEVANT = 255;
constexpr UINT8 MUST_BE_NEW_DAY = 254;

#define NUM_NPC_QUOTE_RECORDS  50

#define TURN_UI_OFF         65000
#define TURN_UI_ON          65001
#define SPECIAL_TURN_UI_OFF 65002
#define SPECIAL_TURN_UI_ON  65003

#define QUOTE_FLAG_SAID               0x0001
#define QUOTE_FLAG_ERASE_ONCE_SAID    0x0002
#define QUOTE_FLAG_SAY_ONCE_PER_CONVO 0x0004

#define NO_MOVE                 65535

#define ACCEPT_ANY_ITEM 1000
#define ANY_RIFLE       1001

struct NPCQuoteInfo
{
	UINT32  ubIdentifier;

	UINT16  fFlags;

	// conditions
	union
	{
		INT16 sRequiredItem;      // item NPC must have to say quote
		INT16 sRequiredGridno;    // location for NPC req'd to say quote
	};
	UINT16  usFactMustBeTrue;   // ...before saying quote
	UINT16  usFactMustBeFalse;  // ...before saying quote
	UINT8   ubQuest;            // quest must be current to say quote
	UINT8   ubFirstDay;         // first day quote can be said
	UINT8   ubLastDay;          // last day quote can be said
	UINT8   ubApproachRequired; // must use this approach to generate quote
	UINT8   ubOpinionRequired;  // opinion needed for this quote

	// quote to say (if any)
	UINT8   ubQuoteNum;         // this is the quote to say
	UINT8   ubNumQuotes;        // total # of quotes to say

	// actions
	UINT8   ubStartQuest;
	UINT8   ubEndQuest;
	UINT8   ubTriggerNPC;
	UINT8   ubTriggerNPCRec;
	UINT16  usSetFactTrue;
	UINT16  usGiftItem;         // item NPC gives to merc after saying quote
	UINT16  usGoToGridno;
	INT16   sActionData;        // special action value

	static std::unique_ptr<NPCQuoteInfo const []> deserialize(const JsonValue& json, const ContentManager * contentManager);
};
