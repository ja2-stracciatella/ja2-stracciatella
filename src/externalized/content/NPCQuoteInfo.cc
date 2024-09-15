#include "ContentManager.h"
#include "Enums.h"
#include "Interface_Dialogue.h"
#include "ItemModel.h"
#include "Json.h"
#include "content/NPCQuoteInfo.h"

std::unique_ptr<NPCQuoteInfo const []> NPCQuoteInfo::deserialize(const JsonValue& json, const ContentManager * contentManager)
{
	auto reader = json.toObject();
	auto jsonRecords = reader["records"].toVec();
	auto buf = std::make_unique<NPCQuoteInfo[]>(NUM_NPC_QUOTE_RECORDS);
	for (auto const& jsonValue : jsonRecords)
	{
		auto const jsonRec = jsonValue.toObject();
		auto const recordIndex = jsonRec.GetUInt("index");
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
			const ItemModel* item = contentManager->getItemByName(jsonItem);
			rec->sRequiredItem = item->getItemIndex();
		}
		else rec->sRequiredGridno = -jsonRec.getOptionalInt("requiredGridNo");

		rec->usFactMustBeTrue = jsonRec.getOptionalUInt("factMustBeTrue", FACT_NONE);
		rec->usFactMustBeFalse = jsonRec.getOptionalUInt("factMustBeFalse", FACT_NONE);

		if (jsonRec.has("quest"))
		{
			auto q = jsonRec["quest"].toObject();
			uint8_t qCode = Internals::getQuestEnumFromString(q.GetString("name"));
			ST::string qStatus = q.GetString("status");
			if (qStatus == "DONE") rec->ubQuest = qCode + QUEST_DONE_NUM;
			else if (qStatus == "NOTSTARTED") rec->ubQuest = qCode + QUEST_NOT_STARTED_NUM;
			else if (qStatus == "INPROGRESS") rec->ubQuest = qCode;
		}
		else rec->ubQuest = Internals::getQuestEnumFromString("NO_QUEST");

		rec->ubFirstDay = jsonRec.getOptionalUInt("firstDay");
		rec->ubLastDay = jsonRec.getOptionalUInt("lastDay", IRRELEVANT);
		rec->ubApproachRequired = Internals::getApproachEnumFromString(jsonRec.getOptionalString("requiredApproach"));
		rec->ubOpinionRequired = jsonRec.getOptionalUInt("requiredOpinion");
		rec->ubQuoteNum = jsonRec.getOptionalUInt("quoteNum", IRRELEVANT);
		rec->ubNumQuotes = jsonRec.getOptionalUInt("numQuotes", IRRELEVANT);
		rec->ubStartQuest = Internals::getQuestEnumFromString(jsonRec.getOptionalString("startQuest"));
		rec->ubEndQuest = Internals::getQuestEnumFromString(jsonRec.getOptionalString("endQuest"));

		if (jsonRec.getOptionalBool("triggerClosestMerc")) rec->ubTriggerNPC = 0;
		else if (jsonRec.getOptionalBool("triggerSelf")) rec->ubTriggerNPC = 1;
		else if (jsonRec.has("triggerNPC"))
		{
			ST::string jsonProfileName = jsonRec.GetString("triggerNPC");
			rec->ubTriggerNPC = contentManager->getMercProfileInfoByName(jsonProfileName)->profileID;
		}
		else rec->ubTriggerNPC = IRRELEVANT;

		rec->ubTriggerNPCRec = jsonRec.getOptionalUInt("triggerRecord", IRRELEVANT);
		rec->usSetFactTrue = jsonRec.getOptionalUInt("setFactTrue", FACT_NONE);

		if (jsonRec.has("giftItem"))
		{
			ST::string jsonItem = jsonRec.GetString("giftItem");
			if (jsonItem != "65535")
			{
				const ItemModel* item = contentManager->getItemByName(jsonItem);
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
				uint8_t jsonProfileId = contentManager->getMercProfileInfoByName(jsonProfileName)->profileID;
				result = NPC_ACTION_TURN_TO_FACE_NEAREST_MERC + jsonProfileId;
			}
			else if (a.has("code")) result = a.GetInt("code");
			if (a.getOptionalBool("doFirst")) result = -result;
			rec->sActionData = result;
		}
		else rec->sActionData = NPC_ACTION_NONE;
	}
	return buf;
}

