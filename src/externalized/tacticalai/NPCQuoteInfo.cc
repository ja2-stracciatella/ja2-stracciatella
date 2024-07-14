#include "Exceptions.h"

#include "ItemModel.h"
#include "Json.h"
#include "tacticalai/NPCQuoteInfo.h"

static uint8_t getApproachEnumFromString(const ST::string& s)
{
	if (s.empty() || s == "NONE") return Approach::APPROACH_NONE;
	else if (s == "FRIENDLY") return Approach::APPROACH_FRIENDLY;
	else if (s == "DIRECT") return Approach::APPROACH_DIRECT;
	else if (s == "THREATEN") return Approach::APPROACH_THREATEN;
	else if (s == "RECRUIT") return Approach::APPROACH_RECRUIT;
	else if (s == "REPEAT") return Approach::APPROACH_REPEAT;
	else if (s == "GIVINGITEM") return Approach::APPROACH_GIVINGITEM;
	else if (s == "NPC_INITIATING_CONV") return Approach::NPC_INITIATING_CONV;
	else if (s == "NPC_INITIAL_QUOTE") return Approach::NPC_INITIAL_QUOTE;
	else if (s == "NPC_WHOAREYOU") return Approach::NPC_WHOAREYOU;
	else if (s == "TRIGGER_NPC") return Approach::TRIGGER_NPC;
	else if (s == "GIVEFIRSTAID") return Approach::APPROACH_GIVEFIRSTAID;
	else if (s == "SPECIAL_INITIAL_QUOTE") return Approach::APPROACH_SPECIAL_INITIAL_QUOTE;
	else if (s == "ENEMY_NPC_QUOTE") return Approach::APPROACH_ENEMY_NPC_QUOTE;
	else if (s == "DECLARATION_OF_HOSTILITY") return Approach::APPROACH_DECLARATION_OF_HOSTILITY;
	else if (s == "EPC_IN_WRONG_SECTOR") return Approach::APPROACH_EPC_IN_WRONG_SECTOR;
	else if (s == "EPC_WHO_IS_RECRUITED") return Approach::APPROACH_EPC_WHO_IS_RECRUITED;
	else if (s == "INITIAL_QUOTE") return Approach::APPROACH_INITIAL_QUOTE;
	else if (s == "CLOSING_SHOP") return Approach::APPROACH_CLOSING_SHOP;
	else if (s == "SECTOR_NOT_SAFE") return Approach::APPROACH_SECTOR_NOT_SAFE;
	else if (s == "DONE_SLAPPED") return Approach::APPROACH_DONE_SLAPPED;
	else if (s == "DONE_PUNCH_0") return Approach::APPROACH_DONE_PUNCH_0;
	else if (s == "DONE_PUNCH_1") return Approach::APPROACH_DONE_PUNCH_1;
	else if (s == "DONE_PUNCH_2") return Approach::APPROACH_DONE_PUNCH_2;
	else if (s == "DONE_OPEN_STRUCTURE") return Approach::APPROACH_DONE_OPEN_STRUCTURE;
	else if (s == "DONE_GET_ITEM") return Approach::APPROACH_DONE_GET_ITEM;
	else if (s == "DONE_GIVING_ITEM") return Approach::APPROACH_DONE_GIVING_ITEM;
	else if (s == "DONE_TRAVERSAL") return Approach::APPROACH_DONE_TRAVERSAL;
	else if (s == "BUYSELL") return Approach::APPROACH_BUYSELL;
	else if (s == "ONE_OF_FOUR_STANDARD") return Approach::APPROACH_ONE_OF_FOUR_STANDARD;
	else if (s == "FRIENDLY_DIRECT_OR_RECRUIT") return Approach::APPROACH_FRIENDLY_DIRECT_OR_RECRUIT;
	else throw DataError(ST::format("unknown requiredApproach value: {}", s));
}

static uint8_t getQuestEnumFromString(const ST::string& s)
{
	if (s.empty()) return IRRELEVANT;
	else if (s == "DELIVER_LETTER") return Quests::QUEST_DELIVER_LETTER;
	else if (s == "FOOD_ROUTE") return Quests::QUEST_FOOD_ROUTE;
	else if (s == "KILL_TERRORISTS") return Quests::QUEST_KILL_TERRORISTS;
	else if (s == "KINGPIN_IDOL") return Quests::QUEST_KINGPIN_IDOL;
	else if (s == "KINGPIN_MONEY") return Quests::QUEST_KINGPIN_MONEY;
	else if (s == "RUNAWAY_JOEY") return Quests::QUEST_RUNAWAY_JOEY;
	else if (s == "RESCUE_MARIA") return Quests::QUEST_RESCUE_MARIA;
	else if (s == "CHITZENA_IDOL") return Quests::QUEST_CHITZENA_IDOL;
	else if (s == "HELD_IN_ALMA") return Quests::QUEST_HELD_IN_ALMA;
	else if (s == "INTERROGATION") return Quests::QUEST_INTERROGATION;
	else if (s == "ARMY_FARM") return Quests::QUEST_ARMY_FARM;
	else if (s == "FIND_SCIENTIST") return Quests::QUEST_FIND_SCIENTIST;
	else if (s == "DELIVER_VIDEO_CAMERA") return Quests::QUEST_DELIVER_VIDEO_CAMERA;
	else if (s == "BLOODCATS") return Quests::QUEST_BLOODCATS;
	else if (s == "FIND_HERMIT") return Quests::QUEST_FIND_HERMIT;
	else if (s == "CREATURES") return Quests::QUEST_CREATURES;
	else if (s == "CHOPPER_PILOT") return Quests::QUEST_CHOPPER_PILOT;
	else if (s == "ESCORT_SKYRIDER") return Quests::QUEST_ESCORT_SKYRIDER;
	else if (s == "FREE_DYNAMO") return Quests::QUEST_FREE_DYNAMO;
	else if (s == "ESCORT_TOURISTS") return Quests::QUEST_ESCORT_TOURISTS;
	else if (s == "FREE_CHILDREN") return Quests::QUEST_FREE_CHILDREN;
	else if (s == "LEATHER_SHOP_DREAM") return Quests::QUEST_LEATHER_SHOP_DREAM;
	else if (s == "ESTONI_FUEL") return Quests::QUEST_ESTONI_FUEL;
	else if (s == "KILL_DEIDRANNA") return Quests::QUEST_KILL_DEIDRANNA;
	else throw DataError(ST::format("unknown quest name value: {}", s));
}

NPCQuoteInfo* NPCQuoteInfo::deserialize(const JsonValue& json, const MercSystem* mercSystem, const ItemSystem* itemSystem)
{
	auto reader = json.toObject();
	auto jsonRecords = reader["records"].toVec();
	int recordIndex = 0;
	auto buf = std::make_unique<NPCQuoteInfo[]>(NUM_NPC_QUOTE_RECORDS);
	for (int i = 0; i < jsonRecords.size(); i++)
	{
		auto jsonRec = jsonRecords[i].toObject();
		recordIndex = jsonRec.GetUInt("index");
		NPCQuoteInfo* rec = &buf[recordIndex];
		rec->ubIdentifier = recordIndex;

		rec->fFlags = 0;
		if (jsonRec.getOptionalBool("alreadySaid")) rec->fFlags |= QUOTE_FLAG_SAID;
		if (jsonRec.getOptionalBool("eraseOnceSaid")) rec->fFlags |= QUOTE_FLAG_ERASE_ONCE_SAID;
		if (jsonRec.getOptionalBool("sayOncePerConvo")) rec->fFlags |= QUOTE_FLAG_SAY_ONCE_PER_CONVO;

		if (jsonRec.getOptionalBool("requiredAnyItem")) rec->sRequiredItem = ACCEPT_ANY_ITEM;
		else if (jsonRec.getOptionalBool("requiredAnyRifle")) rec->sRequiredItem = ANY_RIFLE;
		else if (jsonRec.has("requiredItem"))
		{
			ST::string jsonItem = jsonRec.GetString("requiredItem");
			const ItemModel* item = itemSystem->getItemByName(jsonItem);
			rec->sRequiredItem = item->getItemIndex();
		}
		else rec->sRequiredGridno = -jsonRec.getOptionalInt("requiredGridNo");

		rec->usFactMustBeTrue = jsonRec.getOptionalUInt("factMustBeTrue", FACT_NONE);
		rec->usFactMustBeFalse = jsonRec.getOptionalUInt("factMustBeFalse", FACT_NONE);

		if (jsonRec.has("quest"))
		{
			auto q = jsonRec["quest"].toObject();
			uint8_t qCode = getQuestEnumFromString(q.GetString("name"));
			ST::string qStatus = q.GetString("status");
			if (qStatus == "DONE") rec->ubQuest = qCode + QUEST_DONE_NUM;
			else if (qStatus == "NOTSTARTED") rec->ubQuest = qCode + QUEST_NOT_STARTED_NUM;
			else if (qStatus == "INPROGRESS") rec->ubQuest = qCode;
		}
		else rec->ubQuest = IRRELEVANT;

		rec->ubFirstDay = jsonRec.getOptionalUInt("firstDay");
		rec->ubLastDay = jsonRec.getOptionalUInt("lastDay", IRRELEVANT);
		rec->ubApproachRequired = getApproachEnumFromString(jsonRec.getOptionalString("requiredApproach"));
		rec->ubOpinionRequired = jsonRec.getOptionalUInt("requiredOpinion");
		rec->ubQuoteNum = jsonRec.getOptionalUInt("quoteNum", IRRELEVANT);
		rec->ubNumQuotes = jsonRec.getOptionalUInt("numQuotes", IRRELEVANT);
		rec->ubStartQuest = getQuestEnumFromString(jsonRec.getOptionalString("startQuest"));
		rec->ubEndQuest = getQuestEnumFromString(jsonRec.getOptionalString("endQuest"));

		if (jsonRec.getOptionalBool("triggerClosestMerc")) rec->ubTriggerNPC = 0;
		else if (jsonRec.getOptionalBool("triggerSelf")) rec->ubTriggerNPC = 1;
		else if (jsonRec.has("triggerNPC"))
		{
			ST::string jsonProfileName = jsonRec.GetString("triggerNPC");
			uint8_t jsonProfileId = (mercSystem->getMercProfileInfoByName(jsonProfileName))->profileID;
			rec->ubTriggerNPC = jsonProfileId;
		}
		else rec->ubTriggerNPC = IRRELEVANT;

		rec->ubTriggerNPCRec = jsonRec.getOptionalUInt("triggerRecord", IRRELEVANT);
		rec->usSetFactTrue = jsonRec.getOptionalUInt("setFactTrue", FACT_NONE);

		if (jsonRec.has("giftItem"))
		{
			ST::string jsonItem = jsonRec.GetString("giftItem");
			if (jsonItem != "65535")
			{
				const ItemModel* item = itemSystem->getItemByName(jsonItem);
				rec->usGiftItem = item->getItemIndex();
			}
			else rec->usGiftItem = 65535;
		}
		else if (jsonRec.has("userInterface"))
		{
			ST::string jsonStr = jsonRec.GetString("userInterface");
			if (jsonStr == "TURN_UI_ON") rec->usGiftItem = TURN_UI_ON;
			else if (jsonStr == "TURN_UI_OFF") rec->usGiftItem = TURN_UI_OFF;
		}

		rec->usGoToGridno = jsonRec.getOptionalUInt("goToGridno", NO_MOVE);

		if (jsonRec.has("actionData"))
		{
			auto a = jsonRec["actionData"].toObject();
			int16_t result = 0;
			if (a.has("turnToFace"))
			{
				ST::string jsonProfileName = a.GetString("turnToFace");
				uint8_t jsonProfileId = (mercSystem->getMercProfileInfoByName(jsonProfileName))->profileID;
				result = NPC_ACTION_TURN_TO_FACE_NEAREST_MERC + jsonProfileId;
			}
			else if (a.has("code")) result = a.GetInt("code");
			if (a.getOptionalBool("doFirst")) result = -result;
			rec->sActionData = result;
		}
		else rec->sActionData = NPC_ACTION_NONE;
	}
	return buf.release();
}

